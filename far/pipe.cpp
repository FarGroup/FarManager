﻿/*
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

#include "pipe.hpp"

namespace pipe
{
	static bool ReadPipe(const os::handle& Pipe, void* Data, size_t DataSize)
	{
		DWORD n;
		return ReadFile(Pipe.native_handle(), Data, static_cast<DWORD>(DataSize), &n, nullptr) && n == DataSize;
	}

	static bool WritePipe(const os::handle& Pipe, const void* Data, size_t DataSize)
	{
		DWORD n;
		return WriteFile(Pipe.native_handle(), Data, static_cast<DWORD>(DataSize), &n, nullptr) && n == DataSize;
	}

	bool Read(const os::handle& Pipe, void* Data, size_t DataSize)
	{
		size_t ReadSize = 0;
		if (!ReadPipe(Pipe, &ReadSize, sizeof(ReadSize)))
			return false;

		if (ReadSize != DataSize)
			return false;

		return ReadPipe(Pipe, Data, DataSize);
	}

	bool Read(const os::handle& Pipe, string& Data)
	{
		size_t DataSize = 0;
		if (!ReadPipe(Pipe, &DataSize, sizeof(DataSize)))
			return false;

		if (!DataSize)
			return true;

		wchar_t_ptr_n<256> Buffer(DataSize / sizeof(wchar_t));
		if (!ReadPipe(Pipe, Buffer.get(), DataSize))
			return false;

		Data.assign(Buffer.get(), Buffer.size() - 1);
		return true;
	}

	bool Write(const os::handle& Pipe, const void* Data, size_t DataSize)
	{
		return WritePipe(Pipe, &DataSize, sizeof(DataSize)) && WritePipe(Pipe, Data, DataSize);
	}

	bool Write(const os::handle& Pipe, const string& Data)
	{
		return Write(Pipe, Data.data(), (Data.size() + 1)*sizeof(wchar_t));
	}
}
