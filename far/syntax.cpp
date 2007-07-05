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

#define EOFCH 65536

//---------------------------------------------------------------
// Реализация класса TVar ("кастрированый" вариант - только целое
// и строковое значение)
//---------------------------------------------------------------

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
  str = new wchar_t[wcslen(v)+1];
  if ( str )
    wcscpy(str, v);
};


TVar::TVar(const TVar& v) :
  vType(v.vType),
  inum(v.inum)
{
  if ( v.str )
  {
    str = new wchar_t[wcslen(v.str)+1];
    if ( str )
      wcscpy(str, v.str);
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
    str = new wchar_t[wcslen(v.str)+1];
    if ( str )
      wcscpy(str, v.str);
  }
  return *this;
}

__int64 TVar::i() const
{
  return isInteger() ? inum : ( str ? _wtoi64(str) : 0 );
}

const wchar_t *TVar::s() const
{
  if(isString())
    return  str ? str : L"";
  return ::toString(inum);
}


const wchar_t *TVar::toString()
{
  wchar_t s[128];
  switch ( vType )
  {
    case vtInteger:
      wcsncpy(s, ::toString(inum),(sizeof(s)-1)/sizeof (wchar_t));
      break;
    default:
      return str;
  }
  if ( str )
    delete [] str;
  str = new wchar_t[wcslen(s)+1];
  if ( str )
    wcscpy(str, s);
  vType = vtString;
  return str;
};

__int64 TVar::toInteger()
{
  switch ( vType )
  {
    case vtString:
      inum = str ? _wtoi64(str) : 0;
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
    case vtString:  if ( b.isString() )  r = wcscmp(a.s(), b.s()) == 0; break;
  }
  return r;
};

int operator!=(const TVar& a, const TVar& b)
{
  int r = 1;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum != b.inum;          break;
    case vtString:  if ( b.isString() )  r = wcscmp(a.s(), b.s()) != 0; break;
  }
  return r;
};

int operator<(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum < b.inum;           break;
    case vtString:  if ( b.isString() )  r = wcscmp(a.s(), b.s()) < 0;  break;
  }
  return r;
};

int operator<=(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum <= b.inum;          break;
    case vtString:  if ( b.isString() )  r = wcscmp(a.s(), b.s()) <= 0; break;
  }
  return r;
};

int operator>(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum > b.inum;           break;
    case vtString:  if ( b.isString() )  r = wcscmp(a.s(), b.s()) > 0;  break;
  }
  return r;
};

int operator>=(const TVar& a, const TVar& b)
{
  int r = 0;
  switch ( a.vType )
  {
    case vtInteger: if ( b.isInteger() ) r = a.inum >= b.inum;          break;
    case vtString:  if ( b.isString() )  r = wcscmp(a.s(), b.s()) >= 0; break;
  }
  return r;
};

static TVar addStr(const wchar_t *a, const wchar_t *b)
{
  TVar r(L"");
  wchar_t *c = new wchar_t[wcslen(a ? a : L"")+wcslen(b ? b : L"")+1];
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
    if ( !LocalStricmpW(n->str, p) )
      return 1;
  return 0;
}

TVarSet *varLook(TVarTable table, const wchar_t *p, int& error, int ins)
{
  int i = hash(p);
  error = 0;
  for ( TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next) )
    if ( !LocalStricmpW(n->str, p) )
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

