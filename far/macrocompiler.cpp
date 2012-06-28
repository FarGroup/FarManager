/*
macrocompiler.cpp

Реализация парсера для MacroDrive II

*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#ifdef FAR_LUA
#else
//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------
// Парсер и "выполнятор" выражений
//---------------------------------------------------------------


#include "headers.hpp"
#pragma hdrstop

#include "macroopcode.hpp"
#include "macrocompiler.hpp"
#include "tvar.hpp"
#include "syslog.hpp"
#include "language.hpp"
#include "keyboard.hpp"
#include "macro.hpp"
#include "udlist.hpp"
#include "strmix.hpp"

#define EOFCH 65536

enum TToken
{
	tNo, tEnd,  tLet,
	tVar, tConst, tStr, tInt, tFunc, tFARVar, tFloat,
	tPlus, tMinus, tMul, tDiv, tLp, tRp, tComma,
	tBoolAnd, tBoolOr, tBoolXor,
	tBitAnd, tBitOr, tBitXor, tBitNot, tNot, tBitShl, tBitShr,
	tEq, tNe, tLt, tLe, tGt, tGe,
};

static int exprBuffSize = 0;
static unsigned long FARVar, *exprBuff = nullptr;
static int IsProcessFunc=0;

static string _ErrWord;
static string ErrMessage[4];
static int _macro_nErr = 0;
static int _macro_nLine = 0;
static int _macro_nPos = 0;
static int _macro_ErrCode=err_Success;
static int inloop = 0; // =1 мы в цикле
static wchar_t nameString[1024];
static wchar_t *sSrcString;
static const wchar_t *pSrcString = nullptr;
static wchar_t *oSrcString = nullptr;
static wchar_t emptyString[1]={};

static TToken currTok = tNo;
static TVar currVar;

static void expr();

#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
static void printKeyValue(DWORD* k, int& i);
#endif
#endif

static const wchar_t *__GetNextWord(const wchar_t *BufPtr,string &strCurKeyText,int& Line);
static void keyMacroParseError(int err, const wchar_t *s, const wchar_t *p, const wchar_t *c=nullptr);
static void keyMacroParseError(int err, const wchar_t *c = nullptr);

// Стек структурных операторов
#define MAXEXEXSTACK 256

enum TExecMode
{
	emmMain,
	emmThen,
	emmElse,
	emmWhile,
	emmRep,
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
		int findnearloop(TExecItem**);
		int add(TExecMode, DWORD, DWORD = 0);
		int del();
};

int TExec::findnearloop(TExecItem **found)
{
	for (int I=current; I >= 0; --I)
	{
		if (stack[I].state == emmRep || stack[I].state == emmWhile)
		{
			*found=stack+I;
			return I;
		}
	}
	return -1;
}

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
	size_t Length = (StrLength(s)+1)*sizeof(wchar_t);
	// строка должна быть выровнена на 4
	size_t nSize = Length/sizeof(DWORD);
	memmove(&exprBuff[exprBuffSize],s,Length);

	if (Length == sizeof(wchar_t) || (Length % sizeof(DWORD)) )    // дополнение до sizeof(DWORD) нулями.
		nSize++;

	memset(&exprBuff[exprBuffSize],0,nSize*sizeof(DWORD));
	memmove(&exprBuff[exprBuffSize],s,Length);
	exprBuffSize+=static_cast<int>(nSize);
}

static void keyMacroParseError(int err, const wchar_t *s, const wchar_t *p, const wchar_t *c)
{
	if (!_macro_nErr++)
	{
		_macro_ErrCode=err;

		while (*s && iswspace(*s))
		{
			// TODO: здесь нужно развернуть табуляции на заданное (чем?) количество пробелов
			s++;
		}

		int oPos = 0, ePos = (int)(s-p);
		ErrMessage[0]=ErrMessage[1]=ErrMessage[2]=ErrMessage[3]=L"";

		if (ePos < 0)
		{
			ErrMessage[0] = MSG(MMacroPErrIntParserError);
			_macro_ErrCode=err_IntParserError;
			return;
		}

		ErrMessage[0] = LangString(MMacroPErrUnrecognized_keyword+(err-1)) << c;
		_ErrWord=c;

		if (ePos > 61)
		{
			oPos = ePos-50;
			ErrMessage[1] = L"...";
		}

		ErrMessage[1] += p+oPos;
		//if ( ErrMessage[1][61] ) BUGBUG
		//	strncpy(&ErrMessage[1][61], "...",sizeof(ErrMessage[1])-62);
		_macro_nPos = ePos;
		InsertQuote(ErrMessage[1]);
		ErrMessage[2].Format(L"%*s%c", ePos-oPos+(oPos ? 3 : 0)+1, L"", L'\x2191');
		ErrMessage[3] = LangString(MMacroPErrorPosition) << _macro_nLine+1 << _macro_nPos+1;
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

		while (((ch = *(sSrcString++)) ) && iswspace(ch))
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

static DWORD funcLook(const wchar_t *s)
{
	static bool InitedInternalFuncs=false;
	if (!InitedInternalFuncs)
	{
		KeyMacro::RegisterMacroIntFunction();
		InitedInternalFuncs=true;
	}

	size_t CountMacroFunction=KeyMacro::GetCountMacroFunction();

	for (size_t I=0; I < CountMacroFunction; ++I)
	{
		const TMacroFunction *mFunc=KeyMacro::GetMacroFunction(I);
		if (!StrCmpNI(s, mFunc->Name, (int)Max(StrLength(mFunc->Name),StrLength(s))))
		{
			return (DWORD)mFunc->Code;
		}
	}

	return (DWORD)MCODE_F_NOFUNC;
}

static TToken getToken();

static void calcFunc()
{
	wchar_t nameString0[1024];

	TMacroOpCode nFunc = (TMacroOpCode)funcLook(nameString);

	if (nFunc != MCODE_F_NOFUNC)
	{
		IsProcessFunc++;

		int paramcount=0;

		for (;;)
		{
			xwcsncpy(nameString0,nameString,ARRAYSIZE(nameString));
			getToken();

			if (currTok != tRp)
				paramcount++;

			if ( currTok == tComma) // Mantis#0001863: Отсутствие строки как параметр функции
			{
				put(MCODE_OP_PUSHUNKNOWN);
				put64(0);
				continue;
			}

			expr();
			xwcsncpy(nameString,nameString0,ARRAYSIZE(nameString));

			if (currTok == tRp) break;

			if ( currTok != tComma )
			{
				keyMacroParseError(err_Expected_Token, L")");
				currTok = tEnd;
				break;
			}
		}

		put(MCODE_OP_PUSHINT);
		put64(paramcount);
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
	bool verbStr=false;

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
		case L'^':

			if ((ch = getChar()) == L'^')
				currTok = tBoolXor;
			else
			{
				putBack(ch);
				currTok = tBitXor;
			}

			break;
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

		case L'@':

			ch = getChar();
			if (ch != L'"')
			{
				putBack(ch);
				break;
			}
			verbStr=true;

    	case L'\"':
		{
			TToken __currTok = tNo;
			currVar = L"";

			while (((ch = getChar()) != EOFCH))
			{
				if (ch == L'\"')
				{
					if (verbStr)
					{
						ch = getChar();
						if (ch != L'\"')
						{
							putBack(ch);
							break;
						}
					}
					else
						break;
				}

				if (ch == L'\\' && !verbStr)
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
								keyMacroParseError(err_Bad_Hex_Control_Char,--sSrcString,pSrcString);
								__currTok = tEnd;
							}

							break;
						}
						default:
						{
							keyMacroParseError(err_Bad_Control_Char,--sSrcString,pSrcString);
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
				keyMacroParseError(err_Expected_Token, L")");

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

static void boolXorPrim()
{
	boolAndPrim();

	for (; ;)
		switch (currTok)
		{
			case tBoolXor: getToken(); boolAndPrim(); put(MCODE_OP_XOR);    break;
			default:
				return;
		}
}

static void expr()
{
	boolXorPrim();

	for (; ;)
		switch (currTok)
		{
			case tBoolOr:  getToken(); boolXorPrim(); put(MCODE_OP_OR);     break;
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
		/*pSrcString = */oSrcString = sSrcString = (wchar_t*)BufPtr+(*BufPtr?1:0);

		if (*BufPtr != bound1)
		{
			tmp[0] = bound1;
			tmp[1] = 0;
			keyMacroParseError(err_Expected_Token,--sSrcString,pSrcString,tmp);
			//keyMacroParseError(err_Expected_Token, tmp);
			return 0;
		}
	}
	else
		/*pSrcString = */oSrcString = sSrcString = (wchar_t*)BufPtr;

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
			keyMacroParseError(err_Expected_Token, tmp);
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
				keyMacroParseError(err_Expected_Token, tmp);
				return 0;
			}

			BufPtr++;
		}
	}

