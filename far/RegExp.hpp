#ifndef REGEXP_HPP_18B41BD7_69F8_461A_8A81_069B447D5554
#define REGEXP_HPP_18B41BD7_69F8_461A_8A81_069B447D5554
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

// Internal:
#include "plugin.hpp"
#include "exception.hpp"

// Platform:

// Common:
#include "common/string_utils.hpp"

// External:
#include "stack_allocator.hpp"

//----------------------------------------------------------------------------

//#define RE_DEBUG

//! Possible compile and runtime errors
enum REError
{
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
	//! Incomplete group structure
	errIncompleteGroupStructure,
	//! A subpattern name must be unique
	errSubpatternGroupNameMustBeUnique,
	//! Reference to undefined named bracket
	errReferenceToUndefinedNamedBracket,
	//! Only fixed length look behind assertions are supported
	errVariableLengthLookBehind,
};

enum
{
	OP_NONE         = 0,
	//! Match in a case insensitive manner
	OP_IGNORECASE   = 0x0001,
	//! Single line mode, dot meta-character will match newline symbol
	OP_SINGLELINE   = 0x0002,
	//! MultiLine mode, ^ and $ can match line start and line end
	OP_MULTILINE    = 0x0004,
	//! Extended syntax, spaces symbols are ignored unless escaped
	OP_XTENDEDSYNTAX= 0x0008,
	//! Perl style RegExp provided. i.e. /expression/imsx
	OP_PERLSTYLE    = 0x0010,
	//! Optimize after compile
	OP_OPTIMIZE     = 0x0020,
	//! Strict escapes - only unrecognized escape will produce errInvalidEscape error
	OP_STRICT       = 0x0040,
};

class regex_match
{
public:
	using matches = std::vector<RegExpMatch, stack_allocator<RegExpMatch, 4096>>;

private:
	matches::allocator_type::arena_type m_Arena;

public:
	matches Matches{ m_Arena };
};

class named_regex_match
{
public:
	using matches = std::unordered_map<
		string,
		size_t,
		string_comparer,
		string_comparer,
		stack_allocator<std::pair<string const, size_t>, 4096>
	>;

private:
	matches::allocator_type::arena_type m_Arena;

public:
	matches Matches{ m_Arena };
};

class regex_exception: public far_exception
{
public:
	explicit regex_exception(REError const Code, size_t const Position, source_location const& Location = source_location::current()):
		far_exception(to_string(Code), false, Location),
		m_Code(Code),
		m_Position(Position)
	{}

	REError code() const
	{
		return m_Code;
	}

	size_t position() const
	{
		return m_Position;
	}

private:
	static string_view to_string(REError Code);

	REError m_Code;
	size_t m_Position;
};


/*! Regular expressions support class.

Expressions must be Compile'ed first,
and than Match string or Search for matching fragment.
*/
class RegExp
{
public:
	struct REOpCode;
	struct UniSet;
	struct StateStackItem;
	class state_stack;

private:


		// code
		std::vector<REOpCode> code;

		std::unique_ptr<UniSet> firstptr;
		UniSet& first;

		int havefirst{};
		int havelookahead{};

		int minlength{};

		// options
		int ignorecase{};

		int bracketscount{};
		int maxbackref{};
#ifdef RE_DEBUG
		string resrc;
#endif

		int CalcLength(string_view src);
		void InnerCompile(const wchar_t* start, const wchar_t* src, int srclength, int options);

		bool InnerMatch(const wchar_t* start, const wchar_t* str, const wchar_t* strend, regex_match& RegexMatch, named_regex_match& NamedMatch, state_stack& Statetack) const;

		void TrimTail(const wchar_t* start, const wchar_t*& strend) const;

		int StrCmp(const wchar_t*& str,const wchar_t* start,const wchar_t* end) const;

	public:
		//! Default constructor.
		RegExp();
		~RegExp();

		RegExp(RegExp&&) noexcept;
		RegExp& operator=(RegExp&&) = delete;

		/*! Compile regular expression
		    Generate internal op-codes of expression.

		    \param src - source of expression
		    \param options - compile options

		    If compilation fails an exception is thrown, error and position can be obtained from it.
		    See error codes in REError enumeration.
		    \sa LastError
		    \sa REError
		    \sa ErrorPosition
		    \sa options
		*/
		void Compile(string_view src, int options = OP_NONE);

		/*! Try to optimize regular expression
		    Significantly speedup Search mode in some cases.
		*/
		bool Optimize();

		/*! Try to match string with regular expression
		    \param text - string to match
		    \param match - array of SMatch structures that receive brackets positions.
		    \param NamedMatch - storage of named brackets.
		    \sa SMatch
		*/
		bool Match(string_view text, regex_match& match, named_regex_match* NamedMatch = {}) const;
		/*! Advanced version of match. Can be used for multiple matches
		    on one string (to imitate /g modifier of perl regexp
		*/
		bool MatchEx(string_view text, size_t From, regex_match& match, named_regex_match* NamedMatch = {}) const;
		/*! Try to find substring that will match regexp.
		    Parameters and return value are the same as for Match.
		    It is highly recommended to call Optimize before Search.
		*/
		bool Search(string_view text, regex_match& match, named_regex_match* NamedMatch = {}) const;
		/*! Advanced version of search. Can be used for multiple searches
		    on one string (to imitate /g modifier of perl regexp
		*/
		bool SearchEx(string_view text, size_t From, regex_match& match, named_regex_match* NamedMatch = {}) const;

		bool Search(string_view Str) const;

		int GetBracketsCount() const {return bracketscount;}
};

constexpr string_view get_match(string_view const Str, RegExpMatch const& Match)
{
	if (Match.start < 0)
		return Str.substr(Str.size());

	assert(Match.end >= Match.start);
	return Str.substr(Match.start, Match.end - Match.start);
}

#endif // REGEXP_HPP_18B41BD7_69F8_461A_8A81_069B447D5554
