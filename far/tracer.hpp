#pragma once

/*
tracer.hpp
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

#include "synchro.hpp"
#include "farexcpt.hpp"

class tracer: noncopyable
{
public:
	tracer();
	~tracer();

	static tracer* GetInstance();

	void store(const void* CppObject, const EXCEPTION_POINTERS* ExceptionInfo);

	static std::vector<string> get(const void* CppObject);
	static std::vector<string> get(const EXCEPTION_POINTERS* e);
	static string get_one(const void* Address);

	static bool get_exception_context(const void* CppObject, EXCEPTION_RECORD& ExceptionRecord, CONTEXT& ContextRecord);

private:
	struct exception_context
	{
		EXCEPTION_RECORD ExceptionRecord;
		CONTEXT ContextRecord;
	};
	bool get_context(const void* CppObject, exception_context& Context) const;

	static bool SymInitialise();
	static void SymCleanup();

	static tracer* sTracer;
	mutable CriticalSection m_CS;
	std::unordered_map<const void*, exception_context> m_CppMap;
	veh_handler m_Handler;
	static bool m_SymInitialised;
};
