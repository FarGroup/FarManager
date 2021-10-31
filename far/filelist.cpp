/*
filelist.cpp

Файловая панель
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "filelist.hpp"

// Internal:
#include "keyboard.hpp"
#include "flink.hpp"
#include "keys.hpp"
#include "macroopcode.hpp"
#include "ctrlobj.hpp"
#include "filefilter.hpp"
#include "dialog.hpp"
#include "cmdline.hpp"
#include "manager.hpp"
#include "filepanels.hpp"
#include "help.hpp"
#include "fileedit.hpp"
#include "namelist.hpp"
#include "fileview.hpp"
#include "copy.hpp"
#include "history.hpp"
#include "qview.hpp"
#include "preservelongname.hpp"
#include "scrbuf.hpp"
#include "filemasks.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "delete.hpp"
#include "stddlg.hpp"
#include "print.hpp"
#include "mkdir.hpp"
#include "setattr.hpp"
#include "filetype.hpp"
#include "execute.hpp"
#include "fnparce.hpp"
#include "datetime.hpp"
#include "dirinfo.hpp"
#include "pathmix.hpp"
#include "network.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "exitcode.hpp"
#include "panelmix.hpp"
#include "processname.hpp"
#include "mix.hpp"
#include "elevation.hpp"
#include "uuids.far.hpp"
#include "uuids.far.dialogs.hpp"
#include "plugins.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "taskbar.hpp"
#include "fileowner.hpp"
#include "colormix.hpp"
#include "keybar.hpp"
#include "panelctype.hpp"
#include "diskmenu.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "filefilterparams.hpp"
#include "desktop.hpp"
#include "string_sort.hpp"
#include "global.hpp"
#include "log.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/enum_tokens.hpp"
#include "common/rel_ops.hpp"
#include "common/scope_exit.hpp"
#include "common/string_utils.hpp"
#include "common/utility.hpp"
#include "common/view/zip.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static_assert(static_cast<size_t>(panel_sort::BY_USER) == static_cast<size_t>(OPENPANELINFO_SORTMODES::SM_USER));


constexpr auto operator+(panel_sort const Value) noexcept
{
	return std::to_underlying(Value);
}

static const struct
{
	lng Label;
	int MenuPosition;
	far_key_code MenuKey;

	std::initializer_list<std::pair<panel_sort, sort_order>> DefaultLayers;
}
SortModes[]
{
	{ lng::MMenuUnsorted,             5,  KEY_CTRLF7,  {  { panel_sort::UNSORTED,         sort_order::ascend,  }, }, },
	{ lng::MMenuSortByName,           0,  KEY_CTRLF3,  {  { panel_sort::BY_NAME,          sort_order::ascend,  }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByExt,            2,  KEY_CTRLF4,  {  { panel_sort::BY_EXT,           sort_order::ascend,  }, { panel_sort::BY_NAMEONLY, sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByWrite,          3,  KEY_CTRLF5,  {  { panel_sort::BY_MTIME,         sort_order::descend, }, { panel_sort::BY_NAME,     sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByCreation,       6,  KEY_CTRLF8,  {  { panel_sort::BY_CTIME,         sort_order::descend, }, { panel_sort::BY_NAME,     sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByAccess,         7,  KEY_CTRLF9,  {  { panel_sort::BY_ATIME,         sort_order::descend, }, { panel_sort::BY_NAME,     sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortBySize,           4,  KEY_CTRLF6,  {  { panel_sort::BY_SIZE,          sort_order::descend, }, { panel_sort::BY_NAME,     sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByDiz,            9,  KEY_CTRLF10, {  { panel_sort::BY_DIZ,           sort_order::ascend,  }, { panel_sort::BY_NAME,     sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByOwner,          10, KEY_CTRLF11, {  { panel_sort::BY_OWNER,         sort_order::ascend,  }, { panel_sort::BY_NAME,     sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByAllocatedSize,  11, NO_KEY,      {  { panel_sort::BY_COMPRESSEDSIZE,sort_order::descend, }, { panel_sort::BY_NAME,     sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByNumLinks,       12, NO_KEY,      {  { panel_sort::BY_NUMLINKS,      sort_order::descend, }, { panel_sort::BY_NAME,     sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByNumStreams,     13, NO_KEY,      {  { panel_sort::BY_NUMSTREAMS,    sort_order::descend, }, { panel_sort::BY_NAME,     sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByStreamsSize,    14, NO_KEY,      {  { panel_sort::BY_STREAMSSIZE,   sort_order::descend, }, { panel_sort::BY_NAME,     sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByNameOnly,       1,  NO_KEY,      {  { panel_sort::BY_NAMEONLY,      sort_order::ascend,  }, { panel_sort::BY_EXT,      sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
	{ lng::MMenuSortByChange,         8,  NO_KEY,      {  { panel_sort::BY_CHTIME,        sort_order::descend, }, { panel_sort::BY_NAME,     sort_order::ascend }, { panel_sort::UNSORTED, sort_order::ascend }, }, },
};

static_assert(std::size(SortModes) == static_cast<size_t>(panel_sort::COUNT));

static constexpr auto order_indicator(sort_order const Order)
{
	switch (Order)
	{
	default:
	case sort_order::keep:    return L'=';
	case sort_order::ascend:  return L'▲';
	case sort_order::descend: return L'▼';
	}
}

span<std::pair<panel_sort, sort_order> const> default_sort_layers(panel_sort const SortMode)
{
	return SortModes[static_cast<size_t>(SortMode)].DefaultLayers;
}

template<typename T>
auto compare_numbers(T const First, T const Second)
{
	return First < Second? -1 : First != Second;
}

static auto compare_time(os::chrono::time_point First, os::chrono::time_point Second)
{
	return compare_numbers(First, Second);
}

// FAT Last Write time is rounded up to the even number of seconds, e.g. 2s 1ms -> 4s
static auto to_fat_write_time(os::chrono::time_point Point)
{
	return (Point.time_since_epoch() + 2s - 1ns) / 2s;
}

// However, people also use this function with FTP, which rounds down to whole seconds
static auto to_whole_seconds(os::chrono::time_point Point)
{
	return Point.time_since_epoch() / 1s;
}

// The ultimate question here is "can these times be considered 'equal'",
// so we take an opportunistic approach and try both methods:
static auto compare_fat_write_time(os::chrono::time_point First, os::chrono::time_point Second)
{
	if (!compare_numbers(to_fat_write_time(First), to_fat_write_time(Second)))
		return 0;

	if (!compare_numbers(to_whole_seconds(First), to_whole_seconds(Second)))
		return 0;

	return compare_time(First, Second);
}


enum SELECT_MODES
{
	SELECT_INVERT,
	SELECT_INVERTALL,
	SELECT_INVERTFILES,
	SELECT_ADD,
	SELECT_REMOVE,
	SELECT_ADDEXT,
	SELECT_REMOVEEXT,
	SELECT_ADDNAME,
	SELECT_REMOVENAME,
	SELECT_ADDMASK,
	SELECT_REMOVEMASK,
	SELECT_INVERTMASK,
};

namespace custom_sort
{
	static FileList* FileListPtr;

static void FileListToSortingPanelItem(const FileListItem *arr, int index, SortingPanelItem* ppi)
{
	const auto& fi = arr[index];
	auto& pi = *ppi;

	pi.FileName = fi.FileName.c_str();                   //! CHANGED
	pi.AlternateFileName = fi.AlternateFileName().c_str(); //! CHANGED
	pi.FileSize=fi.FileSize;
	pi.AllocationSize=fi.AllocationSize;
	pi.FileAttributes=fi.Attributes;
	pi.LastWriteTime = os::chrono::nt_clock::to_filetime(fi.LastWriteTime);
	pi.CreationTime = os::chrono::nt_clock::to_filetime(fi.CreationTime);
	pi.LastAccessTime = os::chrono::nt_clock::to_filetime(fi.LastAccessTime);
	pi.ChangeTime = os::chrono::nt_clock::to_filetime(fi.ChangeTime);
	pi.Flags=fi.UserFlags;

	if (fi.Selected)
		pi.Flags|=PPIF_SELECTED;

	pi.CustomColumnData=fi.CustomColumns.data();
	pi.CustomColumnNumber=fi.CustomColumns.size();

	pi.Description=fi.DizText; //BUGBUG???

	pi.UserData = fi.UserData;

	pi.CRC32=fi.CRC32;
	pi.Position=fi.Position;                        //! CHANGED
	pi.SortGroup=fi.SortGroup - DEFAULT_SORT_GROUP; //! CHANGED

	pi.NumberOfLinks = fi.IsNumberOfLinksRead() || FileListPtr->IsColumnDisplayed(column_type::links_number)?fi.NumberOfLinks(FileListPtr) : 0;
	pi.Owner = fi.IsOwnerRead() || FileListPtr->IsColumnDisplayed(column_type::owner)? EmptyToNull(fi.Owner(FileListPtr)) : nullptr;
	pi.NumberOfStreams = fi.IsNumberOfStreamsRead() || FileListPtr->IsColumnDisplayed(column_type::streams_number)? fi.NumberOfStreams(FileListPtr) : 0;
	pi.StreamsSize = fi.IsStreamsSizeRead() || FileListPtr->IsColumnDisplayed(column_type::streams_size)? fi.StreamsSize(FileListPtr) : 0;
}

struct CustomSort
{
	unsigned int        *Positions;
	const FileListItem  *Items;
	size_t               ItemsCount;
	void(*FileListToSortingPanelItem)(const FileListItem*, int, SortingPanelItem*);
	int                  ListSortGroups;
	int                  ListSelectedFirst;
	int                  ListDirectoriesFirst;
	int                  ListSortMode;
	int                  RevertSorting;
	int                  Reserved[2];
	HANDLE               hSortPlugin;
};

static bool SortFileList(CustomSort* cs, wchar_t* indicator)
{
	FarMacroValue values[]={cs};
	FarMacroCall fmc={sizeof(FarMacroCall),std::size(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_PANELSORT,&fmc};
	void *ptr;

	if (!Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Luamacro.Id, OPEN_LUAMACRO, &info, &ptr) || !ptr)
		return false;

	indicator[0] = info.Ret.Values[0].String[0];
	indicator[1] = info.Ret.Values[0].String[1];
	return true;
}

static bool CanSort(int SortMode)
{
	FarMacroValue values[] = {static_cast<double>(SortMode)};
	FarMacroCall fmc = {sizeof(FarMacroCall),std::size(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info = {MCT_CANPANELSORT,&fmc};
	void *ptr;

	return Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Luamacro.Id, OPEN_LUAMACRO, &info, &ptr) && ptr;
}

}

struct FileList::PluginsListItem
{
	NONCOPYABLE(PluginsListItem);
	MOVE_CONSTRUCTIBLE(PluginsListItem);

	PluginsListItem(std::unique_ptr<plugin_panel>&& hPlugin, string_view const HostFile, bool Modified, int PrevViewMode, panel_sort PrevSortMode, bool PrevSortOrder, bool PrevDirectoriesFirst, const PanelViewSettings& PrevViewSettings):
		m_Plugin(std::move(hPlugin)),
		m_HostFile(HostFile),
		m_Modified(Modified),
		m_PrevViewMode(PrevViewMode),
		m_PrevSortMode(PrevSortMode),
		m_PrevSortOrder(PrevSortOrder),
		m_PrevDirectoriesFirst(PrevDirectoriesFirst),
		m_PrevViewSettings(PrevViewSettings.clone())
	{
	}

	std::unique_ptr<plugin_panel> m_Plugin;
	string m_HostFile;
	bool m_Modified;
	int m_PrevViewMode;
	panel_sort m_PrevSortMode;
	bool m_PrevSortOrder;
	bool m_PrevDirectoriesFirst;
	PanelViewSettings m_PrevViewSettings;
};

FileListItem::FileListItem()
{
	m_Owner = values::uninitialised(wchar_t());
}

static string GetItemFullName(const FileListItem& Item, const FileList* Owner)
{
	return path::join(Owner->GetCurDir(), IsParentDirectory(Item)? string{} : Item.FileName);
}

bool FileListItem::IsNumberOfLinksRead() const
{
	return m_NumberOfLinks != values::uninitialised(m_NumberOfLinks);
}

DWORD FileListItem::NumberOfLinks(const FileList* Owner) const
{
	if (IsNumberOfLinksRead())
		return m_NumberOfLinks;

	if (Attributes & FILE_ATTRIBUTE_DIRECTORY || !Owner->HardlinksSupported())
	{
		m_NumberOfLinks = 1;
	}
	else
	{
		SCOPED_ACTION(elevation::suppress);
		const auto Hardlinks = GetNumberOfLinks(GetItemFullName(*this, Owner));
		static_assert(std::is_same_v<decltype(m_NumberOfLinks), DWORD>);
		m_NumberOfLinks = Hardlinks? static_cast<DWORD>(*Hardlinks) : values::unknown(m_NumberOfLinks);
	}

	return m_NumberOfLinks;
}

static void GetStreamsCountAndSize(const FileList* Owner, const FileListItem& Item, unsigned long long& StreamsSize, DWORD& NumberOfStreams, bool Supported)
{
	if (!Supported)
	{
		StreamsSize = Item.FileSize;
		NumberOfStreams = 1;
	}
	else
	{
		SCOPED_ACTION(elevation::suppress);

		size_t StreamsCount = 0;
		if (EnumStreams(GetItemFullName(Item, Owner), StreamsSize, StreamsCount))
		{
			static_assert(std::is_same_v<decltype(NumberOfStreams), DWORD&>);
			NumberOfStreams = static_cast<DWORD>(StreamsCount);
		}
		else
		{
			StreamsSize = FileListItem::values::unknown(StreamsSize);
			NumberOfStreams = FileListItem::values::unknown(NumberOfStreams);
		}
	}
}

bool FileListItem::IsNumberOfStreamsRead() const
{
	return m_NumberOfStreams != values::uninitialised(m_NumberOfStreams);
}

DWORD FileListItem::NumberOfStreams(const FileList* Owner) const
{
	if (IsNumberOfStreamsRead())
		return m_NumberOfStreams;

	GetStreamsCountAndSize(Owner, *this, m_StreamsSize, m_NumberOfStreams, Owner->StreamsSupported());

	return m_NumberOfStreams;
}

bool FileListItem::IsStreamsSizeRead() const
{
	return m_StreamsSize != values::uninitialised(m_StreamsSize);
}

unsigned long long FileListItem::StreamsSize(const FileList* Owner) const
{
	if (IsStreamsSizeRead())
		return m_StreamsSize;

	GetStreamsCountAndSize(Owner, *this, m_StreamsSize, m_NumberOfStreams, Owner->StreamsSupported());

	return m_StreamsSize;
}

bool FileListItem::IsOwnerRead() const
{
	return !(m_Owner.size() == 1 && m_Owner.front() == values::uninitialised(wchar_t()));
}

const string& FileListItem::Owner(const FileList* Owner) const
{
	if (IsOwnerRead())
		return m_Owner;

	if (Owner->GetMode() == panel_mode::NORMAL_PANEL)
	{
		SCOPED_ACTION(elevation::suppress);

		if (!GetFileOwner(Owner->GetComputerName(), GetItemFullName(*this, Owner), m_Owner))
		{
			// One try is enough
			m_Owner.clear();
		}
	}
	else
	{
		m_Owner.clear();
	}

	return m_Owner;
}

bool FileListItem::IsContentDataRead() const
{
	return m_ContentData != nullptr; // bad
}

const std::unique_ptr<content_data>& FileListItem::ContentData(const FileList* Owner) const
{
	if (IsContentDataRead())
		return m_ContentData;

	m_ContentData = Owner->GetContentData(GetItemFullName(*this, Owner));

	return m_ContentData;
}

const string& FileListItem::AlternateOrNormal(bool Alternate) const
{
	return Alternate? AlternateFileName() : FileName;
}

struct FileList::PrevDataItem
{
	NONCOPYABLE(PrevDataItem);
	MOVE_CONSTRUCTIBLE(PrevDataItem);

	PrevDataItem(string rhsPrevName, list_data&& rhsPrevListData, int rhsPrevTopFile):
		strPrevName(std::move(rhsPrevName)),
		PrevListData(std::move(rhsPrevListData)),
		PrevTopFile(rhsPrevTopFile)
	{
	}

	string strPrevName;
	list_data PrevListData;
	int PrevTopFile;
};

file_panel_ptr FileList::create(window_ptr Owner)
{
	return std::make_shared<FileList>(private_tag(), Owner);
}

FileList::FileList(private_tag, window_ptr Owner):
	Panel(std::move(Owner)),
	m_BackgroundUpdater(std::make_unique<background_updater>(this))
{
	if (const auto& data = msg(lng::MPanelBracketsForLongName); data.size() > 1)
	{
		*openBracket = data[0];
		*closeBracket = data[1];
	}

	m_CurDir = os::fs::GetCurrentDirectory();
	strOriginalCurDir = m_CurDir;
	m_SortMode = panel_sort::BY_NAME;
	m_ViewSettings = Global->Opt->ViewSettings[m_ViewMode].clone();
	PreparePanelView();
}


FileList::~FileList()
{
	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		while (PopPlugin(FALSE))
			;

	FileList::StopFSWatcher();

	FileList::ClearAllItem();

	m_ListData.clear();
}


FileList::list_data& FileList::list_data::operator=(FileList::list_data&& rhs) noexcept
{
	clear();

	Items = std::move(rhs.Items);
	rhs.Items.clear();

	m_Plugin = std::move(rhs.m_Plugin);
	rhs.m_Plugin = {};

	return *this;
}

void FileList::list_data::clear()
{
	for (auto& i: Items)
	{
		if (m_Plugin)
		{
			FreePluginPanelItemUserData(m_Plugin, i.UserData);
			if (i.DeleteDiz)
				delete[] i.DizText;
		}

		for (const auto& Column: i.CustomColumns)
		{
			delete[] Column;
		}

		i.CustomColumns.clear();
	}

	Items.clear();
	m_Plugin = nullptr;
}

void FileList::ToBegin()
{
	m_CurFile = 0;
	ShowFileList();
}


void FileList::ToEnd()
{
	m_CurFile = m_ListData.empty()? 0 : static_cast<int>(m_ListData.size() - 1);
	ShowFileList();
}

void FileList::MoveCursor(int offset)
{
	m_CurFile = m_ListData.empty()? 0 : std::clamp(m_CurFile + offset, 0, static_cast<int>(m_ListData.size() - 1));
}

void FileList::MoveCursorAndShow(int offset)
{
	MoveCursor(offset);
	ShowFileList();
}

void FileList::Scroll(int offset)
{
	m_CurTopFile += offset;
	MoveCursorAndShow(offset);
}

void FileList::CorrectPosition()
{
	if (m_ListData.empty())
	{
		m_CurFile=m_CurTopFile=0;
		return;
	}

	if (m_CurTopFile+m_Stripes*m_Height > static_cast<int>(m_ListData.size()))
		m_CurTopFile = static_cast<int>(m_ListData.size() - m_Stripes * m_Height);

	if (m_CurFile<0)
		m_CurFile=0;

	if (m_CurFile > static_cast<int>(m_ListData.size() - 1))
		m_CurFile = static_cast<int>(m_ListData.size() - 1);

	if (m_CurTopFile<0)
		m_CurTopFile=0;

	if (m_CurTopFile > static_cast<int>(m_ListData.size() - 1))
		m_CurTopFile = static_cast<int>(m_ListData.size() - 1);

	if (m_CurFile<m_CurTopFile)
		m_CurTopFile=m_CurFile;

	if (m_CurFile>m_CurTopFile+m_Stripes*m_Height-1)
		m_CurTopFile=m_CurFile-m_Stripes*m_Height+1;
}

class list_less
{
public:
	explicit list_less(const FileList* Owner, const plugin_panel* SortPlugin, bool const IgnorePaths):
		m_Owner(Owner),
		m_ListSortMode(Owner->GetSortMode()),
		m_ListPanelMode(Owner->GetMode()),
		m_SortPlugin(SortPlugin),
		m_SortLayers(Global->Opt->PanelSortLayers[static_cast<size_t>(m_ListSortMode)]),
		m_Reverse(Owner->GetSortOrder()),
		m_ListSortGroups(Owner->GetSortGroups()),
		m_ListSelectedFirst(Owner->GetSelectedFirstMode()),
		m_ListDirectoriesFirst(Owner->GetDirectoriesFirst()),
		m_SortFolderExt(Global->Opt->SortFolderExt),
		m_IgnorePaths(IgnorePaths)
	{
	}

	bool operator()(const FileListItem& Item1, const FileListItem& Item2) const
	{
		const auto IsParentDirItem1 = IsParentDirectory(Item1);
		const auto IsParentDirItem2 = IsParentDirectory(Item2);

		if (IsParentDirItem1 && IsParentDirItem2)
			return Item1.Position < Item2.Position;

		if (IsParentDirItem1)
			return true;

		if (IsParentDirItem2)
			return false;

		if (m_ListDirectoriesFirst)
		{
			const auto IsDirItem1 = (Item1.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			const auto IsDirItem2 = (Item2.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

			if (IsDirItem1 != IsDirItem2)
				return IsDirItem1;
		}

		if (m_ListSelectedFirst && Item1.Selected != Item2.Selected)
			return Item1.Selected;

		if (m_ListSortGroups && Item1.SortGroup != Item2.SortGroup &&
			(
				m_ListSortMode == panel_sort::BY_NAME ||
				m_ListSortMode == panel_sort::BY_EXT ||
				m_ListSortMode == panel_sort::BY_NAMEONLY
			)
		)
			return Item1.SortGroup < Item2.SortGroup;

		if (m_SortPlugin && m_ListSortMode != panel_sort::UNSORTED)
		{
			const auto& [a, b] = m_Reverse? std::tie(Item2, Item1) : std::tie(Item1, Item2);

			// Direct access, no copy. It's its own panel anyways.
			PluginPanelItemHolderRef pi1, pi2;
			m_Owner->FileListToPluginItem(a, pi1);
			m_Owner->FileListToPluginItem(b, pi2);
			if (const auto Result = Global->CtrlObject->Plugins->Compare(m_SortPlugin, &pi1.Item, &pi2.Item, internal_sort_mode_to_plugin(m_ListSortMode)))
			{
				if (Result != -2)
					return Result < 0;
			}
		}

		for (const auto& [ModeValue, Order]: m_SortLayers)
		{
			const auto LayerSort = static_cast<panel_sort>(ModeValue);
			const auto Reverse = LayerSort == m_ListSortMode || Order == sort_order::keep? m_Reverse : Order == sort_order::descend;

			if (const auto Result = compare(LayerSort, Reverse, Item1, Item2))
				return Result < 0;
		}

		return false;
	}

private:
	int compare(panel_sort const SortMode, bool const Reverse, FileListItem const& Item1, FileListItem const& Item2) const
	{
		const auto& [a, b] = Reverse? std::tie(Item2, Item1) : std::tie(Item1, Item2);

		const auto ignore_path_opt = [&](string_view const FullName)
		{
			return m_IgnorePaths? PointToName(FullName) : FullName;
		};

		const auto name_ext_opt = [SortFolderExt = m_SortFolderExt](FileListItem const& i)
		{
			return SortFolderExt || !(i.Attributes & FILE_ATTRIBUTE_DIRECTORY)?
				name_ext(i.FileName) :
				std::pair(string_view(i.FileName), L""sv);
		};

		switch (SortMode)
		{
		case panel_sort::UNSORTED:
			return compare_numbers(a.Position, b.Position);

		case panel_sort::BY_NAME:
			return string_sort::compare(ignore_path_opt(a.FileName), ignore_path_opt(b.FileName));

		case panel_sort::BY_NAMEONLY:
			return string_sort::compare(ignore_path_opt(name_ext_opt(a).first), ignore_path_opt(name_ext_opt(b).first));

		case panel_sort::BY_EXT:
			return string_sort::compare(name_ext_opt(a).second, name_ext_opt(b).second);

		case panel_sort::BY_MTIME:
			return compare_time(a.LastWriteTime, b.LastWriteTime);

		case panel_sort::BY_CTIME:
			return compare_time(a.CreationTime, b.CreationTime);

		case panel_sort::BY_ATIME:
			return compare_time(a.LastAccessTime, b.LastAccessTime);

		case panel_sort::BY_CHTIME:
			return compare_time(a.ChangeTime, b.ChangeTime);

		case panel_sort::BY_SIZE:
			return compare_numbers(a.FileSize, b.FileSize);

		case panel_sort::BY_DIZ:
			return string_sort::compare(NullToEmpty(a.DizText), NullToEmpty(b.DizText));

		case panel_sort::BY_OWNER:
			return string_sort::compare(a.Owner(m_Owner), b.Owner(m_Owner));

		case panel_sort::BY_COMPRESSEDSIZE:
			return compare_numbers(a.AllocationSize, b.AllocationSize);

		case panel_sort::BY_NUMLINKS:
			return compare_numbers(a.NumberOfLinks(m_Owner), b.NumberOfLinks(m_Owner));

		case panel_sort::BY_NUMSTREAMS:
			return compare_numbers(a.NumberOfStreams(m_Owner), b.NumberOfStreams(m_Owner));

		case panel_sort::BY_STREAMSSIZE:
			return compare_numbers(a.StreamsSize(m_Owner), b.StreamsSize(m_Owner));

		default:
			assert(false);
			UNREACHABLE;
		}
	}

	const FileList* const m_Owner;
	const panel_sort m_ListSortMode;
	const panel_mode m_ListPanelMode;
	const plugin_panel* m_SortPlugin;
	const span<std::pair<panel_sort, sort_order>> m_SortLayers;
	const bool m_Reverse;
	bool m_ListSortGroups;
	bool m_ListSelectedFirst;
	bool m_ListDirectoriesFirst;
	bool m_SortFolderExt;
	bool m_IgnorePaths;
};


void FileList::SortFileList(bool KeepPosition)
{
	if (m_ListData.empty() || m_InsideGetFindData)
		return;

	string strCurName;

	if (m_SortMode == panel_sort::BY_DIZ)
		ReadDiz();

	if (KeepPosition)
	{
		assert(m_CurFile < static_cast<int>(m_ListData.size()));
		strCurName = m_ListData[m_CurFile].FileName;
	}

	const auto PluginPanel = GetPluginHandle();
	const auto hSortPlugin = (m_PanelMode == panel_mode::PLUGIN_PANEL && PluginPanel && PluginPanel->plugin()->has(iCompare))? PluginPanel : nullptr;

	// ЭТО ЕСТЬ УЗКОЕ МЕСТО ДЛЯ СКОРОСТНЫХ ХАРАКТЕРИСТИК Far Manager
	// при считывании директории

	if (m_SortMode < panel_sort::COUNT)
	{
		const auto NameColumn = std::find_if(ALL_CONST_RANGE(m_ViewSettings.PanelColumns), [](column const& i){ return i.type == column_type::name; });
		const auto IgnorePaths = NameColumn != m_ViewSettings.PanelColumns.cend() && NameColumn->type_flags & COLFLAGS_NAMEONLY;

		list_less const Predicate(this, hSortPlugin, IgnorePaths);

		const auto& SortLayers = Global->Opt->PanelSortLayers[static_cast<size_t>(m_SortMode)];

		if (std::any_of(ALL_CONST_RANGE(SortLayers), [](std::pair<panel_sort, sort_order> const& Layer){ return Layer.first == panel_sort::UNSORTED; }))
		{
			// Unsorted criterion is deterministic and won't report equality, thus ensuring stability
			std::sort(ALL_RANGE(m_ListData), Predicate);
		}
		else
		{
			std::stable_sort(ALL_RANGE(m_ListData), Predicate);
		}
	}
	else if (m_SortMode >= panel_sort::BY_USER)
	{
		custom_sort::CustomSort cs{};
		custom_sort::FileListPtr = this;
		std::vector<unsigned int> Positions(m_ListData.size());
		std::iota(ALL_RANGE(Positions), 0);
		cs.Positions = Positions.data();
		cs.Items = m_ListData.data();
		cs.ItemsCount = m_ListData.size();
		cs.FileListToSortingPanelItem = custom_sort::FileListToSortingPanelItem;
		cs.ListSortGroups = m_SortGroups;
		cs.ListSelectedFirst = SelectedFirst;
		cs.ListDirectoriesFirst = m_DirectoriesFirst;
		cs.ListSortMode = static_cast<int>(m_SortMode);
		cs.RevertSorting = m_ReverseSortOrder;
		cs.hSortPlugin = hSortPlugin;

		if (custom_sort::SortFileList(&cs, CustomSortIndicator))
		{
			apply_permutation(ALL_RANGE(m_ListData), Positions.begin());
		}
		else
		{
			SetSortMode(panel_sort::BY_NAME); // recursive call
			return;
		}
	}
	else
	{
		LOGWARNING(L"Unknown sort mode {}"sv, m_SortMode);
	}

	if (KeepPosition)
		GoToFile(strCurName);
}

bool FileList::SendKeyToPlugin(DWORD Key, bool Pred)
{
	if (m_PanelMode != panel_mode::PLUGIN_PANEL)
		return false;

	const auto MacroState = Global->CtrlObject->Macro.GetState();
	if (MacroState != MACROSTATE_RECORDING_COMMON && MacroState != MACROSTATE_EXECUTING_COMMON && MacroState != MACROSTATE_NOMACRO)
		return false;

	INPUT_RECORD rec;
	KeyToInputRecord(Key, &rec);
	const auto ProcessCode = Global->CtrlObject->Plugins->ProcessKey(GetPluginHandle(), &rec, Pred);

	return ProcessCode != 0;
}

bool FileList::GetPluginInfo(PluginInfo *PInfo) const
{
	const auto PluginPanel = GetPluginHandle();
	if (GetMode() != panel_mode::PLUGIN_PANEL || !PluginPanel || !PluginPanel->plugin())
		return false;

	PInfo->StructSize = sizeof(PluginInfo);
	return Global->CtrlObject->Plugins->GetPluginInfo(PluginPanel->plugin(), PInfo);
}

long long FileList::VMProcess(int OpCode,void *vParam,long long iParam)
{
	switch (OpCode)
	{
	case MCODE_C_ROOTFOLDER:
		if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		{
			Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
			return !m_CachedOpenPanelInfo.CurDir || !*m_CachedOpenPanelInfo.CurDir;
		}

		return IsRootPath(m_CurDir)? 1 : equal_icase(m_CurDir, GetPathRoot(m_CurDir));

	case MCODE_C_EOF:
		return m_CurFile == static_cast<int>(m_ListData.size() - 1);

	case MCODE_C_BOF:
		return !m_CurFile;

	case MCODE_C_SELECTED:
		return GetRealSelCount() != 0;

	case MCODE_V_ITEMCOUNT:
		return m_ListData.size();

	case MCODE_V_CURPOS:
		return m_CurFile + 1;

	case MCODE_C_APANEL_FILTER:
		return m_Filter && m_Filter->IsEnabledOnPanel();

	case MCODE_V_APANEL_PREFIX:           // APanel.Prefix
	case MCODE_V_PPANEL_PREFIX:           // PPanel.Prefix
		{
			const auto PInfo = static_cast<PluginInfo*>(vParam);
			const auto PluginPanel = GetPluginHandle();
			if (GetMode() == panel_mode::PLUGIN_PANEL && PluginPanel && PluginPanel->plugin())
				return Global->CtrlObject->Plugins->GetPluginInfo(PluginPanel->plugin(), PInfo)?1:0;
			return 0;
		}

	case MCODE_V_APANEL_FORMAT:           // APanel.Format
	case MCODE_V_PPANEL_FORMAT:           // PPanel.Format
		{
			const auto PluginPanel = GetPluginHandle();
			if (GetMode() == panel_mode::PLUGIN_PANEL && PluginPanel)
			{

				Global->CtrlObject->Plugins->GetOpenPanelInfo(PluginPanel, &m_CachedOpenPanelInfo);
				*static_cast<OpenPanelInfo*>(vParam) = m_CachedOpenPanelInfo;
				return 1;
			}
			return 0;
		}

	case MCODE_F_PANEL_SELECT:
		{
			long long Result=-1;
			const auto mps = static_cast<const MacroPanelSelect*>(vParam);

			if (m_ListData.empty())
				return Result;

			if (mps->Mode == 1 && static_cast<size_t>(mps->Index) >= m_ListData.size())
				return Result;

			const auto ApplyToList = [&](const auto& Selector)
			{
				for (const auto& i: enum_tokens_with_quotes(mps->Item, L"\r\n"sv))
				{
					if (i.empty())
						continue;

					const auto Pos = FindFile(PointToName(i), true);
					if (Pos == -1)
						continue;

					Selector(Pos);
					Result++;
				}
			};

			enum class ps_action
			{
				remove,
				add,
				invert,
				restore,
			};

			enum class ps_mode
			{
				all,
				position,
				list_names,
				list_masks,
			};

			switch (static_cast<ps_action>(mps->Action))
			{
			case ps_action::remove:
				switch(static_cast<ps_mode>(mps->Mode))
				{
				case ps_mode::all:
					SaveSelection();
					Result=GetRealSelCount();
					ClearSelection();
					break;

				case ps_mode::position:
					SaveSelection();
					Result=1;
					Select(m_ListData[mps->Index], false);
					break;

				case ps_mode::list_names:
					SaveSelection();
					Result=0;
					ApplyToList([&](size_t Pos){ Select(m_ListData[Pos], false); });
					break;

				case ps_mode::list_masks:
					SaveSelection();
					Result = SelectFiles(SELECT_REMOVEMASK, mps->Item);
					break;
				}
				break;

			case ps_action::add:
				switch(static_cast<ps_mode>(mps->Mode))
				{
				case ps_mode::all:
					SaveSelection();

					for (auto& i: m_ListData)
					{
						Select(i, true);
					}

					Result=GetRealSelCount();
					break;

				case ps_mode::position:
					SaveSelection();
					Result=1;
					Select(m_ListData[mps->Index], true);
					break;

				case ps_mode::list_names:
					SaveSelection();
					Result=0;
					ApplyToList([&](size_t Pos) { Select(m_ListData[Pos], true); });
					break;

				case ps_mode::list_masks:
					SaveSelection();
					Result = SelectFiles(SELECT_ADDMASK, mps->Item);
					break;
				}
				break;

			case ps_action::invert:
				switch(static_cast<ps_mode>(mps->Mode))
				{
				case ps_mode::all:
					SaveSelection();

					for (auto& i: m_ListData)
					{
						Select(i, !i.Selected);
					}

					Result=GetRealSelCount();
					break;

				case ps_mode::position:
					SaveSelection();
					Result=1;
					Select(m_ListData[mps->Index], !m_ListData[mps->Index].Selected);
					break;

				case ps_mode::list_names:
					SaveSelection();
					Result=0;
					ApplyToList([&](size_t Pos) { Select(m_ListData[Pos], !m_ListData[Pos].Selected); });
					break;

				case ps_mode::list_masks:
					SaveSelection();
					Result = SelectFiles(SELECT_INVERTMASK, mps->Item);
					break;
				}
				break;

			case ps_action::restore:
				RestoreSelection();
				Result = GetRealSelCount();
				break;
			}

			if (Result != -1 && mps->Action != 3)
			{
				if (SelectedFirst)
					SortFileList(true);
				Redraw();
			}

			return Result;
		}
	}

	return 0;
}

class file_state: public rel_ops<file_state>
{
public:
	static auto get(string_view const Filename)
	{
		file_state State;
		State.IsValid = os::fs::GetFileTimeSimple(Filename, nullptr, nullptr, &State.Times.first, &State.Times.second);
		return State;
	}

	bool operator==(const file_state& rhs) const
	{
		// Invalid times are considered different
		return IsValid && rhs.IsValid && Times == rhs.Times;
	}

private:
	// TODO: Check the file size too?
	std::pair<os::chrono::time_point, os::chrono::time_point> Times;
	bool IsValid{};
};

// This function is ~1500 lines long /o
bool FileList::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key();
	elevation::instance().ResetApprove();

	int N;
	const auto IsEmptyCmdline = Parent()->GetCmdLine()->GetString().empty();

	if (IsVisible())
	{
		if (
			!InternalProcessKey &&
			(IsEmptyCmdline || none_of(LocalKey, KEY_ENTER, KEY_NUMENTER, KEY_SHIFTENTER, KEY_SHIFTNUMENTER)) &&
			SendKeyToPlugin(LocalKey)
		)
			return true;
	}
	else if (LocalKey < KEY_RCTRL0 || LocalKey > KEY_RCTRL9 || !Global->Opt->ShortcutAlwaysChdir)
	{
		// Те клавиши, которые работают при погашенных панелях:
		switch (LocalKey)
		{
			case KEY_CTRLF:
			case KEY_RCTRLF:
			case KEY_CTRLALTF:
			case KEY_RCTRLRALTF:
			case KEY_RCTRLALTF:
			case KEY_CTRLRALTF:
			case KEY_CTRLENTER:
			case KEY_RCTRLENTER:
			case KEY_CTRLNUMENTER:
			case KEY_RCTRLNUMENTER:
			case KEY_CTRLBRACKET:
			case KEY_RCTRLBRACKET:
			case KEY_CTRLBACKBRACKET:
			case KEY_RCTRLBACKBRACKET:
			case KEY_CTRLSHIFTBRACKET:
			case KEY_RCTRLSHIFTBRACKET:
			case KEY_CTRLSHIFTBACKBRACKET:
			case KEY_RCTRLSHIFTBACKBRACKET:
			case KEY_CTRL|KEY_SEMICOLON:
			case KEY_RCTRL|KEY_SEMICOLON:
			case KEY_CTRL|KEY_ALT|KEY_SEMICOLON:
			case KEY_RCTRL|KEY_RALT|KEY_SEMICOLON:
			case KEY_CTRL|KEY_RALT|KEY_SEMICOLON:
			case KEY_RCTRL|KEY_ALT|KEY_SEMICOLON:
			case KEY_CTRLALTBRACKET:
			case KEY_RCTRLRALTBRACKET:
			case KEY_CTRLRALTBRACKET:
			case KEY_RCTRLALTBRACKET:
			case KEY_CTRLALTBACKBRACKET:
			case KEY_RCTRLRALTBACKBRACKET:
			case KEY_CTRLRALTBACKBRACKET:
			case KEY_RCTRLALTBACKBRACKET:
			case KEY_ALTSHIFTBRACKET:
			case KEY_RALTSHIFTBRACKET:
			case KEY_ALTSHIFTBACKBRACKET:
			case KEY_RALTSHIFTBACKBRACKET:
			case KEY_CTRLG:
			case KEY_RCTRLG:
			case KEY_SHIFTF4:
			case KEY_F7:
			case KEY_CTRLH:
			case KEY_RCTRLH:
			case KEY_ALTSHIFTF9:
			case KEY_RALTSHIFTF9:
			case KEY_CTRLN:
			case KEY_RCTRLN:
			case KEY_GOTFOCUS:
			case KEY_KILLFOCUS:
				break;
				// эти спорные, хотя, если Ctrl-F работает, то и эти должны :-)
				/*
				      case KEY_CTRLINS:
				      case KEY_RCTRLINS:
				      case KEY_CTRLSHIFTINS:
				      case KEY_RCTRLSHIFTINS:
				      case KEY_CTRLALTINS:
				      case KEY_RCTRLRALTINS:
				      case KEY_ALTSHIFTINS:
				      case KEY_RALTSHIFTINS:
				        break;
				*/
			default:
				return false;
		}
	}

	if (!IntKeyState.ShiftPressed() && ShiftSelection!=-1)
	{
		if (SelectedFirst)
		{
			SortFileList(true);
			ShowFileList();
		}

		ShiftSelection=-1;
	}

	if ( !InternalProcessKey )
	{
		// Create a folder shortcut?
		if ((LocalKey>=KEY_CTRLSHIFT0 && LocalKey<=KEY_CTRLSHIFT9) ||
		    (LocalKey>=KEY_RCTRLSHIFT0 && LocalKey<=KEY_RCTRLSHIFT9))
		{
			SaveShortcutFolder((LocalKey&(~(KEY_CTRL | KEY_RCTRL | KEY_SHIFT | KEY_RSHIFT))) - L'0');
			return true;
		}
		// Jump to a folder shortcut?
		else if (LocalKey>=KEY_RCTRL0 && LocalKey<=KEY_RCTRL9)
		{
			ExecShortcutFolder(LocalKey-KEY_RCTRL0);
			return true;
		}
	}

	/* $ 27.08.2002 SVS
	    [*] В панели с одной колонкой Shift-Left/Right аналогично нажатию
	        Shift-PgUp/PgDn.
	*/
	if (m_Stripes == 1 && IsEmptyCmdline)
	{
		if (any_of(LocalKey, KEY_SHIFTLEFT, KEY_SHIFTNUMPAD4))
			LocalKey=KEY_SHIFTPGUP;
		else if (any_of(LocalKey, KEY_SHIFTRIGHT, KEY_SHIFTNUMPAD6))
			LocalKey=KEY_SHIFTPGDN;
	}

	switch (LocalKey)
	{
		case KEY_GOTFOCUS:
			if (Global->Opt->SmartFolderMonitor)
			{
				StartFSWatcher(true);
				Parent()->GetAnotherPanel(this)->StartFSWatcher(true);
			}
			break;

		case KEY_KILLFOCUS:
			if (Global->Opt->SmartFolderMonitor)
			{
				StopFSWatcher();
				Parent()->GetAnotherPanel(this)->StopFSWatcher();
			}
			break;

		case KEY_F1:
			{
				return m_PanelMode == panel_mode::PLUGIN_PANEL && PluginPanelHelp(GetPluginHandle());
			}

		case KEY_ALTSHIFTF9:
		case KEY_RALTSHIFTF9:
			if (m_PanelMode == panel_mode::PLUGIN_PANEL)
				Global->CtrlObject->Plugins->ConfigureCurrent(GetPluginHandle()->plugin(), FarUuid);
			else
				Global->CtrlObject->Plugins->Configure();
			return true;

		case KEY_SHIFTSUBTRACT:
			SaveSelection();
			ClearSelection();
			Redraw();
			return true;

		case KEY_SHIFTADD:
			SaveSelection();

			for (auto& i: m_ListData)
			{
				if (!(i.Attributes & FILE_ATTRIBUTE_DIRECTORY) || Global->Opt->SelectFolders)
					Select(i, true);
			}

			if (SelectedFirst)
				SortFileList(true);

			Redraw();
			return true;

		case KEY_ADD:
			SelectFiles(SELECT_ADD);
			return true;

		case KEY_SUBTRACT:
			SelectFiles(SELECT_REMOVE);
			return true;

		case KEY_CTRLADD:
		case KEY_RCTRLADD:
			SelectFiles(SELECT_ADDEXT);
			return true;

		case KEY_CTRLSUBTRACT:
		case KEY_RCTRLSUBTRACT:
			SelectFiles(SELECT_REMOVEEXT);
			return true;

		case KEY_ALTADD:
		case KEY_RALTADD:
			SelectFiles(SELECT_ADDNAME);
			return true;

		case KEY_ALTSUBTRACT:
		case KEY_RALTSUBTRACT:
			SelectFiles(SELECT_REMOVENAME);
			return true;

		case KEY_MULTIPLY:
			SelectFiles(SELECT_INVERT);
			return true;

		case KEY_CTRLMULTIPLY:
		case KEY_RCTRLMULTIPLY:
			SelectFiles(SELECT_INVERTALL);
			return true;

		case KEY_ALTMULTIPLY:
		case KEY_RALTMULTIPLY:
			SelectFiles(SELECT_INVERTFILES);
			return true;

		case KEY_ALTLEFT:     // Прокрутка длинных имен и описаний
		case KEY_RALTLEFT:
			if (LeftPos != std::numeric_limits<decltype(LeftPos)>::min())
				--LeftPos;
			Redraw();
			return true;

		case KEY_ALTHOME:     // Прокрутка длинных имен и описаний - в начало
		case KEY_RALTHOME:
			LeftPos = std::numeric_limits<decltype(LeftPos)>::min();
			Redraw();
			return true;

		case KEY_ALTRIGHT:    // Прокрутка длинных имен и описаний
		case KEY_RALTRIGHT:
			if (LeftPos != std::numeric_limits<decltype(LeftPos)>::max())
				++LeftPos;
			Redraw();
			return true;

		case KEY_ALTEND:     // Прокрутка длинных имен и описаний - в конец
		case KEY_RALTEND:
			LeftPos = std::numeric_limits<decltype(LeftPos)>::max();
			Redraw();
			return true;

		case KEY_CTRLINS:      case KEY_CTRLNUMPAD0:
		case KEY_RCTRLINS:     case KEY_RCTRLNUMPAD0:
			if (!IsEmptyCmdline)
				return false;
			[[fallthrough]];
		case KEY_CTRLSHIFTINS:  case KEY_CTRLSHIFTNUMPAD0:  // копировать имена
		case KEY_RCTRLSHIFTINS: case KEY_RCTRLSHIFTNUMPAD0:
		case KEY_CTRLALTINS:    case KEY_CTRLALTNUMPAD0:    // копировать UNC-имена
		case KEY_RCTRLRALTINS:  case KEY_RCTRLRALTNUMPAD0:
		case KEY_CTRLRALTINS:   case KEY_CTRLRALTNUMPAD0:
		case KEY_RCTRLALTINS:   case KEY_RCTRLALTNUMPAD0:
		case KEY_ALTSHIFTINS:   case KEY_ALTSHIFTNUMPAD0:   // копировать полные имена
		case KEY_RALTSHIFTINS:  case KEY_RALTSHIFTNUMPAD0:
			//if (FileCount>0 && SetCurPath()) // ?????
			SetCurPath();
			CopyNames(
				any_of(LocalKey,
					KEY_CTRLALTINS,   KEY_CTRLALTNUMPAD0,
					KEY_RCTRLRALTINS, KEY_RCTRLRALTNUMPAD0,
					KEY_CTRLRALTINS,  KEY_CTRLRALTNUMPAD0,
					KEY_RCTRLALTINS,  KEY_RCTRLALTNUMPAD0,
					KEY_ALTSHIFTINS,  KEY_ALTSHIFTNUMPAD0,
					KEY_RALTSHIFTINS, KEY_RALTSHIFTNUMPAD0),
				flags::check_any(LocalKey, KEY_CTRL | KEY_RCTRL) && flags::check_any(LocalKey, KEY_ALT | KEY_RALT)
			);
			return true;

		case KEY_CTRLSHIFTC: // hdrop copy
		case KEY_RCTRLSHIFTC:
			CopyFiles(false);
			return true;

		case KEY_CTRLSHIFTX: // hdrop cut
		case KEY_RCTRLSHIFTX:
			CopyFiles(true);
			return true;

			/* $ 14.02.2001 VVM
			  + Ctrl: вставляет имя файла с пассивной панели.
			  + CtrlAlt: вставляет UNC-имя файла с пассивной панели */
		case KEY_CTRL|KEY_SEMICOLON:
		case KEY_RCTRL|KEY_SEMICOLON:
		case KEY_CTRL|KEY_ALT|KEY_SEMICOLON:
		case KEY_RCTRL|KEY_RALT|KEY_SEMICOLON:
		case KEY_CTRL|KEY_RALT|KEY_SEMICOLON:
		case KEY_RCTRL|KEY_ALT|KEY_SEMICOLON:
		{
			int NewKey = KEY_CTRLF;

			if (LocalKey & (KEY_ALT|KEY_RALT))
				NewKey|=KEY_ALT;

			const auto SrcPanel = Parent()->PassivePanel();
			const auto OldState = SrcPanel->IsVisible() != 0;
			SrcPanel->SetVisible(true);
			SrcPanel->ProcessKey(Manager::Key(NewKey));
			SrcPanel->SetVisible(OldState);
			SetCurPath();
			return true;
		}

		case KEY_CTRLNUMENTER:
		case KEY_RCTRLNUMENTER:
		case KEY_CTRLSHIFTNUMENTER:
		case KEY_RCTRLSHIFTNUMENTER:
		case KEY_CTRLENTER:
		case KEY_RCTRLENTER:
		case KEY_CTRLSHIFTENTER:
		case KEY_RCTRLSHIFTENTER:
		case KEY_CTRLJ:
		case KEY_RCTRLJ:
		case KEY_CTRLF:
		case KEY_RCTRLF:
		case KEY_CTRLALTF:  // 29.01.2001 VVM + По CTRL+ALT+F в командную строку сбрасывается UNC-имя текущего файла.
		case KEY_RCTRLRALTF:
		case KEY_CTRLRALTF:
		case KEY_RCTRLALTF:
		{
			if (!m_ListData.empty() && SetCurPath())
			{
				string strFileName;

				if (any_of(LocalKey, KEY_CTRLSHIFTENTER, KEY_RCTRLSHIFTENTER, KEY_CTRLSHIFTNUMENTER, KEY_RCTRLSHIFTNUMENTER))
				{
					if (MakePathForUI(LocalKey, strFileName))
						strFileName += ' ';
				}
				else
				{
					bool add_slash = false;
					assert(m_CurFile < static_cast<int>(m_ListData.size()));
					const auto& Current = m_ListData[m_CurFile];

					strFileName = Current.AlternateOrNormal(m_ShowShortNames);

					if (IsParentDirectory(Current))
					{
						if (m_PanelMode == panel_mode::PLUGIN_PANEL)
							strFileName.clear();
						else
							strFileName.resize(1); // "."

						add_slash = (LocalKey & 0xFFFF) != (KEY_CTRLF & 0xFFFF);

						if (none_of(LocalKey, KEY_CTRLALTF, KEY_RCTRLRALTF, KEY_CTRLRALTF, KEY_RCTRLALTF))
							LocalKey = KEY_CTRLF;
					}

					if (any_of(LocalKey, KEY_CTRLF, KEY_RCTRLF, KEY_CTRLALTF, KEY_RCTRLRALTF, KEY_CTRLRALTF, KEY_RCTRLALTF))
					{
						if (m_PanelMode != panel_mode::PLUGIN_PANEL)
							strFileName = CreateFullPathName(
								strFileName,
								(Current.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0,
								any_of(LocalKey, KEY_CTRLALTF, KEY_RCTRLRALTF, KEY_CTRLRALTF, KEY_RCTRLALTF)
							);
						else
						{
							Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

							string strFullName = NullToEmpty(m_CachedOpenPanelInfo.CurDir);

							if (Global->Opt->PanelCtrlFRule && (m_ViewSettings.Flags&PVS_FOLDERUPPERCASE))
								inplace::upper(strFullName);

							if (!strFullName.empty())
								AddEndSlash(strFullName,0);

							if (Global->Opt->PanelCtrlFRule)
							{
								/* $ 13.10.2000 tran
								  по Ctrl-f имя должно отвечать условиям на панели */
								if ((m_ViewSettings.Flags&PVS_FILELOWERCASE) && !(Current.Attributes & FILE_ATTRIBUTE_DIRECTORY))
									inplace::lower(strFileName);

								if ((m_ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE))
									if (!(Current.Attributes & FILE_ATTRIBUTE_DIRECTORY) && !IsCaseMixed(strFileName))
										inplace::lower(strFileName);
							}

							strFileName.insert(0, strFullName);
						}
					}

					if (add_slash)
						AddEndSlash(strFileName);

					// добавим первый префикс!
					if (m_PanelMode == panel_mode::PLUGIN_PANEL && Global->Opt->SubstPluginPrefix && none_of(LocalKey, KEY_CTRLENTER, KEY_RCTRLENTER, KEY_CTRLNUMENTER, KEY_RCTRLNUMENTER, KEY_CTRLJ, KEY_RCTRLJ))
					{
						strFileName.insert(0, GetPluginPrefix());
					}

					if (!strFileName.empty() && (Global->Opt->QuotedName&QUOTEDNAME_INSERT) != 0)
						inplace::QuoteSpace(strFileName);

					strFileName += L' ';
				}

				Parent()->GetCmdLine()->InsertString(strFileName);
			}

			return true;
		}

		case KEY_CTRLALTBRACKET:       // Вставить сетевое (UNC) путь из левой панели
		case KEY_RCTRLRALTBRACKET:
		case KEY_CTRLRALTBRACKET:
		case KEY_RCTRLALTBRACKET:
		case KEY_CTRLALTBACKBRACKET:   // Вставить сетевое (UNC) путь из правой панели
		case KEY_RCTRLRALTBACKBRACKET:
		case KEY_CTRLRALTBACKBRACKET:
		case KEY_RCTRLALTBACKBRACKET:
		case KEY_ALTSHIFTBRACKET:      // Вставить сетевое (UNC) путь из активной панели
		case KEY_RALTSHIFTBRACKET:
		case KEY_ALTSHIFTBACKBRACKET:  // Вставить сетевое (UNC) путь из пассивной панели
		case KEY_RALTSHIFTBACKBRACKET:
		case KEY_CTRLBRACKET:          // Вставить путь из левой панели
		case KEY_RCTRLBRACKET:
		case KEY_CTRLBACKBRACKET:      // Вставить путь из правой панели
		case KEY_RCTRLBACKBRACKET:
		case KEY_CTRLSHIFTBRACKET:     // Вставить путь из активной панели
		case KEY_RCTRLSHIFTBRACKET:
		case KEY_CTRLSHIFTBACKBRACKET: // Вставить путь из пассивной панели
		case KEY_RCTRLSHIFTBACKBRACKET:
		{
			string strPanelDir;

			if (MakePathForUI(LocalKey, strPanelDir))
				Parent()->GetCmdLine()->InsertString(strPanelDir);

			return true;
		}

		case KEY_CTRLA:
		case KEY_RCTRLA:
		{
			if (!m_ListData.empty() && SetCurPath())
			{
				ShellSetFileAttributes(this);
				Show();
			}

			return true;
		}

		case KEY_CTRLG:
		case KEY_RCTRLG:
		{
			if (m_PanelMode != panel_mode::PLUGIN_PANEL || PluginManager::UseInternalCommand(GetPluginHandle(), PLUGIN_FAROTHER, m_CachedOpenPanelInfo))
				if (!m_ListData.empty() && ApplyCommand())
				{
					// позиционируемся в панели
					if (!Global->WindowManager->IsPanelsActive())
						Global->WindowManager->SwitchToPanels();

					Update(UPDATE_KEEP_SELECTION);
					Redraw();
					const auto AnotherPanel = Parent()->GetAnotherPanel(this);
					AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
					AnotherPanel->Redraw();
				}

			return true;
		}

		case KEY_CTRLZ:
		case KEY_RCTRLZ:
			if (!m_ListData.empty() && m_PanelMode == panel_mode::NORMAL_PANEL && SetCurPath())
				DescribeFiles();
			return true;

		case KEY_CTRLH:
		case KEY_RCTRLH:
		{
			Global->Opt->ShowHidden=!Global->Opt->ShowHidden;
			Update(UPDATE_KEEP_SELECTION);
			Redraw();
			const auto AnotherPanel = Parent()->GetAnotherPanel(this);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION);//|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
			return true;
		}

		case KEY_CTRLM:
		case KEY_RCTRLM:
			RestoreSelection();
			return true;

		case KEY_CTRLR:
		case KEY_RCTRLR:
		{
			Update(UPDATE_KEEP_SELECTION);
			Redraw();
			{
				const auto AnotherPanel = Parent()->GetAnotherPanel(this);

				if (AnotherPanel->GetType() != panel_type::FILE_PANEL)
				{
					AnotherPanel->SetCurDir(m_CurDir,false);
					AnotherPanel->Redraw();
				}
			}
			break;
		}

		case KEY_CTRLN:
		case KEY_RCTRLN:
			m_ShowShortNames=!m_ShowShortNames;
			Redraw();
			return true;

		case KEY_NUMENTER:
		case KEY_SHIFTNUMENTER:
		case KEY_ENTER:
		case KEY_SHIFTENTER:
		case KEY_CTRLALTENTER:
		case KEY_RCTRLRALTENTER:
		case KEY_CTRLRALTENTER:
		case KEY_RCTRLALTENTER:
		case KEY_CTRLALTNUMENTER:
		case KEY_RCTRLRALTNUMENTER:
		case KEY_CTRLRALTNUMENTER:
		case KEY_RCTRLALTNUMENTER:
		{
			if (m_ListData.empty())
				break;

			if (!IsEmptyCmdline)
			{
				Parent()->GetCmdLine()->ProcessKey(Key);
				return true;
			}

			ProcessEnter(true, (LocalKey & KEY_SHIFT) != 0, true, (LocalKey & (KEY_CTRL | KEY_RCTRL)) && (LocalKey & (KEY_ALT | KEY_RALT)), OFP_NORMAL);
			return true;
		}

		case KEY_CTRLBACKSLASH:
		case KEY_RCTRLBACKSLASH:
		{
			auto NeedChangeDir = true;

			if (m_PanelMode == panel_mode::PLUGIN_PANEL)// && *PluginsList[PluginsListSize-1].HostFile)
			{
				const auto CheckFullScreen=IsFullScreen();
				Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

				if (!m_CachedOpenPanelInfo.CurDir || !*m_CachedOpenPanelInfo.CurDir)
				{
					const auto OldParent = Parent();
					ChangeDir(L".."sv, true);
					NeedChangeDir = false;
					//"this" мог быть удалён в ChangeDir
					const auto ActivePanel = OldParent->ActivePanel();

					if (CheckFullScreen!=ActivePanel->IsFullScreen())
						OldParent->PassivePanel()->Show();
				}
			}

			if (NeedChangeDir)
				ChangeDir(L"\\"sv, false);

			Parent()->ActivePanel()->Show();
			return true;
		}

		case KEY_SHIFTF1:
		{
			if (!m_ListData.empty())
			{
				bool real_files = m_PanelMode != panel_mode::PLUGIN_PANEL;
				if (!real_files && GetType() == panel_type::FILE_PANEL)
				{
					Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
					real_files = (m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES) != 0;
				}

				if (real_files && SetCurPath())
					PluginPutFilesToNew();
			}

			return true;
		}
		case KEY_SHIFTF2:
		{
			if (!m_ListData.empty() && SetCurPath())
			{
				if (m_PanelMode == panel_mode::PLUGIN_PANEL)
				{
					Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

					if (m_CachedOpenPanelInfo.HostFile && *m_CachedOpenPanelInfo.HostFile)
						ProcessKey(Manager::Key(KEY_F5));
					else if ((m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES) == OPIF_REALNAMES)
						PluginHostGetFiles();

					return true;
				}

				PluginHostGetFiles();
			}

			return true;
		}

		case KEY_SHIFTF3:
		{
			ProcessHostFile();
			return true;
		}

		case KEY_F3:
		case KEY_NUMPAD5:      case KEY_SHIFTNUMPAD5:
		case KEY_ALTF3:
		case KEY_RALTF3:
		case KEY_CTRLSHIFTF3:
		case KEY_RCTRLSHIFTF3:
		case KEY_F4:
		case KEY_ALTF4:
		case KEY_RALTF4:
		case KEY_SHIFTF4:
		case KEY_CTRLSHIFTF4:
		case KEY_RCTRLSHIFTF4:
		{
			if (m_PanelMode == panel_mode::PLUGIN_PANEL)
				Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(),&m_CachedOpenPanelInfo);

			if (any_of(LocalKey, KEY_NUMPAD5, KEY_SHIFTNUMPAD5))
				LocalKey=KEY_F3;

			if ((LocalKey==KEY_SHIFTF4 || !m_ListData.empty()) && SetCurPath())
			{
				string strPluginData;
				bool PluginMode =
					m_PanelMode == panel_mode::PLUGIN_PANEL &&
					!PluginManager::UseInternalCommand(GetPluginHandle(), PLUGIN_FARGETFILE, m_CachedOpenPanelInfo) &&
					!(m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES);

				if (PluginMode)
				{
					strPluginData = concat(L'<', NullToEmpty(m_CachedOpenPanelInfo.HostFile), L':', NullToEmpty(m_CachedOpenPanelInfo.CurDir), L'>');
				}

				uintptr_t codepage = CP_DEFAULT;
				const auto Edit = any_of(LocalKey, KEY_F4, KEY_ALTF4, KEY_RALTF4, KEY_SHIFTF4, KEY_CTRLSHIFTF4, KEY_RCTRLSHIFTF4);
				string strFileName;
				string strShortFileName;

				if (LocalKey==KEY_SHIFTF4)
				{
					do
					{
						if (!dlgOpenEditor(strFileName, codepage))
							return false;

						if (!strFileName.empty())
						{
							inplace::unquote(strFileName);

							strShortFileName = ConvertNameToShort(strFileName);

							if (IsAbsolutePath(strFileName))
							{
								PluginMode = false;
							}

							// проверим путь к файлу
							const auto pos = FindLastSlash(strFileName);
							if (pos != string::npos && pos)
							{
								if (!(HasPathPrefix(strFileName) && pos==3))
								{
									if (!os::fs::exists(string_view(strFileName).substr(0, pos)))
									{
										if (Message(MSG_WARNING,
											msg(lng::MWarning),
											{
												msg(lng::MEditNewPath1),
												msg(lng::MEditNewPath2),
												msg(lng::MEditNewPath3)
											},
											{ lng::MHYes, lng::MHNo },
											L"WarnEditorPath"sv) != message_result::first_button)
											return false;
									}
								}
							}
						}
						else if (PluginMode) // пустое имя файла в панели плагина не разрешается!
						{
							if (Message(MSG_WARNING,
								msg(lng::MWarning),
								{
									msg(lng::MEditNewPlugin1),
									msg(lng::MEditNewPath3)
								},
								{ lng::MCancel },
								L"WarnEditorPluginName"sv) != message_result::first_button)
								return false;
						}
						else
						{
							strFileName = msg(lng::MNewFileName);
						}
					}
					while (strFileName.empty());
				}
				else
				{
					assert(m_CurFile < static_cast<int>(m_ListData.size()));
					const auto& Current = m_ListData[m_CurFile];

					if (Current.Attributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (Edit)
							return ProcessKey(Manager::Key(KEY_CTRLA));

						CountDirSize(!PluginMode);
						return true;
					}

					strFileName = Current.FileName;
					strShortFileName = Current.AlternateFileName();
				}

				string strTempName;
				string TemporaryDirectory;
				bool UploadFailed = false, NewFile = false;

				if (PluginMode)
				{
					TemporaryDirectory = MakeTemp();

					if (!os::fs::create_directory(TemporaryDirectory))
						return true;

					strTempName = path::join(TemporaryDirectory, PointToName(strFileName));

					const FileListItem* CurPtr = nullptr;
					if (LocalKey==KEY_SHIFTF4)
					{
						int Pos=FindFile(strFileName);

						if (Pos!=-1)
							CurPtr = &m_ListData[Pos];
						else
						{
							NewFile = true;
							strFileName = strTempName;
						}
					}
					else
					{
						CurPtr = &m_ListData[m_CurFile];
					}

					if (!NewFile)
					{
						PluginPanelItemHolderHeap PanelItem;
						FileListToPluginItem(*CurPtr, PanelItem);

						if (!Global->CtrlObject->Plugins->GetFile(GetPluginHandle(), &PanelItem.Item, TemporaryDirectory, strFileName, OPM_SILENT | (Edit? OPM_EDIT : OPM_VIEW)))
						{
							// BUGBUG check result
							if (!os::fs::remove_directory(TemporaryDirectory))
							{
								LOGWARNING(L"remove_directory({}): {}"sv, TemporaryDirectory, last_error());
							}
							return true;
						}
					}

					strShortFileName = ConvertNameToShort(strFileName);
				}

				auto DeleteViewedFile = PluginMode && !Edit; // внутренний viewer сам все удалит.
				auto Modaling = false;
				auto UploadFile = true;
				auto RefreshedPanel = true;

				if (!strFileName.empty())
				{
					if (Edit)
					{
						const auto EnableExternal = ((any_of(LocalKey, KEY_F4, KEY_SHIFTF4) && Global->Opt->EdOpt.UseExternalEditor) ||
							(any_of(LocalKey, KEY_ALTF4, KEY_RALTF4) && !Global->Opt->EdOpt.UseExternalEditor)) && !Global->Opt->strExternalEditor.empty();
						auto Processed = false;

						const auto SavedState = file_state::get(strFileName);
						if (any_of(LocalKey, KEY_ALTF4, KEY_RALTF4, KEY_F4) && ProcessLocalFileTypes(strFileName, strShortFileName, LocalKey == KEY_F4? FILETYPE_EDIT:FILETYPE_ALTEDIT, PluginMode))
						{
							UploadFile = file_state::get(strFileName) != SavedState;
							Processed = true;
						}

						if (!Processed || any_of(LocalKey, KEY_CTRLSHIFTF4, KEY_RCTRLSHIFTF4))
						{
							if (EnableExternal)
							{
								ProcessExternal(Global->Opt->strExternalEditor, strFileName, strShortFileName, PluginMode, TemporaryDirectory);
								UploadFile = file_state::get(strFileName) != SavedState;
								Modaling = PluginMode; // External editor from plugin panel is Modal!
							}
							else if (PluginMode)
							{
								RefreshedPanel = Global->WindowManager->GetCurrentWindow()->GetType() != windowtype_editor;
								const auto ShellEditor = FileEditor::create(strFileName, codepage, (LocalKey == KEY_SHIFTF4 ? FFILEEDIT_CANNEWFILE : 0) | FFILEEDIT_DISABLEHISTORY, -1, -1, &strPluginData);
								if (-1 == ShellEditor->GetExitCode()) Global->WindowManager->ExecuteModal(ShellEditor);//OT
								UploadFile=ShellEditor->IsFileChanged() || NewFile;
								Modaling = true;
							}
							else
							{
								const auto ShellEditor = FileEditor::create(strFileName, codepage, (LocalKey == KEY_SHIFTF4 ? FFILEEDIT_CANNEWFILE : 0) | FFILEEDIT_ENABLEF6);
								const auto editorExitCode=ShellEditor->GetExitCode();

								if (!(editorExitCode == XC_LOADING_INTERRUPTED || editorExitCode == XC_OPEN_ERROR))
								{
									NamesList EditList;

									for (const auto& i: m_ListData)
									{
										if (!(i.Attributes & FILE_ATTRIBUTE_DIRECTORY))
											EditList.AddName(i.FileName);
									}

									EditList.SetCurName(strFileName);
									ShellEditor->SetNamesList(EditList);
								}
							}
						}

						if (PluginMode && UploadFile)
						{
							PluginPanelItemHolderHeap PanelItem;
							const auto strSaveDir = os::fs::GetCurrentDirectory();

							if (!os::fs::exists(strTempName))
							{
								string_view Path = strTempName;
								CutToSlash(Path);
								const auto Find = os::fs::enum_files(Path + L'*');
								const auto ItemIterator = std::find_if(CONST_RANGE(Find, i) { return !(i.Attributes & FILE_ATTRIBUTE_DIRECTORY); });
								if (ItemIterator != Find.cend())
									strTempName = Path + ItemIterator->FileName;
							}

							if (FileNameToPluginItem(strTempName, PanelItem))
							{
								const auto PutCode = Global->CtrlObject->Plugins->PutFiles(GetPluginHandle(), { &PanelItem.Item, 1 }, false, OPM_EDIT);

								if (PutCode==1 || PutCode==2)
									SetPluginModified();

								if (!PutCode)
									UploadFailed = true;
							}

							FarChDir(strSaveDir);
						}
					}
					else
					{
						const auto EnableExternal =
						(
							(LocalKey == KEY_F3 && Global->Opt->ViOpt.UseExternalViewer) ||
							(any_of(LocalKey, KEY_ALTF3, KEY_RALTF3) && !Global->Opt->ViOpt.UseExternalViewer)
						) &&
						!Global->Opt->strExternalViewer.empty();
						/* $ 02.08.2001 IS обработаем ассоциации для alt-f3 */
						auto Processed = false;

						if (any_of(LocalKey, KEY_ALTF3, KEY_RALTF3) && ProcessLocalFileTypes(strFileName, strShortFileName, FILETYPE_ALTVIEW, PluginMode))
							Processed = true;
						else if (LocalKey == KEY_F3 && ProcessLocalFileTypes(strFileName, strShortFileName, FILETYPE_VIEW, PluginMode))
							Processed = true;

						if (!Processed || any_of(LocalKey, KEY_CTRLSHIFTF3, KEY_RCTRLSHIFTF3))
						{
							if (EnableExternal)
								ProcessExternal(Global->Opt->strExternalViewer, strFileName, strShortFileName, PluginMode, TemporaryDirectory);
							else
							{
								NamesList ViewList;

								if (!PluginMode)
								{
									for (const auto& i: m_ListData)
									{
										if (!(i.Attributes & FILE_ATTRIBUTE_DIRECTORY))
											ViewList.AddName(i.FileName);
									}

									ViewList.SetCurName(strFileName);
								}

								const auto ShellViewer = FileViewer::create(
									strFileName,
									true,
									PluginMode,
									PluginMode,
									-1,
									strPluginData,
									&ViewList);

								/* $ 08.04.2002 IS
								Сбросим DeleteViewedFile, т.к. внутренний viewer сам все удалит
								*/
								if (ShellViewer->GetExitCode() && PluginMode)
								{
									ShellViewer->SetTempViewName(strFileName);
									DeleteViewedFile=false;
								}
							}
						}
					}
				}

				/* $ 08.04.2002 IS
				     для файла, который открывался во внутреннем viewer-е, ничего не
				     предпринимаем, т.к. viewer об этом позаботится сам
				*/
				if (PluginMode)
				{
					if (UploadFailed)
					{
						Message(MSG_WARNING,
							msg(lng::MError),
							{
								msg(lng::MCannotSaveFile),
								msg(lng::MTextSavedToTemp),
								strFileName
							},
							{ lng::MOk });
					}
					else if (Edit || DeleteViewedFile)
					{
						// удаляем файл только для случая открытия его в редакторе или во
						// внешнем viewer-е, т.к. внутренний viewer удаляет файл сам
						DeleteFileWithFolder(strFileName);
					}
				}

				if (Modaling && RefreshedPanel)
				{
					if (!PluginMode || UploadFile)
					{
						Update(UPDATE_KEEP_SELECTION);
						Redraw();
						const auto AnotherPanel = Parent()->GetAnotherPanel(this);

						if (AnotherPanel->GetMode() == panel_mode::NORMAL_PANEL)
						{
							AnotherPanel->Update(UPDATE_KEEP_SELECTION);
							AnotherPanel->Redraw();
						}
					}
//          else
//            SetTitle();
				}
				else if (m_PanelMode == panel_mode::NORMAL_PANEL)
					AccessTimeUpdateRequired = true;
			}

			/* $ 15.07.2000 tran
			   а тут мы вызываем перерисовку панелей
			   потому что этот viewer, editor могут нам неверно восстановить
			   */
			//Parent()->Redraw();
			return true;
		}

		case KEY_F5:
		case KEY_F6:
		case KEY_ALTF6:
		case KEY_RALTF6:
		case KEY_DRAGCOPY:
		case KEY_DRAGMOVE:
		{
			ProcessCopyKeys(LocalKey);

			return true;
		}

		case KEY_ALTF5:  // Печать текущего/выбранных файла/ов
		case KEY_RALTF5:
		{
			if (!m_ListData.empty() && SetCurPath())
				PrintFiles(this);

			return true;
		}

		case KEY_SHIFTF5:
		case KEY_SHIFTF6:
		{
			if (!m_ListData.empty() && SetCurPath())
			{
				assert(m_CurFile < static_cast<int>(m_ListData.size()));
				const auto name = m_ListData[m_CurFile].FileName; // must be a copy
				const auto selected = m_ListData[m_CurFile].Selected;

				int RealName = m_PanelMode != panel_mode::PLUGIN_PANEL;
				ReturnCurrentFile = true;

				if (m_PanelMode == panel_mode::PLUGIN_PANEL)
				{
					Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
					RealName = m_CachedOpenPanelInfo.Flags&OPIF_REALNAMES;
				}

				if (RealName)
				{
					int ToPlugin = 0;
					Copy(shared_from_this(), LocalKey == KEY_SHIFTF6, false, true, true, ToPlugin, nullptr);
				}
				else
				{
					ProcessCopyKeys(LocalKey==KEY_SHIFTF5 ? KEY_F5:KEY_F6);
				}

				ReturnCurrentFile = false;

				if (!m_ListData.empty())
				{
					assert(m_CurFile < static_cast<int>(m_ListData.size()));
					if (LocalKey != KEY_SHIFTF5 && equal_icase(name, m_ListData[m_CurFile].FileName) && selected > m_ListData[m_CurFile].Selected)
					{
						Select(m_ListData[m_CurFile], selected);
						Redraw();
					}
				}
			}

			return true;
		}

		case KEY_F7:
		{
			if (SetCurPath())
			{
				if (m_PanelMode == panel_mode::PLUGIN_PANEL && !PluginManager::UseInternalCommand(GetPluginHandle(), PLUGIN_FARMAKEDIRECTORY, m_CachedOpenPanelInfo))
				{
					string strDirName;
					auto DirName = strDirName.c_str();
					int MakeCode=Global->CtrlObject->Plugins->MakeDirectory(GetPluginHandle(), &DirName,0);
					strDirName = DirName;

					if (!MakeCode)
					{
						Message(MSG_WARNING,
							msg(lng::MError),
							{
								msg(lng::MCannotCreateFolder),
								strDirName
							},
							{ lng::MOk });
					}
					Update(UPDATE_KEEP_SELECTION);

					if (MakeCode==1)
						GoToFile(PointToName(strDirName));

					Redraw();
					const auto AnotherPanel = Parent()->GetAnotherPanel(this);
					/* $ 07.09.2001 VVM
					  ! Обновить соседнюю панель с установкой на новый каталог */
//          AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
//          AnotherPanel->Redraw();

					if (AnotherPanel->GetType() != panel_type::FILE_PANEL)
					{
						AnotherPanel->SetCurDir(m_CurDir,false);
						AnotherPanel->Redraw();
					}
				}
				else
					ShellMakeDir(this);
			}

			return true;
		}

		case KEY_F8:
		case KEY_SHIFTDEL:
		case KEY_SHIFTF8:
		case KEY_SHIFTNUMDEL:
		case KEY_SHIFTDECIMAL:
		case KEY_ALTNUMDEL:
		case KEY_RALTNUMDEL:
		case KEY_ALTDECIMAL:
		case KEY_RALTDECIMAL:
		case KEY_ALTDEL:
		case KEY_RALTDEL:
		{
			if (IsRepeatedKey() /*&& !Global->Opt->Confirmation.Delete*/) // не удаляем, если зажата клавиша
				return true;

			if (!m_ListData.empty() && SetCurPath())
			{
				if (LocalKey==KEY_SHIFTF8)
					ReturnCurrentFile = true;

				if (m_PanelMode == panel_mode::PLUGIN_PANEL && !PluginManager::UseInternalCommand(GetPluginHandle(), PLUGIN_FARDELETEFILES, m_CachedOpenPanelInfo))
				{
					PluginDelete();
				}
				else
				{
					Delete(
						shared_from_this(),
						any_of(LocalKey, KEY_SHIFTDEL, KEY_SHIFTNUMDEL, KEY_SHIFTDECIMAL)? delete_type::remove :
						any_of(LocalKey, KEY_ALTDEL, KEY_RALTDEL, KEY_ALTNUMDEL, KEY_RALTNUMDEL, KEY_ALTDECIMAL, KEY_RALTDECIMAL)? delete_type::erase :
						Global->Opt->DeleteToRecycleBin? delete_type::recycle : delete_type::remove);
				}

				if (LocalKey==KEY_SHIFTF8)
					ReturnCurrentFile = false;
			}

			return true;
		}

		// $ 26.07.2001 VVM  С альтом скролим всегда по 1
		case KEY_MSWHEEL_UP:
		case KEY_MSWHEEL_UP | KEY_ALT:
		case KEY_MSWHEEL_UP | KEY_RALT:
			Scroll(LocalKey & (KEY_ALT | KEY_RALT)? -1 : static_cast<int>(-Global->Opt->MsWheelDelta));
			return true;

		case KEY_MSWHEEL_DOWN:
		case KEY_MSWHEEL_DOWN | KEY_ALT:
		case KEY_MSWHEEL_DOWN | KEY_RALT:
			Scroll(LocalKey & (KEY_ALT | KEY_RALT)? 1 : static_cast<int>(Global->Opt->MsWheelDelta));
			return true;

		case KEY_MSWHEEL_LEFT:
		case KEY_MSWHEEL_LEFT | KEY_ALT:
		case KEY_MSWHEEL_LEFT | KEY_RALT:
		{
			int Roll = LocalKey & (KEY_ALT | KEY_RALT)? 1 : static_cast<int>(Global->Opt->MsHWheelDelta);
			repeat(Roll, [&]{ ProcessKey(Manager::Key(KEY_LEFT)); });
			return true;
		}

		case KEY_MSWHEEL_RIGHT:
		case KEY_MSWHEEL_RIGHT | KEY_ALT:
		case KEY_MSWHEEL_RIGHT | KEY_RALT:
		{
			int Roll = LocalKey & (KEY_ALT | KEY_RALT)? 1 : static_cast<int>(Global->Opt->MsHWheelDelta);
			repeat(Roll, [&]{ ProcessKey(Manager::Key(KEY_RIGHT)); });
			return true;
		}

		case KEY_HOME:         case KEY_NUMPAD7:
			ToBegin();
			return true;


		case KEY_END:          case KEY_NUMPAD1:
			ToEnd();
			return true;

		case KEY_UP:           case KEY_NUMPAD8:
			MoveCursorAndShow(-1);
			return true;

		case KEY_DOWN:         case KEY_NUMPAD2:
			MoveCursorAndShow(1);
			return true;

		case KEY_PGUP:         case KEY_NUMPAD9:
			N=m_Stripes*m_Height-1;
			m_CurTopFile-=N;
			MoveCursorAndShow(-N);
			return true;

		case KEY_PGDN:         case KEY_NUMPAD3:
			N=m_Stripes*m_Height-1;
			m_CurTopFile+=N;
			MoveCursorAndShow(N);
			return true;

		case KEY_LEFT:         case KEY_NUMPAD4:

			if ((m_Stripes == 1 && Global->Opt->ShellRightLeftArrowsRule == 1) || m_Stripes>1 || IsEmptyCmdline)
			{
				if (m_CurTopFile>=m_Height && m_CurFile-m_CurTopFile<m_Height)
					m_CurTopFile-=m_Height;

				MoveCursorAndShow(-m_Height);
				return true;
			}
			return false;

		case KEY_RIGHT:        case KEY_NUMPAD6:

			if ((m_Stripes == 1 && Global->Opt->ShellRightLeftArrowsRule == 1) || m_Stripes>1 || IsEmptyCmdline)
			{
				if (m_CurFile+m_Height < static_cast<int>(m_ListData.size()) && m_CurFile-m_CurTopFile>=(m_Stripes-1)*(m_Height))
					m_CurTopFile+=m_Height;

				MoveCursorAndShow(m_Height);
				return true;
			}

			return false;

		/* $ 25.04.2001 DJ
			оптимизация Shift-стрелок для Selected files first: делаем сортировку
			один раз
		*/
		case KEY_SHIFTHOME:    case KEY_SHIFTNUMPAD7:
		{
			InternalProcessKey++;
			while (m_CurFile>0)
				MoveSelection(up);
			MoveSelection(up);
			InternalProcessKey--;

			if (SelectedFirst)
				SortFileList(true);

			ShowFileList();
			return true;
		}

		case KEY_SHIFTEND:     case KEY_SHIFTNUMPAD1:
		{
			InternalProcessKey++;
			while (m_CurFile < static_cast<int>(m_ListData.size() - 1))
				MoveSelection(down);

			MoveSelection(down);
			InternalProcessKey--;

			if (SelectedFirst)
				SortFileList(true);

			ShowFileList();
			return true;
		}

		case KEY_SHIFTPGUP:    case KEY_SHIFTNUMPAD9:
		case KEY_SHIFTPGDN:    case KEY_SHIFTNUMPAD3:
		{
			N=m_Stripes*m_Height-1;
			InternalProcessKey++;

			while (N--)
				MoveSelection(any_of(LocalKey, KEY_SHIFTPGUP, KEY_SHIFTNUMPAD9)? up : down);

			InternalProcessKey--;

			if (SelectedFirst)
				SortFileList(true);

			ShowFileList();
			return true;
		}

		case KEY_SHIFTLEFT:    case KEY_SHIFTNUMPAD4:
		case KEY_SHIFTRIGHT:   case KEY_SHIFTNUMPAD6:
		{
			if (m_ListData.empty())
				return true;

			if (m_Stripes>1)
			{
				N=m_Height;
				InternalProcessKey++;

				while (N--)
					MoveSelection(any_of(LocalKey, KEY_SHIFTLEFT, KEY_SHIFTNUMPAD4)? up : down);

				assert(m_CurFile < static_cast<int>(m_ListData.size()));
				Select(m_ListData[m_CurFile], ShiftSelection != 0);

				if (SelectedFirst)
					SortFileList(true);

				InternalProcessKey--;

				if (SelectedFirst)
					SortFileList(true);

				ShowFileList();
				return true;
			}

			return false;
		}

		case KEY_SHIFTUP:      case KEY_SHIFTNUMPAD8:
		case KEY_SHIFTDOWN:    case KEY_SHIFTNUMPAD2:
			MoveSelection(any_of(LocalKey, KEY_SHIFTUP, KEY_SHIFTNUMPAD8) ? up : down);
			ShowFileList();
			return true;

		case KEY_INS:          case KEY_NUMPAD0:
		{
			if (m_ListData.empty())
				return true;

			assert(m_CurFile < static_cast<int>(m_ListData.size()));
			auto& Current = m_ListData[m_CurFile];
			Select(Current, !Current.Selected);
			bool avoid_up_jump = SelectedFirst && (m_CurFile > 0) && (m_CurFile+1 == static_cast<int>(m_ListData.size())) && Current.Selected;
			MoveCursorAndShow(1);

			if (SelectedFirst)
			{
				SortFileList(true);
				if (avoid_up_jump)
					ToEnd();
			}

			ShowFileList();
			return true;
		}

		case KEY_CTRLF3:
		case KEY_RCTRLF3:
			SetSortMode(panel_sort::BY_NAME);
			return true;

		case KEY_CTRLF4:
		case KEY_RCTRLF4:
			SetSortMode(panel_sort::BY_EXT);
			return true;

		case KEY_CTRLF5:
		case KEY_RCTRLF5:
			SetSortMode(panel_sort::BY_MTIME);
			return true;

		case KEY_CTRLF6:
		case KEY_RCTRLF6:
			SetSortMode(panel_sort::BY_SIZE);
			return true;

		case KEY_CTRLF7:
		case KEY_RCTRLF7:
			SetSortMode(panel_sort::UNSORTED);
			return true;

		case KEY_CTRLF8:
		case KEY_RCTRLF8:
			SetSortMode(panel_sort::BY_CTIME);
			return true;

		case KEY_CTRLF9:
		case KEY_RCTRLF9:
			SetSortMode(panel_sort::BY_ATIME);
			return true;

		case KEY_CTRLF10:
		case KEY_RCTRLF10:
			SetSortMode(panel_sort::BY_DIZ);
			return true;

		case KEY_CTRLF11:
		case KEY_RCTRLF11:
			SetSortMode(panel_sort::BY_OWNER);
			return true;

		case KEY_CTRLF12:
		case KEY_RCTRLF12:
			SelectSortMode();
			return true;

		case KEY_SHIFTF11:
			m_SortGroups=!m_SortGroups;

			if (m_SortGroups)
				ReadSortGroups();

			SortFileList(true);
			ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
			Show();
			return true;

		case KEY_SHIFTF12:
			SelectedFirst=!SelectedFirst;
			SortFileList(true);
			ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
			Show();
			return true;

		case KEY_CTRLPGUP:     case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP:    case KEY_RCTRLNUMPAD9:
		{
			if (Global->Opt->PgUpChangeDisk || m_PanelMode == panel_mode::PLUGIN_PANEL || !IsRootPath(m_CurDir))
			{
				//"this" может быть удалён в ChangeDir
				const auto CheckFullScreen = IsFullScreen();
				const auto OldParent = Parent();
				ChangeDir(L".."sv, true);
				const auto NewActivePanel = OldParent->ActivePanel();
				NewActivePanel->SetViewMode(NewActivePanel->GetViewMode());

				if (CheckFullScreen!=NewActivePanel->IsFullScreen())
					OldParent->GetAnotherPanel(NewActivePanel)->Show();

				NewActivePanel->Show();
			}
			return true;
		}

		case KEY_CTRLPGDN:
		case KEY_RCTRLPGDN:
		case KEY_CTRLNUMPAD3:
		case KEY_RCTRLNUMPAD3:
		case KEY_CTRLSHIFTPGDN:
		case KEY_RCTRLSHIFTPGDN:
		case KEY_CTRLSHIFTNUMPAD3:
		case KEY_RCTRLSHIFTNUMPAD3:
			ProcessEnter(false, false, !(LocalKey&KEY_SHIFT), false, OFP_ALTERNATIVE);
			return true;

		case KEY_APPS:
		case KEY_SHIFTAPPS:
		{
			//вызовем EMenu если он есть
			if (Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu.Id))
			{
				Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, {}); // EMenu Plugin :-)
			}
			return true;
		}

		default:
			if (((LocalKey>=KEY_ALT_BASE+0x01 && LocalKey<=KEY_ALT_BASE+65535) || (LocalKey>=KEY_RALT_BASE+0x01 && LocalKey<=KEY_RALT_BASE+65535) ||
			        (LocalKey>=KEY_ALTSHIFT_BASE+0x01 && LocalKey<=KEY_ALTSHIFT_BASE+65535) || (LocalKey>=KEY_RALTSHIFT_BASE+0x01 && LocalKey<=KEY_RALTSHIFT_BASE+65535)) &&
			        (LocalKey&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_BS && (LocalKey&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_TAB &&
			        (LocalKey&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_ENTER && (LocalKey&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_ESC &&
			        !(LocalKey&EXTENDED_KEY_BASE)
			   )
			{
				FastFind(Key);
			}
			else
				break;

			return true;
	}

	return false;
}


void FileList::Select(FileListItem& SelItem, bool Selection)
{
	if (IsParentDirectory(SelItem) || SelItem.Selected == Selection)
		return;

	CacheSelIndex = -1;
	CacheSelClearIndex = -1;

	if (Selection)
	{
		SelItem.Selected = true;
		++m_SelFileCount;
		m_SelDirCount += SelItem.Attributes & FILE_ATTRIBUTE_DIRECTORY? 1 : 0;
		SelFileSize += SelItem.FileSize;
	}
	else
	{
		SelItem.Selected = false;
		--m_SelFileCount;
		m_SelDirCount -= SelItem.Attributes & FILE_ATTRIBUTE_DIRECTORY? 1 : 0;
		SelFileSize -= SelItem.FileSize;
	}
}

static bool IsExecutable(string_view const Filename)
{
	const auto Extension = name_ext(Filename).second;

	static const std::array Executables{ L".exe"sv, L".cmd"sv, L".com"sv, L".bat"sv };
	return std::any_of(ALL_CONST_RANGE(Executables), [&](const string_view i)
	{
		return equal_icase(Extension, i);
	});
}

void FileList::ProcessEnter(bool EnableExec,bool SeparateWindow,bool EnableAssoc, bool RunAs, OPENFILEPLUGINTYPE Type)
{
	if (m_CurFile >= static_cast<int>(m_ListData.size()))
		return;

	const auto& CurItem = m_ListData[m_CurFile];
	auto strFileName = CurItem.FileName;
	auto strShortFileName = CurItem.AlternateFileName();

	if (CurItem.Attributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		auto IsRealName = true;

		if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		{
			Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
			IsRealName = (m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES) != 0;
		}

		// Shift-Enter на каталоге вызывает проводник
		if (IsRealName && SeparateWindow)
		{
			string strFullPath;

			if (!IsAbsolutePath(CurItem.FileName))
			{
				strFullPath = m_CurDir;
				AddEndSlash(strFullPath);

				/* 23.08.2001 VVM
				  ! SHIFT+ENTER на ".." срабатывает для текущего каталога, а не родительского */
				if (!IsParentDirectory(CurItem))
					strFullPath += CurItem.FileName;
			}
			else
			{
				strFullPath = CurItem.FileName;
			}

			OpenFolderInShell(strFullPath);
		}
		else
		{
			const auto CheckFullScreen = IsFullScreen();
			const auto OldParent = Parent();

			// Don't use CurItem directly: ChangeDir calls PopPlugin, which clears m_ListData
			const auto DirCopy = CurItem.FileName;
			const auto DataItemCopy = CurItem.UserData;
			ChangeDir(DirCopy, IsParentDirectory(CurItem), false, true, &DataItemCopy, Type, false);

			//"this" может быть удалён в ChangeDir
			const auto ActivePanel = OldParent->ActivePanel();

			if (CheckFullScreen!=ActivePanel->IsFullScreen())
			{
				OldParent->PassivePanel()->Show();
			}

			ActivePanel->Show();
		}
	}
	else
	{
		bool OpenedPlugin = false;
		const auto PluginMode = m_PanelMode == panel_mode::PLUGIN_PANEL && !PluginManager::UseInternalCommand(GetPluginHandle(), PLUGIN_FARGETFILE, m_CachedOpenPanelInfo);
		string FileNameToDelete;
		SCOPE_EXIT{ if (PluginMode && !OpenedPlugin && !FileNameToDelete.empty()) GetPluginHandle()->delayed_delete(FileNameToDelete); };
		file_state SavedState;

		if (PluginMode)
		{
			const auto strTempDir = MakeTemp();
			// BUGBUG check result
			if (!os::fs::create_directory(strTempDir))
			{
				LOGWARNING(L"create_directory({}): {}"sv, strTempDir, last_error());
			}

			PluginPanelItemHolderHeap PanelItem;
			FileListToPluginItem(CurItem, PanelItem);

			if (!Global->CtrlObject->Plugins->GetFile(GetPluginHandle(), &PanelItem.Item, strTempDir, strFileName, OPM_SILENT | OPM_EDIT))
			{
				// BUGBUG check result
				if (!os::fs::remove_directory(strTempDir))
				{
					LOGWARNING(L"remove_directory({}): {}"sv, strTempDir, last_error());
				}

				return;
			}
			FileNameToDelete = strFileName;
			strShortFileName = ConvertNameToShort(strFileName);
			SavedState = file_state::get(strFileName);
		}


		if (EnableExec)
		{
			// Enter
			if (!SetCurPath())
				return;

			if (SeparateWindow || !ProcessLocalFileTypes(strFileName, strShortFileName, FILETYPE_EXEC, PluginMode, true, RunAs))
			{
				const auto IsItExecutable = IsExecutable(strFileName);

				auto StopProcessing = false;
				if (!IsItExecutable && !SeparateWindow)
				{
					OpenedPlugin = OpenFilePlugin(strFileName, TRUE, Type, &StopProcessing) != nullptr;
					if (OpenedPlugin || StopProcessing)
						return;
				}

				if (IsItExecutable || SeparateWindow || Global->Opt->UseRegisteredTypes)
				{
					execute_info Info;
					Info.DisplayCommand = strFileName;
					Info.WaitMode = PluginMode? execute_info::wait_mode::wait_finish : SeparateWindow? execute_info::wait_mode::no_wait : execute_info::wait_mode::if_needed;
					Info.SourceMode = IsItExecutable? execute_info::source_mode::known_executable : execute_info::source_mode::known;
					Info.RunAs = RunAs;

					Info.Command = ConvertNameToFull(strFileName);

					Parent()->GetCmdLine()->ExecString(Info);

					const auto ExclusionFlag = IsItExecutable? EXCLUDECMDHISTORY_NOTPANEL : EXCLUDECMDHISTORY_NOTWINASS;
					if (!(Global->Opt->ExcludeCmdHistory & ExclusionFlag) && !PluginMode)
					{
						Global->CtrlObject->CmdHistory->AddToHistory(QuoteSpace(strFileName), HR_DEFAULT, nullptr, {}, m_CurDir);
					}
				}
			}
		}
		else
		{
			// CtrlPgDn
			if (EnableAssoc && ProcessLocalFileTypes(strFileName, strShortFileName, FILETYPE_ALTEXEC, PluginMode))
				return;

			OpenedPlugin = OpenFilePlugin(strFileName, TRUE, Type) != nullptr;
		}

		if (PluginMode && !OpenedPlugin)
		{
			if (file_state::get(strFileName) != SavedState)
			{
				PluginPanelItemHolderHeap PanelItem;
				if (FileNameToPluginItem(strFileName, PanelItem))
				{
					int PutCode = Global->CtrlObject->Plugins->PutFiles(GetPluginHandle(), { &PanelItem.Item, 1 }, false, OPM_EDIT);
					if (PutCode == 1 || PutCode == 2)
						SetPluginModified();
				}
			}
		}
	}
}


bool FileList::SetCurDir(string_view const NewDir, bool ClosePanel, bool IsUpdated, bool const Silent)
{
	UserDataItem UsedData{};

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		if (ClosePanel)
		{
			const auto CheckFullScreen = IsFullScreen();
			Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
			const string strInfoHostFile = NullToEmpty(m_CachedOpenPanelInfo.HostFile);

			for (;;)
			{
				if (ProcessPluginEvent(FE_CLOSE,nullptr))
					return false;

				if (!PopPlugin(TRUE))
					break;

				if (NewDir.empty())
				{
					Update(0);
					PopPrevData(strInfoHostFile,true,true,true,true);
					break;
				}
			}

			Parent()->RedrawKeyBar();

			if (CheckFullScreen!=IsFullScreen())
			{
				Parent()->GetAnotherPanel(this)->Redraw();
			}
		}
		else if (m_CurFile < static_cast<int>(m_ListData.size()))
		{
			UsedData = m_ListData[m_CurFile].UserData;
		}
	}

	if (!NewDir.empty())
	{
		if (m_PanelMode != panel_mode::PLUGIN_PANEL)
			Panel::SetCurDir(NewDir, ClosePanel, IsUpdated, Silent);

		return ChangeDir(NewDir, NewDir == L".."sv, true, IsUpdated, &UsedData, OFP_NORMAL, Silent);
	}

	return false;
}

bool FileList::ChangeDir(string_view const NewDir, bool IsParent, bool ResolvePath,bool IsUpdated, const UserDataItem* DataItem, OPENFILEPLUGINTYPE OfpType, bool const Silent)
{
	bool IsPopPlugin = false;

	SCOPE_EXIT
	{
		if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		{
			Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
			if (m_CachedOpenPanelInfo.Flags & OPIF_SHORTCUT)
			{
				const auto InfoCurDir = NullToEmpty(m_CachedOpenPanelInfo.CurDir);
				const auto InfoHostFile = NullToEmpty(m_CachedOpenPanelInfo.HostFile);
				const auto InfoData = NullToEmpty(m_CachedOpenPanelInfo.ShortcutData);
				Global->CtrlObject->FolderHistory->AddToHistory(InfoCurDir, HR_DEFAULT, &PluginManager::GetUUID(GetPluginHandle()), InfoHostFile, InfoData);
			}
		}
		else
		{
			Global->CtrlObject->FolderHistory->AddToHistory(GetCurDir());
			if (!IsPopPlugin)
				InitFSWatcher(false);
		}
	};

	if (m_PanelMode != panel_mode::PLUGIN_PANEL && !IsAbsolutePath(NewDir) && !equal_icase(os::fs::GetCurrentDirectory(), m_CurDir))
		FarChDir(m_CurDir);

	string strFindDir;
	string strSetDir(NewDir);

	bool RootPath = false;
	bool NetPath = false;
	bool DrivePath = false;

	if (m_PanelMode != panel_mode::PLUGIN_PANEL)
	{
		if (IsParent)
		{
			strSetDir = m_CurDir;
			const auto Type = ParsePath(m_CurDir, nullptr, &RootPath);
			if(Type == root_type::remote || Type == root_type::unc_remote)
			{
				NetPath = true;
			}
			else if(Type == root_type::drive_letter)
			{
				DrivePath = true;
			}

			if(!RootPath)
			{
				CutToSlash(strSetDir);
			}
		}

		if (!ResolvePath)
			strSetDir = ConvertNameToFull(strSetDir);
		PrepareDiskPath(strSetDir, ResolvePath);

		if (HasPathPrefix(strSetDir) && strSetDir[5] == L':' && !strSetDir[6])
			AddEndSlash(strSetDir);
	}

	if (!IsParent && !IsRelativeRoot(strSetDir))
		UpperFolderTopFile=m_CurTopFile;

	if (m_SelFileCount>0)
		ClearSelection();

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
		/* $ 16.01.2002 VVM
		  + Если у плагина нет OPIF_REALNAMES, то история папок не пишется в реестр */
		string strInfoCurDir = NullToEmpty(m_CachedOpenPanelInfo.CurDir);
		string strInfoHostFile = NullToEmpty(m_CachedOpenPanelInfo.HostFile);

		/* $ 25.04.01 DJ
		   при неудаче SetDirectory не сбрасываем выделение
		*/
		bool SetDirectorySuccess = true;
		bool GoToPanelFile = false;
		bool PluginClosed=false;

		if (IsParent && (strInfoCurDir.empty()
			// BUGBUG this breaks exiting from a real "\" directory but needed for https://forum.farmanager.com/viewtopic.php?p=86267#p86267
			 || IsRelativeRoot(strInfoCurDir)
			))
		{
			if (ProcessPluginEvent(FE_CLOSE,nullptr))
				return true;

			PluginClosed=true;
			strFindDir = strInfoHostFile;

			if (strFindDir.empty() && (m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES) && m_CurFile < static_cast<int>(m_ListData.size()))
			{
				strFindDir = m_ListData[m_CurFile].FileName;
				GoToPanelFile=true;
			}

			PopPlugin(TRUE);
			const auto AnotherPanel = Parent()->GetAnotherPanel(this);

			if (AnotherPanel->GetType() == panel_type::INFO_PANEL)
				AnotherPanel->Redraw();
		}
		else
		{
			strFindDir = strInfoCurDir;
			auto opmode = static_cast<int>(OfpType == OFP_ALTERNATIVE ? OPM_PGDN : OPM_NONE);

			SetDirectorySuccess = Global->CtrlObject->Plugins->SetDirectory(GetPluginHandle(), strSetDir, opmode, DataItem) != FALSE;
		}

		// после закрытия панели нужно сразу установить внутренний каталог, иначе будет "Cannot find the file" - Mantis#1731
		if (m_PanelMode == panel_mode::NORMAL_PANEL)
			SetCurPath();

		if (SetDirectorySuccess)
			Update(0);
		else
			Update(UPDATE_KEEP_SELECTION);

		PopPrevData(strFindDir, PluginClosed, !GoToPanelFile, IsParent, SetDirectorySuccess);
		IsPopPlugin = true;

		return SetDirectorySuccess;
	}
	else
	{
		if (IsParent)
		{
			if (RootPath)
			{
				if (NetPath)
				{
					auto ShareName = m_CurDir; // strCurDir can be altered during next call
					DeleteEndSlash(ShareName);
					if (Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Network.Id,OPEN_FILEPANEL, UNSAFE_CSTR(ShareName))) // NetWork Plugin :-)
					{
						return false;
					}
				}
				if(DrivePath && Global->Opt->PgUpChangeDisk == 2)
				{
					string RemoteName;
					if(DriveLocalToRemoteName(true, m_CurDir, RemoteName))
					{
						if (Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Network.Id, OPEN_FILEPANEL, UNSAFE_CSTR(RemoteName))) // NetWork Plugin :-)
						{
							return false;
						}
					}
				}
				ChangeDisk(Parent()->ActivePanel());
				return true;
			}
		}
	}

	strFindDir = PointToName(m_CurDir);
	/*
		// вот и зачем это? мы уже и так здесь, в Options.Folder
		// + дальше по тексту strSetDir уже содержит полный путь
		if ( strSetDir.empty() || strSetDir[1] != L':' || !path::is_separator(strSetDir[2]))
			FarChDir(Options.Folder);
	*/
	/* $ 26.04.2001 DJ
	   проверяем, удалось ли сменить каталог, и обновляем с KEEP_SELECTION,
	   если не удалось
	*/
	int UpdateFlags = 0;
	bool SetDirectorySuccess = true;

	if (m_PanelMode != panel_mode::PLUGIN_PANEL && IsRelativeRoot(strSetDir))
	{
		strSetDir = extract_root_directory(m_CurDir);
	}

	if (!FarChDir(strSetDir))
	{
		if (!Silent && Global->WindowManager->ManagerStarted())
		{
			/* $ 03.11.2001 IS Укажем имя неудачного каталога */
			Message(MSG_WARNING, last_error(),
				msg(lng::MError),
				{
					IsParent? L".."s : strSetDir
				},
				{ lng::MOk });
			UpdateFlags = UPDATE_KEEP_SELECTION;
		}

		SetDirectorySuccess=false;
	}

	m_CurDir = os::fs::GetCurrentDirectory();

	if (!IsUpdated)
		return SetDirectorySuccess;

	Update(UpdateFlags);

	if (IsParent)
	{
		GoToFile(strFindDir);
		m_CurTopFile=UpperFolderTopFile;
		UpperFolderTopFile=0;
		CorrectPosition();
	}
	else if (UpdateFlags != UPDATE_KEEP_SELECTION)
		m_CurFile=m_CurTopFile=0;

	if (IsFocused())
	{
		Parent()->GetCmdLine()->SetCurDir(m_CurDir);
		Parent()->GetCmdLine()->Refresh();
	}

	const auto AnotherPanel = Parent()->GetAnotherPanel(this);

	if (AnotherPanel->GetType() != panel_type::FILE_PANEL)
	{
		AnotherPanel->SetCurDir(m_CurDir, false);
		AnotherPanel->Redraw();
	}

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		Parent()->RedrawKeyBar();

	return SetDirectorySuccess;
}

