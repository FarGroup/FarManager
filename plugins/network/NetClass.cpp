#include "NetClass.hpp"
#include "NetFavorites.hpp"
#include "NetCommon.hpp"
#include "NetReg.hpp"
#include "NetCfg.hpp"

NetResourceList *CommonRootResources;
BOOL SavedCommonRootResources = FALSE;

// -- NetResourceList --------------------------------------------------------
#ifdef NETWORK_LOGGING
FILE* NetBrowser::LogFile = NULL;
int NetBrowser::LogFileRef = 0;

void NetBrowser::OpenLogFile(char *lpFileName)
{
  if(!LogFileRef)LogFile = fopen (lpFileName, "a+t"), fprintf (LogFile, "Opening plugin\n");
  LogFileRef++;
}

void NetBrowser::CloseLogfile()
{
  LogFileRef--;
  if(!LogFileRef)fclose (LogFile),LogFile = NULL;
}

void NetBrowser::LogData(char * Data)
{
  fprintf(LogFile,"%s\n", Data);
  char buffer[MAX_PATH];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buffer, MAX_PATH, NULL);
  fprintf(LogFile,"GetLastError returns: %s\n", buffer);
}
#endif

NetResourceList::NetResourceList()
: ResList (NULL), ResCount (0)
{
}

NetResourceList::~NetResourceList()
{
  Clear();
}

NetResourceList &NetResourceList::operator= (NetResourceList &other)
{
  Clear();
  for (unsigned I=0; I<other.Count(); I++)
    Push (other [I]);
  return *this;
}

void NetResourceList::Clear()
{
  for (unsigned i=0; i < ResCount; i++)
    DeleteNetResource (ResList [i]);
  free (ResList);
  ResList=NULL;
  ResCount=0;
}

void NetResourceList::DeleteNetResource (NETRESOURCE &Res)
{
  free(Res.lpRemoteName);
  free(Res.lpLocalName);
  free(Res.lpComment);
  free(Res.lpProvider);
}

char *NetResourceList::CopyText (const char *Text)
{
  if (!Text)
    return NULL;
  char *Buf=(char*)malloc(lstrlen(Text)+1);
  lstrcpy(Buf,Text);
  return(Buf);
}

void NetResourceList::InitNetResource (NETRESOURCE &Res)
{
  Res.lpRemoteName = NULL;
  Res.lpLocalName  = NULL;
  Res.lpComment    = NULL;
  Res.lpProvider   = NULL;
}

void NetResourceList::CopyNetResource (NETRESOURCE &Dest, const NETRESOURCE &Src)
{
  free (Dest.lpRemoteName);
  free (Dest.lpLocalName);
  free (Dest.lpComment);
  free (Dest.lpProvider);
  memcpy (&Dest, &Src, sizeof (NETRESOURCE));
  Dest.lpRemoteName = CopyText (Src.lpRemoteName);
  Dest.lpLocalName  = CopyText (Src.lpLocalName);
  Dest.lpComment    = CopyText (Src.lpComment);
  Dest.lpProvider   = CopyText (Src.lpProvider);
}

void NetResourceList::Push (NETRESOURCE &Res)
{
  ResList=(NETRESOURCE *)realloc(ResList,(ResCount+1)*sizeof(*ResList));
  ZeroMemory(&ResList [ResCount], sizeof(*ResList));
  CopyNetResource (ResList [ResCount], Res);
  ResCount++;
}

NETRESOURCE *NetResourceList::Top()
{
  if (ResCount == 0)
    return NULL;
  return &ResList [ResCount-1];
}

void NetResourceList::Pop()
{
  if (ResCount > 0)
  {
    DeleteNetResource (ResList [ResCount-1]);
    ResList=(NETRESOURCE *)realloc(ResList,(ResCount-1)*sizeof(*ResList));
    ResCount--;
  }
}

BOOL NetResourceList::Enumerate (DWORD dwScope, DWORD dwType, DWORD dwUsage,
                                 LPNETRESOURCE lpNetResource)
{
  Clear();

  if (GetFavorites(lpNetResource, this))
    return TRUE;

  HANDLE hEnum;
  if (WNetOpenEnum(dwScope, dwType, dwUsage, lpNetResource, &hEnum)!=NO_ERROR)
    return FALSE;

  BOOL EnumFailed = FALSE;
  for(;;)
  {
    NETRESOURCE nr[1024];
    DWORD NetSize=sizeof(nr),NetCount=sizeof(nr)/sizeof(nr[0]);
    DWORD EnumCode=WNetEnumResource(hEnum,&NetCount,nr,&NetSize);
    if (EnumCode!=NO_ERROR)
    {
      //In W9X the last item in the list may be signaled by ERROR_INVALID_PARAMETER
      if(WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT
        && EnumCode == ERROR_INVALID_PARAMETER
        && dwScope == RESOURCE_CONNECTED)
        break;

      if (EnumCode!=ERROR_NO_MORE_ITEMS)
      {
        Clear();
        EnumFailed = TRUE;
      }
      break;
    }
    if (NetCount>0)
    {
      ResList=(NETRESOURCE *)realloc(ResList,(ResCount+NetCount)*sizeof(*ResList));
      ZeroMemory(&ResList [ResCount], sizeof(*ResList)*NetCount);
      for (unsigned I=0;I<NetCount;I++)
        CopyNetResource (ResList [ResCount+I], nr [I]);
      ResCount+=NetCount;
    }
  }

  WNetCloseEnum(hEnum);
  return !EnumFailed;
}

// -- NetBrowser -------------------------------------------------------------

NetBrowser::NetBrowser()
{
  *(LPDWORD)&PanelMode = 0;
  GetRegKey(HKEY_CURRENT_USER, "", StrPanelMode, PanelMode, "3", sizeof(PanelMode));
  NetResourceList::InitNetResource(CurResource);
  ReenterGetFindData = 0;

  ChangeDirSuccess = TRUE;
  OpenFromFilePanel = FALSE;
  if (SavedCommonRootResources)
  {
    RootResources = *CommonRootResources;
    PCurResource = RootResources.Top();
  }
  else {
    NetResourceList::CopyNetResource (CurResource, CommonCurResource);
    if (PCommonCurResource)
      PCurResource = &CurResource;
    else
      PCurResource = NULL;
  }
  NetListRemoteName [0] = '\0';
  CmdLinePath [0] = '\0';

#ifdef NETWORK_LOGGING
  OpenLogFile("c:\\network.log");
#endif
}


NetBrowser::~NetBrowser()
{
#ifdef NETWORK_LOGGING
  CloseLogfile();
#endif
}

#ifdef NETWORK_LOGGING

void NetBrowser::LogNetResource (NETRESOURCE &Res)
{
  fprintf (LogFile, "dwScope = %d\ndwType = %d\ndwDisplayType = %d\ndwUsage = %d\n",
    Res.dwScope, Res.dwType, Res.dwDisplayType, Res.dwUsage);
  fprintf (LogFile, "lpLocalName = %s\nlpRemoteName = %s\nlpComment = %s\nlpProvider = %s\n\n",
    Res.lpLocalName, Res.lpRemoteName, Res.lpComment, Res.lpProvider);
}

#endif

