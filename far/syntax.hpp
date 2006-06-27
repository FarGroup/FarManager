/*
syntax.hpp

Реализация парсера для MacroDrive II

*/

/* Revision: 1.08 17.03.2006 $ */

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

//---------------------------------------------------------------
// Реализация класса TVar ("кастрированый" вариант - только целое
// и строковое значение)
//---------------------------------------------------------------

#ifndef __SYNTAX_H
#define __SYNTAX_H

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
  friend TVar operator&&(const TVar&, const TVar&);
  friend TVar operator|(const TVar&, const TVar&);
  friend TVar operator||(const TVar&, const TVar&);
  friend TVar operator^(const TVar&, const TVar&);

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
// Работа с таблицами имен переменных
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
        str = new wchar_t[wcslen(s)+1];
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

//---------------------------------------------------------------
// Парсер выражений
//---------------------------------------------------------------

#define MAXEXEXSTACK 256

enum TToken
{
  tNo, tEnd,  tLet,
  tVar, tStr, tInt, tFunc, tFARVar,
  tPlus, tMinus, tMul, tDiv, tLp, tRp, tComma,
  tBoolAnd, tBoolOr, tBitAnd, tBitOr, tBitXor, tNot,
  tEq, tNe, tLt, tLe, tGt, tGe,
};


struct TMacroKeywords {
  int Type;                    // Тип: 0=Area, 1=Flags, 2=Condition
  const wchar_t *Name;                  // Наименование
  DWORD Value;         // Значение
  DWORD Reserved;
};

enum errParseCode
{
  err_Unrecognized_keyword,
  err_Unrecognized_function,
  err_Not_expected_ELSE,
  err_Not_expected_END,
  err_Unexpected_EOS,
  err_Expected,
  err_Bad_Hex_Control_Char,
  err_Bad_Control_Char,
  err_Var_Expected,
  err_Expr_Expected,
};

int parseExpr(const wchar_t*&, unsigned long*, wchar_t, wchar_t);
void keyMacroParseError(int, const wchar_t*, const wchar_t*, const wchar_t* = NULL);
DWORD funcLook(const wchar_t *s, int& nParam);

extern int MKeywordsSize;
extern TMacroKeywords MKeywords[];
extern int _macro_nErr;


#endif
