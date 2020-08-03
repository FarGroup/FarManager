#ifndef DATETIME_HPP_58256A07_E483_4DB7_9DAC_DFA9D90D8A32
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

// Internal:

// Platform:
#include "platform.chrono.hpp"

// Common:
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

using time_component = unsigned int;
constexpr auto time_none = std::numeric_limits<time_component>::max();

struct detailed_time_point
{
	unsigned
		Year,
		Month,
		Day,
		Hour,
		Minute,
		Second,
		Hectonanosecond;
};

detailed_time_point parse_detailed_time_point(string_view Date, string_view Time, int DateFormat);

os::chrono::time_point ParseTimePoint(string_view Date, string_view Time, int DateFormat);
os::chrono::duration ParseDuration(string_view Date, string_view Time);

/*
FullYear:
0: Century only, 2 figures with leading zeros
1: Full, 4 or 5 figures
2: A special case: 4 or 5 figures with a leading or trailing space (depending on the locale).
   For various fixed-with edit fields (attributes, filters etc.).

   Windows supports years 1601 through 30827.
*/
void ConvertDate(os::chrono::time_point Point, string& strDateText, string& strTimeText, int TimeLength, int FullYear, bool Brief = false, bool TextMonth = false);

// (days, time)
std::tuple<string, string> ConvertDuration(os::chrono::duration Duration);

string ConvertDurationToHMS(os::chrono::duration Duration);

string MkStrFTime(string_view Format = {});

bool utc_to_local(os::chrono::time_point UtcTime, SYSTEMTIME& LocalTime);
bool local_to_utc(const SYSTEMTIME& LocalTime, os::chrono::time_point& UtcTime);

class time_check: noncopyable
{
	using clock_type = std::chrono::steady_clock;

public:
	enum class mode { delayed, immediate };

	explicit time_check(mode Mode = mode::delayed) noexcept;
	time_check(mode Mode, clock_type::duration Interval) noexcept;
	void reset(clock_type::time_point Value = clock_type::now()) const noexcept;
	bool is_time() const noexcept;
	explicit operator bool() const noexcept;

private:
	mutable clock_type::time_point m_Begin;
	const clock_type::duration m_Interval;
};


#endif // DATETIME_HPP_58256A07_E483_4DB7_9DAC_DFA9D90D8A32
