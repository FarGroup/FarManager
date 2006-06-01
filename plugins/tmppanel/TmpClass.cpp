/*
TMPCLASS.CPP

Temporary panel plugin class implementation

*/

void WFD2FFD(WIN32_FIND_DATA &wfd, FAR_FIND_DATA &ffd)
{
  ffd.dwFileAttributes=wfd.dwFileAttributes;
  ffd.ftCreationTime=wfd.ftCreationTime;
  ffd.ftLastAccessTime=wfd.ftLastAccessTime;
  ffd.ftLastWriteTime=wfd.ftLastWriteTime;
  ffd.nFileSizeHigh=wfd.nFileSizeHigh;
  ffd.nFileSizeLow=wfd.nFileSizeLow;
  ffd.dwReserved0=wfd.dwReserved0;
  ffd.dwReserved1=wfd.dwReserved1;
  lstrcpy(ffd.cFileName,wfd.cFileName);
  lstrcpy(ffd.cAlternateFileName,wfd.cAlternateFileName);
}

static int _cdecl SortListCmp(const void *el1,const void *el2);

TmpPanel::TmpPanel()
{
  PanelIndex=CurrentCommonPanel;
  IfOptCommonPanel();
  if(!StartupOptCommonPanel)
  {
    TmpPanelItem=NULL;
    TmpItemsNumber=0;
  }

  LastOwnersRead=FALSE;
  LastLinksRead=FALSE;
  UpdateNotNeeded=FALSE;
}


TmpPanel::~TmpPanel()
{
  if(!StartupOptCommonPanel)
    FreePanelItems(TmpPanelItem, TmpItemsNumber);
}

int TmpPanel::GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  IfOptCommonPanel();
  static struct PanelInfo PInfo;
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
  UpdateItems(IsOwnersDisplayed (PInfo),IsLinksDisplayed (PInfo));
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
  Info->CurDir="";

  Info->Format=(char*)GetMsg(MTempPanel);

  static char Title[MAX_PATH];
  static char PanelMode[16];
  if(Opt.SafeModePanel)
    lstrcpy(PanelMode,"(R) ");
  else
    lstrcpy(PanelMode,"");
  if(StartupOptCommonPanel)
    FSF.sprintf(Title,GetMsg(MTempPanelTitleNum),PanelMode,PanelIndex);
  else
    FSF.sprintf(Title," %s%s ",PanelMode,GetMsg(MTempPanel));

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
  Info->PanelModesNumber=sizeof(PanelModesArray)/sizeof(PanelModesArray[0]);
  Info->StartPanelMode='4';
  static struct KeyBarTitles KeyBar;
  memset(&KeyBar,0,sizeof(KeyBar));
  KeyBar.Titles[7-1]=(char*)GetMsg(MF7);
  if(StartupOptCommonPanel)
    KeyBar.AltShiftTitles[12-1]=(char*)GetMsg(MAltShiftF12);
  KeyBar.AltShiftTitles[2-1]=(char*)GetMsg(MAltShiftF2);
  KeyBar.AltShiftTitles[3-1]=(char*)GetMsg(MAltShiftF3);
  Info->KeyBar=&KeyBar;
}


int TmpPanel::SetDirectory(const char *Dir,int OpMode)
{
  if((OpMode & OPM_FIND)/* || lstrcmp(Dir,"\\")==0*/)
    return(FALSE);
  if(lstrcmp(Dir,"\\")==0)
    Info.Control(this,FCTL_CLOSEPLUGIN,(void*)NULL);
  else
    Info.Control(this,FCTL_CLOSEPLUGIN,(void*)Dir);
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
  const char *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempSendFiles)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
  return hScreen;
}

