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

static std::vector<const void*> GetBackTrace(const EXCEPTION_POINTERS* ExceptionInfo)
{
	std::vector<const void*> Result;

	// StackWalk64() may modify context record passed to it, so we will use a copy.
	auto ContextRecord = *ExceptionInfo->ContextRecord;
	STACKFRAME64 StackFrame = {};
#if defined(_WIN64)
	int machine_type = IMAGE_FILE_MACHINE_AMD64;
	StackFrame.AddrPC.Offset = ContextRecord.Rip;
	StackFrame.AddrFrame.Offset = ContextRecord.Rbp;
	StackFrame.AddrStack.Offset = ContextRecord.Rsp;
#else
	int machine_type = IMAGE_FILE_MACHINE_I386;
	StackFrame.AddrPC.Offset = ContextRecord.Eip;
	StackFrame.AddrFrame.Offset = ContextRecord.Ebp;
	StackFrame.AddrStack.Offset = ContextRecord.Esp;
#endif
	StackFrame.AddrPC.Mode = AddrModeFlat;
	StackFrame.AddrFrame.Mode = AddrModeFlat;
	StackFrame.AddrStack.Mode = AddrModeFlat;

	block_ptr<SYMBOL_INFO> Symbol(sizeof(SYMBOL_INFO) + 256);
	Symbol->MaxNameLen = 255;
	Symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	while (Imports().StackWalk64(machine_type, GetCurrentProcess(), GetCurrentThread(), &StackFrame, &ContextRecord, nullptr, nullptr, nullptr, nullptr))
	{
		Result.push_back((const void*)StackFrame.AddrPC.Offset);
	}

	return Result;
}

static std::vector<string> GetSymbols(const std::vector<const void*>& BackTrace)
{
	std::vector<string> Result;

	const auto Process = GetCurrentProcess();
	const auto MaxNameLen = MAX_SYM_NAME;
	block_ptr<SYMBOL_INFO> Symbol(sizeof(SYMBOL_INFO) + MaxNameLen + 1);
	Symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	Symbol->MaxNameLen = MaxNameLen;

	Imports().SymSetOptions(SYMOPT_LOAD_LINES);
	IMAGEHLP_LINE64 Line = { sizeof(Line) };
	DWORD Displacement;

	std::wostringstream Stream;
	FOR(const auto i, BackTrace)
	{
		Stream << L"0x" << i;
		const auto Address = reinterpret_cast<DWORD_PTR>(i);
		if (Imports().SymFromAddr(Process, Address, nullptr, Symbol.get()))
		{
			Stream << " " << Symbol->Name;
		}
		if (Imports().SymGetLineFromAddr64(Process, Address, &Displacement, &Line))
		{
			Stream << L" (" << Line.FileName << L":" << Line.LineNumber << L")";
		}
		Result.emplace_back(Stream.str());
		Stream.str(string());
	}
	return Result;
}

LONG WINAPI StackLogger(EXCEPTION_POINTERS *xp)
{
	static const auto MSCPPExceptionCode = 0xE06D7363;
	if (xp->ExceptionRecord->ExceptionCode == MSCPPExceptionCode)
	{
		tracer::GetInstance()->store(*reinterpret_cast<const std::exception*>(xp->ExceptionRecord->ExceptionInformation[1]), xp);
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

tracer* tracer::sTracer;

tracer* tracer::GetInstance()
{
	return sTracer;
}

tracer::tracer():
	m_Handler(StackLogger)
{
	sTracer = this;
}

tracer::~tracer()
{
	sTracer = nullptr;
}

void tracer::store(const std::exception& e, const EXCEPTION_POINTERS* ExceptionInfo)
{
	SCOPED_ACTION(CriticalSectionLock)(m_CS);
	exception_context Context = { *ExceptionInfo->ExceptionRecord, *ExceptionInfo->ContextRecord };
	m_StdMap.insert(std::make_pair(&e, Context));
}

bool tracer::get_context(const std::exception& e, exception_context& Context) const
{
	SCOPED_ACTION(CriticalSectionLock)(m_CS);
	auto Iter = m_StdMap.find(&e);
	if (Iter == m_StdMap.end())
	{
		return false;
	}
	Context = Iter->second;
	return true;
}

std::vector<string> tracer::get(const std::exception& e)
{
	exception_context Context;
	if (!tracer::GetInstance()->get_context(e, Context))
	{
		return std::vector<string>();
	}
	EXCEPTION_POINTERS xp = { &Context.ExceptionRecord, &Context.ContextRecord };

	SymInitialise();
	SCOPE_EXIT{ SymCleanup(); };

	return GetSymbols(GetBackTrace(&xp));
}

std::vector<string> tracer::get(const EXCEPTION_POINTERS* e)
{
	SymInitialise();
	SCOPE_EXIT{ SymCleanup(); };

	return GetSymbols(GetBackTrace(e));
}

string tracer::get(const void* Address)
{
	SymInitialise();
	SCOPE_EXIT{ SymCleanup(); };

	return GetSymbols(make_vector<const void*>(Address)).front();
}

bool tracer::m_SymInitialised;

bool tracer::SymInitialise()
{
	if (!m_SymInitialised)
	{
		m_SymInitialised = Imports().SymInitialize(GetCurrentProcess(), nullptr, TRUE) != FALSE;
	}
	return m_SymInitialised;
}

void tracer::SymCleanup()
{
	if (m_SymInitialised)
	{
		m_SymInitialised = !Imports().SymCleanup(GetCurrentProcess());
	}
}
