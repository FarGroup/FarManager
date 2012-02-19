#pragma once

/*
strmix.hpp

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

#include "plugin.hpp"

typedef unsigned __int64 FILEPANEL_COLUMN_MODES;
static const FILEPANEL_COLUMN_MODES
	COLUMN_MARK                   = 0x8000000000000000LL,
	COLUMN_NAMEONLY               = 0x4000000000000000LL,
	COLUMN_RIGHTALIGN             = 0x2000000000000000LL,
	COLUMN_FORMATTED              = 0x1000000000000000LL,
	COLUMN_COMMAS                 = 0x0800000000000000LL, // Вставлять разделитель между тысячами
	COLUMN_THOUSAND               = 0x0400000000000000LL, // Вместо делителя 1024 использовать делитель 1000
	COLUMN_BRIEF                  = 0x0200000000000000LL,
	COLUMN_MONTH                  = 0x0100000000000000LL,
	COLUMN_FLOATSIZE              = 0x0080000000000000LL, // Показывать размер файла в стиле Windows Explorer (т.е. 999 байт будут показаны как 999, а 1000 байт как 0.97 K)
	COLUMN_ECONOMIC               = 0x0040000000000000LL, // Экономичный режим, не показывать пробел перед суффиксом размера файла (т.е. 0.97K)
	COLUMN_MINSIZEINDEX           = 0x0020000000000000LL, // Минимально допустимый индекс при форматировании
	COLUMN_SHOWBYTESINDEX         = 0x0010000000000000LL, // Показывать суффиксы B,K,M,G,T,P,E
	COLUMN_FULLOWNER              = 0x0008000000000000LL,
	COLUMN_NOEXTENSION            = 0x0004000000000000LL,
	COLUMN_CENTERALIGN            = 0x0002000000000000LL,
	COLUMN_RIGHTALIGNFORCE        = 0x0001000000000000LL,

	COLUMN_MINSIZEINDEX_MASK      = 0x0000000000000003LL; // MINSIZEINDEX может быть только 0, 1, 2 или 3 (K,M,G,T), например, 1 - "размер как минимум в мегабайтах"


wchar_t* QuoteSpace(wchar_t *Str);
wchar_t* InsertQuote(wchar_t *Str);
void Unquote(wchar_t *Str);
wchar_t* RemoveLeadingSpaces(wchar_t *Str);
wchar_t * RemoveTrailingSpaces(wchar_t *Str);
wchar_t* RemoveExternalSpaces(wchar_t *Str);
wchar_t* QuoteSpaceOnly(wchar_t *Str);

string &QuoteSpace(string &strStr);
string& InsertQuote(string& strStr);
void Unquote(string &strStr);
string& InsertRegexpQuote(string& strStr);
void UnquoteExternal(string &strStr);
string& RemoveLeadingSpaces(string &strStr);
string& RemoveTrailingSpaces(string &strStr);
string & RemoveExternalSpaces(string &strStr);
string & RemoveUnprintableCharacters(string &strStr);
string& QuoteSpaceOnly(string &strStr);

string &RemoveChar(string &strStr,wchar_t Target,bool Dup=true);
wchar_t *InsertString(wchar_t *Str,int Pos,const wchar_t *InsStr,int InsSize=0);
int ReplaceStrings(string &strStr,const wchar_t *FindStr,const wchar_t *ReplStr,int Count=-1,bool IgnoreCase=false);

const wchar_t *GetCommaWord(const wchar_t *Src,string &strWord,wchar_t Separator=L',');

string& FarFormatText(const wchar_t *SrcText, int Width, string &strDestText, const wchar_t* Break, DWORD Flags);

void PrepareUnitStr();
string& FileSizeToStr(string &strDestStr, unsigned __int64 Size, int Width=-1, unsigned __int64 ViewFlags=COLUMN_COMMAS);
bool CheckFileSizeStringFormat(const wchar_t *FileSizeStr);
unsigned __int64 ConvertFileSizeString(const wchar_t *FileSizeStr);
string &FormatNumber(const wchar_t *Src, string &strDest, int NumDigits=0);
string &InsertCommas(unsigned __int64 li, string &strDest);

inline bool IsWordDiv(const wchar_t *WordDiv, wchar_t Chr) { return wcschr(WordDiv, Chr)!=nullptr; }

//   WordDiv  - набор разделителей слова в кодировке OEM
// возвращает указатель на начало слова
const wchar_t * const CalcWordFromString(const wchar_t *Str,int CurPos,int *Start,int *End,const wchar_t *WordDiv);

wchar_t* TruncStr(wchar_t *Str,int MaxLength);
wchar_t* TruncStrFromCenter(wchar_t *Str, int MaxLength);
wchar_t* TruncStrFromEnd(wchar_t *Str,int MaxLength);
wchar_t* TruncPathStr(wchar_t *Str, int MaxLength);

string& TruncStr(string &strStr,int MaxLength);
string& TruncStrFromEnd(string &strStr, int MaxLength);
string& TruncStrFromCenter(string &strStr, int MaxLength);
string& TruncPathStr(string &strStr, int MaxLength);

bool IsCaseMixed(const string &strStr);
bool IsCaseLower(const string &strStr);

string& CenterStr(const wchar_t *Src, string &strDest,int Length);

void Transform(string &strBuffer,const wchar_t *ConvStr,wchar_t TransformType);

wchar_t GetDecimalSeparator();

string ReplaceBrackets(const string& SearchStr,const string& ReplaceStr,RegExpMatch* Match,int Count);

string GuidToStr(const GUID& Guid);
bool StrToGuid(const string& Value,GUID& Guid);
