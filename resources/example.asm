.EXEC:
    
    mov(ses) r6, #0f6x
    .l stitek:
    .start:
    e:
    ldr r8, .b=stitek
    exit
    
.DATA:
d:
    bytes(5) "ahoj"