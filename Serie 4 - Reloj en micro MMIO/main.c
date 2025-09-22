/**
 * Reloj 4 dígitos (HH:MM) con alarma, botón de formato 24h/12h, LED AM y buzzer
 * MCU: STM32L053 (Nucleo-L053R8 t3).
 *
 * Mapeo:
 *  - Segmentos del display a..g  = PB0..PB6   (activo ALTO, cátodo común)
 *  - Dígitos del display D0..D3  = PC0..PC3   (activo ALTO)
 *  - Botón USER      = PC13       (entrada, activo BAJO, pull-up interno)
 *  - LED AM          = PA5        (salida, ON = AM)
 *  - Buzzer (NPN)    = PA6        (salida, maneja transistor en low-side)
 *
 * Notas de tiempo:
 *  - Se usa un "delay_ms" por software. El conteo de "minutos" está
 *    acelerado para pruebas: 1 segundo real = 1 “minuto” del reloj.
 *  - La constante de calibración a mano es de 383.
 */

#include <stdint.h>

/* ===================== BASES DE MEMORIA ===================== */
#define PERIPH_BASE   0x40000000U
#define AHB_BASE      (PERIPH_BASE + 0x00020000U)
#define RCC_BASE      (AHB_BASE   + 0x00001000U)
#define IOPORT_BASE   (PERIPH_BASE + 0x10000000U)

#define GPIOA_BASE    (IOPORT_BASE + 0x00000000U)
#define GPIOB_BASE    (IOPORT_BASE + 0x00000400U)
#define GPIOC_BASE    (IOPORT_BASE + 0x00000800U)

/* ===================== ESTRUCTURAS DE REGISTROS ===================== */
typedef struct {
    volatile uint32_t MODER;   // modo (2 bits por pin)
    volatile uint32_t OTYPER;  // tipo de salida (push-pull/open-drain)
    volatile uint32_t OSPEEDR; // velocidad
    volatile uint32_t PUPDR;   // pull-up/pull-down
    volatile uint32_t IDR;     // entrada
    volatile uint32_t ODR;     // salida
    volatile uint32_t BSRR;    // set/reset atómico de pines
    volatile uint32_t LCKR;    // lock config
    volatile uint32_t AFR[2];  // alternate func (0=0..7, 1=8..15)
    volatile uint32_t BRR;     // reset rápido (bajar pin)
} GPIOx_Reg;

typedef struct {
    volatile uint32_t CR;       // 0x00
    volatile uint32_t ICSCR;    // 0x04
    volatile uint32_t CRRCR;    // 0x08
    volatile uint32_t CFGR;     // 0x0C
    volatile uint32_t CIER;     // 0x10
    volatile uint32_t CIFR;     // 0x14
    volatile uint32_t CICR;     // 0x18
    volatile uint32_t IOPRSTR;  // 0x1C
    volatile uint32_t AHBRSTR;  // 0x20
    volatile uint32_t APB2RSTR; // 0x24
    volatile uint32_t APB1RSTR; // 0x28
    volatile uint32_t IOPENR;   // 0x2C → enable reloj GPIOx
    volatile uint32_t AHBENR;   // 0x30
    volatile uint32_t APB2ENR;  // 0x34
    volatile uint32_t APB1ENR;  // 0x38
    volatile uint32_t IOPSMENR; // 0x3C
    volatile uint32_t AHBSMENR; // 0x40
    volatile uint32_t APB2SMENR;// 0x44
    volatile uint32_t APB1SMENR;// 0x48
    volatile uint32_t CCIPR;    // 0x4C
    volatile uint32_t CSR;      // 0x50
} RCC_Reg;

/* ===================== PUNTEROS A PERIFÉRICOS ===================== */
#define GPIOA ((GPIOx_Reg*)GPIOA_BASE)
#define GPIOB ((GPIOx_Reg*)GPIOB_BASE)
#define GPIOC ((GPIOx_Reg*)GPIOC_BASE)
#define RCC   ((RCC_Reg*)RCC_BASE)

