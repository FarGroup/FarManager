#ifndef SPAN_HPP_3B87674F_96D1_487D_B83E_43E43EFBA4D3
#define SPAN_HPP_3B87674F_96D1_487D_B83E_43E43EFBA4D3
#pragma once

/*
span.hpp
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

#include <span>

//----------------------------------------------------------------------------

template<class span_value_type>
class [[nodiscard]] span: public std::span<span_value_type>
{
	using base_span = std::span<span_value_type>;
	// Can't use base_span alias here, Clang isn't smart enough.
	using std::span<span_value_type>::span;

public:
	template<std::ranges::contiguous_range SpanLike> requires requires { base_span(std::declval<SpanLike&>()); }
	constexpr explicit(false) span(SpanLike&& Span) noexcept:
		base_span(Span)
	{
	}

	// Design by committee
	constexpr span(const std::initializer_list<span_value_type>& List) noexcept:
		base_span(List)
	{
	}
};

template<std::contiguous_iterator Iterator>
span(Iterator Begin, size_t Size) -> span<std::remove_reference_t<decltype(*Begin)>>;

template<std::ranges::contiguous_range container>
span(container&& c) -> span<std::remove_reference_t<decltype(*std::begin(c))>>;

template<typename value_type>
span(const std::initializer_list<value_type>&) -> span<const value_type>;

#endif // SPAN_HPP_3B87674F_96D1_487D_B83E_43E43EFBA4D3
