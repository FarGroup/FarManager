#ifndef LANGUAGE_HPP_36726BFA_4EBB_4CFF_A8F0_42434C4F4865
#define LANGUAGE_HPP_36726BFA_4EBB_4CFF_A8F0_42434C4F4865
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

class Language
{
public:
	NONCOPYABLE(Language);
	TRIVIALLY_MOVABLE(Language);

	Language(const string& Path, int CountNeed = -1) { init(Path, CountNeed); }
	virtual ~Language() = default;

	const wchar_t* GetMsg(LNGID nID) const;

protected:
	Language() = default;

	void init(const string& Path, int CountNeed = -1);
	bool CheckMsgId(LNGID MsgId) const;

private:
	virtual size_t size() const { return m_Messages.size(); }
	virtual void reserve(size_t size) { m_Messages.reserve(size); }
	virtual void add(string&& str) { m_Messages.emplace_back(std::move(str)); }

	std::vector<string> m_Messages;
	string m_FileName;
};

bool OpenLangFile(os::fs::file& LangFile, const string& Path, const string& Mask, const string& Language, string &strFileName, uintptr_t &nCodePage, bool StrongLang = false, string *pstrLangName = nullptr);
int GetLangParam(os::fs::file& LangFile, const string& ParamName,string *strParam1, string *strParam2, UINT nCodePage);
int GetOptionsParam(os::fs::file& LangFile, const wchar_t *KeyName,string &strValue, UINT nCodePage);
bool SelectInterfaceLanguage();
bool SelectHelpLanguage();

template<class T>
LNGID operator+(LNGID Id, T Shift)
{
	return static_cast<LNGID>(static_cast<std::underlying_type_t<LNGID>>(Id) + Shift);
}

inline LNGID operator++(LNGID& Id, int)
{
	const auto Value = Id;
	Id = Id + 1;
	return Value;
}

#define MSG(ID) Global->Lang->GetMsg(ID)

template<typename... args>
auto format(LNGID Id, args&&... Args)
{
	return format(MSG(Id), std::forward<args>(Args)...);
}

#endif // LANGUAGE_HPP_36726BFA_4EBB_4CFF_A8F0_42434C4F4865
