#ifndef ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0
#define ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0
#pragma once

/*
algorithm.hpp
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

// for_each with embedded counter
template<class I, class F>
F for_each_cnt(I First, I Last, F Func)
{
	for (size_t Cnt = 0; First != Last; ++First, ++Cnt)
	{
		Func(*First, Cnt);
	}
	return Func;
}

template<class T>
void repeat(size_t count, const T& f)
{
	for(size_t i = 0; i != count; ++i)
	{
		f();
	}
}

template <class I, class T, class P>
void fill_if(I First, I Last, const T& Value, P Predicate)
{
	while (First != Last)
	{
		if (Predicate(*First))
			*First = Value;
		++First;
	}
}

template <class I, class N, class T, class P>
I fill_n_if(I First, N Size, const T& Value, P Predicate)
{
	while (Size > 0)
	{
		if (Predicate(*First))
			*First = Value;
		++First;
		--Size;
	}
	return First;
}

template<class T, class P>
void for_submatrix(T& Matrix, size_t X1, size_t Y1, size_t X2, size_t Y2, P Predicate)
{
	for (auto i = Y1; i <= Y2; ++i)
	{
		for (auto j = X1; j <= X2; ++j)
		{
			Predicate(Matrix[i][j]);
		}
	}
}

template <class T, class Y>
void reorder(T& Values, Y& Indices)
{
	assert(Values.size() == Indices.size());

	if (true)
	{
		// in place version: less memory, more swaps (and faster according to my measurements)
		for (size_t i = 0, size = Values.size(); i != size; ++i)
		{
			while (i != Indices[i])
			{
				const auto j = Indices[i];
				const auto k = Indices[j];
				using std::swap;
				swap(Values[j], Values[k]);
				swap(Indices[i], Indices[j]);
			}
		}
	}
	else
	{
		// copy version: less swaps, more memory (and quite straightforward)
		T TmpValues;
		TmpValues.reserve(Values.size());
		std::transform(ALL_CONST_RANGE(Indices), std::back_inserter(TmpValues), [&Values](const auto Index) { return std::move(Values[Index]); });
		using std::swap;
		swap(Values, TmpValues);
	}
}

namespace detail
{
	template <typename container, typename predicate>
	void associative_erase_if(container& Container, const predicate& Predicate)
	{
		for (auto i = Container.begin(), End = Container.end(); i != End; )
		{
			if (Predicate(*i))
			{
				i = Container.erase(i);
			}
			else
			{
				++i;
			}
		}
	}
}

// TODO: add proper overloads as per Library fundamentals TS v2: Uniform container erasure.
// Consider moving to cpp.hpp / std::experimental namespace for GCC, or just #include <experimental/...> for VS.

template <typename predicate, typename... traits>
void erase_if(std::set<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

template <typename predicate, typename... traits>
void erase_if(std::multiset<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

template <typename predicate, typename... traits>
void erase_if(std::map<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

template <typename predicate, typename... traits>
void erase_if(std::multimap<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

template <typename predicate, typename... traits>
void erase_if(std::unordered_set<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

template <typename predicate, typename... traits>
void erase_if(std::unordered_multiset<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

template <typename predicate, typename... traits>
void erase_if(std::unordered_map<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }

template <typename predicate, typename... traits>
void erase_if(std::unordered_multimap<traits...>& Container, predicate Predicate) { detail::associative_erase_if(Container, Predicate); }


namespace detail
{
	template<typename T, typename = std::void_t<>>
	struct has_emplace_hint: std::false_type {};

	template<typename T>
	struct has_emplace_hint<T, std::void_t<decltype(std::declval<T&>().emplace_hint(std::declval<T&>().end(), *std::declval<T&>().begin()))>>: std::true_type {};

	template<typename T>
	using has_emplace_hint_t = typename has_emplace_hint<T>::type;
}

// Unified container emplace
template<typename container, typename... args>
std::enable_if_t<detail::has_emplace_hint_t<container>::value> emplace(container& Container, args&&... Args)
{
	Container.emplace_hint(Container.end(), std::forward<args>(Args)...);
}

template<typename container, typename... args>
std::enable_if_t<!detail::has_emplace_hint_t<container>::value> emplace(container& Container, args&&... Args)
{
	Container.emplace(Container.end(), std::forward<args>(Args)...);
}


// uniform "contains"

// strings:
template<typename find_type, typename... traits>
bool contains(const std::basic_string<traits...>& Str, const find_type& What)
{
	return Str.find(What) != Str.npos;
}

namespace detail
{
	template<typename T, typename = std::void_t<>>
	struct has_find: std::false_type {};

	template<typename T>
	struct has_find<T, std::void_t<decltype(std::declval<T&>().find(std::declval<typename T::key_type&>()))>>: std::true_type {};

	template<typename T>
	using has_find_t = typename has_find<T>::type;
}

// associative containers
template<typename container, typename element>
std::enable_if_t<detail::has_find_t<container>::value, bool> contains(const container& Container, const element& Element)
{
	return Container.find(Element) != Container.cend();
}

// everything else
template<typename container, typename element>
std::enable_if_t<!detail::has_find_t<container>::value, bool> contains(const container& Container, const element& Element)
{
	const auto End = std::cend(Container);
	return std::find(std::cbegin(Container), End, Element) != End;
}


// TODO: std::clamp once it's clear how to detect its presence
template<typename type, typename compare>
constexpr const type& Clamp(const type& Value, const type& Low, const type& High, compare Compare)
{
	return assert(!Compare(High, Low)),
		Compare(Value, Low)? Low : Compare(High, Value)? High : Value;
}

template<typename type>
constexpr const type& Clamp(const type& Value, const type& Low, const type& High)
{
	return Clamp(Value, Low, High, std::less<>());
}

#endif // ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0