/* ===================== DEFINICIONES DEL DISPLAY ===================== */
// CTRL de dígitos (PC0..PC3), activo ALTO
#define ALL_DISPLAY_CTRL  0x0Fu        // máscara 4 bits para PC0..PC3 en binario es 0000 1111.
#define D0_CTRL           (1u<<0)      // PC0 → minutos unidad
#define D1_CTRL           (1u<<1)      // PC1 → minutos decena
#define D2_CTRL           (1u<<2)      // PC2 → horas unidad
#define D3_CTRL           (1u<<3)      // PC3 → horas decena

// Segmentos a..g en PB0..PB6 (activo ALTO)
#define SEG_MASK          0x7Fu        // bits 0..6 en binario es 0111 1111 ya que no se usa el punto decimal del display.

/* ===================== TIEMPOS (CALIBRACIÓN TUYA) ===================== */
// Refresco del multiplexado: 1 ms por dígito (rápido y estable)
#define MUX_DELAY_MS       1

// Calibración: 192:
#define TICKS_PER_SECOND   40 //192
// Para pruebas: “1 segundo real” = “1 minuto del reloj” sino multiplicar por 60.
#define TICKS_PER_MINUTE   (TICKS_PER_SECOND)

/* ===================== BUZZER ===================== */
// Buzzer vía NPN low-side controlado por PA6
#define BUZZER_PIN         6           // PA6
#define BUZZER_HZ          500         // tono aproximado 500Hz, enciende y apaga el pin 500 veces por segundo.
#define BUZZER_ON_S        10          // suena 10 s cuando dispara

// Devuelve cada cuántos "ticks" del main loop hago toggle del buzzer.
// Nota: con MUX_DELAY_MS=1 y TICKS_PER_SECOND≈766, dividir entre 1000 da 0,
// lo que provoca toggle "cada vuelta" → ~500 Hz (porque 1 toggle/ms = 500 Hz).
#define BUZZER_TOGGLE_TICKS (TICKS_PER_SECOND / (2*BUZZER_HZ))

/* ===================== LED AM ===================== */
// LED AM en PA5 (ON si h24 < 12)
#define AM_LED_PIN         5

/* ===================== BOTÓN USER ===================== */
// PC13 como entrada, activo-bajo, con pull-up interno
#define BTN_PIN            13
#define BTN_IS_PRESSED()   ((GPIOC->IDR & (1u<<BTN_PIN)) == 0)

/* ===================== ALARMA (formato 24h) ===================== */
#define ALARM_HOUR         5
#define ALARM_MINUTES      30

/* ===================== ESTADO DEL RELOJ ===================== */
// Guardamos cada dígito por claridad HH:MM → hd hu : md mu (24h internos)
typedef struct {
    uint8_t mu; // minutos unidad  (0..9)
    uint8_t md; // minutos decena  (0..5)
    uint8_t hu; // horas  unidad   (0..9)
    uint8_t hd; // horas  decena   (0..2, con lógica 24h)
} timevars_t;

// Variables globales de estado (static para ámbito de este módulo)
static timevars_t clock_digits;     // dígitos actuales del reloj
static uint32_t   loop_ticks = 0;   // cuenta vueltas del loop p/tiempo
static uint8_t    mux_idx   = 0;    // 0..3: qué dígito refrescar

static uint8_t    mode_24h  = 1;    // 1=24h, 0=12h

// Buzzer: estado y contadores
static uint8_t    buzzer_active = 0;
static uint32_t   buzzer_timer  = 0; // cuánto le falta por sonar (en ticks)
static uint32_t   buzzer_div    = 0; // divisor para generar el tono

// Botón: anti-rebote no bloqueante
static uint8_t    btn_state = 1;     // estado estable (1=reposo alto)
static uint8_t    btn_last  = 1;     // última lectura "cruda"
static uint8_t    btn_event = 0;     // flanco de bajada detectado (1 ciclo)
static uint8_t    btn_cnt   = 0;     // contador de estabilidad

