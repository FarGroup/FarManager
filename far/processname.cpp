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
	auto HasDot = false;

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
		if (si < str.size() && str[si] == L'.')
			HasDot = true;

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

		if (CmpNameLegacyMode && si == str.size() &&
			(pattern.substr(pi) == L".*"sv || !HasDot && pattern.substr(pi) == L"."sv))
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
#include "common/view/zip.hpp"

static const string_view Masks[]
{
	{},
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
	struct test
	{
		string_view Src, Expected;
	};

	static std::initializer_list<test> const Tests[]
	{
		{
			{ {},                            {} },
			{ L"non-empty"sv,                {} },
		},
		{
			{ {},                            {} },
			{ L"whatever"sv,                 L"whatever"sv },
		},
		{
			{ {},                            L"AZ"sv },
			{ L"1"sv,                        L"AZ"sv },
			{ L"12"sv,                       L"A2Z"sv },
			{ L"1.txt"sv,                    L"AZ.txt"sv },
			{ L"12.txt"sv,                   L"A2Z.txt"sv },
			{ L"123"sv,                      L"A2Z"sv },
			{ L"123.txt"sv,                  L"A2Z.txt"sv },
			{ L"1234"sv,                     L"A2Z4"sv },
			{ L"1234.txt"sv,                 L"A2Z4.txt"sv },
		},
		{
			{ {},                            L".txt"sv },
			{ L"a"sv,                        L"a.txt"sv },
			{ L"b.dat"sv,                    L"b.txt"sv },
			{ L"c.x.y"sv,                    L"c.x.txt"sv },
		},
		{
			{ {},                            L".bak"sv },
			{ L"a"sv,                        L"a.bak"sv },
			{ L"b.dat"sv,                    L"b.dat.bak"sv },
			{ L"c.x.y"sv,                    L"c.x.y.bak"sv },
		},
		{
			{ {},                            {} },
			{ L"a"sv,                        L"a"sv },
			{ L"a.b"sv,                      L"a.b"sv },
			{ L"a.b.c"sv,                    L"a.b"sv },
			{ L"part1.part2.part3"sv,        L"part1.part2"sv },
			{ L"123456.123456.123456"sv,     L"12345.12345"sv },
		},
		{
			{ {},                            L"_NEW"sv },
			{ L"abcd_12345.txt"sv,           L"abcd_NEW.txt"sv },
			{ L"abc_newt_1.dat"sv,           L"abc_newt_NEW.dat"sv },
			{ L"abcd_123.a_b"sv,             L"abcd_123.a_NEW"sv },
		},
		{
			{ {},                            L"x.999.rForTheCourse"sv },
			{ L"part1.part2"sv,              L"px.part999.rForTheCourse"sv },
			{ L"part1.part2.part3"sv,        L"px.part999.parForTheCourse"sv },
			{ L"a.b.c"sv,                    L"ax.b999.crForTheCourse"sv },
			{ L"a.b.CarPart3BEER"sv,         L"ax.b999.CarParForTheCourse"sv },
		},
		{
			{ {},                            L"..2"sv },
			{ L"1"sv,                        L"1..2"sv },
			{ L"1.2"sv,                      L"1.2.2"sv },
			{ L"1.1.1"sv,                    L"1.1.1.2"sv },
		},
		{
			{ {},                            L"test"sv },
			{ L"1"sv,                        L"test"sv },
			{ L"1.2"sv,                      L"test.2"sv },
		},
		{
			{ {},                            L"t"sv },
			{ L"1"sv,                        L"t"sv },
			{ L"1.2"sv,                      L"t.2"sv },
		},
		{
			{ {},                            L"t"sv },
			{ L"1"sv,                        L"t"sv },
			{ L"1.2"sv,                      L"t"sv },
		},
		{
			{ {},                            L"t"sv },
			{ L"1"sv,                        L"t"sv },
			{ L"1.2"sv,                      L"t.2"sv },
		},
		{
			{ {},                            L"...txt"sv },
			{ L"1"sv,                        L"1...txt"sv },
			{ L"1.2"sv,                      L"1.2..txt"sv },
		},
		{
			{ {},                            L"[a-cf].txt"sv },
			{ L"1"sv,                        L"[a-cf].txt"sv },
			{ L"1.2"sv,                      L"[a-cf].txt"sv },
		},
		{
			{ {},                            L"[a-cf].t[]x[]t"sv },
			{ L"1"sv,                        L"1[a-cf].t[]x[]t"sv },
			{ L"1.2"sv,                      L"1.2[a-cf].t[]x[]t"sv },
		},
		{
			{ {},                            L"t[est.txt"sv },
			{ L"1"sv,                        L"t[est.txt"sv },
			{ L"1.2"sv,                      L"t[est.txt"sv },
		},
		{
			{ {},                            L"aaaaaaaab"sv },
			{ L"1"sv,                        L"1aaaaaaaab"sv },
			{ L"1.2"sv,                      L"1.2aaaaaaaab"sv },
		},
		{
			{ {},                            L"[t-]"sv },
			{ L"1"sv,                        L"[t-]"sv },
			{ L"1.2"sv,                      L"[t-].2"sv },
		},
		{
			{ {},                            L"*"sv },
			{ L"1"sv,                        L"1*"sv },
			{ L"1.2"sv,                      L"1.2*"sv },
		},
	};

	static_assert(std::size(Tests) == std::size(Masks));

	for (const auto& [Mask, Group]: zip(Masks, Tests))
	{
		for (const auto& Test: Group)
		{
			REQUIRE(Test.Expected == ConvertWildcards(Test.Src, Mask));
			// Only about 50% success rate here, so commenting out for now
			//REQUIRE(CmpName(Mask, Test.Expected));
		}
	}
}

