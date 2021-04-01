; Adding a column of numbers using a sentinel 
; Taken from "Introduction to Computing Systems", Yale Patt, Sanjay Patel
; 
; Stefan Wong 2018

    .ORIG x3000
    LEA R1,FirstVal    ; load R1 with value in x3100
    AND R3,R3,#0
    LDR R4,R1,#0
TestEnd:   BRn Done ; figure out when to exit the loop
    ADD R3, R3,R4   ; perform the sum
    ADD R1,R1,#1    ; increment R1
    LDR R4,R1,#0
    BRnzp TestEnd

Done: HALT
FirstVal: .FILL #64
.END