/* ===================== UTILIDADES GPIO ===================== */
// Pone pin como salida (MODER=01).
static inline void gpio_make_output(GPIOx_Reg* port, uint8_t pin) {
    port->MODER  &= ~(3u << (pin*2));   // limpia 2 bits de modo
    port->MODER  |=  (1u << (pin*2));   // 01 = salida
    port->OTYPER &= ~(1u << pin);       // push-pull
    port->PUPDR  &= ~(3u << (pin*2));   // sin pull-up/down
}
static inline void gpio_set(GPIOx_Reg* port, uint8_t pin)   { port->BSRR = (1u << pin); }
static inline void gpio_clr(GPIOx_Reg* port, uint8_t pin)   { port->BSRR = (1u << (pin+16)); }

/* ===================== DELAY POR SOFTWARE ===================== */
// Aprox 1 ms * n. No bloquea “largos” (solo 1ms en el loop).
static void delay_ms(uint16_t n) {
    volatile uint16_t i;
    for (; n>0; n--) {
        for (i=0; i<140; i++) { __asm volatile("nop"); } // ~1ms aprox
    }
}

/* ===================== TABLA 7-SEGMENTOS ===================== */
// Devuelve máscara de segmentos (a..g) para un dígito 0..9 (activo ALTO)
static uint32_t seg_map(uint8_t d) {
    static const uint32_t M[10] = {
        0x3F, // 0: a b c d e f
        0x06, // 1:   b c
        0x5B, // 2: a b   d e   g
        0x4F, // 3: a b c d     g
        0x66, // 4:   b c     f g
        0x6D, // 5: a   c d   f g
        0x7D, // 6: a   c d e f g
        0x07, // 7: a b c
        0x7F, // 8: a b c d e f g
        0x6F  // 9: a b c d   f g
    };
    return M[d];
}

/* ===================== BOTÓN: ANTIRREBOTE ===================== */
// Muestreo periódico (cada iteración ~1ms), detecta flanco de bajada “limpio”
static void button_scan(void) {
    // Lectura cruda: 0 si pulsado (activo-bajo), 1 si suelto
    uint8_t raw = BTN_IS_PRESSED() ? 0 : 1;

    // Si cambió respecto a la lectura anterior, reinicio contador de estabilidad
    if (raw != btn_last) {
        btn_last = raw;
        btn_cnt  = 0;
    } else {
        // Si se mantiene igual, voy acumulando “estabilidad”
        if (btn_cnt < 5) {            // ~5 ms
            btn_cnt++;
        } else {
            // Al pasar el tiempo de estabilidad, actualizo estado estable
            if (btn_state != raw) {
                btn_state = raw;
                // Si el estado estable es 0 (pulsado), genero evento de flanco
                if (btn_state == 0) {
                    btn_event = 1;    // se leerá una sola vez en el bucle
                }
            }
        }
    }
}

/* ===================== ACTUALIZAR TIEMPO (ESCALA PRUEBA) ===================== */
// Suma “1 minuto” del reloj (acelerado) cada TICKS_PER_MINUTE
static void time_update_if_needed(void) {
    if (loop_ticks >= TICKS_PER_MINUTE) {
        loop_ticks = 0;                   // reinicia cuenta para el próximo “minuto”
        // ++ minutos (mu/md)
        if (++clock_digits.mu == 10) {    // 0..9
            clock_digits.mu = 0;
            if (++clock_digits.md == 6) { // 0..5
                clock_digits.md = 0;
                // ++ horas (hu/hd) formato 24h interno
                if (++clock_digits.hu == 10) {
                    clock_digits.hu = 0;
                    clock_digits.hd++;
                }
                // roll-over 24:00 → 00:00
                if (clock_digits.hd == 2 && clock_digits.hu == 4) {
                    clock_digits.hd = 0;
                    clock_digits.hu = 0;
                }
            }
        }
    }
}