BOOL NetBrowser::EnumerateNetList()
{
  if (PCurResource && PCurResource->lpRemoteName)
    lstrcpy (NetListRemoteName, PCurResource->lpRemoteName);
  else
    NetListRemoteName [0] = '\0';
  if (!NetList.Enumerate (RESOURCE_GLOBALNET,RESOURCETYPE_ANY,0,PCurResource))
  {
    if (PCurResource == NULL)
    {
      const char *MsgItems[]={GetMsg(MError),GetMsg(MNetCannotBrowse),GetMsg(MOk)};
      Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
      return(FALSE);
    }
    else {
      // try again with connection
      AddConnection (PCurResource);
      if (!NetList.Enumerate (RESOURCE_GLOBALNET,RESOURCETYPE_ANY,0,PCurResource))
        NetList.Clear();
    }
  }
  if(!CheckFavoriteItem(PCurResource) && Opt.NTGetHideShare)
  {
    PanelInfo PInfo;
    Info.Control (this, FCTL_GETPANELINFO, &PInfo);
    if (!Opt.HiddenSharesAsHidden || (PInfo.Flags & PFLAGS_SHOWHIDDEN))
    {
      // Check whether we need to get the hidden shares.
      if(NetList.Count() > 0)
      {
        if(PCurResource != NULL)
        {
          // If the parent of the current folder is not a server
          if(PCurResource->dwDisplayType != RESOURCEDISPLAYTYPE_SERVER)
            return TRUE;
        }
        // If there are elements, check the first element
        if((NetList[NetList.Count()-1].dwDisplayType) != RESOURCEDISPLAYTYPE_SHARE)
          return TRUE;
      }

      if (WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
        GetHideShareNT();
      else
        GetHideShare95();
    }
  }

  /*
  if (NetCount==0 && CurResource!=NULL && AddConnection(CurResource))
  if (WNetOpenEnum(RESOURCE_GLOBALNET,RESOURCETYPE_ANY,0,CurResource,&hEnum)==NO_ERROR)
  {
  GetNetList(hEnum,NetRes,NetCount);
  WNetCloseEnum(hEnum);
  }
  */
  return TRUE;
}

BOOL NetBrowser::GotoFavorite(char *lpPath)
{
#ifdef NEWTWORK_LOGGING
  LogData("Entered NetBrowser::GotoFavorite")
#endif
  NETRESOURCE nr = {0};
  if(GetFavoriteResource(lpPath, &nr))
  {
#ifdef NETWORK_LOGGING
    LogData("GetFavoriteResource SUCCEEDED");
    LogNetResource(nr);
#endif

    NetResourceList::CopyNetResource(CurResource, nr);
    NetResourceList::DeleteNetResource(nr);
    PCurResource = &CurResource;
    Info.Control (this, FCTL_UPDATEPANEL, NULL);
    Info.Control (this, FCTL_REDRAWPANEL, NULL);
    return TRUE;
  }
  return FALSE;
}

int NetBrowser::GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
#ifdef NETWORK_LOGGING
  LogData("Entering NetBrowser::GetFindData");
#endif
  if(OpMode & OPM_FIND)
    return FALSE;
  if(ReenterGetFindData)
    return TRUE;

  ReenterGetFindData++;

  if(ChangeDirSuccess)
  {
    if (CmdLinePath [0])
    {
      // prevent recursion
      char TmpCmdLinePath [NM];
      lstrcpy (TmpCmdLinePath, CmdLinePath);
      CmdLinePath [0] = 0;
    ReenterGetFindData--;
      if(!GotoFavorite(TmpCmdLinePath))
        GotoComputer (TmpCmdLinePath);
    ReenterGetFindData++;
    }

    *pPanelItem=NULL;
    *pItemsNumber=0;
    TSaveScreen SS;

    // get the list of connections, so that we can show mapped drive letters
    if (!ConnectedList.Enumerate (RESOURCE_CONNECTED,RESOURCETYPE_DISK,0,NULL))
    {
      const char *MsgItems[]={GetMsg(MError),GetMsg(MNetCannotBrowse),GetMsg(MOk)};
      Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
      ReenterGetFindData--;
      return FALSE;
    }

    if (!EnumerateNetList())
    {
      ReenterGetFindData--;
      return FALSE;
    }
  }
  ChangeDirSuccess = TRUE;

  PluginPanelItem *NewPanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*NetList.Count());
  *pPanelItem=NewPanelItem;
  if (NewPanelItem==NULL)
  {
    ReenterGetFindData--;
    return(FALSE);
  }

  int CurItemPos=0;
  for (unsigned I=0;I<NetList.Count();I++)
  {
    if (NetList[I].dwType==RESOURCETYPE_PRINT)
      continue;

    char RemoteName[NM],LocalName[NM],Comment[300];

    GetRemoteName(&NetList [I],RemoteName);
    if (NetList[I].lpComment==NULL)
      *Comment=0;
    else
      CharToOem(NetList[I].lpComment,Comment);
    memset(&NewPanelItem[CurItemPos],0,sizeof(PluginPanelItem));

    NewPanelItem[CurItemPos].CustomColumnData=(LPSTR*)malloc(sizeof(LPSTR)*2);

    GetLocalName(NetList[I].lpRemoteName,LocalName);

    NewPanelItem[CurItemPos].CustomColumnData[0]=(char*)malloc(lstrlen(LocalName)+1);
    lstrcpy(NewPanelItem[CurItemPos].CustomColumnData[0],LocalName);

    NewPanelItem[CurItemPos].CustomColumnData[1]=(char*)malloc(lstrlen(Comment)+1);
    lstrcpy(NewPanelItem[CurItemPos].CustomColumnData[1],Comment);
    NewPanelItem[CurItemPos].CustomColumnNumber=2;

    CharToOem(RemoteName,NewPanelItem[CurItemPos].FindData.cFileName);

    DWORD attr = FILE_ATTRIBUTE_DIRECTORY;
    if (Opt.HiddenSharesAsHidden && RemoteName [lstrlen (RemoteName)-1] == '$')
      attr |= FILE_ATTRIBUTE_HIDDEN;
    NewPanelItem[CurItemPos].FindData.dwFileAttributes=attr;
    CurItemPos++;
  }
  *pItemsNumber=CurItemPos;

  ReenterGetFindData--;
  return(TRUE);
}

void NetBrowser::FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber)
{
  for (int I=0;I<ItemsNumber;I++)
  {
    free(PanelItem[I].CustomColumnData[0]);
    free(PanelItem[I].CustomColumnData[1]);
    free(PanelItem[I].CustomColumnData);
  }
  free(PanelItem);
}


int NetBrowser::ProcessEvent (int Event, void* /*Param*/)
{
  if (Event == FE_CLOSE)
  {
  struct PanelInfo pInfo;
  Info.Control(this, FCTL_GETPANELSHORTINFO, &pInfo);
  pInfo.ViewMode += 0x30;
  SetRegKey(HKEY_CURRENT_USER, "", StrPanelMode, (char*)&pInfo.ViewMode);
    if (PCurResource == NULL || IsMSNetResource (*PCurResource))
    {
      NetResourceList::CopyNetResource (CommonCurResource, CurResource);
      PCommonCurResource = PCurResource ? &CommonCurResource : NULL;
      SavedCommonRootResources = false;
    }
    else {
      *CommonRootResources = RootResources;
      SavedCommonRootResources = true;
    }
  }
  return FALSE;
}


int NetBrowser::DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                            int /*OpMode*/)
{
  if (CheckFavoriteItem(PCurResource))
  {
    //Deleting from favorites
    RemoveItems();
  }
  else
  {
    for (int I=0;I<ItemsNumber;I++)
      if (PanelItem[I].CustomColumnNumber==2 && PanelItem[I].CustomColumnData!=NULL)
      {
        if (*PanelItem[I].CustomColumnData[0])
          if (!CancelConnection (PanelItem [I].FindData.cFileName)) break;
      }
  }
  return(TRUE);
}

BOOL NetBrowser::CancelConnection (char *RemoteName)
{
  char LocalName [NM];
  char szFullName[NM] = {0};
  if(Opt.FullPathShares)
    lstrcpy(szFullName, RemoteName);
  else if(PCurResource && PCurResource->lpRemoteName)
    FSF.sprintf(szFullName, "%s\\%s", PCurResource->lpRemoteName, RemoteName);
  else
    return FALSE;
  if (!GetDriveToDisconnect (szFullName, LocalName))
    return FALSE;

  int UpdateProfile = 0;
  if (!ConfirmCancelConnection (LocalName, szFullName, UpdateProfile))
    return FALSE;

  DWORD status = WNetCancelConnection2(LocalName,UpdateProfile,FALSE);
  // if we're on the drive we're disconnecting, set the directory to
  // a different drive and try again
  if (status != NO_ERROR && HandsOffDisconnectDrive (LocalName))
    status = WNetCancelConnection2(LocalName,UpdateProfile,FALSE);
  if(status!=NO_ERROR)
  {
    int Failed=FALSE;
    char MsgText[200];
    FSF.sprintf(MsgText,GetMsg(MNetCannotDisconnect),LocalName);
    int LastError=GetLastError();
    if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
    {
      const char *MsgItems[]={GetMsg(MError),MsgText,"\x1",GetMsg(MOpenFiles),GetMsg(MAskDisconnect),GetMsg(MOk),GetMsg(MCancel)};
      if (Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)==0)
        // всегда рвать соединение
        if (WNetCancelConnection2(LocalName,UpdateProfile,TRUE)!=NO_ERROR)
          Failed=TRUE;
    }
    else
      Failed=TRUE;
    if (Failed)
    {
      const char *MsgItems[]={GetMsg(MError),MsgText,GetMsg(MOk)};
      Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
      return FALSE;
    }
  }
  return TRUE;
}

