static NETRESOURCE CommonRootResources[16];
static int CommonRootCount;

NetBrowser::NetBrowser()
{
  NetRes=NULL;
  NetCount=0;
  ConnectedRes=NULL;
  ConnectedCount=0;
  FirstRead=TRUE;
  for (int I=0;I<CommonRootCount;I++)
  {
    NETRESOURCE *rr=RootResources+I;
    NETRESOURCE *crr=CommonRootResources+I;
    *rr=*crr;
    rr->lpRemoteName=CopyText(crr->lpRemoteName);
    rr->lpLocalName=CopyText(crr->lpLocalName);
    rr->lpComment=CopyText(crr->lpComment);
    rr->lpProvider=CopyText(crr->lpProvider);
  }
  RootCount=CommonRootCount;
}


NetBrowser::~NetBrowser()
{
  DeleteCommonRootResources();
  memcpy(CommonRootResources,RootResources,sizeof(CommonRootResources));
  CommonRootCount=RootCount;
  DeleteNetList(NetRes,NetCount);
  DeleteNetList(ConnectedRes,ConnectedCount);
}


int NetBrowser::GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  HANDLE hEnum,hConnectedEnum;
  *pPanelItem=NULL;
  *pItemsNumber=0;

  if (WNetOpenEnum(RESOURCE_CONNECTED,RESOURCETYPE_DISK,0,NULL,&hConnectedEnum)!=NO_ERROR)
  {
    char *MsgItems[]={GetMsg(MError),GetMsg(MNetCannotBrowse),GetMsg(MOk)};
    Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
    return(FALSE);
  }
  GetNetList(hConnectedEnum,ConnectedRes,ConnectedCount);
  WNetCloseEnum(hConnectedEnum);

  NETRESOURCE *CurResource=RootCount==0 ? NULL:&RootResources[RootCount-1];

  if (WNetOpenEnum(RESOURCE_GLOBALNET,RESOURCETYPE_ANY,0,CurResource,&hEnum)!=NO_ERROR)
    if (CurResource==NULL || !AddConnection(CurResource) ||
        WNetOpenEnum(RESOURCE_GLOBALNET,RESOURCETYPE_ANY,0,CurResource,&hEnum)!=NO_ERROR)
    {
      hEnum=NULL;
      if (CurResource==NULL)
      {
        char *MsgItems[]={GetMsg(MError),GetMsg(MNetCannotBrowse),GetMsg(MOk)};
        Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
        return(FALSE);
      }
      else
        NetCount=0;
    }

  if (hEnum!=NULL)
  {
    GetNetList(hEnum,NetRes,NetCount);
    WNetCloseEnum(hEnum);
  }

  if (NetCount==0 && CurResource!=NULL && AddConnection(CurResource))
    if (WNetOpenEnum(RESOURCE_GLOBALNET,RESOURCETYPE_ANY,0,CurResource,&hEnum)==NO_ERROR)
    {
      GetNetList(hEnum,NetRes,NetCount);
      WNetCloseEnum(hEnum);
    }

  PluginPanelItem *NewPanelItem=new PluginPanelItem[NetCount];
  *pPanelItem=NewPanelItem;
  if (NewPanelItem==NULL)
    return(FALSE);

  int CurItemPos=0;
  for (int I=0;I<NetCount;I++)
  {
    if (NetRes[I].dwType==RESOURCETYPE_PRINT)
      continue;

    char RemoteName[NM],LocalName[NM],Comment[300];

    GetRemoteName(&NetRes[I],RemoteName);
    if (NetRes[I].lpComment==NULL)
      *Comment=0;
    else
      CharToOem(NetRes[I].lpComment,Comment);
    memset(&NewPanelItem[CurItemPos],0,sizeof(PluginPanelItem));

    NewPanelItem[CurItemPos].CustomColumnData=new LPSTR[2];

    GetLocalName(RemoteName,LocalName);

    NewPanelItem[CurItemPos].CustomColumnData[0]=new char[strlen(LocalName)+1];
    strcpy(NewPanelItem[CurItemPos].CustomColumnData[0],LocalName);

    NewPanelItem[CurItemPos].CustomColumnData[1]=new char[strlen(Comment)+1];
    strcpy(NewPanelItem[CurItemPos].CustomColumnData[1],Comment);
    NewPanelItem[CurItemPos].CustomColumnNumber=2;

    CharToOem(RemoteName,NewPanelItem[CurItemPos].FindData.cFileName);
    NewPanelItem[CurItemPos].FindData.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;
    CurItemPos++;
  }
  *pItemsNumber=CurItemPos;
  WNetCloseEnum(hConnectedEnum);
  if (FirstRead && Opt.SkipTopLevel && RootCount==0 && *pItemsNumber==1)
  {
    char SecondLevelName[NM];
    strcpy(SecondLevelName,(*pPanelItem)[0].FindData.cFileName);
    FreeFindData(*pPanelItem,*pItemsNumber);
    SetDirectory(SecondLevelName,FALSE);
    *pPanelItem=NULL;
    *pItemsNumber=0;
    return(GetFindData(pPanelItem,pItemsNumber,OpMode));
  }
  FirstRead=FALSE;
  return(TRUE);
}


