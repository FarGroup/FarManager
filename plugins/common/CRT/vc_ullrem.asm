LOWORD  equ     [0]
HIWORD  equ     [4]

;*******************************************************************************
;ullrem.asm - unsigned long remainder routine
;
;Entry:
;       Arguments are passed on the stack:
;               1st pushed: divisor (QWORD)
;               2nd pushed: dividend (QWORD)
;
;Exit:
;       EDX:EAX contains the remainder (dividend%divisor)
;       NOTE: this routine removes the parameters from the stack.
;*******************************************************************************
.486
.model flat
.code

public __aullrem
__aullrem proc near
        push    ebx
; Set up the local stack and save the index registers.  When this is done
; the stack frame will look as follows (assuming that the expression a%b will
; generate a call to ullrem(a, b)):
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
;       ESP---->|      EBX      |
;               -----------------

DVND Equ [esp+4+4]
DVSR Equ [esp+4+4+8]
; Now do the divide.  First look to see if the divisor is less than 4194304K.
; If so, then we can use a simple algorithm with word divides, otherwise
; things get a little more complex.
        mov     eax, HIWORD(DVSR)
        or eax, eax
        jnz short @f
        mov     ecx, LOWORD(DVSR)
        mov     eax, HIWORD(DVND)
        xor     edx, edx
        div     ecx             ; edx <- remainder, eax <- quotient
        mov     eax, LOWORD(DVND) ; edx:eax <- remainder:lo word of dividend
        div     ecx             ; edx <- final remainder
        mov     eax, edx        ; edx:eax <- remainder
        xor     edx, edx
        jmp short done

; Here we do it the hard way.  Remember, eax contains DVSRHI
@@:     mov     ecx, eax        ; ecx:ebx <- divisor
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
; We may be off by one, so to check, we will multiply the quotient
; by the divisor and check the result against the orignal dividend
; Note that we must also check for overflow, which can occur if the
; dividend is close to 2**64 and the quotient is off by 1.
        mov     ecx, eax        ; save a copy of quotient in ECX
        mul     dword ptr HIWORD(DVSR)
        xchg    ecx, eax        ; put partial product in ECX, get quotient in EAX
        mul     dword ptr LOWORD(DVSR)
        add     edx, ecx        ; EDX:EAX = QUOT * DVSR
        jc short @f
; do long compare here between original dividend and the result of the
; multiply in edx:eax.  If original is larger or equal, we're ok, otherwise
; subtract the original divisor from the result.
        cmp     edx, HIWORD(DVND) ; compare hi words of result and original
        ja short @f             ; if result > original, do subtract
        jb short lo             ; if result < original, we're ok
        cmp     eax, LOWORD(DVND) ; hi words are equal, compare lo words
        jbe short lo            ; if less or equal we're ok, else subtract
@@:     sub     eax, LOWORD(DVSR)
        sbb     edx, HIWORD(DVSR)
; Calculate remainder by subtracting the result from the original dividend.
; Since the result is already in a register, we will perform the subtract in
; the opposite direction and negate the result to make it positive.
lo:     sub     eax, LOWORD(DVND)
        sbb     edx, HIWORD(DVND)
        neg     edx
        neg     eax
        sbb     edx, 0
done:   pop     ebx
        ret     16
__aullrem endp

end
