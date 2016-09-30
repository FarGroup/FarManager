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

#include "headers.hpp"
#pragma hdrstop

#include "dirinfo.hpp"
#include "keys.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "refreshwindowmanager.hpp"
#include "TPreRedrawFunc.hpp"
#include "ctrlobj.hpp"
#include "filefilter.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "TaskBar.hpp"
#include "constitle.hpp"
#include "keyboard.hpp"
#include "flink.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "wakeful.hpp"
#include "config.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "plugins.hpp"
#include "mix.hpp"
#include "language.hpp"
#include "colormix.hpp"

static void PR_DrawGetDirInfoMsg();

struct DirInfoPreRedrawItem : public PreRedrawItem
{
	DirInfoPreRedrawItem():
		PreRedrawItem(PR_DrawGetDirInfoMsg),
		Size()
	{}

	string Title;
	string Name;
	UINT64 Size;
};

static void DrawGetDirInfoMsg(const wchar_t *Title,const wchar_t *Name, UINT64 Size)
{
	auto strSize = FileSizeToStr(Size, 8, COLUMN_FLOATSIZE|COLUMN_COMMAS);
	RemoveLeadingSpaces(strSize);
	Message(0,0,Title,MSG(MScanningFolder),Name,strSize.data());
	if (!PreRedrawStack().empty())
	{
		const auto item = dynamic_cast<DirInfoPreRedrawItem*>(PreRedrawStack().top());
		assert(item);
		if (item)
		{
			item->Title = Title;
			item->Name = Name;
			item->Size = Size;
		}
	}
}

static void PR_DrawGetDirInfoMsg()
{
	if (!PreRedrawStack().empty())
	{
		const auto item = dynamic_cast<const DirInfoPreRedrawItem*>(PreRedrawStack().top());
		assert(item);
		if (item)
		{
			DrawGetDirInfoMsg(item->Title.data(), item->Name.data(), item->Size);
		}
	}
}

