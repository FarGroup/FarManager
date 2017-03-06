#ifndef ZIP_VIEW_HPP_92A80223_8204_4A14_AACC_93D632A39884
#define ZIP_VIEW_HPP_92A80223_8204_4A14_AACC_93D632A39884
#pragma once

/*
zip_view.hpp
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

namespace detail
{
	struct increment { template<typename T> auto operator()(T& Object) const { return ++Object; } };
	struct decrement { template<typename T> auto operator()(T& Object) const { return --Object; } };

	template<typename... args>
	struct traits
	{
		using iterator_category = std::common_type_t<typename std::iterator_traits<args>::iterator_category...>;
		using pointer = std::tuple<args...>;
		using difference_type = ptrdiff_t;

		template<size_t... index>
		static auto dereference_impl(std::index_sequence<index...>, const pointer& Tuple)
		{
			return std::tie(*std::get<index>(Tuple)...);
		}
		static auto dereference(const pointer& Tuple) { return dereference_impl(std::make_index_sequence<sizeof...(args)>{}, Tuple); }

		using reference = decltype(dereference(pointer()));
		using value_type = reference;

		template<size_t index = 0, REQUIRES(index < sizeof...(args)), typename operation>
		static void alter_all(operation Operation, pointer& Tuple)
		{
			Operation(std::get<index>(Tuple));
			alter_all<index + 1>(Operation, Tuple);
		}

		template<size_t index, REQUIRES(index >= sizeof...(args)), typename operation>
		static void alter_all(operation, pointer&) {}
	};

	inline void check() {}

	template<typename arg, typename... args>
	void check(arg&&, args&&... Args)
	{
		TERSE_STATIC_ASSERT(std::is_lvalue_reference<arg>::value);
		check(std::forward<args>(Args)...);
	}
}

template<typename... args>
class zip_iterator:
	public std::iterator<
		typename detail::traits<args...>::iterator_category,
		typename detail::traits<args...>::value_type,
		typename detail::traits<args...>::difference_type,
		typename detail::traits<args...>::pointer,
		typename detail::traits<args...>::reference
	>,
	public rel_ops<zip_iterator<args...>>
{
public:
	zip_iterator() = default;
	zip_iterator(const args&... Args): m_Tuple(Args...) {}
	auto& operator++() { detail::traits<args...>::alter_all(detail::increment{}, m_Tuple); return *this; }
	auto& operator--() { detail::traits<args...>::alter_all(detail::decrement{}, m_Tuple); return *this; }
	auto operator==(const zip_iterator& rhs) const { return m_Tuple == rhs.m_Tuple; }
	auto operator<(const zip_iterator& rhs) const { return m_Tuple < rhs.m_Tuple; }
	auto operator*() const { return detail::traits<args...>::dereference(m_Tuple); }

private:
	typename zip_iterator::pointer m_Tuple;
};

template<typename... args>
class zip_view
{
public:
	using iterator = zip_iterator<decltype(std::begin(std::declval<args>()))...>;

	zip_view(args&&... Args):
		m_Begin(std::begin(Args)...),
		m_End(std::end(Args)...)
	{
		detail::check(std::forward<args>(Args)...);
	}

	auto begin() const { return m_Begin; }
	auto end() const { return m_End; }

	auto cbegin() const { return begin(); }
	auto cend() const { return end(); }

private:
	iterator m_Begin, m_End;
};

template<typename... args>
auto zip(args&&... Args)
{
	return zip_view<args...>(std::forward<args>(Args)...);
}

#endif // ZIP_VIEW_HPP_92A80223_8204_4A14_AACC_93D632A39884
