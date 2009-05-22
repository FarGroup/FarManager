/*
filestr.cpp

Класс GetFileString
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "filestr.hpp"

GetFileString::GetFileString(FILE *SrcFile)
{
  wStr = (wchar_t*)xf_malloc(1024*sizeof (wchar_t));
  Str=(char*)xf_malloc(1024);
  m_nStrLength=1024;
  GetFileString::SrcFile=SrcFile;

  ReadPos=ReadSize=0;
	SomeDataLost=false;
}


GetFileString::~GetFileString()
{
  if ( Str ) xf_free(Str);
  if ( wStr ) xf_free(wStr);
}


int GetFileString::GetString(wchar_t **DestStr, int nCodePage, int &Length)
{
	int nExitCode;

	if ( nCodePage == CP_UNICODE ) //utf-16
	{
		nExitCode = GetUnicodeString (DestStr, Length);
	}
	else

	if ( nCodePage == CP_REVERSEBOM )
	{
		nExitCode = GetReverseUnicodeString(DestStr, Length);
	}

	else
	{
		char *Str;

		nExitCode = GetAnsiString(&Str, Length);

		if ( nExitCode == 1 )
		{
			// при CP_UTF7 dwFlags должен быть 0, см. MSDN
			int nResultLength = MultiByteToWideChar (nCodePage, (SomeDataLost||nCodePage==CP_UTF7)?0:MB_ERR_INVALID_CHARS, Str, Length, NULL, 0);
			if(!nResultLength && GetLastError()==ERROR_NO_UNICODE_TRANSLATION && !SomeDataLost)
			{
				SomeDataLost=true;
				nResultLength=MultiByteToWideChar(nCodePage, 0, Str, Length, NULL, 0);
			}
			wStr = (wchar_t*)xf_realloc (wStr, (nResultLength+1)*sizeof (wchar_t));
			wmemset (wStr, 0, nResultLength+1);

			MultiByteToWideChar (nCodePage, 0, Str, Length, wStr, nResultLength);

			Length = nResultLength;
			*DestStr = wStr;
		}
	}

	return nExitCode;
}

int GetFileString::GetAnsiString(char **DestStr,int &Length)
{
  int CurLength=0;
  int ExitCode=1;
  int Eol=0;
  char EOL[16];
  char *PtrEol=EOL;
  int x=0;

  memset(EOL,0,sizeof(EOL));
  Length=0;

  while (1)
  {
    if (ReadPos>=ReadSize)
    {
      if ((ReadSize=(int)fread(ReadBuf,1,sizeof(ReadBuf),SrcFile))==0)
      {
        if (CurLength==0)
          ExitCode=0;
        break;
      }
      ReadPos=0;
    }

    int Ch=ReadBuf[ReadPos];

    if( ( Eol && Ch != '\n' && Ch != '\r' ) || Eol >= (int)sizeof(EOL))
      break;

    if(Eol && (Ch == '\n' || Ch == '\r') && (!strcmp(EOL,WIN_EOL_fmtA) || !strcmp(EOL,DOS_EOL_fmtA) || !strcmp(EOL,UNIX_EOL_fmtA)))
      break;

    if(Ch == '\n' || Ch == '\r')
    {
      *PtrEol++=Ch;
      Eol++;
    }

    ReadPos++;

    if (CurLength>=m_nStrLength-1)
    {
      char *NewStr=(char *)xf_realloc(Str,m_nStrLength+(1024<<x));
      if (NewStr==NULL)
        return(-1);
      Str=NewStr;
      m_nStrLength+=1024<<x;
      x++;
    }
    Str[CurLength++]=Ch;
    if(Eol && (Ch == '\n' || Ch == '\r') && (!strcmp(EOL,WIN_EOL_fmtA) || !strcmp(EOL,DOS_EOL_fmtA) || !strcmp(EOL,UNIX_EOL_fmtA)))
      break;
  }
  Str[CurLength]=0;
  *DestStr=Str;
  Length=CurLength;
  return(ExitCode);
}

int GetFileString::GetUnicodeString(wchar_t **DestStr,int &Length)
{
  int CurLength=0;
  int ExitCode=1;
  int Eol=0;
  wchar_t EOL[16];
  wchar_t *PtrEol=EOL;
  int x=0;

  memset(EOL,0,sizeof(EOL));
  Length=0;

  while (1)
  {
    if (ReadPos>=ReadSize)
    {
      if ((ReadSize=(int)fread(wReadBuf,1,sizeof(wReadBuf),SrcFile))==0)
      {
        if (CurLength==0)
          ExitCode=0;
        break;
      }
      ReadPos=0;
    }
    int Ch=wReadBuf[ReadPos/sizeof (wchar_t)];

    if( ( Eol && Ch != L'\n' && Ch != L'\r' ) || Eol >= (int)(sizeof(EOL)/sizeof (wchar_t)))
      break;

    if(Eol && (Ch == L'\n' || Ch == L'\r') && (!StrCmp(EOL,WIN_EOL_fmt) || !StrCmp(EOL,DOS_EOL_fmt) || !StrCmp(EOL,UNIX_EOL_fmt)))
      break;
    if(Ch == L'\n' || Ch == L'\r')
    {
      *PtrEol++=Ch;
      Eol++;
    }

    ReadPos += sizeof (wchar_t);
    if (CurLength>=m_nStrLength-1)
    {
      wchar_t *NewStr=(wchar_t *)xf_realloc(wStr,(m_nStrLength+(1024<<x))*sizeof (wchar_t));
      if (NewStr==NULL)
        return(-1);
      wStr=NewStr;
      m_nStrLength+=1024<<x;
      x++;
    }
    wStr[CurLength++]=Ch;
    if(Eol && (Ch == L'\n' || Ch == L'\r') && (!StrCmp(EOL,WIN_EOL_fmt) || !StrCmp(EOL,DOS_EOL_fmt) || !StrCmp(EOL,UNIX_EOL_fmt)))
      break;
  }
  wStr[CurLength]=0;
  *DestStr=wStr;
  Length=CurLength;
  return(ExitCode);
}

int GetFileString::GetReverseUnicodeString(wchar_t **DestStr,int &Length)
{
	int CurLength=0;
	int ExitCode=1;
	int Eol=0;
	wchar_t EOL[16];
	wchar_t *PtrEol=EOL;
	int x=0;

	memset(EOL,0,sizeof(EOL));
	Length=0;

	while (1)
	{
		if (ReadPos>=ReadSize)
		{
			if ((ReadSize=(int)fread(wReadBuf,1,sizeof(wReadBuf),SrcFile))==0)
			{
				if (CurLength==0)
					ExitCode=0;
				break;
			}

			swab ((char*)&wReadBuf, (char*)&wReadBuf, ReadSize);
			ReadPos=0;
		}
		int Ch=wReadBuf[ReadPos/sizeof (wchar_t)];
		if( ( Eol && Ch != L'\n' && Ch != L'\r' ) || Eol >= (int)(sizeof(EOL)/sizeof (wchar_t)))
			break;

		if(Eol && (Ch == L'\n' || Ch == L'\r') && (!StrCmp(EOL,WIN_EOL_fmt) || !StrCmp(EOL,DOS_EOL_fmt) || !StrCmp(EOL,UNIX_EOL_fmt)))
			break;
		if(Ch == L'\n' || Ch == L'\r')
		{
			*PtrEol++=Ch;
			Eol++;
		}

		ReadPos += sizeof (wchar_t);
		if (CurLength>=m_nStrLength-1)
		{
			wchar_t *NewStr=(wchar_t *)xf_realloc(wStr,(m_nStrLength+(1024<<x))*sizeof (wchar_t));
			if (NewStr==NULL)
				return(-1);
			wStr=NewStr;
			m_nStrLength+=1024<<x;
			x++;
		}
		wStr[CurLength++]=Ch;
		if(Eol && (Ch == L'\n' || Ch == L'\r') && (!StrCmp(EOL,WIN_EOL_fmt) || !StrCmp(EOL,DOS_EOL_fmt) || !StrCmp(EOL,UNIX_EOL_fmt)))
			break;
	}
	wStr[CurLength]=0;
	*DestStr=wStr;
	Length=CurLength;
	return(ExitCode);
}
