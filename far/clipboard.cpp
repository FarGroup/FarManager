/*
clipboard.cpp

Работа с буфером обмена.
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

#include "global.hpp"
#include "fn.hpp"

/* ------------------------------------------------------------ */
// CF_OEMTEXT CF_TEXT CF_UNICODETEXT CF_HDROP
static HGLOBAL hInternalClipboard[5]={0};
static UINT    uInternalClipboardFormat[5]={0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};
static BOOL    OppenedClipboard=FALSE;


static UINT WINAPI FAR_RegisterClipboardFormat(LPCWSTR lpszFormat)
{
  if(UsedInternalClipboard)
  {
    if(!StrCmp(lpszFormat,FAR_VerticalBlock))
    {
      return 0xFEB0;
    }
    return 0;
  }
  return RegisterClipboardFormatW(lpszFormat);
}


static BOOL WINAPI FAR_OpenClipboard(HWND hWndNewOwner)
{
  if(UsedInternalClipboard)
  {
    if(!OppenedClipboard)
    {
      OppenedClipboard=TRUE;
      return TRUE;
    }
    return FALSE;
  }
  return OpenClipboard(hWndNewOwner);
}

static BOOL WINAPI FAR_CloseClipboard(VOID)
{
  if(UsedInternalClipboard)
  {
    if(OppenedClipboard)
    {
      OppenedClipboard=FALSE;
      return TRUE;
    }
    return FALSE;
  }
  return CloseClipboard();
}

BOOL WINAPI FAR_EmptyClipboard(VOID)
{
  if(UsedInternalClipboard)
  {
    if(OppenedClipboard)
    {
      for(size_t I=0; I < countof(hInternalClipboard); ++I)
        if(hInternalClipboard[I])
        {
          GlobalFree(hInternalClipboard[I]);
          hInternalClipboard[I]=0;
          uInternalClipboardFormat[I]=0xFFFF;
        }
      return TRUE;
    }
    return FALSE;
  }
  return EmptyClipboard();
}

static HANDLE WINAPI FAR_GetClipboardData(UINT uFormat)
{
  if(UsedInternalClipboard)
  {
    if(OppenedClipboard)
    {
      for(size_t I=0; I < countof(hInternalClipboard); ++I)
      {
        if(uInternalClipboardFormat[I] != 0xFFFF && uInternalClipboardFormat[I] == uFormat)
        {
          return hInternalClipboard[I];
        }
      }
    }
    return (HANDLE)NULL;
  }
  return GetClipboardData(uFormat);
}

static UINT WINAPI FAR_EnumClipboardFormats(UINT uFormat)
{
  if(UsedInternalClipboard)
  {
    if(OppenedClipboard)
    {
      for(size_t I=0; I < countof(hInternalClipboard); ++I)
      {
        if(uInternalClipboardFormat[I] != 0xFFFF && uInternalClipboardFormat[I] == uFormat)
        {
          return I+1 < countof(hInternalClipboard)?uInternalClipboardFormat[I+1]:0;
        }
      }
    }
    return 0;
  }
  return EnumClipboardFormats(uFormat);
}

static HANDLE WINAPI FAR_SetClipboardData(UINT uFormat,HANDLE hMem)
{
  if(UsedInternalClipboard)
  {
    if(OppenedClipboard)
    {
      for(size_t I=0; I < countof(hInternalClipboard); ++I)
      {
        if(!hInternalClipboard[I])
        {
          hInternalClipboard[I]=hMem;
          uInternalClipboardFormat[I]=uFormat;
          return hMem;
        }
      }
    }
    return (HANDLE)NULL;
  }
  return SetClipboardData(uFormat,hMem);
}


/* ------------------------------------------------------------ */

int WINAPI CopyToClipboard(const wchar_t *Data)
{
  return InternalCopyToClipboard(Data,0);
}

int InternalCopyToClipboard(const wchar_t *Data,int AnsiMode) //AnsiMode - fake
{
  long DataSize;
  if (!FAR_OpenClipboard(NULL))
    return(FALSE);
  FAR_EmptyClipboard();
  if (Data!=NULL && (DataSize=(long)StrLength(Data))!=0)
  {
    HGLOBAL hData;
    void *GData;
    int BufferSize=(DataSize+1)*sizeof (wchar_t);
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,BufferSize);
        GlobalUnlock(hData);
        FAR_SetClipboardData(CF_UNICODETEXT,(HANDLE)hData);
      }
  }
  FAR_CloseClipboard();
  return(TRUE);
}


int CopyFormatToClipboard(const wchar_t *Format,const wchar_t *Data)
{
  int FormatType=FAR_RegisterClipboardFormat(Format);
  if (FormatType==0)
    return(FALSE);

  long DataSize;
  if (Data!=NULL && (DataSize=(long)StrLength(Data))!=0)
  {
    HGLOBAL hData;
    void *GData;
    if (!FAR_OpenClipboard(NULL))
      return(FALSE);
    int BufferSize=(DataSize+1)*sizeof (wchar_t);
    if ((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,BufferSize))!=NULL)
      if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,BufferSize);
        GlobalUnlock(hData);
        FAR_SetClipboardData(FormatType,(HANDLE)hData);
      }
    FAR_CloseClipboard();
  }
  return(TRUE);
}


