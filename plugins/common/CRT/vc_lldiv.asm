LOWORD  equ     [0]
HIWORD  equ     [4]

;*******************************************************************************
;lldiv - signed long divide
;
;Entry:
;       Arguments are passed on the stack:
;               1st pushed: divisor (QWORD)
;               2nd pushed: dividend (QWORD)
;
;Exit:
;       EDX:EAX contains the quotient (dividend/divisor)
;       NOTE: this routine removes the parameters from the stack.
;*******************************************************************************
.486
.model flat
.code

public __alldiv
__alldiv proc near
        push    edi
        push    esi
        push    ebx
; Set up the local stack and save the index registers.  When this is done
; the stack frame will look as follows (assuming that the expression a/b will
; generate a call to lldiv(a, b)):
;
;               -----------------
;               |               |
;               |---------------|
;               |               |
;               |--divisor (b)--|
;               |               |
;               |---------------|
;               |               |
;               |--dividend (a)-|
;               |               |
;               |---------------|
;               | return addr** |
;               |---------------|
;               |      EDI      |
;               |---------------|
;               |      ESI      |
;               |---------------|
;       ESP---->|      EBX      |
;               -----------------
;
DVND Equ [esp+4+(3*4)]
DVSR Equ [esp+4+8+(3*4)]
; Determine sign of the result (edi = 0 if result is positive, non-zero
; otherwise) and make operands positive.
        xor     edi, edi        ; result sign
        mov     eax, HIWORD(DVND)
        or      eax, eax
        jge short @f
        mov     edx, LOWORD(DVND)
        neg     eax
        neg     edx
        sbb     eax, edi
        inc     edi
        mov     HIWORD(DVND), eax
        mov     LOWORD(DVND), edx
@@:     mov     eax, HIWORD(DVSR)
        or      eax, eax
        jge short @f
        dec     edi
        mov     edx, LOWORD(DVSR)
        neg     eax
        neg     edx
        sbb     eax, edi
        mov     HIWORD(DVSR), eax
        mov     LOWORD(DVSR), edx
; Now do the divide.  First look to see if the divisor is less than 4194304K.
; If so, then we can use a simple algorithm with word divides, otherwise
; things get a little more complex.
@@:     or      eax,eax         ; HI(DVSR)
        jnz short @f
        mov     ecx, LOWORD(DVSR)
        mov     eax, HIWORD(DVND)
        xor     edx, edx
        div     ecx
        mov     ebx, eax
        mov     eax, LOWORD(DVND)
        div     ecx
        mov     edx, ebx
        jmp short done

; Here we do it the hard way.  Remember, eax contains the high word of DVSR
@@:     mov     ebx,eax         ; ebx:ecx <- divisor
        mov     ecx, LOWORD(DVSR)
        mov     edx, HIWORD(DVND) ; edx:eax <- dividend
        mov     eax, LOWORD(DVND)
@@:     shr     ebx, 1
        rcr     ecx, 1
        shr     edx, 1
        rcr     eax, 1
        or      ebx, ebx
        jnz short @b            ; loop until divisor < 4194304K
        div     ecx             ; now divide, ignore remainder
        mov     esi, eax        ; save quotient
; We may be off by one, so to check, we will multiply the quotient
; by the divisor and check the result against the orignal dividend
; Note that we must also check for overflow, which can occur if the
; dividend is close to 2**64 and the quotient is off by 1.
        mul     dword ptr HIWORD(DVSR) ; QUOT * HI(DVSR)
        mov     ecx, eax
        mov     eax, LOWORD(DVSR)
        mul     esi             ; QUOT * LO(DVSR)
        add     edx, ecx        ; EDX:EAX = QUOT * DVSR
        jc short @f             ; Quotient is off by 1
; do long compare here between original dividend and the result of the
; multiply in edx:eax.  If original is larger or equal, we are ok, otherwise
; subtract one (1) from the quotient.
        cmp     edx, HIWORD(DVND) ; compare hi words of result and original
        ja short @f             ; if result > original, do subtract
        jb short lo             ; if result < original, we are ok
        cmp     eax, LOWORD(DVND) ; hi words are equal, compare lo words
        jbe short lo            ; if less or equal we are ok, else subtract
@@:     dec     esi             ; subtract 1 from quotient
lo:     xor     edx, edx        ; edx:eax <- quotient
        mov     eax, esi
; Just the cleanup left to do.  edx:eax contains the quotient.  Set the sign
; according to the save value, cleanup the stack, and return.
done:   dec     edi             ; check to see if result is negative
        jnz short @f            ; if EDI == 0, result should be negative
        neg     edx             ; otherwise, negate the result
        neg     eax
        sbb     edx, edi
@@:     pop     ebx
        pop     esi
        pop     edi
        ret     16
__alldiv endp

end
