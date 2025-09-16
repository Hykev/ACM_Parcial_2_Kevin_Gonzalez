# Proyecto — Máquina de sellado y control de talanquera (Moore + Mealy)

> Contenido: módulo de sellado de ticket (MEF tipo Moore), módulo de talanquera (MEF tipo Mealy) y top que los conecta.
> Lenguaje: **SystemVerilog** — Síntesis / Simulación en **Vivado**

---

## 1. Resumen del sistema

En este ejercicio se implementa dos máquinas de estado finito que trabajan juntas para sellar tickets y controlar una talanquera de acceso:

* **M1\_Moore** — máquina de sellado de ticket (Moore). Controla señales internas de ticket y validación (`T`, `V`) y un contador simplificado (`D`) que representa número de monedas ingresadas (max 2 unidades).
* **M2\_Mealy** — máquina que recibe la información del ticket y control de sensor de vehículo (`C`) y controla la talanquera (`TAL`, con 4 códigos: abajo, subiendo, arriba, bajando).
* **Top** — conecta ambas máquinas, comparte `clk` y `R` y realiza las conversiones necesarias entre señales.

---

## 2. Archivos

* `M1_Moore.sv` — implementación de la máquina Moore (sellado).
* `M2_Mealy.sv` — implementación de la máquina Mealy (talanquera).
* `Top.sv` — top-level que instancia y conecta ambas MEF.

---

## 3. Detalle — M1\_Moore (máquina de sellado)

**Función:** recibir ticket (`iT`) y monedas (`iM`) para validar el ticket. Se requieren 2 monedas para validar (`D`), al validar `V=1`. `T` indica presencia del ticket en proceso. `R` es reset síncrono.

### Entradas / Salidas

| Señal    | Dir | Descripción                             |
| -------- | --: | --------------------------------------- |
| `clk`    |  in | reloj principal (flanco posedge)        |
| `iT`     |  in | sensor/entrada de ticket (trigger)      |
| `iM`     |  in | detector de moneda (1 pulse por moneda) |
| `R`      |  in | reset síncrono                          |
| `T`      | out | ticket presente (salida Moore)          |
| `V`      | out | ticket validado (salida Moore)          |
| `D[1:0]` | out | contador/estado de monedas (00/01/10)   |

### Estados (S0..S3) y comportamiento

* **S0 (Inicio, abajo)**

  * Salida: `T=0, V=0, D=00`.
  * Si `iT==1` → `next = S1`.
* **S1 (ticket recibido)**

  * Salida: `T=1, V=0, D=00`.
  * Si `iM==1` → `next = S2`.
* **S2 (1 moneda)**

  * Salida: `T=1, V=0, D=01`.
  * Si `iM==1` → `next = S3`.
* **S3 (2 monedas — validado)**

  * Salida: `T=1, V=1, D=10`.
  * Si `R==1` → `next = S0` (reset al ciclo siguiente).

**Notas de diseño**

* Reset síncrono (`always_ff @(posedge clk)` con `if (R)`).
* `always_comb` con `nextstate = state;` al inicio para evitar rutas sin asignar y latches.
* Salidas dependientes solo del `state` (Moore).

---

## 4. Detalle — M2\_Mealy (control talanquera)

**Función:** controlar el movimiento de la talanquera según entrada de ticket, validación y sensor de carro en la salida (`C`). Es una MEF tipo Mealy: salidas dependen del estado **y** de algunas entradas.

### Entradas / Salidas

| Señal      | Dir | Descripción                                                    |
| ---------- | --: | -------------------------------------------------------------- |
| `clk`      |  in | reloj principal                                                |
| `R`        |  in | reset síncrono                                                 |
| `T`        |  in | ticket presente (en Top se conecta como negado o como quieras) |
| `V`        |  in | ticket validado (conectado desde `V` de M1)                    |
| `C`        |  in | sensor carro en salida (1 = carro presente)                    |
| `TAL[1:0]` | out | estado talanquera codificado (2 bits)                          |

### Codificación de salida (enum)

En el diseño usamos un `typedef enum logic [1:0] { DOWN, UP_START, UP, DOWN_START }` para legibilidad; su mapeo binario (por orden) es:

* `DOWN` = `2'b00` — talanquera abajo
* `UP_START` = `2'b01` — subiendo
* `UP` = `2'b10` — arriba
* `DOWN_START` = `2'b11` — bajando

### Estados (S0..S3) y comportamiento (resumen)

* **S0 (idle abajo)**

  * Si `T & V & C` → pasa a **S1**; mientras no, salida `DOWN`.
  * En S0 cuando la condición se cumple: salida `UP_START`.
* **S1 (en subida / mantener)**

  * Si `T & V & C` → pasa a **S2** y salida `UP`.
  * Si falta alguna entrada, se queda en S1 y salida `UP_START`.
* **S2 (talanquera arriba, carro presente)**

  * La entrada `T` y `V` ya no importan.
  * Si `C==1` → se queda en S2 (salida `UP`).
  * Si `C==0` → pasa a **S3** con salida `DOWN_START`.
* **S3 (bajando)**

  * Mientras `C==1` → se mantiene `DOWN_START`.
  * Si `C==0` → pasa a S0 y salida `DOWN`.

**Notas**

* `nextstate` siempre asignado explícitamente en `always_comb` (evita latches).
* Salidas definidas con valor por defecto y `case(state)` (Mealy: dependen de `state` y `C` o de `T&V&C` en los casos de S0/S1).

---

## 5. Top — Conexión entre M1 y M2

### Conexiones principales

* `clk` y `R` son compartidos entre ambos módulos.
* **Mapping usado en tu Top final (archivo `Top.sv` que compartiste):**

  * `T = ~iT` — la entrada `T` de la máquina M2 está conectada al **negado** de la entrada `iT` de M1.
  * `V2 = V1` — la entrada `V` de M2 toma la salida `V1` de M1.
  * `C` se deja libre en la top (puede ser un switch o estímulo en testbench).


### Visibilidad en simulación

* Para facilitar la lectura en la waveform, en `Top.sv` se definió un `typedef enum` interno y un signal `TAL_text` que mapea los códigos binarios (`TAL[1:0]`) a nombres (`DOWN`, `UP_START`, ...). Vivado muestra los enums legibles en el waveform si el simulador lo soporta.

---

## 6. Diagramas

![Diagrama de estados - M1](images/diagramaMEF1.png)
*Figura 1 — Diagrama de estados de M1 (Moore).*

![Diagrama de estados - M2](images/diagramaMEF2.png)
*Figura 2 — Diagrama de estados de M2 (Mealy).*

## Video explicativo
[Ver video donde explico el funcionamiento y la simulación](https://youtu.be/Br5kJR3VgAM)

---
