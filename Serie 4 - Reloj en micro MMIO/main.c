#include <stdint.h>

/* Direcciones base */
#define PERIPH_BASE  0x40000000U
#define AHB_BASE     (PERIPH_BASE + 0x00020000U)
#define RCC_BASE     (AHB_BASE + 0x00001000U)
#define IOPORT_BASE  (PERIPH_BASE + 0x10000000U)

#define GPIOA_BASE   (IOPORT_BASE + 0x00000000U)
#define GPIOB_BASE   (IOPORT_BASE + 0x00000400U)
#define GPIOC_BASE   (IOPORT_BASE + 0x00000800U)

/* Registros */
typedef struct {
    volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2], BRR;
} GPIOx_Reg;

typedef struct {
    volatile uint32_t CR, ICSCR, CRRCR, CFGR, CIER, CIFR, CICR, IOPRSTR,
                      AHBRSTR, APB2RSTR, APB1RSTR, IOPENR, AHBENR, APB2ENR,
                      APB1ENR, IOPSMENR, AHBSMENR, APB2SMENR, APB1SMENR, CCIPR, CSR;
} RCC_Reg;

#define GPIOA ((GPIOx_Reg*)GPIOA_BASE)
#define GPIOB ((GPIOx_Reg*)GPIOB_BASE)
#define GPIOC ((GPIOx_Reg*)GPIOC_BASE)
#define RCC   ((RCC_Reg*)RCC_BASE)

/* Display */
#define ALL_DISPLAY_CTRL  0x0F
#define D0_CTRL (1u<<0)
#define D1_CTRL (1u<<1)
#define D2_CTRL (1u<<2)
#define D3_CTRL (1u<<3)
#define SEG_MASK 0x7F

/* Tiempo */
#define MUX_DELAY_MS       1
#define TICKS_PER_SECOND   (383 / MUX_DELAY_MS) // 383
#define TICKS_PER_MINUTE   (60 * TICKS_PER_SECOND)

/* Buzzer */
#define BUZZER_PIN   6
#define BUZZER_HZ    500
#define BUZZER_ON_S  10
#define BUZZER_TOGGLE_TICKS 0

/* LED AM */
#define AM_LED_PIN   5

/* Botón */
#define BTN_PIN      13
#define BTN_PRESSED()  ((GPIOC->IDR & (1u<<BTN_PIN)) == 0)

/* Alarma */
#define ALARM_HOUR     5
#define ALARM_MINUTES  30

typedef struct {
    uint8_t mu, md, hu, hd;
} timevars_t;

static timevars_t clock;
static uint32_t time_keeper = 0;
static uint8_t  my_fsm = 0;

static uint8_t mode_24h = 1;
static uint8_t buzzer_active = 0;
static uint32_t buzzer_timer = 0;
static uint32_t buzzer_tick = 0;

/* delay */
static void delay_ms(uint16_t n) {
    volatile uint16_t i;
    for(; n>0; n--) {
        for(i=0;i<140;i++) { __asm volatile("nop"); }
    }
}

