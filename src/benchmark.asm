LOAD R5, 0
OUTER:
    LOAD R0, 7E; Load the end of the capital English ASCII block.
    LOAD R1, 1; Load the increment register.
    LOAD R2, 20h; Load the start of the capital English ASCII block.
LOOP:
    MOVE RF, R2; Move the value at R2 to the STDOUT register.
    ADDI R2, R2, R1; Increment
    JMPLE R2<=R0, loop
    LOAD R0, FEh
    ADDI R5, R5, R1
    JMPLE R5<=R0, OUTER
    HALT