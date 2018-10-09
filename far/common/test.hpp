#ifndef TEST_HPP_C40D0453_C760_47F8_9B10_934BF1C0506E
#define TEST_HPP_C40D0453_C760_47F8_9B10_934BF1C0506E
#pragma once

/*
test.hpp
*/
/*
Copyright © 2018 Far Group
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

#ifdef _DEBUG
#define SELF_TEST(callable) \
namespace \
{ \
	struct SelfTest \
	{ \
		SelfTest() \
		{ \
			callable(); \
		} \
	} _SelfTest; \
}

namespace detail
{
	template<typename expected, typename actual>
	void basic_assert(expected&& Expected, actual&& Actual, const char* Assertion, const char* File, int Line)
	{
		if (Expected != Actual)
		{
			std::wcerr << L"[ FAIL ] " << Assertion << L' ' << File << L':' << Line << L'\n'
			           << std::boolalpha
			           << L"         " << Expected << L" != " << Actual << L'\n'
			           << std::endl;
			assert(false);
		}
	}
}

#define EXPECT_EQ(expected, actual)   detail::basic_assert(expected, actual, "EXPECT_EQ(" #expected ", " #actual ")", __FILE__, __LINE__)
#define EXPECT_TRUE(expression)       detail::basic_assert(true, expression, "EXPECT_TRUE(" #expression ")", __FILE__, __LINE__)
#define EXPECT_FALSE(expression)      detail::basic_assert(false, expression, "EXPECT_FALSE(" #expression ")", __FILE__, __LINE__)

#else
#define SELF_TEST(callable)
#define EXPECT_EQ(expected, actual)
#define EXPECT_TRUE(expression)
#define EXPECT_FALSE(expression)
#endif

#endif // TEST_HPP_C40D0453_C760_47F8_9B10_934BF1C0506E
