static int _cdecl SortListCmp(const void *el1,const void *el2);

TmpPanel::TmpPanel()
{
  if (Opt.CommonPanel)
  {
    TmpPanelItem=CommonTmpPanelItem;
    TmpItemsNumber=CommonTmpItemsNumber;
  }
  else
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
  if (!Opt.CommonPanel)
  {
    for (int I=0;I<TmpItemsNumber;I++)
      if (TmpPanelItem[I].Owner)
        delete TmpPanelItem[I].Owner;
    delete TmpPanelItem;
  }
}


int TmpPanel::GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  if (Opt.CommonPanel)
  {
    TmpPanelItem=CommonTmpPanelItem;
    TmpItemsNumber=CommonTmpItemsNumber;
  }
  UpdateItems(IsOwnersDisplayed(),IsLinksDisplayed());
  *pPanelItem=TmpPanelItem;
  *pItemsNumber=TmpItemsNumber;
  return(TRUE);
}


void TmpPanel::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=OPIF_USEFILTER|OPIF_USESORTGROUPS|OPIF_USEHIGHLIGHTING|
              OPIF_ADDDOTS|OPIF_SHOWNAMESONLY|OPIF_SHOWRIGHTALIGNNAMES|
              OPIF_REALNAMES;
  Info->HostFile=NULL;
  Info->CurDir="";

  Info->Format=GetMsg(MTempPanel);

  static char Title[100];
  sprintf(Title," %s ",GetMsg(MTempPanel));
  Info->PanelTitle=Title;

  Info->InfoLines=NULL;
  Info->InfoLinesNumber=0;

  Info->DescrFiles=NULL;
  Info->DescrFilesNumber=0;

  static struct PanelMode PanelModesArray[10];
  PanelModesArray[4].ColumnTypes="NR,S";
  PanelModesArray[4].ColumnWidths="0,8";
  PanelModesArray[4].FullScreen=FALSE;

  Info->PanelModesArray=PanelModesArray;
  Info->PanelModesNumber=sizeof(PanelModesArray)/sizeof(PanelModesArray[0]);
  Info->StartPanelMode='4';
  static struct KeyBarTitles KeyBar;
  memset(&KeyBar,0,sizeof(KeyBar));
  KeyBar.Titles[7-1]=GetMsg(MF7);
  Info->KeyBar=&KeyBar;
}


int TmpPanel::SetDirectory(char *Dir,int OpMode)
{
  if ((OpMode & OPM_FIND) || strcmp(Dir,"\\")==0)
    return(FALSE);
  Info.Control(this,FCTL_CLOSEPLUGIN,Dir);
  return(TRUE);
}


int TmpPanel::PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                       int Move,int OpMode)
{
  UpdateNotNeeded=FALSE;
  if (Opt.CommonPanel)
  {
    TmpPanelItem=CommonTmpPanelItem;
    TmpItemsNumber=CommonTmpItemsNumber;
  }
  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
  char *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempSendFiles)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
  char CurDir[NM];
  GetCurrentDirectory(sizeof(CurDir),CurDir);
  AddEndSlash(CurDir);
  for (int I=0;I<ItemsNumber;I++)
  {
    struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+1));
    if (NewPanelItem==NULL)
      return(0);
    TmpPanelItem=NewPanelItem;
    struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber++];
    *CurPanelItem=PanelItem[I];
    CurPanelItem->Description=NULL;

    char *CurName=PanelItem[I].FindData.cFileName;
    int NameOnly=(PointToName(CurName)==CurName);
    if (NameOnly)
      strcpy(CurPanelItem->FindData.cFileName,CurDir);
    else
      *CurPanelItem->FindData.cFileName=0;
    strcat(CurPanelItem->FindData.cFileName,PanelItem[I].FindData.cFileName);
    if (NameOnly && (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      struct PluginPanelItem *DirPanelItem;
      int DirItemsNumber;
      if (!Info.GetDirList(CurPanelItem->FindData.cFileName,&DirPanelItem,&DirItemsNumber))
      {
        delete TmpPanelItem;
        TmpPanelItem=NULL;
        TmpItemsNumber=0;
        return(0);
      }
      struct PluginPanelItem *NewPanelItem=(struct PluginPanelItem *)realloc(TmpPanelItem,sizeof(*TmpPanelItem)*(TmpItemsNumber+DirItemsNumber));
      if (NewPanelItem==NULL)
        return(0);
      TmpPanelItem=NewPanelItem;
      for (int I=0;I<DirItemsNumber;I++)
      {
        struct PluginPanelItem *CurPanelItem=&TmpPanelItem[TmpItemsNumber++];
        *CurPanelItem=DirPanelItem[I];
        strcpy(CurPanelItem->FindData.cFileName,CurDir);
        strcat(CurPanelItem->FindData.cFileName,DirPanelItem[I].FindData.cFileName);
      }
      Info.FreeDirList(DirPanelItem);
    }
  }
  SortList();
  RemoveDups();
  Info.RestoreScreen(hScreen);
  return(1);
}


