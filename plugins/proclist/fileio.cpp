#include "Proclist.hpp"

int fprintf(HANDLE stream, const char *format, ...)
{
  int ret=0;
  HANDLE hFile = (HANDLE)stream;
  static char buffer[64*1024];
  DWORD tmp;
  if(format)
  {
    va_list argptr;
    va_start(argptr,format);
    ret=wvsprintf(buffer,format,argptr);
    va_end(argptr);
    if (WriteFile(hFile,buffer,ret,&tmp,NULL))
      ret=tmp;
  }
  return ret;
}

int fputc(int c, HANDLE stream)
{
  HANDLE hFile = (HANDLE)stream;
  DWORD tmp;
  char buf[2];
  buf[0] = c;
  buf[1] = 0;
  WriteFile(hFile,buf,1,&tmp,NULL);
  return c;
}
