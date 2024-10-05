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

	static time make_time(SYSTEMTIME const Time)
	{
		return
		{
			.Year = static_cast<uint16_t>(Time.wYear),
			.Month = static_cast<uint8_t>(Time.wMonth),
			.Day = static_cast<uint8_t>(Time.wDay),
			.Hours = static_cast<uint8_t>(Time.wHour),
			.Minutes = static_cast<uint8_t>(Time.wMinute),
			.Seconds = static_cast<uint8_t>(Time.wSecond),
			.Hectonanoseconds = static_cast<uint32_t>(Time.wMilliseconds * (1ms / 1_hns)),
		};
	}

	static SYSTEMTIME make_system_time(time const Time)
	{
		return
		{
			.wYear = Time.Year,
			.wMonth = Time.Month,
			.wDay = Time.Day,
			.wHour = Time.Hours,
			.wMinute = Time.Minutes,
			.wSecond = Time.Seconds,
			.wMilliseconds = static_cast<WORD>(Time.Hectonanoseconds / (1ms / 1_hns)),
		};
	}

	utc_time now_utc()
	{
		// hns precision
		if (utc_time UtcTime; timepoint_to_utc_time(nt_clock::now(), UtcTime))
			return UtcTime;

		// ms precision
		SYSTEMTIME SystemTime{};
		GetSystemTime(&SystemTime);
		return utc_time{ make_time(SystemTime) };
	}

	local_time now_local()
	{
		// hns precision
		if (local_time LocalTime; utc_to_local(nt_clock::now(), LocalTime))
			return LocalTime;

		// ms precision
		SYSTEMTIME LocalTime;
		GetLocalTime(&LocalTime);
		return local_time{ make_time(LocalTime) };
	}

	bool timepoint_to_utc_time(time_point const TimePoint, utc_time& UtcTime)
	{
		const auto FileTime = nt_clock::to_filetime(TimePoint);

		SYSTEMTIME SystemTime;
		if (!FileTimeToSystemTime(&FileTime, &SystemTime))
			return false;

		UtcTime = utc_time{ make_time(SystemTime) };

		UtcTime.Hectonanoseconds += TimePoint.time_since_epoch() % 1ms / 1_hns;

		return true;
	}

	static bool utc_to_local_impl(SYSTEMTIME const& UtcTime, SYSTEMTIME& LocalTime)
	{
		return SystemTimeToTzSpecificLocalTime({}, &UtcTime, &LocalTime) != FALSE;
	}

	bool utc_to_local(time_point UtcTime, local_time& LocalTime)
	{
		const auto FileTime = nt_clock::to_filetime(UtcTime);

		SYSTEMTIME SystemTime;
		if (!FileTimeToSystemTime(&FileTime, &SystemTime))
			return false;

		SYSTEMTIME LocalSystemTime;
		if (!utc_to_local_impl(SystemTime, LocalSystemTime))
			return false;

		LocalTime = local_time{ make_time(LocalSystemTime) };

		LocalTime.Hectonanoseconds += UtcTime.time_since_epoch() % 1ms / 1_hns;

		return true;
	}

	static bool local_to_utc_impl(SYSTEMTIME const& LocalTime, SYSTEMTIME& UtcTime)
	{
		if (imports.TzSpecificLocalTimeToSystemTime && imports.TzSpecificLocalTimeToSystemTime(nullptr, &LocalTime, &UtcTime))
			return true;

		TIME_ZONE_INFORMATION Tz;
		if (GetTimeZoneInformation(&Tz) != TIME_ZONE_ID_INVALID)
		{
			Tz.Bias = -Tz.Bias;
			Tz.StandardBias = -Tz.StandardBias;
			Tz.DaylightBias = -Tz.DaylightBias;
			if (SystemTimeToTzSpecificLocalTime(&Tz, &LocalTime, &UtcTime))
				return true;
		}

		std::tm ltm
		{
			LocalTime.wSecond,
			LocalTime.wMinute,
			LocalTime.wHour,
			LocalTime.wDay,
			LocalTime.wMonth - 1,
			LocalTime.wYear - 1900,
			LocalTime.wDayOfWeek,
			-1,
			-1
		};

		if (const auto gtim = std::mktime(&ltm); gtim != static_cast<time_t>(-1))
		{
			if (const auto ptm = std::gmtime(&gtim))
			{
				UtcTime.wYear = ptm->tm_year + 1900;
				UtcTime.wMonth = ptm->tm_mon + 1;
				UtcTime.wDay = ptm->tm_mday;
				UtcTime.wHour = ptm->tm_hour;
				UtcTime.wMinute = ptm->tm_min;
				UtcTime.wSecond = ptm->tm_sec;
				UtcTime.wDayOfWeek = ptm->tm_wday;
				UtcTime.wMilliseconds = LocalTime.wMilliseconds;
				return true;
			}
		}

		FILETIME LocalFileTime, UtcFileTime;
		return SystemTimeToFileTime(&LocalTime, &LocalFileTime) && LocalFileTimeToFileTime(&LocalFileTime, &UtcFileTime) && FileTimeToSystemTime(&UtcFileTime, &UtcTime);
	}

	bool local_to_utc(local_time const LocalTime, time_point& UtcTime)
	{
		SYSTEMTIME SystemUtcTime;
		if (!local_to_utc_impl(make_system_time(LocalTime), SystemUtcTime))
			return false;

		FILETIME FileUtcTime;
		if (!SystemTimeToFileTime(&SystemUtcTime, &FileUtcTime))
			return false;

		UtcTime = nt_clock::from_filetime(FileUtcTime);
		UtcTime += LocalTime.Hectonanoseconds % 1000 * 1_hns;
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
		local_time LocalTime;
		if (!utc_to_local(Time, LocalTime))
		{
			return {};
		}

		string Value;
		// BUGBUG check result
		(void)os::detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, Value, [&](std::span<wchar_t> Buffer)
		{
			const auto LocalSystemTime = make_system_time(LocalTime);
			const auto ReturnedSize = ::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &LocalSystemTime, nullptr, Buffer.data(), static_cast<int>(Buffer.size()));
			return ReturnedSize? ReturnedSize - 1 : 0;
		});
		return Value;
	}
}
