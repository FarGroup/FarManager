﻿/*
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

// Self:
#include "preservestyle.hpp"

// Internal:
#include "string_utils.hpp"

// Platform:

// Common:
#include "common/range.hpp"
#include "common/string_utils.hpp"
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

enum PreserveStyleType
{
	UPPERCASE_ALL,
	LOWERCASE_ALL,
	UPPERCASE_FIRST,
	UNKNOWN
};

struct PreserveStyleToken
{
	string Token;
	wchar_t PrependChar{};
	int TypeMask{};
};

static bool IsPreserveStyleTokenSeparator(wchar_t C)
{
	return contains(L"_-."sv, C);
}

static int GetPeserveCaseStyleMask(const string_view strStr)
{
	int Result = 15;

	for (size_t I = 0; I < strStr.size(); I++)
	{
		const auto Upper = is_upper(strStr[I]);
		const auto Lower = is_lower(strStr[I]);
		if (!Upper)
			Result &= ~bit(UPPERCASE_ALL);
		if (!Lower)
			Result &= ~bit(LOWERCASE_ALL);
		if ((!Upper || !is_alpha(strStr[I])) && I == 0)
			Result &= ~bit(UPPERCASE_FIRST);
		if (!Lower && I > 0)
			Result &= ~bit(UPPERCASE_FIRST);
	}

	return Result;
}

static auto InternalPreserveStyleTokenize(const string_view strStr, size_t From, size_t Length)
{
	std::list<PreserveStyleToken> Result;

	std::vector<bool> Seps(Length, false);
	for (size_t I = From+1; I+1 < From+Length; I++)
	{
		if (IsPreserveStyleTokenSeparator(strStr[I])
				&& !IsPreserveStyleTokenSeparator(strStr[I-1])
				&& !IsPreserveStyleTokenSeparator(strStr[I+1]))
		{
			Seps[I-From] = true;
		}
	}

	size_t L = From;
	for (size_t I = From+1; I < From+Length; I++)
	{
		if (Seps[I-From])
		{
			PreserveStyleToken T;
			T.Token = strStr.substr(L, I - L);
			if (L >= From + 1 && Seps[L-1-From])
				T.PrependChar = strStr[L-1];
			Result.emplace_back(T);
			L = I+1;
			I++;
			continue;
		}

		if (!Seps[I-From-1] && is_lower(strStr[I-1]) && is_upper(strStr[I]))
		{
			PreserveStyleToken T;
			T.Token = strStr.substr(L, I - L);
			if (L >= From + 1 && Seps[L-1-From])
				T.PrependChar = strStr[L-1];
			Result.emplace_back(T);
			L = I;
		}
	}

	if (L < From+Length)
	{
		PreserveStyleToken T;
		T.Token = strStr.substr(L, From + Length - L);
		if (L >= From + 1 && Seps[L-1-From])
			T.PrependChar = strStr[L-1];
		Result.emplace_back(T);
	}

	if (Result.size() > 1)
	{
		const auto PrependChar = std::next(Result.cbegin())->PrependChar;
		for (const auto& i: range(std::next(Result.cbegin(), 2), Result.cend()))
		{
			if (PrependChar != i.PrependChar)
			{
				Result.clear();
				PreserveStyleToken T;
				T.Token = strStr.substr(From, Length);
				T.TypeMask = bit(UNKNOWN);
				Result.emplace_back(T);
				return Result;
			}
		}
	}

	return Result;
}

static auto PreserveStyleTokenize(const string_view strStr, size_t From, size_t Length)
{
	auto Tokens = InternalPreserveStyleTokenize(strStr, From, Length);

	for (auto& i: Tokens)
	{
		i.TypeMask = GetPeserveCaseStyleMask(i.Token);
	}

	return Tokens;
}

static void ToPreserveStyleType(string& strStr, PreserveStyleType type)
{
	switch (type)
	{
	case UPPERCASE_ALL:     return inplace::upper(strStr);
	case LOWERCASE_ALL:     return inplace::lower(strStr);
	case UPPERCASE_FIRST:   return (void)inplace::upper(strStr, 0, 1), inplace::lower(strStr, 1);
	case UNKNOWN:           return;
	}
}

static PreserveStyleType ChoosePreserveStyleType(unsigned Mask)
{
	if (Mask == bit(UNKNOWN))
		return UNKNOWN;

	PreserveStyleType Result = UNKNOWN;
	for (int Style = 0; UPPERCASE_ALL+Style < UNKNOWN; Style++)
		if (Mask & bit(Style))
			Result = PreserveStyleType(UPPERCASE_ALL+Style);

	return Result;
}

static void FindStyleTypeMaskAndPrependCharByExpansion(const string_view Source, const int Begin, const int End, int& TypeMask, wchar_t& PrependChar)
{
	// Try to expand to the right.
	{
		int Right = End;

		if (static_cast<size_t>(Right) < Source.size() && (is_alphanumeric(Source[Right]) || IsPreserveStyleTokenSeparator(Source[Right])))
		{
			Right++;

			while (static_cast<size_t>(Right) < Source.size())
			{
				if (!is_alphanumeric(Source[Right]) || (is_lower(Source[Right-1]) && is_upper(Source[Right])))
					break;
				Right++;
			}

			const auto SegmentTokens = PreserveStyleTokenize(Source, Begin, Right-Begin);

			if (SegmentTokens.size() > 1)
			{
				TypeMask = SegmentTokens.back().TypeMask;
				PrependChar = SegmentTokens.back().PrependChar;
				return;
			}
		}
	}

	// Try to expand to the left.
	{
		int Left = Begin - 1;

		if (Left >= 1 && (is_alphanumeric(Source[Left]) || IsPreserveStyleTokenSeparator(Source[Left])))
		{
			Left--;

			while (Left >= 0)
			{
				if (!is_alphanumeric(Source[Left]) || (is_lower(Source[Left]) && is_upper(Source[Left + 1])))
					break;
				Left--;
			}

			Left++;

			const auto SegmentTokens = PreserveStyleTokenize(Source, Left, End-Left);

			if (SegmentTokens.size() > 1)
			{
				TypeMask = SegmentTokens.back().TypeMask;
				PrependChar = SegmentTokens.back().PrependChar;
				return;
			}
		}
	}
}

bool PreserveStyleReplaceString(
	string_view const Source,
	string_view const Str,
	string& ReplaceStr,
	int& CurPos,
	bool Case,
	bool WholeWords,
	string_view const WordDiv,
	bool Reverse,
	int& SearchLength
)
{
	int Position = CurPos;
	SearchLength = 0;

	if (Reverse)
	{
		Position--;

		Position = std::min(Position, static_cast<int>(Source.size() - 1));

		if (Position<0)
			return false;
	}

	if (static_cast<size_t>(Position) >= Source.size() || Str.empty() || ReplaceStr.empty())
		return false;


	const auto StrTokens = PreserveStyleTokenize(Str, 0, Str.size());

	const auto BlankOrWordDiv = [&WordDiv](wchar_t Ch)
	{
		return std::iswblank(Ch) || contains(WordDiv, Ch);
	};

	for (int I=Position; (Reverse && I>=0) || (!Reverse && static_cast<size_t>(I) < Source.size()); Reverse? I-- : I++)
	{
		if (WholeWords && I && !BlankOrWordDiv(Source[I - 1]))
			continue;

		bool Matched = true;

		size_t Idx = I;
		size_t T = 0;

		auto j = StrTokens.cbegin();
		auto LastItem = StrTokens.cend();
		--LastItem;
		while (((j != LastItem) || (j == LastItem && T < j->Token.size()))
			&& Source[Idx])
		{
			bool Sep = (static_cast<size_t>(I) < Idx && static_cast<size_t>(I + 1) != Source.size() && IsPreserveStyleTokenSeparator(Source[Idx])
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

			if (Case && Source[Idx] != j->Token[T])
			{
				Matched = false;
				break;
			}

			if (!Case && upper(Source[Idx]) != upper(j->Token[T]))
			{
				Matched = false;
				break;
			}

			T++;
			Idx++;
		}

		if (WholeWords && Idx < Source.size() && !BlankOrWordDiv(Source[Idx]))
			continue;

		if (Matched && T == j->Token.size() && j == LastItem)
		{
			const auto SourceTokens = PreserveStyleTokenize(Source, I, Idx - I);

			if (std::equal(ALL_CONST_RANGE(SourceTokens), ALL_CONST_RANGE(StrTokens), [](const auto& a, const auto& b)
			{
				return a.Token.size() == b.Token.size();
			}))
			{
				auto ReplaceStrTokens = PreserveStyleTokenize(ReplaceStr, 0, ReplaceStr.size());

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

						for (const auto& SourceI: range(std::next(SourceTokens.cbegin()), SourceTokens.cend()))
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

						for (auto& ReplaceI: range(std::next(ReplaceStrTokens.begin()), ReplaceStrTokens.end()))
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
		}
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
		REQUIRE(i.Result == PreserveStyleReplaceString(i.Src, Patterns[i.PatternIndex].Find, ResultStr, Position, false, false, {}, false, Size));
		if (i.Result)
		{
			REQUIRE(i.ResultStr == ResultStr);
			REQUIRE(i.Position == Position);
			REQUIRE(i.Size == Size);
		}
	}
}
#endif
