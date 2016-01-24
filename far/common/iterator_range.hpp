#ifndef ITERATOR_RANGE_HPP_3B87674F_96D1_487D_B83E_43E43EFBA4D3
#define ITERATOR_RANGE_HPP_3B87674F_96D1_487D_B83E_43E43EFBA4D3
#pragma once

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

template<class iterator_type>
class range
{
public:
	typedef iterator_type iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef typename std::iterator_traits<iterator>::reference reference;
	typedef typename std::iterator_traits<iterator>::pointer pointer;
	typedef typename std::iterator_traits<iterator>::value_type value_type;

	range():
		m_begin(),
		m_end()
	{}

	range(iterator i_begin, iterator i_end):
		m_begin(i_begin),
		m_end(i_end)
	{}

	iterator begin() const { return m_begin; }
	iterator end() const { return m_end; }
	reverse_iterator rbegin() const { return reverse_iterator(m_end); }
	reverse_iterator rend() const { return reverse_iterator(m_begin); }

	reference operator[](size_t n) { return *(m_begin + n); }
	reference operator[](size_t n) const { return *(m_begin + n); }

	reference front() { return *m_begin; }
	reference front() const { return *m_begin; }

	reference back() { return *std::prev(m_end); }
	reference back() const { return *std::prev(m_end); }

	pointer data() { return &*m_begin; }
	pointer data() const { return &*m_begin; }

	bool empty() const { return m_begin == m_end; }
	size_t size() const { return m_end - m_begin; }

private:
	iterator m_begin;
	iterator m_end;
};

template<class iterator_type>
inline range<iterator_type> make_range(iterator_type i_begin, iterator_type i_end)
{
	return range<iterator_type>(i_begin, i_end);
}

template<class T>
class i_iterator: public std::iterator<std::random_access_iterator_tag, T>, public rel_ops<i_iterator<T>>
{
public:
	typedef T value_type;

	i_iterator(const T& value): m_value(value) {}
	i_iterator(const i_iterator& rhs): m_value(rhs.m_value) {}
	i_iterator& operator=(const i_iterator& rhs) { m_value = rhs.m_value; return *this; }
	const value_type* operator->() const { return &m_value; }
	const value_type& operator*() const { return m_value; }
	i_iterator& operator++() { ++m_value; return *this; }
	i_iterator& operator--() { --m_value; return *this; }
	i_iterator& operator+=(size_t n) { m_value += n; return *this; }
	i_iterator& operator-=(size_t n) { m_value -= n; return *this; }
	i_iterator operator++(int) { return m_value++; }
	i_iterator operator--(int) { return m_value--; }
	i_iterator operator+(size_t n) const { return m_value + n; }
	i_iterator operator-(size_t n) const { return m_value - n; }
	ptrdiff_t operator-(const i_iterator& rhs) const { return m_value - rhs.m_value; }
	bool operator==(const i_iterator& rhs) const { return m_value == rhs.m_value; }
	bool operator<(const i_iterator& rhs) const { return m_value < rhs.m_value; }

private:
	T m_value;
};

template<class T>
inline range<i_iterator<T>> make_irange(T i_begin, T i_end)
{
	return range<i_iterator<T>>(i_begin, i_end);
}

template<class T>
inline range<i_iterator<T>> make_irange(T i_end)
{
	return range<i_iterator<T>>(0, i_end);
}

#endif // ITERATOR_RANGE_HPP_3B87674F_96D1_487D_B83E_43E43EFBA4D3
