/*
treelist.cpp

Tree panel

*/

#include "headers.hpp"
#pragma hdrstop

#include "treelist.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "plugin.hpp"
#include "global.hpp"
#include "colors.hpp"
#include "lang.hpp"
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
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"

#define DELTA_TREECOUNT 31

static int _cdecl SortList(const void *el1,const void *el2);
static int _cdecl SortCacheList(const void *el1,const void *el2);
static int StaticSortCaseSensitive;
static int StaticSortNumeric;
static int TreeCmp(char *Str1,char *Str2);
static clock_t TreeStartTime;
static int LastScrX = -1;
static int LastScrY = -1;

static char TreeLineSymbol[4][3]=
{
	{0x20,0x20,/*0x20,*/0x00},
	{0xB3,0x20,/*0x20,*/0x00},
	{0xC0,0xC4,/*0xC4,*/0x00},
	{0xC3,0xC4,/*0xC4,*/0x00},
};

#if defined(USE_WFUNC)
static WCHAR TreeLineSymbolW[4][3]={0};
#endif

static struct TreeListCache
{
	char TreeName[NM];
	char **ListName;
	int TreeCount;
	int TreeSize;

	TreeListCache()
	{
		ListName=NULL;
		TreeCount=0;
		TreeSize=0;
	}

	void Resize()
	{
		if (TreeCount==TreeSize)
		{
			TreeSize+=TreeSize?TreeSize>>2:32;
			char **NewPtr=(char**)xf_realloc(ListName,sizeof(char*)*TreeSize);

			if (!NewPtr)return;

			ListName=NewPtr;
		}
	}

	void Add(const char* name)
	{
		Resize();
		ListName[TreeCount++]=xf_strdup(name);
	}

	void Insert(int idx,const char* name)
	{
		Resize();
		memmove(ListName+idx+1,ListName+idx,sizeof(char*)*(TreeCount-idx));
		ListName[idx]=xf_strdup(name);
		TreeCount++;
	}

	void Delete(int idx)
	{
		if (ListName[idx]) xf_free(ListName[idx]);

		memmove(ListName+idx,ListName+idx+1,sizeof(char*)*(TreeCount-idx-1));
		TreeCount--;
	}

	void Clean()
	{
		if (!TreeSize)return;

		for (int i=0; i<TreeCount; i++)
		{
			if (ListName[i]) xf_free(ListName[i]);
		}

		if (ListName) xf_free(ListName);

		ListName=NULL;
		TreeCount=0;
		TreeSize=0;
		TreeName[0]=0;
	}

	//TODO: необходимо оптимизировать!
	void Copy(struct TreeListCache *Dest)
	{
		Dest->Clean();

		for (int I=0; I < TreeCount; I++)
			Dest->Add(ListName[I]);
	}

} TreeCache, tempTreeCache;


TreeList::TreeList(int IsPanel)
{
	Type=TREE_PANEL;
	ListData=NULL;
	SaveListData=NULL;
	TreeCount=0;
	WorkDir=CurFile=CurTopFile=0;
	GetSelPosition=0;
	*Root=0;
	Flags.Set(FTREELIST_UPDATEREQUIRED);
	CaseSensitiveSort=FALSE;
	NumericSort=FALSE;
	PrevMacroMode = -1;
	Flags.Clear(FTREELIST_TREEISPREPARED);
	ExitCode=1;
	Flags.Change(FTREELIST_ISPANEL,IsPanel);
}


TreeList::~TreeList()
{
	if (ListData)     xf_free(ListData);

	if (SaveListData) xf_free(SaveListData);

	tempTreeCache.Clean();
	FlushCache();
	SetMacroMode(TRUE);
}

void TreeList::SetRootDir(char *NewRootDir)
{
	xstrncpy(Root,NewRootDir,sizeof(Root)-1);
	xstrncpy(CurDir,NewRootDir,sizeof(CurDir)-1);
}

void TreeList::DisplayObject()
{
	if (Flags.Check(FSCROBJ_ISREDRAWING))
		return;

	Flags.Set(FSCROBJ_ISREDRAWING);

	if (Flags.Check(FTREELIST_UPDATEREQUIRED))
		Update(0);

	DisplayTree(FALSE);

	if (ExitCode)
	{
		Panel *RootPanel=GetRootPanel();

		if (RootPanel->GetType()==FILE_PANEL)
		{
			int RootCaseSensitive=((FileList *)RootPanel)->IsCaseSensitive();
			int RootNumeric=RootPanel->GetNumericSort();

			if (RootCaseSensitive!=CaseSensitiveSort || RootNumeric != NumericSort)
			{
				CaseSensitiveSort=RootCaseSensitive;
				NumericSort=RootNumeric;
				StaticSortCaseSensitive=CaseSensitiveSort;
				StaticSortNumeric=NumericSort;
				far_qsort(ListData,TreeCount,sizeof(*ListData),SortList);
				FillLastData();
				SyncDir();
			}
		}
	}

	Flags.Clear(FSCROBJ_ISREDRAWING);
}


const char *TreeList::GetTitle(char *lTitle,int LenTitle,int TruncSize)
{
	char Title[512];
	sprintf(Title," %s ",ModalMode ? MSG(MFindFolderTitle):MSG(MTreeTitle));
	TruncStr(Title,X2-X1-3);
	xstrncpy(lTitle,Title,LenTitle);
	return lTitle;
}

