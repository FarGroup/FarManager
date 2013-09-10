/*
RegExp.cpp

Regular expressions
Syntax and semantics are very close to perl
*/
/*
Copyright © 2000 Konstantin Stupnik
Copyright © 2008 Far Group
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

#include "headers.hpp"
#pragma hdrstop

#include "RegExp.hpp"

#ifdef RE_DEBUG
#ifdef dpf
#undef dpf
#endif
#define dpf(x) printf x

char *ops[]=
{
	"opNone",
	"opLineStart",
	"opLineEnd",
	"opDataStart",
	"opDataEnd",
	"opWordBound",
	"opNotWordBound",
	"opType",
	"opNotType",
	"opCharAny",
	"opCharAnyAll",
	"opSymbol",
	"opNotSymbol",
	"opSymbolIgnoreCase",
	"opNotSymbolIgnoreCase",
	"opSymbolClass",
	"opOpenBracket",
	"opClosingBracket",
	"opAlternative",
	"opBackRef",
	"opRangesBegin",
	"opRange",
	"opMinRange",
	"opSymbolRange",
	"opSymbolMinRange",
	"opNotSymbolRange",
	"opNotSymbolMinRange",
	"opAnyRange",
	"opAnyMinRange",
	"opTypeRange",
	"opTypeMinRange",
	"opNotTypeRange",
	"opNotTypeMinRange",
	"opClassRange",
	"opClassMinRange",
	"opBracketRange",
	"opBracketMinRange",
	"opBackRefRange",
	"opBackRefMinRange",
	"opRangesEnd",
	"opAssertionsBegin",
	"opLookAhead",
	"opNotLookAhead",
	"opLookBehind",
	"opNotLookBehind",
	"opAsserionsEnd",
	"opNoReturn",
	"opRegExpEnd",
};

#else
#define dpf(x)
#endif


#define ISDIGIT(c) iswdigit(c)
#define ISSPACE(c) iswspace(c)
#define ISWORD(c)  (IsCharAlphaNumeric(c) || c=='_')
#define ISLOWER(c) IsCharLower(c)
#define ISUPPER(c) IsCharUpper(c)
#define ISALPHA(c) IsCharAlpha(c)
#define TOUPPER(c) ((wchar_t)(intptr_t)CharUpper((LPTSTR)(intptr_t)c))
#define TOLOWER(c) ((wchar_t)(intptr_t)CharLower((LPTSTR)(intptr_t)c))

bool isType(wchar_t chr,int type)
{
	switch (type)
	{
		case TYPE_DIGITCHAR:return ISDIGIT(chr) != 0;
		case TYPE_SPACECHAR:return ISSPACE(chr) != 0;
		case TYPE_WORDCHAR: return ISWORD(chr) != 0;
		case TYPE_LOWCASE:  return ISLOWER(chr) != 0;
		case TYPE_UPCASE:   return ISUPPER(chr) != 0;
		case TYPE_ALPHACHAR:return ISALPHA(chr) != 0;
	}

	return false;
}

struct UniSet
{
	unsigned char* high[256];
	char types;
	char nottypes;
	char negative;
	UniSet()
	{
		ClearArray(high);
		types=0;
		nottypes=0;
		negative=0;
	}
	UniSet(const UniSet& src)
	{
		for (int i=0; i<256; i++)
		{
			if (src.high[i])
			{
				high[i]=new unsigned char[32];
				memcpy(high[i],src.high[i],32);
			}
			else
			{
				high[i]=nullptr;
			}
		}

		types=src.types;
		nottypes=src.nottypes;
		negative=src.negative;
	}
	UniSet& operator=(const UniSet& src)
	{
		if (this != &src)
		{
			for (int i=0; i<256; i++)
			{
				if (src.high[i])
				{
					if (!high[i])high[i]=new unsigned char[32];

					memcpy(high[i],src.high[i],32);
				}
				else
				{
					if (high[i])delete [] high[i];

					high[i]=nullptr;
				}
			}

			types=src.types;
			nottypes=src.nottypes;
			negative=src.negative;
		}

		return (*this);
	}

	void Reset()
	{
		for (int i=0; i<256; i++)
		{
			if (high[i])
			{
				delete [] high[i];
				high[i]=0;
			}
		}

		types=0;
		nottypes=0;
		negative=0;
	}

	struct Setter
	{
		UniSet& set;
		wchar_t idx;
		Setter(UniSet& s,wchar_t chr):set(s),idx(chr)
		{
		}
		void operator=(int val)
		{
			if (val)set.SetBit(idx);
			else set.ClearBit(idx);
		}
		bool operator!()const
		{
			return !set.GetBit(idx);
		}
	};

	const bool operator[](wchar_t idx)const
	{
		return GetBit(idx);
	}
	Setter operator[](wchar_t idx)
	{
		return Setter(*this,idx);
	}
	~UniSet()
	{
		for (int i=0; i<256; i++)
		{
			if (high[i])delete [] high[i];
		}
	}
	bool CheckType(int t, wchar_t chr) const
	{
		switch (t)
		{
			case TYPE_DIGITCHAR:if (ISDIGIT(chr))return true; else break;
			case TYPE_SPACECHAR:if (ISSPACE(chr))return true; else break;
			case TYPE_WORDCHAR: if (ISWORD(chr)) return true; else break;
			case TYPE_LOWCASE:  if (ISLOWER(chr))return true; else break;
			case TYPE_UPCASE:   if (ISUPPER(chr))return true; else break;
			case TYPE_ALPHACHAR:if (ISALPHA(chr))return true; else break;
		}

		return false;
	}
	bool GetBit(wchar_t chr) const
	{
		if (types)
		{
			int t=TYPE_ALPHACHAR;

			while (t)
			{
				if (types&t)
				{
					if (CheckType(t,chr))
						return !negative;
				}

				t>>=1;
			}
		}

		if (nottypes)
		{
			int t=TYPE_ALPHACHAR;

			while (t)
			{
				if (nottypes&t)
				{
					if (!CheckType(t,chr))
						return !negative;
				}

				t>>=1;
			}
		}

		unsigned char h=(chr&0xff00)>>8;

		if (!high[h]) return negative != 0;

		if (((high[h][(chr&0xff)>>3]&(1<<(chr&7)))?1:0))
		{
			return !negative;
		}

		return negative != 0;
	}
	void SetBit(wchar_t  chr)
	{
		unsigned char h=(chr&0xff00)>>8;

		if (!high[h])
		{
			high[h]=new unsigned char[32]();
		}

		high[h][(chr&0xff)>>3]|=1<<(chr&7);
	}
	void ClearBit(wchar_t  chr)
	{
		unsigned char h=(chr&0xff00)>>8;

		if (!high[h])
		{
			high[h]=new unsigned char[32]();
		}

		high[h][(chr&0xff)>>3]&=~(1<<(chr&7));
	}

};

enum REOp
{
	opLineStart=0x1,        // ^
	opLineEnd,              // $
	opDataStart,            // \A and ^ in single line mode
	opDataEnd,              // \Z and $ in signle line mode

	opWordBound,            // \b
	opNotWordBound,         // \B

	opType,                 // \d\s\w\l\u\e
	opNotType,              // \D\S\W\L\U\E

	opCharAny,              // .
	opCharAnyAll,           // . in single line mode

	opSymbol,               // single char
	opNotSymbol,            // [^c] negative charclass with one char
	opSymbolIgnoreCase,     // symbol with IGNORE_CASE turned on
	opNotSymbolIgnoreCase,  // [^c] with ignore case set.

	opSymbolClass,          // [chars]

	opOpenBracket,          // (

	opClosingBracket,       // )

	opAlternative,          // |

	opBackRef,              // \1

	opRangesBegin,          // for op type check

	opRange,                // generic range
	opMinRange,             // generic minimizing range

	opSymbolRange,          // quantifier applied to single char
	opSymbolMinRange,       // minimizing quantifier

	opNotSymbolRange,       // [^x]
	opNotSymbolMinRange,

	opAnyRange,             // .
	opAnyMinRange,

	opTypeRange,            // \w, \d, \s
	opTypeMinRange,

	opNotTypeRange,         // \W, \D, \S
	opNotTypeMinRange,

	opClassRange,           // for char classes
	opClassMinRange,

	opBracketRange,         // for brackets
	opBracketMinRange,

	opBackRefRange,         // for backrefs
	opBackRefMinRange,

	opRangesEnd,            // end of ranges

	opAssertionsBegin,

	opLookAhead,
	opNotLookAhead,

	opLookBehind,
	opNotLookBehind,

	opAsserionsEnd,

	opNoReturn,

	opRegExpEnd
};

struct REOpCode
{
	int op;
	REOpCode *next,*prev;
#ifdef RE_DEBUG
	int    srcpos;
#endif
	REOpCode()
	{
		ClearStructUnsafe(*this);
	}
	~REOpCode();

	struct SBracket
	{
		REOpCode* nextalt;
		int index;
		REOpCode* pairindex;
	};

	struct SRange
	{
		union
		{
			SBracket bracket;
			int op;
			wchar_t symbol;
			UniSet *symbolclass;
			REOpCode* nextalt;
			int refindex;
			int type;
		};
		int min,max;
	};

	struct SNamedBracket
	{
		REOpCode* nextalt;
		REOpCode* pairindex;
	};

	struct SAssert
	{
		REOpCode* nextalt;
		int length;
		REOpCode* pairindex;
	};

	struct SAlternative
	{
		REOpCode* nextalt;
		REOpCode* endindex;
	};


	union
	{
		SRange range;
		SBracket bracket;
		SAssert assert;
		SAlternative alternative;
		wchar_t symbol;
		UniSet *symbolclass;
		int refindex;
		int type;
	};
};

REOpCode::~REOpCode()
{
	switch (op)
	{
		case opSymbolClass:delete symbolclass; break;
		case opClassRange:
		case opClassMinRange:delete range.symbolclass; break;
	}
}


void RegExp::Init(const wchar_t* expr,int options)
{
	code=nullptr;
	brhandler=nullptr;
	brhdata=nullptr;
	stack=&initstack[0];
	st=&stack[0];
	initstackpage.stack=stack;
	firstpage=lastpage=&initstackpage;
	firstpage->next=nullptr;
	firstpage->prev=nullptr;
	firstptr=new UniSet();
#define first (*firstptr)
	start=nullptr;
	end=nullptr;
	trimend=nullptr;
	Compile(expr,options);
}

RegExp::RegExp():
	code(nullptr),
	stack(&initstack[0]),
	st(&stack[0]),
	slashChar('/'),
	backslashChar('\\'),
	firstpage(&initstackpage),
	lastpage(&initstackpage),
	firstptr(new UniSet()),
	errorcode(errNotCompiled),
	start(nullptr),
	end(nullptr),
	trimend(nullptr),
#ifdef RE_DEBUG
	resrc(nullptr),
#endif
	brhandler(nullptr),
	brhdata(nullptr)
{
	initstackpage.stack=stack;
	firstpage->next=nullptr;
	firstpage->prev=nullptr;
}

RegExp::RegExp(const wchar_t* expr,int options)
{
	slashChar='/';
	backslashChar='\\';
#ifdef RE_DEBUG
	resrc=nullptr;
#endif
	Init(expr,options);
}

RegExp::~RegExp()
{
#ifdef RE_DEBUG
	delete [] resrc;
#endif

	if (code)
	{
		delete [] code;
		code=nullptr;
	}

	CleanStack();
	delete firstptr;
}

int RegExp::CalcLength(const wchar_t* src,int srclength)
{
	int length=3;//global brackets
	int brackets[MAXDEPTH];
	int count=0;
	int i,save;
	bracketscount=1;
	int inquote=0;

	for (i=0; i<srclength; i++,length++)
	{
		if (inquote && src[i]!=backslashChar && src[i+1]!='E')
		{
			continue;
		}

		if (src[i]==backslashChar)
		{
			i++;

			if (src[i]=='Q')inquote=1;

			if (src[i]=='E')inquote=0;

			if (src[i]=='x')
			{
				i++;
				if(isxdigit(src[i]))
				{
					for(int j=1,k=i;j<4;j++)
					{
						if(isxdigit(src[k+j]))
						{
							i++;
						}
						else
						{
							break;
						}
					}
				}
				else return SetError(errSyntax,i);
			}
			continue;
		}

		switch (src[i])
		{
			case '(':
			{
				brackets[count]=i;
				count++;

				if (count==MAXDEPTH)return SetError(errMaxDepth,i);

				if (src[i+1]=='?')
				{
					i+=2;
				}
				else
				{
					bracketscount++;
				}

				break;
			}
			case ')':
			{
				count--;

				if (count<0)return SetError(errBrackets,i);

				break;
			}
			case '{':
			case '*':
			case '+':
			case '?':
			{
				length--;

				if (src[i]=='{')
				{
					save=i;

					while (i<srclength && src[i]!='}')i++;

					if (i>=srclength)return SetError(errBrackets,save);
				}

				if (src[i+1]=='?')i++;

				break;
			}
			case '[':
			{
				save=i;

				while (i<srclength && src[i]!=']')i++;

				if (i>=srclength)return SetError(errBrackets,save);

				break;
			}
		}
	}

	if (count)
	{
		errorpos=brackets[0];
		errorcode=errBrackets;
		return 0;
	}

	return length;
}

int RegExp::Compile(const wchar_t* src,int options)
{
	int srcstart=0,srclength/*=0*/,relength;

	if (options&OP_CPPMODE)
	{
		slashChar='\\';
		backslashChar='/';
	}
	else
	{
		slashChar='/';
		backslashChar='\\';
	}

	havefirst=0;
	if (code)delete [] code;

	code=nullptr;

	if (options&OP_PERLSTYLE)
	{
		if (src[0]!=slashChar)return SetError(errSyntax,0);

		srcstart=1;
		srclength=1;

		while (src[srclength] && src[srclength]!=slashChar)
		{
			if (src[srclength]==backslashChar && src[srclength+1])
			{
				srclength++;
			}

			srclength++;
		}

		if (!src[srclength])
		{
			return SetError(errSyntax,srclength-1);
		}

		int i=srclength+1;
		srclength--;

		while (src[i])
		{
			switch (src[i])
			{
				case 'i':options|=OP_IGNORECASE; break;
				case 's':options|=OP_SINGLELINE; break;
				case 'm':options|=OP_MULTILINE; break;
				case 'x':options|=OP_XTENDEDSYNTAX; break;
				case 'o':options|=OP_OPTIMIZE; break;
				default:return SetError(errOptions,i);
			}

			i++;
		}
	}
	else
	{
		srclength=(int)wcslen(src);
	}

	ignorecase=options&OP_IGNORECASE?1:0;
	relength=CalcLength(src+srcstart,srclength);

	if (!relength)
	{
		return 0;
	}

	code=new REOpCode[relength]();

	for (int i=0; i<relength; i++)
	{
		code[i].next=i<relength-1?code+i+1:0;
		code[i].prev=i>0?code+i-1:0;
	}

	int result=InnerCompile(src+srcstart,srclength,options);

	if (!result)
	{
		delete [] code;
		code=nullptr;
	}
	else
	{
		errorcode=errNone;
		minlength=0;

		if (options&OP_OPTIMIZE)Optimize();
	}

	return result;
}

