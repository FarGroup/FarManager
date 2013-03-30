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
};

wchar_t* PasteFormatFromClipboard(FAR_CLIPBOARD_FORMAT Format);
int CopyFormatToClipboard(FAR_CLIPBOARD_FORMAT Format, const wchar_t *Data);
int CopyToClipboard(const wchar_t *Data);
wchar_t* PasteFromClipboard();
wchar_t* PasteFromClipboardEx(int max);
bool EmptyInternalClipboard();

class Clipboard
{
public:
	Clipboard() {}
	~Clipboard() {}

	bool Open();
	bool Close();
	bool Empty();
	bool Copy(const wchar_t *Data);
	bool CopyFormat(FAR_CLIPBOARD_FORMAT Format, const wchar_t *Data);
	bool CopyHDROP(const void* NamesArray, size_t NamesArraySize);
	wchar_t *Paste();
	wchar_t *PasteEx(int max);
	wchar_t *PasteFormat(FAR_CLIPBOARD_FORMAT Format);
	bool InternalCopy(bool FromWin);

	static bool SetUseInternalClipboardState(bool State); //Sets UseInternalClipboard to State, and returns previous state
	static bool GetUseInternalClipboardState();

private:
	UINT RegisterFormat(FAR_CLIPBOARD_FORMAT Format);
	bool IsFormatAvailable(UINT Format);
	HANDLE GetData(UINT uFormat);
	HANDLE SetData(UINT uFormat, HANDLE hMem);

	static bool UseInternalClipboard;
	static bool InternalClipboardOpen;
};
