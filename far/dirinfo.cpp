/*
dirinfo.cpp

GetDirInfo & GetPluginDirInfo
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
#include "dirinfo.hpp"

// Internal:
#include "keys.hpp"
#include "scantree.hpp"
#include "TPreRedrawFunc.hpp"
#include "ctrlobj.hpp"
#include "filefilter.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "taskbar.hpp"
#include "keyboard.hpp"
#include "flink.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "wakeful.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "plugins.hpp"
#include "mix.hpp"
#include "lang.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "copy_progress.hpp"
#include "global.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

static void PR_DrawGetDirInfoMsg();

struct DirInfoPreRedrawItem : public PreRedrawItem
{
	DirInfoPreRedrawItem():
		PreRedrawItem(PR_DrawGetDirInfoMsg)
	{}

	string_view Title;
	string_view Name;
	unsigned long long Items{};
	unsigned long long Size{};
};

static void DirInfoMsgImpl(string_view const Title, string_view const Name, unsigned long long const Items, unsigned long long const Size)
{
	Message(MSG_LEFTALIGN,
		Title,
		{
			msg(lng::MScanningFolder),
			pad_right(truncate_right(Name, copy_progress::CanvasWidth()), copy_progress::CanvasWidth()),
			L"\1"s,
			copy_progress::FormatCounter(lng::MCopyFilesTotalInfo, lng::MCopyBytesTotalInfo, Items, 0, false, copy_progress::CanvasWidth() - 5),
			copy_progress::FormatCounter(lng::MCopyBytesTotalInfo, lng::MCopyFilesTotalInfo, Size, 0, false, copy_progress::CanvasWidth() - 5),
		},
		{});
}

void DirInfoMsg(string_view const Title, string_view const Name, unsigned long long const Items, unsigned long long const Size)
{
	DirInfoMsgImpl(Title, Name, Items, Size);

	TPreRedrawFunc::instance()([&](DirInfoPreRedrawItem& Item)
	{
		Item.Title = Title;
		Item.Name = Name;
		Item.Items = Items;
		Item.Size = Size;
	});
}

static void PR_DrawGetDirInfoMsg()
{
	TPreRedrawFunc::instance()([](const DirInfoPreRedrawItem& Item)
	{
		DirInfoMsgImpl(Item.Title, Item.Name, Item.Items, Item.Size);
	});
}

int GetDirInfo(string_view const DirName, DirInfoData& Data, FileFilter *Filter, dirinfo_callback const Callback, DWORD Flags)
{
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<DirInfoPreRedrawItem>());
	SCOPED_ACTION(taskbar::indeterminate)(false);
	SCOPED_ACTION(wakeful);

	ScanTree ScTree(false, true, (Flags & GETDIRINFO_SCANSYMLINKDEF? static_cast<DWORD>(-1) : (Flags & GETDIRINFO_SCANSYMLINK)));
	SetCursorType(false, 0);
	/* $ 20.03.2002 DJ
	   для . - покажем имя родительского каталога
	*/
	string_view ShowDirName = DirName;

	const auto strFullDirName = ConvertNameToFull(DirName);
	if (DirName == L"."sv)
	{
		const auto pos = FindLastSlash(strFullDirName);
		if (pos != string::npos)
		{
			ShowDirName = string_view(strFullDirName).substr(pos + 1);
		}
	}

	DWORD SectorsPerCluster=0,BytesPerSector=0,FreeClusters=0,Clusters=0;

	if (GetDiskFreeSpace(GetPathRoot(strFullDirName).c_str(), &SectorsPerCluster, &BytesPerSector, &FreeClusters, &Clusters))
		Data.ClusterSize=SectorsPerCluster*BytesPerSector;

	Data.DirCount=Data.FileCount=0;
	Data.FileSize=Data.AllocationSize=Data.FilesSlack=Data.MFTOverhead=0;
	ScTree.SetFindPath(DirName, L"*"sv);
	std::unordered_set<unsigned long long> FileIds;
	DWORD FileSystemFlags = 0;
	string FileSystemName;
	const auto CheckHardlinks = os::fs::GetVolumeInformation(GetPathRoot(DirName), nullptr, nullptr, nullptr, &FileSystemFlags, &FileSystemName)?
		IsWindows7OrGreater()?
			(FileSystemFlags & FILE_SUPPORTS_HARD_LINKS) != 0 :
			FileSystemName == L"NTFS"sv :
		false;
	string strFullName;
	// Временные хранилища имён каталогов
	string strLastDirName;
	os::fs::find_data FindData;
	while (ScTree.GetNextName(FindData,strFullName))
	{
		// Mantis#0002692
		if (!Global->CtrlObject->Macro.IsExecuting())
		{
			INPUT_RECORD rec;

			switch (PeekInputRecord(&rec))
			{
				case 0:
				case KEY_IDLE:
					break;
				case KEY_NONE:
				case KEY_ALT:
				case KEY_CTRL:
				case KEY_SHIFT:
				case KEY_RALT:
				case KEY_RCTRL:
				case KEY_LWIN: case KEY_RWIN:
					GetInputRecordNoMacroArea(&rec);
					break;
				case KEY_ESC:
				case KEY_BREAK:
					GetInputRecordNoMacroArea(&rec);
					return 0;
				default:

					if (Flags&GETDIRINFO_ENHBREAK)
					{
						return -1;
					}

					GetInputRecordNoMacroArea(&rec);
					break;
			}
		}

		Callback(ShowDirName, Data.DirCount + Data.FileCount, Data.FileSize);

		if (FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// Счётчик каталогов наращиваем только если не включен фильтр,
			// в противном случае это будем делать в подсчёте количества файлов
			if (!(Flags&GETDIRINFO_USEFILTER))
				Data.DirCount++;
			else
			{
				// Если каталог не попадает под фильтр то его надо полностью
				// пропустить - иначе при включенном подсчёте total
				// он учтётся (mantis 551)
				if (!Filter->FileInFilter(FindData, {}, strFullName))
					ScTree.SkipDir();
			}
		}
		else
		{
			/* $ 17.04.2005 KM
			   Проверка попадания файла в условия фильтра
			*/
			if ((Flags&GETDIRINFO_USEFILTER))
			{
				if (!Filter->FileInFilter(FindData, {}, strFullName))
					continue;
			}

			// Наращиваем счётчик каталогов при включенном фильтре только тогда,
			// когда в таком каталоге найден файл, удовлетворяющий условиям
			// фильтра.
			if ((Flags&GETDIRINFO_USEFILTER))
			{
				string_view CurDirName = strFullName;
				CutToSlash(CurDirName); //???

				if (!equal_icase(CurDirName, strLastDirName))
				{
					Data.DirCount++;
					strLastDirName = CurDirName;
				}
			}

			Data.FileCount++;

			Data.FileSize += FindData.FileSize;

			if (!CheckHardlinks || !FindData.FileId || FileIds.emplace(FindData.FileId).second)
			{
				Data.AllocationSize += FindData.AllocationSize;
				if(FindData.AllocationSize > FindData.FileSize)
				{
					if(FindData.AllocationSize >= Data.ClusterSize)
					{
					Data.FilesSlack += FindData.AllocationSize - FindData.FileSize;
					}
					else
					{
						Data.MFTOverhead += FindData.AllocationSize - FindData.FileSize;
					}
				}
			}
		}
	}

	return 1;
}

