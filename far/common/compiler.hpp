#ifndef COMPILER_HPP_6A237B14_5BAA_4106_9D7F_7C7BA14A36B0
#define COMPILER_HPP_6A237B14_5BAA_4106_9D7F_7C7BA14A36B0
#pragma once

/*
compiler.hpp

Compiler-specific macros and definitions
*/
/*
Copyright © 2015 Far Group
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

// WARNING
// Naive #ifdef _MSC_VER is a BAD WAY to check for Microsoft compiler:
// both Intel and Clang preserve this macro.

// Use #if COMPILER == C_%NAME% to check for specific compiler.
// Use _MSC_VER only to find out its specific version or to check for Microsoft standard library / Windows SDK


// Known compilers
#define C_CL          0
#define C_GCC         1
#define C_INTEL       2
#define C_CLANG       3

#if defined __GNUC__
#define COMPILER C_GCC
#elif defined __clang__
#define COMPILER C_CLANG
#elif defined __INTEL_COMPILER
#define COMPILER C_INTEL
#elif defined _MSC_VER
#define COMPILER C_CL
#else
#error Unknown compiler
#endif

#define VS_2012 1700
#define VS_2013 1800
#define VS_2015 1900
#define VS_OLDER_THAN(version) (defined _MSC_VER && _MSC_VER < version)


#if COMPILER == C_GCC || COMPILER == C_CLANG
#define STR_PRAGMA(x) _Pragma(#x)
#endif
//----------------------------------------------------------------------------
#if COMPILER == C_CL || COMPILER == C_INTEL
#define PACK_PUSH(n) __pragma(pack(push, n))
#define PACK_POP() __pragma(pack(pop))
#elif COMPILER == C_GCC || COMPILER == C_CLANG
#define PACK_PUSH(n) STR_PRAGMA(pack(n))
#define PACK_POP() STR_PRAGMA(pack())
#endif
//----------------------------------------------------------------------------
#if COMPILER == C_CL || COMPILER == C_INTEL
#define WARNING_PUSH(...) __pragma(warning(push, __VA_ARGS__))
#define WARNING_POP() __pragma(warning(pop))
#elif COMPILER == C_GCC || COMPILER == C_CLANG
#define WARNING_PUSH(...) STR_PRAGMA(GCC diagnostic push)
#define WARNING_POP() STR_PRAGMA(GCC diagnostic pop)
#endif
//----------------------------------------------------------------------------
#if COMPILER == C_CL || COMPILER == C_INTEL
#define WARNING_DISABLE_MSC(id) __pragma(warning(disable: id))
#else
#define WARNING_DISABLE_MSC(id)
#endif
//----------------------------------------------------------------------------
#if COMPILER == C_GCC || COMPILER == C_CLANG
#define WARNING_DISABLE_GCC(id) STR_PRAGMA(GCC diagnostic ignored id)
#else
#define WARNING_DISABLE_GCC(id)
#endif
//----------------------------------------------------------------------------
#if COMPILER == C_CLANG
#define WARNING_DISABLE_CLANG(id) STR_PRAGMA(clang diagnostic ignored id)
#else
#define WARNING_DISABLE_CLANG(id)
#endif

#if COMPILER == C_CL && VS_OLDER_THAN(VS_2013)
#define NO_VARIADIC_TEMPLATES
#define NO_EXPLICIT_CONVERSION_OPERATORS
#define NO_DEFAULTED_FUNCTIONS
#define NO_DELETED_FUNCTIONS
#endif

#endif // COMPILER_HPP_6A237B14_5BAA_4106_9D7F_7C7BA14A36B0
