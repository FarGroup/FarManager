/*
syntax.cpp

Реализация парсера для MacroDrive II

*/

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------
// Парсер и "выполнятор" выражений
//---------------------------------------------------------------


#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "macroopcode.hpp"
#include "lang.hpp"
#include "fn.hpp"
#include "syntax.hpp"

#define EOFCH 256

//---------------------------------------------------------------
// Реализация класса TVar ("кастрированый" вариант - только целое
// и строковое значение)
//---------------------------------------------------------------

static const char *toString(__int64 num)
{
  static char str[128];
  _i64toa(num, str, 10);
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

TVar::TVar(const char *v) :
  vType(vtString),
  inum(0)
{
  str = new char[strlen(v)+1];
  if ( str )
    strcpy(str, v);
};

TVar::TVar(const TVar& v) :
  vType(v.vType),
  inum(v.inum)
{
  if ( v.str )
  {
    str = new char[strlen(v.str)+1];
    if ( str )
      strcpy(str, v.str);
  }
  else
    str = NULL;
};

TVar& TVar::operator=(const TVar& v)
{
  vType = v.vType;
  inum = v.inum;
  if ( str )
    delete [] str;
  str = NULL;
  if ( v.str )
  {
    str = new char[strlen(v.str)+1];
    if ( str )
      strcpy(str, v.str);
  }
  return *this;
}

__int64 TVar::i() const
{
  return isInteger() ? inum : ( str ? _atoi64(str) : 0 );
}

const char *TVar::s() const
{
  if(isString())
    return  str ? str : "";
  return ::toString(inum);
}


const char *TVar::toString()
{
  char s[128];
  switch ( vType )
  {
    case vtInteger:
      strncpy(s, ::toString(inum),sizeof(s)-1);
      break;
    default:
      return str;
  }
  if ( str )
    delete [] str;
  str = new char[strlen(s)+1];
  if ( str )
    strcpy(str, s);
  vType = vtString;
  return str;
};

__int64 TVar::toInteger()
{
  switch ( vType )
  {
    case vtString:
      inum = str ? _atoi64(str) : 0;
      break;
  }
  vType = vtInteger;
  return inum;
};

int operator==(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum == b.inum;          break;
    case vtString:  if ( b.isString() )  r = strcmp(a.s(), b.s()) == 0; break;
  }
  return r;
};

int operator!=(const TVar& a, const TVar& b)
{
  int r = 1;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum != b.inum;          break;
    case vtString:  if ( b.isString() )  r = strcmp(a.s(), b.s()) != 0; break;
  }
  return r;
};

int operator<(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum < b.inum;           break;
    case vtString:  if ( b.isString() )  r = strcmp(a.s(), b.s()) < 0;  break;
  }
  return r;
};

int operator<=(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum <= b.inum;          break;
    case vtString:  if ( b.isString() )  r = strcmp(a.s(), b.s()) <= 0; break;
  }
  return r;
};

int operator>(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum > b.inum;           break;
    case vtString:  if ( b.isString() )  r = strcmp(a.s(), b.s()) > 0;  break;
  }
  return r;
};

int operator>=(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum >= b.inum;          break;
    case vtString:  if ( b.isString() )  r = strcmp(a.s(), b.s()) >= 0; break;
  }
  return r;
};

static TVar addStr(const char *a, const char *b)
{
  TVar r("");
  char *c = new char[strlen(a ? a : "")+strlen(b ? b : "")+1];
  if ( c )
  {
    r = strcat(strcpy(c, a ? a : ""), b ? b : "");
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

//---------------------------------------------------------------
// Работа с таблицами имен переменных
//---------------------------------------------------------------

int hash(const char *p)
{
  int i = 0;
  char *pp = (char*)p;
  while ( *pp )
    i = i << (1^*(pp++));
  if ( i < 0 )
    i = -i;
  i %= V_TABLE_SIZE;
  return i;
}

int isVar(TVarTable table, const char *p)
{
  int i = hash(p);
  for ( TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next) )
    if ( !stricmp(n->str, p) )
      return 1;
  return 0;
}

TVarSet *varLook(TVarTable table, const char *p, int& error, int ins)
{
  int i = hash(p);
  error = 0;
  for ( TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next) )
    if ( !stricmp(n->str, p) )
      return n;
  if ( !ins )
    error = 1;
  TVarSet *nn = new TVarSet(p);
  nn->next = table[i];
  table[i] = nn;
  return nn;
}

