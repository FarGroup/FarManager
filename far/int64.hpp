#ifndef __INT64_HPP__
#define __INT64_HPP__
/*
int64.hpp

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

class int64
{
  public:
    int64();
    int64(DWORD n);
    int64(DWORD HighPart,DWORD LowPart);

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

    void Set(DWORD HighPart,DWORD LowPart);
    void Set(__int64 n);
    void itoa(char *Str);

    DWORD LowPart,HighPart;
};

#endif // __INT64_HPP__
