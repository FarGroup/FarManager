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

template<class T>
class matrix
{
public:
	class row
	{
	public:
		typedef T value_type;
		typedef T* iterator;
		typedef const T* const_iterator;
		typedef T& reference;
		typedef const T& const_reference;

		row(T* row, size_t size): m_row(row), m_size(size) {}

		size_t size() const { return m_size; }

		// assert for <= is ok, &row[size] can be used as an 'end' iterator
		reference operator[](size_t n) { assert(n <= m_size); return m_row[n]; }
		const_reference operator[](size_t n) const { assert(n <= m_size); return m_row[n]; }
		reference front() { assert(m_size != 0); return m_row[0]; }
		reference back() { assert(m_size != 0); return m_row[m_size - 1]; }

		T* data() { return m_row; }
		const T* data() const { return m_row; }

		iterator begin() { return m_row; }
		iterator end() { return m_row + m_size; }

		const_iterator cbegin() const { return m_row; }
		const_iterator cend() const { return m_row + m_size; }

		const_iterator begin() const { return m_row; }
		const_iterator end() const { return m_row + m_size; }

	private:
		T* m_row;
		size_t m_size;
	};

	matrix(): m_rows(), m_cols() {}
	matrix(size_t rows, size_t cols) { allocate(rows, cols); }

	void allocate(size_t rows, size_t cols)
	{
		m_rows = rows;
		m_cols = cols;

		// don't call vector.resize() here:
		// - it's never shrink
		// - we don't care about old content
		m_buffer = std::vector<T>(m_rows * m_cols);
	}

	// assert for <= is ok, &matirx[size] can be used as an 'end' iterator
	row operator[](size_t n) { assert(n <= m_rows); return row(m_buffer.data() + m_cols * n, m_cols); }
	const row operator[](size_t n) const { assert(n <= m_rows); return row(const_cast<T*>(m_buffer.data()) + m_cols * n, m_cols); }

	size_t height() const { return m_rows; }
	size_t width() const { return m_cols; }

	row front() { assert(m_rows != 0); return (*this)[0]; }
	row back() { assert(m_rows != 0); return (*this)[m_rows - 1]; }

	bool empty() const { return m_buffer.empty(); }
	size_t size() const { return m_buffer.size(); }

	T* data() { return m_buffer.data(); }
	const T* data() const { return m_buffer.data(); }

	// TODO: iterator interface

	std::vector<T>& vector() { return m_buffer; }
	const std::vector<T>& vector() const { return m_buffer; }

	void swap(matrix& rhs) noexcept
	{
		using std::swap;
		m_buffer.swap(rhs.m_buffer);
		swap(m_rows, rhs.m_rows);
		swap(m_cols, rhs.m_cols);
	}

	FREE_SWAP(matrix);

private:
	std::vector<T> m_buffer;
	size_t m_rows;
	size_t m_cols;
};
