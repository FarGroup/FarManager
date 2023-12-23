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
#include "../preprocessor.hpp"

#include <iterator>
#include <optional>
#include <tuple>

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

		template<size_t... index>
		[[nodiscard]]
		static bool binary_any_of_impl(std::index_sequence<index...>, auto Predicate, const auto& Tuple1, const auto& Tuple2)
		{
			return (... || Predicate(std::get<index>(Tuple1), std::get<index>(Tuple2)));
		}

		[[nodiscard]]
		static bool binary_any_of(auto Predicate, const auto& Tuple1, const auto& Tuple2)
		{
			return binary_any_of_impl(index_sequence{}, Predicate, Tuple1, Tuple2);
		}

		template<size_t... index>
		[[nodiscard]]
		static bool binary_all_of_impl(std::index_sequence<index...>, auto Predicate, const auto& Tuple1, const auto& Tuple2)
		{
			return (... && Predicate(std::get<index>(Tuple1), std::get<index>(Tuple2)));
		}

		[[nodiscard]]
		static bool binary_all_of(auto Predicate, const auto& Tuple1, const auto& Tuple2)
		{
			return binary_all_of_impl(index_sequence{}, Predicate, Tuple1, Tuple2);
		}
	};

	template<typename... args>
	auto tuple_difference(std::tuple<args...> const& a, std::tuple<args...> const& b)
	{
		return std::get<0>(a) - std::get<0>(b);
	}

	template<typename... iterators>
	class zip_iterator;

	template<typename... args>
	class zip_sentinel
	{
		template<typename...>
		friend class zip_iterator;

	public:
		MOVABLE(zip_sentinel);

		zip_sentinel() = default;
		explicit zip_sentinel(args&&... Args): m_Tuple(Args...) {}

		zip_sentinel(zip_sentinel const& rhs):
			m_Tuple(rhs.m_Tuple)
		{
		}

		auto& operator=(zip_sentinel const& rhs)
		{
			m_Tuple = rhs.m_Tuple;
			return *this;
		}

		template<typename... iterators>
		[[nodiscard]]
		auto operator-(zip_iterator<iterators...> const& rhs) const
		{
			return tuple_difference(m_Tuple, rhs.m_Tuple);
		}

	private:
		std::tuple<args...> m_Tuple;
	};

	template<typename... args>
	class zip_iterator
	{
		template<typename...>
		friend class zip_sentinel;

	public:
		using iterator_category = typename traits<args...>::iterator_category;
		using value_type        = typename traits<args...>::value_type;
		using difference_type   = typename traits<args...>::difference_type;
		using pointer           = typename traits<args...>::pointer;
		using reference         = typename traits<args...>::reference;

		MOVABLE(zip_iterator);

		zip_iterator() = default;
		explicit zip_iterator(args&&... Args): m_Tuple(Args...) {}

		zip_iterator(zip_iterator const& rhs):
			m_Tuple(rhs.m_Tuple)
		{
		}

		auto& operator=(zip_iterator const& rhs)
		{
			m_Tuple = rhs.m_Tuple;
			return *this;
		}

		auto& operator++() { std::apply([](auto&... Item){ (..., ++Item); }, m_Tuple); return *this; }
		auto& operator--() { std::apply([](auto&... Item){ (..., --Item); }, m_Tuple); return *this; }
		POSTFIX_OPS()

		template<typename... sentinels>
		[[nodiscard]]
		bool operator==(zip_sentinel<sentinels...> const& rhs) const { return traits<args...>::binary_any_of(std::equal_to{}, m_Tuple, rhs.m_Tuple); }

		// tuple's operators == and < are inappropriate as ranges might be of different length and we want to stop on the shortest one
		[[nodiscard]]
		bool operator==(const zip_iterator& rhs) const { return traits<args...>::binary_any_of(std::equal_to{}, m_Tuple, rhs.m_Tuple); }

		[[nodiscard]]
		bool operator<(const zip_iterator& rhs) const { return traits<args...>::binary_all_of(std::less{}, m_Tuple, rhs.m_Tuple); }

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

		template<typename... sentinels>
		[[nodiscard]]
		auto operator-(const zip_sentinel<sentinels...>& rhs) const { return tuple_difference(m_Tuple, rhs.m_Tuple); }

		[[nodiscard]]
		auto operator-(const zip_iterator& rhs) const { return tuple_difference(m_Tuple, rhs.m_Tuple); }

	private:
		pointer m_Tuple;
		mutable std::optional<value_type> m_Value;
	};
}

template<typename... args>
class [[nodiscard]] zip
{
public:
	using iterator = detail::zip_iterator<decltype(std::ranges::begin(std::declval<args&>()))...>;
	using sentinel = std::conditional_t<
		std::same_as<
			std::tuple<decltype(std::ranges::begin(std::declval<args&>()))...>,
			std::tuple<decltype(std::ranges::end(std::declval<args&>()))...>
		>,
		iterator,
		detail::zip_sentinel<decltype(std::ranges::end(std::declval<args&>()))...>
	>;

	explicit zip(auto&&... ArgsRef):
		m_Args(FWD(ArgsRef)...),
		m_Begin(std::apply([](auto&&... Args) { return iterator(std::ranges::begin(Args)...); }, m_Args)),
		m_End(std::apply([](auto&&... Args) { return sentinel(std::ranges::end(Args)...); }, m_Args))
	{
	}

	[[nodiscard]] auto begin()  const { return m_Begin; }
	[[nodiscard]] auto end()    const { return m_End; }

	[[nodiscard]] auto cbegin() const { return begin(); }
	[[nodiscard]] auto cend()   const { return end(); }

private:
	std::tuple<args...> m_Args;
	iterator m_Begin;
	sentinel m_End;
};

template<typename... args>
zip(args&&... Args) -> zip<keep_alive_type<decltype(Args)>...>;

#endif // ZIP_HPP_92A80223_8204_4A14_AACC_93D632A39884
