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

#include "platform.fwd.hpp"

class FileListItem;
struct PluginPanelItem;

namespace path
{
	inline auto separators()
	{
		return L"\\/"sv;
	}

	namespace detail
	{
		inline void append_one(string& Str, wchar_t const Arg, size_t const, size_t const Size) { Str.append(&Arg, Size); }
		inline void append_one(string& Str, const wchar_t* const Arg, size_t const Offset, size_t const Size) { Str.append(Arg + Offset, Size); }
		inline void append_one(string& Str, const string& Arg, size_t const Offset, size_t const Size) { Str.append(Arg, Offset, Size); }
		inline void append_one(string& Str, string_view const Arg, size_t const Offset, size_t const Size) { Str.append(Arg.data() + Offset, Size); }

		inline void append_impl(string&, size_t, size_t, const size_t*, const size_t*) {}

		template<typename arg, typename... args>
		void append_impl(string& Str, size_t const Index, size_t const Count, const size_t* const Offsets, const size_t* const Sizes, arg&& Arg, args&&... Args)
		{
			if (!Str.empty() && (*Sizes || Index + 1 == Count))
				Str += separators().front();

			append_one(Str, FWD(Arg), *Offsets, *Sizes);
			append_impl(Str, Index + 1, Count, Offsets + 1, Sizes + 1, FWD(Args)...);
		}

		inline size_t size_one(size_t&, wchar_t const Char)
		{
			return separators().find(Char) == string_view::npos? 1 : 0;
		}

		inline size_t size_one(size_t& Offset, string_view Str)
		{
			const auto Begin = Str.find_first_not_of(separators());
			if (Begin == string_view::npos)
				return 0;

			Str.remove_prefix(Offset = Begin);

			const auto LastCharPos = Str.find_last_not_of(separators());
			if (LastCharPos == string_view::npos)
				return 0;

			Str.remove_suffix(Str.size() - LastCharPos - 1);

			return Str.size();
		}

		inline size_t size_impl(size_t*, size_t*) { return 0; }

		template<typename arg, typename... args>
		size_t size_impl(size_t* const Offsets, size_t* const Sizes, arg&& Arg, args&&... Args)
		{
			*Sizes = size_one(*Offsets, FWD(Arg));
			return *Sizes + size_impl(Offsets + 1, Sizes + 1, FWD(Args)...);
		}
	}

	template<typename... args>
	void append(string& Str, args&&... Args)
	{
		const auto LastCharPos = string_view(Str).find_last_not_of(separators());
		Str.resize(LastCharPos == string::npos? 0 : LastCharPos + 1);

		size_t Sizes[sizeof...(Args)];
		size_t Offsets[sizeof...(Args)]{};
		reserve_exp_noshrink(Str, Str.size() + detail::size_impl(Offsets, Sizes, FWD(Args)...) + sizeof...(Args) - 1);
		detail::append_impl(Str, 0, sizeof...(Args), Offsets, Sizes, FWD(Args)...);
	}

	template<typename arg, typename... args>
	[[nodiscard]]
	string join(arg&& Arg, args&&... Args)
	{
		auto Str = string(FWD(Arg));
		path::append(Str, FWD(Args)...);
		return Str;
	}
}

class NTPath:public string
{
	void Transform();
public:
	NTPath(const string_view Src):
		string(Src)
	{
		Transform();
	}
};


string KernelPath(const string& NtPath);
string KernelPath(string&& NtPath);

inline bool IsSlash(wchar_t x) { return x==L'\\' || x==L'/'; }

enum class root_type
{
	unknown,
	drive_letter,
	unc_drive_letter,
	remote,
	unc_remote,
	volume,
	pipe,
};

root_type ParsePath(string_view Path, size_t* DirectoryOffset = nullptr, bool* Root = nullptr);

inline bool IsRelativeRoot(string_view Path) { return Path.size() == 1 && IsSlash(Path.front()); }
bool IsAbsolutePath(string_view Path);
bool IsRootPath(string_view Path);
bool HasPathPrefix(string_view Path);
bool PathStartsWith(string_view Path, string_view Start);
bool PathCanHoldRegularFile(const string& Path);
bool IsPluginPrefixPath(const string &Path);
bool CutToSlash(string &strStr, bool bInclude = false); // BUGBUG, deprecated. Use CutToParent.
bool CutToParent(string_view& Str);
bool CutToParent(string& Str);
string_view PointToName(string_view Path);
string_view PointToFolderNameIfFolder(string_view Path);
string_view PointToExt(string_view Path);

void AddEndSlash(string &strPath, wchar_t TypeSlash);
void AddEndSlash(string &strPath);
bool AddEndSlash(wchar_t *Path, wchar_t TypeSlash);
bool AddEndSlash(wchar_t *Path);
void DeleteEndSlash(wchar_t* Path);
void DeleteEndSlash(string& Path);
void DeleteEndSlash(string_view& Path);
inline void ReplaceSlashToBackslash(string &strStr) { std::replace(ALL_RANGE(strStr), L'/', L'\\'); }
inline void ReplaceBackslashToSlash(string &strStr) { std::replace(ALL_RANGE(strStr), L'\\', L'/'); }

bool ContainsSlash(string_view Str);
size_t FindSlash(string_view Str);
size_t FindLastSlash(string_view Str);

bool IsParentDirectory(string_view Str);
bool IsParentDirectory(const FileListItem& Data);
bool IsParentDirectory(const os::fs::find_data& Data);
bool IsParentDirectory(const PluginPanelItem& Data);

bool IsCurrentDirectory(string_view Str);

string ExtractPathRoot(string_view Path);
string ExtractFileName(string_view Path);
string ExtractFilePath(string_view Path);

void TestPathParser();


#endif // PATHMIX_HPP_4A60B3C3_4328_407E_A0E8_F55A9A9BE343
