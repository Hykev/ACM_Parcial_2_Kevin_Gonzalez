# ACM_Parcial_2_Kevin_Gonzalez
- Examen parcial No. 2 de la clase de Arquitectura de Computadoras y Microcontroladores (ACM).

---

# 📚 Resumen

Este repositorio contiene los entregables de las diferentes **series de ejercicios** realizados durante el parcial, organizados en carpetas por serie.

Cada serie cuenta con su propia documentación en **Markdown** y los **archivos fuente** correspondientes (códigos en C/SystemVerilog Logisim, etc.).

---

## 📘 Serie 0 – Máquina de Estados (Moore + Mealy)

* Proyecto: **control de talanquera y sellado de ticket**.
* Dos MEFs implementadas en **SystemVerilog**:

  * Moore → sellado de ticket (validación con monedas).
  * Mealy → control de la talanquera.
* Módulo Top conecta ambas y se probó en simulación.
* Documentación con diagramas de estado y videos explicativos.

📄 Más detalle en [`Serie3.md`](./Serie3.md)

---

## 📘 Serie 1 – Estudio de Sumadores en Logisim

* Implementación en **Logisim Evolution** de tres arquitecturas de sumadores:

  * Ripple-Carry Adder (RCA)
  * Carry-Lookahead Adder (CLA)
  * Prefix Adder (PA)
* Se analizaron los **tiempos de propagación** usando delays reales de compuertas (datasheets DigiKey).
* Comparación de ventajas/desventajas según aplicación.

📄 Más detalle en [`Serie1.md`](./Serie1.md)

---

## 📘 Serie 2 – ALU personalizada

* Implementación de una **ALU de N bits** (definido por número de carné).
* Operaciones básicas: suma, resta, AND, OR.
* Extensión a 6 operaciones: se añadieron desplazamientos (Shift Left/Right).
* Se diseñó primero en **Logisim** y luego se replicó en **SystemVerilog (Vivado)**.

📄 Más detalle en [`Serie2.md`](./Serie2.md)

---

## 📘 Serie 3 – Implementación de FSM y ALU en Hardware
* Pendiente.

---

## 📘 Serie 4 – Reloj digital con alarma (STM32 Nucleo)

* Implementación en **C bare-metal** sobre un **STM32 Nucleo-L053R8**.
* Características:

  * Reloj en formato **24h/12h** (seleccionable con el botón USER).
  * Visualización en **4 displays de 7 segmentos** multiplexados.
  * **Alarma programada en código** que activa un buzzer durante 10 s.
  * LED adicional que indica **AM/PM**.
* Uso de **transistores 2N2222A** para manejar cada display y buzzer.

📄 Más detalle en [`Serie4.md`](./Serie4.md)

---

## ▶️ Videos explicativos

Cada serie cuenta con un video explicativo donde se muestra el diseño, simulación o pruebas físicas.
Los enlaces están incluidos dentro de cada markdown detallado.

---
