#ifndef RANGE_HPP_3B87674F_96D1_487D_B83E_43E43EFBA4D3
#define RANGE_HPP_3B87674F_96D1_487D_B83E_43E43EFBA4D3
#pragma once

/*
range.hpp
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

#include "rel_ops.hpp"

template<class iterator_type>
class range
{
public:
	using iterator = iterator_type;
	using value_type = typename std::iterator_traits<iterator>::value_type;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using reference = typename std::iterator_traits<iterator>::reference;
	using pointer = typename std::iterator_traits<iterator>::pointer;

	constexpr range() = default;

	template<typename begin_type, typename end_type>
	constexpr range(begin_type Begin, end_type End):
		m_Begin(Begin),
		m_End(End)
	{}

	template<typename begin_type>
	constexpr range(begin_type Begin, size_t Count):
		range(Begin, Begin + Count)
	{
	}

	template<typename compatible_iterator, REQUIRES((std::is_convertible_v<compatible_iterator, iterator_type>))>
	constexpr range(const range<compatible_iterator>& Rhs):
		range(ALL_RANGE(Rhs))
	{
	}

	constexpr bool empty() const { return m_Begin == m_End; }

	constexpr auto begin() const { return m_Begin; }
	constexpr auto end() const { return m_End; }

	constexpr auto cbegin() const { static_assert(is_const); return begin(); }
	constexpr auto cend() const { static_assert(is_const); return end(); }

	constexpr auto rbegin() const { return reverse_iterator(m_End); }
	constexpr auto rend() const { return reverse_iterator(m_Begin); }

	constexpr auto crbegin() const { static_assert(is_const); return rbegin(); }
	constexpr auto crend() const { static_assert(is_const); return rend(); }

	constexpr auto& front() const
	{
		assert(!empty());
		return *m_Begin;
	}

	constexpr auto& back() const
	{
		assert(!empty());
		return *std::prev(m_End);
	}

	constexpr auto& operator[](size_t n) const
	{
		assert(n < size());
		return *(m_Begin + n);
	}

	constexpr auto data() const { return &*m_Begin; }
	constexpr size_t size() const { return m_End - m_Begin; }

	/*constexpr*/ auto pop_front()
	{
		assert(!empty());
		return m_Begin++;
	}

	/*constexpr*/ auto pop_back()
	{
		assert(!empty());
		return m_End--;
	}

	void swap(range& Rhs) noexcept
	{
		using std::swap;
		swap(*this, Rhs);
	}

private:
	enum { is_const = std::is_const_v<std::remove_reference_t<reference>> };

	iterator m_Begin {};
	iterator m_End {};
};

template<class iterator_type>
constexpr auto make_range(iterator_type i_begin, iterator_type i_end)
{
	return range<iterator_type>(i_begin, i_end);
}

template<class iterator_type>
constexpr auto make_range(iterator_type i_begin, size_t Size)
{
	return range<iterator_type>(i_begin, i_begin + Size);
}

template<class container>
constexpr auto make_range(container& Container)
{
	return make_range(ALL_RANGE(Container));
}

template<class container>
constexpr auto make_const_range(const container& Container)
{
	return make_range(ALL_CONST_RANGE(Container));
}

template<class T>
class i_iterator: public rel_ops<i_iterator<T>>
{
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using reference = T&;

	explicit i_iterator(const T& value): m_value(value) {}

	auto operator->() const { return &m_value; }
	auto& operator*() const { return m_value; }

	auto& operator++() { ++m_value; return *this; }
	auto& operator--() { --m_value; return *this; }

	auto& operator+=(size_t n) { m_value += n; return *this; }
	auto& operator-=(size_t n) { m_value -= n; return *this; }

	auto operator+(size_t n) const { return i_iterator(m_value + n); }
	auto operator-(size_t n) const { return i_iterator(m_value - n); }

	auto operator-(const i_iterator& rhs) const { return m_value - rhs.m_value; }
	auto operator==(const i_iterator& rhs) const { return m_value == rhs.m_value; }
	auto operator<(const i_iterator& rhs) const { return m_value < rhs.m_value; }

private:
	T m_value;
};

template<class T>
auto make_irange(T i_begin, T i_end)
{
	using iterator = i_iterator<T>;
	return range<iterator>(iterator(i_begin), iterator(i_end));
}

template<class T>
auto make_irange(T i_end)
{
	using iterator = i_iterator<T>;
	return range<iterator>(iterator(0), iterator(i_end));
}

#endif // RANGE_HPP_3B87674F_96D1_487D_B83E_43E43EFBA4D3
