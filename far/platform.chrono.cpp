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

#include "platform.chrono.hpp"

#include "platform.hpp"

namespace os::chrono
{
	nt_clock::time_point nt_clock::now() noexcept
	{
		FILETIME Time;
		GetSystemTimeAsFileTime(&Time);
		return from_filetime(Time);
	}

	static nt_clock::duration posix_shift()
	{
		return std::chrono::seconds{ 11644473600 };
	}

	time_t nt_clock::to_time_t(const time_point& Time) noexcept
	{
		return std::chrono::duration_cast<std::chrono::seconds>(Time.time_since_epoch() - posix_shift()).count();
	}

	time_point nt_clock::from_time_t(time_t Time) noexcept
	{
		return time_point(posix_shift() + std::chrono::seconds(Time));
	}

	FILETIME nt_clock::to_filetime(const time_point& Time) noexcept
	{
		const auto Count = Time.time_since_epoch().count();
		return { static_cast<DWORD>(Count), static_cast<DWORD>(Count >> 32) };
	}

	time_point nt_clock::from_filetime(FILETIME Time) noexcept
	{
		return time_point(duration(static_cast<unsigned long long>(Time.dwHighDateTime) << 32 | Time.dwLowDateTime));
	}

	bool get_process_creation_time(HANDLE Process, time_point& CreationTime)
	{
		FILETIME FtCreationTime, NotUsed;
		if (!GetProcessTimes(Process, &FtCreationTime, &NotUsed, &NotUsed, &NotUsed))
			return false;

		CreationTime = nt_clock::from_filetime(FtCreationTime);
		return true;
	}

	duration process_uptime()
	{
		static const auto ProcessCreationTime = []
		{
			time_point CreationTime;
			get_process_creation_time(GetCurrentProcess(), CreationTime);
			return CreationTime;
		}();

		return nt_clock::now() - ProcessCreationTime;
	}

	string format_time()
	{
		string Value;
		os::detail::ApiDynamicStringReceiver(Value, [&](wchar_t* Buffer, size_t const Size)
		{
			const auto ReturnedSize = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, nullptr, nullptr, Buffer, static_cast<int>(Size));
			return ReturnedSize? ReturnedSize - 1 : 0;
		});
		return Value;
	}
}
