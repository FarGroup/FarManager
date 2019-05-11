#ifndef TVAR_HPP_684F1D24_C3BA_43F6_9099_C617C30385EE
#define TVAR_HPP_684F1D24_C3BA_43F6_9099_C617C30385EE
#pragma once

/*
tvar.hpp

Реализация класса TVar ("кастрированый" вариант - только целое и строковое значение)
(для макросов)

*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

// Internal:

// Platform:

// Common:
#include "common/preprocessor.hpp"

// External:

//----------------------------------------------------------------------------

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

class TVar
{
public:
	enum class Type
	{
		Unknown = 0,
		Integer = 1,
		String = 2,
		Double = 3,
	};

	COPYABLE(TVar);
	MOVABLE(TVar);

	explicit TVar();
	explicit TVar(int);
	explicit TVar(long long);
	explicit TVar(string_view);
	explicit TVar(const wchar_t*);
	explicit TVar(double);

	COPY_AND_MOVE(TVar, long long)
	COPY_AND_MOVE(TVar, string_view)
	COPY_AND_MOVE(TVar, const wchar_t*)
	COPY_AND_MOVE(TVar, int)
	COPY_AND_MOVE(TVar, double)

	bool operator<(const TVar&) const;

	Type type() const { return vType; }
	Type ParseType() const;
	void SetType(Type newType) {vType=newType;}

	bool isString()   const { return vType == Type::String; }
	bool isInteger()  const { return vType == Type::Integer || vType == Type::Unknown; }
	bool isDouble()   const { return vType == Type::Double; }
	bool isUnknown()  const { return vType == Type::Unknown; }

	bool isNumber()   const;

	const string& toString();
	double toDouble();
	long long toInteger();

	const string& asString() const;
	double asDouble() const;
	long long asInteger() const;

private:
	union
	{
		long long inum{};
		double dnum;
	};
	mutable string str;
	Type vType;
};

#endif // TVAR_HPP_684F1D24_C3BA_43F6_9099_C617C30385EE
