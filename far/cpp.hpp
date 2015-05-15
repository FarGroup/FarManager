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

	inline string to_string(int val) {return to_string(static_cast<long long>(val));}
	inline string to_string(unsigned val) {return to_string(static_cast<unsigned long long>(val));}
	inline string to_string(long val) {return to_string(static_cast<long long>(val));}
	inline string to_string(unsigned long val) {return to_string(static_cast<unsigned long long>(val));}
	inline string to_string(float val) {return to_string(static_cast<long double>(val));}
	inline string to_string(double val) {return to_string(static_cast<long double>(val));}
};
#endif

// already included in VC2012
#if defined _MSC_VER && _MSC_VER < 1700
// hash specializations for smart pointers are missed in VC++ 2010
namespace std
{
	template<class T, class D>
	struct hash<unique_ptr<T, D>>: public unary_function<unique_ptr<T, D>, size_t>
	{
		size_t operator()(const unique_ptr<T, D>& Value) const
		{
			return hash<unique_ptr<T, D>::pointer>()(Value.get());
		}
	};

	template<class T>
	struct hash<shared_ptr<T>>: public unary_function<shared_ptr<T>, size_t>
	{
		size_t operator()(const shared_ptr<T>& Value) const
		{
			return hash<T*>()(Value.get());
		}
	};
}
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
#if defined _MSC_VER && _MSC_VER < 1800
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

	#define MAKE_UNIQUE_VTE(TYPENAME_LIST, ARG_LIST, REF_ARG_LIST, FWD_ARG_LIST) \
	template<typename T, TYPENAME_LIST> \
	inline typename enable_if<!is_array<T>::value, unique_ptr<T>>::type make_unique(REF_ARG_LIST) \
	{ \
		return unique_ptr<T>(new T(FWD_ARG_LIST)); \
	}

	#include "common/variadic_emulation_helpers_begin.hpp"
	VTE_GENERATE(MAKE_UNIQUE_VTE)
	#include "common/variadic_emulation_helpers_end.hpp"

	#undef MAKE_UNIQUE_VTE
};
#endif

#if defined _MSC_VER && _MSC_VER < 1900
// already included in VC2015
#define thread_local __declspec(thread)
#endif

// already included in VC2015
#if defined _MSC_VER && _MSC_VER < 1900
#if !defined __clang__ && !defined __INTEL_COMPILER
#define noexcept throw()
#endif
#endif

// already included in VC2013
#if defined _MSC_VER && _MSC_VER < 1800
namespace std
{
	inline long long strtoll(const char* Str, char** EndPtr, int Radix) { return _strtoi64(Str, EndPtr, Radix); }
	inline long long wcstoll(const wchar_t* Str, wchar_t** EndPtr, int Radix) { return _wcstoi64(Str, EndPtr, Radix); }
	inline unsigned long long strtoull(const char* Str, char** EndPtr, int Radix) { return _strtoui64(Str, EndPtr, Radix); }
	inline unsigned long long wsctoull(const wchar_t* Str, wchar_t** EndPtr, int Radix) { return _wcstoui64(Str, EndPtr, Radix); }
}

using std::strtoll;
using std::wcstoll;
using std::strtoull;
using std::wsctoull;
#endif

// already fixed in VC2013
#if defined _MSC_VER && _MSC_VER < 1800 || defined __INTEL_COMPILER
// operator :: doesn't work with decltype(T) in VC prior to 2013, this trick fixes it:
#define decltype(T) std::enable_if<true, decltype(T)>::type
#endif
