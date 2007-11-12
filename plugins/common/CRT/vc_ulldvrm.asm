LOWORD  equ     [0]
HIWORD  equ     [4]

;*******************************************************************************
;ulldvrm.asm - unsigned long divide and remainder routine
;
;Entry:
;       Arguments are passed on the stack:
;               1st pushed: divisor (QWORD)
;               2nd pushed: dividend (QWORD)
;
;Exit:
;       EDX:EAX contains the quotient (dividend/divisor)
;       EBX:ECX contains the remainder (divided % divisor)
;       NOTE: this routine removes the parameters from the stack.
;*******************************************************************************
.486
.model flat
.code

public __aulldvrm
__aulldvrm proc near
        push    esi
; Set up the local stack and save the index registers.  When this is done
; the stack frame will look as follows (assuming that the expression a/b will
; generate a call to aulldvrm(a, b)):
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
;       ESP---->|      ESI      |
;               -----------------
;
DVND Equ [esp+4+4]
DVSR Equ [esp+4+4+8]
; Now do the divide.  First look to see if the divisor is less than 4194304K.
; If so, then we can use a simple algorithm with word divides, otherwise
; things get a little more complex.
        mov     eax, HIWORD(DVSR) ; check to see if divisor < 4194304K
        or      eax, eax
        jnz short @f
        mov     ecx, LOWORD(DVSR) ; load divisor
        mov     eax, HIWORD(DVND) ; load high word of dividend
        xor     edx, edx
        div     ecx             ; get high order bits of quotient
        mov     ebx, eax        ; save high bits of quotient
        mov     eax, LOWORD(DVND) ; edx:eax <- remainder:lo word of dividend
        div     ecx             ; get low order bits of quotient
        mov     esi, eax        ; ebx:esi <- quotient
; Now we need to do a multiply so that we can compute the remainder.
        mov     eax, ebx        ; set up high word of quotient
        mul     dword ptr LOWORD(DVSR) ; HIWORD(QUOT) * DVSR
        mov     ecx, eax        ; save the result in ecx
        mov     eax, esi        ; set up low word of quotient
        mul     dword ptr LOWORD(DVSR) ; LOWORD(QUOT) * DVSR
        add     edx, ecx        ; EDX:EAX = QUOT * DVSR
        jmp short mkrem

; Here we do it the hard way.  Remember, eax contains DVSRHI
@@:     mov     ecx, eax         ; ecx:ebx <- divisor
        mov     ebx, LOWORD(DVSR)
        mov     edx, HIWORD(DVND) ; edx:eax <- dividend
        mov     eax, LOWORD(DVND)
@@:     shr     ecx, 1
        rcr     ebx, 1
        shr     edx, 1
        rcr     eax, 1
        or      ecx,ecx
        jnz short @b            ; loop until divisor < 4194304K
        div     ebx
        mov     esi, eax
; We may be off by one, so to check, we will multiply the quotient
; by the divisor and check the result against the orignal dividend
; Note that we must also check for overflow, which can occur if the
; dividend is close to 2**64 and the quotient is off by 1.
        mul     dword ptr HIWORD(DVSR) ; QUOT * HIWORD(DVSR)
        mov     ecx, eax
        mov     eax, LOWORD(DVSR)
        mul     esi             ; QUOT * LOWORD(DVSR)
        add     edx, ecx        ; EDX:EAX = QUOT * DVSR
        jc short @f
; do long compare here between original dividend and the result of the
; multiply in edx:eax.  If original is larger or equal, we are ok, otherwise
; subtract one (1) from the quotient.
        cmp     edx,HIWORD(DVND) ; compare hi words of result and original
        ja short @f             ; if result > original, do subtract
        jb short lo             ; if result < original, we are ok
        cmp     eax, LOWORD(DVND) ; hi words are equal, compare lo words
        jbe short lo            ; if less or equal we are ok, else subtract
@@:     dec     esi
        sub     eax, LOWORD(DVSR)
        sbb     edx, HIWORD(DVSR)
lo:     xor     ebx,ebx         ; ebx:esi <- quotient
mkrem:  sub     eax, LOWORD(DVND)
        sbb     edx, HIWORD(DVND)
        neg     edx
        neg     eax
        sbb     edx, 0
; Now we need to get the quotient into edx:eax and the remainder into ebx:ecx.
        mov     ecx, edx
        mov     edx, ebx
        mov     ebx, ecx
        mov     ecx, eax
        mov     eax, esi
        pop     esi
        ret     16
__aulldvrm endp

end
