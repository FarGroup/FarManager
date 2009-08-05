/*
tvar.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "tvar.hpp"

static const wchar_t *toString(__int64 num)
{
  static wchar_t str[128];
  _i64tow(num, str, 10);
  return str;
};

TVar::~TVar()
{
  if ( str )
    delete [] str;
};

TVar::TVar(__int64 v) :
  vType(vtInteger),
  inum(v)
{
  str = NULL;
};

TVar::TVar(const wchar_t *v) :
  vType(vtString),
  inum(0)
{
  str = new wchar_t[StrLength(v)+1];
  if ( str )
    wcscpy(str, v);
};


TVar::TVar(const TVar& v) :
  vType(v.vType),
  inum(v.inum)
{
  if ( v.str )
  {
    str = new wchar_t[StrLength(v.str)+1];
    if ( str )
      wcscpy(str, v.str);
  }
  else
  {
    str = NULL;
  }
};

TVar& TVar::operator=(const TVar& v)
{
  if (this != &v)
  {
    vType = v.vType;
    inum = v.inum;
    if ( str )
      delete [] str;
    str = NULL;
    if ( v.str )
    {
      str = new wchar_t[StrLength(v.str)+1];
      if ( str )
        wcscpy(str, v.str);
    }
  }
  return *this;
}

__int64 TVar::i() const
{
  return isInteger() ? inum : ( str ? _wtoi64(str) : 0 );
}

const wchar_t *TVar::s() const
{
  if (isString())
    return  str ? str : L"";
  return ::toString(inum);
}


const wchar_t *TVar::toString()
{
  wchar_t s[128];
  switch ( vType )
  {
    case vtInteger:
      xwcsncpy(s, ::toString(inum),countof(s)-1);
      break;
    default:
      return str;
  }
  if ( str )
    delete [] str;
  str = new wchar_t[StrLength(s)+1];
  if ( str )
    wcscpy(str, s);
  vType = vtString;
  return str;
};

__int64 TVar::toInteger()
{
  if ( vType == vtString )
    inum = str ? _wtoi64(str) : 0;
  vType = vtInteger;
  return inum;
};

__int64 TVar::getInteger() const
{
	__int64 ret = inum;
  if ( vType == vtString )
    ret = str ? _wtoi64(str) : 0;
  return ret;
};

int operator==(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum == b.inum;          break;
    case vtString:  if ( b.isString() )  r = StrCmp(a.s(), b.s()) == 0; break;
  }
  return r;
};

int operator!=(const TVar& a, const TVar& b)
{
  int r = 1;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum != b.inum;          break;
    case vtString:  if ( b.isString() )  r = StrCmp(a.s(), b.s()) != 0; break;
  }
  return r;
};

int operator<(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum < b.inum;           break;
    case vtString:  if ( b.isString() )  r = StrCmp(a.s(), b.s()) < 0;  break;
  }
  return r;
};

int operator<=(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum <= b.inum;          break;
    case vtString:  if ( b.isString() )  r = StrCmp(a.s(), b.s()) <= 0; break;
  }
  return r;
};

int operator>(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum > b.inum;           break;
    case vtString:  if ( b.isString() )  r = StrCmp(a.s(), b.s()) > 0;  break;
  }
  return r;
};

int operator>=(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum >= b.inum;          break;
    case vtString:  if ( b.isString() )  r = StrCmp(a.s(), b.s()) >= 0; break;
  }
  return r;
};

static TVar addStr(const wchar_t *a, const wchar_t *b)
{
  TVar r(L"");
  wchar_t *c = new wchar_t[StrLength(a ? a : L"")+StrLength(b ? b : L"")+1];
  if ( c )
  {
    r = wcscat(wcscpy(c, a ? a : L""), b ? b : L"");
    delete [] c;
  }
  return r;
}

TVar operator+(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger: r = a.inum + b.inum;                   break;
        case vtString:  r = addStr(::toString(a.inum), b.s()); break;
      }
      break;
    case vtString:
      switch ( b.vType )
      {
        case vtInteger: r = addStr(a.s(), ::toString(b.inum)); break;
        case vtString:  r = addStr(a.s(), b.s());              break;
      }
      break;
  }
  return r;
};

TVar operator-(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger: r = a.inum - b.inum;                  break;
        case vtString:  r = a;                                break;
      }
      break;
    case vtString:
      r = a;
      break;
  }
  return r;
};

TVar operator*(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger: r = a.inum * b.inum;                  break;
        case vtString:  r = a;                                break;
      }
      break;
    case vtString:
      r = a;
      break;
  }
  return r;
};

TVar operator/(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger:
          r = b.inum ? ( a.inum / b.inum ) : _i64(0);
          break;
        case vtString:
          r = a;
          break;
      }
      break;
    case vtString:
      r = a;
      break;
  }
  return r;
};

TVar operator%(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger:
          r = b.inum ? ( a.inum % b.inum ) : _i64(0);
          break;
        case vtString:
          r = a;
          break;
      }
      break;
    case vtString:
      r = a;
      break;
  }
  return r;
};

TVar operator|(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger: r = a.inum | b.inum;                  break;
        case vtString:  r = a;                                break;
      }
      break;
    case vtString:
      r = a;
      break;
  }
  return r;
};

TVar operator&(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger: r = a.inum & b.inum;                  break;
        case vtString:  r = a;                                break;
      }
      break;
    case vtString:
      r = a;
      break;
  }
  return r;
};

TVar operator||(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger: r = a.inum || b.inum;                 break;
        case vtString:  r = a;                                break;
      }
      break;
    case vtString:
      r = a;
      break;
  }
  return r;
};

TVar operator&&(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger: r = a.inum && b.inum;                 break;
        case vtString:  r = a;                                break;
      }
      break;
    case vtString:
      r = a;
      break;
  }
  return r;
};

TVar operator^(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger: r = a.inum ^ b.inum;                  break;
        case vtString:  r = a;                                break;
      }
      break;
    case vtString:
      r = a;
      break;
  }
  return r;
}

TVar operator>>(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger: r = a.inum >> b.inum;                 break;
        case vtString:  r = a;                                break;
      }
      break;
    case vtString:
      r = a;
      break;
  }
  return r;
}

TVar operator<<(const TVar& a, const TVar& b)
{
  TVar r;
  switch ( a.vType )
  {
    case vtInteger:
      switch ( b.vType )
      {
        case vtInteger: r = a.inum << b.inum;                 break;
        case vtString:  r = a;                                break;
      }
      break;
    case vtString:
      r = a;
      break;
  }
  return r;
}

TVar TVar::operator+()
{
  return *this;
};


TVar TVar::operator-()
{
  switch ( vType )
  {
    case vtInteger:
      return TVar(-inum);
    default:
      return *this;
  }
};

TVar TVar::operator!()
{
  switch ( vType )
  {
    case vtInteger:
      return TVar(!inum);
    default:
      return *this;
  }
};

TVar TVar::operator~()
{
  switch ( vType )
  {
    case vtInteger:
      return TVar(~inum);
    default:
      return *this;
  }
};

//---------------------------------------------------------------
// –абота с таблицами имен переменных
//---------------------------------------------------------------

int hash(const wchar_t *p)
{
  int i = 0;
  wchar_t *pp = (wchar_t*)p;
  while ( *pp )
    i = i << (1^*(pp++));
  if ( i < 0 )
    i = -i;
  i %= V_TABLE_SIZE;
  return i;
}

int isVar(TVarTable table, const wchar_t *p)
{
  int i = hash(p);
  for ( TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next) )
    if ( !StrCmpI(n->str, p) )
      return 1;
  return 0;
}

TVarSet *varLook(TVarTable table, const wchar_t *p, bool ins)
{
  int i = hash(p);
  for ( TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next) )
    if ( !StrCmpI(n->str, p) )
      return n;
  if ( ins )
  {
    TVarSet *nn = new TVarSet(p);
    nn->next = table[i];
    table[i] = nn;
    return nn;
  }
  return NULL;
}

TVarSet *varEnum(TVarTable table,int NumTable, int Index)
{
  if(NumTable >= V_TABLE_SIZE)
    return NULL;

  TVarSet *n = table[NumTable];
  for(int I=0; I < Index && n; ++I)
    n = ((TVarSet*)n->next);

  return n;
}

void varKill(TVarTable table, const wchar_t *p)
{
  int i = hash(p);
  TVarSet *nn = table[i];
  for ( TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next) )
  {
    if ( !StrCmpI(n->str, p) )
    {
      if(n == table[i])
         table[i]=((TVarSet*)n->next);
      else
         nn->next= n->next;

      //( ( n == table[i] ) ? table[i] : nn->next ) = n->next;
      delete n;
      return;
    }
    nn = n;
  }
}

void initVTable(TVarTable table)
{
  for ( int i = 0 ; i < V_TABLE_SIZE ; i++ )
    table[i] = NULL;
}

void deleteVTable(TVarTable table)
{
  for ( int i = 0 ; i < V_TABLE_SIZE ; i++ )
  {
    while ( table[i] != NULL )
    {
      TVarSet *n = ((TVarSet*)(table[i]->next));
      table[i]->next = NULL;
      delete table[i];
      table[i] = n;
    }
  }
}