bool FileList::ChangeDir(string_view const NewDir, bool IsParent)
{
	return ChangeDir(NewDir, IsParent, false, true, nullptr, OFP_NORMAL, false);
}

bool FileList::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!IsMouseInClientArea(MouseEvent))
		return false;

	elevation::instance().ResetApprove();

	if (IsVisible() && Global->Opt->ShowColumnTitles && !MouseEvent->dwEventFlags &&
		MouseEvent->dwMousePosition.Y == m_Where.top + 1 &&
		MouseEvent->dwMousePosition.X > m_Where.left && MouseEvent->dwMousePosition.X < m_Where.left + 3)
	{
		if (MouseEvent->dwButtonState)
		{
			if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
				ChangeDisk(shared_from_this());
			else
				SelectSortMode();
		}

		return true;
	}

	if (IsVisible() && Global->Opt->ShowPanelScrollbar && IntKeyState.MousePos.x == m_Where.right &&
	        (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && !(MouseEvent->dwEventFlags & MOUSE_MOVED) && !IsDragging())
	{
		const auto ScrollY = m_Where.top + 1 + Global->Opt->ShowColumnTitles;

		if (IntKeyState.MousePos.y == ScrollY)
		{
			// Press and hold the [▲] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_UP));
				return true;
			});

			Parent()->SetActivePanel(shared_from_this());
			return true;
		}
		else if (IntKeyState.MousePos.y == ScrollY + m_Height - 1)
		{
			// Press and hold the [▼] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_DOWN));
				return true;
			});

			Parent()->SetActivePanel(shared_from_this());
			return true;
		}
		else if (IntKeyState.MousePos.y > ScrollY && IntKeyState.MousePos.y < ScrollY + m_Height - 1 && m_Height > 2)
		{
			// Drag the thumb
			while (IsMouseButtonPressed() == FROM_LEFT_1ST_BUTTON_PRESSED)
			{
				m_CurFile = static_cast<int>((m_ListData.size() - 1)*(IntKeyState.MousePos.y - ScrollY) / (m_Height - 2));
				ShowFileList();
				Parent()->SetActivePanel(shared_from_this());
			}

			return true;
		}
	}

	static bool delayedShowEMenu = false;
	if (delayedShowEMenu && MouseEvent->dwButtonState == 0)
	{
		delayedShowEMenu = false;
		Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, const_cast<COORD*>(&MouseEvent->dwMousePosition));
	}

	if (Panel::ProcessMouseDrag(MouseEvent))
		return true;

	if (!(MouseEvent->dwButtonState & MOUSE_ANY_BUTTON_PRESSED))
		return false;

	if (
		MouseEvent->dwMousePosition.Y > m_Where.top + Global->Opt->ShowColumnTitles &&
		MouseEvent->dwMousePosition.Y < m_Where.bottom - 2 * Global->Opt->ShowPanelStatus
		)
	{
		Parent()->SetActivePanel(shared_from_this());

		if (m_ListData.empty())
			return true;

		MoveToMouse(MouseEvent);
		assert(m_CurFile < static_cast<int>(m_ListData.size()));

		if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
		        MouseEvent->dwEventFlags==DOUBLE_CLICK)
		{
			if (m_PanelMode == panel_mode::PLUGIN_PANEL)
			{
				FlushInputBuffer(); // !!!
				INPUT_RECORD rec;
				FarKeyToInputRecord({VK_RETURN, IntKeyState.ShiftPressed()? SHIFT_PRESSED : 0u}, &rec);
				if (Global->CtrlObject->Plugins->ProcessKey(GetPluginHandle(), &rec, false))
					return true;
			}

			/*$ 21.02.2001 SKV
			  Если пришел DOUBLE_CLICK без предшествующего ему
			  простого клика, то курсор не перерисовывается.
			  Перерисуем его.
			  По идее при нормальном DOUBLE_CLICK, будет
			  двойная перерисовка...
			  Но мы же вызываем Fast=TRUE...
			  Вроде всё должно быть ок.
			*/
			ShowFileList();
			FlushInputBuffer();
			ProcessEnter(true, IntKeyState.ShiftPressed(), true, IntKeyState.OnlyCtrlAltPressed(), OFP_NORMAL);
			return true;
		}
		else
		{
			/* $ 11.09.2000 SVS
			   Bug #17: Выделяем при условии, что колонка ПОЛНОСТЬЮ пуста.
			*/
			if ((MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) && !empty)
			{
				const DWORD control = MouseEvent->dwControlKeyState&(SHIFT_PRESSED|LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED);

				//вызовем EMenu если он есть
				if (!Global->Opt->RightClickSelect && MouseEvent->dwButtonState == RIGHTMOST_BUTTON_PRESSED)
				{
					if ((!control || control==SHIFT_PRESSED) && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu.Id))
					{
						ShowFileList();

						delayedShowEMenu = GetAsyncKeyState(VK_RBUTTON)<0 || GetAsyncKeyState(VK_LBUTTON)<0 || GetAsyncKeyState(VK_MBUTTON)<0;
						if (!delayedShowEMenu) // show immediately if all mouse buttons released
							Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, const_cast<COORD*>(&MouseEvent->dwMousePosition));

						return true;
					}
				}

				if (!MouseEvent->dwEventFlags || MouseEvent->dwEventFlags==DOUBLE_CLICK)
					MouseSelection = !m_ListData[m_CurFile].Selected;

				Select(m_ListData[m_CurFile], MouseSelection);

				if (SelectedFirst)
					SortFileList(true);
			}
		}

		ShowFileList();
		return true;
	}

	if (MouseEvent->dwMousePosition.Y <= m_Where.top + 1)
	{
		Parent()->SetActivePanel(shared_from_this());

		if (m_ListData.empty())
			return true;

		while_mouse_button_pressed([&](DWORD)
		{
			if (IntKeyState.MousePos.y > m_Where.top + 1)
				return false;

			MoveCursorAndShow(-1);

			if (IntKeyState.MouseButtonState==RIGHTMOST_BUTTON_PRESSED)
			{
				assert(m_CurFile < static_cast<int>(m_ListData.size()));
				Select(m_ListData[m_CurFile], MouseSelection);
			}

			return true;
		});

		if (SelectedFirst)
			SortFileList(true);

		return true;
	}

	if (MouseEvent->dwMousePosition.Y >= m_Where.bottom - 2)
	{
		Parent()->SetActivePanel(shared_from_this());

		if (m_ListData.empty())
			return true;

		while_mouse_button_pressed([&](DWORD)
		{
			if (IntKeyState.MousePos.y < m_Where.bottom - 2)
				return false;

			MoveCursorAndShow(1);

			if (IntKeyState.MouseButtonState==RIGHTMOST_BUTTON_PRESSED)
			{
				assert(m_CurFile < static_cast<int>(m_ListData.size()));
				Select(m_ListData[m_CurFile], MouseSelection);
			}

			return true;
		});

		if (SelectedFirst)
			SortFileList(true);

		return true;
	}

	return false;
}


