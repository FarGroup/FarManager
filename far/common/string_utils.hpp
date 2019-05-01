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

//----------------------------------------------------------------------------

/*
Helper class to safely pass string_view to low level C or platform API.
Builds a compatible null-terminated std::basic_string if the given view is not null-terminated,
otherwise uses the same data.
*/
template<typename T>
class [[nodiscard]] null_terminated_t
{
public:
	NONCOPYABLE(null_terminated_t);

WARNING_PUSH()
WARNING_DISABLE_MSC(4582) // no page                                                'class': constructor is not implicitly called
WARNING_DISABLE_MSC(4583) // no page                                                'class': destructor is not implicitly called

	explicit null_terminated_t(const std::basic_string_view<T> Str)
	{
		if (Str.data() && !Str.data()[Str.size()])
		{
			m_Mode = mode::view;
			placement::construct(m_View, Str);
		}
		else if (Str.size() < std::size(m_Buffer))
		{
			m_Mode = mode::buffer;
			*std::copy(ALL_CONST_RANGE(Str), m_Buffer) = {};
		}
		else
		{
			m_Mode = mode::string;
			placement::construct(m_Str, ALL_CONST_RANGE(Str));
		}
	}

	~null_terminated_t()
	{
		switch(m_Mode)
		{
		case mode::view:
			placement::destruct(m_View);
			break;

		case mode::buffer:
			break;

		case mode::string:
			placement::destruct(m_Str);
			break;
		}
	}

WARNING_POP()

	[[nodiscard]]
	const T* c_str() const
	{
		switch (m_Mode)
		{
		case mode::view:
			return m_View.data();

		case mode::buffer:
			return m_Buffer;

		case mode::string:
			return m_Str.c_str();

		default:
			throw std::runtime_error("unknown mode"s);
		}
	}

	[[nodiscard]]
	bool empty() const
	{
		return !*c_str();
	}

private:
	union
	{
		std::basic_string_view<T> m_View;
		T m_Buffer[MAX_PATH];
		std::basic_string<T> m_Str;
	};

	enum class mode: char
	{
		view,
		buffer,
		string
	}
	m_Mode;
};

using null_terminated = null_terminated_t<wchar_t>;


namespace string_utils::detail
{
	// The overload for string literals is deliberately omitted as it also matches arrays
	// and while for string literals the length formula is pretty simple: N - 1,
	// for arrays it is not, as they could have no trailing \0 at all, or (worse) have multiple.
	// Use string_view literal if you need to.
	[[nodiscard]] inline size_t size_one(wchar_t) { return 1; }
	[[nodiscard]] inline size_t size_one(string_view const Str) { return Str.size(); }

	inline void append_one(string& Str, wchar_t const Arg, size_t) { Str += Arg; }
	inline void append_one(string& Str, wchar_t const* const Arg, size_t const Size) { Str.append(Arg, Size); }
	inline void append_one(string& Str, string const & Arg, size_t) { Str += Arg; }
	inline void append_one(string& Str, string_view const Arg, size_t) { Str += Arg; }

	template<size_t... I, typename... args>
	void append_all(string& Str, std::index_sequence<I...> Sequence, args const&... Args)
	{
		size_t Sizes[Sequence.size()];
		const auto TotalSize = (Str.size() + ... + (Sizes[I] = size_one(Args)));
		reserve_exp_noshrink(Str, TotalSize);
		(..., append_one(Str, Args, Sizes[I]));
	}
}

template<typename... args>
void append(string& Str, args const&... Args)
{
	string_utils::detail::append_all(Str, std::index_sequence_for<args...>{}, Args...);
}

template<typename... args>
[[nodiscard]]
auto concat(args const&... Args)
{
	string Str;
	append(Str, Args...);
	return Str;
}

namespace detail
{
	template<typename begin_iterator, typename end_iterator>
	[[nodiscard]]
	size_t get_space_count(begin_iterator Begin, end_iterator End)
	{
		return std::find_if_not(Begin, End, std::iswspace) - Begin;
	}
}

namespace inplace
{
	inline void cut_left(string& Str, size_t MaxWidth)
	{
		if (Str.size() > MaxWidth)
			Str.erase(0, Str.size() - MaxWidth);
	}

	inline void cut_right(string& Str, size_t MaxWidth)
	{
		if (Str.size() > MaxWidth)
			Str.resize(MaxWidth);
	}

