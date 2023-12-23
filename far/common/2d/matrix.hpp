#ifndef MATRIX_HPP_FD448106_F9CF_43E3_8148_E9680D79AFB7
#define MATRIX_HPP_FD448106_F9CF_43E3_8148_E9680D79AFB7
#pragma once

/*
matrix.hpp

*/
/*
Copyright © 2014 Far Group
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

#include "../preprocessor.hpp"
#include "../span.hpp"

#include <algorithm>
#include <vector>

#include <cassert>

//----------------------------------------------------------------------------

namespace detail
{
	template<typename T>
	class matrix_row: public std::span<T>
	{
	public:
		using std::span<T>::span;

		[[nodiscard]]
		bool operator==(const matrix_row& rhs) const
		{
			return std::ranges::equal(*this, rhs);
		}
	};

	// GCC isn't smart enough to deduce this
	template<typename T>
	matrix_row(T*, size_t) -> matrix_row<T>;

	template<typename T>
	class matrix_iterator
	{
	public:
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using reference = T;
		using value_type = T;
		using pointer = T*;

		matrix_iterator() = default;

		explicit matrix_iterator(T* Data, size_t Width):
			m_Data(Data),
			m_Width(Width)
		{
		}

		[[nodiscard]]
		decltype(auto) operator*() { return matrix_row(m_Data, m_Width); }

		[[nodiscard]]
		decltype(auto) operator*() const { return matrix_row(m_Data, m_Width); }

		[[nodiscard]]
		auto operator->() { return &**this; }

		[[nodiscard]]
		auto operator->() const { return &**this; }

		auto& operator++() { m_Data += m_Width; return *this; }
		auto& operator--() { m_Data -= m_Width; return *this; }

		POSTFIX_OPS()

		auto& operator+=(size_t const n) { m_Data += n * m_Width; return *this; }
		auto& operator-=(size_t const n) { m_Data -= n * m_Width; return *this; }

		auto operator+(size_t const n) const { return matrix_iterator(m_Data + n * m_Width, m_Width); }
		auto operator-(size_t const n) const { return matrix_iterator(m_Data - n * m_Width, m_Width); }

		auto operator-(const matrix_iterator& rhs) const
		{
			assert(m_Width == rhs.m_Width);
			return m_Data - rhs.m_Data;
		}

		[[nodiscard]]
		bool operator==(const matrix_iterator&) const = default;

		[[nodiscard]]
		auto operator<(const matrix_iterator& rhs) const
		{
			assert(m_Width == rhs.m_Width);
			return m_Data < rhs.m_Data;
		}

	private:
		T* m_Data{};
		size_t m_Width{};
	};
}

template<class T>
class matrix_view
{
public:
	COPYABLE(matrix_view);
	MOVABLE(matrix_view);

	matrix_view() = default;
	matrix_view(T* Data, size_t const Rows, size_t const Cols):
		m_Data(Data),
		m_Rows(Rows),
		m_Cols(Cols)
	{
	}

	[[nodiscard]] auto begin()        { return detail::matrix_iterator(data(), m_Cols); }
	[[nodiscard]] auto end()          { return detail::matrix_iterator(data() + size(), m_Cols); }
	[[nodiscard]] auto begin()  const { return detail::matrix_iterator(data(), m_Cols); }
	[[nodiscard]] auto end()    const { return detail::matrix_iterator(data() + size(), m_Cols); }
	[[nodiscard]] auto cbegin() const { return detail::matrix_iterator(data(), m_Cols); }
	[[nodiscard]] auto cend()   const { return detail::matrix_iterator(data() + size(), m_Cols); }

	[[nodiscard]]
	// BUGBUG assert for <= is due to the fact that &row[size] can be used as an 'end' iterator
	// TODO: use iterators
	auto operator[](size_t const Index) { assert(Index <= m_Rows); return detail::matrix_row(m_Data + m_Cols * Index, m_Cols); }

	[[nodiscard]]
	// BUGBUG assert for <= is due to the fact that &row[size] can be used as an 'end' iterator
	// TODO: use iterators
	auto operator[](size_t const Index) const { assert(Index <= m_Rows); return detail::matrix_row(m_Data + m_Cols * Index, m_Cols); }

	const auto& at(size_t const Row, size_t const Col) const
	{
		assert(Row < m_Rows);
		assert(Col < m_Cols);
		return m_Data[m_Cols * Row + Col];
	}

	auto& at(size_t const Row, size_t const Col)
	{
		return const_cast<T&>(std::as_const(*this).at(Row, Col));
	}

	[[nodiscard]]
	auto height() const { return m_Rows; }

	[[nodiscard]]
	auto width() const { return m_Cols; }

	[[nodiscard]]
	auto front() { assert(m_Rows != 0); return (*this)[0]; }

	[[nodiscard]]
	auto back() { assert(m_Rows != 0); return (*this)[m_Rows - 1]; }

	[[nodiscard]]
	auto front() const { assert(m_Rows != 0); return (*this)[0]; }

	[[nodiscard]]
	auto back() const { assert(m_Rows != 0); return (*this)[m_Rows - 1]; }

	[[nodiscard]]
	auto empty() const { return !m_Rows || !m_Cols; }

	[[nodiscard]]
	auto size() const { return m_Rows * m_Cols; }

	[[nodiscard]]
	auto data() { return m_Data; }

	[[nodiscard]]
	auto data() const { return m_Data; }

private:
	T* m_Data{};
	size_t m_Rows{};
	size_t m_Cols{};
};

namespace detail
{
	template<class T>
	struct matrix_data
	{
		template<typename... args>
		explicit matrix_data(args&&... Args):
			m_Buffer(FWD(Args)...)
		{
		}

		std::vector<T> m_Buffer;
	};
}

template<class T>
class matrix: private detail::matrix_data<T>, public matrix_view<T>
{
public:
	matrix() = default;

	matrix(size_t const Rows, size_t const Cols)
	{
		allocate(Rows, Cols);
	}

	matrix(const matrix& rhs):
		detail::matrix_data<T>{ rhs.m_Buffer },
		matrix_view<T>(this->m_Buffer.data(), rhs.height(), rhs.width())
	{
	}

	template<typename Y> requires std::same_as<std::remove_const_t<Y>, T>
	explicit matrix(const matrix_view<Y>& rhs):
		detail::matrix_data<T>(rhs.data(), rhs.data() + rhs.size()),
		matrix_view<T>(this->m_Buffer.data(), rhs.height(), rhs.width())
	{
	}

	matrix(matrix&& rhs) noexcept:
		detail::matrix_data<T>{ std::move(rhs.m_Buffer) },
		matrix_view<T>(this->m_Buffer.data(), rhs.height(), rhs.width())
	{
	}

	COPY_ASSIGNABLE_SWAP(matrix)

	template<typename Y> requires std::same_as<std::remove_const_t<Y>, T>
	matrix& operator=(const matrix_view<Y>& rhs)
	{
		return *this = matrix<T>(rhs);
	}

	matrix& operator=(matrix&& rhs) noexcept
	{
		this->m_Buffer = std::move(rhs.m_Buffer);
		static_cast<matrix_view<T>&>(*this) = matrix_view<T>(this->m_Buffer.data(), rhs.height(), rhs.width());
		return *this;
	}

	void allocate(size_t const Rows, size_t const Cols)
	{
		// Force memory release
		clear_and_shrink(this->m_Buffer);
		this->m_Buffer.resize(Rows * Cols);
		static_cast<matrix_view<T>&>(*this) = matrix_view<T>(this->m_Buffer.data(), Rows, Cols);
	}

	[[nodiscard]]
	auto& vector() { return this->m_Buffer; }

	[[nodiscard]]
	auto& vector() const { return this->m_Buffer; }
};

template<typename T>
matrix(const matrix_view<T>&) -> matrix<std::remove_const_t<T>>;

#endif // MATRIX_HPP_FD448106_F9CF_43E3_8148_E9680D79AFB7
