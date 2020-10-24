/*
platform.chrono.cpp

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
#include "platform.chrono.hpp"

// Internal:
#include "imports.hpp"

// Platform:
#include "platform.hpp"

// Common:
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::chrono
{
	nt_clock::time_point nt_clock::now() noexcept
	{
		FILETIME Time;
		(imports.GetSystemTimePreciseAsFileTime? imports.GetSystemTimePreciseAsFileTime : GetSystemTimeAsFileTime)(&Time);
		return from_filetime(Time);
	}

	static constexpr nt_clock::duration posix_shift()
	{
		return 3234576h;
	}

	std::time_t nt_clock::to_time_t(time_point const Time) noexcept
	{
		return (Time.time_since_epoch() - posix_shift()) / 1s;
	}

	time_point nt_clock::from_time_t(std::time_t const Time) noexcept
	{
		return time_point(posix_shift() + std::chrono::seconds(Time));
	}

	FILETIME nt_clock::to_filetime(time_point const Time) noexcept
	{
		const auto Count = to_hectonanoseconds(Time);
		return { static_cast<DWORD>(Count), static_cast<DWORD>(Count >> 32) };
	}

	time_point nt_clock::from_filetime(FILETIME const Time) noexcept
	{
		return from_hectonanoseconds(static_cast<unsigned long long>(Time.dwHighDateTime) << 32 | Time.dwLowDateTime);
	}

	time_point nt_clock::from_hectonanoseconds(int64_t const Time) noexcept
	{
		return time_point(hectonanoseconds(Time));
	}

	int64_t nt_clock::to_hectonanoseconds(time_point const Time) noexcept
	{
		return to_hectonanoseconds(Time.time_since_epoch());
	}

	int64_t nt_clock::to_hectonanoseconds(duration const Duration) noexcept
	{
		return Duration / 1_hns;
	}

	void sleep_for(std::chrono::milliseconds const Duration)
	{
		Sleep(static_cast<DWORD>(Duration / 1ms));
	}

	bool get_process_creation_time(HANDLE Process, time_point& CreationTime)
	{
		FILETIME FtCreationTime, NotUsed;
		if (!GetProcessTimes(Process, &FtCreationTime, &NotUsed, &NotUsed, &NotUsed))
			return false;

		CreationTime = nt_clock::from_filetime(FtCreationTime);
		return true;
	}

	string format_time()
	{
		string Value;
		// BUGBUG check result
		(void)os::detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, Value, [&](span<wchar_t> Buffer)
		{
			const auto ReturnedSize = ::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, nullptr, nullptr, Buffer.data(), static_cast<int>(Buffer.size()));
			return ReturnedSize? ReturnedSize - 1 : 0;
		});
		return Value;
	}
}
