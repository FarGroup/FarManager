#ifndef COMMON_HPP_1BD5AB87_3379_4AFE_9F63_DB850DCF72B4
#define COMMON_HPP_1BD5AB87_3379_4AFE_9F63_DB850DCF72B4
#pragma once

/*
common.hpp

Some useful classes, templates && macros.

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

// TODO: use separately where required

#include "common/compiler.hpp"
#include "common/preprocessor.hpp"
#include "common/utility.hpp"
#include "common/noncopyable.hpp"
#include "common/smart_ptr.hpp"
#include "common/algorithm.hpp"
#include "common/string_utils.hpp"
#include "common/chrono.hpp"

// TODO: clean up & split

template <typename T>
bool CheckStructSize(const T* s)
{
	return s && (s->StructSize >= sizeof(T));
}

template <typename T>
bool CheckNullOrStructSize(const T* s)
{
	return !s || CheckStructSize(s);
}

template<class T>
auto NullToEmpty(const T* Str)
{
	static const T empty{};
	return Str? Str : &empty;
}

template<class T>
auto EmptyToNull(const T* Str)
{
	return (Str && !*Str)? nullptr : Str;
}

template <class T>
T Round(const T &a, const T &b)
{
	return a / b + ((a % b * 2 > b)? 1 : 0);
}

inline void* ToPtr(intptr_t Value)
{
	return reinterpret_cast<void*>(Value);
}

template<typename min_type, typename value_type, typename max_type>
constexpr bool InRange(min_type const Min, value_type const Value, max_type const Max)
{
	return Min <= Value && Value <= Max;
}

#ifdef _DEBUG
#define SELF_TEST(code) \
namespace \
{ \
	struct SelfTest \
	{ \
		SelfTest() \
		{ \
			code; \
		} \
	} _SelfTest; \
}
#else
#define SELF_TEST(code)
#endif

#define SIGN_UNICODE    0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8       0xBFBBEF

template<typename T>
class base: public T
{
protected:
	using T::T;
	using base_type = base;
};

namespace detail
{
	inline void from_string(const string& Str, int& Value, size_t* Pos, int Base) { Value = std::stoi(Str, Pos, Base); }
	inline void from_string(const string& Str, unsigned int& Value, size_t* Pos, int Base) { Value = std::stoul(Str, Pos, Base); }
	inline void from_string(const string& Str, long& Value, size_t* Pos, int Base) { Value = std::stol(Str, Pos, Base); }
	inline void from_string(const string& Str, unsigned long& Value, size_t* Pos, int Base) { Value = std::stoul(Str, Pos, Base); }
	inline void from_string(const string& Str, long long& Value, size_t* Pos, int Base) { Value = std::stoll(Str, Pos, Base); }
	inline void from_string(const string& Str, unsigned long long& Value, size_t* Pos, int Base) { Value = std::stoull(Str, Pos, Base); }
	inline void from_string(const string& Str, double & Value, size_t* Pos, int) { Value = std::stod(Str, Pos); }
}

template<typename T>
bool from_string(const string& Str, T& Value, size_t* Pos = nullptr, int Base = 10)
{
	try
	{
		detail::from_string(Str, Value, Pos, Base);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

#endif // COMMON_HPP_1BD5AB87_3379_4AFE_9F63_DB850DCF72B4
