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

#include "compiler.hpp"
#include "preprocessor.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

//----------------------------------------------------------------------------

template<typename string_type>
auto copy_string(string_type const& Str, typename string_type::value_type* Destination)
{
	return Destination + Str.copy(Destination, Str.npos);
}

/*
Helper class to safely pass string_view to low level C or platform API.
Builds a compatible null-terminated string if the given view is not null-terminated,
otherwise uses the same data.
*/
template<typename T>
class [[nodiscard]] null_terminated_t
{
public:
	NONCOPYABLE(null_terminated_t);

	explicit null_terminated_t(const std::basic_string_view<T> Str)
	{
		if (Str.data() && !Str.data()[Str.size()])
		{
			m_Str.template emplace<view_type>(Str);
		}
		else if (Str.size() < std::tuple_size_v<buffer_type>)
		{
			*copy_string(Str, m_Str.template emplace<buffer_type>().data()) = {};
		}
		else
		{
			m_Str.template emplace<string_type>(Str);
		}
	}

	[[nodiscard]]
	const T* c_str() const noexcept
	{
		return std::visit([](const auto& Str){ return Str.data(); }, m_Str);
	}

	[[nodiscard]]
	bool empty() const
	{
		return !*c_str();
	}

private:
	using view_type = std::basic_string_view<T>;
	using buffer_type = std::array<T, MAX_PATH>;
	using string_type = std::basic_string<T>;

	std::variant<view_type, buffer_type, string_type> m_Str;
};

using null_terminated = null_terminated_t<wchar_t>;


class string_copyref
{
public:
	string_copyref(std::wstring_view Str) noexcept:
		m_Str(Str)
	{
	}

	string_copyref(std::wstring&& Str) noexcept:
		m_StrBuffer(std::move(Str)),
		m_Str(m_StrBuffer)
	{
	}

	operator std::wstring_view() const noexcept
	{
		return m_Str;
	}

private:
	std::wstring m_StrBuffer;
	std::wstring_view m_Str;
};


namespace string_utils::detail
{
	class append_arg: public std::wstring_view
	{
	public:
		explicit append_arg(std::wstring_view const Str) noexcept:
			std::wstring_view(Str)
		{
		}

		explicit append_arg(const wchar_t& Char) noexcept:
			std::wstring_view(&Char, 1)
		{
		}
	};

	inline void append_impl(std::wstring& Str, const std::initializer_list<append_arg>& Args)
	{
		const auto TotalSize = std::accumulate(ALL_RANGE(Args), size_t{}, [](size_t const Value, const append_arg& Element)
		{
			return Value + Element.size();
		});

		reserve_exp_noshrink(Str, TotalSize);
		for (const auto& i: Args)
		{
			Str += i;
		}
	}
}

template<typename... args>
void append(std::wstring& Str, args const&... Args)
{
	string_utils::detail::append_impl(Str, { string_utils::detail::append_arg(Args)... });
}

template<typename... args>
[[nodiscard]]
auto concat(args const&... Args)
{
	static_assert(sizeof...(Args) > 1);

	std::wstring Str;
	string_utils::detail::append_impl(Str, { string_utils::detail::append_arg(Args)... });
	return Str;
}

// uniform "contains"
template<typename find_type, typename... traits>
[[nodiscard]]
bool contains(const std::basic_string<traits...>& Str, const find_type& What) noexcept
{
	return Str.find(What) != Str.npos;
}

template<typename find_type, typename... traits>
[[nodiscard]]
bool contains(const std::basic_string_view<traits...> Str, const find_type& What) noexcept
{
	return Str.find(What) != Str.npos;
}

namespace detail
{
	template<typename raw_string_type>
	using char_type = std::remove_const_t<std::remove_pointer_t<raw_string_type>>;

	template<typename raw_string_type>
	inline constexpr bool is_supported_type = std::conjunction_v<
		std::is_pointer<raw_string_type>,
		is_one_of<char_type<raw_string_type>, wchar_t, char>
	>;
}

