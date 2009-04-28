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
#include "tvar.hpp"

#define EOFCH 256

static int exprBuffSize = 0;
static unsigned long nFARVar, *exprBuff = NULL;
static int IsProcessFunc=0;

static int _macro_nErr = 0;
static int _macro_ErrCode=err_Success;
static char nameString[1024];
static char *sSrcString;
static char *pSrcString = NULL;
static char *oSrcString = NULL;
static char ErrMessage[3][256];

static TToken currTok = tNo;
static TVar currVar;


static void keyMacroParseError(int err, const char *s, const char *p, const char *c = NULL);
static void keyMacroParseError(int err, const char *c = NULL);
static void expr(void);
static __int64 _cdecl getInt64();

#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
static void printKeyValue(DWORD* k, int& i);
#endif
#endif

// Стек структурных операторов
enum TExecMode
{
  emmMain, emmWhile, emmThen, emmElse, emmRep
};

struct TExecItem
{
  TExecMode state;
  DWORD pos1, pos2;
};

class TExec
{
  private:
    TExecItem stack[MAXEXEXSTACK];
  public:
    int current;
    void init()
    {
      current = 0;
      stack[current].state = emmMain;
      stack[current].pos1 = stack[current].pos2 = 0;
    }
    TExec() { init(); }
    TExecItem& operator()() { return stack[current]; }
    int add(TExecMode, DWORD, DWORD = 0);
    int del();
};

int TExec::add(TExecMode s, DWORD p1, DWORD p2)
{
  if ( ++current < MAXEXEXSTACK )
  {
    stack[current].state = s;
    stack[current].pos1 = p1;
    stack[current].pos2 = p2;
    return TRUE;
  }
  // Stack Overflow
  return FALSE;
};

int TExec::del()
{
  if ( --current < 0 )
  {
    // Stack Underflow ???
    current = 0;
    return FALSE;
  }
  return TRUE;
};


#if defined(SYSLOG)
static const char *_MacroParserToken_ToName(int Token)
{
#define DEF_TOKEN_(m) { m , #m }
  static struct TOKENName{
    int Msg;
    const char *Name;
  } TOKEN[]={
    DEF_TOKEN_(tNo),
    DEF_TOKEN_(tEnd),
    DEF_TOKEN_(tLet),
    DEF_TOKEN_(tVar),
    DEF_TOKEN_(tConst),
    DEF_TOKEN_(tStr),
    DEF_TOKEN_(tInt),
    DEF_TOKEN_(tFunc),
    DEF_TOKEN_(tFARVar),
    DEF_TOKEN_(tPlus),
    DEF_TOKEN_(tMinus),
    DEF_TOKEN_(tMul),
    DEF_TOKEN_(tDiv),
    DEF_TOKEN_(tLp),
    DEF_TOKEN_(tRp),
    DEF_TOKEN_(tComma),
    DEF_TOKEN_(tBoolAnd),
    DEF_TOKEN_(tBoolOr),
    DEF_TOKEN_(tBitAnd),
    DEF_TOKEN_(tBitOr),
    DEF_TOKEN_(tBitXor),
    DEF_TOKEN_(tBitNot),
    DEF_TOKEN_(tNot),
    DEF_TOKEN_(tBitShl),
    DEF_TOKEN_(tBitShr),
    DEF_TOKEN_(tEq),
    DEF_TOKEN_(tNe),
    DEF_TOKEN_(tLt),
    DEF_TOKEN_(tLe),
    DEF_TOKEN_(tGt),
    DEF_TOKEN_(tGe),
  };
  int I;
  static char Name[512];
  for(I=0; I < sizeof(TOKEN)/sizeof(TOKEN[0]); ++I)
    if(TOKEN[I].Msg == Token)
    {
      sprintf(Name,"\"%s\" [%d/0x%04X]",TOKEN[I].Name,Token,Token);
      return Name;
    }
  sprintf(Name,"\"t???\" [%d/0x%04X]",Token,Token);
  return Name;
}
#endif

static void put(unsigned long code)
{
  exprBuff[exprBuffSize++] = code;
}

static void put64(unsigned __int64 code)
{
  FARINT64 i64;
  i64.i64=code;
  exprBuff[exprBuffSize++] = i64.Part.HighPart;   //???
  exprBuff[exprBuffSize++] = i64.Part.LowPart;    //???
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
    xstrncpy((char*)d, s, sizeof(unsigned long));
    s += sizeof(unsigned long);
    exprBuff[exprBuffSize++] = *d;
  }
}


static void keyMacroParseError(int err, const char *s, const char *p, const char *c)
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
    xstrncat(ErrMessage[1], p+oPos,sizeof(ErrMessage[1])-1);
    if ( ErrMessage[1][61] )
      xstrncpy(&ErrMessage[1][61], "...",sizeof(ErrMessage[1])-62);
    int lPos = ePos-oPos+(oPos ? 3 : 0);

    InsertQuote(ErrMessage[1]);
    sprintf(ErrMessage[2],"%*s%c", lPos+1, "", '^');

    _KEYMACRO(SysLog(ErrMessage[0]));
    _KEYMACRO(SysLog(ErrMessage[1]));
    _KEYMACRO(SysLog(ErrMessage[2]));
  }
}

static void keyMacroParseError(int err, const char *c)
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
  int oParam;                   // необязательные параметры
  TMacroOpCode Code;            // байткод функции
} TMacroFunction;

