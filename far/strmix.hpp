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

#include "panelctype.hpp"
#include "string_utils.hpp"

class RegExp;
struct RegExpMatch;
struct MatchHash;

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

size_t ReplaceStrings(string &strStr, const string_view& FindStr, const string_view& ReplStr, bool IgnoreCase = false, size_t Count = string::npos);

const wchar_t *GetCommaWord(const wchar_t *Src,string &strWord,wchar_t Separator=L',');

string& FarFormatText(const string& SrcText, size_t Width, string &strDestText, const wchar_t* Break, DWORD Flags);

void PrepareUnitStr();
string FileSizeToStr(unsigned long long Size, int Width = -1, unsigned long long ViewFlags = COLUMN_COMMAS);
bool CheckFileSizeStringFormat(const string& FileSizeStr);
unsigned long long ConvertFileSizeString(const string& FileSizeStr);
string FormatNumber(const string& Src, int NumDigits=0);
inline string InsertCommas(unsigned long long Value) { return FormatNumber(str(Value)); }

inline bool IsWordDiv(const string& WordDiv, wchar_t Chr) { return !Chr || contains(WordDiv, Chr); }

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

string ReplaceBrackets(const wchar_t *SearchStr, const string& ReplaceStr, const RegExpMatch* Match, size_t Count, const MatchHash* HMatch);

bool SearchString(const wchar_t* Source, int StrSize, const string& Str, const string &UpperStr, const string &LowerStr, RegExp &re, RegExpMatch *pm, MatchHash* hm, string& ReplaceStr,int& CurPos, int Position,int Case,int WholeWords,int Reverse,int Regexp,int PreserveStyle, int *SearchLength,const wchar_t* WordDiv=nullptr);

inline wchar_t* UNSAFE_CSTR(const string& s) {return const_cast<wchar_t*>(s.data());}

enum STL_FLAGS
{
	STLF_PACKASTERISKS   = bit(0), // вместо "*.*" в список помещать просто "*", вместо "***" в список помещать просто "*"
	STLF_PROCESSBRACKETS = bit(1), // учитывать квадратные скобки при анализе строки инициализации
	STLF_ALLOWEMPTY      = bit(2), // allow empty items
	STLF_UNIQUE          = bit(3), // убирать дублирующиеся элементы
	STLF_SORT            = bit(4), // отсортировать (с учетом регистра)
	STLF_NOTRIM          = bit(5), // не удалять пробелы
	STLF_NOUNQUOTE       = bit(6), // не раскавычивать
	STLF_NOQUOTING       = bit(7), // do not give special meaning for quotes
};

void split(const string& InitString, DWORD Flags, const wchar_t* Separators, const std::function<void(string&&)>& inserter);

// TODO: replace with enum_tokens. ACHTUNG: there are many side effects due to way too flexible flags above, be extremely careful
template <typename container>
auto split(const string& InitString, DWORD Flags = 0, const wchar_t* Separators = L";,")
{
	container Container;
	split(InitString, Flags, Separators, [&](string&& Str) { emplace(Container, std::move(Str)); });
	return Container;
}

template<class container>
auto FlagsToString(unsigned long long Flags, const container& From, wchar_t Separator = L' ')
{
	string strFlags;
	std::for_each(CONST_RANGE(From, i)
	{
		if (Flags & i.first)
		{
			append(strFlags, i.second, Separator);
		}
	});

	if (!strFlags.empty())
	{
		strFlags.pop_back();
	}

	return strFlags;
}

template<class container>
auto StringToFlags(const string& strFlags, const container& From, const wchar_t* Separators = L"|;, ")
{
	decltype(std::begin(From)->first) Flags {};

	if (strFlags.empty())
		return Flags;

	for (const auto& i: enum_tokens(strFlags, Separators))
	{
		const auto ItemIterator = std::find_if(CONST_RANGE(From, j) { return equal_icase(i, j.second); });
		if (ItemIterator != std::cend(From))
			Flags |= ItemIterator->first;
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
auto to_hex_string_t(T Value)
{
	static_assert(std::is_integral<T>::value);
	S Result(sizeof(T) * 2, '0');
	for (int i = sizeof(T) * 2 - 1; i >= 0; --i, Value >>= 4)
		Result[i] = IntToHex(Value & 0xF);
	return Result;
}

template<class T>
auto to_hex_string(T Value) { return to_hex_string_t<std::string>(Value); }

template<class T>
auto to_hex_wstring(T Value) { return to_hex_string_t<string>(Value); }

string ExtractHexString(const string& HexString);
string ConvertHexString(const string& From, uintptr_t Codepage, bool FromHex);

struct string_i_less
{
	bool operator()(const string& a, const string& b) const
	{
		return StrCmpI(a, b) < 0;
	}
};

char* xstrncpy(char* dest, const char* src, size_t DestSize);
wchar_t* xwcsncpy(wchar_t* dest, const wchar_t* src, size_t DestSize);

std::pair<string, string> split_name_value(const wchar_t* Line);

template<typename T>
std::pair<string, string> split_name_value(const T& Line)
{
	const auto SeparatorPos = std::find(ALL_CONST_RANGE(Line), L'=');
	return { { Line.begin(), SeparatorPos }, { SeparatorPos + 1, Line.end() } };
}

};

using namespace strmix;

#endif // STRMIX_HPP_66F8DC2A_61A6_4C06_9B54_E0513A9735FA
