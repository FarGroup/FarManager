/*
syntax.hpp

Реализация парсера для MacroDrive II

*/

/* Revision: 1.05 16.03.2006 $ */

/*
Modify:
  16.03.2006 SVS
    + err_Unexpected_function
  02.03.2006 SVS
    ! Числа в макросах имеют суть __int64
  24.04.2005 AY
    ! GCC
  05.03.2005 SVS
    + varEnum()
  05.08.2004 SVS
    + funcLook()
  14.06.2004 SVS & AN
    + Адд
*/
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
  char *str;
public:
  TVar(__int64 = 0);
  TVar(const char*);
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

  __int64 i()      const;// { return isInteger() ? inum : 0; };
  const char *s()  const;// { return isString() ? ( str ? str : "" ) : ""; };

  const char *toString();
  __int64 toInteger();
};

//---------------------------------------------------------------
// Работа с таблицами имен переменных
//---------------------------------------------------------------

class TAbstractSet
{
  public:
    char *str;
    TAbstractSet *next;
    TAbstractSet(const char *s)
    {
      str = NULL;
      next = NULL;
      if ( s )
      {
        str = new char[strlen(s)+1];
        strcpy(str, s);
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
    TVarSet(const char *s) :
      TAbstractSet(s),
      value() {}
};

const int V_TABLE_SIZE = 23;

typedef TVarSet *(TVarTable)[V_TABLE_SIZE];
extern int isVar(TVarTable, const char*);
extern TVarSet *varEnum(TVarTable, int, int);
extern TVarSet *varLook(TVarTable, const char*, int&, int = 0);
extern void varKill(TVarTable, const char*);
extern void initVTable(TVarTable);
extern void deleteVTable(TVarTable);

inline TVarSet *varInsert(TVarTable t, const char *s)
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
  char *Name;                  // Наименование
  DWORD Value;         // Значение
  DWORD Reserved;
};

enum errParseCode
{
  err_Unrecognised_keyword,
  err_Unexpected_function,
  err_Not_expected_ELSE,
  err_Not_expected_END,
  err_Unexpected_EOS,
  err_Expected,
  err_Bad_Hex_Control_Char,
  err_Bad_Control_Char,
  err_Var_Expected,
  err_Expr_Expected,
};

int parseExpr(const char*&, unsigned long*, char, char);
void keyMacroParseError(int, const char*, const char*, const char* = NULL);
DWORD funcLook(const char *s, int& nParam);

extern int MKeywordsSize;
extern TMacroKeywords MKeywords[];
extern int _macro_nErr;


#endif
