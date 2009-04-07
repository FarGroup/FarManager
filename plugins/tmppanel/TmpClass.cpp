/*
TMPCLASS.CPP

Temporary panel plugin class implementation

*/

#include "TmpPanel.hpp"

static int _cdecl SortListCmp(const void *el1,const void *el2);

TmpPanel::TmpPanel()
{
  LastOwnersRead=FALSE;
  LastLinksRead=FALSE;
  UpdateNotNeeded=FALSE;
  TmpPanelItem=NULL;
  TmpItemsNumber=0;
  PanelIndex=CurrentCommonPanel;
  IfOptCommonPanel();
}


TmpPanel::~TmpPanel()
{
  if(!StartupOptCommonPanel)
    FreePanelItems(TmpPanelItem, TmpItemsNumber);
}

int TmpPanel::GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  IfOptCommonPanel();
#ifndef UNICODE
  struct PanelInfo PInfo;
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
  UpdateItems(IsOwnersDisplayed (PInfo.ColumnTypes),IsLinksDisplayed (PInfo.ColumnTypes));
#else
  int Size=Info.Control(this,FCTL_GETCOLUMNTYPES,0,NULL);
  wchar_t* ColumnTypes=new wchar_t[Size];
  Info.Control(this,FCTL_GETCOLUMNTYPES,Size,(LONG_PTR)ColumnTypes);
  UpdateItems(IsOwnersDisplayed (ColumnTypes),IsLinksDisplayed (ColumnTypes));
  delete[] ColumnTypes;
#endif
  *pPanelItem=TmpPanelItem;
  *pItemsNumber=TmpItemsNumber;

  return(TRUE);
}


void TmpPanel::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=OPIF_USEFILTER|OPIF_USESORTGROUPS|OPIF_USEHIGHLIGHTING|
    OPIF_ADDDOTS|OPIF_SHOWNAMESONLY;

  if(!Opt.SafeModePanel) Info->Flags|=OPIF_REALNAMES;

  Info->HostFile=NULL;
  Info->CurDir=_T("");

  Info->Format=(TCHAR*)GetMsg(MTempPanel);

  static TCHAR Title[MAX_PATH];
#define PANEL_MODE  (Opt.SafeModePanel ? _T("(R) ") : _T(""))
  if(StartupOptCommonPanel)
    FSF.sprintf(Title,GetMsg(MTempPanelTitleNum),PANEL_MODE,PanelIndex);
  else
    FSF.sprintf(Title,_T(" %s%s "),PANEL_MODE,GetMsg(MTempPanel));
#undef PANEL_MODE

  Info->PanelTitle=Title;

  static struct PanelMode PanelModesArray[10];
  PanelModesArray[4].FullScreen=(StartupOpenFrom==OPEN_COMMANDLINE)?
    Opt.FullScreenPanel:StartupOptFullScreenPanel;
  PanelModesArray[4].ColumnTypes=Opt.ColumnTypes;
  PanelModesArray[4].ColumnWidths=Opt.ColumnWidths;
  PanelModesArray[4].StatusColumnTypes=Opt.StatusColumnTypes;
  PanelModesArray[4].StatusColumnWidths=Opt.StatusColumnWidths;
  PanelModesArray[4].CaseConversion=TRUE;

  Info->PanelModesArray=PanelModesArray;
  Info->PanelModesNumber=ArraySize(PanelModesArray);
  Info->StartPanelMode=_T('4');
  static struct KeyBarTitles KeyBar;
  memset(&KeyBar,0,sizeof(KeyBar));
  KeyBar.Titles[7-1]=(TCHAR*)GetMsg(MF7);
  if(StartupOptCommonPanel)
    KeyBar.AltShiftTitles[12-1]=(TCHAR*)GetMsg(MAltShiftF12);
  KeyBar.AltShiftTitles[2-1]=(TCHAR*)GetMsg(MAltShiftF2);
  KeyBar.AltShiftTitles[3-1]=(TCHAR*)GetMsg(MAltShiftF3);
  Info->KeyBar=&KeyBar;
}


int TmpPanel::SetDirectory(const TCHAR *Dir,int OpMode)
{
  if((OpMode & OPM_FIND)/* || lstrcmp(Dir,_T("\\"))==0*/)
    return(FALSE);
  if(lstrcmp(Dir,_T("\\"))==0)
#ifndef UNICODE
    Info.Control(this,FCTL_CLOSEPLUGIN,(void*)NULL);
#else
    Info.Control(this,FCTL_CLOSEPLUGIN,0,NULL);
#endif
  else
#ifndef UNICODE
    Info.Control(this,FCTL_CLOSEPLUGIN,(void*)Dir);
#else
    Info.Control(this,FCTL_CLOSEPLUGIN,0,(LONG_PTR)Dir);
#endif
  return(TRUE);
}


