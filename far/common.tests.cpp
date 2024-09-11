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

#include "common.hpp"

TEST_CASE("common.CheckStructSize")
{
	struct s
	{
		size_t StructSize;
	}
	const S1{ 1 }, S2{ sizeof(s) }, S3{ sizeof(s) + 1 };

	REQUIRE(!CheckStructSize(static_cast<s const*>(nullptr)));
	REQUIRE(!CheckStructSize(&S1));
	REQUIRE(CheckStructSize(&S2));
	REQUIRE(CheckStructSize(&S3));

	REQUIRE(CheckNullOrStructSize(static_cast<s const*>(nullptr)));
	REQUIRE(!CheckNullOrStructSize(&S1));
	REQUIRE(CheckNullOrStructSize(&S2));
	REQUIRE(CheckNullOrStructSize(&S3));
}

TEST_CASE("common.NullToEmpty")
{
	const char* Null{}, *Empty = "", *NonEmpty = "banana";

	REQUIRE(!*NullToEmpty(Null));
	REQUIRE(NullToEmpty(Empty) == Empty);
	REQUIRE(NullToEmpty(NonEmpty) == NonEmpty);

	REQUIRE(!EmptyToNull(Null));
	REQUIRE(!EmptyToNull(Empty));
	REQUIRE(EmptyToNull(NonEmpty) == NonEmpty);
}

//----------------------------------------------------------------------------

#if COMPILER(GCC)
#include "common/cpp.hpp"

TEST_CASE("cpp.const_return")
{
	STATIC_REQUIRE(std::same_as<decltype(     wcschr(L"", '0')), wchar_t const*>);
	STATIC_REQUIRE(std::same_as<decltype(std::wcschr(L"", '0')), wchar_t const*>);

	STATIC_REQUIRE(std::same_as<decltype(     wcspbrk(L"", L"")), wchar_t const*>);
	STATIC_REQUIRE(std::same_as<decltype(std::wcspbrk(L"", L"")), wchar_t const*>);

	STATIC_REQUIRE(std::same_as<decltype(     wcsrchr(L"", '0')), wchar_t const*>);
	STATIC_REQUIRE(std::same_as<decltype(std::wcsrchr(L"", '0')), wchar_t const*>);

	STATIC_REQUIRE(std::same_as<decltype(     wcsstr(L"", L"")), wchar_t const*>);
	STATIC_REQUIRE(std::same_as<decltype(std::wcsstr(L"", L"")), wchar_t const*>);

	STATIC_REQUIRE(std::same_as<decltype(     wmemchr(L"", L'0', 0)), wchar_t const*>);
	STATIC_REQUIRE(std::same_as<decltype(std::wmemchr(L"", L'0', 0)), wchar_t const*>);
}
#endif

//----------------------------------------------------------------------------

#include "common/2d/matrix.hpp"

namespace detail
{
	template<typename T>
	constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;
};

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

	matrix_view<const size_t> const ConstView(Copy);

	REQUIRE(Matrix.width() == Width);
	REQUIRE(Matrix.height() == Height);
	REQUIRE(Matrix.size() == Height * Width);

	REQUIRE(std::views::reverse(Matrix).front().front() == Data[Width * (Height - 1)]);

	STATIC_REQUIRE(detail::is_const<decltype(*Matrix.data())>);
	STATIC_REQUIRE(detail::is_const<decltype(*Matrix[0].data())>);
	STATIC_REQUIRE(detail::is_const<decltype(*Matrix.front().data())>);
	STATIC_REQUIRE(detail::is_const<decltype(Matrix.at(0, 0))>);

	STATIC_REQUIRE(!detail::is_const<decltype(*Copy.data())>);
	STATIC_REQUIRE(!detail::is_const<decltype(*Copy[0].data())>);
	STATIC_REQUIRE(!detail::is_const<decltype(*Copy.front().data())>);
	STATIC_REQUIRE(!detail::is_const<decltype(Copy.at(0, 0))>);

	STATIC_REQUIRE(!detail::is_const<decltype(*std::as_const(Copy).data())>);
	STATIC_REQUIRE(!detail::is_const<decltype(*std::as_const(Copy)[0].data())>);
	STATIC_REQUIRE(!detail::is_const<decltype(*std::as_const(Copy).front().data())>);
	STATIC_REQUIRE(!detail::is_const<decltype(std::as_const(Copy).at(0, 0))>);

	STATIC_REQUIRE(detail::is_const<decltype(*ConstView.data())>);
	STATIC_REQUIRE(detail::is_const<decltype(*ConstView[0].data())>);
	STATIC_REQUIRE(detail::is_const<decltype(*ConstView.front().data())>);
	STATIC_REQUIRE(detail::is_const<decltype(ConstView.at(0, 0))>);

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

#include "common/2d/algorithm.hpp"

