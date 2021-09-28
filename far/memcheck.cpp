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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "memcheck.hpp"

// Internal:
#include "strmix.hpp"
#include "encoding.hpp"
#include "exception.hpp"
#include "console.hpp"
#include "tracer.hpp"
#include "imports.hpp"

// Platform:
#include "platform.concurrency.hpp"

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

#ifdef MEMCHECK

namespace memcheck
{
enum class allocation_type: unsigned
{
	scalar = 0xa75ca1ae,
	vector = 0xa77ec10e,
};

static const int EndMarker = 0xDEADBEEF;

struct MEMINFO
{
	allocation_type AllocationType;
	unsigned HeaderSize;

	size_t Size;

	void* Stack[10];

	MEMINFO* prev;
	MEMINFO* next;

	int& end_marker()
	{
		return *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + Size - sizeof(EndMarker));
	}
};

static MEMINFO FirstMemBlock{ {}, sizeof(FirstMemBlock) };
static MEMINFO* LastMemBlock = &FirstMemBlock;

static auto to_real(void* address, std::align_val_t Alignment)
{
	return static_cast<MEMINFO*>(static_cast<void*>(static_cast<char*>(address) - aligned_size(sizeof(MEMINFO), static_cast<size_t>(Alignment))));
}

static void* to_user(MEMINFO* address)
{
	return static_cast<char*>(static_cast<void*>(address)) + address->HeaderSize;
}

static void check_chain()
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

static string format_type(allocation_type Type, size_t Size)
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

	return format(FSTR(L"{} ({} bytes)"sv), sType, Size);
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

#ifdef __SANITIZE_ADDRESS__
extern "C"
{
	void __asan_poison_memory_region(void const volatile* addr, size_t size);
	void __asan_unpoison_memory_region(void const volatile* addr, size_t size);
}

#define ASAN_POISON_MEMORY_REGION(addr, size)   __asan_poison_memory_region((addr), (size))
#define ASAN_UNPOISON_MEMORY_REGION(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
#define ASAN_POISON_MEMORY_REGION(addr, size)   ((void)(addr), (void)(size))
#define ASAN_UNPOISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
#endif

void poison_block(MEMINFO* Block)
{
	ASAN_POISON_MEMORY_REGION(&Block->end_marker(), sizeof(EndMarker));
	ASAN_POISON_MEMORY_REGION(Block, Block->HeaderSize);
}

void unpoison_block(MEMINFO* Block)
{
	ASAN_UNPOISON_MEMORY_REGION(Block, sizeof(*Block));
	ASAN_UNPOISON_MEMORY_REGION(Block + 1, Block->HeaderSize - sizeof(*Block));
	ASAN_UNPOISON_MEMORY_REGION(&Block->end_marker(), sizeof(EndMarker));
}

class checker
{
public:
	NONCOPYABLE(checker);

	checker() = default;

	~checker()
	{
		m_Enabled = false;

		try
		{
			print_summary();
		}
		catch (...)
		{
		}
	}

	void register_block(MEMINFO *block)
	{
		if (!m_Enabled)
			return;


		block->prev = LastMemBlock;
		block->next = nullptr;

		unpoison_block(LastMemBlock);
		LastMemBlock->next = block;
		poison_block(LastMemBlock);

		LastMemBlock = block;

		check_chain();

		update_call_count(block->AllocationType, true);
		++m_AllocatedMemoryBlocks;
		++m_TotalAllocationCalls;
		m_AllocatedMemorySize += block->Size;
		m_AllocatedPayloadSize += block->Size - block->HeaderSize - sizeof(EndMarker);
	}

	void unregister_block(MEMINFO *block)
	{
		if (!m_Enabled)
			return;

		if (block->prev)
		{
			unpoison_block(block->prev);
			block->prev->next = block->next;
			poison_block(block->prev);
		}

		if (block->next)
		{
			unpoison_block(block->next);
			block->next->prev = block->prev;
			poison_block(block->next);
		}

		if (block == LastMemBlock)
			LastMemBlock = LastMemBlock->prev;

		check_chain();

		update_call_count(block->AllocationType, false);
		++m_TotalDeallocationCalls;
		--m_AllocatedMemoryBlocks;
		m_AllocatedMemorySize -= block->Size;
		m_AllocatedPayloadSize -= block->Size - block->HeaderSize - sizeof(EndMarker);
	}

