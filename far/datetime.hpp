﻿#ifndef DATETIME_HPP_58256A07_E483_4DB7_9DAC_DFA9D90D8A32
#define DATETIME_HPP_58256A07_E483_4DB7_9DAC_DFA9D90D8A32
#pragma once

/*
datetime.hpp

Функции для работы с датой и временем
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "common/range.hpp"

inline auto get_local_time() { SYSTEMTIME Time; GetLocalTime(&Time); return Time; }
inline auto get_utc_time() { SYSTEMTIME Time; GetSystemTime(&Time); return Time; }

DWORD ConvertYearToFull(DWORD ShortYear);

enum { date_none = std::numeric_limits<WORD>::max() };
using date_ranges = std::array<std::pair<size_t, size_t>, 3>;
using time_ranges = std::array<std::pair<size_t, size_t>, 4>;

void ParseDateComponents(string_view Src, range<const std::pair<size_t, size_t>*> Ranges, range<WORD*> Dst, WORD Default = date_none);
os::chrono::time_point ParseDate(const string& Date, const string& Time, int DateFormat, const date_ranges& DateRanges, const time_ranges& TimeRanges);
os::chrono::duration ParseDuration(const string& Date, const string& Time, int DateFormat, const time_ranges& TimeRanges);
void ConvertDate(os::chrono::time_point Point, string& strDateText, string& StrTimeText, int TimeLength, int Brief = FALSE, int TextMonth = FALSE, int FullYear = 0);
void ConvertDuration(os::chrono::duration Duration, string& strDaysText, string& strTimeText);

string StrFTime(const wchar_t* Format, const tm* t);
string MkStrFTime(const wchar_t* Format = nullptr);

bool Utc2Local(os::chrono::time_point UtcTime, SYSTEMTIME& LocalTime);
bool Local2Utc(const SYSTEMTIME& LocalTime, os::chrono::time_point& UtcTime);

class time_check: noncopyable
{
	using clock_type = std::chrono::steady_clock;

public:
	enum class mode { delayed, immediate };
	time_check(mode Mode, clock_type::duration Interval):
		m_Begin(Mode == mode::delayed? clock_type::now() : clock_type::now() - Interval),
		m_Interval(Interval)
	{
	}

	void reset(clock_type::time_point Value = clock_type::now()) const
	{
		m_Begin = Value;
	}

	explicit operator bool() const noexcept
	{
		const auto Current = clock_type::now();
		if (m_Interval != 0s && Current - m_Begin > m_Interval)
		{
			reset(Current);
			return true;
		}
		return false;
	}

private:
	mutable clock_type::time_point m_Begin;
	const clock_type::duration m_Interval;
};


#endif // DATETIME_HPP_58256A07_E483_4DB7_9DAC_DFA9D90D8A32
