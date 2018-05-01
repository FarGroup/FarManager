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

#include "RegExp.hpp"
#include "strmix.hpp"
#include "lang.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "preservestyle.hpp"
#include "locale.hpp"
#include "stddlg.hpp"
#include "encoding.hpp"
#include "regex_helpers.hpp"
#include "string_utils.hpp"
#include "bitflags.hpp"
#include "exception.hpp"

string GroupDigits(unsigned long long Value)
{
	NUMBERFMT Fmt{};

	wchar_t DecimalSeparator[] { locale::GetDecimalSeparator(), L'\0' };
	wchar_t ThousandSeparator[] { locale::GetThousandSeparator(), L'\0' };

	// TODO pick regional settings
	Fmt.NumDigits = 0;
	Fmt.LeadingZero = 1;
	Fmt.Grouping = 3;
	Fmt.lpDecimalSep = DecimalSeparator;
	Fmt.lpThousandSep = ThousandSeparator;
	Fmt.NegativeOrder = 1;

	string strSrc = str(Value);
	const size_t Size = GetNumberFormat(LOCALE_USER_DEFAULT, 0, strSrc.c_str(), &Fmt, nullptr, 0);
	wchar_t_ptr_n<MAX_PATH> Dest(Size);
	GetNumberFormat(LOCALE_USER_DEFAULT, 0, strSrc.c_str(), &Fmt, Dest.get(), static_cast<int>(Size));
	return { Dest.get(), Size - 1 };
}

wchar_t* InsertQuote(wchar_t *Str)
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