static TMacroFunction macroFunction[]={
  {"ABS",              1, 0,   MCODE_F_ABS},                 // N=abs(N)
  {"AKEY",             1, 0,   MCODE_F_AKEY},                // V=akey(N)
  {"ASC",              1, 0,   MCODE_F_ASC},                 // N=asc(N)
  {"ATOI",             2, 1,   MCODE_F_ATOI},                // N=atoi(S[,radix])
  {"BM.ADD",           0, 0,   MCODE_F_BM_ADD},              // N=BM.Add()
  {"BM.CLEAR",         0, 0,   MCODE_F_BM_CLEAR},            // N=BM.Clear()
  {"BM.DEL",           1, 1,   MCODE_F_BM_DEL},              // N=BM.Del([Idx]) - удаляет закладку с указанным индексом (x=0...), -1 - удаляет текущую закладку
  {"BM.GET",           2, 0,   MCODE_F_BM_GET},              // N=BM.Get(Idx,M) - возвращает координаты строки (M==0) или колонки (M==1) закладки с индексом (Idx=0...)
  {"BM.NEXT",          0, 0,   MCODE_F_BM_NEXT},             // N=BM.Next()
  {"BM.PREV",          0, 0,   MCODE_F_BM_PREV},             // N=BM.Prev()
  {"BM.STAT",          1, 1,   MCODE_F_BM_STAT},             // N=BM.Stat([N])
  {"CHECKHOTKEY",      1, 0,   MCODE_F_MENU_CHECKHOTKEY},    // N=checkhotkey(S)
  {"CALLPLUGIN",       2, 1,   MCODE_F_CALLPLUGIN},          // V=callplugin(SysID[,param])
  {"CHR",              1, 0,   MCODE_F_CHR},                 // S=chr(N)
  {"CLIP",             2, 1,   MCODE_F_CLIP},                // V=clip(N[,S])
  {"DATE",             1, 0,   MCODE_F_DATE},                // S=date(S)
  {"DLG.GETVALUE",     2, 0,   MCODE_F_DLG_GETVALUE},        // V=Dlg.GetValue(ID,N)
  {"EDITOR.SEL",       2, 1,   MCODE_F_EDITOR_SEL},          // V=Editor.Sel(Action[,Opt])
  {"EDITOR.SET",       2, 0,   MCODE_F_EDITOR_SET},          // N=Editor.Set(N,Var)
  {"ENV",              1, 0,   MCODE_F_ENVIRON},             // S=env(S)
  {"EVAL",             2, 1,   MCODE_F_EVAL},                // N=eval(S[,N])
  {"FATTR",            1, 0,   MCODE_F_FATTR},               // N=fattr(S)
  {"FEXIST",           1, 0,   MCODE_F_FEXIST},              // N=fexist(S)
  {"FLOCK",            2, 0,   MCODE_F_FLOCK},               // N=FLock(N,N)
  {"FSPLIT",           2, 0,   MCODE_F_FSPLIT},              // S=fsplit(S,N)
  {"GETHOTKEY",        1, 1,   MCODE_F_MENU_GETHOTKEY},      // S=gethotkey([N])
  {"IIF",              3, 0,   MCODE_F_IIF},                 // V=iif(Condition,V1,V2)
  {"INDEX",            2, 0,   MCODE_F_INDEX},               // S=index(S1,S2)
  {"INT",              1, 0,   MCODE_F_INT},                 // N=int(V)
  {"ITOA",             2, 1,   MCODE_F_ITOA},                // S=itoa(N[,radix])
  {"KEY",              1, 0,   MCODE_F_KEY},                 // S=key(V)
  {"LCASE",            1, 0,   MCODE_F_LCASE},               // S=lcase(S1)
  {"LEN",              1, 0,   MCODE_F_LEN},                 // N=len(S)
  {"MAX",              2, 0,   MCODE_F_MAX},                 // N=max(N1,N2)
  {"MENU.SELECT",      2, 1,   MCODE_F_MENU_SELECT},         // N=Menu.Select(S[,N])
  {"MOD",              2, 0,   MCODE_F_MOD},                 // N=mod(a,b) == a %  b
  {"MSAVE",            1, 0,   MCODE_F_MSAVE},               // N=msave(S)
  {"MSGBOX",           3, 3,   MCODE_F_MSGBOX},              // N=msgbox(["Title"[,"Text"[,flags]]])
  {"MIN",              2, 0,   MCODE_F_MIN},                 // N=min(N1,N2)
  {"PANEL.FATTR",      2, 0,   MCODE_F_PANEL_FATTR},         // N=Panel.FAttr(panelType,fileMask)
  {"PANEL.FEXIST",     2, 0,   MCODE_F_PANEL_FEXIST},        // N=Panel.FExist(panelType,fileMask)
  {"PANEL.SETPATH",    3, 1,   MCODE_F_PANEL_SETPATH},       // N=panel.SetPath(panelType,pathName[,fileName])
  {"PANEL.SETPOS",     2, 0,   MCODE_F_PANEL_SETPOS},        // N=panel.SetPos(panelType,fileName)
  {"PANEL.SETPOSIDX",  2, 0,   MCODE_F_PANEL_SETPOSIDX},     // N=Panel.SetPosIdx(panelType,Idx)
  {"PANELITEM",        3, 0,   MCODE_F_PANELITEM},           // V=panelitem(Panel,Index,TypeInfo)
  {"PROMPT",           5, 4,   MCODE_F_PROMPT},              // S=prompt("Title"[,"Prompt"[,flags[, "Src"[, "History"]]]])
  {"REG.CHECK",        3, 1,   MCODE_F_REG_CHECK},           // V=reg.check(iRoot, "Key"[, "Value"])
  {"REG.GET",          3, 1,   MCODE_F_REG_GET},             // V=reg.get(iRoot, "Key"[, "Value"])
  {"REPLACE",          4, 1,   MCODE_F_REPLACE},             // S=replace(Str,Find,Replace[,Cnt])
  {"RINDEX",           2, 0,   MCODE_F_RINDEX},              // S=rindex(S1,S2)
  {"SLEEP",            1, 0,   MCODE_F_SLEEP},               // N=sleep(N)
  {"STRING",           1, 0,   MCODE_F_STRING},              // S=string(V)
  {"SUBSTR",           3, 1,   MCODE_F_SUBSTR},              // S=substr(S,N1[,N2])
  {"TRIM",             2, 1,   MCODE_F_TRIM},                // S=trim(S[,N])
  {"UCASE",            1, 0,   MCODE_F_UCASE},               // S=ucase(S1)
  {"WAITKEY",          2, 2,   MCODE_F_WAITKEY},             // V=waitkey([N,[T]])
  {"XLAT",             1, 0,   MCODE_F_XLAT},                // S=xlat(S)
};

static DWORD funcLook(const char *s, int& nParam, int& oParam)
{
  oParam=nParam=0;
  for(int I=0; I < sizeof(macroFunction)/sizeof(macroFunction[0]); ++I)
    //if(!strnicmp(s, macroFunction[I].Name, strlen(macroFunction[I].Name)))
    if(!strnicmp(s, macroFunction[I].Name, Max(strlen(macroFunction[I].Name),strlen(s))))
    {
      nParam = macroFunction[I].nParam;
      oParam = macroFunction[I].oParam;
      return (DWORD)macroFunction[I].Code;
    }

  return (DWORD)MCODE_F_NOFUNC;
}

static TToken getToken(void);