/* $ 12.09.2000 SVS
  + Опциональное поведение для правой клавиши мыши на пустой панели
*/
void FileList::MoveToMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	int CurColumn=1,ColumnsWidth = 0;
	const auto PanelX = MouseEvent->dwMousePosition.X - m_Where.left - 1;
	int Level = 0;

	for (const auto& i: m_ViewSettings.PanelColumns)
	{
		if (Level == m_ColumnsInStripe)
		{
			CurColumn++;
			Level = 0;
		}

		ColumnsWidth += i.width;

		if (ColumnsWidth>=PanelX)
			break;

		ColumnsWidth++;
		Level++;
	}

//  if (!CurColumn)
//    CurColumn=1;
	const auto OldCurFile = m_CurFile;
	m_CurFile = m_CurTopFile + MouseEvent->dwMousePosition.Y - m_Where.top - 1 - Global->Opt->ShowColumnTitles;

	if (CurColumn>1)
		m_CurFile+=(CurColumn-1)*m_Height;

	CorrectPosition();

	/* $ 11.09.2000 SVS
	   Bug #17: Проверим на ПОЛНОСТЬЮ пустую колонку.
	*/
	if (Global->Opt->PanelRightClickRule == 1)
		empty=((CurColumn-1)*m_Height > static_cast<int>(m_ListData.size()));
	else if (Global->Opt->PanelRightClickRule == 2 &&
	         (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) &&
	         ((CurColumn-1)*m_Height > static_cast<int>(m_ListData.size())))
	{
		m_CurFile=OldCurFile;
		empty = true;
	}
	else
		empty = false;
}

