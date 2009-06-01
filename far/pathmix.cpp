/*
pathmix.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "pathmix.hpp"
#include "strmix.hpp"

NTPath::NTPath(LPCWSTR Src)
{
	if(Src&&*Src)
	{
		Str=Src;
		if(!PathPrefix(Src))
		{
			ConvertNameToFull(Str,Str);
			if(!PathPrefix(Str))
			{
				if(IsLocalPath(Str))
					Str=string(L"\\\\?\\")+Str;
				else
					Str=string(L"\\\\?\\UNC\\")+&Str[2];
			}
		}
	}
}

int PathMayBeAbsolute(const wchar_t *Path)
{
	return (Path && ((IsAlpha(*Path) && Path[1]==L':' && IsSlash(Path[2])) || (Path[0]==L'\\' && Path[1]==L'\\')));
}

BOOL IsNetworkPath(const wchar_t *Path)
{
	return Path && ((Path[0] == L'\\' && Path[1] == L'\\' && !PathPrefix(Path))||(PathPrefix(Path) && !StrCmpNI(Path+4,L"UNC\\",4)));
}

BOOL IsLocalPath(const wchar_t *Path)
{
	return (Path && *Path && Path[1]==L':');
}

BOOL IsLocalRootPath(const wchar_t *Path)
{
	return (Path && *Path && Path[1]==L':' && IsSlash(Path[2]) && !Path[3]);
}

bool PathPrefix(const wchar_t *Path)
{
/*
	\\?\
	\\.\
	\??\
*/
	return Path && Path[0] == L'\\' && (Path[1] == L'\\' || Path[1] == L'?') && (Path[2] == L'?' || Path[2] == L'.') && Path[3] == L'\\';
}

BOOL IsLocalPrefixPath(const wchar_t *Path)
{
	return PathPrefix(Path) && Path[4] && Path[5] == L':' && Path[6] == L'\\';
}

BOOL IsLocalVolumePath(const wchar_t *Path)
{
	return PathPrefix(Path) && !wcsnicmp(&Path[4],L"Volume{",7) && Path[47] == L'}';
}

BOOL IsLocalVolumeRootPath(const wchar_t *Path)
{
	return IsLocalVolumePath(Path) && !Path[48];
}

BOOL TestParentFolderName(const wchar_t *Name)
{
	return Name[0] == L'.' && Name[1] == L'.' && (!Name[2] || (IsSlash(Name[2]) && !Name[3]));
}

BOOL TestCurrentFolderName(const wchar_t *Name)
{
	return Name[0] == L'.' && (!Name[1] || (IsSlash(Name[1]) && !Name[2]));
}