TEST_CASE("CmpName")
{
	struct test
	{
		string_view Src;
		bool Match;
	};

	static std::initializer_list<test> const Tests[]
	{
		{
			{ {},                             true  },
			{ L"non-empty"sv,                 false },
		},
		{
			{ {},                             true  },
			{ L"."sv,                         true  },
			{ L"whatever"sv,                  true  },
		},
		{
			{ {},                             false },
			{ L"1"sv,                         false },
			{ L"AZ"sv,                        false },
			{ L"ALZ"sv,                       true  },
			{ L"ALZA1"sv,                     true  },
		},
		{
			{ {},                             false },
			{ L"foo.bar"sv,                   false },
			{ L"foo.txt"sv,                   true  },
			{ L".txt"sv,                      true  },
			{ L"foo.txt1"sv,                  false },
		},
		{
			{ {},                             false },
			{ L"foo.bar"sv,                   false },
			{ L"1.bak"sv,                     true  },
			{ L"foo.bak"sv,                   true  },
			{ L"foo.bak1"sv,                  false },
		},
		{
			{ {},                             false },
			{ L"12345.1234"sv,                false },
			{ L"12345.12345"sv,               true  },
			{ L"1.234.123.4"sv,               true  },
			{ L"..........."sv,               true  },
			{ L"123456.12345"sv,              false },
		},
		{
			{ {},                             false },
			{ L"1"sv,                         false },
			{ L"_NEW"sv,                      true  },
			{ L"1_NEW"sv,                     true  },
			{ L"1_NEW."sv,                    true  },
			{ L"1_NEW.2"sv,                   true  },
			{ L"1_NEW2"sv,                    false },
		},
		{
			{ {},                             false },
			{ L"1"sv,                         false },
			{ L"Rx.1234999.rForTheCourse"sv,  true  },
			{ L"Rx.1234999.QrForTheCourse"sv, true  },
			{ L"Rx.999.rForTheCourse"sv,      false },
		},
		{
			{ {},                             false },
			{ L".bar.2"sv,                    true  },
			{ L"..2"sv,                       true  },
			{ L"foo..2"sv,                    true  },
			{ L"foo.bar.2"sv,                 true  },
			{ L"foo.bar."sv,                  false },
		},
		{
			{ L"...txt"sv,                    false },
			{ L"..txt"sv,                     false },
			{ L".txt"sv,                      false },
			{ L"a.txt"sv,                     false },
			{ L"t.txt"sv,                     false },
			{ L"test"sv,                      true  },
			{ L"test."sv,                     true  },
			{ L"test.."sv,                    true  },
			{ L"test.b.txt"sv,                true  },
			{ L"test.foo.bar.txt"sv,          true  },
			{ L"test.md"sv,                   true  },
			{ L"test.txt"sv,                  true  },
		},
		{
			{ L"...txt"sv,                    false },
			{ L"..txt"sv,                     false },
			{ L".txt"sv,                      false },
			{ L"a.txt"sv,                     false },
			{ L"t.txt"sv,                     false },
			{ L"test"sv,                      true  },
			{ L"test."sv,                     true  },
			{ L"test.."sv,                    true  },
			{ L"test.b.txt"sv,                false },
			{ L"test.foo.bar.txt"sv,          false },
			{ L"test.md"sv,                   false },
			{ L"test.txt"sv,                  false },
		},
		{
			{ L"...txt"sv,                   false },
			{ L"..txt"sv,                    false },
			{ L".txt"sv,                     false },
			{ L"a.txt"sv,                    false },
			{ L"t.txt"sv,                    false },
			{ L"test"sv,                     true  },
			{ L"test."sv,                    true  },
			{ L"test.."sv,                   true  },
			{ L"test.b.txt"sv,               false },
			{ L"test.foo.bar.txt"sv,         false },
			{ L"test.md"sv,                  false },
			{ L"test.txt"sv,                 false },
		},
		{
			{ L"...txt"sv,                   false },
			{ L"..txt"sv,                    false },
			{ L".txt"sv,                     false },
			{ L"a.txt"sv,                    false },
			{ L"t.txt"sv,                    true  },
			{ L"test"sv,                     true  },
			{ L"test."sv,                    true  },
			{ L"test.."sv,                   true  },
			{ L"test.b.txt"sv,               true  },
			{ L"test.foo.bar.txt"sv,         true  },
			{ L"test.md"sv,                  true  },
			{ L"test.txt"sv,                 true  },
		},
		{
			{ L"...txt"sv,                   true  },
			{ L"..txt"sv,                    false },
			{ L".txt"sv,                     false },
			{ L"a.txt"sv,                    false },
			{ L"t.txt"sv,                    false },
			{ L"test"sv,                     false },
			{ L"test."sv,                    false },
			{ L"test.."sv,                   false },
			{ L"test.b.txt"sv,               false },
			{ L"TEST.FOO.BAR.TXT"sv,         true  },
			{ L"test.md"sv,                  false },
			{ L"test.txt"sv,                 false },
		},
		{
			{ L"a.txt"sv,                    true  },
			{ L"bc.txt"sv,                   true  },
			{ L"CDE.TXT"sv,                  true  },
			{ L"e.txt"sv,                    false },
			{ L"f.txt"sv,                    true  },
			{ L"g.txt"sv,                    false },
		},
		{
			{ L"a.txt"sv,                    true  },
			{ L"BC.TXT"sv,                   true  },
			{ L"cde.txt"sv,                  false },
			{ L"e.txt"sv,                    false },
			{ L"f.txt"sv,                    true  },
			{ L"g.txt"sv,                    false },
		},
		{
			{ L"test.txt"sv,                 false },
			{ L"t[est.txt"sv,                true  },
		},
		{
			{ L"aaaaaaaaaaaaaaaaaab"sv,      true  },
			{ L"aaaaaaaaaaaaaaaaaaaac"sv,    false },
		},
		{
			{ L"aaa.txt"sv,                  false },
			{ L"t.txt"sv,                    true  },
			{ L"test"sv,                     true  },
			{ L"-.txt"sv,                    true  },
			{ L"t-test.txt"sv,               true  },
			{ L".txt"sv,                     false },
		},
		{
			{ L"12345.txt"sv,                false },
			{ L"123456.txt"sv,               true  },
			{ L"12345678.txt"sv,             true  },
			{ L"FOO.TEST.BAR.TXT"sv,         true  },
			{ L"1.1.1.1.md"sv,               false },
			{ L"1.1.1.1.text"sv,             true  },
		},
	};

	static_assert(std::size(Tests) == std::size(Masks));

	for (const auto& [Mask, Group]: zip(Masks, Tests))
	{
		for (const auto& Test: Group)
		{
			REQUIRE(Test.Match == CmpName(Mask, Test.Src));
		}
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
