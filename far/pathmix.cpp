/*
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

#include "pathmix.hpp"

#include "strmix.hpp"
#include "regex_helpers.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "filelist.hpp"
#include "plugin.hpp"

#include "platform.fs.hpp"

#include "format.hpp"

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
				while (ReplaceStrings(Data, L"\\\\"sv, L"\\"sv))
					;
				Data.insert(0, Prefix.data(), Prefix.size());
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

string KernelPath(const string& NtPath)
{
	return KernelPath(string(NtPath));
}

string KernelPath(string&& NtPath)
{
	if (NtPath.size() > 1 && NtPath[1] == L'\\')
	{
		NtPath[1] = L'?';
	}
	return NtPath;
}


root_type ParsePath(const string_view Path, size_t* const DirectoryOffset, bool* const Root)
{
	auto Result = root_type::unknown;

	static struct
	{
		root_type Type;
		const wchar_t* REStr;
		std::wregex re;
	}
	PathTypes[] =
	{
		// TODO: tests for all these types

#define RE_PATH_PREFIX(x) RE_C_GROUP(RE_BEGIN RE_BACKSLASH RE_REPEAT(2) RE_ANY_OF(RE_Q_MARK RE_DOT) RE_BACKSLASH x )

		// x:<whatever> or x:\\<whatever>
		{ root_type::drive_letter, RE_C_GROUP(RE_BEGIN RE_ANY RE_ESCAPE(L":")) RE_NC_GROUP(RE_ANY_SLASH RE_ZERO_OR_ONE_GREEDY) },
		// \\?\x: or \\?\x:\ or \\?\x:\<whatever>
		{ root_type::unc_drive_letter, RE_PATH_PREFIX(L".\\:") RE_ANY_SLASH_OR_NONE },
		// \\server\share or \\server\share\ or \\server\share<whatever>
		{ root_type::remote, RE_C_GROUP(RE_BEGIN RE_ANY_SLASH RE_REPEAT(2) RE_NONE_OF(RE_SPACE RE_SLASHES RE_Q_MARK) RE_ONE_OR_MORE_LAZY RE_ANY_SLASH RE_ONE_OR_MORE_LAZY RE_NONE_OF(RE_SLASHES) RE_ONE_OR_MORE_GREEDY) RE_ANY_SLASH_OR_NONE },
		// \\?\unc\server\share or \\?\unc\server\share\ or \\?\unc\server\share<whatever>
		{ root_type::unc_remote, RE_PATH_PREFIX(L"unc" RE_BACKSLASH RE_NONE_OF(RE_SPACE RE_SLASHES RE_Q_MARK) RE_ONE_OR_MORE_LAZY RE_BACKSLASH RE_NONE_OF(RE_SLASHES) RE_ONE_OR_MORE_GREEDY) RE_ANY_SLASH_OR_NONE },
		// \\?\Volume{GUID} or \\?\Volume{GUID}\ or \\?\Volume{GUID}<whatever>
		{ root_type::volume, RE_PATH_PREFIX(L"volume" RE_ESCAPE(L"{") RE_ANY_UUID RE_ESCAPE(L"}")) RE_ANY_SLASH_OR_NONE },
		// \\?\pipe\ or \\?\pipe
		{ root_type::pipe, RE_PATH_PREFIX(L"pipe") RE_ANY_SLASH_OR_NONE },

#undef RE_PATH_REFIX
	};
	static bool REInit = false;
	if(!REInit)
	{
		std::for_each(RANGE(PathTypes, i)
		{
			i.re.assign(i.REStr, std::regex::icase | std::regex::optimize);
		});
		REInit = true;
	}

	std::wcmatch Match;

	const auto ItemIterator = std::find_if(CONST_RANGE(PathTypes, i) { return std::regex_search(Path.data(), Path.data() + Path.size(), Match, i.re); });

	if (ItemIterator != std::cend(PathTypes))
	{
		const size_t MatchLength = Match[0].length();
		if (DirectoryOffset)
		{
			*DirectoryOffset = MatchLength;
		}
		if (Root)
		{
			*Root = Path.size() == MatchLength || (Path.size() == (MatchLength + 1) && IsSlash(Path[MatchLength]));
		}
		Result = ItemIterator->Type;
	}

	return Result;
}

bool IsAbsolutePath(const string_view Path)
{
	const auto Type = ParsePath(Path);

	return (Type != root_type::unknown && Type != root_type::drive_letter) ||
	       (Type == root_type::drive_letter && (Path.size() > 2 && IsSlash(Path[2])));
}

bool HasPathPrefix(const string_view Path)
{
	/*
		\\?\
		\\.\
		\??\
	*/
	return Path.size() > 4 &&  Path[0] == L'\\' && (Path[1] == L'\\' || Path[1] == L'?') && (Path[2] == L'?' || Path[2] == L'.') && Path[3] == L'\\';
}