void varKill(TVarTable table, const wchar_t *p)
{
  int i = hash(p);
  TVarSet *nn = table[i];
  for ( TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next) )
  {
    if ( !LocalStricmpW(n->str, p) )
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

static void putstr(const wchar_t *s)
{
  _KEYMACRO(CleverSysLog Clev(L"putstr"));
  _KEYMACRO(SysLog(L"s[%p]='%s'", s,s));

  int Length = (int)(wcslen(s)+1)*sizeof(wchar_t);
  // строка должна быть выровнена на 4
  int nSize = Length/sizeof(DWORD);
  memmove(&exprBuff[Size],s,Length);
  if ( Length == sizeof(wchar_t) || ( Length % sizeof(DWORD)) != 0 ) // дополнение до sizeof(DWORD) нулями.
    nSize++;
  memset(&exprBuff[Size],0,nSize*sizeof(DWORD));
  memmove(&exprBuff[Size],s,Length);
  Size+=nSize;
}

int _macro_nErr = 0;
int _macro_ErrCode=err_Success;
static wchar_t nameString[1024];
static wchar_t *sSrcString;
static wchar_t *pSrcString = NULL;
static wchar_t *oSrcString = NULL;

static TToken currTok = tNo;
static TVar currVar;

static void expr(void);
static __int64 _cdecl getInt64();

//-----------------------------------------------
static string ErrMessage[3];

BOOL GetMacroParseError(string *strErrMsg1,string *strErrMsg2,string *strErrMsg3)
{
  if(_macro_nErr)
  {
    if(strErrMsg1)
      *strErrMsg1 = ErrMessage[0];
    if(strErrMsg2)
      *strErrMsg2 = ErrMessage[1];
    if(strErrMsg3)
      *strErrMsg3 = ErrMessage[2];
    return TRUE;
  }
  return FALSE;
}


void keyMacroParseError(int err, const wchar_t *s, const wchar_t *p, const wchar_t *c)
{
  if ( !_macro_nErr++ )
  {
    _macro_ErrCode=err;
    int oPos = 0, ePos = (int)(s-p);
    ErrMessage[0]=ErrMessage[1]=ErrMessage[2]=L"";
    if ( ePos < 0 )
    {
      ErrMessage[0] = UMSG(MMacroPErrExpr_Expected); // TODO: .Format !
      return;
    }

    ErrMessage[0].Format (UMSG(MMacroPErrUnrecognized_keyword+err-1),c);
    if ( ePos > 61 )
    {
      oPos = ePos-50;
      ErrMessage[1] = L"...";
    }
    ErrMessage[1] += p+oPos;

//    if ( ErrMessage[1][61] ) BUGBUG
//      strncpy(&ErrMessage[1][61], "...",sizeof(ErrMessage[1])-62);

    int lPos = ePos-oPos+(oPos ? 3 : 0);

    InsertQuote(ErrMessage[1]);
    ErrMessage[2].Format (L"%*s%c", lPos+1, L"", L'^');
  }
}

void keyMacroParseError(int err, const wchar_t *c = NULL)
{
  keyMacroParseError(err, oSrcString, pSrcString, c);
  //                      ^ s?
}
//-----------------------------------------------

static int getNextChar()
{
  if ( *sSrcString )
  {
    int ch;
    while ( ( ( ch = *(sSrcString++) ) != 0 ) && iswspace(ch) )
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
  const wchar_t *Name;             // имя функции
  int nParam;                   // количество параметров
  TFunction Code;               // байткод функции
} TMacroFunction;

static TMacroFunction macroFunction[]={
  {L"ABS",            1,    MCODE_F_ABS},                 // N=abs(N)
  {L"AKEY",           0,    MCODE_F_AKEY},                // S=akey()
  {L"ASC",            1,    MCODE_F_ASC},                 // N=asc(N)
  {L"CHECKHOTKEY",    1,    MCODE_F_MENU_CHECKHOTKEY},    // N=checkhotkey(S)
  {L"CHR",            1,    MCODE_F_CHR},                 // S=chr(N)
  {L"CLIP",           2,    MCODE_F_CLIP},                // V=clip(N,S)
  {L"DATE",           1,    MCODE_F_DATE},                // S=date(S)
  {L"DLG.GETVALUE",   2,    MCODE_F_DLG_GETVALUE},        // V=Dlg.GetValue(ID,N)
  {L"EDITOR.SET",     2,    MCODE_F_EDITOR_SET},          // N=Editor.Set(N,Var)
  {L"ENV",            1,    MCODE_F_ENVIRON},             // S=env(S)
  {L"EVAL",           1,    MCODE_F_EVAL},                // N=eval(S)
  {L"FATTR",          1,    MCODE_F_FATTR},               // N=fattr(S)
  {L"FEXIST",         1,    MCODE_F_FEXIST},              // N=fexist(S)
  {L"FSPLIT",         2,    MCODE_F_FSPLIT},              // S=fsplit(S,N)
  {L"GETHOTKEY",      1,    MCODE_F_MENU_GETHOTKEY},      // S=gethotkey(N)
  {L"IIF",            3,    MCODE_F_IIF},                 // V=iif(Condition,V1,V2)
  {L"INDEX",          2,    MCODE_F_INDEX},               // S=index(S1,S2)
  {L"INT",            1,    MCODE_F_INT},                 // N=int(V)
  {L"ITOA",           2,    MCODE_F_ITOA},                // S=itoa(N,radix)
  {L"LCASE",          1,    MCODE_F_LCASE},               // S=lcase(S1)
  {L"LEN",            1,    MCODE_F_LEN},                 // N=len(S)
  {L"MAX",            2,    MCODE_F_MAX},                 // N=max(N1,N2)
  {L"MSAVE",          1,    MCODE_F_MSAVE},               // N=msave(S)
  {L"MSGBOX",         3,    MCODE_F_MSGBOX},              // N=msgbox("Title","Text",flags)
  {L"MIN",            2,    MCODE_F_MIN},                 // N=min(N1,N2)
  {L"PANEL.FATTR",    2,    MCODE_F_PANEL_FATTR},         // N=Panel.FAttr(panelType,fileMask)
  {L"PANEL.FEXIST",   2,    MCODE_F_PANEL_FEXIST},        // N=Panel.FExist(panelType,fileMask)
  {L"PANEL.SETPOS",   2,    MCODE_F_PANEL_SETPOS},        // N=panel.SetPos(panelType,fileName)
  {L"PANEL.SETPOSIDX",2,    MCODE_F_PANEL_SETPOSIDX},     // N=Panel.SetPosIdx(panelType,Idx)
  {L"PANELITEM",      3,    MCODE_F_PANELITEM},           // V=panelitem(Panel,Index,TypeInfo)
  {L"RINDEX",         2,    MCODE_F_RINDEX},              // S=rindex(S1,S2)
  {L"SLEEP",          1,    MCODE_F_SLEEP},               // N=sleep(N)
  {L"STRING",         1,    MCODE_F_STRING},              // S=string(V)
  {L"SUBSTR",         3,    MCODE_F_SUBSTR},              // S=substr(S,N1,N2)
  {L"UCASE",          1,    MCODE_F_UCASE},               // S=ucase(S1)
  {L"WAITKEY",        1,    MCODE_F_WAITKEY},             // S=waitkey(N)
  {L"XLAT",           1,    MCODE_F_XLAT},                // S=xlat(S)
};

DWORD funcLook(const wchar_t *s, int& nParam)
{
  nParam=0;
  for(int I=0; I < sizeof(macroFunction)/sizeof(macroFunction[0]); ++I)
    //if(!strnicmp(s, macroFunction[I].Name, strlen(macroFunction[I].Name)))
    if(!LocalStrnicmpW(s, macroFunction[I].Name, (int)Max(wcslen(macroFunction[I].Name),wcslen(s))))
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
            keyMacroParseError(err_Expected, L")");
          else
            keyMacroParseError(err_Expected, L",");
          currTok = tEnd;
        }
       }
    }
    else
    {
      getToken();
      if ( currTok != tRp )
      {
        keyMacroParseError(err_Expected, L")");
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
  wchar_t* p = nameString;
  *p++ = (wchar_t)ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && ( iswalnum(ch) || ( ch == L'_') ) )
    *p++ = (wchar_t)ch;
  *p = 0;
}

