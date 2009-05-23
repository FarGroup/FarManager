#ifndef __PATHMIX_HPP__
#define __PATHMIX_HPP__
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

void CreatePath(string &strPath);

string& PrepareDiskPath(string &strPath, BOOL CheckFullPath=TRUE);

bool PathPrefix(const wchar_t *Path);
BOOL IsNetworkPath(const wchar_t *Path);
BOOL IsLocalPath(const wchar_t *Path);
BOOL IsLocalRootPath(const wchar_t *Path);
BOOL IsLocalPrefixPath(const wchar_t *Path);
BOOL IsLocalVolumePath(const wchar_t *Path);
BOOL IsLocalVolumeRootPath(const wchar_t *Path);
int PathMayBeAbsolute(const wchar_t *Src);

int _MakePath1(DWORD Key,string &strPathName, const wchar_t *Param2,int ShortNameAsIs=TRUE);

#endif // __PATHMIX_HPP__