static void calcFunc(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("calcFunc"));
  int nParam, oParam;
  TMacroOpCode nFunc = (TMacroOpCode)funcLook(nameString, nParam, oParam);
  if ( nFunc != MCODE_F_NOFUNC )
  {
    IsProcessFunc++;
    if ( nParam )
    {
      int i=0;
      if (nParam >= oParam)
      {
        for ( ; i < nParam ; i++ )
        {
          getToken();
          expr();
          _KEYMACRO_PARSE(SysLog("i=%d, nParam=%d  oParam=%d currTok=%s",i,nParam,oParam,_MacroParserToken_ToName(currTok)));
          if ( currTok != ( (i == nParam-1 ) ? tRp : tComma ) )
          {
            if ( oParam > 0 &&  currTok != tEnd) // если опциональные параметры есть и...
              break;

            _KEYMACRO_PARSE(SysLog("ERROR currTok=%s",_MacroParserToken_ToName(currTok)));
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
        expr();
      }

      if(oParam > 0)  //???
      {
        // добьем нулями опциональные параметры
        for( ; i < nParam-1; ++i)
        {
          _KEYMACRO_PARSE(SysLog("Optional params [%d] ==> currTok=%s, MCODE_OP_PUSHINT 0",i,_MacroParserToken_ToName(currTok)));
          put(MCODE_OP_PUSHINT);
          // исключение для substr
          if(nFunc == MCODE_F_SUBSTR)
            put64(_i64(-1));
          else
            put64(_i64(0));
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
  _KEYMACRO_PARSE(CleverSysLog Clev("getToken"));
  oSrcString = sSrcString;
  int ch = getNextChar();
  switch ( ch )
  {
    case EOFCH:
    case 0:   currTok = tEnd;    break;
    case ',': currTok = tComma;  break;
    case '+': currTok = tPlus;   break;
    case '-': currTok = tMinus;  break;
    case '*': currTok = tMul;    break;
    case '/': currTok = tDiv;    break;
    case '(': currTok = tLp;     break;
    case ')': currTok = tRp;     break;
    case '^': currTok = tBitXor; break;
    case '~':
      if ( ( ch = getChar() ) != ' ')
      {
        putBack(ch);
        currTok = tBitNot;
        break;
      }
      putBack(ch);   //????
      currTok = tEnd;
      break;
    case '|':
      if ( ( ch = getChar() ) == '|')
        currTok = tBoolOr;
      else
      {
        putBack(ch);
        currTok = tBitOr;
      }
      break;
    case '&':
      if ( ( ch = getChar() ) == '&')
        currTok = tBoolAnd;
      else
      {
        putBack(ch);
        currTok = tBitAnd;
      }
      break;
    case '=':
      if ( ( ch = getChar() ) == '=')
        currTok = tEq;
      else
      {
        putBack(ch);
        currTok = tLet;
      }
      break;
    case '>':
      switch ( ( ch = getChar() ) )
      {
        case '=': currTok = tGe;     break;
        case '>': currTok = tBitShr; break;
        default:
          putBack(ch);
          currTok = tGt;
          break;
      }
      break;
    case '<':
      switch ( ch = getChar() )
      {
        case '=': currTok = tLe;     break;
        case '<': currTok = tBitShl; break;
        default:
          putBack(ch);
          currTok = tLt;
          break;
      }
      break;
    case '!':
      if((ch = getChar() ) != '=')
      {
        putBack(ch);
        currTok = tNot;
        break;
      }
      else
        currTok = tNe;
      break;

    case '\"':
    {
      //-AN----------------------------------------------
      // Вообще-то это почти полный аналог ParsePlainText
      //-AN----------------------------------------------
      TToken __currTok = tNo;
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
                __currTok = tEnd;
              }
              break;
            default:
              keyMacroParseError(err_Bad_Control_Char);
              __currTok = tEnd;
              break;
          }
        }
        if(__currTok != tNo)
          break;
        char p[] = " ";
        *p = (char)ch;
        currVar = currVar+TVar(p);
      }

      if(__currTok == tNo)
        currTok = tStr;
      else
        currTok = __currTok;

      break;
    }

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
      putBack(ch);
      currVar = getInt64();
      currTok = tInt;
      break;
    }

    case '%':
      ch = getChar();
      if ( (isalnum(ch) || ch == '_') || ( ch == '%'  && (isalnum(*sSrcString) || *sSrcString == '_')))
      {
        getVarName(ch);
        putBack(ch);
        currTok = tVar;
      }
      else
      {
        _KEYMACRO_PARSE(SysLog("[%d] IsProcessFunc = %d, ch=%c",__LINE__,IsProcessFunc,ch));
        keyMacroParseError(err_Var_Expected,"");//nameString); // BUG nameString
      }
      break;

    default:
      if ( isalpha(ch) ) //  || ch == '_' ????
      {
        TToken __currTok = tNo;
        getFarName(ch);
        if(ch == ' ')
        {
          while(ch == ' ')
            ch = getNextChar();
        }
        if ( ch == '(' ) //!!!! а пробелы пропустить? ДА!
          __currTok = tFunc;
        else
        {
          putBack(ch);
          for ( int i = 0 ; i < MKeywordsSize ; i++ )
            if ( !stricmp(nameString, MKeywords[i].Name) )
            {
              nFARVar = MKeywords[i].Value;
              __currTok = tFARVar;
              break;
            }

          if(__currTok == tNo)
          {
            if(IsProcessFunc || currTok == tFunc || currTok == tLt) // TODO: уточнить
            {
              if(KeyNameMacroToKey(nameString) == -1 && KeyNameToKey(nameString) == -1 && checkMacroConst(nameString))
                 __currTok = tConst;
              else
              {
                _KEYMACRO_PARSE(SysLog("[%d] IsProcessFunc = %d, currTok=%s",__LINE__,IsProcessFunc,_MacroParserToken_ToName(currTok)));
                keyMacroParseError(err_Var_Expected,oSrcString,pSrcString,nameString);
              }
            }
            else
            {
              if(KeyNameMacroToKey(nameString) == -1)
              {
                if(KeyNameToKey(nameString) == -1)
                {
                  if(checkMacroConst(nameString))
                    __currTok = tConst;
                  else
                     keyMacroParseError(err_Unrecognized_keyword,nameString);
                }
                else
                {
                  currVar = (__int64)KeyNameToKey(nameString);
                  __currTok = tInt; //??
                }
              }
            }
          }
        }

        if(__currTok != tNo)
          currTok=__currTok;
      }
      else
        currTok = tEnd;
      break;
  }
  _KEYMACRO_PARSE(SysLog("pSrcString = %s",pSrcString));
  _KEYMACRO_PARSE(SysLog("oSrcString = %s",oSrcString));
  _KEYMACRO_PARSE(SysLog("sSrcString = %s",sSrcString));
  _KEYMACRO_PARSE(SysLog("currTok=%s",_MacroParserToken_ToName(currTok)));
  return currTok;
}

static void prim(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("prim"));
  _KEYMACRO_PARSE(SysLog("currTok=%s",_MacroParserToken_ToName(currTok)));
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
    case tConst:
      put(MCODE_OP_PUSHCONST);
      putstr(nameString);
      getToken();
      break;
    case tInt:
      put(MCODE_OP_PUSHINT);
      put64(currVar.i());
      getToken();
      break;
    case tFARVar:
      put(nFARVar); // nFARVar получаем в getToken()
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
    case tBitNot:
      getToken();
      prim();
      put(MCODE_OP_BITNOT);
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
    case tRp:
      break;

    default:
      keyMacroParseError(err_Expr_Expected);
      break;
  }
}