void FileList::SetViewMode(int Mode)
{
	if (static_cast<size_t>(Mode) >= Global->Opt->ViewSettings.size())
		Mode=VIEW_0;

	const auto CurFullScreen = IsFullScreen();
	const auto OldOwner = IsColumnDisplayed(column_type::owner);
	const auto OldPacked = IsColumnDisplayed(column_type::size_compressed);
	const auto OldNumLink = IsColumnDisplayed(column_type::links_number);
	const auto OldNumStreams = IsColumnDisplayed(column_type::streams_number);
	const auto OldStreamsSize = IsColumnDisplayed(column_type::streams_size);
	const auto OldDiz = IsColumnDisplayed(column_type::description);
	PrepareViewSettings(Mode);
	const auto NewOwner = IsColumnDisplayed(column_type::owner);
	auto NewPacked = IsColumnDisplayed(column_type::size_compressed);
	const auto NewNumLink = IsColumnDisplayed(column_type::links_number);
	const auto NewNumStreams = IsColumnDisplayed(column_type::streams_number);
	const auto NewStreamsSize = IsColumnDisplayed(column_type::streams_size);
	const auto NewDiz = IsColumnDisplayed(column_type::description);
	const auto NewAccessTime = IsColumnDisplayed(column_type::date_access);

	DWORD FileSystemFlags = 0;
	if (NewPacked && os::fs::GetVolumeInformation(GetPathRoot(m_CurDir), nullptr, nullptr, nullptr, &FileSystemFlags, nullptr))
		if (!(FileSystemFlags&FILE_FILE_COMPRESSION))
			NewPacked = false;

	if (!m_ListData.empty() && m_PanelMode != panel_mode::PLUGIN_PANEL &&
	        ((!OldOwner && NewOwner) || (!OldPacked && NewPacked) ||
	         (!OldNumLink && NewNumLink) ||
	         (!OldNumStreams && NewNumStreams) ||
	         (!OldStreamsSize && NewStreamsSize) ||
	         IsColumnDisplayed(column_type::custom_0) ||
	         (AccessTimeUpdateRequired && NewAccessTime)))
		Update(UPDATE_KEEP_SELECTION);

	if (!OldDiz && NewDiz)
		ReadDiz();

	if ((m_ViewSettings.Flags&PVS_FULLSCREEN) && !CurFullScreen)
	{
		if (m_Where.bottom > 0)
			SetPosition({ 0, m_Where.top, ScrX, m_Where.bottom });

		m_ViewMode=Mode;
	}
	else
	{
		if (!(m_ViewSettings.Flags&PVS_FULLSCREEN) && CurFullScreen)
		{
			if (m_Where.bottom > 0)
			{
				if (Parent()->IsLeft(shared_from_this()))
					SetPosition({ 0, m_Where.top, static_cast<int>(ScrX / 2 - Global->Opt->WidthDecrement), m_Where.bottom });
				else
					SetPosition({ static_cast<int>(ScrX / 2 + 1 - Global->Opt->WidthDecrement), m_Where.top, ScrX, m_Where.bottom });
			}

			m_ViewMode=Mode;
		}
		else
		{
			m_ViewMode=Mode;
			Global->WindowManager->RefreshWindow();
		}
	}

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		//SetScreenPosition();
		const auto Result = SerialiseViewSettings(m_ViewSettings.PanelColumns);
		ProcessPluginEvent(FE_CHANGEVIEWMODE, UNSAFE_CSTR(Result.first));
	}
}

void FileList::ApplySortMode(panel_sort Mode)
{
	m_SortMode = Mode;

	if (!m_ListData.empty())
		SortFileList(true);

	ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
	Global->WindowManager->RefreshWindow();
}

void FileList::SetSortMode(panel_sort Mode, bool KeepOrder)
{
	if (Mode < panel_sort::COUNT)
	{
		if (!KeepOrder)
		{
			m_ReverseSortOrder = (m_SortMode == Mode && Global->Opt->AllowReverseSort)?
				!m_ReverseSortOrder :
				Global->Opt->PanelSortLayers[static_cast<size_t>(Mode)].front().second == sort_order::descend;
		}

		ApplySortMode(Mode);
	}
	else if (Mode >= panel_sort::BY_USER)
	{
		SetCustomSortMode(Mode, KeepOrder? sort_order::keep : sort_order::flip_or_default, false);
	}
}

void FileList::SetCustomSortMode(panel_sort const Mode, sort_order const Order, bool const InvertByDefault)
{
	if (Mode < panel_sort::BY_USER)
		return;

	switch (Order)
	{
	default:
	case sort_order::flip_or_default:
		m_ReverseSortOrder = (Mode == m_SortMode && Global->Opt->AllowReverseSort)? !m_ReverseSortOrder : InvertByDefault;
		break;

	case sort_order::keep:
		break;

	case sort_order::ascend:
		m_ReverseSortOrder = false;
		break;

	case sort_order::descend:
		m_ReverseSortOrder = true;
		break;
	}

	ApplySortMode(Mode);
}

void FileList::ChangeDirectoriesFirst(bool Mode)
{
	Panel::ChangeDirectoriesFirst(Mode);
	SortFileList(true);
	ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
	Show();
}

void FileList::OnSortingChange()
{
	Panel::OnSortingChange();
	SortFileList(true);
	ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
	if (IsVisible())
		Show();
}

void FileList::InitCurDir(string_view CurDir)
{
	Panel::InitCurDir(CurDir);
	InitFSWatcher(false);
}

bool FileList::GoToFile(long idxItem)
{
	if (static_cast<size_t>(idxItem) >= m_ListData.size())
		return false;

	m_CurFile=idxItem;
	CorrectPosition();
	return true;
}

bool FileList::GoToFile(const string_view Name, const bool OnlyPartName)
{
	return GoToFile(FindFile(Name,OnlyPartName));
}

long FileList::FindFile(const string_view Name, const bool OnlyPartName)
{
	long II = -1;
	for (const auto& I: irange(m_ListData.size()))
	{
		const auto CurPtrName = OnlyPartName? PointToName(m_ListData[I].FileName) : m_ListData[I].FileName;

		if (Name == CurPtrName)
			return static_cast<long>(I);

		if (II < 0 && equal_icase(Name, CurPtrName))
			II = static_cast<long>(I);
	}

	return II;
}

long FileList::FindFirst(string_view const Name)
{
	return FindNext(0, Name);
}

long FileList::FindNext(int StartPos, string_view const Name)
{
	if (static_cast<size_t>(StartPos) >= m_ListData.size())
		return -1;

	for (const auto& I: irange(StartPos, m_ListData.size()))
	{
		if (CmpName(Name, m_ListData[I].FileName, true) && !IsParentDirectory(m_ListData[I]))
			return static_cast<long>(I);
	}

	return -1;
}


bool FileList::IsSelected(string_view const Name)
{
	const long Pos = FindFile(Name);
	return Pos!=-1 && (m_ListData[Pos].Selected || (!m_SelFileCount && Pos==m_CurFile));
}

bool FileList::IsSelected(size_t idxItem)
{
	return idxItem < m_ListData.size() && m_ListData[idxItem].Selected; //  || (Sel!FileCount && idxItem==CurFile) ???
}

bool FileList::FilterIsEnabled()
{
	return m_Filter && m_Filter->IsEnabledOnPanel();
}

bool FileList::FileInFilter(size_t idxItem)
{
	return idxItem < m_ListData.size() && (!m_Filter || !m_Filter->IsEnabledOnPanel() || m_Filter->FileInFilter(m_ListData[idxItem])); // BUGBUG, cast
}

