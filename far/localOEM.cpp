/*
localOEM.cpp

Сравнение без учета регистра, преобразование регистра для OEM кодировки
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
#include "farwinapi.hpp"
#include "syslog.hpp"
#include "registry.hpp"

static int _cdecl LCSort(const void *el1,const void *el2);

unsigned char LowerToUpper[256];
unsigned char UpperToLower[256];
unsigned char IsUpperOrLower[256];
static unsigned char LCOrder[256];
static unsigned char KeyToKey[256];

void LocalUpperInit()
{
  unsigned char CvtStr[2],ReverseCvtStr[2];
  CvtStr[1]=0;

  for (unsigned int I=0;I<countof(LowerToUpper);I++)
  {
    CvtStr[0]=I;
    LowerToUpper[I]=UpperToLower[I]=I;
    OemToCharA((char *)CvtStr,(char *)CvtStr);
    CharToOemA((char *)CvtStr,(char *)ReverseCvtStr);
    IsUpperOrLower[I]=0;
    if (IsCharAlphaA(CvtStr[0]) && ReverseCvtStr[0]==I)
    {
      IsUpperOrLower[I]=IsCharLowerA(CvtStr[0])?1:(IsCharUpperA(CvtStr[0])?2:0);
      CharUpperA((char *)CvtStr);
      CharToOemA((char *)CvtStr,(char *)CvtStr);
      LowerToUpper[I]=CvtStr[0];
      CvtStr[0]=I;
      OemToCharA((char *)CvtStr,(char *)CvtStr);
      CharLowerA((char *)CvtStr);
      CharToOemA((char *)CvtStr,(char *)CvtStr);
      UpperToLower[I]=CvtStr[0];
    }
  }
}

/*
   Инициализация системозависимой сортировки строк.
   Вызывать только после CopyGlobalSettings (потому что только тогда GetRegKey
   считает правильные данные) и перед InitKeysArray (потому что там уже
   используется сортировка)!
*/
void InitLCIDSort()
{
  unsigned char LCSortBuffer[256];
  unsigned int I;

  for (I=0;I<countof(LCSortBuffer);I++)
    LCSortBuffer[I]=I;

  Opt.LCIDSort=GetRegKey(L"System",L"LCID",LOCALE_USER_DEFAULT);
  far_qsort((void *)LCSortBuffer,256,sizeof(LCSortBuffer[0]),LCSort);

  for (I=0;I<countof(LCSortBuffer);I++)
    LCOrder[LCSortBuffer[I]]=I;

  LCOrder[0]=0;
  LCOrder[(unsigned)'\\']=1;
  LCOrder[(unsigned)'.']=2;

  for (I=0;I<countof(LCSortBuffer)-1;I++)
    if (LCSort(&LCSortBuffer[I],&LCSortBuffer[I+1])==0)
      LCOrder[LCSortBuffer[I+1]]=LCOrder[LCSortBuffer[I]];

  for (I=0;I<countof(LCOrder);I++)
    LCOrder[I]=LCOrder[UpperToLower[I]];
}

