LOWORD  equ     [0]
HIWORD  equ     [4]

;*******************************************************************************
;llmul - long multiply routine
;
;Entry:
;       Parameters are passed on the stack:
;               1st pushed: multiplier (QWORD)
;               2nd pushed: multiplicand (QWORD)
;Exit:
;       EDX:EAX - product of multiplier and multiplicand
;       NOTE: parameters are removed from the stack
;*******************************************************************************
.486
.model flat
.code

public __allmul
__allmul proc near
;             LO(A) * LO(B)
;       LO(A) * HI(B)
; +     LO(B) * HI(A)
; ---------------------
A Equ [esp+4]
B Equ [esp+4+8]
        mov     eax, HIWORD(A)
        mov     ecx, HIWORD(B)
        or      ecx, eax
        mov     ecx, LOWORD(B)
        jnz short @f
        mov     eax, LOWORD(A)
        mul     ecx
        ret     16

@@:     push    ebx
A Equ [esp+8]
B Equ [esp+8+8]
        mul     ecx             ; eax has HI(A), ecx has LO(B)
        mov     ebx, eax
        mov     eax, LOWORD(A)
        mul     dword ptr HIWORD(B) ; LO(A) * HI(B)
        add     ebx, eax        ; ebx = ((LO(A) * HI(B)) + (HI(A) * LO(B)))
        mov     eax, LOWORD(A)
        mul     ecx             ; ecx = LO(B)
        add     edx, ebx        ; edx has all the LO*HI stuff
        pop     ebx
        ret     16
__allmul endp

        end