// $ 02.08.2000 IG  Wish.Mix #21 - при нажатии '/' или '\' в QuickSerach переходим на директорию
bool FileList::FindPartName(string_view const Name,int Next,int Direct)
{
	if constexpr (!features::mantis_698) {

	int DirFind = 0;
	string_view NameView = Name;

	if (!Name.empty() && path::is_separator(Name.back()))
	{
		DirFind = 1;
		NameView.remove_suffix(1);
	}

	const auto strMask = exclude_sets(NameView + L'*');

	const auto Match = [&](int const I)
	{
		if (CmpName(strMask, m_ListData[I].FileName, true, false))
		{
			if (!IsParentDirectory(m_ListData[I]))
			{
				if (!DirFind || (m_ListData[I].Attributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					m_CurFile = I;
					m_CurTopFile = m_CurFile - (m_Where.height() - 1) / 2;
					ShowFileList();
					return true;
				}
			}
		}
		return false;
	};


	for (int I=m_CurFile+(Next?Direct:0); I >= 0 && I < static_cast<int>(m_ListData.size()); I+=Direct)
	{
		if (Match(I))
			return true;
	}

	for (int I=(Direct > 0)?0:static_cast<int>(m_ListData.size()-1); (Direct > 0) ? I < m_CurFile:I > m_CurFile; I+=Direct)
	{
		if (Match(I))
			return true;
	}

	return false;

	} else {

	// АХТУНГ! В разработке
	string Dest;
	int DirFind = 0;
	string strMask = upper(Name);

	if (!Name.empty() && path::is_separator(Name.back()))
	{
		DirFind = 1;
		strMask.pop_back();
	}

/*
	strMask = exclude_sets(strMask + L'*');
*/

	for (int I = m_CurFile + (Next ? Direct : 0); I >= 0 && static_cast<size_t>(I) < m_ListData.size(); I += Direct)
	{
		if (GetPlainString(Dest,I) && contains(upper(Dest), strMask))
		//if (CmpName(strMask,ListData[I].FileName,true,I==CurFile))
		{
			if (!IsParentDirectory(m_ListData[I]))
			{
				if (!DirFind || (m_ListData[I].Attributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					m_CurFile=I;
					m_CurTopFile = m_CurFile - (m_Where.height() - 1) / 2;
					ShowFileList();
					return true;
				}
			}
		}
	}

	for (int I = (Direct > 0)? 0 : static_cast<int>(m_ListData.size() - 1); (Direct > 0)? I < m_CurFile : I > m_CurFile; I += Direct)
	{
		if (
			!GetPlainString(Dest, I) ||
			!contains(upper(Dest), strMask) ||
			IsParentDirectory(m_ListData[I]) ||
			(DirFind && !(m_ListData[I].Attributes & FILE_ATTRIBUTE_DIRECTORY))
		)
			continue;

		m_CurFile = I;
		m_CurTopFile = m_CurFile - (m_Where.height() - 1) / 2;
		ShowFileList();
		return true;
	}

	return false;

	}
}

// собрать в одну строку все данные в отображаемых колонках
bool FileList::GetPlainString(string& Dest, int ListPos) const
{
	Dest.clear();

	if constexpr (features::mantis_698)
	{
		if (static_cast<size_t>(ListPos) >= m_ListData.size())
			return false;

		for (const auto& Column : m_ViewSettings.PanelColumns)
		{
			if (Column.type >= column_type::custom_0 && Column.type <= column_type::custom_max)
			{
				const size_t ColumnNumber = static_cast<size_t>(Column.type) - static_cast<size_t>(column_type::custom_0);
				if (ColumnNumber < m_ListData[ListPos].CustomColumns.size())
					Dest += m_ListData[ListPos].CustomColumns[ColumnNumber];

				continue;
			}

			switch (Column.type)
			{
			case column_type::name:
			{
				string_view Name = m_ListData[ListPos].AlternateOrNormal(m_ShowShortNames);

				if (!(m_ListData[ListPos].Attributes & FILE_ATTRIBUTE_DIRECTORY) && Column.type_flags & COLFLAGS_NOEXTENSION)
				{
					Name = name_ext(Name).first;
				}

				if (Column.type_flags & COLFLAGS_NAMEONLY)
				{
					//BUGBUG!!!
					// !!! НЕ УВЕРЕН, но то, что отображается пустое
					// пространство вместо названия - бага
					Name = PointToFolderNameIfFolder(Name);
				}

				Dest += Name;
				break;
			}

			case column_type::extension:
			{
				string_view Ext;
				if (!(m_ListData[ListPos].Attributes & FILE_ATTRIBUTE_DIRECTORY))
					Ext = name_ext(m_ListData[ListPos].AlternateOrNormal(m_ShowShortNames)).second;

				if (!Ext.empty())
					Ext.remove_prefix(1);

				Dest += Ext;
				break;
			}

			case column_type::size:
			case column_type::size_compressed:
			case column_type::streams_size:
			{
				const auto SizeToDisplay = (Column.type == column_type::size_compressed) ?
					m_ListData[ListPos].AllocationSize :
					Column.type == column_type::streams_size ?
					m_ListData[ListPos].StreamsSize(this) :
					m_ListData[ListPos].FileSize;

				Dest += FormatStr_Size(
					SizeToDisplay,
					m_ListData[ListPos].FileName,
					m_ListData[ListPos].Attributes,
					m_ListData[ListPos].ShowFolderSize,
					m_ListData[ListPos].ReparseTag,
					Column.type,
					Column.type_flags,
					Column.width, // BUGBUG width_type
					m_CurDir);
				break;
			}

			case column_type::date:
			case column_type::time:
			case column_type::date_write:
			case column_type::date_creation:
			case column_type::date_access:
			case column_type::date_change:
			{
				os::chrono::time_point const FileListItem::* FileTime;

				switch (Column.type)
				{
				case column_type::date_creation:
					FileTime = &FileListItem::CreationTime;
					break;

				case column_type::date_access:
					FileTime = &FileListItem::LastAccessTime;
					break;

				case column_type::date_change:
					FileTime = &FileListItem::ChangeTime;
					break;

				default:
					FileTime = &FileListItem::LastWriteTime;
					break;
				}

				Dest += FormatStr_DateTime(std::invoke(FileTime, m_ListData[ListPos]), Column.type, Column.type_flags, Column.width); // BUGBUG width_type
				break;
			}

			case column_type::attributes:
				Dest += FormatStr_Attribute(m_ListData[ListPos].Attributes, Column.width); // BUGBUG width_type
				break;

			case column_type::description:
				Dest += NullToEmpty(m_ListData[ListPos].DizText);
				break;

			case column_type::owner:
				Dest += m_ListData[ListPos].Owner(this);
				break;

			case column_type::links_number:
				Dest += str(m_ListData[ListPos].NumberOfLinks(this));
				break;

			case column_type::streams_number:
				Dest += str(m_ListData[ListPos].NumberOfStreams(this));
				break;

			default:
				break; // BUGBUG?
			}
		}

		return true;
	}

	return false;
}

size_t FileList::GetSelCount() const
{
	assert(m_ListData.empty() || !(ReturnCurrentFile||!m_SelFileCount) || (m_CurFile < static_cast<int>(m_ListData.size())));

	return !m_ListData.empty()? ((ReturnCurrentFile || !m_SelFileCount)? (IsParentDirectory(m_ListData[m_CurFile])? 0 : 1) : m_SelFileCount) : 0;
}

size_t FileList::GetRealSelCount() const
{
	return !m_ListData.empty()? m_SelFileCount : 0;
}

bool FileList::GetSelName(string* strName, string* strShortName, os::fs::find_data* fd)
{
	if (!strName)
	{
		GetSelPosition=0;
		LastSelPosition=-1;
		return true;
	}

	const auto CopyFrom = [&](const FileListItem& Src)
	{
		*strName = Src.FileName;

		if (strShortName)
			*strShortName = Src.AlternateFileName();

		if (fd)
			*fd = Src;
	};

	if (!m_SelFileCount || ReturnCurrentFile)
	{
		if (GetSelPosition || m_CurFile >= static_cast<int>(m_ListData.size()))
			return false;

		GetSelPosition=1;
		CopyFrom(m_ListData[m_CurFile]);
		LastSelPosition=m_CurFile;
		return true;
	}

	while (GetSelPosition < static_cast<int>(m_ListData.size()))
	{
		if (!m_ListData[GetSelPosition++].Selected)
			continue;

		CopyFrom(m_ListData[GetSelPosition - 1]);
		LastSelPosition = GetSelPosition - 1;
		return true;
	}

	return false;
}


void FileList::ClearLastGetSelection()
{
	if (LastSelPosition < 0 || LastSelPosition >= static_cast<int>(m_ListData.size()))
		return;

	Select(m_ListData[LastSelPosition], false);
}

const FileListItem* FileList::GetLastSelectedItem() const
{
	if (LastSelPosition < 0 || LastSelPosition >= static_cast<int>(m_ListData.size()))
		return nullptr;

	return &m_ListData[LastSelPosition];
}

bool FileList::GetCurName(string &strName, string &strShortName) const
{
	if (m_ListData.empty())
		return false;

	assert(m_CurFile < static_cast<int>(m_ListData.size()));

	strName = m_ListData[m_CurFile].FileName;
	strShortName = m_ListData[m_CurFile].AlternateFileName();
	return true;
}

bool FileList::GetCurBaseName(string &strName, string &strShortName) const
{
	if (m_ListData.empty())
		return false;

	if (m_PanelMode == panel_mode::PLUGIN_PANEL && !PluginsList.empty()) // для плагинов
	{
		strName = PointToName(PluginsList.front()->m_HostFile);
		strShortName = strName;
	}
	else if (m_PanelMode == panel_mode::NORMAL_PANEL)
	{
		assert(m_CurFile < static_cast<int>(m_ListData.size()));

		strName = m_ListData[m_CurFile].FileName;
		strShortName = m_ListData[m_CurFile].AlternateFileName();
	}

	return true;
}

bool FileList::HardlinksSupported() const
{
	return m_HardlinksSupported;
}

bool FileList::StreamsSupported() const
{
	return m_StreamsSupported;
}

const string& FileList::GetComputerName() const
{
	return m_ComputerName;
}

long FileList::SelectFiles(int Mode, string_view const Mask)
{
	enum
	{
		sf_doublebox,
		sf_edit,
		sf_separator,
		sf_button_ok,
		sf_button_filter,
		sf_button_cancel,

		sf_count
	};

	auto SelectDlg = MakeDialogItems<sf_count>(
	{
		{ DI_DOUBLEBOX, {{3,  1}, {51, 5}}, DIF_NONE, },
		{ DI_EDIT,      {{5,  2}, {49, 2}}, DIF_FOCUS | DIF_HISTORY, },
		{ DI_TEXT,      {{-1, 3}, {0,  3}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,  4}, {0,  4}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,    {{0,  4}, {0,  4}}, DIF_CENTERGROUP, msg(lng::MSelectFilter), },
		{ DI_BUTTON,    {{0,  4}, {0,  4}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	SelectDlg[sf_edit].strHistory = L"Masks"sv;

	multifilter Filter(this, FFT_SELECT);
	bool bUseFilter = false;
	static auto strPrevMask = L"*.*"s;
	/* $ 20.05.2002 IS
	   При обработке маски, если работаем с именем файла на панели,
	   берем каждую квадратную скобку в имени при образовании маски в скобки,
	   чтобы подобные имена захватывались полученной маской - это специфика,
	   диктуемая CmpName.
	*/
	auto strMask = L"*.*"s;
	string strRawMask;
	bool WrapBrackets=false; // говорит о том, что нужно взять кв.скобки в скобки

	if (m_CurFile >= static_cast<int>(m_ListData.size()))
		return 0;

	filemasks FileMask; // Класс для работы с масками

	int RawSelection=FALSE;

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
		RawSelection=(m_CachedOpenPanelInfo.Flags & OPIF_RAWSELECTION);
	}

	const auto& strCurName = m_ListData[m_CurFile].AlternateOrNormal(m_ShowShortNames);

	if (Mode==SELECT_ADDEXT || Mode==SELECT_REMOVEEXT)
	{
		const auto pos = strCurName.rfind(L'.');

		if (pos != string::npos)
		{
			// Учтем тот момент, что расширение может содержать символы-разделители
			strRawMask = concat(L'"', L"*."sv, string_view(strCurName).substr(pos + 1), L'"');
			WrapBrackets=true;
		}
		else
		{
			strMask = L"*."sv;
		}

		Mode=(Mode==SELECT_ADDEXT) ? SELECT_ADD:SELECT_REMOVE;
	}
	else
	{
		if (Mode==SELECT_ADDNAME || Mode==SELECT_REMOVENAME)
		{
			// Учтем тот момент, что имя может содержать символы-разделители
			strRawMask = concat(L"\""sv, strCurName);
			const auto pos = strRawMask.rfind(L'.');

			if (pos != string::npos && pos!=strRawMask.size()-1)
				strRawMask.resize(pos);

			append(strRawMask, L".*\""sv);
			WrapBrackets=true;
			Mode=(Mode==SELECT_ADDNAME) ? SELECT_ADD:SELECT_REMOVE;
		}
		else
		{
			if (Mode==SELECT_ADD || Mode==SELECT_REMOVE)
			{
				SelectDlg[sf_edit].strData = strPrevMask;
				SelectDlg[sf_doublebox].strData = msg(Mode == SELECT_ADD? lng::MSelectTitle : lng::MUnselectTitle);

				{
					const auto Dlg = Dialog::create(SelectDlg);
					Dlg->SetHelp(L"SelectFiles"sv);
					Dlg->SetPosition({ -1, -1, 55, 7 });
					Dlg->SetId(Mode==SELECT_ADD?SelectDialogId:UnSelectDialogId);

					for (;;)
					{
						Dlg->ClearDone();
						Dlg->Process();

						if (Dlg->GetExitCode() == sf_button_filter)
						{
							filters::EditFilters(Filter.area(), Filter.panel());
							//Рефреш текущему времени для фильтра сразу после выхода из диалога
							Filter.UpdateCurrentTime();
							bUseFilter = true;
							break;
						}

						if (Dlg->GetExitCode() != sf_button_ok)
							return 0;

						strMask = SelectDlg[sf_edit].strData;

						if (FileMask.assign(strMask)) // Проверим вводимые пользователем маски на ошибки
						{
							strPrevMask = strMask;
							break;
						}
					}
				}
			}
			else if (Mode==SELECT_ADDMASK || Mode==SELECT_REMOVEMASK || Mode==SELECT_INVERTMASK)
			{
				strMask = Mask;

				if (!FileMask.assign(strMask)) // Проверим маски на ошибки
					return 0;
			}
		}
	}

	SaveSelection();

	if (!bUseFilter && WrapBrackets) // возьмем кв.скобки в скобки, чтобы получить
	{                               // работоспособную маску
		strMask.clear();
		for (const auto& i: strRawMask)
		{
			if (i == L']' || i == L'[')
			{
				append(strMask, L'[', i, L']');
			}
			else
			{
				strMask += i;
			}
		}
	}

	long workCount=0;

	if (bUseFilter || FileMask.assign(strMask, FMF_SILENT)) // Скомпилируем маски файлов и работаем
	{                                                // дальше в зависимости от успеха компиляции
		for (auto& i: m_ListData)
		{
			if (
				Mode != SELECT_INVERT &&
				Mode != SELECT_INVERTALL &&
				Mode != SELECT_INVERTFILES &&
				!(bUseFilter? Filter.FileInFilter(i) : FileMask.check(i.AlternateOrNormal(m_ShowShortNames)))
			)
				continue;

			bool Selection = false;

			switch (Mode)
			{
				case SELECT_ADD:
				case SELECT_ADDMASK:
					Selection = true;
					break;

				case SELECT_REMOVE:
				case SELECT_REMOVEMASK:
					Selection = false;
					break;

				case SELECT_INVERT:
				case SELECT_INVERTALL:
				case SELECT_INVERTFILES:
				case SELECT_INVERTMASK:
					Selection=!i.Selected;
					break;
			}

			if (
				bUseFilter ||
				!(i.Attributes & FILE_ATTRIBUTE_DIRECTORY) ||
				(Global->Opt->SelectFolders && Mode != SELECT_INVERTFILES) ||
				!Selection ||
				RawSelection ||
				Mode == SELECT_INVERTALL ||
				Mode == SELECT_INVERTMASK
			)
			{
				Select(i, Selection);
				workCount++;
			}
		}
	}

	if (SelectedFirst)
		SortFileList(true);

	ShowFileList();

	return workCount;
}

void FileList::UpdateViewPanel()
{
	const auto ViewPanel = std::dynamic_pointer_cast<QuickView>(Parent()->GetAnotherPanel(this));

	if (!ViewPanel || m_ListData.empty() || !ViewPanel->IsVisible() || !SetCurPath())
		return;

	assert(m_CurFile < static_cast<int>(m_ListData.size()));

	const auto& Current = m_ListData[m_CurFile];

	if (m_PanelMode != panel_mode::PLUGIN_PANEL || PluginManager::UseInternalCommand(GetPluginHandle(), PLUGIN_FARGETFILE, m_CachedOpenPanelInfo))
	{
		if (IsParentDirectory(Current))
			ViewPanel->ShowFile(m_CurDir, nullptr, false, nullptr);
		else
			ViewPanel->ShowFile(Current.FileName, &Current.UserData, false, nullptr);
	}
	else if (!(Current.Attributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		const auto strTempDir = MakeTemp();
		// BUGBUG check result
		if (!os::fs::create_directory(strTempDir))
		{
			LOGWARNING(L"create_directory({}): {}"sv, strTempDir, last_error());
		}

		PluginPanelItemHolderHeap PanelItem;
		FileListToPluginItem(Current, PanelItem);
		string strFileName;

		if (!Global->CtrlObject->Plugins->GetFile(GetPluginHandle(), &PanelItem.Item, strTempDir, strFileName, OPM_SILENT | OPM_VIEW | OPM_QUICKVIEW))
		{
			ViewPanel->ShowFile({}, nullptr, false, nullptr);
			// BUGBUG check result
			if (!os::fs::remove_directory(strTempDir))
			{
				LOGWARNING(L"remove_directory({}): {}"sv, strTempDir, last_error());
			}

			return;
		}

		ViewPanel->ShowFile(strFileName, nullptr, true, nullptr);
	}
	else if (!IsParentDirectory(Current))
	{
		ViewPanel->ShowFile(Current.FileName, &Current.UserData, false, GetPluginHandle());
	}
	else
	{
		ViewPanel->ShowFile({}, nullptr, false, nullptr);
	}

	RefreshTitle();
}


void FileList::CompareDir()
{
	const auto Another = std::dynamic_pointer_cast<FileList>(Parent()->GetAnotherPanel(this));

	if (!Another || !Another->IsVisible())
	{
		Message(MSG_WARNING,
			msg(lng::MCompareTitle),
			{
				msg(lng::MCompareFilePanelsRequired1),
				msg(lng::MCompareFilePanelsRequired2)
			},
			{ lng::MOk });
		return;
	}

	Global->ScrBuf->Flush();

	const auto select_files = [&](FileList& Panel)
	{
		for (auto& i: Panel.m_ListData)
		{
			Panel.Select(i, !(i.Attributes & FILE_ATTRIBUTE_DIRECTORY));
		}
	};

	select_files(*this);
	select_files(*Another);

	const auto use_fat_time = [&](FileList& Panel)
	{
		if (Panel.m_PanelMode == panel_mode::PLUGIN_PANEL)
		{
			OpenPanelInfo OpInfo{ sizeof(OpInfo) };
			Global->CtrlObject->Plugins->GetOpenPanelInfo(Panel.GetPluginHandle(), &OpInfo);
			return (OpInfo.Flags & OPIF_COMPAREFATTIME) != 0;
		}
		else
		{
			string FileSystemName;
			return os::fs::GetVolumeInformation(GetPathRoot(Panel.m_CurDir), {}, {}, {}, {}, &FileSystemName) && contains_icase(FileSystemName, L"FAT"sv);
		}
	};

	const auto UseFatTime = use_fat_time(*this) || use_fat_time(*Another);

	// теперь начнем цикл по снятию выделений
	// каждый элемент активной панели...
	for (auto& This: m_ListData)
	{
		if (!This.Selected)
			continue;

		// ...сравниваем с элементом пассивной панели...
		for (auto& That: Another->m_ListData)
		{
			if (!That.Selected)
				continue;

			if (!equal_icase(PointToName(This.FileName), PointToName(That.FileName)))
				continue;

			const auto Cmp = (UseFatTime? compare_fat_write_time : compare_time)(This.LastWriteTime, That.LastWriteTime);

			if (!Cmp && (This.FileSize != That.FileSize))
				continue;

			if (Cmp <= 0)
				Select(This, false);

			if (Cmp >= 0)
				Another->Select(That, false);

			if (Another->m_PanelMode != panel_mode::PLUGIN_PANEL)
				break;
		}
	}

	const auto refresh = [](FileList& Panel)
	{
		if (Panel.GetSelectedFirstMode())
			Panel.SortFileList(true);
		Panel.Redraw();
	};

	refresh(*this);
	refresh(*Another);

	if (!m_SelFileCount && !Another->m_SelFileCount)
		Message(0,
			msg(lng::MCompareTitle),
			{
				msg(lng::MCompareSameFolders1),
				msg(lng::MCompareSameFolders2)
			},
			{ lng::MOk });
}

void FileList::CopyFiles(bool bMoved)
{
	bool RealNames=false;
	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
		RealNames = (m_CachedOpenPanelInfo.Flags&OPIF_REALNAMES) != 0;
	}

	if (m_PanelMode != panel_mode::PLUGIN_PANEL || RealNames)
	{
		string CopyData;
		for (const auto& i: enum_selected())
		{
			string_view Name = i.FileName;

			if (IsParentDirectory(i))
				Name = Name.substr(0, 1);

			append(CopyData, CreateFullPathName(Name, (i.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0, false), L'\0');
		}

		if(!CopyData.empty())
		{
			clipboard_accessor Clip;
			if(Clip->Open())
			{
				Clip->SetHDROP(CopyData, bMoved);
			}
		}
	}
}

void FileList::CopyNames(bool FillPathName, bool UNC)
{
	string CopyData;

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
	}

	const auto Eol = eol::system.str();
	for (const auto& i: enum_selected())
	{
		if (!CopyData.empty())
			append(CopyData, Eol);

		auto strQuotedName = m_ShowShortNames? i.AlternateFileName() : i.FileName;

		if (FillPathName)
		{
			if (m_PanelMode != panel_mode::PLUGIN_PANEL)
			{
				/* $ 14.02.2002 IS
				   ".." в текущем каталоге обработаем как имя текущего каталога
				*/
				if (IsParentDirectory(i))
					strQuotedName.resize(1);

				strQuotedName = CreateFullPathName(strQuotedName, (i.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0, UNC);
			}
			else
			{
				string strFullName = NullToEmpty(m_CachedOpenPanelInfo.CurDir);

				if (Global->Opt->PanelCtrlFRule && (m_ViewSettings.Flags&PVS_FOLDERUPPERCASE))
					inplace::upper(strFullName);

				if (!strFullName.empty())
					AddEndSlash(strFullName);

				if (Global->Opt->PanelCtrlFRule)
				{
					// имя должно отвечать условиям на панели
					if ((m_ViewSettings.Flags&PVS_FILELOWERCASE) && !(i.Attributes & FILE_ATTRIBUTE_DIRECTORY))
						inplace::lower(strQuotedName);

					if (m_ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE)
						if (!(i.Attributes & FILE_ATTRIBUTE_DIRECTORY) && !IsCaseMixed(strQuotedName))
							inplace::lower(strQuotedName);
				}

				strQuotedName.insert(0, strFullName);

				// добавим первый префикс!
				if (m_PanelMode == panel_mode::PLUGIN_PANEL && Global->Opt->SubstPluginPrefix)
				{
					strQuotedName.insert(0, GetPluginPrefix());
				}
			}
		}
		else
		{
			if (IsParentDirectory(i))
			{
				if (m_PanelMode == panel_mode::PLUGIN_PANEL)
				{
					strQuotedName=NullToEmpty(m_CachedOpenPanelInfo.CurDir);
				}
				else
				{
					strQuotedName = GetCurDir();
				}

				strQuotedName = PointToName(strQuotedName);
			}
		}

		if (Global->Opt->QuotedName&QUOTEDNAME_CLIPBOARD)
			inplace::QuoteSpace(strQuotedName);

		CopyData += strQuotedName;
	}

	SetClipboardText(CopyData);
}

void FileList::RefreshTitle()
{
	m_Title = L'{';

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
		append(m_Title, trim(string_view(NullToEmpty(m_CachedOpenPanelInfo.PanelTitle))));
	}
	else
	{
		m_Title += m_CurDir;
	}

	m_Title += L'}';
}

size_t FileList::GetFileCount() const
{
	return m_ListData.size();
}

void FileList::ClearSelection()
{
	for (auto& i: m_ListData)
	{
		Select(i, false);
	}

	if (SelectedFirst)
		SortFileList(true);
}


void FileList::SaveSelection()
{
	for (auto& i: m_ListData)
	{
		i.PrevSelected = i.Selected;
	}
}


void FileList::RestoreSelection()
{
	for (auto& i: m_ListData)
	{
		Select(i, std::exchange(i.PrevSelected, i.Selected));
	}

	if (SelectedFirst)
		SortFileList(true);

	Redraw();
}



bool FileList::GetFileName(string& strName, int Pos, os::fs::attributes& FileAttr) const
{
	if (Pos >= static_cast<int>(m_ListData.size()))
		return false;

	strName = m_ListData[Pos].FileName;
	FileAttr=m_ListData[Pos].Attributes;
	return true;
}

const std::unordered_set<string>* FileList::GetFilteredExtensions() const
{
	return &m_FilteredExtensions;
}

int FileList::GetCurrentPos() const
{
	return m_CurFile;
}

void FileList::EditFilter()
{
	if (!m_Filter)
		m_Filter = std::make_unique<multifilter>(this, FFT_PANEL);

	filters::EditFilters(m_Filter->area(), m_Filter->panel());
}

static int select_sort_layer(std::vector<std::pair<panel_sort, sort_order>> const& SortLayers)
{
	std::vector<menu_item> AvailableSortModesMenuItems(static_cast<size_t>(panel_sort::COUNT));
	auto VisibleCount = AvailableSortModesMenuItems.size();

	for (const auto& i: SortModes)
	{
		auto& Item = AvailableSortModesMenuItems[i.MenuPosition];
		Item.Name = msg(i.Label);

		if (std::any_of(ALL_CONST_RANGE(SortLayers), [&](std::pair<panel_sort, sort_order> const& Layer) { return Layer.first == static_cast<panel_sort>(&i - SortModes); }))
		{
			Item.Flags |= MIF_HIDDEN;
			--VisibleCount;
		}
	}

	if (!VisibleCount)
		return -1;

	const auto AvailableSortModesMenu = VMenu2::create({}, AvailableSortModesMenuItems);
	AvailableSortModesMenu->SetHelp(L"PanelCmdSort"sv);
	AvailableSortModesMenu->SetPosition({ -1, -1, 0, 0 });
	AvailableSortModesMenu->SetMenuFlags(VMENU_WRAPMODE);

	return AvailableSortModesMenu->Run();
}

static void edit_sort_layers(int MenuPos)
{
	if (MenuPos >= static_cast<int>(panel_sort::COUNT))
		return;

	const auto SortMode = std::find_if(CONST_RANGE(SortModes, i){ return i.MenuPosition == MenuPos; }) - SortModes;
	if (static_cast<panel_sort>(SortMode) == panel_sort::UNSORTED)
		return;

	auto& SortLayers = Global->Opt->PanelSortLayers[SortMode];

	std::vector<menu_item> SortLayersMenuItems;
	SortLayersMenuItems.reserve(SortLayers.size());
	std::transform(ALL_CONST_RANGE(SortLayers), std::back_inserter(SortLayersMenuItems), [](std::pair<panel_sort, sort_order> const& Layer)
	{
		return menu_item{ msg(SortModes[static_cast<size_t>(Layer.first)].Label), LIF_CHECKED | order_indicator(Layer.second) };
	});

	SortLayersMenuItems.front().Flags |= LIF_DISABLE;

	const auto SortLayersMenu = VMenu2::create({}, SortLayersMenuItems);

	SortLayersMenu->SetHelp(L"PanelCmdSort"sv);
	SortLayersMenu->SetPosition({ -1, -1, 0, 0 });
	SortLayersMenu->SetMenuFlags(VMENU_WRAPMODE);
	SortLayersMenu->SetBottomTitle(KeysToLocalizedText(KEY_INS, KEY_DEL, KEY_F4, L'+', L'-', L'*', L'=', KEY_CTRLUP, KEY_CTRLDOWN, KEY_CTRLR));

	const auto SetCheck = [&](int const Pos, sort_order const Order)
	{
		SortLayersMenu->SetCustomCheck(order_indicator(Order), SortLayersMenu->GetSelectPos());
		SortLayers[Pos].second = Order;
	};

	SortLayersMenu->Run([&](const Manager::Key& RawKey)
	{
		const auto Pos = SortLayersMenu->GetSelectPos();
		if (!Pos)
			return false;

		switch (const auto Key = RawKey())
		{
		case KEY_INS:
		case KEY_NUMPAD0:
			if (Pos > 0)
			{
				if (const auto Result = select_sort_layer(SortLayers); Result >= 0)
				{
					const auto NewSortModeIndex = std::find_if(CONST_RANGE(SortModes, i) { return i.MenuPosition == Result; }) - SortModes;
					const auto Order = SortModes[NewSortModeIndex].DefaultLayers.begin()->second;
					SortLayersMenu->AddItem(MenuItemEx{ msg(SortModes[NewSortModeIndex].Label), MIF_CHECKED | order_indicator(Order) }, Pos);
					SortLayersMenu->SetSelectPos(Pos);
					SortLayers.emplace(SortLayers.begin() + Pos, static_cast<panel_sort>(NewSortModeIndex), Order);
				}
			}
			break;

		case KEY_DEL:
		case KEY_NUMDEL:
			if (Pos > 0)
			{
				SortLayersMenu->DeleteItem(Pos);
				SortLayers.erase(SortLayers.begin() + Pos);
			}
			break;

		case KEY_F4:
			if (const auto Result = select_sort_layer(SortLayers); Result >= 0)
			{
				const auto NewSortModeIndex = std::find_if(CONST_RANGE(SortModes, i) { return i.MenuPosition == Result; }) - SortModes;
				const auto Order = SortModes[NewSortModeIndex].DefaultLayers.begin()->second;
				SortLayersMenu->at(Pos).Name = msg(SortModes[NewSortModeIndex].Label);
				SortLayersMenu->at(Pos).SetCustomCheck(order_indicator(Order));
				SortLayers[Pos] = { static_cast<panel_sort>(NewSortModeIndex), Order };
			}
			break;

		case L'+':
		case KEY_ADD:
			SetCheck(Pos, sort_order::ascend);
			break;

		case L'-':
		case KEY_SUBTRACT:
			SetCheck(Pos, sort_order::descend);
			break;

		case L'*':
		case KEY_MULTIPLY:
			{
				const auto CurrentOrder = SortLayers[Pos].second;
				const auto NewOrder =
					CurrentOrder == sort_order::ascend?
						sort_order::descend :
						CurrentOrder == sort_order::descend?
							sort_order::ascend :
							CurrentOrder;

				SetCheck(Pos, NewOrder);
			}
			break;

		case L'=':
			if (Pos > 0)
				SetCheck(Pos, sort_order::keep);
			break;

		case KEY_CTRLUP:
		case KEY_RCTRLUP:
		case KEY_CTRLDOWN:
		case KEY_RCTRLDOWN:
			if (Pos > 0)
			{
				const auto OtherPos = Pos + ((Key & KEY_UP) == KEY_UP? -1 : 1);
				if (in_closed_range(1, OtherPos, static_cast<int>(SortLayers.size() - 1)))
				{
					using std::swap;
					swap(SortLayersMenu->at(Pos), SortLayersMenu->at(OtherPos));
					swap(SortLayers[Pos], SortLayers[OtherPos]);
					SortLayersMenu->SetSelectPos(OtherPos);
				}
			}
			break;

		case KEY_CTRLR:
		case KEY_RCTRLR:
			{
				const auto DefaultLayers = default_sort_layers(static_cast<panel_sort>(SortMode));
				SortLayers.assign(ALL_CONST_RANGE(DefaultLayers));
				SortLayersMenu->Close(-1);
				return true;
			}

		default:
			break;
		}

		return false;
	});
}

void FileList::SelectSortMode()
{
	std::vector<menu_item> SortMenu(std::size(SortModes));
	for (const auto& i: SortModes)
	{
		auto& Item = SortMenu[i.MenuPosition];

		Item.Name = msg(i.Label);
		Item.AccelKey = i.MenuKey;
	}

	static const menu_item MenuSeparator = { {}, LIF_SEPARATOR };

	OpenMacroPluginInfo ompInfo = { MCT_GETCUSTOMSORTMODES,nullptr };
	MacroPluginReturn* mpr = nullptr;
	size_t extra = 0; // number of additional menu items due to custom sort modes
	{
		void *ptr;
		if (Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Luamacro.Id, OPEN_LUAMACRO, &ompInfo, &ptr) && ptr)
		{
			mpr = &ompInfo.Ret;
			if (mpr->Count >= 3)
			{
				extra = 1 + mpr->Count/3; // add 1 item for separator

				SortMenu.reserve(SortMenu.size() + extra);

				SortMenu.emplace_back(MenuSeparator);
				for (size_t i=0; i < mpr->Count; i += 3)
				{
					SortMenu.emplace_back(menu_item{ mpr->Values[i + 2].String });
				}
			}
			else
				mpr = nullptr;
		}
	}

	const auto& SetCheckAndSelect = [&](size_t const Index)
	{
		auto& MenuItem = SortMenu[Index];
		MenuItem.SetCustomCheck(order_indicator(m_ReverseSortOrder? sort_order::descend : sort_order::ascend));
		MenuItem.SetSelect(true);
	};

	if (m_SortMode < panel_sort::COUNT)
	{
		SetCheckAndSelect(SortModes[static_cast<size_t>(m_SortMode)].MenuPosition);
	}
	else if (m_SortMode >= panel_sort::BY_USER && mpr)
	{
		for (size_t i=0; i < mpr->Count; i += 3)
		{
			if (static_cast<int>(mpr->Values[i].Double) == static_cast<int>(m_SortMode))
			{
				SetCheckAndSelect(std::size(SortModes) + 1 + i / 3);
				break;
			}
		}
	}
	else
	{
		LOGWARNING(L"Unknown sort mode {}"sv, m_SortMode);
	}

	enum SortOptions
	{
		SortOptUseGroups,
		SortOptSelectedFirst,
		SortOptDirectoriesFirst,

		SortOptCount
	};
	const menu_item InitSortMenuOptions[]=
	{
		{ msg(lng::MMenuSortUseGroups), GetSortGroups()? MIF_CHECKED : 0, KEY_SHIFTF11 },
		{ msg(lng::MMenuSortSelectedFirst), SelectedFirst? MIF_CHECKED : 0, KEY_SHIFTF12 },
		{ msg(lng::MMenuSortDirectoriesFirst), m_DirectoriesFirst? MIF_CHECKED : 0, 0 },
	};
	static_assert(std::size(InitSortMenuOptions) == SortOptCount);

	SortMenu.reserve(SortMenu.size() + 1 + std::size(InitSortMenuOptions)); // + 1 for separator
	SortMenu.emplace_back(MenuSeparator);
	SortMenu.insert(SortMenu.end(), ALL_CONST_RANGE(InitSortMenuOptions));

	int SortCode = -1;
	bool InvertPressed = true;
	bool PlusPressed = false;

	{
		const auto MenuStrings = VMenu::AddHotkeys(SortMenu);

		const auto SortModeMenu = VMenu2::create(msg(lng::MMenuSortTitle), SortMenu);
		SortModeMenu->SetHelp(L"PanelCmdSort"sv);
		SortModeMenu->SetPosition({ m_Where.left + 4, -1, 0, 0 });
		SortModeMenu->SetMenuFlags(VMENU_WRAPMODE);
		SortModeMenu->SetId(SelectSortModeId);
		SortModeMenu->SetBottomTitle(KeysToLocalizedText(L'+', L'-', L'*', KEY_F4));

		SortCode=SortModeMenu->Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			bool KeyProcessed = false;

			switch (Key)
			{
				case L'*':
				case KEY_MULTIPLY:
					KeyProcessed = true;
					break;

				case L'+':
				case KEY_ADD:
				case L'-':
				case KEY_SUBTRACT:
					InvertPressed = false;
					PlusPressed = any_of(Key, L'+', KEY_ADD);
					KeyProcessed = true;
					break;

				case KEY_F4:
					edit_sort_layers(SortModeMenu->GetSelectPos());
					break;

				default:
					break;
			}

			if (KeyProcessed)
			{
				SortModeMenu->Close(SortModeMenu->GetSelectPos());
			}
			return KeyProcessed;
		});
	}

	if (SortCode<0)
	{
		return;
	}

	// predefined sort modes
	if (static_cast<size_t>(SortCode) < std::size(SortModes))
	{
		bool KeepOrder = false;

		if (!InvertPressed)
		{
			m_ReverseSortOrder = !PlusPressed;
			KeepOrder = true;
		}

		const auto SortMode = static_cast<panel_sort>(std::find_if(CONST_RANGE(SortModes, i){ return i.MenuPosition == SortCode; }) - SortModes);
		SetSortMode(SortMode, KeepOrder);
	}
	// custom sort modes
	else if (static_cast<size_t>(SortCode) >= std::size(SortModes) + 1 && static_cast<size_t>(SortCode) < std::size(SortModes) + 1 + extra - 1)
	{
		const auto index = 3*(SortCode-std::size(SortModes)-1);
		const auto mode = static_cast<int>(mpr->Values[index].Double);

		if (custom_sort::CanSort(mode))
		{
			const auto InvertByDefault = mpr->Values[index+1].Boolean != 0;
			auto Order = sort_order::flip_or_default;

			if (!InvertPressed)
			{
				m_ReverseSortOrder = !PlusPressed;
				Order = sort_order::keep;
			}

			SetCustomSortMode(panel_sort{ mode }, Order, InvertByDefault);
		}
	}
	// sort options
	else
	{
		const auto Switch = [&](bool CurrentState)
		{
			return PlusPressed || (InvertPressed && !CurrentState);
		};

		switch (SortCode - std::size(SortModes) - extra - 1) // -1 for separator
		{
		case SortOptUseGroups:
			if (m_SortGroups != Switch(m_SortGroups))
				ProcessKey(Manager::Key(KEY_SHIFTF11));
			break;

		case SortOptSelectedFirst:
			if (SelectedFirst != Switch(SelectedFirst))
				ProcessKey(Manager::Key(KEY_SHIFTF12));
			break;

		case SortOptDirectoriesFirst:
			ChangeDirectoriesFirst(Switch(m_DirectoriesFirst));
			break;
		}
	}
}


void FileList::DeleteDiz(const string& Name, const string& ShortName)
{
	if (m_PanelMode == panel_mode::NORMAL_PANEL)
		Diz.Erase(Name,ShortName);
}


void FileList::FlushDiz()
{
	if (m_PanelMode == panel_mode::NORMAL_PANEL)
		Diz.Flush(m_CurDir);
}


string FileList::GetDizName() const
{
	return m_PanelMode == panel_mode::NORMAL_PANEL? Diz.GetDizName() : string();
}

string_view FileList::GetDescription(const string& Name, const string& ShortName, long long const FileSize) const
{
	return Diz.Get(Name, ShortName, FileSize);
}

void FileList::CopyDiz(const string& Name, const string& ShortName,const string& DestName,
                       const string& DestShortName,DizList *DestDiz)
{
	Diz.CopyDiz(Name, ShortName, DestName, DestShortName, DestDiz);
}


void FileList::DescribeFiles()
{
	int DizCount=0;
	ReadDiz();
	SaveSelection();
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);
	const auto AnotherType = AnotherPanel->GetType();
	for (const auto& i: enum_selected())
	{
		const auto PrevText = Diz.Get(i.FileName, i.AlternateFileName(), i.FileSize);
		const auto strMsg = concat(msg(lng::MEnterDescription), L' ', quote_space(i.FileName), L':');

		/* $ 09.08.2000 SVS
		   Для Ctrl-Z не нужно брать предыдущее значение!
		*/
		string strDizText;

		if (!GetString(
			msg(lng::MDescribeFiles),
			strMsg,
			L"DizText"sv,
			PrevText,
			strDizText,
			L"FileDiz"sv,
			FIB_ENABLEEMPTY | (!DizCount? FIB_NOUSELASTHISTORY : 0) | FIB_BUTTONS,
			{},
			{},
			{},
			&DescribeFileId))
		{
			break;
		}
		DizCount++;

		if (strDizText.empty())
		{
			Diz.Erase(i.FileName, i.AlternateFileName());
		}
		else
		{
			Diz.Set(i.FileName, i.AlternateFileName(), strDizText);
		}

		ClearLastGetSelection();
		// BugZ#442 - Deselection is late when making file descriptions
		FlushDiz();

		// BugZ#863 - При редактировании группы дескрипшенов они не обновляются на ходу
		//if (AnotherType==QVIEW_PANEL) continue; //TODO ???
		if (AnotherType == panel_type::INFO_PANEL) AnotherPanel->Update(0);

		Update(UPDATE_KEEP_SELECTION);
		Redraw();
	}

	/*if (DizCount>0)
	{
	  FlushDiz();
	  Update(UPDATE_KEEP_SELECTION);
	  Redraw();
	  const auto AnotherPanel = Parent()->GetAnotherPanel(this);
	  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	  AnotherPanel->Redraw();
	}*/
}


void FileList::SetReturnCurrentFile(bool Mode)
{
	ReturnCurrentFile=Mode;
}


bool FileList::ApplyCommand()
{
	static string strPrevCommand;
	string strCommand;

	if (!GetString(
		msg(lng::MAskApplyCommandTitle),
		msg(lng::MAskApplyCommand),
		L"ApplyCmd"sv,
		strPrevCommand,
		strCommand,
		L"ApplyCmd"sv,
		FIB_BUTTONS | FIB_EDITPATH | FIB_EDITPATHEXEC,
		{},
		{},
		{},
		&ApplyCommandId
	) || !SetCurPath())
		return false;

	strPrevCommand = strCommand;
	inplace::trim_left(strCommand);

	SaveSelection();

	++UpdateDisabled;
	Parent()->GetCmdLine()->LockUpdatePanel(true);
	{
		const auto ExecutionContext = Global->WindowManager->Desktop()->ConsoleSession().GetContext();
		for (const auto& i: enum_selected())
		{
			if (CheckForEscAndConfirmAbort())
				break;

			string strConvertedCommand = strCommand;
			bool PreserveLFN = false;

			if (SubstFileName(strConvertedCommand, { i.FileName, i.AlternateFileName() }, &PreserveLFN) && !strConvertedCommand.empty())
			{
				SCOPED_ACTION(PreserveLongName)(i.FileName, PreserveLFN);

				execute_info Info;
				Info.DisplayCommand = strConvertedCommand;
				Info.Command = strConvertedCommand;

				Parent()->GetCmdLine()->ExecString(Info);
			}

			ClearLastGetSelection();
		}
	}
	Parent()->GetCmdLine()->LockUpdatePanel(false);
	Parent()->GetCmdLine()->Show();
	if (Global->Opt->ShowKeyBar)
	{
		Parent()->GetKeybar().Show();
	}
	if (GetSelPosition >= static_cast<int>(m_ListData.size()))
		ClearSelection();

	--UpdateDisabled;
	return true;
}


void FileList::CountDirSize(bool IsRealNames)
{
	unsigned long SelDirCount=0;
	DirInfoData Data = {};

	//Рефреш текущему времени для фильтра перед началом операции
	m_Filter->UpdateCurrentTime();

	struct
	{
		unsigned long long Items;
		unsigned long long Size;
	}
	Total{};

	time_check TimeCheck;
	std::optional<dirinfo_progress> DirinfoProgress;

	const auto DirInfoCallback = [&](string_view const Name, unsigned long long const ItemsCount, unsigned long long const Size)
	{
		if (!TimeCheck)
			return;

		if (!DirinfoProgress)
			DirinfoProgress.emplace(msg(lng::MDirInfoViewTitle));

		DirinfoProgress->set_name(Name);
		DirinfoProgress->set_count(Total.Items + ItemsCount);
		DirinfoProgress->set_size(Total.Size + Size);
	};

	for (auto& i: m_ListData)
	{
		if (i.Selected && (i.Attributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			SelDirCount++;
			if ((!IsRealNames && !IsParentDirectory(i) && GetPluginDirInfo(GetPluginHandle(), i.FileName, &i.UserData, Data, DirInfoCallback)) ||
			     (IsRealNames && GetDirInfo(i.FileName, Data, m_Filter.get(), DirInfoCallback, GETDIRINFO_SCANSYMLINKDEF) == 1))
			{
				SelFileSize -= i.FileSize;
				SelFileSize += Data.FileSize;

				i.FileSize = Data.FileSize;
				i.AllocationSize = Data.AllocationSize;
				i.ShowFolderSize = 1;

				Total.Items += Data.DirCount + Data.FileCount;
				Total.Size += Data.FileSize;
			}
			else
				break;
		}
	}

	const auto GetPluginDirInfoOrParent = [this, &Total](const plugin_panel* const ph, string_view const DirName, const UserDataItem* const UserData, BasicDirInfoData& BasicData, const dirinfo_callback& Callback)
	{
		if (!m_CurFile && IsParentDirectory(m_ListData[0]))
		{
			const auto PluginCurDir = (m_CachedOpenPanelInfo.CurDir && *m_CachedOpenPanelInfo.CurDir)? PointToName(m_CachedOpenPanelInfo.CurDir) : L"\\"sv;
			const auto ParentDirInfoCallback = [&](string_view const, unsigned long long const ItemsCount, unsigned long long const Size)
			{
				return Callback(PluginCurDir, ItemsCount, Size);
			};

			for (const auto& i: range(m_ListData.begin() + 1, m_ListData.end()))
			{
				if (i.Attributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					++BasicData.DirCount;

					if (!IsParentDirectory(i))
					{
						BasicDirInfoData SubData{};
						if (!GetPluginDirInfo(GetPluginHandle(), i.FileName, &i.UserData, SubData, ParentDirInfoCallback))
							return false;

						BasicData.FileCount += SubData.FileCount;
						BasicData.DirCount += SubData.DirCount;
						BasicData.FileSize += SubData.FileSize;
						BasicData.AllocationSize += SubData.AllocationSize;

						Total.Items += SubData.DirCount + SubData.FileCount;
						Total.Size += SubData.FileSize;
					}
				}
				else
				{
					++BasicData.FileCount;
					BasicData.FileSize += i.FileSize;
					BasicData.AllocationSize += i.AllocationSize;

					++Total.Items;
					Total.Size += i.FileSize;
				}
			}

			return true;
		}
		else
		{
			return GetPluginDirInfo(ph, DirName, UserData, BasicData, Callback);
		}
	};

	if (!SelDirCount)
	{
		assert(m_CurFile < static_cast<int>(m_ListData.size()));
		auto& CurFile = m_ListData[m_CurFile];
		if ((!IsRealNames && GetPluginDirInfoOrParent(GetPluginHandle(), CurFile.FileName, &CurFile.UserData, Data, DirInfoCallback)) ||
		     (IsRealNames && GetDirInfo(IsParentDirectory(CurFile)? L"."s : CurFile.FileName, Data, m_Filter.get(), DirInfoCallback, GETDIRINFO_SCANSYMLINKDEF) == 1))
		{
			CurFile.FileSize = Data.FileSize;
			CurFile.AllocationSize = Data.AllocationSize;
			CurFile.ShowFolderSize = 1;
		}
	}

	SortFileList(true);
	ShowFileList();
	Parent()->Redraw();

	InitFSWatcher(true);
}


int FileList::GetPrevViewMode() const
{
	return (m_PanelMode == panel_mode::PLUGIN_PANEL && !PluginsList.empty())?PluginsList.front()->m_PrevViewMode:m_ViewMode;
}


panel_sort FileList::GetPrevSortMode() const
{
	return (m_PanelMode == panel_mode::PLUGIN_PANEL && !PluginsList.empty())?PluginsList.front()->m_PrevSortMode:m_SortMode;
}


bool FileList::GetPrevSortOrder() const
{
	return (m_PanelMode == panel_mode::PLUGIN_PANEL && !PluginsList.empty())?PluginsList.front()->m_PrevSortOrder : m_ReverseSortOrder;
}

bool FileList::GetPrevDirectoriesFirst() const
{
	return (m_PanelMode == panel_mode::PLUGIN_PANEL && !PluginsList.empty())?PluginsList.front()->m_PrevDirectoriesFirst:m_DirectoriesFirst;
}

plugin_panel* FileList::OpenFilePlugin(const string& FileName, int PushPrev, OPENFILEPLUGINTYPE Type, bool* StopProcessing)
{
	if (!PushPrev && m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		for (;;)
		{
			if (ProcessPluginEvent(FE_CLOSE,nullptr))
			{
				if (StopProcessing)
					*StopProcessing = true;
				return nullptr;
			}

			if (!PopPlugin(TRUE))
				break;
		}
	}

	auto hNewPlugin = OpenPluginForFile(FileName, 0, Type, StopProcessing);

	const auto hNewPluginRawCopy = hNewPlugin.get();

	if (hNewPlugin)
	{
		if (PushPrev)
		{
			PrevDataList.emplace_back(FileName, std::move(m_ListData), m_CurTopFile);
		}

		const auto WasFullscreen = IsFullScreen();
		SetPluginMode(std::move(hNewPlugin), FileName);  // SendOnFocus??? true???
		m_PanelMode = panel_mode::PLUGIN_PANEL;
		UpperFolderTopFile=m_CurTopFile;
		m_CurFile=0;
		Update(0);
		Redraw();
		const auto AnotherPanel = Parent()->GetAnotherPanel(this);

		if ((AnotherPanel->GetType() == panel_type::INFO_PANEL) || WasFullscreen)
			AnotherPanel->Redraw();
	}

	return hNewPluginRawCopy;
}


void FileList::ProcessCopyKeys(unsigned const Key)
{
	if (m_ListData.empty() || !SetCurPath())
		return;

	const auto Drag = any_of(Key, KEY_DRAGCOPY, KEY_DRAGMOVE);
	const auto Ask = !Drag || Global->Opt->Confirm.Drag;
	const auto Move = any_of(Key, KEY_F6, KEY_DRAGMOVE);
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);
	auto AnotherDir = false;

	if (const auto AnotherFilePanel = std::dynamic_pointer_cast<FileList>(AnotherPanel))
	{
		assert(AnotherFilePanel->m_ListData.empty() || AnotherFilePanel->m_CurFile < static_cast<int>(AnotherFilePanel->m_ListData.size()));
		if (!AnotherFilePanel->m_ListData.empty() &&
		        (AnotherFilePanel->m_ListData[AnotherFilePanel->m_CurFile].Attributes & FILE_ATTRIBUTE_DIRECTORY) &&
		        !IsParentDirectory(AnotherFilePanel->m_ListData[AnotherFilePanel->m_CurFile]))
		{
			AnotherDir = true;
		}
	}

	if (m_PanelMode == panel_mode::PLUGIN_PANEL && !PluginManager::UseInternalCommand(GetPluginHandle(), PLUGIN_FARGETFILES, m_CachedOpenPanelInfo))
	{
		if (none_of(Key, KEY_ALTF6, KEY_RALTF6))
		{
			string strPluginDestPath;
			int ToPlugin = 0;

			if (
				AnotherPanel->GetMode() == panel_mode::PLUGIN_PANEL &&
				AnotherPanel->IsVisible() &&
				!PluginManager::UseInternalCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES, m_CachedOpenPanelInfo))
			{
				ToPlugin=2;
				Copy(shared_from_this(), Move, false, false, Ask, ToPlugin, &strPluginDestPath);
			}

			if (ToPlugin!=-1)
			{
				if (ToPlugin)
					PluginToPluginFiles(Move);
				else
				{
					string strDestPath;

					if (!strPluginDestPath.empty())
						strDestPath = strPluginDestPath;
					else
					{
						strDestPath = AnotherPanel->GetCurDir();

						if (!AnotherPanel->IsVisible())
						{
							Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

							if (m_CachedOpenPanelInfo.HostFile && *m_CachedOpenPanelInfo.HostFile)
							{
								strDestPath = PointToName(m_CachedOpenPanelInfo.HostFile);
								const auto pos = strDestPath.rfind(L'.');
								if (pos != string::npos)
									strDestPath.resize(pos);
							}
						}
					}

					PluginGetFiles(strDestPath, Move);
				}
			}
		}
	}
	else
	{
		int ToPlugin =
			AnotherPanel->GetMode() == panel_mode::PLUGIN_PANEL &&
			AnotherPanel->IsVisible() && none_of(Key, KEY_ALTF6, KEY_RALTF6) &&
			!PluginManager::UseInternalCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES, m_CachedOpenPanelInfo);

		Copy(shared_from_this(), Move, any_of(Key, KEY_ALTF6, KEY_RALTF6), false, Ask, ToPlugin, nullptr, Drag && AnotherDir);

		if (ToPlugin==1)
			PluginPutFilesToAnother(Move,AnotherPanel);
	}
}

void FileList::SetSelectedFirstMode(bool Mode)
{
	SelectedFirst=Mode;
	SortFileList(true);
	ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
}

void FileList::ChangeSortOrder(bool Reverse)
{
	Panel::ChangeSortOrder(Reverse);
	SortFileList(true);
	ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
	Show();
}

void FileList::UpdateKeyBar()
{
	auto& Keybar = Parent()->GetKeybar();
	Keybar.SetLabels(lng::MF1);
	Keybar.SetCustomLabels(KBA_SHELL);

	if (GetMode() == panel_mode::PLUGIN_PANEL)
	{
		GetOpenPanelInfo(&m_CachedOpenPanelInfo);

		if (m_CachedOpenPanelInfo.KeyBar)
			Keybar.Change(m_CachedOpenPanelInfo.KeyBar);
	}
	}

bool FileList::PluginPanelHelp(const plugin_panel* hPlugin) const
{
	string_view strPath = hPlugin->plugin()->ModuleName();
	CutToSlash(strPath);
	const auto [File, Name, Codepage] = OpenLangFile(strPath, Global->HelpFileMask, Global->Opt->strHelpLanguage);
	if (!File)
		return false;

	help::show(help::make_link(strPath, L"Contents"sv));
	return true;
}

/* $ 19.11.2001 IS
     для файловых панелей с реальными файлами никакого префикса не добавляем
*/
string FileList::GetPluginPrefix() const
{
	if (!Global->Opt->SubstPluginPrefix || GetMode() != panel_mode::PLUGIN_PANEL)
		return {};

	Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

	if (!(m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES))
	{
		PluginInfo PInfo = {sizeof(PInfo)};
		if (Global->CtrlObject->Plugins->GetPluginInfo(GetPluginHandle()->plugin(), &PInfo) && PInfo.CommandPrefix && *PInfo.CommandPrefix)
		{
			const string_view Prefix = PInfo.CommandPrefix;
			return Prefix.substr(0, Prefix.find(L':')) + L":"sv;
		}
	}

	return {};
}


void FileList::GoHome(string_view const Drive)
{
	const auto FarRoot = extract_root_directory(Global->g_strFarModuleName);

	const auto go_home = [&](Panel& p)
	{
		if (p.GetMode() == panel_mode::PLUGIN_PANEL)
			return;

		if (starts_with_icase(p.GetCurDir(), Drive))
			p.SetCurDir(FarRoot, false);
	};

	// Passive first to prevent redraw issues in wide panel mode
	go_home(*Parent()->GetAnotherPanel(this));
	go_home(*this);
}

const FileListItem* FileList::GetItem(size_t Index) const
{
	if (static_cast<int>(Index) == -1 || static_cast<int>(Index) == -2)
		Index=GetCurrentPos();

	if (Index >= m_ListData.size())
		return nullptr;

	return &m_ListData[Index];
}

void FileList::ClearAllItem()
{
	PrevDataList.clear();
}

// flplugin
// Файловая панель - работа с плагинами

/*
   В стеке ФАРова панель не хранится - только плагиновые!
*/

void FileList::PushPlugin(std::unique_ptr<plugin_panel>&& hPlugin, string_view const HostFile)
{
	PluginsList.emplace_back(std::make_shared<PluginsListItem>(std::move(hPlugin), HostFile, false, m_ViewMode, m_SortMode, m_ReverseSortOrder, m_DirectoriesFirst, m_ViewSettings));
	++Global->PluginPanelsCount;
}

bool FileList::PopPlugin(int EnableRestoreViewMode)
{
	m_ListData.clear();

	if (PluginsList.empty())
	{
		m_PanelMode = panel_mode::NORMAL_PANEL;
		return false;
	}

	auto CurPlugin = std::move(PluginsList.back());
	PluginsList.pop_back();
	--Global->PluginPanelsCount;

	// We have removed current plugin panel from PluginsList already.
	// However, ClosePanel provides a notification and plugins might call API functions from it.
	// So GetPluginHandle() will look into m_ExpiringPluginPanel first.
	{
		m_ExpiringPluginPanel = CurPlugin;
		SCOPE_EXIT{ m_ExpiringPluginPanel = nullptr; };
		Global->CtrlObject->Plugins->ClosePanel(std::move(CurPlugin->m_Plugin));
	}

	if (!PluginsList.empty())
	{
		if (EnableRestoreViewMode)
		{
			SetViewMode(CurPlugin->m_PrevViewMode);
			m_SortMode = CurPlugin->m_PrevSortMode;
			m_ReverseSortOrder = CurPlugin->m_PrevSortOrder;
			m_DirectoriesFirst = CurPlugin->m_PrevDirectoriesFirst;
		}

		if (CurPlugin->m_Modified)
		{
			PluginPanelItemHolderHeap PanelItem={};
			const auto strSaveDir = os::fs::GetCurrentDirectory();

			if (FileNameToPluginItem(CurPlugin->m_HostFile, PanelItem))
			{
				Global->CtrlObject->Plugins->PutFiles(GetPluginHandle(), { &PanelItem.Item, 1 }, false, 0);
			}
			else
			{
				PluginPanelItem Item{};
				Item.FileName = PointToName(CurPlugin->m_HostFile).data();
				Global->CtrlObject->Plugins->DeleteFiles(GetPluginHandle(), { &Item, 1 }, 0);
			}

			FarChDir(strSaveDir);
		}


		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

		if (!(m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES))
		{
			DeleteFileWithFolder(CurPlugin->m_HostFile);  // удаление файла от предыдущего плагина
		}
	}
	else
	{
		m_PanelMode = panel_mode::NORMAL_PANEL;

		if (EnableRestoreViewMode)
		{
			SetViewMode(CurPlugin->m_PrevViewMode);
			m_SortMode = CurPlugin->m_PrevSortMode;
			m_ReverseSortOrder = CurPlugin->m_PrevSortOrder;
			m_DirectoriesFirst = CurPlugin->m_PrevDirectoriesFirst;
		}
	}

	if (EnableRestoreViewMode)
		Parent()->RedrawKeyBar();

	return true;
}

/*
	DefaultName - имя элемента на которое позиционируемся.
	Closed - панель закрывается, если в PrevDataList что-то есть - восстанавливаемся оттуда.
	UsePrev - если востанавливаемся из PrevDataList, элемент для позиционирования брать оттуда же.
	Position - надо ли вообще устанавливать текущий элемент.
*/
void FileList::PopPrevData(string_view const DefaultName, bool const Closed, bool const UsePrev, bool const Position, bool const SetDirectorySuccess)
{
	string strName(DefaultName);
	if (Closed && !PrevDataList.empty())
	{
		PrevDataItem& Item = PrevDataList.back();
		if (Item.PrevListData.size() > 1)
		{
			MoveSelection(Item.PrevListData, m_ListData);
			UpperFolderTopFile = Item.PrevTopFile;

			if (UsePrev)
				strName = Item.strPrevName;

			Item.PrevListData.clear();

			if (SelectedFirst)
				SortFileList(false);
			else if (!m_ListData.empty())
				SortFileList(true);
		}
		PrevDataList.pop_back();
	}
	if (Position)
	{
		const auto Pos = FindFile(PointToName(strName));

		if (Pos!=-1)
			m_CurFile=Pos;
		else
			GoToFile(strName);

		m_CurTopFile=UpperFolderTopFile;
		UpperFolderTopFile=0;
		CorrectPosition();
	}
	else if (SetDirectorySuccess)
		m_CurFile = m_CurTopFile = 0;
}

