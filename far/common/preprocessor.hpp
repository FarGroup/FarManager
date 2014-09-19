#pragma once

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

#define _ADD_SUFFIX(s, suffix) s ## suffix
#define ADD_SUFFIX(s, suffix) _ADD_SUFFIX(s, suffix)

#define PTRTYPE(T) std::remove_pointer<decltype(T)>::type

#define VALUE_TYPE(T) std::remove_reference<decltype(*std::begin(T))>::type
#define CONST_VALUE_TYPE(T) std::remove_reference<decltype(*std::cbegin(T))>::type

#define DECLTYPEOF(T, subtype) std::remove_reference<typename std::remove_pointer<decltype(T)>::type>::type::subtype
#define ITERATOR(T) DECLTYPEOF(T, iterator)
#define CONST_ITERATOR(T) DECLTYPEOF(T, const_iterator)
#define REVERSE_ITERATOR(T) DECLTYPEOF(T, reverse_iterator)
#define CONST_REVERSE_ITERATOR(T) DECLTYPEOF(T, const_reverse_iterator)

#ifdef __GNUC__
#define T_VALUE_TYPE(T) typename VALUE_TYPE(T)
#define T_CONST_VALUE_TYPE(T) typename CONST_VALUE_TYPE(T)
#else
#define T_VALUE_TYPE(T) VALUE_TYPE(T)
#define T_CONST_VALUE_TYPE(T) CONST_VALUE_TYPE(T)
#endif

#define LAMBDA_PREDICATE(T, i, ...) [&](T_VALUE_TYPE(T)& i, ##__VA_ARGS__)
#define CONST_LAMBDA_PREDICATE(T, i, ...) [&](T_CONST_VALUE_TYPE(T)& i, ##__VA_ARGS__)

#define ALL_RANGE(T) std::begin(T), std::end(T)
#define ALL_REVERSE_RANGE(T) std::rbegin(T), std::rend(T)
#define ALL_CONST_RANGE(T) std::cbegin(T), std::cend(T)
#define ALL_CONST_REVERSE_RANGE(T) std::crbegin(T), std::crend(T)

#define RANGE(T, i, ...) ALL_RANGE(T), LAMBDA_PREDICATE(T, i, ##__VA_ARGS__)
#define REVERSE_RANGE(T, i, ...) ALL_REVERSE_RANGE(T), LAMBDA_PREDICATE(T, i, ##__VA_ARGS__)
#define CONST_RANGE(T, i, ...) ALL_CONST_RANGE(T), CONST_LAMBDA_PREDICATE(T, i, ##__VA_ARGS__)
#define CONST_REVERSE_RANGE(T, i, ...) ALL_CONST_REVERSE_RANGE(T), CONST_LAMBDA_PREDICATE(T, i, ##__VA_ARGS__)

#define FOR_RANGE(T, i) for(auto i = std::begin(T), ADD_SUFFIX(end, __LINE__) = std::end(T); i != ADD_SUFFIX(end, __LINE__); ++i)
#define FOR_REVERSE_RANGE(T, i) for(auto i = std::rbegin(T), ADD_SUFFIX(rend, __LINE__) = std::rand(T); i != ADD_SUFFIX(rend, __LINE__); ++i)
#define FOR_CONST_RANGE(T, i) for(auto i = std::cbegin(T), ADD_SUFFIX(cend, __LINE__) = std::cend(T); i != ADD_SUFFIX(cend, __LINE__); ++i)
#define FOR_CONST_REVERSE_RANGE(T, i) for(auto i = std::crbegin(T), ADD_SUFFIX(crend, __LINE__) = std::crend(T); i != ADD_SUFFIX(crend, __LINE__); ++i)


#define COPY_OPERATOR_BY_SWAP(Type) \
Type& operator=(const Type& rhs) { Type t(rhs); swap(t); return *this; }

#define MOVE_OPERATOR_BY_SWAP(Type) \
Type& operator=(Type&& rhs) noexcept { swap(rhs); return *this; }

#define STD_SWAP_SPEC(Type) \
namespace std \
{ \
	template<> \
	inline void swap(Type& a, Type& b) \
	{ \
		a.swap(b); \
	} \
}

#define ALLOW_SWAP_ACCESS(Type) \
friend void std::swap<Type>(Type&, Type&);

#define SCOPED_ACTION(RAII_type) \
RAII_type ADD_SUFFIX(scoped_object_, __LINE__)
