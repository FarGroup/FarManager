#pragma once

/*
codepage.hpp

Работа с кодовыми страницами
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

#include "plugin.hpp"

// Тип выбранной таблицы символов
enum CPSelectType
{
	// "Любимая" таблица символов
	CPST_FAVORITE = 1,
	// Таблица символов участвующая в поиске по всем таблицам символов
	CPST_FIND = 2
};

extern const wchar_t *FavoriteCodePagesKey;

const int StandardCPCount = 2 /* OEM, ANSI */ + 2 /* UTF-16 LE, UTF-16 BE */ + 2 /* UTF-7, UTF-8 */;

inline bool IsStandardCodePage(UINT CP) { return(CP==CP_UNICODE)||(CP==CP_UTF8)||(CP==CP_UTF7)||(CP==CP_REVERSEBOM)||(CP==GetOEMCP()||CP==GetACP()); }

inline bool IsUnicodeCodePage(UINT CP) { return(CP==CP_UNICODE)||(CP==CP_REVERSEBOM); }

inline bool IsUnicodeOrUtfCodePage(UINT CP) { return(CP==CP_UNICODE)||(CP==CP_UTF8)||(CP==CP_UTF7)||(CP==CP_REVERSEBOM); }

bool IsCodePageSupported(UINT CodePage);

UINT SelectCodePage(UINT nCurrent, bool bShowUnicode, bool bShowUTF, bool bShowUTF7=false, bool bShowAutoDetect=false);

UINT FillCodePagesList(HANDLE dialogHandle, UINT controlId, UINT codePage, bool allowAuto, bool allowAll, bool allowDefault=false, bool allowM2=false);

wchar_t *FormatCodePageName(UINT CodePage, wchar_t *CodePageName, size_t Length);
