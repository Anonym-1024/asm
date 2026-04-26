
.CODE:
    main:
        mov   r0, #0          ; counter = 0

    .l loop:
        adds  r0, r0, #5      ; counter++, set flags
        cmp   r0, #15         ; compare with 10
        br(ne) .b=loop        ; if not equal, go back

    exit