int TmpPanel::PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int,int)
{
  UpdateNotNeeded=FALSE;

  HANDLE hScreen = BeginPutFiles();
  for(int i=0;i<ItemsNumber;i++)
  {
    if (!PutOneFile (PanelItem [i]))
    {
      CommitPutFiles (hScreen, FALSE);
      return FALSE;
    }
  }
  CommitPutFiles (hScreen, TRUE);
  return(1);
}

HANDLE TmpPanel::BeginPutFiles()
{
  IfOptCommonPanel();
  Opt.SelectedCopyContents = Opt.CopyContents;

  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
  const TCHAR *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempSendFiles)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ArraySize(MsgItems),0);
  return hScreen;
}

static inline int cmp_names(const WIN32_FIND_DATA &wfd, const FAR_FIND_DATA &ffd)
{
#ifndef UNICODE
#define _NAME cFileName
#else
#define _NAME lpwszFileName
#endif
  return lstrcmp(wfd.cFileName, FSF.PointToName(ffd._NAME));
#undef _NAME
}


int TmpPanel::PutOneFile(PluginPanelItem &PanelItem)
{
  TCHAR CurDir[NM];
  GetCurrentDirectory(ArraySize(CurDir),CurDir);
  FSF.AddEndSlash(CurDir);

  struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+1));
  if(NewPanelItem==NULL)
    return FALSE;
  TmpPanelItem=NewPanelItem;
  struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber++];
  memset(CurPanelItem,0,sizeof(*CurPanelItem));
  CurPanelItem->FindData=PanelItem.FindData;

#ifdef UNICODE
#define cFileName lpwszFileName
#endif
#define CurName PanelItem.FindData.cFileName
  if(CheckForCorrect(CurName,&CurPanelItem->FindData,Opt.AnyInPanel))
  {
    int NameOnly=(FSF.PointToName(CurName)==CurName);
#undef CurName

#ifndef UNICODE
    lstrcpy(CurPanelItem->FindData.cFileName, NameOnly ? CurDir : "");
    lstrcat(CurPanelItem->FindData.cFileName,PanelItem.FindData.cFileName);
#else
    size_t wlen = lstrlen(PanelItem.FindData.lpwszFileName);
    if (NameOnly) wlen += lstrlen(CurDir);
    register wchar_t *wp = (wchar_t*)malloc((wlen+1)*sizeof(wchar_t));
    lstrcpy(wp, NameOnly ? CurDir : L"");
    lstrcat(wp, PanelItem.FindData.lpwszFileName);
    CurPanelItem->FindData.lpwszFileName = wp;
#endif
    if(Opt.SelectedCopyContents && NameOnly && (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      if (Opt.SelectedCopyContents==2)
      {
        const TCHAR *MsgItems[]={GetMsg(MWarning),GetMsg(MCopyContensMsg)};
        Opt.SelectedCopyContents=!Info.Message(Info.ModuleNumber,FMSG_MB_YESNO,_T("Config"),
                                  MsgItems,ArraySize(MsgItems),0);
      }
      if (Opt.SelectedCopyContents)
      {
#ifndef UNICODE
        struct PluginPanelItem *DirItems;
#else
        FAR_FIND_DATA *DirItems;
#endif
        int DirItemsNumber;
        if(!Info.GetDirList(CurPanelItem->FindData.cFileName,
                            &DirItems,
                            &DirItemsNumber))
        {
          FreePanelItems(TmpPanelItem, TmpItemsNumber);
          TmpItemsNumber=0;
          return FALSE;
        }
        struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+DirItemsNumber));
        if(NewPanelItem==NULL)
          return FALSE;
        TmpPanelItem=NewPanelItem;
        memset(&TmpPanelItem[TmpItemsNumber],0,sizeof(*TmpPanelItem)*DirItemsNumber);
#ifdef UNICODE
        wlen = lstrlen(CurDir) + 1;
#endif
        for(int i=0;i<DirItemsNumber;i++)
        {
          struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber++];
#ifndef UNICODE
          CurPanelItem->FindData=DirItems[i].FindData;
          lstrcpy(CurPanelItem->FindData.cFileName,CurDir);
          lstrcat(CurPanelItem->FindData.cFileName,DirItems[i].FindData.cFileName);
#else
          CurPanelItem->FindData=DirItems[i];
          wp = (wchar_t*)malloc((wlen+lstrlen(DirItems[i].lpwszFileName))*sizeof(wchar_t));
          lstrcpy(wp, CurDir);
          lstrcat(wp, DirItems[i].lpwszFileName);
          CurPanelItem->FindData.lpwszFileName = wp;
#endif
        }
        Info.FreeDirList(DirItems
#ifdef UNICODE
                                 , DirItemsNumber
#endif
                        );
      }
    }
  }
  return TRUE;
}