int TmpPanel::SetFindList(struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  if (Opt.CommonPanel)
  {
    TmpPanelItem=CommonTmpPanelItem;
    TmpItemsNumber=CommonTmpItemsNumber;
  }
  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
  char *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempSendFiles)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
  delete TmpPanelItem;
  TmpPanelItem=new PluginPanelItem[ItemsNumber];
  if (TmpPanelItem==NULL)
    TmpItemsNumber=0;
  else
  {
    TmpItemsNumber=ItemsNumber;
    memcpy(TmpPanelItem,PanelItem,ItemsNumber*sizeof(*TmpPanelItem));
  }
  SortList();
  RemoveDups();
  Info.RestoreScreen(hScreen);
  UpdateNotNeeded=TRUE;
  return(TRUE);
}


void TmpPanel::SortList()
{
  qsort(TmpPanelItem,TmpItemsNumber,sizeof(*TmpPanelItem),SortListCmp);
}


int _cdecl SortListCmp(const void *el1,const void *el2)
{
  struct PluginPanelItem *Item1,*Item2;
  Item1=(struct PluginPanelItem *)el1;
  Item2=(struct PluginPanelItem *)el2;
  return(strcmp(Item1->FindData.cFileName,Item2->FindData.cFileName));
}


void TmpPanel::RemoveDups()
{
  struct PluginPanelItem *CurItem=TmpPanelItem;
  for (int I=0;I<TmpItemsNumber-1;I++,CurItem++)
    if (strcmp(CurItem->FindData.cFileName,CurItem[1].FindData.cFileName)==0)
      CurItem->Flags|=REMOVE_FLAG;
  RemoveEmptyItems();
}


void TmpPanel::RemoveEmptyItems()
{
  int EmptyCount=0;
  struct PluginPanelItem *CurItem=TmpPanelItem;
  for (int I=0;I<TmpItemsNumber;I++,CurItem++)
    if (CurItem->Flags & REMOVE_FLAG)
    {
      if (CurItem->Owner)
        delete CurItem->Owner;
      EmptyCount++;
    }
    else
      if (EmptyCount>0)
        *(CurItem-EmptyCount)=*CurItem;
  TmpItemsNumber-=EmptyCount;
  if (Opt.CommonPanel)
  {
    CommonTmpPanelItem=TmpPanelItem;
    CommonTmpItemsNumber=TmpItemsNumber;
  }
}