/*
   Инициализация массива клавиш.
   Вызывать только после CopyGlobalSettings, потому что только тогда GetRegKey
   считает правильные данные.
*/
void InitKeysArray()
{
  GetRegKey(L"Interface",L"HotkeyRules",Opt.HotkeyRules,1);
  int I;
  HKL Layout[10];

  int LayoutNumber=GetKeyboardLayoutList(sizeof(Layout)/sizeof(Layout[0]),Layout);

  if (LayoutNumber<5)
  {
    if(!Opt.HotkeyRules)
    {
      unsigned char CvtStr[2];
      CvtStr[1]=0;
      for (I=0;I<=255;I++)
      {
        int Keys[10];
        memset(Keys,0,sizeof(Keys));
        for (int J=0; J < LayoutNumber; J++)
        {
          int AnsiKey=MapVirtualKeyExA(I,2,Layout[J]) & 0xff;
          if (AnsiKey==0)
            continue;
          CvtStr[0]=AnsiKey;
          CvtStr[1]=0;
          //CharToOemA((char *)CvtStr,(char *)CvtStr); //???
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
      for (I=0;I<=255;I++)
      {
        for (int J=0;J<LayoutNumber;J++)
        {
          DWORD AnsiKey=VkKeyScanExA(I,Layout[J])&0xFF;
          if (AnsiKey==0xFF)
            continue;
          DWORD MapKey=MapVirtualKeyA(AnsiKey,2);
          KeyToKey[I]=static_cast<unsigned char>( MapKey ? MapKey : AnsiKey );
          break;
        }
      }
    }
  }
  _SVS(SysLogDump(L"KeyToKey calculate",0,KeyToKey,sizeof(KeyToKey),NULL));
  unsigned char KeyToKeyMap[256];
  if(GetRegKey(L"System",L"KeyToKeyMap",KeyToKeyMap,KeyToKey,sizeof(KeyToKeyMap)))
    memcpy(KeyToKey,KeyToKeyMap,sizeof(KeyToKey));
  //_SVS(SysLogDump("KeyToKey readed",0,KeyToKey,sizeof(KeyToKey),NULL));
}

int LocalKeyToKey(int Key)
{
  _KEYMACRO(CleverSysLog Clev(L"LocalKeyToKey()"));
  _KEYMACRO(SysLog(L"Param: Key=%08X",Key));
  unsigned char CvtStr[2];
  wchar_t wCvtStr[2];
  wCvtStr[1]=0;
  wCvtStr[0]=Key&0x0000FFFF;
  UnicodeToANSI((wchar_t *)wCvtStr,(char *)CvtStr,1);
  //_SVS(SysLog(L"CvtStr[0]=%X, return KeyToKey[CvtStr[0]] ==> %X",CvtStr[0],KeyToKey[CvtStr[0]]));
  return(KeyToKey[CvtStr[0]]);
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
  OemToCharBuffA((char *)CvtStr,(char *)CvtStr,1);
  return(IsCharAlphaA(CvtStr[0]));
}

int WINAPI LocalIsalphanum(unsigned Ch)
{
  if (Ch>=256)
    return(FALSE);

  unsigned char CvtStr[1];
  CvtStr[0]=Ch;
  OemToCharBuffA((char *)CvtStr,(char *)CvtStr,1);
  return(IsCharAlphaNumericA(CvtStr[0]));
}

unsigned WINAPI LocalUpper(unsigned LowerChar)
{
  return(LowerChar < 256 ? LowerToUpper[LowerChar]:LowerChar);
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
  while (1)
  {
    if (UpperToLower[(unsigned)*s1] != UpperToLower[(unsigned)*s2])
      return (UpperToLower[(unsigned)*s1] < UpperToLower[(unsigned)*s2]) ? -1 : 1;
    if (*(s1++) == 0)
      break;
    s2++;
  }
  return(0);
}

int __cdecl LocalStrnicmp(const char *s1,const char *s2,int n)
{
  while (n-- > 0)
  {
    if (UpperToLower[(unsigned)*s1] != UpperToLower[(unsigned)*s2])
      return (UpperToLower[(unsigned)*s1] < UpperToLower[(unsigned)*s2]) ? -1 : 1;
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

int _cdecl LCSort(const void *el1,const void *el2)
{
  char Str1[3],Str2[3];
  Str1[0]=*(char *)el1;
  Str2[0]=*(char *)el2;
  Str1[1]=Str2[1]=0;
  Str1[2]=Str2[2]=0;
  OemToCharBuffA(Str1,Str1,1);
  OemToCharBuffA(Str2,Str2,1);
  return(CompareStringA(Opt.LCIDSort,NORM_IGNORENONSPACE|SORT_STRINGSORT|NORM_IGNORECASE,Str1,1,Str2,1)-2);
}
