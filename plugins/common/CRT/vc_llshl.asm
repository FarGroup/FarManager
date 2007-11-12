;*******************************************************************************
;llshl.asm - long shift left
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

public __allshl
__allshl proc near
        cmp     cl, 64
        jae short clean
        cmp     cl, 32
        jae short @f
        shld    edx, eax, cl
        shl     eax, cl
        ret

@@:     mov     edx, eax
        xor     eax, eax
        and     cl, 31
        shl     edx, cl
        ret

clean:  xor     eax, eax
        mov     edx, eax
        ret
__allshl endp

end
