#pragma once
/*
RegExp.hpp

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

enum
{
	RE_CHAR_COUNT = 1<<sizeof(wchar_t)*8,
};

//! Possible compile and runtime errors returned by LastError.
enum REError
{
	//! No errors
	errNone=0,
	//! RegExp wasn't even tried to compile
	errNotCompiled,
	//! expression contain syntax error
	errSyntax,
	//! Unbalanced brackets
	errBrackets,
	//! Max recursive brackets level reached. Controlled in compile time
	errMaxDepth,
	//! Invalid options combination
	errOptions,
	//! Reference to nonexistent bracket
	errInvalidBackRef,
	//! Invalid escape char
	errInvalidEscape,
	//! Invalid range value
	errInvalidRange,
	//! Quantifier applied to invalid object. f.e. lookahead assertion
	errInvalidQuantifiersCombination,
	//! Size of match array isn't large enough.
	errNotEnoughMatches,
	//! Attempt to match RegExp with Named Brackets, and no storage class provided.
	errNoStorageForNB,
	//! Reference to undefined named bracket
	errReferenceToUndefinedNamedBracket,
	//! Only fixed length look behind assertions are supported
	errVariableLengthLookBehind
};

//! Used internally
struct REOpCode;
//! Used internally
typedef REOpCode *PREOpCode;

//! Max brackets depth
enum
{
	MAXDEPTH=256,
};

/**
  \defgroup options Regular expression compile time options
*/
enum
{
	//! Match in a case insensitive manner
	OP_IGNORECASE   =0x0001,
	//! Single line mode, dot meta-character will match newline symbol
	OP_SINGLELINE   =0x0002,
	//! MultiLine mode, ^ and $ can match line start and line end
	OP_MULTILINE    =0x0004,
	//! Extended syntax, spaces symbols are ignored unless escaped
	OP_XTENDEDSYNTAX=0x0008,
	//! Perl style RegExp provided. i.e. /expression/imsx
	OP_PERLSTYLE    =0x0010,
	//! Optimize after compile
	OP_OPTIMIZE     =0x0020,
	//! Strict escapes - only unrecognized escape will produce errInvalidEscape error
	OP_STRICT       =0x0040,
	//! Replace backslash with slash, used
	//! when RegExp source embedded in c++ sources
	OP_CPPMODE      =0x0080,
};



/**
 \defgroup localebits Locale Info bits

*/
enum
{
	//! Digits
	TYPE_DIGITCHAR  =0x01,
	//! space, newlines tab etc
	TYPE_SPACECHAR  =0x02,
	//! alphanumeric and _
	TYPE_WORDCHAR   =0x04,
	//! lo-case symbol
	TYPE_LOWCASE    =0x08,
	//! up-case symbol
	TYPE_UPCASE     =0x10,
	//! letter
	TYPE_ALPHACHAR  =0x20,
};

/**
  \defgroup brhactions Bracket handler actions

*/
enum
{
	//! Matched Closing bracket
	bhMatch=1,
	//! Bracket rollback
	bhRollBack=2,
};

//! default preallocated stack size, and stack page size
enum
{
	STACK_PAGE_SIZE=16,
};

typedef struct RegExpMatch SMatch,*PMatch;

//! Used internally
typedef struct StateStackItem
{
	int op;
	REOpCode* pos;
	const wchar_t* savestr;
	const wchar_t* startstr;
	int min;
	int cnt;
	int max;
	int forward;
}*PStateStackItem;

//! Used internally
typedef struct StateStackPage
{
	PStateStackItem stack;
	StateStackPage* prev;
	StateStackPage* next;
}*PStateStackPage;

struct UniSet;

/*! Regular expressions support class.

Expressions must be Compile'ed first,
and than Match string or Search for matching fragment.
*/
class RegExp
{
	private:
		// code
		PREOpCode code;
#ifdef RE_DEBUG
		wchar_t* resrc;
#endif

		StateStackItem initstack[STACK_PAGE_SIZE];
		StateStackPage initstackpage;

// current stack page and upper stack element
		PStateStackItem stack,st;
		int stackcount;
		char slashChar;
		char backslashChar;

		PStateStackPage firstpage;
		PStateStackPage lastpage;

		UniSet *firstptr;
		int havefirst;
		int havelookahead;

		int minlength;

		// error info
		int errorcode;
		int errorpos;

		// options
		int ignorecase;

		int bracketscount;
		int maxbackref;

		const wchar_t* start;
		const wchar_t* end;
		const wchar_t* trimend;

		int CalcLength(const wchar_t* src,int srclength);
		int InnerCompile(const wchar_t* src,int srclength,int options);

