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

#include "filestr.hpp"
#include "nsUniversalDetectorEx.h"

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

bool IsTextUTF8(const LPBYTE Buffer,size_t Length)
{
  bool Ascii=true;
  UINT Octets=0;
  for(size_t i=0;i<Length;i++)
  {
    BYTE c=Buffer[i];
    if(c&0x80)
      Ascii=false;
    if(Octets)
    {
      if((c&0xC0)!=0x80)
        return false;
      Octets--;
    }
    else
    {
      if(c&0x80)
      {
        while(c&0x80)
        {
          c<<=1;
          Octets++;
        }
        Octets--;
        if(!Octets)
          return false;
      }
    }
  }
  return (Octets>0||Ascii)?false:true;
}

bool GetFileFormat (FILE *file, UINT &nCodePage, bool *pSignatureFound, bool bUseHeuristics)
{
  DWORD dwTemp=0;

  bool bSignatureFound = false;
  bool bDetect=false;

  if ( fread (&dwTemp, 1, 4, file) )
  {
    if ( LOWORD (dwTemp) == SIGN_UNICODE )
    {
      nCodePage = CP_UNICODE;
      fseek (file, 2, SEEK_SET);
      bSignatureFound = true;
    }
    else

    if ( LOWORD (dwTemp) == SIGN_REVERSEBOM )
    {
      nCodePage = CP_REVERSEBOM;
      fseek (file, 2, SEEK_SET);
      bSignatureFound = true;
    }
    else

    if ( (dwTemp & 0x00FFFFFF) == SIGN_UTF8 )
    {
      nCodePage = CP_UTF8;
      fseek (file, 3, SEEK_SET);
      bSignatureFound = true;
    }
    else
      fseek (file, 0, SEEK_SET);
  }

  if( bSignatureFound )
  {
    bDetect = true;
  }
  else

  if ( bUseHeuristics )
  {
    fseek (file, 0, SEEK_SET);
    size_t sz=0x8000; // BUGBUG. TODO: configurable
    LPVOID Buffer=xf_malloc(sz);
    sz=fread(Buffer,1,sz,file);
    fseek (file,0,SEEK_SET);

    if ( sz )
    {
      int test=
        IS_TEXT_UNICODE_STATISTICS|
        IS_TEXT_UNICODE_REVERSE_STATISTICS|
        IS_TEXT_UNICODE_CONTROLS|
        IS_TEXT_UNICODE_REVERSE_CONTROLS|
        IS_TEXT_UNICODE_ILLEGAL_CHARS|
        IS_TEXT_UNICODE_ODD_LENGTH|
        IS_TEXT_UNICODE_NULL_BYTES;

      if ( IsTextUnicode (Buffer, (int)sz, &test) )
      {
        if ( !(test&IS_TEXT_UNICODE_ODD_LENGTH) && !(test&IS_TEXT_UNICODE_ILLEGAL_CHARS) )
        {
          if( (test&IS_TEXT_UNICODE_NULL_BYTES) ||
            (test&IS_TEXT_UNICODE_CONTROLS) ||
            (test&IS_TEXT_UNICODE_REVERSE_CONTROLS) )
          {
            if ( (test&IS_TEXT_UNICODE_CONTROLS) || (test&IS_TEXT_UNICODE_STATISTICS) )
            {
              nCodePage=CP_UNICODE;
              bDetect=true;
            }
            else

            if ( (test&IS_TEXT_UNICODE_REVERSE_CONTROLS) || (test&IS_TEXT_UNICODE_REVERSE_STATISTICS) )
            {
              nCodePage=CP_REVERSEBOM;
              bDetect=true;
            }
          }
        }
      }
      else

      if ( IsTextUTF8 ((const LPBYTE)Buffer, sz) )
      {
        nCodePage=CP_UTF8;
        bDetect=true;
      }
      else
      {
        nsUniversalDetectorEx *ns = new nsUniversalDetectorEx();

        ns->HandleData((const char*)Buffer,(PRUint32)sz);
        ns->DataEnd();

        int cp = ns->getCodePage();

        if ( cp != -1 )
        {
          nCodePage = cp;
          bDetect = true;
        }

        delete ns;

      }
    }

    xf_free(Buffer);
  }

  if ( pSignatureFound )
    *pSignatureFound = bSignatureFound;

  return bDetect;
}

wchar_t *ReadString (FILE *file, wchar_t *lpwszDest, int nDestLength, int nCodePage)
{
    char *lpDest = (char*)xf_malloc ((nDestLength+1)*3); //UTF-8, up to 3 bytes per char support

    memset (lpDest, 0, (nDestLength+1)*3);
    memset (lpwszDest, 0, nDestLength*sizeof (wchar_t));

    if ( (nCodePage == CP_UNICODE) || (nCodePage == CP_REVERSEBOM) )
    {
        if ( !fgetws (lpwszDest, nDestLength, file) )
        {
            xf_free (lpDest);
            return NULL;
        }

        if ( nCodePage == CP_REVERSEBOM )
        {
            swab ((char*)lpwszDest, (char*)lpwszDest, nDestLength*sizeof (wchar_t));

            wchar_t *Ch = lpwszDest;
            int nLength = Min (static_cast<int>(wcslen (lpwszDest)), nDestLength);

            while ( *Ch )
            {
                if ( *Ch == L'\n' )
                {
                    *(Ch+1) = 0;
                    break;
                }

                Ch++;
            }

            int nNewLength = Min (static_cast<int>(wcslen (lpwszDest)), nDestLength);

            fseek (file, (nNewLength-nLength)*sizeof (wchar_t), SEEK_CUR);
        }

    }
    else

    if ( nCodePage == CP_UTF8 )
    {
        if ( fgets (lpDest, nDestLength*3, file) )
            MultiByteToWideChar (CP_UTF8, 0, lpDest, -1, lpwszDest, nDestLength);
        else
        {
            xf_free (lpDest);
            return NULL;
        }

    }
    else

    if ( nCodePage != -1 )
    {
        if ( fgets (lpDest, nDestLength, file) )
            MultiByteToWideChar (nCodePage, 0, lpDest, -1, lpwszDest, nDestLength);
        else
        {
            xf_free (lpDest);
            return NULL;
        }
    }

    xf_free (lpDest);

    return lpwszDest;
}