int RegExp::GetNum(const wchar_t* src,int& i)
{
	int res=0;

	while (ISDIGIT(src[i]))
	{
		res*=10;
		res+=src[i]-'0';
		i++;
	}

	return res;
}

static int CalcPatternLength(PREOpCode from, const PREOpCode to)
{
	int len=0;
	int altcnt=0;
	int altlen=-1;

	for (; from->prev!=to; from=from->next)
	{
		switch (from->op)
		{
				//zero width
			case opLineStart:
			case opLineEnd:
			case opDataStart:
			case opDataEnd:
			case opWordBound:
			case opNotWordBound:continue;
			case opType:
			case opNotType:
			case opCharAny:
			case opCharAnyAll:
			case opSymbol:
			case opNotSymbol:
			case opSymbolIgnoreCase:
			case opNotSymbolIgnoreCase:
			case opSymbolClass:
				len++;
				altcnt++;
				continue;
			case opOpenBracket:
			{
				int l=CalcPatternLength(from->next,from->bracket.pairindex->prev);

				if (l==-1)return -1;

				len+=l;
				altcnt+=l;
				from=from->bracket.pairindex;
				continue;
			}
			case opClosingBracket:
				break;
			case opAlternative:

				if (altlen!=-1 && altcnt!=altlen)return -1;

				altlen=altcnt;
				altcnt=0;
				continue;
			case opBackRef:
				return -1;
			case opRangesBegin:
			case opRange:
			case opMinRange:
			case opSymbolRange:
			case opSymbolMinRange:
			case opNotSymbolRange:
			case opNotSymbolMinRange:
			case opAnyRange:
			case opAnyMinRange:
			case opTypeRange:
			case opTypeMinRange:
			case opNotTypeRange:
			case opNotTypeMinRange:
			case opClassRange:
			case opClassMinRange:

				if (from->range.min!=from->range.max)return -1;

				len+=from->range.min;
				altcnt+=from->range.min;
				continue;
			case opBracketRange:
			case opBracketMinRange:
			{
				if (from->range.min!=from->range.max)return -1;

				int l=CalcPatternLength(from->next,from->bracket.pairindex->prev);

				if (l==-1)return -1;

				len+=from->range.min*l;
				altcnt+=from->range.min*l;
				from=from->bracket.pairindex;
				continue;
			}
			case opBackRefRange:
			case opBackRefMinRange:
				return -1;
			case opRangesEnd:
			case opAssertionsBegin:
			case opLookAhead:
			case opNotLookAhead:
			case opLookBehind:
			case opNotLookBehind:
				from=from->assert.pairindex;
				continue;
			case opAsserionsEnd:
			case opNoReturn:
				continue;
		}
	}

	if (altlen!=-1 && altlen!=altcnt)return -1;

	return altlen==-1?len:altlen;
}