void NetBrowser::FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber)
{
  for (int I=0;I<ItemsNumber;I++)
  {
    delete PanelItem[I].CustomColumnData[0];
    delete PanelItem[I].CustomColumnData[1];
    delete PanelItem[I].CustomColumnData;
  }
  delete PanelItem;
}


int NetBrowser::DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                            int OpMode)
{
  for (int I=0;I<ItemsNumber;I++)
    if (PanelItem[I].CustomColumnNumber==2 && PanelItem[I].CustomColumnData!=NULL)
    {
      char *LocalName=PanelItem[I].CustomColumnData[0];
      if (*LocalName)
        if (WNetCancelConnection2(LocalName,0,FALSE)!=NO_ERROR)
        {
          int Failed=FALSE;
          char MsgText[200];
          sprintf(MsgText,GetMsg(MNetCannotDisconnect),LocalName);
          int LastError=GetLastError();
          if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
          {
            char *MsgItems[]={GetMsg(MError),MsgText,"\x1",GetMsg(MOpenFiles),GetMsg(MAskDisconnect),GetMsg(MOk),GetMsg(MCancel)};
            if (Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)==0)
              if (WNetCancelConnection2(LocalName,0,TRUE)!=NO_ERROR)
                Failed=TRUE;
          }
          else
            Failed=TRUE;
          if (Failed)
          {
            char *MsgItems[]={GetMsg(MError),MsgText,GetMsg(MOk)};
            Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
            break;
          }
        }
    }
  return(TRUE);
}


void NetBrowser::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=OPIF_USEHIGHLIGHTING|OPIF_ADDDOTS|OPIF_RAWSELECTION|
              OPIF_SHOWPRESERVECASE|OPIF_FINDFOLDERS;
  Info->HostFile=NULL;
  if (RootCount==0)
    Info->CurDir="";
  else
    if (RootResources[RootCount-1].lpRemoteName==NULL)
      Info->CurDir="network";
    else
    {
      static char CurDir[NM];
      CharToOem(RootResources[RootCount-1].lpRemoteName,CurDir);
      Info->CurDir=CurDir;
    }

  Info->Format=GetMsg(MNetwork);

  static char Title[100];
  sprintf(Title," %s ",GetMsg(MNetwork));
  Info->PanelTitle=Title;

  Info->InfoLines=NULL;
  Info->InfoLinesNumber=0;

  Info->DescrFiles=NULL;
  Info->DescrFilesNumber=0;

  static struct PanelMode PanelModesArray[10];
  static char *ColumnTitles[3];
  ColumnTitles[0]=GetMsg(MColumnName);
  ColumnTitles[1]=GetMsg(MColumnDisk);
  ColumnTitles[2]=GetMsg(MColumnComment);

  PanelModesArray[3].ColumnTypes="N,C0,C1";
  PanelModesArray[3].ColumnWidths="0,2,0";
  PanelModesArray[3].ColumnTitles=ColumnTitles;
  PanelModesArray[3].FullScreen=FALSE;
  PanelModesArray[4].ColumnTypes="N,C0";
  PanelModesArray[4].ColumnWidths="0,2";
  PanelModesArray[4].ColumnTitles=ColumnTitles;
  PanelModesArray[4].FullScreen=FALSE;
  PanelModesArray[5].ColumnTypes="N,C0,C1";
  PanelModesArray[5].ColumnWidths="0,2,0";
  PanelModesArray[5].ColumnTitles=ColumnTitles;
  PanelModesArray[5].FullScreen=TRUE;

  Info->PanelModesArray=PanelModesArray;
  Info->PanelModesNumber=sizeof(PanelModesArray)/sizeof(PanelModesArray[0]);
  Info->StartPanelMode='3';
  static struct KeyBarTitles KeyBar={
    NULL,NULL,"","",NULL,NULL,"",NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    NULL,NULL,"","","","",NULL,NULL,NULL,NULL,NULL,NULL,
    "","","","","","","","",NULL,NULL,NULL,NULL
  };
  KeyBar.Titles[5-1]=GetMsg(MF5);
  KeyBar.Titles[6-1]=GetMsg(MF6);
  KeyBar.Titles[8-1]=GetMsg(MF8);
  Info->KeyBar=&KeyBar;
}


