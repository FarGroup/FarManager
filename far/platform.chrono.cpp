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
#include "log.hpp"

// Platform:
#include "platform.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

template<>
struct formattable<SYSTEMTIME>
{
	static string to_string(SYSTEMTIME const& Time)
	{
		return far::format(L"{{{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}}}"sv,
			Time.wYear,
			Time.wMonth,
			Time.wDay,
			Time.wHour,
			Time.wMinute,
			Time.wSecond,
			Time.wMilliseconds
		);
	}
};

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
			.wMilliseconds = Time.milliseconds(),
		};
	}

	uint16_t time::milliseconds() const
	{
		return static_cast<uint16_t>(Hectonanoseconds / (1ms / 1_hns));
	}

	utc_time now_utc()
	{
		// hns precision
		if (utc_time UtcTime; timepoint_to_utc(nt_clock::now(), UtcTime))
			return UtcTime;

		// ms precision
		SYSTEMTIME SystemTime{};
		GetSystemTime(&SystemTime);
		return utc_time{ make_time(SystemTime) };
	}

	local_time now_local()
	{
		// hns precision
		if (local_time LocalTime; timepoint_to_localtime(nt_clock::now(), LocalTime))
			return LocalTime;

		// ms precision
		SYSTEMTIME LocalTime;
		GetLocalTime(&LocalTime);
		return local_time{ make_time(LocalTime) };
	}

	static bool timepoint_to_system_time(time_point const TimePoint, SYSTEMTIME& SystemTime)
	{
		const auto FileTime = nt_clock::to_filetime(TimePoint);
		return FileTimeToSystemTime(&FileTime, &SystemTime);
	}

	static bool system_time_to_timepoint(SYSTEMTIME const& SystemTime, unsigned const Hectonanoseconds, time_point& TimePoint)
	{
		FILETIME FileTime;
		if (!SystemTimeToFileTime(&SystemTime, &FileTime))
		{
			LOGWARNING(L"SystemTimeToFileTime({}): {}"sv, SystemTime, os::last_error());
			return false;
		}

		TimePoint = nt_clock::from_filetime(FileTime) + hectonanoseconds{ Hectonanoseconds } % 1ms;
		return true;
	}

	static void transfer_hns(time_point const TimePoint, time& Time)
	{
		assert(!(Time.Hectonanoseconds % (1ms / 1_hns)));
		Time.Hectonanoseconds += TimePoint.time_since_epoch() % 1ms / 1_hns;
	}

	bool timepoint_to_utc(time_point const TimePoint, utc_time& UtcTime)
	{
		SYSTEMTIME SystemTime;
		if (!timepoint_to_system_time(TimePoint, SystemTime))
			return false;

		UtcTime = utc_time{ make_time(SystemTime) };

		transfer_hns(TimePoint, UtcTime);

		return true;
	}

	bool utc_to_timepoint(utc_time const UtcTime, time_point& TimePoint)
	{
		return system_time_to_timepoint(make_system_time(UtcTime), UtcTime.Hectonanoseconds, TimePoint);
	}

	static SYSTEMTIME utc_to_local_impl(SYSTEMTIME const& UtcTime)
	{
		SYSTEMTIME LocalTime;

		if (imports.SystemTimeToTzSpecificLocalTimeEx && imports.SystemTimeToTzSpecificLocalTimeEx({}, &UtcTime, &LocalTime))
			return LocalTime;

		if (SystemTimeToTzSpecificLocalTime({}, &UtcTime, &LocalTime))
			return LocalTime;

		if (FILETIME UtcFileTime, LocalFileTime;
			SystemTimeToFileTime(&UtcTime, &UtcFileTime) &&
			FileTimeToLocalFileTime(&UtcFileTime, &LocalFileTime) &&
			FileTimeToSystemTime(&LocalFileTime, &LocalTime)
		)
			return LocalTime;

		LOGWARNING(L"Failed to convert UTC {} to local time: {}"sv, UtcTime, os::last_error());

		// Better than nothing
		return UtcTime;
	}

	bool timepoint_to_localtime(time_point const TimePoint, local_time& LocalTime)
	{
		SYSTEMTIME SystemTime;
		if (!timepoint_to_system_time(TimePoint, SystemTime))
			return false;

		const auto LocalSystemTime = utc_to_local_impl(SystemTime);

		LocalTime = local_time{ make_time(LocalSystemTime) };

		transfer_hns(TimePoint, LocalTime);

		return true;
	}

	static SYSTEMTIME local_to_utc_impl(SYSTEMTIME const& LocalTime)
	{
		SYSTEMTIME UtcTime;

		if (imports.TzSpecificLocalTimeToSystemTimeEx && imports.TzSpecificLocalTimeToSystemTimeEx({}, &LocalTime, &UtcTime))
			return UtcTime;

		if (imports.TzSpecificLocalTimeToSystemTime && imports.TzSpecificLocalTimeToSystemTime({}, &LocalTime, &UtcTime))
			return UtcTime;

		if (FILETIME LocalFileTime, UtcFileTime;
			SystemTimeToFileTime(&LocalTime, &LocalFileTime) &&
			LocalFileTimeToFileTime(&LocalFileTime, &UtcFileTime) &&
			FileTimeToSystemTime(&UtcFileTime, &UtcTime)
		)
			return UtcTime;

		LOGWARNING(L"Failed to convert local time {} to UTC: {}"sv, LocalTime, os::last_error());

		// Better than nothing
		return LocalTime;
	}

	bool localtime_to_timepoint(local_time const LocalTime, time_point& TimePoint)
	{
		return system_time_to_timepoint(local_to_utc_impl(make_system_time(LocalTime)), LocalTime.Hectonanoseconds, TimePoint);
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
		if (!timepoint_to_localtime(Time, LocalTime))
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

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("chrono.timepoint_conversions")
{
	enum direction: unsigned
	{
		to_utc     = 0_bit,
		from_utc   = 1_bit,
		to_local   = 2_bit,
		from_local = 3_bit,
		utc        = to_utc | from_utc,
		local      = to_local | from_local,
		to         = to_utc | to_local,
		from       = from_utc | from_local,
		all        = utc | local,
	};

	static constexpr struct
	{
		std::optional<os::chrono::time> Time;
		std::optional<os::chrono::duration> TimeSinceEpoch;
		unsigned Direction = direction::all;
	}
	Tests[]
	{
		{ {{  1600,  1,  1,  0,  0,  0,       0 }}, {                        },                }, // Prehistoric
		{ {{  1601,  1,  1,  0,  0,  0,       0 }}, {                  0_hns }, utc            }, // Epoch. No localtime test because it might fail in the Western Hemisphere.
		{ {{  1601,  1,  1,  0,  0,  0,       1 }}, {                  1_hns }, utc            }, // No localtime test because it might fail in the Western Hemisphere.
		{ {{  2026,  4,  6, 13, 47,  4, 1234567 }}, { 134199568241234567_hns },                },
		{ {{ 30827, 12, 31, 23, 59, 59, 9990000 }}, { 0x7fff35f4f06c58f0_hns }, utc | to_local }, // Latest possible SYSTEMTIME. No localtime test because it might resolve to 30828 in the Eastern Hemisphere.
		{ {{ 30828,  9, 14,  2, 48,  5, 4775807 }}, { 0x7fffffffffffffff_hns }, to,            }, // Latest possible FILETIME
		{ {{ 30828,  9, 14,  2, 48,  5, 4775808 }}, {                        },                }, // Post-apocalyptic
		{ {{ 65535, 13, 32, 25, 60, 60, 9999999 }}, {                        },                }, // Rubbish
	};

	for (const auto& i: Tests)
	{
		if (i.Time)
		{
			if (i.TimeSinceEpoch)
			{
				if (i.Direction & direction::from_utc)
				{
					os::chrono::time_point UtcResult;
					REQUIRE(os::chrono::utc_to_timepoint(os::chrono::utc_time{ *i.Time }, UtcResult));
					REQUIRE(i.TimeSinceEpoch == UtcResult.time_since_epoch());
				}

				if (i.Direction & direction::to_utc)
				{
					os::chrono::utc_time UtcTimeResult;
					REQUIRE(os::chrono::timepoint_to_utc(os::chrono::time_point{ *i.TimeSinceEpoch }, UtcTimeResult));
					REQUIRE(*i.Time == UtcTimeResult);
				}

				if (i.Direction & direction::to_local)
				{
					os::chrono::local_time LocalTimeResult;
					REQUIRE(os::chrono::timepoint_to_localtime(os::chrono::time_point{ *i.TimeSinceEpoch }, LocalTimeResult));

					if (i.Direction & direction::from_local)
					{
						os::chrono::time_point TimePoint;
						REQUIRE(os::chrono::localtime_to_timepoint(LocalTimeResult, TimePoint));
						REQUIRE(i.TimeSinceEpoch == TimePoint.time_since_epoch());
					}
				}
			}
			else
			{
				os::chrono::time_point UtcResult;
				REQUIRE(!os::chrono::utc_to_timepoint(os::chrono::utc_time{ *i.Time }, UtcResult));
			}
		}
		else
		{
			if (i.TimeSinceEpoch)
			{
				os::chrono::utc_time UtcTimeResult;
				REQUIRE(!os::chrono::timepoint_to_utc(os::chrono::time_point{ *i.TimeSinceEpoch }, UtcTimeResult));
			}
			else
				assert(false);
		}
	}
}
#endif
