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
#include "cddrv.hpp"
#include "syslog.hpp"
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
#include "FarGuid.hpp"
#include "DlgGuid.hpp"
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

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/enum_tokens.hpp"
#include "common/rel_ops.hpp"
#include "common/scope_exit.hpp"
#include "common/string_utils.hpp"
#include "common/utility.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static auto compare_time(os::chrono::time_point First, os::chrono::time_point Second)
{
	return as_signed((First - Second).count());
}

static auto compare_fat_time(os::chrono::time_point First, os::chrono::time_point Second)
{
	constexpr auto FatPrecision = std::chrono::duration_cast<os::chrono::duration>(2s).count();

	const auto Difference = compare_time(First, Second);

	return as_unsigned(std::abs(Difference)) < FatPrecision? 0 : Difference;
}


enum SELECT_MODES
{
	SELECT_INVERT,
	SELECT_INVERTALL,
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

	pi.CustomColumnData=fi.CustomColumnData;
	pi.CustomColumnNumber=fi.CustomColumnNumber;
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

	if (Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Luamacro.Id, OPEN_LUAMACRO, &info, &ptr) && ptr)
	{
		indicator[0] = info.Ret.Values[0].String[0];
		indicator[1] = info.Ret.Values[0].String[1];
		return true;
	}
	return false;
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

	PluginsListItem(std::unique_ptr<plugin_panel>&& hPlugin, string HostFile, int Modified, int PrevViewMode, panel_sort PrevSortMode, bool PrevSortOrder, bool PrevDirectoriesFirst, const PanelViewSettings& PrevViewSettings):
		m_Plugin(std::move(hPlugin)),
		m_HostFile(std::move(HostFile)),
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
	int m_Modified;
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
	if (!IsNumberOfLinksRead())
	{
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
	if (!IsNumberOfStreamsRead())
	{
		GetStreamsCountAndSize(Owner, *this, m_StreamsSize, m_NumberOfStreams, Owner->StreamsSupported());
	}
	return m_NumberOfStreams;
}

bool FileListItem::IsStreamsSizeRead() const
{
	return m_StreamsSize != values::uninitialised(m_StreamsSize);
}

unsigned long long FileListItem::StreamsSize(const FileList* Owner) const
{
	if (!IsStreamsSizeRead())
	{
		GetStreamsCountAndSize(Owner, *this, m_StreamsSize, m_NumberOfStreams, Owner->StreamsSupported());
	}
	return m_StreamsSize;
}

bool FileListItem::IsOwnerRead() const
{
	return !(m_Owner.size() == 1 && m_Owner.front() == values::uninitialised(wchar_t()));
}

const string& FileListItem::Owner(const FileList* Owner) const
{
	if (!IsOwnerRead())
	{
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
	}
	return m_Owner;
}

bool FileListItem::IsContentDataRead() const
{
	return m_ContentData != nullptr; // bad
}

