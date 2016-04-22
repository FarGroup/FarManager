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

#include "headers.hpp"
#pragma hdrstop

#include "preservestyle.hpp"

static const wchar_t PreserveStyleTokenSeparators[] = L"_-.";

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
	wchar_t PrependChar;
	int TypeMask;
};

static inline bool IsPreserveStyleTokenSeparator(wchar_t C)
{
	return wcschr(PreserveStyleTokenSeparators, C) != nullptr;
}

static int GetPeserveCaseStyleMask(const string& strStr)
{
	int Result = 15;

	for (size_t I = 0; I < strStr.size(); I++)
	{
		int Upper = IsUpper(strStr[I]);
		int Lower = IsLower(strStr[I]);
		if (!Upper)
			Result &= ~(1 << UPPERCASE_ALL);
		if (!Lower)
			Result &= ~(1 << LOWERCASE_ALL);
		if ((!Upper || !IsAlpha(strStr[I])) && I == 0)
			Result &= ~(1 << UPPERCASE_FIRST);
		if (!Lower && I > 0)
			Result &= ~(1 << UPPERCASE_FIRST);
	}

	return Result;
}

static auto InternalPreserveStyleTokenize(const string& strStr, size_t From, size_t Length)
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
			T.Token = strStr.substr(L, I-L);
			T.PrependChar = 0;
			if (L >= From + 1 && Seps[L-1-From])
				T.PrependChar = strStr[L-1];
			Result.emplace_back(T);
			L = I+1;
			I++;
			continue;
		}

		if (!Seps[I-From-1] && IsLower(strStr[I-1]) && IsUpper(strStr[I]))
		{
			PreserveStyleToken T;
			T.Token = strStr.substr(L, I-L);
			T.PrependChar = 0;
			if (L >= From + 1 && Seps[L-1-From])
				T.PrependChar = strStr[L-1];
			Result.emplace_back(T);
			L = I;
		}
	}

	if (L < From+Length)
	{
		PreserveStyleToken T;
		T.Token = strStr.substr(L, From+Length-L);
		T.PrependChar = 0;
		if (L >= From + 1 && Seps[L-1-From])
			T.PrependChar = strStr[L-1];
		Result.emplace_back(T);
	}

	if (Result.size() > 1)
	{
		wchar_t PrependChar = std::next(Result.cbegin())->PrependChar;
		for (const auto& i: make_range(std::next(Result.cbegin(), 2), Result.cend()))
		{
			if (PrependChar != i.PrependChar)
			{
				Result.clear();
				PreserveStyleToken T;
				T.Token = strStr.substr(From, Length);
				T.PrependChar = 0;
				T.TypeMask = 1 << UNKNOWN;
				Result.emplace_back(T);
				return Result;
			}
		}
	}

	return Result;
}

static auto PreserveStyleTokenize(const string& strStr, size_t From, size_t Length)
{
	auto Tokens = InternalPreserveStyleTokenize(strStr, From, Length);

	std::for_each(RANGE(Tokens, i)
	{
		i.TypeMask = GetPeserveCaseStyleMask(i.Token);
	});

	return Tokens;
}

static void ToPreserveStyleType(string& strStr, PreserveStyleType type)
{
	std::function<void(size_t)> Handler;

	switch (type)
	{
	case UPPERCASE_ALL:
		Handler = [&strStr](size_t I) { strStr[I] = ToUpper(strStr[I]); };
		break;
	case LOWERCASE_ALL:
		Handler = [&strStr](size_t I) { strStr[I] = ToLower(strStr[I]); };
		break;
	case UPPERCASE_FIRST:
		Handler = [&strStr](size_t I) { strStr[I] = I? ToLower(strStr[I]) : ToUpper(strStr[I]); };
		break;
	case UNKNOWN:
		break;
	}

	if (Handler)
	{
		for_each_cnt(RANGE(strStr, i, size_t index)
		{
			Handler(index);
		});
	}
}

static PreserveStyleType ChoosePreserveStyleType(int Mask)
{
	if (Mask == (1 << UNKNOWN))
		return UNKNOWN;
	
	PreserveStyleType Result = UNKNOWN;
	for (int Style = 0; UPPERCASE_ALL+Style < UNKNOWN; Style++)
		if ((Mask & (1 << Style)) != 0)
			Result = PreserveStyleType(UPPERCASE_ALL+Style);

	return Result;
}

