#ifndef PREPROCESSOR_HPP_35FF3F1D_40F4_4741_9366_6A0723C14CBB
#define PREPROCESSOR_HPP_35FF3F1D_40F4_4741_9366_6A0723C14CBB
#pragma once

/*
preprocessor.hpp
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

#include "compiler.hpp"

//----------------------------------------------------------------------------

#define EXPAND(x) x

#define DETAIL_CONCATENATE_IMPL(s1, s2) s1 ## s2
#define CONCATENATE(s1, s2) DETAIL_CONCATENATE_IMPL(s1, s2)

#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __LINE__)

#define DETAIL_STD_DEFAULT_MUTATOR(Function) std::ranges::Function
#define DETAIL_STD_CONST_MUTATOR(Function) std::ranges::c##Function
#define DETAIL_STD_REVERSE_MUTATOR(Function) std::ranges::r##Function
#define DETAIL_STD_CONST_REVERSE_MUTATOR(Function) std::ranges::cr##Function


#define DETAIL_ALL_RANGE_IMPL(Object, MUTATOR_PARAM) MUTATOR_PARAM(begin)(Object), MUTATOR_PARAM(end)(Object)

#define ALL_RANGE(Object) DETAIL_ALL_RANGE_IMPL(Object, DETAIL_STD_DEFAULT_MUTATOR)
#define ALL_CONST_RANGE(Object) DETAIL_ALL_RANGE_IMPL(Object, DETAIL_STD_CONST_MUTATOR)
#define ALL_REVERSE_RANGE(Object) DETAIL_ALL_RANGE_IMPL(Object, DETAIL_STD_REVERSE_MUTATOR)
#define ALL_CONST_REVERSE_RANGE(Object) DETAIL_ALL_RANGE_IMPL(Object, DETAIL_STD_CONST_REVERSE_MUTATOR)


#define DETAIL_FOR_RANGE_IMPL(Object, i, MUTATOR_PARAM) for(auto i = MUTATOR_PARAM(begin)(Object), CONCATENATE(end, __LINE__) = MUTATOR_PARAM(end)(Object); i != CONCATENATE(end, __LINE__); ++i)

#define FOR_RANGE(Object, i) DETAIL_FOR_RANGE_IMPL(Object, i, DETAIL_STD_DEFAULT_MUTATOR)
#define FOR_CONST_RANGE(Object, i) DETAIL_FOR_RANGE_IMPL(Object, i, DETAIL_STD_CONST_MUTATOR)
#define FOR_REVERSE_RANGE(Object, i) DETAIL_FOR_RANGE_IMPL(Object, i, DETAIL_STD_REVERSE_MUTATOR)
#define FOR_CONST_REVERSE_RANGE(Object, i) DETAIL_FOR_RANGE_IMPL(Object, i, DETAIL_STD_CONST_REVERSE_MUTATOR)


#define COPY_AND_MOVE(Type, ...) \
	Type& operator=(__VA_ARGS__ rhs) { return *this = Type(rhs); }

#define COPY_CONSTRUCTIBLE(Type) \
	Type(const Type&) = default

#define NOT_COPY_CONSTRUCTIBLE(Type) \
	Type(const Type&) = delete

#define COPY_ASSIGNABLE_DEFAULT(Type) \
	Type& operator=(const Type&) = default

#define COPY_ASSIGNABLE_SWAP(Type) \
	COPY_AND_MOVE(Type, const Type&)

#define NOT_COPY_ASSIGNABLE(Type) \
	Type& operator=(const Type&) = delete

#define COPYABLE(Type) \
	COPY_ASSIGNABLE_SWAP(Type) \
	COPY_CONSTRUCTIBLE(Type) \

#define NONCOPYABLE(Type) \
	NOT_COPY_CONSTRUCTIBLE(Type); \
	NOT_COPY_ASSIGNABLE(Type)

#define MOVE_CONSTRUCTIBLE(Type) \
	Type(Type&&) = default

#define NOT_MOVE_CONSTRUCTIBLE(Type) \
	Type(Type&&) = delete

#define MOVE_ASSIGNABLE(Type) \
	Type& operator=(Type&&) = default

#define NOT_MOVE_ASSIGNABLE(Type) \
	Type& operator=(Type&&) = delete

#define MOVABLE(Type) \
	~Type() = default; \
	MOVE_CONSTRUCTIBLE(Type); \
	MOVE_ASSIGNABLE(Type)

#define NONMOVABLE(Type) \
	NOT_MOVE_CONSTRUCTIBLE(Type); \
	NOT_MOVE_ASSIGNABLE(Type)

#define POSTFIX_INCREMENT() \
	auto operator++(int) { auto Copy = *this; ++*this; return Copy; }

#define POSTFIX_DECREMENT() \
	auto operator--(int) { auto Copy = *this; --*this; return Copy; }

#define POSTFIX_OPS() \
	POSTFIX_INCREMENT() \
	POSTFIX_DECREMENT()

#define SCOPED_ACTION(RAII_type) \
const RAII_type ANONYMOUS_VARIABLE(scoped_object_)

#define DETAIL_CHAR_IMPL(x, ...) x##__VA_ARGS__
#define CHAR_S(x) DETAIL_CHAR_IMPL(x, s)
#define CHAR_SV(x) DETAIL_CHAR_IMPL(x, sv)

#define DETAIL_WIDE_IMPL(x, ...) L##x##__VA_ARGS__
#define WIDE(x) DETAIL_WIDE_IMPL(x)
#define WIDE_S(x) DETAIL_WIDE_IMPL(x, s)
#define WIDE_SV(x) DETAIL_WIDE_IMPL(x, sv)

#define LITERAL(x) #x
#define WIDE_LITERAL(x) WIDE(#x)
#define WIDE_SV_LITERAL(x) WIDE_SV(#x)

#define EXPAND_TO_LITERAL(x) LITERAL(x)
#define EXPAND_TO_WIDE_LITERAL(x) WIDE(LITERAL(x))
#define EXPAND_TO_WIDE_SV_LITERAL(x) WIDE_SV(LITERAL(x))

#define FWD(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

#define LIFT(...) [](auto&&... Args) noexcept(noexcept(__VA_ARGS__(FWD(Args)...))) -> decltype(auto) \
{ \
	return __VA_ARGS__(FWD(Args)...); \
}

#define LIFT_MF(...) [](auto&& Self, auto&&... Args) noexcept(noexcept(FWD(Self).__VA_ARGS__(FWD(Args)...))) -> decltype(auto) \
{ \
	return FWD(Self).__VA_ARGS__(FWD(Args)...); \
}

#endif // PREPROCESSOR_HPP_35FF3F1D_40F4_4741_9366_6A0723C14CBB