BOOL NetBrowser::GetDriveToDisconnect (const char *RemoteName, char *LocalName)
{
  char LocalNames [NM][10];
  DWORD LocalNameCount = 0;
  DWORD i;

  for (i = 0; i < ConnectedList.Count(); i++)
  {
    NETRESOURCE &connRes = ConnectedList [i];
    if (connRes.lpRemoteName && connRes.lpLocalName &&
      *connRes.lpLocalName && lstrcmpi (connRes.lpRemoteName, RemoteName) == 0)
    {
      if (connRes.dwScope == RESOURCE_CONNECTED ||
        connRes.dwScope == RESOURCE_REMEMBERED)
      {
        CharToOem (connRes.lpLocalName, LocalNames [LocalNameCount++]);
        if (LocalNameCount == 10) break;
      }
    }
  }
  if (!LocalNameCount) return FALSE;   // hmmm... strange

  if (LocalNameCount == 1)
    lstrcpy (LocalName, LocalNames [0]);
  else
  {
    char MsgText [512];
    FSF.sprintf (MsgText, GetMsg (MMultipleDisconnect), RemoteName);
    for (i=0; i<LocalNameCount; i++)
    {
      lstrcat (MsgText, LocalNames [i]);
      lstrcat (MsgText, "\n");
    }
    int index = Info.Message (Info.ModuleNumber, FMSG_ALLINONE, NULL,
      (const char **) MsgText, 3+LocalNameCount, LocalNameCount);
    if (index < 0)
      return FALSE;

    lstrcpy (LocalName, LocalNames [index]);
  }
  return TRUE;
}

