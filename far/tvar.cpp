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

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

#include "headers.hpp"
#pragma hdrstop

#include "tvar.hpp"
#include "config.hpp"
#include "strmix.hpp"

//#define TVAR_USE_STRMUN

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
		const wchar_t *ptrTestStr=TestStr.data();
		wchar_t ch, ch2;
		bool isNum     = true;
//		bool isDec     = false;
		bool isBegDec  = false;
//		bool isHex     = false;
		bool isBegHex  = false;
//		bool isOct     = false;
		bool isBegOct  = false;
//		bool isE       = false;
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

					if (ptrTestStr == TestStr.data() + 1)
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
						wchar_t ch3=*ptrTestStr++;

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

static string toString(long long num)
{
	return std::to_wstring(num);
};

static string toString(double num)
{
	return str_printf(Global->Opt->Macro.strMacroCONVFMT.Get().data(), num);
};

static TVar addStr(const string& a, const string& b)
{
	return TVar(a + b);
}

TVar& TVar::AppendStr(const TVar& appStr)
{
	vType = vtString;
	return operator=(addStr(str,appStr.str));
}

TVar& TVar::AppendStr(wchar_t addChr)
{
	wchar_t tmp[2]={};
	tmp[0]=addChr;
	vType = vtString;
	return operator=(addStr(str,tmp));
}

TVar::TVar():
	inum(),
	dnum(),
	vType(vtUnknown)
{
}

TVar::TVar(__int64 v) :
	inum(v),
	dnum(),
	vType(vtInteger)
{
}

TVar::TVar(int v) :
	inum(v),
	dnum(),
	vType(vtInteger)
{
}

TVar::TVar(double v) :
	inum(),
	dnum(v),
	vType(vtDouble)
{
}

TVar::TVar(const string& v) :
	inum(),
	dnum(),
	str(v),
	vType(vtString)
{
}

TVar::TVar(const wchar_t* v):
	inum(),
	dnum(),
	str(NullToEmpty(v)),
	vType(vtString)
{
}

const string& TVar::toString()
{
	switch (vType)
	{
	case vtDouble:
		str = ::toString(dnum);
		break;
	case vtUnknown:
	case vtInteger:
		str = ::toString(inum);
		break;
	case vtString:
		// TODO: log
		break;
	default:
		// TODO: log
		break;
	}

	vType = vtString;
	return str;
}

__int64 TVar::toInteger()
{
	inum = asInteger();
	vType = vtInteger;
	return inum;
}

double TVar::toDouble()
{
	dnum = asDouble();
	vType = vtDouble;
	return dnum;
};

const string& TVar::asString() const
{
	if (!isString())
	{
		str = isInteger() ? ::toString(inum) : ::toString(dnum);
	}
	return str;
}

long long TVar::asInteger() const
{
	long long ret = 0;

	if (isInteger())
	{
		ret = inum;
	}
	else if (isDouble())
	{
		ret = static_cast<long long>(dnum);
	}
	else if (isString())
	{
		try
		{
			ret = std::stoll(str);
		}
		catch (const std::exception&)
		{
			// TODO: log
		}
	}
	else
	{
		// TODO: log
	}
	return ret;
};

double TVar::asDouble() const
{
	double ret = 0;

	if (isDouble())
	{
		ret = dnum;
	}
	else if (isInteger())
	{
		ret = static_cast<double>(inum);
	}
	else if (isString())
	{
		try
		{
			ret = std::stod(str);
		}
		catch (const std::exception&)
		{
			// TODO: log
		}
	}
	else
	{
		// TODO: log
	}

	return ret;
};

bool TVar::isNumber() const
{
	switch (type())
	{
		case vtUnknown:
		case vtInteger:
		case vtDouble:
			return true;

		case vtString:
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

static bool _cmp_Eq(TVarType vt,const void *a, const void *b)
{
	bool r = false;

	switch (vt)
	{
		case vtUnknown:
		case vtInteger: r = *(const __int64*)a == *(const __int64*)b; break;
		case vtDouble:  r = fabs(*(const double*)a - *(const double*)b) < DBL_EPSILON; break;
		case vtString:  r = !StrCmp((const wchar_t*)a, (const wchar_t*)b); break;
	}

	return r;
}

static bool _cmp_Lt(TVarType vt,const void *a, const void *b)
{
	bool r = false;

	switch (vt)
	{
		case vtUnknown:
		case vtInteger: r = *(const __int64*)a < *(const __int64*)b; break;
		case vtDouble:  r = *(const double*)a < *(const double*)b; break;
		case vtString:  r = StrCmp((const wchar_t*)a, (const wchar_t*)b) < 0; break;
	}

	return r;
}

static bool _cmp_Gt(TVarType vt, const void *a, const void *b)
{
	bool r = false;

	switch (vt)
	{
		case vtUnknown:
		case vtInteger: r = *(const __int64*)a > *(const __int64*)b; break;
		case vtDouble:  r = *(const double*)a > *(const double*)b; break;
		case vtString:  r = StrCmp((const wchar_t*)a, (const wchar_t*)b) > 0; break;
	}

	return r;
}

bool CompAB(const TVar& a, const TVar& b, TVarFuncCmp fcmp)
{
	bool r = true;
	__int64 bi;
	double bd;

	switch (a.vType)
	{
		case vtUnknown:
		case vtInteger:

			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = fcmp(vtInteger,&a.inum,&b.inum); break;
				case vtDouble:  r = fcmp(vtDouble,&a.inum,&b.dnum);  break;
				case vtString:
				{
					switch (checkTypeString(b.asString()))
					{
						case tsStr:   r = fcmp(vtString,a.asString().data(),b.str.data()); break;
						case tsInt:   bi=b.asInteger(); r = fcmp(vtInteger,&a.inum,&bi);  break;
						case tsFloat: bd=b.asDouble(); r = fcmp(vtDouble,&a.inum,&bd);   break;
					}

					break;
				}
			}

			break;
		case vtDouble:

			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = fcmp(vtInteger,&a.dnum,&b.inum); break;
				case vtDouble:  r = fcmp(vtDouble,&a.dnum,&b.dnum);  break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = fcmp(vtString,a.asString().data(),b.str.data()); break;
						case tsInt:
						case tsFloat: bd=b.asDouble(); r = fcmp(vtDouble,&a.inum,&bd);   break;
					}

					break;
				}
			}

			break;
		case vtString:
		{
			r = fcmp(vtString,a.asString().data(),b.asString().data());
			break;
		}
	}

	return r;
};

