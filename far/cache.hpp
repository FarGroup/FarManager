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

class CachedRead: noncopyable
{
public:
	CachedRead(os::fs::file& file, size_t buffer_size = 0);
	~CachedRead();
	void AdjustAlignment(); // file have to be opened already
	bool Read(void* Data, size_t DataSize, size_t* BytesRead);
	bool FillBuffer();
	bool Unread(size_t BytesUnread);
	void Clear();

private:
	os::fs::file& file;
	size_t ReadSize;
	size_t BytesLeft;
	uint64_t LastPtr;
	int Alignment;
	std::vector<char> Buffer; // = 2*k*Alignment (k >= 2)
};


class CachedWrite: noncopyable
{
public:
	CachedWrite(os::fs::file& file);
	~CachedWrite();
	bool Write(const void* Data, size_t DataSize);
	bool Flush();

private:
	os::fs::file& file;
	std::vector<char> Buffer;
	size_t FreeSize;
	bool Flushed;
};
