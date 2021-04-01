; This is just some LC3 assembly that I found somewhere
; It creates an array of 10 successive powers of 10
    .ORIG x3000       
    LD R1,Max       
    LD R0,Zero       
    LD R7,Zero   
Loop:
    LEA R6,Array
    LD R3,Zero
    ADD R4,R0,#0
Inner:
    ADD R3,R3,#2
    ADD R4,R4,#-1
    BRp Inner

    ADD R6,R6,R7
    STR R3,R6,#0

    ADD R2,R0,#0
    AND R0,R0,#0
MostInner:
    ADD R0,R0,#2
    ADD R2,R2,#-1
    BRp MostInner

    ADD R7,R7,#1
    ADD R1,R1,#-1
    BRp Loop


    ; Loading Last value in register R2
    LDR R2,R6,#0

    HALT
Array    .BLKW #10
Max    .FILL #10
Zero    .FILL #0
    .END
