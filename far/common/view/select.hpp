﻿#ifndef SELECT_HPP_5E3DDA01_4008_4C35_9F2F_0AC4D2CD432A
#define SELECT_HPP_5E3DDA01_4008_4C35_9F2F_0AC4D2CD432A
#pragma once

/*
select.hpp
*/
/*
Copyright © 2019 Far Group
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

#include "../keep_alive.hpp"

#include <iterator>
#include <tuple>

//----------------------------------------------------------------------------

namespace detail
{
	template<typename T, typename accessor>
	class select_iterator_data
	{
	public:
		explicit select_iterator_data(const T& Value, accessor& Accessor):
			m_Value(Value),
			m_Accessor(&Accessor)
		{
		}

	protected:
		[[nodiscard]] decltype(auto) operator*()       { return call(*this); }
		[[nodiscard]] decltype(auto) operator*() const { return call(*this); }
		[[nodiscard]] auto operator->()       { return &**this; }
		[[nodiscard]] auto operator->() const { return &**this; }

		template<typename self_t>
		[[nodiscard]]
		static decltype(auto) call(self_t& Self)
		{
			if constexpr (requires(accessor&& Accessor, T&& Value){ Accessor(*Value); })
				return (*Self.m_Accessor)(*Self.m_Value);
			else
				return std::apply(*Self.m_Accessor, *Self.m_Value);
		}

		T m_Value;
		accessor* m_Accessor;
	};

	template<typename T, typename accessor>
	class select_iterator: private select_iterator_data<T, accessor>
	{
		using base = select_iterator_data<T, accessor>;

	public:
		using iterator_category = typename std::iterator_traits<T>::iterator_category;
		using difference_type = std::ptrdiff_t;
		using reference = decltype(base::call(std::declval<base&>()));
		using value_type = std::remove_reference_t<reference>;
		using pointer = value_type*;

		using base::select_iterator_data;
		using base::operator*;
		using base::operator->;

		auto& operator++() { ++this->m_Value; return *this; }
		auto& operator--() { --this->m_Value; return *this; }

		auto& operator+=(size_t n) { this->m_Value += n; return *this; }
		auto& operator-=(size_t n) { this->m_Value -= n; return *this; }

		auto operator+(size_t n) const { return select_iterator(this->m_Value + n); }
		auto operator-(size_t n) const { return select_iterator(this->m_Value - n); }

		auto operator-(const select_iterator& rhs) const { return this->m_Value - rhs.m_Value; }

		[[nodiscard]]
		bool operator==(const select_iterator& rhs) const { return this->m_Value == rhs.m_Value; }

		[[nodiscard]]
		bool operator<(const select_iterator& rhs) const { return this->m_Value < rhs.m_Value; }
	};

	template<typename T, typename accessor>
	select_iterator(const T&, accessor&) -> select_iterator<T, accessor>;
}

template<typename container, typename container_ref, typename accessor, typename accessor_ref, typename T>
class select
{
public:
	select(container_ref Container, accessor_ref Accessor):
		m_Container(FWD(Container)),
		m_Accessor{FWD(Accessor)}
	{
	}

	[[nodiscard]] auto begin()        { return detail::select_iterator(std::begin(this->m_Container), this->m_Accessor.value); }
	[[nodiscard]] auto end()          { return detail::select_iterator(std::end(this->m_Container), this->m_Accessor.value); }
	[[nodiscard]] auto begin()  const { return detail::select_iterator(std::begin(this->m_Container), this->m_Accessor.value); }
	[[nodiscard]] auto end()    const { return detail::select_iterator(std::end(this->m_Container), this->m_Accessor.value); }
	[[nodiscard]] auto cbegin() const { return detail::select_iterator(std::begin(this->m_Container), this->m_Accessor.value); }
	[[nodiscard]] auto cend()   const { return detail::select_iterator(std::end(this->m_Container), this->m_Accessor.value); }

private:
	container m_Container;
	// "All problems in computer science can be solved by another level of indirection"
	// This class can hold both values and references, but the language doesn't allow mutable references.
	mutable struct accessor_wrapper { accessor value; } m_Accessor;
};

template<typename container, typename accessor>
select(container&& Container, accessor&& Accessor) ->
	select<
		keep_alive_type<decltype(Container)>, decltype(Container),
		keep_alive_type<decltype(Accessor)>, decltype(Accessor),
		decltype(std::begin(Container))
	>;

#endif // SELECT_HPP_5E3DDA01_4008_4C35_9F2F_0AC4D2CD432A
