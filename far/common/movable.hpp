#ifndef MOVABLE_HPP_A063CBC7_C7FC_470D_901E_620E0D6A2D51
#define MOVABLE_HPP_A063CBC7_C7FC_470D_901E_620E0D6A2D51
#pragma once

/*
movable.hpp

Helper class to set variable to default value after move,
e. g. to use default move ctor/operator=, but skip any custom logic in dtor dependent on this variable

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

//----------------------------------------------------------------------------

template<typename T, T Default = T{}>
class movable
{
public:
	NONCOPYABLE(movable);

	movable() = default;
	movable(T Value): m_Value(Value){}
	auto& operator=(T Value) { m_Value = Value; return *this; }
	auto& operator+=(T Value) { m_Value += Value; return *this; }
	auto& operator-=(T Value) { m_Value -= Value; return *this; }
	auto& operator++() { ++m_Value; return *this; }
	auto& operator--() { --m_Value; return *this; }

	movable(movable&& rhs) noexcept { *this = std::move(rhs); }
	auto& operator=(movable&& rhs) noexcept { m_Value = std::exchange(rhs.m_Value, Default); return *this; }

	[[nodiscard]]
	bool operator==(T Value) const { return m_Value == Value; }

	[[nodiscard]]
	bool operator<(T Value) const { return m_Value < Value; }

	[[nodiscard]]
	operator T() const { return m_Value; }

private:
	T m_Value{Default};
};

#endif // MOVABLE_HPP_A063CBC7_C7FC_470D_901E_620E0D6A2D51
