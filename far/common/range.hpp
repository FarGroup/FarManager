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

#include "preprocessor.hpp"
#include "keep_alive.hpp"
#include "rel_ops.hpp"
#include "type_traits.hpp"

//----------------------------------------------------------------------------

template<typename iterator_type, typename const_iterator_type = iterator_type>
class range
{
public:
	using iterator = iterator_type;
	using const_iterator = std::conditional_t<std::is_same_v<iterator, const_iterator_type>&& std::is_pointer_v<iterator>,
		std::add_pointer_t<std::add_const_t<std::remove_pointer_t<iterator>>>,
		const_iterator_type>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using value_type = typename std::iterator_traits<iterator>::value_type;
	using reference = typename std::iterator_traits<iterator>::reference;
	using pointer = typename std::iterator_traits<iterator>::pointer;
	using iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;

public:
	constexpr range() = default;

	constexpr range(iterator Begin, iterator End):
		m_Begin(Begin),
		m_End(End)
	{}

	constexpr range(iterator Begin, size_t Count):
		range(Begin, Begin + Count)
	{
	}

	template<typename compatible_iterator, typename compatible_const_iterator,
		REQUIRES((std::is_convertible_v<compatible_iterator, iterator> && std::is_convertible_v<compatible_const_iterator, const_iterator>))>
	constexpr range(const range<compatible_iterator, compatible_const_iterator>& Rhs):
		range(ALL_RANGE(Rhs))
	{
	}

	template<typename container, REQUIRES(is_range_v<container>)>
	constexpr range(container& Container):
		range(ALL_RANGE(Container))
	{
	}

	constexpr range(const std::initializer_list<value_type>& List):
		range(ALL_RANGE(List))
	{
	}

	[[nodiscard]]
	constexpr bool empty() const { return m_Begin == m_End; }

	[[nodiscard]]
	constexpr auto begin() const { return m_Begin; }

	[[nodiscard]]
	constexpr auto end() const { return m_End; }

	[[nodiscard]]
	constexpr auto cbegin() const { return const_iterator(m_Begin); }

	[[nodiscard]]
	constexpr auto cend() const { return const_iterator(m_End); }

	[[nodiscard]]
	constexpr auto rbegin() const { return reverse_iterator(m_End); }

	[[nodiscard]]
	constexpr auto rend() const { return reverse_iterator(m_Begin); }

	[[nodiscard]]
	constexpr auto crbegin() const { return const_reverse_iterator(m_End); }

	[[nodiscard]]
	constexpr auto crend() const { return const_reverse_iterator(m_Begin); }

	[[nodiscard]]
	constexpr auto& front() const
	{
		assert(!empty());
		return *m_Begin;
	}

	[[nodiscard]]
	constexpr auto& back() const
	{
		assert(!empty());
		return *std::prev(m_End);
	}

	[[nodiscard]]
	constexpr auto& operator[](size_t n) const
	{
		static_assert(std::is_convertible_v<iterator_category, std::random_access_iterator_tag>);
		assert(n < size() || (!n && empty()));
		return *(m_Begin + n);
	}

	[[nodiscard]]
	constexpr auto data() const { return &(*this)[0]; }

	[[nodiscard]]
	constexpr size_t size() const { return m_End - m_Begin; }

	constexpr void pop_front()
	{
		assert(!empty());
		++m_Begin;
	}

	constexpr void pop_front(size_t const Distance)
	{
		assert(size() >= Distance);
		std::advance(m_Begin, Distance);
	}

	constexpr void pop_back()
	{
		assert(!empty());
		--m_End;
	}

	constexpr void pop_back(size_t const Distance)
	{
		assert(size() >= Distance);
		std::advance(m_End, -static_cast<ptrdiff_t>(Distance));
	}

	void swap(range& Rhs) noexcept
	{
		using std::swap;
		swap(*this, Rhs);
	}

private:
	iterator m_Begin{};
	iterator m_End{};
};

template<typename container>
range(container& c) -> range<std::remove_reference_t<decltype(std::begin(c))>, std::remove_reference_t<decltype(std::cbegin(c))>>;

template<typename value_type>
range(const std::initializer_list<value_type>&) -> range<const value_type*>;


template<class span_value_type>
class span: public range<span_value_type*, span_value_type const*>
{
public:
	constexpr span() = default;

	constexpr span(span_value_type* Begin, span_value_type* End):
		range<span_value_type*, span_value_type const*>(Begin, End)
	{}

	constexpr span(span_value_type* Data, size_t Size):
		span(Data, Data + Size)
	{
	}

	template<typename compatible_span_value_type, REQUIRES((std::is_convertible_v<compatible_span_value_type*, span_value_type*>))>
	constexpr span(const span<compatible_span_value_type>& Rhs):
		span(ALL_RANGE(Rhs))
	{
	}

	template<typename container, REQUIRES(is_span_v<container>)>
	constexpr span(container& Container):
		span(std::data(Container), std::size(Container))
	{
	}

	constexpr span(const std::initializer_list<span_value_type>& List) :
		span(ALL_CONST_RANGE(List))
	{
	}
};

template<typename container>
span(container& c) -> span<std::remove_pointer_t<decltype(std::data(c))>>;

template<typename value_type>
span(const std::initializer_list<value_type>&) -> span<const value_type>;


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

	[[nodiscard]]
	auto operator->() const { return &m_value; }

	[[nodiscard]]
	auto& operator*() const { return m_value; }

	auto& operator++() { ++m_value; return *this; }
	auto& operator--() { --m_value; return *this; }

	auto& operator+=(size_t n) { m_value += n; return *this; }
	auto& operator-=(size_t n) { m_value -= n; return *this; }

	auto operator+(size_t n) const { return i_iterator(m_value + n); }
	auto operator-(size_t n) const { return i_iterator(m_value - n); }

	auto operator-(const i_iterator& rhs) const { return m_value - rhs.m_value; }

	[[nodiscard]]
	auto operator==(const i_iterator& rhs) const { return m_value == rhs.m_value; }

	[[nodiscard]]
	auto operator<(const i_iterator& rhs) const { return m_value < rhs.m_value; }

private:
	T m_value;
};

template<class T>
[[nodiscard]]
auto make_irange(T i_begin, T i_end)
{
	return range(i_iterator(i_begin), i_iterator(i_end));
}

template<class T>
[[nodiscard]]
auto make_irange(T i_end)
{
	return range(i_iterator(0), i_iterator(i_end));
}

#endif // RANGE_HPP_3B87674F_96D1_487D_B83E_43E43EFBA4D3
