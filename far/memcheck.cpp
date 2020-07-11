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

// Self:
#include "memcheck.hpp"

// Internal:
#include "strmix.hpp"
#include "encoding.hpp"
#include "exception.hpp"
#include "console.hpp"

// Platform:
#include "platform.concurrency.hpp"

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

#ifdef MEMCHECK

#undef new

namespace memcheck
{

enum class allocation_type: unsigned
{
	scalar = 0xa75ca1ae,
	vector = 0xa77ec10e,
};

static const int EndMarker = 0xDEADBEEF;

struct alignas(MEMORY_ALLOCATION_ALIGNMENT) MEMINFO
{
	allocation_type AllocationType;
	int Line;
	string_view File;
	const char* Function;
	size_t Size;
	MEMINFO* prev;
	MEMINFO* next;

	int& end_marker()
	{
		return *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + Size - sizeof(EndMarker));
	}
};

static_assert(alignof(MEMINFO) == MEMORY_ALLOCATION_ALIGNMENT);

static MEMINFO FirstMemBlock;
static MEMINFO* LastMemBlock = &FirstMemBlock;

static auto ToReal(void* address) { return static_cast<MEMINFO*>(address) - 1; }
static void* ToUser(MEMINFO* address) { return address + 1; }

static void CheckChain()
{
	if constexpr ((false))
	{
		auto p = &FirstMemBlock;

		while (p->next)
			p = p->next;
		assert(p == LastMemBlock);

		while (p->prev)
			p = p->prev;
		assert(p == &FirstMemBlock);
	}
}

class checker
{
public:
	NONCOPYABLE(checker);

	checker()
	{
		m_Enabled = true;
	}

	~checker()
	{
		m_Enabled = false;

		try
		{
			summary();
		}
		catch (...)
		{
		}
	}

	void RegisterBlock(MEMINFO *block);
	void UnregisterBlock(MEMINFO *block);

private:
	void updateCallCount(allocation_type type, bool increment);
	void summary() const;

	os::critical_section m_CS;

	intptr_t m_CallNewDeleteVector{};
	intptr_t m_CallNewDeleteScalar{};
	size_t m_AllocatedMemoryBlocks{};
	size_t m_AllocatedMemorySize{};
	size_t m_TotalAllocationCalls{};
	size_t m_TotalDeallocationCalls{};

	bool m_Enabled;
};

void checker::updateCallCount(allocation_type type, bool increment)
{
	const auto op = increment? 1 : -1;
	switch (type)
	{
	case allocation_type::scalar: m_CallNewDeleteScalar += op; break;
	case allocation_type::vector: m_CallNewDeleteVector += op; break;
	default: throw MAKE_FAR_FATAL_EXCEPTION(L"Unknown allocation type"sv);
	}
}

void checker::RegisterBlock(MEMINFO *block)
{
	if (!m_Enabled)
		return;

	SCOPED_ACTION(std::lock_guard)(m_CS);

	block->prev = LastMemBlock;
	block->next = nullptr;

	LastMemBlock->next = block;
	LastMemBlock = block;

	CheckChain();

	updateCallCount(block->AllocationType, true);
	++m_AllocatedMemoryBlocks;
	++m_TotalAllocationCalls;
	m_AllocatedMemorySize += block->Size;
}

void checker::UnregisterBlock(MEMINFO *block)
{
	if (!m_Enabled)
		return;

	SCOPED_ACTION(std::lock_guard)(m_CS);

	if (block->prev)
		block->prev->next = block->next;
	if (block->next)
		block->next->prev = block->prev;
	if(block == LastMemBlock)
		LastMemBlock = LastMemBlock->prev;

	CheckChain();

	updateCallCount(block->AllocationType, false);
	++m_TotalDeallocationCalls;
	--m_AllocatedMemoryBlocks;
	m_AllocatedMemorySize -= block->Size;
}

static string FormatLine(string_view const File, int Line, string_view const Function, allocation_type Type, size_t Size)
{
	string_view sType;
	switch (Type)
	{
	case allocation_type::scalar:
		sType = L"operator new"sv;
		break;

	case allocation_type::vector:
		sType = L"operator new[]"sv;
		break;

	default:
		throw MAKE_FAR_FATAL_EXCEPTION(L"Unknown allocation type"sv);
	}

	return format(FSTR(L"{0}:{1} -> {2}:{3} ({4} bytes)"), File, Line, Function, sType, Size);
}

static size_t GetRequiredSize(size_t RequestedSize)
{
	assert(std::numeric_limits<size_t>::max() - RequestedSize >= sizeof(MEMINFO) + sizeof(EndMarker));

	return sizeof(MEMINFO) + RequestedSize + sizeof(EndMarker);
}