int NetBrowser::SetDirectory(char *Dir,int OpMode)
{
  if (strcmp(Dir,"\\")==0)
  {
    for (int I=0;I<RootCount;I++)
      DeleteNetText(RootResources[I]);
    RootCount=0;
    return(TRUE);
  }
  if (strcmp(Dir,"..")==0)
  {
    if (RootCount>0)
    {
      RootCount--;
      DeleteNetText(RootResources[RootCount]);
      return(TRUE);
    }
    else
      return(FALSE);
  }
  else
  {
    if (!FirstRead)
    {
      PluginPanelItem *PanelItems;
      int ItemsNumber;
      GetFindData(&PanelItems,&ItemsNumber,OpMode);
      FreeFindData(PanelItems,ItemsNumber);
    }

    char AnsiDir[NM];
    OemToChar(Dir,AnsiDir);
    for (int I=0;I<NetCount;I++)
    {
      char RemoteName[NM];
      GetRemoteName(&NetRes[I],RemoteName);
      if (strcmp(AnsiDir,RemoteName)==0)
        if (RootCount<sizeof(RootResources)/sizeof(RootResources[0]))
        {
          if ((NetRes[I].dwUsage & RESOURCEUSAGE_CONTAINER)==0 &&
              (NetRes[I].dwType & RESOURCETYPE_DISK) &&
              NetRes[I].lpRemoteName!=NULL)
          {
            if (OpMode & OPM_FIND)
              return(FALSE);
            char NewDir[NM],LocalName[NM];
            GetLocalName(NetRes[I].lpRemoteName,LocalName);
            if (*LocalName)
              strcpy(NewDir,LocalName);
            else
            {
              strcpy(NewDir,NetRes[I].lpRemoteName);
              CharToOem(NewDir,NewDir);
              if (!IsReadable(NewDir))
                if (!AddConnection(&NetRes[I]) && GetLastError()==ERROR_CANCELLED)
                  break;
            }
            Info.Control(this,FCTL_CLOSEPLUGIN,NewDir);
            return(TRUE);
          }
          RootResources[RootCount]=NetRes[I];
          RootResources[RootCount].lpRemoteName=CopyText(NetRes[I].lpRemoteName);
          RootResources[RootCount].lpLocalName=CopyText(NetRes[I].lpLocalName);
          RootResources[RootCount].lpComment=CopyText(NetRes[I].lpComment);
          RootResources[RootCount].lpProvider=CopyText(NetRes[I].lpProvider);
          RootCount++;
          return(TRUE);
        }
    }
  }
  return(FALSE);
}


