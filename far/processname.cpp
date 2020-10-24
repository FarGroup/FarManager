/*
processname.cpp

Обработать имя файла: сравнить с маской, масками, сгенерировать по маске
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
#include "processname.hpp"

// Internal:
#include "pathmix.hpp"
#include "string_utils.hpp"

// Platform:

// Common:
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

/* $ 09.10.2000 IS
    Генерация нового имени по маске
    (взял из ShellCopy::ShellCopyConvertWildcards)
*/
// На основе имени файла (SrcName) и маски (Mask) генерируем новое имя
// SelectedFolderNameLength - длина каталога. Например, есть
// каталог dir1, а в нем файл file1. Нужно сгенерировать имя по маске для dir1.
// Параметры могут быть следующими: Src="dir1", SelectedFolderNameLength=0
// или Src="dir1\\file1", а SelectedFolderNameLength=4 (длина "dir1")
string ConvertWildcards(string_view const SrcName, string_view const Mask, int const SelectedFolderNameLength)
{
	const auto WildName = PointToName(Mask);
	if (WildName.find_first_of(L"*?"sv) == WildName.npos)
		return string(Mask);

	const auto Src = SelectedFolderNameLength? SrcName.substr(0, SelectedFolderNameLength) : SrcName;
	auto SrcNamePart = PointToName(Src);

	string Result;
	Result.reserve(SrcName.size());
	Result = Mask.substr(0, Mask.size() - WildName.size());

	const auto BeforeNameLength = Result.empty()? 0 : Src.size() - SrcNamePart.size();

	auto WildPtr = WildName;

	// https://superuser.com/questions/475874/how-does-the-windows-rename-command-interpret-wildcards

	while (!WildPtr.empty())
	{
		switch (WildPtr.front())
		{
		case L'?':
			WildPtr.remove_prefix(1);

			if (!SrcNamePart.empty() && SrcNamePart.front() != L'.')
			{
				Result.push_back(SrcNamePart.front());
				SrcNamePart.remove_prefix(1);
			}
			break;

		case L'*':
			{
				WildPtr.remove_prefix(1);
				if (!WildPtr.empty())
				{
					const auto Char = WildPtr.front();
					WildPtr.remove_prefix(1);
					size_t LastCharPos;
					if (Char == L'?')
						LastCharPos = SrcNamePart.size();
					else
					{
						LastCharPos = SrcNamePart.rfind(Char);
						if (LastCharPos == SrcNamePart.npos)
							LastCharPos = SrcNamePart.size();
					}
					std::copy_n(SrcNamePart.cbegin(), LastCharPos, std::back_inserter(Result));
					if (Char != L'?')
						Result.push_back(Char);
					SrcNamePart.remove_prefix(LastCharPos);
					if (!SrcNamePart.empty())
						SrcNamePart.remove_prefix(1);
				}
				else
				{
					append(Result, SrcNamePart);
				}
			}
			break;

		case L'.':
			{
				WildPtr.remove_prefix(1);
				Result.push_back(L'.');

				auto FirstDotPos = SrcNamePart.find(L'.');
				if (FirstDotPos == SrcNamePart.npos)
					FirstDotPos = SrcNamePart.size();
				else
					++FirstDotPos;

				SrcNamePart.remove_prefix(FirstDotPos);
			}
			break;

		default:
			Result.push_back(WildPtr.front());
			WildPtr.remove_prefix(1);
			if (!SrcNamePart.empty() && SrcNamePart.front() != L'.')
				SrcNamePart.remove_prefix(1);
			break;
		}
	}

	if (ends_with(Result, L'.'))
		Result.pop_back();

	Result.insert(0, Src.data(), BeforeNameLength);

	if (SelectedFolderNameLength)
	{
		append(Result, SrcName.substr(SelectedFolderNameLength)); //BUGBUG???, was src in 1.7x
	}

	return Result;
}

