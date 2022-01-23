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
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

enum class filter_area: int;
class FileFilterParams;
class FileListItem;
class VMenu2;
class HierarchicalConfig;

enum class filter_action
{
	include,
	exclude,
	ignore,
};

enum filter_state
{
	has_file_include    = 0_bit,
	has_folder_include  = 1_bit,

	has_include = has_file_include | has_folder_include,
};

struct filter_result
{
	filter_action Action{filter_action::ignore};
	int State{};

	explicit operator bool() const;
};

class multifilter: noncopyable
{
public:
	multifilter(Panel *HostPanel, FAR_FILE_FILTER_TYPE FilterType);

	void UpdateCurrentTime();
	filter_result FileInFilterEx(const os::fs::find_data& fde, string_view FullName = {}) const;
	bool FileInFilter(const os::fs::find_data& fde, string_view FullName = {}) const;
	bool FileInFilter(const FileListItem& fli) const;
	bool FileInFilter(const PluginPanelItem& fd) const;
	bool IsEnabledOnPanel() const;
	filter_area area() const;
	Panel* panel() const;

private:
	bool should_include_folders_by_default(filter_result const& Result) const;

	Panel *m_HostPanel;
	filter_area m_Area;
	os::chrono::time_point CurrentTime;
};

namespace filters
{
	FileFilterParams LoadFilter(/*const*/ HierarchicalConfig& cfg, unsigned long long KeyId);
	void SaveFilter(HierarchicalConfig& cfg, unsigned long long KeyId, const FileFilterParams& Item);

	void InitFilters();
	void SwapPanelFilters();
	void RefreshMasks();
	void Save(bool always);
	void EditFilters(filter_area Area, Panel* HostPanel);
}

#endif // FILEFILTER_HPP_DC322D87_FC69_401A_8EF8_9710B11909CB
