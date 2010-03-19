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

const wchar_t FAR_VerticalBlock[] = L"FAR_VerticalBlock";
const wchar_t FAR_VerticalBlock_Unicode[] = L"FAR_VerticalBlock_Unicode";

/* ------------------------------------------------------------ */
// CF_OEMTEXT CF_TEXT CF_UNICODETEXT CF_HDROP
static HGLOBAL hInternalClipboard[5] = {0};
static UINT    uInternalClipboardFormat[5] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};

static bool OppenedClipboard = false;
static bool UseInternalClipboard = false;

//Sets UseInternalClipboard to State, and returns previous state
bool SetUseInternalClipboardState(bool State)
{
	bool OldState = UseInternalClipboard;
	UseInternalClipboard = State;
	return OldState;
}

bool GetUseInternalClipboardState()
{
	return UseInternalClipboard;
}

static UINT FAR_RegisterClipboardFormat(LPCWSTR lpszFormat)
{
	if (UseInternalClipboard)
	{
		if (!StrCmp(lpszFormat,FAR_VerticalBlock))
		{
			return 0xFEB0;
		}
		else if (!StrCmp(lpszFormat,FAR_VerticalBlock_Unicode))
		{
			return 0xFEB1;
		}

		return 0;
	}

	return RegisterClipboardFormat(lpszFormat);
}


static BOOL FAR_OpenClipboard()
{
	if (UseInternalClipboard)
	{
		if (!OppenedClipboard)
		{
			OppenedClipboard=true;
			return TRUE;
		}

		return FALSE;
	}

	return OpenClipboard(GetConsoleWindow());
}

static BOOL FAR_CloseClipboard()
{
	if (UseInternalClipboard)
	{
		if (OppenedClipboard)
		{
			OppenedClipboard=false;
			return TRUE;
		}

		return FALSE;
	}

	return CloseClipboard();
}

static BOOL FAR_EmptyClipboard()
{
	if (UseInternalClipboard)
	{
		if (OppenedClipboard)
		{
			for (size_t I=0; I < countof(hInternalClipboard); ++I)
			{
				if (hInternalClipboard[I])
				{
					GlobalFree(hInternalClipboard[I]);
					hInternalClipboard[I]=0;
					uInternalClipboardFormat[I]=0xFFFF;
				}
			}

			return TRUE;
		}

		return FALSE;
	}

	return EmptyClipboard();
}

static HANDLE FAR_GetClipboardData(UINT uFormat)
{
	if (UseInternalClipboard)
	{
		if (OppenedClipboard)
		{
			for (size_t I=0; I < countof(hInternalClipboard); ++I)
			{
				if (uInternalClipboardFormat[I] != 0xFFFF && uInternalClipboardFormat[I] == uFormat)
				{
					return hInternalClipboard[I];
				}
			}
		}

		return (HANDLE)nullptr;
	}

	return GetClipboardData(uFormat);
}

/*
static UINT FAR_EnumClipboardFormats(UINT uFormat)
{
  if(UseInternalClipboard)
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
*/

static HANDLE FAR_SetClipboardData(UINT uFormat,HANDLE hMem)
{
	if (UseInternalClipboard)
	{
		if (OppenedClipboard)
		{
			for (size_t I=0; I < countof(hInternalClipboard); ++I)
			{
				if (!hInternalClipboard[I])
				{
					hInternalClipboard[I]=hMem;
					uInternalClipboardFormat[I]=uFormat;
					return hMem;
				}
			}
		}

		return (HANDLE)nullptr;
	}

	HANDLE hData=SetClipboardData(uFormat,hMem);

	if (hData)
	{
		HANDLE hLC=GlobalAlloc(GMEM_MOVEABLE,sizeof(LCID));

		if (hLC)
		{
			PLCID pLc=(PLCID)GlobalLock(hLC);

			if (pLc)
			{
				*pLc=LOCALE_USER_DEFAULT;
				GlobalUnlock(hLC);

				if (!SetClipboardData(CF_LOCALE,pLc))
					GlobalFree(hLC);
			}
			else
			{
				GlobalFree(hLC);
			}
		}
	}

	return hData;
}