int RegExp::InnerCompile(const wchar_t* src,int srclength,int options)
{
	int i,j;
	PREOpCode brackets[MAXDEPTH];
	// current brackets depth
	// one place reserved for surrounding 'main' brackets
	int brdepth=1;
	// compiling interior of lookbehind
	// used to apply restrictions of lookbehind
	int lookbehind=0;
	// counter of normal brackets
	int brcount=0;
	// counter of closed brackets
	// used to check correctness of backreferences
	bool closedbrackets[MAXDEPTH];
	// quoting is active
	int inquote=0;
	maxbackref=0;
	UniSet *tmpclass;
	code->op=opOpenBracket;
	code->bracket.index=0;
	int pos=1;
	register PREOpCode op;//=code;
	brackets[0]=code;
#ifdef RE_DEBUG
	resrc=new wchar_t[srclength+4];
	resrc[0]='(';
	resrc[1]=0;
	memcpy(resrc+1,src,srclength*sizeof(wchar_t));
	resrc[srclength+1]=')';
	resrc[srclength+2]=27;
	resrc[srclength+3]=0;
#endif
	havelookahead=0;

	for (i=0; i<srclength; i++)
	{
		op=code+pos;
		pos++;
#ifdef RE_DEBUG
		op->srcpos=i+1;
#endif

		if (inquote && src[i]!=backslashChar)
		{
			op->op=ignorecase?opSymbolIgnoreCase:opSymbol;
			op->symbol=ignorecase?TOLOWER(src[i]):src[i];

			if (ignorecase && TOUPPER(op->symbol)==op->symbol)op->op=opSymbol;

			continue;
		}

		if (src[i]==backslashChar)
		{
			i++;

			if (inquote && src[i]!='E')
			{
				op->op=opSymbol;
				op->symbol=backslashChar;
				op=code+pos;
				pos++;
				op->op=ignorecase?opSymbolIgnoreCase:opSymbol;
				op->symbol=ignorecase?TOLOWER(src[i]):src[i];

				if (ignorecase && TOUPPER(op->symbol)==op->symbol)op->op=opSymbol;

				continue;
			}

			op->op=opType;

			switch (src[i])
			{
				case 'Q':inquote=1; pos--; continue;
				case 'E':inquote=0; pos--; continue;
				case 'b':op->op=opWordBound; continue;
				case 'B':op->op=opNotWordBound; continue;
				case 'D':op->op=opNotType;
				case 'd':op->type=TYPE_DIGITCHAR; continue;
				case 'S':op->op=opNotType;
				case 's':op->type=TYPE_SPACECHAR; continue;
				case 'W':op->op=opNotType;
				case 'w':op->type=TYPE_WORDCHAR; continue;
				case 'U':op->op=opNotType;
				case 'u':op->type=TYPE_UPCASE; continue;
				case 'L':op->op=opNotType;
				case 'l':op->type=TYPE_LOWCASE; continue;
				case 'I':op->op=opNotType;
				case 'i':op->type=TYPE_ALPHACHAR; continue;
				case 'A':op->op=opDataStart; continue;
				case 'Z':op->op=opDataEnd; continue;
				case 'n':op->op=opSymbol; op->symbol='\n'; continue;
				case 'r':op->op=opSymbol; op->symbol='\r'; continue;
				case 't':op->op=opSymbol; op->symbol='\t'; continue;
				case 'f':op->op=opSymbol; op->symbol='\f'; continue;
				case 'e':op->op=opSymbol; op->symbol=27; continue;
				case 'O':op->op=opNoReturn; continue;
				case 'x':
				{
					i++;

					if (i>=srclength)return SetError(errSyntax,i-1);

					if(isxdigit(src[i]))
					{
						int c=TOLOWER(src[i])-'0';

						if (c>9)c-='a'-'0'-10;

						op->op=ignorecase?opSymbolIgnoreCase:opSymbol;
						op->symbol=c;
						for(int j=1,k=i;j<4 && k+j<srclength;j++)
						{
							if(isxdigit(src[k+j]))
							{
								i++;
								c=TOLOWER(src[k+j])-'0';
								if (c>9)c-='a'-'0'-10;
								op->symbol<<=4;
								op->symbol|=c;
							}
							else
							{
								break;
							}
						}
						if (ignorecase)
						{
							op->symbol=TOLOWER(op->symbol);
							if (TOUPPER(op->symbol)==TOLOWER(op->symbol))
							{
								op->op=opSymbol;
							}
						}
					}
					else return SetError(errSyntax,i);

					continue;
				}
				default:
				{
					if (ISDIGIT(src[i]))
					{
						int save=i;
						op->op=opBackRef;
						op->refindex=GetNum(src,i); i--;

						if (op->refindex<=0 || op->refindex>brcount || !closedbrackets[op->refindex])
						{
							return SetError(errInvalidBackRef,save-1);
						}

						if (op->refindex>maxbackref)maxbackref=op->refindex;
					}
					else
					{
						if (options&OP_STRICT && ISALPHA(src[i]))
						{
							return SetError(errInvalidEscape,i-1);
						}

						op->op=ignorecase?opSymbolIgnoreCase:opSymbol;
						op->symbol=ignorecase?TOLOWER(src[i]):src[i];

						if (TOLOWER(op->symbol)==TOUPPER(op->symbol))
						{
							op->op=opSymbol;
						}
					}
				}
			}

			continue;
		}

		switch (src[i])
		{
			case '.':
			{
				if (options&OP_SINGLELINE)
				{
					op->op=opCharAnyAll;
				}
				else
				{
					op->op=opCharAny;
				}

				continue;
			}
			case '^':
			{
				if (options&OP_MULTILINE)
				{
					op->op=opLineStart;
				}
				else
				{
					op->op=opDataStart;
				}

				continue;
			}
			case '$':
			{
				if (options&OP_MULTILINE)
				{
					op->op=opLineEnd;
				}
				else
				{
					op->op=opDataEnd;
				}

				continue;
			}
			case '|':
			{
				if (brackets[brdepth-1]->op==opAlternative)
				{
					brackets[brdepth-1]->alternative.nextalt=op;
				}
				else
				{
					if (brackets[brdepth-1]->op==opOpenBracket)
					{
						brackets[brdepth-1]->bracket.nextalt=op;
					}
					else
					{
						brackets[brdepth-1]->assert.nextalt=op;
					}
				}

				if (brdepth==MAXDEPTH)return SetError(errMaxDepth,i);

				brackets[brdepth++]=op;
				op->op=opAlternative;
				continue;
			}
			case '(':
			{
				op->op=opOpenBracket;

				if (src[i+1]=='?')
				{
					i+=2;

					switch (src[i])
					{
						case ':':op->bracket.index=-1; break;
						case '=':op->op=opLookAhead; havelookahead=1; break;
						case '!':op->op=opNotLookAhead; havelookahead=1; break;
						case '<':
						{
							i++;

							if (src[i]=='=')
							{
								op->op=opLookBehind;
							}
							else if (src[i]=='!')
							{
								op->op=opNotLookBehind;
							}
							else return SetError(errSyntax,i);

							lookbehind++;
						} break;
						default:
						{
							return SetError(errSyntax,i);
						}
					}
				}
				else
				{
					brcount++;
					closedbrackets[brcount]=false;
					op->bracket.index=brcount;
				}

				brackets[brdepth]=op;
				brdepth++;
				continue;
			}
			case ')':
			{
				op->op=opClosingBracket;
				brdepth--;

				while (brackets[brdepth]->op==opAlternative)
				{
					brackets[brdepth]->alternative.endindex=op;
					brdepth--;
				}

				switch (brackets[brdepth]->op)
				{
					case opOpenBracket:
					{
						op->bracket.pairindex=brackets[brdepth];
						brackets[brdepth]->bracket.pairindex=op;
						op->bracket.index=brackets[brdepth]->bracket.index;

						if (op->bracket.index!=-1)
						{
							closedbrackets[op->bracket.index]=true;
						}

						break;
					}
					case opLookBehind:
					case opNotLookBehind:
					{
						lookbehind--;
						int l=CalcPatternLength(brackets[brdepth]->next,op->prev);

						if (l==-1)return SetError(errVariableLengthLookBehind,i);

						brackets[brdepth]->assert.length=l;
					}// there is no break and this is correct!
					case opLookAhead:
					case opNotLookAhead:
					{
						op->assert.pairindex=brackets[brdepth];
						brackets[brdepth]->assert.pairindex=op;
						break;
					}
				}

				continue;
			}
			case '[':
			{
				i++;
				int negative=0;

				if (src[i]=='^')
				{
					negative=1;
					i++;
				}

				int lastchar=-1;
				int classsize=0;
				op->op=opSymbolClass;
				//op->symbolclass=new wchar_t[32]();
				op->symbolclass=new UniSet();
				tmpclass=op->symbolclass;

				for (; src[i]!=']'; i++)
				{
					if (src[i]==backslashChar)
					{
						i++;
						int isnottype=0;
						int type=0;
						lastchar=-1;

						switch (src[i])
						{
							case 'D':isnottype=1;
							case 'd':type=TYPE_DIGITCHAR; break;
							case 'W':isnottype=1;
							case 'w':type=TYPE_WORDCHAR; break;
							case 'S':isnottype=1;
							case 's':type=TYPE_SPACECHAR; break;
							case 'L':isnottype=1;
							case 'l':type=TYPE_LOWCASE; break;
							case 'U':isnottype=1;
							case 'u':type=TYPE_UPCASE; break;
							case 'I':isnottype=1;
							case 'i':type=TYPE_ALPHACHAR; break;
							case 'n':lastchar='\n'; break;
							case 'r':lastchar='\r'; break;
							case 't':lastchar='\t'; break;
							case 'f':lastchar='\f'; break;
							case 'e':lastchar=27; break;
							case 'x':
							{
								i++;

								if (i>=srclength)return SetError(errSyntax,i-1);

								if (isxdigit(src[i]))
								{
									int c=TOLOWER(src[i])-'0';

									if (c>9)c-='a'-'0'-10;

									lastchar=c;

									for(int j=1,k=i;j<4 && k+j<srclength;j++)
									{
										if (isxdigit(src[k+j]))
										{
											i++;
											c=TOLOWER(src[k+j])-'0';

											if (c>9)c-='a'-'0'-10;

											lastchar<<=4;
											lastchar|=c;
										}
										else
										{
											break;
										}
									}
									dpf(("Last char=%c(%02x)\n",lastchar,lastchar));
								}
								else return SetError(errSyntax,i);

								break;
							}
							default:
							{
								if (options&OP_STRICT && ISALPHA(src[i]))
								{
									return SetError(errInvalidEscape,i-1);
								}

								lastchar=src[i];
							}
						}

						if (type)
						{
							if (isnottype)
							{
								tmpclass->nottypes|=type;
							}
							else
							{
								tmpclass->types|=type;
							}
							classsize=257;
							//for(int j=0;j<32;j++)op->symbolclass[j]|=charbits[classindex+j]^isnottype;
							//classsize+=charsizes[classindex>>5];
							//int setbit;
							/*for(int j=0;j<256;j++)
							{
							  setbit=(chartypes[j]^isnottype)&type;
							  if(setbit)
							  {
							    if(ignorecase)
							    {
							      SetBit(op->symbolclass,lc[j]);
							      SetBit(op->symbolclass,uc[j]);
							    }else
							    {
							      SetBit(op->symbolclass,j);
							    }
							    classsize++;
							  }
							}*/
						}
						else
						{
							if (options&OP_IGNORECASE)
							{
								tmpclass->SetBit(TOLOWER(lastchar));
								tmpclass->SetBit(TOUPPER(lastchar));
							}
							else
							{
								tmpclass->SetBit(lastchar);
							}

							classsize++;
						}

						continue;
					}

					if (src[i]=='-')
					{
						if (lastchar!=-1 && src[i+1]!=']')
						{
							int to=src[i+1];

							if (to==backslashChar)
							{
								to=src[i+2];

								if (to=='x')
								{
									i+=2;
									to=TOLOWER(src[i+1]);

									if(isxdigit(to))
									{
										to-='0';

										if (to>9)to-='a'-'0'-10;

										for(int j=1,k=(i+1);j<4 && k+j<srclength;j++)
										{
											int c=TOLOWER(src[k+j]);
											if(isxdigit(c))
											{
												i++;
												c-='0';

												if (c>9)c-='a'-'0'-10;

												to<<=4;
												to|=c;
											}
											else
											{
												break;
											}
										}
									}
									else return SetError(errSyntax,i);
								}
								else
								{
									tmpclass->SetBit('-');
									classsize++;
									continue;
								}
							}

							i++;
							dpf(("from %d to %d\n",lastchar,to));

							for (j=lastchar; j<=to; j++)
							{
								if (ignorecase)
								{
									tmpclass->SetBit(TOLOWER(j));
									tmpclass->SetBit(TOUPPER(j));
								}
								else
								{
									tmpclass->SetBit(j);
								}

								classsize++;
							}

							continue;
						}
					}

					lastchar=src[i];

					if (ignorecase)
					{
						tmpclass->SetBit(TOLOWER(lastchar));
						tmpclass->SetBit(TOUPPER(lastchar));
					}
					else
					{
						tmpclass->SetBit(lastchar);
					}

					classsize++;
				}

				if (negative && classsize>1)
				{
					tmpclass->negative=negative;
					//for(int j=0;j<32;j++)op->symbolclass[j]^=0xff;
				}

				if (classsize==1)
				{
					delete op->symbolclass;
					op->symbolclass=0;
					tmpclass=0;
					op->op=negative?opNotSymbol:opSymbol;

					if (ignorecase)
					{
						op->op+=2;
						op->symbol=TOLOWER(lastchar);
					}
					else
					{
						op->symbol=lastchar;
					}
				}

				if (tmpclass)tmpclass->negative=negative;
				continue;
			}
			case '+':
			case '*':
			case '?':
			case '{':
			{
				int min=0,max=0;

				switch (src[i])
				{
					case '+':min=1; max=-2; break;
					case '*':min=0; max=-2; break;
					case '?':
					{
						//if(src[i+1]=='?') return SetError(errInvalidQuantifiersCombination,i);
						min=0; max=1;
						break;
					}
					case '{':
					{
						i++;
						int save=i;
						min=GetNum(src,i);
						max=min;

						if (min<0)return SetError(errInvalidRange,save);

//            i++;
						if (src[i]==',')
						{
							if (src[i+1]=='}')
							{
								i++;
								max=-2;
							}
							else
							{
								i++;
								max=GetNum(src,i);

//                i++;
								if (max<min)return SetError(errInvalidRange,save);
							}
						}

						if (src[i]!='}')return SetError(errInvalidRange,save);
					}
				}

				pos--;
				op=code+pos-1;

				if (min==1 && max==1)continue;

				op->range.min=min;
				op->range.max=max;

				switch (op->op)
				{
					case opLineStart:
					case opLineEnd:
					case opDataStart:
					case opDataEnd:
					case opWordBound:
					case opNotWordBound:
					{
						return SetError(errInvalidQuantifiersCombination,i);
//            op->range.op=op->op;
//            op->op=opRange;
//            continue;
					}
					case opCharAny:
					case opCharAnyAll:
					{
						op->range.op=op->op;
						op->op=opAnyRange;
						break;
					}
					case opType:
					{
						op->op=opTypeRange;
						break;
					}
					case opNotType:
					{
						op->op=opNotTypeRange;
						break;
					}
					case opSymbolIgnoreCase:
					case opSymbol:
					{
						op->op=opSymbolRange;
						break;
					}
					case opNotSymbol:
					case opNotSymbolIgnoreCase:
					{
						op->op=opNotSymbolRange;
						break;
					}
					case opSymbolClass:
					{
						op->op=opClassRange;
						break;
					}
					case opBackRef:
					{
						op->op=opBackRefRange;
						break;
					}
					case opClosingBracket:
					{
						op=op->bracket.pairindex;

						if (op->op!=opOpenBracket)return SetError(errInvalidQuantifiersCombination,i);

						op->range.min=min;
						op->range.max=max;
						op->op=opBracketRange;
						break;
					}
					default:
					{
						dpf(("op->=%d\n",op->op));
						return SetError(errInvalidQuantifiersCombination,i);
					}
				}//switch(code.op)

				if (src[i+1]=='?')
				{
					op->op++;
					i++;
				}

				continue;
			}// case +*?{
			case ' ':
			case '\t':
			case '\n':
			case '\r':
			{
				if (options&OP_XTENDEDSYNTAX)
				{
					pos--;
					continue;
				}
			}
			default:
			{
				op->op=options&OP_IGNORECASE?opSymbolIgnoreCase:opSymbol;

				if (ignorecase)
				{
					op->symbol=TOLOWER(src[i]);
				}
				else
				{
					op->symbol=src[i];
				}
			}
		}//switch(src[i])
	}//for()

	op=code+pos;
	pos++;
	brdepth--;

	while (brdepth>=0 && brackets[brdepth]->op==opAlternative)
	{
		brackets[brdepth]->alternative.endindex=op;
		brdepth--;
	}

	op->op=opClosingBracket;
	op->bracket.pairindex=code;
	code->bracket.pairindex=op;
#ifdef RE_DEBUG
	op->srcpos=i;
#endif
	op=code+pos;
	//pos++;
	op->op=opRegExpEnd;
#ifdef RE_DEBUG
	op->srcpos=i+1;
#endif
	return 1;
}

