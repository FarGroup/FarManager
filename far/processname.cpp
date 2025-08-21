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

string ConvertWildcards(string_view const SrcName, string_view const Mask)
{
	const auto WildName = PointToName(Mask);
	if (WildName.find_first_of(L"*?"sv) == WildName.npos)
		return string(Mask);

	auto SrcNamePart = PointToName(SrcName);

	string Result;
	Result.reserve(SrcName.size());
	Result = Mask.substr(0, Mask.size() - WildName.size());

	const auto BeforeNameLength = Result.empty()? SrcName.size() - SrcNamePart.size() : 0;

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

	if (Result.ends_with(L'.'))
		Result.pop_back();

	Result.insert(0, SrcName.data(), BeforeNameLength);

	return Result;
}

bool CmpName(const string_view pattern, string_view str, const bool skippath, const bool CmpNameLegacyMode)
{
	// Special case for these simplest and most common masks
	if (pattern == L"*"sv || (CmpNameLegacyMode && pattern == L"*.*"sv))
		return true;

	if (pattern.empty() || str.empty())
		return pattern.empty() && str.empty(); // used to be `false` but this looks more accurate

	if (skippath)
		str = PointToName(str);

	size_t pi = 0, si = 0;
	auto StarPI = string_view::npos;
	auto StarSI = string_view::npos;

	const auto try_backtrack = [&]
	{
		if (StarPI == string_view::npos || StarSI >= str.size())
			return false;

		++StarSI;
		si = StarSI;
		pi = StarPI;
		return true;
	};

	const auto match_char_in_set = [&](const size_t SetStart, const size_t SetEnd, const wchar_t ch)
	{
		for (auto i = SetStart; i < SetEnd; ++i)
		{
			if (i + 2 < SetEnd && pattern[i + 1] == L'-')
			{
				if (ch >= upper(pattern[i]) && ch <= upper(pattern[i + 2]))
					return true;

				i += 2;
				continue;
			}

			if (string_comparer_icase()(ch, pattern[i]))
				return true;
		}

		return false;
	};

	while (si <= str.size())
	{
		if (pi >= pattern.size())
		{
			if (si == str.size())
				return true;

			if (!try_backtrack())
				return false;

			continue;
		}

		const wchar_t pc = pattern[pi];

		if (pc == L'*')
		{
			StarPI = ++pi;
			StarSI = si;
			continue;
		}

		if (pc == L'?')
		{
			if (si == str.size())
				return false;

			++pi;
			++si;
			continue;
		}

		if (pc == L'[')
		{
			const auto SetStart = pi + 1;
			auto SetEnd = SetStart;

			while (SetEnd < pattern.size() && pattern[SetEnd] != L']')
				++SetEnd;

			if (SetEnd != pattern.size())
			{
				if (SetEnd == SetStart)
				{
					pi = SetEnd + 1;
					continue;
				}

				if (si == str.size())
					return false;

				if (match_char_in_set(SetStart, SetEnd, upper(str[si])))
				{
					pi = SetEnd + 1;
					++si;

					continue;
				}

				if (!try_backtrack())
					return false;

				continue;
			}
		}

		if (si < str.size() && string_comparer_icase()(pc, str[si]))
		{
			++pi;
			++si;
			continue;
		}

		if (CmpNameLegacyMode && si == str.size() && pattern.substr(pi) == L".*"sv)
			return true;

		if (!try_backtrack())
			return false;
	}

	return false;
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
	L"test.*"sv,
	L"t*?."sv,
	L"t?*."sv,
	L"t*?.*"sv,
	L"*.*.*.txt"sv,
	L"[a-cf]*.txt"sv,
	L"*[a-cf].t[]x[]t"sv,
	L"t[est.txt"sv,
	L"*a*a*a*a*a*a*a*a*b"sv,
	L"[t-]*"sv,
	L"*?**??*???*.??*?"sv,
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
		REQUIRE(i.Expected == ConvertWildcards(i.Src, Masks[i.Mask]));
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

		{ 8, L"...txt"sv,                    false },
		{ 8, L"..txt"sv,                     false },
		{ 8, L".txt"sv,                      false },
		{ 8, L"a.txt"sv,                     false },
		{ 8, L"t.txt"sv,                     false },
		{ 8, L"test"sv,                      true  },
		{ 8, L"test."sv,                     true  },
		{ 8, L"test.."sv,                    true  },
		{ 8, L"test.b.txt"sv,                true  },
		{ 8, L"test.foo.bar.txt"sv,          true  },
		{ 8, L"test.md"sv,                   true  },
		{ 8, L"test.txt"sv,                  true  },

		{ 9, L"...txt"sv,                    false },
		{ 9, L"..txt"sv,                     false },
		{ 9, L".txt"sv,                      false },
		{ 9, L"a.txt"sv,                     false },
		{ 9, L"t.txt"sv,                     false },
		{ 9, L"test"sv,                      false },
		{ 9, L"test."sv,                     true  },
		{ 9, L"test.."sv,                    true  },
		{ 9, L"test.b.txt"sv,                false },
		{ 9, L"test.foo.bar.txt"sv,          false },
		{ 9, L"test.md"sv,                   false },
		{ 9, L"test.txt"sv,                  false },

		{ 10, L"...txt"sv,                   false },
		{ 10, L"..txt"sv,                    false },
		{ 10, L".txt"sv,                     false },
		{ 10, L"a.txt"sv,                    false },
		{ 10, L"t.txt"sv,                    false },
		{ 10, L"test"sv,                     false },
		{ 10, L"test."sv,                    true  },
		{ 10, L"test.."sv,                   true  },
		{ 10, L"test.b.txt"sv,               false },
		{ 10, L"test.foo.bar.txt"sv,         false },
		{ 10, L"test.md"sv,                  false },
		{ 10, L"test.txt"sv,                 false },

		{ 11, L"...txt"sv,                   false },
		{ 11, L"..txt"sv,                    false },
		{ 11, L".txt"sv,                     false },
		{ 11, L"a.txt"sv,                    false },
		{ 11, L"t.txt"sv,                    true  },
		{ 11, L"test"sv,                     true  },
		{ 11, L"test."sv,                    true  },
		{ 11, L"test.."sv,                   true  },
		{ 11, L"test.b.txt"sv,               true  },
		{ 11, L"test.foo.bar.txt"sv,         true  },
		{ 11, L"test.md"sv,                  true  },
		{ 11, L"test.txt"sv,                 true  },

		{ 12, L"...txt"sv,                   true  },
		{ 12, L"..txt"sv,                    false },
		{ 12, L".txt"sv,                     false },
		{ 12, L"a.txt"sv,                    false },
		{ 12, L"t.txt"sv,                    false },
		{ 12, L"test"sv,                     false },
		{ 12, L"test."sv,                    false },
		{ 12, L"test.."sv,                   false },
		{ 12, L"test.b.txt"sv,               false },
		{ 12, L"TEST.FOO.BAR.TXT"sv,         true  },
		{ 12, L"test.md"sv,                  false },
		{ 12, L"test.txt"sv,                 false },

		{ 13, L"a.txt"sv,                    true  },
		{ 13, L"bc.txt"sv,                   true  },
		{ 13, L"CDE.TXT"sv,                  true  },
		{ 13, L"e.txt"sv,                    false },
		{ 13, L"f.txt"sv,                    true  },
		{ 13, L"g.txt"sv,                    false },

		{ 14, L"a.txt"sv,                    true  },
		{ 14, L"BC.TXT"sv,                   true  },
		{ 14, L"cde.txt"sv,                  false },
		{ 14, L"e.txt"sv,                    false },
		{ 14, L"f.txt"sv,                    true  },
		{ 14, L"g.txt"sv,                    false },

		{ 15, L"test.txt"sv,                 false },
		{ 15, L"t[est.txt"sv,                true  },

		{ 16, L"aaaaaaaaaaaaaaaaaab"sv,      true  },
		{ 16, L"aaaaaaaaaaaaaaaaaaaac"sv,    false },

		{ 17, L"aaa.txt"sv,                  false },
		{ 17, L"t.txt"sv,                    true  },
		{ 17, L"test"sv,                     true  },
		{ 17, L"-.txt"sv,                    true  },
		{ 17, L"t-test.txt"sv,               true  },
		{ 17, L".txt"sv,                     false },

		{ 18, L"12345.txt"sv,                false },
		{ 18, L"123456.txt"sv,               true  },
		{ 18, L"12345678.txt"sv,             true  },
		{ 18, L"FOO.TEST.BAR.TXT"sv,         true  },
		{ 18, L"1.1.1.1.md"sv,               false },
		{ 18, L"1.1.1.1.text"sv,             true  },
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
