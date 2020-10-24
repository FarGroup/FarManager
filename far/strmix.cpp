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
#include "stddlg.hpp"
#include "encoding.hpp"
#include "regex_helpers.hpp"
#include "string_utils.hpp"
#include "exception.hpp"
#include "global.hpp"

// Platform:

// Common:
#include "common/bytes_view.hpp"
#include "common/from_string.hpp"
#include "common/function_ref.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

string GroupDigits(unsigned long long Value)
{
	NUMBERFMT Fmt{};

	// Not needed - can't be decimal
	Fmt.NumDigits = 0;
	// Don't care - can't be decimal
	Fmt.LeadingZero = 1;

	Fmt.Grouping = locale.digits_grouping();

	wchar_t DecimalSeparator[]{ locale.decimal_separator(), L'\0' };
	Fmt.lpDecimalSep = DecimalSeparator;

	wchar_t ThousandSeparator[]{ locale.thousand_separator(), L'\0' };
	Fmt.lpThousandSep = ThousandSeparator;

	// Don't care - can't be negative
	Fmt.NegativeOrder = 1;

	auto Src = str(Value);
	string Result;

	if (os::detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, Result, [&](span<wchar_t> Buffer)
	{
		const size_t Size = GetNumberFormat(LOCALE_USER_DEFAULT, 0, Src.c_str(), &Fmt, Buffer.data(), static_cast<int>(Buffer.size()));
		return Size? Size - 1 : 0;
	}))
		return Result;

	// Better than nothing
	return Src;
}

