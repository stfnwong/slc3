; ADD_TEST
; Test program for ADD instruction 
; 
; Stefan Wong 2018

    .ORIG x3000
    LD R1,Val1
    LD R2,Val2
    ADD R3,R1,R2
    HALT 
Val1: .FILL #1
Val2: .FILL #2
.END