TVarSet *varEnum(TVarTable table,int NumTable, int Index)
{
  if((DWORD)NumTable >= V_TABLE_SIZE)
    return NULL;

  TVarSet *n = table[NumTable];
  for(int I=0; I < Index && n; ++I)
    n = ((TVarSet*)n->next);

  return n;
}

void varKill(TVarTable table, const char *p)
{
  int i = hash(p);
  TVarSet *nn = table[i];
  for ( TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next) )
  {
    if ( !stricmp(n->str, p) )
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
    while ( table[i] != NULL )
    {
      TVarSet *n = ((TVarSet*)(table[i]->next));
      table[i]->next = NULL;
      delete table[i];
      table[i] = n;
    }
}

static int Size = 0;
static unsigned long FARVar, *exprBuff = NULL;
static int IsProcessFunc=0;

static void put(unsigned long code)
{
  exprBuff[Size++] = code;
}

static void put64(unsigned __int64 code)
{
  FARINT64 i64;
  i64.i64=code;
  exprBuff[Size++] = i64.Part.HighPart;   //???
  exprBuff[Size++] = i64.Part.LowPart;    //???
}

static void putstr(const char *s)
{
  int Length = (int)strlen(s)+1;
  // строка должна быть выровнена на 4
  int nSize = Length/sizeof(unsigned long);
  if ( Length == 1 || ( Length % sizeof(unsigned long)) != 0 ) // дополнение до sizeof(DWORD) нулями.
    nSize++;
  for ( int i = 0 ; i < nSize ; i++ )
  {
    unsigned long d[2] = { 0, 0 };
    strncpy((char*)d, s, sizeof(unsigned long));
    s += sizeof(unsigned long);
    exprBuff[Size++] = *d;
  }
}

int _macro_nErr = 0;
int _macro_ErrCode=err_Success;
static char nameString[1024];
static char *sSrcString;
static char *pSrcString = NULL;
static char *oSrcString = NULL;

static TToken currTok = tNo;
static TVar currVar;

static void expr(void);
static __int64 _cdecl getInt64();

//-----------------------------------------------
static char ErrMessage[3][256];

BOOL GetMacroParseError(char *ErrMsg1,char *ErrMsg2,char *ErrMsg3)
{
  if(_macro_nErr)
  {
    if(ErrMsg1)
      strcpy(ErrMsg1,ErrMessage[0]);
    if(ErrMsg2)
      strcpy(ErrMsg2,ErrMessage[1]);
    if(ErrMsg3)
      strcpy(ErrMsg3,ErrMessage[2]);
    return TRUE;
  }
  return FALSE;
}


void keyMacroParseError(int err, const char *s, const char *p, const char *c)
{
  if ( !_macro_nErr++ )
  {
    _macro_ErrCode=err;
    int oPos = 0, ePos = (int)(s-p);
    ErrMessage[0][0]=ErrMessage[1][0]=ErrMessage[2][0]=0;
    if ( ePos < 0 )
    {
      xstrncpy(ErrMessage[0],MSG(MMacroPErrExpr_Expected),sizeof(ErrMessage[0])-1); // TODO!!!
      return;
    }

    sprintf(ErrMessage[0],MSG(MMacroPErrUnrecognized_keyword+err-1),c);
    if ( ePos > 61 )
    {
      oPos = ePos-50;
      strcpy(ErrMessage[1], "...");
    }
    strncat(ErrMessage[1], p+oPos,sizeof(ErrMessage[1])-1);
    if ( ErrMessage[1][61] )
      strncpy(&ErrMessage[1][61], "...",sizeof(ErrMessage[1])-62);
    int lPos = ePos-oPos+(oPos ? 3 : 0);

    InsertQuote(ErrMessage[1]);
    sprintf(ErrMessage[2],"%*s%c", lPos+1, "", '^');

    _KEYMACRO(SysLog(ErrMessage[0]));
    _KEYMACRO(SysLog(ErrMessage[1]));
    _KEYMACRO(SysLog(ErrMessage[2]));
  }
}

void keyMacroParseError(int err, const char *c = NULL) {
  keyMacroParseError(err, oSrcString, pSrcString, c);
  //                      ^ s?
}
//-----------------------------------------------

static int getNextChar()
{
  if ( *sSrcString )
  {
    int ch;
    while ( ( ( ch = *(sSrcString++) ) != 0 ) && isspace(ch) )
      ;
    return ch ? ch : EOFCH;
  }
  return EOFCH;
}

