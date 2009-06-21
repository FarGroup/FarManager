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

#include "clipboard.hpp"

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
    else if(!StrCmp(lpszFormat,FAR_VerticalBlock_Unicode))
    {
      return 0xFEB1;
    }
    return 0;
  }
	return RegisterClipboardFormat(lpszFormat);
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

	HANDLE hData=SetClipboardData(uFormat,hMem);
	if(hData)
	{
		HANDLE hLC=GlobalAlloc(GMEM_MOVEABLE,sizeof(LCID));
		if(hLC)
		{
			PLCID pLc=(PLCID)GlobalLock(hLC);
			if(pLc)
			{
				*pLc=LOCALE_USER_DEFAULT;
				GlobalUnlock(hLC);
				if(!SetClipboardData(CF_LOCALE,pLc))
					GlobalFree(hLC);
			}
			else
				GlobalFree(hLC);
		}
	}
	return hData;
}


/* ------------------------------------------------------------ */
// Перед вставкой производится очистка буфера
int WINAPI CopyToClipboard(const wchar_t *Data)
{
  if (!FAR_OpenClipboard(NULL))
    return(FALSE);
  FAR_EmptyClipboard();
  FAR_CloseClipboard();
  if(Data && *Data)
    return InternalCopyToClipboard(Data,0);
  else
    return TRUE;
}

int InternalCopyToClipboard(const wchar_t *Data,int AnsiMode) //AnsiMode - fake
{
  long DataSize;
  if (!FAR_OpenClipboard(NULL))
    return(FALSE);
  if (Data!=NULL && (DataSize=(long)StrLength(Data))!=0)
  {
    HGLOBAL hData;
    void *GData;
    int BufferSize=(DataSize+1)*sizeof (wchar_t);
    if ((hData=GlobalAlloc(GMEM_MOVEABLE,BufferSize))!=NULL)
		{
			if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,BufferSize);
        GlobalUnlock(hData);
				if(!FAR_SetClipboardData(CF_UNICODETEXT,(HANDLE)hData))
					GlobalFree(hData);
      }
			else
					GlobalFree(hData);
		}
  }
  FAR_CloseClipboard();
  return(TRUE);
}


// вставка без очистки буфера - на добавление
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
    if ((hData=GlobalAlloc(GMEM_MOVEABLE,BufferSize))!=NULL)
		{
			if ((GData=GlobalLock(hData))!=NULL)
      {
        memcpy(GData,Data,BufferSize);
        GlobalUnlock(hData);
				if(!FAR_SetClipboardData(FormatType,(HANDLE)hData))
					GlobalFree(hData);
      }
			else
				GlobalFree(hData);
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
    if (Format==CF_UNICODETEXT)
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
    if (ClipAddr)
    {
      //if (Unicode)
        BufferSize=StrLength(ClipAddr)+1;
      //else
        //BufferSize=strlen(ClipAddr)+1;

      ClipText=(wchar_t *)xf_malloc(BufferSize*sizeof(wchar_t));
      if (ClipText!=NULL)
        wcscpy (ClipText, ClipAddr);
/*
        if (Unicode)
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
              CharToOemA(ClipAddr,ClipText);
          }
          else
            strcpy(ClipText,ClipAddr);
*/
      GlobalUnlock(hClipData);
    }
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
    if (Format==CF_UNICODETEXT)
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
    if (ClipAddr)
    {
      //if (Unicode)
        BufferSize=StrLength(ClipAddr)+1;
      //else
       //BufferSize=strlen(ClipAddr)+1;
      if ( BufferSize>max )
        BufferSize=max;

      ClipText=(wchar_t *)xf_malloc((BufferSize+2)*sizeof(wchar_t));
      if (ClipText!=NULL)
      {
        wmemset(ClipText,0,BufferSize+2);

        xwcsncpy(ClipText,ClipAddr,BufferSize);

/*
        if (Unicode)
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
              CharToOemA(ClipText,ClipText);
            ClipText[BufferSize]=0;
          }
          else
          {
            xstrncpy(ClipText,ClipAddr,BufferSize);
            ClipText[BufferSize]=0;
          }
        }
*/
      }
      GlobalUnlock(hClipData);
    }
  }
  FAR_CloseClipboard();
  return(ClipText);
}

static BOOL FAR_IsClipboardFormatAvailable(UINT Format)
{
	if(UsedInternalClipboard)
	{
		for(size_t I=0; I < countof(hInternalClipboard); ++I)
		{
			if(uInternalClipboardFormat[I] != 0xFFFF && uInternalClipboardFormat[I]==Format)
			{
				return TRUE;
			}
		}
		return FALSE;
	}
	return IsClipboardFormatAvailable(Format);
}

wchar_t* PasteFormatFromClipboard(const wchar_t *Format)
{
  bool isOEMVBlock=false;
  UINT FormatType=FAR_RegisterClipboardFormat(Format);
  if (FormatType==0)
    return(NULL);

  if(!StrCmp(Format,FAR_VerticalBlock_Unicode) && !FAR_IsClipboardFormatAvailable(FormatType))
	{
		FormatType=FAR_RegisterClipboardFormat(FAR_VerticalBlock);
		isOEMVBlock=true;
	}

	if (FormatType==0 || !FAR_IsClipboardFormatAvailable(FormatType))
		return NULL;

  if (!FAR_OpenClipboard(NULL))
    return(NULL);

  HANDLE hClipData;
  wchar_t *ClipText=NULL;

  if ((hClipData=FAR_GetClipboardData(FormatType))!=NULL)
  {
    wchar_t *ClipAddr=(wchar_t *)GlobalLock(hClipData);
    if (ClipAddr)
    {
      size_t BufferSize;
      if(isOEMVBlock)
				BufferSize=strlen((LPCSTR)ClipAddr)+1;
			else
				BufferSize=wcslen(ClipAddr)+1;

      ClipText=(wchar_t *)xf_malloc(BufferSize*sizeof(wchar_t));
      if (ClipText!=NULL)
			{
				if(isOEMVBlock)
					MultiByteToWideChar(CP_OEMCP,0,(LPCSTR)ClipAddr,-1,ClipText,(int)BufferSize);
				else
					wcscpy(ClipText,ClipAddr);
			}
      GlobalUnlock(hClipData);
    }
  }

  FAR_CloseClipboard();

  return(ClipText);
}