	inline void pad_left(string& Str, size_t MinWidth, wchar_t Padding = L' ')
	{
		if (Str.size() < MinWidth)
			Str.insert(0, MinWidth - Str.size(), Padding);
	}

	inline void pad_right(string& Str, size_t MinWidth, wchar_t Padding = L' ')
	{
		if (Str.size() < MinWidth)
			Str.append(MinWidth - Str.size(), Padding);
	}

	inline void fit_to_left(string& Str, size_t Size)
	{
		Str.size() < Size? pad_right(Str, Size) : cut_right(Str, Size);
	}

	inline void fit_to_center(string& Str, size_t Size)
	{
		const auto StrSize = Str.size();

		if (Str.size() < Size)
		{
			pad_left(Str, StrSize + (Size - StrSize) / 2);
			pad_right(Str, Size);
		}
		else
		{
			cut_right(Str, Size);
		}
	}

	inline void fit_to_right(string& Str, size_t Size)
	{
		Str.size() < Size? pad_left(Str, Size) : cut_right(Str, Size);
	}

	inline void erase_all(string& Str, wchar_t Char)
	{
		Str.erase(std::remove(ALL_RANGE(Str), Char), Str.end());
	}

	inline void unquote(string& Str)
	{
		erase_all(Str, L'"');
	}

	inline void quote(string& Str)
	{
		if (Str.empty() || Str.front() != L'"')
			Str.insert(0, 1, L'"');

		if (Str.size() == 1 || Str.back() != L'"')
			Str.push_back(L'"');
	}

	inline void quote_unconditional(string& Str)
	{
		Str.insert(0, 1, L'"');
		Str.push_back(L'"');
	}

	inline void quote_normalise(string& Str)
	{
		unquote(Str);
		quote(Str);
	}

	inline void trim_left(string& Str)
	{
		Str.erase(0, detail::get_space_count(ALL_CONST_RANGE(Str)));
	}

	inline void trim_left(string_view& Str)
	{
		Str.remove_prefix(detail::get_space_count(ALL_CONST_RANGE(Str)));
	}

	inline void trim_right(string& Str)
	{
		Str.resize(Str.size() - detail::get_space_count(ALL_CONST_REVERSE_RANGE(Str)));
	}

	inline void trim_right(string_view& Str)
	{
		Str.remove_suffix(detail::get_space_count(ALL_CONST_REVERSE_RANGE(Str)));
	}

	inline void trim(string& Str)
	{
		trim_right(Str);
		trim_left(Str);
	}

	inline void trim(string_view& Str)
	{
		trim_right(Str);
		trim_left(Str);
	}
}

namespace copy
{
	template<typename iterator>
	void unquote(const string_view Str, const iterator Destination)
	{
		std::remove_copy(ALL_CONST_RANGE(Str), Destination, L'"');
	}
}

[[nodiscard]]
inline auto cut_left(string Str, size_t MaxWidth)
{
	inplace::cut_left(Str, MaxWidth);
	return Str;
}

[[nodiscard]]
inline auto cut_right(string Str, size_t MaxWidth)
{
	inplace::cut_right(Str, MaxWidth);
	return Str;
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
	inplace::pad_left(Str, MinWidth, Padding);
	return Str;
}

[[nodiscard]]
inline auto pad_right(string Str, size_t MinWidth, wchar_t Padding = L' ')
{
	inplace::pad_right(Str, MinWidth, Padding);
	return Str;
}

[[nodiscard]]
inline auto fit_to_left(string Str, size_t Size)
{
	inplace::fit_to_left(Str, Size);
	return Str;
}

[[nodiscard]]
inline auto fit_to_center(string Str, size_t Size)
{
	inplace::fit_to_center(Str, Size);
	return Str;
}

[[nodiscard]]
inline auto fit_to_right(string Str, size_t Size)
{
	inplace::fit_to_right(Str, Size);
	return Str;
}

[[nodiscard]]
inline auto erase_all(string Str, wchar_t Char)
{
	inplace::erase_all(Str, Char);
	return Str;
}

[[nodiscard]]
inline auto unquote(string Str)
{
	inplace::unquote(Str);
	return Str;
}

[[nodiscard]]
inline auto quote(string Str)
{
	inplace::quote(Str);
	return Str;
}

[[nodiscard]]
inline auto quote_unconditional(string Str)
{
	inplace::quote_unconditional(Str);
	return Str;
}

[[nodiscard]]
inline auto quote_normalise(string Str)
{
	inplace::quote_normalise(Str);
	return Str;
}