static void getFarName(int& ch)
{
  wchar_t* p = nameString;
  *p++ = (wchar_t)ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && ( iswalnum(ch) || ( ch == L'_') || ( ch == L'.') ) )
    *p++ = (wchar_t)ch;
  *p = 0;
}

static wchar_t *putBack(int ch)
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
  static wchar_t buffer[32];
  wchar_t *p = buffer;
  int ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && (iswxdigit(ch) || ch == L'x') && ( (p-buffer) < 32 ))
    *p++ = (wchar_t)ch;
  *p = 0;
  putBack(ch);
  wchar_t *endptr;
  return wcstol(buffer,&endptr,0);
}


static __int64 _cdecl getInt64()
{
  static wchar_t buffer[128];
  wchar_t *p = buffer;
  int ch;
  while ( ( ( ch = getChar() ) != EOFCH ) && (iswxdigit(ch) || ch == 'x') && ( (p-buffer) < 32 ))
    *p++ = (char)ch;
  *p = 0;
  putBack(ch);
  wchar_t *endptr;
  __int64 __val=_wcstoi64(buffer,&endptr,0);
  return __val;
}

static wchar_t hex2ch(wchar_t b1, wchar_t b2)
{
  if ( b1 >= L'0' && b1 <= L'9' )
    b1 -= L'0';
  else
  {
    b1 &= ~0x20;
    b1 -= (wchar_t)(L'A'-10);
  }
  if ( b2 >= L'0' && b2 <= L'9')
    b2 -= L'0';
  else
  {
    b2 &= ~0x20;
    b2 -= (wchar_t)(L'A'-10);
  }
  return (wchar_t)( ( ( b1 << 4 ) & 0x00F0 ) | ( b2 & 0x000F ) );
}

