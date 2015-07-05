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

namespace detail
{
	class any_base
	{
	public:
		virtual ~any_base() {}
		virtual std::unique_ptr<any_base> clone() const = 0;
	};

	template<class T>
	class any_impl: noncopyable, public any_base
	{
	public:
		template<class Y>
		any_impl(Y&& Data):
			m_Data(std::forward<Y>(Data))
		{
		}

		virtual std::unique_ptr<any_base> clone() const override { return std::make_unique<any_impl>(m_Data); }

		const T& get() const noexcept { return m_Data; }
		T& get() noexcept { return m_Data; }

	private:
		T m_Data;
	};
};


class any: swapable<any>
{
public:
	any() {}

	any(const any& rhs):
		m_Data(construct(rhs))
	{
	}

	template<class T>
	any(T&& rhs):
		m_Data(construct(std::forward<T>(rhs)))
	{
	}

	any(any&& rhs) noexcept { *this = std::move(rhs); }

	any& operator=(const any& rhs)
	{
		any(rhs).swap(*this);
		return *this;
	}

	template<class T>
	any& operator=(T&& rhs)
	{
		any(std::forward<T>(rhs)).swap(*this);
		return *this;
	}

	MOVE_OPERATOR_BY_SWAP(any);

	void swap(any& rhs) noexcept { m_Data.swap(rhs.m_Data); }

	bool empty() const noexcept { return m_Data != nullptr; }

	template<class T>
	friend T* any_cast(any* Any) noexcept
	{
		if (Any)
		{
			auto Impl = dynamic_cast<detail::any_impl<T>*>(Any->m_Data.get());
			return Impl? &Impl->get() : nullptr;
		}
		return nullptr;
	}

private:
	template<class T>
	static typename std::enable_if<!std::is_same<typename std::decay<T>::type, any>::value, std::unique_ptr<detail::any_base>>::type construct(T&& rhs)
	{
		return std::make_unique<detail::any_impl<typename std::decay<T>::type>>(std::forward<T>(rhs));
	}

	static std::unique_ptr<detail::any_base> construct(const any& rhs)
	{
		return rhs.m_Data? rhs.m_Data->clone() : nullptr;
	}

	std::unique_ptr<detail::any_base> m_Data;
};

template<class T>
const T* any_cast(const any* Any) noexcept
{
	return any_cast<T>(const_cast<any*>(Any));
}

template<class T>
T& any_cast(any& Any)
{
	auto Result = any_cast<T>(&Any);
	if (!Result)
	{
		throw std::bad_cast();
	}
	return *Result;
}

template<class T>
const T& any_cast(const any& Any)
{
	return any_cast<T>(const_cast<any&>(Any));
}