const content_data_ptr& FileListItem::ContentData(const FileList* Owner) const
{
	if (!IsContentDataRead())
	{
		m_ContentData = Owner->GetContentData(GetItemFullName(*this, Owner));
	}
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
	Panel(std::move(Owner))
{
	_OT(SysLog(L"[%p] FileList::FileList()", this));
	{
		const auto& data = msg(lng::MPanelBracketsForLongName);

		if (data.size() > 1)
		{
			*openBracket=data[0];
			*closeBracket=data[1];
		}
		else
		{
			*openBracket=L'{';
			*closeBracket=L'}';
		}

		openBracket[1]=closeBracket[1]=0;
	}

	m_CurDir = os::fs::GetCurrentDirectory();
	strOriginalCurDir = m_CurDir;
	m_SortMode = panel_sort::BY_NAME;
	m_ViewSettings = Global->Opt->ViewSettings[m_ViewMode].clone();
	PreparePanelView();
}


FileList::~FileList()
{
	_OT(SysLog(L"[%p] FileList::~FileList()", this));

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		while (PopPlugin(FALSE))
			;

	FileList::StopFSWatcher();

	FileList::ClearAllItem();

	m_ListData.clear();
}


void FileList::list_data::clear()
{
	for (const auto& i: Items)
	{
		if (m_Plugin)
		{
			FreePluginPanelItemUserData(m_Plugin, i.UserData);
			if (i.DeleteDiz)
				delete[] i.DizText;
		}

		DeleteRawArray(span(i.CustomColumnData, i.CustomColumnNumber));
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
	explicit list_less(const FileList* Owner, const plugin_panel* SortPlugin):
		m_Owner(Owner),
		ListSortMode(Owner->GetSortMode()),
		RevertSorting(Owner->GetSortOrder()),
		ListPanelMode(Owner->GetMode()),
		hSortPlugin(SortPlugin),
		ListSortGroups(Owner->GetSortGroups()),
		ListSelectedFirst(Owner->GetSelectedFirstMode()),
		ListDirectoriesFirst(Owner->GetDirectoriesFirst()),
		SortFolderExt(Global->Opt->SortFolderExt)
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

		if (ListDirectoriesFirst)
		{
			const auto IsDirItem1 = (Item1.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			const auto IsDirItem2 = (Item2.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

			if (IsDirItem1 != IsDirItem2)
				return IsDirItem1 > IsDirItem2;
		}

		if (ListSelectedFirst && Item1.Selected != Item2.Selected)
			return Item1.Selected > Item2.Selected;

		if (ListSortGroups &&
			(ListSortMode == panel_sort::BY_NAME || ListSortMode == panel_sort::BY_EXT || ListSortMode == panel_sort::BY_FULLNAME) &&
			Item1.SortGroup != Item2.SortGroup
		)
			return Item1.SortGroup < Item2.SortGroup;

		// Reverse sorting is taken into account from this point
		const auto& a = RevertSorting? Item2 : Item1;
		const auto& b = RevertSorting? Item1 : Item2;

		bool UseReverseNameSort = false;
		string_view Ext1, Ext2;

		if (ListSortMode == panel_sort::UNSORTED)
		{
			return a.Position < b.Position;
		}

		if (hSortPlugin)
		{
			PluginPanelItemHolder pi1, pi2;
			m_Owner->FileListToPluginItem(a, pi1);
			m_Owner->FileListToPluginItem(b, pi2);
			if (const auto Result = Global->CtrlObject->Plugins->Compare(hSortPlugin, &pi1.Item, &pi2.Item, static_cast<int>(ListSortMode) + (SM_UNSORTED - static_cast<int>(panel_sort::UNSORTED))))
			{
				if (Result != -2)
					return Result < 0;
			}
		}

		const auto CompareTime = [&a, &b](const os::chrono::time_point FileListItem::*time)
		{
			return compare_time(std::invoke(time, a), std::invoke(time, b));
		};

		const auto GetExt = [SortFolderExt = SortFolderExt](const FileListItem& i)
		{
			if ((i.Attributes & FILE_ATTRIBUTE_DIRECTORY) && !SortFolderExt)
				return string_view(i.FileName).substr(i.FileName.size());

			return PointToExt(i.FileName);
		};

		switch (ListSortMode)
		{
		case panel_sort::UNSORTED:
			break;

		case panel_sort::BY_NAME:
			UseReverseNameSort = true;
			break;

		case panel_sort::BY_EXT:
			UseReverseNameSort = true;

			Ext1 = GetExt(a);
			Ext2 = GetExt(b);

			if (Ext1.empty())
			{
				if (Ext2.empty())
					break;
				else
					return true;
			}
			if (Ext2.empty())
				return false;

			if (const auto Result = string_sort::compare(Ext1, Ext2))
				return Result < 0;

			break;

		case panel_sort::BY_MTIME:
			if (const auto Result = CompareTime(&FileListItem::LastWriteTime))
				return Result < 0;
			break;

		case panel_sort::BY_CTIME:
			if (const auto Result = CompareTime(&FileListItem::CreationTime))
				return Result < 0;
			break;

		case panel_sort::BY_ATIME:
			if (const auto Result = CompareTime(&FileListItem::LastAccessTime))
				return Result < 0;
			break;

		case panel_sort::BY_CHTIME:
			if (const auto Result = CompareTime(&FileListItem::ChangeTime))
				return Result < 0;
			break;

		case panel_sort::BY_SIZE:
			if (a.FileSize != b.FileSize)
				return a.FileSize < b.FileSize;
			break;

		case panel_sort::BY_DIZ:
			if (!a.DizText)
			{
				if (!b.DizText)
					break;
				else
					return false;
			}

			if (!b.DizText)
				return true;

			if (const auto Result = string_sort::compare(a.DizText, b.DizText))
				return Result < 0;
			break;

		case panel_sort::BY_OWNER:
			{
				if (const auto Result = string_sort::compare(a.Owner(m_Owner), b.Owner(m_Owner)))
					return Result < 0;
			}
			break;

		case panel_sort::BY_COMPRESSEDSIZE:
			if (a.AllocationSize != b.AllocationSize)
				return a.AllocationSize < b.AllocationSize;
			break;

		case panel_sort::BY_NUMLINKS:
			{
				const auto aValue = a.NumberOfLinks(m_Owner);
				const auto bValue = b.NumberOfLinks(m_Owner);
				if (aValue != bValue)
					return aValue < bValue;
			}
			break;

		case panel_sort::BY_NUMSTREAMS:
			{
				const auto aValue = a.NumberOfStreams(m_Owner);
				const auto bValue = b.NumberOfStreams(m_Owner);
				if (aValue != bValue)
					return aValue < bValue;
			}
			break;

		case panel_sort::BY_STREAMSSIZE:
			{
				const auto aValue = a.StreamsSize(m_Owner);
				const auto bValue = b.StreamsSize(m_Owner);
				if (aValue != bValue)
					return aValue < bValue;
			}
			break;

		case panel_sort::BY_FULLNAME:
			UseReverseNameSort = true;
			if (const auto Result = string_sort::compare(a.FileName, b.FileName))
				return Result < 0;
			break;

		case panel_sort::COUNT:
			// this case makes no sense - just to suppress the warning
			break;
		}

		if (Ext1.empty())
			Ext1 = GetExt(a);

		if (Ext2.empty())
			Ext2 = GetExt(b);

		const auto GetNameOnly = [](string_view const FullName, string_view const Ext)
		{
			return PointToName(FullName.substr(0, FullName.size() - Ext.size()));
		};

		int NameCmp = string_sort::compare(GetNameOnly(a.FileName, Ext1), GetNameOnly(b.FileName, Ext2));

		if (!NameCmp)
		{
			NameCmp = string_sort::compare(Ext1, Ext2);
		}

		if (NameCmp)
		{
			if (RevertSorting && !UseReverseNameSort)
			{
				// We have swapped references earlier for convenience.
				// However, we don't want to reverse fallback sorting by name, so it's time to "undo" that:
				return NameCmp > 0;
			}
			return NameCmp < 0;
		}
		else
		{
			return a.Position < b.Position;
		}
	}

private:
	const FileList* const m_Owner;
	const panel_sort ListSortMode;
	const bool RevertSorting;
	const panel_mode ListPanelMode;
	const plugin_panel* hSortPlugin;
	bool ListSortGroups;
	bool ListSelectedFirst;
	bool ListDirectoriesFirst;
	bool SortFolderExt;
};


void FileList::SortFileList(bool KeepPosition)
{
	if (!m_ListData.empty() && !m_InsideGetFindData)
	{
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
			std::sort(ALL_RANGE(m_ListData), list_less(this, hSortPlugin));
		}
		else
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

		if (KeepPosition)
			GoToFile(strCurName);
	}
}

bool FileList::SendKeyToPlugin(DWORD Key, bool Pred)
{
	_ALGO(CleverSysLog clv(L"FileList::SendKeyToPlugin()"));
	_ALGO(SysLog(L"Key=%s Pred=%d",_FARKEY_ToName(Key),Pred));

	if (m_PanelMode != panel_mode::PLUGIN_PANEL)
		return false;

	const auto MacroState = Global->CtrlObject->Macro.GetState();
	if (MacroState != MACROSTATE_RECORDING_COMMON && MacroState != MACROSTATE_EXECUTING_COMMON && MacroState != MACROSTATE_NOMACRO)
		return false;

	_ALGO(SysLog(L"call Plugins.ProcessKey() {"));
	INPUT_RECORD rec;
	KeyToInputRecord(Key, &rec);
	const auto ProcessCode = Global->CtrlObject->Plugins->ProcessKey(GetPluginHandle(), &rec, Pred);
	_ALGO(SysLog(L"} ProcessCode=%d", ProcessCode));

	return ProcessCode != 0;
}

bool FileList::GetPluginInfo(PluginInfo *PInfo) const
{
	const auto PluginPanel = GetPluginHandle();
	if (GetMode() == panel_mode::PLUGIN_PANEL && PluginPanel && PluginPanel->plugin())
	{
		PInfo->StructSize=sizeof(PluginInfo);
		return Global->CtrlObject->Plugins->GetPluginInfo(PluginPanel->plugin(), PInfo);
	}
	return false;
}

long long FileList::VMProcess(int OpCode,void *vParam,long long iParam)
{
	switch (OpCode)
	{
		case MCODE_C_ROOTFOLDER:
		{
			if (m_PanelMode == panel_mode::PLUGIN_PANEL)
			{
				Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);
				return !m_CachedOpenPanelInfo.CurDir || !*m_CachedOpenPanelInfo.CurDir;
			}
			else
			{
				return IsRootPath(m_CurDir)? 1 : equal_icase(m_CurDir, GetPathRoot(m_CurDir));
			}
		}
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
			// vParam = MacroPanelSelect*, iParam = 0
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

			// mps->ActionFlags
			switch (mps->Action)
			{
				case 0:  // снять выделение
				{
					switch(mps->Mode)
					{
						case 0: // снять со всего?
							SaveSelection();
							Result=GetRealSelCount();
							ClearSelection();
							break;

						case 1: // по индексу?
							SaveSelection();
							Result=1;
							Select(m_ListData[mps->Index], false);
							break;

						case 2: // набор строк
							SaveSelection();
							Result=0;
							ApplyToList([&](size_t Pos){ Select(m_ListData[Pos], false); });
							break;

						case 3: // масками файлов, разделенных запятыми
							SaveSelection();
							Result = SelectFiles(SELECT_REMOVEMASK, mps->Item);
							break;
					}
					break;
				}

				case 1:  // добавить выделение
				{
					switch(mps->Mode)
					{
						case 0: // выделить все?
							SaveSelection();

							for (auto& i: m_ListData)
							{
								Select(i, true);
							}

							Result=GetRealSelCount();
							break;

						case 1: // по индексу?
							SaveSelection();
							Result=1;
							Select(m_ListData[mps->Index], true);
							break;

						case 2: // набор строк через CRLF
							SaveSelection();
							Result=0;
							ApplyToList([&](size_t Pos) { Select(m_ListData[Pos], true); });
							break;

						case 3: // масками файлов, разделенных запятыми
							SaveSelection();
							Result = SelectFiles(SELECT_ADDMASK, mps->Item);
							break;
					}
					break;
				}

				case 2:  // инвертировать выделение
				{
					switch(mps->Mode)
					{
						case 0: // инвертировать все?
							SaveSelection();

							for (auto& i: m_ListData)
							{
								Select(i, !i.Selected);
							}

							Result=GetRealSelCount();
							break;

						case 1: // по индексу?
							SaveSelection();
							Result=1;
							Select(m_ListData[mps->Index], !m_ListData[mps->Index].Selected);
							break;

						case 2: // набор строк через CRLF
							SaveSelection();
							Result=0;
							ApplyToList([&](size_t Pos) { Select(m_ListData[Pos], !m_ListData[Pos].Selected); });
							break;

						case 3: // масками файлов, разделенных запятыми
							SaveSelection();
							Result = SelectFiles(SELECT_INVERTMASK, mps->Item);
							break;
					}
					break;
				}

				case 3:  // восстановить выделение
				{
					RestoreSelection();
					Result=GetRealSelCount();
					break;
				}
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
	static auto get(const string& Filename)
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

bool FileList::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key();
	elevation::instance().ResetApprove();

	int N;
	const auto IsEmptyCmdline = Parent()->GetCmdLine()->GetString().empty();

	if (IsVisible())
	{
		if (!InternalProcessKey)
			if ((!(LocalKey == KEY_ENTER || LocalKey == KEY_NUMENTER) && !(LocalKey == KEY_SHIFTENTER || LocalKey == KEY_SHIFTNUMENTER)) || IsEmptyCmdline)
				if (SendKeyToPlugin(LocalKey))
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
		if (LocalKey == KEY_SHIFTLEFT || LocalKey == KEY_SHIFTNUMPAD4)
			LocalKey=KEY_SHIFTPGUP;
		else if (LocalKey == KEY_SHIFTRIGHT || LocalKey == KEY_SHIFTNUMPAD6)
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
				_ALGO(CleverSysLog clv(L"F1"));
				_ALGO(SysLog(L"%s, FileCount=%d", (m_PanelMode == PLUGIN_PANEL?"PluginPanel":"FilePanel"), FileCount));
				return m_PanelMode == panel_mode::PLUGIN_PANEL && PluginPanelHelp(GetPluginHandle());
			}

		case KEY_ALTSHIFTF9:
		case KEY_RALTSHIFTF9:
			if (m_PanelMode == panel_mode::PLUGIN_PANEL)
				Global->CtrlObject->Plugins->ConfigureCurrent(GetPluginHandle()->plugin(), FarGuid);
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

		case KEY_ALTLEFT:     // Прокрутка длинных имен и описаний
		case KEY_RALTLEFT:
		case KEY_ALTHOME:     // Прокрутка длинных имен и описаний - в начало
		case KEY_RALTHOME:
			LeftPos=(LocalKey == KEY_ALTHOME || LocalKey == KEY_RALTHOME)?-0x7fff:LeftPos-1;
			Redraw();
			return true;

		case KEY_ALTRIGHT:    // Прокрутка длинных имен и описаний
		case KEY_RALTRIGHT:
		case KEY_ALTEND:     // Прокрутка длинных имен и описаний - в конец
		case KEY_RALTEND:
			LeftPos=(LocalKey == KEY_ALTEND || LocalKey == KEY_RALTEND)?0x7fff:LeftPos+1;
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
					LocalKey == KEY_CTRLALTINS || LocalKey == KEY_RCTRLRALTINS || LocalKey == KEY_CTRLRALTINS || LocalKey == KEY_RCTRLALTINS ||
					LocalKey == KEY_ALTSHIFTINS || LocalKey == KEY_RALTSHIFTINS ||
					LocalKey == KEY_CTRLALTNUMPAD0 || LocalKey == KEY_RCTRLRALTNUMPAD0 || LocalKey == KEY_CTRLRALTNUMPAD0 || LocalKey == KEY_RCTRLALTNUMPAD0 ||
					LocalKey == KEY_ALTSHIFTNUMPAD0 || LocalKey == KEY_RALTSHIFTNUMPAD0,
				(LocalKey&(KEY_CTRL|KEY_ALT))==(KEY_CTRL|KEY_ALT) || (LocalKey&(KEY_RCTRL|KEY_RALT))==(KEY_RCTRL|KEY_RALT)
			);
			return true;

		case KEY_CTRLC: // hdrop  copy
		case KEY_RCTRLC:
			CopyFiles();
			return true;

#if 0
		case KEY_CTRLX: // hdrop cut !!!NEED KEY!!!
		case KEY_RCTRLX:
			CopyFiles(true);
			return true;
#endif
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

				if (LocalKey==KEY_CTRLSHIFTENTER || LocalKey==KEY_RCTRLSHIFTENTER || LocalKey==KEY_CTRLSHIFTNUMENTER || LocalKey==KEY_RCTRLSHIFTNUMENTER)
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

						if (!(LocalKey==KEY_CTRLALTF || LocalKey==KEY_RCTRLRALTF || LocalKey==KEY_CTRLRALTF || LocalKey==KEY_RCTRLALTF))
							LocalKey = KEY_CTRLF;
					}

					if (LocalKey==KEY_CTRLF || LocalKey==KEY_RCTRLF || LocalKey==KEY_CTRLALTF || LocalKey==KEY_RCTRLRALTF || LocalKey==KEY_CTRLRALTF || LocalKey==KEY_RCTRLALTF)
					{
						if (m_PanelMode != panel_mode::PLUGIN_PANEL)
							strFileName = CreateFullPathName(
								strFileName,
								(Current.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0,
								LocalKey == KEY_CTRLALTF || LocalKey == KEY_RCTRLRALTF || LocalKey == KEY_CTRLRALTF || LocalKey == KEY_RCTRLALTF
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
					if (m_PanelMode == panel_mode::PLUGIN_PANEL && Global->Opt->SubstPluginPrefix && !(LocalKey == KEY_CTRLENTER || LocalKey == KEY_RCTRLENTER || LocalKey == KEY_CTRLNUMENTER || LocalKey == KEY_RCTRLNUMENTER || LocalKey == KEY_CTRLJ || LocalKey == KEY_RCTRLJ))
					{
						strFileName.insert(0, GetPluginPrefix());
					}

					if (!strFileName.empty() && (Global->Opt->QuotedName&QUOTEDNAME_INSERT) != 0)
						QuoteSpace(strFileName);

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
			_ALGO(CleverSysLog clv(L"Ctrl-A"));

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
			_ALGO(CleverSysLog clv(L"Ctrl-G"));

			if (m_PanelMode != panel_mode::PLUGIN_PANEL ||
			        Global->CtrlObject->Plugins->UseFarCommand(GetPluginHandle(), PLUGIN_FAROTHER))
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
			_ALGO(CleverSysLog clv(L"Enter/Shift-Enter"));
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(m_PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(LocalKey)));

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
			_ALGO(CleverSysLog clv(L"Ctrl-\\"));
			_ALGO(SysLog(L"%s, FileCount=%d",(m_PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));
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
			_ALGO(CleverSysLog clv(L"Shift-F1"));
			_ALGO(SysLog(L"%s, FileCount=%d",(m_PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

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
			_ALGO(CleverSysLog clv(L"Shift-F2"));
			_ALGO(SysLog(L"%s, FileCount=%d",(m_PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

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
			_ALGO(CleverSysLog clv(L"Shift-F3"));
			_ALGO(SysLog(L"%s, FileCount=%d",(m_PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));
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
			_ALGO(CleverSysLog clv(L"Edit/View"));
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(m_PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(LocalKey)));

			if (m_PanelMode == panel_mode::PLUGIN_PANEL)
				Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(),&m_CachedOpenPanelInfo);

			if (LocalKey == KEY_NUMPAD5 || LocalKey == KEY_SHIFTNUMPAD5)
				LocalKey=KEY_F3;

			if ((LocalKey==KEY_SHIFTF4 || !m_ListData.empty()) && SetCurPath())
			{
				string strPluginData;
				bool PluginMode =
					m_PanelMode == panel_mode::PLUGIN_PANEL &&
					!Global->CtrlObject->Plugins->UseFarCommand(GetPluginHandle(), PLUGIN_FARGETFILE) &&
					!(m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES);

				if (PluginMode)
				{
					const string strHostFile = NullToEmpty(m_CachedOpenPanelInfo.HostFile);
					const string strInfoCurDir = NullToEmpty(m_CachedOpenPanelInfo.CurDir);
					strPluginData = L'<' + strHostFile + L':' + strInfoCurDir + L'>';
				}

				uintptr_t codepage = CP_DEFAULT;
				const auto Edit = (LocalKey == KEY_F4 || LocalKey == KEY_ALTF4 || LocalKey == KEY_RALTF4 || LocalKey == KEY_SHIFTF4 || LocalKey == KEY_CTRLSHIFTF4 || LocalKey == KEY_RCTRLSHIFTF4);
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
											L"WarnEditorPath"sv) != Message::first_button)
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
								L"WarnEditorPluginName"sv) != Message::first_button)
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
						PluginPanelItemHolder PanelItem;
						FileListToPluginItem(*CurPtr, PanelItem);

						if (!Global->CtrlObject->Plugins->GetFile(GetPluginHandle(), &PanelItem.Item, TemporaryDirectory, strFileName, OPM_SILENT | (Edit? OPM_EDIT : OPM_VIEW)))
						{
							// BUGBUG check result
							(void)os::fs::remove_directory(TemporaryDirectory);
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
					std::optional<os::fs::current_directory_guard> Guard;

					// We have to set it - users can have associations like !.! which will work funny without this
					if (PluginMode)
						Guard.emplace(TemporaryDirectory);

					if (Edit)
					{
						const auto EnableExternal = (((LocalKey == KEY_F4 || LocalKey == KEY_SHIFTF4) && Global->Opt->EdOpt.UseExternalEditor) ||
							((LocalKey == KEY_ALTF4 || LocalKey == KEY_RALTF4) && !Global->Opt->EdOpt.UseExternalEditor)) && !Global->Opt->strExternalEditor.empty();
						auto Processed = false;

						const auto SavedState = file_state::get(strFileName);
						if (LocalKey == KEY_ALTF4 || LocalKey == KEY_RALTF4 || LocalKey == KEY_F4)
						{
							if (ProcessLocalFileTypes(strFileName, strShortFileName, (LocalKey == KEY_F4)?FILETYPE_EDIT:FILETYPE_ALTEDIT, PluginMode))
							{
								UploadFile = file_state::get(strFileName) != SavedState;
								Processed = true;
							}
						}

						if (!Processed || LocalKey==KEY_CTRLSHIFTF4 || LocalKey==KEY_RCTRLSHIFTF4)
						{
							if (EnableExternal)
							{
								ProcessExternal(Global->Opt->strExternalEditor, strFileName, strShortFileName, PluginMode);
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
							PluginPanelItemHolder PanelItem;
							const auto strSaveDir = os::fs::GetCurrentDirectory();

							if (!os::fs::exists(strTempName))
							{
								auto strPath = strTempName;
								CutToSlash(strPath, false);
								const auto Find = os::fs::enum_files(strPath + L'*');
								const auto ItemIterator = std::find_if(CONST_RANGE(Find, i) { return !(i.Attributes & FILE_ATTRIBUTE_DIRECTORY); });
								if (ItemIterator != Find.cend())
									strTempName = strPath + ItemIterator->FileName;
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
						const auto EnableExternal = ((LocalKey == KEY_F3 && Global->Opt->ViOpt.UseExternalViewer) ||
							((LocalKey == KEY_ALTF3 || LocalKey == KEY_RALTF3) && !Global->Opt->ViOpt.UseExternalViewer)) &&
							!Global->Opt->strExternalViewer.empty();
						/* $ 02.08.2001 IS обработаем ассоциации для alt-f3 */
						auto Processed = false;

						if ((LocalKey == KEY_ALTF3 || LocalKey == KEY_RALTF3) && ProcessLocalFileTypes(strFileName, strShortFileName, FILETYPE_ALTVIEW, PluginMode))
							Processed = true;
						else if (LocalKey == KEY_F3 && ProcessLocalFileTypes(strFileName, strShortFileName, FILETYPE_VIEW, PluginMode))
							Processed = true;

						if (!Processed || LocalKey==KEY_CTRLSHIFTF3 || LocalKey==KEY_RCTRLSHIFTF3)
						{
							if (EnableExternal)
								ProcessExternal(Global->Opt->strExternalViewer,strFileName,strShortFileName,PluginMode);
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
//			Parent()->Redraw();
			return true;
		}

		case KEY_F5:
		case KEY_F6:
		case KEY_ALTF6:
		case KEY_RALTF6:
		case KEY_DRAGCOPY:
		case KEY_DRAGMOVE:
		{
			_ALGO(CleverSysLog clv(L"F5/F6/Alt-F6/DragCopy/DragMove"));
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(m_PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(LocalKey)));

			ProcessCopyKeys(LocalKey);

			return true;
		}

		case KEY_ALTF5:  // Печать текущего/выбранных файла/ов
		case KEY_RALTF5:
		{
			_ALGO(CleverSysLog clv(L"Alt-F5"));
			_ALGO(SysLog(L"%s, FileCount=%d",(m_PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

			if (!m_ListData.empty() && SetCurPath())
				PrintFiles(this);

			return true;
		}

		case KEY_SHIFTF5:
		case KEY_SHIFTF6:
		{
			_ALGO(CleverSysLog clv(L"Shift-F5/Shift-F6"));
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(m_PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(LocalKey)));

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
					ShellCopy(shared_from_this(), LocalKey == KEY_SHIFTF6, false, true, true, ToPlugin, nullptr);
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
			_ALGO(CleverSysLog clv(L"F7"));
			_ALGO(SysLog(L"%s, FileCount=%d",(m_PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

			if (SetCurPath())
			{
				if (m_PanelMode == panel_mode::PLUGIN_PANEL && !Global->CtrlObject->Plugins->UseFarCommand(GetPluginHandle(), PLUGIN_FARMAKEDIRECTORY))
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

			_ALGO(CleverSysLog clv(L"F8/Shift-F8/Shift-Del/Alt-Del"));
			_ALGO(SysLog(L"%s, FileCount=%d, Key=%s",(m_PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(LocalKey)));

			if (!m_ListData.empty() && SetCurPath())
			{
				if (LocalKey==KEY_SHIFTF8)
					ReturnCurrentFile = true;

				if (m_PanelMode == panel_mode::PLUGIN_PANEL &&
				        !Global->CtrlObject->Plugins->UseFarCommand(GetPluginHandle(), PLUGIN_FARDELETEFILES))
					PluginDelete();
				else
				{
					bool SaveOpt=Global->Opt->DeleteToRecycleBin;

					if (LocalKey==KEY_SHIFTDEL || LocalKey==KEY_SHIFTNUMDEL || LocalKey==KEY_SHIFTDECIMAL)
						Global->Opt->DeleteToRecycleBin = false;

					Delete(shared_from_this(), LocalKey == KEY_ALTDEL || LocalKey == KEY_RALTDEL || LocalKey == KEY_ALTNUMDEL || LocalKey == KEY_RALTNUMDEL || LocalKey == KEY_ALTDECIMAL || LocalKey == KEY_RALTDECIMAL);
					Global->Opt->DeleteToRecycleBin=SaveOpt;
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
			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_LEFT));
			return true;
		}

		case KEY_MSWHEEL_RIGHT:
		case KEY_MSWHEEL_RIGHT | KEY_ALT:
		case KEY_MSWHEEL_RIGHT | KEY_RALT:
		{
			int Roll = LocalKey & (KEY_ALT | KEY_RALT)? 1 : static_cast<int>(Global->Opt->MsHWheelDelta);
			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_RIGHT));
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
				MoveSelection(LocalKey == KEY_SHIFTPGUP || LocalKey == KEY_SHIFTNUMPAD9 ? up : down);

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
					MoveSelection(LocalKey == KEY_SHIFTLEFT || LocalKey == KEY_SHIFTNUMPAD4 ? up : down);

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
			MoveSelection(LocalKey == KEY_SHIFTUP || LocalKey == KEY_SHIFTNUMPAD8 ? up : down);
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
				Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, ToPtr(1)); // EMenu Plugin :-)
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
	if (!IsParentDirectory(SelItem) && SelItem.Selected != Selection)
	{
		CacheSelIndex=-1;
		CacheSelClearIndex=-1;

		if ((SelItem.Selected = Selection) != 0)
		{
			m_SelFileCount++;
			m_SelDirCount += SelItem.Attributes & FILE_ATTRIBUTE_DIRECTORY? 1 : 0;
			SelFileSize += SelItem.FileSize;
		}
		else
		{
			m_SelFileCount--;
			m_SelDirCount -= SelItem.Attributes & FILE_ATTRIBUTE_DIRECTORY? 1 : 0;
			SelFileSize -= SelItem.FileSize;
		}
	}
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

			QuoteSpace(strFullPath);
			OpenFolderInShell(strFullPath);
		}
		else
		{
			const auto CheckFullScreen = IsFullScreen();
			const auto OldParent = Parent();

			// Don't use CurItem directly: ChangeDir calls PopPlugin, which clears m_ListData
			const auto DirCopy = CurItem.FileName;
			const auto DataItemCopy = CurItem.UserData;
			ChangeDir(DirCopy, IsParentDirectory(CurItem), false, true, &DataItemCopy, Type);

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
		const auto PluginMode = m_PanelMode == panel_mode::PLUGIN_PANEL && !Global->CtrlObject->Plugins->UseFarCommand(GetPluginHandle(), PLUGIN_FARGETFILE);
		string FileNameToDelete;
		SCOPE_EXIT{ if (PluginMode && !OpenedPlugin && !FileNameToDelete.empty()) GetPluginHandle()->delayed_delete(FileNameToDelete); };
		file_state SavedState;

		if (PluginMode)
		{
			const auto strTempDir = MakeTemp();
			// BUGBUG check result
			(void)os::fs::create_directory(strTempDir);
			PluginPanelItemHolder PanelItem;
			FileListToPluginItem(CurItem, PanelItem);

			if (!Global->CtrlObject->Plugins->GetFile(GetPluginHandle(), &PanelItem.Item, strTempDir, strFileName, OPM_SILENT | OPM_EDIT))
			{
				// BUGBUG check result
				(void)os::fs::remove_directory(strTempDir);
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
					Info.WaitMode = PluginMode? execute_info::wait_mode::wait_finish : execute_info::wait_mode::no_wait;
					Info.NewWindow = SeparateWindow;
					Info.SourceMode = execute_info::source_mode::known;
					Info.RunAs = RunAs;

					Info.Command = ConvertNameToFull(strFileName);
					QuoteSpace(Info.Command);

					Parent()->GetCmdLine()->ExecString(Info);

					const auto ExclusionFlag = IsItExecutable? EXCLUDECMDHISTORY_NOTPANEL : EXCLUDECMDHISTORY_NOTWINASS;
					if (!(Global->Opt->ExcludeCmdHistory & ExclusionFlag) && !PluginMode)
					{
						string QuotedName = strFileName;
						QuoteSpace(QuotedName);
						Global->CtrlObject->CmdHistory->AddToHistory(QuotedName, HR_DEFAULT, nullptr, {}, m_CurDir);
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
				PluginPanelItemHolder PanelItem;
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


bool FileList::SetCurDir(const string& NewDir,bool ClosePanel,bool IsUpdated)
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
		return ChangeDir(NewDir, NewDir == L".."sv, true, IsUpdated, &UsedData, OFP_NORMAL);
	}

	return false;
}

bool FileList::ChangeDir(string_view const NewDir, bool IsParent, bool ResolvePath,bool IsUpdated, const UserDataItem* DataItem, OPENFILEPLUGINTYPE OfpType)
{
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
		//string strInfoFormat=NullToEmpty(Info.Format);
		string strInfoHostFile = NullToEmpty(m_CachedOpenPanelInfo.HostFile);
		if (m_CachedOpenPanelInfo.Flags&OPIF_SHORTCUT)
		{
			const auto strInfoData = NullToEmpty(m_CachedOpenPanelInfo.ShortcutData);
			Global->CtrlObject->FolderHistory->AddToHistory(strInfoCurDir, HR_DEFAULT, &PluginManager::GetGUID(GetPluginHandle()), strInfoHostFile, strInfoData);
		}
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

		return SetDirectorySuccess;
	}
	else
	{
		if (!equal_icase(ConvertNameToFull(strSetDir), m_CurDir))
			Global->CtrlObject->FolderHistory->AddToHistory(m_CurDir);

		if (IsParent)
		{
			if (RootPath)
			{
				if (NetPath)
				{
					string tmp = m_CurDir;	// strCurDir can be altered during next call
					if (Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Network.Id,OPEN_FILEPANEL, UNSAFE_CSTR(tmp))) // NetWork Plugin :-)
					{
						return false;
					}
				}
				if(DrivePath && Global->Opt->PgUpChangeDisk == 2)
				{
					string RemoteName;
					if(DriveLocalToRemoteName(DRIVE_UNKNOWN, m_CurDir.front(), RemoteName))
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
		if ( strSetDir.empty() || strSetDir[1] != L':' || !IsSlash(strSetDir[2]))
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
		strSetDir = ExtractPathRoot(m_CurDir);
	}

	if (!FarChDir(strSetDir))
	{
		if (Global->WindowManager->ManagerStarted())
		{
			/* $ 03.11.2001 IS Укажем имя неудачного каталога */
			Message(MSG_WARNING, error_state::fetch(),
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
	return ChangeDir(NewDir, IsParent, false, true, nullptr, OFP_NORMAL);
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
			while_mouse_button_pressed([&]
			{
				ProcessKey(Manager::Key(KEY_UP));
				return true;
			});

			Parent()->SetActivePanel(shared_from_this());
			return true;
		}
		else if (IntKeyState.MousePos.y == ScrollY + m_Height - 1)
		{
			// Press and hold the [▼] button
			while_mouse_button_pressed([&]
			{
				ProcessKey(Manager::Key(KEY_DOWN));
				return true;
			});

			Parent()->SetActivePanel(shared_from_this());
			return true;
		}
		else if (IntKeyState.MousePos.y > ScrollY && IntKeyState.MousePos.y < ScrollY + m_Height - 1 && m_Height > 2)
		{
			// Drag the thumb
			while (IsMouseButtonPressed())
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
		Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, nullptr);
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
				ProcessKeyToInputRecord(VK_RETURN,IntKeyState.ShiftPressed()? PKF_SHIFT:0,&rec);
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
							Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, nullptr);

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

		while_mouse_button_pressed([&]
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

		while_mouse_button_pressed([&]
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
			static bool const InvertByDefault[]
			{
				false, // UNSORTED,
				false, // BY_NAME,
				false, // BY_EXT,
				true,  // BY_MTIME,
				true,  // BY_CTIME,
				true,  // BY_ATIME,
				true,  // BY_SIZE,
				false, // BY_DIZ,
				false, // BY_OWNER,
				true,  // BY_COMPRESSEDSIZE,
				true,  // BY_NUMLINKS,
				true,  // BY_NUMSTREAMS,
				true,  // BY_STREAMSSIZE,
				false, // BY_FULLNAME,
				true,  // BY_CHTIME,
			};
			static_assert(std::size(InvertByDefault) == static_cast<size_t>(panel_sort::COUNT));

			m_ReverseSortOrder = (m_SortMode == Mode && Global->Opt->ReverseSort)? !m_ReverseSortOrder : InvertByDefault[static_cast<size_t>(Mode)];
		}

		ApplySortMode(Mode);
	}
	else
		SetCustomSortMode(static_cast<int>(Mode), (KeepOrder ? SO_KEEPCURRENT : SO_AUTO), false);
}

void FileList::SetCustomSortMode(int Mode, sort_order Order, bool InvertByDefault)
{
	if (Mode >= static_cast<int>(panel_sort::COUNT))
	{
		switch (Order)
		{
			default:
			case SO_AUTO:
				m_ReverseSortOrder = (static_cast<int>(m_SortMode) == Mode && Global->Opt->ReverseSort)? !m_ReverseSortOrder : InvertByDefault;
				break;
			case SO_KEEPCURRENT: break;
			case SO_DIRECT: m_ReverseSortOrder = false; break;
			case SO_REVERSE: m_ReverseSortOrder = true; break;
		}

		ApplySortMode(panel_sort(Mode));
	}
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

bool FileList::GoToFile(long idxItem)
{
	if (static_cast<size_t>(idxItem) < m_ListData.size())
	{
		m_CurFile=idxItem;
		CorrectPosition();
		return true;
	}

	return false;
}

bool FileList::GoToFile(const string_view Name, const bool OnlyPartName)
{
	return GoToFile(FindFile(Name,OnlyPartName));
}


long FileList::FindFile(const string_view Name, const bool OnlyPartName)
{
	long II = -1;
	for (long I=0; I < static_cast<int>(m_ListData.size()); I++)
	{
		const auto CurPtrName = OnlyPartName? PointToName(m_ListData[I].FileName) : m_ListData[I].FileName;

		if (Name == CurPtrName)
			return I;

		if (II < 0 && equal_icase(Name, CurPtrName))
			II = I;
	}

	return II;
}

long FileList::FindFirst(const string& Name)
{
	return FindNext(0,Name);
}

long FileList::FindNext(int StartPos, const string& Name)
{
	if (static_cast<size_t>(StartPos) < m_ListData.size())
		for (long I=StartPos; I < static_cast<int>(m_ListData.size()); I++)
		{
			if (CmpName(Name, m_ListData[I].FileName, true))
				if (!IsParentDirectory(m_ListData[I]))
					return I;
		}

	return -1;
}


bool FileList::IsSelected(const string& Name)
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
	return (idxItem < m_ListData.size()) && (!m_Filter || !m_Filter->IsEnabledOnPanel() || m_Filter->FileInFilter(&m_ListData[idxItem])); // BUGBUG, cast
}

// $ 02.08.2000 IG  Wish.Mix #21 - при нажатии '/' или '\' в QuickSerach переходим на директорию
bool FileList::FindPartName(const string& Name,int Next,int Direct)
{
#if !defined(Mantis_698)
	int DirFind = 0;
	string strMask = Name;

	if (!Name.empty() && IsSlash(Name.back()))
	{
		DirFind = 1;
		strMask.pop_back();
	}

	strMask += L'*';

	exclude_sets(strMask);

	for (int I=m_CurFile+(Next?Direct:0); I >= 0 && I < static_cast<int>(m_ListData.size()); I+=Direct)
	{
		if (CmpName(strMask, m_ListData[I].FileName, true, I == m_CurFile))
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

	for (int I=(Direct > 0)?0:static_cast<int>(m_ListData.size()-1); (Direct > 0) ? I < m_CurFile:I > m_CurFile; I+=Direct)
	{
		if (CmpName(strMask, m_ListData[I].FileName, true))
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

	return false;
#else
	// Mantis_698
	// АХТУНГ! В разработке
	string Dest;
	int DirFind = 0;
	string strMask = Name;
	upper(strMask);

	if (!Name.empty() && IsSlash(Name.back()))
	{
		DirFind = 1;
		strMask.pop_back();
	}

/*
	strMask += L'*';

	exclude_sets(strMask);
*/

	for (int I=m_CurFile+(Next?Direct:0); I >= 0 && I < m_ListData.size(); I+=Direct)
	{
		if (GetPlainString(Dest,I) && contains(upper(Dest), strMask))
		//if (CmpName(strMask,ListData[I].FileName,true,I==CurFile))
		{
			if (!IsParentDirectory(m_ListData[I]))
			{
				if (!DirFind || (m_ListData[I].Attributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					m_CurFile=I;
					m_CurTopFile=m_CurFile-(m_Y2-m_Y1)/2;
					ShowFileList();
					return true;
				}
			}
		}
	}

	for (int I=(Direct > 0)?0:m_ListData.size()-1; (Direct > 0) ? I < m_CurFile:I > m_CurFile; I+=Direct)
	{
		if (GetPlainString(Dest,I) && contains(upper(Dest), strMask))
		{
			if (!IsParentDirectory(m_ListData[I]))
			{
				if (!DirFind || (m_ListData[I].Attributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					m_CurFile=I;
					m_CurTopFile=m_CurFile-(m_Y2-m_Y1)/2;
					ShowFileList();
					return true;
				}
			}
		}
	}

	return false;
#endif
}

// собрать в одну строку все данные в отображаемых колонках
bool FileList::GetPlainString(string& Dest, int ListPos) const
{
	Dest.clear();
#if defined(Mantis_698)
	if (ListPos < FileCount)
	{
		unsigned long long *ColumnTypes=m_ViewSettings.ColumnType;
		int ColumnCount=m_ViewSettings.ColumnCount;
		int *ColumnWidths=m_ViewSettings.ColumnWidth;

		for (int K=0; K<ColumnCount; K++)
		{
			int ColumnType=static_cast<int>(ColumnTypes[K] & 0xff);
			int ColumnWidth=ColumnWidths[K];
			if (ColumnType>=custom_0 && ColumnType<=CUSTOM_COLUMN_MAX)
			{
				size_t ColumnNumber=ColumnType-custom_0;
				const wchar_t *ColumnData=nullptr;

				if (ColumnNumber<m_ListData[ListPos].CustomColumnNumber)
					ColumnData=m_ListData[ListPos].CustomColumnData[ColumnNumber];

				if (!ColumnData)
				{
					ColumnData=m_ListData[ListPos].strCustomData;//L"";
				}
				Dest += ColumnData;
			}
			else
			{
				switch (ColumnType)
				{
					case column_type::name:
					{
						unsigned long long ViewFlags=ColumnTypes[K];
						const wchar_t *NamePtr = m_ShowShortNames && !m_ListData[ListPos].strShortName.empty() ? m_ListData[ListPos].strShortName:m_ListData[ListPos].FileName;

						string strNameCopy;
						if (!(m_ListData[ListPos].Attributes & FILE_ATTRIBUTE_DIRECTORY) && (ViewFlags & COLFLAGS_NOEXTENSION))
						{
							const wchar_t *ExtPtr = PointToExt(NamePtr);
							if (ExtPtr)
							{
								strNameCopy.assign(NamePtr, ExtPtr-NamePtr);
								NamePtr = strNameCopy;
							}
						}

						const wchar_t *NameCopy = NamePtr;

						if (ViewFlags & COLFLAGS_NAMEONLY)
						{
							//BUGBUG!!!
							// !!! НЕ УВЕРЕН, но то, что отображается пустое
							// пространство вместо названия - бага
							NamePtr=PointToFolderNameIfFolder(NamePtr);
						}

						Dest += NamePtr;
						break;
					}

					case column_type::extension:
					{
						const wchar_t *ExtPtr = nullptr;
						if (!(m_ListData[ListPos].Attributes & FILE_ATTRIBUTE_DIRECTORY))
						{
							const wchar_t *NamePtr = m_ShowShortNames && !m_ListData[ListPos].strShortName.empty()? m_ListData[ListPos].strShortName:m_ListData[ListPos].FileName;
							ExtPtr = PointToExt(NamePtr);
						}
						if (ExtPtr && *ExtPtr) ExtPtr++; else ExtPtr = L"";

						Dest += ExtPtr;
						break;
					}

					case column_type::size:
					case column_type::size_compressed:
					case column_type::streams_size:
					{
						const auto SizeToDisplay = (ColumnType == size_compressed)
							? m_ListData[ListPos].AllocationSize
							: (ColumnType == streams_size)
							? m_ListData[ListPos].StreamsSize()
							: m_ListData[ListPos].FileSize;

						Dest += FormatStr_Size(
							SizeToDisplay,
							m_ListData[ListPos].FileName,
							m_ListData[ListPos].Attributes,
							m_ListData[ListPos].ShowFolderSize,
							m_ListData[ListPos].ReparseTag,
							ColumnType,
							ColumnTypes[K],
							ColumnWidth,
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
						time_point* FileTime;

						switch (ColumnType)
						{
							case date_creation:
								FileTime=&m_ListData[ListPos].CreationTime;
								break;
							case date_access:
								FileTime=&m_ListData[ListPos].AccessTime;
								break;
							case date_change:
								FileTime=&m_ListData[ListPos].ChangeTime;
								break;
							case date:
							case time:
							case date_write:
							default:
								FileTime=&m_ListData[ListPos].WriteTime;
								break;
						}

						Dest += FormatStr_DateTime(*FileTime, ColumnType, ColumnTypes[K], ColumnWidth);
						break;
					}

					case column_type::attributes:
					{
						Dest += FormatStr_Attribute(m_ListData[ListPos].Attributes,ColumnWidth);
						break;
					}

					case column_type::description:
					{
						Dest += NullToEmpty(m_ListData[ListPos].DizText?);
						break;
					}

					case column_type::owner:
					{
						Dest += m_ListData[ListPos].strOwner;
						break;
					}

					case column_type::links_number:
					{
						Dest += str(m_ListData[ListPos].NumberOfLinks);
						break;
					}

					case column_type::streams_number:
					{
						Dest += str(m_ListData[ListPos].NumberOfStreams);
						break;
					}

				}
			}
		}

		return true;
	}
#endif
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
		if (!GetSelPosition && m_CurFile < static_cast<int>(m_ListData.size()))
		{
			GetSelPosition=1;
			CopyFrom(m_ListData[m_CurFile]);
			LastSelPosition=m_CurFile;
			return true;
		}
		return false;
	}

	while (GetSelPosition < static_cast<int>(m_ListData.size()))
	{
		if (m_ListData[GetSelPosition++].Selected)
		{
			CopyFrom(m_ListData[GetSelPosition - 1]);
			LastSelPosition = GetSelPosition - 1;
			return true;
		}
	}

	return false;
}


void FileList::ClearLastGetSelection()
{
	if (LastSelPosition>=0 && LastSelPosition < static_cast<int>(m_ListData.size()))
		Select(m_ListData[LastSelPosition], false);
}

const FileListItem* FileList::GetLastSelectedItem() const
{
	if (LastSelPosition>=0 && LastSelPosition < static_cast<int>(m_ListData.size()))
	{
		return &m_ListData[LastSelPosition];
	}

	return nullptr;
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

	FileFilter Filter(this, FFT_SELECT);
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
			size_t pos = strRawMask.rfind(L'.');

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
							Filter.FilterEdit();
							//Рефреш текущему времени для фильтра сразу после выхода из диалога
							Filter.UpdateCurrentTime();
							bUseFilter = true;
							break;
						}

						if (Dlg->GetExitCode() != sf_button_ok)
							return 0;

						strMask = SelectDlg[sf_edit].strData;

						if (FileMask.Set(strMask)) // Проверим вводимые пользователем маски на ошибки
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

				if (!FileMask.Set(strMask)) // Проверим маски на ошибки
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

	if (bUseFilter || FileMask.Set(strMask, FMF_SILENT)) // Скомпилируем маски файлов и работаем
	{                                                // дальше в зависимости от успеха компиляции
		for (auto& i: m_ListData)
		{
			int Match=FALSE;

			if (Mode==SELECT_INVERT || Mode==SELECT_INVERTALL)
				Match=TRUE;
			else
			{
				if (bUseFilter)
					Match = Filter.FileInFilter(&i);
				else
					Match = FileMask.Compare(i.AlternateOrNormal(m_ShowShortNames));
			}

			if (Match)
			{
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
					case SELECT_INVERTMASK:
						Selection=!i.Selected;
						break;
				}

				if (bUseFilter || !(i.Attributes & FILE_ATTRIBUTE_DIRECTORY) || Global->Opt->SelectFolders ||
				        !Selection || RawSelection || Mode==SELECT_INVERTALL || Mode==SELECT_INVERTMASK)
				{
					Select(i, Selection);
					workCount++;
				}
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

	if (ViewPanel && !m_ListData.empty() && ViewPanel->IsVisible() && SetCurPath())
	{
		assert(m_CurFile < static_cast<int>(m_ListData.size()));
		const auto& Current = m_ListData[m_CurFile];

		if (m_PanelMode != panel_mode::PLUGIN_PANEL ||
			Global->CtrlObject->Plugins->UseFarCommand(GetPluginHandle(), PLUGIN_FARGETFILE))
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
			(void)os::fs::create_directory(strTempDir);
			PluginPanelItemHolder PanelItem;
			FileListToPluginItem(Current, PanelItem);
			string strFileName;

			if (!Global->CtrlObject->Plugins->GetFile(GetPluginHandle(), &PanelItem.Item, strTempDir, strFileName, OPM_SILENT | OPM_VIEW | OPM_QUICKVIEW))
			{
				ViewPanel->ShowFile({}, nullptr, false, nullptr);
				// BUGBUG check result
				(void)os::fs::remove_directory(strTempDir);
				return;
			}

			ViewPanel->ShowFile(strFileName, nullptr, true, nullptr);
		}
		else if (!IsParentDirectory(Current))
			ViewPanel->ShowFile(Current.FileName, &Current.UserData, false, GetPluginHandle());
		else
			ViewPanel->ShowFile({}, nullptr, false, nullptr);

		RefreshTitle();
	}
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
	// полностью снимаем выделение с обоих панелей
	ClearSelection();
	Another->ClearSelection();

	for (auto& i: m_ListData)
	{
		if (!(i.Attributes & FILE_ATTRIBUTE_DIRECTORY))
			Select(i, true);
	}

	for (auto& i: Another->m_ListData)
	{
		if (!(i.Attributes & FILE_ATTRIBUTE_DIRECTORY))
			Another->Select(i, true);
	}

	bool UseFatTime = false;

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

		if (m_CachedOpenPanelInfo.Flags & OPIF_COMPAREFATTIME)
			UseFatTime = true;

	}

	if (Another->m_PanelMode == panel_mode::PLUGIN_PANEL && !UseFatTime)
	{
		Global->CtrlObject->Plugins->GetOpenPanelInfo(Another->GetPluginHandle(), &m_CachedOpenPanelInfo);

		if (m_CachedOpenPanelInfo.Flags & OPIF_COMPAREFATTIME)
			UseFatTime = true;

	}

	if (m_PanelMode == panel_mode::NORMAL_PANEL && Another->m_PanelMode == panel_mode::NORMAL_PANEL)
	{
		const auto is_fat = [](string_view const Path)
		{
			string FileSystemName;
			return os::fs::GetVolumeInformation(GetPathRoot(Path), nullptr, nullptr, nullptr, nullptr, &FileSystemName) && contains_icase(FileSystemName, L"FAT"sv);
		};

		UseFatTime = is_fat(m_CurDir) || is_fat(Another->m_CurDir);
	}

	// теперь начнем цикл по снятию выделений
	// каждый элемент активной панели...
	for (auto& i: m_ListData)
	{
		if (i.Attributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		// ...сравниваем с элементом пассивной панели...
		for (auto& j: Another->m_ListData)
		{
			if (j.Attributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			if (equal_icase(PointToName(i.FileName), PointToName(j.FileName)))
			{
				const auto Cmp = (UseFatTime? compare_fat_time : compare_time)(i.LastWriteTime, j.LastWriteTime);

				if (!Cmp && (i.FileSize != j.FileSize))
					continue;

				if (Cmp <= 0 && i.Selected)
					Select(i, false);

				if (Cmp >= 0 && j.Selected)
					Another->Select(j, false);

				if (Another->m_PanelMode != panel_mode::PLUGIN_PANEL)
					break;
			}
		}
	}

	const auto refresh = [](FileList& Panel)
	{
		if (Panel.GetSelectedFirstMode())
			Panel.SortFileList(true);
		Panel.Redraw();
	};
	refresh(*this);
	refresh(*Another.get());

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

	const auto Eol = eol::str(eol::system());
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
			QuoteSpace(strQuotedName);

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



bool FileList::GetFileName(string &strName, int Pos, DWORD &FileAttr) const
{
	if (Pos >= static_cast<int>(m_ListData.size()))
		return false;

	strName = m_ListData[Pos].FileName;
	FileAttr=m_ListData[Pos].Attributes;
	return true;
}


int FileList::GetCurrentPos() const
{
	return m_CurFile;
}


void FileList::EditFilter()
{
	if (!m_Filter)
		m_Filter = std::make_unique<FileFilter>(this, FFT_PANEL);

	m_Filter->FilterEdit();
}

void FileList::SelectSortMode()
{
	const menu_item InitSortMenuModes[]
	{
		{ msg(lng::MMenuSortByName),              LIF_SELECTED,    KEY_CTRLF3,      },
		{ msg(lng::MMenuSortByExt),               0,               KEY_CTRLF4,      },
		{ msg(lng::MMenuSortByWrite),             0,               KEY_CTRLF5,      },
		{ msg(lng::MMenuSortBySize),              0,               KEY_CTRLF6,      },
		{ msg(lng::MMenuUnsorted),                0,               KEY_CTRLF7,      },
		{ msg(lng::MMenuSortByCreation),          0,               KEY_CTRLF8,      },
		{ msg(lng::MMenuSortByAccess),            0,               KEY_CTRLF9,      },
		{ msg(lng::MMenuSortByChange),            0,               0,               },
		{ msg(lng::MMenuSortByDiz),               0,               KEY_CTRLF10,     },
		{ msg(lng::MMenuSortByOwner),             0,               KEY_CTRLF11,     },
		{ msg(lng::MMenuSortByAllocatedSize),     0,               0,               },
		{ msg(lng::MMenuSortByNumLinks),          0,               0,               },
		{ msg(lng::MMenuSortByNumStreams),        0,               0,               },
		{ msg(lng::MMenuSortByStreamsSize),       0,               0,               },
		{ msg(lng::MMenuSortByFullName),          0,               0,               },
	};
	static_assert(std::size(InitSortMenuModes) == static_cast<size_t>(panel_sort::COUNT));

	std::vector<menu_item> SortMenu(ALL_CONST_RANGE(InitSortMenuModes));

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

	static const panel_sort SortModes[] =
	{
		panel_sort::BY_NAME,
		panel_sort::BY_EXT,
		panel_sort::BY_MTIME,
		panel_sort::BY_SIZE,
		panel_sort::UNSORTED,
		panel_sort::BY_CTIME,
		panel_sort::BY_ATIME,
		panel_sort::BY_CHTIME,
		panel_sort::BY_DIZ,
		panel_sort::BY_OWNER,
		panel_sort::BY_COMPRESSEDSIZE,
		panel_sort::BY_NUMLINKS,
		panel_sort::BY_NUMSTREAMS,
		panel_sort::BY_STREAMSSIZE,
		panel_sort::BY_FULLNAME,
	};
	static_assert(std::size(SortModes) == static_cast<size_t>(panel_sort::COUNT));

	{
		const auto ItemIterator = std::find(ALL_CONST_RANGE(SortModes), m_SortMode);
		const wchar_t Check = m_ReverseSortOrder? L'-' : L'+';

		if (ItemIterator != std::cend(SortModes))
		{
			SortMenu[ItemIterator - std::cbegin(SortModes)].SetCustomCheck(Check);
			SortMenu[ItemIterator - std::cbegin(SortModes)].SetSelect(true);
		}
		else if (mpr)
		{
			for (size_t i=0; i < mpr->Count; i += 3)
			{
				if (static_cast<int>(mpr->Values[i].Double) == static_cast<int>(m_SortMode))
				{
					SortMenu[std::size(SortModes) + 1 + i/3].SetCustomCheck(Check);
					SortMenu[std::size(SortModes) + 1 + i/3].SetSelect(true);
					break;
				}
			}
		}
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

		const auto SortModeMenu = VMenu2::create(msg(lng::MMenuSortTitle), SortMenu, 0);
		SortModeMenu->SetHelp(L"PanelCmdSort"sv);
		SortModeMenu->SetPosition({ m_Where.left + 4, -1, 0, 0 });
		SortModeMenu->SetMenuFlags(VMENU_WRAPMODE);
		SortModeMenu->SetId(SelectSortModeId);

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
					PlusPressed = Key == L'+' || Key == KEY_ADD;
					KeyProcessed = true;
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

		SetSortMode(SortModes[SortCode], KeepOrder);
	}
	// custom sort modes
	else if (static_cast<size_t>(SortCode) >= std::size(SortModes) + 1 && static_cast<size_t>(SortCode) < std::size(SortModes) + 1 + extra - 1)
	{
		const auto index = 3*(SortCode-std::size(SortModes)-1);
		const auto mode = static_cast<int>(mpr->Values[index].Double);

		if (custom_sort::CanSort(mode))
		{
			const auto InvertByDefault = mpr->Values[index+1].Boolean != 0;
			sort_order Order = SO_AUTO;

			if (!InvertPressed)
			{
				m_ReverseSortOrder = !PlusPressed;
				Order = SO_KEEPCURRENT;
			}

			SetCustomSortMode(mode, Order, InvertByDefault);
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
			if (CheckForEsc())
				break;

			string strConvertedCommand = strCommand;
			delayed_deleter ListNames(false);
			bool PreserveLFN = false;

			if (SubstFileName(strConvertedCommand, subst_context(i.FileName, i.AlternateFileName()), &ListNames, &PreserveLFN) && !strConvertedCommand.empty())
			{
				SCOPED_ACTION(PreserveLongName)(i.AlternateFileName(), PreserveLFN);

				execute_info Info;
				Info.DisplayCommand = strConvertedCommand;
				Info.Command = strConvertedCommand;
				Info.WaitMode = ListNames.any()? execute_info::wait_mode::wait_idle : execute_info::wait_mode::no_wait;

				Parent()->GetCmdLine()->ExecString(Info);

				//if (!(Global->Opt->ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTAPPLYCMD))
				//	Global->CtrlObject->CmdHistory->AddToHistory(strConvertedCommand);
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

	time_check TimeCheck(time_check::mode::delayed, GetRedrawTimeout());

	struct
	{
		unsigned long long Items;
		unsigned long long Size;
	}
	Total{};

	const auto DirInfoCallback = [&](string_view const Name, unsigned long long const ItemsCount, unsigned long long const Size)
	{
		if (TimeCheck)
			DirInfoMsg(msg(lng::MDirInfoViewTitle), Name, Total.Items + ItemsCount, Total.Size + Size);
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

	const auto GetPluginDirInfoOrParent = [this, &Total](const plugin_panel* const ph, const string& DirName, const UserDataItem* const UserData, BasicDirInfoData& BasicData, const dirinfo_callback& Callback)
	{
		if (!m_CurFile && IsParentDirectory(m_ListData[0]))
		{
			const auto PluginCurDir = *m_CachedOpenPanelInfo.CurDir? PointToName(m_CachedOpenPanelInfo.CurDir) : L"\\"sv;
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


void FileList::ProcessCopyKeys(int Key)
{
	if (!m_ListData.empty() && SetCurPath())
	{
		const auto Drag = Key == KEY_DRAGCOPY || Key == KEY_DRAGMOVE;
		const auto Ask = !Drag || Global->Opt->Confirm.Drag;
		const auto Move = Key == KEY_F6 || Key == KEY_DRAGMOVE;
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

		if (m_PanelMode == panel_mode::PLUGIN_PANEL && !Global->CtrlObject->Plugins->UseFarCommand(GetPluginHandle(), PLUGIN_FARGETFILES))
		{
			if (Key!=KEY_ALTF6 && Key!=KEY_RALTF6)
			{
				string strPluginDestPath;
				int ToPlugin = 0;

				if (AnotherPanel->GetMode() == panel_mode::PLUGIN_PANEL && AnotherPanel->IsVisible() &&
				        !Global->CtrlObject->Plugins->UseFarCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES))
				{
					ToPlugin=2;
					ShellCopy(shared_from_this(), Move, false, false, Ask, ToPlugin, &strPluginDestPath);
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
			int ToPlugin = AnotherPanel->GetMode() == panel_mode::PLUGIN_PANEL &&
			             AnotherPanel->IsVisible() && (Key!=KEY_ALTF6 && Key!=KEY_RALTF6) &&
			             !Global->CtrlObject->Plugins->UseFarCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES);
			ShellCopy(shared_from_this(), Move, Key == KEY_ALTF6 || Key == KEY_RALTF6, false, Ask, ToPlugin, nullptr, Drag && AnotherDir);

			if (ToPlugin==1)
				PluginPutFilesToAnother(Move,AnotherPanel);
		}
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
	auto strPath = hPlugin->plugin()->ModuleName();
	CutToSlash(strPath);
	const auto [File, Name, Codepage] = OpenLangFile(strPath, Global->HelpFileMask, Global->Opt->strHelpLanguage);
	if (!File)
		return false;

	help::show(help::make_link(strPath, L"Contents"sv));
	return true;
}

void FileList::ResetLastUpdateTime()
{
	LastUpdateTime = {};
}

/* $ 19.11.2001 IS
     для файловых панелей с реальными файлами никакого префикса не добавляем
*/
string FileList::GetPluginPrefix() const
{
	if (Global->Opt->SubstPluginPrefix && GetMode() == panel_mode::PLUGIN_PANEL)
	{
		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

		if (!(m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES))
		{
			PluginInfo PInfo = {sizeof(PInfo)};
			if (Global->CtrlObject->Plugins->GetPluginInfo(GetPluginHandle()->plugin(), &PInfo) && PInfo.CommandPrefix && *PInfo.CommandPrefix)
			{
				string_view Prefix = PInfo.CommandPrefix;
				return Prefix.substr(0, Prefix.find(L':')) + L":"sv;
			}
		}
	}

	return {};
}


void FileList::IfGoHome(wchar_t Drive)
{
	string strTmpCurDir;
	string strFName=Global->g_strFarModuleName;

	{
		strFName.resize(3); //BUGBUG!
		// СНАЧАЛА ПАССИВНАЯ ПАНЕЛЬ!!!
		/*
			Почему? - Просто - если активная широкая (или пассивная
			широкая) - получаем багу с прорисовкой!
		*/
		const auto Another = Parent()->GetAnotherPanel(this);

		if (Another->GetMode() != panel_mode::PLUGIN_PANEL)
		{
			strTmpCurDir = Another->GetCurDir();

			if (strTmpCurDir[0] == Drive && strTmpCurDir[1] == L':')
				Another->SetCurDir(strFName, false);
		}

		if (GetMode() != panel_mode::PLUGIN_PANEL)
		{
			strTmpCurDir = GetCurDir();

			if (strTmpCurDir[0] == Drive && strTmpCurDir[1] == L':')
				SetCurDir(strFName, false); // переходим в корень диска с far.exe
		}
	}
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

void FileList::PushPlugin(std::unique_ptr<plugin_panel>&& hPlugin,const string& HostFile)
{
	PluginsList.emplace_back(std::make_shared<PluginsListItem>(std::move(hPlugin), HostFile, FALSE, m_ViewMode, m_SortMode, m_ReverseSortOrder, m_DirectoriesFirst, m_ViewSettings));
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
			PluginPanelItemHolder PanelItem={};
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
void FileList::PopPrevData(const string& DefaultName,bool Closed,bool UsePrev,bool Position,bool SetDirectorySuccess)
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

bool FileList::FileNameToPluginItem(const string& Name, PluginPanelItemHolder& pi)
{
	string strTempDir = Name;

	if (!CutToSlash(strTempDir,true))
		return false;

	FarChDir(strTempDir);
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
	To.FileSize = From.FileSize;
	To.AllocationSize = From.AllocationSize;
	To.FileAttributes = From.Attributes;
	To.LastWriteTime = os::chrono::nt_clock::to_filetime(From.LastWriteTime);
	To.CreationTime = os::chrono::nt_clock::to_filetime(From.CreationTime);
	To.LastAccessTime = os::chrono::nt_clock::to_filetime(From.LastAccessTime);
	To.ChangeTime = os::chrono::nt_clock::to_filetime(From.ChangeTime);
	To.Flags = From.UserFlags | (From.Selected? PPIF_SELECTED : 0);
	To.CustomColumnNumber = From.CustomColumnNumber;
	To.CRC32 = From.CRC32;
	To.UserData = From.UserData;
	To.Reserved[0] = To.Reserved[1] = 0;
}

void FileList::FileListToPluginItem(const FileListItem& fi, PluginPanelItemHolder& Holder) const
{
	auto& pi = Holder.Item;

	FileListItemToPluginPanelItemBasic(fi, pi);
	pi.NumberOfLinks = fi.IsNumberOfLinksRead()? fi.NumberOfLinks(this) : 0;

	const auto MakeCopy = [](string_view const Str)
	{
		auto Buffer = std::make_unique<wchar_t[]>(Str.size() + 1);
		*std::copy(ALL_CONST_RANGE(Str), Buffer.get()) = L'\0';
		return Buffer.release();
	};

	pi.FileName = MakeCopy(fi.FileName);
	pi.AlternateFileName = MakeCopy(fi.AlternateFileName());
	pi.CustomColumnData=fi.CustomColumnData;
	pi.Description=fi.DizText; //BUGBUG???
	pi.Owner = fi.IsOwnerRead()? EmptyToNull(fi.Owner(this)) : nullptr;
}

size_t FileList::FileListToPluginItem2(const FileListItem& fi,FarGetPluginPanelItem* gpi) const
{
	const auto StringSizeInBytes = [](string_view const Str)
	{
		return (Str.size() + 1) * sizeof(wchar_t);
	};

	const auto size =
		aligned_sizeof<PluginPanelItem>() +
		StringSizeInBytes(fi.FileName) +
		StringSizeInBytes(fi.AlternateFileName()) +
		fi.CustomColumnNumber * sizeof(wchar_t*) +
		std::accumulate(fi.CustomColumnData, fi.CustomColumnData + fi.CustomColumnNumber, size_t(0), [&](size_t s, const wchar_t* i) { return s + (i? StringSizeInBytes(i) : 0); }) +
		(fi.DizText? StringSizeInBytes(fi.DizText) : 0) +
		((fi.IsOwnerRead() && !fi.Owner(this).empty())? StringSizeInBytes(fi.Owner(this)) : 0);

	if (gpi)
	{
		if(gpi->Item && gpi->Size >= size)
		{
			FileListItemToPluginPanelItemBasic(fi, *gpi->Item);
			gpi->Item->NumberOfLinks = fi.IsNumberOfLinksRead()? fi.NumberOfLinks(this) : 0;

			auto data = reinterpret_cast<char*>(gpi->Item) + aligned_sizeof<PluginPanelItem>();

			const auto CopyToBuffer = [&](string_view const Str)
			{
				const auto Result = reinterpret_cast<const wchar_t*>(data);
				*std::copy(ALL_CONST_RANGE(Str), reinterpret_cast<wchar_t*>(data)) = L'\0';
				data += StringSizeInBytes(Str);
				return Result;
			};

			gpi->Item->FileName = CopyToBuffer(fi.FileName);
			gpi->Item->AlternateFileName = CopyToBuffer(fi.AlternateFileName());

			gpi->Item->CustomColumnData = reinterpret_cast<const wchar_t* const*>(data);
			data += fi.CustomColumnNumber * sizeof(wchar_t*);

			for (size_t i = 0; i != fi.CustomColumnNumber; ++i)
			{
				const_cast<const wchar_t**>(gpi->Item->CustomColumnData)[i] = fi.CustomColumnData[i]? CopyToBuffer(fi.CustomColumnData[i]) : nullptr;
			}

			gpi->Item->Description = fi.DizText? CopyToBuffer(fi.DizText) : nullptr;
			gpi->Item->Owner = (fi.IsOwnerRead() && !fi.Owner(this).empty())? CopyToBuffer(fi.Owner(this)) : nullptr;
		}
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
		CustomColumnData = new wchar_t*[pi.CustomColumnNumber];
		CustomColumnNumber = pi.CustomColumnNumber;

		for (size_t i = 0; i != pi.CustomColumnNumber; ++i)
		{
			if (!pi.CustomColumnData[i])
			{
				CustomColumnData[i] = nullptr;
				continue;
			}

			string_view const Data = pi.CustomColumnData[i];
			CustomColumnData[i] = new wchar_t[Data.size() + 1];
			*std::copy(ALL_CONST_RANGE(Data), CustomColumnData[i]) = L'\0';
		}
	}

	SortGroup = DEFAULT_SORT_GROUP;
	CRC32 = pi.CRC32;

	if (pi.Description)
	{
		string_view const Description = pi.Description;
		const auto Str = new wchar_t[Description.size() + 1];
		*std::copy(ALL_CONST_RANGE(Description), Str) = L'\0';
		DizText = Str;
		DeleteDiz = true;
	}

	FileName = NullToEmpty(pi.FileName);
	SetAlternateFileName(NullToEmpty(pi.AlternateFileName));
	m_Owner = NullToEmpty(pi.Owner);

	m_NumberOfLinks = pi.NumberOfLinks;
	m_NumberOfStreams = 1;
	m_StreamsSize = FileSize;
}


std::unique_ptr<plugin_panel> FileList::OpenPluginForFile(const string& FileName, DWORD FileAttr, OPENFILEPLUGINTYPE Type, bool* StopProcessing)
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
		PluginPanelItemHolderNonOwning NewItem;
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
	_ALGO(CleverSysLog clv(L"FileList::PluginDelete()"));
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
	_ALGO(CleverSysLog clv(L"FileList::PutDizToPlugin()"));

	Global->CtrlObject->Plugins->GetOpenPanelInfo(DestPanel->GetPluginHandle(), &m_CachedOpenPanelInfo);

	if (DestPanel->strPluginDizName.empty() && m_CachedOpenPanelInfo.DescrFilesNumber>0)
		DestPanel->strPluginDizName = m_CachedOpenPanelInfo.DescrFiles[0];

	if (((Global->Opt->Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed()) ||
	        Global->Opt->Diz.UpdateMode==DIZ_UPDATE_ALWAYS) && !DestPanel->strPluginDizName.empty() &&
	        (!m_CachedOpenPanelInfo.HostFile || !*m_CachedOpenPanelInfo.HostFile || DestPanel->GetModalMode() ||
             os::fs::exists(m_CachedOpenPanelInfo.HostFile)))
	{
		Parent()->LeftPanel()->ReadDiz();
		Parent()->RightPanel()->ReadDiz();

		if (DestPanel->GetModalMode())
			DestPanel->ReadDiz();

		bool DizPresent = false;

		for (const auto& i: ItemList)
		{
			if (i.Flags & PPIF_PROCESSDESCR)
			{
				int Code;

				if (Delete)
					Code = DestPanel->Diz.Erase(i.FileName, i.AlternateFileName);
				else
				{
					Code = SrcDiz->CopyDiz(i.FileName, i.AlternateFileName, i.FileName, i.AlternateFileName, &DestPanel->Diz);

					if (Code && Move)
						SrcDiz->Erase(i.FileName, i.AlternateFileName);
				}

				if (Code)
					DizPresent = true;
			}
		}

		if (DizPresent)
		{
			const auto strTempDir = MakeTemp();
			if (os::fs::create_directory(strTempDir))
			{
				const auto strSaveDir = os::fs::GetCurrentDirectory();
				auto strDizName = concat(strTempDir, L'\\', DestPanel->strPluginDizName);
				DestPanel->Diz.Flush({}, &strDizName);

				if (Move)
					SrcDiz->Flush({});

				PluginPanelItemHolder PanelItem;
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
		}
	}
}


void FileList::PluginGetFiles(const string& DestPath, bool Move)
{
	_ALGO(CleverSysLog clv(L"FileList::PluginGetFiles()"));
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
	_ALGO(CleverSysLog clv(L"FileList::PluginToPluginFiles()"));

	const auto AnotherFilePanel = std::dynamic_pointer_cast<FileList>(Parent()->GetAnotherPanel(this));
	if (!AnotherFilePanel || AnotherFilePanel->GetMode() != panel_mode::PLUGIN_PANEL)
	{
		return;
	}

	SaveSelection();
	auto strTempDir = MakeTemp();
	const auto OriginalTempDir = strTempDir;
	// BUGBUG check result
	(void)os::fs::create_directory(strTempDir);

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
		(void)os::fs::remove_directory(OriginalTempDir);
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

	if (((!AnotherPanel->IsVisible() || AnotherPanel->GetType() != panel_type::FILE_PANEL) &&
	        !m_SelFileCount) || strDestPath.empty())
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

		if (hCurPlugin)
		{
			int OpMode=OPM_TOPLEVEL;
			if (contains(UsedPlugins, hCurPlugin->plugin()))
				OpMode|=OPM_SILENT;

			span<PluginPanelItem> Items;
			_ALGO(SysLog(L"call Plugins.GetFindData()"));

			if (Global->CtrlObject->Plugins->GetFindData(hCurPlugin.get(), Items, OpMode))
			{
				_ALGO(SysLog(L"call Plugins.GetFiles()"));
				auto DestPath = strDestPath.c_str();
				ExitLoop = Global->CtrlObject->Plugins->GetFiles(hCurPlugin.get(), Items, false, &DestPath, OpMode) != 1;
				strDestPath=DestPath;

				if (!ExitLoop)
				{
					_ALGO(SysLog(L"call ClearLastGetSelection()"));
					ClearLastGetSelection();
				}

				_ALGO(SysLog(L"call Plugins.FreeFindData()"));
				Global->CtrlObject->Plugins->FreeFindData(hCurPlugin.get(), Items, true);
				UsedPlugins.emplace(hCurPlugin->plugin());
			}

			_ALGO(SysLog(L"call Plugins.ClosePanel"));
			Global->CtrlObject->Plugins->ClosePanel(std::move(hCurPlugin));
		}
	}

	Update(UPDATE_KEEP_SELECTION);
	Redraw();
	AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	AnotherPanel->Redraw();
}


void FileList::PluginPutFilesToNew()
{
	_ALGO(CleverSysLog clv(L"FileList::PluginPutFilesToNew()"));
	//_ALGO(SysLog(L"FileName='%s'",(FileName?FileName:"(nullptr)")));
	_ALGO(SysLog(L"call Plugins.OpenFilePlugin(nullptr, 0)"));
	auto hNewPlugin = Global->CtrlObject->Plugins->OpenFilePlugin(nullptr, OPM_NONE, OFP_CREATE);
	if (!hNewPlugin)
		return;

	_ALGO(SysLog(L"Create: FileList TmpPanel, FileCount=%d",FileCount));
	auto TmpPanel = create(nullptr);
	TmpPanel->SetPluginMode(std::move(hNewPlugin), {});  // SendOnFocus??? true???
	TmpPanel->m_ModalMode = TRUE;
	const auto PrevFileCount = m_ListData.size();
	/* $ 12.04.2002 IS
		Если PluginPutFilesToAnother вернула число, отличное от 2, то нужно
		попробовать установить курсор на созданный файл.
	*/
	const auto rc = PluginPutFilesToAnother(false, TmpPanel);

	if (rc != 2 && m_ListData.size() == PrevFileCount+1)
	{
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
		_ALGO(SysLog(L"call Plugins.PutFiles"));
		PutCode = Global->CtrlObject->Plugins->PutFiles(AnotherFilePanel->GetPluginHandle(), ItemList, Move, 0);

		if (PutCode==1 || PutCode==2)
		{
			if (!ReturnCurrentFile)
			{
				_ALGO(SysLog(L"call ClearSelection()"));
				ClearSelection();
			}

			_ALGO(SysLog(L"call PutDizToPlugin"));
			PutDizToPlugin(AnotherFilePanel.get(), ItemList.items(), false, Move, &Diz);
			AnotherFilePanel->SetPluginModified();
		}
		else if (!ReturnCurrentFile)
			PluginClearSelection(ItemList.items());

		_ALGO(SysLog(L"call DeletePluginItemList"));
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
	_ALGO(CleverSysLog clv(L"FileList::GetOpenPanelInfo()"));
	//_ALGO(SysLog(L"FileName='%s'",(FileName?FileName:"(nullptr)")));
	*Info = {};

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), Info);
}


/*
   Функция для вызова команды "Архивные команды" (Shift-F3)
*/
void FileList::ProcessHostFile()
{
	_ALGO(CleverSysLog clv(L"FileList::ProcessHostFile()"));

	//_ALGO(SysLog(L"FileName='%s'",(FileName?FileName:"(nullptr)")));
	if (!m_ListData.empty() && SetCurPath())
	{
		int Done=FALSE;
		SaveSelection();

		if (m_PanelMode == panel_mode::PLUGIN_PANEL && !PluginsList.back()->m_HostFile.empty())
		{
			{
				_ALGO(SysLog(L"call CreatePluginItemList"));
				auto ItemList = CreatePluginItemList();
				_ALGO(SysLog(L"call Plugins.ProcessHostFile"));
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
	_ALGO(CleverSysLog clv(L"FileList::ProcessOneHostFile()"));
	int Done=-1;

	auto hNewPlugin = OpenPluginForFile(Item->FileName, Item->Attributes, OFP_COMMANDS);
	if (!hNewPlugin)
		return Done;

	span<PluginPanelItem> Items;
	_ALGO(SysLog(L"call Plugins.GetFindData"));

	if (Global->CtrlObject->Plugins->GetFindData(hNewPlugin.get(), Items, OPM_TOPLEVEL))
	{
		_ALGO(SysLog(L"call Plugins.ProcessHostFile"));
		Done = Global->CtrlObject->Plugins->ProcessHostFile(hNewPlugin.get(), Items, OPM_TOPLEVEL);
		_ALGO(SysLog(L"call Plugins.FreeFindData"));
		Global->CtrlObject->Plugins->FreeFindData(hNewPlugin.get(), Items, true);
	}

	_ALGO(SysLog(L"call Plugins.ClosePanel"));
	Global->CtrlObject->Plugins->ClosePanel(std::move(hNewPlugin));

	return Done;
}



void FileList::SetPluginMode(std::unique_ptr<plugin_panel>&& PluginPanel, const string& PluginFile, bool SendOnFocus)
{
	const auto ParentWindow = Parent();

	if (m_PanelMode != panel_mode::PLUGIN_PANEL)
	{
		Global->CtrlObject->FolderHistory->AddToHistory(m_CurDir);
	}

	PushPlugin(std::move(PluginPanel), PluginFile);

	m_PanelMode = panel_mode::PLUGIN_PANEL;

	if (SendOnFocus)
		OnFocusChange(true);

	Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

	if (m_CachedOpenPanelInfo.StartPanelMode)
		SetViewMode(VIEW_0 + m_CachedOpenPanelInfo.StartPanelMode-L'0');

	if (m_CachedOpenPanelInfo.StartSortMode)
	{
		if (m_CachedOpenPanelInfo.StartSortMode <= SM_COUNT)
			m_SortMode = panel_sort(m_CachedOpenPanelInfo.StartSortMode - (SM_UNSORTED - static_cast<int>(panel_sort::UNSORTED)));
		else
			m_SortMode = panel_sort(m_CachedOpenPanelInfo.StartSortMode);
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
	size_t result=0;

	if (static_cast<size_t>(ItemNumber) < m_ListData.size())
	{
		if (ItemNumber==CacheSelIndex)
		{
			result=FileListToPluginItem2(m_ListData[CacheSelPos], Item);
		}
		else
		{
			if (ItemNumber<CacheSelIndex) CacheSelIndex=-1;

			int CurSel = CacheSelIndex;
			const int StartValue = CacheSelIndex >= 0? CacheSelPos + 1 : 0;

			for (size_t i=StartValue; i<m_ListData.size(); i++)
			{
				if (m_ListData[i].Selected)
					CurSel++;

				if (CurSel==ItemNumber)
				{
					result=FileListToPluginItem2(m_ListData[i], Item);
					CacheSelIndex=ItemNumber;
					CacheSelPos=static_cast<int>(i);
					break;
				}
			}

			if (CurSel==-1 && !ItemNumber)
			{
				result=FileListToPluginItem2(m_ListData[m_CurFile], Item);
				CacheSelIndex=-1;
			}
		}
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
	if (static_cast<size_t>(SelectedItemNumber) < m_ListData.size())
	{
		if (SelectedItemNumber<=CacheSelClearIndex)
		{
			CacheSelClearIndex=-1;
		}

		int CurSel = CacheSelClearIndex;
		const auto StartValue = CacheSelClearIndex >= 0? CacheSelClearPos + 1 : 0;

		for (size_t i=StartValue; i < m_ListData.size(); i++)
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
	if(!PluginsList.empty())
	{
		PluginsList.back()->m_Modified = TRUE;
	}
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
	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		return Global->CtrlObject->Plugins->ProcessEvent(GetPluginHandle(), Event, Param) != FALSE;

	return false;
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
	_ALGO(CleverSysLog clv(L"FileList::Update"));
	_ALGO(SysLog(L"(Mode=[%d/0x%08X] %s)",Mode,Mode,(Mode==UPDATE_KEEP_SELECTION?L"UPDATE_KEEP_SELECTION":L"")));

	if (m_EnableUpdate)
		switch (m_PanelMode)
		{
		case panel_mode::NORMAL_PANEL:
				ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
				break;
		case panel_mode::PLUGIN_PANEL:
			{
				Global->CtrlObject->Plugins->GetOpenPanelInfo(GetPluginHandle(), &m_CachedOpenPanelInfo);

				if (m_PanelMode != panel_mode::PLUGIN_PANEL)
					ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
				else if ((m_CachedOpenPanelInfo.Flags & OPIF_REALNAMES) || Parent()->GetAnotherPanel(this)->GetMode() == panel_mode::PLUGIN_PANEL || !(Mode & UPDATE_SECONDARY))
					UpdatePlugin(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE);
			}
			break;
		}

	LastUpdateTime = std::chrono::steady_clock::now();
}

void FileList::UpdateIfRequired()
{
	if (UpdateRequired && !UpdateDisabled)
	{
		UpdateRequired = false;
		Update(UpdateRequiredMode | UPDATE_IGNORE_VISIBLE);
	}
}

static void PR_ReadFileNamesMsg();

struct FileListPreRedrawItem : PreRedrawItem
{
	FileListPreRedrawItem() : PreRedrawItem(PR_ReadFileNamesMsg){}

	string Msg;
};

static void ReadFileNamesMsgImpl(const string& Msg)
{
	Message(0,
		msg(lng::MReadingTitleFiles),
		{
			Msg
		},
		{});
}

static void ReadFileNamesMsg(const string& Msg)
{
	ReadFileNamesMsgImpl(Msg);

	TPreRedrawFunc::instance()([&](FileListPreRedrawItem& Item)
	{
		Item.Msg = Msg;
	});
}

static void PR_ReadFileNamesMsg()
{
	TPreRedrawFunc::instance()([](const FileListPreRedrawItem& Item)
	{
		ReadFileNamesMsgImpl(Item.Msg);
	});
}

void FileList::ReadFileNames(int KeepSelection, int UpdateEvenIfPanelInvisible, int DrawMessage)
{
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<FileListPreRedrawItem>());
	SCOPED_ACTION(taskbar::indeterminate)(false);

	strOriginalCurDir = m_CurDir;

	if (!IsVisible() && !UpdateEvenIfPanelInvisible)
	{
		UpdateRequired = true;
		UpdateRequiredMode=KeepSelection;
		return;
	}

	UpdateRequired = false;
	AccessTimeUpdateRequired = false;
	DizRead = false;
	decltype(m_ListData) OldData;
	string strCurName, strNextCurName;
	StopFSWatcher();

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
					IfGoHome(strOldCurDir.front());

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
		(void)os::fs::get_disk_size(m_CurDir, nullptr, nullptr, &FreeDiskSize);
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

	DWORD FileSystemFlags = 0;
	string FileSystemName;
	// BUGBUG check result
	(void)os::fs::GetVolumeInformation(GetPathRoot(m_CurDir), nullptr, nullptr, nullptr, &FileSystemFlags, &FileSystemName);

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
		m_Filter = std::make_unique<FileFilter>(this, FFT_PANEL);

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

	error_state ErrorState;
	const time_check TimeCheck(time_check::mode::delayed, GetRedrawTimeout());

	for (const auto& fdata: Find)
	{
		ErrorState = error_state::fetch();

		if ((Global->Opt->ShowHidden || !(fdata.Attributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))) && (!UseFilter || m_Filter->FileInFilter(fdata, nullptr, &fdata.FileName)))
		{
			{
				FileListItem NewItem{};

				static_cast<os::fs::find_data&>(NewItem) = fdata;

				NewItem.Position = m_ListData.size();
				NewItem.SortGroup = DEFAULT_SORT_GROUP;

				if (!(fdata.Attributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					TotalFileSize += NewItem.FileSize;
				}

				m_ListData.emplace_back(std::move(NewItem));
			}

			if (fdata.Attributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				++m_TotalDirCount;
			}
			else
			{
				++m_TotalFileCount;
			}

			if (TimeCheck)
			{
				if (IsVisible())
				{
					if (!IsShowTitle)
					{
						if (!DrawMessage)
						{
							Text({ m_Where.left + 1, m_Where.top }, colors::PaletteColorToFarColor(COL_PANELBOX), Title);
							IsShowTitle = true;
							SetColor(IsFocused()? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
						}
					}

					auto strReadMsg = format(msg(lng::MReadingFiles), m_ListData.size());

					if (DrawMessage)
					{
						ReadFileNamesMsg(strReadMsg);
					}
					else
					{
						inplace::truncate_left(strReadMsg, Title.size() - 2);
						GotoXY(m_Where.left + 1 + static_cast<int>(Title.size() - strReadMsg.size() - 1) / 2, m_Where.top);
						Text(concat(L' ', strReadMsg, L' '));
					}
				}

				Global->CtrlObject->Macro.SuspendMacros(true);
				bool check = CheckForEsc();
				Global->CtrlObject->Macro.SuspendMacros(false);
				if (check)
					break;
			}
		}
	}

	if (!ErrorState)
		ErrorState = error_state::fetch();

	if (!(ErrorState.Win32Error == ERROR_SUCCESS || ErrorState.Win32Error == ERROR_NO_MORE_FILES || ErrorState.Win32Error == ERROR_FILE_NOT_FOUND))
	{
		Message(MSG_WARNING, ErrorState,
			msg(lng::MError),
			{
				msg(lng::MReadFolderError),
			},
			{ lng::MOk });
	}

	if ((Global->Opt->ShowDotsInRoot || !bCurDirRoot) || (NetRoot && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Network.Id))) // NetWork Plugin
	{
		FileListItem NewItem;
		FillParentPoint(NewItem, m_ListData.size() + 1);

		os::chrono::time_point TwoDotsTimes[4];
		// BUGBUG check result
		(void)os::fs::GetFileTimeSimple(m_CurDir, &TwoDotsTimes[0], &TwoDotsTimes[1], &TwoDotsTimes[2], &TwoDotsTimes[3]);

		NewItem.CreationTime = TwoDotsTimes[0];
		NewItem.LastAccessTime = TwoDotsTimes[1];
		NewItem.LastWriteTime = TwoDotsTimes[2];
		NewItem.ChangeTime = TwoDotsTimes[3];

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
			const auto OldSize = m_ListData.size();
			auto Position = OldSize - 1;
			m_ListData.reserve(m_ListData.size() + PanelData.size());

			OpenPanelInfo AnotherPanelInfo{};
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hAnotherPlugin, &AnotherPanelInfo);

			auto ParentPointSeen = AnotherPanelInfo.Flags & OPIF_ADDDOTS? true : false;

			for (const auto& i: PanelData)
			{
				FileListItem Item{ i };
				Item.Position = Position++;
				Item.PrevSelected = Item.Selected = false;
				Item.ShowFolderSize = 0;
				Item.SortGroup = Global->CtrlObject->HiFiles->GetGroup(Item, this);

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

	InitFSWatcher(false);
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

/*$ 22.06.2001 SKV
  Добавлен параметр для вызова после исполнения команды.
*/
bool FileList::UpdateIfChanged(bool Idle)
{
	//_SVS(SysLog(L"CurDir='%s' Global->Opt->AutoUpdateLimit=%d <= FileCount=%d",CurDir,Global->Opt->AutoUpdateLimit,FileCount));
	if (!Global->Opt->AutoUpdateLimit || m_ListData.size() <= static_cast<size_t>(Global->Opt->AutoUpdateLimit))
	{
		/* $ 19.12.2001 VVM
		  ! Сменим приоритеты. При Force обновление всегда! */
		if (IsVisible() && (std::chrono::steady_clock::now() - LastUpdateTime > 2s))
		{
			if (Idle) ProcessPluginEvent(FE_IDLE,nullptr);

			/* $ 24.12.2002 VVM
			  ! Поменяем логику обновления панелей. */
			if (m_PanelMode == panel_mode::NORMAL_PANEL && FSWatcher.Signaled())
			{
				const auto AnotherPanel = Parent()->GetAnotherPanel(this);

				if (AnotherPanel->GetType() == panel_type::INFO_PANEL)
				{
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);
					AnotherPanel->Redraw();
				}

				Update(UPDATE_KEEP_SELECTION);
				Redraw();
				return true;
			}
		}
	}

	return false;
}

void FileList::InitFSWatcher(bool CheckTree)
{
	DWORD DriveType=DRIVE_REMOTE;
	StopFSWatcher();
	const auto Type = ParsePath(m_CurDir);

	if (Type == root_type::drive_letter || Type == root_type::unc_drive_letter)
	{
		DriveType = FAR_GetDriveType(os::fs::get_root_directory(m_CurDir[(Type == root_type::drive_letter) ? 0 : 4]));
	}

	if (Global->Opt->AutoUpdateRemoteDrive || (!Global->Opt->AutoUpdateRemoteDrive && DriveType != DRIVE_REMOTE) || Type == root_type::volume)
	{
		FSWatcher.Set(m_CurDir, CheckTree);
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

static struct search_list_less
{
	bool operator()(const FileListItem& a, const FileListItem& b) const
	{
		return a.FileName < b.FileName;
	}
}
SearchListLess;

void FileList::MoveSelection(list_data& From, list_data& To)
{
	m_SelFileCount=0;
	m_SelDirCount = 0;
	SelFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;

	std::sort(From.begin(), From.end(), SearchListLess);

	for (auto& i: To)
	{
		const auto OldItem = std::lower_bound(ALL_CONST_RANGE(From), i, SearchListLess);
		if (OldItem != From.end())
		{
			if (OldItem->FileName == i.FileName)
			{
				if (OldItem->ShowFolderSize)
				{
					i.ShowFolderSize = 2;
					i.FileSize = OldItem->FileSize;
					i.AllocationSize = OldItem->AllocationSize;
				}

				Select(i, OldItem->Selected);
				i.PrevSelected = OldItem->PrevSelected;
			}
		}
	}
}

void FileList::UpdatePlugin(int KeepSelection, int UpdateEvenIfPanelInvisible)
{
	_ALGO(CleverSysLog clv(L"FileList::UpdatePlugin"));
	_ALGO(SysLog(L"(KeepSelection=%d, IgnoreVisible=%d)",KeepSelection,UpdateEvenIfPanelInvisible));

	if (!IsVisible() && !UpdateEvenIfPanelInvisible)
	{
		UpdateRequired = true;
		UpdateRequiredMode=KeepSelection;
		return;
	}

	DizRead = false;
	decltype(m_ListData) OldData;
	string strCurName, strNextCurName;
	StopFSWatcher();
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
			(void)os::fs::get_disk_size(m_CurDir, nullptr, nullptr, &FreeDiskSize);
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

	if (!m_Filter)
		m_Filter = std::make_unique<FileFilter>(this, FFT_PANEL);

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
		FillParentPoint(NewItem, m_ListData.size() + 1);

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
		m_ListData.emplace_back(std::move(NewItem));
		ParentPoint.TryToFind = false;
	}

	size_t Position = 0;
	for (const auto& PanelItem: PanelData)
	{
		if (UseFilter && !(m_CachedOpenPanelInfo.Flags & OPIF_DISABLEFILTER))
		{
			//if (!(CurPanelData->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			if (!m_Filter->FileInFilter(PanelItem))
				continue;
		}

		if (!Global->Opt->ShowHidden && (PanelItem.FileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
			continue;

		FileListItem NewItem(PanelItem);
		NewItem.Position = Position++;

		NewItem.SortGroup = (m_CachedOpenPanelInfo.Flags & OPIF_DISABLESORTGROUPS)? DEFAULT_SORT_GROUP : Global->CtrlObject->HiFiles->GetGroup(NewItem, this);

		m_ListData.emplace_back(std::move(NewItem));

		if (ParentPoint.TryToFind && !ParentPoint.Item && IsParentDirectory(PanelItem))
		{
			// We reserve capacity so no reallocation will happen and pointer will stay valid.
			ParentPoint.Item = &m_ListData.back();
			FillParentPoint(*ParentPoint.Item, 0);
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

	string strLastSel, strGetSel;

	if (KeepSelection || PrevSelFileCount>0)
	{
		if (LastSelPosition >= 0 && LastSelPosition < static_cast<long>(OldData.size()))
			strLastSel = OldData[LastSelPosition].FileName;
		if (GetSelPosition >= 0 && GetSelPosition < static_cast<long>(OldData.size()))
			strGetSel = OldData[GetSelPosition].FileName;

		MoveSelection(OldData, m_ListData);
		OldData.clear();
	}

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
			for (size_t I=0; I<m_CachedOpenPanelInfo.DescrFilesNumber; I++)
			{
				for (auto& CurPanelData: PanelData)
				{
					if (equal_icase(CurPanelData.FileName, m_CachedOpenPanelInfo.DescrFiles[I]))
					{
						const auto strTempDir = MakeTemp();

						if (os::fs::create_directory(strTempDir))
						{
							string strDizName;
							if (Global->CtrlObject->Plugins->GetFile(GetPluginHandle(), &CurPanelData, strTempDir, strDizName, OPM_SILENT | OPM_VIEW | OPM_QUICKVIEW | OPM_DESCR))
							{
								strPluginDizName = m_CachedOpenPanelInfo.DescrFiles[I];
								Diz.Read({}, &strDizName);
								DeleteFileWithFolder(strDizName);
								I = m_CachedOpenPanelInfo.DescrFilesNumber;
								break;
							}

							// BUGBUG check result
							(void)os::fs::remove_directory(strTempDir);
							//ViewPanel->ShowFile(nullptr,FALSE,nullptr);
						}
					}
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
	if (!SortGroupsRead)
	{
		if (UpdateFilterCurrentTime)
		{
			Global->CtrlObject->HiFiles->UpdateCurrentTime();
		}

		SortGroupsRead = true;

		for (auto& i: m_ListData)
		{
			i.SortGroup = Global->CtrlObject->HiFiles->GetGroup(i, this);
		}
	}
}

// занести предопределенные данные для каталога ".."
void FileList::FillParentPoint(FileListItem& Item, size_t CurFilePos)
{
	Item.Attributes = FILE_ATTRIBUTE_DIRECTORY;
	Item.FileName = L".."sv;
	Item.SetAlternateFileName(Item.FileName);
	Item.Position = CurFilePos;
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
	_OT(SysLog(L"[%p] FileList::DisplayObject()",this));

	if (UpdateRequired)
	{
		UpdateRequired = false;
		Update(UpdateRequiredMode);
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

	bool CurFullScreen=IsFullScreen();
	PrepareViewSettings(m_ViewMode);
	CorrectPosition();

	if (CurFullScreen!=IsFullScreen())
	{
		Parent()->SetScreenPosition();
		Parent()->GetAnotherPanel(this)->Update(UPDATE_KEEP_SELECTION | UPDATE_SECONDARY);
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
			lng IDMessage = lng::MColumnUnknown;

			switch (m_ViewSettings.PanelColumns[I].type)
			{
				case column_type::name:               IDMessage = lng::MColumnName;            break;
				case column_type::extension:          IDMessage = lng::MColumnExtension;       break;
				case column_type::size:               IDMessage = lng::MColumnSize;            break;
				case column_type::size_compressed:    IDMessage = lng::MColumnAlocatedSize;    break;
				case column_type::date:               IDMessage = lng::MColumnDate;            break;
				case column_type::time:               IDMessage = lng::MColumnTime;            break;
				case column_type::date_write:         IDMessage = lng::MColumnWrited;          break;
				case column_type::date_creation:      IDMessage = lng::MColumnCreated;         break;
				case column_type::date_access:        IDMessage = lng::MColumnAccessed;        break;
				case column_type::date_change:        IDMessage = lng::MColumnChanged;         break;
				case column_type::attributes:         IDMessage = lng::MColumnAttr;            break;
				case column_type::description:        IDMessage = lng::MColumnDescription;     break;
				case column_type::owner:              IDMessage = lng::MColumnOwner;           break;
				case column_type::links_number:       IDMessage = lng::MColumnMumLinks;        break;
				case column_type::streams_number:     IDMessage = lng::MColumnNumStreams;      break;
				case column_type::streams_size:       IDMessage = lng::MColumnStreamsSize;     break;
				default: break;
			}

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

		BoxText(BoxSymbols[DoubleLine?BS_T_H2V2:BS_T_H2V1]);

		if (Global->Opt->ShowColumnTitles)
		{
			const auto ColumnTitleColor = colors::PaletteColorToFarColor(COL_PANELCOLUMNTITLE);
			auto Color = colors::PaletteColorToFarColor(COL_PANELBOX);
			Color.BackgroundColor = ColumnTitleColor.BackgroundColor;
			Color.SetBg4Bit(ColumnTitleColor.IsBg4Bit());
			SetColor(Color);

			GotoXY(static_cast<int>(ColumnPos), m_Where.top + 1);
			BoxText(BoxSymbols[DoubleLine?BS_V2:BS_V1]);
		}

		if (!Global->Opt->ShowPanelStatus)
		{
			GotoXY(static_cast<int>(ColumnPos), m_Where.bottom);
			BoxText(BoxSymbols[DoubleLine?BS_B_H2V2:BS_B_H2V1]);
		}

		ColumnPos++;
	}

	int NextX1 = m_Where.left + 1;

	if (Global->Opt->ShowSortMode)
	{
		wchar_t Indicator = 0;

		if (m_SortMode < panel_sort::COUNT)
		{
			static const std::pair<panel_sort, lng> ModeNames[] =
			{
				{panel_sort::UNSORTED,                lng::MMenuUnsorted},
				{panel_sort::BY_NAME,                 lng::MMenuSortByName},
				{panel_sort::BY_EXT,                  lng::MMenuSortByExt},
				{panel_sort::BY_MTIME,                lng::MMenuSortByWrite},
				{panel_sort::BY_CTIME,                lng::MMenuSortByCreation},
				{panel_sort::BY_ATIME,                lng::MMenuSortByAccess},
				{panel_sort::BY_CHTIME,               lng::MMenuSortByChange},
				{panel_sort::BY_SIZE,                 lng::MMenuSortBySize},
				{panel_sort::BY_DIZ,                  lng::MMenuSortByDiz},
				{panel_sort::BY_OWNER,                lng::MMenuSortByOwner},
				{panel_sort::BY_COMPRESSEDSIZE,       lng::MMenuSortByAllocatedSize},
				{panel_sort::BY_NUMLINKS,             lng::MMenuSortByNumLinks},
				{panel_sort::BY_NUMSTREAMS,           lng::MMenuSortByNumStreams},
				{panel_sort::BY_STREAMSSIZE,          lng::MMenuSortByStreamsSize},
				{panel_sort::BY_FULLNAME,             lng::MMenuSortByFullName},
			};
			static_assert(std::size(ModeNames) == static_cast<size_t>(panel_sort::COUNT));

			const auto& CurrentModeName = msg(std::find_if(CONST_RANGE(ModeNames, i) { return i.first == m_SortMode; })->second);
			// Owerflow from npos to 0 is ok - pick the first character if & isn't there.
			const auto Char = CurrentModeName[CurrentModeName.find(L'&') + 1];
			Indicator = m_ReverseSortOrder? upper(Char) : lower(Char);
		}
		else
		{
			Indicator = m_ReverseSortOrder? CustomSortIndicator[1] : CustomSortIndicator[0];
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
		ScrollBarEx(m_Where.right, m_Where.top + 1 + Global->Opt->ShowColumnTitles, m_Height, Round(m_CurTopFile, m_Stripes), Round(static_cast<int>(m_ListData.size()), m_Stripes));
	}

	ShowScreensCount();

	if (!ProcessingPluginCommand && LastCurFile!=m_CurFile)
	{
		LastCurFile=m_CurFile;
		UpdateViewPanel();
	}

	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
		Parent()->RedrawKeyBar();
}


FarColor FileList::GetShowColor(int Position, bool FileColor) const
{
	auto ColorAttr = colors::PaletteColorToFarColor(COL_PANELTEXT);

	if (static_cast<size_t>(Position) < m_ListData.size())
	{
		int Pos = highlight::color::normal;

		if (m_CurFile == Position && IsFocused() && !m_ListData.empty())
		{
			Pos=m_ListData[Position].Selected? highlight::color::selected_current : highlight::color::normal_current;
		}
		else if (m_ListData[Position].Selected)
			Pos = highlight::color::selected;

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
	}

	return ColorAttr;
}

void FileList::SetShowColor(int Position, bool FileColor) const
{
	SetColor(GetShowColor(Position,FileColor));
}

static string size2str(ULONGLONG Size, int width, bool FloatStyle, bool short_mode)
{
	if (!short_mode)
	{
		return GroupDigits(Size);
	}

	if (FloatStyle) // float style
	{
		return trim(FileSizeToStr(Size, width, COLFLAGS_FLOATSIZE | COLFLAGS_SHOW_MULTIPLIER));
	}

	auto Str = str(Size);
	if (static_cast<int>(Str.size()) <= width)
		return Str;

	return trim(FileSizeToStr(Size, width, COLFLAGS_SHOW_MULTIPLIER));
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
			BoxText(BoxSymbols[DoubleLine?BS_B_H1V2:BS_B_H1V1]);
			ColumnPos++;
		}
	}
	if (m_SelFileCount)
	{
		auto strFormStr = size2str(SelFileSize, 6, false, false);
		auto strSelStr = format(msg(lng::MListFileSize), strFormStr, m_SelFileCount - m_SelDirCount, m_SelDirCount, m_SelFileCount);
		const auto BorderSize = 1;
		const auto MarginSize = 1;
		const auto AvailableWidth = static_cast<size_t>(std::max(0, ObjWidth() - BorderSize * 2 - MarginSize * 2));
		if (strSelStr.size() > AvailableWidth)
		{
			strFormStr = size2str(SelFileSize, 6, false, true);
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

	const auto calc_total_string = [this, Info](bool short_mode)
	{
		string strFreeSize, strTotalSize;
		auto strFormSize = size2str(TotalFileSize, 6, false, short_mode);
		if (Global->Opt->ShowPanelFree && (m_PanelMode != panel_mode::PLUGIN_PANEL || (Info.Flags & (OPIF_REALNAMES | OPIF_USEFREESIZE))))
			strFreeSize = (FreeDiskSize != static_cast<unsigned long long>(-1)) ? size2str(FreeDiskSize, 10, true, short_mode) : L"?"sv;

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

	auto TotalStr = calc_total_string(!Global->Opt->ShowBytes);
	if (TotalStr.size() > AvailableWidth)
	{
		if (Global->Opt->ShowBytes)
			TotalStr = calc_total_string(true);
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

bool FileList::ConvertName(const string_view SrcName, string& strDest, const int MaxLength, const unsigned long long RightAlign, const int ShowStatus, const DWORD FileAttr) const
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
	        (Extension = PointToExt(SrcName)).size() > 1 && Extension.size() != SrcName.size() &&
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
			for (auto& i: m_ViewSettings.PanelColumns)
			{
				if (i.type == column_type::name)
				{
					if (m_CachedOpenPanelInfo.Flags & OPIF_SHOWNAMESONLY)
						i.type_flags |= COLFLAGS_NAMEONLY;

					if (m_CachedOpenPanelInfo.Flags & OPIF_SHOWRIGHTALIGNNAMES)
						i.type_flags |= COLFLAGS_RIGHTALIGN;
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

	for (int StripeStride = 1; StripeStride <= ColumnsSize / 2; StripeStride++)
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
			SetShowColor(J);
			GotoXY(m_Where.left + 1, I);
		}

		int StatusLine=FALSE;
		int Level = 1;

		for (size_t K=0; K<ColumnCount; K++)
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
					BoxText(CurX - 1 == m_Where.right? BoxSymbols[BS_V2] : L' ');
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

					if (ColumnNumber<m_ListData[ListPos].CustomColumnNumber)
						ColumnData = m_ListData[ListPos].CustomColumnData[ColumnNumber];

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
						int Length = static_cast<int>(wcslen(ColumnData));
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
								const auto Mark = m_ListData[ListPos].Selected? L"\x221A "sv : ViewFlags & COLFLAGS_MARK_DYNAMIC ? L""sv : L"  "sv;
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
								const auto ExtPtr = PointToExt(Name);
								if (!ExtPtr.empty())
								{
									Name.remove_suffix(ExtPtr.size());
								}
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
								ExtPtr = PointToExt(Name);
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
							os::chrono::time_point* FileTime;

							switch (ColumnType)
							{
								case column_type::date_creation:
									FileTime=&m_ListData[ListPos].CreationTime;
									break;
								case column_type::date_access:
									FileTime=&m_ListData[ListPos].LastAccessTime;
									break;
								case column_type::date_change:
									FileTime=&m_ListData[ListPos].ChangeTime;
									break;
								case column_type::date:
								case column_type::time:
								case column_type::date_write:
								default:
									FileTime=&m_ListData[ListPos].LastWriteTime;
									break;
							}

							Text(FormatStr_DateTime(*FileTime, ColumnType, Columns[K].type_flags, ColumnWidth));
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
								int Length = static_cast<int>(wcslen(NullToEmpty(m_ListData[ListPos].DizText)));
								if (Length>ColumnWidth)
								{
									CurLeftPos = std::min(LeftPos, Length-ColumnWidth);
									MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
								}
							}

							string strDizText=m_ListData[ListPos].DizText? m_ListData[ListPos].DizText+CurLeftPos : L""s;
							size_t pos = strDizText.find(L'\4');
							if (pos != string::npos)
								strDizText.resize(pos);

							Text(fit_to_left(strDizText, ColumnWidth));
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
							else if(!Owner.empty() && IsSlash(Owner.front()))
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
					BoxText(CurX + ColumnWidth == m_Where.right? BoxSymbols[BS_V2] : L' ');
				else
					BoxText(ShowStatus? L' ' : BoxSymbols[Level == m_ColumnsInStripe? BS_V2 : BS_V1]);

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
	return (Global->Opt->ViewSettings[Mode].Flags&PVS_FULLSCREEN)==PVS_FULLSCREEN;
}


bool FileList::IsDizDisplayed() const
{
	return IsColumnDisplayed(column_type::description);
}

bool FileList::IsColumnDisplayed(function_ref<bool(const column&)> const Compare) const
{
	return std::any_of(ALL_CONST_RANGE(m_ViewSettings.PanelColumns), Compare) ||
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

content_data_ptr FileList::GetContentData(const string& Item) const
{
	content_data_ptr Result;
	if (!m_ContentPlugins.empty())
	{
		Result = std::make_unique<decltype(Result)::element_type>();
		Global->CtrlObject->Plugins->GetContentData(m_ContentPlugins, Item, m_ContentNamesPtrs, m_ContentValues, *Result.get());
	}
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
