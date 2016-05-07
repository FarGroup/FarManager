#ifndef STRMIX_HPP_66F8DC2A_61A6_4C06_9B54_E0513A9735FA
#define STRMIX_HPP_66F8DC2A_61A6_4C06_9B54_E0513A9735FA
#pragma once

/*
strmix.hpp

Куча разных вспомогательных функций по работе со строками
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

#include "RegExp.hpp"
#include "panelctype.hpp"

namespace strmix
{
wchar_t* QuoteSpace(wchar_t *Str);
wchar_t* InsertQuote(wchar_t *Str);
void Unquote(wchar_t *Str);
wchar_t* RemoveLeadingSpaces(wchar_t *Str);
wchar_t * RemoveTrailingSpaces(wchar_t *Str);
wchar_t* RemoveExternalSpaces(wchar_t *Str);
wchar_t* QuoteSpaceOnly(wchar_t *Str);

string &QuoteSpace(string &strStr);

string& InsertQuote(string& strStr);
inline string InsertQuote(string&& strStr) { InsertQuote(strStr); return strStr; }

string& Unquote(string &strStr);
inline string Unquote(string&& strStr) { Unquote(strStr); return strStr; }

string& InsertRegexpQuote(string& strStr);
void UnquoteExternal(string &strStr);
string& RemoveLeadingSpaces(string &strStr);
string& RemoveTrailingSpaces(string &strStr);
string& RemoveExternalSpaces(string &strStr);
string& RemoveUnprintableCharacters(string &strStr);
string& QuoteSpaceOnly(string &strStr);
string& QuoteOuterSpace(string &strStr);
inline string QuoteOuterSpace(string&& strStr) { QuoteOuterSpace(strStr); return strStr; }

size_t ReplaceStrings(string &strStr, const wchar_t* FindStr, size_t FindStrSize, const wchar_t* ReplStr, size_t ReplStrSize, bool IgnoreCase = false, size_t Count = string::npos);

inline size_t ReplaceStrings(string &strStr, const string& FindStr, const string& ReplStr, bool IgnoreCase = false, size_t Count = string::npos)
{
	return ReplaceStrings(strStr, FindStr.data(), FindStr.size(), ReplStr.data(), ReplStr.size(), IgnoreCase, Count);
}

inline size_t ReplaceStrings(string &strStr, const wchar_t* FindStr, const wchar_t* ReplStr, bool IgnoreCase = false, size_t Count = string::npos)
{
	return ReplaceStrings(strStr, FindStr, wcslen(FindStr), ReplStr, wcslen(ReplStr), IgnoreCase, Count);
}

const wchar_t *GetCommaWord(const wchar_t *Src,string &strWord,wchar_t Separator=L',');

string& FarFormatText(const string& SrcText, int Width, string &strDestText, const wchar_t* Break, DWORD Flags);

void PrepareUnitStr();
string FileSizeToStr(unsigned long long Size, int Width = -1, unsigned long long ViewFlags = COLUMN_COMMAS);
bool CheckFileSizeStringFormat(const string& FileSizeStr);
unsigned __int64 ConvertFileSizeString(const string& FileSizeStr);
string FormatNumber(const string& Src, int NumDigits=0);
inline string InsertCommas(unsigned long long Value) { return FormatNumber(std::to_wstring(Value)); }

inline bool IsWordDiv(const string& WordDiv, wchar_t Chr) { return !Chr || WordDiv.find(Chr) != string::npos; }

bool FindWordInString(const string& Str, size_t CurPos, size_t& Begin, size_t& End, const string& WordDiv);

wchar_t* TruncStr(wchar_t *Str,int MaxLength);
wchar_t* TruncStrFromCenter(wchar_t *Str, int MaxLength);
wchar_t* TruncStrFromEnd(wchar_t *Str,int MaxLength);
wchar_t* TruncPathStr(wchar_t *Str, int MaxLength);

string& TruncStr(string &strStr,int MaxLength);
string& TruncStrFromEnd(string &strStr, int MaxLength);
string& TruncStrFromCenter(string &strStr, int MaxLength);
string& TruncPathStr(string &strStr, int MaxLength);

bool IsCaseMixed(const string &strStr);

string& CenterStr(const string& Src, string &strDest,int Length);
string& RightStr(const string& Src, string &strDest, int Length);

string ReplaceBrackets(const wchar_t *SearchStr, const string& ReplaceStr, const RegExpMatch* Match, size_t Count, const MatchHash* HMatch);

string GuidToStr(const GUID& Guid);
bool StrToGuid(const wchar_t* Value,GUID& Guid);
inline bool StrToGuid(const string& Value, GUID& Guid) { return StrToGuid(Value.data(), Guid); }

bool SearchString(const wchar_t* Source, int StrSize, const string& Str, const string &UpperStr, const string &LowerStr, class RegExp &re, RegExpMatch *pm, MatchHash* hm, string& ReplaceStr,int& CurPos, int Position,int Case,int WholeWords,int Reverse,int Regexp,int PreserveStyle, int *SearchLength,const wchar_t* WordDiv=nullptr);

inline int StrCmp(const string& a, const string& b) { return ::StrCmp(a.data(), b.data()); }
inline int StrCmp(const wchar_t* a, const string& b) { return ::StrCmp(a, b.data()); }
inline int StrCmp(const string& a, const wchar_t* b) { return ::StrCmp(a.data(), b); }

inline int StrCmpI(const string& a, const string& b) { return ::StrCmpI(a.data(), b.data()); }
inline int StrCmpI(const wchar_t* a, const string& b) { return ::StrCmpI(a, b.data()); }
inline int StrCmpI(const string& a, const wchar_t* b) { return ::StrCmpI(a.data(), b); }

string wide_n(const char *str, size_t size, uintptr_t codepage = CP_OEMCP);
inline string wide(const char *str, uintptr_t codepage = CP_OEMCP) { return wide_n(str, strlen(str), codepage); }
inline string wide(const std::string& str, uintptr_t codepage = CP_OEMCP) { return wide_n(str.data(), str.size(), codepage); }

std::string narrow_n(const wchar_t *str, size_t size, uintptr_t codepage = CP_OEMCP);
inline std::string narrow(const wchar_t *str, uintptr_t codepage = CP_OEMCP) { return narrow_n(str, wcslen(str), codepage); }
inline std::string narrow(const string& str, uintptr_t codepage = CP_OEMCP) { return narrow_n(str.data(), str.size(), codepage); }

template<class T>
std::string Utf8String(const T& Str) { return narrow(Str, CP_UTF8); }

string str_printf(const wchar_t * format, ...);
string str_vprintf(const wchar_t * format, va_list argptr);

inline string& InplaceUpper(string& str, size_t pos = 0, size_t n = string::npos) { std::transform(str.begin() + pos, n == string::npos? str.end() : str.begin() + pos + n, str.begin() + pos, ::Upper); return str; }
inline string& InplaceLower(string& str, size_t pos = 0, size_t n = string::npos) { std::transform(str.begin() + pos, n == string::npos? str.end() : str.begin() + pos + n, str.begin() + pos, ::Lower); return str; }
inline string Upper(string str, size_t pos = 0, size_t n = string::npos) { return InplaceUpper(str, pos, n); }
inline string Lower(string str, size_t pos = 0, size_t n = string::npos) { return InplaceLower(str, pos, n); }

inline wchar_t* UNSAFE_CSTR(const string& s) {return const_cast<wchar_t*>(s.data());}

enum STL_FLAGS
{
	STLF_PACKASTERISKS   = BIT(0), // вместо "*.*" в список помещать просто "*", вместо "***" в список помещать просто "*"
	STLF_PROCESSBRACKETS = BIT(1), // учитывать квадратные скобки при анализе строки инициализации
	STLF_ALLOWEMPTY      = BIT(2), // allow empty items
	STLF_UNIQUE          = BIT(3), // убирать дублирующиеся элементы
	STLF_SORT            = BIT(4), // отсортировать (с учетом регистра)
	STLF_NOTRIM          = BIT(5), // не удалять пробелы
	STLF_NOUNQUOTE       = BIT(6), // не раскавычивать
	STLF_NOQUOTING       = BIT(7), // do not give special meaning for quotes
};

void split(const string& InitString, DWORD Flags, const wchar_t* Separators, const std::function<void(string&&)>& inserter);

template <class Container>
auto split(const string& InitString, DWORD Flags = 0, const wchar_t* Separators = L";,")
{
	Container C;
	split(InitString, Flags, Separators, [&](string&& str) { C.emplace(C.end(), std::move(str)); });
	return C;
}

template<class container>
string FlagsToString(unsigned long long Flags, const container& From, wchar_t Separator = L' ')
{
	string strFlags;
	std::for_each(CONST_RANGE(From, i)
	{
		if (Flags & i.first)
		{
			strFlags.append(i.second).append(1, Separator);
		}
	});

	if (!strFlags.empty())
	{
		strFlags.pop_back();
	}

	return strFlags;
}

template<class container>
unsigned long long StringToFlags(const string& strFlags, const container& From, const wchar_t* Separators = L"|;, ")
{
	auto Flags = decltype(std::begin(From)->first)();
	if (!strFlags.empty())
	{
		for (const auto& i: split<std::vector<string>>(strFlags, STLF_UNIQUE, Separators))
		{
			const auto ItemIterator = std::find_if(CONST_RANGE(From, j)
			{
				return !StrCmpI(i, j.second);
			});
			if (ItemIterator != std::cend(From))
				Flags |= ItemIterator->first;
		}
	}
	return Flags;
}

char IntToHex(int h);
int HexToInt(char h);

std::string BlobToHexString(const void* Blob, size_t Size, char Separator = ',');
std::vector<char> HexStringToBlob(const char* Hex, char Separator = ',');

string BlobToHexWString(const void* Blob, size_t Size, wchar_t Separator = L',');
std::vector<char> HexStringToBlob(const wchar_t* Hex, wchar_t Separator = L',');


template<class S, class T>
S to_hex_string_t(T Value)
{
	static_assert(std::is_integral<T>::value, "Integral value required");
	S Result;
	Result.resize(sizeof(T) * 2, '0');
	for (int i = sizeof(T) * 2 - 1; i >= 0; --i, Value >>= 4)
		Result[i] = IntToHex(Value & 0xF);
	return Result;
}

template<class T>
auto to_hex_string(T Value) { return to_hex_string_t<std::string>(Value); }

template<class T>
auto to_hex_wstring(T Value) { return to_hex_string_t<string>(Value); }

struct string_i_less
{
	bool operator()(const string& a, const string& b) const
	{
		return StrCmpI(a, b) < 0;
	}
};

};

using namespace strmix;

#endif // STRMIX_HPP_66F8DC2A_61A6_4C06_9B54_E0513A9735FA
