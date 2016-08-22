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
		Values.swap(TmpValues);
	}
}

namespace detail
{
	template <class Container, class Predicate>
	void erase_if_set_map(Container& c, const Predicate& pred)
	{
		for (auto i = c.begin(), End = c.end(); i != End; )
		{
			if (pred(*i))
			{
				i = c.erase(i);
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

template <class Key, class Compare, class Alloc, class Predicate>
void erase_if(std::set<Key, Compare, Alloc>& Container, Predicate Pred) { detail::erase_if_set_map(Container, Pred); }

template <class Key, class Compare, class Alloc, class Predicate>
void erase_if(std::multiset<Key, Compare, Alloc>& Container, Predicate Pred) { detail::erase_if_set_map(Container, Pred); }

template <class Key, class T, class Compare, class Alloc, class Predicate>
void erase_if(std::map<Key, T, Compare, Alloc>& Container, Predicate Pred) { detail::erase_if_set_map(Container, Pred); }

template <class Key, class T, class Compare, class Alloc, class Predicate>
void erase_if(std::multimap<Key, T, Compare, Alloc>& Container, Predicate Pred) { detail::erase_if_set_map(Container, Pred); }

template <class Key, class Hash, class KeyEqual, class Alloc, class Predicate>
void erase_if(std::unordered_set<Key, Hash, KeyEqual, Alloc>& Container, Predicate Pred) { detail::erase_if_set_map(Container, Pred); }

template <class Key, class Hash, class KeyEqual, class Alloc, class Predicate>
void erase_if(std::unordered_multiset<Key, Hash, KeyEqual, Alloc>& Container, Predicate Pred) { detail::erase_if_set_map(Container, Pred); }

template <class Key, class T, class Hash, class KeyEqual, class Alloc, class Predicate>
void erase_if(std::unordered_map<Key, T, Hash, KeyEqual, Alloc>& Container, Predicate Pred) { detail::erase_if_set_map(Container, Pred); }

template <class Key, class T, class Hash, class KeyEqual, class Alloc, class Predicate>
void erase_if(std::unordered_multimap<Key, T, Hash, KeyEqual, Alloc>& Container, Predicate Pred) { detail::erase_if_set_map(Container, Pred); }


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

#endif // ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0
