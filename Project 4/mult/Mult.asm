// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
// The algorithm is based on repetitive addition.

    @R1
    D=M
    @n
    M=D
    @total
    M=0 
    
    //initialising i=0 to Count
    @i
    M=0

(LOOP)
    //if i-n==0 then go to STOP
    @i
    D=M
    @n
    D=D-M
    @STOP
    D;JEQ

    //keep adding R1 and incrementing i by 1
    @R0
    D=M
    @total
    M=M+D
    @i
    M=M+1
    @LOOP
    0;JMP

(STOP)
    @total
    D=M
    @R2
    M=D
    @END
    0;JMP

(END)
    @END
    0;JMP



