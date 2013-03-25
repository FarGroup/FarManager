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

static const wchar_t *PreserveStyleTokenSeparators = L"_-.";

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
	return wcschr(PreserveStyleTokenSeparators, C) != 0;
}

static int GetPeserveCaseStyleMask(const string& strStr)
{
	int Result = 15;

	for (size_t I = 0; I < strStr.GetLength(); I++)
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

static std::list<PreserveStyleToken> InternalPreserveStyleTokenize(const string& strStr, size_t From, size_t Length)
{
	std::list<PreserveStyleToken> Result;

	wchar_t Sep = 0;
	std::vector<bool> Seps(Length, false);
	for (size_t I = From+1; I+1 < From+Length; I++)
		if (IsPreserveStyleTokenSeparator(strStr[I])
				&& !IsPreserveStyleTokenSeparator(strStr[I-1])
				&& !IsPreserveStyleTokenSeparator(strStr[I+1]))
		{
			if (Sep != 0 && strStr[I] != Sep)
			{
				PreserveStyleToken T;
				T.Token = strStr.SubStr(From, Length);
				T.PrependChar = 0;
				T.TypeMask = 1 << UNKNOWN;
				Result.emplace_back(T);
				return Result;
			}

			Seps[I-From] = true;
		}

	size_t L = From;
	for (size_t I = From+1; I < From+Length; I++)
	{
		if (Seps[I-From])
		{
			PreserveStyleToken T;
			T.Token = strStr.SubStr(L, I-L);
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
			T.Token = strStr.SubStr(L, I-L);
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
		T.Token = strStr.SubStr(L, From+Length-L);
		T.PrependChar = 0;
		if (L >= From + 1 && Seps[L-1-From])
			T.PrependChar = strStr[L-1];
		Result.emplace_back(T);
	}

	if (Result.size() > 1)
	{
		auto Start = Result.cbegin();
		++Start;
		wchar_t PrependChar = Start->PrependChar;
		++Start;
		for (auto i = Start; i != Result.end(); ++i)
		{
			if (PrependChar != i->PrependChar)
			{
				Result.clear();
				PreserveStyleToken T;
				T.Token = strStr.SubStr(From, Length);
				T.PrependChar = 0;
				T.TypeMask = 1 << UNKNOWN;
				Result.emplace_back(T);
				return Result;
			}
		}
	}

	return Result;
}

static std::list<PreserveStyleToken> PreserveStyleTokenize(const string& strStr, size_t From, size_t Length)
{
	std::list<PreserveStyleToken> Tokens = InternalPreserveStyleTokenize(strStr, From, Length);

	int Mask = -1;
	bool First = true;
	std::for_each(RANGE(Tokens, i)
	{
		if (First)
		{
			i.TypeMask = GetPeserveCaseStyleMask(i.Token);
			First = false;
		}
		else
		{
			if (Mask == -1)
				Mask = GetPeserveCaseStyleMask(i.Token);
			else
				Mask &= GetPeserveCaseStyleMask(i.Token);
		}
	});

	auto Start = Tokens.begin();
	++Start;
	std::for_each(Start, Tokens.end(), [&Mask](VALUE_TYPE(Tokens)& i)
	{
		i.TypeMask = Mask;
	});
	return Tokens;
}

static void ToPreserveStyleType(string& strStr, PreserveStyleType type)
{
	if (type != UNKNOWN)
	{
		for (size_t I = 0; I < strStr.GetLength(); I++)
		{
			if (type == UPPERCASE_ALL)
				strStr.GetBuffer()[I] = Upper(strStr[I]);
			if (type == LOWERCASE_ALL)
				strStr.GetBuffer()[I] = Lower(strStr[I]);
			if (type == UPPERCASE_FIRST)
			{
				if (I == 0)
					strStr.GetBuffer()[I] = Upper(strStr[I]);
				else
					strStr.GetBuffer()[I] = Lower(strStr[I]);
			}
		}
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

bool PreserveStyleReplaceString(const wchar_t *Source, size_t StrSize, const string& Str, string& ReplaceStr, int& CurPos, int Position, int Case, int WholeWords, const wchar_t *WordDiv, int Reverse, int& SearchLength)
{
	SearchLength = 0;

	if (Reverse)
	{
		Position--;

		Position = std::min(Position, static_cast<int>(StrSize-1));

		if (Position<0)
			return false;
	}

	if (static_cast<size_t>(Position) >= StrSize || Str.IsEmpty() || ReplaceStr.IsEmpty())
		return false;


	std::list<PreserveStyleToken> StrTokens = PreserveStyleTokenize(Str, 0, Str.GetLength());

	for (int I=Position; (Reverse && I>=0) || (!Reverse && static_cast<size_t>(I)<StrSize); Reverse ? I--:I++)
	{
		if (WholeWords && !(I == 0 || IsSpace(Source[I-1]) || wcschr(WordDiv, Source[I-1])))
			continue;

		bool Matched = true;

		size_t Idx = I;
		size_t T = 0;

		auto j = StrTokens.cbegin();
		auto LastItem = StrTokens.cend();
		--LastItem;
		while (((j != LastItem) || (j == LastItem && T < j->Token.GetLength()))
			&& Source[Idx])
		{
			bool Sep = (static_cast<size_t>(I) < Idx && Source[I+1] && IsPreserveStyleTokenSeparator(Source[Idx])
					&& !IsPreserveStyleTokenSeparator(Source[Idx-1])
					&& !IsPreserveStyleTokenSeparator(Source[Idx+1]));

			if (Sep)
			{
				if (T == j->Token.GetLength())
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
				if (T == j->Token.GetLength())
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

			if (T >= j->Token.GetLength())
			{
				Matched = false;
				break;
			}

			if (Case && Source[Idx] != j->Token[T])
			{
				Matched = false;
				break;
			}

			if (!Case && Upper(Source[Idx]) != Upper(j->Token[T]))
			{
				Matched = false;
				break;
			}

			T++;
			Idx++;
		}

		if (WholeWords && !(Idx >= StrSize || IsSpace(Source[Idx]) || wcschr(WordDiv, Source[Idx])))
			continue;
		
		if (Matched && T == j->Token.GetLength() && j == LastItem)
		{
			std::list<PreserveStyleToken> SourceTokens = PreserveStyleTokenize(Source, I, Idx-I);
			
			bool Same = SourceTokens.size() == StrTokens.size();
			if(Same)
			{
				for(auto SrcI = SourceTokens.cbegin(), StrI = StrTokens.cbegin(); Same && SrcI != SourceTokens.cend(); ++SrcI, ++StrI)
					Same &= SrcI->Token.GetLength() == StrI->Token.GetLength();
			}
			if (Same)
			{
				std::list<PreserveStyleToken> ReplaceStrTokens = PreserveStyleTokenize(ReplaceStr, 0, ReplaceStr.GetLength());
				ToPreserveStyleType(ReplaceStrTokens.front().Token, ChoosePreserveStyleType(SourceTokens.front().TypeMask));
				ReplaceStrTokens.front().PrependChar = SourceTokens.front().PrependChar;

				if (!SourceTokens.empty())
				{
					auto ReplaceI = ReplaceStrTokens.begin();
					++ReplaceI;
					auto SourceI = SourceTokens.cbegin();
					++SourceI;
					for (; ReplaceI != ReplaceStrTokens.end(); ++ReplaceI, ++SourceI)
					{
						ToPreserveStyleType(ReplaceI->Token, ChoosePreserveStyleType(SourceI->TypeMask));
						ReplaceI->PrependChar = SourceTokens.back().PrependChar;
					}
				}
				ReplaceStr.Clear();
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