void TreeList::DisplayTree(int Fast)
{
	int I,J,K;
	struct TreeItem *CurPtr;
	char Title[100];
	LockScreen *LckScreen=NULL;

	if (CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == QVIEW_PANEL)
		LckScreen=new LockScreen;

	CorrectPosition();

	if (TreeCount>0)
		xstrncpy(CurDir,ListData[CurFile].Name,sizeof(CurDir)-1);

	if (!Fast)
	{
		Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
		DrawSeparator(Y2-2-(ModalMode!=0));
		GetTitle(Title,sizeof(Title)-1);

		if (*Title)
		{
			SetColor((Focus || ModalMode) ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
			GotoXY(X1+(X2-X1+1-(int)strlen(Title))/2,Y1);
			Text(Title);
		}
	}

	for (I=Y1+1,J=CurTopFile; I<Y2-2-(ModalMode!=0); I++,J++)
	{
		CurPtr=&ListData[J];
		GotoXY(X1+1,I);
		SetColor(COL_PANELTEXT);
		Text(" ");

		if (J<TreeCount && Flags.Check(FTREELIST_TREEISPREPARED))
		{
			if (J==0)
				DisplayTreeName("\\",J);
			else
			{
#if defined(USE_WFUNC)

				// первоначальная инициализация
				if (TreeLineSymbolW[0][0] == 0x0000)
				{
					for (int IW=0; IW < 4; ++IW)
					{
						for (int JW=0; JW < 2; ++JW)
							TreeLineSymbolW[IW][JW]=(TreeLineSymbol[IW][JW] == 0x20)?0x20:BoxSymbols[TreeLineSymbol[IW][JW]-0x0B0];
					}
				}

				WCHAR OutStrW[200];
				*OutStrW=0;
#endif
				char  OutStr[200];

				for (*OutStr=0,K=0; K<CurPtr->Depth-1 && WhereX()+3*K<X2-6; K++)
				{
					if (CurPtr->Last[K])
					{
#if defined(USE_WFUNC)

						if (Opt.UseUnicodeConsole)
							wcscat(OutStrW,TreeLineSymbolW[0]);
						else
#endif
							strcat(OutStr,TreeLineSymbol[0]);
					}
					else
					{
#if defined(USE_WFUNC)

						if (Opt.UseUnicodeConsole)
							wcscat(OutStrW,TreeLineSymbolW[1]);
						else
#endif
							strcat(OutStr,TreeLineSymbol[1]);
					}
				}

				if (CurPtr->Last[CurPtr->Depth-1])
				{
#if defined(USE_WFUNC)

					if (Opt.UseUnicodeConsole)
						wcscat(OutStrW,TreeLineSymbolW[2]);
					else
#endif
						strcat(OutStr,TreeLineSymbol[2]);
				}
				else
				{
#if defined(USE_WFUNC)

					if (Opt.UseUnicodeConsole)
						wcscat(OutStrW,TreeLineSymbolW[3]);
					else
#endif
						strcat(OutStr,TreeLineSymbol[3]);
				}

#if defined(USE_WFUNC)

				if (Opt.UseUnicodeConsole)
					BoxTextW(OutStrW,FALSE);
				else
#endif
					Text(OutStr);

				char *ChPtr=strrchr(CurPtr->Name,'\\');

				if (ChPtr!=NULL)
					DisplayTreeName(ChPtr+1,J);
			}
		}

		SetColor(COL_PANELTEXT);

		if ((K=WhereX())<X2)
			mprintf("%*s",X2-WhereX(),"");
	}

	if (Opt.ShowPanelScrollbar)
	{
		SetColor(COL_PANELSCROLLBAR);
		ScrollBar(X2,Y1+1,Y2-Y1-3,CurFile,TreeCount>1 ? TreeCount-1:TreeCount);
	}

	SetColor(COL_PANELTEXT);
	SetScreen(X1+1,Y2-(ModalMode?2:1),X2-1,Y2-1,' ',COL_PANELTEXT);

	if (TreeCount>0)
	{
		GotoXY(X1+1,Y2-1);
		mprintf("%-*.*s",X2-X1-1,X2-X1-1,ListData[CurFile].Name);
	}

	UpdateViewPanel();
	SetTitle(); // не забудим прорисовать заголовок

	if (LckScreen)
		delete LckScreen;
}


void TreeList::DisplayTreeName(char *Name,int Pos)
{
	if (WhereX()>X2-4)
		GotoXY(X2-4,WhereY());

	if (Pos==CurFile)
	{
		GotoXY(WhereX()-1,WhereY());

		if (Focus || ModalMode)
		{
			SetColor((Pos==WorkDir) ? COL_PANELSELECTEDCURSOR:COL_PANELCURSOR);
			mprintf(" %.*s ",X2-WhereX()-3,Name);
		}
		else
		{
			SetColor((Pos==WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
			mprintf("[%.*s]",X2-WhereX()-3,Name);
		}
	}
	else
	{
		SetColor((Pos==WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
		mprintf("%.*s",X2-WhereX()-1,Name);
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
	int LastTreeCount=TreeCount;
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

	if (RetFromReadTree && TreeCount>0 && ((Mode & UPDATE_KEEP_SELECTION)==0 || LastTreeCount!=TreeCount))
	{
		SyncDir();
		struct TreeItem *CurPtr=ListData+CurFile;

		if (GetFileAttributes(CurPtr->Name)==(DWORD)-1)
		{
			DelTreeName(CurPtr->Name);
			Update(UPDATE_KEEP_SELECTION);
			//  Show();
		}
	}
	else if (!RetFromReadTree)
	{
		//Show();
		if (!Flags.Check(FTREELIST_ISPANEL))
		{
			Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
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
	WIN32_FIND_DATA fdata;
	char FullName[NM];
	SaveState();
	FlushCache();
	GetRoot();
	int RootLength=(int)strlen(Root)-1;

	if (RootLength<0)
		RootLength=0;

	if (ListData) xf_free(ListData);

	TreeCount=0;

	if ((ListData=(struct TreeItem*)xf_malloc((TreeCount+256+1)*sizeof(struct TreeItem)))==NULL)
	{
		RestoreState();
		return FALSE;
	}

	memset(&ListData[0], 0, sizeof(ListData[0]));
	strcpy(ListData->Name,Root);
	SaveScreen SaveScrTree;
	UndoGlobalSaveScrPtr UndSaveScr(&SaveScrTree);
	/* Т.к. мы можем вызвать диалог подтверждения (который не перерисовывает панельки,
	   а восстанавливает сохраненный образ экрана, то нарисуем чистую панель */
	//Redraw();

	if (RootLength>0 && Root[RootLength-1]!=':' && Root[RootLength]=='\\')
		ListData->Name[RootLength]=0;

	TreeCount=1;
	int FirstCall=TRUE, AscAbort=FALSE;
	TreeStartTime = clock();
	RefreshFrameManager frref(ScrX,ScrY,TreeStartTime,FALSE);//DontRedrawFrame);
	ScTree.SetFindPath(Root,"*.*",0);
	LastScrX = ScrX;
	LastScrY = ScrY;
	TaskBar TB;

	while (ScTree.GetNextName(&fdata,FullName, sizeof(FullName)-1))
	{
//    if(TreeCount > 3)
		TreeList::MsgReadTree(TreeCount,FirstCall);

		if (CheckForEscSilent())
		{
			AscAbort=ConfirmAbortOp()!=0;
			FirstCall=TRUE;
		}

		if (AscAbort)
			break;

		struct TreeItem *NewListData=NULL;

		if ((TreeCount & 255)==0 && (NewListData=(struct TreeItem *)xf_realloc(ListData,(TreeCount+256+1)*sizeof(struct TreeItem)))==NULL)
		{
			AscAbort=TRUE;
			break;
		}

		if (NewListData)
			ListData=NewListData;

		if (!(fdata.dwFileAttributes & FA_DIREC))
			continue;

		memset(&ListData[TreeCount], 0, sizeof(ListData[0]));
		strcpy(ListData[TreeCount++].Name,FullName);
	}

	if (AscAbort && !Flags.Check(FTREELIST_ISPANEL))
	{
		if (ListData) xf_free(ListData);

		ListData=NULL;
		TreeCount=0;
		RestoreState();
		return FALSE;
	}

	StaticSortCaseSensitive=CaseSensitiveSort=StaticSortNumeric=NumericSort=FALSE;
	far_qsort(ListData,TreeCount,sizeof(*ListData),SortList);
	FillLastData();

	if (!AscAbort)
		SaveTreeFile();

	if (!FirstCall && !Flags.Check(FTREELIST_ISPANEL))
	{ // Перерисуем другую панель - удалим следы сообщений :)
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
		AnotherPanel->Redraw();
	}

	return TRUE;
}

#ifdef _MSC_VER
#pragma warning(disable:4018)
#endif
void TreeList::SaveTreeFile()
{
	if (TreeCount<4)
		return;

	char Name[NM];
	FILE *TreeFile;
	long I;
	int RootLength=(int)strlen(Root)-1;

	if (RootLength<0)
		RootLength=0;

	MkTreeFileName(Root,Name,sizeof(Name)-1);
	// получим и сразу сбросим атрибуты (если получится)
	DWORD FileAttributes=GetFileAttributes(Name);

	if (FileAttributes != -1)
		SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);

	if ((TreeFile=fopen(Name,"wb"))==NULL)
	{
		/* $ 16.10.2000 tran
		   если диск должен кешироваться, то и пытаться не стоит */
		if (MustBeCached(Root) || (TreeFile=fopen(Name,"wb"))==NULL)
			if (!GetCacheTreeName(Root,Name,TRUE) || (TreeFile=fopen(Name,"wb"))==NULL)
				return;

		/* tran $ */
	}

	for (I=0; I<TreeCount; I++)
		if (RootLength>=strlen(ListData[I].Name))
			fprintf(TreeFile,"\\\n");
		else
			fprintf(TreeFile,"%s\n",ListData[I].Name+RootLength);

	if (fclose(TreeFile)==EOF)
	{
		clearerr(TreeFile);
		fclose(TreeFile);
		remove(Name);
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotSaveTree),Name,MSG(MOk));
	}
	else if (FileAttributes != -1) // вернем атрибуты (если получится :-)
		SetFileAttributes(Name,FileAttributes);
}


int TreeList::GetCacheTreeName(char *Root,char *Name,int CreateDir)
{
	char VolumeName[NM],FileSystemName[NM];
	DWORD MaxNameLength,FileSystemFlags,VolumeNumber;

	if (!GetVolumeInformation(Root,VolumeName,sizeof(VolumeName),&VolumeNumber,
	                          &MaxNameLength,&FileSystemFlags,
	                          FileSystemName,sizeof(FileSystemName)))
		return(FALSE);

	char FolderName[NM];
	MkTreeCacheFolderName(FarPath,FolderName,sizeof(FolderName)-1);

	if (CreateDir)
	{
		mkdir(FolderName);
		SetFileAttributes(FolderName,Opt.Tree.TreeFileAttr);
	}

	char RemoteName[NM*3];
	*RemoteName=0;

	if (*Root=='\\')
		strcpy(RemoteName,Root);
	else
	{
		char LocalName [8];
		strcpy(LocalName, "A:");
		*LocalName=*Root;
		DWORD RemoteNameSize=sizeof(RemoteName);
		WNetGetConnection(LocalName,RemoteName,&RemoteNameSize);

		if (*RemoteName)
			AddEndSlash(RemoteName);
	}

	for (int I=0; RemoteName[I]!=0; I++)
		if (RemoteName[I]=='\\')
			RemoteName[I]='_';

	sprintf(Name,"%s\\%s.%x.%s.%s",FolderName,VolumeName,VolumeNumber,
	        FileSystemName,RemoteName);
	return(TRUE);
}


void TreeList::GetRoot()
{
	char PanelDir[NM];
	Panel *RootPanel=GetRootPanel();
	RootPanel->GetCurDir(PanelDir);
	GetPathRoot(PanelDir,Root);
}


Panel* TreeList::GetRootPanel()
{
	Panel *RootPanel;

	if (ModalMode)
	{
		if (ModalMode==MODALTREE_ACTIVE)
			RootPanel=CtrlObject->Cp()->ActivePanel;
		else if (ModalMode==MODALTREE_FREE)
			RootPanel=this;
		else
		{
			RootPanel=CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);

			if (!RootPanel->IsVisible())
				RootPanel=CtrlObject->Cp()->ActivePanel;
		}
	}
	else
		RootPanel=CtrlObject->Cp()->GetAnotherPanel(this);

	return(RootPanel);
}


void TreeList::SyncDir()
{
	char PanelDir[NM];
	Panel *AnotherPanel=GetRootPanel();
	AnotherPanel->GetCurDir(PanelDir);

	if (*PanelDir)
		if (AnotherPanel->GetType()==FILE_PANEL)
		{
			if (!SetDirPosition(PanelDir))
			{
				ReadSubTree(PanelDir);
				ReadTreeFile();
				SetDirPosition(PanelDir);
			}
		}
		else
			SetDirPosition(PanelDir);
}


void TreeList::PR_MsgReadTree(void)
{
	int FirstCall=1;
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	TreeList::MsgReadTree(preRedrawItem.Param.Flags,FirstCall);
}

int TreeList::MsgReadTree(int TreeCount,int &FirstCall)
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
		char NumStr[32];
		itoa(TreeCount,NumStr,10);
		Message((FirstCall ? 0:MSG_KEEPBACKGROUND),0,MSG(MTreeTitle),
		        MSG(MReadingTree),NumStr);
		PreRedrawItem preRedrawItem=PreRedraw.Peek();
		preRedrawItem.Param.Flags=TreeCount;
		PreRedraw.SetParam(preRedrawItem.Param);
		TreeStartTime = clock();
	}

	return(1);
}


void TreeList::FillLastData()
{
	long Last,Depth,PathLength,SubDirPos,I,J;
	int RootLength=(int)strlen(Root)-1;

	if (RootLength<0)
		RootLength=0;

	for (I=1; I<TreeCount; I++)
	{
		PathLength=(int)(strrchr(ListData[I].Name,'\\')-ListData[I].Name+1);
		Depth=ListData[I].Depth=CountSlash(ListData[I].Name+RootLength);

		for (J=I+1,SubDirPos=I,Last=1; J<TreeCount; J++)
			if (CountSlash(ListData[J].Name+RootLength)>Depth)
			{
				SubDirPos=J;
				continue;
			}
			else
			{
				if (LocalStrnicmp(ListData[I].Name,ListData[J].Name,PathLength)==0)
					Last=0;

				break;
			}

		for (J=I; J<=SubDirPos; J++)
			ListData[J].Last[Depth-1]=Last;
	}
}


int TreeList::CountSlash(char *Str)
{
	int Count=0;

	while ((Str=strchr(Str,'\\'))!=NULL)
	{
		Str++;
		Count++;
	}

	return(Count);
}


__int64 TreeList::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return (__int64)(TreeCount<=0);
		case MCODE_C_EOF:
			return (__int64)(CurFile==TreeCount-1);
		case MCODE_C_BOF:
			return (__int64)(CurFile==0);
		case MCODE_C_SELECTED:
			return _i64(0);
		case MCODE_V_ITEMCOUNT:
			return (__int64)TreeCount;
		case MCODE_V_CURPOS:
			return (__int64)(CurFile+1);
	}

	return _i64(0);
}

