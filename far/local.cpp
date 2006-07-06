/*
local.cpp

��������� ��� ����� ��������, �������������� ��������

*/

/* Revision: 1.30 07.07.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"

static int _cdecl LCSort(const void *el1,const void *el2);

unsigned char LowerToUpper[256];
unsigned char UpperToLower[256];
unsigned char IsUpperOrLower[256];
static unsigned char LCOrder[256];
static unsigned char KeyToKey[256];

void LocalUpperInit()
{
  unsigned char CvtStr[2],ReverseCvtStr[2];
  int I;

  CvtStr[1]=0;

  for (I=0;I<sizeof(LowerToUpper)/sizeof(LowerToUpper[0]);I++)
  {
    CvtStr[0]=I;
    LowerToUpper[I]=UpperToLower[I]=I;
    FAR_OemToChar((char *)CvtStr,(char *)CvtStr);
    FAR_CharToOem((char *)CvtStr,(char *)ReverseCvtStr);
    IsUpperOrLower[I]=0;
    if (IsCharAlpha(CvtStr[0]) && ReverseCvtStr[0]==I)
    {
      IsUpperOrLower[I]=IsCharLower(CvtStr[0])?1:(IsCharUpper(CvtStr[0])?2:0);
      CharUpper((char *)CvtStr);
      FAR_CharToOem((char *)CvtStr,(char *)CvtStr);
      LowerToUpper[I]=CvtStr[0];
      CvtStr[0]=I;
      FAR_OemToChar((char *)CvtStr,(char *)CvtStr);
      CharLower((char *)CvtStr);
      FAR_CharToOem((char *)CvtStr,(char *)CvtStr);
      UpperToLower[I]=CvtStr[0];
    }
  }
}

/* $ 26.03.2002 IS
   ������������� ���������������� ���������� �����.
   �������� ������ ����� CopyGlobalSettings (������ ��� ������ ����� GetRegKey
   ������� ���������� ������) � ����� InitKeysArray (������ ��� ��� ���
   ������������ ����������)!
*/
void InitLCIDSort()
{
  char LCSortBuffer[256];
  int I;

  for (I=0;I<sizeof(LCSortBuffer)/sizeof(LCSortBuffer[0]);I++)
    LCSortBuffer[I]=I;

  Opt.LCIDSort=GetRegKeyW(L"System",L"LCID",LOCALE_USER_DEFAULT);
  far_qsort((void *)LCSortBuffer,256,sizeof(LCSortBuffer[0]),LCSort);

  for (I=0;I<sizeof(LCSortBuffer)/sizeof(LCSortBuffer[0]);I++)
    LCOrder[LCSortBuffer[I]]=I;

  LCOrder[0]=0;
  LCOrder['\\']=1;
  LCOrder['.']=2;

  for (I=0;I<sizeof(LCSortBuffer)/sizeof(LCSortBuffer[0])-1;I++)
    if (LCSort(&LCSortBuffer[I],&LCSortBuffer[I+1])==0)
      LCOrder[LCSortBuffer[I+1]]=LCOrder[LCSortBuffer[I]];

  for (I=0;I<sizeof(LCOrder)/sizeof(LCOrder[0]);I++)
    LCOrder[I]=LCOrder[UpperToLower[I]];

/*  for (I=0;I<sizeof(KeyToKey)/sizeof(KeyToKey[0]);I++)
    KeyToKey[I]=I;
*/
}
/* IS $ */

/* $ 11.01.2002 IS
   ������������� ������� ������.
   �������� ������ ����� CopyGlobalSettings, ������ ��� ������ ����� GetRegKey
   ������� ���������� ������.
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
  return(Ch<256 && IsUpperOrLower[Ch]==1);
}


int WINAPI LocalIsupper(unsigned Ch)
{
  return(Ch<256 && IsUpperOrLower[Ch]==2);
}

int WINAPI LocalIsalpha(unsigned Ch)
{
  if (Ch>=256)
    return(FALSE);

  unsigned char CvtStr[1];
  CvtStr[0]=Ch;
  FAR_OemToCharBuff((char *)CvtStr,(char *)CvtStr,1);
  return(IsCharAlpha(CvtStr[0]));
}

int WINAPI LocalIsalphanum(unsigned Ch)
{
  if (Ch>=256)
    return(FALSE);

  unsigned char CvtStr[1];
  CvtStr[0]=Ch;
  FAR_OemToCharBuff((char *)CvtStr,(char *)CvtStr,1);
  return(IsCharAlphaNumeric(CvtStr[0]));
}


unsigned WINAPI LocalUpper(unsigned LowerChar)
{
  return(LowerChar < 256 ? LowerToUpper[LowerChar]:LowerChar);
}


void WINAPI LocalUpperBuf(char *Buf,int Length)
{
  for (int I=0;I<Length;I++)
    Buf[I]=LowerToUpper[Buf[I]];
}

/* $ 28.08.2000 SVS
   ��������� ������� LocalLowerBuf
*/
void WINAPI LocalLowerBuf(char *Buf,int Length)
{
  for (int I=0;I<Length;I++)
    Buf[I]=UpperToLower[Buf[I]];
}
/* SVS $ */

