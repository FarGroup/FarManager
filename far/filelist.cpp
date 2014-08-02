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

#include "headers.hpp"
#pragma hdrstop

#include "filelist.hpp"
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
#include "savescr.hpp"
#include "fileview.hpp"
#include "copy.hpp"
#include "history.hpp"
#include "qview.hpp"
#include "rdrwdsk.hpp"
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
#include "shortcuts.hpp"
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
#include "constitle.hpp"
#include "elevation.hpp"
#include "FarGuid.hpp"
#include "DlgGuid.hpp"
#include "plugins.hpp"
#include "language.hpp"
#include "TaskBar.hpp"
#include "fileowner.hpp"
#include "colormix.hpp"

static int ListSortGroups,ListSelectedFirst,ListDirectoriesFirst;
static int ListSortMode;
static bool RevertSorting;
static int ListPanelMode,ListNumericSort,ListCaseSensitiveSort;
static PluginHandle* hSortPlugin;

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

static void FileListToSortingPanelItem(const FileListItem *fi, SortingPanelItem *pi)
{
	pi->FileName = fi->strName.data();               //! CHANGED
	pi->AlternateFileName = fi->strShortName.data(); //! CHANGED
	pi->FileSize=fi->FileSize;
	pi->AllocationSize=fi->AllocationSize;
	pi->FileAttributes=fi->FileAttr;
	pi->LastWriteTime=fi->WriteTime;
	pi->CreationTime=fi->CreationTime;
	pi->LastAccessTime=fi->AccessTime;
	pi->ChangeTime=fi->ChangeTime;
	pi->NumberOfLinks=fi->NumberOfLinks;
	pi->Flags=fi->UserFlags;

	if (fi->Selected)
		pi->Flags|=PPIF_SELECTED;

	pi->CustomColumnData=fi->CustomColumnData;
	pi->CustomColumnNumber=fi->CustomColumnNumber;
	pi->Description=fi->DizText; //BUGBUG???

	pi->UserData.Data=fi->UserData;
	pi->UserData.FreeData=fi->Callback;

	pi->CRC32=fi->CRC32;
	pi->Position=fi->Position;                        //! CHANGED
	pi->SortGroup=fi->SortGroup - DEFAULT_SORT_GROUP; //! CHANGED
	pi->Owner = EmptyToNull(fi->strOwner.data());
	pi->NumberOfStreams=fi->NumberOfStreams;
	pi->StreamsSize=fi->StreamsSize;
}

struct CustomSort
{
	const FileListItem  *Items;
	size_t               ItemsCount;
	size_t               ItemSize;
	decltype(FileListToSortingPanelItem)* FileListToPluginItem;
	int                  ListSortGroups;
	int                  ListSelectedFirst;
	int                  ListDirectoriesFirst;
	int                  ListSortMode;
	int                  RevertSorting;
	int                  ListNumericSort;
	int                  ListCaseSensitiveSort;
	HANDLE               hSortPlugin;
};

};

struct FileList::PrevDataItem
{
	PrevDataItem(const string& rhsPrevName, std::vector<FileListItem>&& rhsPrevListData, int rhsPrevTopFile):
		strPrevName(rhsPrevName),
		PrevTopFile(rhsPrevTopFile)
	{
		PrevListData.swap(rhsPrevListData);
	}

	PrevDataItem(PrevDataItem&& rhs): PrevTopFile() { *this = std::move(rhs); }

	MOVE_OPERATOR_BY_SWAP(PrevDataItem);

	void swap(PrevDataItem& rhs) noexcept
	{
		strPrevName.swap(rhs.strPrevName);
		PrevListData.swap(rhs.PrevListData);
		std::swap(PrevTopFile, rhs.PrevTopFile);
	}

	string strPrevName;
	std::vector<FileListItem> PrevListData;
	int PrevTopFile;
};

STD_SWAP_SPEC(FileList::PrevDataItem);

FileListItem::~FileListItem()
{
	if (CustomColumnNumber && CustomColumnData)
	{
		for (size_t i = 0; i < CustomColumnNumber; ++i)
			delete[] CustomColumnData[i];
		delete[] CustomColumnData;
	}

	if (DeleteDiz)
		delete[] DizText;

}


