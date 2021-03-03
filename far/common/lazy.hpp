#ifndef LAZY_HPP_37F8CBE9_FC5C_491B_B3CD_8024E5B7CB5D
#define LAZY_HPP_37F8CBE9_FC5C_491B_B3CD_8024E5B7CB5D
#pragma once

/*
lazy.hpp
*/
/*
Copyright © 2021 Far Group
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

template<typename T>
class lazy
{

public:
	explicit lazy(std::function<T()> Initialiser):
		m_Initialiser(std::move(Initialiser))
	{
	}

	T& operator*()
	{
		return m_Data?
			*m_Data :
			*(m_Data = m_Initialiser());
	}

	T const& operator*() const
	{
		return *const_cast<lazy&>(*this);
	}

	T const* operator->() const
	{
		return &*this;
	}

	T* operator->()
	{
		return &*this;
	}

	auto& operator=(const T& rhs)
	{
		m_Data = rhs;
		return *this;
	}

	template<class U = T>
	auto& operator=(U&& rhs)
	{
		m_Data = FWD(rhs);
		return *this;
	}

private:
	std::function<T()> m_Initialiser;
	std::optional<T> mutable m_Data;
};

#endif // LAZY_HPP_37F8CBE9_FC5C_491B_B3CD_8024E5B7CB5D