BOOL NetBrowser::ConfirmCancelConnection (char *LocalName, char *RemoteName, int &UpdateProfile)
{
  char MsgText[NM];
  struct InitDialogItem InitItems[]=
  {
    /* 0 */ { DI_DOUBLEBOX, 3, 1, 72, 9, 0, 0, 0,                0,"" },
    /* 1 */ { DI_TEXT,      5, 2,  0, 0, 0, 0, DIF_SHOWAMPERSAND,0,"" },
    /* 2 */ { DI_TEXT,      5, 3,  0, 0, 0, 0, DIF_SHOWAMPERSAND,0,"" },
    /* 3 */ { DI_TEXT,      5, 4,  0, 0, 0, 0, DIF_SHOWAMPERSAND,0,"" },
    /* 4 */ { DI_TEXT,      0, 5,  0, 6, 0, 0, DIF_SEPARATOR,    0,"" },
    /* 5 */ { DI_CHECKBOX,  5, 6, 70, 5, 0, 0, 0,                0,"" },
    /* 6 */ { DI_TEXT,      0, 7,  0, 6, 0, 0, DIF_SEPARATOR,    0,"" },
    /* 7 */ { DI_BUTTON,    0, 8,  0, 0, 1, 0, DIF_CENTERGROUP,  1,"" },
    /* 8 */ { DI_BUTTON,    0, 8,  0, 0, 0, 0, DIF_CENTERGROUP,  0,"" }
  };
  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  int Len1=lstrlen(lstrcpy(DialogItems[0].Data,GetMsg(MConfirmDisconnectTitle)));
  FSF.sprintf(MsgText,GetMsg(MConfirmDisconnectQuestion),LocalName);
  Len1=max (Len1, lstrlen(lstrcpy(DialogItems[1].Data,MsgText)));
  FSF.sprintf(MsgText,GetMsg(MConfirmDisconnectMapped),LocalName);
  Len1=max (Len1, lstrlen(lstrcpy(DialogItems[2].Data,MsgText)));
  Len1=max (Len1, lstrlen(lstrcpy(DialogItems[5].Data,GetMsg(MConfirmDisconnectReconnect))));

  lstrcpy(DialogItems[3].Data,FSF.TruncPathStr(RemoteName,Len1));

  lstrcpy(DialogItems[7].Data,GetMsg(MYes));
  lstrcpy(DialogItems[8].Data,GetMsg(MCancel));

  BOOL IsPersistent = TRUE;
  // Check if this was a permanent connection or not.
  {
    HKEY hKey;
    FSF.sprintf(MsgText,"Network\\%c",FSF.LUpper(LocalName [0]));
    if(RegOpenKeyEx(HKEY_CURRENT_USER,MsgText,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
    {
      DialogItems[5].Flags|=DIF_DISABLE;
      DialogItems[5].Selected=0;
      IsPersistent=FALSE;
    }
    else
      DialogItems[5].Selected=Opt.DisconnectMode;
    RegCloseKey(hKey);
  }

  // adjust the dialog size
  DialogItems[0].X2=DialogItems[0].X1+Len1+3;

  int ExitCode;
  if (!NeedConfirmCancelConnection())
    ExitCode = 7;
  else
    ExitCode = Info.Dialog (Info.ModuleNumber, -1, -1, DialogItems [0].X2+4, 11,
    "DisconnectDrive", DialogItems, sizeof (DialogItems)/sizeof (DialogItems [0]));

  UpdateProfile=DialogItems[5].Selected?0:CONNECT_UPDATE_PROFILE;
  if(ExitCode == 7 && IsPersistent)
  {
    Opt.DisconnectMode=DialogItems[5].Selected;
    SetRegKey(HKEY_CURRENT_USER,"",StrDisconnectMode,Opt.DisconnectMode);
  }
  return ExitCode == 7;
}


BOOL NetBrowser::NeedConfirmCancelConnection()
{
  return (Info.AdvControl (Info.ModuleNumber, ACTL_GETCONFIRMATIONS, NULL) &
    FCS_DISCONNECTNETWORKDRIVE) != 0;
}


BOOL NetBrowser::HandsOffDisconnectDrive (const char *LocalName)
{
  char DirBuf [NM];
  GetCurrentDirectory (sizeof (DirBuf)-1, DirBuf);
  if (FSF.LUpper (DirBuf [0]) != FSF.LUpper (LocalName [0]))
    return FALSE;

  // change to the root of the drive where network.dll resides
  if (!GetModuleFileName (NULL, DirBuf, sizeof (DirBuf)-1))
    return FALSE;

  DirBuf [3] = '\0';   // truncate to "X:\\"
  return SetCurrentDirectory (DirBuf);
}

void NetBrowser::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
#ifdef NETWORK_LOGGING__
  if(PCurResource)
    LogData("Entering NetBrowser::GetOpenPluginInfo. Info->Flags will contain OPIF_ADDDOTS");
  else
    LogData("Entering NetBrowser::GetOpenPluginInfo. Info->Flags will NOT contain OPIF_ADDDOTS");
#endif
  Info->StructSize=sizeof(*Info);
  Info->Flags=OPIF_USEHIGHLIGHTING|OPIF_ADDDOTS|OPIF_RAWSELECTION|
    OPIF_SHOWPRESERVECASE|OPIF_FINDFOLDERS;
  Info->HostFile=NULL;
  if (PCurResource == NULL)
  {
    Info->CurDir="";
    if(Opt.NoRootDoublePoint)
      Info->Flags &= ~OPIF_ADDDOTS;
  }
  else
  {
    static char CurDir[NM];
    if (PCurResource->lpRemoteName==NULL)
      if (CheckFavoriteItem(PCurResource))
        lstrcpy(CurDir, GetMsg(MFavorites));
      else
        CharToOem (PCurResource->lpProvider, CurDir);
      else
        CharToOem (PCurResource->lpRemoteName, CurDir);
      Info->CurDir=CurDir;
  }

  Info->Format=(char *) GetMsg(MNetwork);

  static char Title[NM];
  FSF.sprintf(Title," %s: %s ",GetMsg(MNetwork), Info->CurDir);
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
  Info->StartPanelMode=*(LPDWORD)&PanelMode;//'3';  // TODO Panel mode should be read from the registry
  static struct KeyBarTitles KeyBar={
    {NULL,NULL,"","","","","","",NULL,NULL,NULL,NULL},
    {NULL,NULL,NULL,NULL,"","",NULL,NULL,NULL,NULL,NULL,NULL},
    {NULL,NULL,"","","","",NULL,NULL,NULL,NULL,NULL,NULL},
    {"","","","","","","","",NULL,NULL,NULL,NULL}
  };
  if (PCurResource && PCurResource->dwDisplayType == RESOURCEDISPLAYTYPE_SERVER)
  {
    if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
      KeyBar.Titles[4-1]=GetMsg(MF4);
    KeyBar.Titles[5-1]=GetMsg(MF5);
    KeyBar.Titles[6-1]=GetMsg(MF6);
    KeyBar.ShiftTitles[5-1]=GetMsg(MSHIFTF5);
    KeyBar.ShiftTitles[6-1]=GetMsg(MSHIFTF6);
    KeyBar.Titles[8-1]=GetMsg(MF8);
  }
  else
  {
    if(PCurResource && PCurResource->dwDisplayType == RESOURCEDISPLAYTYPE_DOMAIN)
      KeyBar.Titles[4-1]=GetMsg(MF4);
    else
      KeyBar.Titles[4-1]="";
    KeyBar.Titles[5-1]="";
    KeyBar.Titles[6-1]="";
    KeyBar.ShiftTitles[5-1]="";
    KeyBar.ShiftTitles[6-1]="";
    if(CheckFavoriteItem(PCurResource))
      KeyBar.Titles[8-1] = GetMsg(MF8Fav);
    else
      KeyBar.Titles[8-1]="";
  }
  Info->KeyBar=&KeyBar;
}


int NetBrowser::SetDirectory(const char *Dir,int OpMode)
{
  if(OpMode & OPM_FIND)
    return TRUE;
  ChangeDirSuccess = TRUE;

  if (OpenFromFilePanel)
    PCurResource = NULL;

  BOOL TmpOpenFromFilePanel = OpenFromFilePanel;
  OpenFromFilePanel = FALSE;

  if (!Dir || lstrcmp(Dir,"\\")==0)
  {
    PCurResource = NULL;
    RootResources.Clear();
    return(TRUE);
  }
  if (lstrcmp(Dir,"..")==0)
  {
    if (PCurResource == NULL)
      return FALSE;

    if (IsMSNetResource (*PCurResource))
    {
      NETRESOURCE nrParent;
      NetResourceList::InitNetResource (nrParent);
      if (!GetResourceParent (*PCurResource, &nrParent))
        PCurResource = NULL;
      else
      {
        CurResource = nrParent;
        PCurResource = &CurResource;
      }
    }
    else
    {
      RootResources.Pop();
      PCurResource = RootResources.Top();
    }
    return TRUE;
  }
  else
  {
    ChangeDirSuccess = TRUE;
    if (ChangeToDirectory (Dir, ((OpMode & OPM_FIND) != 0), 0))
      return ChangeDirSuccess;

    char AnsiDir[NM];
    OemToChar(Dir,AnsiDir);
    if (AnsiDir [0] == '/')
      AnsiDir [0] = '\\';
    if (AnsiDir [1] == '/')
      AnsiDir [1] = '\\';

    // if still haven't found and the name starts with \\, try to jump to a
    // computer in a different domain
    if (strncmp (AnsiDir, "\\\\", 2) == 0)
    {
      if (!TmpOpenFromFilePanel && strchr (AnsiDir+2, '\\'))
      {
        if (!IsReadable (AnsiDir))
        {
#ifdef NETWORK_LOGGING
          char szErrBuff[MAX_PATH*2];
          sprintf(szErrBuff, "GetLastError = %d at line %d, file %s", GetLastError(), __LINE__, __FILE__);
          LogData(szErrBuff);
#endif
          Info.Message (Info.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK | FMSG_ALLINONE,
            NULL, (const char **) GetMsg (MError), 0, 0);
          return FALSE;
        }
        Info.Control (this, FCTL_CLOSEPLUGIN, (void *) Dir);
        return TRUE;
      }
      ChangeDirSuccess = GotoComputer (AnsiDir);
      return ChangeDirSuccess;
    }
  }
  return(FALSE);
}


BOOL NetBrowser::ChangeToDirectory (const char *Dir, int IsFind, int IsExplicit)
{
  // if we already have the resource list for the current directory,
  // do not scan it again
  if (!PCurResource || !PCurResource->lpRemoteName ||
    lstrcmp (PCurResource->lpRemoteName, NetListRemoteName) != 0)
    EnumerateNetList();

  char AnsiDir[NM];
  OemToChar(Dir,AnsiDir);
  if (AnsiDir [0] == '/')
    AnsiDir [0] = '\\';
  if (AnsiDir [1] == '/')
    AnsiDir [1] = '\\';

  for (unsigned I=0;I<NetList.Count();I++)
  {
    char RemoteName[NM];
    GetRemoteName(&NetList[I],RemoteName);
    if (FSF.LStricmp(AnsiDir,RemoteName)==0)
    {
      if (CheckFavoriteItem(&NetList[I]))
      {
        NetResourceList::CopyNetResource (CurResource, NetList [I]);
        PCurResource = &CurResource;
        //RootResources.Push (CurResource);

        return TRUE;
      }
      if ((NetList[I].dwUsage & RESOURCEUSAGE_CONTAINER)==0 &&
        (NetList[I].dwType & RESOURCETYPE_DISK) &&
        NetList[I].lpRemoteName!=NULL)
      {
        if (IsFind)
          return(FALSE);
        char NewDir[NM],LocalName[NM];
        GetLocalName(NetList[I].lpRemoteName,LocalName);
        if (*LocalName)
          if(IsReadable(LocalName))
            lstrcpy(NewDir,LocalName);
          else
          {
            Info.Message (Info.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK | FMSG_ALLINONE,
              NULL, (const char **) GetMsg (MError), 0, 0);
            return TRUE;
          }
          else
          {
            BOOL ConnectError = FALSE;
            lstrcpy(NewDir,NetList[I].lpRemoteName);
            CharToOem(NewDir,NewDir);
            if (IsExplicit)
            {
              if (!AddConnectionExplicit (&NetList [I]) || !IsReadable (NewDir))
                ConnectError = TRUE;
            }
            else {
              if (!IsReadable(NewDir))
                if (!AddConnection(&NetList[I]) || !IsReadable (NewDir))
                  ConnectError = TRUE;
            }
            if (ConnectError)
            {
              DWORD res = GetLastError();
              if(!IsExplicit)
                if (res == ERROR_INVALID_PASSWORD || res == ERROR_LOGON_FAILURE || res == ERROR_ACCESS_DENIED || res == ERROR_INVALID_HANDLE)
                  ConnectError = !((AddConnectionFromFavorites(&NetList[I]) ||
                  AddConnectionExplicit(&NetList[I])) && IsReadable (NewDir));
                if(ConnectError)
                {
                  ChangeDirSuccess = FALSE;
                  if (GetLastError() != ERROR_CANCELLED)
                    Info.Message (Info.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK | FMSG_ALLINONE,
                    NULL, (const char **) GetMsg (MError), 0, 0);
                  return TRUE;
                }
            }
          }
          Info.Control(this,FCTL_CLOSEPLUGIN,NewDir);
          return(TRUE);
      }
      if (IsExplicit?!AddConnectionExplicit(&NetList[I]):!IsResourceReadable (NetList [I]))
      {
        int res = GetLastError();
        if (res == ERROR_INVALID_PASSWORD || res == ERROR_LOGON_FAILURE || res == ERROR_ACCESS_DENIED || res == ERROR_LOGON_TYPE_NOT_GRANTED)
          ChangeDirSuccess = IsExplicit?FALSE:(AddConnectionFromFavorites(&NetList[I]) || AddConnectionExplicit(&NetList[I]));
        else
          ChangeDirSuccess = FALSE;
        if(!ChangeDirSuccess)
        {
          if (GetLastError() != ERROR_CANCELLED)
            Info.Message (Info.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK | FMSG_ALLINONE,
            NULL, (const char **) GetMsg (MError), 0, 0);
          return FALSE;
        }
      }
      NetResourceList::CopyNetResource (CurResource, NetList [I]);
      PCurResource = &CurResource;
      if (!IsMSNetResource (CurResource))
      {
#ifdef NETWORK_LOGGING
        LogData("Resource is not MSN");
        LogNetResource(CurResource);
#endif
        RootResources.Push (CurResource);
      }
#ifdef NETWORK_LOGGING
      else
      {
        LogData("Resource is MSN");
        LogNetResource(CurResource);
      }
#endif
      return(TRUE);
    }
  }
  return FALSE;
}


BOOL NetBrowser::IsMSNetResource (const NETRESOURCE &Res)
{
  if (!Res.lpProvider)
    return TRUE;
  return (strstr (Res.lpProvider, "Microsoft") != NULL) ||
    CheckFavoriteItem((LPNETRESOURCE)&Res);
}


BOOL NetBrowser::IsResourceReadable (NETRESOURCE &Res)
{
  if(CheckFavoriteItem(&Res))
    return TRUE;
  HANDLE hEnum = INVALID_HANDLE_VALUE;
  DWORD result = WNetOpenEnum (RESOURCE_GLOBALNET, RESOURCETYPE_ANY, 0, &Res, &hEnum);
  if (result != NO_ERROR)
  {
    if (!AddConnection (&Res))
      return FALSE;
    result = WNetOpenEnum (RESOURCE_GLOBALNET, RESOURCETYPE_ANY, 0, &Res, &hEnum);
    if (result != NO_ERROR)
      return FALSE;
  }
  if (hEnum != INVALID_HANDLE_VALUE)
    WNetCloseEnum (hEnum);
  return TRUE;
}

/*DELETING
BOOL NetBrowser::GetDfsParent(const NETRESOURCE &SrcRes, NETRESOURCE &Parent)
{
  if(!FNetDfsGetInfo)
    return FALSE;
  //we should allocate memory for Wide chars
  int nSize = MultiByteToWideChar(CP_ACP, 0, SrcRes.lpRemoteName, -1, NULL, 0);
  if(!nSize)
    return FALSE;
  WCHAR *szRes = new WCHAR[nSize++];

  if(!szRes)
    return FALSE;

    int Res = FALSE;
    if(MultiByteToWideChar(CP_ACP, 0, SrcRes.lpRemoteName, -1, szRes, nSize*sizeof(WCHAR)))
    {
    LPDFS_INFO_3 lpData;
    if(ERROR_SUCCESS == FNetDfsGetInfo(szRes, NULL, NULL, 3, (LPBYTE *) &lpData))
    {
      DWORD dwBuffSize = 32*sizeof(NETRESOURCE);
      NETRESOURCE *resResult = (NETRESOURCE *)malloc(dwBuffSize);
      CHAR *pszSys = NULL;
      for(DWORD i = 0; i < lpData->NumberOfStorages; i++)
      {
        nSize = WideCharToMultiByte(CP_ACP, 0, lpData->Storage[i].ServerName,
          -1, NULL, 0, NULL, NULL);
        if(!nSize)
          break;
        nSize += 3;
        CHAR *szServ =new CHAR[nSize];
        szServ[0] = '\\';
        szServ[1] = '\\';

        WideCharToMultiByte(CP_ACP, 0, lpData->Storage[i].ServerName,
          -1, &szServ[2], nSize - 2, NULL, NULL);
        NETRESOURCE inRes = {0};
        inRes.dwScope = RESOURCE_CONNECTED;
        inRes.dwType = RESOURCETYPE_ANY;
        inRes.lpRemoteName = szServ;
        DWORD dwRes;
        while((dwRes = FWNetGetResourceInformation(&inRes, resResult,
          &dwBuffSize, &pszSys)) == ERROR_MORE_DATA)
        {
          resResult = (NETRESOURCE *)realloc(resResult, dwBuffSize);
        }
        if(dwRes == ERROR_SUCCESS)
        {
          if(IsResourceReadable(*resResult))
          {
            NetResourceList::CopyNetResource(Parent, *resResult);
            Res = TRUE;
          }
        }

        delete(szServ);
        if(Res)
          break;
      }
      free(resResult);
    }
  }

  delete (szRes);
  return Res;
}
*/


BOOL NetBrowser::GetResourceInfo(char *SrcName,LPNETRESOURCE DstNetResource)
{
  NETRESOURCE nr = {0};

  if(!FWNetGetResourceInformation || !FWNetGetResourceParent)
    return FALSE;

#ifdef NETWORK_LOGGING
  fprintf (LogFile, "GetResourceInfo %s\n", SrcName);
#endif

  NETRESOURCE nrOut [32];   // provide buffer space
  NETRESOURCE *lpnrOut = &nrOut [0];
  DWORD cbBuffer = sizeof(nrOut);
  LPTSTR pszSystem = NULL;          // pointer to variable-length strings

  nr.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
  nr.dwScope       = RESOURCE_GLOBALNET;
  nr.dwType        = RESOURCETYPE_ANY;
  nr.dwUsage       = RESOURCEUSAGE_ALL;
  nr.lpRemoteName  = SrcName;
  nr.lpLocalName   = "";
  nr.lpComment     = "";
  nr.lpProvider    = "";

  DWORD dwError=FWNetGetResourceInformation(&nr,lpnrOut,&cbBuffer,&pszSystem);

  // If the call fails because the buffer is too small,
  //   call the LocalAlloc function to allocate a larger buffer.
  if (dwError == ERROR_MORE_DATA)
  {
    if((lpnrOut = (LPNETRESOURCE)LocalAlloc(LMEM_FIXED, cbBuffer)) != NULL)
      dwError = FWNetGetResourceInformation(&nr, lpnrOut, &cbBuffer, &pszSystem);
  }
  if (dwError == NO_ERROR)
  {
    if(DstNetResource)
      NetResourceList::CopyNetResource (*DstNetResource, *lpnrOut);

#ifdef NETWORK_LOGGING
    fprintf (LogFile, "Result:\n");
    LogNetResource (*DstNetResource);
#endif

    if (lpnrOut != &nrOut [0])
      LocalFree(lpnrOut);
    return TRUE;
  }
#ifdef NETWORK_LOGGING
  else
    fprintf (LogFile, "error %d\n", GetLastError());
#endif
  return FALSE;
}


BOOL NetBrowser::GetResourceParent (NETRESOURCE &SrcRes, LPNETRESOURCE DstNetResource)
{
  if(CheckFavoriteItem(&SrcRes) ||
    Opt.FavoritesFlags & FAVORITES_UPBROWSE_TO_FAVORITES)
  {
    if(GetFavoritesParent(SrcRes, DstNetResource))
      return TRUE;
  }
  if(!FWNetGetResourceInformation || !FWNetGetResourceParent)
    return FALSE;

#ifdef NETWORK_LOGGING
  fprintf (LogFile, "GetResourceParent() for:\n");
  LogNetResource (SrcRes);
#endif

  TSaveScreen ss;

  BOOL Ret=FALSE;
  NETRESOURCE nrOut [32];           // provide buffer space
  NETRESOURCE *lpnrOut = &nrOut [0];
  DWORD cbBuffer = sizeof(nrOut);
  LPTSTR pszSystem = NULL;          // pointer to variable-length strings

  NETRESOURCE nrSrc = SrcRes;
  nrSrc.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
  nrSrc.dwScope       = RESOURCE_GLOBALNET;
  nrSrc.dwUsage       = 0;
  nrSrc.dwType        = RESOURCETYPE_ANY;
  DWORD dwError=FWNetGetResourceInformation(&nrSrc,lpnrOut,&cbBuffer,&pszSystem);

  // If the call fails because the buffer is too small,
  //   call the LocalAlloc function to allocate a larger buffer.
  if (dwError == ERROR_MORE_DATA)
  {
    if((lpnrOut = (LPNETRESOURCE)LocalAlloc(LMEM_FIXED, cbBuffer)) != NULL)
      dwError = FWNetGetResourceInformation(&nrSrc, lpnrOut, &cbBuffer, &pszSystem);
  }
  if (dwError == NO_ERROR)
  {
#ifdef NETWORK_LOGGING
    fprintf (LogFile, "WNetGetResourceInformation() returned:\n");
    LogNetResource (*lpnrOut);
#endif

    nrSrc.lpProvider=lpnrOut->lpProvider;
    if(FWNetGetResourceParent(&nrSrc,lpnrOut,&cbBuffer) == NO_ERROR)
    {
      if(DstNetResource)
        NetResourceList::CopyNetResource (*DstNetResource, *lpnrOut);
#ifdef NETWORK_LOGGING
      fprintf (LogFile, "Result:\n");
      LogNetResource (*DstNetResource);
#endif
      Ret=TRUE;
    }
    if (lpnrOut != &nrOut [0])
      LocalFree(lpnrOut);
  }

  return Ret;
}

BOOL NetBrowser::EditFavorites()
{
  if(!PCurResource)
    return TRUE;
  // First we should determine the type of Favorite Item under cursor
  char szPath[NM] = {0};
  struct PanelInfo PInfo;
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);

  OemToChar(PInfo.CurDir, szPath);
  char *p = szPath + lstrlen(szPath);
  *p++ = '\\';
  OemToChar(PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName, p);
  NETRESOURCE nr = {0};
  if(GetFavoriteResource(szPath, &nr))
  {
    CharToOem(nr.lpRemoteName, szPath);
    switch(nr.dwDisplayType)
    {
    case RESOURCEDISPLAYTYPE_DOMAIN:
      Info.Message(
        Info.ModuleNumber,
        FMSG_ALLINONE,
        "Data",
        (const char * const *)"This is a domain",
        0,1);
      break;
    case RESOURCEDISPLAYTYPE_SERVER:
      Info.Message(
        Info.ModuleNumber,
        FMSG_ALLINONE,
        "Data",
        (const char * const *)"This is a SERVER",
        0,1);
      break;
    default:
      Info.Message(
        Info.ModuleNumber,
        FMSG_ALLINONE,
        "Data",
        (const char * const *)szPath,
        0,1);
    }

    NetResourceList::DeleteNetResource(nr);
    return TRUE;
  }

  return FALSE;
}