int TreeList::ProcessKey(int Key)
{
	struct TreeItem *CurPtr;

	if (!IsVisible())
		return(FALSE);

	if (TreeCount==0 && Key!=KEY_CTRLR)
		return(FALSE);

	if (SaveFolderShortcut(Key,CurDir,"","",""))
		return(TRUE);

	if (ProcessShortcutFolder(Key,TRUE))
		return(TRUE);

	switch (Key)
	{
			/* $ 08.12.2001 IS просят справку для "дерева", ее и покажем
			*/
		case KEY_F1:
		{
			{
				Help Hlp("TreePanel");
			}
			return TRUE;
		}
		/* IS $ */
		case KEY_SHIFTNUMENTER:
		case KEY_CTRLNUMENTER:
		case KEY_SHIFTENTER:
		case KEY_CTRLENTER:
		case KEY_CTRLF:
		case KEY_CTRLALTINS:  case KEY_CTRLALTNUMPAD0:
		{
			{
				char QuotedName[NM+2];
				int CAIns=(Key == KEY_CTRLALTINS || Key == KEY_CTRLALTNUMPAD0);
				CurPtr=ListData+CurFile;

				if (strchr(CurPtr->Name,' ')!=NULL)
					sprintf(QuotedName,"\"%s\"%s",CurPtr->Name,(CAIns?"":" "));
				else
					sprintf(QuotedName,"%s%s",CurPtr->Name,(CAIns?"":" "));

				if (CAIns)
					CopyToClipboard(QuotedName);
				else if (Key == KEY_SHIFTENTER||Key == KEY_SHIFTNUMENTER)
					Execute(QuotedName,FALSE,TRUE,TRUE);
				else
					CtrlObject->CmdLine->InsertString(QuotedName);
			}
			return(TRUE);
		}
		case KEY_CTRLBACKSLASH:
		{
			CurFile=0;
			ProcessEnter();
			return(TRUE);
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!ModalMode && CtrlObject->CmdLine->GetLength()>0)
				break;

			ProcessEnter();
			return(TRUE);
		}
		case KEY_F4:
		case KEY_CTRLA:
		{
			if (SetCurPath())
				ShellSetFileAttributes(this);

			return(TRUE);
		}
		case KEY_CTRLR:
		{
			ReadTree();

			if (TreeCount>0)
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
				ShellCopy ShCopy(this,Key==KEY_SHIFTF6,FALSE,TRUE,TRUE,ToPlugin,NULL);
			}

			return(TRUE);
		}
		case KEY_F5:
		case KEY_DRAGCOPY:
		case KEY_F6:
		case KEY_ALTF6:
		case KEY_DRAGMOVE:
		{
			if (SetCurPath() && TreeCount>0)
			{
				Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
				int Ask=(Key!=KEY_DRAGCOPY && Key!=KEY_DRAGMOVE || Opt.Confirm.Drag);
				int Move=(Key==KEY_F6 || Key==KEY_DRAGMOVE);
				int ToPlugin=AnotherPanel->GetMode()==PLUGIN_PANEL &&
				             AnotherPanel->IsVisible() &&
				             !CtrlObject->Plugins.UseFarCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES);
				int Link=(Key==KEY_ALTF6 && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5 && !ToPlugin);

				if (Key==KEY_ALTF6 && !Link) // молча отвалим :-)
					return TRUE;

				{
					ShellCopy ShCopy(this,Move,Link,FALSE,Ask,ToPlugin,NULL);
				}

				if (ToPlugin==1)
				{
					struct PluginPanelItem *ItemList=new PluginPanelItem[1];
					int ItemNumber=1;
					HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();
					FileList::FileNameToPluginItem(ListData[CurFile].Name,ItemList);
					int PutCode=CtrlObject->Plugins.PutFiles(hAnotherPlugin,ItemList,ItemNumber,Move,0);

					if (PutCode==1 || PutCode==2)
						AnotherPanel->SetPluginModified();

					/* $ 13.07.2000 SVS
					   не надо смешивать new/delete с realloc
					*/
					if (ItemList) xf_free(ItemList);

					/* SVS $ */
					if (Move)
						ReadSubTree(ListData[CurFile].Name);

					Update(0);
					Redraw();
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);
					AnotherPanel->Redraw();
				}
			}

			return(TRUE);
		}
		case KEY_F7:
		{
			if (SetCurPath())
				ShellMakeDir(this);

			return(TRUE);
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
		case KEY_SHIFTNUMDEL:
		case KEY_SHIFTDECIMAL:
		case KEY_SHIFTDEL:
		case KEY_ALTNUMDEL:
		case KEY_ALTDECIMAL:
		case KEY_ALTDEL:
		{
			if (SetCurPath())
			{
				int SaveOpt=Opt.DeleteToRecycleBin;

				if (Key==KEY_SHIFTDEL||Key==KEY_SHIFTNUMDEL||Key==KEY_SHIFTDECIMAL)
					Opt.DeleteToRecycleBin=0;

				ShellDelete(this,Key==KEY_ALTDEL||Key==KEY_ALTNUMDEL||Key==KEY_ALTDECIMAL);
				// Надобно не забыть обновить противоположную панель...
				Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
				AnotherPanel->Update(UPDATE_KEEP_SELECTION);
				AnotherPanel->Redraw();
				Opt.DeleteToRecycleBin=SaveOpt;

				if (Opt.Tree.AutoChangeFolder && !ModalMode)
					ProcessKey(KEY_ENTER);
			}

			return(TRUE);
		}
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		{
			Scroll(Key & KEY_ALT?-1:-Opt.MsWheelDelta);
			return(TRUE);
		}
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		{
			Scroll(Key & KEY_ALT?1:Opt.MsWheelDelta);
			return(TRUE);
		}
		case KEY_MSWHEEL_LEFT:
		case(KEY_MSWHEEL_LEFT | KEY_ALT):
		{
			int Roll = Key & KEY_ALT?1:Opt.MsHWheelDelta;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_LEFT);

			return TRUE;
		}
		case KEY_MSWHEEL_RIGHT:
		case(KEY_MSWHEEL_RIGHT | KEY_ALT):
		{
			int Roll = Key & KEY_ALT?1:Opt.MsHWheelDelta;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_RIGHT);

			return TRUE;
		}
		case KEY_HOME:        case KEY_NUMPAD7:
		{
			Up(0x7fffff);

			if (Opt.Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return(TRUE);
		}
		case KEY_ADD: // OFM: Gray+/Gray- navigation
		{
			CurFile=GetNextNavPos();

			if (Opt.Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);
			else
				DisplayTree(TRUE);

			return TRUE;
		}
		case KEY_SUBTRACT: // OFM: Gray+/Gray- navigation
		{
			CurFile=GetPrevNavPos();

			if (Opt.Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);
			else
				DisplayTree(TRUE);

			return TRUE;
		}
		case KEY_END:         case KEY_NUMPAD1:
		{
			Down(0x7fffff);

			if (Opt.Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return(TRUE);
		}
		case KEY_UP:          case KEY_NUMPAD8:
		{
			Up(1);

			if (Opt.Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return(TRUE);
		}
		case KEY_DOWN:        case KEY_NUMPAD2:
		{
			Down(1);

			if (Opt.Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return(TRUE);
		}
		case KEY_PGUP:        case KEY_NUMPAD9:
		{
			CurTopFile-=Y2-Y1-3-ModalMode;
			CurFile-=Y2-Y1-3-ModalMode;
			DisplayTree(TRUE);

			if (Opt.Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return(TRUE);
		}
		case KEY_PGDN:        case KEY_NUMPAD3:
		{
			CurTopFile+=Y2-Y1-3-ModalMode;
			CurFile+=Y2-Y1-3-ModalMode;
			DisplayTree(TRUE);

			if (Opt.Tree.AutoChangeFolder && !ModalMode)
				ProcessKey(KEY_ENTER);

			return(TRUE);
		}
		default:

			if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+255 ||
			        Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+255)
			{
				FastFind(Key);

				if (Opt.Tree.AutoChangeFolder && !ModalMode)
					ProcessKey(KEY_ENTER);
			}
			else
				break;

			return(TRUE);
	}

	return(FALSE);
}


int TreeList::GetNextNavPos()
{
	int NextPos=CurFile;

	if (CurFile+1 < TreeCount)
	{
		int CurDepth=ListData[CurFile].Depth;

		for (int I=CurFile+1; I < TreeCount; ++I)
			if (ListData[I].Depth == CurDepth)
			{
				NextPos=I;
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
		int CurDepth=ListData[CurFile].Depth;

		for (int I=CurFile-1; I > 0; --I)
			if (ListData[I].Depth == CurDepth)
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
	if (TreeCount==0)
	{
		CurFile=CurTopFile=0;
		return;
	}

	int Height=Y2-Y1-3-(ModalMode!=0);

	if (CurTopFile+Height>TreeCount)
		CurTopFile=TreeCount-Height;

	if (CurFile<0)
		CurFile=0;

	if (CurFile > TreeCount-1)
		CurFile=TreeCount-1;

	if (CurTopFile<0)
		CurTopFile=0;

	if (CurTopFile > TreeCount-1)
		CurTopFile=TreeCount-1;

	if (CurFile<CurTopFile)
		CurTopFile=CurFile;

	if (CurFile>CurTopFile+Height-1)
		CurTopFile=CurFile-(Height-1);
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
BOOL TreeList::SetCurDir(const char *NewDir,int ClosePlugin)
{
	char SetDir[NM];
	xstrncpy(SetDir,NewDir,sizeof(SetDir)-1);

	if (TreeCount==0)
		Update(0);

	if (TreeCount>0 && !SetDirPosition(SetDir))
	{
		Update(0);
		SetDirPosition(SetDir);
	}

	if (GetFocus())
	{
		CtrlObject->CmdLine->SetCurDir(SetDir);
		CtrlObject->CmdLine->Show();
	}

	return TRUE; //???
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


int TreeList::SetDirPosition(char *NewDir)
{
	long I;

	for (I=0; I<TreeCount; I++)
		if (LocalStricmp(NewDir,ListData[I].Name)==0)
		{
			WorkDir=CurFile=I;
			CurTopFile=CurFile-(Y2-Y1-1)/2;
			CorrectPosition();
			return(TRUE);
		}

	return(FALSE);
}


int TreeList::GetCurDir(char *CurDir)
{
	char *Ptr="";

	if (TreeCount==0)
	{
		if (ModalMode==MODALTREE_FREE)
			Ptr=Root;
	}
	else
		Ptr=ListData[CurFile].Name;

	if (CurDir)
		strcpy(CurDir,Ptr); // TODO: ОПАСНО!!!

	return (int)strlen(Ptr);
}


int TreeList::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	int OldFile=CurFile;
	int RetCode;

	if (Opt.ShowPanelScrollbar && MouseX==X2 &&
	        (MouseEvent->dwButtonState & 1) && !IsDragging())
	{
		int ScrollY=Y1+1;
		int Height=Y2-Y1-3;

		if (MouseY==ScrollY)
		{
			while (IsMouseButtonPressed())
				ProcessKey(KEY_UP);

			if (!ModalMode)
				SetFocus();

			return(TRUE);
		}

		if (MouseY==ScrollY+Height-1)
		{
			while (IsMouseButtonPressed())
				ProcessKey(KEY_DOWN);

			if (!ModalMode)
				SetFocus();

			return(TRUE);
		}

		if (MouseY>ScrollY && MouseY<ScrollY+Height-1 && Height>2)
		{
			CurFile=(TreeCount-1)*(MouseY-ScrollY)/(Height-2);
			DisplayTree(TRUE);

			if (!ModalMode)
				SetFocus();

			return(TRUE);
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

		if (TreeCount==0)
			return(TRUE);

		if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
		        MouseEvent->dwEventFlags==DOUBLE_CLICK ||
		        (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) &&
		        MouseEvent->dwEventFlags==0 ||
		        OldFile!=CurFile && Opt.Tree.AutoChangeFolder && !ModalMode)
		{
			ProcessEnter();
			return(TRUE);
		}

		return(TRUE);
	}

	if (MouseEvent->dwMousePosition.Y<=Y1+1)
	{
		if (!ModalMode)
			SetFocus();

		if (TreeCount==0)
			return(TRUE);

		while (IsMouseButtonPressed() && MouseY<=Y1+1)
			Up(1);

		if (Opt.Tree.AutoChangeFolder && !ModalMode)
			ProcessKey(KEY_ENTER);

		return(TRUE);
	}

	if (MouseEvent->dwMousePosition.Y>=Y2-2)
	{
		if (!ModalMode)
			SetFocus();

		if (TreeCount==0)
			return(TRUE);

		while (IsMouseButtonPressed() && MouseY>=Y2-2)
			Down(1);

		if (Opt.Tree.AutoChangeFolder && !ModalMode)
			ProcessKey(KEY_ENTER);

		return(TRUE);
	}

	return(FALSE);
}


void TreeList::MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	CurFile=CurTopFile+MouseEvent->dwMousePosition.Y-Y1-1;
	CorrectPosition();
}


void TreeList::ProcessEnter()
{
	struct TreeItem *CurPtr;
	DWORD Attr;
	CurPtr=ListData+CurFile;

	if ((Attr=GetFileAttributes(CurPtr->Name))!=(DWORD)-1 && (Attr & FA_DIREC))
	{
		if (!ModalMode && FarChDir(CurPtr->Name))
		{
			Panel *AnotherPanel=GetRootPanel();
			SetCurDir(CurPtr->Name,TRUE);
			Show();
			AnotherPanel->SetCurDir(CurPtr->Name,TRUE);
			AnotherPanel->Redraw();
		}
	}
	else
	{
		DelTreeName(CurPtr->Name);
		Update(UPDATE_KEEP_SELECTION);
		Show();
	}
}


int TreeList::ReadTreeFile()
{
	char Name[NM],DirName[NM],LastDirName[NM],*ChPtr;
	FILE *TreeFile=NULL;
	int RootLength=(int)strlen(Root)-1;

	if (RootLength<0)
		RootLength=0;

	//SaveState();
	FlushCache();
	MkTreeFileName(Root,Name,sizeof(Name)-1);

	if (MustBeCached(Root) || (TreeFile=fopen(Name,"rb"))==NULL)
		if (!GetCacheTreeName(Root,Name,FALSE) || (TreeFile=fopen(Name,"rb"))==NULL)
		{
			//RestoreState();
			return(FALSE);
		}

	if (ListData) xf_free(ListData);

	ListData=NULL;
	TreeCount=0;
	*LastDirName=0;
	xstrncpy(DirName,Root,sizeof(DirName)-1);

	while (fgets(DirName+RootLength,sizeof(DirName)-RootLength,TreeFile)!=NULL)
	{
		if (LocalStricmp(DirName,LastDirName)==0)
			continue;

		strcpy(LastDirName,DirName);

		if ((ChPtr=strchr(DirName,'\n'))!=NULL)
			*ChPtr=0;

		if (RootLength>0 && DirName[RootLength-1]!=':' &&
		        DirName[RootLength]=='\\' && DirName[RootLength+1]==0)
			DirName[RootLength]=0;

		struct TreeItem *NewListData=NULL;

		if ((TreeCount & 255)==0 && (NewListData=(struct TreeItem *)xf_realloc(ListData,(TreeCount+256+1)*sizeof(struct TreeItem)))==0)
		{
			if (ListData) xf_free(ListData);

			ListData=NULL;
			TreeCount=0;
			fclose(TreeFile);
			//RestoreState();
			return(FALSE);
		}

		if (NewListData)
			ListData=NewListData;

		xstrncpy(ListData[TreeCount++].Name,DirName,sizeof(ListData[0].Name)-1);
	}

	fclose(TreeFile);

	if (TreeCount==0)
		return(FALSE);

	CaseSensitiveSort=FALSE;
	NumericSort=FALSE;
	FillLastData();
	return(TRUE);
}


int TreeList::FindPartName(char *Name,int Next,int Direct,int ExcludeSets)
{
	char Mask[NM*2];
	xstrncpy(Mask,Name,sizeof(Mask)-1);
	xstrncat(Mask,"*",sizeof(Mask)-1);

	if (ExcludeSets)
	{
		ReplaceStrings(Mask,"[","<[%>",-1,1);
		ReplaceStrings(Mask,"]","[]]",-1,1);
		ReplaceStrings(Mask,"<[%>","[[]",-1,1);
	}

	int I;

	for (I=CurFile+(Next?Direct:0); I >= 0 && I < TreeCount; I+=Direct)
	{
		CmpNameSearchMode=(I==CurFile);

		if (CmpName(Mask,ListData[I].Name,TRUE))
		{
			CmpNameSearchMode=FALSE;
			CurFile=I;
			CurTopFile=CurFile-(Y2-Y1-1)/2;
			DisplayTree(TRUE);
			return(TRUE);
		}
	}

	CmpNameSearchMode=FALSE;

	for (
	    I=(Direct > 0)?0:TreeCount-1;
	    (Direct > 0) ? I < CurFile:I > CurFile;
	    I+=Direct
	)
	{
		if (CmpName(Mask,ListData[I].Name,TRUE))
		{
			CurFile=I;
			CurTopFile=CurFile-(Y2-Y1-1)/2;
			DisplayTree(TRUE);
			return(TRUE);
		}
	}

	return(FALSE);
}


int TreeList::GetSelCount()
{
	return(1);
}


int TreeList::GetSelName(char *Name,int &FileAttr,char *ShortName,WIN32_FIND_DATA *fd)
{
	if (Name==NULL)
	{
		GetSelPosition=0;
		return(TRUE);
	}

	if (GetSelPosition==0)
	{
		GetCurDir(Name);

		if (ShortName!=NULL)
			strcpy(ShortName,Name);

		FileAttr=FA_DIREC;
		GetSelPosition++;
		return(TRUE);
	}

	GetSelPosition=0;
	return(FALSE);
}


int TreeList::GetCurName(char *Name,char *ShortName)
{
	if (TreeCount==0)
	{
		*Name=*ShortName=0;
		return(FALSE);
	}

	strcpy(Name,ListData[CurFile].Name);
	strcpy(ShortName,Name);
	return(TRUE);
}


void TreeList::AddTreeName(char *Name)
{
	char FullName[NM],Root[NM],*ChPtr;
	long CachePos;

	if (*Name==0)
		return;

	ConvertNameToFull(Name,FullName, sizeof(FullName));
	Name=FullName;
	GetPathRoot(Name,Root);
	Name+=strlen(Root)-1;

	if ((ChPtr=strrchr(Name,'\\'))==NULL)
		return;

	ReadCache(Root);

	for (CachePos=0; CachePos<TreeCache.TreeCount; CachePos++)
	{
		int Result=LCStricmp(TreeCache.ListName[CachePos],Name);

		if (!Result)
			break;

		if (Result > 0)
		{
			TreeCache.Insert(CachePos,Name);
			break;
		}
	}
}


void TreeList::DelTreeName(char *Name)
{
	char FullName[NM],*DirName,Root[NM];
	long CachePos;
	int Length,DirLength;

	if (*Name==0)
		return;

	ConvertNameToFull(Name,FullName, sizeof(FullName));
	Name=FullName;
	GetPathRoot(Name,Root);
	Name+=strlen(Root)-1;
	ReadCache(Root);

	for (CachePos=0; CachePos<TreeCache.TreeCount; CachePos++)
	{
		DirName=TreeCache.ListName[CachePos];
		Length=(int)strlen(Name);
		DirLength=(int)strlen(DirName);

		if (DirLength<Length)continue;

		if (LocalStrnicmp(Name,DirName,Length)==0 &&
		        (DirName[Length]==0 || DirName[Length]=='\\'))
		{
			TreeCache.Delete(CachePos);
			CachePos--;
		}
	}
}


void TreeList::RenTreeName(char *SrcName,char *DestName)
{
	if (*SrcName==0 || *DestName==0)
		return;

	char SrcRoot[NM],DestRoot[NM];
	GetPathRoot(SrcName,SrcRoot);
	GetPathRoot(DestName,DestRoot);

	if (LocalStricmp(SrcRoot,DestRoot)!=0)
	{
		DelTreeName(SrcName);
		ReadSubTree(SrcName);
	}

	SrcName+=strlen(SrcRoot)-1;
	DestName+=strlen(DestRoot)-1;
	ReadCache(SrcRoot);
	int SrcLength=(int)strlen(SrcName);

	for (int CachePos=0; CachePos<TreeCache.TreeCount; CachePos++)
	{
		char *DirName=TreeCache.ListName[CachePos];

		if (LocalStrnicmp(SrcName,DirName,SrcLength)==0 &&
		        (DirName[SrcLength]==0 || DirName[SrcLength]=='\\'))
		{
			char NewName[2*NM];
			strcpy(NewName,DestName);
			strcat(NewName,DirName+SrcLength);

			//xstrncpy(DirName,NewName,NM-1);
			if (TreeCache.ListName[CachePos]) xf_free(TreeCache.ListName[CachePos]);

			TreeCache.ListName[CachePos]=xf_strdup(NewName);
		}
	}
}


void TreeList::ReadSubTree(char *Path)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	//SaveScreen SaveScr;
	TPreRedrawFuncGuard preRedrawFuncGuard(TreeList::PR_MsgReadTree);
	ScanTree ScTree(FALSE);
	WIN32_FIND_DATA fdata;
	char FullName[NM],DirName[NM];
	int Count=0,FileAttr;

	if ((FileAttr=GetFileAttributes(Path))==-1 || (FileAttr & FA_DIREC)==0)
		return;

	ConvertNameToFull(Path,DirName, sizeof(DirName));
	AddTreeName(DirName);
	int FirstCall=TRUE, AscAbort=FALSE;
	ScTree.SetFindPath(DirName,"*.*",0);
	LastScrX = ScrX;
	LastScrY = ScrY;

	while (ScTree.GetNextName(&fdata,FullName, sizeof(FullName)-1))
	{
		if (fdata.dwFileAttributes & FA_DIREC)
		{
			TreeList::MsgReadTree(Count+1,FirstCall);

			if (CheckForEscSilent())
			{
				AscAbort=ConfirmAbortOp()!=0;
				FirstCall=TRUE;
			}

			if (AscAbort)
				break;

			AddTreeName(FullName);
			++Count;
		}
	}
}


void TreeList::ClearCache(int EnableFreeMem)
{
	TreeCache.Clean();
}


void TreeList::ReadCache(char *TreeRoot)
{
	char TreeName[NM],DirName[NM],*ChPtr;
	FILE *TreeFile=NULL;

	if (strcmp(MkTreeFileName(TreeRoot,TreeName,sizeof(TreeName)-1),TreeCache.TreeName)==0)
		return;

	if (TreeCache.TreeCount!=0)
		FlushCache();

	if (MustBeCached(TreeRoot) || (TreeFile=fopen(TreeName,"rb"))==NULL)
		if (!GetCacheTreeName(TreeRoot,TreeName,FALSE) || (TreeFile=fopen(TreeName,"rb"))==NULL)
		{
			ClearCache(1);
			return;
		}

	strcpy(TreeCache.TreeName,TreeName);

	while (fgets(DirName,sizeof(DirName),TreeFile)!=NULL)
	{
		if ((ChPtr=strchr(DirName,'\n'))!=NULL)
			*ChPtr=0;

		TreeCache.Add(DirName);
	}

	fclose(TreeFile);
}


void TreeList::FlushCache()
{
	FILE *TreeFile;
	int I;

	if (*TreeCache.TreeName)
	{
		DWORD FileAttributes=GetFileAttributes(TreeCache.TreeName);

		if (FileAttributes != -1)
			SetFileAttributes(TreeCache.TreeName,FILE_ATTRIBUTE_NORMAL);

		if ((TreeFile=fopen(TreeCache.TreeName,"wb"))==NULL)
		{
			ClearCache(1);
			return;
		}

		far_qsort(TreeCache.ListName,TreeCache.TreeCount,sizeof(char*),SortCacheList);

		for (I=0; I<TreeCache.TreeCount; I++)
			fprintf(TreeFile,"%s\n",TreeCache.ListName[I]);

		if (fclose(TreeFile)==EOF)
		{
			clearerr(TreeFile);
			fclose(TreeFile);
			remove(TreeCache.TreeName);
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotSaveTree),
			        TreeCache.TreeName,MSG(MOk));
		}
		else if (FileAttributes != -1) // вернем атрибуты (если получится :-)
			SetFileAttributes(TreeCache.TreeName,FileAttributes);
	}

	ClearCache(1);
}


void TreeList::UpdateViewPanel()
{
	if (!ModalMode)
	{
		Panel *AnotherPanel=GetRootPanel();
		char CurName[NM];
		GetCurDir(CurName);

		if (AnotherPanel->GetType()==QVIEW_PANEL && SetCurPath())
			((QuickView *)AnotherPanel)->ShowFile(CurName,FALSE,NULL);
	}
}

int TreeList::GoToFile(long idxItem)
{
	if ((DWORD)idxItem < (DWORD)TreeCount)
	{
		CurFile=idxItem;
		CorrectPosition();
		return TRUE;
	}

	return FALSE;
}


int TreeList::GoToFile(const char *Name,BOOL OnlyPartName)
{
	return GoToFile(FindFile(Name,OnlyPartName));
}

long TreeList::FindFile(const char *Name,BOOL OnlyPartName)
{
	long I;
	struct TreeItem *CurPtr;

	for (CurPtr=ListData, I=0; I < TreeCount; I++, CurPtr++)
	{
		char *CurPtrName=CurPtr->Name;

		if (OnlyPartName)
			CurPtrName=PointToName(CurPtr->Name);

		if (strcmp(Name,CurPtrName)==0)
			return I;

		if (LocalStricmp(Name,CurPtrName)==0)
			return I;
	}

	return -1;
}

long TreeList::FindFirst(const char *Name)
{
	return FindNext(0,Name);
}

long TreeList::FindNext(int StartPos, const char *Name)
{
	int I;
	struct TreeItem *CurPtr;

	if ((DWORD)StartPos < (DWORD)TreeCount)
		for (CurPtr=ListData+StartPos, I=StartPos; I < TreeCount; I++, CurPtr++)
		{
			if (CmpName(Name,CurPtr->Name,TRUE))
				if (!TestParentFolderName(CurPtr->Name))
					return I;
		}

	return -1;
}

int TreeList::GetFileName(char *Name,int Pos,int &FileAttr)
{
	if (Pos < 0 || Pos >= TreeCount)
		return FALSE;

	if (Name)
		strcpy(Name,ListData[Pos].Name);

	FileAttr=FA_DIREC|GetFileAttributes(ListData[Pos].Name);
	return TRUE;
}

int _cdecl SortList(const void *el1,const void *el2)
{
	char *NamePtr1=((struct TreeItem *)el1)->Name;
	char *NamePtr2=((struct TreeItem *)el2)->Name;

	if (!StaticSortNumeric)
		return(StaticSortCaseSensitive ? TreeCmp(NamePtr1,NamePtr2):LCStricmp(NamePtr1,NamePtr2));
	else
		return(StaticSortCaseSensitive ? TreeCmp(NamePtr1,NamePtr2):LCNumStricmp(NamePtr1,NamePtr2));
}

int _cdecl SortCacheList(const void *el1,const void *el2)
{
//  if(!StaticSortNumeric)
	return(LCStricmp(*(char **)el1,*(char **)el2));
//  else
//    return(LCNumStricmp(*(char **)el1,*(char **)el2));
}


int TreeCmp(char *Str1,char *Str2)
{
	while (1)
	{
		if (*Str1 != *Str2)
		{
			if (*Str1==0)
				return(-1);

			if (*Str2==0)
				return(1);

			if (*Str1=='\\')
				return(-1);

			if (*Str2=='\\')
				return(1);

			return(*Str1<*Str2 ? -1:1);
		}

		if (*(Str1++) == 0)
			break;

		Str2++;
	}

	return(0);
}

/* $ 16.10.2000 tran
 функция, определяющаяя необходимость кеширования
 файла */
int TreeList::MustBeCached(char *Root)
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
			if (toupper(Root[0])=='A' || toupper(Root[0])=='B')
			{
				return FALSE; // это дискеты
			}
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
	if (CurFile<TreeCount)
	{
		struct TreeItem *CurPtr=ListData+CurFile;

		if (GetFileAttributes(CurPtr->Name)==(DWORD)-1)
		{
			DelTreeName(CurPtr->Name);
			Update(UPDATE_KEEP_SELECTION);
		}
	}

	Panel::KillFocus();
	SetMacroMode(TRUE);
}

