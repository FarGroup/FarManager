/*
tvar.cpp

Реализация класса TVar (для макросов)

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
#include "tvar.hpp"

// Internal:
#include "string_sort.hpp"

// Platform:

// Common:
#include "common.hpp"
#include "common/from_string.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

TVar::TVar():
	vType(Type::Unknown)
{
}

TVar::TVar(long long v):
	inum(v),
	vType(Type::Integer)
{
}

TVar::TVar(long long const v, Type const t):
	inum(v),
	vType(t)
{
}

TVar::TVar(int v):
	TVar(static_cast<long long>(v))
{
}

TVar::TVar(double v):
	dnum(v),
	vType(Type::Double)
{
}

TVar::TVar(string_view const v):
	str(v),
	vType(Type::String)
{
}

TVar::TVar(const wchar_t* v):
	TVar(string_view(NullToEmpty(v)))
{
}

TVar::TVar(void* v):
	ptr(v),
	vType(Type::Pointer)
{
}

TVar::TVar(Dialog* v):
	ptr(v),
	vType(Type::Dialog)
{
}

const string& TVar::toString()
{
	// this already writes to str
	(void)asString();
	vType = Type::String;
	return str;
}

long long TVar::toInteger()
{
	inum = asInteger();
	vType = Type::Integer;
	return inum;
}

double TVar::toDouble()
{
	dnum = asDouble();
	vType = Type::Double;
	return dnum;
}

const string& TVar::asString() const
{
	switch (vType)
	{
	case Type::String:
		break;

	case Type::Integer:
		str = ::str(inum);
		break;

	case Type::Double:
		// str() is implemented in terms of fmt::to_wstring().
		// For doubles fmt::to_wstring adds ".0" even if there's no fractional part
		// (e.g. 1234.0 to "1234.0"), and it's a feature (see issue #1153).
		// For historical reasons we prefer the shortest possible representation, hence "g".
		str = far::format(L"{:.14g}"sv, dnum);
		break;

	default:
		str.clear();
		break;
	}

	return str;
}

long long TVar::asInteger() const
{
	switch (vType)
	{
	case Type::Integer:
	case Type::Table:
		return inum;

	case Type::Double:
		return dnum;

	case Type::String:
		{
			long long Value;
			return from_string(str, Value)? Value : 0;
		}

	default:
		return 0;
	}
}

double TVar::asDouble() const
{
	switch (vType)
	{
	case Type::Integer:
	case Type::Table:
		return inum;

	case Type::Double:
		return dnum;

	case Type::String:
		{
			double Value;
			return from_string(str, Value)? Value : 0;
		}

	default:
		return 0;
	}
}

void* TVar::asPointer() const
{
	switch (vType)
	{
	case Type::Pointer:
		return ptr;

	default:
		return nullptr;
	}
}

Dialog* TVar::asDialog() const
{
	switch (vType)
	{
	case Type::Dialog:
		return static_cast<Dialog*>(ptr);

	default:
		return nullptr;
	}
}

bool TVar::isNumber() const
{
	switch (type())
	{
	case Type::Integer:
	case Type::Double:
	case Type::Table:
		return true;

	default:
		return false;
	}
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("TVar")
{
	const auto
		IntMin = std::numeric_limits<int64_t>::min(),
		IntMax = std::numeric_limits<int64_t>::max();

	static const struct tests
	{
		TVar t;
		TVar::Type Type;
		bool IsNumber;
		string_view sValue;
		long long iValue;
		double dValue;
	}
	Tests[]
	{
		{ TVar{},                                     TVar::Type::Unknown, false, {},                        {},     {} },
		{ TVar{ 0 },                                  TVar::Type::Integer, true,  L"0"sv,                    {},     {} },
		{ TVar{ IntMin },                             TVar::Type::Integer, true,  L"-9223372036854775808"sv, IntMin, static_cast<double>(IntMin) },
		{ TVar{ IntMax },                             TVar::Type::Integer, true,  L"9223372036854775807"sv,  IntMax, static_cast<double>(IntMax) },
		{ TVar{ 42.0 },                               TVar::Type::Double,  true,  L"42"sv,                   42,     42.0 },
		{ TVar{ 42.123 },                             TVar::Type::Double,  true,  L"42.123"sv,               42,     42.123 },
		{ TVar{ L"banana"sv },                        TVar::Type::String,  false, L"banana"sv,               {},     {} },
		{ TVar{ L"Bamboléo" },                        TVar::Type::String,  false, L"Bamboléo"sv,             {},     {} },
		{ TVar{ std::bit_cast<void*>(intptr_t{42}) }, TVar::Type::Pointer, false, L""sv,                     {},     {} },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.t.type() == i.Type);
		REQUIRE(i.t.isNumber() == i.IsNumber);
		REQUIRE(i.t.asString() == i.sValue);
		REQUIRE(i.t.asInteger() == i.iValue);
		REQUIRE(i.t.asDouble() == i.dValue);
	}
}

#endif
