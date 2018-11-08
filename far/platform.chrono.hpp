#ifndef PLATFORM_CHRONO_HPP_4942BDE7_47FB_49F8_B8F6_EE0AFF4EC61D
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

namespace os::chrono
{
	// TrivialClock with fixed period (100 ns) and epoch (1 Jan 1601)
	class nt_clock
	{
	public:
		using rep = unsigned long long;
		using period = std::ratio_multiply<std::ratio<100, 1>, std::nano>;
		using duration = std::chrono::duration<rep, period>;
		using time_point = std::chrono::time_point<nt_clock>;
		static constexpr bool is_steady = false;
		static time_point now() noexcept;

		static time_t to_time_t(const time_point& Time) noexcept;
		static time_point from_time_t(time_t Time) noexcept;
		static FILETIME to_filetime(const time_point& Time) noexcept;
		static time_point from_filetime(FILETIME Time) noexcept;

		static void sleep_for_ms(size_t Count);
	};

	using duration = nt_clock::duration;
	using time_point = nt_clock::time_point;

	template<class rep, class period>
	void sleep_for(const std::chrono::duration<rep, period>& Duration)
	{
		nt_clock::sleep_for_ms(std::chrono::duration_cast<std::chrono::milliseconds>(Duration).count());
	}

	bool get_process_creation_time(HANDLE Process, time_point& CreationTime);

	string format_time();
}
#endif // PLATFORM_CHRONO_HPP_4942BDE7_47FB_49F8_B8F6_EE0AFF4EC61D