FileList::FileList():
	Filter(nullptr),
	DizRead(FALSE),
	hPlugin(nullptr),
	UpperFolderTopFile(0),
	LastCurFile(-1),
	ReturnCurrentFile(FALSE),
	SelFileCount(0),
	GetSelPosition(0), LastSelPosition(-1),
	TotalFileCount(0),
	SelFileSize(0),
	TotalFileSize(0),
	FreeDiskSize(-1),
	LastUpdateTime(0),
	Height(0),
	LeftPos(0),
	ShiftSelection(-1),
	MouseSelection(0),
	SelectedFirst(0),
	empty(TRUE),
	AccessTimeUpdateRequired(FALSE),
	UpdateRequired(FALSE),
	UpdateRequiredMode(0),
	UpdateDisabled(0),
	SortGroupsRead(FALSE),
	InternalProcessKey(FALSE),
	CacheSelIndex(-1),
	CacheSelPos(0),
	CacheSelClearIndex(-1),
	CacheSelClearPos(0),
	CustomSortIndicator()
{
	_OT(SysLog(L"[%p] FileList::FileList()", this));
	{
		const wchar_t *data=MSG(MPanelBracketsForLongName);

		if (wcslen(data) > 1)
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
	Type=FILE_PANEL;
	api::GetCurrentDirectory(strCurDir);
	strOriginalCurDir = strCurDir;
	CurTopFile=CurFile=0;
	ShowShortNames=0;
	SortMode=BY_NAME;
	ReverseSortOrder = false;
	SortGroups=0;
	ViewMode=VIEW_3;
	ViewSettings = Global->Opt->ViewSettings[ViewMode].clone();
	NumericSort=0;
	CaseSensitiveSort=0;
	DirectoriesFirst=1;
	Columns=PreparePanelView(&ViewSettings);
	PluginCommand=-1;
}


FileList::~FileList()
{
	_OT(SysLog(L"[%p] FileList::~FileList()", this));
	StopFSWatcher();

	ClearAllItem();

	DeleteListData(ListData);

	if (PanelMode==PLUGIN_PANEL)
		while (PopPlugin(FALSE))
			;
}


void FileList::DeleteListData(std::vector<FileListItem>& ListData)
{
	std::for_each(CONST_RANGE(ListData, i)
	{
		if (this->PanelMode == PLUGIN_PANEL && i.Callback)
		{
			FarPanelItemFreeInfo info = {sizeof(FarPanelItemFreeInfo), hPlugin};
			i.Callback(i.UserData, &info);
		}
	});

	ListData.clear();
}

void FileList::ToBegin()
{
	CurFile = 0;
	ShowFileList(TRUE);
}


void FileList::ToEnd()
{
	CurFile = static_cast<int>(ListData.size() - 1);
	ShowFileList(TRUE);
}

void FileList::MoveCursor(int offset)
{
	CurFile = std::min(std::max(0, CurFile + offset), static_cast<int>(ListData.size() - 1));
	ShowFileList(TRUE);
}

void FileList::Scroll(int offset)
{
	CurTopFile += offset;
	MoveCursor(offset);
}

void FileList::CorrectPosition()
{
	if (ListData.empty())
	{
		CurFile=CurTopFile=0;
		return;
	}

	if (CurTopFile+Columns*Height > static_cast<int>(ListData.size()))
		CurTopFile = static_cast<int>(ListData.size() - Columns * Height);

	if (CurFile<0)
		CurFile=0;

	if (CurFile > static_cast<int>(ListData.size() - 1))
		CurFile = static_cast<int>(ListData.size() - 1);

	if (CurTopFile<0)
		CurTopFile=0;

	if (CurTopFile > static_cast<int>(ListData.size() - 1))
		CurTopFile = static_cast<int>(ListData.size() - 1);

	if (CurFile<CurTopFile)
		CurTopFile=CurFile;

	if (CurFile>CurTopFile+Columns*Height-1)
		CurTopFile=CurFile-Columns*Height+1;
}

static struct list_less
{
	bool operator()(const FileListItem& a, const FileListItem& b) const
	{
		auto less_opt = [](bool less)
		{
			return RevertSorting ? !less : less;
		};

		int RetCode;
		bool UseReverseNameSort = false;
		const wchar_t *Ext1=nullptr,*Ext2=nullptr;

		if (a.strName == L".." && (a.strShortName.empty() || a.strShortName == L".."))
			return true;

		if (b.strName == L".." && (b.strShortName.empty() || b.strShortName == L".."))
			return false;

		if (ListSortMode==UNSORTED)
		{
			if (ListSelectedFirst && a.Selected != b.Selected)
				return a.Selected > b.Selected;
			return less_opt(a.Position < b.Position);
		}

		if (ListDirectoriesFirst)
		{
			if ((a.FileAttr & FILE_ATTRIBUTE_DIRECTORY) < (b.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				return false;

			if ((a.FileAttr & FILE_ATTRIBUTE_DIRECTORY) > (b.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				return true;
		}

		if (ListSelectedFirst && a.Selected != b.Selected)
			return a.Selected > b.Selected;

		if (ListSortGroups && (ListSortMode==BY_NAME || ListSortMode==BY_EXT || ListSortMode==BY_FULLNAME) && a.SortGroup != b.SortGroup)
			return a.SortGroup < b.SortGroup;

		if (hSortPlugin)
		{
			PluginPanelItem pi1, pi2;
			FileList::FileListToPluginItem(a, &pi1);
			FileList::FileListToPluginItem(b, &pi2);
			pi1.Flags = a.Selected? PPIF_SELECTED : 0;
			pi2.Flags = b.Selected? PPIF_SELECTED : 0;
			RetCode=Global->CtrlObject->Plugins->Compare(hSortPlugin,&pi1,&pi2,ListSortMode+(SM_UNSORTED-UNSORTED));
			FreePluginPanelItem(pi1);
			FreePluginPanelItem(pi2);
			if (RetCode!=-2 && RetCode)
				return less_opt(RetCode < 0);
		}

		__int64 RetCode64;

		auto CompareTime = [&a, &b](const FILETIME FileListItem::*time)
		{
			return CompareFileTime(a.*time, b.*time);
		};

		switch (ListSortMode)
		{
			case BY_NAME:
				UseReverseNameSort = true;
				break;

			case BY_EXT:
				UseReverseNameSort = true;

				{
					auto GetExt = [](const FileListItem& i)
					{
						return !Global->Opt->SortFolderExt && (i.FileAttr & FILE_ATTRIBUTE_DIRECTORY) ? i.strName.data() + i.strName.size() : PointToExt(i.strName);
					};

					Ext1 = GetExt(a);
					Ext2 = GetExt(b);
				}

				if (!*Ext1)
				{
					if (!*Ext2)
						break;
					else
						return less_opt(true);
				}
				if (!*Ext2)
					return less_opt(false);
				{
					auto Comparer = ListNumericSort ? (ListCaseSensitiveSort ? NumStrCmpC : NumStrCmpI) : (ListCaseSensitiveSort ? StrCmpC : ::StrCmpI);
					RetCode = Comparer(Ext1 + 1, Ext2 + 1);
				}
				if (RetCode)
					return less_opt(RetCode < 0);
				break;

			case BY_MTIME:
				if ((RetCode64 = CompareTime(&FileListItem::WriteTime)))
					return less_opt(RetCode64 < 0);
				break;

			case BY_CTIME:
				if ((RetCode64 = CompareTime(&FileListItem::CreationTime)))
					return less_opt(RetCode64 < 0);
				break;

			case BY_ATIME:
				if ((RetCode64 = CompareTime(&FileListItem::AccessTime)))
					return less_opt(RetCode64 < 0);
				break;

			case BY_CHTIME:
				if ((RetCode64 = CompareTime(&FileListItem::ChangeTime)))
					return less_opt(RetCode64 < 0);
				break;

			case BY_SIZE:
				if (a.FileSize != b.FileSize)
					return less_opt(a.FileSize < b.FileSize);
				break;

			case BY_DIZ:
				if (!a.DizText)
				{
					if (!b.DizText)
						break;
					else
						return less_opt(false);
				}

				if (!b.DizText)
					return less_opt(true);

				{
					auto Comparer = ListNumericSort ? (ListCaseSensitiveSort ? NumStrCmpC : NumStrCmpI) : (ListCaseSensitiveSort ? StrCmpC : ::StrCmpI);
					RetCode = Comparer(a.DizText, b.DizText);
				}
				if (RetCode)
					return less_opt(RetCode < 0);
				break;

			case BY_OWNER:
				RetCode = StrCmpI(a.strOwner, b.strOwner);
				if (RetCode)
					return less_opt(RetCode < 0);
				break;

			case BY_COMPRESSEDSIZE:
				if (a.AllocationSize != b.AllocationSize)
					return less_opt(a.AllocationSize < b.AllocationSize);
				break;

			case BY_NUMLINKS:
				if (a.NumberOfLinks != b.NumberOfLinks)
					return less_opt(a.NumberOfLinks < b.NumberOfLinks);
				break;

			case BY_NUMSTREAMS:
				if (a.NumberOfStreams != b.NumberOfStreams)
					return less_opt(a.NumberOfStreams < b.NumberOfStreams);
				break;

			case BY_STREAMSSIZE:
				if (a.StreamsSize != b.StreamsSize)
					return less_opt(a.StreamsSize < b.StreamsSize);
				break;

			case BY_FULLNAME:
				UseReverseNameSort = true;
				if (ListNumericSort)
				{
					auto Path1 = a.strName.data();
					auto Path2 = b.strName.data();
					auto Name1 = PointToName(a.strName);
					auto Name2 = PointToName(b.strName);
					auto NameComparer = ListCaseSensitiveSort ? StrCmpNNC : StrCmpNNI;
					auto NumPathComparer = ListCaseSensitiveSort ? NumStrCmpC : NumStrCmpI;
					auto PathComparer = ListCaseSensitiveSort ? StrCmpC : ::StrCmpI;

					if (!NameComparer(Path1, Name1 - Path1, Path2, Name2 - Path2))
						RetCode = NumPathComparer(Name1, Name2);
					else
						RetCode = PathComparer(Path1, Path2);
				}
				else
				{
					auto Comparer = ListCaseSensitiveSort ? StrCmpC : ::StrCmpI;
					RetCode = Comparer(a.strName.data(), b.strName.data());
				}
				if (RetCode)
					return less_opt(RetCode < 0);
				break;

			case BY_CUSTOMDATA:
				if (a.strCustomData.empty())
				{
					if (b.strCustomData.empty())
						break;
					else
						return less_opt(false);
				}

				if (b.strCustomData.empty())
					return less_opt(true);

				{
					auto Comparer = ListNumericSort? (ListCaseSensitiveSort? NumStrCmpC : NumStrCmpI) : (ListCaseSensitiveSort? StrCmpC : ::StrCmpI);
					RetCode = Comparer(a.strCustomData.data(), b.strCustomData.data());
				}
				if (RetCode)
					return less_opt(RetCode < 0);
				break;
			}

		int NameCmp=0;

		if (!Global->Opt->SortFolderExt && (a.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			Ext1=a.strName.data()+a.strName.size();
		}
		else
		{
			if (!Ext1) Ext1=PointToExt(a.strName);
		}

		if (!Global->Opt->SortFolderExt && (b.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			Ext2=b.strName.data()+b.strName.size();
		}
		else
		{
			if (!Ext2) Ext2=PointToExt(b.strName);
		}

		const wchar_t *Name1=PointToName(a.strName);
		const wchar_t *Name2=PointToName(b.strName);

		{
			auto Comparer = ListNumericSort? (ListCaseSensitiveSort? NumStrCmpNC : NumStrCmpNI) : (ListCaseSensitiveSort? StrCmpNNC : StrCmpNNI);
			NameCmp = Comparer(Name1, Ext1 - Name1, Name2, Ext2 - Name2);
		}

		if (!NameCmp)
		{
			auto Comparer = ListNumericSort? (ListCaseSensitiveSort? NumStrCmpC : NumStrCmpI) : (ListCaseSensitiveSort? StrCmpC : ::StrCmpI);
			NameCmp = Comparer(Ext1, Ext2);
		}

		if (!NameCmp)
			NameCmp = a.Position < b.Position? -1 : 1;

		return UseReverseNameSort? less_opt(NameCmp < 0) : NameCmp < 0;
	}
}
ListLess;

void FileList::SortFileList(int KeepPosition)
{
	if (!ListData.empty())
	{
		string strCurName;

		if (SortMode==BY_DIZ)
			ReadDiz();

		ListSortMode=SortMode;
		RevertSorting = ReverseSortOrder;
		ListSortGroups=SortGroups;
		ListSelectedFirst=SelectedFirst;
		ListDirectoriesFirst=DirectoriesFirst;
		ListPanelMode=PanelMode;
		ListNumericSort=NumericSort;
		ListCaseSensitiveSort=CaseSensitiveSort;

		if (KeepPosition)
		{
			assert(CurFile < static_cast<int>(ListData.size()));
			strCurName = ListData[CurFile].strName;
		}

		hSortPlugin=(PanelMode==PLUGIN_PANEL && hPlugin && hPlugin->pPlugin->HasCompare()) ? hPlugin:nullptr;

		// ЭТО ЕСТЬ УЗКОЕ МЕСТО ДЛЯ СКОРОСТНЫХ ХАРАКТЕРИСТИК Far Manager
		// при считывании дирректории

		if (SortMode < SORTMODE_COUNT)
		{
			std::sort(ALL_RANGE(ListData), ListLess);
		}
		else
		{
			custom_sort::CustomSort cs;
			cs.Items = ListData.data();
			cs.ItemsCount = ListData.size();
			cs.ItemSize = sizeof(FileListItem);
			cs.FileListToPluginItem = custom_sort::FileListToSortingPanelItem;
			cs.ListSortGroups = ListSortGroups;
			cs.ListSelectedFirst = ListSelectedFirst;
			cs.ListDirectoriesFirst = ListDirectoriesFirst;
			cs.ListSortMode = SortMode;
			cs.RevertSorting = RevertSorting?1:0;
			cs.ListNumericSort = ListNumericSort;
			cs.ListCaseSensitiveSort = ListCaseSensitiveSort;
			cs.hSortPlugin = hSortPlugin;

			FarMacroValue values[]={&cs};
			FarMacroCall fmc={sizeof(FarMacroCall),ARRAYSIZE(values),values,nullptr,nullptr};
			OpenMacroPluginInfo info={MCT_PANELSORT,&fmc};
			void *ptr;
			if (Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Luamacro.Id, OPEN_LUAMACRO, &info, &ptr) && ptr)
			{
				CustomSortIndicator[0] = info.Ret.Values[0].String[0];
				CustomSortIndicator[1] = info.Ret.Values[0].String[1];
			}
			else
			{
				SetSortMode(BY_NAME); // recursive call
				return;
			}
		}

		if (KeepPosition)
			GoToFile(strCurName);
	}
}

void FileList::SetFocus()
{
	Panel::SetFocus();

	/* $ 07.04.2002 KM
	  ! Рисуем заголовок консоли фара только тогда, когда
	    не идёт процесс перерисовки всех фреймов. В данном
	    случае над панелями висит диалог и незачем выводить
	    панельный заголовок.
	*/
	if (!Global->IsRedrawFramesInProcess)
		SetTitle();
}

int FileList::SendKeyToPlugin(DWORD Key,bool Pred)
{
	_ALGO(CleverSysLog clv(L"FileList::SendKeyToPlugin()"));
	_ALGO(SysLog(L"Key=%s Pred=%d",_FARKEY_ToName(Key),Pred));

	if (PanelMode==PLUGIN_PANEL &&
	        (Global->CtrlObject->Macro.IsRecording() == MACROSTATE_RECORDING_COMMON || Global->CtrlObject->Macro.IsExecuting() == MACROSTATE_EXECUTING_COMMON || Global->CtrlObject->Macro.GetCurRecord() == MACROSTATE_NOMACRO)
	   )
	{
		_ALGO(SysLog(L"call Plugins.ProcessKey() {"));
		INPUT_RECORD rec;
		KeyToInputRecord(Key,&rec);
		int ProcessCode=Global->CtrlObject->Plugins->ProcessKey(hPlugin,&rec,Pred);
		_ALGO(SysLog(L"} ProcessCode=%d",ProcessCode));
		ProcessPluginCommand();

		if (ProcessCode)
			return TRUE;
	}

	return FALSE;
}

bool FileList::GetPluginInfo(PluginInfo *PInfo)
{
	if (GetMode() == PLUGIN_PANEL && hPlugin && hPlugin->pPlugin)
	{
		PInfo->StructSize=sizeof(PluginInfo);
		return hPlugin->pPlugin->GetPluginInfo(PInfo) != 0;
	}
	return false;
}

__int64 FileList::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_C_ROOTFOLDER:
		{
			if (PanelMode==PLUGIN_PANEL)
			{
				OpenPanelInfo Info;
				Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
				return !Info.CurDir || !*Info.CurDir;
			}
			else
			{
				if (!IsRootPath(strCurDir))
				{
					string strDriveRoot;
					GetPathRoot(strCurDir, strDriveRoot);
					return !StrCmpI(strCurDir, strDriveRoot);
				}

				return 1;
			}
		}
		case MCODE_C_EOF:
			return CurFile == static_cast<int>(ListData.size() - 1);
		case MCODE_C_BOF:
			return !CurFile;
		case MCODE_C_SELECTED:
			return GetRealSelCount() > 1;
		case MCODE_V_ITEMCOUNT:
			return ListData.size();
		case MCODE_V_CURPOS:
			return CurFile + 1;
		case MCODE_C_APANEL_FILTER:
			return Filter && Filter->IsEnabledOnPanel();

		case MCODE_V_APANEL_PREFIX:           // APanel.Prefix
		case MCODE_V_PPANEL_PREFIX:           // PPanel.Prefix
		{
			PluginInfo *PInfo=(PluginInfo *)vParam;
			if (GetMode() == PLUGIN_PANEL && hPlugin && hPlugin->pPlugin)
				return hPlugin->pPlugin->GetPluginInfo(PInfo)?1:0;
			return 0;
		}

		case MCODE_V_APANEL_FORMAT:           // APanel.Format
		case MCODE_V_PPANEL_FORMAT:           // PPanel.Format
		{
			OpenPanelInfo *PInfo=(OpenPanelInfo *)vParam;
			if (GetMode() == PLUGIN_PANEL && hPlugin)
			{
				Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,PInfo);
				return 1;
			}
			return 0;
		}

		case MCODE_V_APANEL_PATH0:
		case MCODE_V_PPANEL_PATH0:
		{
			if (PluginsList.empty())
				return 0;
			*(string *)vParam = PluginsList.back().m_PrevOriginalCurDir;
			return 1;
		}

		case MCODE_F_PANEL_SELECT:
		{
			// vParam = MacroPanelSelect*, iParam = 0
			__int64 Result=-1;
			MacroPanelSelect *mps=(MacroPanelSelect *)vParam;

			if (ListData.empty())
				return Result;

			if (mps->Mode == 1 && static_cast<size_t>(mps->Index) >= ListData.size())
				return Result;

			std::vector<string> itemsList;

			if (mps->Action != 3)
			{
				if (mps->Mode == 2)
				{
					itemsList = split_to_vector::get(mps->Item->asString(), STLF_UNIQUE, L"\r\n");
					if (itemsList.empty())
						return Result;
				}

				SaveSelection();
			}

			// mps->ActionFlags
			switch (mps->Action)
			{
				case 0:  // снять выделение
				{
					switch(mps->Mode)
					{
						case 0: // снять со всего?
							Result=GetRealSelCount();
							ClearSelection();
							break;
						case 1: // по индексу?
							Result=1;
							Select(ListData[mps->Index], FALSE);
							break;
						case 2: // набор строк
						{
							Result=0;
							std::for_each(CONST_RANGE(itemsList, i)
							{
								int Pos = this->FindFile(PointToName(i), TRUE);
								if (Pos != -1)
								{
									this->Select(ListData[Pos],FALSE);
									Result++;
								}
							});
							break;
						}
						case 3: // масками файлов, разделенных запятыми
							Result=SelectFiles(SELECT_REMOVEMASK,mps->Item->asString().data());
							break;
					}
					break;
				}

				case 1:  // добавить выделение
				{
					switch(mps->Mode)
					{
						case 0: // выделить все?
							std::for_each(RANGE(ListData, i)
							{
								this->Select(i, TRUE);
							});
							Result=GetRealSelCount();
							break;
						case 1: // по индексу?
							Result=1;
							Select(ListData[mps->Index], TRUE);
							break;
						case 2: // набор строк через CRLF
						{
							Result=0;
							std::for_each(CONST_RANGE(itemsList, i)
							{
								int Pos = this->FindFile(PointToName(i), TRUE);
								if (Pos != -1)
								{
									this->Select(ListData[Pos], TRUE);
									Result++;
								}
							});
							break;
						}
						case 3: // масками файлов, разделенных запятыми
							Result=SelectFiles(SELECT_ADDMASK,mps->Item->asString().data());
							break;
					}
					break;
				}

				case 2:  // инвертировать выделение
				{
					switch(mps->Mode)
					{
						case 0: // инвертировать все?
							std::for_each(RANGE(ListData, i)
							{
								this->Select(i, !i.Selected);
							});
							Result=GetRealSelCount();
							break;
						case 1: // по индексу?
							Result=1;
							Select(ListData[mps->Index], !ListData[mps->Index].Selected);
							break;
						case 2: // набор строк через CRLF
						{
							Result=0;
							std::for_each(CONST_RANGE(itemsList, i)
							{
								int Pos = this->FindFile(PointToName(i), TRUE);
								if (Pos != -1)
								{
									this->Select(ListData[Pos], !ListData[Pos].Selected);
									Result++;
								}
							});
							break;
						}
						case 3: // масками файлов, разделенных запятыми
							Result=SelectFiles(SELECT_INVERTMASK,mps->Item->asString().data());
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
					SortFileList(TRUE);
				Redraw();
			}

			return Result;
		}
	}

	return 0;
}

int FileList::ProcessKey(const Manager::Key& Key)
{
	int LocalKey=Key.FarKey;
	Global->Elevation->ResetApprove();

	FileListItem *CurPtr=nullptr;
	int N;
	int CmdLength=Global->CtrlObject->CmdLine->GetLength();

	if (IsVisible())
	{
		if (!InternalProcessKey)
			if ((!(LocalKey==KEY_ENTER||LocalKey==KEY_NUMENTER) && !(LocalKey==KEY_SHIFTENTER||LocalKey==KEY_SHIFTNUMENTER)) || !CmdLength)
				if (SendKeyToPlugin(LocalKey))
					return TRUE;
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
				return FALSE;
		}
	}

	if (!IntKeyState.ShiftPressed && ShiftSelection!=-1)
	{
		if (SelectedFirst)
		{
			SortFileList(TRUE);
			ShowFileList(TRUE);
		}

		ShiftSelection=-1;
	}

	if ( !InternalProcessKey )
	{
		// Create a folder shortcut?
		if ((LocalKey>=KEY_CTRLSHIFT0 && LocalKey<=KEY_CTRLSHIFT9) ||
			(LocalKey>=KEY_RCTRLSHIFT0 && LocalKey<=KEY_RCTRLSHIFT9) ||
			(LocalKey>=KEY_CTRLALT0 && LocalKey<=KEY_CTRLALT9) ||
			(LocalKey>=KEY_RCTRLRALT0 && LocalKey<=KEY_RCTRLRALT9) ||
			(LocalKey>=KEY_CTRLRALT0 && LocalKey<=KEY_CTRLRALT9) ||
			(LocalKey>=KEY_RCTRLALT0 && LocalKey<=KEY_RCTRLALT9)
		)
		{
			bool Add = (LocalKey&KEY_SHIFT) == KEY_SHIFT;
			SaveShortcutFolder((LocalKey&(~(KEY_CTRL|KEY_RCTRL|KEY_ALT|KEY_RALT|KEY_SHIFT|KEY_RSHIFT)))-'0', Add);
			return TRUE;
		}
		// Jump to a folder shortcut?
		else if (LocalKey>=KEY_RCTRL0 && LocalKey<=KEY_RCTRL9)
		{
			ExecShortcutFolder(LocalKey-KEY_RCTRL0);
			return TRUE;
		}
	}

	/* $ 27.08.2002 SVS
	    [*] В панели с одной колонкой Shift-Left/Right аналогично нажатию
	        Shift-PgUp/PgDn.
	*/
	if (Columns==1 && !CmdLength)
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
				Global->CtrlObject->Cp()->GetAnotherPanel(this)->StartFSWatcher(true);
			}
			break;

		case KEY_KILLFOCUS:
			if (Global->Opt->SmartFolderMonitor)
			{
				StopFSWatcher();
				Global->CtrlObject->Cp()->GetAnotherPanel(this)->StopFSWatcher();
			}
			break;

		case KEY_F1:
		{
			_ALGO(CleverSysLog clv(L"F1"));
			_ALGO(SysLog(L"%s, FileCount=%d",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

			if (PanelMode==PLUGIN_PANEL && PluginPanelHelp(hPlugin))
				return TRUE;

			return FALSE;
		}
		case KEY_ALTSHIFTF9:
		case KEY_RALTSHIFTF9:
		{
			if (PanelMode==PLUGIN_PANEL)
				Global->CtrlObject->Plugins->ConfigureCurrent(hPlugin->pPlugin, FarGuid);
			else
				Global->CtrlObject->Plugins->Configure();

			return TRUE;
		}
		case KEY_SHIFTSUBTRACT:
		{
			SaveSelection();
			ClearSelection();
			Redraw();
			return TRUE;
		}
		case KEY_SHIFTADD:
		{
			SaveSelection();
			{
				std::for_each(RANGE(ListData, i)
				{
					if (!(i.FileAttr & FILE_ATTRIBUTE_DIRECTORY) || Global->Opt->SelectFolders)
						this->Select(i, TRUE);
				});
			}

			if (SelectedFirst)
				SortFileList(TRUE);

			Redraw();
			return TRUE;
		}
		case KEY_ADD:
			SelectFiles(SELECT_ADD);
			return TRUE;
		case KEY_SUBTRACT:
			SelectFiles(SELECT_REMOVE);
			return TRUE;
		case KEY_CTRLADD:
		case KEY_RCTRLADD:
			SelectFiles(SELECT_ADDEXT);
			return TRUE;
		case KEY_CTRLSUBTRACT:
		case KEY_RCTRLSUBTRACT:
			SelectFiles(SELECT_REMOVEEXT);
			return TRUE;
		case KEY_ALTADD:
		case KEY_RALTADD:
			SelectFiles(SELECT_ADDNAME);
			return TRUE;
		case KEY_ALTSUBTRACT:
		case KEY_RALTSUBTRACT:
			SelectFiles(SELECT_REMOVENAME);
			return TRUE;
		case KEY_MULTIPLY:
			SelectFiles(SELECT_INVERT);
			return TRUE;
		case KEY_CTRLMULTIPLY:
		case KEY_RCTRLMULTIPLY:
			SelectFiles(SELECT_INVERTALL);
			return TRUE;
		case KEY_ALTLEFT:     // Прокрутка длинных имен и описаний
		case KEY_RALTLEFT:
		case KEY_ALTHOME:     // Прокрутка длинных имен и описаний - в начало
		case KEY_RALTHOME:
			LeftPos=(LocalKey == KEY_ALTHOME || LocalKey == KEY_RALTHOME)?-0x7fff:LeftPos-1;
			Redraw();
			return TRUE;
		case KEY_ALTRIGHT:    // Прокрутка длинных имен и описаний
		case KEY_RALTRIGHT:
		case KEY_ALTEND:     // Прокрутка длинных имен и описаний - в конец
		case KEY_RALTEND:
			LeftPos=(LocalKey == KEY_ALTEND || LocalKey == KEY_RALTEND)?0x7fff:LeftPos+1;
			Redraw();
			return TRUE;
		case KEY_CTRLINS:      case KEY_CTRLNUMPAD0:
		case KEY_RCTRLINS:     case KEY_RCTRLNUMPAD0:

			if (CmdLength>0)
				return FALSE;

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
			return TRUE;

		case KEY_CTRLC: // hdrop  copy
		case KEY_RCTRLC:
			CopyFiles();
			return TRUE;
		#if 0
		case KEY_CTRLX: // hdrop cut !!!NEED KEY!!!
		case KEY_RCTRLX:
			CopyFiles(true);
			return TRUE;
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

			Panel *SrcPanel = Global->CtrlObject->Cp()->GetAnotherPanel(Global->CtrlObject->Cp()->ActivePanel);
			bool OldState = SrcPanel->IsVisible()!=0;
			SrcPanel->SetVisible(1);
			SrcPanel->ProcessKey(Manager::Key(NewKey));
			SrcPanel->SetVisible(OldState);
			SetCurPath();
			return TRUE;
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
			if (!ListData.empty() && SetCurPath())
			{
				string strFileName;

				if (LocalKey==KEY_CTRLSHIFTENTER || LocalKey==KEY_RCTRLSHIFTENTER || LocalKey==KEY_CTRLSHIFTNUMENTER || LocalKey==KEY_RCTRLSHIFTNUMENTER)
				{
					_MakePath1(LocalKey,strFileName, L" ");
				}
				else
				{
					int CurrentPath=FALSE;
					assert(CurFile < static_cast<int>(ListData.size()));
					CurPtr = &ListData[CurFile];

					if (ShowShortNames && !CurPtr->strShortName.empty())
						strFileName = CurPtr->strShortName;
					else
						strFileName = CurPtr->strName;

					if (TestParentFolderName(strFileName))
					{
						if (PanelMode==PLUGIN_PANEL)
							strFileName.clear();
						else
							strFileName.resize(1); // "."

						if (!(LocalKey==KEY_CTRLALTF || LocalKey==KEY_RCTRLRALTF || LocalKey==KEY_CTRLRALTF || LocalKey==KEY_RCTRLALTF))
							LocalKey=KEY_CTRLF;

						CurrentPath=TRUE;
					}

					if (LocalKey==KEY_CTRLF || LocalKey==KEY_RCTRLF || LocalKey==KEY_CTRLALTF || LocalKey==KEY_RCTRLRALTF || LocalKey==KEY_CTRLRALTF || LocalKey==KEY_RCTRLALTF)
					{
						OpenPanelInfo Info={};

						if (PanelMode==PLUGIN_PANEL)
						{
							Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
						}

						if (PanelMode!=PLUGIN_PANEL)
							CreateFullPathName(CurPtr->strName,CurPtr->strShortName,CurPtr->FileAttr, strFileName, LocalKey==KEY_CTRLALTF || LocalKey==KEY_RCTRLRALTF || LocalKey==KEY_CTRLRALTF || LocalKey==KEY_RCTRLALTF);
						else
						{
							string strFullName = NullToEmpty(Info.CurDir);

							if (Global->Opt->PanelCtrlFRule && (ViewSettings.Flags&PVS_FOLDERUPPERCASE))
								Upper(strFullName);

							if (!strFullName.empty())
								AddEndSlash(strFullName,0);

							if (Global->Opt->PanelCtrlFRule)
							{
								/* $ 13.10.2000 tran
								  по Ctrl-f имя должно отвечать условиям на панели */
								if ((ViewSettings.Flags&PVS_FILELOWERCASE) && !(CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
									Lower(strFileName);

								if ((ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE))
									if (!(CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY) && !IsCaseMixed(strFileName))
										Lower(strFileName);
							}

							strFullName += strFileName;
							strFileName = strFullName;
						}
					}

					if (CurrentPath)
						AddEndSlash(strFileName);

					// добавим первый префикс!
					if (PanelMode==PLUGIN_PANEL && Global->Opt->SubstPluginPrefix && !(LocalKey == KEY_CTRLENTER || LocalKey == KEY_RCTRLENTER || LocalKey == KEY_CTRLNUMENTER || LocalKey == KEY_RCTRLNUMENTER || LocalKey == KEY_CTRLJ || LocalKey == KEY_RCTRLJ))
					{
						string strPrefix;

						/* $ 19.11.2001 IS оптимизация по скорости :) */
						if (!AddPluginPrefix((FileList *)Global->CtrlObject->Cp()->ActivePanel,strPrefix).empty())
						{
							strPrefix += strFileName;
							strFileName = strPrefix;
						}
					}

					if (Global->Opt->QuotedName&QUOTEDNAME_INSERT)
						QuoteSpace(strFileName);

					strFileName += L" ";
				}

				Global->CtrlObject->CmdLine->InsertString(strFileName);
			}

			return TRUE;
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

			if (_MakePath1(LocalKey,strPanelDir, L""))
				Global->CtrlObject->CmdLine->InsertString(strPanelDir);

			return TRUE;
		}
		case KEY_CTRLA:
		case KEY_RCTRLA:
		{
			_ALGO(CleverSysLog clv(L"Ctrl-A"));

			if (!ListData.empty() && SetCurPath())
			{
				ShellSetFileAttributes(this);
				Show();
			}

			return TRUE;
		}
		case KEY_CTRLG:
		case KEY_RCTRLG:
		{
			_ALGO(CleverSysLog clv(L"Ctrl-G"));

			if (PanelMode!=PLUGIN_PANEL ||
			        Global->CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FAROTHER))
				if (!ListData.empty() && ApplyCommand())
				{
					// позиционируемся в панели
					if (!Global->FrameManager->IsPanelsActive())
						Global->FrameManager->ActivateFrame(0);

					Update(UPDATE_KEEP_SELECTION);
					Redraw();
					Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
					AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
					AnotherPanel->Redraw();
				}

			return TRUE;
		}
		case KEY_CTRLZ:
		case KEY_RCTRLZ:

			if (!ListData.empty() && PanelMode==NORMAL_PANEL && SetCurPath())
				DescribeFiles();

			return TRUE;
		case KEY_CTRLH:
		case KEY_RCTRLH:
		{
			Global->Opt->ShowHidden=!Global->Opt->ShowHidden;
			Update(UPDATE_KEEP_SELECTION);
			Redraw();
			Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION);//|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
			return TRUE;
		}
		case KEY_CTRLM:
		case KEY_RCTRLM:
		{
			RestoreSelection();
			return TRUE;
		}
		case KEY_CTRLR:
		case KEY_RCTRLR:
		{
			Update(UPDATE_KEEP_SELECTION);
			Redraw();
			{
				Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

				if (AnotherPanel->GetType()!=FILE_PANEL)
				{
					AnotherPanel->SetCurDir(strCurDir,false);
					AnotherPanel->Redraw();
				}
			}
			break;
		}
		case KEY_CTRLN:
		case KEY_RCTRLN:
		{
			ShowShortNames=!ShowShortNames;
			Redraw();
			return TRUE;
		}
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
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(LocalKey)));

			if (ListData.empty())
				break;

			if (CmdLength)
			{
				Global->CtrlObject->CmdLine->ProcessKey(Key);
				return TRUE;
			}

			ProcessEnter(1,LocalKey==KEY_SHIFTENTER||LocalKey==KEY_SHIFTNUMENTER, true,
					LocalKey == KEY_CTRLALTENTER || LocalKey == KEY_RCTRLRALTENTER || LocalKey == KEY_CTRLRALTENTER || LocalKey == KEY_RCTRLALTENTER ||
					LocalKey == KEY_CTRLALTNUMENTER || LocalKey == KEY_RCTRLRALTNUMENTER || LocalKey == KEY_CTRLRALTNUMENTER || LocalKey == KEY_RCTRLALTNUMENTER, OFP_NORMAL);
			return TRUE;
		}
		case KEY_CTRLBACKSLASH:
		case KEY_RCTRLBACKSLASH:
		{
			_ALGO(CleverSysLog clv(L"Ctrl-\\"));
			_ALGO(SysLog(L"%s, FileCount=%d",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));
			BOOL NeedChangeDir=TRUE;

			if (PanelMode==PLUGIN_PANEL)// && *PluginsList[PluginsListSize-1].HostFile)
			{
				bool CheckFullScreen=IsFullScreen();
				OpenPanelInfo Info;
				Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

				if (!Info.CurDir || !*Info.CurDir)
				{
					ChangeDir(L"..");
					NeedChangeDir=FALSE;
					//"this" мог быть удалён в ChangeDir
					Panel* ActivePanel = Global->CtrlObject->Cp()->ActivePanel;

					if (CheckFullScreen!=ActivePanel->IsFullScreen())
						Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel)->Show();
				}
			}

			if (NeedChangeDir)
				ChangeDir(L"\\");

			Global->CtrlObject->Cp()->ActivePanel->Show();
			return TRUE;
		}
		case KEY_SHIFTF1:
		{
			_ALGO(CleverSysLog clv(L"Shift-F1"));
			_ALGO(SysLog(L"%s, FileCount=%d",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

			if (!ListData.empty() && PanelMode!=PLUGIN_PANEL && SetCurPath())
				PluginPutFilesToNew();

			return TRUE;
		}
		case KEY_SHIFTF2:
		{
			_ALGO(CleverSysLog clv(L"Shift-F2"));
			_ALGO(SysLog(L"%s, FileCount=%d",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

			if (!ListData.empty() && SetCurPath())
			{
				if (PanelMode==PLUGIN_PANEL)
				{
					OpenPanelInfo Info;
					Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

					if (Info.HostFile && *Info.HostFile)
						ProcessKey(Manager::Key(KEY_F5));
					else if ((Info.Flags & OPIF_REALNAMES) == OPIF_REALNAMES)
						PluginHostGetFiles();

					return TRUE;
				}

				PluginHostGetFiles();
			}

			return TRUE;
		}
		case KEY_SHIFTF3:
		{
			_ALGO(CleverSysLog clv(L"Shift-F3"));
			_ALGO(SysLog(L"%s, FileCount=%d",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));
			ProcessHostFile();
			return TRUE;
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
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(LocalKey)));
			OpenPanelInfo Info={};
			BOOL RefreshedPanel=TRUE;

			if (PanelMode==PLUGIN_PANEL)
				Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

			if (LocalKey == KEY_NUMPAD5 || LocalKey == KEY_SHIFTNUMPAD5)
				LocalKey=KEY_F3;

			if ((LocalKey==KEY_SHIFTF4 || !ListData.empty()) && SetCurPath())
			{
				int Edit=(LocalKey==KEY_F4 || LocalKey==KEY_ALTF4 || LocalKey==KEY_RALTF4 || LocalKey==KEY_SHIFTF4 || LocalKey==KEY_CTRLSHIFTF4 || LocalKey==KEY_RCTRLSHIFTF4);
				BOOL Modaling=FALSE; ///
				int UploadFile=TRUE;
				string strPluginData;
				string strFileName;
				string strShortFileName;
				string strHostFile=NullToEmpty(Info.HostFile);
				string strInfoCurDir=NullToEmpty(Info.CurDir);
				bool PluginMode=PanelMode==PLUGIN_PANEL && !Global->CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARGETFILE);

				if (PluginMode)
				{
					if (Info.Flags & OPIF_REALNAMES)
						PluginMode = false;
					else
						strPluginData = str_printf(L"<%s:%s>",strHostFile.data(),strInfoCurDir.data());
				}

				if (!PluginMode)
					strPluginData.clear();

				uintptr_t codepage = CP_DEFAULT;

				if (LocalKey==KEY_SHIFTF4)
				{
					do
					{
						if (!dlgOpenEditor(strFileName, codepage))
							return FALSE;

						if (!strFileName.empty())
						{
							ConvertNameToShort(Unquote(strFileName), strShortFileName);

							if (IsAbsolutePath(strFileName))
							{
								PluginMode = false;
							}

							size_t pos;

							// проверим путь к файлу
							if (FindLastSlash(pos,strFileName) && pos)
							{
								if (!(HasPathPrefix(strFileName) && pos==3))
								{
									string Path = strFileName.substr(0, pos);
									DWORD CheckFAttr=api::GetFileAttributes(Path);

									if (CheckFAttr == INVALID_FILE_ATTRIBUTES)
									{
										const wchar_t* const Items[] = {MSG(MEditNewPath1), MSG(MEditNewPath2), MSG(MEditNewPath3), MSG(MHYes), MSG(MHNo)};
										if (Message(MSG_WARNING, 2, MSG(MWarning), Items, ARRAYSIZE(Items), L"WarnEditorPath") != 0)
											return FALSE;
									}
								}
							}
						}
						else if (PluginMode) // пустое имя файла в панели плагина не разрешается!
						{
							const wchar_t* const Items[] = {MSG(MEditNewPlugin1), MSG(MEditNewPath3), MSG(MCancel)};
							if (Message(MSG_WARNING, 2, MSG(MWarning), Items, ARRAYSIZE(Items), L"WarnEditorPluginName") != 0)
								return FALSE;
						}
						else
						{
							strFileName = MSG(MNewFileName);
						}
					}
					while (strFileName.empty());
				}
				else
				{
					assert(CurFile < static_cast<int>(ListData.size()));
					CurPtr = &ListData[CurFile];

					if (CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (Edit)
							return ProcessKey(Manager::Key(KEY_CTRLA));

						CountDirSize(Info.Flags);
						return TRUE;
					}

					strFileName = CurPtr->strName;

					if (!CurPtr->strShortName.empty())
						strShortFileName = CurPtr->strShortName;
					else
						strShortFileName = CurPtr->strName;
				}

				string strTempDir, strTempName;
				int UploadFailed=FALSE, NewFile=FALSE;

				if (PluginMode)
				{
					if (!FarMkTempEx(strTempDir))
						return TRUE;

					api::CreateDirectory(strTempDir,nullptr);
					strTempName=strTempDir+L"\\"+PointToName(strFileName);

					if (LocalKey==KEY_SHIFTF4)
					{
						int Pos=FindFile(strFileName);

						if (Pos!=-1)
							CurPtr = &ListData[Pos];
						else
						{
							NewFile=TRUE;
							strFileName = strTempName;
						}
					}

					if (!NewFile)
					{
						PluginPanelItem PanelItem;
						FileListToPluginItem(*CurPtr, &PanelItem);
						int Result=Global->CtrlObject->Plugins->GetFile(hPlugin,&PanelItem,strTempDir,strFileName,OPM_SILENT|(Edit ? OPM_EDIT:OPM_VIEW));
						FreePluginPanelItem(PanelItem);

						if (!Result)
						{
							api::RemoveDirectory(strTempDir);
							return TRUE;
						}
					}

					ConvertNameToShort(strFileName,strShortFileName);
				}

				/* $ 08.04.2002 IS
				   Флаг, говорящий о том, что нужно удалить файл, который открывали во
				   вьюере. Если файл открыли во внутреннем вьюере, то DeleteViewedFile
				   должно быт равно false, т.к. внутренний вьюер сам все удалит.
				*/
				bool DeleteViewedFile=PluginMode && !Edit;

				if (!strFileName.empty())
				{
					if (Edit)
					{
						int EnableExternal=(((LocalKey==KEY_F4 || LocalKey==KEY_SHIFTF4) && Global->Opt->EdOpt.UseExternalEditor) ||
						                    ((LocalKey==KEY_ALTF4 || LocalKey==KEY_RALTF4) && !Global->Opt->EdOpt.UseExternalEditor)) && !Global->Opt->strExternalEditor.empty();
						/* $ 02.08.2001 IS обработаем ассоциации для alt-f4 */
						BOOL Processed=FALSE;

						if ((LocalKey==KEY_ALTF4 || LocalKey==KEY_RALTF4) &&
						        ProcessLocalFileTypes(strFileName,strShortFileName,FILETYPE_ALTEDIT,
						                              PluginMode))
							Processed=TRUE;
						else if (LocalKey==KEY_F4 &&
						         ProcessLocalFileTypes(strFileName,strShortFileName,FILETYPE_EDIT,
						                               PluginMode))
							Processed=TRUE;

						if (!Processed || LocalKey==KEY_CTRLSHIFTF4 || LocalKey==KEY_RCTRLSHIFTF4)
						{
							if (EnableExternal)
								ProcessExternal(Global->Opt->strExternalEditor,strFileName,strShortFileName,PluginMode);
							else if (PluginMode)
							{
								RefreshedPanel = Global->FrameManager->GetCurrentFrame()->GetType() != MODALTYPE_EDITOR;
								FileEditor ShellEditor(strFileName,codepage,(LocalKey==KEY_SHIFTF4?FFILEEDIT_CANNEWFILE:0)|FFILEEDIT_DISABLEHISTORY,-1,-1,&strPluginData);
								ShellEditor.SetDynamicallyBorn(false);
								Global->FrameManager->EnterModalEV();
								Global->FrameManager->ExecuteModal();//OT
								Global->FrameManager->ExitModalEV();
								/* $ 24.11.2001 IS
								     Если мы создали новый файл, то не важно, изменялся он
								     или нет, все равно добавим его на панель плагина.
								*/
								UploadFile=ShellEditor.IsFileChanged() || NewFile;
								Modaling=TRUE;///
							}
							else
							{
								FileEditor *ShellEditor=new FileEditor(strFileName,codepage,(LocalKey==KEY_SHIFTF4?FFILEEDIT_CANNEWFILE:0)|FFILEEDIT_ENABLEF6);

									int editorExitCode=ShellEditor->GetExitCode();
									if (editorExitCode == XC_LOADING_INTERRUPTED || editorExitCode == XC_OPEN_ERROR)
									{
										delete ShellEditor;
									}
									else
									{
										if (!PluginMode)
										{
											NamesList EditList;

											std::for_each(CONST_RANGE(ListData, i)
											{
												if (!(i.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
													EditList.AddName(i.strName);
											});
											EditList.SetCurName(strFileName);
											ShellEditor->SetNamesList(EditList);
										}

										Global->FrameManager->ExecuteModal();
									}
							}
						}

						if (PluginMode && UploadFile)
						{
							PluginPanelItem PanelItem;
							string strSaveDir;
							api::GetCurrentDirectory(strSaveDir);

							if (api::GetFileAttributes(strTempName)==INVALID_FILE_ATTRIBUTES)
							{
								string strFindName;
								string strPath;
								strPath = strTempName;
								CutToSlash(strPath, false);
								strFindName = strPath+L"*";
								api::enum_file Find(strFindName);
								auto ItemIterator = std::find_if(CONST_RANGE(Find, i) { return (i.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0; });
								if (ItemIterator != Find.end())
									strTempName = strPath + ItemIterator->strFileName;
							}

							if (FileNameToPluginItem(strTempName,&PanelItem))
							{
								int PutCode = Global->CtrlObject->Plugins->PutFiles(hPlugin, &PanelItem, 1, false, OPM_EDIT);

								if (PutCode==1 || PutCode==2)
									SetPluginModified();

								if (!PutCode)
									UploadFailed=TRUE;
							}

							FarChDir(strSaveDir);
						}
					}
					else
					{
						int EnableExternal=((LocalKey==KEY_F3 && Global->Opt->ViOpt.UseExternalViewer) ||
						                    ((LocalKey==KEY_ALTF3 || LocalKey==KEY_RALTF3) && !Global->Opt->ViOpt.UseExternalViewer)) &&
						                   !Global->Opt->strExternalViewer.empty();
						/* $ 02.08.2001 IS обработаем ассоциации для alt-f3 */
						BOOL Processed=FALSE;

						if ((LocalKey==KEY_ALTF3 || LocalKey==KEY_RALTF3) &&
						        ProcessLocalFileTypes(strFileName,strShortFileName,FILETYPE_ALTVIEW,PluginMode))
							Processed=TRUE;
						else if (LocalKey==KEY_F3 &&
						         ProcessLocalFileTypes(strFileName,strShortFileName,FILETYPE_VIEW,PluginMode))
							Processed=TRUE;

						if (!Processed || LocalKey==KEY_CTRLSHIFTF3 || LocalKey==KEY_RCTRLSHIFTF3)
						{
							if (EnableExternal)
								ProcessExternal(Global->Opt->strExternalViewer,strFileName,strShortFileName,PluginMode);
							else
							{
								NamesList ViewList;

								if (!PluginMode)
								{
									std::for_each(CONST_RANGE(ListData, i)
									{
										if (!(i.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
											ViewList.AddName(i.strName);
									});
									ViewList.SetCurName(strFileName);
								}

								FileViewer *ShellViewer=new FileViewer(strFileName, TRUE,PluginMode,PluginMode,-1,strPluginData.data(),&ViewList);

									if (!ShellViewer->GetExitCode())
									{
										delete ShellViewer;
									}
									/* $ 08.04.2002 IS
									Сбросим DeleteViewedFile, т.к. внутренний вьюер сам все удалит
									*/
									else if (PluginMode)
									{
										ShellViewer->SetTempViewName(strFileName);
										DeleteViewedFile=false;
									}

								Modaling=FALSE;
							}
						}
					}
				}

				/* $ 08.04.2002 IS
				     для файла, который открывался во внутреннем вьюере, ничего не
				     предпринимаем, т.к. вьюер об этом позаботится сам
				*/
				if (PluginMode)
				{
					if (UploadFailed)
						Message(MSG_WARNING,1,MSG(MError),MSG(MCannotSaveFile),
						        MSG(MTextSavedToTemp),strFileName.data(),MSG(MOk));
					else if (Edit || DeleteViewedFile)
						// удаляем файл только для случая окрытия его в редакторе или во
						// внешнем вьюере, т.к. внутренний вьюер удаляет файл сам
						DeleteFileWithFolder(strFileName);
				}

				if (Modaling && (Edit || IsColumnDisplayed(ADATE_COLUMN)) && RefreshedPanel)
				{
					if (!PluginMode || UploadFile)
					{
						Update(UPDATE_KEEP_SELECTION);
						Redraw();
						Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

						if (AnotherPanel->GetMode()==NORMAL_PANEL)
						{
							AnotherPanel->Update(UPDATE_KEEP_SELECTION);
							AnotherPanel->Redraw();
						}
					}
//          else
//            SetTitle();
				}
				else if (PanelMode==NORMAL_PANEL)
					AccessTimeUpdateRequired=TRUE;
			}

			/* $ 15.07.2000 tran
			   а тут мы вызываем перерисовку панелей
			   потому что этот viewer, editor могут нам неверно восстановить
			   */
//      Global->CtrlObject->Cp()->Redraw();
			return TRUE;
		}
		case KEY_F5:
		case KEY_F6:
		case KEY_ALTF6:
		case KEY_RALTF6:
		case KEY_DRAGCOPY:
		case KEY_DRAGMOVE:
		{
			_ALGO(CleverSysLog clv(L"F5/F6/Alt-F6/DragCopy/DragMove"));
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(LocalKey)));

			ProcessCopyKeys(LocalKey);

			return TRUE;
		}

		case KEY_ALTF5:  // Печать текущего/выбранных файла/ов
		case KEY_RALTF5:
		{
			_ALGO(CleverSysLog clv(L"Alt-F5"));
			_ALGO(SysLog(L"%s, FileCount=%d",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

			if (!ListData.empty() && SetCurPath())
				PrintFiles(this);

			return TRUE;
		}
		case KEY_SHIFTF5:
		case KEY_SHIFTF6:
		{
			_ALGO(CleverSysLog clv(L"Shift-F5/Shift-F6"));
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(LocalKey)));

			if (!ListData.empty() && SetCurPath())
			{
				assert(CurFile < static_cast<int>(ListData.size()));
				string name = ListData[CurFile].strName;
				char selected = ListData[CurFile].Selected;

				int RealName=PanelMode!=PLUGIN_PANEL;
				ReturnCurrentFile=TRUE;

				if (PanelMode==PLUGIN_PANEL)
				{
					OpenPanelInfo Info;
					Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
					RealName=Info.Flags&OPIF_REALNAMES;
				}

				if (RealName)
				{
					int ToPlugin=0;
					ShellCopy ShCopy(this,LocalKey==KEY_SHIFTF6,FALSE,TRUE,TRUE,ToPlugin,nullptr);
				}
				else
				{
					ProcessCopyKeys(LocalKey==KEY_SHIFTF5 ? KEY_F5:KEY_F6);
				}

				ReturnCurrentFile=FALSE;

				if (!ListData.empty())
				{
					assert(CurFile < static_cast<int>(ListData.size()));
					if (LocalKey != KEY_SHIFTF5 && !StrCmpI(name, ListData[CurFile].strName) && selected > ListData[CurFile].Selected)
					{
						Select(ListData[CurFile], selected);
						Redraw();
					}
				}
			}

			return TRUE;
		}
		case KEY_F7:
		{
			_ALGO(CleverSysLog clv(L"F7"));
			_ALGO(SysLog(L"%s, FileCount=%d",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

			if (SetCurPath())
			{
				if (PanelMode==PLUGIN_PANEL && !Global->CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARMAKEDIRECTORY))
				{
					string strDirName;
					const wchar_t *lpwszDirName=strDirName.data();
					int MakeCode=Global->CtrlObject->Plugins->MakeDirectory(hPlugin,&lpwszDirName,0);
					Global->CatchError();
					strDirName=lpwszDirName;

					if (!MakeCode)
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateFolder),strDirName.data(),MSG(MOk));

					Update(UPDATE_KEEP_SELECTION);

					if (MakeCode==1)
						GoToFile(PointToName(strDirName));

					Redraw();
					Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
					/* $ 07.09.2001 VVM
					  ! Обновить соседнюю панель с установкой на новый каталог */
//          AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
//          AnotherPanel->Redraw();

					if (AnotherPanel->GetType()!=FILE_PANEL)
					{
						AnotherPanel->SetCurDir(strCurDir,false);
						AnotherPanel->Redraw();
					}
				}
				else
					ShellMakeDir(this);
			}

			return TRUE;
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
				return TRUE;

			_ALGO(CleverSysLog clv(L"F8/Shift-F8/Shift-Del/Alt-Del"));
			_ALGO(SysLog(L"%s, FileCount=%d, Key=%s",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(LocalKey)));

			if (!ListData.empty() && SetCurPath())
			{
				if (LocalKey==KEY_SHIFTF8)
					ReturnCurrentFile=TRUE;

				if (PanelMode==PLUGIN_PANEL &&
				        !Global->CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARDELETEFILES))
					PluginDelete();
				else
				{
					bool SaveOpt=Global->Opt->DeleteToRecycleBin;

					if (LocalKey==KEY_SHIFTDEL || LocalKey==KEY_SHIFTNUMDEL || LocalKey==KEY_SHIFTDECIMAL)
						Global->Opt->DeleteToRecycleBin=0;

					ShellDelete(this,LocalKey==KEY_ALTDEL||LocalKey==KEY_RALTDEL||LocalKey==KEY_ALTNUMDEL||LocalKey==KEY_RALTNUMDEL||LocalKey==KEY_ALTDECIMAL||LocalKey==KEY_RALTDECIMAL);
					Global->Opt->DeleteToRecycleBin=SaveOpt;
				}

				if (LocalKey==KEY_SHIFTF8)
					ReturnCurrentFile=FALSE;
			}

			return TRUE;
		}
		// $ 26.07.2001 VVM  С альтом скролим всегда по 1
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		case(KEY_MSWHEEL_UP | KEY_RALT):
			Scroll(LocalKey & (KEY_ALT|KEY_RALT)?-1:(int)-Global->Opt->MsWheelDelta);
			return TRUE;
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		case(KEY_MSWHEEL_DOWN | KEY_RALT):
			Scroll(LocalKey & (KEY_ALT|KEY_RALT)?1:(int)Global->Opt->MsWheelDelta);
			return TRUE;
		case KEY_MSWHEEL_LEFT:
		case(KEY_MSWHEEL_LEFT | KEY_ALT):
		case(KEY_MSWHEEL_LEFT | KEY_RALT):
		{
			int Roll = LocalKey & (KEY_ALT|KEY_RALT)?1:(int)Global->Opt->MsHWheelDelta;

			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_LEFT));

			return TRUE;
		}
		case KEY_MSWHEEL_RIGHT:
		case(KEY_MSWHEEL_RIGHT | KEY_ALT):
		case(KEY_MSWHEEL_RIGHT | KEY_RALT):
		{
			int Roll = LocalKey & (KEY_ALT|KEY_RALT)?1:(int)Global->Opt->MsHWheelDelta;

			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_RIGHT));

			return TRUE;
		}
		case KEY_HOME:         case KEY_NUMPAD7:
			ToBegin();
			return TRUE;
		case KEY_END:          case KEY_NUMPAD1:
			ToEnd();
			return TRUE;
		case KEY_UP:           case KEY_NUMPAD8:
			MoveCursor(-1);
			return TRUE;
		case KEY_DOWN:         case KEY_NUMPAD2:
			MoveCursor(1);
			return TRUE;
		case KEY_PGUP:         case KEY_NUMPAD9:
			N=Columns*Height-1;
			CurTopFile-=N;
			MoveCursor(-N);
			return TRUE;
		case KEY_PGDN:         case KEY_NUMPAD3:
			N=Columns*Height-1;
			CurTopFile+=N;
			MoveCursor(N);
			return TRUE;
		case KEY_LEFT:         case KEY_NUMPAD4:

			if ((Columns==1 && Global->Opt->ShellRightLeftArrowsRule == 1) || Columns>1 || !CmdLength)
			{
				if (CurTopFile>=Height && CurFile-CurTopFile<Height)
					CurTopFile-=Height;

				MoveCursor(-Height);
				return TRUE;
			}

			return FALSE;
		case KEY_RIGHT:        case KEY_NUMPAD6:

			if ((Columns==1 && Global->Opt->ShellRightLeftArrowsRule == 1) || Columns>1 || !CmdLength)
			{
				if (CurFile+Height < static_cast<int>(ListData.size()) && CurFile-CurTopFile>=(Columns-1)*(Height))
					CurTopFile+=Height;

				MoveCursor(Height);
				return TRUE;
			}

			return FALSE;
			/* $ 25.04.2001 DJ
			   оптимизация Shift-стрелок для Selected files first: делаем сортировку
			   один раз
			*/
		case KEY_SHIFTHOME:    case KEY_SHIFTNUMPAD7:
		{
			InternalProcessKey++;
			Lock();

			while (CurFile>0)
				ProcessKey(Manager::Key(KEY_SHIFTUP));

			ProcessKey(Manager::Key(KEY_SHIFTUP));
			InternalProcessKey--;
			Unlock();

			if (SelectedFirst)
				SortFileList(TRUE);

			ShowFileList(TRUE);
			return TRUE;
		}
		case KEY_SHIFTEND:     case KEY_SHIFTNUMPAD1:
		{
			InternalProcessKey++;
			Lock();

			while (CurFile < static_cast<int>(ListData.size() - 1))
				ProcessKey(Manager::Key(KEY_SHIFTDOWN));

			ProcessKey(Manager::Key(KEY_SHIFTDOWN));
			InternalProcessKey--;
			Unlock();

			if (SelectedFirst)
				SortFileList(TRUE);

			ShowFileList(TRUE);
			return TRUE;
		}
		case KEY_SHIFTPGUP:    case KEY_SHIFTNUMPAD9:
		case KEY_SHIFTPGDN:    case KEY_SHIFTNUMPAD3:
		{
			N=Columns*Height-1;
			InternalProcessKey++;
			Lock();

			while (N--)
				ProcessKey(Manager::Key(LocalKey==KEY_SHIFTPGUP||LocalKey==KEY_SHIFTNUMPAD9? KEY_SHIFTUP:KEY_SHIFTDOWN));

			InternalProcessKey--;
			Unlock();

			if (SelectedFirst)
				SortFileList(TRUE);

			ShowFileList(TRUE);
			return TRUE;
		}
		case KEY_SHIFTLEFT:    case KEY_SHIFTNUMPAD4:
		case KEY_SHIFTRIGHT:   case KEY_SHIFTNUMPAD6:
		{
			if (ListData.empty())
				return TRUE;

			if (Columns>1)
			{
				N=Height;
				InternalProcessKey++;
				Lock();

				while (N--)
					ProcessKey(Manager::Key(LocalKey==KEY_SHIFTLEFT || LocalKey==KEY_SHIFTNUMPAD4? KEY_SHIFTUP:KEY_SHIFTDOWN));

				assert(CurFile < static_cast<int>(ListData.size()));
				Select(ListData[CurFile], ShiftSelection);

				if (SelectedFirst)
					SortFileList(TRUE);

				InternalProcessKey--;
				Unlock();

				if (SelectedFirst)
					SortFileList(TRUE);

				ShowFileList(TRUE);
				return TRUE;
			}

			return FALSE;
		}
		case KEY_SHIFTUP:      case KEY_SHIFTNUMPAD8:
		case KEY_SHIFTDOWN:    case KEY_SHIFTNUMPAD2:
		{
			if (ListData.empty())
				return TRUE;

			assert(CurFile < static_cast<int>(ListData.size()));
			CurPtr = &ListData[CurFile];

			if (ShiftSelection==-1)
			{
				// .. is never selected
				if (CurFile < static_cast<int>(ListData.size() - 1) && TestParentFolderName(CurPtr->strName))
					ShiftSelection = !ListData[CurFile+1].Selected;
				else
					ShiftSelection=!CurPtr->Selected;
			}

			Select(*CurPtr, ShiftSelection);

			if (LocalKey==KEY_SHIFTUP || LocalKey == KEY_SHIFTNUMPAD8)
				MoveCursor(-1);
			else
				MoveCursor(1);

			if (SelectedFirst && !InternalProcessKey)
				SortFileList(TRUE);

			ShowFileList(TRUE);
			return TRUE;
		}
		case KEY_INS:          case KEY_NUMPAD0:
		{
			if (ListData.empty())
				return TRUE;

			assert(CurFile < static_cast<int>(ListData.size()));
			CurPtr = &ListData[CurFile];
			Select(*CurPtr,!CurPtr->Selected);
			bool avoid_up_jump = SelectedFirst && (CurFile > 0) && (CurFile+1 == static_cast<int>(ListData.size())) && CurPtr->Selected;
			MoveCursor(1);

			if (SelectedFirst)
			{
				SortFileList(TRUE);
				if (avoid_up_jump)
					ToEnd();
			}

			ShowFileList(TRUE);
			return TRUE;
		}
		case KEY_CTRLF3:
		case KEY_RCTRLF3:
			SetSortMode(BY_NAME);
			return TRUE;
		case KEY_CTRLF4:
		case KEY_RCTRLF4:
			SetSortMode(BY_EXT);
			return TRUE;
		case KEY_CTRLF5:
		case KEY_RCTRLF5:
			SetSortMode(BY_MTIME);
			return TRUE;
		case KEY_CTRLF6:
		case KEY_RCTRLF6:
			SetSortMode(BY_SIZE);
			return TRUE;
		case KEY_CTRLF7:
		case KEY_RCTRLF7:
			SetSortMode(UNSORTED);
			return TRUE;
		case KEY_CTRLF8:
		case KEY_RCTRLF8:
			SetSortMode(BY_CTIME);
			return TRUE;
		case KEY_CTRLF9:
		case KEY_RCTRLF9:
			SetSortMode(BY_ATIME);
			return TRUE;
		case KEY_CTRLF10:
		case KEY_RCTRLF10:
			SetSortMode(BY_DIZ);
			return TRUE;
		case KEY_CTRLF11:
		case KEY_RCTRLF11:
			SetSortMode(BY_OWNER);
			return TRUE;
		case KEY_CTRLF12:
		case KEY_RCTRLF12:
			SelectSortMode();
			return TRUE;
		case KEY_SHIFTF11:
			SortGroups=!SortGroups;

			if (SortGroups)
				ReadSortGroups();

			SortFileList(TRUE);
			ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
			Show();
			return TRUE;
		case KEY_SHIFTF12:
			SelectedFirst=!SelectedFirst;
			SortFileList(TRUE);
			ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
			Show();
			return TRUE;
		case KEY_CTRLPGUP:     case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP:    case KEY_RCTRLNUMPAD9:
		{
			if (Global->Opt->PgUpChangeDisk || PanelMode==PLUGIN_PANEL || !IsRootPath(strCurDir))
			{
				//"this" может быть удалён в ChangeDir
				bool CheckFullScreen=IsFullScreen();
				ChangeDir(L"..");
				Panel *NewActivePanel = Global->CtrlObject->Cp()->ActivePanel;
				NewActivePanel->SetViewMode(NewActivePanel->GetViewMode());

				if (CheckFullScreen!=NewActivePanel->IsFullScreen())
					Global->CtrlObject->Cp()->GetAnotherPanel(NewActivePanel)->Show();

				NewActivePanel->Show();
			}
			return TRUE;
		}
		case KEY_CTRLPGDN:
		case KEY_RCTRLPGDN:
		case KEY_CTRLNUMPAD3:
		case KEY_RCTRLNUMPAD3:
		case KEY_CTRLSHIFTPGDN:
		case KEY_RCTRLSHIFTPGDN:
		case KEY_CTRLSHIFTNUMPAD3:
		case KEY_RCTRLSHIFTNUMPAD3:
			ProcessEnter(0,0,!(LocalKey&KEY_SHIFT), false, OFP_ALTERNATIVE);
			return TRUE;

		case KEY_APPS:
		case KEY_SHIFTAPPS:
		{
			//вызовем EMenu если он есть
			if (Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu.Id))
			{
				Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, ToPtr(1)); // EMenu Plugin :-)
			}
			return TRUE;
		}

		default:

			if (((LocalKey>=KEY_ALT_BASE+0x01 && LocalKey<=KEY_ALT_BASE+65535) || (LocalKey>=KEY_RALT_BASE+0x01 && LocalKey<=KEY_RALT_BASE+65535) ||
			        (LocalKey>=KEY_ALTSHIFT_BASE+0x01 && LocalKey<=KEY_ALTSHIFT_BASE+65535) || (LocalKey>=KEY_RALTSHIFT_BASE+0x01 && LocalKey<=KEY_RALTSHIFT_BASE+65535)) &&
			        (LocalKey&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_BS && (LocalKey&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_TAB &&
			        (LocalKey&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_ENTER && (LocalKey&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_ESC &&
			        !(LocalKey&EXTENDED_KEY_BASE)
			   )
			{
				//_SVS(SysLog(L">FastFind: Key=%s",_FARKEY_ToName(Key)));
				// Скорректирем уже здесь нужные клавиши, т.к. WaitInFastFind
				// в это время еще равно нулю.
				static const char Code[]=")!@#$%^&*(";

				if (LocalKey >= KEY_ALTSHIFT0 && LocalKey <= KEY_ALTSHIFT9)
					LocalKey=(DWORD)Code[LocalKey-KEY_ALTSHIFT0];
				else if (LocalKey >= KEY_RALTSHIFT0 && LocalKey <= KEY_RALTSHIFT9)
					LocalKey=(DWORD)Code[LocalKey-KEY_RALTSHIFT0];
				else if ((LocalKey&(~(KEY_ALT|KEY_RALT|KEY_SHIFT))) == '/')
					LocalKey='?';
				else if ((LocalKey == KEY_ALTSHIFT+'-') || (LocalKey == KEY_RALT+KEY_SHIFT+'-'))
					LocalKey='_';
				else if ((LocalKey == KEY_ALTSHIFT+'=') || (LocalKey == KEY_RALT+KEY_SHIFT+'='))
					LocalKey='+';

				//_SVS(SysLog(L"<FastFind: Key=%s",_FARKEY_ToName(Key)));
				FastFind(LocalKey);
			}
			else
				break;

			return TRUE;
	}

	return FALSE;
}


void FileList::Select(FileListItem& SelItem, int Selection)
{
	if (!TestParentFolderName(SelItem.strName) && SelItem.Selected!=Selection)
	{
		CacheSelIndex=-1;
		CacheSelClearIndex=-1;

		if ((SelItem.Selected=Selection))
		{
			SelFileCount++;
			SelFileSize += SelItem.FileSize;
		}
		else
		{
			SelFileCount--;
			SelFileSize -= SelItem.FileSize;
		}
	}
}


void FileList::ProcessEnter(bool EnableExec,bool SeparateWindow,bool EnableAssoc, bool RunAs, OPENFILEPLUGINTYPE Type)
{
	string strFileName, strShortFileName;

	if (CurFile >= static_cast<int>(ListData.size()))
		return;

	FileListItem *CurPtr = &ListData[CurFile];
	strFileName = CurPtr->strName;

	if (!CurPtr->strShortName.empty())
		strShortFileName = CurPtr->strShortName;
	else
		strShortFileName = CurPtr->strName;

	if (CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
		BOOL IsRealName=FALSE;

		if (PanelMode==PLUGIN_PANEL)
		{
			OpenPanelInfo Info;
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
			IsRealName=Info.Flags&OPIF_REALNAMES;
		}

		// Shift-Enter на каталоге вызывает проводник
		if ((PanelMode!=PLUGIN_PANEL || IsRealName) && SeparateWindow)
		{
			string strFullPath;

			if (!IsAbsolutePath(CurPtr->strName))
			{
				strFullPath = strCurDir;
				AddEndSlash(strFullPath);

				/* 23.08.2001 VVM
				  ! SHIFT+ENTER на ".." срабатывает для текущего каталога, а не родительского */
				if (!TestParentFolderName(CurPtr->strName))
					strFullPath += CurPtr->strName;
			}
			else
			{
				strFullPath = CurPtr->strName;
			}

			QuoteSpace(strFullPath);
			Execute(strFullPath, false, true, true, true);
		}
		else
		{
			bool CheckFullScreen=IsFullScreen();

			ChangeDir(CurPtr->strName,false,true,CurPtr);

			//"this" может быть удалён в ChangeDir
			Panel *ActivePanel = Global->CtrlObject->Cp()->ActivePanel;

			if (CheckFullScreen!=ActivePanel->IsFullScreen())
			{
				Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel)->Show();
			}

			ActivePanel->Show();
		}
	}
	else
	{
		bool PluginMode=PanelMode==PLUGIN_PANEL && !Global->CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARGETFILE);

		if (PluginMode)
		{
			string strTempDir;

			if (!FarMkTempEx(strTempDir))
				return;

			api::CreateDirectory(strTempDir,nullptr);
			PluginPanelItem PanelItem;
			FileListToPluginItem(*CurPtr, &PanelItem);
			int Result=Global->CtrlObject->Plugins->GetFile(hPlugin,&PanelItem,strTempDir,strFileName,OPM_SILENT|OPM_VIEW);
			FreePluginPanelItem(PanelItem);

			if (!Result)
			{
				api::RemoveDirectory(strTempDir);
				return;
			}

			ConvertNameToShort(strFileName,strShortFileName);
		}

		if (EnableExec && SetCurPath() && !SeparateWindow &&
		        ProcessLocalFileTypes(strFileName,strShortFileName,FILETYPE_EXEC,PluginMode)) //?? is was var!
		{
			if (PluginMode)
				DeleteFileWithFolder(strFileName);

			return;
		}

		const wchar_t *ExtPtr = wcsrchr(strFileName.data(), L'.');
		int ExeType=FALSE,BatType=FALSE;

		if (ExtPtr)
		{
			ExeType=!StrCmpI(ExtPtr, L".exe") || !StrCmpI(ExtPtr, L".com");
			BatType=IsBatchExtType(ExtPtr);
		}

		if (EnableExec && (ExeType || BatType))
		{
			QuoteSpace(strFileName);

			if (!(Global->Opt->ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTPANEL) && !PluginMode) //AN
				Global->CtrlObject->CmdHistory->AddToHistory(strFileName, HR_DEFAULT, nullptr, nullptr, strCurDir.data());


			Global->CtrlObject->CmdLine->ExecString(strFileName, PluginMode, SeparateWindow, true, false, RunAs, false, true);

			if (PluginMode)
				DeleteFileWithFolder(strFileName);
		}
		else if (SetCurPath())
		{
			HANDLE hOpen = nullptr;

			if (EnableAssoc &&
			        !EnableExec &&     // не запускаем и не в отдельном окне,
			        !SeparateWindow && // следовательно это Ctrl-PgDn
			        ProcessLocalFileTypes(strFileName,strShortFileName,FILETYPE_ALTEXEC,
			                              PluginMode)
			   )
			{
				if (PluginMode)
				{
					DeleteFileWithFolder(strFileName);
				}

				return;
			}

			if (SeparateWindow || !(hOpen=OpenFilePlugin(&strFileName,TRUE, Type)) ||
			        hOpen==PANEL_STOP)
			{
				if (EnableExec && hOpen!=PANEL_STOP)
					if (SeparateWindow || Global->Opt->UseRegisteredTypes)
						ProcessGlobalFileTypes(strFileName, PluginMode, RunAs, true);

				if (PluginMode)
				{
					DeleteFileWithFolder(strFileName);
				}
			}

			return;
		}
	}
}


