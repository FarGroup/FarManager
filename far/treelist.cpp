/*
treelist.cpp

Tree panel
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include "ScrBuf.hpp"
#include "TPreRedrawFunc.hpp"


#define DELTA_TREECOUNT 31

static int _cdecl SortList(const void *el1,const void *el2);
static int _cdecl SortCacheList(const void *el1,const void *el2);
static int StaticSortCaseSensitive;
static int StaticSortNumeric;
static int TreeCmp(const wchar_t *Str1, const wchar_t *Str2);
static clock_t TreeStartTime;
static int LastScrX = -1;
static int LastScrY = -1;

static struct TreeListCache
{
  string strTreeName;
  wchar_t **ListName;
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
    if(TreeCount==TreeSize)
    {
      TreeSize+=TreeSize?TreeSize>>2:32;
      wchar_t **NewPtr=(wchar_t**)xf_realloc(ListName,sizeof(wchar_t*)*TreeSize);
      if(!NewPtr)
        return;
      ListName=NewPtr;
    }
  }

  void Add(const wchar_t* name)
  {
    Resize();
    ListName[TreeCount++]=xf_wcsdup(name);
  }

  void Insert(int idx,const wchar_t* name)
  {
    Resize();
    memmove(ListName+idx+1,ListName+idx,sizeof(wchar_t*)*(TreeCount-idx));
    ListName[idx]=xf_wcsdup(name);
    TreeCount++;
  }

  void Delete(int idx)
  {
    if(ListName[idx]) xf_free(ListName[idx]);
    memmove(ListName+idx,ListName+idx+1,sizeof(wchar_t*)*(TreeCount-idx-1));
    TreeCount--;
  }

  void Clean()
  {
    if(!TreeSize)return;
    for(int i=0;i<TreeCount;i++)
    {
      if(ListName[i]) xf_free(ListName[i]);
    }
    if(ListName) xf_free(ListName);
    ListName=NULL;
    TreeCount=0;
    TreeSize=0;
    strTreeName = L"";
  }

  //TODO: необходимо оптимизировать!
  void Copy(struct TreeListCache *Dest)
  {
    Dest->Clean();
    for(int I=0; I < TreeCount; I++)
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
//  *Root=0;
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
  if(ListData)
  {
  	for (long i=0; i<TreeCount; i++)
		delete ListData[i];
    xf_free(ListData);
  }
  if(SaveListData) delete [] SaveListData;

  tempTreeCache.Clean();
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
  if(ExitCode)
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
    DisplayTree(FALSE);
  }
  Flags.Clear(FSCROBJ_ISREDRAWING);
}


string &TreeList::GetTitle(string &strTitle,int SubLen,int TruncSize)
{
  strTitle.Format (L" %s ",ModalMode ? MSG(MFindFolderTitle):MSG(MTreeTitle));
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

  int I,J,K;
  struct TreeItem *CurPtr;

  string strTitle;
  LockScreen *LckScreen=NULL;

  if(CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == QVIEW_PANEL)
    LckScreen=new LockScreen;

  CorrectPosition();
  if (TreeCount>0)
    strCurDir = ListData[CurFile]->strName; //BUGBUG
//    xstrncpy(CurDir,ListData[CurFile].Name,sizeof(CurDir)-1);
  if (!Fast)
  {
    Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
    DrawSeparator(Y2-2-(ModalMode!=0));
    GetTitle(strTitle);
    if ( !strTitle.IsEmpty() )
    {
      SetColor((Focus || ModalMode) ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
      GotoXY(X1+(X2-X1+1-(int)strTitle.GetLength())/2,Y1);
      Text(strTitle);
    }
  }
  for (I=Y1+1,J=CurTopFile;I<Y2-2-(ModalMode!=0);I++,J++)
  {
    GotoXY(X1+1,I);
    SetColor(COL_PANELTEXT);
    Text(L" ");
    if (J<TreeCount && Flags.Check(FTREELIST_TREEISPREPARED))
    {
      CurPtr=ListData[J];

      if (J==0)
      {
        DisplayTreeName(L"\\",J);
      }
      else
      {
        string strOutStr;
        for (K=0;K<CurPtr->Depth-1 && WhereX()+3*K<X2-6;K++)
        {
          strOutStr+=TreeLineSymbol[CurPtr->Last[K]?0:1];
        }
        strOutStr+=TreeLineSymbol[CurPtr->Last[CurPtr->Depth-1]?2:3];
        BoxText(strOutStr);

        const wchar_t *ChPtr=wcsrchr(CurPtr->strName,L'\\');
				if(!ChPtr)
					ChPtr=wcsrchr(CurPtr->strName,L'/');
        if (ChPtr!=NULL)
          DisplayTreeName(ChPtr+1,J);
      }
    }
    SetColor(COL_PANELTEXT);
    if ((K=WhereX())<X2)
      mprintf(L"%*s",X2-WhereX(),L"");
  }
  if (Opt.ShowPanelScrollbar)
  {
    SetColor(COL_PANELSCROLLBAR);
		ScrollBarEx(X2,Y1+1,Y2-Y1-3,CurTopFile,TreeCount);
  }
  SetColor(COL_PANELTEXT);

  SetScreen(X1+1,Y2-(ModalMode?2:1),X2-1,Y2-1,L' ',COL_PANELTEXT);
  if (TreeCount>0)
  {
    GotoXY(X1+1,Y2-1);
    mprintf(L"%-*.*s",X2-X1-1,X2-X1-1,(const wchar_t*)ListData[CurFile]->strName);
  }

  UpdateViewPanel();
  SetTitle(); // не забудим прорисовать заголовок
  if (LckScreen)
    delete LckScreen;
}


void TreeList::DisplayTreeName(const wchar_t *Name,int Pos)
{
  if (WhereX()>X2-4)
    GotoXY(X2-4,WhereY());
  if (Pos==CurFile)
  {
    GotoXY(WhereX()-1,WhereY());
    if (Focus || ModalMode)
    {
      SetColor((Pos==WorkDir) ? COL_PANELSELECTEDCURSOR:COL_PANELCURSOR);
      mprintf(L" %.*s ",X2-WhereX()-3,Name);
    }
    else
    {
      SetColor((Pos==WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
      mprintf(L"[%.*s]",X2-WhereX()-3,Name);
    }
  }
  else
  {
    SetColor((Pos==WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
    mprintf(L"%.*s",X2-WhereX()-1,Name);
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

  if(!RetFromReadTree && !Flags.Check(FTREELIST_ISPANEL))
  {
    ExitCode=0;
    return;
  }

  if (RetFromReadTree && TreeCount>0 && ((Mode & UPDATE_KEEP_SELECTION)==0 || LastTreeCount!=TreeCount))
  {
    SyncDir();

    struct TreeItem *CurPtr=ListData[CurFile];
		if (apiGetFileAttributes(CurPtr->strName)==INVALID_FILE_ATTRIBUTES)
    {
      DelTreeName(CurPtr->strName);
      Update(UPDATE_KEEP_SELECTION);
      Show();
    }
  }
  else if(!RetFromReadTree)
  {
    Show();
    if(!Flags.Check(FTREELIST_ISPANEL))
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
  FAR_FIND_DATA_EX fdata;
  string strFullName;

  SaveState();

  FlushCache();
  GetRoot();

  int RootLength=(int)strRoot.GetLength ()-1;
  if (RootLength<0)
    RootLength=0;

  if(ListData)
  {
  	for (long i=0; i<TreeCount; i++)
		delete ListData[i];
    xf_free(ListData);
  }
  TreeCount=0;
  if ((ListData=(struct TreeItem**)xf_malloc((TreeCount+256+1)*sizeof(struct TreeItem*)))==NULL)
  {
    RestoreState();
    return FALSE;
  }

  ListData[0] = new TreeItem;
  ListData[0]->Clear();

  ListData[0]->strName = strRoot;

  SaveScreen SaveScrTree;
  UndoGlobalSaveScrPtr UndSaveScr(&SaveScrTree);

  /* Т.к. мы можем вызвать диалог подтверждения (который не перерисовывает панельки,
     а восстанавливает сохраненный образ экрана, то нарисуем чистую панель */
  Redraw();

	if (RootLength>0 && strRoot.At (RootLength-1) != L':' && IsSlash(strRoot.At (RootLength)))
    ListData[0]->strName.SetLength (RootLength);

  TreeCount=1;

  int FirstCall=TRUE, AscAbort=FALSE;
  TreeStartTime = clock();

  RefreshFrameManager frref(ScrX,ScrY,TreeStartTime,FALSE);//DontRedrawFrame);

  ScTree.SetFindPath(strRoot, L"*.*", 0);
  LastScrX = ScrX;
  LastScrY = ScrY;
  while (ScTree.GetNextName(&fdata,strFullName))
  {
//    if(TreeCount > 3)
    TreeList::MsgReadTree(TreeCount,FirstCall);
    if (CheckForEscSilent())
    {
      AscAbort=ConfirmAbortOp()!=0;
      FirstCall=TRUE;
    }
    if(AscAbort)
      break;
    if ((TreeCount & 255)==0 )
    {
      TreeItem **TmpListData=(TreeItem **)xf_realloc(ListData,(TreeCount+256+1)*sizeof(TreeItem*));

      if ( !TmpListData )
      {
        AscAbort=TRUE;
        break;
      }

      ListData = TmpListData;
    }

    if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      continue;

    ListData[TreeCount] = new TreeItem;
    ListData[TreeCount]->Clear();

    ListData[TreeCount]->strName = strFullName;

    TreeCount++;
  }

  if(AscAbort && !Flags.Check(FTREELIST_ISPANEL))
  {
  	if (ListData)
  	{
  	  for (long i=0; i<TreeCount; i++)
        delete ListData[i];
      xf_free(ListData);
    }
    ListData=NULL;
    TreeCount=0;
    RestoreState();
    return FALSE;
  }

  StaticSortCaseSensitive=CaseSensitiveSort=StaticSortNumeric=NumericSort=FALSE;
  far_qsort(ListData,TreeCount,sizeof(*ListData),SortList);

	if(!FillLastData())
		return FALSE;

  if ( !AscAbort )
    SaveTreeFile();

  if (!FirstCall && !Flags.Check(FTREELIST_ISPANEL))
  { // Перерисуем другую панель - удалим следы сообщений :)
    Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
    AnotherPanel->Redraw();
  }
  return TRUE;
}

