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
#include "language.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "preservestyle.hpp"
#include "locale.hpp"
#include "stddlg.hpp"

namespace strmix
{

string &FormatNumber(const string& Src, string &strDest, int NumDigits)
{
	static bool first = true;
	static NUMBERFMT fmt;
	static wchar_t DecimalSep[4] = {};
	static wchar_t ThousandSep[4] = {};

	if (first)
	{
		DecimalSep[0] = locale::GetDecimalSeparator();
		ThousandSep[0] = locale::GetThousandSeparator();

		fmt.LeadingZero = 1;
		fmt.Grouping = 3;
		fmt.lpDecimalSep = DecimalSep;
		fmt.lpThousandSep = ThousandSep;
		fmt.NegativeOrder = 1;
		first = false;
	}

	fmt.NumDigits = NumDigits;
	string strSrc=Src;
	int Size=GetNumberFormat(LOCALE_USER_DEFAULT,0,strSrc.data(),&fmt,nullptr,0);
	wchar_t_ptr Dest(Size);
	GetNumberFormat(LOCALE_USER_DEFAULT,0,strSrc.data(),&fmt, Dest.get(), Size);
	strDest.assign(Dest.get(), Size - 1);
	return strDest;
}

string &InsertCommas(unsigned __int64 li,string &strDest)
{
	strDest = str_printf(L"%I64u", li);
	return FormatNumber(strDest,strDest);
}

static wchar_t * InsertCustomQuote(wchar_t *Str,wchar_t QuoteChar)
{
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

static string& InsertCustomQuote(string &strStr, wchar_t QuoteChar)
{
	size_t l = strStr.size();

	if (!l || strStr[0] != QuoteChar)
	{
		strStr.insert(0, 1, QuoteChar);
		l++;
	}

	if (l==1 || strStr[l-1] != QuoteChar)
	{
		strStr += QuoteChar;
	}

	return strStr;
}

wchar_t * InsertQuote(wchar_t *Str)
{
	return InsertCustomQuote(Str,L'\"');
}

wchar_t* QuoteSpace(wchar_t *Str)
{
	if (wcspbrk(Str, Global->Opt->strQuotedSymbols.data()) )
		InsertQuote(Str);

	return Str;
}


string& InsertQuote(string &strStr)
{
	return InsertCustomQuote(strStr,L'\"');
}

string& InsertRegexpQuote(string &strStr)
{
	//выражение вида /regexp/i не дополняем слешами
	if (strStr.empty() || strStr[0] != L'/')
	{
		strStr.insert(0, 1, L'/');
		strStr += L'/';
	}

	return strStr;
}

string &QuoteSpace(string &strStr)
{
	if (strStr.find_first_of(Global->Opt->strQuotedSymbols) != string::npos)
		InsertQuote(strStr);

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
	if (strStr.find(L' ') != string::npos)
		InsertQuote(strStr);

	return strStr;
}

string &QuoteLeadingSpace(string &strStr)
{
	size_t len = strStr.size();
	if (len > 0 && (L' ' == strStr[0] || L' ' == strStr[len-1]))
		InsertQuote(strStr);

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

	if (Str)
	{
		int Length = StrLength(Str);

		if (Length > MaxLength)
		{
			if (MaxLength > DotsLen)
			{
				int Len1 = (MaxLength - DotsLen) / 2;
				int Len2 = MaxLength - DotsLen - Len1;
				std::copy_n(L"...", DotsLen, Str + Len1);
				std::copy_n(Str + Length - Len2, Len2, Str + Len1 + DotsLen);
			}

			Str[MaxLength] = 0;
		}
	}
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

static int StartOffset(const wchar_t *Str, int nLength)
{
	if (nLength > 2)
	{
		if (Str[1] == L':' && IsSlash(Str[2]))
			return 3;

		else if (Str[0] == L'\\' && Str[1] == L'\\')
		{
			for (int n=2, i=2; i < nLength; ++i)
			{
				if (Str[i] == L'\\' && --n == 0)
					return i + 1;
			}
		}
	}
	return 0;
}

wchar_t* TruncPathStr(wchar_t *Str, int MaxLength)
{
	assert(MaxLength >= 0);
	MaxLength = std::max(0, MaxLength);

	if (Str)
	{
		int nLength = StrLength(Str);

		if (nLength > MaxLength && nLength >= 2)
		{
			int start = StartOffset(Str, nLength);

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

	if (nLength > MaxLength && nLength >= 2)
	{
		int start = StartOffset(strStr.data(), nLength);

		if (!start || start+DotsLen+2 > MaxLength)
			return TruncStr(strStr, MaxLength);

		strStr.replace(start, nLength-MaxLength+DotsLen, DotsLen, L'.');
	}
	return strStr;
}


wchar_t* RemoveLeadingSpaces(wchar_t *Str)
{
	wchar_t *ChPtr = Str;

	if (!ChPtr)
		return nullptr;

	for (; IsSpace(*ChPtr) || IsEol(*ChPtr); ChPtr++)
		;

	if (ChPtr!=Str)
		std::copy_n(ChPtr, wcslen(ChPtr) + 1, Str);

	return Str;
}


string& RemoveLeadingSpaces(string &strStr)
{
	const wchar_t *ChPtr = strStr.data();

	for (; IsSpace(*ChPtr) || IsEol(*ChPtr); ChPtr++)
		;

	strStr.erase(0, ChPtr - strStr.data());
	return strStr;
}


// удалить конечные пробелы
wchar_t* RemoveTrailingSpaces(wchar_t *Str)
{
	if (!Str)
		return nullptr;

	if (!*Str)
		return Str;

	for (wchar_t *ChPtr = Str + wcslen(Str) - 1; ChPtr >= Str; ChPtr--)
	{
		if (IsSpace(*ChPtr) || IsEol(*ChPtr))
			*ChPtr=0;
		else
			break;
	}

	return Str;
}


string& RemoveTrailingSpaces(string &strStr)
{
	if (strStr.empty())
		return strStr;

	const wchar_t *Str = strStr.data();
	const wchar_t *ChPtr = Str + strStr.size() - 1;

	for (; ChPtr >= Str && (IsSpace(*ChPtr) || IsEol(*ChPtr)); ChPtr--)
		;

	strStr.resize(ChPtr < Str ? 0 : ChPtr-Str+1);
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

string& CenterStr(const string& Src, string &strDest, int Length)
{
	int SrcLength=static_cast<int>(Src.size());
	string strTempStr = Src; //если Src == strDest, то надо копировать Src!

	if (SrcLength >= Length)
	{
		strDest = strTempStr;
		strDest.resize(Length);
	}
	else
	{
		int Space=(Length-SrcLength)/2;
		strDest = FormatString()<<fmt::MinWidth(Space)<<L""<<strTempStr<<fmt::MinWidth(Length-Space-SrcLength)<<L"";
	}

	return strDest;
}

string& RightStr(const string& Src, string &strDest, int Length)
{
	int SrcLength = static_cast<int>(Src.size());
	string strTempStr = Src; //если Src == strDest, то надо копировать Src!

	if (SrcLength >= Length)
	{
		strDest = strTempStr;
		strDest.resize(Length);
	}
	else
	{
		int Space=Length-SrcLength;
		strDest = FormatString()<<fmt::MinWidth(Space)<<L""<<strTempStr;
	}

	return strDest;
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

bool IsCaseMixed(const string &strSrc)
{
	const auto AlphaBegin = std::find_if(ALL_CONST_RANGE(strSrc), IsAlpha);
	if (AlphaBegin != strSrc.cend())
	{
		const auto Case = IsLower(*AlphaBegin);
		return std::any_of(AlphaBegin, strSrc.cend(), [&Case](wchar_t c){ return IsAlpha(c) && IsLower(c) != Case; });
	}
	return false;
}

void Unquote(wchar_t *Str)
{
	if (!Str)
		return;

	wchar_t *Dst=Str;

	while (*Str)
	{
		if (*Str!=L'\"')
			*Dst++=*Str;

		Str++;
	}

	*Dst=0;
}

string& Unquote(string &strStr)
{
	strStr.erase(std::remove(ALL_RANGE(strStr), L'"'), strStr.end());
	return strStr;
}


void UnquoteExternal(string &strStr)
{
	if (!strStr.empty() && strStr.front() == L'\"' && strStr.back() == L'\"')
	{
		strStr.pop_back();
		strStr.erase(0, 1);
	}
}


/* FileSizeToStr()
   Форматирование размера файла в удобочитаемый вид.
*/
enum
{
	UNIT_COUNT = 7, // byte, kilobyte, megabyte, gigabyte, terabyte, petabyte, exabyte.
};

static string& UnitStr(size_t B, size_t Div)
{
	static string Data[UNIT_COUNT][2];
	return Data[B][Div];
}

void PrepareUnitStr()
{
	for (int i=0; i<UNIT_COUNT; i++)
	{
		UnitStr(i, 0) = UnitStr(i, 1) = MSG(MListBytes + i);
		ToLower(UnitStr(i, 0));
		ToUpper(UnitStr(i, 1));
	}
}

string & FileSizeToStr(string &strDestStr, unsigned __int64 Size, int Width, unsigned __int64 ViewFlags)
{
	FormatString strStr;
	unsigned __int64 Divider;
	size_t IndexDiv, IndexB;

	// подготовительные мероприятия
	if (UnitStr(0, 0).empty())
	{
		PrepareUnitStr();
	}

	bool Commas=(ViewFlags & COLUMN_COMMAS)!=0;
	bool FloatSize=(ViewFlags & COLUMN_FLOATSIZE)!=0;
	bool Economic=(ViewFlags & COLUMN_ECONOMIC)!=0;
	bool UseMinSizeIndex=(ViewFlags & COLUMN_MINSIZEINDEX)!=0;
	size_t MinSizeIndex=(ViewFlags & COLUMN_MINSIZEINDEX_MASK)+1;
	bool ShowBytesIndex=(ViewFlags & COLUMN_SHOWBYTESINDEX)!=0;

	if (ViewFlags & COLUMN_THOUSAND)
	{
		Divider=1000;
		IndexDiv=0;
	}
	else
	{
		Divider=1024;
		IndexDiv=1;
	}

	unsigned __int64 Sz = Size, Divider2 = Divider/2, Divider64 = Divider, OldSize;

	if (FloatSize)
	{
		unsigned __int64 Divider64F = 1, Divider64F_mul = 1000, Divider64F2 = 1, Divider64F2_mul = Divider;

		//выравнивание идёт по 1000 но само деление происходит на Divider
		//например 999 bytes покажутся как 999 а вот 1000 bytes уже покажутся как 0.97 K
		for (IndexB=0; IndexB<UNIT_COUNT-1; IndexB++)
		{
			if (Sz < Divider64F*Divider64F_mul)
				break;

			Divider64F = Divider64F*Divider64F_mul;
			Divider64F2  = Divider64F2*Divider64F2_mul;
		}

		if (!IndexB)
		{
			strStr << Sz;
		}
		else
		{
			Sz = (OldSize=Sz) / Divider64F2;
			OldSize = (OldSize % Divider64F2) / (Divider64F2 / Divider64F2_mul);
			DWORD Decimal = (DWORD)(0.5+(double)(DWORD)OldSize/(double)Divider*100.0);

			if (Decimal >= 100)
			{
				Decimal -= 100;
				Sz++;
			}

			strStr << Sz << L"." << fmt::MinWidth(2) << fmt::FillChar(L'0') << Decimal;
			FormatNumber(strStr,strStr,2);
		}

		if (IndexB>0 || ShowBytesIndex)
		{
			Width-=(Economic?1:2);

			if (Width<0)
				Width=0;

			strDestStr = str_printf(Economic ? L"%*.*s%1.1s" : L"%*.*s %1.1s", Width, Width, strStr.data(), UnitStr(IndexB, IndexDiv).data());
		}
		else
			strDestStr = str_printf(L"%*.*s",Width,Width,strStr.data());

		return strDestStr;
	}

	if (Commas)
		InsertCommas(Sz,strStr);
	else
		strStr << Sz;

	if ((!UseMinSizeIndex && strStr.size()<=static_cast<size_t>(Width)) || Width<5)
	{
		if (ShowBytesIndex)
		{
			Width-=(Economic?1:2);

			if (Width<0)
				Width=0;

			strDestStr = str_printf(Economic? L"%*.*s%1.1s" : L"%*.*s %1.1s", Width, Width, strStr.data(), UnitStr(0, IndexDiv).data());
		}
		else
			strDestStr = str_printf(L"%*.*s",Width,Width,strStr.data());
	}
	else
	{
		Width-=(Economic?1:2);
		IndexB=0;

		do
		{
			//Sz=(Sz+Divider2)/Divider64;
			Sz = (OldSize=Sz) / Divider64;

			if ((OldSize % Divider64) > Divider2)
				++Sz;

			IndexB++;

			if (Commas)
			{
				InsertCommas(Sz,strStr);
			}
			else
			{
				strStr.clear();
				strStr << Sz;
			}
		}
		while ((UseMinSizeIndex && IndexB<MinSizeIndex) || strStr.size() > static_cast<size_t>(Width));

		strDestStr = str_printf(Economic? L"%*.*s%1.1s" : L"%*.*s %1.1s", Width, Width, strStr.data(), UnitStr(IndexB, IndexDiv).data());
	}

	return strDestStr;
}


// Заменить в строке Str Count вхождений подстроки FindStr на подстроку ReplStr
// Если Count == npos - заменять "до полной победы"
// Return - количество замен
size_t ReplaceStrings(string &strStr, const wchar_t* FindStr, size_t FindStrSize, const wchar_t* ReplStr, size_t ReplStrSize, bool IgnoreCase, size_t Count)
{
	if ( !FindStrSize || !Count )
		return 0;

	const auto Comparer = IgnoreCase? StrEqualNI : StrEqualN;

	size_t replaced = 0;
	for (size_t I=0, L=strStr.size(); I+FindStrSize <= L; ++I)
	{
		if (Comparer(strStr.data() + I, FindStr, FindStrSize))
		{
			strStr.replace(I, FindStrSize, ReplStr);
			L += ReplStrSize - FindStrSize;
			I += ReplStrSize - 1;
			++replaced;

			if (!--Count)
				break;
		}
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
	FFTM_BREAKLONGWORD = 0x00000001,
};

string& FarFormatText(const string& SrcText,      // источник
                            int Width,            // заданная ширина
                            string &strDestText,  // приёмник
                            const wchar_t* Break, // разделитель, если = nullptr, принимается "\n"
                            DWORD Flags)          // один из FFTM_*
{
	const wchar_t *breakchar;
	breakchar = Break?Break:L"\n";

	if (SrcText.empty())
	{
		strDestText.clear();
		return strDestText;
	}

	string strSrc = SrcText; //copy string in case of SrcText == strDestText

	if (strSrc.find_first_of(breakchar) == string::npos && strSrc.size() <= static_cast<size_t>(Width))
	{
		strDestText = strSrc;
		return strDestText;
	}

	long l=0, pgr=0;
	string newtext;
	const wchar_t *text= strSrc.data();
	long linelength = Width;
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
					if (breakcharlen == 1 || !StrCmpN(text+i+l, breakchar, breakcharlen))
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

/*
  Ptr=CalcWordFromString(Str,I,&Start,&End);
  xstrncpy(Dest,Ptr,End-Start+1);
  Dest[End-Start+1]=0;

// Параметры:
//   WordDiv  - набор разделителей слова в кодировке OEM
  возвращает указатель на начало слова
*/
const wchar_t * const CalcWordFromString(const wchar_t *Str,int CurPos,int *Start,int *End, const string& WordDiv0)
{
	int StartWPos, EndWPos;
	int StrSize=StrLength(Str);

	if (CurPos > StrSize)
		return nullptr;

	string strWordDiv(WordDiv0);
	strWordDiv += L" \t\n\r";

	if (IsWordDiv(strWordDiv,Str[CurPos]))
	{
		// вычисляем дистанцию - куда копать, где ближе слово - слева или справа
		int I = CurPos, J = CurPos;
		// копаем влево
		DWORD DistLeft=-1;

		while (I >= 0 && IsWordDiv(strWordDiv,Str[I]))
		{
			DistLeft++;
			I--;
		}

		if (I < 0)
			DistLeft=-1;

		// копаем вправо
		DWORD DistRight=-1;

		while (J < StrSize && IsWordDiv(strWordDiv,Str[J]))
		{
			DistRight++;
			J++;
		}

		if (J >= StrSize)
			DistRight=-1;

		if (DistLeft > DistRight) // ?? >=
			EndWPos=StartWPos=J;
		else
			EndWPos=StartWPos=I;
	}
	else // здесь все оби, т.е. стоим на буковке
		EndWPos=StartWPos=CurPos;

	if (StartWPos < StrSize)
	{
		while (StartWPos >= 0)
			if (IsWordDiv(strWordDiv,Str[StartWPos]))
			{
				StartWPos++;
				break;
			}
			else
				StartWPos--;

		while (EndWPos < StrSize)
			if (IsWordDiv(strWordDiv,Str[EndWPos]))
			{
				EndWPos--;
				break;
			}
			else
				EndWPos++;
	}

	if (StartWPos < 0)
		StartWPos=0;

	if (EndWPos >= StrSize)
		EndWPos=StrSize;

	*Start=StartWPos;
	*End=EndWPos;
	return Str+StartWPos;
}


bool CheckFileSizeStringFormat(const string& FileSizeStr)
{
//проверяет если формат строки такой: [0-9]+[BbKkMmGgTtPpEe]?
	const wchar_t *p = FileSizeStr.data();

	while (iswdigit(*p))
		p++;

	if (p == FileSizeStr.data())
		return false;

	if (*p)
	{
		if (*(p+1))
			return false;

		if (!StrStrI(L"BKMGTPE", p))
			return false;
	}

	return true;
}

unsigned __int64 ConvertFileSizeString(const string& FileSizeStr)
{
	if (!CheckFileSizeStringFormat(FileSizeStr))
		return 0;

	unsigned __int64 n = std::stoull(FileSizeStr);
	wchar_t c = ::ToUpper(FileSizeStr.back());

	// http://en.wikipedia.org/wiki/SI_prefix
	switch (c)
	{
		case L'K':		// kilo 10x3
			n <<= 10;
			break;
		case L'M':		// mega 10x6
			n <<= 20;
			break;
		case L'G':		// giga 10x9
			n <<= 30;
			break;
		case L'T':		// tera 10x12
			n <<= 40;
			break;
		case L'P':		// peta 10x15
			n <<= 50;
			break;
		case L'E':		// exa  10x18
			n <<= 60;
			break;
			// Z - zetta 10x21
			// Y - yotta 10x24
	}

	return n;
}

string ReplaceBrackets(const wchar_t *SearchStr,const string& ReplaceStr, const RegExpMatch* Match,int Count)
{
	string result;
	size_t pos=0,length=ReplaceStr.size();

	while (pos<length)
	{
		bool common=true;

		if (ReplaceStr[pos]=='$')
		{
			++pos;

			if (pos>=length)
				break;

			wchar_t symbol = ::ToUpper(ReplaceStr[pos]);
			int index=-1;

			if (symbol>='0'&&symbol<='9')
			{
				index=symbol-'0';
			}
			else if (symbol>='A'&&symbol<='Z')
			{
				index=symbol-'A'+10;
			}

			if (index>=0)
			{
				if (index<Count&&Match[index].end>=0)
				{
					string bracket(SearchStr+Match[index].start,Match[index].end-Match[index].start);
					result+=bracket;
				}

				common=false;
			}
		}

		if (common)
		{
			result+=ReplaceStr[pos];
		}

		++pos;
	}

	return result;
}

string GuidToStr(const GUID& Guid)
{
	string result;
	RPC_WSTR str;
	// declared as non-const in GCC headers :(
	if(UuidToString(const_cast<GUID*>(&Guid), &str) == RPC_S_OK)
	{
		SCOPE_EXIT{ RpcStringFree(&str); };
		result = reinterpret_cast<const wchar_t*>(str);
	}
	return ToUpper(result);
}

bool StrToGuid(const wchar_t* Value,GUID& Guid)
{
	return UuidFromString(reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(Value)), &Guid) == RPC_S_OK;
}

bool SearchString(const wchar_t* Source, int StrSize, const string& Str, const string &UpperStr, const string &LowerStr, RegExp &re, RegExpMatch *pm, string& ReplaceStr,int& CurPos, int Position,int Case,int WholeWords,int Reverse,int Regexp,int PreserveStyle, int *SearchLength,const wchar_t* WordDiv)
{
	*SearchLength = 0;

	if (!WordDiv)
		WordDiv=Global->Opt->strWordDiv.data();

	if (!Regexp && PreserveStyle && PreserveStyleReplaceString(Source, StrSize, Str, ReplaceStr, CurPos, Position, Case, WholeWords, WordDiv, Reverse, *SearchLength))
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
				if (re.SearchEx(Source, Source + Position, Source + StrSize, pm, n))
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
					if (!re.SearchEx(Source, Source + pos, Source + StrSize, pm+half, n))
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
				ReplaceStr=ReplaceBrackets(Source,ReplaceStr,pm+half,n);
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
					if (PreserveStyle && !ReplaceStr.empty() && IsAlpha(ReplaceStr.front()) && IsAlpha(Source[I]))
					{
						if (IsUpper(Source[I]))
							ReplaceStr.front() = ::ToUpper(ReplaceStr.front());
						if (IsLower(Source[I]))
							ReplaceStr.front() = ::ToLower(ReplaceStr.front());
					}

					return true;
				}

				if (WholeWords)
				{
					int locResultLeft=FALSE;
					int locResultRight=FALSE;
					wchar_t ChLeft=Source[I-1];

					if (I>0)
						locResultLeft=(IsSpace(ChLeft) || wcschr(WordDiv,ChLeft));
					else
						locResultLeft=TRUE;

					if (I+Length<StrSize)
					{
						wchar_t ChRight=Source[I+Length];
						locResultRight=(IsSpace(ChRight) || wcschr(WordDiv,ChRight));
					}
					else
					{
						locResultRight=TRUE;
					}

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

string wide_n(const char *str, size_t size, uintptr_t codepage)
{
	if (size_t Size = MultiByteToWideChar(codepage, 0, str, static_cast<int>(size), nullptr, 0))
	{
		std::vector<wchar_t> Buffer(Size);
		MultiByteToWideChar(codepage, 0, str, static_cast<int>(size), Buffer.data(), static_cast<int>(Buffer.size()));
		return string(Buffer.data(), Buffer.size());
	}
	return string();
}

std::string narrow_n(const wchar_t* str, size_t size, uintptr_t codepage)
{
	if (size_t Size = WideCharToMultiByte(codepage, 0, str, static_cast<int>(size), nullptr, 0, nullptr, nullptr))
	{
		std::vector<char> Buffer(Size);
		WideCharToMultiByte(codepage, 0, str, static_cast<int>(size), Buffer.data(), static_cast<int>(Buffer.size()), nullptr, nullptr);
		return std::string(Buffer.data(), Buffer.size());
	}
	return std::string();
}

string str_vprintf(const wchar_t * format, va_list argptr)
{
	wchar_t_ptr buffer;
	size_t size = 128;
	int length = -1;
	do
	{
		buffer.reset(size *= 2);

		//_vsnwprintf не всегда ставит '\0' в конце.
		//Поэтому надо обнулить и передать в _vsnwprintf размер-1.
		buffer[size - 1] = 0;
		length = _vsnwprintf(buffer.get(), size - 1, format, argptr);
	}
	while (length < 0);

	return string(buffer.get());
}

string str_printf(const wchar_t * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	SCOPE_EXIT{ va_end(argptr); };
	return str_vprintf(format, argptr);
}

	class UserDefinedList
	{
	public:
		typedef std::pair<string, size_t> value_type;

		UserDefinedList(const string& List, DWORD InitFlags, const wchar_t* InitSeparators)
		{
			BitFlags Flags(InitFlags);
			string strSeparators(InitSeparators);
			static const wchar_t Brackets[] = L"[]";

			if (!List.empty() &&
				strSeparators.find(L'\"') == string::npos &&
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
						item.first = L"*";
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
						ItemsList.unique([](value_type& a, value_type& b)->bool
						{
							if (a.second > b.second)
								a.second = b.second;
							return !StrCmpI(a.first, b.first);
						});
					}
				}
			}
		}

		static bool GetToken(const string& List, string::const_iterator& Iterator, const string& strSeparators, const BitFlags& Flags, string& Token)
		{
			if (Iterator == List.cend())
				return false;

			if (strSeparators.find(*Iterator) != string::npos)
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
					else if (*cur == L'[' && std::find(cur + 1, List.cend(), L']') != List.cend())
						InBrackets = true;
				}

				if (!Flags.Check(STLF_NOQUOTING) && *cur == L'\"')
				{
					InQuotes = InQuotes? false : std::find(cur + 1, List.cend(), L'\"') != List.cend();
				}

				if (!InBrackets && !InQuotes && strSeparators.find(*cur) != string::npos)
					break;

				++cur;
			}

			Token.assign(Iterator, cur);
			Iterator = cur == List.cend() ? cur : cur + 1;

			if (!Flags.Check(STLF_NOTRIM))
				RemoveExternalSpaces(Token);

			if (!Flags.Check(STLF_NOUNQUOTE))
				Unquote(Token);

			return true;
		}

		std::list<value_type> ItemsList;
	};

	void split(const string& InitString, DWORD Flags, const wchar_t* Separators, const std::function<void(string&)>& inserter)
	{
		FOR(auto& i, UserDefinedList(InitString, Flags, Separators).ItemsList)
		{
			inserter(i.first);
		}
	}

int IntToHex(int h)
{
	if (h >= 10)
		return 'A' + h - 10;
	return '0' + h;
}

int HexToInt(int h)
{
	if (h >= 'a' && h <= 'f')
		return h - 'a' + 10;

	if (h >= 'A' && h <= 'F')
		return h - 'A' + 10;

	if (h >= '0' && h <= '9')
		return h - '0';

	return 0;
}

template<class S, class C>
S BlobToHexStringT(const void* Blob, size_t Size, C Separator)
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
std::vector<char> HexStringToBlobT(const C* Hex, size_t Size, C Separator)
{
	std::vector<char> Blob;
	Blob.reserve((Size + 1) / 3);
	while (*Hex && *(Hex+1))
	{
		Blob.push_back((HexToInt(*Hex)<<4) | HexToInt(*(Hex+1)));
		Hex += 2;
		if (!*Hex)
		{
			break;
		}
		if (Separator)
		{
			++Hex;
		}
	}
	return Blob;
}

std::string BlobToHexString(const void* Blob, size_t Size, char Separator)
{
	return BlobToHexStringT<std::string>(Blob, Size, Separator);
}

std::vector<char> HexStringToBlob(const char* Hex, char Separator)
{
	return HexStringToBlobT(Hex, strlen(Hex), Separator);
}

string BlobToHexWString(const void* Blob, size_t Size, wchar_t Separator)
{
	return BlobToHexStringT<string>(Blob, Size, Separator);
}

std::vector<char> HexStringToBlob(const wchar_t* Hex, wchar_t Separator)
{
	return HexStringToBlobT(Hex, wcslen(Hex), Separator);
}


}
