#ifndef PIPE_HPP_C460EC90_9861_4D55_B47D_D1E8F6EEBC78
#define PIPE_HPP_C460EC90_9861_4D55_B47D_D1E8F6EEBC78
#pragma once

/*
pipe.hpp

Pipe-based IPC
*/
/*
Copyright © 2014 Far Group
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
#include "platform.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

namespace pipe
{
	namespace detail
	{
		template<typename T>
		concept supported_type = std::conjunction_v<
			std::negation<std::is_pointer<T>>,
			std::is_trivially_copyable<T>
		>;

		template<typename... args>
		concept supported_types = sizeof...(args) > 1 && std::conjunction_v<
			std::negation<std::is_pointer<args>>...
		>;

	}

	void read(const os::handle& Pipe, void* Data, size_t DataSize);

	void read(const os::handle& Pipe, detail::supported_type auto& Data)
	{
		read(Pipe, &Data, sizeof(Data));
	}

	void read(const os::handle& Pipe, string& Data);

	template<typename... args> requires detail::supported_types<args...>
	void read(const os::handle& Pipe, args&... Args)
	{
		(..., read(Pipe, Args));
	}

	void write(const os::handle& Pipe, const void* Data, size_t DataSize);

	void write(const os::handle& Pipe, detail::supported_type auto const& Data)
	{
		write(Pipe, &Data, sizeof(Data));
	}

	void write(const os::handle& Pipe, string_view Data);

	template<typename... args> requires detail::supported_types<args...>
	void write(const os::handle& Pipe, args const&... Args)
	{
		(..., write(Pipe, Args));
	}

}

#endif // PIPE_HPP_C460EC90_9861_4D55_B47D_D1E8F6EEBC78
