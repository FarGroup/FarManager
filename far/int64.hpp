#ifndef __FAR_INT64_HPP__
#define __FAR_INT64_HPP__
/*
int64.hpp

64-битная арифметика

*/

/* Revision: 1.06 25.04.2005 $ */

/*
Modify:
  24.04.2005 AY
    ! GCC
  05.10.2003 KM
    + Конструкторы int64(int n) и int64(__int64 n)
  22.02.2002 SVS
    ! Введение FAR_INT64
  13.03.2001 SVS
    ! Нда. Стареем. << и <<= получились как бы одинаковыми :-(
  12.03.2001 SVS
    ! применяем __int64 в классе int64 :-)
  16.10.2000 SVS
    + __int64: функции =, Set
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define __NEW_CLASS_FAR_INT64__
#if defined(__NEW_CLASS_FAR_INT64__)
#include "farconst.hpp"
#endif

class int64
{
  public:
    int64();
    int64(DWORD n);
    int64(int n);
    int64(__int64 n);
    int64(LONG HighPart,DWORD LowPart);

    int64 operator = (int64 n);
    int64 operator = (__int64 n);
    int64 operator << (int n);
    int64 operator >> (int n);

    friend int64 operator / (int64 n1,int64 n2);
    friend int64 operator * (int64 n1,int64 n2);
    friend int64 operator % (int64 n1,int64 n2);
    friend int64 operator += (int64 &n1,int64 n2);
    friend int64 operator -= (int64 &n1,int64 n2);
    friend int64 operator + (int64 n1,int64 n2);
    friend int64 operator - (int64 n1,int64 n2);
    friend int64 operator ++ (int64 &n);
    friend int64 operator -- (int64 &n);
    friend bool operator == (int64 n1,int64 n2);
    friend bool operator > (int64 n1,int64 n2);
    friend bool operator < (int64 n1,int64 n2);
    friend bool operator != (int64 n1,int64 n2);
    friend bool operator >= (int64 n1,int64 n2);
    friend bool operator <= (int64 n1,int64 n2);

    void Set(LONG HighPart,DWORD LowPart);
    void Set(__int64 n);
    char* itoa(char *Str);

#if defined(__NEW_CLASS_FAR_INT64__)
    DWORD& PLow() {return Number.Part.LowPart;}
    LONG&  PHigh() {return Number.Part.HighPart;}

    FAR_INT64 Number;
#else
    DWORD& PLow() {return LowPart;}
    LONG& PHigh() {return HighPart;}

    DWORD LowPart;
    LONG  HighPart;
#endif
};

#if defined(__NEW_CLASS_FAR_INT64__)
inline int64::int64()
{
  Number.i64=_i64(0);
}

inline int64::int64(DWORD n)
{
  Number.i64=(__int64)n;
}

inline int64::int64(int n)
{
  Number.i64=(__int64)n;
}

inline int64::int64(__int64 n)
{
  Number.i64=n;
}

inline int64::int64(LONG HighPart,DWORD LowPart)
{
  Set(HighPart,LowPart);
}

inline void  int64::Set(__int64 n)
{
  Number.i64=n;
}

inline char* int64::itoa(char *Str)
{
  return _i64toa(Number.i64,Str,10);
}

inline void int64::Set(LONG HighPart,DWORD LowPart)
{
  Number.Part.HighPart=HighPart;
  Number.Part.LowPart=LowPart;
}

inline int64 int64::operator = (int64 n)
{
  Number.i64=n.Number.i64;
  return(*this);
}

inline int64 int64::operator = (__int64 n)
{
  Number.i64=n;
  return(*this);
}

inline bool operator == (int64 n1,int64 n2)
{
  return (n1.Number.i64 == n2.Number.i64);
}

inline bool operator > (int64 n1,int64 n2)
{
  return (n1.Number.i64 > n2.Number.i64);
}

inline bool operator < (int64 n1,int64 n2)
{
  return (n1.Number.i64 < n2.Number.i64);
}


inline bool operator != (int64 n1,int64 n2)
{
  return (n1.Number.i64 != n2.Number.i64);
}

inline bool operator >= (int64 n1,int64 n2)
{
  return (n1.Number.i64 >= n2.Number.i64);
}


inline bool operator <= (int64 n1,int64 n2)
{
  return (n1.Number.i64 <= n2.Number.i64);
}

inline int64 operator ++ (int64 &n)
{
  n.Number.i64++;
  return(n);
}


inline int64 operator -- (int64 &n)
{
  n.Number.i64--;
  return(n);
}

inline int64 int64::operator << (int n)
{
  int64 res=*this;
  res.Number.i64<<=n;
  return(res);
}


inline int64 int64::operator >> (int n)
{
  int64 res=*this;
  res.Number.i64>>=n;
  return(res);
}

inline int64 operator * (int64 n1,int64 n2)
{
  int64 res;
  res.Number.i64=n1.Number.i64*n2.Number.i64;
  return(res);
}

inline int64 operator += (int64 &n1,int64 n2)
{
  n1.Number.i64+=n2.Number.i64;
  return(n1);
}


inline int64 operator -= (int64 &n1,int64 n2)
{
  n1.Number.i64-=n2.Number.i64;
  return(n1);
}


inline int64 operator + (int64 n1,int64 n2)
{
  int64 res;
  res.Number.i64=n1.Number.i64+n2.Number.i64;
  return(res);
}


inline int64 operator - (int64 n1,int64 n2)
{
  int64 res;
  res.Number.i64=n1.Number.i64-n2.Number.i64;
  return(res);
}

#endif

#endif // __FAR_INT64_HPP__