bool FileList::FileNameToPluginItem(string_view const Name, PluginPanelItemHolder& pi)
{
	string_view TempDir = Name;

	if (!CutToSlash(TempDir,true))
		return false;

	FarChDir(TempDir);

	os::fs::find_data fdata;
	if (os::fs::get_find_data(Name, fdata))
	{
		FindDataExToPluginPanelItemHolder(fdata, pi);
		return true;
	}

	return false;
}

static void FileListItemToPluginPanelItemBasic(const FileListItem& From, PluginPanelItem& To)
{
	To.CreationTime = os::chrono::nt_clock::to_filetime(From.CreationTime);
	To.LastAccessTime = os::chrono::nt_clock::to_filetime(From.LastAccessTime);
	To.LastWriteTime = os::chrono::nt_clock::to_filetime(From.LastWriteTime);
	To.ChangeTime = os::chrono::nt_clock::to_filetime(From.ChangeTime);
	To.FileSize = From.FileSize;
	To.AllocationSize = From.AllocationSize;
	To.FileName = {};
	To.AlternateFileName = {};
	To.Description = {};
	To.Owner = {};
	To.CustomColumnData = {};
	To.CustomColumnNumber = From.CustomColumns.size();
	To.Flags = From.UserFlags | (From.Selected ? PPIF_SELECTED : 0);
	To.UserData = From.UserData;
	To.FileAttributes = From.Attributes;
	To.NumberOfLinks = {};
	To.CRC32 = From.CRC32;
	std::fill(ALL_RANGE(To.Reserved), 0);
}

void FileList::FileListToPluginItem(const FileListItem& fi, PluginPanelItemHolder& Holder) const
{
	auto& pi = Holder.Item;

	FileListItemToPluginPanelItemBasic(fi, pi);
	pi.NumberOfLinks = fi.IsNumberOfLinksRead() ? fi.NumberOfLinks(this) : 0;

	Holder.set_name(fi.FileName);
	Holder.set_alt_name(fi.AlternateFileName());
	Holder.set_columns(fi.CustomColumns);

	if (fi.DizText)
		Holder.set_description(fi.DizText);

	if (fi.IsOwnerRead())
	{
		if (const auto& Owner = fi.Owner(this); !Owner.empty())
			Holder.set_owner(Owner);
	}
}

size_t FileList::FileListToPluginItem2(const FileListItem& fi,FarGetPluginPanelItem* gpi) const
{
	const auto StringSizeInBytes = [](string_view const Str)
	{
		return (Str.size() + 1) * sizeof(wchar_t);
	};

	const auto
		StaticSize      = aligned_sizeof<PluginPanelItem, sizeof(wchar_t)>(),
		FilenameSize    = aligned_size(StringSizeInBytes(fi.FileName), alignof(wchar_t)),
		AltNameSize     = aligned_size(StringSizeInBytes(fi.AlternateFileName()), alignof(wchar_t*)),
		ColumnsSize     = aligned_size(fi.CustomColumns.size() * sizeof(wchar_t*), alignof(wchar_t)),
		ColumnsDataSize = aligned_size(std::accumulate(ALL_CONST_RANGE(fi.CustomColumns), size_t(0), [&](size_t s, const wchar_t* i) { return s + (i? StringSizeInBytes(i) : 0); }), alignof(wchar_t)),
		DescriptionSize = aligned_size(fi.DizText? StringSizeInBytes(fi.DizText) : 0, alignof(wchar_t)),
		OwnerSize       = aligned_size(fi.IsOwnerRead() && !fi.Owner(this).empty()? StringSizeInBytes(fi.Owner(this)) : 0, alignof(wchar_t));

	const auto size =
		StaticSize +
		FilenameSize +
		AltNameSize +
		ColumnsSize +
		ColumnsDataSize +
		DescriptionSize +
		OwnerSize;

	if (!gpi || !gpi->Item || gpi->Size < sizeof(*gpi->Item))
		return size;

	FileListItemToPluginPanelItemBasic(fi, *gpi->Item);
	gpi->Item->NumberOfLinks = fi.IsNumberOfLinksRead()? fi.NumberOfLinks(this) : 0;

	auto data = reinterpret_cast<char*>(gpi->Item);
	const auto end = data + gpi->Size;

	data += StaticSize;

	const auto CopyToBuffer = [&](string_view const Str)
	{
		const auto Result = reinterpret_cast<const wchar_t*>(data);
		*copy_string(Str, reinterpret_cast<wchar_t*>(data)) = {};
		return Result;
	};

	const auto not_enough_for = [&](size_t const Size)
	{
		return data == end || data + Size > end;
	};

	if (not_enough_for(FilenameSize))
		return size;
	gpi->Item->FileName = CopyToBuffer(fi.FileName);
	data += FilenameSize;

	if (not_enough_for(AltNameSize))
		return size;
	gpi->Item->AlternateFileName = CopyToBuffer(fi.AlternateFileName());
	data += AltNameSize;


	if (ColumnsSize)
	{
		if (not_enough_for(ColumnsSize))
			return size;
		gpi->Item->CustomColumnData = reinterpret_cast<const wchar_t* const*>(data);
		data += ColumnsSize;
	}

	if (ColumnsDataSize)
	{
		const auto dataBegin = data;

		if (not_enough_for(ColumnsDataSize))
		{
			gpi->Item->CustomColumnData = {};
			return size;
		}

		for (const auto& [Column, Data]: zip(fi.CustomColumns, span(const_cast<const wchar_t**>(gpi->Item->CustomColumnData), fi.CustomColumns.size())))
		{
			if (!Column)
			{
				Data = {};
				continue;
			}

			Data = CopyToBuffer(Column);
			data += StringSizeInBytes(Column);
		}

		data = dataBegin + ColumnsDataSize;
	}

	if (DescriptionSize)
	{
		if (not_enough_for(DescriptionSize))
			return size;
		gpi->Item->Description = CopyToBuffer(fi.DizText);
		data += DescriptionSize;
	}

	if (OwnerSize)
	{
		if (not_enough_for(OwnerSize))
			return size;
		gpi->Item->Owner = CopyToBuffer(fi.Owner(this));
		data += OwnerSize;
	}

	return size;
}

FileListItem::FileListItem(const PluginPanelItem& pi)
{
	CreationTime = os::chrono::nt_clock::from_filetime(pi.CreationTime);
	LastAccessTime = os::chrono::nt_clock::from_filetime(pi.LastAccessTime);
	LastWriteTime = os::chrono::nt_clock::from_filetime(pi.LastWriteTime);
	ChangeTime = os::chrono::nt_clock::from_filetime(pi.ChangeTime);

	FileSize = pi.FileSize;
	AllocationSize = pi.AllocationSize;

	UserFlags = pi.Flags;
	UserData = pi.UserData;

	Attributes = pi.FileAttributes;
	// we don't really know, but it's better than show it as 'unknown'
	ReparseTag = (Attributes & FILE_ATTRIBUTE_REPARSE_POINT)? IO_REPARSE_TAG_SYMLINK : 0;

	if (pi.CustomColumnData && pi.CustomColumnNumber)
	{
		CustomColumns.reserve(pi.CustomColumnNumber);

		for (const auto& i: span(pi.CustomColumnData, pi.CustomColumnNumber))
		{
			if (!i)
			{
				CustomColumns.emplace_back();
				continue;
			}

			string_view const Data = i;
			auto Str = std::make_unique<wchar_t[]>(Data.size() + 1);
			*copy_string(Data, Str.get()) = {};
			CustomColumns.emplace_back(Str.release());
		}
	}

	SortGroup = DEFAULT_SORT_GROUP;
	CRC32 = pi.CRC32;

	if (pi.Description)
	{
		string_view const Description = pi.Description;
		auto Str = std::make_unique<wchar_t[]>(Description.size() + 1);
		*copy_string(Description, Str.get()) = {};
		DizText = Str.release();
		DeleteDiz = true;
	}

	FileName = NullToEmpty(pi.FileName);
	SetAlternateFileName(NullToEmpty(pi.AlternateFileName));
	m_Owner = NullToEmpty(pi.Owner);

	m_NumberOfLinks = pi.NumberOfLinks;
	m_NumberOfStreams = 1;
	m_StreamsSize = FileSize;
}


std::unique_ptr<plugin_panel> FileList::OpenPluginForFile(const string& FileName, os::fs::attributes const FileAttr, OPENFILEPLUGINTYPE const Type, bool* const StopProcessing)
{
	if (FileName.empty() || FileAttr & FILE_ATTRIBUTE_DIRECTORY)
		return nullptr;

	SetCurPath();
	Parent()->GetAnotherPanel(this)->CloseFile();
	return Global->CtrlObject->Plugins->OpenFilePlugin(&FileName, OPM_NONE, Type, StopProcessing);
}

plugin_item_list FileList::CreatePluginItemList()
{
	plugin_item_list ItemList;

	if (m_ListData.empty())
		return ItemList;

	const auto SaveSelPosition = GetSelPosition;
	const auto OldLastSelPosition = LastSelPosition;

	ItemList.reserve(m_SelFileCount+1);

	const auto ConvertAndAddToList = [&](const FileListItem& What)
	{
		PluginPanelItemHolderHeapNonOwning NewItem;
		FileListToPluginItem(What, NewItem);
		ItemList.emplace_back(NewItem.Item);
	};

	for (const auto& i: enum_selected())
	{
		if (!IsParentDirectory(i) && LastSelPosition >= 0 && static_cast<size_t>(LastSelPosition) < m_ListData.size())
			ConvertAndAddToList(m_ListData[LastSelPosition]);
	}

	if (ItemList.empty() && !m_ListData.empty() && IsParentDirectory(m_ListData.front()))
		ConvertAndAddToList(m_ListData.front());

	LastSelPosition=OldLastSelPosition;
	GetSelPosition=SaveSelPosition;
	return ItemList;
}

void FileList::PluginDelete()
{
	SaveSelection();

	{
		auto ItemList = CreatePluginItemList();
		if (ItemList.empty())
			return;

		const auto Item = GetPluginItem();
		if (Global->CtrlObject->Plugins->DeleteFiles(Item.lock()->m_Plugin.get(), ItemList, 0) && !Item.expired())
		{
			SetPluginModified();
			PutDizToPlugin(this, ItemList.items(), true, false, nullptr);
		}
	}

	Update(UPDATE_KEEP_SELECTION);
	Redraw();
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);
	AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	AnotherPanel->Redraw();
}


void FileList::PutDizToPlugin(FileList *DestPanel, const std::vector<PluginPanelItem>& ItemList, bool Delete, bool Move, DizList *SrcDiz) const
{
	Global->CtrlObject->Plugins->GetOpenPanelInfo(DestPanel->GetPluginHandle(), &m_CachedOpenPanelInfo);

	if (DestPanel->strPluginDizName.empty() && m_CachedOpenPanelInfo.DescrFilesNumber>0)
		DestPanel->strPluginDizName = m_CachedOpenPanelInfo.DescrFiles[0];

	if (Global->Opt->Diz.UpdateMode != DIZ_UPDATE_ALWAYS && !(Global->Opt->Diz.UpdateMode == DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed()))
		return;

	if (DestPanel->strPluginDizName.empty())
		return;

	if (m_CachedOpenPanelInfo.HostFile && *m_CachedOpenPanelInfo.HostFile && !DestPanel->GetModalMode() && !os::fs::exists(m_CachedOpenPanelInfo.HostFile))
		return;

	Parent()->LeftPanel()->ReadDiz();
	Parent()->RightPanel()->ReadDiz();

	if (DestPanel->GetModalMode())
		DestPanel->ReadDiz();

	bool DizPresent = false;

	for (const auto& i: ItemList)
	{
		if (!(i.Flags & PPIF_PROCESSDESCR))
			continue;

		if (Delete)
		{
			if (!DestPanel->Diz.Erase(i.FileName, i.AlternateFileName))
				continue;
		}
		else
		{
			if (!SrcDiz->CopyDiz(i.FileName, i.AlternateFileName, i.FileName, i.AlternateFileName, &DestPanel->Diz))
				continue;

			if (Move)
				SrcDiz->Erase(i.FileName, i.AlternateFileName);
		}

		DizPresent = true;
	}

	if (!DizPresent)
		return;

	const auto strTempDir = MakeTemp();
	if (!os::fs::create_directory(strTempDir))
		return;

	const auto strSaveDir = os::fs::GetCurrentDirectory();
	const auto strDizName = path::join(strTempDir, DestPanel->strPluginDizName);
	DestPanel->Diz.Flush({}, &strDizName);

	if (Move)
		SrcDiz->Flush({});

	PluginPanelItemHolderHeap PanelItem;
	if (FileNameToPluginItem(strDizName, PanelItem))
	{
		Global->CtrlObject->Plugins->PutFiles(DestPanel->GetPluginHandle(), { &PanelItem.Item, 1 }, false, OPM_SILENT | OPM_DESCR);
	}
	else if (Delete)
	{
		PluginPanelItem pi={};
		pi.FileName = DestPanel->strPluginDizName.c_str();
		Global->CtrlObject->Plugins->DeleteFiles(DestPanel->GetPluginHandle(), { &pi,1 }, OPM_SILENT);
	}

	FarChDir(strSaveDir);
	DeleteFileWithFolder(strDizName);
}


void FileList::PluginGetFiles(const string& DestPath, bool Move)
{
	SaveSelection();

	{
		auto ItemList = CreatePluginItemList();
		if (ItemList.empty())
			return;

		auto DestPathPtr = DestPath.c_str();
		const auto Item = GetPluginItem();
		const auto GetCode = Global->CtrlObject->Plugins->GetFiles(Item.lock()->m_Plugin.get(), ItemList, Move, &DestPathPtr, 0);

		if (!Item.expired())
		{
			if (GetCode == 1)
			{
				if ((Global->Opt->Diz.UpdateMode == DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed()) || Global->Opt->Diz.UpdateMode == DIZ_UPDATE_ALWAYS)
				{
					const auto NewDestPath = DestPathPtr;
					DizList DestDiz;
					bool DizFound = false;

					for (auto& i: ItemList.items())
					{
						if (i.Flags & PPIF_PROCESSDESCR)
						{
							if (!DizFound)
							{
								Parent()->LeftPanel()->ReadDiz();
								Parent()->RightPanel()->ReadDiz();
								DestDiz.Read(NewDestPath);
								DizFound = true;
							}
							CopyDiz(i.FileName, i.AlternateFileName, i.FileName, i.FileName, &DestDiz);
						}
					}

					DestDiz.Flush(NewDestPath);
				}

				if (!ReturnCurrentFile)
					ClearSelection();

				if (Move)
				{
					SetPluginModified();
					PutDizToPlugin(this, ItemList.items(), true, false, nullptr);
				}
			}
			else if (!ReturnCurrentFile)
				PluginClearSelection(ItemList.items());
		}
	}

	Update(UPDATE_KEEP_SELECTION);
	Redraw();
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);
	AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	AnotherPanel->Redraw();
}


void FileList::PluginToPluginFiles(bool Move)
{
	const auto AnotherFilePanel = std::dynamic_pointer_cast<FileList>(Parent()->GetAnotherPanel(this));
	if (!AnotherFilePanel || AnotherFilePanel->GetMode() != panel_mode::PLUGIN_PANEL)
		return;

	SaveSelection();
	auto strTempDir = MakeTemp();
	const auto OriginalTempDir = strTempDir;
	// BUGBUG check result
	if (!os::fs::create_directory(strTempDir))
	{
		LOGWARNING(L"create_directory({}): {}"sv, strTempDir, last_error());
	}

	{
		auto ItemList = CreatePluginItemList();
		if (ItemList.empty())
			return;

		auto TempDir=strTempDir.c_str();
		int PutCode = Global->CtrlObject->Plugins->GetFiles(GetPluginHandle(), ItemList, false, &TempDir, OPM_SILENT);
		strTempDir=TempDir;

		if (PutCode==1 || PutCode==2)
		{
			const auto strSaveDir = os::fs::GetCurrentDirectory();
			FarChDir(strTempDir);
			PutCode = Global->CtrlObject->Plugins->PutFiles(AnotherFilePanel->GetPluginHandle(), ItemList, false, 0);

			if (PutCode==1 || PutCode==2)
			{
				if (!ReturnCurrentFile)
					ClearSelection();

				AnotherFilePanel->SetPluginModified();
				PutDizToPlugin(AnotherFilePanel.get(), ItemList.items(), false, false, &Diz);

				if (Move && Global->CtrlObject->Plugins->DeleteFiles(GetPluginHandle(), ItemList, OPM_SILENT))
				{
					SetPluginModified();
					PutDizToPlugin(this, ItemList.items(), true, false, nullptr);
				}
			}
			else if (!ReturnCurrentFile)
				PluginClearSelection(ItemList.items());

			FarChDir(strSaveDir);
		}

		DeleteDirTree(strTempDir);
		// BUGBUG check result
		if (!os::fs::remove_directory(OriginalTempDir))
		{
			LOGWARNING(L"remove_directory({}): {}"sv, OriginalTempDir, last_error());
		}
	}

	Update(UPDATE_KEEP_SELECTION);
	Redraw();

	AnotherFilePanel->Update(UPDATE_KEEP_SELECTION | (m_PanelMode == panel_mode::PLUGIN_PANEL? UPDATE_SECONDARY : 0));
	AnotherFilePanel->Redraw();
}

void FileList::PluginHostGetFiles()
{
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);
	SaveSelection();

	const auto Enumerator = enum_selected();
	const auto it = Enumerator.begin();
	if (it == Enumerator.end())
		return;

	const auto& Data = *it;

	auto strDestPath = AnotherPanel->GetCurDir();

	if (((!AnotherPanel->IsVisible() || AnotherPanel->GetType() != panel_type::FILE_PANEL) && !m_SelFileCount) || strDestPath.empty())
	{
		strDestPath = PointToName(Data.FileName);
		// SVS: А зачем здесь велся поиск точки с начала?
		const auto pos = strDestPath.rfind(L'.');
		if (pos != string::npos)
			strDestPath.resize(pos);
	}

	auto ExitLoop = false;
	std::unordered_set<Plugin*> UsedPlugins;
	for (const auto& i: Enumerator)
	{
		if (ExitLoop)
			break;

		auto hCurPlugin = OpenPluginForFile(i.FileName, i.Attributes, OFP_EXTRACT);

		if (!hCurPlugin)
			continue;

		int OpMode=OPM_TOPLEVEL;
		if (contains(UsedPlugins, hCurPlugin->plugin()))
			OpMode|=OPM_SILENT;

		span<PluginPanelItem> Items;

		if (Global->CtrlObject->Plugins->GetFindData(hCurPlugin.get(), Items, OpMode))
		{
			auto DestPath = strDestPath.c_str();
			ExitLoop = Global->CtrlObject->Plugins->GetFiles(hCurPlugin.get(), Items, false, &DestPath, OpMode) != 1;
			strDestPath=DestPath;

			if (!ExitLoop)
			{
				ClearLastGetSelection();
			}

			Global->CtrlObject->Plugins->FreeFindData(hCurPlugin.get(), Items, true);
			UsedPlugins.emplace(hCurPlugin->plugin());
		}

		Global->CtrlObject->Plugins->ClosePanel(std::move(hCurPlugin));
	}

	Update(UPDATE_KEEP_SELECTION);
	Redraw();
	AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	AnotherPanel->Redraw();
}


void FileList::PluginPutFilesToNew()
{
	auto hNewPlugin = Global->CtrlObject->Plugins->OpenFilePlugin(nullptr, OPM_NONE, OFP_CREATE);
	if (!hNewPlugin)
		return;

	auto TmpPanel = create(nullptr);
	TmpPanel->SetPluginMode(std::move(hNewPlugin), {});  // SendOnFocus??? true???
	TmpPanel->m_ModalMode = TRUE;
	const auto PrevFileCount = m_ListData.size();
	/* $ 12.04.2002 IS
		Если PluginPutFilesToAnother вернула число, отличное от 2, то нужно
		попробовать установить курсор на созданный файл.
	*/
	const auto rc = PluginPutFilesToAnother(false, TmpPanel);

	if (rc == 2 || m_ListData.size() != PrevFileCount + 1)
		return;

	int LastPos = 0;
	/* Место, где вычисляются координаты вновь созданного файла
		Позиционирование происходит на файл с максимальной датой
		создания файла. Посему, если какой-то злобный буратино поимел
		в текущем каталоге файло с датой создания поболее текущей,
		то корректного позиционирования не произойдет!
	*/
	const FileListItem *PtrLastPos = nullptr;
	int n = 0;

	for (const auto& i: m_ListData)
	{
		if (!(i.Attributes & FILE_ATTRIBUTE_DIRECTORY) && (!PtrLastPos || PtrLastPos->CreationTime < i.CreationTime))
		{
			LastPos = n;
			PtrLastPos = &i;
		}
		++n;
	}

	if (PtrLastPos)
	{
		m_CurFile = LastPos;
		Redraw();
	}
}


/* $ 12.04.2002 IS
     PluginPutFilesToAnother теперь int - возвращает то, что возвращает
     PutFiles:
     -1 - прервано пользователем
      0 - неудача
      1 - удача
      2 - удача, курсор принудительно установлен на файл и заново его
          устанавливать не нужно (см. PluginPutFilesToNew)
*/
int FileList::PluginPutFilesToAnother(bool Move, panel_ptr AnotherPanel)
{
	const auto AnotherFilePanel = std::dynamic_pointer_cast<FileList>(AnotherPanel);
	if (!AnotherFilePanel || AnotherFilePanel->GetMode() != panel_mode::PLUGIN_PANEL)
		return 0;

	SaveSelection();

	int PutCode = 0;

	{
		auto ItemList = CreatePluginItemList();
		if (ItemList.empty())
			return 0;

		SetCurPath();
		PutCode = Global->CtrlObject->Plugins->PutFiles(AnotherFilePanel->GetPluginHandle(), ItemList, Move, 0);

		if (PutCode==1 || PutCode==2)
		{
			if (!ReturnCurrentFile)
			{
				ClearSelection();
			}

			PutDizToPlugin(AnotherFilePanel.get(), ItemList.items(), false, Move, &Diz);
			AnotherFilePanel->SetPluginModified();
		}
		else if (!ReturnCurrentFile)
			PluginClearSelection(ItemList.items());
	}

	Update(UPDATE_KEEP_SELECTION);
	Redraw();

	if (AnotherFilePanel == Parent()->GetAnotherPanel(this))
	{
		AnotherFilePanel->Update(UPDATE_KEEP_SELECTION);
		AnotherFilePanel->Redraw();
	}

	return PutCode;
}


void FileList::GetOpenPanelInfo(OpenPanelInfo *Info) const
{
	*Info = {};

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), Info);
}


/*
   Функция для вызова команды "Архивные команды" (Shift-F3)
*/
void FileList::ProcessHostFile()
{
	if (m_ListData.empty() || !SetCurPath())
		return;

	int Done=FALSE;
	SaveSelection();

	if (m_PanelMode == panel_mode::PLUGIN_PANEL && !PluginsList.back()->m_HostFile.empty())
	{
		{
			auto ItemList = CreatePluginItemList();
			Done = Global->CtrlObject->Plugins->ProcessHostFile(GetPluginHandle(), ItemList, 0);

			if (Done)
				SetPluginModified();
			else
			{
				if (!ReturnCurrentFile)
					PluginClearSelection(ItemList.items());

				Redraw();
			}
		}

		if (Done)
			ClearSelection();
	}
	else
	{
		const auto SCount = GetRealSelCount();

		if (SCount > 0)
		{
			for (auto& i: m_ListData)
			{
				if (i.Selected)
				{
					Done=ProcessOneHostFile(&i);

					if (Done == 1)
						Select(i, false);
					else if (Done == -1)
						continue;
					else       // Если ЭТО убрать, то... будем жать ESC до потери пульса
						break;   //
				}
			}

			if (SelectedFirst)
				SortFileList(true);
		}
		else
		{
			if ((Done=ProcessOneHostFile(&*(m_ListData.begin() + m_CurFile))) == 1)
				ClearSelection();
		}
	}

	if (Done)
	{
		Update(UPDATE_KEEP_SELECTION);
		Redraw();
		const auto AnotherPanel = Parent()->GetAnotherPanel(this);
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		AnotherPanel->Redraw();
	}
}

/*
  Обработка одного хост-файла.
  Return:
    -1 - Этот файл никаким плагином не поддержан
     0 - Плагин вернул FALSE
     1 - Плагин вернул TRUE
*/
int FileList::ProcessOneHostFile(const FileListItem* Item)
{
	int Done=-1;

	auto hNewPlugin = OpenPluginForFile(Item->FileName, Item->Attributes, OFP_COMMANDS);
	if (!hNewPlugin)
		return Done;

	span<PluginPanelItem> Items;

	if (Global->CtrlObject->Plugins->GetFindData(hNewPlugin.get(), Items, OPM_TOPLEVEL))
	{
		Done = Global->CtrlObject->Plugins->ProcessHostFile(hNewPlugin.get(), Items, OPM_TOPLEVEL);
		Global->CtrlObject->Plugins->FreeFindData(hNewPlugin.get(), Items, true);
	}

	Global->CtrlObject->Plugins->ClosePanel(std::move(hNewPlugin));

	return Done;
}

void FileList::SetPluginMode(std::unique_ptr<plugin_panel>&& PluginPanel, string_view const PluginFile, bool SendOnFocus)
{
	const auto ParentWindow = Parent();

	PushPlugin(std::move(PluginPanel), PluginFile);

	m_PanelMode = panel_mode::PLUGIN_PANEL;

	if (SendOnFocus)
		OnFocusChange(true);

	Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

	if (m_CachedOpenPanelInfo.StartPanelMode)
		SetViewMode(VIEW_0 + m_CachedOpenPanelInfo.StartPanelMode-L'0');

	if (m_CachedOpenPanelInfo.StartSortMode)
	{
		m_SortMode = plugin_sort_mode_to_internal(m_CachedOpenPanelInfo.StartSortMode);
		m_ReverseSortOrder = m_CachedOpenPanelInfo.StartSortOrder != 0;
	}

	if (ParentWindow)
	{
		// BUGBUG, redraw logic shouldn't be here

		ParentWindow->RedrawKeyBar();

		const auto AnotherPanel = ParentWindow->GetAnotherPanel(this);

		if (AnotherPanel->GetType() != panel_type::FILE_PANEL)
		{
			AnotherPanel->Update(UPDATE_KEEP_SELECTION);
			AnotherPanel->Redraw();
		}
	}
}

void FileList::PluginGetPanelInfo(PanelInfo &Info)
{
	CorrectPosition();
	Info.CurrentItem=m_CurFile;
	Info.TopPanelItem=m_CurTopFile;
	if(m_ShowShortNames)
		Info.Flags|=PFLAGS_ALTERNATIVENAMES;
	Info.ItemsNumber = m_ListData.size();
	Info.SelectedItemsNumber=m_ListData.empty()? 0 : GetSelCount();
}

size_t FileList::PluginGetPanelItem(int ItemNumber,FarGetPluginPanelItem *Item)
{
	size_t result=0;

	if (static_cast<size_t>(ItemNumber) < m_ListData.size())
	{
		result=FileListToPluginItem2(m_ListData[ItemNumber], Item);
	}

	return result;
}

size_t FileList::PluginGetSelectedPanelItem(int ItemNumber,FarGetPluginPanelItem *Item)
{
	if (static_cast<size_t>(ItemNumber) >= m_ListData.size())
		return 0;

	if (ItemNumber == CacheSelIndex)
		return FileListToPluginItem2(m_ListData[CacheSelPos], Item);

	if (ItemNumber < CacheSelIndex)
		CacheSelIndex = -1;

	int CurSel = CacheSelIndex;
	const size_t StartValue = CacheSelIndex >= 0 ? CacheSelPos + 1 : 0;

	size_t result = 0;

	for (const auto& i: irange(StartValue, m_ListData.size()))
	{
		if (m_ListData[i].Selected)
			CurSel++;

		if (CurSel == ItemNumber)
		{
			result = FileListToPluginItem2(m_ListData[i], Item);
			CacheSelIndex = ItemNumber;
			CacheSelPos = static_cast<int>(i);
			break;
		}
	}

	if (CurSel == -1 && !ItemNumber)
	{
		result = FileListToPluginItem2(m_ListData[m_CurFile], Item);
		CacheSelIndex=-1;
	}

	return result;
}

void FileList::PluginGetColumnTypesAndWidths(string& strColumnTypes,string& strColumnWidths) const
{
	std::tie(strColumnTypes, strColumnWidths) = SerialiseViewSettings(m_ViewSettings.PanelColumns);
}

void FileList::PluginBeginSelection()
{
	SaveSelection();
}

void FileList::PluginSetSelection(int ItemNumber,bool Selection)
{
	Select(m_ListData[ItemNumber], Selection);
}

void FileList::PluginClearSelection(int SelectedItemNumber)
{
	if (static_cast<size_t>(SelectedItemNumber) >= m_ListData.size())
		return;

	if (SelectedItemNumber<=CacheSelClearIndex)
		CacheSelClearIndex=-1;

	int CurSel = CacheSelClearIndex;
	const size_t StartValue = CacheSelClearIndex >= 0? CacheSelClearPos + 1 : 0;

	for (const auto& i: irange(StartValue, m_ListData.size()))
	{
		if (m_ListData[i].Selected)
		{
			CurSel++;
		}

		if (CurSel==SelectedItemNumber)
		{
			Select(m_ListData[i], false);
			CacheSelClearIndex=SelectedItemNumber;
			CacheSelClearPos=static_cast<int>(i);
			break;
		}
	}
}

void FileList::PluginEndSelection()
{
	if (SelectedFirst)
	{
		SortFileList(true);
	}
}

void FileList::SetPluginModified()
{
	if (PluginsList.empty())
		return;

	PluginsList.back()->m_Modified = true;
}

plugin_panel* FileList::GetPluginHandle() const
{
	return m_ExpiringPluginPanel? m_ExpiringPluginPanel->m_Plugin.get() : !PluginsList.empty()? PluginsList.back()->m_Plugin.get() : nullptr;
}

std::weak_ptr<FileList::PluginsListItem> FileList::GetPluginItem() const
{
	assert(!PluginsList.empty());
	return m_ExpiringPluginPanel? m_ExpiringPluginPanel : PluginsList.back();
}

bool FileList::ProcessPluginEvent(int Event,void *Param)
{
	if (m_PanelMode != panel_mode::PLUGIN_PANEL)
		return false;

	return Global->CtrlObject->Plugins->ProcessEvent(GetPluginHandle(), Event, Param) != FALSE;
}

void FileList::PluginClearSelection(const std::vector<PluginPanelItem>& ItemList)
{
	SaveSelection();
	size_t FileNumber=0,PluginNumber=0;

	while (PluginNumber < ItemList.size())
	{
		const auto& CurPlugin = ItemList[PluginNumber];

		if (!(CurPlugin.Flags & PPIF_SELECTED))
		{
			while (!equal_icase(CurPlugin.FileName, m_ListData[FileNumber].FileName))
				if (++FileNumber >= m_ListData.size())
					return;

			Select(m_ListData[FileNumber++], false);
		}

		PluginNumber++;
	}
}

// flupdate
// Файловая панель - чтение имен файлов

// Флаги для ReadDiz()
enum ReadDizFlags
{
	RDF_NO_UPDATE         = 0_bit,
};

void FileList::Update(int Mode)
{
	if (m_EnableUpdate)
	{
		const auto
			IsKeepSelection = (Mode & UPDATE_KEEP_SELECTION) != 0,
			IsIgnoreVisible = (Mode & UPDATE_IGNORE_VISIBLE) != 0;

		switch (m_PanelMode)
		{
		case panel_mode::NORMAL_PANEL:
			ReadFileNames(IsKeepSelection, IsIgnoreVisible);
			break;

		case panel_mode::PLUGIN_PANEL:
			Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

			if (m_PanelMode != panel_mode::PLUGIN_PANEL)
				ReadFileNames(IsKeepSelection, IsIgnoreVisible);
			else if ((m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES) || Parent()->GetAnotherPanel(this)->GetMode() == panel_mode::PLUGIN_PANEL || !(Mode & UPDATE_SECONDARY))
				UpdatePlugin(IsKeepSelection, IsIgnoreVisible);
			break;
		}
	}
}

void FileList::UpdateIfRequired()
{
	if (!UpdateRequired || UpdateDisabled)
		return;

	UpdateRequired = false;
	Update((m_KeepSelection? UPDATE_KEEP_SELECTION : 0) | UPDATE_IGNORE_VISIBLE);
}

