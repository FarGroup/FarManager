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

// Internal:
#include "platform.fwd.hpp"

// Platform:

// Common:
#include "common/preprocessor.hpp"
#include "common/string_utils.hpp"
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

class FileListItem;
struct PluginPanelItem;

namespace path
{
	inline constexpr wchar_t separator = L'\\';
	inline constexpr auto separators = L"\\/"sv;

	static_assert(separators.front() == separator);

	constexpr bool is_nt_separator(wchar_t x) noexcept { return x == separator; }
	constexpr bool is_separator(wchar_t x) noexcept { return contains(separators, x); }

	decltype(is_separator)* get_is_separator(string_view Path);

	namespace detail
	{
		class append_arg: public string_view
		{
		public:
			explicit append_arg(string_view Str);
			explicit append_arg(const wchar_t& Char);
		};

		void append_impl(string& Str, const std::initializer_list<append_arg>& Args);
	}

	void append(string& Str, auto const&... Args)
	{
		detail::append_impl(Str, { detail::append_arg(Args)... });
	}

	[[nodiscard]]
	string join(auto&& Arg, const auto&... Args)
	{
		static_assert(sizeof...(Args) > 0);

		string Str(FWD(Arg));
		detail::append_impl(Str, { detail::append_arg(Args)... });
		return Str;
	}

	namespace inplace
	{
		void normalize_separators(string& Path);
	}

	string normalize_separators(string Path);
	string normalize_separators(string_view Path);
}

string nt_path(string_view Path);
string nt_path(string Path);

string kernel_path(string_view NtPath);
string kernel_path(string NtPath);

enum class root_type
{
	unknown,
	drive_letter,
	win32nt_drive_letter,
	remote,
	unc_remote,
	volume,
	unknown_rootlike
};

root_type ParsePath(string_view Path, size_t* RootSize = {}, bool* RootOnly = {});

inline bool IsRelativeRoot(string_view Path) { return Path.size() == 1 && path::is_separator(Path.front()); }
bool IsAbsolutePath(string_view Path);
bool IsRootPath(string_view Path);
bool HasPathPrefix(string_view Path);
string_view ExtractPathPrefix(string_view Path);
bool PathStartsWith(string_view Path, string_view Start);
bool PathCanHoldRegularFile(string_view Path);
std::optional<string_view> GetPluginPrefixPath(string_view Path);
bool CutToSlash(string& Str, bool RemoveSlash = false); // BUGBUG, deprecated. Use CutToParent.
bool CutToSlash(string_view& Str, bool RemoveSlash = false); // BUGBUG, deprecated. Use CutToParent.
bool CutToParent(string_view& Str);
bool CutToParent(string& Str);
[[nodiscard]]
string_view PointToName(string_view Path);
[[nodiscard]]
string_view PointToFolderNameIfFolder(string_view Path);
[[nodiscard]]
std::pair<string_view, string_view> name_ext(string_view Path);

namespace legacy
{
	bool AddEndSlash(wchar_t* Path);

#ifndef NO_WRAPPER
	bool AddEndSlash(char* Path);
#endif

	void DeleteEndSlash(wchar_t* Path);
}

void AddEndSlash(string& Path);
[[nodiscard]] string AddEndSlash(string_view Path);
bool AddEndSlash(wchar_t* Path) = delete;

void DeleteEndSlash(string& Path);
[[nodiscard]] string_view DeleteEndSlash(string_view Path);
void DeleteEndSlash(wchar_t* Path) = delete;

inline void ReplaceSlashToBackslash(string &strStr) { std::ranges::replace(strStr, L'/', L'\\'); }
inline void ReplaceBackslashToSlash(string &strStr) { std::ranges::replace(strStr, L'\\', L'/'); }

bool ContainsSlash(string_view Str);
[[nodiscard]]
size_t FindSlash(string_view Str);
[[nodiscard]]
size_t FindLastSlash(string_view Str);

bool IsParentDirectory(string_view Str);
bool IsParentDirectory(const FileListItem& Data);
bool IsParentDirectory(const os::fs::find_data& Data);
bool IsParentDirectory(const PluginPanelItem& Data);

bool IsCurrentDirectory(string_view Str);

string_view extract_root_device(string_view Path);
string extract_root_directory(string_view Path);
string_view ExtractFileName(string_view Path);
string ExtractFilePath(string_view Path);

string unique_name();

#endif // PATHMIX_HPP_4A60B3C3_4328_407E_A0E8_F55A9A9BE343
