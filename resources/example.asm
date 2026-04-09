
.HEAD:
    .glob j
    .extern ja


.CODE:

    mov(ses) r6, #0f6x

    j:
    ldr r8, =j
    .start:
    exit


.DATA:
l:
    bytes(10) "ahojjjdj"
