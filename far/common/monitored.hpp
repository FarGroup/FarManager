#ifndef MONITORED_HPP_2F3A1061_FB63_4B9F_8EB9_9DA8C7B7CF22
#define MONITORED_HPP_2F3A1061_FB63_4B9F_8EB9_9DA8C7B7CF22
#pragma once

/*
monitored.hpp
*/
/*
Copyright © 2015 Far Group
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

template<class T>
class monitored
{
public:
	MOVABLE(monitored);

	monitored(): m_Value(), m_Touched() {}
	monitored(const T& Value): m_Value(Value), m_Touched() {}
	monitored(const monitored& rhs): m_Value(rhs.m_Value), m_Touched() {}

	monitored(T&& Value) noexcept: m_Value(std::move(Value)), m_Touched() {}

	auto& operator=(const T& Value) { m_Value = Value; m_Touched = true; return *this; }
	auto& operator=(const monitored& rhs) { m_Value = rhs.m_Value; m_Touched = true; return *this; }

	auto& operator=(T&& Value) noexcept { m_Value = std::move(Value); m_Touched = true; return *this; }

	[[nodiscard]]
	auto& value() noexcept { return m_Value; }

	[[nodiscard]]
	const auto& value() const noexcept { return m_Value; }

	[[nodiscard]]
	operator T&() noexcept { return m_Value; }

	[[nodiscard]]
	operator const T&() const noexcept { return m_Value; }

	[[nodiscard]]
	auto touched() const noexcept { return m_Touched; }

	void forget() noexcept { m_Touched = false; }

private:
	T m_Value;
	bool m_Touched;
};

#endif // MONITORED_HPP_2F3A1061_FB63_4B9F_8EB9_9DA8C7B7CF22