void TreeList::SetMacroMode(int Restore)
{
	if (CtrlObject == NULL)
		return;

	if (PrevMacroMode == -1)
		PrevMacroMode = CtrlObject->Macro.GetMode();

	CtrlObject->Macro.SetMode(Restore ? PrevMacroMode:MACRO_TREEPANEL);
}

BOOL TreeList::UpdateKeyBar()
{
	KeyBar *KB = CtrlObject->MainKeyBar;
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
	KeyBar *KB = CtrlObject->MainKeyBar;
	KB->ReadRegGroup("Tree",Opt.Language);
	KB->SetAllRegGroup();
}

void TreeList::SetTitle()
{
	if (GetFocus())
	{
		char TitleDir[NM+30];
		char *Ptr="";

		if (ListData)
		{
			struct TreeItem *CurPtr=ListData+CurFile;
			Ptr=CurPtr->Name;
		}

		if (*Ptr)
			sprintf(TitleDir,"{%.*s - Tree}",NM-1,Ptr);
		else
		{
			sprintf(TitleDir,"{Tree}");
		}

		strcpy(LastFarTitle,TitleDir);
		SetFarTitle(TitleDir);
	}
}

/*
   "Local AppData" = [HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders]/Local AppData
   "AppData"       = [HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders]/AppData

*/

