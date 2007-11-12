LOWORD  equ     [0]
HIWORD  equ     [4]

;*******************************************************************************
;lldvrm - signed long divide and remainder
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

public __alldvrm
__alldvrm proc near
        push    edi
        push    esi
        push    ebp

; Set up the local stack and save the index registers.  When this is done
; the stack frame will look as follows (assuming that the expression a/b will
; generate a call to alldvrm(a, b)):
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
;       ESP---->|      EBP      |
;               -----------------
;
DVND Equ [esp+4+(3*4)]
DVSR Equ [esp+4+8+(3*4)]
; Determine sign of the quotient (edi = 0 if result is positive, non-zero
; otherwise) and make operands positive.
; Sign of the remainder is kept in ebp.
        xor     edi, edi        ; result sign assumed positive
        mov     eax, HIWORD(DVND)
        or      eax, eax
        jge short @f
        mov     edx, LOWORD(DVND) ; lo word of a
        neg     eax             ; make a positive
        neg     edx
        sbb     eax, edi
        mov     HIWORD(DVND), eax
        mov     LOWORD(DVND), edx
        inc     edi             ; complement result sign flag
@@:     mov     ebp, edi
        mov     eax, HIWORD(DVSR)
        or      eax, eax
        jge short @f
        dec     edi             ; complement the result sign flag
        mov     edx, LOWORD(DVSR) ; lo word of a
        neg     eax             ; make b positive
        neg     edx
        sbb     eax, edi
        mov     HIWORD(DVSR), eax
        mov     LOWORD(DVSR), edx
; Now do the divide.  First look to see if the divisor is less than 4194304K.
; If so, then we can use a simple algorithm with word divides, otherwise
; things get a little more complex.
;
; NOTE - eax currently contains the high order word of DVSR
@@:     or      eax, eax         ; check to see if divisor < 4194304K
        jnz short @f
        mov     ecx, LOWORD(DVSR)
        mov     eax, HIWORD(DVND)
        xor     edx, edx
        div     ecx             ; eax <- high order bits of quotient
        mov     ebx, eax        ; save high bits of quotient
        mov     eax,LOWORD(DVND) ; edx:eax <- remainder:lo word of dividend
        div     ecx             ; eax <- low order bits of quotient
        mov     esi, eax        ; ebx:esi <- quotient
; Now we need to do a multiply so that we can compute the remainder.
        mov     eax, ebx        ; set up high word of quotient
        mul     dword ptr LOWORD(DVSR) ; HIWORD(QUOT) * DVSR
        mov     ecx, eax        ; save the result in ecx
        mov     eax, esi        ; set up low word of quotient
        mul     dword ptr LOWORD(DVSR) ; LOWORD(QUOT) * DVSR
        add     edx, ecx        ; EDX:EAX = QUOT * DVSR
        jmp short donrem        ; complete remainder calculation

; Here we do it the hard way.  Remember, eax contains the high word of DVSR
@@:     mov     ebx, eax        ; ebx:ecx <- divisor
        mov     ecx, LOWORD(DVSR)
        mov     edx, HIWORD(DVND) ; edx:eax <- dividend
        mov     eax, LOWORD(DVND)
@@:     shr     ebx, 1
        rcr     ecx, 1
        shr     edx, 1
        rcr     eax, 1
        or      ebx, ebx
        jnz short @b            ; loop until divisor < 4194304K
        div     ecx
        mov     esi, eax
; We may be off by one, so to check, we will multiply the quotient
; by the divisor and check the result against the orignal dividend
; Note that we must also check for overflow, which can occur if the
; dividend is close to 2**64 and the quotient is off by 1.
        mul     dword ptr HIWORD(DVSR) ; QUOT * HIWORD(DVSR)
        mov     ecx, eax
        mov     eax, LOWORD(DVSR)
        mul     esi             ; QUOT * LOWORD(DVSR)
        add     edx, ecx         ; EDX:EAX = QUOT * DVSR
        jc short @f
; do long compare here between original dividend and the result of the
; multiply in edx:eax.  If original is larger or equal, we are ok, otherwise
; subtract one (1) from the quotient.
        cmp     edx, HIWORD(DVND) ; compare hi words of result and original
        ja short @f             ; if result > original, do subtract
        jb short lo             ; if result < original, we are ok
        cmp     eax, LOWORD(DVND) ; hi words are equal, compare lo words
        jbe short lo            ; if less or equal we are ok, else subtract
@@:     dec     esi
        sub     eax, LOWORD(DVSR)
        sbb     edx, HIWORD(DVSR)
lo:     xor     ebx, ebx        ; ebx:esi <- quotient
; Calculate remainder by subtracting the result from the original dividend.
; Since the result is already in a register, we will do the subtract in the
; opposite direction and negate the result if necessary.
donrem: sub     eax, LOWORD(DVND)
        sbb     edx, HIWORD(DVND)
; Now check the result sign flag to see if the result is supposed to be positive
; or negative.  It is currently negated (because we subtracted in the 'wrong'
; direction), so if the sign flag is set we are done, otherwise we must negate
; the result to make it positive again.
        dec     ebp             ; check result sign flag
        jz short @f             ; result is ok, set up the quotient
        neg     edx             ; otherwise, negate the result
        neg     eax
        sbb     edx, 0
; Now we need to get the quotient into edx:eax and the remainder into ebx:ecx.
@@:     mov     ecx, edx
        mov     edx, ebx
        mov     ebx, ecx
        mov     ecx, eax
        mov     eax, esi
; Just the cleanup left to do.  edx:eax contains the quotient.  Set the sign
; according to the save value, cleanup the stack, and return.
        dec     edi             ; check to see if result is negative
        jnz short @f            ; if EDI == 0, result should be negative
        neg     edx             ; otherwise, negate the result
        neg     eax
        sbb     edx, edi
@@:     pop     ebp
        pop     esi
        pop     edi
        ret     16
__alldvrm endp

end
