/*
tvar.cpp

Реализация класса TVar (для макросов)

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "tvar.hpp"

// Internal:
#include "string_sort.hpp"

// Platform:

// Common:
#include "common.hpp"
#include "common/from_string.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

enum TypeString
{
	tsStr,
	tsInt,
	tsFloat,
};

static TypeString checkTypeString(const string& TestStr)
{
	TypeString typeTestStr=tsStr;

	if (!TestStr.empty())
	{
		auto ptrTestStr = TestStr.c_str();
		wchar_t ch, ch2;
		bool isNum     = true;
		//bool isDec     = false;
		bool isBegDec  = false;
		//bool isHex     = false;
		bool isBegHex  = false;
		//bool isOct     = false;
		bool isBegOct  = false;
		//bool isE       = false;
		bool isExp     = false;
		bool isPoint   = false;
		bool isSign    = false;
		bool isExpSign = false;

		if (*ptrTestStr == L'-' || *ptrTestStr == L'+')
		{
			isSign=true;
			ptrTestStr++;
		}

		if (*ptrTestStr == L'.' && std::iswdigit(ptrTestStr[1]))
		{
			isPoint=true;
			ptrTestStr++;
		}

		if (*ptrTestStr >= L'1' && *ptrTestStr <=L'9')
			isBegDec=true;
		else if (*ptrTestStr == L'0')
		{
			if ((ptrTestStr[1] == L'x' || ptrTestStr[1] == L'X') && std::iswxdigit(ptrTestStr[2]))
			{
				isBegHex=true;
				ptrTestStr+=2;
			}
			else
			{
				if (std::iswdigit(ptrTestStr[1]) || ptrTestStr[1] == L'.')
					isBegDec=true;
				else if (!ptrTestStr[1])
					return tsInt;
				else
					isBegOct=true;
			}
		}

		while ((ch=*ptrTestStr++) != 0)
		{
			switch (ch)
			{
				case L'-':
				case L'+':

					if (ptrTestStr == TestStr.c_str() + 1)
						isSign=true;
					else if (isSign)
					{
						isNum=false;
						break;
					}

					if (isExp)
					{
						if (isExpSign)
						{
							isNum=false;
							break;
						}

						isExpSign=true;
					}

					break;
				case L'.':

					if (isPoint)
					{
						isNum=false;
						break;
					}

					isPoint=true;

					if (!(std::iswdigit(ptrTestStr[1]) || ptrTestStr[1] == L'e' || ptrTestStr[1] == L'E' || !ptrTestStr[1]))
					{
						isNum=false;
						break;
					}

					break;
				case L'e':
				case L'E':
					//isHex=true;
					//isE=true;
					ch2=*ptrTestStr++;

					if (ch2 == L'-' || ch2 == L'+')  // E+D
					{
						if (isBegHex || isExpSign)  // начало hex или уже был знак у порядка?
						{
							isNum=false;
							break;
						}

						isExpSign=true;
						const auto ch3 = *ptrTestStr++;

						if (!std::iswdigit(ch3))   // за знаком идет число?
						{
							isNum=false;
							break;
						}
						else
						{
							isExp=true;
						}
					}
					else if (!std::iswdigit(ch2))   // ED
					{
						if (isBegDec)
						{
							isNum=false;
							break;
						}

						ptrTestStr--;
					}
					else
					{
						isExp=true;
						ptrTestStr--;
					}

					break;
				case L'a': case L'A': case L'b': case L'B': case L'c': case L'C': case L'd': case L'D': case L'f': case L'F':

					if (isBegDec || isExp)
					{
						isNum=false;
						break;
					}

					//isHex=true;
					break;
				case L'0': case L'1': case L'2': case L'3': case L'4': case L'5': case L'6': case L'7':
					//isOct=true;
				case L'8': case L'9':

					if (isBegOct && (ch == L'8' || ch == L'9'))
					{
						isNum=false;
						break;
					}

					//isDec=true;
					break;
				default:
					isNum=false;
			}

			if (!isNum)
				break;
		}

		if (isNum)
		{
			if (isBegDec && (isExp || isPoint))
				typeTestStr=tsFloat;

			if ((isBegDec || isBegHex || isBegOct) && !(isExp || isPoint))
				typeTestStr=tsInt;
		}
	}

	return typeTestStr;
}

TVar::TVar():
	vType(Type::Unknown)
{
}

TVar::TVar(long long v):
	inum(v),
	vType(Type::Integer)
{
}

TVar::TVar(int v):
	TVar(static_cast<long long>(v))
{
}

TVar::TVar(double v):
	dnum(v),
	vType(Type::Double)
{
}

TVar::TVar(string_view const v):
	str(v),
	vType(Type::String)
{
}

TVar::TVar(const wchar_t* v):
	TVar(string_view(NullToEmpty(v)))
{
}

const string& TVar::toString()
{
	// this already writes to str
	(void)asString();
	vType = Type::String;
	return str;
}

long long TVar::toInteger()
{
	inum = asInteger();
	vType = Type::Integer;
	return inum;
}

double TVar::toDouble()
{
	dnum = asDouble();
	vType = Type::Double;
	return dnum;
}

const string& TVar::asString() const
{
	if (!isString())
	{
		// str() is implemented in terms of fmt::to_wstring().
		// For doubles fmt::to_wstring adds ".0" even if there's no fractional part
		// (e.g. 1234.0 to "1234.0"), and it's a feature (see issue #1153).
		// For historical reasons we prefer the shortest possible representation, hence "g".
		str = isInteger()? ::str(inum) : far::format(L"{:.6g}"sv, dnum);
	}
	return str;
}

long long TVar::asInteger() const
{
	switch (vType)
	{
	case Type::Integer:
	case Type::Unknown:
		return inum;

	case Type::Double:
		return dnum;

	case Type::String:
		{
			long long Value;
			return from_string(str, Value)? Value : 0;
		}

	default:
		return 0;
	}
}

double TVar::asDouble() const
{
	switch (vType)
	{
	case Type::Integer:
	case Type::Unknown:
		return inum;

	case Type::Double:
		return dnum;

	case Type::String:
		{
			double Value;
			return from_string(str, Value)? Value : 0;
		}

	default:
		return 0;
	}
}

bool TVar::isNumber() const
{
	switch (type())
	{
		case Type::Unknown:
		case Type::Integer:
		case Type::Double:
			return true;

		case Type::String:
			switch (checkTypeString(str))
			{
				case tsInt:
				case tsFloat:
					return true;
				default:
					return false;
			}
	}

	return false;
}

TVar::Type TVar::ParseType() const
{
	if (vType != Type::String)
		return vType;

	switch(checkTypeString(str))
	{
	case tsInt:
		return Type::Integer;

	case tsFloat:
		return Type::Double;

	default:
		return Type::String;
	}
}

bool TVar::operator<(const TVar& rhs) const
{
	switch (type())
	{
	case Type::Unknown:
	case Type::Integer:
		switch (rhs.type())
		{
		case Type::Unknown:
		case Type::Integer:
			return asInteger() < rhs.asInteger();

		case Type::Double:
			return asDouble() < rhs.asDouble();

		case Type::String:
			switch (checkTypeString(rhs.asString()))
			{
			case tsStr:
				return string_sort::less(asString(), rhs.asString());

			case tsInt:
				return asInteger() < rhs.asInteger();

			case tsFloat:
				return asDouble() < rhs.asDouble();
			}
			break;
		}
		break;

	case Type::Double:
		switch (rhs.type())
		{
		case Type::Unknown:
		case Type::Integer:
		case Type::Double:
			return asDouble() < rhs.asDouble();

		case Type::String:
			switch (checkTypeString(rhs.asString()))
			{
			case tsStr:
				return string_sort::less(asString(), rhs.asString());

			case tsInt:
			case tsFloat:
				return asDouble() < rhs.asDouble();
			}
			break;
		}
		break;

	case Type::String:
		return string_sort::less(asString(), rhs.asString());
	}

	return false;
}