void TmpPanel::CommitPutFiles (HANDLE hRestoreScreen, int Success)
{
  if (Success)
  {
    SortList();
    RemoveDups();
  }
  Info.RestoreScreen (hRestoreScreen);
}


int TmpPanel::SetFindList(const struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  HANDLE hScreen = BeginPutFiles();
  FindSearchResultsPanel();
  FreePanelItems(TmpPanelItem, TmpItemsNumber);
  TmpItemsNumber=0;
  TmpPanelItem=(PluginPanelItem*)malloc(sizeof(PluginPanelItem)*ItemsNumber);
  if(TmpPanelItem)
  {
    TmpItemsNumber=ItemsNumber;
    memcpy(TmpPanelItem,PanelItem,ItemsNumber*sizeof(*TmpPanelItem));
    for(int i=0;i<ItemsNumber;++i) {
      TmpPanelItem[i].Flags&=~PPIF_SELECTED;
      if(TmpPanelItem[i].Owner)
        TmpPanelItem[i].Owner = _tcsdup(TmpPanelItem[i].Owner);
#ifdef UNICODE
      if(TmpPanelItem[i].FindData.lpwszFileName)
        TmpPanelItem[i].FindData.lpwszFileName = wcsdup(TmpPanelItem[i].FindData.lpwszFileName);
#endif
    }
  }
  CommitPutFiles (hScreen, TRUE);
  UpdateNotNeeded=TRUE;
  return(TRUE);
}


void TmpPanel::FindSearchResultsPanel()
{
  if(StartupOptCommonPanel)
  {
    if (!Opt.NewPanelForSearchResults)
      IfOptCommonPanel();
    else
    {
      int SearchResultsPanel = -1;
      for (int i=0; i<COMMONPANELSNUMBER; i++)
      {
        if (CommonPanels [i].ItemsNumber == 0)
        {
          SearchResultsPanel = i;
          break;
        }
      }
      if (SearchResultsPanel < 0)
      {
        // all panels are full - use least recently used panel
        SearchResultsPanel = Opt.LastSearchResultsPanel++;
        if (Opt.LastSearchResultsPanel >= COMMONPANELSNUMBER)
          Opt.LastSearchResultsPanel = 0;
      }
      if(PanelIndex != SearchResultsPanel) {
        CommonPanels[PanelIndex].Items = TmpPanelItem;
        CommonPanels[PanelIndex].ItemsNumber = TmpItemsNumber;
        PanelIndex = SearchResultsPanel;
        TmpPanelItem = CommonPanels[PanelIndex].Items;
        TmpItemsNumber = CommonPanels[PanelIndex].ItemsNumber;
      }
      CurrentCommonPanel = PanelIndex;
    }
  }
}


void TmpPanel::SortList()
{
  FSF.qsort(TmpPanelItem,TmpItemsNumber,sizeof(*TmpPanelItem),SortListCmp);
}


int _cdecl SortListCmp(const void *el1,const void *el2)
{
  struct PluginPanelItem *Item1,*Item2;
  Item1=(struct PluginPanelItem *)el1;
  Item2=(struct PluginPanelItem *)el2;
  return(lstrcmp(Item1->FindData.cFileName,Item2->FindData.cFileName));
}


void TmpPanel::RemoveDups()
{
  struct PluginPanelItem *CurItem=TmpPanelItem;
  for(int i=0;i<TmpItemsNumber-1;i++,CurItem++)
    if(lstrcmp(CurItem->FindData.cFileName,CurItem[1].FindData.cFileName)==0)
      CurItem->Flags|=REMOVE_FLAG;
    RemoveEmptyItems();
}


void TmpPanel::RemoveEmptyItems()
{
  int EmptyCount=0;
  struct PluginPanelItem *CurItem=TmpPanelItem;
  for(int i=0;i<TmpItemsNumber;i++,CurItem++)
    if(CurItem->Flags & REMOVE_FLAG)
    {
      if(CurItem->Owner) {
        free(CurItem->Owner);
        CurItem->Owner = NULL;
      }
#ifdef UNICODE
      if(CurItem->FindData.lpwszFileName) {
        free((wchar_t*)CurItem->FindData.lpwszFileName);
        CurItem->FindData.lpwszFileName = NULL;
      }
#endif
      EmptyCount++;
    }
    else if(EmptyCount) {
      *(CurItem-EmptyCount)=*CurItem;
      memset(CurItem, 0, sizeof(*CurItem));
    }

  TmpItemsNumber-=EmptyCount;
  if(EmptyCount>1)
    TmpPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+1));

  if(StartupOptCommonPanel)
  {
    CommonPanels[PanelIndex].Items=TmpPanelItem;
    CommonPanels[PanelIndex].ItemsNumber=TmpItemsNumber;
  }
}


