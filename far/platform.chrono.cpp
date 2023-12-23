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

// External:

//----------------------------------------------------------------------------

namespace os::chrono
{
	nt_clock::time_point nt_clock::now() noexcept
	{
		static const auto Get = imports.GetSystemTimePreciseAsFileTime? imports.GetSystemTimePreciseAsFileTime : GetSystemTimeAsFileTime;

		FILETIME Time;
		Get(&Time);
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
		return
		{
			extract_integer<DWORD, 0>(Count),
			extract_integer<DWORD, 1>(Count)
		};
	}

	time_point nt_clock::from_filetime(FILETIME const Time) noexcept
	{
		return from_hectonanoseconds(make_integer<unsigned long long>(Time.dwLowDateTime, Time.dwHighDateTime));
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

	SYSTEMTIME now_utc()
	{
		SYSTEMTIME SystemTime{};
		GetSystemTime(&SystemTime);
		return SystemTime;
	}

	SYSTEMTIME now_local()
	{

		SYSTEMTIME LocalTime;
		GetLocalTime(&LocalTime);
		return LocalTime;
	}

	bool utc_to_local(time_point UtcTime, SYSTEMTIME& LocalTime)
	{
		const auto FileTime = nt_clock::to_filetime(UtcTime);
		SYSTEMTIME SystemTime;
		return FileTimeToSystemTime(&FileTime, &SystemTime) && SystemTimeToTzSpecificLocalTime(nullptr, &SystemTime, &LocalTime);
	}

	static bool local_to_utc(const SYSTEMTIME& lst, SYSTEMTIME& ust)
	{
		if (imports.TzSpecificLocalTimeToSystemTime && imports.TzSpecificLocalTimeToSystemTime(nullptr, &lst, &ust))
			return true;

		TIME_ZONE_INFORMATION Tz;
		if (GetTimeZoneInformation(&Tz) != TIME_ZONE_ID_INVALID)
		{
			Tz.Bias = -Tz.Bias;
			Tz.StandardBias = -Tz.StandardBias;
			Tz.DaylightBias = -Tz.DaylightBias;
			if (SystemTimeToTzSpecificLocalTime(&Tz, &lst, &ust))
				return true;
		}

		std::tm ltm
		{
			lst.wSecond,
			lst.wMinute,
			lst.wHour,
			lst.wDay,
			lst.wMonth - 1,
			lst.wYear - 1900,
			lst.wDayOfWeek,
			-1,
			-1
		};

		if (const auto gtim = std::mktime(&ltm); gtim != static_cast<time_t>(-1))
		{
			if (const auto ptm = std::gmtime(&gtim))
			{
				ust.wYear = ptm->tm_year + 1900;
				ust.wMonth = ptm->tm_mon + 1;
				ust.wDay = ptm->tm_mday;
				ust.wHour = ptm->tm_hour;
				ust.wMinute = ptm->tm_min;
				ust.wSecond = ptm->tm_sec;
				ust.wDayOfWeek = ptm->tm_wday;
				ust.wMilliseconds = lst.wMilliseconds;
				return true;
			}
		}

		FILETIME lft, uft;
		return SystemTimeToFileTime(&lst, &lft) && LocalFileTimeToFileTime(&lft, &uft) && FileTimeToSystemTime(&uft, &ust);
	}

	bool local_to_utc(const SYSTEMTIME& LocalTime, time_point& UtcTime)
	{
		SYSTEMTIME SystemUtcTime;
		if (!local_to_utc(LocalTime, SystemUtcTime))
			return false;

		FILETIME FileUtcTime;
		if (!SystemTimeToFileTime(&SystemUtcTime, &FileUtcTime))
			return false;

		UtcTime = nt_clock::from_filetime(FileUtcTime);
		return true;
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

	string wall_time(time_point const Time)
	{
		SYSTEMTIME LocalTime;
		if (!utc_to_local(Time, LocalTime))
		{
			return {};
		}

		string Value;
		// BUGBUG check result
		(void)os::detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, Value, [&](std::span<wchar_t> Buffer)
		{
			const auto ReturnedSize = ::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &LocalTime, nullptr, Buffer.data(), static_cast<int>(Buffer.size()));
			return ReturnedSize? ReturnedSize - 1 : 0;
		});
		return Value;
	}
}
