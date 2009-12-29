/*
syntax.cpp

Реализация парсера для MacroDrive II

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
// Парсер и "выполнятор" выражений
//---------------------------------------------------------------


#include "headers.hpp"
#pragma hdrstop

#include "macroopcode.hpp"
#include "lang.hpp"
#include "syntax.hpp"
#include "tvar.hpp"
#include "syslog.hpp"
#include "language.hpp"
#include "keyboard.hpp"
#include "macro.hpp"
#include "strmix.hpp"

#define EOFCH 65536

static int exprBuffSize = 0;
static unsigned long FARVar, *exprBuff = NULL;
static int IsProcessFunc=0;

static int _macro_nErr = 0;
static int _macro_ErrCode=err_Success;
static wchar_t nameString[1024];
static wchar_t *sSrcString;
static wchar_t *pSrcString = NULL;
static wchar_t *oSrcString = NULL;
static wchar_t emptyString[1]={0};

static TToken currTok = tNo;
static TVar currVar;

static void expr();

#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
static void printKeyValue(DWORD* k, int& i);
#endif
#endif

static const wchar_t *__GetNextWord(const wchar_t *BufPtr,string &strCurKeyText);
static void keyMacroParseError(int err, const wchar_t *s, const wchar_t *p, const wchar_t *c=NULL);
static void keyMacroParseError(int err, const wchar_t *c = NULL);

// Стек структурных операторов
enum TExecMode
{
	emmMain,
	emmWhile,
	emmThen,
	emmElse,
	emmRep
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

	public:
		TExec() { init(); }

	public:
		void init()
		{
			current = 0;
			stack[current].state = emmMain;
			stack[current].pos1 = stack[current].pos2 = 0;
		}

		TExecItem& operator()() { return stack[current]; }
		int add(TExecMode, DWORD, DWORD = 0);
		int del();
};

int TExec::add(TExecMode s, DWORD p1, DWORD p2)
{
	if (++current < MAXEXEXSTACK)
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
	if (--current < 0)
	{
		// Stack Underflow ???
		current = 0;
		return FALSE;
	}

	return TRUE;
};


//-----------------------------------------------
static string ErrMessage[3];

static void put(unsigned long code)
{
	exprBuff[exprBuffSize++] = code;
}

static void putDouble(double code)
{
	LARGE_INTEGER *i64=(LARGE_INTEGER *)&code;
	exprBuff[exprBuffSize++] = i64->u.HighPart;
	exprBuff[exprBuffSize++] = i64->u.LowPart;
}

static void put64(unsigned __int64 code)
{
	LARGE_INTEGER i64;
	i64.QuadPart=code;
	exprBuff[exprBuffSize++] = i64.u.HighPart;
	exprBuff[exprBuffSize++] = i64.u.LowPart;
}

static void putstr(const wchar_t *s)
{
	_KEYMACRO(CleverSysLog Clev(L"putstr"));
	_KEYMACRO(SysLog(L"s[%p]='%s'", s,s));
	int Length = (int)(StrLength(s)+1)*sizeof(wchar_t);
	// строка должна быть выровнена на 4
	int nSize = Length/sizeof(DWORD);
	memmove(&exprBuff[exprBuffSize],s,Length);

	if (Length == sizeof(wchar_t) || (Length % sizeof(DWORD)) != 0)    // дополнение до sizeof(DWORD) нулями.
		nSize++;

	memset(&exprBuff[exprBuffSize],0,nSize*sizeof(DWORD));
	memmove(&exprBuff[exprBuffSize],s,Length);
	exprBuffSize+=nSize;
}

static void keyMacroParseError(int err, const wchar_t *s, const wchar_t *p, const wchar_t *c)
{
	if (!_macro_nErr++)
	{
		_macro_ErrCode=err;
		int oPos = 0, ePos = (int)(s-p);
		ErrMessage[0]=ErrMessage[1]=ErrMessage[2]=L"";

		if (ePos < 0)
		{
			ErrMessage[0] = MSG(MMacroPErrExpr_Expected); // TODO: .Format !
			return;
		}

		ErrMessage[0].Format(MSG(MMacroPErrUnrecognized_keyword+err-1),c);

		if (ePos > 61)
		{
			oPos = ePos-50;
			ErrMessage[1] = L"...";
		}

		ErrMessage[1] += p+oPos;
		//if ( ErrMessage[1][61] ) BUGBUG
		//	strncpy(&ErrMessage[1][61], "...",sizeof(ErrMessage[1])-62);
		int lPos = ePos-oPos+(oPos ? 3 : 0);
		InsertQuote(ErrMessage[1]);
		ErrMessage[2].Format(L"%*s%c", lPos+1, L"", L'^');
	}
}

static void keyMacroParseError(int err, const wchar_t *c)
{
	keyMacroParseError(err, oSrcString, pSrcString, c);
	//                      ^ s?
}
//-----------------------------------------------

static int getNextChar()
{
	if (*sSrcString)
	{
		int ch;

		while (((ch = *(sSrcString++)) != 0) && iswspace(ch))
			;

		return ch ? ch : EOFCH;
	}

	return EOFCH;
}

static inline int getChar()
{
	if (*sSrcString)
	{
		int ch = *(sSrcString++);
		return (ch) ? ch : EOFCH;
	}

	return EOFCH;
}

struct TMacroFunction
{
	const wchar_t *Name;             // имя функции
	int nParam;                      // количество параметров
	int oParam;                      // необязательные параметры
	TMacroOpCode Code;               // байткод функции
	const wchar_t *fnGUID;           // GUID обработчика функции
	/*
	int    BufferSize;               // Размер буфера компилированной последовательности
	DWORD *Buffer;                   // компилированная последовательность (OpCode) макроса
	wchar_t  *Src;                   // оригинальный "текст" макроса
	wchar_t  *Description;           // описание макроса
	*/
	const wchar_t *Syntax;           // Синтаксис функции
};

