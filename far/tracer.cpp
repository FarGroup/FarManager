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

#include "headers.hpp"
#pragma hdrstop

#include "tracer.hpp"
#include "imports.hpp"
#include "farexcpt.hpp"
#include "encoding.hpp"
#include "pathmix.hpp"

static auto GetBackTrace(const exception_context& Context)
{
	std::vector<const void*> Result;

	// StackWalk64() may modify context record passed to it, so we will use a copy.
	auto ContextRecord = *Context.GetPointers()->ContextRecord;
	STACKFRAME64 StackFrame = {};
#if defined(_WIN64)
	const DWORD MachineType = IMAGE_FILE_MACHINE_AMD64;
	StackFrame.AddrPC.Offset = ContextRecord.Rip;
	StackFrame.AddrFrame.Offset = ContextRecord.Rbp;
	StackFrame.AddrStack.Offset = ContextRecord.Rsp;
#else
	const DWORD MachineType = IMAGE_FILE_MACHINE_I386;
	StackFrame.AddrPC.Offset = ContextRecord.Eip;
	StackFrame.AddrFrame.Offset = ContextRecord.Ebp;
	StackFrame.AddrStack.Offset = ContextRecord.Esp;
#endif
	StackFrame.AddrPC.Mode = AddrModeFlat;
	StackFrame.AddrFrame.Mode = AddrModeFlat;
	StackFrame.AddrStack.Mode = AddrModeFlat;

	while (imports::instance().StackWalk64(MachineType, GetCurrentProcess(), Context.GetThreadHandle(), &StackFrame, &ContextRecord, nullptr, nullptr, nullptr, nullptr))
	{
		Result.emplace_back(reinterpret_cast<const void*>(StackFrame.AddrPC.Offset));
	}

	return Result;
}

static void GetSymbols(const std::vector<const void*>& BackTrace, const std::function<void(string&&, string&&, string&&)>& Consumer)
{
	const auto Process = GetCurrentProcess();
	const auto MaxNameLen = MAX_SYM_NAME;
	block_ptr<SYMBOL_INFO> Symbol(sizeof(SYMBOL_INFO) + MaxNameLen + 1);
	Symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	Symbol->MaxNameLen = MaxNameLen;

	imports::instance().SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES);

	IMAGEHLP_MODULEW64 Module{ static_cast<DWORD>(aligned_size(offsetof(IMAGEHLP_MODULEW64, LoadedImageName), 8)) }; // use the pre-07-Jun-2002 struct size, aligned to 8
	IMAGEHLP_LINE64 Line{ sizeof(Line) };
	DWORD Displacement;

	for (const auto i: BackTrace)
	{
		const auto Address = reinterpret_cast<DWORD_PTR>(i);
		const auto GotName = imports::instance().SymFromAddr(Process, Address, nullptr, Symbol.get()) != FALSE;
		const auto GotModule = imports::instance().SymGetModuleInfoW64(Process, Address, &Module) != FALSE;
		const auto GotSource = imports::instance().SymGetLineFromAddr64(Process, Address, &Displacement, &Line) != FALSE;

		Consumer(str(i),
			format(L"{0}!{1}", GotModule? PointToName(Module.ImageName) : L""_sv, GotName? Symbol->Name : ""),
			GotSource? format(L"{0}:{1}", Line.FileName, Line.LineNumber) : L""s);
	}
}

static auto GetSymbols(const std::vector<const void*>& BackTrace)
{
	std::vector<string> Result;
	GetSymbols(BackTrace, [&](string&& Address, string&& Name, string&& Source)
	{
		if (!Name.empty())
			append(Address, L' ', Name);

		if (!Source.empty())
			append(Address, L" ("_sv, Source, L')');

		Result.emplace_back(std::move(Address));
	});
	return Result;
}

tracer* StaticInstance;

static LONG WINAPI StackLogger(EXCEPTION_POINTERS *xp)
{
	if (IsCppException(xp))
	{
		// 0 indicates rethrow
		if (xp->ExceptionRecord->ExceptionInformation[1])
		{
			if (StaticInstance)
				StaticInstance->store(ToPtr(xp->ExceptionRecord->ExceptionInformation[1]), xp);
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

tracer::veh_handler::veh_handler(PVECTORED_EXCEPTION_HANDLER Handler):
	m_Handler(imports::instance().AddVectoredExceptionHandler(TRUE, Handler))
{
}

tracer::veh_handler::~veh_handler()
{
	imports::instance().RemoveVectoredExceptionHandler(m_Handler);
}

tracer::tracer():
	m_Handler(StackLogger)
{
	assert(!StaticInstance);

	string Path;
	if (os::fs::GetModuleFileName(nullptr, nullptr, Path))
	{
		CutToParent(Path);
		m_SymbolSearchPath = encoding::ansi::get_bytes(Path);
	}

	StaticInstance = this;
}

tracer::~tracer()
{
	assert(StaticInstance);

	StaticInstance = nullptr;
}

void tracer::store(const void* CppObject, const EXCEPTION_POINTERS* ExceptionInfo)
{
	SCOPED_ACTION(os::critical_section_lock)(m_CS);
	if (m_CppMap.size() > 2048)
	{
		// We can't store them forever
		m_CppMap.clear();
	}
	m_CppMap.emplace(CppObject, std::make_unique<exception_context>(ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo));
}

const exception_context* tracer::get_context(const void* CppObject) const
{
	SCOPED_ACTION(os::critical_section_lock)(m_CS);
	auto Iter = m_CppMap.find(CppObject);
	return Iter == m_CppMap.end()? nullptr : Iter->second.get();
}

const exception_context* tracer::get_exception_context(const void* CppObject)
{
	return StaticInstance? StaticInstance->get_context(CppObject) : nullptr;
}

class with_symbols
{
public:
	NONCOPYABLE(with_symbols);

	with_symbols()
	{
		if (StaticInstance)
			StaticInstance->SymInitialise();
	}

	~with_symbols()
	{
		if (StaticInstance)
			StaticInstance->SymCleanup();
	}
};

std::vector<string> tracer::get(const void* CppObject)
{
	const auto Context = StaticInstance? StaticInstance->get_context(CppObject) : nullptr;
	if (!Context)
		return {};

	SCOPED_ACTION(with_symbols);
	return GetSymbols(GetBackTrace(*Context));
}

std::vector<string> tracer::get(const exception_context& Context)
{
	SCOPED_ACTION(with_symbols);
	return GetSymbols(GetBackTrace(Context));
}

void tracer::get_one(const void* Ptr, string& Address, string& Name, string& Source)
{
	SCOPED_ACTION(with_symbols);
	GetSymbols({Ptr}, [&](string&& StrAddress, string&& StrName, string&& StrSource)
	{
		Address = std::move(StrAddress);
		Name = std::move(StrName);
		Source = std::move(StrSource);
	});
}

bool tracer::SymInitialise()
{
	if (!m_SymInitialised)
	{
		m_SymInitialised = imports::instance().SymInitialize(GetCurrentProcess(), EmptyToNull(m_SymbolSearchPath.data()), TRUE) != FALSE;
	}
	return m_SymInitialised;
}

void tracer::SymCleanup()
{
	if (m_SymInitialised)
	{
		m_SymInitialised = !imports::instance().SymCleanup(GetCurrentProcess());
	}
}