static void* DebugAllocator(size_t const size, bool const Noexcept, allocation_type const type, const char* const Function, string_view const File, int const Line)
{
	const auto realSize = GetRequiredSize(size);

	for(;;)
	{
		if (const auto RawBlock = malloc(realSize))
		{
			const auto Info = static_cast<MEMINFO*>(RawBlock);
			placement::construct(*Info, type, Line, File, Function, realSize);
			Info->end_marker() = EndMarker;
			Checker.RegisterBlock(Info);
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

static void DebugDeallocator(void* block, allocation_type type) noexcept
{
	if (const auto Info = block? ToReal(block) : nullptr)
	{
		assert(Info->AllocationType == type);
		assert(Info->end_marker() == EndMarker);
		Checker.UnregisterBlock(Info);

		placement::destruct(*Info);

		free(Info);
	}
}

static string printable_string(string Str)
{
	for (auto& i: Str)
	{
		if (!std::iswprint(i))
			i = L'.';
	}

	return Str;
}

static string printable_wide_string(void const* const Data, size_t const Size)
{
	return printable_string({ static_cast<const wchar_t*>(Data), Size / sizeof(wchar_t) });
}

static string printable_ansi_string(void const* const Data, size_t const Size)
{
	return printable_string(encoding::ansi::get_chars({ static_cast<const char*>(Data), Size }));
}

void checker::summary() const
{
	if (!m_CallNewDeleteVector && !m_CallNewDeleteScalar && !m_AllocatedMemoryBlocks && !m_AllocatedMemorySize)
		return;

	// Q: Why?
	// A: The same reason we owerride stream buffers everywhere else: the default one is shite - it goes through FILE* and breaks wide characters.
	//    At this point the regular overrider is already dead so we need to revive it once more:
	SCOPED_ACTION(auto)(console_detail::console::create_temporary_stream_buffers_overrider());

	const auto Print = [](const string& Str)
	{
		std::wcerr << Str;
		OutputDebugString(Str.c_str());
	};

	auto Message = L"Memory leaks detected:\n"s;

	if (m_CallNewDeleteVector)
		Message += format(FSTR(L"  delete[]:   {0}\n"), m_CallNewDeleteVector);
	if (m_CallNewDeleteScalar)
		Message += format(FSTR(L"  delete:     {0}\n"), m_CallNewDeleteScalar);
	if (m_AllocatedMemoryBlocks)
		Message += format(FSTR(L"Total blocks: {0}\n"), m_AllocatedMemoryBlocks);
	if (m_AllocatedMemorySize)
		Message += format(FSTR(L"Total bytes:  {0} payload, {1} overhead\n"), m_AllocatedMemorySize - m_AllocatedMemoryBlocks * (sizeof(MEMINFO) + sizeof(EndMarker)), m_AllocatedMemoryBlocks * sizeof(MEMINFO));

	append(Message, L"\nNot freed blocks:\n"sv);

	Print(Message);
	Message.clear();

	for(auto i = FirstMemBlock.next; i; i = i->next)
	{
		const auto BlockSize = i->Size - sizeof(MEMINFO) - sizeof(EndMarker);
		const auto UserAddress = ToUser(i);
		const size_t Width = 80 - 7 - 1;
		Message = concat(
			str(UserAddress), L", "sv, FormatLine(i->File, i->Line, encoding::utf8::get_chars(i->Function), i->AllocationType, BlockSize),
			L"\nData: "sv, BlobToHexString({ static_cast<std::byte const*>(UserAddress), std::min(BlockSize, Width / 3) }, L' '),
			L"\nAnsi: "sv, printable_ansi_string(UserAddress, std::min(BlockSize, Width)),
			L"\nWide: "sv, printable_wide_string(UserAddress, std::min(BlockSize, Width * sizeof(wchar_t))), L"\n\n"sv);

		Print(Message);
	}
}

}

void* operator new(size_t size)
{
	return memcheck::DebugAllocator(size, false, memcheck::allocation_type::scalar, __FUNCTION__, WIDE_SV(__FILE__), __LINE__);
}

void* operator new(size_t size, const std::nothrow_t&) noexcept
{
	return memcheck::DebugAllocator(size, true, memcheck::allocation_type::scalar, __FUNCTION__, WIDE_SV(__FILE__), __LINE__);
}

void* operator new[](size_t size)
{
	return memcheck::DebugAllocator(size, false, memcheck::allocation_type::vector, __FUNCTION__, WIDE_SV(__FILE__), __LINE__);
}

void* operator new[](size_t size, const std::nothrow_t&) noexcept
{
	return memcheck::DebugAllocator(size, true, memcheck::allocation_type::vector, __FUNCTION__, WIDE_SV(__FILE__), __LINE__);
}

void* operator new(size_t const size, const char* const Function, string_view const File, int const Line)
{
	return memcheck::DebugAllocator(size, false, memcheck::allocation_type::scalar, Function, File, Line);
}

void* operator new(size_t const size, const std::nothrow_t&, const char* const Function, string_view const File, int const Line) noexcept
{
	return memcheck::DebugAllocator(size, true, memcheck::allocation_type::scalar, Function, File, Line);
}

void* operator new[](size_t const size, const char* const Function, string_view const File, int const Line)
{
	return memcheck::DebugAllocator(size, false, memcheck::allocation_type::vector, Function, File, Line);
}

void* operator new[](size_t const size, const std::nothrow_t&, const char* const Function, string_view const File, int const Line) noexcept
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

void operator delete(void* const block, const char* const Function, string_view const File, int const Line)
{
	return memcheck::DebugDeallocator(block, memcheck::allocation_type::scalar);
}

void operator delete[](void* const block, const char* const Function, string_view const File, int const Line)
{
	return memcheck::DebugDeallocator(block, memcheck::allocation_type::vector);
}

NIFTY_DEFINE(memcheck::checker, Checker);

#endif