bool FileList::SetCurDir(const string& NewDir,bool ClosePanel,bool IsUpdated)
{

	FileListItem *CurPtr=nullptr;

	if (PanelMode==PLUGIN_PANEL)
	{
		if (ClosePanel)
		{
			bool CheckFullScreen=IsFullScreen();
			OpenPanelInfo Info;
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
			string strInfoHostFile=NullToEmpty(Info.HostFile);

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

			Global->CtrlObject->Cp()->RedrawKeyBar();

			if (CheckFullScreen!=IsFullScreen())
			{
				Global->CtrlObject->Cp()->GetAnotherPanel(this)->Redraw();
			}
		}
		else if (CurFile < static_cast<int>(ListData.size()))
		{
			CurPtr = &ListData[CurFile];
		}
	}

	if (!NewDir.empty())
	{
		return ChangeDir(NewDir,true,IsUpdated,CurPtr);
	}

	return false;
}

bool FileList::ChangeDir(const string& NewDir,bool ResolvePath,bool IsUpdated,const FileListItem *CurPtr)
{
	string strFindDir, strSetDir;

	if (PanelMode!=PLUGIN_PANEL && !IsAbsolutePath(NewDir) && !TestCurrentDirectory(strCurDir))
		FarChDir(strCurDir);

	strSetDir = NewDir;
	bool dot2Present = strSetDir == L"..";

	bool RootPath = false;
	bool NetPath = false;
	bool DrivePath = false;

	if (PanelMode!=PLUGIN_PANEL)
	{
		if (dot2Present)
		{
			strSetDir = strCurDir;
			PATH_TYPE Type = ParsePath(strCurDir, nullptr, &RootPath);
			if(Type == PATH_REMOTE || Type == PATH_REMOTEUNC)
			{
				NetPath = true;
			}
			else if(Type == PATH_DRIVELETTER)
			{
				DrivePath = true;
			}

			if(!RootPath)
			{
				CutToSlash(strSetDir);
			}
		}

		if (!ResolvePath)
			ConvertNameToFull(strSetDir,strSetDir);
		PrepareDiskPath(strSetDir, ResolvePath);

		if (HasPathPrefix(strSetDir) && strSetDir[5] == L':' && !strSetDir[6])
			AddEndSlash(strSetDir);
	}

	if (!dot2Present && strSetDir != L"\\")
		UpperFolderTopFile=CurTopFile;

	if (SelFileCount>0)
		ClearSelection();

	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		/* $ 16.01.2002 VVM
		  + Если у плагина нет OPIF_REALNAMES, то история папок не пишется в реестр */
		string strInfoCurDir=NullToEmpty(Info.CurDir);
		//string strInfoFormat=NullToEmpty(Info.Format);
		string strInfoHostFile=NullToEmpty(Info.HostFile);
		string strInfoData=NullToEmpty(Info.ShortcutData);
		if(Info.Flags&OPIF_SHORTCUT) Global->CtrlObject->FolderHistory->AddToHistory(strInfoCurDir, HR_DEFAULT, &PluginManager::GetGUID(hPlugin), strInfoHostFile.data(), strInfoData.data());
		/* $ 25.04.01 DJ
		   при неудаче SetDirectory не сбрасываем выделение
		*/
		bool SetDirectorySuccess = true;
		bool GoToPanelFile = false;
		bool PluginClosed=false;

		if (dot2Present && (strInfoCurDir.empty() || strInfoCurDir == L"\\"))
		{
			if (ProcessPluginEvent(FE_CLOSE,nullptr))
				return true;

			PluginClosed=true;
			strFindDir = strInfoHostFile;

			if (strFindDir.empty() && (Info.Flags & OPIF_REALNAMES) && CurFile < static_cast<int>(ListData.size()))
			{
				strFindDir = ListData[CurFile].strName;
				GoToPanelFile=true;
			}

			PopPlugin(TRUE);
			Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

			if (AnotherPanel->GetType()==INFO_PANEL)
				AnotherPanel->Redraw();
		}
		else
		{
			strFindDir = strInfoCurDir;

			UserDataItem UserData = {};
			UserData.Data=CurPtr?CurPtr->UserData:nullptr;
			UserData.FreeData=CurPtr?CurPtr->Callback:nullptr;

			SetDirectorySuccess=Global->CtrlObject->Plugins->SetDirectory(hPlugin,strSetDir,0,&UserData) != FALSE;
		}

		ProcessPluginCommand();

		// после закрытия панели нужно сразу установить внутренний каталог, иначе будет "Cannot find the file" - Mantis#1731
		if (PanelMode == NORMAL_PANEL)
			SetCurPath();

		if (SetDirectorySuccess)
			Update(0);
		else
			Update(UPDATE_KEEP_SELECTION);

		PopPrevData(strFindDir,PluginClosed,!GoToPanelFile,dot2Present,SetDirectorySuccess);

		return SetDirectorySuccess;
	}
	else
	{
		{
			string strFullNewDir;
			ConvertNameToFull(strSetDir, strFullNewDir);

			if (StrCmpI(strFullNewDir, strCurDir))
				Global->CtrlObject->FolderHistory->AddToHistory(strCurDir);
		}

		if (dot2Present)
		{
			if (RootPath)
			{
				if (NetPath)
				{
					string tmp = strCurDir;	// strCurDir can be altered during next call
					if (Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Network.Id,OPEN_FILEPANEL, UNSAFE_CSTR(tmp))) // NetWork Plugin :-)
					{
						return false;
					}
				}
				if(DrivePath && Global->Opt->PgUpChangeDisk == 2)
				{
					string RemoteName;
					if(DriveLocalToRemoteName(DRIVE_REMOTE, strCurDir.front(), RemoteName))
					{
						if (Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Network.Id, OPEN_FILEPANEL, UNSAFE_CSTR(RemoteName))) // NetWork Plugin :-)
						{
							return false;
						}
					}
				}
				Global->CtrlObject->Cp()->ActivePanel->ChangeDisk();
				return true;
			}
		}
	}

	strFindDir = PointToName(strCurDir);
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

	if (PanelMode!=PLUGIN_PANEL && strSetDir == L"\\")
	{
		strSetDir = ExtractPathRoot(strCurDir);
	}

	if (!FarChDir(strSetDir))
	{
		Global->CatchError();
		if (Global->FrameManager->ManagerStarted())
		{
			/* $ 03.11.2001 IS Укажем имя неудачного каталога */
			Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG(MError), (dot2Present?L"..":strSetDir.data()), MSG(MOk));
			UpdateFlags = UPDATE_KEEP_SELECTION;
		}

		SetDirectorySuccess=false;
	}

	/* $ 28.04.2001 IS
	     Закомментарим "до лучших времен".
	     Я не знаю, почему глюк проявлялся только у меня, но зато знаю, почему он
	     был просто-таки обязан проявится. Желающие могут немного RTFM. Тема для
	     изучения: chdir, setdisk, SetCurrentDirectory и переменные окружения

	*/
	/*else {
	  if (isalpha(SetDir[0]) && SetDir[1]==':')
	  {
	    int CurDisk=toupper(SetDir[0])-'A';
	    setdisk(CurDisk);
	  }
	}*/
	api::GetCurrentDirectory(strCurDir);
	if (!IsUpdated)
		return SetDirectorySuccess;

	Update(UpdateFlags);

	if (dot2Present)
	{
		GoToFile(strFindDir);
		CurTopFile=UpperFolderTopFile;
		UpperFolderTopFile=0;
		CorrectPosition();
	}
	else if (UpdateFlags != UPDATE_KEEP_SELECTION)
		CurFile=CurTopFile=0;

	if (GetFocus())
	{
		Global->CtrlObject->CmdLine->SetCurDir(strCurDir);
		Global->CtrlObject->CmdLine->Show();
	}

	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->GetType()!=FILE_PANEL)
	{
		AnotherPanel->SetCurDir(strCurDir, false);
		AnotherPanel->Redraw();
	}

	if (PanelMode==PLUGIN_PANEL)
		Global->CtrlObject->Cp()->RedrawKeyBar();

	return SetDirectorySuccess;
}


int FileList::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	Global->Elevation->ResetApprove();

	int RetCode;

	if (IsVisible() && Global->Opt->ShowColumnTitles && !MouseEvent->dwEventFlags &&
	        MouseEvent->dwMousePosition.Y==Y1+1 &&
	        MouseEvent->dwMousePosition.X>X1 && MouseEvent->dwMousePosition.X<X1+3)
	{
		if (MouseEvent->dwButtonState)
		{
			if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
				ChangeDisk();
			else
				SelectSortMode();
		}

		return TRUE;
	}

	if (IsVisible() && Global->Opt->ShowPanelScrollbar && IntKeyState.MouseX==X2 &&
	        (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && !(MouseEvent->dwEventFlags & MOUSE_MOVED) && !IsDragging())
	{
		int ScrollY=Y1+1+Global->Opt->ShowColumnTitles;

		if (IntKeyState.MouseY==ScrollY)
		{
			while (IsMouseButtonPressed())
				ProcessKey(Manager::Key(KEY_UP));

			SetFocus();
			return TRUE;
		}

		if (IntKeyState.MouseY==ScrollY+Height-1)
		{
			while (IsMouseButtonPressed())
				ProcessKey(Manager::Key(KEY_DOWN));

			SetFocus();
			return TRUE;
		}

		if (IntKeyState.MouseY>ScrollY && IntKeyState.MouseY<ScrollY+Height-1 && Height>2)
		{
			while (IsMouseButtonPressed())
			{
				CurFile=static_cast<int>((ListData.size() - 1)*(IntKeyState.MouseY-ScrollY)/(Height-2));
				ShowFileList(TRUE);
				SetFocus();
			}

			return TRUE;
		}
	}

	if(MouseEvent->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED && MouseEvent->dwEventFlags!=MOUSE_MOVED)
	{
		int Key = KEY_ENTER;
		if(MouseEvent->dwControlKeyState&SHIFT_PRESSED)
		{
			Key |= KEY_SHIFT;
		}
		if(MouseEvent->dwControlKeyState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED))
		{
			Key|=KEY_CTRL;
		}
		if(MouseEvent->dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
		{
			Key|=KEY_ALT;
		}
		ProcessKey(Manager::Key(Key));
		return TRUE;
	}

	if (Panel::PanelProcessMouse(MouseEvent,RetCode))
		return RetCode;

	if (MouseEvent->dwMousePosition.Y>Y1+Global->Opt->ShowColumnTitles &&
	        MouseEvent->dwMousePosition.Y<Y2-2*Global->Opt->ShowPanelStatus)
	{
		SetFocus();

		if (ListData.empty())
			return TRUE;

		MoveToMouse(MouseEvent);
		assert(CurFile < static_cast<int>(ListData.size()));

		if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
		        MouseEvent->dwEventFlags==DOUBLE_CLICK)
		{
			if (PanelMode==PLUGIN_PANEL)
			{
				FlushInputBuffer(); // !!!
				INPUT_RECORD rec;
				ProcessKeyToInputRecord(VK_RETURN,IntKeyState.ShiftPressed ? PKF_SHIFT:0,&rec);
				int ProcessCode=Global->CtrlObject->Plugins->ProcessKey(hPlugin,&rec,false);
				ProcessPluginCommand();

				if (ProcessCode)
					return TRUE;
			}

			/*$ 21.02.2001 SKV
			  Если пришел DOUBLE_CLICK без предшевствующего ему
			  простого клика, то курсор не перерисовывается.
			  Перересуем его.
			  По идее при нормальном DOUBLE_CLICK, будет
			  двойная перерисовка...
			  Но мы же вызываем Fast=TRUE...
			  Вроде всё должно быть ок.
			*/
			ShowFileList(TRUE);
			FlushInputBuffer();
			ProcessEnter(true, IntKeyState.ShiftPressed!=0, true, false, OFP_NORMAL);
			return TRUE;
		}
		else
		{
			/* $ 11.09.2000 SVS
			   Bug #17: Выделяем при условии, что колонка ПОЛНОСТЬЮ пуста.
			*/
			if ((MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) && !empty)
			{
				DWORD control=MouseEvent->dwControlKeyState&(SHIFT_PRESSED|LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED);

				//вызовем EMenu если он есть
				if (!Global->Opt->RightClickSelect && MouseEvent->dwButtonState == RIGHTMOST_BUTTON_PRESSED && (control == 0 || control == SHIFT_PRESSED) && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu.Id))
				{
					ShowFileList(TRUE);
					Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, nullptr); // EMenu Plugin :-)
					return TRUE;
				}

				if (!MouseEvent->dwEventFlags || MouseEvent->dwEventFlags==DOUBLE_CLICK)
					MouseSelection = !ListData[CurFile].Selected;

				Select(ListData[CurFile], MouseSelection);

				if (SelectedFirst)
					SortFileList(TRUE);
			}
		}

		ShowFileList(TRUE);
		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y<=Y1+1)
	{
		SetFocus();

		if (ListData.empty())
			return TRUE;

		while (IsMouseButtonPressed() && IntKeyState.MouseY<=Y1+1)
		{
			MoveCursor(-1);

			if (IntKeyState.MouseButtonState==RIGHTMOST_BUTTON_PRESSED)
			{
				assert(CurFile < static_cast<int>(ListData.size()));
				Select(ListData[CurFile], MouseSelection);
			}
		}

		if (SelectedFirst)
			SortFileList(TRUE);

		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y>=Y2-2)
	{
		SetFocus();

		if (ListData.empty())
			return TRUE;

		while (IsMouseButtonPressed() && IntKeyState.MouseY>=Y2-2)
		{
			MoveCursor(1);

			if (IntKeyState.MouseButtonState==RIGHTMOST_BUTTON_PRESSED)
			{
				assert(CurFile < static_cast<int>(ListData.size()));
				Select(ListData[CurFile], MouseSelection);
			}
		}

		if (SelectedFirst)
			SortFileList(TRUE);

		return TRUE;
	}

	return FALSE;
}


/* $ 12.09.2000 SVS
  + Опциональное поведение для правой клавиши мыши на пустой панели
*/
void FileList::MoveToMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	int CurColumn=1,ColumnsWidth = 0;
	int PanelX=MouseEvent->dwMousePosition.X-X1-1;
	int Level = 0;

	FOR(const auto& i, ViewSettings.PanelColumns)
	{
		if (Level == ColumnsInGlobal)
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
	int OldCurFile=CurFile;
	CurFile=CurTopFile+MouseEvent->dwMousePosition.Y-Y1-1-Global->Opt->ShowColumnTitles;

	if (CurColumn>1)
		CurFile+=(CurColumn-1)*Height;

	CorrectPosition();

	/* $ 11.09.2000 SVS
	   Bug #17: Проверим на ПОЛНОСТЬЮ пустую колонку.
	*/
	if (Global->Opt->PanelRightClickRule == 1)
		empty=((CurColumn-1)*Height > static_cast<int>(ListData.size()));
	else if (Global->Opt->PanelRightClickRule == 2 &&
	         (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) &&
	         ((CurColumn-1)*Height > static_cast<int>(ListData.size())))
	{
		CurFile=OldCurFile;
		empty=TRUE;
	}
	else
		empty=FALSE;
}

void FileList::SetViewMode(int Mode)
{
	if (static_cast<size_t>(Mode) >= Global->Opt->ViewSettings.size())
		Mode=VIEW_0;

	bool CurFullScreen=IsFullScreen();
	bool OldOwner=IsColumnDisplayed(OWNER_COLUMN);
	bool OldPacked=IsColumnDisplayed(PACKED_COLUMN);
	bool OldNumLink=IsColumnDisplayed(NUMLINK_COLUMN);
	bool OldNumStreams=IsColumnDisplayed(NUMSTREAMS_COLUMN);
	bool OldStreamsSize=IsColumnDisplayed(STREAMSSIZE_COLUMN);
	bool OldDiz=IsColumnDisplayed(DIZ_COLUMN);
	PrepareViewSettings(Mode,nullptr);
	bool NewOwner=IsColumnDisplayed(OWNER_COLUMN);
	bool NewPacked=IsColumnDisplayed(PACKED_COLUMN);
	bool NewNumLink=IsColumnDisplayed(NUMLINK_COLUMN);
	bool NewNumStreams=IsColumnDisplayed(NUMSTREAMS_COLUMN);
	bool NewStreamsSize=IsColumnDisplayed(STREAMSSIZE_COLUMN);
	bool NewDiz=IsColumnDisplayed(DIZ_COLUMN);
	bool NewAccessTime=IsColumnDisplayed(ADATE_COLUMN);
	int ResortRequired=FALSE;
	string strDriveRoot;
	DWORD FileSystemFlags = 0;
	GetPathRoot(strCurDir,strDriveRoot);

	if (NewPacked && api::GetVolumeInformation(strDriveRoot,nullptr,nullptr,nullptr,&FileSystemFlags,nullptr))
		if (!(FileSystemFlags&FILE_FILE_COMPRESSION))
			NewPacked = false;

	if (!ListData.empty() && PanelMode!=PLUGIN_PANEL &&
	        ((!OldOwner && NewOwner) || (!OldPacked && NewPacked) ||
	         (!OldNumLink && NewNumLink) ||
	         (!OldNumStreams && NewNumStreams) ||
	         (!OldStreamsSize && NewStreamsSize) ||
	         (AccessTimeUpdateRequired && NewAccessTime)))
		Update(UPDATE_KEEP_SELECTION);

	if (!OldDiz && NewDiz)
		ReadDiz();

	if ((ViewSettings.Flags&PVS_FULLSCREEN) && !CurFullScreen)
	{
		if (Y2>0)
			SetPosition(0,Y1,ScrX,Y2);

		ViewMode=Mode;
	}
	else
	{
		if (!(ViewSettings.Flags&PVS_FULLSCREEN) && CurFullScreen)
		{
			if (Y2>0)
			{
				if (this==Global->CtrlObject->Cp()->LeftPanel)
					SetPosition(0,Y1,ScrX/2-Global->Opt->WidthDecrement,Y2);
				else
					SetPosition(ScrX/2+1-Global->Opt->WidthDecrement,Y1,ScrX,Y2);
			}

			ViewMode=Mode;
		}
		else
		{
			ViewMode=Mode;
			Global->FrameManager->RefreshFrame();
		}
	}

	if (PanelMode==PLUGIN_PANEL)
	{
		string strColumnTypes,strColumnWidths;
//    SetScreenPosition();
		ViewSettingsToText(ViewSettings.PanelColumns, strColumnTypes, strColumnWidths);
		ProcessPluginEvent(FE_CHANGEVIEWMODE, UNSAFE_CSTR(strColumnTypes));
	}

	if (ResortRequired)
	{
		SortFileList(TRUE);
		ShowFileList(TRUE);
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

		if (AnotherPanel->GetType()==TREE_PANEL)
			AnotherPanel->Redraw();
	}
}

void FileList::ApplySortMode(int Mode)
{
	SortMode = Mode;

	if (!ListData.empty())
		SortFileList(TRUE);

	ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
	Global->FrameManager->RefreshFrame();
}

void FileList::SetSortMode(int Mode, bool KeepOrder)
{
	if (!KeepOrder)
	{
		static bool InvertByDefault[] =
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
			false  // BY_CUSTOMDATA,
		};
		static_assert(ARRAYSIZE(InvertByDefault) == SORTMODE_COUNT, "incomplete InvertByDefault array");

		assert(Mode < SORTMODE_COUNT);

		ReverseSortOrder = (SortMode==Mode && Global->Opt->ReverseSort)? !ReverseSortOrder : InvertByDefault[Mode];
	}

	ApplySortMode(Mode);
}

void FileList::SetCustomSortMode(int Mode, bool KeepOrder, bool InvertByDefault)
{
	if (Mode >= SORTMODE_COUNT)
	{
		if (!KeepOrder)
		{
			ReverseSortOrder = (SortMode == Mode && Global->Opt->ReverseSort)? !ReverseSortOrder : InvertByDefault;
		}

		ApplySortMode(Mode);
	}
}

void FileList::ChangeNumericSort(bool Mode)
{
	Panel::ChangeNumericSort(Mode);
	SortFileList(TRUE);
	ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
	Show();
}

void FileList::ChangeCaseSensitiveSort(bool Mode)
{
	Panel::ChangeCaseSensitiveSort(Mode);
	SortFileList(TRUE);
	ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
	Show();
}

void FileList::ChangeDirectoriesFirst(bool Mode)
{
	Panel::ChangeDirectoriesFirst(Mode);
	SortFileList(TRUE);
	ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
	Show();
}

int FileList::GoToFile(long idxItem)
{
	if (static_cast<size_t>(idxItem) < ListData.size())
	{
		CurFile=idxItem;
		CorrectPosition();
		return TRUE;
	}

	return FALSE;
}

