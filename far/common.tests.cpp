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

// BUGBUG
#include "platform.headers.hpp"

// Internal:
#include "testing.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

#include "common/2d/matrix.hpp"

TEST_CASE("2d.matrix")
{
	static const size_t Data[]
	{
		0,  1,  2,  3,
		4,  5,  6,  7,
		8,  9, 10, 11,
	};

	const size_t Width = 4, Height = 3;

	const matrix_view Matrix(Data, Height, Width);

	matrix Copy(Matrix);
	Copy = Matrix;

	REQUIRE(Matrix.width() == Width);
	REQUIRE(Matrix.height() == Height);
	REQUIRE(Matrix.size() == Height * Width);

	size_t Counter = 0;
	size_t RowNumber = 0;

WARNING_PUSH()
WARNING_DISABLE_CLANG("-Wrange-loop-analysis")
	for (const auto& Row: Matrix)
WARNING_POP()
	{
		REQUIRE(Row.size() == Width);

		for (const auto& Cell: Row)
		{
			REQUIRE(Cell == Counter);
			++Counter;
		}

		++RowNumber;

		REQUIRE(Counter == RowNumber * Width);
	}

	REQUIRE(Counter == Width * Height);
}

//----------------------------------------------------------------------------

#include "common/algorithm.hpp"

TEST_CASE("algorithm.repeat")
{
	auto Value = 0;
	auto const Count = 7;
	auto Increment = [&]{ ++Value; };

	repeat(Count, Increment);

	REQUIRE(Value == Count);
}

TEST_CASE("algorithm.apply_permutation")
{
	{
		std::vector<int> Data, Indices;
		apply_permutation(ALL_RANGE(Data), Indices.begin());
		REQUIRE(Data.empty());
		REQUIRE(Indices.empty());
	}

	{
		std::array Data{ 'E', 'L', 'V', 'I', 'S' };
		std::array const Expected{ 'L', 'I', 'V', 'E', 'S' };
		std::array Indices{ 1, 3, 2, 0, 4 };
		STATIC_REQUIRE(
			std::size(Data) == std::size(Expected) &&
			std::size(Data) == std::size(Indices)
		);
		apply_permutation(ALL_RANGE(Data), Indices.begin());
		REQUIRE(Data == Expected);
	}
}

TEST_CASE("algorithm.emplace")
{
	{
		std::vector<int> Data;
		emplace(Data, 42);
		REQUIRE(Data.front() == 42);
	}

	{
		std::set<int> Data;
		emplace(Data, 42);
		REQUIRE(*Data.begin() == 42);
	}
}

TEST_CASE("algorithm.contains")
{
	{
		constexpr std::array Data{ 1, 2, 3 };

		// TODO: STATIC_REQUIRE
		// GCC stdlib isn't constexpr yet :(
		REQUIRE(contains(Data, 1));
		REQUIRE(contains(Data, 2));
		REQUIRE(contains(Data, 3));
		REQUIRE(!contains(Data, 4));
	}

	{
		std::set Data{ 1, 2, 3 };

		REQUIRE(contains(Data, 1));
		REQUIRE(contains(Data, 2));
		REQUIRE(contains(Data, 3));
		REQUIRE(!contains(Data, 4));
	}
}

TEST_CASE("algorithm.in_closed_range")
{
	STATIC_REQUIRE(in_closed_range(0, 0, 0));
	STATIC_REQUIRE(in_closed_range(0, 0, 1));
	STATIC_REQUIRE(in_closed_range(0, 1, 1));
	STATIC_REQUIRE(in_closed_range(1, 1, 1));
	STATIC_REQUIRE(in_closed_range(1, 3, 5));

	STATIC_REQUIRE(!in_closed_range(0, 1, 0));
	STATIC_REQUIRE(!in_closed_range(1, 0, 0));
	STATIC_REQUIRE(!in_closed_range(1, 1, 0));
	STATIC_REQUIRE(!in_closed_range(1, 0, 1));
	STATIC_REQUIRE(!in_closed_range(5, 3, 1));
}

TEST_CASE("algorithm.any_none_of")
{
	STATIC_REQUIRE(any_of(1, 1));
	STATIC_REQUIRE(any_of(1, 1, 2, 3));
	STATIC_REQUIRE(none_of(1, 0));
	STATIC_REQUIRE(none_of(1, 2, 3));
}

//----------------------------------------------------------------------------

#include "common/base64.hpp"

TEST_CASE("base64.legit")
{
	static const struct
	{
		bytes_view Src;
		std::string_view Encoded;
	}
	Tests[]
	{
		{ {},           {}           },
		{ "f"_bv,       "Zg=="sv     },
		{ "fo"_bv,      "Zm8="sv     },
		{ "foo"_bv,     "Zm9v"sv     },
		{ "foob"_bv,    "Zm9vYg=="sv },
		{ "fooba"_bv,   "Zm9vYmE="sv },
		{ "foobar"_bv,  "Zm9vYmFy"sv },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(base64::encode(i.Src) == i.Encoded);
		REQUIRE(base64::decode(i.Encoded) == i.Src);
	}
}

