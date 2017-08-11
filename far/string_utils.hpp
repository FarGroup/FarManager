#ifndef STRING_UTILS_HPP_82ECD8BE_D484_4023_AB42_21D93B2DF8B9
#define STRING_UTILS_HPP_82ECD8BE_D484_4023_AB42_21D93B2DF8B9
#pragma once

/*
string_utils.hpp

Сравнение без учета регистра, преобразование регистра
*/
/*
Copyright © 2017 Far Group
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

extern const wchar_t WIN_EOL_fmt[];     // <CR><LF>     // same as DOS
extern const wchar_t UNIX_EOL_fmt[];    // <LF>         //
extern const wchar_t OLD_MAC_EOL_fmt[]; // <CR>         // modern is Unix <LF>
extern const wchar_t BAD_WIN_EOL_fmt[]; // <CR><CR><LF> // result of <CR><LF> text mode conversion

inline int StrLength(const wchar_t *str) { return static_cast<int>(wcslen(str)); }

inline bool IsSpace(wchar_t x) { return x == L' ' || x == L'\t'; }

inline bool IsEol(wchar_t x) { return x == L'\r' || x == L'\n'; }

inline bool IsSpaceOrEos(wchar_t x) { return IsSpace(x) || !x; }

inline bool IsSpaceOrEol(wchar_t x) { return IsSpace(x) || IsEol(x); }

const string& GetSpaces();

const string& GetEols();

const string& GetSpacesAndEols();

bool is_alpha(wchar_t Char);
bool is_alphanumeric(wchar_t Char);

bool is_upper(wchar_t Char);
bool is_lower(wchar_t Char);

namespace inplace
{
	void upper(wchar_t* Str, size_t Size);
	void lower(wchar_t* Str, size_t Size);

	void upper(wchar_t* Str);
	void lower(wchar_t* Str);

	string& upper(string& Str, size_t Pos = 0, size_t Count = string::npos);
	string& lower(string& Str, size_t Pos = 0, size_t Count = string::npos);
}

wchar_t upper(wchar_t Char);
wchar_t lower(wchar_t Char);

string upper(string Str);
string lower(string Str);

struct hash_icase
{
	size_t operator()(const string& Str) const;
};

struct equal_to_icase
{
	bool operator()(wchar_t Chr1, wchar_t Chr2) const;
	bool operator()(const string_view& Str1, const string_view& Str2) const;
};

bool equal_icase(const string_view& Str1, const string_view& Str2);
bool starts_with_icase(const string_view& Str, const string_view& Prefix);
bool ends_with_icase(const string_view& Str, const string_view& Suffix);
bool contains_icase(const string_view& Str, const string_view& Token);

int StrCmp(const wchar_t *s1, const wchar_t *s2);
int StrCmpI(const wchar_t *s1, const wchar_t *s2);

// deprecated, for pluginapi::apiStrCmpNI only
int StrCmpNI(const wchar_t *s1, const wchar_t *s2, size_t n);

int StrCmp(const string_view& Str1, const string_view& Str2);
int StrCmpI(const string_view& Str1, const string_view& Str2);
int StrCmpC(const string_view& Str1, const string_view& Str2);

int NumStrCmp(const string_view& Str1, const string_view& Str2);
int NumStrCmpI(const string_view& Str1, const string_view& Str2);
int NumStrCmpC(const string_view& Str1, const string_view& Str2);

using str_comparer = int(*)(const string_view&, const string_view&);
str_comparer get_comparer(bool Numeric, bool CaseSensitive);

#endif // STRING_UTILS_HPP_82ECD8BE_D484_4023_AB42_21D93B2DF8B9