void TmpPanel::UpdateItems(int ShowOwners,int ShowLinks)
{
  if(UpdateNotNeeded || TmpItemsNumber == 0)
  {
    UpdateNotNeeded=FALSE;
    return;
  }
  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
  const TCHAR *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempUpdate)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ArraySize(MsgItems),0);
  LastOwnersRead=ShowOwners;
  LastLinksRead=ShowLinks;
  struct PluginPanelItem *CurItem=TmpPanelItem;
  for(int i=0;i<TmpItemsNumber;i++,CurItem++)
  {
    TCHAR FullName[NM];
    HANDLE FindHandle;
    lstrcpy(FullName,CurItem->FindData.cFileName);

    TCHAR *Slash=_tcsrchr(/*(const TCHAR*)*/FullName,_T('\\'));
    int Length=Slash ? (int)(Slash-FullName+1):0;

    int SameFolderItems=1;
    /* $ 23.12.2001 DJ
       если FullName - это каталог, то FindFirstFile (FullName+"*.*")
       этот каталог не найдет. Поэтому для каталогов оптимизацию с
       SameFolderItems пропускаем.
    */
    if(Length>0 && Length > (int)lstrlen (FullName))   /* DJ $ */
      for(int j=1;i+j<TmpItemsNumber;j++)
        if(memcmp(FullName,CurItem[j].FindData.cFileName,Length*sizeof(TCHAR))==0 &&
          _tcschr((const TCHAR*)CurItem[j].FindData.cFileName+Length,_T('\\'))==0)
          SameFolderItems++;
        else
          break;

    // SameFolderItems - оптимизация для случая, когда в панели лежат
    // несколько файлов из одного и того же каталога. При этом
    // FindFirstFile() делается один раз на каталог, а не отдельно для
    // каждого файла.
    if(SameFolderItems>2)
    {
      WIN32_FIND_DATA FindData;
      lstrcpy(Slash+1,_T("*.*"));
      for(int J=0;J<SameFolderItems;J++)
        CurItem[J].Flags|=REMOVE_FLAG;
      int Done=(FindHandle=FindFirstFile(FullName,&FindData))==INVALID_HANDLE_VALUE;
      while(!Done)
      {
        for(int J=0;J<SameFolderItems;J++)
          if((CurItem[J].Flags & 1) && cmp_names(FindData, CurItem[J].FindData)==0)
          {
            CurItem[J].Flags&=~REMOVE_FLAG;
#ifndef UNICODE
            lstrcpy(FullName,CurItem[J].FindData.cFileName);
#else
            wchar_t *save = CurItem[J].FindData.lpwszFileName;
#endif
            WFD2FFD(FindData,CurItem[J].FindData);
#ifndef UNICODE
            lstrcpy(CurItem[J].FindData.cFileName,FullName);
#else
            free((wchar_t*)CurItem[J].FindData.lpwszFileName);
            CurItem[J].FindData.lpwszFileName = save;
#endif
            break;
          }
          Done=!FindNextFile(FindHandle,&FindData);
      }
      FindClose(FindHandle);
      i+=SameFolderItems-1;
      CurItem+=SameFolderItems-1;
    }
    else
      if(!CheckForCorrect(FullName,&CurItem->FindData,Opt.AnyInPanel))
        CurItem->Flags|=REMOVE_FLAG;
  }

  RemoveEmptyItems();

  if(ShowOwners || ShowLinks)
  {
    struct PluginPanelItem *CurItem=TmpPanelItem;
    for(int i=0;i<TmpItemsNumber;i++,CurItem++)
    {
      if(ShowOwners)
      {
        TCHAR Owner[80];
        if(CurItem->Owner)
        {
          free(CurItem->Owner);
          CurItem->Owner=NULL;
        }
        if(FSF.GetFileOwner(NULL,CurItem->FindData.cFileName,Owner,80))
          CurItem->Owner=_tcsdup(Owner);
      }
      if(ShowLinks)
        CurItem->NumberOfLinks=FSF.GetNumberOfLinks(CurItem->FindData.cFileName);
    }
  }
  Info.RestoreScreen(hScreen);
}


