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

#include "global.hpp"
#include "fn.hpp"

static unsigned char KeyToKey[256];

/*
   Инициализация массива клавиш.
   Вызывать только после CopyGlobalSettings, потому что только тогда GetRegKey
   считает правильные данные.
*/
void InitKeysArray()
{
  GetRegKey(L"Interface",L"HotkeyRules",Opt.HotkeyRules,1);
  unsigned char CvtStr[2];
  int I;
  CvtStr[1]=0;
  HKL Layout[10];

  int LayoutNumber=GetKeyboardLayoutList(sizeof(Layout)/sizeof(Layout[0]),Layout);

  if (LayoutNumber<5)
  {
    if(!Opt.HotkeyRules)
    {
      for (I=0;I<=255;I++)
      {
        int Keys[10];
        memset(Keys,0,sizeof(Keys));
        for (int J=0; J < LayoutNumber; J++)
        {
          int AnsiKey=MapVirtualKeyEx(I,2,Layout[J]) & 0xff;
          if (AnsiKey==0)
            continue;
          CvtStr[0]=AnsiKey;
          CvtStr[1]=0;
          FAR_CharToOem((char *)CvtStr,(char *)CvtStr);
          Keys[J]=CvtStr[0];
        }
        if (Keys[0]!=0 && Keys[1]!=0)
        {
          KeyToKey[LocalLower(Keys[0])]=LocalUpper(Keys[1]);
          KeyToKey[LocalUpper(Keys[0])]=LocalUpper(Keys[1]);
          KeyToKey[LocalLower(Keys[1])]=LocalUpper(Keys[0]);
          KeyToKey[LocalUpper(Keys[1])]=LocalUpper(Keys[0]);
        }
      }
    }
    else
    {
      CvtStr[1]=0;
      for (I=0;I<=255;I++)
      {
        for (int J=0;J<LayoutNumber;J++)
        {
          SHORT AnsiKey=VkKeyScanEx(I,Layout[J])&0xFF;
          if (AnsiKey==0xFF)
            continue;
          CvtStr[0]=I;
          FAR_CharToOem((char *)CvtStr,(char *)CvtStr);
          KeyToKey[CvtStr[0]]=static_cast<unsigned char>(AnsiKey);
        }
      }
    }
  }
  //_SVS(SysLogDump("KeyToKey calculate",0,KeyToKey,sizeof(KeyToKey),NULL));
  unsigned char KeyToKeyMap[256];
  if(GetRegKey(L"System",L"KeyToKeyMap",KeyToKeyMap,KeyToKey,sizeof(KeyToKeyMap)))
    memcpy(KeyToKey,KeyToKeyMap,sizeof(KeyToKey));
  //_SVS(SysLogDump("KeyToKey readed",0,KeyToKey,sizeof(KeyToKey),NULL));
}

int WINAPI LocalIslower(unsigned Ch)
{
	if (Ch>=256)
		return FALSE;

	char s[2];
	s[0] = Ch; s[1] = 0;
	string strS(s,CP_OEMCP);
	return IsLower(strS.At(0)); //BUGBUG - юникодный символ может быть шире чем один элемент
}

int WINAPI LocalIsupper(unsigned Ch)
{
	if (Ch>=256)
		return FALSE;

	char s[2];
	s[0] = Ch; s[1] = 0;
	string strS(s,CP_OEMCP);
	return IsUpper(strS.At(0)); //BUGBUG - юникодный символ может быть шире чем один элемент
}



int WINAPI LocalIsalpha(unsigned Ch)
{
	if (Ch>=256)
		return FALSE;

	char s[2];
	s[0] = Ch; s[1] = 0;
	string strS(s,CP_OEMCP);
	return IsAlpha(strS.At(0)); //BUGBUG - юникодный символ может быть шире чем один элемент
}


int WINAPI LocalIsalphanum(unsigned Ch)
{
	if (Ch>=256)
		return FALSE;

	char s[2];
	s[0] = Ch; s[1] = 0;
	string strS(s,CP_OEMCP);
	return IsAlphaNum(strS.At(0)); //BUGBUG - юникодный символ может быть шире чем один элемент
}



unsigned WINAPI LocalUpper(unsigned LowerChar)
{
	if (LowerChar>=256)
		return LowerChar;

	char s[2];
	s[0] = LowerChar; s[1] = 0;
	string strS(s,CP_OEMCP);
	strS.Upper();
	UnicodeToAnsi(strS,s,sizeof(s));
	return s[0];
}


void WINAPI LocalUpperBuf(char *Buf,int Length)
{
  for (int I=0;I<Length;I++)
    Buf[I]=LocalUpper(Buf[I]);
}

