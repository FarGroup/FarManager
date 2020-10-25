#ifndef PLATFORM_MEMORY_HPP_87E958A6_C4DE_4F53_A9F6_337D97D664E6
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

// Internal:

// Platform:

// Common:
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::memory
{
	namespace global
	{
		namespace detail
		{
			struct deleter
			{
				void operator()(HGLOBAL MemoryBlock) const noexcept;
			};

			struct unlocker
			{
				void operator()(const void* MemoryBlock) const noexcept;
			};
		}

		using ptr = std::unique_ptr<std::remove_pointer_t<HGLOBAL>, detail::deleter>;

		ptr alloc(unsigned Flags, size_t Size);

		template<class T>
		using lock_t = std::unique_ptr<std::remove_pointer_t<T>, detail::unlocker>;

		template<class T>
		[[nodiscard]]
		auto lock(HGLOBAL Ptr) noexcept
		{
			return lock_t<T>(static_cast<T>(GlobalLock(Ptr)));
		}

		template<class T>
		[[nodiscard]]
		auto lock(const ptr& Ptr) noexcept
		{
			return lock<T>(Ptr.get());
		}

		ptr copy(HGLOBAL Ptr);

		template<class T>
		[[nodiscard]]
		ptr copy(const T& Object)
		{
			static_assert(std::is_trivially_copyable_v<T>);

			auto Memory = alloc(GMEM_MOVEABLE, sizeof(Object));
			if (!Memory)
				return nullptr;

			const auto Copy = lock<T*>(Memory);
			if (!Copy)
				return nullptr;

			*Copy = Object;
			return Memory;
		}

		[[nodiscard]]
		ptr copy(string_view Str);
	}

	namespace local
	{
		namespace detail
		{
			struct deleter
			{
				void operator()(const void* MemoryBlock) const noexcept;
			};
		}

		template<class T>
		class ptr: public base<std::unique_ptr<T, detail::deleter>>
		{
			using ptr::base_ctor::base_ctor;
		};

		template<class T>
		ptr(T*) -> ptr<T>;

		template<class T>
		[[nodiscard]]
		auto alloc(unsigned const Flags, size_t const Size)
		{
			return ptr<T>(static_cast<T*>(LocalAlloc(Flags, Size)));
		}

		template<class T>
		ptr<T> to_ptr(T* Ptr)
		{
			return ptr<T>{ Ptr };
		}
	}

	[[nodiscard]]
	bool is_pointer(const void* Address);

	void enable_low_fragmentation_heap();
}

#endif // PLATFORM_MEMORY_HPP_87E958A6_C4DE_4F53_A9F6_337D97D664E6