bool CmpName(string_view pattern, string_view str, const bool skippath, const bool CmpNameLegacyMode)
{
	// BUGBUG rewrite

	// Special case for these simplest and most common masks:
	if (pattern == L"*"sv || (CmpNameLegacyMode && pattern == L"*.*"sv))
		return true;

	if (pattern.empty() || str.empty())
		return false;

	if (skippath)
		str = PointToName(str);

	for (;; str.remove_prefix(1))
	{
		if (pattern.empty())
			return str.empty();

		const auto stringc = str.empty()? 0 : upper(str.front());
		const auto patternc = upper(pattern.front());
		pattern.remove_prefix(1);

		switch (patternc)
		{
		case L'?':
			if (str.empty())
				return false;
			break;

		case L'*':
			if (pattern.empty())
				return true;

			/* $ 01.05.2001 DJ
				оптимизированная ветка работает и для имен с несколькими
				точками
			*/
			if (pattern[0] == L'.')
			{
				if (pattern.size() == 2 && pattern[1]==L'*')
					return true;

				if (std::none_of(ALL_CONST_RANGE(pattern), [](wchar_t Char) { return contains(L"*?["sv, Char); }))
				{
					const auto RDotIt = std::find(ALL_CONST_REVERSE_RANGE(str), L'.');
					const auto DotIt = RDotIt == str.crend()? str.cend() : (RDotIt + 1).base();

					if (pattern.size() == 1)
						return DotIt == str.cend() || DotIt + 1 == str.cend();

					const auto PatternContainsDot = contains(pattern.substr(1), L'.');

					if (PatternContainsDot && DotIt == str.cend())
						return false;

					if (!PatternContainsDot && DotIt != str.cend())
						return equal_icase(pattern.substr(1), str.substr(DotIt + 1 - str.cbegin()));
				}
			}

			for(;;)
			{
				if(CmpName(pattern, str, false, CmpNameLegacyMode))
					return true;

				if (str.empty())
					break;

				str.remove_prefix(1);
			}

			return false;

		case L'[':
			{
				if (!contains(pattern, L']'))
				{
					if (patternc != stringc)
						return false;

					break;
				}

				if (pattern.size() > 1 && pattern[1] == L']')
				{
					if (str.empty() || pattern[0] != str[0])
						return false;

					pattern.remove_prefix(2);
					break;
				}

				int match = 0;

				for(;;)
				{
					if (pattern.empty())
						return false;

					const auto rangec = upper(pattern[0]);

					pattern.remove_prefix(1);

					if (rangec == L']')
					{
						if (match)
							break;
						else
							return false;
					}

					if (match)
						continue;

					// BUGBUG data() - 2 is legal but awful
					if (rangec == L'-' && *(pattern.data() - 2) != L'[' && pattern[0] != L']')
					{
						match = (stringc <= upper(pattern[0]) &&
									upper(*(pattern.data() - 2)) <= stringc);
						pattern.remove_prefix(1);
					}
					else
						match = (stringc == rangec);
				}
			}
			break;

		default:
			if (patternc != stringc)
			{
				return CmpNameLegacyMode && str.empty() && patternc == L'.' && pattern == L"*"sv;
			}
			break;
		}
	}
}

string exclude_sets(string_view const Str)
{
	string Result;
	Result.reserve(Str.size());

	for (const auto i: Str)
	{
		switch (i)
		{
		case L'[':
		case L']':
			append(Result, L'[', i, L']');
			break;

		default:
			Result.push_back(i);
			break;
		}
	}

	return Result;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

static const string_view Masks[]
{
	L"*"sv,
	L"A?Z*"sv,
	L"*.txt"sv,
	L"*?.bak"sv,
	L"?????.?????"sv,
	L"*_NEW.*"sv,
	L"?x.????999.*rForTheCourse"sv,
	L"*.*.2"sv,
};

TEST_CASE("ConvertWildcards")
{
	static const struct
	{
		size_t Mask;
		string_view Src, Expected;
	}
	Tests[]
	{
		{ 0, {},                            {} },
		{ 0, L"whatever"sv,                 L"whatever"sv },

		{ 1, L"1"sv,                        L"AZ"sv },
		{ 1, L"12"sv,                       L"A2Z"sv },
		{ 1, L"1.txt"sv,                    L"AZ.txt"sv },
		{ 1, L"12.txt"sv,                   L"A2Z.txt"sv },
		{ 1, L"123"sv,                      L"A2Z"sv },
		{ 1, L"123.txt"sv,                  L"A2Z.txt"sv },
		{ 1, L"1234"sv,                     L"A2Z4"sv },
		{ 1, L"1234.txt"sv,                 L"A2Z4.txt"sv },

		{ 2, L"a"sv,                        L"a.txt"sv },
		{ 2, L"b.dat"sv,                    L"b.txt"sv },
		{ 2, L"c.x.y"sv,                    L"c.x.txt"sv },

		{ 3, L"a"sv,                        L"a.bak"sv },
		{ 3, L"b.dat"sv,                    L"b.dat.bak"sv },
		{ 3, L"c.x.y"sv,                    L"c.x.y.bak"sv },

		{ 4, L"a"sv,                        L"a"sv },
		{ 4, L"a.b"sv,                      L"a.b"sv },
		{ 4, L"a.b.c"sv,                    L"a.b"sv },
		{ 4, L"part1.part2.part3"sv,        L"part1.part2"sv },
		{ 4, L"123456.123456.123456"sv,     L"12345.12345"sv },

		{ 5, L"abcd_12345.txt"sv,           L"abcd_NEW.txt"sv },
		{ 5, L"abc_newt_1.dat"sv,           L"abc_newt_NEW.dat"sv },
		{ 5, L"abcd_123.a_b"sv,             L"abcd_123.a_NEW"sv },

		{ 6, L"part1.part2"sv,              L"px.part999.rForTheCourse"sv },
		{ 6, L"part1.part2.part3"sv,        L"px.part999.parForTheCourse"sv },
		{ 6, L"a.b.c"sv,                    L"ax.b999.crForTheCourse"sv },
		{ 6, L"a.b.CarPart3BEER"sv,         L"ax.b999.CarParForTheCourse"sv },

		{ 7, L"1.1.1"sv,                    L"1.1.1.2"sv },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Expected == ConvertWildcards(i.Src, Masks[i.Mask], 0));
	}
}