//extern FILE *_wfopen(const wchar_t *filename, const wchar_t *mode);

void TreeList::SaveTreeFile()
{
  if (TreeCount<4)
    return;

  string strName;

  FILE *TreeFile;
  long I;
  int RootLength = (int)strRoot.GetLength()-1;
  if (RootLength<0)
    RootLength=0;

  MkTreeFileName(strRoot, strName);
  // получим и сразу сбросим атрибуты (если получится)
	DWORD FileAttributes=apiGetFileAttributes(strName);
  if(FileAttributes != INVALID_FILE_ATTRIBUTES)
		apiSetFileAttributes(strName,FILE_ATTRIBUTE_NORMAL);
  if ((TreeFile=_wfopen(strName,L"wb"))==NULL)
  {
    /* $ 16.10.2000 tran
       если диск должен кешироваться, то и пытаться не стоит */
    if (MustBeCached(strRoot) || (TreeFile=_wfopen(strName,L"wb"))==NULL)
      if (!GetCacheTreeName(strRoot,strName,TRUE) || (TreeFile=_wfopen(strName,L"wb"))==NULL)
        return;
    /* tran $ */
  }
  for (I=0;I<TreeCount;I++)
    if (RootLength>=(int)ListData[I]->strName.GetLength())
      fwprintf(TreeFile,L"\\\n");
    else
      fwprintf(TreeFile,L"%s\n",(const wchar_t*)ListData[I]->strName+RootLength);
  if (fclose(TreeFile)==EOF)
  {
    clearerr(TreeFile);
    fclose(TreeFile);

		apiDeleteFile (strName);

    Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotSaveTree),strName,MSG(MOk));
  }
  else if(FileAttributes != INVALID_FILE_ATTRIBUTES) // вернем атрибуты (если получится :-)
		apiSetFileAttributes(strName,FileAttributes);
}