TEST_CASE("2d.algorithm")
{
	int Data[]
	{
		 0,  1,  2,  3,  4,
		 5,  6,  7,  8,  9,
		10, 11, 12, 13, 15,
		16, 17, 18, 19, 20,
		21, 22, 23, 24, 25,
	};

	matrix_view const Matrix(Data, 5, 5);

	const auto StartValue = 42;
	int Value = StartValue;

	for_submatrix(Matrix, {1, 1, 3, 3}, [&](int& i){ i = Value++; });

	const int Expected[]
	{
		 0,  1,  2,  3,  4,
		 5, 42, 43, 44,  9,
		10, 45, 46, 47, 15,
		16, 48, 49, 50, 20,
		21, 22, 23, 24, 25,
	};

	REQUIRE(std::ranges::equal(Data, Expected));
	REQUIRE(Value == StartValue + 3 * 3);
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
		apply_permutation(Data, Indices.begin());
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
		apply_permutation(Data, Indices.begin());
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

		STATIC_REQUIRE(contains(Data, 1));
		STATIC_REQUIRE(contains(Data, 2));
		STATIC_REQUIRE(contains(Data, 3));
		STATIC_REQUIRE(!contains(Data, 4));
	}

	{
		std::set const Data{ 1, 2, 3 };

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
		{ "="sv,        {},            },
		{ "=="sv,       {},            },
		{ "==="sv,      {},            },
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
	}
	Tests[]
	{
		{ "?"sv,         },
		{ "!!!"sv,       },
		{ "<Z:m!9;v>"sv, },
		{ "だもの"sv,    },
	};

	for (const auto& i: Tests)
	{
		REQUIRE_THROWS_AS(base64::decode(i.Src), std::runtime_error);
	}
}

