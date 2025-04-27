#ifndef SEGMENT_HPP_DC38F7F7_0E1B_486E_8368_2ACE6415D234
#define SEGMENT_HPP_DC38F7F7_0E1B_486E_8368_2ACE6415D234
#pragma once

/*
segment.hpp

*/
/*
Copyright © 2025 Far Group
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
class segment_t
{
public:
	struct sentinel_tag { T m_end{}; };
	struct length_tag { T m_length{}; };

	segment_t() noexcept = default;

	segment_t(T const Start, sentinel_tag const End) noexcept
		: segment_t{ Start, End.m_end }
	{}

	segment_t(T const Start, length_tag const Length) noexcept
		: segment_t{ Start, static_cast<T>(Start + Length.m_length) }
	{}

	template<typename Y>
	explicit(false) segment_t(segment_t<Y> const Segment) noexcept:
		segment_t(Segment.m_Start, Segment.m_End)
	{}

	[[nodiscard]]
	auto length() const noexcept { assert(m_Start <= m_End); return m_End - m_Start; }

	[[nodiscard]]
	auto empty() const noexcept { return !length(); }

	// Not begin to avoid accidental misuse of std::begin(MySegment)
	[[nodiscard]]
	auto start() const noexcept { assert(!empty()); return m_Start; }

	[[nodiscard]]
	auto end() const noexcept { assert(!empty()); return m_End; }

	bool operator==(segment_t const& Other) const noexcept
	{
		return (empty() && Other.empty())
			|| (m_Start == Other.m_Start && m_End == Other.m_End);
	}

private:
	segment_t(T const Start, T const End) noexcept
		: m_Start{ Start }
		, m_End{ End }
	{
		assert(length() >= 0);
	}

	T m_Start{};
	T m_End{}; // One past last
};

using small_segment = segment_t<short>;
using segment = segment_t<int>;

#endif // SEGMENT_HPP_DC38F7F7_0E1B_486E_8368_2ACE6415D234