TEST_CASE("CmpName")
{
	static const struct
	{
		size_t Mask;
		string_view Src;
		bool Match;
	}
	Tests[]
	{
		{ 0, {},                             true  },
		{ 0, L"."sv,                         true  },
		{ 0, L"whatever"sv,                  true  },

		{ 1, {},                             false },
		{ 1, L"1"sv,                         false },
		{ 1, L"AZ"sv,                        false },
		{ 1, L"ALZ"sv,                       true  },
		{ 1, L"ALZA1"sv,                     true  },

		{ 2, {},                             false },
		{ 2, L"foo.bar"sv,                   false },
		{ 2, L"foo.txt"sv,                   true  },
		{ 2, L".txt"sv,                      true  },
		{ 2, L"foo.txt1"sv,                  false },

		{ 3, {},                             false },
		{ 3, L"foo.bar"sv,                   false },
		{ 3, L"1.bak"sv,                     true  },
		{ 3, L"foo.bak"sv,                   true  },
		{ 3, L"foo.bak1"sv,                  false },

		{ 4, {},                             false },
		{ 4, L"12345.1234"sv,                false },
		{ 4, L"12345.12345"sv,               true  },
		{ 4, L"1.234.123.4"sv,               true  },
		{ 4, L"..........."sv,               true  },
		{ 4, L"123456.12345"sv,              false },

		{ 5, {},                             false },
		{ 5, L"1"sv,                         false },
		{ 5, L"_NEW"sv,                      true  },
		{ 5, L"1_NEW"sv,                     true  },
		{ 5, L"1_NEW."sv,                    true  },
		{ 5, L"1_NEW.2"sv,                   true  },
		{ 5, L"1_NEW2"sv,                    false },

		{ 6, {},                             false },
		{ 6, L"1"sv,                         false },
		{ 6, L"Rx.1234999.rForTheCourse"sv,  true  },
		{ 6, L"Rx.1234999.QrForTheCourse"sv, true  },
		{ 6, L"Rx.999.rForTheCourse"sv,      false },

		{ 7, {},                             false },
		{ 7, L".bar.2"sv,                    true  },
		{ 7, L"..2"sv,                       true  },
		{ 7, L"foo..2"sv,                    true  },
		{ 7, L"foo.bar.2"sv,                 true  },
		{ 7, L"foo.bar."sv,                  false },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Match == CmpName(Masks[i.Mask], i.Src));
	}
}

TEST_CASE("exclude_sets")
{
	static const struct
	{
		string_view Src, Result;
	}
	Tests[]
	{
		{ L""sv,          L""sv                   },
		{ L"-"sv,         L"-"sv                  },
		{ L"["sv,         L"[[]"sv                },
		{ L"]"sv,         L"[]]"sv                },
		{ L"[]"sv,        L"[[][]]"sv             },
		{ L"[[]]"sv,      L"[[][[][]][]]"sv       },
		{ L"[a[b]c]"sv,   L"[[]a[[]b[]]c[]]"sv    },
		{ L"[]]]"sv,      L"[[][]][]][]]"sv       },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(exclude_sets(i.Src) == i.Result);
	}
}
#endif
