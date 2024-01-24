/*
platform.memory.cpp

*/
/*
Copyright © 2017 Far Group
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
#include "platform.memory.hpp"

// Internal:
#include "imports.hpp"

// Platform:

// Common:
#include "common/algorithm.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::memory
{
	namespace local
	{
		namespace detail
		{
			void deleter::operator()(const void* MemoryBlock) const noexcept
			{
				LocalFree(const_cast<HLOCAL>(MemoryBlock));
			}
		}
	}

	bool is_pointer(const void* Address)
	{
		static const auto info = []
		{
			SYSTEM_INFO Info;
			GetSystemInfo(&Info);
			return Info;
		}();

		return in_closed_range(
			std::bit_cast<uintptr_t>(info.lpMinimumApplicationAddress),
			std::bit_cast<uintptr_t>(Address),
			std::bit_cast<uintptr_t>(info.lpMaximumApplicationAddress)
		);
	}

	void enable_low_fragmentation_heap()
	{
		// Starting with Windows Vista, the system uses the low-fragmentation heap (LFH) as needed to service memory allocation requests.
		// Applications do not need to enable the LFH for their heaps.
		if (IsWindowsVistaOrGreater())
			return;

		if (!imports.HeapSetInformation)
			return;

		std::vector<HANDLE> Heaps(10);
		for (;;)
		{
			const auto NumberOfHeaps = GetProcessHeaps(static_cast<DWORD>(Heaps.size()), Heaps.data());
			const auto Received = NumberOfHeaps <= Heaps.size();
			Heaps.resize(NumberOfHeaps);
			if (Received)
				break;
		}

		for (const auto i: Heaps)
		{
			ULONG HeapFragValue = 2;
			imports.HeapSetInformation(i, HeapCompatibilityInformation, &HeapFragValue, sizeof(HeapFragValue));
		}
	}
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("os.memory.is_pointer")
{
	REQUIRE(!os::memory::is_pointer(nullptr));
	REQUIRE(!os::memory::is_pointer(std::bit_cast<void*>(42uz)));
	REQUIRE(os::memory::is_pointer("42"));
}
#endif