void FileList::ReadFileNames(bool const KeepSelection, bool const UpdateEvenIfPanelInvisible)
{
	SCOPED_ACTION(taskbar::indeterminate)(false);

	strOriginalCurDir = m_CurDir;

	if (!IsVisible() && !UpdateEvenIfPanelInvisible)
	{
		UpdateRequired = true;
		m_KeepSelection = KeepSelection;
		return;
	}

	UpdateRequired = false;
	AccessTimeUpdateRequired = false;
	DizRead = false;
	decltype(m_ListData) OldData;
	string strCurName, strNextCurName;

	// really?
	if (!Parent()->IsLeft(this) && !Parent()->IsRight(this))
		return;

	const auto strSaveDir = os::fs::GetCurrentDirectory();
	{
		string strOldCurDir(m_CurDir);

		if (!SetCurPath())
		{
			FlushInputBuffer(); // Очистим буфер ввода, т.к. мы уже можем быть в другом месте...

			if (m_CurDir == strOldCurDir) //?? i??
			{
				strOldCurDir = GetPathRoot(strOldCurDir);

				if (!os::fs::IsDiskInDrive(strOldCurDir))
					GoHome(strOldCurDir);

				/* При смене каталога путь не изменился */
			}

			return;
		}
	}
	SortGroupsRead = false;

	if (IsFocused())
		Parent()->GetCmdLine()->SetCurDir(m_CurDir);

	LastCurFile=-1;
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);
	AnotherPanel->QViewDelTempName();
	size_t PrevSelFileCount=m_SelFileCount;
	m_SelFileCount=0;
	m_SelDirCount = 0;
	SelFileSize=0;
	m_TotalFileCount=0;
	m_TotalDirCount = 0;
	TotalFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;
	FreeDiskSize = -1;
	if (Global->Opt->ShowPanelFree)
	{
		// BUGBUG check result
		if (!os::fs::get_disk_size(m_CurDir, nullptr, nullptr, &FreeDiskSize))
		{
			LOGWARNING(L"get_disk_size({}): {}"sv, m_CurDir, last_error());
		}
	}

	if (!m_ListData.empty())
	{
		strCurName = m_ListData[m_CurFile].FileName;

		if (m_ListData[m_CurFile].Selected && !ReturnCurrentFile)
		{
			const auto NotSelectedIterator = std::find_if(m_ListData.begin() + m_CurFile + 1, m_ListData.end(), [](const auto& i) { return !i.Selected; });
			if (NotSelectedIterator != m_ListData.cend())
			{
				strNextCurName = NotSelectedIterator->FileName;
			}
		}
	}

	if (KeepSelection || PrevSelFileCount>0)
	{
		OldData = std::move(m_ListData);
	}

	m_ListData.initialise(nullptr);
	m_FilteredExtensions.clear();

	DWORD FileSystemFlags = 0;
	string FileSystemName;
	// BUGBUG check result
	if (const auto Root = GetPathRoot(m_CurDir); !os::fs::GetVolumeInformation(Root, nullptr, nullptr, nullptr, &FileSystemFlags, &FileSystemName))
	{
		LOGWARNING(L"GetVolumeInformation({}): {}"sv, Root, last_error());
	}

	m_HardlinksSupported = true;
	m_StreamsSupported = true;

	if (IsWindows7OrGreater())
	{
		if (!(FileSystemFlags & FILE_SUPPORTS_HARD_LINKS))
		{
			m_HardlinksSupported = false;
		}
	}
	else
	{
		if (FileSystemName != L"NTFS"sv)
		{
			m_HardlinksSupported = false;
		}
	}

	if(!(FileSystemFlags&FILE_NAMED_STREAMS))
	{
		m_StreamsSupported = false;
	}

	m_ComputerName = ExtractComputerName(m_CurDir);

	SetLastError(ERROR_SUCCESS);
	// сформируем заголовок вне цикла
	const auto Title = MakeLine(m_Where.width() - 2, line_type::h2);
	bool IsShowTitle = false;

	if (!m_Filter)
		m_Filter = std::make_unique<multifilter>(this, FFT_PANEL);

	//Рефреш текущему времени для фильтра перед началом операции
	m_Filter->UpdateCurrentTime();
	Global->CtrlObject->HiFiles->UpdateCurrentTime();
	bool bCurDirRoot = false;
	const auto Type = ParsePath(m_CurDir, nullptr, &bCurDirRoot);
	bool NetRoot = bCurDirRoot && (Type == root_type::remote || Type == root_type::unc_remote);

	const auto Find = os::fs::enum_files(path::join(m_CurDir, L'*'), true);
	bool UseFilter=m_Filter->IsEnabledOnPanel();

	{
		m_ContentPlugins.clear();
		m_ContentNames.clear();
		m_ContentNamesPtrs.clear();
		m_ContentValues.clear();

		std::unordered_set<string> ColumnsSet;

		for (const auto& ColumnsContainer: { &m_ViewSettings.PanelColumns, &m_ViewSettings.StatusColumns })
		{
			for (const auto& Column: *ColumnsContainer)
			{
				if (Column.type == column_type::custom_0)
				{
					if (ColumnsSet.emplace(Column.title).second)
					{
						m_ContentNames.emplace_back(Column.title);
					}
				}
			}
		}

		if (!m_ContentNames.empty())
		{
			m_ContentNamesPtrs.reserve(m_ContentNames.size());
			std::transform(ALL_CONST_RANGE(m_ContentNames), std::back_inserter(m_ContentNamesPtrs), [](const string& i) { return i.c_str(); });
			m_ContentPlugins = Global->CtrlObject->Plugins->GetContentPlugins(m_ContentNamesPtrs);
			m_ContentValues.resize(m_ContentNames.size());
		}
	}

	std::optional<error_state> ErrorState;
	const time_check TimeCheck;

	for (const auto& fdata: Find)
	{
		ErrorState = last_error();

		const auto IsDirectory = (fdata.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (fdata.Attributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM) && !Global->Opt->ShowHidden)
			continue;

		if (UseFilter && !m_Filter->FileInFilter(fdata, fdata.FileName))
		{
			if (!IsDirectory)
				m_FilteredExtensions.emplace(name_ext(fdata.FileName).second);

			continue;
		}

		{
			FileListItem NewItem{};

			static_cast<os::fs::find_data&>(NewItem) = fdata;

			if (!IsDirectory)
			{
				TotalFileSize += NewItem.FileSize;
			}

			NewItem.SortGroup = DEFAULT_SORT_GROUP;
			NewItem.Position = m_ListData.size();
			m_ListData.emplace_back(std::move(NewItem));
		}

		++(IsDirectory? m_TotalDirCount : m_TotalFileCount);

		if (TimeCheck)
		{
			if (IsVisible())
			{
				if (!IsShowTitle)
				{
					Text({ m_Where.left + 1, m_Where.top }, colors::PaletteColorToFarColor(COL_PANELBOX), Title);
					IsShowTitle = true;
					SetColor(IsFocused()? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
				}

				auto strReadMsg = format(msg(lng::MReadingFiles), m_ListData.size());
				inplace::truncate_left(strReadMsg, Title.size() - 2);
				GotoXY(m_Where.left + 1 + static_cast<int>(Title.size() - strReadMsg.size() - 1) / 2, m_Where.top);
				Text(concat(L' ', strReadMsg, L' '));
			}

			Global->CtrlObject->Macro.SuspendMacros(true);
			SCOPE_EXIT{ Global->CtrlObject->Macro.SuspendMacros(false); };

			if (CheckForEscAndConfirmAbort())
				break;
		}
	}

	if (!ErrorState)
		ErrorState = last_error();

	if (!(ErrorState->Win32Error == ERROR_SUCCESS || ErrorState->Win32Error == ERROR_NO_MORE_FILES || ErrorState->Win32Error == ERROR_FILE_NOT_FOUND))
	{
		Message(MSG_WARNING, *ErrorState,
			msg(lng::MError),
			{
				msg(lng::MReadFolderError),
			},
			{ lng::MOk });
	}

	if ((Global->Opt->ShowDotsInRoot || !bCurDirRoot) || (NetRoot && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Network.Id))) // NetWork Plugin
	{
		FileListItem NewItem;
		FillParentPoint(NewItem);

		os::chrono::time_point TwoDotsTimes[4];
		if (os::fs::GetFileTimeSimple(m_CurDir, &TwoDotsTimes[0], &TwoDotsTimes[1], &TwoDotsTimes[2], &TwoDotsTimes[3]))
		{
			NewItem.CreationTime = TwoDotsTimes[0];
			NewItem.LastAccessTime = TwoDotsTimes[1];
			NewItem.LastWriteTime = TwoDotsTimes[2];
			NewItem.ChangeTime = TwoDotsTimes[3];
		}
		else
		{
			LOGWARNING(L"GetFileTimeSimple({}): {}"sv, m_CurDir, last_error());
		}

		NewItem.Position = m_ListData.size();
		m_ListData.emplace_back(std::move(NewItem));
	}

	if (IsColumnDisplayed(column_type::description))
		ReadDiz();

	if (AnotherPanel->GetMode() == panel_mode::PLUGIN_PANEL)
	{
		const auto hAnotherPlugin = AnotherPanel->GetPluginHandle();
		string strPath(m_CurDir);
		AddEndSlash(strPath);
		span<PluginPanelItem> PanelData;
		if (Global->CtrlObject->Plugins->GetVirtualFindData(hAnotherPlugin, PanelData, strPath))
		{
			m_ListData.reserve(m_ListData.size() + PanelData.size());

			OpenPanelInfo AnotherPanelInfo{};
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hAnotherPlugin, &AnotherPanelInfo);

			auto ParentPointSeen = AnotherPanelInfo.Flags & OPIF_ADDDOTS? true : false;

			for (const auto& i: PanelData)
			{
				FileListItem Item{ i };
				Item.PrevSelected = Item.Selected = false;
				Item.ShowFolderSize = 0;
				Item.SortGroup = Global->CtrlObject->HiFiles->GetGroup(Item, this);
				Item.Position = m_ListData.size();
				m_ListData.emplace_back(std::move(Item));

				if (!ParentPointSeen && IsParentDirectory(i))
				{
					ParentPointSeen = true;
				}
				else
				{
					i.FileAttributes & FILE_ATTRIBUTE_DIRECTORY? ++m_TotalDirCount : ++m_TotalFileCount;
				}
				TotalFileSize += i.FileSize;
			}

			Global->CtrlObject->Plugins->FreeVirtualFindData(hAnotherPlugin, PanelData);
		}
	}

	CorrectPosition();

	string strLastSel, strGetSel;

	if (KeepSelection || PrevSelFileCount>0)
	{
		if (LastSelPosition >= 0 && static_cast<size_t>(LastSelPosition) < OldData.size())
			strLastSel = OldData[LastSelPosition].FileName;
		if (GetSelPosition >= 0 && static_cast<size_t>(GetSelPosition) < OldData.size())
			strGetSel = OldData[GetSelPosition].FileName;

		MoveSelection(OldData, m_ListData);
	}

	OldData.clear();

	if (m_SortGroups)
		ReadSortGroups(false);

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(false);

	if (!strLastSel.empty())
		LastSelPosition = FindFile(strLastSel, false);
	if (!strGetSel.empty())
		GetSelPosition = FindFile(strGetSel, false);

	if (m_CurFile >= static_cast<int>(m_ListData.size()) || !equal_icase(m_ListData[m_CurFile].FileName, strCurName))
		if (!GoToFile(strCurName) && !strNextCurName.empty())
			GoToFile(strNextCurName);

	/* $ 13.02.2002 DJ
		SetTitle() - только если мы текущее окно!
	*/
	if (Parent() == Global->WindowManager->GetCurrentWindow().get())
		RefreshTitle();

	FarChDir(strSaveDir); //???
}

void FileList::UpdateIfChanged(bool Changed)
{
	if (Global->Opt->AutoUpdateLimit && m_ListData.size() > static_cast<size_t>(Global->Opt->AutoUpdateLimit))
		return;

	if (m_PanelMode != panel_mode::NORMAL_PANEL)
		return;

	if (!Changed && !FSWatcher.TimeChanged())
		return;

	m_UpdatePending = false;

	if (const auto AnotherPanel = Parent()->GetAnotherPanel(this); AnotherPanel->GetType() == panel_type::INFO_PANEL)
	{
		AnotherPanel->Update(UPDATE_KEEP_SELECTION);
		AnotherPanel->Redraw();
	}

	Update(UPDATE_KEEP_SELECTION);
}

class FileList::background_updater
{
public:
	explicit background_updater(FileList* const Owner):
		m_Owner(Owner)
	{
	}

	const auto& event_id() const
	{
		return m_Listener.GetEventName();
	}

private:
	listener m_Listener{[this]
	{
		if (Global->WindowManager->IsPanelsActive() && m_Owner->IsVisible())
		{
			m_Owner->UpdateIfChanged(true);
			m_Owner->Redraw();
		}
		else
		{
			m_Owner->m_UpdatePending = true;
		}
	}};

	FileList* m_Owner;
};

void FileList::InitFSWatcher(bool CheckTree)
{
	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		return;

	StopFSWatcher();

	DWORD DriveType=DRIVE_REMOTE;
	const auto Type = ParsePath(m_CurDir);

	if (Type == root_type::drive_letter || Type == root_type::win32nt_drive_letter)
	{
		DriveType = os::fs::drive::get_type(os::fs::drive::get_win32nt_root_directory(m_CurDir[Type == root_type::drive_letter? 0 : 4]));
	}

	if (Global->Opt->AutoUpdateRemoteDrive || (!Global->Opt->AutoUpdateRemoteDrive && DriveType != DRIVE_REMOTE) || Type == root_type::volume)
	{
		FSWatcher.Set(m_BackgroundUpdater->event_id(), m_CurDir, CheckTree);
		StartFSWatcher(false, false); //check_time=false, prevent reading file time twice (slow on network)
	}
}

void FileList::StartFSWatcher(bool got_focus, bool check_time)
{
	FSWatcher.Watch(got_focus, check_time);
}

void FileList::StopFSWatcher()
{
	FSWatcher.Release();
}

struct hash_less
{
	struct arg
	{
		arg(unsigned long long const Value): m_Value(Value) {}
		arg(FileListItem const& Value): m_Value(Value.FileId) {}
		unsigned long long m_Value;
	};

	bool operator()(arg const a, arg const b) const
	{
		return a.m_Value < b.m_Value;
	}
};

void FileList::MoveSelection(list_data& From, list_data& To)
{
	m_SelFileCount=0;
	m_SelDirCount = 0;
	SelFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;

	// Repurpose some no longer used fields to turn From into a flat map & intrusively store its order
	for (auto& i: From)
	{
		i.FileId = make_hash(i.FileName);
		i.Position = &i - From.data();
	}

	std::sort(From.begin(), From.end(), hash_less{});

	std::vector<size_t> OldPositions;
	OldPositions.reserve(To.size());

	const auto npos = size_t(-1);

	for (auto& i: To)
	{
		const auto& [EqualBegin, EqualEnd] = std::equal_range(ALL_RANGE(From), make_hash(i.FileName), hash_less{});
		const auto OldItemIterator = std::find_if(EqualBegin, EqualEnd, [&](FileListItem const& Item){ return Item.FileName == i.FileName;});
		if (OldItemIterator == EqualEnd)
		{
			OldPositions.emplace_back(npos);
			continue;
		}

		OldPositions.emplace_back(OldItemIterator->Position);

		if (OldItemIterator->ShowFolderSize)
		{
			i.ShowFolderSize = 2;
			i.FileSize = OldItemIterator->FileSize;
			i.AllocationSize = OldItemIterator->AllocationSize;
		}

		Select(i, OldItemIterator->Selected);
		i.PrevSelected = OldItemIterator->PrevSelected;

		// The state has been transferred, so invalidate the old item to prevent propagating
		// to other items in To with the same name (plugins can do that).
		OldItemIterator->FileName.clear();
	}

	std::sort(ALL_RANGE(To), [&](FileListItem const& a, FileListItem const& b)
	{
		const auto OldPosA = OldPositions[a.Position];
		const auto OldPosB = OldPositions[b.Position];

		return OldPosA != npos && OldPosB != npos?
			&From[OldPosA] < &From[OldPosB] :
			a.Position < b.Position;
	});

	From.clear();
}

void FileList::UpdatePlugin(bool const KeepSelection, bool const UpdateEvenIfPanelInvisible)
{
	if (!IsVisible() && !UpdateEvenIfPanelInvisible)
	{
		UpdateRequired = true;
		m_KeepSelection = KeepSelection;
		return;
	}

	DizRead = false;
	decltype(m_ListData) OldData;
	std::optional<string> strCurName, strNextCurName;
	LastCurFile=-1;

	const auto Item = GetPluginItem();
	Global->CtrlObject->Plugins->GetOpenPanelInfo(Item.lock()->m_Plugin.get(), &m_CachedOpenPanelInfo);
	if (Item.expired()) return;

	FreeDiskSize=-1;
	if (Global->Opt->ShowPanelFree)
	{
		if (m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES)
		{
			// BUGBUG check result
			if (!os::fs::get_disk_size(m_CurDir, nullptr, nullptr, &FreeDiskSize))
			{
				LOGWARNING(L"get_disk_size({}): {}"sv, m_CurDir, last_error());
			}
		}
		else if (m_CachedOpenPanelInfo.Flags & OPIF_USEFREESIZE)
			FreeDiskSize = m_CachedOpenPanelInfo.FreeSize;
	}

	span<PluginPanelItem> PanelData;

	int result = FALSE;
	{
		++m_InsideGetFindData;
		SCOPE_EXIT { --m_InsideGetFindData; };
		result = Global->CtrlObject->Plugins->GetFindData(Item.lock()->m_Plugin.get(), PanelData, 0);
	}
	if (Item.expired())
	{
		Update(0);
		//панель не сортируется внутри GetFindData, и если плагиновая панель закрыта, то панель - несортированная,
		//Update сохранил позицию, поэтому переместим курсор в начало.
		GoToFile(0);
		return;
	}
	if (!result)
	{
		PopPlugin(TRUE);
		Update(KeepSelection);

		// WARP> явный хак, но очень способствует - восстанавливает позицию на панели при ошибке чтения архива.
		if (!PrevDataList.empty())
			GoToFile(PrevDataList.back().strPrevName);

		return;
	}

	size_t PrevSelFileCount=m_SelFileCount;
	m_SelFileCount=0;
	m_SelDirCount = 0;
	SelFileSize=0;
	m_TotalFileCount=0;
	m_TotalDirCount = 0;
	TotalFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;
	strPluginDizName.clear();

	if (!m_ListData.empty())
	{
		strCurName = m_ListData[m_CurFile].FileName;

		if (m_ListData[m_CurFile].Selected)
		{
			const auto ItemIterator = std::find_if(m_ListData.cbegin() + m_CurFile + 1, m_ListData.cend(), [](const auto& i) { return !i.Selected; });
			if (ItemIterator != m_ListData.cend())
			{
				strNextCurName = ItemIterator->FileName;
			}
		}
	}
	else if (m_CachedOpenPanelInfo.Flags & OPIF_ADDDOTS)
	{
		strCurName = L".."sv;
	}

	if (KeepSelection || PrevSelFileCount>0)
	{
		OldData = std::move(m_ListData);
	}

	m_ListData.initialise(Item.lock()->m_Plugin.get());
	m_FilteredExtensions.clear();

	if (!m_Filter)
		m_Filter = std::make_unique<multifilter>(this, FFT_PANEL);

	//Рефреш текущему времени для фильтра перед началом операции
	m_Filter->UpdateCurrentTime();
	Global->CtrlObject->HiFiles->UpdateCurrentTime();
	bool UseFilter=m_Filter->IsEnabledOnPanel();

	m_ListData.reserve(PanelData.size() + ((m_CachedOpenPanelInfo.Flags & OPIF_ADDDOTS)? 1 : 0));

	struct
	{
		FileListItem* Item{};
		bool TryToFind{ true };
	}
	ParentPoint;

	if (m_CachedOpenPanelInfo.Flags & OPIF_ADDDOTS)
	{
		FileListItem NewItem;
		FillParentPoint(NewItem);

		if (m_CachedOpenPanelInfo.HostFile && *m_CachedOpenPanelInfo.HostFile)
		{
			os::fs::find_data FindData;

			if (os::fs::get_find_data(m_CachedOpenPanelInfo.HostFile, FindData))
			{
				NewItem.LastWriteTime = FindData.LastWriteTime;
				NewItem.CreationTime = FindData.CreationTime;
				NewItem.LastAccessTime = FindData.LastAccessTime;
				NewItem.ChangeTime = FindData.ChangeTime;
			}
		}
		NewItem.Position = m_ListData.size();
		m_ListData.emplace_back(std::move(NewItem));
		ParentPoint.TryToFind = false;
	}

	for (const auto& PanelItem: PanelData)
	{
		if (UseFilter && !(m_CachedOpenPanelInfo.Flags & OPIF_DISABLEFILTER))
		{
			if (!m_Filter->FileInFilter(PanelItem))
			{
				if (!(PanelItem.FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					m_FilteredExtensions.emplace(name_ext(PanelItem.FileName).second);

				continue;
			}
		}

		if (!Global->Opt->ShowHidden && (PanelItem.FileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
			continue;

		FileListItem NewItem(PanelItem);

		NewItem.SortGroup = (m_CachedOpenPanelInfo.Flags & OPIF_DISABLESORTGROUPS)? DEFAULT_SORT_GROUP : Global->CtrlObject->HiFiles->GetGroup(NewItem, this);
		NewItem.Position = m_ListData.size();
		m_ListData.emplace_back(std::move(NewItem));

		if (ParentPoint.TryToFind && !ParentPoint.Item && IsParentDirectory(PanelItem))
		{
			// We reserve capacity so no reallocation will happen and pointer will stay valid.
			ParentPoint.Item = &m_ListData.back();
			FillParentPoint(*ParentPoint.Item);
		}
		else
		{
			PanelItem.FileAttributes & FILE_ATTRIBUTE_DIRECTORY? ++m_TotalDirCount : ++m_TotalFileCount;
			TotalFileSize += PanelItem.FileSize;
		}
	}

	if (m_CurFile >= static_cast<int>(m_ListData.size()))
		m_CurFile = m_ListData.empty()? 0 : static_cast<int>(m_ListData.size() - 1);

	/* $ 25.02.2001 VVM
	    ! Не считывать повторно список файлов с панели плагина */
	if (IsColumnDisplayed(column_type::description))
		ReadDiz(PanelData);

	CorrectPosition();
	Global->CtrlObject->Plugins->FreeFindData(Item.lock()->m_Plugin.get(), PanelData, false);

	std::optional<string> strLastSel, strGetSel;

	if (KeepSelection || PrevSelFileCount>0)
	{
		if (LastSelPosition >= 0 && LastSelPosition < static_cast<long>(OldData.size()))
			strLastSel = OldData[LastSelPosition].FileName;
		if (GetSelPosition >= 0 && GetSelPosition < static_cast<long>(OldData.size()))
			strGetSel = OldData[GetSelPosition].FileName;

		MoveSelection(OldData, m_ListData);
	}

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(false);

	if (strLastSel)
		LastSelPosition = FindFile(*strLastSel, false);
	if (strGetSel)
		GetSelPosition = FindFile(*strGetSel, false);

	if (strCurName && (m_CurFile >= static_cast<int>(m_ListData.size()) || !equal_icase(m_ListData[m_CurFile].FileName, *strCurName)))
		if (!GoToFile(*strCurName) && strNextCurName)
			GoToFile(*strNextCurName);

	RefreshTitle();
}


void FileList::ReadDiz(span<PluginPanelItem> const Items)
{
	if (DizRead)
		return;

	DizRead = true;
	Diz.Reset();

	if (m_PanelMode == panel_mode::NORMAL_PANEL)
	{
		Diz.Read(m_CurDir);
	}
	else
	{
		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

		if (!m_CachedOpenPanelInfo.DescrFilesNumber)
			return;

		int GetCode=TRUE;

		span<PluginPanelItem> PanelData;

		if (Items.empty())
		{
			GetCode = Global->CtrlObject->Plugins->GetFindData(GetPluginHandle(), PanelData, 0);
		}
		else
		{
			PanelData = Items;
		}

		if (GetCode)
		{
			for (const auto& i: span(m_CachedOpenPanelInfo.DescrFiles, m_CachedOpenPanelInfo.DescrFilesNumber))
			{
				for (auto& CurPanelData: PanelData)
				{
					if (!equal_icase(CurPanelData.FileName, i))
						continue;

					const auto strTempDir = MakeTemp();

					if (!os::fs::create_directory(strTempDir))
					{
						LOGWARNING(L"create_directory({}): {}"sv, strTempDir, last_error());
						continue;
					}

					string strDizName;
					if (Global->CtrlObject->Plugins->GetFile(GetPluginHandle(), &CurPanelData, strTempDir, strDizName, OPM_SILENT | OPM_VIEW | OPM_QUICKVIEW | OPM_DESCR))
					{
						strPluginDizName = i;
						Diz.Read({}, &strDizName);
						DeleteFileWithFolder(strDizName);
						break;
					}

					// BUGBUG check result
					if (!os::fs::remove_directory(strTempDir))
					{
						LOGWARNING(L"remove_directory({}): {}"sv, strTempDir, last_error());
					}
					//ViewPanel->ShowFile(nullptr,FALSE,nullptr);
				}
			}

			if (Items.empty())
				Global->CtrlObject->Plugins->FreeFindData(GetPluginHandle(), PanelData, true);
		}
	}

	for(auto& i: m_ListData)
	{
		if (!i.DizText)
		{
			i.DeleteDiz = false;
			// It's ok, the description is null-terminated here
			i.DizText = Diz.Get(i.FileName, i.AlternateFileName(), i.FileSize).data();
		}
	}
}


void FileList::ReadSortGroups(bool UpdateFilterCurrentTime)
{
	if (SortGroupsRead)
		return;

	if (UpdateFilterCurrentTime)
	{
		Global->CtrlObject->HiFiles->UpdateCurrentTime();
	}

	for (auto& i: m_ListData)
	{
		i.SortGroup = Global->CtrlObject->HiFiles->GetGroup(i, this);
	}

	SortGroupsRead = true;
}

// занести предопределенные данные для каталога ".."
void FileList::FillParentPoint(FileListItem& Item)
{
	Item.Attributes = FILE_ATTRIBUTE_DIRECTORY;
	Item.FileName = L".."sv;
	Item.SetAlternateFileName(Item.FileName);
	Item.UserFlags = PPIF_RESERVED;
}

// flshow.cpp
// Файловая панель - вывод на экран

void FileList::UpdateHeight()
{
	m_Height = m_Where.height() - 2 - (Global->Opt->ShowColumnTitles? 1 : 0) - (Global->Opt->ShowPanelStatus? 2 : 0);
}

void FileList::DisplayObject()
{
	UpdateHeight();

	if (UpdateRequired)
	{
		UpdateRequired = false;
		Update(m_KeepSelection? UPDATE_KEEP_SELECTION : 0);
	}
	ShowFileList(false);
}

void FileList::ShowFileList(bool Fast)
{
	string strTitle;
	string strInfoCurDir;

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		if (ProcessPluginEvent(FE_REDRAW,nullptr))
			return;

		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
		strInfoCurDir = NullToEmpty(m_CachedOpenPanelInfo.CurDir);
	}
	else
	{
		UpdateIfChanged(m_UpdatePending);
	}

	bool CurFullScreen=IsFullScreen();
	PrepareViewSettings(m_ViewMode);
	CorrectPosition();

	if (CurFullScreen!=IsFullScreen())
	{
		Parent()->SetScreenPosition();
		Parent()->GetAnotherPanel(this)->Update(UPDATE_KEEP_SELECTION | UPDATE_SECONDARY);
	}

	if (!ProcessingPluginCommand && LastCurFile != m_CurFile)
	{
		LastCurFile = m_CurFile;
		UpdateViewPanel();
	}

	SetScreen({ m_Where.left + 1, m_Where.top + 1, m_Where.right - 1, m_Where.bottom - 1 }, L' ', colors::PaletteColorToFarColor(COL_PANELTEXT));
	Box(m_Where, colors::PaletteColorToFarColor(COL_PANELBOX), DOUBLE_BOX);

	if (Global->Opt->ShowColumnTitles)
	{
//    SetScreen(X1+1,Y1+1,X2-1,Y1+1,' ',COL_PANELTEXT);
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y1+1);
		//Text(string(X2 - X1 - 1, L' '));
	}

	for (size_t I = 0, ColumnPos = m_Where.left + 1; I < m_ViewSettings.PanelColumns.size(); I++)
	{
		if (m_ViewSettings.PanelColumns[I].width < 0)
			continue;

		if (Global->Opt->ShowColumnTitles)
		{
			const auto IDMessage = [&]
			{
				switch (m_ViewSettings.PanelColumns[I].type)
				{
				case column_type::name:               return lng::MColumnName;
				case column_type::extension:          return lng::MColumnExtension;
				case column_type::size:               return lng::MColumnSize;
				case column_type::size_compressed:    return lng::MColumnAlocatedSize;
				case column_type::date:               return lng::MColumnDate;
				case column_type::time:               return lng::MColumnTime;
				case column_type::date_write:         return lng::MColumnWrited;
				case column_type::date_creation:      return lng::MColumnCreated;
				case column_type::date_access:        return lng::MColumnAccessed;
				case column_type::date_change:        return lng::MColumnChanged;
				case column_type::attributes:         return lng::MColumnAttr;
				case column_type::description:        return lng::MColumnDescription;
				case column_type::owner:              return lng::MColumnOwner;
				case column_type::links_number:       return lng::MColumnMumLinks;
				case column_type::streams_number:     return lng::MColumnNumStreams;
				case column_type::streams_size:       return lng::MColumnStreamsSize;
				default:                              return lng::MColumnUnknown;
				}
			}();

			strTitle = IDMessage == lng::MColumnUnknown && !m_ViewSettings.PanelColumns[I].title.empty()? m_ViewSettings.PanelColumns[I].title : msg(IDMessage);

			if (m_PanelMode == panel_mode::PLUGIN_PANEL && m_CachedOpenPanelInfo.PanelModesArray &&
			        m_ViewMode<static_cast<int>(m_CachedOpenPanelInfo.PanelModesNumber) &&
			        m_CachedOpenPanelInfo.PanelModesArray[m_ViewMode].ColumnTitles)
			{
				const wchar_t *NewTitle = m_CachedOpenPanelInfo.PanelModesArray[m_ViewMode].ColumnTitles[I];

				if (NewTitle)
					strTitle=NewTitle;
			}

			SetColor(COL_PANELCOLUMNTITLE);
			GotoXY(static_cast<int>(ColumnPos), m_Where.top + 1);
			Text(fit_to_center(strTitle, m_ViewSettings.PanelColumns[I].width));
		}

		if (I == m_ViewSettings.PanelColumns.size() - 1)
			break;

		if (m_ViewSettings.PanelColumns[I + 1].width < 0)
			continue;

		SetColor(COL_PANELBOX);
		ColumnPos += m_ViewSettings.PanelColumns[I].width;
		GotoXY(static_cast<int>(ColumnPos), m_Where.top);

		const auto DoubleLine = !((I + 1) % m_ColumnsInStripe);

		Text(BoxSymbols[DoubleLine?BS_T_H2V2:BS_T_H2V1]);

		if (Global->Opt->ShowColumnTitles)
		{
			const auto& ColumnTitleColor = colors::PaletteColorToFarColor(COL_PANELCOLUMNTITLE);
			auto Color = colors::PaletteColorToFarColor(COL_PANELBOX);
			Color.BackgroundColor = ColumnTitleColor.BackgroundColor;
			Color.SetBg4Bit(ColumnTitleColor.IsBg4Bit());
			SetColor(Color);

			GotoXY(static_cast<int>(ColumnPos), m_Where.top + 1);
			Text(BoxSymbols[DoubleLine?BS_V2:BS_V1]);
		}

		if (!Global->Opt->ShowPanelStatus)
		{
			GotoXY(static_cast<int>(ColumnPos), m_Where.bottom);
			Text(BoxSymbols[DoubleLine?BS_B_H2V2:BS_B_H2V1]);
		}

		ColumnPos++;
	}

	int NextX1 = m_Where.left + 1;

	if (Global->Opt->ShowSortMode)
	{
		wchar_t Indicator = 0;

		if (m_SortMode < panel_sort::COUNT)
		{
			const auto& CurrenModeData = SortModes[static_cast<size_t>(m_SortMode)];
			const auto& CurrentModeName = msg(CurrenModeData.Label);
			// Owerflow from npos to 0 is ok - pick the first character if & isn't there.
			const auto Char = CurrentModeName[CurrentModeName.find(L'&') + 1];
			const auto UseReverseIndicator = Global->Opt->ReverseSortCharCompat && Global->Opt->PanelSortLayers[static_cast<size_t>(m_SortMode)].front().second == sort_order::descend? !m_ReverseSortOrder: m_ReverseSortOrder;
			Indicator = UseReverseIndicator? upper(Char) : lower(Char);
		}
		else if (m_SortMode >= panel_sort::BY_USER)
		{
			Indicator = m_ReverseSortOrder? CustomSortIndicator[1] : CustomSortIndicator[0];
		}
		else
		{
			LOGWARNING(L"Unknown sort mode {}"sv, m_SortMode);
		}

		if (Indicator)
		{
			GotoXY(NextX1, m_Where.top + (Global->Opt->ShowColumnTitles? 1 : 0));
			SetColor(COL_PANELCOLUMNTITLE);
			Text(Indicator);
			NextX1++;

			if (m_Filter && m_Filter->IsEnabledOnPanel())
			{
				Text(L"*"sv);
				NextX1++;
			}
		}
	}

	/* <режимы сортировки> */
	if (/* GetSortGroups() || */GetSelectedFirstMode())
	{
		GotoXY(NextX1, m_Where.top + (Global->Opt->ShowColumnTitles? 1 : 0));
		SetColor(COL_PANELCOLUMNTITLE);

		string Indicators;

		//if (GetSelectedFirstMode())
			Indicators.push_back(L'^');

		/*
		if(GetSortGroups())
			Indicators.push_back(L'@');
		*/
		Text(Indicators);
	}

	/* </режимы сортировки> */

	if (!Fast && IsFocused())
	{
		if (m_PanelMode == panel_mode::PLUGIN_PANEL)
			Parent()->GetCmdLine()->SetCurDir(NullToEmpty(m_CachedOpenPanelInfo.CurDir));
		else
			Parent()->GetCmdLine()->SetCurDir(m_CurDir);

		Parent()->GetCmdLine()->Show();
	}

	strTitle = GetTitle();
	int TitleX2 = m_Where.right;
	if (Global->Opt->Clock && !Global->Opt->ShowMenuBar && m_Where.left + strTitle.size() + 2 >= ScrX - Global->CurrentTime.size())
		TitleX2 = std::min(static_cast<int>(ScrX - Global->CurrentTime.size()), static_cast<int>(m_Where.right));

	int MaxSize = TitleX2 - m_Where.left - 1;
	int XShift = 0;
	if (!Global->Opt->ShowColumnTitles && Global->Opt->ShowSortMode)
	{
		++XShift;
		if (m_Filter && m_Filter->IsEnabledOnPanel())
			++XShift;
	}
	MaxSize -= XShift;
	if (MaxSize >= 2)
	{
		inplace::truncate_path(strTitle, MaxSize - 2);
	}
	strTitle.insert(0, 1, L' ');
	strTitle.push_back(L' ');

	const auto TitleSize = static_cast<int>(strTitle.size());
	int TitleX = m_Where.left + 1 + XShift + (TitleX2 - m_Where.left - XShift - TitleSize) / 2;

	if (Global->Opt->Clock && !Global->Opt->ShowMenuBar && TitleX + TitleSize > ScrX - static_cast<int>(Global->CurrentTime.size()))
		TitleX = ScrX - static_cast<int>(Global->CurrentTime.size()) - TitleSize;

	SetColor(IsFocused()? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
	GotoXY(TitleX, m_Where.top);
	Text(strTitle);

	if (m_ListData.empty())
	{
		SetScreen({ m_Where.left + 1, m_Where.bottom - 1, m_Where.right - 1, m_Where.bottom - 1 }, L' ', colors::PaletteColorToFarColor(COL_PANELTEXT));
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y2-1);
		//Text(string(X2 - X1 - 1, L' '));
	}

	if (m_PanelMode == panel_mode::PLUGIN_PANEL
		&& !m_ListData.empty()
		&& (m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES)
		// Network plugin sets OPIF_REALNAMES when showing list of shares as they are real FS objects (see M#650).
		// However, \\server itself is not a real FS directory in any way and it is not possible to set it on a real panel.
		// We can't simply remove the code below, as other plugins rely on this feature (e. g. Ctrl+PgUp in TmpPanel
		// "exits" to the CurDir, dynamically set by the plugin), so disabling it for the Network plugin only for now.
		// Yes, it is ugly, I know. It probably would be better to set CurDir in the Network plugin dynamically in this case
		// as \\server\current_share_under_cursor, but the plugin doesn't keep that information currently.
		&& GetPluginHandle()->plugin()->Id() != Global->Opt->KnownIDs.Network.Id
	)
	{
		if (!strInfoCurDir.empty())
		{
			m_CurDir = strInfoCurDir;
		}
		else
		{
			if (!IsParentDirectory(m_ListData[m_CurFile]))
			{
				m_CurDir=m_ListData[m_CurFile].FileName;
				const auto pos = FindLastSlash(m_CurDir);
				if (pos != string::npos && pos)
				{
					m_CurDir.resize(m_CurDir[pos - 1] != L':'? pos : pos + 1);
				}
			}
			else
			{
				m_CurDir = strOriginalCurDir;
			}
		}

		if (IsFocused())
		{
			Parent()->GetCmdLine()->SetCurDir(m_CurDir);
			Parent()->GetCmdLine()->Show();
		}
	}

	if ((Global->Opt->ShowPanelTotals || Global->Opt->ShowPanelFree) &&
	        (Global->Opt->ShowPanelStatus || !m_SelFileCount))
	{
		ShowTotalSize(m_CachedOpenPanelInfo);
	}

	ShowList(FALSE,0);
	ShowSelectedSize();

	if (Global->Opt->ShowPanelScrollbar)
	{
		SetColor(COL_PANELSCROLLBAR);

		const auto per_stripe = [&](size_t const Value)
		{
			return Value / m_Stripes + (Value % m_Stripes != 0);
		};

		ScrollBar(
			m_Where.right,
			m_Where.top + 1 + Global->Opt->ShowColumnTitles,
			m_Height,
			per_stripe(m_CurTopFile),
			per_stripe(m_ListData.size()));
	}

	ShowScreensCount();

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		Parent()->RedrawKeyBar();
}


FarColor FileList::GetShowColor(int Position, bool FileColor) const
{
	auto ColorAttr = colors::PaletteColorToFarColor(COL_PANELTEXT);

	if (static_cast<size_t>(Position) >= m_ListData.size())
		return ColorAttr;

	int Pos = highlight::color::normal;

	if (m_CurFile == Position && IsFocused() && !m_ListData.empty())
	{
		Pos=m_ListData[Position].Selected? highlight::color::selected_current : highlight::color::normal_current;
	}
	else if (m_ListData[Position].Selected)
	{
		Pos = highlight::color::selected;
	}

	const auto HighlightingEnabled = Global->Opt->Highlight && (m_PanelMode != panel_mode::PLUGIN_PANEL || !(m_CachedOpenPanelInfo.Flags & OPIF_DISABLEHIGHLIGHTING));

	if (HighlightingEnabled)
	{
		if (!m_ListData[Position].Colors)
		{
			const auto UseAttrHighlighting = m_PanelMode == panel_mode::PLUGIN_PANEL && m_CachedOpenPanelInfo.Flags & OPIF_USEATTRHIGHLIGHTING;
			m_ListData[Position].Colors = Global->CtrlObject->HiFiles->GetHiColor(m_ListData[Position], this, UseAttrHighlighting);
		}

		auto Colors = m_ListData[Position].Colors->Color[Pos];
		highlight::configuration::ApplyFinalColor(Colors, Pos);
		ColorAttr = FileColor ? Colors.FileColor : Colors.MarkColor;
	}

	if (!HighlightingEnabled || (!ColorAttr.ForegroundColor && !ColorAttr.BackgroundColor)) // black on black, default
	{
		static const PaletteColors PalColor[] = {COL_PANELTEXT, COL_PANELSELECTEDTEXT, COL_PANELCURSOR, COL_PANELSELECTEDCURSOR};
		ColorAttr=colors::PaletteColorToFarColor(PalColor[Pos]);
	}

	return ColorAttr;
}

void FileList::SetShowColor(int Position, bool FileColor) const
{
	SetColor(GetShowColor(Position,FileColor));
}

static string size2str(uint64_t const Size, int const Width, bool const FloatStyle, bool const ShowBytes)
{
	if (ShowBytes)
	{
		return GroupDigits(Size);
	}

	if (FloatStyle) // float style
	{
		return trim(FileSizeToStr(Size, Width, COLFLAGS_FLOATSIZE | COLFLAGS_SHOW_MULTIPLIER));
	}

	auto Str = str(Size);
	if (static_cast<int>(Str.size()) <= Width)
		return Str;

	return trim(FileSizeToStr(Size, Width, COLFLAGS_SHOW_MULTIPLIER));
}

void FileList::ShowSelectedSize()
{
	if (Global->Opt->ShowPanelStatus)
	{
		SetColor(COL_PANELBOX);
		DrawSeparator(m_Where.bottom - 2);
		for (size_t I = 0, ColumnPos = m_Where.left + 1; I < m_ViewSettings.PanelColumns.size() - 1; I++)
		{
			if (m_ViewSettings.PanelColumns[I].width < 0 || (I == m_ViewSettings.PanelColumns.size() - 2 && m_ViewSettings.PanelColumns[I+1].width < 0))
				continue;

			ColumnPos += m_ViewSettings.PanelColumns[I].width;
			GotoXY(static_cast<int>(ColumnPos), m_Where.bottom - 2);

			const auto DoubleLine = !((I + 1) % m_ColumnsInStripe);
			Text(BoxSymbols[DoubleLine?BS_B_H1V2:BS_B_H1V1]);
			ColumnPos++;
		}
	}

	if (m_SelFileCount)
	{
		auto strFormStr = size2str(SelFileSize, 6, false, true);
		auto strSelStr = format(msg(lng::MListFileSize), strFormStr, m_SelFileCount - m_SelDirCount, m_SelDirCount, m_SelFileCount);
		const auto BorderSize = 1;
		const auto MarginSize = 1;
		const auto AvailableWidth = static_cast<size_t>(std::max(0, ObjWidth() - BorderSize * 2 - MarginSize * 2));
		if (strSelStr.size() > AvailableWidth)
		{
			strFormStr = size2str(SelFileSize, 6, false, false);
			strSelStr = format(msg(lng::MListFileSize), strFormStr, m_SelFileCount - m_SelDirCount, m_SelDirCount, m_SelFileCount);
			if (strSelStr.size() > AvailableWidth)
				inplace::truncate_right(strSelStr, AvailableWidth);
		}
		SetColor(COL_PANELSELECTEDINFO);
		GotoXY(static_cast<int>(m_Where.left + BorderSize + (AvailableWidth - strSelStr.size()) / 2), m_Where.bottom - 2 * Global->Opt->ShowPanelStatus);
		Text(L' ');
		Text(strSelStr);
		Text(L' ');
	}
}

void FileList::ShowTotalSize(const OpenPanelInfo &Info)
{
	if (!Global->Opt->ShowPanelTotals && m_PanelMode == panel_mode::PLUGIN_PANEL && !(Info.Flags & OPIF_REALNAMES))
		return;

	const auto calc_total_string = [this, Info](bool ShowBytes)
	{
		string strFreeSize, strTotalSize;
		auto strFormSize = size2str(TotalFileSize, 10, true, ShowBytes);
		if (Global->Opt->ShowPanelFree && (m_PanelMode != panel_mode::PLUGIN_PANEL || (Info.Flags & (OPIF_REALNAMES | OPIF_USEFREESIZE))))
			strFreeSize = (FreeDiskSize != static_cast<unsigned long long>(-1)) ? size2str(FreeDiskSize, 10, true, ShowBytes) : L"?"sv;

		if (Global->Opt->ShowPanelTotals)
		{
			if (!Global->Opt->ShowPanelFree || strFreeSize.empty())
			{
				strTotalSize = format(msg(lng::MListFileSize), strFormSize, m_TotalFileCount, m_TotalDirCount, m_TotalFileCount + m_TotalDirCount);
			}
			else
			{
				const string DHLine(3, BoxSymbols[BS_H2]);
				strTotalSize = format(msg(lng::MListFileSizeStatus), strFormSize, m_TotalFileCount, m_TotalDirCount, DHLine, strFreeSize);
			}
		}
		else
		{
			strTotalSize = format(msg(lng::MListFreeSize), strFreeSize.empty() ? L"?"s : strFreeSize);
		}
		return strTotalSize;
	};

	const auto BorderSize = 1;
	const auto MarginSize = 1;
	const auto AvailableWidth = static_cast<size_t>(std::max(0, ObjWidth() - BorderSize * 2 - MarginSize * 2));

	auto TotalStr = calc_total_string(Global->Opt->ShowBytes);
	if (TotalStr.size() > AvailableWidth)
	{
		if (Global->Opt->ShowBytes)
			TotalStr = calc_total_string(false);
		inplace::truncate_right(TotalStr, AvailableWidth);
	}

	const string_view TotalStrView = TotalStr;

	SetColor(COL_PANELTOTALINFO);
	GotoXY(static_cast<int>(m_Where.left + BorderSize + (AvailableWidth - TotalStrView.size()) / 2), m_Where.bottom);
	const auto BoxPos = TotalStrView.find(BoxSymbols[BS_H2]);

	Text(L' ');
	if (const auto BoxLength = BoxPos == string::npos? 0 : std::count(TotalStrView.begin() + BoxPos, TotalStrView.end(), BoxSymbols[BS_H2]))
	{
		Text(TotalStrView.substr(0, BoxPos));
		SetColor(COL_PANELBOX);
		Text(TotalStrView.substr(BoxPos, BoxLength));
		SetColor(COL_PANELTOTALINFO);
		Text(TotalStrView.substr(BoxPos + BoxLength));
	}
	else
	{
		Text(TotalStrView);
	}
	Text(L' ');
}

bool FileList::ConvertName(const string_view SrcName, string& strDest, const int MaxLength, const unsigned long long RightAlign, const int ShowStatus, os::fs::attributes const FileAttr) const
{
	strDest.reserve(MaxLength);

	const auto SrcLength = static_cast<int>(SrcName.size());

	if ((RightAlign & COLFLAGS_RIGHTALIGNFORCE) || (RightAlign && (SrcLength>MaxLength)))
	{
		if (SrcLength>MaxLength)
		{
			strDest = SrcName.substr(SrcLength - MaxLength, MaxLength);
		}
		else
		{
			strDest.assign(MaxLength - SrcLength, L' ');
			append(strDest, SrcName);
		}
		return SrcLength > MaxLength;
	}

	string_view Extension;

	if (!ShowStatus &&
	        ((!(FileAttr & FILE_ATTRIBUTE_DIRECTORY) && (m_ViewSettings.Flags & PVS_ALIGNEXTENSIONS)) ||
	          ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && (m_ViewSettings.Flags & PVS_FOLDERALIGNEXTENSIONS))) &&
	        SrcLength <= MaxLength &&
	        (Extension = name_ext(SrcName).second).size() > 1 && Extension.size() != SrcName.size() &&
	        (SrcName.size() > 2 || SrcName[0] != L'.') && !contains(Extension, L' '))
	{
		Extension.remove_prefix(1);
		auto Name = SrcName.substr(0, SrcName.size() - Extension.size());
		const auto DotPos = std::max(MaxLength - std::max(Extension.size(), size_t(3)), Name.size());

		if (Name.size() > 1 && Name[Name.size() - 2] != L' ')
			Name.remove_suffix(1);

		strDest += Name;
		strDest.resize(DotPos, L' ');
		strDest += Extension;
		strDest.resize(MaxLength, L' ');
	}
	else
	{
		strDest.assign(SrcName, 0, std::min(SrcLength, MaxLength));
		strDest.resize(MaxLength, L' ');
	}

	return SrcLength > MaxLength;
}

void FileList::PrepareViewSettings(int ViewMode)
{
	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
	}

	m_ViewSettings = Global->Opt->ViewSettings[ViewMode].clone();

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		if (m_CachedOpenPanelInfo.PanelModesArray && ViewMode<static_cast<int>(m_CachedOpenPanelInfo.PanelModesNumber) &&
			m_CachedOpenPanelInfo.PanelModesArray[ViewMode].ColumnTypes &&
			m_CachedOpenPanelInfo.PanelModesArray[ViewMode].ColumnWidths)
		{
			m_ViewSettings.PanelColumns = DeserialiseViewSettings(m_CachedOpenPanelInfo.PanelModesArray[ViewMode].ColumnTypes, m_CachedOpenPanelInfo.PanelModesArray[ViewMode].ColumnWidths);

			if (m_CachedOpenPanelInfo.PanelModesArray[ViewMode].StatusColumnTypes &&
				m_CachedOpenPanelInfo.PanelModesArray[ViewMode].StatusColumnWidths)
			{
				m_ViewSettings.StatusColumns = DeserialiseViewSettings(m_CachedOpenPanelInfo.PanelModesArray[ViewMode].StatusColumnTypes, m_CachedOpenPanelInfo.PanelModesArray[ViewMode].StatusColumnWidths);
			}
			else if (m_CachedOpenPanelInfo.PanelModesArray[ViewMode].Flags&PMFLAGS_DETAILEDSTATUS)
			{
				m_ViewSettings.StatusColumns =
				{
					{ column_type::name, COLFLAGS_RIGHTALIGN, 0, },
					{ column_type::size, 0,                 8, },
					{ column_type::date, 0,                 0, },
					{ column_type::time, 0,                 5, },
				};
			}
			else
			{
				m_ViewSettings.StatusColumns =
				{
					{ column_type::name, COLFLAGS_RIGHTALIGN, 0, },
				};
			}

			if (m_CachedOpenPanelInfo.PanelModesArray[ViewMode].Flags&PMFLAGS_FULLSCREEN)
				m_ViewSettings.Flags|=PVS_FULLSCREEN;
			else
				m_ViewSettings.Flags&=~PVS_FULLSCREEN;

			if (m_CachedOpenPanelInfo.PanelModesArray[ViewMode].Flags&PMFLAGS_ALIGNEXTENSIONS)
				m_ViewSettings.Flags|=PVS_ALIGNEXTENSIONS;
			else
				m_ViewSettings.Flags&=~PVS_ALIGNEXTENSIONS;

			if (!(m_CachedOpenPanelInfo.PanelModesArray[ViewMode].Flags&PMFLAGS_CASECONVERSION))
				m_ViewSettings.Flags&=~(PVS_FOLDERUPPERCASE|PVS_FILELOWERCASE|PVS_FILEUPPERTOLOWERCASE);
		}
		else
		{
			const auto NameOnlyFlag = m_CachedOpenPanelInfo.Flags & OPIF_SHOWNAMESONLY? COLFLAGS_NAMEONLY : COLFLAGS_NONE;
			const auto RightAlignFlag = m_CachedOpenPanelInfo.Flags & OPIF_SHOWRIGHTALIGNNAMES? COLFLAGS_RIGHTALIGN : COLFLAGS_NONE;

			for (auto& i: m_ViewSettings.PanelColumns)
			{
				if (i.type == column_type::name)
				{
					i.type_flags |= NameOnlyFlag;
					i.type_flags |= RightAlignFlag;
				}
			}

			if (m_CachedOpenPanelInfo.Flags & OPIF_SHOWPRESERVECASE)
				m_ViewSettings.Flags&=~(PVS_FOLDERUPPERCASE|PVS_FILELOWERCASE|PVS_FILEUPPERTOLOWERCASE);
		}
	}

	PreparePanelView();
	UpdateHeight();
}