inline void RegExp::PushState()
{
	stackcount++;

	if (stackcount==STACK_PAGE_SIZE)
	{
		if (lastpage->next)
		{
			lastpage=lastpage->next;
			stack=lastpage->stack;
		}
		else
		{
			lastpage->next=new StateStackPage;
			lastpage->next->prev=lastpage;
			lastpage=lastpage->next;
			lastpage->next=nullptr;
			lastpage->stack=new StateStackItem[STACK_PAGE_SIZE];
			stack=lastpage->stack;
		}

		stackcount=0;
	}

	st=&stack[stackcount];
}

inline int RegExp::PopState()
{
	stackcount--;
	if (stackcount<0)
	{
		if (!lastpage->prev)
			return 0;

		lastpage=lastpage->prev;
		stack=lastpage->stack;
		stackcount=STACK_PAGE_SIZE-1;
	}

	st=&stack[stackcount];
	return 1;
}

inline StateStackItem *RegExp::GetState()
{
	int tempcount=stackcount;
	StateStackPage* temppage=lastpage;
	StateStackItem* tempstack=lastpage->stack;
	tempcount--;

	if (tempcount<0)
	{
		if (!temppage->prev)
			return 0;

		temppage=temppage->prev;
		tempstack=temppage->stack;
		tempcount=STACK_PAGE_SIZE-1;
	}

	return &tempstack[tempcount];
}

inline StateStackItem *RegExp::FindStateByPos(const PREOpCode pos,int op)
{
	int tempcount=stackcount;
	StateStackPage* temppage=lastpage;
	StateStackItem* tempstack=lastpage->stack;

	do
	{
		tempcount--;

		if (tempcount<0)
		{
			if (!temppage->prev)
				return 0;

			temppage=temppage->prev;
			tempstack=temppage->stack;
			tempcount=STACK_PAGE_SIZE-1;
		}
	}
	while (tempstack[tempcount].pos!=pos || tempstack[tempcount].op!=op);

	return &tempstack[tempcount];
}

inline int RegExp::StrCmp(const wchar_t*& str,const wchar_t* _st,const wchar_t* ed)
{
	const wchar_t* save=str;

	if (ignorecase)
	{
		while (_st<ed)
		{
			if (TOLOWER(*str)!=TOLOWER(*_st)) {str=save; return 0;}

			str++;
			_st++;
		}
	}
	else
	{
		while (_st<ed)
		{
			if (*str!=*_st) {str=save; return 0;}

			str++;
			_st++;
		}
	}

	return 1;
}