static BOOL FAR_IsClipboardFormatAvailable(UINT Format)
{
	if (UseInternalClipboard)
	{
		for (size_t I=0; I < countof(hInternalClipboard); ++I)
		{
			if (uInternalClipboardFormat[I] != 0xFFFF && uInternalClipboardFormat[I]==Format)
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	return IsClipboardFormatAvailable(Format);
}

/* ------------------------------------------------------------ */
// Перед вставкой производится очистка буфера
int WINAPI CopyToClipboard(const wchar_t *Data)
{
	if (!FAR_OpenClipboard())
		return FALSE;

	FAR_EmptyClipboard();

	if (Data && *Data)
	{
		HGLOBAL hData;
		void *GData;
		int BufferSize=(StrLength(Data)+1)*sizeof(wchar_t);

		if ((hData=GlobalAlloc(GMEM_MOVEABLE,BufferSize))!=nullptr)
		{
			if ((GData=GlobalLock(hData))!=nullptr)
			{
				memcpy(GData,Data,BufferSize);
				GlobalUnlock(hData);

				if (!FAR_SetClipboardData(CF_UNICODETEXT,(HANDLE)hData))
					GlobalFree(hData);
			}
			else
			{
				GlobalFree(hData);
			}
		}
	}

	FAR_CloseClipboard();
	return TRUE;
}


// вставка без очистки буфера - на добавление
int CopyFormatToClipboard(const wchar_t *Format,const wchar_t *Data)
{
	int FormatType=FAR_RegisterClipboardFormat(Format);

	if (FormatType==0)
		return FALSE;

	if (Data && *Data)
	{
		HGLOBAL hData;
		void *GData;

		if (!FAR_OpenClipboard())
			return FALSE;

		int BufferSize=(StrLength(Data)+1)*sizeof(wchar_t);

		if ((hData=GlobalAlloc(GMEM_MOVEABLE,BufferSize))!=nullptr)
		{
			if ((GData=GlobalLock(hData))!=nullptr)
			{
				memcpy(GData,Data,BufferSize);
				GlobalUnlock(hData);

				if (!FAR_SetClipboardData(FormatType,(HANDLE)hData))
					GlobalFree(hData);
			}
			else
			{
				GlobalFree(hData);
			}
		}

		FAR_CloseClipboard();
	}

	return TRUE;
}


wchar_t* WINAPI PasteFromClipboard()
{
	if (!FAR_OpenClipboard())
		return nullptr;

	/*
	bool Unicode=false;
	int Format=0;
	int ReadType=CF_OEMTEXT;

	while ((Format=FAR_EnumClipboardFormats(Format))!=0)
	{
	  if (Format==CF_UNICODETEXT)
	  {
	    Unicode=true;
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
	*/
	wchar_t *ClipText=nullptr;
	HANDLE hClipData=FAR_GetClipboardData(/*Unicode ?*/ CF_UNICODETEXT/*:ReadType*/);

	if (hClipData)
	{
		wchar_t *ClipAddr=(wchar_t *)GlobalLock(hClipData);

		if (ClipAddr)
		{
			int BufferSize;
			//if (Unicode)
			BufferSize=StrLength(ClipAddr)+1;
			//else
			//BufferSize=strlen(ClipAddr)+1;
			ClipText=(wchar_t *)xf_malloc(BufferSize*sizeof(wchar_t));

			if (ClipText!=nullptr)
				wcscpy(ClipText, ClipAddr);

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
	return ClipText;
}


// max - без учета символа конца строки!
wchar_t *PasteFromClipboardEx(int max)
{
	if (!FAR_OpenClipboard())
		return nullptr;

	/*
	bool Unicode=false;
	int Format=0;
	int ReadType=CF_OEMTEXT;

	while ((Format=FAR_EnumClipboardFormats(Format))!=0)
	{
	  if (Format==CF_UNICODETEXT)
	  {
	    Unicode=true;
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
	*/
	wchar_t *ClipText=nullptr;
	HANDLE hClipData=FAR_GetClipboardData(/*Unicode ? */CF_UNICODETEXT/*:ReadType*/);

	if (hClipData)
	{
		wchar_t *ClipAddr=(wchar_t *)GlobalLock(hClipData);

		if (ClipAddr)
		{
			int BufferSize;
			//if (Unicode)
			BufferSize=StrLength(ClipAddr)+1;

			//else
			//BufferSize=strlen(ClipAddr)+1;
			if (BufferSize>max)
				BufferSize=max;

			ClipText=(wchar_t *)xf_malloc((BufferSize+1)*sizeof(wchar_t));

			if (ClipText!=nullptr)
			{
				wmemset(ClipText,0,BufferSize+1);
				xwcsncpy(ClipText,ClipAddr,BufferSize+1);
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
	return ClipText;
}

wchar_t* PasteFormatFromClipboard(const wchar_t *Format)
{
	bool isOEMVBlock=false;
	UINT FormatType=FAR_RegisterClipboardFormat(Format);

	if (FormatType==0)
		return nullptr;

	if (!StrCmp(Format,FAR_VerticalBlock_Unicode) && !FAR_IsClipboardFormatAvailable(FormatType))
	{
		FormatType=FAR_RegisterClipboardFormat(FAR_VerticalBlock);
		isOEMVBlock=true;
	}

	if (FormatType==0 || !FAR_IsClipboardFormatAvailable(FormatType))
		return nullptr;

	if (!FAR_OpenClipboard())
		return nullptr;

	wchar_t *ClipText=nullptr;
	HANDLE hClipData=FAR_GetClipboardData(FormatType);

	if (hClipData)
	{
		wchar_t *ClipAddr=(wchar_t *)GlobalLock(hClipData);

		if (ClipAddr)
		{
			size_t BufferSize;

			if (isOEMVBlock)
				BufferSize=strlen((LPCSTR)ClipAddr)+1;
			else
				BufferSize=wcslen(ClipAddr)+1;

			ClipText=(wchar_t *)xf_malloc(BufferSize*sizeof(wchar_t));

			if (ClipText!=nullptr)
			{
				if (isOEMVBlock)
					MultiByteToWideChar(CP_OEMCP,0,(LPCSTR)ClipAddr,-1,ClipText,(int)BufferSize);
				else
					wcscpy(ClipText,ClipAddr);
			}

			GlobalUnlock(hClipData);
		}
	}

	FAR_CloseClipboard();
	return ClipText;
}

BOOL EmptyInternalClipboard()
{
	bool OldState = SetUseInternalClipboardState(true);

	BOOL ret = FAR_EmptyClipboard();

	SetUseInternalClipboardState(OldState);

	return ret;
}
