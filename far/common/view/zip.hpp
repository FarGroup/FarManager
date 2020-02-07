#ifndef ZIP_HPP_92A80223_8204_4A14_AACC_93D632A39884
#define ZIP_HPP_92A80223_8204_4A14_AACC_93D632A39884
#pragma once

/*
zip.hpp
*/
/*
Copyright © 2016 Far Group
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
	template<typename... args>
	struct traits
	{
		using iterator_category = std::common_type_t<typename std::iterator_traits<args>::iterator_category...>;
		using pointer = std::tuple<args...>;
		using difference_type = ptrdiff_t;

		using index_sequence = std::index_sequence_for<args...>;

		template<size_t... index>
		[[nodiscard]]
		static auto dereference_impl(std::index_sequence<index...>, const pointer& Tuple)
		{
			return std::tuple<decltype(*std::get<index>(Tuple))...>(*std::get<index>(Tuple)...);
		}

		[[nodiscard]]
		static auto dereference(const pointer& Tuple)
		{
			return dereference_impl(index_sequence{}, Tuple);
		}

		using reference = decltype(dereference(pointer()));
		using value_type = reference;

		template<size_t... index, typename predicate>
		[[nodiscard]]
		static bool binary_any_of_impl(std::index_sequence<index...>, predicate Predicate, const pointer& Tuple1, const pointer& Tuple2)
		{
			return (... || Predicate(std::get<index>(Tuple1), std::get<index>(Tuple2)));
		}

		template<typename predicate>
		[[nodiscard]]
		static bool binary_any_of(predicate Predicate, const pointer& Tuple1, const pointer& Tuple2)
		{
			return binary_any_of_impl(index_sequence{}, Predicate, Tuple1, Tuple2);
		}

		template<size_t... index, typename predicate>
		[[nodiscard]]
		static bool binary_all_of_impl(std::index_sequence<index...>, predicate Predicate, const pointer& Tuple1, const pointer& Tuple2)
		{
			return (... && Predicate(std::get<index>(Tuple1), std::get<index>(Tuple2)));
		}

		template<typename predicate>
		[[nodiscard]]
		static bool binary_all_of(predicate Predicate, const pointer& Tuple1, const pointer& Tuple2)
		{
			return binary_all_of_impl(index_sequence{}, Predicate, Tuple1, Tuple2);
		}
	};

	template<typename... args>
	class zip_iterator: public rel_ops<zip_iterator<args...>>
	{
	public:
		using iterator_category = typename traits<args...>::iterator_category;
		using value_type        = typename traits<args...>::value_type;
		using difference_type   = typename traits<args...>::difference_type;
		using pointer           = typename traits<args...>::pointer;
		using reference         = typename traits<args...>::reference;

		zip_iterator() = default;
		explicit zip_iterator(args&&... Args): m_Tuple(Args...) {}
		auto& operator++() { std::apply([](auto&... Item){ (..., ++Item); }, m_Tuple); return *this; }
		auto& operator--() { std::apply([](auto&... Item){ (..., --Item); }, m_Tuple); return *this; }

		// tuple's operators == and < are inappropriate as ranges might be of different length and we want to stop on a shortest one
		[[nodiscard]]
		auto operator==(const zip_iterator& rhs) const { return traits<args...>::binary_any_of(std::equal_to<>{}, m_Tuple, rhs.m_Tuple); }

		[[nodiscard]]
		auto operator<(const zip_iterator& rhs) const { return traits<args...>::binary_all_of(std::less<>{}, m_Tuple, rhs.m_Tuple); }

		[[nodiscard]]
		auto& operator*() const
		{
			// Q: Why not just return by value?
			// A: We can, and everyone usually does that. However, returning by value makes analysers mad:
			//    first they suggest to remove '&' from "for (const auto& i: ...)" because "the range does not return a reference",
			//    and then "this creates a copy in each iteration; consider making this a reference".

			// A tuple of references is not assignable, hence the trick with optional.
			m_Value.emplace(traits<args...>::dereference(m_Tuple));
			return *m_Value;
		}

		[[nodiscard]]
		auto operator-(const zip_iterator& rhs) const { return std::get<0>(m_Tuple) - std::get<0>(rhs.m_Tuple); }

	private:
		pointer m_Tuple;
		mutable std::optional<value_type> m_Value;
	};
}

// the size_t is a workaround for GCC
template<size_t, typename... args>
class [[nodiscard]] zip
{
public:
	using iterator = detail::zip_iterator<decltype(std::begin(std::declval<args>()))...>;

	template<typename... args_ref>
	explicit zip(args_ref&&... ArgsRef):
		m_Args(FWD(ArgsRef)...),
		m_Begin(std::apply([](auto&&... Args) { return iterator(std::begin(Args)...); }, m_Args)),
		m_End(std::apply([](auto&&... Args) { return iterator(std::end(Args)...); }, m_Args))
	{
	}

	[[nodiscard]] auto begin()  const { return m_Begin; }
	[[nodiscard]] auto end()    const { return m_End; }

	[[nodiscard]] auto cbegin() const { return begin(); }
	[[nodiscard]] auto cend()   const { return end(); }

private:
	std::tuple<args...> m_Args;
	iterator m_Begin, m_End;
};

template<typename... args>
zip(args&&... Args) -> zip<sizeof...(args), keep_alive_type<decltype(Args)>...>;

#endif // ZIP_HPP_92A80223_8204_4A14_AACC_93D632A39884