int NetBrowser::ProcessKey(int Key,unsigned int ControlState)
{
  if ((ControlState==0 || (ControlState&PKF_SHIFT)==PKF_SHIFT) &&
    (Key==VK_F5 || Key==VK_F6))
  {
    if (PCurResource && PCurResource->dwDisplayType == RESOURCEDISPLAYTYPE_SERVER)
    {
      struct PanelInfo PInfo;
      Info.Control(this,FCTL_GETPANELINFO,&PInfo);

      for (int I=0;I<PInfo.SelectedItemsNumber;I++)
      {
        if (!MapNetworkDrive (PInfo.SelectedItems[I].FindData.cFileName,
          (Key == VK_F6), ((ControlState&PKF_SHIFT)==0)))
          break;
      }
      Info.Control(this,FCTL_UPDATEPANEL,NULL);
      Info.Control(this,FCTL_REDRAWPANEL,NULL);
    }
    return(TRUE);
  }
  else if (Key == 'F' && (ControlState & PKF_CONTROL) == PKF_CONTROL)
  {
    PutCurrentFileName (TRUE);
    return TRUE;
  }
  else if (Key == VK_INSERT && (ControlState & (PKF_CONTROL | PKF_ALT)) == (PKF_CONTROL | PKF_ALT))
  {
    PutCurrentFileName (FALSE);
    return TRUE;
  }
  else if(Key == VK_F4 && !ControlState && WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    struct PanelInfo PInfo;
    Info.Control(this,FCTL_GETPANELINFO,&PInfo);
    if(lstrcmp(PInfo.SelectedItems[0].FindData.cFileName, ".."))
      if(ChangeToDirectory(PInfo.SelectedItems[0].FindData.cFileName, FALSE, TRUE))
        if(PointToName(PInfo.SelectedItems[0].FindData.cFileName) -
          PInfo.SelectedItems[0].FindData.cFileName <= 2)
        {
          Info.Control(this,FCTL_UPDATEPANEL,(void*)1);
          PanelRedrawInfo ri = {0};
          ri.CurrentItem = ri.TopPanelItem = 0;
          Info.Control(this,FCTL_REDRAWPANEL,&ri);
        }
        return TRUE;
  }
  else if(Key == VK_F4 && ControlState & PKF_SHIFT)
  {
    EditFavorites();
    return TRUE;
  }
  else if(Key == VK_F7 && !ControlState)
  {
    CreateFavSubFolder();
    return TRUE;
  }
  // disable processing of F3 - avoid unnecessary slowdown
  else if ((Key == VK_F3 || Key == VK_CLEAR) &&
    (ControlState == 0 || ((ControlState & PKF_ALT) == PKF_ALT)))
    return TRUE;
  else if ((Key == VK_PRIOR || Key == 0xDC) && (ControlState & PKF_CONTROL) == PKF_CONTROL
    && !PCurResource && Opt.NoRootDoublePoint)
    return TRUE;
  return(FALSE);
}


