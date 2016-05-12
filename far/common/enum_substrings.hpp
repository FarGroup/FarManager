#ifndef ENUM_SUBSTRINGS_HPP_AD490DED_6C5F_4C74_82ED_F858919C4277
#define ENUM_SUBSTRINGS_HPP_AD490DED_6C5F_4C74_82ED_F858919C4277
#pragma once

/*
enum_substrings.hpp
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

#include "enumerator.hpp"
#include "iterator_range.hpp"

namespace detail
{
	template<class container>
	auto Begin(const container& Container) { return std::begin(Container); }
	template<class char_type>
	auto Begin(const char_type* Container) { return Container; }
	template<class char_type>
	auto Begin(char_type* Container) { return Container; }

	template<class container>
	bool IsEnd(const container& Container, decltype(std::cbegin(Container)) Iterator) { return Iterator == Container.cend(); }
	template<class char_type>
	bool IsEnd(const char_type* Container, const char_type* Iterator) { return Iterator != Container && !*Iterator && !*(Iterator - 1); }
}

// Enumerator for string1\0string2\0string3\0...stringN
// Stops on double \0 or if end of container is reached.

template<class provider>
class enum_substrings_t: public enumerator<enum_substrings_t<provider>, range<decltype(&*detail::Begin(std::declval<std::decay_t<provider>>()))>>
{
public:
	NONCOPYABLE(enum_substrings_t);
	TRIVIALLY_MOVABLE(enum_substrings_t);

	explicit enum_substrings_t(const provider& Provider): m_Provider(&Provider), m_Offset() {}

	bool get(size_t Index, typename enum_substrings_t<provider>::value_type& Value)
	{
		const auto Begin = detail::Begin(*m_Provider) + (Index? m_Offset : 0);
		auto End = Begin;
		bool IsEnd;
		for (; (IsEnd = detail::IsEnd(*m_Provider, End)) == false && *End; ++End)
			;
		if (End != Begin)
		{
			const auto Ptr = &*Begin;
			Value = typename enum_substrings_t<provider>::value_type(Ptr, Ptr + (End - Begin));
			m_Offset += Value.size() + (IsEnd? 0 : 1);
			return true;
		}
		return false;
	}

private:
	const provider* m_Provider;
	size_t m_Offset;
};

template<class T>
auto enum_substrings(T&& Provider)
{
	static_assert(std::is_lvalue_reference<T>::value, "Argument must be lvalue");
	return enum_substrings_t<std::remove_reference_t<T>>(Provider);
}

#endif // ENUM_SUBSTRINGS_HPP_AD490DED_6C5F_4C74_82ED_F858919C4277
