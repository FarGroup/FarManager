/*
filelist.cpp

Файловая панель - общие функции
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
#include "vmenu.hpp"
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
#include "plognmn.hpp"
#include "scrbuf.hpp"
#include "CFileMask.hpp"
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

extern PanelViewSettings ViewSettingsArray[];
extern size_t SizeViewSettingsArray;


static int _cdecl SortList(const void *el1,const void *el2);

static int ListSortMode,ListSortOrder,ListSortGroups,ListSelectedFirst,ListDirectoriesFirst;
static int ListPanelMode,ListNumericSort,ListCaseSensitiveSort;
static HANDLE hSortPlugin;

enum SELECT_MODES
{
	SELECT_INVERT          =  0,
	SELECT_INVERTALL       =  1,
	SELECT_ADD             =  2,
	SELECT_REMOVE          =  3,
	SELECT_ADDEXT          =  4,
	SELECT_REMOVEEXT       =  5,
	SELECT_ADDNAME         =  6,
	SELECT_REMOVENAME      =  7,
	SELECT_ADDMASK         =  8,
	SELECT_REMOVEMASK      =  9,
	SELECT_INVERTMASK      = 10,
};

FileList::FileList():
	Filter(nullptr),
	DizRead(FALSE),
	ListData(nullptr),
	FileCount(0),
	hPlugin(nullptr),
	UpperFolderTopFile(0),
	LastCurFile(-1),
	ReturnCurrentFile(FALSE),
	SelFileCount(0),
	GetSelPosition(0),
	TotalFileCount(0),
	SelFileSize(0),
	TotalFileSize(0),
	FreeDiskSize(0),
	LastUpdateTime(0),
	Height(0),
	LeftPos(0),
	ShiftSelection(-1),
	MouseSelection(0),
	SelectedFirst(0),
	AccessTimeUpdateRequired(FALSE),
	UpdateRequired(FALSE),
	UpdateDisabled(0),
	InternalProcessKey(FALSE),
	CacheSelIndex(-1),
	CacheSelClearIndex(-1)
{
	_OT(SysLog(L"[%p] FileList::FileList()", this));
	{
		const wchar_t *data=MSG(MPanelBracketsForLongName);

		if (StrLength(data)>1)
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
	apiGetCurrentDirectory(strCurDir);
	strOriginalCurDir = strCurDir;
	CurTopFile=CurFile=0;
	ShowShortNames=0;
	SortMode=BY_NAME;
	SortOrder=1;
	SortGroups=0;
	ViewMode=VIEW_3;
	ViewSettings=ViewSettingsArray[ViewMode];
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

	DeleteListData(ListData,FileCount);

	if (PanelMode==PLUGIN_PANEL)
		while (PopPlugin(FALSE))
			;

	delete Filter;
}


void FileList::DeleteListData(FileListItem **(&ListData),int &FileCount)
{
	if (ListData)
	{
		for (int I=0; I<FileCount; I++)
		{
			if (ListData[I]->CustomColumnNumber>0 && ListData[I]->CustomColumnData)
			{
				for (size_t J=0; J < ListData[I]->CustomColumnNumber; J++)
					delete[] ListData[I]->CustomColumnData[J];

				delete[] ListData[I]->CustomColumnData;
			}

			if (ListData[I]->UserFlags & PPIF_USERDATA)
				xf_free((void *)ListData[I]->UserData);

			if (ListData[I]->DizText && ListData[I]->DeleteDiz)
				delete[] ListData[I]->DizText;

			delete ListData[I]; //!!!
		}

		xf_free(ListData);
		ListData=nullptr;
		FileCount=0;
	}
}

void FileList::Up(int Count)
{
	CurFile-=Count;

	if (CurFile < 0)
		CurFile=0;

	ShowFileList(TRUE);
}


void FileList::Down(int Count)
{
	CurFile+=Count;

	if (CurFile >= FileCount)
		CurFile=FileCount-1;

	ShowFileList(TRUE);
}

void FileList::Scroll(int Count)
{
	CurTopFile+=Count;

	if (Count<0)
		Up(-Count);
	else
		Down(Count);
}

void FileList::CorrectPosition()
{
	if (!FileCount)
	{
		CurFile=CurTopFile=0;
		return;
	}

	if (CurTopFile+Columns*Height>FileCount)
		CurTopFile=FileCount-Columns*Height;

	if (CurFile<0)
		CurFile=0;

	if (CurFile > FileCount-1)
		CurFile=FileCount-1;

	if (CurTopFile<0)
		CurTopFile=0;

	if (CurTopFile > FileCount-1)
		CurTopFile=FileCount-1;

	if (CurFile<CurTopFile)
		CurTopFile=CurFile;

	if (CurFile>CurTopFile+Columns*Height-1)
		CurTopFile=CurFile-Columns*Height+1;
}


void FileList::SortFileList(int KeepPosition)
{
	if (FileCount>1)
	{
		string strCurName;

		if (SortMode==BY_DIZ)
			ReadDiz();

		ListSortMode=SortMode;
		ListSortOrder=SortOrder;
		ListSortGroups=SortGroups;
		ListSelectedFirst=SelectedFirst;
		ListDirectoriesFirst=DirectoriesFirst;
		ListPanelMode=PanelMode;
		ListNumericSort=NumericSort;
		ListCaseSensitiveSort=CaseSensitiveSort;

		if (KeepPosition)
		{
			assert(CurFile<FileCount);
			strCurName = ListData[CurFile]->strName;
		}

		hSortPlugin=(PanelMode==PLUGIN_PANEL && hPlugin && static_cast<PluginHandle*>(hPlugin)->pPlugin->HasCompare()) ? hPlugin:nullptr;

		far_qsort(ListData,FileCount,sizeof(*ListData),SortList);

		if (KeepPosition)
			GoToFile(strCurName);
	}
}

int _cdecl SortList(const void *el1,const void *el2)
{
	int RetCode;
	const wchar_t *Ext1=nullptr,*Ext2=nullptr;
	FileListItem *SPtr1,*SPtr2;
	SPtr1=((FileListItem **)el1)[0];
	SPtr2=((FileListItem **)el2)[0];

	if (SPtr1->strName.GetLength() == 2 && SPtr1->strName.At(0)==L'.' && SPtr1->strName.At(1)==L'.')
		return -1;

	if (SPtr2->strName.GetLength() == 2 && SPtr2->strName.At(0)==L'.' && SPtr2->strName.At(1)==L'.')
		return 1;

	if (ListSortMode==UNSORTED)
	{
		if (ListSelectedFirst && SPtr1->Selected!=SPtr2->Selected)
			return SPtr1->Selected>SPtr2->Selected ? -1 : 1;

		return (SPtr1->Position>SPtr2->Position) ? ListSortOrder : -ListSortOrder;
	}

	if (ListDirectoriesFirst)
	{
		if ((SPtr1->FileAttr & FILE_ATTRIBUTE_DIRECTORY) < (SPtr2->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
			return 1;

		if ((SPtr1->FileAttr & FILE_ATTRIBUTE_DIRECTORY) > (SPtr2->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
			return -1;
	}

	if (ListSelectedFirst && SPtr1->Selected!=SPtr2->Selected)
		return SPtr1->Selected>SPtr2->Selected ? -1 : 1;

	if (ListSortGroups && (ListSortMode==BY_NAME || ListSortMode==BY_EXT || ListSortMode==BY_FULLNAME) &&
	        SPtr1->SortGroup!=SPtr2->SortGroup)
		return SPtr1->SortGroup<SPtr2->SortGroup ? -1 : 1;

	if (hSortPlugin)
	{
		UINT64 SaveFlags1 = SPtr1->UserFlags ,SaveFlags2 = SPtr2->UserFlags;
		SPtr1->UserFlags=SPtr2->UserFlags=0;
		PluginPanelItem pi1,pi2;
		FileList::FileListToPluginItem(SPtr1,&pi1);
		FileList::FileListToPluginItem(SPtr2,&pi2);
		SPtr1->UserFlags=SaveFlags1;
		SPtr2->UserFlags=SaveFlags2;
		RetCode=CtrlObject->Plugins->Compare(hSortPlugin,&pi1,&pi2,ListSortMode+(SM_UNSORTED-UNSORTED));
		FileList::FreePluginPanelItem(&pi1);
		FileList::FreePluginPanelItem(&pi2);

		if (RetCode!=-2)
			return ListSortOrder*RetCode;
	}

	// НЕ СОРТИРУЕМ КАТАЛОГИ В РЕЖИМЕ "ПО РАСШИРЕНИЮ" (Опционально!)
	if (!(ListSortMode == BY_EXT && !Opt.SortFolderExt && ((SPtr1->FileAttr & FILE_ATTRIBUTE_DIRECTORY) && (SPtr2->FileAttr & FILE_ATTRIBUTE_DIRECTORY))))
	{
		__int64 RetCode64;
		switch (ListSortMode)
		{
			case BY_NAME:
				break;

			case BY_EXT:
				Ext1=PointToExt(SPtr1->strName);
				Ext2=PointToExt(SPtr2->strName);

				if (!*Ext1 && !*Ext2)
					break;

				if (!*Ext1)
					return(-ListSortOrder);

				if (!*Ext2)
					return(ListSortOrder);

				if (ListNumericSort)
				{
					RetCode=ListSortOrder*(ListCaseSensitiveSort?NumStrCmpC(Ext1+1,Ext2+1):NumStrCmpI(Ext1+1,Ext2+1));
				}
				else
				{
					RetCode=ListSortOrder*(ListCaseSensitiveSort?StrCmpC(Ext1+1,Ext2+1):StrCmpI(Ext1+1,Ext2+1));
				}

				if (RetCode)
					return RetCode;
				break;

			case BY_MTIME:
				if (!(RetCode64=FileTimeDifference(&SPtr1->WriteTime,&SPtr2->WriteTime)))
					break;

				return -ListSortOrder*(RetCode64<0?-1:1);

			case BY_CTIME:
				if (!(RetCode64=FileTimeDifference(&SPtr1->CreationTime,&SPtr2->CreationTime)))
					break;

				return -ListSortOrder*(RetCode64<0?-1:1);

			case BY_ATIME:
				if (!(RetCode64=FileTimeDifference(&SPtr1->AccessTime,&SPtr2->AccessTime)))
					break;

				return -ListSortOrder*(RetCode64<0?-1:1);

			case BY_CHTIME:
				if (!(RetCode64=FileTimeDifference(&SPtr1->ChangeTime, &SPtr2->ChangeTime)))
					break;

				return -ListSortOrder*(RetCode64<0?-1:1);

			case BY_SIZE:
				if (SPtr1->FileSize==SPtr2->FileSize)
					break;

				return ((SPtr1->FileSize > SPtr2->FileSize) ? -ListSortOrder : ListSortOrder);

			case BY_DIZ:
				if (!SPtr1->DizText)
				{
					if (!SPtr2->DizText)
						break;
					else
						return ListSortOrder;
				}

				if (!SPtr2->DizText)
					return -ListSortOrder;

				if (ListNumericSort)
				{
					RetCode=ListSortOrder*(ListCaseSensitiveSort?NumStrCmpC(SPtr1->DizText,SPtr2->DizText):NumStrCmpI(SPtr1->DizText,SPtr2->DizText));
				}
				else
				{
					RetCode=ListSortOrder*(ListCaseSensitiveSort?StrCmpC(SPtr1->DizText,SPtr2->DizText):StrCmpI(SPtr1->DizText,SPtr2->DizText));
				}

				if (RetCode)
					return RetCode;
				break;

			case BY_OWNER:
				RetCode=ListSortOrder*StrCmpI(SPtr1->strOwner,SPtr2->strOwner);
				if (RetCode)
					return RetCode;
				break;

			case BY_COMPRESSEDSIZE:
				return (SPtr1->AllocationSize > SPtr2->AllocationSize) ? -ListSortOrder : ListSortOrder;

			case BY_NUMLINKS:
				if (SPtr1->NumberOfLinks==SPtr2->NumberOfLinks)
					break;

				return (SPtr1->NumberOfLinks > SPtr2->NumberOfLinks) ? -ListSortOrder : ListSortOrder;

			case BY_NUMSTREAMS:
				if (SPtr1->NumberOfStreams==SPtr2->NumberOfStreams)
					break;

				return (SPtr1->NumberOfStreams > SPtr2->NumberOfStreams) ? -ListSortOrder : ListSortOrder;

			case BY_STREAMSSIZE:
				if (SPtr1->StreamsSize==SPtr2->StreamsSize)
					break;

				return (SPtr1->StreamsSize > SPtr2->StreamsSize) ? -ListSortOrder : ListSortOrder;

			case BY_FULLNAME:
			{
				int NameCmp;
				if (ListNumericSort)
				{
					const wchar_t *Path1 = SPtr1->strName.CPtr();
					const wchar_t *Path2 = SPtr2->strName.CPtr();
					const wchar_t *Name1 = PointToName(SPtr1->strName);
					const wchar_t *Name2 = PointToName(SPtr2->strName);
					NameCmp = ListCaseSensitiveSort ? StrCmpNNC(Path1, static_cast<int>(Name1-Path1), Path2, static_cast<int>(Name2-Path2)) : StrCmpNNI(Path1, static_cast<int>(Name1-Path1), Path2, static_cast<int>(Name2-Path2));
					if (!NameCmp)
						NameCmp = ListCaseSensitiveSort ? NumStrCmpC(Name1, Name2) : NumStrCmpI(Name1, Name2);
					else
						NameCmp = ListCaseSensitiveSort ? StrCmpC(Path1, Path2) : StrCmpI(Path1, Path2);
				}
				else
				{
					NameCmp = ListCaseSensitiveSort ? StrCmpC(SPtr1->strName, SPtr2->strName) : StrCmpI(SPtr1->strName, SPtr2->strName);
				}
				NameCmp *= ListSortOrder;
				if (!NameCmp)
					NameCmp = SPtr1->Position > SPtr2->Position ? ListSortOrder : -ListSortOrder;
				return NameCmp;
			}

			case BY_CUSTOMDATA:
				if (SPtr1->strCustomData.IsEmpty())
				{
					if (SPtr2->strCustomData.IsEmpty())
						break;
					else
						return ListSortOrder;
				}

				if (SPtr2->strCustomData.IsEmpty())
					return -ListSortOrder;

				if (ListNumericSort)
				{
					RetCode=ListSortOrder*(ListCaseSensitiveSort?NumStrCmpC(SPtr1->strCustomData, SPtr2->strCustomData):NumStrCmpI(SPtr1->strCustomData, SPtr2->strCustomData));
				}
				else
				{
					RetCode=ListSortOrder*(ListCaseSensitiveSort?StrCmpC(SPtr1->strCustomData, SPtr2->strCustomData):StrCmpI(SPtr1->strCustomData, SPtr2->strCustomData));
				}

				if (RetCode)
					return RetCode;
				break;
		}
	}

	int NameCmp=0;

	if (!Opt.SortFolderExt && (SPtr1->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		Ext1=SPtr1->strName.CPtr()+SPtr1->strName.GetLength();
	}
	else
	{
		if (!Ext1) Ext1=PointToExt(SPtr1->strName);
	}

	if (!Opt.SortFolderExt && (SPtr2->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		Ext2=SPtr2->strName.CPtr()+SPtr2->strName.GetLength();
	}
	else
	{
		if (!Ext2) Ext2=PointToExt(SPtr2->strName);
	}

	const wchar_t *Name1=PointToName(SPtr1->strName);
	const wchar_t *Name2=PointToName(SPtr2->strName);

	if (ListNumericSort)
		NameCmp=ListCaseSensitiveSort?NumStrCmpNC(Name1,static_cast<int>(Ext1-Name1),Name2,static_cast<int>(Ext2-Name2)):NumStrCmpNI(Name1,static_cast<int>(Ext1-Name1),Name2,static_cast<int>(Ext2-Name2));
	else
		NameCmp=ListCaseSensitiveSort?StrCmpNNC(Name1,static_cast<int>(Ext1-Name1),Name2,static_cast<int>(Ext2-Name2)):StrCmpNNI(Name1,static_cast<int>(Ext1-Name1),Name2,static_cast<int>(Ext2-Name2));

	if (!NameCmp)
	{
		if (ListNumericSort)
			NameCmp=ListCaseSensitiveSort?NumStrCmpC(Ext1,Ext2):NumStrCmpI(Ext1,Ext2);
		else
			NameCmp=ListCaseSensitiveSort?StrCmpC(Ext1,Ext2):StrCmpI(Ext1,Ext2);
	}

	NameCmp*=ListSortOrder;

	if (!NameCmp)
		NameCmp=SPtr1->Position>SPtr2->Position ? ListSortOrder:-ListSortOrder;

	return NameCmp;
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
	if (!IsRedrawFramesInProcess)
		SetTitle();
}

int FileList::SendKeyToPlugin(DWORD Key,bool Pred)
{
	_ALGO(CleverSysLog clv(L"FileList::SendKeyToPlugin()"));
	_ALGO(SysLog(L"Key=%s Pred=%d",_FARKEY_ToName(Key),Pred));

	if (PanelMode==PLUGIN_PANEL &&
	        (CtrlObject->Macro.IsRecording() == MACROMODE_RECORDING_COMMON || CtrlObject->Macro.IsExecuting() == MACROMODE_EXECUTING_COMMON || CtrlObject->Macro.GetCurRecord(nullptr,nullptr) == MACROMODE_NOMACRO)
	   )
	{
		_ALGO(SysLog(L"call Plugins.ProcessKey() {"));
		INPUT_RECORD rec;
		KeyToInputRecord(Key,&rec);
		int ProcessCode=CtrlObject->Plugins->ProcessKey(hPlugin,&rec,Pred);
		_ALGO(SysLog(L"} ProcessCode=%d",ProcessCode));
		ProcessPluginCommand();

		if (ProcessCode)
			return TRUE;
	}

	return FALSE;
}

#ifdef FAR_LUA
#else
__int64 FileList::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_C_ROOTFOLDER:
		{
			if (PanelMode==PLUGIN_PANEL)
			{
				OpenPanelInfo Info;
				CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
				return (__int64)(!*NullToEmpty(Info.CurDir));
			}
			else
			{
				if (!IsRootPath(strCurDir))
				{
					string strDriveRoot;
					GetPathRoot(strCurDir, strDriveRoot);
					return (__int64)(!StrCmpI(strCurDir, strDriveRoot));
				}

				return 1;
			}
		}
		case MCODE_C_EOF:
			return (CurFile == FileCount-1);
		case MCODE_C_BOF:
			return !CurFile;
		case MCODE_C_SELECTED:
			return (GetRealSelCount()>1);
		case MCODE_V_ITEMCOUNT:
			return (FileCount);
		case MCODE_V_CURPOS:
			return (CurFile+1);
		case MCODE_C_APANEL_FILTER:
			return (Filter && Filter->IsEnabledOnPanel());

		case MCODE_V_APANEL_PREFIX:           // APanel.Prefix
		case MCODE_V_PPANEL_PREFIX:           // PPanel.Prefix
		{
			PluginInfo *PInfo=(PluginInfo *)vParam;
			if (GetMode() == PLUGIN_PANEL && hPlugin && ((PluginHandle*)hPlugin)->pPlugin)
				return ((PluginHandle*)hPlugin)->pPlugin->GetPluginInfo(PInfo)?1:0;
			return 0;
		}

		case MCODE_V_APANEL_FORMAT:           // APanel.Format
		case MCODE_V_PPANEL_FORMAT:           // PPanel.Format
		{
			OpenPanelInfo *PInfo=(OpenPanelInfo *)vParam;
			if (GetMode() == PLUGIN_PANEL && hPlugin)
			{
				CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,PInfo);
				return 1;
			}
			return 0;
		}

		case MCODE_V_APANEL_PATH0:
		case MCODE_V_PPANEL_PATH0:
		{
			if (PluginsList.Empty())
				return 0;
			*(string *)vParam = (*PluginsList.Last())->strPrevOriginalCurDir;
			return 1;
		}

		case MCODE_F_PANEL_SELECT:
		{
			// vParam = MacroPanelSelect*, iParam = 0
			__int64 Result=-1;
			MacroPanelSelect *mps=(MacroPanelSelect *)vParam;

			if (!ListData)
				return Result;

			if (mps->Mode == 1 && (DWORD)mps->Index >= (DWORD)FileCount)
				return Result;

			UserDefinedList *itemsList=nullptr;

			if (mps->Action != 3)
			{
				if (mps->Mode == 2)
				{
					itemsList=new UserDefinedList(ULF_UNIQUE,  L"\r\n");
					if (!itemsList->Set(mps->Item->s()))
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
							Result=(__int64)GetRealSelCount();
							ClearSelection();
							break;
						case 1: // по индексу?
							Result=1;
							Select(ListData[mps->Index],FALSE);
							break;
						case 2: // набор строк
						{
							const wchar_t *namePtr;
							int Pos;
							Result=0;

							while((namePtr=itemsList->GetNext()) )
							{
								if ((Pos=FindFile(PointToName(namePtr),TRUE)) != -1)
								{
									Select(ListData[Pos],FALSE);
									Result++;
								}
							}
							break;
						}
						case 3: // масками файлов, разделенных запятыми
							Result=SelectFiles(SELECT_REMOVEMASK,mps->Item->s());
							break;
					}
					break;
				}

				case 1:  // добавить выделение
				{
					switch(mps->Mode)
					{
						case 0: // выделить все?
						    for (int i=0; i < FileCount; i++)
						    	Select(ListData[i],TRUE);
							Result=(__int64)GetRealSelCount();
							break;
						case 1: // по индексу?
							Result=1;
							Select(ListData[mps->Index],TRUE);
							break;
						case 2: // набор строк через CRLF
						{
							const wchar_t *namePtr;
							int Pos;
							Result=0;

							while((namePtr=itemsList->GetNext()) )
							{
								if ((Pos=FindFile(PointToName(namePtr),TRUE)) != -1)
								{
									Select(ListData[Pos],TRUE);
									Result++;
								}
							}
							break;
						}
						case 3: // масками файлов, разделенных запятыми
							Result=SelectFiles(SELECT_ADDMASK,mps->Item->s());
							break;
					}
					break;
				}

				case 2:  // инвертировать выделение
				{
					switch(mps->Mode)
					{
						case 0: // инвертировать все?
						    for (int i=0; i < FileCount; i++)
						    	Select(ListData[i],ListData[i]->Selected?FALSE:TRUE);
							Result=(__int64)GetRealSelCount();
							break;
						case 1: // по индексу?
							Result=1;
							Select(ListData[mps->Index],ListData[mps->Index]->Selected?FALSE:TRUE);
							break;
						case 2: // набор строк через CRLF
						{
							const wchar_t *namePtr;
							int Pos;
							Result=0;

							while((namePtr=itemsList->GetNext()) )
							{
								if ((Pos=FindFile(PointToName(namePtr),TRUE)) != -1)
								{
									Select(ListData[Pos],ListData[Pos]->Selected?FALSE:TRUE);
									Result++;
								}
							}
							break;
						}
						case 3: // масками файлов, разделенных запятыми
							Result=SelectFiles(SELECT_INVERTMASK,mps->Item->s());
							break;
					}
					break;
				}

				case 3:  // восстановить выделение
				{
					RestoreSelection();
					Result=(__int64)GetRealSelCount();
					break;
				}
			}

			if (Result != -1 && mps->Action != 3)
			{
				if (SelectedFirst)
					SortFileList(TRUE);
				Redraw();
			}

			if (itemsList)
				delete itemsList;

			return Result;
		}
	}

	return 0;
}
#endif

int FileList::ProcessKey(int Key)
{
	Elevation.ResetApprove();

	FileListItem *CurPtr=nullptr;
	int N;
	int CmdLength=CtrlObject->CmdLine->GetLength();

	if (IsVisible())
	{
		if (!InternalProcessKey)
			if ((!(Key==KEY_ENTER||Key==KEY_NUMENTER) && !(Key==KEY_SHIFTENTER||Key==KEY_SHIFTNUMENTER)) || !CmdLength)
				if (SendKeyToPlugin(Key))
					return TRUE;
	}
	else
	{
		// Те клавиши, которые работают при погашенных панелях:
		switch (Key)
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
		if ((Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9) ||
			(Key>=KEY_RCTRLSHIFT0 && Key<=KEY_RCTRLSHIFT9) ||
			(Key>=KEY_CTRLALT0 && Key<=KEY_CTRLALT9) ||
			(Key>=KEY_RCTRLRALT0 && Key<=KEY_RCTRLRALT9) ||
			(Key>=KEY_CTRLRALT0 && Key<=KEY_CTRLRALT9) ||
			(Key>=KEY_RCTRLALT0 && Key<=KEY_RCTRLALT9)
		)
		{
			bool Add = (Key&KEY_SHIFT) == KEY_SHIFT;
			SaveShortcutFolder((Key&(~(KEY_CTRL|KEY_RCTRL|KEY_ALT|KEY_RALT|KEY_SHIFT|KEY_RSHIFT)))-'0', Add);
			return TRUE;
		}
		// Jump to a folder shortcut?
		else if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
		{
			ExecShortcutFolder(Key-KEY_RCTRL0);
			return TRUE;
		}
	}

	/* $ 27.08.2002 SVS
	    [*] В панели с одной колонкой Shift-Left/Right аналогично нажатию
	        Shift-PgUp/PgDn.
	*/
	if (Columns==1 && !CmdLength)
	{
		if (Key == KEY_SHIFTLEFT || Key == KEY_SHIFTNUMPAD4)
			Key=KEY_SHIFTPGUP;
		else if (Key == KEY_SHIFTRIGHT || Key == KEY_SHIFTNUMPAD6)
			Key=KEY_SHIFTPGDN;
	}

	switch (Key)
	{
		case KEY_GOTFOCUS:
			if (Opt.SmartFolderMonitor)
			{
				StartFSWatcher();
				CtrlObject->Cp()->GetAnotherPanel(this)->StartFSWatcher();
			}
			break;

		case KEY_KILLFOCUS:
			if (Opt.SmartFolderMonitor)
			{
				StopFSWatcher();
				CtrlObject->Cp()->GetAnotherPanel(this)->StopFSWatcher();
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
			PluginHandle *ph = (PluginHandle*)hPlugin;

			if (PanelMode==PLUGIN_PANEL)
				CtrlObject->Plugins->ConfigureCurrent(ph->pPlugin, FarGuid);
			else
				CtrlObject->Plugins->Configure();

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
				for (int I=0; I < FileCount; I++)
				{
					CurPtr = ListData[I];

					if (!(CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY) || Opt.SelectFolders)
						Select(CurPtr,1);
				}
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
			LeftPos=(Key == KEY_ALTHOME || Key == KEY_RALTHOME)?-0x7fff:LeftPos-1;
			Redraw();
			return TRUE;
		case KEY_ALTRIGHT:    // Прокрутка длинных имен и описаний
		case KEY_RALTRIGHT:
		case KEY_ALTEND:     // Прокрутка длинных имен и описаний - в конец
		case KEY_RALTEND:
			LeftPos=(Key == KEY_ALTEND || Key == KEY_RALTEND)?0x7fff:LeftPos+1;
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
					Key == KEY_CTRLALTINS || Key == KEY_RCTRLRALTINS || Key == KEY_CTRLRALTINS || Key == KEY_RCTRLALTINS ||
					Key == KEY_ALTSHIFTINS || Key == KEY_RALTSHIFTINS ||
					Key == KEY_CTRLALTNUMPAD0 || Key == KEY_RCTRLRALTNUMPAD0 || Key == KEY_CTRLRALTNUMPAD0 || Key == KEY_RCTRLALTNUMPAD0 ||
					Key == KEY_ALTSHIFTNUMPAD0 || Key == KEY_RALTSHIFTNUMPAD0,
				(Key&(KEY_CTRL|KEY_ALT))==(KEY_CTRL|KEY_ALT) || (Key&(KEY_RCTRL|KEY_RALT))==(KEY_RCTRL|KEY_RALT)
			);
			return TRUE;

		case KEY_CTRLC: // hdrop
		case KEY_RCTRLC:
			CopyFiles();
			return TRUE;

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

			if (Key & (KEY_ALT|KEY_RALT))
				NewKey|=KEY_ALT;

			Panel *SrcPanel = CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);
			int OldState = SrcPanel->IsVisible();
			SrcPanel->SetVisible(1);
			SrcPanel->ProcessKey(NewKey);
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
			if (FileCount>0 && SetCurPath())
			{
				string strFileName;

				if (Key==KEY_CTRLSHIFTENTER || Key==KEY_RCTRLSHIFTENTER || Key==KEY_CTRLSHIFTNUMENTER || Key==KEY_RCTRLSHIFTNUMENTER)
				{
					_MakePath1(Key,strFileName, L" ");
				}
				else
				{
					int CurrentPath=FALSE;
					assert(CurFile<FileCount);
					CurPtr=ListData[CurFile];

					if (ShowShortNames && !CurPtr->strShortName.IsEmpty())
						strFileName = CurPtr->strShortName;
					else
						strFileName = CurPtr->strName;

					if (TestParentFolderName(strFileName))
					{
						if (PanelMode==PLUGIN_PANEL)
							strFileName.Clear();
						else
							strFileName.SetLength(1); // "."

						if (!(Key==KEY_CTRLALTF || Key==KEY_RCTRLRALTF || Key==KEY_CTRLRALTF || Key==KEY_RCTRLALTF))
							Key=KEY_CTRLF;

						CurrentPath=TRUE;
					}

					if (Key==KEY_CTRLF || Key==KEY_RCTRLF || Key==KEY_CTRLALTF || Key==KEY_RCTRLRALTF || Key==KEY_CTRLRALTF || Key==KEY_RCTRLALTF)
					{
						OpenPanelInfo Info={};

						if (PanelMode==PLUGIN_PANEL)
						{
							CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
						}

						if (PanelMode!=PLUGIN_PANEL)
							CreateFullPathName(CurPtr->strName,CurPtr->strShortName,CurPtr->FileAttr, strFileName, Key==KEY_CTRLALTF || Key==KEY_RCTRLRALTF || Key==KEY_CTRLRALTF || Key==KEY_RCTRLALTF);
						else
						{
							string strFullName = Info.CurDir;

							if (Opt.PanelCtrlFRule && (ViewSettings.Flags&PVS_FOLDERUPPERCASE))
								strFullName.Upper();

							if (!strFullName.IsEmpty())
								AddEndSlash(strFullName,0);

							if (Opt.PanelCtrlFRule)
							{
								/* $ 13.10.2000 tran
								  по Ctrl-f имя должно отвечать условиям на панели */
								if ((ViewSettings.Flags&PVS_FILELOWERCASE) && !(CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
									strFileName.Lower();

								if ((ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE))
									if (!(CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY) && !IsCaseMixed(strFileName))
										strFileName.Lower();
							}

							strFullName += strFileName;
							strFileName = strFullName;
						}
					}

					if (CurrentPath)
						AddEndSlash(strFileName);

					// добавим первый префикс!
					if (PanelMode==PLUGIN_PANEL && Opt.SubstPluginPrefix && !(Key == KEY_CTRLENTER || Key == KEY_RCTRLENTER || Key == KEY_CTRLNUMENTER || Key == KEY_RCTRLNUMENTER || Key == KEY_CTRLJ || Key == KEY_RCTRLJ))
					{
						string strPrefix;

						/* $ 19.11.2001 IS оптимизация по скорости :) */
						if (*AddPluginPrefix((FileList *)CtrlObject->Cp()->ActivePanel,strPrefix))
						{
							strPrefix += strFileName;
							strFileName = strPrefix;
						}
					}

					if (Opt.QuotedName&QUOTEDNAME_INSERT)
						QuoteSpace(strFileName);

					strFileName += L" ";
				}

				CtrlObject->CmdLine->InsertString(strFileName);
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

			if (_MakePath1(Key,strPanelDir, L""))
				CtrlObject->CmdLine->InsertString(strPanelDir);

			return TRUE;
		}
		case KEY_CTRLA:
		case KEY_RCTRLA:
		{
			_ALGO(CleverSysLog clv(L"Ctrl-A"));

			if (FileCount>0 && SetCurPath())
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
			        CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FAROTHER))
				if (FileCount>0 && ApplyCommand())
				{
					// позиционируемся в панели
					if (!FrameManager->IsPanelsActive())
						FrameManager->ActivateFrame(0);

					Update(UPDATE_KEEP_SELECTION);
					Redraw();
					Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
					AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
					AnotherPanel->Redraw();
				}

			return TRUE;
		}
		case KEY_CTRLZ:
		case KEY_RCTRLZ:

			if (FileCount>0 && PanelMode==NORMAL_PANEL && SetCurPath())
				DescribeFiles();

			return TRUE;
		case KEY_CTRLH:
		case KEY_RCTRLH:
		{
			Opt.ShowHidden=!Opt.ShowHidden;
			Update(UPDATE_KEEP_SELECTION);
			Redraw();
			Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
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
				Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

				if (AnotherPanel->GetType()!=FILE_PANEL)
				{
					AnotherPanel->SetCurDir(strCurDir,FALSE);
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
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(Key)));

			if (!FileCount)
				break;

			if (CmdLength)
			{
				CtrlObject->CmdLine->ProcessKey(Key);
				return TRUE;
			}

			ProcessEnter(1,Key==KEY_SHIFTENTER||Key==KEY_SHIFTNUMENTER, true,
					Key == KEY_CTRLALTENTER || Key == KEY_RCTRLRALTENTER || Key == KEY_CTRLRALTENTER || Key == KEY_RCTRLALTENTER ||
					Key == KEY_CTRLALTNUMENTER || Key == KEY_RCTRLRALTNUMENTER || Key == KEY_CTRLRALTNUMENTER || Key == KEY_RCTRLALTNUMENTER);
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
				CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

				if (!Info.CurDir || !*Info.CurDir)
				{
					ChangeDir(L"..");
					NeedChangeDir=FALSE;
					//"this" мог быть удалён в ChangeDir
					Panel* ActivePanel = CtrlObject->Cp()->ActivePanel;

					if (CheckFullScreen!=ActivePanel->IsFullScreen())
						CtrlObject->Cp()->GetAnotherPanel(ActivePanel)->Show();
				}
			}

			if (NeedChangeDir)
				ChangeDir(L"\\");

			CtrlObject->Cp()->ActivePanel->Show();
			return TRUE;
		}
		case KEY_SHIFTF1:
		{
			_ALGO(CleverSysLog clv(L"Shift-F1"));
			_ALGO(SysLog(L"%s, FileCount=%d",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

			if (FileCount>0 && PanelMode!=PLUGIN_PANEL && SetCurPath())
				PluginPutFilesToNew();

			return TRUE;
		}
		case KEY_SHIFTF2:
		{
			_ALGO(CleverSysLog clv(L"Shift-F2"));
			_ALGO(SysLog(L"%s, FileCount=%d",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

			if (FileCount>0 && SetCurPath())
			{
				if (PanelMode==PLUGIN_PANEL)
				{
					OpenPanelInfo Info;
					CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

					if (Info.HostFile && *Info.HostFile)
						ProcessKey(KEY_F5);
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
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(Key)));
			OpenPanelInfo Info={};
			BOOL RefreshedPanel=TRUE;

			if (PanelMode==PLUGIN_PANEL)
				CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

			if (Key == KEY_NUMPAD5 || Key == KEY_SHIFTNUMPAD5)
				Key=KEY_F3;

			if ((Key==KEY_SHIFTF4 || FileCount>0) && SetCurPath())
			{
				int Edit=(Key==KEY_F4 || Key==KEY_ALTF4 || Key==KEY_RALTF4 || Key==KEY_SHIFTF4 || Key==KEY_CTRLSHIFTF4 || Key==KEY_RCTRLSHIFTF4);
				BOOL Modaling=FALSE; ///
				int UploadFile=TRUE;
				string strPluginData;
				string strFileName;
				string strShortFileName;
				string strHostFile=Info.HostFile;
				string strInfoCurDir=Info.CurDir;
				bool PluginMode=PanelMode==PLUGIN_PANEL && !CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARGETFILE);

				if (PluginMode)
				{
					if (Info.Flags & OPIF_REALNAMES)
						PluginMode=FALSE;
					else
						strPluginData.Format(L"<%s:%s>",(const wchar_t*)strHostFile,(const wchar_t*)strInfoCurDir);
				}

				if (!PluginMode)
					strPluginData.Clear();

				UINT codepage = CP_DEFAULT;

				if (Key==KEY_SHIFTF4)
				{
					do
					{
						if (!dlgOpenEditor(strFileName, codepage))
							return FALSE;

						if (!strFileName.IsEmpty())
						{
							Unquote(strFileName);
							ConvertNameToShort(strFileName,strShortFileName);

							if (IsAbsolutePath(strFileName))
							{
								PluginMode=FALSE;
							}

							size_t pos;

							// проверим путь к файлу
							if (FindLastSlash(pos,strFileName) && pos)
							{
								if (!(HasPathPrefix(strFileName) && pos==3))
								{
									wchar_t *lpwszFileName = strFileName.GetBuffer();
									wchar_t wChr = lpwszFileName[pos+1];
									lpwszFileName[pos+1]=0;
									DWORD CheckFAttr=apiGetFileAttributes(lpwszFileName);

									if (CheckFAttr == INVALID_FILE_ATTRIBUTES)
									{
										SetMessageHelp(L"WarnEditorPath");

										if (Message(MSG_WARNING,2,MSG(MWarning),
													MSG(MEditNewPath1),
													MSG(MEditNewPath2),
													MSG(MEditNewPath3),
													MSG(MHYes),MSG(MHNo)))
											return FALSE;
									}

									lpwszFileName[pos+1]=wChr;
									//strFileName.ReleaseBuffer (); это не надо так как строка не поменялась
								}
							}
						}
						else if (PluginMode) // пустое имя файла в панели плагина не разрешается!
						{
							SetMessageHelp(L"WarnEditorPluginName");

							if (Message(MSG_WARNING,2,MSG(MWarning),
							            MSG(MEditNewPlugin1),
							            MSG(MEditNewPath3),MSG(MCancel)))
								return FALSE;
						}
						else
						{
							strFileName = MSG(MNewFileName);
						}
					}
					while (strFileName.IsEmpty());
				}
				else
				{
					assert(CurFile<FileCount);
					CurPtr=ListData[CurFile];

					if (CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (Edit)
							return ProcessKey(KEY_CTRLA);

						CountDirSize(Info.Flags);
						return TRUE;
					}

					strFileName = CurPtr->strName;

					if (!CurPtr->strShortName.IsEmpty())
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

					apiCreateDirectory(strTempDir,nullptr);
					strTempName=strTempDir+L"\\"+PointToName(strFileName);

					if (Key==KEY_SHIFTF4)
					{
						int Pos=FindFile(strFileName);

						if (Pos!=-1)
							CurPtr=ListData[Pos];
						else
						{
							NewFile=TRUE;
							strFileName = strTempName;
						}
					}

					if (!NewFile)
					{
						PluginPanelItem PanelItem;
						FileListToPluginItem(CurPtr,&PanelItem);
						int Result=CtrlObject->Plugins->GetFile(hPlugin,&PanelItem,strTempDir,strFileName,OPM_SILENT|(Edit ? OPM_EDIT:OPM_VIEW));
						FreePluginPanelItem(&PanelItem);

						if (!Result)
						{
							apiRemoveDirectory(strTempDir);
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

				if (!strFileName.IsEmpty())
				{
					if (Edit)
					{
						int EnableExternal=(((Key==KEY_F4 || Key==KEY_SHIFTF4) && Opt.EdOpt.UseExternalEditor) ||
						                    ((Key==KEY_ALTF4 || Key==KEY_RALTF4) && !Opt.EdOpt.UseExternalEditor)) && !Opt.strExternalEditor.IsEmpty();
						/* $ 02.08.2001 IS обработаем ассоциации для alt-f4 */
						BOOL Processed=FALSE;

						if ((Key==KEY_ALTF4 || Key==KEY_RALTF4) &&
						        ProcessLocalFileTypes(strFileName,strShortFileName,FILETYPE_ALTEDIT,
						                              PluginMode))
							Processed=TRUE;
						else if (Key==KEY_F4 &&
						         ProcessLocalFileTypes(strFileName,strShortFileName,FILETYPE_EDIT,
						                               PluginMode))
							Processed=TRUE;

						if (!Processed || Key==KEY_CTRLSHIFTF4 || Key==KEY_RCTRLSHIFTF4)
						{
							if (EnableExternal)
								ProcessExternal(Opt.strExternalEditor,strFileName,strShortFileName,PluginMode);
							else if (PluginMode)
							{
								RefreshedPanel=FrameManager->GetCurrentFrame()->GetType()==MODALTYPE_EDITOR?FALSE:TRUE;
								FileEditor ShellEditor(strFileName,codepage,(Key==KEY_SHIFTF4?FFILEEDIT_CANNEWFILE:0)|FFILEEDIT_DISABLEHISTORY,-1,-1,&strPluginData);
								ShellEditor.SetDynamicallyBorn(false);
								FrameManager->EnterModalEV();
								FrameManager->ExecuteModal();//OT
								FrameManager->ExitModalEV();
								/* $ 24.11.2001 IS
								     Если мы создали новый файл, то не важно, изменялся он
								     или нет, все равно добавим его на панель плагина.
								*/
								UploadFile=ShellEditor.IsFileChanged() || NewFile;
								Modaling=TRUE;///
							}
							else
							{
								FileEditor *ShellEditor=new FileEditor(strFileName,codepage,(Key==KEY_SHIFTF4?FFILEEDIT_CANNEWFILE:0)|FFILEEDIT_ENABLEF6);

								if (ShellEditor)
								{
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

											for (int I=0; I<FileCount; I++)
												if (!(ListData[I]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
													EditList.AddName(ListData[I]->strName, ListData[I]->strShortName);

											EditList.SetCurDir(strCurDir);
											EditList.SetCurName(strFileName);
											ShellEditor->SetNamesList(&EditList);
										}

										FrameManager->ExecuteModal();
									}
								}
							}
						}

						if (PluginMode && UploadFile)
						{
							PluginPanelItem PanelItem;
							string strSaveDir;
							apiGetCurrentDirectory(strSaveDir);

							if (apiGetFileAttributes(strTempName)==INVALID_FILE_ATTRIBUTES)
							{
								string strFindName;
								string strPath;
								strPath = strTempName;
								CutToSlash(strPath, false);
								strFindName = strPath+L"*";
								FAR_FIND_DATA_EX FindData;
								::FindFile Find(strFindName);
								while(Find.Get(FindData))
								{
									if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
									{
										strTempName = strPath+FindData.strFileName;
										break;
									}
								}
							}

							if (FileNameToPluginItem(strTempName,&PanelItem))
							{
								int PutCode=CtrlObject->Plugins->PutFiles(hPlugin,&PanelItem,1,FALSE,OPM_EDIT);

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
						int EnableExternal=((Key==KEY_F3 && Opt.ViOpt.UseExternalViewer) ||
						                    ((Key==KEY_ALTF3 || Key==KEY_RALTF3) && !Opt.ViOpt.UseExternalViewer)) &&
						                   !Opt.strExternalViewer.IsEmpty();
						/* $ 02.08.2001 IS обработаем ассоциации для alt-f3 */
						BOOL Processed=FALSE;

						if ((Key==KEY_ALTF3 || Key==KEY_RALTF3) &&
						        ProcessLocalFileTypes(strFileName,strShortFileName,FILETYPE_ALTVIEW,PluginMode))
							Processed=TRUE;
						else if (Key==KEY_F3 &&
						         ProcessLocalFileTypes(strFileName,strShortFileName,FILETYPE_VIEW,PluginMode))
							Processed=TRUE;

						if (!Processed || Key==KEY_CTRLSHIFTF3 || Key==KEY_RCTRLSHIFTF3)
						{
							if (EnableExternal)
								ProcessExternal(Opt.strExternalViewer,strFileName,strShortFileName,PluginMode);
							else
							{
								NamesList ViewList;

								if (!PluginMode)
								{
									for (int I=0; I<FileCount; I++)
										if (!(ListData[I]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
											ViewList.AddName(ListData[I]->strName,ListData[I]->strShortName);

									ViewList.SetCurDir(strCurDir);
									ViewList.SetCurName(strFileName);
								}

								FileViewer *ShellViewer=new FileViewer(strFileName, TRUE,PluginMode,PluginMode,-1,strPluginData,&ViewList);

								if (ShellViewer)
								{
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
						        MSG(MTextSavedToTemp),strFileName,MSG(MOk));
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
						Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

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
//      CtrlObject->Cp()->Redraw();
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
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(Key)));

			ProcessCopyKeys(Key);

			return TRUE;
		}

		case KEY_ALTF5:  // Печать текущего/выбранных файла/ов
		case KEY_RALTF5:
		{
			_ALGO(CleverSysLog clv(L"Alt-F5"));
			_ALGO(SysLog(L"%s, FileCount=%d",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount));

			if (FileCount>0 && SetCurPath())
				PrintFiles(this);

			return TRUE;
		}
		case KEY_SHIFTF5:
		case KEY_SHIFTF6:
		{
			_ALGO(CleverSysLog clv(L"Shift-F5/Shift-F6"));
			_ALGO(SysLog(L"%s, FileCount=%d Key=%s",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(Key)));

			if (FileCount>0 && SetCurPath())
			{
				int OldFileCount=FileCount,OldCurFile=CurFile;
				assert(CurFile<FileCount);
				int OldSelection=ListData[CurFile]->Selected;

				int RealName=PanelMode!=PLUGIN_PANEL;
				ReturnCurrentFile=TRUE;

				if (PanelMode==PLUGIN_PANEL)
				{
					OpenPanelInfo Info;
					CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
					RealName=Info.Flags&OPIF_REALNAMES;
				}

				if (RealName)
				{
					int ToPlugin=0;
					ShellCopy ShCopy(this,Key==KEY_SHIFTF6,FALSE,TRUE,TRUE,ToPlugin,nullptr);
				}
				else
				{
					ProcessCopyKeys(Key==KEY_SHIFTF5 ? KEY_F5:KEY_F6);
				}

				ReturnCurrentFile=FALSE;

				assert(CurFile<FileCount);
				if (Key!=KEY_SHIFTF5 && FileCount==OldFileCount &&
				        CurFile==OldCurFile && OldSelection!=ListData[CurFile]->Selected)
				{
					Select(ListData[CurFile],OldSelection);
					Redraw();
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
				if (PanelMode==PLUGIN_PANEL && !CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARMAKEDIRECTORY))
				{
					string strDirName;
					const wchar_t *lpwszDirName=strDirName;
					int MakeCode=CtrlObject->Plugins->MakeDirectory(hPlugin,&lpwszDirName,0);
					strDirName=lpwszDirName;

					if (!MakeCode)
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateFolder),strDirName,MSG(MOk));

					Update(UPDATE_KEEP_SELECTION);

					if (MakeCode==1)
						GoToFile(PointToName(strDirName));

					Redraw();
					Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
					/* $ 07.09.2001 VVM
					  ! Обновить соседнюю панель с установкой на новый каталог */
//          AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
//          AnotherPanel->Redraw();

					if (AnotherPanel->GetType()!=FILE_PANEL)
					{
						AnotherPanel->SetCurDir(strCurDir,FALSE);
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
			_ALGO(CleverSysLog clv(L"F8/Shift-F8/Shift-Del/Alt-Del"));
			_ALGO(SysLog(L"%s, FileCount=%d, Key=%s",(PanelMode==PLUGIN_PANEL?"PluginPanel":"FilePanel"),FileCount,_FARKEY_ToName(Key)));

			if (FileCount>0 && SetCurPath())
			{
				if (Key==KEY_SHIFTF8)
					ReturnCurrentFile=TRUE;

				if (PanelMode==PLUGIN_PANEL &&
				        !CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARDELETEFILES))
					PluginDelete();
				else
				{
					bool SaveOpt=Opt.DeleteToRecycleBin;

					if (Key==KEY_SHIFTDEL || Key==KEY_SHIFTNUMDEL || Key==KEY_SHIFTDECIMAL)
						Opt.DeleteToRecycleBin=0;

					ShellDelete(this,Key==KEY_ALTDEL||Key==KEY_RALTDEL||Key==KEY_ALTNUMDEL||Key==KEY_RALTNUMDEL||Key==KEY_ALTDECIMAL||Key==KEY_RALTDECIMAL);
					Opt.DeleteToRecycleBin=SaveOpt;
				}

				if (Key==KEY_SHIFTF8)
					ReturnCurrentFile=FALSE;
			}

			return TRUE;
		}
		// $ 26.07.2001 VVM  С альтом скролим всегда по 1
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		case(KEY_MSWHEEL_UP | KEY_RALT):
			Scroll(Key & (KEY_ALT|KEY_RALT)?-1:(int)-Opt.MsWheelDelta);
			return TRUE;
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		case(KEY_MSWHEEL_DOWN | KEY_RALT):
			Scroll(Key & (KEY_ALT|KEY_RALT)?1:(int)Opt.MsWheelDelta);
			return TRUE;
		case KEY_MSWHEEL_LEFT:
		case(KEY_MSWHEEL_LEFT | KEY_ALT):
		case(KEY_MSWHEEL_LEFT | KEY_RALT):
		{
			int Roll = Key & (KEY_ALT|KEY_RALT)?1:(int)Opt.MsHWheelDelta;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_LEFT);

			return TRUE;
		}
		case KEY_MSWHEEL_RIGHT:
		case(KEY_MSWHEEL_RIGHT | KEY_ALT):
		case(KEY_MSWHEEL_RIGHT | KEY_RALT):
		{
			int Roll = Key & (KEY_ALT|KEY_RALT)?1:(int)Opt.MsHWheelDelta;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_RIGHT);

			return TRUE;
		}
		case KEY_HOME:         case KEY_NUMPAD7:
			Up(0x7fffff);
			return TRUE;
		case KEY_END:          case KEY_NUMPAD1:
			Down(0x7fffff);
			return TRUE;
		case KEY_UP:           case KEY_NUMPAD8:
			Up(1);
			return TRUE;
		case KEY_DOWN:         case KEY_NUMPAD2:
			Down(1);
			return TRUE;
		case KEY_PGUP:         case KEY_NUMPAD9:
			N=Columns*Height-1;
			CurTopFile-=N;
			Up(N);
			return TRUE;
		case KEY_PGDN:         case KEY_NUMPAD3:
			N=Columns*Height-1;
			CurTopFile+=N;
			Down(N);
			return TRUE;
		case KEY_LEFT:         case KEY_NUMPAD4:

			if ((Columns==1 && Opt.ShellRightLeftArrowsRule == 1) || Columns>1 || !CmdLength)
			{
				if (CurTopFile>=Height && CurFile-CurTopFile<Height)
					CurTopFile-=Height;

				Up(Height);
				return TRUE;
			}

			return FALSE;
		case KEY_RIGHT:        case KEY_NUMPAD6:

			if ((Columns==1 && Opt.ShellRightLeftArrowsRule == 1) || Columns>1 || !CmdLength)
			{
				if (CurFile+Height<FileCount && CurFile-CurTopFile>=(Columns-1)*(Height))
					CurTopFile+=Height;

				Down(Height);
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
				ProcessKey(KEY_SHIFTUP);

			ProcessKey(KEY_SHIFTUP);
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

			while (CurFile<FileCount-1)
				ProcessKey(KEY_SHIFTDOWN);

			ProcessKey(KEY_SHIFTDOWN);
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
				ProcessKey(Key==KEY_SHIFTPGUP||Key==KEY_SHIFTNUMPAD9? KEY_SHIFTUP:KEY_SHIFTDOWN);

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
			if (!FileCount)
				return TRUE;

			if (Columns>1)
			{
				N=Height;
				InternalProcessKey++;
				Lock();

				while (N--)
					ProcessKey(Key==KEY_SHIFTLEFT || Key==KEY_SHIFTNUMPAD4? KEY_SHIFTUP:KEY_SHIFTDOWN);

				assert(CurFile<FileCount);
				Select(ListData[CurFile],ShiftSelection);

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
			if (!FileCount)
				return TRUE;

			assert(CurFile<FileCount);
			CurPtr=ListData[CurFile];

			if (ShiftSelection==-1)
			{
				// .. is never selected
				if (CurFile < FileCount-1 && TestParentFolderName(CurPtr->strName))
					ShiftSelection = !ListData [CurFile+1]->Selected;
				else
					ShiftSelection=!CurPtr->Selected;
			}

			Select(CurPtr,ShiftSelection);

			if (Key==KEY_SHIFTUP || Key == KEY_SHIFTNUMPAD8)
				Up(1);
			else
				Down(1);

			if (SelectedFirst && !InternalProcessKey)
				SortFileList(TRUE);

			ShowFileList(TRUE);
			return TRUE;
		}
		case KEY_INS:          case KEY_NUMPAD0:
		{
			if (!FileCount)
				return TRUE;

			assert(CurFile<FileCount);
			CurPtr=ListData[CurFile];
			Select(CurPtr,!CurPtr->Selected);
			Down(1);

			if (SelectedFirst)
				SortFileList(TRUE);

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
			Show();
			return TRUE;
		case KEY_SHIFTF12:
			SelectedFirst=!SelectedFirst;
			SortFileList(TRUE);
			Show();
			return TRUE;
		case KEY_CTRLPGUP:     case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP:    case KEY_RCTRLNUMPAD9:
		{
			if (Opt.PgUpChangeDisk || PanelMode==PLUGIN_PANEL || !IsRootPath(strCurDir))
			{
				//"this" может быть удалён в ChangeDir
				bool CheckFullScreen=IsFullScreen();
				ChangeDir(L"..");
				Panel *NewActivePanel = CtrlObject->Cp()->ActivePanel;
				NewActivePanel->SetViewMode(NewActivePanel->GetViewMode());

				if (CheckFullScreen!=NewActivePanel->IsFullScreen())
					CtrlObject->Cp()->GetAnotherPanel(NewActivePanel)->Show();

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
			ProcessEnter(0,0,!(Key&KEY_SHIFT), false, OFP_ALTERNATIVE);
			return TRUE;

		case KEY_APPS:
		case KEY_SHIFTAPPS:
		{
			//вызовем EMenu если он есть
			if (CtrlObject->Plugins->FindPlugin(Opt.KnownIDs.Emenu))
			{
				CtrlObject->Plugins->CallPlugin(Opt.KnownIDs.Emenu, OPEN_FILEPANEL, reinterpret_cast<void*>(static_cast<intptr_t>(1))); // EMenu Plugin :-)
			}
			return TRUE;
		}

		default:

			if (((Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+65535) || (Key>=KEY_RALT_BASE+0x01 && Key<=KEY_RALT_BASE+65535) ||
			        (Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+65535) || (Key>=KEY_RALTSHIFT_BASE+0x01 && Key<=KEY_RALTSHIFT_BASE+65535)) &&
			        (Key&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_BS && (Key&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_TAB &&
			        (Key&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_ENTER && (Key&~(KEY_ALT|KEY_RALT|KEY_SHIFT))!=KEY_ESC &&
			        !(Key&EXTENDED_KEY_BASE)
			   )
			{
				//_SVS(SysLog(L">FastFind: Key=%s",_FARKEY_ToName(Key)));
				// Скорректирем уже здесь нужные клавиши, т.к. WaitInFastFind
				// в это время еще равно нулю.
				static const char Code[]=")!@#$%^&*(";

				if (Key >= KEY_ALTSHIFT0 && Key <= KEY_ALTSHIFT9)
					Key=(DWORD)Code[Key-KEY_ALTSHIFT0];
				else if (Key >= KEY_RALTSHIFT0 && Key <= KEY_RALTSHIFT9)
					Key=(DWORD)Code[Key-KEY_RALTSHIFT0];
				else if ((Key&(~(KEY_ALT|KEY_RALT|KEY_SHIFT))) == '/')
					Key='?';
				else if ((Key == KEY_ALTSHIFT+'-') || (Key == KEY_RALT+KEY_SHIFT+'-'))
					Key='_';
				else if ((Key == KEY_ALTSHIFT+'=') || (Key == KEY_RALT+KEY_SHIFT+'='))
					Key='+';

				//_SVS(SysLog(L"<FastFind: Key=%s",_FARKEY_ToName(Key)));
				FastFind(Key);
			}
			else
				break;

			return TRUE;
	}

	return FALSE;
}


void FileList::Select(FileListItem *SelPtr,int Selection)
{
	if (!TestParentFolderName(SelPtr->strName) && SelPtr->Selected!=Selection)
	{
		CacheSelIndex=-1;
		CacheSelClearIndex=-1;

		if ((SelPtr->Selected=Selection))
		{
			SelFileCount++;
			SelFileSize += SelPtr->FileSize;
		}
		else
		{
			SelFileCount--;
			SelFileSize -= SelPtr->FileSize;
		}
	}
}


void FileList::ProcessEnter(bool EnableExec,bool SeparateWindow,bool EnableAssoc, bool RunAs, OPENFILEPLUGINTYPE Type)
{
	FileListItem *CurPtr;
	string strFileName, strShortFileName;
	const wchar_t *ExtPtr;

	if (CurFile>=FileCount)
		return;

	CurPtr=ListData[CurFile];
	strFileName = CurPtr->strName;

	if (!CurPtr->strShortName.IsEmpty())
		strShortFileName = CurPtr->strShortName;
	else
		strShortFileName = CurPtr->strName;

	if (CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
		BOOL IsRealName=FALSE;

		if (PanelMode==PLUGIN_PANEL)
		{
			OpenPanelInfo Info;
			CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
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
			Execute(strFullPath, false, SeparateWindow, true, (CurPtr->FileAttr&FILE_ATTRIBUTE_DIRECTORY)!=0);
		}
		else
		{
			bool CheckFullScreen=IsFullScreen();

			if (PanelMode==PLUGIN_PANEL || !wcschr(CurPtr->strName,L'?') || CurPtr->strShortName.IsEmpty())
			{
				ChangeDir(CurPtr->strName);
			}
			else
			{
				ChangeDir(CurPtr->strShortName);
			}

			//"this" может быть удалён в ChangeDir
			Panel *ActivePanel = CtrlObject->Cp()->ActivePanel;

			if (CheckFullScreen!=ActivePanel->IsFullScreen())
			{
				CtrlObject->Cp()->GetAnotherPanel(ActivePanel)->Show();
			}

			ActivePanel->Show();
		}
	}
	else
	{
		bool PluginMode=PanelMode==PLUGIN_PANEL && !CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARGETFILE);

		if (PluginMode)
		{
			string strTempDir;

			if (!FarMkTempEx(strTempDir))
				return;

			apiCreateDirectory(strTempDir,nullptr);
			PluginPanelItem PanelItem;
			FileListToPluginItem(CurPtr,&PanelItem);
			int Result=CtrlObject->Plugins->GetFile(hPlugin,&PanelItem,strTempDir,strFileName,OPM_SILENT|OPM_VIEW);
			FreePluginPanelItem(&PanelItem);

			if (!Result)
			{
				apiRemoveDirectory(strTempDir);
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

		ExtPtr = wcsrchr(strFileName,L'.');
		int ExeType=FALSE,BatType=FALSE;

		if (ExtPtr)
		{
			ExeType=!StrCmpI(ExtPtr, L".exe") || !StrCmpI(ExtPtr, L".com");
			BatType=IsBatchExtType(ExtPtr);
		}

		if (EnableExec && (ExeType || BatType))
		{
			QuoteSpace(strFileName);

			if (!(Opt.ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTPANEL) && !PluginMode) //AN
				CtrlObject->CmdHistory->AddToHistory(strFileName);

			CtrlObject->CmdLine->ExecString(strFileName, PluginMode, SeparateWindow, true, false, false, RunAs);

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
					if (SeparateWindow || Opt.UseRegisteredTypes)
						ProcessGlobalFileTypes(strFileName, PluginMode, RunAs);

				if (PluginMode)
				{
					DeleteFileWithFolder(strFileName);
				}
			}

			return;
		}
	}
}


BOOL FileList::SetCurDir(const string& NewDir,int ClosePanel,BOOL IsUpdated)
{
	bool CheckFullScreen=false;

	if (ClosePanel && PanelMode==PLUGIN_PANEL)
	{
		CheckFullScreen=IsFullScreen();
		OpenPanelInfo Info;
		CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		string strInfoHostFile=Info.HostFile;

		for (;;)
		{
			if (ProcessPluginEvent(FE_CLOSE,nullptr))
				return FALSE;

			if (!PopPlugin(TRUE))
				break;

			if (NewDir.IsEmpty())
			{
				Update(0);
				PopPrevData(strInfoHostFile,true,true,true,true);
				break;
			}
		}

		CtrlObject->Cp()->RedrawKeyBar();

		if (CheckFullScreen!=IsFullScreen())
		{
			CtrlObject->Cp()->GetAnotherPanel(this)->Redraw();
		}
	}

	if (!NewDir.IsEmpty())
	{
		return ChangeDir(NewDir,IsUpdated);
	}

	return FALSE;
}

BOOL FileList::ChangeDir(const wchar_t *NewDir,BOOL IsUpdated)
{
	string strFindDir, strSetDir;

	if (PanelMode!=PLUGIN_PANEL && !IsAbsolutePath(NewDir) && !TestCurrentDirectory(strCurDir))
		FarChDir(strCurDir);

	strSetDir = NewDir;
	bool dot2Present = !StrCmp(strSetDir, L"..");

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

		PrepareDiskPath(strSetDir);

		if (!StrCmpN(strSetDir, L"\\\\?\\", 4) && strSetDir.At(5) == L':' && !strSetDir.At(6))
			AddEndSlash(strSetDir);
	}

	if (!dot2Present && StrCmp(strSetDir,L"\\"))
		UpperFolderTopFile=CurTopFile;

	if (SelFileCount>0)
		ClearSelection();

	bool PluginClosed=false,GoToPanelFile=false;

	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		/* $ 16.01.2002 VVM
		  + Если у плагина нет OPIF_REALNAMES, то история папок не пишется в реестр */
		string strInfoCurDir=Info.CurDir;
		//string strInfoFormat=Info.Format;
		string strInfoHostFile=Info.HostFile;
		string strInfoData=Info.ShortcutData;
		if(Info.Flags&OPIF_SHORTCUT) CtrlObject->FolderHistory->AddToHistory(strInfoCurDir,0,&PluginManager::GetGUID(hPlugin),strInfoHostFile,strInfoData);
		/* $ 25.04.01 DJ
		   при неудаче SetDirectory не сбрасываем выделение
		*/
		bool SetDirectorySuccess = true;

		if (dot2Present && (strInfoCurDir.IsEmpty() || !StrCmp(strInfoCurDir,L"\\")))
		{
			if (ProcessPluginEvent(FE_CLOSE,nullptr))
				return TRUE;

			PluginClosed=true;
			strFindDir = strInfoHostFile;

			if (strFindDir.IsEmpty() && (Info.Flags & OPIF_REALNAMES) && CurFile<FileCount)
			{
				strFindDir = ListData[CurFile]->strName;
				GoToPanelFile=true;
			}

			PopPlugin(TRUE);
			Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

			if (AnotherPanel->GetType()==INFO_PANEL)
				AnotherPanel->Redraw();
		}
		else
		{
			strFindDir = strInfoCurDir;
			SetDirectorySuccess=CtrlObject->Plugins->SetDirectory(hPlugin,strSetDir,0) != FALSE;
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
				CtrlObject->FolderHistory->AddToHistory(strCurDir);
		}

		if (dot2Present)
		{
			if (RootPath)
			{
				if (NetPath)
				{
					if (CtrlObject->Plugins->CallPlugin(Opt.KnownIDs.Network,OPEN_FILEPANEL,(void*)strCurDir.CPtr())) // NetWork Plugin :-)
					{
						return FALSE;
					}
				}
				if(DrivePath && Opt.PgUpChangeDisk == 2)
				{
					string RemoteName;
					if(DriveLocalToRemoteName(DRIVE_REMOTE, strCurDir.At(0), RemoteName))
					{
						if (CtrlObject->Plugins->CallPlugin(Opt.KnownIDs.Network,OPEN_FILEPANEL,(void*)RemoteName.CPtr())) // NetWork Plugin :-)
						{
							return FALSE;
						}
					}
				}
				CtrlObject->Cp()->ActivePanel->ChangeDisk();
				return TRUE;
			}
		}
	}

	strFindDir = PointToName(strCurDir);
	/*
		// вот и зачем это? мы уже и так здесь, в strCurDir
		// + дальше по тексту strSetDir уже содержит полный путь
		if ( strSetDir.IsEmpty() || strSetDir.At(1) != L':' || !IsSlash(strSetDir.At(2)))
			FarChDir(strCurDir);
	*/
	/* $ 26.04.2001 DJ
	   проверяем, удалось ли сменить каталог, и обновляем с KEEP_SELECTION,
	   если не удалось
	*/
	int UpdateFlags = 0;
	BOOL SetDirectorySuccess = TRUE;

	if (PanelMode!=PLUGIN_PANEL && !StrCmp(strSetDir,L"\\"))
	{
		strSetDir = ExtractPathRoot(strCurDir);
	}

	if (!FarChDir(strSetDir))
	{
		if (FrameManager && FrameManager->ManagerStarted())
		{
			/* $ 03.11.2001 IS Укажем имя неудачного каталога */
			Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG(MError), (dot2Present?L"..":strSetDir), MSG(MOk));
			UpdateFlags = UPDATE_KEEP_SELECTION;
		}

		SetDirectorySuccess=FALSE;
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
	apiGetCurrentDirectory(strCurDir);

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
		CtrlObject->CmdLine->SetCurDir(strCurDir);
		CtrlObject->CmdLine->Show();
	}

	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->GetType()!=FILE_PANEL)
	{
		AnotherPanel->SetCurDir(strCurDir,FALSE);
		AnotherPanel->Redraw();
	}

	if (PanelMode==PLUGIN_PANEL)
		CtrlObject->Cp()->RedrawKeyBar();

	return SetDirectorySuccess;
}


int FileList::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	Elevation.ResetApprove();

	FileListItem *CurPtr;
	int RetCode;

	if (IsVisible() && Opt.ShowColumnTitles && !MouseEvent->dwEventFlags &&
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

	if (IsVisible() && Opt.ShowPanelScrollbar && IntKeyState.MouseX==X2 &&
	        (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && !(MouseEvent->dwEventFlags & MOUSE_MOVED) && !IsDragging())
	{
		int ScrollY=Y1+1+Opt.ShowColumnTitles;

		if (IntKeyState.MouseY==ScrollY)
		{
			while (IsMouseButtonPressed())
				ProcessKey(KEY_UP);

			SetFocus();
			return TRUE;
		}

		if (IntKeyState.MouseY==ScrollY+Height-1)
		{
			while (IsMouseButtonPressed())
				ProcessKey(KEY_DOWN);

			SetFocus();
			return TRUE;
		}

		if (IntKeyState.MouseY>ScrollY && IntKeyState.MouseY<ScrollY+Height-1 && Height>2)
		{
			while (IsMouseButtonPressed())
			{
				CurFile=(FileCount-1)*(IntKeyState.MouseY-ScrollY)/(Height-2);
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
		ProcessKey(Key);
		return TRUE;
	}

	if (Panel::PanelProcessMouse(MouseEvent,RetCode))
		return(RetCode);

	if (MouseEvent->dwMousePosition.Y>Y1+Opt.ShowColumnTitles &&
	        MouseEvent->dwMousePosition.Y<Y2-2*Opt.ShowPanelStatus)
	{
		SetFocus();

		if (!FileCount)
			return TRUE;

		MoveToMouse(MouseEvent);
		assert(CurFile<FileCount);
		CurPtr=ListData[CurFile];

		if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
		        MouseEvent->dwEventFlags==DOUBLE_CLICK)
		{
			if (PanelMode==PLUGIN_PANEL)
			{
				FlushInputBuffer(); // !!!
				INPUT_RECORD rec;
				ProcessKeyToInputRecord(VK_RETURN,IntKeyState.ShiftPressed ? PKF_SHIFT:0,&rec);
				int ProcessCode=CtrlObject->Plugins->ProcessKey(hPlugin,&rec,false);
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
			ProcessEnter(true,IntKeyState.ShiftPressed!=0);
			return TRUE;
		}
		else
		{
			/* $ 11.09.2000 SVS
			   Bug #17: Выделяем при условии, что колонка ПОЛНОСТЬЮ пуста.
			*/
			if ((MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) && !IsEmpty)
			{
				DWORD control=MouseEvent->dwControlKeyState&(SHIFT_PRESSED|LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED);

				//вызовем EMenu если он есть
				if (MouseEvent->dwButtonState == RIGHTMOST_BUTTON_PRESSED && (control==0 || control==SHIFT_PRESSED) && CtrlObject->Plugins->FindPlugin(Opt.KnownIDs.Emenu))
				{
					ShowFileList(TRUE);
					CtrlObject->Plugins->CallPlugin(Opt.KnownIDs.Emenu,OPEN_FILEPANEL,nullptr); // EMenu Plugin :-)
					return TRUE;
				}

				if (!MouseEvent->dwEventFlags || MouseEvent->dwEventFlags==DOUBLE_CLICK)
					MouseSelection=!CurPtr->Selected;

				Select(CurPtr,MouseSelection);

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

		if (!FileCount)
			return TRUE;

		while (IsMouseButtonPressed() && IntKeyState.MouseY<=Y1+1)
		{
			Up(1);

			if (IntKeyState.MouseButtonState==RIGHTMOST_BUTTON_PRESSED)
			{
				assert(CurFile<FileCount);
				CurPtr=ListData[CurFile];
				Select(CurPtr,MouseSelection);
			}
		}

		if (SelectedFirst)
			SortFileList(TRUE);

		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y>=Y2-2)
	{
		SetFocus();

		if (!FileCount)
			return TRUE;

		while (IsMouseButtonPressed() && IntKeyState.MouseY>=Y2-2)
		{
			Down(1);

			if (IntKeyState.MouseButtonState==RIGHTMOST_BUTTON_PRESSED)
			{
				assert(CurFile<FileCount);
				CurPtr=ListData[CurFile];
				Select(CurPtr,MouseSelection);
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
void FileList::MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	int CurColumn=1,ColumnsWidth,I;
	int PanelX=MouseEvent->dwMousePosition.X-X1-1;
	int Level = 0;

	for (ColumnsWidth=I=0; I<ViewSettings.ColumnCount; I++)
	{
		if (Level == ColumnsInGlobal)
		{
			CurColumn++;
			Level = 0;
		}

		ColumnsWidth+=ViewSettings.ColumnWidth[I];

		if (ColumnsWidth>=PanelX)
			break;

		ColumnsWidth++;
		Level++;
	}

//  if (!CurColumn)
//    CurColumn=1;
	int OldCurFile=CurFile;
	CurFile=CurTopFile+MouseEvent->dwMousePosition.Y-Y1-1-Opt.ShowColumnTitles;

	if (CurColumn>1)
		CurFile+=(CurColumn-1)*Height;

	CorrectPosition();

	/* $ 11.09.2000 SVS
	   Bug #17: Проверим на ПОЛНОСТЬЮ пустую колонку.
	*/
	if (Opt.PanelRightClickRule == 1)
		IsEmpty=((CurColumn-1)*Height > FileCount);
	else if (Opt.PanelRightClickRule == 2 &&
	         (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) &&
	         ((CurColumn-1)*Height > FileCount))
	{
		CurFile=OldCurFile;
		IsEmpty=TRUE;
	}
	else
		IsEmpty=FALSE;
}

void FileList::SetViewMode(int ViewMode)
{
	if ((DWORD)ViewMode > (DWORD)SizeViewSettingsArray)
		ViewMode=VIEW_0;

	bool CurFullScreen=IsFullScreen();
	int OldOwner=IsColumnDisplayed(OWNER_COLUMN);
	int OldPacked=IsColumnDisplayed(PACKED_COLUMN);
	int OldNumLink=IsColumnDisplayed(NUMLINK_COLUMN);
	int OldNumStreams=IsColumnDisplayed(NUMSTREAMS_COLUMN);
	int OldStreamsSize=IsColumnDisplayed(STREAMSSIZE_COLUMN);
	int OldDiz=IsColumnDisplayed(DIZ_COLUMN);
	PrepareViewSettings(ViewMode,nullptr);
	int NewOwner=IsColumnDisplayed(OWNER_COLUMN);
	int NewPacked=IsColumnDisplayed(PACKED_COLUMN);
	int NewNumLink=IsColumnDisplayed(NUMLINK_COLUMN);
	int NewNumStreams=IsColumnDisplayed(NUMSTREAMS_COLUMN);
	int NewStreamsSize=IsColumnDisplayed(STREAMSSIZE_COLUMN);
	int NewDiz=IsColumnDisplayed(DIZ_COLUMN);
	int NewAccessTime=IsColumnDisplayed(ADATE_COLUMN);
	int ResortRequired=FALSE;
	string strDriveRoot;
	DWORD FileSystemFlags = 0;
	GetPathRoot(strCurDir,strDriveRoot);

	if (NewPacked && apiGetVolumeInformation(strDriveRoot,nullptr,nullptr,nullptr,&FileSystemFlags,nullptr))
		if (!(FileSystemFlags&FILE_FILE_COMPRESSION))
			NewPacked=FALSE;

	if (FileCount>0 && PanelMode!=PLUGIN_PANEL &&
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

		FileList::ViewMode=ViewMode;
	}
	else
	{
		if (!(ViewSettings.Flags&PVS_FULLSCREEN) && CurFullScreen)
		{
			if (Y2>0)
			{
				if (this==CtrlObject->Cp()->LeftPanel)
					SetPosition(0,Y1,ScrX/2-Opt.WidthDecrement,Y2);
				else
					SetPosition(ScrX/2+1-Opt.WidthDecrement,Y1,ScrX,Y2);
			}

			FileList::ViewMode=ViewMode;
		}
		else
		{
			FileList::ViewMode=ViewMode;
			FrameManager->RefreshFrame();
		}
	}

	if (PanelMode==PLUGIN_PANEL)
	{
		string strColumnTypes,strColumnWidths;
//    SetScreenPosition();
		ViewSettingsToText(ViewSettings.ColumnType,ViewSettings.ColumnWidth,ViewSettings.ColumnWidthType,
		                   ViewSettings.ColumnCount,strColumnTypes,strColumnWidths);
		ProcessPluginEvent(FE_CHANGEVIEWMODE,(void*)strColumnTypes.CPtr());
	}

	if (ResortRequired)
	{
		SortFileList(TRUE);
		ShowFileList(TRUE);
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

		if (AnotherPanel->GetType()==TREE_PANEL)
			AnotherPanel->Redraw();
	}
}


void FileList::SetSortMode(int SortMode)
{
	if (SortMode==FileList::SortMode && Opt.ReverseSort)
		SortOrder=-SortOrder;
	else
		SortOrder=1;

	SetSortMode0(SortMode);
}

void FileList::SetSortMode0(int SortMode)
{
	FileList::SortMode=SortMode;

	if (FileCount>0)
		SortFileList(TRUE);

	FrameManager->RefreshFrame();
}

void FileList::ChangeNumericSort(int Mode)
{
	Panel::ChangeNumericSort(Mode);
	SortFileList(TRUE);
	Show();
}

void FileList::ChangeCaseSensitiveSort(int Mode)
{
	Panel::ChangeCaseSensitiveSort(Mode);
	SortFileList(TRUE);
	Show();
}

void FileList::ChangeDirectoriesFirst(int Mode)
{
	Panel::ChangeDirectoriesFirst(Mode);
	SortFileList(TRUE);
	Show();
}

int FileList::GoToFile(long idxItem)
{
	if ((DWORD)idxItem < (DWORD)FileCount)
	{
		CurFile=idxItem;
		CorrectPosition();
		return TRUE;
	}

	return FALSE;
}

int FileList::GoToFile(const wchar_t *Name,BOOL OnlyPartName)
{
	return GoToFile(FindFile(Name,OnlyPartName));
}


long FileList::FindFile(const wchar_t *Name,BOOL OnlyPartName)
{
   long II = -1;
	for (long I=0; I < FileCount; I++)
	{
		const wchar_t *CurPtrName=OnlyPartName?PointToName(ListData[I]->strName):ListData[I]->strName.CPtr();

		if (!StrCmp(Name,CurPtrName))
			return I;

		if (II < 0 && !StrCmpI(Name,CurPtrName))
			II = I;
	}

	return II;
}

long FileList::FindFirst(const wchar_t *Name)
{
	return FindNext(0,Name);
}

long FileList::FindNext(int StartPos, const wchar_t *Name)
{
	if ((DWORD)StartPos < (DWORD)FileCount)
		for (long I=StartPos; I < FileCount; I++)
		{
			if (CmpName(Name,ListData[I]->strName,true))
				if (!TestParentFolderName(ListData[I]->strName))
					return I;
		}

	return -1;
}


int FileList::IsSelected(const wchar_t *Name)
{
	long Pos=FindFile(Name);
	return(Pos!=-1 && (ListData[Pos]->Selected || (!SelFileCount && Pos==CurFile)));
}

int FileList::IsSelected(long idxItem)
{
	if ((DWORD)idxItem < (DWORD)FileCount)
		return(ListData[idxItem]->Selected); //  || (Sel!FileCount && idxItem==CurFile) ???
	return FALSE;
}

bool FileList::FilterIsEnabled()
{
	return Filter && Filter->IsEnabledOnPanel()?true:false;
}

bool FileList::FileInFilter(long idxItem)
{
	if ( ( (DWORD)idxItem < (DWORD)FileCount ) && ( !Filter || !Filter->IsEnabledOnPanel() || Filter->FileInFilter(*ListData[idxItem]) ) )
		return true;
	return false;
}

// $ 02.08.2000 IG  Wish.Mix #21 - при нажатии '/' или '\' в QuickSerach переходим на директорию
int FileList::FindPartName(const wchar_t *Name,int Next,int Direct,int ExcludeSets)
{
#if !defined(Mantis_698)
	int DirFind = 0;
	int Length = StrLength(Name);
	string strMask;
	strMask = Name;

	if (Length > 0 && IsSlash(Name[Length-1]))
	{
		DirFind = 1;
		strMask.SetLength(strMask.GetLength()-1);
	}

	strMask += L"*";

	if (ExcludeSets)
	{
		ReplaceStrings(strMask,L"[",L"<[%>",-1,true);
		ReplaceStrings(strMask,L"]",L"[]]",-1,true);
		ReplaceStrings(strMask,L"<[%>",L"[[]",-1,true);
	}

	for (int I=CurFile+(Next?Direct:0); I >= 0 && I < FileCount; I+=Direct)
	{
		if (CmpName(strMask,ListData[I]->strName,true,I==CurFile))
		{
			if (!TestParentFolderName(ListData[I]->strName))
			{
				if (!DirFind || (ListData[I]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				{
					CurFile=I;
					CurTopFile=CurFile-(Y2-Y1)/2;
					ShowFileList(TRUE);
					return TRUE;
				}
			}
		}
	}

	for (int I=(Direct > 0)?0:FileCount-1; (Direct > 0) ? I < CurFile:I > CurFile; I+=Direct)
	{
		if (CmpName(strMask,ListData[I]->strName,true))
		{
			if (!TestParentFolderName(ListData[I]->strName))
			{
				if (!DirFind || (ListData[I]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
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
	int Length = StrLength(Name);
	string strMask;
	strMask = Name;
	strMask.Upper();

	if (Length > 0 && IsSlash(Name[Length-1]))
	{
		DirFind = 1;
		strMask.SetLength(strMask.GetLength()-1);
	}

/*
	strMask += L"*";

	if (ExcludeSets)
	{
		ReplaceStrings(strMask,L"[",L"<[%>",-1,true);
		ReplaceStrings(strMask,L"]",L"[]]",-1,true);
		ReplaceStrings(strMask,L"<[%>",L"[[]",-1,true);
	}
*/

	for (int I=CurFile+(Next?Direct:0); I >= 0 && I < FileCount; I+=Direct)
	{
		if (GetPlainString(Dest,I) && Dest.Upper().Contains(strMask))
		//if (CmpName(strMask,ListData[I]->strName,true,I==CurFile))
		{
			if (!TestParentFolderName(ListData[I]->strName))
			{
				if (!DirFind || (ListData[I]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				{
					CurFile=I;
					CurTopFile=CurFile-(Y2-Y1)/2;
					ShowFileList(TRUE);
					return TRUE;
				}
			}
		}
	}

	for (int I=(Direct > 0)?0:FileCount-1; (Direct > 0) ? I < CurFile:I > CurFile; I+=Direct)
	{
		if (GetPlainString(Dest,I) && Dest.Upper().Contains(strMask))
		{
			if (!TestParentFolderName(ListData[I]->strName))
			{
				if (!DirFind || (ListData[I]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
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
bool FileList::GetPlainString(string& Dest,int ListPos)
{
	Dest=L"";
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

				if (ColumnNumber<ListData[ListPos]->CustomColumnNumber)
					ColumnData=ListData[ListPos]->CustomColumnData[ColumnNumber];

				if (!ColumnData)
				{
					ColumnData=ListData[ListPos]->strCustomData;//L"";
				}
				Dest.Append(ColumnData);
			}
			else
			{
				switch (ColumnType)
				{
					case NAME_COLUMN:
					{
						unsigned __int64 ViewFlags=ColumnTypes[K];
						const wchar_t *NamePtr = ShowShortNames && !ListData[ListPos]->strShortName.IsEmpty() ? ListData[ListPos]->strShortName:ListData[ListPos]->strName;

						string strNameCopy;
						if (!(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY) && (ViewFlags & COLUMN_NOEXTENSION))
						{
							const wchar_t *ExtPtr = PointToExt(NamePtr);
							if (ExtPtr)
							{
								strNameCopy.Copy(NamePtr, ExtPtr-NamePtr);
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

						Dest.Append(NamePtr);
						break;
					}

					case EXTENSION_COLUMN:
					{
						const wchar_t *ExtPtr = nullptr;
						if (!(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
						{
							const wchar_t *NamePtr = ShowShortNames && !ListData[ListPos]->strShortName.IsEmpty()? ListData[ListPos]->strShortName:ListData[ListPos]->strName;
							ExtPtr = PointToExt(NamePtr);
						}
						if (ExtPtr && *ExtPtr) ExtPtr++; else ExtPtr = L"";

						Dest.Append(ExtPtr);
						break;
					}

					case SIZE_COLUMN:
					case PACKED_COLUMN:
					case STREAMSSIZE_COLUMN:
					{
						Dest.Append(FormatStr_Size(
							ListData[ListPos]->FileSize,
							ListData[ListPos]->AllocationSize,
							ListData[ListPos]->StreamsSize,
							ListData[ListPos]->strName,
							ListData[ListPos]->FileAttr,
							ListData[ListPos]->ShowFolderSize,
							ListData[ListPos]->ReparseTag,
							ColumnType,
							ColumnTypes[K],
							ColumnWidth,
							strCurDir.CPtr()));
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
								FileTime=&ListData[ListPos]->CreationTime;
								break;
							case ADATE_COLUMN:
								FileTime=&ListData[ListPos]->AccessTime;
								break;
							case CHDATE_COLUMN:
								FileTime=&ListData[ListPos]->ChangeTime;
								break;
							case DATE_COLUMN:
							case TIME_COLUMN:
							case WDATE_COLUMN:
							default:
								FileTime=&ListData[ListPos]->WriteTime;
								break;
						}

						Dest.Append(FormatStr_DateTime(FileTime,ColumnType,ColumnTypes[K],ColumnWidth));
						break;
					}

					case ATTR_COLUMN:
					{
						Dest.Append(FormatStr_Attribute(ListData[ListPos]->FileAttr,ColumnWidth));
						break;
					}

					case DIZ_COLUMN:
					{
						string strDizText=ListData[ListPos]->DizText ? ListData[ListPos]->DizText:L"";
						Dest.Append(strDizText);
						break;
					}

					case OWNER_COLUMN:
					{
						Dest.Append(ListData[ListPos]->strOwner);
						break;
					}

					case NUMLINK_COLUMN:
					{
						string s;
						s.Format(L"%d",ListData[ListPos]->NumberOfLinks);
						Dest.Append(s);
						break;
					}

					case NUMSTREAMS_COLUMN:
					{
						string s;
						s.Format(L"%d",ListData[ListPos]->NumberOfStreams);
						Dest.Append(s);
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

size_t FileList::GetSelCount()
{
	assert(!FileCount || !(ReturnCurrentFile||!SelFileCount) || (CurFile<FileCount));
	return FileCount?((ReturnCurrentFile||!SelFileCount)?(TestParentFolderName(ListData[CurFile]->strName)?0:1):SelFileCount):0;
}

size_t FileList::GetRealSelCount()
{
	return FileCount?SelFileCount:0;
}


int FileList::GetSelName(string *strName,DWORD &FileAttr,string *strShortName,FAR_FIND_DATA_EX *fde)
{
	if (!strName)
	{
		GetSelPosition=0;
		LastSelPosition=-1;
		return TRUE;
	}

	if (!SelFileCount || ReturnCurrentFile)
	{
		if (!GetSelPosition && CurFile<FileCount)
		{
			GetSelPosition=1;
			*strName = ListData[CurFile]->strName;

			if (strShortName)
			{
				*strShortName = ListData[CurFile]->strShortName;

				if (strShortName->IsEmpty())
					*strShortName = *strName;
			}

			FileAttr=ListData[CurFile]->FileAttr;
			LastSelPosition=CurFile;

			if (fde)
			{
				fde->dwFileAttributes=ListData[CurFile]->FileAttr;
				fde->ftCreationTime=ListData[CurFile]->CreationTime;
				fde->ftLastAccessTime=ListData[CurFile]->AccessTime;
				fde->ftLastWriteTime=ListData[CurFile]->WriteTime;
				fde->ftChangeTime=ListData[CurFile]->ChangeTime;
				fde->nFileSize=ListData[CurFile]->FileSize;
				fde->nAllocationSize=ListData[CurFile]->AllocationSize;
				fde->strFileName = ListData[CurFile]->strName;
				fde->strAlternateFileName = ListData[CurFile]->strShortName;
			}

			return TRUE;
		}
		else
			return FALSE;
	}

	while (GetSelPosition<FileCount)
		if (ListData[GetSelPosition++]->Selected)
		{
			*strName = ListData[GetSelPosition-1]->strName;

			if (strShortName)
			{
				*strShortName = ListData[GetSelPosition-1]->strShortName;

				if (strShortName->IsEmpty())
					*strShortName = *strName;
			}

			FileAttr=ListData[GetSelPosition-1]->FileAttr;
			LastSelPosition=GetSelPosition-1;

			if (fde)
			{
				fde->dwFileAttributes=ListData[GetSelPosition-1]->FileAttr;
				fde->ftCreationTime=ListData[GetSelPosition-1]->CreationTime;
				fde->ftLastAccessTime=ListData[GetSelPosition-1]->AccessTime;
				fde->ftLastWriteTime=ListData[GetSelPosition-1]->WriteTime;
				fde->ftChangeTime=ListData[GetSelPosition-1]->ChangeTime;
				fde->nFileSize=ListData[GetSelPosition-1]->FileSize;
				fde->nAllocationSize=ListData[GetSelPosition-1]->AllocationSize;
				fde->strFileName = ListData[GetSelPosition-1]->strName;
				fde->strAlternateFileName = ListData[GetSelPosition-1]->strShortName;
			}

			return TRUE;
		}

	return FALSE;
}


void FileList::ClearLastGetSelection()
{
	if (LastSelPosition>=0 && LastSelPosition<FileCount)
		Select(ListData[LastSelPosition],0);
}


void FileList::UngetSelName()
{
	GetSelPosition=LastSelPosition;
}


unsigned __int64 FileList::GetLastSelectedSize()
{
	if (LastSelPosition>=0 && LastSelPosition<FileCount)
		return ListData[LastSelPosition]->FileSize;

	return (unsigned __int64)(-1);
}


int FileList::GetLastSelectedItem(FileListItem *LastItem)
{
	if (LastSelPosition>=0 && LastSelPosition<FileCount)
	{
		*LastItem=*ListData[LastSelPosition];
		return TRUE;
	}

	return FALSE;
}

int FileList::GetCurName(string &strName, string &strShortName)
{
	if (!FileCount)
	{
		strName.Clear();
		strShortName.Clear();
		return FALSE;
	}

	assert(CurFile<FileCount);
	strName = ListData[CurFile]->strName;
	strShortName = ListData[CurFile]->strShortName;

	if (strShortName.IsEmpty())
		strShortName = strName;

	return TRUE;
}

int FileList::GetCurBaseName(string &strName, string &strShortName)
{
	if (!FileCount)
	{
		strName.Clear();
		strShortName.Clear();
		return FALSE;
	}

	if (PanelMode==PLUGIN_PANEL && !PluginsList.Empty()) // для плагинов
	{
		strName = PointToName((*PluginsList.First())->strHostFile);
	}
	else if (PanelMode==NORMAL_PANEL)
	{
		assert(CurFile<FileCount);
		strName = ListData[CurFile]->strName;
		strShortName = ListData[CurFile]->strShortName;
	}

	if (strShortName.IsEmpty())
		strShortName = strName;

	return TRUE;
}

long FileList::SelectFiles(int Mode,const wchar_t *Mask)
{
	CFileMask FileMask; // Класс для работы с масками
	const wchar_t *HistoryName=L"Masks";
	FarDialogItem SelectDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,51,5,0,nullptr,nullptr,0,L""},
		{DI_EDIT,5,2,49,2,0,HistoryName,nullptr,DIF_FOCUS|DIF_HISTORY,L""},
		{DI_TEXT,0,3,0,3,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,4,0,4,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,4,0,4,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MSelectFilter)},
		{DI_BUTTON,0,4,0,4,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	MakeDialogItemsEx(SelectDlgData,SelectDlg);
	FileFilter Filter(this,FFT_SELECT);
	bool bUseFilter = false;
	FileListItem *CurPtr;
	static string strPrevMask=L"*.*";
	/* $ 20.05.2002 IS
	   При обработке маски, если работаем с именем файла на панели,
	   берем каждую квадратную скобку в имени при образовании маски в скобки,
	   чтобы подобные имена захватывались полученной маской - это специфика,
	   диктуемая CmpName.
	*/
	string strMask=L"*.*", strRawMask;
	bool WrapBrackets=false; // говорит о том, что нужно взять кв.скобки в скобки

	if (CurFile>=FileCount)
		return 0;

	int RawSelection=FALSE;

	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		RawSelection=(Info.Flags & OPIF_RAWSELECTION);
	}

	CurPtr=ListData[CurFile];
	string strCurName=(ShowShortNames && !CurPtr->strShortName.IsEmpty() ? CurPtr->strShortName:CurPtr->strName);

	if (Mode==SELECT_ADDEXT || Mode==SELECT_REMOVEEXT)
	{
		size_t pos;

		if (strCurName.RPos(pos,L'.'))
		{
			// Учтем тот момент, что расширение может содержать символы-разделители
			strRawMask.Format(L"\"*.%s\"", strCurName.CPtr()+pos+1);
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
			size_t pos;

			if (strRawMask.RPos(pos,L'.') && pos!=strRawMask.GetLength()-1)
				strRawMask.SetLength(pos);

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
					Dialog Dlg(SelectDlg,ARRAYSIZE(SelectDlg));
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

						if (FileMask.Set(strMask, 0)) // Проверим вводимые пользователем маски на ошибки
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

				if (!FileMask.Set(strMask, 0)) // Проверим маски на ошибки
					return 0;
			}
		}
	}

	SaveSelection();

	if (!bUseFilter && WrapBrackets) // возьмем кв.скобки в скобки, чтобы получить
	{                               // работоспособную маску
		const wchar_t *src = strRawMask;
		strMask.Clear();

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
		for (int I=0; I < FileCount; I++)
		{
			CurPtr=ListData[I];
			int Match=FALSE;

			if (Mode==SELECT_INVERT || Mode==SELECT_INVERTALL)
				Match=TRUE;
			else
			{
				if (bUseFilter)
					Match=Filter.FileInFilter(*CurPtr);
				else
					Match=FileMask.Compare((ShowShortNames && !CurPtr->strShortName.IsEmpty() ? CurPtr->strShortName:CurPtr->strName));
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
						Selection=!CurPtr->Selected;
						break;
				}

				if (bUseFilter || !(CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY) || Opt.SelectFolders ||
				        !Selection || RawSelection || Mode==SELECT_INVERTALL || Mode==SELECT_INVERTMASK)
				{
					Select(CurPtr,Selection);
					workCount++;
				}
			}
		}
	}

	if (SelectedFirst)
		SortFileList(TRUE);

	ShowFileList(TRUE);

	return workCount;
}

void FileList::UpdateViewPanel()
{
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

	if (FileCount>0 && AnotherPanel->IsVisible() &&
	        AnotherPanel->GetType()==QVIEW_PANEL && SetCurPath())
	{
		QuickView *ViewPanel=(QuickView *)AnotherPanel;
		assert(CurFile<FileCount);
		FileListItem *CurPtr=ListData[CurFile];

		if (PanelMode!=PLUGIN_PANEL ||
		        CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARGETFILE))
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

			apiCreateDirectory(strTempDir,nullptr);
			PluginPanelItem PanelItem;
			FileListToPluginItem(CurPtr,&PanelItem);
			int Result=CtrlObject->Plugins->GetFile(hPlugin,&PanelItem,strTempDir,strFileName,OPM_SILENT|OPM_VIEW|OPM_QUICKVIEW);
			FreePluginPanelItem(&PanelItem);

			if (!Result)
			{
				ViewPanel->ShowFile(nullptr,FALSE,nullptr);
				apiRemoveDirectory(strTempDir);
				return;
			}

			ViewPanel->ShowFile(strFileName,TRUE,nullptr);
		}
		else if (!TestParentFolderName(CurPtr->strName))
			ViewPanel->ShowFile(CurPtr->strName,FALSE,hPlugin);
		else
			ViewPanel->ShowFile(nullptr,FALSE,nullptr);

		SetTitle();
	}
}


void FileList::CompareDir()
{
	FileList *Another=(FileList *)CtrlObject->Cp()->GetAnotherPanel(this);

	if (Another->GetType()!=FILE_PANEL || !Another->IsVisible())
	{
		Message(MSG_WARNING,1,MSG(MCompareTitle),MSG(MCompareFilePanelsRequired1),
		        MSG(MCompareFilePanelsRequired2),MSG(MOk));
		return;
	}

	ScrBuf.Flush();
	// полностью снимаем выделение с обоих панелей
	ClearSelection();
	Another->ClearSelection();
	string strTempName1, strTempName2;
	const wchar_t *PtrTempName1, *PtrTempName2;

	// помечаем ВСЕ, кроме каталогов на активной панели
	for (int I=0; I < FileCount; I++)
	{
		if (!(ListData[I]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
			Select(ListData[I],TRUE);
	}

	// помечаем ВСЕ, кроме каталогов на пассивной панели
	for (int J=0; J < Another->FileCount; J++)
	{
		if (!(Another->ListData[J]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
			Another->Select(Another->ListData[J],TRUE);
	}

	int CompareFatTime=FALSE;

	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

		if (Info.Flags & OPIF_COMPAREFATTIME)
			CompareFatTime=TRUE;

	}

	if (Another->PanelMode==PLUGIN_PANEL && !CompareFatTime)
	{
		OpenPanelInfo Info;
		CtrlObject->Plugins->GetOpenPanelInfo(Another->hPlugin,&Info);

		if (Info.Flags & OPIF_COMPAREFATTIME)
			CompareFatTime=TRUE;

	}

	if (PanelMode==NORMAL_PANEL && Another->PanelMode==NORMAL_PANEL)
	{
		string strFileSystemName1, strFileSystemName2;
		string strRoot1, strRoot2;
		GetPathRoot(strCurDir, strRoot1);
		GetPathRoot(Another->strCurDir, strRoot2);

		if (apiGetVolumeInformation(strRoot1,nullptr,nullptr,nullptr,nullptr,&strFileSystemName1) &&
		        apiGetVolumeInformation(strRoot2,nullptr,nullptr,nullptr,nullptr,&strFileSystemName2))
			if (StrCmpI(strFileSystemName1,strFileSystemName2))
				CompareFatTime=TRUE;
	}

	// теперь начнем цикл по снятию выделений
	// каждый элемент активной панели...
	for (int I=0; I < FileCount; I++)
	{
		if ((ListData[I]->FileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
			continue;

		// ...сравниваем с элементом пассивной панели...
		for (int J=0; J < Another->FileCount; J++)
		{
			if ((Another->ListData[J]->FileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
				continue;

			PtrTempName1=PointToName(ListData[I]->strName);
			PtrTempName2=PointToName(Another->ListData[J]->strName);

			if (!StrCmpI(PtrTempName1,PtrTempName2))
			{
				int Cmp=0;
				if (CompareFatTime)
				{
					WORD DosDate,DosTime,AnotherDosDate,AnotherDosTime;
					FileTimeToDosDateTime(&ListData[I]->WriteTime,&DosDate,&DosTime);
					FileTimeToDosDateTime(&Another->ListData[J]->WriteTime,&AnotherDosDate,&AnotherDosTime);
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
					__int64 RetCompare=FileTimeDifference(&ListData[I]->WriteTime,&Another->ListData[J]->WriteTime);
					Cmp=!RetCompare?0:(RetCompare > 0?1:-1);
				}

				if (!Cmp && (ListData[I]->FileSize != Another->ListData[J]->FileSize))
					continue;

				if (Cmp < 1 && ListData[I]->Selected)
					Select(ListData[I],0);

				if (Cmp > -1 && Another->ListData[J]->Selected)
					Another->Select(Another->ListData[J],0);

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

void FileList::CopyFiles()
{
	bool RealNames=false;
	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		RealNames = (Info.Flags&OPIF_REALNAMES) == OPIF_REALNAMES;
	}

	if (PanelMode!=PLUGIN_PANEL || RealNames)
	{
		LPWSTR CopyData=nullptr;
		size_t DataSize=0;
		string strSelName, strSelShortName;
		DWORD FileAttr;
		GetSelName(nullptr,FileAttr);
		while (GetSelName(&strSelName, FileAttr, &strSelShortName))
		{
			if (TestParentFolderName(strSelName) && TestParentFolderName(strSelShortName))
			{
				strSelName.SetLength(1);
				strSelShortName.SetLength(1);
			}
			if (!CreateFullPathName(strSelName,strSelShortName,FileAttr,strSelName,FALSE))
			{
				if (CopyData)
				{
					xf_free(CopyData);
					CopyData=nullptr;
				}
				break;
			}
			size_t Length=strSelName.GetLength()+1;
			wchar_t *NewPtr=static_cast<wchar_t *>(xf_realloc(CopyData, (DataSize+Length+1)*sizeof(wchar_t)));
			if (!NewPtr)
			{
				if (CopyData)
				{
					xf_free(CopyData);
					CopyData=nullptr;
				}
				break;
			}
			CopyData=NewPtr;
			wcscpy(CopyData+DataSize, strSelName);
			DataSize+=Length;
			CopyData[DataSize]=0;
		}

		if(CopyData)
		{
			DataSize++;
			Clipboard clip;
			if(clip.Open())
			{
				clip.CopyHDROP(CopyData, DataSize*sizeof(WCHAR));
				clip.Close();
			}
			xf_free(CopyData);
		}
	}
}

void FileList::CopyNames(bool FillPathName, bool UNC)
{
	OpenPanelInfo Info={};
	wchar_t *CopyData=nullptr;
	long DataSize=0;
	string strSelName, strSelShortName, strQuotedName;
	DWORD FileAttr;

	if (PanelMode==PLUGIN_PANEL)
	{
		CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
	}

	GetSelName(nullptr,FileAttr);

	while (GetSelName(&strSelName,FileAttr,&strSelShortName))
	{
		if (DataSize>0)
		{
			wcscat(CopyData+DataSize,L"\r\n");
			DataSize+=2;
		}

		strQuotedName = (ShowShortNames && !strSelShortName.IsEmpty()) ? strSelShortName:strSelName;

		if (FillPathName)
		{
			if (PanelMode!=PLUGIN_PANEL)
			{
				/* $ 14.02.2002 IS
				   ".." в текущем каталоге обработаем как имя текущего каталога
				*/
				if (TestParentFolderName(strQuotedName) && TestParentFolderName(strSelShortName))
				{
					strQuotedName.SetLength(1);
					strSelShortName.SetLength(1);
				}

				if (!CreateFullPathName(strQuotedName,strSelShortName,FileAttr,strQuotedName,UNC))
				{
					if (CopyData)
					{
						xf_free(CopyData);
						CopyData=nullptr;
					}

					break;
				}
			}
			else
			{
				string strFullName = Info.CurDir;

				if (Opt.PanelCtrlFRule && (ViewSettings.Flags&PVS_FOLDERUPPERCASE))
					strFullName.Upper();

				if (!strFullName.IsEmpty())
					AddEndSlash(strFullName);

				if (Opt.PanelCtrlFRule)
				{
					// имя должно отвечать условиям на панели
					if ((ViewSettings.Flags&PVS_FILELOWERCASE) && !(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
						strQuotedName.Lower();

					if (ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE)
						if (!(FileAttr & FILE_ATTRIBUTE_DIRECTORY) && !IsCaseMixed(strQuotedName))
							strQuotedName.Lower();
				}

				strFullName += strQuotedName;
				strQuotedName = strFullName;

				// добавим первый префикс!
				if (PanelMode==PLUGIN_PANEL && Opt.SubstPluginPrefix)
				{
					string strPrefix;

					/* $ 19.11.2001 IS оптимизация по скорости :) */
					if (*AddPluginPrefix((FileList *)CtrlObject->Cp()->ActivePanel,strPrefix))
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
					strQuotedName=Info.CurDir;
				}
				else
				{
					GetCurDir(strQuotedName);
				}

				strQuotedName=PointToName(strQuotedName);
			}
		}

		if (Opt.QuotedName&QUOTEDNAME_CLIPBOARD)
			QuoteSpace(strQuotedName);

		int Length=(int)strQuotedName.GetLength();
		wchar_t *NewPtr=(wchar_t *)xf_realloc(CopyData, (DataSize+Length+3)*sizeof(wchar_t));

		if (!NewPtr)
		{
			if (CopyData)
			{
				xf_free(CopyData);
				CopyData=nullptr;
			}

			break;
		}

		CopyData=NewPtr;
		CopyData[DataSize]=0;
		wcscpy(CopyData+DataSize, strQuotedName);
		DataSize+=Length;
	}

	CopyToClipboard(CopyData);
	xf_free(CopyData);
}

string &FileList::CreateFullPathName(const wchar_t *Name, const wchar_t *ShortName,DWORD FileAttr, string &strDest, int UNC,int ShortNameAsIs)
{
	string strFileName = strDest;
	const wchar_t *ShortNameLastSlash=LastSlash(ShortName);
	const wchar_t *NameLastSlash=LastSlash(Name);

	if (nullptr==ShortNameLastSlash && nullptr==NameLastSlash)
	{
		ConvertNameToFull(strFileName, strFileName);
	}

	/* BUGBUG весь этот if какая то чушь
	else if (ShowShortNames)
	{
	  string strTemp = Name;

	  if (NameLastSlash)
	    strTemp.SetLength(1+NameLastSlash-Name);

	  const wchar_t *NamePtr = wcsrchr(strFileName, L'\\');

	  if(NamePtr )
	    NamePtr++;
	  else
	    NamePtr=strFileName;

	  strTemp += NameLastSlash?NameLastSlash+1:Name; //??? NamePtr??? BUGBUG
	  strFileName = strTemp;
	}
	*/

	if (ShowShortNames && ShortNameAsIs)
		ConvertNameToShort(strFileName,strFileName);

	/* $ 29.01.2001 VVM
	  + По CTRL+ALT+F в командную строку сбрасывается UNC-имя текущего файла. */
	if (UNC)
		ConvertNameToUNC(strFileName);

	// $ 20.10.2000 SVS Сделаем фичу Ctrl-F опциональной!
	if (Opt.PanelCtrlFRule)
	{
		/* $ 13.10.2000 tran
		  по Ctrl-f имя должно отвечать условиям на панели */
		if (ViewSettings.Flags&PVS_FOLDERUPPERCASE)
		{
			if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
			{
				strFileName.Upper();
			}
			else
			{
				size_t pos;

				if (FindLastSlash(pos,strFileName))
					strFileName.Upper(0,pos);
				else
					strFileName.Upper();
			}
		}

		if ((ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE) && !(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			size_t pos;

			if (FindLastSlash(pos,strFileName) && !IsCaseMixed(strFileName.CPtr()+pos))
				strFileName.Lower(pos);
		}

		if ((ViewSettings.Flags&PVS_FILELOWERCASE) && !(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			size_t pos;

			if (FindLastSlash(pos,strFileName))
				strFileName.Lower(pos);
		}
	}

	strDest = strFileName;
	return strDest;
}


void FileList::SetTitle()
{
	if (GetFocus() || CtrlObject->Cp()->GetAnotherPanel(this)->GetType()!=FILE_PANEL)
	{
		string strTitleDir(L"{");

		if (PanelMode==PLUGIN_PANEL)
		{
			OpenPanelInfo Info;
			CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
			string strPluginTitle = Info.PanelTitle;
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
	for (int I=0; I < FileCount; I++)
	{
		Select(ListData[I],0);
	}

	if (SelectedFirst)
		SortFileList(TRUE);
}


void FileList::SaveSelection()
{
	for (int I=0; I < FileCount; I++)
	{
		ListData[I]->PrevSelected=ListData[I]->Selected;
	}
}


void FileList::RestoreSelection()
{
	for (int I=0; I < FileCount; I++)
	{
		int NewSelection=ListData[I]->PrevSelected;
		ListData[I]->PrevSelected=ListData[I]->Selected;
		Select(ListData[I],NewSelection);
	}

	if (SelectedFirst)
		SortFileList(TRUE);

	Redraw();
}



int FileList::GetFileName(string &strName,int Pos,DWORD &FileAttr)
{
	if (Pos>=FileCount)
		return FALSE;

	strName = ListData[Pos]->strName;
	FileAttr=ListData[Pos]->FileAttr;
	return TRUE;
}


int FileList::GetCurrentPos()
{
	return(CurFile);
}


void FileList::EditFilter()
{
	if (!Filter)
		Filter=new FileFilter(this,FFT_PANEL);

	Filter->FilterEdit();
}


void FileList::SelectSortMode()
{
	MenuDataEx SortMenu[]=
	{
		MSG(MMenuSortByName),LIF_SELECTED,KEY_CTRLF3,
		MSG(MMenuSortByExt),0,KEY_CTRLF4,
		MSG(MMenuSortByWrite),0,KEY_CTRLF5,
		MSG(MMenuSortBySize),0,KEY_CTRLF6,
		MSG(MMenuUnsorted),0,KEY_CTRLF7,
		MSG(MMenuSortByCreation),0,KEY_CTRLF8,
		MSG(MMenuSortByAccess),0,KEY_CTRLF9,
		MSG(MMenuSortByChange),0,0,
		MSG(MMenuSortByDiz),0,KEY_CTRLF10,
		MSG(MMenuSortByOwner),0,KEY_CTRLF11,
		MSG(MMenuSortByAllocatedSize),0,0,
		MSG(MMenuSortByNumLinks),0,0,
		MSG(MMenuSortByNumStreams),0,0,
		MSG(MMenuSortByStreamsSize),0,0,
		MSG(MMenuSortByFullName),0,0,
		MSG(MMenuSortByCustomData),0,0,
		L"",LIF_SEPARATOR,0,
		MSG(MMenuSortUseNumeric),0,0,
		MSG(MMenuSortUseCaseSensitive),0,0,
		MSG(MMenuSortUseGroups),0,KEY_SHIFTF11,
		MSG(MMenuSortSelectedFirst),0,KEY_SHIFTF12,
		MSG(MMenuSortDirectoriesFirst),0,0,
	};
	static int SortModes[]=
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

	for (size_t i=0; i<ARRAYSIZE(SortModes); i++)
		if (SortModes[i]==SortMode)
		{
			SortMenu[i].SetCheck(SortOrder==1 ? L'+':L'-');
			break;
		}

	int SG=GetSortGroups();
	SortMenu[BY_CUSTOMDATA+2].SetCheck(NumericSort);
	SortMenu[BY_CUSTOMDATA+3].SetCheck(CaseSensitiveSort);
	SortMenu[BY_CUSTOMDATA+4].SetCheck(SG);
	SortMenu[BY_CUSTOMDATA+5].SetCheck(SelectedFirst);
	SortMenu[BY_CUSTOMDATA+6].SetCheck(DirectoriesFirst);
	int SortCode=-1;
	bool setSortMode0=false;

	{
		VMenu SortModeMenu(MSG(MMenuSortTitle),SortMenu,ARRAYSIZE(SortMenu),0);
		SortModeMenu.SetHelp(L"PanelCmdSort");
		SortModeMenu.SetPosition(X1+4,-1,0,0);
		SortModeMenu.SetFlags(VMENU_WRAPMODE);
		//SortModeMenu.Process();
		bool MenuNeedRefresh=true;

		while (!SortModeMenu.Done())
		{
			if (MenuNeedRefresh)
			{
				SortModeMenu.Hide(); // спрячем
				// заставим манагер менюхи корректно отрисовать ширину и
				// высоту, а заодно и скорректировать вертикальные позиции
				SortModeMenu.SetPosition(X1+4,-1,0,0);
				SortModeMenu.Show();
				MenuNeedRefresh=false;
			}

			int Key=SortModeMenu.ReadInput();
			int MenuPos=SortModeMenu.GetSelectPos();

			if (Key == KEY_SUBTRACT)
				Key=L'-';
			else if (Key == KEY_ADD)
				Key=L'+';
			else if (Key == KEY_MULTIPLY)
				Key=L'*';

			if (MenuPos < (int)ARRAYSIZE(SortModes) && (Key == L'+' || Key == L'-' || Key == L'*'))
			{
				// clear check
				for (size_t i=0; i<ARRAYSIZE(SortModes); i++)
					SortModeMenu.SetCheck(0,static_cast<int>(i));
			}

			switch (Key)
			{
				case L'*':
					setSortMode0=false;
					SortModeMenu.SetExitCode(MenuPos);
					break;

				case L'+':
					if (MenuPos<(int)ARRAYSIZE(SortModes))
					{
						SortOrder=1;
						setSortMode0=true;
					}
					else
					{
						switch (MenuPos)
						{
							case BY_CUSTOMDATA+2:
								NumericSort=0;
								break;
							case BY_CUSTOMDATA+3:
								CaseSensitiveSort=0;
								break;
							case BY_CUSTOMDATA+4:
								SortGroups=0;
								break;
							case BY_CUSTOMDATA+5:
								SelectedFirst=0;
								break;
							case BY_CUSTOMDATA+6:
								DirectoriesFirst=0;
								break;
						}
					}
					SortModeMenu.SetExitCode(MenuPos);
					break;

				case L'-':
					if (MenuPos<(int)ARRAYSIZE(SortModes))
					{
						SortOrder=-1;
						setSortMode0=true;
					}
					else
					{
						switch (MenuPos)
						{
							case BY_CUSTOMDATA+2:
								NumericSort=1;
								break;
							case BY_CUSTOMDATA+3:
								NumericSort=1;
								break;
							case BY_CUSTOMDATA+4:
								SortGroups=1;
								break;
							case BY_CUSTOMDATA+5:
								SelectedFirst=1;
								break;
							case BY_CUSTOMDATA+6:
								DirectoriesFirst=1;
								break;
						}
					}
					SortModeMenu.SetExitCode(MenuPos);
					break;

				default:
					SortModeMenu.ProcessInput();
					break;
			}
		}

		if ((SortCode=SortModeMenu.Modal::GetExitCode())<0)
			return;
	}

	if (SortCode<(int)ARRAYSIZE(SortModes))
	{
		if (setSortMode0)
			SetSortMode0(SortModes[SortCode]);
		else
			SetSortMode(SortModes[SortCode]);
	}
	else
		switch (SortCode)
		{
			case BY_CUSTOMDATA+2:
				ChangeNumericSort(NumericSort?0:1);
				break;
			case BY_CUSTOMDATA+3:
				ChangeCaseSensitiveSort(CaseSensitiveSort?0:1);
				break;
			case BY_CUSTOMDATA+4:
				ProcessKey(KEY_SHIFTF11);
				break;
			case BY_CUSTOMDATA+5:
				ProcessKey(KEY_SHIFTF12);
				break;
			case BY_CUSTOMDATA+6:
				ChangeDirectoriesFirst(DirectoriesFirst?0:1);
				break;
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


void FileList::GetDizName(string &strDizName)
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
	Panel* AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
	int AnotherType=AnotherPanel->GetType();

	while (GetSelName(&strSelName,FileAttr,&strSelShortName))
	{
		string strDizText, strMsg, strQuotedName;
		const wchar_t *PrevText;
		PrevText=Diz.GetDizTextAddr(strSelName,strSelShortName,GetLastSelectedSize());
		strQuotedName = strSelName;
		QuoteSpaceOnly(strQuotedName);
		strMsg.Append(MSG(MEnterDescription)).Append(L" ").Append(strQuotedName).Append(L":");

		/* $ 09.08.2000 SVS
		   Для Ctrl-Z ненужно брать предыдущее значение!
		*/
		if (!GetString(MSG(MDescribeFiles),strMsg,L"DizText",
		               PrevText ? PrevText:L"",strDizText,
		               L"FileDiz",FIB_ENABLEEMPTY|(!DizCount?FIB_NOUSELASTHISTORY:0)|FIB_BUTTONS))
			break;

		DizCount++;

		if (strDizText.IsEmpty())
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
	  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
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
	bool isSilent=false;

	if (!GetString(MSG(MAskApplyCommandTitle),MSG(MAskApplyCommand),L"ApplyCmd",strPrevCommand,strCommand,L"ApplyCmd",FIB_BUTTONS|FIB_EDITPATH|FIB_EDITPATHEXEC) || !SetCurPath())
		return false;

	strPrevCommand = strCommand;
	RemoveLeadingSpaces(strCommand);

	if (strCommand.At(0) == L'@')
	{
		strCommand.LShift(1);
		isSilent=true;
	}

	string strSelName, strSelShortName;
	DWORD FileAttr;

	SaveSelection();

	string CommandLine;
	CtrlObject->CmdLine->GetString(CommandLine);
	int SelStart, SelEnd;
	CtrlObject->CmdLine->GetSelection(SelStart, SelEnd);
	int CursorPosition = CtrlObject->CmdLine->GetCurPos();
	int LeftPosition = CtrlObject->CmdLine->GetLeftPos();

	++UpdateDisabled;
	GetSelName(nullptr,FileAttr);
	CtrlObject->CmdLine->LockUpdatePanel(true);
	while (GetSelName(&strSelName,FileAttr,&strSelShortName) && !CheckForEsc())
	{
		string strListName, strAnotherListName;
		string strShortListName, strAnotherShortListName;
		string strConvertedCommand = strCommand;
		int PreserveLFN=SubstFileName(strConvertedCommand,strSelName, strSelShortName, &strListName, &strAnotherListName, &strShortListName, &strAnotherShortListName);
		bool ListFileUsed=!strListName.IsEmpty()||!strAnotherListName.IsEmpty()||!strShortListName.IsEmpty()||!strAnotherShortListName.IsEmpty();

		if (ExtractIfExistCommand(strConvertedCommand))
		{
			PreserveLongName PreserveName(strSelShortName,PreserveLFN);
			RemoveExternalSpaces(strConvertedCommand);

			if (!strConvertedCommand.IsEmpty())
			{
				ProcessOSAliases(strConvertedCommand);

				if (!isSilent)   // TODO: Здесь не isSilent!
				{
					CtrlObject->CmdLine->ExecString(strConvertedCommand,FALSE, 0, 0, ListFileUsed); // Param2 == TRUE?
					//if (!(Opt.ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTAPPLYCMD))
					//	CtrlObject->CmdHistory->AddToHistory(strConvertedCommand);
				}
				else
				{
					CtrlObject->Cp()->LeftPanel->CloseFile();
					CtrlObject->Cp()->RightPanel->CloseFile();
					Execute(strConvertedCommand,FALSE,FALSE, 0, 0, ListFileUsed, true);
				}
			}

			ClearLastGetSelection();
		}

		if (!strListName.IsEmpty())
			apiDeleteFile(strListName);

		if (!strAnotherListName.IsEmpty())
			apiDeleteFile(strAnotherListName);

		if (!strShortListName.IsEmpty())
			apiDeleteFile(strShortListName);

		if (!strAnotherShortListName.IsEmpty())
			apiDeleteFile(strAnotherShortListName);
	}


	CtrlObject->CmdLine->SetString(CommandLine);
	CtrlObject->CmdLine->Select(SelStart, SelEnd);
	CtrlObject->CmdLine->SetCurPos(CursorPosition, LeftPosition);

	CtrlObject->CmdLine->LockUpdatePanel(false);
	CtrlObject->CmdLine->Show();
	if (Opt.ShowKeyBar)
	{
		CtrlObject->MainKeyBar->Show();
	}
	if (GetSelPosition >= FileCount)
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
	if (PanelMode==PLUGIN_PANEL && !CurFile && TestParentFolderName(ListData[0]->strName))
	{
		FileListItem *DoubleDotDir = nullptr;

		if (SelFileCount)
		{
			DoubleDotDir = ListData[0];

			for (int I=0; I < FileCount; I++)
			{
				if (ListData[I]->Selected && (ListData[I]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				{
					DoubleDotDir = nullptr;
					break;
				}
			}
		}
		else
		{
			DoubleDotDir = ListData[0];
		}

		if (DoubleDotDir)
		{
			DoubleDotDir->ShowFolderSize=1;
			DoubleDotDir->FileSize     = 0;
			DoubleDotDir->AllocationSize    = 0;

			for (int I=1; I < FileCount; I++)
			{
				if (ListData[I]->FileAttr & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (GetPluginDirInfo(hPlugin,ListData[I]->strName, Data.DirCount, Data.FileCount, Data.FileSize, Data.AllocationSize))
					{
						DoubleDotDir->FileSize += Data.FileSize;
						DoubleDotDir->AllocationSize += Data.AllocationSize;
					}
				}
				else
				{
					DoubleDotDir->FileSize     += ListData[I]->FileSize;
					DoubleDotDir->AllocationSize    += ListData[I]->AllocationSize;
				}
			}
		}
	}

	//Рефреш текущему времени для фильтра перед началом операции
	Filter->UpdateCurrentTime();

	for (int I=0; I < FileCount; I++)
	{
		if (ListData[I]->Selected && (ListData[I]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			SelDirCount++;
			if ((PanelMode==PLUGIN_PANEL && !(PluginFlags & OPIF_REALNAMES) &&
			        GetPluginDirInfo(hPlugin,ListData[I]->strName, Data.DirCount, Data.FileCount, Data.FileSize, Data.AllocationSize))
			        ||
			        ((PanelMode!=PLUGIN_PANEL || (PluginFlags & OPIF_REALNAMES)) &&
			         GetDirInfo(MSG(MDirInfoViewTitle), ListData[I]->strName, Data, 0, Filter, GETDIRINFO_DONTREDRAWFRAME|GETDIRINFO_SCANSYMLINKDEF)==1))
			{
				SelFileSize -= ListData[I]->FileSize;
				SelFileSize += Data.FileSize;
				ListData[I]->FileSize = Data.FileSize;
				ListData[I]->AllocationSize = Data.AllocationSize;
				ListData[I]->ShowFolderSize=1;
			}
			else
				break;
		}
	}

	if (!SelDirCount)
	{
		assert(CurFile<FileCount);
		if ((PanelMode==PLUGIN_PANEL && !(PluginFlags & OPIF_REALNAMES) &&
		        GetPluginDirInfo(hPlugin,ListData[CurFile]->strName, Data.DirCount, Data.FileCount, Data.FileSize, Data.AllocationSize))
		        ||
		        ((PanelMode!=PLUGIN_PANEL || (PluginFlags & OPIF_REALNAMES)) &&
		         GetDirInfo(MSG(MDirInfoViewTitle),
		                    TestParentFolderName(ListData[CurFile]->strName) ? L".":ListData[CurFile]->strName,
		                    Data, 0, Filter, GETDIRINFO_DONTREDRAWFRAME|GETDIRINFO_SCANSYMLINKDEF)==1))
		{
			ListData[CurFile]->FileSize = Data.FileSize;
			ListData[CurFile]->AllocationSize = Data.AllocationSize;
			ListData[CurFile]->ShowFolderSize=1;
		}
	}

	SortFileList(TRUE);
	ShowFileList(TRUE);
	CtrlObject->Cp()->Redraw();
	InitFSWatcher(true);
}


int FileList::GetPrevViewMode()
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.Empty())?(*PluginsList.First())->PrevViewMode:ViewMode;
}


int FileList::GetPrevSortMode()
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.Empty())?(*PluginsList.First())->PrevSortMode:SortMode;
}


int FileList::GetPrevSortOrder()
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.Empty())?(*PluginsList.First())->PrevSortOrder:SortOrder;
}

int FileList::GetPrevNumericSort()
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.Empty())?(*PluginsList.First())->PrevNumericSort:NumericSort;
}

int FileList::GetPrevCaseSensitiveSort()
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.Empty())?(*PluginsList.First())->PrevCaseSensitiveSort:CaseSensitiveSort;
}

int FileList::GetPrevDirectoriesFirst()
{
	return (PanelMode==PLUGIN_PANEL && !PluginsList.Empty())?(*PluginsList.First())->PrevDirectoriesFirst:DirectoriesFirst;
}

HANDLE FileList::OpenFilePlugin(const string* FileName, int PushPrev, OPENFILEPLUGINTYPE Type)
{
	if (!PushPrev && PanelMode==PLUGIN_PANEL)
	{
		for (;;)
		{
			if (ProcessPluginEvent(FE_CLOSE,nullptr))
				return(PANEL_STOP);

			if (!PopPlugin(TRUE))
				break;
		}
	}

	HANDLE hNewPlugin=OpenPluginForFile(FileName, 0, Type);

	if (hNewPlugin && hNewPlugin!=PANEL_STOP)
	{
		if (PushPrev)
		{
			PrevDataItem* Item=new PrevDataItem;;
			Item->PrevListData=ListData;
			Item->PrevFileCount=FileCount;
			Item->PrevTopFile = CurTopFile;
			Item->strPrevName = FileName? *FileName : L"";
			PrevDataList.Push(&Item);
			ListData=nullptr;
			FileCount=0;
		}

		bool WasFullscreen = IsFullScreen();
		SetPluginMode(hNewPlugin, FileName ? *FileName : L"");  // SendOnFocus??? true???
		PanelMode=PLUGIN_PANEL;
		UpperFolderTopFile=CurTopFile;
		CurFile=0;
		Update(0);
		Redraw();
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

		if ((AnotherPanel->GetType()==INFO_PANEL) || WasFullscreen)
			AnotherPanel->Redraw();
	}

	return hNewPlugin;
}


void FileList::ProcessCopyKeys(int Key)
{
	if (FileCount>0)
	{
		int Drag=Key==KEY_DRAGCOPY || Key==KEY_DRAGMOVE;
		int Ask=!Drag || Opt.Confirm.Drag;
		int Move=(Key==KEY_F6 || Key==KEY_DRAGMOVE);
		int AnotherDir=FALSE;
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

		if (AnotherPanel->GetType()==FILE_PANEL)
		{
			FileList *AnotherFilePanel=(FileList *)AnotherPanel;

			assert(AnotherFilePanel->FileCount==0 || AnotherFilePanel->CurFile<AnotherFilePanel->FileCount);
			if (AnotherFilePanel->FileCount>0 &&
			        (AnotherFilePanel->ListData[AnotherFilePanel->CurFile]->FileAttr & FILE_ATTRIBUTE_DIRECTORY) &&
			        !TestParentFolderName(AnotherFilePanel->ListData[AnotherFilePanel->CurFile]->strName))
			{
				AnotherDir=TRUE;
			}
		}

		if (PanelMode==PLUGIN_PANEL && !CtrlObject->Plugins->UseFarCommand(hPlugin,PLUGIN_FARGETFILES))
		{
			if (Key!=KEY_ALTF6 && Key!=KEY_RALTF6)
			{
				string strPluginDestPath;
				int ToPlugin=FALSE;

				if (AnotherPanel->GetMode()==PLUGIN_PANEL && AnotherPanel->IsVisible() &&
				        !CtrlObject->Plugins->UseFarCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES))
				{
					ToPlugin=2;
					ShellCopy ShCopy(this,Move,FALSE,FALSE,Ask,ToPlugin,strPluginDestPath);
				}

				if (ToPlugin!=-1)
				{
					if (ToPlugin)
						PluginToPluginFiles(Move);
					else
					{
						string strDestPath;

						if (!strPluginDestPath.IsEmpty())
							strDestPath = strPluginDestPath;
						else
						{
							AnotherPanel->GetCurDir(strDestPath);

							if (!AnotherPanel->IsVisible())
							{
								OpenPanelInfo Info;
								CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

								if (Info.HostFile && *Info.HostFile)
								{
									size_t pos;
									strDestPath = PointToName(Info.HostFile);

									if (strDestPath.RPos(pos,L'.'))
										strDestPath.SetLength(pos);
								}
							}
						}

						const wchar_t *lpwszDestPath=strDestPath;

						PluginGetFiles(&lpwszDestPath,Move);
						strDestPath=lpwszDestPath;
					}
				}
			}
		}
		else
		{
			int ToPlugin=AnotherPanel->GetMode()==PLUGIN_PANEL &&
			             AnotherPanel->IsVisible() && (Key!=KEY_ALTF6 && Key!=KEY_RALTF6) &&
			             !CtrlObject->Plugins->UseFarCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES);
			ShellCopy ShCopy(this,Move,(Key==KEY_ALTF6 || Key==KEY_RALTF6),FALSE,Ask,ToPlugin,nullptr, Drag && AnotherDir);

			if (ToPlugin==1)
				PluginPutFilesToAnother(Move,AnotherPanel);
		}
	}
}

void FileList::SetSelectedFirstMode(int Mode)
{
	SelectedFirst=Mode;
	SortFileList(TRUE);
}

void FileList::ChangeSortOrder(int NewOrder)
{
	Panel::ChangeSortOrder(NewOrder);
	SortFileList(TRUE);
	Show();
}

BOOL FileList::UpdateKeyBar()
{
	KeyBar *KB=CtrlObject->MainKeyBar;
	KB->SetAllGroup(KBL_MAIN, MF1, 12);
	KB->SetAllGroup(KBL_SHIFT, MShiftF1, 12);
	KB->SetAllGroup(KBL_ALT, MAltF1, 12);
	KB->SetAllGroup(KBL_CTRL, MCtrlF1, 12);
	KB->SetAllGroup(KBL_CTRLSHIFT, MCtrlShiftF1, 12);
	KB->SetAllGroup(KBL_CTRLALT, MCtrlAltF1, 12);
	KB->SetAllGroup(KBL_ALTSHIFT, MAltShiftF1, 12);
	KB->SetAllGroup(KBL_CTRLALTSHIFT, MCtrlAltShiftF1, 12);
	KB->ReadRegGroup(L"Shell",Opt.strLanguage);
	KB->SetAllRegGroup();

	if (GetMode() == PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		GetOpenPanelInfo(&Info);

		if (Info.KeyBar)
			KB->Change(Info.KeyBar);
	}

	return TRUE;
}

int FileList::PluginPanelHelp(HANDLE hPlugin)
{
	string strPath, strFileName, strStartTopic;
	PluginHandle *ph = (PluginHandle*)hPlugin;
	strPath = ph->pPlugin->GetModuleName();
	CutToSlash(strPath);
	UINT nCodePage = CP_OEMCP;
	FILE *HelpFile=OpenLangFile(strPath,HelpFileMask,Opt.strHelpLanguage,strFileName, nCodePage);

	if (!HelpFile)
		return FALSE;

	fclose(HelpFile);
	strStartTopic.Format(HelpFormatLink,strPath.CPtr(),L"Contents");
	Help PanelHelp(strStartTopic);
	return TRUE;
}

/* $ 19.11.2001 IS
     для файловых панелей с реальными файлами никакого префикса не добавляем
*/
string &FileList::AddPluginPrefix(FileList *SrcPanel,string &strPrefix)
{
	strPrefix.Clear();

	if (Opt.SubstPluginPrefix && SrcPanel->GetMode()==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		PluginHandle *ph = (PluginHandle*)SrcPanel->hPlugin;
		CtrlObject->Plugins->GetOpenPanelInfo(ph,&Info);

		if (!(Info.Flags & OPIF_REALNAMES))
		{
			PluginInfo PInfo = {sizeof(PInfo)};
			ph->pPlugin->GetPluginInfo(&PInfo);

			if (PInfo.CommandPrefix && *PInfo.CommandPrefix)
			{
				strPrefix = PInfo.CommandPrefix;
				size_t pos;

				if (strPrefix.Pos(pos,L':'))
					strPrefix.SetLength(pos+1);
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
	string strFName=g_strFarModuleName;

	{
		strFName.SetLength(3); //BUGBUG!
		// СНАЧАЛА ПАССИВНАЯ ПАНЕЛЬ!!!
		/*
			Почему? - Просто - если активная широкая (или пассивная
			широкая) - получаем багу с прорисовкой!
		*/
		Panel *Another=CtrlObject->Cp()->GetAnotherPanel(this);

		if (Another->GetMode() != PLUGIN_PANEL)
		{
			Another->GetCurDir(strTmpCurDir);

			if (strTmpCurDir.At(0) == Drive && strTmpCurDir.At(1) == L':')
				Another->SetCurDir(strFName, FALSE);
		}

		if (GetMode() != PLUGIN_PANEL)
		{
			GetCurDir(strTmpCurDir);

			if (strTmpCurDir.At(0) == Drive && strTmpCurDir.At(1) == L':')
				SetCurDir(strFName, FALSE); // переходим в корень диска с far.exe
		}
	}
}


BOOL FileList::GetItem(int Index,void *Dest)
{
	if (Index == -1 || Index == -2)
		Index=GetCurrentPos();

	if ((DWORD)Index >= (DWORD)FileCount)
		return FALSE;

	*((FileListItem *)Dest)=*ListData[Index];
	return TRUE;
}

void FileList::ClearAllItem()
{
	for (PrevDataItem **i=PrevDataList.First();i;i=PrevDataList.Next(i))
	{
		DeleteListData((*i)->PrevListData,(*i)->PrevFileCount);
		delete *i;
	}

	PrevDataList.Clear();
}
