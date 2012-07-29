/*
tvar.cpp

–еализаци€ класса TVar (дл€ макросов)

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

#ifdef FAR_LUA_TEMP
#else
//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

#include "headers.hpp"
#pragma hdrstop

#include "tvar.hpp"
#include "config.hpp"

//#define TVAR_USE_STRMUN

enum TypeString
{
	tsStr,
	tsInt,
	tsFloat,
};

static TypeString checkTypeString(const wchar_t *TestStr)
{
	TypeString typeTestStr=tsStr;

	if (TestStr && *TestStr)
	{
		const wchar_t *ptrTestStr=TestStr;
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

		if (*ptrTestStr == L'.' && iswdigit(ptrTestStr[1]))
		{
			isPoint=true;
			ptrTestStr++;
		}

		if (*ptrTestStr >= L'1' && *ptrTestStr <=L'9')
			isBegDec=true;
		else if (*ptrTestStr == L'0')
		{
			if ((ptrTestStr[1] == L'x' || ptrTestStr[1] == L'X') && iswxdigit(ptrTestStr[2]))
			{
				isBegHex=true;
				ptrTestStr+=2;
			}
			else
			{
				if (iswdigit(ptrTestStr[1]) || ptrTestStr[1] == L'.')
					isBegDec=true;
				else if (!ptrTestStr[1])
					return tsInt;
				else
					isBegOct=true;
			}
		}

		while ((ch=*ptrTestStr++) )
		{
			switch (ch)
			{
				case L'-':
				case L'+':

					if (ptrTestStr == TestStr+1)
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

					if (!(iswdigit(ptrTestStr[1]) || ptrTestStr[1] == L'e' || ptrTestStr[1] == L'E' || !ptrTestStr[1]))
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
						if (isBegHex || isExpSign)  // начало hex или уже был знак у пор€дка?
						{
							isNum=false;
							break;
						}

						isExpSign=true;
						wchar_t ch3=*ptrTestStr++;

						if (!iswdigit(ch3))   // за знаком идет число?
						{
							isNum=false;
							break;
						}
						else
						{
							isExp=true;
						}
					}
					else if (!iswdigit(ch2))   // ED
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

static const wchar_t *toString(__int64 num)
{
	static wchar_t str[128];
	_i64tow(num, str, 10);
	return str;
};

static const wchar_t *toString(double num)
{
	static wchar_t str[256];
	_snwprintf(str, ARRAYSIZE(str)-1, Opt.Macro.strMacroCONVFMT.Get().CPtr(), num);
	return str;
};

static wchar_t *dubstr(const wchar_t *s)
{
	wchar_t *newStr=nullptr;

	if (s)
	{
		newStr = new wchar_t[StrLength(s)+1];

		if (newStr)
			wcscpy(newStr, s);
	}

	return newStr;
}

static TVar addStr(const wchar_t *a, const wchar_t *b)
{
	TVar r(L"");
	wchar_t *c = new wchar_t[StrLength(a ? a : L"")+StrLength(b ? b : L"")+1];

	if (c)
	{
		r = wcscat(wcscpy(c, a ? a : L""), b ? b : L"");
		delete [] c;
	}

	return r;
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

TVar::~TVar()
{
	if (str)
		delete [] str;
}

TVar::TVar(__int64 v) :
	inum(v),
	dnum(0.0),
	str(nullptr),
	vType(vtInteger)
{
}

TVar::TVar(int v) :
	inum(v),
	dnum(0.0),
	str(nullptr),
	vType(vtInteger)
{
}

TVar::TVar(double v) :
	inum(0),
	dnum(v),
	str(nullptr),
	vType(vtDouble)
{
}

TVar::TVar(const wchar_t *v) :
	inum(0),
	dnum(0.0),
	str(dubstr(v)),
	vType(vtString)
{
}

TVar::TVar(const TVar& v) :
	inum(v.inum),
	dnum(v.dnum),
	str(dubstr(v.str)),
	vType(v.vType)
{
}

TVar& TVar::operator=(const TVar& v)
{
	if (this != &v)
	{
		vType = v.vType;
		inum = v.inum;
		dnum = v.dnum;

		if (str)
			delete [] str;

		str=dubstr(v.str);
	}

	return *this;
}

__int64 TVar::i() const
{
	return isInteger() ? inum : (isDouble() ? (__int64)dnum : (str ? _wtoi64(str) : 0));
}

double TVar::d() const
{
	wchar_t *endptr;
	return isDouble() ? dnum : (isInteger() ? (double)inum : (str ? wcstod(str,&endptr) : 0));
}

// вернуть ссылку
const wchar_t *TVar::s() const
{
	if (isString())
		return  str ? str : L"";

	return isInteger()? (::toString(inum)) : (::toString(dnum));
}

// вернуть ссылку и преобразовать значени€ "переменной" к типу vtString
const wchar_t *TVar::toString()
{
	wchar_t s[256];

	switch (vType)
	{
		case vtDouble:
			xwcsncpy(s, ::toString(dnum),ARRAYSIZE(s));
			break;
		case vtUnknown:
		case vtInteger:
			xwcsncpy(s, ::toString(inum),ARRAYSIZE(s));
			break;
		default:
			if (!str)
			{
				s[0]=0;
				break;
			}
			return str;
	}

	if (str)
		delete [] str;

	str = dubstr(s);
	vType = vtString;
	return str;
}

__int64 TVar::toInteger()
{
	if (vType == vtString)
		inum = str ? _wtoi64(str) : 0;
	else if (vType == vtDouble)
		inum=(__int64)dnum;

	vType = vtInteger;
	return inum;
}

double TVar::toDouble()
{
	if (isString())
	{
		wchar_t *endptr;
		dnum = str ? wcstod(str,&endptr) : 0;
	}
	else if (isInteger())
		dnum=(double)inum;

	vType = vtDouble;
	return dnum;
};

__int64 TVar::getInteger() const
{
	__int64 ret = inum;

	if (isString())
		ret = str ? _wtoi64(str) : 0;
	else if (isDouble())
		ret=(__int64)dnum;

	return ret;
};

double TVar::getDouble() const
{
	double ret = dnum;

	if (isString())
	{
		wchar_t *endptr;
		ret = str ? wcstod(str,&endptr) : 0;
	}
	else if (isInteger())
		ret=(double)inum;

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

static int _cmp_Ne(TVarType vt,const void *a, const void *b)
{
	int r = 1;

	switch (vt)
	{
		case vtUnknown:
		case vtInteger: r = *(__int64*)a != *(__int64*)b?1:0; break;
		case vtDouble:  r = fabs(*(double*)a - *(double*)b) > DBL_EPSILON? 1 : 0; break;
		case vtString:  r = StrCmp((const wchar_t*)a, (const wchar_t*)b) ; break;
	}

	return r;
}

static int _cmp_Eq(TVarType vt,const void *a, const void *b)
{
	int r = 0;

	switch (vt)
	{
		case vtUnknown:
		case vtInteger: r = *(__int64*)a == *(__int64*)b?1:0; break;
		case vtDouble:  r = fabs(*(double*)a - *(double*)b) < DBL_EPSILON? 1 : 0; break;
		case vtString:  r = !StrCmp((const wchar_t*)a, (const wchar_t*)b); break;
	}

	return r;
}
static int _cmp_Lt(TVarType vt,const void *a, const void *b)
{
	int r = 0;

	switch (vt)
	{
		case vtUnknown:
		case vtInteger: r = *(__int64*)a < *(__int64*)b?1:0; break;
		case vtDouble:  r = *(double*)a < *(double*)b?1:0; break;
		case vtString:  r = StrCmp((const wchar_t*)a, (const wchar_t*)b) < 0; break;
	}

	return r;
}

static int _cmp_Le(TVarType vt,const void *a, const void *b)
{
	int r = 0;

	switch (vt)
	{
		case vtUnknown:
		case vtInteger: r = *(__int64*)a <= *(__int64*)b?1:0; break;
		case vtDouble:  r = *(double*)a <= *(double*)b?1:0; break;
		case vtString:  r = StrCmp((const wchar_t*)a, (const wchar_t*)b) <= 0; break;
	}

	return r;
}

static int _cmp_Gt(TVarType vt,const void *a, const void *b)
{
	int r = 0;

	switch (vt)
	{
		case vtUnknown:
		case vtInteger: r = *(__int64*)a > *(__int64*)b?1:0; break;
		case vtDouble:  r = *(double*)a > *(double*)b?1:0; break;
		case vtString:  r = StrCmp((const wchar_t*)a, (const wchar_t*)b) > 0; break;
	}

	return r;
}

static int _cmp_Ge(TVarType vt,const void *a, const void *b)
{
	int r = 0;

	switch (vt)
	{
		case vtUnknown:
		case vtInteger: r = *(__int64*)a >= *(__int64*)b?1:0; break;
		case vtDouble:  r = *(double*)a >= *(double*)b?1:0; break;
		case vtString:  r = StrCmp((const wchar_t*)a, (const wchar_t*)b) >= 0; break;
	}

	return r;
}

int TVar::CompAB(const TVar& a, const TVar& b, TVarFuncCmp fcmp)
{
	int r = 1;
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
					switch (checkTypeString(b.s()))
					{
						case tsStr:             r = fcmp(vtString,a.s(),b.str); break;
						case tsInt:   bi=b.i(); r = fcmp(vtInteger,&a.inum,&bi);  break;
						case tsFloat: bd=b.d(); r = fcmp(vtDouble,&a.inum,&bd);   break;
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
						case tsStr:             r = fcmp(vtString,a.s(),b.str); break;
						case tsInt:
						case tsFloat: bd=b.d(); r = fcmp(vtDouble,&a.inum,&bd);   break;
					}

					break;
				}
			}

			break;
		case vtString:
		{
#if defined(TVAR_USE_STRMUN)
			__int64 bi;
			double bd;
			TypeString tsA=checkTypeString(a.str), tsB;

			if (b.vType == vtInteger || b.vType == vtUnknown)
				tsB=tsInt;
			else if (b.vType == vtDouble)
				tsB=tsFloat;
			else
				tsB=checkTypeString(b.str);

			if ((tsA == tsStr && tsB == tsStr) || (tsA != tsStr && tsB == tsStr) || (tsA == tsStr && tsB != tsStr))
				r = fcmp(vtString,a.s(),b.s());
			else if (tsA == tsInt && tsB == tsInt)
			{
				ai=a.i();
				bi=b.i();
				r = fcmp(vtInteger,&ai,&bi);
			}
			else
			{
				ad=a.d();
				bd=b.d();
				r = fcmp(vtDouble,&ad,&bd);
			}

#else
			r = fcmp(vtString,a.s(),b.s());
#endif
			break;
		}
	}

	return r;
};

int operator!=(const TVar& a, const TVar& b)
{
	return TVar::CompAB(a,b,(TVarFuncCmp)_cmp_Ne);
}

int operator==(const TVar& a, const TVar& b)
{
	return TVar::CompAB(a,b,(TVarFuncCmp)_cmp_Eq);
}

int operator<(const TVar& a, const TVar& b)
{
	return TVar::CompAB(a,b,(TVarFuncCmp)_cmp_Lt);
}

int operator<=(const TVar& a, const TVar& b)
{
	return TVar::CompAB(a,b,(TVarFuncCmp)_cmp_Le);
}

int operator>(const TVar& a, const TVar& b)
{
	return TVar::CompAB(a,b,(TVarFuncCmp)_cmp_Gt);
}

int operator>=(const TVar& a, const TVar& b)
{
	return TVar::CompAB(a,b,(TVarFuncCmp)_cmp_Ge);
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
						case tsStr:   r = addStr(::toString(a.inum), b.s());   break;
						case tsInt:   r = a.inum + b.i();                  break;
						case tsFloat: r = (double)a.inum + b.d();          break;
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
						case tsStr:   r = addStr(::toString(a.dnum), b.s());  break;
						case tsInt:
						case tsFloat: r = a.dnum * b.d(); break;
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
				r = addStr(a.s(), b.s());
			else if (tsA == tsInt && tsB == tsInt)
				r = a.i() + b.i();
			else
				r = a.d() + b.d();

			break;
		}
	}

	return r;
};

TVar operator-(const TVar& a, const TVar& b)
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
				case vtInteger: r = a.inum - b.inum;                   break;
				case vtDouble:  r = (double)a.inum - b.dnum;           break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a; break; //addStr(::toString(a.inum), b.s());   break;
						case tsInt:   r = a.inum - b.i();                  break;
						case tsFloat: r = (double)a.inum - b.d();          break;
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
				case vtInteger: r = (double)a.inum - b.dnum;                break;
				case vtDouble:  r = a.dnum - b.dnum;                        break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a; break; //addStr(::toString(a.dnum), b.s());  break;
						case tsInt:
						case tsFloat: r = a.dnum - b.d(); break;
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
				r = a.i() - b.i();
			else
				r = a.d() - b.d();

			break;
		}
	}

	return r;
}

TVar operator*(const TVar& a, const TVar& b)
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
				case vtInteger: r = a.inum * b.inum; break;
				case vtDouble:  r = (double)a.inum * b.dnum; break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:   r = a.inum * b.i(); break;
						case tsFloat: r = (double)a.inum * b.d(); break;
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
				case vtInteger: r = a.dnum * (double)b.inum;          break;
				case vtDouble:  r = a.dnum * b.dnum;                  break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat: r = a.dnum * b.d(); break;
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
				r = a.i() * b.i();
			else
				r = a.d() * b.d();

			break;
		}
	}

	return r;
}

TVar operator/(const TVar& a, const TVar& b)
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
				case vtInteger: r = b.inum ? (a.inum / b.inum) : 0; break;
				case vtDouble:  r = fabs(b.dnum) > DBL_EPSILON? ((double)a.inum / b.dnum) : 0.0; break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						{
							__int64 bi=b.i();
							r = bi ? a.inum / bi : 0; break;
						}
						case tsFloat:
						{
							double bd=b.d();
							r = fabs(bd) > DBL_EPSILON? (double)a.inum / bd : 0.0; break;
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
				case vtInteger: r = b.inum ? (a.dnum / (double)b.inum) : 0.0; break;
				case vtDouble:  r = fabs(b.dnum) > DBL_EPSILON? (a.dnum / b.dnum) : 0.0; break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat:
						{
							double bd=b.d();
							r = fabs(bd) > DBL_EPSILON? a.dnum / bd : 0.0; break;
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
				__int64 bi=b.i();
				r = bi ? (a.i() / bi) : 0;
			}
			else
			{
				double bd=b.d();
				r = fabs(bd) > DBL_EPSILON? (a.d() / bd) : 0.0;
			}

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
							__int64 bi=b.i();
							r = bi ? a.inum % bi : 0; break;
						}
						case tsFloat:
						{
							double bd=b.d();
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
							double bd=b.d();
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
				__int64 bi=b.i();
				r = bi ? (a.i() % bi) : 0;
			}
			else
			{
				double bd=b.d();
				r = fabs(bd) > DBL_EPSILON? fmod(a.d() , bd) : 0.0;
			}

			break;
		}
	}

	return r;
};

TVar operator|(const TVar& a, const TVar& b)
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
				case vtInteger: r = a.inum | b.inum;                  break;
				case vtDouble:  r = a.inum | b.i();                   break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat: r = a.inum | b.i(); break;
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
				case vtInteger: r = a.i() | b.inum;                   break;
				case vtDouble:  r = a.i() | b.i();                    break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat: r = a.i() | b.i(); break;
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
			else
				r = a.i() | b.i();

			break;
		}
	}

	return r;
};

TVar operator&(const TVar& a, const TVar& b)
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
				case vtInteger: r = a.inum & b.inum;                  break;
				case vtDouble:  r = a.inum & b.i();                   break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat: r = a.inum & b.i(); break;
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
				case vtInteger: r = a.i() & b.inum;                   break;
				case vtDouble:  r = a.i() & b.i();                    break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat: r = a.i() & b.i(); break;
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
			else
				r = a.i() & b.i();

			break;
		}
	}

	return r;
};

TVar operator^(const TVar& a, const TVar& b)
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
				case vtInteger: r = a.inum ^ b.inum;                  break;
				case vtDouble:  r = a.inum ^ b.i();                   break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat: r = a.inum ^ b.i(); break;
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
				case vtInteger: r = a.i() ^ b.inum;                   break;
				case vtDouble:  r = a.i() ^ b.i();                    break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat: r = a.i() ^ b.i(); break;
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
			else
				r = a.i() ^ b.i();

			break;
		}
	}

	return r;
};

TVar operator>>(const TVar& a, const TVar& b)
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
				case vtInteger: r = a.inum >> b.inum;                  break;
				case vtDouble:  r = a.inum >> b.i();                   break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat: r = a.inum >> b.i(); break;
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
				case vtInteger: r = a.i() >> b.inum;                   break;
				case vtDouble:  r = a.i() >> b.i();                    break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat: r = a.i() >> b.i(); break;
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
			else
				r = a.i() >> b.i();

			break;
		}
	}

	return r;
}

TVar operator<<(const TVar& a, const TVar& b)
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
				case vtInteger: r = a.inum << b.inum;                  break;
				case vtDouble:  r = a.inum << b.i();                   break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat: r = a.inum << b.i(); break;
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
				case vtInteger: r = a.i() << b.inum;                   break;
				case vtDouble:  r = a.i() << b.i();                    break;
				case vtString:
				{
					switch (checkTypeString(b.str))
					{
						case tsStr:   r = a;     break;
						case tsInt:
						case tsFloat: r = a.i() << b.i(); break;
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
			else
				r = a.i() << b.i();

			break;
		}
	}

	return r;
}

TVar operator||(const TVar& a, const TVar& b)
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
				case vtInteger: r = a.inum || b.inum?1:0; break;
				case vtDouble:  r = a.inum || b.dnum?1:0; break;
				case vtString:  r = a.inum || *b.str?1:0; break;
			}

			break;
		}
		case vtDouble:
		{
			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = a.dnum || b.inum?1:0; break;
				case vtDouble:  r = a.dnum || b.dnum?1:0; break;
				case vtString:  r = a.dnum || *b.str?1:0; break;
			}

			break;
		}
		case vtString:
		{
			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = *a.str || b.inum?1:0;  break;
				case vtDouble:  r = *a.str || b.dnum?1:0;   break;
				case vtString:  r = *a.str || *b.str?1:0; break;
			}

			break;
		}
	}

	return r;
};

TVar operator&&(const TVar& a, const TVar& b)
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
				case vtInteger: r = a.inum && b.inum?1:0; break;
				case vtDouble:  r = a.inum && b.dnum?1:0; break;
				case vtString:  r = a.inum && *b.str?1:0; break;
			}

			break;
		}
		case vtDouble:
		{
			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = a.dnum && b.inum?1:0; break;
				case vtDouble:  r = a.dnum && b.dnum?1:0; break;
				case vtString:  r = a.dnum && *b.str?1:0; break;
			}

			break;
		}
		case vtString:
		{
			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = *a.str && b.inum?1:0;  break;
				case vtDouble:  r = *a.str && b.dnum?1:0;   break;
				case vtString:  r = *a.str && *b.str?1:0; break;
			}

			break;
		}
	}

	return r;
};

TVar xor_op(const TVar& a, const TVar& b)
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
				case vtInteger: r = (a.inum || b.inum) && (! (a.inum && b.inum))?1:0; break;
				case vtDouble:  r = (a.inum || b.dnum) && (! (a.inum && b.dnum))?1:0; break;
				case vtString:  r = (a.inum || *b.str) && (! (a.inum && *b.str))?1:0; break;
			}

			break;
		}
		case vtDouble:
		{
			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = (a.dnum || b.inum) && (! (a.dnum && b.inum))?1:0; break;
				case vtDouble:  r = (a.dnum || b.dnum) && (! (a.dnum && b.dnum))?1:0; break;
				case vtString:  r = (a.dnum || *b.str) && (! (a.dnum && *b.str))?1:0; break;
			}

			break;
		}
		case vtString:
		{
			switch (b.vType)
			{
				case vtUnknown:
				case vtInteger: r = (*a.str || b.inum) && (! (*a.str && b.inum))?1:0; break;
				case vtDouble:  r = (*a.str || b.dnum) && (! (*a.str && b.dnum))?1:0; break;
				case vtString:  r = (*a.str || *b.str) && (! (*a.str && *b.str))?1:0; break;
			}

			break;
		}
	}

	return r;
}

TVar TVar::operator+()
{
	return *this;
}

TVar TVar::operator-()
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

TVar TVar::operator!()
{
	switch (vType)
	{
		case vtUnknown:
		case vtInteger:
			return TVar((__int64)!inum);
		case vtDouble:
			return TVar((double)!dnum);
		case vtString:
		default:
			return *this;
	}
}

TVar TVar::operator~()
{
	switch (vType)
	{
		case vtUnknown:
		case vtInteger:
			return TVar(~inum);
		case vtDouble:
			return TVar(~((__int64)dnum));
		case vtString:
		default:
			return *this;
	}
}

TVar& TVar::operator ++()     // ++a
{
	if (vType != vtString)
	{
		if (vType==vtDouble)
			operator+=(1.0);
		else
			operator+=(1);
	}
	return *this;
}

TVar TVar::operator ++(int)  // a++
{
	TVar tmp(*this);
	operator++();
	return tmp;
}

TVar& TVar::operator --()     // --a
{
	if (vType != vtString)
	{
		if (vType==vtDouble)
			operator-=(1.0);
		else
			operator-=(1);
	}
	return *this;
}

TVar TVar::operator --(int)  // a--
{
	TVar tmp(*this);
	operator--();
	return tmp;
}

//---------------------------------------------------------------
// –абота с таблицами имен переменных
//---------------------------------------------------------------
int hash(const wchar_t *p)
{
	int i = 0;

	while (*p)
		i = i*5 + *(p++);

	if (i < 0)
		i = -i;

	i %= V_TABLE_SIZE;
	return i;
}

int isVar(TVarTable table, const wchar_t *p)
{
	int i = hash(p);

	for (TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next))
		if (!StrCmpI(n->str, p))
			return 1;

	return 0;
}

TVarSet *varLook(TVarTable table, const wchar_t *p, bool ins)
{
	int i = hash(p);

	for (TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next))
		if (!StrCmpI(n->str, p))
			return n;

	if (ins)
	{
		TVarSet *nn = new TVarSet(p);
		nn->next = table[i];
		table[i] = nn;
		return nn;
	}

	return nullptr;
}

TVarSet *varEnum(TVarTable table,int Index)
{
	bool found=false;
	TVarSet *n=nullptr, *nfound=nullptr;
	int Idx=0;
	for (int I=0; I < V_TABLE_SIZE; I++)
	{
		n = table[I];
		if (n)
		{
			for (int J=0;; ++J)
			{
				if (Index == Idx++)
				{
					nfound=n;
					found=true;
					break;
				}

				if (!(n = ((TVarSet*)n->next)))
					break;

			}
		}
		if (found)
			break;
	}

	return nfound;
}

void varKill(TVarTable table, const wchar_t *p)
{
	int i = hash(p);
	TVarSet *nn = table[i];

	for (TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next))
	{
		if (!StrCmpI(n->str, p))
		{
			if (n == table[i])
				table[i]=((TVarSet*)n->next);
			else
				nn->next= n->next;

			//( ( n == table[i] ) ? table[i] : nn->next ) = n->next;
			delete n;
			return;
		}

		nn = n;
	}
}

void initVTable(TVarTable table)
{
	for (int i = 0 ; i < V_TABLE_SIZE ; i++)
		table[i] = nullptr;
}

void deleteVTable(TVarTable table)
{
	for (int i = 0 ; i < V_TABLE_SIZE ; i++)
	{
		while (table[i] )
		{
			TVarSet *n = ((TVarSet*)(table[i]->next));
			table[i]->next = nullptr;
			delete table[i];
			table[i] = n;
		}
	}
}
#endif
