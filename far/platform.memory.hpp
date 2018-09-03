﻿#ifndef PLATFORM_MEMORY_HPP_87E958A6_C4DE_4F53_A9F6_337D97D664E6
#define PLATFORM_MEMORY_HPP_87E958A6_C4DE_4F53_A9F6_337D97D664E6
#pragma once

/*
platform.memory.hpp

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

namespace os::memory
{
	namespace global
	{
		namespace detail
		{
			struct deleter
			{
				void operator()(HGLOBAL MemoryBlock) const;
			};

			struct unlocker
			{
				void operator()(const void* MemoryBlock) const;
			};
		};

		using ptr = std::unique_ptr<std::remove_pointer_t<HGLOBAL>, detail::deleter>;

		ptr alloc(UINT Flags, size_t size);

		template<class T>
		using lock_t = std::unique_ptr<std::remove_pointer_t<T>, detail::unlocker>;

		template<class T>
		auto lock(HGLOBAL Ptr)
		{
			return lock_t<T>(static_cast<T>(GlobalLock(Ptr)));
		}

		template<class T>
		auto lock(const ptr& Ptr)
		{
			return lock<T>(Ptr.get());
		}

		template<class T>
		ptr copy(const T& Object)
		{
			static_assert(std::is_pod_v<T>);

			auto Memory = alloc(GMEM_MOVEABLE, sizeof(Object));
			if (!Memory)
				return nullptr;

			const auto Copy = lock<T*>(Memory);
			if (!Copy)
				return nullptr;

			*Copy = Object;
			return Memory;
		}

		ptr copy(const wchar_t* Data, size_t Size);
	}

	namespace local
	{
		namespace detail
		{
			struct deleter
			{
				void operator()(const void* MemoryBlock) const;
			};
		};

		template<class T>
		using ptr = std::unique_ptr<T, detail::deleter>;

		template<class T>
		auto alloc(UINT Flags, size_t size)
		{
			return ptr<T>(static_cast<T*>(LocalAlloc(Flags, size)));
		}
	};

	bool is_pointer(const void* Address);
}

#endif // PLATFORM_MEMORY_HPP_87E958A6_C4DE_4F53_A9F6_337D97D664E6
