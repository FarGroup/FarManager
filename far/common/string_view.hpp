#ifndef STRING_VIEW_HPP_102EA19D_CDD6_433E_ACD2_6D6E4022C273
#define STRING_VIEW_HPP_102EA19D_CDD6_433E_ACD2_6D6E4022C273
#pragma once

/*
string_view.hpp
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

#include "range.hpp"

namespace string_view_impl
{

template<typename T>
class basic_string_view : public range<const T*>
{
public:
	using const_iterator = const T*;
	using iterator = const_iterator;

	constexpr basic_string_view() = default;
	constexpr basic_string_view(const basic_string_view&) = default;

	constexpr basic_string_view(const T* Str, size_t Size) :
		range<const T*>(Str, Size)
	{
	}

	constexpr basic_string_view(const T* Str) :
		basic_string_view(Str, std::char_traits<T>::length(Str))
	{
	}

	constexpr basic_string_view(const std::basic_string<T>& Str) :
		basic_string_view(Str.data(), Str.size())
	{
	}

	explicit operator std::basic_string<T>() const
	{
		return { ALL_CONST_RANGE(*this) };
	}

	static constexpr size_t npos = size_t(-1);

	constexpr basic_string_view substr(size_t Pos = 0, size_t Count = npos) const
	{
		assert(Pos <= this->size());
		return { this->data() + Pos, std::min(Count, this->size() - Pos) };
	}

	constexpr const auto& front() const
	{
		assert(!this->empty());
		return *this->cbegin();
	}

	constexpr const auto& back() const
	{
		assert(!this->empty());
		return *std::prev(this->cend());
	}

	/*constexpr*/ void remove_prefix(size_t Size)
	{
		assert(Size <= this->size());
		*this = substr(Size);
	}

	/*constexpr*/ void remove_suffix(size_t Size)
	{
		assert(Size <= this->size());
		*this = substr(0, this->size() - Size);
	}

/*
	// C++20
	constexpr bool starts_with(const basic_string_view<T> Str) const noexcept
	{
		return this->size() >= Str.size() && this->substr(0, Str.size()) == Str;
	}

	constexpr bool starts_with(wchar_t const Char) const noexcept
	{
		return !this->empty() && this->front() == Char;
	}

	constexpr bool ends_with(const basic_string_view<T> Str) const noexcept
	{
		return this->size() >= Str.size() && this->substr(this->size() - Str.size()) == Str;
	}

	constexpr bool ends_with(wchar_t const Char) const noexcept
	{
		return !this->empty() && this->back() == Char;
	}
*/

	/*constexpr*/ size_t find(const basic_string_view<T> Str, const size_t Pos = 0) const noexcept
	{
		if (Pos >= this->size())
			return npos;

		const auto Result = std::search(this->cbegin() + Pos, this->cend(), ALL_CONST_RANGE(Str));
		return Result == this->cend()? npos : Result - this->begin();
	}

	/*constexpr*/ size_t find(T Char, size_t Pos = 0) const noexcept
	{
		return find({&Char, 1}, Pos);
	}

	/*constexpr*/ size_t find_first_of(const basic_string_view<T> Str, const size_t Pos = 0) const noexcept
	{
		if (Str.empty() || Pos >= this->size())
			return npos;

		for (auto Iterator = this->begin() + Pos; Iterator != this->end(); ++Iterator)
		{
			if (Str.find(*Iterator) != npos)
				return Iterator - this->begin();
		}

		return npos;
	}

	/*constexpr*/ size_t find_first_of(T Char, const size_t Pos = 0) const noexcept
	{
		return find_first_of({ &Char, 1 }, Pos);
	}

	/*constexpr*/ size_t find_first_not_of(const basic_string_view<T> Str, const size_t Pos = 0) const noexcept
	{
		if (Str.empty() || Pos >= this->size())
			return npos;

		for (auto Iterator = this->begin() + Pos; Iterator != this->end(); ++Iterator)
		{
			if (Str.find(*Iterator) == npos)
				return Iterator - this->begin();
		}

		return npos;
	}

	/*constexpr*/ size_t find_first_not_of(T Char, const size_t Pos = 0) const noexcept
	{
		return find_first_not_of({ &Char, 1 }, Pos);
	}

	/*constexpr*/ size_t rfind(T Char, const size_t Pos = npos) const noexcept
	{
		return find_last_of(Char, Pos);
	}

	/*constexpr*/ size_t find_last_of(const basic_string_view<T> Str, size_t Pos = npos) const noexcept
	{
		if (Str.empty())
			return npos;

		for (auto Iterator = this->begin() + (Pos < this->size()? Pos : this->size() - 1); ; --Iterator)
		{
			if (Str.find(*Iterator) != npos)
				return Iterator - this->begin();

			if (Iterator == this->begin())
				break;
		}

		return npos;
	}

	/*constexpr*/ size_t find_last_of(T Char, const size_t Pos = npos) const noexcept
	{
		return find_last_of({ &Char, 1 }, Pos);
	}

	/*constexpr*/ size_t find_last_not_of(const basic_string_view<T> Str, size_t Pos = npos) const noexcept
	{
		if (Str.empty())
			return npos;

		for (auto Iterator = this->begin() + (Pos < this->size()? Pos : this->size() - 1); ; --Iterator)
		{
			if (Str.find(*Iterator) == npos)
				return Iterator - this->begin();

			if (Iterator == this->begin())
				break;
		}

		return npos;
	}

	/*constexpr*/ size_t find_last_not_of(T Char, const size_t Pos = npos) const noexcept
	{
		return find_last_not_of({ &Char, 1 }, Pos);
	}
};

namespace string_view_literals
{
WARNING_PUSH()
WARNING_DISABLE_MSC(4455) // no page                                                'operator ""sv': literal suffix identifiers that do not start with an underscore are reserved

constexpr auto operator ""sv(const char* Data, size_t Size) noexcept { return basic_string_view<char>(Data, Size); }
constexpr auto operator ""sv(const wchar_t* Data, size_t Size) noexcept { return basic_string_view<wchar_t>(Data, Size); }

WARNING_POP()
}

template<typename T>
bool operator==(const basic_string_view<T> Lhs, const basic_string_view<T> Rhs)
{
	return std::equal(ALL_CONST_RANGE(Lhs), ALL_CONST_RANGE(Rhs));
}

template<typename T>
bool operator==(const basic_string_view<T> Lhs, const std::basic_string<T>& Rhs)
{
	return Lhs == basic_string_view<T>(Rhs);
}

template<typename T>
bool operator==(const std::basic_string<T>& Lhs, const basic_string_view<T> Rhs)
{
	return basic_string_view<T>(Lhs) == Rhs;
}

template<typename T>
bool operator!=(const basic_string_view<T> Lhs, const basic_string_view<T> Rhs)
{
	return !(Lhs == Rhs);
}

template<typename T>
bool operator!=(const basic_string_view<T> Lhs, const std::basic_string<T>& Rhs)
{
	return !(Lhs == Rhs);
}

template<typename T>
bool operator!=(const std::basic_string<T>& Lhs, const basic_string_view<T> Rhs)
{
	return !(Lhs == Rhs);
}

template<typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& Stream, const basic_string_view<T> Str)
{
	return Stream.write(Str.data(), Str.size());
}

using string_view = basic_string_view<char>;
using wstring_view = basic_string_view<wchar_t>;

} // string_view_impl

#endif // STRING_VIEW_HPP_102EA19D_CDD6_433E_ACD2_6D6E4022C273
