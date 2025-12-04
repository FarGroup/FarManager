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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "RegExp.hpp"

// Internal:
#include "string_utils.hpp"
#include "plugin.hpp"

// Platform:

// Common:
#include "common/algorithm.hpp"
#include "common/function_ref.hpp"
#include "common/from_string.hpp"
#include "common/scope_exit.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

bool is_far_eol_before(const wchar_t* const Begin, const wchar_t* const Iterator, const wchar_t* const End)
{
	if (Iterator == Begin)
		return true;

	// After \n or \r\n or \r\r\n
	if (Iterator[-1] == L'\n')
		return true;

	// After \r, but not within \r\n or \r\r\n
	string_view const Str(Iterator, End);
	return Iterator[-1] == L'\r' && !Str.starts_with(L"\n"sv) && !Str.starts_with(L"\r\n");
}

bool is_far_eol(const wchar_t* const Begin, const wchar_t* const Iterator, const wchar_t* const End)
{
	if (Iterator == End)
		return true;

	string_view const Str(Iterator, End);

	// \n or \r\n, but not within \r\n or \r\r\n
	if (*Iterator == L'\n' || Str.starts_with(L"\r\n"sv))
		return Iterator == Begin || Iterator[-1] != L'\r';

	return *Iterator == L'\r';
}

string_view regex_exception::to_string(REError const Code)
{
	// TODO: localization
	switch (Code)
	{
	case errNotCompiled:                       return L"Regex must be compiled before usage"sv;
	case errSyntax:                            return L"Expression contains a syntax error"sv;
	case errBrackets:                          return L"Unbalanced brackets"sv;
	case errMaxDepth:                          return L"Maximum recursive brackets level reached"sv;
	case errOptions:                           return L"Invalid options combination"sv;
	case errInvalidBackRef:                    return L"Reference to a non-existent bracket"sv;
	case errInvalidEscape:                     return L"Invalid escape char"sv;
	case errInvalidRange:                      return L"Invalid range value"sv;
	case errInvalidQuantifiersCombination:     return L"Quantifier applied to an invalid object, e.g. a lookahead assertion"sv;
	case errIncompleteGroupStructure:          return L"Incomplete group structure"sv;
	case errSubpatternGroupNameMustBeUnique:   return L"A subpattern name must be unique"sv;
	case errReferenceToUndefinedNamedBracket:  return L"Reference to an undefined named bracket"sv;
	case errVariableLengthLookBehind:          return L"Only fixed length look behind assertions are supported"sv;
	default:
		std::unreachable();
	}
}

#ifdef RE_DEBUG

#ifdef dpf
#undef dpf
#endif
#define dpf(x)

static const wchar_t* ops[]=
{
	L"opNone",
	L"opLineStart",
	L"opLineEnd",
	L"opDataStart",
	L"opDataEnd",
	L"opWordBound",
	L"opNotWordBound",
	L"opType",
	L"opNotType",
	L"opCharAny",
	L"opCharAnyAll",
	L"opSymbol",
	L"opNotSymbol",
	L"opSymbolIgnoreCase",
	L"opNotSymbolIgnoreCase",
	L"opSymbolClass",
	L"opOpenBracket",
	L"opClosingBracket",
	L"opAlternative",
	L"opBackRef",
	L"opNamedBracket",
	L"opRangesBegin",
	L"opRange",
	L"opMinRange",
	L"opSymbolRange",
	L"opSymbolMinRange",
	L"opNotSymbolRange",
	L"opNotSymbolMinRange",
	L"opAnyRange",
	L"opAnyMinRange",
	L"opTypeRange",
	L"opTypeMinRange",
	L"opNotTypeRange",
	L"opNotTypeMinRange",
	L"opClassRange",
	L"opClassMinRange",
	L"opBracketRange",
	L"opBracketMinRange",
	L"opBackRefRange",
	L"opBackRefMinRange",
	L"opRangesEnd",
	L"opAssertionsBegin",
	L"opLookAhead",
	L"opNotLookAhead",
	L"opLookBehind",
	L"opNotLookBehind",
	L"opAsserionsEnd",
	L"opNoReturn",
	L"opRegExpEnd",
};

#else
#define dpf(x)
#endif


#define ISDIGIT(c) std::iswdigit(c)
#define ISSPACE(c) std::iswspace(c)
#define ISWORD(c)  (is_alphanumeric(c) || c == '_')
#define ISLOWER(c) is_lower(c)
#define ISUPPER(c) is_upper(c)
#define ISALPHA(c) is_alpha(c)
#define TOUPPER(c) upper(c)
#define TOLOWER(c) lower(c)

constexpr auto
	slashChar = '/',
	backslashChar = '\\';

//! Max brackets depth
enum
{
	MAXDEPTH = 256,
};

// Locale Info bits
enum
{
	//! Digits
	TYPE_DIGITCHAR = 0x01,
	//! space, newlines tab etc
	TYPE_SPACECHAR = 0x02,
	//! alphanumeric and _
	TYPE_WORDCHAR = 0x04,
	//! lo-case symbol
	TYPE_LOWCASE = 0x08,
	//! up-case symbol
	TYPE_UPCASE = 0x10,
	//! letter
	TYPE_ALPHACHAR = 0x20,
};

// Bracket handler actions
enum
{
	//! Matched Closing bracket
	bhMatch = 1,
	//! Bracket rollback
	bhRollBack = 2,
};

enum
{
	RE_CHAR_COUNT = std::numeric_limits<wchar_t>::max() + 1,
};

static_assert(sizeof(wchar_t) == 2, "512 MB for a bitset is too much, rewrite it.");

static bool isType(wchar_t chr,int type)
{
	switch (type)
	{
	case TYPE_DIGITCHAR: return ISDIGIT(chr) != 0;
	case TYPE_SPACECHAR: return ISSPACE(chr) != 0;
	case TYPE_WORDCHAR:  return ISWORD(chr) != 0;
	case TYPE_LOWCASE:   return ISLOWER(chr) != 0;
	case TYPE_UPCASE:    return ISUPPER(chr) != 0;
	case TYPE_ALPHACHAR: return ISALPHA(chr) != 0;
	}

	return false;
}

struct RegExp::UniSet
{
	std::bitset<RE_CHAR_COUNT> Bits;
	char types{};
	char nottypes{};
	char negative{};

	UniSet()
	{
		types=0;
		nottypes=0;
		negative=0;
	}

	void Reset()
	{
		Bits.reset();
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

		Setter& operator=(bool const val)
		{
			val? set.SetBit(idx) : set.ClearBit(idx);
			return *this;
		}

		explicit operator bool() const
		{
			return set.GetBit(idx);
		}
	};

	bool operator[](wchar_t idx) const
	{
		return GetBit(idx);
	}

	Setter operator[](wchar_t idx)
	{
		return Setter(*this,idx);
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
					if (isType(chr, t))
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
					if (!isType(chr, t))
						return !negative;
				}

				t>>=1;
			}
		}

		const bool Set = Bits[chr];
		return negative? !Set : Set;
	}

	void SetBit(wchar_t  chr)
	{
		Bits.set(chr, true);
	}

	void ClearBit(wchar_t chr)
	{
		Bits.set(chr, false);
	}
};

enum REOp
{
	opNone,
	opLineStart,            // ^
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

	opBackRef,              // \1 \g1 \g-1 \g{1} \g{-1) \g{group1} \p2 \p-2 \p{2} \p{-2} \p{group2}

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

struct REOpCode_data
{
	int op;
#ifdef RE_DEBUG
	int    srcpos;
#endif

	struct SBracket
	{
		RegExp::REOpCode* nextalt;
		int index;
		RegExp::REOpCode* pairindex;
	};

	struct SRange
	{
		union
		{
			SBracket bracket;
			int op;
			wchar_t symbol;
			RegExp::UniSet *symbolclass;
			RegExp::REOpCode* nextalt;
			int refindex;
			int type;
		};
		int min,max;
	};


	struct SAssert
	{
		RegExp::REOpCode* nextalt;
		int length;
		RegExp::REOpCode* pairindex;
	};

	struct SAlternative
	{
		RegExp::REOpCode* nextalt;
		RegExp::REOpCode* endindex;
	};

	union
	{
		SRange range;
		SBracket bracket;
		SAssert assert;
		SAlternative alternative;
		wchar_t symbol;
		RegExp::UniSet *symbolclass;
		int refindex;
		int type;
	};
};

struct RegExp::REOpCode: public REOpCode_data
{
	NONCOPYABLE(REOpCode);
	MOVE_CONSTRUCTIBLE(REOpCode);

	REOpCode():
		REOpCode_data{}
	{
	}