static inline int getChar()
{
  if ( *sSrcString )
  {
    int ch = *(sSrcString++);
    return ( ch ) ? ch : EOFCH;
  }
  return EOFCH;
}

typedef struct __TMacroFunction{
  const char *Name;             // имя функции
  int nParam;                   // количество параметров
  TFunction Code;               // байткод функции
} TMacroFunction;


static TMacroFunction macroFunction[]={
  {"ABS",            1,    MCODE_F_ABS},                 // N=abs(N)
  {"AKEY",           0,    MCODE_F_AKEY},                // S=akey()
  {"CHECKHOTKEY",    1,    MCODE_F_MENU_CHECKHOTKEY},    // N=checkhotkey(S)
  {"CLIP",           2,    MCODE_F_CLIP},                // V=clip(N,S)
  {"DATE",           1,    MCODE_F_DATE},                // S=date(S)
  {"DLG.GETVALUE",   2,    MCODE_F_DLG_GETVALUE},        // V=Dlg.GetValue(ID,N)
  {"EDITOR.SET",     2,    MCODE_F_EDITOR_SET},          // N=Editor.Set(N,Var)
  {"ENV",            1,    MCODE_F_ENVIRON},             // S=env(S)
  {"FATTR",          1,    MCODE_F_FATTR},               // N=fattr(S)
  {"FEXIST",         1,    MCODE_F_FEXIST},              // N=fexist(S)
  {"FSPLIT",         2,    MCODE_F_FSPLIT},              // S=fsplit(S,N)
  {"IIF",            3,    MCODE_F_IIF},                 // V=iif(Condition,V1,V2)
  {"INDEX",          2,    MCODE_F_INDEX},               // S=index(S1,S2)
  {"INT",            1,    MCODE_F_INT},                 // N=int(V)
  {"ITOA",           2,    MCODE_F_ITOA},                // S=itoa(N,radix)
  {"LCASE",          1,    MCODE_F_LCASE},               // S=lcase(S1)
  {"LEN",            1,    MCODE_F_LEN},                 // N=len(S)
  {"MAX",            2,    MCODE_F_MAX},                 // N=max(N1,N2)
  {"MSAVE",          1,    MCODE_F_MSAVE},               // N=msave(S)
  {"MSGBOX",         3,    MCODE_F_MSGBOX},              // N=msgbox("Title","Text",flags)
  {"MIN",            2,    MCODE_F_MIN},                 // N=min(N1,N2)
  {"PANEL.FATTR",    2,    MCODE_F_PANEL_FATTR},         // N=Panel.FAttr(panelType,fileMask)
  {"PANEL.FEXIST",   2,    MCODE_F_PANEL_FEXIST},        // N=Panel.FExist(panelType,fileMask)
  {"PANEL.SETPOS",   2,    MCODE_F_PANEL_SETPOS},        // N=panel.SetPos(panelType,fileName)
  {"PANELITEM",      3,    MCODE_F_PANELITEM},           // V=panelitem(Panel,Index,TypeInfo)
  {"PLAYMACRO",      1,    MCODE_F_PLAYMACRO},           // N=playmacro(S)
  {"RINDEX",         2,    MCODE_F_RINDEX},              // S=rindex(S1,S2)
  {"SLEEP",          1,    MCODE_F_SLEEP},               // N=sleep(N)
  {"STRING",         1,    MCODE_F_STRING},              // S=string(V)
  {"SUBSTR",         3,    MCODE_F_SUBSTR},              // S=substr(S,N1,N2)
  {"UCASE",          1,    MCODE_F_UCASE},               // S=ucase(S1)
  {"WAITKEY",        1,    MCODE_F_WAITKEY},             // S=waitkey(N)
  {"XLAT",           1,    MCODE_F_XLAT},                // S=xlat(S)
};

DWORD funcLook(const char *s, int& nParam)
{
  nParam=0;
  for(int I=0; I < sizeof(macroFunction)/sizeof(macroFunction[0]); ++I)
    //if(!strnicmp(s, macroFunction[I].Name, strlen(macroFunction[I].Name)))
    if(!strnicmp(s, macroFunction[I].Name, Max(strlen(macroFunction[I].Name),strlen(s))))
    {
      nParam = macroFunction[I].nParam;
      return (DWORD)macroFunction[I].Code;
    }

  return (DWORD)MCODE_F_NOFUNC;
}