static TMacroFunction macroFunction[]=
{
	{L"ABS",              1, 0,   MCODE_F_ABS,              NULL, L"N=Abs(N)"},
	{L"AKEY",             1, 0,   MCODE_F_AKEY,             NULL, L"V=Akey(N)"},
	{L"ASC",              1, 0,   MCODE_F_ASC,              NULL, L"N=Asc(N)"},
	{L"ATOI",             2, 1,   MCODE_F_ATOI,             NULL, L"N=Atoi(S[,radix])"},
	{L"BM.ADD",           0, 0,   MCODE_F_BM_ADD,           NULL, L"N=BM.Add()"},
	{L"BM.CLEAR",         0, 0,   MCODE_F_BM_CLEAR,         NULL, L"N=BM.Clear()"},
	{L"BM.DEL",           1, 1,   MCODE_F_BM_DEL,           NULL, L"N=BM.Del([Idx])"},
	{L"BM.GET",           2, 0,   MCODE_F_BM_GET,           NULL, L"N=BM.Get(Idx,M)"},
	{L"BM.NEXT",          0, 0,   MCODE_F_BM_NEXT,          NULL, L"N=BM.Next()"},
	{L"BM.PREV",          0, 0,   MCODE_F_BM_PREV,          NULL, L"N=BM.Prev()"},
	{L"BM.STAT",          1, 1,   MCODE_F_BM_STAT,          NULL, L"N=BM.Stat([N])"},
	{L"CHECKHOTKEY",      2, 1,   MCODE_F_MENU_CHECKHOTKEY, NULL, L"N=checkhotkey(S[,N])"},
	{L"CALLPLUGIN",       2, 1,   MCODE_F_CALLPLUGIN,       NULL, L"V=CallPlugin(SysID[,param])"},
	{L"CHR",              1, 0,   MCODE_F_CHR,              NULL, L"S=Chr(N)"},
	{L"CLIP",             2, 1,   MCODE_F_CLIP,             NULL, L"V=Clip(N[,S])"},
	{L"DATE",             1, 0,   MCODE_F_DATE,             NULL, L"S=Date(S)"},
	{L"DLG.GETVALUE",     2, 0,   MCODE_F_DLG_GETVALUE,     NULL, L"V=Dlg.GetValue(ID,N)"},
	{L"EDITOR.POS",       3, 1,   MCODE_F_EDITOR_POS,       NULL, L"N=Editor.Pos(Op,What[,Where])"},
	{L"EDITOR.SEL",       2, 1,   MCODE_F_EDITOR_SEL,       NULL, L"V=Editor.Sel(Action[,Opt])"},
	{L"EDITOR.SET",       2, 0,   MCODE_F_EDITOR_SET,       NULL, L"N=Editor.Set(N,Var)"},
	{L"EDITOR.UNDO",      1, 0,   MCODE_F_EDITOR_UNDO,      NULL, L"V=Editor.Undo(N)"},
	{L"ENV",              1, 0,   MCODE_F_ENVIRON,          NULL, L"S=Env(S)"},
	{L"EVAL",             2, 1,   MCODE_F_EVAL,             NULL, L"N=Eval(S[,N])"},
	{L"FATTR",            1, 0,   MCODE_F_FATTR,            NULL, L"N=FAttr(S)"},
	{L"FEXIST",           1, 0,   MCODE_F_FEXIST,           NULL, L"N=FExist(S)"},
	{L"FLOAT",            1, 0,   MCODE_F_FLOAT,            NULL, L"N=Float(V)"},
	{L"FLOCK",            2, 0,   MCODE_F_FLOCK,            NULL, L"N=FLock(N,N)"},
	{L"FSPLIT",           2, 0,   MCODE_F_FSPLIT,           NULL, L"S=FSplit(S,N)"},
	{L"GETHOTKEY",        1, 1,   MCODE_F_MENU_GETHOTKEY,   NULL, L"S=GetHotkey([N])"},
	{L"IIF",              3, 0,   MCODE_F_IIF,              NULL, L"V=Iif(Condition,V1,V2)"},
	{L"INDEX",            3, 1,   MCODE_F_INDEX,            NULL, L"S=Index(S1,S2[,Mode])"},
	{L"INT",              1, 0,   MCODE_F_INT,              NULL, L"N=Int(V)"},
	{L"ITOA",             2, 1,   MCODE_F_ITOA,             NULL, L"S=Itoa(N[,radix])"},
	{L"LCASE",            1, 0,   MCODE_F_LCASE,            NULL, L"S=LCase(S1)"},
	{L"KEY",              1, 0,   MCODE_F_KEY,              NULL, L"S=Key(V)"},
	{L"LEN",              1, 0,   MCODE_F_LEN,              NULL, L"N=Len(S)"},
	{L"MAX",              2, 0,   MCODE_F_MAX,              NULL, L"N=Max(N1,N2)"},
	{L"MENU.SELECT",      3, 2,   MCODE_F_MENU_SELECT,      NULL, L"N=Menu.Select(S[,N[,Dir]])"},
	{L"MOD",              2, 0,   MCODE_F_MOD,              NULL, L"N=Mod(a,b)"},
	{L"MSAVE",            1, 0,   MCODE_F_MSAVE,            NULL, L"N=MSave(S)"},
	{L"MSGBOX",           3, 3,   MCODE_F_MSGBOX,           NULL, L"N=MsgBox([Title[,Text[,flags]]])"},
	{L"MIN",              2, 0,   MCODE_F_MIN,              NULL, L"N=Min(N1,N2)"},
	{L"PANEL.FATTR",      2, 0,   MCODE_F_PANEL_FATTR,      NULL, L"N=Panel.FAttr(panelType,fileMask)"},
	{L"PANEL.FEXIST",     2, 0,   MCODE_F_PANEL_FEXIST,     NULL, L"N=Panel.FExist(panelType,fileMask)"},
	{L"PANEL.SETPATH",    3, 1,   MCODE_F_PANEL_SETPATH,    NULL, L"N=panel.SetPath(panelType,pathName[,fileName])"},
	{L"PANEL.SETPOS",     2, 0,   MCODE_F_PANEL_SETPOS,     NULL, L"N=panel.SetPos(panelType,fileName)"},
	{L"PANEL.SETPOSIDX",  2, 0,   MCODE_F_PANEL_SETPOSIDX,  NULL, L"N=Panel.SetPosIdx(panelType,Idx)"},
	{L"PANELITEM",        3, 0,   MCODE_F_PANELITEM,        NULL, L"V=PanelItem(Panel,Index,TypeInfo)"},
	{L"PROMPT",           5, 4,   MCODE_F_PROMPT,           NULL, L"S=Prompt(Title[,Prompt[,flags[,Src[,HistoryName]]]])"},
	{L"REPLACE",          5, 2,   MCODE_F_REPLACE,          NULL, L"S=Replace(Str,Find,Replace[,Cnt[,Mode]])"},
	{L"RINDEX",           3, 1,   MCODE_F_RINDEX,           NULL, L"S=RIndex(S1,S2[,Mode])"},
	{L"SLEEP",            1, 0,   MCODE_F_SLEEP,            NULL, L"N=Sleep(N)"},
	{L"STRING",           1, 0,   MCODE_F_STRING,           NULL, L"S=String(V)"},
	{L"SUBSTR",           3, 1,   MCODE_F_SUBSTR,           NULL, L"S=SubStr(S,N1[,N2])"},
	{L"TRIM",             2, 1,   MCODE_F_TRIM,             NULL, L"S=Trim(S[,N])"},
	{L"UCASE",            1, 0,   MCODE_F_UCASE,            NULL, L"S=UCase(S1)"},
	{L"WAITKEY",          2, 2,   MCODE_F_WAITKEY,          NULL, L"V=Waitkey([N,[T]])"},
	{L"XLAT",             1, 0,   MCODE_F_XLAT,             NULL, L"S=Xlat(S)"},
};

