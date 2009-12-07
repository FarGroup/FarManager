#pragma once

/*
pathmix.hpp

Misc functions for processing of path names
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

extern const wchar_t *ReservedFilenameSymbols;

const size_t cVolumeGuidLen = 48;

class NTPath
{
	public:
		string Str;
		NTPath(LPCWSTR Src);

		operator LPCWSTR() const
		{
			return Str;
		}
};

inline int IsSlash(wchar_t x) { return x==L'\\' || x==L'/'; }

bool IsNetworkPath(const wchar_t *Path);
bool IsLocalPath(const wchar_t *Path);
bool IsLocalRootPath(const wchar_t *Path);
bool IsLocalPrefixPath(const wchar_t *Path);
bool IsLocalPrefixRootPath(const wchar_t *Path);
bool IsLocalVolumePath(const wchar_t *Path);
bool IsLocalVolumeRootPath(const wchar_t *Path);
bool IsAbsolutePath(const wchar_t *Path);
bool IsRootPath(const string &Path);
bool HasPathPrefix(const wchar_t *Path);
bool PathStartsWith(const string &Path, const string &Start);
bool PathCanHoldRegularFile(const wchar_t *Path);

bool CutToSlash(string &strStr, bool bInclude = false);
string &CutToNameUNC(string &strPath);
string &CutToFolderNameIfFolder(string &strPath);
const wchar_t *PointToNameUNC(const wchar_t *lpwszPath);
const wchar_t* __stdcall PointToName(const wchar_t *lpwszPath);
const wchar_t* PointToName(string &strPath);
const wchar_t* PointToName(const wchar_t *lpwszPath,const wchar_t *lpwszEndPtr);
const wchar_t* __stdcall PointToFolderNameIfFolder(const wchar_t *lpwszPath);
const wchar_t* PointToExt(const wchar_t *lpwszPath);
const wchar_t* PointToExt(string& strPath);
const wchar_t* PointToExt(const wchar_t *lpwszPath,const wchar_t *lpwszEndPtr);

BOOL AddEndSlash(string &strPath, wchar_t TypeSlash);
BOOL AddEndSlash(string &strPath);
BOOL AddEndSlash(wchar_t *Path, wchar_t TypeSlash);
BOOL WINAPI AddEndSlash(wchar_t *Path);
bool DeleteEndSlash(wchar_t* Path, bool AllEndSlash = false);
BOOL WINAPI DeleteEndSlash(string &strPath,bool AllEndSlash = false);
string &ReplaceSlashToBSlash(string &strStr);

const wchar_t *FirstSlash(const wchar_t *String);
const wchar_t *LastSlash(const wchar_t *String);
bool FindSlash(size_t &Pos, const string &Str, size_t StartPos = 0);
bool FindLastSlash(size_t &Pos, const string &Str);

bool TestParentFolderName(const wchar_t *Name);
bool TestCurrentFolderName(const wchar_t *Name);
bool TestCurrentDirectory(const wchar_t *TestDir);

string ExtractPathRoot(const string &Path);
string ExtractFileName(const string &Path);
string ExtractFilePath(const string &Path);

int MatchNtPathRoot(const string &NtPath, const wchar_t *DeviceName);