template<typename raw_string_type, REQUIRES(detail::is_supported_type<raw_string_type>)>
[[nodiscard]]
bool contains(raw_string_type const& Str, raw_string_type const& What)
{
	if constexpr (std::is_same_v<detail::char_type<raw_string_type>, wchar_t>)
		return wcsstr(Str, What) != nullptr;
	else
		return strstr(Str, What) != nullptr;
}

template<typename raw_string_type, REQUIRES(detail::is_supported_type<raw_string_type>)>
[[nodiscard]]
bool contains(raw_string_type const& Str, detail::char_type<raw_string_type> const What)
{
	if constexpr (std::is_same_v<detail::char_type<raw_string_type>, wchar_t>)
		return wcschr(Str, What) != nullptr;
	else
		return strchr(Str, What) != nullptr;
}

namespace detail
{
	template<typename begin_iterator, typename end_iterator>
	[[nodiscard]]
	size_t get_space_count(begin_iterator Begin, end_iterator End) noexcept
	{
		return std::find_if_not(Begin, End, std::iswspace) - Begin;
	}
}

namespace inplace
{
	inline void cut_left(std::wstring& Str, size_t MaxWidth)
	{
		if (Str.size() > MaxWidth)
			Str.erase(0, Str.size() - MaxWidth);
	}

	inline void cut_right(std::wstring& Str, size_t MaxWidth)
	{
		if (Str.size() > MaxWidth)
			Str.resize(MaxWidth);
	}

	inline void pad_left(std::wstring& Str, size_t MinWidth, wchar_t Padding = L' ')
	{
		if (Str.size() < MinWidth)
			Str.insert(0, MinWidth - Str.size(), Padding);
	}

	inline void pad_right(std::wstring& Str, size_t MinWidth, wchar_t Padding = L' ')
	{
		if (Str.size() < MinWidth)
			Str.append(MinWidth - Str.size(), Padding);
	}

	inline void fit_to_left(std::wstring& Str, size_t Size)
	{
		Str.size() < Size? pad_right(Str, Size) : cut_right(Str, Size);
	}

	inline void fit_to_center(std::wstring& Str, size_t Size)
	{
		const auto StrSize = Str.size();

		if (StrSize < Size)
		{
			pad_left(Str, StrSize + (Size - StrSize) / 2);
			pad_right(Str, Size);
		}
		else
		{
			cut_right(Str, Size);
		}
	}

	inline void fit_to_right(std::wstring& Str, size_t Size)
	{
		Str.size() < Size? pad_left(Str, Size) : cut_right(Str, Size);
	}

	inline void erase_all(std::wstring& Str, wchar_t Char)
	{
		Str.erase(std::remove(ALL_RANGE(Str), Char), Str.end());
	}

	inline void unquote(std::wstring& Str)
	{
		erase_all(Str, L'"');
	}

	inline void quote(std::wstring& Str)
	{
		if (Str.empty() || Str.front() != L'"')
			Str.insert(0, 1, L'"');

		if (Str.size() == 1 || Str.back() != L'"')
			Str.push_back(L'"');
	}

	inline void quote_unconditional(std::wstring& Str)
	{
		Str.insert(0, 1, L'"');
		Str.push_back(L'"');
	}

	inline void quote_normalise(std::wstring& Str)
	{
		unquote(Str);
		quote(Str);
	}

	inline void quote_space(std::wstring& Str)
	{
		if (contains(Str, L' '))
			quote(Str);
	}

	inline void trim_left(std::wstring& Str)
	{
		Str.erase(0, detail::get_space_count(ALL_CONST_RANGE(Str)));
	}

	inline void trim_left(std::wstring_view& Str) noexcept
	{
		Str.remove_prefix(detail::get_space_count(ALL_CONST_RANGE(Str)));
	}