bool operator==(const TVar& a, const TVar& b)
{
	return CompAB(a, b, _cmp_Eq);
}

bool operator<(const TVar& a, const TVar& b)
{
	return CompAB(a, b, _cmp_Lt);
}

bool operator>(const TVar& a, const TVar& b)
{
	return CompAB(a, b, _cmp_Gt);
}

TVar operator+(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtUnknown:
		case vtInteger:
		{
			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = a.inum + b.inum;                   break;
				case vtDouble:  r = (double)a.inum + b.dnum;           break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = addStr(::toString(a.inum), b.asString());   break;
						case tsInt:   r = a.inum + b.asInteger();                  break;
						case tsFloat: r = (double)a.inum + b.asDouble();          break;
					}

					break;
				}
			}

			break;
		}
		case vtDouble:
		{
			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = (double)a.inum + b.dnum;                break;
				case vtDouble:  r = a.dnum + b.dnum;                        break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = addStr(::toString(a.dnum), b.asString());  break;
						case tsInt:
						case tsFloat: r = a.dnum * b.asDouble(); break;
					}

					break;
				}
			}

			break;
		}
		case vtString:
		{
			TypeString tsA=checkTypeString(a.str),tsB;

			if (b.vType == vtInteger || b.vType == vtUnknown)
				tsB=tsInt;
			else if (b.vType == vtDouble)
				tsB=tsFloat;
			else
				tsA=tsB=tsStr;

			if ((tsA == tsStr && tsB == tsStr) || (tsA != tsStr && tsB == tsStr) || (tsA == tsStr && tsB != tsStr))
				r = addStr(a.asString(), b.asString());
			else if (tsA == tsInt && tsB == tsInt)
				r = a.asInteger() + b.asInteger();
			else
				r = a.asDouble() + b.asDouble();

			break;
		}
	}

	return r;
};

TVar operator%(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtUnknown:
		case vtInteger:
		{
			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = b.inum ? (a.inum % b.inum) : 0; break;
				case vtDouble:  r = fabs(b.dnum) > DBL_EPSILON? fmod((double)a.inum, b.dnum) : 0.0; break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						{
							__int64 bi=b.asInteger();
							r = bi ? a.inum % bi : 0; break;
						}
						case tsFloat:
						{
							double bd=b.asDouble();
							r = fabs(bd) > DBL_EPSILON? fmod((double)a.inum, bd) : 0.0; break;
						}
					}

					break;
				}
			}

			break;
		}
		case vtDouble:
		{
			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = b.inum ? fmod(a.dnum,(double)b.inum) : 0.0; break;
				case vtDouble:  r = fabs(b.dnum) > DBL_EPSILON? fmod(a.dnum,b.dnum) : 0.0; break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat:
						{
							double bd=b.asDouble();
							r = fabs(bd) > DBL_EPSILON? fmod(a.dnum, bd) : 0.0; break;
						}
					}

					break;
				}
			}

			break;
		}
		case vtString:
		{
			TypeString tsA=checkTypeString(a.str),tsB;

			if (b.vType == vtInteger || b.vType == vtUnknown)
				tsB=tsInt;
			else if (b.vType == vtDouble)
				tsB=tsFloat;
			else
				tsA=tsB=tsStr;

			if ((tsA == tsStr && tsB == tsStr) || (tsA != tsStr && tsB == tsStr) || (tsA == tsStr && tsB != tsStr))
				r = a;
			else if (tsA == tsInt && tsB == tsInt)
			{
				__int64 bi=b.asInteger();
				r = bi ? (a.asInteger() % bi) : 0;
			}
			else
			{
				double bd=b.asDouble();
				r = fabs(bd) > DBL_EPSILON? fmod(a.asDouble() , bd) : 0.0;
			}

			break;
		}
	}

	return r;
};

TVar TVar::operator-() const
{
	switch (vType)
	{
		case vtUnknown:
		case vtInteger:
			return TVar(-inum);
		case vtDouble:
			return TVar(-dnum);
		default:
			return *this;
	}
}