TEST_CASE("base64.incomplete")
{
	static const struct
	{
		std::string_view Src;
		bytes_view Decoded;
	}
	Tests[]
	{
		{ "Z"sv,        {},            },
		{ "Zg"sv,       "f"_bv,        },
		{ "Zg="sv,      "f"_bv,        },
		{ "Zg=="sv,     "f"_bv,        },

		{ "Zm"sv,       "f"_bv,        },
		{ "Zm8"sv,      "fo"_bv,       },
		{ "Zm8="sv,     "fo"_bv,       },

		{ "Zm9"sv,      "fo"_bv,       },
		{ "Zm9v"sv,     "foo"_bv,      },
		{ "Zm9vY"sv,    "foo"_bv,      },

		{ "Zm9vYg"sv,   "foob"_bv,     },
		{ "Zm9vYg="sv,  "foob"_bv,     },
		{ "Zm9vYg=="sv, "foob"_bv,     },

		{ "Zm9vYm"sv,   "foob"_bv,     },
		{ "Zm9vYmE"sv,  "fooba"_bv,    },
		{ "Zm9vYmE="sv, "fooba"_bv,    },

		{ "Zm9vYmF"sv,  "fooba"_bv,    },
		{ "Zm9vYmFy"sv, "foobar"_bv,   },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(base64::decode(i.Src) == i.Decoded);
	}
}

TEST_CASE("base64.rubbish")
{
	static const struct
	{
		std::string_view Src;
		bytes_view Decoded;
	}
	Tests[]
	{
		{ "!!!"sv,              {},            },
		{ "<Z:m!9;v>"sv,        "foo"_bv,      },
		{ "_Z!m:9,v Y;m>F<y"sv, "foobar"_bv,   },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(base64::decode(i.Src) == i.Decoded);
	}
}

TEST_CASE("base64.random.roundtrip")
{
	std::mt19937 mt(clock()); // std::random_device doesn't work in w2k
	std::uniform_int_distribution CharDist(0, UCHAR_MAX);

	char RandomInput[256];
	std::generate(ALL_RANGE(RandomInput), [&]{ return CharDist(mt); });

	const auto Encoded = base64::encode(view_bytes(RandomInput));
	REQUIRE(base64::decode(Encoded) == view_bytes(RandomInput));
}

//----------------------------------------------------------------------------

#include "common/bytes_view.hpp"

TEST_CASE("bytes")
{
	uint32_t Value = 0;
	const auto BytesRef = edit_bytes(Value);
	std::fill(BytesRef.begin(), BytesRef.end(), std::byte{0x42});
	REQUIRE(Value == 0x42424242);
	const auto View = view_bytes(Value);
	REQUIRE(View == "\x42\x42\x42\x42"_bv);
	bytes Copy(View);
	REQUIRE(Copy == View);

	const auto Str = "BANANA"sv;
	const auto Bytes = view_bytes(Str);
	REQUIRE(Bytes == "BANANA"_bv);
	const auto Str2 = to_string_view(Bytes);
	REQUIRE(Str2 == Str);
}

//----------------------------------------------------------------------------

#include "common/chrono.hpp"

TEST_CASE("chrono")
{
	const auto Duration = 47h + 63min + 71s + 3117ms;

	const auto check = [](const auto& Result, auto Arg)
	{
		REQUIRE(Result.template get<decltype(Arg)>() == Arg);
	};

	// The explicit capture is a workaround for VS2017.
	// TODO: remove once we drop support for VS2017.
	const auto check_split_duration = [&, check](auto... Args)
	{
		const auto Result = split_duration<decltype(Args)...>(Duration);
		(..., check(Result, Args));
	};

	check_split_duration(2_d);
	check_split_duration(48h);
	check_split_duration(2884min);
	check_split_duration(173054s);
	check_split_duration(173054117ms);
	check_split_duration(48h, 254117ms);
	check_split_duration(2884min, 14s);
	check_split_duration(2_d, 0h, 4min, 14s, 117ms);
}

//----------------------------------------------------------------------------

#include "common/enum_substrings.hpp"