BOOL NetBrowser::MapNetworkDrive (char *RemoteName, BOOL AskDrive, BOOL Permanent)
{
  char AnsiRemoteName[NM];
  OemToChar(RemoteName,AnsiRemoteName);
  DWORD DriveMask=GetLogicalDrives();
  char NewLocalName[10];
  *NewLocalName=0;

  if (!AskDrive)
    GetFreeLetter(DriveMask,NewLocalName);
  else
  {
    if (!AskMapDrive (NewLocalName, Permanent))
      return FALSE;
  }
  if (*NewLocalName)
  {
    NETRESOURCE newnr;
    // char LocalName[10];
    newnr.dwType=RESOURCETYPE_DISK;
    newnr.lpLocalName=NewLocalName;
    newnr.lpRemoteName=AnsiRemoteName;
    newnr.lpProvider=NULL;

    for(;;)
    {
      if(IsReadable(AnsiRemoteName))
      {
        if (AddConnection(&newnr,Permanent))
          break;
      }else
        if((AddConnectionFromFavorites(&newnr, Permanent) || AddConnectionExplicit(&newnr, Permanent)) && IsReadable(newnr.lpLocalName))
          break;
        else if(ERROR_CANCELLED == GetLastError())
          break;

        if (GetLastError()==ERROR_DEVICE_ALREADY_REMEMBERED)
        {
          if (!AskDrive)
          {
            GetFreeLetter(DriveMask,NewLocalName);
            if (*NewLocalName==0)
            {
              const char *MsgItems[]={GetMsg(MError),GetMsg(MNoFreeLetters),GetMsg(MOk)};
              Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
              return FALSE;
            }
          }
          else
          {
            const char *MsgItems[]={GetMsg(MError),GetMsg(MAlreadyRemembered),GetMsg(MOk)};
            Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
            return FALSE;
          }
        }
        else
        {
          char MsgText[300];
          FSF.sprintf(MsgText,GetMsg(MNetCannotConnect),RemoteName,NewLocalName);
          const char *MsgItems[]={GetMsg(MError),MsgText,GetMsg(MOk)};
          Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
          return FALSE;
        }
    }
  }
  else
  {
    const char *MsgItems[]={GetMsg(MError),GetMsg(MNoFreeLetters),GetMsg(MOk)};
    Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
    return FALSE;
  }
  return TRUE;
}


BOOL NetBrowser::AskMapDrive (char *NewLocalName, BOOL &Permanent)
{
  int ExitCode = 0;
  for(;;)
  {
    struct FarMenuItem MenuItems['Z'-'A'+1];
    int MenuItemsNumber=0;
    memset(MenuItems,0,sizeof(MenuItems));
    DWORD DriveMask=GetLogicalDrives();
    for (int I=0;I<='Z'-'A';I++)
      if ((DriveMask & (1<<I))==0)
        FSF.sprintf(MenuItems[MenuItemsNumber++].Text,"&%c:",'A'+I);
      MenuItems [ExitCode].Selected = TRUE;

      if (!MenuItemsNumber)
        return FALSE;

      char *MenuTitle, *MenuBottom;
      if (Permanent)
      {
        MenuTitle = GetMsg (MPermanentTo);
        MenuBottom = GetMsg (MToggleTemporary);
      }
      else {
        MenuTitle = GetMsg (MTemporaryTo);
        MenuBottom = GetMsg (MTogglePermanent);
      }
      int BreakKeys[] = { VK_F6, 0};
      int BreakCode;

      ExitCode=Info.Menu(Info.ModuleNumber,-1,-1,0,0,
        MenuTitle,MenuBottom,StrHelpNetBrowse,
        BreakKeys,&BreakCode,MenuItems,MenuItemsNumber);
      if (ExitCode<0)
        return FALSE;
      if (BreakCode == -1)
      {
        lstrcpy(NewLocalName,MenuItems[ExitCode].Text+1);
        break;
      }
      Permanent = !Permanent;
  }
  return TRUE;
}


void NetBrowser::GetFreeLetter(DWORD &DriveMask,char *DiskName)
{
  *DiskName=0;
  for (char I=2;I<='Z'-'A';I++)
    if ((DriveMask & (1<<I))==0)
    {
      DriveMask |= 1<<I;
      DiskName[0]='A'+I;
      DiskName[1]=':';
      DiskName[2]=0;
      break;
    }
}


int NetBrowser::AddConnection(NETRESOURCE *nr,int Remember)
{
  NETRESOURCE connectnr=*nr;
  DWORD lastErrDebug = WNetAddConnection2(&connectnr,NULL,NULL,(Remember?CONNECT_UPDATE_PROFILE:0));
  if (lastErrDebug==NO_ERROR)
  {
    lastErrDebug = GetLastError();
    return(TRUE);
  }
  return(FALSE);
}

int NetBrowser::AddConnectionExplicit (NETRESOURCE *nr, int Remember)
{
  char Name[256],Password[256];
  NETRESOURCE connectnr=*nr;
  /*static*/ BOOL bSelected = FALSE;
  NameAndPassInfo passInfo={connectnr.lpRemoteName,Name,Password,&bSelected};
  if (!GetNameAndPassword(&passInfo))
  {
    SetLastError(ERROR_CANCELLED);
    return(FALSE);
  }
  if(AddConnectionWithLogon(&connectnr, Name, Password, Remember))
  {
    if(bSelected)
    {
      FAVORITEITEM Item;
      Item.lpRemoteName = connectnr.lpRemoteName;
      Item.lpUserName = Name;
      Item.lpPassword = Password;
      WriteFavoriteItem(&Item, passInfo.szFavoritePath);
    }
    return TRUE;
  }
  return FALSE;
}