static int PluginSearchMsgOut;

static void PushPluginDirItem(std::vector<PluginPanelItem>& PluginDirList, const PluginPanelItem *CurPanelItem, string_view const PluginSearchPath, BasicDirInfoData& Data)
{
	const auto MakeCopy = [](string_view const Str)
	{
		auto Buffer = std::make_unique<wchar_t[]>(Str.size() + 1);
		*copy_string(Str, Buffer.get()) = {};
		return Buffer.release();
	};

	auto NewItem = *CurPanelItem;

	NewItem.FileName = MakeCopy(path::join(PluginSearchPath, CurPanelItem->FileName));

	NewItem.AlternateFileName = {};

	if (CurPanelItem->Description)
		NewItem.Description = MakeCopy(CurPanelItem->Description);

	if (CurPanelItem->Owner)
		NewItem.Owner = MakeCopy(CurPanelItem->Owner);

	if (NewItem.CustomColumnNumber>0)
	{
		auto CustomColumnData = std::make_unique<wchar_t*[]>(NewItem.CustomColumnNumber);
		for (size_t ii = 0; ii < NewItem.CustomColumnNumber; ii++)
		{
			if (CurPanelItem->CustomColumnData[ii])
				CustomColumnData[ii] = MakeCopy(CurPanelItem->CustomColumnData[ii]);
		}
		NewItem.CustomColumnData = CustomColumnData.release();
	}

	// UserData is not used in PluginDirList
	NewItem.UserData.Data=nullptr;
	NewItem.UserData.FreeData=nullptr;


	if (NewItem.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		++Data.DirCount;
	}
	else
	{
		++Data.FileCount;
		Data.FileSize += NewItem.FileSize;
		Data.AllocationSize += NewItem.AllocationSize? NewItem.AllocationSize : NewItem.FileSize;
	}

	PluginDirList.emplace_back(NewItem);
}

static void ScanPluginDir(plugin_panel* hDirListPlugin, OPERATION_MODES OpMode, string_view const BaseDir, string_view const PluginSearchPath, std::vector<PluginPanelItem>& PluginDirList, bool& StopSearch, BasicDirInfoData& Data, dirinfo_callback const Callback)
{
	Callback(BaseDir, Data.DirCount + Data.FileCount, Data.FileSize);

	span<PluginPanelItem> PanelData;

	if (CheckForEscSilent())
	{
		if (ConfirmAbortOp())
			StopSearch = true;
	}

	if (StopSearch || !Global->CtrlObject->Plugins->GetFindData(hDirListPlugin, PanelData, OPM_FIND | OpMode))
		return;

	SCOPE_EXIT{ Global->CtrlObject->Plugins->FreeFindData(hDirListPlugin, PanelData, true); };

	PluginDirList.reserve(PluginDirList.size() + PanelData.size());

	for (const auto& i: PanelData)
	{
		if (StopSearch)
			return;

		if (!(i.FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			PushPluginDirItem(PluginDirList, &i, PluginSearchPath, Data);
	}

	for (const auto& i: PanelData)
	{
		if (StopSearch)
			return;

		if (!(i.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) || IsParentDirectory(i))
			continue;

		/* $ 30.11.2001 DJ
				используем общую функцию для копирования FindData (не забываем
				обработать PPIF_USERDATA)
		*/
		PushPluginDirItem(PluginDirList, &i, PluginSearchPath, Data);
		const auto FileNameCopy = i.FileName;

		if (Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin, FileNameCopy, OPM_FIND | OpMode, &i.UserData))
		{
			ScanPluginDir(hDirListPlugin, OpMode, BaseDir, path::join(PluginSearchPath, i.FileName), PluginDirList, StopSearch, Data, Callback);

			if (!Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin, L".."s, OPM_FIND | OpMode))
				return;
		}
	}
}

