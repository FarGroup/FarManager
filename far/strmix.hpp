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
#include "common/bytes_view.hpp"
#include "common/enum_tokens.hpp"

// External:

//----------------------------------------------------------------------------

class RegExp;
struct RegExpMatch;
class regex_match;
class named_regex_match;

namespace legacy
{
	wchar_t* QuoteSpaceOnly(wchar_t* Str);
}

[[nodiscard]]
string InsertRegexpQuote(string strStr);

namespace inplace
{
	void QuoteSpace(string& Str);
	void QuoteOuterSpace(string& Str);
}

[[nodiscard]]
inline string QuoteSpace(string Str) { inplace::QuoteSpace(Str); return Str; }

[[nodiscard]]
inline string QuoteSpace(string_view const Str) { return QuoteSpace(string(Str)); }

[[nodiscard]]
inline string QuoteOuterSpace(string Str) { inplace::QuoteOuterSpace(Str); return Str; }

[[nodiscard]]
inline string QuoteOuterSpace(string_view const Str) { return QuoteOuterSpace(string(Str)); }

bool ReplaceStrings(string& strStr, string_view FindStr, string_view ReplStr, bool IgnoreCase = false, size_t Count = string::npos);

inline void replace(string& Str, string_view const Find, string_view const Replace)
{
	ReplaceStrings(Str, Find, Replace, false);
}

inline auto replace(string_view const Str, string_view const Find, string_view const Replace)
{
	string Copy(Str);
	replace(Copy, Find, Replace);
	return Copy;
}

inline void replace_icase(string& Str, string_view const Find, string_view const Replace)
{
	ReplaceStrings(Str, Find, Replace, true);
}

inline auto replace_icase(string_view const Str, string_view const Find, string_view const Replace)
{
	string Copy(Str);
	replace_icase(Copy, Find, Replace);
	return Copy;
}

void remove_duplicates(string& Str, wchar_t Char, bool IgnoreCase = false);

class [[nodiscard]] wrapped_text : public enumerator<wrapped_text, string_view>
{
	IMPLEMENTS_ENUMERATOR(wrapped_text);

public:
	explicit wrapped_text(auto&& Str, size_t const Width):
		m_Str(FWD(Str)),
		m_Tail(m_Str),
		m_Width(Width? Width : m_Tail.size())
	{
	}

private:
	[[nodiscard]]
	bool get(bool Reset, string_view& Value) const;

	string_copyref m_Str;
	string_view mutable m_Tail;
	size_t m_Width;
};

void PrepareUnitStr();

[[nodiscard]]
string FileSizeToStr(unsigned long long FileSize, int WidthWithSign = -1, unsigned long long ViewFlags = COLFLAGS_GROUPDIGITS);

[[nodiscard]]
string FileSizeToStrInvariant(unsigned long long FileSize, int WidthWithSign = -1, unsigned long long ViewFlags = COLFLAGS_GROUPDIGITS);

[[nodiscard]]
bool CheckFileSizeStringFormat(string_view FileSizeStr);

[[nodiscard]]
unsigned long long ConvertFileSizeString(string_view FileSizeStr);

[[nodiscard]]
string ReplaceBrackets(string_view SearchStr, string_view ReplaceStr, std::span<RegExpMatch const> Match, const named_regex_match* NamedMatch);

[[nodiscard]]
string GroupDigits(unsigned long long Value);
[[nodiscard]]
string GroupDigitsInvariant(unsigned long long Value);

[[nodiscard]]
inline bool IsWordDiv(string_view const WordDiv, wchar_t const Chr)
{
	return !Chr || contains(WordDiv, Chr);
}

[[nodiscard]]
bool FindWordInString(string_view Str, size_t CurPos, size_t& Begin, size_t& End, string_view WordDiv0);

