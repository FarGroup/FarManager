/*
local.cpp

Сравнение без учета регистра, преобразование регистра

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"

static unsigned char KeyToKey[256];

/* $ 11.01.2002 IS
   Инициализация массива клавиш.
   Вызывать только после CopyGlobalSettings, потому что только тогда GetRegKey
   считает правильные данные.
*/
void InitKeysArray()
{
  GetRegKeyW(L"Interface",L"HotkeyRules",Opt.HotkeyRules,1);
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
  if(GetRegKeyW(L"System",L"KeyToKeyMap",KeyToKeyMap,KeyToKey,sizeof(KeyToKeyMap)))
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
	return LocalIslowerW(strS.At(0)); //BUGBUG - юникодный символ может быть шире чем один элемент
}


int WINAPI LocalIsupper(unsigned Ch)
{
	if (Ch>=256)
		return FALSE;

	char s[2];
	s[0] = Ch; s[1] = 0;
	string strS(s,CP_OEMCP);
	return LocalIsupperW(strS.At(0)); //BUGBUG - юникодный символ может быть шире чем один элемент
}

int WINAPI LocalIsalpha(unsigned Ch)
{
	if (Ch>=256)
		return FALSE;

	char s[2];
	s[0] = Ch; s[1] = 0;
	string strS(s,CP_OEMCP);
	return IsCharAlphaW(strS.At(0)); //BUGBUG - юникодный символ может быть шире чем один элемент
}

int WINAPI LocalIsalphanum(unsigned Ch)
{
	if (Ch>=256)
		return FALSE;

	char s[2];
	s[0] = Ch; s[1] = 0;
	string strS(s,CP_OEMCP);
	return IsCharAlphaNumericW(strS.At(0)); //BUGBUG - юникодный символ может быть шире чем один элемент
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
	return LocalStricmpW(strS1,strS2);
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


const wchar_t * __cdecl StrstriW(const wchar_t *str1, const wchar_t *str2)
{
  wchar_t *cp = (wchar_t *) str1;
  wchar_t *s1, *s2;

  if ( !*str2 )
      return str1;

  while (*cp)
  {
    s1 = cp;
    s2 = (wchar_t *) str2;

    while ( *s1 && *s2 && !(LocalLowerW(*s1) - LocalLowerW(*s2)) )
    {
      s1++;
      s2++;
    }

    if (!*s2)
      return (const wchar_t *)cp;

    cp++;
  }

  return (const wchar_t *)NULL;
}

const wchar_t * __cdecl RevStrstriW(const wchar_t *str1, const wchar_t *str2)
{
  int len1 = (int)wcslen(str1);
  int len2 = (int)wcslen(str2);

  if (len2 > len1)
    return (const wchar_t *)NULL;

  if ( !*str2 )
    return &str1[len1];

  wchar_t *cp = (wchar_t *)&str1[len1 - len2];
  wchar_t *s1, *s2;

  while (cp >= str1)
  {
    s1 = cp;
    s2 = (wchar_t *) str2;

    while ( *s1 && *s2 && !(LocalLowerW(*s1) - LocalLowerW(*s2)) )
    {
      s1++;
      s2++;
    }

    if (!*s2)
      return (const wchar_t *)cp;

    cp--;
  }

  return (const wchar_t *)NULL;
}


wchar_t WINAPI LocalUpperW (wchar_t Ch)
{
    wchar_t Buf = Ch;

    CharUpperBuffW (&Buf, 1);

    return Buf;
}

wchar_t WINAPI LocalLowerW (wchar_t Ch)
{
    wchar_t Buf = Ch;

    CharLowerBuffW (&Buf, 1);

    return Buf;
}

int WINAPI LocalStrnicmpW (const wchar_t *s1, const wchar_t *s2, int n)
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

int WINAPI LocalStricmpW (const wchar_t *s1, const wchar_t *s2)
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

int WINAPI LocalStrncmpW (const wchar_t *s1, const wchar_t *s2, int n)
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

int WINAPI LocalStrcmpW (const wchar_t *s1, const wchar_t *s2)
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

int __cdecl LocalNumStricmpW (const wchar_t *s1, const wchar_t *s2)
{
  int ret;
  while(*s1 && *s2)
  {
    if(iswdigit(*s1) && iswdigit(*s2))
    {
       // берем длину числа без ведущих нулей
       int dig_len1 = __digit_cnt_0(s1, &s1);
       int dig_len2 = __digit_cnt_0(s2, &s2);
       // если одно длиннее другого, значит они и больше! :)
       if(dig_len1 != dig_len2)
         return dig_len1 - dig_len2;
       // длины одинаковы, сопоставляем...
       while(iswdigit(*s1) && iswdigit(*s2))
       {
         ret = LocalStrnicmpW(s1,s2,1);
         if (ret)
           return ret;
         s1++; s2++;
       }
       if(*s1 == 0)
         break;
    }
    ret = LocalStrnicmpW(s1,s2,1);
    if (ret)
      return ret;
    s1++; s2++;
  }
  return LocalStricmpW(s1,s2);
}

int __cdecl LocalNumStrcmpW (const wchar_t *s1, const wchar_t *s2)
{
  int ret;
  while(*s1 && *s2)
  {
    if(iswdigit(*s1) && iswdigit(*s2))
    {
       // берем длину числа без ведущих нулей
       int dig_len1 = __digit_cnt_0(s1, &s1);
       int dig_len2 = __digit_cnt_0(s2, &s2);
       // если одно длиннее другого, значит они и больше! :)
       if(dig_len1 != dig_len2)
         return dig_len1 - dig_len2;
       // длины одинаковы, сопоставляем...
       while(iswdigit(*s1) && iswdigit(*s2))
       {
         ret = LocalStrncmpW(s1,s2,1);
         if (ret)
           return ret;
         s1++; s2++;
       }
       if(*s1 == 0)
         break;
    }
    ret = LocalStrncmpW(s1,s2,1);
    if (ret)
      return ret;
    s1++; s2++;
  }
  return LocalStrcmpW(s1,s2);
}

int WINAPI LocalIsupperW (wchar_t Ch)
{
    return IsCharUpperW (Ch);
}

int WINAPI LocalIslowerW (wchar_t Ch)
{
    return IsCharLowerW (Ch);
}

int WINAPI LocalIsalphaW (wchar_t Ch)
{
    return IsCharAlphaW (Ch);
}

int WINAPI LocalIsalphanumW (wchar_t Ch)
{
    return IsCharAlphaNumericW (Ch);
}


void WINAPI LocalUpperBufW(wchar_t *Buf, int Length)
{
    CharUpperBuffW (Buf, Length);
}


void WINAPI LocalLowerBufW(wchar_t *Buf,int Length)
{
    CharLowerBuffW (Buf, Length);
}

void WINAPI LocalStruprW(wchar_t *s1)
{
    LocalUpperBufW (s1, (int)wcslen (s1));
}


void WINAPI LocalStrlwrW(wchar_t *s1)
{
    LocalLowerBufW (s1, (int)wcslen (s1));
}
