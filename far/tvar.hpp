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

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

enum TVarType
{
	vtUnknown = 0,
	vtInteger = 1,
	vtString  = 2,
	vtDouble  = 3,
};

using TVarFuncCmp = bool (*)(TVarType vt, const void*, const void*);

class TVar
{
public:
	COPYABLE(TVar);
	MOVABLE(TVar);

	explicit TVar();
	explicit TVar(int);
	explicit TVar(long long);
	explicit TVar(const string&);
	explicit TVar(const wchar_t*);
	explicit TVar(double);

	COPY_AND_MOVE(long long);
	COPY_AND_MOVE(const string&);
	COPY_AND_MOVE(const wchar_t*);
	COPY_AND_MOVE(int);
	COPY_AND_MOVE(double);

	bool operator<(const TVar&) const;
	bool operator>(const TVar&) const;
	bool operator==(const TVar&) const;

	TVar& operator+=(const TVar& b) { return *this = *this + b; }
	TVar operator-() const;

	TVar operator+(const TVar&) const;
	TVar operator%(const TVar&) const;

	TVar& AppendStr(wchar_t);
	TVar& AppendStr(const TVar&);

	TVarType type() const { return vType; }
	void SetType(TVarType newType) {vType=newType;}

	bool isString()   const { return vType == vtString;  }
	bool isInteger()  const { return vType == vtInteger || vType == vtUnknown; }
	bool isDouble()   const { return vType == vtDouble;  }
	bool isUnknown()  const { return vType == vtUnknown;  }

	bool isNumber()   const;

	const string& toString();
	double toDouble();
	long long toInteger();

	const string& asString() const;
	double asDouble() const;
	long long asInteger() const;

private:
	friend bool CompAB(const TVar& a, const TVar& b, TVarFuncCmp fcmp);

	long long inum;
	double dnum;
	mutable string str;
	TVarType vType;
};

#endif // TVAR_HPP_684F1D24_C3BA_43F6_9099_C617C30385EE
