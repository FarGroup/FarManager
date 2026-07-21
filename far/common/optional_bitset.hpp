#ifndef OPTIONAL_BITSET_HPP_2F835FC0_7824_407C_8018_3013F670DF15
#define OPTIONAL_BITSET_HPP_2F835FC0_7824_407C_8018_3013F670DF15
#pragma once

/*
optional_bitset.hpp
*/
/*
Copyright © 2026 Far Group
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

#include <bitset>

//----------------------------------------------------------------------------

enum class optional_bitset_state
{
	unknown,
	yes,
	no
};

template<size_t N>
class optional_bitset
{
public:
	void assign(size_t const Bit, bool const Value)
	{
		m_Bits.set(Bit, Value);
		m_Bits.set(N + Bit);
	}

	void assign(size_t const Bit, optional_bitset_state const Value)
	{
		if (Value == optional_bitset_state::unknown)
		{
			m_Bits.reset(N + Bit);
			return;
		}

		assign(Bit, Value == optional_bitset_state::yes);
	}


	void reset()
	{
		m_Bits.reset();
	}

	optional_bitset_state check(size_t const Bit) const
	{
		if (!m_Bits[N + Bit])
			return optional_bitset_state::unknown;

		return m_Bits[Bit] ? optional_bitset_state::yes : optional_bitset_state::no;
	}

private:
	std::bitset<N * 2> m_Bits;
};

#endif // OPTIONAL_BITSET_HPP_2F835FC0_7824_407C_8018_3013F670DF15
