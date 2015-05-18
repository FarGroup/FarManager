#pragma once

/*
Copyright © 2014 Far Group
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

// begin() / end() interface for null-terminated strings

#include "enumerator.hpp"

template<class T>
class as_string_t: public enumerator<T>
{
public:
	as_string_t(const T* str, size_t size): m_str(str), m_size(size) {}
	virtual bool get(size_t index, typename as_string_t::value_type& value) override
	{
		if (m_size == size_t(-1))
		{
			return (value = m_str[index]) != 0;
		}
		else
		{
			if (index == m_size)
			{
				return false;
			}
			else
			{
				value = m_str[index];
				return true;
			}
		}
	}

private:
	const T* m_str;
	size_t m_size;
};

template<class T>
typename std::enable_if<!std::is_array<T>::value, as_string_t<T>>::type as_string(const T* str, size_t size = size_t(-1)) { return as_string_t<T>(str, size); }

// to avoid processing null character in cases like char c[] = "foo".
template <typename T, size_t N>
typename std::enable_if<std::is_array<T>::value, as_string_t<T>>::type as_string(T(&str)[N]) { return as_string_t<T>(str, N - 1); }