static bool GetPluginDirListImpl(Plugin* PluginNumber, HANDLE hPlugin, string_view const Dir, const UserDataItem* const UserData, std::vector<PluginPanelItem>& Items, BasicDirInfoData& Data, dirinfo_callback const Callback)
{
	Items.clear();

	std::unique_ptr<plugin_panel> DirListPlugin;
	plugin_panel* hDirListPlugin;

	OPERATION_MODES OpMode=0;
	if (Global->CtrlObject->Cp()->PassivePanel()->GetType() == panel_type::QVIEW_PANEL || Global->CtrlObject->Cp()->ActivePanel()->GetType() == panel_type::QVIEW_PANEL)
		OpMode|=OPM_QUICKVIEW;

	// А не хочет ли плагин посмотреть на текущую панель?
	if (!hPlugin || hPlugin==PANEL_ACTIVE || hPlugin==PANEL_PASSIVE)
	{
		/* $ 30.11.2001 DJ
			А плагиновая ли это панель?
		*/
		const auto Handle = ((!hPlugin || hPlugin == PANEL_ACTIVE) ? Global->CtrlObject->Cp()->ActivePanel() : Global->CtrlObject->Cp()->PassivePanel())->GetPluginHandle();

		if (!Handle)
			return false;

		hDirListPlugin = Handle;
	}
	else
	{
		DirListPlugin = std::make_unique<plugin_panel>(PluginNumber, hPlugin);
		hDirListPlugin = DirListPlugin.get();
	}

	bool StopSearch = false;

	{
		SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<DirInfoPreRedrawItem>());
		{
			const auto strDirName = fit_to_center(truncate_left(Dir, 30), 30);
			SetCursorType(false, 0);
			PluginSearchMsgOut=FALSE;

			OpenPanelInfo Info;
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hDirListPlugin,&Info);
			const string strPrevDir = NullToEmpty(Info.CurDir);

			if (Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin, string(Dir), OPM_SILENT | OpMode, UserData))
			{
				ScanPluginDir(hDirListPlugin, OpMode, Dir, Dir, Items, StopSearch, Data, Callback);

				Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin, L".."s, OPM_SILENT | OpMode);

				OpenPanelInfo NewInfo;
				Global->CtrlObject->Plugins->GetOpenPanelInfo(hDirListPlugin,&NewInfo);

				if (!equal_icase(strPrevDir, NullToEmpty(NewInfo.CurDir)))
				{
					span<PluginPanelItem> PanelData;

					if (Global->CtrlObject->Plugins->GetFindData(hDirListPlugin, PanelData, OPM_SILENT | OpMode))
					{
						Global->CtrlObject->Plugins->FreeFindData(hDirListPlugin, PanelData, true);
					}

					Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,strPrevDir,OPM_SILENT|OpMode,&Info.UserData);
				}
			}
		}
	}
	return !StopSearch;
}

bool GetPluginDirList(Plugin* PluginNumber, HANDLE hPlugin, string_view const Dir, const UserDataItem* const UserData, std::vector<PluginPanelItem>& Items, dirinfo_callback const Callback)
{
	BasicDirInfoData Data{};
	return GetPluginDirListImpl(PluginNumber, hPlugin, Dir, UserData, Items, Data, Callback);
}

void FreePluginDirList(HANDLE hPlugin, std::vector<PluginPanelItem>& Items)
{
	for (const auto& i: Items)
	{
		FreePluginPanelItemData(i);
		FreePluginPanelItemUserData(hPlugin, i.UserData);
	}

	Items.clear();
}

bool GetPluginDirInfo(
	plugin_panel const* const hPlugin,
	string_view const DirName,
	UserDataItem const* const UserData,
	BasicDirInfoData& Data,
	dirinfo_callback const Callback
)
{
	Data = {};

	std::vector<PluginPanelItem> PanelItems;
	// Must be cleared unconditionally: GetPluginDirList can fill it partially and return false
	SCOPE_EXIT{ FreePluginDirList(hPlugin->panel(), PanelItems); };

	return GetPluginDirListImpl(hPlugin->plugin(), hPlugin->panel(), DirName, UserData, PanelItems, Data, Callback); //intptr_t - BUGBUG
}