int GetDirInfo(const wchar_t *Title, const string& DirName, DirInfoData& Data, getdirinfo_message_delay MessageDelay, FileFilter *Filter, DWORD Flags)
{
	SaveScreen SaveScr;
	SCOPED_ACTION(UndoGlobalSaveScrPtr)(&SaveScr);
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<DirInfoPreRedrawItem>());
	SCOPED_ACTION(IndeterminateTaskBar)(MessageDelay != getdirinfo_infinite_delay);
	SCOPED_ACTION(wakeful);
	ScanTree ScTree(false, true, (Flags & GETDIRINFO_SCANSYMLINKDEF? (DWORD)-1 : (Flags & GETDIRINFO_SCANSYMLINK)));
	clock_t StartTime=clock();
	SetCursorType(false, 0);
	/* $ 20.03.2002 DJ
	   для . - покажем имя родительского каталога
	*/
	const wchar_t *ShowDirName = DirName.data();

	const auto strFullDirName = ConvertNameToFull(DirName);
	if (DirName.size() ==1 && DirName[0] == L'.')
	{
		const auto pos = FindLastSlash(strFullDirName);
		if (pos != string::npos)
		{
			ShowDirName = strFullDirName.data() + pos + 1;
		}
	}

	std::unique_ptr<RefreshWindowManager> frref;
	if (!(Flags & GETDIRINFO_NOREDRAW))
		frref = std::make_unique<RefreshWindowManager>(ScrX, ScrY, MessageDelay != getdirinfo_infinite_delay);

	DWORD SectorsPerCluster=0,BytesPerSector=0,FreeClusters=0,Clusters=0;

	if (GetDiskFreeSpace(GetPathRoot(strFullDirName).data(),&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters))
		Data.ClusterSize=SectorsPerCluster*BytesPerSector;

	Data.DirCount=Data.FileCount=0;
	Data.FileSize=Data.AllocationSize=Data.FilesSlack=Data.MFTOverhead=0;
	ScTree.SetFindPath(DirName,L"*");
	std::unordered_set<UINT64> FileIds;
	bool CheckHardlinks = false;
	DWORD FileSystemFlags = 0;
	string FileSystemName;
	if(os::GetVolumeInformation(GetPathRoot(DirName), nullptr, nullptr, nullptr, &FileSystemFlags, &FileSystemName))
	{
		if (!IsWindows7OrGreater())
		{
			CheckHardlinks = FileSystemName == L"NTFS";
		}
		else
		{
			CheckHardlinks = (FileSystemFlags&FILE_SUPPORTS_HARD_LINKS) != 0;
		}
	}
	string strFullName;
	// Временные хранилища имён каталогов
	string strCurDirName, strLastDirName;
	os::FAR_FIND_DATA FindData;
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

		clock_t CurTime=clock();

		if (MessageDelay != getdirinfo_infinite_delay && CurTime - StartTime > MessageDelay)
		{
			StartTime=CurTime;
			MessageDelay = getdirinfo_default_delay;
			ConsoleTitle::SetFarTitle(MSG(MScanningFolder) + L" "s + ShowDirName);
			SetCursorType(false, 0);
			DrawGetDirInfoMsg(Title,ShowDirName, Data.FileSize);
		}

		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
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
				if (!Filter->FileInFilter(FindData, nullptr, &strFullName))
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
				if (!Filter->FileInFilter(FindData,nullptr, &strFullName))
					continue;
			}

			// Наращиваем счётчик каталогов при включенном фильтре только тогда,
			// когда в таком каталоге найден файл, удовлетворяющий условиям
			// фильтра.
			if ((Flags&GETDIRINFO_USEFILTER))
			{
				strCurDirName = strFullName;
				CutToSlash(strCurDirName); //???

				if (StrCmpI(strCurDirName, strLastDirName))
				{
					Data.DirCount++;
					strLastDirName = strCurDirName;
				}
			}

			Data.FileCount++;

			Data.FileSize += FindData.nFileSize;

			if (!CheckHardlinks || !FindData.FileId || FileIds.emplace(FindData.FileId).second)
			{
				Data.AllocationSize += FindData.nAllocationSize;
				if(FindData.nAllocationSize > FindData.nFileSize)
				{
					if(FindData.nAllocationSize >= Data.ClusterSize)
					{
						Data.FilesSlack += FindData.nAllocationSize - FindData.nFileSize;
					}
					else
					{
						Data.MFTOverhead += FindData.nAllocationSize - FindData.nFileSize;
					}
				}
			}
		}
	}

	return 1;
}

static int PluginSearchMsgOut;

static void PR_FarGetPluginDirListMsg();

struct PluginDirInfoPreRedrawItem : public PreRedrawItem
{
	PluginDirInfoPreRedrawItem():
		PreRedrawItem(PR_FarGetPluginDirListMsg),
		Flags()
	{}

	string Name;
	DWORD Flags;
};

static void FarGetPluginDirListMsg(const string& Name,DWORD Flags)
{
	Message(Flags,0,L"",MSG(MPreparingList),Name.data());
	if (!PreRedrawStack().empty())
	{
		const auto item = dynamic_cast<PluginDirInfoPreRedrawItem*>(PreRedrawStack().top());
		assert(item);
		if (item)
		{
			item->Name = Name;
			item->Flags = Flags;
		}
	}
}

static void PR_FarGetPluginDirListMsg()
{
	if (!PreRedrawStack().empty())
	{
		const auto item = dynamic_cast<const PluginDirInfoPreRedrawItem*>(PreRedrawStack().top());
		assert(item);
		if (item)
		{
			FarGetPluginDirListMsg(item->Name, item->Flags);
		}
	}
}

