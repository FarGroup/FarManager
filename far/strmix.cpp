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

#include "headers.hpp"
#pragma hdrstop

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
	const size_t Size = GetNumberFormat(GetThreadLocale(), 0, strSrc.data(), &Fmt, nullptr, 0);
	wchar_t_ptr_n<MAX_PATH> Dest(Size);
	GetNumberFormat(GetThreadLocale(), 0, strSrc.data(), &Fmt, Dest.get(), static_cast<int>(Size));
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
	if (wcspbrk(Str, Global->Opt->strQuotedSymbols.data()))
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
		int Length = StrLength(Str);

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
		int Length = StrLength(Str);

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

	const auto Length = StrLength(Str);
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
		int nLength = StrLength(Str);

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


wchar_t* RemoveLeadingSpaces(wchar_t *Str)
{
	const auto Iterator = null_iterator(Str);
	const auto NewBegin = std::find_if_not(Iterator, Iterator.end(), IsSpaceOrEol);
	if (NewBegin != Iterator)
	{
		*std::copy(NewBegin, Iterator.end(), Str) = L'\0';
	}
	return Str;
}

string& RemoveLeadingSpaces(string &strStr)
{
	strStr.erase(strStr.begin(), std::find_if_not(ALL_RANGE(strStr), IsSpaceOrEol));
	return strStr;
}

// удалить конечные пробелы
wchar_t* RemoveTrailingSpaces(wchar_t *Str)
{
	const auto REnd = std::make_reverse_iterator(Str);
	Str[REnd - std::find_if_not(REnd - wcslen(Str), REnd, IsSpaceOrEol)] = 0;
	return Str;
}

string& RemoveTrailingSpaces(string &strStr)
{
	strStr.resize(strStr.rend() - std::find_if_not(ALL_REVERSE_RANGE(strStr), IsSpaceOrEol));
	return strStr;
}

wchar_t* RemoveExternalSpaces(wchar_t *Str)
{
	return RemoveTrailingSpaces(RemoveLeadingSpaces(Str));
}

string& RemoveExternalSpaces(string &strStr)
{
	return RemoveTrailingSpaces(RemoveLeadingSpaces(strStr));
}


/* $ 02.02.2001 IS
   Заменяет пробелами непечатные символы в строке. В настоящий момент
   обрабатываются только cr и lf.
*/
string& RemoveUnprintableCharacters(string &strStr)
{
	std::replace_if(ALL_RANGE(strStr), IsEol, L' ');
	return RemoveExternalSpaces(strStr);
}

const wchar_t *GetCommaWord(const wchar_t *Src, string &strWord,wchar_t Separator)
{
	if (!*Src)
		return nullptr;

	const wchar_t *StartPtr = Src;
	size_t WordLen;
	bool SkipBrackets=false;

	for (WordLen=0; *Src; Src++,WordLen++)
	{
		if (*Src==L'[' && wcschr(Src+1,L']'))
			SkipBrackets=true;

		if (*Src==L']')
			SkipBrackets=false;

		if (*Src==Separator && !SkipBrackets)
		{
			Src++;

			while (IsSpace(*Src))
				Src++;

			strWord.assign(StartPtr,WordLen);
			return Src;
		}
	}

	strWord.assign(StartPtr,WordLen);
	return Src;
}

bool IsCaseMixed(const string_view& strSrc)
{
	const auto AlphaBegin = std::find_if(ALL_CONST_RANGE(strSrc), is_alpha);
	if (AlphaBegin == strSrc.cend())
		return false;

	const auto Case = is_lower(*AlphaBegin);
	return std::any_of(AlphaBegin, strSrc.cend(), [Case](wchar_t c){ return is_alpha(c) && is_lower(c) != Case; });
}

void UnquoteExternal(string &strStr)
{
	auto len = strStr.size();
	if (len > 0 && strStr.front() == L'\"')
	{
		if (len < 2) // '"'
		{
			strStr.clear();
		}
		else if (strStr.back() == L'\"') // '"D:\Path Name"'
		{
			strStr.pop_back();
			strStr.erase(0, 1);
		}
		else if (len >= 3 && IsSlash(strStr[len-1]) && strStr[len-2] == L'\"') // '"D:\Path Name"\'
		{
			strStr.erase(len-2, 1);
			strStr.erase(0, 1);
		}
	}
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
size_t ReplaceStrings(string &strStr, const string_view& FindStr, const string_view& ReplStr, bool IgnoreCase, size_t Count)
{
	if (FindStr.empty() || !Count)
		return 0;

	const auto AreEqual = IgnoreCase? equal_icase : equal;

	size_t replaced = 0;
	for (size_t I = 0, L = strStr.size(); I + FindStr.size() <= L; ++I)
	{
		if (!AreEqual(make_string_view(strStr, I, FindStr.size()), FindStr))
			continue;

		strStr.replace(I, FindStr.size(), ReplStr.raw_data(), ReplStr.size());

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

	const auto strSrc = SrcText; //copy string in case of SrcText == strDestText

	if (strSrc.find_first_of(breakchar) == string::npos && strSrc.size() <= static_cast<size_t>(Width))
	{
		strDestText = strSrc;
		return strDestText;
	}

	long l=0, pgr=0;
	string newtext;
	const wchar_t *text= strSrc.data();
	long linelength = static_cast<long>(Width);
	size_t breakcharlen = wcslen(breakchar);
	int docut = Flags&FFTM_BREAKLONGWORD?1:0;
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
			if (!IsSpaceOrEol(Str[Begin - 1]))
				--Begin;
		}
		else
		{
			if (!IsSpaceOrEol(Str[Begin]))
			{
				++End;
			}
			else
			{
				if (Begin && !IsSpaceOrEol(Str[Begin - 1]))
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

string ReplaceBrackets(const wchar_t *SearchStr, const string& ReplaceStr, const RegExpMatch* Match, size_t Count, const MatchHash* HMatch)
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
					if (std::regex_search(ReplaceStr.data() + TokenStart, CMatch, re))
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
						result += string(SearchStr + start, end - start);
					}
				}
			}
		}

		if (common)
		{
			result += CurrentChar;
		}
	}

	return result;
}

