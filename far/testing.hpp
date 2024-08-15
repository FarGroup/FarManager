#ifndef TESTING_HPP_DF49B287_DB16_4C12_AB55_9D6F14D3A409
#define TESTING_HPP_DF49B287_DB16_4C12_AB55_9D6F14D3A409
#pragma once

/*
testing.hpp

Testing framework wrapper

*/
/*
Copyright © 2019 Far Group
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

#ifdef ENABLE_TESTS

#ifndef TESTS_ENTRYPOINT_ONLY

#include "disable_warnings_in_std_begin.hpp"

WARNING_PUSH()

WARNING_DISABLE_MSC(4459) // declaration of 'identifier' hides global declaration
WARNING_DISABLE_MSC(4800) // Implicit conversion from 'type' to bool. Possible information loss
WARNING_DISABLE_MSC(5220) // 'name': a non-static data member with a volatile qualified type no longer implies that compiler generated copy/move constructors and copy/move assignment operators are not trivial
WARNING_DISABLE_MSC(5267) // definition of implicit assignment operator for 'class' is deprecated because it has a user-provided destructor

WARNING_DISABLE_GCC("-Wctor-dtor-privacy")
WARNING_DISABLE_GCC("-Wdouble-promotion")
WARNING_DISABLE_GCC("-Wnon-virtual-dtor")
WARNING_DISABLE_GCC("-Wredundant-decls")
WARNING_DISABLE_GCC("-Wsubobject-linkage")

WARNING_DISABLE_CLANG("-Weverything")

#define CATCH_AMALGAMATED_CUSTOM_MAIN
#define CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS
// It's rubbish
#define CATCH_CONFIG_NO_WINDOWS_SEH

#ifdef _M_ARM64
#define CATCH_CONFIG_NO_MSVC_UMUL128
#endif

#include "thirdparty/catch2/catch_amalgamated.hpp"
#ifdef CATCH_CONFIG_RUNNER
#include "thirdparty/catch2/catch_amalgamated.cpp"
#endif

WARNING_POP()

#include "disable_warnings_in_std_end.hpp"

class generic_exception_matcher: public Catch::Matchers::MatcherGenericBase
{
public:
	explicit generic_exception_matcher(std::function<bool(std::any const&)> Matcher);

	bool match(std::any const& e) const;
	std::string describe() const override;

private:
	std::function<bool(std::any const&)> m_Matcher;
};

#define STATIC_REQUIRE_ERROR(Type, ...) \
	do \
	{ \
		constexpr auto Result = []<typename TestType>(){ return requires { __VA_ARGS__; }; }.template operator()<Type>(); \
		STATIC_REQUIRE_FALSE(Result); \
	} \
	while (false)

#endif

std::optional<int> testing_main(std::span<wchar_t const* const> Args);

#endif

#endif // TESTING_HPP_DF49B287_DB16_4C12_AB55_9D6F14D3A409
