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

static std::vector<PreserveStyleToken> InternalPreserveStyleTokenize(const string& strStr, size_t From, size_t Length)
{
	std::vector<PreserveStyleToken> Result;

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
				Result.push_back(T);
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
			Result.push_back(T);
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
			Result.push_back(T);
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
		Result.push_back(T);
	}

	if (Result.size() > 1)
	{
		wchar_t PrependChar = Result[1].PrependChar;
		for (size_t I = 2; I < Result.size(); I++)
			if (PrependChar != Result[I].PrependChar)
			{
				Result.clear();
				PreserveStyleToken T;
				T.Token = strStr.SubStr(From, Length);
				T.PrependChar = 0;
				T.TypeMask = 1 << UNKNOWN;
				Result.push_back(T);
				return Result;
			}
	}

	return Result;
}

static std::vector<PreserveStyleToken> PreserveStyleTokenize(const string& strStr, size_t From, size_t Length)
{
	std::vector<PreserveStyleToken> Tokens = InternalPreserveStyleTokenize(strStr, From, Length);

	int Mask = -1;
	for (size_t I = 0; I < Tokens.size(); I++)
	{
		if (I == 0)
			Tokens[I].TypeMask = GetPeserveCaseStyleMask(Tokens[I].Token);
		else
		{
			if (Mask == -1)
				Mask = GetPeserveCaseStyleMask(Tokens[I].Token);
			else
				Mask &= GetPeserveCaseStyleMask(Tokens[I].Token);
		}
	}

	for (size_t I = 1; I < Tokens.size(); I++)
		Tokens[I].TypeMask = Mask;

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


	std::vector<PreserveStyleToken> StrTokens = PreserveStyleTokenize(Str, 0, Str.GetLength());

	for (int I=Position; (Reverse && I>=0) || (!Reverse && static_cast<size_t>(I)<StrSize); Reverse ? I--:I++)
	{
		if (WholeWords && !(I == 0 || IsSpace(Source[I-1]) || wcschr(WordDiv, Source[I-1])))
			continue;

		bool Matched = true;

		size_t Idx = I;
		size_t J = 0;
		size_t T = 0;

		while (((J+1 < StrTokens.size()) || (J+1 == StrTokens.size() && T < StrTokens[J].Token.GetLength()))
			&& Source[Idx])
		{
			bool Sep = (static_cast<size_t>(I) < Idx && Source[I+1] && IsPreserveStyleTokenSeparator(Source[Idx])
					&& !IsPreserveStyleTokenSeparator(Source[Idx-1])
					&& !IsPreserveStyleTokenSeparator(Source[Idx+1]));

			if (Sep)
			{
				if (T == StrTokens[J].Token.GetLength())
				{
					Idx++;
					T = 0;
					J++;
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
				if (T == StrTokens[J].Token.GetLength())
				{
					T = 0;
					J++;
					continue;
				}
				else
				{
					Matched = false;
					break;
				}
			}

			if (T >= StrTokens[J].Token.GetLength())
			{
				Matched = false;
				break;
			}

			if (Case && Source[Idx] != StrTokens[J].Token[T])
			{
				Matched = false;
				break;
			}

			if (!Case && Upper(Source[Idx]) != Upper(StrTokens[J].Token[T]))
			{
				Matched = false;
				break;
			}

			T++;
			Idx++;
		}

		if (WholeWords && !(Idx >= StrSize || IsSpace(Source[Idx]) || wcschr(WordDiv, Source[Idx])))
			continue;
		
		if (Matched && T == StrTokens[J].Token.GetLength() && J+1 == StrTokens.size())
		{
			std::vector<PreserveStyleToken> SourceTokens = PreserveStyleTokenize(Source, I, Idx-I);
			
			bool Same = SourceTokens.size() == StrTokens.size();
			for (size_t K = 0; Same && K < SourceTokens.size(); K++)
				Same &= SourceTokens[K].Token.GetLength() == StrTokens[K].Token.GetLength();

			if (Same)
			{
				std::vector<PreserveStyleToken> ReplaceStrTokens = PreserveStyleTokenize(ReplaceStr, 0, ReplaceStr.GetLength());
				ToPreserveStyleType(ReplaceStrTokens[0].Token, ChoosePreserveStyleType(SourceTokens[0].TypeMask));
				ReplaceStrTokens[0].PrependChar = SourceTokens[0].PrependChar;

				if (!SourceTokens.empty())
				{
					for (size_t K = 1; K < ReplaceStrTokens.size(); K++)
					{
						ToPreserveStyleType(ReplaceStrTokens[K].Token, ChoosePreserveStyleType(SourceTokens.back().TypeMask));
						ReplaceStrTokens[K].PrependChar = SourceTokens.back().PrependChar;
					}
				}
				ReplaceStr.Clear();
				for (size_t K = 0; K < ReplaceStrTokens.size(); K++)
				{
					if (ReplaceStrTokens[K].PrependChar != 0)
						ReplaceStr += ReplaceStrTokens[K].PrependChar;
					ReplaceStr += ReplaceStrTokens[K].Token;
				}

				CurPos = I;
				SearchLength = static_cast<int>(Idx-I);

				return true;
			}
		}
	}

	return false;		
}