#endif
	return exprBuffSize;
}


static const wchar_t *__GetNextWord(const wchar_t *BufPtr,string &strCurKeyText,int& Line)
{
	// пропускаем ведущие пробельные символы
	while (IsSpace(*BufPtr) || IsEol(*BufPtr))
	{
		if (IsEol(*BufPtr))
		{
			Line++;//TODO!!!
		}

		BufPtr++;
	}

	if (!*BufPtr)
		return nullptr;

	const wchar_t *CurBufPtr=BufPtr;
	wchar_t Chr=*BufPtr, Chr2=BufPtr[1];
	BOOL SpecMacro=Chr==L'$' && Chr2 && !(IsSpace(Chr2) || IsEol(Chr2));

	// ищем конец очередного названия клавиши
	while (Chr && !(IsSpace(Chr) || IsEol(Chr)))
	{
		if (SpecMacro && (Chr == L'[' || Chr == L'(' || Chr == L'{'))
			break;

		if (IsEol(Chr))
		{
			Line++;//TODO!!!
		}

		BufPtr++;
		Chr=*BufPtr;
	}

	int Length=(int)(BufPtr-CurBufPtr);
	wchar_t *CurKeyText = strCurKeyText.GetBuffer(Length+1);
	xwcsncpy(CurKeyText,CurBufPtr,Length+1);
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

	if (Code >= MCODE_F_NOFUNC && Code <= KEY_MACRO_C_BASE-1 || Code >= KEY_MACRO_U_BASE && Code <= KEY_MACRO_ENDBASE-1)
	{
		size_t CountMacroFunction=KeyMacro::GetCountMacroFunction();
		for (size_t J=0; J < CountMacroFunction; ++J)
		{
			const TMacroFunction *mFunc=KeyMacro::GetMacroFunction(J);
			if (mFunc[J].Code == static_cast<TMacroOpCode>(Code))
			{
				cmt=mFunc[J].Syntax;
				break;
			}
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
				SysLog(L"%08X: %08X | Key: '%s'", i,Code,strTmp.CPtr());
			else
				SysLog(L"%08X: %08X | ???", i,Code);

			++i;
		}

		SysLog(L"%08X: %08X | MCODE_OP_ENDKEYS", i,MCODE_OP_ENDKEYS);
		return;
	}

	if (Code >= KEY_MACRO_BASE && Code <= KEY_MACRO_ENDBASE)
	{
		SysLog(L"%08X: %s  %s%s", i,_mcodename.CPtr(),(!cmt.IsEmpty()?L"# ":L""),(!cmt.IsEmpty()?cmt.CPtr():L""));
	}

	int ii = i;

	if (!Code)
	{
		SysLog(L"%08X: %08X | <null>", ii,k[i]);
	}
	else if (Code == MCODE_OP_REP)
	{
		LARGE_INTEGER i64 = {k[i+2], k[i+1]};
		SysLog(L"%08X: %08X |   %I64d", ii,k[i+1],i64.QuadPart);
		SysLog(L"%08X: %08X |", ii,k[i+2]);
		i+=2;
	}
	else if (Code == MCODE_OP_PUSHINT)
	{
		++i;
		LARGE_INTEGER i64 = {k[i+1], k[i]};
		SysLog(L"%08X: %08X |   %I64d", ++ii,k[i],i64.QuadPart);
		++i;
		SysLog(L"%08X: %08X |", ++ii,k[i]);
	}
	else if (Code == MCODE_OP_PUSHFLOAT)
	{
		++i;
		LARGE_INTEGER i64 = {k[i+1], k[i]};
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
				SysLog(L"%08X: %08X | Key: '%s'", ii,Code,strTmp.CPtr());
			else if (!cmt.IsEmpty())
				SysLog(L"%08X: %08X | ???", ii,Code);
		}
	}
}
#endif
#endif

