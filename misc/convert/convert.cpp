/*
Copyright (c) 2009 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <windows.h>
#include <stdio.h>

bool ReadFromFile(LPWSTR InFile, LPVOID *Buffer, DWORD *sz)
{
  bool ret=false;
  *Buffer=NULL;
  *sz=0;
  HANDLE In=CreateFileW(InFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  if (In!=INVALID_HANDLE_VALUE)
  {
    DWORD fsz=GetFileSize(In,NULL);
    *Buffer=HeapAlloc(GetProcessHeap(),0,fsz);
    if (*Buffer)
    {
      DWORD readsz;
      if (ReadFile(In,*Buffer,fsz,sz,NULL) && fsz==*sz)
      {
        ret=true;
      }
      else
      {
        HeapFree(GetProcessHeap(),0,*Buffer);
        *Buffer=NULL;
        *sz=0;
      }
    }
    CloseHandle(In);
  }
  return ret;
}

bool WriteToFile(LPWSTR OutFile, LPVOID Buffer, DWORD sz, bool bin)
{
  bool ret=false;
  HANDLE Out=CreateFileW(OutFile,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
  if(Out!=INVALID_HANDLE_VALUE)
  {
    ret=true;
    LPSTR ptr=(LPSTR)Buffer;
    DWORD count=0;
    if (bin && sz)
      count=sz-1;
    while (ptr+count-(LPSTR)Buffer < sz)
    {
      bool write=false, add=false;

      if (ptr+count-(LPSTR)Buffer <= sz-12)
      {
        if (!_strnicmp(ptr+count,"software\\far",12))
        {
          count+=12;
          write=true;
          add=true;
        }
        else
        {
          count++;
        }
      }
      else
      {
        count+=sz-(ptr+count-(LPSTR)Buffer);
        write=true;
      }

      if (write)
      {
        DWORD wsz;
        if (!(WriteFile(Out,ptr,count,&wsz,NULL) && wsz==count))
        {
          ret=false;
          break;
        }

        if (add)
        {
          if (!(WriteFile(Out,"2",1,&wsz,NULL) && wsz==1))
          {
            ret=false;
            break;
          }
        }

        ptr+=count;
        count=0;
      }
    }
    CloseHandle(Out);
  }
  return ret;
}


bool ConvertToUTF8(LPVOID IBuffer, DWORD isz, LPVOID *OBuffer, DWORD *osz)
{
  bool ret=false;
  *OBuffer=NULL;
  *osz=0;
  LPWSTR WideBuffer=(LPWSTR)HeapAlloc(GetProcessHeap(),0,isz*sizeof(WCHAR));
  if (WideBuffer)
  {
    DWORD wSz=MultiByteToWideChar(866,NULL,(LPCSTR)IBuffer,isz,WideBuffer,isz);
    *OBuffer=HeapAlloc(GetProcessHeap(),0,wSz*2+3);
    if (*OBuffer)
    {
      LPSTR Ptr=(LPSTR)*OBuffer;
      DWORD outSz=WideCharToMultiByte(CP_UTF8,NULL,WideBuffer,wSz,Ptr+3,wSz*2,NULL,NULL);
      Ptr[0] = '\xEF';
      Ptr[1] = '\xBB';
      Ptr[2] = '\xBF';
      *osz=outSz+3;
      ret=true;
    }
    HeapFree(GetProcessHeap(),0,WideBuffer);
  }
  return ret;
}

bool DoConvert(LPWSTR InFile, LPWSTR OutFile, bool UTF8, bool bin)
{
  bool ret=false;
  LPVOID IBuffer=NULL;
  DWORD isz=0;
  LPVOID OBuffer=NULL;
  DWORD osz=0;
  if (ReadFromFile(InFile, &IBuffer, &isz))
  {
    if (UTF8)
    {
      if (ConvertToUTF8(IBuffer, isz, &OBuffer, &osz))
      {
        if (WriteToFile(OutFile, OBuffer, osz, bin))
        {
          ret=true;
        }
        HeapFree(GetProcessHeap(),0,OBuffer);
      }
    }
    else
    {
      if (WriteToFile(OutFile, IBuffer, isz, bin))
      {
        ret=true;
      }
    }
    HeapFree(GetProcessHeap(),0,IBuffer);
  }
  return ret;
}

int main(int,char **)
{
  int ret=1;
  int argc=0;
  LPWSTR *argv=CommandLineToArgvW(GetCommandLineW(),&argc);
  if (argv && argc>2)
  {
    ret=0;
    WCHAR InFile[260];
    WCHAR OutFile[260];
    LPWSTR OutNamePtr = OutFile;

    wcscpy(OutFile,argv[argc-1]);
    OutNamePtr += wcslen(OutFile)-1;
    if (*OutNamePtr!=L'\\' && *OutNamePtr!=L'/')
      *(++OutNamePtr)=L'\\';
    OutNamePtr++;

    for (int i=1; i<argc-1; i++)
    {
      wcscpy(InFile,argv[i]);

      LPWSTR ptr = wcsrchr(InFile,L'\\');
      if (!ptr)
        ptr = wcsrchr(InFile,L'/');

      wcscpy(OutNamePtr,ptr?ptr+1:InFile);

      bool UTF8=false, bin=false;

      ptr = wcsrchr(InFile,L'.');
      if (ptr)
      {
        ptr++;
        if (!wcsicmp(ptr,L"lng") || !wcsicmp(ptr,L"hlf") || !wcsicmp(ptr,L"temp"))
        {
          UTF8=true;
        }
        else if (!wcsicmp(ptr,L"dll"))
        {
          bin=true;
        }
      }

      if (!DoConvert(InFile, OutFile, UTF8, bin))
      {
        ret=1;
        wprintf(L"Error converting %s to %s\n",InFile, OutFile);
      }
    }
  }
  LocalFree(argv);
  return ret;
}