TEST_CASE("enum_substrings")
{
	const std::array Baseline{ L"abc"sv, L"def"sv, L"q"sv };
	auto BaselineIterator = Baseline.begin();

	for (const auto& i: enum_substrings(L"abc\0def\0q\0"sv.data()))
	{
		REQUIRE(i == *BaselineIterator++);
	}

	REQUIRE(BaselineIterator == Baseline.end());

	BaselineIterator = Baseline.begin();

	auto DataNoLastNulls = L"abc\0def\0q_"sv;
	DataNoLastNulls.remove_suffix(1);
	for (const auto& i: enum_substrings(DataNoLastNulls))
	{
		REQUIRE(i == *BaselineIterator++);
	}

	REQUIRE(BaselineIterator == Baseline.end());
}

//----------------------------------------------------------------------------

#include "common/enum_tokens.hpp"

TEST_CASE("enum_tokens")
{
	{
		const std::array Baseline{ L"abc"sv, L""sv, L"def"sv, L" q "sv, L"123"sv };
		auto BaselineIterator = Baseline.begin();

		for (const auto& i: enum_tokens(L"abc;,def; q ,123;"sv, L",;"sv))
		{
			REQUIRE(i == *BaselineIterator++);
		}

		REQUIRE(BaselineIterator == Baseline.end());
	}

	{
		const std::array Baseline{ L"abc;"sv, L"de;,f"sv, L"123"sv, L""sv };
		auto BaselineIterator = Baseline.begin();

		for (const auto& i: enum_tokens_with_quotes(L"\"abc;\",\"de;,f\";123;;"sv, L",;"sv))
		{
			REQUIRE(i == *BaselineIterator++);
		}

		REQUIRE(BaselineIterator == Baseline.end());
	}

	{
		const std::array Baseline{ L"abc"sv, L"def"sv, L""sv };
		auto BaselineIterator = Baseline.begin();

		for (const auto& i: enum_tokens_custom_t<with_trim>(L"  abc|   def  |  "sv, L"|"sv))
		{
			REQUIRE(i == *BaselineIterator++);
		}

		REQUIRE(BaselineIterator == Baseline.end());
	}

}

//----------------------------------------------------------------------------

#include "common/enumerator.hpp"

