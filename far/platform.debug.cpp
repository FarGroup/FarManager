/*
platform.debug.cpp

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "platform.debug.hpp"

// Internal:
#include "imports.hpp"

// Platform:
#include "platform.hpp"
#include "platform.memory.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

namespace os::debug
{
	bool debugger_present()
	{
		return IsDebuggerPresent() != FALSE;
	}

	void breakpoint(bool const Always)
	{
		if (Always || debugger_present())
			DebugBreak();
	}

	void print(const wchar_t* const Str)
	{
		OutputDebugString(Str);
	}

	void print(string const& Str)
	{
		print(Str.c_str());
	}

	void set_thread_name(const wchar_t* const Name)
	{
		if (imports.SetThreadDescription)
			imports.SetThreadDescription(GetCurrentThread(), Name);
	}

	string get_thread_name(HANDLE const ThreadHandle)
	{
		if (!imports.GetThreadDescription)
			return {};

		memory::local::ptr<wchar_t> Name;
		if (FAILED(imports.GetThreadDescription(ThreadHandle, &ptr_setter(Name))))
			return {};

		return Name.get();
	}

	std::vector<uintptr_t> current_stack(size_t const FramesToSkip, size_t const FramesToCapture)
	{
		if (!imports.RtlCaptureStackBackTrace)
			return {};

		std::vector<uintptr_t> Stack;
		Stack.reserve(128);

		// http://web.archive.org/web/20140815000000*/http://msdn.microsoft.com/en-us/library/windows/hardware/ff552119(v=vs.85).aspx
		// In Windows XP and Windows Server 2003, the sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
		static const auto Limit = IsWindowsVistaOrGreater()? std::numeric_limits<size_t>::max() : 62;

		const auto Skip = FramesToSkip + 1; // 1 for this frame
		const auto Capture = std::min(FramesToCapture, Limit - Skip);

		for (size_t i = 0; i != FramesToCapture;)
		{
			void* Pointers[128];

			DWORD DummyHash;
			const auto Size = imports.RtlCaptureStackBackTrace(
				static_cast<DWORD>(Skip + i),
				static_cast<DWORD>(std::min(std::size(Pointers), Capture - i)),
				Pointers,
				&DummyHash // MSDN says it's optional, but it's not true on Win2k
			);

			if (!Size)
				break;

			std::transform(Pointers, Pointers + Size, std::back_inserter(Stack), [](void* Ptr)
			{
				return reinterpret_cast<uintptr_t>(Ptr);
			});

			i += Size;
		}

		return Stack;
	}
}
