.EXEC:
    mov r4, #5
    .start:
    ldr r4, #-23
    br d
    
.DATA:
    c:
    bytes(8) {8,9,9,0}
    a:
    byte
.EXEC:

        