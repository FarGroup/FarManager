/*
platform.version.cpp

*/
/*
Copyright © 2020 Far Group
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
#include "platform.version.hpp"

// Internal:

// Platform:

// Common:
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace os::version
{
	template<class T>
	static auto get_value(std::vector<std::byte> const& Data, const string_view SubBlock)
	{
		unsigned Length;
		T* Result;
		return VerQueryValue(Data.data(), null_terminated(SubBlock).c_str(), reinterpret_cast<void**>(&Result), &Length) && Length? Result : nullptr;
	}

	bool file_version::read(string_view const Filename)
	{
		null_terminated const c_Name(Filename);

		const auto Size = GetFileVersionInfoSize(c_Name.c_str(), nullptr);
		if (!Size)
			return false;

		m_Buffer.resize(Size);

		if (!GetFileVersionInfo(c_Name.c_str(), 0, Size, m_Buffer.data()))
			return false;

		const auto Translation = get_value<DWORD>(m_Buffer, L"\\VarFileInfo\\Translation"sv);
		if (!Translation)
			return false;

		m_BlockPath = format(FSTR(L"\\StringFileInfo\\{0:04X}{1:04X}\\"), LOWORD(*Translation), HIWORD(*Translation));
		return true;
	}

	wchar_t const* file_version::get_string(string_view const Value) const
	{
		return get_value<wchar_t>(m_Buffer, m_BlockPath + Value);
	}

	VS_FIXEDFILEINFO const* file_version::get_fixed_info() const
	{
		return get_value<VS_FIXEDFILEINFO>(m_Buffer, L"\\"sv);
	}


	template<DWORD... Components>
	static unsigned long long condition_mask(DWORD const Operation)
	{
		return (... | VerSetConditionMask(0, Components, Operation));
	}

	bool is_win10_build_or_later(DWORD const Build)
	{
		static const auto Result = [&]
		{
			OSVERSIONINFOEXW osvi{ sizeof(osvi), HIBYTE(_WIN32_WINNT_WIN10), LOBYTE(_WIN32_WINNT_WIN10), Build };
				const auto ConditionMask = condition_mask<VER_MAJORVERSION, VER_MINORVERSION, VER_BUILDNUMBER>(VER_GREATER_EQUAL);
				return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, ConditionMask) != FALSE;
		}();

		return Result;
	}
}