void FileList::PreparePanelView()
{
	PrepareColumnWidths(m_ViewSettings.StatusColumns, (m_ViewSettings.Flags&PVS_FULLSCREEN) != 0);
	PrepareColumnWidths(m_ViewSettings.PanelColumns, (m_ViewSettings.Flags&PVS_FULLSCREEN) != 0);
	PrepareStripes(m_ViewSettings.PanelColumns);
}


void FileList::PrepareColumnWidths(std::vector<column>& Columns, bool FullScreen) const
{
	int ZeroLengthCount = 0;
	int EmptyColumns = 0;
	int TotalPercentCount = 0;
	int TotalPercentWidth = 0;
	int TotalWidth = static_cast<int>(Columns.size() - 1);

	for (auto& i: Columns)
	{
		if (i.width < 0)
		{
			EmptyColumns++;
			continue;
		}

		if (!i.width)
		{
			i.width_type = col_width::fixed; //manage all zero-width columns in same way
			i.width = GetDefaultWidth(i);
		}

		if (!i.width)
			ZeroLengthCount++;

		switch (i.width_type)
		{
			case col_width::fixed:
				TotalWidth += i.width;
				break;
			case col_width::percent:
				TotalPercentWidth += i.width;
				TotalPercentCount++;
				break;
		}
	}

	TotalWidth-=EmptyColumns;
	int PanelTextWidth = m_Where.width() - 2;

	if (FullScreen)
		PanelTextWidth=ScrX-1;

	int ExtraWidth=PanelTextWidth-TotalWidth;

	if (TotalPercentCount>0)
	{
		const auto ExtraPercentWidth = (TotalPercentWidth > 100 || !ZeroLengthCount) ? ExtraWidth : ExtraWidth * TotalPercentWidth / 100;
		int TempWidth=0;

		for (auto& i: Columns)
		{
			if (!TotalPercentCount)
				break;

			if (i.width_type == col_width::percent)
			{
				int PercentWidth = (TotalPercentCount > 1)? (ExtraPercentWidth * i.width / TotalPercentWidth) : (ExtraPercentWidth - TempWidth);

				if (PercentWidth<1)
					PercentWidth=1;

				TempWidth+=PercentWidth;
				i.width = PercentWidth;
				i.width_type = col_width::fixed;
				TotalPercentCount--;
			}
		}
		ExtraWidth-=TempWidth;
	}

	for (auto& i: Columns)
	{
		if (!ZeroLengthCount)
			break;

		if (!i.width)
		{
			int AutoWidth=ExtraWidth/ZeroLengthCount;

			if (AutoWidth<1)
				AutoWidth=1;

			i.width = AutoWidth;
			ExtraWidth-=AutoWidth;
			ZeroLengthCount--;
		}
	}

	for (;;)
	{
		const auto LastColumn = static_cast<int>(Columns.size() - 1);
		TotalWidth=LastColumn-EmptyColumns;

		for (const auto& i: Columns)
		{
			if (i.width > 0)
				TotalWidth += i.width;
		}

		if (TotalWidth<=PanelTextWidth)
			break;

		if (Columns.size() <= 1)
		{
			Columns.front().width = PanelTextWidth;
			break;
		}
		else if (PanelTextWidth >= TotalWidth - Columns[LastColumn].width)
		{
			Columns[LastColumn].width = PanelTextWidth - (TotalWidth - Columns[LastColumn].width);
			break;
		}
		else
			Columns.pop_back();
	}
}


namespace
{
bool CanMakeStripes(const std::vector<column>& Columns, int StripeStride)
{
	if (Columns.size() % StripeStride != 0)
		return false;

	const auto FirstStripeBegin = Columns.cbegin();
	const auto FirstStripeEnd = FirstStripeBegin + StripeStride;

	for (auto Stripe = FirstStripeEnd; Stripe != Columns.cend(); Stripe += StripeStride)
	{
		if (!std::equal(FirstStripeBegin, FirstStripeEnd, Stripe,
			[](const column& a, const column& b) { return a.type == b.type; }))
		{
			return false;
		}
	}

	return true;
}
}


void FileList::PrepareStripes(const std::vector<column>& Columns)
{
	const auto ColumnsSize = static_cast<int>(Columns.size());

	for (const auto& StripeStride: irange(1, ColumnsSize / 2 + 1))
	{
		if (CanMakeStripes(Columns, StripeStride))
		{
			m_Stripes = ColumnsSize / StripeStride;
			m_ColumnsInStripe = StripeStride;
			return;
		}
	}

	m_Stripes = 1;
	m_ColumnsInStripe = ColumnsSize;
}


void FileList::HighlightBorder(int Level, int ListPos) const
{
	if (Level == m_ColumnsInStripe)
	{
		SetColor(COL_PANELBOX);
	}
	else
	{
		const auto FileColor = GetShowColor(ListPos, true);
		auto Color = colors::PaletteColorToFarColor(COL_PANELBOX);
		Color.BackgroundColor = FileColor.BackgroundColor;
		Color.SetBg4Bit(FileColor.IsBg4Bit());
		SetColor(Color);
	}
}

void FileList::ShowList(int ShowStatus,int StartColumn)
{
	int StatusShown=FALSE;
	int MaxLeftPos=0,MinLeftPos=FALSE;
	size_t ColumnCount=ShowStatus ? m_ViewSettings.StatusColumns.size() : m_ViewSettings.PanelColumns.size();
	const auto& Columns = ShowStatus ? m_ViewSettings.StatusColumns : m_ViewSettings.PanelColumns;

	for (int I = m_Where.top + 1 + Global->Opt->ShowColumnTitles, J = m_CurTopFile; I < m_Where.bottom - 2 * Global->Opt->ShowPanelStatus; I++, J++)
	{
		int CurColumn=StartColumn;

		if (ShowStatus)
		{
			SetColor(COL_PANELTEXT);
			GotoXY(m_Where.left + 1, m_Where.bottom - 1);
		}
		else
		{
			GotoXY(m_Where.left + 1, I);
		}

		int StatusLine=FALSE;
		int Level = 1;

		for (const auto& K: irange(ColumnCount))
		{
			int ListPos=J+CurColumn*m_Height;

			if (ShowStatus)
			{
				if (m_CurFile!=ListPos)
				{
					CurColumn++;
					continue;
				}
				else
					StatusLine=TRUE;
			}
			else
			{
				SetShowColor(ListPos);
			}

			int CurX=WhereX();
			int CurY=WhereY();
			int ShowDivider=TRUE;
			const auto ColumnType = Columns[K].type;
			int ColumnWidth=Columns[K].width;

			if (ColumnWidth<0)
			{
				if (!ShowStatus && K==ColumnCount-1)
				{
					SetColor(COL_PANELBOX);
					GotoXY(CurX-1,CurY);
					Text(CurX - 1 == m_Where.right? BoxSymbols[BS_V2] : L' ');
				}

				continue;
			}

			if (ListPos < static_cast<int>(m_ListData.size()))
			{
				if (!ShowStatus && !StatusShown && m_CurFile==ListPos && Global->Opt->ShowPanelStatus)
				{
					ShowList(TRUE,CurColumn);
					GotoXY(CurX,CurY);
					StatusShown=TRUE;
					SetShowColor(ListPos);
				}

				if (!ShowStatus)
					SetShowColor(ListPos);

				if (ColumnType >= column_type::custom_0 && ColumnType <= column_type::custom_max)
				{
					size_t ColumnNumber = static_cast<size_t>(ColumnType) - static_cast<size_t>(column_type::custom_0);
					const wchar_t *ColumnData = nullptr;

					if (ColumnNumber < m_ListData[ListPos].CustomColumns.size())
						ColumnData = m_ListData[ListPos].CustomColumns[ColumnNumber];

					if (!ColumnData)
					{
						const auto GetContentData = [&]
						{
							const auto& ContentMapPtr = m_ListData[ListPos].ContentData(this);
							if (!ContentMapPtr)
								return L"";
							const auto Iterator = ContentMapPtr->find(Columns[K].title);
							return Iterator != ContentMapPtr->cend()? Iterator->second.c_str() : L"";
						};

						ColumnData = GetContentData();
					}

					int CurLeftPos=0;

					if (!ShowStatus && LeftPos>0)
					{
						int Length = static_cast<int>(std::wcslen(ColumnData));
						if (Length>ColumnWidth)
						{
							CurLeftPos = std::min(LeftPos, Length-ColumnWidth);
							MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
						}
					}

					Text(fit_to_left(ColumnData + CurLeftPos, ColumnWidth));
				}
				else
				{
					switch (ColumnType)
					{
						case column_type::name:
						{
							int Width=ColumnWidth;
							const auto ViewFlags = Columns[K].type_flags;

							if ((ViewFlags & COLFLAGS_MARK) && Width>2)
							{
								const auto Mark = m_ListData[ListPos].Selected? L"√ "sv : ViewFlags & COLFLAGS_MARK_DYNAMIC ? L""sv : L"  "sv;
								Text(Mark);
								Width -= static_cast<int>(Mark.size());
							}

							if (Global->Opt->Highlight && m_ListData[ListPos].Colors && m_ListData[ListPos].Colors->Mark.Char && Width>1)
							{
								Width--;
								const auto OldColor = GetColor();
								if (!ShowStatus)
									SetShowColor(ListPos, false);

								Text(m_ListData[ListPos].Colors->Mark.Char);
								SetColor(OldColor);
							}

							string_view Name = m_ListData[ListPos].AlternateOrNormal(m_ShowShortNames);

							if (!(m_ListData[ListPos].Attributes & FILE_ATTRIBUTE_DIRECTORY) && (ViewFlags & COLFLAGS_NOEXTENSION))
							{
								Name = name_ext(Name).first;
							}

							const auto NameCopy = Name;

							if (ViewFlags & COLFLAGS_NAMEONLY)
							{
								//BUGBUG!!!
								// !!! НЕ УВЕРЕН, но то, что отображается пустое
								// пространство вместо названия - бага
								Name = PointToFolderNameIfFolder(Name);
							}

							int CurLeftPos=0;
							unsigned long long RightAlign=(ViewFlags & (COLFLAGS_RIGHTALIGN|COLFLAGS_RIGHTALIGNFORCE));
							int LeftBracket=FALSE,RightBracket=FALSE;

							if (!ShowStatus && LeftPos)
							{
								const auto Length = static_cast<int>(Name.size());

								if (Length>Width)
								{
									if (LeftPos>0)
									{
										if (!RightAlign)
										{
											CurLeftPos = std::min(LeftPos, Length-Width);
											MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
											Name.remove_prefix(CurLeftPos);
										}
									}
									else if (RightAlign)
									{
										int CurRightPos=LeftPos;

										if (Length+CurRightPos<Width)
										{
											CurRightPos=Width-Length;
										}
										else
										{
											RightBracket=TRUE;
											LeftBracket=(ViewFlags & COLFLAGS_RIGHTALIGNFORCE)==COLFLAGS_RIGHTALIGNFORCE;
										}

										Name.remove_prefix(Length + CurRightPos - Width);
										RightAlign=FALSE;

										MinLeftPos = std::min(MinLeftPos, CurRightPos);
									}
								}
							}

							string strName;
							int TooLong=ConvertName(Name, strName, Width, RightAlign,ShowStatus,m_ListData[ListPos].Attributes);

							if (CurLeftPos)
								LeftBracket=TRUE;

							if (TooLong)
							{
								if (RightAlign)
									LeftBracket=TRUE;

								if (!RightAlign && static_cast<int>(Name.size()) > Width)
									RightBracket=TRUE;
							}

							if (!ShowStatus)
							{
								if (m_ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE)
									if (!(m_ListData[ListPos].Attributes & FILE_ATTRIBUTE_DIRECTORY) && !IsCaseMixed(NameCopy))
										inplace::lower(strName);

								if ((m_ViewSettings.Flags&PVS_FOLDERUPPERCASE) && (m_ListData[ListPos].Attributes & FILE_ATTRIBUTE_DIRECTORY))
									inplace::upper(strName);

								if ((m_ViewSettings.Flags&PVS_FILELOWERCASE) && !(m_ListData[ListPos].Attributes & FILE_ATTRIBUTE_DIRECTORY))
									inplace::lower(strName);
							}

							Text(strName);


							if (!ShowStatus)
							{
								int NameX=WhereX();

								if (LeftBracket)
								{
									GotoXY(CurX-1,CurY);

									if (Level == 1)
										SetColor(COL_PANELBOX);

									Text(openBracket);
									SetShowColor(J);
								}

								if (RightBracket)
								{
									HighlightBorder(Level, ListPos);
									GotoXY(NameX,CurY);
									Text(closeBracket);
									ShowDivider=FALSE;

									if (Level == m_ColumnsInStripe)
										SetColor(COL_PANELTEXT);
									else
										SetShowColor(J);
								}
							}
						}
						break;
						case column_type::extension:
						{
							string_view ExtPtr;
							if (!(m_ListData[ListPos].Attributes & FILE_ATTRIBUTE_DIRECTORY))
							{
								const auto& Name = m_ListData[ListPos].AlternateOrNormal(m_ShowShortNames);
								ExtPtr = name_ext(Name).second;
							}
							if (!ExtPtr.empty())
								ExtPtr.remove_prefix(1);

							const auto ViewFlags = Columns[K].type_flags;
							Text((ViewFlags & COLFLAGS_RIGHTALIGN? fit_to_right : fit_to_left)(string(ExtPtr), ColumnWidth));

							if (!ShowStatus && static_cast<int>(ExtPtr.size()) > ColumnWidth)
							{
								int NameX=WhereX();

								HighlightBorder(Level, ListPos);

								GotoXY(NameX,CurY);
								Text(closeBracket);
								ShowDivider=FALSE;

								if (Level == m_ColumnsInStripe)
									SetColor(COL_PANELTEXT);
								else
									SetShowColor(J);
							}

							break;
						}

						case column_type::size:
						case column_type::size_compressed:
						case column_type::streams_size:
						{
							const auto SizeToDisplay = (ColumnType == column_type::size_compressed)
								? m_ListData[ListPos].AllocationSize
								: (ColumnType == column_type::streams_size)
									? m_ListData[ListPos].StreamsSize(this)
									: m_ListData[ListPos].FileSize;

							Text(FormatStr_Size(
								SizeToDisplay,
								m_ListData[ListPos].FileName,
								m_ListData[ListPos].Attributes,
								m_ListData[ListPos].ShowFolderSize,
								m_ListData[ListPos].ReparseTag,
								ColumnType,
								Columns[K].type_flags,
								ColumnWidth,
								m_CurDir));
							break;
						}

						case column_type::date:
						case column_type::time:
						case column_type::date_write:
						case column_type::date_creation:
						case column_type::date_access:
						case column_type::date_change:
						{
							os::chrono::time_point const FileListItem::* FileTime;

							switch (ColumnType)
							{
							case column_type::date_creation:
								FileTime = &FileListItem::CreationTime;
								break;

							case column_type::date_access:
								FileTime = &FileListItem::LastAccessTime;
								break;

							case column_type::date_change:
								FileTime = &FileListItem::ChangeTime;
								break;

							default:
								FileTime = &FileListItem::LastWriteTime;
								break;
							}

							Text(FormatStr_DateTime(std::invoke(FileTime, m_ListData[ListPos]), ColumnType, Columns[K].type_flags, ColumnWidth));
							break;
						}

						case column_type::attributes:
						{
							Text(FormatStr_Attribute(m_ListData[ListPos].Attributes,ColumnWidth));
							break;
						}

						case column_type::description:
						{
							int CurLeftPos=0;

							if (!ShowStatus && LeftPos>0)
							{
								int Length = static_cast<int>(std::wcslen(NullToEmpty(m_ListData[ListPos].DizText)));
								if (Length>ColumnWidth)
								{
									CurLeftPos = std::min(LeftPos, Length-ColumnWidth);
									MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
								}
							}

							auto DizText = m_ListData[ListPos].DizText? m_ListData[ListPos].DizText + CurLeftPos : L""sv;
							const auto pos = DizText.find(L'\4');
							if (pos != string::npos)
								DizText.remove_suffix(DizText.size() - pos);

							Text(fit_to_left(string(DizText), ColumnWidth));
							break;
						}

						case column_type::owner:
						{
							const auto& Owner = m_ListData[ListPos].Owner(this);
							size_t Offset = 0;

							if (!(Columns[K].type_flags & COLFLAGS_FULLOWNER) && m_PanelMode != panel_mode::PLUGIN_PANEL)
							{
								const auto SlashPos = FindSlash(Owner);
								if (SlashPos != string::npos)
								{
									Offset = SlashPos + 1;
								}
							}
							else if(!Owner.empty() && path::is_separator(Owner.front()))
							{
								Offset = 1;
							}

							int CurLeftPos=0;

							if (!ShowStatus && LeftPos>0)
							{
								int Length = static_cast<int>(Owner.size() - Offset);
								if (Length>ColumnWidth)
								{
									CurLeftPos = std::min(LeftPos, Length-ColumnWidth);
									MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
								}
							}

							Text(fit_to_left(Owner.substr(Offset + CurLeftPos), ColumnWidth));
							break;
						}

						case column_type::links_number:
						{
							const auto Value = m_ListData[ListPos].NumberOfLinks(this);
							Text(fit_to_right(Value == FileListItem::values::unknown(Value)? L"?"s : str(Value), ColumnWidth));
							break;
						}

						case column_type::streams_number:
						{
							const auto Value = m_ListData[ListPos].NumberOfStreams(this);
							Text(fit_to_right(Value == FileListItem::values::unknown(Value)? L"?"s : str(Value), ColumnWidth));
							break;
						}

						default:
							break;
					}
				}
			}
			else
			{
				Text(string(ColumnWidth, L' '));
			}

			if (ShowDivider==FALSE)
				GotoXY(CurX+ColumnWidth+1,CurY);
			else
			{
				if (!ShowStatus)
				{
					HighlightBorder(Level, ListPos);
				}

				if (K == ColumnCount-1)
					SetColor(COL_PANELBOX);

				GotoXY(CurX+ColumnWidth,CurY);

				if (K==ColumnCount-1)
					Text(CurX + ColumnWidth == m_Where.right? BoxSymbols[BS_V2] : L' ');
				else
					Text(ShowStatus? L' ' : BoxSymbols[Level == m_ColumnsInStripe? BS_V2 : BS_V1]);

				if (!ShowStatus)
					SetColor(COL_PANELTEXT);
			}

			if (!ShowStatus)
			{
				if (Level == m_ColumnsInStripe)
				{
					Level = 0;
					CurColumn++;
				}

				Level++;
			}
		}

		if ((!ShowStatus || StatusLine) && WhereX() < m_Where.right)
		{
			SetColor(COL_PANELTEXT);
			Text(string(m_Where.right - WhereX(), L' '));
		}
	}

	if (!ShowStatus && !StatusShown && Global->Opt->ShowPanelStatus)
	{
		SetScreen({ m_Where.left + 1, m_Where.bottom - 1, m_Where.right - 1, m_Where.bottom - 1 }, L' ', colors::PaletteColorToFarColor(COL_PANELTEXT));
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y2-1);
		//Text(string(X2 - X1 - 1, L' '));
	}

	if (!ShowStatus)
	{
		if (LeftPos<0)
			LeftPos=MinLeftPos;

		if (LeftPos>0)
			LeftPos=MaxLeftPos;
	}
}

bool FileList::IsModeFullScreen(int Mode)
{
	return (Global->Opt->ViewSettings[Mode].Flags & PVS_FULLSCREEN) != 0;
}


bool FileList::IsDizDisplayed() const
{
	return IsColumnDisplayed(column_type::description);
}

bool FileList::IsColumnDisplayed(function_ref<bool(const column&)> const Compare) const
{
	return
		std::any_of(ALL_CONST_RANGE(m_ViewSettings.PanelColumns), Compare) ||
		std::any_of(ALL_CONST_RANGE(m_ViewSettings.StatusColumns), Compare);
}

bool FileList::IsColumnDisplayed(column_type Type) const
{
	return IsColumnDisplayed([&Type](const column& i) { return i.type == Type; });
}

int FileList::GetColumnsCount() const
{
	return m_Stripes;
}

bool FileList::GetSelectedFirstMode() const
{
	return SelectedFirst;
}

std::unique_ptr<content_data> FileList::GetContentData(const string& Item) const
{
	if (m_ContentPlugins.empty())
		return {};

	auto Result = std::make_unique<content_data>();
	Global->CtrlObject->Plugins->GetContentData(m_ContentPlugins, Item, m_ContentNamesPtrs, m_ContentValues, *Result.get());

	return Result;
}

void FileList::MoveSelection(direction Direction)
{
	if (m_ListData.empty())
		return;

	assert(m_CurFile < static_cast<int>(m_ListData.size()));
	const auto CurPtr = &m_ListData[m_CurFile];

	if (ShiftSelection==-1)
	{
		// .. is never selected
		if (m_CurFile < static_cast<int>(m_ListData.size() - 1) && IsParentDirectory(*CurPtr))
			ShiftSelection = !m_ListData[m_CurFile+1].Selected;
		else
			ShiftSelection=!CurPtr->Selected;
	}

	Select(*CurPtr, ShiftSelection != 0);

	if (Direction == up)
		MoveCursor(-1);
	else
		MoveCursor(1);

	if (SelectedFirst && !InternalProcessKey)
		SortFileList(true);
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("fat_time")
{
	using namespace std::chrono_literals;

	static const struct
	{
		os::chrono::duration First, Second;
		int Result;
	}
	Tests[]
	{
		{ 0s,          0s,    0, },
		{ 0s + 1ms,    0s,    0, },
		{ 0s + 1ms,    2s,    0, },
		{ 1s - 1ms,    0s,    0, },
		{ 1s - 1ms,    2s,    0, },
		{ 1s,          2s,    0, },
		{ 1s + 1ms,    1s,    0, },
		{ 1s + 1ms,    2s,    0, },
		{ 2s - 1ms,    1s,    0, },
		{ 2s - 1ms,    2s,    0, },
		{ 2s,          2s,    0, },
		{ 2s + 1ms,    2s,    0, },
		{ 2s + 1ms,    4s,    0, },
		{ 3s - 1ms,    3s,    0, },
		{ 3s - 1ms,    4s,    0, },
		{ 3s,          4s,    0, },
		{ 3s + 1ms,    3s,    0, },
		{ 3s + 1ms,    4s,    0, },
		{ 4s - 1ms,    3s,    0, },
		{ 4s - 1ms,    4s,    0, },
		{ 4s,          4s,    0, },
		{ 4s + 1ms,    4s,    0, },
		{ 4s + 1ms,    6s,    0, },
		{ 0s,          2s,   -1, },
		{ 2s,          4s,   -1, },
		{ 2s,          0s,    1, },
		{ 4s,          2s,    1, },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(compare_fat_write_time(os::chrono::time_point(i.First), os::chrono::time_point(i.Second)) == i.Result);
	}
}
#endif
