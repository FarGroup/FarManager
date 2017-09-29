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

#include "headers.hpp"
#pragma hdrstop

#include "platform.memory.hpp"

namespace os::memory
{
	namespace global
	{
		namespace detail
		{
			void deleter::operator()(HGLOBAL MemoryBlock) const
			{
				GlobalFree(MemoryBlock);
			}

			void unlocker::operator()(const void* MemoryBlock) const
			{
				GlobalUnlock(const_cast<HGLOBAL>(MemoryBlock));
			}
		}

		ptr alloc(UINT Flags, size_t size)
		{
			return ptr(GlobalAlloc(Flags, size));
		}

		ptr copy(const wchar_t* Data, size_t Size)
		{
			auto Memory = alloc(GMEM_MOVEABLE, (Size + 1) * sizeof(wchar_t));
			if (!Memory)
				return nullptr;

			const auto Copy = lock<wchar_t*>(Memory);
			if (!Copy)
				return nullptr;

			*std::copy_n(Data, Size, Copy.get()) = L'\0';
			return Memory;
		}

	}

	namespace local
	{
		namespace detail
		{
			void deleter::operator()(HLOCAL MemoryBlock) const
			{
				LocalFree(MemoryBlock);
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

		return InRange(
			reinterpret_cast<uintptr_t>(info.lpMinimumApplicationAddress),
			reinterpret_cast<uintptr_t>(Address),
			reinterpret_cast<uintptr_t>(info.lpMaximumApplicationAddress)
		);
	}
}
