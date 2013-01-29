/*
treelist.cpp

Tree panel
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

#include "treelist.hpp"
#include "flink.hpp"
#include "keyboard.hpp"
#include "colors.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "chgprior.hpp"
#include "scantree.hpp"
#include "copy.hpp"
#include "qview.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "help.hpp"
#include "lockscrn.hpp"
#include "macroopcode.hpp"
#include "RefreshFrameManager.hpp"
#include "scrbuf.hpp"
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "config.hpp"
#include "delete.hpp"
#include "mkdir.hpp"
#include "setattr.hpp"
#include "execute.hpp"
#include "shortcuts.hpp"
#include "dirmix.hpp"
#include "pathmix.hpp"
#include "processname.hpp"
#include "constitle.hpp"
#include "syslog.hpp"
#include "cache.hpp"
#include "filestr.hpp"
#include "wakeful.hpp"
#include "palette.hpp"
#include "FarGuid.hpp"

static bool StaticSortNumeric;
static bool StaticSortCaseSensitive;
static clock_t TreeStartTime;
static int LastScrX = -1;
static int LastScrY = -1;

static int TreeCmp(const wchar_t *Str1, const wchar_t *Str2, bool Numeric, bool CaseSensitive)
{
	auto cmpfunc = Numeric? (CaseSensitive? NumStrCmpN : NumStrCmpNI) : (CaseSensitive? StrCmpNN : StrCmpNNI);

	if (*Str1 == L'\\' && *Str2 == L'\\')
	{
		Str1++;
		Str2++;
	}

	const wchar_t *s1 = wcschr(Str1,L'\\');
	const wchar_t *s2 = wcschr(Str2,L'\\');

	while (s1 && s2)
	{
		int r = cmpfunc(Str1,static_cast<int>(s1-Str1),Str2,static_cast<int>(s2-Str2));

		if (r)
			return r;

		Str1 = s1 + 1;
		Str2 = s2 + 1;
		s1 = wcschr(Str1,L'\\');
		s2 = wcschr(Str2,L'\\');
	}

	if (s1 || s2)
	{
		int r = cmpfunc(Str1,s1?static_cast<int>(s1-Str1):-1,Str2,s2?static_cast<int>(s2-Str2):-1);

		if (r)
			return r;

		return s1 ? 1 : -1;
	}

	return cmpfunc(Str1, -1, Str2,-1);
}

static struct list_less
{
	bool operator()(const TreeItem* a, const TreeItem* b)
	{
		return TreeCmp(a->strName, b->strName, StaticSortNumeric, StaticSortCaseSensitive) < 0;
	}
}
ListLess;

static struct cache_less
{
	bool operator()(const string& a, const string& b)
	{
		return TreeCmp(a, b, StaticSortNumeric, 0) < 0;
	}
}
CacheLess;

void TreeListCache::Sort()
{
	std::sort(ListName.begin(), ListName.end(), CacheLess);
}


TreeList::TreeList(bool IsPanel):
	PrevMacroMode(MACRO_INVALID),
	WorkDir(0),
	GetSelPosition(0),
	NumericSort(FALSE),
	CaseSensitiveSort(FALSE),
	ExitCode(1),
	SaveWorkDir(0)
{
	Type=TREE_PANEL;
	CurFile=CurTopFile=0;
	Flags.Set(FTREELIST_UPDATEREQUIRED);
	Flags.Clear(FTREELIST_TREEISPREPARED);
	Flags.Change(FTREELIST_ISPANEL,IsPanel);
}

TreeList::~TreeList()
{
	std::for_each(RANGE(ListData, i)
	{
		delete i;
	});
	ListData.clear();
	SaveListData.clear();
	Global->tempTreeCache->Clean();
	FlushCache();
	SetMacroMode(TRUE);
}

void TreeList::SetRootDir(const wchar_t *NewRootDir)
{
	strRoot = NewRootDir;
	strCurDir = NewRootDir;
}

void TreeList::DisplayObject()
{
	if (Flags.Check(FSCROBJ_ISREDRAWING))
		return;

	Flags.Set(FSCROBJ_ISREDRAWING);

	if (Flags.Check(FTREELIST_UPDATEREQUIRED))
		Update(0);

	if (ExitCode)
	{
		Panel *RootPanel=GetRootPanel();

		if (RootPanel->GetType()==FILE_PANEL)
		{
			bool RootCaseSensitiveSort=RootPanel->GetCaseSensitiveSort() != 0;
			bool RootNumeric=RootPanel->GetNumericSort() != 0;

			if (RootNumeric != NumericSort || RootCaseSensitiveSort!=CaseSensitiveSort)
			{
				NumericSort=RootNumeric;
				CaseSensitiveSort=RootCaseSensitiveSort;
				StaticSortNumeric=NumericSort;
				StaticSortCaseSensitive=CaseSensitiveSort;
				std::sort(ListData.begin(), ListData.end(), ListLess);
				FillLastData();
				SyncDir();
			}
		}

		DisplayTree(FALSE);
	}

	Flags.Clear(FSCROBJ_ISREDRAWING);
}

string &TreeList::GetTitle(string &strTitle,int SubLen,int TruncSize)
{
	strTitle = L" ";
	strTitle += ModalMode? MSG(MFindFolderTitle) : MSG(MTreeTitle);
	strTitle += L" ";
	TruncStr(strTitle,X2-X1-3);
	return strTitle;
}

void TreeList::DisplayTree(int Fast)
{
	wchar_t TreeLineSymbol[4][3]=
	{
		{L' ',                  L' ',             0},
		{BoxSymbols[BS_V1],     L' ',             0},
		{BoxSymbols[BS_LB_H1V1],BoxSymbols[BS_H1],0},
		{BoxSymbols[BS_L_H1V1], BoxSymbols[BS_H1],0},
	};
	TreeItem *CurPtr;
	string strTitle;
	LockScreen *LckScreen=nullptr;

	if (Global->CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == QVIEW_PANEL)
		LckScreen=new LockScreen;

	CorrectPosition();

	if (!ListData.empty())
		strCurDir = ListData[CurFile]->strName; //BUGBUG

//    xstrncpy(CurDir,ListData[CurFile].Name,sizeof(CurDir));
	if (!Fast)
	{
		Box(X1,Y1,X2,Y2,ColorIndexToColor(COL_PANELBOX),DOUBLE_BOX);
		DrawSeparator(Y2-2-(ModalMode));
		GetTitle(strTitle);

		if (!strTitle.IsEmpty())
		{
			SetColor((Focus || ModalMode) ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
			GotoXY(X1+(X2-X1+1-(int)strTitle.GetLength())/2,Y1);
			Text(strTitle);
		}
	}

	for (size_t I=Y1+1,J=CurTopFile; I<static_cast<size_t>(Y2-2-ModalMode); I++,J++)
	{
		GotoXY(X1+1, static_cast<int>(I));
		SetColor(COL_PANELTEXT);
		Text(L" ");

		if (J < ListData.size() && Flags.Check(FTREELIST_TREEISPREPARED))
		{
			CurPtr=ListData[J];

			if (!J)
			{
				DisplayTreeName(L"\\",J);
			}
			else
			{
				string strOutStr;

				for (int i=0; i<CurPtr->Depth-1 && WhereX()+3*i<X2-6; i++)
				{
					strOutStr+=TreeLineSymbol[CurPtr->Last[i]?0:1];
				}

				strOutStr+=TreeLineSymbol[CurPtr->Last[CurPtr->Depth-1]?2:3];
				BoxText(strOutStr);
				const wchar_t *ChPtr=LastSlash(CurPtr->strName);

				if (ChPtr)
					DisplayTreeName(ChPtr+1,J);
			}
		}

		SetColor(COL_PANELTEXT);

		if (WhereX()<X2)
		{
			Global->FS << fmt::MinWidth(X2-WhereX())<<L"";
		}
	}

	if (Global->Opt->ShowPanelScrollbar)
	{
		SetColor(COL_PANELSCROLLBAR);
		ScrollBarEx(X2, Y1+1, Y2-Y1-3, CurTopFile, ListData.size());
	}

	SetColor(COL_PANELTEXT);
	SetScreen(X1+1,Y2-(ModalMode?2:1),X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));

	if (!ListData.empty())
	{
		GotoXY(X1+1,Y2-1);
		Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(X2-X1-1)<<ListData[CurFile]->strName;
	}

	UpdateViewPanel();
	SetTitle(); // не забудим прорисовать заголовок

	if (LckScreen)
		delete LckScreen;
}

void TreeList::DisplayTreeName(const wchar_t *Name, size_t Pos)
{
	if (WhereX()>X2-4)
		GotoXY(X2-4,WhereY());

	if (Pos==static_cast<size_t>(CurFile))
	{
		GotoXY(WhereX()-1,WhereY());

		if (Focus || ModalMode)
		{
			SetColor((Pos==WorkDir) ? COL_PANELSELECTEDCURSOR:COL_PANELCURSOR);
			Global->FS << L" "<<fmt::MaxWidth(X2-WhereX()-3)<<Name<<L" ";
		}
		else
		{
			SetColor((Pos==WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
			Global->FS << L"["<<fmt::MaxWidth(X2-WhereX()-3)<<Name<<L"]";
		}
	}
	else
	{
		SetColor((Pos==WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
		Global->FS << fmt::MaxWidth(X2-WhereX()-1)<<Name;
	}
}

void TreeList::Update(int Mode)
{
	if (!EnableUpdate)
		return;

	if (!IsVisible())
	{
		Flags.Set(FTREELIST_UPDATEREQUIRED);
		return;
	}

	Flags.Clear(FTREELIST_UPDATEREQUIRED);
	GetRoot();
	size_t LastTreeCount = ListData.size();
	int RetFromReadTree=TRUE;
	Flags.Clear(FTREELIST_TREEISPREPARED);
	int TreeFilePresent=ReadTreeFile();

	if (!TreeFilePresent)
		RetFromReadTree=ReadTree();

	Flags.Set(FTREELIST_TREEISPREPARED);

	if (!RetFromReadTree && !Flags.Check(FTREELIST_ISPANEL))
	{
		ExitCode=0;
		return;
	}

	if (RetFromReadTree && !ListData.empty() && (!(Mode & UPDATE_KEEP_SELECTION) || LastTreeCount != ListData.size()))
	{
		SyncDir();
		TreeItem *CurPtr=ListData[CurFile];

		if (apiGetFileAttributes(CurPtr->strName)==INVALID_FILE_ATTRIBUTES)
		{
			DelTreeName(CurPtr->strName);
			Update(UPDATE_KEEP_SELECTION);
			Show();
		}
	}
	else if (!RetFromReadTree)
	{
		Show();

		if (!Flags.Check(FTREELIST_ISPANEL))
		{
			Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
		}
	}
}

int TreeList::ReadTree()
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	//SaveScreen SaveScr;
	TPreRedrawFuncGuard preRedrawFuncGuard(TreeList::PR_MsgReadTree);
	ScanTree ScTree(FALSE);
	FAR_FIND_DATA_EX fdata;
	string strFullName;
	SaveState();
	FlushCache();
	GetRoot();

	std::for_each(RANGE(ListData, i)
	{
		delete i;
	});
	ListData.clear();

	ListData.reserve(4096);

	auto NewItem = new TreeItem;
	NewItem->strName = strRoot;
	ListData.push_back(NewItem);
	SaveScreen SaveScrTree;
	UndoGlobalSaveScrPtr UndSaveScr(&SaveScrTree);
	/* Т.к. мы можем вызвать диалог подтверждения (который не перерисовывает панельки,
	   а восстанавливает сохраненный образ экрана, то нарисуем чистую панель */
	//Redraw();
	int FirstCall=TRUE, AscAbort=FALSE;
	TreeStartTime = clock();
	RefreshFrameManager frref(ScrX,ScrY,TreeStartTime,FALSE);//DontRedrawFrame);
	ScTree.SetFindPath(strRoot, L"*", 0);
	LastScrX = ScrX;
	LastScrY = ScrY;
	TaskBar TB;
	wakeful W;
	while (ScTree.GetNextName(&fdata,strFullName))
	{
		TreeList::MsgReadTree(ListData.size(), FirstCall);

		if (CheckForEscSilent())
		{
			AscAbort=ConfirmAbortOp();
			FirstCall=TRUE;
		}

		if (AscAbort)
			break;

		if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		if (ListData.size() == ListData.capacity())
		{
			ListData.reserve(ListData.size() + 4096);
		}

		auto NewItem = new TreeItem;
		NewItem->strName = strFullName;
		ListData.push_back(NewItem);
	}

	if (AscAbort && !Flags.Check(FTREELIST_ISPANEL))
	{
		std::for_each(RANGE(ListData, i)
		{
			delete i;
		});
		ListData.clear();
		RestoreState();
		return FALSE;
	}

	StaticSortNumeric=NumericSort=StaticSortCaseSensitive=CaseSensitiveSort=FALSE;
	std::sort(ListData.begin(), ListData.end(), ListLess);

	if (!FillLastData())
		return FALSE;

	if (!AscAbort)
		SaveTreeFile();

	if (!FirstCall && !Flags.Check(FTREELIST_ISPANEL))
	{ // Перерисуем другую панель - удалим следы сообщений :)
		Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
		AnotherPanel->Redraw();
	}

	return TRUE;
}