/* mapa de segmentos */
static uint32_t segmap(uint8_t d){
    static const uint32_t M[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
    return M[d];
}

int main(void){
    // habilitar clocks
    RCC->IOPENR |= (1u<<0) | (1u<<1) | (1u<<2);

    // configurar GPIOB como salida para segmentos
    for(int i=0;i<7;i++){
        GPIOB->MODER &= ~(3u<<(i*2));
        GPIOB->MODER |=  (1u<<(i*2));
    }
    GPIOB->BSRR = (SEG_MASK<<16);

    // configurar GPIOC como salida para dígitos
    for(int i=0;i<4;i++){
        GPIOC->MODER &= ~(3u<<(i*2));
        GPIOC->MODER |=  (1u<<(i*2));
    }
    GPIOC->BSRR = (ALL_DISPLAY_CTRL<<16);

    // configurar PA6 buzzer
    GPIOA->MODER &= ~(3u<<(BUZZER_PIN*2));
    GPIOA->MODER |=  (1u<<(BUZZER_PIN*2));
    GPIOA->BSRR = (1u<<(BUZZER_PIN+16));

    // configurar PA5 LED AM
    GPIOA->MODER &= ~(3u<<(AM_LED_PIN*2));
    GPIOA->MODER |=  (1u<<(AM_LED_PIN*2));
    GPIOA->BSRR = (1u<<(AM_LED_PIN+16));

    // configurar PC13 botón
    GPIOC->MODER &= ~(3u<<(BTN_PIN*2));
    GPIOC->PUPDR &= ~(3u<<(BTN_PIN*2));
    GPIOC->PUPDR |=  (1u<<(BTN_PIN*2));

    // hora inicial
    clock.hd=0; clock.hu=0; clock.md=0; clock.mu=0;

    uint8_t btn_prev = 1;

    // loop infinito
    while(1){
        delay_ms(MUX_DELAY_MS);
        time_keeper++;

        // botón
        uint8_t btn_now = BTN_PRESSED();
        if(btn_now && !btn_prev){
            mode_24h ^= 1;
        }
        btn_prev = btn_now;

        // incremento de tiempo
        if(time_keeper >= TICKS_PER_MINUTE){
            time_keeper = 0;
            if(++clock.mu == 10){ clock.mu=0; if(++clock.md==6){
                clock.md=0;
                if(++clock.hu==10){ clock.hu=0; ++clock.hd; }
                if(clock.hd==2 && clock.hu==4){ clock.hd=0; clock.hu=0; }
            }}
        }

        // LED AM
        uint8_t h24 = clock.hd*10 + clock.hu;
        if(h24 < 12) GPIOA->BSRR = (1u<<AM_LED_PIN);
        else         GPIOA->BSRR = (1u<<(AM_LED_PIN+16));

        // alarma
        uint8_t m = clock.md*10 + clock.mu;
        if(h24==ALARM_HOUR && m==ALARM_MINUTES && !buzzer_active){
            buzzer_active = 1;
            buzzer_timer  = TICKS_PER_SECOND * BUZZER_ON_S;
            buzzer_tick   = 0;
        }

        // buzzer
        if(buzzer_active){
            if(++buzzer_tick >= BUZZER_TOGGLE_TICKS){
                buzzer_tick = 0;
                if(GPIOA->ODR & (1u<<BUZZER_PIN)) GPIOA->BSRR = (1u<<(BUZZER_PIN+16));
                else                              GPIOA->BSRR = (1u<<BUZZER_PIN);
            }
            if(--buzzer_timer == 0){
                buzzer_active = 0;
                GPIOA->BSRR = (1u<<(BUZZER_PIN+16));
            }
        } else {
            GPIOA->BSRR = (1u<<(BUZZER_PIN+16));
        }

        // formato 12h
        uint8_t disp_h = h24;
        if(!mode_24h){
            if(disp_h==0) disp_h=12;
            else if(disp_h>12) disp_h-=12;
        }

        // dígitos a mostrar
        uint8_t digits[4] = {
            clock.mu,
            clock.md,
            (uint8_t)(disp_h%10),
            (uint8_t)(disp_h/10)
        };

        // multiplex
        GPIOC->BSRR = (ALL_DISPLAY_CTRL<<16);
        GPIOB->BSRR = (SEG_MASK<<16);

        switch(my_fsm){
            case 0: GPIOB->BSRR = segmap(digits[0]); GPIOC->BSRR = D0_CTRL; my_fsm=1; break;
            case 1: GPIOB->BSRR = segmap(digits[1]); GPIOC->BSRR = D1_CTRL; my_fsm=2; break;
            case 2: GPIOB->BSRR = segmap(digits[2]); GPIOC->BSRR = D2_CTRL; my_fsm=3; break;
            default:GPIOB->BSRR = segmap(digits[3]); GPIOC->BSRR = D3_CTRL; my_fsm=0; break;
        }
    }
}
