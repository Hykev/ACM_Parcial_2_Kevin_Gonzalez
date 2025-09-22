module shifter (
    input  logic [6:0] A,
    input  logic [1:0] OP2,   // 00=normal, 01=right, 10=left
    output logic [6:0] A_shift
);
    always_comb begin
        case (OP2)
            2'b00: A_shift = A;
            2'b01: A_shift = {A[0], A[6:1]};
            2'b10: A_shift = {A[5:0], A[6]};
            default: A_shift = A;
        endcase
    end
endmodule

// 1010110

// 