int FileList::GoToFile(const string& Name,BOOL OnlyPartName)
{
	return GoToFile(FindFile(Name,OnlyPartName));
}


long FileList::FindFile(const string& Name,BOOL OnlyPartName)
{
	long II = -1;
	for (long I=0; I < static_cast<int>(ListData.size()); I++)
	{
		const wchar_t *CurPtrName=OnlyPartName?PointToName(ListData[I].strName):ListData[I].strName.data();

		if (Name == CurPtrName)
			return I;

		if (II < 0 && !StrCmpI(Name.data(),CurPtrName))
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
	if (static_cast<size_t>(StartPos) < ListData.size())
		for (long I=StartPos; I < static_cast<int>(ListData.size()); I++)
		{
			if (CmpName(Name.data(),ListData[I].strName.data(),true))
				if (!TestParentFolderName(ListData[I].strName))
					return I;
		}

	return -1;
}


int FileList::IsSelected(const string& Name)
{
	long Pos=FindFile(Name);
	return Pos!=-1 && (ListData[Pos].Selected || (!SelFileCount && Pos==CurFile));
}

int FileList::IsSelected(size_t idxItem)
{
	if (idxItem < ListData.size()) // BUGBUG
		return ListData[idxItem].Selected; //  || (Sel!FileCount && idxItem==CurFile) ???
	return FALSE;
}

bool FileList::FilterIsEnabled()
{
	return Filter && Filter->IsEnabledOnPanel();
}

bool FileList::FileInFilter(size_t idxItem)
{
	if ( ( idxItem < ListData.size() ) && ( !Filter || !Filter->IsEnabledOnPanel() || Filter->FileInFilter(&ListData[idxItem]) ) ) // BUGBUG, cast
		return true;
	return false;
}

// $ 02.08.2000 IG  Wish.Mix #21 - при нажатии '/' или '\' в QuickSerach переходим на директорию
int FileList::FindPartName(const string& Name,int Next,int Direct)
{
#if !defined(Mantis_698)
	int DirFind = 0;
	string strMask = Name;

	if (!Name.empty() && IsSlash(Name.back()))
	{
		DirFind = 1;
		strMask.pop_back();
	}

	strMask += L"*";

	Panel::exclude_sets(strMask);

	for (int I=CurFile+(Next?Direct:0); I >= 0 && I < static_cast<int>(ListData.size()); I+=Direct)
	{
		if (CmpName(strMask.data(),ListData[I].strName.data(),true,I==CurFile))
		{
			if (!TestParentFolderName(ListData[I].strName))
			{
				if (!DirFind || (ListData[I].FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				{
					CurFile=I;
					CurTopFile=CurFile-(Y2-Y1)/2;
					ShowFileList(TRUE);
					return TRUE;
				}
			}
		}
	}

	for (int I=(Direct > 0)?0:static_cast<int>(ListData.size()-1); (Direct > 0) ? I < CurFile:I > CurFile; I+=Direct)
	{
		if (CmpName(strMask.data(),ListData[I].strName.data(),true))
		{
			if (!TestParentFolderName(ListData[I].strName))
			{
				if (!DirFind || (ListData[I].FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				{
					CurFile=I;
					CurTopFile=CurFile-(Y2-Y1)/2;
					ShowFileList(TRUE);
					return TRUE;
				}
			}
		}
	}

	return FALSE;
#else
	// Mantis_698
	// АХТУНГ! В разработке
	string Dest;
	int DirFind = 0;
	string strMask = Name;
	Upper(strMask);

	if (!Name.empty() && IsSlash(Name.back()))
	{
		DirFind = 1;
		strMask.pop_back();
	}

/*
	strMask += L"*";

	Panel::exclude_sets(strMask);
*/

	for (int I=CurFile+(Next?Direct:0); I >= 0 && I < ListData.size(); I+=Direct)
	{
		if (GetPlainString(Dest,I) && Upper(Dest).find(strMask) != string::npos)
		//if (CmpName(strMask,ListData[I].strName,true,I==CurFile))
		{
			if (!TestParentFolderName(ListData[I].strName))
			{
				if (!DirFind || (ListData[I].FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				{
					CurFile=I;
					CurTopFile=CurFile-(Y2-Y1)/2;
					ShowFileList(TRUE);
					return TRUE;
				}
			}
		}
	}

	for (int I=(Direct > 0)?0:ListData.size()-1; (Direct > 0) ? I < CurFile:I > CurFile; I+=Direct)
	{
		if (GetPlainString(Dest,I) && Upper(Dest).find(strMask) != string::npos)
		{
			if (!TestParentFolderName(ListData[I].strName))
			{
				if (!DirFind || (ListData[I].FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				{
					CurFile=I;
					CurTopFile=CurFile-(Y2-Y1)/2;
					ShowFileList(TRUE);
					return TRUE;
				}
			}
		}
	}

	return FALSE;
#endif
}

// собрать в одну строку все данные в отображаемых колонках
bool FileList::GetPlainString(string& Dest, int ListPos) const
{
	Dest.clear();
#if defined(Mantis_698)
	if (ListPos < FileCount)
	{
		unsigned __int64 *ColumnTypes=ViewSettings.ColumnType;
		int ColumnCount=ViewSettings.ColumnCount;
		int *ColumnWidths=ViewSettings.ColumnWidth;

		for (int K=0; K<ColumnCount; K++)
		{
			int ColumnType=static_cast<int>(ColumnTypes[K] & 0xff);
			int ColumnWidth=ColumnWidths[K];
			if (ColumnType>=CUSTOM_COLUMN0 && ColumnType<=CUSTOM_COLUMN9)
			{
				size_t ColumnNumber=ColumnType-CUSTOM_COLUMN0;
				const wchar_t *ColumnData=nullptr;

				if (ColumnNumber<ListData[ListPos].CustomColumnNumber)
					ColumnData=ListData[ListPos].CustomColumnData[ColumnNumber];

				if (!ColumnData)
				{
					ColumnData=ListData[ListPos].strCustomData;//L"";
				}
				Dest.append(ColumnData);
			}
			else
			{
				switch (ColumnType)
				{
					case NAME_COLUMN:
					{
						unsigned __int64 ViewFlags=ColumnTypes[K];
						const wchar_t *NamePtr = ShowShortNames && !ListData[ListPos].strShortName.empty() ? ListData[ListPos].strShortName:ListData[ListPos].strName;

						string strNameCopy;
						if (!(ListData[ListPos].FileAttr & FILE_ATTRIBUTE_DIRECTORY) && (ViewFlags & COLUMN_NOEXTENSION))
						{
							const wchar_t *ExtPtr = PointToExt(NamePtr);
							if (ExtPtr)
							{
								strNameCopy.assign(NamePtr, ExtPtr-NamePtr);
								NamePtr = strNameCopy;
							}
						}

						const wchar_t *NameCopy = NamePtr;

						if (ViewFlags & COLUMN_NAMEONLY)
						{
							//BUGBUG!!!
							// !!! НЕ УВЕРЕН, но то, что отображается пустое
							// пространство вместо названия - бага
							NamePtr=PointToFolderNameIfFolder(NamePtr);
						}

						Dest.append(NamePtr);
						break;
					}

					case EXTENSION_COLUMN:
					{
						const wchar_t *ExtPtr = nullptr;
						if (!(ListData[ListPos].FileAttr & FILE_ATTRIBUTE_DIRECTORY))
						{
							const wchar_t *NamePtr = ShowShortNames && !ListData[ListPos].strShortName.empty()? ListData[ListPos].strShortName:ListData[ListPos].strName;
							ExtPtr = PointToExt(NamePtr);
						}
						if (ExtPtr && *ExtPtr) ExtPtr++; else ExtPtr = L"";

						Dest.append(ExtPtr);
						break;
					}

					case SIZE_COLUMN:
					case PACKED_COLUMN:
					case STREAMSSIZE_COLUMN:
					{
						Dest.append(FormatStr_Size(
							ListData[ListPos].FileSize,
							ListData[ListPos].AllocationSize,
							ListData[ListPos].StreamsSize,
							ListData[ListPos].strName,
							ListData[ListPos].FileAttr,
							ListData[ListPos].ShowFolderSize,
							ListData[ListPos].ReparseTag,
							ColumnType,
							ColumnTypes[K],
							ColumnWidth,
							strCurDir.data()));
						break;
					}

					case DATE_COLUMN:
					case TIME_COLUMN:
					case WDATE_COLUMN:
					case CDATE_COLUMN:
					case ADATE_COLUMN:
					case CHDATE_COLUMN:
					{
						FILETIME *FileTime;

						switch (ColumnType)
						{
							case CDATE_COLUMN:
								FileTime=&ListData[ListPos].CreationTime;
								break;
							case ADATE_COLUMN:
								FileTime=&ListData[ListPos].AccessTime;
								break;
							case CHDATE_COLUMN:
								FileTime=&ListData[ListPos].ChangeTime;
								break;
							case DATE_COLUMN:
							case TIME_COLUMN:
							case WDATE_COLUMN:
							default:
								FileTime=&ListData[ListPos].WriteTime;
								break;
						}

						Dest.append(FormatStr_DateTime(FileTime,ColumnType,ColumnTypes[K],ColumnWidth));
						break;
					}

					case ATTR_COLUMN:
					{
						Dest.append(FormatStr_Attribute(ListData[ListPos].FileAttr,ColumnWidth));
						break;
					}

					case DIZ_COLUMN:
					{
						string strDizText=ListData[ListPos].DizText ? ListData[ListPos].DizText:L"";
						Dest.append(strDizText);
						break;
					}

					case OWNER_COLUMN:
					{
						Dest.append(ListData[ListPos].strOwner);
						break;
					}

					case NUMLINK_COLUMN:
					{
						Dest.append(str_printf(L"%d",ListData[ListPos].NumberOfLinks));
						break;
					}

					case NUMSTREAMS_COLUMN:
					{
						Dest.append(L"%d",ListData[ListPos].NumberOfStreams);
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
	assert(ListData.empty() || !(ReturnCurrentFile||!SelFileCount) || (CurFile < static_cast<int>(ListData.size())));
	return !ListData.empty()? ((ReturnCurrentFile||!SelFileCount)?(TestParentFolderName(ListData[CurFile].strName)?0:1):SelFileCount):0;
}

size_t FileList::GetRealSelCount() const
{
	return !ListData.empty()? SelFileCount : 0;
}


int FileList::GetSelName(string *strName, DWORD &FileAttr, string *strShortName, api::FAR_FIND_DATA *fde)
{
	if (!strName)
	{
		GetSelPosition=0;
		LastSelPosition=-1;
		return TRUE;
	}

	if (!SelFileCount || ReturnCurrentFile)
	{
		if (!GetSelPosition && CurFile < static_cast<int>(ListData.size()))
		{
			GetSelPosition=1;
			*strName = ListData[CurFile].strName;

			if (strShortName)
			{
				*strShortName = ListData[CurFile].strShortName;

				if (strShortName->empty())
					*strShortName = *strName;
			}

			FileAttr=ListData[CurFile].FileAttr;
			LastSelPosition=CurFile;

			if (fde)
			{
				fde->dwFileAttributes=ListData[CurFile].FileAttr;
				fde->ftCreationTime=ListData[CurFile].CreationTime;
				fde->ftLastAccessTime=ListData[CurFile].AccessTime;
				fde->ftLastWriteTime=ListData[CurFile].WriteTime;
				fde->ftChangeTime=ListData[CurFile].ChangeTime;
				fde->nFileSize=ListData[CurFile].FileSize;
				fde->nAllocationSize=ListData[CurFile].AllocationSize;
				fde->strFileName = ListData[CurFile].strName;
				fde->strAlternateFileName = ListData[CurFile].strShortName;
				if (fde->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
				{
					fde->dwReserved0 = ListData[CurFile].ReparseTag;
				}
			}

			return TRUE;
		}
		else
			return FALSE;
	}

	while (GetSelPosition < static_cast<int>(ListData.size()))
		if (ListData[GetSelPosition++].Selected)
		{
			const auto& PrevItem = ListData[GetSelPosition-1];
			*strName = PrevItem.strName;

			if (strShortName)
			{
				*strShortName = PrevItem.strShortName;

				if (strShortName->empty())
					*strShortName = *strName;
			}

			FileAttr=PrevItem.FileAttr;
			LastSelPosition=GetSelPosition-1;

			if (fde)
			{
				fde->dwFileAttributes = PrevItem.FileAttr;
				fde->ftCreationTime = PrevItem.CreationTime;
				fde->ftLastAccessTime = PrevItem.AccessTime;
				fde->ftLastWriteTime = PrevItem.WriteTime;
				fde->ftChangeTime = PrevItem.ChangeTime;
				fde->nFileSize = PrevItem.FileSize;
				fde->nAllocationSize = PrevItem.AllocationSize;
				fde->strFileName = PrevItem.strName;
				fde->strAlternateFileName = PrevItem.strShortName;
			}

			return TRUE;
		}

	return FALSE;
}


void FileList::ClearLastGetSelection()
{
	if (LastSelPosition>=0 && LastSelPosition < static_cast<int>(ListData.size()))
		Select(ListData[LastSelPosition], FALSE);
}


void FileList::UngetSelName()
{
	GetSelPosition=LastSelPosition;
}


unsigned __int64 FileList::GetLastSelectedSize() const
{
	if (LastSelPosition>=0 && LastSelPosition < static_cast<int>(ListData.size()))
		return ListData[LastSelPosition].FileSize;

	return -1;
}


const FileListItem* FileList::GetLastSelectedItem() const
{
	if (LastSelPosition>=0 && LastSelPosition < static_cast<int>(ListData.size()))
	{
		return &ListData[LastSelPosition];
	}

	return nullptr;
}

int FileList::GetCurName(string &strName, string &strShortName) const
{
	if (ListData.empty())
	{
		strName.clear();
		strShortName.clear();
		return FALSE;
	}

	assert(CurFile < static_cast<int>(ListData.size()));
	strName = ListData[CurFile].strName;
	strShortName = ListData[CurFile].strShortName;

	if (strShortName.empty())
		strShortName = strName;

	return TRUE;
}

int FileList::GetCurBaseName(string &strName, string &strShortName) const
{
	if (ListData.empty())
	{
		strName.clear();
		strShortName.clear();
		return FALSE;
	}

	if (PanelMode==PLUGIN_PANEL && !PluginsList.empty()) // для плагинов
	{
		strName = PointToName(PluginsList.front().m_HostFile);
	}
	else if (PanelMode==NORMAL_PANEL)
	{
		assert(CurFile < static_cast<int>(ListData.size()));
		strName = ListData[CurFile].strName;
		strShortName = ListData[CurFile].strShortName;
	}

	if (strShortName.empty())
		strShortName = strName;

	return TRUE;
}

long FileList::SelectFiles(int Mode,const wchar_t *Mask)
{
	filemasks FileMask; // Класс для работы с масками
	FarDialogItem SelectDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,51,5,0,nullptr,nullptr,0,L""},
		{DI_EDIT,5,2,49,2,0,L"Masks",nullptr,DIF_FOCUS|DIF_HISTORY,L""},
		{DI_TEXT,-1,3,0,3,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,4,0,4,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,4,0,4,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MSelectFilter)},
		{DI_BUTTON,0,4,0,4,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	auto SelectDlg = MakeDialogItemsEx(SelectDlgData);
	FileFilter Filter(this,FFT_SELECT);
	bool bUseFilter = false;
	static string strPrevMask=L"*.*";
	/* $ 20.05.2002 IS
	   При обработке маски, если работаем с именем файла на панели,
	   берем каждую квадратную скобку в имени при образовании маски в скобки,
	   чтобы подобные имена захватывались полученной маской - это специфика,
	   диктуемая CmpName.
	*/
	string strMask=L"*.*", strRawMask;
	bool WrapBrackets=false; // говорит о том, что нужно взять кв.скобки в скобки

	if (CurFile >= static_cast<int>(ListData.size()))
		return 0;

	int RawSelection=FALSE;

	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		RawSelection=(Info.Flags & OPIF_RAWSELECTION);
	}

	string strCurName=(ShowShortNames && !ListData[CurFile].strShortName.empty()? ListData[CurFile].strShortName : ListData[CurFile].strName);

	if (Mode==SELECT_ADDEXT || Mode==SELECT_REMOVEEXT)
	{
		size_t pos = strCurName.rfind(L'.');

		if (pos != string::npos)
		{
			// Учтем тот момент, что расширение может содержать символы-разделители
			strRawMask = str_printf(L"\"*.%s\"", strCurName.data()+pos+1);
			WrapBrackets=true;
		}
		else
		{
			strMask = L"*.";
		}

		Mode=(Mode==SELECT_ADDEXT) ? SELECT_ADD:SELECT_REMOVE;
	}
	else
	{
		if (Mode==SELECT_ADDNAME || Mode==SELECT_REMOVENAME)
		{
			// Учтем тот момент, что имя может содержать символы-разделители
			strRawMask=L"\"";
			strRawMask+=strCurName;
			size_t pos = strRawMask.rfind(L'.');

			if (pos != string::npos && pos!=strRawMask.size()-1)
				strRawMask.resize(pos);

			strRawMask += L".*\"";
			WrapBrackets=true;
			Mode=(Mode==SELECT_ADDNAME) ? SELECT_ADD:SELECT_REMOVE;
		}
		else
		{
			if (Mode==SELECT_ADD || Mode==SELECT_REMOVE)
			{
				SelectDlg[1].strData = strPrevMask;

				if (Mode==SELECT_ADD)
					SelectDlg[0].strData = MSG(MSelectTitle);
				else
					SelectDlg[0].strData = MSG(MUnselectTitle);

				{
					Dialog Dlg(SelectDlg);
					Dlg.SetHelp(L"SelectFiles");
					Dlg.SetPosition(-1,-1,55,7);

					for (;;)
					{
						Dlg.ClearDone();
						Dlg.Process();

						if (Dlg.GetExitCode()==4 && Filter.FilterEdit())
						{
							//Рефреш текущему времени для фильтра сразу после выхода из диалога
							Filter.UpdateCurrentTime();
							bUseFilter = true;
							break;
						}

						if (Dlg.GetExitCode()!=3)
							return 0;

						strMask = SelectDlg[1].strData;

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
		const wchar_t *src = strRawMask.data();
		strMask.clear();

		while (*src)
		{
			if (*src==L']' || *src==L'[')
			{
				strMask += L'[';
				strMask += *src;
				strMask += L']';
			}
			else
			{
				strMask += *src;
			}

			src++;
		}
	}

	long workCount=0;

	if (bUseFilter || FileMask.Set(strMask, FMF_SILENT)) // Скомпилируем маски файлов и работаем
	{                                                // дальше в зависимости от успеха компиляции
		std::for_each(RANGE(ListData, i)
		{
			int Match=FALSE;

			if (Mode==SELECT_INVERT || Mode==SELECT_INVERTALL)
				Match=TRUE;
			else
			{
				if (bUseFilter)
					Match = Filter.FileInFilter(&i);
				else
					Match=FileMask.Compare((this->ShowShortNames && !i.strShortName.empty()) ? i.strShortName : i.strName);
			}

			if (Match)
			{
				int Selection = 0;
				switch (Mode)
				{
					case SELECT_ADD:
					case SELECT_ADDMASK:
						Selection=1;
						break;
					case SELECT_REMOVE:
					case SELECT_REMOVEMASK:
						Selection=0;
						break;
					case SELECT_INVERT:
					case SELECT_INVERTALL:
					case SELECT_INVERTMASK:
						Selection=!i.Selected;
						break;
				}

				if (bUseFilter || !(i.FileAttr & FILE_ATTRIBUTE_DIRECTORY) || Global->Opt->SelectFolders ||
				        !Selection || RawSelection || Mode==SELECT_INVERTALL || Mode==SELECT_INVERTMASK)
				{
					this->Select(i, Selection);
					workCount++;
				}
			}
		});
	}

	if (SelectedFirst)
		SortFileList(TRUE);

	ShowFileList(TRUE);

	return workCount;
}

void FileList::UpdateViewPanel()
{
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

	if (!ListData.empty() && AnotherPanel->IsVisible() &&
	        AnotherPanel->GetType()==QVIEW_PANEL && SetCurPath())
	{
		QuickView *ViewPanel=(QuickView *)AnotherPanel;
		assert(CurFile < static_cast<int>(ListData.size()));
		FileListItem *CurPtr = &ListData[CurFile];

		if (PanelMode!=PLUGIN_PANEL ||
		        Global->CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARGETFILE))
		{
			if (TestParentFolderName(CurPtr->strName))
				ViewPanel->ShowFile(strCurDir,FALSE,nullptr);
			else
				ViewPanel->ShowFile(CurPtr->strName,FALSE,nullptr);
		}
		else if (!(CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			string strTempDir,strFileName;
			strFileName = CurPtr->strName;

			if (!FarMkTempEx(strTempDir))
				return;

			api::CreateDirectory(strTempDir,nullptr);
			PluginPanelItem PanelItem;
			FileListToPluginItem(*CurPtr, &PanelItem);
			int Result=Global->CtrlObject->Plugins->GetFile(hPlugin,&PanelItem,strTempDir,strFileName,OPM_SILENT|OPM_VIEW|OPM_QUICKVIEW);
			FreePluginPanelItem(PanelItem);

			if (!Result)
			{
				ViewPanel->ShowFile(L"",FALSE,nullptr);
				api::RemoveDirectory(strTempDir);
				return;
			}

			ViewPanel->ShowFile(strFileName,TRUE,nullptr);
		}
		else if (!TestParentFolderName(CurPtr->strName))
			ViewPanel->ShowFile(CurPtr->strName,FALSE,hPlugin);
		else
			ViewPanel->ShowFile(L"",FALSE,nullptr);

		if (ViewPanel->Destroyed())
			return;

		SetTitle();
	}
}


void FileList::CompareDir()
{
	FileList *Another=(FileList *)Global->CtrlObject->Cp()->GetAnotherPanel(this);

	if (Another->GetType()!=FILE_PANEL || !Another->IsVisible())
	{
		Message(MSG_WARNING,1,MSG(MCompareTitle),MSG(MCompareFilePanelsRequired1),
		        MSG(MCompareFilePanelsRequired2),MSG(MOk));
		return;
	}

	Global->ScrBuf->Flush();
	// полностью снимаем выделение с обоих панелей
	ClearSelection();
	Another->ClearSelection();
	const wchar_t *PtrTempName1, *PtrTempName2;

	// помечаем ВСЕ, кроме каталогов на активной панели
	std::for_each(RANGE(ListData, i)
	{
		if (!(i.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
			this->Select(i, TRUE);
	});

	// помечаем ВСЕ, кроме каталогов на пассивной панели
	std::for_each(RANGE(Another->ListData, i)
	{
		if (!(i.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
			Another->Select(i, TRUE);
	});

	int CompareFatTime=FALSE;

	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

		if (Info.Flags & OPIF_COMPAREFATTIME)
			CompareFatTime=TRUE;

	}

	if (Another->PanelMode==PLUGIN_PANEL && !CompareFatTime)
	{
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(Another->hPlugin,&Info);

		if (Info.Flags & OPIF_COMPAREFATTIME)
			CompareFatTime=TRUE;

	}

	if (PanelMode==NORMAL_PANEL && Another->PanelMode==NORMAL_PANEL)
	{
		string strFileSystemName1, strFileSystemName2;
		string strRoot1, strRoot2;
		GetPathRoot(strCurDir, strRoot1);
		GetPathRoot(Another->strCurDir, strRoot2);

		if (api::GetVolumeInformation(strRoot1,nullptr,nullptr,nullptr,nullptr,&strFileSystemName1) &&
		        api::GetVolumeInformation(strRoot2,nullptr,nullptr,nullptr,nullptr,&strFileSystemName2))
			if (StrCmpI(strFileSystemName1, strFileSystemName2))
				CompareFatTime=TRUE;
	}

	// теперь начнем цикл по снятию выделений
	// каждый элемент активной панели...
	FOR(auto& i, ListData)
	{
		if (i.FileAttr & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		// ...сравниваем с элементом пассивной панели...
		FOR(auto& j, Another->ListData)
		{
			if (j.FileAttr & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			PtrTempName1=PointToName(i.strName);
			PtrTempName2=PointToName(j.strName);

			if (!StrCmpI(PtrTempName1,PtrTempName2))
			{
				int Cmp=0;
				if (CompareFatTime)
				{
					WORD DosDate,DosTime,AnotherDosDate,AnotherDosTime;
					FileTimeToDosDateTime(&i.WriteTime,&DosDate,&DosTime);
					FileTimeToDosDateTime(&j.WriteTime,&AnotherDosDate,&AnotherDosTime);
					DWORD FullDosTime,AnotherFullDosTime;
					FullDosTime=((DWORD)DosDate<<16)+DosTime;
					AnotherFullDosTime=((DWORD)AnotherDosDate<<16)+AnotherDosTime;
					int D=FullDosTime-AnotherFullDosTime;

					if (D>=-1 && D<=1)
						Cmp=0;
					else
						Cmp=(FullDosTime<AnotherFullDosTime) ? -1:1;
				}
				else
				{
					Cmp = CompareFileTime(i.WriteTime, j.WriteTime);
				}

				if (!Cmp && (i.FileSize != j.FileSize))
					continue;

				if (Cmp < 1 && i.Selected)
					Select(i, FALSE);

				if (Cmp > -1 && j.Selected)
					Another->Select(j, FALSE);

				if (Another->PanelMode!=PLUGIN_PANEL)
					break;
			}
		}
	}

	if (SelectedFirst)
		SortFileList(TRUE);

	Redraw();
	Another->Redraw();

	if (!SelFileCount && !Another->SelFileCount)
		Message(0,1,MSG(MCompareTitle),MSG(MCompareSameFolders1),MSG(MCompareSameFolders2),MSG(MOk));
}

void FileList::CopyFiles(bool bMoved)
{
	bool RealNames=false;
	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		RealNames = (Info.Flags&OPIF_REALNAMES) == OPIF_REALNAMES;
	}

	if (PanelMode!=PLUGIN_PANEL || RealNames)
	{
		string CopyData;
		string strSelName, strSelShortName;
		DWORD FileAttr;
		GetSelName(nullptr,FileAttr);
		while (GetSelName(&strSelName, FileAttr, &strSelShortName))
		{
			if (TestParentFolderName(strSelName) && TestParentFolderName(strSelShortName))
			{
				strSelName.resize(1);
				strSelShortName.resize(1);
			}
			if (!CreateFullPathName(strSelName,strSelShortName,FileAttr,strSelName,FALSE))
			{
				break;
			}
			CopyData += strSelName;
			CopyData.push_back(L'\0');
		}
		if(!CopyData.empty())
		{
			Clipboard clip;
			if(clip.Open())
			{
				clip.SetHDROP(CopyData.data(), (CopyData.size()+1)*sizeof(wchar_t),bMoved);
			}
		}
	}
}

void FileList::CopyNames(bool FillPathName, bool UNC)
{
	OpenPanelInfo Info={};
	string CopyData;
	string strSelName, strSelShortName, strQuotedName;
	DWORD FileAttr;

	if (PanelMode==PLUGIN_PANEL)
	{
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
	}

	GetSelName(nullptr,FileAttr);

	while (GetSelName(&strSelName,FileAttr,&strSelShortName))
	{
		if (!CopyData.empty())
		{
			CopyData += L"\r\n";
		}

		strQuotedName = (ShowShortNames && !strSelShortName.empty()) ? strSelShortName:strSelName;

		if (FillPathName)
		{
			if (PanelMode!=PLUGIN_PANEL)
			{
				/* $ 14.02.2002 IS
				   ".." в текущем каталоге обработаем как имя текущего каталога
				*/
				if (TestParentFolderName(strQuotedName) && TestParentFolderName(strSelShortName))
				{
					strQuotedName.resize(1);
					strSelShortName.resize(1);
				}

				if (!CreateFullPathName(strQuotedName,strSelShortName,FileAttr,strQuotedName,UNC))
				{
					break;
				}
			}
			else
			{
				string strFullName = NullToEmpty(Info.CurDir);

				if (Global->Opt->PanelCtrlFRule && (ViewSettings.Flags&PVS_FOLDERUPPERCASE))
					Upper(strFullName);

				if (!strFullName.empty())
					AddEndSlash(strFullName);

				if (Global->Opt->PanelCtrlFRule)
				{
					// имя должно отвечать условиям на панели
					if ((ViewSettings.Flags&PVS_FILELOWERCASE) && !(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
						Lower(strQuotedName);

					if (ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE)
						if (!(FileAttr & FILE_ATTRIBUTE_DIRECTORY) && !IsCaseMixed(strQuotedName))
							Lower(strQuotedName);
				}

				strFullName += strQuotedName;
				strQuotedName = strFullName;

				// добавим первый префикс!
				if (PanelMode==PLUGIN_PANEL && Global->Opt->SubstPluginPrefix)
				{
					string strPrefix;

					/* $ 19.11.2001 IS оптимизация по скорости :) */
					if (!AddPluginPrefix(static_cast<FileList *>(Global->CtrlObject->Cp()->ActivePanel),strPrefix).empty())
					{
						strPrefix += strQuotedName;
						strQuotedName = strPrefix;
					}
				}
			}
		}
		else
		{
			if (TestParentFolderName(strQuotedName) && TestParentFolderName(strSelShortName))
			{
				if (PanelMode==PLUGIN_PANEL)
				{
					strQuotedName=NullToEmpty(Info.CurDir);
				}
				else
				{
					strQuotedName = GetCurDir();
				}

				strQuotedName=PointToName(strQuotedName);
			}
		}

		if (Global->Opt->QuotedName&QUOTEDNAME_CLIPBOARD)
			QuoteSpace(strQuotedName);

		CopyData += strQuotedName;
	}

	SetClipboard(CopyData);
}

void FileList::SetTitle()
{
	if (GetFocus() || Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetType()!=FILE_PANEL)
	{
		string strTitleDir(L"{");

		if (PanelMode==PLUGIN_PANEL)
		{
			OpenPanelInfo Info;
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
			string strPluginTitle = NullToEmpty(Info.PanelTitle);
			RemoveExternalSpaces(strPluginTitle);
			strTitleDir += strPluginTitle;
		}
		else
		{
			strTitleDir += strCurDir;
		}

		strTitleDir += L"}";

		ConsoleTitle::SetFarTitle(strTitleDir);
	}
}


void FileList::ClearSelection()
{
	std::for_each(RANGE(ListData, i)
	{
		this->Select(i, FALSE);
	});

	if (SelectedFirst)
		SortFileList(TRUE);
}


void FileList::SaveSelection()
{
	std::for_each(RANGE(ListData, i)
	{
		i.PrevSelected = i.Selected;
	});
}


void FileList::RestoreSelection()
{
	std::for_each(RANGE(ListData, i)
	{
		int NewSelection = i.PrevSelected;
		i.PrevSelected = i.Selected;
		this->Select(i, NewSelection);
	});

	if (SelectedFirst)
		SortFileList(TRUE);

	Redraw();
}



int FileList::GetFileName(string &strName, int Pos, DWORD &FileAttr) const
{
	if (Pos >= static_cast<int>(ListData.size()))
		return FALSE;

	strName = ListData[Pos].strName;
	FileAttr=ListData[Pos].FileAttr;
	return TRUE;
}


int FileList::GetCurrentPos() const
{
	return CurFile;
}


void FileList::EditFilter()
{
	if (!Filter)
		Filter = std::make_unique<FileFilter>(this,FFT_PANEL);

	Filter->FilterEdit();
}


void FileList::SelectSortMode()
{
	const MenuDataEx InitSortMenuModes[]=
	{
		{MSG(MMenuSortByName),LIF_SELECTED,KEY_CTRLF3},
		{MSG(MMenuSortByExt),0,KEY_CTRLF4},
		{MSG(MMenuSortByWrite),0,KEY_CTRLF5},
		{MSG(MMenuSortBySize),0,KEY_CTRLF6},
		{MSG(MMenuUnsorted),0,KEY_CTRLF7},
		{MSG(MMenuSortByCreation),0,KEY_CTRLF8},
		{MSG(MMenuSortByAccess),0,KEY_CTRLF9},
		{MSG(MMenuSortByChange),0,0},
		{MSG(MMenuSortByDiz),0,KEY_CTRLF10},
		{MSG(MMenuSortByOwner),0,KEY_CTRLF11},
		{MSG(MMenuSortByAllocatedSize),0,0},
		{MSG(MMenuSortByNumLinks),0,0},
		{MSG(MMenuSortByNumStreams),0,0},
		{MSG(MMenuSortByStreamsSize),0,0},
		{MSG(MMenuSortByFullName),0,0},
		{MSG(MMenuSortByCustomData),0,0},
	};
	static_assert(ARRAYSIZE(InitSortMenuModes) == SORTMODE_COUNT, "Incomplete InitSortMenuModes array");

	std::vector<MenuDataEx> SortMenu(ALL_CONST_RANGE(InitSortMenuModes));

	static const MenuDataEx MenuSeparator = { L"",LIF_SEPARATOR };

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
					MenuDataEx item = { mpr->Values[i+2].String };
					SortMenu.emplace_back(item);
				}
			}
			else
				mpr = nullptr;
		}
	}

	static const int SortModes[]=
	{
		BY_NAME,
		BY_EXT,
		BY_MTIME,
		BY_SIZE,
		UNSORTED,
		BY_CTIME,
		BY_ATIME,
		BY_CHTIME,
		BY_DIZ,
		BY_OWNER,
		BY_COMPRESSEDSIZE,
		BY_NUMLINKS,
		BY_NUMSTREAMS,
		BY_STREAMSSIZE,
		BY_FULLNAME,
		BY_CUSTOMDATA
	};
	static_assert(ARRAYSIZE(SortModes) == SORTMODE_COUNT, "Incomplete SortModes array");

	{
		const auto ItemIterator = std::find(ALL_CONST_RANGE(SortModes), SortMode);
		const wchar_t Check = ReverseSortOrder? L'-' : L'+';

		if (ItemIterator != std::cend(SortModes))
		{
			SortMenu[ItemIterator - std::cbegin(SortModes)].SetCheck(Check);
		}
		else if (mpr)
		{
			for (size_t i=0; i < mpr->Count; i += 3)
			{
				if (mpr->Values[i].Double == SortMode)
				{
					SortMenu[ARRAYSIZE(SortModes) + 1 + i/3].SetCheck(Check);
					break;
				}
			}
		}
	}

	enum SortOptions
	{
		SortOptUseNumeric,
		SortOptUseCaseSensitive,
		SortOptUseGroups,
		SortOptSelectedFirst,
		SortOptDirectoriesFirst,

		SortOptCount
	};
	const MenuDataEx InitSortMenuOptions[]=
	{
		{MSG(MMenuSortUseNumeric), NumericSort? (DWORD)MIF_CHECKED : 0, 0},
		{MSG(MMenuSortUseCaseSensitive), CaseSensitiveSort? (DWORD)MIF_CHECKED : 0, 0},
		{MSG(MMenuSortUseGroups), GetSortGroups()? (DWORD)MIF_CHECKED : 0, KEY_SHIFTF11},
		{MSG(MMenuSortSelectedFirst), SelectedFirst? (DWORD)MIF_CHECKED : 0, KEY_SHIFTF12},
		{MSG(MMenuSortDirectoriesFirst), DirectoriesFirst? (DWORD)MIF_CHECKED : 0, 0},
	};
	static_assert(ARRAYSIZE(InitSortMenuOptions) == SortOptCount, "Incomplete InitSortMenuOptions array");

	SortMenu.reserve(SortMenu.size() + 1 + ARRAYSIZE(InitSortMenuOptions)); // + 1 for separator
	SortMenu.push_back(MenuSeparator);
	SortMenu.insert(SortMenu.end(), ALL_CONST_RANGE(InitSortMenuOptions));

	int SortCode = -1;
	bool InvertPressed = true;
	bool PlusPressed = false;

	{
		std::vector<string> MenuStrings(SortMenu.size());
		VMenu::AddHotkeys(MenuStrings, SortMenu.data(), SortMenu.size());

		VMenu2 SortModeMenu(MSG(MMenuSortTitle), SortMenu.data(), SortMenu.size(), 0);
		SortModeMenu.SetHelp(L"PanelCmdSort");
		SortModeMenu.SetPosition(X1+4,-1,0,0);
		SortModeMenu.SetFlags(VMENU_WRAPMODE);
		SortModeMenu.SetId(SelectSortModeId);

		SortCode=SortModeMenu.Run([&](int Key)->int
		{
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
				SortModeMenu.Close(SortModeMenu.GetSelectPos());
			}
			return KeyProcessed;
		});
	}

	if (SortCode<0)
	{
		return;
	}

	// predefined sort modes
	if (SortCode<(int)ARRAYSIZE(SortModes))
	{
		bool KeepOrder = false;

		if (!InvertPressed)
		{
			ReverseSortOrder = !PlusPressed;
			KeepOrder = true;
		}

		SetSortMode(SortModes[SortCode], KeepOrder);
	}
	// custom sort modes
	else if (SortCode>=(int)ARRAYSIZE(SortModes) + 1 && SortCode<(int)(ARRAYSIZE(SortModes) + 1 + extra - 1))
	{
		const int index = 3*(SortCode-ARRAYSIZE(SortModes)-1);
		const int mode = (int)mpr->Values[index].Double;
		const bool InvertByDefault = mpr->Values[index+1].Boolean != 0;

		bool KeepOrder = false;

		if (!InvertPressed)
		{
			ReverseSortOrder = !PlusPressed;
			KeepOrder = true;
		}

		SetCustomSortMode(mode, KeepOrder, InvertByDefault);
	}
	// sort options
	else
	{
		auto Switch = [&](bool CurrentState)
		{
			return PlusPressed? true : InvertPressed? !CurrentState : false;
		};

		switch (SortCode - ARRAYSIZE(SortModes) - extra - 1) // -1 for separator
		{
		case SortOptUseNumeric:
			ChangeNumericSort(Switch(NumericSort));
			break;

		case SortOptUseCaseSensitive:
			ChangeCaseSensitiveSort(Switch(CaseSensitiveSort));
			break;

		case SortOptUseGroups:
			if (SortGroups != Switch(SortGroups))
				ProcessKey(Manager::Key(KEY_SHIFTF11));
			break;

		case SortOptSelectedFirst:
			if (SelectedFirst != Switch(SelectedFirst))
				ProcessKey(Manager::Key(KEY_SHIFTF12));
			break;

		case SortOptDirectoriesFirst:
			ChangeDirectoriesFirst(Switch(DirectoriesFirst));
			break;
		}
	}
}


void FileList::DeleteDiz(const string& Name, const string& ShortName)
{
	if (PanelMode==NORMAL_PANEL)
		Diz.DeleteDiz(Name,ShortName);
}


void FileList::FlushDiz()
{
	if (PanelMode==NORMAL_PANEL)
		Diz.Flush(strCurDir);
}


void FileList::GetDizName(string &strDizName) const
{
	if (PanelMode==NORMAL_PANEL)
		Diz.GetDizName(strDizName);
}


void FileList::CopyDiz(const string& Name, const string& ShortName,const string& DestName,
                       const string& DestShortName,DizList *DestDiz)
{
	Diz.CopyDiz(Name, ShortName, DestName, DestShortName, DestDiz);
}


void FileList::DescribeFiles()
{
	string strSelName, strSelShortName;
	DWORD FileAttr;
	int DizCount=0;
	ReadDiz();
	SaveSelection();
	GetSelName(nullptr,FileAttr);
	Panel* AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
	int AnotherType=AnotherPanel->GetType();

	while (GetSelName(&strSelName,FileAttr,&strSelShortName))
	{
		string strDizText, strMsg, strQuotedName;
		const wchar_t *PrevText;
		PrevText=Diz.GetDizTextAddr(strSelName,strSelShortName,GetLastSelectedSize());
		strQuotedName = strSelName;
		QuoteSpaceOnly(strQuotedName);
		strMsg.append(MSG(MEnterDescription)).append(L" ").append(strQuotedName).append(L":");

		/* $ 09.08.2000 SVS
		   Для Ctrl-Z ненужно брать предыдущее значение!
		*/
		if (!GetString(MSG(MDescribeFiles),strMsg.data(),L"DizText",
		               PrevText ? PrevText:L"",strDizText,
		               L"FileDiz",FIB_ENABLEEMPTY|(!DizCount?FIB_NOUSELASTHISTORY:0)|FIB_BUTTONS))
			break;

		DizCount++;

		if (strDizText.empty())
		{
			Diz.DeleteDiz(strSelName,strSelShortName);
		}
		else
		{
			Diz.AddDizText(strSelName,strSelShortName,strDizText);
		}

		ClearLastGetSelection();
		// BugZ#442 - Deselection is late when making file descriptions
		FlushDiz();

		// BugZ#863 - При редактировании группы дескрипшенов они не обновляются на ходу
		//if (AnotherType==QVIEW_PANEL) continue; //TODO ???
		if (AnotherType==INFO_PANEL) AnotherPanel->Update(UIC_UPDATE_NORMAL);

		Update(UPDATE_KEEP_SELECTION);
		Redraw();
	}

	/*if (DizCount>0)
	{
	  FlushDiz();
	  Update(UPDATE_KEEP_SELECTION);
	  Redraw();
	  Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
	  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	  AnotherPanel->Redraw();
	}*/
}


void FileList::SetReturnCurrentFile(int Mode)
{
	ReturnCurrentFile=Mode;
}


bool FileList::ApplyCommand()
{
	static string strPrevCommand;
	string strCommand;

	if (!GetString(MSG(MAskApplyCommandTitle),MSG(MAskApplyCommand),L"ApplyCmd",strPrevCommand.data(),strCommand,L"ApplyCmd",FIB_BUTTONS|FIB_EDITPATH|FIB_EDITPATHEXEC) || !SetCurPath())
		return false;

	strPrevCommand = strCommand;
	RemoveLeadingSpaces(strCommand);

	string strSelName, strSelShortName;
	DWORD FileAttr;

	SaveSelection();

	++UpdateDisabled;
	GetSelName(nullptr,FileAttr);
	Global->CtrlObject->CmdLine->LockUpdatePanel(true);
	while (GetSelName(&strSelName,FileAttr,&strSelShortName) && !CheckForEsc())
	{
		string strListName, strAnotherListName;
		string strShortListName, strAnotherShortListName;
		string strConvertedCommand = strCommand;
		int PreserveLFN=SubstFileName(nullptr,strConvertedCommand,strSelName, strSelShortName, &strListName, &strAnotherListName, &strShortListName, &strAnotherShortListName);
		bool ListFileUsed=!strListName.empty()||!strAnotherListName.empty()||!strShortListName.empty()||!strAnotherShortListName.empty();

		if (ExtractIfExistCommand(strConvertedCommand))
		{
			PreserveLongName PreserveName(strSelShortName,PreserveLFN);
			RemoveExternalSpaces(strConvertedCommand);

			if (!strConvertedCommand.empty())
			{
				Global->CtrlObject->CmdLine->ExecString(strConvertedCommand, false, 0, 0, ListFileUsed, false, true); // Param2 == TRUE?
					//if (!(Global->Opt->ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTAPPLYCMD))
					//	Global->CtrlObject->CmdHistory->AddToHistory(strConvertedCommand);
			}

			ClearLastGetSelection();
		}

		if (!strListName.empty())
			api::DeleteFile(strListName);

		if (!strAnotherListName.empty())
			api::DeleteFile(strAnotherListName);

		if (!strShortListName.empty())
			api::DeleteFile(strShortListName);

		if (!strAnotherShortListName.empty())
			api::DeleteFile(strAnotherShortListName);
	}

	Global->CtrlObject->CmdLine->LockUpdatePanel(false);
	Global->CtrlObject->CmdLine->Show();
	if (Global->Opt->ShowKeyBar)
	{
		Global->CtrlObject->MainKeyBar->Show();
	}
	if (GetSelPosition >= static_cast<int>(ListData.size()))
		ClearSelection();

	--UpdateDisabled;
	return true;
}


void FileList::CountDirSize(UINT64 PluginFlags)
{
	unsigned long SelDirCount=0;
	DirInfoData Data = {};
	/* $ 09.11.2000 OT
	  F3 на ".." в плагинах
	*/
	if (PanelMode==PLUGIN_PANEL && !CurFile && TestParentFolderName(ListData[0].strName))
	{
		FileListItem *DoubleDotDir = nullptr;

		if (SelFileCount)
		{
			DoubleDotDir = &ListData.front();

			if (std::any_of(CONST_RANGE(ListData, i) {return i.Selected && i.FileAttr & FILE_ATTRIBUTE_DIRECTORY;}))
				DoubleDotDir = nullptr;
		}
		else
		{
			DoubleDotDir = &ListData.front();
		}

		if (DoubleDotDir)
		{
			DoubleDotDir->ShowFolderSize=1;
			DoubleDotDir->FileSize     = 0;
			DoubleDotDir->AllocationSize    = 0;

			FOR(const auto& i, make_range(ListData.begin() + 1, ListData.end()))
			{
				if (i.FileAttr & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (GetPluginDirInfo(hPlugin, i.strName, Data.DirCount, Data.FileCount, Data.FileSize, Data.AllocationSize))
					{
						DoubleDotDir->FileSize += Data.FileSize;
						DoubleDotDir->AllocationSize += Data.AllocationSize;
					}
				}
				else
				{
					DoubleDotDir->FileSize += i.FileSize;
					DoubleDotDir->AllocationSize += i.AllocationSize;
				}
			}
		}
	}

	//Рефреш текущему времени для фильтра перед началом операции
	Filter->UpdateCurrentTime();

	FOR(auto& i, ListData)
	{
		if (i.Selected && (i.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			SelDirCount++;
			if ((PanelMode==PLUGIN_PANEL && !(PluginFlags & OPIF_REALNAMES) &&
			        GetPluginDirInfo(hPlugin, i.strName, Data.DirCount, Data.FileCount, Data.FileSize, Data.AllocationSize))
			        ||
			        ((PanelMode!=PLUGIN_PANEL || (PluginFlags & OPIF_REALNAMES)) &&
			         GetDirInfo(MSG(MDirInfoViewTitle), i.strName, Data, 0, Filter.get(), GETDIRINFO_DONTREDRAWFRAME|GETDIRINFO_SCANSYMLINKDEF)==1))
			{
				SelFileSize -= i.FileSize;
				SelFileSize += Data.FileSize;
				i.FileSize = Data.FileSize;
				i.AllocationSize = Data.AllocationSize;
				i.ShowFolderSize=1;
			}
			else
				break;
		}
	}

	if (!SelDirCount)
	{
		assert(CurFile < static_cast<int>(ListData.size()));
		if ((PanelMode==PLUGIN_PANEL && !(PluginFlags & OPIF_REALNAMES) &&
		        GetPluginDirInfo(hPlugin,ListData[CurFile].strName, Data.DirCount, Data.FileCount, Data.FileSize, Data.AllocationSize))
		        ||
		        ((PanelMode!=PLUGIN_PANEL || (PluginFlags & OPIF_REALNAMES)) &&
		         GetDirInfo(MSG(MDirInfoViewTitle),
		                    TestParentFolderName(ListData[CurFile].strName) ? L".":ListData[CurFile].strName,
		                    Data, 0, Filter.get(), GETDIRINFO_DONTREDRAWFRAME|GETDIRINFO_SCANSYMLINKDEF)==1))
		{
			ListData[CurFile].FileSize = Data.FileSize;
			ListData[CurFile].AllocationSize = Data.AllocationSize;
			ListData[CurFile].ShowFolderSize=1;
		}
	}

	SortFileList(TRUE);
	ShowFileList(TRUE);
	Global->CtrlObject->Cp()->Redraw();
	InitFSWatcher(true);
}


int FileList::GetPrevViewMode() const
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.empty())?PluginsList.front().m_PrevViewMode:ViewMode;
}


int FileList::GetPrevSortMode() const
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.empty())?PluginsList.front().m_PrevSortMode:SortMode;
}


bool FileList::GetPrevSortOrder() const
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.empty())?PluginsList.front().m_PrevSortOrder : ReverseSortOrder;
}

bool FileList::GetPrevNumericSort() const
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.empty())?PluginsList.front().m_PrevNumericSort:NumericSort;
}

bool FileList::GetPrevCaseSensitiveSort() const
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.empty())?PluginsList.front().m_PrevCaseSensitiveSort:CaseSensitiveSort;
}

bool FileList::GetPrevDirectoriesFirst() const
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.empty())?PluginsList.front().m_PrevDirectoriesFirst:DirectoriesFirst;
}

PluginHandle* FileList::OpenFilePlugin(const string* FileName, int PushPrev, OPENFILEPLUGINTYPE Type)
{
	if (!PushPrev && PanelMode==PLUGIN_PANEL)
	{
		for (;;)
		{
			if (ProcessPluginEvent(FE_CLOSE,nullptr))
				return static_cast<PluginHandle*>(PANEL_STOP);

			if (!PopPlugin(TRUE))
				break;
		}
	}

	auto hNewPlugin=OpenPluginForFile(FileName, 0, Type);

	if (hNewPlugin && hNewPlugin!=PANEL_STOP)
	{
		if (PushPrev)
		{
			PrevDataList.emplace_back(VALUE_TYPE(PrevDataList)(FileName? *FileName : L"", std::move(ListData), CurTopFile));
		}

		bool WasFullscreen = IsFullScreen();
		SetPluginMode(hNewPlugin, FileName ? *FileName : L"");  // SendOnFocus??? true???
		PanelMode=PLUGIN_PANEL;
		UpperFolderTopFile=CurTopFile;
		CurFile=0;
		Update(0);
		Redraw();
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

		if ((AnotherPanel->GetType()==INFO_PANEL) || WasFullscreen)
			AnotherPanel->Redraw();
	}

	return hNewPlugin;
}


void FileList::ProcessCopyKeys(int Key)
{
	if (!ListData.empty())
	{
		int Drag=Key==KEY_DRAGCOPY || Key==KEY_DRAGMOVE;
		int Ask=!Drag || Global->Opt->Confirm.Drag;
		int Move=(Key==KEY_F6 || Key==KEY_DRAGMOVE);
		int AnotherDir=FALSE;
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

		if (AnotherPanel->GetType()==FILE_PANEL)
		{
			FileList *AnotherFilePanel=(FileList *)AnotherPanel;

			assert(AnotherFilePanel->ListData.empty() || AnotherFilePanel->CurFile < static_cast<int>(AnotherFilePanel->ListData.size()));
			if (!AnotherFilePanel->ListData.empty() &&
			        (AnotherFilePanel->ListData[AnotherFilePanel->CurFile].FileAttr & FILE_ATTRIBUTE_DIRECTORY) &&
			        !TestParentFolderName(AnotherFilePanel->ListData[AnotherFilePanel->CurFile].strName))
			{
				AnotherDir=TRUE;
			}
		}

		if (PanelMode==PLUGIN_PANEL && !Global->CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARGETFILES))
		{
			if (Key!=KEY_ALTF6 && Key!=KEY_RALTF6)
			{
				string strPluginDestPath;
				int ToPlugin=FALSE;

				if (AnotherPanel->GetMode()==PLUGIN_PANEL && AnotherPanel->IsVisible() &&
				        !Global->CtrlObject->Plugins->UseFarCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES))
				{
					ToPlugin=2;
					ShellCopy ShCopy(this,Move,FALSE,FALSE,Ask,ToPlugin,strPluginDestPath.data());
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
								OpenPanelInfo Info;
								Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

								if (Info.HostFile && *Info.HostFile)
								{
									strDestPath = PointToName(Info.HostFile);
									size_t pos = strDestPath.rfind(L'.');
									if (pos != string::npos)
										strDestPath.resize(pos);
								}
							}
						}

						const wchar_t *lpwszDestPath=strDestPath.data();

						PluginGetFiles(&lpwszDestPath,Move);
						// BUGBUG, never used
						strDestPath=lpwszDestPath;
					}
				}
			}
		}
		else
		{
			int ToPlugin=AnotherPanel->GetMode()==PLUGIN_PANEL &&
			             AnotherPanel->IsVisible() && (Key!=KEY_ALTF6 && Key!=KEY_RALTF6) &&
			             !Global->CtrlObject->Plugins->UseFarCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES);
			ShellCopy ShCopy(this,Move,(Key==KEY_ALTF6 || Key==KEY_RALTF6),FALSE,Ask,ToPlugin,nullptr, Drag && AnotherDir);

			if (ToPlugin==1)
				PluginPutFilesToAnother(Move,AnotherPanel);
		}
	}
}

void FileList::SetSelectedFirstMode(bool Mode)
{
	SelectedFirst=Mode;
	SortFileList(TRUE);
	ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
}

void FileList::ChangeSortOrder(bool Reverse)
{
	Panel::ChangeSortOrder(Reverse);
	SortFileList(TRUE);
	ProcessPluginEvent(FE_CHANGESORTPARAMS, nullptr);
	Show();
}

void FileList::UpdateKeyBar()
{
	KeyBar *KB=Global->CtrlObject->MainKeyBar;
	KB->SetLabels(MF1);
	KB->SetCustomLabels(KBA_SHELL);

	if (GetMode() == PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		GetOpenPanelInfo(&Info);

		if (Info.KeyBar)
			KB->Change(Info.KeyBar);
	}

}

int FileList::PluginPanelHelp(const PluginHandle* hPlugin) const
{
	string strPath, strFileName, strStartTopic;
	strPath = hPlugin->pPlugin->GetModuleName();
	CutToSlash(strPath);
	uintptr_t nCodePage = CP_OEMCP;
	api::File HelpFile;
	if (!OpenLangFile(HelpFile, strPath,Global->HelpFileMask,Global->Opt->strHelpLanguage,strFileName, nCodePage))
		return FALSE;

	strStartTopic = Help::MakeLink(strPath, L"Contents");
	Help PanelHelp(strStartTopic);
	return TRUE;
}

/* $ 19.11.2001 IS
     для файловых панелей с реальными файлами никакого префикса не добавляем
*/
string &FileList::AddPluginPrefix(const FileList *SrcPanel, string &strPrefix)
{
	strPrefix.clear();

	if (Global->Opt->SubstPluginPrefix && SrcPanel->GetMode()==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		auto ph = SrcPanel->hPlugin;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(ph,&Info);

		if (!(Info.Flags & OPIF_REALNAMES))
		{
			PluginInfo PInfo = {sizeof(PInfo)};
			ph->pPlugin->GetPluginInfo(&PInfo);

			if (PInfo.CommandPrefix && *PInfo.CommandPrefix)
			{
				strPrefix = PInfo.CommandPrefix;
				size_t pos = strPrefix.find(L':');
				if (pos != string::npos)
					strPrefix.resize(pos+1);
				else
					strPrefix += L":";
			}
		}
	}

	return strPrefix;
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
		Panel *Another=Global->CtrlObject->Cp()->GetAnotherPanel(this);

		if (Another->GetMode() != PLUGIN_PANEL)
		{
			strTmpCurDir = Another->GetCurDir();

			if (strTmpCurDir[0] == Drive && strTmpCurDir[1] == L':')
				Another->SetCurDir(strFName, false);
		}

		if (GetMode() != PLUGIN_PANEL)
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

	if (Index >= ListData.size())
		return nullptr;

	return &ListData[Index];
}

void FileList::ClearAllItem()
{
	std::for_each(RANGE(PrevDataList, i)
	{
		DeleteListData(i.PrevListData);
	});
	PrevDataList.clear();
}

// flplugin
// Файловая панель - работа с плагинами

/*
   В стеке ФАРова панель не хранится - только плагиновые!
*/

void FileList::PushPlugin(PluginHandle* hPlugin,const string& HostFile)
{
	PluginsList.emplace_back(VALUE_TYPE(PluginsList)(hPlugin, HostFile, strOriginalCurDir, FALSE, ViewMode, SortMode, ReverseSortOrder, NumericSort, CaseSensitiveSort, DirectoriesFirst, ViewSettings));
	++Global->PluginPanelsCount;
}

int FileList::PopPlugin(int EnableRestoreViewMode)
{
	DeleteListData(ListData);

	OpenPanelInfo Info={};

	if (PluginsList.empty())
	{
		PanelMode=NORMAL_PANEL;
		return FALSE;
	}

	const PluginsListItem CurPlugin = std::move(PluginsList.back());

	PluginsList.pop_back();
	--Global->PluginPanelsCount;

	Global->CtrlObject->Plugins->ClosePanel(hPlugin);

	if (!PluginsList.empty())
	{
		hPlugin = PluginsList.back().m_Plugin;
		strOriginalCurDir=CurPlugin.m_PrevOriginalCurDir;

		if (EnableRestoreViewMode)
		{
			SetViewMode(CurPlugin.m_PrevViewMode);
			SortMode = CurPlugin.m_PrevSortMode;
			NumericSort = CurPlugin.m_PrevNumericSort;
			CaseSensitiveSort = CurPlugin.m_PrevCaseSensitiveSort;
			ReverseSortOrder = CurPlugin.m_PrevSortOrder;
			DirectoriesFirst = CurPlugin.m_PrevDirectoriesFirst;
		}

		if (CurPlugin.m_Modified)
		{
			PluginPanelItem PanelItem={};
			string strSaveDir;
			api::GetCurrentDirectory(strSaveDir);

			if (FileNameToPluginItem(CurPlugin.m_HostFile,&PanelItem))
			{
				Global->CtrlObject->Plugins->PutFiles(hPlugin, &PanelItem, 1, false, 0);
			}
			else
			{
				PanelItem.FileName = DuplicateString(PointToName(CurPlugin.m_HostFile));
				Global->CtrlObject->Plugins->DeleteFiles(hPlugin,&PanelItem,1,0);
				delete[] PanelItem.FileName;
			}

			FarChDir(strSaveDir);
		}


		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

		if (!(Info.Flags & OPIF_REALNAMES))
		{
			DeleteFileWithFolder(CurPlugin.m_HostFile);  // удаление файла от предыдущего плагина
		}
	}
	else
	{
		PanelMode=NORMAL_PANEL;
		hPlugin = nullptr;

		if (EnableRestoreViewMode)
		{
			SetViewMode(CurPlugin.m_PrevViewMode);
			SortMode = CurPlugin.m_PrevSortMode;
			NumericSort = CurPlugin.m_PrevNumericSort;
			CaseSensitiveSort = CurPlugin.m_PrevCaseSensitiveSort;
			ReverseSortOrder = CurPlugin.m_PrevSortOrder;
			DirectoriesFirst = CurPlugin.m_PrevDirectoriesFirst;
		}
	}

	if (EnableRestoreViewMode)
		Global->CtrlObject->Cp()->RedrawKeyBar();

	return TRUE;
}

/*
	DefaultName - имя элемента на которое позиционируемся.
	Closed - панель закрывается, если в PrevDataList что-то есть - восстанавливаемчся оттуда.
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
			MoveSelection(Item.PrevListData, ListData);
			UpperFolderTopFile = Item.PrevTopFile;

			if (UsePrev)
				strName = Item.strPrevName;

			DeleteListData(Item.PrevListData);

			if (SelectedFirst)
				SortFileList(FALSE);
			else if (!ListData.empty())
				SortFileList(TRUE);
		}
		PrevDataList.pop_back();
	}
	if (Position)
	{
		long Pos=FindFile(PointToName(strName));

		if (Pos!=-1)
			CurFile=Pos;
		else
			GoToFile(strName);

		CurTopFile=UpperFolderTopFile;
		UpperFolderTopFile=0;
		CorrectPosition();
	}
	/* $ 26.04.2001 DJ
	   доделка про несброс выделения при неудаче SetDirectory
	*/
	else if (SetDirectorySuccess)
		CurFile=CurTopFile=0;
}