TEST_CASE("enumerator")
{
	enum
	{
		e_begin = 5,
		e_end = 14
	};

	bool Finalised = false;

	{
		auto TestEnumerator = make_inline_enumerator<int>([Counter = static_cast<int>(e_begin)](bool const Reset, int& Value) mutable
		{
			if (Reset)
				Counter = e_begin;

			Value = Counter++;
			return Value != e_end;
		},
		[&]
		{
			Finalised = true;
		});

		for (size_t N = 1; N != 3; ++N)
		{
			int Start = e_begin;

			for (auto& i: TestEnumerator)
			{
				REQUIRE(i == Start++);
				i = 0;
			}

			REQUIRE(Start == e_end);
		}
	}

	REQUIRE(Finalised);
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
	REQUIRE(from_string<int32_t>(L"-2147483648"sv) == std::numeric_limits<int32_t>::min());
	REQUIRE(from_string<int32_t>(L"2147483647"sv) == std::numeric_limits<int32_t>::max());
	REQUIRE(from_string<uint32_t>(L"4294967295"sv) == std::numeric_limits<uint32_t>::max());
	REQUIRE(from_string<int64_t>(L"-9223372036854775808"sv) == std::numeric_limits<int64_t>::min());
	REQUIRE(from_string<int64_t>(L"9223372036854775807"sv) == std::numeric_limits<int64_t>::max());
	REQUIRE(from_string<uint64_t>(L"18446744073709551615"sv) == std::numeric_limits<uint64_t>::max());

	REQUIRE_THROWS_AS(from_string<uint64_t>(L"18446744073709551616"sv), std::out_of_range);
	REQUIRE_THROWS_AS(from_string<int64_t>(L"-9223372036854775809"sv), std::out_of_range);
	REQUIRE_THROWS_AS(from_string<int64_t>(L"9223372036854775808"sv), std::out_of_range);
	REQUIRE_THROWS_AS(from_string<uint32_t>(L"4294967296"sv), std::out_of_range);
	REQUIRE_THROWS_AS(from_string<int32_t>(L"-2147483649"sv), std::out_of_range);
	REQUIRE_THROWS_AS(from_string<int32_t>(L"2147483648"sv), std::out_of_range);
	REQUIRE_THROWS_AS(from_string<uint16_t>(L"65536"sv), std::out_of_range);
	REQUIRE_THROWS_AS(from_string<int16_t>(L"-32769"sv), std::out_of_range);
	REQUIRE_THROWS_AS(from_string<int16_t>(L"32768"sv), std::out_of_range);
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

#include "common/function_traits.hpp"

TEST_CASE("function_traits")
{
	{
		using t = function_traits<void()>;
		STATIC_REQUIRE(t::arity == 0);
		STATIC_REQUIRE(std::is_same_v<t::result_type, void>);
	}

	{
		using t = function_traits<char(short, int, long)>;
		STATIC_REQUIRE(t::arity == 3);
		STATIC_REQUIRE(std::is_same_v<t::arg<0>, short>);
		STATIC_REQUIRE(std::is_same_v<t::arg<1>, int>);
		STATIC_REQUIRE(std::is_same_v<t::arg<2>, long>);
		STATIC_REQUIRE(std::is_same_v<t::result_type, char>);
	}

	{
		struct s { double f(bool) const { return 0; } };
		using t = function_traits<decltype(&s::f)>;
		STATIC_REQUIRE(t::arity == 1);
		STATIC_REQUIRE(std::is_same_v<t::arg<0>, bool>);
		STATIC_REQUIRE(std::is_same_v<t::result_type, double>);
	}
}

//----------------------------------------------------------------------------

#include "common/io.hpp"

TEST_CASE("io")
{
	std::stringstream Stream;
	constexpr auto Str = "12345"sv;
	REQUIRE_NOTHROW(io::write(Stream, Str));

	std::byte Buffer[Str.size()];
	REQUIRE(io::read(Stream, Buffer) == Str.size());
	REQUIRE(!io::read(Stream, Buffer));
}

//----------------------------------------------------------------------------

#include "common/keep_alive.hpp"

template<typename type>
static void TestKeepAlive()
{
	STATIC_REQUIRE(std::is_same_v<decltype(keep_alive(std::declval<type>())), keep_alive<type>>);
}

TEST_CASE("keep_alive")
{
	TestKeepAlive<int>();
	TestKeepAlive<int&>();
	TestKeepAlive<const int&>();
}

//----------------------------------------------------------------------------

#include "common/lazy.hpp"

TEST_CASE("lazy")
{
	int Magic1 = 42, Magic2 = 69;
	int CallCount;
	lazy<int> const LazyValue([&]{ ++CallCount; return Magic1; });

	{
		CallCount = 0;
		auto Value = LazyValue;

		REQUIRE(CallCount == 0);
		REQUIRE(*Value == Magic1);
		REQUIRE(*Value == Magic1);
		REQUIRE(CallCount == 1);

		Value = Magic2;
		REQUIRE(*Value == Magic2);
		REQUIRE(CallCount == 1);
	}

	{
		CallCount = 0;
		auto Value = LazyValue;

		Value = Magic2;
		REQUIRE(*Value == Magic2);
		REQUIRE(CallCount == 0);
	}

}

//----------------------------------------------------------------------------

#include "common/monitored.hpp"

TEST_CASE("monitored")
{
	monitored a(42);
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
	movable m1 = Value;
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
	REQUIRE(Count == std::wcslen(Ptr));
}

//----------------------------------------------------------------------------

#include "common/placement.hpp"

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

	alignas(raii) std::byte Data[sizeof(raii)];
	auto& Object = edit_as<raii>(&Data);

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
					// Workaround for VS19
					[[maybe_unused]] auto& RangeRef = Range;

					STATIC_REQUIRE(std::is_same_v<decltype(ContainerGetter(ContainerVersion)), decltype(RangeGetter(Range))>);
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
		STATIC_REQUIRE(std::is_same_v<decltype(*Range.begin()), int&>);
		STATIC_REQUIRE(std::is_same_v<decltype(*Range.cbegin()), const int&>);
	}

	{
		std::vector<int> Data;
		range Range(std::begin(Data), std::end(Data));
		STATIC_REQUIRE(std::is_same_v<decltype(*Range.begin()), int&>);
		// It's not possible to deduce const_iterator here
		STATIC_REQUIRE(std::is_same_v<decltype(*Range.cbegin()), int&>);
	}

	{
		int Data[2]{};
		span Span(Data);
		STATIC_REQUIRE(std::is_same_v<decltype(*Span.begin()), int&>);
		STATIC_REQUIRE(std::is_same_v<decltype(*Span.cbegin()), const int&>);
	}
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

	{
		size_t const Total = 10;
		size_t Count = 0;

		for (const auto& i: irange(Total))
		{
			REQUIRE(i == Count);
			++Count;
		}

		REQUIRE(Count == Total);
	}
}

//----------------------------------------------------------------------------

#include "common/scope_exit.hpp"

namespace
{
	template<scope_exit::scope_type type>
	void test_scope_impl(bool const Throw, bool const MustBeTriggered)
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

	enum
	{
		on_fail    = bit(0),
		on_success = bit(1),
	};

	template<scope_exit::scope_type type>
	void test_scope(int const When)
	{
		test_scope_impl<type>(true, (When & on_fail) != 0);
		test_scope_impl<type>(false, (When & on_success) != 0);
	}
}
TEST_CASE("scope_exit")
{
	test_scope<scope_exit::scope_type::exit>(on_fail | on_success);
	test_scope<scope_exit::scope_type::fail>(on_fail);
	test_scope<scope_exit::scope_type::success>(on_success);
}

