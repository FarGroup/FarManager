/*
filestr.cpp

Класс GetFileString

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
}


GetFileString::~GetFileString()
{
  if ( Str ) xf_free(Str);
  if ( wStr ) xf_free(wStr);
}


int GetFileString::GetStringW(wchar_t **DestStr, int nCodePage, int &Length)
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
			int nResultLength = MultiByteToWideChar (nCodePage, 0, Str, Length, NULL, 0);

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
  Length=0;
  int CurLength=0,ExitCode=1;
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
    if (Ch!='\n' && CurLength>0 && Str[CurLength-1]=='\r')
      break;
    ReadPos++;
    if (CurLength>=m_nStrLength-1)
    {
      char *NewStr=(char *)xf_realloc(Str,m_nStrLength+1024);
      if (NewStr==NULL)
        return(-1);
      Str=NewStr;
      m_nStrLength+=1024;
    }
    Str[CurLength++]=Ch;
    if (Ch=='\n')
      break;
  }
  Str[CurLength]=0;
  *DestStr=Str;
  Length=CurLength;
  return(ExitCode);
}

int GetFileString::GetUnicodeString(wchar_t **DestStr,int &Length)
{
  Length=0;
  int CurLength=0,ExitCode=1;
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
    if (Ch!=L'\n' && CurLength>0 && wStr[CurLength-1]==L'\r')
      break;
    ReadPos += sizeof (wchar_t);
    if (CurLength>=m_nStrLength-1)
    {
      wchar_t *NewStr=(wchar_t *)xf_realloc(wStr,(m_nStrLength+1024)*sizeof (wchar_t));
      if (NewStr==NULL)
        return(-1);
      wStr=NewStr;
      m_nStrLength+=1024;
    }
    wStr[CurLength++]=Ch;
    if (Ch==L'\n')
      break;
  }
  wStr[CurLength]=0;
  *DestStr=wStr;
  Length=CurLength;
  return(ExitCode);
}

int GetFileString::GetReverseUnicodeString(wchar_t **DestStr,int &Length)
{
	Length=0;
	int CurLength=0,ExitCode=1;
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
		if (Ch!=L'\n' && CurLength>0 && wStr[CurLength-1]==L'\r')
			break;
		ReadPos += sizeof (wchar_t);
		if (CurLength>=m_nStrLength-1)
		{
			wchar_t *NewStr=(wchar_t *)xf_realloc(wStr,(m_nStrLength+1024)*sizeof (wchar_t));
			if (NewStr==NULL)
				return(-1);
			wStr=NewStr;
			m_nStrLength+=1024;
		}
		wStr[CurLength++]=Ch;
		if (Ch==L'\n')
			break;
	}
	wStr[CurLength]=0;
	*DestStr=wStr;
	Length=CurLength;
	return(ExitCode);
}
