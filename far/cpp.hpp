#pragma once

/*
cpp.hpp

Some workarounds & emulations for C++11/14 features, missed in currently used compilers & libraries.

Here be dragons
*/
/*
Copyright © 2013 Far Group
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

// already included in VC2012
#if defined _MSC_VER && _MSC_VER < 1700
// In VC++ 2010, there are three overloads of std::to_wstring taking long long, unsigned long long, and long double.
// Clearly, int is none of these, and no one conversion is better than another, so the conversion cannot be done implicitly/unambiguously.
// In terms of real C++11 support, this is a failing on the part of VC++ 2010's standard library implementation –
// the C++11 standard itself actually calls for nine overloads of std::to_wstring (§21.5)
namespace std
{
	inline wstring to_wstring(int val) {return to_wstring(static_cast<long long>(val));}
	inline wstring to_wstring(unsigned val) {return to_wstring(static_cast<unsigned long long>(val));}
	inline wstring to_wstring(long val) {return to_wstring(static_cast<long long>(val));}
	inline wstring to_wstring(unsigned long val) {return to_wstring(static_cast<unsigned long long>(val));}
	inline wstring to_wstring(float val) {return to_wstring(static_cast<long double>(val));}
	inline wstring to_wstring(double val) {return to_wstring(static_cast<long double>(val));}
};
#endif

// already included in VC2013
#if !defined _MSC_VER || (defined _MSC_VER && _MSC_VER < 1800)
// const, reverse and const-reverse versions of std::begin and std::end are missed in C++11,
// possibly they will be added in C++14, until then they are manually defined here:
namespace std
{
	template<class T>
	inline auto cbegin(const T& t) -> decltype(begin(t)) {return begin(t);}
	template<class T>
	inline auto cend(const T& t) -> decltype(end(t)) {return end(t);}

	template<class T>
	inline auto rbegin(T& t) -> reverse_iterator<decltype(end(t))> {return reverse_iterator<decltype(end(t))>(end(t));}
	template<class T>
	inline auto rend(T& t) -> reverse_iterator<decltype(begin(t))> {return reverse_iterator<decltype(begin(t))>(begin(t));}

	template<class T>
	inline auto crbegin(const T& t) -> decltype(rbegin(t)) {return rbegin(t);}
	template<class T>
	inline auto crend(const T& t) -> decltype(rend(t)) {return rend(t);}
};
#endif

// already included in VC2013
#if !defined _MSC_VER || (defined _MSC_VER && _MSC_VER < 1800)
namespace std
{
	template<typename T>
	inline typename enable_if<is_array<T>::value && extent<T>::value == 0, unique_ptr<T> >::type make_unique(size_t Size)
	{
		typedef typename remove_extent<T>::type Elem;
		return unique_ptr<T>(new Elem[Size]());
	}

	template<typename T>
	inline typename enable_if<!is_array<T>::value, unique_ptr<T>>::type make_unique()
	{
		return unique_ptr<T>(new T());
	}

	#define TYPE(name) name##_type
	#define TYPENAME(name) typename TYPE(name)
	#define ARG(name) TYPE(name) name
	#define FARG(name) std::forward<TYPE(name)>(name)

	#define MAKE_UNIQUE_BODY(TYPENAME_LIST, ARG_LIST, FARG_LIST) \
	template<typename T, TYPENAME_LIST> \
	inline typename enable_if<!is_array<T>::value, unique_ptr<T>>::type make_unique(ARG_LIST) \
	{ \
		return unique_ptr<T>(new T(FARG_LIST)); \
	}

	#define MAKE_UNIQUE(LISTN, ...) MAKE_UNIQUE_BODY(LISTN(TYPENAME, __VA_ARGS__), LISTN(ARG, __VA_ARGS__), LISTN(FARG, __VA_ARGS__))

	#define LIST1(MODE, n1) MODE(n1)
	#define LIST2(MODE, n1, n2) LIST1(MODE, n1), MODE(n2)
	#define LIST3(MODE, n1, n2, n3) LIST2(MODE, n1, n2), MODE(n3)
	#define LIST4(MODE, n1, n2, n3, n4) LIST3(MODE, n1, n2, n3), MODE(n4)
	#define LIST5(MODE, n1, n2, n3, n4, n5) LIST4(MODE, n1, n2, n3, n4), MODE(n5)

	MAKE_UNIQUE(LIST1, a1)
	MAKE_UNIQUE(LIST2, a1, a2)
	MAKE_UNIQUE(LIST3, a1, a2, a3)
	MAKE_UNIQUE(LIST4, a1, a2, a3, a4)
	MAKE_UNIQUE(LIST5, a1, a2, a3, a4, a5)

	#undef LIST5
	#undef LIST4
	#undef LIST3
	#undef LIST2
	#undef LIST1
	#undef MAKE_UNIQUE
	#undef MAKE_UNIQUE_BODY
	#undef FARG
	#undef ARG
	#undef TYPE
};
#endif

#ifdef _MSC_VER
#define thread_local __declspec(thread)
#endif

#ifdef _MSC_VER
#define noexcept throw()
#endif

// already included in VC2013
#if defined _MSC_VER && _MSC_VER < 1800
# define wcstoll _wcstoi64
#endif
