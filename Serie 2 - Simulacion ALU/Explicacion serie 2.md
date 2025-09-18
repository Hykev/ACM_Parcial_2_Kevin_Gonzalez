# Serie 2 – Ejercicio 1: Implementación de una ALU personalizada en Logisim

## 📖 Instrucciones
La ALU debía cumplir con los siguientes requisitos:
- Operar con un tamaño de palabra **N bits** calculado en función del número de carné:  

\[
N = (2 \times \text{último dígito}) - (\text{penúltimo dígito})
\]

Para mi carné `16698`:  

\[
N = (2 \times 8) - 9 = 7 \, \text{bits}
\]

- Implementar inicialmente **4 operaciones básicas**:
  - Suma  
  - Resta  
  - AND  
  - OR  

- Extender la funcionalidad de **4 a 6 operaciones**, agregando:
  - Shift Left (desplazamiento a la izquierda)  
  - Shift Right (desplazamiento a la derecha)  

- El bloque de control (**ALUControl**) se amplió para soportar las 6 operaciones.  
- Se utilizó **diseño jerárquico en Logisim**.

---

## ⚙️ Diseño de la ALU en Logisim

### Versión 1 – 4 operaciones
La primera versión de la ALU se diseñó con entradas de **7 bits** (`A` y `B`) y salida de **8 bits** (incluyendo el `CarryOut`).  

Operaciones disponibles:
- `00`: Suma  
- `01`: Resta (implementada con complemento a 2)  
- `10`: AND  
- `11`: OR  

### Versión 2 – 6 operaciones
En la segunda versión se amplió el **ALUControl** para soportar dos operaciones adicionales:
- Shift Left  
- Shift Right  

El resultado de los **shifts** se conecta como entrada auxiliar para las 4 operaciones anteriores, extendiendo así la funcionalidad sin duplicar lógica innecesaria.

---

## 📊 Características del diseño
- **Entradas:**  
  - `A` (7 bits)  
  - `B` (7 bits)  
  - `ALUControl` (4 bits, selecciona entre 6 operaciones)  

- **Salida:**  
  - `Resultado` (8 bits, incluye `CarryOut`)  
  - `Flags`:  
    - `Cero` (cuando el resultado es 0)  
    - `Neg` (cuando el resultado es negativo)  
    - `CarryOut` (de la suma/resta)  
    - `Overflow` (detectado al comparar `carry[6]` y `carry[5]`)  

---

## 🎥 Video explicativo
🔗 [Explicación ALU en Logisim – Serie 2, Ejercicio 1](https://youtu.be/uXdzYs134HA)

---


## Ejercicio 2 – Implementación en Vivado (SystemVerilog)

- En este segundo ejercicio, se replicó la misma lógica diseñada en Logisim pero implementada en SystemVerilog dentro de Vivado.

### Características:

- **Entradas:** A[6:0], B[6:0] (operandos de 7 bits).

- Control de operación expandido para soportar 6 funciones.

- **Salidas:** Resultado[7:0], CarryOut, Overflow, Cero.

- La lógica de Overflow y CarryOut se maneja en función de si la operación es suma o resta.

- El shifter mantiene el mismo comportamiento circular que en Logisim.


## Video explicativo

🔗 [Explicación ALU en Logisim – Serie 2, Ejercicio 1](https://youtu.be/DH39wHL2PXU)