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

// Internal:

// Platform:

// Common:
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------


[[nodiscard]]
constexpr bool IsEol(wchar_t x) noexcept { return x == L'\r' || x == L'\n'; }

[[nodiscard]]
inline bool IsBlankOrEos(wchar_t x) noexcept { return std::iswblank(x) || !x; }

[[nodiscard]]
string_view GetSpaces();

[[nodiscard]]
string_view GetEols();

[[nodiscard]]
bool is_alpha(wchar_t Char);
[[nodiscard]]
bool is_alphanumeric(wchar_t Char);

[[nodiscard]]
bool is_upper(wchar_t Char);
[[nodiscard]]
bool is_lower(wchar_t Char);

namespace inplace
{
	void upper(span<wchar_t> Str);
	void lower(span<wchar_t> Str);

	void upper(wchar_t& Char);
	void lower(wchar_t& Char);

	void upper(wchar_t* Str);
	void lower(wchar_t* Str);

	void upper(string& Str, size_t Pos = 0, size_t Count = string::npos);
	void lower(string& Str, size_t Pos = 0, size_t Count = string::npos);
}

[[nodiscard]]
wchar_t upper(wchar_t Char);
[[nodiscard]]
wchar_t lower(wchar_t Char);

[[nodiscard]]
string upper(string Str);
[[nodiscard]]
string lower(string Str);

[[nodiscard]]
string upper(string_view Str);
[[nodiscard]]
string lower(string_view Str);

struct [[nodiscard]] hash_icase_t
{
	[[nodiscard]]
	size_t operator()(wchar_t Char) const;

	[[nodiscard]]
	size_t operator()(string_view Str) const;
};

struct [[nodiscard]] equal_icase_t
{
	[[nodiscard]]
	bool operator()(wchar_t Chr1, wchar_t Chr2) const;

	[[nodiscard]]
	bool operator()(string_view Str1, string_view Str2) const;
};

[[nodiscard]]
bool equal_icase(string_view Str1, string_view Str2);
[[nodiscard]]
bool starts_with_icase(string_view Str, string_view Prefix);
[[nodiscard]]
bool ends_with_icase(string_view Str, string_view Suffix);
[[nodiscard]]
size_t find_icase(string_view Str, string_view What, size_t Pos = 0);
[[nodiscard]]
size_t find_icase(string_view Str, wchar_t What, size_t Pos = 0);
[[nodiscard]]
bool contains_icase(string_view Str, string_view What);
[[nodiscard]]
bool contains_icase(string_view Str, wchar_t What);

class i_searcher
{
public:
	virtual ~i_searcher() = default;
	virtual std::optional<std::pair<size_t, size_t>> find_in(string_view Haystack, bool Reverse = {}) const = 0;
	virtual bool contains_in(string_view Haystack) const = 0;
};

class exact_searcher: public i_searcher
{
public:
	NONCOPYABLE(exact_searcher);
	explicit exact_searcher(string_view Needle, bool CanReverse = true);
	std::optional<std::pair<size_t, size_t>> find_in(string_view Haystack, bool Reverse = {}) const override;
	bool contains_in(string_view Haystack) const override;

private:
	template<typename... args>
	using searcher = std::boyer_moore_horspool_searcher<args...>;

	searcher<string_view::const_iterator> m_Searcher;
	std::optional<searcher<string_view::const_reverse_iterator>> m_ReverseSearcher;
	size_t m_NeedleSize;
};

class fuzzy_searcher: public i_searcher
{
public:
	NONCOPYABLE(fuzzy_searcher);
	explicit fuzzy_searcher(string_view Needle, bool CanReverse = true);
	std::optional<std::pair<size_t, size_t>> find_in(string_view Haystack, bool Reverse = {}) const override;
	bool contains_in(string_view Haystack) const override;

private:
	std::optional<std::pair<size_t, size_t>> find_in_uncorrected(string_view Haystack, bool Reverse = {}) const;

	string m_Needle;
	mutable string m_HayStack, m_Intermediate;
	mutable std::vector<WORD> m_Types;

	exact_searcher m_Searcher;
};

using searchers = std::variant
<
	bool, // Just to make it default-constructible
	exact_searcher,
	fuzzy_searcher
>;

inline i_searcher const& init_searcher(searchers& Searchers, bool const Exact, string_view const Needle, const bool CanReverse = true)
{
	return Exact?
		static_cast<i_searcher const&>(Searchers.emplace<exact_searcher>(Needle, CanReverse)) :
		static_cast<i_searcher const&>(Searchers.emplace<fuzzy_searcher>(Needle, CanReverse));
}

#endif // STRING_UTILS_HPP_82ECD8BE_D484_4023_AB42_21D93B2DF8B9