static void multExpr(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("multExpr"));
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

static void additionExpr(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("additionExpr"));
  multExpr();
  for ( ; ; )
    switch ( currTok )
    {
      case tPlus:    getToken(); multExpr(); put(MCODE_OP_ADD); break;
      case tMinus:   getToken(); multExpr(); put(MCODE_OP_SUB); break;
      default:
        return;
    }
}

static void shiftExpr(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("shiftExpr"));
  additionExpr();
  for ( ; ; )
    switch ( currTok )
    {
      case tBitShl:  getToken(); additionExpr(); put(MCODE_OP_BITSHL);  break;
      case tBitShr:  getToken(); additionExpr(); put(MCODE_OP_BITSHR);  break;
      default:
        return;
    }
}

static void relation2Expr(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("relation2Expr"));
  shiftExpr();
  for ( ; ; )
    switch ( currTok )
    {
      case tLt: getToken(); shiftExpr(); put(MCODE_OP_LT); break;
      case tLe: getToken(); shiftExpr(); put(MCODE_OP_LE); break;
      case tGt: getToken(); shiftExpr(); put(MCODE_OP_GT); break;
      case tGe: getToken(); shiftExpr(); put(MCODE_OP_GE); break;
      default:
        return;
    }
}

static void relationExpr(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("relationExpr"));
  relation2Expr();
  for ( ; ; )
    switch ( currTok )
    {
      case tEq: getToken(); relation2Expr(); put(MCODE_OP_EQ); break;
      case tNe: getToken(); relation2Expr(); put(MCODE_OP_NE); break;
      default:
        return;
    }
}

static void bitAndPrim(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("bitAndPrim"));
  relationExpr();
  for ( ; ; )
    switch ( currTok )
    {
      case tBitAnd:  getToken(); relationExpr(); put(MCODE_OP_BITAND); break;
      default:
        return;
    }
}

static void bitXorPrim(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("bitXorPrim"));
  bitAndPrim();
  for ( ; ; )
    switch ( currTok )
    {
      case tBitXor:  getToken(); bitAndPrim(); put(MCODE_OP_BITXOR);  break;
      default:
        return;
    }
}

static void bitOrPrim(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("bitOrPrim"));
  bitXorPrim();
  for ( ; ; )
    switch ( currTok )
    {
      case tBitOr:   getToken(); bitXorPrim(); put(MCODE_OP_BITOR);  break;
      default:
        return;
    }
}

static void boolAndPrim(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("boolAndPrim"));
  bitOrPrim();
  for ( ; ; )
    switch ( currTok )
    {
      case tBoolAnd: getToken(); bitOrPrim(); put(MCODE_OP_AND);    break;
      default:
        return;
    }
}

static void expr(void)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("expr"));
  boolAndPrim();
  for ( ; ; )
    switch ( currTok )
    {
      case tBoolOr:  getToken(); boolAndPrim(); put(MCODE_OP_OR);     break;
      default:
        return;
    }
}