int RegExp::InnerMatch(const wchar_t* str,const wchar_t* strend,RegExpMatch* match,intptr_t& matchcount)
{
	int i,j;
	int minimizing;
	PREOpCode op,tmp=nullptr;
	RegExpMatch* m;
	UniSet *cl;
	int inrangebracket=0;

	if (errorcode==errNotCompiled)return 0;

	if (matchcount<maxbackref)return SetError(errNotEnoughMatches,maxbackref);

	stackcount=0;
	lastpage=firstpage;
	stack=lastpage->stack;
	st=&stack[0];
	StateStackItem *ps;
	errorcode=errNone;

	if (bracketscount<matchcount)matchcount=bracketscount;

	memset(match,-1,sizeof(*match)*matchcount);

	for (op=code; op; op=op->next)
	{
		//dpf(("op:%s,\tpos:%d,\tstr:%d\n",ops[op->op],pos,str-start));
		dpf(("=================\n"));
		dpf(("S:%s\n%*s\n",start,str-start+3,"^"));
		dpf(("R:%s\n%*s\n",resrc,op->srcpos+3,"^"));

		if (str<=strend)
		{
			auto MinSkip = [&](bool cmp)
			{
				int jj;
				switch(op->next->op)
				{
				case opSymbol:
					jj=op->next->symbol;
					if(*str!=jj)
					{
						while(str<strend && cmp && st->max--)
						{
							str++;
							if(str[1]!=jj)
								break;
						}
					}
					break;

				case opNotSymbol:
					jj=op->next->symbol;
					if(*str==jj)
					{
						while(str<strend && cmp && st->max--)
						{
							str++;
							if(str[1]==jj)
								break;
						}
					}
					break;

				case opSymbolIgnoreCase:
					jj=op->next->symbol;
					if(TOLOWER(*str)!=jj)
					{
						while(str<strend && cmp && st->max--)
						{
							str++;
							if(TOLOWER(str[1])!=jj)
								break;
						}
					}
					break;

				case opNotSymbolIgnoreCase:
					jj=op->next->symbol;
					if(TOLOWER(*str)==jj)
					{
						while(str<strend && cmp && st->max--)
						{
							str++;
							if(TOLOWER(str[1])==jj)
								break;
						}
					}
					break;

				case opType:
					jj=op->next->type;
					if(!isType(*str,jj))
					{
						while(str<strend && cmp && st->max--)
						{
							str++;
							if(!isType(str[1],jj))
								break;
						}
					}
					break;

				case opNotType:
					jj=op->next->type;
					if(isType(*str,jj))
					{
						while(str<strend && cmp && st->max--)
						{
							str++;
							if(isType(str[1],jj))
								break;
						}
					}
					break;

				case opSymbolClass:
					cl=op->next->symbolclass;
					if(!cl->GetBit(*str))
					{
						while(str<strend && cmp && st->max--)
						{
							str++;
							if(!cl->GetBit(str[1]))
								break;
						}
					}
					break;
				}
			};

			switch (op->op)
			{
				case opLineStart:
				{
					if (str==start || str[-1]==0x0d || str[-1]==0x0a)continue;

					break;
				}
				case opLineEnd:
				{
					if (str==strend || str[0]==0x0d || str[0]==0x0a)continue;

					break;
				}
				case opDataStart:
				{
					if (str==start)continue;

					break;
				}
				case opDataEnd:
				{
					if (str==strend)continue;

					break;
				}
				case opWordBound:
				{
					if ((str==start && ISWORD(*str))||
					        (!(ISWORD(str[-1])) && ISWORD(*str)) ||
					        (!(ISWORD(*str)) && ISWORD(str[-1])) ||
					        (str==strend && ISWORD(str[-1])))continue;

					break;
				}
				case opNotWordBound:
				{
					if (!((str==start && ISWORD(*str))||
					        (!(ISWORD(str[-1])) && ISWORD(*str)) ||
					        (!(ISWORD(*str)) && ISWORD(str[-1])) ||
					        (str==strend && ISWORD(str[-1]))))continue;

					break;
				}
				case opType:
				{
					if (isType(*str,op->type))
					{
						str++;
						continue;
					}

					break;
				}
				case opNotType:
				{
					if (!isType(*str,op->type))
					{
						str++;
						continue;
					}

					break;
				}
				case opCharAny:
				{
					if (*str!=0x0d && *str!=0x0a)
					{
						str++;
						continue;
					}

					break;
				}
				case opCharAnyAll:
				{
					str++;
					continue;
				}
				case opSymbol:
				{
					if (*str==op->symbol)
					{
						str++;
						continue;
					}

					break;
				}
				case opNotSymbol:
				{
					if (*str!=op->symbol)
					{
						str++;
						continue;
					}

					break;
				}
				case opSymbolIgnoreCase:
				{
					if (TOLOWER(*str)==op->symbol)
					{
						str++;
						continue;
					}

					break;
				}
				case opNotSymbolIgnoreCase:
				{
					if (TOLOWER(*str)!=op->symbol)
					{
						str++;
						continue;
					}

					break;
				}
				case opSymbolClass:
				{
					if (op->symbolclass->GetBit(*str))
					{
						str++;
						continue;
					}

					break;
				}
				case opOpenBracket:
				{
					if (op->bracket.index>=0 && op->bracket.index<matchcount)
					{
						//if (inrangebracket) Mantis#1388
						{
							st->op=opOpenBracket;
							st->pos=op;
							st->min=match[op->bracket.index].start;
							st->max=match[op->bracket.index].end;
							PushState();
						}

						match[op->bracket.index].start=(int)(str-start);
					}

					if (op->bracket.nextalt)
					{
						st->op=opAlternative;
						st->pos=op->bracket.nextalt;
						st->savestr=str;
						PushState();
					}

					continue;
				}
				case opClosingBracket:
				{
					switch (op->bracket.pairindex->op)
					{
						case opOpenBracket:
						{
							if (op->bracket.index>=0 && op->bracket.index<matchcount)
							{
								match[op->bracket.index].end=(int)(str-start);

								if (brhandler)
								{
									if (
									    !brhandler
									    (
									        brhdata,
									        bhMatch,
									        op->bracket.index,
									        match[op->bracket.index].start,
									        match[op->bracket.index].end
									    )
									)
									{
										return -1;
									}
								}
							}

							continue;
						}
						case opBracketRange:
						{
							ps=FindStateByPos(op->bracket.pairindex,opBracketRange);
							*st=*ps;

							if (str==st->startstr)
							{
								if (op->range.bracket.index>=0 && op->range.bracket.index<matchcount)
								{
									match[op->range.bracket.index].end=(int)(str-start);

									if (brhandler)
									{
										if (
										    !brhandler
										    (
										        brhdata,
										        bhMatch,
										        op->range.bracket.index,
										        match[op->range.bracket.index].start,
										        match[op->range.bracket.index].end
										    )
										)
										{
											return -1;
										}
									}
								}

								inrangebracket--;
								continue;
							}

							if (st->min>0)st->min--;

							if (st->min)
							{
								st->max--;
								st->startstr=str;
								st->savestr=str;
								op=st->pos;
								PushState();

								if (op->range.bracket.index>=0 && op->range.bracket.index<matchcount)
								{
									match[op->range.bracket.index].start=(int)(str-start);
									st->op=opOpenBracket;
									st->pos=op;
									st->min=match[op->range.bracket.index].start;
									st->max=match[op->range.bracket.index].end;
									PushState();
								}

								if (op->range.bracket.nextalt)
								{
									st->op=opAlternative;
									st->pos=op->range.bracket.nextalt;
									st->savestr=str;
									PushState();
								}

								continue;
							}

							st->max--;

							if (!st->max)
							{
								if (op->range.bracket.index>=0 && op->range.bracket.index<matchcount)
								{
									match[op->range.bracket.index].end=(int)(str-start);

									if (brhandler)
									{
										if (
										    !brhandler
										    (
										        brhdata,
										        bhMatch,
										        op->range.bracket.index,
										        match[op->range.bracket.index].start,
										        match[op->range.bracket.index].end
										    )
										)
										{
											return -1;
										}
									}
								}

								inrangebracket--;
								continue;
							}

							if (op->range.bracket.index>=0 && op->range.bracket.index<matchcount)
							{
								match[op->range.bracket.index].end=(int)(str-start);

								if (brhandler)
								{
									if (
									    !brhandler
									    (
									        brhdata,
									        bhMatch,
									        op->range.bracket.index,
									        match[op->range.bracket.index].start,
									        match[op->range.bracket.index].end
									    )
									)
									{
										return -1;
									}
								}

								tmp=op;
							}

							st->startstr=str;
							st->savestr=str;
							op=st->pos;
							PushState();

							if (op->range.bracket.index>=0 && op->range.bracket.index<matchcount)
							{
								st->op=opOpenBracket;
								st->pos=tmp;
								st->min=match[op->range.bracket.index].start;
								st->max=match[op->range.bracket.index].end;
								PushState();
								match[op->range.bracket.index].start=(int)(str-start);
							}

							if (op->range.bracket.nextalt)
							{
								st->op=opAlternative;
								st->pos=op->range.bracket.nextalt;
								st->savestr=str;
								PushState();
							}

							continue;
						}
						case opBracketMinRange:
						{
							ps=FindStateByPos(op->bracket.pairindex,opBracketMinRange);
							*st=*ps;

							if (st->min>0)st->min--;

							if (st->min)
							{
								//st->min--;
								st->max--;
								st->startstr=str;
								st->savestr=str;
								op=st->pos;
								PushState();

								if (op->range.bracket.index>=0 && op->range.bracket.index<matchcount)
								{
									if (brhandler)
									{
										if (
										    !brhandler
										    (
										        brhdata,
										        bhMatch,
										        op->range.bracket.index,
										        match[op->range.bracket.index].start,
										        (int)(str-start)
										    )
										)
										{
											return -1;
										}
									}

									match[op->range.bracket.index].start=(int)(str-start);
									st->op=opOpenBracket;
									st->pos=op;
									st->min=match[op->range.bracket.index].start;
									st->max=match[op->range.bracket.index].end;
									PushState();
								}

								if (op->range.bracket.nextalt)
								{
									st->op=opAlternative;
									st->pos=op->range.bracket.nextalt;
									st->savestr=str;
									PushState();
								}

								continue;
							}

							if (op->range.bracket.index>=0 && op->range.bracket.index<matchcount)
							{
								match[op->range.bracket.index].end=(int)(str-start);

								if (brhandler)
								{
									if (
									    !brhandler
									    (
									        brhdata,
									        bhMatch,
									        op->range.bracket.index,
									        match[op->range.bracket.index].start,
									        match[op->range.bracket.index].end
									    )
									)
									{
										return -1;
									}
								}
							}

							st->max--;
							inrangebracket--;

							if (!st->max)continue;

							st->forward=str>ps->startstr?1:0;
							st->startstr=str;
							st->savestr=str;
							PushState();

							if (op->range.bracket.nextalt)
							{
								st->op=opAlternative;
								st->pos=op->range.bracket.nextalt;
								st->savestr=str;
								PushState();
							}

							continue;
						}
						case opLookAhead:
						{
							tmp=op->bracket.pairindex;

							do
							{
								PopState();
							}
							while (st->pos!=tmp || st->op!=opLookAhead);

							str=st->savestr;
							continue;
						}
						case opNotLookAhead:
						{
							do
							{
								PopState();
							}
							while (st->op!=opNotLookAhead);

							str=st->savestr;
							break;
						}
						case opLookBehind:
						{
							continue;
						}
						case opNotLookBehind:
						{
							ps=GetState();
							ps->forward=0;
							break;
						}
					}//switch(code[pairindex].op)

					break;
				}//case opClosingBracket
				case opAlternative:
				{
					op=op->alternative.endindex->prev;
					continue;
				}
				case opBackRef:
				{
					m=&match[op->refindex];

					if (m->start==-1 || m->end==-1)break;

					if (ignorecase)
					{
						j=m->end;

						for (i=m->start; i<j; i++,str++)
						{
							if (TOLOWER(start[i])!=TOLOWER(*str))break;

							if (str>strend)break;
						}

						if (i<j)break;
					}
					else
					{
						j=m->end;

						for (i=m->start; i<j; i++,str++)
						{
							if (start[i]!=*str)break;

							if (str>strend)break;
						}

						if (i<j)break;
					}

					continue;
				}
				case opAnyRange:
				case opAnyMinRange:
				{
					st->op=op->op;
					minimizing=op->op==opAnyMinRange;
					j=op->range.min;
					st->max=op->range.max-j;

					if (op->range.op==opCharAny)
					{
						for (i=0; i<j; i++,str++)
						{
							if (str>strend || *str==0x0d || *str==0x0a)break;
						}

						if (i<j)
						{
							break;
						}

						st->startstr=str;

						if (!minimizing)
						{
							while (str<strend && *str!=0x0d && *str!=0x0a && st->max--)str++;
						}
						else
						{
							MinSkip(*str!=0x0d && *str!=0x0a);

							if (st->max==-1)break;
						}
					}
					else
					{
						//opCharAnyAll:
						str+=j;

						if (str>strend)break;

						st->startstr=str;

						if (!minimizing)
						{
							if (st->max>=0)
							{
								if (str+st->max<strend)
								{
									str+=st->max;
									st->max=0;
								}
								else
								{
									st->max-=(int)(strend-str);
									str=strend;
								}
							}
							else
							{
								str=strend;
							}
						}
						else
						{
							MinSkip(true);

							if (st->max==-1)break;
						}
					}

					if (op->range.max==j)continue;

					st->savestr=str;
					st->pos=op;
					PushState();
					continue;
				}
				case opSymbolRange:
				case opSymbolMinRange:
				{
					st->op=op->op;
					minimizing=op->op==opSymbolMinRange;
					j=op->range.min;
					st->max=op->range.max-j;

					if (ignorecase)
					{
						for (i=0; i<j; i++,str++)
						{
							if (str>strend || TOLOWER(*str)!=op->range.symbol)break;
						}

						if (i<j)break;

						st->startstr=str;

						if (!minimizing)
						{
							while (str<strend && TOLOWER(*str)==op->range.symbol && st->max--)str++;
						}
						else
						{
							MinSkip(TOLOWER(*str)==op->range.symbol);

							if (st->max==-1)break;
						}
					}
					else
					{
						for (i=0; i<j; i++,str++)
						{
							if (str>strend || *str!=op->range.symbol)break;
						}

						if (i<j)break;

						st->startstr=str;

						if (!minimizing)
						{
							while (str<strend && *str==op->range.symbol && st->max--)str++;
						}
						else
						{
							MinSkip(*str==op->range.symbol);

							if (st->max==-1)break;
						}
					}

					if (op->range.max==j)continue;

					st->savestr=str;
					st->pos=op;
					PushState();
					continue;
				}
				case opNotSymbolRange:
				case opNotSymbolMinRange:
				{
					st->op=op->op;
					minimizing=op->op==opNotSymbolMinRange;
					j=op->range.min;
					st->max=op->range.max-j;

					if (ignorecase)
					{
						for (i=0; i<j; i++,str++)
						{
							if (str>strend || TOLOWER(*str)==op->range.symbol)break;
						}

						if (i<j)break;

						st->startstr=str;

						if (!minimizing)
						{
							while (str<strend && TOLOWER(*str)!=op->range.symbol && st->max--)str++;
						}
						else
						{
							MinSkip(TOLOWER(*str)!=op->range.symbol);

							if (st->max==-1)break;
						}
					}
					else
					{
						for (i=0; i<j; i++,str++)
						{
							if (str>strend || *str==op->range.symbol)break;
						}

						if (i<j)break;

						st->startstr=str;

						if (!minimizing)
						{
							while (str<strend && *str!=op->range.symbol && st->max--)str++;
						}
						else
						{
							MinSkip(*str!=op->range.symbol);

							if (st->max==-1)break;
						}
					}

					if (op->range.max==j)continue;

					st->savestr=str;
					st->pos=op;
					PushState();
					continue;
				}
				case opClassRange:
				case opClassMinRange:
				{
					st->op=op->op;
					minimizing=op->op==opClassMinRange;
					j=op->range.min;
					st->max=op->range.max-j;

					for (i=0; i<j; i++,str++)
					{
						if (str>strend || !op->range.symbolclass->GetBit(*str))break;
					}

					if (i<j)break;

					st->startstr=str;

					if (!minimizing)
					{
						while (str<strend && op->range.symbolclass->GetBit(*str) && st->max--)str++;
					}
					else
					{
						MinSkip(op->range.symbolclass->GetBit(*str));

						if (st->max==-1)break;
					}

					if (op->range.max==j)continue;

					st->savestr=str;
					st->pos=op;
					PushState();
					continue;
				}
				case opTypeRange:
				case opTypeMinRange:
				{
					st->op=op->op;
					minimizing=op->op==opTypeMinRange;
					j=op->range.min;
					st->max=op->range.max-j;

					for (i=0; i<j; i++,str++)
					{
						if (str>strend || !isType(*str,op->range.type))break;
					}

					if (i<j)break;

					st->startstr=str;

					if (!minimizing)
					{
						while (str<strend && isType(*str,op->range.type) && st->max--)str++;
					}
					else
					{
						MinSkip(isType(*str,op->range.type));

						if (st->max==-1)break;
					}

					if (op->range.max==j)continue;

					st->savestr=str;
					st->pos=op;
					PushState();
					continue;
				}
				case opNotTypeRange:
				case opNotTypeMinRange:
				{
					st->op=op->op;
					minimizing=op->op==opNotTypeMinRange;
					j=op->range.min;
					st->max=op->range.max-j;

					for (i=0; i<j; i++,str++)
					{
						if (str>strend || isType(*str,op->range.type))break;
					}

					if (i<j)break;

					st->startstr=str;

					if (!minimizing)
					{
						while (str<strend && !isType(*str,op->range.type) && st->max--)str++;
					}
					else
					{
						MinSkip(!isType(*str,op->range.type));

						if (st->max==-1)break;
					}

					if (op->range.max==j)continue;

					st->savestr=str;
					st->pos=op;
					PushState();
					continue;
				}
				case opBackRefRange:
				case opBackRefMinRange:
				{
					st->op=op->op;
					minimizing=op->op==opBackRefMinRange;
					j=op->range.min;
					st->max=op->range.max-j;
					m=&match[op->range.refindex];

					if (!m)break;

					if (m->start==-1 || m->end==-1)
					{
						if (!j)continue;
						else break;
					}

					for (i=0; i<j; i++)
					{
						if (str>strend || !StrCmp(str,start+m->start,start+m->end))break;
					}

					if (i<j)break;

					st->startstr=str;

					if (!minimizing)
					{
						while (str<strend && StrCmp(str,start+m->start,start+m->end) && st->max--);
					}
					else
					{
						MinSkip(StrCmp(str,start+m->start,start+m->end) != 0);

						if (st->max==-1)break;
					}

					if (op->range.max==j)continue;

					st->savestr=str;
					st->pos=op;
					PushState();
					continue;
				}
				case opBracketRange:
				case opBracketMinRange:
				{
					if (inrangebracket && op->range.bracket.index>=0 && op->range.bracket.index<matchcount)
					{
						st->op=opOpenBracket;
						st->pos=op->range.bracket.pairindex;
						st->min=match[op->range.bracket.index].start;
						st->max=match[op->range.bracket.index].end;
						PushState();
					}

					st->op=op->op;
					st->pos=op;
					st->min=op->range.min;
					st->max=op->range.max;
					st->startstr=str;
					st->savestr=str;
					st->forward=1;
					PushState();

					if (op->range.nextalt)
					{
						st->op=opAlternative;
						st->pos=op->range.bracket.nextalt;
						st->savestr=str;
						PushState();
					}

					if (op->range.bracket.index>=0 && op->range.bracket.index<matchcount)
					{
						match[op->range.bracket.index].start=
						    /*match[op->range.bracket.index].end=*/(int)(str-start);
					}

					if (op->op==opBracketMinRange && !op->range.min)
					{
						op=op->range.bracket.pairindex;
					}
					else
					{
						inrangebracket++;
					}

					continue;
				}
				case opLookAhead:
				case opNotLookAhead:
				{
					st->op=op->op;
					st->savestr=str;
					st->pos=op;
					st->forward=1;
					PushState();

					if (op->assert.nextalt)
					{
						st->op=opAlternative;
						st->pos=op->assert.nextalt;
						st->savestr=str;
						PushState();
					}

					continue;
				}
				case opLookBehind:
				case opNotLookBehind:
				{
					if (str-op->assert.length<start)
					{
						if (op->op==opLookBehind)break;

						op=op->assert.pairindex;
						continue;
					}

					st->op=op->op;
					st->savestr=str;
					st->pos=op;
					st->forward=1;
					str-=op->assert.length;
					PushState();

					if (op->assert.nextalt)
					{
						st->op=opAlternative;
						st->pos=op->assert.nextalt;
						st->savestr=str;
						PushState();
					}

					continue;
				}
				case opNoReturn:
				{
					st->op=opNoReturn;
					PushState();
					continue;
				}
				case opRegExpEnd:return 1;
			}//switch(op)
		}

		for (;; PopState())
		{
			if (0==(ps=GetState()))return 0;

			//dpf(("ps->op:%s\n",ops[ps->op]));
			switch (ps->op)
			{
				case opAlternative:
				{
					str=ps->savestr;
					op=ps->pos;

					if (op->alternative.nextalt)
					{
						ps->pos=op->alternative.nextalt;
					}
					else
					{
						PopState();
					}

					break;
				}
				case opAnyRange:
				case opSymbolRange:
				case opNotSymbolRange:
				case opClassRange:
				case opTypeRange:
				case opNotTypeRange:
				{
					str=ps->savestr-1;
					op=ps->pos;

					if (str<ps->startstr)
					{
						continue;
					}

					ps->savestr=str;
					break;
				}
				case opBackRefRange:
				{
					m=&match[ps->pos->range.refindex];
					str=ps->savestr-(m->end-m->start);
					op=ps->pos;

					if (str<ps->startstr)
					{
						continue;
					}

					ps->savestr=str;
					break;
				}
				case opAnyMinRange:
				{
					if (!(ps->max--))continue;

					str=ps->savestr;
					op=ps->pos;

					if (ps->pos->range.op==opCharAny)
					{
						if (str<strend && *str!=0x0a && *str!=0x0d)
						{
							str++;
							ps->savestr=str;
						}
						else
						{
							continue;
						}
					}
					else
					{
						if (str<strend)
						{
							str++;
							ps->savestr=str;
						}
						else
						{
							continue;
						}
					}

					break;
				}
				case opSymbolMinRange:
				{
					if (!(ps->max--))continue;

					str=ps->savestr;
					op=ps->pos;

					if (ignorecase)
					{
						if (str<strend && TOLOWER(*str)==op->symbol)
						{
							str++;
							ps->savestr=str;
						}
						else
						{
							continue;
						}
					}
					else
					{
						if (str<strend && *str==op->symbol)
						{
							str++;
							ps->savestr=str;
						}
						else
						{
							continue;
						}
					}

					break;
				}
				case opNotSymbolMinRange:
				{
					if (!(ps->max--))continue;

					str=ps->savestr;
					op=ps->pos;

					if (ignorecase)
					{
						if (str<strend && TOLOWER(*str)!=op->symbol)
						{
							str++;
							ps->savestr=str;
						}
						else
						{
							continue;
						}
					}
					else
					{
						if (str<strend && *str!=op->symbol)
						{
							str++;
							ps->savestr=str;
						}
						else
						{
							continue;
						}
					}

					break;
				}
				case opClassMinRange:
				{
					if (!(ps->max--))continue;

					str=ps->savestr;
					op=ps->pos;

					if (str<strend && op->range.symbolclass->GetBit(*str))
					{
						str++;
						ps->savestr=str;
					}
					else
					{
						continue;
					}

					break;
				}
				case opTypeMinRange:
				{
					if (!(ps->max--))continue;

					str=ps->savestr;
					op=ps->pos;

					if (str<strend && isType(*str,op->range.type))
					{
						str++;
						ps->savestr=str;
					}
					else
					{
						continue;
					}

					break;
				}
				case opNotTypeMinRange:
				{
					if (!(ps->max--))continue;

					str=ps->savestr;
					op=ps->pos;

					if (str<strend && !isType(*str,op->range.type))
					{
						str++;
						ps->savestr=str;
					}
					else
					{
						continue;
					}

					break;
				}
				case opBackRefMinRange:
				{
					if (!(ps->max--))continue;

					str=ps->savestr;
					op=ps->pos;
					m=&match[op->range.refindex];

					if (str+m->end-m->start<strend && StrCmp(str,start+m->start,start+m->end))
					{
						ps->savestr=str;
					}
					else
					{
						continue;
					}

					break;
				}
				case opBracketRange:
				{
					if (ps->pos->range.bracket.index>=0 && brhandler)
					{
						if (
						    !brhandler
						    (
						        brhdata,
						        bhRollBack,
						        ps->pos->range.bracket.index,
						        -1,
						        -1
						    )
						)
						{
							return -1;
						}
					}

					if (ps->min)
					{
						inrangebracket--;
						continue;
					}

					if (ps->forward)
					{
						ps->forward=0;
						op=ps->pos->range.bracket.pairindex;
						inrangebracket--;
						str=ps->savestr;

						if (op->range.nextalt)
						{
							st->op=opAlternative;
							st->pos=op->range.bracket.nextalt;
							st->savestr=str;
							PushState();
						}

//            if(op->bracket.index>=0 && op->bracket.index<matchcount)
//            {
//              match[op->bracket.index].end=str-start;
//            }
						break;
					}

					continue;
				}
				case opBracketMinRange:
				{
					if (ps->pos->range.bracket.index>=0 && brhandler)
					{
						if (
						    !brhandler
						    (
						        brhdata,
						        bhRollBack,
						        ps->pos->range.bracket.index,
						        -1,
						        -1
						    )
						)
						{
							return -1;
						}
					}

					if (!(ps->max--))
					{
						inrangebracket--;
						continue;
					}

					if (ps->forward)
					{
						ps->forward=0;
						op=ps->pos;
						str=ps->savestr;

						if (op->range.bracket.index>=0 && op->range.bracket.index<matchcount)
						{
							match[op->range.bracket.index].start=(int)(str-start);
							st->op=opOpenBracket;
							st->pos=op;
							st->min=match[op->range.bracket.index].start;
							st->max=match[op->range.bracket.index].end;
							PushState();
						}

						if (op->range.nextalt)
						{
							st->op=opAlternative;
							st->pos=op->range.bracket.nextalt;
							st->savestr=str;
							PushState();
						}

						inrangebracket++;
						break;
					}

					inrangebracket--;
					continue;
				}
				case opOpenBracket:
				{
					j=ps->pos->bracket.index;

					if (j>=0 && j<matchcount)
					{
						if (brhandler)
						{
							if (
							    !brhandler
							    (
							        brhdata,
							        bhRollBack,
							        j,
							        match[j].start,
							        match[j].end
							    )
							)
							{
								return -1;
							}
						}

						match[j].start=ps->min;
						match[j].end=ps->max;
					}

					continue;
				}
				case opLookAhead:
				case opLookBehind:
				{
					continue;
				}
				case opNotLookBehind:
				case opNotLookAhead:
				{
					op=ps->pos->assert.pairindex;
					str=ps->savestr;

					if (ps->forward)
					{
						PopState();
						break;
					}
					else
					{
						continue;
					}
				}
				case opNoReturn:
				{
					return 0;
				}
			}//switch(op)

			break;
		}
	}

	return 1;
}

