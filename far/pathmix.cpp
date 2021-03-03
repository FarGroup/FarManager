﻿/*
pathmix.cpp

Misc functions for processing of path names
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
#include "pathmix.hpp"

// Internal:
#include "strmix.hpp"
#include "regex_helpers.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "filelist.hpp"
#include "plugin.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

void NTPath::Transform()
{
	string& Data = *this;
	if (!Data.empty())
	{
		if(!HasPathPrefix(Data))
		{
			Data = ConvertNameToFull(Data);

			if (!HasPathPrefix(Data))
			{
				ReplaceSlashToBackslash(Data);
				const auto Prefix = ParsePath(Data) == root_type::drive_letter? L"\\\\?\\"sv : L"\\\\?\\UNC"sv;
				remove_duplicates(Data, path::separator);
				Data.insert(0, Prefix);
			}
		}
		static const bool is_win2k = !IsWindowsXPOrGreater();
		if(is_win2k && Data.size() > 5 && Data[5] == L':')
		{
			// "\\?\C:" -> "\\?\c:"
			// Some file operations fail on Win2k if a drive letter is in upper case
			inplace::lower(Data, 4, 1);
		}
	}
}

string KernelPath(string_view const NtPath)
{
	return KernelPath(string(NtPath));
}

string KernelPath(string NtPath)
{
	if (HasPathPrefix(NtPath))
	{
		NtPath[1] = NtPath[2] = L'?';
	}
	return NtPath;
}


root_type ParsePath(const string_view Path, size_t* const RootSize, bool* const RootOnly)
{
	const auto re = [](const wchar_t* const Str) { return std::wregex(Str, std::regex::icase | std::regex::optimize); };

	static const struct
	{
		root_type Type;
		std::wregex re;
	}
	PathTypes[]
	{
#define RE_PATH_PREFIX(x) RE_C_GROUP(RE_BEGIN RE_BACKSLASH RE_NC_GROUP(RE_BACKSLASH RE_Q_MARK RE_OR RE_BACKSLASH RE_DOT RE_OR RE_Q_MARK RE_Q_MARK) RE_BACKSLASH x )

		{
			// x:(...)
			root_type::drive_letter,
			re(RE_C_GROUP(RE_BEGIN RE_ANY RE_ESCAPE(L":")) RE_NC_GROUP(RE_ANY_SLASH RE_ZERO_OR_ONE_GREEDY)),
		},
		{
			// \\?\x:(\...)
			root_type::win32nt_drive_letter,
			re(RE_PATH_PREFIX(L".\\:") RE_ANY_SLASH_OR_NONE),
		},
		{
			// \\server\share(\...)
			root_type::remote,
			re(RE_C_GROUP(RE_BEGIN RE_ANY_SLASH RE_REPEAT(2) RE_NONE_OF(RE_DOT) RE_NONE_OF(RE_SPACE RE_SLASHES RE_Q_MARK) RE_ONE_OR_MORE_LAZY RE_ANY_SLASH RE_ONE_OR_MORE_LAZY RE_NONE_OF(RE_SLASHES) RE_ONE_OR_MORE_GREEDY) RE_ANY_SLASH_OR_NONE),
		},
		{
			// \\?\unc\server\share(\...)
			root_type::unc_remote,
			re(RE_PATH_PREFIX(L"unc" RE_BACKSLASH RE_NONE_OF(RE_DOT) RE_NONE_OF(RE_SPACE RE_SLASHES RE_Q_MARK) RE_ONE_OR_MORE_LAZY RE_BACKSLASH RE_NONE_OF(RE_SLASHES) RE_ONE_OR_MORE_GREEDY) RE_ANY_SLASH_OR_NONE),
		},
		{
			// \\?\Volume{UUID}(\...)
			root_type::volume,
			re(RE_PATH_PREFIX(L"volume" RE_ESCAPE(L"{") RE_ANY_UUID RE_ESCAPE(L"}")) RE_ANY_SLASH_OR_NONE),
		},
		{
			// \\?\pipe(\...)
			root_type::pipe,
			re(RE_PATH_PREFIX(L"pipe") RE_ANY_SLASH_OR_NONE),
		},
		{
			// \\?\<anything_else>(\...)
			root_type::unknown_rootlike,
			re(RE_PATH_PREFIX(L"." RE_ONE_OR_MORE_LAZY) RE_ANY_SLASH_OR_NONE),
		}

#undef RE_PATH_REFIX
	};

	std::wcmatch Match;

	const auto ItemIterator = std::find_if(CONST_RANGE(PathTypes, i)
	{
		return std::regex_search(Path.data(), Path.data() + Path.size(), Match, i.re);
	});

	if (ItemIterator == std::cend(PathTypes))
		return root_type::unknown;

	const size_t MatchLength = Match[0].length();

	if (RootSize)
		*RootSize = MatchLength;

	if (RootOnly)
		*RootOnly = Path.size() == MatchLength || (Path.size() == MatchLength + 1 && path::is_separator(Path[MatchLength]));

	return ItemIterator->Type;
}

bool IsAbsolutePath(const string_view Path)
{
	const auto Type = ParsePath(Path);

	return
		(Type != root_type::unknown && Type != root_type::drive_letter) ||
		(Type == root_type::drive_letter && (Path.size() > 2 && path::is_separator(Path[2])));
}

bool HasPathPrefix(const string_view Path)
{
	return
		starts_with(Path, L"\\\\?\\"sv) ||
		starts_with(Path, L"\\\\.\\"sv) ||
		starts_with(Path, L"\\??\\"sv);
}

string_view ExtractPathPrefix(const string_view Path)
{
	if (starts_with_icase(Path, L"\\\\?\\UNC\\"sv))
		return Path.substr(0, 8);

	return HasPathPrefix(Path)? Path.substr(0, 4) : L""sv;
}

bool PathCanHoldRegularFile(string_view const Path)
{
	return ParsePath(Path) != root_type::unknown;
}

bool IsPluginPrefixPath(string_view const Path) //Max:
{
	if (Path.empty() || path::is_separator(Path[0]))
		return false;

	const auto pos = Path.find(L':');

	if (pos == string::npos || !pos)
		return false;

	if (pos == 1) // односимвольный префикс
	{
		if ((Path[0] >= L'a' && Path[0] <= L'z') || (Path[0] >= L'A' && Path[0] <= L'Z'))
			return false;

		string dev;
		if (os::fs::QueryDosDevice(Path.substr(0,2), dev))
			return false;
	}

	const auto SlashPos = FindSlash(Path);
	return SlashPos == string::npos || SlashPos > pos;
}

bool IsParentDirectory(string_view const Str)
{
	return DeleteEndSlash(Str) == L".."sv;
}

bool IsParentDirectory(const FileListItem& Data)
{
	return Data.UserFlags & PPIF_RESERVED && IsParentDirectory(static_cast<const os::fs::find_data&>(Data));
}

bool IsParentDirectory(const os::fs::find_data& Data)
{
	return
		Data.Attributes & FILE_ATTRIBUTE_DIRECTORY &&
		IsParentDirectory(Data.FileName) &&
		(!Data.HasAlternateFileName() || Data.AlternateFileName() == Data.FileName);
}

bool IsParentDirectory(const PluginPanelItem& Data)
{
	return
		// Plugins are unreliable and sometimes don't set this attribute
		// Data.FileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
		IsParentDirectory(NullToEmpty(Data.FileName)) &&
		(!Data.AlternateFileName || !*Data.AlternateFileName || equal(NullToEmpty(Data.AlternateFileName), NullToEmpty(Data.FileName)));
}

bool IsCurrentDirectory(string_view const Str)
{
	return starts_with(Str, L"."sv) && (Str.size() == 1 || (Str.size() == 2 && path::is_separator(Str[1])));
}

string_view PointToName(string_view const Path)
{
	const auto NameStart = std::find_if(ALL_CONST_REVERSE_RANGE(Path), path::is_separator);
	return Path.substr(Path.crend() - NameStart);
}

//   Аналог PointToName, только для строк типа
//   "name\" (оканчивается на слеш) возвращает указатель на name, а не на пустую
//   строку
string_view PointToFolderNameIfFolder(string_view Path)
{
	while(!Path.empty() && path::is_separator(Path.back()))
		Path.remove_suffix(1);

	return PointToName(Path);
}

std::pair<string_view, string_view> name_ext(string_view const Path)
{
	auto ExtensionStart = std::find_if(ALL_CONST_REVERSE_RANGE(Path), [](wchar_t const Char){ return Char == L'.' || path::is_separator(Char); });
	if (ExtensionStart != Path.crend() && *ExtensionStart != L'.')
		ExtensionStart = Path.crend();

	const auto NameSize = ExtensionStart == Path.crend()? Path.size() : Path.crend() - ExtensionStart - 1;

	return { Path.substr(0, NameSize), Path.substr(NameSize) };
}


static size_t SlashType(const wchar_t* Begin, const wchar_t* End, wchar_t &TypeSlash)
{
	size_t Slash = 0, BackSlash = 0;

	auto Iterator = Begin;
	for (; End? Iterator != End : *Iterator; ++Iterator)
	{
		const auto c = *Iterator;
		BackSlash += (c == L'\\');
		Slash += (c == L'/');
	}

	TypeSlash = Slash > BackSlash? L'/' : L'\\';
	return Iterator - Begin;
}

// Функция работает с обоими видами слешей, также происходит
// изменение уже существующего конечного слеша на такой, который
// указан, или встречается чаще (при равенстве '\').
//
bool AddEndSlash(wchar_t *Path, wchar_t TypeSlash)
{
	if (!Path)
		return false;

	auto len = path::is_separator(TypeSlash)? wcslen(Path) : SlashType(Path, nullptr, TypeSlash);

	if (len && path::is_separator(Path[len-1]))
		--len;

	Path[len++] = TypeSlash;
	Path[len] = {};
	return true;
}

bool AddEndSlash(wchar_t *Path)
{
	return AddEndSlash(Path, {});
}

string AddEndSlash(string_view const Path)
{
	string Result(Path);
	AddEndSlash(Result);
	return Result;
}

void AddEndSlash(string &strPath, wchar_t TypeSlash)
{
	if (!path::is_separator(TypeSlash))
		SlashType(strPath.data(), strPath.data() + strPath.size(), TypeSlash);

	wchar_t LastSlash{};

	if (!strPath.empty() && path::is_separator(strPath.back()))
		LastSlash = strPath.back();

	if (TypeSlash != LastSlash)
	{
		if (LastSlash)
			strPath.back() = TypeSlash;
		else
			strPath.push_back(TypeSlash);
	}
}

void AddEndSlash(string &strPath)
{
	AddEndSlash(strPath, {});
}

void DeleteEndSlash(wchar_t *Path)
{
	const auto REnd = std::make_reverse_iterator(Path);
	Path[REnd - std::find_if_not(REnd - wcslen(Path), REnd, path::is_separator)] = 0;
}

void DeleteEndSlash(string &Path)
{
	Path.resize(Path.rend() - std::find_if_not(Path.rbegin(), Path.rend(), path::is_separator));
}

string_view DeleteEndSlash(string_view Path)
{
	Path.remove_suffix(std::find_if_not(Path.rbegin(), Path.rend(), path::is_separator) - Path.rbegin());
	return Path;
}

bool CutToSlash(string_view& Str, bool const RemoveSlash)
{
	const auto pos = FindLastSlash(Str);
	if (pos == string::npos)
		return false;

	if (pos == 3 && HasPathPrefix(Str))
		return false;

	Str.remove_suffix(Str.size() - pos - (RemoveSlash? 0 : 1));

	return true;
}

bool CutToSlash(string& Str, bool const RemoveSlash)
{
	string_view View(Str);
	if (!CutToSlash(View, RemoveSlash))
		return false;

	Str.resize(View.size());
	return true;
}

bool CutToParent(string_view& Str)
{
	if (Str.empty())
		return false;

	size_t RootLength = 0;
	ParsePath(Str, &RootLength);

	if (RootLength == Str.size())
		return false;

	const auto REnd = Str.crend() - RootLength;
	const auto LastNotSlash = std::find_if_not(Str.crbegin(), REnd, path::is_separator);
	const auto PrevSlash = std::find_if(LastNotSlash, REnd, path::is_separator);
	const auto PrevNotSlash = std::find_if_not(PrevSlash, REnd, path::is_separator);

	const auto NewSize = RootLength + REnd - PrevNotSlash;
	if (!NewSize)
		return false;

	Str.remove_suffix(Str.size() - NewSize);
	return true;
}

bool CutToParent(string& Str)
{
	string_view View(Str);
	if (!CutToParent(View))
		return false;

	Str.resize(View.size());
	return true;
}

bool ContainsSlash(const string_view Str)
{
	return FindSlash(Str) != string::npos;
}

size_t FindSlash(const string_view Str)
{
	const auto SlashPos = std::find_if(ALL_CONST_RANGE(Str), path::is_separator);
	return SlashPos == Str.cend()? string::npos : SlashPos - Str.cbegin();
}

size_t FindLastSlash(const string_view Str)
{
	const auto SlashPos = std::find_if(ALL_CONST_REVERSE_RANGE(Str), path::is_separator);
	return SlashPos == Str.crend()? string::npos : Str.crend() - SlashPos - 1;
}

// find path root component (drive letter / volume name / server share) and calculate its length
static size_t GetPathRootLength(string_view const Path)
{
	size_t DirOffset = 0;
	return ParsePath(Path, &DirOffset) == root_type::unknown? 0 : DirOffset;
}

string_view extract_root_device(string_view const Path)
{
	const auto RootSize = GetPathRootLength(Path);
	if (!RootSize)
		return{};

	return Path.substr(0, RootSize - (path::is_separator(Path[RootSize - 1])? 1 : 0));
}

string extract_root_directory(string_view const Path)
{
	const auto RootSize = GetPathRootLength(Path);
	if (!RootSize)
		return{};

	if (path::is_separator(Path[RootSize - 1]))
		return string{ Path.substr(0, RootSize) };

	// A fancy way to add a trailing slash
	return path::join(Path.substr(0, RootSize), L""sv);
}

string_view ExtractFileName(string_view const Path)
{
	auto p = FindLastSlash(Path);
	p = p == string::npos? 0 : p + 1;
	p = std::max(p, GetPathRootLength(Path));
	return Path.substr(p);
}

string ExtractFilePath(string_view const Path)
{
	auto p = FindLastSlash(Path);
	if (p == string::npos)
	{
		p = 0;
	}

	const auto PathRootLen = GetPathRootLength(Path);

	if (p <= PathRootLen && PathRootLen)
		return path::join(Path.substr(0, PathRootLen), L""sv);

	return string(Path.substr(0, p));
}

bool IsRootPath(const string_view Path)
{
	bool IsRoot = false;
	ParsePath(Path, nullptr, &IsRoot);
	return IsRoot || IsRelativeRoot(Path);
}

bool PathStartsWith(const string_view Path, const string_view Start)
{
	const auto PathPart = DeleteEndSlash(Start);
	return starts_with(Path, PathPart) && (Path.size() == PathPart.size() || path::is_separator(Path[PathPart.size()]));
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("path.join")
{
	REQUIRE(path::join(L"foo"sv, L""sv)                         == L"foo\\"sv);
	REQUIRE(path::join(L"foo"sv, L"\\"sv)                       == L"foo\\"sv);
	REQUIRE(path::join(L""sv, L""sv)                            == L""sv);
	REQUIRE(path::join(L""sv, L"\\"sv)                          == L""sv);
	REQUIRE(path::join(L""sv, L"foo"sv)                         == L"foo"sv);
	REQUIRE(path::join(L"\\foo"sv, L""sv)                       == L"\\foo\\"sv);
	REQUIRE(path::join(L"\\foo"sv, L"\\"sv)                     == L"\\foo\\"sv);
	REQUIRE(path::join(L"\\"sv, L"foo\\"sv)                     == L"foo"sv);
	REQUIRE(path::join(L"foo"sv, L"bar"sv)                      == L"foo\\bar"sv);
	REQUIRE(path::join(L"\\foo"sv, L"bar\\"sv)                  == L"\\foo\\bar"sv);
	REQUIRE(path::join(L"foo\\"sv, L"bar"sv)                    == L"foo\\bar"sv);
	REQUIRE(path::join(L"foo\\"sv, L"\\bar"sv)                  == L"foo\\bar"sv);
	REQUIRE(path::join(L"foo\\"sv, L'\\', L"\\bar"sv)           == L"foo\\bar"sv);
	REQUIRE(path::join(L"foo\\"sv, L""sv, L"\\bar"sv)           == L"foo\\bar"sv);
	REQUIRE(path::join(L"\\\\foo\\\\"sv, L"\\\\bar\\"sv)        == L"\\\\foo\\bar"sv);
}

TEST_CASE("path.extract_root")
{
	static const struct
	{
		string_view Input, RootDevice, RootDirectory;
	}
	Tests[]
	{
		{{},                                                                       {}},
		{L"\\"sv,                                                                  {}},
		{L"file"sv,                                                                {}},
		{L"path\\file"sv,                                                          {}},
		{L"C:"sv,                                                                  L"C:"sv},
		{L"C:\\"sv,                                                                L"C:"sv},
		{L"C:\\path\\file"sv,                                                      L"C:"sv},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}"sv,                 L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}"sv},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\"sv,               L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}"sv},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path\\file"sv,     L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}"sv},
		{L"\\\\server\\share"sv,                                                   L"\\\\server\\share"sv},
		{L"\\\\server\\share\\"sv,                                                 L"\\\\server\\share"sv},
		{L"\\\\server\\share\\path\\file"sv,                                       L"\\\\server\\share"sv},
		{L"\\\\1.2.3.4\\share\\path\\file"sv,                                      L"\\\\1.2.3.4\\share"sv},
		{L"\\\\?\\UNC\\server\\share"sv,                                           L"\\\\?\\UNC\\server\\share"sv},
		{L"\\\\?\\UNC\\server\\share\\"sv,                                         L"\\\\?\\UNC\\server\\share"sv},
		{L"\\\\?\\UNC\\server\\share\\path\\file"sv,                               L"\\\\?\\UNC\\server\\share"sv},
		{L"\\\\?\\UNC\\1.2.3.4\\share\\path\\file"sv,                              L"\\\\?\\UNC\\1.2.3.4\\share"sv},
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.RootDevice == extract_root_device(i.Input));

		const auto RootDirectory = extract_root_directory(i.Input);
		if (i.RootDevice.empty())
			REQUIRE(RootDirectory.empty());
		else
			REQUIRE(RootDirectory == i.RootDevice + L"\\"sv);
	}
}

TEST_CASE("path.ExtractFilePath")
{
	static const struct
	{
		string_view Input, Result;
	}
	Tests[]
	{
		{{},                                                                       {}},
		{L"\\"sv,                                                                  {}},
		{L"\\file"sv,                                                              {}},
		{L"file"sv,                                                                {}},
		{L"path\\"sv,                                                              L"path"sv},
		{L"path\\file"sv,                                                          L"path"sv},
		{L"C:"sv,                                                                  L"C:\\"sv},
		{L"C:\\"sv,                                                                L"C:\\"sv},
		{L"C:\\file"sv,                                                            L"C:\\"sv},
		{L"C:\\path\\file"sv,                                                      L"C:\\path"sv},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}"sv,                 L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\"sv},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\"sv,               L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\"sv},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\file"sv,           L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\"sv},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path\\file"sv,     L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path"sv},
		{L"\\\\server\\share"sv,                                                   L"\\\\server\\share\\"sv},
		{L"\\\\server\\share\\"sv,                                                 L"\\\\server\\share\\"sv},
		{L"\\\\server\\share\\file"sv,                                             L"\\\\server\\share\\"sv},
		{L"\\\\server\\share\\path\\file"sv,                                       L"\\\\server\\share\\path"sv},
		{L"\\\\?\\UNC\\server\\share"sv,                                           L"\\\\?\\UNC\\server\\share\\"sv},
		{L"\\\\?\\UNC\\server\\share\\"sv,                                         L"\\\\?\\UNC\\server\\share\\"sv},
		{L"\\\\?\\UNC\\server\\share\\file"sv,                                     L"\\\\?\\UNC\\server\\share\\"sv},
		{L"\\\\?\\UNC\\server\\share\\path\\file"sv,                               L"\\\\?\\UNC\\server\\share\\path"sv},
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Result == ExtractFilePath(i.Input));
	}
}

TEST_CASE("path.ExtractFileName")
{
	static const struct
	{
		string_view Input, Result;
	}
	Tests[]
	{
		{{},                                                                       {}},
		{L"\\"sv,                                                                  {}},
		{L"\\file"sv,                                                              L"file"sv},
		{L"file"sv,                                                                L"file"sv},
		{L"path\\"sv,                                                              {}},
		{L"path\\file"sv,                                                          L"file"sv},
		{L"C:"sv,                                                                  {}},
		{L"C:\\"sv,                                                                {}},
		{L"C:\\file"sv,                                                            L"file"sv},
		{L"C:\\path\\file"sv,                                                      L"file"sv},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}"sv,                 {}},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\"sv,               {}},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\file"sv,           L"file"sv},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path\\file"sv,     L"file"sv},
		{L"\\\\server\\share"sv,                                                   {}},
		{L"\\\\server\\share\\"sv,                                                 {}},
		{L"\\\\server\\share\\file"sv,                                             L"file"sv},
		{L"\\\\server\\share\\path\\file"sv,                                       L"file"sv},
		{L"\\\\?\\UNC\\server\\share"sv,                                           {}},
		{L"\\\\?\\UNC\\server\\share\\"sv,                                         {}},
		{L"\\\\?\\UNC\\server\\share\\file"sv,                                     L"file"sv},
		{L"\\\\?\\UNC\\server\\share\\path\\file"sv,                               L"file"sv},
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Result == ExtractFileName(i.Input));
	}
}

TEST_CASE("path.PointToName")
{
	static const struct
	{
		string_view Input, Result;
	}
	Tests[]
	{
		{{},                                                                       {}},
		{L"\\"sv,                                                                  {}},
		{L"\\file"sv,                                                              L"file"sv},
		{L"file"sv,                                                                L"file"sv},
		{L"path\\"sv,                                                              {}},
		{L"path\\file"sv,                                                          L"file"sv},
		// {L"C:"sv,                                                                  {}},
		{L"C:\\"sv,                                                                {}},
		{L"C:\\file"sv,                                                            L"file"sv},
		{L"C:\\path\\file"sv,                                                      L"file"sv},
		// {L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}"sv,                 {}},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\"sv,               {}},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\file"sv,           L"file"sv},
		{L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path\\file"sv,     L"file"sv},
		// {L"\\\\server\\share"sv,                                                   {}},
		{L"\\\\server\\share\\"sv,                                                 {}},
		{L"\\\\server\\share\\file"sv,                                             L"file"sv},
		{L"\\\\server\\share\\path\\file"sv,                                       L"file"sv},
		// {L"\\\\?\\UNC\\server\\share"sv,                                           {}},
		{L"\\\\?\\UNC\\server\\share\\"sv,                                         {}},
		{L"\\\\?\\UNC\\server\\share\\file"sv,                                     L"file"sv},
		{L"\\\\?\\UNC\\server\\share\\path\\file"sv,                               L"file"sv},
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Result == PointToName(i.Input));
	}
}

TEST_CASE("path.name_ext")
{
	static const struct
	{
		string_view Input, Name, Extension;
	}
	Tests[]
	{
		{ {},                                 {},                        {}         },
		{ L"file"sv,                          L"file"sv,                 {}         },
		{ L"path\\file"sv,                    L"path\\file"sv,           {}         },
		{ L"path.ext\\file"sv,                L"path.ext\\file"sv,       {}         },
		{ L".\\"sv,                           L".\\"sv,                  {}         },
		{ L"\\."sv,                           L"\\"sv,                   L"."sv     },
		{ L"."sv,                             {},                        L"."sv     },
		{ L".."sv,                            L"."sv,                    L"."sv     },
		{ L"..."sv,                           L".."sv,                   L"."sv     },
		{ L"file."sv,                         L"file"sv,                 L"."sv     },
		{ L".e"sv,                            {},                        L".e"sv    },
		{ L".ext"sv,                          {},                        L".ext"sv  },
		{ L"..ext"sv,                         L"."sv,                    L".ext"sv  },
		{ L"file.ext"sv,                      L"file"sv,                 L".ext"sv  },
		{ L"path\\file.ext"sv,                L"path\\file"sv,           L".ext"sv  },
		{ L"file.ext1.ext2"sv,                L"file.ext1"sv,            L".ext2"sv },
		{ L"path\\file.ext1.ext2"sv,          L"path\\file.ext1"sv,      L".ext2"sv },
	};

	for (const auto& i: Tests)
	{
		const auto& [Name, Extension] = name_ext(i.Input);
		REQUIRE(i.Name == Name);
		REQUIRE(i.Extension == Extension);
	}
}

TEST_CASE("path.ParsePath")
{
	static const struct
	{
		string_view Str;
		root_type Type;
		size_t DirOffset;
		bool Root;
	}
	Tests[]
	{
		{ {},                                                              root_type::unknown,                 0,   false, },
		{ L"1"sv,                                                          root_type::unknown,                 0,   false, },
		{ L"\\"sv,                                                         root_type::unknown,                 0,   false, },
		{ L"path\\file"sv,                                                 root_type::unknown,                 0,   false, },
		{ L"A:"sv,                                                         root_type::drive_letter,            2,   true,  },
		{ L"A:path"sv,                                                     root_type::drive_letter,            2,   false, },
		{ L"B:\\"sv,                                                       root_type::drive_letter,            3,   true,  },
		{ L"C:\\path"sv,                                                   root_type::drive_letter,            3,   false, },
		{ L"CC:\\path"sv,                                                  root_type::unknown,                 0,   false, },
		{ L"\\\\?\\A:"sv,                                                  root_type::win32nt_drive_letter,    6,   true,  },
		{ L"\\\\?\\B:\\"sv,                                                root_type::win32nt_drive_letter,    7,   true,  },
		{ L"\\\\?\\C:\\path"sv,                                            root_type::win32nt_drive_letter,    7,   false, },
		{ L"\\\\?\\CC:\\path"sv,                                           root_type::unknown_rootlike,        8,   false, },
		{ L"\\\\.\\A:"sv,                                                  root_type::win32nt_drive_letter,    6,   true,  },
		{ L"\\\\.\\B:\\"sv,                                                root_type::win32nt_drive_letter,    7,   true,  },
		{ L"\\\\.\\C:\\path"sv,                                            root_type::win32nt_drive_letter,    7,   false, },
		{ L"\\\\.\\CC:\\path"sv,                                           root_type::unknown_rootlike,        8,   false, },
		{ L"\\??\\A:"sv,                                                   root_type::win32nt_drive_letter,    6,   true,  },
		{ L"\\??\\B:\\"sv,                                                 root_type::win32nt_drive_letter,    7,   true,  },
		{ L"\\??\\C:\\path"sv,                                             root_type::win32nt_drive_letter,    7,   false, },
		{ L"\\??\\CC:\\path"sv,                                            root_type::unknown_rootlike,        8,   false, },
		{ L"\\\\server\\share"sv,                                          root_type::remote,                 14,   true,  },
		{ L"\\\\server\\share\\"sv,                                        root_type::remote,                 15,   true,  },
		{ L"\\\\server\\share\\path"sv,                                    root_type::remote,                 15,   false, },
		{ L"\\\\server"sv,                                                 root_type::unknown,                 0,   false, },
		{ L"\\\\?\\UNC\\server\\share"sv,                                  root_type::unc_remote,             20,   true,  },
		{ L"\\\\?\\UNC\\server\\share\\"sv,                                root_type::unc_remote,             21,   true,  },
		{ L"\\\\?\\UNC\\server\\share\\path"sv,                            root_type::unc_remote,             21,   false, },
		{ L"\\\\?\\Volume{01234567-89AB-CDEF-0123-456789ABCDEF}"sv,        root_type::volume,                 48,   true,  },
		{ L"\\\\?\\Volume{01234567-89AB-CDEF-0123-456789ABCDEF}\\"sv,      root_type::volume,                 49,   true,  },
		{ L"\\\\?\\Volume{01234567-89AB-CDEF-0123-456789ABCDEF}\\path"sv,  root_type::volume,                 49,   false, },
		{ L"\\\\?\\Volume{01234567-89AB-CDEF-0123-456789ABCDEZ}\\path"sv,  root_type::unknown_rootlike,       49,   false, },
		{ L"\\\\?\\Volume{01234567-89AB-CDEF-0123-456789ABCDEF}_\\"sv,     root_type::unknown_rootlike,       50,   true,  },
		{ L"\\\\?\\pipe"sv,                                                root_type::pipe,                    8,   true,  },
		{ L"\\\\?\\pipe\\"sv,                                              root_type::pipe,                    9,   true,  },
		{ L"\\\\?\\pipe\\path"sv,                                          root_type::pipe,                    9,   false, },
		{ L"\\\\?\\pepe\\path"sv,                                          root_type::unknown_rootlike,        9,   false, },
		{ L"\\\\?\\pipe_\\"sv,                                             root_type::unknown_rootlike,       10,   true,  },
		{ L"\\\\?\\storage#volume#_??_usbstor#disk&ven_usb&prod_flash_disk&rev_1100#6&295c6d19&0#{53f56307-b6bf-11d0-94f2-00a0c91efb8b}#{53f5630d-b6bf-11d0-94f2-00a0c91efb8b}\\"sv, root_type::unknown_rootlike, 160, true, },
	};

	for (const auto& i: Tests)
	{
		size_t DirOffset{};
		bool Root{};
		REQUIRE(ParsePath(i.Str, &DirOffset, &Root) == i.Type);
		REQUIRE(DirOffset == i.DirOffset);
		REQUIRE(Root == i.Root);
	}
}

TEST_CASE("path.PathStartsWith")
{
	static const struct
	{
		bool Result;
		string_view Path, Prefix;
	}
	Tests[]
	{
		{ true,  {},                    {}                },
		{ false, {},                    L"Q"sv            },
		{ true,  L"\\"sv,               {}                },
		{ false, L"C:\\path\\file"sv,   {}                },
		{ true,  L"C:\\path\\file"sv,   L"C:\\path"sv     },
		{ true,  L"C:\\path\\file"sv,   L"C:\\path\\"sv   },
		{ false, L"C:\\path\\file"sv,   L"C:\\pat"sv      },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Result == PathStartsWith(i.Path, i.Prefix));
	}
}

TEST_CASE("path.CutToParent")
{
	static const string_view TestRoots[]
	{
		{},
		L"C:"sv,
		L"C:\\"sv,
		L"\\\\server\\share\\"sv,
		L"\\\\?\\C:\\"sv,
		L"\\\\?\\UNC\\server\\share\\"sv,
		L"\\\\?\\Volume{f26b206c-f912-11e1-b516-806e6f6e6963}\\"sv,
		L"\\\\?\\pipe\\"sv,
		L"\\\\?\\pineapple#pizza\\"sv,
	};

	static const struct
	{
		const string_view InputPath;
		const string_view ExpectedPath;
		bool RootMustExist;
		bool ExpectedResult;
	}
	TestCases[]
	{
		// root directory, shall fail
		{ L"{}"sv,                  L"{}"sv,        false, false},

		// one level, shall return root directory
		{ L"{}dir1"sv,              L"{}"sv,        true,  true },

		// one level without root, shall fail
		{ L"dir1"sv,                L"dir1"sv,      false, false },

		// two levels, shall return first level
		{ L"{}dir1\\dir2"sv,        L"{}dir1"sv,    false, true },

		// two levels with trailing slash, shall return first level
		{ L"{}dir1\\dir2\\"sv,      L"{}dir1"sv,    false, true },
	};

	for (const auto& Root: TestRoots)
	{
		for (const auto& Test: TestCases)
		{
			if (Root.empty() && Test.RootMustExist)
				continue;

			const auto Baseline = format(Test.ExpectedPath, Root);
			auto Path = format(Test.InputPath, Root);

			REQUIRE(Test.ExpectedResult == CutToParent(Path));
			REQUIRE(Baseline == Path);
		}
	}
}

TEST_CASE("path.AddEndSlash")
{
	static const struct
	{
		string_view Input, Result;
	}
	Tests[]
	{
		{ {},              L"\\"sv },
		{ L"\\"sv,         L"\\"sv },
		{ L"/"sv,          L"/"sv },
		{ L"a"sv,          L"a\\"sv },
		{ L"a\\"sv,        L"a\\"sv },
		{ L"a\\\\"sv,      L"a\\\\"sv },
		{ L"a\\b/"sv,      L"a\\b\\"sv },
		{ L"a\\b/c/d"sv,   L"a\\b/c/d/"sv },
	};

	for (const auto& i: Tests)
	{
		string Str(i.Input);
		AddEndSlash(Str);
		REQUIRE(Str == i.Result);

		wchar_t Buffer[64];
		REQUIRE(i.Input.size() < std::size(Buffer));
		*copy_string(i.Input, Buffer) = {};
		AddEndSlash(Buffer);
		REQUIRE(Buffer == i.Result);
	}
}

TEST_CASE("path.DeleteEndSlash")
{
	static const struct
	{
		string_view Input, Result;
	}
	Tests[]
	{
		{ {},              {} },
		{ L"\\"sv,         {} },
		{ L"/"sv,          {} },
		{ L"a"sv,          L"a"sv },
		{ L"a\\"sv,        L"a"sv },
		{ L"a\\\\"sv,      L"a"sv },
		{ L"a\\b/"sv,      L"a\\b"sv },
		{ L"a\\b/c/d"sv,   L"a\\b/c/d"sv },
	};

	for (const auto& i : Tests)
	{
		string Str(i.Input);
		DeleteEndSlash(Str);
		REQUIRE(Str == i.Result);

		wchar_t Buffer[64];
		REQUIRE(i.Input.size() < std::size(Buffer));
		*copy_string(i.Input, Buffer) = {};
		DeleteEndSlash(Buffer);
		REQUIRE(Buffer == i.Result);
	}
}
#endif