void TmpPanel::UpdateItems(int ShowOwners,int ShowLinks)
{
  if (UpdateNotNeeded)
  {
    UpdateNotNeeded=FALSE;
    return;
  }
  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
  char *MsgItems[]={GetMsg(MTempPanel),GetMsg(MTempUpdate)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
  LastOwnersRead=ShowOwners;
  LastLinksRead=ShowLinks;
  struct PluginPanelItem *CurItem=TmpPanelItem;
  for (int I=0;I<TmpItemsNumber;I++,CurItem++)
  {
    HANDLE FindHandle;
    char FullName[NM];
    strcpy(FullName,CurItem->FindData.cFileName);
    char *Slash=strrchr(FullName,'\\');
    int Length=Slash ? Slash-FullName+1:0;
    int SameFolderItems=1;
    if (Length>0)
      for (int J=1;I+J<TmpItemsNumber;J++)
        if (memcmp(FullName,CurItem[J].FindData.cFileName,Length)==0 &&
            strchr(CurItem[J].FindData.cFileName+Length,'\\')==0)
          SameFolderItems++;
        else
          break;

    if (SameFolderItems>2)
    {
      WIN32_FIND_DATA FindData;
      strcpy(Slash+1,"*.*");
      for (int J=0;J<SameFolderItems;J++)
        CurItem[J].Flags|=REMOVE_FLAG;
      int Done=(FindHandle=FindFirstFile(FullName,&FindData))==INVALID_HANDLE_VALUE;
      while (!Done)
      {
        for (int J=0;J<SameFolderItems;J++)
          if ((CurItem[J].Flags & 1) && strcmp(FindData.cFileName,PointToName(CurItem[J].FindData.cFileName))==0)
          {
            CurItem[J].Flags&=~REMOVE_FLAG;
            strcpy(FullName,CurItem[J].FindData.cFileName);
            CurItem[J].FindData=FindData;
            strcpy(CurItem[J].FindData.cFileName,FullName);
            break;
          }
        Done=!FindNextFile(FindHandle,&FindData);
      }
      FindClose(FindHandle);
      I+=SameFolderItems-1;
      CurItem+=SameFolderItems-1;
    }
    else
    {
      if ((FindHandle=FindFirstFile(FullName,&CurItem->FindData))==INVALID_HANDLE_VALUE)
        CurItem->Flags|=REMOVE_FLAG;
      else
        strcpy(CurItem->FindData.cFileName,FullName);
      FindClose(FindHandle);
    }
  }

  RemoveEmptyItems();

  if (ShowOwners || ShowLinks)
  {
    struct PluginPanelItem *CurItem=TmpPanelItem;
    for (int I=0;I<TmpItemsNumber;I++,CurItem++)
    {
      if (ShowOwners)
      {
        char Owner[80];
        if (CurItem->Owner)
        {
          delete CurItem->Owner;
          CurItem->Owner=NULL;
        }
        if (GetFileOwner(CurItem->FindData.cFileName,Owner))
        {
          CurItem->Owner=new char[strlen(Owner)+1];
          strcpy(CurItem->Owner,Owner);
        }
      }
      if (ShowLinks)
        CurItem->NumberOfLinks=GetNumberOfLinks(CurItem->FindData.cFileName);
    }
  }
  Info.RestoreScreen(hScreen);
}


int TmpPanel::ProcessEvent(int Event,void *Param)
{
  if (Event==FE_CHANGEVIEWMODE)
  {
    if (Opt.CommonPanel)
    {
      TmpPanelItem=CommonTmpPanelItem;
      TmpItemsNumber=CommonTmpItemsNumber;
    }
    int UpdateOwners=IsOwnersDisplayed() && !LastOwnersRead;
    int UpdateLinks=IsLinksDisplayed() && !LastLinksRead;
    if (UpdateOwners || UpdateLinks)
    {
      UpdateItems(UpdateOwners,UpdateLinks);
      Info.Control(this,FCTL_UPDATEPANEL,(void *)TRUE);
      Info.Control(this,FCTL_REDRAWPANEL,NULL);
    }
  }
  return(FALSE);
}


int TmpPanel::ProcessKey(int Key,unsigned int ControlState)
{
  if (ControlState==0 && Key==VK_F7)
  {
    if (Opt.CommonPanel)
    {
      TmpPanelItem=CommonTmpPanelItem;
      TmpItemsNumber=CommonTmpItemsNumber;
    }
    struct PanelInfo PInfo;
    Info.Control(this,FCTL_GETPANELINFO,&PInfo);

    for (int I=0;I<PInfo.SelectedItemsNumber;I++)
    {
      struct PluginPanelItem *RemovedItem=(struct PluginPanelItem *)
             bsearch(&PInfo.SelectedItems[I],TmpPanelItem,TmpItemsNumber,
                     sizeof(struct PluginPanelItem),SortListCmp);
      if (RemovedItem!=NULL)
        RemovedItem->Flags|=REMOVE_FLAG;
    }
    RemoveEmptyItems();
    Info.Control(this,FCTL_UPDATEPANEL,NULL);
    Info.Control(this,FCTL_REDRAWPANEL,NULL);

    Info.Control(this,FCTL_GETANOTHERPANELINFO,&PInfo);
    if (PInfo.PanelType==PTYPE_QVIEWPANEL)
    {
      Info.Control(this,FCTL_UPDATEANOTHERPANEL,NULL);
      Info.Control(this,FCTL_REDRAWANOTHERPANEL,NULL);
    }
    return(TRUE);
  }
  return(FALSE);
}


int TmpPanel::IsOwnersDisplayed()
{
  struct PanelInfo PInfo;
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
  for (int I=0;PInfo.ColumnTypes[I];I++)
    if (PInfo.ColumnTypes[I]=='O' && (I==0 || PInfo.ColumnTypes[I-1]==',') &&
        (PInfo.ColumnTypes[I+1]==',' || PInfo.ColumnTypes[I+1]==0))
      return(TRUE);
  return(FALSE);
}


int TmpPanel::IsLinksDisplayed()
{
  struct PanelInfo PInfo;
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
  for (int I=0;PInfo.ColumnTypes[I];I++)
    if (PInfo.ColumnTypes[I]=='L' && PInfo.ColumnTypes[I+1]=='N' &&
        (I==0 || PInfo.ColumnTypes[I-1]==',') &&
        (PInfo.ColumnTypes[I+2]==',' || PInfo.ColumnTypes[I+2]==0))
      return(TRUE);
  return(FALSE);
}

