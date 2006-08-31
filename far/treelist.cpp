/*
treelist.cpp

Tree panel

*/

/* Revision: 1.94 01.09.2006 $ */

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
#include "ScrBuf.hpp"


#define DELTA_TREECOUNT 31

static int _cdecl SortList(const void *el1,const void *el2);
static int _cdecl SortCacheList(const void *el1,const void *el2);
static int StaticSortCaseSensitive;
static int StaticSortNumeric;
static int TreeCmp(const wchar_t *Str1, const wchar_t *Str2);
static clock_t TreeStartTime;
static int LastScrX = -1;
static int LastScrY = -1;

static char TreeLineSymbol[4][3]={
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
    ListName[TreeCount++]=_wcsdup(name);
  }
  void Insert(int idx,const wchar_t* name)
  {
    Resize();
    memmove(ListName+idx+1,ListName+idx,sizeof(wchar_t*)*(TreeCount-idx));
    ListName[idx]=_wcsdup(name);
    TreeCount++;
  }
  void Delete(int idx)
  {
    xf_free(ListName[idx]);
    memmove(ListName+idx,ListName+idx+1,sizeof(wchar_t*)*(TreeCount-idx-1));
    TreeCount--;
  }

  void Clean()
  {
    if(!TreeSize)return;
    for(int i=0;i<TreeCount;i++)
    {
      xf_free(ListName[i]);
    }
    xf_free(ListName);
    ListName=NULL;
    TreeCount=0;
    TreeSize=0;
    strTreeName = L"";
  }
} TreeCache;


TreeList::TreeList(int IsPanel)
{
  Type=TREE_PANEL;
  ListData=NULL;
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
  xf_free(ListData);
  FlushCache();
  SetMacroMode(TRUE);
}

void TreeList::SetRootDirW(const wchar_t *NewRootDir)
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


void TreeList::GetTitle(string &strTitle,int SubLen,int TruncSize)
{
  strTitle.Format (L" %s ",ModalMode ? UMSG(MFindFolderTitle):UMSG(MTreeTitle));
  TruncStrW(strTitle,X2-X1-3);
}