		int InnerMatch(const wchar_t* str,const wchar_t* end,PMatch match,intptr_t& matchcount);

		void TrimTail(const wchar_t*& end);

		int SetError(int _code,int pos) {errorcode=_code; errorpos=pos; return 0;}

		int GetNum(const wchar_t* src,int& i);

		static inline void SetBit(wchar_t* bitset,int charindex)
		{
			bitset[charindex>>3]|=1<<(charindex&7);
		}
		static inline int GetBit(wchar_t* bitset,int charindex)
		{
			return bitset[charindex>>3]&(1<<(charindex&7));
		}

		void PushState();
		StateStackItem* GetState();
		StateStackItem* FindStateByPos(PREOpCode pos,int op);
		int PopState();


		int StrCmp(const wchar_t*& str,const wchar_t* start,const wchar_t* end);

		void Init(const wchar_t*,int options);
		//RegExp(const RegExp& re) {};

	public:
		//! Default constructor.
		RegExp();
		/*! Create object with compiled expression

		   \param expr - source of expression
		   \param options - compilation options

		   By default expression in perl style expected,
		   and will be optimized after compilation.

		   Compilation status can be verified with LastError method.
		   \sa LastError
		*/
		RegExp(const wchar_t* expr,int options=OP_PERLSTYLE|OP_OPTIMIZE);
		virtual ~RegExp();

		/*! Compile regular expression
		    Generate internall op-codes of expression.

		    \param src - source of expression
		    \param options - compile options
		    \return 1 on success, 0 otherwise

		    If compilation fails error code can be obtained with LastError function,
		    position of error in a expression can be obtained with ErrorPosition function.
		    See error codes in REError enumeration.
		    \sa LastError
		    \sa REError
		    \sa ErrorPosition
		    \sa options
		*/
		int Compile(const wchar_t* src,int options=OP_PERLSTYLE|OP_OPTIMIZE);

		/*! Try to optimize regular expression
		    Significally speedup Search mode in some cases.
		    \return 1 on success, 0 if optimization failed.
		*/
		int Optimize();

		/*! Try to match string with regular expression
		    \param textstart - start of string to match
		    \param textend - point to symbol after last symbols of the string.
		    \param match - array of SMatch structures that receive brackets positions.
		    \param matchcount - in/out parameter that indicate number of items in
		    match array on input, and number of brackets on output.
		    \return 1 on success, 0 if match failed.
		    \sa SMatch
		*/
		int Match(const wchar_t* textstart,const wchar_t* textend,PMatch match,intptr_t& matchcount);
		/*! Same as Match(const char* textstart,const char* textend,...), but for ASCIIZ string.
		    textend calculated automatically.
		*/
		int Match(const wchar_t* textstart,PMatch match,intptr_t& matchcount);
		/*! Advanced version of match. Can be used for multiple matches
		    on one string (to imitate /g modifier of perl regexp
		*/
		int MatchEx(const wchar_t* datastart,const wchar_t* textstart,const wchar_t* textend,PMatch match,intptr_t& matchcount);
		/*! Try to find substring that will match regexp.
		    Parameters and return value are the same as for Match.
		    It is highly recommended to call Optimize before Search.
		*/
		int Search(const wchar_t* textstart,const wchar_t* textend,PMatch match,intptr_t& matchcount);
		/*! Same as Search with specified textend, but for ASCIIZ strings only.
		    textend calculated automatically.
		*/
		int Search(const wchar_t* textstart,PMatch match,intptr_t& matchcount);
		/*! Advanced version of search. Can be used for multiple searches
		    on one string (to imitate /g modifier of perl regexp
		*/
		int SearchEx(const wchar_t* datastart,const wchar_t* textstart,const wchar_t* textend,PMatch match,intptr_t& matchcount);

		/*! Clean regexp execution stack.
		    After match large string with complex regexp, significant
		    amount of memory can be allocated for execution stack.
		*/
		void CleanStack();

		/*! Get last error
		    \return code of the last error
		    Check REError for explanation
		    \sa REError
		    \sa ErrorPosition
		*/
		int LastError() const {return errorcode;}
		/*! Get last error position.
		    \return position of the last error in the regexp source.
		    \sa LastError
		*/
		int ErrorPosition() const {return errorpos;}
		/*! Get number of brackets in expression
		    \return number of brackets, excluding brackets of type (:expr)
		    and named brackets.
		*/
		int GetBracketsCount() const {return bracketscount;}
		typedef bool(*BracketHandler)(void* data,int action,int brindex,int start,int end);
		void SetBracketHandler(BracketHandler bh,void* data)
		{
			brhandler=bh;
			brhdata=data;
		}
	protected:
		BracketHandler brhandler;
		void* brhdata;
};
