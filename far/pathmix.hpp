#ifndef PATHMIX_HPP_4A60B3C3_4328_407E_A0E8_F55A9A9BE343
#define PATHMIX_HPP_4A60B3C3_4328_407E_A0E8_F55A9A9BE343
#pragma once

/*
pathmix.hpp

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

class NTPath:public string
{
	void Transform();
public:
	NTPath(const string& Src):string(Src) {Transform();}
};


string KernelPath(const string& NtPath);
string KernelPath(string&& NtPath);

inline bool IsSlash(wchar_t x) { return x==L'\\' || x==L'/'; }

enum PATH_TYPE
{
	PATH_UNKNOWN,
	PATH_DRIVELETTER,
	PATH_DRIVELETTERUNC,
	PATH_REMOTE,
	PATH_REMOTEUNC,
	PATH_VOLUMEGUID,
	PATH_PIPE,
};

PATH_TYPE ParsePath(const string& path, size_t* DirectoryOffset = nullptr, bool* Root = nullptr);

inline bool IsRelativeRoot(const string& Path) { return Path.size() == 1 && IsSlash(Path.front()); }
bool IsAbsolutePath(const string &Path);
bool IsRootPath(const string &Path);
bool HasPathPrefix(const string &Path);
bool PathStartsWith(const string &Path, const string &Start);
bool PathCanHoldRegularFile(const string& Path);
bool IsPluginPrefixPath(const string &Path);
bool CutToSlash(string &strStr, bool bInclude = false); // BUGBUG, deprecated. Use CutToParent.
bool CutToParent(string &strStr);
string &CutToNameUNC(string &strPath);
string &CutToFolderNameIfFolder(string &strPath);
const wchar_t* PointToName(const wchar_t *Path);
const wchar_t* PointToName(const wchar_t *Path,const wchar_t* EndPtr);
const wchar_t* PointToFolderNameIfFolder(const wchar_t *Path);
const wchar_t* PointToExt(const wchar_t *Path);
const wchar_t* PointToExt(const wchar_t *Path,const wchar_t* EndPtr);

inline const wchar_t* PointToName(const string& Path) {return PointToName(Path.data(), Path.data() + Path.size());}
inline const wchar_t* PointToExt(const string& Path) {return PointToExt(Path.data(), Path.data() + Path.size());}


void AddEndSlash(string &strPath, wchar_t TypeSlash);
void AddEndSlash(string &strPath);
bool AddEndSlash(wchar_t *Path, wchar_t TypeSlash);
bool AddEndSlash(wchar_t *Path);
void DeleteEndSlash(wchar_t* Path);
void DeleteEndSlash(string& Path);
inline void ReplaceSlashToBackslash(string &strStr) { std::replace(ALL_RANGE(strStr), L'/', L'\\'); }
inline void ReplaceBackslashToSlash(string &strStr) { std::replace(ALL_RANGE(strStr), L'\\', L'/'); }

bool ContainsSlash(const wchar_t *Str);
size_t FindSlash(const string &Str);
size_t FindLastSlash(const string &Str);

bool TestParentFolderName(const string& Name);
bool TestCurrentDirectory(const string& TestDir);

string ExtractPathRoot(const string &Path);
string ExtractFileName(const string &Path);
string ExtractFilePath(const string &Path);

int MatchNtPathRoot(const string &NtPath, const string& DeviceName);
void TestPathParser();


#endif // PATHMIX_HPP_4A60B3C3_4328_407E_A0E8_F55A9A9BE343
