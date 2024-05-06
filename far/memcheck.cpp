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
#include "platform.debug.hpp"

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

#ifdef MEMCHECK
#define MEMCHECK_ENABLED

namespace memcheck
{
enum class allocation_type: uint8_t
{
	scalar = 0x5C,
	vector = 0x7E,
};

static const int EndMarker = 0xDEADBEEF;

struct memory_block
{
	unsigned short HeaderSize;
	allocation_type AllocationType;
	size_t DataSize;
	size_t TotalSize;

	// Initializers aren't really needed here, just to stop GCC from complaining about them.
	void* Stack[10]{};

	memory_block* prev{};
	memory_block* next{};

	static constexpr auto from_data(void* address, std::align_val_t Alignment)
	{
		return edit_as<memory_block*>(address, 0 - aligned_size(sizeof(memory_block), static_cast<size_t>(Alignment)));
	}

	constexpr void* data()
	{
		return edit_as<char*>(this, this->HeaderSize);
	}

	constexpr int& end_marker()
	{
		return *edit_as<int*>(this, HeaderSize + aligned_size(DataSize, alignof(int)));
	}
};

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
		throw far_fatal_exception(L"Unknown allocation type"sv);
	}

	return far::format(L"{} ({} bytes)"sv, sType, Size);
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

static string printable_string(string_view const Str)
{
	string Result;
	Result.reserve(Str.size());

	std::ranges::replace_copy_if(Str, std::back_inserter(Result), [](wchar_t const Char){ return !std::iswprint(Char); }, L'.');

	return Result;
}

static string printable_wide_string(void const* const Data, size_t const Size)
{
	ASAN_UNPOISON_MEMORY_REGION(Data, Size);
	return printable_string({ static_cast<const wchar_t*>(Data), Size / sizeof(wchar_t) });
}

static string printable_ansi_string(void const* const Data, size_t const Size)
{
	ASAN_UNPOISON_MEMORY_REGION(Data, Size);
	return printable_string(encoding::ansi::get_chars({ static_cast<const char*>(Data), Size }));
}

static void poison_block(memory_block* Block)
{
	ASAN_POISON_MEMORY_REGION(&Block->end_marker(), sizeof(EndMarker));
	ASAN_POISON_MEMORY_REGION(Block, Block->HeaderSize);
}

static void unpoison_block(memory_block* Block)
{
	ASAN_UNPOISON_MEMORY_REGION(Block, sizeof(*Block));
	ASAN_UNPOISON_MEMORY_REGION(Block + 1, Block->HeaderSize - sizeof(*Block));
	ASAN_UNPOISON_MEMORY_REGION(&Block->end_marker(), sizeof(EndMarker));
}

static class blocks_list
{
public:
	void add(memory_block* const Block)
	{
		check_chain();

		Block->prev = m_Last;
		Block->next = nullptr;

		if (m_Last)
		{
			unpoison_block(m_Last);
			m_Last->next = Block;
			poison_block(m_Last);
		}
		else
			m_First = Block;

		m_Last = Block;

		++m_Size;
	}

	void remove(memory_block const* const Block)
	{
		check_chain();

		if (Block->prev)
		{
			unpoison_block(Block->prev);
			Block->prev->next = Block->next;
			poison_block(Block->prev);
		}

		if (Block->next)
		{
			unpoison_block(Block->next);
			Block->next->prev = Block->prev;
			poison_block(Block->next);
		}

		if (Block == m_Last)
			m_Last = m_Last->prev;

		if (Block == m_First)
			m_First = Block->next;

		--m_Size;
	}

	auto begin() const
	{
		return m_First;
	}

	auto size() const
	{
		return m_Size;
	}

private:
	void check_chain() const
	{
		if constexpr ((false))
		{
			if (!m_First)
				return;

			auto p = m_First;

			while (p->next)
				p = p->next;
			assert(p == m_Last);

			while (p->prev)
				p = p->prev;
			assert(p == m_First);
		}
	}

