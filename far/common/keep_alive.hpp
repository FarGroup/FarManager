#ifndef KEEP_ALIVE_HPP_9C3E665F_56D5_4A21_9950_F1F8F6BFC7A3
#define KEEP_ALIVE_HPP_9C3E665F_56D5_4A21_9950_F1F8F6BFC7A3
#pragma once

/*
keep_alive.hpp
*/
/*
Copyright © 2018 Far Group
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

template<typename arg_type>
using keep_alive_type =
	std::enable_if_t<std::is_reference_v<arg_type>,
		std::conditional_t<
			std::is_rvalue_reference_v<arg_type>,
			std::remove_reference_t<arg_type>,
			arg_type
		>
	>;

template<typename type>
class [[nodiscard]] keep_alive
{
public:
	explicit keep_alive(type&& Value):
		m_Value(FWD(Value))
	{}

	[[nodiscard]]
	operator const type&() const noexcept { return m_Value; }

	[[nodiscard]]
	auto operator&() const noexcept { return &m_Value; }

	[[nodiscard]]
	auto& get() const noexcept { return m_Value; }

private:
	type m_Value;
};

template<typename type>
keep_alive(type&& Value) -> keep_alive<keep_alive_type<decltype(Value)>>;

#endif // KEEP_ALIVE_HPP_9C3E665F_56D5_4A21_9950_F1F8F6BFC7A3
