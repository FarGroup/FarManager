#ifndef BYTES_VIEW_HPP_3707377A_7C4B_4B2E_89EC_6411A1988FB3
#define BYTES_VIEW_HPP_3707377A_7C4B_4B2E_89EC_6411A1988FB3
#pragma once

/*
bytes_view.hpp
*/
/*
Copyright © 2016 Far Group
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

#include "range.hpp"
#include "utility.hpp"

//----------------------------------------------------------------------------

class bytes_view: public span<char const>
{
public:
	template<typename T>
	bytes_view(T const* const Data, size_t const Size):
		span<char const>(static_cast<char const*>(static_cast<void const*>(Data)), Size)
	{
		static_assert(std::disjunction_v<std::is_void<T>, std::is_trivially_copyable<T>>);
	}

	template<typename T>
	explicit bytes_view(const T& Object):
		bytes_view(&Object, sizeof(Object))
	{
		static_assert(std::is_trivially_copyable_v<T>);
	}
};

class bytes: public base<span<char>>
{
public:
	NONCOPYABLE(bytes);
	MOVABLE(bytes);

	bytes() = default;

	[[nodiscard]]
	static bytes copy(const bytes_view& Object)
	{
		bytes Bytes;
		Bytes = Object;
		return Bytes;
	}

	template<typename T>
	[[nodiscard]]
	static bytes copy(const T& Object)
	{
		return copy(bytes_view(Object));
	}

	template<typename T>
	[[nodiscard]]
	static bytes reference(T* const Data, size_t const Size)
	{
		static_assert(std::is_trivially_copyable_v<T>);

		bytes Bytes;
		static_cast<base_type&>(Bytes) = { static_cast<char*>(static_cast<void*>(Data)), Size };
		return Bytes;
	}

	template<typename T>
	[[nodiscard]]
	static bytes reference(T& Object)
	{
		return reference(&Object, sizeof(Object));
	}

	bytes& operator=(const bytes_view& rhs)
	{
		if (data())
		{
			if (size() != rhs.size())
				throw std::runtime_error("Incorrect blob size: "s + std::to_string(rhs.size()) + ", expected "s + std::to_string(size()));
		}
		else
		{
			m_Buffer = std::make_unique<char[]>(rhs.size());
			static_cast<base_type&>(*this) = { m_Buffer.get(), rhs.size() };
		}
		std::copy(ALL_CONST_RANGE(rhs), begin());
		return *this;
	}

	[[nodiscard]]
	operator bytes_view() const
	{
		return { data(), size() };
	}

private:
	std::unique_ptr<char[]> m_Buffer;
};

template<typename T>
[[nodiscard]]
T deserialise(const bytes_view& Bytes)
{
	T Value;
	bytes::reference(Value) = Bytes;
	return Value;
}

#endif // BYTES_VIEW_HPP_3707377A_7C4B_4B2E_89EC_6411A1988FB3
