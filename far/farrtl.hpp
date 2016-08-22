#ifndef FARRTL_HPP_3DC127D2_3D5C_4E0C_BFDD_6CE23AE099DB
#define FARRTL_HPP_3DC127D2_3D5C_4E0C_BFDD_6CE23AE099DB
#pragma once

/*
farrtl.hpp

Переопределение различных CRT функций
*/
/*
Copyright © 2016 Far Group
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

#ifdef _DEBUG
#define MEMCHECK
#endif

#ifdef MEMCHECK
wchar_t* DuplicateString(const wchar_t* str, const char* Function, const char* File, int Line);

void* operator new(size_t size, const char* Function, const char* File, int Line);
void* operator new(size_t size, const std::nothrow_t& nothrow_value, const char* Function, const char* File, int Line) noexcept;
void* operator new[](size_t size, const char* Function, const char* File, int Line);
void* operator new[](size_t size, const std::nothrow_t& nothrow_value, const char* Function, const char* File, int Line) noexcept;
void operator delete(void* block, const char* Function, const char* File, int Line);
void operator delete[](void* block, const char* Function, const char* File, int Line);

#define DuplicateString(str) DuplicateString(str, __FUNCTION__, __FILE__, __LINE__)
#define new new(__FUNCTION__, __FILE__, __LINE__)
#else
wchar_t* DuplicateString(const wchar_t* string);
#endif

void PrintMemory();

char* xstrncpy(char* dest, const char* src, size_t DestSize);
wchar_t* xwcsncpy(wchar_t* dest, const wchar_t* src, size_t DestSize);

constexpr size_t aligned_size(size_t Size, size_t Alignment = MEMORY_ALLOCATION_ALIGNMENT)
{
	return (Size + (Alignment - 1)) & ~(Alignment - 1);
}

template<class T, int Alignment = MEMORY_ALLOCATION_ALIGNMENT>
struct aligned_sizeof
{
	enum
	{
		value = aligned_size(sizeof(T), Alignment)
	};
};

#endif // FARRTL_HPP_3DC127D2_3D5C_4E0C_BFDD_6CE23AE099DB
