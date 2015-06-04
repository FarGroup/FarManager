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

#define _STD_DEFAULT_MUTATOR(Function) std::Function
#define _STD_CONST_MUTATOR(Function) std::c##Function
#define _STD_REVERSE_MUTATOR(Function) std::r##Function
#define _STD_CONST_REVERSE_MUTATOR(Function) std::cr##Function


#define _REFERENCE_IMPL(Object, MUTATOR_PARAM) decltype(*MUTATOR_PARAM(begin)(Object))

#define REFERENCE(Object) _REFERENCE_IMPL(Object, _STD_DEFAULT_MUTATOR)
#define CONST_REFERENCE(Object) _REFERENCE_IMPL(Object, _STD_CONST_MUTATOR)


#define _VALUE_TYPE_IMPL(Object, REFERENCE_PARAM) std::remove_reference<REFERENCE_PARAM(Object)>::type

#define VALUE_TYPE(Object) _VALUE_TYPE_IMPL(Object, REFERENCE)
#define CONST_VALUE_TYPE(Object) _VALUE_TYPE_IMPL(Object, CONST_REFERENCE)


#define _ITERATOR_IMPL(Object, MUTATOR_PARAM) decltype(MUTATOR_PARAM(begin)(Object))

#define ITERATOR(Object) _ITERATOR_IMPL(Object, _STD_DEFAULT_MUTATOR)
#define CONST_ITERATOR(Object) _ITERATOR_IMPL(Object, _STD_CONST_MUTATOR)
#define REVERSE_ITERATOR(Object) _ITERATOR_IMPL(Object, _STD_REVERSE_MUTATOR)
#define CONST_REVERSE_ITERATOR(Object) _ITERATOR_IMPL(Object, _STD_CONST_REVERSE_MUTATOR)


#define _LAMBDA_PREDICATE_IMPL(Object, i, REFERENCE_PARAM, ...) [&](REFERENCE_PARAM(Object) i, ##__VA_ARGS__)

#define LAMBDA_PREDICATE(Object, i, ...) _LAMBDA_PREDICATE_IMPL(Object, i, REFERENCE, ##__VA_ARGS__)
#define CONST_LAMBDA_PREDICATE(Object, i, ...) _LAMBDA_PREDICATE_IMPL(Object, i, CONST_REFERENCE, ##__VA_ARGS__)


#define _ALL_RANGE_IMPL(Object, MUTATOR_PARAM) MUTATOR_PARAM(begin)(Object), MUTATOR_PARAM(end)(Object)

#define ALL_RANGE(Object) _ALL_RANGE_IMPL(Object, _STD_DEFAULT_MUTATOR)
#define ALL_CONST_RANGE(Object) _ALL_RANGE_IMPL(Object, _STD_CONST_MUTATOR)
#define ALL_REVERSE_RANGE(Object) _ALL_RANGE_IMPL(Object, _STD_REVERSE_MUTATOR)
#define ALL_CONST_REVERSE_RANGE(Object) _ALL_RANGE_IMPL(Object, _STD_CONST_REVERSE_MUTATOR)


#define RANGE(Object, i, ...) ALL_RANGE(Object), LAMBDA_PREDICATE(Object, i, ##__VA_ARGS__)
#define CONST_RANGE(Object, i, ...) ALL_CONST_RANGE(Object), CONST_LAMBDA_PREDICATE(Object, i, ##__VA_ARGS__)
#define REVERSE_RANGE(Object, i, ...) ALL_REVERSE_RANGE(Object), LAMBDA_PREDICATE(Object, i, ##__VA_ARGS__)
#define CONST_REVERSE_RANGE(Object, i, ...) ALL_CONST_REVERSE_RANGE(Object), CONST_LAMBDA_PREDICATE(Object, i, ##__VA_ARGS__)


#define _FOR_RANGE_IMPL(Object, i, MUTATOR_PARAM) for(auto i = MUTATOR_PARAM(begin)(Object), ADD_SUFFIX(end, __LINE__) = MUTATOR_PARAM(end)(Object); i != ADD_SUFFIX(end, __LINE__); ++i)

#define FOR_RANGE(Object, i) _FOR_RANGE_IMPL(Object, i, _STD_DEFAULT_MUTATOR)
#define FOR_CONST_RANGE(Object, i) _FOR_RANGE_IMPL(Object, i, _STD_CONST_MUTATOR)
#define FOR_REVERSE_RANGE(Object, i) _FOR_RANGE_IMPL(Object, i, _STD_REVERSE_MUTATOR)
#define FOR_CONST_REVERSE_RANGE(Object, i) _FOR_RANGE_IMPL(Object, i, _STD_CONST_REVERSE_MUTATOR)


#define COPY_OPERATOR_BY_SWAP(Type) \
Type& operator=(const Type& rhs) { Type(rhs).swap(*this); return *this; }

#define MOVE_OPERATOR_BY_SWAP(Type) \
Type& operator=(Type&& rhs) noexcept { swap(rhs); return *this; }

#define FREE_SWAP(Type) \
friend inline void swap(Type& a, Type& b) noexcept { a.swap(b); }

#define SCOPED_ACTION(RAII_type) \
const RAII_type ADD_SUFFIX(scoped_object_, __LINE__)

//----------------------------------------------------------------------------
#ifdef __GNUC__
#define GCC_STR_PRAGMA(x) _Pragma(#x)
#endif
//----------------------------------------------------------------------------
#ifdef __GNUC__
#define PACK_PUSH(n) GCC_STR_PRAGMA(pack(n))
#define PACK_POP() GCC_STR_PRAGMA(pack())
#endif

#ifdef _MSC_VER
#define PACK_PUSH(n) __pragma(pack(push, n))
#define PACK_POP() __pragma(pack(pop))
#endif
//----------------------------------------------------------------------------
#ifdef __GNUC__
#define WARNING_PUSH(...) GCC_STR_PRAGMA(GCC diagnostic push)
#define WARNING_POP() GCC_STR_PRAGMA(GCC diagnostic pop)
#endif

#ifdef _MSC_VER
#define WARNING_PUSH(...) __pragma(warning(push, __VA_ARGS__))
#define WARNING_POP() __pragma(warning(pop))
#endif
//----------------------------------------------------------------------------
#ifdef __GNUC__
#define WARNING_DISABLE_GCC(id) GCC_STR_PRAGMA(GCC diagnostic ignored id)
#else
#define WARNING_DISABLE_GCC(id)
#endif
//----------------------------------------------------------------------------
#ifdef _MSC_VER
#define WARNING_DISABLE_MSC(id) __pragma(warning(disable: id))
#else
#define WARNING_DISABLE_MSC(id)
#endif
//----------------------------------------------------------------------------


#if defined _MSC_VER && _MSC_VER < 1800
#define DELETED_FUNCTION(...) private: __VA_ARGS__
#else
#define DELETED_FUNCTION(...) __VA_ARGS__ = delete
#endif

#define STR(x) #x
#define WSTR(x) L###x
