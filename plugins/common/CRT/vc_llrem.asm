LOWORD  equ     [0]
HIWORD  equ     [4]

;*******************************************************************************
;llrem - signed long remainder
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

public __allrem
__allrem proc near
        push    ebx
        push    edi
; Set up the local stack and save the index registers.  When this is done
; the stack frame will look as follows (assuming that the expression a%b will
; generate a call to lrem(a, b)):
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
;               |       EBX     |
;               |---------------|
;       ESP---->|       EDI     |
;               -----------------
DVND Equ [esp+4+8]
DVSR Equ [esp+4+8+8]
        xor     edi, edi        ; nosign flag
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
        mov     edx, LOWORD(DVSR)
        neg     eax
        neg     edx
        sbb     eax, 0
        mov     HIWORD(DVSR), eax
        mov     LOWORD(DVSR), edx
; Now do the divide.  First look to see if the divisor is less than 4194304K.
; If so, then we can use a simple algorithm with word divides, otherwise
; things get a little more complex.
;
; NOTE - eax currently contains the high order word of DVSR
@@:     or      eax, eax        ; check to see if divisor < 4194304K
        jnz short @f
        mov     ecx, LOWORD(DVSR)
        mov     eax, HIWORD(DVND)
        xor     edx, edx
        div     ecx             ; edx <- remainder
        mov     eax, LOWORD(DVND) ; edx:eax <- remainder:lo word of dividend
        div     ecx             ; edx <- final remainder
        xor     eax, eax
        xchg    edx, eax
        jmp short cksign

; Here we do it the hard way.  Remember, eax contains the high word of DVSR
@@:     mov     ebx, eax        ; ebx:ecx <- divisor
        mov     ecx,LOWORD(DVSR)
        mov     edx,HIWORD(DVND) ; edx:eax <- dividend
        mov     eax,LOWORD(DVND)
@@:     shr     ebx, 1
        rcr     ecx, 1
        shr     edx, 1
        rcr     eax, 1
        or      ebx, ebx
        jnz short @b            ; loop until divisor < 4194304K
        div     ecx             ; now divide, ignore remainder
; We may be off by one, so to check, we will multiply the quotient
; by the divisor and check the result against the orignal dividend
; Note that we must also check for overflow, which can occur if the
; dividend is close to 2**64 and the quotient is off by 1.
        mov     ecx, eax        ; save a copy of quotient in ECX
        mul     dword ptr HIWORD(DVSR)
        xchg    ecx, eax        ; save product, get quotient in EAX
        mul     dword ptr LOWORD(DVSR)
        add     edx, ecx        ; EDX:EAX = QUOT * DVSR
        jc short @f
; do long compare here between original dividend and the result of the
; multiply in edx:eax.  If original is larger or equal, we are ok, otherwise
; subtract the original divisor from the result.
        cmp     edx,HIWORD(DVND) ; compare hi words of result and original
        ja short @f             ; if result > original, do subtract
        jb short lo             ; if result < original, we are ok
        cmp     eax, LOWORD(DVND) ; hi words are equal, compare lo words
        jbe short lo            ; if less or equal we are ok, else subtract
@@:     sub     eax, LOWORD(DVSR)
        sbb     edx, HIWORD(DVSR)
; Calculate remainder by subtracting the result from the original dividend.
; Since the result is already in a register, we will do the subtract in the
; opposite direction and negate the result if necessary.
lo:     sub     eax, LOWORD(DVND)
        sbb     edx, HIWORD(DVND)
; Now check the result sign flag to see if the result is supposed to be positive
; or negative.  It is currently negated (because we subtracted in the 'wrong'
; direction), so if the sign flag is set we are done, otherwise we must negate
; the result to make it positive again.
        xor     edi, 1
cksign: dec     edi
        jnz short @f
        neg     edx
        neg     eax
        sbb     edx, edi
@@:     pop     edi
        pop     ebx
        ret     16
__allrem endp

end
