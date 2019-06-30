#ifndef WHERE_HPP_AF7DFD0D_8FF7_4F1B_B5D0_3659D6AEA862
#define WHERE_HPP_AF7DFD0D_8FF7_4F1B_B5D0_3659D6AEA862
#pragma once

/*
where.hpp
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
#include "../rel_ops.hpp"

//----------------------------------------------------------------------------

namespace detail
{
	template<typename T, typename predicate>
	class where_iterator : public rel_ops<where_iterator<T, predicate>>
	{
	public:
		using iterator_category = std::common_type_t<std::bidirectional_iterator_tag, typename std::iterator_traits<T>::iterator_category>;
		using difference_type = std::ptrdiff_t;
		using reference = std::invoke_result_t<predicate, typename std::iterator_traits<T>::value_type>;
		using value_type = std::remove_reference_t<reference>;
		using pointer = value_type*;

		explicit where_iterator(const T& Value, const T& End, const predicate& Predicate):
			m_Value(Value),
			m_End(End),
			m_Predicate(&Predicate)
		{
			while (m_Value != m_End && !(*m_Predicate)(*m_Value))
				++m_Value;
		}

		[[nodiscard]]
		decltype(auto) operator*() { return *m_Value; }

		[[nodiscard]]
		decltype(auto) operator*() const { return *m_Value; }

		[[nodiscard]]
		auto operator->() { return &**this; }

		[[nodiscard]]
		auto operator->() const { return &**this; }

		auto& operator++()
		{
			do ++m_Value;
			while (m_Value != m_End && !(*m_Predicate)(*m_Value));
			return *this;
		}

		auto& operator--()
		{
			do --m_Value;
			while (m_Value != m_End && !(*m_Predicate)(*m_Value));
			return *this;
		}

		[[nodiscard]]
		auto operator==(const where_iterator& rhs) const { return m_Value == rhs.m_Value; }

	private:
		T m_Value;
		T m_End;
		predicate const* m_Predicate;
	};
}

template<typename container, typename container_ref, typename predicate, typename predicate_ref, typename T>
class where
{
public:
	where(container_ref Container, predicate_ref Predicate):
		m_Container(FWD(Container)),
		m_Predicate(FWD(Predicate))
	{
	}

	[[nodiscard]] auto begin()        { return detail::where_iterator(std::begin(this->m_Container), std::end(this->m_Container), this->m_Predicate); }
	[[nodiscard]] auto end()          { return detail::where_iterator(std::end(this->m_Container),   std::end(this->m_Container), this->m_Predicate); }
	[[nodiscard]] auto begin()  const { return detail::where_iterator(std::begin(this->m_Container), std::end(this->m_Container), this->m_Predicate); }
	[[nodiscard]] auto end()    const { return detail::where_iterator(std::end(this->m_Container),   std::end(this->m_Container), this->m_Predicate); }
	[[nodiscard]] auto cbegin() const { return detail::where_iterator(std::begin(this->m_Container), std::end(this->m_Container), this->m_Predicate); }
	[[nodiscard]] auto cend()   const { return detail::where_iterator(std::end(this->m_Container),   std::end(this->m_Container), this->m_Predicate); }

private:
	container m_Container;
	predicate m_Predicate;
};

template<typename container, typename predicate>
where(container&& Container, predicate&& Predicate) ->
	where<
		keep_alive_type<decltype(Container)>, decltype(Container),
		keep_alive_type<decltype(Predicate)>, decltype(Predicate),
		decltype(std::begin(Container))
	>;

#endif // WHERE_HPP_AF7DFD0D_8FF7_4F1B_B5D0_3659D6AEA862
