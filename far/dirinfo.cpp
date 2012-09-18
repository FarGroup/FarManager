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

static void DrawGetDirInfoMsg(const wchar_t *Title,const wchar_t *Name,const UINT64* Size)
{
	string strSize;
	FileSizeToStr(strSize,*Size,8,COLUMN_FLOATSIZE|COLUMN_COMMAS);
	RemoveLeadingSpaces(strSize);
	Message(0,0,Title,MSG(MScanningFolder),Name,strSize);
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Param1=Title;
	preRedrawItem.Param.Param2=Name;
	preRedrawItem.Param.Param3=Size;
	PreRedraw.SetParam(preRedrawItem.Param);
}

static void PR_DrawGetDirInfoMsg()
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	DrawGetDirInfoMsg(
		static_cast<const wchar_t*>(preRedrawItem.Param.Param1),
		static_cast<const wchar_t*>(preRedrawItem.Param.Param2),
		static_cast<const UINT64*>(preRedrawItem.Param.Param3)
	);
}

class FileIdTree: public Tree<UINT64>
{
public:
	FileIdTree(){}
	~FileIdTree(){clear();}
	long compare(Node<UINT64>* first, UINT64* second) {return *first->data-*second;}
};

int GetDirInfo(const wchar_t *Title, const wchar_t *DirName, DirInfoData& Data, clock_t MsgWaitTime, FileFilter *Filter, DWORD Flags)
{
	string strFullDirName, strDriveRoot;
	string strFullName, strCurDirName, strLastDirName;
	ConvertNameToFull(DirName, strFullDirName);
	SaveScreen SaveScr;
	UndoGlobalSaveScrPtr UndSaveScr(&SaveScr);
	TPreRedrawFuncGuard preRedrawFuncGuard(PR_DrawGetDirInfoMsg);
	TaskBar TB(MsgWaitTime!=-1);
	wakeful W;
	ScanTree ScTree(FALSE,TRUE,(Flags&GETDIRINFO_SCANSYMLINKDEF?(DWORD)-1:(Flags&GETDIRINFO_SCANSYMLINK)));
	FAR_FIND_DATA_EX FindData;
	clock_t StartTime=clock();
	SetCursorType(FALSE,0);
	GetPathRoot(strFullDirName,strDriveRoot);
	/* $ 20.03.2002 DJ
	   для . - покажем имя родительского каталога
	*/
	const wchar_t *ShowDirName = DirName;

	if (DirName[0] == L'.' && !DirName[1])
	{
		const wchar_t *p = LastSlash(strFullDirName);

		if (p)
			ShowDirName = p + 1;
	}

	ConsoleTitle OldTitle;
	RefreshFrameManager frref(ScrX,ScrY,MsgWaitTime,Flags&GETDIRINFO_DONTREDRAWFRAME);
	DWORD SectorsPerCluster=0,BytesPerSector=0,FreeClusters=0,Clusters=0;

	if (GetDiskFreeSpace(strDriveRoot,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters))
		Data.ClusterSize=SectorsPerCluster*BytesPerSector;

	// Временные хранилища имён каталогов
	strLastDirName.Clear();
	strCurDirName.Clear();
	Data.DirCount=Data.FileCount=0;
	Data.FileSize=Data.AllocationSize=Data.FilesSlack=Data.MFTOverhead=0;
	ScTree.SetFindPath(DirName,L"*");

	FileIdTree FileIds;

	bool CheckHardlinks = false;
	DWORD FileSystemFlags = 0;
	string FileSystemName;
	string Root;
	GetPathRoot(DirName, Root);
	if(apiGetVolumeInformation(Root, nullptr, nullptr, nullptr, &FileSystemFlags, &FileSystemName))
	{
		if(WinVer < _WIN32_WINNT_WIN7)
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
		if (!CtrlObject->Macro.IsExecuting())
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
					GetInputRecord(&rec);
					break;
				case KEY_ESC:
				case KEY_BREAK:
					GetInputRecord(&rec);
					return 0;
				default:

					if (Flags&GETDIRINFO_ENHBREAK)
					{
						return -1;
					}

					GetInputRecord(&rec);
					break;
			}
		}

		clock_t CurTime=clock();

		if (MsgWaitTime!=-1 && CurTime-StartTime > MsgWaitTime)
		{
			StartTime=CurTime;
			MsgWaitTime=500;
			OldTitle << MSG(MScanningFolder) << L" " << ShowDirName << fmt::Flush(); // покажем заголовок консоли
			SetCursorType(FALSE,0);
			DrawGetDirInfoMsg(Title,ShowDirName,&Data.FileSize);
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

				if (StrCmpI(strCurDirName,strLastDirName))
				{
					Data.DirCount++;
					strLastDirName = strCurDirName;
				}
			}

			Data.FileCount++;

			Data.FileSize += FindData.nFileSize;

			bool IsDuplicate = false;
			if (CheckHardlinks && FindData.FileId)
			{
				if(FileIds.query(&FindData.FileId))
				{
					IsDuplicate = true;
				}
				else
				{
					FileIds.insert(new UINT64(FindData.FileId));
				}
			}
			if (!IsDuplicate)
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

