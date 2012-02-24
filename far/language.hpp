#pragma once

/*
language.hpp

Работа с LNG-файлами
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

#include "lang.hpp"

enum LngErrors
{
	LERROR_SUCCESS,
	LERROR_FILE_NOT_FOUND,
	LERROR_BAD_FILE,
};

class Language
{
public:
	Language();
	~Language();

	bool Init(const wchar_t *Path, int CountNeed=-1);
#ifndef NO_WRAPPER
	bool InitA(const wchar_t *Path, int CountNeed=-1);
#endif // NO_WRAPPER
	void Close();

	const wchar_t* GetMsg(LNGID nID) const;

#ifndef NO_WRAPPER
	const char* GetMsgA(LNGID nID) const;
#endif // NO_WRAPPER

	bool IsLanguageLoaded() const {return LanguageLoaded;}
	LngErrors GetLastError() const {return LastError;}

private:
	bool CheckMsgId(LNGID MsgId) const;
	void Free();

	string strMessageFile;
	wchar_t **MsgAddr;
	wchar_t *MsgList;
#ifndef NO_WRAPPER
	char **MsgAddrA; //фантастика, да
	char *MsgListA;
#endif // NO_WRAPPER
	int MsgCount;
	LngErrors LastError;
#ifndef NO_WRAPPER
	bool m_bUnicode;
#endif // NO_WRAPPER
	bool LanguageLoaded;
};

extern Language Lang;

#define MSG(ID) ::Lang.GetMsg(ID)

class VMenu;

FILE* OpenLangFile(const wchar_t *Path,const wchar_t *Mask,const wchar_t *Language, string &strFileName, UINT &nCodePage, BOOL StrongLang=FALSE,string *pstrLangName=nullptr);
int GetLangParam(FILE *SrcFile,const wchar_t *ParamName,string *strParam1, string *strParam2, UINT nCodePage);
int GetOptionsParam(FILE *SrcFile,const wchar_t *KeyName,string &strValue, UINT nCodePage);
bool Select(int HelpLanguage,VMenu **MenuPtr);

template<class T>
LNGID operator+(LNGID Id, T Shift)
{
	return static_cast<LNGID>(static_cast<DWORD>(Id)+static_cast<DWORD>(Shift));
}