[[nodiscard]]
inline bool equal(const string_view Str1, const string_view Str2)
{
	return Str1 == Str2;
}

[[nodiscard]]
inline bool starts_with(const string_view Str, const string_view Prefix)
{
	return Str.size() >= Prefix.size() && Str.substr(0, Prefix.size()) == Prefix;
}

[[nodiscard]]
inline bool starts_with(const string_view Str, wchar_t const Prefix)
{
	return !Str.empty() && Str.front() == Prefix;
}

[[nodiscard]]
inline bool ends_with(const string_view Str, const string_view Suffix)
{
	return Str.size() >= Suffix.size() && Str.substr(Str.size() - Suffix.size()) == Suffix;
}

[[nodiscard]]
inline bool ends_with(const string_view Str, wchar_t const Suffix)
{
	return !Str.empty() && Str.back() == Suffix;
}

[[nodiscard]]
inline auto trim_left(string Str)
{
	inplace::trim_left(Str);
	return Str;
}

[[nodiscard]]
inline auto trim_left(string_view Str)
{
	inplace::trim_left(Str);
	return Str;
}

[[nodiscard]]
inline auto trim_right(string Str)
{
	inplace::trim_right(Str);
	return Str;
}

[[nodiscard]]
inline auto trim_right(string_view Str)
{
	inplace::trim_right(Str);
	return Str;
}

[[nodiscard]]
inline auto trim(string Str)
{
	inplace::trim(Str);
	return Str;
}

[[nodiscard]]
inline auto trim(string_view Str)
{
	inplace::trim(Str);
	return Str;
}

template<typename container>
void join(string& Str, const container& Container, string_view const Separator)
{
	bool First = true;

	for (const auto& i: Container)
	{
		if (First)
		{
			First = false;
			append(Str, i);
		}
		else
		{
			append(Str, Separator, i);
		}
	}
}

template<typename container>
[[nodiscard]]
string join(const container& Container, string_view const Separator)
{
	string Str;
	join(Str, Container, Separator);
	return Str;
}

// uniform "contains"
template<typename find_type, typename... traits>
[[nodiscard]]
bool contains(const std::basic_string<traits...>& Str, const find_type& What)
{
	return Str.find(What) != Str.npos;
}

template<typename find_type, typename... traits>
[[nodiscard]]
bool contains(const std::basic_string_view<traits...> Str, const find_type& What)
{
	return Str.find(What) != Str.npos;
}

[[nodiscard]]
inline bool contains(const wchar_t* const Str, const wchar_t* const What)
{
	return wcsstr(Str, What) != nullptr;
}

[[nodiscard]]
inline bool contains(const wchar_t* const Str, wchar_t const What)
{
	return wcschr(Str, What) != nullptr;
}

[[nodiscard]]
inline bool contains(const char* const Str, const char* const What)
{
	return strstr(Str, What) != nullptr;
}

[[nodiscard]]
inline bool contains(const char* const Str, char const What)
{
	return strchr(Str, What) != nullptr;
}

inline std::pair<string_view, string_view> split_name_value(string_view const Str)
{
	const auto SeparatorPos = Str.find(L'=');
	return { Str.substr(0, SeparatorPos), Str.substr(SeparatorPos + 1) };
}


// std::string_view is a drop-in replacement for const std::string& they say
template<typename T>
[[nodiscard]]
auto operator+(const std::basic_string<T>& Lhs, const std::basic_string_view<T> Rhs)
{
	return concat(Lhs, Rhs);
}

template<typename T>
[[nodiscard]]
auto operator+(const std::basic_string_view<T> Lhs, const std::basic_string<T>& Rhs)
{
	return concat(Lhs, Rhs);
}

template<typename T>
[[nodiscard]]
auto operator+(const std::basic_string_view<T> Lhs, std::basic_string_view<T> Rhs)
{
	return concat(Lhs, Rhs);
}


// string_view has iterators, but you cannot construct it from them
// "Design by committee" *facepalm*
template <typename T>
[[nodiscard]]
auto make_string_view(T const Begin, T const End)
{
	using char_type = typename std::iterator_traits<T>::value_type;
	static_assert(std::is_same_v<typename std::basic_string_view<char_type>::const_iterator, T>);

	const auto Size = static_cast<size_t>(End - Begin);
	return std::basic_string_view<char_type>{ Size ? &*Begin : nullptr, Size };
}

#endif // STRING_UTILS_HPP_DE39ECEB_2377_44CB_AF4B_FA5BEA09C8C8
