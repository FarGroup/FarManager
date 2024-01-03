#ifndef LOG_HPP_FFF449CA_C49B_436C_A7B2_94E40CA80629
#define LOG_HPP_FFF449CA_C49B_436C_A7B2_94E40CA80629
#pragma once

/*
log.hpp
*/
/*
Copyright © 2021 Far Group
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
#include "common/nifty_counter.hpp"
#include "common/preprocessor.hpp"
#include "common/source_location.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace logging
{
	class engine;

	namespace detail
	{
		constexpr auto step = 0x2000'0000u;
	}

	enum class level: unsigned
	{
		off     = detail::step * 0,
		fatal   = detail::step * 1,
		error   = detail::step * 2,
		warning = detail::step * 3,
		notice  = detail::step * 4,
		info    = detail::step * 5,
		debug   = detail::step * 6,
		trace   = detail::step * 7,

		all     = std::numeric_limits<decltype(detail::step)>::max()
	};

	[[nodiscard]]
	bool filter(level Level);

	void log(string&& Str, level Level, source_location const& Location = source_location::current());

	void show();

	void configure(string_view Parameters);

	[[nodiscard]]
	bool is_log_argument(const wchar_t* Argument);

	[[nodiscard]]
	int main(string_view PipeName);

	class suppressor
	{
	public:
		NONCOPYABLE(suppressor);

		suppressor();
		~suppressor();
	};
}

#define LOG(log_level, Format, ...) \
	do \
	{ \
		if (logging::filter(log_level)) \
		{ \
			logging::log( \
				far::format(Format, ##__VA_ARGS__), \
				log_level \
			); \
		} \
	} \
	while (false) \

#define LOGFATAL(Arg, ...)   LOG(logging::level::fatal,   Arg, ##__VA_ARGS__)
#define LOGERROR(Arg, ...)   LOG(logging::level::error,   Arg, ##__VA_ARGS__)
#define LOGWARNING(Arg, ...) LOG(logging::level::warning, Arg, ##__VA_ARGS__)
#define LOGNOTICE(Arg, ...)  LOG(logging::level::notice,  Arg, ##__VA_ARGS__)
#define LOGINFO(Arg, ...)    LOG(logging::level::info,    Arg, ##__VA_ARGS__)
#define LOGDEBUG(Arg, ...)   LOG(logging::level::debug,   Arg, ##__VA_ARGS__)
#define LOGTRACE(Arg, ...)   LOG(logging::level::trace,   Arg, ##__VA_ARGS__)

NIFTY_DECLARE(logging::engine, log_engine);

#endif // LOG_HPP_FFF449CA_C49B_436C_A7B2_94E40CA80629
