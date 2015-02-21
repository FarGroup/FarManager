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
	bool Read(HANDLE Pipe, void* Data, size_t DataSize);

	template<typename T>
	bool Read(HANDLE Pipe, T& Data)
	{
		static_assert(!std::is_pointer<T>::value, "This template requires a reference to an object");
		static_assert(std::is_pod<T>::value, "This template requires a POD type"); // TODO: replace with is_trivially_copyable

		return Read(Pipe, &Data, sizeof(Data));
	}

	bool Read(HANDLE Pipe, string& Data);

	// writes size first, then data
	bool Write(HANDLE Pipe, const void* Data, size_t DataSize);

	template<typename T>
	bool Write(HANDLE Pipe, const T& Data)
	{
		static_assert(!std::is_pointer<T>::value, "This template requires a reference to an object");
		static_assert(std::is_pod<T>::value, "This template requires a POD type"); // TODO: replace with is_trivially_copyable

		return Write(Pipe, &Data, sizeof(Data));
	}

	bool Write(HANDLE Pipe, const string& Data);
}
