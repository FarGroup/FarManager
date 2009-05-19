/*
tvar.hpp

–еализаци€ класса TVar ("кастрированый" вариант - только целое и строковое значение)
(дл€ макросов)

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
  TVar(const wchar_t*);
  TVar(const TVar&);
  ~TVar();

  friend TVar operator+(const TVar&, const TVar&);
  friend TVar operator-(const TVar&, const TVar&);
  friend TVar operator*(const TVar&, const TVar&);
  friend TVar operator/(const TVar&, const TVar&);
  friend TVar operator%(const TVar&, const TVar&);

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
  TVar& operator%=(const TVar& b) { return *this = *this%b; };

  TVar operator+();
  TVar operator-();
  TVar operator!();
  TVar operator~();

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
  __int64 getInteger() const;
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
extern TVarSet *varLook(TVarTable worktable, const wchar_t* name, bool ins=false);
extern void varKill(TVarTable, const wchar_t*);
extern void initVTable(TVarTable);
extern void deleteVTable(TVarTable);

inline TVarSet *varInsert(TVarTable t, const wchar_t *s)
{
  return varLook(t, s, true);
}

#endif // __TVAR_H