bool SearchString(const wchar_t* Source, int StrSize, const string& Str, const string &UpperStr, const string &LowerStr, RegExp &re, RegExpMatch *pm, MatchHash* hm, string& ReplaceStr, int& CurPos, int Case, int WholeWords, int Reverse, int Regexp, int PreserveStyle, int *SearchLength, const wchar_t* WordDiv)
{
	int Position = CurPos;
	*SearchLength = 0;

	if (!WordDiv)
		WordDiv=Global->Opt->strWordDiv.data();

	if (!Regexp && PreserveStyle && PreserveStyleReplaceString(Source, StrSize, Str, ReplaceStr, CurPos, Case, WholeWords, WordDiv, Reverse, *SearchLength))
		return true;

	if (Reverse)
	{
		Position--;

		if (Position>=StrSize)
			Position=StrSize-1;

		if (Position<0)
			return false;
	}

	if ((Position<StrSize || (!Position && !StrSize)) && !Str.empty())
	{
		if (Regexp)
		{
			intptr_t n = re.GetBracketsCount();
			bool found = false;
			int half = 0;
			if (!Reverse)
			{
				if (re.SearchEx(Source, Source + Position, Source + StrSize, pm, n, hm))
				{
					found = true;
				}
				else
				{
					ReMatchErrorMessage(re);
				}
			}
			else
			{
				int pos = 0;
				for (;;)
				{
					if (!re.SearchEx(Source, Source + pos, Source + StrSize, pm + half, n, hm))
					{
						ReMatchErrorMessage(re);
						break;
					}
					pos = static_cast<int>(pm[half].start);
					if (pos > Position)
						break;

					found = true;
					++pos;
					half = n - half;
				}
				half = n - half;
			}
			if (found)
			{
				*SearchLength = pm[half].end - pm[half].start;
				CurPos = pm[half].start;
				ReplaceStr = ReplaceBrackets(Source, ReplaceStr, pm + half, n, hm);
			}

			return found;
		}

		if (Position==StrSize)
			return false;

		int Length = *SearchLength = (int)Str.size();

		for (int I=Position; (Reverse && I>=0) || (!Reverse && I<StrSize); Reverse ? I--:I++)
		{
			for (int J=0;; J++)
			{
				if (!Str[J])
				{
					CurPos=I;

					// В случае PreserveStyle: если не получилось сделать замену c помощью PreserveStyleReplaceString,
					// то хотя бы сохранить регистр первой буквы.
					if (PreserveStyle && !ReplaceStr.empty() && is_alpha(ReplaceStr.front()) && is_alpha(Source[I]))
					{
						if (is_upper(Source[I]))
							ReplaceStr.front() = ::upper(ReplaceStr.front());
						if (is_lower(Source[I]))
							ReplaceStr.front() = ::lower(ReplaceStr.front());
					}

					return true;
				}

				if (WholeWords)
				{
					const auto locResultLeft = I <= 0 || IsSpace(Source[I - 1]) || wcschr(WordDiv, Source[I - 1]);
					const auto locResultRight = I + Length >= StrSize || IsSpace(Source[I + Length]) || wcschr(WordDiv, Source[I + Length]);

					if (!locResultLeft || !locResultRight)
						break;
				}

				wchar_t Ch=Source[I+J];

				if (Case)
				{
					if (Ch!=Str[J])
						break;
				}
				else
				{
					if (Ch!=UpperStr[J] && Ch!=LowerStr[J])
						break;
				}
			}
		}
	}

	return false;
}

	class UserDefinedList
	{
	public:
		using value_type = std::pair<string, size_t>;

		UserDefinedList(const string& List, DWORD InitFlags, const wchar_t* InitSeparators)
		{
			BitFlags Flags(InitFlags);
			string strSeparators(InitSeparators);
			static const wchar_t Brackets[] = L"[]";

			if (!List.empty() &&
				!contains(strSeparators, L'\"') &&
				(!Flags.Check(STLF_PROCESSBRACKETS) || std::find_first_of(ALL_CONST_RANGE(strSeparators), ALL_CONST_RANGE(Brackets)) == strSeparators.cend()))
			{
				value_type item;
				item.second = ItemsList.size();

				auto Iterator = List.cbegin();
				string Token;
				while (GetToken(List, Iterator, strSeparators, Flags, Token))
				{
					if (Flags.Check(STLF_PACKASTERISKS) && Token.size() == 3 && Token == L"*.*")
					{
						item.first = L'*';
						ItemsList.emplace_back(item);
					}
					else
					{
						if (Token.empty() && !Flags.Check(STLF_ALLOWEMPTY))
						{
							continue;
						}

						item.first = Token;

						if (Flags.Check(STLF_PACKASTERISKS))
						{
							size_t i = 0;
							bool lastAsterisk = false;

							while (i < Token.size())
							{
								if (item.first[i] == L'*')
								{
									if (!lastAsterisk)
										lastAsterisk = true;
									else
									{
										item.first.erase(i, 1);
										--i;
									}
								}
								else
									lastAsterisk = false;

								++i;
							}
						}
						ItemsList.emplace_back(item);
					}

					++item.second;
				}
				if (Flags.Check(STLF_UNIQUE | STLF_SORT))
				{
					ItemsList.sort([](const value_type& a, const value_type& b)
					{
						return a.second < b.second;
					});

					if (Flags.Check(STLF_UNIQUE))
					{
						ItemsList.unique([](value_type& a, value_type& b)
						{
							if (a.second > b.second)
								a.second = b.second;
							return equal_icase(a.first, b.first);
						});
					}
				}
			}
		}

		static bool GetToken(const string& List, string::const_iterator& Iterator, const string& strSeparators, const BitFlags& Flags, string& Token)
		{
			if (Iterator == List.cend())
				return false;

			if (contains(strSeparators, *Iterator))
			{
				Token.clear();
				++Iterator;
				return true;
			}

			auto cur = Iterator;
			bool InBrackets = false;
			bool InQuotes = false;

			while (cur != List.cend()) // важно! проверка *cur должна стоять первой
			{
				if (Flags.Check(STLF_PROCESSBRACKETS)) // чтобы не сортировать уже отсортированное
				{
					if (*cur == L']')
						InBrackets = false;
					else if (*cur == L'[' && contains(make_range(cur + 1, List.cend()), L']'))
						InBrackets = true;
				}

				if (!Flags.Check(STLF_NOQUOTING) && *cur == L'\"')
				{
					InQuotes = InQuotes? false : contains(make_range(cur + 1, List.cend()), L'\"');
				}

				if (!InBrackets && !InQuotes && contains(strSeparators, *cur))
					break;

				++cur;
			}

			Token.assign(Iterator, cur);
			Iterator = cur == List.cend() ? cur : cur + 1;

			if (!Flags.Check(STLF_NOTRIM))
				RemoveExternalSpaces(Token);

			if (!Flags.Check(STLF_NOUNQUOTE))
				inplace::unquote(Token);

			return true;
		}

		std::list<value_type> ItemsList;
	};

	void split(const string& InitString, DWORD Flags, const wchar_t* Separators, const std::function<void(string&&)>& inserter)
	{
		for (auto& i: UserDefinedList(InitString, Flags, Separators).ItemsList)
		{
			inserter(std::move(i.first));
		}
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

template<class C>
static auto HexStringToBlobT(const C* Hex, size_t Size, C Separator)
{
	// Size shall be either 3 * N + 2 or even
	if (Size && (Separator? Size % 3 != 2 : Size & 1))
		throw MAKE_FAR_EXCEPTION(L"Incomplete hex string");

	const auto SeparatorSize = Separator? 1 : 0;
	const auto StepSize = 2 + SeparatorSize;
	const auto AlignedSize = Size + SeparatorSize;
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

bytes HexStringToBlob(const char* Hex, char Separator)
{
	return HexStringToBlobT(Hex, strlen(Hex), Separator);
}

string BlobToHexWString(const void* Blob, size_t Size, wchar_t Separator)
{
	return BlobToHexStringT<string>(Blob, Size, Separator);
}

string BlobToHexWString(const bytes_view& Blob, char Separator)
{
	return BlobToHexWString(Blob.data(), Blob.size(), Separator);
}

bytes HexStringToBlob(const wchar_t* Hex, wchar_t Separator)
{
	return HexStringToBlobT(Hex, wcslen(Hex), Separator);
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
		const auto Blob = HexStringToBlob(ExtractHexString(From).data(), 0);
		return encoding::get_chars(CompatibleCp, Blob.data(), Blob.size());
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

std::pair<string, string> split_name_value(const wchar_t* Line)
{
	const auto SeparatorPos = wcschr(Line + 1, L'=');
	return { { Line, SeparatorPos }, SeparatorPos + 1 };
}