void FindStyleTypeMaskAndPrependCharByExpansion(const wchar_t* Source, const size_t StrSize, const int Begin, const int End, int& TypeMask, wchar_t& PrependChar)
{
	// Try to expand to the right.
	{
		int Right = End;

		if (static_cast<size_t>(Right) < StrSize && (IsAlphaNum(Source[Right]) || IsPreserveStyleTokenSeparator(Source[Right])))
		{
			Right++;
			
			while (static_cast<size_t>(Right) < StrSize)
			{
				if (!IsAlphaNum(Source[Right]) || (IsLower(Source[Right-1]) && IsUpper(Source[Right])))
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

		if (Left >= 1 && (IsAlphaNum(Source[Left]) || IsPreserveStyleTokenSeparator(Source[Left])))
		{
			Left--;

			while (Left >= 0)
			{
				if (!IsAlphaNum(Source[Left]) || (IsLower(Source[Left]) && IsUpper(Source[Left + 1])))
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

bool PreserveStyleReplaceString(const wchar_t* Source, size_t StrSize, const string& Str, string& ReplaceStr, int& CurPos, int Position, int Case, int WholeWords, const wchar_t *WordDiv, int Reverse, int& SearchLength)
{
	SearchLength = 0;

	if (Reverse)
	{
		Position--;

		Position = std::min(Position, static_cast<int>(StrSize-1));

		if (Position<0)
			return false;
	}

	if (static_cast<size_t>(Position) >= StrSize || Str.empty() || ReplaceStr.empty())
		return false;


	const auto StrTokens = PreserveStyleTokenize(Str, 0, Str.size());

	for (int I=Position; (Reverse && I>=0) || (!Reverse && static_cast<size_t>(I)<StrSize); Reverse ? I--:I++)
	{
		if (WholeWords && I && !IsSpace(Source[I-1]) && !wcschr(WordDiv, Source[I-1]))
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
			bool Sep = (static_cast<size_t>(I) < Idx && Source[I+1] && IsPreserveStyleTokenSeparator(Source[Idx])
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

			Sep = (static_cast<size_t>(I) < Idx && IsLower(Source[Idx-1]) && IsUpper(Source[Idx])
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

			if (!Case && ToUpper(Source[Idx]) != ToUpper(j->Token[T]))
			{
				Matched = false;
				break;
			}

			T++;
			Idx++;
		}

		if (WholeWords && !(Idx >= StrSize || IsSpace(Source[Idx]) || wcschr(WordDiv, Source[Idx])))
			continue;
		
		if (Matched && T == j->Token.size() && j == LastItem)
		{
			const auto SourceTokens = PreserveStyleTokenize(Source, I, Idx - I);
			
			bool Same = SourceTokens.size() == StrTokens.size();
			if(Same)
			{
				Same = std::equal(ALL_CONST_RANGE(SourceTokens), StrTokens.cbegin(), [](const auto& a, const auto& b)
				{
					return a.Token.size() == b.Token.size();
				});
			}

			if (Same)
			{
				auto ReplaceStrTokens = PreserveStyleTokenize(ReplaceStr, 0, ReplaceStr.size());

				if (ReplaceStrTokens.size() == SourceTokens.size())
				{
					int CommonTypeMask = -1;
					std::for_each(CONST_RANGE(SourceTokens, i)
					{
						if (CommonTypeMask == -1)
							CommonTypeMask = i.TypeMask;
						else
							CommonTypeMask &= i.TypeMask;
					});

					PreserveStyleType CommonType = ChoosePreserveStyleType(CommonTypeMask);
					if (CommonTypeMask != -1 && CommonType == UNKNOWN)
						CommonTypeMask = -1;

					auto SourceI = SourceTokens.cbegin();
					std::for_each(RANGE(ReplaceStrTokens, i)
					{
						ToPreserveStyleType(i.Token, CommonTypeMask != -1 ? CommonType : ChoosePreserveStyleType(SourceI->TypeMask));
						i.PrependChar = SourceI->PrependChar;
						++SourceI;
					});
				}
				else
				{
					ToPreserveStyleType(ReplaceStrTokens.front().Token, ChoosePreserveStyleType(SourceTokens.front().TypeMask));
					ReplaceStrTokens.front().PrependChar = SourceTokens.front().PrependChar;

					if (!SourceTokens.empty())
					{
						int AfterFirstCommonTypeMask = -1;
						wchar_t PrependChar = SourceTokens.back().PrependChar;

						for (const auto& SourceI: make_range(std::next(SourceTokens.cbegin()), SourceTokens.cend()))
						{
							if (AfterFirstCommonTypeMask == -1)
								AfterFirstCommonTypeMask = SourceI.TypeMask;
							else
								AfterFirstCommonTypeMask &= SourceI.TypeMask;
						}

						if (AfterFirstCommonTypeMask == -1)
							FindStyleTypeMaskAndPrependCharByExpansion(Source, StrSize, I, static_cast<int>(Idx), AfterFirstCommonTypeMask, PrependChar);

						if (AfterFirstCommonTypeMask == -1)
						{
							AfterFirstCommonTypeMask = SourceTokens.front().TypeMask;
							PrependChar = ReplaceStrTokens.back().PrependChar;
						}

						for (auto& ReplaceI: make_range(std::next(ReplaceStrTokens.begin()), ReplaceStrTokens.end()))
						{
							ToPreserveStyleType(ReplaceI.Token, ChoosePreserveStyleType(AfterFirstCommonTypeMask));
							ReplaceI.PrependChar = PrependChar;
						}
					}

				}

				ReplaceStr.clear();
				std::for_each(CONST_RANGE(ReplaceStrTokens, i)
				{
					if (i.PrependChar)
						ReplaceStr += i.PrependChar;
					ReplaceStr += i.Token;
				});

				CurPos = I;
				SearchLength = static_cast<int>(Idx-I);

				return true;
			}
		}
	}

	return false;
}
