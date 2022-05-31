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
#include "global.hpp"
#include "imports.hpp"
#include "pathmix.hpp"
#include "RegExp.hpp"

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
	string_view BaseName = ModuleName;
	const auto Ext = name_ext(BaseName).second;
	BaseName.remove_suffix(Ext.size());
	return BaseName + L".map"sv;
}

map_file::map_file(string_view const ModuleName)
{
	os::fs::file const Handle(get_map_name(ModuleName), FILE_READ_DATA, FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING);
	if (!Handle)
		return;

	try
	{
		os::fs::filebuf StreamBuffer(Handle, std::ios::in);
		std::istream Stream(&StreamBuffer);
		Stream.exceptions(Stream.badbit | Stream.failbit);

		read(Stream);
	}
	catch (std::exception const&)
	{
		m_Symbols.clear();
		m_Files.clear();
	}
}

map_file::~map_file() = default;

static string undecorate(string const& Symbol)
{
	const auto UndecorateFlags =
		UNDNAME_NO_FUNCTION_RETURNS |
		UNDNAME_NO_ALLOCATION_MODEL |
		UNDNAME_NO_ALLOCATION_LANGUAGE |
		UNDNAME_NO_ACCESS_SPECIFIERS |
		UNDNAME_NO_MEMBER_TYPE |
		UNDNAME_32_BIT_DECODE;

	if (wchar_t Buffer[MAX_SYM_NAME]; imports.UnDecorateSymbolNameW && imports.UnDecorateSymbolNameW(Symbol.c_str(), Buffer, static_cast<DWORD>(std::size(Buffer)), UndecorateFlags))
		return Buffer;

	if (char Buffer[MAX_SYM_NAME]; imports.UnDecorateSymbolName && imports.UnDecorateSymbolName(encoding::ansi::get_bytes(Symbol).c_str(), Buffer, static_cast<DWORD>(std::size(Buffer)), UndecorateFlags))
		return encoding::ansi::get_chars(Buffer);

	return Symbol;
}

std::pair<string, string_view> map_file::get(uintptr_t const Address) const
{
	auto [Begin, End] = m_Symbols.equal_range(Address);
	if (Begin == m_Symbols.cend())
		return {};

	if (Begin->first > Address)
		--Begin;

	return { format(FSTR(L"{:04X}+{}"sv), Address - Begin->first, undecorate(Begin->second.Name)), *Begin->second.File};
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
	if (starts_with(Str, L' '))
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

static auto group(string_view const Str, std::vector<RegExpMatch> const& m, size_t const Index)
{
	const auto& Match = m[Index];
	return Str.substr(Match.start, Match.end - Match.start);
}

static void read_vc(std::istream& Stream, unordered_string_set& Files, std::map<uintptr_t, map_file::line>& Symbols)
{
	RegExp ReBase, ReSymbol;
	ReBase.Compile(L"^ +Preferred load address is ([0-9A-Fa-f]+)$"sv, OP_OPTIMIZE);
	ReSymbol.Compile(L"^ +([0-9A-Fa-f]+):([0-9A-Fa-f]+) +([^ ]+) +([0-9A-Fa-f]+) .+ ([^ ]+)$"sv, OP_OPTIMIZE);
	std::vector<RegExpMatch> m;

	uintptr_t BaseAddress{};

	for (const auto& i: enum_lines(Stream, CP_UTF8))
	{
		if (i.Str.empty())
			continue;

		if (!BaseAddress && ReBase.Search(i.Str, m))
		{
			BaseAddress = from_string<uintptr_t>(group(i.Str, m, 1), {}, 16);
			continue;
		}

		if (ReSymbol.Search(i.Str, m))
		{
			auto Address = from_string<uintptr_t>(group(i.Str, m, 4), {}, 16);
			if (!Address)
				continue;

			if (Address >= BaseAddress)
				Address -= BaseAddress;

			map_file::line Line;
			Line.Name = group(i.Str, m, 3);
			if (m.size() > 5)
			{
				const auto File = group(i.Str, m, 5);
				Line.File = &*Files.emplace(File).first;
			}

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
	std::vector<RegExpMatch> m;

	string ObjName;

	for (const auto& i: enum_lines(Stream, CP_UTF8))
	{
		if (i.Str.empty())
			continue;

		if (ReSymbol.Search(i.Str, m))
		{
			map_file::line Line;
			Line.Name = group(i.Str, m, 2);
			Line.File = &*Files.emplace(ObjName).first;
			const auto Address = from_string<uintptr_t>(group(i.Str, m, 1), {}, 16);
			Symbols.emplace(Address, std::move(Line));
			continue;
		}

		if (ReObject.Search(i.Str, m))
		{
			ObjName = group(i.Str, m, 1);
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
	std::vector<RegExpMatch> m;

	const auto BaseAddress = 0x1000;

	string LastLine, FileName;

	for (const auto& i: enum_lines(Stream, CP_UTF8))
	{
		if (i.Str.empty())
			continue;

		if (ReFile.Search(i.Str, m) && ReFileName.Search(LastLine, m))
		{
			FileName = group(LastLine, m, 1);
			LastLine.clear();
			continue;
		}

		if (ReSymbol.Search(i.Str, m))
		{
			map_file::line Line;
			Line.Name = group(i.Str, m, 2);
			Line.File = &*Files.emplace(FileName).first;
			const auto Address = from_string<uintptr_t>(group(i.Str, m, 1), {}, 16) + BaseAddress;
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
