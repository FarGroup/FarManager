#ifndef MULTIFUNCTION_HPP_C07E54A9_8AD7_4BAB_98C9_B8B1BF309CAA
#define MULTIFUNCTION_HPP_C07E54A9_8AD7_4BAB_98C9_B8B1BF309CAA
#pragma once

/*
multifunction.hpp
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

//----------------------------------------------------------------------------

namespace detail
{
	template<typename...>
	struct icallable_base;

	template<typename result_type, typename... args>
	struct icallable_base<result_type(args...)>
	{
		virtual ~icallable_base() = default;
		virtual result_type operator()(args&&...) const = 0;
	};

	template<typename result_type, typename... args, typename... signatures>
	struct icallable_base<result_type(args...), signatures...>: icallable_base<signatures...>
	{
		using icallable_base<signatures...>::operator();

		~icallable_base() override = default;
		virtual result_type operator()(args&&...) const = 0;
	};

	template<typename... signatures>
	struct icallable: icallable_base<signatures...>
	{
		virtual std::unique_ptr<icallable> clone() const = 0;
	};


	template<typename interface_type, typename callable_type, typename... signatures>
	class callable_base;

	template<typename interface_type, typename callable_type>
	class callable_base<interface_type, callable_type> : public interface_type
	{
	public:
		NONCOPYABLE(callable_base);

		explicit callable_base(const callable_type& Callable):
			m_Callable(Callable)
		{
		}

	protected:
		callable_type m_Callable;
	};

	template<typename interface_type, typename callable_type, typename result_type, typename... args, typename... signatures>
	class callable_base<interface_type, callable_type, result_type(args...), signatures...>: public callable_base<interface_type, callable_type, signatures...>
	{
	public:
		NONCOPYABLE(callable_base);

		explicit callable_base(const callable_type& Callable):
			callable_base<interface_type, callable_type, signatures...>(Callable)
		{
		}

		using callable_base<interface_type, callable_type, signatures...>::operator();

		result_type operator()(args&&... Args) const override
		{
			return this->m_Callable(FWD(Args)...);
		}
	};

	template<typename callable_type, typename... signatures>
	class callable: public callable_base<icallable<signatures...>, callable_type, signatures...>
	{
	public:
		NONCOPYABLE(callable);

		explicit callable(const callable_type& Callable):
			callable_base<icallable<signatures...>, callable_type, signatures...>(Callable)
		{
		}

		[[nodiscard]]
		std::unique_ptr<icallable<signatures...>> clone() const override
		{
			return std::make_unique<callable>(this->m_Callable);
		}
	};
}

template<typename... signatures>
class multifunction
{
public:
	MOVABLE(multifunction);
	COPY_AND_MOVE(multifunction, const multifunction&)

	multifunction() = default;

	template<typename callable_type>
	multifunction(const callable_type& Callable):
		m_Callable(std::make_unique<detail::callable<callable_type, signatures...>>(Callable))
	{
	}

	multifunction(const multifunction& rhs):
		m_Callable(rhs? rhs.m_Callable->clone() : nullptr)
	{
	}

	template<typename... args>
	auto operator()(args&&... Args)
	{
		return (*m_Callable)(FWD(Args)...);
	}

	template<typename... args>
	auto operator()(args&&... Args) const
	{
		return (*m_Callable)(FWD(Args)...);
	}

	[[nodiscard]]
	explicit operator bool() const noexcept
	{
		return m_Callable.operator bool();
	}

private:
	std::unique_ptr<detail::icallable<signatures...>> m_Callable;
};

#endif // MULTIFUNCTION_HPP_C07E54A9_8AD7_4BAB_98C9_B8B1BF309CAA