void TreeList::SaveTreeFile()
{
	if (ListData.size() < static_cast<size_t>(Global->Opt->Tree.MinTreeCount))
		return;

	string strName;

	size_t RootLength=strRoot.IsEmpty()?0:strRoot.GetLength()-1;
	MkTreeFileName(strRoot, strName);
	// получим и сразу сбросим атрибуты (если получится)
	DWORD FileAttributes=apiGetFileAttributes(strName);

	if (FileAttributes != INVALID_FILE_ATTRIBUTES)
		apiSetFileAttributes(strName,FILE_ATTRIBUTE_NORMAL);

	File TreeFile;
	if (!TreeFile.Open(strName,GENERIC_WRITE,FILE_SHARE_READ,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL))
	{
		if (MustBeCached(strRoot))
		{	if (!GetCacheTreeName(strRoot,strName,TRUE) || !TreeFile.Open(strName,GENERIC_WRITE,FILE_SHARE_READ,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL))
			{
				return;
			}
		}
		else
		{
			return;
		}
	}

	bool Success=true;
	CachedWrite Cache(TreeFile);
	for (auto i = ListData.begin(); i != ListData.end() && Success; ++i)
	{
		if (RootLength>=(*i)->strName.GetLength())
		{
			Success=Cache.Write(L"\\\n", 2 * sizeof(wchar_t));
		}
		else
		{
			Success=Cache.Write((*i)->strName+RootLength, static_cast<DWORD>((*i)->strName.GetLength() - RootLength) * sizeof(wchar_t));
			if(Success)
			{
				Success=Cache.Write(L"\n",1 * sizeof(wchar_t));
			}
		}
	}
	Cache.Flush();
	TreeFile.SetEnd();
	TreeFile.Close();

	if (!Success)
	{
		apiDeleteFile(strName);
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotSaveTree),strName,MSG(MOk));
	}
	else if (FileAttributes != INVALID_FILE_ATTRIBUTES) // вернем атрибуты (если получится :-)
		apiSetFileAttributes(strName,FileAttributes);
}

