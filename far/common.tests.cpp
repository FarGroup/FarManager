// validator: no-self-include
/*
common.tests.cpp

Tests for common library

*/
/*
Copyright © 2019 Far Group
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

#ifdef ENABLE_TESTS

// Internal:
#include "testing.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

#include "common/2d/matrix.hpp"

TEST_CASE("2d/matrix")
{
	static const int Data[]
	{
		0,  1,  2,  3,
		4,  5,  6,  7,
		8,  9, 10, 11,
	};

	matrix_view Matrix(Data, 3, 4);

	int Counter = 0;

WARNING_PUSH()
WARNING_DISABLE_CLANG("-Wrange-loop-analysis")
	for (const auto& Row: Matrix)
WARNING_POP()
	{
		for (const auto& Cell : Row)
		{
			REQUIRE(Cell == Counter);
			++Counter;
		}
	}
}

//----------------------------------------------------------------------------

#include "common/bytes_view.hpp"

TEST_CASE("bytes_view")
{
	uint32_t Value = 0;
	const auto BytesRef = bytes::reference(Value);
	std::fill(BytesRef.begin(), BytesRef.end(), '\x42');
	REQUIRE(Value == 0x42424242);

	auto BytesCopy = bytes::copy(Value);
	std::fill(BytesCopy.begin(), BytesCopy.end(), '\x33');
	REQUIRE(Value == 0x42424242);
	const auto NewValue = deserialise<uint32_t>(BytesCopy);
	REQUIRE(NewValue == 0x33333333);

	uint8_t SmallerValue = 1;
	auto SmallerView = bytes_view(SmallerValue);
	auto SmallerRef = bytes::reference(SmallerValue);
	REQUIRE_THROWS_AS(BytesCopy = SmallerView, std::runtime_error);
	REQUIRE_THROWS_AS(SmallerRef = bytes_view(Value), std::runtime_error);
	REQUIRE_NOTHROW(SmallerView = BytesCopy);
}

//----------------------------------------------------------------------------

#include "common/chrono.hpp"

TEST_CASE("chrono")
{
	using namespace std::chrono;

	const auto Duration = 47h + 63min + 71s + 3117ms;

	{
		const auto Result = split_duration<chrono::days, hours, minutes, seconds, milliseconds>(Duration);
		REQUIRE(Result.get<chrono::days>() == chrono::days{2});
		REQUIRE(Result.get<hours>() == 0h);
		REQUIRE(Result.get<minutes>() == 4min);
		REQUIRE(Result.get<seconds>() == 14s);
		REQUIRE(Result.get<milliseconds>() == 117ms);
	}

	{
		const auto Result = split_duration<hours>(Duration);
		REQUIRE(Result.get<hours>() == 48h);
	}

	{
		const auto Result = split_duration<seconds>(Duration);
		REQUIRE(Result.get<seconds>() == 173054s);
	}
}

//----------------------------------------------------------------------------

#include "common/enum_tokens.hpp"

TEST_CASE("enum_tokens")
{
	{
		const std::array Baseline = { L"abc"sv, L""sv, L"def"sv, L" q "sv, L"123"sv };
		auto BaselineIterator = Baseline.begin();

		for (const auto& i: enum_tokens(L"abc;,def; q ,123;"sv, L",;"sv))
		{
			REQUIRE(i == *BaselineIterator++);
		}

		REQUIRE(BaselineIterator == Baseline.end());
	}

	{
		const std::array Baseline = { L"abc;"sv, L"de;,f"sv, L"123"sv, L""sv };
		auto BaselineIterator = Baseline.begin();

		for (const auto& i: enum_tokens_with_quotes(L"\"abc;\",\"de;,f\";123;;"sv, L",;"sv))
		{
			REQUIRE(i == *BaselineIterator++);
		}

		REQUIRE(BaselineIterator == Baseline.end());
	}

	{
		const std::array Baseline = { L"abc"sv, L"def"sv, L""sv };
		auto BaselineIterator = Baseline.begin();

		for (const auto& i: enum_tokens_custom_t<with_trim>(L"  abc|   def  |  "sv, L"|"sv))
		{
			REQUIRE(i == *BaselineIterator++);
		}

		REQUIRE(BaselineIterator == Baseline.end());
	}

}

//----------------------------------------------------------------------------

#include "common/enum_substrings.hpp"

TEST_CASE("enum_substrings")
{
	const std::array Baseline = { L"abc"sv, L"def"sv, L"q"sv };
	auto BaselineIterator = Baseline.begin();

	for (const auto& i: enum_substrings(L"abc\0def\0q\0"sv.data()))
	{
		REQUIRE(i == *BaselineIterator++);
	}

	REQUIRE(BaselineIterator == Baseline.end());
}

//----------------------------------------------------------------------------

#include "common/from_string.hpp"

TEST_CASE("from_string")
{
	{
		size_t Pos = 0;
		REQUIRE(from_string<int>(L"0x42qqq"sv, &Pos, 16) == 66);
		REQUIRE(Pos == 4u);
	}

	REQUIRE(from_string<int>(L"-1"sv) == -1);
	REQUIRE(from_string<unsigned int>(L"4294967295"sv) == 4294967295u);
	REQUIRE(from_string<unsigned long long>(L"18446744073709551615"sv) == 18446744073709551615ull);

	REQUIRE_THROWS_AS(from_string<short>(L"32768"sv), std::out_of_range);
	REQUIRE_THROWS_AS(from_string<unsigned int>(L"4294967296"sv), std::out_of_range);
	REQUIRE_THROWS_AS(from_string<unsigned int>(L"-42"sv), std::out_of_range);
	REQUIRE_THROWS_AS(from_string<int>(L"fubar"sv), std::invalid_argument);
	REQUIRE_THROWS_AS(from_string<int>({}), std::invalid_argument);
	REQUIRE_THROWS_AS(from_string<int>(L" 42"sv), std::invalid_argument);
	REQUIRE_THROWS_AS(from_string<int>(L" +42"sv), std::invalid_argument);

	{
		int Value;
		REQUIRE(!from_string(L"qqq"sv, Value));
	}
}

//----------------------------------------------------------------------------

#include "common/keep_alive.hpp"

template<typename type>
static void TestKeepAlive()
{
	static_assert(std::is_same_v<decltype(keep_alive(std::declval<type>())), keep_alive<type>>);
}

TEST_CASE("keep_alive")
{
	TestKeepAlive<int>();
	TestKeepAlive<int&>();
	TestKeepAlive<const int&>();

	SUCCEED();
}

//----------------------------------------------------------------------------

#include "common/io.hpp"

TEST_CASE("io")
{
	std::stringstream Stream;
	constexpr auto Str = "12345"sv;
	REQUIRE_NOTHROW(io::write(Stream, Str));

	char Buffer[Str.size()];
	REQUIRE(io::read(Stream, Buffer) == Str.size());
	REQUIRE(!io::read(Stream, Buffer));
}

//----------------------------------------------------------------------------

#include "common/monitored.hpp"

TEST_CASE("monitored")
{
	monitored<int> a(42);
	REQUIRE(!a.touched());
	a = 33;
	REQUIRE(a.touched());
	REQUIRE(a == 33);
	a.forget();
	REQUIRE(!a.touched());
	REQUIRE(a == 33);
}

//----------------------------------------------------------------------------

#include "common/movable.hpp"

TEST_CASE("movable")
{
	const auto Value = 42;
	movable<int> m1 = Value;
	REQUIRE(m1 == Value);

	const auto m2 = std::move(m1);
	REQUIRE(m2 == Value);
	REQUIRE(m1 == 0);
}

//----------------------------------------------------------------------------

#include "common/null_iterator.hpp"

TEST_CASE("null_iterator")
{
	const auto Ptr = L"12345";
	const null_iterator Iterator(Ptr);
	const size_t Count = std::distance(Iterator, Iterator.end());
	REQUIRE(Count == wcslen(Ptr));
}

TEST_CASE("placement")
{
	struct raii
	{
		explicit raii(int& Value):
			m_Value(Value)
		{
			m_Value = 42;
		}

		~raii()
		{
			m_Value = 33;
		}

		int& m_Value;
	};

	std::aligned_storage_t<sizeof(raii), alignof(raii)> Data;
	auto& Object = reinterpret_cast<raii&>(Data);

	int Value = 0;
	placement::construct(Object, Value);
	REQUIRE(Value == 42);

	placement::destruct(Object);
	REQUIRE(Value == 33);
}

//----------------------------------------------------------------------------

#include "common/range.hpp"

TEST_CASE("range.static")
{
	{
		const auto Test = [](auto&& Container)
		{
			const auto TestImpl = [](auto& ContainerVersion)
			{
				auto Range = range(ContainerVersion);

				const auto TestType = [&](const auto & ContainerGetter, const auto & RangeGetter)
				{
					static_assert(std::is_same_v<decltype(ContainerGetter(ContainerVersion)), decltype(RangeGetter(Range))>);
				};

// std::cbegin and friends are broken in the standard for shallow-const containers, thus the member version.
#define TEST_TYPE(x) TestType(LIFT(std::x), LIFT_MF(x))
#define TEST_ALL_ACCESSORS(callable, x) callable(x), callable(c##x), callable(r##x), callable(cr##x)

				TEST_ALL_ACCESSORS(TEST_TYPE, begin);
				TEST_ALL_ACCESSORS(TEST_TYPE, end);

#undef TEST_ALL_ACCESSORS
#undef TEST_TYPE
			};

			TestImpl(Container);
			TestImpl(std::as_const(Container));
		};

		{ Test(std::vector<int>{}); }
		{ Test(std::list<int>{}); }
		using ints = int[2];
		{ Test(ints{}); }
	}

	{
		int Data[2]{};
		range Range(std::begin(Data), std::end(Data));
		static_assert(std::is_same_v<decltype(*Range.begin()), int&>);
		static_assert(std::is_same_v<decltype(*Range.cbegin()), const int&>);
	}

	{
		std::vector<int> Data;
		range Range(std::begin(Data), std::end(Data));
		static_assert(std::is_same_v<decltype(*Range.begin()), int&>);
		// It's not possible to deduce const_iterator here
		static_assert(std::is_same_v<decltype(*Range.cbegin()), int&>);
	}

	{
		int Data[2]{};
		span Span(Data);
		static_assert(std::is_same_v<decltype(*Span.begin()), int&>);
		static_assert(std::is_same_v<decltype(*Span.cbegin()), const int&>);
	}
	SUCCEED();
}

TEST_CASE("range.dynamic")
{
	{
		std::array Value{ 1, 2, 3, 4, 5 };
		range Range(Value);
		REQUIRE(Range.size() == Value.size());
		REQUIRE(Range.data() == Value.data());

		Range.pop_front();
		REQUIRE(Range.size() == Value.size() - 1);
		REQUIRE(Range.data() == Value.data() + 1);

		Range.pop_back();
		REQUIRE(Range.size() == Value.size() - 2);
		REQUIRE(Range.data() == Value.data() + 1);
	}

	{
		const auto Begin = 3, End = 7;
		const irange Range(Begin, End);
		auto Iterator = Range.begin();

		for (auto i = Begin; i != End; ++i, ++Iterator)
		{
			REQUIRE(Iterator != Range.cend());
			REQUIRE(*Iterator == i);
		}

		REQUIRE(Iterator == Range.cend());
	}
}

//----------------------------------------------------------------------------

#include "common/scope_exit.hpp"

namespace
{
	template<scope_exit::scope_type type>
	static void test_scope_impl(bool const Throw, bool const MustBeTriggered)
	{
		bool IsTriggered = false;

		try
		{
			SCOPE_TYPE(type) { IsTriggered = true; };
			REQUIRE(!IsTriggered);
			if (Throw)
				throw 1;
		}
		catch (int)
		{
		}

		REQUIRE(IsTriggered == MustBeTriggered);
	}

	template<scope_exit::scope_type type>
	static void test_scope(bool OnFail, bool OnSuccess)
	{
		test_scope_impl<type>(true, OnFail);
		test_scope_impl<type>(false, OnSuccess);
	}
}
TEST_CASE("scope_exit")
{
	test_scope<scope_exit::scope_type::exit>(true, true);
	test_scope<scope_exit::scope_type::fail>(true, false);
	test_scope<scope_exit::scope_type::success>(false, true);
}

//----------------------------------------------------------------------------

#include "common/string_utils.hpp"

TEST_CASE("string_utils.cut")
{
	static const struct
	{
		string_view Src;
		size_t Size;
		string_view ResultLeft, ResultRight;
	}
	Tests[]
	{
		{ L"12345"sv, 6, L"12345"sv, L"12345"sv, },
		{ L"12345"sv, 3, L"345"sv,   L"123"sv,   },
		{ L"12345"sv, 1, L"5"sv,     L"1"sv,     },
		{ L"12345"sv, 0, {},         {},         },

		{ L"1"sv,     2, L"1"sv,     L"1"sv,     },
		{ L"1"sv,     1, L"1"sv,     L"1"sv,     },
		{ L"1"sv,     0, {},         {},         },

		{ {},         1, {},         {},         },
		{ {},         0, {},         {},         },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(cut_left(i.Src, i.Size) == i.ResultLeft);
		REQUIRE(cut_right(i.Src, i.Size) == i.ResultRight);
	}
}

TEST_CASE("string_utils.pad")
{
	static const struct
	{
		string_view Src;
		size_t Size;
		string_view ResultLeft, ResultRight;
	}
	Tests[]
	{
		{ L"123"sv, 5, L"  123"sv, L"123  "sv, },
		{ L"123"sv, 2, L"123"sv,   L"123"sv,   },
		{ L"123"sv, 0, L"123"sv,   L"123"sv,   },

		{ L"1"sv,   3, L"  1"sv,   L"1  "sv,   },
		{ L"1"sv,   1, L"1"sv,     L"1"sv,     },
		{ L"1"sv,   0, L"1"sv,     L"1"sv,     },

		{ {},       2, L"  "sv,    L"  "sv,    },
		{ {},       1, L" "sv,     L" "sv,     },
		{ {},       0, {},         {},         },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(pad_left(string(i.Src), i.Size) == i.ResultLeft);
		REQUIRE(pad_right(string(i.Src), i.Size) == i.ResultRight);
	}
}

TEST_CASE("string_utils.trim")
{
	static const struct
	{
		string_view Src;
		string_view ResultLeft, ResultRight, Result;
	}
	Tests[]
	{
		{ L"12345"sv,     L"12345"sv,   L"12345"sv,   L"12345"sv, },
		{ L"  12345"sv,   L"12345"sv,   L"  12345"sv, L"12345"sv, },
		{ L"12345  "sv,   L"12345  "sv, L"12345"sv,   L"12345"sv, },
		{ L"  12345  "sv, L"12345  "sv, L"  12345"sv, L"12345"sv, },
		{ L" "sv,         {},           {},           {},         },
		{ {},             {},           {},           {},         },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(trim_left(string(i.Src)) == i.ResultLeft);
		REQUIRE(trim_right(string(i.Src)) == i.ResultRight);
		REQUIRE(trim(string(i.Src)) == i.Result);
	}
}

TEST_CASE("string_utils.fit")
{
	static const struct
	{
		string_view Src;
		size_t Size;
		string_view ResultLeft, ResultCenter, ResultRight;
	}
	Tests[]
	{
		{ L"12345"sv,  7,   L"12345  "sv,   L" 12345 "sv,   L"  12345"sv, },
		{ L"12345"sv,  5,   L"12345"sv,     L"12345"sv,     L"12345"sv,   },
		{ L"12345"sv,  3,   L"123"sv,       L"123"sv,       L"123"sv,     },
		{ L"12345"sv,  1,   L"1"sv,         L"1"sv,         L"1"sv,       },
		{ L"12345"sv,  0,   {},             {},             {},           },

		{ L"1"sv,      2,   L"1 "sv,        L"1 "sv,        L" 1"sv,      },
		{ L"1"sv,      1,   L"1"sv,         L"1"sv,         L"1"sv,       },
		{ L"1"sv,      0,   {},             {},             {},           },

		{ {},          2,   L"  "sv,        L"  "sv,        L"  "sv,      },
		{ {},          1,   L" "sv,         L" "sv,         L" "sv,       },
		{ {},          0,   {},             {},             {},           },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(fit_to_left(string(i.Src), i.Size) == i.ResultLeft);
		REQUIRE(fit_to_center(string(i.Src), i.Size) == i.ResultCenter);
		REQUIRE(fit_to_right(string(i.Src), i.Size) == i.ResultRight);
	}
}

TEST_CASE("string_utils.contains")
{
	static const struct
	{
		string_view Src;
		string_view Pattern;
		bool Starts, Ends, Contains;
	}
	Tests[]
	{
		{ L"12345"sv,      L"123456"sv, false,  false,  false,  },
		{ L"12345"sv,      L"12345"sv,  true,   true,   true,   },
		{ L"12345"sv,      L"123"sv,    true,   false,  true,   },
		{ L"12345"sv,      L"345"sv,    false,  true,   true,   },
		{ L"12345"sv,      L"234"sv,    false,  false,  true,   },
		{ L"12345"sv,      {},          true,   true,   true,   },
		{ L"12345"sv,      L"foo"sv,    false,  false,  false,  },
		{ L"12345"sv,      L"24"sv,     false,  false,  false,  },

		{ {},              L"1"sv,      false,  false,  false,  },
		{ {},              {},          true,   true,   true,   },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(starts_with(i.Src, i.Pattern) == i.Starts);
		REQUIRE(ends_with(i.Src, i.Pattern) == i.Ends);
		REQUIRE(contains(i.Src, i.Pattern) == i.Contains);
	}
}

TEST_CASE("string_utils.quotes")
{
	REQUIRE(unquote(LR"("12"345")"s) == L"12345"sv);
	REQUIRE(unquote(L"12345"s) == L"12345"sv);
	REQUIRE(unquote(L""s).empty());

	REQUIRE(quote(L"12345"s) == LR"("12345")"sv);
	REQUIRE(quote(LR"("12345")"s) == LR"("12345")"sv);
	REQUIRE(quote(L""s) == LR"("")"sv);
	REQUIRE(quote(LR"("")"s) == LR"("")"sv);

	REQUIRE(quote_unconditional(LR"("12345")"s) == LR"(""12345"")"sv);
	REQUIRE(quote_unconditional(L"12345"s) == LR"("12345")"sv);
	REQUIRE(quote_unconditional(L""s) == LR"("")"sv);

	REQUIRE(quote_normalise(LR"("12"345")"s) == LR"("12345")"sv);
	REQUIRE(quote_normalise(L"12345"s) == LR"("12345")"sv);
	REQUIRE(quote_normalise(L""s) == LR"("")"sv);
}

TEST_CASE("string_utils.split_name_value")
{
	static const struct
	{
		string_view Src;
		std::pair<string_view, string_view> Result;
	}
	Tests[]
	{
		{ L"foo=bar"sv,    { L"foo"sv,   L"bar"sv,   }, },
		{ L"foo=bar="sv,   { L"foo"sv,   L"bar="sv,  }, },
		{ L"=foo"sv,       { {},         L"foo"sv,   }, },
		{ L"==foo"sv,      { {},         L"=foo"sv,  }, },
		{ L"foo="sv,       { L"foo"sv,   {},         }, },
		{ L"foo=="sv,      { L"foo"sv,   L"="sv,     }, },
		{ L"="sv,          { {},         {},         }, },
		{ {},              { {},         {},         }, },
		{ L"foo"sv,        { L"foo"sv,   L"foo"sv,   }, },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(split_name_value(i.Src) == i.Result);
	}
}

TEST_CASE("string_utils.misc")
{
	REQUIRE(!null_terminated(L"12345"sv.substr(0, 2)).c_str()[2]);

	{
		auto Str = L"Sempre assim em cima, em cima, em cima, em cima"s;
		string_copyref StrCr1(Str);
		REQUIRE(string_view(StrCr1).data() == Str.data());

		string_copyref StrCr2(std::move(Str));
		REQUIRE(Str.empty());
		REQUIRE(string_view(StrCr2).front() == L'S');
		REQUIRE(string_view(StrCr1).data() == string_view(StrCr2).data());
	}

	{
		string_copyref StrCr(L"Chorando se foi quem um dia só me fez chorar"s);
		REQUIRE(string_view(StrCr).front() == L'C');
	}

	REQUIRE(concat(L'a', L"bc", L"def"sv, L"1234"s) == L"abcdef1234"sv);
	REQUIRE(concat(L""sv, L""sv).empty());
	REQUIRE(concat(L""sv).empty());

	REQUIRE(erase_all(L"1,2,3,4,5"s, L',') == L"12345"sv);
	REQUIRE(erase_all(L"12345"s, L',') == L"12345"sv);
	REQUIRE(erase_all(L""s, L',').empty());

	REQUIRE(join(std::array{ L"123"sv, L"45"sv, L""sv, L"6"sv }, L","sv) == L"123,45,,6"sv);

	REQUIRE(L"123"s + L"45"sv == L"12345"sv);
	REQUIRE(L"123"sv + L"45"s == L"12345"sv);
	REQUIRE(L"123"sv + L"45"sv == L"12345"sv);

	const auto Str = L"12345"sv;
	REQUIRE(make_string_view(Str.begin() + 1, Str.end() - 1) == L"234"sv);
}

//----------------------------------------------------------------------------

#include "common/utility.hpp"

TEMPLATE_TEST_CASE("utility.reserve_exp_noshrink", "", string, std::vector<int>)
{
	TestType Container;
	Container.resize(42);
	const auto InitialCapacity = Container.capacity();
	const auto GrowthFactor = 1.5;

	SECTION("no shrink")
	{
		reserve_exp_noshrink(Container, 1);
		REQUIRE(Container.capacity() >= InitialCapacity);
	}

	SECTION("exponential < factor")
	{
		reserve_exp_noshrink(Container, InitialCapacity * 1.1);
		REQUIRE(Container.capacity() >= InitialCapacity * GrowthFactor);
	}

	SECTION("exponential > factor")
	{
		reserve_exp_noshrink(Container, InitialCapacity * 2);
		REQUIRE(Container.capacity() >= InitialCapacity * 2);
	}
}

TEMPLATE_TEST_CASE("utility.clear_and_shrink", "", string, std::vector<int>)
{
	TestType Container;
	Container.resize(42);
	clear_and_shrink(Container);
	REQUIRE(!Container.size());
	REQUIRE(Container.capacity() == TestType{}.capacity());
}

TEST_CASE("utility.node_swap")
{
	std::list List{ 1, 2, 3, 4, 5 };
	const auto Ptr1 = &List.front();
	const auto Ptr5 = &List.back();
	node_swap(List, List.begin(), std::prev(List.end()));
	REQUIRE(Ptr1 == &List.back());
	REQUIRE(Ptr5 == &List.front());
	REQUIRE(*Ptr1 == 1);
	REQUIRE(*Ptr5 == 5);
}

TEST_CASE("utility.copy_memory")
{
	char Buffer[] = "12345";
	copy_memory("ABCDE", Buffer, 3);
	REQUIRE(Buffer == "ABC45"sv);
	copy_memory(static_cast<char const*>(nullptr), Buffer, 0);
	REQUIRE(Buffer == "ABC45"sv);
}

TEST_CASE("utility.hash_range")
{
	const auto s1 = L"12345"sv;
	const auto s2 = L"12345"s;
	const auto s3 = L"abcde"s;

	const auto h1 = hash_range(ALL_CONST_RANGE(s1));
	const auto h2 = hash_range(ALL_CONST_RANGE(s2));
	const auto h3 = hash_range(ALL_CONST_RANGE(s3));

	REQUIRE(h1 == h2);
	REQUIRE(h2 != h3);
}

TEST_CASE("utility.aligned_size")
{
	for (size_t i = 0; i <= MEMORY_ALLOCATION_ALIGNMENT * 8; ++i)
	{
		const size_t Expected = std::ceil(static_cast<double>(i) / MEMORY_ALLOCATION_ALIGNMENT) * MEMORY_ALLOCATION_ALIGNMENT;
		REQUIRE(aligned_size(i) == Expected);
	}
}

//----------------------------------------------------------------------------

#include "common/view/enumerate.hpp"

TEST_CASE("view.enumerate")
{
	size_t Index = 0;
	for (const auto& [Item, i]: enumerate(std::vector{ 'A', 'B', 'C' }))
	{
		REQUIRE(Item == static_cast<char>('A' + Index));
		REQUIRE(i == Index);

		++Index;
	}

	REQUIRE(Index == 3u);
}

//----------------------------------------------------------------------------

#include "common/view/reverse.hpp"

TEST_CASE("view.reverse")
{
	const std::array Data = { 1, 2, 3, 4, 5 };
	const std::array Reversed = { 5, 4, 3, 2, 1 };

	auto Iterator = std::cbegin(Reversed);

	for (const auto& i: reverse(Data))
	{
		REQUIRE(Iterator != std::cend(Reversed));
		REQUIRE(i == *Iterator);
		++Iterator;
	}

	REQUIRE(Iterator == std::cend(Reversed));
}

//----------------------------------------------------------------------------

#include "common/view/select.hpp"

TEST_CASE("view.select")
{
	const auto Test = [](const auto& Data, const auto& Selector)
	{
		auto Iterator = std::cbegin(Data);

WARNING_PUSH()
WARNING_DISABLE_CLANG("-Wrange-loop-analysis")
		for (const auto& i: select(Data, Selector))
WARNING_POP()
		{
			REQUIRE(Iterator != std::cend(Data));
			REQUIRE(i == Selector(*Iterator));
			++Iterator;
		}

		REQUIRE(Iterator == std::cend(Data));
	};

	{
		std::pair<bool, int> const Data[]
		{
			{ true, 42 },
			{ false, 33 },
		};

		Test(Data, [](const auto& i) { return i.second; });

		std::vector DataCopy(std::cbegin(Data), std::cend(Data));
		for (auto& i: select(DataCopy, [](auto& i) -> auto& { return i.second; }))
		{
			i *= 2;
		}

		for (size_t i = 0; i != std::size(Data); ++i)
		{
			REQUIRE(DataCopy[i].second == Data[i].second * 2);
		}
	}

	{
		std::vector const Data{ 1, 2, 3, 4, 5 };
		Test(Data, [](const auto& i) { return i * 2; });
	}

	{
		std::vector<int> const Data;
		Test(Data, [](const auto& i) { return i; });
	}
}

//----------------------------------------------------------------------------

#include "common/view/where.hpp"

TEST_CASE("view.where")
{
	const auto Test = [](const auto& Data, const auto& Baseline, const auto& Predicate)
	{
		auto BaselineIterator = std::cbegin(Baseline);

		for (const auto& i: where(Data, Predicate))
		{
			REQUIRE(i == *BaselineIterator);
			++BaselineIterator;
		}

		REQUIRE(BaselineIterator == std::cend(Baseline));
	};

	using ints = std::vector<int>;
	const auto Even = [](const auto& Item) { return (Item & 1) == 0; };
	const auto Odd = [](const auto& Item) { return (Item & 1) != 0; };

	Test(
		ints{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
		ints{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
		[](const auto&) { return true; });

	Test(
		ints{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
		ints{},
		[](const auto&) { return false; });

	Test(
		ints{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
		ints{ 0, 2, 4, 6, 8 },
		Even);

	Test(
		ints{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
		ints{ 1, 3, 5, 7, 9 },
		Odd);

	Test(
		ints{ 2, 4, 6 },
		ints{},
		Odd);

	Test(
		ints{},
		ints{},
		Even);
}

//----------------------------------------------------------------------------

#include "common/view/zip.hpp"

TEST_CASE("view.zip")
{
	{
		const std::array Source      { 1, 2, 3 };
		      std::array Destination { 9, 8, 7, 6, 5 };
		const std::array Baseline    { 1, 2, 3, 6, 5 };

		for (const auto& [Src, Dst] : zip(Source, Destination))
		{
			Dst = Src;
		}

		REQUIRE(Destination == Baseline);
	}

	{
		int Index = 0;
		for (const auto& [i, l, b]: zip(std::vector{ 1, 2, 3 }, std::list{ 'A', 'B', 'C' }, std::vector{true, true, true}))
		{
			REQUIRE(i == 1 + Index);
			REQUIRE(l == 'A' + Index);
			REQUIRE(b);
			++Index;
		}

		REQUIRE(Index == 3);
	}
}
#endif
