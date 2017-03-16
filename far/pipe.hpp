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

namespace pipe
{
	// reads size first, then data
	bool Read(const os::handle& Pipe, void* Data, size_t DataSize);

	template<typename T>
	bool Read(const os::handle& Pipe, T& Data)
	{
		static_assert(!std::is_pointer<T>::value);
		static_assert(std::is_trivially_copyable<T>::value);

		return Read(Pipe, &Data, sizeof(Data));
	}

	bool Read(const os::handle& Pipe, string& Data);

	// writes size first, then data
	bool Write(const os::handle& Pipe, const void* Data, size_t DataSize);

	template<typename T>
	bool Write(const os::handle& Pipe, const T& Data)
	{
		static_assert(!std::is_pointer<T>::value);
		static_assert(std::is_trivially_copyable<T>::value);

		return Write(Pipe, &Data, sizeof(Data));
	}

	bool Write(const os::handle& Pipe, const string& Data);
}

#endif // PIPE_HPP_C460EC90_9861_4D55_B47D_D1E8F6EEBC78