	inline void trim_right(std::wstring& Str)
	{
		Str.resize(Str.size() - detail::get_space_count(ALL_CONST_REVERSE_RANGE(Str)));
	}

	inline void trim_right(std::wstring_view& Str) noexcept
	{
		Str.remove_suffix(detail::get_space_count(ALL_CONST_REVERSE_RANGE(Str)));
	}

	inline void trim(std::wstring& Str)
	{
		trim_right(Str);
		trim_left(Str);
	}

	inline void trim(std::wstring_view& Str) noexcept
	{
		trim_right(Str);
		trim_left(Str);
	}
}

namespace copy
{
	template<typename iterator>
	void unquote(const std::wstring_view Str, const iterator Destination)
	{
		std::remove_copy(ALL_CONST_RANGE(Str), Destination, L'"');
	}
}

[[nodiscard]]
inline auto cut_left(std::wstring Str, size_t MaxWidth)
{
	inplace::cut_left(Str, MaxWidth);
	return Str;
}

[[nodiscard]]
inline auto cut_right(std::wstring Str, size_t MaxWidth)
{
	inplace::cut_right(Str, MaxWidth);
	return Str;
}

[[nodiscard]]
inline auto cut_left(std::wstring_view Str, size_t MaxWidth) noexcept
{
	if (Str.size() > MaxWidth)
		Str.remove_prefix(Str.size() - MaxWidth);
	return Str;
}

[[nodiscard]]
inline auto cut_right(std::wstring_view Str, size_t MaxWidth) noexcept
{
	if (Str.size() > MaxWidth)
		Str.remove_suffix(Str.size() - MaxWidth);
	return Str;
}

[[nodiscard]]
inline auto pad_left(std::wstring Str, size_t MinWidth, wchar_t Padding = L' ')
{
	inplace::pad_left(Str, MinWidth, Padding);
	return Str;
}

[[nodiscard]]
inline auto pad_right(std::wstring Str, size_t MinWidth, wchar_t Padding = L' ')
{
	inplace::pad_right(Str, MinWidth, Padding);
	return Str;
}

[[nodiscard]]
inline auto fit_to_left(std::wstring Str, size_t Size)
{
	inplace::fit_to_left(Str, Size);
	return Str;
}

[[nodiscard]]
inline auto fit_to_center(std::wstring Str, size_t Size)
{
	inplace::fit_to_center(Str, Size);
	return Str;
}

[[nodiscard]]
inline auto fit_to_right(std::wstring Str, size_t Size)
{
	inplace::fit_to_right(Str, Size);
	return Str;
}

[[nodiscard]]
inline auto erase_all(std::wstring Str, wchar_t Char)
{
	inplace::erase_all(Str, Char);
	return Str;
}

[[nodiscard]]
inline auto unquote(std::wstring Str)
{
	inplace::unquote(Str);
	return Str;
}

[[nodiscard]]
inline auto unquote(std::wstring_view const Str)
{
	return unquote(std::wstring(Str));
}

[[nodiscard]]
inline auto quote(std::wstring Str)
{
	inplace::quote(Str);
	return Str;
}

[[nodiscard]]
inline auto quote(std::wstring_view const Str)
{
	return quote(std::wstring(Str));
}

[[nodiscard]]
inline auto quote_unconditional(std::wstring Str)
{
	inplace::quote_unconditional(Str);
	return Str;
}

[[nodiscard]]
inline auto quote_unconditional(std::wstring_view const Str)
{
	return quote_unconditional(std::wstring(Str));
}

[[nodiscard]]
inline auto quote_normalise(std::wstring Str)
{
	inplace::quote_normalise(Str);
	return Str;
}

[[nodiscard]]
inline auto quote_normalise(std::wstring_view const Str)
{
	return quote_normalise(std::wstring(Str));
}

[[nodiscard]]
inline auto quote_space(std::wstring Str)
{
	inplace::quote_space(Str);
	return Str;
}