TEST_CASE("base64.random.roundtrip")
{
	std::mt19937 mt(clock()); // std::random_device doesn't work in w2k
	std::uniform_int_distribution CharDist(0, UCHAR_MAX);

	char RandomInput[256];
	std::ranges::generate(RandomInput, [&]{ return CharDist(mt); });

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
	const auto check_split_duration = [](const auto Duration, auto... Args)
	{
		auto Result = split_duration<decltype(Args)...>(Duration);
		return (... && (Result.template get<decltype(Args)>() == Args));
	};

	constexpr auto Duration = 47h + 63min + 71s + 3117ms;

	STATIC_REQUIRE(check_split_duration(Duration, 2_d));
	STATIC_REQUIRE(check_split_duration(Duration, 48h));
	STATIC_REQUIRE(check_split_duration(Duration, 2884min));
	STATIC_REQUIRE(check_split_duration(Duration, 173054s));
	STATIC_REQUIRE(check_split_duration(Duration, 173054117ms));
	STATIC_REQUIRE(check_split_duration(Duration, 48h, 254117ms));
	STATIC_REQUIRE(check_split_duration(Duration, 2884min, 14s));
	STATIC_REQUIRE(check_split_duration(Duration, 2_d, 0h, 4min, 14s, 117ms));
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

template<typename T>
void test_enum_tokens(const auto& Expected, string_view const Input, string_view const Separators)
{
	auto Iterator = Expected.begin();

	for (const auto& t: T(Input, Separators))
	{
		REQUIRE(t == *Iterator++);
	}

	REQUIRE(Iterator == Expected.end());
}

TEST_CASE("enum_tokens")
{
	enum class test_type
	{
		simple,
		quotes,
		trim
	};

	static const struct
	{
		test_type TestType;
		std::initializer_list<string_view> Expected;
		string_view Input, Separators;
	}
	Tests[]
	{
		{ test_type::simple, { {} }, {}, L","sv },
		{ test_type::simple, { {}, {} }, L",", L","sv },
		{ test_type::simple, { L"abc"sv, {}, L"def"sv, L" q "sv, L"123"sv, {} }, L"abc;,def; q ,123;"sv, L",;"sv },
		{ test_type::quotes, { L"abc;"sv, L"de;,f"sv, L"123"sv, {}, {} }, L"\"abc;\",\"de;,f\";123;;"sv, L",;"sv },
		{ test_type::trim,   { L"abc"sv, L"def"sv, {}, {} }, L"  abc|   def  |  |"sv, L"|"sv },
	};

	for (const auto& i: Tests)
	{
		switch(i.TestType)
		{
		case test_type::simple: test_enum_tokens<enum_tokens>(i.Expected, i.Input, i.Separators); break;
		case test_type::quotes: test_enum_tokens<enum_tokens_with_quotes>(i.Expected, i.Input, i.Separators); break;
		case test_type::trim:   test_enum_tokens<enum_tokens_custom_t<with_trim>>(i.Expected, i.Input, i.Separators); break;
		default:
			std::unreachable();
		}
	}
}

//----------------------------------------------------------------------------

#include "common/enumerator.hpp"

TEST_CASE("enumerator.static")
{
	using test_range = decltype(inline_enumerator<int>([](bool, int&){ return false; }));

	STATIC_REQUIRE(std::ranges::range<test_range>);
	STATIC_REQUIRE(std::ranges::input_range<test_range>);

	STATIC_REQUIRE(requires(test_range Range) { std::ranges::begin(Range); });
	STATIC_REQUIRE(requires(test_range Range) { std::ranges::cbegin(Range); });

	STATIC_REQUIRE(requires(test_range Range) { std::ranges::end(Range); });
	STATIC_REQUIRE(requires(test_range Range) { std::ranges::cend(Range); });

	STATIC_REQUIRE(requires(test_range Range) { std::ranges::find(Range, *Range.begin()); });
}

TEST_CASE("enumerator.dynamic")
{
	enum
	{
		e_begin = 5,
		e_end = 14
	};

	bool Finalised = false;

	{
		auto TestEnumerator = inline_enumerator<int>([Counter = static_cast<int>(e_begin)](bool const Reset, int& Value) mutable
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

#include "common/expected.hpp"

TEST_CASE("expected")
{
	expected<string, int> const GoodValue(L"42"s), BadValue(42);

	REQUIRE(GoodValue.has_value());
	REQUIRE(GoodValue.value() == L"42"sv);
	REQUIRE_THROWS_AS(GoodValue.error(), std::logic_error);

	REQUIRE(!BadValue.has_value());
	REQUIRE(BadValue.error() == 42);
	REQUIRE_THROWS_AS(BadValue.value(), int);
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
	REQUIRE(from_string<double>(L"0.03125"sv) == 0.03125);

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
	REQUIRE_THROWS_AS(from_string<double>(L"1"sv, {}, 3), std::invalid_argument);

	{
		int Value;
		REQUIRE(!from_string(L"qqq"sv, Value));
	}
}

//----------------------------------------------------------------------------

#include "common/function_ref.hpp"

TEST_CASE("function_ref")
{
	{
		struct s
		{
			static int square(int const i) { return i * i; }
		};

		function_ref const Func = &s::square;
		REQUIRE(Func(2) == 4);
	}

	{
		struct square
		{
			int operator()(int const i) const { return i * i; }
		};

		square const Square;
		function_ref const Func = Square;
		REQUIRE(Func(2) == 4);
	}

	{
		const auto square = [](int const i) { return i * i; };

		function_ref const Func = square;
		REQUIRE(Func(2) == 4);
	}

	{
		const auto square = [&](int const i) { return i * i; };

		function_ref const Func = square;
		REQUIRE(Func(2) == 4);
	}

	{
		auto square = [&](int const i) mutable { return i * i; };

		function_ref const Func = square;
		REQUIRE(Func(2) == 4);
	}

	{
		const auto square = [](int const i) mutable { return i * i; };

		function_ref const Func = square;
		REQUIRE(Func(2) == 4);
	}
}

//----------------------------------------------------------------------------

#include "common/function_traits.hpp"

TEST_CASE("function_traits")
{
	{
		using t = function_traits<void()>;
		STATIC_REQUIRE(t::arity == 0);
		STATIC_REQUIRE(std::same_as<t::result_type, void>);
	}

	{
		using t = function_traits<char(short, int, long)>;
		STATIC_REQUIRE(t::arity == 3);
		STATIC_REQUIRE(std::same_as<t::arg<0>, short>);
		STATIC_REQUIRE(std::same_as<t::arg<1>, int>);
		STATIC_REQUIRE(std::same_as<t::arg<2>, long>);
		STATIC_REQUIRE(std::same_as<t::result_type, char>);
	}

	{
		struct s { double f(bool) const { return 0; } };
		using t = function_traits<decltype(&s::f)>;
		STATIC_REQUIRE(t::arity == 1);
		STATIC_REQUIRE(std::same_as<t::arg<0>, bool>);
		STATIC_REQUIRE(std::same_as<t::result_type, double>);
	}

	{
		const auto l = [&](char){ return 0.0; };
		using t = function_traits<decltype(l)>;
		STATIC_REQUIRE(t::arity == 1);
		STATIC_REQUIRE(std::same_as<t::arg<0>, char>);
		STATIC_REQUIRE(std::same_as<t::result_type, double>);
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
	STATIC_REQUIRE(std::same_as<decltype(keep_alive(std::declval<type>())), keep_alive<type>>);
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

	{
		lazy<string> Str([&] { return L""s; });
		STATIC_REQUIRE(std::same_as<decltype(*Str), string&>);
		STATIC_REQUIRE(std::same_as<decltype(Str->c_str()), decltype(std::declval<string&>().c_str())>);
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
	movable m1;
	REQUIRE(m1);

	const auto m2 = std::move(m1);
	REQUIRE(m2);
	REQUIRE(!m1);
}

//----------------------------------------------------------------------------

#include "common/multifunction.hpp"

TEST_CASE("multifunction")
{
	struct impl
	{
		auto operator()(double const v) const { return v; }
		auto operator()(bool const v)   const { return v; }
		auto operator()(int const v)    const { return v; }
	};

	multifunction
	<
		double(double),
		bool(bool),
		int(int)
	>
	Func = impl{};

	REQUIRE(Func(42.25) == 42.25);
	REQUIRE(Func(true) == true);
	REQUIRE(Func(99) == 99);
}

//----------------------------------------------------------------------------

#include "common/noncopyable.hpp"

TEST_CASE("noncopyable")
{
	class c: noncopyable
	{
	};

	STATIC_REQUIRE(!std::is_copy_constructible_v<c>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<c>);
	STATIC_REQUIRE(std::movable<c>);
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

#ifdef _DEBUG
	REQUIRE(std::ranges::all_of(Data, [](std::byte const Value) { return Value == std::byte{0xFE}; }));
#endif
}

//----------------------------------------------------------------------------

#include "common/preprocessor.hpp"

TEST_CASE("preprocessor.copy-move")
{
	{
		struct s
		{
			NONCOPYABLE(s);
			s() = default;
		};

		STATIC_REQUIRE(!std::is_copy_constructible_v<s>);
		STATIC_REQUIRE(!std::is_copy_assignable_v<s>);
		STATIC_REQUIRE(!std::is_move_constructible_v<s>);
		STATIC_REQUIRE(!std::is_move_assignable_v<s>);
	}

	{
		struct s
		{
			NONMOVABLE(s);
			s() = default;
		};

		STATIC_REQUIRE(!std::is_copy_constructible_v<s>);
		STATIC_REQUIRE(!std::is_copy_assignable_v<s>);
		STATIC_REQUIRE(!std::is_move_constructible_v<s>);
		STATIC_REQUIRE(!std::is_move_assignable_v<s>);
	}

	{
		struct s
		{
			MOVABLE(s);
			s() = default;
		};

		STATIC_REQUIRE(!std::is_copy_constructible_v<s>);
		STATIC_REQUIRE(!std::is_copy_assignable_v<s>);
		STATIC_REQUIRE(std::is_move_constructible_v<s>);
		STATIC_REQUIRE(std::is_move_assignable_v<s>);
	}

	{
		struct s
		{
			NONCOPYABLE(s);
			MOVABLE(s);
			s() = default;
		};

		STATIC_REQUIRE(!std::is_copy_constructible_v<s>);
		STATIC_REQUIRE(!std::is_copy_assignable_v<s>);
		STATIC_REQUIRE(std::is_move_constructible_v<s>);
		STATIC_REQUIRE(std::is_move_assignable_v<s>);
	}

	{
		struct s
		{
			COPYABLE(s);
			MOVABLE(s);
			s() = default;
		};

		STATIC_REQUIRE(std::is_copy_constructible_v<s>);
		STATIC_REQUIRE(std::is_copy_assignable_v<s>);
		STATIC_REQUIRE(std::is_move_constructible_v<s>);
		STATIC_REQUIRE(std::is_move_assignable_v<s>);
	}

	{
		struct s
		{
			NOT_COPY_ASSIGNABLE(s);
			s() = default;
		};

		STATIC_REQUIRE(std::is_copy_constructible_v<s>);
		STATIC_REQUIRE(!std::is_copy_assignable_v<s>);
		STATIC_REQUIRE(std::is_move_constructible_v<s>);
		STATIC_REQUIRE(!std::is_move_assignable_v<s>);
	}

	{
		struct s
		{
			MOVE_ASSIGNABLE(s);
			s() = default;
		};

		STATIC_REQUIRE(!std::is_copy_constructible_v<s>);
		STATIC_REQUIRE(!std::is_copy_assignable_v<s>);
		STATIC_REQUIRE(!std::is_move_constructible_v<s>);
		STATIC_REQUIRE(std::is_move_assignable_v<s>);
	}

	{
		struct s
		{
			NOT_MOVE_ASSIGNABLE(s);
			s() = default;
		};

		STATIC_REQUIRE(!std::is_copy_constructible_v<s>);
		STATIC_REQUIRE(!std::is_copy_assignable_v<s>);
		STATIC_REQUIRE(!std::is_move_constructible_v<s>);
		STATIC_REQUIRE(!std::is_move_assignable_v<s>);
	}
}

TEST_CASE("preprocessor.literals")
{
	{
		#define TEST_LITERAL "la\0rd"
		constexpr size_t Size = 5;
		static_assert(sizeof(TEST_LITERAL) - 1 == Size);

		const auto Str = CHAR_S(TEST_LITERAL);
		STATIC_REQUIRE(std::same_as<decltype(Str), std::string const>);
		REQUIRE(Str.size() == Size);

		constexpr auto View = CHAR_SV(TEST_LITERAL);
		STATIC_REQUIRE(std::same_as<decltype(View), std::string_view const>);
		STATIC_REQUIRE(View.size() == Size);

		const auto WStr = WIDE_S(TEST_LITERAL);
		STATIC_REQUIRE(std::same_as<decltype(WStr), std::wstring const>);
		REQUIRE(WStr.size() == Size);

		constexpr auto WView = WIDE_SV(TEST_LITERAL);
		STATIC_REQUIRE(std::same_as<decltype(WView), std::wstring_view const>);
		STATIC_REQUIRE(WView.size() == Size);

		#undef TEST_LITERAL
	}

	{
		#define TEST_TOKEN meow

		{
			STATIC_REQUIRE(std::same_as<decltype(LITERAL(TEST_TOKEN)), char const(&)[11]>);
			STATIC_REQUIRE(std::same_as<decltype(WIDE_LITERAL(TEST_TOKEN)), wchar_t const(&)[11]>);

			const auto Literal = LITERAL(TEST_TOKEN);
			STATIC_REQUIRE(std::same_as<decltype(Literal), char const* const>);
			REQUIRE(Literal == "TEST_TOKEN"sv);

			const auto WLiteral = WIDE_LITERAL(TEST_TOKEN);
			STATIC_REQUIRE(std::same_as<decltype(WLiteral), wchar_t const* const>);
			REQUIRE(WLiteral == L"TEST_TOKEN"sv);

			const auto WView = WIDE_SV_LITERAL(TEST_TOKEN);
			STATIC_REQUIRE(std::same_as<decltype(WView), std::wstring_view const>);
			REQUIRE(WView == L"TEST_TOKEN"sv);
		}

		{
			STATIC_REQUIRE(std::same_as<decltype(EXPAND_TO_LITERAL(TEST_TOKEN)), char const(&)[5]>);
			STATIC_REQUIRE(std::same_as<decltype(EXPAND_TO_WIDE_LITERAL(TEST_TOKEN)), wchar_t const(&)[5]>);

			const auto Literal = EXPAND_TO_LITERAL(TEST_TOKEN);
			STATIC_REQUIRE(std::same_as<decltype(Literal), char const* const>);
			REQUIRE(Literal == "meow"sv);

			const auto WLiteral = EXPAND_TO_WIDE_LITERAL(TEST_TOKEN);
			STATIC_REQUIRE(std::same_as<decltype(WLiteral), wchar_t const* const>);
			REQUIRE(WLiteral == L"meow"sv);

			const auto WView = EXPAND_TO_WIDE_SV_LITERAL(TEST_TOKEN);
			STATIC_REQUIRE(std::same_as<decltype(WView), std::wstring_view const>);
			REQUIRE(WView == L"meow"sv);
		}

		#undef TEST_TOKEN
	}
}

//----------------------------------------------------------------------------

#include "common/source_location.hpp"

TEST_CASE("source_location")
{
	struct test
	{
		static void method()
		{
			constexpr auto Location = source_location::current(); constexpr auto Line = __LINE__;

			STATIC_REQUIRE(std::string_view(Location.file_name()) == __FILE__);
			STATIC_REQUIRE(std::string_view(Location.function_name()) == __func__);
			STATIC_REQUIRE(Location.line() == Line);
		}
	};

	{
		source_location constexpr Location("squash", "banana", 42);

		STATIC_REQUIRE(Location.file_name() == "squash"sv);
		STATIC_REQUIRE(Location.function_name() == "banana"sv);
		STATIC_REQUIRE(Location.line() == 42);
	}
}

//----------------------------------------------------------------------------

#include "common/span.hpp"

TEST_CASE("span.static")
{
	{
		int Data[2]{};
		span Span(Data);
		STATIC_REQUIRE(std::same_as<decltype(*Span.begin()), int&>);
	}

	{
		int Data[2]{};
		span Span{ Data };
		STATIC_REQUIRE(std::same_as<decltype(*Span.begin()), int* const&>);
	}

	{
		using v = int;
		using cv = int const;

		STATIC_REQUIRE(std::same_as<decltype(span({1, 2})), span<cv>>);
		STATIC_REQUIRE(std::same_as<decltype(span(std::array<v, 0>{})), span<v>>);
		STATIC_REQUIRE(std::same_as<decltype(span(std::array<cv, 0>{})), span<cv>>);
	}

	{
		using v = int;
		using cv = int const;
		using m_f_span = span<v>;
		using c_f_span = span<cv>;
		using m_s_span = std::span<v>;
		using c_s_span = std::span<cv>;

		STATIC_REQUIRE(std::convertible_to<m_f_span, m_s_span>);
		STATIC_REQUIRE(std::convertible_to<m_s_span, m_f_span>);
		STATIC_REQUIRE(std::convertible_to<c_f_span, c_s_span>);
		STATIC_REQUIRE(std::convertible_to<c_s_span, c_f_span>);

		STATIC_REQUIRE(std::assignable_from<m_s_span&, m_f_span>);
		STATIC_REQUIRE(std::assignable_from<m_f_span&, m_s_span>);
		STATIC_REQUIRE(std::assignable_from<c_s_span&, c_f_span>);
		STATIC_REQUIRE(std::assignable_from<c_f_span&, c_s_span>);

		STATIC_REQUIRE(std::convertible_to<m_f_span, c_f_span>);
		STATIC_REQUIRE(std::convertible_to<m_f_span, c_s_span>);
		STATIC_REQUIRE(std::convertible_to<m_s_span, c_f_span>);

		STATIC_REQUIRE(std::assignable_from<c_f_span&, m_f_span>);
		STATIC_REQUIRE(std::assignable_from<c_s_span&, m_f_span>);
		STATIC_REQUIRE(std::assignable_from<c_f_span&, m_s_span>);

		STATIC_REQUIRE(!std::convertible_to<c_f_span, m_f_span>);
		STATIC_REQUIRE(!std::convertible_to<c_f_span, m_s_span>);
		STATIC_REQUIRE(!std::convertible_to<c_s_span, m_f_span>);

		STATIC_REQUIRE(!std::assignable_from<m_f_span&, c_f_span>);
		STATIC_REQUIRE(!std::assignable_from<m_s_span&, c_f_span>);
		STATIC_REQUIRE(!std::assignable_from<m_f_span&, c_s_span>);
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
		bool IsThrown = false;

		try
		{
			SCOPE_TYPE(type) { IsTriggered = true; };
			REQUIRE(!IsTriggered);
			if (Throw)
				throw 1;
		}
		catch (int)
		{
			IsThrown = true;
		}

		REQUIRE(IsTriggered == MustBeTriggered);
		REQUIRE(IsThrown == Throw);
	}

	enum
	{
		on_fail    = 0_bit,
		on_success = 1_bit,
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

#include "common/singleton.hpp"

TEST_CASE("singleton")
{
	class c: public singleton<c>
	{
		IMPLEMENTS_SINGLETON;
	};

	const auto& Ref1 = c::instance();
	const auto& Ref2 = c::instance();

	REQUIRE(&Ref1 == &Ref2);
}

//----------------------------------------------------------------------------

#include "common/smart_ptr.hpp"

TEST_CASE("smart_ptr.char_ptr_n")
{
	const char_ptr_n<1> Ptr;
	constexpr auto ActualStaticSize = sizeof(std::unique_ptr<char[]>) / sizeof(char);
	const char_ptr_n<ActualStaticSize> Ptr2;

	STATIC_REQUIRE(sizeof(Ptr) == sizeof(Ptr2));
	REQUIRE(Ptr.size() == Ptr2.size());
}

TEST_CASE("smart_ptr.block_ptr")
{
	struct s
	{
		int size;
		char str[1];
	};

	{
		const struct test
		{
			block_ptr<s, sizeof(s) + 8> S{ sizeof(s) + 8 };
			int Sentinel = 0x12345678;
		}
		Test;

		REQUIRE(in_closed_range(
			std::bit_cast<uintptr_t>(&Test.S),
			std::bit_cast<uintptr_t>(&Test.S->str),
			std::bit_cast<uintptr_t>(&Test.Sentinel)
		));

		const auto Str = "01234657"sv;
		const auto Ptr = Test.S->str;
		copy_memory(Str.data(), Ptr, Str.size());
		REQUIRE(Str == std::string_view(Ptr, Str.size()));
		REQUIRE(Test.Sentinel == 0x12345678);
	}

	{
		const struct test
		{
			block_ptr<s> S{ sizeof(s) + 8 };
			int Sentinel = 0x12345678;
		}
		Test;

		REQUIRE(!in_closed_range(
			std::bit_cast<uintptr_t>(&Test.S),
			std::bit_cast<uintptr_t>(&Test.S->str),
			std::bit_cast<uintptr_t>(&Test.Sentinel)
		));

		const auto Str = "01234657"sv;
		const auto Ptr = Test.S->str;
		copy_memory(Str.data(), Ptr, Str.size());
		REQUIRE(Str == std::string_view(Ptr, Str.size()));
		REQUIRE(Test.Sentinel == 0x12345678);
	}
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
		REQUIRE(contains(i.Src, i.Pattern) == i.Contains);
	}
}

TEST_CASE("string_utils.within")
{
	const auto Haystack = L"banana"sv;

	REQUIRE(within(Haystack, Haystack.substr(0)));
	REQUIRE(within(Haystack, Haystack.substr(0, 2)));
	REQUIRE(within(Haystack, Haystack.substr(2, 2)));
	REQUIRE(within(Haystack, Haystack.substr(4)));
	REQUIRE(within(Haystack, Haystack.substr(Haystack.size() - 1)));

	// Empty views are not within anything.
	REQUIRE(!within(Haystack, Haystack.substr(0, 0)));
	REQUIRE(!within(Haystack, Haystack.substr(1, 0)));
	REQUIRE(!within(Haystack, Haystack.substr(Haystack.size())));
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

TEST_CASE("string_utils.null_terminated")
{
	const auto Str = L"12345"sv;

	const auto LuckyCase = null_terminated(Str.substr(1));
	REQUIRE(!LuckyCase.c_str()[4]);
	REQUIRE(LuckyCase.c_str() == Str.data() + 1);

	const auto UnluckyCase = null_terminated(Str.substr(0, 2));
	REQUIRE(!UnluckyCase.c_str()[2]);
	REQUIRE(UnluckyCase.c_str() != Str.data());
}

TEST_CASE("string_utils.string_copyref")
{
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

	{
		STATIC_REQUIRE(!std::is_copy_constructible_v<string_copyref>);
		STATIC_REQUIRE(!std::is_copy_assignable_v<string_copyref>);
		STATIC_REQUIRE(!std::is_move_constructible_v<string_copyref>);
		STATIC_REQUIRE(!std::is_move_assignable_v<string_copyref>);
	}
}

TEST_CASE("string_utils.concat")
{
	{
		auto Str = L"ba"s;
		append(Str, L"nan"sv, L'a');
		REQUIRE(Str == L"banana"sv);
	}

	REQUIRE(concat(L'a', L"bc", L"def"sv, L"1234"s) == L"abcdef1234"sv);
	REQUIRE(concat(L""sv, L""sv).empty());

	REQUIRE(join(L","sv, std::array{ L"123"sv, L"45"sv, L""sv, L"6"sv }) == L"123,45,,6"sv);

	REQUIRE(L"123"s + L"45"sv == L"12345"sv);
	REQUIRE(L"123"sv + L"45"s == L"12345"sv);
	REQUIRE(L"123"sv + L"45"sv == L"12345"sv);
}

TEST_CASE("string_utils.lvalue_string_view")
{
	STATIC_REQUIRE(std::constructible_from<lvalue_string_view, string_view>);
	STATIC_REQUIRE(std::constructible_from<lvalue_string_view, string const&>);
	STATIC_REQUIRE(std::constructible_from<lvalue_string_view, string&>);
	STATIC_REQUIRE(!std::constructible_from<lvalue_string_view, string&&>);
	STATIC_REQUIRE(!std::constructible_from<lvalue_string_view, string>);
}

TEST_CASE("string_utils.generic_lookup")
{
	const unordered_string_map<int> Map
	{
		{ L"123"s, 123 },
	};

	REQUIRE(Map.find(L"123"sv) != Map.cend());
	REQUIRE(Map.find(L"123") != Map.cend());
}

//----------------------------------------------------------------------------

#include "common/type_traits.hpp"

TEST_CASE("type_traits")
{
	STATIC_REQUIRE(is_one_of_v<int, char, bool, int>);
	STATIC_REQUIRE(!is_one_of_v<int, char, bool, unsigned int>);

	enum class foo: int;
	STATIC_REQUIRE(std::same_as<sane_underlying_type<foo>, int>);
	STATIC_REQUIRE(std::same_as<sane_underlying_type<int>, int>);
	STATIC_REQUIRE_ERROR(void, typename sane_underlying_type<TestType>);
}

//----------------------------------------------------------------------------

#include "common/utility.hpp"

TEST_CASE("utility.base")
{
	struct c
	{
	};

	struct d: base<c>
	{
		d()
		{
			STATIC_REQUIRE(std::same_as<d::base_type, c>);
			STATIC_REQUIRE(std::same_as<d::base_ctor, base<c>>);
		}
	}
	D;
}

TEST_CASE("utility.grow_exp")
{
	for (const auto i: std::views::iota(0uz, 32uz))
	{
		const auto ExpectedIncrease = std::max(1uz, i / 2);
		REQUIRE(grow_exp(i, 0) == i);
		REQUIRE(grow_exp(i, {}) == i + ExpectedIncrease);
		REQUIRE(grow_exp(i, i + 1) == i + ExpectedIncrease);
		REQUIRE(grow_exp(i, i * 2) == i * 2);
	}
}

TEMPLATE_TEST_CASE("utility.reserve_exp", "", string, std::vector<int>)
{
	TestType Container;
	Container.resize(42);
	const auto InitialCapacity = Container.capacity();

	SECTION("no shrink")
	{
		// Mandated by the standard now, but just in case.
		reserve_exp(Container, 1);
		REQUIRE(Container.capacity() >= InitialCapacity);
	}

	SECTION("exponential < factor")
	{
		reserve_exp(Container, InitialCapacity + InitialCapacity / 10);
		REQUIRE(Container.capacity() >= InitialCapacity  + InitialCapacity / 2);
	}

	SECTION("exponential > factor")
	{
		reserve_exp(Container, InitialCapacity * 2);
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

	const auto h1 = hash_range(s1);
	const auto h2 = hash_range(s2);
	const auto h3 = hash_range(s3);

	REQUIRE(h1 == h2);
	REQUIRE(h2 != h3);
}

TEST_CASE("utility.sign")
{
	STATIC_REQUIRE(std::same_as<decltype(as_signed(0u)), signed>);
	STATIC_REQUIRE(std::same_as<decltype(as_unsigned(0)), unsigned>);
}

TEST_CASE("utility.bit")
{
	STATIC_REQUIRE(0_bit == 1ull << 0);
	STATIC_REQUIRE(63_bit == 1ull << 63);
}

TEST_CASE("utility.flags")
{
	{
		enum class scoped_flags
		{
			f0 = 0_bit,
			f1 = 1_bit,
			f2 = 2_bit,
			f3 = 3_bit,

			is_bit_flags
		};

		auto Flags = scoped_flags::f0 | scoped_flags::f3;

		REQUIRE(flags::check_any(Flags, scoped_flags::f0));
		REQUIRE(flags::check_one(Flags, scoped_flags::f0));
		REQUIRE(flags::check_all(Flags, scoped_flags::f3));

		flags::set(Flags, scoped_flags::f1);
		REQUIRE(flags::check_one(Flags, scoped_flags::f1));

		flags::clear(Flags, scoped_flags::f1);
		REQUIRE(!flags::check_one(Flags, scoped_flags::f1));

		flags::invert(Flags, scoped_flags::f0);
		REQUIRE(!flags::check_one(Flags, scoped_flags::f0));

		flags::change(Flags, scoped_flags::f0, true);
		REQUIRE(flags::check_one(Flags, scoped_flags::f0));

		Flags = {};
		flags::copy(Flags, scoped_flags::f1 | scoped_flags::f2, -1);
		REQUIRE(flags::check_all(Flags, scoped_flags::f1 | scoped_flags::f2));
	}

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

TEST_CASE("utility.is_aligned")
{
	{
		const int i = 42;
		REQUIRE(is_aligned(i));
	}

	{
		alignas(int) const char data[sizeof(int) * 3]{};
		static_assert(sizeof(int) > sizeof(char));

		REQUIRE(is_aligned(view_as<int>(data, 0)));
		REQUIRE(!is_aligned(*std::bit_cast<int*>(data + 1))); // view_as asserts alignof(T)
		REQUIRE(is_aligned(view_as<int>(data, sizeof(int))));
		REQUIRE(!is_aligned(*std::bit_cast<int*>(data + sizeof(int) + 1))); // view_as asserts alignof(T)
	}
}

TEST_CASE("utility.enum_helpers")
{
	enum class e
	{
		foo = 1,
		bar = 2,
	};

	{
		constexpr auto r = enum_helpers::detail::operation<std::plus<>>(e::foo, e::bar);
		STATIC_REQUIRE(std::same_as<decltype(r), e const>);
		STATIC_REQUIRE(std::to_underlying(r) == 3);
	}

	{
		constexpr auto r = enum_helpers::detail::operation<std::bit_and<>>(e::foo, e::bar);
		STATIC_REQUIRE(std::same_as<decltype(r), e const>);
		STATIC_REQUIRE(std::to_underlying(r) == 0);
	}
}

TEST_CASE("utility.overload")
{
	const auto Composite = overload
	{
		[](int    i) { return i; },
		[](bool   b) { return b; },
		[](double d) { return d; },
	};

	STATIC_REQUIRE(Composite(42) == 42);
	STATIC_REQUIRE(Composite(false) == false);
	STATIC_REQUIRE(Composite(0.5) == 0.5);
	STATIC_REQUIRE(!std::convertible_to<decltype(Composite('q')), char>);
}

TEST_CASE("utility.casts")
{
	int Data[]{ 42, 24 };
	void* Ptr = &Data;

	{
		auto& ObjectView = view_as<int>(Ptr);
		STATIC_REQUIRE(std::same_as<decltype(ObjectView), int const&>);
		REQUIRE(&ObjectView == &Data[0]);
	}

	{
		auto& ObjectView = view_as<int>(Ptr, sizeof(int));
		STATIC_REQUIRE(std::same_as<decltype(ObjectView), int const&>);
		REQUIRE(&ObjectView == &Data[1]);
	}

	{
		auto ObjectView = view_as_opt<int>(Ptr, sizeof(Data), 0);
		STATIC_REQUIRE(std::same_as<decltype(ObjectView), int const*>);
		REQUIRE(ObjectView == &Data[0]);
	}

	{
		auto ObjectView = view_as_opt<int>(Ptr, sizeof(Data), sizeof(int) * 1);
		STATIC_REQUIRE(std::same_as<decltype(ObjectView), int const*>);
		REQUIRE(ObjectView == &Data[1]);
	}

	{
		auto ObjectView = view_as_opt<int>(Ptr, sizeof(Data), sizeof(int) * 2);
		STATIC_REQUIRE(std::same_as<decltype(ObjectView), int const*>);
		REQUIRE(ObjectView == nullptr);
	}

	{
		auto& ObjectEdit = edit_as<int>(Ptr);
		STATIC_REQUIRE(std::same_as<decltype(ObjectEdit), int&>);
		REQUIRE(&ObjectEdit == &Data[0]);
	}

	{
		auto& ObjectEdit = edit_as<int>(Ptr, sizeof(int) * 1);
		STATIC_REQUIRE(std::same_as<decltype(ObjectEdit), int&>);
		REQUIRE(&ObjectEdit == &Data[1]);
	}

	{
		[[maybe_unused]] const auto* const ConstPtr = Ptr;
		STATIC_REQUIRE_ERROR(int, edit_as<TestType>(ConstPtr));
		STATIC_REQUIRE_ERROR(int, edit_as<TestType>(ConstPtr, sizeof(int) * 1));

		[[maybe_unused]] string NonTrivial;
		STATIC_REQUIRE_ERROR(int, view_as<TestType>(&NonTrivial));
		STATIC_REQUIRE_ERROR(string, view_as_opt<TestType>(Ptr));
		STATIC_REQUIRE_ERROR(int, edit_as<TestType>(&NonTrivial));
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
		REQUIRE(uuid::try_parse(i.Str).has_value() == i.Valid);

		if (i.Valid)
			REQUIRE_NOTHROW(uuid::parse(i.Str));
		else
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

#include "common/view/zip.hpp"

TEST_CASE("view.zip.static")
{
	using test_range = decltype(zip(std::declval<std::span<int>&>(), std::declval<std::span<int>&>()));

	STATIC_REQUIRE(std::ranges::range<test_range>);
	STATIC_REQUIRE(std::ranges::bidirectional_range<test_range>);
	STATIC_REQUIRE(std::ranges::sized_range<test_range>);

	STATIC_REQUIRE(requires(test_range Range) { std::ranges::begin(Range); });
	STATIC_REQUIRE(requires(test_range Range) { std::ranges::cbegin(Range); });
	STATIC_REQUIRE(requires(test_range Range) { std::ranges::rbegin(Range); });
	STATIC_REQUIRE(requires(test_range Range) { std::ranges::crbegin(Range); });

	STATIC_REQUIRE(requires(test_range Range) { std::ranges::end(Range); });
	STATIC_REQUIRE(requires(test_range Range) { std::ranges::cend(Range); });
	STATIC_REQUIRE(requires(test_range Range) { std::ranges::rend(Range); });
	STATIC_REQUIRE(requires(test_range Range) { std::ranges::crend(Range); });

	STATIC_REQUIRE(requires(test_range Range) { std::ranges::find(Range, *Range.begin()); });
}

TEST_CASE("view.zip.dynamic")
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
