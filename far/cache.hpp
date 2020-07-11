#ifndef CACHE_HPP_2D98721D_C727_4F3B_86A2_BEDD0B1D6D8A
#define CACHE_HPP_2D98721D_C727_4F3B_86A2_BEDD0B1D6D8A
#pragma once

/*
cache.hpp

Кеширование записи в файл/чтения из файла
*/
/*
Copyright © 2009 Far Group
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
#include "platform.fwd.hpp"

// Platform:

// Common:
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

class CachedRead: noncopyable
{
public:
	explicit CachedRead(os::fs::file& File, size_t BufferSize = 0);
	void AdjustAlignment(); // file has to be opened already
	bool Read(void* Data, size_t DataSize, size_t* BytesRead);
	bool Unread(size_t BytesUnread);
	void Clear();

private:
	bool FillBuffer();

	os::fs::file& m_File;
	size_t m_ReadSize{};
	size_t m_BytesLeft{};
	unsigned long long m_LastPtr{};
	int m_Alignment;
	std::vector<std::byte> m_Buffer; // = 2*k*Alignment (k >= 2)
};

#endif // CACHE_HPP_2D98721D_C727_4F3B_86A2_BEDD0B1D6D8A