/*
	for(size_t I=0; I < countof(macroFunction); ++I)
	{
		RegisterMacroFunction(macroFunction[I].Name, macroFunction[I].nParam, macroFunction[I].oParam, macroFunction[I].Code, macroFunction[I].fnGUID, macroFunction[I].Src, macroFunction[I].Syntax, macroFunction[I].Description);
	}


bool KeyMacro::RegisterMacroFunction(const wchar_t *Name, int nParam, int oParam, DWORD OpCode, const wchar_t *fnGUID, const wchar_t *Src, const wchar_t *Syntax, const wchar_t *Description)
{
	if ( !OpCode ) // получить временный OpCode относительно KEY_MACRO_U_BASE
		OpCode=GetNewOpCode();
	...
}
*/

static DWORD funcLook(const wchar_t *s, int& nParam, int& oParam)
{
	oParam=nParam=0;

	for (size_t I=0; I < countof(macroFunction); ++I)
	{
		if (!StrCmpNI(s, macroFunction[I].Name, (int)Max(StrLength(macroFunction[I].Name),StrLength(s))))
		{
			nParam = macroFunction[I].nParam;
			oParam = macroFunction[I].oParam;
			return (DWORD)macroFunction[I].Code;
		}
	}

	return (DWORD)MCODE_F_NOFUNC;
}

static TToken getToken();

