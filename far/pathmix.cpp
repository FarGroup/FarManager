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
				ReplaceSlashToBSlash(Data);
				string Prefix(ParsePath(Data) == PATH_DRIVELETTER? L"\\\\?\\" : L"\\\\?\\UNC");
				while(ReplaceStrings(Data,L"\\\\",L"\\"));
				Data=Prefix+Data;
			}
		}
		if(Data.size() > 5 && Data[5] == L':')
		{
			// "\\?\C:" -> "\\?\c:"
			// Some file operations fails on Win2k if a drive letter is in upper case
			ToLower(Data, 4, 1);
		}
	}
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
		// x:<whatever> or x:\\<whatever>
		{PATH_DRIVELETTER, L"(^.\\:)(?:[\\\\\\/]|$)"},
		// \\?\x: or \\?\x:\ or \\?\x:\<whatever>
		{PATH_DRIVELETTERUNC, L"(^\\\\{2}[\\?\\.]\\\\.\\:)(?:[\\\\\\/]|$)"},
		// \\server\share or \\server\share\ or \\server\share<whatever>
		{PATH_REMOTE, L"(^\\\\{2}[^ \\\\\\/\\?\\.][^ \\\\\\/\\?]+?\\\\[^\\\\\\/]+?)(?:[\\\\\\/]|$)"},
		// \\?\unc\server\share or \\?\unc\server\share\ or \\?\unc\server\share<whatever>
		{PATH_REMOTEUNC, L"(^\\\\{2}[\\?\\.]\\\\unc\\\\[^ \\\\\\/]+?\\\\[^\\\\\\/]+?)(?:[\\\\\\/]|$)"},
		// \\?\Volume{GUID} or \\?\Volume{GUID}\ or \\?\Volume{GUID}<whatever>
		{PATH_VOLUMEGUID, L"(^\\\\{2}[\\?\\.]\\\\volume\\{[0-9A-Fa-f]{8}-(?:[0-9A-Fa-f]{4}-){3}[0-9A-Fa-f]{12}\\})(?:[\\\\\\/]|$)"},
		// \\?\pipe\ or \\?\pipe
		{PATH_PIPE, L"(^\\\\{2}[\\?\\.]\\\\pipe)(?:[\\\\\\/]|$)"},
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
		if (api::QueryDosDevice(Path.substr(0,2), dev))
			return false;
	}

	const wchar_t* pS = FirstSlash(Path.data());

	if (pS && static_cast<size_t>(pS - Path.data()) < pos)
		return false;

	return true;
}

bool TestParentFolderName(const string& Name)
{
	return (Name.size() == 2 && Name[0] == L'.' && Name[1] == L'.') || (Name.size() == 3 && Name[0] == L'.' && Name[1] == L'.' && IsSlash(Name[2]));
}

bool TestCurrentDirectory(const string& TestDir)
{
	string strCurDir;

	if (api::GetCurrentDirectory(strCurDir) && !StrCmpI(strCurDir, TestDir))
		return true;

	return false;
}

const wchar_t* PointToName(const wchar_t *lpwszPath)
{
	return PointToName(lpwszPath,nullptr);
}