int TreeList::GetCacheTreeName(const wchar_t *Root, string &strName,int CreateDir)
{
  string strVolumeName, strFileSystemName;
  DWORD dwVolumeSerialNumber;

  if ( !apiGetVolumeInformation (
        Root,
        &strVolumeName,
        &dwVolumeSerialNumber,
        NULL,
        NULL,
        &strFileSystemName
        ) )
    return(FALSE);

  string strFolderName;

  string strFarPath;

  MkTreeCacheFolderName(strFarPath, strFolderName);

  if (CreateDir)
  {
		apiCreateDirectory (strFolderName, NULL);
		apiSetFileAttributes (strFolderName,Opt.Tree.TreeFileAttr);
  }

  string strRemoteName;
  wchar_t *lpwszRemoteName;
//  char RemoteName[NM*3];
//  *RemoteName=0;
  if ( *Root == L'\\')
    strRemoteName = Root;
  else
  {
    wchar_t wszLocalName [10];

    wcscpy (wszLocalName, L"A:");

    *wszLocalName=*Root;

    apiWNetGetConnection (wszLocalName, strRemoteName);

    if ( !strRemoteName.IsEmpty () )
      AddEndSlash(strRemoteName);
  }

  lpwszRemoteName = strRemoteName.GetBuffer ();

  for (int I=0; lpwszRemoteName[I] != 0;I++)
    if ( lpwszRemoteName[I]==L'\\' )
      lpwszRemoteName[I]=L'_';

  strRemoteName.ReleaseBuffer ();

  strName.Format (
        L"%s\\%s.%x.%s.%s",
        (const wchar_t*)strFolderName,
        (const wchar_t*)strVolumeName,
        dwVolumeSerialNumber,
        (const wchar_t*)strFileSystemName,
        (const wchar_t*)strRemoteName
        );

  return(TRUE);
}


void TreeList::GetRoot()
{
  string strPanelDir;
  Panel *RootPanel=GetRootPanel();
  RootPanel->GetCurDir(strPanelDir);

  GetPathRoot(strPanelDir, strRoot);
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
  string strPanelDir;
  Panel *AnotherPanel=GetRootPanel();
  AnotherPanel->GetCurDir(strPanelDir);

  if ( !strPanelDir.IsEmpty() )
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
  if(IsChangeConsole)
  {
    LastScrX = ScrX;
    LastScrY = ScrY;
  }

  if (IsChangeConsole || (clock() - TreeStartTime) > 1000)
  {
    wchar_t NumStr[32];
    _itow(TreeCount,NumStr,10); //BUGBUG
    Message((FirstCall ? 0:MSG_KEEPBACKGROUND),0,MSG(MTreeTitle),
            MSG(MReadingTree),NumStr);
    PreRedrawItem preRedrawItem=PreRedraw.Peek();
    preRedrawItem.Param.Flags=TreeCount;
    PreRedraw.SetParam(preRedrawItem.Param);
    TreeStartTime = clock();
  }

  return(1);
}


