#ifndef STRING_UTILS_HPP_DE39ECEB_2377_44CB_AF4B_FA5BEA09C8C8
#define STRING_UTILS_HPP_DE39ECEB_2377_44CB_AF4B_FA5BEA09C8C8
#pragma once

/*
string_utils.hpp
*/
/*
Copyright © 2016 Far Group
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

#include "preprocessor.hpp"
#include "placement.hpp"

/*
Helper class to safely pass string_view to low level C or platform API.
Builds a compatible null-terminated std::basic_string if the given view is not null-terminated,
otherwise uses the same data.
*/
template<typename T>
class null_terminated_t
{
public:
	NONCOPYABLE(null_terminated_t);

WARNING_PUSH()
WARNING_DISABLE_MSC(4582) // no page                                                '%$S': constructor is not implicitly called
WARNING_DISABLE_MSC(4583) // no page                                                '%$S': destructor is not implicitly called

	explicit null_terminated_t(const basic_string_view<T>& Str):
		m_Terminated(!Str.raw_data()[Str.size()])
	{
		if (m_Terminated)
			placement::construct(m_View, Str);
		else
			placement::construct(m_Str, ALL_CONST_RANGE(Str));
	}

	~null_terminated_t()
	{
		if (m_Terminated)
			placement::destruct(m_View);
		else
			placement::destruct(m_Str);
	}

WARNING_POP()

	auto data() const
	{
		return m_Terminated? m_View.raw_data() : m_Str.data();
	}

	auto size() const
	{
		return m_Terminated? m_View.size() : m_Str.size();
	}

private:
	union
	{
		std::basic_string<T> m_Str;
		basic_string_view<T> m_View;
	};
	bool m_Terminated;
};

using null_terminated = null_terminated_t<wchar_t>;


namespace detail
{
	inline void append_one(string& Str, wchar_t Arg, size_t) { Str += Arg; }
	inline void append_one(string& Str, const wchar_t* Arg, size_t Size) { Str.append(Arg, Size); }
	inline void append_one(string& Str, const string& Arg, size_t) { Str += Arg; }
	inline void append_one(string& Str, const string_view& Arg, size_t) { Str.append(ALL_CONST_RANGE(Arg)); }

	inline void append_impl(string&, const size_t*) {}

	template<typename arg, typename... args>
	void append_impl(string& Str, const size_t* Sizes, arg&& Arg, args&&... Args)
	{
		append_one(Str, FWD(Arg), *Sizes);
		append_impl(Str, Sizes + 1, FWD(Args)...);
	}

	// The overload for string literals is deliberately omitted as it also matches arrays
	// and while for string literals the length formula is pretty simple: N - 1,
	// for arrays it is not, as they could have no trailing \0 at all, or (worse) have multiple.
	// Use string_view literal if you need to.
	inline size_t size_one(wchar_t) { return 1; }
	inline size_t size_one(const string_view& Str) { return Str.size(); }

	inline size_t size_impl(size_t*) { return 0; }

	template<typename arg, typename... args>
	size_t size_impl(size_t* Sizes, arg&& Arg, args&&... Args)
	{
		*Sizes = size_one(FWD(Arg));
		return *Sizes + size_impl(Sizes + 1, FWD(Args)...);
	}
}

template<typename... args>
void append(string& Str, args&&... Args)
{
	size_t Sizes[sizeof...(Args)];
	reserve_exp_noshrink(Str, Str.size() + detail::size_impl(Sizes, FWD(Args)...));
	detail::append_impl(Str, Sizes, FWD(Args)...);
}

template<typename... args>
[[nodiscard]]
auto concat(args&&... Args)
{
	string Str;
	append(Str, FWD(Args)...);
	return Str;
}

template<typename char_type>
void assign(std::basic_string<char_type>& Str, const basic_string_view<char_type>& View)
{
	Str.assign(ALL_CONST_RANGE(View));
}

namespace detail
{
	template<typename begin_iterator, typename end_iterator>
	size_t get_space_count(begin_iterator Begin, end_iterator End)
	{
		return std::find_if_not(Begin, End, std::iswspace) - Begin;
	}
}

namespace inplace
{
	inline auto& cut_left(string& Str, size_t MaxWidth)
	{
		if (Str.size() > MaxWidth)
			Str.erase(0, Str.size() - MaxWidth);
		return Str;
	}

	inline auto& cut_right(string& Str, size_t MaxWidth)
	{
		if (Str.size() > MaxWidth)
			Str.resize(MaxWidth);
		return Str;
	}

	inline auto& pad_left(string& Str, size_t MinWidth, wchar_t Padding = L' ')
	{
		if (Str.size() < MinWidth)
			Str.insert(0, MinWidth - Str.size(), Padding);
		return Str;
	}

	inline auto& pad_right(string& Str, size_t MinWidth, wchar_t Padding = L' ')
	{
		if (Str.size() < MinWidth)
			Str.append(MinWidth - Str.size(), Padding);
		return Str;
	}

	inline auto& fit_to_left(string& Str, size_t Size)
	{
		return Str.size() < Size? pad_right(Str, Size) : cut_right(Str, Size);
	}