wchar_t* WINAPI PasteFromClipboard(void)
{
  return InternalPasteFromClipboard(0);
}

wchar_t* InternalPasteFromClipboard(int AnsiMode) //AnsiMode - fake!!
{
  HANDLE hClipData;
  if (!FAR_OpenClipboard(NULL))
    return(NULL);
  int Unicode=FALSE;
  int Format=0;
  int ReadType=CF_OEMTEXT;
  while ((Format=FAR_EnumClipboardFormats(Format))!=0)
  {
    if (Format==CF_UNICODETEXT && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      Unicode=TRUE;
      break;
    }
    if (Format==CF_TEXT)
    {
      ReadType=CF_TEXT;
      break;
    }
    if (Format==CF_OEMTEXT)
      break;
  }
  wchar_t *ClipText=NULL;
  if ((hClipData=FAR_GetClipboardData(/*Unicode ?*/ CF_UNICODETEXT/*:ReadType*/))!=NULL)
  {
    int BufferSize;
    wchar_t *ClipAddr=(wchar_t *)GlobalLock(hClipData);
//    if (Unicode)
      BufferSize=StrLength(ClipAddr)+1;
//    else
//      BufferSize=strlen(ClipAddr)+1;

    ClipText=(wchar_t *)xf_malloc(BufferSize*sizeof(wchar_t));
    if (ClipText!=NULL)
        wcscpy (ClipText, ClipAddr);
/*      if (Unicode)
      {
        if(AnsiMode)
          UnicodeToANSI((LPCWSTR)ClipAddr,ClipText,BufferSize);
        else
          UnicodeToOEM((LPCWSTR)ClipAddr,ClipText,BufferSize);
      }
      else
        if (ReadType==CF_TEXT)
        {
          if(!AnsiMode)
            FAR_CharToOem(ClipAddr,ClipText);
        }
        else
          strcpy(ClipText,ClipAddr);*/
    GlobalUnlock(hClipData);
  }
  FAR_CloseClipboard();
  return(ClipText);
}


wchar_t* WINAPI PasteFromClipboardEx(int max)
{
  return InternalPasteFromClipboardEx(max,0);
}


// max - без учета символа конца строки!
wchar_t* InternalPasteFromClipboardEx(int max,int AnsiMode) //AnsiMode - fake
{
  wchar_t *ClipText=NULL;
  HANDLE hClipData;
  int Unicode=FALSE;
  int Format=0;
  int ReadType=CF_OEMTEXT;

  if (!FAR_OpenClipboard(NULL))
    return(NULL);

  while ((Format=FAR_EnumClipboardFormats(Format))!=0)
  {
    if (Format==CF_UNICODETEXT && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      Unicode=TRUE;
      break;
    }
    if (Format==CF_TEXT)
    {
      ReadType=CF_TEXT;
      break;
    }
    if (Format==CF_OEMTEXT)
      break;
  }

  if ((hClipData=FAR_GetClipboardData(/*Unicode ? */CF_UNICODETEXT/*:ReadType*/))!=NULL)
  {
    int BufferSize;
    wchar_t *ClipAddr=(wchar_t *)GlobalLock(hClipData);
//    if (Unicode)
      BufferSize=StrLength(ClipAddr)+1;
 //   else
//      BufferSize=strlen(ClipAddr)+1;
    if ( BufferSize>max )
        BufferSize=max;

    ClipText=(wchar_t *)xf_malloc((BufferSize+2)*sizeof(wchar_t));
    if (ClipText!=NULL)
    {
      wmemset(ClipText,0,BufferSize+2);

      xwcsncpy(ClipText,ClipAddr,BufferSize);

/*      if (Unicode)
        if(AnsiMode)
          UnicodeToANSI((LPCWSTR)ClipAddr,ClipText,BufferSize);
        else
          UnicodeToOEM((LPCWSTR)ClipAddr,ClipText,BufferSize);
      else
      {
        if (ReadType==CF_TEXT)
        {
          xstrncpy(ClipText,ClipAddr,BufferSize);
          if(!AnsiMode)
            FAR_CharToOem(ClipText,ClipText);
          ClipText[BufferSize]=0;
        }
        else
        {
          xstrncpy(ClipText,ClipAddr,BufferSize);
          ClipText[BufferSize]=0;
        }
      }    */
    }
    GlobalUnlock(hClipData);
  }
  FAR_CloseClipboard();
  return(ClipText);
}


wchar_t* PasteFormatFromClipboard(const wchar_t *Format)
{
  int FormatType=FAR_RegisterClipboardFormat(Format);
  if (FormatType==0)
    return(NULL);
  if (!FAR_OpenClipboard(NULL))
    return(NULL);
  HANDLE hClipData;
  wchar_t *ClipText=NULL;
  if ((hClipData=FAR_GetClipboardData(FormatType))!=NULL)
  {
    wchar_t *ClipAddr=(wchar_t *)GlobalLock(hClipData);
    int BufferSize=StrLength(ClipAddr)+1;
    ClipText=(wchar_t *)xf_malloc(BufferSize*sizeof(wchar_t));
    if (ClipText!=NULL)
      wcscpy(ClipText,ClipAddr);
    GlobalUnlock(hClipData);
  }
  FAR_CloseClipboard();
  return(ClipText);
}
