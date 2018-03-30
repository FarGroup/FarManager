#ifndef ANY_HPP_A07ADDF9_9BA4_4CE8_AE6E_14C3B7A61D35
#define ANY_HPP_A07ADDF9_9BA4_4CE8_AE6E_14C3B7A61D35
#pragma once

/*
any.hpp
*/
/*
Copyright © 2015 Far Group
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

namespace detail
{
	class any_base
	{
	public:
		virtual ~any_base() = default;
		virtual std::unique_ptr<any_base> clone() const = 0;
	};

	template<class T>
	class any_impl: public any_base
	{
	public:
		NONCOPYABLE(any_impl);

		template<class Y>
		explicit any_impl(Y&& Data):
			m_Data(FWD(Data))
		{
		}

		std::unique_ptr<any_base> clone() const override { return std::make_unique<any_impl>(m_Data); }

		const T& get() const noexcept { return m_Data; }
		T& get() noexcept { return m_Data; }

	private:
		T m_Data;
	};
}


class any
{
public:
	MOVABLE(any);

	any() = default;

	any(const any& rhs):
		m_Data(construct(rhs))
	{
	}

	template<class T>
	any(T&& rhs):
		m_Data(construct(FWD(rhs)))
	{
	}

	any& operator=(const any& rhs)
	{
		auto Tmp(rhs);
		using std::swap;
		swap(*this, Tmp);
		return *this;
	}

	template<class T>
	any& operator=(T&& rhs)
	{
		any Tmp(FWD(rhs));
		using std::swap;
		swap(*this, Tmp);
		return *this;
	}

	bool has_value() const noexcept { return m_Data != nullptr; }

	template<typename any_type>
	friend auto detail_get_impl(any_type* Any)
	{
		return Any->m_Data.get();
	}

private:
	template<class T, REQUIRES(!std::is_same_v<std::decay_t<T>, any>)>
	static std::unique_ptr<detail::any_base> construct(T&& rhs)
	{
		return std::make_unique<detail::any_impl<std::decay_t<T>>>(FWD(rhs));
	}

	static std::unique_ptr<detail::any_base> construct(const any& rhs)
	{
		return rhs.m_Data? rhs.m_Data->clone() : nullptr;
	}

	std::unique_ptr<detail::any_base> m_Data;
};

class bad_any_cast: public std::bad_cast
{
	using bad_cast::bad_cast;
};

template<class T>
T* any_cast(any* Any) noexcept
{
	if (!Any)
		return nullptr;

	const auto Impl = dynamic_cast<detail::any_impl<T>*>(detail_get_impl(Any));
	return Impl? &Impl->get() : nullptr;
}

template<class T>
const T* any_cast(const any* Any) noexcept
{
	if (!Any)
		return nullptr;

	const auto Impl = dynamic_cast<const detail::any_impl<T>*>(detail_get_impl(Any));
	return Impl? &Impl->get() : nullptr;
}

template<class T>
T any_cast(any& Any)
{
	const auto Ptr = any_cast<std::remove_cv_t<std::remove_reference_t<T>>>(&Any);
	if (!Ptr)
		throw bad_any_cast{};

	return static_cast<T>(*Ptr);
}

template<class T>
T any_cast(const any& Any)
{
	return any_cast<T>(const_cast<any&>(Any));
}

#endif // ANY_HPP_A07ADDF9_9BA4_4CE8_AE6E_14C3B7A61D35