	inline auto& fit_to_center(string& Str, size_t Size)
	{
		const auto StrSize = Str.size();
		return Str.size() < Size? pad_right(pad_left(Str, StrSize + (Size - StrSize) / 2), Size) : cut_right(Str, Size);
	}

	inline auto& fit_to_right(string& Str, size_t Size)
	{
		return Str.size() < Size? pad_left(Str, Size) : cut_right(Str, Size);
	}

	inline auto& unquote(string& Str)
	{
		Str.erase(std::remove(ALL_RANGE(Str), L'"'), Str.end());
		return Str;
	}

	inline auto& quote(string& Str)
	{
		if (Str.empty() || Str.front() != L'"')
			Str.insert(0, 1, L'"');

		if (Str.size() == 1 || Str.back() != L'"')
			Str.push_back(L'"');

		return Str;
	}

	inline auto& quote_unconditional(string& Str)
	{
		Str.insert(0, 1, L'"');
		Str.push_back(L'"');
		return Str;
	}

	inline auto& quote_normalise(string& Str)
	{
		return quote(unquote(Str));
	}

	inline auto& trim_left(string& Str)
	{
		Str.erase(0, detail::get_space_count(ALL_CONST_RANGE(Str)));
		return Str;
	}

	inline auto& trim_right(string& Str)
	{
		Str.resize(Str.size() - detail::get_space_count(ALL_CONST_REVERSE_RANGE(Str)));
		return Str;
	}

	inline auto& trim(string& Str)
	{
		return trim_left(trim_right(Str));
	}
}

namespace copy
{
	template<typename iterator>
	void unquote(const string_view& Str, iterator Destination)
	{
		std::remove_copy(ALL_CONST_RANGE(Str), Destination, L'"');
	}
}

[[nodiscard]]
inline auto cut_left(string Str, size_t MaxWidth)
{
	return inplace::cut_left(Str, MaxWidth);
}

[[nodiscard]]
inline auto cut_right(string Str, size_t MaxWidth)
{
	return inplace::cut_right(Str, MaxWidth);
}

[[nodiscard]]
inline auto cut_left(string_view Str, size_t MaxWidth)
{
	if (Str.size() > MaxWidth)
		Str.remove_prefix(Str.size() - MaxWidth);
	return Str;
}

[[nodiscard]]
inline auto cut_right(string_view Str, size_t MaxWidth)
{
	if (Str.size() > MaxWidth)
		Str.remove_suffix(Str.size() - MaxWidth);
	return Str;
}

[[nodiscard]]
inline auto pad_left(string Str, size_t MinWidth, wchar_t Padding = L' ')
{
	return inplace::pad_left(Str, MinWidth, Padding);
}

[[nodiscard]]
inline auto pad_right(string Str, size_t MinWidth, wchar_t Padding = L' ')
{
	return inplace::pad_right(Str, MinWidth, Padding);
}

[[nodiscard]]
inline auto fit_to_left(string Str, size_t Size)
{
	return inplace::fit_to_left(Str, Size);
}

[[nodiscard]]
inline auto fit_to_center(string Str, size_t Size)
{
	return inplace::fit_to_center(Str, Size);
}

[[nodiscard]]
inline auto fit_to_right(string Str, size_t Size)
{
	return inplace::fit_to_right(Str, Size);
}

[[nodiscard]]
inline auto unquote(string Str)
{
	return inplace::unquote(Str);
}

[[nodiscard]]
inline auto quote(string Str)
{
	return inplace::quote(Str);
}

[[nodiscard]]
inline auto quote_unconditional(string Str)
{
	return inplace::quote_unconditional(Str);
}

[[nodiscard]]
inline auto quote_normalise(string Str)
{
	return inplace::quote_normalise(Str);
}

[[nodiscard]]
inline bool equal(const string_view& Str1, const string_view& Str2)
{
	return Str1 == Str2;
}

[[nodiscard]]
inline bool starts_with(const string_view& Str, const string_view& Prefix)
{
	return Str.starts_with(Prefix);
}

[[nodiscard]]
inline bool ends_with(const string_view& Str, const string_view& Suffix)
{
	return Str.ends_with(Suffix);
}

[[nodiscard]]
inline bool contains(const string_view& Str, const string_view& Token)
{
	return std::search(ALL_CONST_RANGE(Str), ALL_CONST_RANGE(Token)) != Str.cend();
}

[[nodiscard]]
inline auto trim_left(string Str)
{
	return inplace::trim_left(Str);
}

[[nodiscard]]
inline auto trim_right(string Str)
{
	return inplace::trim_right(Str);
}

[[nodiscard]]
inline auto trim(string Str)
{
	return inplace::trim(Str);
}

[[nodiscard]]
inline auto trim_left(string_view Str)
{
	Str.remove_prefix(detail::get_space_count(ALL_CONST_RANGE(Str)));
	return Str;
}

[[nodiscard]]
inline auto trim_right(string_view Str)
{
	Str.remove_suffix(detail::get_space_count(ALL_CONST_REVERSE_RANGE(Str)));
	return Str;
}

[[nodiscard]]
inline auto trim(string_view Str)
{
	return trim_left(trim_right(Str));
}

#endif // STRING_UTILS_HPP_DE39ECEB_2377_44CB_AF4B_FA5BEA09C8C8
