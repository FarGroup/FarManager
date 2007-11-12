LOWORD  equ     [0]
HIWORD  equ     [4]

;*******************************************************************************
;ulldiv.asm - unsigned long divide routine
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

public __aulldiv
__aulldiv proc near
        push    ebx
        push    esi
; Set up the local stack and save the index registers.  When this is done
; the stack frame will look as follows (assuming that the expression a/b will
; generate a call to uldiv(a, b)):
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
;               |      EBX      |
;               |---------------|
;       ESP---->|      ESI      |
;               -----------------
DVND Equ [esp+4+(2*4)]
DVSR Equ [esp+4+8+(2*4)]
; Now do the divide.  First look to see if the divisor is less than 4194304K.
; If so, then we can use a simple algorithm with word divides, otherwise
; things get a little more complex.
        mov     eax, HIWORD(DVSR)
        or      eax, eax
        jnz short @f
        mov     ecx, LOWORD(DVSR)
        mov     eax, HIWORD(DVND)
        xor     edx, edx
        div     ecx             ; get high order bits of quotient
        mov     ebx, eax        ; save high bits of quotient
        mov     eax, LOWORD(DVND) ; edx:eax <- remainder:lo word of dividend
        div     ecx             ; get low order bits of quotient
        mov     edx, ebx        ; edx:eax <- quotient hi:quotient lo
        jmp short done          ; restore stack and return

; Here we do it the hard way.  Remember, eax contains DVSRHI
@@:     mov     ecx,eax         ; ecx:ebx <- divisor
        mov     ebx, LOWORD(DVSR)
        mov     edx, HIWORD(DVND) ; edx:eax <- dividend
        mov     eax, LOWORD(DVND)
@@:     shr     ecx, 1
        rcr     ebx, 1
        shr     edx, 1
        rcr     eax, 1
        or      ecx, ecx
        jnz short @b            ; loop until divisor < 4194304K
        div     ebx
        mov     esi, eax
; We may be off by one, so to check, we will multiply the quotient
; by the divisor and check the result against the orignal dividend
; Note that we must also check for overflow, which can occur if the
; dividend is close to 2**64 and the quotient is off by 1.
        mul     dword ptr HIWORD(DVSR) ; QUOT * HI(DVSR)
        mov     ecx, eax
        mov     eax, LOWORD(DVSR)
        mul     esi             ; QUOT * LO(DVSR)
        add     edx, ecx        ; EDX:EAX = QUOT * DVSR
        jc short @f
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
done:   pop     esi
        pop     ebx
        ret     16
__aulldiv endp

end
