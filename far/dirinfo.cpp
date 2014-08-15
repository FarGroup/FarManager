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
#include "plugapi.hpp"
#include "keys.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "RefreshFrameManager.hpp"
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
	string strSize;
	FileSizeToStr(strSize, Size, 8, COLUMN_FLOATSIZE|COLUMN_COMMAS);
	RemoveLeadingSpaces(strSize);
	Message(0,0,Title,MSG(MScanningFolder),Name,strSize.data());
	if (!PreRedrawStack().empty())
	{
		auto item = dynamic_cast<DirInfoPreRedrawItem*>(PreRedrawStack().top());
		item->Title = Title;
		item->Name = Name;
		item->Size = Size;
	}
}

static void PR_DrawGetDirInfoMsg()
{
	if (!PreRedrawStack().empty())
	{
		auto item = dynamic_cast<const DirInfoPreRedrawItem*>(PreRedrawStack().top());
		DrawGetDirInfoMsg(item->Title.data(), item->Name.data(), item->Size);
	}
}

int GetDirInfo(const wchar_t *Title, const string& DirName, DirInfoData& Data, clock_t MsgWaitTime, FileFilter *Filter, DWORD Flags)
{
	string strFullDirName, strDriveRoot;
	string strFullName, strCurDirName, strLastDirName;
	ConvertNameToFull(DirName, strFullDirName);
	SaveScreen SaveScr;
	SCOPED_ACTION(UndoGlobalSaveScrPtr)(&SaveScr);
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<DirInfoPreRedrawItem>());
	SCOPED_ACTION(IndeterminateTaskBar)(MsgWaitTime != -1);
	SCOPED_ACTION(wakeful);
	ScanTree ScTree(false, true, (Flags & GETDIRINFO_SCANSYMLINKDEF? (DWORD)-1 : (Flags & GETDIRINFO_SCANSYMLINK)));
	api::FAR_FIND_DATA FindData;
	clock_t StartTime=clock();
	SetCursorType(false, 0);
	GetPathRoot(strFullDirName,strDriveRoot);
	/* $ 20.03.2002 DJ
	   для . - покажем имя родительского каталога
	*/
	const wchar_t *ShowDirName = DirName.data();

	if (DirName.size() ==1 && DirName[0] == L'.')
	{
		const wchar_t *p = LastSlash(strFullDirName.data());

		if (p)
			ShowDirName = p + 1;
	}

	ConsoleTitle OldTitle;
	RefreshFrameManager frref(ScrX,ScrY,MsgWaitTime,Flags&GETDIRINFO_DONTREDRAWFRAME);
	DWORD SectorsPerCluster=0,BytesPerSector=0,FreeClusters=0,Clusters=0;

	if (GetDiskFreeSpace(strDriveRoot.data(),&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters))
		Data.ClusterSize=SectorsPerCluster*BytesPerSector;

	// Временные хранилища имён каталогов
	strLastDirName.clear();
	strCurDirName.clear();
	Data.DirCount=Data.FileCount=0;
	Data.FileSize=Data.AllocationSize=Data.FilesSlack=Data.MFTOverhead=0;
	ScTree.SetFindPath(DirName,L"*");
	std::unordered_set<UINT64> FileIds;
	bool CheckHardlinks = false;
	DWORD FileSystemFlags = 0;
	string FileSystemName;
	string Root;
	GetPathRoot(DirName, Root);
	if(api::GetVolumeInformation(Root, nullptr, nullptr, nullptr, &FileSystemFlags, &FileSystemName))
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
	while (ScTree.GetNextName(&FindData,strFullName))
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

		if (MsgWaitTime!=-1 && CurTime-StartTime > MsgWaitTime)
		{
			StartTime=CurTime;
			MsgWaitTime=500;
			OldTitle << MSG(MScanningFolder) << L" " << ShowDirName << fmt::Flush(); // покажем заголовок консоли
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
			   Проверка попадания файла в условия фильра
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

static int StopSearch;
static PluginHandle* hDirListPlugin;
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
		auto item = dynamic_cast<PluginDirInfoPreRedrawItem*>(PreRedrawStack().top());
		item->Name = Name;
		item->Flags = Flags;
	}
}

static void PR_FarGetPluginDirListMsg()
{
	if (!PreRedrawStack().empty())
	{
		auto item = dynamic_cast<const PluginDirInfoPreRedrawItem*>(PreRedrawStack().top());
		FarGetPluginDirListMsg(item->Name, item->Flags);
	}
}

static void PushPluginDirItem(std::vector<PluginPanelItem>& PluginDirList, const PluginPanelItem *CurPanelItem, const string& strPluginSearchPath)
{
	string strFullName;
	strFullName = strPluginSearchPath;
	strFullName += CurPanelItem->FileName;

	std::replace(ALL_RANGE(strFullName), L'\x1', L'\\');

	PluginDirList.emplace_back(*CurPanelItem);

	PluginDirList.back().FileName = DuplicateString(strFullName.data());
	PluginDirList.back().AlternateFileName=nullptr;

	// UserData is not used in PluginDirList
	PluginDirList.back().UserData.Data=nullptr;
	PluginDirList.back().UserData.FreeData=nullptr;
}

