.EXEC:
    
    mov r6, #06x
    .stafrt:
    ldr r8, =d
    exit
    
.DATA:
d:
    bytes(7) "ahoj" 