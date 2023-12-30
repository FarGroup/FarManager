#ifndef FUNCTION_REF_HPP_0B2E3AF4_AB0A_4C89_9FC1_1A92AC2699A4
#define FUNCTION_REF_HPP_0B2E3AF4_AB0A_4C89_9FC1_1A92AC2699A4
#pragma once

/*
function_ref.hpp
*/
/*
Copyright © 2018 Far Group
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

#include "preprocessor.hpp"
#include "function_traits.hpp"

#include <bit>
#include <functional>

//----------------------------------------------------------------------------

template<typename callable_type>
class function_ref;

template<typename return_type, typename... args>
class function_ref<return_type(args...)> final
{
public:
WARNING_PUSH()
WARNING_DISABLE_MSC(4180) // qualifier applied to function type has no meaning; ignored
	template<typename callable_type> requires (!std::same_as<std::decay_t<callable_type>, function_ref>)
	explicit(false) function_ref(callable_type&& Callable) noexcept:
		m_Ptr(to_ptr(FWD(Callable))),
		m_ErasedFn([](intptr_t Ptr, args... Args) -> return_type
		{
			return std::invoke(from_ptr<callable_type>(Ptr), FWD(Args)...);
		})
	{
	}
WARNING_POP()

	explicit(false) function_ref(std::nullptr_t) noexcept:
		m_Ptr(),
		m_ErasedFn()
	{
	}

	decltype(auto) operator()(args... Args) const
	{
		return m_ErasedFn(m_Ptr, FWD(Args)...);
	}

	[[nodiscard]]
	explicit operator bool() const noexcept
	{
		return m_Ptr != 0;
	}

private:
	template<typename callable_type>
	static auto to_ptr(callable_type&& Callable)
	{
		if constexpr (std::is_pointer_v<callable_type>)
			return std::bit_cast<intptr_t>(Callable);
		else
			return std::bit_cast<intptr_t>(&Callable);
	}

	template<typename callable_type>
	static decltype(auto) from_ptr(intptr_t Ptr)
	{
		if constexpr (std::is_pointer_v<callable_type>)
			return std::bit_cast<callable_type>(Ptr);
		else
			return *std::bit_cast<std::add_pointer_t<callable_type>>(Ptr);
	}

	using signature_type = return_type(intptr_t, args...);

	intptr_t m_Ptr;
	signature_type* m_ErasedFn;
};

template<typename object>
function_ref(object) -> function_ref<typename function_traits<object>::signature_type>;

#endif // FUNCTION_REF_HPP_0B2E3AF4_AB0A_4C89_9FC1_1A92AC2699A4
