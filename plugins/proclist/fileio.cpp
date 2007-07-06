#include "Proclist.hpp"

int fprintf(HANDLE stream, const TCHAR *format, ...)
{
  int ret=0;
  HANDLE hFile = (HANDLE)stream;
  static TCHAR buffer[64*1024];
  DWORD tmp;
  if(format)
  {
    va_list argptr;
    va_start(argptr,format);
    ret=wvsprintf(buffer,format,argptr);
    va_end(argptr);
    if (WriteFile(hFile,buffer,ret*sizeof(TCHAR),&tmp,NULL))
      ret = (tmp + sizeof(TCHAR)-1) / sizeof(TCHAR);
  }
  return ret;
}

int fputc(int c, HANDLE stream)
{
  HANDLE hFile = (HANDLE)stream;
  DWORD tmp;
  TCHAR b = (TCHAR)c;
  WriteFile(hFile,&b,sizeof(b),&tmp,NULL);
  return c;
}
