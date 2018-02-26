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

// TODO: use std::wstring_view

template<typename T>
class basic_string_view : public range<const T*>
{
public:
	constexpr basic_string_view() = default;
	constexpr basic_string_view(const basic_string_view&) = default;

	constexpr basic_string_view(const T* Str, size_t Size) :
		range<const T*>(Str, Size)
	{
	}

	constexpr basic_string_view(const T* Str) :
		basic_string_view(Str, length(Str))
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
		return { this->raw_data() + Pos, std::min(Count, this->size() - Pos) };
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

	constexpr bool starts_with(const basic_string_view<T>& Str) const noexcept
	{
		return this->size() >= Str.size() && this->substr(0, Str.size()) == Str;
	}

	constexpr bool ends_with(const basic_string_view<T>& Str) const noexcept
	{
		return this->size() >= Str.size() && this->substr(this->size() - Str.size()) == Str;
	}

	/*constexpr*/ size_t find(const basic_string_view<T>& Str, size_t Pos = 0) const noexcept
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

	/*
	ISO/IEC N4659 24.4.2.4 771
	"Note: Unlike basic_string::data() and string literals, data() may return a pointer to a buffer that
	is not null-terminated. Therefore it is typically a mistake to pass data() to a function that takes just a
	const charT* and expects a null-terminated string."

	- Another splendid design decision from the committee.
	If it's "typically a mistake", why didn't you give some other, "less-similar-to-basic_string::data()" name?

	For now, our implementation intentionally does not provide data() member function -
	we heavily rely on the platform API which requires C strings in about 99% of the cases and it's too easy to make a mistake
	and "rescue the Princess, her dog, her entire wardrobe & everything she has ever eaten...".
	Hopefully we will thin out C strings numbers enough by the time we switch to a C++17-conformant compiler.
	*/
private:
	constexpr auto data() const
	{
		return range<const T*>::data();
	}

	template<typename container>
	friend constexpr auto std::data(const container& Container) -> decltype(Container.data());

public:
	constexpr auto raw_data() const
	{
		return data();
	}

private:
	static auto length(const char* Str) { return strlen(Str); }
	static auto length(const wchar_t* Str) { return wcslen(Str); }
};

constexpr auto operator "" _sv(const char* Data, size_t Size) noexcept { return basic_string_view<char>(Data, Size); }
constexpr auto operator "" _sv(const wchar_t* Data, size_t Size) noexcept { return basic_string_view<wchar_t>(Data, Size); }

template<typename T>
auto operator+(const std::basic_string<T>& Lhs, const basic_string_view<T>& Rhs)
{
	return concat(Lhs, Rhs);
}

template<typename T>
auto operator+(const basic_string_view<T>& Lhs, const std::basic_string<T>& Rhs)
{
	return concat(Lhs, Rhs);
}

template<typename T>
auto operator+(const basic_string_view<T>& Lhs, const basic_string_view<T>& Rhs)
{
	return concat(Lhs, Rhs);
}

template<typename T>
bool operator==(const basic_string_view<T>& Lhs, const basic_string_view<T>& Rhs)
{
	return std::equal(ALL_CONST_RANGE(Lhs), ALL_CONST_RANGE(Rhs));
}

template<typename T>
bool operator==(const basic_string_view<T>& Lhs, const std::basic_string<T>& Rhs)
{
	return Lhs == basic_string_view<T>(Rhs);
}

template<typename T>
bool operator==(const std::basic_string<T>& Lhs, const basic_string_view<T>& Rhs)
{
	return basic_string_view<T>(Lhs) == Rhs;
}

template<typename T>
bool operator!=(const basic_string_view<T>& Lhs, const basic_string_view<T>& Rhs)
{
	return !(Lhs == Rhs);
}

template<typename T>
bool operator!=(const basic_string_view<T>& Lhs, const std::basic_string<T>& Rhs)
{
	return !(Lhs == Rhs);
}

template<typename T>
bool operator!=(const std::basic_string<T>& Lhs, const basic_string_view<T>& Rhs)
{
	return !(Lhs == Rhs);
}

template<typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& Stream, const basic_string_view<T>& Str)
{
	return Stream.write(Str.raw_data(), Str.size());
}

using string_view = basic_string_view<wchar_t>;

#endif // STRING_VIEW_HPP_102EA19D_CDD6_433E_ACD2_6D6E4022C273
