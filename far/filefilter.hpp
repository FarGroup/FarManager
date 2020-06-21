#ifndef FILEFILTER_HPP_DC322D87_FC69_401A_8EF8_9710B11909CB
#define FILEFILTER_HPP_DC322D87_FC69_401A_8EF8_9710B11909CB
#pragma once

/*
filefilter.hpp

Файловый фильтр
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
#include "panelfwd.hpp"
#include "plugin.hpp"

// Platform:
#include "platform.chrono.hpp"
#include "platform.fwd.hpp"

// Common:
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

enum enumFileFilterFlagsType: int;
class FileFilterParams;
class FileListItem;
class VMenu2;
class HierarchicalConfig;

// почему FileInFilter вернул true или false
enum class filter_status
{
	not_in_filter,
	in_include,
	in_exclude,
};


class FileFilter: noncopyable
{
public:
	FileFilter(Panel *HostPanel, FAR_FILE_FILTER_TYPE FilterType);

	void FilterEdit();
	void UpdateCurrentTime();
	bool FileInFilter(const FileListItem* fli, filter_status* FilterStatus = nullptr);
	bool FileInFilter(const os::fs::find_data& fde, filter_status* FilterStatus = nullptr, string_view FullName = {});
	bool FileInFilter(const PluginPanelItem& fd, filter_status* FilterStatus = nullptr);
	bool IsEnabledOnPanel();

	static void InitFilter();
	static FileFilterParams LoadFilter(/*const*/ HierarchicalConfig& cfg, unsigned long long KeyId);
	static void SaveFilter(HierarchicalConfig& cfg, unsigned long long KeyId, const FileFilterParams& Item);
	static void CloseFilter();
	static void SwapFilter();
	static void Save(bool always);

private:
	void ProcessSelection(VMenu2 *FilterList) const;
	enumFileFilterFlagsType GetFFFT() const;
	wchar_t GetCheck(const FileFilterParams& FFP) const;

	Panel *m_HostPanel;
	FAR_FILE_FILTER_TYPE m_FilterType;
	os::chrono::time_point CurrentTime;
};

#endif // FILEFILTER_HPP_DC322D87_FC69_401A_8EF8_9710B11909CB
