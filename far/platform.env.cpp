/*
platform.env.cpp

*/
/*
Copyright © 2017 Far Group
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "platform.env.hpp"

// Internal:

// Platform:
#include "platform.hpp"

// Common:
#include "common/range.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::env
{
	const wchar_t* provider::detail::provider::data() const
	{
		return m_Data;
	}

	//-------------------------------------------------------------------------

	provider::strings::strings()
	{
		m_Data = GetEnvironmentStrings();
	}

	provider::strings::~strings()
	{
		if (m_Data)
		{
			FreeEnvironmentStrings(m_Data);
		}
	}

	//-------------------------------------------------------------------------

	provider::block::block()
	{
		m_Data = nullptr;
		handle TokenHandle;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &ptr_setter(TokenHandle)))
		{
			CreateEnvironmentBlock(reinterpret_cast<void**>(&m_Data), TokenHandle.native_handle(), TRUE);
		}
	}

	provider::block::~block()
	{
		if (m_Data)
		{
			DestroyEnvironmentBlock(m_Data);
		}
	}

	//-------------------------------------------------------------------------

	bool get(const string_view Name, string& Value)
	{
		last_error_guard ErrorGuard;
		null_terminated C_Name(Name);

		// GetEnvironmentVariable might return 0 not only in case of failure, but also when the variable is empty.
		// To recognise this, we set LastError to ERROR_SUCCESS manually and check it after the call,
		// which doesn't change it upon success.
		SetLastError(ERROR_SUCCESS);

		if (detail::ApiDynamicStringReceiver(Value, [&](span<wchar_t> Buffer)
		{
			return ::GetEnvironmentVariable(C_Name.c_str(), Buffer.data(), static_cast<DWORD>(Buffer.size()));
		}))
		{
			return true;
		}

		if (GetLastError() == ERROR_SUCCESS)
		{
			Value.clear();
			return true;
		}

		// Something went wrong, it's better to leave the last error as is
		ErrorGuard.dismiss();
		return false;
	}

	string get(const string_view Name)
	{
		string Result;
		get(Name, Result);
		return Result;
	}

	bool set(const string_view Name, const string_view Value)
	{
		return ::SetEnvironmentVariable(null_terminated(Name).c_str(), null_terminated(Value).c_str()) != FALSE;
	}

	bool del(const string_view Name)
	{
		return ::SetEnvironmentVariable(null_terminated(Name).c_str(), nullptr) != FALSE;
	}

	string expand(const string_view Str)
	{
		null_terminated C_Str(Str);

		bool Failure = false;

		string Result;
		if (!detail::ApiDynamicStringReceiver(Result, [&](span<wchar_t> Buffer)
		{
			// ExpandEnvironmentStrings return value always includes the terminating null character.
			// ApiDynamicStringReceiver expects a string length upon success (e.g. without the \0),
			// but we cannot simply subtract 1 from the returned value - the function can also return 1
			// when the result exists, but empty, so if we do that, it will be treated as error.
			const auto ReturnedSize = ::ExpandEnvironmentStrings(C_Str.c_str(), Buffer.data(), static_cast<DWORD>(Buffer.size()));
			switch (ReturnedSize)
			{
			case 0:
				// Failure
				Failure = true;
				return 0ul;

			case 1:
				// Empty result
				return 0ul;

			default:
				// Non-empty result
				return ReturnedSize > Buffer.size()? ReturnedSize : ReturnedSize - 1;
			}
		}))
		{
			if (Failure)
				Result = Str;
		}
		return Result;
	}

	string get_pathext()
	{
		const auto PathExt = get(L"PATHEXT"sv);
		return !PathExt.empty()? PathExt : L".COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC"s;
	}
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("platform.env")
{
	const auto Name = L"FAR_TEST_ENV_NAME"sv;
	const auto Value = L"VALUE"sv;

	string Str;

	REQUIRE(os::env::set(Name, Value));
	REQUIRE(os::env::get(Name, Str));
	REQUIRE(Str == Value);

	REQUIRE(os::env::expand(concat(L"PRE_%"sv, Name, L"%_POST"sv)) == concat(L"PRE_"sv, Value, L"_POST"sv));

	REQUIRE(os::env::set(Name, {}));
	REQUIRE(os::env::get(Name, Str));
	REQUIRE(Str.empty());

	REQUIRE(os::env::del(Name));
	REQUIRE(!os::env::get(Name, Str));

	REQUIRE(!os::env::get_pathext().empty());
}
#endif