const wchar_t* __stdcall PointToName(const wchar_t *lpwszPath)
{
  if ( !lpwszPath )
    return NULL;

  if ( *lpwszPath!=0 && *(lpwszPath+1)==L':' ) lpwszPath+=2;

  const wchar_t *lpwszNamePtr = lpwszPath;
  while ( *lpwszNamePtr ) lpwszNamePtr++;

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


//   јналог PointToName, только дл€ строк типа
//   "name\" (оканчиваетс€ на слеш) возвращает указатель на name, а не на пустую
//   строку
const wchar_t* __stdcall PointToFolderNameIfFolder(const wchar_t *Path)
{
  if(!Path)
    return NULL;

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
  return ((*NamePtr)?NamePtr:prevNamePtr);
}

const wchar_t* PointToExt(const wchar_t *lpwszPath)
{
  if ( !lpwszPath )
    return NULL;

  const wchar_t *lpwszEndPtr = lpwszPath;
  while ( *lpwszEndPtr ) lpwszEndPtr++;
  const wchar_t *lpwszExtPtr = lpwszEndPtr;

  while (lpwszExtPtr != lpwszPath)
  {
    if ( *lpwszExtPtr==L'.' )
    {
      if (IsSlash(*(lpwszExtPtr-1)) || *(lpwszExtPtr-1)==L':' )
        return lpwszEndPtr;
      else
        return lpwszExtPtr;
    }
    if (IsSlash(*lpwszExtPtr) || *lpwszExtPtr==L':' )
      return lpwszEndPtr;
    lpwszExtPtr--;
  }
  return lpwszEndPtr;
}

BOOL AddEndSlash(wchar_t *Path, wchar_t TypeSlash)
{
  BOOL Result=FALSE;
  if (Path)
  {
    /* $ 06.12.2000 IS
      ! “еперь функци€ работает с обоими видами слешей, также происходит
        изменение уже существующего конечного слеша на такой, который
        встречаетс€ чаще.
    */
    wchar_t *end;
    int Slash=0, BackSlash=0;
    if (!TypeSlash)
    {
      end=Path;
      while (*end)
      {
        Slash+=(*end==L'\\');
        BackSlash+=(*end==L'/');
        end++;
      }
    }
    else
    {
      end=Path+StrLength(Path);
      if (TypeSlash == L'\\')
        Slash=1;
      else
        BackSlash=1;
    }
    int Length=(int)(end-Path);
    char c=(Slash<BackSlash)?L'/':L'\\';
    Result=TRUE;
    if (Length==0)
    {
      *end=c;
      end[1]=0;
    }
    else
    {
      end--;
      if (!IsSlash(*end))
      {
        end[1]=c;
        end[2]=0;
      }
      else
      {
        *end=c;
      }
    }
  }
  return Result;
}


BOOL WINAPI AddEndSlash(wchar_t *Path)
{
	return AddEndSlash(Path, 0);
}


BOOL AddEndSlash(string &strPath)
{
	return AddEndSlash(strPath, 0);
}

BOOL AddEndSlash(
		string &strPath,
		wchar_t TypeSlash
		)
{
	wchar_t *lpwszPath = strPath.GetBuffer (strPath.GetLength()+2);

	BOOL Result = AddEndSlash(lpwszPath, TypeSlash);

	strPath.ReleaseBuffer ();

	return Result;
}

BOOL WINAPI DeleteEndSlash (string &strPath,bool allendslash)
{
  BOOL Ret=FALSE;
  if ( !strPath.IsEmpty() )
  {
    size_t len=strPath.GetLength();
    wchar_t *lpwszPath = strPath.GetBuffer ();
		while ( len && IsSlash(lpwszPath[--len]) )
    {
      Ret=TRUE;
      lpwszPath[len] = L'\0';
      if (!allendslash)
        break;
    }
    strPath.ReleaseBuffer();
  }
  return Ret;
}

bool CutToSlash(string &strStr, bool bInclude)
{
  size_t pos;
	bool bFound=LastSlash(strStr,pos);
	if(bFound)
	{
		if ( bInclude )
			strStr.SetLength(pos);
		else
			strStr.SetLength(pos+1);
	}

  return bFound;
}

string& CutToNameUNC(string &strPath)
{
  wchar_t *lpwszPath = strPath.GetBuffer ();

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

  wchar_t *lpwszNamePtr = lpwszPath;

  while ( *lpwszPath )
  {
    if (IsSlash(*lpwszPath) || (*lpwszPath==L':' && lpwszPath == lpwszNamePtr+1) )
      lpwszNamePtr = lpwszPath+1;

    lpwszPath++;
  }

  *lpwszNamePtr = 0;

  strPath.ReleaseBuffer ();

  return strPath;

}

string& CutToFolderNameIfFolder(string &strPath)
{
  wchar_t *lpwszPath = strPath.GetBuffer ();

  wchar_t *lpwszNamePtr=lpwszPath, *lpwszprevNamePtr=lpwszPath;

  while (*lpwszPath)
  {
    if (IsSlash(*lpwszPath) || (*lpwszPath==L':' && lpwszPath==lpwszNamePtr+1))
    {
      lpwszprevNamePtr=lpwszNamePtr;
      lpwszNamePtr=lpwszPath+1;
    }
    ++lpwszPath;
  }

  if (*lpwszNamePtr)
    *lpwszNamePtr=0;
  else
    *lpwszprevNamePtr=0;

  strPath.ReleaseBuffer ();

  return strPath;
}

const wchar_t* PointToNameUNC(const wchar_t *lpwszPath)
{
  if ( !lpwszPath )
    return NULL;

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

  while ( *lpwszPath )
  {
    if (IsSlash(*lpwszPath) || (*lpwszPath==L':' && lpwszPath == lpwszNamePtr+1) )
      lpwszNamePtr = lpwszPath+1;

    lpwszPath++;
  }
  return lpwszNamePtr;
}

string& ReplaceSlashToBSlash(string& strStr)
{
	wchar_t *lpwszStr = strStr.GetBuffer ();

	while ( *lpwszStr )
	{
		if ( *lpwszStr == L'/' )
			*lpwszStr = L'\\';
		lpwszStr++;
	}

	strStr.ReleaseBuffer ();

	return strStr;
}

const wchar_t *FirstSlash(const wchar_t *String)
{
	do
	{
		if(IsSlash(*String))
			return String;
	}
	while (*String++);
	return NULL;
}

bool FirstSlash(const wchar_t *String,size_t &pos)
{
	bool Ret=false;
	const wchar_t *Ptr=FirstSlash(String);
	if(Ptr)
	{
		pos=Ptr-String;
		Ret=true;
	}
	return Ret;
}

const wchar_t *LastSlash(const wchar_t *String)
{
	const wchar_t *Start = String;
	while (*String++)
		;
	while (--String!=Start && !IsSlash(*String))
		;
	return IsSlash(*String)?String:NULL;
}

bool LastSlash(const wchar_t *String,size_t &pos)
{
	bool Ret=false;
	const wchar_t *Ptr=LastSlash(String);
	if(Ptr)
	{
		pos=Ptr-String;
		Ret=true;
	}
	return Ret;
}