void TreeList::DisplayTree(int Fast)
{
  int I,J,K;
  struct TreeItem *CurPtr;

  string strTitle;
  LockScreen *LckScreen=NULL;

  if(CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == QVIEW_PANEL)
    LckScreen=new LockScreen;

  CorrectPosition();
  if (TreeCount>0)
    strCurDir = ListData[CurFile].strName; //BUGBUG
//    xstrncpy(CurDir,ListData[CurFile].Name,sizeof(CurDir)-1);
  if (!Fast)
  {
    Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
    DrawSeparator(Y2-2-(ModalMode!=0));
    GetTitle(strTitle);
    if ( !strTitle.IsEmpty() )
    {
      SetColor((Focus || ModalMode) ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
      GotoXY(X1+(X2-X1+1-strTitle.GetLength())/2,Y1);
      TextW(strTitle);
    }
  }
  for (I=Y1+1,J=CurTopFile;I<Y2-2-(ModalMode!=0);I++,J++)
  {
    CurPtr=&ListData[J];
    GotoXY(X1+1,I);
    SetColor(COL_PANELTEXT);
    TextW(L" ");
    if (J<TreeCount && Flags.Check(FTREELIST_TREEISPREPARED))
    {
      if (J==0)
        DisplayTreeName(L"\\",J);
      else
      {
#if defined(USE_WFUNC)
        // �������������� �������������
        if(TreeLineSymbolW[0][0] == 0x0000)
        {
          for(int IW=0; IW < 4; ++IW)
          {
            for(int JW=0; JW < 2; ++JW)
              TreeLineSymbolW[IW][JW]=(TreeLineSymbol[IW][JW] == 0x20)?0x20:BoxSymbols[TreeLineSymbol[IW][JW]-0x0B0];
          }
        }
        WCHAR OutStrW[200];
        *OutStrW=0;
#endif
        char  OutStr[200];
        for (*OutStr=0,K=0;K<CurPtr->Depth-1 && WhereX()+3*K<X2-6;K++)
        {
          if (CurPtr->Last[K])
          {
#if defined(USE_WFUNC)
            if(Opt.UseUnicodeConsole)
              wcscat(OutStrW,TreeLineSymbolW[0]);
            else
#endif
              strcat(OutStr,TreeLineSymbol[0]);
          }
          else
          {
#if defined(USE_WFUNC)
            if(Opt.UseUnicodeConsole)
              wcscat(OutStrW,TreeLineSymbolW[1]);
            else
#endif
            strcat(OutStr,TreeLineSymbol[1]);
          }
        }
        if (CurPtr->Last[CurPtr->Depth-1])
        {
#if defined(USE_WFUNC)
          if(Opt.UseUnicodeConsole)
            wcscat(OutStrW,TreeLineSymbolW[2]);
          else
#endif
            strcat(OutStr,TreeLineSymbol[2]);
        }
        else
        {
#if defined(USE_WFUNC)
          if(Opt.UseUnicodeConsole)
            wcscat(OutStrW,TreeLineSymbolW[3]);
          else
#endif
            strcat(OutStr,TreeLineSymbol[3]);
        }
#if defined(USE_WFUNC)
        if(Opt.UseUnicodeConsole)
          BoxTextW(OutStrW,FALSE);
        else
#endif
          Text(OutStr);

        const wchar_t *ChPtr=wcsrchr(CurPtr->strName,L'\\');
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

  SetScreen(X1+1,Y2-(ModalMode?2:1),X2-1,Y2-1,L' ',COL_PANELTEXT);
  if (TreeCount>0)
  {
    GotoXY(X1+1,Y2-1);
    mprintfW(L"%-*.*s",X2-X1-1,X2-X1-1,(const wchar_t*)ListData[CurFile].strName);
  }

  UpdateViewPanel();
  SetTitle(); // �� ������� ����������� ���������
  if(LckScreen)
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
      mprintfW(L" %.*s ",X2-WhereX()-3,Name);
    }
    else
    {
      SetColor((Pos==WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
      mprintfW(L"[%.*s]",X2-WhereX()-3,Name);
    }
  }
  else
  {
    SetColor((Pos==WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
    mprintfW(L"%.*s",X2-WhereX()-1,Name);
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

    struct TreeItem *CurPtr=ListData+CurFile;
    if (GetFileAttributesW(CurPtr->strName)==(DWORD)-1)
    {
      DelTreeName(CurPtr->strName);
      Update(UPDATE_KEEP_SELECTION);
      Show();
    }
  }
  else if(!RetFromReadTree)
  {
    Show();
    Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    AnotherPanel->Redraw();
  }
}


int TreeList::ReadTree()
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  //SaveScreen SaveScr;
  ScanTree ScTree(FALSE);
  FAR_FIND_DATA_EX fdata;
  string strFullName;

  FlushCache();
  GetRoot();

  int RootLength=strRoot.GetLength ()-1;
  if (RootLength<0)
    RootLength=0;

  /* $ 13.07.2000 SVS
     �� ���� ��������� new/delete � realloc
  */
  xf_free(ListData);
  TreeCount=0;
  if ((ListData=(struct TreeItem*)xf_malloc((TreeCount+256+1)*sizeof(struct TreeItem)))==NULL)
//  if ((ListData=(struct TreeItem*)xf_malloc(sizeof(struct TreeItem)))==NULL)
    return FALSE;
  /* SVS $ */

  memset(&ListData[0], 0, sizeof(ListData[0]));

  ListData->strName = strRoot;

  SaveScreen SaveScrTree;
  UndoGlobalSaveScrPtr UndSaveScr(&SaveScrTree);

  /* �.�. �� ����� ������� ������ ������������� (������� �� �������������� ��������,
     � ��������������� ����������� ����� ������, �� �������� ������ ������ */
  Redraw();

  if (RootLength>0 && strRoot.At (RootLength-1) != L':' && strRoot.At (RootLength)==L'\\')
  {
    ListData->strName.GetBuffer (); //BUGBUG
    ListData->strName.ReleaseBuffer (RootLength);
  }

  TreeCount=1;

  int FirstCall=TRUE, AscAbort=FALSE;
  TreeStartTime = clock();
  SetPreRedrawFunc(TreeList::PR_MsgReadTree);

  RefreshFrameManager frref(ScrX,ScrY,TreeStartTime,FALSE);//DontRedrawFrame);

  ScTree.SetFindPathW(strRoot, L"*.*", 0);
  LastScrX = ScrX;
  LastScrY = ScrY;
  while (ScTree.GetNextNameW(&fdata,strFullName))
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
      int OldCount = TreeCount;

      ListData=(struct TreeItem *)xf_realloc(ListData,(TreeCount+256+1)*sizeof(struct TreeItem));

      if ( ListData )
        memset (ListData+OldCount, 0, (TreeCount+256+1-OldCount)*sizeof (TreeItem));
      else
      {
        AscAbort=TRUE;
        break;
      }
    }

    if (!(fdata.dwFileAttributes & FA_DIREC))
      continue;

    memset(&ListData[TreeCount], 0, sizeof(ListData[0]));

    ListData[TreeCount].strName = strFullName;

    TreeCount++;
  }

  if(AscAbort)
  {
    xf_free(ListData);
    ListData=NULL;
    TreeCount=0;
    SetPreRedrawFunc(NULL);
    return FALSE;
  }

  SetPreRedrawFunc(NULL);
  StaticSortCaseSensitive=CaseSensitiveSort=StaticSortNumeric=NumericSort=FALSE;
  far_qsort(ListData,TreeCount,sizeof(*ListData),SortList);

  FillLastData();
  SaveTreeFile();
  if (!FirstCall)
  { // ���������� ������ ������ - ������ ����� ��������� :)
    Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
    AnotherPanel->Redraw();
  }
  return TRUE;
}

#ifdef _MSC_VER
#pragma warning(disable:4018)
#endif

extern FILE *_wfopen(const wchar_t *filename, const wchar_t *mode);

void TreeList::SaveTreeFile()
{
  if (TreeCount<4)
    return;

  string strName;

  FILE *TreeFile;
  long I;
  int RootLength = strRoot.GetLength()-1;
  if (RootLength<0)
    RootLength=0;

  MkTreeFileName(strRoot, strName);
  // ������� � ����� ������� �������� (���� ���������)
  DWORD FileAttributes=GetFileAttributesW(strName);
  if(FileAttributes != -1)
    SetFileAttributesW(strName,FILE_ATTRIBUTE_NORMAL);
  if ((TreeFile=_wfopen(strName,L"wb"))==NULL)
  {
    /* $ 16.10.2000 tran
       ���� ���� ������ ������������, �� � �������� �� ����� */
    if (MustBeCached(strRoot) || (TreeFile=_wfopen(strName,L"wb"))==NULL)
      if (!GetCacheTreeName(strRoot,strName,TRUE) || (TreeFile=_wfopen(strName,L"wb"))==NULL)
        return;
    /* tran $ */
  }
  for (I=0;I<TreeCount;I++)
    if (RootLength>=ListData[I].strName.GetLength())
      fwprintf(TreeFile,L"\\\n");
    else
      fwprintf(TreeFile,L"%s\n",(const wchar_t*)ListData[I].strName+RootLength);
  if (fclose(TreeFile)==EOF)
  {
    clearerr(TreeFile);
    fclose(TreeFile);

    //wremove(strName); BUGBUG
    DeleteFileW (strName);

    MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MCannotSaveTree),strName,UMSG(MOk));
  }
  else if(FileAttributes != -1) // ������ �������� (���� ��������� :-)
    SetFileAttributesW(strName,FileAttributes);
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
    CreateDirectoryW (strFolderName, NULL);
    SetFileAttributesW (strFolderName,FA_HIDDEN);
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
      AddEndSlashW (strRemoteName);
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
  RootPanel->GetCurDirW(strPanelDir);

  GetPathRootW(strPanelDir, strRoot);
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
  AnotherPanel->GetCurDirW(strPanelDir);

  if ( !strPanelDir.IsEmpty() )
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


void TreeList::PR_MsgReadTree(void)
{
  int FirstCall=1;
  TreeList::MsgReadTree(PreRedrawParam.Flags,FirstCall);
}

int TreeList::MsgReadTree(int TreeCount,int &FirstCall)
{
  /* $ 24.09.2001 VVM
    ! ������ ��������� � ������ ������ ������, ���� ��� ������ ����� 500 ����. */
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
    MessageW((FirstCall ? 0:MSG_KEEPBACKGROUND),0,UMSG(MTreeTitle),
            UMSG(MReadingTree),NumStr);
    PreRedrawParam.Flags=TreeCount;
    TreeStartTime = clock();
  }
  /* VVM $ */
  return(1);
}


void TreeList::FillLastData()
{
  long Last,Depth,PathLength,SubDirPos,I,J;
  int RootLength = strRoot.GetLength()-1;
  if (RootLength<0)
    RootLength=0;
  for (I=1;I<TreeCount;I++)
  {
    PathLength=wcsrchr(ListData[I].strName,L'\\')-(const wchar_t*)ListData[I].strName+1;
    Depth=ListData[I].Depth=CountSlash((const wchar_t*)ListData[I].strName+RootLength);
    for (J=I+1,SubDirPos=I,Last=1;J<TreeCount;J++)
      if (CountSlash((const wchar_t*)ListData[J].strName+RootLength)>Depth)
      {
        SubDirPos=J;
        continue;
      }
      else
      {
        if ( LocalStrnicmpW(ListData[I].strName,ListData[J].strName,PathLength)==0 )
          Last=0;
        break;
      }
    for (J=I;J<=SubDirPos;J++)
      ListData[J].Last[Depth-1]=Last;
  }
}


int TreeList::CountSlash(const wchar_t *Str)
{
  int Count=0;
  while ((Str=wcschr(Str,L'\\'))!=NULL)
  {
    Str++;
    Count++;
  }
  return(Count);
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
/*
    case MCODE_OP_PLAINTEXT:
    {
      const char *str = eStackAsString();
      if (!*str)
        return FALSE;
      Key=*str;
      break;
    }
*/
    case MCODE_C_EMPTY:
      return TreeCount<=0;
    case MCODE_C_EOF:
      return CurFile==TreeCount-1;
    case MCODE_C_BOF:
      return CurFile==0;
    case MCODE_C_SELECTED:
      return FALSE;
    case MCODE_V_ITEMCOUNT:
      return TreeCount;
    case MCODE_V_CURPOS:
      return CurFile+1;
/*
    case MCODE_F_MENU_CHECKHOTKEY:
    {
      const char *str = eStackAsString(1);
      if ( *str )
        return CheckHighlights(*str);
      return FALSE;
    }
*/
  }

  switch(Key)
  {
    /* $ 08.12.2001 IS ������ ������� ��� "������", �� � �������
    */
    case KEY_F1:
    {
      {
         Help Hlp (L"TreePanel");
      }
      return TRUE;
    }
    /* IS $ */

    case KEY_SHIFTENTER:
    case KEY_CTRLENTER:
    case KEY_CTRLF:
    case KEY_CTRLALTINS:  case KEY_CTRLALTNUMPAD0:
    {
      {
        string strQuotedName;
        int CAIns=(Key == KEY_CTRLALTINS || Key == KEY_CTRLALTNUMPAD0);

        CurPtr=ListData+CurFile;
        if (wcschr(CurPtr->strName,L' ')!=NULL)
          strQuotedName.Format (L"\"%s\"%s",(const wchar_t *)CurPtr->strName,(CAIns?L"":L" "));
        else
          strQuotedName.Format (L"%s%s",(const wchar_t *)CurPtr->strName,(CAIns?L"":L" "));

        if(CAIns)
          CopyToClipboardW(strQuotedName);

        else if(Key == KEY_SHIFTENTER)
          Execute(strQuotedName,FALSE,TRUE,TRUE);
        else
          CtrlObject->CmdLine->InsertStringW(strQuotedName);
      }
      return(TRUE);
    }

    case KEY_CTRLBACKSLASH:
    {
      CurFile=0;
      ProcessEnter();
      return(TRUE);
    }

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
        ShellCopy ShCopy(this,Key==KEY_SHIFTF6,FALSE,TRUE,TRUE,ToPlugin,NULL, TRUE); //UNICODE!!!
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
        if(Key==KEY_ALTF6 && !Link) // ����� ������� :-)
          return TRUE;

        {
          ShellCopy ShCopy(this,Move,Link,FALSE,Ask,ToPlugin,NULL, TRUE); //UNICODE!!!
        }

        if (ToPlugin==1)
        {
          struct PluginPanelItemW *ItemList=new PluginPanelItemW[1];
          int ItemNumber=1;
          HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();

          FileList::FileNameToPluginItem(ListData[CurFile].strName,ItemList);
          int PutCode=CtrlObject->Plugins.PutFiles(hAnotherPlugin,ItemList,ItemNumber,Move,0);
          if (PutCode==1 || PutCode==2)
            AnotherPanel->SetPluginModified();
          /* $ 13.07.2000 SVS
             �� ���� ��������� new/delete � realloc
          */
          xf_free(ItemList);
          /* SVS $ */
          if (Move)
            ReadSubTree(ListData[CurFile].strName);
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
      ��������                                   Shift-Del, Shift-F8, F8

      �������� ������ � �����. F8 � Shift-Del ������� ��� ���������
     �����, Shift-F8 - ������ ���� ��� ��������. Shift-Del ������ �������
     �����, �� ��������� ������� (Recycle Bin). ������������� �������
     ��������� F8 � Shift-F8 ������� �� ������������.

      ����������� ������ � �����                                 Alt-Del
    */
    case KEY_F8:
    case KEY_SHIFTDEL:
    case KEY_ALTDEL:
    {
      if (SetCurPath())
      {
        int SaveOpt=Opt.DeleteToRecycleBin;
        if (Key==KEY_SHIFTDEL)
          Opt.DeleteToRecycleBin=0;
        ShellDelete(this,Key==KEY_ALTDEL);
        // ������� �� ������ �������� ��������������� ������...
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

    case KEY_HOME:        case KEY_SHIFTNUMPAD7:
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

    case KEY_END:         case KEY_SHIFTNUMPAD1:
    {
      Down(0x7fffff);
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    }

    case KEY_UP:          case KEY_SHIFTNUMPAD8:
    {
      Up(1);
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    }

    case KEY_DOWN:        case KEY_SHIFTNUMPAD2:
    {
      Down(1);
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    }

    case KEY_PGUP:        case KEY_SHIFTNUMPAD9:
    {
      CurTopFile-=Y2-Y1-3-ModalMode;
      CurFile-=Y2-Y1-3-ModalMode;
      DisplayTree(TRUE);
      if (Opt.Tree.AutoChangeFolder && !ModalMode)
        ProcessKey(KEY_ENTER);
      return(TRUE);
    }

    case KEY_PGDN:        case KEY_SHIFTNUMPAD3:
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
  if(CurFile+1 < TreeCount)
  {
    int CurDepth=ListData[CurFile].Depth;
    for(int I=CurFile+1; I < TreeCount; ++I)
      if(ListData[I].Depth == CurDepth)
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
    int CurDepth=ListData[CurFile].Depth;
    for(int I=CurFile-1; I > 0; --I)
      if(ListData[I].Depth == CurDepth)
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
void TreeList::SetCurDirW(const wchar_t *NewDir,int ClosePlugin)
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
    CtrlObject->CmdLine->SetCurDirW(NewDir);
    CtrlObject->CmdLine->Show();
  }
}


#if defined(__BORLANDC__)
#pragma warn +par
#endif


int TreeList::SetDirPosition(const wchar_t *NewDir)
{
  long I;
  for (I=0;I<TreeCount;I++)
  {
    if (LocalStricmpW(NewDir,ListData[I].strName)==0)
    {
      WorkDir=CurFile=I;
      CurTopFile=CurFile-(Y2-Y1-1)/2;
      CorrectPosition();
      return(TRUE);
    }
  }
  return(FALSE);
}


int TreeList::GetCurDirW (string &strCurDir)
{
  if (TreeCount==0)
  {
    if (ModalMode==MODALTREE_FREE)
      strCurDir = strRoot;
    else
      strCurDir = L"";
  }
  else
    strCurDir = ListData[CurFile].strName; //BUGBUG

  return strCurDir.GetLength();
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
  if ((Attr=GetFileAttributesW(CurPtr->strName))!=(DWORD)-1 && (Attr & FA_DIREC))
  {
    if (!ModalMode && FarChDirW(CurPtr->strName))
    {
      Panel *AnotherPanel=GetRootPanel();
      SetCurDirW(CurPtr->strName,TRUE);
      Show();

      AnotherPanel->SetCurDirW(CurPtr->strName,TRUE);
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
  wchar_t DirName[NM]; //BUGBUG, todo better!!!
  wchar_t LastDirName[NM],*ChPtr;

  FILE *TreeFile=NULL;
  int RootLength=strRoot.GetLength()-1;
  if (RootLength<0)
    RootLength=0;

  string strName;

  FlushCache();
  MkTreeFileName(strRoot,strName);

  if (MustBeCached(strRoot) || (TreeFile=_wfopen(strName,L"rb"))==NULL)
    if (!GetCacheTreeName(strRoot,strName,FALSE) || (TreeFile=_wfopen(strName,L"rb"))==NULL)
      return(FALSE);
  /* $ 13.07.2000 SVS
     �� ���� ��������� new/delete � realloc
  */
  xf_free(ListData);
  /* SVS $ */
  ListData=NULL;
  TreeCount=0;
  *LastDirName=0;

  wcscpy (DirName, strRoot);//BUGBUG

  while (fgetws(DirName+RootLength, NM-RootLength,TreeFile)!=NULL)
  {
    if ( LocalStricmpW (DirName,LastDirName)==0)
      continue;

    wcscpy (LastDirName,DirName);

    if ((ChPtr=wcschr(DirName,L'\n'))!=NULL)
      *ChPtr=0;
    if (RootLength>0 && DirName[RootLength-1]!=L':' &&
        DirName[RootLength]==L'\\' && DirName[RootLength+1]==0)
      DirName[RootLength]=0;

    if ((TreeCount & 255)==0 )
    {
      int OldCount = TreeCount;

      ListData=(struct TreeItem *)xf_realloc(ListData,(TreeCount+256+1)*sizeof(struct TreeItem));

      if ( ListData )
        memset (ListData+OldCount, 0, (TreeCount+256+1-OldCount)*sizeof (TreeItem));
      else
      {
        xf_free(ListData);
      /* SVS $ */
        ListData=NULL;
        TreeCount=0;
        fclose(TreeFile);
        return(FALSE);
      }
    }

    ListData[TreeCount].strName = DirName;

    //UnicodeToAnsi (DirName, ListData[TreeCount].Name);

    TreeCount++;
  }
  fclose(TreeFile);

  if (TreeCount==0)
    return(FALSE);
  CaseSensitiveSort=FALSE;
  NumericSort=FALSE;

  FillLastData();
  return(TRUE);
}


int TreeList::FindPartName(const wchar_t *Name,int Next,int Direct)
{
  string strMask;

  strMask = Name;
  strMask += L"*";

  int I;
  for (I=CurFile+(Next?Direct:0); I >= 0 && I < TreeCount; I+=Direct)
  {
    CmpNameSearchMode=(I==CurFile);

    if (CmpNameW(strMask,ListData[I].strName,TRUE))
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
    if (CmpNameW(strMask,ListData[I].strName,TRUE))
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


int TreeList::GetSelNameW (string *strName,int &FileAttr,string *strShortName,FAR_FIND_DATA_EX *fd)
{
  if ( strName==NULL)
  {
    GetSelPosition=0;
    return(TRUE);
  }

  if (GetSelPosition==0)
  {
    GetCurDirW(*strName);
    if ( strShortName != NULL)
      *strShortName = *strName;
    FileAttr=FA_DIREC;
    GetSelPosition++;
    return(TRUE);
  }
  GetSelPosition=0;
  return(FALSE);
}


int TreeList::GetCurNameW(string &strName, string &strShortName)
{
  if (TreeCount==0)
  {
    strName = L"";
    strShortName = L"";
    return(FALSE);
  }

  strName = ListData[CurFile].strName;
  strShortName = strName;
  return(TRUE);
}


void TreeList::AddTreeName(const wchar_t *Name)
{
  string strRoot;

  const wchar_t *ChPtr;
  long CachePos;

  if (*Name==0)
    return;

  string strFullName;

  ConvertNameToFullW(Name, strFullName);

  Name = strFullName;

  GetPathRootW(Name, strRoot);

  Name += strRoot.GetLength ()-1;

  if ((ChPtr=wcsrchr(Name,L'\\'))==NULL)
    return;

  ReadCache(strRoot);
  for(CachePos=0;CachePos<TreeCache.TreeCount;CachePos++)
  {
    int Result=LocalStricmpW(TreeCache.ListName[CachePos],Name);
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
  ConvertNameToFullW(Name,strFullName);
  Name=strFullName;

  GetPathRootW(Name, strRoot);

  Name += strRoot.GetLength()-1;
  ReadCache(strRoot);
  for (CachePos=0;CachePos<TreeCache.TreeCount;CachePos++)
  {
    wszDirName=TreeCache.ListName[CachePos];
    Length=wcslen(Name);
    DirLength=wcslen(wszDirName);
    if(DirLength<Length)continue;
    if (LocalStrnicmpW(Name,wszDirName,Length)==0 &&
        (wszDirName[Length]==0 || wszDirName[Length]==L'\\'))
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
  GetPathRootW(SrcName,strSrcRoot);
  GetPathRootW(DestName,strDestRoot);

  if ( LocalStricmpW (strSrcRoot,strDestRoot)!=0)
  {
    DelTreeName(SrcName);
    ReadSubTree(SrcName);
  }

  SrcName+=strSrcRoot.GetLength()-1;
  DestName+=strDestRoot.GetLength()-1;

  ReadCache(strSrcRoot);

  int SrcLength=wcslen(SrcName);

  for (int CachePos=0;CachePos<TreeCache.TreeCount;CachePos++)
  {
    const wchar_t *DirName=TreeCache.ListName[CachePos];
    if (LocalStrnicmpW(SrcName,DirName,SrcLength)==0 &&
        (DirName[SrcLength]==0 || DirName[SrcLength]==L'\\'))
    {
      string strNewName = DestName;

      strNewName += (const wchar_t*)(DirName+SrcLength);

      xf_free(TreeCache.ListName[CachePos]);

      TreeCache.ListName[CachePos]=_wcsdup(strNewName);
    }
  }
}


void TreeList::ReadSubTree(const wchar_t *Path)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  //SaveScreen SaveScr;
  ScanTree ScTree(FALSE);
  FAR_FIND_DATA_EX fdata;

  string strDirName;
  string strFullName;

  int Count=0,FileAttr;

  if ((FileAttr=GetFileAttributesW(Path))==-1 || (FileAttr & FA_DIREC)==0)
    return;

  ConvertNameToFullW(Path, strDirName);

  AddTreeName(strDirName);

  int FirstCall=TRUE, AscAbort=FALSE;

  ScTree.SetFindPathW(strDirName,L"*.*",0);

  SetPreRedrawFunc(TreeList::PR_MsgReadTree);

  LastScrX = ScrX;
  LastScrY = ScrY;

  while (ScTree.GetNextNameW(&fdata, strFullName))
  {
    if (fdata.dwFileAttributes & FA_DIREC)
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
  SetPreRedrawFunc(NULL);
}


void TreeList::ClearCache(int EnableFreeMem)
{
  TreeCache.Clean();
}


void TreeList::ReadCache(const wchar_t *TreeRoot)
{
  string strTreeName;
  wchar_t DirName[NM]; //BUGBUG, to do better!!!

  wchar_t *ChPtr;
  FILE *TreeFile=NULL;

  if (wcscmp(MkTreeFileName(TreeRoot,strTreeName),TreeCache.strTreeName)==0)
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

  while (fgetws(DirName, NM, TreeFile)!=NULL)
  {
    if ((ChPtr=wcschr(DirName,L'\n'))!=NULL)
      *ChPtr=0;

    TreeCache.Add(DirName);
  }
  fclose(TreeFile);
}


void TreeList::FlushCache()
{
  FILE *TreeFile;
  int I;
  if ( !TreeCache.strTreeName.IsEmpty() )
  {
    DWORD FileAttributes=GetFileAttributesW(TreeCache.strTreeName);
    if(FileAttributes != -1)
      SetFileAttributesW(TreeCache.strTreeName,FILE_ATTRIBUTE_NORMAL);
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

      DeleteFileW (TreeCache.strTreeName);

      //remove(TreeCache.TreeName); BUGBUG

      MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MCannotSaveTree),
              TreeCache.strTreeName,UMSG(MOk));
    }
    else if(FileAttributes != -1) // ������ �������� (���� ��������� :-)
      SetFileAttributesW(TreeCache.strTreeName,FileAttributes);
  }
  ClearCache(1);
}


void TreeList::UpdateViewPanel()
{
  if (!ModalMode)
  {
    Panel *AnotherPanel=GetRootPanel();

    string strCurName;
    GetCurDirW(strCurName);

    if (AnotherPanel->GetType()==QVIEW_PANEL && SetCurPath())
      ((QuickView *)AnotherPanel)->ShowFile(strCurName,FALSE,NULL);
  }
}


int TreeList::GoToFileW(const wchar_t *Name,BOOL OnlyPartName)
{
  long Pos=FindFileW(Name,OnlyPartName);
  if (Pos!=-1)
  {
    CurFile=Pos;
    CorrectPosition();
    return(TRUE);
  }
  return(FALSE);

}

int TreeList::FindFileW(const wchar_t *Name,BOOL OnlyPartName)
{
  long I;
  struct TreeItem *CurPtr;

  for (CurPtr=ListData, I=0; I < TreeCount; I++, CurPtr++)
  {
    const wchar_t *CurPtrName=CurPtr->strName;
    if(OnlyPartName)
      CurPtrName=PointToNameW(CurPtr->strName);

    if (wcscmp(Name,CurPtrName)==0)
      return I;

    if (LocalStricmpW(Name,CurPtrName)==0)
      return I;
  }
  return -1;
}


int TreeList::FindFirstW(const wchar_t *Name)
{
  return FindNextW(0,Name);
}

int TreeList::FindNextW(int StartPos, const wchar_t *Name)
{
  int I;
  struct TreeItem *CurPtr;

  if((DWORD)StartPos < (DWORD)TreeCount)
    for( CurPtr=ListData+StartPos, I=StartPos; I < TreeCount; I++, CurPtr++ )
    {
      const wchar_t *CurPtrName=CurPtr->strName;
      if (CmpNameW(Name,CurPtrName,TRUE))
        if (!TestParentFolderNameW(CurPtrName))
          return I;
    }
  return -1;
}

int TreeList::GetFileNameW(string &strName,int Pos,int &FileAttr)
{
  if (Pos < 0 || Pos >= TreeCount)
    return FALSE;

  strName = ListData[Pos].strName;

  FileAttr=FA_DIREC|GetFileAttributesW(ListData[Pos].strName);

  return TRUE;
}


int _cdecl SortList(const void *el1,const void *el2)
{
  string strName1=((struct TreeItem *)el1)->strName;
  string strName2=((struct TreeItem *)el2)->strName;
  //if(!StaticSortNumeric) BUGBUG
    return(StaticSortCaseSensitive ? TreeCmp(strName1,strName2):LocalStricmpW(strName1,strName2));
  //else
   // return(StaticSortCaseSensitive ? TreeCmp(strName1,strName2):LCNumStricmp(NamePtr1,NamePtr2));
}

int _cdecl SortCacheList(const void *el1,const void *el2)
{
//  if(!StaticSortNumeric)
    return(LocalStricmpW(*(wchar_t **)el1,*(wchar_t **)el2));
//  else
//    return(LCNumStricmp(*(char **)el1,*(char **)el2));
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
 �������, ������������� ������������� �����������
 ����� */
int TreeList::MustBeCached(const wchar_t *Root)
{
    UINT type;

    type=FAR_GetDriveTypeW (Root);
    if ( type==DRIVE_UNKNOWN ||
         type==DRIVE_NO_ROOT_DIR ||
         type==DRIVE_REMOVABLE ||
         IsDriveTypeCDROM(type)
         )
    {
        if ( type==DRIVE_REMOVABLE )
        {
            if ( LocalUpperW(Root[0])==L'A' || LocalUpperW(Root[0])==L'B')
                return FALSE; // ��� �������
        }
        return TRUE;
        // ���������� CD, removable � ���������� ��� :)
    }
    /* ��������
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
    if (GetFileAttributesW(CurPtr->strName)==(DWORD)-1)
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
  KB->ClearGroup (KBL_CTRLSHIFT);
  KB->ClearGroup (KBL_CTRLALT);
  KB->ClearGroup (KBL_ALTSHIFT);

  DynamicUpdateKeyBar();

  return TRUE;
}

void TreeList::DynamicUpdateKeyBar()
{
  ;//KeyBar *KB = CtrlObject->MainKeyBar;
}

void TreeList::SetTitle()
{
  if (GetFocus())
  {
    string strTitleDir; //BUGBUG
    const wchar_t *Ptr=L"";
    if(ListData)
    {
      struct TreeItem *CurPtr=ListData+CurFile;
      Ptr=CurPtr->strName;
    }
    if (*Ptr)
      strTitleDir.Format (L"{%s - Tree}",Ptr);
    else
      strTitleDir = L"{Tree}";

    strLastFarTitle = strTitleDir;
    SetFarTitleW(strTitleDir);
  }
}

/*
   "Local AppData" = [HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders]/Local AppData
   "AppData"       = [HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders]/AppData

*/

// TODO: ����� "Tree.Far" ��� ��������� ������ ������ ��������� � "Local AppData\Far"
// TODO: ����� "Tree.Far" ��� ������� ������ ������ ��������� � "%HOMEDRIVE%\%HOMEPATH%",
//                        ���� ��� ���������� ����� �� ����������, �� "%APPDATA%\Far"
// �p���� "X.tree" (��� 'X'  - ����� �����, ���� �� ������� ����)
// �p���� "server.share.tree" - ��� �������� ����� ��� �����
string &TreeList::MkTreeFileName(const wchar_t *RootDir,string &strDest)
{
    strDest = RootDir;
    AddEndSlashW (strDest);

    strDest += L"tree2.far";

    return strDest;
}

// TODO: ����� �������� (Tree.Cache) ����� �� � FarPath, � � "Local AppData\Far\"
string &TreeList::MkTreeCacheFolderName(const wchar_t *RootDir,string &strDest)
{
    strDest = RootDir;
    AddEndSlashW (strDest);

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

   ��������� ������ - "X.nnnnnnnn.tree"
   ������� ������ - "X.nnnnnnnn.tree"
   ������� ����� - "Server.share.tree"
   ������� ������(DRIVE_REMOVABLE) - "Far.nnnnnnnn.tree"
   ������� ������(CD) - "Label.nnnnnnnn.tree"

*/
string &TreeList::CreateTreeFileName(const wchar_t *Path,string &strDest)
{
#if 0
  char RootPath[NM];
  GetPathRoot(Path,RootPath);
  UINT DriveType = FAR_GetDriveType(RootPath,NULL,FALSE);

  // ��������� ���� � ����
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
  if((DWORD)Index >= TreeCount)
    return FALSE;
  memcpy(Dest,ListData+Index,sizeof(struct TreeItem));
  return TRUE;
}

int TreeList::GetCurrentPos()
{
  return CurFile;
}
