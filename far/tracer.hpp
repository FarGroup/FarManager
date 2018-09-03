﻿#ifndef TRACER_HPP_AD7B9307_ECFD_46FC_B001_E48C9B89DE64
#define TRACER_HPP_AD7B9307_ECFD_46FC_B001_E48C9B89DE64
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

#include "platform.concurrency.hpp"

class tracer: noncopyable
{
public:
	tracer();
	~tracer();

	void store(const void* CppObject, const EXCEPTION_POINTERS* ExceptionInfo);

	static std::vector<string> get(const void* CppObject);
	static std::vector<string> get(const exception_context& Context);
	static void get_one(const void* Ptr, string& Address, string& Name, string& Source);

	static std::unique_ptr<exception_context> get_exception_context(const void* CppObject);

	static auto with_symbols()
	{
		return make_raii_wrapper(get_instance(),
			[](tracer* const Tracer)
			{
				if (Tracer)
					Tracer->SymInitialise();
			},
			[](tracer* const Tracer)
			{
				if (Tracer)
					Tracer->SymCleanup();
			});
	}

private:
	static tracer* get_instance();

	std::unique_ptr<exception_context> get_context(const void* CppObject);

	bool SymInitialise();
	void SymCleanup();

	static tracer* sTracer;
	mutable os::critical_section m_CS;
	std::unordered_map<const void*, std::unique_ptr<exception_context>> m_CppMap;
	std::string m_SymbolSearchPath;

	class veh_handler: noncopyable
	{
	public:
		explicit veh_handler(PVECTORED_EXCEPTION_HANDLER Handler);
		~veh_handler();

	private:
		void* m_Handler;
	}
	m_Handler;
	std::atomic_int32_t m_SymInitialised{};
};

#endif // TRACER_HPP_AD7B9307_ECFD_46FC_B001_E48C9B89DE64