/* ===================== ALARMA ===================== */
// Activa el buzzer 10 s cuando h:m coincide con ALARM_HOUR:ALARM_MINUTES
static void alarm_check_and_trigger(void) {
    uint8_t h24 = (uint8_t)(clock_digits.hd*10 + clock_digits.hu);
    uint8_t m   = (uint8_t)(clock_digits.md*10 + clock_digits.mu);

    if (h24 == ALARM_HOUR && m == ALARM_MINUTES && !buzzer_active) {
        buzzer_active = 1;                                  // enciende modo buzzer
        buzzer_timer  = TICKS_PER_SECOND * BUZZER_ON_S;     // cuenta atrás (10 s)
        buzzer_div    = 0;                                  // reinicia divisor de tono
    }
}

/* ===================== BUZZER (TONO NO BLOQUEANTE) ===================== */
// Genera una onda cuadrada con toggles espaciados por BUZZER_TOGGLE_TICKS.
static void buzzer_update(void) {
    if (buzzer_active) {
        // Si BUZZER_TOGGLE_TICKS sale 0 (por enteros), el >= se cumple siempre
        // → toggle cada iteración (~1k toggles/s a MUX=1ms) ⇒ ~500 Hz
        if (++buzzer_div >= BUZZER_TOGGLE_TICKS) {
            buzzer_div = 0;
            // Toggle PA6 leyendo ODR (más claro de entender)
            if (GPIOA->ODR & (1u << BUZZER_PIN)) {
                gpio_clr(GPIOA, BUZZER_PIN);
            } else {
                gpio_set(GPIOA, BUZZER_PIN);
            }
        }
        // Cuenta atrás del tiempo de buzzer
        if (buzzer_timer > 0) {
            buzzer_timer--;
        }
        // Al llegar a cero, apaga buzzer
        if (buzzer_timer == 0) {
            buzzer_active = 0;
            gpio_clr(GPIOA, BUZZER_PIN);
        }
    } else {
        // Asegura buzzer apagado cuando no está activo
        gpio_clr(GPIOA, BUZZER_PIN);
    }
}

/* ===================== LED AM ===================== */
// Enciende PA5 si h24 < 12 (AM), lo apaga en PM
static void am_led_update(void) {
    uint8_t h24 = (uint8_t)(clock_digits.hd*10 + clock_digits.hu);
    if (h24 < 12) { gpio_set(GPIOA, AM_LED_PIN); }
    else          { gpio_clr(GPIOA, AM_LED_PIN); }
}

/* ===================== FORMATO 24h / 12h ===================== */
// Devuelve la hora que se debe MOSTRAR considerando el modo
static uint8_t display_hour_from_h24(uint8_t h24) {
    if (mode_24h) {           // formato 24h → se muestra tal cual
        return h24;
    } else {                  // formato 12h → 0→12, 13..23→1..11
        if (h24 == 0) return 12;
        if (h24 > 12) return (uint8_t)(h24 - 12);
        return h24;
    }
}