int TreeList::GetCacheTreeName(const string& Root, string& strName,int CreateDir)
{
	string strVolumeName, strFileSystemName;
	DWORD dwVolumeSerialNumber;

	if (!apiGetVolumeInformation(
	            Root,
	            &strVolumeName,
	            &dwVolumeSerialNumber,
	            nullptr,
	            nullptr,
	            &strFileSystemName
	        ))
		return FALSE;

	string strFolderName;
	string strFarPath;
	MkTreeCacheFolderName(Global->Opt->LocalProfilePath, strFolderName);

	if (CreateDir)
	{
		apiCreateDirectory(strFolderName, nullptr);
		apiSetFileAttributes(strFolderName,Global->Opt->Tree.TreeFileAttr);
	}

	string strRemoteName;
	wchar_t *lpwszRemoteName;

//  char RemoteName[NM*3];
//  *RemoteName=0;
	if (*Root == L'\\')
		strRemoteName = Root;
	else
	{
		string LocalName(L"?:");
		LocalName.Replace(0, *Root);
		apiWNetGetConnection(LocalName, strRemoteName);

		if (!strRemoteName.IsEmpty())
			AddEndSlash(strRemoteName);
	}

	lpwszRemoteName = strRemoteName.GetBuffer();

	for (int I=0; lpwszRemoteName[I] ; I++)
		if (lpwszRemoteName[I]==L'\\')
			lpwszRemoteName[I]=L'_';

	strRemoteName.ReleaseBuffer();
	strName = FormatString() << strFolderName << L"\\" << strVolumeName << L"." << fmt::Radix(16) << dwVolumeSerialNumber << L"." << strFileSystemName << L"." << strRemoteName;
	return TRUE;
}

void TreeList::GetRoot()
{
	string strPanelDir;
	Panel *RootPanel=GetRootPanel();
	RootPanel->GetCurDir(strPanelDir);
	strRoot = ExtractPathRoot(strPanelDir);
}

Panel* TreeList::GetRootPanel()
{
	Panel *RootPanel;

	if (ModalMode)
	{
		if (ModalMode==MODALTREE_ACTIVE)
			RootPanel=Global->CtrlObject->Cp()->ActivePanel;
		else if (ModalMode==MODALTREE_FREE)
			RootPanel=this;
		else
		{
			RootPanel=Global->CtrlObject->Cp()->GetAnotherPanel(Global->CtrlObject->Cp()->ActivePanel);

			if (!RootPanel->IsVisible())
				RootPanel=Global->CtrlObject->Cp()->ActivePanel;
		}
	}
	else
		RootPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);

	return(RootPanel);
}

void TreeList::SyncDir()
{
	string strPanelDir;
	Panel *AnotherPanel=GetRootPanel();
	AnotherPanel->GetCurDir(strPanelDir);

	if (!strPanelDir.IsEmpty())
	{
		if (AnotherPanel->GetType()==FILE_PANEL)
		{
			if (!SetDirPosition(strPanelDir))
			{
				ReadSubTree(strPanelDir);
				ReadTreeFile();
				SetDirPosition(strPanelDir);
			}
		}
		else
			SetDirPosition(strPanelDir);
	}
}

void TreeList::PR_MsgReadTree()
{
	int FirstCall=1;
	PreRedrawItem preRedrawItem=Global->PreRedraw->Peek();
	TreeList::MsgReadTree(preRedrawItem.Param.Flags,FirstCall);
}

int TreeList::MsgReadTree(size_t TreeCount,int &FirstCall)
{
	/* $ 24.09.2001 VVM
	  ! Писать сообщение о чтении дерева только, если это заняло более 500 мсек. */
	BOOL IsChangeConsole = LastScrX != ScrX || LastScrY != ScrY;

	if (IsChangeConsole)
	{
		LastScrX = ScrX;
		LastScrY = ScrY;
	}

	if (IsChangeConsole || (clock() - TreeStartTime) > 1000)
	{
		Message((FirstCall ? 0:MSG_KEEPBACKGROUND),0,MSG(MTreeTitle), MSG(MReadingTree), FormatString() << TreeCount);
		PreRedrawItem preRedrawItem=Global->PreRedraw->Peek();
		preRedrawItem.Param.Flags = static_cast<DWORD>(TreeCount);
		Global->PreRedraw->SetParam(preRedrawItem.Param);
		TreeStartTime = clock();
	}

	return 1;
}

bool TreeList::FillLastData()
{
	size_t RootLength = strRoot.IsEmpty()? 0 : strRoot.GetLength()-1;
	for (auto i = ListData.begin() + 1 ; i != ListData.end(); ++i)
	{
		size_t Pos;
		int PathLength = ((*i)->strName.RPos(Pos,L'\\'))? (int)Pos+1 : 0;

		size_t Depth=(*i)->Depth=CountSlash((*i)->strName.CPtr()+RootLength);

		if (!Depth)
			return false;

		auto SubDirPos = i;
		int Last = 1;

		for (auto j = i+1; j != ListData.end(); ++j)
		{
			if (CountSlash((*j)->strName.CPtr()+RootLength)>Depth)
			{
				SubDirPos = j;
				continue;
			}
			else
			{
				if (!StrCmpNI((*i)->strName, (*j)->strName, PathLength))
					Last=0;
				break;
			}
		}

		for (auto j = i; j != SubDirPos + 1; ++j)
		{
 			if (Depth > (*j)->Last.size())
			{
				(*j)->Last.resize((*j)->Last.size() + MAX_PATH, 0);
			}
			(*j)->Last[Depth-1]=Last;
		}
	}
	return true;
}

UINT TreeList::CountSlash(const wchar_t *Str)
{
	UINT Count=0;

	for (; *Str; Str++)
		if (IsSlash(*Str))
			Count++;

	return(Count);
}

__int64 TreeList::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return ListData.empty();
		case MCODE_C_EOF:
			return static_cast<size_t>(CurFile) == ListData.size() - 1;
		case MCODE_C_BOF:
			return !CurFile;
		case MCODE_C_SELECTED:
			return 0;
		case MCODE_V_ITEMCOUNT:
			return ListData.size();
		case MCODE_V_CURPOS:
			return CurFile+1;
	}

	return 0;
}

