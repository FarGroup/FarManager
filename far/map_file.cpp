/*
map_file.cpp
*/
/*
Copyright © 2021 Far Group
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
#include "map_file.hpp"

// Internal:
#include "encoding.hpp"
#include "filestr.hpp"
#include "imports.hpp"
#include "pathmix.hpp"
#include "RegExp.hpp"
#include "log.hpp"
#include "global.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/from_string.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

struct map_file::line
{
	string Name;
	string const* File;
};

static string get_map_name(string_view const ModuleName)
{
	return name_ext(
		!ModuleName.empty()?
		ModuleName :
		Global?
			Global->g_strFarModuleName :
			os::fs::get_current_process_file_name()
	).first + L".map"sv;
}

map_file::map_file(string_view const ModuleName)
{
	os::fs::file const Handle(get_map_name(ModuleName), FILE_READ_DATA, os::fs::file_share_read, nullptr, OPEN_EXISTING);
	if (!Handle)
		return;

	try
	{
		os::fs::filebuf StreamBuffer(Handle, std::ios::in);
		std::istream Stream(&StreamBuffer);
		Stream.exceptions(Stream.badbit | Stream.failbit);

		read(Stream);
	}
	catch (std::exception const& e)
	{
		m_Symbols.clear();
		m_Files.clear();

		LOGERROR(L"{}"sv, e);
	}
}

map_file::~map_file() = default;

static void undecorate(string& Symbol)
{
	const auto UndecorateFlags =
		UNDNAME_NO_FUNCTION_RETURNS |
		UNDNAME_NO_ALLOCATION_MODEL |
		UNDNAME_NO_ALLOCATION_LANGUAGE |
		UNDNAME_NO_ACCESS_SPECIFIERS |
		UNDNAME_NO_MEMBER_TYPE |
		UNDNAME_32_BIT_DECODE;

	if (wchar_t Buffer[MAX_SYM_NAME]; imports.UnDecorateSymbolNameW && imports.UnDecorateSymbolNameW(Symbol.c_str(), Buffer, static_cast<DWORD>(std::size(Buffer)), UndecorateFlags))
	{
		Symbol = Buffer;
		return;
	}

	if (char Buffer[MAX_SYM_NAME]; imports.UnDecorateSymbolName && imports.UnDecorateSymbolName(encoding::ansi::get_bytes(Symbol).c_str(), Buffer, static_cast<DWORD>(std::size(Buffer)), UndecorateFlags))
	{
		encoding::ansi::get_chars(Buffer, Symbol);
		return;
	}
}

static map_file::info get_impl(uintptr_t const Address, std::map<uintptr_t, map_file::line>& Symbols)
{
	auto [Begin, End] = Symbols.equal_range(Address);

	if (Begin == Symbols.end() || Begin->first > Address)
	{
		if (Begin == Symbols.begin())
			return {};

		--Begin;
	}

	undecorate(Begin->second.Name);

	return
	{
		.File = *Begin->second.File,
		.Symbol = Begin->second.Name,
		.Displacement = Address - Begin->first
	};
}

map_file::info map_file::get(uintptr_t const Address)
{
	return get_impl(Address, m_Symbols);
}

enum class map_format
{
	unknown,
	msvc,
	clang,
	gcc,
};

static auto determine_format(string_view const Str)
{
	if (Str.starts_with(L' '))
		return map_format::msvc;

	if (Str == L"Address  Size     Align Out     In      Symbol"sv)
		return map_format::clang;

	if (Str.empty())
		return map_format::gcc;

	return map_format::unknown;
}

static auto determine_format(std::istream& Stream)
{
	const auto Position = Stream.tellg();
	SCOPE_EXIT{ Stream.seekg(Position); };

	const auto Lines = enum_lines(Stream, CP_UTF8);
	if (Lines.empty())
		return map_format::unknown;

	return determine_format(Lines.begin()->Str);
}

static void read_vc(std::istream& Stream, unordered_string_set& Files, std::map<uintptr_t, map_file::line>& Symbols)
{
	RegExp ReBase, ReSymbol;
	ReBase.Compile(L"^ +Preferred load address is ([0-9A-Fa-f]+)$"sv, OP_OPTIMIZE);
	ReSymbol.Compile(L"^ +[0-9A-Fa-f]+:[0-9A-Fa-f]+ +([^ ]+) +([0-9A-Fa-f]+) .+ ([^ ]+)$"sv, OP_OPTIMIZE);

	regex_match Match;
	auto& m = Match.Matches;
	m.reserve(3);

	uintptr_t BaseAddress{};

	for (const auto& i: enum_lines(Stream, encoding::codepage::ansi()))
	{
		if (i.Str.empty())
			continue;

		if (!BaseAddress && ReBase.Search(i.Str, Match))
		{
			BaseAddress = from_string<uintptr_t>(get_match(i.Str, m[1]), {}, 16);
			continue;
		}

		if (ReSymbol.Search(i.Str, Match))
		{
			auto Address = from_string<uintptr_t>(get_match(i.Str, m[2]), {}, 16);
			if (!Address)
				continue;

			if (Address >= BaseAddress)
				Address -= BaseAddress;

			map_file::line Line;
			Line.Name = get_match(i.Str, m[1]);
			const auto File = get_match(i.Str, m[3]);
			Line.File = std::to_address(Files.emplace(File).first);

			Symbols.emplace(Address, std::move(Line));
			continue;
		}
	}
}

static void read_clang(std::istream& Stream, unordered_string_set& Files, std::map<uintptr_t, map_file::line>& Symbols)
{
	RegExp ReObject, ReSymbol;
	ReObject.Compile(L"^[0-9A-Fa-f]+ [0-9A-Fa-f]+ +[0-9]+         (.+)$"sv);
	ReSymbol.Compile(L"^([0-9A-Fa-f]+) [0-9A-Fa-f]+     0                 (.+)$"sv);

	regex_match Match;
	auto& m = Match.Matches;
	m.reserve(2);

	string ObjName;

	for (const auto& i: enum_lines(Stream, CP_UTF8))
	{
		if (i.Str.empty())
			continue;

		if (ReSymbol.Search(i.Str, Match))
		{
			map_file::line Line;
			Line.Name = get_match(i.Str, m[2]);
			Line.File = std::to_address(Files.emplace(ObjName).first);
			const auto Address = from_string<uintptr_t>(get_match(i.Str, m[1]), {}, 16);
			Symbols.emplace(Address, std::move(Line));
			continue;
		}

		if (ReObject.Search(i.Str, Match))
		{
			ObjName = get_match(i.Str, m[1]);
			continue;
		}
	}
}

static void read_gcc(std::istream& Stream, unordered_string_set& Files, std::map<uintptr_t, map_file::line>& Symbols)
{
	RegExp ReFile, ReFileName, ReSymbol;
	ReFile.Compile(L"^File $"sv);
	ReFileName.Compile(L"^\\[ *[0-9]+\\]\\(.+\\)\\(.+\\)\\(.+\\)\\(.+\\) \\(nx 1\\) 0x[0-9A-Fa-f]+ (.+)$"sv);
	ReSymbol.Compile(L"^\\[ *[0-9]+\\]\\(.+\\)\\(.+\\)\\(.+\\)\\(.+\\) \\(nx 0\\) 0x([0-9A-Fa-f]+) (.+)$"sv);

	regex_match Match;
	auto& m = Match.Matches;
	m.reserve(2);

	const auto BaseAddress = 0x1000;

	string LastLine, FileName;

	for (const auto& i: enum_lines(Stream, CP_UTF8))
	{
		if (i.Str.empty())
			continue;

		if (ReFile.Search(i.Str, Match) && ReFileName.Search(LastLine, Match))
		{
			FileName = get_match(LastLine, m[1]);
			LastLine.clear();
			continue;
		}

		if (ReSymbol.Search(i.Str, Match))
		{
			map_file::line Line;
			Line.Name = get_match(i.Str, m[2]);
			Line.File = std::to_address(Files.emplace(FileName).first);
			const auto Address = from_string<uintptr_t>(get_match(i.Str, m[1]), {}, 16) + BaseAddress;
			Symbols.emplace(Address, std::move(Line));
			continue;
		}

		LastLine = i.Str;
	}
}

void map_file::read(std::istream& Stream)
{
	switch (determine_format(Stream))
	{
	case map_format::msvc:
		return read_vc(Stream, m_Files, m_Symbols);

	case map_format::clang:
		return read_clang(Stream, m_Files, m_Symbols);

	case map_format::gcc:
		return read_gcc(Stream, m_Files, m_Symbols);

	case map_format::unknown:
		return;
	}
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("map_file.msvc")
{
	const auto MapFileData =
R"( Far

 Preferred load address is 00400000

  Address         Publics by Value              Rva+Base       Lib:Object

 0000:00000000       ___AbsoluteZero            00000000     <absolute>
 0000:000007e9       ___safe_se_handler_count   000007e9     <absolute>
 0000:00000000       ___ImageBase               00400000     <linker-defined>
 0001:000050e0       ??3@YAXPAX@Z               004060e0 f   LIBCMT:delete_scalar.obj
 0001:0002e450       _sqlite3_step              0042f450 f   sqlite.obj
 0001:000a0d90       _sqlite3_open              004a1d90 f   sqlite.obj
 0001:000a7670       ?_Xlength@?$vector@Ucolumn@@V?$allocator@Ucolumn@@@std@@@std@@CAXXZ 004a8670 f i config.obj
 0001:000a7670       ?_Xlength@?$vector@_WV?$allocator@_W@std@@@std@@CAXXZ 004a8670 f i configdb.obj
 0001:000fe9b0       ??0config_provider@@QAE@Uclear_cache@0@@Z 004ff9b0 f   configdb.obj
 0001:000fec00       ??1config_provider@@QAE@XZ 004ffc00 f   configdb.obj
)"sv;

	std::stringstream Stream(std::string{ MapFileData });

	REQUIRE(determine_format(Stream) == map_format::msvc);

	unordered_string_set Files;
	std::map<uintptr_t, map_file::line> Symbols;

	read_vc(Stream, Files, Symbols);

	REQUIRE(Files.size() == 6u);
	REQUIRE(Symbols.size() == 8u);

	static const struct
	{
		uintptr_t Address;
		map_file::info Info;
	}
	Tests[]
	{
		{ 0x00000000, { L"<linker-defined>"sv,           L"___ImageBase"sv, 0, }, },
		{ 0x000007e9, { L"<absolute>"sv,                 L"___safe_se_handler_count"sv, 0, }, },
		{ 0x000060e0, { L"LIBCMT:delete_scalar.obj"sv,   L"operator delete(void *)"sv, 0, }, },
		{ 0x000060e8, { L"LIBCMT:delete_scalar.obj"sv,   L"operator delete(void *)"sv, 0x8, }, },
		{ 0x0002f450, { L"sqlite.obj"sv,                 L"_sqlite3_step"sv, 0, }, },
		{ 0x000a1d8F, { L"sqlite.obj"sv,                 L"_sqlite3_step"sv, 0x7293f, }, },
		{ 0x000a1d90, { L"sqlite.obj"sv,                 L"_sqlite3_open"sv, 0, }, },
		{ 0x000a8670, { L"config.obj"sv,                 L"std::vector<struct column,class std::allocator<struct column> >::_Xlength(void)"sv, 0, }, },
		{ 0x000a8678, { L"config.obj"sv,                 L"std::vector<struct column,class std::allocator<struct column> >::_Xlength(void)"sv, 0x8, }, },
		{ 0x000ff9AF, { L"config.obj"sv,                 L"std::vector<struct column,class std::allocator<struct column> >::_Xlength(void)"sv, 0x5733f, }, },
		{ 0x000ff9b0, { L"configdb.obj"sv,               L"config_provider::config_provider(struct config_provider::clear_cache)"sv, 0, }, },
		{ 0x000ffc00, { L"configdb.obj"sv,               L"config_provider::~config_provider(void)"sv, 0, }, },
		{ 0xffffffff, { L"configdb.obj"sv,               L"config_provider::~config_provider(void)"sv, 0xfff003ff, }, },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Info == get_impl(i.Address, Symbols));
	}
}

TEST_CASE("map_file.clang")
{
	const auto MapFileData =
R"(Address  Size     Align Out     In      Symbol
00001550 0014512f    16         obj/sqlite.o:(.text)
000016c0 00000000     0                 sqlite3_mutex_enter
00001740 00000000     0                 sqlite3_mutex_leave
0014ebc0 0000002e    16         obj/cddrv.o:(.text$_ZNSt18bad_variant_accessD0Ev)
0014ebc0 00000000     0                 std::bad_variant_access::~bad_variant_access()
001515b0 00000019    16         obj/cddrv.o:(.text$_ZNSt14pointer_traitsIPwE10pointer_toERw)
001515b0 00000000     0                 std::pointer_traits<wchar_t*>::pointer_to(wchar_t&)
0064f310 00000ea4    16         obj/palette.o:(.text)
0064f420 00000000     0                 palette::palette()
0064f4e0 00000000     0                 palette::ResetToDefault()
)"sv;

	std::stringstream Stream(std::string{ MapFileData });

	REQUIRE(determine_format(Stream) == map_format::clang);

	unordered_string_set Files;
	std::map<uintptr_t, map_file::line> Symbols;

	read_clang(Stream, Files, Symbols);

	REQUIRE(Files.size() == 4u);
	REQUIRE(Symbols.size() == 6u);

	static const struct
	{
		uintptr_t Address;
		map_file::info Info;
	}
	Tests[]
	{
		{ 0x00000000, { {},                                                                {}, 0, }, },
		{ 0x000016c0, { L"obj/sqlite.o:(.text)"sv,                                         L"sqlite3_mutex_enter"sv, 0, }, },
		{ 0x000016c8, { L"obj/sqlite.o:(.text)"sv,                                         L"sqlite3_mutex_enter"sv, 0x8, }, },
		{ 0x00001740, { L"obj/sqlite.o:(.text)"sv,                                         L"sqlite3_mutex_leave"sv, 0, }, },
		{ 0x0014ebbf, { L"obj/sqlite.o:(.text)"sv,                                         L"sqlite3_mutex_leave"sv, 0x14d47f, }, },
		{ 0x0014ebc0, { L"obj/cddrv.o:(.text$_ZNSt18bad_variant_accessD0Ev)"sv,            L"std::bad_variant_access::~bad_variant_access()"sv, 0, }, },
		{ 0x001515b0, { L"obj/cddrv.o:(.text$_ZNSt14pointer_traitsIPwE10pointer_toERw)"sv, L"std::pointer_traits<wchar_t*>::pointer_to(wchar_t&)"sv, 0, }, },
		{ 0x0064f420, { L"obj/palette.o:(.text)"sv,                                        L"palette::palette()"sv, 0, }, },
		{ 0x0064f4e0, { L"obj/palette.o:(.text)"sv,                                        L"palette::ResetToDefault()"sv, 0, }, },
		{ 0xffffffff, { L"obj/palette.o:(.text)"sv,                                        L"palette::ResetToDefault()"sv, 0xff9b0b1f }, },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Info == get_impl(i.Address, Symbols));
	}
}

TEST_CASE("map_file.gcc")
{
	const auto MapFileData =
R"(
Release.64.gcc/Far.exe:     file format pei-x86-64

SYMBOL TABLE:
[  0](sec -2)(fl 0x00)(ty    0)(scl 103) (nx 1) 0x000000000000005f crtexe.c
File )" R"(
[  4](sec  1)(fl 0x00)(ty   20)(scl   3) (nx 0) 0x0000000000000010 pre_c_init
[ 56](sec  1)(fl 0x00)(ty   20)(scl   2) (nx 0) 0x00000000000004b0 WinMainCRTStartup
[106](sec -2)(fl 0x00)(ty    0)(scl 103) (nx 1) 0x00000000000006a4 sqlite.c
File )" R"(
[114](sec  1)(fl 0x00)(ty   20)(scl   2) (nx 0) 0x0000000000000580 sqlite3_mutex_enter
[115](sec  1)(fl 0x00)(ty   20)(scl   2) (nx 0) 0x00000000000005a0 sqlite3_mutex_leave
[2752](sec -2)(fl 0x00)(ty    0)(scl 103) (nx 1) 0x0000000000000c35 cmdline.cpp
File )" R"(
[2761](sec  1)(fl 0x00)(ty   20)(scl   2) (nx 0) 0x00000000006bdfa0 SimpleScreenObject::ShowConsoleTitle()
[2764](sec  1)(fl 0x00)(ty   20)(scl   2) (nx 0) 0x00000000006e44f0 Panel::ProcessPluginEvent(int, void*)
[31289](sec -2)(fl 0x00)(ty    0)(scl 103) (nx 1) 0x0000000000007b45 sqlitedb.cpp
File )" R"(
[31293](sec  1)(fl 0x00)(ty   20)(scl   3) (nx 0) 0x00000000005dc140 (anonymous namespace)::GetErrorString(int)
[31321](sec  1)(fl 0x00)(ty   20)(scl   3) (nx 0) 0x00000000005dcab0 (anonymous namespace)::scoped_object_148::{lambda()#1}::_FUN()
)"sv;

	std::stringstream Stream(std::string{ MapFileData });

	REQUIRE(determine_format(Stream) == map_format::gcc);

	unordered_string_set Files;
	std::map<uintptr_t, map_file::line> Symbols;

	read_gcc(Stream, Files, Symbols);

	REQUIRE(Files.size() == 4u);
	REQUIRE(Symbols.size() == 8u);

	static const struct
	{
		uintptr_t Address;
		map_file::info Info;
	}
	Tests[]
	{
		{ 0x00000000, { {},                {}, }, },
		{ 0x00001010, { L"crtexe.c"sv,     L"pre_c_init"sv, 0, }, },
		{ 0x000014b0, { L"crtexe.c"sv,     L"WinMainCRTStartup"sv, 0, }, },
		{ 0x000014b8, { L"crtexe.c"sv,     L"WinMainCRTStartup"sv, 0x8, }, },
		{ 0x0000157F, { L"crtexe.c"sv,     L"WinMainCRTStartup"sv, 0xcf, }, },
		{ 0x00001580, { L"sqlite.c"sv,     L"sqlite3_mutex_enter"sv, 0, }, },
		{ 0x000015a0, { L"sqlite.c"sv,     L"sqlite3_mutex_leave"sv, 0, }, },
		{ 0x005dd140, { L"sqlitedb.cpp"sv, L"(anonymous namespace)::GetErrorString(int)"sv, 0, }, },
		{ 0x005ddab0, { L"sqlitedb.cpp"sv, L"(anonymous namespace)::scoped_object_148::{lambda()#1}::_FUN()"sv, 0, }, },
		{ 0x006befa0, { L"cmdline.cpp"sv,  L"SimpleScreenObject::ShowConsoleTitle()"sv, 0, }, },
		{ 0x006e54f0, { L"cmdline.cpp"sv,  L"Panel::ProcessPluginEvent(int, void*)"sv, 0, }, },
		{ 0xffffffff, { L"cmdline.cpp"sv,  L"Panel::ProcessPluginEvent(int, void*)"sv, 0xff91ab0f, }, },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Info == get_impl(i.Address, Symbols));
	}
}
#endif