int FileList::FileNameToPluginItem(const string& Name,PluginPanelItem *pi)
{
	string strTempDir = Name;

	if (!CutToSlash(strTempDir,true))
		return FALSE;

	FarChDir(strTempDir);
	ClearStruct(*pi);
	api::FAR_FIND_DATA fdata;

	if (api::GetFindDataEx(Name, fdata))
	{
		FindDataExToPluginPanelItem(&fdata, pi);
		return TRUE;
	}

	return FALSE;
}


void FileList::FileListToPluginItem(const FileListItem& fi, PluginPanelItem *pi)
{
	pi->FileName = DuplicateString(fi.strName.data());
	pi->AlternateFileName = DuplicateString(fi.strShortName.data());
	pi->FileSize=fi.FileSize;
	pi->AllocationSize=fi.AllocationSize;
	pi->FileAttributes=fi.FileAttr;
	pi->LastWriteTime=fi.WriteTime;
	pi->CreationTime=fi.CreationTime;
	pi->LastAccessTime=fi.AccessTime;
	pi->ChangeTime=fi.ChangeTime;
	pi->NumberOfLinks=fi.NumberOfLinks;
	pi->Flags=fi.UserFlags;

	if (fi.Selected)
		pi->Flags|=PPIF_SELECTED;

	pi->CustomColumnData=fi.CustomColumnData;
	pi->CustomColumnNumber=fi.CustomColumnNumber;
	pi->Description=fi.DizText; //BUGBUG???

	pi->UserData.Data=fi.UserData;
	pi->UserData.FreeData=fi.Callback;

	pi->CRC32=fi.CRC32;
	pi->Reserved[0]=pi->Reserved[1]=0;
	pi->Owner = EmptyToNull(fi.strOwner.data());
}

size_t FileList::FileListToPluginItem2(FileListItem *fi,FarGetPluginPanelItem *gpi)
{
	size_t size=ALIGN(sizeof(PluginPanelItem)),offset=size;
	size+=fi->CustomColumnNumber*sizeof(wchar_t*);
	size+=sizeof(wchar_t)*(fi->strName.size()+1);
	size+=sizeof(wchar_t)*(fi->strShortName.size()+1);
	for (size_t ii=0; ii<fi->CustomColumnNumber; ii++)
	{
		size+=fi->CustomColumnData[ii]?sizeof(wchar_t)*(wcslen(fi->CustomColumnData[ii])+1):0;
	}
	size+=fi->DizText?sizeof(wchar_t)*(wcslen(fi->DizText)+1):0;
	size+=fi->strOwner.empty()?0:sizeof(wchar_t)*(fi->strOwner.size()+1);

	if (gpi)
	{
		if(gpi->Item && gpi->Size >= size)
		{
			char* data=(char*)(gpi->Item)+offset;

			gpi->Item->FileSize=fi->FileSize;
			gpi->Item->AllocationSize=fi->AllocationSize;
			gpi->Item->FileAttributes=fi->FileAttr;
			gpi->Item->LastWriteTime=fi->WriteTime;
			gpi->Item->CreationTime=fi->CreationTime;
			gpi->Item->LastAccessTime=fi->AccessTime;
			gpi->Item->ChangeTime=fi->ChangeTime;
			gpi->Item->NumberOfLinks=fi->NumberOfLinks;
			gpi->Item->Flags=fi->UserFlags;
			if (fi->Selected)
				gpi->Item->Flags|=PPIF_SELECTED;
			gpi->Item->CustomColumnNumber=fi->CustomColumnNumber;
			gpi->Item->CRC32=fi->CRC32;
			gpi->Item->Reserved[0]=gpi->Item->Reserved[1]=0;

			gpi->Item->CustomColumnData=(wchar_t**)data;
			data+=fi->CustomColumnNumber*sizeof(wchar_t*);

			gpi->Item->UserData.Data=fi->UserData;
			gpi->Item->UserData.FreeData=fi->Callback;

			gpi->Item->FileName=wcscpy((wchar_t*)data,fi->strName.data());
			data+=sizeof(wchar_t)*(fi->strName.size()+1);

			gpi->Item->AlternateFileName=wcscpy((wchar_t*)data,fi->strShortName.data());
			data+=sizeof(wchar_t)*(fi->strShortName.size()+1);

			for (size_t ii=0; ii<fi->CustomColumnNumber; ii++)
			{
				if (!fi->CustomColumnData[ii])
				{
					const_cast<const wchar_t**>(gpi->Item->CustomColumnData)[ii] = nullptr;
				}
				else
				{
					const_cast<const wchar_t**>(gpi->Item->CustomColumnData)[ii] = wcscpy(reinterpret_cast<wchar_t*>(data), fi->CustomColumnData[ii]);
					data+=sizeof(wchar_t)*(wcslen(fi->CustomColumnData[ii])+1);
				}
			}

			if (!fi->DizText)
			{
				gpi->Item->Description=nullptr;
			}
			else
			{
				gpi->Item->Description=wcscpy((wchar_t*)data,fi->DizText);
				data+=sizeof(wchar_t)*(wcslen(fi->DizText)+1);
			}


			if (fi->strOwner.empty())
			{
				gpi->Item->Owner=nullptr;
			}
			else
			{
				gpi->Item->Owner=wcscpy((wchar_t*)data,fi->strOwner.data());
			}
		}
	}
	return size;
}

