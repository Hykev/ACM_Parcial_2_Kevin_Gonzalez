`timescale 1ns / 1ps

module ALU_main(
    input  logic [6:0] A, B,
    input  logic [1:0] OP,
    output logic [7:0] Resultado,
    output logic CarryOut, Overflow, Cero
);
    logic [7:0] sum, sub;

    assign sum = A + B;
    assign sub = A + (~B + 1'b1); // Complemento A2

    always_comb begin
        Resultado = '0;
        case (OP)
            2'b00: Resultado = sum;           // SUMA
            2'b01: Resultado = sub;           // RESTA
            2'b10: Resultado = {1'b0, A & B}; // AND
            2'b11: Resultado = {1'b0, A | B}; // OR
        endcase
    end

    // Banderas
    assign CarryOut = (OP == 2'b00) ? sum[7] : 1'b0;

    assign Overflow = (OP == 2'b01) ?
                       (A[6] & ~B[6] & ~sub[6]) |
                       (~A[6] & B[6] & sub[6]) :
                       1'b0;

    assign Cero = (Resultado == 0);

endmodule

// --------

// overflow
// -50 - 30
// A (-50) = 0110010 → 1001110
// B (30) = 0011110 → 1100010

// 10110000

// -------

// sin overflow
// -20 - 30
// A (-20) = 1101100
// B (30) = 0011110 → 1100010

// 11001110