// TODO: Файлы "Tree.Far" для локальных дисков должны храниться в "Local AppData\Far"
// TODO: Файлы "Tree.Far" для сетевых дисков должны храниться в "%HOMEDRIVE%\%HOMEPATH%",
//                        если эти переменные среды не определены, то "%APPDATA%\Far"
// хpаним "X.tree" (где 'X'  - буква диска, если не сетевой путь)
// хpаним "server.share.tree" - для сетевого диска без буквы
char *TreeList::MkTreeFileName(const char *RootDir,char *Dest,int DestSize)
{
	xstrncpy(Dest,RootDir,DestSize-1);
	AddEndSlash(Dest);
	xstrncat(Dest,"Tree.Far",DestSize-1);
	return Dest;
}

// TODO: этому каталогу (Tree.Cache) место не в FarPath, а в "Local AppData\Far\"
char *TreeList::MkTreeCacheFolderName(const char *RootDir,char *Dest,int DestSize)
{
	xstrncpy(Dest,RootDir,DestSize-1);
	AddEndSlash(Dest);
	xstrncat(Dest,"Tree.Cache",DestSize);
	return Dest;
}


/*
  Opt.Tree.LocalDisk
  Opt.Tree.NetDisk
  Opt.Tree.NetPath
  Opt.Tree.RemovableDisk
  Opt.Tree.CDROM
  Opt.Tree.SavedTreePath

   локальных дисков - "X.nnnnnnnn.tree"
   сетевых дисков - "X.nnnnnnnn.tree"
   сетевых путей - "Server.share.tree"
   сменных дисков(DRIVE_REMOVABLE) - "Far.nnnnnnnn.tree"
   сменных дисков(CD) - "Label.nnnnnnnn.tree"

*/
char * TreeList::CreateTreeFileName(const char *Path,char *Dest,int DestSize)
{
#if 0
	char RootPath[NM];
	GetPathRoot(Path,RootPath);
	UINT DriveType = FAR_GetDriveType(RootPath,NULL,FALSE);
	// получение инфы о томе
	char VolumeName[NM],FileSystemName[NM];
	DWORD MaxNameLength,FileSystemFlags,VolumeNumber;

	if (!GetVolumeInformation(RootDir,VolumeName,sizeof(VolumeName),&VolumeNumber,
	                          &MaxNameLength,&FileSystemFlags,
	                          FileSystemName,sizeof(FileSystemName)))
		Opt.Tree.SavedTreePath
#endif
		return Dest;
}

