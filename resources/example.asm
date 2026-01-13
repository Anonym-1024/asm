.DATA:
    bytes(2) {89, 90}
    byte 7

    var1:
    bytes(10) "AHOJ."


.EXEC:
main:
.l loop:
    cmp r0, r1
    !b(eq) .f loop
    mov r1, r0
    !b .b loop
.l loop:

.DATA:
    

