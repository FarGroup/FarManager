/*
clipboard.cpp

Работа с буфером обмена.
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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
#include "console.hpp"

const wchar_t FAR_VerticalBlock[] = L"FAR_VerticalBlock";
const wchar_t FAR_VerticalBlock_Unicode[] = L"FAR_VerticalBlock_Unicode";

/* ------------------------------------------------------------ */
// CF_OEMTEXT CF_TEXT CF_UNICODETEXT CF_HDROP
HGLOBAL Clipboard::hInternalClipboard[COUNT_INTERNAL_CLIPBOARD] = {};
UINT    Clipboard::uInternalClipboardFormat[COUNT_INTERNAL_CLIPBOARD] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};

bool Clipboard::UseInternalClipboard = false;
bool Clipboard::InternalClipboardOpen = false;

//Sets UseInternalClipboard to State, and returns previous state
bool Clipboard::SetUseInternalClipboardState(bool State)
{
	bool OldState = UseInternalClipboard;
	UseInternalClipboard = State;
	return OldState;
}

bool Clipboard::GetUseInternalClipboardState()
{
	return UseInternalClipboard;
}

UINT Clipboard::RegisterFormat(LPCWSTR lpszFormat)
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


BOOL Clipboard::Open()
{
	if (UseInternalClipboard)
	{
		if (!InternalClipboardOpen)
		{
			InternalClipboardOpen=true;
			return TRUE;
		}

		return FALSE;
	}

	return OpenClipboard(Console.GetWindow());
}

BOOL Clipboard::Close()
{
	if (UseInternalClipboard)
	{
		if (InternalClipboardOpen)
		{
			InternalClipboardOpen=false;
			return TRUE;
		}

		return FALSE;
	}

	return CloseClipboard();
}

