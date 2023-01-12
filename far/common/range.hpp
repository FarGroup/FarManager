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
#include "type_traits.hpp"

#include <limits>

//----------------------------------------------------------------------------

template<typename iterator_type, typename const_iterator_type = iterator_type>
class [[nodiscard]] range
{
public:
	using iterator = iterator_type;
	using const_iterator = std::conditional_t<std::same_as<iterator, const_iterator_type> && std::is_pointer_v<iterator>,
		std::add_pointer_t<std::add_const_t<std::remove_pointer_t<iterator>>>,
		const_iterator_type>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using value_type = typename std::iterator_traits<iterator>::value_type;
	using reference = typename std::iterator_traits<iterator>::reference;
	using pointer = typename std::iterator_traits<iterator>::pointer;
	using iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;

	constexpr range() = default;

	constexpr range(iterator Begin, iterator End):
		m_Begin(Begin),
		m_End(End)
	{}

	constexpr range(iterator Begin, size_t Count):
		range(Begin, Begin + Count)
	{
	}

	template<typename compatible_iterator, typename compatible_const_iterator>
		requires std::convertible_to<compatible_iterator, iterator> && std::convertible_to<compatible_const_iterator, const_iterator>
	explicit(false) constexpr range(const range<compatible_iterator, compatible_const_iterator>& Rhs):
		range(ALL_RANGE(Rhs))
	{
	}

	constexpr explicit(false) range(range_like auto& Range):
		range(ALL_RANGE(Range))
	{
	}

	constexpr range(const std::initializer_list<value_type>& List):
		range(ALL_RANGE(List))
	{
	}

	template<typename iterator>
	constexpr explicit (false) range(std::pair<iterator, iterator> const& Pair):
		range(Pair.first, Pair.second)
	{}

	[[nodiscard]]
	constexpr bool empty() const noexcept { return m_Begin == m_End; }

	[[nodiscard]]
	constexpr auto begin() const noexcept { return m_Begin; }

	[[nodiscard]]
	constexpr auto end() const noexcept { return m_End; }

	[[nodiscard]]
	constexpr auto cbegin() const noexcept { return const_iterator(m_Begin); }

	[[nodiscard]]
	constexpr auto cend() const noexcept { return const_iterator(m_End); }

	[[nodiscard]]
	constexpr auto rbegin() const noexcept { return reverse_iterator(m_End); }

	[[nodiscard]]
	constexpr auto rend() const noexcept { return reverse_iterator(m_Begin); }

	[[nodiscard]]
	constexpr auto crbegin() const noexcept { return const_reverse_iterator(m_End); }

	[[nodiscard]]
	constexpr auto crend() const noexcept { return const_reverse_iterator(m_Begin); }

	[[nodiscard]]
	constexpr auto& front() const noexcept
	{
		assert(!empty());
		return *m_Begin;
	}

	[[nodiscard]]
	constexpr auto& back() const noexcept
	{
		assert(!empty());
		return *std::prev(m_End);
	}

	[[nodiscard]]
	constexpr auto& operator[](size_t n) const noexcept
	{
		static_assert(std::convertible_to<iterator_category, std::random_access_iterator_tag>);
		assert(n < size() || (!n && empty()));
		return *(m_Begin + n);
	}

	[[nodiscard]]
	constexpr auto data() const noexcept
	{
		if constexpr (std::is_pointer_v<iterator>)
			return m_Begin;
		else
			return &(*this)[0];
	}

	[[nodiscard]]
	constexpr size_t size() const noexcept { return m_End - m_Begin; }

	constexpr void pop_front() noexcept
	{
		assert(!empty());
		++m_Begin;
	}

	constexpr void pop_front(size_t const Count) noexcept
	{
		assert(size() >= Count);
		std::advance(m_Begin, Count);
	}

	constexpr void pop_back() noexcept
	{
		assert(!empty());
		--m_End;
	}

	constexpr void pop_back(size_t const Count) noexcept
	{
		assert(size() >= Count);
		std::advance(m_End, -static_cast<ptrdiff_t>(Count));
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

template<typename iterator>
range(std::pair<iterator, iterator> const& Pair) -> range<iterator, iterator>;


template<class span_value_type>
class [[nodiscard]] span: public range<span_value_type*, span_value_type const*>
{
public:
	constexpr span() noexcept = default;

	constexpr span(span_value_type* Begin, span_value_type* End) noexcept:
		range<span_value_type*, span_value_type const*>(Begin, End)
	{}

	constexpr span(span_value_type* Data, size_t Size) noexcept:
		span(Data, Data + Size)
	{
	}

	template<typename compatible_span_value_type> requires std::convertible_to<compatible_span_value_type*, span_value_type*>
	constexpr explicit(false) span(const span<compatible_span_value_type>& Rhs) noexcept:
		span(ALL_RANGE(Rhs))
	{
	}

	constexpr explicit(false) span(span_like auto&& Range) noexcept:
		span(std::data(Range), std::size(Range))
	{
	}

	constexpr span(const std::initializer_list<span_value_type>& List) noexcept:
		span(ALL_CONST_RANGE(List))
	{
	}

	constexpr span subspan(size_t const Offset, size_t const Size = std::numeric_limits<size_t>::max()) const noexcept
	{
		assert(Offset <= this->size());
		assert(Size == std::numeric_limits<size_t>::max() || (Size <= this->size() - Offset));

		return { this->data() + Offset, Size == std::numeric_limits<size_t>::max()? this->size() - Offset : Size };
	}
};

template<typename container>
span(container&& c) -> span<std::remove_pointer_t<decltype(std::data(c))>>;

template<typename value_type>
span(const std::initializer_list<value_type>&) -> span<const value_type>;


template<class T>
class [[nodiscard]] i_iterator
{
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using reference = T&;

	i_iterator() = default;
	explicit i_iterator(const T& value) noexcept: m_value(value) {}

	[[nodiscard]]
	auto operator->() const noexcept { return &m_value; }

	[[nodiscard]]
	auto& operator*() const noexcept { return m_value; }

	auto& operator++() noexcept { ++m_value; return *this; }
	auto& operator--() noexcept { --m_value; return *this; }

	auto& operator+=(size_t n) noexcept { m_value += n; return *this; }
	auto& operator-=(size_t n) noexcept { m_value -= n; return *this; }

	[[nodiscard]]
	auto operator+(size_t n) const noexcept { return i_iterator(m_value + n); }

	[[nodiscard]]
	auto operator-(size_t n) const noexcept { return i_iterator(m_value - n); }

	[[nodiscard]]
	auto operator-(const i_iterator& rhs) const noexcept { return m_value - rhs.m_value; }

	[[nodiscard]]
	auto operator<=>(const i_iterator&) const = default;

private:
	T m_value{};
};

template<typename T1, typename T2 = T1> requires std::integral<std::common_type_t<sane_underlying_type<T1>, sane_underlying_type<T2>>>
class [[nodiscard]] irange: public range<i_iterator<std::common_type_t<sane_underlying_type<T1>, sane_underlying_type<T2>>>>
{
	using integer_type = typename irange::value_type;

public:
	irange(T1 Begin, T2 End) noexcept:
		range<i_iterator<integer_type>>(i_iterator{static_cast<integer_type>(Begin)}, i_iterator{static_cast<integer_type>(End)})
	{
	}

	explicit irange(T1 End) noexcept:
		range<i_iterator<integer_type>>(i_iterator{integer_type{}}, i_iterator{static_cast<integer_type>(End)})
	{
	}
};

#endif // RANGE_HPP_3B87674F_96D1_487D_B83E_43E43EFBA4D3