static TToken getToken(void)
{
  oSrcString = sSrcString;
  int ch = getNextChar();
  switch ( ch )
  {
    case EOFCH:
    case 0:   return currTok = tEnd;
    case L',': return currTok = tComma;
    case L'+': return currTok = tPlus;
    case L'-': return currTok = tMinus;
    case L'*': return currTok = tMul;
    case L'/': return currTok = tDiv;
    case L'(': return currTok = tLp;
    case L')': return currTok = tRp;
    case L'^': return currTok = tBitXor;
    case L'|':
      if ( ( ch = getChar() ) == L'|')
        return currTok = tBoolOr;
      else
      {
        putBack(ch);
        return currTok = tBitOr;
      }
    case L'&':
      if ( ( ch = getChar() ) == L'&')
        return currTok = tBoolAnd;
      else
      {
        putBack(ch);
        return currTok = tBitAnd;
      }
    case L'=':
      if ( ( ch = getChar() ) == L'=')
        return currTok = tEq;
      else
      {
        putBack(ch);
        return currTok = tLet;
      }
    case L'>':
      switch ( ( ch = getChar() ) )
      {
        case L'=': return currTok = tGe;
        case L'>': return currTok = tBitShr;
        default:
          putBack(ch);
          return currTok = tGt;
      }
    case L'<':
      switch ( ch = getChar() )
      {
        case L'=': return currTok = tLe;
        case L'<': return currTok = tBitShl;
        default:
          putBack(ch);
          return currTok = tLt;
      }
    case L'!':
      if((ch = getChar() ) != L'=')
      {
        putBack(ch);
        return currTok = tNot;
      }
      return currTok = tNe;
    case L'\"':
      //-AN----------------------------------------------
      // Вообще-то это почти полный аналог ParsePlainText
      //-AN----------------------------------------------
      currVar = L"";
      while ( ( ( ch = getChar() ) != EOFCH ) && ( ch != L'\"' ) )
      {
        if ( ch == L'\\' )
        {
          switch ( ch = getChar() )
          {
            case L'a' : ch = L'\a'; break;
            case L'b' : ch = L'\b'; break;
            case L'f' : ch = L'\f'; break;
            case L'n' : ch = L'\n'; break;
            case L'r' : ch = L'\r'; break;
            case L't' : ch = L'\t'; break;
            case L'v' : ch = L'\v'; break;
            case L'\'': ch = L'\''; break;
            case L'\"': ch = L'\"'; break;
            case L'\\': ch = L'\\'; break;
            case L'0': case L'1': case L'2': case L'3': case L'4': case L'5': case L'6': case L'7': // octal: \d \dd \ddd
            {
              BYTE n = ch - L'0';
              if ((ch = getChar()) >= L'0' && ch < L'8')
              {
                n = 8 * n + ch - L'0';
                if ((ch = getChar()) >= L'0' && ch < L'8')
                  n = 8 * n + ch - L'0';
                else
                  putBack(ch);
              }
              else
                putBack(ch);
              ch = n;
              break;
            }
            case L'x':
              if ( iswxdigit(ch = getChar()) )
              {
                wchar_t hBuf[3] = { static_cast<wchar_t>(ch), 0, 0 };
                if ( iswxdigit(ch = getChar()) )
                  hBuf[1] = static_cast<wchar_t>(ch);
                else
                {
                  hBuf[1] = hBuf[0];
                  hBuf[0] = L'0';
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
        wchar_t p[] = L" ";
        *p = (wchar_t)ch;
        currVar = currVar+TVar(p);
      }
      return currTok = tStr;
    case L'0': case L'1': case L'2': case L'3': case L'4':
    case L'5': case L'6': case L'7': case L'8': case L'9':
      putBack(ch);
      currVar = getInt64();
      return currTok = tInt;
    case L'%':
      ch = getChar();
      if ( (LocalIsalphanumW(ch) || ch == L'_') || ( ch == L'%'  && (LocalIsalphanumW(*sSrcString) || *sSrcString == L'_')))
      {
        getVarName(ch);
        putBack(ch);
        return currTok = tVar;
      }
      else
        keyMacroParseError(err_Var_Expected,L""); // BUG nameString
      break;
    default:
      if ( LocalIsalphaW(ch) ) // || ch == L'_' ????
      {
        getFarName(ch);
        if(ch == L' ')
        {
          while(ch == L' ')
            ch = getNextChar();
        }
        if ( ch == L'(' ) //!!!! а пробелы пропустить? ДА!
          return currTok = tFunc;
        else
        {
          putBack(ch);
          for ( int i = 0 ; i < MKeywordsSize ; i++ )
            if ( !LocalStricmpW(nameString, MKeywords[i].Name) )
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
        keyMacroParseError(err_Expected, L")");
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
      case tPlus:    getToken(); term(); put(MCODE_OP_ADD);     break;
      case tMinus:   getToken(); term(); put(MCODE_OP_SUB);     break;
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

int parseExpr(const wchar_t*& BufPtr, unsigned long *eBuff, wchar_t bound1, wchar_t bound2)
{
  wchar_t tmp[4];
  IsProcessFunc=0;
  _macro_ErrCode = Size = _macro_nErr = 0;
  while ( *BufPtr && iswspace(*BufPtr) )
    BufPtr++;
  if ( bound1 )
  {
    pSrcString = oSrcString = sSrcString = (wchar_t*)BufPtr+1;
    if ( *BufPtr != bound1 )
    {
      tmp[0] = bound1;
      tmp[1] = 0;
      keyMacroParseError(err_Expected, tmp);
      return 0;
    }
  }
  else
    pSrcString = oSrcString = sSrcString = (wchar_t*)BufPtr;
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
  while ( *BufPtr && iswspace(*BufPtr) )
    BufPtr++;
  if ( bound2 )
  {
    if ( ( *BufPtr != bound2 ) || !( !BufPtr[1] || iswspace(BufPtr[1]) ) )
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
    while ( *BufPtr && iswspace(*BufPtr) )
      BufPtr++;
    if ( bound2 )
    {
      if ( ( *BufPtr != bound2 ) || !( !BufPtr[1] || iswspace(BufPtr[1]) ) )
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
