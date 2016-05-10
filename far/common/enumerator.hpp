#ifndef ENUMERATOR_HPP_6BCD3B36_3A68_400C_82B5_AB3644D0A874
#define ENUMERATOR_HPP_6BCD3B36_3A68_400C_82B5_AB3644D0A874
#pragma once

/*
enumerator.hpp
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

template<typename Derived, typename T>
class enumerator
{
public:
	typedef T value_type;

	class const_iterator: public std::iterator<std::forward_iterator_tag, T>
	{
	public:
		typedef const T value_type;

		const_iterator(Derived* collection = nullptr, size_t index = -1): m_collection(collection), m_index(index), m_value() {}
		const_iterator(const const_iterator& rhs): m_collection(rhs.m_collection), m_index(rhs.m_index), m_value(rhs.m_value) {}
		const_iterator& operator =(const const_iterator& rhs) { m_collection = rhs.m_collection; m_index = rhs.m_index; m_value = rhs.m_value; return *this;}
		value_type* operator ->() const { return &m_value; }
		value_type& operator *() const { return m_value; }
		const_iterator& operator ++() { if (!m_collection->get(++m_index, m_value)) m_index = size_t(-1); return *this; }
		bool operator ==(const const_iterator& rhs) const { return m_index == rhs.m_index; }
		bool operator !=(const const_iterator& rhs) const { return !(*this == rhs); }

	private:
		friend enumerator;

		Derived* m_collection;
		size_t m_index;
		T m_value;
	};

	class iterator: public const_iterator
	{
	public:
		typedef T value_type;

		iterator(Derived* collection = nullptr, size_t index = -1): const_iterator(collection, index) {}
		iterator(const iterator& rhs): const_iterator(rhs) {}
		value_type* operator ->() { return const_cast<T*>(const_iterator::operator->()); }
		value_type& operator *() { return const_cast<T&>(const_iterator::operator*()); }
		iterator& operator ++() { const_iterator::operator++(); return *this; }
	};

	iterator begin() { iterator result(static_cast<Derived*>(this), 0); if (!static_cast<Derived*>(this)->get(result.m_index, result.m_value)) result.m_index = size_t(-1); return result; }
	iterator end() { return iterator(static_cast<Derived*>(this), size_t(-1)); }
	const_iterator begin() const { return const_cast<enumerator*>(this)->begin(); }
	const_iterator end() const { return const_cast<enumerator*>(this)->end(); }
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }

private:
	friend iterator;
};

#endif // ENUMERATOR_HPP_6BCD3B36_3A68_400C_82B5_AB3644D0A874
