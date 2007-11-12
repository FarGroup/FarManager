;*******************************************************************************
;ullshr - long shift right
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

public __aullshr
__aullshr proc near
        cmp     cl, 64
        jae short clean
        cmp     cl, 32
        jae short @f
        shrd    eax, edx, cl
        shr     edx, cl
        ret

@@:     mov     eax, edx
        xor     edx, edx
        and     cl, 31
        shr     eax, cl
        ret

clean:  xor     eax, eax
        mov     edx, eax
        ret
__aullshr endp

end
