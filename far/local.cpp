/*
local.cpp

Сравнение без учета регистра, преобразование регистра

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

static int _cdecl LCSort(const void *el1,const void *el2);

static unsigned char LowerToUpper[256];
static unsigned char UpperToLower[256];
static unsigned char LCOrder[256];
static unsigned char KeyToKey[256];

void LocalUpperInit()
{
  unsigned char CvtStr[2],ReverseCvtStr[2];
  CvtStr[1]=0;
  for (int I=0;I<sizeof(LowerToUpper)/sizeof(LowerToUpper[0]);I++)
  {
    CvtStr[0]=I;
    LowerToUpper[I]=UpperToLower[I]=I;
    OemToChar(CvtStr,CvtStr);
    CharToOem(CvtStr,ReverseCvtStr);
    if (IsCharAlpha(CvtStr[0]) && ReverseCvtStr[0]==I)
    {
      CharUpper(CvtStr);
      CharToOem(CvtStr,CvtStr);
      LowerToUpper[I]=CvtStr[0];
      CvtStr[0]=I;
      OemToChar(CvtStr,CvtStr);
      CharLower(CvtStr);
      CharToOem(CvtStr,CvtStr);
      UpperToLower[I]=CvtStr[0];
    }
  }
  char LCSortBuffer[256];
  for (int I=0;I<sizeof(LCSortBuffer)/sizeof(LCSortBuffer[0]);I++)
    LCSortBuffer[I]=I;
  qsort((void *)LCSortBuffer,256,sizeof(LCSortBuffer[0]),LCSort);
  for (int I=0;I<sizeof(LCSortBuffer)/sizeof(LCSortBuffer[0]);I++)
    LCOrder[LCSortBuffer[I]]=I;
  LCOrder[0]=0;
  LCOrder['\\']=1;
  LCOrder['.']=2;
  for (int I=0;I<sizeof(LCSortBuffer)/sizeof(LCSortBuffer[0])-1;I++)
    if (LCSort(&LCSortBuffer[I],&LCSortBuffer[I+1])==0)
      LCOrder[LCSortBuffer[I+1]]=LCOrder[LCSortBuffer[I]];
  for (int I=0;I<sizeof(LCOrder)/sizeof(LCOrder[0]);I++)
    LCOrder[I]=LCOrder[UpperToLower[I]];
  for (int I=0;I<sizeof(KeyToKey)/sizeof(KeyToKey[0]);I++)
    KeyToKey[I]=I;
  HKL Layout[10];
  int LayoutNumber=GetKeyboardLayoutList(sizeof(Layout)/sizeof(Layout[0]),Layout);
  if (LayoutNumber<5)
    for (int I=0;I<=255;I++)
    {
      int Keys[10];
      memset(Keys,0,sizeof(Keys));
      for (int J=0;J<LayoutNumber;J++)
      {
        int AnsiKey=MapVirtualKeyEx(I,2,Layout[J]) & 0xff;
        if (AnsiKey==0)
          continue;
        CvtStr[0]=AnsiKey;
        CvtStr[1]=0;
        CharToOem(CvtStr,CvtStr);
        Keys[J]=CvtStr[0];
      }
      if (Keys[0]!=0 && Keys[1]!=0)
      {
        KeyToKey[LocalLower(Keys[0])]=Keys[1];
        KeyToKey[LocalUpper(Keys[0])]=Keys[1];
        KeyToKey[LocalLower(Keys[1])]=Keys[0];
        KeyToKey[LocalUpper(Keys[1])]=Keys[0];
      }
    }
}


int LocalIslower(int Ch)
{
  return(Ch<256 && LowerToUpper[Ch]!=Ch);
}


int LocalIsupper(int Ch)
{
  return(Ch<256 && LowerToUpper[Ch]==Ch);
}



int LocalIsalpha(int Ch)
{
  if (Ch>=256)
    return(FALSE);
  unsigned char CvtStr[1];
  CvtStr[0]=Ch;
  OemToCharBuff(CvtStr,CvtStr,1);
  return(IsCharAlpha(CvtStr[0]));
}


int LocalIsalphanum(int Ch)
{
  if (Ch>=256)
    return(FALSE);
  unsigned char CvtStr[1];
  CvtStr[0]=Ch;
  OemToCharBuff(CvtStr,CvtStr,1);
  return(IsCharAlphaNumeric(CvtStr[0]));
}


int LocalUpper(int LowerChar)
{
  return(LowerChar < 256 ? LowerToUpper[LowerChar]:LowerChar);
}


void LocalUpperBuf(char *Buf,int Length)
{
  for (int I=0;I<Length;I++)
    Buf[I]=LowerToUpper[Buf[I]];
}

int LocalLower(int UpperChar)
{
  return(UpperChar < 256 ? UpperToLower[UpperChar]:UpperChar);
}


void LocalStrupr(char *s1)
{
  while (*s1)
    *s1=LowerToUpper[*(s1++)];
}


void LocalStrlwr(char *s1)
{
  while (*s1)
    *s1=UpperToLower[*(s1++)];
}


int LocalStricmp(char *s1,char *s2)
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


int LocalStrnicmp(char *s1,char *s2,int n)
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


int LCStricmp(char *s1,char *s2)
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


int _cdecl LCSort(const void *el1,const void *el2)
{
  char Str1[3],Str2[3];
  Str1[0]=*(char *)el1;
  Str2[0]=*(char *)el2;
  Str1[1]=Str2[1]=0;
  Str1[2]=Str2[2]=0;
  OemToCharBuff(Str1,Str1,1);
  OemToCharBuff(Str2,Str2,1);
  return(CompareString(LOCALE_USER_DEFAULT,NORM_IGNORENONSPACE|SORT_STRINGSORT|NORM_IGNORECASE,Str1,1,Str2,1)-2);
}


int LocalKeyToKey(int Key)
{
  return(KeyToKey[Key]);
}
