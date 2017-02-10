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
	using value_type = T;
	using enumerator_type = enumerator;

	template<typename item_type, typename owner>
	class iterator_t: public std::iterator<std::forward_iterator_tag, item_type>
	{
	public:
		using value_type = item_type;

		iterator_t() = default;
		iterator_t(const owner& Owner): m_Owner{ Owner }, m_Index{} {}

		auto operator->() { return static_cast<value_type*>(&m_Value); }
		auto operator->() const { return static_cast<value_type*>(&m_Value); }

		auto& operator*() { return static_cast<value_type&>(m_Value); }
		auto& operator*() const { return static_cast<value_type&>(m_Value); }

		auto& operator++()
		{
			if (m_Index == invalid_index)
			{
				throw std::runtime_error("enumerator iterator is out of range");
			}
			m_Index = m_Owner->get(m_Index, m_Value)? m_Index + 1 : invalid_index;
			return *this;
		}

		bool operator==(const iterator_t& rhs) const { return m_Index == rhs.m_Index; }
		bool operator!=(const iterator_t& rhs) const { return !(*this == rhs); }

	private:
		static constexpr size_t invalid_index { size_t(-1) };

		owner m_Owner {};
		size_t m_Index { invalid_index };
		std::remove_const_t<value_type> m_Value {};
	};

	using iterator = iterator_t<T, Derived*>;
	using const_iterator = iterator_t<const T, const Derived*>;

	auto begin() { return std::next(iterator{ static_cast<Derived*>(this) }); }
	auto end() { return iterator{}; }

	auto begin() const { return std::next(const_iterator{ static_cast<const Derived*>(this) }); }
	auto end() const { return const_iterator{}; }

	auto cbegin() const { return begin(); }
	auto cend() const { return end(); }

protected:
	enumerator() { TERSE_STATIC_ASSERT(std::is_base_of<enumerator, Derived>::value); }
};

#define IMPLEMENTS_ENUMERATOR(type) friend typename type::enumerator_type;

template<typename value_type, typename callable>
class inline_enumerator: public enumerator<inline_enumerator<value_type, callable>, value_type>
{
	IMPLEMENTS_ENUMERATOR(inline_enumerator);

public:
	inline_enumerator(callable&& Callable):
		m_Callable(std::forward<callable>(Callable))
	{
	}

private:
	bool get(size_t Index, value_type& Value) const
	{
		return m_Callable(Index, Value);
	}

	callable m_Callable;
};

template<typename value_type, typename callable>
auto make_inline_enumerator(callable&& Callable)
{
	return inline_enumerator<value_type, callable>(std::forward<callable>(Callable));
}

#endif // ENUMERATOR_HPP_6BCD3B36_3A68_400C_82B5_AB3644D0A874
