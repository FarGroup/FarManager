/*
int64.cpp

64-битная арифметика

*/

/* Revision: 1.01 16.10.2000 $ */

/*
Modify:
  16.10.2000 SVS
    + __int64: функции =, Set
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#ifndef __INT64_HPP__
#include "int64.hpp"
#endif

int64::int64()
{
  HighPart=LowPart=0;
}


int64::int64(DWORD n)
{
  HighPart=0;
  LowPart=n;
}


int64::int64(DWORD HighPart,DWORD LowPart)
{
  int64::HighPart=HighPart;
  int64::LowPart=LowPart;
}


int64 int64::operator = (int64 n)
{
  HighPart=n.HighPart;
  LowPart=n.LowPart;
  return(*this);
}

int64 int64::operator = (__int64 n)
{
  HighPart=(n>>32)&0xFFFFFFFFUL;
  LowPart=n&0xFFFFFFFFUL;
  return(*this);
}

int64 int64::operator << (int n)
{
  int64 res=*this;
  while (n--)
  {
    res.HighPart<<=1;
    if (res.LowPart & 0x80000000)
      res.HighPart|=1;
    res.LowPart<<=1;
  }
  return(res);
}


int64 int64::operator >> (int n)
{
  int64 res=*this;
  while (n--)
  {
    res.LowPart>>=1;
    if (res.HighPart & 1)
      res.LowPart|=0x80000000;
    res.HighPart>>=1;
  }
  return(res);
}


int64 operator / (int64 n1,int64 n2)
{
  if (n1.HighPart==0 && n2.HighPart==0)
    return(int64(0,n1.LowPart/n2.LowPart));
  int ShiftCount=0;
  while (n1>n2)
  {
    n2=n2<<1;
    if (++ShiftCount>64)
      return(0);
  }
  int64 res=0;
  while (ShiftCount-- >= 0)
  {
    res=res<<1;
    if (n1>=n2)
    {
      n1-=n2;
      ++res;
    }
    n2=n2>>1;
  }
  return(res);
}


int64 operator * (int64 n1,int64 n2)
{
  if (n1.HighPart==0 && n2.HighPart==0)
    return(int64(0,n1.LowPart*n2.LowPart));
  int64 res=0;
  for (int I=0;I<64;I++)
  {
    if (n2.LowPart & 1)
      res+=n1;
    n1=n1<<1;
    n2=n2>>1;
  }
  return(res);
}


int64 operator % (int64 n1,int64 n2)
{
  if (n1.HighPart==0 && n2.HighPart==0)
    return(int64(0,n1.LowPart%n2.LowPart));
  return(n1-n1/n2*n2);
}


int64 operator += (int64 &n1,int64 n2)
{
  n1=n1+n2;
  return(n1);
}


int64 operator -= (int64 &n1,int64 n2)
{
  n1=n1-n2;
  return(n1);
}


int64 operator + (int64 n1,int64 n2)
{
  int64 res;
  res.LowPart=n1.LowPart+n2.LowPart;
  res.HighPart=n1.HighPart+n2.HighPart;
  if (res.LowPart<n1.LowPart)
    res.HighPart++;
  return(res);
}


int64 operator - (int64 n1,int64 n2)
{
  int64 res;
  res.LowPart=n1.LowPart-n2.LowPart;
  res.HighPart=n1.HighPart-n2.HighPart;
  if (res.LowPart>n1.LowPart)
    res.HighPart--;
  return(res);
}


int64 operator ++ (int64 &n)
{
  n=n+1;
  return(n);
}


int64 operator -- (int64 &n)
{
  n=n-1;
  return(n);
}


bool operator == (int64 n1,int64 n2)
{
  return(n1.LowPart==n2.LowPart && n1.HighPart==n2.HighPart);
}


bool operator > (int64 n1,int64 n2)
{
  return(n1.HighPart>n2.HighPart || n1.HighPart==n2.HighPart && n1.LowPart>n2.LowPart);
}


bool operator < (int64 n1,int64 n2)
{
  return(n1.HighPart<n2.HighPart || n1.HighPart==n2.HighPart && n1.LowPart<n2.LowPart);
}


bool operator != (int64 n1,int64 n2)
{
  return(n1.LowPart!=n2.LowPart || n1.HighPart!=n2.HighPart);
}


bool operator >= (int64 n1,int64 n2)
{
  return(n1>n2 || n1==n2);
}


bool operator <= (int64 n1,int64 n2)
{
  return(n1<n2 || n1==n2);
}


void int64::Set(DWORD HighPart,DWORD LowPart)
{
  int64::HighPart=HighPart;
  int64::LowPart=LowPart;
}

void int64::Set(__int64 n)
{
  HighPart=(n>>32)&0xFFFFFFFFUL;
  LowPart=n&0xFFFFFFFFUL;
}

void int64::itoa(char *Str)
{
  int64 tmp=*this;
  char TmpStr[50];
  int Pos=0;

  do
  {
    TmpStr[Pos++]=(tmp%10).LowPart+'0';
    tmp=tmp/10;
  } while (tmp!=0);

  for (int I=0;I<Pos;I++)
    Str[I]=TmpStr[Pos-I-1];
  Str[Pos]=0;
}
