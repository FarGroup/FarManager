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

#include "headers.hpp"
#pragma hdrstop

#include "pathmix.hpp"
#include "strmix.hpp"
#include "imports.hpp"
#include "regex_helpers.hpp"

void NTPath::Transform()
{
	string& Data = *this;
	if (!Data.empty())
	{
		if(!HasPathPrefix(Data))
		{
			ConvertNameToFull(Data,Data);

			if (!HasPathPrefix(Data))
			{
				ReplaceSlashToBackslash(Data);
				string Prefix(ParsePath(Data) == PATH_DRIVELETTER? L"\\\\?\\" : L"\\\\?\\UNC");
				while(ReplaceStrings(Data,L"\\\\",L"\\"))
					;
				Data=Prefix+Data;
			}
		}
		static const bool is_win2k = !IsWindowsXPOrGreater();
		if(is_win2k && Data.size() > 5 && Data[5] == L':')
		{
			// "\\?\C:" -> "\\?\c:"
			// Some file operations fail on Win2k if a drive letter is in upper case
			ToLower(Data, 4, 1);
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


PATH_TYPE ParsePath(const string& path, size_t* DirectoryOffset, bool* Root)
{
	PATH_TYPE Result = PATH_UNKNOWN;

	static struct
	{
		PATH_TYPE Type;
		const wchar_t* REStr;
		std::wregex re;
	}
	PathTypes[] =
	{
		// TODO: tests for all these types

#define RE_PATH_PREFIX(x) RE_C_GROUP(RE_BEGIN RE_BACKSLASH RE_REPEAT(2) RE_ANY_OF(RE_Q_MARK RE_DOT) RE_BACKSLASH x )

		// x:<whatever> or x:\\<whatever>
		{ PATH_DRIVELETTER, RE_C_GROUP(RE_BEGIN RE_ANY RE_ESCAPE(L":")) RE_NC_GROUP(RE_ANY_SLASH RE_ZERO_OR_ONE_GREEDY) },
		// \\?\x: or \\?\x:\ or \\?\x:\<whatever>
		{ PATH_DRIVELETTERUNC, RE_PATH_PREFIX(L".\\:") RE_ANY_SLASH_OR_NONE },
		// \\server\share or \\server\share\ or \\server\share<whatever>
		{ PATH_REMOTE, RE_C_GROUP(RE_BEGIN RE_ANY_SLASH RE_REPEAT(2) RE_NONE_OF(RE_SPACE RE_SLASHES RE_Q_MARK) RE_ONE_OR_MORE_LAZY RE_ANY_SLASH RE_ONE_OR_MORE_LAZY RE_NONE_OF(RE_SLASHES) RE_ONE_OR_MORE_GREEDY) RE_ANY_SLASH_OR_NONE },
		// \\?\unc\server\share or \\?\unc\server\share\ or \\?\unc\server\share<whatever>
		{ PATH_REMOTEUNC, RE_PATH_PREFIX(L"unc" RE_BACKSLASH RE_NONE_OF(RE_SPACE RE_SLASHES RE_Q_MARK) RE_ONE_OR_MORE_LAZY RE_BACKSLASH RE_NONE_OF(RE_SLASHES) RE_ONE_OR_MORE_GREEDY) RE_ANY_SLASH_OR_NONE },
		// \\?\Volume{GUID} or \\?\Volume{GUID}\ or \\?\Volume{GUID}<whatever>
		{ PATH_VOLUMEGUID, RE_PATH_PREFIX(L"volume" RE_ESCAPE(L"{") RE_ANY_UUID RE_ESCAPE(L"}")) RE_ANY_SLASH_OR_NONE },
		// \\?\pipe\ or \\?\pipe
		{ PATH_PIPE, RE_PATH_PREFIX(L"pipe") RE_ANY_SLASH_OR_NONE },

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

	std::wsmatch SMatch;

	const auto ItemIterator = std::find_if(CONST_RANGE(PathTypes, i) { return std::regex_search(path, SMatch, i.re); });

	if (ItemIterator != std::cend(PathTypes))
	{
		const size_t MatchLength = SMatch[0].length();
		if (DirectoryOffset)
		{
			*DirectoryOffset = MatchLength;
		}
		if (Root)
		{
			*Root = path.size() == MatchLength || (path.size() == (MatchLength + 1) && IsSlash(path[MatchLength]));
		}
		Result = ItemIterator->Type;

	}

	return Result;
}

bool IsAbsolutePath(const string& Path)
{
	const auto Type = ParsePath(Path);
	return Type == PATH_DRIVELETTERUNC || Type == PATH_REMOTE || Type == PATH_REMOTEUNC || Type == PATH_VOLUMEGUID || (Type == PATH_DRIVELETTER && (Path.size() > 2 && IsSlash(Path[2])));
}

bool HasPathPrefix(const string& Path)
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
	return ParsePath(Path) != PATH_UNKNOWN;
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
		if (os::QueryDosDevice(Path.substr(0,2), dev))
			return false;
	}

	const auto SlashPos = FindSlash(Path);
	return SlashPos == string::npos || SlashPos > pos;
}

bool TestParentFolderName(const string& Name)
{
	return (Name.size() == 2 && Name[0] == L'.' && Name[1] == L'.') || (Name.size() == 3 && Name[0] == L'.' && Name[1] == L'.' && IsSlash(Name[2]));
}

bool TestCurrentDirectory(const string& TestDir)
{
	return !StrCmpI(os::GetCurrentDirectory(), TestDir);
}

const wchar_t* PointToName(const wchar_t* Path)
{
	return PointToName(Path,nullptr);
}

const wchar_t* PointToName(const wchar_t* Path,const wchar_t* EndPtr)
{
	if (!Path)
		return nullptr;

	if (*Path && *(Path+1)==L':') Path+=2;

	auto NamePtr = EndPtr? EndPtr : Path + wcslen(Path);

	while (NamePtr != Path)
	{
		if (IsSlash(*NamePtr))
			return NamePtr+1;

		NamePtr--;
	}

	if (IsSlash(*Path))
		return Path+1;
	else
		return Path;
}

//   Аналог PointToName, только для строк типа
//   "name\" (оканчивается на слеш) возвращает указатель на name, а не на пустую
//   строку
const wchar_t* PointToFolderNameIfFolder(const wchar_t *Path)
{
	if (!Path)
		return nullptr;

	const wchar_t *NamePtr=Path, *prevNamePtr=Path;

	while (*Path)
	{
		if (IsSlash(*Path) ||
		        (*Path==L':' && Path==NamePtr+1))
		{
			prevNamePtr=NamePtr;
			NamePtr=Path+1;
		}

		++Path;
	}

	return *NamePtr? NamePtr : prevNamePtr;
}

const wchar_t* PointToExt(const wchar_t* Path)
{
	return Path? PointToExt(Path, Path + wcslen(Path)) : nullptr;
}

const wchar_t* PointToExt(const wchar_t* Path,const wchar_t* EndPtr)
{
	if (!Path || !EndPtr)
		return nullptr;

	const wchar_t* ExtPtr = EndPtr;

	const auto IsPath = [&Path](const wchar_t* Ptr)
	{
		return IsSlash(*Ptr) || (*Ptr == L':' && Ptr - Path == 1); // ':' only in c:
	};

	while (ExtPtr != Path)
	{
		if (*ExtPtr==L'.')
		{
			if (IsPath(ExtPtr - 1))
				return EndPtr;
			else
				return ExtPtr;
		}

		if (IsPath(ExtPtr))
			return EndPtr;

		ExtPtr--;
	}

	return EndPtr;
}


static int SlashType(const wchar_t *pw, const wchar_t *pe, wchar_t &TypeSlash)
{
	int Len = 0, Slash = 0, BackSlash = 0;
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

	int len = IsSlash(TypeSlash) ? StrLength(Path) : SlashType(Path, nullptr, TypeSlash);

	if (len > 0 && IsSlash(Path[len-1]))
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
	const wchar_t *Path = strPath.data();
	int len = static_cast<int>(strPath.size());
	if (!IsSlash(TypeSlash))
		SlashType(Path, Path+len, TypeSlash);

	wchar_t LastSlash = L'\0';
	if (len > 0 && IsSlash(Path[len-1]))
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
	const auto REnd = std::reverse_iterator<wchar_t*>(Path);
	Path[REnd - std::find_if_not(REnd - wcslen(Path), REnd, IsSlash)] = 0;
}

void DeleteEndSlash(string &Path)
{
	Path.resize(Path.rend() - std::find_if_not(Path.rbegin(), Path.rend(), IsSlash));
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

bool CutToParent(string &strStr)
{
	bool Result = false;
	size_t RootLength = 0;
	ParsePath(strStr, &RootLength);
	const auto RootOnly = RootLength == strStr.size();
	const auto EndPos = !strStr.empty() && IsSlash(strStr.back()) && !RootOnly? strStr.size() - 1 : strStr.size();
	const auto RBegin = strStr.rbegin() + strStr.size() - EndPos, REnd = strStr.rend() - RootLength;
	const auto LastSlashIter = std::find_if(RBegin, REnd, IsSlash);
	if (LastSlashIter != REnd)
	{
		strStr.resize(strStr.rend() - LastSlashIter - 1);
		Result = true;
	}
	else if (RootLength && !RootOnly)
	{
		strStr.resize(RootLength);
		Result = true;
	}
	return Result;
}

bool ContainsSlash(const wchar_t *Str)
{
	const auto Iterator = null_iterator(Str);
	return std::find_if(Iterator, Iterator.end(), IsSlash) != Iterator.end();
}

size_t FindSlash(const string &Str)
{
	return Str.find_first_of(L"/\\", 0, 2);
}

size_t FindLastSlash(const string &Str)
{
	return Str.find_last_of(L"/\\", string::npos, 2);
}

// find path root component (drive letter / volume name / server share) and calculate its length
size_t GetPathRootLength(const string &Path)
{
	size_t DirOffset = 0;
	return (ParsePath(Path, &DirOffset) == PATH_UNKNOWN)? 0 : DirOffset;
}

string ExtractPathRoot(const string &Path)
{
	size_t PathRootLen = GetPathRootLength(Path);

	if (PathRootLen)
	{
		string result(Path, 0, PathRootLen);
		AddEndSlash(result);
		return result;
	}
	else
		return {};
}

string ExtractFileName(const string &Path)
{
	auto p = FindLastSlash(Path);
	p = p == string::npos? 0 : p + 1;
	p = std::max(p, GetPathRootLength(Path));
	return Path.substr(p);
}

string ExtractFilePath(const string &Path)
{
	auto p = FindLastSlash(Path);
	if (p == string::npos)
	{
		p = 0;
	}

	const auto PathRootLen = GetPathRootLength(Path);

	if (p <= PathRootLen && PathRootLen)
	{
		string result(Path, 0, PathRootLen);
		AddEndSlash(result);
		return result;
	}

	return string(Path.data(), p);
}

bool IsRootPath(const string &Path)
{
	bool IsRoot = false;
	ParsePath(Path, nullptr, &IsRoot);
	return IsRoot || IsRelativeRoot(Path);
}

bool PathStartsWith(const string &Path, const string &Start)
{
	string PathPart(Start);
	DeleteEndSlash(PathPart);
	return Path.compare(0, PathPart.size(), PathPart) == 0 && (Path.size() == PathPart.size() || IsSlash(Path[PathPart.size()]));
}

int MatchNtPathRoot(const string &NtPath, const string& DeviceName)
{
	string TargetPath;
	if (os::QueryDosDevice(DeviceName, TargetPath))
	{
		if (PathStartsWith(NtPath, TargetPath))
			return static_cast<int>(TargetPath.size());

		// path could be an Object Manager symlink, try to resolve
		UNICODE_STRING ObjName;
		ObjName.Length = ObjName.MaximumLength = static_cast<USHORT>(TargetPath.size() * sizeof(wchar_t));
		ObjName.Buffer = UNSAFE_CSTR(TargetPath);
		OBJECT_ATTRIBUTES ObjAttrs;
		InitializeObjectAttributes(&ObjAttrs, &ObjName, 0, nullptr, nullptr);
		HANDLE hSymLink;
		NTSTATUS Res = Imports().NtOpenSymbolicLinkObject(&hSymLink, GENERIC_READ, &ObjAttrs);

		if (Res == STATUS_SUCCESS)
		{
			SCOPE_EXIT{ Imports().NtClose(hSymLink); };

			ULONG BufSize = 32767;
			wchar_t_ptr Buffer(BufSize);
			UNICODE_STRING LinkTarget;
			LinkTarget.MaximumLength = static_cast<USHORT>(BufSize * sizeof(wchar_t));
			LinkTarget.Buffer = Buffer.get();
			Res = Imports().NtQuerySymbolicLinkObject(hSymLink, &LinkTarget, nullptr);

			if (Res == STATUS_SUCCESS)
			{
				TargetPath.assign(LinkTarget.Buffer, LinkTarget.Length / sizeof(wchar_t));
			}

			if (PathStartsWith(NtPath, TargetPath))
				return static_cast<int>(TargetPath.size());
		}
	}

	return 0;
}

void TestPathParser()
{
#ifdef _DEBUG
    assert(ExtractPathRoot(L"") == L"");
    assert(ExtractPathRoot(L"\\") == L"");
    assert(ExtractPathRoot(L"file") == L"");
    assert(ExtractPathRoot(L"path\\file") == L"");
    assert(ExtractPathRoot(L"C:") == L"C:\\");
    assert(ExtractPathRoot(L"C:\\") == L"C:\\");
    assert(ExtractPathRoot(L"C:\\path\\file") == L"C:\\");
    assert(ExtractPathRoot(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}") == L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
    assert(ExtractPathRoot(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\") == L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
    assert(ExtractPathRoot(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path\\file") == L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
    assert(ExtractPathRoot(L"\\\\server\\share") == L"\\\\server\\share\\");
    assert(ExtractPathRoot(L"\\\\server\\share\\") == L"\\\\server\\share\\");
    assert(ExtractPathRoot(L"\\\\server\\share\\path\\file") == L"\\\\server\\share\\");
    assert(ExtractPathRoot(L"\\\\1.2.3.4\\share\\path\\file") == L"\\\\1.2.3.4\\share\\");
    assert(ExtractPathRoot(L"\\\\?\\UNC\\server\\share") == L"\\\\?\\UNC\\server\\share\\");
    assert(ExtractPathRoot(L"\\\\?\\UNC\\server\\share\\") == L"\\\\?\\UNC\\server\\share\\");
    assert(ExtractPathRoot(L"\\\\?\\UNC\\server\\share\\path\\file") == L"\\\\?\\UNC\\server\\share\\");
    assert(ExtractPathRoot(L"\\\\?\\UNC\\1.2.3.4\\share\\path\\file") == L"\\\\?\\UNC\\1.2.3.4\\share\\");

    assert(ExtractFilePath(L"") == L"");
    assert(ExtractFilePath(L"\\") == L"");
    assert(ExtractFilePath(L"\\file") == L"");
    assert(ExtractFilePath(L"file") == L"");
    assert(ExtractFilePath(L"path\\") == L"path");
    assert(ExtractFilePath(L"path\\file") == L"path");
    assert(ExtractFilePath(L"C:") == L"C:\\");
    assert(ExtractFilePath(L"C:\\") == L"C:\\");
    assert(ExtractFilePath(L"C:\\file") == L"C:\\");
    assert(ExtractFilePath(L"C:\\path\\file") == L"C:\\path");
    assert(ExtractFilePath(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}") == L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
    assert(ExtractFilePath(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\") == L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
    assert(ExtractFilePath(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\file") == L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\");
    assert(ExtractFilePath(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path\\file") == L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path");
    assert(ExtractFilePath(L"\\\\server\\share") == L"\\\\server\\share\\");
    assert(ExtractFilePath(L"\\\\server\\share\\") == L"\\\\server\\share\\");
    assert(ExtractFilePath(L"\\\\server\\share\\file") == L"\\\\server\\share\\");
    assert(ExtractFilePath(L"\\\\server\\share\\path\\file") == L"\\\\server\\share\\path");
    assert(ExtractFilePath(L"\\\\?\\UNC\\server\\share") == L"\\\\?\\UNC\\server\\share\\");
    assert(ExtractFilePath(L"\\\\?\\UNC\\server\\share\\") == L"\\\\?\\UNC\\server\\share\\");
    assert(ExtractFilePath(L"\\\\?\\UNC\\server\\share\\file") == L"\\\\?\\UNC\\server\\share\\");
    assert(ExtractFilePath(L"\\\\?\\UNC\\server\\share\\path\\file") == L"\\\\?\\UNC\\server\\share\\path");

    assert(ExtractFileName(L"") == L"");
    assert(ExtractFileName(L"\\") == L"");
    assert(ExtractFileName(L"\\file") == L"file");
    assert(ExtractFileName(L"file") == L"file");
    assert(ExtractFileName(L"path\\") == L"");
    assert(ExtractFileName(L"path\\file") == L"file");
    assert(ExtractFileName(L"C:") == L"");
    assert(ExtractFileName(L"C:\\") == L"");
    assert(ExtractFileName(L"C:\\file") == L"file");
    assert(ExtractFileName(L"C:\\path\\file") == L"file");
    assert(ExtractFileName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}") == L"");
    assert(ExtractFileName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\") == L"");
    assert(ExtractFileName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\file") == L"file");
    assert(ExtractFileName(L"\\\\?\\Volume{01e45c83-9ce4-11db-b27f-806d6172696f}\\path\\file") == L"file");
    assert(ExtractFileName(L"\\\\server\\share") == L"");
    assert(ExtractFileName(L"\\\\server\\share\\") == L"");
    assert(ExtractFileName(L"\\\\server\\share\\file") == L"file");
    assert(ExtractFileName(L"\\\\server\\share\\path\\file") == L"file");
    assert(ExtractFileName(L"\\\\?\\UNC\\server\\share") == L"");
    assert(ExtractFileName(L"\\\\?\\UNC\\server\\share\\") == L"");
    assert(ExtractFileName(L"\\\\?\\UNC\\server\\share\\file") == L"file");
    assert(ExtractFileName(L"\\\\?\\UNC\\server\\share\\path\\file") == L"file");

    assert(IsRootPath(L"C:"));
    assert(IsRootPath(L"C:\\"));
    assert(IsRootPath(L"\\"));
    assert(!IsRootPath(L"C:\\path"));

    assert(PathStartsWith(L"C:\\path\\file", L"C:\\path"));
    assert(PathStartsWith(L"C:\\path\\file", L"C:\\path\\"));
    assert(!PathStartsWith(L"C:\\path\\file", L"C:\\pat"));
    assert(PathStartsWith(L"\\", L""));
    assert(!PathStartsWith(L"C:\\path\\file", L""));

	const wchar_t* TestRoots[] =
	{
		L"",
		L"C:",
		L"C:\\",
		L"\\\\server\\share\\",
		L"\\\\?\\C:\\",
		L"\\\\?\\UNC\\server\\share\\",
		L"\\\\?\\Volume{f26b206c-f912-11e1-b516-806e6f6e6963}\\",
		L"\\\\?\\pipe\\",
	};

	const struct
	{
		const wchar_t* InputPath;
		const wchar_t* ExpectedPath;
		bool RootMustExist;
		bool ExpectedReult;
	}
	TestCases[] =
	{
		// root directory, shall fail
		{ L"[root]", L"[root]", false, false},

		// one level, shall return root directory
		{ L"[root]dir1", L"[root]", true, true },

		// one level without root, shall fail
		{ L"dir1", L"dir1", false, false },

		// two levels, shall return first level
		{ L"[root]dir1\\dir2", L"[root]dir1", false, true },

		// two levels with trailing slash, shall return first level
		{ L"[root]dir1\\dir2\\", L"[root]dir1", false, true },
	};

	string Path, Baseline;
	for (const auto& Root: TestRoots)
	{
		for (const auto& Test: TestCases)
		{
			if (!*Root && Test.RootMustExist)
				continue;

			Path = Test.InputPath;
			ReplaceStrings(Path, L"[root]", Root);
			Baseline = Test.ExpectedPath;
			ReplaceStrings(Baseline, L"[root]", Root);

			const auto Result = CutToParent(Path);
			assert(Result == Test.ExpectedReult);
			assert(Path == Baseline);
		}
	}
#endif
}