static int parseExpr(const char*& BufPtr, unsigned long *eBuff, char bound1, char bound2)
{
  _KEYMACRO_PARSE(CleverSysLog Clev("parseExpr"));
  _KEYMACRO_PARSE(SysLog("bound1=%X, bound2=%X",bound1, bound2));
  char tmp[4];
  IsProcessFunc=0;
  _macro_ErrCode = exprBuffSize = _macro_nErr = 0;
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
  _KEYMACRO_PARSE(SysLog("pSrcString = %s",pSrcString));
  _KEYMACRO_PARSE(SysLog("oSrcString = %s",oSrcString));
  _KEYMACRO_PARSE(SysLog("sSrcString = %s",sSrcString));
#if 1
//!defined(TEST000)
  getToken();
  if ( bound2 )
    expr();
  else
    prim();
  BufPtr = oSrcString;
  while ( *BufPtr && isspace(*BufPtr) )
    BufPtr++;
  if ( bound2 )
  {
    if ( ( *BufPtr != bound2 ) /* ?лишнее ли? ==> */|| !( !BufPtr[1] || isspace(BufPtr[1]) ) )
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
  return exprBuffSize;
}

/* ********************************************************************************* */
// ПАРСЕР
/* ********************************************************************************* */

// Парсер строковых эквивалентов в коды клавиш
//- AN ----------------------------------------------
//  Парсер строковых эквивалентов в байткод
//  Переписан практически с нуля 15.11.2003
//- AN ----------------------------------------------

#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
static char *printfStr(DWORD* k, int& i)
{
  i++;
  char *s = (char*)&k[i];
  while ( strlen((char*)&k[i]) > 3 )
    i++;
  return s;
}

static void printKeyValue(DWORD* k, int& i)
{
  DWORD Code=k[i];
  const char *_mcodename=_MCODE_ToName(Code);
  const char *cmt="";

  static struct {
    DWORD c;
    const char *n;
  } kmf[]={
    {MCODE_F_ABS,              "N=abs(N)"},
    {MCODE_F_ASC,              "N=asc(S)"},
    {MCODE_F_CHR,              "S=chr(N)"},
    {MCODE_F_AKEY,             "V=akey(N)"},
    {MCODE_F_CLIP,             "V=clip(N[,S])"},
    {MCODE_F_DATE,             "S=date(S)"},
    {MCODE_F_DLG_GETVALUE,     "V=Dlg.GetValue(ID,N)"},
    {MCODE_F_EDITOR_SET,       "N=Editor.Set(N,Var)"},
    {MCODE_F_EDITOR_SEL,       "V=Editor.Sel(Action[,Opt])"},
    {MCODE_F_ENVIRON,          "S=env(S)"},
    {MCODE_F_FATTR,            "N=fattr(S)"},
    {MCODE_F_FEXIST,           "S=fexist(S)"},
    {MCODE_F_FLOCK,            "N=FLock(N,N)"},
    {MCODE_F_FSPLIT,           "S=fsplit(S,N)"},
    {MCODE_F_IIF,              "V=iif(Condition,V1,V2)"},
    {MCODE_F_INDEX,            "S=index(S1,S2)"},
    {MCODE_F_INT,              "N=int(V)"},
    {MCODE_F_ATOI,             "N=atoi(S[,radix])"},
    {MCODE_F_ITOA,             "S=itoa(N[,radix])"},
    {MCODE_F_LCASE,            "S=lcase(S1)"},
    {MCODE_F_LEN,              "N=len(S)"},
    {MCODE_F_MAX,              "N=max(N1,N2)"},
    {MCODE_F_MENU_CHECKHOTKEY, "N=checkhotkey(S)"},
    {MCODE_F_MENU_SELECT,      "N=Menu.Select(S[,N])"},
    {MCODE_F_MENU_GETHOTKEY,   "S=gethotkey([N])"},
    {MCODE_F_MIN,              "N=min(N1,N2)"},
    {MCODE_F_MOD,              "N=mod(N1,N2)"},
    {MCODE_F_MSAVE,            "N=msave(S)"},
    {MCODE_F_MSGBOX,           "N=msgbox([sTitle[,sText[,flags]]])"},
    {MCODE_F_PANEL_FATTR,      "N=panel.fattr(panelType,S)"},
    {MCODE_F_PANEL_FEXIST,     "S=panel.fexist(panelType,S)"},
    {MCODE_F_PANEL_SETPATH,    "N=panel.SetPath(panelType,pathName[,fileName])"},
    {MCODE_F_PANEL_SETPOS,     "N=panel.SetPos(panelType,fileName)"},
    {MCODE_F_PANEL_SETPOSIDX,  "N=panel.SetPosIdx(panelType,Index)"},
    {MCODE_F_PANELITEM,        "V=panelitem(Panel,Index,TypeInfo)"},
    {MCODE_F_EVAL,             "N=eval(S[,N])"},
    {MCODE_F_CALLPLUGIN,       "V=callplugin(SysID[,param])"},
    {MCODE_F_REPLACE,          "S=replace(sS,sF,sR[,cnt])"},
    {MCODE_F_RINDEX,           "S=rindex(S1,S2)"},
    {MCODE_F_SLEEP,            "N=Sleep(N)"},
    {MCODE_F_STRING,           "S=string(V)"},
    {MCODE_F_SUBSTR,           "S=substr(S1,S2[,N])"},
    {MCODE_F_UCASE,            "S=ucase(S1)"},
    {MCODE_F_WAITKEY,          "V=waitkey([N,[T]])"},
    {MCODE_F_XLAT,             "S=xlat(S)"},
    {MCODE_F_BM_ADD,           "N=BM.Add()"},
    {MCODE_F_BM_CLEAR,         "N=BM.Clear()"},
    {MCODE_F_BM_NEXT,          "N=BM.Next()"},
    {MCODE_F_BM_PREV,          "N=BM.Prev()"},
    {MCODE_F_BM_STAT,          "N=BM.Stat([N])"},
    {MCODE_F_BM_GET,           "N=BM.Get(Idx,M)"},
    {MCODE_F_BM_DEL,           "N=BM.Del([Idx])"},
    {MCODE_F_TRIM,             "S=trim(S[,N])"},
    {MCODE_F_REG_GET,          "V=reg.get(iRoot, \"Key\"[, \"Value\"])"},
    {MCODE_F_REG_CHECK,        "V=reg.check(iRoot, \"Key\"[, \"Value\"])"},
    {MCODE_F_KEY,              "S=key(V)"},
 };

  if(Code >= MCODE_F_NOFUNC && Code <= KEY_MACRO_C_BASE-1)
  {
    for(int J=0; J <= sizeof(kmf)/sizeof(kmf[0]); ++J)
      if(kmf[J].c == Code)
      {
         cmt=kmf[J].n;
         break;
      }
  }

  if(Code == MCODE_OP_KEYS)
  {
    char tmp[128];
    SysLog("%08X: %08X | MCODE_OP_KEYS", i,MCODE_OP_KEYS);
    ++i;
    while((Code=k[i]) != MCODE_OP_ENDKEYS)
    {
      if ( KeyToText(Code, tmp, sizeof(tmp)) )
        SysLog("%08X: %08X | Key: '%s'", i,Code,tmp);
      else
        SysLog("%08X: %08X | ???", i,Code);
      ++i;
    }
    SysLog("%08X: %08X | MCODE_OP_ENDKEYS", i,MCODE_OP_ENDKEYS);
    return;
  }

  if(Code >= KEY_MACRO_BASE && Code <= KEY_MACRO_ENDBASE)
  {
    SysLog("%08X: %s  %s%s", i,_mcodename,(*cmt?"# ":""),(*cmt?cmt:""));
    //++i;
   // return;
  }


  int ii = i;

  if ( !Code )
  {
    SysLog("%08X: %08X | <null>", ii,k[i]);
  }
  else if ( Code == MCODE_OP_REP )
  {
    FARINT64 i64;
    i64.Part.HighPart=k[i+1];
    i64.Part.LowPart=k[i+2];
    SysLog("%08X: %08X |   %I64d", ii,k[i+1],i64.i64);
    SysLog("%08X: %08X |", ii,k[i+2]);
    i+=2;
  }
  else if ( Code == MCODE_OP_PUSHINT )
  {
    FARINT64 i64;
    ++i;
    i64.Part.HighPart=k[i];
    i64.Part.LowPart=k[i+1];
    SysLog("%08X: %08X |   %I64d", ++ii,k[i],i64.i64);
    ++i;
    SysLog("%08X: %08X |", ++ii,k[i]);
  }
  else if ( Code == MCODE_OP_PUSHSTR || Code == MCODE_OP_PUSHVAR || Code == MCODE_OP_SAVE || Code == MCODE_OP_PUSHCONST)
  {
    int iii=i+1;
    const char *s=printfStr(k, i);
    if(Code == MCODE_OP_PUSHSTR || Code == MCODE_OP_PUSHCONST)
      SysLog("%08X: %08X |   \"%s\"", iii,k[iii], s);
    else
      SysLog("%08X: %08X |   %%%s", iii,k[iii], s);
    for(iii++; iii <= i; ++iii)
      SysLog("%08X: %08X |", iii,k[iii]);
  }
  else if ( Code >= MCODE_OP_JMP && Code <= MCODE_OP_JGE)
  {
    ++i;
    SysLog("%08X: %08X |   %08X (%s)", i,k[i],k[i],((int)k[i]<i?"up":"down"));
  }
/*
  else if ( Code == MCODE_OP_DATE )
  {
    //sprint(k[i],ii, "$date ''");
  }
  else if ( Code == MCODE_OP_PLAINTEXT )
  {
    //sprint(k[i],ii, "$text ''");
  }
*/

  else if(k[i] < KEY_MACRO_BASE || k[i] > KEY_MACRO_ENDBASE)
  {
    int FARFunc = 0;
    for ( int j = 0 ; j < MKeywordsSize ; j++ )
    {
      if ( Code == MKeywords[j].Value)
      {
        FARFunc = 1;
        SysLog("%08X: %08X | %s", ii,Code,MKeywords[j].Name);
        break;
      }
    }
    for ( int j = 0 ; j < MKeywordsFlagsSize ; j++ )
    {
      if ( Code == MKeywordsFlags[j].Value)
      {
        FARFunc = 1;
        SysLog("%08X: %08X | %s", ii,Code,MKeywordsFlags[j].Name);
        break;
      }
    }
    if ( !FARFunc )
    {
      char tmp[128];
      if ( KeyMacroToText(Code, tmp, sizeof(tmp)) )
        SysLog("%08X: %08X | Key: '%s'", ii,Code,tmp);
      //else if(*cmt)
      //  SysLog("%08X: %s  %s%s", ii,_mcodename,(*cmt?"# ":""),(*cmt?cmt:""));
      else if(!*cmt)
        SysLog("%08X: %08X | ???", ii,Code);
    }
  }
}

#endif
#endif



#if 0
#define DeltaByteCodeBuff 8
class ByteCodeBuff{
  private:
    DWROD *Buff;
    int    Count;
    int    Idx;

  private:
    int add(const DWORD *Code, int Size);

  public:
    ByteCodeBuff() {Buff=NULL;Count=0,Idx=0};
    ~ByteCodeBuff() {if(Buff) xf_free(Buff);};

  public:
    DWORD& operator()(int I) {return Buff[Idx+I];};
    DWORD& operator[](int I) {return Buff[I];};

    ByteCodeBuff& operator+=(DWORD Code) {_add(&Code,1);return *this;};
    ByteCodeBuff& add(DWORD Code)  {_add(&Code,1);return *this;};
    ByteCodeBuff& add(const DWORD *Code, int Size) {_add(Code,Size);return *this;};
};

int ByteCodeBuff::add(const DWORD *Code, int Size)
{
  if (Idx+Size >= Count)
  {
    Count+=((Size+7)/8)*8;
    DWORD *CurPtr;
    if ((CurPtr=(DWORD *)xf_realloc(Buff,Count*sizeof(DWORD)))==NULL)
      return 0;
    Buff=CurPtr;
  }
  memcpy(Buff+Idx, Code, Size*sizeof(DWORD));
  Idx+=Size;
  return 1;
}
#endif



static const char *__GetNextWord(const char *BufPtr,char *CurKeyText)
{
   // пропускаем ведущие пробельные символы
   while (IsSpace(*BufPtr) || IsEol(*BufPtr))
   {
     //if(IsEol(*BufPtr))
     //{
       // TODO!!!
     //}
     BufPtr++;
   }

   if (*BufPtr==0)
     return NULL;

   const char *CurBufPtr=BufPtr;
   char Chr=*BufPtr, Chr2=BufPtr[1];
   BOOL SpecMacro=Chr=='$' && Chr2 && !(IsSpace(Chr2) || IsEol(Chr2));

   // ищем конец очередного названия клавиши
   while (Chr && !(IsSpace(Chr) || IsEol(Chr))) // удалить IsEol(*BufPtr)?
   {
     if(SpecMacro && (Chr == '[' || Chr == '(' || Chr == '{'))
       break;
     BufPtr++;
     Chr=*BufPtr;
   }
   int Length=(int)(BufPtr-CurBufPtr);

   memcpy(CurKeyText,CurBufPtr,Length);
   CurKeyText[Length]=0;
   return BufPtr;
}

//- AN ----------------------------------------------
//  Компиляция строки BufPtr в байткод CurMacroBuffer
//- AN ----------------------------------------------
int __parseMacroString(DWORD *&CurMacroBuffer, int &CurMacroBufferSize, const char *BufPtr)
{
  _KEYMACRO(CleverSysLog Clev("parseMacroString"));
  _KEYMACRO(SysLog("Param: BufPtr[%p]='%s'", BufPtr,BufPtr));

  _macro_nErr = 0;
  if ( BufPtr == NULL || !*BufPtr)
  {
    _KEYMACRO_PARSE(SysLog("[%d] return FALSE: BufPtr == NULL || !*BufPtr", __LINE__));
    return FALSE;
  }

  int SizeCurKeyText = (int)strlen(BufPtr)*2;
  char *CurrKeyText = (char*)xf_malloc(SizeCurKeyText);
  if ( CurrKeyText == NULL )
  {
    _KEYMACRO_PARSE(SysLog("[%d] return FALSE: xf_malloc == NULL (for CurrKeyText)", __LINE__));
    return FALSE;
  }
  //- AN ----------------------------------------------
  //  Буфер под парсинг выражений
  //- AN ----------------------------------------------
  DWORD *dwExprBuff = (DWORD*)xf_malloc(SizeCurKeyText*sizeof(DWORD));
  if ( dwExprBuff == NULL )
  {
    xf_free(CurrKeyText);
    _KEYMACRO_PARSE(SysLog("[%d] return FALSE: xf_malloc == NULL (for dwExprBuff)", __LINE__));
    return FALSE;
  }

  TExec exec;
  char varName[256];
  DWORD KeyCode, *CurMacro_Buffer = NULL;

  _KEYMACRO_PARSE(SysLog("<Parse>{"));
  _KEYMACRO_PARSE(int __ItNum=0);
  for (;;)
  {
    _KEYMACRO_PARSE(++__ItNum);
    //_KEYMACRO_PARSE(SysLog("iteration=%d", __ItNum));

    int Size = 1;
    int SizeVarName = 0;
    const char *oldBufPtr = BufPtr;

    if ( ( BufPtr = __GetNextWord(BufPtr, CurrKeyText) ) == NULL )
       break;

    _KEYMACRO_PARSE(SysLog("(%d) CurrKeyText='%s'", __ItNum,CurrKeyText));

    //- AN ----------------------------------------------
    //  Проверка на строковый литерал
    //  Сделаем $Text опциональным
    //- AN ----------------------------------------------
    if ( *CurrKeyText == '\"' && CurrKeyText[1] )
    {
      KeyCode = MCODE_OP_PLAINTEXT;
      BufPtr = oldBufPtr;
      _KEYMACRO_PARSE(SysLog("(%d) KeyCode = MCODE_OP_PLAINTEXT", __ItNum));
    }
    else if ( ( KeyCode = KeyNameMacroToKey(CurrKeyText) ) == (DWORD)-1 && ( KeyCode = KeyNameToKey(CurrKeyText) ) == (DWORD)-1)
    {
      _KEYMACRO_PARSE(CleverSysLog Clev("KeyNameToKey() == -1"));
      int ProcError=0;

      if ( *CurrKeyText == '%' && ( ( isalnum(CurrKeyText[1]) || CurrKeyText[1] == '_' ) || ( CurrKeyText[1] == '%' && ( isalnum(CurrKeyText[2]) || CurrKeyText[2]=='_' ) ) ) )
      {
        _KEYMACRO_PARSE(CleverSysLog Clev("Detect Vars"));

        BufPtr = oldBufPtr;
        while ( *BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)))
          BufPtr++;
        memset(varName, 0, sizeof(varName));
        KeyCode = MCODE_OP_SAVE;
        char* p = varName;
        char* s = CurrKeyText+1;
        if ( *s == '%' )
          *p++ = *s++;
        char ch;
        *p++ = *s++;
        while ( ( isalnum(ch = *s++) || ( ch == '_') ) )
          *p++ = ch;
        *p = 0;
        int Length = (int)strlen(varName)+1;
        // строка должна быть выровнена на 4
        SizeVarName = Length/sizeof(DWORD);
        if ( Length == 1 || ( Length % sizeof(DWORD)) != 0 ) // дополнение до sizeof(DWORD) нулями.
          SizeVarName++;
        _KEYMACRO_PARSE(SysLog("(%d) KeyCode = MCODE_OP_SAVE, varName='%s', SizeVarName=%d", __ItNum,varName,SizeVarName));
        BufPtr += Length;
        Size += parseExpr(BufPtr, dwExprBuff, '=', ';');
        if(_macro_nErr)
        {
          ProcError++;
        }
      }
      else
      {
        _KEYMACRO_PARSE(CleverSysLog Clev("not Vars"));
        // проверим вариант, когда вызвали функцию, но результат не присвоили,
        // например, вызвали MsgBox(), но результат неважен
        // тогда SizeVarName=1 и varName=""
        int __nParam, __oParam;

        char *Brack=strpbrk(CurrKeyText,"( "), Chr=0;
        if(Brack)
        {
          Chr=*Brack;
          *Brack=0;
        }

        if(funcLook(CurrKeyText, __nParam, __oParam) != MCODE_F_NOFUNC)
        {
          _KEYMACRO_PARSE(CleverSysLog Clev("Detect Func"));
          if(Brack) *Brack=Chr;
          BufPtr = oldBufPtr;
          while ( *BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)) )
            BufPtr++;
          Size += parseExpr(BufPtr, dwExprBuff, 0, 0);
          /*
          // этого пока ненадо, считаем, что ';' идет сразу за функцией, иначе это отдельный символ ';', который нужно поместить в поток
          while ( *BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)) )
            BufPtr++;
          */
          if(*BufPtr == ';')
            BufPtr++; // здесь Size не увеличиваем, т.к. мы прокидываем символ ';'

          //Size--; //???
          if(_macro_nErr)
          {
            ProcError++;
          }
          else
          {
            KeyCode=MCODE_OP_SAVE;
            SizeVarName=1;
            memset(varName, 0, sizeof(varName));
          }
        }
        else
        {
          if(Brack) *Brack=Chr;
          ProcError++;
        }
      }

      if(ProcError)
      {
        if(!_macro_nErr)
          keyMacroParseError(err_Unrecognized_keyword, CurrKeyText, CurrKeyText,CurrKeyText);

        if ( CurMacro_Buffer != NULL )
        {
          xf_free(CurMacro_Buffer);
          CurMacroBuffer = NULL;
        }
        CurMacroBufferSize = 0;
        xf_free(CurrKeyText);
        xf_free(dwExprBuff);
        return FALSE;
      }

    }
    else if(!(CurrKeyText[0] == '$' && CurrKeyText[1]))
    {
      Size=3;
      KeyCode=MCODE_OP_KEYS;
    }

    switch ( KeyCode )
    {
      case MCODE_OP_DATE:
        while ( *BufPtr && IsSpace(*BufPtr) )
          BufPtr++;
        if ( *BufPtr == '\"' && BufPtr[1] )
          Size += parseExpr(BufPtr, dwExprBuff, 0, 0);
        else // Опциональность аргумента
        {
          Size += 2;
          dwExprBuff[0] = MCODE_OP_PUSHSTR;
          dwExprBuff[1] = 0;
        }
        break;
      case MCODE_OP_PLAINTEXT:
      case MCODE_OP_MACROMODE:
        Size += parseExpr(BufPtr, dwExprBuff, 0, 0);
        break;

// $Rep (expr) ... $End
// -------------------------------------
//            <expr>
//            MCODE_OP_SAVEREPCOUNT       1
// +--------> MCODE_OP_REP                    p1=*
// |          <counter>                   3
// |          <counter>                   4
// |          MCODE_OP_JZ  ------------+  5   p2=*+2
// |          ...                      |
// +--------- MCODE_OP_JMP             |
//            MCODE_OP_END <-----------+

      case MCODE_OP_REP:
        Size += parseExpr(BufPtr, dwExprBuff, '(', ')');
        if ( !exec.add(emmRep, CurMacroBufferSize+Size, CurMacroBufferSize+Size+4) ) //??? 3
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(CurrKeyText);
          xf_free(dwExprBuff);
          return FALSE;
        }
        Size += 5;  // естественно, размер будет больше = 4
        break;