const wchar_t* PointToName(const wchar_t *lpwszPath,const wchar_t *lpwszEndPtr)
{
	if (!lpwszPath)
		return nullptr;

	if (*lpwszPath && *(lpwszPath+1)==L':') lpwszPath+=2;

	auto lpwszNamePtr = lpwszEndPtr? lpwszEndPtr : lpwszPath + wcslen(lpwszPath);

	while (lpwszNamePtr != lpwszPath)
	{
		if (IsSlash(*lpwszNamePtr))
			return lpwszNamePtr+1;

		lpwszNamePtr--;
	}

	if (IsSlash(*lpwszPath))
		return lpwszPath+1;
	else
		return lpwszPath;
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

const wchar_t* PointToExt(const wchar_t *lpwszPath)
{
	return lpwszPath? PointToExt(lpwszPath, lpwszPath + wcslen(lpwszPath)) : nullptr;
}

const wchar_t* PointToExt(const wchar_t *lpwszPath,const wchar_t *lpwszEndPtr)
{
	if (!lpwszPath || !lpwszEndPtr)
		return nullptr;

	const wchar_t *lpwszExtPtr = lpwszEndPtr;

	const auto IsPath = [&lpwszPath](const wchar_t* Ptr)
	{
		return IsSlash(*Ptr) || (*Ptr == L':' && Ptr - lpwszPath == 1); // ':' only in c:
	};

	while (lpwszExtPtr != lpwszPath)
	{
		if (*lpwszExtPtr==L'.')
		{
			if (IsPath(lpwszExtPtr - 1))
				return lpwszEndPtr;
			else
				return lpwszExtPtr;
		}

		if (IsPath(lpwszExtPtr))
			return lpwszEndPtr;

		lpwszExtPtr--;
	}

	return lpwszEndPtr;
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
	size_t pos;

	if (FindLastSlash(pos,strStr))
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

string &CutToNameUNC(string &strPath)
{
	const wchar_t *lpwszPath = strPath.data();

	if (IsSlash(lpwszPath[0]) && IsSlash(lpwszPath[1]))
	{
		lpwszPath+=2;

		for (int i=0; i<2; i++)
		{
			while (*lpwszPath && !IsSlash(*lpwszPath))
				lpwszPath++;

			if (*lpwszPath)
				lpwszPath++;
		}
	}

	const wchar_t *lpwszNamePtr = lpwszPath;

	while (*lpwszPath)
	{
		if (IsSlash(*lpwszPath) || (*lpwszPath==L':' && lpwszPath == lpwszNamePtr+1))
			lpwszNamePtr = lpwszPath+1;

		lpwszPath++;
	}

	strPath.resize(lpwszNamePtr - strPath.data());

	return strPath;
}

string &CutToFolderNameIfFolder(string &strPath)
{
	const wchar_t *lpwszPath = strPath.data();
	const wchar_t *lpwszNamePtr=lpwszPath, *lpwszprevNamePtr=lpwszPath;

	while (*lpwszPath)
	{
		if (IsSlash(*lpwszPath) || (*lpwszPath==L':' && lpwszPath==lpwszNamePtr+1))
		{
			lpwszprevNamePtr=lpwszNamePtr;
			lpwszNamePtr=lpwszPath+1;
		}

		++lpwszPath;
	}

	size_t size = *lpwszNamePtr ? lpwszNamePtr - strPath.data() : lpwszprevNamePtr - strPath.data();
	strPath.resize(size);
	return strPath;
}

string &ReplaceSlashToBSlash(string &strStr)
{
	std::replace(ALL_RANGE(strStr), L'/', L'\\');
	return strStr;
}

const wchar_t *FirstSlash(const wchar_t *String)
{
	do
	{
		if (IsSlash(*String))
			return String;
	}
	while (*String++);

	return nullptr;
}

const wchar_t *LastSlash(const wchar_t *String)
{
	const wchar_t *Start = String;
	String += wcslen(String) + 1;

	while (--String!=Start && !IsSlash(*String))
		;

	return IsSlash(*String)?String:nullptr;
}

bool FindSlash(size_t &Pos, const string &Str, size_t StartPos)
{
	for (size_t p = StartPos; p < Str.size(); p++)
	{
		if (IsSlash(Str[p]))
		{
			Pos = p;
			return true;
		}
	}

	return false;
}

bool FindLastSlash(size_t &Pos, const string &Str)
{
	for (size_t p = Str.size(); p > 0; p--)
	{
		if (IsSlash(Str[p - 1]))
		{
			Pos = p - 1;
			return true;
		}
	}

	return false;
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
		string result(Path.data(), PathRootLen);
		AddEndSlash(result);
		return result;
	}
	else
		return string();
}

string ExtractFileName(const string &Path)
{
	size_t p;

	if (FindLastSlash(p, Path))
		p++;
	else
		p = 0;

	size_t PathRootLen = GetPathRootLength(Path);

	if (p < PathRootLen && PathRootLen)
		return string();

	return string(Path.data() + p, Path.size() - p);
}

string ExtractFilePath(const string &Path)
{
	size_t p;

	if (!FindLastSlash(p, Path))
		p = 0;

	size_t PathRootLen = GetPathRootLength(Path);

	if (p <= PathRootLen && PathRootLen)
	{
		string result(Path.data(), PathRootLen);
		AddEndSlash(result);
		return result;
	}

	return string(Path.data(), p);
}

bool IsRootPath(const string &Path)
{
	size_t PathRootLen = GetPathRootLength(Path);

	if (Path.size() == PathRootLen)
		return true;

	if (Path.size() == PathRootLen + 1 && IsSlash(Path[Path.size() - 1]))
		return true;

	return false;
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
	if (api::QueryDosDevice(DeviceName, TargetPath))
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
    assert(ExtractPathRoot(L"\\\\?\\UNC\\server\\share") == L"\\\\?\\UNC\\server\\share\\");
    assert(ExtractPathRoot(L"\\\\?\\UNC\\server\\share\\") == L"\\\\?\\UNC\\server\\share\\");
    assert(ExtractPathRoot(L"\\\\?\\UNC\\server\\share\\path\\file") == L"\\\\?\\UNC\\server\\share\\");

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
#endif
}