wchar_t* QuoteSpace(wchar_t *Str)
{
	if (Global->Opt->strQuotedSymbols.Get().find_first_of(Str) != string::npos)
	{
		InsertQuote(Str);

		// forward slash can't harm the quotation mark, but consistency is preferable
		const auto Size = wcslen(Str);
		if (IsSlash(Str[Size - 2]))
		{
			using std::swap;
			swap(Str[Size - 2], Str[Size - 1]);
		}
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

string &QuoteSpace(string &strStr)
{
	if (strStr.find_first_of(Global->Opt->strQuotedSymbols) != string::npos)
	{
		inplace::quote(strStr);
		
		// forward slash can't harm the quotation mark, but consistency is preferable
		if (IsSlash(*(strStr.end() - 2)))
		{
			std::iter_swap(strStr.end() - 2, strStr.end() - 1);
		}
	}
	return strStr;
}

wchar_t* QuoteSpaceOnly(wchar_t *Str)
{
	if (wcschr(Str,L' '))
		InsertQuote(Str);

	return Str;
}

string& QuoteSpaceOnly(string &strStr)
{
	if (contains(strStr, L' '))
		inplace::quote(strStr);

	return strStr;
}

string &QuoteOuterSpace(string &strStr)
{
	if (!strStr.empty() && (strStr.front() == L' ' || strStr.back() == L' '))
		inplace::quote(strStr);

	return strStr;
}


static const int DotsLen = 3;

string& TruncStrFromEnd(string &strStr, int maxLength)
{
	assert(maxLength >= 0);
	size_t MaxLength = static_cast<size_t>(std::max(0, maxLength));

	if (strStr.size() > MaxLength)
	{
		strStr.resize(MaxLength);
		if (MaxLength > (size_t)DotsLen)
			strStr.replace(MaxLength-DotsLen, DotsLen, DotsLen, L'.');
	}
	return strStr;
}

wchar_t* TruncStrFromEnd(wchar_t *Str, int MaxLength)
{
	assert(MaxLength >= 0);
	MaxLength=std::max(0, MaxLength);

	if (Str)
	{
		int Length = static_cast<int>(wcslen(Str));

		if (Length > MaxLength)
		{
			if (MaxLength > DotsLen)
				std::fill_n(Str + MaxLength - DotsLen, DotsLen, L'.');

			Str[MaxLength] = '\0';
		}
	}
	return Str;
}

wchar_t* TruncStr(wchar_t *Str, int MaxLength)
{
	assert(MaxLength >= 0);
	MaxLength = std::max(0, MaxLength);

	if (Str)
	{
		int Length = static_cast<int>(wcslen(Str));

		if (Length > MaxLength)
		{
			std::copy_n(Str + Length - MaxLength, MaxLength + 1, Str);
			if (MaxLength > DotsLen)
				std::fill_n(Str, DotsLen, L'.');
		}
	}
	return Str;
}

string& TruncStr(string &strStr, int maxLength)
{
	assert(maxLength >= 0);
	size_t MaxLength = static_cast<size_t>(std::max(0, maxLength));
	size_t Length = strStr.size();

	if (Length > MaxLength)
	{
		strStr = strStr.substr(Length-MaxLength, MaxLength);
		if (MaxLength > (size_t)DotsLen)
			strStr.replace(0, DotsLen, DotsLen, L'.');
	}
	return strStr;
}

wchar_t* TruncStrFromCenter(wchar_t *Str, int MaxLength)
{
	assert(MaxLength >= 0);
	MaxLength=std::max(0, MaxLength);

	if (!Str)
		return nullptr;

	const auto Length = static_cast<int>(wcslen(Str));
	if (Length <= MaxLength)
		return Str;

	if (MaxLength > DotsLen)
	{
		int Len1 = (MaxLength - DotsLen) / 2;
		int Len2 = MaxLength - DotsLen - Len1;
		std::copy_n(L"...", DotsLen, Str + Len1);
		std::copy_n(Str + Length - Len2, Len2, Str + Len1 + DotsLen);
	}

	Str[MaxLength] = 0;
	return Str;
}

string& TruncStrFromCenter(string &strStr, int maxLength)
{
	assert(maxLength >= 0);
	size_t MaxLength = static_cast<size_t>(std::max(0, maxLength));
	size_t Length = strStr.size();

	if (Length > MaxLength)
	{
		if (MaxLength > (size_t)DotsLen)
		{
			size_t start = (MaxLength - DotsLen) / 2;
			strStr.replace(start, Length-MaxLength+DotsLen, DotsLen, L'.');
		}
		else
			strStr.resize(MaxLength);
	}
	return strStr;
}

static int StartOffset(const string& Str)
{
	size_t DirOffset = 0;
	ParsePath(Str, &DirOffset);
	return static_cast<int>(DirOffset);
}

wchar_t* TruncPathStr(wchar_t *Str, int MaxLength)
{
	assert(MaxLength >= 0);
	MaxLength = std::max(0, MaxLength);

	if (Str)
	{
		int nLength = static_cast<int>(wcslen(Str));

		if (nLength > MaxLength)
		{
			int start = StartOffset(Str);

			if (!start || start+2+DotsLen > MaxLength)
				return TruncStr(Str, MaxLength);

			std::fill_n(Str + start, DotsLen, L'.');
			wcscpy(Str+start+DotsLen, Str+start+DotsLen+nLength-MaxLength);
		}
	}
	return Str;
}

string& TruncPathStr(string &strStr, int MaxLength)
{
	assert(MaxLength >= 0);
	MaxLength = std::max(0, MaxLength);

	int nLength = static_cast<int>(strStr.size());

	if (nLength > MaxLength)
	{
		int start = StartOffset(strStr);

		if (!start || start+DotsLen+2 > MaxLength)
			return TruncStr(strStr, MaxLength);

		strStr.replace(start, nLength-MaxLength+DotsLen, DotsLen, L'.');
	}
	return strStr;
}

/* $ 02.02.2001 IS
   Заменяет пробелами непечатные символы в строке. В настоящий момент
   обрабатываются только cr и lf.
*/
string& RemoveUnprintableCharacters(string &strStr)
{
	std::replace_if(ALL_RANGE(strStr), IsEol, L' ');
	return inplace::trim(strStr);
}

bool IsCaseMixed(const string_view strSrc)
{
	const auto AlphaBegin = std::find_if(ALL_CONST_RANGE(strSrc), is_alpha);
	if (AlphaBegin == strSrc.cend())
		return false;

	const auto Case = is_lower(*AlphaBegin);
	return std::any_of(AlphaBegin, strSrc.cend(), [Case](wchar_t c){ return is_alpha(c) && is_lower(c) != Case; });
}

/* FileSizeToStr()
   Форматирование размера файла в удобочитаемый вид.
*/
enum
{
	UNIT_COUNT = 7, // byte, kilobyte, megabyte, gigabyte, terabyte, petabyte, exabyte.
};

static string& UnitStr(size_t Unit, bool Binary)
{
	static string Data[UNIT_COUNT][2];
	return Data[Unit][Binary? 1 : 0];
}

void PrepareUnitStr()
{
	for (int i=0; i<UNIT_COUNT; i++)
	{
		UnitStr(i, false) = lower(msg(lng::MListBytes + i));
		UnitStr(i, true) = upper(msg(lng::MListBytes + i));
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
	const bool UseCommas = (ViewFlags & COLUMN_COMMAS) != 0;
	const bool UseFloatSize = (ViewFlags & COLUMN_FLOATSIZE) != 0;
	const bool UseCompact = (ViewFlags & COLUMN_ECONOMIC) != 0;
	const bool UseUnit = (ViewFlags & COLUMN_USE_UNIT) != 0;
	const bool ShowUnit = (ViewFlags & COLUMN_SHOWUNIT) != 0;
	const bool UseBinaryUnit = (ViewFlags & COLUMN_THOUSAND) == 0;
	const size_t MinUnit = (ViewFlags & COLUMN_UNIT_MASK) + 1;

	static const auto BinaryDivider = std::make_pair(1024, std::log(1024));
	static const auto DecimalDivider = std::make_pair(1000, std::log(1000));

	const auto& Divider = ViewFlags & COLUMN_THOUSAND? DecimalDivider : BinaryDivider;

	const auto& FormatSize = [&](string&& StrSize, size_t UnitIndex)
	{
		const auto& FitToWidth = [&](string&& Str)
		{
			if (!Width)
				return Str;

			if (Str.size() <= Width)
				return (LeftAlign? inplace::pad_right : inplace::pad_left)(Str, Width, L' ');

			Str = (LeftAlign? inplace::cut_right : inplace::cut_left)(Str, Width - 1);
			Str.insert(LeftAlign? Str.end() : Str.begin(), L'\x2026');
			return Str;
		};

		if (!UnitIndex && !ShowUnit)
			return FitToWidth(std::move(StrSize));

		return FitToWidth(concat(StrSize, UseCompact? L""_sv : L" "_sv, UnitStr(UnitIndex, UseBinaryUnit).front()));
	};

	if (UseFloatSize)
	{
		const size_t UnitIndex = FileSize? std::log(FileSize) / Divider.second : 0;

		string Str;

		if (!UnitIndex)
		{
			Str = str(FileSize);
		}
		else
		{
			const auto SizeInUnits = FileSize / std::pow(Divider.first, UnitIndex);

			double Parts[2];
			Parts[1] = std::modf(SizeInUnits, &Parts[0]);

			auto Integral = static_cast<int>(Parts[0]);

			const auto FixedPrecision = 0; // 0 for floating, else fixed. TODO: option?

			if (const auto NumDigits = FixedPrecision? FixedPrecision : Integral < 10? 2 : Integral < 100? 1 : 0)
			{
				const auto AjustedParts = [&]
				{
					const auto Multiplier = std::pow(10, NumDigits);
					const auto Value = Parts[1] * Multiplier;
					const auto UseRound = true;
					const auto Fractional = static_cast<unsigned long long>(UseRound? std::round(Value) : Value);
					return Fractional == Multiplier? std::make_pair(Integral + 1, 0ull) : std::make_pair(Integral, Fractional);
				}();

				Str = concat(str(AjustedParts.first), locale::GetDecimalSeparator(), pad_left(str(AjustedParts.second), NumDigits, L'0'));
			}
			else
			{
				Str = str(static_cast<int>(std::round(SizeInUnits)));
			}
		}

		return FormatSize(std::move(Str), UnitIndex);
	}

	const auto& ToStr = [UseCommas](auto Size)
	{
		return UseCommas? GroupDigits(Size) : str(Size);
	};

	size_t UnitIndex = 0;
	auto Str = ToStr(FileSize);

	const auto SuffixSize = (ShowUnit || (Width && Str.size() > Width))? UseCompact? 1u : 2u : 0u;
	
	const auto MaxNumberWidth = Width > SuffixSize? Width - SuffixSize : 0;

	while ((UseUnit && UnitIndex < MinUnit) || (Width && Str.size() > MaxNumberWidth))
	{
		if (unsigned long long SizeInUnits = std::round(FileSize / std::pow(Divider.first, UnitIndex + 1)))
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
// Return - количество замен
size_t ReplaceStrings(string& strStr, const string_view FindStr, const string_view ReplStr, const bool IgnoreCase, size_t Count)
{
	if (strStr.empty() || FindStr.empty() || !Count)
		return 0;

	const auto AreEqual = IgnoreCase? equal_icase : equal;

	size_t replaced = 0;
	for (size_t I = 0, L = strStr.size(); I + FindStr.size() <= L; ++I)
	{
		if (!AreEqual(string_view(strStr).substr(I, FindStr.size()), FindStr))
			continue;

		strStr.replace(I, FindStr.size(), ReplStr.data(), ReplStr.size());

		L += ReplStr.size();
		L -= FindStr.size();

		I += ReplStr.size();
		I -= 1;

		++replaced;

		if (Count != string::npos && !--Count)
			break;
	}
	return replaced;
}

/*
From PHP 4.x.x
Форматирует исходный текст по заданной ширине, используя
разделительную строку. Возвращает строку SrcText свёрнутую
в колонке, заданной параметром Width. Строка рубится при
помощи строки Break.

Разбивает на строки с выравниваением влево.

Если параметр Flahs & FFTM_BREAKLONGWORD, то строка всегда
сворачивается по заданной ширине. Так если у вас есть слово,
которое больше заданной ширины, то оно будет разрезано на части.

Example 1.
FarFormatText("Пример строки, которая будет разбита на несколько строк по ширине в 20 символов.", 20 ,Dest, "\n", 0);
Этот пример вернет:
---
Пример строки,
которая будет
разбита на
несколько строк по
ширине в 20
символов.
---

Example 2.
FarFormatText( "Эта строка содержит оооооооооооооччччччччеееень длиное слово", 9, Dest, nullptr, FFTM_BREAKLONGWORD);
Этот пример вернет:

---
Эта
строка
содержит
ооооооооо
ооооччччч
чччеееень
длиное
слово
---

*/

enum FFTMODE
{
	FFTM_BREAKLONGWORD = bit(0),
};

string& FarFormatText(const string& SrcText,      // источник
                            size_t Width,         // заданная ширина
                            string &strDestText,  // приёмник
                            const wchar_t* Break, // разделитель, если = nullptr, принимается "\n"
                            DWORD Flags)          // один из FFTM_*
{
	const auto breakchar = Break? Break : L"\n";

	if (SrcText.empty())
	{
		strDestText.clear();
		return strDestText;
	}

	if (SrcText.find_first_of(breakchar) == string::npos && SrcText.size() <= static_cast<size_t>(Width))
	{
		strDestText = SrcText;
		return strDestText;
	}

	long l=0, pgr=0;
	string newtext;
	const auto text= SrcText.c_str();
	const long linelength = static_cast<long>(Width);
	const size_t breakcharlen = wcslen(breakchar);
	const int docut = Flags&FFTM_BREAKLONGWORD?1:0;
	/* Special case for a single-character break as it needs no
	   additional storage space */

	if (breakcharlen == 1 && !docut)
	{
		newtext = text;
		size_t i = 0;

		while (i < newtext.size())
		{
			/* prescan line to see if it is greater than linelength */
			l = 0;

			while (i+l < newtext.size() && newtext[i+l] != breakchar[0])
			{
				if (newtext[i+l] == L'\0')
				{
					l--;
					break;
				}

				l++;
			}

			if (l >= linelength)
			{
				pgr = l;
				l = linelength;

				/* needs breaking; work backwards to find previous word */
				while (l >= 0)
				{
					if (newtext[i+l] == L' ')
					{
						newtext[i+l] = breakchar[0];
						break;
					}

					l--;
				}

				if (l == -1)
				{
					/* couldn't break is backwards, try looking forwards */
					l = linelength;

					while (l <= pgr)
					{
						if (newtext[i+l] == L' ')
						{
							newtext[i+l] = breakchar[0];
							break;
						}

						l++;
					}
				}
			}

			i += l+1;
		}
	}
	else
	{
		int last = 0;
		long i = 0;

		while (text[i] != L'\0')
		{
			/* prescan line to see if it is greater than linelength */
			l = 0;

			while (text[i+l] != L'\0')
			{
				if (text[i+l] == breakchar[0])
				{
					if (breakcharlen == 1 || starts_with(text + i + l, { breakchar, breakcharlen }))
						break;
				}

				l++;
			}

			if (l >= linelength)
			{
				pgr = l;
				l = linelength;

				/* needs breaking; work backwards to find previous word */
				while (l >= 0)
				{
					if (text[i+l] == L' ')
					{
						newtext.append(text+last, i+l-last);
						newtext += breakchar;
						last = i + l + 1;
						break;
					}

					l--;
				}

				if (l == -1)
				{
					/* couldn't break it backwards, try looking forwards */
					l = linelength - 1;

					while (l <= pgr)
					{
						if (!docut)
						{
							if (text[i+l] == L' ')
							{
								newtext.append(text+last, i+l-last);
								newtext += breakchar;
								last = i + l + 1;
								break;
							}
						}

						if (docut == 1)
						{
							if (text[i+l] == L' ' || l > i-last)
							{
								newtext.append(text+last, i+l-last+1);
								newtext += breakchar;
								last = i + l + 1;
								break;
							}
						}

						l++;
					}
				}

				i += l+1;
			}
			else
			{
				i += (l ? l : 1);
			}
		}

		if (i+l > last)
		{
			newtext += text+last;
		}
	}

	strDestText = newtext;
	return strDestText;
}

bool FindWordInString(const string& Str, size_t CurPos, size_t& Begin, size_t& End, const string& WordDiv0)
{
	if (Str.empty() || CurPos > Str.size())
		return false;

	const auto WordDiv = WordDiv0 + GetSpacesAndEols();

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

bool CheckFileSizeStringFormat(const string& FileSizeStr)
{
	static const std::wregex SizeRegex(RE_BEGIN RE_ANY_OF(L"0-9") RE_ONE_OR_MORE_LAZY RE_ANY_OF(L"BKMGTPE") RE_ZERO_OR_ONE_GREEDY RE_END, std::regex::icase | std::regex::optimize);
	return std::regex_search(FileSizeStr, SizeRegex);
}

unsigned long long ConvertFileSizeString(const string& FileSizeStr)
{
	if (!CheckFileSizeStringFormat(FileSizeStr))
		return 0;

	const auto n = std::stoull(FileSizeStr);

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
		const string& ReplaceStr,
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
						while (TokenEnd != TokenStart && (index = std::stoul(ReplaceStr.substr(TokenStart, TokenEnd - TokenStart))) >= Count)
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
						if (std::regex_search(ReplaceStr.c_str() + TokenStart, CMatch, re))
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
			if (re.SearchEx(Source.data(), Source.data() + Position, Source.data() + Source.size(), pm, n, hm))
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
			if (!re.SearchEx(Source.data(), Source.data() + pos, Source.data() + Source.size(), pm + half, n, hm))
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
	const auto& BlankOrWordDiv = [&WordDiv](wchar_t Ch)
	{
		return std::iswblank(Ch) || WordDiv.find(Ch) != WordDiv.npos;
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
	if (h > 15)
		throw MAKE_FAR_EXCEPTION(L"Not a hex char");
	if (h >= 10)
		return 'A' + h - 10;
	return '0' + h;
}

int HexToInt(char h)
{
	if (h >= 'a' && h <= 'f')
		return h - 'a' + 10;

	if (h >= 'A' && h <= 'F')
		return h - 'A' + 10;

	if (std::iswdigit(h))
		return h - '0';

	throw MAKE_FAR_EXCEPTION(L"Not a hex char");
}

template<class S, class C>
static S BlobToHexStringT(const void* Blob, size_t Size, C Separator)
{
	S Hex;

	Hex.reserve(Size * (Separator? 3 : 2));

	const auto CharBlob = reinterpret_cast<const char*>(Blob);
	std::for_each(CharBlob, CharBlob + Size, [&](char i)
	{
		Hex.push_back(IntToHex((i & 0xF0) >> 4));
		Hex.push_back(IntToHex(i & 0x0F));
		if (Separator)
		{
			Hex.push_back(Separator);
		}
	});
	if (Separator && !Hex.empty())
	{
		Hex.pop_back();
	}
	return Hex;
}

template<typename char_type>
static auto HexStringToBlobT(const basic_string_view<char_type> Hex, const char_type Separator)
{
	// Size shall be either 3 * N + 2 or even
	if (!Hex.empty() && (Separator? Hex.size() % 3 != 2 : Hex.size() & 1))
		throw MAKE_FAR_EXCEPTION(L"Incomplete hex string");

	const auto SeparatorSize = Separator? 1 : 0;
	const auto StepSize = 2 + SeparatorSize;
	const auto AlignedSize = Hex.size() + SeparatorSize;
	const auto BlobSize = AlignedSize / StepSize;

	if (!BlobSize)
		return bytes();

	std::vector<char> Blob;
	Blob.reserve(BlobSize);
	for (size_t i = 0; i != AlignedSize; i += StepSize)
	{
		Blob.emplace_back(HexToInt(Hex[i]) << 4 | HexToInt(Hex[i + 1]));
	}

	return bytes::copy(bytes_view(Blob.data(), Blob.size()));
}

std::string BlobToHexString(const void* Blob, size_t Size, char Separator)
{
	return BlobToHexStringT<std::string>(Blob, Size, Separator);
}

std::string BlobToHexString(const bytes_view& Blob, char Separator)
{
	return BlobToHexString(Blob.data(), Blob.size(), Separator);
}

bytes HexStringToBlob(const basic_string_view<char> Hex, const char Separator)
{
	return HexStringToBlobT(Hex, Separator);
}

string BlobToHexWString(const void* Blob, size_t Size, wchar_t Separator)
{
	return BlobToHexStringT<string>(Blob, Size, Separator);
}

string BlobToHexWString(const bytes_view& Blob, char Separator)
{
	return BlobToHexWString(Blob.data(), Blob.size(), Separator);
}

bytes HexStringToBlob(const string_view Hex, const wchar_t Separator)
{
	return HexStringToBlobT(Hex, Separator);
}

string ExtractHexString(const string& HexString)
{
	auto Result{ HexString };
	// TODO: Fix these and trailing spaces in Dialog class?
	Result.erase(std::remove(ALL_RANGE(Result), L' '), Result.end());
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

string ConvertHexString(const string& From, uintptr_t Codepage, bool FromHex)
{
	const auto CompatibleCp = IsVirtualCodePage(Codepage)? CP_ACP : Codepage;
	if (FromHex)
	{
		const auto Blob = HexStringToBlob(ExtractHexString(From), 0);
		return encoding::get_chars(CompatibleCp, { Blob.data(), Blob.size() });
	}
	else
	{
		const auto Blob = encoding::get_bytes(CompatibleCp, From);
		return BlobToHexWString(Blob.data(), Blob.size(), 0);
	}
}

// dest и src НЕ ДОЛЖНЫ пересекаться
char * xstrncpy(char * dest, const char * src, size_t DestSize)
{
	char *tmpsrc = dest;

	while (DestSize > 1 && (*dest++ = *src++) != 0)
	{
		DestSize--;
	}

	*dest = 0;
	return tmpsrc;
}

wchar_t * xwcsncpy(wchar_t * dest, const wchar_t * src, size_t DestSize)
{
	wchar_t *tmpsrc = dest;

	while (DestSize > 1 && (*dest++ = *src++) != 0)
		DestSize--;

	*dest = 0;
	return tmpsrc;
}

std::pair<string_view, string_view> split_name_value(string_view Str)
{
	const auto SeparatorPos = Str.find(L'=');
	return { Str.substr(0, SeparatorPos), Str.substr(SeparatorPos + 1) };
}
