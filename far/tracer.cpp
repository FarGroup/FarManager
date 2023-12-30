/*
tracer.cpp
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
#include "tracer.hpp"

// Internal:
#include "pathmix.hpp"
#include "map_file.hpp"

// Platform:
#include "platform.fs.hpp"
#include "platform.memory.hpp"

// Common:
#include "common.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static constexpr auto BitsPerHexChar = 4;

template<typename T>
static constexpr auto width_in_hex_chars = std::numeric_limits<T>::digits / BitsPerHexChar;

static auto format_address(uintptr_t const Value)
{
	// It is unlikely that RVAs will be above 4 GiB,
	// so we can save some screen space here.
	const auto Width =
#ifdef _WIN64
		Value > std::numeric_limits<uint32_t>::max()?
			width_in_hex_chars<decltype(Value)> :
#endif
			width_in_hex_chars<uint32_t>;

	return far::format(L"{:0{}X}"sv, Value, Width);
}

static auto format_symbol(uintptr_t const Address, string_view const ImageName, os::debug::symbols::symbol const Symbol)
{
	// If it's not a legit pointer, it's likely a member of a struct at nullptr or something like that, no point in suggesting PDBs.
	if (ImageName.empty() && Symbol.Name.empty() && !os::memory::is_pointer(ToPtr(Address)))
		return L""s;

	return far::format(
		L"{}!{}{}"sv,
		!ImageName.empty()?
			PointToName(ImageName):
			L"<unknown>"sv,
		!Symbol.Name.empty()?
			Symbol.Name :
			L"<unknown> (get the pdb)"sv,
		!Symbol.Name.empty()?
			far::format(L"+0x{:X}"sv, Symbol.Displacement) :
			L""s
	);
}

static auto format_location(os::debug::symbols::location const Location)
{
	return !Location.FileName.empty()?
		concat(Location.FileName, Location.Line? far::format(L"({})"sv, *Location.Line) : L""s) :
		L""s;
}

tracer_detail::tracer::tracer():
	m_MapFiles(std::make_unique<std::unordered_map<uintptr_t, map_file>>())
{
}

tracer_detail::tracer::~tracer() = default;

void tracer_detail::tracer::get_symbols(string_view const Module, std::span<os::debug::stack_frame const> const Trace, function_ref<void(string&& Line)> const Consumer) const
{
	SCOPED_ACTION(with_symbols)(Module);

	os::debug::symbols::get(Module, Trace, *m_MapFiles, [&](uintptr_t const Address, string_view const ImageName, bool const InlineFrame, os::debug::symbols::symbol const Symbol, os::debug::symbols::location const Location)
	{
		auto Result = format_address(Address);

		if (Address)
		{
			if (const auto FormattedSymbol = format_symbol(Address, ImageName, Symbol); !FormattedSymbol.empty())
				append(Result, InlineFrame? L" I "sv : L"   "sv, FormattedSymbol);

			if (const auto LocationStr = format_location(Location); !LocationStr.empty())
				append(Result, L" ("sv, LocationStr, L')');
		}

		Consumer(std::move(Result));
	});
}

void tracer_detail::tracer::get_symbol(string_view const Module, const void* Ptr, string& AddressStr, string& Name, string& Source) const
{
	SCOPED_ACTION(with_symbols)(Module);

	os::debug::stack_frame const Stack[]{ { std::bit_cast<uintptr_t>(Ptr), INLINE_FRAME_CONTEXT_INIT } };

	os::debug::symbols::get(Module, Stack, *m_MapFiles, [&](uintptr_t const Address, string_view const ImageName, bool const InlineFrame, os::debug::symbols::symbol const Symbol, os::debug::symbols::location const Location)
	{
		AddressStr = format_address(Address);

		if (Address)
		{
			Name = format_symbol(Address, ImageName, Symbol);
			Source = format_location(Location);
		}
		else
		{
			Name.clear();
			Source.clear();
		}
	});
}

std::vector<os::debug::stack_frame> tracer_detail::tracer::current_stacktrace(string_view const Module, size_t const FramesToSkip, size_t const FramesToCapture) const
{
	SCOPED_ACTION(with_symbols)(Module);

	return os::debug::current_stacktrace(FramesToSkip, FramesToCapture);
}

std::vector<os::debug::stack_frame> tracer_detail::tracer::stacktrace(string_view const Module, CONTEXT const ContextRecord, HANDLE const ThreadHandle) const
{
	SCOPED_ACTION(with_symbols)(Module);

	return os::debug::stacktrace(ContextRecord, ThreadHandle);
}

std::vector<os::debug::stack_frame> tracer_detail::tracer::exception_stacktrace(string_view const Module) const
{
	SCOPED_ACTION(with_symbols)(Module);

	return os::debug::exception_stacktrace();
}

void tracer_detail::tracer::current_stacktrace(string_view Module, function_ref<void(string&& Line)> const Consumer, size_t const FramesToSkip, size_t const FramesToCapture) const
{
	SCOPED_ACTION(with_symbols)(Module);

	return get_symbols(Module, os::debug::current_stacktrace(FramesToSkip + 1, FramesToCapture), Consumer);
}

void tracer_detail::tracer::stacktrace(string_view Module, function_ref<void(string&& Line)> const Consumer, CONTEXT const ContextRecord, HANDLE const ThreadHandle) const
{
	SCOPED_ACTION(with_symbols)(Module);

	return get_symbols(Module, os::debug::stacktrace(ContextRecord, ThreadHandle), Consumer);
}

void tracer_detail::tracer::exception_stacktrace(string_view Module, function_ref<void(string&& Line)> const Consumer) const
{
	SCOPED_ACTION(with_symbols)(Module);

	return get_symbols(Module, os::debug::exception_stacktrace(), Consumer);
}

void tracer_detail::tracer::sym_initialise(string_view Module)
{
	SCOPED_ACTION(std::scoped_lock)(m_CS);

	// SymInitialize / SymCleanup do not support recursion, so we have to do it ourselves.
	++m_SymInitializeLevel;

	if (!m_SymInitialized)
		m_SymInitialized = os::debug::symbols::initialize(Module);
}

void tracer_detail::tracer::sym_cleanup()
{
	SCOPED_ACTION(std::scoped_lock)(m_CS);

	if (m_SymInitializeLevel)
		--m_SymInitializeLevel;

	if (m_SymInitializeLevel)
		return;

	if (m_SymInitialized)
	{
		os::debug::symbols::clean();
		m_SymInitialized = false;
	}

	m_MapFiles->clear();
}

tracer_detail::tracer::with_symbols::with_symbols(string_view const Module)
{
	::tracer.sym_initialise(Module);
}

tracer_detail::tracer::with_symbols::~with_symbols()
{
	::tracer.sym_cleanup();
}

NIFTY_DEFINE(tracer_detail::tracer, tracer);