BOOL Clipboard::Empty()
{
	if (UseInternalClipboard)
	{
		if (InternalClipboardOpen)
		{
			for (size_t I=0; I < ARRAYSIZE(hInternalClipboard); ++I)
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

HANDLE Clipboard::GetData(UINT uFormat)
{
	if (UseInternalClipboard)
	{
		if (InternalClipboardOpen)
		{
			for (size_t I=0; I < ARRAYSIZE(hInternalClipboard); ++I)
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
UINT Clipboard::EnumFormats(UINT uFormat)
{
  if(UseInternalClipboard)
  {
    if(InternalClipboardOpen)
    {
      for(size_t I=0; I < ARRAYSIZE(hInternalClipboard); ++I)
      {
        if(uInternalClipboardFormat[I] xFFFF && uInternalClipboardFormat[I] == uFormat)
        {
          return I+1 < ARRAYSIZE(hInternalClipboard)?uInternalClipboardFormat[I+1]:0;
        }
      }
    }
    return 0;
  }
  return EnumClipboardFormats(uFormat);
}
*/

HANDLE Clipboard::SetData(UINT uFormat,HANDLE hMem)
{
	if (UseInternalClipboard)
	{
		if (InternalClipboardOpen)
		{
			for (size_t I=0; I < ARRAYSIZE(hInternalClipboard); ++I)
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

BOOL Clipboard::IsFormatAvailable(UINT Format)
{
	if (UseInternalClipboard)
	{
		for (size_t I=0; I < ARRAYSIZE(hInternalClipboard); ++I)
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

// Перед вставкой производится очистка буфера
bool Clipboard::Copy(const wchar_t *Data)
{
	Empty();
	if (Data && *Data)
	{
		HGLOBAL hData;
		void *GData;
		size_t BufferSize=(StrLength(Data)+1)*sizeof(wchar_t);

		if ((hData=GlobalAlloc(GMEM_MOVEABLE,BufferSize)))
		{
			if ((GData=GlobalLock(hData)))
			{
				memcpy(GData,Data,BufferSize);
				GlobalUnlock(hData);
				if (!SetData(CF_UNICODETEXT,(HANDLE)hData))
					GlobalFree(hData);
			}
			else
			{
				GlobalFree(hData);
			}
		}
	}

	return true;
}

// вставка без очистки буфера - на добавление
bool Clipboard::CopyFormat(const wchar_t *Format, const wchar_t *Data)
{
	UINT FormatType=RegisterFormat(Format);

	if (!FormatType)
		return false;

	if (Data && *Data)
	{
		HGLOBAL hData;
		void *GData;

		size_t BufferSize=(StrLength(Data)+1)*sizeof(wchar_t);

		if ((hData=GlobalAlloc(GMEM_MOVEABLE,BufferSize)))
		{
			if ((GData=GlobalLock(hData)))
			{
				memcpy(GData,Data,BufferSize);
				GlobalUnlock(hData);

				if (!SetData(FormatType, hData))
					GlobalFree(hData);
			}
			else
			{
				GlobalFree(hData);
			}
		}
	}

	return true;
}

bool Clipboard::CopyHDROP(LPVOID NamesArray, size_t NamesArraySize)
{
	bool Result=false;
	if (NamesArray && NamesArraySize)
	{
		HGLOBAL hMemory=GlobalAlloc(GMEM_MOVEABLE, sizeof(DROPFILES)+NamesArraySize);
		if (hMemory)
		{
			LPDROPFILES Drop = static_cast<LPDROPFILES>(GlobalLock(hMemory));
			if(Drop)
			{
				Drop->pFiles=sizeof(DROPFILES);
				Drop->pt.x=0;
				Drop->pt.y=0;
				Drop->fNC = TRUE;
				Drop->fWide = TRUE;
				memcpy(Drop+1,NamesArray,NamesArraySize);
				GlobalUnlock(hMemory);
				Empty();
				if(SetData(CF_HDROP, hMemory))
				{
					Result = true;
				}
				else
				{
					GlobalFree(hMemory);
				}
			}
			else
			{
				GlobalFree(hMemory);
			}
		}
	}
	return Result;
}

wchar_t *Clipboard::Paste()
{
	wchar_t *ClipText=nullptr;
	HANDLE hClipData=GetData(CF_UNICODETEXT);

	if (hClipData)
	{
		wchar_t *ClipAddr=(wchar_t *)GlobalLock(hClipData);

		if (ClipAddr)
		{
			int BufferSize;
			BufferSize=StrLength(ClipAddr)+1;
			ClipText=(wchar_t *)xf_malloc(BufferSize*sizeof(wchar_t));

			if (ClipText)
				wcscpy(ClipText, ClipAddr);

			GlobalUnlock(hClipData);
		}
	}
	else
	{
		hClipData=GetData(CF_HDROP);
		if (hClipData)
		{
			LPDROPFILES Files=static_cast<LPDROPFILES>(GlobalLock(hClipData));
			if (Files)
			{
				LPCSTR StartA=reinterpret_cast<LPCSTR>(Files)+Files->pFiles;
				LPCWSTR Start=reinterpret_cast<LPCWSTR>(StartA);
				string strClipText;
				if(Files->fWide)
				{
					while(*Start)
					{
						size_t l1=strClipText.GetLength();
						strClipText+=Start;
						Start+=strClipText.GetLength()-l1;
						Start++;
						if(*Start)
						{
							strClipText+=L"\r\n";
						}
					}
				}
				else
				{
					while(*StartA)
					{
						size_t l1=strClipText.GetLength();
						strClipText+=StartA;
						StartA+=strClipText.GetLength()-l1;
						StartA++;
						if(*StartA)
						{
							strClipText+=L"\r\n";
						}
					}
				}
				if(!strClipText.IsEmpty())
				{
					ClipText=static_cast<LPWSTR>(xf_malloc((strClipText.GetLength()+1)*sizeof(WCHAR)));
					wcscpy(ClipText, strClipText);
				}
				GlobalUnlock(hClipData);
			}
		}
	}
	return ClipText;
}

// max - без учета символа конца строки!
wchar_t *Clipboard::PasteEx(int max)
{
	wchar_t *ClipText=nullptr;
	HANDLE hClipData=GetData(CF_UNICODETEXT);

	if (hClipData)
	{
		wchar_t *ClipAddr=(wchar_t *)GlobalLock(hClipData);

		if (ClipAddr)
		{
			int BufferSize;
			BufferSize=StrLength(ClipAddr);

			if (BufferSize>max)
				BufferSize=max;

			ClipText=(wchar_t *)xf_malloc((BufferSize+1)*sizeof(wchar_t));

			if (ClipText)
			{
				wmemset(ClipText,0,BufferSize+1);
				xwcsncpy(ClipText,ClipAddr,BufferSize+1);
			}

			GlobalUnlock(hClipData);
		}
	}

	return ClipText;
}

wchar_t *Clipboard::PasteFormat(const wchar_t *Format)
{
	bool isOEMVBlock=false;
	UINT FormatType=RegisterFormat(Format);

	if (!FormatType)
		return nullptr;

	if (!StrCmp(Format,FAR_VerticalBlock_Unicode) && !IsFormatAvailable(FormatType))
	{
		FormatType=RegisterFormat(FAR_VerticalBlock);
		isOEMVBlock=true;
	}

	if (!FormatType || !IsFormatAvailable(FormatType))
		return nullptr;

	wchar_t *ClipText=nullptr;
	HANDLE hClipData=GetData(FormatType);

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

			if (ClipText)
			{
				if (isOEMVBlock)
					MultiByteToWideChar(CP_OEMCP,0,(LPCSTR)ClipAddr,-1,ClipText,(int)BufferSize);
				else
					wcscpy(ClipText,ClipAddr);
			}

			GlobalUnlock(hClipData);
		}
	}

	return ClipText;
}

bool Clipboard::InternalCopy(bool FromWin)
{
	bool Ret=false;
	bool OldUseInternalClipboard=SetUseInternalClipboardState(FromWin?false:true);

	UINT uFormat;
	HANDLE hClipData=GetData(uFormat = CF_UNICODETEXT);

	if (!hClipData)
		hClipData = GetData(uFormat = CF_HDROP);

	if (hClipData)
	{
		SetUseInternalClipboardState(!Clipboard::GetUseInternalClipboardState());
		SetData(uFormat,hClipData);
		Ret=true;
	}

	SetUseInternalClipboardState(OldUseInternalClipboard);

	return Ret;
}

/* ------------------------------------------------------------ */
int CopyToClipboard(const wchar_t *Data)
{
	Clipboard clip;

	if (!clip.Open())
		return FALSE;

	BOOL ret = clip.Copy(Data);

	clip.Close();

	return ret;
}

int CopyFormatToClipboard(const wchar_t *Format,const wchar_t *Data)
{
	Clipboard clip;

	if (!clip.Open())
		return FALSE;

	BOOL ret = clip.CopyFormat(Format,Data);

	clip.Close();

	return ret;
}

wchar_t * PasteFromClipboard()
{
	Clipboard clip;

	if (!clip.Open())
		return nullptr;

	wchar_t *ClipText = clip.Paste();

	clip.Close();

	return ClipText;
}

// max - без учета символа конца строки!
wchar_t *PasteFromClipboardEx(int max)
{
	Clipboard clip;

	if (!clip.Open())
		return nullptr;

	wchar_t *ClipText = clip.PasteEx(max);

	clip.Close();

	return ClipText;
}

wchar_t *PasteFormatFromClipboard(const wchar_t *Format)
{
	Clipboard clip;

	if (!clip.Open())
		return nullptr;

	wchar_t *ClipText = clip.PasteFormat(Format);

	clip.Close();

	return ClipText;
}

BOOL EmptyInternalClipboard()
{
	bool OldState = Clipboard::SetUseInternalClipboardState(true);

	Clipboard clip;

	if (!clip.Open())
		return FALSE;

	BOOL ret = clip.Empty();

	clip.Close();

	Clipboard::SetUseInternalClipboardState(OldState);

	return ret;
}
