/*
preservestyle.cpp

*/
/*
Copyright © 2013 Far Group
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
#include "preservestyle.hpp"

// Internal:
#include "string_utils.hpp"

// Platform:

// Common:
#include "common/string_utils.hpp"
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

enum PreserveStyleType
{
	UNKNOWN = -1,

	UPPERCASE_ALL,
	LOWERCASE_ALL,
	UPPERCASE_FIRST,

	COUNT
};

static bool IsPreserveStyleTokenSeparator(wchar_t C)
{
	return contains(L"_-."sv, C);
}

static int GetPreserveCaseStyleMask(const string_view Str)
{
	std::bitset<PreserveStyleType::COUNT> Result;
	Result.set();

	for (const auto& i: Str)
	{
		const auto Upper = is_upper(i);
		const auto Lower = !Upper && is_lower(i);

		if (!Upper)
		{
			Result[UPPERCASE_ALL] = false;

			if (&i == &Str.front())
				Result[UPPERCASE_FIRST] = false;
		}

		if (!Lower)
		{
			Result[LOWERCASE_ALL] = false;

			if (&i != &Str.front())
				Result[UPPERCASE_FIRST] = false;
		}
	}

	return Result.to_ulong();
}

struct PreserveStyleToken
{
	PreserveStyleToken() = default;

	explicit PreserveStyleToken(string_view const Str, wchar_t const Char):
		Token(Str),
		PrependChar(Char),
		TypeMask(GetPreserveCaseStyleMask(Token))
	{
	}

	string Token;
	wchar_t PrependChar{};
	int TypeMask{};
};

static auto PreserveStyleTokenize(string_view const Str)
{
	std::vector<PreserveStyleToken> Result;
	std::vector<bool> Seps(Str.size(), false);

	for (size_t i = 0, Size = Str.size(); i + 2 < Size; ++i)
	{
		if (
			!IsPreserveStyleTokenSeparator(Str[i]) &&
			IsPreserveStyleTokenSeparator(Str[i + 1]) &&
			!IsPreserveStyleTokenSeparator(Str[i + 2])
		)
		{
			Seps[i + 1] = true;
			++i;
		}
	}

	size_t L = 0;

	const auto prepend_char = [&]
	{
		return L >= 1 && Seps[L - 1]? Str[L - 1] : 0;
	};

	for (size_t i = 0, Size = Str.size(); i + 2 < Size; ++i)
	{
		if (Seps[i + 1])
		{
			Result.emplace_back(Str.substr(L, i + 1 - L), prepend_char());
			L = i + 2;
			++i;
			continue;
		}

		if (!Seps[i] && is_lower(Str[i]) && is_upper(Str[i + 1]))
		{
			Result.emplace_back(Str.substr(L, i + 1 - L), prepend_char());
			L = i + 1;
		}
	}

	if (L < Str.size())
	{
		Result.emplace_back(Str.substr(L), prepend_char());
	}

	if (Result.size() > 1)
	{
		const auto PrependChar = Result[1].PrependChar;
		for (const auto& i: std::span(Result).subspan(2))
		{
			if (PrependChar != i.PrependChar)
			{
				Result.clear();
				Result.emplace_back(Str, 0);
				return Result;
			}
		}
	}

	return Result;
}

static void ToPreserveStyleType(string& strStr, PreserveStyleType type)
{
	switch (type)
	{
	case UPPERCASE_ALL:
		inplace::upper(strStr);
		break;

	case LOWERCASE_ALL:
		inplace::lower(strStr);
		break;

	case UPPERCASE_FIRST:
		inplace::upper(strStr, 0, 1);
		inplace::lower(strStr, 1);
		break;

	default:
		break;
	}
}

static auto ChoosePreserveStyleType(unsigned Mask)
{
	if (!Mask)
		return UNKNOWN;

	for (int Style = COUNT; Style != UNKNOWN; --Style)
		if (Mask & bit(Style))
			return static_cast<PreserveStyleType>(Style);

	return UNKNOWN;
}

static void FindStyleTypeMaskAndPrependCharByExpansion(const string_view Source, const int Begin, const int End, int& TypeMask, wchar_t& PrependChar)
{
	// Try to expand to the right.
	if (int Right = End; static_cast<size_t>(Right) < Source.size() && (is_alphanumeric(Source[Right]) || IsPreserveStyleTokenSeparator(Source[Right])))
	{
		Right++;

		while (static_cast<size_t>(Right) < Source.size())
		{
			if (!is_alphanumeric(Source[Right]) || (is_lower(Source[Right-1]) && is_upper(Source[Right])))
				break;
			Right++;
		}

		const auto SegmentTokens = PreserveStyleTokenize(Source.substr(Begin, Right-Begin));

		if (SegmentTokens.size() > 1)
		{
			TypeMask = SegmentTokens.back().TypeMask;
			PrependChar = SegmentTokens.back().PrependChar;
			return;
		}
	}

	// Try to expand to the left.
	if (int Left = Begin - 1; Left >= 1 && (is_alphanumeric(Source[Left]) || IsPreserveStyleTokenSeparator(Source[Left])))
	{
		Left--;

		while (Left >= 0)
		{
			if (!is_alphanumeric(Source[Left]) || (is_lower(Source[Left]) && is_upper(Source[Left + 1])))
				break;
			Left--;
		}

		Left++;

		const auto SegmentTokens = PreserveStyleTokenize(Source.substr(Left, End-Left));

		if (SegmentTokens.size() > 1)
		{
			TypeMask = SegmentTokens.back().TypeMask;
			PrependChar = SegmentTokens.back().PrependChar;
			return;
		}
	}
}

bool PreserveStyleReplaceString(
	string_view const Source,
	string_view const Str,
	string& ReplaceStr,
	int& CurPos,
	search_replace_string_options const options,
	string_view const WordDiv,
	int& SearchLength
)
{
	int Position = CurPos;
	SearchLength = 0;

	if (options.Reverse)
	{
		Position = std::min(Position - 1, static_cast<int>(Source.size() - 1));

		if (Position<0)
			return false;
	}

	if (static_cast<size_t>(Position) >= Source.size() || Str.empty() || ReplaceStr.empty())
		return false;

	const auto StrTokens = PreserveStyleTokenize(Str);

	const auto BlankOrWordDiv = [&WordDiv](wchar_t Ch)
	{
		return std::iswblank(Ch) || contains(WordDiv, Ch);
	};

	for (int I=Position; (options.Reverse && I>=0) || (!options.Reverse && static_cast<size_t>(I) < Source.size()); options.Reverse? I-- : I++)
	{
		if (options.WholeWords && I && !BlankOrWordDiv(Source[I - 1]))
			continue;

		bool Matched = true;

		size_t Idx = I;
		size_t T = 0;

		auto j = StrTokens.cbegin();
		auto LastItem = StrTokens.cend();
		--LastItem;
		while ((j != LastItem || T < j->Token.size()) && Source[Idx])
		{
			bool Sep = (static_cast<size_t>(I) < Idx && static_cast<size_t>(I) + 1 != Source.size() && IsPreserveStyleTokenSeparator(Source[Idx])
					&& !IsPreserveStyleTokenSeparator(Source[Idx-1])
					&& !IsPreserveStyleTokenSeparator(Source[Idx+1]));

			if (Sep)
			{
				if (T == j->Token.size())
				{
					Idx++;
					T = 0;
					++j;
					continue;
				}
				else
				{
					Matched = false;
					break;
				}
			}

			Sep = (static_cast<size_t>(I) < Idx && is_lower(Source[Idx-1]) && is_upper(Source[Idx])
					&& !IsPreserveStyleTokenSeparator(Source[Idx-1])
					&& !IsPreserveStyleTokenSeparator(Source[Idx]));

			if (Sep && T != 0)
			{
				if (T == j->Token.size())
				{
					T = 0;
					++j;
					continue;
				}
				else
				{
					Matched = false;
					break;
				}
			}

			if (T >= j->Token.size())
			{
				Matched = false;
				break;
			}

			if (options.CaseSensitive && Source[Idx] != j->Token[T])
			{
				Matched = false;
				break;
			}

			if (!options.CaseSensitive && upper(Source[Idx]) != upper(j->Token[T]))
			{
				Matched = false;
				break;
			}

			T++;
			Idx++;
		}

		if (options.WholeWords && Idx < Source.size() && !BlankOrWordDiv(Source[Idx]))
			continue;

		if (!Matched || T != j->Token.size() || j != LastItem)
			continue;

		const auto SourceTokens = PreserveStyleTokenize(Source.substr(I, Idx - I));

		const auto by_token_size = [](const auto& i){ return i.Token.size(); };
		if (!std::ranges::equal(SourceTokens, StrTokens, {}, by_token_size, by_token_size))
			continue;

		auto ReplaceStrTokens = PreserveStyleTokenize(ReplaceStr);

		if (ReplaceStrTokens.size() == SourceTokens.size())
		{
			int CommonTypeMask = -1;

			for (const auto& i: SourceTokens)
			{
				if (CommonTypeMask == -1)
					CommonTypeMask = i.TypeMask;
				else
					CommonTypeMask &= i.TypeMask;
			}

			const auto CommonType = ChoosePreserveStyleType(CommonTypeMask);
			if (CommonTypeMask != -1 && CommonType == UNKNOWN)
				CommonTypeMask = -1;

			auto SourceI = SourceTokens.cbegin();

			for (auto& i: ReplaceStrTokens)
			{
				ToPreserveStyleType(i.Token, CommonTypeMask != -1 ? CommonType : ChoosePreserveStyleType(SourceI->TypeMask));
				i.PrependChar = SourceI->PrependChar;
				++SourceI;
			}
		}
		else
		{
			ToPreserveStyleType(ReplaceStrTokens.front().Token, ChoosePreserveStyleType(SourceTokens.front().TypeMask));
			ReplaceStrTokens.front().PrependChar = SourceTokens.front().PrependChar;

			if (!SourceTokens.empty())
			{
				int AfterFirstCommonTypeMask = -1;
				wchar_t PrependChar = SourceTokens.back().PrependChar;

				for (const auto& SourceI: std::span(SourceTokens).subspan(1))
				{
					if (AfterFirstCommonTypeMask == -1)
						AfterFirstCommonTypeMask = SourceI.TypeMask;
					else
						AfterFirstCommonTypeMask &= SourceI.TypeMask;
				}

				if (AfterFirstCommonTypeMask == -1)
					FindStyleTypeMaskAndPrependCharByExpansion(Source, I, static_cast<int>(Idx), AfterFirstCommonTypeMask, PrependChar);

				if (AfterFirstCommonTypeMask == -1)
				{
					AfterFirstCommonTypeMask = SourceTokens.front().TypeMask;
					PrependChar = ReplaceStrTokens.back().PrependChar;
				}

				for (auto& ReplaceI: std::span(ReplaceStrTokens).subspan(1))
				{
					ToPreserveStyleType(ReplaceI.Token, ChoosePreserveStyleType(AfterFirstCommonTypeMask));
					ReplaceI.PrependChar = PrependChar;
				}
			}

		}

		ReplaceStr.clear();

		for (const auto& i: ReplaceStrTokens)
		{
			if (i.PrependChar)
				ReplaceStr += i.PrependChar;
			ReplaceStr += i.Token;
		}

		CurPos = I;
		SearchLength = static_cast<int>(Idx-I);

		return true;
	}

	return false;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("PreserveStyleReplaceString")
{
	static const struct
	{
		string_view Find, Replace;
	}
	Patterns[]
	{
		{ {},                     {},                   },
		{ {},                     L"la"sv,              },
		{ L"na"sv,                {},                   },
		{ L"na"sv,                L"la"sv,              },
		{ L"abc-def-ghi"sv,       L"pq.RST.Xyz"sv,      },
		{ L"AA-B-C"sv,            L"xxx.Yy.ZZ"sv,       },
		{ L"abc-def-ghi"sv,       L"pq.RST.uvw.Xyz"sv,  },
		{ L"A-B-C"sv,             L"aa.Bb.cc.DD"sv,     },
		{ L"ijk"sv,               L"MnoPqrStu"sv,       },
		{ L"ab.cd"sv,             L"wx-yz"sv,           },
	};

	static const struct
	{
		size_t PatternIndex;
		string_view Src, ResultStr;
		bool Result;
		int Position, Size;
	}
	Tests[]
	{
		{ 0, {},                   {},                      false, 0, 0,  },
		{ 0, L"LaLaNaLa"sv,        {},                      false, 0, 0,  },

		{ 1, {},                   {},                      false, 0, 0,  },
		{ 1, L"LaLaNaLa"sv,        {},                      false, 0, 0,  },

		{ 2, {},                   {},                      false, 0, 0,  },
		{ 2, L"LaLaNaLa"sv,        {},                      false, 0, 0,  },

		{ 3, {},                   {},                      false, 0, 0,  },
		{ 3, L"LaLaNaLa"sv,        L"La"sv,                 true,  4, 2,  },

		{ 4, L"AbcDefGhi"sv,       L"PqRstXyz"sv,           true,  0, 9,  },
		{ 4, L"ABC_DEF_GHI"sv,     L"PQ_RST_XYZ"sv,         true,  0, 11, },
		{ 4, L"abc.def.ghi"sv,     L"pq.rst.xyz"sv,         true,  0, 11, },
		{ 4, L"abcDefGhi"sv,       L"pqRstXyz"sv,           true,  0, 9,  },
		{ 4, L"ABC_Def_Ghi"sv,     L"PQ_Rst_Xyz"sv,         true,  0, 11, },

		{ 5, L"Aa_B_C"sv,          L"Xxx_Yy_Zz"sv,          true,  0, 6,  },
		{ 5, L"aa-b-c"sv,          L"xxx-yy-zz"sv,          true,  0, 6,  },
		{ 5, L"AA_B_C"sv,          L"XXX_YY_ZZ"sv,          true,  0, 6,  },
		{ 5, L"aa.B.C"sv,          L"xxx.Yy.Zz"sv,          true,  0, 6,  },
		{ 5, L"Aa.B.c"sv,          L"Xxx.Yy.zz"sv,          true,  0, 6,  },

		{ 6, L"Abc.def.ghi"sv,     L"Pq.rst.uvw.xyz"sv,     true,  0, 11, },
		{ 6, L"ABC.Def.Ghi"sv,     L"PQ.Rst.Uvw.Xyz"sv,     true,  0, 11, },
		{ 6, L"abc.Def.ghi"sv,     L"pq.RST.uvw.Xyz"sv,     true,  0, 11, },
		{ 6, L"ABC.DEF.ghi"sv,     L"PQ.RST.uvw.Xyz"sv,     true,  0, 11, },

		{ 7, L"A_B_C"sv,           L"Aa_Bb_Cc_Dd"sv,        true,  0, 5,  },
		{ 7, L"a-b-c"sv,           L"aa-bb-cc-dd"sv,        true,  0, 5,  },
		{ 7, L"A.B.c"sv,           L"Aa.Bb.cc.DD"sv,        true,  0, 5,  },

		{ 8, L"ijk.Zzz"sv,         L"mno.Pqr.Stu"sv,        true,  0, 3,  },
		{ 8, L"AAA-ijk"sv,         L"mno-pqr-stu"sv,        true,  4, 3,  },
		{ 8, L"aaa-ijk_ZZZ"sv,     L"mno_PQR_STU"sv,        true,  4, 3,  },
		{ 8, L"AaaIjk"sv,          L"MnoPqrStu"sv,          true,  3, 3,  },
		{ 8, L"0_ijk_9"sv,         L"mno_Pqr_Stu"sv,        true,  2, 3,  },
		{ 8, L">ijk<"sv,           L"mnopqrstu"sv,          true,  1, 3,  },

		{ 9, L"Ab.cD"sv,           {},                      false, 0, 0,  },
	};

	string ResultStr;
	for (const auto& i: Tests)
	{
		int Position = 0;
		int Size;
		ResultStr = Patterns[i.PatternIndex].Replace;
		REQUIRE(i.Result == PreserveStyleReplaceString(i.Src, Patterns[i.PatternIndex].Find, ResultStr, Position, {}, {}, Size));
		if (i.Result)
		{
			REQUIRE(i.ResultStr == ResultStr);
			REQUIRE(i.Position == Position);
			REQUIRE(i.Size == Size);
		}
	}
}
#endif
