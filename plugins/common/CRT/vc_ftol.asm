;*******************************************************************************
;ftol - double to long
;
;Entry:
;       EAX - double value
;
;Exit:
;       EAX - long value
;*******************************************************************************
.486
.model flat
.code

public __ftol2_sse
__ftol2_sse proc near
	sub	esp, 4
	fistp	dword ptr [esp]
	mov	eax, [esp]
	add	esp, 4
	ret

;	sub	esp, 8
;	fstp	qword ptr [esp]
;	cvttsd2si eax, [esp]
;	add	esp, 8
;	ret
__ftol2_sse endp

end
