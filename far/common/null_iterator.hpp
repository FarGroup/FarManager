﻿#ifndef NULL_ITERATOR_HPP_18FC84FA_D7EE_47C4_9979_72EC06E57C37
#define NULL_ITERATOR_HPP_18FC84FA_D7EE_47C4_9979_72EC06E57C37
#pragma once

/*
null_iterator.hpp
*/
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

#include <iterator>

//----------------------------------------------------------------------------

template<class T>
class null_iterator
{
public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using reference = T&;

	explicit null_iterator(T* Data) noexcept: m_Data(Data) {}
	auto& operator++() noexcept { ++m_Data; return *this; }
	auto operator++(int) noexcept { return null_iterator(m_Data++); }

	[[nodiscard]]
	auto& operator*() noexcept { return *m_Data; }

	[[nodiscard]]
	auto operator->() noexcept { return m_Data; }

	[[nodiscard]]
	auto& operator*() const noexcept { return *m_Data; }

	[[nodiscard]]
	auto operator->() const noexcept { return m_Data; }

	[[nodiscard]]
	static const auto& end() noexcept { static T Empty{}; static const null_iterator Iter(&Empty); return Iter; }

	[[nodiscard]]
	bool operator==(const null_iterator& rhs) const noexcept { return m_Data == rhs.m_Data || (rhs.m_Data == end().m_Data && !*m_Data); }

private:
	T* m_Data;
};

#endif // NULL_ITERATOR_HPP_18FC84FA_D7EE_47C4_9979_72EC06E57C37
