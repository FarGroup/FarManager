/*
strmix.cpp

Куча разных вспомогательных функций по работе со строками
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "strmix.hpp"

// Internal:
#include "RegExp.hpp"
#include "lang.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "preservestyle.hpp"
#include "locale.hpp"
#include "encoding.hpp"
#include "regex_helpers.hpp"
#include "string_utils.hpp"
#include "global.hpp"
#include "codepage.hpp"

// Platform:

// Common:
#include "common/bytes_view.hpp"
#include "common/from_string.hpp"
#include "common/function_ref.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static auto GroupDigitsImpl(unsigned long long Value, detail::locale const& Locale)
{
	wchar_t DecimalSeparator[]{ Locale.decimal_separator(), L'\0' };
	wchar_t ThousandSeparator[]{ Locale.thousand_separator(), L'\0' };

	NUMBERFMT const Fmt
	{
		// Not needed - can't be decimal
		.NumDigits = 0,
		// Don't care - can't be decimal
		.LeadingZero = 1,
		.Grouping = Locale.digits_grouping(),
		.lpDecimalSep = DecimalSeparator,
		.lpThousandSep = ThousandSeparator,
		// Don't care - can't be negative
		.NegativeOrder = 1,
	};

	auto Src = str(Value);

	if (string Result; os::detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, Result, [&](std::span<wchar_t> Buffer)
	{
		const size_t Size = GetNumberFormat(LOCALE_USER_DEFAULT, 0, Src.c_str(), &Fmt, Buffer.data(), static_cast<int>(Buffer.size()));
		return Size? Size - 1 : 0;
	}))
		return Result;

	// Better than nothing
	return Src;
}

string GroupDigits(unsigned long long Value)
{
	return GroupDigitsImpl(Value, locale);
}

string GroupDigitsInvariant(unsigned long long Value)
{
	return GroupDigitsImpl(Value, invariant_locale());
}

static wchar_t* legacy_InsertQuotes(wchar_t *Str)
{
	const auto QuoteChar = L'"';
	size_t l = std::wcslen(Str);

	if (*Str != QuoteChar)
	{
		std::copy_n(Str, ++l, Str + 1);
		*Str=QuoteChar;
	}

	if (l==1 || Str[l-1] != QuoteChar)
	{
		Str[l++] = QuoteChar;
		Str[l] = 0;
	}

	return Str;
}

string InsertRegexpQuote(string strStr)
{
	//выражение вида /regexp/i не дополняем слешами
	if (!strStr.empty() && strStr[0] != L'/')
	{
		strStr.insert(0, 1, L'/');
		strStr += L'/';
	}

	return strStr;
}

wchar_t* legacy::QuoteSpaceOnly(wchar_t* Str)
{
	if (contains(Str, L' '))
		legacy_InsertQuotes(Str);

	return Str;
}

void inplace::QuoteSpace(string& Str)
{
	if (Str.find_first_of(Global->Opt->strQuotedSymbols.Get()) != string::npos)
		quote(Str);
}

void inplace::QuoteOuterSpace(string& Str)
{
	if (!Str.empty() && (Str.front() == L' ' || Str.back() == L' '))
		quote(Str);
}

// TODO: "…" is displayed as "." in raster fonts. Make it lng-customisable?
static const auto Dots = L"…"sv;

static auto legacy_operation(wchar_t* Str, int MaxLength, function_ref<void(std::span<wchar_t>, size_t, string_view)> const Handler)
{
	assert(MaxLength >= 0);
	const size_t Max = std::max(0, MaxLength);

	if (!Str || !*Str)
		return Str;

	const auto Size = std::wcslen(Str);

	if (Size <= Max)
		return Str;

	Handler({ Str, Size }, Max, Dots.substr(0, Max));
	return Str;
}

void inplace::truncate_right(string& Str, size_t const MaxLength)
{
	if (Str.size() <= MaxLength)
		return;

	const auto CurrentDots = Dots.substr(0, MaxLength);
	Str.replace(MaxLength - CurrentDots.size(), Str.size() - MaxLength + CurrentDots.size(), CurrentDots);
}

string truncate_right(string Str, size_t const MaxLength)
{
	inplace::truncate_right(Str, MaxLength);
	return Str;
}

string truncate_right(string_view const Str, size_t const MaxLength)
{
	return truncate_right(string(Str), MaxLength);
}

wchar_t* legacy::truncate_left(wchar_t *Str, int MaxLength)
{
	return legacy_operation(Str, MaxLength, [](std::span<wchar_t> const StrParam, size_t const MaxLengthParam, string_view const CurrentDots)
	{
		const auto Iterator = copy_string(CurrentDots, StrParam.data());

		const auto StrEnd = StrParam.end();
		const auto StrBegin = StrEnd - MaxLengthParam + CurrentDots.size();

		*std::copy(StrBegin, StrEnd, Iterator) = {};
	});
}

void inplace::truncate_left(string& Str, size_t const MaxLength)
{
	if (Str.size() <= MaxLength)
		return;

	const auto CurrentDots = Dots.substr(0, MaxLength);
	Str.replace(0, Str.size() - MaxLength + CurrentDots.size(), CurrentDots);
}

string truncate_left(string Str, size_t const MaxLength)
{
	inplace::truncate_left(Str, MaxLength);
	return Str;
}

string truncate_left(string_view const Str, size_t const MaxLength)
{
	return truncate_left(string(Str), MaxLength);
}

void inplace::truncate_center(string& Str, size_t const MaxLength)
{
	if (Str.size() <= MaxLength)
		return;

	const auto CurrentDots = Dots.substr(0, MaxLength);
	Str.replace((MaxLength - CurrentDots.size()) / 2, Str.size() - MaxLength + CurrentDots.size(), CurrentDots);
}

string truncate_center(string Str, size_t const MaxLength)
{
	inplace::truncate_center(Str, MaxLength);
	return Str;
}

string truncate_center(string_view const Str, size_t const MaxLength)
{
	return truncate_center(string(Str), MaxLength);
}

static auto StartOffset(string_view const Str)
{
	size_t DirOffset = 0;
	ParsePath(Str, &DirOffset);
	return DirOffset;
}

wchar_t* legacy::truncate_path(wchar_t*Str, int MaxLength)
{
	return legacy_operation(Str, MaxLength, [](std::span<wchar_t> const StrParam, size_t const MaxLengthParam, string_view const CurrentDots)
	{
		const auto Offset = std::min(StartOffset(StrParam.data()), MaxLengthParam - CurrentDots.size());

		const auto Iterator = copy_string(CurrentDots, StrParam.data() + Offset);

		const auto StrEnd = StrParam.end();
		const auto StrBegin = StrEnd - MaxLengthParam + CurrentDots.size() + Offset;

		*std::copy(StrBegin, StrEnd, Iterator) = {};
	});
}

void inplace::truncate_path(string& Str, size_t const MaxLength)
{
	if (Str.size() <= MaxLength)
		return;

	const auto CurrentDots = Dots.substr(0, MaxLength);
	const auto Offset = std::min(StartOffset(Str), MaxLength - CurrentDots.size());
	Str.replace(Offset, Str.size() - MaxLength + CurrentDots.size(), CurrentDots);
}

string truncate_path(string Str, size_t const MaxLength)
{
	inplace::truncate_path(Str, MaxLength);
	return Str;
}

string truncate_path(string_view const Str, size_t const MaxLength)
{
	return truncate_path(string(Str), MaxLength);
}

bool IsCaseMixed(const string_view Str)
{
	const auto AlphaBegin = std::ranges::find_if(Str, is_alpha);
	if (AlphaBegin == Str.cend())
		return false;

	const auto Case = is_lower(*AlphaBegin);
	return std::any_of(AlphaBegin, Str.cend(), [Case](wchar_t c){ return is_alpha(c) && is_lower(c) != Case; });
}

template<size_t multiplier>
struct units
{
	enum: unsigned long long
	{
		B = 1,
		K = B * multiplier,
		M = K * multiplier,
		G = M * multiplier,
		T = G * multiplier,
		P = T * multiplier,
		E = P * multiplier,
	};
};

using binary = units<1024>;
using decimal = units<1000>;

namespace id
{
	enum { invariant, localized };
	enum { binary, decimal };
}

static constexpr struct
{
	unsigned long long Value;
	char Symbol;
}
BytesInUnit[][2]
{
#define BD_UNIT(x) { { binary::x, *#x }, { decimal::x, *#x + ('a' - 'A') } }

	BD_UNIT(B),
	BD_UNIT(K),
	BD_UNIT(M),
	BD_UNIT(G),
	BD_UNIT(T),
	BD_UNIT(P),
	BD_UNIT(E),

#undef BD_UNIT
};

static constexpr unsigned long long PrecisionMultiplier[]
{
	  1ull,
	 10ull,
	100ull,
};

static consteval auto invariant_symbols()
{
	std::array<std::array<wchar_t, 2>, std::size(BytesInUnit)> Result;

	for (size_t i = 0; i != std::size(BytesInUnit); ++i)
	{
		Result[i][id::binary] = BytesInUnit[i][id::binary].Symbol;
		Result[i][id::decimal] = BytesInUnit[i][id::decimal].Symbol;
	}

	return Result;
}

static constinit decltype(invariant_symbols()) UnitSymbol[2]
{
	invariant_symbols()
};

void PrepareUnitStr()
{
	for (const auto i: std::views::iota(0uz, std::size(BytesInUnit)))
	{
		const auto LocalizedSymbol = msg(lng::MListBytes + i).front();
		auto& Dest = UnitSymbol[id::localized][i];
		Dest[id::binary] = upper(LocalizedSymbol);
		Dest[id::decimal] = lower(LocalizedSymbol);
	}
}

static string FileSizeToStrImpl(unsigned long long const FileSize, int const WidthWithSign, unsigned long long const ViewFlags, detail::locale const& Locale)
{
	const auto IsInvariantLocale = Locale.is_invariant();

	if (!IsInvariantLocale && !UnitSymbol[id::localized][0][0])
		PrepareUnitStr();

	const auto& Symbol = UnitSymbol[IsInvariantLocale? id::invariant : id::localized];
	const size_t Width = std::abs(WidthWithSign);
	const bool LeftAlign = WidthWithSign < 0;
	const bool UseGroupDigits = (ViewFlags & COLFLAGS_GROUPDIGITS) != 0;
	const bool UseFloatSize = (ViewFlags & COLFLAGS_FLOATSIZE) != 0;
	const bool UseCompact = (ViewFlags & COLFLAGS_ECONOMIC) != 0;
	const bool UseUnit = (ViewFlags & COLFLAGS_USE_MULTIPLIER) != 0;
	const bool ShowUnit = (ViewFlags & COLFLAGS_SHOW_MULTIPLIER) != 0;
	const bool UseBinaryUnit = (ViewFlags & COLFLAGS_THOUSAND) == 0;
	const size_t MinUnit = (ViewFlags & COLFLAGS_MULTIPLIER_MASK & ~COLFLAGS_USE_MULTIPLIER) + 1;

	constexpr auto
		log2_of_1024 = 10,
		log10_of_1000 = 3;

	constexpr std::pair
		BinaryDivider(id::binary, log2_of_1024),
		DecimalDivider(id::decimal, log10_of_1000);

	const auto& [BaseIndex, BasePower] = ViewFlags & COLFLAGS_THOUSAND? DecimalDivider : BinaryDivider;

	const auto FormatSize = [&](string&& StrSize, size_t UnitIndex)
	{
		const auto FitToWidth = [&](string Str)
		{
			if (!Width)
				return Str;

			if (Str.size() <= Width)
			{
				(LeftAlign? inplace::pad_right : inplace::pad_left)(Str, Width, L' ');
				return Str;
			}

			LeftAlign?
				inplace::cut_right(Str, Width - 1) :
				inplace::cut_left(Str, Width - 1);

			Str.insert(LeftAlign? Str.end() : Str.begin(), L'…');
			return Str;
		};

		if (!UnitIndex && !ShowUnit)
			return FitToWidth(std::move(StrSize));

		return FitToWidth(concat(StrSize, UseCompact? L""sv : L" "sv, Symbol[UnitIndex][UseBinaryUnit? id::binary : id::decimal]));
	};

	if (UseFloatSize)
	{
		const auto Numerator = FileSize? (ViewFlags & COLFLAGS_THOUSAND)? std::log10(FileSize) : std::log2(FileSize) : 0;
		const size_t UnitIndex = Numerator / BasePower;

		string Str;

		if (!UnitIndex)
		{
			Str = str(FileSize);
		}
		else
		{
			const auto Denominator = BytesInUnit[UnitIndex][BaseIndex].Value;
			const auto RawIntegral = FileSize / Denominator;
			const auto RawFractional = static_cast<double>(FileSize % Denominator) / static_cast<double>(Denominator);

			const auto FixedPrecision = 0; // 0 for floating, else fixed. TODO: option?

			if (const auto NumDigits = FixedPrecision? std::min(FixedPrecision, static_cast<int>(std::size(PrecisionMultiplier) - 1)) : RawIntegral < 10? 2 : RawIntegral < 100? 1 : 0)
			{
				const auto [IntegralPart, FractionalPart] = [&]
				{
					const auto Multiplier = PrecisionMultiplier[NumDigits];
					const auto FractionalDigits = RawFractional * static_cast<double>(Multiplier);
					const auto UseRound = true;
					const auto RoundedFractionalDigits = static_cast<unsigned>(UseRound? std::round(FractionalDigits) : FractionalDigits);
					return RoundedFractionalDigits == Multiplier? std::pair(RawIntegral + 1, 0u) : std::pair(RawIntegral, RoundedFractionalDigits);
				}();

				Str = concat(str(IntegralPart), Locale.decimal_separator(), pad_left(str(FractionalPart), NumDigits, L'0'));
			}
			else
			{
				Str = str(static_cast<unsigned long long>(std::round(static_cast<double>(RawIntegral) + RawFractional)));
			}
		}

		return FormatSize(std::move(Str), UnitIndex);
	}

	const auto ToStr = [&](auto Size)
	{
		return UseGroupDigits? GroupDigitsImpl(Size, Locale) : str(Size);
	};

	size_t UnitIndex = 0;
	auto Str = ToStr(FileSize);

	const auto SuffixSize = (ShowUnit || (Width && Str.size() > Width))? UseCompact? 1u : 2u : 0u;

	const auto MaxNumberWidth = Width > SuffixSize? Width - SuffixSize : 0;

	while ((UseUnit && UnitIndex < MinUnit) || (Width && Str.size() > MaxNumberWidth))
	{
		const auto Denominator = BytesInUnit[UnitIndex + 1][BaseIndex].Value;
		const auto IntegralPart = FileSize / Denominator;
		const auto FractionalPart = static_cast<double>(FileSize % Denominator) / static_cast<double>(Denominator);

		if (const auto SizeInUnits = IntegralPart + static_cast<unsigned long long>(std::round(FractionalPart)))
		{
			++UnitIndex;
			Str = ToStr(SizeInUnits);
		}
		else
			break;
	}

	return FormatSize(std::move(Str), UnitIndex);
}

string FileSizeToStr(unsigned long long FileSize, int WidthWithSign, unsigned long long ViewFlags)
{
	return FileSizeToStrImpl(FileSize, WidthWithSign, ViewFlags, locale);
}

string FileSizeToStrInvariant(unsigned long long FileSize, int WidthWithSign, unsigned long long ViewFlags)
{
	return FileSizeToStrImpl(FileSize, WidthWithSign, ViewFlags, invariant_locale());
}

// Заменить в строке Str Count вхождений подстроки FindStr на подстроку ReplStr
// Если Count == npos - заменять "до полной победы"
bool ReplaceStrings(string& strStr, string_view FindStr, string_view ReplStr, const bool IgnoreCase, size_t Count)
{
	if (strStr.empty() || FindStr.empty() || !Count)
		return false;

	string FindCopy, ReplaceCopy;

	if (Count != 1 && within(strStr, FindStr))
	{
		FindCopy = FindStr;
		FindStr = FindCopy;
	}

	if (Count != 1 && within(strStr, ReplStr))
	{
		ReplaceCopy = ReplStr;
		ReplStr = ReplaceCopy;
	}

	size_t replaced = 0;
	size_t StartPos = 0;

	while ((StartPos = IgnoreCase?
		find_icase(strStr, FindStr, StartPos) :
		strStr.find(FindStr, StartPos)
		) != strStr.npos)
	{
		strStr.replace(StartPos, FindStr.size(), ReplStr);
		StartPos += ReplStr.size();
		++replaced;

		if (replaced == Count)
			break;
	}

	return replaced != 0;
}

void remove_duplicates(string& Str, wchar_t const Char, bool const IgnoreCase)
{
	const auto Removed = IgnoreCase?
		std::ranges::unique(Str, [Char, Eq = string_comparer_icase{}](wchar_t const First, wchar_t const Second){ return Eq(First, Char) && Eq(Second, Char); }) :
		std::ranges::unique(Str, [Char](wchar_t const First, wchar_t const Second){ return First == Char && Second == Char; });

	Str.resize(Str.size() - Removed.size());
}

bool wrapped_text::get(bool Reset, string_view& Value) const
{
	// TODO: implement in terms of enum_lines to support all kinds of EOLs?

	if (Reset)
		m_Tail = m_Str;

	if (m_Tail.empty())
		return false;

	const auto LineBreaks = L"\n"sv;
	const auto WordSpaceBreaks = L" "sv;
	const auto WordOtherBreaks = L",-"sv; // TODO: Opt.WordDiv?

	const auto advance = [&](size_t TokenEnd, size_t SeparatorSize)
	{
		Value = m_Tail.substr(0, TokenEnd);
		m_Tail.remove_prefix(TokenEnd + SeparatorSize);
		return true;
	};

	// Try to take a line, drop line breaks
	auto ChopSize = m_Tail.find_first_of(LineBreaks);
	auto BreaksSize = 1;

	if (ChopSize == m_Tail.npos)
	{
		ChopSize = m_Tail.size();
		BreaksSize = 0;
	}

	if (ChopSize <= m_Width)
		return advance(ChopSize, BreaksSize);

	// Try to take some words, drop spaces
	ChopSize = m_Tail.find_last_of(WordSpaceBreaks, m_Width);
	BreaksSize = 1;

	if (ChopSize == m_Tail.npos)
	{
		ChopSize = m_Tail.size();
		BreaksSize = 0;
	}

	if (ChopSize <= m_Width)
		return advance(ChopSize, BreaksSize);

	// Try to take some words, keep separators
	ChopSize = m_Tail.find_last_of(WordOtherBreaks, m_Width);
	BreaksSize = 1;

	if (ChopSize == m_Tail.npos)
	{
		ChopSize = m_Tail.size();
		BreaksSize = 0;
	}

	if (ChopSize + BreaksSize <= m_Width)
		return advance(ChopSize + BreaksSize, 0);

	// Take a part of the word
	return advance(m_Width, 0);
}

bool FindWordInString(string_view const Str, size_t CurPos, size_t& Begin, size_t& End, string_view const WordDiv0)
{
	if (Str.empty() || CurPos > Str.size())
		return false;

	const auto WordDiv = concat(WordDiv0, GetBlanks(), GetEols());

	if (!CurPos)
	{
		Begin = 0;
	}
	else
	{
		Begin = Str.find_last_of(WordDiv, CurPos - 1);
		Begin = Begin == string::npos? 0 : Begin + 1;
	}

	if (CurPos == Str.size())
	{
		End = CurPos;
	}
	else
	{
		End = Str.find_first_of(WordDiv, CurPos);
		if (End == string::npos)
		{
			End = Str.size();
		}
	}

	if (Begin == End)
	{
		// Go deeper and find one-character words even if they are in WordDiv, e.g. {}()<>,.= etc. (except whitespace)
		if (Begin == Str.size())
		{
			if (!std::iswspace(Str[Begin - 1]))
				--Begin;
		}
		else
		{
			if (!std::iswspace(Str[Begin]))
			{
				++End;
			}
			else
			{
				if (Begin && !std::iswspace(Str[Begin - 1]))
					--Begin;
			}
		}
	}

	return Begin != End;
}

bool CheckFileSizeStringFormat(string_view const FileSizeStr)
{
	// <Number>[Suffix]
	const auto Iterator = std::ranges::find_if_not(FileSizeStr, std::iswdigit);
	if (Iterator == FileSizeStr.cbegin())
		return false;

	if (Iterator == FileSizeStr.cend())
		return true;

	if (Iterator + 1 != FileSizeStr.cend())
		return false;

	return contains(L"BbKkMmGgTtPpEe"sv, *Iterator);
}

unsigned long long ConvertFileSizeString(string_view const FileSizeStr)
{
	if (!CheckFileSizeStringFormat(FileSizeStr))
		return 0;

	const auto n = from_string<unsigned long long>(FileSizeStr);

	// https://en.wikipedia.org/wiki/Binary_prefix
	// https://en.wikipedia.org/wiki/SI_prefix
	switch (upper(FileSizeStr.back()))
	{
		case L'K': return n << 10; // Ki kibi 1024^1 - ~kilo ~10^3
		case L'M': return n << 20; // Mi mebi 1024^2 - ~mega ~10^6
		case L'G': return n << 30; // Gi gibi 1024^3 - ~giga ~10^9
		case L'T': return n << 40; // Ti tebi 1024^4 - ~tera ~10^12
		case L'P': return n << 50; // Pi pebi 1024^5 - ~peta ~10^15
		case L'E': return n << 60; // Ei exbi 1024^6 - ~exa  ~10^18
		default:   return n;
	}
}

string ReplaceBrackets(
		const string_view SearchStr,
		const string_view ReplaceStr,
		std::span<RegExpMatch const> Match,
		const named_regex_match* NamedMatch
)
{
	string result;

	for (size_t i = 0, length = ReplaceStr.size(); i < length; ++i)
	{
		const auto CurrentChar = ReplaceStr[i];

		if (CurrentChar != L'$' || i + 1 == length)
		{
			result.push_back(CurrentChar);
			continue;
		}

		const auto TokenStart = i + 1;
		auto TokenEnd = TokenStart;
		size_t TokenSize = 0;

		size_t GroupNumber = 0;
		string_view Replacement;

		while (TokenEnd != length && std::iswdigit(ReplaceStr[TokenEnd]))
		{
			const auto NewGroupNumber = GroupNumber * 10 + ReplaceStr[TokenEnd] - L'0';
			if (NewGroupNumber >= Match.size())
				break;

			GroupNumber = NewGroupNumber;
			++TokenEnd;
		}

		if (TokenEnd != TokenStart)
		{
			Replacement = get_match(SearchStr, Match[GroupNumber]);
			TokenSize = TokenEnd - TokenStart;
		}
		else if (NamedMatch)
		{
			// {some text}
			const auto Part = ReplaceStr.substr(TokenStart);

			if (const auto MatchFirst = Part.find(L'{'); MatchFirst != Part.npos)
			{
				if (const auto MatchLast = Part.find(L'}', MatchFirst + 1); MatchLast != Part.npos)
				{
					TokenSize = MatchLast - MatchFirst + 1;
					const auto Iterator = NamedMatch->Matches.find(Part.substr(MatchFirst + 1, TokenSize - 2));
					Replacement = Iterator == NamedMatch->Matches.cend()?
						ReplaceStr.substr(i, TokenSize + 1) :
						get_match(SearchStr, Match[Iterator->second]);
				}
			}
		}

		if (TokenSize)
		{
			i += TokenSize;
			result += Replacement;
		}
		else
		{
			result += CurrentChar;
		}
	}

	return result;
}

namespace
{
	bool CanContainWholeWord(string_view const Haystack, size_t const Offset, size_t const NeedleSize, string_view const WordDiv)
	{
		assert(Offset <= Haystack.size());

		const auto SpaceOrWordDiv = [&WordDiv](wchar_t Ch)
		{
			return std::iswspace(Ch) || contains(WordDiv, Ch);
		};

		if (Offset && !SpaceOrWordDiv(Haystack[Offset - 1]))
			return false;

		if (Offset + NeedleSize > Haystack.size())
			return false;

		if (Offset + NeedleSize < Haystack.size() && !SpaceOrWordDiv(Haystack[Offset + NeedleSize]))
			return false;

		return true;
	}

	bool SearchStringRegex(
		string_view const Source,
		const RegExp& re,
		regex_match& Match,
		named_regex_match* const NamedMatch,
		intptr_t Position,
		search_replace_string_options const options,
		string& ReplaceStr,
		int& CurPos,
		int& SearchLength,
		string_view WordDiv)
	{
		if (!options.Reverse)
		{
			auto CurrentPosition = Position;

			do
			{
				if (!re.SearchEx(Source, CurrentPosition, Match, NamedMatch))
					return false;

				if (options.WholeWords && !CanContainWholeWord(Source, Match.Matches[0].start, Match.Matches[0].end - Match.Matches[0].start, WordDiv))
				{
					++CurrentPosition;
					continue;
				}

				ReplaceStr = ReplaceBrackets(Source, ReplaceStr, Match.Matches, NamedMatch);
				CurPos = Match.Matches[0].start;
				SearchLength = Match.Matches[0].end - Match.Matches[0].start;
				return true;
			}
			while (static_cast<size_t>(CurrentPosition) != Source.size());
		}

		bool found = false;
		intptr_t pos = 0;

		regex_match FoundMatch;
		named_regex_match FoundNamedMatch;

		while (re.SearchEx(Source, pos, Match, NamedMatch))
		{
			pos = Match.Matches[0].start;
			if (pos > Position)
				break;

			if (options.WholeWords && !CanContainWholeWord(Source, Match.Matches[0].start, Match.Matches[0].end - Match.Matches[0].start, WordDiv))
			{
				++pos;
				continue;
			}

			found = true;
			FoundMatch.Matches = std::move(Match.Matches);
			if (NamedMatch)
				FoundNamedMatch.Matches = std::move(NamedMatch->Matches);
			++pos;
		}

		if (found)
		{
			ReplaceStr = ReplaceBrackets(Source, ReplaceStr, FoundMatch.Matches, NamedMatch? &FoundNamedMatch : nullptr);
			CurPos = FoundMatch.Matches[0].start;
			SearchLength = FoundMatch.Matches[0].end - FoundMatch.Matches[0].start;

		}

		return found;
	}
}

bool SearchString(
	string_view const Haystack,
	string_view const Needle,
	i_searcher const& NeedleSearcher,
	const RegExp& re,
	regex_match& Match,
	named_regex_match* const NamedMatch,
	int& CurPos,
	search_replace_string_options const options,
	int& SearchLength,
	string_view WordDiv)
{
	string Dummy;
	return SearchAndReplaceString(
		Haystack,
		Needle,
		NeedleSearcher,
		re,
		Match,
		NamedMatch,
		Dummy,
		CurPos,
		options,
		SearchLength,
		WordDiv
	);
}

bool SearchAndReplaceString(
	string_view const Haystack,
	string_view const Needle,
	i_searcher const& NeedleSearcher,
	const RegExp& re,
	regex_match& Match,
	named_regex_match* const NamedMatch,
	string& ReplaceStr,
	int& CurPos,
	search_replace_string_options const options,
	int& SearchLength,
	string_view WordDiv)
{
	SearchLength = 0;

	if (WordDiv.empty())
		WordDiv = Global->Opt->strWordDiv;

	if (!options.Regex && options.PreserveStyle && PreserveStyleReplaceString(Haystack, Needle, ReplaceStr, CurPos, options, WordDiv, SearchLength))
		return true;

	if (Needle.empty())
		return true;

	auto Position = CurPos;
	const auto HaystackSize = static_cast<int>(Haystack.size());

	if (options.Reverse)
	{
		// MZK 2018-04-01 BUGBUG: regex reverse search: "^$" does not match empty string
		Position = std::min(Position - 1, HaystackSize - 1);

		if (Position < 0)
			return false;
	}

	if (options.Regex)
	{
		// Empty Haystack is ok for regex search, e.g. ^$
		if ((Position || HaystackSize) && Position >= HaystackSize)
			return false;

		return SearchStringRegex(Haystack, re, Match, NamedMatch, Position, options, ReplaceStr, CurPos, SearchLength, WordDiv);
	}

	if (Position >= HaystackSize)
		return false;

	auto Where = options.Reverse?
		Haystack.substr(0, Position + 1) :
		Haystack.substr(Position);

	const auto Next = [&](size_t const Offset)
	{
		Where = options.Reverse?
			Where.substr(0, Offset > 0? Offset - 1 : 0) :
			Where.substr(Offset + 1);
	};

	while (!Where.empty())
	{
		const auto FoundPosition = NeedleSearcher.find_in(Where, options.Reverse);
		if (!FoundPosition)
			return false;

		const auto [FoundOffset, FoundSize] = *FoundPosition;

		const auto AbsoluteOffset = options.Reverse? FoundOffset : Haystack.size() - Where.size() + FoundOffset;

		if (options.WholeWords && !CanContainWholeWord(Haystack, AbsoluteOffset, FoundSize, WordDiv))
		{
			Next(FoundOffset);
			continue;
		}

		CurPos = static_cast<int>(AbsoluteOffset);
		SearchLength = static_cast<int>(FoundSize);

		// В случае PreserveStyle: если не получилось сделать замену c помощью PreserveStyleReplaceString,
		// то хотя бы сохранить регистр первой буквы.
		if (options.PreserveStyle && !ReplaceStr.empty() && is_alpha(ReplaceStr.front()) && is_alpha(Haystack[CurPos]))
		{
			if (is_upper(Haystack[CurPos]))
				inplace::upper(ReplaceStr.front());
			else if (is_lower(Haystack[CurPos]))
				inplace::lower(ReplaceStr.front());
		}

		return true;
	}

	return false;
}

char IntToHex(int h)
{
	if (h > 0xF)
		throw far_fatal_exception(L"Not a hex char"sv);
	if (h >= 0xA)
		return 'A' + h - 0xA;
	return '0' + h;
}

int HexToInt(char h)
{
	if (h >= 'a' && h <= 'f')
		return h - 'a' + 0xA;

	if (h >= 'A' && h <= 'F')
		return h - 'A' + 0xA;

	if (std::isdigit(h))
		return h - '0';

	throw far_fatal_exception(L"Not a hex char"sv);
}

string BlobToHexString(bytes_view const Blob, wchar_t Separator)
{
	string Hex;

	Hex.reserve(Blob.size() * (Separator? 3 : 2));

	for (const auto& i: Blob)
	{
		Hex.push_back(IntToHex((std::to_integer<int>(i) & 0xF0) >> 4));
		Hex.push_back(IntToHex(std::to_integer<int>(i) & 0x0F));
		if (Separator)
		{
			Hex.push_back(Separator);
		}
	}

	if (Separator && !Hex.empty())
	{
		Hex.pop_back();
	}
	return Hex;
}

bytes HexStringToBlob(const string_view Hex, const wchar_t Separator)
{
	// Size shall be either 3 * N + 2 or even
	if (!Hex.empty() && (Separator? Hex.size() % 3 != 2 : Hex.size() & 1))
		throw far_fatal_exception(L"Incomplete hex string"sv);

	const auto SeparatorSize = Separator? 1 : 0;
	const auto StepSize = 2 + SeparatorSize;
	const auto AlignedSize = Hex.size() + SeparatorSize;
	const auto BlobSize = AlignedSize / StepSize;

	if (!BlobSize)
		return {};

	bytes Blob;
	Blob.reserve(BlobSize);
	for (size_t i = 0; i != AlignedSize; i += StepSize)
	{
		Blob.push_back(static_cast<std::byte>(HexToInt(Hex[i]) << 4 | HexToInt(Hex[i + 1])));
	}

	return Blob;
}

string ExtractHexString(string_view const HexString)
{
	const auto Trimmed = trim_right(HexString);
	string Result;
	Result.reserve((Trimmed.size() + 2) / 3 * 2);
	// TODO: Fix these and trailing spaces in Dialog class?
	std::ranges::remove_copy(Trimmed, std::back_inserter(Result), L' ');
	if (Result.size() & 1)
	{
		// Odd length - hex string is not valid.
		// This is an UI helper, so we don't want to throw.
		// Fixing it gracefully and in 1.7x compatible way:
		// "12 34 5" -> "12 34 05"
		Result.insert(Result.end() - 1, L'0');
	}
	return Result;
}

string ConvertHexString(string_view const From, uintptr_t Codepage, bool FromHex)
{
	const auto CompatibleCp = IsVirtualCodePage(Codepage)? encoding::codepage::ansi() : Codepage;
	if (FromHex)
	{
		const auto Blob = HexStringToBlob(ExtractHexString(From), 0);
		return encoding::get_chars(CompatibleCp, Blob);
	}
	else
	{
		const auto Blob = encoding::get_bytes(CompatibleCp, From);
		return BlobToHexString(view_bytes(Blob), 0);
	}
}

string BytesToString(bytes_view const Bytes, uintptr_t const Codepage)
{
	return encoding::get_chars(IsVirtualCodePage(Codepage)? encoding::codepage::ansi() : Codepage, Bytes);
}

string HexMask(size_t ByteCount)
{
	assert(ByteCount);

	string Result(ByteCount * 3 - 1, L'H');
	for (size_t i{ 2 }; i < Result.size(); i += 3)
	{
		Result[i] = L' '; // "HH HH ... HH"
	}
	return Result;
}

// dest и src НЕ ДОЛЖНЫ пересекаться
template<typename T>
static void xncpy(T* dest, const T* src, size_t DestSize)
{
	while (DestSize > 1 && (*dest++ = *src++) != 0)
	{
		DestSize--;
	}

	*dest = 0;
}

void xstrncpy(char* dest, const char* src, size_t DestSize)
{
	return xncpy(dest, src, DestSize);
}

void xwcsncpy(wchar_t* dest, const wchar_t* src, size_t DestSize)
{
	return xncpy(dest, src, DestSize);
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("InsertRegexpQuote")
{
	static const struct
	{
		string_view Str, Result;
	}
	Tests[]
	{
		{},
		{ L"/"sv,         L"/"sv },
		{ L"//"sv,        L"//"sv },
		{ L"///"sv,       L"///"sv },
		{ L"test"sv,      L"/test/"sv },
		{ L"/test/i"sv,   L"/test/i"sv },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(InsertRegexpQuote(string(i.Str)) == i.Result);
	}
}

TEST_CASE("ConvertFileSizeString")
{
	constexpr auto
		B =  0_bit,
		K = 10_bit,
		M = 20_bit,
		G = 30_bit,
		T = 40_bit,
		P = 50_bit,
		E = 60_bit;

	static const struct
	{
		string_view Src;
		uint64_t Result;
	}
	Tests[]
	{
		{ {},           0     },
		{ L"Beep"sv,    0     },
		{ L"0"sv,       0 * B },
		{ L"1"sv,       1 * B },
		{ L"32K"sv,    32 * K },
		{ L"32k"sv,    32 * K },
		{ L"a32K"sv,    0     },
		{ L"32K+"sv,    0     },
		{ L"640K"sv,  640 * K },
		{ L"1M"sv,      1 * M },
		{ L"345M"sv,  345 * M },
		{ L"2G"sv,      2 * G },
		{ L"3T"sv,      3 * T },
		{ L"42P"sv,    42 * P },
		{ L"12E"sv,    12 * E },
		{ L"0E"sv,      0 * E },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Result == ConvertFileSizeString(i.Src));
	}
}

TEST_CASE("ReplaceBrackets")
{
	static const struct
	{
		string_view Str, Replace, Result;
		std::initializer_list<RegExpMatch> Match;
		std::initializer_list<std::pair<string_view, size_t>> NamedMatch;
	}
	Tests[]
	{
		{},
		{ L"dorime"sv },
		{ L"meow"sv, L"${a }$cat$"sv, L"${a }$cat$"sv },
		{ L"Ni!"sv, L"$0$0$0"sv, L"Ni!Ni!Ni!"sv, { { 0, 3 } } },
		{ L"Fus Ro Dah"sv, L"$321-${first}-$2-$123-${nope}${oops$"sv, L"Dah21-Fus-Ro-Fus23-${nope}${oops$"sv, { { 0, 10 }, { 0, 3 }, { 4, 6 }, { 7, 10 } }, { { L"first"sv, 1 } } },
	};

	named_regex_match NamedRegexMatch;

	for (const auto& i: Tests)
	{
		NamedRegexMatch.Matches.clear();
		for (const auto& [k, v]: i.NamedMatch)
		{
			NamedRegexMatch.Matches.emplace(k, v);
		}

		REQUIRE(i.Result == ReplaceBrackets(i.Str, i.Replace, i.Match, &NamedRegexMatch));
	}
}

TEST_CASE("CanContainWholeWord")
{
	static const struct
	{
		string_view Haystack;
		std::initializer_list<std::initializer_list<int>> Table;
	}
	Tests[]
	{
		{
			{},
			{
				{ 1, 0 },
			}
		},
		{
			L" "sv,
			{
				{ 1, 1 },
				{ 1, 0 },
			}
		},
		{
			L"a"sv,
			{
				{ 0, 1 },
				{ 0, 0 },
			}
		},
		{
			L"ab"sv,
			{
				{ 0, 0, 1, 0 },
				{ 0, 0, 0 },
				{ 0, 0 },
			}
		},
		{
			L" ab "sv,
			{
				{ 1, 0, 0, 1, 1, 0 },
				{ 0, 0, 1, 1, 0 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0 },
				{ 1, 0 },
			}
		}
	};

	const auto WordDiv = L" "sv;

	for (const auto& i: Tests)
	{
		for (const auto& Row: i.Table)
		{
			for (const auto& Cell: Row)
			{
				REQUIRE(!!Cell == CanContainWholeWord(i.Haystack, &Row - i.Table.begin(), &Cell - Row.begin(), WordDiv));
			}
		}
	}
}

static consteval auto mutiply(auto const Value, auto const Multiplier)
{
	return static_cast<unsigned long long>(Value * static_cast<decltype(Value)>(std::to_underlying(Multiplier)));
}

#define DEFINE_LITERAL(U) \
static consteval auto operator ""_##U(unsigned long long const Value) { return mutiply(Value, binary::U); } \
static consteval auto operator ""_##U(long double const Value)        { return mutiply(Value, binary::U); }

DEFINE_LITERAL(K)
DEFINE_LITERAL(M)
DEFINE_LITERAL(G)
DEFINE_LITERAL(T)
DEFINE_LITERAL(P)
DEFINE_LITERAL(E)

#undef DEFINE_LITERAL

TEST_CASE("FileSizeToStrInvariant")
{
	const auto max = std::numeric_limits<uint64_t>::max();

	const struct
	{
		unsigned long long Size;
		string_view ResultBinary, ResultDecimal;
		int Width;
		unsigned long long Flags;
	}
	Tests[]
	{
		{    0,       L"0"sv,        L"0"sv,          0, 0 },
		{    1,       L"1"sv,        L"1"sv,          0, 0 },
		{ 1023,       L"1023"sv,     L"1023"sv,       0, 0 },
		{ 1024,       L"1024"sv,     L"1024"sv,       0, 0 },
		{ 1025,       L"1025"sv,     L"1025"sv,       0, 0 },

		{ 1024,       L"1024 "sv,    L"1024 "sv,     -5, 0 },
		{ 1024,       L"1024"sv,     L"1024"sv,      -4, 0 },
		{ 1024,       L"1 K"sv,      L"1 k"sv,       -3, 0 },
		{ 1024,       L"1…"sv,       L"1…"sv,        -2, 0 },
		{ 1024,       L"…"sv,        L"…"sv,         -1, 0 },

		{ 1024,       L"1K"sv,       L"1k"sv,        -2, COLFLAGS_ECONOMIC },

		{    0,       L"0"sv,        L"0"sv,          0, COLFLAGS_MULTIPLIER_K },
		{    1,       L"1"sv,        L"1"sv,          0, COLFLAGS_MULTIPLIER_K },
		{  511,       L"511"sv,      L"1 k"sv,        0, COLFLAGS_MULTIPLIER_K },
		{  512,       L"1 K"sv,      L"1 k"sv,        0, COLFLAGS_MULTIPLIER_K },
		{ 1023,       L"1 K"sv,      L"1 k"sv,        0, COLFLAGS_MULTIPLIER_K },
		{ 1024,       L"1 K"sv,      L"1 k"sv,        0, COLFLAGS_MULTIPLIER_K },
		{ 1025,       L"1 K"sv,      L"1 k"sv,        0, COLFLAGS_MULTIPLIER_K },

		{ 10_M,       L"10240 K"sv,  L"10486 k"sv,    0, COLFLAGS_MULTIPLIER_K },
		{ 1024_M,     L"1024 M"sv,   L"1074 m"sv,     0, COLFLAGS_MULTIPLIER_M },
		{ 400000_M,   L"391 G"sv,    L"419 g"sv,      0, COLFLAGS_MULTIPLIER_G },
		{ 4_P,        L"4096 T"sv,   L"4504 t"sv,     0, COLFLAGS_MULTIPLIER_T },
		{ 3000_T,     L"3 P"sv,      L"3 p"sv,        0, COLFLAGS_MULTIPLIER_P },
		{ max,        L"16 E"sv,     L"18 e"sv,       0, COLFLAGS_MULTIPLIER_E },

		{ 999,        L"999"sv,      L"999"sv,        0, COLFLAGS_FLOATSIZE },
		{ 1000,       L"1000"sv,     L"1.00 k"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1023,       L"1023"sv,     L"1.02 k"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1024,       L"1.00 K"sv,   L"1.02 k"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1536,       L"1.50 K"sv,   L"1.54 k"sv,     0, COLFLAGS_FLOATSIZE },
		{ 2042,       L"1.99 K"sv,   L"2.04 k"sv,     0, COLFLAGS_FLOATSIZE },
		{ 2043,       L"2.00 K"sv,   L"2.04 k"sv,     0, COLFLAGS_FLOATSIZE },
		{ 10_K,       L"10.0 K"sv,   L"10.2 k"sv,     0, COLFLAGS_FLOATSIZE },
		{ 10.14_K,    L"10.1 K"sv,   L"10.4 k"sv,     0, COLFLAGS_FLOATSIZE },
		{ 10.18_K,    L"10.2 K"sv,   L"10.4 k"sv,     0, COLFLAGS_FLOATSIZE },
		{ 100_K,      L"100 K"sv,    L"102 k"sv,      0, COLFLAGS_FLOATSIZE },
		{ 1_K,        L"1.00 K"sv,   L"1.02 k"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1.0_K,      L"1.00 K"sv,   L"1.02 k"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1_M,        L"1.00 M"sv,   L"1.05 m"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1.0_M,      L"1.00 M"sv,   L"1.05 m"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1_G,        L"1.00 G"sv,   L"1.07 g"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1.0_G,      L"1.00 G"sv,   L"1.07 g"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1_T,        L"1.00 T"sv,   L"1.10 t"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1.0_T,      L"1.00 T"sv,   L"1.10 t"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1_P,        L"1.00 P"sv,   L"1.13 p"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1.0_P,      L"1.00 P"sv,   L"1.13 p"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1_E,        L"1.00 E"sv,   L"1.15 e"sv,     0, COLFLAGS_FLOATSIZE },
		{ 1.0_E,      L"1.00 E"sv,   L"1.15 e"sv,     0, COLFLAGS_FLOATSIZE },
		{ max,        L"16.0 E"sv,   L"18.4 e"sv,     0, COLFLAGS_FLOATSIZE },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.ResultBinary == FileSizeToStrInvariant(i.Size, i.Width, i.Flags));
		REQUIRE(i.ResultDecimal == FileSizeToStrInvariant(i.Size, i.Width, i.Flags | COLFLAGS_THOUSAND));
	}
}

TEST_CASE("ReplaceStrings")
{
	static const struct
	{
		string_view Src, Find, Replace, Result;
	}
	Tests[]
	{
		{ L"lorem ipsum dolor"sv,    L"loREm"sv,                {},               L" ipsum dolor"sv,      },
		{ L"lorem ipsum dolor"sv,    L"lorem"sv,                L"alpha"sv,       L"alpha ipsum dolor"sv, },
		{ L"lorem ipsum dolor"sv,    L"m"sv,                    L"q"sv,           L"loreq ipsuq dolor"sv, },
		{ L"lorem ipsum dolor"sv,    {},                        L"alpha"sv,       L"lorem ipsum dolor"sv, },
		{ L"lorem ipsum dolor"sv,    {},                        {},               L"lorem ipsum dolor"sv, },
		{ L"lorem ipsum dolor"sv,    L"lorem ipsum dolor"sv,    {},               {},                     },
		{ L"lorem ipsum dolor"sv,    L"lorem ipsum dolor"sv,    L"bravo"sv,       L"bravo"sv,             },
		{ L"lorem"sv,                L"lorem ipsum"sv,          L"charlie"sv,     L"lorem"sv,             },
	};

	string Src;
	for (const auto& i: Tests)
	{
		Src = i.Src;
		ReplaceStrings(Src, i.Find, i.Replace, true);
		REQUIRE(i.Result == Src);
	}
}

TEST_CASE("ReplaceStrings.within")
{
	{
		auto Str = L"99 little bugs in the code. 99 little bugs in the code"s;
		const auto Find = string_view(Str).substr(0, 2);
		const auto Replace = string_view(Str).substr(3, 6);
		ReplaceStrings(Str, Find, Replace);
		REQUIRE(Str == L"little little bugs in the code. little little bugs in the code"sv);
	}

	{
		auto Str = L"banana banana banana banana"s;
		const auto Find = string_view(Str).substr(21, 6);
		const auto Replace = string_view(Str).substr(2, 2);
		ReplaceStrings(Str, Find, Replace);
		REQUIRE(Str == L"na na na na"sv);
	}

	{
		auto Str = L"Alegría Macarena"s;
		const auto Find = string_view(Str).substr(0, 7);
		const auto Replace = string_view(Str).substr(8, 8);
		// A single replace should pick the fast path
		ReplaceStrings(Str, Find, Replace, false, 1);
		REQUIRE(Str == L"Macarena Macarena"sv);
	}
}

TEST_CASE("remove_duplicates")
{
	static const struct
	{
		wchar_t Char;
		bool IgnoreCase;
		string_view Src, Result;
	}
	Tests[]
	{
		{ L'1', false, {},               {},          },
		{ L'2', false, L"1"sv,           L"1"sv,      },
		{ L'1', false, L"12"sv,          L"12"sv,     },
		{ L'2', false, L"122"sv,         L"12"sv,     },
		{ L'1', false, L"122"sv,         L"122"sv,    },
		{ L'1', false, L"111"sv,         L"1"sv,      },
		{ L'1', false, L"1122"sv,        L"122"sv,    },
		{ L'2', false, L"1122"sv,        L"112"sv,    },
		{ L'a', false, L"qaaaz"sv,       L"qaz"sv,    },
		{ L'b', false, L"qaaaz"sv,       L"qaaaz"sv,  },
		{ L'a', false, L"qAaAz"sv,       L"qAaAz"sv,  },
		{ L'a', true,  L"qqAaAzz"sv,     L"qqAzz"sv,  },
		{ L'a', true,  L"qqaAazz"sv,     L"qqazz"sv,  },
	};

	string Src;
	for (const auto& i: Tests)
	{
		Src = i.Src;
		remove_duplicates(Src, i.Char, i.IgnoreCase);
		REQUIRE(Src == i.Result);
	}
}

TEST_CASE("wrapped_text")
{
	static const struct tests
	{
		string_view Src;
		size_t Width;
		std::initializer_list<const string_view> Result;
	}
	Tests[]
	{
		{ {}, 1, {
			}
		},
		{ L"AB\nCD"sv, 0, {
			L"AB"sv,
			L"CD"sv,
		}},
		{ L"12345"sv, 1, {
			L"1"sv,
			L"2"sv,
			L"3"sv,
			L"4"sv,
			L"5"sv,
		}},
		{ L"12345-67890,ABCDE"sv, 10, {
			L"12345-"sv,
			L"67890,"sv,
			L"ABCDE"sv,
		}},
		{ L"Supercalifragilisticexpialidocious"sv, 5, {
			L"Super"sv,
			L"calif"sv,
			L"ragil"sv,
			L"istic"sv,
			L"expia"sv,
			L"lidoc"sv,
			L"ious"sv,
		}},
		{ L"Dale a tu cuerpo alegría Macarena\nQue tu cuerpo es pa' darle alegría why cosa buena\nDale a tu cuerpo alegría, Macarena\nHey Macarena"sv, 35, {
			L"Dale a tu cuerpo alegría Macarena"sv,
			L"Que tu cuerpo es pa' darle alegría"sv,
			L"why cosa buena"sv,
			L"Dale a tu cuerpo alegría, Macarena"sv,
			L"Hey Macarena"sv,
		}},
		{ L"I used to wonder what friendship could be\nUntil you all shared its magic with me"sv, 2000, {
			L"I used to wonder what friendship could be"sv,
			L"Until you all shared its magic with me"sv,
		}},
		{ L"Rah, rah, ah, ah, ah, roma, roma, ma. Gaga, ooh, la, la"sv, 10, {
			L"Rah, rah,"sv,
			L"ah, ah,"sv,
			L"ah, roma,"sv,
			L"roma, ma."sv,
			L"Gaga, ooh,"sv,
			L"la, la"sv,
		}},
		{ L"Ma-i-a hi\nMa-i-a hu\nMa-i-a ho\nMa-i-a ha-ha"sv, 3, {
			L"Ma-"sv,
			L"i-a"sv,
			L"hi"sv,
			L"Ma-"sv,
			L"i-a"sv,
			L"hu"sv,
			L"Ma-"sv,
			L"i-a"sv,
			L"ho"sv,
			L"Ma-"sv,
			L"i-a"sv,
			L"ha-"sv,
			L"ha"sv,
		}},
	};

	for (const auto& Test: Tests)
	{
		auto Iterator = Test.Result.begin();
		for (const auto& i: wrapped_text(Test.Src, Test.Width))
		{
			REQUIRE(Iterator != Test.Result.end());

			if (Test.Width)
				REQUIRE(i.size() <= Test.Width);

			REQUIRE(i == *Iterator);
			++Iterator;
		}

		REQUIRE(Iterator == Test.Result.end());
	}
}

TEST_CASE("truncate")
{
	static const struct tests
	{
		string_view Src;
		struct size
		{
			size_t Size;
			string_view ResultLeft, ResultCenter, ResultRight, ResultPath;
		};
		std::initializer_list<size> Sizes;
	}
	Tests[]
	{
		{ {}, {
			{ 0,  {},               {},               {},               {},              },
			{ 1,  {},               {},               {},               {},              },
			{ 2,  {},               {},               {},               {},              },
			{ 3,  {},               {},               {},               {},              },
			{ 4,  {},               {},               {},               {},              },
		}},
		{ L"0"sv, {
			{ 0,  {},               {},               {},               {},              },
			{ 1,  L"0"sv,           L"0"sv,           L"0"sv,           L"0"sv,          },
			{ 2,  L"0"sv,           L"0"sv,           L"0"sv,           L"0"sv,          },
		}},
		{ L"01"sv, {
			{ 0,  {},               {},               {},               {},              },
			{ 1,  L"…"sv,           L"…"sv,           L"…"sv,           L"…"sv,          },
			{ 2,  L"01"sv,          L"01"sv,          L"01"sv,          L"01"sv,         },
			{ 3,  L"01"sv,          L"01"sv,          L"01"sv,          L"01"sv,         },
		}},
		{ L"012"sv, {
			{ 0,  {},               {},               {},               {},              },
			{ 1,  L"…"sv,           L"…"sv,           L"…"sv,           L"…"sv,          },
			{ 2,  L"…2"sv,          L"…2"sv,          L"0…"sv,          L"…2"sv,         },
			{ 3,  L"012"sv,         L"012"sv,         L"012"sv,         L"012"sv,        },
			{ 4,  L"012"sv,         L"012"sv,         L"012"sv,         L"012"sv,        },
		}},

		{ L"0123"sv, {
			{ 0,  {},               {},               {},               {},              },
			{ 1,  L"…"sv,           L"…"sv,           L"…"sv,           L"…"sv,          },
			{ 2,  L"…3"sv,          L"…3"sv,          L"0…"sv,          L"…3"sv,         },
			{ 3,  L"…23"sv,         L"0…3"sv,         L"01…"sv,         L"…23"sv,        },
			{ 4,  L"0123"sv,        L"0123"sv,        L"0123"sv,        L"0123"sv,       },
			{ 5,  L"0123"sv,        L"0123"sv,        L"0123"sv,        L"0123"sv,       },
		}},
		{ L"0123456789"sv, {
			{ 0,  {},               {},               {},               {},              },
			{ 1,  L"…"sv,           L"…"sv,           L"…"sv,           L"…"sv,          },
			{ 2,  L"…9"sv,          L"…9"sv,          L"0…"sv,          L"…9"sv,         },
			{ 3,  L"…89"sv,         L"0…9"sv,         L"01…"sv,         L"…89"sv,        },
			{ 4,  L"…789"sv,        L"0…89"sv,        L"012…"sv,        L"…789"sv,       },
			{ 5,  L"…6789"sv,       L"01…89"sv,       L"0123…"sv,       L"…6789"sv,      },
			{ 6,  L"…56789"sv,      L"01…789"sv,      L"01234…"sv,      L"…56789"sv,     },
			{ 7,  L"…456789"sv,     L"012…789"sv,     L"012345…"sv,     L"…456789"sv,    },
			{ 8,  L"…3456789"sv,    L"012…6789"sv,    L"0123456…"sv,    L"…3456789"sv,   },
			{ 9,  L"…23456789"sv,   L"0123…6789"sv,   L"01234567…"sv,   L"…23456789"sv,  },
			{ 10, L"0123456789"sv,  L"0123456789"sv,  L"0123456789"sv,  L"0123456789"sv, },
			{ 20, L"0123456789"sv,  L"0123456789"sv,  L"0123456789"sv,  L"0123456789"sv, },
		}},

		{ L"c:/123/456"sv, {
			{ 0,  {},               {},               {},               {},              },
			{ 1,  L"…"sv,           L"…"sv,           L"…"sv,           L"…"sv,          },
			{ 2,  L"…6"sv,          L"…6"sv,          L"c…"sv,          L"c…"sv,         },
			{ 3,  L"…56"sv,         L"c…6"sv,         L"c:…"sv,         L"c:…"sv,        },
			{ 4,  L"…456"sv,        L"c…56"sv,        L"c:/…"sv,        L"c:/…"sv,       },
			{ 5,  L"…/456"sv,       L"c:…56"sv,       L"c:/1…"sv,       L"c:/…6"sv,      },
			{ 6,  L"…3/456"sv,      L"c:…456"sv,      L"c:/12…"sv,      L"c:/…56"sv,     },
			{ 7,  L"…23/456"sv,     L"c:/…456"sv,     L"c:/123…"sv,     L"c:/…456"sv,    },
			{ 8,  L"…123/456"sv,    L"c:/…/456"sv,    L"c:/123/…"sv,    L"c:/…/456"sv,   },
			{ 9,  L"…/123/456"sv,   L"c:/1…/456"sv,   L"c:/123/4…"sv,   L"c:/…3/456"sv,  },
			{ 10, L"c:/123/456"sv,  L"c:/123/456"sv,  L"c:/123/456"sv,  L"c:/123/456"sv, },
			{ 20, L"c:/123/456"sv,  L"c:/123/456"sv,  L"c:/123/456"sv,  L"c:/123/456"sv, },
		}},
	};

	static const struct
	{
		string(*Truncate)(string_view, size_t);
		wchar_t*(*TruncateLegacy)(wchar_t*, int);
		string_view tests::size::*StrAccessor;
	}
	Functions[]
	{
		{ truncate_left,   legacy::truncate_left,   &tests::size::ResultLeft   },
		{ truncate_center, {},                      &tests::size::ResultCenter },
		{ truncate_right,  {},                      &tests::size::ResultRight },
		{ truncate_path,   legacy::truncate_path,   &tests::size::ResultPath   },
	};

	for (const auto& i: Tests)
	{
		for (const auto& Size: i.Sizes)
		{
			for (const auto& f: Functions)
			{
				const auto Baseline = std::invoke(f.StrAccessor, Size);

				REQUIRE(f.Truncate(string(i.Src), Size.Size) == Baseline);

				if (f.TruncateLegacy)
				{
					string Buffer(i.Src);
					REQUIRE(f.TruncateLegacy(Buffer.data(), static_cast<int>(Size.Size)) == Baseline);
				}
			}
		}
	}
}

TEST_CASE("IsCaseMixed")
{
	static const struct
	{
		string_view Src;
		bool Result;
	}
	Tests[]
	{
		{ {},             false },
		{ L"123"sv,       false },
		{ L"FUBAR"sv,     false },
		{ L"burrito"sv,   false },
		{ L"CamelCase"sv, true  },
		{ L"sPoNgEbOb"sv, true  },
		{ L"12345Nz67"sv, true  },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Result == IsCaseMixed(i.Src));
	}
}

TEST_CASE("hex")
{
	static const struct
	{
		string_view Src, Numbers;
		bytes_view Bytes;
	}
	Tests[]
	{
		{ {},            {},          {},                },
		{ L" "sv,        {},          {},                },
		{ L"  "sv,       {},          {},                },
		{ L"12 "sv,      L"12"sv,     "\x12"_bv,         },
		{ L"12 3"sv,     L"1203"sv,   "\x12\x03"_bv,     },
		{ L"12 34"sv,    L"1234"sv,   "\x12\x34"_bv,     },
		{ L"12 34 56"sv, L"123456"sv, "\x12\x34\x56"_bv, },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(ExtractHexString(i.Src) == i.Numbers);
		REQUIRE(HexStringToBlob(i.Numbers, 0) == i.Bytes);
		REQUIRE(BlobToHexString(i.Bytes, 0) == i.Numbers);
	}
}

TEST_CASE("hex.mask")
{
	REQUIRE(HexMask(1) == L"HH"sv);
	REQUIRE(HexMask(2) == L"HH HH"sv);
	REQUIRE(HexMask(3) == L"HH HH HH"sv);
}

TEST_CASE("xwcsncpy")
{
	static const struct
	{
		string_view Src;
	}
	Tests[]
	{
		{ L""sv,      },
		{ L"1"sv,     },
		{ L"12"sv,    },
		{ L"123"sv,   },
		{ L"1234"sv,  },
		{ L"12345"sv, },
	};

	const auto MaxBufferSize = std::ranges::fold_left(Tests, 0uz, [](size_t const Value, auto const& Item){ return std::max(Value, Item.Src.size()); }) + 1;

	for (const auto BufferSize: std::views::iota(0uz, MaxBufferSize + 1))
	{
		for (const auto& i: Tests)
		{
			wchar_t Buffer[10];
			assert(std::size(Buffer) >= BufferSize);
			xwcsncpy(Buffer, i.Src.data(), BufferSize);
			const auto ResultSize = BufferSize? std::min(i.Src.size(), BufferSize - 1) : 0;
			REQUIRE(std::equal(i.Src.cbegin(), i.Src.cbegin() + ResultSize, Buffer, Buffer + ResultSize));
			REQUIRE(Buffer[ResultSize] == L'\0');
		}
	}
}
#endif
