/*
string_utils.cpp

Сравнение без учета регистра, преобразование регистра
*/
/*
Copyright © 2017 Far Group
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

#include "string_utils.hpp"

const string& GetSpaces()
{
	// TODO: test for consistency with IsSpace()
	static const auto Spaces = L" \t"s;
	return Spaces;
}

const string& GetEols()
{
	// TODO: test for consistency with IsEol()
	static const auto Eols = L"\r\n"s;
	return Eols;
}

const string& GetSpacesAndEols()
{
	static const auto SpacesOrEols = GetSpaces() + GetEols();
	return SpacesOrEols;
}

bool is_alpha(wchar_t Char)
{
	return IsCharAlpha(Char) != FALSE;
}

bool is_alphanumeric(wchar_t Char)
{
	return IsCharAlphaNumeric(Char) != FALSE;
}

bool is_upper(wchar_t Char)
{
	return IsCharUpper(Char) != FALSE;
}

bool is_lower(wchar_t Char)
{
	return IsCharLower(Char) != FALSE;
}

wchar_t upper(wchar_t Char)
{
	CharUpperBuff(&Char, 1);
	return Char;
}

wchar_t lower(wchar_t Char)
{
	CharLowerBuff(&Char, 1);
	return Char;
}

void inplace::upper(wchar_t *Str, size_t Size)
{
	CharUpperBuff(Str, static_cast<DWORD>(Size));
}

void inplace::lower(wchar_t *Str, size_t Size)
{
	CharLowerBuff(Str, static_cast<DWORD>(Size));
}

void inplace::upper(wchar_t* Str)
{
	upper(Str, wcslen(Str));
}

void inplace::lower(wchar_t* Str)
{
	lower(Str, wcslen(Str));
}

string& inplace::upper(string& Str, size_t Pos, size_t Count)
{
	upper(&Str[Pos], Count == string::npos? Str.size() - Pos : Count);
	return Str;
}

string& inplace::lower(string& Str, size_t Pos, size_t Count)
{
	lower(&Str[Pos], Count == string::npos? Str.size() - Pos : Count);
	return Str;
}

string upper(string Str)
{
	return inplace::upper(Str, 0, string::npos);
}

string lower(string Str)
{
	return inplace::lower(Str, 0, string::npos);
}

size_t hash_icase_t::operator()(const string& Str) const
{
	return make_hash(upper(Str));
}

bool equal_icase_t::operator()(wchar_t Chr1, wchar_t Chr2) const
{
	return upper(Chr1) == upper(Chr2);
}

bool equal_icase_t::operator()(const string_view Str1, const string_view Str2) const
{
	return equal_icase(Str1, Str2);
}

bool equal_icase(const string_view Str1, const string_view Str2)
{
	return std::equal(ALL_CONST_RANGE(Str1), ALL_CONST_RANGE(Str2), equal_icase_t{});
}

bool starts_with_icase(const string_view Str, const string_view Prefix)
{
	return Str.size() >= Prefix.size() && equal_icase(Str.substr(0, Prefix.size()), Prefix);
}

bool ends_with_icase(const string_view Str, const string_view Suffix)
{
	return Str.size() >= Suffix.size() && equal_icase(Str.substr(Str.size() - Suffix.size()), Suffix);
}

bool contains_icase(const string_view Str, const string_view Token)
{
	return std::search(ALL_CONST_RANGE(Str), ALL_CONST_RANGE(Token), equal_icase_t{}) != Str.cend();
}
