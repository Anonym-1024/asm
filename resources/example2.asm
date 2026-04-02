.EXEC:
    
    mov(ses) r6, #0f6x
    
    f:
    ldr r8, =g
    exit
    
.DATA:
g:
    bytes(9) "nazdar"