static TToken getToken(void);

static void calcFunc(void)
{
  int nParam;
  TFunction nFunc = (TFunction)funcLook(nameString, nParam);
  if ( nFunc != MCODE_F_NOFUNC )
  {
    IsProcessFunc++;
    if ( nParam )
    {
      for ( int i = 0 ; i < nParam ; i++ )
      {
        getToken();
        expr();
        if ( currTok != ( (i == nParam-1) ? tRp : tComma ) )
        {
          if ( i == nParam-1 )
            keyMacroParseError(err_Expected, ")");
          else
            keyMacroParseError(err_Expected, ",");
          currTok = tEnd;
        }
       }
    }
    else
    {
      getToken();
      if ( currTok != tRp )
      {
        keyMacroParseError(err_Expected, ")");
        currTok = tEnd;
      }
    }
    put(nFunc);
    IsProcessFunc--;
  }
  else if(currTok == tFunc)
  {
    keyMacroParseError(err_Unrecognized_function, nameString);
  }
}

static void getVarName(int& ch)
{
  char* p = nameString;
  *p++ = (char)ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && ( isalnum(ch) || ( ch == '_') ) )
    *p++ = (char)ch;
  *p = 0;
}

static void getFarName(int& ch)
{
  char* p = nameString;
  *p++ = (char)ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && ( isalnum(ch) || ( ch == '_') || ( ch == '.') ) )
    *p++ = (char)ch;
  *p = 0;
}

static char *putBack(int ch)
{
  if ( ( ch && ( ch != EOFCH ) ) && ( sSrcString > pSrcString ) )
    sSrcString--;
  return sSrcString;
}

static inline int peekChar()
{
  int c;
  putBack(c = getChar());
  return c;
}

static long getLong()
{
  static char buffer[32];
  char *p = buffer;
  int ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && (isxdigit(ch) || ch == 'x') && ( (p-buffer) < 32 ))
    *p++ = (char)ch;
  *p = 0;
  putBack(ch);
  char *endptr;
  return strtol(buffer,&endptr,0);
}

static __int64 _cdecl getInt64()
{
  static char buffer[128];
  char *p = buffer;
  int ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && (isxdigit(ch) || ch == 'x') && ( (p-buffer) < 32 ))
    *p++ = (char)ch;
  *p = 0;
  putBack(ch);
  char *endptr;
  __int64 __val=_strtoi64(buffer,&endptr,0);
  return __val;
}


static char hex2ch(char b1, char b2)
{
  if ( b1 >= '0' && b1 <= '9' )
    b1 -= '0';
  else
  {
    b1 &= ~0x20;
    b1 -= (char)('A'-10);
  }
  if ( b2 >= '0' && b2 <= '9')
    b2 -= '0';
  else
  {
    b2 &= ~0x20;
    b2 -= (char)('A'-10);
  }
  return (char)( ( ( b1 << 4 ) & 0x00F0 ) | ( b2 & 0x000F ) );
}