bool PathCanHoldRegularFile(const string& Path)
{
	return ParsePath(Path) != root_type::unknown;
}

bool IsPluginPrefixPath(const string& Path) //Max:
{
	if (Path.empty() || Path[0] == L'\\')
		return false;

	size_t pos = Path.find(L':');

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
	return starts_with(Str, L".."sv) && (Str.size() == 2 || (Str.size() == 3 && IsSlash(Str[2])));
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

bool IsCurrentDirectory(string_view Str)
{
	return starts_with(Str, L"."sv) && (Str.size() == 1 || (Str.size() == 2 && IsSlash(Str[1])));
}

string_view PointToName(string_view Path)
{
	if (Path.empty())
		return Path;

	auto NameStart = std::find_if(ALL_CONST_REVERSE_RANGE(Path), IsSlash);
	Path.remove_prefix(Path.crend() - NameStart);

	return Path;
}

//   Аналог PointToName, только для строк типа
//   "name\" (оканчивается на слеш) возвращает указатель на name, а не на пустую
//   строку
string_view PointToFolderNameIfFolder(string_view Path)
{
	while(!Path.empty() && IsSlash(Path.back()))
		Path.remove_suffix(1);

	return PointToName(Path);
}

string_view PointToExt(string_view Path)
{
	Path = PointToName(Path);

	if (Path.empty())
		return Path;

	const auto ExtensionStart = std::find(ALL_CONST_REVERSE_RANGE(Path), L'.');
	Path.remove_prefix(ExtensionStart == Path.crend()? Path.size() : Path.crend() - ExtensionStart - 1);
	return Path;
}


static size_t SlashType(const wchar_t *pw, const wchar_t *pe, wchar_t &TypeSlash)
{
	size_t Len = 0, Slash = 0, BackSlash = 0;
	while ((pe && pw < pe) || (!pe && *pw))
	{
		wchar_t c = *pw++;
		BackSlash += (c == L'\\');
		Slash += (c == L'/');
		++Len;
	}

	TypeSlash = (Slash > BackSlash ? L'/' : L'\\');
	return Len;
}

// Функция работает с обоими видами слешей, также происходит
//	изменение уже существующего конечного слеша на такой, который
// указан, или встречается чаще (при равенстве '\'). 
//
bool AddEndSlash(wchar_t *Path, wchar_t TypeSlash)
{
	if (!Path)
		return false;

	auto len = IsSlash(TypeSlash)? wcslen(Path) : SlashType(Path, nullptr, TypeSlash);

	if (len && IsSlash(Path[len-1]))
		--len;

	Path[len++] = TypeSlash;
	Path[len] = L'\0';
	return true;
}

bool AddEndSlash(wchar_t *Path)
{
	return AddEndSlash(Path, L'\0');
}

void AddEndSlash(string &strPath, wchar_t TypeSlash)
{
	const auto Path = strPath.c_str();
	auto len = strPath.size();
	if (!IsSlash(TypeSlash))
		SlashType(Path, Path+len, TypeSlash);

	wchar_t LastSlash = L'\0';
	if (len && IsSlash(Path[len-1]))
		LastSlash = Path[--len];

	if (TypeSlash != LastSlash)
	{
		if (LastSlash)
			strPath[len] = TypeSlash;
		else
			strPath.push_back(TypeSlash);
	}
}

void AddEndSlash(string &strPath)
{
	AddEndSlash(strPath, L'\0');
}

void DeleteEndSlash(wchar_t *Path)
{
	const auto REnd = std::make_reverse_iterator(Path);
	Path[REnd - std::find_if_not(REnd - wcslen(Path), REnd, IsSlash)] = 0;
}

void DeleteEndSlash(string &Path)
{
	Path.resize(Path.rend() - std::find_if_not(Path.rbegin(), Path.rend(), IsSlash));
}

void DeleteEndSlash(string_view& Path)
{
	Path.remove_suffix(std::find_if_not(Path.rbegin(), Path.rend(), IsSlash) - Path.rbegin());
}

bool CutToSlash(string &strStr, bool bInclude)
{
	const auto pos = FindLastSlash(strStr);
	if (pos != string::npos)
	{
		if (pos==3 && HasPathPrefix(strStr))
			return false;

		if (bInclude)
			strStr.resize(pos);
		else
			strStr.resize(pos+1);

		return true;
	}

	return false;
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
	const auto LastNotSlash = std::find_if_not(Str.crbegin(), REnd, IsSlash);
	const auto PrevSlash = std::find_if(LastNotSlash, REnd, IsSlash);
	const auto PrevNotSlash = std::find_if_not(PrevSlash, REnd, IsSlash);

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
	const auto SlashPos = std::find_if(ALL_CONST_RANGE(Str), IsSlash);
	return SlashPos == Str.cend()? string::npos : SlashPos - Str.cbegin();
}

size_t FindLastSlash(const string_view Str)
{
	const auto SlashPos = std::find_if(ALL_CONST_REVERSE_RANGE(Str), IsSlash);
	return SlashPos == Str.crend()? string::npos : Str.crend() - SlashPos - 1;
}

// find path root component (drive letter / volume name / server share) and calculate its length
static size_t GetPathRootLength(string_view const Path)
{
	size_t DirOffset = 0;
	return ParsePath(Path, &DirOffset) == root_type::unknown? 0 : DirOffset;
}

string ExtractPathRoot(string_view const Path)
{
	const auto PathRootLen = GetPathRootLength(Path);
	if (!PathRootLen)
		return{};

	return path::join(Path.substr(0, PathRootLen), L""sv);
}

string ExtractFileName(string_view const Path)
{
	auto p = FindLastSlash(Path);
	p = p == string::npos? 0 : p + 1;
	p = std::max(p, GetPathRootLength(Path));
	return string(Path.substr(p));
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
	auto PathPart = Start;
	DeleteEndSlash(PathPart);
	return starts_with(Path, PathPart) && (Path.size() == PathPart.size() || IsSlash(Path[PathPart.size()]));
}

#include "common/test.hpp"

#ifdef _DEBUG
void TestPaths()
{

	ASSERT_EQ(path::join(L"foo"sv, L""sv), L"foo\\"sv);
	ASSERT_EQ(path::join(L"foo"sv, L"\\"sv), L"foo\\"sv);
	ASSERT_EQ(path::join(L""sv, L""sv), L""sv);
	ASSERT_EQ(path::join(L""sv, L"\\"sv), L""sv);
	ASSERT_EQ(path::join(L""sv, L"foo"sv), L"foo"sv);
	ASSERT_EQ(path::join(L"\\foo"sv, L""sv), L"\\foo\\"sv);
	ASSERT_EQ(path::join(L"\\foo"sv, L"\\"sv), L"\\foo\\"sv);
	ASSERT_EQ(path::join(L"\\"sv, L"foo\\"sv), L"foo"sv);
	ASSERT_EQ(path::join(L"foo"sv, L"bar"sv), L"foo\\bar"sv);
	ASSERT_EQ(path::join(L"\\foo"sv, L"bar\\"sv), L"\\foo\\bar"sv);
	ASSERT_EQ(path::join(L"foo\\"sv, L"bar"sv), L"foo\\bar"sv);
	ASSERT_EQ(path::join(L"foo\\"sv, L"\\bar"sv), L"foo\\bar"sv);
	ASSERT_EQ(path::join(L"foo\\"sv, L'\\', L"\\bar"sv), L"foo\\bar"sv);
	ASSERT_EQ(path::join(L"foo\\"sv, L""sv, L"\\bar"sv), L"foo\\bar"sv);
	ASSERT_EQ(path::join(L"\\\\foo\\\\"sv, L"\\\\bar\\"sv), L"\\\\foo\\bar"sv);

	ASSERT_EQ(path::join(L"foo"sv, L""sv), L"foo\\"sv);
	ASSERT_EQ(path::join(L"foo"sv, L"\\"sv), L"foo\\"sv);
	ASSERT_EQ(path::join(L""sv, L""sv), L""sv);
	ASSERT_EQ(path::join(L""sv, L"\\"sv), L""sv);
	ASSERT_EQ(path::join(L""sv, L"foo"sv), L"foo"sv);
	ASSERT_EQ(path::join(L"\\foo"sv, L""sv), L"\\foo\\"sv);
	ASSERT_EQ(path::join(L"\\foo"sv, L"\\"sv), L"\\foo\\"sv);
	ASSERT_EQ(path::join(L"\\"sv, L"foo\\"sv), L"foo"sv);
	ASSERT_EQ(path::join(L"foo"sv, L"bar"sv), L"foo\\bar"sv);
	ASSERT_EQ(path::join(L"\\foo"sv, L"bar\\"sv), L"\\foo\\bar"sv);
	ASSERT_EQ(path::join(L"foo\\"sv, L"bar"sv), L"foo\\bar"sv);
	ASSERT_EQ(path::join(L"foo\\"sv, L"\\bar"sv), L"foo\\bar"sv);
	ASSERT_EQ(path::join(L"foo\\"sv, L'\\', L"\\bar"sv), L"foo\\bar"sv);
	ASSERT_EQ(path::join(L"foo\\"sv, L""sv, L"\\bar"sv), L"foo\\bar"sv);
	ASSERT_EQ(path::join(L"\\\\foo\\\\"sv, L"\\\\bar\\"sv), L"\\\\foo\\bar"sv);

	ASSERT_EQ(ExtractPathRoot(L""), L"");
	ASSERT_EQ(ExtractPathRoot(L"\\"), L"");
	ASSERT_EQ(ExtractPathRoot(L"file"), L"");
	ASSERT_EQ(ExtractPathRoot(L"path\\file"), L"");
	ASSERT_EQ(ExtractPathRoot(L"C:"), L"C:\\");
	ASSERT_EQ(ExtractPathRoot(L"C:\\"), L"C:\\");
	ASSERT_EQ(ExtractPathRoot(L"C:\\path\\file"), L"C:\\");
	ASSERT_EQ(ExtractPathRoot(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}"), L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
	ASSERT_EQ(ExtractPathRoot(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\"), L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
	ASSERT_EQ(ExtractPathRoot(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path\\file"), L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
	ASSERT_EQ(ExtractPathRoot(L"\\\\server\\share"), L"\\\\server\\share\\");
	ASSERT_EQ(ExtractPathRoot(L"\\\\server\\share\\"), L"\\\\server\\share\\");
	ASSERT_EQ(ExtractPathRoot(L"\\\\server\\share\\path\\file"), L"\\\\server\\share\\");
	ASSERT_EQ(ExtractPathRoot(L"\\\\1.2.3.4\\share\\path\\file"), L"\\\\1.2.3.4\\share\\");
	ASSERT_EQ(ExtractPathRoot(L"\\\\?\\UNC\\server\\share"), L"\\\\?\\UNC\\server\\share\\");
	ASSERT_EQ(ExtractPathRoot(L"\\\\?\\UNC\\server\\share\\"), L"\\\\?\\UNC\\server\\share\\");
	ASSERT_EQ(ExtractPathRoot(L"\\\\?\\UNC\\server\\share\\path\\file"), L"\\\\?\\UNC\\server\\share\\");
	ASSERT_EQ(ExtractPathRoot(L"\\\\?\\UNC\\1.2.3.4\\share\\path\\file"), L"\\\\?\\UNC\\1.2.3.4\\share\\");

	ASSERT_EQ(ExtractFilePath(L""), L"");
	ASSERT_EQ(ExtractFilePath(L"\\"), L"");
	ASSERT_EQ(ExtractFilePath(L"\\file"), L"");
	ASSERT_EQ(ExtractFilePath(L"file"), L"");
	ASSERT_EQ(ExtractFilePath(L"path\\"), L"path");
	ASSERT_EQ(ExtractFilePath(L"path\\file"), L"path");
	ASSERT_EQ(ExtractFilePath(L"C:"), L"C:\\");
	ASSERT_EQ(ExtractFilePath(L"C:\\"), L"C:\\");
	ASSERT_EQ(ExtractFilePath(L"C:\\file"), L"C:\\");
	ASSERT_EQ(ExtractFilePath(L"C:\\path\\file"), L"C:\\path");
	ASSERT_EQ(ExtractFilePath(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}"), L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
	ASSERT_EQ(ExtractFilePath(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\"), L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
	ASSERT_EQ(ExtractFilePath(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\file"), L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
	ASSERT_EQ(ExtractFilePath(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path\\file"), L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path");
	ASSERT_EQ(ExtractFilePath(L"\\\\server\\share"), L"\\\\server\\share\\");
	ASSERT_EQ(ExtractFilePath(L"\\\\server\\share\\"), L"\\\\server\\share\\");
	ASSERT_EQ(ExtractFilePath(L"\\\\server\\share\\file"), L"\\\\server\\share\\");
	ASSERT_EQ(ExtractFilePath(L"\\\\server\\share\\path\\file"), L"\\\\server\\share\\path");
	ASSERT_EQ(ExtractFilePath(L"\\\\?\\UNC\\server\\share"), L"\\\\?\\UNC\\server\\share\\");
	ASSERT_EQ(ExtractFilePath(L"\\\\?\\UNC\\server\\share\\"), L"\\\\?\\UNC\\server\\share\\");
	ASSERT_EQ(ExtractFilePath(L"\\\\?\\UNC\\server\\share\\file"), L"\\\\?\\UNC\\server\\share\\");
	ASSERT_EQ(ExtractFilePath(L"\\\\?\\UNC\\server\\share\\path\\file"), L"\\\\?\\UNC\\server\\share\\path");

	ASSERT_EQ(ExtractFileName(L""), L"");
	ASSERT_EQ(ExtractFileName(L"\\"), L"");
	ASSERT_EQ(ExtractFileName(L"\\file"), L"file");
	ASSERT_EQ(ExtractFileName(L"file"), L"file");
	ASSERT_EQ(ExtractFileName(L"path\\"), L"");
	ASSERT_EQ(ExtractFileName(L"path\\file"), L"file");
	ASSERT_EQ(ExtractFileName(L"C:"), L"");
	ASSERT_EQ(ExtractFileName(L"C:\\"), L"");
	ASSERT_EQ(ExtractFileName(L"C:\\file"), L"file");
	ASSERT_EQ(ExtractFileName(L"C:\\path\\file"), L"file");
	ASSERT_EQ(ExtractFileName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}"), L"");
	ASSERT_EQ(ExtractFileName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\"), L"");
	ASSERT_EQ(ExtractFileName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\file"), L"file");
	ASSERT_EQ(ExtractFileName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path\\file"), L"file");
	ASSERT_EQ(ExtractFileName(L"\\\\server\\share"), L"");
	ASSERT_EQ(ExtractFileName(L"\\\\server\\share\\"), L"");
	ASSERT_EQ(ExtractFileName(L"\\\\server\\share\\file"), L"file");
	ASSERT_EQ(ExtractFileName(L"\\\\server\\share\\path\\file"), L"file");
	ASSERT_EQ(ExtractFileName(L"\\\\?\\UNC\\server\\share"), L"");
	ASSERT_EQ(ExtractFileName(L"\\\\?\\UNC\\server\\share\\"), L"");
	ASSERT_EQ(ExtractFileName(L"\\\\?\\UNC\\server\\share\\file"), L"file");
	ASSERT_EQ(ExtractFileName(L"\\\\?\\UNC\\server\\share\\path\\file"), L"file");

	ASSERT_EQ(PointToName(L""sv), L""sv);
	ASSERT_EQ(PointToName(L"\\"sv), L""sv);
	ASSERT_EQ(PointToName(L"\\file"sv), L"file"sv);
	ASSERT_EQ(PointToName(L"file"sv), L"file"sv);
	ASSERT_EQ(PointToName(L"path\\"sv), L""sv);
	ASSERT_EQ(PointToName(L"path\\file"sv), L"file"sv);
	//ASSERT_EQ(PointToName(L"C:"sv), L""sv);
	ASSERT_EQ(PointToName(L"C:\\"sv), L""sv);
	ASSERT_EQ(PointToName(L"C:\\file"sv), L"file"sv);
	ASSERT_EQ(PointToName(L"C:\\path\\file"sv), L"file"sv);
	//ASSERT_EQ(PointToName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}"sv), L""sv);
	ASSERT_EQ(PointToName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\"sv), L""sv);
	ASSERT_EQ(PointToName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\file"sv), L"file"sv);
	ASSERT_EQ(PointToName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path\\file"sv), L"file"sv);
	//ASSERT_EQ(PointToName(L"\\\\server\\share"sv), L""sv);
	ASSERT_EQ(PointToName(L"\\\\server\\share\\"sv), L""sv);
	ASSERT_EQ(PointToName(L"\\\\server\\share\\file"sv), L"file"sv);
	ASSERT_EQ(PointToName(L"\\\\server\\share\\path\\file"sv), L"file"sv);
	//ASSERT_EQ(PointToName(L"\\\\?\\UNC\\server\\share"sv), L""sv);
	ASSERT_EQ(PointToName(L"\\\\?\\UNC\\server\\share\\"sv), L""sv);
	ASSERT_EQ(PointToName(L"\\\\?\\UNC\\server\\share\\file"sv), L"file"sv);
	ASSERT_EQ(PointToName(L"\\\\?\\UNC\\server\\share\\path\\file"sv), L"file"sv);

	ASSERT_EQ(PointToExt(L""sv), L""sv);
	ASSERT_EQ(PointToExt(L"file"sv), L""sv);
	ASSERT_EQ(PointToExt(L"path\\file"sv), L""sv);
	ASSERT_EQ(PointToExt(L"file.ext"sv), L".ext"sv);
	ASSERT_EQ(PointToExt(L"path\\file.ext"sv), L".ext"sv);
	ASSERT_EQ(PointToExt(L"file.ext1.ext2"sv), L".ext2"sv);
	ASSERT_EQ(PointToExt(L"path\\file.ext1.ext2"sv), L".ext2"sv);

	ASSERT_TRUE(IsRootPath(L"C:"));
	ASSERT_TRUE(IsRootPath(L"C:\\"));
	ASSERT_TRUE(IsRootPath(L"\\"));
	ASSERT_FALSE(IsRootPath(L"C:\\path"));

	ASSERT_TRUE(PathStartsWith(L"C:\\path\\file", L"C:\\path"));
	ASSERT_TRUE(PathStartsWith(L"C:\\path\\file", L"C:\\path\\"));
	ASSERT_FALSE(PathStartsWith(L"C:\\path\\file", L"C:\\pat"));
	ASSERT_TRUE(PathStartsWith(L"\\", L""));
	ASSERT_FALSE(PathStartsWith(L"C:\\path\\file", L""));

	static const string_view TestRoots[] =
	{
		L""sv,
		L"C:"sv,
		L"C:\\"sv,
		L"\\\\server\\share\\"sv,
		L"\\\\?\\C:\\"sv,
		L"\\\\?\\UNC\\server\\share\\"sv,
		L"\\\\?\\Volume{f26b206c-f912-11e1-b516-806e6f6e6963}\\"sv,
		L"\\\\?\\pipe\\"sv,
	};

	static const struct
	{
		const wchar_t* InputPath;
		const wchar_t* ExpectedPath;
		bool RootMustExist;
		bool ExpectedResult;
	}
	TestCases[] =
	{
		// root directory, shall fail
		{ L"{0}", L"{0}", false, false},

		// one level, shall return root directory
		{ L"{0}dir1", L"{0}", true, true },

		// one level without root, shall fail
		{ L"dir1", L"dir1", false, false },

		// two levels, shall return first level
		{ L"{0}dir1\\dir2", L"{0}dir1", false, true },

		// two levels with trailing slash, shall return first level
		{ L"{0}dir1\\dir2\\", L"{0}dir1", false, true },
	};

	string Path, Baseline;
	for (const auto& Root: TestRoots)
	{
		for (const auto& Test: TestCases)
		{
			if (Root.empty() && Test.RootMustExist)
				continue;

			Path = format(Test.InputPath, Root);
			Baseline = format(Test.ExpectedPath, Root);

			ASSERT_EQ(Test.ExpectedResult, CutToParent(Path));
			ASSERT_EQ(Baseline, Path);
		}
	}
}
#endif

SELF_TEST(TestPaths)