int TreeList::ProcessKey(int Key)
{
	if (!IsVisible())
		return FALSE;

	if (ListData.empty() && Key!=KEY_CTRLR && Key!=KEY_RCTRLR)
		return FALSE;

	string strTemp;

	if ((Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9) || (Key>=KEY_CTRLALT0 && Key<=KEY_CTRLALT9))
	{
		bool Add = (Key>=KEY_CTRLALT0 && Key<=KEY_CTRLALT9);
		SaveShortcutFolder(Key-(Add?KEY_CTRLALT0:KEY_CTRLSHIFT0), Add);
		return TRUE;
	}

	if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
	{
		ExecShortcutFolder(Key-KEY_RCTRL0);
		return TRUE;
	}

	switch (Key)
	{
		case KEY_F1:
		{
			Help Hlp(L"TreePanel");
			return TRUE;
		}
		case KEY_SHIFTNUMENTER:
		case KEY_CTRLNUMENTER:
		case KEY_RCTRLNUMENTER:
		case KEY_SHIFTENTER:
		case KEY_CTRLENTER:
		case KEY_RCTRLENTER:
		case KEY_CTRLF:
		case KEY_RCTRLF:
		case KEY_CTRLALTINS:
		case KEY_RCTRLRALTINS:
		case KEY_CTRLRALTINS:
		case KEY_RCTRLALTINS:
		case KEY_CTRLALTNUMPAD0:
		case KEY_RCTRLRALTNUMPAD0:
		case KEY_CTRLRALTNUMPAD0:
		case KEY_RCTRLALTNUMPAD0:
		{
			string strQuotedName=ListData[CurFile]->strName;
			QuoteSpace(strQuotedName);

			if (Key==KEY_CTRLALTINS||Key==KEY_RCTRLRALTINS||Key==KEY_CTRLRALTINS||Key==KEY_RCTRLALTINS||
				Key==KEY_CTRLALTNUMPAD0||Key==KEY_RCTRLRALTNUMPAD0||Key==KEY_CTRLRALTNUMPAD0||Key==KEY_RCTRLALTNUMPAD0)
			{
				CopyToClipboard(strQuotedName);
			}
			else
			{
				if (Key == KEY_SHIFTENTER||Key == KEY_SHIFTNUMENTER)
				{
					Execute(strQuotedName,FALSE,TRUE,TRUE);
				}
				else
				{
					strQuotedName+=L" ";
					Global->CtrlObject->CmdLine->InsertString(strQuotedName);
				}
			}

			return TRUE;
		}
		case KEY_CTRLBACKSLASH:
		case KEY_RCTRLBACKSLASH:
		{
			CurFile=0;
			ProcessEnter();
			return TRUE;
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!ModalMode && Global->CtrlObject->CmdLine->GetLength()>0)
				break;

			ProcessEnter();
			return TRUE;
		}
		case KEY_F4:
		case KEY_CTRLA:
		case KEY_RCTRLA:
		{
			if (SetCurPath())
				ShellSetFileAttributes(this);

			return TRUE;
		}
		case KEY_CTRLR:
		case KEY_RCTRLR:
		{
			ReadTree();

			if (!ListData.empty())
				SyncDir();

			Redraw();
			break;
		}
		case KEY_SHIFTF5:
		case KEY_SHIFTF6:
		{
			if (SetCurPath())
			{
				int ToPlugin=0;
				ShellCopy ShCopy(this,Key==KEY_SHIFTF6,FALSE,TRUE,TRUE,ToPlugin,nullptr);
			}

			return TRUE;
		}
		case KEY_F5:
		case KEY_DRAGCOPY:
		case KEY_F6:
		case KEY_ALTF6:
		case KEY_RALTF6:
		case KEY_DRAGMOVE:
		{
			if (!ListData.empty() && SetCurPath())
			{
				Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
				int Ask=((Key!=KEY_DRAGCOPY && Key!=KEY_DRAGMOVE) || Global->Opt->Confirm.Drag);
				int Move=(Key==KEY_F6 || Key==KEY_DRAGMOVE);
				int ToPlugin=AnotherPanel->GetMode()==PLUGIN_PANEL &&
				             AnotherPanel->IsVisible() &&
				             !Global->CtrlObject->Plugins->UseFarCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES);
				int Link=((Key==KEY_ALTF6||Key==KEY_RALTF6) && !ToPlugin);

				if ((Key==KEY_ALTF6||Key==KEY_RALTF6) && !Link) // молча отвалим :-)
					return TRUE;

				{
					ShellCopy ShCopy(this,Move,Link,FALSE,Ask,ToPlugin,nullptr);
				}

				if (ToPlugin==1)
				{
					PluginPanelItem Item;
					int ItemNumber=1;
					HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();
					FileList::FileNameToPluginItem(ListData[CurFile]->strName, &Item);
					int PutCode=Global->CtrlObject->Plugins->PutFiles(hAnotherPlugin, &Item, ItemNumber, Move != 0, 0);

					if (PutCode==1 || PutCode==2)
						AnotherPanel->SetPluginModified();

					if (Move)
						ReadSubTree(ListData[CurFile]->strName);

					Update(0);
					Redraw();
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);
					AnotherPanel->Redraw();
				}
			}

			return TRUE;
		}
		case KEY_F7:
		{
			if (SetCurPath())
				ShellMakeDir(this);

			return TRUE;
		}
		/*
		  Удаление                                   Shift-Del, Shift-F8, F8

		  Удаление файлов и папок. F8 и Shift-Del удаляют все выбранные
		 файлы, Shift-F8 - только файл под курсором. Shift-Del всегда удаляет
		 файлы, не используя Корзину (Recycle Bin). Использование Корзины
		 командами F8 и Shift-F8 зависит от конфигурации.

		  Уничтожение файлов и папок                                 Alt-Del
		*/
		case KEY_F8:
		case KEY_SHIFTDEL:
		case KEY_SHIFTNUMDEL:
		case KEY_SHIFTDECIMAL:
		case KEY_ALTNUMDEL:
		case KEY_RALTNUMDEL:
		case KEY_ALTDECIMAL:
		case KEY_RALTDECIMAL:
		case KEY_ALTDEL:
		case KEY_RALTDEL:
		{
			if (SetCurPath())
			{
				bool SaveOpt=Global->Opt->DeleteToRecycleBin;

				if (Key==KEY_SHIFTDEL||Key==KEY_SHIFTNUMDEL||Key==KEY_SHIFTDECIMAL)
					Global->Opt->DeleteToRecycleBin=0;

				ShellDelete(this,Key==KEY_ALTDEL||Key==KEY_RALTDEL||Key==KEY_ALTNUMDEL||Key==KEY_RALTNUMDEL||Key==KEY_ALTDECIMAL||Key==KEY_RALTDECIMAL);
				// Надобно не забыть обновить противоположную панель...
				Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(this);
				AnotherPanel->Update(UPDATE_KEEP_SELECTION);
				AnotherPanel->Redraw();
				Global->Opt->DeleteToRecycleBin=SaveOpt;

				if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
					ProcessKey(KEY_ENTER);
			}

			return TRUE;
		}
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		case(KEY_MSWHEEL_UP | KEY_RALT):
		{
			Scroll(Key & (KEY_ALT|KEY_RALT)?-1:(int)-Global->Opt->MsWheelDelta);
			return TRUE;
		}
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		case(KEY_MSWHEEL_DOWN | KEY_RALT):
		{
			Scroll(Key & (KEY_ALT|KEY_RALT)?1:(int)Global->Opt->MsWheelDelta);
			return TRUE;
		}
		case KEY_MSWHEEL_LEFT:
		case(KEY_MSWHEEL_LEFT | KEY_ALT):
		case(KEY_MSWHEEL_LEFT | KEY_RALT):
		{
			int Roll = Key & (KEY_ALT|KEY_RALT)?1:(int)Global->Opt->MsHWheelDelta;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_LEFT);

			return TRUE;
		}
		case KEY_MSWHEEL_RIGHT:
		case(KEY_MSWHEEL_RIGHT | KEY_ALT):
		case(KEY_MSWHEEL_RIGHT | KEY_RALT):
		{
			int Roll = Key & (KEY_ALT|KEY_RALT)?1:(int)Global->Opt->MsHWheelDelta;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_RIGHT);

			return TRUE;
		}
		case KEY_HOME:        case KEY_NUMPAD7:
		{
			Up(0x7fffff);

			if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return TRUE;
		}
		case KEY_ADD: // OFM: Gray+/Gray- navigation
		{
			CurFile=GetNextNavPos();

			if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);
			else
				DisplayTree(TRUE);

			return TRUE;
		}
		case KEY_SUBTRACT: // OFM: Gray+/Gray- navigation
		{
			CurFile=GetPrevNavPos();

			if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);
			else
				DisplayTree(TRUE);

			return TRUE;
		}
		case KEY_END:         case KEY_NUMPAD1:
		{
			Down(0x7fffff);

			if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return TRUE;
		}
		case KEY_UP:          case KEY_NUMPAD8:
		{
			Up(1);

			if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return TRUE;
		}
		case KEY_DOWN:        case KEY_NUMPAD2:
		{
			Down(1);

			if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return TRUE;
		}
		case KEY_PGUP:        case KEY_NUMPAD9:
		{
			CurTopFile-=Y2-Y1-3-ModalMode;
			CurFile-=Y2-Y1-3-ModalMode;
			DisplayTree(TRUE);

			if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return TRUE;
		}
		case KEY_PGDN:        case KEY_NUMPAD3:
		{
			CurTopFile+=Y2-Y1-3-ModalMode;
			CurFile+=Y2-Y1-3-ModalMode;
			DisplayTree(TRUE);

			if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return TRUE;
		}

		case KEY_APPS:
		case KEY_SHIFTAPPS:
		{
			//вызовем EMenu если он есть
			if (Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu))
			{
				Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu, OPEN_FILEPANEL, reinterpret_cast<void*>(static_cast<intptr_t>(1))); // EMenu Plugin :-)
			}
			return TRUE;
		}

		default:
			if ((Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+65535) || (Key>=KEY_RALT_BASE+0x01 && Key<=KEY_RALT_BASE+65535) ||
			        (Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+65535) || (Key>=KEY_RALTSHIFT_BASE+0x01 && Key<=KEY_RALTSHIFT_BASE+65535))
			{
				FastFind(Key);

				if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
					ProcessKey(KEY_ENTER);
			}
			else
				break;

			return TRUE;
	}

	return FALSE;
}

