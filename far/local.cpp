/*
local.cpp

Сравнение без учета регистра, преобразование регистра

*/

/* Revision: 1.17 11.07.2003 $ */

/*
Modify:
  11.07.2003 SVS
    + LCNumStricmp() - "цифровое" сравнение двух строк с учетом локали
  08.04.2003 SVS
    + IsUpperOrLower[] - в ответ на "почему LocalIsupper держит пробел за большую букву"
  17.10.2002 SVS
    ! Закомменчу свои _SVS() ;-)
  10.04.2002 SVS
    ! Технологический патч.
  26.03.2002 IS
    + InitLCIDSort - инициализация системозависимой сортировки строк.
      Вызывать только после CopyGlobalSettings, потому что только тогда
      GetRegKey считает правильные данные.
  03.03.2002 SVS
    ! Если для VC вставить ключ /Gr, то видим кучу багов :-/
  14.01.2002 SVS
    ! Содержимое KeyToKey приведем к верхнему регистру
  11.01.2002 IS
    + void InitKeysArray()
      Инициализация массива клавиш. Вызывать только после CopyGlobalSettings,
      потому что только тогда GetRegKey считает правильные данные.
  27.11.2001 DJ
    - всякая мелочевка
  04.07.2001 SVS
    + Opt.LCIDSort
  25.06.2001 IS
    ! Приведение Local* в соответствие с "официальным" plugin.hpp
    ! Внедрение const
  07.05.2001 DJ
    ! LowerToUpper/UpperToLower более не static (для оптимизированных
      inline-функций)
  06.05.2001 DJ
    ! перетрях #include
  28.12.2000 SVS
    + добавлена обработка Opt.HotkeyRules
  08.11.2000 SVS
    ! Изменен массив клавиш Keys - теперь содержит сканкоды.
  28.08.2000 SVS
    + Добавлена функция LocalLowerBuf
    ! Функции модифицированы под WINAPI с целью применения в FSF
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

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
    OemToChar((char *)CvtStr,(char *)CvtStr);
    CharToOem((char *)CvtStr,(char *)ReverseCvtStr);
    IsUpperOrLower[I]=0;
    if (IsCharAlpha(CvtStr[0]) && ReverseCvtStr[0]==I)
    {
      IsUpperOrLower[I]=IsCharLower(CvtStr[0])?1:(IsCharUpper(CvtStr[0])?2:0);
      CharUpper((char *)CvtStr);
      CharToOem((char *)CvtStr,(char *)CvtStr);
      LowerToUpper[I]=CvtStr[0];
      CvtStr[0]=I;
      OemToChar((char *)CvtStr,(char *)CvtStr);
      CharLower((char *)CvtStr);
      CharToOem((char *)CvtStr,(char *)CvtStr);
      UpperToLower[I]=CvtStr[0];
    }
  }
}

/* $ 26.03.2002 IS
   Инициализация системозависимой сортировки строк.
   Вызывать только после CopyGlobalSettings (потому что только тогда GetRegKey
   считает правильные данные) и перед InitKeysArray (потому что там уже
   используется сортировка)!
*/
void InitLCIDSort()
{
  char LCSortBuffer[256];
  int I;

  for (I=0;I<sizeof(LCSortBuffer)/sizeof(LCSortBuffer[0]);I++)
    LCSortBuffer[I]=I;

  Opt.LCIDSort=GetRegKey("System","LCID",LOCALE_USER_DEFAULT);
  qsort((void *)LCSortBuffer,256,sizeof(LCSortBuffer[0]),LCSort);

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

  for (I=0;I<sizeof(KeyToKey)/sizeof(KeyToKey[0]);I++)
    KeyToKey[I]=I;
}
/* IS $ */

/* $ 11.01.2002 IS
   Инициализация массива клавиш.
   Вызывать только после CopyGlobalSettings, потому что только тогда GetRegKey
   считает правильные данные.
*/
void InitKeysArray()
{
  GetRegKey("Interface","HotkeyRules",Opt.HotkeyRules,1);
  unsigned char CvtStr[2];
  int I;
  CvtStr[1]=0;
  HKL Layout[10];

  int LayoutNumber=GetKeyboardLayoutList(sizeof(Layout)/sizeof(Layout[0]),Layout);

  if (LayoutNumber<5)
  {
    /* $ 08.11.2000 SVS
       Изменен массив клавиш Keys - теперь содержит сканкоды.
    */
    /* 28.12.2000 SVS
      + добавлена обработка Opt.HotkeyRules */
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
          CharToOem((char *)CvtStr,(char *)CvtStr);
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
          CharToOem((char *)CvtStr,(char *)CvtStr);
          KeyToKey[CvtStr[0]]=AnsiKey;
        }
      }
    }
  }
  //_SVS(SysLogDump("KeyToKey calculate",0,KeyToKey,sizeof(KeyToKey),NULL));
  unsigned char KeyToKeyMap[256];
  if(GetRegKey("System","KeyToKeyMap",KeyToKeyMap,KeyToKey,sizeof(KeyToKeyMap)))
    memcpy(KeyToKey,KeyToKeyMap,sizeof(KeyToKey));
  //_SVS(SysLogDump("KeyToKey readed",0,KeyToKey,sizeof(KeyToKey),NULL));
  /* SVS $ */
  /* SVS $ */
}
/* IS 11.01.2002 $ */

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
  OemToCharBuff((char *)CvtStr,(char *)CvtStr,1);
  return(IsCharAlpha(CvtStr[0]));
}

int WINAPI LocalIsalphanum(unsigned Ch)
{
  if (Ch>=256)
    return(FALSE);

  unsigned char CvtStr[1];
  CvtStr[0]=Ch;
  OemToCharBuff((char *)CvtStr,(char *)CvtStr,1);
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
   Добавлена функция LocalLowerBuf
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
   устраняем неоднозначность :-)
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
       // берем длину числа без ведущих нулей
       int dig_len1 = __digit_cnt_0(s1, &s1);
       int dig_len2 = __digit_cnt_0(s2, &s2);
       // если одно длиннее другого, значит они и больше! :)
       if(dig_len1 != dig_len2)
         return dig_len1 - dig_len2;
       // длины одинаковы, сопоставляем...
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
    return strlen(ts2)-strlen(ts1);
  return (Ret < 0) ? -1 : 1;
}

int _cdecl LCSort(const void *el1,const void *el2)
{
  char Str1[3],Str2[3];
  Str1[0]=*(char *)el1;
  Str2[0]=*(char *)el2;
  Str1[1]=Str2[1]=0;
  Str1[2]=Str2[2]=0;
  OemToCharBuff(Str1,Str1,1);
  OemToCharBuff(Str2,Str2,1);
  return(CompareString(Opt.LCIDSort,NORM_IGNORENONSPACE|SORT_STRINGSORT|NORM_IGNORECASE,Str1,1,Str2,1)-2);
}


int LocalKeyToKey(int Key)
{
  return(KeyToKey[Key]);
}
