#include <Rtl.Base.h>

#ifdef _WIN64

//.code
//     mov  rax, [rsp]
//     mov  [0AAFFAAFFAAFFAAFFh], rax
//     mov  [rsp+4*8], r9
//     mov  r9, r8
//     mov  r8, rdx
//     mov  rdx, rcx
//     mov  rcx, 0AAFFAAFFAAFFAAFFh
//     mov  rax, 0AAFFAAFFAAFFAAFFh
//     call rax
//     add  rsp, 8
//     mov  rdx, 0AAFFAAFFAAFFAAFFh
//     jmp dword ptr [rdx]
//end



byte _thunk_code_x64[] = {
		0x48, 0x8B, 0x04, 0x24,
		0x48, 0xA3, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA,
		0x4C, 0x89, 0x4C, 0x24, 0x20,
		0x4D, 0x8B, 0xC8,
		0x4C, 0x8B, 0xC2,
		0x48, 0x8B, 0xD1,
		0x48, 0xB9, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA,
		0x48, 0xB8, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA,
		0xFF, 0xD0,
		0x48, 0x83, 0xC4, 0x08,
		0x48, 0xBA, 0x22, 0x11, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA,
		0xFF, 0x22
		};

byte* CreateThunk (void *pClass, void *pClassProc)
{
	byte *code = (byte*)VirtualAlloc (NULL, sizeof _thunk_code_x64, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	byte *addr = (byte*)malloc (8);

	memcpy (code, &_thunk_code_x64, sizeof _thunk_code_x64);

	memcpy (&code[6], &addr, 8);

	memcpy (&code[30], &pClass, 8);
	memcpy (&code[40], &pClassProc, 8);

	memcpy (&code[56], &addr, 8);

	return code;
}

void ReleaseThunk(byte *pThunk)
{
	free ((void*)*(DWORD_PTR*)(pThunk+6));
	VirtualFree (pThunk, 0, MEM_RELEASE);
}


#else

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

#endif



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
