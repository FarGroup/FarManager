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

#include "../rel_ops.hpp"

template<class T>
class matrix
{
public:
	COPYABLE(matrix);
	MOVABLE(matrix);

	template<class pointer>
	class row_t: public rel_ops<row_t<pointer>>
	{
	public:
		using iterator = pointer;
		using value_type = typename std::iterator_traits<iterator>::value_type;
		using reference = typename std::iterator_traits<iterator>::reference;

		row_t(iterator Row, size_t Size): m_row(Row), m_size(Size) {}

		auto size() const { return m_size; }

		// assert for <= is ok, &row[size] can be used as an 'end' iterator
		decltype(auto) operator[](size_t n) { assert(n <= m_size); return m_row[n]; }
		decltype(auto) operator[](size_t n) const { assert(n <= m_size); return m_row[n]; }
		decltype(auto) front() const { assert(m_size != 0); return m_row[0]; }
		decltype(auto) back() const { assert(m_size != 0); return m_row[m_size - 1]; }

		auto data() { return m_row; }
		auto data() const { return m_row; }

		auto begin() { return m_row; }
		auto end() { return m_row + m_size; }

		auto begin() const { return m_row; }
		auto end() const { return m_row + m_size; }

		auto cbegin() const { return m_row; }
		auto cend() const { return m_row + m_size; }

		bool operator==(const row_t& rhs) const { return m_size == rhs.m_size && std::equal(m_row, m_row + m_size, rhs.m_row); }

	private:
		iterator m_row;
		size_t m_size;
	};

	using row = row_t<T*>;
	using const_row = row_t<const T*>;

	matrix(): m_rows(), m_cols() {}
	matrix(size_t rows, size_t cols) { allocate(rows, cols); }

	void allocate(size_t rows, size_t cols)
	{
		m_rows = rows;
		m_cols = cols;

		// Force memory release
		clear_and_shrink(m_buffer);
		m_buffer.resize(m_rows * m_cols);
	}

	// assert for <= is ok, &matirx[size] can be used as an 'end' iterator
	auto operator[](size_t n) { assert(n <= m_rows); return row(m_buffer.data() + m_cols * n, m_cols); }
	auto operator[](size_t n) const { assert(n <= m_rows); return const_row(m_buffer.data() + m_cols * n, m_cols); }

	auto height() const { return m_rows; }
	auto width() const { return m_cols; }

	auto front() { assert(m_rows != 0); return (*this)[0]; }
	auto back() { assert(m_rows != 0); return (*this)[m_rows - 1]; }

	auto front() const { assert(m_rows != 0); return (*this)[0]; }
	auto back() const { assert(m_rows != 0); return (*this)[m_rows - 1]; }

	auto empty() const { return m_buffer.empty(); }
	auto size() const { return m_buffer.size(); }

	auto data() { return m_buffer.data(); }
	auto data() const { return m_buffer.data(); }

	// TODO: iterator interface

	auto& vector() { return m_buffer; }
	auto& vector() const { return m_buffer; }

private:
	std::vector<T> m_buffer;
	size_t m_rows;
	size_t m_cols;
};

#endif // MATRIX_HPP_FD448106_F9CF_43E3_8148_E9680D79AFB7