int TreeList::GetNextNavPos()
{
	int NextPos=CurFile;

	if (static_cast<size_t>(CurFile+1) < ListData.size())
	{
		int CurDepth=ListData[CurFile]->Depth;

		for (size_t I=CurFile+1; I < ListData.size(); ++I)
			if (ListData[I]->Depth == CurDepth)
			{
				NextPos=static_cast<int>(I);
				break;
			}
	}

	return NextPos;
}

int TreeList::GetPrevNavPos()
{
	int PrevPos=CurFile;

	if (CurFile-1 > 0)
	{
		int CurDepth=ListData[CurFile]->Depth;

		for (int I=CurFile-1; I > 0; --I)
			if (ListData[I]->Depth == CurDepth)
			{
				PrevPos=I;
				break;
			}
	}

	return PrevPos;
}

void TreeList::Up(int Count)
{
	CurFile-=Count;
	DisplayTree(TRUE);
}

void TreeList::Down(int Count)
{
	CurFile+=Count;
	DisplayTree(TRUE);
}

void TreeList::Scroll(int Count)
{
	CurFile+=Count;
	CurTopFile+=Count;
	DisplayTree(TRUE);
}

void TreeList::CorrectPosition()
{
	if (ListData.empty())
	{
		CurFile=CurTopFile=0;
		return;
	}

	int Height=Y2-Y1-3-(ModalMode);

	if (CurTopFile+Height > static_cast<int>(ListData.size()))
		CurTopFile = static_cast<int>(ListData.size() - Height);

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

	if (CurFile>CurTopFile+Height-1)
		CurTopFile=CurFile-(Height-1);
}

BOOL TreeList::SetCurDir(const string& NewDir,int ClosePanel,BOOL /*IsUpdated*/)
{
	if (ListData.empty())
		Update(0);

	if (!ListData.empty() && !SetDirPosition(NewDir))
	{
		Update(0);
		SetDirPosition(NewDir);
	}

	if (GetFocus())
	{
		Global->CtrlObject->CmdLine->SetCurDir(NewDir);
		Global->CtrlObject->CmdLine->Show();
	}

	return TRUE; //???
}

int TreeList::SetDirPosition(const wchar_t *NewDir)
{
	for (size_t i = 0; i < ListData.size(); ++i)
	{
		if (!StrCmpI(NewDir,ListData[i]->strName))
		{
			WorkDir = i;
			CurFile = static_cast<int>(i);
			CurTopFile=CurFile-(Y2-Y1-1)/2;
			CorrectPosition();
			return TRUE;
		}
	}

	return FALSE;
}

int TreeList::GetCurDir(string &strCurDir)
{
	if (ListData.empty())
	{
		if (ModalMode==MODALTREE_FREE)
			strCurDir = strRoot;
		else
			strCurDir.Clear();
	}
	else
		strCurDir = ListData[CurFile]->strName; //BUGBUG

	return (int)strCurDir.GetLength();
}

int TreeList::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	int OldFile=CurFile;
	int RetCode;

	if (Global->Opt->ShowPanelScrollbar && IntKeyState.MouseX==X2 &&
	        (MouseEvent->dwButtonState & 1) && !IsDragging())
	{
		int ScrollY=Y1+1;
		int Height=Y2-Y1-3;

		if (IntKeyState.MouseY==ScrollY)
		{
			while (IsMouseButtonPressed())
				ProcessKey(KEY_UP);

			if (!ModalMode)
				SetFocus();

			return TRUE;
		}

		if (IntKeyState.MouseY==ScrollY+Height-1)
		{
			while (IsMouseButtonPressed())
				ProcessKey(KEY_DOWN);

			if (!ModalMode)
				SetFocus();

			return TRUE;
		}

		if (IntKeyState.MouseY>ScrollY && IntKeyState.MouseY<ScrollY+Height-1 && Height>2)
		{
			CurFile = static_cast<int>(ListData.size() - 1) * (IntKeyState.MouseY - ScrollY) / (Height - 2);
			DisplayTree(TRUE);

			if (!ModalMode)
				SetFocus();

			return TRUE;
		}
	}

	if (Panel::PanelProcessMouse(MouseEvent,RetCode))
		return(RetCode);

	if (MouseEvent->dwMousePosition.Y>Y1 && MouseEvent->dwMousePosition.Y<Y2-2)
	{
		if (!ModalMode)
			SetFocus();

		MoveToMouse(MouseEvent);
		DisplayTree(TRUE);

		if (ListData.empty())
			return TRUE;

		if (((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
		        MouseEvent->dwEventFlags==DOUBLE_CLICK) ||
		        ((MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) &&
		         !MouseEvent->dwEventFlags) ||
		        (OldFile!=CurFile && Global->Opt->Tree.AutoChangeFolder && !ModalMode))
		{
			DWORD control=MouseEvent->dwControlKeyState&(SHIFT_PRESSED|LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED);

			//вызовем EMenu если он есть
			if (!Global->Opt->RightClickSelect && MouseEvent->dwButtonState == RIGHTMOST_BUTTON_PRESSED && (control==0 || control==SHIFT_PRESSED) && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu))
			{
				Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu,OPEN_FILEPANEL,nullptr); // EMenu Plugin :-)
				return TRUE;
			}

			ProcessEnter();
			return TRUE;
		}

		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y<=Y1+1)
	{
		if (!ModalMode)
			SetFocus();

		if (ListData.empty())
			return TRUE;

		while (IsMouseButtonPressed() && IntKeyState.MouseY<=Y1+1)
			Up(1);

		if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
			ProcessKey(KEY_ENTER);

		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y>=Y2-2)
	{
		if (!ModalMode)
			SetFocus();

		if (ListData.empty())
			return TRUE;

		while (IsMouseButtonPressed() && IntKeyState.MouseY>=Y2-2)
			Down(1);

		if (Global->Opt->Tree.AutoChangeFolder && !ModalMode)
			ProcessKey(KEY_ENTER);

		return TRUE;
	}

	return FALSE;
}

