﻿#ifndef PLATFORM_CHRONO_HPP_4942BDE7_47FB_49F8_B8F6_EE0AFF4EC61D
#define PLATFORM_CHRONO_HPP_4942BDE7_47FB_49F8_B8F6_EE0AFF4EC61D
#pragma once

/*
platform.chrono.hpp

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

// Internal:

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

namespace os::chrono
{
	using hectonanoseconds = std::chrono::duration<unsigned long long, std::ratio_multiply<std::hecto, std::nano>>;

	// TrivialClock with fixed period (100 ns) and epoch (1 Jan 1601)
	class nt_clock
	{
	public:
		using duration = hectonanoseconds;
		using rep = duration::rep;
		using period = duration::period;
		using time_point = std::chrono::time_point<nt_clock>;

		static inline constexpr bool is_steady = false;

		[[nodiscard]]
		static time_point now() noexcept;

		[[nodiscard]]
		static std::time_t to_time_t(time_point Time) noexcept;

		[[nodiscard]]
		static time_point from_time_t(std::time_t Time) noexcept;

		[[nodiscard]]
		static FILETIME to_filetime(time_point Time) noexcept;

		[[nodiscard]]
		static time_point from_filetime(FILETIME Time) noexcept;

		[[nodiscard]]
		static time_point from_hectonanoseconds(int64_t Time) noexcept;

		[[nodiscard]]
		static int64_t to_hectonanoseconds(time_point Time) noexcept;

		[[nodiscard]]
		static int64_t to_hectonanoseconds(duration Duration) noexcept;
	};

	using duration = nt_clock::duration;
	using time_point = nt_clock::time_point;

	bool utc_to_local(time_point UtcTime, SYSTEMTIME& LocalTime);
	bool local_to_utc(const SYSTEMTIME& LocalTime, time_point& UtcTime);

	// Q: WTF is this, it's in the standard!
	// A: MSVC implemented it in terms of sleep_until, which is mental
	void sleep_for(std::chrono::milliseconds Duration);

	[[nodiscard]]
	bool get_process_creation_time(HANDLE Process, time_point& CreationTime);

	[[nodiscard]]
	string format_time(time_point Time);

	namespace literals
	{
		[[nodiscard]]
		constexpr auto operator"" _hns(unsigned long long const Value) noexcept
		{
			return hectonanoseconds(Value);
		}
	}
}

inline namespace literals
{
	using namespace os::chrono::literals;
}

#endif // PLATFORM_CHRONO_HPP_4942BDE7_47FB_49F8_B8F6_EE0AFF4EC61D
