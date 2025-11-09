// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

// Fill.asm
// Sets screen to black when any key is pressed
// Clears screen when no key is pressed

(START)
    @SCREEN
    D=A
    @addr
    M=D         // addr = SCREEN base address
    @8192
    D=A
    @count
    M=D         // count = total number of screen words

    @KBD
    D=M         // D = keyboard input
    @BLACK_LOOP
    D;JNE       // if any key is pressed, go to BLACK
    @WHITE_LOOP
    0;JMP       // else, go to WHITE
    

(BLACK_LOOP)
    @addr
    A=M
    M=-1        // set pixel to black
    @addr
    M=M+1       // addr++
    @count
    M=M-1       // count--
    D=M
    @BLACK_LOOP
    D;JGT       // repeat until count == 0
    @START
    0;JMP       // go back to keyboard check

(WHITE_LOOP)
    @addr
    A=M
    M=0         // clear pixel
    @addr
    M=M+1       // addr++
    @count
    M=M-1       // count--
    D=M
    @WHITE_LOOP
    D;JGT       // repeat until count == 0
    @START
    0;JMP       // go back to keyboard check
