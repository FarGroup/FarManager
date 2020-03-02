#include <Rtl.Base.h>

PBYTE CreateThunkEx(void *obj, void *start, void *end)
{
	int size = (PBYTE)end-(PBYTE)start;

	PBYTE code = (PBYTE)VirtualAlloc (NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy (code, start, size);

	for (PBYTE p = code; p < code+size-sizeof(DWORD_PTR); p++)
	{
		if ( *(DWORD_PTR*)p == THUNK_MAGIC )
			memcpy(p, &obj, sizeof(void*));
	}

	return code;
}

PBYTE CreateThunkFastEx(void *obj, void *start)
{
	PBYTE code = (PBYTE)VirtualAlloc(NULL, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	memcpy (code, (PBYTE)start-512, 4096);

	for (PBYTE p = code; p < code+4096-sizeof(DWORD_PTR); p++)
	{
		if ( *(DWORD_PTR*)p == THUNK_MAGIC )
			*(DWORD_PTR*)p = (DWORD_PTR)obj;
	}

	//skip debug
	return code+512;
}

void ReleaseThunkEx(PBYTE pThunk)
{
	VirtualFree(pThunk, 0, MEM_RELEASE);
}



BYTE _thunk_code[] =	{
		0x5A, // pop edx - pop ret addr
		0xB9, 0xFF, 0xFF, 0xFF, 0xFF, // mov ecx instance
		0x51, // push ecx - push instance
		0x52, // push edx - push ret address back
		0xB8, 0xFF, 0xFF, 0xFF, 0xFF, // mov eax, proc
		0xFF, 0xE0// jmp eax - switch to proc (we already have right ret addr)
		};


PBYTE CreateThunk (void *pClass, void *pClassProc)
{
	PBYTE pThunk = (PBYTE)VirtualAlloc (NULL, sizeof (_thunk_code), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy (pThunk, &_thunk_code, sizeof _thunk_code);
	memcpy (&pThunk[2], &pClass, 4);
	memcpy (&pThunk[9], &pClassProc, 4);
	return pThunk;
}

void ReleaseThunk (PBYTE pThunk)
{
	VirtualFree (pThunk, 0, MEM_RELEASE);
}

unsigned char registerCode[] = {
		0x59, //pop ecx
		0x52, //push edx
		0x50, //push eax
		0xB8, 0xFF, 0xFF, 0xFF, 0xFF, // mov eax, instance
		0x50, // push eax
		0x51, //push ecx
		0xB8, 0xFF, 0xFF, 0xFF, 0xFF, //mov eax, calladdr
		0xFF, 0xE0 //jmp eax
		};

PBYTE CreateThunkRegister(void *pClass, void *pClassProc)
{
	PBYTE pThunk = (byte*)VirtualAlloc (NULL, sizeof (registerCode), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy (pThunk, &registerCode, sizeof (registerCode));
	memcpy (&pThunk[4], &pClass, 4);
	memcpy (&pThunk[11], &pClassProc, 4);
	return pThunk;
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


PBYTE CreateThunkCdecl (void *pClass, void *pClassProc)
{
	PBYTE code = (PBYTE)VirtualAlloc (NULL, sizeof cdeclCode, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	PBYTE addr = (PBYTE)malloc (4);

	memcpy (code, &cdeclCode, sizeof cdeclCode);

	memcpy (&code[3], &addr, 4);

	memcpy (&code[8], &pClass, 4);
	memcpy (&code[14], &pClassProc, 4);

	memcpy (&code[25], &addr, 4);

	return code;
}


void ReleaseThunkCdecl (PBYTE pThunk)
{
	free((void*)*(DWORD_PTR*)(pThunk+3));
	VirtualFree(pThunk, 0, MEM_RELEASE);
}