// $If (expr) ... $End
// -------------------------------------
//            <expr>
//            MCODE_OP_JZ  ------------+      p1=*+0
//            ...                      |
// +--------- MCODE_OP_JMP             |
// |          ...          <-----------+
// +--------> MCODE_OP_END

// или

//            <expr>
//            MCODE_OP_JZ  ------------+      p1=*+0
//            ...                      |
//            MCODE_OP_END <-----------+

      case MCODE_OP_IF:
        Size += parseExpr(BufPtr, dwExprBuff, '(', ')');
        if ( !exec.add(emmThen, CurMacroBufferSize+Size) )
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(CurrKeyText);
          xf_free(dwExprBuff);
          return FALSE;
        }
        Size++;
        break;

      case MCODE_OP_ELSE:
        Size++;
        break;

// $While (expr) ... $End
// -------------------------------------
// +--------> <expr>
// |          MCODE_OP_JZ  ------------+
// |          ...                      |
// +--------- MCODE_OP_JMP             |
//            MCODE_OP_END <-----------+

      case MCODE_OP_WHILE:
        Size += parseExpr(BufPtr, dwExprBuff, '(', ')');
        if ( !exec.add(emmWhile, CurMacroBufferSize, CurMacroBufferSize+Size) )
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(CurrKeyText);
          xf_free(dwExprBuff);
          return FALSE;
        }
        Size++;
        break;
      case MCODE_OP_END:
        switch ( exec().state )
        {
          case emmRep:
          case emmWhile:
            Size += 2; // Место под дополнительный JMP
            break;
        }
        break;
    }
    if(_macro_nErr)
    {
      if ( CurMacro_Buffer != NULL )
      {
        xf_free(CurMacro_Buffer);
        CurMacroBuffer = NULL;
      }
      CurMacroBufferSize = 0;
      xf_free(CurrKeyText);
      xf_free(dwExprBuff);
      return FALSE;
    }

    if ( BufPtr == NULL ) // ???
      break;
    // код найден, добавим этот код в буфер последовательности.
    CurMacro_Buffer = (DWORD *)xf_realloc(CurMacro_Buffer,sizeof(*CurMacro_Buffer)*(CurMacroBufferSize+Size+SizeVarName));
    if ( CurMacro_Buffer == NULL )
    {
      CurMacroBuffer = NULL;
      CurMacroBufferSize = 0;
      xf_free(CurrKeyText);
      xf_free(dwExprBuff);
      return FALSE;
    }
    switch ( KeyCode )
    {
      case MCODE_OP_DATE:
      case MCODE_OP_PLAINTEXT:
      case MCODE_OP_MACROMODE:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, dwExprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
        break;
      case MCODE_OP_SAVE:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, dwExprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
        memcpy(CurMacro_Buffer+CurMacroBufferSize+Size, varName, SizeVarName*sizeof(DWORD));
        break;
      case MCODE_OP_IF:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, dwExprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
        break;
      case MCODE_OP_REP:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, dwExprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-6] = MCODE_OP_SAVEREPCOUNT;
        CurMacro_Buffer[CurMacroBufferSize+Size-5] = KeyCode;
        CurMacro_Buffer[CurMacroBufferSize+Size-4] = 0; // Initilize 0
        CurMacro_Buffer[CurMacroBufferSize+Size-3] = 0;
        CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
        break;
      case MCODE_OP_WHILE:
        memcpy(CurMacro_Buffer+CurMacroBufferSize, dwExprBuff, Size*sizeof(DWORD));
        CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
        break;
      case MCODE_OP_ELSE:
        if ( exec().state == emmThen )
        {
          exec().state = emmElse;
          CurMacro_Buffer[exec().pos1] = CurMacroBufferSize+2;
          exec().pos1 = CurMacroBufferSize;
          CurMacro_Buffer[CurMacroBufferSize] = 0;
        }
        else // тут $else и не предвиделось :-/
        {
          keyMacroParseError(err_Not_expected_ELSE, oldBufPtr+1, oldBufPtr); // CurrKeyText
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(CurrKeyText);
          xf_free(dwExprBuff);
          return FALSE;
        }
        break;
      case MCODE_OP_END:
        switch ( exec().state )
        {
          case emmMain:
            // тут $end и не предвиделось :-/
            keyMacroParseError(err_Not_expected_END, oldBufPtr+1, oldBufPtr); // CurrKeyText
            if ( CurMacro_Buffer != NULL )
            {
              xf_free(CurMacro_Buffer);
              CurMacroBuffer = NULL;
            }
            CurMacroBufferSize = 0;
            xf_free(CurrKeyText);
            xf_free(dwExprBuff);
            return FALSE;
          case emmThen:
            CurMacro_Buffer[exec().pos1-1] = MCODE_OP_JZ;
            CurMacro_Buffer[exec().pos1+0] = CurMacroBufferSize+Size-1;
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
          case emmElse:
            CurMacro_Buffer[exec().pos1-0] = MCODE_OP_JMP; //??
            CurMacro_Buffer[exec().pos1+1] = CurMacroBufferSize+Size-1; //??
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
          case emmRep:
            CurMacro_Buffer[exec().pos2] = CurMacroBufferSize+Size-1;   //??????
            CurMacro_Buffer[CurMacroBufferSize+Size-3] = MCODE_OP_JMP;
            CurMacro_Buffer[CurMacroBufferSize+Size-2] = exec().pos1;
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
          case emmWhile:
            CurMacro_Buffer[exec().pos2] = CurMacroBufferSize+Size-1;
            CurMacro_Buffer[CurMacroBufferSize+Size-3] = MCODE_OP_JMP;
            CurMacro_Buffer[CurMacroBufferSize+Size-2] = exec().pos1;
            CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
            break;
        }

        if ( !exec.del() )  // Вообще-то этого быть не должно,  но подстрахуемся
        {
          if ( CurMacro_Buffer != NULL )
          {
            xf_free(CurMacro_Buffer);
            CurMacroBuffer = NULL;
          }
          CurMacroBufferSize = 0;
          xf_free(CurrKeyText);
          xf_free(dwExprBuff);
          return FALSE;
        }
        break;

      case MCODE_OP_KEYS:
      {
        CurMacro_Buffer[CurMacroBufferSize+Size-3]=MCODE_OP_KEYS;
        CurMacro_Buffer[CurMacroBufferSize+Size-2]=KeyNameToKey(CurrKeyText);
        CurMacro_Buffer[CurMacroBufferSize+Size-1]=MCODE_OP_ENDKEYS;
        break;
      }

      default:
        _KEYMACRO_PARSE(SysLog("(%d) default: KeyCode = %08X", __ItNum,KeyCode));
        CurMacro_Buffer[CurMacroBufferSize]=KeyCode;


    } // end switch(KeyCode)

    CurMacroBufferSize += Size+SizeVarName;
    _KEYMACRO_PARSE(SysLog("CurMacroBufferSize=%d",CurMacroBufferSize));
  } // END for (;;)
  _KEYMACRO_PARSE(SysLog("}</Parse>"));

  if(CurMacroBufferSize == 1)
  {
    CurMacro_Buffer = (DWORD *)xf_realloc(CurMacro_Buffer,sizeof(*CurMacro_Buffer)*(CurMacroBufferSize+1));
    if ( CurMacro_Buffer == NULL )
    {
      CurMacroBuffer = NULL;
      CurMacroBufferSize = 0;
      xf_free(CurrKeyText);
      xf_free(dwExprBuff);
      return FALSE;
    }
    CurMacro_Buffer[CurMacroBufferSize]=MCODE_OP_NOP;
    CurMacroBufferSize++;

  }