//----------------------------------------------------------------------------

#include "common/smart_ptr.hpp"

TEST_CASE("smart_ptr")
{
	const char_ptr_n<1> Ptr;
	constexpr auto ActualStaticSize = sizeof(std::unique_ptr<char[]>) / sizeof(char);
	const char_ptr_n<ActualStaticSize> Ptr2;

	STATIC_REQUIRE(sizeof(Ptr) == sizeof(Ptr2));
	REQUIRE(Ptr.size() == Ptr2.size());
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
		{ {},         0, {},         {},         },

		{ {},         1, {},         {},         },

		{ L"1"sv,     0, {},         {},         },
		{ L"1"sv,     1, L"1"sv,     L"1"sv,     },
		{ L"1"sv,     2, L"1"sv,     L"1"sv,     },

		{ L"12345"sv, 0, {},         {},         },
		{ L"12345"sv, 1, L"5"sv,     L"1"sv,     },
		{ L"12345"sv, 3, L"345"sv,   L"123"sv,   },
		{ L"12345"sv, 6, L"12345"sv, L"12345"sv, },
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
		{ {},       0, {},         {},         },
		{ {},       1, L" "sv,     L" "sv,     },
		{ {},       2, L"  "sv,    L"  "sv,    },

		{ L"1"sv,   0, L"1"sv,     L"1"sv,     },
		{ L"1"sv,   1, L"1"sv,     L"1"sv,     },
		{ L"1"sv,   3, L"  1"sv,   L"1  "sv,   },

		{ L"123"sv, 0, L"123"sv,   L"123"sv,   },
		{ L"123"sv, 2, L"123"sv,   L"123"sv,   },
		{ L"123"sv, 5, L"  123"sv, L"123  "sv, },
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
		{ {},             {},           {},           {},         },
		{ L" "sv,         {},           {},           {},         },
		{ L"12345"sv,     L"12345"sv,   L"12345"sv,   L"12345"sv, },
		{ L"  12345"sv,   L"12345"sv,   L"  12345"sv, L"12345"sv, },
		{ L"12345  "sv,   L"12345  "sv, L"12345"sv,   L"12345"sv, },
		{ L"  12345  "sv, L"12345  "sv, L"  12345"sv, L"12345"sv, },
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
		{ {},          0,   {},             {},             {},           },
		{ {},          1,   L" "sv,         L" "sv,         L" "sv,       },
		{ {},          2,   L"  "sv,        L"  "sv,        L"  "sv,      },

		{ L"1"sv,      0,   {},             {},             {},           },
		{ L"1"sv,      1,   L"1"sv,         L"1"sv,         L"1"sv,       },
		{ L"1"sv,      2,   L"1 "sv,        L"1 "sv,        L" 1"sv,      },

		{ L"12345"sv,  0,   {},             {},             {},           },
		{ L"12345"sv,  1,   L"1"sv,         L"1"sv,         L"1"sv,       },
		{ L"12345"sv,  3,   L"123"sv,       L"123"sv,       L"123"sv,     },
		{ L"12345"sv,  5,   L"12345"sv,     L"12345"sv,     L"12345"sv,   },
		{ L"12345"sv,  7,   L"12345  "sv,   L" 12345 "sv,   L"  12345"sv, },
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
		{ {},              {},          true,   true,   true,   },
		{ {},              L"1"sv,      false,  false,  false,  },

		{ L"12345"sv,      {},          true,   true,   true,   },
		{ L"12345"sv,      L"123"sv,    true,   false,  true,   },
		{ L"12345"sv,      L"234"sv,    false,  false,  true,   },
		{ L"12345"sv,      L"345"sv,    false,  true,   true,   },
		{ L"12345"sv,      L"12345"sv,  true,   true,   true,   },
		{ L"12345"sv,      L"123456"sv, false,  false,  false,  },
		{ L"12345"sv,      L"24"sv,     false,  false,  false,  },
		{ L"12345"sv,      L"foo"sv,    false,  false,  false,  },

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
	static const struct
	{
		string_view Src, ResultUnquote, ResultQuote, ResultQuoteUnconditional, ResultQuoteNormalise, ResultQuoteSpace;
	}
	Tests[]
	{
		{ {},                  {},            LR"("")"sv,         LR"("")"sv,           LR"("")"sv,        {},                 },
		{ L" "sv,              L" "sv,        LR"(" ")"sv,        LR"(" ")"sv,          LR"(" ")"sv,       LR"(" ")"sv,        },
		{ LR"(")"sv,           {},            LR"("")"sv,         LR"(""")"sv,          LR"("")"sv,        LR"(")"sv,          },
		{ LR"(" )"sv,          L" "sv,        LR"(" ")"sv,        LR"("" ")"sv,         LR"(" ")"sv,       LR"(" ")"sv,        },
		{ LR"( ")"sv,          L" "sv,        LR"(" ")"sv,        LR"(" "")"sv,         LR"(" ")"sv,       LR"(" ")"sv,        },
		{ LR"("")"sv,          {},            LR"("")"sv,         LR"("""")"sv,         LR"("")"sv,        LR"("")"sv,         },
		{ LR"(" ")"sv,         L" "sv,        LR"(" ")"sv,        LR"("" "")"sv,        LR"(" ")"sv,       LR"(" ")"sv,        },
		{ L"12345"sv,          L"12345"sv,    LR"("12345")"sv,    LR"("12345")"sv,      LR"("12345")"sv,   L"12345"sv,         },
		{ L"12 345"sv,         L"12 345"sv,   LR"("12 345")"sv,   LR"("12 345")"sv,     LR"("12 345")"sv,  LR"("12 345")"sv,   },
		{ LR"("12345")"sv,     L"12345"sv,    LR"("12345")"sv,    LR"(""12345"")"sv,    LR"("12345")"sv,   LR"("12345")"sv,    },
		{ LR"("12"345")"sv,    L"12345"sv,    LR"("12"345")"sv,   LR"(""12"345"")"sv,   LR"("12345")"sv,   LR"("12"345")"sv,   },
		{ LR"("12" 345")"sv,   L"12 345"sv,   LR"("12" 345")"sv,  LR"(""12" 345"")"sv,  LR"("12 345")"sv,  LR"("12" 345")"sv,  },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(unquote(i.Src) == i.ResultUnquote);
		REQUIRE(quote(i.Src) == i.ResultQuote);
		REQUIRE(quote_unconditional(i.Src) == i.ResultQuoteUnconditional);
		REQUIRE(quote_normalise(i.Src) == i.ResultQuoteNormalise);
		REQUIRE(quote_space(i.Src) == i.ResultQuoteSpace);
	}
}

