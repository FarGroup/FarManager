#pragma once

/*
variant.hpp
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
	class variant_base
	{
	public:
		virtual ~variant_base() {}
		virtual std::unique_ptr<variant_base> clone() const = 0;
	};

	template<class T>
	class variant_impl: noncopyable, public variant_base
	{
	public:
		variant_impl(const T& Data):
			m_Data(Data)
		{}

		virtual std::unique_ptr<variant_base> clone() const override { return std::make_unique<variant_impl>(m_Data); }

		const T& get() const noexcept { return m_Data; }
		T& get() noexcept{ return m_Data; }

	private:
		T m_Data;
	};
};

class variant
{
public:
	variant() {}

	variant(const variant& rhs):
		m_Data(rhs.m_Data->clone())
	{
	}

	template<class T>
	variant(const T& Data):
		m_Data(std::make_unique<detail::variant_impl<typename std::decay<T>::type>>(Data))
	{
	}

	variant(variant&& rhs) { *this = std::move(rhs); }

	variant& operator=(const variant& rhs)
	{
		variant(rhs.m_Data->clone().get()).swap(*this);
		return *this;
	}

	MOVE_OPERATOR_BY_SWAP(variant);
	FREE_SWAP(variant);

	void swap(variant& rhs) noexcept { m_Data.swap(rhs.m_Data); }

	template<class T>
	const T& get() const
	{
		return dynamic_cast<const detail::variant_impl<typename std::decay<T>::type>&>(*m_Data.get()).get();
	}

	template<class T>
	T& get()
	{
		return const_cast<T&>(const_cast<const variant*>(this)->get<T>());
	}

	template<class T>
	const T* get_ptr() const
	{
		const auto Derived = dynamic_cast<const detail::variant_impl<typename std::decay<T>::type>*>(m_Data.get());
		return Derived? &Derived->get() : nullptr;
	}

	template<class T>
	T* get_ptr()
	{
		return const_cast<T*>(const_cast<const variant*>(this)->get_ptr<T>());
	}

private:
	std::unique_ptr<detail::variant_base> m_Data;
};