int NetBrowser::AddConnectionWithLogon(NETRESOURCE *nr, char *Name, char *Password, int Remember)
{
  for(;;)
  {
    if(NO_ERROR == WNetAddConnection2(nr,Password,*Name ? Name:NULL,(Remember?CONNECT_UPDATE_PROFILE:0)))
    {
      return TRUE;
    }
    else
    {
      if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && ERROR_SESSION_CREDENTIAL_CONFLICT == GetLastError())
      {
        //Trying to cancel existing connections
        DisconnectFromServer(nr);

        if(NO_ERROR == WNetAddConnection2(nr,Password,*Name ? Name:NULL,(Remember?CONNECT_UPDATE_PROFILE:0)))
        {
          return TRUE;
        }
      }
    }
    if(ERROR_SUCCESS != GetLastError() && Name?(!strstr(Name, "\\")&&!strstr(Name, "@")):FALSE)
    {
      //If the specified user name does not look like "ComputerName\UserName" nor "User@Domain"
      //and the plug-in failed to log on to the remote machine, the specified user name can be
      //interpreted as user name of the remote computer, so let's transform it to look like
      //"Computer\User"

      char szServer[MAX_PATH];
      char szNameCopy[MAX_PATH];

      //make copy of Name
      lstrcpy(szNameCopy, Name);
      char *p = nr->lpRemoteName;
      int n = (int)(PointToName(p) - p);
      if(n <= 2)
        lstrcpyn(szServer, p + n, sizeof(szServer));
      else
      {
        while(*++p == '\\') n--;
        if(n > MAX_PATH) n = MAX_PATH;
        lstrcpyn(szServer, p, n-1);
      }
      FSF.sprintf(Name, "%s\\%s", szServer, szNameCopy);

      //Try again to log on with the transformed user name.
      continue;
    }
    return FALSE;
  }
}

int NetBrowser::AddConnectionFromFavorites(NETRESOURCE *nr,int Remember)
{
  //Try to search login info in registry
  if(nr)
  {
    char Name[NM] = {0};
    char Pass[NM] = {0};
    FAVORITEITEM Item =
    {
      nr->lpRemoteName,
        lstrlen(nr->lpRemoteName),
        Name,
        sizeof(Name),
        Pass,
        sizeof(Pass)
    };

    if(ReadFavoriteItem(&Item))
    {
      return AddConnectionWithLogon(nr, Name, Pass, Remember);
    }
  }
  return FALSE;
}

void NetBrowser::DisconnectFromServer(NETRESOURCE *nr)
{
  //We cannot disconnect if we run on Win9X
  if(WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT)
    return;
  //First we should know a name of the server
  int n = (int)(PointToName(nr->lpRemoteName) - nr->lpRemoteName);
  if(n <= 2)
    n = lstrlen(nr->lpRemoteName) + 1;
  char *szServer = (char*)malloc(n + 1);
  if(szServer)
  {
    char *szBuff = (char*)malloc(n + 1);
    if(szBuff)
    {
      lstrcpyn(szServer, nr->lpRemoteName, n);

      NETRESOURCE *lpBuff = 0;

      HANDLE hEnum;
      if(NO_ERROR == WNetOpenEnum(RESOURCE_CONNECTED, RESOURCETYPE_ANY, 0, NULL, &hEnum))
      {
        DWORD cCount = (DWORD)-1;
        DWORD nBuffSize = 0;
        //Let's determine buffer's size we need to store all the connections
        if(ERROR_MORE_DATA == WNetEnumResource(hEnum, &cCount, NULL, &nBuffSize))
        {
          lpBuff = (NETRESOURCE*)malloc(nBuffSize);
          if(lpBuff)
          {
            cCount = (DWORD)-1;
            if(NO_ERROR != WNetEnumResource(hEnum, &cCount, lpBuff, &nBuffSize))
              free(lpBuff), lpBuff = NULL;
          }
        }

        WNetCloseEnum(hEnum);
        if(lpBuff)
        {
          for(DWORD i = 0; i < cCount; i++)
          {
            lstrcpyn(szBuff, lpBuff[i].lpRemoteName, n);
            if(0 == lstrcmpi(szServer, szBuff))
              WNetCancelConnection2(lpBuff[i].lpRemoteName, 0, TRUE);
          }
          free(lpBuff);
        }
      }

      free(szBuff);
    }

    //Trying harder to disconnect from the server
    WNetCancelConnection2(szServer, 0, TRUE);
    free(szServer);
  }

  //Let's check the current dir and if it's remote try to change it to %TMP%
  char lpszPath[MAX_PATH];
  if(GetCurrentDirectory(MAX_PATH, lpszPath))
  {
    if(lpszPath[0] == '\\')
    {
      ExpandEnvironmentStrings("%TMP%", lpszPath, MAX_PATH);
      SetCurrentDirectory(lpszPath);
    }
  }
}


void NetBrowser::GetLocalName(char *RemoteName,char *LocalName)
{
  *LocalName=0;
  if (RemoteName!=NULL && *RemoteName)
    for (int I=ConnectedList.Count()-1;I>=0;I--)
      if (ConnectedList [I].lpRemoteName && ConnectedList [I].lpLocalName!=NULL &&
        *ConnectedList [I].lpLocalName &&
        lstrcmpi(ConnectedList [I].lpRemoteName,RemoteName)==0)
      {
        if (ConnectedList [I].dwScope==RESOURCE_CONNECTED ||
          ConnectedList [I].dwScope==RESOURCE_REMEMBERED)
          CharToOem(ConnectedList [I].lpLocalName,LocalName);
        break;
      }
}