void FileList::PluginToFileListItem(PluginPanelItem *pi,FileListItem *fi)
{
	fi->strName = NullToEmpty(pi->FileName);
	fi->strShortName = NullToEmpty(pi->AlternateFileName);
	fi->strOwner = NullToEmpty(pi->Owner);

	if (pi->Description)
	{
		auto Str = new wchar_t[StrLength(pi->Description)+1];
		wcscpy(Str, pi->Description);
		fi->DizText = Str;
		fi->DeleteDiz=true;
	}
	else
		fi->DizText=nullptr;

	fi->FileSize=pi->FileSize;
	fi->AllocationSize=pi->AllocationSize;
	fi->FileAttr=pi->FileAttributes;
	fi->WriteTime=pi->LastWriteTime;
	fi->CreationTime=pi->CreationTime;
	fi->AccessTime=pi->LastAccessTime;
	fi->ChangeTime = pi->ChangeTime;
	fi->NumberOfLinks=pi->NumberOfLinks;
	fi->NumberOfStreams=1;
	fi->UserFlags=pi->Flags;

	fi->UserData=pi->UserData.Data;
	fi->Callback=pi->UserData.FreeData;

	if (pi->CustomColumnNumber>0)
	{
		fi->CustomColumnData=new wchar_t*[pi->CustomColumnNumber];

		for (size_t I=0; I<pi->CustomColumnNumber; I++)
			if (pi->CustomColumnData && pi->CustomColumnData[I])
			{
				fi->CustomColumnData[I]=new wchar_t[StrLength(pi->CustomColumnData[I])+1];
				wcscpy(fi->CustomColumnData[I],pi->CustomColumnData[I]);
			}
			else
			{
				fi->CustomColumnData[I]=new wchar_t[1];
				fi->CustomColumnData[I][0]=0;
			}
	}

	fi->CustomColumnNumber=pi->CustomColumnNumber;
	fi->CRC32=pi->CRC32;

	if (fi->FileAttr & FILE_ATTRIBUTE_REPARSE_POINT)
	{
		// we don't really know, but it's better than show it as 'unknown'
		fi->ReparseTag = IO_REPARSE_TAG_SYMLINK;
	}
}


PluginHandle* FileList::OpenPluginForFile(const string* FileName, DWORD FileAttr, OPENFILEPLUGINTYPE Type)
{
	PluginHandle* Result = nullptr;
	if(!FileName->empty() && !(FileAttr&FILE_ATTRIBUTE_DIRECTORY))
	{
		SetCurPath();
		_ALGO(SysLog(L"close AnotherPanel file"));
		Global->CtrlObject->Cp()->GetAnotherPanel(this)->CloseFile();
		_ALGO(SysLog(L"call Plugins.OpenFilePlugin {"));
		Result = Global->CtrlObject->Plugins->OpenFilePlugin(FileName, 0, Type);
		_ALGO(SysLog(L"}"));
	}
	return Result;
}


std::vector<PluginPanelItem> FileList::CreatePluginItemList(bool AddTwoDot)
{
	std::vector<PluginPanelItem> ItemList;

	if (ListData.empty())
		return ItemList;

	long SaveSelPosition=GetSelPosition;
	long OldLastSelPosition=LastSelPosition;
	string strSelName;

	ItemList.reserve(SelFileCount+1);

	DWORD FileAttr;
	GetSelName(nullptr,FileAttr);

	while (GetSelName(&strSelName,FileAttr))
	{
		if ((!(FileAttr & FILE_ATTRIBUTE_DIRECTORY) || !TestParentFolderName(strSelName)) && LastSelPosition>=0 && static_cast<size_t>(LastSelPosition) < ListData.size())
		{
			ItemList.emplace_back(VALUE_TYPE(ItemList)());
			FileListToPluginItem(ListData[LastSelPosition], &ItemList.back());
		}
	}

	if (AddTwoDot && ItemList.empty() && (FileAttr & FILE_ATTRIBUTE_DIRECTORY)) // это про ".."
	{
		FileListToPluginItem(ListData[0], &ItemList.front());
		//ItemList->FindData.lpwszFileName = DuplicateString(ListData[0]->strName);
		//ItemList->FindData.dwFileAttributes=ListData[0]->FileAttr;
	}

	LastSelPosition=OldLastSelPosition;
	GetSelPosition=SaveSelPosition;
	return ItemList;
}


void FileList::DeletePluginItemList(std::vector<PluginPanelItem> &ItemList)
{
	std::for_each(ALL_RANGE(ItemList), FreePluginPanelItem);
	ItemList.clear();
}


void FileList::PluginDelete()
{
	_ALGO(CleverSysLog clv(L"FileList::PluginDelete()"));
	SaveSelection();
	auto ItemList = CreatePluginItemList();

	if (!ItemList.empty())
	{
		if (Global->CtrlObject->Plugins->DeleteFiles(hPlugin, ItemList.data(), ItemList.size(), 0))
		{
			SetPluginModified();
			PutDizToPlugin(this, ItemList, TRUE, FALSE, nullptr);
		}

		DeletePluginItemList(ItemList);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		AnotherPanel->Redraw();
	}
}


void FileList::PutDizToPlugin(FileList *DestPanel, const std::vector<PluginPanelItem>& ItemList, int Delete, int Move, DizList *SrcDiz)
{
	_ALGO(CleverSysLog clv(L"FileList::PutDizToPlugin()"));
	OpenPanelInfo Info;
	Global->CtrlObject->Plugins->GetOpenPanelInfo(DestPanel->hPlugin,&Info);

	if (DestPanel->strPluginDizName.empty() && Info.DescrFilesNumber>0)
		DestPanel->strPluginDizName = Info.DescrFiles[0];

	if (((Global->Opt->Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed()) ||
	        Global->Opt->Diz.UpdateMode==DIZ_UPDATE_ALWAYS) && !DestPanel->strPluginDizName.empty() &&
	        (!Info.HostFile || !*Info.HostFile || DestPanel->GetModalMode() ||
	         api::GetFileAttributes(Info.HostFile)!=INVALID_FILE_ATTRIBUTES))
	{
		Global->CtrlObject->Cp()->LeftPanel->ReadDiz();
		Global->CtrlObject->Cp()->RightPanel->ReadDiz();

		if (DestPanel->GetModalMode())
			DestPanel->ReadDiz();

		bool DizPresent = false;

		std::for_each(CONST_RANGE(ItemList, i)
		{
			if (i.Flags & PPIF_PROCESSDESCR)
			{
				int Code;

				if (Delete)
					Code = DestPanel->Diz.DeleteDiz(i.FileName, i.AlternateFileName);
				else
				{
					Code = SrcDiz->CopyDiz(i.FileName, i.AlternateFileName, i.FileName, i.AlternateFileName, &DestPanel->Diz);

					if (Code && Move)
						SrcDiz->DeleteDiz(i.FileName, i.AlternateFileName);
				}

				if (Code)
					DizPresent = true;
			}
		});

		if (DizPresent)
		{
			string strTempDir;

			if (FarMkTempEx(strTempDir) && api::CreateDirectory(strTempDir,nullptr))
			{
				string strSaveDir;
				api::GetCurrentDirectory(strSaveDir);
				string strDizName=strTempDir+L"\\"+DestPanel->strPluginDizName;
				DestPanel->Diz.Flush(L"", &strDizName);

				if (Move)
					SrcDiz->Flush(L"");

				PluginPanelItem PanelItem;

				if (FileNameToPluginItem(strDizName,&PanelItem))
					Global->CtrlObject->Plugins->PutFiles(DestPanel->hPlugin, &PanelItem, 1, false, OPM_SILENT | OPM_DESCR);
				else if (Delete)
				{
					PluginPanelItem pi={};
					pi.FileName = DestPanel->strPluginDizName.data();
					Global->CtrlObject->Plugins->DeleteFiles(DestPanel->hPlugin,&pi,1,OPM_SILENT);
				}

				FarChDir(strSaveDir);
				DeleteFileWithFolder(strDizName);
			}
		}
	}
}


void FileList::PluginGetFiles(const wchar_t **DestPath,int Move)
{
	_ALGO(CleverSysLog clv(L"FileList::PluginGetFiles()"));
	SaveSelection();
	auto ItemList = CreatePluginItemList();
	if (!ItemList.empty())
	{
		int GetCode=Global->CtrlObject->Plugins->GetFiles(hPlugin, ItemList.data(), ItemList.size(), Move!=0, DestPath, 0);

		if ((Global->Opt->Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && IsDizDisplayed()) ||
		        Global->Opt->Diz.UpdateMode==DIZ_UPDATE_ALWAYS)
		{
			DizList DestDiz;
			int DizFound=FALSE;

			std::for_each(RANGE(ItemList, i)
			{
				if (i.Flags & PPIF_PROCESSDESCR)
				{
					if (!DizFound)
					{
						Global->CtrlObject->Cp()->LeftPanel->ReadDiz();
						Global->CtrlObject->Cp()->RightPanel->ReadDiz();
						DestDiz.Read(*DestPath);
						DizFound=TRUE;
					}
					CopyDiz(i.FileName, i.AlternateFileName, i.FileName, i.FileName, &DestDiz);
				}
			});
			DestDiz.Flush(*DestPath);
		}

		if (GetCode==1)
		{
			if (!ReturnCurrentFile)
				ClearSelection();

			if (Move)
			{
				SetPluginModified();
				PutDizToPlugin(this, ItemList, TRUE, FALSE, nullptr);
			}
		}
		else if (!ReturnCurrentFile)
			PluginClearSelection(ItemList);

		DeletePluginItemList(ItemList);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		AnotherPanel->Redraw();
	}
}


void FileList::PluginToPluginFiles(int Move)
{
	_ALGO(CleverSysLog clv(L"FileList::PluginToPluginFiles()"));
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
	string strTempDir;

	if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
		return;

	FileList *AnotherFilePanel=(FileList *)AnotherPanel;

	if (!FarMkTempEx(strTempDir))
		return;

	SaveSelection();
	api::CreateDirectory(strTempDir,nullptr);
	auto ItemList = CreatePluginItemList();

	if (!ItemList.empty())
	{
		const wchar_t *lpwszTempDir=strTempDir.data();
		int PutCode = Global->CtrlObject->Plugins->GetFiles(hPlugin, ItemList.data(), ItemList.size(), false, &lpwszTempDir, OPM_SILENT);
		strTempDir=lpwszTempDir;

		if (PutCode==1 || PutCode==2)
		{
			string strSaveDir;
			api::GetCurrentDirectory(strSaveDir);
			FarChDir(strTempDir);
			PutCode = Global->CtrlObject->Plugins->PutFiles(AnotherFilePanel->hPlugin, ItemList.data(), ItemList.size(), false, 0);

			if (PutCode==1 || PutCode==2)
			{
				if (!ReturnCurrentFile)
					ClearSelection();

				AnotherPanel->SetPluginModified();
				PutDizToPlugin(AnotherFilePanel, ItemList, FALSE, FALSE, &Diz);

				if (Move && Global->CtrlObject->Plugins->DeleteFiles(hPlugin, ItemList.data(), ItemList.size(), OPM_SILENT))
				{
					SetPluginModified();
					PutDizToPlugin(this, ItemList, TRUE, FALSE, nullptr);
				}
			}
			else if (!ReturnCurrentFile)
				PluginClearSelection(ItemList);

			FarChDir(strSaveDir);
		}

		DeleteDirTree(strTempDir);
		DeletePluginItemList(ItemList);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();

		if (PanelMode==PLUGIN_PANEL)
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		else
			AnotherPanel->Update(UPDATE_KEEP_SELECTION);

		AnotherPanel->Redraw();
	}
}

void FileList::PluginHostGetFiles()
{
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
	string strDestPath;
	string strSelName;
	DWORD FileAttr;
	SaveSelection();
	GetSelName(nullptr,FileAttr);

	if (!GetSelName(&strSelName,FileAttr))
		return;

	strDestPath = AnotherPanel->GetCurDir();

	if (((!AnotherPanel->IsVisible() || AnotherPanel->GetType()!=FILE_PANEL) &&
	        !SelFileCount) || strDestPath.empty())
	{
		strDestPath = PointToName(strSelName);
		// SVS: А зачем здесь велся поиск точки с начала?
		size_t pos = strDestPath.rfind(L'.');
		if (pos != string::npos)
			strDestPath.resize(pos);
	}

	int ExitLoop=FALSE;
	GetSelName(nullptr,FileAttr);
	std::unordered_set<Plugin*> UsedPlugins;

	while (!ExitLoop && GetSelName(&strSelName,FileAttr))
	{
		PluginHandle* hCurPlugin;

		if ((hCurPlugin=OpenPluginForFile(&strSelName,FileAttr, OFP_EXTRACT))!=nullptr &&
		        hCurPlugin!=PANEL_STOP)
		{
			int OpMode=OPM_TOPLEVEL;
			if(UsedPlugins.find(hCurPlugin->pPlugin) != UsedPlugins.cend())
				OpMode|=OPM_SILENT;

			PluginPanelItem *ItemList;
			size_t ItemNumber;
			_ALGO(SysLog(L"call Plugins.GetFindData()"));

			if (Global->CtrlObject->Plugins->GetFindData(hCurPlugin,&ItemList,&ItemNumber,OpMode))
			{
				_ALGO(SysLog(L"call Plugins.GetFiles()"));
				const wchar_t *lpwszDestPath=strDestPath.data();
				ExitLoop = Global->CtrlObject->Plugins->GetFiles(hCurPlugin, ItemList, ItemNumber, false, &lpwszDestPath, OpMode) != 1;
				strDestPath=lpwszDestPath;

				if (!ExitLoop)
				{
					_ALGO(SysLog(L"call ClearLastGetSelection()"));
					ClearLastGetSelection();
				}

				_ALGO(SysLog(L"call Plugins.FreeFindData()"));
				Global->CtrlObject->Plugins->FreeFindData(hCurPlugin,ItemList,ItemNumber,true);
				UsedPlugins.emplace(hCurPlugin->pPlugin);
			}

			_ALGO(SysLog(L"call Plugins.ClosePanel"));
			Global->CtrlObject->Plugins->ClosePanel(hCurPlugin);
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
	auto hNewPlugin=Global->CtrlObject->Plugins->OpenFilePlugin(nullptr, 0, OFP_CREATE);

	if (hNewPlugin && hNewPlugin!=PANEL_STOP)
	{
		_ALGO(SysLog(L"Create: FileList TmpPanel, FileCount=%d",FileCount));
		FileList *TmpPanel=new FileList;
		TmpPanel->SetPluginMode(hNewPlugin,L"");  // SendOnFocus??? true???
		TmpPanel->SetModalMode(TRUE);
		auto PrevFileCount = ListData.size();
		/* $ 12.04.2002 IS
		   Если PluginPutFilesToAnother вернула число, отличное от 2, то нужно
		   попробовать установить курсор на созданный файл.
		*/
		int rc=PluginPutFilesToAnother(FALSE,TmpPanel);

		if (rc != 2 && ListData.size() == PrevFileCount+1)
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
			std::for_each(CONST_RANGE(ListData, i)
			{
				if ((i.FileAttr & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					if (PtrLastPos)
					{
						if (PtrLastPos->CreationTime < i.CreationTime)
						{
							LastPos = n;
							PtrLastPos = &i;
						}
					}
					else
					{
						LastPos = n;
						PtrLastPos = &i;
					}
				}
				++n;
			});

			if (PtrLastPos)
			{
				CurFile = LastPos;
				Redraw();
			}
		}
		TmpPanel->Destroy();
	}
}


/* $ 12.04.2002 IS
     PluginPutFilesToAnother теперь int - возвращает то, что возвращает
     PutFiles:
     -1 - прервано пользовтелем
      0 - неудача
      1 - удача
      2 - удача, курсор принудительно установлен на файл и заново его
          устанавливать не нужно (см. PluginPutFilesToNew)
*/
int FileList::PluginPutFilesToAnother(int Move,Panel *AnotherPanel)
{
	if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
		return 0;

	FileList *AnotherFilePanel=(FileList *)AnotherPanel;
	int PutCode=0;
	SaveSelection();
	auto ItemList = CreatePluginItemList();

	if (!ItemList.empty())
	{
		SetCurPath();
		_ALGO(SysLog(L"call Plugins.PutFiles"));
		PutCode=Global->CtrlObject->Plugins->PutFiles(AnotherFilePanel->hPlugin, ItemList.data(), ItemList.size(), Move!=0, 0);

		if (PutCode==1 || PutCode==2)
		{
			if (!ReturnCurrentFile)
			{
				_ALGO(SysLog(L"call ClearSelection()"));
				ClearSelection();
			}

			_ALGO(SysLog(L"call PutDizToPlugin"));
			PutDizToPlugin(AnotherFilePanel, ItemList, FALSE, Move, &Diz);
			AnotherPanel->SetPluginModified();
		}
		else if (!ReturnCurrentFile)
			PluginClearSelection(ItemList);

		_ALGO(SysLog(L"call DeletePluginItemList"));
		DeletePluginItemList(ItemList);
		Update(UPDATE_KEEP_SELECTION);
		Redraw();

		if (AnotherPanel==Global->CtrlObject->Cp()->GetAnotherPanel(this))
		{
			AnotherPanel->Update(UPDATE_KEEP_SELECTION);
			AnotherPanel->Redraw();
		}
	}

	return PutCode;
}


void FileList::GetOpenPanelInfo(OpenPanelInfo *Info) const
{
	_ALGO(CleverSysLog clv(L"FileList::GetOpenPanelInfo()"));
	//_ALGO(SysLog(L"FileName='%s'",(FileName?FileName:"(nullptr)")));
	ClearStruct(*Info);

	if (PanelMode==PLUGIN_PANEL)
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,Info);
}


/*
   Функция для вызова команды "Архивные команды" (Shift-F3)
*/
void FileList::ProcessHostFile()
{
	_ALGO(CleverSysLog clv(L"FileList::ProcessHostFile()"));

	//_ALGO(SysLog(L"FileName='%s'",(FileName?FileName:"(nullptr)")));
	if (!ListData.empty() && SetCurPath())
	{
		int Done=FALSE;
		SaveSelection();

		if (PanelMode==PLUGIN_PANEL && !PluginsList.back().m_HostFile.empty())
		{
			_ALGO(SysLog(L"call CreatePluginItemList"));
			auto ItemList = CreatePluginItemList();
			_ALGO(SysLog(L"call Plugins.ProcessHostFile"));
			Done=Global->CtrlObject->Plugins->ProcessHostFile(hPlugin, ItemList.data(), ItemList.size(), 0);

			if (Done)
				SetPluginModified();
			else
			{
				if (!ReturnCurrentFile)
					PluginClearSelection(ItemList);

				Redraw();
			}

			_ALGO(SysLog(L"call DeletePluginItemList"));
			DeletePluginItemList(ItemList);

			if (Done)
				ClearSelection();
		}
		else
		{
			size_t SCount=GetRealSelCount();

			if (SCount > 0)
			{
				FOR(auto& i, ListData)
				{
					if (i.Selected)
					{
						Done=ProcessOneHostFile(&i);

						if (Done == 1)
							Select(i, FALSE);
						else if (Done == -1)
							continue;
						else       // Если ЭТО убрать, то... будем жать ESC до потере пулься
							break;   //
					}
				}

				if (SelectedFirst)
					SortFileList(TRUE);
			}
			else
			{
				if ((Done=ProcessOneHostFile(&*(ListData.begin() + CurFile))) == 1)
					ClearSelection();
			}
		}

		if (Done)
		{
			Update(UPDATE_KEEP_SELECTION);
			Redraw();
			Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
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
	auto hNewPlugin=OpenPluginForFile(&Item->strName, Item->FileAttr, OFP_COMMANDS);

	if (hNewPlugin && hNewPlugin!=PANEL_STOP)
	{
		PluginPanelItem *ItemList;
		size_t ItemNumber;
		_ALGO(SysLog(L"call Plugins.GetFindData"));

		if (Global->CtrlObject->Plugins->GetFindData(hNewPlugin,&ItemList,&ItemNumber,OPM_TOPLEVEL))
		{
			_ALGO(SysLog(L"call Plugins.ProcessHostFile"));
			Done=Global->CtrlObject->Plugins->ProcessHostFile(hNewPlugin,ItemList,ItemNumber,OPM_TOPLEVEL);
			_ALGO(SysLog(L"call Plugins.FreeFindData"));
			Global->CtrlObject->Plugins->FreeFindData(hNewPlugin,ItemList,ItemNumber,true);
		}

		_ALGO(SysLog(L"call Plugins.ClosePanel"));
		Global->CtrlObject->Plugins->ClosePanel(hNewPlugin);
	}

	return Done;
}



void FileList::SetPluginMode(PluginHandle* hPlugin,const string& PluginFile,bool SendOnFocus)
{
	if (PanelMode!=PLUGIN_PANEL)
	{
		Global->CtrlObject->FolderHistory->AddToHistory(strCurDir);
	}

	PushPlugin(hPlugin,PluginFile);
	FileList::hPlugin=hPlugin;
	PanelMode=PLUGIN_PANEL;

	if (SendOnFocus)
		SetFocus();

	OpenPanelInfo Info;
	Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

	if (Info.StartPanelMode)
		SetViewMode(VIEW_0+Info.StartPanelMode-L'0');

	Global->CtrlObject->Cp()->RedrawKeyBar();

	if (Info.StartSortMode)
	{
		SortMode=Info.StartSortMode-(SM_UNSORTED-UNSORTED);
		ReverseSortOrder = Info.StartSortOrder != 0;
	}

	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->GetType()!=FILE_PANEL)
	{
		AnotherPanel->Update(UPDATE_KEEP_SELECTION);
		AnotherPanel->Redraw();
	}
}

void FileList::PluginGetPanelInfo(PanelInfo &Info)
{
	CorrectPosition();
	Info.CurrentItem=CurFile;
	Info.TopPanelItem=CurTopFile;
	if(ShowShortNames)
		Info.Flags|=PFLAGS_ALTERNATIVENAMES;
	Info.ItemsNumber = ListData.size();
	Info.SelectedItemsNumber=ListData.empty()? 0 : GetSelCount();
}

size_t FileList::PluginGetPanelItem(int ItemNumber,FarGetPluginPanelItem *Item)
{
	size_t result=0;

	if (static_cast<size_t>(ItemNumber) < ListData.size())
	{
		result=FileListToPluginItem2(&ListData[ItemNumber], Item);
	}

	return result;
}

size_t FileList::PluginGetSelectedPanelItem(int ItemNumber,FarGetPluginPanelItem *Item)
{
	size_t result=0;

	if (static_cast<size_t>(ItemNumber) < ListData.size())
	{
		if (ItemNumber==CacheSelIndex)
		{
			result=FileListToPluginItem2(&ListData[CacheSelPos], Item);
		}
		else
		{
			if (ItemNumber<CacheSelIndex) CacheSelIndex=-1;

			int CurSel=CacheSelIndex,StartValue=CacheSelIndex>=0?CacheSelPos+1:0;

			for (size_t i=StartValue; i<ListData.size(); i++)
			{
				if (ListData[i].Selected)
					CurSel++;

				if (CurSel==ItemNumber)
				{
					result=FileListToPluginItem2(&ListData[i], Item);
					CacheSelIndex=ItemNumber;
					CacheSelPos=static_cast<int>(i);
					break;
				}
			}

			if (CurSel==-1 && !ItemNumber)
			{
				result=FileListToPluginItem2(&ListData[CurFile], Item);
				CacheSelIndex=-1;
			}
		}
	}

	return result;
}

void FileList::PluginGetColumnTypesAndWidths(string& strColumnTypes,string& strColumnWidths)
{
	ViewSettingsToText(ViewSettings.PanelColumns, strColumnTypes, strColumnWidths);
}

void FileList::PluginBeginSelection()
{
	SaveSelection();
}

void FileList::PluginSetSelection(int ItemNumber,bool Selection)
{
	Select(ListData[ItemNumber], Selection);
}

void FileList::PluginClearSelection(int SelectedItemNumber)
{
	if (static_cast<size_t>(SelectedItemNumber) < ListData.size())
	{
		if (SelectedItemNumber<=CacheSelClearIndex)
		{
			CacheSelClearIndex=-1;
		}

		int CurSel=CacheSelClearIndex,StartValue=CacheSelClearIndex>=0?CacheSelClearPos+1:0;

		for (size_t i=StartValue; i < ListData.size(); i++)
		{
			if (ListData[i].Selected)
			{
				CurSel++;
			}

			if (CurSel==SelectedItemNumber)
			{
				Select(ListData[i], FALSE);
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
		SortFileList(TRUE);
	}
}

void FileList::ProcessPluginCommand()
{
	_ALGO(CleverSysLog clv(L"FileList::ProcessPluginCommand"));
	_ALGO(SysLog(L"PanelMode=%s",(PanelMode==PLUGIN_PANEL?"PLUGIN_PANEL":"NORMAL_PANEL")));
	int Command=PluginCommand;
	PluginCommand=-1;

	if (PanelMode==PLUGIN_PANEL)
		switch (Command)
		{
			case FCTL_CLOSEPANEL:
				_ALGO(SysLog(L"Command=FCTL_CLOSEPANEL"));
				SetCurDir(strPluginParam,true);

				if (strPluginParam.empty())
					Update(UPDATE_KEEP_SELECTION);

				Redraw();
				break;
		}
}

void FileList::SetPluginModified()
{
	if(!PluginsList.empty())
	{
		PluginsList.back().m_Modified = TRUE;
	}
}


PluginHandle* FileList::GetPluginHandle() const
{
	return hPlugin;
}


int FileList::ProcessPluginEvent(int Event,void *Param)
{
	if (PanelMode==PLUGIN_PANEL)
		return Global->CtrlObject->Plugins->ProcessEvent(hPlugin,Event,Param);

	return FALSE;
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
			while (StrCmpI(CurPlugin.FileName, ListData[FileNumber].strName.data()))
				if (++FileNumber >= ListData.size())
					return;

			Select(ListData[FileNumber++], FALSE);
		}

		PluginNumber++;
	}
}

// flupdate
// Файловая панель - чтение имен файлов

// Флаги для ReadDiz()
enum ReadDizFlags
{
	RDF_NO_UPDATE         = 0x00000001UL,
};

void FileList::Update(int Mode)
{
	_ALGO(CleverSysLog clv(L"FileList::Update"));
	_ALGO(SysLog(L"(Mode=[%d/0x%08X] %s)",Mode,Mode,(Mode==UPDATE_KEEP_SELECTION?L"UPDATE_KEEP_SELECTION":L"")));

	if (EnableUpdate)
		switch (PanelMode)
		{
			case NORMAL_PANEL:
				ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
				break;
			case PLUGIN_PANEL:
			{
				OpenPanelInfo Info;
				Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
				ProcessPluginCommand();

				if (PanelMode!=PLUGIN_PANEL)
					ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
				else if ((Info.Flags & OPIF_REALNAMES) ||
				         Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetMode()==PLUGIN_PANEL ||
				         !(Mode & UPDATE_SECONDARY))
					UpdatePlugin(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE);
			}
			ProcessPluginCommand();
			break;
		}

	LastUpdateTime=clock();
}

void FileList::UpdateIfRequired()
{
	if (UpdateRequired && !UpdateDisabled)
	{
		UpdateRequired = FALSE;
		Update(UpdateRequiredMode | UPDATE_IGNORE_VISIBLE);
	}
}

static void PR_ReadFileNamesMsg();

struct FileListPreRedrawItem : PreRedrawItem
{
	FileListPreRedrawItem() : PreRedrawItem(PR_ReadFileNamesMsg){}

	string Msg;
};

void ReadFileNamesMsg(const string& Msg)
{
	Message(0,0,MSG(MReadingTitleFiles),Msg.data());

	if (!PreRedrawStack().empty())
	{
		auto item = dynamic_cast<FileListPreRedrawItem*>(PreRedrawStack().top());
		item->Msg = Msg;
	}
}

static void PR_ReadFileNamesMsg()
{
	if (!PreRedrawStack().empty())
	{
		auto item = dynamic_cast<const FileListPreRedrawItem*>(PreRedrawStack().top());
		ReadFileNamesMsg(item->Msg);
	}
}

void FileList::ReadFileNames(int KeepSelection, int UpdateEvenIfPanelInvisible, int DrawMessage)
{
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<FileListPreRedrawItem>());
	SCOPED_ACTION(IndeterminateTaskBar)(false);

	strOriginalCurDir = strCurDir;

	if (!IsVisible() && !UpdateEvenIfPanelInvisible)
	{
		UpdateRequired=TRUE;
		UpdateRequiredMode=KeepSelection;
		return;
	}

	UpdateRequired=FALSE;
	AccessTimeUpdateRequired=FALSE;
	DizRead=FALSE;
	api::FAR_FIND_DATA fdata;
	decltype(ListData) OldData;
	string strCurName, strNextCurName;
	StopFSWatcher();

	if (this!=Global->CtrlObject->Cp()->LeftPanel && this!=Global->CtrlObject->Cp()->RightPanel)
		return;

	string strSaveDir;
	api::GetCurrentDirectory(strSaveDir);
	{
		string strOldCurDir(strCurDir);

		if (!SetCurPath())
		{
			FlushInputBuffer(); // Очистим буффер ввода, т.к. мы уже можем быть в другом месте...

			if (strCurDir == strOldCurDir) //?? i??
			{
				GetPathRoot(strOldCurDir,strOldCurDir);

				if (!api::IsDiskInDrive(strOldCurDir))
					IfGoHome(strOldCurDir.front());

				/* При смене каталога путь не изменился */
			}

			return;
		}
	}
	SortGroupsRead=FALSE;

	if (GetFocus())
		Global->CtrlObject->CmdLine->SetCurDir(strCurDir);

	LastCurFile=-1;
	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
	AnotherPanel->QViewDelTempName();
	size_t PrevSelFileCount=SelFileCount;
	SelFileCount=0;
	SelFileSize=0;
	TotalFileCount=0;
	TotalFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;
	FreeDiskSize = -1;
	if (Global->Opt->ShowPanelFree)
	{
		api::GetDiskSize(strCurDir, nullptr, nullptr, &FreeDiskSize);
	}

	if (!ListData.empty())
	{
		strCurName = ListData[CurFile].strName;

		if (ListData[CurFile].Selected && !ReturnCurrentFile)
		{
			for (size_t i=CurFile+1; i < ListData.size(); i++)
			{
				if (!ListData[i].Selected)
				{
					strNextCurName = ListData[i].strName;
					break;
				}
			}
		}
	}

	if (KeepSelection || PrevSelFileCount>0)
	{
		OldData.swap(ListData);
	}
	else
		DeleteListData(ListData);

	DWORD FileSystemFlags = 0;
	string PathRoot;
	GetPathRoot(strCurDir, PathRoot);
	api::GetVolumeInformation(PathRoot, nullptr, nullptr, nullptr, &FileSystemFlags, nullptr);

	ListData.clear();

	bool ReadOwners = IsColumnDisplayed(OWNER_COLUMN);
	bool ReadNumLinks = IsColumnDisplayed(NUMLINK_COLUMN);
	bool ReadNumStreams = IsColumnDisplayed(NUMSTREAMS_COLUMN);
	bool ReadStreamsSize = IsColumnDisplayed(STREAMSSIZE_COLUMN);

	if (!(FileSystemFlags&FILE_SUPPORTS_HARD_LINKS) && IsWindows7OrGreater())
	{
		ReadNumLinks = false;
	}

	if(!(FileSystemFlags&FILE_NAMED_STREAMS))
	{
		ReadNumStreams = false;
		ReadStreamsSize = false;
	}

	string strComputerName;

	if (ReadOwners)
	{
		string strTemp;
		CurPath2ComputerName(strCurDir, strComputerName, strTemp);
	}

	SetLastError(ERROR_SUCCESS);
	// сформируем заголовок вне цикла
	string Title = MakeSeparator(X2-X1-1, 9, nullptr);
	BOOL IsShowTitle=FALSE;
	BOOL NeedHighlight=Global->Opt->Highlight && PanelMode != PLUGIN_PANEL;

	if (!Filter)
		Filter = std::make_unique<FileFilter>(this,FFT_PANEL);

	//Рефреш текущему времени для фильтра перед началом операции
	Filter->UpdateCurrentTime();
	Global->CtrlObject->HiFiles->UpdateCurrentTime();
	bool bCurDirRoot = false;
	ParsePath(strCurDir, nullptr, &bCurDirRoot);
	PATH_TYPE Type = ParsePath(strCurDir, nullptr, &bCurDirRoot);
	bool NetRoot = bCurDirRoot && (Type == PATH_REMOTE || Type == PATH_REMOTEUNC);

	string strFind(strCurDir);
	AddEndSlash(strFind);
	strFind+=L'*';
	api::enum_file Find(strFind, true);
	DWORD FindErrorCode = ERROR_SUCCESS;
	bool UseFilter=Filter->IsEnabledOnPanel();
	bool ReadCustomData=IsColumnDisplayed(CUSTOM_COLUMN0)!=0;

	DWORD StartTime = GetTickCount();

	std::all_of(CONST_RANGE(Find, fdata) -> bool
	{
		Global->CatchError();
		FindErrorCode = Global->CaughtError();

		if ((Global->Opt->ShowHidden || !(fdata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))) && (!UseFilter || Filter->FileInFilter(fdata, nullptr, &fdata.strFileName)))
		{
			if (ListData.size() == ListData.capacity())
				ListData.reserve(ListData.size() + 4096);

			ListData.emplace_back(VALUE_TYPE(ListData)());
			auto& NewItem = ListData.back();

			NewItem.FileAttr = fdata.dwFileAttributes;
			NewItem.CreationTime = fdata.ftCreationTime;
			NewItem.AccessTime = fdata.ftLastAccessTime;
			NewItem.WriteTime = fdata.ftLastWriteTime;
			NewItem.ChangeTime = fdata.ftChangeTime;
			NewItem.FileSize = fdata.nFileSize;
			NewItem.AllocationSize = fdata.nAllocationSize;
			NewItem.strName = fdata.strFileName;
			NewItem.strShortName = fdata.strAlternateFileName;
			NewItem.Position = ListData.size() - 1;
			NewItem.NumberOfLinks=1;

			if (fdata.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			{
				NewItem.ReparseTag=fdata.dwReserved0; //MSDN
			}
			if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				TotalFileSize += NewItem.FileSize;

				if (ReadNumLinks)
					NewItem.NumberOfLinks = GetNumberOfLinks(fdata.strFileName, true);
			}
			else
			{
				NewItem.AllocationSize = 0;
			}

			NewItem.SortGroup=DEFAULT_SORT_GROUP;

			if (ReadOwners)
			{
				string strOwner;
				GetFileOwner(strComputerName, NewItem.strName,strOwner);
				NewItem.strOwner = strOwner;
			}

			NewItem.NumberOfStreams=NewItem.FileAttr&FILE_ATTRIBUTE_DIRECTORY?0:1;
			NewItem.StreamsSize=NewItem.FileSize;

			if (ReadNumStreams||ReadStreamsSize)
			{
				EnumStreams(TestParentFolderName(fdata.strFileName)? strCurDir : fdata.strFileName, NewItem.StreamsSize, NewItem.NumberOfStreams);
			}

			if (ReadCustomData)
				NewItem.strCustomData = Global->CtrlObject->Plugins->GetCustomData(NewItem.strName);

			if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				TotalFileCount++;

			DWORD CurTime = GetTickCount();
			if (CurTime - StartTime > (DWORD)Global->Opt->RedrawTimeout)
			{
				StartTime = CurTime;
				if (IsVisible())
				{
					if (!IsShowTitle)
					{
						if (!DrawMessage)
						{
							Text(X1+1,Y1,ColorIndexToColor(COL_PANELBOX),Title);
							IsShowTitle=TRUE;
							SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
						}
					}

					LangString strReadMsg(MReadingFiles);
					strReadMsg << ListData.size();

					if (DrawMessage)
					{
						ReadFileNamesMsg(strReadMsg);
					}
					else
					{
						TruncStr(strReadMsg,static_cast<int>(Title.size())-2);
						int MsgLength=(int)strReadMsg.size();
						GotoXY(X1+1+(static_cast<int>(Title.size())-MsgLength-1)/2,Y1);
						Global->FS << L" "<<strReadMsg<<L" ";
					}
				}

				Global->CtrlObject->Macro.SuspendMacros(true);
				bool check = CheckForEsc();
				Global->CtrlObject->Macro.SuspendMacros(false);
				if (check)
				{
					// break loop
					return false;
				}
			}
		}
		return true;
	});

	if (!(FindErrorCode==ERROR_SUCCESS || FindErrorCode==ERROR_NO_MORE_FILES || FindErrorCode==ERROR_FILE_NOT_FOUND))
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MReadFolderError),MSG(MOk));

	if ((Global->Opt->ShowDotsInRoot || !bCurDirRoot) || (NetRoot && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Network.Id))) // NetWork Plugin
	{
		ListData.emplace_back(VALUE_TYPE(ListData)());
		auto& NewItem = ListData.back();

		string TwoDotsOwner;
		if (ReadOwners)
		{
			GetFileOwner(strComputerName,strCurDir,TwoDotsOwner);
		}

		FILETIME TwoDotsTimes[4]={};
		api::GetFileTimeSimple(strCurDir,&TwoDotsTimes[0],&TwoDotsTimes[1],&TwoDotsTimes[2],&TwoDotsTimes[3]);

		AddParentPoint(&NewItem, ListData.size(), TwoDotsTimes, TwoDotsOwner);
	}

	if (IsColumnDisplayed(DIZ_COLUMN))
		ReadDiz();

	if (NeedHighlight)
	{
		std::for_each(RANGE(ListData, i)
		{
			Global->CtrlObject->HiFiles->GetHiColor(&i);
		});
	}

	if (AnotherPanel->GetMode()==PLUGIN_PANEL)
	{
		auto hAnotherPlugin=AnotherPanel->GetPluginHandle();
		PluginPanelItem *PanelData=nullptr;
		string strPath(strCurDir);
		AddEndSlash(strPath);
		size_t PanelCount=0;

		if (Global->CtrlObject->Plugins->GetVirtualFindData(hAnotherPlugin,&PanelData,&PanelCount,strPath))
		{
			auto OldSize = ListData.size(), Position = OldSize - 1;
			ListData.resize(ListData.size() + PanelCount);

			auto PluginPtr = PanelData;
			FOR(auto& i, make_range(ListData.begin() + OldSize, ListData.end()))
			{
				PluginToFileListItem(PluginPtr, &i);
				i.Position = Position;
				TotalFileSize += PluginPtr->FileSize;
				i.PrevSelected = i.Selected=0;
				i.ShowFolderSize = 0;
				i.SortGroup=Global->CtrlObject->HiFiles->GetGroup(&i);

				if (!TestParentFolderName(PluginPtr->FileName) && !(i.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
					TotalFileCount++;

				++PluginPtr;
				++Position;
			}

			// цветовую боевую раскраску в самом конце, за один раз
			FOR(auto& i, make_range(ListData.begin() + OldSize, ListData.begin() + OldSize + PanelCount))
			{
				Global->CtrlObject->HiFiles->GetHiColor(&i);
			}
			Global->CtrlObject->Plugins->FreeVirtualFindData(hAnotherPlugin,PanelData,PanelCount);
		}
	}

	InitFSWatcher(false);
	CorrectPosition();

	string strLastSel, strGetSel;

	if (KeepSelection || PrevSelFileCount>0)
	{
		if (LastSelPosition >= 0 && static_cast<size_t>(LastSelPosition) < OldData.size())
			strLastSel = OldData[LastSelPosition].strName;
		if (GetSelPosition >= 0 && static_cast<size_t>(GetSelPosition) < OldData.size())
			strGetSel = OldData[GetSelPosition].strName;

		MoveSelection(OldData, ListData);
		DeleteListData(OldData);
	}

	if (SortGroups)
		ReadSortGroups(false);

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(FALSE);

	if (!strLastSel.empty())
		LastSelPosition = FindFile(strLastSel, FALSE);
	if (!strGetSel.empty())
		GetSelPosition = FindFile(strGetSel, FALSE);

	if (CurFile >= static_cast<int>(ListData.size()) || StrCmpI(ListData[CurFile].strName, strCurName))
		if (!GoToFile(strCurName) && !strNextCurName.empty())
			GoToFile(strNextCurName);

	/* $ 13.02.2002 DJ
		SetTitle() - только если мы текущий фрейм!
	*/
	if (Global->CtrlObject->Cp() == Global->FrameManager->GetCurrentFrame())
		SetTitle();

	FarChDir(strSaveDir); //???
}

/*$ 22.06.2001 SKV
  Добавлен параметр для вызова после исполнения команды.
*/
int FileList::UpdateIfChanged(panel_update_mode UpdateMode)
{
	//_SVS(SysLog(L"CurDir='%s' Global->Opt->AutoUpdateLimit=%d <= FileCount=%d",CurDir,Global->Opt->AutoUpdateLimit,FileCount));
	if (!Global->Opt->AutoUpdateLimit || ListData.size() <= static_cast<size_t>(Global->Opt->AutoUpdateLimit))
	{
		/* $ 19.12.2001 VVM
		  ! Сменим приоритеты. При Force обновление всегда! */
		if ((IsVisible() && (clock()-LastUpdateTime>2000)) || (UpdateMode != UIC_UPDATE_NORMAL))
		{
			if (UpdateMode == UIC_UPDATE_NORMAL)
				ProcessPluginEvent(FE_IDLE,nullptr);

			/* $ 24.12.2002 VVM
			  ! Поменяем логику обновления панелей. */
			if (// Нормальная панель, на ней установлено уведомление и есть сигнал
			    (PanelMode==NORMAL_PANEL && FSWatcher.Signaled()) ||
			    // Или Нормальная панель, но нет уведомления и мы попросили обновить через UPDATE_FORCE
			    (PanelMode==NORMAL_PANEL && UpdateMode==UIC_UPDATE_FORCE) ||
			    // Или плагинная панель и обновляем через UPDATE_FORCE
			    (PanelMode!=NORMAL_PANEL && UpdateMode==UIC_UPDATE_FORCE)
			)
			{
				Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

				if (AnotherPanel->GetType()==INFO_PANEL)
				{
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);

					if (UpdateMode==UIC_UPDATE_NORMAL)
						AnotherPanel->Redraw();
				}

				Update(UPDATE_KEEP_SELECTION);

				if (UpdateMode==UIC_UPDATE_NORMAL)
					Show();

				return TRUE;
			}
		}
	}

	return FALSE;
}

