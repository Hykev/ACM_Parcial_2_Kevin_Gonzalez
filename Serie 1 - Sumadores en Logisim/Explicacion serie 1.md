# 📘 Serie 1 – Estudio de Sumadores y Simulación en Logisim

## 🎯 Instrucciones
Utilizando **Logisim Evolution**, se implementaron y simularon tres arquitecturas de sumadores para analizar su operación, funcionamiento y tiempos de propagación:

- Ripple-Carry Adder  
- Carry-Lookahead Adder  
- Prefix Adder  

Además, se realizó un análisis de ventajas y desventajas de cada arquitectura y se determinó qué sumador conviene utilizar en diferentes escenarios de aplicación.  

---

## 📐 Ecuaciones utilizadas

### Carry-Lookahead Adder
Para generar los **carry-in** intermedios, se utilizó la fórmula:

\[
C_3 = G_3 + P_3 G_2 + P_3 P_2 G_1 + P_3 P_2 P_1 G_0 + P_3 P_2 P_1 P_0 C_{-1}
\]

donde:
- \( G_i \): señales de **generate**  
- \( P_i \): señales de **propagate**  
- \( C_{-1} \): carry inicial (carry-in)  

---

## ⏱️ Análisis de Timing

Los tiempos de propagación se calcularon utilizando los **delays físicos de compuertas reales** (según hojas de datos de DigiKey):

- **XOR:** 14.5 ns → [74AHC1G86SE-7](https://www.digikey.com/en/products/detail/diodes-incorporated/74AHC1G86SE-7/2639252)  
- **AND:** 4.5 ns → [SN74LVC1G08DRLR](https://www.digikey.com/en/products/detail/texas-instruments/SN74LVC1G08DRLR/770502)  
- **OR:** 4.5 ns → [SN74LVC1G32DBVR](https://www.digikey.com/en/products/detail/texas-instruments/SN74LVC1G32DBVR/381323)  

### Ripple-Carry Adder (RCA)
\[
t_{ripple} = N_{tfa} \cdot t_{fa}
\]  
\[
t_{fa} = t_{XOR} + t_{AND} + t_{OR} = 14.5 + 4.5 + 4.5 = 23.5 \, ns
\]  
\[
t_{ripple} = 4 \cdot 23.5 = 94 \, ns
\]  

➡️ **Ventaja:** diseño simple y de bajo costo.  
➡️ **Desventaja:** tiempo de propagación crece linealmente con el número de bits.  

---

### Carry-Lookahead Adder (CLA)
Reduce la dependencia lineal del ripple al calcular los carries en paralelo mediante señales \( G \) y \( P \).  

⚡ Se obtiene una reducción significativa en los niveles lógicos comparado con el RCA, aunque el circuito requiere más compuertas.  

➡️ **Ventaja:** mucho más rápido que el RCA.  
➡️ **Desventaja:** más costoso en términos de área y complejidad.  

---

### Prefix Adder (PA)
\[
t_{PA} = t_{pg} + \log_2(N) \cdot t_{pg\_prefix} + t_{XOR}
\]  
\[
t_{PA} = 4.5 + \log_2(4)(4.5 + 4.5) + 14.5
\]  
\[
t_{PA} = 4.5 + 2(9) + 14.5 = 37 \, ns
\]  

➡️ **Ventaja:** el más rápido, escala logarítmicamente con los bits.  
➡️ **Desventaja:** mayor número de compuertas y área.  

---

## 📊 Comparación de Tiempos

| Arquitectura         | Tiempo (ns) | Ventaja principal         | Desventaja principal         |
|-----------------------|-------------|---------------------------|-------------------------------|
| Ripple-Carry Adder    | 94 ns       | Simplicidad, bajo costo   | Lento para más bits          |
| Carry-Lookahead Adder | 72.47 ns    | Más rápido que ripple     | Más compuertas               |
| Prefix Adder          | 37 ns       | Escala logarítmicamente   | Alto consumo de área         |

---

## 📌 Selección de Sumador por Aplicación

- **Aplicaciones lentas con restricción de espacio y presupuesto:**  
  → ✔️ **Ripple-Carry Adder** (simple y barato).  

- **Aplicaciones rápidas sin restricción de espacio/presupuesto:**  
  → ✔️ **Prefix Adder** (máxima velocidad).  

- **Aplicaciones rápidas con restricción de espacio/presupuesto:**  
  → ✔️ **Prefix Adder** (sigue siendo la mejor opción por velocidad).  

---

## 🎥 Video Explicativo
👉 [Ver video de la explicación](https://youtu.be/dRjvz8mXMr0)  

---