int NetBrowser::GetNameAndPassword(NameAndPassInfo* passInfo)
{
  struct InitDialogItem InitItems[]={
    {DI_DOUBLEBOX,3,1,72,10,0,0,0,0,""},
    {DI_TEXT,5,2,0,0,0,0,0,0,(char *)MNetUserName},
    {DI_EDIT,5,3,70,3,1,(DWORD_PTR)"NetworkUser",DIF_HISTORY|DIF_USELASTHISTORY,0,""},
    {DI_TEXT,5,4,0,0,0,0,0,0,(char *)MNetUserPassword},
    {DI_PSWEDIT,5,5,70,3,0,0,0,0,""},
    {DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
    {DI_CHECKBOX,5,7,0,0,0,0,(DWORD)DIF_DISABLE,0,(char *)MRememberPass},
    {DI_TEXT,3,8,0,8,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
    {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk},
    {DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel}
  };
  if(passInfo->pRemember)
  {
    InitItems[6].Flags &= ~DIF_DISABLE;
    InitItems[6].Selected = *passInfo->pRemember;//*pRemember;
  }
  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  static char LastName[256],LastPassword[256];
  if (passInfo->Title!=NULL)
    CharToOem(passInfo->Title,DialogItems[0].Data);
  lstrcpy(DialogItems[2].Data,LastName);
  lstrcpy(DialogItems[4].Data,LastPassword);
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,12,
    StrHelpNetBrowse,DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
  //if (ExitCode!=7)
  if (ExitCode!=(ARRAYLEN(DialogItems)-2))
    return(FALSE);
  if (passInfo->pRemember)
    *passInfo->pRemember = DialogItems[6].Selected;
  lstrcpy(LastName,DialogItems[2].Data);
  lstrcpy(LastPassword,DialogItems[4].Data);

  // Convert Name and Password to Ansi
  lstrcpy(passInfo->Name,DialogItems[2].Data);
  OemToChar(passInfo->Name,passInfo->Name);
  lstrcpy(passInfo->Password,DialogItems[4].Data);
  OemToChar(passInfo->Password,passInfo->Password);

  return(TRUE);
}


void NetBrowser::PutCurrentFileName (BOOL ToCommandLine)
{
  PanelInfo PInfo;
  Info.Control (this, FCTL_GETPANELINFO, &PInfo);
  if (PInfo.ItemsNumber > 0)
  {
    char CurFile [NM];
    lstrcpy (CurFile, PInfo.PanelItems [PInfo.CurrentItem].FindData.cFileName);
    if (!lstrcmp (CurFile, ".."))
    {
      if (PCurResource == NULL)
        lstrcpy (CurFile, ".\\");
      else
        CharToOem(PCurResource->lpRemoteName,CurFile);
    }
    FSF.QuoteSpaceOnly (CurFile);
    if (ToCommandLine)
    {
      lstrcat (CurFile, " ");
      Info.Control (this, FCTL_INSERTCMDLINE, CurFile);
    }
    else
      FSF.CopyToClipboard (CurFile);
  }
}


void NetBrowser::ManualConnect()
{
  PanelInfo PInfo;
  Info.Control (this, FCTL_GETPANELINFO, &PInfo);
  if (PInfo.ItemsNumber == 0)
    return;

  ChangeToDirectory (PInfo.PanelItems [PInfo.CurrentItem].FindData.cFileName,
    FALSE, TRUE);
}


void NetBrowser::GetRemoteName(NETRESOURCE *NetRes,char *RemoteName)
{
  if (CheckFavoriteItem(NetRes))
  {
    char buff[NM];
    if(!NetRes->lpRemoteName)
    {
      OemToChar(GetMsg(MFavorites), buff);
      lstrcpy(RemoteName, buff);
    }
    else
    {
      OemToChar(GetMsg(MFavoritesFolder), buff);
      free (NetRes->lpComment);
      NetRes->lpComment = NetResourceList::CopyText(buff);
      lstrcpy(RemoteName, PointToName(NetRes->lpRemoteName));
    }
  }
  else if (NetRes->lpProvider!=NULL && (NetRes->lpRemoteName==NULL ||
    NetRes->dwDisplayType==RESOURCEDISPLAYTYPE_NETWORK))
    lstrcpy(RemoteName,NetRes->lpProvider);
  else
    if (NetRes->lpRemoteName==NULL)
      *RemoteName=0;
    else if (Opt.FullPathShares)
      lstrcpy(RemoteName,NetRes->lpRemoteName);
    else
      lstrcpy(RemoteName,PointToName(NetRes->lpRemoteName));
}


BOOL NetBrowser::IsReadable(const char *Remote)
{
  char Mask[NM];
  FSF.sprintf(Mask,"%s\\*",Remote);
  HANDLE FindHandle;
  WIN32_FIND_DATA FindData;
  FindHandle=FindFirstFile(Mask,&FindData);
  DWORD err = GetLastError();
  FindClose(FindHandle);
  SetLastError(err);
  if(err == ERROR_FILE_NOT_FOUND)
  {
    SetLastError(0);
    return TRUE;
  }
  return(FindHandle!=INVALID_HANDLE_VALUE);
}

void NetBrowser::SetOpenFromCommandLine (char *ShareName)
{
  //lstrcpy (CmdLinePath, ShareName);
#ifdef NETWORK_LOGGING
  LogData("SetOpenFromCommandLine ShareName is");
  LogData(ShareName);
#endif
  lstrcpy(CmdLinePath, ShareName);

  /*if(!GotoFavorite(ShareName))
    GotoComputer(ShareName);*/
}

BOOL NetBrowser::SetOpenFromFilePanel (char *ShareName)
{
  NETRESOURCE nr;
  NetResourceList::InitNetResource (nr);

  char ShareNameANSI [NM];
  OemToChar (ShareName, ShareNameANSI);
  if (!GetResourceInfo (ShareNameANSI, &nr))
    return FALSE;
  if (!IsMSNetResource (nr))
    return FALSE;

  OpenFromFilePanel = TRUE;
  return TRUE;
}

int NetBrowser::GotoComputer (const char *Dir)
{
#ifdef NETWORK_LOGGING
  LogData("Entering GotoComputer");
#endif
  // if there are backslashes in the name, truncate them
  char ComputerName [NM];
  lstrcpy (ComputerName, Dir);
  BOOL IsShare = FALSE;

  char *p = strchr (ComputerName + 2, '\\'); // skip past leading backslashes
  if (p)
  {
    IsShare = TRUE;
    *p = '\0';
  }
  else {
    p = strchr (ComputerName + 2, '/');
    if (p)
    {
      IsShare = TRUE;
      *p = '\0';
    }
  }

  CharUpper(ComputerName);

  NETRESOURCE res;
  NetResourceList::InitNetResource (res);
  if (!GetResourceInfo (ComputerName, &res))
    return FALSE;
  /*
  if (!IsMSNetResource (res))
    return FALSE;
  */
  if (!IsResourceReadable(res))
  {
    int err = GetLastError();
    if (err == ERROR_INVALID_PASSWORD || err == ERROR_LOGON_FAILURE || err == ERROR_ACCESS_DENIED || err == ERROR_INVALID_HANDLE || err == ERROR_LOGON_TYPE_NOT_GRANTED)
      if(!((AddConnectionFromFavorites(&res)||AddConnectionExplicit(&res))&&IsResourceReadable (res)))
      {
        if(GetLastError() != ERROR_CANCELLED)
          Info.Message (Info.ModuleNumber, FMSG_WARNING|FMSG_ERRORTYPE|FMSG_MB_OK|FMSG_ALLINONE,
          NULL, (const char **) GetMsg (MError), 0, 0);
        return FALSE;
      }
  }

  CurResource = res;
  PCurResource = &CurResource;

  /*int result = */Info.Control (this, FCTL_UPDATEPANEL, NULL);

  if (IsShare)
  {
    char ShareName [NM];
    lstrcpy (ShareName, Dir);

    // replace forward slashes with backslashes
    for (p = ShareName; *p; p++)
      if (*p == '/')
        *p = '\\';

      SetCursorToShare (ShareName);
  }
  else
    Info.Control (this, FCTL_REDRAWPANEL, NULL);
  return TRUE;
}

void NetBrowser::GotoLocalNetwork()
{
  TSaveScreen ss;
  char ComputerName [NM];
  lstrcpy (ComputerName, "\\\\");
  DWORD ComputerNameLength = NM-3;
  if (!GetComputerName (ComputerName+2, &ComputerNameLength))
    return;

  NETRESOURCE res;
  NetResourceList::InitNetResource (res);

  if (!GetResourceInfo (ComputerName, &res) || !IsMSNetResource (res))
    return;

  NETRESOURCE parent;
  NetResourceList::InitNetResource (parent);
  if (!GetResourceParent (res, &parent))
    return;

  NetResourceList::CopyNetResource (CurResource, parent);
  PCurResource = &CurResource;

  Info.Control (this, FCTL_UPDATEPANEL, NULL);
  Info.Control (this, FCTL_REDRAWPANEL, NULL);
}


void NetBrowser::SetCursorToShare (char *Share)
{
  PanelInfo PInfo = {0};
  // this returns the items in sorted order, so we can position correctly
  Info.Control (this, FCTL_GETPANELINFO, &PInfo);
  if (!PInfo.ItemsNumber)
    return;

  // prevent recursion
  for (int i=0; i<PInfo.ItemsNumber; i++)
  {
    char szAnsiName[MAX_PATH];
    OemToChar(PInfo.PanelItems [i].FindData.cFileName, szAnsiName);
    if (!FSF.LStricmp (szAnsiName, Opt.FullPathShares?Share:PointToName(Share)))
    {
      PanelRedrawInfo info;
      info.CurrentItem = i;
      info.TopPanelItem = 0;
      Info.Control (this, FCTL_REDRAWPANEL, &info);
      break;
    }
  }
}


void WINAPI ExitFAR()
{
  if(!IsOldFAR)
  {
    delete CommonRootResources;
    NetResourceList::DeleteNetResource (CommonCurResource);
  }
}

void NetBrowser::RemoveItems()
{
  if(!CheckFavoriteItem(PCurResource))
    return;
  // We are in Favorites folder, so we can remove items from this folder
  struct PanelInfo PInfo;
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
  if(PInfo.SelectedItemsNumber <= 0) // Something strange is happen
    return;
  char szConfirmation[NM*2];
  if(PInfo.SelectedItemsNumber == 1)
    FSF.sprintf(szConfirmation, GetMsg(MRemoveFavItem), PInfo.SelectedItems[0].FindData.cFileName);
  else // PInfo.SelectedItemsNumber > 1
    FSF.sprintf(szConfirmation, GetMsg(MRemoveFavItems), PInfo.SelectedItemsNumber);

  char* Msg[4];
  Msg[0] = GetMsg(MRemoveFavCaption);
  Msg[1] = szConfirmation;
  Msg[2] = GetMsg(MOk);
  Msg[3] = GetMsg(MCancel);

  if(0 != Info.Message(Info.ModuleNumber, FMSG_WARNING, "RemoveItemFav", Msg,
    sizeof(Msg)/sizeof(Msg[0]), 2))
    return; // User canceled deletion
  char szName[MAX_PATH*2] = {0};
  OemToChar(PCurResource->lpRemoteName, szName);
  char* p = szName + lstrlen(szName);
  if((p>szName)&&(p[-1] != '\\'))
    *p++ = '\\';
  for(int i = 0; i < PInfo.SelectedItemsNumber; i++)
  {
    OemToChar(PInfo.SelectedItems[i].FindData.cFileName, p);
    RemoveFromFavorites(szName, NULL, NULL);
  }
  Info.Control (this, FCTL_UPDATEPANEL, NULL);
  Info.Control (this, FCTL_REDRAWPANEL, NULL);
}

void NetBrowser::CreateFavSubFolder()
{
  if(!CheckFavoriteItem(PCurResource))
    return;
  char buff[MAX_PATH];
  if(DlgCreateFolder(buff, sizeof(buff)))
  {
    if(!CreateSubFolder(PCurResource->lpRemoteName, buff))
    {
      ShowMessage("Failed to create folder");
      return;
    }
    Info.Control(this,FCTL_UPDATEPANEL,NULL);
    Info.Control(this,FCTL_REDRAWPANEL,NULL);
  }
}