static void calcFunc()
{
	int nParam, oParam;
	TMacroOpCode nFunc = (TMacroOpCode)funcLook(nameString, nParam, oParam);

	if (nFunc != MCODE_F_NOFUNC)
	{
		IsProcessFunc++;

		if (nParam)
		{
			int i=0;
			int foundparam=0;

			if (nParam >= oParam)
			{
				for (; i < nParam ; i++)
				{
					getToken();

					if (currTok != tRp)
						foundparam++;

					expr();

					if (currTok != ((i == nParam-1) ? tRp : tComma))
					{
						if (oParam > 0 &&  currTok != tEnd)  // если опциональные параметры есть и...
							break;

						if (i == nParam-1)
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
				expr();
			}

			if (oParam > 0) //???
			{
				if (nParam-(i+1) > oParam || (!i && nParam && nParam > oParam && !foundparam))  // проскакивает eval() без параметров!
				{
					keyMacroParseError(err_Func_Param, nameString);
					currTok = tEnd;
				}

				// добьем нулями опциональные параметры
				for (; i < nParam-1; ++i)
				{
					put(MCODE_OP_PUSHINT);

					// исключение для substr
					if (nFunc == MCODE_F_SUBSTR)
						put64((unsigned __int64)-1);
					else
						put64(0);
				}
			}
		}
		else
		{
			getToken();

			if (currTok != tRp)
			{
				keyMacroParseError(err_Expected, L")");
				currTok = tEnd;
			}
		}

		put(nFunc);
		IsProcessFunc--;
	}
	else if (currTok == tFunc)
	{
		keyMacroParseError(err_Unrecognized_function, nameString);
	}
}

static void getVarName(int& ch)
{
	wchar_t* p = nameString;
	*p++ = (wchar_t)ch;

	while (((ch = getChar()) != EOFCH) && (iswalnum(ch) || (ch == L'_')))
		*p++ = (wchar_t)ch;

	*p = 0;
}

static void getFarName(int& ch)
{
	wchar_t* p = nameString;
	*p++ = (wchar_t)ch;

	while (((ch = getChar()) != EOFCH) && (iswalnum(ch) || (ch == L'_') || (ch == L'.')))
		*p++ = (wchar_t)ch;

	*p = 0;
}

static wchar_t *putBack(int ch)
{
	if ((ch && (ch != EOFCH)) && (sSrcString > pSrcString))
		sSrcString--;

	return sSrcString;
}

static inline int peekChar()
{
	int c;
	putBack(c = getChar());
	return c;
}

static wchar_t hex2ch(wchar_t b1)
{
	if (b1 >= L'0' && b1 <= L'9')
		b1 -= L'0';
	else
	{
		b1 &= ~0x20;
		b1 -= (wchar_t)(L'A'-10);
	}

	return (wchar_t)(b1&0x000F);
}

static TToken getToken()
{
	oSrcString = sSrcString;
	int ch = getNextChar();

	switch (ch)
	{
		case EOFCH:
		case 0:    currTok = tEnd;    break;
		case L',': currTok = tComma;  break;
		case L'+': currTok = tPlus;   break;
		case L'-': currTok = tMinus;  break;
		case L'*': currTok = tMul;    break;
		case L'/': currTok = tDiv;    break;
		case L'(': currTok = tLp;     break;
		case L')': currTok = tRp;     break;
		case L'^': currTok = tBitXor; break;
		case L'~':

			if ((ch = getChar()) != L' ')
			{
				putBack(ch);
				currTok = tBitNot;
				break;
			}

			putBack(ch);   //????
			currTok = tEnd;
			break;
		case L'|':

			if ((ch = getChar()) == L'|')
				currTok = tBoolOr;
			else
			{
				putBack(ch);
				currTok = tBitOr;
			}

			break;
		case L'&':

			if ((ch = getChar()) == L'&')
				currTok = tBoolAnd;
			else
			{
				putBack(ch);
				currTok = tBitAnd;
			}

			break;
		case L'=':

			if ((ch = getChar()) == L'=')
				currTok = tEq;
			else
			{
				putBack(ch);
				currTok = tLet;
			}

			break;
		case L'>':

			switch ((ch = getChar()))
			{
				case L'=': currTok = tGe;     break;
				case L'>': currTok = tBitShr; break;
				default:
					putBack(ch);
					currTok = tGt;
					break;
			}

			break;
		case L'<':

			switch (ch = getChar())
			{
				case L'=': currTok = tLe;     break;
				case L'<': currTok = tBitShl; break;
				default:
					putBack(ch);
					currTok = tLt;
					break;
			}

			break;
		case L'!':

			if ((ch = getChar()) != L'=')
			{
				putBack(ch);
				currTok = tNot;
				break;
			}
			else
				currTok = tNe;

			break;
		case L'\"':
		{
			//-AN----------------------------------------------
			// Вообще-то это почти полный аналог ParsePlainText
			//-AN----------------------------------------------
			TToken __currTok = tNo;
			currVar = L"";

			while (((ch = getChar()) != EOFCH) && (ch != L'\"'))
			{
				if (ch == L'\\')
				{
					switch (ch = getChar())
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

							if ((unsigned int)(ch = getChar()) >= L'0' && (unsigned int)ch < L'8')
							{
								n = 8 * n + ch - L'0';

								if ((unsigned int)(ch = getChar()) >= L'0' && (unsigned int)ch < L'8')
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
						{
							if (iswxdigit(ch = getChar()))
							{
								wchar_t value=hex2ch(ch);

								for (int ii=0; ii<3; ii++)
								{
									if (iswxdigit(ch = getChar()))
									{
										value=(value<<4)|hex2ch(ch);
									}
									else
									{
										putBack(ch);
										break;
									}
								}

								ch = value;
							}
							else
							{
								keyMacroParseError(err_Bad_Hex_Control_Char);
								__currTok = tEnd;
							}

							break;
						}
						default:
						{
							keyMacroParseError(err_Bad_Control_Char);
							__currTok = tEnd;
							break;
						}
					}
				}

				if (__currTok != tNo)
					break;

				currVar.AppendStr((wchar_t)ch);
			}

			if (__currTok == tNo)
				currTok = tStr;
			else
				currTok = __currTok;

			break;
		}
		case L'.':
		{
			ch = getChar();

			if (iswdigit(ch))
			{
				putBack(ch);
				ch=L'.';
			}
			else
			{
				currTok = tEnd; //???
				break;
			}
		}
		case L'0': case L'1': case L'2': case L'3': case L'4':
		case L'5': case L'6': case L'7': case L'8': case L'9':
		{
			static wchar_t buffer[256];
			wchar_t *ptrbuffer=buffer;
			bool isNum   = false;
			bool isHex   = false;
			bool isE     = false;
			bool isPoint = false;
			int ch2;

			for (;;)
			{
				*ptrbuffer++=(wchar_t)ch;

				switch (ch)
				{
					case L'x':
					case L'X':

						if (ptrbuffer == buffer + 2)
						{
							ch = getChar();

							if (iswxdigit(ch))
							{
								isHex=true;
								putBack(ch);
							}
							else
							{
								putBack(ch);
								isNum=true;
								break;
							}
						}

						break;
					case L'.':

						if (isPoint || isE)
						{
							isNum=true;
							break;
						}

						isPoint=true;
						break;
					case L'e':
					case L'E':

						if (isHex)
							break;

						if (isE)
						{
							isNum=true;
							break;
						}

						isE=true;
						ch2 = getChar();

						if (ch2 == L'-' || ch2 == L'+')
						{
							int ch3=getChar();

							if (iswdigit(ch3))
							{
								*ptrbuffer++=(wchar_t)ch2;
								*ptrbuffer++=(wchar_t)ch3;
							}
							else
							{
								putBack(ch3);  // !iswdigit
								putBack(ch2);  // -+
								putBack(ch);   // eE
							}
						}
						else if (!iswdigit(ch2))
						{
							putBack(ch2); // !iswdigit
							putBack(ch);  // eE
						}
						else
							putBack(ch);

						break;
					case L'a': case L'A':
					case L'b': case L'B':
					case L'c': case L'C':
					case L'd': case L'D':
					case L'f': case L'F':

						if (!isHex)
						{
							isNum=true;
							break;
						}

					case L'0': case L'1': case L'2': case L'3': case L'4':
					case L'5': case L'6': case L'7': case L'8': case L'9':
						//isNum=true;
						break;
					default:
						isNum=true;
						break;
				}

				if (isNum)
					break;

				ch = getChar();
			}

			if (ch != EOFCH)
				putBack(ch);

			*ptrbuffer++=(wchar_t)0;
			bool CheckIntNumber=true;

			if (buffer[0])
			{
				if (!(buffer[1] == L'x' || buffer[1] == L'X'))
				{
					for (ptrbuffer=buffer; *ptrbuffer ; ptrbuffer++)
					{
						if (*ptrbuffer == L'e' || *ptrbuffer == L'E' || *ptrbuffer == L'.')
						{
							CheckIntNumber=false;
							break;
						}
						else if (!iswdigit(*ptrbuffer))
							break;
					}
				}
			}
			else
				CheckIntNumber=false;

			if (CheckIntNumber)
			{
				currVar = _wcstoi64(buffer,&ptrbuffer,0);
				currTok = tInt;
			}
			else
			{
				currVar = wcstod(buffer,&ptrbuffer);
				currTok = tFloat;
			}

			break;
		}
		case L'%':
			ch = getChar();

			if ((IsAlphaNum(ch) || ch == L'_') || (ch == L'%'  && (IsAlphaNum(*sSrcString) || *sSrcString == L'_')))
			{
				getVarName(ch);
				putBack(ch);
				currTok = tVar;
			}
			else
				keyMacroParseError(err_Var_Expected,L""); // BUG nameString

			break;
		default:
		{
			if (IsAlpha(ch))   // || ch == L'_' ????
			{
				TToken __currTok = tNo;
				getFarName(ch);

				if (ch == L' ')
				{
					while (ch == L' ')
						ch = getNextChar();
				}

				if (ch == L'(')   //!!!! а пробелы пропустить? ДА!
					__currTok = tFunc;
				else
				{
					putBack(ch);

					for (int i = 0 ; i < MKeywordsSize ; i++)
						if (!StrCmpI(nameString, MKeywords[i].Name))
						{
							FARVar = MKeywords[i].Value;
							__currTok = tFARVar;
							break;
						}

					if (__currTok == tNo)
					{
						if (IsProcessFunc || currTok == tFunc || currTok == tLt) // TODO: уточнить
						{
							if (KeyNameMacroToKey(nameString) == -1 && KeyNameToKey(nameString) == -1 && checkMacroConst(nameString))
								__currTok = tConst;
							else
							{
								DWORD k=KeyNameToKey(nameString);

								if (k != (DWORD)-1)
								{
									currVar = (__int64)k;
									__currTok = tInt; //??
								}
								else
								{
									keyMacroParseError(err_Var_Expected,oSrcString,pSrcString,nameString);
								}
							}
						}
						else
						{
							if (KeyNameMacroToKey(nameString) == -1)
							{
								if (KeyNameToKey(nameString) == -1)
								{
									if (checkMacroConst(nameString))
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

				if (__currTok != tNo)
					currTok=__currTok;
			}
			else
				currTok = tEnd;

			break;
		}
	}

	return currTok;
}

static void prim()
{
	switch (currTok)
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
		case tFloat:
			put(MCODE_OP_PUSHFLOAT);
			putDouble(currVar.d());
			getToken();
			break;
		case tFARVar:
			put(FARVar); // nFARVar получаем в getToken()
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

			if (currTok != tRp)
				keyMacroParseError(err_Expected, L")");

			getToken();
			break;
		case tRp: //???
			break;
		default:
			keyMacroParseError(err_Expr_Expected);
			break;
	}
}

static void multExpr()
{
	prim();

	for (; ;)
		switch (currTok)
		{
			case tMul: getToken(); prim(); put(MCODE_OP_MUL); break;
			case tDiv: getToken(); prim(); put(MCODE_OP_DIV); break;
			default:
				return;
		}
}

static void additionExpr()
{
	multExpr();

	for (; ;)
		switch (currTok)
		{
			case tPlus:    getToken(); multExpr(); put(MCODE_OP_ADD); break;
			case tMinus:   getToken(); multExpr(); put(MCODE_OP_SUB); break;
			default:
				return;
		}
}

static void shiftExpr()
{
	additionExpr();

	for (; ;)
		switch (currTok)
		{
			case tBitShl:  getToken(); additionExpr(); put(MCODE_OP_BITSHL);  break;
			case tBitShr:  getToken(); additionExpr(); put(MCODE_OP_BITSHR);  break;
			default:
				return;
		}
}

static void relation2Expr()
{
	shiftExpr();

	for (; ;)
		switch (currTok)
		{
			case tLt: getToken(); shiftExpr(); put(MCODE_OP_LT); break;
			case tLe: getToken(); shiftExpr(); put(MCODE_OP_LE); break;
			case tGt: getToken(); shiftExpr(); put(MCODE_OP_GT); break;
			case tGe: getToken(); shiftExpr(); put(MCODE_OP_GE); break;
			default:
				return;
		}
}

static void relationExpr()
{
	relation2Expr();

	for (; ;)
		switch (currTok)
		{
			case tEq: getToken(); relation2Expr(); put(MCODE_OP_EQ); break;
			case tNe: getToken(); relation2Expr(); put(MCODE_OP_NE); break;
			default:
				return;
		}
}

static void bitAndPrim()
{
	relationExpr();

	for (; ;)
		switch (currTok)
		{
			case tBitAnd:  getToken(); relationExpr(); put(MCODE_OP_BITAND); break;
			default:
				return;
		}
}

static void bitXorPrim()
{
	bitAndPrim();

	for (; ;)
		switch (currTok)
		{
			case tBitXor:  getToken(); bitAndPrim(); put(MCODE_OP_BITXOR);  break;
			default:
				return;
		}
}

static void bitOrPrim()
{
	bitXorPrim();

	for (; ;)
		switch (currTok)
		{
			case tBitOr:   getToken(); bitXorPrim(); put(MCODE_OP_BITOR);  break;
			default:
				return;
		}
}

static void boolAndPrim()
{
	bitOrPrim();

	for (; ;)
		switch (currTok)
		{
			case tBoolAnd: getToken(); bitOrPrim(); put(MCODE_OP_AND);    break;
			default:
				return;
		}
}


static void expr()
{
	boolAndPrim();

	for (; ;)
		switch (currTok)
		{
			case tBoolOr:  getToken(); boolAndPrim(); put(MCODE_OP_OR);     break;
			default:
				return;
		}
}

static int parseExpr(const wchar_t*& BufPtr, unsigned long *eBuff, wchar_t bound1, wchar_t bound2)
{
	wchar_t tmp[4];
	IsProcessFunc=0;
	_macro_ErrCode = exprBuffSize = _macro_nErr = 0;

	while (*BufPtr && iswspace(*BufPtr))
		BufPtr++;

	if (bound1)
	{
		pSrcString = oSrcString = sSrcString = (wchar_t*)BufPtr+(*BufPtr?1:0);

		if (*BufPtr != bound1)
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
#if 1
//!defined(TEST000)
	getToken();

	if (bound2)
		expr();
	else
		prim();

	BufPtr = oSrcString;

	while (*BufPtr && iswspace(*BufPtr))
		BufPtr++;

	if (bound2)
	{
		if ((*BufPtr != bound2) || !(!BufPtr[1] || iswspace(BufPtr[1]) || bound2 == L';'))
		{
			tmp[0] = bound2;
			tmp[1] = 0;
			keyMacroParseError(err_Expected, tmp);
			return 0;
		}

		BufPtr++;
	}

#else

	if (getToken() == tEnd)
		keyMacroParseError(err_Expr_Expected);
	else
	{
		if (bound2)
			expr();
		else
			prim();

		BufPtr = oSrcString;

		while (*BufPtr && iswspace(*BufPtr))
			BufPtr++;

		if (bound2)
		{
			if ((*BufPtr != bound2) || !(!BufPtr[1] || iswspace(BufPtr[1])))
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


static const wchar_t *__GetNextWord(const wchar_t *BufPtr,string &strCurKeyText)
{
	// пропускаем ведущие пробельные символы
	while (IsSpace(*BufPtr) || IsEol(*BufPtr))
	{
		if (IsEol(*BufPtr))
		{
			//TODO!!!
		}

		BufPtr++;
	}

	if (*BufPtr==0)
		return NULL;

	const wchar_t *CurBufPtr=BufPtr;
	wchar_t Chr=*BufPtr, Chr2=BufPtr[1];
	BOOL SpecMacro=Chr==L'$' && Chr2 && !(IsSpace(Chr2) || IsEol(Chr2));

	// ищем конец очередного названия клавиши
	while (Chr && !(IsSpace(Chr) || IsEol(Chr)))
	{
		if (SpecMacro && (Chr == L'[' || Chr == L'(' || Chr == L'{'))
			break;

		BufPtr++;
		Chr=*BufPtr;
	}

	int Length=(int)(BufPtr-CurBufPtr);
	wchar_t *CurKeyText = strCurKeyText.GetBuffer(Length+1);
	xwcsncpy(CurKeyText,CurBufPtr,Length);
	strCurKeyText.ReleaseBuffer();
	return BufPtr;
}

// Парсер строковых эквивалентов в коды клавиш
//- AN ----------------------------------------------
//  Парсер строковых эквивалентов в байткод
//  Переписан практически с нуля 15.11.2003
//- AN ----------------------------------------------

#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
static wchar_t *printfStr(DWORD* k, int& i)
{
	i++;
	wchar_t *s = (wchar_t *)&k[i];

	while (StrLength((wchar_t*)&k[i]) > 2)
		i++;

	return s;
}

static void printKeyValue(DWORD* k, int& i)
{
	DWORD Code=k[i];
	string _mcodename=_MCODE_ToName(Code);
	string cmt;

	if (Code >= MCODE_F_NOFUNC && Code <= KEY_MACRO_C_BASE-1)
	{
		for (int J=0; J <= countof(macroFunction); ++J)
			if (macroFunction[J].Code == Code)
			{
				cmt=macroFunction[J].Syntax;
				break;
			}
	}

	if (Code == MCODE_OP_KEYS)
	{
		string strTmp;
		SysLog(L"%08X: %08X | MCODE_OP_KEYS", i,MCODE_OP_KEYS);
		++i;

		while ((Code=k[i]) != MCODE_OP_ENDKEYS)
		{
			if (KeyToText(Code, strTmp))
				SysLog(L"%08X: %08X | Key: '%s'", i,Code,(const wchar_t*)strTmp);
			else
				SysLog(L"%08X: %08X | ???", i,Code);

			++i;
		}

		SysLog(L"%08X: %08X | MCODE_OP_ENDKEYS", i,MCODE_OP_ENDKEYS);
		return;
	}

	if (Code >= KEY_MACRO_BASE && Code <= KEY_MACRO_ENDBASE)
	{
		SysLog(L"%08X: %s  %s%s", i,_mcodename.CPtr(),(!cmt.IsEmpty()?L"# ":L""),(!cmt.IsEmpty()?(const wchar_t*)cmt:L""));
	}

	int ii = i;

	if (!Code)
	{
		SysLog(L"%08X: %08X | <null>", ii,k[i]);
	}
	else if (Code == MCODE_OP_REP)
	{
		LARGE_INTEGER i64;
		i64.u.HighPart=k[i+1];
		i64.u.LowPart=k[i+2];
		SysLog(L"%08X: %08X |   %I64d", ii,k[i+1],i64.QuadPart);
		SysLog(L"%08X: %08X |", ii,k[i+2]);
		i+=2;
	}
	else if (Code == MCODE_OP_PUSHINT)
	{
		LARGE_INTEGER i64;
		++i;
		i64.u.HighPart=k[i];
		i64.u.LowPart=k[i+1];
		SysLog(L"%08X: %08X |   %I64d", ++ii,k[i],i64.QuadPart);
		++i;
		SysLog(L"%08X: %08X |", ++ii,k[i]);
	}
	else if (Code == MCODE_OP_PUSHFLOAT)
	{
		LARGE_INTEGER i64;
		++i;
		i64.u.HighPart=k[i];
		i64.u.LowPart=k[i+1];
		double dval=*(double*)&i64;
		SysLog(L"%08X: %08X |   %lf", ++ii,k[i],dval);
		++i;
		SysLog(L"%08X: %08X |", ++ii,k[i]);
	}
	else if (Code == MCODE_OP_PUSHSTR || Code == MCODE_OP_PUSHVAR || Code == MCODE_OP_SAVE || Code == MCODE_OP_PUSHCONST)
	{
		int iii=i+1;
		const wchar_t *s=printfStr(k, i);

		if (Code == MCODE_OP_PUSHSTR || Code == MCODE_OP_PUSHCONST)
			SysLog(L"%08X: %08X |   \"%s\"", iii,k[iii], s);
		else
			SysLog(L"%08X: %08X |   %%%s", iii,k[iii], s);

		for (iii++; iii <= i; ++iii)
			SysLog(L"%08X: %08X |", iii,k[iii]);
	}
	else if (Code >= MCODE_OP_JMP && Code <= MCODE_OP_JGE)
	{
		++i;
		SysLog(L"%08X: %08X |   %08X (%s)", i,k[i],k[i],((DWORD)k[i]<(DWORD)i?L"up":L"down"));
	}
	/*
	  else if ( Code == MCODE_OP_DATE )
	  {
	    //sprint(ii, L"$date ''");
	  }
	  else if ( Code == MCODE_OP_PLAINTEXT )
	  {
	    //sprint(ii, L"$text ''");
	  }
	*/
	else if (k[i] < KEY_MACRO_BASE || k[i] > KEY_MACRO_ENDBASE)
	{
		int FARFunc = 0;

		for (int j = 0 ; j < MKeywordsSize ; j++)
		{
			if (k[i] == MKeywords[j].Value)
			{
				FARFunc = 1;
				SysLog(L"%08X: %08X | %s", ii,Code,MKeywords[j].Name);
				break;
			}
			else if (Code == MKeywordsFlags[j].Value)
			{
				FARFunc = 1;
				SysLog(L"%08X: %08X | %s", ii,Code,MKeywordsFlags[j].Name);
				break;
			}
		}

		if (!FARFunc)
		{
			string strTmp;

			if (KeyToText(k[i], strTmp))
				SysLog(L"%08X: %08X | Key: '%s'", ii,Code,(const wchar_t*)strTmp);
			else if (!cmt.IsEmpty())
				SysLog(L"%08X: %08X | ???", ii,Code);
		}
	}
}
#endif
#endif


//- AN ----------------------------------------------
//  Компиляция строки BufPtr в байткод CurMacroBuffer
//- AN ----------------------------------------------
int __parseMacroString(DWORD *&CurMacroBuffer, int &CurMacroBufferSize, const wchar_t *BufPtr)
{
	_KEYMACRO(CleverSysLog Clev(L"parseMacroString"));
	_KEYMACRO(SysLog(L"BufPtr[%p]='%s'", BufPtr,BufPtr));
	_macro_nErr = 0;
	pSrcString = oSrcString = sSrcString = emptyString;

	if (BufPtr == NULL || !*BufPtr)
	{
		keyMacroParseError(err_ZeroLengthMacro);
		return FALSE;
	}

	int SizeCurKeyText = (int)(StrLength(BufPtr)*2)*sizeof(wchar_t);
	string strCurrKeyText;
	//- AN ----------------------------------------------
	//  Буфер под парсинг выражений
	//- AN ----------------------------------------------
	DWORD *dwExprBuff = (DWORD*)xf_malloc(SizeCurKeyText*sizeof(DWORD));

	if (dwExprBuff == NULL)
		return FALSE;

	TExec exec;
	wchar_t varName[256];
	DWORD KeyCode, *CurMacro_Buffer = NULL;

	for (;;)
	{
		int Size = 1;
		int SizeVarName = 0;
		const wchar_t *oldBufPtr = BufPtr;

		if ((BufPtr = __GetNextWord(BufPtr, strCurrKeyText)) == NULL)
			break;

		_SVS(SysLog(L"strCurrKeyText=%s",(const wchar_t*)strCurrKeyText));

		//- AN ----------------------------------------------
		//  Проверка на строковый литерал
		//  Сделаем $Text опциональным
		//- AN ----------------------------------------------
		if (strCurrKeyText.At(0) == L'\"' && strCurrKeyText.At(1))
		{
			KeyCode = MCODE_OP_PLAINTEXT;
			BufPtr = oldBufPtr;
		}
		else if ((KeyCode = KeyNameMacroToKey(strCurrKeyText)) == (DWORD)-1 && (KeyCode = KeyNameToKey(strCurrKeyText)) == (DWORD)-1)
		{
			int ProcError=0;

			if (strCurrKeyText.At(0) == L'%' &&
			        (
			            (IsAlphaNum(strCurrKeyText.At(1)) || strCurrKeyText.At(1) == L'_') ||
			            (
			                strCurrKeyText.At(1) == L'%' &&
			                (IsAlphaNum(strCurrKeyText.At(2)) || strCurrKeyText.At(2)==L'_')
			            )
			        )
			   )
			{
				BufPtr = oldBufPtr;

				while (*BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)))
					BufPtr++;

				memset(varName, 0, sizeof(varName));
				KeyCode = MCODE_OP_SAVE;
				wchar_t* p = varName;
				const wchar_t* s = (const wchar_t*)strCurrKeyText+1;

				if (*s == L'%')
					*p++ = *s++;

				wchar_t ch;
				*p++ = *s++;

				while ((iswalnum(ch = *s++) || (ch == L'_')))
					*p++ = ch;

				*p = 0;
				int Length = (int)(StrLength(varName)+1)*sizeof(wchar_t);
				// строка должна быть выровнена на 4
				SizeVarName = Length/sizeof(DWORD);

				if (Length == sizeof(wchar_t) || (Length % sizeof(DWORD)) != 0)    // дополнение до sizeof(DWORD) нулями.
					SizeVarName++;

				_SVS(SysLog(L"BufPtr=%s",BufPtr));
				BufPtr += Length/sizeof(wchar_t);
				_SVS(SysLog(L"BufPtr=%s",BufPtr));
				Size += parseExpr(BufPtr, dwExprBuff, L'=', L';');

				if (_macro_nErr)
				{
					ProcError++;
				}
			}
			else
			{
				// проверим вариант, когда вызвали функцию, но результат не присвоили,
				// например, вызвали MsgBox(), но результат неважен
				// тогда SizeVarName=1 и varName=""
				int __nParam,__oParam;
				wchar_t *lpwszCurrKeyText = strCurrKeyText.GetBuffer();
				wchar_t *Brack=(wchar_t *)wcspbrk(lpwszCurrKeyText,L"( "), Chr=0;

				if (Brack)
				{
					Chr=*Brack;
					*Brack=0;
				}

				if (funcLook(lpwszCurrKeyText, __nParam, __oParam) != MCODE_F_NOFUNC)
				{
					if (Brack) *Brack=Chr;

					BufPtr = oldBufPtr;

					while (*BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)))
						BufPtr++;

					Size += parseExpr(BufPtr, dwExprBuff, 0, 0);

					/*
					// этого пока ненадо, считаем, что ';' идет сразу за функцией, иначе это отдельный символ ';', который нужно поместить в поток
					while ( *BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)) )
					  BufPtr++;
					*/
					if (*BufPtr == L';')
						BufPtr++; // здесь Size не увеличиваем, т.к. мы прокидываем символ ';'

					//Size--; //???
					if (_macro_nErr)
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
					if (Brack) *Brack=Chr;

					ProcError++;
				}

				strCurrKeyText.ReleaseBuffer();
			}

			if (ProcError)
			{
				if (!_macro_nErr)
					keyMacroParseError(err_Unrecognized_keyword, strCurrKeyText, strCurrKeyText,strCurrKeyText);

				if (CurMacro_Buffer != NULL)
				{
					xf_free(CurMacro_Buffer);
					CurMacroBuffer = NULL;
				}

				CurMacroBufferSize = 0;
				xf_free(dwExprBuff);
				return FALSE;
			}
		}
		else if (!(strCurrKeyText.At(0) == L'$' && strCurrKeyText.At(1)))
		{
			Size=3;
			KeyCode=MCODE_OP_KEYS;
		}

		switch (KeyCode)
		{
			case MCODE_OP_DATE:

				while (*BufPtr && IsSpace(*BufPtr))
					BufPtr++;

				if (*BufPtr == L'\"' && BufPtr[1])
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
				Size += parseExpr(BufPtr, dwExprBuff, L'(', L')');

				if (!exec.add(emmRep, CurMacroBufferSize+Size, CurMacroBufferSize+Size+4))   //??? 3
				{
					if (CurMacro_Buffer != NULL)
					{
						xf_free(CurMacro_Buffer);
						CurMacroBuffer = NULL;
					}

					CurMacroBufferSize = 0;
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
				Size += parseExpr(BufPtr, dwExprBuff, L'(', L')');

				if (!exec.add(emmThen, CurMacroBufferSize+Size))
				{
					if (CurMacro_Buffer != NULL)
					{
						xf_free(CurMacro_Buffer);
						CurMacroBuffer = NULL;
					}

					CurMacroBufferSize = 0;
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
				Size += parseExpr(BufPtr, dwExprBuff, L'(', L')');

				if (!exec.add(emmWhile, CurMacroBufferSize, CurMacroBufferSize+Size))
				{
					if (CurMacro_Buffer != NULL)
					{
						xf_free(CurMacro_Buffer);
						CurMacroBuffer = NULL;
					}

					CurMacroBufferSize = 0;
					xf_free(dwExprBuff);
					return FALSE;
				}

				Size++;
				break;
			case MCODE_OP_END:

				switch (exec().state)
				{
					case emmRep:
					case emmWhile:
						Size += 2; // Место под дополнительный JMP
						break;
				}

				break;
		}

		if (_macro_nErr)
		{
			if (CurMacro_Buffer != NULL)
			{
				xf_free(CurMacro_Buffer);
				CurMacroBuffer = NULL;
			}

			CurMacroBufferSize = 0;
			xf_free(dwExprBuff);
			return FALSE;
		}

		if (BufPtr == NULL)   // ???
			break;

		// код найден, добавим этот код в буфер последовательности.
		CurMacro_Buffer = (DWORD *)xf_realloc(CurMacro_Buffer,sizeof(*CurMacro_Buffer)*(CurMacroBufferSize+Size+SizeVarName));

		if (CurMacro_Buffer == NULL)
		{
			CurMacroBuffer = NULL;
			CurMacroBufferSize = 0;
			xf_free(dwExprBuff);
			return FALSE;
		}

		switch (KeyCode)
		{
			case MCODE_OP_DATE:
			case MCODE_OP_PLAINTEXT:
			case MCODE_OP_MACROMODE:
				_SVS(SysLog(L"[%d] Size=%u",__LINE__,Size));
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

				if (exec().state == emmThen)
				{
					exec().state = emmElse;
					CurMacro_Buffer[exec().pos1] = CurMacroBufferSize+2;
					exec().pos1 = CurMacroBufferSize;
					CurMacro_Buffer[CurMacroBufferSize] = 0;
				}
				else // тут $else и не предвиделось :-/
				{
					keyMacroParseError(err_Not_expected_ELSE, oldBufPtr+1, oldBufPtr); // strCurrKeyText

					if (CurMacro_Buffer != NULL)
					{
						xf_free(CurMacro_Buffer);
						CurMacroBuffer = NULL;
					}

					CurMacroBufferSize = 0;
					xf_free(dwExprBuff);
					return FALSE;
				}

				break;
			case MCODE_OP_END:

				switch (exec().state)
				{
					case emmMain:
						// тут $end и не предвиделось :-/
						keyMacroParseError(err_Not_expected_END, oldBufPtr+1, oldBufPtr); // strCurrKeyText

						if (CurMacro_Buffer != NULL)
						{
							xf_free(CurMacro_Buffer);
							CurMacroBuffer = NULL;
						}

						CurMacroBufferSize = 0;
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

				if (!exec.del())    // Вообще-то этого быть не должно,  но подстрахуемся
				{
					if (CurMacro_Buffer != NULL)
					{
						xf_free(CurMacro_Buffer);
						CurMacroBuffer = NULL;
					}

					CurMacroBufferSize = 0;
					xf_free(dwExprBuff);
					return FALSE;
				}

				break;
			case MCODE_OP_KEYS:
			{
				CurMacro_Buffer[CurMacroBufferSize+Size-3]=MCODE_OP_KEYS;
				CurMacro_Buffer[CurMacroBufferSize+Size-2]=KeyNameToKey(strCurrKeyText);
				CurMacro_Buffer[CurMacroBufferSize+Size-1]=MCODE_OP_ENDKEYS;
				break;
			}
			default:
				CurMacro_Buffer[CurMacroBufferSize]=KeyCode;
		} // end switch(KeyCode)

		CurMacroBufferSize += Size+SizeVarName;
	} // END for (;;)

	if (CurMacroBufferSize == 1)
	{
		CurMacro_Buffer = (DWORD *)xf_realloc(CurMacro_Buffer,sizeof(*CurMacro_Buffer)*(CurMacroBufferSize+1));

		if (CurMacro_Buffer == NULL)
		{
			CurMacroBuffer = NULL;
			CurMacroBufferSize = 0;
			xf_free(dwExprBuff);
			return FALSE;
		}

		CurMacro_Buffer[CurMacroBufferSize]=MCODE_OP_NOP;
		CurMacroBufferSize++;
	}

#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
	SysLogDump(L"Macro Buffer",0,(LPBYTE)CurMacro_Buffer,CurMacroBufferSize*sizeof(DWORD),NULL);
	SysLog(L"<ByteCode>{");

	if (CurMacro_Buffer)
	{
		int ii;

		for (ii = 0 ; ii < CurMacroBufferSize ; ii++)
			printKeyValue(CurMacro_Buffer, ii);
	}
	else
		SysLog(L"??? is NULL");

	SysLog(L"}</ByteCode>");
#endif
#endif

	if (CurMacroBufferSize > 1)
		CurMacroBuffer = CurMacro_Buffer;
	else if (CurMacro_Buffer)
	{
		CurMacroBuffer = reinterpret_cast<DWORD*>((DWORD_PTR)(*CurMacro_Buffer));
		xf_free(CurMacro_Buffer);
	}

	xf_free(dwExprBuff);

	if (exec().state != emmMain)
	{
		keyMacroParseError(err_Unexpected_EOS, strCurrKeyText, strCurrKeyText);
		return FALSE;
	}

	if (_macro_nErr)
		return FALSE;

	return TRUE;
}

BOOL __getMacroParseError(string *strErrMsg1,string *strErrMsg2,string *strErrMsg3)
{
	if (_macro_nErr)
	{
		if (strErrMsg1)
			*strErrMsg1 = ErrMessage[0];

		if (strErrMsg2)
			*strErrMsg2 = ErrMessage[1];

		if (strErrMsg3)
			*strErrMsg3 = ErrMessage[2];

		return TRUE;
	}

	return FALSE;
}

int  __getMacroErrorCode(int *nErr)
{
	if (nErr)
		*nErr=_macro_nErr;

	return _macro_ErrCode;
}