	~REOpCode()
	{
		switch (op)
		{
		case opSymbolClass:delete symbolclass; break;
		case opClassRange:
		case opClassMinRange:delete range.symbolclass; break;
		}
	}
};

RegExp::RegExp():
	firstptr(std::make_unique<UniSet>()),
	first(*firstptr)
{
}

RegExp::~RegExp() = default;
RegExp::RegExp(RegExp&&) noexcept = default;

static wchar_t get_next_char(string_view src, int& pos, const int shift=0, const bool do_throw=false)
{
	++pos;
	if (static_cast<size_t>(pos) < src.size())
		return src[pos];
	pos = static_cast<int>(src.size());
	if (do_throw)
		throw regex_exception(errSyntax, pos + shift);
	return L'\0';
}

// \xh \x{h} \xhh \x{hh} \xhhh \x{hhh} \xhhhh \x{hhhh}
//
static int get_HexChar(string_view src, int& pos, const int shift)
{
	auto c = get_next_char(src, pos);

	const auto curve_pos = pos + shift;
	const auto curves = c == L'{';
	if (curves)
		c = get_next_char(src, pos);
	if (!isxdigit(c))
	{
		if (!curves || c) throw regex_exception(errSyntax, pos + shift);
		throw regex_exception(errBrackets, curve_pos);
	}

	int unicode_char = 0;
	for (int j = 1; j <= 4; ++j)
	{
		c = TOLOWER(c);
		unicode_char = (unicode_char << 4) | (c - (c > '9' ? 'a' - 10 : '0'));
		c = get_next_char(src, pos);
		if (!isxdigit(c))
			break;
	}
	if (!curves)
		--pos;
	else if (c != L'}')
	{
		if (static_cast<size_t>(pos) >= src.size()) throw regex_exception(errBrackets, curve_pos);
		else throw regex_exception(errSyntax, pos + shift);
	}

	return unicode_char;
}

// (?{name}... or (?<name>...
//   ^              ^
static string_view get_NamedGroup(string_view src, int& pos, const int shift)
{
	const auto close_bracket = src[pos] == L'{' ? L'}' : (src[pos] == L'<' ? L'>' : src[pos]);
	const auto start_bracket = pos + shift;

	wchar_t c;
	do { c = get_next_char(src, pos); } while (ISSPACE(c));
	if (!ISALPHA(c))
	{
		if (c && c != close_bracket)
			throw regex_exception(errSyntax, pos + shift);
		else
			throw regex_exception(c? errIncompleteGroupStructure : errBrackets, start_bracket);
	}

	const auto b_pos = pos;
	do { c = get_next_char(src, pos); } while (ISWORD(c));
	const auto e_pos = pos;

	while (ISSPACE(c)) c = get_next_char(src, pos);
	if (c != close_bracket)
		throw regex_exception(c ? errSyntax : errBrackets, c ? pos + shift : start_bracket);

	return src.substr(b_pos, e_pos - b_pos);
}

// \num \pnum \p{num} \p-num \p{-num} \p{name} \gnum ... \g{name}
//  ^    ^     ^       ^      ^        ^        ^         ^
static string_view get_BackRef(string_view src, int& pos, const int shift)
{
	auto c = src[pos];
	if (c == L'p' || c == L'g')
		c = get_next_char(src, pos);

	const auto curve_pos = pos;
	const auto curves = c == L'{';
	if (curves)
		do { c = get_next_char(src, pos); } while (c == L' '); // {g1} { g2 } \g{ -5} ...

	const auto n_pos = pos;
	const auto minus = c == L'-';
	if (minus)
		c = get_next_char(src, pos);

	const auto number_mode = !curves || minus || ISDIGIT(c);
	if ((number_mode && !ISDIGIT(c)) || (!number_mode && !ISALPHA(c)))
	{
		if (curves)
		{
			if (!c) throw regex_exception(errBrackets, curve_pos + shift);
			if (c == L'}') throw regex_exception(errIncompleteGroupStructure, curve_pos + shift);
		}
		throw regex_exception(errSyntax, pos + shift);
	}

	do { c = get_next_char(src, pos); } while (ISDIGIT(c) || (!number_mode && ISWORD(c)));
	const auto e_pos = pos;

	if (curves)
	{
		while (c == L' ') { c = get_next_char(src, pos); }
		if (c != L'}')
			throw regex_exception(c ? errSyntax : errBrackets, (c ? pos : curve_pos) + shift);
	}
	else
		--pos;

	return src.substr(n_pos, e_pos - n_pos);
}

constexpr auto MinCodeLength = 3; //global brackets

int RegExp::CalcLength(string_view src, const int shift)
{
	const auto srclength = static_cast<int>(src.size());
	int length = MinCodeLength;
	int brackets[MAXDEPTH];
	int count=0;
	int save;
	bracketscount=1;
	int inquote=0;

	const auto next_char = [src, shift](int& pos, const bool do_throw = false)
	{
		return get_next_char(src, pos, shift, do_throw);
	};

	const auto test_char = [src](const int pos)
	{
		return static_cast<size_t>(pos) < src.size() ? src[pos] : L'\0';
	};

	const auto hex_char = [src, shift](int& pos)
	{
		return get_HexChar(src, pos, shift);
	};

	const auto named_group = [src, shift](int& pos)
	{
		return get_NamedGroup(src, pos, shift);
	};

	const auto back_ref = [src, shift](int& pos)
	{
		return get_BackRef(src, pos, shift);
	};

	for (int i=0; i<srclength; i++,length++)
	{
		if (inquote && src[i]!=backslashChar && (i + 1 == srclength || src[i+1] != L'E'))
		{
			continue;
		}

		if (src[i] == backslashChar)
		{
			auto c = next_char(i);
			if (i >= srclength)
				continue;

			if (c == L'Q')
			{
				inquote = 1; continue;
			}

			if (c == L'E')
			{
				inquote = 0; continue;
			}

			if (c == L'x')
			{
				hex_char(i);
				continue;
			}

			// \n
			// \pn \p-n \p{n} \p{-n} \p{name}
			// \gn \g-n \g{n} \g{-n} \g{name}
			if (c == L'p' || c == L'g' || ISDIGIT(c))
			{
				back_ref(i);
			}
			continue;
		}

		switch (src[i])
		{
			case L'(':
			{
				brackets[count++]=i;
				if (count >= MAXDEPTH)
					throw regex_exception(errMaxDepth, i + shift);

				if (test_char(i + 1) == L'?')
				{
					i += 2;
					auto c1 = test_char(i), c2 = test_char(i + 1);
					if (c1 == L'{' || (c1 == L'<' && c2 != L'=' && c2 != L'!'))
					{
						named_group(i);
						++bracketscount;
					}
				}
				else
				{
					++bracketscount;
				}

				break;
			}
			case L')':
			{
				count--;

				if (count < 0)
					throw regex_exception(errBrackets,i + shift);

				break;
			}
			case L'{':
			case L'*':
			case L'+':
			case L'?':
			{
				length--;

				if (src[i] == L'{')
				{
					save=i;

					while (i < srclength && src[i] != L'}')
						++i;

					if (i >= srclength)
						throw regex_exception(errBrackets, save + shift);
				}

				if (i + 1 != srclength && src[i + 1] == L'?')
					++i;

				break;
			}
			case L'[':
			{
				save=i;

				++i;

				// []]
				if (i < srclength && src[i] == L']')
					++i;

				while (i < srclength && src[i] != L']')
					i += (backslashChar == src[i] && src[i+1] ? 2 : 1);

				if (i >= srclength)
					throw regex_exception(errBrackets, save + shift);

				break;
			}
		}
	}

	if (count)
	{
		throw regex_exception(errBrackets, brackets[0] + shift);
	}
	return length;
}

void RegExp::Compile(string_view const src, int options)
{
	SCOPE_FAIL
	{
		code.clear();
		NamedGroups.clear();
	};

	havefirst=0;

	code.clear();
	NamedGroups.clear();

	string_view Regex;

	if (options&OP_PERLSTYLE)
	{
		if (!src.starts_with(slashChar))
			throw regex_exception(errSyntax, 0);

		const auto End = src.rfind(slashChar);
		if (End == 0)
			throw regex_exception(errSyntax, static_cast<int>(src.size()));

		Regex = src.substr(1, End - 1);
		const auto Options = src.substr(End + 1);
		for (auto i = Options.cbegin(); i != Options.cend(); ++i)
		{
			switch (*i)
			{
				case L'i':options|=OP_IGNORECASE; break;
				case L's':options|=OP_SINGLELINE; break;
				case L'm':options|=OP_MULTILINE; break;
				case L'x':options|=OP_XTENDEDSYNTAX; break;
				case L'o':options|=OP_OPTIMIZE; break;
				default: throw regex_exception(errOptions, 1 + Regex.size() + 1 + (i - Options.cbegin()));
			}
		}
	}
	else
	{
		Regex = src;
	}

	ignorecase = options & OP_IGNORECASE ? 1 : 0;

	const auto shift = static_cast<int>(Regex.data() - src.data());
	code.resize(CalcLength(Regex, shift));

	InnerCompile(Regex.data(), static_cast<int>(Regex.size()), shift, options);

	minlength = 0;

	if (options & OP_OPTIMIZE)
		Optimize();
}

static int GetNum(const wchar_t* src,int& i)
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

static int CalcPatternLength(const RegExp::REOpCode* from, const RegExp::REOpCode* to)
{
	int len=0;
	int altcnt=0;
	int altlen=-1;

	for (; from <= to; ++from)
	{
		switch (from->op)
		{
				//zero width
			case opLineStart:
			case opLineEnd:
			case opDataStart:
			case opDataEnd:
			case opWordBound:
			case opNotWordBound:
				continue;

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
				const auto l = CalcPatternLength(from + 1, from->bracket.pairindex - 1);

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

				const auto l = CalcPatternLength(from + 1,from->bracket.pairindex - 1);

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

void RegExp::InnerCompile(const wchar_t* src, const int srclength, const int shift, int options)
{
	REOpCode* brackets[MAXDEPTH];
	// current brackets depth
	// one place reserved for surrounding 'main' brackets
	int brdepth=1;
	// counter of normal brackets
	int brcount=0;
	// counter of closed brackets
	// used to check correctness of back-references
	std::vector<bool> closedbrackets(1);
	// quoting is active
	int inquote=0;
	maxbackref=0;
	UniSet *tmpclass;
	code[0].op=opOpenBracket;
	code[0].bracket.index = 0;
	int pos=1;
	brackets[0]=code.data();
#ifdef RE_DEBUG
	resrc = L'(';
	resrc.append(src, srclength).append(L")←");
#endif
	havelookahead=0;

	const auto next_char = [src, srclength, shift](int& pos, const bool do_throw = false)
	{
		return get_next_char({src, static_cast<size_t>(srclength)}, pos, shift, do_throw);
	};

	const auto peek_char = [src, srclength](const int pos)
	{
		return pos < srclength ? src[pos] : L'\0';
	};

	const auto hex_char = [src, srclength, shift](int& pos)
	{
		return get_HexChar({src, static_cast<size_t>(srclength)}, pos, shift);
	};

	const auto named_group = [src, srclength, shift](int& pos)
	{
		return get_NamedGroup({src, static_cast<size_t>(srclength)}, pos, shift);
	};

	const auto back_ref = [src, srclength, shift](int& pos)
	{
		return get_BackRef({ src, static_cast<size_t>(srclength) }, pos, shift);
	};

	for (int i = 0; i < srclength; i++)
	{
		auto op = &code[pos];
		pos++;
#ifdef RE_DEBUG
		op->srcpos=i+1;
#endif

		if (inquote && src[i] != backslashChar)
		{
			op->op = ignorecase ? opSymbolIgnoreCase : opSymbol;
			op->symbol = ignorecase ? TOLOWER(src[i]) : src[i];

			if (ignorecase && TOUPPER(op->symbol) == op->symbol) op->op = opSymbol;

			continue;
		}

		if (src[i] == backslashChar)
		{
			auto c = next_char(i);

			if (inquote && c != 'E')
			{
				op->op = opSymbol;
				op->symbol = backslashChar;
				op = &code[pos];
				pos++;
				op->op = ignorecase?opSymbolIgnoreCase:opSymbol;
				op->symbol = ignorecase ? TOLOWER(src[i]) : src[i];

				if (ignorecase && TOUPPER(op->symbol) == op->symbol) op->op=opSymbol;

				continue;
			}

			op->op=opType;

			switch (c)
			{
				case L'Q':inquote=1; pos--; continue;
				case L'E':inquote=0; pos--; continue;
				case L'b':op->op=opWordBound; continue;
				case L'B':op->op=opNotWordBound; continue;
				case L'D':op->op=opNotType; [[fallthrough]];
				case L'd':op->type=TYPE_DIGITCHAR; continue;
				case L'S':op->op=opNotType; [[fallthrough]];
				case L's':op->type=TYPE_SPACECHAR; continue;
				case L'W':op->op=opNotType; [[fallthrough]];
				case L'w':op->type=TYPE_WORDCHAR; continue;
				case L'U':op->op=opNotType; [[fallthrough]];
				case L'u':op->type=TYPE_UPCASE; continue;
				case L'L':op->op=opNotType; [[fallthrough]];
				case L'l':op->type=TYPE_LOWCASE; continue;
				case L'I':op->op=opNotType; [[fallthrough]];
				case L'i':op->type=TYPE_ALPHACHAR; continue;
				case L'A':op->op=opDataStart; continue;
				case L'Z':op->op=opDataEnd; continue;
				case L'n':op->op=opSymbol; op->symbol=L'\n'; continue;
				case L'r':op->op=opSymbol; op->symbol=L'\r'; continue;
				case L't':op->op=opSymbol; op->symbol=L'\t'; continue;
				case L'f':op->op=opSymbol; op->symbol=L'\f'; continue;
				case L'e':op->op=opSymbol; op->symbol=L'\x1B'; continue;
				case L'O':op->op=opNoReturn; continue;

				case L'x': // \xH \x{H} ... \xHHHH \x{HHHH}
				{
					const auto unicode_char = hex_char(i);

					op->op = ignorecase ? opSymbolIgnoreCase : opSymbol;
					op->symbol = ignorecase ? TOLOWER(unicode_char) : unicode_char;
					if (ignorecase && unicode_char == TOLOWER(unicode_char)) op->op = opSymbol;

					continue;
				}

				default:
				{
					if (c == L'p' || c == L'g' || ISDIGIT(c)) // \1 \p1 \p-1 \p{1} \p{-1} \p{g1} \g2 ... \g{g2}
					{
						const auto bref = back_ref(i);
						const auto b_pos = static_cast<int>(bref.data() - src);

						int number = -1;
						const auto number_mode = ISDIGIT(bref[0]) || bref[0] == L'-';
						if (number_mode)
						{
							number = from_string<int>(bref);
							if (number < 0) number = brcount + 1 + number; // -1 == brcount
							if (number <= 0 || number > brcount || !closedbrackets[number])
								throw regex_exception(errInvalidBackRef, b_pos + shift);
						}
						else
						{
							const auto found = NamedGroups.find(bref);
							if (found != NamedGroups.cend())
								number = static_cast<int>(found->second);
							if (number <= 0)
								throw regex_exception(errReferenceToUndefinedNamedBracket, b_pos + shift);
						}

						op->op = opBackRef;
						op->refindex = number;

					}
					else
					{
						if (!c)
							throw regex_exception(errSyntax, i + shift - 1);
						if (options & OP_STRICT && ISALPHA(c))
							throw regex_exception(errInvalidEscape, i + shift - 1);

						op->op = ignorecase ? opSymbolIgnoreCase:opSymbol;
						op->symbol = ignorecase ? TOLOWER(c) : c;
						if (TOLOWER(op->symbol) == TOUPPER(op->symbol))
							op->op = opSymbol;
					}
				}
				continue;
			}
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

				if ((brdepth + 1) >= MAXDEPTH)
					throw regex_exception(errMaxDepth, i + shift);

				brackets[brdepth++]=op;
				op->op=opAlternative;
				continue;
			}
			case '(':
			{
				op->op=opOpenBracket;

				if (src[i+1] == L'?')
				{
					i += 2;
					const char c1 = peek_char(i);
					const char c2 = peek_char(i+1);

					switch (c1)
					{
						case L':': op->bracket.index=-1; break;
						case L'=': op->op=opLookAhead; havelookahead=1; break;
						case L'!': op->op=opNotLookAhead; havelookahead=1; break;

						case L'<':
						{
							if (c2 == L'=')
							{
								++i;
								op->op = opLookBehind;
								break;
							}
							else if (c2 == L'!')
							{
								++i;
								op->op = opNotLookBehind;
								break;
							}
							} [[fallthrough]];
						case L'{':
						{
							auto group_name = named_group(i);

							op->op = opOpenBracket; // opNamedBracket;
							++brcount;
							closedbrackets.push_back(false);
							op->bracket.index = brcount;

							if (!NamedGroups.emplace(group_name, brcount).second)
								throw regex_exception(errSubpatternGroupNameMustBeUnique, (group_name.data() - src) + shift);

						} break;

						default:
						{
							throw regex_exception(errSyntax, i + shift);
						}
					}
				}
				else
				{
					++brcount;
					closedbrackets.push_back(false);
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
						int l=CalcPatternLength(brackets[brdepth] + 1, op - 1);

						if (l == -1)
							throw regex_exception(errVariableLengthLookBehind, i + shift);

						brackets[brdepth]->assert.length=l;
					}
						[[fallthrough]];

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
				op->symbolclass=new UniSet();
				tmpclass=op->symbolclass;

				// []]
				for (auto FirstIndex = i; i == FirstIndex || src[i] != ']'; i++)
				{
					if (src[i]==backslashChar)
					{
						i++;
						int isnottype=0;
						int type=0;
						lastchar=-1;

						switch (src[i])
						{
							case L'D':isnottype=1; [[fallthrough]];
							case L'd':type=TYPE_DIGITCHAR; break;
							case L'W':isnottype=1; [[fallthrough]];
							case L'w':type=TYPE_WORDCHAR; break;
							case L'S':isnottype=1; [[fallthrough]];
							case L's':type=TYPE_SPACECHAR; break;
							case L'L':isnottype=1; [[fallthrough]];
							case L'l':type=TYPE_LOWCASE; break;
							case L'U':isnottype=1; [[fallthrough]];
							case L'u':type=TYPE_UPCASE; break;
							case L'I':isnottype=1; [[fallthrough]];
							case L'i':type=TYPE_ALPHACHAR; break;
							case L'n':lastchar=L'\n'; break;
							case L'r':lastchar=L'\r'; break;
							case L't':lastchar=L'\t'; break;
							case L'f':lastchar=L'\f'; break;
							case L'e':lastchar=L'\x1B'; break;
							case L'x':
							{
								lastchar = hex_char(i);
								dpf((L"Last char=%c(%04x)\n", lastchar, lastchar));
								break;
							}

							default:
							{
								if (options & OP_STRICT && ISALPHA(src[i]))
									throw regex_exception(errInvalidEscape, i + shift - 1);

								lastchar = src[i];
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
									else
										throw regex_exception(errSyntax, i + shift);
								}
								else
								{
									tmpclass->SetBit('-');
									classsize++;
									continue;
								}
							}

							i++;
							dpf((L"from %d to %d\n",lastchar,to));

							for (int j=lastchar; j<=to; j++)
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
				}

				if (classsize==1)
				{
					delete op->symbolclass;
					op->symbolclass = nullptr;
					tmpclass = nullptr;
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
						min=0; max=1;
						break;
					}
					case '{':
					{
						i++;
						int save=i;
						min=GetNum(src,i);
						max=min;

						if (min<0)
							throw regex_exception(errInvalidRange, save + shift);

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

								if (max<min)
									throw regex_exception(errInvalidRange, save + shift);
							}
						}

						if (src[i] != '}')
							throw regex_exception(errInvalidRange, save + shift);
					}
				}

				pos--;
				op = code.data() + pos - 1;

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
						throw regex_exception(errInvalidQuantifiersCombination, i + shift);
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

						if (op->op != opOpenBracket /* && op->op != opNamedBracket */)
							throw regex_exception(errInvalidQuantifiersCombination, i + shift);


						op->range.min=min;
						op->range.max=max;
						op->op=opBracketRange;
						break;
					}
					default:
					{
						dpf((L"op->=%d\n",op->op));
						throw regex_exception(errInvalidQuantifiersCombination, i + shift);
					}
				}//switch(code.op)

				if (src[i+1]=='?')
				{
					++op->op;
					++i;
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
			[[fallthrough]];
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

	auto op = &code[pos];
	pos++;
	brdepth--;

	while (brdepth>=0 && brackets[brdepth]->op==opAlternative)
	{
		brackets[brdepth]->alternative.endindex=op;
		brdepth--;
	}

	op->op=opClosingBracket;
	op->bracket.pairindex = code.data();
	code[0].bracket.pairindex=op;
#ifdef RE_DEBUG
	op->srcpos=i;
#endif
	op = &code[pos];
	op->op=opRegExpEnd;
#ifdef RE_DEBUG
	op->srcpos=i+1;
#endif
}

struct RegExp::StateStackItem
{
	int op{};
	const REOpCode* pos{};
	const wchar_t* savestr{};
	const wchar_t* startstr{};
	int min{};
	int cnt{};
	int max{};
	int forward{};
};

class RegExp::state_stack
{
public:
	using state = std::vector<StateStackItem, stack_allocator<StateStackItem, 4096>>;

private:
	state::allocator_type::arena_type m_Arena;

public:
	state State{ m_Arena };
};


static const RegExp::StateStackItem& FindStateByPos(std::span<RegExp::StateStackItem const> const stack, RegExp::REOpCode* pos, int op)
{
	return *std::ranges::find_if(stack | std::views::reverse, [&](const auto& i){ return i.pos == pos && i.op == op; });
}

int RegExp::StrCmp(const wchar_t*& str, const wchar_t* start, const wchar_t* end) const
{
	const wchar_t* save=str;

	if (ignorecase)
	{
		while (start<end)
		{
			if (TOLOWER(*str)!=TOLOWER(*start)) {str=save; return 0;}

			str++;
			start++;
		}
	}
	else
	{
		while (start<end)
		{
			if (*str!=*start) {str=save; return 0;}

			str++;
			start++;
		}
	}

	return 1;
}

static constexpr RegExpMatch DefaultMatch{ -1, -1 };

bool RegExp::InnerMatch(const wchar_t* start, const wchar_t* str, const wchar_t* strend, regex_match& RegexMatch, state_stack& StateStack) const
{
	int i,j;
	int minimizing;
	const REOpCode* tmp=nullptr;
	RegExpMatch* m;
	UniSet *cl;
	int inrangebracket=0;

	auto& match = RegexMatch.Matches;
	auto& stack = StateStack.State;

	stack.clear();
	match.clear();
	match.resize(bracketscount, DefaultMatch);

	for(const auto* op = code.data(), *end = op + code.size(); op != end; ++op)
	{
		//dpf(("op:%s,\tpos:%d,\tstr:%d\n",ops[op->op],pos,str-start));
		dpf((L"=================\n"));
		dpf((L"S:%s\n%*s\n", start, str - start + 3, "^"));
		dpf((L"R:%s\n%*s\n", resrc.data(), op->srcpos + 3, "^"));

		if (str<=strend)
		{
			const auto MinSkip = [&](StateStackItem& st, auto const& cmp)
			{
				int jj;
				switch (std::next(op)->op)
				{
				case opSymbol:
					jj = std::next(op)->symbol;
					if(*str!=jj)
					{
						while(str<strend && cmp(str) && st.max--)
						{
							str++;
							if(str[0]==jj)
								break;
						}
					}
					break;

				case opNotSymbol:
					jj = std::next(op)->symbol;
					if(*str==jj)
					{
						while(str<strend && cmp(str) && st.max--)
						{
							str++;
							if(str[0]!=jj)
								break;
						}
					}
					break;

				case opSymbolIgnoreCase:
					jj = std::next(op)->symbol;
					if(TOLOWER(*str)!=jj)
					{
						while(str<strend && cmp(str) && st.max--)
						{
							str++;
							if(TOLOWER(str[0])==jj)
								break;
						}
					}
					break;

				case opNotSymbolIgnoreCase:
					jj = std::next(op)->symbol;
					if(TOLOWER(*str)==jj)
					{
						while(str<strend && cmp(str) && st.max--)
						{
							str++;
							if(TOLOWER(str[0])!=jj)
								break;
						}
					}
					break;

				case opType:
					jj = std::next(op)->type;
					if(!isType(*str,jj))
					{
						while(str<strend && cmp(str) && st.max--)
						{
							str++;
							if(isType(str[0],jj))
								break;
						}
					}
					break;

				case opNotType:
					jj = std::next(op)->type;
					if(isType(*str,jj))
					{
						while(str<strend && cmp(str) && st.max--)
						{
							str++;
							if(!isType(str[0],jj))
								break;
						}
					}
					break;

				case opSymbolClass:
					cl = std::next(op)->symbolclass;
					if(!cl->GetBit(*str))
					{
						while(str<strend && cmp(str) && st.max--)
						{
							str++;
							if(cl->GetBit(str[0]))
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
					if (str == start || is_far_eol_before(start, str, strend))
						continue;

					break;
				}
				case opLineEnd:
				{
					if (str == strend || is_far_eol(start, str, strend))
						continue;

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
				case opNotWordBound:
				{
					const auto IsWordBoundary = [&]
					{
						if (start == strend)
							return false;

						if (str == start)
							return ISWORD(*str);

						if (str == strend)
							return ISWORD(str[-1]);

						return ISWORD(str[-1]) != ISWORD(*str);
					}();

					if (op->op == opWordBound? IsWordBoundary : !IsWordBoundary)
						continue;

					break;
				}
				case opType:
				{
					if (str != strend && isType(*str, op->type))
					{
						str++;
						continue;
					}

					break;
				}
				case opNotType:
				{
					if (str != strend && !isType(*str, op->type))
					{
						str++;
						continue;
					}

					break;
				}
				case opCharAny:
				{
					if (str != strend && !IsEol(*str))
					{
						str++;
						continue;
					}

					break;
				}
				case opCharAnyAll:
				{
					if (str != strend)
					{
						str++;
						continue;
					}

					break;
				}
				case opSymbol:
				{
					if (str != strend && *str == op->symbol)
					{
						str++;
						continue;
					}

					break;
				}
				case opNotSymbol:
				{
					if (str != strend && *str != op->symbol)
					{
						str++;
						continue;
					}

					break;
				}
				case opSymbolIgnoreCase:
				{
					if (str != strend && TOLOWER(*str) == op->symbol)
					{
						str++;
						continue;
					}

					break;
				}
				case opNotSymbolIgnoreCase:
				{
					if (str != strend && TOLOWER(*str)!=op->symbol)
					{
						str++;
						continue;
					}

					break;
				}
				case opSymbolClass:
				{
					if (str != strend && op->symbolclass->GetBit(*str))
					{
						str++;
						continue;
					}

					break;
				}
				case opOpenBracket:
				{
					if (op->bracket.index >= 0)
					{
						//if (inrangebracket) Mantis#1388
						{
							StateStackItem st;
							st.op=opOpenBracket;
							st.pos=op;
							st.min = match[op->bracket.index].start;
							st.max = match[op->bracket.index].end;
							stack.emplace_back(st);
						}

						match[op->bracket.index].start = str - start;
					}

					if (op->bracket.nextalt)
					{
						StateStackItem st;
						st.op = opAlternative;
						st.pos=op->bracket.nextalt;
						st.savestr=str;
						stack.emplace_back(st);
					}

					continue;
				}

				case opClosingBracket:
				{
					switch (op->bracket.pairindex->op)
					{
						case opOpenBracket:
						{
							if (op->bracket.index >= 0)
							{
								match[op->bracket.index].end = str - start;
							}
							continue;
						}

						case opBracketRange:
						{
							auto st = FindStateByPos(stack, op->bracket.pairindex,opBracketRange);

							if (str==st.startstr)
							{
								if (op->range.bracket.index >= 0)
								{
									match[op->range.bracket.index].end = str - start;
								}

								inrangebracket--;
								continue;
							}

							if (st.min>0)st.min--;

							if (st.min)
							{
								st.max--;
								st.startstr=str;
								st.savestr=str;
								op=st.pos;
								stack.emplace_back(st);

								if (op->range.bracket.index >= 0)
								{
									StateStackItem Item;
									match[op->range.bracket.index].start = str - start;
									Item.op=opOpenBracket;
									Item.pos=op;
									Item.min = match[op->range.bracket.index].start;
									Item.max = match[op->range.bracket.index].end;
									stack.emplace_back(Item);
								}

								if (op->range.bracket.nextalt)
								{
									StateStackItem Item;
									Item.op=opAlternative;
									Item.pos=op->range.bracket.nextalt;
									Item.savestr=str;
									stack.emplace_back(Item);
								}

								continue;
							}

							st.max--;

							if (!st.max)
							{
								if (op->range.bracket.index >= 0)
								{
									match[op->range.bracket.index].end = str - start;
								}

								inrangebracket--;
								continue;
							}

							if (op->range.bracket.index >= 0)
							{
								match[op->range.bracket.index].end = str - start;
								tmp=op;
							}

							st.startstr=str;
							st.savestr=str;
							op=st.pos;
							stack.emplace_back(st);

							if (op->range.bracket.index >= 0)
							{
								StateStackItem Item;
								Item.op=opOpenBracket;
								Item.pos=tmp;
								Item.min = match[op->range.bracket.index].start;
								Item.max = match[op->range.bracket.index].end;
								stack.emplace_back(Item);
								match[op->range.bracket.index].start = str - start;
							}

							if (op->range.bracket.nextalt)
							{
								StateStackItem Item;
								Item.op=opAlternative;
								Item.pos=op->range.bracket.nextalt;
								Item.savestr=str;
								stack.emplace_back(Item);
							}

							continue;
						}
						case opBracketMinRange:
						{
							auto& ps = FindStateByPos(stack, op->bracket.pairindex, opBracketMinRange);
							auto st = ps;

							if (st.min>0)st.min--;

							if (st.min)
							{
								st.max--;
								st.startstr=str;
								st.savestr=str;
								op=st.pos;
								stack.emplace_back(st);

								if (op->range.bracket.index >= 0)
								{
									match[op->range.bracket.index].start = str - start;
									st.op=opOpenBracket;
									st.pos=op;
									st.min = match[op->range.bracket.index].start;
									st.max = match[op->range.bracket.index].end;
									stack.emplace_back(st);
								}

								if (op->range.bracket.nextalt)
								{
									st.op=opAlternative;
									st.pos=op->range.bracket.nextalt;
									st.savestr=str;
									stack.emplace_back(st);
								}

								continue;
							}

							if (op->range.bracket.index >= 0)
							{
								match[op->range.bracket.index].end = str - start;
							}

							st.max--;
							inrangebracket--;

							if (!st.max)continue;

							st.forward=str>ps.startstr?1:0;
							st.startstr=str;
							st.savestr=str;
							stack.emplace_back(st);

							if (op->range.bracket.nextalt)
							{
								st.op=opAlternative;
								st.pos=op->range.bracket.nextalt;
								st.savestr=str;
								stack.emplace_back(st);
							}

							continue;
						}
						case opLookAhead:
						{
							tmp=op->bracket.pairindex;

							while (stack.back().pos != tmp || stack.back().op != opLookAhead)
							{
								stack.pop_back();
							}
							str = stack.back().savestr;
							stack.pop_back();
							continue;
						}
						case opNotLookAhead:
						{
							while (stack.back().op!=opNotLookAhead)
							{
								stack.pop_back();
							}
							str = stack.back().savestr;
							stack.pop_back();
							break;
						}
						case opLookBehind:
						{
							continue;
						}
						case opNotLookBehind:
						{
							stack.back().forward=0;
							break;
						}
					}//switch(code[pairindex].op)

					break;
				}//case opClosingBracket
				case opAlternative:
				{
					op = std::prev(op->alternative.endindex);
					continue;
				}

				case opBackRef:
				{
					m = &match[op->refindex];
					if (m->start== - 1 || m->end == -1)
						break;

					if (ignorecase)
					{
						j = m->end;
						for (i = m->start; i < j; i++, str++)
						{
							if (TOLOWER(start[i]) != TOLOWER(*str))
								break;
							if (str > strend)
								break;
						}
						if (i < j)
							break;
					}
					else
					{
						j = m->end;
						for (i = m->start; i < j; i++, str++)
						{
							if (start[i] != *str)
								break;
							if (str > strend)
								break;
						}
						if (i < j)
							break;
					}

					continue;
				}

				case opAnyRange:
				case opAnyMinRange:
				{
					StateStackItem st;
					st.op=op->op;
					minimizing=op->op==opAnyMinRange;
					j=op->range.min;
					st.max=op->range.max-j;

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

						st.startstr=str;

						if (!minimizing)
						{
							while (str<strend && *str!=0x0d && *str!=0x0a && st.max--)str++;
						}
						else
						{
							MinSkip(st, [](const wchar_t* Str) { return !IsEol(*Str); });

							if (st.max==-1)break;
						}
					}
					else
					{
						//opCharAnyAll:
						str+=j;

						if (str>strend)break;

						st.startstr=str;

						if (!minimizing)
						{
							if (st.max>=0)
							{
								if (str+st.max<strend)
								{
									str+=st.max;
									st.max=0;
								}
								else
								{
									st.max -= static_cast<int>(strend - str);
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
							MinSkip(st, [](const wchar_t*) { return true; });

							if (st.max==-1)break;
						}
					}

					if (op->range.max==j)continue;

					st.savestr=str;
					st.pos=op;
					stack.emplace_back(st);
					continue;
				}
				case opSymbolRange:
				case opSymbolMinRange:
				{
					StateStackItem st;
					st.op=op->op;
					minimizing=op->op==opSymbolMinRange;
					j=op->range.min;
					st.max=op->range.max-j;

					if (ignorecase)
					{
						for (i=0; i<j; i++,str++)
						{
							if (str>strend || TOLOWER(*str)!=op->range.symbol)break;
						}

						if (i<j)break;

						st.startstr=str;

						if (!minimizing)
						{
							while (str<strend && TOLOWER(*str)==op->range.symbol && st.max--)str++;
						}
						else
						{
							MinSkip(st, [op](const wchar_t* Str) { return TOLOWER(*Str) == op->range.symbol; });

							if (st.max==-1)break;
						}
					}
					else
					{
						for (i=0; i<j; i++,str++)
						{
							if (str>strend || *str!=op->range.symbol)break;
						}

						if (i<j)break;

						st.startstr=str;

						if (!minimizing)
						{
							while (str<strend && *str==op->range.symbol && st.max--)str++;
						}
						else
						{
							MinSkip(st, [op](const wchar_t* Str) { return *Str == op->range.symbol; });

							if (st.max==-1)break;
						}
					}

					if (op->range.max==j)continue;

					st.savestr=str;
					st.pos=op;
					stack.emplace_back(st);
					continue;
				}
				case opNotSymbolRange:
				case opNotSymbolMinRange:
				{
					StateStackItem st;
					st.op=op->op;
					minimizing=op->op==opNotSymbolMinRange;
					j=op->range.min;
					st.max=op->range.max-j;

					if (ignorecase)
					{
						for (i=0; i<j; i++,str++)
						{
							if (str>strend || TOLOWER(*str)==op->range.symbol)break;
						}

						if (i<j)break;

						st.startstr=str;

						if (!minimizing)
						{
							while (str<strend && TOLOWER(*str)!=op->range.symbol && st.max--)str++;
						}
						else
						{
							MinSkip(st, [op](const wchar_t* Str) { return TOLOWER(*Str) != op->range.symbol; });

							if (st.max==-1)break;
						}
					}
					else
					{
						for (i=0; i<j; i++,str++)
						{
							if (str>strend || *str==op->range.symbol)break;
						}

						if (i<j)break;

						st.startstr=str;

						if (!minimizing)
						{
							while (str<strend && *str!=op->range.symbol && st.max--)str++;
						}
						else
						{
							MinSkip(st, [op](const wchar_t* Str) { return *Str != op->range.symbol; });

							if (st.max==-1)break;
						}
					}

					if (op->range.max==j)continue;

					st.savestr=str;
					st.pos=op;
					stack.emplace_back(st);
					continue;
				}
				case opClassRange:
				case opClassMinRange:
				{
					StateStackItem st;
					st.op=op->op;
					minimizing=op->op==opClassMinRange;
					j=op->range.min;
					st.max=op->range.max-j;

					for (i=0; i<j; i++,str++)
					{
						if (str>strend || !op->range.symbolclass->GetBit(*str))break;
					}

					if (i<j)break;

					st.startstr=str;

					if (!minimizing)
					{
						while (str<strend && op->range.symbolclass->GetBit(*str) && st.max--)str++;
					}
					else
					{
						MinSkip(st, [op](const wchar_t* Str) { return op->range.symbolclass->GetBit(*Str); });

						if (st.max==-1)break;
					}

					if (op->range.max==j)continue;

					st.savestr=str;
					st.pos=op;
					stack.emplace_back(st);
					continue;
				}
				case opTypeRange:
				case opTypeMinRange:
				{
					StateStackItem st;
					st.op=op->op;
					minimizing=op->op==opTypeMinRange;
					j=op->range.min;
					st.max=op->range.max-j;

					for (i=0; i<j; i++,str++)
					{
						if (str>strend || !isType(*str,op->range.type))break;
					}

					if (i<j)break;

					st.startstr=str;

					if (!minimizing)
					{
						while (str<strend && isType(*str,op->range.type) && st.max--)str++;
					}
					else
					{
						MinSkip(st, [op](const wchar_t* Str) { return isType(*Str, op->range.type); });

						if (st.max==-1)break;
					}

					if (op->range.max==j)continue;

					st.savestr=str;
					st.pos=op;
					stack.emplace_back(st);
					continue;
				}
				case opNotTypeRange:
				case opNotTypeMinRange:
				{
					StateStackItem st;
					st.op = op->op;
					minimizing=op->op==opNotTypeMinRange;
					j=op->range.min;
					st.max=op->range.max-j;

					for (i=0; i<j; i++,str++)
					{
						if (str>strend || isType(*str,op->range.type))break;
					}

					if (i<j)break;

					st.startstr=str;

					if (!minimizing)
					{
						while (str<strend && !isType(*str,op->range.type) && st.max--)str++;
					}
					else
					{
						MinSkip(st, [op](const wchar_t* Str) { return !isType(*Str, op->range.type); });

						if (st.max==-1)break;
					}

					if (op->range.max==j)continue;

					st.savestr=str;
					st.pos=op;
					stack.emplace_back(st);
					continue;
				}
				case opBackRefRange:
				case opBackRefMinRange:
				{
					StateStackItem st;
					st.op = op->op;
					minimizing = op->op == opBackRefMinRange;
					j=op->range.min;
					st.max=op->range.max-j;
					m = &match[op->range.refindex];

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

					st.startstr=str;

					if (!minimizing)
					{
						while (str<strend && StrCmp(str,start+m->start,start+m->end) && st.max--);
					}
					else
					{
						MinSkip(st, [&](const wchar_t* Str) { return this->StrCmp(Str, start + m->start, start + m->end) != 0; });

						if (st.max==-1)break;
					}

					if (op->range.max==j)continue;

					st.savestr=str;
					st.pos=op;
					stack.emplace_back(st);
					continue;
				}
				case opBracketRange:
				case opBracketMinRange:
				{
					if (inrangebracket && op->range.bracket.index >= 0)
					{
						StateStackItem st;
						st.op=opOpenBracket;
						st.pos=op->range.bracket.pairindex;
						st.min = match[op->range.bracket.index].start;
						st.max = match[op->range.bracket.index].end;
						stack.emplace_back(st);
					}

					{
						StateStackItem st;
						st.op = op->op;
						st.pos = op;
						st.min = op->range.min;
						st.max = op->range.max;
						st.startstr = str;
						st.savestr = str;
						st.forward = 1;
						stack.emplace_back(st);
					}

					if (op->range.nextalt)
					{
						StateStackItem st;
						st.op=opAlternative;
						st.pos=op->range.bracket.nextalt;
						st.savestr=str;
						stack.emplace_back(st);
					}

					if (op->range.bracket.index >= 0)
					{
						match[op->range.bracket.index].start=
						    /*match[op->range.bracket.index].end=*/ str - start;
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
					StateStackItem st;
					st.op=op->op;
					st.savestr=str;
					st.pos=op;
					st.forward=1;
					stack.emplace_back(st);

					if (op->assert.nextalt)
					{
						StateStackItem Item;
						Item.op=opAlternative;
						Item.pos=op->assert.nextalt;
						Item.savestr=str;
						stack.emplace_back(Item);
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

					{
						StateStackItem st;
						st.op = op->op;
						st.savestr = str;
						st.pos = op;
						st.forward = 1;
						str -= op->assert.length;
						stack.emplace_back(st);
					}

					if (op->assert.nextalt)
					{
						StateStackItem st;
						st.op=opAlternative;
						st.pos=op->assert.nextalt;
						st.savestr=str;
						stack.emplace_back(st);
					}

					continue;
				}
				case opNoReturn:
				{
					StateStackItem st;
					st.op=opNoReturn;
					stack.emplace_back(st);
					continue;
				}
				case opRegExpEnd:
					return true;
			}//switch(op)
		}

		for (;; stack.pop_back())
		{
			if (stack.empty())
				return false;

			auto& ps = stack.back();

			switch (ps.op)
			{
				case opAlternative:
				{
					str = ps.savestr;
					op = ps.pos;

					if (op->alternative.nextalt)
					{
						ps.pos=op->alternative.nextalt;
					}
					else
					{
						stack.pop_back();
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
					str = ps.savestr-1;
					op = ps.pos;

					if (str < ps.startstr)
					{
						continue;
					}

					ps.savestr = str;
					break;
				}
				case opBackRefRange:
				{
					m = &match[ps.pos->range.refindex];
					str = ps.savestr-(m->end-m->start);
					op = ps.pos;

					if (str < ps.startstr)
					{
						continue;
					}

					ps.savestr = str;
					break;
				}
				case opAnyMinRange:
				{
					if (!(ps.max--))
						continue;

					str = ps.savestr;
					op = ps.pos;

					if (ps.pos->range.op == opCharAny)
					{
						if (str < strend && !IsEol(*str))
						{
							str++;
							ps.savestr = str;
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
							ps.savestr = str;
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
					if (!(ps.max--))
						continue;

					str = ps.savestr;
					op = ps.pos;

					if (ignorecase)
					{
						if (str<strend && TOLOWER(*str)==op->symbol)
						{
							str++;
							ps.savestr = str;
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
							ps.savestr = str;
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
					if (!(ps.max--))
						continue;

					str = ps.savestr;
					op = ps.pos;

					if (ignorecase)
					{
						if (str<strend && TOLOWER(*str)!=op->symbol)
						{
							str++;
							ps.savestr = str;
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
							ps.savestr = str;
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
					if (!(ps.max--))
						continue;

					str = ps.savestr;
					op = ps.pos;

					if (str<strend && op->range.symbolclass->GetBit(*str))
					{
						str++;
						ps.savestr = str;
					}
					else
					{
						continue;
					}

					break;
				}
				case opTypeMinRange:
				{
					if (!(ps.max--))
						continue;

					str = ps.savestr;
					op = ps.pos;

					if (str<strend && isType(*str,op->range.type))
					{
						str++;
						ps.savestr = str;
					}
					else
					{
						continue;
					}

					break;
				}
				case opNotTypeMinRange:
				{
					if (!(ps.max--))
						continue;

					str = ps.savestr;
					op = ps.pos;

					if (str<strend && !isType(*str,op->range.type))
					{
						str++;
						ps.savestr=str;
					}
					else
					{
						continue;
					}

					break;
				}
				case opBackRefMinRange:
				{
					if (!(ps.max--))
						continue;

					str = ps.savestr;
					op = ps.pos;
					m = &match[op->range.refindex];

					if (str+m->end-m->start<strend && StrCmp(str,start+m->start,start+m->end))
					{
						ps.savestr = str;
					}
					else
					{
						continue;
					}

					break;
				}
				case opBracketRange:
				{
					if (ps.min)
					{
						inrangebracket--;
						continue;
					}

					if (ps.forward)
					{
						ps.forward = 0;
						op = ps.pos->range.bracket.pairindex;
						inrangebracket--;
						str = ps.savestr;

						if (op->range.nextalt)
						{
							StateStackItem st;
							st.op=opAlternative;
							st.pos=op->range.bracket.nextalt;
							st.savestr=str;
							stack.emplace_back(st);
						}

						if (op->bracket.index >= 0 && static_cast<size_t>(op->bracket.index) < match.size() && inrangebracket < 0)
						{
							match[op->bracket.index].start = -1;
							match[op->bracket.index].end = -1;
						}

						break;
					}

					continue;
				}
				case opBracketMinRange:
				{
					if (!(ps.max--))
					{
						inrangebracket--;
						continue;
					}

					if (ps.forward)
					{
						ps.forward = 0;
						op = ps.pos;
						str = ps.savestr;

						if (op->range.bracket.index >= 0)
						{
							match[op->range.bracket.index].start = str - start;
							StateStackItem st;
							st.op=opOpenBracket;
							st.pos=op;
							st.min = match[op->range.bracket.index].start;
							st.max = match[op->range.bracket.index].end;
							stack.emplace_back(st);
						}

						if (op->range.nextalt)
						{
							StateStackItem st;
							st.op=opAlternative;
							st.pos=op->range.bracket.nextalt;
							st.savestr=str;
							stack.emplace_back(st);
						}

						inrangebracket++;
						break;
					}

					inrangebracket--;
					continue;
				}
				case opOpenBracket:
				{
					j = ps.pos->bracket.index;

					if (j >= 0)
					{
						match[j].start = ps.min;
						match[j].end = ps.max;
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
					op = ps.pos->assert.pairindex;
					str = ps.savestr;

					if (ps.forward)
					{
						stack.pop_back();
						break;
					}
					else
					{
						continue;
					}
				}
				case opNoReturn:
				{
					return false;
				}
			}//switch(op)

			break;
		}
	}

	return true;
}

bool RegExp::Match(string_view const text, regex_match& match) const
{
	return MatchEx(text, 0, match);
}

bool RegExp::MatchEx(string_view const text, size_t const From, regex_match& match) const
{
	// Logic errors, no need to catch them
	if (code.empty())
		throw regex_exception(errNotCompiled, 0);

	const auto start = text.data();
	const auto textstart = text.data() + From;
	const auto textend = text.data() + text.size();


	if (textstart != textend && havefirst && !first[*textstart])
		return false;

	auto tempend = textend;
	TrimTail(start, tempend);

	if (tempend<textstart)
		return false;

	// Empty needle & non-empty haystack
	// Maybe it can be done in a more elegant way, but this is fine too
	if (tempend > start && code.size() == MinCodeLength)
		return false;

	if (minlength && tempend-start<minlength)
		return false;

	state_stack stack;

	if (!InnerMatch(start, textstart, tempend, match, stack))
		return false;

	for (auto& i: match.Matches)
	{
		if (i.start == -1 || i.end == -1 || i.start > i.end)
			i.start = i.end = -1;
	}

	return true;
}

bool RegExp::Optimize()
{
	REOpCode* jumps[MAXDEPTH];
	int jumpcount=0;

	if (havefirst)
		return true;

	first.Reset();

	minlength=0;
	int mlstackmin[MAXDEPTH]{};
	int mlstacksave[MAXDEPTH]{};
	int mlscnt=0;

	for (const auto* it = code.data(), *end = it + code.size(); it != end; ++it)
	{
		switch (it->op)
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
				minlength+=it->range.min;
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
				if (it->bracket.pairindex->op>opAssertionsBegin &&
					it->bracket.pairindex->op<opAsserionsEnd)
				{
					continue;
				}

				if (mlstackmin[mlscnt-1]!=-1 && mlstackmin[mlscnt-1]<minlength)
				{
					minlength=mlstackmin[mlscnt-1];
				}

				switch (it->bracket.pairindex->op)
				{
					case opBracketRange:
					case opBracketMinRange:
						minlength *= it->range.min;
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
				it = it->assert.pairindex;
				continue;
			}
			case opRegExpEnd:
				it = nullptr;
				break;
		}

		if (!it)break;
	}

	dpf((L"minlength=%d\n",minlength));

	for (const auto* op = code.data(), *end = op + code.size(); op != end; ++op)
	{
		switch (op->op)
		{
			default:
			{
				return false;
			}
			case opType:
			{
				for (int i = 0; i < RE_CHAR_COUNT; i++)
					if (isType(i, op->type))
						first[i] = true;

				break;
			}
			case opNotType:
			{
				for (int i = 0; i < RE_CHAR_COUNT; i++)
					if (!isType(i, op->type))
						first[i] = true;

				break;
			}
			case opSymbol:
			{
				first[op->symbol] = true;
				break;
			}
			case opSymbolIgnoreCase:
			{
				first[op->symbol] = true;
				first[TOUPPER(op->symbol)] = true;
				break;
			}
			case opSymbolClass:
			{
				for (int i = 0; i < RE_CHAR_COUNT; i++)
					if (op->symbolclass->GetBit(i))
						first[i] = true;

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
				return false;
			}
			case opSymbolRange:
			case opSymbolMinRange:
			{
				if (ignorecase)
				{
					first[TOLOWER(op->range.symbol)] = true;
					first[TOUPPER(op->range.symbol)] = true;
				}
				else
				{
					first[op->range.symbol] = true;
				}

				if (!op->range.min)continue;

				break;
			}
			case opTypeRange:
			case opTypeMinRange:
			{
				for (int i=0; i<RE_CHAR_COUNT; i++)
					if (isType(i,op->range.type))
						first[i] = true;

				if (!op->range.min)continue;

				break;
			}
			case opNotTypeRange:
			case opNotTypeMinRange:
			{
				for (int i=0; i<RE_CHAR_COUNT; i++)
					if (!isType(i, op->range.type))
						first[i] = true;

				if (!op->range.min)continue;

				break;
			}
			case opClassRange:
			case opClassMinRange:
			{
				for (int i=0; i<RE_CHAR_COUNT; i++)
					if (op->range.symbolclass->GetBit(i))
						first[i] = true;

				if (!op->range.min)continue;

				break;
			}
			case opBracketRange:
			case opBracketMinRange:
			{
				if (!op->range.min)
					return false;

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
			case opRegExpEnd:
				return false;
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
	return true;
}

bool RegExp::Search(string_view const text, regex_match& match) const
{
	return SearchEx(text, 0, match);
}

bool RegExp::SearchEx(string_view const text, size_t const From, regex_match& match) const
{
	// Logic errors, no need to catch them
	if (code.empty())
		throw regex_exception(errNotCompiled, 0);

	const auto start = text.data();
	const auto textstart = text.data() + From;
	const auto textend = text.data() + text.size();

	auto tempend = textend;
	TrimTail(start, tempend);

	if (tempend<textstart)
		return false;

	if (minlength && tempend-start<minlength)
		return false;

	state_stack stack;

	auto str = textstart;

	if (!code[0].bracket.nextalt && code[1].op == opDataStart)
	{
		if (!InnerMatch(start, str, tempend, match, stack))
			return false;
	}
	else
	{
		if (!code[0].bracket.nextalt && code[1].op == opDataEnd && code[2].op == opClosingBracket)
		{
			match.Matches.resize(1);
			match.Matches[0].start = textend - start;
			match.Matches[0].end = match.Matches[0].start;
			return true;
		}

		if (havefirst)
		{
			bool res = false;
			do
			{
				while (!first[*str] && str<tempend)str++;

				if (InnerMatch(start, str, tempend, match, stack))
				{
					res = true;
					break;
				}

				str++;
			}
			while (str<tempend);

			if (!res && !InnerMatch(start, str, tempend, match, stack))
				return false;
		}
		else
		{
			bool res = false;
			do
			{
				if (InnerMatch(start, str, tempend, match, stack))
				{
					res = true;
					break;
				}

				str++;
			}
			while (str<=tempend);
			if (!res)
				return false;
		}
	}

	for (auto& i: match.Matches)
	{
		if (i.start == -1 || i.end == -1 || i.start > i.end)
			i.start = i.end = -1;
	}

	return true;
}

bool RegExp::Search(string_view const Str) const
{
	regex_match Match;
	Match.Matches.resize(1);
	return Search(Str, Match);
}

void RegExp::TrimTail(const wchar_t* const start, const wchar_t*& strend) const
{
	if (start == strend)
		return;

	if (havelookahead)return;

	if (code.empty() || code[0].bracket.nextalt)return;

	REOpCode* op = code[0].bracket.pairindex - 1;

	while (op->op==opClosingBracket)
	{
		if (op->bracket.pairindex->op!=opOpenBracket)return;

		if (op->bracket.pairindex->bracket.nextalt)return;

		--op;
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

#ifdef ENABLE_TESTS

#include "testing.hpp"

static std::ostream& operator<<(std::ostream& Stream, RegExpMatch const& m)
{
	Stream << '{' << m.start << ", "sv << m.end << '}';
	return Stream;
}

static bool operator==(RegExpMatch const& a, RegExpMatch const& b)
{
	return a.start == b.start && a.end == b.end;
}

static bool operator==(std::span<RegExpMatch const> const a, std::span<RegExpMatch const> const b)
{
	return std::ranges::equal(a, b);
}

TEST_CASE("regex.corner.empty_needle")
{
	static const struct tests
	{
		string_view Haystack;
		std::initializer_list<RegExpMatch> Match;
	}
	Tests[]
	{
		{ {},        {{ 0, 0 }}, },
		{ L"1"sv,    {}, },
	};

	RegExp re;
	regex_match Match;

	for (const auto Flag: { OP_NONE, OP_OPTIMIZE })
	{
		REQUIRE_NOTHROW(re.Compile({}, Flag));

		for (const auto& i: Tests)
		{
			const auto MatchExpected = i.Match.size() != 0;

			REQUIRE(re.Match(i.Haystack, Match) == MatchExpected);
			if (MatchExpected)
				REQUIRE(Match.Matches == i.Match);
		}
	}
}

TEST_CASE("regex.corner.empty_haystack")
{
	static const struct tests
	{
		string_view Needle;
		std::initializer_list<RegExpMatch> Match;
	}
	Tests[]
	{
		{ {},        {{ 0, 0 }}, },
		{ L"^$"sv,   {{ 0, 0 }}, },
		{ L"^"sv,    {{ 0, 0 }}, },
		{ L"$"sv,    {{ 0, 0 }}, },
		{ L"1"sv,    {}, },
		{ L"[a]"sv,  {}, },
		{ L"[^a]"sv, {}, },
		{ L"\\d"sv,  {}, },
		{ L"\\w"sv,  {}, },
	};

	RegExp re;
	regex_match Match;

	for (const auto& i: Tests)
	{
		const auto MatchExpected = i.Match.size() != 0;

		for (const auto Flag: { OP_NONE, OP_OPTIMIZE })
		{
			REQUIRE_NOTHROW(re.Compile(i.Needle, Flag));
			REQUIRE(re.Match({}, Match) == MatchExpected);

			if (MatchExpected)
				REQUIRE(Match.Matches == i.Match);
		}
	}
}

TEST_CASE("regex.list.special")
{
	static const struct tests
	{
		string_view Regex, BadMatch, GoodMatch;
		std::initializer_list<RegExpMatch> Match;
	}
	Tests[]
	{
		{ L"[]]",     L"!"sv, L"]"sv, { { 0, 1 } } },
		{ L"[^]]"sv,  L"]"sv, L"!"sv, { { 0, 1 } } },
	};

	RegExp re;
	regex_match Match;

	for (const auto& i: Tests)
	{
		REQUIRE_NOTHROW(re.Compile(i.Regex));
		REQUIRE(!re.Match(i.BadMatch, Match));
		REQUIRE(re.Match(i.GoodMatch, Match));
		REQUIRE(Match.Matches == i.Match);
	}
}

TEST_CASE("regex.exceptions")
{
	static const struct tests
	{
		string_view Regex;
		int Error;
		int Flags;
	}
	CompileTests[]
	{
		{ L"\\"sv,               errSyntax },
		{ L"("sv,                errBrackets },
		{ L")"sv,                errBrackets },
		{ L"["sv,                errBrackets },
		{ L"[]"sv,               errBrackets },
		{ L"(?{q}a)(?{q}b)"sv,   errSubpatternGroupNameMustBeUnique },
		{ L"/./q"sv,             errOptions, OP_PERLSTYLE },
		{ L"\\1"sv,              errInvalidBackRef },
		{ L"\\j"sv,              errInvalidEscape, OP_STRICT },
		{ L"{1,0}."sv,           errInvalidRange },
		{ L"{1,2}^"sv,           errInvalidQuantifiersCombination },
		{ L"\\p{q}"sv,           errReferenceToUndefinedNamedBracket },
		{ L"(?<=.+)"sv,          errVariableLengthLookBehind },
		{ L"(?{}.)"sv,           errIncompleteGroupStructure },
	};

	const auto Matcher = [](int const Code)
	{
		return generic_exception_matcher([=](std::any const& e)
		{
			auto ee = std::any_cast<regex_exception const&>(e);
			return ee.code() == Code;
		});
	};

	RegExp re;

	for (const auto& i: CompileTests)
	{
		REQUIRE_THROWS_MATCHES(re.Compile(i.Regex, i.Flags), regex_exception, Matcher(i.Error));
	}

	REQUIRE_THROWS_MATCHES(re.Search(L"meow"sv), regex_exception, Matcher(errNotCompiled));
	REQUIRE_NOTHROW(re.Compile(L"]"sv));
}

TEST_CASE("regex.regression")
{
	static const struct tests
	{
		string_view Regex, Input;
		std::initializer_list<RegExpMatch> Match;
	}
	Tests[]
	{
		{ L"a*?ca"sv,                              L"abca"sv,       {{ 2,  4}} },
		{ L"a(.)?b"sv,                             L"ab"sv,         {{ 0,  2}, {-1, -1}} },
		{ L"a(?{lol}.)?b"sv,                       L"ab"sv,         {{ 0,  2}, {-1, -1}} },
		{ L"^\\[([\\w.]+)\\]:\\s*\\[(.*)\\]$"sv,   L"[i]: [r]"sv,   {{ 0,  8}, { 1,  2}, { 6,  7}} },
		{ L"([bc]+)|(zz)"sv,                       L"abc"sv,        {{ 1,  3}, { 1,  3}, {-1, -1}} },
		{ L"(?:abc)"sv,                            L"abc"sv,        {{ 0,  3}} },
		{ L"a(?!b)d"sv,                            L"ad"sv,         {{ 0,  2}} },
		// https://bugs.farmanager.com/view.php?id=1388
		{ L"(\\d+)A|(\\d+)"sv,                     L"123"sv,        {{ 0,  3}, {-1, -1}, { 0,  3}} },
		// https://github.com/FarGroup/FarManager/issues/609
		{ L"(8)+"sv,                               L"88"sv,         {{ 0,  2}, { 1, 2 }} },
		// https://bugs.farmanager.com/view.php?id=3336
		{ L"\\{(.)?\\}"sv,                         L"{}"sv,         {{ 0,  2}, {-1, -1}} },
		{ L"(b)?b(b)?(b)?b"sv,                     L"bbb"sv,        {{ 0,  3}, { 0, 1 }, {-1, -1}, {-1, -1}} },
		{ L"([bc]+)"sv,                            L"abc"sv,        {{ 1,  3}, { 1, 3 }} },
		{ L"([bc]+)|(zz)"sv,                       L"abc"sv,        {{ 1,  3}, { 1, 3 }, {-1, -1}} },
	};

	RegExp re;
	regex_match Match;

	for (const auto& i: Tests)
	{
		const auto MatchExpected = i.Match.size() != 0;

		for (const auto Flag: { OP_NONE, OP_OPTIMIZE })
		{
			REQUIRE_NOTHROW(re.Compile(i.Regex, Flag));
			REQUIRE(re.Search(i.Input, Match) == MatchExpected);

			if (MatchExpected)
				REQUIRE(Match.Matches == i.Match);
		}
	}
}

TEST_CASE("regex.named_groups")
{
	static const struct tests
	{
		string_view Regex, Input;
		std::initializer_list<RegExpMatch> Match;
		std::initializer_list<std::pair<string_view, size_t>> NamedGroups;
	}
	Tests[]
	{
		{ L"(?{g1}a)(b)(?{g3}c)"sv,        L"abc"sv,       {{0, 3}, {0, 1}, {1, 2},  {2, 3}}, {{L"g1"sv, 1}, { L"g3"sv, 3 } }},
		{ L"(?{q}['\"])hello\\1"sv,        L"'hello'"sv,   {{0, 7}, {0, 1}},                  {{L"q"sv, 1}}},
		{ L"(?{q}['\"])hello\\p{q}"sv,     L"\"hello\""sv, {{0, 7}, {0, 1}},                  {{L"q"sv, 1}}},
		{ L"(?{q}['\"])hello\\p{q}"sv,     L"\"hello'"sv, },
	};

	RegExp re;
	regex_match Match;

	for (const auto& i: Tests)
	{
		const auto MatchExpected = i.Match.size() != 0;

		for (const auto Flag: { OP_NONE, OP_OPTIMIZE })
		{
			REQUIRE_NOTHROW(re.Compile(i.Regex, Flag));
			REQUIRE(re.Search(i.Input, Match) == MatchExpected);

			if (!MatchExpected)
				continue;

			REQUIRE(Match.Matches == i.Match);

			const auto& NamedGroups = re.GetNamedGroups();
			REQUIRE(NamedGroups.size() == i.NamedGroups.size());

			for (const auto& [k, v]: i.NamedGroups)
			{
				const auto It = NamedGroups.find(k);
				REQUIRE(It != NamedGroups.cend());
				REQUIRE(It->second == v);
			}
		}
	}
}

TEST_CASE("regex.ex")
{
	RegExp re;
	REQUIRE_NOTHROW(re.Compile(L"."sv));

	const auto Str = L"abc"sv;
	for (const auto i: std::views::iota(uint8_t{}, Str.size()))
	{
		regex_match Match;
		re.MatchEx(Str, i, Match);
		REQUIRE(Match.Matches[0].start == i);
		REQUIRE(Match.Matches[0].end == i + 1);
	}
}

TEST_CASE("regex.multiline")
{
	RegExp reBegin, reEnd;
	REQUIRE_NOTHROW(reBegin.Compile(L"^"sv, OP_MULTILINE));
	REQUIRE_NOTHROW(reEnd.Compile(L"$"sv, OP_MULTILINE));

	// We expect it to work correctly with all the EOLs we support - \r, \n, \r\n, \r\r\n

	const auto Str = L"1\r2\n3\r\n\n4\r\r\r\n5\r\r67\n8"sv;
	//                 ^  ^  ^     ^^   ^    ^   ^^   ^
	//                   $  $  $   $  $ $      $ $   $ $
	//                 0 00 00 0 0 00 0 1 1 11 1 111 112
	//                 0 12 34 5 6 78 9 0 1 23 4 567 890

	struct line
	{
		int begin;
		int end;
	}
	const ExpectedLines[]
	{
		{  0,  1 },
		{  2,  3 },
		{  4,  5 },
		{  7,  7 },
		{  8,  9 },
		{ 10, 10 },
		{ 13, 14 },
		{ 15, 15 },
		{ 16, 18 },
		{ 19, 20 },
	};

	regex_match Match;

	for (size_t i = 0; i != Str.size(); ++i)
	{
		const auto Line = *std::ranges::find_if(ExpectedLines, [&](line const& p) { return p.begin >= static_cast<int>(i); });

		REQUIRE(reBegin.SearchEx(Str, i, Match));
		REQUIRE(Match.Matches.size() == 1uz);
		REQUIRE(Match.Matches[0].start == Line.begin);
		REQUIRE(Match.Matches[0].end == Line.begin);

		REQUIRE(reEnd.SearchEx(Str, Line.begin, Match));
		REQUIRE(Match.Matches.size() == 1uz);
		REQUIRE(Match.Matches[0].start == Line.end);
		REQUIRE(Match.Matches[0].end == Line.end);
	}
}
#endif