namespace legacy
{
	wchar_t* truncate_left(wchar_t* Str, int MaxLength);
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
string truncate_left(string_view Str, size_t MaxLength);

[[nodiscard]]
string truncate_right(string Str, size_t MaxLength);

[[nodiscard]]
string truncate_right(string_view Str, size_t MaxLength);

[[nodiscard]]
string truncate_center(string Str, size_t MaxLength);

[[nodiscard]]
string truncate_center(string_view Str, size_t MaxLength);

[[nodiscard]]
string truncate_path(string Str, size_t MaxLength);

[[nodiscard]]
string truncate_path(string_view Str, size_t MaxLength);

[[nodiscard]]
bool IsCaseMixed(string_view Str);

[[nodiscard]]
bool SearchString(
	string_view Haystack,
	string_view Needle,
	i_searcher const& NeedleSearcher,
	const RegExp& re,
	regex_match& Match,
	named_regex_match* NamedMatch,
	int& CurPos,
	search_replace_string_options options,
	int& SearchLength,
	string_view WordDiv
);

[[nodiscard]]
bool SearchAndReplaceString(
	string_view Haystack,
	string_view Needle,
	i_searcher const& NeedleSearcher,
	const RegExp& re,
	regex_match& Match,
	named_regex_match* NamedMatch,
	string& ReplaceStr,
	int& CurPos,
	search_replace_string_options options,
	int& SearchLength,
	string_view WordDiv
);

[[nodiscard]]
inline wchar_t* UNSAFE_CSTR(const string& s) noexcept {return const_cast<wchar_t*>(s.c_str());}

[[nodiscard]]
inline wchar_t* UNSAFE_CSTR(null_terminated const& s) noexcept {return const_cast<wchar_t*>(s.c_str());}

namespace detail
{
	template<typename flags_type>
	[[nodiscard]]
	auto FlagsToString(unsigned long long Flags, std::span<std::pair<flags_type, string_view> const> const From, wchar_t Separator = L' ')
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

	template<typename flags_type>
	[[nodiscard]]
	auto StringToFlags(string_view const strFlags, std::span<std::pair<flags_type, string_view> const> const From, const string_view Separators = L"|;, "sv)
	{
		flags_type Flags{};

		if (strFlags.empty())
			return Flags;

		for (const auto& i: enum_tokens(strFlags, Separators))
		{
			if (i.empty())
				continue;

			const auto ItemIterator = std::ranges::find_if(From, [&](auto const& j){ return equal_icase(i, j.second); });
			if (ItemIterator != From.end())
				Flags |= ItemIterator->first;
		}

		return Flags;
	}
}

[[nodiscard]]
auto FlagsToString(unsigned long long const Flags, std::ranges::contiguous_range auto const& From, wchar_t const Separator = L' ')
{
	return detail::FlagsToString(Flags, span(From), Separator);
}

[[nodiscard]]
auto StringToFlags(string_view const strFlags, std::ranges::contiguous_range auto const& From, string_view const Separators = L"|;, "sv)
{
	return detail::StringToFlags(strFlags, span(From), Separators);
}

[[nodiscard]]
char IntToHex(int h);

[[nodiscard]]
int HexToInt(char h);

[[nodiscard]]
string BlobToHexString(bytes_view Blob, wchar_t Separator = L',');

[[nodiscard]]
bytes HexStringToBlob(string_view Hex, wchar_t Separator = L',');

[[nodiscard]]
auto to_hex_wstring(std::integral auto Value)
{
	string Result(sizeof(Value) * 2, '0');
	for (int i = sizeof(Value) * 2 - 1; i >= 0; --i, Value >>= 4)
		Result[i] = IntToHex(Value & 0xF);
	return Result;
}

[[nodiscard]]
string ExtractHexString(string_view HexString);

[[nodiscard]]
string ConvertHexString(string_view From, uintptr_t Codepage, bool FromHex);

[[nodiscard]]
string BytesToString(bytes_view Bytes, uintptr_t Codepage);

[[nodiscard]]
string HexMask(size_t ByteCount);

void xstrncpy(char* dest, const char* src, size_t DestSize);
void xwcsncpy(wchar_t* dest, const wchar_t* src, size_t DestSize);

#endif // STRMIX_HPP_66F8DC2A_61A6_4C06_9B54_E0513A9735FA