void FileList::InitFSWatcher(bool CheckTree)
{
	DWORD DriveType=DRIVE_REMOTE;
	StopFSWatcher();
	PATH_TYPE Type = ParsePath(strCurDir);

	if (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC)
	{
		wchar_t RootDir[4]=L" :\\";
		RootDir[0] = strCurDir[(Type == PATH_DRIVELETTER)? 0 : 4];
		DriveType=FAR_GetDriveType(RootDir);
	}

	if (Global->Opt->AutoUpdateRemoteDrive || (!Global->Opt->AutoUpdateRemoteDrive && DriveType != DRIVE_REMOTE) || Type == PATH_VOLUMEGUID)
	{
		FSWatcher.Set(strCurDir, CheckTree);
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

struct search_list_less
{
	bool operator()(const FileListItem& a, const FileListItem& b) const
	{
		return a.strName < b.strName;
	}
}
SearchListLess;

void FileList::MoveSelection(std::vector<FileListItem>& From, std::vector<FileListItem>& To)
{
	SelFileCount=0;
	SelFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;

	std::sort(From.begin(), From.end(), SearchListLess);

	std::for_each(RANGE(To, i)
	{
		auto OldItem = std::lower_bound(ALL_CONST_RANGE(From), i, SearchListLess);
		if (OldItem != From.end())
		{
			if (OldItem->strName == i.strName)
			{
				if (OldItem->ShowFolderSize)
				{
					i.ShowFolderSize = 2;
					i.FileSize = OldItem->FileSize;
					i.AllocationSize = OldItem->AllocationSize;
				}

				this->Select(i, OldItem->Selected);
				i.PrevSelected = OldItem->PrevSelected;
			}
		}
	});
}

void FileList::UpdatePlugin(int KeepSelection, int UpdateEvenIfPanelInvisible)
{
	_ALGO(CleverSysLog clv(L"FileList::UpdatePlugin"));
	_ALGO(SysLog(L"(KeepSelection=%d, IgnoreVisible=%d)",KeepSelection,UpdateEvenIfPanelInvisible));

	if (!IsVisible() && !UpdateEvenIfPanelInvisible)
	{
		UpdateRequired=TRUE;
		UpdateRequiredMode=KeepSelection;
		return;
	}

	DizRead=FALSE;
	std::vector<FileListItem> OldData;
	string strCurName, strNextCurName;
	StopFSWatcher();
	LastCurFile=-1;
	OpenPanelInfo Info;
	Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

	FreeDiskSize=-1;
	if (Global->Opt->ShowPanelFree)
	{
		if (Info.Flags & OPIF_REALNAMES)
		{
			api::GetDiskSize(strCurDir, nullptr, nullptr, &FreeDiskSize);
		}
		else if (Info.Flags & OPIF_USEFREESIZE)
			FreeDiskSize=Info.FreeSize;
	}

	PluginPanelItem *PanelData=nullptr;
	size_t PluginFileCount;

	if (!Global->CtrlObject->Plugins->GetFindData(hPlugin,&PanelData,&PluginFileCount,0))
	{
		PopPlugin(TRUE);
		Update(KeepSelection);

		// WARP> явный хак, но очень способствует - восстанавливает позицию на панели при ошибке чтения архива.
		if (!PrevDataList.empty())
			GoToFile(PrevDataList.back().strPrevName);

		return;
	}

	size_t PrevSelFileCount=SelFileCount;
	SelFileCount=0;
	SelFileSize=0;
	TotalFileCount=0;
	TotalFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;
	strPluginDizName.clear();

	if (!ListData.empty())
	{
		strCurName = ListData[CurFile].strName;

		if (ListData[CurFile].Selected)
		{
			auto ItemIterator = std::find_if(ListData.cbegin() + CurFile + 1, ListData.cend(), [](CONST_VALUE_TYPE(ListData)& i) { return !i.Selected; });
			if (ItemIterator != ListData.cend())
			{
				strNextCurName = ItemIterator->strName;
			}
		}
	}
	else if (Info.Flags & OPIF_ADDDOTS)
	{
		strCurName = L"..";
	}

	if (KeepSelection || PrevSelFileCount>0)
	{
		OldData.swap(ListData);
	}
	else
	{
		DeleteListData(ListData);
	}

	if (!Filter)
		Filter = std::make_unique<FileFilter>(this, FFT_PANEL);

	//Рефреш текущему времени для фильтра перед началом операции
	Filter->UpdateCurrentTime();
	Global->CtrlObject->HiFiles->UpdateCurrentTime();
	int DotsPresent=FALSE;
	bool UseFilter=Filter->IsEnabledOnPanel();

	ListData.reserve(PluginFileCount);

	for (size_t i = 0; i < PluginFileCount; i++)
	{
		if (UseFilter && !(Info.Flags & OPIF_DISABLEFILTER))
		{
			//if (!(CurPanelData->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			if (!Filter->FileInFilter(PanelData[i]))
				continue;
		}

		if (!Global->Opt->ShowHidden && (PanelData[i].FileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
			continue;

		ListData.emplace_back(VALUE_TYPE(ListData)());
		auto& CurListData = ListData.back();

		PluginToFileListItem(&PanelData[i], &CurListData);
		CurListData.Position=i;

		if (!(Info.Flags & OPIF_DISABLESORTGROUPS)/* && !(CurListData->FileAttr & FILE_ATTRIBUTE_DIRECTORY)*/)
			CurListData.SortGroup=Global->CtrlObject->HiFiles->GetGroup(&CurListData);
		else
			CurListData.SortGroup=DEFAULT_SORT_GROUP;

		if (!CurListData.DizText)
		{
			CurListData.DeleteDiz=false;
			//CurListData.DizText=nullptr;
		}

		if (TestParentFolderName(CurListData.strName))
		{
			DotsPresent=TRUE;
			CurListData.FileAttr|=FILE_ATTRIBUTE_DIRECTORY;
		}
		else if (!(CurListData.FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			TotalFileCount++;
		}

		TotalFileSize += CurListData.FileSize;
	}

	if (!(Info.Flags & OPIF_DISABLEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
	{
		std::for_each(RANGE(ListData, i)
		{
			Global->CtrlObject->HiFiles->GetHiColor(&i, (Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);
		});
	}

	if ((Info.Flags & OPIF_ADDDOTS) && !DotsPresent)
	{
		ListData.emplace_back(VALUE_TYPE(ListData)());
		auto& NewItem = ListData.back();
		AddParentPoint(&NewItem, ListData.size());

		if (!(Info.Flags & OPIF_DISABLEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
			Global->CtrlObject->HiFiles->GetHiColor(&ListData.back(), (Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);

		if (Info.HostFile && *Info.HostFile)
		{
			api::FAR_FIND_DATA FindData;

			if (api::GetFindDataEx(Info.HostFile, FindData))
			{
				NewItem.WriteTime=FindData.ftLastWriteTime;
				NewItem.CreationTime=FindData.ftCreationTime;
				NewItem.AccessTime=FindData.ftLastAccessTime;
				NewItem.ChangeTime=FindData.ftChangeTime;
			}
		}
	}

	if (CurFile >= static_cast<int>(ListData.size()))
		CurFile = ListData.size() ? static_cast<int>(ListData.size() - 1) : 0;

	/* $ 25.02.2001 VVM
	    ! Не считывать повторно список файлов с панели плагина */
	if (IsColumnDisplayed(DIZ_COLUMN))
		ReadDiz(PanelData,static_cast<int>(PluginFileCount),RDF_NO_UPDATE);

	CorrectPosition();
	Global->CtrlObject->Plugins->FreeFindData(hPlugin,PanelData,PluginFileCount,false);

	string strLastSel, strGetSel;

	if (KeepSelection || PrevSelFileCount>0)
	{
		if (LastSelPosition >= 0 && LastSelPosition < static_cast<long>(OldData.size()))
			strLastSel = OldData[LastSelPosition].strName;
		if (GetSelPosition >= 0 && GetSelPosition < static_cast<long>(OldData.size()))
			strGetSel = OldData[GetSelPosition].strName;

		MoveSelection(OldData, ListData);
		DeleteListData(OldData);
	}

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(FALSE);

	if (!strLastSel.empty())
		LastSelPosition = FindFile(strLastSel, FALSE);
	if (!strGetSel.empty())
		GetSelPosition = FindFile(strGetSel, FALSE);

	if (CurFile >= static_cast<int>(ListData.size()) || StrCmpI(ListData[CurFile].strName,strCurName))
		if (!GoToFile(strCurName) && !strNextCurName.empty())
			GoToFile(strNextCurName);

	SetTitle();
}


void FileList::ReadDiz(PluginPanelItem *ItemList,int ItemLength,DWORD dwFlags)
{
	if (DizRead)
		return;

	DizRead=TRUE;
	Diz.Reset();

	if (PanelMode==NORMAL_PANEL)
	{
		Diz.Read(strCurDir);
	}
	else
	{
		PluginPanelItem *PanelData=nullptr;
		size_t PluginFileCount=0;
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

		if (!Info.DescrFilesNumber)
			return;

		int GetCode=TRUE;

		/* $ 25.02.2001 VVM
		    + Обработка флага RDF_NO_UPDATE */
		if (!ItemList && !(dwFlags & RDF_NO_UPDATE))
		{
			GetCode=Global->CtrlObject->Plugins->GetFindData(hPlugin,&PanelData,&PluginFileCount,0);
		}
		else
		{
			PanelData=ItemList;
			PluginFileCount=ItemLength;
		}

		if (GetCode)
		{
			for (size_t I=0; I<Info.DescrFilesNumber; I++)
			{
				PluginPanelItem *CurPanelData=PanelData;

				for (size_t J=0; J < PluginFileCount; J++, CurPanelData++)
				{
					string strFileName = CurPanelData->FileName;

					if (!StrCmpI(strFileName.data(),Info.DescrFiles[I]))
					{
						string strTempDir, strDizName;

						if (FarMkTempEx(strTempDir) && api::CreateDirectory(strTempDir,nullptr))
						{
							if (Global->CtrlObject->Plugins->GetFile(hPlugin,CurPanelData,strTempDir,strDizName,OPM_SILENT|OPM_VIEW|OPM_QUICKVIEW|OPM_DESCR))
							{
								strPluginDizName = Info.DescrFiles[I];
								Diz.Read(L"", &strDizName);
								DeleteFileWithFolder(strDizName);
								I=Info.DescrFilesNumber;
								break;
							}

							api::RemoveDirectory(strTempDir);
							//ViewPanel->ShowFile(nullptr,FALSE,nullptr);
						}
					}
				}
			}

			/* $ 25.02.2001 VVM
			    + Обработка флага RDF_NO_UPDATE */
			if (!ItemList && !(dwFlags & RDF_NO_UPDATE))
				Global->CtrlObject->Plugins->FreeFindData(hPlugin,PanelData,PluginFileCount,true);
		}
	}

	std::for_each(RANGE(ListData, i)
	{
		if (!i.DizText)
		{
			i.DeleteDiz = false;
			i.DizText = Diz.GetDizTextAddr(i.strName, i.strShortName, i.FileSize);
		}
	});
}


void FileList::ReadSortGroups(bool UpdateFilterCurrentTime)
{
	if (!SortGroupsRead)
	{
		if (UpdateFilterCurrentTime)
		{
			Global->CtrlObject->HiFiles->UpdateCurrentTime();
		}

		SortGroupsRead=TRUE;

		std::for_each(RANGE(ListData, i)
		{
			i.SortGroup = Global->CtrlObject->HiFiles->GetGroup(&i);
		});
	}
}

// занести предопределенные данные для каталога "..". Ожидается, что CurPtr пуст.
void FileList::AddParentPoint(FileListItem *CurPtr, size_t CurFilePos, const FILETIME* Times, const string& Owner)
{
	CurPtr->FileAttr = FILE_ATTRIBUTE_DIRECTORY;
	CurPtr->strName = L"..";
	CurPtr->strShortName = L"..";

	if (Times)
	{
		CurPtr->CreationTime = Times[0];
		CurPtr->AccessTime = Times[1];
		CurPtr->WriteTime = Times[2];
		CurPtr->ChangeTime = Times[3];
	}

	CurPtr->strOwner = Owner;
	CurPtr->Position = CurFilePos;
}

// flshow.cpp
// Файловая панель - вывод на экран

extern int ColumnTypeWidth[];

static wchar_t OutCharacter[8]={};

static LNGID __FormatEndSelectedPhrase(size_t Count)
{
	LNGID M_Fmt=MListFileSize;

	if (Count != 1)
	{
		FormatString StrItems;
		StrItems << Count;
		size_t LenItems= StrItems.size();

		if (StrItems[LenItems-1] == '1' && Count != 11)
			M_Fmt=MListFilesSize1;
		else
			M_Fmt=MListFilesSize2;
	}

	return M_Fmt;
}


void FileList::DisplayObject()
{
	Height=Y2-Y1-4+!Global->Opt->ShowColumnTitles+(Global->Opt->ShowPanelStatus ? 0:2);
	_OT(SysLog(L"[%p] FileList::DisplayObject()",this));

	if (UpdateRequired)
	{
		UpdateRequired=FALSE;
		Update(UpdateRequiredMode);
	}

	ProcessPluginCommand();
	ShowFileList(FALSE);
}


void FileList::ShowFileList(int Fast)
{
	if (Locked())
	{
		CorrectPosition();
		return;
	}

	string strTitle;
	string strInfoCurDir;
	OpenPanelInfo Info;

	if (PanelMode==PLUGIN_PANEL)
	{
		if (ProcessPluginEvent(FE_REDRAW,nullptr))
			return;

		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		strInfoCurDir = NullToEmpty(Info.CurDir);
	}

	bool CurFullScreen=IsFullScreen();
	PrepareViewSettings(ViewMode,&Info);
	CorrectPosition();

	if (CurFullScreen!=IsFullScreen())
	{
		Global->CtrlObject->Cp()->SetScreenPosition();
		Global->CtrlObject->Cp()->GetAnotherPanel(this)->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	}

	SetScreen(X1+1,Y1+1,X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));
	Box(X1,Y1,X2,Y2,ColorIndexToColor(COL_PANELBOX),DOUBLE_BOX);

	if (Global->Opt->ShowColumnTitles)
	{
//    SetScreen(X1+1,Y1+1,X2-1,Y1+1,' ',COL_PANELTEXT);
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y1+1);
		//Global->FS << fmt::Width(X2-X1-1)<<L"";
	}

	for (size_t I=0,ColumnPos=X1+1; I < ViewSettings.PanelColumns.size(); I++)
	{
		if (ViewSettings.PanelColumns[I].width < 0)
			continue;

		if (Global->Opt->ShowColumnTitles)
		{
			LNGID IDMessage=MColumnUnknown;

			switch (ViewSettings.PanelColumns[I].type & 0xff)
			{
				case NAME_COLUMN:
					IDMessage=MColumnName;
					break;
				case EXTENSION_COLUMN:
					IDMessage=MColumnExtension;
					break;
				case SIZE_COLUMN:
					IDMessage=MColumnSize;
					break;
				case PACKED_COLUMN:
					IDMessage=MColumnAlocatedSize;
					break;
				case DATE_COLUMN:
					IDMessage=MColumnDate;
					break;
				case TIME_COLUMN:
					IDMessage=MColumnTime;
					break;
				case WDATE_COLUMN:
					IDMessage=MColumnWrited;
					break;
				case CDATE_COLUMN:
					IDMessage=MColumnCreated;
					break;
				case ADATE_COLUMN:
					IDMessage=MColumnAccessed;
					break;
				case CHDATE_COLUMN:
					IDMessage=MColumnChanged;
					break;
				case ATTR_COLUMN:
					IDMessage=MColumnAttr;
					break;
				case DIZ_COLUMN:
					IDMessage=MColumnDescription;
					break;
				case OWNER_COLUMN:
					IDMessage=MColumnOwner;
					break;
				case NUMLINK_COLUMN:
					IDMessage=MColumnMumLinks;
					break;
				case NUMSTREAMS_COLUMN:
					IDMessage=MColumnNumStreams;
					break;
				case STREAMSSIZE_COLUMN:
					IDMessage=MColumnStreamsSize;
					break;
			}

			strTitle=MSG(IDMessage);

			if (PanelMode==PLUGIN_PANEL && Info.PanelModesArray &&
			        ViewMode<static_cast<int>(Info.PanelModesNumber) &&
			        Info.PanelModesArray[ViewMode].ColumnTitles)
			{
				const wchar_t *NewTitle=Info.PanelModesArray[ViewMode].ColumnTitles[I];

				if (NewTitle)
					strTitle=NewTitle;
			}

			string strTitleMsg;
			CenterStr(strTitle,strTitleMsg,ViewSettings.PanelColumns[I].width);
			SetColor(COL_PANELCOLUMNTITLE);
			GotoXY(static_cast<int>(ColumnPos),Y1+1);
			Global->FS << fmt::MaxWidth(ViewSettings.PanelColumns[I].width) << strTitleMsg;
		}

		if (I == ViewSettings.PanelColumns.size() - 1)
			break;

		if (ViewSettings.PanelColumns[I + 1].width < 0)
			continue;

		SetColor(COL_PANELBOX);
		ColumnPos += ViewSettings.PanelColumns[I].width;
		GotoXY(static_cast<int>(ColumnPos),Y1);

		bool DoubleLine = Global->Opt->DoubleGlobalColumnSeparator && (!((I+1)%ColumnsInGlobal));

		BoxText(BoxSymbols[DoubleLine?BS_T_H2V2:BS_T_H2V1]);

		if (Global->Opt->ShowColumnTitles)
		{
			FarColor c = ColorIndexToColor(COL_PANELBOX);
			c.BackgroundColor = ColorIndexToColor(COL_PANELCOLUMNTITLE).BackgroundColor;
			SetColor(c);

			GotoXY(static_cast<int>(ColumnPos),Y1+1);
			BoxText(BoxSymbols[DoubleLine?BS_V2:BS_V1]);
		}

		if (!Global->Opt->ShowPanelStatus)
		{
			GotoXY(static_cast<int>(ColumnPos),Y2);
			BoxText(BoxSymbols[DoubleLine?BS_B_H2V2:BS_B_H2V1]);
		}

		ColumnPos++;
	}

	int NextX1=X1+1;

	if (Global->Opt->ShowSortMode)
	{
		const wchar_t *Ch = nullptr;
		if (SortMode < SORTMODE_COUNT)
		{
			static const simple_pair<int, LNGID> ModeNames[] =
			{
				{UNSORTED, MMenuUnsorted},
				{BY_NAME, MMenuSortByName},
				{BY_EXT, MMenuSortByExt},
				{BY_MTIME, MMenuSortByWrite},
				{BY_CTIME, MMenuSortByCreation},
				{BY_ATIME, MMenuSortByAccess},
				{BY_CHTIME, MMenuSortByChange},
				{BY_SIZE, MMenuSortBySize},
				{BY_DIZ, MMenuSortByDiz},
				{BY_OWNER, MMenuSortByOwner},
				{BY_COMPRESSEDSIZE, MMenuSortByAllocatedSize},
				{BY_NUMLINKS, MMenuSortByNumLinks},
				{BY_NUMSTREAMS, MMenuSortByNumStreams},
				{BY_STREAMSSIZE, MMenuSortByStreamsSize},
				{BY_FULLNAME, MMenuSortByFullName},
				{BY_CUSTOMDATA, MMenuSortByCustomData},
			};
			static_assert(ARRAYSIZE(ModeNames) == SORTMODE_COUNT, "Incomplete ModeNames array");

			Ch = wcschr(MSG(std::find_if(CONST_RANGE(ModeNames, i) { return i.first == SortMode; })->second), L'&');
		}

		if (Ch || SortMode >= SORTMODE_COUNT)
		{
			if (Global->Opt->ShowColumnTitles)
				GotoXY(NextX1,Y1+1);
			else
				GotoXY(NextX1,Y1);

			SetColor(COL_PANELCOLUMNTITLE);
			if (Ch)
				OutCharacter[0] = ReverseSortOrder? Upper(Ch[1]) : Lower(Ch[1]);
			else
				OutCharacter[0] = ReverseSortOrder? CustomSortIndicator[1] : CustomSortIndicator[0];

			Text(OutCharacter);
			NextX1++;

			if (Filter && Filter->IsEnabledOnPanel())
			{
				OutCharacter[0]=L'*';
				Text(OutCharacter);
				NextX1++;
			}
		}
	}

	/* <режимы сортировки> */
	if (/*GetNumericSort() || GetCaseSensitiveSort() || GetSortGroups() || */GetSelectedFirstMode())
	{
		if (Global->Opt->ShowColumnTitles)
			GotoXY(NextX1,Y1+1);
		else
			GotoXY(NextX1,Y1);

		SetColor(COL_PANELCOLUMNTITLE);
		wchar_t *PtrOutCharacter=OutCharacter;
		*PtrOutCharacter=0;

		//if (GetSelectedFirstMode())
			*PtrOutCharacter++=L'^';

		/*
		    if(GetNumericSort())
		      *PtrOutCharacter++=L'#';
		    if(GetSortGroups())
		      *PtrOutCharacter++=L'@';
		*/
		/*
		if(GetCaseSensitiveSort())
		{

		}
		*/
		*PtrOutCharacter=0;
		Text(OutCharacter);
		PtrOutCharacter[1]=0;
	}

	/* </режимы сортировки> */

	if (!Fast && GetFocus())
	{
		if (PanelMode==PLUGIN_PANEL)
			Global->CtrlObject->CmdLine->SetCurDir(NullToEmpty(Info.CurDir));
		else
			Global->CtrlObject->CmdLine->SetCurDir(strCurDir);

		Global->CtrlObject->CmdLine->Show();
	}

	GetTitle(strTitle);
	int TitleX2 = X2 == ScrX? X2 - 1 : X2;
	if (Global->Opt->Clock && !Global->Opt->ShowMenuBar && X1 + strTitle.size() + 2 >= static_cast<size_t>(ScrX-5))
		TitleX2 = std::min(ScrX-5,(int)X2);

	int MaxSize=TitleX2-X1-1;
	int XShift = 0;
	if (!Global->Opt->ShowColumnTitles && Global->Opt->ShowSortMode)
	{
		++XShift;
		if (Filter && Filter->IsEnabledOnPanel())
			++XShift;
	}
	MaxSize -= XShift;
	TruncPathStr(strTitle, MaxSize - 2);
	strTitle.insert(0, 1, L' ');
	strTitle.push_back(L' ');

	size_t TitleX=X1+1+XShift+(TitleX2-X1-XShift-strTitle.size())/2;

	if (Global->Opt->Clock && !Global->Opt->ShowMenuBar && TitleX + strTitle.size() > static_cast<size_t>(ScrX-5))
		TitleX = ScrX - 5 - strTitle.size();

	SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
	GotoXY(static_cast<int>(TitleX),Y1);
	Text(strTitle);

	if (ListData.empty())
	{
		SetScreen(X1+1,Y2-1,X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y2-1);
		//Global->FS << fmt::Width(X2-X1-1)<<L"";
	}

	if (PanelMode==PLUGIN_PANEL && !ListData.empty() && (Info.Flags & OPIF_REALNAMES))
	{
		if (!strInfoCurDir.empty())
		{
			strCurDir = strInfoCurDir;
		}
		else
		{
			if (!TestParentFolderName(ListData[CurFile].strName))
			{
				strCurDir=ListData[CurFile].strName;
				size_t pos;

				if (FindLastSlash(pos,strCurDir))
				{
					if (pos)
					{
						if (strCurDir[pos-1] != L':')
							strCurDir.resize(pos);
						else
							strCurDir.resize(pos+1);
					}
				}
			}
			else
			{
				strCurDir = strOriginalCurDir;
			}
		}

		if (GetFocus())
		{
			Global->CtrlObject->CmdLine->SetCurDir(strCurDir);
			Global->CtrlObject->CmdLine->Show();
		}
	}

	if ((Global->Opt->ShowPanelTotals || Global->Opt->ShowPanelFree) &&
	        (Global->Opt->ShowPanelStatus || !SelFileCount))
	{
		ShowTotalSize(Info);
	}

	ShowList(FALSE,0);
	ShowSelectedSize();

	if (Global->Opt->ShowPanelScrollbar)
	{
		SetColor(COL_PANELSCROLLBAR);
		ScrollBarEx(X2,Y1+1+Global->Opt->ShowColumnTitles,Height,Round(CurTopFile,Columns),Round(static_cast<int>(ListData.size()), Columns));
	}

	ShowScreensCount();

	if (!ProcessingPluginCommand && LastCurFile!=CurFile)
	{
		LastCurFile=CurFile;
		UpdateViewPanel();
	}

	if (PanelMode==PLUGIN_PANEL)
		Global->CtrlObject->Cp()->RedrawKeyBar();
}


const FarColor FileList::GetShowColor(int Position, bool FileColor) const
{
	FarColor ColorAttr=ColorIndexToColor(COL_PANELTEXT);

	if (static_cast<size_t>(Position) < ListData.size())
	{
		int Pos = HighlightFiles::NORMAL_COLOR;

		if (CurFile==Position && Focus && !ListData.empty())
		{
			Pos=ListData[Position].Selected? HighlightFiles::SELECTEDUNDERCURSOR_COLOR : HighlightFiles::UNDERCURSOR_COLOR;
		}
		else if (ListData[Position].Selected)
			Pos = HighlightFiles::SELECTED_COLOR;

		if (Global->Opt->Highlight)
		{
			if (ListData[Position].Colors)
				ColorAttr = FileColor ? ListData[Position].Colors->Color[Pos].FileColor : ListData[Position].Colors->Color[Pos].MarkColor;
			else
				ColorAttr.ForegroundColor = ColorAttr.BackgroundColor = 0; // black on black, default
		}

		if (!Global->Opt->Highlight || (!ColorAttr.ForegroundColor && !ColorAttr.BackgroundColor)) // black on black, default
		{
			static const PaletteColors PalColor[] = {COL_PANELTEXT, COL_PANELSELECTEDTEXT, COL_PANELCURSOR, COL_PANELSELECTEDCURSOR};
			ColorAttr=ColorIndexToColor(PalColor[Pos]);
		}
	}

	return ColorAttr;
}

void FileList::SetShowColor(int Position, bool FileColor) const
{
	SetColor(GetShowColor(Position,FileColor));
}

void FileList::ShowSelectedSize()
{

	if (Global->Opt->ShowPanelStatus)
	{
		SetColor(COL_PANELBOX);
		DrawSeparator(Y2-2);
		for (size_t I=0,ColumnPos=X1+1; I<ViewSettings.PanelColumns.size() - 1; I++)
		{
			if (ViewSettings.PanelColumns[I].width < 0 || (I == ViewSettings.PanelColumns.size() - 2 && ViewSettings.PanelColumns[I+1].width < 0))
				continue;

			ColumnPos += ViewSettings.PanelColumns[I].width;
			GotoXY(static_cast<int>(ColumnPos),Y2-2);

			bool DoubleLine = Global->Opt->DoubleGlobalColumnSeparator && (!((I+1)%ColumnsInGlobal));
			BoxText(BoxSymbols[DoubleLine?BS_B_H1V2:BS_B_H1V1]);
			ColumnPos++;
		}
	}

	if (SelFileCount)
	{
		string strFormStr;
		InsertCommas(SelFileSize,strFormStr);
		LangString strSelStr(__FormatEndSelectedPhrase(SelFileCount));
		strSelStr << strFormStr << SelFileCount;
		TruncStr(strSelStr,X2-X1-1);
		int Length=(int)strSelStr.size();
		SetColor(COL_PANELSELECTEDINFO);
		GotoXY(X1+(X2-X1+1-Length)/2,Y2-2*Global->Opt->ShowPanelStatus);
		Text(strSelStr);
	}
}


void FileList::ShowTotalSize(const OpenPanelInfo &Info)
{
	if (!Global->Opt->ShowPanelTotals && PanelMode==PLUGIN_PANEL && !(Info.Flags & OPIF_REALNAMES))
		return;

	string strFormSize, strFreeSize, strTotalStr;
	int Length;
	InsertCommas(TotalFileSize,strFormSize);

	if (Global->Opt->ShowPanelFree && (PanelMode!=PLUGIN_PANEL || (Info.Flags & OPIF_REALNAMES)))
	{
		if(FreeDiskSize != static_cast<unsigned __int64>(-1))
		{
			InsertCommas(FreeDiskSize,strFreeSize);
		}
		else
		{
			strFreeSize = L"?";
		}
	}

	if (Global->Opt->ShowPanelTotals)
	{
		if (!Global->Opt->ShowPanelFree || strFreeSize.empty())
		{
			strTotalStr = LangString(__FormatEndSelectedPhrase(TotalFileCount)) << strFormSize << TotalFileCount;
		}
		else
		{
			wchar_t DHLine[4]={BoxSymbols[BS_H2],BoxSymbols[BS_H2],BoxSymbols[BS_H2],0};
 			FormatString str;
			str << L" " << strFormSize << L" (" << TotalFileCount << L") " << DHLine << L" " << strFreeSize << L" ";

			if ((int)str.size() > X2-X1-1)
			{
				if(FreeDiskSize != static_cast<unsigned __int64>(-1))
				{
					InsertCommas(FreeDiskSize>>20,strFreeSize);
				}
				else
				{
					strFreeSize = L"?";
				}
				InsertCommas(TotalFileSize>>20,strFormSize);
				str.clear();
				str << L" " << strFormSize << L" " << MSG(MListMb) << L" (" << TotalFileCount << L") " << DHLine << L" " << strFreeSize << L" " << MSG(MListMb) << L" ";
			}
			strTotalStr = str;
		}
	}
	else
	{
		strTotalStr = LangString(MListFreeSize) << (!strFreeSize.empty()? strFreeSize : L"?");
	}
	SetColor(COL_PANELTOTALINFO);
	/* $ 01.08.2001 VVM
	  + Обрезаем строчку справа, а не слева */
	TruncStrFromEnd(strTotalStr, std::max(0, X2-X1-1));
	Length=(int)strTotalStr.size();
	GotoXY(X1+(X2-X1+1-Length)/2,Y2);
	size_t BoxPos = strTotalStr.find(BoxSymbols[BS_H2]);
	int BoxLength=0;
	if (BoxPos != string::npos)
		for (int I=0; strTotalStr[BoxPos+I] == BoxSymbols[BS_H2]; I++)
			BoxLength++;

	if (BoxPos == string::npos || !BoxLength)
		Text(strTotalStr);
	else
	{
		Global->FS << fmt::MaxWidth(BoxPos)<<strTotalStr;
		SetColor(COL_PANELBOX);
		Global->FS << fmt::MaxWidth(BoxLength)<<strTotalStr.data()+BoxPos;
		SetColor(COL_PANELTOTALINFO);
		Text(strTotalStr.data()+BoxPos+BoxLength);
	}
}

int FileList::ConvertName(const wchar_t *SrcName,string &strDest,int MaxLength,unsigned __int64 RightAlign,int ShowStatus,DWORD FileAttr) const
{
	strDest.reserve(MaxLength);

	int SrcLength=StrLength(SrcName);

	if ((RightAlign & COLUMN_RIGHTALIGNFORCE) || (RightAlign && (SrcLength>MaxLength)))
	{
		if (SrcLength>MaxLength)
		{
			strDest.assign(SrcName + SrcLength - MaxLength, MaxLength);
		}
		else
		{
			strDest.assign(MaxLength - SrcLength, L' ');
			strDest.append(SrcName, SrcLength);
		}
		return SrcLength > MaxLength;
	}

	const wchar_t *DotPtr;

	if (!ShowStatus &&
	        ((!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (ViewSettings.Flags&PVS_ALIGNEXTENSIONS))
	         || ((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (ViewSettings.Flags&PVS_FOLDERALIGNEXTENSIONS)))
	        && SrcLength<=MaxLength &&
	        (DotPtr=wcsrchr(SrcName,L'.')) && DotPtr!=SrcName &&
	        (SrcName[0]!=L'.' || SrcName[2]) && !wcschr(DotPtr+1,L' '))
	{
		int DotLength=StrLength(DotPtr+1);
		int NameLength=DotLength?(int)(DotPtr-SrcName):SrcLength;
		int DotPos = std::max(MaxLength - std::max(DotLength,3), NameLength + 1);

		strDest.assign(SrcName, NameLength);

		if (DotPos>0 && NameLength>0 && SrcName[NameLength-1]==L' ')
			strDest += L'.';

		strDest.resize(DotPos, L' ');
		strDest.append(DotPtr + 1, DotLength);
		strDest.resize(MaxLength, L' ');
	}
	else
	{
		strDest.assign(SrcName, std::min(SrcLength, MaxLength));
		strDest.resize(MaxLength, L' ');
	}

	return SrcLength > MaxLength;
}


void FileList::PrepareViewSettings(int ViewMode, const OpenPanelInfo *PlugInfo)
{
	OpenPanelInfo Info={};

	if (PanelMode==PLUGIN_PANEL)
	{
		if (!PlugInfo)
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		else
			Info=*PlugInfo;
	}

	ViewSettings = Global->Opt->ViewSettings[ViewMode].clone();

	if (PanelMode==PLUGIN_PANEL)
	{
		if (Info.PanelModesArray && ViewMode<static_cast<int>(Info.PanelModesNumber) &&
		        Info.PanelModesArray[ViewMode].ColumnTypes &&
		        Info.PanelModesArray[ViewMode].ColumnWidths)
		{
			TextToViewSettings(Info.PanelModesArray[ViewMode].ColumnTypes, Info.PanelModesArray[ViewMode].ColumnWidths, ViewSettings.PanelColumns);

			if (Info.PanelModesArray[ViewMode].StatusColumnTypes &&
			        Info.PanelModesArray[ViewMode].StatusColumnWidths)
			{
				TextToViewSettings(Info.PanelModesArray[ViewMode].StatusColumnTypes, Info.PanelModesArray[ViewMode].StatusColumnWidths, ViewSettings.StatusColumns);
			}
			else if (Info.PanelModesArray[ViewMode].Flags&PMFLAGS_DETAILEDSTATUS)
			{
				ViewSettings.StatusColumns.resize(4);
				ViewSettings.StatusColumns[0].type = COLUMN_RIGHTALIGN|NAME_COLUMN;
				ViewSettings.StatusColumns[1].type = SIZE_COLUMN;
				ViewSettings.StatusColumns[2].type = DATE_COLUMN;
				ViewSettings.StatusColumns[3].type = TIME_COLUMN;
				ViewSettings.StatusColumns[0].width = 0;
				ViewSettings.StatusColumns[1].width = 8;
				ViewSettings.StatusColumns[2].width = 0;
				ViewSettings.StatusColumns[3].width = 5;
			}
			else
			{
				ViewSettings.StatusColumns.resize(1);
				ViewSettings.StatusColumns[0].type = COLUMN_RIGHTALIGN|NAME_COLUMN;
				ViewSettings.StatusColumns[0].width = 0;
			}

			if (Info.PanelModesArray[ViewMode].Flags&PMFLAGS_FULLSCREEN)
				ViewSettings.Flags|=PVS_FULLSCREEN;
			else
				ViewSettings.Flags&=~PVS_FULLSCREEN;

			if (Info.PanelModesArray[ViewMode].Flags&PMFLAGS_ALIGNEXTENSIONS)
				ViewSettings.Flags|=PVS_ALIGNEXTENSIONS;
			else
				ViewSettings.Flags&=~PVS_ALIGNEXTENSIONS;

			if (!(Info.PanelModesArray[ViewMode].Flags&PMFLAGS_CASECONVERSION))
				ViewSettings.Flags&=~(PVS_FOLDERUPPERCASE|PVS_FILELOWERCASE|PVS_FILEUPPERTOLOWERCASE);
		}
		else
		{
			std::for_each(RANGE(ViewSettings.PanelColumns, i)
			{
				if ((i.type & 0xff) == NAME_COLUMN)
				{
					if (Info.Flags & OPIF_SHOWNAMESONLY)
						i.type |= COLUMN_NAMEONLY;

					if (Info.Flags & OPIF_SHOWRIGHTALIGNNAMES)
						i.type |= COLUMN_RIGHTALIGN;
				}
			});
			if (Info.Flags & OPIF_SHOWPRESERVECASE)
				ViewSettings.Flags&=~(PVS_FOLDERUPPERCASE|PVS_FILELOWERCASE|PVS_FILEUPPERTOLOWERCASE);
		}
	}

	Columns=PreparePanelView(&ViewSettings);
	Height=Y2-Y1-4;

	if (!Global->Opt->ShowColumnTitles)
		Height++;

	if (!Global->Opt->ShowPanelStatus)
		Height+=2;
}


int FileList::PreparePanelView(PanelViewSettings *PanelView)
{
	PrepareColumnWidths(PanelView->StatusColumns, (PanelView->Flags&PVS_FULLSCREEN) != 0, true);
	return PrepareColumnWidths(PanelView->PanelColumns, (PanelView->Flags&PVS_FULLSCREEN) != 0, false);
}


int FileList::PrepareColumnWidths(std::vector<column>& Columns, bool FullScreen, bool StatusLine)
{
	int TotalPercentWidth,TotalPercentCount,ZeroLengthCount,EmptyColumns;
	ZeroLengthCount=EmptyColumns=0;
	int TotalWidth = static_cast<int>(Columns.size()-1);
	TotalPercentCount=TotalPercentWidth=0;

	FOR(auto& i, Columns)
	{
		if (i.width < 0)
		{
			EmptyColumns++;
			continue;
		}

		int ColumnType = i.type & 0xff;

		if (!i.width)
		{
			i.width_type = COUNT_WIDTH; //manage all zero-width columns in same way
			i.width = ColumnTypeWidth[ColumnType];

			if (ColumnType==WDATE_COLUMN || ColumnType==CDATE_COLUMN || ColumnType==ADATE_COLUMN || ColumnType==CHDATE_COLUMN)
			{
				if (i.type & COLUMN_BRIEF)
					i.width -= 3;

				if (i.type & COLUMN_MONTH)
					++i.width;
			}
		}

		if (!i.width)
			ZeroLengthCount++;

		switch (i.width_type)
		{
			case COUNT_WIDTH:
				TotalWidth += i.width;
				break;
			case PERCENT_WIDTH:
				TotalPercentWidth += i.width;
				TotalPercentCount++;
				break;
		}
	}

	TotalWidth-=EmptyColumns;
	int PanelTextWidth=X2-X1-1;

	if (FullScreen)
		PanelTextWidth=ScrX-1;

	int ExtraWidth=PanelTextWidth-TotalWidth;

	if (TotalPercentCount>0)
	{
		int ExtraPercentWidth=(TotalPercentWidth>100 || !ZeroLengthCount)?ExtraWidth:ExtraWidth*TotalPercentWidth/100;
		int TempWidth=0;

		FOR(auto& i, Columns)
		{
			if (!TotalPercentCount)
				break;

			if (i.width_type == PERCENT_WIDTH)
			{
				int PercentWidth = (TotalPercentCount > 1)? (ExtraPercentWidth * i.width / TotalPercentWidth) : (ExtraPercentWidth - TempWidth);

				if (PercentWidth<1)
					PercentWidth=1;

				TempWidth+=PercentWidth;
				i.width = PercentWidth;
				i.width_type = COUNT_WIDTH;
				TotalPercentCount--;
			}
		}
		ExtraWidth-=TempWidth;
	}

	FOR(auto& i, Columns)
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

	while (1)
	{
		int LastColumn = static_cast<int>(Columns.size() - 1);
		TotalWidth=LastColumn-EmptyColumns;

		std::for_each(CONST_RANGE(Columns, i)
		{
			if (i.width > 0)
				TotalWidth += i.width;
		});

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

	ColumnsInGlobal = 1;
	int GlobalColumns=0;

	FOR_CONST_RANGE(ViewSettings.PanelColumns, i)
	{
		int Remainder = ViewSettings.PanelColumns.size() % ColumnsInGlobal;
		GlobalColumns = static_cast<int>(ViewSettings.PanelColumns.size() / ColumnsInGlobal);

		if (!Remainder)
		{
			bool UnEqual = false;
			for (int k = 0; k < GlobalColumns-1 && !UnEqual; k++)
			{
				for (int j = 0; j < ColumnsInGlobal && !UnEqual; j++)
				{
					if ((ViewSettings.PanelColumns[k*ColumnsInGlobal+j].type & 0xFF) !=
					        (ViewSettings.PanelColumns[(k+1)*ColumnsInGlobal+j].type & 0xFF))
						UnEqual = true;
				}
			}

			if (!UnEqual)
				break;
		}

		ColumnsInGlobal++;
	}

	return GlobalColumns;
}


void FileList::HighlightBorder(int Level, int ListPos) const
{
	if (Level == ColumnsInGlobal)
	{
		SetColor(COL_PANELBOX);
	}
	else
	{
		FarColor FileColor = GetShowColor(ListPos, true);
		if (Global->Opt->HighlightColumnSeparator)
		{
			SetColor(FileColor);
		}
		else
		{
			FarColor Color = ColorIndexToColor(COL_PANELBOX);
			Color.BackgroundColor = FileColor.BackgroundColor;
			FileColor.Flags&FCF_BG_4BIT? Color.Flags|=FCF_BG_4BIT : Color.Flags&=~FCF_BG_4BIT;
			SetColor(Color);
		}
	}
}

void FileList::ShowList(int ShowStatus,int StartColumn)
{
	int StatusShown=FALSE;
	int MaxLeftPos=0,MinLeftPos=FALSE;
	size_t ColumnCount=ShowStatus ? ViewSettings.StatusColumns.size() : ViewSettings.PanelColumns.size();
	const auto& Columns = ShowStatus ? ViewSettings.StatusColumns : ViewSettings.PanelColumns;

	for (int I=Y1+1+Global->Opt->ShowColumnTitles,J=CurTopFile; I<Y2-2*Global->Opt->ShowPanelStatus; I++,J++)
	{
		int CurColumn=StartColumn;

		if (ShowStatus)
		{
			SetColor(COL_PANELTEXT);
			GotoXY(X1+1,Y2-1);
		}
		else
		{
			SetShowColor(J);
			GotoXY(X1+1,I);
		}

		int StatusLine=FALSE;
		int Level = 1;

		for (size_t K=0; K<ColumnCount; K++)
		{
			int ListPos=J+CurColumn*Height;

			if (ShowStatus)
			{
				if (CurFile!=ListPos)
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
			int ColumnType=static_cast<int>(Columns[K].type & 0xff);
			int ColumnWidth=Columns[K].width;

			if (ColumnWidth<0)
			{
				if (!ShowStatus && K==ColumnCount-1)
				{
					SetColor(COL_PANELBOX);
					GotoXY(CurX-1,CurY);
					BoxText(CurX-1==X2 ? BoxSymbols[BS_V2]:L' ');
				}

				continue;
			}

			if (ListPos < static_cast<int>(ListData.size()))
			{
				if (!ShowStatus && !StatusShown && CurFile==ListPos && Global->Opt->ShowPanelStatus)
				{
					ShowList(TRUE,CurColumn);
					GotoXY(CurX,CurY);
					StatusShown=TRUE;
					SetShowColor(ListPos);
				}

				if (!ShowStatus)
					SetShowColor(ListPos);

				if (ColumnType>=CUSTOM_COLUMN0 && ColumnType<=CUSTOM_COLUMN9)
				{
					size_t ColumnNumber=ColumnType-CUSTOM_COLUMN0;
					const wchar_t *ColumnData=nullptr;

					if (ColumnNumber<ListData[ListPos].CustomColumnNumber)
						ColumnData=ListData[ListPos].CustomColumnData[ColumnNumber];

					if (!ColumnData)
					{
						ColumnData=ListData[ListPos].strCustomData.data();//L"";
					}

					int CurLeftPos=0;

					if (!ShowStatus && LeftPos>0)
					{
						int Length=StrLength(ColumnData);
						if (Length>ColumnWidth)
						{
							CurLeftPos = std::min(LeftPos, Length-ColumnWidth);
							MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
						}
					}

					Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(ColumnWidth)<<ColumnData+CurLeftPos;
				}
				else
				{
					switch (ColumnType)
					{
						case NAME_COLUMN:
						{
							int Width=ColumnWidth;
							unsigned __int64 ViewFlags=Columns[K].type;

							if ((ViewFlags & COLUMN_MARK) && Width>2)
							{
								Text(ListData[ListPos].Selected?L"\x221A ":L"  ");
								Width-=2;
							}

							if (Global->Opt->Highlight && ListData[ListPos].Colors && ListData[ListPos].Colors->Mark.Char && Width>1)
							{
								Width--;
								OutCharacter[0] = ListData[ListPos].Colors->Mark.Char;
								FarColor OldColor=GetColor();

								if (!ShowStatus)
									SetShowColor(ListPos, false);

								Text(OutCharacter);
								SetColor(OldColor);
							}

							const wchar_t *NamePtr = ShowShortNames && !ListData[ListPos].strShortName.empty() && !ShowStatus ? ListData[ListPos].strShortName.data():ListData[ListPos].strName.data();

							string strNameCopy;
							if (!(ListData[ListPos].FileAttr & FILE_ATTRIBUTE_DIRECTORY) && (ViewFlags & COLUMN_NOEXTENSION))
							{
								const wchar_t *ExtPtr = PointToExt(NamePtr);
								if (ExtPtr)
								{
									strNameCopy.assign(NamePtr, ExtPtr-NamePtr);
									NamePtr = strNameCopy.data();
								}
							}

							const wchar_t *NameCopy = NamePtr;

							if (ViewFlags & COLUMN_NAMEONLY)
							{
								//BUGBUG!!!
								// !!! НЕ УВЕРЕН, но то, что отображается пустое
								// пространство вместо названия - бага
								NamePtr=PointToFolderNameIfFolder(NamePtr);
							}

							int CurLeftPos=0;
							unsigned __int64 RightAlign=(ViewFlags & (COLUMN_RIGHTALIGN|COLUMN_RIGHTALIGNFORCE));
							int LeftBracket=FALSE,RightBracket=FALSE;

							if (!ShowStatus && LeftPos)
							{
								int Length = StrLength(NamePtr);

								if (Length>Width)
								{
									if (LeftPos>0)
									{
										if (!RightAlign)
										{
											CurLeftPos = std::min(LeftPos, Length-Width);
											MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
											NamePtr += CurLeftPos;
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
											LeftBracket=(ViewFlags & COLUMN_RIGHTALIGNFORCE)==COLUMN_RIGHTALIGNFORCE;
										}

										NamePtr += Length+CurRightPos-Width;
										RightAlign=FALSE;

										MinLeftPos = std::min(MinLeftPos, CurRightPos);
									}
								}
							}

							string strName;
							int TooLong=ConvertName(NamePtr, strName, Width, RightAlign,ShowStatus,ListData[ListPos].FileAttr);

							if (CurLeftPos)
								LeftBracket=TRUE;

							if (TooLong)
							{
								if (RightAlign)
									LeftBracket=TRUE;

								if (!RightAlign && StrLength(NamePtr)>Width)
									RightBracket=TRUE;
							}

							if (!ShowStatus)
							{
								if (ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE)
									if (!(ListData[ListPos].FileAttr & FILE_ATTRIBUTE_DIRECTORY) && !IsCaseMixed(NameCopy))
										Lower(strName);

								if ((ViewSettings.Flags&PVS_FOLDERUPPERCASE) && (ListData[ListPos].FileAttr & FILE_ATTRIBUTE_DIRECTORY))
									Upper(strName);

								if ((ViewSettings.Flags&PVS_FILELOWERCASE) && !(ListData[ListPos].FileAttr & FILE_ATTRIBUTE_DIRECTORY))
									Lower(strName);
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

									if (Level == ColumnsInGlobal)
										SetColor(COL_PANELTEXT);
									else
										SetShowColor(J);
								}
							}
						}
						break;
						case EXTENSION_COLUMN:
						{
							const wchar_t *ExtPtr = nullptr;
							if (!(ListData[ListPos].FileAttr & FILE_ATTRIBUTE_DIRECTORY))
							{
								const wchar_t *NamePtr = ShowShortNames && !ListData[ListPos].strShortName.empty() && !ShowStatus ? ListData[ListPos].strShortName.data():ListData[ListPos].strName.data();
								ExtPtr = PointToExt(NamePtr);
							}
							if (ExtPtr && *ExtPtr) ExtPtr++; else ExtPtr = L"";

							unsigned __int64 ViewFlags=Columns[K].type;
							if (ViewFlags&COLUMN_RIGHTALIGN)
								Global->FS << fmt::RightAlign()<<fmt::ExactWidth(ColumnWidth)<<ExtPtr;
							else
								Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(ColumnWidth)<<ExtPtr;

							if (!ShowStatus && StrLength(ExtPtr) > ColumnWidth)
							{
								int NameX=WhereX();

								HighlightBorder(Level, ListPos);

								GotoXY(NameX,CurY);
								Text(closeBracket);
								ShowDivider=FALSE;

								if (Level == ColumnsInGlobal)
									SetColor(COL_PANELTEXT);
								else
									SetShowColor(J);
							}
							break;
						}
						break;
						case SIZE_COLUMN:
						case PACKED_COLUMN:
						case STREAMSSIZE_COLUMN:
						{
							Text(FormatStr_Size(
								ListData[ListPos].FileSize,
								ListData[ListPos].AllocationSize,
								ListData[ListPos].StreamsSize,
								ListData[ListPos].strName,
								ListData[ListPos].FileAttr,
								ListData[ListPos].ShowFolderSize,
								ListData[ListPos].ReparseTag,
								ColumnType,
								Columns[K].type,
								ColumnWidth,
								strCurDir.data()));
							break;
						}

						case DATE_COLUMN:
						case TIME_COLUMN:
						case WDATE_COLUMN:
						case CDATE_COLUMN:
						case ADATE_COLUMN:
						case CHDATE_COLUMN:
						{
							FILETIME *FileTime;

							switch (ColumnType)
							{
								case CDATE_COLUMN:
									FileTime=&ListData[ListPos].CreationTime;
									break;
								case ADATE_COLUMN:
									FileTime=&ListData[ListPos].AccessTime;
									break;
								case CHDATE_COLUMN:
									FileTime=&ListData[ListPos].ChangeTime;
									break;
								case DATE_COLUMN:
								case TIME_COLUMN:
								case WDATE_COLUMN:
								default:
									FileTime=&ListData[ListPos].WriteTime;
									break;
							}

							Global->FS << FormatStr_DateTime(FileTime,ColumnType,Columns[K].type,ColumnWidth);
							break;
						}

						case ATTR_COLUMN:
						{
							Global->FS << FormatStr_Attribute(ListData[ListPos].FileAttr,ColumnWidth);
							break;
						}

						case DIZ_COLUMN:
						{
							int CurLeftPos=0;

							if (!ShowStatus && LeftPos>0)
							{
								int Length=ListData[ListPos].DizText ? StrLength(ListData[ListPos].DizText):0;
								if (Length>ColumnWidth)
								{
									CurLeftPos = std::min(LeftPos, Length-ColumnWidth);
									MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
								}
							}

							string strDizText=ListData[ListPos].DizText ? ListData[ListPos].DizText+CurLeftPos:L"";
							size_t pos = strDizText.find(L'\4');
							if (pos != string::npos)
								strDizText.resize(pos);

							Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(ColumnWidth)<<strDizText;
							break;
						}

						case OWNER_COLUMN:
						{
							const wchar_t* Owner=ListData[ListPos].strOwner.data();

							if (!(Columns[K].type & COLUMN_FULLOWNER) && PanelMode!=PLUGIN_PANEL)
							{
								const wchar_t* SlashPos=FirstSlash(Owner);

								if (SlashPos)
									Owner=SlashPos+1;
							}
							else if(IsSlash(*Owner))
							{
								Owner++;
							}

							int CurLeftPos=0;

							if (!ShowStatus && LeftPos>0)
							{
								int Length=StrLength(Owner);
								if (Length>ColumnWidth)
								{
									CurLeftPos = std::min(LeftPos, Length-ColumnWidth);
									MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
								}
							}

							Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(ColumnWidth)<<Owner+CurLeftPos;
							break;
						}

						case NUMLINK_COLUMN:
						{
							int nlink = ListData[ListPos].NumberOfLinks;
							if (nlink >= 0)
								Global->FS << fmt::ExactWidth(ColumnWidth) << nlink;
							else
								Global->FS << fmt::ExactWidth(ColumnWidth) << L"?";
							break;
						}

						case NUMSTREAMS_COLUMN:
						{
							Global->FS << fmt::ExactWidth(ColumnWidth)<<ListData[ListPos].NumberOfStreams;
							break;
						}

					}
				}
			}
			else
			{
				Global->FS << fmt::MinWidth(ColumnWidth)<<L"";
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
					BoxText(CurX+ColumnWidth==X2 ? BoxSymbols[BS_V2]:L' ');
				else
					BoxText(ShowStatus ? L' ':BoxSymbols[(Global->Opt->DoubleGlobalColumnSeparator && Level == ColumnsInGlobal)?BS_V2:BS_V1]);

				if (!ShowStatus)
					SetColor(COL_PANELTEXT);
			}

			if (!ShowStatus)
			{
				if (Level == ColumnsInGlobal)
				{
					Level = 0;
					CurColumn++;
				}

				Level++;
			}
		}

		if ((!ShowStatus || StatusLine) && WhereX()<X2)
		{
			SetColor(COL_PANELTEXT);
			Global->FS << fmt::MinWidth(X2-WhereX())<<L"";
		}
	}

	if (!ShowStatus && !StatusShown && Global->Opt->ShowPanelStatus)
	{
		SetScreen(X1+1,Y2-1,X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y2-1);
		//Global->FS << fmt::Width(X2-X1-1)<<L"";
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


bool FileList::IsDizDisplayed()
{
	return IsColumnDisplayed(DIZ_COLUMN);
}


bool FileList::IsColumnDisplayed(int Type)
{
	auto is_same_type = [&Type](const column& i) {return static_cast<int>(i.type & 0xff) == Type;};

	return std::any_of(ALL_CONST_RANGE(ViewSettings.PanelColumns), is_same_type) ||
		std::any_of(ALL_CONST_RANGE(ViewSettings.StatusColumns), is_same_type);
}