int TmpPanel::PutOneFile (PluginPanelItem &PanelItem)
{
  static char CurDir[NM];
  GetCurrentDirectory(sizeof(CurDir),CurDir);
  FSF.AddEndSlash(CurDir);

  struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+1));
  if(NewPanelItem==NULL)
    return FALSE;
  TmpPanelItem=NewPanelItem;
  struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber++];
  *CurPanelItem=PanelItem;
  CurPanelItem->Flags&=~PPIF_SELECTED;
  CurPanelItem->Description=NULL;

  char *CurName=PanelItem.FindData.cFileName;
  if(CheckForCorrect(CurName,&CurPanelItem->FindData,Opt.AnyInPanel))
  {
    int NameOnly=(FSF.PointToName(CurName)==CurName);
    if(NameOnly)
      lstrcpy(CurPanelItem->FindData.cFileName,CurDir);
    else
      *CurPanelItem->FindData.cFileName=0;
    lstrcat(CurPanelItem->FindData.cFileName,PanelItem.FindData.cFileName);
    if(Opt.SelectedCopyContents && NameOnly && (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      if (Opt.SelectedCopyContents==2)
      {
        const char *MsgItems[]={GetMsg(MWarning),GetMsg(MCopyContensMsg)};
        Opt.SelectedCopyContents=!Info.Message(Info.ModuleNumber,FMSG_MB_YESNO,"Config",MsgItems,
          sizeof(MsgItems)/sizeof(MsgItems[0]),0);
      }
      if (Opt.SelectedCopyContents)
      {
        struct PluginPanelItem *DirPanelItem;
        int DirItemsNumber;
        if(!Info.GetDirList(CurPanelItem->FindData.cFileName,
                            &DirPanelItem,
                            &DirItemsNumber))
        {
          if(TmpPanelItem)
            free(TmpPanelItem);
          TmpPanelItem=NULL;
          TmpItemsNumber=0;
          return FALSE;
        }
        struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+DirItemsNumber));
        if(NewPanelItem==NULL)
          return FALSE;
        TmpPanelItem=NewPanelItem;
        for(int i=0;i<DirItemsNumber;i++)
        {
          struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber++];
          *CurPanelItem=DirPanelItem[i];
          CurPanelItem->Flags&=~PPIF_SELECTED;
          lstrcpy(CurPanelItem->FindData.cFileName,CurDir);
          lstrcat(CurPanelItem->FindData.cFileName,DirPanelItem[i].FindData.cFileName);
        }
        Info.FreeDirList(DirPanelItem);
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
  if(TmpPanelItem) free(TmpPanelItem);
  TmpPanelItem=(PluginPanelItem*) malloc(sizeof(PluginPanelItem)*ItemsNumber+1);
  if(TmpPanelItem==NULL)
    TmpItemsNumber=0;
  else
  {
    TmpItemsNumber=ItemsNumber;
    memcpy(TmpPanelItem,PanelItem,ItemsNumber*sizeof(*TmpPanelItem));
    for(int i=0;i<ItemsNumber;++i)
      TmpPanelItem[i].Flags&=~PPIF_SELECTED;
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
      PanelIndex = SearchResultsPanel;
      CurrentCommonPanel=PanelIndex;
      TmpPanelItem = CommonPanels[PanelIndex].Items;
      TmpItemsNumber = CommonPanels[PanelIndex].ItemsNumber;
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
      if(CurItem->Owner)
        free (CurItem->Owner);
      EmptyCount++;
    }
    else if(EmptyCount>0)
      *(CurItem-EmptyCount)=*CurItem;
  TmpItemsNumber-=EmptyCount;
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
  const char *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempUpdate)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
  LastOwnersRead=ShowOwners;
  LastLinksRead=ShowLinks;
  struct PluginPanelItem *CurItem=TmpPanelItem;
  for(int i=0;i<TmpItemsNumber;i++,CurItem++)
  {
    HANDLE FindHandle;
    static char FullName[NM];
    lstrcpy(FullName,CurItem->FindData.cFileName);

    char *Slash=strrchr(/*(const char*)*/FullName,(int)'\\');
    int Length=Slash ? Slash-FullName+1:0;

    int SameFolderItems=1;
    /* $ 23.12.2001 DJ
       если FullName - это каталог, то FindFirstFile (FullName+"*.*")
       этот каталог не найдет. Поэтому для каталогов оптимизацию с
       SameFolderItems пропускаем.
    */
    if(Length>0 && Length > (int)lstrlen (FullName))   /* DJ $ */
      for(int j=1;i+j<TmpItemsNumber;j++)
        if(memcmp(FullName,CurItem[j].FindData.cFileName,Length)==0 &&
          strchr((const char*)CurItem[j].FindData.cFileName+Length,(int)'\\')==0)
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
      lstrcpy(Slash+1,"*.*");
      for(int J=0;J<SameFolderItems;J++)
        CurItem[J].Flags|=REMOVE_FLAG;
      int Done=(FindHandle=FindFirstFile(FullName,&FindData))==INVALID_HANDLE_VALUE;
      while(!Done)
      {
        for(int J=0;J<SameFolderItems;J++)
          if((CurItem[J].Flags & 1) && lstrcmp(FindData.cFileName,FSF.PointToName(CurItem[J].FindData.cFileName))==0)
          {
            CurItem[J].Flags&=~REMOVE_FLAG;
            lstrcpy(FullName,CurItem[J].FindData.cFileName);
            WFD2FFD(FindData,CurItem[J].FindData);
            lstrcpy(CurItem[J].FindData.cFileName,FullName);
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
        static char Owner[80];
        if(CurItem->Owner)
        {
          free (CurItem->Owner);
          CurItem->Owner=NULL;
        }
        if(FSF.GetFileOwner(NULL,CurItem->FindData.cFileName,Owner))
        {
          CurItem->Owner=(char *) malloc(lstrlen(Owner)+1);
          if(CurItem->Owner) lstrcpy(CurItem->Owner,Owner);
        }
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
    static struct PanelInfo PInfo;
    Info.Control(this,FCTL_GETPANELINFO,&PInfo);
    int UpdateOwners=IsOwnersDisplayed (PInfo) && !LastOwnersRead;
    int UpdateLinks=IsLinksDisplayed (PInfo) && !LastLinksRead;
    if(UpdateOwners || UpdateLinks)
    {
      UpdateItems(UpdateOwners,UpdateLinks);
      Info.Control(this,FCTL_UPDATEPANEL,(void *)TRUE);
      Info.Control(this,FCTL_REDRAWPANEL,NULL);
    }
  }
  return(FALSE);
}


int TmpPanel::IsCurrentFileCorrect (char *pCurFileName)
{
  static struct PanelInfo PInfo;
  static char CurFileName[NM];
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
  lstrcpy(CurFileName, PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName);

  BOOL IsCorrectFile = FALSE;
  if (lstrcmp (CurFileName, "..") == 0)
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
     return Configure(0);
  }

  if(ControlState==(PKF_SHIFT|PKF_ALT) && Key==VK_F3)
  {
    char CurFileName [NM];
    if (IsCurrentFileCorrect(CurFileName))
    {
      static struct PanelInfo PInfo;
      Info.Control(this,FCTL_GETPANELINFO,&PInfo);
      if (lstrcmp(CurFileName,"..")!=0)
      {
        if (PInfo.PanelItems[PInfo.CurrentItem].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
          Info.Control(INVALID_HANDLE_VALUE, FCTL_SETANOTHERPANELDIR,&CurFileName);
        else
          GoToFile(CurFileName, true);
        Info.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWANOTHERPANEL,NULL);
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
    char CurFileName [NM];
    if (!IsCurrentFileCorrect (CurFileName))
    {
      Info.Control(this,FCTL_SETCMDLINE,&CurFileName);
      return(TRUE);
    }
  }

  if (Opt.SafeModePanel && ControlState == PKF_CONTROL && Key == VK_PRIOR)
  {
    char CurFileName[NM]="";
    if(IsCurrentFileCorrect(CurFileName) && 0!=lstrcmp(CurFileName,".."))
    {
      GoToFile(CurFileName, false);
      return TRUE;
    }
    if(!lstrcmp(CurFileName,".."))
    {
      SetDirectory(".",0);
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
      else if (Key >= '0' && Key <= '9')
      {
        SwitchToPanel (Key - '0');
        return TRUE;
      }
    }
  }
  return(FALSE);
}


void TmpPanel::ProcessRemoveKey()
{
  IfOptCommonPanel();

  static struct PanelInfo PInfo;
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);

  for(int i=0;i<PInfo.SelectedItemsNumber;i++)
  {
    struct PluginPanelItem *RemovedItem=(struct PluginPanelItem *)
      FSF.bsearch(&PInfo.SelectedItems[i],TmpPanelItem,TmpItemsNumber,
        sizeof(struct PluginPanelItem),SortListCmp);
    if(RemovedItem!=NULL)
      RemovedItem->Flags|=REMOVE_FLAG;
  }
  RemoveEmptyItems();
  Info.Control(this,FCTL_UPDATEPANEL,NULL);
  Info.Control(this,FCTL_REDRAWPANEL,NULL);

  Info.Control(this,FCTL_GETANOTHERPANELINFO,(void *) &PInfo);
  if(PInfo.PanelType==PTYPE_QVIEWPANEL)
  {
    Info.Control(this,FCTL_UPDATEANOTHERPANEL,NULL);
    Info.Control(this,FCTL_REDRAWANOTHERPANEL,NULL);
  }
}

void TmpPanel::ProcessSaveListKey()
{
  IfOptCommonPanel();
  if (TmpItemsNumber == 0)
    return;

  PanelInfo PInfo;
  Info.Control (this, FCTL_GETANOTHERPANELINFO, &PInfo);

  // default path: opposite panel directory\panel<index>.<mask extension>
  static char ListPath [NM];
  lstrcpy (ListPath, PInfo.CurDir);
  FSF.AddEndSlash (ListPath);
  lstrcat (ListPath, "panel");
  if (Opt.CommonPanel)
    FSF.itoa (PanelIndex, ListPath + lstrlen (ListPath), 10);

  static char ExtBuf [NM];
  lstrcpy (ExtBuf, Opt.Mask);
  char *comma = strchr (ExtBuf, ',');
  if (comma)
    *comma = '\0';
  char *ext = strchr (ExtBuf, '.');
  if (ext && !strchr (ext, '*') && !strchr (ext, '?'))
    lstrcat (ListPath, ext);

  if (Info.InputBox (GetMsg (MTempPanel), GetMsg (MListFilePath),
      "TmpPanel.SaveList", ListPath, ListPath, sizeof (ListPath)-1,
      NULL, FIB_BUTTONS))
  {
    SaveListFile (ListPath);
    Info.Control (this, FCTL_UPDATEANOTHERPANEL, NULL);
    Info.Control (this, FCTL_REDRAWANOTHERPANEL, NULL);
  }
}

void TmpPanel::SaveListFile (const char *Path)
{
  HANDLE hFile = CreateFile (Path, GENERIC_WRITE, 0, NULL,
    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    const char *Items[] = { GetMsg (MError) };
    Info.Message (Info.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK,
      NULL, Items, 1, 0);
    return;
  }

  IfOptCommonPanel();

  for (int i=0; i<TmpItemsNumber; i++)
  {
    const char *CRLF = "\r\n";
    DWORD BytesWritten;
    const char *FName = TmpPanelItem [i].FindData.cFileName;
    WriteFile (hFile, FName, lstrlen (FName), &BytesWritten, NULL);
    WriteFile (hFile, CRLF, 2, &BytesWritten, NULL);
  }
  CloseHandle (hFile);
}

void TmpPanel::ProcessPanelSwitchMenu()
{
  FarMenuItem *fmi=(FarMenuItem*)malloc(COMMONPANELSNUMBER*sizeof(FarMenuItem));
  if(fmi)
  {
    const char *txt=GetMsg(MSwitchMenuTxt), fmt1[]="&%c. %s %d";
    for(unsigned int i=0;i<COMMONPANELSNUMBER;++i)
    {
      if(i<10)
        FSF.sprintf(fmi[i].Text,fmt1,'0'+i,txt,CommonPanels[i].ItemsNumber);
      else if(i<36)
        FSF.sprintf(fmi[i].Text,fmt1,'A'-10+i,txt,CommonPanels[i].ItemsNumber);
      else
        FSF.sprintf(fmi[i].Text,"   %s %d",txt,CommonPanels[i].ItemsNumber);
    }
    fmi[PanelIndex].Selected=TRUE;
    int ExitCode=Info.Menu(Info.ModuleNumber,-1,-1,0,
      FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
      GetMsg(MSwitchMenuTitle),NULL,NULL,
      NULL,NULL,fmi,COMMONPANELSNUMBER);
    free(fmi);
    SwitchToPanel (ExitCode);
  }
}


void TmpPanel::SwitchToPanel (int NewPanelIndex)
{
  if(NewPanelIndex>-1 && NewPanelIndex<COMMONPANELSNUMBER && NewPanelIndex!=(int)PanelIndex)
  {
    CommonPanels[PanelIndex].Items=TmpPanelItem;
    CommonPanels[PanelIndex].ItemsNumber=TmpItemsNumber;
    if(!CommonPanels[NewPanelIndex].Items)
    {
      CommonPanels[NewPanelIndex].ItemsNumber=0;
      CommonPanels[NewPanelIndex].Items=(PluginPanelItem*)malloc(sizeof(PluginPanelItem));
    }
    if(CommonPanels[NewPanelIndex].Items)
    {
      CurrentCommonPanel=NewPanelIndex;
      PanelIndex=NewPanelIndex;
      Info.Control(this,FCTL_UPDATEPANEL,NULL);
      Info.Control(this,FCTL_REDRAWPANEL,NULL);
    }
  }
}


int TmpPanel::IsOwnersDisplayed (const struct PanelInfo &PInfo)
{
  for(int i=0;PInfo.ColumnTypes[i];i++)
    if(PInfo.ColumnTypes[i]=='O' && (i==0 || PInfo.ColumnTypes[i-1]==',') &&
       (PInfo.ColumnTypes[i+1]==',' || PInfo.ColumnTypes[i+1]==0))
      return(TRUE);
    return(FALSE);
}


int TmpPanel::IsLinksDisplayed (const struct PanelInfo &PInfo)
{
  for(int i=0;PInfo.ColumnTypes[i];i++)
    if(PInfo.ColumnTypes[i]=='L' && PInfo.ColumnTypes[i+1]=='N' &&
       (i==0 || PInfo.ColumnTypes[i-1]==',') &&
      (PInfo.ColumnTypes[i+2]==',' || PInfo.ColumnTypes[i+2]==0))
      return(TRUE);
    return(FALSE);
}

int TmpPanel::CheckForCorrect(const char *Dir,FAR_FIND_DATA *FindData,int Any)
{
  static char TempDir[MAX_PATH], SavedDir[MAX_PATH];
  FSF.ExpandEnvironmentStr(Dir,TempDir,MAX_PATH);
  WIN32_FIND_DATA wfd;

  char *p=TempDir;
  ParseParam(p);
  OutputDebugString("CheckForCorrect. Any ==");
  OutputDebugString(Any?"true":"false");
  OutputDebugString(Dir);
  OutputDebugString(TempDir);
  OutputDebugString(p);
  lstrcpy(SavedDir, p);
  if(lstrlen(p) && lstrcmp(p,"\\")!=0 && lstrcmp(p,"..")!=0)
  {
    HANDLE fff=FindFirstFile(p,&wfd);
    WFD2FFD(wfd,*FindData);

    if(fff != INVALID_HANDLE_VALUE)
    {
      FindClose(fff);
      lstrcpy((*FindData).cFileName,p);
      return(TRUE);
    }
    else
    {
      char *t = p + lstrlen(p) - 1;
      if (*t == '\\') *t = 0;
      static char TMP[MAX_PATH];
      fff=FindFirstFile(lstrcat(lstrcpy(TMP, p),"\\*.*"),&wfd);
      WFD2FFD(wfd,*FindData);
      if(fff != INVALID_HANDLE_VALUE)
      {
        FindClose(fff);
        (*FindData).dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        lstrcpy((*FindData).cFileName,SavedDir);
        return(TRUE);
      }
    }
    if(Any)
    {
      (*FindData).dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
      lstrcpy((*FindData).cFileName,p);
      return(TRUE);
    }
  }
  OutputDebugString("Returns false");
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