int RegExp::Match(const wchar_t* textstart,const wchar_t* textend,RegExpMatch* match,intptr_t& matchcount)
{
	start=textstart;
	const wchar_t* tempend=textend;

	if (havefirst && !first[*start])return 0;

	TrimTail(tempend);

	if (tempend<start)return 0;

	if (minlength && tempend-start<minlength)return 0;

	int res=InnerMatch(start,tempend,match,matchcount);

	if (res==1)
	{
		for (int i=0; i<matchcount; i++)
		{
			if (match[i].start==-1 || match[i].end==-1 || match[i].start>match[i].end)
			{
				match[i].start=match[i].end=-1;
			}
		}
	}

	return res;
}

int RegExp::MatchEx(const wchar_t* datastart,const wchar_t* textstart,const wchar_t* textend,RegExpMatch* match,intptr_t& matchcount)
{
	if (havefirst && !first[(wchar_t)*textstart])return 0;

	const wchar_t* tempend=textend;

	if (datastart==start && textend==end)
	{
		tempend=trimend;
	}
	else
	{
		start=datastart;
		TrimTail(tempend);
		trimend=tempend;
	}

	end=textend;

	if (tempend<textstart)return 0;

	if (minlength && tempend-start<minlength)return 0;

	int res=InnerMatch(textstart,tempend,match,matchcount);

	if (res==1)
	{
		for (int i=0; i<matchcount; i++)
		{
			if (match[i].start==-1 || match[i].end==-1 || match[i].start>match[i].end)
			{
				match[i].start=match[i].end=-1;
			}
		}
	}

	return res;
}