static TToken getToken(void)
{
  oSrcString = sSrcString;
  int ch = getNextChar();
  switch ( ch )
  {
    case EOFCH:
    case 0:   return currTok = tEnd;
    case ',': return currTok = tComma;
    case '+': return currTok = tPlus;
    case '-': return currTok = tMinus;
    case '*': return currTok = tMul;
    case '/': return currTok = tDiv;
    case '(': return currTok = tLp;
    case ')': return currTok = tRp;
    case '^': return currTok = tBitXor;
    case '|':
      if ( ( ch = getChar() ) == '|')
        return currTok = tBoolOr;
      else
      {
        putBack(ch);
        return currTok = tBitOr;
      }
    case '&':
      if ( ( ch = getChar() ) == '&')
        return currTok = tBoolAnd;
      else
      {
        putBack(ch);
        return currTok = tBitAnd;
      }
    case '=':
      if ( ( ch = getChar() ) == '=')
        return currTok = tEq;
      else
      {
        putBack(ch);
        return currTok = tLet;
      }
    case '>':
      switch ( ( ch = getChar() ) )
      {
        case '=': return currTok = tGe;
        case '>': return currTok = tBitShr;
        default:
          putBack(ch);
          return currTok = tGt;
      }
    case '<':
      switch ( ch = getChar() )
      {
        case '=': return currTok = tLe;
        case '<': return currTok = tBitShl;
        default:
          putBack(ch);
          return currTok = tLt;
      }
    case '!':
      if((ch = getChar() ) != '=')
      {
        putBack(ch);
        return currTok = tNot;
      }
      return currTok = tNe;
    case '\"':
      //-AN----------------------------------------------
      // Вообще-то это почти полный аналог ParsePlainText
      //-AN----------------------------------------------
      currVar = "";
      while ( ( ( ch = getChar() ) != EOFCH ) && ( ch != '\"' ) )
      {
        if ( ch == '\\' )
        {
          switch ( ch = getChar() )
          {
            case 'a' : ch = '\a'; break;
            case 'b' : ch = '\b'; break;
            case 'f' : ch = '\f'; break;
            case 'n' : ch = '\n'; break;
            case 'r' : ch = '\r'; break;
            case 't' : ch = '\t'; break;
            case 'v' : ch = '\v'; break;
            case '\'': ch = '\''; break;
            case '\"': ch = '\"'; break;
            case '\\': ch = '\\'; break;
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': // octal: \d \dd \ddd
            {
              BYTE n = ch - '0';
              if ((ch = getChar()) >= '0' && ch < '8')
              {
                n = 8 * n + ch - '0';
                if ((ch = getChar()) >= '0' && ch < '8')
                  n = 8 * n + ch - '0';
                else
                  putBack(ch);
              }
              else
                putBack(ch);
              ch = n;
              break;
            }
            case 'x':
              if ( isxdigit(ch = getChar()) )
              {
                char hBuf[3] = { (char)ch, 0, 0 };
                if ( isxdigit(ch = getChar()) )
                  hBuf[1] = (char)ch;
                else
                {
                  hBuf[1] = hBuf[0];
                  hBuf[0] = '0';
                  putBack(ch);
                }
                ch = hex2ch(hBuf[0], hBuf[1]);
              }
              else
              {
                keyMacroParseError(err_Bad_Hex_Control_Char);
                return currTok = tEnd;
              }
              break;
            default:
              keyMacroParseError(err_Bad_Control_Char);
              return currTok = tEnd;
          }
        }
        char p[] = " ";
        *p = (char)ch;
        currVar = currVar+TVar(p);
      }
      return currTok = tStr;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
      putBack(ch);
      currVar = getInt64();
      return currTok = tInt;
    }
    case '%':
      ch = getChar();
      if ( (isalpha(ch) || ch == '_') || ( ch == '%'  && (isalpha(*sSrcString) || *sSrcString == '_')))
      {
        getVarName(ch);
        putBack(ch);
        return currTok = tVar;
      }
      else
        keyMacroParseError(err_Var_Expected,"");//nameString); // BUG nameString
      break;
    default:
      if ( isalpha(ch) ) //  || ch == '_' ????
      {
        getFarName(ch);
        if(ch == ' ')
        {
          while(ch == ' ')
            ch = getNextChar();
        }
        if ( ch == '(' ) //!!!! а пробелы пропустить? ДА!
          return currTok = tFunc;
        else
        {
          putBack(ch);
          for ( int i = 0 ; i < MKeywordsSize ; i++ )
            if ( !stricmp(nameString, MKeywords[i].Name) )
            {
              FARVar = MKeywords[i].Value;
              return currTok = tFARVar;
            }
          if(IsProcessFunc || currTok == tFunc || currTok == tLt) // TODO: уточнить
            keyMacroParseError(err_Var_Expected,oSrcString,pSrcString,nameString);
          else if(KeyNameToKey(nameString) == -1)
            keyMacroParseError(err_Unrecognized_keyword,nameString);
        }
      }
      break;
  }
  return currTok = tEnd;
}

static void prim(void)
{
  switch ( currTok )
  {
    case tEnd:
      break;
    case tFunc:
      calcFunc();
      getToken();
      break;
    case tVar:
      put(MCODE_OP_PUSHVAR);
      putstr(nameString);
      getToken();
      break;
    case tInt:
      put(MCODE_OP_PUSHINT);
      put64(currVar.i());
      getToken();
      break;
    case tFARVar:
      put(FARVar);
      getToken();
      break;
    case tStr:
      put(MCODE_OP_PUSHSTR);
      putstr(currVar.s());
      getToken();
      break;
    case tMinus:
      getToken();
      prim();
      put(MCODE_OP_NEGATE);
      break;
    case tNot:
      getToken();
      prim();
      put(MCODE_OP_NOT);
      break;
    case tLp:
      getToken();
      expr();
      if ( currTok != tRp )
        keyMacroParseError(err_Expected, ")");
      getToken();
      break;
    default:
      keyMacroParseError(err_Expr_Expected);
      break;
  }
}

