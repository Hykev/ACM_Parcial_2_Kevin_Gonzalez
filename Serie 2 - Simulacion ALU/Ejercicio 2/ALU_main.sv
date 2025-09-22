`timescale 1ns / 1ps

module ALU_main(
    input  logic [6:0] A, B,
    input  logic [1:0] OP,            // 00=SUMA, 01=RESTA, 10=AND, 11=OR
    output logic [6:0] Resultado,
    output logic CarryOut, Overflow, Cero, Negativo
);
    logic [6:0] sum;
    logic sub;
    logic [6:0] B2;
    logic cout;

    // Control de resta
    assign sub = (OP == 2'b01);
    assign B2  = sub ? ~B : B;

    // Instancia sumador de 7 bits
    B7Adder Adder7bits (
        .A(A),
        .B(B2),
        .Cin(sub),
        .S(sum),
        .Cout(cout)
    );

    // Selección de operación
    always_comb begin
        case (OP)
            2'b00: Resultado = sum;      // SUMA
            2'b01: Resultado = sum;      // RESTA
            2'b10: Resultado = A & B;    // AND
            2'b11: Resultado = A | B;    // OR
        endcase
    end

    // Banderas
    assign Cero     = (Resultado == 0 & CarryOut == 0);                   
    assign Negativo = (OP == 2'b00 || OP == 2'b01) ? Resultado[6] : 1'b0;
    assign Overflow = (OP == 2'b00 || OP == 2'b01) ?
                      (A[6] & B2[6] & ~sum[6]) | (~A[6] & ~B2[6] & sum[6]) :
                      1'b0;
    assign CarryOut = (OP == 2'b00) ? cout : 1'b0;
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