int NetBrowser::ProcessKey(int Key,unsigned int ControlState)
{
  if (ControlState==0 && (Key==VK_F5 || Key==VK_F6))
  {
    struct PanelInfo PInfo;
    Info.Control(this,FCTL_GETPANELINFO,&PInfo);

    for (int I=0;I<PInfo.SelectedItemsNumber;I++)
    {
      char *RemoteName=PInfo.SelectedItems[I].FindData.cFileName;
//      char *LocalName=PInfo.SelectedItems[I].CustomColumnData[0];
      char AnsiRemoteName[NM];
      OemToChar(RemoteName,AnsiRemoteName);
      DWORD DriveMask=GetLogicalDrives();
      char NewLocalName[10];
      *NewLocalName=0;

      if (Key==VK_F5)
        GetFreeLetter(DriveMask,NewLocalName);
      else
      {
        struct FarMenuItem MenuItems['Z'-'A'+1];
        int MenuItemsNumber=0;
        memset(MenuItems,0,sizeof(MenuItems));
        DWORD DriveMask=GetLogicalDrives();
        for (int I=0;I<='Z'-'A';I++)
          if ((DriveMask & (1<<I))==0)
            sprintf(MenuItems[MenuItemsNumber++].Text,"&%c:",'A'+I);
        if (MenuItemsNumber>0)
        {
          int ExitCode=Info.Menu(Info.ModuleNumber,-1,-1,0,0,
                       GetMsg(MDisksListTitle),NULL,"NetBrowse",
                       NULL,NULL,MenuItems,MenuItemsNumber);
          if (ExitCode<0)
            break;
          strcpy(NewLocalName,MenuItems[ExitCode].Text+1);
        }
      }
      if (*NewLocalName)
      {
        NETRESOURCE newnr;
        char LocalName[10];
        newnr.dwType=RESOURCETYPE_DISK;
        newnr.lpLocalName=NewLocalName;
        newnr.lpRemoteName=AnsiRemoteName;
        newnr.lpProvider=NULL;

        while (1)
        {
          if (AddConnection(&newnr))
            break;

          if (GetLastError()==ERROR_DEVICE_ALREADY_REMEMBERED)
          {
            if (Key==VK_F5)
            {
              GetFreeLetter(DriveMask,NewLocalName);
              if (*NewLocalName==0)
              {
                char *MsgItems[]={GetMsg(MError),GetMsg(MNoFreeLetters),GetMsg(MOk)};
                Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
                PInfo.SelectedItemsNumber=0;
                break;
              }
            }
            else
            {
              char *MsgItems[]={GetMsg(MError),GetMsg(MAlreadyRemembered),GetMsg(MOk)};
              Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
              PInfo.SelectedItemsNumber=0;
              break;
            }
          }
          else
          {
            char MsgText[300];
            sprintf(MsgText,GetMsg(MNetCannotConnect),RemoteName,NewLocalName);
            char *MsgItems[]={GetMsg(MError),MsgText,GetMsg(MOk)};
            Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
            PInfo.SelectedItemsNumber=0;
            break;
          }
        }
      }
      else
      {
        char *MsgItems[]={GetMsg(MError),GetMsg(MNoFreeLetters),GetMsg(MOk)};
        Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
        break;
      }
    }
    Info.Control(this,FCTL_UPDATEPANEL,NULL);
    Info.Control(this,FCTL_REDRAWPANEL,NULL);
    return(TRUE);
  }
  return(FALSE);
}


void NetBrowser::GetFreeLetter(DWORD &DriveMask,char *DiskName)
{
  *DiskName=0;
  for (int I=2;I<='Z'-'A';I++)
    if ((DriveMask & (1<<I))==0)
    {
      DriveMask |= 1<<I;
      sprintf(DiskName,"%c:",'A'+I);
      break;
    }
}


int NetBrowser::AddConnection(NETRESOURCE *nr)
{
  char Name[256],Password[256];
  NETRESOURCE connectnr=*nr;
  if (WNetAddConnection2(&connectnr,NULL,NULL,0)==NO_ERROR)
    return(TRUE);
  int LastError=GetLastError();
  if (LastError!=ERROR_INVALID_PASSWORD && LastError!=ERROR_LOGON_FAILURE)
    return(FALSE);
  if (!GetNameAndPassword(connectnr.lpRemoteName,Name,Password))
  {
    SetLastError(ERROR_CANCELLED);
    return(FALSE);
  }
  return(WNetAddConnection2(&connectnr,Password,*Name ? Name:NULL,0)==NO_ERROR);
}


void NetBrowser::GetLocalName(char *RemoteName,char *LocalName)
{
  *LocalName=0;
  if (RemoteName!=NULL && *RemoteName)
    for (int I=ConnectedCount-1;I>=0;I--)
      if (ConnectedRes[I].lpRemoteName && ConnectedRes[I].lpLocalName!=NULL &&
          *ConnectedRes[I].lpLocalName &&
          lstrcmpi(ConnectedRes[I].lpRemoteName,RemoteName)==0)
        {
          if (ConnectedRes[I].dwScope==RESOURCE_CONNECTED ||
              ConnectedRes[I].dwScope==RESOURCE_REMEMBERED)
            CharToOem(ConnectedRes[I].lpLocalName,LocalName);
          break;
        }
}


