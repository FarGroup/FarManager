﻿#ifndef ENUMERATOR_HPP_6BCD3B36_3A68_400C_82B5_AB3644D0A874
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

#include "rel_ops.hpp"

template<typename Derived, typename T>
class enumerator
{
public:
	// VS2015
	//NONCOPYABLE(enumerator);
	//MOVABLE(enumerator);

	using value_type = T;
	using enumerator_type = enumerator;

	template<typename item_type, typename owner>
	class iterator_t:
		public rel_ops<iterator_t<item_type, owner>>
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = item_type;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		using owner_type = owner;

		enum class position
		{
			begin,
			middle,
			end
		};

		iterator_t() = default;
		iterator_t(owner_type Owner, position Position): m_Owner(Owner), m_Position(Position) {}

		auto operator->() { return &m_Value; }
		auto operator->() const { return &m_Value; }

		auto& operator*() { return m_Value; }
		auto& operator*() const { return m_Value; }

		auto& operator++()
		{
			assert(m_Position != position::end);
			m_Position = m_Owner->get(m_Position == position::begin, m_Value)? position::middle : position::end;
			return *this;
		}

		auto operator++(int)
		{
			auto Copy = *this;
			++*this;
			return Copy;
		}

		bool operator==(const iterator_t& rhs) const
		{
			assert(!m_Owner || !rhs.m_Owner || m_Owner == rhs.m_Owner);
			return m_Owner == rhs.m_Owner && m_Position == rhs.m_Position;
		}

		explicit operator bool() const
		{
			return m_Owner != nullptr;
		}

		static const size_t invalid_index{ size_t(-1) };

	private:
		owner_type m_Owner {};
		position m_Position{ position::end };
		std::remove_const_t<value_type> m_Value {};
	};

	using iterator = iterator_t<T, Derived*>;
	using const_iterator = iterator_t<const T, const Derived*>;

	auto begin() { return std::next(make_iterator<iterator>(this, iterator::position::begin)); }
	auto end() { return make_iterator<iterator>(this); }

	auto begin() const { return std::next(make_iterator<const_iterator>(this, const_iterator::position::begin)); }
	auto end() const { return make_iterator<const_iterator>(this); }

	auto cbegin() const { return begin(); }
	auto cend() const { return end(); }

protected:
	enumerator() { static_assert((std::is_base_of_v<enumerator, Derived>)); }

private:
	template<typename iterator_type, typename owner_type>
	static auto make_iterator(owner_type Owner, typename iterator_type::position Position = iterator_type::position::end) { return iterator_type{ static_cast<typename iterator_type::owner_type>(Owner), Position }; }
};

#define IMPLEMENTS_ENUMERATOR(type) friend typename type::enumerator_type

template<typename value_type, typename callable>
class inline_enumerator: public enumerator<inline_enumerator<value_type, callable>, value_type>
{
	IMPLEMENTS_ENUMERATOR(inline_enumerator);

public:
	explicit inline_enumerator(callable&& Callable):
		m_Callable(FWD(Callable))
	{
	}

private:
	bool get(bool Reset, value_type& Value) const
	{
		return m_Callable(Reset, Value);
	}

	mutable callable m_Callable;
};

template<typename value_type, typename callable>
[[nodiscard]]
auto make_inline_enumerator(callable&& Callable)
{
	return inline_enumerator<value_type, callable>(FWD(Callable));
}

#endif // ENUMERATOR_HPP_6BCD3B36_3A68_400C_82B5_AB3644D0A874