	void lock() { m_CS.lock(); }
	void unlock() { m_CS.unlock(); }

private:
	void update_call_count(allocation_type type, bool increment)
	{
		const auto op = increment? 1 : -1;
		switch (type)
		{
		case allocation_type::scalar: m_CallNewDeleteScalar += op; break;
		case allocation_type::vector: m_CallNewDeleteVector += op; break;
		default: throw MAKE_FAR_FATAL_EXCEPTION(L"Unknown allocation type"sv);
		}
	}

	void print_summary() const
	{
		if (!m_AllocatedMemorySize)
			return;

		os::debug::breakpoint(false);

		// Q: Why?
		// A: The regular instances are already dead at this point, this voodoo will bring them back from the underworld:
		SCOPED_ACTION(imports_nifty_objects::initialiser);
		SCOPED_ACTION(console_nifty_objects::initialiser);
		SCOPED_ACTION(tracer_nifty_objects::initialiser);

		const auto Print = [](const string& Str)
		{
			std::wcerr << Str;
			os::debug::print(Str);
		};

		auto Message = L"Memory leaks detected:\n"s;

		if (m_CallNewDeleteVector)
			format_to(Message, FSTR(L" new[]:   {}\n"sv), m_CallNewDeleteVector);
		if (m_CallNewDeleteScalar)
			format_to(Message, FSTR(L" new:     {}\n"sv), m_CallNewDeleteScalar);

		Message += L'\n';

		format_to(Message, FSTR(L" Blocks:  {}\n"sv), m_AllocatedMemoryBlocks);
		format_to(Message, FSTR(L" Payload: {}\n"sv), m_AllocatedPayloadSize);
		format_to(Message, FSTR(L" Bytes:   {}\n"sv), m_AllocatedMemorySize);

		append(Message, L"\nNot freed blocks:\n"sv);

		Print(Message);
		Message.clear();

		for (auto i = FirstMemBlock.next; i; i = i->next)
		{
			unpoison_block(i);

			const auto BlockSize = i->Size - i->HeaderSize - sizeof(EndMarker);
			const auto UserAddress = to_user(i);
			const size_t Width = 80 - 7 - 1;

			Message = concat(
				L"--------------------------------------------------------------------------------\n"sv,
				str(UserAddress), L", "sv, format_type(i->AllocationType, BlockSize),
				L"\nData: "sv, BlobToHexString({ static_cast<std::byte const*>(UserAddress), std::min(BlockSize, Width / 3) }, L' '),
				L"\nAnsi: "sv, printable_ansi_string(UserAddress, std::min(BlockSize, Width)),
				L"\nWide: "sv, printable_wide_string(UserAddress, std::min(BlockSize, Width * sizeof(wchar_t))),
				L"\nStack:\n"sv);

			uintptr_t Stack[ARRAYSIZE(MEMINFO::Stack)];
			size_t StackSize;

			for (StackSize = 0; StackSize != std::size(Stack) && i->Stack[StackSize]; ++StackSize)
			{
				Stack[StackSize] = reinterpret_cast<uintptr_t>(i->Stack[StackSize]);
			}

			tracer.get_symbols({}, span(Stack, StackSize), [&](string_view const Line)
			{
				append(Message, Line, L'\n');
			});

			Print(Message);
		}
	}

	os::critical_section m_CS;

	intptr_t m_CallNewDeleteVector{};
	intptr_t m_CallNewDeleteScalar{};
	size_t m_AllocatedMemoryBlocks{};
	size_t m_AllocatedMemorySize{};
	size_t m_AllocatedPayloadSize{};
	size_t m_TotalAllocationCalls{};
	size_t m_TotalDeallocationCalls{};

	bool m_Enabled{true};
};

