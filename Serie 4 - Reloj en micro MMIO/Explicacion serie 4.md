# Serie 4 – Ejercicio: Reloj digital con alarma en STM32 Nucleo

## 📖 Instrucciones

El ejercicio consistía en implementar un **reloj digital** con las siguientes características:

* Mostrar horas y minutos en **formato 24h o 12h seleccionable** con el botón de usuario de la tarjeta.
* Utilizar **4 displays de 7 segmentos** (cátodo común) multiplexados para mostrar `HH:MM`.
* Incluir una **alarma programada en el código** que active un **buzzer** durante 10 segundos cuando la hora coincida.
* Agregar un **LED indicador de AM/PM**:

  * Encendido = AM
  * Apagado = PM

---

## ⚙️ Diseño físico del sistema

### 🔹 Microcontrolador

* **Placa:** STM32 Nucleo-L053R8
* **MCU:** STM32L053R8T6 (ARM Cortex-M0+)
* **Memoria Flash:** 64 KB
* **Frecuencia:** hasta 32 MHz (se trabajó con HSI16, calibrado con pruebas empíricas).
* **Periféricos usados:**

  * GPIOA, GPIOB, GPIOC
  * RCC (para habilitar relojes de los puertos)

### 🔹 Displays de 7 segmentos

* Se usó un bloque de **4 dígitos de cátodo común**.
* **Segmentos (a–g):** conectados a `PB0–PB6`.
* **Control de dígito:** cada display se enciende mediante `PC0–PC3`, activados de uno en uno (multiplexado).
* Para manejar la corriente de cada display, se utilizaron **transistores NPN 2N2222A**, cada uno conectado a la línea común de un dígito.

### 🔹 Botón de usuario

* **PC13** (ya presente en la Nucleo).
* Entrada con **pull-up interno**, activo bajo.
* Se usa para alternar entre formato **24h ↔ 12h**.

### 🔹 LED indicador de AM/PM

* **PA5** (LED extra colocado en protoboard).
* Encendido = AM, apagado = PM.

### 🔹 Buzzer

* **PA6** controla un **2N2222A** que conmuta el buzzer a GND (low-side).
* Buzzer conectado a +5 V → buzzer → colector del NPN.
* Se incluye una resistencia de **1 kΩ en la base** del transistor.
* Genera un tono aproximado de **500 Hz** al activar la alarma.
* Puede colocarse un **LED en paralelo con el buzzer** para visualizar cuándo está activo.

---

## 📊 Características del diseño

* **Entradas:**

  * Botón `USER` (PC13).

* **Salidas:**

  * 4 displays multiplexados (PC0–PC3, PB0–PB6).
  * Buzzer en PA6.
  * LED AM en PA5.

* **Funciones principales del código:**

  * **Multiplexado:** activa cada display durante \~1 ms, refrescando los 4 en bucle.
  * **Contador de tiempo:** simula minutos y horas a partir de un retardo calibrado.
  * **Formato 24h/12h:** controlado por el botón con antirrebote no bloqueante.
  * **Alarma:** definida en código (`ALARM_HOUR`, `ALARM_MINUTES`).
  * **Buzzer:** genera un tono de 500 Hz durante 10 segundos al coincidir la alarma.
  * **LED AM:** indica si la hora actual es AM (<12h) o PM (≥12h).

---

## 📝 Resumen del código

El programa en **C bare-metal** (sin HAL, solo registros) sigue estos pasos:

1. **Inicialización:**

   * Se habilitan relojes de GPIOA, GPIOB, GPIOC.
   * Se configuran pines como salidas (segmentos, dígitos, buzzer, LED) o entradas (botón).

2. **Bucle principal:**

   * Actualiza el tiempo con base en ticks calibrados.
   * Escanea el botón con antirrebote para cambiar entre 24h/12h.
   * Enciende/apaga el LED AM según la hora interna.
   * Revisa si coincide con la hora de la alarma y activa el buzzer.
   * Genera el tono del buzzer sin bloquear el programa.
   * Refresca un dígito del display en cada iteración (multiplexado).

El código se adjunta en el repositorio/documentación con comentarios línea por línea para su análisis.

---

## 🎥 Video explicativo

- 
🔗 [Explicación Reloj Digital - Físico – Serie 4](https://youtu.be/5oLm2V7d4OQ)

🔗 [Explicación Reloj Digital - Código en STM32 – Serie 4](https://youtu.be/8-lAR5zSuz8)

---