static void term(void)
{
  prim();
  for ( ; ; )
    switch ( currTok )
    {
      case tMul: getToken(); prim(); put(MCODE_OP_MUL); break;
      case tDiv: getToken(); prim(); put(MCODE_OP_DIV); break;
      default:
        return;
    }
}

static void mathExpr(void)
{
  term();
  for ( ; ; )
    switch ( currTok )
    {
      case tPlus:    getToken(); term(); put(MCODE_OP_ADD); break;
      case tMinus:   getToken(); term(); put(MCODE_OP_SUB); break;
      case tBitShl:  getToken(); term(); put(MCODE_OP_BITSHL);  break;
      case tBitShr:  getToken(); term(); put(MCODE_OP_BITSHR);  break;
      default:
        return;
    }
}

static void booleanPrim(void)
{
  mathExpr();
  for ( ; ; )
    switch ( currTok )
    {
      case tLt: getToken(); mathExpr(); put(MCODE_OP_LT); break;
      case tLe: getToken(); mathExpr(); put(MCODE_OP_LE); break;
      case tGt: getToken(); mathExpr(); put(MCODE_OP_GT); break;
      case tGe: getToken(); mathExpr(); put(MCODE_OP_GE); break;
      case tEq: getToken(); mathExpr(); put(MCODE_OP_EQ); break;
      case tNe: getToken(); mathExpr(); put(MCODE_OP_NE); break;
      default:
        return;
    }
}

static void expr(void)
{
  booleanPrim();
  for ( ; ; )
    switch ( currTok )
    {
      case tBoolAnd: getToken(); booleanPrim(); put(MCODE_OP_AND);    break;
      case tBoolOr:  getToken(); booleanPrim(); put(MCODE_OP_OR);     break;
      case tBitAnd:  getToken(); booleanPrim(); put(MCODE_OP_BITAND); break;
      case tBitOr:   getToken(); booleanPrim(); put(MCODE_OP_BITOR);  break;
      case tBitXor:  getToken(); booleanPrim(); put(MCODE_OP_BITXOR);  break;
      default:
        return;
    }
}

int parseExpr(const char*& BufPtr, unsigned long *eBuff, char bound1, char bound2)
{
  char tmp[4];
  IsProcessFunc=0;
  _macro_ErrCode = Size = _macro_nErr = 0;
  while ( *BufPtr && isspace(*BufPtr) )
    BufPtr++;
  if ( bound1 )
  {
    pSrcString = oSrcString = sSrcString = (char*)BufPtr+1;
    if ( *BufPtr != bound1 )
    {
      tmp[0] = bound1;
      tmp[1] = 0;
      keyMacroParseError(err_Expected, tmp);
      return 0;
    }
  }
  else
    pSrcString = oSrcString = sSrcString = (char*)BufPtr;
  exprBuff = eBuff;
  put(MCODE_OP_EXPR);
#if !defined(TEST000)
  getToken();
  if ( bound2 )
    expr();
  else
    prim();
  put(MCODE_OP_DOIT);
  BufPtr = oSrcString;
  while ( *BufPtr && isspace(*BufPtr) )
    BufPtr++;
  if ( bound2 )
  {
    if ( ( *BufPtr != bound2 ) || !( !BufPtr[1] || isspace(BufPtr[1]) ) )
    {
      tmp[0] = bound2;
      tmp[1] = 0;
      keyMacroParseError(err_Expected, tmp);
      return 0;
    }
    BufPtr++;
  }
#else
  if ( getToken() == tEnd )
    keyMacroParseError(err_Expr_Expected);
  else
  {
    if ( bound2 )
      expr();
    else
      prim();
    put(MCODE_OP_DOIT);
    BufPtr = oSrcString;
    while ( *BufPtr && isspace(*BufPtr) )
      BufPtr++;
    if ( bound2 )
    {
      if ( ( *BufPtr != bound2 ) || !( !BufPtr[1] || isspace(BufPtr[1]) ) )
      {
        tmp[0] = bound2;
        tmp[1] = 0;
        keyMacroParseError(err_Expected, tmp);
        return 0;
      }
      BufPtr++;
    }
  }
#endif
  return Size;
}
