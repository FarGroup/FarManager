#include "Proclist.hpp"

TCHAR FPRINTFbuffer[FPRINTFbufferLen];

int fputc(int c, HANDLE stream)
{
  HANDLE hFile = (HANDLE)stream;
  DWORD tmp;
  TCHAR b = (TCHAR)c;
  WriteFile(hFile,&b,sizeof(b),&tmp,NULL);
  return c;
}