int TmpPanel::ProcessEvent(int Event,void *)
{
  if(Event==FE_CHANGEVIEWMODE)
  {
    IfOptCommonPanel();
#ifndef UNICODE
    struct PanelInfo PInfo;
    Info.Control(this,FCTL_GETPANELINFO,&PInfo);
    int UpdateOwners=IsOwnersDisplayed (PInfo.ColumnTypes) && !LastOwnersRead;
    int UpdateLinks=IsLinksDisplayed (PInfo.ColumnTypes) && !LastLinksRead;
#else
    int Size=Info.Control(this,FCTL_GETCOLUMNTYPES,0,NULL);
    wchar_t* ColumnTypes=new wchar_t[Size];
    Info.Control(this,FCTL_GETCOLUMNTYPES,Size,(LONG_PTR)ColumnTypes);
    int UpdateOwners=IsOwnersDisplayed (ColumnTypes) && !LastOwnersRead;
    int UpdateLinks=IsLinksDisplayed (ColumnTypes) && !LastLinksRead;
    delete[] ColumnTypes;
#endif


    if(UpdateOwners || UpdateLinks)
    {
      UpdateItems(UpdateOwners,UpdateLinks);
#ifndef UNICODE
      Info.Control(this,FCTL_UPDATEPANEL,(void *)TRUE);
      Info.Control(this,FCTL_REDRAWPANEL,NULL);
#else
      Info.Control(this,FCTL_UPDATEPANEL,TRUE,NULL);
      Info.Control(this,FCTL_REDRAWPANEL,0,NULL);
#endif
    }
  }
  return(FALSE);
}


int TmpPanel::IsCurrentFileCorrect (TCHAR *pCurFileName)
{
  struct PanelInfo PInfo;
  TCHAR CurFileName[NM];
#ifndef UNICODE
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
  lstrcpy(CurFileName, PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName);
#else
  Info.Control(this,FCTL_GETPANELINFO,0,(LONG_PTR)&PInfo);
  PluginPanelItem* PPI=(PluginPanelItem*)malloc(Info.Control(this,FCTL_GETPANELITEM,PInfo.CurrentItem,0));
  if(PPI)
  {
    Info.Control(this,FCTL_GETPANELITEM,PInfo.CurrentItem,(LONG_PTR)PPI);
    lstrcpy(CurFileName,PPI->FindData.lpwszFileName);
    free(PPI);
  }
#endif

  BOOL IsCorrectFile = FALSE;
  if (lstrcmp (CurFileName, _T("..")) == 0)
    IsCorrectFile = TRUE;
  else
  {
    FAR_FIND_DATA TempFindData;
    IsCorrectFile=CheckForCorrect(CurFileName,&TempFindData,FALSE);
  }
  if (pCurFileName)
    lstrcpy (pCurFileName, CurFileName);
  return IsCorrectFile;
}