unsigned WINAPI LocalLower(unsigned UpperChar)
{
  return(UpperChar < 256 ? UpperToLower[UpperChar]:UpperChar);
}

/* $ 27.11.2001 DJ
   ��������� ��������������� :-)
*/

void WINAPI LocalStrupr(char *s1)
{
  while (*s1)
  {
    *s1=LowerToUpper[*s1];
    s1++;
  }
}


void WINAPI LocalStrlwr(char *s1)
{
  while (*s1)
  {
  *s1=UpperToLower[*s1];
  s1++;
  }
}

/* DJ $ */


int WINAPI LStricmp(const char *s1,const char *s2)
{
  return LocalStricmp(s1,s2);
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

    while ( *s1 && *s2 && !(UpperToLower[*s1] - UpperToLower[*s2]) )
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
  int len1 = strlen(str1);
  int len2 = strlen(str2);

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

    while ( *s1 && *s2 && !(UpperToLower[*s1] - UpperToLower[*s2]) )
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
  while (1)
  {
    if (UpperToLower[*s1] != UpperToLower[*s2])
      return (UpperToLower[*s1] < UpperToLower[*s2]) ? -1 : 1;
    if (*(s1++) == 0)
      break;
    s2++;
  }
  return(0);
}

int WINAPI LStrnicmp(const char *s1,const char *s2,int n)
{
  return LocalStrnicmp(s1,s2,n);
}

int __cdecl LocalStrnicmp(const char *s1,const char *s2,int n)
{
  while (n-- > 0)
  {
    if (UpperToLower[*s1] != UpperToLower[*s2])
      return (UpperToLower[*s1] < UpperToLower[*s2]) ? -1 : 1;
    if (*(s1++) == 0)
      break;
    s2++;
  }
  return(0);
}


int __cdecl LCStricmp(const char *s1,const char *s2)
{
  while (1)
  {
    if (LCOrder[*s1] != LCOrder[*s2])
      return (LCOrder[*s1] < LCOrder[*s2]) ? -1 : 1;
    if (*(s1++) == 0)
      break;
    s2++;
  }
  return(0);
}

int __cdecl LCNumStricmp(const char *s1,const char *s2)
{
  const char *ts1 = s1, *ts2 = s2;
  while(*s1 && *s2)
  {
    if(isdigit(*s1) && isdigit(*s2))
    {
       // ����� ����� ����� ��� ������� �����
       int dig_len1 = __digit_cnt_0(s1, &s1);
       int dig_len2 = __digit_cnt_0(s2, &s2);
       // ���� ���� ������� �������, ������ ��� � ������! :)
       if(dig_len1 != dig_len2)
         return dig_len1 - dig_len2;
       // ����� ���������, ������������...
       while(isdigit(*s1) && isdigit(*s2))
       {
         if (LCOrder[*s1] != LCOrder[*s2])
           return (LCOrder[*s1] < LCOrder[*s2]) ? -1 : 1;
         s1++; s2++;
       }
       if(*s1 == 0)
         break;
   }
    if (LCOrder[*s1] != LCOrder[*s2])
      return (LCOrder[*s1] < LCOrder[*s2]) ? -1 : 1;
    s1++; s2++;
  }
  int Ret=LCOrder[*s1] - LCOrder[*s2];
  if(!Ret)
    return strlen(ts1)-strlen(ts2);
  return (Ret < 0) ? -1 : 1;
}

int _cdecl LCSort(const void *el1,const void *el2)
{
  char Str1[3],Str2[3];
  Str1[0]=*(char *)el1;
  Str2[0]=*(char *)el2;
  Str1[1]=Str2[1]=0;
  Str1[2]=Str2[2]=0;
  FAR_OemToCharBuff(Str1,Str1,1);
  FAR_OemToCharBuff(Str2,Str2,1);
  return(CompareString(Opt.LCIDSort,NORM_IGNORENONSPACE|SORT_STRINGSORT|NORM_IGNORECASE,Str1,1,Str2,1)-2);
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
  int len1 = wcslen(str1);
  int len2 = wcslen(str2);

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
    LocalUpperBufW (s1, wcslen (s1));
}


void WINAPI LocalStrlwrW(wchar_t *s1)
{
    LocalLowerBufW (s1, wcslen (s1));
}
