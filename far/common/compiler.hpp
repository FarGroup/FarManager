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

//----------------------------------------------------------------------------

// WARNING
// Naive #ifdef _MSC_VER is a BAD WAY to check for Microsoft compiler:
// both Intel and Clang preserve this macro.

// Use #if COMPILER(%NAME%) to check for a specific compiler.
// Use IS_MICROSOFT_SDK() to check for Microsoft standard library / Windows SDK
// Use _MSC_VER only to find out its specific version.


// Known compilers
#define DETAIL_COMPILER_CL()          0
#define DETAIL_COMPILER_GCC()         1
#define DETAIL_COMPILER_INTEL()       2
#define DETAIL_COMPILER_CLANG()       3

#if defined __clang__
#define DETAIL_COMPILER_CURRENT() DETAIL_COMPILER_CLANG()
#elif defined __GNUC__
#define DETAIL_COMPILER_CURRENT() DETAIL_COMPILER_GCC()
#elif defined __INTEL_COMPILER
#define DETAIL_COMPILER_CURRENT() DETAIL_COMPILER_INTEL()
#elif defined _MSC_VER
#define DETAIL_COMPILER_CURRENT() DETAIL_COMPILER_CL()
#else
#error Unknown compiler
#endif

#define COMPILER(Name) (DETAIL_COMPILER_CURRENT() == DETAIL_COMPILER_##Name())

#ifdef _MSC_VER
#define IS_MICROSOFT_SDK() 1
#else
#define IS_MICROSOFT_SDK() 0
#endif

#if COMPILER(CL) || COMPILER(INTEL)
#define STR_PRAGMA(...) __pragma(__VA_ARGS__)
#elif COMPILER(GCC) || COMPILER(CLANG)
#define STR_PRAGMA(...) _Pragma(#__VA_ARGS__)
#else
#define STR_PRAGMA(...)
#endif

//----------------------------------------------------------------------------
#define PACK_PUSH(n) STR_PRAGMA(pack(push, n))
#define PACK_POP()   STR_PRAGMA(pack(pop))

//----------------------------------------------------------------------------
#if COMPILER(CL) || COMPILER(INTEL)
#define WARNING_PUSH(...) __pragma(warning(push, ## __VA_ARGS__))
#define WARNING_POP()     __pragma(warning(pop))
#elif COMPILER(GCC) || COMPILER(CLANG)
#define WARNING_PUSH(...) STR_PRAGMA(GCC diagnostic push)
#define WARNING_POP()     STR_PRAGMA(GCC diagnostic pop)
#else
#define WARNING_PUSH(...)
#define WARNING_POP()
#endif

//----------------------------------------------------------------------------
#if COMPILER(CL) || COMPILER(INTEL)
#define WARNING_DISABLE_MSC(id) __pragma(warning(disable: id))
#else
#define WARNING_DISABLE_MSC(id)
#endif

//----------------------------------------------------------------------------
#if COMPILER(GCC) || COMPILER(CLANG)
#define WARNING_DISABLE_GCC(id) STR_PRAGMA(GCC diagnostic ignored id)
#else
#define WARNING_DISABLE_GCC(id)
#endif

//----------------------------------------------------------------------------
#if COMPILER(CLANG)
#define WARNING_DISABLE_CLANG(id) STR_PRAGMA(clang diagnostic ignored id)
#else
#define WARNING_DISABLE_CLANG(id)
#endif

//----------------------------------------------------------------------------
#if COMPILER(CL)
#define COMPILER_NAME L"Microsoft Visual C++"
#define COMPILER_VERSION_MAJOR (_MSC_FULL_VER / 10000000)
#define COMPILER_VERSION_MINOR (_MSC_FULL_VER % 10000000 / 100000)
#define COMPILER_VERSION_PATCH (_MSC_FULL_VER % 100000)
#elif COMPILER(GCC)
#define COMPILER_NAME L"GCC"
#define COMPILER_VERSION_MAJOR __GNUC__
#define COMPILER_VERSION_MINOR __GNUC_MINOR__
#define COMPILER_VERSION_PATCH __GNUC_PATCHLEVEL__
#elif COMPILER(INTEL)
#define COMPILER_NAME L"Intel C++ Compiler"
#define COMPILER_VERSION_MAJOR (__INTEL_COMPILER / 100)
#define COMPILER_VERSION_MINOR (__INTEL_COMPILER % 100)
#define COMPILER_VERSION_PATCH __INTEL_COMPILER_UPDATE
#elif COMPILER(CLANG)
#define COMPILER_NAME L"Clang"
#define COMPILER_VERSION_MAJOR __clang_major__
#define COMPILER_VERSION_MINOR __clang_minor__
#define COMPILER_VERSION_PATCH __clang_patchlevel__
#else
#define COMPILER_NAME L"Unknown compiler"
#define COMPILER_VERSION_MAJOR 0
#define COMPILER_VERSION_MINOR 0
#define COMPILER_VERSION_PATCH 0
#endif

//----------------------------------------------------------------------------

#if COMPILER(GCC) || COMPILER(CLANG)
#define COMPILER_MESSAGE(str) STR_PRAGMA(message str)
#define COMPILER_WARNING(str) STR_PRAGMA(GCC warning str)
#define COMPILER_ERROR(str)   STR_PRAGMA(GCC error str)
#else
#define COMPILER_MESSAGE_IMPL_STR0(x) #x
#define COMPILER_MESSAGE_IMPL_STR1(x) COMPILER_MESSAGE_IMPL_STR0(x)
#define COMPILER_MESSAGE_IMPL(type, str) \
	STR_PRAGMA(message(__FILE__ "(" COMPILER_MESSAGE_IMPL_STR1(__LINE__) "): " type ": " str))

#define COMPILER_MESSAGE(str) COMPILER_MESSAGE_IMPL("message", str)
#define COMPILER_WARNING(str) COMPILER_MESSAGE_IMPL("warning", str)
#define COMPILER_ERROR(str)   COMPILER_MESSAGE_IMPL("error", str)
#endif

//----------------------------------------------------------------------------

#define DETAIL_COMPILER_VERSION(major, minor, patch) \
	((((major) * 1ull) << 32) | (((minor) * 1ull) << 16) | (((patch) * 1ull) << 0))

#define CHECK_COMPILER(compiler, major, minor, patch) \
	( \
		!COMPILER(compiler) || ( \
			DETAIL_COMPILER_VERSION(COMPILER_VERSION_MAJOR, COMPILER_VERSION_MINOR, COMPILER_VERSION_PATCH) >= \
			DETAIL_COMPILER_VERSION(major, minor, patch) \
		) \
	)

#endif // COMPILER_HPP_6A237B14_5BAA_4106_9D7F_7C7BA14A36B0