int TmpPanel::ProcessKey (int Key,unsigned int ControlState)
{
  if(!ControlState && Key==VK_F1)
  {
    Info.ShowHelp(Info.ModuleName, NULL, FHELP_USECONTENTS|FHELP_NOSHOWERROR);
    return TRUE;
  }

  if(ControlState==(PKF_SHIFT|PKF_ALT) && Key==VK_F9)
  {
     EXP_NAME(Configure)(0);
     return TRUE;
  }

  if(ControlState==(PKF_SHIFT|PKF_ALT) && Key==VK_F3)
  {
    TCHAR CurFileName [NM];
    if (IsCurrentFileCorrect(CurFileName))
    {
      struct PanelInfo PInfo;
#ifndef UNICODE
      Info.Control(this,FCTL_GETPANELINFO,&PInfo);
#else
      Info.Control(this,FCTL_GETPANELINFO,0,(LONG_PTR)&PInfo);
#endif
      if (lstrcmp(CurFileName,_T(".."))!=0)
      {
#ifndef UNICODE

        if (PInfo.PanelItems[PInfo.CurrentItem].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        {
#else
        PluginPanelItem* PPI=(PluginPanelItem*)malloc(Info.Control(this,FCTL_GETPANELITEM,PInfo.CurrentItem,0));
        DWORD attributes=0;
        if(PPI)
        {
          Info.Control(this,FCTL_GETPANELITEM,PInfo.CurrentItem,(LONG_PTR)PPI);
          attributes=PPI->FindData.dwFileAttributes;
          free(PPI);
        }
        if(attributes&FILE_ATTRIBUTE_DIRECTORY)
        {
#endif
#ifndef UNICODE
          Info.Control(INVALID_HANDLE_VALUE, FCTL_SETANOTHERPANELDIR,&CurFileName);
#else
          Info.Control(PANEL_PASSIVE, FCTL_SETPANELDIR,0,(LONG_PTR)&CurFileName);
#endif
        }
        else
          GoToFile(CurFileName, true);
#ifndef UNICODE
        Info.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWANOTHERPANEL,NULL);
#else
        Info.Control(PANEL_PASSIVE, FCTL_REDRAWPANEL,0,NULL);
#endif
        return(TRUE);
      }
    }
  }

  if (ControlState!=PKF_CONTROL && Key>=VK_F3 && Key<=VK_F8 && Key!=VK_F7)
  {
    if(!IsCurrentFileCorrect (NULL))
      return(TRUE);
  }

  if(ControlState==0 && Key==VK_RETURN && Opt.AnyInPanel)
  {
    TCHAR CurFileName [NM];
    if (!IsCurrentFileCorrect (CurFileName))
    {
#ifndef UNICODE
      Info.Control(this,FCTL_SETCMDLINE,&CurFileName);
#else
      Info.Control(this,FCTL_SETCMDLINE,0,(LONG_PTR)&CurFileName);
#endif
      return(TRUE);
    }
  }

  if (Opt.SafeModePanel && ControlState == PKF_CONTROL && Key == VK_PRIOR)
  {
    TCHAR CurFileName[NM];
    CurFileName[0] = _T('\0');
    if(IsCurrentFileCorrect(CurFileName) && 0!=lstrcmp(CurFileName,_T("..")))
    {
      GoToFile(CurFileName, false);
      return TRUE;
    }
    if(!lstrcmp(CurFileName,_T("..")))
    {
      SetDirectory(_T("."),0);
      return TRUE;
    }
  }

  if(ControlState==0 && Key==VK_F7)
  {
    ProcessRemoveKey();
    return TRUE;
  }
  else if (ControlState == (PKF_SHIFT|PKF_ALT) && Key == VK_F2)
  {
    ProcessSaveListKey();
    return TRUE;
  }
  else
  {
    if(StartupOptCommonPanel && ControlState==(PKF_SHIFT|PKF_ALT))
    {
      if (Key==VK_F12)
      {
        ProcessPanelSwitchMenu();
        return(TRUE);
      }
      else if (Key >= _T('0') && Key <= _T('9'))
      {
        SwitchToPanel (Key - _T('0'));
        return TRUE;
      }
    }
  }
  return(FALSE);
}


void TmpPanel::ProcessRemoveKey()
{
  IfOptCommonPanel();

  struct PanelInfo PInfo;
#ifndef UNICODE
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
#else
  Info.Control(this,FCTL_GETPANELINFO,0,(LONG_PTR)&PInfo);
#endif
  for(int i=0;i<PInfo.SelectedItemsNumber;i++)
  {
#ifndef UNICODE
    struct PluginPanelItem *RemovedItem=(struct PluginPanelItem *)
      FSF.bsearch(&PInfo.SelectedItems[i],TmpPanelItem,TmpItemsNumber,
        sizeof(struct PluginPanelItem),SortListCmp);
#else
    struct PluginPanelItem *RemovedItem=NULL;
    PluginPanelItem* PPI=(PluginPanelItem*)malloc(Info.Control(this,FCTL_GETSELECTEDPANELITEM,i,0));
    if(PPI)
    {
      Info.Control(this,FCTL_GETSELECTEDPANELITEM,i,(LONG_PTR)PPI);
      RemovedItem=(struct PluginPanelItem *)FSF.bsearch(PPI,TmpPanelItem,TmpItemsNumber,sizeof(struct PluginPanelItem),SortListCmp);
    }
#endif
    if(RemovedItem!=NULL)
      RemovedItem->Flags|=REMOVE_FLAG;
#ifdef UNICODE
    free(PPI);
#endif
  }
  RemoveEmptyItems();

#ifndef UNICODE
  Info.Control(this,FCTL_UPDATEPANEL,NULL);
  Info.Control(this,FCTL_REDRAWPANEL,NULL);
#else
  Info.Control(this,FCTL_UPDATEPANEL,0,NULL);
  Info.Control(this,FCTL_REDRAWPANEL,0,NULL);
#endif

#ifndef UNICODE
  Info.Control(this,FCTL_GETANOTHERPANELSHORTINFO,(void *)&PInfo);
#else
  Info.Control(PANEL_PASSIVE,FCTL_GETPANELINFO,0,(LONG_PTR)&PInfo);
#endif
  if(PInfo.PanelType==PTYPE_QVIEWPANEL)
  {
#ifndef UNICODE
    Info.Control(this,FCTL_UPDATEANOTHERPANEL,NULL);
    Info.Control(this,FCTL_REDRAWANOTHERPANEL,NULL);
#else
    Info.Control(PANEL_PASSIVE,FCTL_UPDATEPANEL,0,NULL);
    Info.Control(PANEL_PASSIVE,FCTL_REDRAWPANEL,0,NULL);
#endif
  }
}

void TmpPanel::ProcessSaveListKey()
{
  IfOptCommonPanel();
  if (TmpItemsNumber == 0)
    return;

  // default path: opposite panel directory\panel<index>.<mask extension>
  TCHAR ListPath [NM];
#ifndef UNICODE
  PanelInfo PInfo;
  Info.Control (this, FCTL_GETANOTHERPANELINFO, &PInfo);
  lstrcpy (ListPath, PInfo.CurDir);
#else
  Info.Control (PANEL_PASSIVE,FCTL_GETCURRENTDIRECTORY,NM,(LONG_PTR)ListPath);
#endif

  FSF.AddEndSlash (ListPath);
  lstrcat (ListPath, _T("panel"));
  if (Opt.CommonPanel)
    FSF.itoa (PanelIndex, ListPath + lstrlen (ListPath), 10);

  TCHAR ExtBuf [NM];
  lstrcpy (ExtBuf, Opt.Mask);
  TCHAR *comma = _tcschr (ExtBuf, _T(','));
  if (comma)
    *comma = _T('\0');
  TCHAR *ext = _tcschr (ExtBuf, _T('.'));
  if (ext && !_tcschr (ext, _T('*')) && !_tcschr (ext, _T('?')))
    lstrcat (ListPath, ext);

  if (Info.InputBox (GetMsg (MTempPanel), GetMsg (MListFilePath),
      _T("TmpPanel.SaveList"), ListPath, ListPath, ArraySize(ListPath)-1,
      NULL, FIB_BUTTONS))
  {
    SaveListFile (ListPath);
#ifndef UNICODE
    Info.Control (this, FCTL_UPDATEANOTHERPANEL, NULL);
    Info.Control (this, FCTL_REDRAWANOTHERPANEL, NULL);
#else
    Info.Control (PANEL_PASSIVE, FCTL_UPDATEPANEL,0,NULL);
    Info.Control (PANEL_PASSIVE, FCTL_REDRAWPANEL,0,NULL);
#endif

  }
#undef _HANDLE
#undef _UPDATE
#undef _REDRAW
#undef _GET
}

void TmpPanel::SaveListFile (const TCHAR *Path)
{
  IfOptCommonPanel();

  if (!TmpItemsNumber)
    return;

  HANDLE hFile = CreateFile (Path, GENERIC_WRITE, 0, NULL,
    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    const TCHAR *Items[] = { GetMsg (MError) };
    Info.Message (Info.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK,
      NULL, Items, 1, 0);
    return;
  }

  DWORD BytesWritten;
#ifdef UNICODE
  static const unsigned short bom = BOM_UCS2;
  WriteFile (hFile, &bom, sizeof(bom), &BytesWritten, NULL);
#endif
  int i = 0;
  do {
    static const TCHAR *CRLF = _T("\r\n");
    const TCHAR *FName = TmpPanelItem [i].FindData.cFileName;
    WriteFile (hFile, FName, sizeof(TCHAR)*lstrlen(FName), &BytesWritten, NULL);
    WriteFile (hFile, CRLF, 2*sizeof(TCHAR), &BytesWritten, NULL);
  }while(++i < TmpItemsNumber);
  CloseHandle (hFile);
}

void TmpPanel::SwitchToPanel (int NewPanelIndex)
{
  if((unsigned)NewPanelIndex<COMMONPANELSNUMBER && NewPanelIndex!=(int)PanelIndex)
  {
    CommonPanels[PanelIndex].Items=TmpPanelItem;
    CommonPanels[PanelIndex].ItemsNumber=TmpItemsNumber;
    if(!CommonPanels[NewPanelIndex].Items)
    {
      CommonPanels[NewPanelIndex].ItemsNumber=0;
      CommonPanels[NewPanelIndex].Items=(PluginPanelItem*)calloc(1,sizeof(PluginPanelItem));
    }
    if(CommonPanels[NewPanelIndex].Items)
    {
      CurrentCommonPanel = PanelIndex = NewPanelIndex;
#ifndef UNICODE
      Info.Control(this,FCTL_UPDATEPANEL,NULL);
      Info.Control(this,FCTL_REDRAWPANEL,NULL);
#else
      Info.Control(this,FCTL_UPDATEPANEL,0,NULL);
      Info.Control(this,FCTL_REDRAWPANEL,0,NULL);
#endif
    }
  }
}


void TmpPanel::ProcessPanelSwitchMenu()
{
  FarMenuItem fmi[COMMONPANELSNUMBER];
  memset(&fmi,0,sizeof(FarMenuItem)*COMMONPANELSNUMBER);
  const TCHAR *txt=GetMsg(MSwitchMenuTxt);
#ifdef UNICODE
  wchar_t tmpstr[COMMONPANELSNUMBER][128];
#endif
  static const TCHAR fmt1[]=_T("&%c. %s %d");
  for(unsigned int i=0;i<COMMONPANELSNUMBER;++i)
  {
#ifndef UNICODE
#define _OUT  fmi[i].Text
#else
#define _OUT  tmpstr[i]
    fmi[i].Text = tmpstr[i];
#endif
    if(i<10)
      FSF.sprintf(_OUT,fmt1,_T('0')+i,txt,CommonPanels[i].ItemsNumber);
    else if(i<36)
      FSF.sprintf(_OUT,fmt1,_T('A')-10+i,txt,CommonPanels[i].ItemsNumber);
    else
      FSF.sprintf(_OUT,_T("   %s %d"),txt,CommonPanels[i].ItemsNumber);
#undef _OUT
  }
  fmi[PanelIndex].Selected=TRUE;
  int ExitCode=Info.Menu(Info.ModuleNumber,-1,-1,0,
    FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
    GetMsg(MSwitchMenuTitle),NULL,NULL,
    NULL,NULL,fmi,COMMONPANELSNUMBER);
  SwitchToPanel (ExitCode);
}

int TmpPanel::IsOwnersDisplayed(LPCTSTR ColumnTypes)
{
  for(int i=0;ColumnTypes[i];i++)
    if(ColumnTypes[i]==_T('O') && (i==0 || ColumnTypes[i-1]==_T(',')) &&
       (ColumnTypes[i+1]==_T(',') || ColumnTypes[i+1]==0))
      return(TRUE);
    return(FALSE);
}


int TmpPanel::IsLinksDisplayed(LPCTSTR ColumnTypes)
{
  for(int i=0;ColumnTypes[i];i++)
    if(ColumnTypes[i]==_T('L') && ColumnTypes[i+1]==_T('N') &&
       (i==0 || ColumnTypes[i-1]==_T(',')) &&
      (ColumnTypes[i+2]==_T(',') || ColumnTypes[i+2]==0))
      return(TRUE);
    return(FALSE);
}

inline bool isDevice(const TCHAR* FileName, const TCHAR* dev_begin)
{
    const int len=(int)lstrlen(dev_begin);
    if(FSF.LStrnicmp(FileName, dev_begin, len)) return false;
    FileName+=len;
    if(!*FileName) return false;
    while(*FileName>=_T('0') && *FileName<=_T('9')) FileName++;
    return !*FileName;
}

int TmpPanel::CheckForCorrect(const TCHAR *Dir,FAR_FIND_DATA *FindData,int Any)
{
  TCHAR TempDir[MAX_PATH], SavedDir[MAX_PATH];
  ExpandEnvStrs(Dir,TempDir,ArraySize(TempDir));
  WIN32_FIND_DATA wfd;

  TCHAR *p=TempDir;
  ParseParam(p);
  lstrcpy(SavedDir, p);

  if (!FSF.LStrnicmp(p, _T("\\\\.\\"), 4) && FSF.LIsAlpha(p[4]) && p[5]==_T(':') && p[6]==0)
  {
copy_name_set_attr:
    FindData->dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
copy_name:
#ifndef UNICODE
    lstrcpy(FindData->cFileName,p);
#else
    if(FindData->lpwszFileName)
      free(FindData->lpwszFileName);
    FindData->lpwszFileName = wcsdup(p);
#endif
    return(TRUE);
  }

  if (isDevice(p, _T("\\\\.\\PhysicalDrive")) || isDevice(p, _T("\\\\.\\cdrom")))
    goto copy_name_set_attr;

  if(lstrlen(p) && lstrcmp(p,_T("\\"))!=0 && lstrcmp(p,_T(".."))!=0)
  {
    HANDLE fff=FindFirstFile(p,&wfd);

    if(fff != INVALID_HANDLE_VALUE)
    {
      WFD2FFD(wfd,*FindData);
      FindClose(fff);
      goto copy_name;
    }
    else
    {
      TCHAR *t = p + lstrlen(p) - 1;
      if (*t == _T('\\')) *t = 0;
      TCHAR TMP[MAX_PATH];
      fff=FindFirstFile(lstrcat(lstrcpy(TMP, p),_T("\\*.*")),&wfd);
      if(fff != INVALID_HANDLE_VALUE)
      {
        WFD2FFD(wfd,*FindData);
        FindClose(fff);
        FindData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        goto copy_name;
      }
    }
    if(Any)
      goto copy_name_set_attr;
  }
  return(FALSE);
}


void TmpPanel::IfOptCommonPanel(void)
{
  if(StartupOptCommonPanel)
  {
    TmpPanelItem=CommonPanels[PanelIndex].Items;
    TmpItemsNumber=CommonPanels[PanelIndex].ItemsNumber;
  }
}
