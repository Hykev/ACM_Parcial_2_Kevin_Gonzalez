`timescale 1ns / 1ps

module ALU (
    input  logic [6:0] A, B,
    input  logic [3:0] ALUControl,   // bits [3:2] de shift y bits [1:0] de op
    output logic [6:0] Resultado,
    output logic CarryOut, Overflow, Cero, Negativo
);
    logic [6:0] A_shifted;

    // shifter
    shifter shift (
        .A(A),
        .OP2(ALUControl[3:2]),
        .A_shift(A_shifted)
    );

    // ALU
    ALU_main main (
        .A(A_shifted),
        .B(B),
        .OP(ALUControl[1:0]),
        .Resultado(Resultado),
        .CarryOut(CarryOut),
        .Overflow(Overflow),
        .Cero(Cero),
        .Negativo(Negativo)
    );
endmodule
