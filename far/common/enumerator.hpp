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

#include "movable.hpp"
#include "preprocessor.hpp"

#include <functional>
#include <iterator>

#include <cassert>

//----------------------------------------------------------------------------

template<typename Derived, typename T, bool Dereference = false>
class [[nodiscard]] enumerator
{
public:
	NONCOPYABLE(enumerator);
	MOVABLE(enumerator);

	using value_type = T;
	using enumerator_type = enumerator;

	template<typename item_type, typename owner>
	class iterator_t
	{
	public:
		using iterator_category = std::input_iterator_tag;
		using value_type = std::conditional_t<Dereference, std::remove_pointer_t<item_type>, item_type>;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		using owner_type = owner;

		enum class position
		{
			begin,
			middle,
			end
		};

		iterator_t() = default;

		iterator_t(owner_type Owner, position Position):
			m_Owner(Owner),
			m_Position(Position)
		{
		}

		iterator_t(iterator_t const& rhs):
			m_Owner(rhs.m_Owner),
			m_Position(rhs.m_Position),
			m_Value(rhs.m_Value)
		{
		}

		auto& operator=(iterator_t const& rhs)
		{
			m_Owner = rhs.m_Owner;
			m_Position = rhs.m_Position;
			m_Value = rhs.m_Value;
			return *this;
		}

		[[nodiscard]]
		auto operator->() const noexcept
		{
			return &remove_pointer(m_Value);
		}

		[[nodiscard]]
		auto& operator*() const noexcept
		{
			return remove_pointer(m_Value);
		}

		auto& operator++()
		{
			assert(m_Position != position::end);
			m_Position = m_Owner->get(m_Position == position::begin, m_Value)? position::middle : position::end;
			return *this;
		}

		POSTFIX_INCREMENT()

		[[nodiscard]]
		bool operator==(const iterator_t& rhs) const noexcept
		{
			assert(!m_Owner || !rhs.m_Owner || m_Owner == rhs.m_Owner);
			return m_Owner == rhs.m_Owner && m_Position == rhs.m_Position;
		}

		[[nodiscard]]
		explicit operator bool() const noexcept
		{
			return m_Owner != nullptr;
		}

		static inline constexpr size_t invalid_index{ static_cast<size_t>(-1) };

	private:
		template<typename V>
		static decltype(auto) remove_pointer(V& Value)
		{
			if constexpr (Dereference && std::is_pointer_v<V>)
				return *Value;
			else
				return Value;
		}

		item_type& value()
		{
			return m_Value;
		}

		owner_type m_Owner{};
		position m_Position{ position::end };
		std::remove_const_t<item_type> mutable m_Value{};
	};

	using iterator = iterator_t<T, Derived*>;
	using const_iterator = iterator_t<const T, const Derived*>;

	[[nodiscard]]
	auto begin() { return std::next(make_iterator<iterator>(this, iterator::position::begin)); }

	[[nodiscard]]
	auto end() { return make_iterator<iterator>(this); }

	[[nodiscard]]
	auto begin() const { return std::next(make_iterator<const_iterator>(this, const_iterator::position::begin)); }

	[[nodiscard]]
	auto end() const { return make_iterator<const_iterator>(this); }

	[[nodiscard]]
	auto cbegin() const { return begin(); }

	[[nodiscard]]
	auto cend() const { return end(); }

	[[nodiscard]]
	auto empty() const { return cbegin() == cend(); }

protected:
	enumerator() { static_assert(std::derived_from<Derived, enumerator>); }

private:
	template<typename iterator_type>
	[[nodiscard]]
	static auto make_iterator(auto Owner, typename iterator_type::position Position = iterator_type::position::end)
	{
		return iterator_type{ static_cast<typename iterator_type::owner_type>(Owner), Position };
	}
};

#define IMPLEMENTS_ENUMERATOR(type) friend typename type::enumerator_type

template<typename value_type>
class [[nodiscard]] inline_enumerator: public enumerator<inline_enumerator<value_type>, value_type>
{
	IMPLEMENTS_ENUMERATOR(inline_enumerator);

public:
	NONCOPYABLE(inline_enumerator);
	MOVE_CONSTRUCTIBLE(inline_enumerator);
	MOVE_ASSIGNABLE(inline_enumerator);

	using callable = std::function<bool(bool, value_type&)>;
	using finaliser = std::function<void()>;

	explicit inline_enumerator(callable Callable, finaliser Finaliser = {}):
		m_Callable(std::move(Callable)),
		m_Finaliser(std::move(Finaliser))
	{
	}

	~inline_enumerator()
	{
		if (m_Engaged && m_Finaliser)
			m_Finaliser();
	}

private:
	[[nodiscard]]
	bool get(bool Reset, value_type& Value) const
	{
		assert(m_Callable);

		return m_Callable(Reset, Value);
	}

	callable m_Callable;
	finaliser m_Finaliser;
	movable m_Engaged;
};

#endif // ENUMERATOR_HPP_6BCD3B36_3A68_400C_82B5_AB3644D0A874
