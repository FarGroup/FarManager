/*
tvar.hpp

–еализаци€ класса TVar ("кастрированый" вариант - только целое и строковое значение)
(дл€ макросов)

*/

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

#ifndef __TVAR_H
#define __TVAR_H

#include "fn.hpp"

enum TVarType { vtInteger, vtString };

class TVar
{
private:
  TVarType vType;
  __int64 inum;
  wchar_t *str;
public:
  TVar(__int64 = 0);
//  TVar(const char*);
  TVar(const wchar_t*);
  TVar(const TVar&);
  ~TVar();

  friend TVar operator+(const TVar&, const TVar&);
  friend TVar operator-(const TVar&, const TVar&);
  friend TVar operator*(const TVar&, const TVar&);
  friend TVar operator/(const TVar&, const TVar&);

  friend TVar operator&(const TVar&, const TVar&);
  friend TVar operator|(const TVar&, const TVar&);
  friend TVar operator^(const TVar&, const TVar&);
  friend TVar operator>>(const TVar&, const TVar&);
  friend TVar operator<<(const TVar&, const TVar&);

  friend TVar operator&&(const TVar&, const TVar&);
  friend TVar operator||(const TVar&, const TVar&);

  TVar& operator=(const TVar&);

  TVar& operator+=(const TVar& b) { return *this = *this+b; };
  TVar& operator-=(const TVar& b) { return *this = *this-b; };
  TVar& operator*=(const TVar& b) { return *this = *this*b; };
  TVar& operator/=(const TVar& b) { return *this = *this/b; };

  TVar operator+();
  TVar operator-();
  TVar operator!();

  friend int operator==(const TVar&, const TVar&);
  friend int operator!=(const TVar&, const TVar&);
  friend int operator<(const TVar&, const TVar&);
  friend int operator<=(const TVar&, const TVar&);
  friend int operator>(const TVar&, const TVar&);
  friend int operator>=(const TVar&, const TVar&);

  TVarType type() { return vType; };

  int isString()   const { return vType == vtString;  }
  int isInteger()  const { return vType == vtInteger; }

  __int64 i()         const;// { return isInteger() ? inum : 0; };
  const wchar_t *s()  const;// { return isString() ? ( str ? str : "" ) : ""; };

  const wchar_t *toString();
  __int64 toInteger();
};

//---------------------------------------------------------------
// –абота с таблицами имен переменных
//---------------------------------------------------------------

class TAbstractSet
{
  public:
    wchar_t *str;
    TAbstractSet *next;
    TAbstractSet(const wchar_t *s)
    {
      str = NULL;
      next = NULL;
      if ( s )
      {
        str = new wchar_t[StrLength(s)+1];
        wcscpy(str, s);
      }
    }
    ~TAbstractSet()
    {
      if ( str )
        delete [] str;
    }
};

class TVarSet : public TAbstractSet
{
  public:
    TVar value;
    TVarSet(const wchar_t *s) :
      TAbstractSet(s),
      value() {}
};

const int V_TABLE_SIZE = 23;

typedef TVarSet *(TVarTable)[V_TABLE_SIZE];
extern int isVar(TVarTable, const wchar_t*);
extern TVarSet *varEnum(TVarTable, int, int);
extern TVarSet *varLook(TVarTable, const wchar_t*, int&, int = 0);
extern void varKill(TVarTable, const wchar_t*);
extern void initVTable(TVarTable);
extern void deleteVTable(TVarTable);

inline TVarSet *varInsert(TVarTable t, const wchar_t *s)
{
  int e;
  return varLook(t, s, e, 1);
}

#endif // __TVAR_H
