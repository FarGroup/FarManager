/*
localOEM.cpp

—равнение без учета регистра, преобразование регистра дл€ OEM кодировки
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

#include "headers.hpp"
#pragma hdrstop

#include "localOEM.hpp"
#include "syslog.hpp"
#include "config.hpp"
#include "configdb.hpp"

static int _cdecl LCSort(const void *el1,const void *el2);

static unsigned char LowerToUpper[256];
static unsigned char UpperToLower[256];
static unsigned char IsUpperOrLower[256];
static unsigned char LCOrder[256];

void LocalUpperInit()
{
	for (unsigned int I=0; I<ARRAYSIZE(LowerToUpper); I++)
	{
		char CvtStr[]={I,L'\0'},ReverseCvtStr[2];
		LowerToUpper[I]=UpperToLower[I]=I;
		OemToCharA(CvtStr,CvtStr);
		CharToOemA(CvtStr,ReverseCvtStr);
		IsUpperOrLower[I]=0;

		if (IsCharAlphaA(CvtStr[0]) && ReverseCvtStr[0]==static_cast<char>(I))
		{
			IsUpperOrLower[I]=IsCharLowerA(CvtStr[0])?1:(IsCharUpperA(CvtStr[0])?2:0);
			CharUpperA(CvtStr);
			CharToOemA(CvtStr,CvtStr);
			LowerToUpper[I]=CvtStr[0];
			CvtStr[0]=I;
			OemToCharA(CvtStr,CvtStr);
			CharLowerA(CvtStr);
			CharToOemA(CvtStr,CvtStr);
			UpperToLower[I]=CvtStr[0];
		}
	}
}

/*
   »нициализаци€ системозависимой сортировки строк.
   ¬ызывать только после CopyGlobalSettings (потому что только тогда GetRegKey
   считает правильные данные) и перед InitKeysArray (потому что там уже
   используетс€ сортировка)!
*/
void InitLCIDSort()
{
	unsigned char LCSortBuffer[256];

	for (size_t i=0; i<ARRAYSIZE(LCSortBuffer); i++)
	{
		LCSortBuffer[i]=static_cast<BYTE>(i);
	}

	Opt.LCIDSort=GeneralCfg->GetValue(L"System",L"LCID",LOCALE_USER_DEFAULT);
	far_qsort(LCSortBuffer,256,sizeof(LCSortBuffer[0]),LCSort);

	for (size_t i=0; i<ARRAYSIZE(LCSortBuffer); i++)
	{
		LCOrder[LCSortBuffer[i]]=static_cast<BYTE>(i);
	}

	LCOrder[0]=0;
	LCOrder[(unsigned)'\\']=1;
	LCOrder[(unsigned)'.']=2;

	for (size_t i=0; i<ARRAYSIZE(LCSortBuffer)-1; i++)
	{
		if (!LCSort(&LCSortBuffer[i],&LCSortBuffer[i+1]))
		{
			LCOrder[LCSortBuffer[i+1]]=LCOrder[LCSortBuffer[i]];
		}
	}

	for (size_t i=0; i<ARRAYSIZE(LCOrder); i++)
	{
		LCOrder[i]=LCOrder[UpperToLower[i]];
	}
}

int WINAPI LocalIslower(unsigned Ch)
{
	return(Ch<256 && IsUpperOrLower[Ch]==1);
}

int WINAPI LocalIsupper(unsigned Ch)
{
	return(Ch<256 && IsUpperOrLower[Ch]==2);
}

int WINAPI LocalIsalpha(unsigned Ch)
{
	if (Ch>=256)
		return FALSE;

	char CvtCh=Ch;
	OemToCharBuffA(&CvtCh,&CvtCh,1);
	return(IsCharAlphaA(CvtCh));
}

int WINAPI LocalIsalphanum(unsigned Ch)
{
	if (Ch>=256)
		return FALSE;

	char CvtCh=Ch;
	OemToCharBuffA(&CvtCh,&CvtCh,1);
	return(IsCharAlphaNumericA(CvtCh));
}

unsigned WINAPI LocalUpper(unsigned LowerChar)
{
	return(LowerChar < 256 ? LowerToUpper[LowerChar]:LowerChar);
}

void WINAPI LocalUpperBuf(char *Buf,int Length)
{
	for (int I=0; I<Length; I++)
		Buf[I]=LocalUpper(Buf[I]);
}

void WINAPI LocalLowerBuf(char *Buf,int Length)
{
	for (int I=0; I<Length; I++)
		Buf[I]=LocalLower(Buf[I]);
}

unsigned WINAPI LocalLower(unsigned UpperChar)
{
	return(UpperChar < 256 ? UpperToLower[UpperChar]:UpperChar);
}

void WINAPI LocalStrupr(char *s1)
{
	while (*s1)
	{
		*s1=LowerToUpper[(unsigned)*s1];
		s1++;
	}
}

void WINAPI LocalStrlwr(char *s1)
{
	while (*s1)
	{
		*s1=UpperToLower[(unsigned)*s1];
		s1++;
	}
}

const char * __cdecl LocalStrstri(const char *str1, const char *str2)
{
	const char *cp = str1;
	const char *s1, *s2;

	if (!*str2)
		return str1;

	while (*cp)
	{
		s1 = cp;
		s2 = str2;

		while (*s1 && *s2 && !(LocalLower(*s1) - LocalLower(*s2)))
		{
			s1++;
			s2++;
		}

		if (!*s2)
			return cp;

		cp++;
	}

	return nullptr;
}

const char * __cdecl LocalRevStrstri(const char *str1, const char *str2)
{
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);

	if (len2 > len1)
		return nullptr;

	if (!*str2)
		return &str1[len1];

	const char *cp = &str1[len1 - len2];
	const char *s1, *s2;

	while (cp >= str1)
	{
		s1 = cp;
		s2 = str2;

		while (*s1 && *s2 && !(LocalLower(*s1) - LocalLower(*s2)))
		{
			s1++;
			s2++;
		}

		if (!*s2)
			return cp;

		cp--;
	}

	return nullptr;
}

int __cdecl LocalStricmp(const char *s1,const char *s2)
{
	while (1)
	{
		if (UpperToLower[(unsigned)*s1] != UpperToLower[(unsigned)*s2])
			return (UpperToLower[(unsigned)*s1] < UpperToLower[(unsigned)*s2]) ? -1 : 1;

		if (!*(s1++))
			break;

		s2++;
	}

	return 0;
}

int __cdecl LocalStrnicmp(const char *s1,const char *s2,int n)
{
	while (n-- > 0)
	{
		if (UpperToLower[(unsigned)*s1] != UpperToLower[(unsigned)*s2])
			return (UpperToLower[(unsigned)*s1] < UpperToLower[(unsigned)*s2]) ? -1 : 1;

		if (!*(s1++))
			break;

		s2++;
	}

	return 0;
}

int WINAPI LStricmp(const char *s1,const char *s2)
{
	return LocalStricmp(s1,s2);
}

int WINAPI LStrnicmp(const char *s1,const char *s2,int n)
{
	return LocalStrnicmp(s1,s2,n);
}

int _cdecl LCSort(const void *el1,const void *el2)
{
	char Str1[]={*static_cast<const char*>(el1),L'\0'},
		Str2[]={*static_cast<const char*>(el2),L'\0'};
	OemToCharBuffA(Str1,Str1,1);
	OemToCharBuffA(Str2,Str2,1);
	return(CompareStringA(Opt.LCIDSort,NORM_IGNORENONSPACE|SORT_STRINGSORT|NORM_IGNORECASE,Str1,1,Str2,1)-2);
}