/* ===================== REFRESCO DISPLAY (MULTIPLEXADO) ===================== */
// Apaga dígitos y segmentos, luego muestra el dígito seleccionado por mux_idx.
static void display_refresh_step(void) {
    // 1) Apaga todos los dígitos y todos los segmentos
    GPIOC->BSRR = (ALL_DISPLAY_CTRL << 16); // OFF dígitos (PC0..PC3)
    GPIOB->BSRR = (SEG_MASK << 16);         // OFF segmentos (PB0..PB6)

    // 2) Calcula los 4 dígitos a mostrar (según formato)
    uint8_t h24    = (uint8_t)(clock_digits.hd*10 + clock_digits.hu);
    uint8_t h_show = display_hour_from_h24(h24);

    uint8_t d_min_u = clock_digits.mu;              // D0
    uint8_t d_min_d = clock_digits.md;              // D1
    uint8_t d_hr_u  = (uint8_t)(h_show % 10);       // D2
    uint8_t d_hr_d  = (uint8_t)(h_show / 10);       // D3

    // 3) Enciende UNO de los dígitos y carga sus segmentos
    switch (mux_idx) {
        case 0: // Minutos Unidad (PC0)
            GPIOB->BSRR = seg_map(d_min_u); // enciende segmentos del número
            GPIOC->BSRR = D0_CTRL;          // habilita dígito D0
            mux_idx = 1;                    // siguiente
            break;

        case 1: // Minutos Decena (PC1)
            GPIOB->BSRR = seg_map(d_min_d);
            GPIOC->BSRR = D1_CTRL;
            mux_idx = 2;
            break;

        case 2: // Horas Unidad (PC2)
            GPIOB->BSRR = seg_map(d_hr_u);
            GPIOC->BSRR = D2_CTRL;
            mux_idx = 3;
            break;

        default: // 3: Horas Decena (PC3)
            GPIOB->BSRR = seg_map(d_hr_d);
            GPIOC->BSRR = D3_CTRL;
            mux_idx = 0;
            break;
    }
}

/* ===================== MAIN ===================== */
int main(void) {
    /* --- Habilitar reloj de GPIO A/B/C --- */
    RCC->IOPENR |= (1u<<0) | (1u<<1) | (1u<<2); // bits: A=0, B=1, C=2

    /* --- Configurar PB0..PB6 como salidas (segmentos a..g) --- */
    for (uint8_t i=0; i<7; i++) { gpio_make_output(GPIOB, i); }
    GPIOB->BSRR = (SEG_MASK << 16); // todos los segmentos OFF

    /* --- Configurar PC0..PC3 como salidas (control de dígitos) --- */
    for (uint8_t i=0; i<4; i++) { gpio_make_output(GPIOC, i); }
    GPIOC->BSRR = (ALL_DISPLAY_CTRL << 16); // todos los dígitos OFF

    /* --- Configurar PA6 como salida (buzzer) --- */
    gpio_make_output(GPIOA, BUZZER_PIN);
    gpio_clr(GPIOA, BUZZER_PIN); // buzzer apagado

    /* --- Configurar PA5 como salida (LED AM) --- */
    gpio_make_output(GPIOA, AM_LED_PIN);
    gpio_clr(GPIOA, AM_LED_PIN); // LED AM apagado al inicio

    /* --- Configurar PC13 como entrada con pull-up (botón) --- */
    GPIOC->MODER &= ~(3u << (BTN_PIN*2));     // input
    GPIOC->PUPDR &= ~(3u << (BTN_PIN*2));     // limpia pull
    GPIOC->PUPDR |=  (1u << (BTN_PIN*2));     // 01 = pull-up

    /* --- Inicializar hora 00:00 --- */
    clock_digits.hd = 0;
    clock_digits.hu = 0;
    clock_digits.md = 0;
    clock_digits.mu = 0;

    /* --- Bucle principal --- */
    while (1) {
        // 1) Pequeño delay fijo (controla la cadencia del multiplexado)
        delay_ms(MUX_DELAY_MS);
        // 2) Cuenta ticks para el “minuto” de prueba
        loop_ticks++;

        // 3) Escanear botón sin bloquear (antirrebote)
        button_scan();
        if (btn_event) {         // hubo flanco de bajada confirmado
            btn_event = 0;       // consume el evento (solo 1 vez)
            mode_24h ^= 1;       // alterna 24h <-> 12h
        }

        // 4) Actualiza “minutos” si pasaron los ticks necesarios
        time_update_if_needed();

        // 5) LED AM según hora 24h interna
        am_led_update();

        // 6) Chequear alarma y activarla si coincide
        alarm_check_and_trigger();

        // 7) Hacer sonar (o no) el buzzer sin bloquear
        buzzer_update();

        // 8) Refrescar un dígito del display (multiplexado)
        display_refresh_step();
    }
}
