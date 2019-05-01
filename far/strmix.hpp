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

// Internal:
#include "panelctype.hpp"
#include "string_utils.hpp"

// Platform:

// Common:
#include "common/enum_tokens.hpp"

// External:

//----------------------------------------------------------------------------

class bytes;
class bytes_view;
class RegExp;
struct RegExpMatch;
struct MatchHash;

wchar_t* QuoteSpace(wchar_t *Str);
wchar_t* InsertQuote(wchar_t *Str);
wchar_t* QuoteSpaceOnly(wchar_t *Str);

string &QuoteSpace(string &strStr);

[[nodiscard]]
string InsertRegexpQuote(string strStr);

namespace inplace
{
	void QuoteSpaceOnly(string &strStr);
	void QuoteOuterSpace(string &strStr);
}

[[nodiscard]]
inline string QuoteOuterSpace(string strStr) { inplace::QuoteOuterSpace(strStr); return strStr; }
[[nodiscard]]
inline string QuoteSpaceOnly(string strStr) { inplace::QuoteSpaceOnly(strStr); return strStr; }

bool ReplaceStrings(string& strStr, string_view FindStr, string_view ReplStr, bool IgnoreCase = false, size_t Count = string::npos);

inline void replace(string& Str, string_view const Find, string_view const Replace)
{
	ReplaceStrings(Str, Find, Replace, false);
}

inline void replace_icase(string& Str, string_view const Find, string_view const Replace)
{
	ReplaceStrings(Str, Find, Replace, true);
}

void remove_duplicates(string& Str, wchar_t Char, bool IgnoreCase = false);

class [[nodiscard]] wrapped_text : public enumerator<wrapped_text, string_view>
{
	IMPLEMENTS_ENUMERATOR(wrapped_text);

public:
	explicit wrapped_text(string_view Str, size_t Width, string_view Break = L"\n"sv, bool BreakWords = true);
	explicit wrapped_text(string&& Str, size_t Width, string_view Break = L"\n"sv, bool BreakWords = true);

private:
	bool get(bool Reset, string_view& Value) const;

	string m_StrBuffer;
	string_view m_Str;
	string_view mutable m_Tail;
	string_view m_Break;
	size_t m_Width;
	bool m_BreakWords;
};

void PrepareUnitStr();

[[nodiscard]]
string FileSizeToStr(unsigned long long Size, int Width = -1, unsigned long long ViewFlags = COLUMN_GROUPDIGITS);

[[nodiscard]]
bool CheckFileSizeStringFormat(const string& FileSizeStr);

[[nodiscard]]
unsigned long long ConvertFileSizeString(const string& FileSizeStr);

[[nodiscard]]
string GroupDigits(unsigned long long Value);

[[nodiscard]]
inline bool IsWordDiv(const string& WordDiv, wchar_t Chr) { return !Chr || contains(WordDiv, Chr); }

[[nodiscard]]
bool FindWordInString(const string& Str, size_t CurPos, size_t& Begin, size_t& End, const string& WordDiv);

namespace legacy
{
	wchar_t* truncate_left(wchar_t* Str, int MaxLength);
	wchar_t* truncate_center(wchar_t* Str, int MaxLength);
	wchar_t* truncate_right(wchar_t* Str, int MaxLength);
	wchar_t* truncate_path(wchar_t* Str, int MaxLength);
}

namespace inplace
{
	void truncate_left(string& Str, size_t MaxLength);
	void truncate_right(string& Str, size_t MaxLength);
	void truncate_center(string& Str, size_t MaxLength);
	void truncate_path(string& Str, size_t MaxLength);
}

[[nodiscard]]
string truncate_left(string Str, size_t MaxLength);
[[nodiscard]]
string truncate_right(string Str, size_t MaxLength);
[[nodiscard]]
string truncate_center(string Str, size_t MaxLength);
[[nodiscard]]
string truncate_path(string Str, size_t MaxLength);

[[nodiscard]]
bool IsCaseMixed(string_view strStr);

[[nodiscard]]
bool SearchString(
	string_view Haystack,
	string_view Needle,
	string_view NeedleUpper,
	string_view NeedleLower,
	const RegExp& re,
	RegExpMatch* pm,
	MatchHash* hm,
	string& ReplaceStr,
	int& CurPos,
	bool Case,
	bool WholeWords,
	bool Reverse,
	bool Regexp,
	bool PreserveStyle,
	int* SearchLength,
	string_view WordDiv = {}
);

[[nodiscard]]
inline wchar_t* UNSAFE_CSTR(const string& s) {return const_cast<wchar_t*>(s.c_str());}

template<class container>
[[nodiscard]]
auto FlagsToString(unsigned long long Flags, const container& From, wchar_t Separator = L' ')
{
	string strFlags;
	for (const auto& [Value, Name]: From)
	{
		if (Flags & Value)
		{
			append(strFlags, Name, Separator);
		}
	}

	if (!strFlags.empty())
	{
		strFlags.pop_back();
	}

	return strFlags;
}

template<class container>
[[nodiscard]]
auto StringToFlags(const string& strFlags, const container& From, const string_view Separators = L"|;, "sv)
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

[[nodiscard]]
char IntToHex(int h);

[[nodiscard]]
int HexToInt(char h);

[[nodiscard]]
std::string BlobToHexString(const void* Blob, size_t Size, char Separator = ',');

[[nodiscard]]
std::string BlobToHexString(const bytes_view& Blob, char Separator = ',');

[[nodiscard]]
bytes HexStringToBlob(std::string_view Hex, char Separator = ',');

[[nodiscard]]
string BlobToHexWString(const void* Blob, size_t Size, wchar_t Separator = L',');

[[nodiscard]]
string BlobToHexWString(const bytes_view& Blob, char Separator = ',');

[[nodiscard]]
bytes HexStringToBlob(string_view Hex, wchar_t Separator = L',');

template<class S, class T>
[[nodiscard]]
auto to_hex_string_t(T Value)
{
	static_assert(std::is_integral_v<T>);
	S Result(sizeof(T) * 2, '0');
	for (int i = sizeof(T) * 2 - 1; i >= 0; --i, Value >>= 4)
		Result[i] = IntToHex(Value & 0xF);
	return Result;
}

template<class T>
[[nodiscard]]
auto to_hex_string(T Value) { return to_hex_string_t<std::string>(Value); }

template<class T>
[[nodiscard]]
auto to_hex_wstring(T Value) { return to_hex_string_t<string>(Value); }

[[nodiscard]]
string ExtractHexString(string_view HexString);

[[nodiscard]]
string ConvertHexString(const string& From, uintptr_t Codepage, bool FromHex);

char* xstrncpy(char* dest, const char* src, size_t DestSize);
wchar_t* xwcsncpy(wchar_t* dest, const wchar_t* src, size_t DestSize);

[[nodiscard]]
std::pair<string_view, string_view> split_name_value(string_view Str);

#endif // STRMIX_HPP_66F8DC2A_61A6_4C06_9B54_E0513A9735FA