int NetBrowser::GetNameAndPassword(char *Title,char *Name,char *Password)
{
  struct InitDialogItem InitItems[]={
    DI_DOUBLEBOX,3,1,72,8,0,0,0,0,"",
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MNetUserName,
    DI_EDIT,5,3,70,3,1,0,0,0,"",
    DI_TEXT,5,4,0,0,0,0,0,0,(char *)MNetUserPassword,
    DI_PSWEDIT,5,5,70,3,0,0,0,0,"",
    DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  static char LastName[256],LastPassword[256];
  if (Title!=NULL)
    CharToOem(Title,DialogItems[0].Data);
  strcpy(DialogItems[2].Data,LastName);
  strcpy(DialogItems[4].Data,LastPassword);
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,10,"NetBrowse",DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (ExitCode!=6)
    return(FALSE);
  strcpy(LastName,DialogItems[2].Data);
  strcpy(LastPassword,DialogItems[4].Data);
  strcpy(Name,DialogItems[2].Data);
  strcpy(Password,DialogItems[4].Data);
  return(TRUE);
}


void NetBrowser::GetRemoteName(NETRESOURCE *NetRes,char *RemoteName)
{
  if (NetRes->lpProvider!=NULL && (NetRes->lpRemoteName==NULL ||
      NetRes->dwDisplayType==RESOURCEDISPLAYTYPE_NETWORK))
    strcpy(RemoteName,NetRes->lpProvider);
  else
    if (NetRes->lpRemoteName==NULL)
      *RemoteName=0;
    else
      strcpy(RemoteName,NetRes->lpRemoteName);
}


void NetBrowser::GetNetList(HANDLE hEnum,NETRESOURCE *(&Res),DWORD &EnumCount)
{
  DeleteNetList(Res,EnumCount);
  while (1)
  {
    NETRESOURCE nr[1024];
    DWORD NetSize=sizeof(nr),NetCount=sizeof(nr)/sizeof(nr[0]);
    DWORD EnumCode=WNetEnumResource(hEnum,&NetCount,nr,&NetSize);
    if (EnumCode!=NO_ERROR)
    {
      if (EnumCode!=ERROR_NO_MORE_ITEMS)
        DeleteNetList(Res,EnumCount);
      break;
    }
    if (NetCount>0)
    {
      Res=(NETRESOURCE *)realloc(Res,(EnumCount+NetCount)*sizeof(*Res));
      for (int I=0;I<NetCount;I++)
      {
        NETRESOURCE *destnr=Res+EnumCount+I;
        *destnr=nr[I];

        destnr->lpRemoteName=CopyText(nr[I].lpRemoteName);
        destnr->lpLocalName=CopyText(nr[I].lpLocalName);
        destnr->lpComment=CopyText(nr[I].lpComment);
        destnr->lpProvider=CopyText(nr[I].lpProvider);
      }
      EnumCount+=NetCount;
    }
  }
}


void NetBrowser::DeleteNetList(NETRESOURCE *(&Res),DWORD &ResCount)
{
  for (int I=0;I<ResCount;I++)
    DeleteNetText(Res[I]);
  delete Res;
  Res=NULL;
  ResCount=0;
}


char* NetBrowser::CopyText(char *Text)
{
  if (Text==NULL)
    return(NULL);
  char *Buf=new char[strlen(Text)+1];
  strcpy(Buf,Text);
  return(Buf);
}


void NetBrowser::DeleteNetText(NETRESOURCE &Res)
{
  delete Res.lpRemoteName;
  delete Res.lpLocalName;
  delete Res.lpComment;
  delete Res.lpProvider;
}


int NetBrowser::IsReadable(char *Remote)
{
  char Mask[NM];
  sprintf(Mask,"%s\\*",Remote);
  HANDLE FindHandle;
  WIN32_FIND_DATA FindData;
  FindHandle=FindFirstFile(Mask,&FindData);
  FindClose(FindHandle);
  return(FindHandle!=INVALID_HANDLE_VALUE);
}


void NetBrowser::DeleteCommonRootResources()
{
  for (int I=0;I<CommonRootCount;I++)
    DeleteNetText(CommonRootResources[I]);
}