bool TreeList::FillLastData()
{
	int Last,PathLength,SubDirPos,I,J;
	size_t Pos,Depth;
	int RootLength = (int)strRoot.GetLength()-1;
	if (RootLength<0)
		RootLength=0;

	for (I=1;I<TreeCount;I++)
	{
		if (ListData[I]->strName.RPos(Pos,L'\\'))
			PathLength=(int)Pos+1;
		else
			PathLength=0;

		Depth=ListData[I]->Depth=CountSlash((const wchar_t*)ListData[I]->strName+RootLength);
		if(!Depth)
			return false;

		for (J=I+1,SubDirPos=I,Last=1;J<TreeCount;J++)
		{
			if (CountSlash((const wchar_t*)ListData[J]->strName+RootLength)>Depth)
			{
				SubDirPos=J;
				continue;
			}
			else
			{
				if ( StrCmpNI(ListData[I]->strName,ListData[J]->strName,PathLength)==0 )
					Last=0;
				break;
			}
		}
		for (J=I;J<=SubDirPos;J++)
		{
			if(Depth>ListData[J]->LastCount)
			{
				ListData[J]->LastCount<<=1;
				ListData[J]->Last=static_cast<int*>(xf_realloc(ListData[J]->Last,ListData[J]->LastCount*sizeof(int)));
			}
			ListData[J]->Last[Depth-1]=Last;
		}
	}
	return true;
}


UINT TreeList::CountSlash(const wchar_t *Str)
{
	UINT Count=0;
	for(;*Str;Str++)
		if(IsSlash(*Str))
			Count++;
  return(Count);
}