TEST_CASE("string_utils.split")
{
	static const struct
	{
		string_view Src;
		std::pair<string_view, string_view> Result;
	}
	Tests[]
	{
		{ {},              { {},         {},         }, },
		{ L"="sv,          { {},         {},         }, },
		{ L"=foo"sv,       { {},         L"foo"sv,   }, },
		{ L"==foo"sv,      { {},         L"=foo"sv,  }, },
		{ L"foo="sv,       { L"foo"sv,   {},         }, },
		{ L"foo=="sv,      { L"foo"sv,   L"="sv,     }, },
		{ L"foo"sv,        { L"foo"sv,   {},         }, },
		{ L"foo=bar"sv,    { L"foo"sv,   L"bar"sv,   }, },
		{ L"foo=bar="sv,   { L"foo"sv,   L"bar="sv,  }, },
		{ L"foo==bar="sv,  { L"foo"sv,   L"=bar="sv, }, },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(split(i.Src) == i.Result);
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

	REQUIRE(join(std::array{ L"123"sv, L"45"sv, L""sv, L"6"sv }, L","sv) == L"123,45,,6"sv);

	REQUIRE(L"123"s + L"45"sv == L"12345"sv);
	REQUIRE(L"123"sv + L"45"s == L"12345"sv);
	REQUIRE(L"123"sv + L"45"sv == L"12345"sv);

	const auto Str = L"12345"sv;
	REQUIRE(make_string_view(Str.begin() + 1, Str.end() - 1) == L"234"sv);
}

#ifdef __cpp_lib_generic_unordered_lookup
TEST_CASE("string_utils.generic_lookup")
{
	const unordered_string_map<int> Map
	{
		{ L"123"s, 123 },
	};

	REQUIRE(Map.find(L"123"sv) != Map.cend());
	REQUIRE(Map.find(L"123") != Map.cend());
}
#endif

//----------------------------------------------------------------------------

#include "common/utility.hpp"

TEST_CASE("utility.grow_exp_noshrink")
{
	static const struct
	{
		size_t Current, Desired, Expected;
	}
	Tests[]
	{
		{ 0, 1, 1 },
		{ 1, 1, 1 },
		{ 1, 0, 1 },
		{ 0, 2, 2 },
		{ 1, 2, 2 },
		{ 2, 2, 2 },
	};

	for (const auto& i : Tests)
	{
		REQUIRE(grow_exp_noshrink(i.Current, i.Desired) == i.Expected);
	}
}

TEMPLATE_TEST_CASE("utility.reserve_exp_noshrink", "", string, std::vector<int>)
{
	TestType Container;
	Container.resize(42);
	const auto InitialCapacity = Container.capacity();

	SECTION("no shrink")
	{
		reserve_exp_noshrink(Container, 1);
		REQUIRE(Container.capacity() >= InitialCapacity);
	}

	SECTION("exponential < factor")
	{
		reserve_exp_noshrink(Container, InitialCapacity + InitialCapacity / 10);
		REQUIRE(Container.capacity() >= InitialCapacity  + InitialCapacity / 2);
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

TEST_CASE("utility.flags")
{
	using flags_type = uint8_t;

	{
		static const struct
		{
			using test = std::pair<flags_type, bool>;

			flags_type Value;
			test Any, All;
		}
		Tests[]
		{
			{ 0b00000000, { 0b00000000, false }, { 0b00000000, true  } },
			{ 0b00000000, { 0b10000001, false }, { 0b10000001, false } },
			{ 0b00000001, { 0b00000001, true  }, { 0b00000001, true  } },
			{ 0b00000001, { 0b10000001, true  }, { 0b10000001, false } },
			{ 0b10000000, { 0b10000001, true  }, { 0b10000001, false } },
			{ 0b10000001, { 0b10000001, true  }, { 0b10000001, true  } },
		};

		for (const auto& i : Tests)
		{
			REQUIRE(flags::check_any(i.Value, i.Any.first) == i.Any.second);
			REQUIRE(flags::check_all(i.Value, i.All.first) == i.All.second);
		}
	}

	{
		static const struct
		{
			flags_type Value;
			struct
			{
				flags_type Value, Expected;
			}
			Set, Clear, Invert, Copy;
		}
		Tests[]
		{
			//Value         Set         Expected        Clear       Expected        Invert      Expected        Copy Mask   Expected
			{ 0b00000000, { 0b00000000, 0b00000000 }, { 0b00000000, 0b00000000 }, { 0b00000000, 0b00000000 }, { 0b00000000, 0b00000000 }},
			{ 0b00000000, { 0b00001111, 0b00001111 }, { 0b00001111, 0b00000000 }, { 0b00001111, 0b00001111 }, { 0b00001010, 0b00001010 }},
			{ 0b11111111, { 0b00000000, 0b11111111 }, { 0b00000000, 0b11111111 }, { 0b00000000, 0b11111111 }, { 0b00001111, 0b11110000 }},
			{ 0b11111111, { 0b00001111, 0b11111111 }, { 0b00001111, 0b11110000 }, { 0b00001111, 0b11110000 }, { 0b01010000, 0b10101111 }},
			{ 0b10101010, { 0b10101010, 0b10101010 }, { 0b10101010, 0b00000000 }, { 0b10101010, 0b00000000 }, { 0b11110000, 0b10101010 }},
			{ 0b10101010, { 0b01010101, 0b11111111 }, { 0b01010101, 0b10101010 }, { 0b01010101, 0b11111111 }, { 0b11001100, 0b01100110 }},
			{ 0b11001100, { 0b00110011, 0b11111111 }, { 0b10101010, 0b01000100 }, { 0b11001100, 0b00000000 }, { 0b01100110, 0b10101010 }},
		};

		for (const auto& i: Tests)
		{
			{
				auto Value = i.Value;
				flags::set(Value, i.Set.Value);
				REQUIRE(Value == i.Set.Expected);
			}
			{
				auto Value = i.Value;
				flags::clear(Value, i.Clear.Value);
				REQUIRE(Value == i.Clear.Expected);
			}
			{
				auto Value = i.Value;
				flags::invert(Value, i.Invert.Value);
				REQUIRE(Value == i.Invert.Expected);
			}
			{
				auto Value = i.Value;
				flags::copy(Value, i.Copy.Value, i.Set.Value);
				REQUIRE(Value == i.Copy.Expected);
			}
		}
	}

}

TEST_CASE("utility.aligned_size")
{
	constexpr auto Alignment = alignof(std::max_align_t);

	for (size_t i = 0; i <= Alignment * 8; ++i)
	{
		const size_t Expected = std::ceil(static_cast<double>(i) / Alignment) * Alignment;
		REQUIRE(aligned_size(i) == Expected);
	}
}

namespace utility_integers_detail
{
	template<const auto& LargeValues, size_t Index, const auto& SmallValues>
	static void check_make_integer()
	{
		using L = std::remove_reference_t<decltype(LargeValues[0])>;
		constexpr auto Offset = sizeof(LargeValues[0]) / sizeof(SmallValues[0]) * Index;
		STATIC_REQUIRE(make_integer<L>(SmallValues[Offset + 0], SmallValues[Offset + 1]) == LargeValues[Index]);
	}

	template<const auto& LargeValues, const auto& SmallValues, size_t... LargeI>
	static void check_make_integers_impl(std::index_sequence<LargeI...>)
	{
		(check_make_integer<LargeValues, LargeI, SmallValues>(), ...);
	}

	template<const auto& LargeValues, const auto& SmallValues>
	static void check_make_integers()
	{
		check_make_integers_impl<LargeValues, SmallValues>(std::make_index_sequence<std::size(LargeValues)>{});
	}


	template<const auto& LargeValues, size_t Index, const auto& SmallValues, size_t... SmallI>
	static void check_extract_integer(std::index_sequence<SmallI...>)
	{
		using S = std::remove_reference_t<decltype(SmallValues[0])>;
		STATIC_REQUIRE(((extract_integer<S, SmallI>(LargeValues[Index]) == SmallValues[sizeof...(SmallI) * Index + SmallI]) && ...));
	}

	template<const auto& LargeValues, const auto& SmallValues, size_t... LargeI>
	static void check_extract_integers_impl(std::index_sequence<LargeI...>)
	{
		(check_extract_integer<LargeValues, LargeI, SmallValues>(std::make_index_sequence<sizeof(LargeValues[0]) / sizeof(SmallValues[0])>{}), ...);
	}

	template<const auto& LargeValues, const auto& SmallValues>
	static void check_extract_integers()
	{
		check_extract_integers_impl<LargeValues, SmallValues>(std::make_index_sequence<std::size(LargeValues)>{});
	}

	static constexpr uint64_t u64[]
	{
		0xFEDCBA9876543210,
	};

	static constexpr uint32_t u32[]
	{
		0x76543210,
		0xFEDCBA98,
	};

	static constexpr uint16_t u16[]
	{
		0x3210,
		0x7654,
		0xBA98,
		0xFEDC,
	};

	static constexpr uint8_t u8[]
	{
		0x10,
		0x32,
		0x54,
		0x76,
		0x98,
		0xBA,
		0xDC,
		0xFE,
	};
}

TEST_CASE("utility.integers")
{
	using namespace utility_integers_detail;

	check_make_integers<u64, u32>();
	check_make_integers<u32, u16>();
	check_make_integers<u16, u8>();

	check_extract_integers<u64, u32>();
	check_extract_integers<u64, u16>();
	check_extract_integers<u64, u8>();

	check_extract_integers<u32, u16>();
	check_extract_integers<u32, u8>();

	check_extract_integers<u16, u8>();
}

//----------------------------------------------------------------------------

#include "common/uuid.hpp"

TEST_CASE("uuid")
{
#define TEST_UUID "01234567-89AB-CDEF-0123-456789ABCDEF"

	constexpr auto Uuid = TEST_UUID ""_uuid;
	constexpr auto UuidWithBrackets = "{" TEST_UUID "}"_uuid;
	constexpr auto UuidStr = WIDE_SV(TEST_UUID);

	REQUIRE(Uuid == UuidWithBrackets);

	STATIC_REQUIRE(Uuid.Data1 == 0x01234567);
	STATIC_REQUIRE(Uuid.Data2 == 0x89AB);
	STATIC_REQUIRE(Uuid.Data3 == 0xCDEF);
	STATIC_REQUIRE(Uuid.Data4[0] == 0x01);
	STATIC_REQUIRE(Uuid.Data4[1] == 0x23);
	STATIC_REQUIRE(Uuid.Data4[2] == 0x45);
	STATIC_REQUIRE(Uuid.Data4[3] == 0x67);
	STATIC_REQUIRE(Uuid.Data4[4] == 0x89);
	STATIC_REQUIRE(Uuid.Data4[5] == 0xAB);
	STATIC_REQUIRE(Uuid.Data4[6] == 0xCD);
	STATIC_REQUIRE(Uuid.Data4[7] == 0xEF);

	REQUIRE(uuid::str(Uuid) == UuidStr);

#undef TEST_UUID

	static const struct
	{
		bool Valid;
		std::string_view Str;
	}
	Tests[]
	{
		{ false, {} },
		{ false, "Bamboléo"sv },
		{ false, "01234567-89AB-CDEF+0123-456789ABCDEF"sv, },
		{ false, "01234567-89AB-CDEF-0123-456789ABCDEFGHI"sv, },
		{ false, "Z1234567-89AB-CDEF-0123-456789ABCDEF"sv, },
		{ true,  "01234567-89AB-CDEF-0123-456789ABCDEF"sv, },
		{ false, "{01234567-89AB-CDEF-0123-456789ABCDEF"sv, },
		{ false, "01234567-89AB-CDEF-0123-456789ABCDEF}"sv, },
		{ true,  "{01234567-89AB-CDEF-0123-456789ABCDEF}"sv, },
	};

	for (const auto& i: Tests)
	{
		if (!i.Valid)
			REQUIRE_THROWS(uuid::parse(i.Str));
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
	const std::array Data    { 1, 2, 3, 4, 5 };
	const std::array Reversed{ 5, 4, 3, 2, 1 };

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
			{ true,  42 },
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
