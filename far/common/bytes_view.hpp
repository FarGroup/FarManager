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

#include "movable.hpp"
#include "range.hpp"

class bytes_view: public range<const char*>
{
public:
	bytes_view(const void* Data, size_t Size):
		range<const char*>(reinterpret_cast<const char*>(Data), reinterpret_cast<const char*>(Data) + Size)
	{
	}

	template<typename T>
	explicit bytes_view(const T& Object):
		bytes_view(&Object, sizeof(Object))
	{
		static_assert(std::is_trivially_copyable_v<T>);
	}
};

class bytes: public range<char*>
{
public:
	NONCOPYABLE(bytes);
	MOVABLE(bytes);

	bytes() = default;

	static bytes copy(const bytes_view& Object)
	{
		bytes Bytes;
		Bytes = Object;
		return Bytes;
	}

	template<typename T>
	static bytes copy(const T& Object)
	{
		return copy(bytes_view(Object));
	}

	template<typename T>
	static bytes reference(T& Object)
	{
		static_assert(std::is_trivially_copyable_v<T>);

		bytes Bytes;
		static_cast<range<char*>&>(Bytes) =
		{
			static_cast<char*>(static_cast<void*>(&Object)),
			static_cast<char*>(static_cast<void*>(&Object)) + sizeof(Object)
		};
		return Bytes;
	}

	~bytes()
	{
		if (*m_Allocated)
			delete[] static_cast<const char*>(data());
	}

	bytes& operator=(const bytes_view& rhs)
	{
		if (data())
		{
			if (size() != rhs.size())
				throw std::runtime_error("Incorrect blob size: " + std::to_string(rhs.size()) + ", expected " + std::to_string(size()));
		}
		else
		{
			static_cast<range<char*>&>(*this) = make_range(new char[rhs.size()], rhs.size());
			*m_Allocated = true;
		}
		std::copy(ALL_CONST_RANGE(rhs), begin());
		return *this;
	}

	operator bytes_view() const
	{
		return { data(), size() };
	}

private:
	movable<bool> m_Allocated{ false };
};

template<typename T>
T deserialise(const bytes_view& Bytes)
{
	T Value;
	bytes::reference(Value) = Bytes;
	return Value;
}

#endif // BYTES_VIEW_HPP_3707377A_7C4B_4B2E_89EC_6411A1988FB3