BOOL TreeList::GetItem(int Index,void *Dest)
{
	if (Index == -1 || Index == -2)
		Index=GetCurrentPos();

	if ((DWORD)Index >= TreeCount)
		return FALSE;

	memcpy(Dest,ListData+Index,sizeof(struct TreeItem));
	return TRUE;
}

int TreeList::GetCurrentPos()
{
	return CurFile;
}

bool TreeList::SaveState()
{
	if (SaveListData) xf_free(SaveListData);

	SaveListData=NULL;
	SaveTreeCount=SaveWorkDir=0;

	if (TreeCount > 0 && (SaveListData=(struct TreeItem *)xf_malloc(TreeCount*sizeof(struct TreeItem))) != NULL)
	{
		memmove(SaveListData,ListData,TreeCount*sizeof(struct TreeItem));
		SaveTreeCount=TreeCount;
		SaveWorkDir=WorkDir;
		TreeCache.Copy(&tempTreeCache);
		return true;
	}

	return false;
}

bool TreeList::RestoreState()
{
	if (ListData) xf_free(ListData);

	TreeCount=WorkDir=0;
	ListData=NULL;

	if (SaveTreeCount > 0 && (ListData=(struct TreeItem *)xf_malloc(SaveTreeCount*sizeof(struct TreeItem))) != NULL)
	{
		memmove(ListData,SaveListData,SaveTreeCount*sizeof(struct TreeItem));
		TreeCount=SaveTreeCount;
		WorkDir=SaveWorkDir;
		tempTreeCache.Copy(&TreeCache);
		tempTreeCache.Clean();
		return true;
	}

	return false;
}