wchar_t* legacy::InsertQuotes(wchar_t *Str)
{
	const auto QuoteChar = L'"';
	size_t l = wcslen(Str);

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

wchar_t* legacy::QuoteSpace(wchar_t *Str)
{
	if (Global->Opt->strQuotedSymbols.Get().find_first_of(Str) != string::npos)
		InsertQuotes(Str);

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

string &QuoteSpace(string &strStr)
{
	if (strStr.find_first_of(Global->Opt->strQuotedSymbols.Get()) != string::npos)
		inplace::quote(strStr);

	return strStr;
}

wchar_t* legacy::QuoteSpaceOnly(wchar_t* Str)
{
	if (contains(Str, L' '))
		InsertQuotes(Str);

	return Str;
}

void inplace::QuoteOuterSpace(string &strStr)
{
	if (!strStr.empty() && (strStr.front() == L' ' || strStr.back() == L' '))
		inplace::quote(strStr);
}

// TODO: "…" is displayed as "." in raster fonts. Make it lng-customisable?
static const auto Dots = L"…"sv;

static auto legacy_operation(wchar_t* Str, int MaxLength, function_ref<void(span<wchar_t>, size_t, string_view)> const Handler)
{
	assert(MaxLength >= 0);
	const size_t Max = std::max(0, MaxLength);

	if (!Str || !*Str)
		return Str;

	const auto Size = wcslen(Str);

	if (Size <= Max)
		return Str;

	Handler({ Str, Size }, Max, Dots.substr(0, Max));
	return Str;
}

wchar_t* legacy::truncate_right(wchar_t *Str, int MaxLength)
{
	return legacy_operation(Str, MaxLength, [](span<wchar_t> const StrParam, size_t const MaxLengthParam, string_view const CurrentDots)
	{
		*copy_string(CurrentDots, StrParam.data() + MaxLengthParam - CurrentDots.size()) = {};
	});
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
	return legacy_operation(Str, MaxLength, [](span<wchar_t> const StrParam, size_t const MaxLengthParam, string_view const CurrentDots)
	{
		const auto Iterator = copy_string(CurrentDots, StrParam.begin());

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

wchar_t* legacy::truncate_center(wchar_t *Str, int MaxLength)
{
	return legacy_operation(Str, MaxLength, [](span<wchar_t> const StrParam, size_t const MaxLengthParam, string_view const CurrentDots)
	{
		const auto Iterator = copy_string(CurrentDots, StrParam.data() + (MaxLengthParam - CurrentDots.size()) / 2);

		const auto StrEnd = StrParam.end();
		const auto StrBegin = Iterator + (StrParam.size() - MaxLengthParam);

		*std::copy(StrBegin, StrEnd, Iterator) = {};
	});
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
	return legacy_operation(Str, MaxLength, [](span<wchar_t> const StrParam, size_t const MaxLengthParam, string_view const CurrentDots)
	{
		const auto Offset = std::min(StartOffset(StrParam.data()), MaxLengthParam - CurrentDots.size());

		const auto Iterator = copy_string(CurrentDots, StrParam.begin() + Offset);

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
	const auto AlphaBegin = std::find_if(ALL_CONST_RANGE(Str), is_alpha);
	if (AlphaBegin == Str.cend())
		return false;

	const auto Case = is_lower(*AlphaBegin);
	return std::any_of(AlphaBegin, Str.cend(), [Case](wchar_t c){ return is_alpha(c) && is_lower(c) != Case; });
}

/* FileSizeToStr()
   Форматирование размера файла в удобочитаемый вид.
*/

static const unsigned long long BytesInUnit[][2]
{
	{0x0000000000000001ull,                   1ull}, // B
	{0x0000000000000400ull,                1000ull}, // KiB / KB
	{0x0000000000100000ull,             1000000ull}, // MiB / MB
	{0x0000000040000000ull,          1000000000ull}, // GiB / GB
	{0x0000010000000000ull,       1000000000000ull}, // TiB / TB
	{0x0004000000000000ull,    1000000000000000ull}, // PiB / PB
	{0x1000000000000000ull, 1000000000000000000ull}, // EiB / EB
};

static const unsigned long long PrecisionMultiplier[]
{
	  1ull,
	 10ull,
	100ull,
};


static string& UnitStr(size_t Unit, bool Binary)
{
	static string Data[std::size(BytesInUnit)][2];
	return Data[Unit][Binary? 0 : 1];
}

void PrepareUnitStr()
{
	for (size_t i = 0; i != std::size(BytesInUnit); ++i)
	{
		UnitStr(i, true) = upper(msg(lng::MListBytes + i));
		UnitStr(i, false) = lower(msg(lng::MListBytes + i));
	}
}

string FileSizeToStr(unsigned long long FileSize, int WidthWithSign, unsigned long long ViewFlags)
{
	// подготовительные мероприятия
	if (UnitStr(0, false) != lower(msg(lng::MListBytes)))
	{
		PrepareUnitStr();
	}

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
		binary_index = 0,
		decimal_index = 1,
		log2_of_2014 = 10,
		log10_of_1000 = 3;

	constexpr std::pair
		BinaryDivider(binary_index, log2_of_2014),
		DecimalDivider(decimal_index, log10_of_1000);

	const auto& Divider = ViewFlags & COLFLAGS_THOUSAND? DecimalDivider : BinaryDivider;

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

			(LeftAlign? inplace::cut_right : inplace::cut_left)(Str, Width - 1);
			Str.insert(LeftAlign? Str.end() : Str.begin(), L'\x2026');
			return Str;
		};

		if (!UnitIndex && !ShowUnit)
			return FitToWidth(std::move(StrSize));

		return FitToWidth(concat(StrSize, UseCompact? L""sv : L" "sv, UnitStr(UnitIndex, UseBinaryUnit).front()));
	};

	if (UseFloatSize)
	{
		const auto Numerator = FileSize? (ViewFlags & COLFLAGS_THOUSAND)? std::log10(FileSize) : std::log2(FileSize) : 0;
		const size_t UnitIndex = Numerator / Divider.second;

		string Str;

		if (!UnitIndex)
		{
			Str = str(FileSize);
		}
		else
		{
			const auto Denominator = BytesInUnit[UnitIndex][Divider.first];
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

				Str = concat(str(IntegralPart), locale.decimal_separator(), pad_left(str(FractionalPart), NumDigits, L'0'));
			}
			else
			{
				Str = str(static_cast<unsigned long long>(std::round(static_cast<double>(RawIntegral) + RawFractional)));
			}
		}

		return FormatSize(std::move(Str), UnitIndex);
	}

	const auto ToStr = [UseGroupDigits](auto Size)
	{
		return UseGroupDigits? GroupDigits(Size) : str(Size);
	};

	size_t UnitIndex = 0;
	auto Str = ToStr(FileSize);

	const auto SuffixSize = (ShowUnit || (Width && Str.size() > Width))? UseCompact? 1u : 2u : 0u;

	const auto MaxNumberWidth = Width > SuffixSize? Width - SuffixSize : 0;

	while ((UseUnit && UnitIndex < MinUnit) || (Width && Str.size() > MaxNumberWidth))
	{
		const auto Denominator = BytesInUnit[UnitIndex + 1][Divider.first];
		const auto IntegralPart = FileSize / Denominator;
		const auto FractionalPart = static_cast<double>(FileSize % Denominator) / static_cast<double>(Denominator);
		const auto SizeInUnits = IntegralPart + static_cast<unsigned long long>(std::round(FractionalPart));

		if (SizeInUnits)
		{
			++UnitIndex;
			Str = ToStr(SizeInUnits);
		}
		else
			break;
	}

	return FormatSize(std::move(Str), UnitIndex);
}


// Заменить в строке Str Count вхождений подстроки FindStr на подстроку ReplStr
// Если Count == npos - заменять "до полной победы"
bool ReplaceStrings(string& strStr, const string_view FindStr, const string_view ReplStr, const bool IgnoreCase, size_t Count)
{
	if (strStr.empty() || FindStr.empty() || !Count)
		return false;

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
	const auto NewEnd = IgnoreCase?
		std::unique(ALL_RANGE(Str), [Char, Eq = equal_icase_t{}](wchar_t const First, wchar_t const Second){ return Eq(First, Char) && Eq(Second, Char); }) :
		std::unique(ALL_RANGE(Str), [Char](wchar_t const First, wchar_t const Second){ return First == Char && Second == Char; });

	Str.resize(NewEnd - Str.begin());
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

	const auto WordDiv = concat(WordDiv0, GetSpaces(), GetEols());

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
	static const std::wregex SizeRegex(RE_BEGIN RE_ANY_OF(L"0-9") RE_ONE_OR_MORE_LAZY RE_ANY_OF(L"BKMGTPE") RE_ZERO_OR_ONE_GREEDY RE_END, std::regex::icase | std::regex::optimize);
	return std::regex_search(ALL_CONST_RANGE(FileSizeStr), SizeRegex);
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

namespace
{
	string ReplaceBrackets(
		const string_view SearchStr,
		const string_view ReplaceStr,
		const RegExpMatch* Match,
		size_t Count,
		const MatchHash* HMatch,
		int& CurPos,
		int* SearchLength)
	{
		string result;
		for (size_t i = 0, length = ReplaceStr.size(); i < length; ++i)
		{
			const auto CurrentChar = ReplaceStr[i];
			bool common = true;

			if (CurrentChar == L'$')
			{
				const auto TokenStart = i + 1;

				if (TokenStart < length)
				{
					intptr_t start = 0, end = 0;
					size_t ShiftLength = 0;
					auto TokenEnd = TokenStart;
					bool Success = false;

					while (TokenEnd != length && std::iswdigit(ReplaceStr[TokenEnd]))
					{
						++TokenEnd;
					}

					if (TokenEnd != TokenStart)
					{
						size_t index = 0;
						while (TokenEnd != TokenStart && (index = from_string<unsigned long>(ReplaceStr.substr(TokenStart, TokenEnd - TokenStart))) >= Count)
						{
							--TokenEnd;
						}

						if (TokenEnd != TokenStart)
						{
							Success = true;
							start = Match[index].start;
							end = Match[index].end;
							ShiftLength = TokenEnd - TokenStart;
						}
					}
					else
					{
						static const std::wregex re(RE_BEGIN RE_ESCAPE(L"{") RE_C_GROUP(RE_ANY_OF(L"\\w\\s") RE_ZERO_OR_MORE_LAZY) RE_ESCAPE(L"}"), std::regex::optimize);
						std::wcmatch CMatch;
						if (std::regex_search(ReplaceStr.data() + TokenStart, ReplaceStr.data() + (ReplaceStr.size() - TokenStart), CMatch, re))
						{
							ShiftLength = CMatch[0].length();
							if (HMatch)
							{
								const auto Iterator = HMatch->Matches.find(string(CMatch[1].first, CMatch[1].second));
								if (Iterator != HMatch->Matches.cend())
								{
									Success = true;
									start = Iterator->second.start;
									end = Iterator->second.end;
								}
							}
						}
					}

					if (ShiftLength)
					{
						i += ShiftLength;
						common = false;

						if (Success)
						{
							result.append(SearchStr.data() + start, end - start);
						}
					}
				}
			}

			if (common)
			{
				result += CurrentChar;
			}
		}

		*SearchLength = Match->end - Match->start;
		CurPos = Match->start;
		return result;
	}

	bool SearchStringRegex(
		string_view const Source,
		const RegExp& re,
		RegExpMatch* const pm,
		MatchHash* const hm,
		intptr_t Position,
		int const Reverse,
		string& ReplaceStr,
		int& CurPos,
		int* SearchLength)
	{
		intptr_t n = re.GetBracketsCount();

		if (!Reverse)
		{
			if (re.SearchEx(Source, Position, pm, n, hm))
			{
				ReplaceStr = ReplaceBrackets(Source, ReplaceStr, pm, n, hm, CurPos, SearchLength);
				return true;
			}

			ReMatchErrorMessage(re);
			return false;
		}

		bool found = false;
		intptr_t half = 0;
		intptr_t pos = 0;

		for (;;)
		{
			if (!re.SearchEx(Source, pos, pm + half, n, hm))
			{
				ReMatchErrorMessage(re);
				break;
			}
			pos = pm[half].start;
			if (pos > Position)
				break;

			found = true;
			++pos;
			half = n - half;
		}

		if (found)
		{
			half = n - half;
			ReplaceStr = ReplaceBrackets(Source, ReplaceStr, pm + half, n, hm, CurPos, SearchLength);
		}

		return found;
	}
}

static bool CanContainWholeWord(string_view const Haystack, size_t const Offset, size_t const NeedleSize, string_view const WordDiv)
{
	const auto BlankOrWordDiv = [&WordDiv](wchar_t Ch)
	{
		return std::iswblank(Ch) || contains(WordDiv, Ch);
	};

	if (Offset && !BlankOrWordDiv(Haystack[Offset - 1]))
		return false;

	if (Offset + NeedleSize < Haystack.size() && !BlankOrWordDiv(Haystack[Offset + NeedleSize]))
		return false;

	return true;
}

bool SearchString(
	string_view const Haystack,
	string_view const Needle,
	string_view const NeedleUpper,
	string_view const NeedleLower,
	const RegExp& re,
	RegExpMatch* const pm,
	MatchHash* const hm,
	int& CurPos,
	bool const Case,
	bool const WholeWords,
	bool const Reverse,
	bool const Regexp,
	int* const SearchLength,
	string_view WordDiv)
{
	string Dummy;
	return SearchAndReplaceString(
		Haystack,
		Needle,
		NeedleUpper,
		NeedleLower,
		re,
		pm,
		hm,
		Dummy,
		CurPos,
		Case,
		WholeWords,
		Reverse,
		Regexp,
		false,
		SearchLength,
		WordDiv
	);
}

bool SearchAndReplaceString(
	string_view const Haystack,
	string_view const Needle,
	string_view const NeedleUpper,
	string_view const NeedleLower,
	const RegExp& re,
	RegExpMatch* const pm,
	MatchHash* const hm,
	string& ReplaceStr,
	int& CurPos,
	bool const Case,
	bool const WholeWords,
	bool const Reverse,
	bool const Regexp,
	bool const PreserveStyle,
	int* const SearchLength,
	string_view WordDiv)
{
	*SearchLength = 0;

	if (WordDiv.empty())
		WordDiv = Global->Opt->strWordDiv;

	if (!Regexp && PreserveStyle && PreserveStyleReplaceString(Haystack, Needle, ReplaceStr, CurPos, Case, WholeWords, WordDiv, Reverse, *SearchLength))
		return true;

	if (Needle.empty())
		return true;

	auto Position = CurPos;
	const auto HaystackSize = static_cast<int>(Haystack.size());

	if (Reverse)
	{
		// MZK 2018-04-01 BUGBUG: regex reverse search: "^$" does not match empty string
		Position = std::min(Position - 1, HaystackSize - 1);

		if (Position < 0)
			return false;
	}

	if (Regexp)
	{
		// Empty Haystack is ok for regex search, e.g. ^$
		if ((Position || HaystackSize) && Position >= HaystackSize)
			return false;

		return SearchStringRegex(Haystack, re, pm, hm, Position, Reverse, ReplaceStr, CurPos, SearchLength);
	}

	if (Position >= HaystackSize)
		return false;

	const auto NeedleSize = *SearchLength = static_cast<int>(Needle.size());

	for (int HaystackIndex = Position; HaystackIndex != -1 && HaystackIndex != HaystackSize; Reverse? --HaystackIndex : ++HaystackIndex)
	{
		if (WholeWords && !CanContainWholeWord(Haystack, HaystackIndex, NeedleSize, WordDiv))
			continue;

		for (size_t NeedleIndex = 0; ; ++NeedleIndex)
		{
			if (NeedleIndex == Needle.size())
			{
				CurPos = HaystackIndex;

				// В случае PreserveStyle: если не получилось сделать замену c помощью PreserveStyleReplaceString,
				// то хотя бы сохранить регистр первой буквы.
				if (PreserveStyle && !ReplaceStr.empty() && is_alpha(ReplaceStr.front()) && is_alpha(Haystack[HaystackIndex]))
				{
					if (is_upper(Haystack[HaystackIndex]))
						ReplaceStr.front() = ::upper(ReplaceStr.front());
					else if (is_lower(Haystack[HaystackIndex]))
						ReplaceStr.front() = ::lower(ReplaceStr.front());
				}

				return true;
			}

			if (HaystackIndex + NeedleIndex == Haystack.size())
				break;

			const auto Ch = Haystack[HaystackIndex + NeedleIndex];

			if (Case)
			{
				if (Ch != Needle[NeedleIndex])
					break;
			}
			else
			{
				if (Ch != NeedleUpper[NeedleIndex] && Ch != NeedleLower[NeedleIndex])
					break;
			}
		}
	}

	return false;
}

char IntToHex(int h)
{
	if (h > 0xF)
		throw MAKE_FAR_FATAL_EXCEPTION(L"Not a hex char"sv);
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

	throw MAKE_FAR_FATAL_EXCEPTION(L"Not a hex char"sv);
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
		throw MAKE_FAR_FATAL_EXCEPTION(L"Incomplete hex string"sv);

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
		Blob.push_back(std::byte(HexToInt(Hex[i]) << 4 | HexToInt(Hex[i + 1])));
	}

	return Blob;
}

string ExtractHexString(string_view const HexString)
{
	const auto Trimmed = trim_right(HexString);
	string Result;
	Result.reserve((Trimmed.size() + 2) / 3 * 2);
	// TODO: Fix these and trailing spaces in Dialog class?
	std::remove_copy(ALL_CONST_RANGE(Trimmed), std::back_inserter(Result), L' ');
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
	const auto CompatibleCp = IsVirtualCodePage(Codepage)? CP_ACP : Codepage;
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

TEST_CASE("ConvertFileSizeString")
{
	constexpr auto
		B = 1ull,
		K = B * 1024,
		M = K * 1024,
		G = M * 1024,
		T = G * 1024,
		P = T * 1024,
		E = P * 1024;

	static const struct
	{
		string_view Src;
		uint64_t Result;
	}
	Tests[]
	{
		{ {},           0     },
		{ {},           0     },
		{ L"Beep"sv,    0     },
		{ L"0"sv,       0 * B },
		{ L"1"sv,       1 * B },
		{ L"32K"sv,    32 * K },
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
		size_t Size;
		string_view ResultLeft, ResultCenter, ResultRight, ResultPath;
	}
	Tests[]
	{
		{ {},              0,  {},               {},               {},               {},              },
		{ {},              1,  {},               {},               {},               {},              },
		{ {},              2,  {},               {},               {},               {},              },
		{ {},              3,  {},               {},               {},               {},              },
		{ {},              4,  {},               {},               {},               {},              },

		{ L"0"sv,          0,  {},               {},               {},               {},              },
		{ L"0"sv,          1,  L"0"sv,           L"0"sv,           L"0"sv,           L"0"sv,          },
		{ L"0"sv,          2,  L"0"sv,           L"0"sv,           L"0"sv,           L"0"sv,          },

		{ L"01"sv,         0,  {},               {},               {},               {},              },
		{ L"01"sv,         1,  L"…"sv,           L"…"sv,           L"…"sv,           L"…"sv,          },
		{ L"01"sv,         2,  L"01"sv,          L"01"sv,          L"01"sv,          L"01"sv,         },
		{ L"01"sv,         3,  L"01"sv,          L"01"sv,          L"01"sv,          L"01"sv,         },

		{ L"012"sv,        0,  {},               {},               {},               {},              },
		{ L"012"sv,        1,  L"…"sv,           L"…"sv,           L"…"sv,           L"…"sv,          },
		{ L"012"sv,        2,  L"…2"sv,          L"…2"sv,          L"0…"sv,          L"…2"sv,         },
		{ L"012"sv,        3,  L"012"sv,         L"012"sv,         L"012"sv,         L"012"sv,        },
		{ L"012"sv,        4,  L"012"sv,         L"012"sv,         L"012"sv,         L"012"sv,        },

		{ L"0123"sv,       0,  {},               {},               {},               {},              },
		{ L"0123"sv,       1,  L"…"sv,           L"…"sv,           L"…"sv,           L"…"sv,          },
		{ L"0123"sv,       2,  L"…3"sv,          L"…3"sv,          L"0…"sv,          L"…3"sv,         },
		{ L"0123"sv,       3,  L"…23"sv,         L"0…3"sv,         L"01…"sv,         L"…23"sv,        },
		{ L"0123"sv,       4,  L"0123"sv,        L"0123"sv,        L"0123"sv,        L"0123"sv,       },
		{ L"0123"sv,       5,  L"0123"sv,        L"0123"sv,        L"0123"sv,        L"0123"sv,       },

		{ L"0123456789"sv, 0,  {},               {},               {},               {},              },
		{ L"0123456789"sv, 1,  L"…"sv,           L"…"sv,           L"…"sv,           L"…"sv,          },
		{ L"0123456789"sv, 2,  L"…9"sv,          L"…9"sv,          L"0…"sv,          L"…9"sv,         },
		{ L"0123456789"sv, 3,  L"…89"sv,         L"0…9"sv,         L"01…"sv,         L"…89"sv,        },
		{ L"0123456789"sv, 4,  L"…789"sv,        L"0…89"sv,        L"012…"sv,        L"…789"sv,       },
		{ L"0123456789"sv, 5,  L"…6789"sv,       L"01…89"sv,       L"0123…"sv,       L"…6789"sv,      },
		{ L"0123456789"sv, 6,  L"…56789"sv,      L"01…789"sv,      L"01234…"sv,      L"…56789"sv,     },
		{ L"0123456789"sv, 7,  L"…456789"sv,     L"012…789"sv,     L"012345…"sv,     L"…456789"sv,    },
		{ L"0123456789"sv, 8,  L"…3456789"sv,    L"012…6789"sv,    L"0123456…"sv,    L"…3456789"sv,   },
		{ L"0123456789"sv, 9,  L"…23456789"sv,   L"0123…6789"sv,   L"01234567…"sv,   L"…23456789"sv,  },
		{ L"0123456789"sv, 10, L"0123456789"sv,  L"0123456789"sv,  L"0123456789"sv,  L"0123456789"sv, },
		{ L"0123456789"sv, 20, L"0123456789"sv,  L"0123456789"sv,  L"0123456789"sv,  L"0123456789"sv, },

		{ L"c:/123/456"sv, 0,  {},               {},               {},               {},              },
		{ L"c:/123/456"sv, 1,  L"…"sv,           L"…"sv,           L"…"sv,           L"…"sv,          },
		{ L"c:/123/456"sv, 2,  L"…6"sv,          L"…6"sv,          L"c…"sv,          L"c…"sv,         },
		{ L"c:/123/456"sv, 3,  L"…56"sv,         L"c…6"sv,         L"c:…"sv,         L"c:…"sv,        },
		{ L"c:/123/456"sv, 4,  L"…456"sv,        L"c…56"sv,        L"c:/…"sv,        L"c:/…"sv,       },
		{ L"c:/123/456"sv, 5,  L"…/456"sv,       L"c:…56"sv,       L"c:/1…"sv,       L"c:/…6"sv,      },
		{ L"c:/123/456"sv, 6,  L"…3/456"sv,      L"c:…456"sv,      L"c:/12…"sv,      L"c:/…56"sv,     },
		{ L"c:/123/456"sv, 7,  L"…23/456"sv,     L"c:/…456"sv,     L"c:/123…"sv,     L"c:/…456"sv,    },
		{ L"c:/123/456"sv, 8,  L"…123/456"sv,    L"c:/…/456"sv,    L"c:/123/…"sv,    L"c:/…/456"sv,   },
		{ L"c:/123/456"sv, 9,  L"…/123/456"sv,   L"c:/1…/456"sv,   L"c:/123/4…"sv,   L"c:/…3/456"sv,  },
		{ L"c:/123/456"sv, 10, L"c:/123/456"sv,  L"c:/123/456"sv,  L"c:/123/456"sv,  L"c:/123/456"sv, },
		{ L"c:/123/456"sv, 20, L"c:/123/456"sv,  L"c:/123/456"sv,  L"c:/123/456"sv,  L"c:/123/456"sv, },
	};

	using handler = string(string_view, size_t);
	using legacy_handler = wchar_t*(wchar_t*, int);
	using result_ptr = decltype(&tests::ResultLeft);
	using tp = std::tuple<handler*, legacy_handler*, result_ptr>;

	static const std::array Functions
	{
		tp{ truncate_left,   legacy::truncate_left,   &tests::ResultLeft   },
		tp{ truncate_center, legacy::truncate_center, &tests::ResultCenter },
		tp{ truncate_right,  legacy::truncate_right,  &tests::ResultRight  },
		tp{ truncate_path,   legacy::truncate_path,   &tests::ResultPath   },
	};

	for (const auto& i: Tests)
	{
		for (const auto& [Truncate, TruncateLegacy, StrAccessor]: Functions)
		{
			const auto Baseline = std::invoke(StrAccessor, i);

			REQUIRE(Truncate(string(i.Src), i.Size) == Baseline);

			string Buffer(i.Src);
			REQUIRE(TruncateLegacy(Buffer.data(), static_cast<int>(i.Size)) == Baseline);
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
		REQUIRE(BlobToHexString(view_bytes(i.Bytes), 0) == i.Numbers);
	}
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

	const auto MaxBufferSize = std::max_element(ALL_CONST_RANGE(Tests), [](const auto& a, const auto& b){ return a.Src.size() < b.Src.size(); })->Src.size() + 1;

	for (size_t BufferSize = 0; BufferSize != MaxBufferSize + 1; ++BufferSize)
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