	memory_block* m_First{};
	memory_block* m_Last{};
	size_t m_Size{};
}
Blocks;

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

	void register_block(memory_block* const Block)
	{
		if (!m_Enabled)
			return;

		Blocks.add(Block);

		update_call_count(Block->AllocationType, true);

		m_AllocatedMemorySize += Block->TotalSize;
		m_AllocatedPayloadSize += Block->DataSize;
	}

	void unregister_block(memory_block const* const Block)
	{
		if (!m_Enabled)
			return;

		Blocks.remove(Block);

		update_call_count(Block->AllocationType, false);

		m_AllocatedMemorySize -= Block->TotalSize;
		m_AllocatedPayloadSize -= Block->DataSize;
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
		default: throw far_fatal_exception(L"Unknown allocation type"sv);
		}
	}

	void print_summary() const
	{
		if (!m_AllocatedMemorySize)
			return;

		os::debug::breakpoint_if_debugging();

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
			far::format_to(Message, L" new[]:   {}\n"sv, m_CallNewDeleteVector);
		if (m_CallNewDeleteScalar)
			far::format_to(Message, L" new:     {}\n"sv, m_CallNewDeleteScalar);

		Message += L'\n';

		far::format_to(Message, L" Blocks:  {}\n"sv, Blocks.size());
		far::format_to(Message, L" Payload: {}\n"sv, m_AllocatedPayloadSize);
		far::format_to(Message, L" Bytes:   {}\n"sv, m_AllocatedMemorySize);

		append(Message, L"\nNot freed blocks:\n"sv);

		Print(Message);
		Message.clear();

		for (auto i = Blocks.begin(); i; i = i->next)
		{
			unpoison_block(i);

			const auto Data = i->data();
			const auto Size = i->DataSize;
			const size_t Width = 80 - 7 - 1;

			Message = concat(
				L"--------------------------------------------------------------------------------\n"sv,
				str(Data), L", "sv, format_type(i->AllocationType, Size),
				L"\nData: "sv, BlobToHexString({ static_cast<std::byte const*>(Data), std::min(Size, Width / 3) }, L' '),
				L"\nAnsi: "sv, printable_ansi_string(Data, std::min(Size, Width)),
				L"\nWide: "sv, printable_wide_string(Data, std::min(Size, Width * sizeof(wchar_t))),
				L"\nStack:\n"sv);

			os::debug::stack_frame Stack[ARRAYSIZE(memory_block::Stack)];
			size_t StackSize;

			for (StackSize = 0; StackSize != std::size(Stack) && i->Stack[StackSize]; ++StackSize)
			{
				Stack[StackSize] = { std::bit_cast<uintptr_t>(i->Stack[StackSize]), INLINE_FRAME_CONTEXT_INIT };
			}

			tracer.get_symbols({}, { Stack, StackSize }, [&](string_view const Line)
			{
				append(Message, Line, L'\n');
			});

			Print(Message);
		}
	}

	os::critical_section m_CS;

	intptr_t m_CallNewDeleteVector{};
	intptr_t m_CallNewDeleteScalar{};
	size_t m_AllocatedMemorySize{};
	size_t m_AllocatedPayloadSize{};

	bool m_Enabled{true};
};

static void* debug_allocator(size_t const size, std::align_val_t Alignment, allocation_type const type, bool const Noexcept)
{
	const auto BlockAlignment = std::max(alignof(memory_block), static_cast<size_t>(Alignment));
	const auto HeaderSize = static_cast<unsigned>(aligned_size(sizeof(memory_block), BlockAlignment));
	assert(std::numeric_limits<size_t>::max() - size >= HeaderSize + sizeof(EndMarker));

	const auto realSize = HeaderSize + aligned_size(size, alignof(int)) + sizeof(EndMarker);

	for(;;)
	{
		if (const auto RawBlock = _aligned_malloc(realSize, BlockAlignment))
		{
			const auto Block = static_cast<memory_block*>(RawBlock);
			placement::construct(*Block, HeaderSize, type, size, realSize);

			const auto FramesToSkip = 2; // This function and the operator
			// RtlCaptureStackBackTrace is invoked directly since we don't need to make debug builds Win2k compatible
			if (const auto Captured = RtlCaptureStackBackTrace(FramesToSkip, static_cast<DWORD>(std::size(Block->Stack)), Block->Stack, {}); Captured < std::size(Block->Stack))
				Block->Stack[Captured] = {};

			Block->end_marker() = EndMarker;

			{
				SCOPED_ACTION(std::scoped_lock)(Checker);
				Checker.register_block(Block);
				poison_block(Block);
			}

			const auto Data = Block->data();
			assert(is_aligned(Data, static_cast<size_t>(Alignment)));

			return Data;
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

static void debug_deallocator(void* const Data, std::optional<size_t> const Size, std::align_val_t Alignment, allocation_type type) noexcept
{
	if (!Data)
		return;

	const auto Block = memory_block::from_data(Data, Alignment);

	{
		SCOPED_ACTION(std::scoped_lock)(Checker);
		unpoison_block(Block);
		Checker.unregister_block(Block);
	}

	const auto BlockAlignment = std::max(alignof(memory_block), static_cast<size_t>(Alignment));

	assert(Block->HeaderSize == static_cast<unsigned>(aligned_size(sizeof(memory_block), BlockAlignment)));
	assert(Block->AllocationType == type);
	assert(Block->end_marker() == EndMarker);

	if (Size)
	{
		assert(Block->DataSize == Size);
		assert(Block->TotalSize == Block->HeaderSize + aligned_size(*Size, alignof(int)) + sizeof(EndMarker));
	}

	placement::destruct(*Block);
	_aligned_free(Block);
}

static constexpr std::align_val_t default_alignment{ __STDCPP_DEFAULT_NEW_ALIGNMENT__ };

}

#ifdef MEMCHECK_ENABLED

// ReSharper disable CppParameterNamesMismatch
WARNING_PUSH()
WARNING_DISABLE_CLANG("-Wmissing-prototypes")

//----------------------------------------------------------------------------
void* operator new(size_t const Size)
{
	using namespace memcheck;
	return debug_allocator(Size, default_alignment, allocation_type::scalar, false);
}

void operator delete(void* const Block) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, {}, default_alignment, allocation_type::scalar);
}