int RegExp::Match(const wchar_t* textstart,RegExpMatch* match,intptr_t& matchcount)
{
	const wchar_t* textend=textstart+wcslen(textstart);
	return Match(textstart,textend,match,matchcount);
}

int RegExp::Optimize()
{
	PREOpCode jumps[MAXDEPTH];
	int jumpcount=0;

	if (havefirst)return 1;

	first.Reset();
	PREOpCode op;
	minlength=0;
	int mlstackmin[MAXDEPTH];
	int mlstacksave[MAXDEPTH];
	int mlscnt=0;

	for (op=code; op; op=op->next)
	{
		switch (op->op)
		{
			case opType:
			case opNotType:
			case opCharAny:
			case opCharAnyAll:
			case opSymbol:
			case opNotSymbol:
			case opSymbolIgnoreCase:
			case opNotSymbolIgnoreCase:
			case opSymbolClass:
				minlength++;
				continue;
			case opSymbolRange:
			case opSymbolMinRange:
			case opNotSymbolRange:
			case opNotSymbolMinRange:
			case opAnyRange:
			case opAnyMinRange:
			case opTypeRange:
			case opTypeMinRange:
			case opNotTypeRange:
			case opNotTypeMinRange:
			case opClassRange:
			case opClassMinRange:
				minlength+=op->range.min;
				break;
			case opOpenBracket:
			case opBracketRange:
			case opBracketMinRange:
				mlstacksave[mlscnt]=minlength;
				mlstackmin[mlscnt++]=-1;
				minlength=0;
				continue;
			case opClosingBracket:
			{
				if (op->bracket.pairindex->op>opAssertionsBegin &&
				        op->bracket.pairindex->op<opAsserionsEnd)
				{
					continue;
				}

				if (mlstackmin[mlscnt-1]!=-1 && mlstackmin[mlscnt-1]<minlength)
				{
					minlength=mlstackmin[mlscnt-1];
				}

				switch (op->bracket.pairindex->op)
				{
					case opBracketRange:
					case opBracketMinRange:
						minlength*=op->range.min;
						break;
				}

				minlength+=mlstacksave[--mlscnt];
			} continue;
			case opAlternative:
			{
				if (mlstackmin[mlscnt-1]==-1)
				{
					mlstackmin[mlscnt-1]=minlength;
				}
				else
				{
					if (minlength<mlstackmin[mlscnt-1])
					{
						mlstackmin[mlscnt-1]=minlength;
					}
				}

				minlength=0;
				break;
			}
			case opLookAhead:
			case opNotLookAhead:
			case opLookBehind:
			case opNotLookBehind:
			{
				op=op->assert.pairindex;
				continue;
			}
			case opRegExpEnd:
				op=0;
				break;
		}

		if (!op)break;
	}

	dpf(("minlength=%d\n",minlength));

	for (op=code;; op=op->next)
	{
		switch (op->op)
		{
			default:
			{
				return 0;
			}
			case opType:
			{
				for (int i=0; i<RE_CHAR_COUNT; i++)if (isType(i,op->type))first[i]=1;

				break;
			}
			case opNotType:
			{
				for (int i=0; i<RE_CHAR_COUNT; i++)if (!isType(i,op->type))first[i]=1;

				break;
			}
			case opSymbol:
			{
				first[op->symbol]=1;
				break;
			}
			case opSymbolIgnoreCase:
			{
				first[op->symbol]=1;
				first[TOUPPER(op->symbol)]=1;
				break;
			}
			case opSymbolClass:
			{
				for (int i=0; i<RE_CHAR_COUNT; i++)
				{
					if (op->symbolclass->GetBit(i))first[i]=1;
				}

				break;
			}
			case opOpenBracket:
			{
				if (op->bracket.nextalt)
				{
					jumps[jumpcount++]=op->bracket.nextalt;
				}

				continue;
			}
			case opClosingBracket:
			{
				continue;
			}
			case opAlternative:
			{
				return 0;
			}
			case opSymbolRange:
			case opSymbolMinRange:
			{
				if (ignorecase)
				{
					first[TOLOWER(op->range.symbol)]=1;
					first[TOUPPER(op->range.symbol)]=1;
				}
				else
				{
					first[op->range.symbol]=1;
				}

				if (!op->range.min)continue;

				break;
			}
			case opTypeRange:
			case opTypeMinRange:
			{
				for (int i=0; i<RE_CHAR_COUNT; i++)
				{
					if (isType(i,op->range.type))first[i]=1;
				}

				if (!op->range.min)continue;

				break;
			}
			case opNotTypeRange:
			case opNotTypeMinRange:
			{
				for (int i=0; i<RE_CHAR_COUNT; i++)
				{
					if (!isType(i,op->range.type))first[i]=1;
				}

				if (!op->range.min)continue;

				break;
			}
			case opClassRange:
			case opClassMinRange:
			{
				for (int i=0; i<RE_CHAR_COUNT; i++)
				{
					if (op->range.symbolclass->GetBit(i))first[i]=1;
				}

				if (!op->range.min)continue;

				break;
			}
			case opBracketRange:
			case opBracketMinRange:
			{
				if (!op->range.min)return 0;

				if (op->range.bracket.nextalt)
				{
					jumps[jumpcount++]=op->range.bracket.nextalt;
				}

				continue;
			}
			//case opLookAhead:
			//case opNotLookAhead:
			//case opLookBehind:
			//case opNotLookBehind:
			case opRegExpEnd:return 0;
		}

		if (jumpcount>0)
		{
			op=jumps[--jumpcount];

			if (op->op==opAlternative && op->alternative.nextalt)
			{
				jumps[jumpcount++]=op->alternative.nextalt;
			}

			continue;
		}

		break;
	}

	havefirst=1;
	return 1;
}