static void PushPluginDirItem(std::vector<PluginPanelItem>& PluginDirList, const PluginPanelItem *CurPanelItem, const string& strPluginSearchPath)
{
	auto strFullName = strPluginSearchPath + CurPanelItem->FileName;
	std::replace(ALL_RANGE(strFullName), L'\x1', L'\\');

	PluginDirList.emplace_back(*CurPanelItem);

	PluginDirList.back().FileName = DuplicateString(strFullName.data());
	PluginDirList.back().AlternateFileName=nullptr;
	PluginDirList.back().Description = DuplicateString(PluginDirList.back().Description);
	PluginDirList.back().Owner = DuplicateString(PluginDirList.back().Owner);
	if (PluginDirList.back().CustomColumnNumber>0)
	{
		auto CustomColumnData = new wchar_t*[PluginDirList.back().CustomColumnNumber];
		for (size_t ii = 0; ii<PluginDirList.back().CustomColumnNumber; ii++)
			CustomColumnData[ii] = DuplicateString(PluginDirList.back().CustomColumnData[ii]);
		PluginDirList.back().CustomColumnData = CustomColumnData;
	}

	// UserData is not used in PluginDirList
	PluginDirList.back().UserData.Data=nullptr;
	PluginDirList.back().UserData.FreeData=nullptr;
}

static void ScanPluginDir(plugin_panel* hDirListPlugin, OPERATION_MODES OpMode,string& strPluginSearchPath, std::vector<PluginPanelItem>& PluginDirList, bool& StopSearch)
{
	PluginPanelItem *PanelData=nullptr;
	size_t ItemCount=0;
	int AbortOp=FALSE;
	auto strDirName = strPluginSearchPath;
	std::replace(ALL_RANGE(strDirName), L'\x1', L'\\');
	DeleteEndSlash(strDirName);
	TruncStr(strDirName,30);
	CenterStr(strDirName,strDirName,30);

	if (CheckForEscSilent())
	{
		if (Global->Opt->Confirm.Esc) // Будет выдаваться диалог?
			AbortOp=TRUE;

		if (ConfirmAbortOp())
			StopSearch = true;
	}

	FarGetPluginDirListMsg(strDirName,AbortOp?0:MSG_KEEPBACKGROUND);

	if (StopSearch || !Global->CtrlObject->Plugins->GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND|OpMode))
		return;

	PluginDirList.reserve(PluginDirList.size() + ItemCount);

	for (size_t i=0; i<ItemCount && !StopSearch; i++)
	{
		PluginPanelItem *CurPanelItem=PanelData+i;

		if (!(CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			PushPluginDirItem(PluginDirList, CurPanelItem, strPluginSearchPath);
	}

	for (size_t i=0; i<ItemCount && !StopSearch; i++)
	{
		PluginPanelItem *CurPanelItem=PanelData+i;

		if ((CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
		        StrCmp(CurPanelItem->FileName,L".") &&
		        !TestParentFolderName(CurPanelItem->FileName))
		{
			/* $ 30.11.2001 DJ
					используем общую функцию для копирования FindData (не забываем
					обработать PPIF_USERDATA)
			*/
			PushPluginDirItem(PluginDirList, CurPanelItem, strPluginSearchPath);
			string strFileName = CurPanelItem->FileName;

			if (Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,strFileName,OPM_FIND|OpMode,&CurPanelItem->UserData))
			{
				strPluginSearchPath += CurPanelItem->FileName;
				strPluginSearchPath += L"\x1";
				ScanPluginDir(hDirListPlugin, OpMode, strPluginSearchPath, PluginDirList, StopSearch);
				size_t pos = strPluginSearchPath.rfind(L'\x1');
				if (pos != string::npos)
					strPluginSearchPath.resize(pos);

				if ((pos = strPluginSearchPath.rfind(L'\x1'))!= string::npos)
					strPluginSearchPath.resize(pos+1);
				else
					strPluginSearchPath.clear();

				if (!Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,L"..",OPM_FIND|OpMode))
				{
					StopSearch = true;
					break;
				}
			}
		}
	}

	Global->CtrlObject->Plugins->FreeFindData(hDirListPlugin,PanelData,ItemCount,true);
}

