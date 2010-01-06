#include <Rtl.Base.h>

byte *CreateThunkEx(void *obj, void *start, void *end)
{
	int size = (byte*)end-(byte*)start;

	byte *code = (byte*)VirtualAlloc (NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy (code, start, size);

	for (byte *p = code; p < code+size-sizeof(DWORD_PTR); p++)
	{
		if ( *(DWORD_PTR*)p == THUNK_MAGIC )
			memcpy(p, &obj, sizeof(void*));
	}

	return code;
}


byte *CreateThunkFastEx(void *obj, void *start)
{
	byte *code = (byte*)VirtualAlloc (NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy (code, start, 4096);

	for (byte *p = code; p < code+4096-sizeof(DWORD_PTR); p++)
	{
		if ( *(DWORD_PTR*)p == THUNK_MAGIC )
			memcpy(p, &obj, sizeof(void*));
	}

	return code;
}


void ReleaseThunkEx (byte *pThunk)
{
	VirtualFree (pThunk, 0, MEM_RELEASE);
}



byte _thunk_code[] =	{
		0x5A, // pop edx - pop ret addr
		0xB9, 0xFF, 0xFF, 0xFF, 0xFF, // mov ecx instance
		0x51, // push ecx - push instance
		0x52, // push edx - push ret address back
		0xB8, 0xFF, 0xFF, 0xFF, 0xFF, // mov eax, proc
		0xFF, 0xE0// jmp eax - switch to proc (we already have right ret addr)
		};


byte* CreateThunk (void *pClass, void *pClassProc)
{
	byte *pThunk = (byte*)VirtualAlloc (NULL, sizeof (_thunk_code), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy (pThunk, &_thunk_code, sizeof _thunk_code);
	memcpy (&pThunk[2], &pClass, 4);
	memcpy (&pThunk[9], &pClassProc, 4);
	return pThunk;
}

void ReleaseThunk (byte *pThunk)
{
	VirtualFree (pThunk, 0, MEM_RELEASE);
}


unsigned char cdeclCode[] = {//all ecx!!
		0x59, //pop ebx
		0x89, 0x0D, 0xFF, 0xFF, 0xFF, 0xFF, // mov [???], ebx
		0xB9, 0xFF, 0xFF, 0xFF, 0xFF, // mov ebx, ???
		0x51, // push ebx
		0xB9, 0xFF, 0xFF, 0xFF, 0xFF, //mov ebx, ????
		0xFF, 0xD1, //call ebx
		0x83, 0xC4, 0x04, //add esp, 4
		0x8B, 0x0D, 0xFF, 0xFF, 0xFF, 0xFF, // mov ebx, [???]
		0x51, //push ebx
		0xC3
		};


byte *CreateThunkCdecl (void *pClass, void *pClassProc)
{
	byte *code = (byte*)VirtualAlloc (NULL, sizeof cdeclCode, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	byte *addr = (byte*)malloc (4);

	memcpy (code, &cdeclCode, sizeof cdeclCode);

	memcpy (&code[3], &addr, 4);

	memcpy (&code[8], &pClass, 4);
	memcpy (&code[14], &pClassProc, 4);

	memcpy (&code[25], &addr, 4);

	return code;
}


void ReleaseThunkCdecl (byte *pThunk)
{
	free ((void*)*(DWORD_PTR*)(pThunk+3));
	VirtualFree (pThunk, 0, MEM_RELEASE);
}
