#ifndef CLIPBOARD_HPP_989E040C_4D10_4D7C_88C0_5EF499171878
#define CLIPBOARD_HPP_989E040C_4D10_4D7C_88C0_5EF499171878
#pragma once

/*
clipboard.hpp

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

enum FAR_CLIPBOARD_FORMAT
{
	FCF_VERTICALBLOCK_OEM,
	FCF_VERTICALBLOCK_UNICODE,
	FCF_CFSTR_PREFERREDDROPEFFECT,
	FCF_MSDEVCOLUMNSELECT,
	FCF_BORLANDIDEVBLOCK,

	FCF_COUNT
};

int SetClipboardFormat(FAR_CLIPBOARD_FORMAT Format, const wchar_t *Data, size_t Size);
inline int SetClipboardFormat(FAR_CLIPBOARD_FORMAT Format, const wchar_t* Data) { return SetClipboardFormat(Format, Data, wcslen(Data)); }
inline int SetClipboardFormat(FAR_CLIPBOARD_FORMAT Format, const string& Data) { return SetClipboardFormat(Format, Data.data(), Data.size()); }

int SetClipboard(const wchar_t* Data, size_t Size);
inline int SetClipboard(const wchar_t* Data) { return SetClipboard(Data, wcslen(Data)); }
inline int SetClipboard(const string& Data) { return SetClipboard(Data.data(), Data.size()); }

bool GetClipboardFormat(FAR_CLIPBOARD_FORMAT Format, string& data);
bool GetClipboard(string& data);
bool GetClipboardEx(int max, string& data);
bool ClearInternalClipboard();

class Clipboard: noncopyable
{
public:
	~Clipboard() { Close(); }
	static bool Open();
	static bool Close();
	static bool Clear();
	static bool SetText(const wchar_t *Data, size_t Size);
	static bool SetText(const wchar_t *Data) { return SetText(Data, wcslen(Data)); }
	static bool SetText(const string& Data) { return SetText(Data.data(), Data.size()); }
	static bool SetFormat(FAR_CLIPBOARD_FORMAT Format, const wchar_t *Data, size_t Size);
	static bool SetFormat(FAR_CLIPBOARD_FORMAT Format, const wchar_t *Data) { return SetFormat(Format, Data, wcslen(Data)); }
	static bool SetFormat(FAR_CLIPBOARD_FORMAT Format, const string& Data) { return SetFormat(Format, Data.data(), Data.size()); }
	static bool SetHDROP(const string& NamesData, bool bMoved = false);
	static bool Get(string& data);
	static bool GetEx(int max, string& data);
	static bool GetFormat(FAR_CLIPBOARD_FORMAT Format, string& data);
	static bool InternalCopy(bool FromWin);

	static bool SetUseInternalClipboardState(bool State); //Sets UseInternalClipboard to State, and returns previous state
	static bool GetUseInternalClipboardState();

private:
	static UINT RegisterFormat(FAR_CLIPBOARD_FORMAT Format);
	static bool IsFormatAvailable(UINT Format);
	static HANDLE GetData(UINT uFormat);
	static bool SetData(UINT uFormat, HGLOBAL hMem);
	static bool SetData(UINT uFormat, os::memory::global::ptr&& hMem);

	static bool UseInternalClipboard;
	static bool InternalClipboardOpened;
	static bool SystemClipboardOpened;
};

#endif // CLIPBOARD_HPP_989E040C_4D10_4D7C_88C0_5EF499171878
