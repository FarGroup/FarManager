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

struct internal_clipboard
{
	HGLOBAL Handle;
	UINT Format;
};

static std::array<internal_clipboard, 5> InternalClipboard = 
{
	// CF_OEMTEXT CF_TEXT CF_UNICODETEXT CF_HDROP
	{
		{nullptr, 0xFFFF},
		{nullptr, 0xFFFF},
		{nullptr, 0xFFFF},
		{nullptr, 0xFFFF},
		{nullptr, 0xFFFF},
	}
};


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

UINT Clipboard::RegisterFormat(FAR_CLIPBOARD_FORMAT Format)
{
	switch (Format)
	{
	case FCF_VERTICALBLOCK_OEM:
		return UseInternalClipboard? 0xFEB0 : RegisterClipboardFormat(L"FAR_VerticalBlock");
	case FCF_VERTICALBLOCK_UNICODE:
		return UseInternalClipboard? 0xFEB1 : RegisterClipboardFormat(L"FAR_VerticalBlock_Unicode");
	}
	return 0;
}

bool Clipboard::Open()
{
	if (UseInternalClipboard)
	{
		if (!InternalClipboardOpen)
		{
			InternalClipboardOpen=true;
			return true;
		}

		return false;
	}

	return OpenClipboard(Global->Console->GetWindow()) != FALSE;
}

bool Clipboard::Close()
{
	if (UseInternalClipboard)
	{
		if (InternalClipboardOpen)
		{
			InternalClipboardOpen=false;
			return true;
		}

		return false;
	}

	return CloseClipboard() != FALSE;
}

bool Clipboard::Empty()
{
	if (UseInternalClipboard)
	{
		if (InternalClipboardOpen)
		{
			std::for_each(RANGE(InternalClipboard, i)
			{
				if (i.Handle)
				{
					GlobalFree(i.Handle);
					i.Handle = nullptr;
					i.Format = 0xFFFF;
				}
			});

			return true;
		}

		return false;
	}

	return EmptyClipboard() != FALSE;
}

HANDLE Clipboard::GetData(UINT uFormat)
{
	if (UseInternalClipboard)
	{
		if (InternalClipboardOpen && uFormat != 0xFFFF)
		{
			auto Item = std::find_if(CONST_RANGE(InternalClipboard, i)
			{
				return i.Format == uFormat;
			});

			if (Item != InternalClipboard.cend())
				return Item->Handle;
		}
		return nullptr;
	}

	return GetClipboardData(uFormat);
}

HANDLE Clipboard::SetData(UINT uFormat,HANDLE hMem)
{
	if (UseInternalClipboard)
	{
		if (InternalClipboardOpen)
		{
			auto Item = std::find_if(RANGE(InternalClipboard, i)
			{
				return !i.Handle;
			});

			if (Item != InternalClipboard.cend())
			{
				Item->Handle = hMem;
				Item->Format = uFormat;
				return hMem;
			}
		}

		return nullptr;
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

bool Clipboard::IsFormatAvailable(UINT Format)
{
	if (UseInternalClipboard)
	{
		return Format != 0xFFFF && std::find_if(CONST_RANGE(InternalClipboard, i)
		{
			return i.Format == Format;
		}) != InternalClipboard.cend();
	}

	return IsClipboardFormatAvailable(Format) != FALSE;
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
bool Clipboard::CopyFormat(FAR_CLIPBOARD_FORMAT Format, const wchar_t *Data)
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
			ClipText = DuplicateString(ClipAddr);
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
					ClipText = DuplicateString(strClipText);
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
			int length = std::min(max, StrLength(ClipAddr));
			ClipText = new wchar_t[length + 1];

			if (ClipText)
			{
				xwcsncpy(ClipText,ClipAddr, length + 1);
				ClipText[length] = 0;
			}

			GlobalUnlock(hClipData);
		}
	}

	return ClipText;
}

wchar_t *Clipboard::PasteFormat(FAR_CLIPBOARD_FORMAT Format)
{
	bool isOEMVBlock=false;
	UINT FormatType=RegisterFormat(Format);

	if (!FormatType)
		return nullptr;

	if (Format == FCF_VERTICALBLOCK_UNICODE && !IsFormatAvailable(FormatType))
	{
		FormatType=RegisterFormat(FCF_VERTICALBLOCK_OEM);
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

			ClipText = new wchar_t[BufferSize];

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

int CopyFormatToClipboard(FAR_CLIPBOARD_FORMAT Format,const wchar_t *Data)
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

wchar_t *PasteFormatFromClipboard(FAR_CLIPBOARD_FORMAT Format)
{
	Clipboard clip;

	if (!clip.Open())
		return nullptr;

	wchar_t *ClipText = clip.PasteFormat(Format);

	clip.Close();

	return ClipText;
}

bool EmptyInternalClipboard()
{
	bool OldState = Clipboard::SetUseInternalClipboardState(true);

	Clipboard clip;

	if (!clip.Open())
		return false;

	bool ret = clip.Empty();

	clip.Close();

	Clipboard::SetUseInternalClipboardState(OldState);

	return ret;
}
