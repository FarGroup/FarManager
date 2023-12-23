#ifndef TRACER_HPP_AD7B9307_ECFD_46FC_B001_E48C9B89DE64
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

// Internal:

// Platform:
#include "platform.concurrency.hpp"
#include "platform.debug.hpp"

// Common:
#include "common/function_ref.hpp"
#include "common/nifty_counter.hpp"

// External:

//----------------------------------------------------------------------------

class map_file;

namespace tracer_detail
{
	class tracer
	{
	public:
		NONCOPYABLE(tracer);

		tracer();
		~tracer();

		void get_symbols(string_view Module, std::span<os::debug::stack_frame const> Trace, function_ref<void(string&& Line)> Consumer) const;
		void get_symbol(string_view Module, const void* Ptr, string& AddressStr, string& Name, string& Source) const;

		// Same as os::debug::*, but with symbols initialized
		std::vector<os::debug::stack_frame> current_stacktrace(string_view Module, size_t FramesToSkip = 0, size_t FramesToCapture = std::numeric_limits<size_t>::max()) const;
		std::vector<os::debug::stack_frame> stacktrace(string_view Module, CONTEXT ContextRecord, HANDLE ThreadHandle) const;
		std::vector<os::debug::stack_frame> exception_stacktrace(string_view Module) const;

		void current_stacktrace(string_view Module, function_ref<void(string&& Line)> Consumer, size_t FramesToSkip = 0, size_t FramesToCapture = std::numeric_limits<size_t>::max()) const;
		void stacktrace(string_view Module, function_ref<void(string&& Line)> Consumer, CONTEXT ContextRecord, HANDLE ThreadHandle) const;
		void exception_stacktrace(string_view Module, function_ref<void(string&& Line)> Consumer) const;

		class with_symbols
		{
		public:
			NONCOPYABLE(with_symbols);

			explicit with_symbols(string_view Module);
			~with_symbols();
		};

	private:
		void sym_initialise(string_view Module);
		void sym_cleanup();

		os::concurrency::critical_section m_CS;
		std::unique_ptr<std::unordered_map<uintptr_t, map_file>> m_MapFiles;
		size_t m_SymInitializeLevel{};
		bool m_SymInitialized{};
	};
}

NIFTY_DECLARE(tracer_detail::tracer, tracer);

#endif // TRACER_HPP_AD7B9307_ECFD_46FC_B001_E48C9B89DE64
