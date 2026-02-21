

.EXEC:
main:
.l loop:
    cmp r0, r1
    !b(eq) .f(7) loop
    mov r1, r0
    !b .b loop
.l loop:



