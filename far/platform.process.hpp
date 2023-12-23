#ifndef PLATFORM_PROCESS_HPP_234140CB_C857_40CF_901D_A10C5EBEA85B
#define PLATFORM_PROCESS_HPP_234140CB_C857_40CF_901D_A10C5EBEA85B
#pragma once

/*
platform.process.hpp

*/
/*
Copyright © 2020 Far Group
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
#include "platform.hpp"

// Common:
#include "common/enumerator.hpp"
#include "common/function_ref.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::process
{
	enum class image_type
	{
		unknown,
		console,
		graphical,
	};

	image_type get_process_subsystem(HANDLE Process);
	string get_process_name(DWORD Pid);

	using enumerate_callback = function_ref<bool(DWORD Pid, const wchar_t* AppName, const wchar_t* ServiceShortName)>;
	size_t enumerate_locking_processes_rm(const string& Filename, DWORD& Reasons, enumerate_callback Handler);
	size_t enumerate_locking_processes_nt(string_view Filename, enumerate_callback Handler);

	struct enum_process_entry
	{
		DWORD Pid;
		string_view Name;
		std::span<SYSTEM_THREAD_INFORMATION const> Threads;
	};

	class [[nodiscard]] enum_processes: public enumerator<enum_processes, enum_process_entry>
	{
		IMPLEMENTS_ENUMERATOR(enum_processes);

	public:
		explicit enum_processes();

	private:
		[[nodiscard]]
		bool get(bool Reset, enum_process_entry& Value) const;

		block_ptr<SYSTEM_PROCESS_INFORMATION> m_Info;
		mutable size_t m_Offset{};
	};

	bool terminate_other(int Pid);
	[[noreturn]]
	void terminate(int ExitCode);
	[[noreturn]]
	void terminate_by_user(int ExitCode = EXIT_FAILURE);
}

#endif // PLATFORM_PROCESS_HPP_234140CB_C857_40CF_901D_A10C5EBEA85B
