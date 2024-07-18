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

// External:

//----------------------------------------------------------------------------


[[nodiscard]]
constexpr bool IsEol(wchar_t x) noexcept { return x == L'\r' || x == L'\n'; }

[[nodiscard]]
inline bool IsBlankOrEos(wchar_t x) noexcept { return !x || std::iswblank(x); }

[[nodiscard]]
string_view GetBlanks();

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
	void upper(std::span<wchar_t> Str);
	void lower(std::span<wchar_t> Str);

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

struct [[nodiscard]] string_comparer_icase
{
	using is_transparent = void;

	[[nodiscard]]
	size_t operator()(wchar_t Char) const;

	[[nodiscard]]
	size_t operator()(string_view Str) const;

	[[nodiscard]]
	bool operator()(wchar_t Chr1, wchar_t Chr2) const;

	[[nodiscard]]
	bool operator()(string_view Str1, string_view Str2) const;
};

using unordered_string_set_icase = std::unordered_set<string, string_comparer_icase, string_comparer_icase>;
using unordered_string_multiset_icase = std::unordered_multiset<string, string_comparer_icase, string_comparer_icase>;

template<typename T>
using unordered_string_map_icase = std::unordered_map<string, T, string_comparer_icase, string_comparer_icase>;

template<typename T>
using unordered_string_multimap_icase = std::unordered_multimap<string, T, string_comparer_icase, string_comparer_icase>;


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
};

class exact_searcher final: public i_searcher
{
public:
	NONCOPYABLE(exact_searcher);
	explicit exact_searcher(string_view Needle, bool CanReverse = true);
	std::optional<std::pair<size_t, size_t>> find_in(string_view Haystack, bool Reverse = {}) const override;

private:
	template<typename... args>
	using searcher = std::boyer_moore_horspool_searcher<args...>;

	string m_Needle;
	searcher<string::const_iterator> m_Searcher;
	std::optional<searcher<string::const_reverse_iterator>> m_ReverseSearcher;
};

class icase_searcher final: public i_searcher
{
public:
	NONCOPYABLE(icase_searcher);
	explicit icase_searcher(string_view Needle, bool CanReverse = true);
	std::optional<std::pair<size_t, size_t>> find_in(string_view Haystack, bool Reverse = {}) const override;

private:
	mutable string m_HayStack;
	exact_searcher m_Searcher;
};

namespace detail
{
	class fuzzy_searcher_impl
	{
	public:
		string_view normalize(string_view Str);
		std::optional<std::pair<size_t, size_t>> find_in(const i_searcher& searcher, string_view Haystack, bool Reverse);

	private:
		string m_Result, m_Intermediate;
		std::vector<WORD> m_Types;
	};
}

template<typename Searcher>
class fuzzy_searcher final: public i_searcher
{
public:
	NONCOPYABLE(fuzzy_searcher);

	explicit fuzzy_searcher(string_view const Needle, bool const CanReverse = true):
		m_Searcher(m_Impl.normalize(Needle), CanReverse)
	{
	}

	std::optional<std::pair<size_t, size_t>> find_in(string_view Haystack, bool Reverse = {}) const override
	{
		return m_Impl.find_in(m_Searcher, Haystack, Reverse);
	}

private:
	mutable detail::fuzzy_searcher_impl m_Impl;
	Searcher m_Searcher;
};

using fuzzy_cs_searcher = fuzzy_searcher<exact_searcher>;
using fuzzy_ic_searcher = fuzzy_searcher<icase_searcher>;

using searchers = std::variant
<
	std::monostate, // Just to make it default-constructible
	exact_searcher,
	icase_searcher,
	fuzzy_cs_searcher,
	fuzzy_ic_searcher
>;

inline i_searcher const& init_searcher(searchers& Searchers, const bool CaseSensitive, const bool Fuzzy, string_view const Needle, const bool CanReverse = true)
{
	if (Fuzzy)
	{
		if (CaseSensitive) return Searchers.emplace<fuzzy_cs_searcher>(Needle, CanReverse);
		else               return Searchers.emplace<fuzzy_ic_searcher>(Needle, CanReverse);
	}
	else
	{
		if (CaseSensitive) return Searchers.emplace<exact_searcher>(Needle, CanReverse);
		else               return Searchers.emplace<icase_searcher>(Needle, CanReverse);
	}
}

struct search_replace_string_options
{
	bool CaseSensitive{};
	bool WholeWords{};
	bool Reverse{};
	bool Regex{};
	bool PreserveStyle{};
};

#endif // STRING_UTILS_HPP_82ECD8BE_D484_4023_AB42_21D93B2DF8B9