static PluginPanelItem *PluginDirList;
static int DirListItemsNumber;
static string strPluginSearchPath;
static int StopSearch;
static HANDLE hDirListPlugin;
static int PluginSearchMsgOut;

static void FarGetPluginDirListMsg(const wchar_t *Name,DWORD Flags)
{
	Message(Flags,0,L"",MSG(MPreparingList),Name);
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Flags=Flags;
	preRedrawItem.Param.Param1=(void*)Name;
	PreRedraw.SetParam(preRedrawItem.Param);
}

static void PR_FarGetPluginDirListMsg()
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	FarGetPluginDirListMsg((const wchar_t *)preRedrawItem.Param.Param1,preRedrawItem.Param.Flags&(~MSG_KEEPBACKGROUND));
}

static void CopyPluginDirItem(PluginPanelItem *CurPanelItem)
{
	string strFullName;
	strFullName = strPluginSearchPath;
	strFullName += CurPanelItem->FileName;
	wchar_t *lpwszFullName = strFullName.GetBuffer();

	for (int I=0; lpwszFullName[I]; I++)
		if (lpwszFullName[I]==L'\x1')
			lpwszFullName[I]=L'\\';

	strFullName.ReleaseBuffer();
	PluginPanelItem *DestItem=PluginDirList+DirListItemsNumber;
	*DestItem=*CurPanelItem;

	DestItem->FileName = xf_wcsdup(strFullName);
	DestItem->AlternateFileName=nullptr;
	DirListItemsNumber++;
}