void TreeList::MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	CurFile=CurTopFile+MouseEvent->dwMousePosition.Y-Y1-1;
	CorrectPosition();
}

void TreeList::ProcessEnter()
{
	TreeItem *CurPtr;
	DWORD Attr;
	CurPtr=ListData[CurFile];

	if ((Attr=apiGetFileAttributes(CurPtr->strName))!=INVALID_FILE_ATTRIBUTES && (Attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		if (!ModalMode && FarChDir(CurPtr->strName))
		{
			Panel *AnotherPanel=GetRootPanel();
			SetCurDir(CurPtr->strName,TRUE);
			Show();
			AnotherPanel->SetCurDir(CurPtr->strName,TRUE);
			AnotherPanel->Redraw();
		}
	}
	else
	{
		DelTreeName(CurPtr->strName);
		Update(UPDATE_KEEP_SELECTION);
		Show();
	}
}

int TreeList::ReadTreeFile()
{
	size_t RootLength=strRoot.IsEmpty()?0:strRoot.GetLength()-1;
	string strName;
	//SaveState();
	FlushCache();
	MkTreeFileName(strRoot,strName);

	File TreeFile;
	if (MustBeCached(strRoot) || (!TreeFile.Open(strName, FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING)))
	{
		if (!GetCacheTreeName(strRoot,strName,FALSE) || (!TreeFile.Open(strName, FILE_READ_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING)))
		{
			//RestoreState();
			return FALSE;
		}
	}

	std::for_each(RANGE(ListData, i)
	{
		delete i;
	});
	ListData.clear();

	{
		string strLastDirName;
		GetFileString GetStr(TreeFile);
		LPWSTR Record=nullptr;
		int RecordLength=0;
		while(GetStr.GetString(&Record, CP_UNICODE, RecordLength) > 0)
		{
			string strDirName(strRoot, RootLength);
			strDirName.Append(Record, RecordLength);
			if (!IsSlash(*Record) || !StrCmpI(strDirName,strLastDirName))
			{
				continue;
			}

			strLastDirName=strDirName;
			size_t Pos=0;
			if(strDirName.Pos(Pos, L'\n'))
			{
				strDirName.SetLength(Pos);
			}

			if (RootLength>0 && strDirName.At(RootLength-1)!=L':' && IsSlash(strDirName.At(RootLength)) && !strDirName.At(RootLength+1))
			{
				strDirName.SetLength(RootLength);
			}

			if (ListData.size() == ListData.capacity())
			{
				ListData.reserve(ListData.size() + 4096);
			}

			auto NewItem = new TreeItem;
			NewItem->strName = strDirName;
			ListData.push_back(NewItem);
		}
	}

	TreeFile.Close();

	if (ListData.empty())
		return FALSE;

	NumericSort=FALSE;
	CaseSensitiveSort=FALSE;
	Global->TreeCache->Sort();
	return FillLastData();
}

bool TreeList::GetPlainString(string& Dest,int ListPos)
{
	Dest=L"";
#if defined(Mantis_698)
	if (ListPos<TreeCount)
	{
		Dest=ListData[ListPos]->strName;
		return true;
	}
#endif
	return false;
}

int TreeList::FindPartName(const wchar_t *Name,int Next,int Direct,int ExcludeSets)
{
	string strMask;
	strMask = Name;
	strMask += L"*";

	if (ExcludeSets)
	{
		ReplaceStrings(strMask,L"[",L"<[%>",-1,true);
		ReplaceStrings(strMask,L"]",L"[]]",-1,true);
		ReplaceStrings(strMask,L"<[%>",L"[[]",-1,true);
	}

	for (int i=CurFile+(Next?Direct:0); i >= 0 && static_cast<size_t>(i) < ListData.size(); i+=Direct)
	{
		if (CmpName(strMask,ListData[i]->strName,true,(i==CurFile)))
		{
			CurFile=i;
			CurTopFile=CurFile-(Y2-Y1-1)/2;
			DisplayTree(TRUE);
			return TRUE;
		}
	}

	for (size_t i=(Direct > 0)?0:ListData.size()-1; (Direct > 0) ? i < static_cast<size_t>(CurFile):i > static_cast<size_t>(CurFile); i+=Direct)
	{
		if (CmpName(strMask,ListData[i]->strName,true))
		{
			CurFile=static_cast<int>(i);
			CurTopFile=CurFile-(Y2-Y1-1)/2;
			DisplayTree(TRUE);
			return TRUE;
		}
	}

	return FALSE;
}

size_t TreeList::GetSelCount()
{
	return 1;
}

int TreeList::GetSelName(string *strName,DWORD &FileAttr,string *strShortName,FAR_FIND_DATA_EX *fd)
{
	if (!strName)
	{
		GetSelPosition=0;
		return TRUE;
	}

	if (!GetSelPosition)
	{
		GetCurDir(*strName);

		if (strShortName )
			*strShortName = *strName;

		FileAttr=FILE_ATTRIBUTE_DIRECTORY;
		GetSelPosition++;
		return TRUE;
	}

	GetSelPosition=0;
	return FALSE;
}

int TreeList::GetCurName(string &strName, string &strShortName)
{
	if (ListData.empty())
	{
		strName.Clear();
		strShortName.Clear();
		return FALSE;
	}

	strName = ListData[CurFile]->strName;
	strShortName = strName;
	return TRUE;
}

void TreeList::AddTreeName(const wchar_t *Name)
{
	if (!*Name)
		return;

	string strFullName;
	ConvertNameToFull(Name, strFullName);
	string strRoot = ExtractPathRoot(strFullName);
	Name = strFullName;
	Name += strRoot.GetLength() - 1;

	if (!LastSlash(Name))
		return;

	ReadCache(strRoot);

	FOR_RANGE(Global->TreeCache->ListName, i)
	{
		int Result = StrCmpI(*i, Name);

		if (!Result)
			break;

		if (Result > 0)
		{
			i = Global->TreeCache->Insert(i, Name);
			break;
		}
	}
}

void TreeList::DelTreeName(const wchar_t *Name)
{
	if (!*Name)
		return;

	string strFullName;
	ConvertNameToFull(Name, strFullName);
	string strRoot = ExtractPathRoot(strFullName);
	Name = strFullName;
	Name += strRoot.GetLength() - 1;
	ReadCache(strRoot);

	FOR_RANGE(Global->TreeCache->ListName, i)
	{
		size_t Length = StrLength(Name);

		if (i->GetLength() < Length) continue;

		if (!StrCmpNI(Name, *i, static_cast<int>(Length)) && (!i->At(Length) || IsSlash(i->At(Length))))
		{
			i = Global->TreeCache->Delete(i);
		}
	}
}

void TreeList::RenTreeName(const string& strSrcName,const string& strDestName)
{
	string SrcNameFull, DestNameFull;
	ConvertNameToFull(strSrcName, SrcNameFull);
	ConvertNameToFull(strDestName, DestNameFull);
	string strSrcRoot = ExtractPathRoot(SrcNameFull);
	string strDestRoot = ExtractPathRoot(DestNameFull);

	if (StrCmpI(strSrcRoot, strDestRoot) )
	{
		DelTreeName(strSrcName);
		ReadSubTree(strSrcName);
	}

	const wchar_t* SrcName = strSrcName;
	SrcName += strSrcRoot.GetLength() - 1;
	const wchar_t* DestName = strDestName;
	DestName += strDestRoot.GetLength() - 1;
	ReadCache(strSrcRoot);
	int SrcLength = StrLength(SrcName);

	for (size_t CachePos = 0; CachePos < Global->TreeCache->ListName.size(); CachePos++)
	{
		const wchar_t* DirName = Global->TreeCache->ListName[CachePos];

		if (!StrCmpNI(SrcName,DirName,SrcLength) && (!DirName[SrcLength] || IsSlash(DirName[SrcLength])))
		{
			Global->TreeCache->ListName[CachePos] = string(DestName) + DirName + SrcLength;
		}
	}
}

void TreeList::ReadSubTree(const string& Path)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	//SaveScreen SaveScr;
	TPreRedrawFuncGuard preRedrawFuncGuard(TreeList::PR_MsgReadTree);
	ScanTree ScTree(FALSE);
	FAR_FIND_DATA_EX fdata;
	string strDirName;
	string strFullName;
	int Count=0;
	DWORD FileAttr;

	if ((FileAttr=apiGetFileAttributes(Path))==INVALID_FILE_ATTRIBUTES || !(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		return;

	ConvertNameToFull(Path, strDirName);
	AddTreeName(strDirName);
	int FirstCall=TRUE, AscAbort=FALSE;
	ScTree.SetFindPath(strDirName,L"*",0);
	LastScrX = ScrX;
	LastScrY = ScrY;

	while (ScTree.GetNextName(&fdata, strFullName))
	{
		if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			TreeList::MsgReadTree(Count+1,FirstCall);

			if (CheckForEscSilent())
			{
				AscAbort=ConfirmAbortOp();
				FirstCall=TRUE;
			}

			if (AscAbort)
				break;

			AddTreeName(strFullName);
			++Count;
		}
	}
}

void TreeList::ClearCache(int EnableFreeMem)
{
	Global->TreeCache->Clean();
}

void TreeList::ReadCache(const string& TreeRoot)
{
	string strTreeName;
	FILE *TreeFile=nullptr;

	if (MkTreeFileName(TreeRoot,strTreeName) == Global->TreeCache->strTreeName)
		return;

	if (!Global->TreeCache->ListName.empty())
		FlushCache();

	if (MustBeCached(TreeRoot) || !(TreeFile=_wfopen(strTreeName,L"rb")))
		if (!GetCacheTreeName(TreeRoot,strTreeName,FALSE) || !(TreeFile=_wfopen(strTreeName,L"rb")))
		{
			ClearCache(1);
			return;
		}

	Global->TreeCache->strTreeName = strTreeName;
	wchar_t *DirName=new wchar_t[NT_MAX_PATH];

	if (DirName)
	{
		while (fgetws(DirName,NT_MAX_PATH,TreeFile))
		{
			if (!IsSlash(*DirName))
				continue;

			wchar_t *ChPtr=wcschr(DirName,L'\n');

			if (ChPtr)
				*ChPtr=0;

			Global->TreeCache->Add(DirName);
		}

		delete[] DirName;
	}

	fclose(TreeFile);
}

void TreeList::FlushCache()
{
	if (!Global->TreeCache->strTreeName.IsEmpty())
	{
		DWORD FileAttributes=apiGetFileAttributes(Global->TreeCache->strTreeName);

		if (FileAttributes != INVALID_FILE_ATTRIBUTES)
			apiSetFileAttributes(Global->TreeCache->strTreeName,FILE_ATTRIBUTE_NORMAL);

		File TreeFile;
		if (!TreeFile.Open(Global->TreeCache->strTreeName, FILE_WRITE_DATA, FILE_SHARE_READ, nullptr, OPEN_ALWAYS))
		{
			ClearCache(1);
			return;
		}
		CachedWrite Cache(TreeFile);

		Global->TreeCache->Sort();

		bool WriteError = false;
		FOR_RANGE(Global->TreeCache->ListName, i)
		{
			if (!Cache.Write(i->CPtr(), i->GetLength() * sizeof(wchar_t)) || !Cache.Write(L"\n", 1 * sizeof(wchar_t)))
			{
				WriteError = true;
				break;
			}
		}
		if (!WriteError)
			WriteError = !Cache.Flush();
		TreeFile.SetEnd();
		TreeFile.Close();
		if (WriteError)
		{
			apiDeleteFile(Global->TreeCache->strTreeName);
			Message(MSG_WARNING|MSG_ERRORTYPE, 1, MSG(MError), MSG(MCannotSaveTree), Global->TreeCache->strTreeName, MSG(MOk));
		}
		else if (FileAttributes != INVALID_FILE_ATTRIBUTES) // вернем атрибуты (если получится :-)
			apiSetFileAttributes(Global->TreeCache->strTreeName,FileAttributes);
	}
	ClearCache(1);
}

void TreeList::UpdateViewPanel()
{
	if (!ModalMode)
	{
		Panel *AnotherPanel=GetRootPanel();
		string strCurName;
		GetCurDir(strCurName);

		if (AnotherPanel->GetType()==QVIEW_PANEL && SetCurPath())
			((QuickView *)AnotherPanel)->ShowFile(strCurName,FALSE,nullptr);
	}
}

int TreeList::GoToFile(long idxItem)
{
	if (static_cast<size_t>(idxItem) < ListData.size())
	{
		CurFile=idxItem;
		CorrectPosition();
		return TRUE;
	}

	return FALSE;
}

int TreeList::GoToFile(const wchar_t *Name,BOOL OnlyPartName)
{
	return GoToFile(FindFile(Name,OnlyPartName));
}

long TreeList::FindFile(const wchar_t *Name,BOOL OnlyPartName)
{
	for (size_t i=0; i < ListData.size(); ++i)
	{
		const wchar_t *CurPtrName=OnlyPartName?PointToName(ListData[i]->strName):ListData[i]->strName.CPtr();

		if (!StrCmp(Name,CurPtrName))
			return static_cast<long>(i);

		if (!StrCmpI(Name,CurPtrName))
			return static_cast<long>(i);
	}

	return -1;
}

long TreeList::FindFirst(const wchar_t *Name)
{
	return FindNext(0,Name);
}

long TreeList::FindNext(int StartPos, const wchar_t *Name)
{
	if (static_cast<size_t>(StartPos) < ListData.size())
	{
		for (size_t i = StartPos; i < ListData.size(); ++i)
		{
			if (CmpName(Name,ListData[i]->strName,true))
				if (!TestParentFolderName(ListData[i]->strName))
					return static_cast<long>(i);
		}
	}

	return -1;
}

int TreeList::GetFileName(string &strName,int Pos,DWORD &FileAttr)
{
	if (Pos < 0 || static_cast<size_t>(Pos) >= ListData.size())
		return FALSE;

	strName = ListData[Pos]->strName;
	FileAttr=FILE_ATTRIBUTE_DIRECTORY|apiGetFileAttributes(ListData[Pos]->strName);
	return TRUE;
}

/* $ 16.10.2000 tran
 функция, определяющаяя необходимость кеширования
 файла */
int TreeList::MustBeCached(const wchar_t *Root)
{
	UINT type;
	type=FAR_GetDriveType(Root);

	if (type==DRIVE_UNKNOWN ||
	        type==DRIVE_NO_ROOT_DIR ||
	        type==DRIVE_REMOVABLE ||
	        IsDriveTypeCDROM(type)
	   )
	{
		if (type==DRIVE_REMOVABLE)
		{
			if (Upper(Root[0])==L'A' || Upper(Root[0])==L'B')
				return FALSE; // это дискеты
		}

		return TRUE;
		// кешируются CD, removable и неизвестно что :)
	}

	/* остались
	    DRIVE_REMOTE
	    DRIVE_RAMDISK
	    DRIVE_FIXED
	*/
	return FALSE;
}

void TreeList::SetFocus()
{
	Panel::SetFocus();
	SetTitle();
	SetMacroMode(FALSE);
}

void TreeList::KillFocus()
{
	if (static_cast<size_t>(CurFile) < ListData.size())
	{
		if (apiGetFileAttributes(ListData[CurFile]->strName)==INVALID_FILE_ATTRIBUTES)
		{
			DelTreeName(ListData[CurFile]->strName);
			Update(UPDATE_KEEP_SELECTION);
		}
	}

	Panel::KillFocus();
	SetMacroMode(TRUE);
}

void TreeList::SetMacroMode(int Restore)
{
	if (!Global->CtrlObject)
		return;

	if (PrevMacroMode == MACRO_INVALID)
		PrevMacroMode = Global->CtrlObject->Macro.GetMode();

	Global->CtrlObject->Macro.SetMode(Restore ? PrevMacroMode:MACRO_TREEPANEL);
}

BOOL TreeList::UpdateKeyBar()
{
	KeyBar *KB = Global->CtrlObject->MainKeyBar;
	KB->SetAllGroup(KBL_MAIN, MKBTreeF1, 12);
	KB->SetAllGroup(KBL_SHIFT, MKBTreeShiftF1, 12);
	KB->SetAllGroup(KBL_ALT, MKBTreeAltF1, 12);
	KB->SetAllGroup(KBL_CTRL, MKBTreeCtrlF1, 12);
	KB->SetAllGroup(KBL_CTRLSHIFT, MKBTreeCtrlShiftF1, 12);
	KB->SetAllGroup(KBL_CTRLALT, MKBTreeCtrlAltF1, 12);
	KB->SetAllGroup(KBL_ALTSHIFT, MKBTreeAltShiftF1, 12);
	KB->SetAllGroup(KBL_CTRLALTSHIFT, MKBTreeCtrlAltShiftF1, 12);
	DynamicUpdateKeyBar();
	return TRUE;
}

void TreeList::DynamicUpdateKeyBar()
{
	KeyBar *KB = Global->CtrlObject->MainKeyBar;
	KB->ReadRegGroup(L"Tree",Global->Opt->strLanguage);
	KB->SetAllRegGroup();
}

void TreeList::SetTitle()
{
	if (GetFocus())
	{
		string strTitleDir(L"{");

		const wchar_t *Ptr=ListData.empty()? L"" : ListData[CurFile]->strName.CPtr();

		if (*Ptr)
		{
			strTitleDir += Ptr;
			strTitleDir += L" - ";
		}

		strTitleDir += L"Tree}";

		ConsoleTitle::SetFarTitle(strTitleDir);
	}
}

// TODO: Файлы "Tree3.Far" для локальных дисков должны храниться в "Local AppData\Far Manager"
// TODO: Файлы "Tree3.Far" для сменных дисков должны храниться на самих "дисках"
// TODO: Файлы "Tree3.Far" для сетевых дисков должны храниться в "%HOMEDRIVE%\%HOMEPATH%",
//                        если эти переменные среды не определены, то "%APPDATA%\Far Manager"
string &TreeList::MkTreeFileName(const wchar_t *RootDir,string &strDest)
{
	strDest = RootDir;
	AddEndSlash(strDest);
	strDest += L"tree3.far";
	return strDest;
}

// этому каталогу (Tree.Cache) место не в FarPath, а в "Local AppData\Far\"
string &TreeList::MkTreeCacheFolderName(const wchar_t *RootDir,string &strDest)
{
	strDest = RootDir;
	AddEndSlash(strDest);
	strDest += L"tree3.cache";
	return strDest;
}

/*
  Global->Opt->Tree.LocalDisk          Хранить файл структуры папок для локальных дисков
  Global->Opt->Tree.NetDisk            Хранить файл структуры папок для сетевых дисков
  Global->Opt->Tree.NetPath            Хранить файл структуры папок для сетевых путей
  Global->Opt->Tree.RemovableDisk      Хранить файл структуры папок для сменных дисков
  Global->Opt->Tree.CDDisk             Хранить файл структуры папок для CD/DVD/BD/etc дисков

  Global->Opt->Tree.strLocalDisk;      шаблон имени файла-деревяхи для локальных дисков
     constLocalDiskTemplate=L"%D.%SN.tree"
  Global->Opt->Tree.strNetDisk;        шаблон имени файла-деревяхи для сетевых дисков
     constNetDiskTemplate=L"%D.%SN.tree";
  Global->Opt->Tree.strNetPath;        шаблон имени файла-деревяхи для сетевых путей
     constNetPathTemplate=L"%SR.%SH.tree";
  Global->Opt->Tree.strRemovableDisk;  шаблон имени файла-деревяхи для сменных дисков
     constRemovableDiskTemplate=L"%SN.tree";
  Global->Opt->Tree.strCDDisk;         шаблон имени файла-деревяхи для CD/DVD/BD/etc дисков
     constCDDiskTemplate=L"CD.%L.%SN.tree";

     %D    - буква диска
     %SN   - серийный номер
     %L    - метка диска
     %SR   - server name
     %SH   - share name

  Global->Opt->Tree.strExceptPath;     // для перечисленных здесь не хранить

  Global->Opt->Tree.strSaveLocalPath;  // сюда сохраняем локальные диски
  Global->Opt->Tree.strSaveNetPath;    // сюда сохраняем сетевые диски
*/
string &TreeList::CreateTreeFileName(const wchar_t *Path,string &strDest)
{
#if 0
	char RootPath[NM];
	RootPath = ExtractPathRoot(Path);
	UINT DriveType = FAR_GetDriveType(RootPath,nullptr,FALSE);
	// получение инфы о томе
	char VolumeName[NM],FileSystemName[NM];
	DWORD MaxNameLength,FileSystemFlags,VolumeNumber;

	if (!GetVolumeInformation(RootDir,VolumeName,sizeof(VolumeName),&VolumeNumber,
	                          &MaxNameLength,&FileSystemFlags,
	                          FileSystemName,sizeof(FileSystemName)))
		Global->Opt->Tree.SavedTreePath
#endif
		return strDest;
}

BOOL TreeList::GetItem(int Index,void *Dest)
{
	if (Index == -1 || Index == -2)
		Index=GetCurrentPos();

	if (static_cast<size_t>(Index) >= ListData.size())
		return FALSE;

	*((TreeItem *)Dest) = *ListData[Index];
	return TRUE;
}

int TreeList::GetCurrentPos()
{
	return CurFile;
}

bool TreeList::SaveState()
{
	SaveListData.clear();
	SaveWorkDir=0;

	if (!ListData.empty())
	{
		SaveListData.resize(ListData.size());

		for (size_t i=0; i < ListData.size(); ++i)
			SaveListData[i] = *ListData[i];

		SaveWorkDir=WorkDir;
		*Global->tempTreeCache = *Global->TreeCache;
		return true;
	}

	return false;
}

bool TreeList::RestoreState()
{
	std::for_each(RANGE(ListData, i)
	{
		delete i;
	});
	ListData.clear();

	WorkDir=0;

	if (!SaveListData.empty())
	{
		ListData.resize(SaveListData.size());
		for (size_t i=0; i < SaveListData.size(); ++i)
		{
			auto NewItem = new TreeItem;
			*NewItem = SaveListData[i];
			ListData.push_back(NewItem);
		}

		WorkDir=SaveWorkDir;
		*Global->TreeCache = *Global->tempTreeCache;
		Global->tempTreeCache->Clean();
		return true;
	}

	return false;
}
