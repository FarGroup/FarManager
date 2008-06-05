#include <Rtl.Base.h>

byte _thunk_code[] =	{
		0x5A, // pop edx - pop ret addr
		0xB9, 0xFF, 0xFF, 0xFF, 0xFF, // mov ecx instance
		0x51, // push ecx - push instance
		0x52, // push edx - push ret address back
		0xB8, 0xFF, 0xFF, 0xFF, 0xFF, // mov eax, proc
		0xFF, 0xE0// jmp eax - switch to proc (we already have right ret addr)
		};


byte _thunk_code_x64_short[] = {
		0x4D, 0x8B, 0xC8, //mov r9, r8
		0x4C, 0x8B, 0xC2, //mov r8, rdx
		0x48, 0x8B, 0xD1, //mov rdx, rcx
		0x48, 0xB9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //mov rcx inst
		0x48, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //mov rax proc
		0xFF, 0xE0 //jmp rax
		};

#ifdef __X64__

byte* CreateThunk(void *pClass, void *pClassProc)
{
	byte *pThunk = (byte*)VirtualAlloc (NULL, sizeof (_thunk_code_x64_short), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy (pThunk, &_thunk_code_x64_short, sizeof (_thunk_code_x64_short));
	memcpy (&pThunk[11], &pClass, 8);
	memcpy (&pThunk[21], &pClassProc, 8);
	return pThunk;
}

#else

byte* CreateThunk (void *pClass, void *pClassProc)
{
	byte *pThunk = (byte*)VirtualAlloc (NULL, sizeof (_thunk_code), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy (pThunk, &_thunk_code, sizeof _thunk_code);
	memcpy (&pThunk[2], &pClass, 4);
	memcpy (&pThunk[9], &pClassProc, 4);
	return pThunk;
}

#endif

byte* CreateThunk64_Short (void *pClass, void *pClassProc)
{
	byte *pThunk = (byte*)VirtualAlloc (NULL, sizeof (_thunk_code_x64_short), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy (pThunk, &_thunk_code_x64_short, sizeof (_thunk_code_x64_short));
	memcpy (&pThunk[11], &pClass, 8);
	memcpy (&pThunk[21], &pClassProc, 4);
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

void ReleaseThunk (byte *pThunk)
{
	VirtualFree (pThunk, 0, MEM_RELEASE);
}

void ReleaseThunkCdecl (byte *pThunk)
{
	free ((void*)*(DWORD_PTR*)(pThunk+3));
	VirtualFree (pThunk, 0, MEM_RELEASE);
}
