/*
local.cpp

Сравнение без учета регистра, преобразование регистра
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "headers.hpp"
#pragma hdrstop

#include "local.hpp"

const wchar_t * __cdecl StrStrI(const wchar_t *str1, const wchar_t *str2)
{
	wchar_t *cp = (wchar_t *) str1;
	wchar_t *s1, *s2;

	if ( !*str2 )
		return str1;

	while ( *cp )
	{
		s1 = cp;
		s2 = (wchar_t *) str2;

		while ( *s1 && *s2 && !(Lower(*s1)-Lower(*s2)) )
		{
			s1++;
			s2++;
		}

		if ( !*s2 )
			return (const wchar_t *)cp;

		cp++;
	}

	return (const wchar_t *)NULL;
}


const wchar_t * __cdecl RevStrStrI(const wchar_t *str1, const wchar_t *str2)
{
	int len1 = StrLength(str1);
	int len2 = StrLength(str2);

	if (len2 > len1)
		return (const wchar_t *)NULL;

	if ( !*str2 )
		return &str1[len1];

	wchar_t *cp = (wchar_t *)&str1[len1-len2];
	wchar_t *s1, *s2;

	while ( cp >= str1 )
	{
		s1 = cp;
		s2 = (wchar_t *) str2;

		while ( *s1 && *s2 && !(Lower(*s1)-Lower(*s2)) )
		{
			s1++;
			s2++;
		}

		if ( !*s2 )
			return (const wchar_t *)cp;

		cp--;
	}

	return (const wchar_t *)NULL;
}


wchar_t __cdecl Upper(wchar_t Ch)
{
    wchar_t Buf = Ch;

		CharUpperBuff(&Buf, 1);

    return Buf;
}


wchar_t __cdecl Lower(wchar_t Ch)
{
    wchar_t Buf = Ch;

		CharLowerBuff(&Buf, 1);

    return Buf;
}

int __cdecl StrCmpNI(const wchar_t *s1, const wchar_t *s2, int n)
{
	return CompareString(
			0,
			NORM_IGNORECASE,
			s1,
			n,
			s2,
			n
			)-2;
}

int __cdecl StrCmpI(const wchar_t *s1, const wchar_t *s2)
{
	return CompareString(
			0,
			NORM_IGNORECASE,
			s1,
			-1,
			s2,
			-1
			)-2;

}

int __cdecl StrCmpN(const wchar_t *s1, const wchar_t *s2, int n)
{
	return CompareString(
			0,
			0,
			s1,
			n,
			s2,
			n
			)-2;
}

int __cdecl StrCmp(const wchar_t *s1, const wchar_t *s2)
{
	return CompareString(
			0,
			0,
			s1,
			-1,
			s2,
			-1
			)-2;

}

int __digit_cnt_0(const wchar_t* s, const wchar_t ** beg)
{
  int n = 0;
  while(*s == L'0') s++;
  *beg = s;
  while(iswdigit(*s)) { s++; n++; }
  return n;
}

int __cdecl NumStrCmpI(const wchar_t *s1, const wchar_t *s2)
{
	int ret;

	while ( *s1 && *s2 )
	{
		if ( iswdigit(*s1) && iswdigit(*s2) )
		{
			// берем длину числа без ведущих нулей
			int dig_len1 = __digit_cnt_0(s1, &s1);
			int dig_len2 = __digit_cnt_0(s2, &s2);
			// если одно длиннее другого, значит они и больше! :)

			if(dig_len1 != dig_len2)
				return dig_len1 - dig_len2;

			// длины одинаковы, сопоставляем...
			while ( iswdigit(*s1) && iswdigit(*s2) )
			{
				ret = StrCmpNI(s1,s2,1);

				if ( ret )
					return ret;

				s1++; s2++;
			}

            if ( *s1 == 0 )
            	break;
		}

		ret = StrCmpNI(s1,s2,1);

		if ( ret )
			return ret;

		s1++; s2++;
	}

    return StrCmpI(s1,s2);
}

int __cdecl NumStrCmp(const wchar_t *s1, const wchar_t *s2)
{
	int ret;

	while ( *s1 && *s2 )
	{
		if ( iswdigit(*s1) && iswdigit(*s2) )
		{
			// берем длину числа без ведущих нулей
			int dig_len1 = __digit_cnt_0(s1, &s1);
			int dig_len2 = __digit_cnt_0(s2, &s2);
			// если одно длиннее другого, значит они и больше! :)

			if(dig_len1 != dig_len2)
				return dig_len1 - dig_len2;

			// длины одинаковы, сопоставляем...
			while ( iswdigit(*s1) && iswdigit(*s2) )
			{
				ret = StrCmpN(s1,s2,1);

				if ( ret )
					return ret;

				s1++; s2++;
			}

            if ( *s1 == 0 )
            	break;
		}

		ret = StrCmpN(s1,s2,1);

		if ( ret )
			return ret;

		s1++; s2++;
	}

    return StrCmp(s1,s2);
}


int __cdecl IsUpper(wchar_t Ch)
{
		return IsCharUpper(Ch);
}

int __cdecl IsLower(wchar_t Ch)
{
		return IsCharLower(Ch);
}

int __cdecl IsAlpha(wchar_t Ch)
{
		return IsCharAlpha(Ch);
}

int __cdecl IsAlphaNum(wchar_t Ch)
{
		return IsCharAlphaNumeric(Ch);
}


void __cdecl UpperBuf(wchar_t *Buf, int Length)
{
		CharUpperBuff(Buf, Length);
}


void __cdecl LowerBuf(wchar_t *Buf,int Length)
{
		CharLowerBuff(Buf, Length);
}

void __cdecl StrUpper(wchar_t *s1)
{
    UpperBuf(s1, StrLength(s1));
}


void __cdecl StrLower(wchar_t *s1)
{
    LowerBuf(s1, StrLength(s1));
}
