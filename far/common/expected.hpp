#ifndef EXPECTED_HPP_01A3444E_1CF2_446D_851F_F91C5A5A34AA
#define EXPECTED_HPP_01A3444E_1CF2_446D_851F_F91C5A5A34AA
#pragma once

/*
expected.hpp
*/
/*
Copyright © 2024 Far Group
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

#include <stdexcept>
#include <variant>

//----------------------------------------------------------------------------

template<typename T, typename E, typename W = E>
class expected
{
public:
	expected() = default;

	template<typename U>
	explicit(false) expected(U&& Value):
		m_Data(FWD(Value))
	{}

	bool has_value() const
	{
		return m_Data.index() == 0;
	}

	explicit operator bool() const
	{
		return has_value();
	}

	T const& value() const
	{
		if (has_value())
			return std::get<0>(m_Data);

		throw W{ std::get<1>(m_Data) };
	}

	T& value()
	{
		return const_cast<T&>(std::as_const(*this).value());
	}

	const T& operator*() const
	{
		return value();
	}

	T& operator*()
	{
		return value();
	}

	const T* operator->() const
	{
		return &value();
	}

	T* operator->()
	{
		return &value();
	}

	E const& error() const
	{
		if (!has_value())
			return std::get<1>(m_Data);

		throw std::logic_error("No error stored");
	}

private:
	std::variant<T, E> m_Data;
};

#endif // EXPECTED_HPP_01A3444E_1CF2_446D_851F_F91C5A5A34AA