static void ScanPluginDir()
{
	PluginPanelItem *PanelData=nullptr;
	size_t ItemCount=0;
	int AbortOp=FALSE;
	string strDirName;
	strDirName = strPluginSearchPath;
	wchar_t *lpwszDirName = strDirName.GetBuffer();

	for (int i=0; lpwszDirName[i]; i++)
		if (lpwszDirName[i]=='\x1')
			lpwszDirName[i]=lpwszDirName[i+1]?L'\\':0;

	strDirName.ReleaseBuffer();
	TruncStr(strDirName,30);
	CenterStr(strDirName,strDirName,30);

	if (CheckForEscSilent())
	{
		if (Opt.Confirm.Esc) // Будет выдаваться диалог?
			AbortOp=TRUE;

		if (ConfirmAbortOp())
			StopSearch=TRUE;
	}

	FarGetPluginDirListMsg(strDirName,AbortOp?0:MSG_KEEPBACKGROUND);

	if (StopSearch || !CtrlObject->Plugins->GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND))
		return;

	PluginPanelItem *NewList=(PluginPanelItem *)xf_realloc(PluginDirList,1+sizeof(*PluginDirList)*(DirListItemsNumber+ItemCount));

	if (!NewList)
	{
		StopSearch=TRUE;
		return;
	}

	PluginDirList=NewList;

	for (size_t i=0; i<ItemCount && !StopSearch; i++)
	{
		PluginPanelItem *CurPanelItem=PanelData+i;

		if (!(CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			CopyPluginDirItem(CurPanelItem);
	}

	for (size_t i=0; i<ItemCount && !StopSearch; i++)
	{
		PluginPanelItem *CurPanelItem=PanelData+i;

		if ((CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
		        StrCmp(CurPanelItem->FileName,L".") &&
		        !TestParentFolderName(CurPanelItem->FileName))
		{
			PluginPanelItem *NewList=(PluginPanelItem *)xf_realloc(PluginDirList,sizeof(*PluginDirList)*(DirListItemsNumber+1));

			if (!NewList)
			{
				StopSearch=TRUE;
				return;
			}

			PluginDirList=NewList;
			/* $ 30.11.2001 DJ
					используем общую функцию для копирования FindData (не забываем
					обработать PPIF_USERDATA)
			*/
			CopyPluginDirItem(CurPanelItem);
			string strFileName = CurPanelItem->FileName;

			if (CtrlObject->Plugins->SetDirectory(hDirListPlugin,strFileName,OPM_FIND))
			{
				strPluginSearchPath += CurPanelItem->FileName;
				strPluginSearchPath += L"\x1";
				ScanPluginDir();
				size_t pos = (size_t)-1;
				strPluginSearchPath.RPos(pos,L'\x1');
				strPluginSearchPath.SetLength(pos);

				if (strPluginSearchPath.RPos(pos,L'\x1'))
					strPluginSearchPath.SetLength(pos+1);
				else
					strPluginSearchPath.Clear();

				if (!CtrlObject->Plugins->SetDirectory(hDirListPlugin,L"..",OPM_FIND))
				{
					StopSearch=TRUE;
					break;
				}
			}
		}
	}

	CtrlObject->Plugins->FreeFindData(hDirListPlugin,PanelData,ItemCount);
}

int GetPluginDirList(Plugin* PluginNumber, HANDLE hPlugin, const wchar_t *Dir, PluginPanelItem **pPanelItem, size_t *pItemsNumber)
{
	if (!StrCmp(Dir,L".") || TestParentFolderName(Dir))
		return FALSE;

	static PluginHandle DirListPlugin;

	// А не хочет ли плагин посмотреть на текущую панель?
	if (!hPlugin || hPlugin==PANEL_ACTIVE || hPlugin==PANEL_PASSIVE)
	{
		/* $ 30.11.2001 DJ
			А плагиновая ли это панель?
		*/
		HANDLE Handle = ((!hPlugin || hPlugin==PANEL_ACTIVE)?CtrlObject->Cp()->ActivePanel:CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel))->GetPluginHandle();

		if (!Handle)
			return FALSE;

		DirListPlugin=*(PluginHandle *)Handle;
	}
	else
	{
		DirListPlugin.pPlugin=(Plugin*)PluginNumber;
		DirListPlugin.hPlugin=hPlugin;
	}

	{
		SaveScreen SaveScr;
		TPreRedrawFuncGuard preRedrawFuncGuard(PR_FarGetPluginDirListMsg);
		{
			string strDirName;
			strDirName = Dir;
			TruncStr(strDirName,30);
			CenterStr(strDirName,strDirName,30);
			SetCursorType(FALSE,0);
			FarGetPluginDirListMsg(strDirName,0);
			PluginSearchMsgOut=FALSE;
			hDirListPlugin=(HANDLE)&DirListPlugin;
			StopSearch=FALSE;
			*pItemsNumber=DirListItemsNumber=0;
			*pPanelItem=PluginDirList=nullptr;
			OpenPanelInfo Info;
			CtrlObject->Plugins->GetOpenPanelInfo(hDirListPlugin,&Info);
			string strPrevDir = Info.CurDir;

			if (CtrlObject->Plugins->SetDirectory(hDirListPlugin,Dir,OPM_SILENT))
			{
				strPluginSearchPath = Dir;
				strPluginSearchPath += L"\x1";
				ScanPluginDir();
				*pPanelItem=PluginDirList;
				*pItemsNumber=DirListItemsNumber;
				CtrlObject->Plugins->SetDirectory(hDirListPlugin,L"..",OPM_SILENT);
				OpenPanelInfo NewInfo;
				CtrlObject->Plugins->GetOpenPanelInfo(hDirListPlugin,&NewInfo);

				if (StrCmpI(strPrevDir, NewInfo.CurDir) )
				{
					PluginPanelItem *PanelData=nullptr;
					size_t ItemCount=0;

					if (CtrlObject->Plugins->GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_SILENT))
					{
						CtrlObject->Plugins->FreeFindData(hDirListPlugin,PanelData,ItemCount);
					}

					CtrlObject->Plugins->SetDirectory(hDirListPlugin,strPrevDir,OPM_SILENT);
				}
			}
		}
	}
	return !StopSearch;
}

int GetPluginDirInfo(HANDLE hPlugin,const wchar_t *DirName,unsigned long &DirCount,
                     unsigned long &FileCount,unsigned __int64 &FileSize,
                     unsigned __int64 &CompressedFileSize)
{
	PluginPanelItem *PanelItem=nullptr;
	size_t ItemsNumber=0;
	int ExitCode;
	DirCount=FileCount=0;
	FileSize=CompressedFileSize=0;
	PluginHandle *ph = (PluginHandle*)hPlugin;

	if ((ExitCode=GetPluginDirList(ph->pPlugin, ph->hPlugin, DirName, &PanelItem,&ItemsNumber))==TRUE) //intptr_t - BUGBUG
	{
		for (size_t I=0; I<ItemsNumber; I++)
		{
			if (PanelItem[I].FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				DirCount++;
			}
			else
			{
				FileCount++;
				FileSize+=PanelItem[I].FileSize;
				CompressedFileSize+=PanelItem[I].AllocationSize?PanelItem[I].AllocationSize:PanelItem[I].FileSize;
			}
		}
	}

	if (PanelItem)
	{
		pluginapi::apiFreePluginDirList(ph->hPlugin, PanelItem, ItemsNumber);
		if (PanelItem==PluginDirList) // Mantins#0002077
		{
			PluginDirList=nullptr;
			DirListItemsNumber=0;
		}
	}

	return(ExitCode);
}