void WINAPI LocalLowerBuf(char *Buf,int Length)
{
  for (int I=0;I<Length;I++)
    Buf[I]=LocalLower(Buf[I]);
}


unsigned WINAPI LocalLower(unsigned UpperChar)
{
	if (UpperChar>=256)
		return UpperChar;

	char s[2];
	s[0] = UpperChar; s[1] = 0;
	string strS(s,CP_OEMCP);
	strS.Lower();
	UnicodeToAnsi(strS,s,sizeof(s));
	return s[0];
}


void WINAPI LocalStrupr(char *s1)
{
	string strS(s1,CP_OEMCP);
	strS.Upper();
	UnicodeToAnsi(strS,s1,(int)strlen(s1)+1);
}


void WINAPI LocalStrlwr(char *s1)
{
	string strS(s1,CP_OEMCP);
	strS.Lower();
	UnicodeToAnsi(strS,s1,(int)strlen(s1)+1);
}

const char * __cdecl LocalStrstri(const char *str1, const char *str2)
{
  char *cp = (char *) str1;
  char *s1, *s2;

  if ( !*str2 )
      return str1;

  while (*cp)
  {
    s1 = cp;
    s2 = (char *) str2;

    while ( *s1 && *s2 && !(LocalLower(*s1) - LocalLower(*s2)) )
    {
      s1++;
      s2++;
    }

    if (!*s2)
      return (const char *)cp;

    cp++;
  }

  return (const char *)NULL;
}

const char * __cdecl LocalRevStrstri(const char *str1, const char *str2)
{
  int len1 = (int)strlen(str1);
  int len2 = (int)strlen(str2);

  if (len2 > len1)
    return (const char *)NULL;

  if ( !*str2 )
    return &str1[len1];

  char *cp = (char *)&str1[len1 - len2];
  char *s1, *s2;

  while (cp >= str1)
  {
    s1 = cp;
    s2 = (char *) str2;

    while ( *s1 && *s2 && !(LocalLower(*s1) - LocalLower(*s2)) )
    {
      s1++;
      s2++;
    }

    if (!*s2)
      return (const char *)cp;

    cp--;
  }

  return (const char *)NULL;
}

int __cdecl LocalStricmp(const char *s1,const char *s2)
{
	string strS1(s1,CP_OEMCP);
	string strS2(s2,CP_OEMCP);
	return StrCmpI(strS1,strS2);
}

int __cdecl LocalStrnicmp(const char *s1,const char *s2,int n)
{
  while (n-- > 0)
  {
    if (LocalLower(*s1) != LocalLower(*s2))
      return (LocalLower(*s1) < LocalLower(*s2)) ? -1 : 1;
    if (*(s1++) == 0)
      break;
    s2++;
  }
  return(0);
}

int WINAPI LStricmp(const char *s1,const char *s2)
{
	return LocalStricmp(s1,s2);
}

int WINAPI LStrnicmp(const char *s1,const char *s2,int n)
{
  return LocalStrnicmp(s1,s2,n);
}

int LocalKeyToKey(int Key)
{
  return(KeyToKey[Key]);
}

/*---------------------------------------*/
int __cdecl StrLength(const wchar_t *str)
{
	return lstrlenW(str);
}


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

    CharUpperBuffW (&Buf, 1);

    return Buf;
}


wchar_t __cdecl Lower(wchar_t Ch)
{
    wchar_t Buf = Ch;

    CharLowerBuffW (&Buf, 1);

    return Buf;
}

int __cdecl StrCmpNI(const wchar_t *s1, const wchar_t *s2, int n)
{
	return CompareStringW (
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
	return CompareStringW (
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
	return CompareStringW (
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
	return CompareStringW (
			0,
			0,
			s1,
			-1,
			s2,
			-1
			)-2;

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
    return IsCharUpperW(Ch);
}

int __cdecl IsLower(wchar_t Ch)
{
    return IsCharLowerW(Ch);
}

int __cdecl IsAlpha(wchar_t Ch)
{
    return IsCharAlphaW(Ch);
}

int __cdecl IsAlphaNum(wchar_t Ch)
{
    return IsCharAlphaNumericW(Ch);
}


void __cdecl UpperBuf(wchar_t *Buf, int Length)
{
    CharUpperBuffW(Buf, Length);
}


void __cdecl LowerBuf(wchar_t *Buf,int Length)
{
    CharLowerBuffW(Buf, Length);
}

void __cdecl StrUpper(wchar_t *s1)
{
    UpperBuf(s1, StrLength(s1));
}


void __cdecl StrLower(wchar_t *s1)
{
    LowerBuf(s1, StrLength(s1));
}