int RegExp::Search(const wchar_t* textstart,RegExpMatch* match,intptr_t& matchcount)
{
	const wchar_t* textend=textstart+wcslen(textstart);
	return Search(textstart,textend,match,matchcount);
}

int RegExp::Search(const wchar_t* textstart,const wchar_t* textend,RegExpMatch* match,intptr_t& matchcount)
{
	start=textstart;
	const wchar_t* str=start;
	const wchar_t* tempend=textend;
	TrimTail(tempend);

	if (tempend<start)return 0;

	if (minlength && tempend-start<minlength)return 0;

	int res=0;

	if (!code->bracket.nextalt && code->next->op==opDataStart)
	{
		res=InnerMatch(start,tempend,match,matchcount);
	}
	else
	{
		if (!code->bracket.nextalt && code->next->op==opDataEnd && code->next->next->op==opClosingBracket)
		{
			matchcount=1;
			match[0].start=(int)(textend-textstart);
			match[0].end=match[0].start;
			return 1;
		}

		if (havefirst)
		{
			do
			{
				while (!first[*str] && str<tempend)str++;

				if (0!=(res=InnerMatch(str,tempend,match,matchcount)))
				{
					break;
				}

				str++;
			}
			while (str<tempend);

			if (!res && InnerMatch(str,tempend,match,matchcount))
			{
				res=1;
			}
		}
		else
		{
			do
			{
				if (0!=(res=InnerMatch(str,tempend,match,matchcount)))
				{
					break;
				}

				str++;
			}
			while (str<=tempend);
		}
	}

	if (res==1)
	{
		for (int i=0; i<matchcount; i++)
		{
			if (match[i].start==-1 || match[i].end==-1 || match[i].start>match[i].end)
			{
				match[i].start=match[i].end=-1;
			}
		}
	}

	return res;
}

int RegExp::SearchEx(const wchar_t* datastart,const wchar_t* textstart,const wchar_t* textend,RegExpMatch* match,intptr_t& matchcount)
{
	start=datastart;
	const wchar_t* str=textstart;
	const wchar_t* tempend=textend;
	TrimTail(tempend);

	if (tempend<start)return 0;

	if (minlength && tempend-start<minlength)return 0;

	int res=0;

	if (!code->bracket.nextalt && code->next->op==opDataStart)
	{
		res=InnerMatch(str,tempend,match,matchcount);
	}
	else
	{
		if (!code->bracket.nextalt && code->next->op==opDataEnd && code->next->next->op==opClosingBracket)
		{
			matchcount=1;
			match[0].start=(int)(textend-datastart);
			match[0].end=match[0].start;
			return 1;
		}

		if (havefirst)
		{
			do
			{
				while (!first[*str] && str<tempend)str++;

				if (0!=(res=InnerMatch(str,tempend,match,matchcount)))
				{
					break;
				}

				str++;
			}
			while (str<tempend);

			if (!res && InnerMatch(str,tempend,match,matchcount))
			{
				res=1;
			}
		}
		else
		{
			do
			{
				if (0!=(res=InnerMatch(str,tempend,match,matchcount)))
				{
					break;
				}

				str++;
			}
			while (str<=tempend);
		}
	}

	if (res==1)
	{
		for (int i=0; i<matchcount; i++)
		{
			if (match[i].start==-1 || match[i].end==-1 || match[i].start>match[i].end)
			{
				match[i].start=match[i].end=-1;
			}
		}
	}

	return res;
}

void RegExp::TrimTail(const wchar_t*& strend)
{
	if (havelookahead)return;

	if (!code || code->bracket.nextalt)return;

	PREOpCode op=code->bracket.pairindex->prev;

	while (op->op==opClosingBracket)
	{
		if (op->bracket.pairindex->op!=opOpenBracket)return;

		if (op->bracket.pairindex->bracket.nextalt)return;

		op=op->prev;
	}

	strend--;

	switch (op->op)
	{
		case opSymbol:
		{
			while (strend>=start && *strend!=op->symbol)strend--;

			break;
		}
		case opNotSymbol:
		{
			while (strend>=start && *strend==op->symbol)strend--;

			break;
		}
		case opSymbolIgnoreCase:
		{
			while (strend>=start && TOLOWER(*strend)!=op->symbol)strend--;

			break;
		}
		case opNotSymbolIgnoreCase:
		{
			while (strend>=start && TOLOWER(*strend)==op->symbol)strend--;

			break;
		}
		case opType:
		{
			while (strend>=start && !isType(*strend,op->type))strend--;

			break;
		}
		case opNotType:
		{
			while (strend>=start && isType(*strend,op->type))strend--;

			break;
		}
		case opSymbolClass:
		{
			while (strend>=start && !op->symbolclass->GetBit(*strend))strend--;

			break;
		}
		case opSymbolRange:
		case opSymbolMinRange:
		{
			if (!op->range.min)break;

			if (ignorecase)
			{
				while (strend>=start && TOLOWER(*strend)!=op->range.symbol)strend--;
			}
			else
			{
				while (strend>=start && *strend!=op->range.symbol)strend--;
			}

			break;
		}
		case opNotSymbolRange:
		case opNotSymbolMinRange:
		{
			if (!op->range.min)break;

			if (ignorecase)
			{
				while (strend>=start && TOLOWER(*strend)==op->range.symbol)strend--;
			}
			else
			{
				while (strend>=start && *strend==op->range.symbol)strend--;
			}

			break;
		}
		case opTypeRange:
		case opTypeMinRange:
		{
			if (!op->range.min)break;

			while (strend>=start && !isType(*strend,op->range.type))strend--;

			break;
		}
		case opNotTypeRange:
		case opNotTypeMinRange:
		{
			if (!op->range.min)break;

			while (strend>=start && isType(*strend,op->range.type))strend--;

			break;
		}
		case opClassRange:
		case opClassMinRange:
		{
			if (!op->range.min)break;

			while (strend>=start && !op->range.symbolclass->GetBit(*strend))strend--;

			break;
		}
		default:break;
	}

	strend++;
}

void RegExp::CleanStack()
{
	PStateStackPage tmp=firstpage->next,tmp2;

	while (tmp)
	{
		tmp2=tmp->next;
		delete [] tmp->stack;
		delete tmp;
		tmp=tmp2;
	}
}
