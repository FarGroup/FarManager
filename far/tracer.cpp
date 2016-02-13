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

std::vector<string> printStack()
{
	std::vector<string> Result;
//#ifdef _MSC_VER
	// ## Windows Server 2003 and Windows XP:
	// ## The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
	const int kMaxCallers = 62;

	void* callers_stack[kMaxCallers];
	const auto process = GetCurrentProcess();
	Imports().SymInitialize(process, nullptr, TRUE);
	auto frames = Imports().RtlCaptureStackBackTrace(0, kMaxCallers, callers_stack, nullptr);
	const auto MaxNameLen = 2047;
	block_ptr<SYMBOL_INFO> Symbol(sizeof(SYMBOL_INFO) + MaxNameLen + 1);
	Symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	Symbol->MaxNameLen = MaxNameLen;
	frames = frames < kMaxCallers? frames : kMaxCallers;
	std::wostringstream s;
	for (unsigned int i = 0; i < frames; i++)
	{
		Imports().SymFromAddr(process, reinterpret_cast<DWORD_PTR>(callers_stack[i]), nullptr, Symbol.get());
		s << i << ": " << callers_stack[i] << " " << Symbol->Name << " - 0x" << Symbol->Address;
		Result.emplace_back(s.str());
		s.str(string());
	}
//#endif
	return Result;
}

LONG WINAPI StackLogger(EXCEPTION_POINTERS *xp)
{
	if (xp->ExceptionRecord->ExceptionCode == 0xE06D7363) // MS C++ exception
		tracer::GetInstance()->store(*reinterpret_cast<const std::exception*>(xp->ExceptionRecord->ExceptionInformation[1]), printStack());
	else
		tracer::GetInstance()->store(xp->ExceptionRecord, printStack());

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


void tracer::store(const EXCEPTION_RECORD* Record, std::vector<string>&& Trace)
{
	SCOPED_ACTION(CriticalSectionLock)(m_CS);
	m_SehMap.insert(std::make_pair(Record, std::move(Trace)));
}

void tracer::store(const std::exception& e, std::vector<string>&& Trace)
{
	SCOPED_ACTION(CriticalSectionLock)(m_CS);
	m_StdMap.insert(std::make_pair(&e, std::move(Trace)));
}

std::vector<string> tracer::get(const EXCEPTION_RECORD* Record)
{
	SCOPED_ACTION(CriticalSectionLock)(m_CS);
	return m_SehMap[Record];
}

std::vector<string> tracer::get(const std::exception& e)
{
	SCOPED_ACTION(CriticalSectionLock)(m_CS);
	return m_StdMap[&e];
}