__int64 TreeList::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
  switch(OpCode)
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

  string strTemp = L"";
  if (SaveFolderShortcut(Key,&strCurDir,&strTemp,&strTemp,&strTemp))
    return(TRUE);
  if(ProcessShortcutFolder(Key,TRUE))
    return(TRUE);

  switch(Key)
  {
    case KEY_F1:
    {
      {
         Help Hlp (L"TreePanel");
      }
      return TRUE;
    }

    case KEY_SHIFTNUMENTER:
    case KEY_CTRLNUMENTER:
    case KEY_SHIFTENTER:
    case KEY_CTRLENTER:
    case KEY_CTRLF:
    case KEY_CTRLALTINS:  case KEY_CTRLALTNUMPAD0:
    {
      {
        string strQuotedName;
        int CAIns=(Key == KEY_CTRLALTINS || Key == KEY_CTRLALTNUMPAD0);

        CurPtr=ListData[CurFile];
        if (wcschr(CurPtr->strName,L' ')!=NULL)
          strQuotedName.Format (L"\"%s\"%s",(const wchar_t *)CurPtr->strName,(CAIns?L"":L" "));
        else
          strQuotedName.Format (L"%s%s",(const wchar_t *)CurPtr->strName,(CAIns?L"":L" "));

        if(CAIns)
          CopyToClipboard(strQuotedName);

        else if(Key == KEY_SHIFTENTER||Key == KEY_SHIFTNUMENTER)
          Execute(strQuotedName,FALSE,TRUE,TRUE);
        else
          CtrlObject->CmdLine->InsertString(strQuotedName);
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
        int Ask=((Key!=KEY_DRAGCOPY && Key!=KEY_DRAGMOVE) || Opt.Confirm.Drag);
        int Move=(Key==KEY_F6 || Key==KEY_DRAGMOVE);
        int ToPlugin=AnotherPanel->GetMode()==PLUGIN_PANEL &&
                     AnotherPanel->IsVisible() &&
                     !CtrlObject->Plugins.UseFarCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES);
        int Link=(Key==KEY_ALTF6 && !ToPlugin);
        if(Key==KEY_ALTF6 && !Link) // молча отвалим :-)
          return TRUE;

        {
          ShellCopy ShCopy(this,Move,Link,FALSE,Ask,ToPlugin,NULL);
        }

        if (ToPlugin==1)
        {
          struct PluginPanelItem *ItemList=new PluginPanelItem[1];
          int ItemNumber=1;
          HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();

          FileList::FileNameToPluginItem(ListData[CurFile]->strName,ItemList);
          int PutCode=CtrlObject->Plugins.PutFiles(hAnotherPlugin,ItemList,ItemNumber,Move,0);
          if (PutCode==1 || PutCode==2)
            AnotherPanel->SetPluginModified();
          if(ItemList) xf_free(ItemList);
          if (Move)
            ReadSubTree(ListData[CurFile]->strName);
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
    case KEY_SHIFTDEL:
    case KEY_SHIFTNUMDEL:
    case KEY_SHIFTDECIMAL:
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
    case (KEY_MSWHEEL_UP | KEY_ALT):
    {
      Scroll(Key & KEY_ALT?-1:-Opt.MsWheelDelta);
      return(TRUE);
    }

    case KEY_MSWHEEL_DOWN:
    case (KEY_MSWHEEL_DOWN | KEY_ALT):
    {
      Scroll(Key & KEY_ALT?1:Opt.MsWheelDelta);
      return(TRUE);
    }

    case KEY_MSWHEEL_LEFT:
    case (KEY_MSWHEEL_LEFT | KEY_ALT):
    {
      int Roll = Key & KEY_ALT?1:Opt.MsHWheelDelta;
      for (int i=0; i<Roll; i++)
        ProcessKey(KEY_LEFT);
      return TRUE;
    }

    case KEY_MSWHEEL_RIGHT:
    case (KEY_MSWHEEL_RIGHT | KEY_ALT):
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
      if ((Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+255) ||
          (Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+255))
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
  if(CurFile+1 < TreeCount)
  {
    int CurDepth=ListData[CurFile]->Depth;
    for(int I=CurFile+1; I < TreeCount; ++I)
      if(ListData[I]->Depth == CurDepth)
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
  if(CurFile-1 > 0)
  {
    int CurDepth=ListData[CurFile]->Depth;
    for(int I=CurFile-1; I > 0; --I)
      if(ListData[I]->Depth == CurDepth)
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

BOOL TreeList::SetCurDir(const wchar_t *NewDir,int ClosePlugin)
{
  if (TreeCount==0)
    Update(0);
  if (TreeCount>0 && !SetDirPosition(NewDir))
  {
    Update(0);
    SetDirPosition(NewDir);
  }
  if (GetFocus())
  {
    CtrlObject->CmdLine->SetCurDir(NewDir);
    CtrlObject->CmdLine->Show();
  }
  return TRUE; //???
}

int TreeList::SetDirPosition(const wchar_t *NewDir)
{
  long I;
  for (I=0;I<TreeCount;I++)
  {
    if (StrCmpI(NewDir,ListData[I]->strName)==0)
    {
      WorkDir=CurFile=I;
      CurTopFile=CurFile-(Y2-Y1-1)/2;
      CorrectPosition();
      return(TRUE);
    }
  }
  return(FALSE);
}


int TreeList::GetCurDir(string &strCurDir)
{
  if (TreeCount==0)
  {
    if (ModalMode==MODALTREE_FREE)
      strCurDir = strRoot;
    else
      strCurDir = L"";
  }
  else
    strCurDir = ListData[CurFile]->strName; //BUGBUG

  return (int)strCurDir.GetLength();
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

    if (((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
        MouseEvent->dwEventFlags==DOUBLE_CLICK) ||
        ((MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) &&
        MouseEvent->dwEventFlags==0) ||
        (OldFile!=CurFile && Opt.Tree.AutoChangeFolder && !ModalMode))
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
  FILE *TreeFile=NULL;
  int RootLength=(int)strRoot.GetLength()-1;
  if (RootLength<0)
    RootLength=0;

  string strName;

  //SaveState();
  FlushCache();
  MkTreeFileName(strRoot,strName);

  if (MustBeCached(strRoot) || (TreeFile=_wfopen(strName,L"rb"))==NULL)
    if (!GetCacheTreeName(strRoot,strName,FALSE) || (TreeFile=_wfopen(strName,L"rb"))==NULL)
    {
      //RestoreState();
      return(FALSE);
    }
  if(ListData)
  {
  	for (long i=0; i<TreeCount; i++)
		delete ListData[i];
    xf_free(ListData);
  }
  ListData=NULL;
  TreeCount=0;

	wchar_t *DirName=new wchar_t[NT_MAX_PATH];
	if(DirName)
	{
		xwcsncpy (DirName, strRoot, NT_MAX_PATH-1);

		string strLastDirName;

		while (fgetws(DirName+RootLength,NT_MAX_PATH-RootLength,TreeFile)!=NULL)
		{
			if (!IsSlash(*(DirName+RootLength)) || StrCmpI (DirName,strLastDirName)==0)
				continue;

			strLastDirName=DirName;

			wchar_t *ChPtr=wcschr(DirName,L'\n');
			if(ChPtr)
				*ChPtr=0;
			if (RootLength>0 && DirName[RootLength-1]!=L':' && IsSlash(DirName[RootLength]) && DirName[RootLength+1]==0)
				DirName[RootLength]=0;

			if ((TreeCount & 255)==0 )
			{
				TreeItem **TmpListData=(TreeItem **)xf_realloc(ListData,(TreeCount+256+1)*sizeof(TreeItem*));

				if ( !TmpListData )
				{
					if(ListData)
					{
						for (long i=0; i<TreeCount; i++)
							delete ListData[i];
						xf_free(ListData);
					}
					ListData=NULL;
					TreeCount=0;
					delete[] DirName;
					fclose(TreeFile);
					//RestoreState();
					return(FALSE);
				}

				ListData = TmpListData;
			}

			ListData[TreeCount] = new TreeItem;
			ListData[TreeCount]->Clear();
			ListData[TreeCount]->strName = DirName;

			TreeCount++;
		}
		delete[] DirName;
	}
  fclose(TreeFile);

  if (TreeCount==0)
    return(FALSE);
  CaseSensitiveSort=FALSE;
  NumericSort=FALSE;

  return FillLastData();
}


int TreeList::FindPartName(const wchar_t *Name,int Next,int Direct,int ExcludeSets)
{
  string strMask;

  strMask = Name;
  strMask += L"*";

  if(ExcludeSets)
  {
    ReplaceStrings(strMask,L"[",L"<[%>",-1,1);
    ReplaceStrings(strMask,L"]",L"[]]",-1,1);
    ReplaceStrings(strMask,L"<[%>",L"[[]",-1,1);
  }

  int I;
  for (I=CurFile+(Next?Direct:0); I >= 0 && I < TreeCount; I+=Direct)
  {
    CmpNameSearchMode=(I==CurFile);

    if (CmpName(strMask,ListData[I]->strName,TRUE))
    {
      CmpNameSearchMode=FALSE;
      CurFile=I;
      CurTopFile=CurFile-(Y2-Y1-1)/2;
      DisplayTree(TRUE);
      return(TRUE);
    }
  }

  CmpNameSearchMode=FALSE;

  for(
      I=(Direct > 0)?0:TreeCount-1;
      (Direct > 0) ? I < CurFile:I > CurFile;
      I+=Direct
     )
  {
    if (CmpName(strMask,ListData[I]->strName,TRUE))
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


int TreeList::GetSelName(string *strName,DWORD &FileAttr,string *strShortName,FAR_FIND_DATA_EX *fd)
{
  if ( strName==NULL)
  {
    GetSelPosition=0;
    return(TRUE);
  }

  if (GetSelPosition==0)
  {
    GetCurDir(*strName);
    if ( strShortName != NULL)
      *strShortName = *strName;
    FileAttr=FILE_ATTRIBUTE_DIRECTORY;
    GetSelPosition++;
    return(TRUE);
  }
  GetSelPosition=0;
  return(FALSE);
}


int TreeList::GetCurName(string &strName, string &strShortName)
{
  if (TreeCount==0)
  {
    strName = L"";
    strShortName = L"";
    return(FALSE);
  }

  strName = ListData[CurFile]->strName;
  strShortName = strName;
  return(TRUE);
}


void TreeList::AddTreeName(const wchar_t *Name)
{
  string strRoot;

  long CachePos;

  if (*Name==0)
    return;

  string strFullName;

  ConvertNameToFull(Name, strFullName);

  Name = strFullName;

  GetPathRoot(Name, strRoot);

  Name += strRoot.GetLength ()-1;

	if (!wcsrchr(Name,L'\\') && !wcsrchr(Name,L'/'))
    return;

  ReadCache(strRoot);
  for(CachePos=0;CachePos<TreeCache.TreeCount;CachePos++)
  {
    int Result=StrCmpI(TreeCache.ListName[CachePos],Name);
    if(!Result)
      break;
    if(Result > 0)
    {
      TreeCache.Insert(CachePos,Name);
      break;
    }
  }
}


void TreeList::DelTreeName(const wchar_t *Name)
{
  string strFullName;
  string strRoot;

  const wchar_t *wszDirName;

  long CachePos;
  int Length,DirLength;
  if (*Name==0)
    return;
  ConvertNameToFull(Name,strFullName);
  Name=strFullName;

  GetPathRoot(Name, strRoot);

  Name += strRoot.GetLength()-1;
  ReadCache(strRoot);
  for (CachePos=0;CachePos<TreeCache.TreeCount;CachePos++)
  {
    wszDirName=TreeCache.ListName[CachePos];
    Length=StrLength(Name);
    DirLength=StrLength(wszDirName);
    if(DirLength<Length) continue;
		if (StrCmpNI(Name,wszDirName,Length)==0 && (wszDirName[Length]==0 || IsSlash(wszDirName[Length])))
    {
      TreeCache.Delete(CachePos);
      CachePos--;
    }
  }
}


void TreeList::RenTreeName(const wchar_t *SrcName,const wchar_t *DestName)
{
  if (*SrcName==0 || *DestName==0)
    return;

  string strSrcRoot, strDestRoot;
  GetPathRoot(SrcName,strSrcRoot);
  GetPathRoot(DestName,strDestRoot);

  if ( StrCmpI (strSrcRoot,strDestRoot)!=0)
  {
    DelTreeName(SrcName);
    ReadSubTree(SrcName);
  }

  SrcName+=strSrcRoot.GetLength()-1;
  DestName+=strDestRoot.GetLength()-1;

  ReadCache(strSrcRoot);

  int SrcLength=StrLength(SrcName);

  for (int CachePos=0;CachePos<TreeCache.TreeCount;CachePos++)
  {
    const wchar_t *DirName=TreeCache.ListName[CachePos];
		if (StrCmpNI(SrcName,DirName,SrcLength)==0 && (DirName[SrcLength]==0 || IsSlash(DirName[SrcLength])))
    {
      string strNewName = DestName;

      strNewName += (const wchar_t*)(DirName+SrcLength);

      if(TreeCache.ListName[CachePos]) xf_free(TreeCache.ListName[CachePos]);

      TreeCache.ListName[CachePos]=xf_wcsdup(strNewName);
    }
  }
}


void TreeList::ReadSubTree(const wchar_t *Path)
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

	if ((FileAttr=apiGetFileAttributes(Path))==INVALID_FILE_ATTRIBUTES || (FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
    return;

  ConvertNameToFull(Path, strDirName);

  AddTreeName(strDirName);

  int FirstCall=TRUE, AscAbort=FALSE;

  ScTree.SetFindPath(strDirName,L"*.*",0);


  LastScrX = ScrX;
  LastScrY = ScrY;

  while (ScTree.GetNextName(&fdata, strFullName))
  {
    if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      TreeList::MsgReadTree(Count+1,FirstCall);
      if (CheckForEscSilent())
      {
        AscAbort=ConfirmAbortOp()!=0;
        FirstCall=TRUE;
      }
      if(AscAbort)
        break;
      AddTreeName(strFullName);
      ++Count;
    }
  }
}


void TreeList::ClearCache(int EnableFreeMem)
{
  TreeCache.Clean();
}


void TreeList::ReadCache(const wchar_t *TreeRoot)
{
  string strTreeName;

  FILE *TreeFile=NULL;

  if (StrCmp(MkTreeFileName(TreeRoot,strTreeName),TreeCache.strTreeName)==0)
    return;

  if (TreeCache.TreeCount!=0)
    FlushCache();

  if (MustBeCached(TreeRoot) || (TreeFile=_wfopen(strTreeName,L"rb"))==NULL)
    if (!GetCacheTreeName(TreeRoot,strTreeName,FALSE) || (TreeFile=_wfopen(strTreeName,L"rb"))==NULL)
    {
      ClearCache(1);
      return;
    }

  TreeCache.strTreeName = strTreeName;

	wchar_t *DirName=new wchar_t[NT_MAX_PATH];
	if(DirName)
	{
		while (fgetws(DirName,NT_MAX_PATH,TreeFile)!=NULL)
		{
			if(!IsSlash(*DirName))
				continue;
			wchar_t *ChPtr=wcschr(DirName,L'\n');
			if(ChPtr)
				*ChPtr=0;
			TreeCache.Add(DirName);
		}
		delete[] DirName;
	}
  fclose(TreeFile);
}


void TreeList::FlushCache()
{
  FILE *TreeFile;
  int I;
  if ( !TreeCache.strTreeName.IsEmpty() )
  {
		DWORD FileAttributes=apiGetFileAttributes(TreeCache.strTreeName);
    if(FileAttributes != INVALID_FILE_ATTRIBUTES)
			apiSetFileAttributes(TreeCache.strTreeName,FILE_ATTRIBUTE_NORMAL);
    if ((TreeFile=_wfopen(TreeCache.strTreeName,L"wb"))==NULL)
    {
      ClearCache(1);
      return;
    }
    far_qsort(TreeCache.ListName,TreeCache.TreeCount,sizeof(wchar_t*),SortCacheList);
    for (I=0;I<TreeCache.TreeCount;I++)
      fwprintf(TreeFile,L"%s\n",TreeCache.ListName[I]);
    if (fclose(TreeFile)==EOF)
    {
      clearerr(TreeFile);
      fclose(TreeFile);

			apiDeleteFile (TreeCache.strTreeName);

      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotSaveTree),
              TreeCache.strTreeName,MSG(MOk));
    }
    else if(FileAttributes != INVALID_FILE_ATTRIBUTES) // вернем атрибуты (если получится :-)
			apiSetFileAttributes(TreeCache.strTreeName,FileAttributes);
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
      ((QuickView *)AnotherPanel)->ShowFile(strCurName,FALSE,NULL);
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

int TreeList::GoToFile(const wchar_t *Name,BOOL OnlyPartName)
{
  return GoToFile(FindFile(Name,OnlyPartName));
}

long TreeList::FindFile(const wchar_t *Name,BOOL OnlyPartName)
{
  long I;
  struct TreeItem *CurPtr;

  for (I=0; I < TreeCount; I++)
  {
  	CurPtr=ListData[I];
    const wchar_t *CurPtrName=CurPtr->strName;
    if(OnlyPartName)
      CurPtrName=PointToName(CurPtr->strName);

    if (StrCmp(Name,CurPtrName)==0)
      return I;

    if (StrCmpI(Name,CurPtrName)==0)
      return I;
  }
  return -1;
}


long TreeList::FindFirst(const wchar_t *Name)
{
  return FindNext(0,Name);
}

long TreeList::FindNext(int StartPos, const wchar_t *Name)
{
  int I;
  struct TreeItem *CurPtr;

  if((DWORD)StartPos < (DWORD)TreeCount)
    for( I=StartPos; I < TreeCount; I++)
    {
      CurPtr=ListData[I];
      const wchar_t *CurPtrName=CurPtr->strName;
      if (CmpName(Name,CurPtrName,TRUE))
        if (!TestParentFolderName(CurPtrName))
          return I;
    }
  return -1;
}

int TreeList::GetFileName(string &strName,int Pos,DWORD &FileAttr)
{
  if (Pos < 0 || Pos >= TreeCount)
    return FALSE;

  strName = ListData[Pos]->strName;

	FileAttr=FILE_ATTRIBUTE_DIRECTORY|apiGetFileAttributes(ListData[Pos]->strName);

  return TRUE;
}


int _cdecl SortList(const void *el1,const void *el2)
{
  string strName1=((struct TreeItem **)el1)[0]->strName;
  string strName2=((struct TreeItem **)el2)[0]->strName;
  if(!StaticSortNumeric)
    return(StaticSortCaseSensitive ? TreeCmp(strName1,strName2):StrCmpI(strName1,strName2));
  else
    return(StaticSortCaseSensitive ? TreeCmp(strName1,strName2):NumStrCmpI(strName1,strName2));
}

int _cdecl SortCacheList(const void *el1,const void *el2)
{
  if(!StaticSortNumeric)
    return(StrCmpI(*(wchar_t **)el1,*(wchar_t **)el2));
  else
    return(NumStrCmpI(*(wchar_t **)el1,*(wchar_t **)el2));
}


int TreeCmp(const wchar_t *Str1,const wchar_t *Str2)
{
  while (1)
  {
    if (*Str1 != *Str2)
    {
      if (*Str1==0)
        return(-1);
      if (*Str2==0)
        return(1);
      if (*Str1==L'\\')
        return(-1);
      if (*Str2==L'\\')
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
int TreeList::MustBeCached(const wchar_t *Root)
{
    UINT type;

    type=FAR_GetDriveType(Root);
    if ( type==DRIVE_UNKNOWN ||
         type==DRIVE_NO_ROOT_DIR ||
         type==DRIVE_REMOVABLE ||
         IsDriveTypeCDROM(type)
         )
    {
        if ( type==DRIVE_REMOVABLE )
        {
            if ( Upper(Root[0])==L'A' || Upper(Root[0])==L'B')
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
  if (CurFile<TreeCount)
  {
    struct TreeItem *CurPtr=ListData[CurFile];
		if (apiGetFileAttributes(CurPtr->strName)==INVALID_FILE_ATTRIBUTES)
    {
      DelTreeName(CurPtr->strName);
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
  KB->SetAllGroup (KBL_MAIN, MKBTreeF1, 12);
  KB->SetAllGroup (KBL_SHIFT, MKBTreeShiftF1, 12);
  KB->SetAllGroup (KBL_ALT, MKBTreeAltF1, 12);
  KB->SetAllGroup (KBL_CTRL, MKBTreeCtrlF1, 12);
  KB->SetAllGroup (KBL_CTRLSHIFT, MKBTreeCtrlShiftF1, 12);
  KB->SetAllGroup (KBL_CTRLALT, MKBTreeCtrlAltF1, 12);
  KB->SetAllGroup (KBL_ALTSHIFT, MKBTreeAltShiftF1, 12);
  KB->SetAllGroup (KBL_CTRLALTSHIFT, MKBTreeCtrlAltShiftF1, 12);

  DynamicUpdateKeyBar();

  return TRUE;
}

void TreeList::DynamicUpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;

  KB->ReadRegGroup(L"Tree",Opt.strLanguage);
  KB->SetAllRegGroup();
}

void TreeList::SetTitle()
{
  if (GetFocus())
  {
    string strTitleDir; //BUGBUG
    const wchar_t *Ptr=L"";
    if(ListData)
    {
      struct TreeItem *CurPtr=ListData[CurFile];
      Ptr=CurPtr->strName;
    }
    if (*Ptr)
      strTitleDir.Format (L"{%s - Tree}",Ptr);
    else
      strTitleDir = L"{Tree}";

    strLastFarTitle = strTitleDir;
    SetFarTitle(strTitleDir);
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
string &TreeList::MkTreeFileName(const wchar_t *RootDir,string &strDest)
{
    strDest = RootDir;
    AddEndSlash(strDest);

    strDest += L"tree2.far";

    return strDest;
}

// TODO: этому каталогу (Tree.Cache) место не в FarPath, а в "Local AppData\Far\"
string &TreeList::MkTreeCacheFolderName(const wchar_t *RootDir,string &strDest)
{
    strDest = RootDir;
    AddEndSlash(strDest);

    strDest += L"tree2.cache";

    return strDest;
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
string &TreeList::CreateTreeFileName(const wchar_t *Path,string &strDest)
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
  return strDest;
}

BOOL TreeList::GetItem(int Index,void *Dest)
{
  if(Index == -1 || Index == -2)
    Index=GetCurrentPos();
  if(Index >= (int)TreeCount)
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
  if(SaveListData) delete [] SaveListData;
  SaveListData=NULL;
  SaveTreeCount=SaveWorkDir=0;
  if(TreeCount > 0)
  {
    SaveListData= new TreeItem[TreeCount];
    if (SaveListData)
    {
      for (int i=0; i<TreeCount; i++)
        SaveListData[i] = *ListData[i];
      SaveTreeCount=TreeCount;
      SaveWorkDir=WorkDir;
      TreeCache.Copy(&tempTreeCache);
      return true;
    }
  }
  return false;
}

bool TreeList::RestoreState()
{
  if(ListData)
  {
  	for (long i=0; i<TreeCount; i++)
		delete ListData[i];
    xf_free(ListData);
  }
  TreeCount=WorkDir=0;
  ListData=NULL;
  if(SaveTreeCount > 0 && (ListData=(struct TreeItem **)xf_realloc(ListData,SaveTreeCount*sizeof(struct TreeItem*))) != NULL)
  {
    for (int i=0; i<SaveTreeCount; i++)
    {
      ListData[i] = new TreeItem;
      *ListData[i] = SaveListData[i];
    }
    TreeCount=SaveTreeCount;
    WorkDir=SaveWorkDir;
    tempTreeCache.Copy(&TreeCache);
    tempTreeCache.Clean();
    return true;
  }
  return false;
}