static void ScanPluginDir(OPERATION_MODES OpMode,string& strPluginSearchPath, std::vector<PluginPanelItem>& PluginDirList)
{
	PluginPanelItem *PanelData=nullptr;
	size_t ItemCount=0;
	int AbortOp=FALSE;
	string strDirName;
	strDirName = strPluginSearchPath;

	std::replace(ALL_RANGE(strDirName), L'\x1', L'\\');
	DeleteEndSlash(strDirName);

	TruncStr(strDirName,30);
	CenterStr(strDirName,strDirName,30);

	if (CheckForEscSilent())
	{
		if (Global->Opt->Confirm.Esc) // Будет выдаваться диалог?
			AbortOp=TRUE;

		if (ConfirmAbortOp())
			StopSearch=TRUE;
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
				ScanPluginDir(OpMode,strPluginSearchPath, PluginDirList);
				size_t pos = strPluginSearchPath.rfind(L'\x1');
				if (pos != string::npos)
					strPluginSearchPath.resize(pos);

				if ((pos = strPluginSearchPath.rfind(L'\x1'))!= string::npos)
					strPluginSearchPath.resize(pos+1);
				else
					strPluginSearchPath.clear();

				if (!Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,L"..",OPM_FIND|OpMode))
				{
					StopSearch=TRUE;
					break;
				}
			}
		}
	}

	Global->CtrlObject->Plugins->FreeFindData(hDirListPlugin,PanelData,ItemCount,true);
}

int GetPluginDirList(Plugin* PluginNumber, HANDLE hPlugin, const string& Dir, PluginPanelItem **pPanelItem, size_t *pItemsNumber)
{
	if (Dir == L"." || TestParentFolderName(Dir))
		return FALSE;

	static PluginHandle DirListPlugin;
	OPERATION_MODES OpMode=0;
	if (Global->CtrlObject->Cp()->PassivePanel()->GetType()==QVIEW_PANEL || Global->CtrlObject->Cp()->ActivePanel()->GetType()==QVIEW_PANEL)
		OpMode|=OPM_QUICKVIEW;

	// А не хочет ли плагин посмотреть на текущую панель?
	if (!hPlugin || hPlugin==PANEL_ACTIVE || hPlugin==PANEL_PASSIVE)
	{
		/* $ 30.11.2001 DJ
			А плагиновая ли это панель?
		*/
		auto Handle = ((!hPlugin || hPlugin==PANEL_ACTIVE)?Global->CtrlObject->Cp()->ActivePanel():Global->CtrlObject->Cp()->PassivePanel())->GetPluginHandle();

		if (!Handle)
			return FALSE;

		DirListPlugin = *Handle;
	}
	else
	{
		DirListPlugin.pPlugin = PluginNumber;
		DirListPlugin.hPlugin=hPlugin;
	}


	{
		SaveScreen SaveScr;
		SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<PluginDirInfoPreRedrawItem>());
		{
			string strDirName;
			strDirName = Dir;
			TruncStr(strDirName,30);
			CenterStr(strDirName,strDirName,30);
			SetCursorType(false, 0);
			FarGetPluginDirListMsg(strDirName,0);
			PluginSearchMsgOut=FALSE;
			hDirListPlugin = &DirListPlugin;
			StopSearch=FALSE;

			auto PluginDirList = new std::vector<PluginPanelItem>;
			// first item is reserved for internal needs
			PluginDirList->emplace_back(VALUE_TYPE(*PluginDirList)());
			PluginDirList->front().Reserved[0] = reinterpret_cast<intptr_t>(PluginDirList);

			*pItemsNumber = 0;
			*pPanelItem = nullptr;
			OpenPanelInfo Info;
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hDirListPlugin,&Info);
			string strPrevDir = NullToEmpty(Info.CurDir);

			UserDataItem UserData = {};  // How to find the value of a variable?

			if (Global->CtrlObject->Plugins->SetDirectory(hDirListPlugin,Dir,OPM_SILENT|OpMode,&UserData))
			{
				string strPluginSearchPath = Dir;
				strPluginSearchPath += L"\x1";
				ScanPluginDir(OpMode,strPluginSearchPath, *PluginDirList);


				*pPanelItem = PluginDirList->data() + 1;
				*pItemsNumber = PluginDirList->size() - 1;

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

void FreePluginDirList(HANDLE hPlugin, const PluginPanelItem *PanelItem)
{
	if (!PanelItem)
		return;

	auto PluginDirList = reinterpret_cast<std::vector<PluginPanelItem>*>((PanelItem-1)->Reserved[0]);

	// first item is reserved for internal needs
	std::for_each(PluginDirList->begin() + 1, PluginDirList->end(), LAMBDA_PREDICATE(*PluginDirList, i)
	{
		if(i.UserData.FreeData)
		{
			FarPanelItemFreeInfo info={sizeof(FarPanelItemFreeInfo),hPlugin};
			i.UserData.FreeData(i.UserData.Data,&info);
		}
		FreePluginPanelItem(i);
	});

	delete PluginDirList;
}

int GetPluginDirInfo(PluginHandle* ph,const string& DirName,unsigned long &DirCount,
                     unsigned long &FileCount,unsigned __int64 &FileSize,
                     unsigned __int64 &CompressedFileSize)
{
	PluginPanelItem *PanelItem=nullptr;
	size_t ItemsNumber=0;
	int ExitCode;
	DirCount=FileCount=0;
	FileSize=CompressedFileSize=0;

	if ((ExitCode=GetPluginDirList(ph->pPlugin, ph->hPlugin, DirName, &PanelItem,&ItemsNumber))==TRUE) //intptr_t - BUGBUG
	{
		std::for_each(PanelItem, PanelItem + ItemsNumber, [&](PluginPanelItem& i)
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

	if (PanelItem)
	{
		FreePluginDirList(ph->hPlugin, PanelItem);
	}

	return ExitCode;
}