#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
  SysLogDump("Macro Buffer",0,(LPBYTE)CurMacro_Buffer,CurMacroBufferSize*sizeof(DWORD),NULL);
  SysLog("<ByteCode>{");
  if ( CurMacro_Buffer )
  {
    int ii;
    for ( ii = 0 ; ii < CurMacroBufferSize ; ii++ )
      printKeyValue(CurMacro_Buffer, ii);
  }
  else
    SysLog("??? is NULL");
  SysLog("}</ByteCode>");
#endif
#endif
  if ( CurMacroBufferSize > 1 )
    CurMacroBuffer = CurMacro_Buffer;
  else if ( CurMacro_Buffer )
  {
    CurMacroBuffer = reinterpret_cast<DWORD*>((DWORD_PTR)(*CurMacro_Buffer));
    xf_free(CurMacro_Buffer);
  }
  xf_free(dwExprBuff);
  if ( exec().state != emmMain )
  {
    keyMacroParseError(err_Unexpected_EOS, CurrKeyText, CurrKeyText);
    return FALSE;
  }
  xf_free(CurrKeyText);
  if ( _macro_nErr )
    return FALSE;
  return TRUE;
}

BOOL __getMacroParseError(char *ErrMsg1,char *ErrMsg2,char *ErrMsg3)
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

int  __getMacroErrorCode(int *nErr)
{
  if(nErr)
    *nErr=_macro_nErr;
  return _macro_ErrCode;
}
