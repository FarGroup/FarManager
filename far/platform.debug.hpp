#ifndef PLATFORM_DEBUG_HPP_8453E69F_3955_416D_BB64_A3A88D3D1D8D
#define PLATFORM_DEBUG_HPP_8453E69F_3955_416D_BB64_A3A88D3D1D8D
#pragma once

/*
platform.debug.hpp

*/
/*
Copyright © 2022 Far Group
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

// Common:
#include "common/function_ref.hpp"

// External:

//----------------------------------------------------------------------------

class map_file;

namespace os::debug
{
	// TODO: std
	bool is_debugger_present();
	void breakpoint();
	void breakpoint_if_debugging();

	void print(const wchar_t* Str);
	void print(string const& Str);
	void set_thread_name(const wchar_t* Name);
	void set_thread_name(string const& Name);
	string get_thread_name(HANDLE ThreadHandle);

	struct stack_frame
	{
		uintptr_t Address;
		DWORD InlineContext;
	};

	constexpr NTSTATUS EH_EXCEPTION_NUMBER = 0xE06D7363; // 'msc'

	EXCEPTION_POINTERS exception_information();
	EXCEPTION_POINTERS fake_exception_information(unsigned Code, bool Continuable = false);

	// Symbols should be initialized before calling these.
	// Use tracer.*, they do exactly that.
	std::vector<stack_frame> current_stacktrace(size_t FramesToSkip = 0, size_t FramesToCapture = std::numeric_limits<size_t>::max());
	std::vector<stack_frame> stacktrace(CONTEXT ContextRecord, HANDLE ThreadHandle);
	std::vector<stack_frame> exception_stacktrace();

	bool is_inline_frame(DWORD InlineContext);

	namespace symbols
	{
		struct symbol
		{
			string_view Name;
			size_t Displacement{};
		};

		struct location
		{
			string_view FileName;
			std::optional<size_t> Line;
			size_t Displacement{};
		};

		bool initialize(string_view Module);

		void clean();

		void get(
			string_view ModuleName,
			std::span<stack_frame const> BackTrace,
			std::unordered_map<uintptr_t, map_file>& MapFiles,
			function_ref<void(uintptr_t, string_view, bool, symbol, location)> Consumer
		);
	}

	void crt_report_to_ui();
	void crt_report_to_stderr();
}

#endif // PLATFORM_DEBUG_HPP_8453E69F_3955_416D_BB64_A3A88D3D1D8D
