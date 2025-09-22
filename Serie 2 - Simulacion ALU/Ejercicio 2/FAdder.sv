`timescale 1ns / 1ps

module FAdder(
        input logic A, B, Cin,
        output logic S, Cout);
        
    logic P, G;
    

    assign P = A ^ B;
    assign G = A & B;

    assign S = P ^ Cin; // Suma
    assign Cout = G | (P & Cin); // Carry
endmodule