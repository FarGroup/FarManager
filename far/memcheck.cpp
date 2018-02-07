/*
memcheck.cpp

Memory leak detector
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

#include "headers.hpp"
#pragma hdrstop

#include "memcheck.hpp"
#include "strmix.hpp"
#include "encoding.hpp"
#include "string_utils.hpp"
#include "exception.hpp"
#include "platform.concurrency.hpp"

#ifdef MEMCHECK

#undef new

namespace memcheck
{
static intptr_t CallNewDeleteVector = 0;
static intptr_t CallNewDeleteScalar = 0;
static size_t AllocatedMemoryBlocks = 0;
static size_t AllocatedMemorySize = 0;
static size_t TotalAllocationCalls = 0;
static std::atomic<bool> MonitoringEnabled = true;

enum class allocation_type: unsigned
{
	scalar = 0xa75ca1ae,
	vector = 0xa77ec10e,
};

struct alignas(MEMORY_ALLOCATION_ALIGNMENT) MEMINFO
{
	allocation_type AllocationType;
	int Line;
	const char* File;
	const char* Function;
	size_t Size;
	MEMINFO* prev;
	MEMINFO* next;
};

static_assert(alignof(MEMINFO) == MEMORY_ALLOCATION_ALIGNMENT);

static MEMINFO FirstMemBlock = {};
static MEMINFO* LastMemBlock = &FirstMemBlock;

static auto ToReal(void* address) { return static_cast<MEMINFO*>(address) - 1; }
static void* ToUser(MEMINFO* address) { return address + 1; }

static void CheckChain()
{
#if 0
	auto p = &FirstMemBlock;

	while(p->next)
		p = p->next;
	assert(p==LastMemBlock);

	while(p->prev)
		p = p->prev;
	assert(p==&FirstMemBlock);
#endif
}

static void updateCallCount(allocation_type type, bool increment)
{
	int op = increment? 1 : -1;
	switch(type)
	{
	case allocation_type::scalar: CallNewDeleteScalar += op; break;
	case allocation_type::vector: CallNewDeleteVector += op; break;
	default:
		throw MAKE_FAR_EXCEPTION(L"Unknown allocation type");
	}
}

static const int EndMarker = 0xDEADBEEF;

static int& GetMarker(MEMINFO* Info)
{
	return *reinterpret_cast<int*>(reinterpret_cast<char*>(Info)+Info->Size-sizeof(EndMarker));
}

void PrintMemory();

static auto& CriticalSection()
{
	static os::critical_section s_Cs;
	return s_Cs;
}

static void RegisterBlock(MEMINFO *block)
{
	if (!MonitoringEnabled)
		return;

	SCOPED_ACTION(std::lock_guard<os::critical_section>)(CriticalSection());

	static auto AtExitSet = false;
	if (!AtExitSet)
	{
		atexit(PrintMemory);
		AtExitSet = true;
	}

	block->prev = LastMemBlock;
	block->next = nullptr;

	LastMemBlock->next = block;
	LastMemBlock = block;

	CheckChain();

	updateCallCount(block->AllocationType, true);
	++AllocatedMemoryBlocks;
	++TotalAllocationCalls;
	AllocatedMemorySize+=block->Size;
}

static void UnregisterBlock(MEMINFO *block)
{
	if (!MonitoringEnabled)
		return;

	SCOPED_ACTION(std::lock_guard<os::critical_section>)(CriticalSection());

	if (block->prev)
		block->prev->next = block->next;
	if (block->next)
		block->next->prev = block->prev;
	if(block == LastMemBlock)
		LastMemBlock = LastMemBlock->prev;

	CheckChain();

	updateCallCount(block->AllocationType, false);
	--AllocatedMemoryBlocks;
	AllocatedMemorySize-=block->Size;
}

static std::string FormatLine(const char* File, int Line, const char* Function, allocation_type Type, size_t Size)
{
	const char* sType;
	switch (Type)
	{
	case allocation_type::scalar:
		sType = "operator new";
		break;

	case allocation_type::vector:
		sType = "operator new[]";
		break;

	default:
		throw MAKE_FAR_EXCEPTION(L"Unknown allocation type");
	};

	return format("{0}:{1} -> {2}:{3} ({4} bytes)", File, Line, Function, sType, Size);
}

static size_t GetRequiredSize(size_t RequestedSize)
{
	assert(std::numeric_limits<size_t>::max() - RequestedSize >= sizeof(MEMINFO) + sizeof(EndMarker));

	return sizeof(MEMINFO) + RequestedSize + sizeof(EndMarker);
}

static void* DebugAllocator(size_t size, bool Noexcept, allocation_type type, const char* Function, const char* File, int Line)
{
	const auto realSize = GetRequiredSize(size);

	for(;;)
	{
		if (const auto Info = static_cast<MEMINFO*>(malloc(realSize)))
		{
			*Info = { type, Line, File, Function, realSize };
			GetMarker(Info) = EndMarker;
			RegisterBlock(Info);
			return ToUser(Info);
		}

		if (const auto Handler = std::get_new_handler())
		{
			Handler();
			if (std::get_new_handler())
				continue;
		}

		return Noexcept? nullptr : throw std::bad_alloc{};
	}
}

static void DebugDeallocator(void* block, allocation_type type)
{
	if (const auto Info = block? ToReal(block) : nullptr)
	{
		assert(Info->AllocationType == type);
		assert(GetMarker(Info) == EndMarker);
		UnregisterBlock(Info);
		free(Info);
	}
}

static string FindStr(const void* Data, size_t Size)
{
	const auto ABegin = reinterpret_cast<const char*>(Data), AEnd = ABegin + Size - 1;

	if (std::all_of(ABegin, AEnd, [](char c){ return c >= ' ' || IsEol(c) || c == '\t'; }))
	{
		return encoding::ansi::get_chars(ABegin, AEnd - ABegin);
	}

	const auto WBegin = reinterpret_cast<const wchar_t*>(Data), WEnd = WBegin + (Size - 1) / sizeof(wchar_t);

	if (std::all_of(WBegin, WEnd, [](wchar_t c){ return c >= L' ' || IsEol(c) || c == L'\t'; }))
	{
		return { WBegin, WEnd };
	}

	return {};
}

void PrintMemory()
{
	const auto MonitoringState = MonitoringEnabled.exchange(false);

	if (CallNewDeleteVector || CallNewDeleteScalar || AllocatedMemoryBlocks || AllocatedMemorySize)
	{
		string Message(L"Memory leaks detected:\n");

		if (CallNewDeleteVector)
			Message += format(L"  delete[]:   {0}\n", CallNewDeleteVector);
		if (CallNewDeleteScalar)
			Message += format(L"  delete:     {0}\n", CallNewDeleteScalar);
		if (AllocatedMemoryBlocks)
			Message += format(L"Total blocks: {0}\n", AllocatedMemoryBlocks);
		if (AllocatedMemorySize)
			Message += format(L"Total bytes:  {0} payload, {1} overhead\n", AllocatedMemorySize - AllocatedMemoryBlocks * (sizeof(MEMINFO) + sizeof(EndMarker)), AllocatedMemoryBlocks * sizeof(MEMINFO));
		Message += L'\n';

		Message += L"Not freed blocks:\n";

		std::wcerr << Message;
		OutputDebugString(Message.data());
		Message.clear();

		for(auto i = FirstMemBlock.next; i; i = i->next)
		{
			const auto BlockSize = i->Size - sizeof(MEMINFO) - sizeof(EndMarker);
			const auto UserAddress = ToUser(i);
			const size_t Width = 16;
			Message = concat(str(UserAddress), L", "_sv, encoding::ansi::get_chars(FormatLine(i->File, i->Line, i->Function, i->AllocationType, BlockSize)),
				L"\nData: "_sv, BlobToHexWString(UserAddress, std::min(BlockSize, Width), L' '),
				L"\nText: "_sv, FindStr(UserAddress, std::min(BlockSize, Width * 3)), L'\n');

			std::wcerr << Message;
			OutputDebugString(Message.data());
		}
	}
	MonitoringEnabled = MonitoringState;
}

};

void* operator new(size_t size)
{
	return memcheck::DebugAllocator(size, false, memcheck::allocation_type::scalar, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new(size_t size, const std::nothrow_t&) noexcept
{
	return memcheck::DebugAllocator(size, true, memcheck::allocation_type::scalar, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new[](size_t size)
{
	return memcheck::DebugAllocator(size, false, memcheck::allocation_type::vector, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new[](size_t size, const std::nothrow_t&) noexcept
{
	return memcheck::DebugAllocator(size, true, memcheck::allocation_type::vector, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new(size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugAllocator(size, false, memcheck::allocation_type::scalar, Function, File, Line);
}

void* operator new(size_t size, const std::nothrow_t&, const char* Function, const char* File, int Line) noexcept
{
	return memcheck::DebugAllocator(size, true, memcheck::allocation_type::scalar, Function, File, Line);
}

void* operator new[](size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugAllocator(size, false, memcheck::allocation_type::vector, Function, File, Line);
}

void* operator new[](size_t size, const std::nothrow_t&, const char* Function, const char* File, int Line) noexcept
{
	return memcheck::DebugAllocator(size, true, memcheck::allocation_type::vector, Function, File, Line);
}

void operator delete(void* block) noexcept
{
	return memcheck::DebugDeallocator(block, memcheck::allocation_type::scalar);
}

void operator delete[](void* block) noexcept
{
	return memcheck::DebugDeallocator(block, memcheck::allocation_type::vector);
}

void operator delete(void* block, size_t size) noexcept
{
	return memcheck::DebugDeallocator(block, memcheck::allocation_type::scalar);
}

void operator delete[](void* block, size_t size) noexcept
{
	return memcheck::DebugDeallocator(block, memcheck::allocation_type::vector);
}

void operator delete(void* block, const char* Function, const char* File, int Line)
{
	return memcheck::DebugDeallocator(block, memcheck::allocation_type::scalar);
}

void operator delete[](void* block, const char* Function, const char* File, int Line)
{
	return memcheck::DebugDeallocator(block, memcheck::allocation_type::vector);
}

wchar_t* DuplicateString(const wchar_t * str, const char* Function, const char* File, int Line)
{
	return str? wcscpy(new(Function, File, Line) wchar_t[wcslen(str) + 1], str) : nullptr;
}

#endif