void operator delete(void* const Block, size_t const Size) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, Size, default_alignment, allocation_type::scalar);
}

//----------------------------------------------------------------------------
void* operator new[](size_t const Size)
{
	using namespace memcheck;
	return debug_allocator(Size, default_alignment, allocation_type::vector, false);
}

void operator delete[](void* const Block) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, {}, default_alignment, allocation_type::vector);
}

void operator delete[](void* const Block, size_t const Size) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, Size, default_alignment, allocation_type::vector);
}

//----------------------------------------------------------------------------
void* operator new(size_t const Size, std::align_val_t const Alignment)
{
	using namespace memcheck;
	return debug_allocator(Size, Alignment, allocation_type::scalar, false);
}

void operator delete(void* const Block, std::align_val_t const Alignment) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, {}, Alignment, allocation_type::scalar);
}

void operator delete(void* const Block, size_t const Size, std::align_val_t const Alignment) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, Size, Alignment, allocation_type::scalar);
}

//----------------------------------------------------------------------------
void* operator new[](size_t const Size, std::align_val_t const Alignment)
{
	using namespace memcheck;
	return debug_allocator(Size, Alignment, allocation_type::vector, false);
}

void operator delete[](void* const Block, std::align_val_t const Alignment) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, {}, Alignment, allocation_type::vector);
}

void operator delete[](void* const Block, size_t const Size, std::align_val_t const Alignment) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, Size, Alignment, allocation_type::vector);
}

//----------------------------------------------------------------------------
void* operator new(size_t const Size, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_allocator(Size, default_alignment, allocation_type::scalar, true);
}

void operator delete(void* const Block, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, {}, default_alignment, allocation_type::scalar);
}

//----------------------------------------------------------------------------
void* operator new[](size_t const Size, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_allocator(Size, default_alignment, allocation_type::vector, true);
}

void operator delete[](void* const Block, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, {}, default_alignment, allocation_type::vector);
}

//----------------------------------------------------------------------------
void* operator new(size_t const Size, std::align_val_t const Alignment, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_allocator(Size, Alignment, allocation_type::scalar, true);
}

void operator delete(void* const Block, std::align_val_t const Alignment, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, {}, Alignment, allocation_type::scalar);
}

//----------------------------------------------------------------------------
void* operator new[](size_t const Size, std::align_val_t const Alignment, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_allocator(Size, Alignment, allocation_type::vector, true);
}

void operator delete[](void* const Block, std::align_val_t const Alignment, std::nothrow_t const&) noexcept
{
	using namespace memcheck;
	return debug_deallocator(Block, {}, Alignment, allocation_type::vector);
}

// ReSharper restore CppParameterNamesMismatch
WARNING_POP()

#endif // MEMCHECK_ENABLED

NIFTY_DEFINE(memcheck::checker, Checker);

#endif // MEMCHECK
