#include "Proclist.hpp"

wchar_t FPRINTFbuffer[FPRINTFbufferLen];

int fputc(int c, HANDLE stream)
{
	HANDLE hFile = (HANDLE)stream;
	DWORD tmp;
	wchar_t b = (wchar_t)c;
	WriteFile(hFile,&b,sizeof(b),&tmp,NULL);
	return c;
}