static void* debug_allocator(size_t const size, std::align_val_t Alignment, allocation_type const type, bool const Noexcept)
{
	const auto HeaderSize = static_cast<unsigned>(aligned_size(sizeof(MEMINFO), static_cast<size_t>(Alignment)));
	assert(std::numeric_limits<size_t>::max() - size >= HeaderSize + sizeof(EndMarker));

	const auto realSize = HeaderSize + size + sizeof(EndMarker);

	for(;;)
	{
		if (const auto RawBlock = _aligned_malloc(realSize, static_cast<size_t>(Alignment)))
		{
			const auto Info = static_cast<MEMINFO*>(RawBlock);
			placement::construct(*Info, type, HeaderSize, realSize);

			const auto FramesToSkip = 2; // This function and the operator
			// RtlCaptureStackBackTrace is invoked directly since we don't need to make debug builds Win2k compatible
			if (const auto Captured = RtlCaptureStackBackTrace(FramesToSkip, static_cast<DWORD>(std::size(Info->Stack)), Info->Stack, {}); Captured < std::size(Info->Stack))
				Info->Stack[Captured] = {};

			Info->end_marker() = EndMarker;

			const auto Address = to_user(Info);

			{
				SCOPED_ACTION(std::lock_guard)(Checker);
				Checker.register_block(Info);
				poison_block(Info);
			}

			return Address;
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

static void debug_deallocator(void* const Block, std::align_val_t Alignment, allocation_type type) noexcept
{
	if (!Block)
		return;

	const auto Info = to_real(Block, Alignment);

	{
		SCOPED_ACTION(std::lock_guard)(Checker);
		unpoison_block(Info);
		Checker.unregister_block(Info);
	}

	assert(Info->AllocationType == type);
	assert(Info->end_marker() == EndMarker);

	placement::destruct(*Info);
	_aligned_free(Info);
}

static constexpr auto default_alignment()
{
	return std::align_val_t{ __STDCPP_DEFAULT_NEW_ALIGNMENT__ };
}

}

// ReSharper disable CppParameterNamesMismatch
WARNING_PUSH()
WARNING_DISABLE_CLANG("-Wmissing-prototypes")

void* operator new(size_t const Size)
{
	using namespace memcheck;
	return debug_allocator(Size, default_alignment(), allocation_type::scalar, false);
}

void* operator new[](size_t const Size)
{
	using namespace memcheck;
	return debug_allocator(Size, default_alignment(), allocation_type::vector, false);
}

void* operator new(size_t const Size, std::align_val_t const Alignment)
{
	using namespace memcheck;
	return debug_allocator(Size, Alignment, allocation_type::scalar, false);
}

void* operator new[](size_t const Size, std::align_val_t const Alignment)
{
	using namespace memcheck;
	return debug_allocator(Size, Alignment, allocation_type::vector, false);
}

void* operator new(size_t const Size, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_allocator(Size, default_alignment(), allocation_type::scalar, true);
}

void* operator new[](size_t const Size, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_allocator(Size, default_alignment(), allocation_type::vector, true);
}

void* operator new(size_t const Size, std::align_val_t const Alignment, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_allocator(Size, Alignment, allocation_type::scalar, true);
}

void* operator new[](size_t const Size, std::align_val_t const Alignment, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_allocator(Size, Alignment, allocation_type::vector, true);
}


void operator delete(void* const Block) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, default_alignment(), allocation_type::scalar);
}

void operator delete[](void* const Block) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, default_alignment(), allocation_type::vector);
}

void operator delete(void* const Block, size_t) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, default_alignment(), allocation_type::scalar);
}

void operator delete[](void* const Block, size_t) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, default_alignment(), allocation_type::vector);
}

void operator delete(void* const Block, std::align_val_t const Alignment) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, Alignment, allocation_type::scalar);
}

void operator delete[](void* const Block, std::align_val_t const Alignment) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, Alignment, allocation_type::vector);
}

void operator delete(void* const Block, size_t, std::align_val_t const Alignment) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, Alignment, allocation_type::scalar);
}

void operator delete[](void* const Block, size_t, std::align_val_t const Alignment) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, Alignment, allocation_type::vector);
}

void operator delete(void* const Block, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, default_alignment(), allocation_type::scalar);
}

void operator delete[](void* const Block, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, default_alignment(), allocation_type::vector);
}

void operator delete(void* const Block, std::align_val_t const Alignment, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, Alignment, allocation_type::scalar);
}

void operator delete[](void* const Block, std::align_val_t const Alignment, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, Alignment, allocation_type::vector);
}

// ReSharper restore CppParameterNamesMismatch
WARNING_POP()

NIFTY_DEFINE(memcheck::checker, Checker);

#endif
