`timescale 1ns / 1ps

module B7Adder (
    input  logic [6:0] A, B,
    input  logic       Cin,
    output logic [6:0] S,
    output logic       Cout
);
    // carries internos
    logic [6:0] carry;

    // Primer bit
    FAdder FA0 (.A(A[0]), .B(B[0]), .Cin(Cin), .S(S[0]), .Cout(carry[0]));
    FAdder FA1 (.A(A[1]), .B(B[1]), .Cin(carry[0]), .S(S[1]), .Cout(carry[1]));
    FAdder FA2 (.A(A[2]), .B(B[2]), .Cin(carry[1]), .S(S[2]), .Cout(carry[2]));
    FAdder FA3 (.A(A[3]), .B(B[3]), .Cin(carry[2]), .S(S[3]), .Cout(carry[3]));
    FAdder FA4 (.A(A[4]), .B(B[4]), .Cin(carry[3]), .S(S[4]), .Cout(carry[4]));
    FAdder FA5 (.A(A[5]), .B(B[5]), .Cin(carry[4]), .S(S[5]), .Cout(carry[5]));
    FAdder FA6 (.A(A[6]), .B(B[6]), .Cin(carry[5]), .S(S[6]), .Cout(Cout));
    
endmodule
