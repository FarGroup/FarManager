;*******************************************************************************
;llshr - long shift right
;
;Entry:
;       EDX:EAX - long value to be shifted
;       CL    - number of bits to shift by
;
;Exit:
;       EDX:EAX - shifted value
;*******************************************************************************
.486
.model flat
.code

public __allshr
__allshr proc near
        cmp     cl, 64
        jae short dosign
        cmp     cl, 32
        jae short @f
        shrd    eax, edx, cl
        sar     edx, cl
        ret

@@:     mov     eax, edx
        sar     edx, 31
        and     cl, 31
        sar     eax, cl
        ret

dosign: sar     edx, 31
        mov     eax, edx
        ret
__allshr endp

end
