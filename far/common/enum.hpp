#ifndef ENUM_HPP_3B45F837_E295_40BC_B3EE_A7D344E8B1ED
#define ENUM_HPP_3B45F837_E295_40BC_B3EE_A7D344E8B1ED
#pragma once

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

#if COMPILER == C_CL && VS_OLDER_THAN(VS_2012)
#define ENUM(ENUM_NAME) enum ENUM_NAME
#else
#define ENUM(ENUM_NAME) enum ENUM_NAME:int
#endif

/*
Usage:

namespace detail
{
	struct data { enum value_type
	{
		foo,
		bar,
	};};
}
typedef enum_class<detail::data> data;
data Data(data::foo);
*/

template<class T>
class enum_class: public T, public rel_ops<enum_class<T>>
{
public:
	enum_class(typename T::value_type Value): m_Value(Value) {}
	enum_class& operator=(typename T::value_type rhs) { m_Value = rhs; return *this; }

	friend bool operator==(const enum_class& lhs, const enum_class& rhs) { return lhs.m_Value == rhs.m_Value; }
	friend bool operator==(const enum_class& lhs, typename T::value_type rhs) { return lhs.m_Value == rhs; }
	friend bool operator==(typename T::value_type lhs, const enum_class& rhs) { return lhs == rhs.m_Value; }

	friend bool operator<(const enum_class& lhs, const enum_class& rhs) { return lhs.m_Value < rhs.m_Value; }
	friend bool operator<(const enum_class& lhs, typename T::value_type rhs) { return lhs.m_Value < rhs; }
	friend bool operator<(typename T::value_type lhs, const enum_class& rhs) { return lhs < rhs.m_Value; }

	typename T::value_type value() const { return m_Value; }

private:
	typename T::value_type m_Value;
};

#endif // ENUM_HPP_3B45F837_E295_40BC_B3EE_A7D344E8B1ED
