/*
pipe.cpp

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "pipe.hpp"

// Internal:
#include "exception.hpp"

// Platform:

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace pipe
{
	void read(const os::handle& Pipe, void* Data, size_t DataSize)
	{
		DWORD BytesRead;
		if (!ReadFile(Pipe.native_handle(), Data, static_cast<DWORD>(DataSize), &BytesRead, nullptr))
			throw far_exception(L"Pipe read error"sv);

		if (BytesRead != DataSize)
			throw far_exception(far::format(L"Pipe read error: {} bytes requested, but {} bytes read"sv, DataSize, BytesRead));
	}

	void read(const os::handle& Pipe, string& Data)
	{
		size_t StringSize;
		read(Pipe, &StringSize, sizeof(StringSize));

		Data.resize(StringSize);

		if (StringSize)
			read(Pipe, Data.data(), StringSize * sizeof(string::value_type));
	}

	void write(const os::handle& Pipe, const void* Data, size_t DataSize)
	{
		DWORD BytesWritten;
		if (!WriteFile(Pipe.native_handle(), Data, static_cast<DWORD>(DataSize), &BytesWritten, nullptr))
			throw far_exception(L"Pipe write error"sv);

		if (BytesWritten != DataSize)
			throw far_exception(far::format(L"Pipe write error: {} bytes sent, but {} bytes written"sv, DataSize, BytesWritten));
	}

	void write(const os::handle& Pipe, string_view const Data)
	{
		write(Pipe, Data.size());

		if (Data.size())
			write(Pipe, Data.data(), Data.size() * sizeof(string_view::value_type));
	}
}