int GetPluginDirList(Plugin* PluginNumber, HANDLE hPlugin, const string& Dir, std::vector<PluginPanelItem>& Items)
{
	Items.clear();

	if (Dir == L"." || TestParentFolderName(Dir))
		return FALSE;

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
			return FALSE;

		hDirListPlugin = Handle;
	}
	else
	{
		DirListPlugin = std::make_unique<plugin_panel>(PluginNumber, hPlugin);
		hDirListPlugin = DirListPlugin.get();
	}

	bool StopSearch = false;

	{
		SCOPED_ACTION(SaveScreen);
		SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<PluginDirInfoPreRedrawItem>());
		{
			auto strDirName = Dir;
			TruncStr(strDirName,30);
			CenterStr(strDirName,strDirName,30);
			SetCursorType(false, 0);
			FarGetPluginDirListMsg(strDirName,0);
			PluginSearchMsgOut=FALSE;

			OpenPanelInfo Info;
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hDirListPlugin,&Info);
			string strPrevDir = NullToEmpty(Info.CurDir);

			UserDataItem UserData = {};  // How to find the value of a variable?

			if (Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,Dir,OPM_SILENT|OpMode,&UserData))
			{
				string strPluginSearchPath = Dir;
				strPluginSearchPath += L"\x1";
				ScanPluginDir(hDirListPlugin, OpMode,strPluginSearchPath, Items, StopSearch);

				Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,L"..",OPM_SILENT|OpMode);
				OpenPanelInfo NewInfo;
				Global->CtrlObject->Plugins->GetOpenPanelInfo(hDirListPlugin,&NewInfo);

				if (StrCmpI(strPrevDir.data(), NullToEmpty(NewInfo.CurDir)))
				{
					PluginPanelItem *PanelData=nullptr;
					size_t ItemCount=0;

					if (Global->CtrlObject->Plugins->GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_SILENT|OpMode))
					{
						Global->CtrlObject->Plugins->FreeFindData(hDirListPlugin,PanelData,ItemCount,true);
					}

					Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,strPrevDir,OPM_SILENT|OpMode,&Info.UserData);
				}
			}
		}
	}
	return !StopSearch;
}

void FreePluginDirList(HANDLE hPlugin, std::vector<PluginPanelItem>& Items)
{
	std::for_each(RANGE(Items, i)
	{
		if(i.UserData.FreeData)
		{
			FarPanelItemFreeInfo info={sizeof(FarPanelItemFreeInfo),hPlugin};
			i.UserData.FreeData(i.UserData.Data,&info);
		}
		FreePluginPanelItem(i);
		delete[] i.Description;
		delete[] i.Owner;
		DeleteRawArray(i.CustomColumnData, i.CustomColumnNumber);
	});

	Items.clear();
}

int GetPluginDirInfo(plugin_panel* ph,const string& DirName,unsigned long& DirCount,
                     unsigned long& FileCount,unsigned long long& FileSize,
                     unsigned long long& CompressedFileSize)
{
	int ExitCode;
	DirCount=FileCount=0;
	FileSize=CompressedFileSize=0;

	std::vector<PluginPanelItem> PanelItems;
	if ((ExitCode = GetPluginDirList(ph->plugin(), ph->panel(), DirName, PanelItems)) == TRUE) //intptr_t - BUGBUG
	{
		std::for_each(ALL_CONST_RANGE(PanelItems), [&](const PluginPanelItem& i)
		{
			if (i.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				DirCount++;
			}
			else
			{
				FileCount++;
				FileSize += i.FileSize;
				CompressedFileSize += i.AllocationSize? i.AllocationSize : i.FileSize;
			}
		});
	}

	FreePluginDirList(ph->panel(), PanelItems);

	return ExitCode;
}