static void CorrectBreakCode(DWORD *byteCodeBuff,DWORD *aBegin, DWORD *aEnd)
{
	for (DWORD *p=aBegin; p <= aEnd; ++p)
	{
		if (p[0] == MCODE_OP_BREAK && p[1] == 0)
		{
			p[0]=MCODE_OP_JMP; // меняем пвсевдо-код break на код jump
			p[1]=(DWORD)(aEnd-byteCodeBuff);
		}
	}
}

//- AN ----------------------------------------------
//  Компиляция строки BufPtr в байткод CurMacroBuffer
//- AN ----------------------------------------------
int __parseMacroString(DWORD *&CurMacroBuffer, int &CurMacroBufferSize, const wchar_t *BufPtr)
{
	_KEYMACRO(CleverSysLog Clev(L"parseMacroString"));
	//_KEYMACRO(SysLog(L"BufPtr[%p]='%s'", BufPtr,BufPtr));
	_macro_nErr = 0;
	_macro_nLine= 0;
	_macro_nPos = 0;
	pSrcString = emptyString;
	inloop = 0;
	/*pSrcString = */oSrcString = sSrcString = emptyString;

	if (!BufPtr || !*BufPtr)
	{
		keyMacroParseError(err_ZeroLengthMacro);
		return FALSE;
	}

	bool useUDL=true;
	const wchar_t *NewBufPtr;

	UserDefinedList MacroSrcList(ULF_NOTRIM|ULF_NOUNQUOTE, L"\r\n");
	if(!MacroSrcList.Set(BufPtr))
		useUDL=false; // все в одну строку
	//{
	//	_SVS(SysLog(L"MacroSrcList.GetTotal()=%d",MacroSrcList.GetTotal()));
	//	while((NewBufPtr=MacroSrcList.GetNext()) )
	//		_SVS(SysLog(L"[%s]",NewBufPtr));
	//	MacroSrcList.Reset();
	//}

	size_t SizeCurKeyText = (StrLength(BufPtr)*2)*sizeof(wchar_t);
	string strCurrKeyText;
	//- AN ----------------------------------------------
	//  Буфер под парсинг выражений
	//- AN ----------------------------------------------
	DWORD *dwExprBuff = (DWORD*)xf_malloc(SizeCurKeyText*sizeof(DWORD));

	if (!dwExprBuff)
		return FALSE;

	TExec exec;
	wchar_t varName[256];
	DWORD KeyCode, *CurMacro_Buffer = nullptr;

	if(useUDL)
		BufPtr=MacroSrcList.GetNext();

	pSrcString=BufPtr;

	for (;;)
	{
		int Size = 1;
		int SizeVarName = 0;
		const wchar_t *oldBufPtr = BufPtr;

		if (!(BufPtr = __GetNextWord(BufPtr, strCurrKeyText, _macro_nLine)))
		{
			if(!useUDL)
				break;
			NewBufPtr=MacroSrcList.GetNext();
			if(!NewBufPtr)
				break;
			_macro_nLine++;
			pSrcString=BufPtr=NewBufPtr;
			continue;
		}
		_SVS(SysLog(L"_macro_nLine   = [%d]",_macro_nLine));
		_SVS(SysLog(L"BufPtr         = [%s]",BufPtr));
		_SVS(SysLog(L"pSrcString     = [%s]",pSrcString));
		_SVS(SysLog(L"strCurrKeyText = [%s]",strCurrKeyText.CPtr()));

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
				{
					if (IsEol(*BufPtr))
					{
						_macro_nLine++;//TODO!!!
					}
					BufPtr++;
				}

				ClearArray(varName);
				KeyCode = MCODE_OP_SAVE;
				wchar_t* p = varName;
				const wchar_t* s = strCurrKeyText.CPtr()+1;

				if (*s == L'%')
					*p++ = *s++;

				wchar_t ch;
				*p++ = *s++;

				while ((iswalnum(ch = *s++) || (ch == L'_')))
					*p++ = ch;

				*p = 0;
				size_t Length = (StrLength(varName)+1)*sizeof(wchar_t);
				// строка должна быть выровнена на 4
				SizeVarName = static_cast<int>(Length/sizeof(DWORD));

				if (Length == sizeof(wchar_t) || (Length % sizeof(DWORD)) )    // дополнение до sizeof(DWORD) нулями.
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
				wchar_t *lpwszCurrKeyText = strCurrKeyText.GetBuffer();
				wchar_t *Brack=(wchar_t *)wcspbrk(lpwszCurrKeyText,L"( "), Chr=0;

				if (Brack)
				{
					Chr=*Brack;
					*Brack=0;
				}

				if (funcLook(lpwszCurrKeyText) != MCODE_F_NOFUNC)
				{
					if (Brack) *Brack=Chr;

					BufPtr = oldBufPtr;

					while (*BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)))
					{
						if (IsEol(*BufPtr))
						{
							_macro_nLine++;//TODO!!!
						}
						BufPtr++;
					}

					Size += parseExpr(BufPtr, dwExprBuff, 0, 0);

					/*
					// этого пока ненадо, считаем, что ';' идет сразу за функцией, иначе это отдельный символ ';', который нужно поместить в поток
					while ( *BufPtr && (IsSpace(*BufPtr) || IsEol(*BufPtr)) )
					{
						if (IsEol(*BufPtr))
						{
							_macro_nLine++;//TODO!!!
						}
						BufPtr++;
					}
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
						ClearArray(varName);
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
					//keyMacroParseError(err_Unrecognized_keyword, strCurrKeyText, strCurrKeyText,strCurrKeyText);
					keyMacroParseError(err_Unrecognized_keyword, oldBufPtr, pSrcString, strCurrKeyText);

				if (CurMacro_Buffer )
				{
					xf_free(CurMacro_Buffer);
					CurMacroBuffer = nullptr;
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
			// $Text
			// -------------------------------------
			//            MCODE_OP_PLAINTEXT
			//            <expr>
			case MCODE_OP_PLAINTEXT:
			{
				Size += parseExpr(BufPtr, dwExprBuff, 0, 0);
				break;
			}
			// $If (expr) ... $End
			// -------------------------------------
			//            <expr>
			//            MCODE_OP_JZ                     p1=*+0
			//            addr1        ------------+
			//            ...                      |
			//            MCODE_OP_JMP             |
			// +--------- addr2                    |
			// |          ...          <-----------+
			// +--------> MCODE_OP_END
			// или
			//            <expr>
			//            MCODE_OP_JZ                     p1=*+0
			//            addr1        ------------+
			//            ...                      |
			//            MCODE_OP_END <-----------+
			case MCODE_OP_IF:
			{
				Size += parseExpr(BufPtr, dwExprBuff, L'(', L')');

				if (!exec.add(emmThen, CurMacroBufferSize+Size))
				{
					if (CurMacro_Buffer )
					{
						xf_free(CurMacro_Buffer);
						CurMacroBuffer = nullptr;
					}

					CurMacroBufferSize = 0;
					xf_free(dwExprBuff);
					return FALSE;
				}

				Size++;
				break;
			}
			case MCODE_OP_ELSE:
			{
				Size++;
				break;
			}
			// $Rep (expr) ... $End
			// -------------------------------------
			//            <expr>
			//            MCODE_OP_SAVEREPCOUNT       1
			// +--------> MCODE_OP_REP                    p1=*
			// |          <counter>                   3
			// |          <counter>                   4
			// |          MCODE_OP_JZ                 5   p2=*+2
			// |          addr1        ------------+
			// |          ...                      |
			// |          MCODE_OP_JMP             |
			// +--------- addr2                    |
			//            MCODE_OP_END <-----------+
			case MCODE_OP_REP:
			{
				inloop++;
				Size += parseExpr(BufPtr, dwExprBuff, L'(', L')');

				if (!exec.add(emmRep, CurMacroBufferSize+Size, CurMacroBufferSize+Size+4))   //??? 3
				{
					if (CurMacro_Buffer )
					{
						xf_free(CurMacro_Buffer);
						CurMacroBuffer = nullptr;
					}

					CurMacroBufferSize = 0;
					xf_free(dwExprBuff);
					return FALSE;
				}

				Size += 5;  // естественно, размер будет больше = 4
				break;
			}
			// $While (expr) ... $End
			// -------------------------------------
			// +--------> <expr>
			// |          MCODE_OP_JZ                    CurMacroBufferSize + Size - 2
			// |          addr1        ------------+     CurMacroBufferSize + Size - 1
			// |          ...                      |     ...
			// |          MCODE_OP_JMP             |     CurMacroBufferSize + Size - 3
			// +--------- addr2                    |     CurMacroBufferSize + Size - 2
			//            MCODE_OP_END <-----------+     CurMacroBufferSize + Size - 1
			//                                           CurMacroBufferSize + Size
			case MCODE_OP_WHILE:
			{
				inloop++;
				Size += parseExpr(BufPtr, dwExprBuff, L'(', L')');

				if (!exec.add(emmWhile, CurMacroBufferSize, CurMacroBufferSize+Size))
				{
					if (CurMacro_Buffer )
					{
						xf_free(CurMacro_Buffer);
						CurMacroBuffer = nullptr;
					}

					CurMacroBufferSize = 0;
					xf_free(dwExprBuff);
					return FALSE;
				}

				Size++;
				break;
			}
			// $continue
			// -------------------------------------
			// ^          MCODE_OP_CONTINUE
			// |          MCODE_OP_JMP
			// +--------- addr
			case MCODE_OP_CONTINUE:
			{
				Size++; // Место под адрес
				break;
			}
			// $break
			// -------------------------------------
			//            MCODE_OP_BREAK
			//            MCODE_OP_JMP
			//            addr -->
			case MCODE_OP_BREAK:
			{
				Size++; // Место под адрес
				break;
			}
			case MCODE_OP_END:
			{
				switch (exec().state)
				{
					case emmRep:
					case emmWhile:
						Size += 2; // Место под дополнительный JMP
						break;
					default:
						break;
				}

				break;
			}
		}

		if (_macro_nErr)
		{
			if (CurMacro_Buffer )
			{
				xf_free(CurMacro_Buffer);
				CurMacroBuffer = nullptr;
			}

			CurMacroBufferSize = 0;
			xf_free(dwExprBuff);
			return FALSE;
		}

		if (!BufPtr)   // ???
			break;

		// код найден, добавим этот код в буфер последовательности.
		CurMacro_Buffer = (DWORD *)xf_realloc(CurMacro_Buffer,sizeof(*CurMacro_Buffer)*(CurMacroBufferSize+Size+SizeVarName));

		if (!CurMacro_Buffer)
		{
			CurMacroBuffer = nullptr;
			CurMacroBufferSize = 0;
			xf_free(dwExprBuff);
			return FALSE;
		}

		switch (KeyCode)
		{
			case MCODE_OP_PLAINTEXT:
			{
				_SVS(SysLog(L"[%d] Size=%u",__LINE__,Size));
				memcpy(CurMacro_Buffer+CurMacroBufferSize, dwExprBuff, Size*sizeof(DWORD));
				CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
				break;
			}
			case MCODE_OP_SAVE:
			{
				memcpy(CurMacro_Buffer+CurMacroBufferSize, dwExprBuff, Size*sizeof(DWORD));
				CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
				memcpy(CurMacro_Buffer+CurMacroBufferSize+Size, varName, SizeVarName*sizeof(DWORD));
				break;
			}
			case MCODE_OP_IF:
			{
				memcpy(CurMacro_Buffer+CurMacroBufferSize, dwExprBuff, Size*sizeof(DWORD));
				CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
				break;
			}
			case MCODE_OP_REP:
			{
				memcpy(CurMacro_Buffer+CurMacroBufferSize, dwExprBuff, Size*sizeof(DWORD));
				CurMacro_Buffer[CurMacroBufferSize+Size-6] = MCODE_OP_SAVEREPCOUNT;
				CurMacro_Buffer[CurMacroBufferSize+Size-5] = KeyCode;
				CurMacro_Buffer[CurMacroBufferSize+Size-4] = 0; // Initilize 0
				CurMacro_Buffer[CurMacroBufferSize+Size-3] = 0;
				CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
				break;
			}
			case MCODE_OP_WHILE:
			{
				memcpy(CurMacro_Buffer+CurMacroBufferSize, dwExprBuff, Size*sizeof(DWORD));
				CurMacro_Buffer[CurMacroBufferSize+Size-2] = MCODE_OP_JZ;
				break;
			}
			case MCODE_OP_ELSE:
			{
				if (exec().state == emmThen)
				{
					exec().state = emmElse;
					CurMacro_Buffer[exec().pos1] = CurMacroBufferSize+2;
					exec().pos1 = CurMacroBufferSize;
					CurMacro_Buffer[CurMacroBufferSize] = 0;
				}
				else // тут $else и не предвиделось :-/
				{
					keyMacroParseError(err_Not_expected_ELSE, BufPtr, pSrcString); // strCurrKeyText

					if (CurMacro_Buffer )
					{
						xf_free(CurMacro_Buffer);
						CurMacroBuffer = nullptr;
					}

					CurMacroBufferSize = 0;
					xf_free(dwExprBuff);
					return FALSE;
				}

				break;
			}
			case MCODE_OP_BREAK:
			case MCODE_OP_CONTINUE:
			{
				TExecItem *ei=nullptr;
				if (!inloop || (KeyCode==MCODE_OP_CONTINUE && exec.findnearloop(&ei) == -1))
				{
					keyMacroParseError(KeyCode==MCODE_OP_CONTINUE?err_Continue_Outside_The_Loop:err_Break_Outside_The_Loop, oldBufPtr, pSrcString);//BufPtr, pSrcString); // strCurrKeyText

					if (CurMacro_Buffer )
					{
						xf_free(CurMacro_Buffer);
						CurMacroBuffer = nullptr;
					}

					CurMacroBufferSize = 0;
					xf_free(dwExprBuff);
					return FALSE;
				}
				CurMacro_Buffer[CurMacroBufferSize+Size-2] = KeyCode==MCODE_OP_CONTINUE?MCODE_OP_JMP:MCODE_OP_BREAK;
				CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode==MCODE_OP_CONTINUE?ei->pos1:0;
				break;
			}
			case MCODE_OP_END:
			{
				switch (exec().state)
				{
					case emmMain:
						// тут $end и не предвиделось :-/
						keyMacroParseError(err_Not_expected_END, BufPtr, pSrcString); // strCurrKeyText

						if (CurMacro_Buffer )
						{
							xf_free(CurMacro_Buffer);
							CurMacroBuffer = nullptr;
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
						CurMacro_Buffer[exec().pos1-0] = MCODE_OP_JMP;
						CurMacro_Buffer[exec().pos1+1] = CurMacroBufferSize+Size-1;
						CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
						break;
					case emmRep:
					case emmWhile:
						inloop--;
						CurMacro_Buffer[exec().pos2] = CurMacroBufferSize+Size-1;
						CurMacro_Buffer[CurMacroBufferSize+Size-3] = MCODE_OP_JMP;
						CurMacro_Buffer[CurMacroBufferSize+Size-2] = exec().pos1;
						CurMacro_Buffer[CurMacroBufferSize+Size-1] = KeyCode;
						CorrectBreakCode(CurMacro_Buffer,CurMacro_Buffer+exec().pos1,CurMacro_Buffer+(CurMacroBufferSize+Size-1));
						break;
				}

				if (!exec.del())    // Вообще-то этого быть не должно,  но подстрахуемся
				{
					if (CurMacro_Buffer )
					{
						xf_free(CurMacro_Buffer);
						CurMacroBuffer = nullptr;
					}

					CurMacroBufferSize = 0;
					xf_free(dwExprBuff);
					return FALSE;
				}

				break;
			}
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

		if (!CurMacro_Buffer)
		{
			CurMacroBuffer = nullptr;
			CurMacroBufferSize = 0;
			xf_free(dwExprBuff);
			return FALSE;
		}

		CurMacro_Buffer[CurMacroBufferSize]=MCODE_OP_NOP;
		CurMacroBufferSize++;
	}

#ifdef _DEBUG
#ifdef SYSLOG_KEYMACRO
	SysLogDump(L"Macro Buffer",0,(LPBYTE)CurMacro_Buffer,CurMacroBufferSize*sizeof(DWORD),nullptr);
	SysLog(L"<ByteCode>{");

	if (CurMacro_Buffer)
	{
		int ii;

		for (ii = 0 ; ii < CurMacroBufferSize ; ii++)
			printKeyValue(CurMacro_Buffer, ii);
	}
	else
		SysLog(L"??? is nullptr");

	SysLog(L"}</ByteCode>");
#endif
#endif

	if (CurMacroBufferSize > 1)
		CurMacroBuffer = CurMacro_Buffer;
	else if (CurMacro_Buffer)
	{
		CurMacroBuffer = reinterpret_cast<DWORD*>((intptr_t)(*CurMacro_Buffer));
		xf_free(CurMacro_Buffer);
	}

	xf_free(dwExprBuff);

	if (exec().state != emmMain)
	{
		keyMacroParseError(err_Unexpected_EOS, strCurrKeyText, strCurrKeyText);
		return FALSE;
	}

	if (_macro_nErr)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL __getMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc)
{
	if (_macro_nErr)
	{
		if (ErrSrc)
			*ErrSrc = _ErrWord;

		if (ErrCode)
			*ErrCode = _macro_ErrCode;

		if (ErrPos)
		{
			ErrPos->X=(SHORT)_macro_nPos;
			ErrPos->Y=(SHORT)_macro_nLine;
		}

		return TRUE;
	}

	return FALSE;
}

BOOL __getMacroParseError(string *strErrMsg1,string *strErrMsg2,string *strErrMsg3,string *strErrMsg4)
{
	if (_macro_nErr)
	{
		if (strErrMsg1)
			*strErrMsg1 = ErrMessage[0];

		if (strErrMsg2)
			*strErrMsg2 = ErrMessage[1];

		if (strErrMsg3)
			*strErrMsg3 = ErrMessage[2];

		if (strErrMsg4)
			*strErrMsg4 = ErrMessage[3];

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

void  __setMacroErrorCode(int ErrCode)
{
	_macro_ErrCode=ErrCode;
}
#endif