[[nodiscard]]
inline auto quote_space(std::wstring_view const Str)
{
	return quote_space(std::wstring(Str));
}

[[nodiscard]]
inline bool equal(const std::wstring_view Str1, const std::wstring_view Str2) noexcept
{
	return Str1 == Str2;
}

[[nodiscard]]
inline bool starts_with(const std::wstring_view Str, const std::wstring_view Prefix) noexcept
{
	return Str.size() >= Prefix.size() && Str.substr(0, Prefix.size()) == Prefix;
}

[[nodiscard]]
inline bool starts_with(const std::wstring_view Str, wchar_t const Prefix) noexcept
{
	return !Str.empty() && Str.front() == Prefix;
}

[[nodiscard]]
inline bool ends_with(const std::wstring_view Str, const std::wstring_view Suffix) noexcept
{
	return Str.size() >= Suffix.size() && Str.substr(Str.size() - Suffix.size()) == Suffix;
}

[[nodiscard]]
inline bool ends_with(const std::wstring_view Str, wchar_t const Suffix) noexcept
{
	return !Str.empty() && Str.back() == Suffix;
}

[[nodiscard]]
inline auto trim_left(std::wstring Str)
{
	inplace::trim_left(Str);
	return Str;
}

[[nodiscard]]
inline auto trim_left(std::wstring_view Str) noexcept
{
	inplace::trim_left(Str);
	return Str;
}

[[nodiscard]]
inline auto trim_right(std::wstring Str)
{
	inplace::trim_right(Str);
	return Str;
}

[[nodiscard]]
inline auto trim_right(std::wstring_view Str) noexcept
{
	inplace::trim_right(Str);
	return Str;
}

[[nodiscard]]
inline auto trim(std::wstring Str)
{
	inplace::trim(Str);
	return Str;
}

[[nodiscard]]
inline auto trim(std::wstring_view Str) noexcept
{
	inplace::trim(Str);
	return Str;
}

template<typename container>
void join(std::wstring& Str, const container& Container, std::wstring_view const Separator)
{
	const auto Size = std::accumulate(ALL_CONST_RANGE(Container), size_t{}, [Separator](size_t const Value, const auto& Element)
	{
		return Value + Separator.size() + string_utils::detail::append_arg(Element).size();
	});

	reserve_exp_noshrink(Str, Size? Size - Separator.size() : Size);

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
std::wstring join(const container& Container, std::wstring_view const Separator)
{
	std::wstring Str;
	join(Str, Container, Separator);
	return Str;
}

[[nodiscard]]
inline std::pair<std::wstring_view, std::wstring_view> split(std::wstring_view const Str, wchar_t const Separator = L'=') noexcept
{
	const auto SeparatorPos = Str.find(Separator);
	return { Str.substr(0, SeparatorPos), Str.substr(SeparatorPos == Str.npos? Str.size() : SeparatorPos + 1) };
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

template<typename T>
[[nodiscard]]
auto operator+(const std::basic_string_view<T> Lhs, T const Rhs)
{
	return concat(Lhs, Rhs);
}

template<typename T>
[[nodiscard]]
auto operator+(T const Lhs, std::basic_string_view<T> Rhs)
{
	return concat(Lhs, Rhs);
}


// string_view has iterators, but you cannot construct it from them
// "Design by committee" *facepalm*
template <typename T>
[[nodiscard]]
constexpr auto make_string_view(T const Begin, T const End) noexcept
{
	using char_type = typename std::iterator_traits<T>::value_type;
	static_assert(std::is_same_v<typename std::basic_string_view<char_type>::const_iterator, T>);

	const auto Size = static_cast<size_t>(End - Begin);
	return std::basic_string_view<char_type>{ Size ? &*Begin : nullptr, Size };
}

#endif // STRING_UTILS_HPP_DE39ECEB_2377_44CB_AF4B_FA5BEA09C8C8
