static int TrafficCallback(int Size);

static char LastHostsPath[1024];

extern int BusySocket;

static struct TrafficInformation
{
  char FileName[NM];
  char ConsoleTitle[100];
  long FileSize;
  long LastFileSize;
  long StartFileSize;
  long FullFileSize;
  clock_t FileStartTime;
  long TotalFiles;
  long TotalSize;
  long TotalSkipped;
  long TotalFullSize;
  clock_t TotalStartTime;
  int FirstDraw;
  int Upload;
  int ShowStatus;
} TrafficInfo;


FTP::FTP()
{
  *HostName=*Password=*HostDescr=0;
  AskLogin=FALSE;
  AsciiMode=0;
  PassiveMode=Opt.PassiveMode;
  UseFirewall=(*Opt.Firewall!=0);
  *HostTable=0;
  hConnect=NULL;
  ResetCache=TRUE;
  *PanelTitle=0;
  ShowHosts=TRUE;
  SwitchingToFTP=FALSE;
  strcpy(HostsPath,LastHostsPath);
  StartViewMode=-1;
  *DirToRestore=0;
  LastUsedTableNum=0;
  RereadRequired=FALSE;
}


FTP::~FTP()
{
  if (hConnect)
    FtpDisconnect(hConnect);
}


int FTP::Connect()
{
  if (hConnect)
  {
    FtpDisconnect(hConnect);
    hConnect=NULL;
  }

  if (*HostName==0)
    return(FALSE);

  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
  ConnectMessage(0,HostName);

  char UserName[NM],Host[NM],Home[NM],*HostStart,*NameEnd,*PswStart;
  if ((HostStart=strstr(HostName,"://"))==NULL)
    HostStart=HostName;
  else
    HostStart+=3;
  if ((NameEnd=strrchr(HostStart,'@'))!=NULL)
  {
    strcpy(UserName,HostStart);
    UserName[NameEnd-HostStart]=0;
    HostStart=NameEnd+1;
  }
  else
    *UserName=0;

  char CurPassword[256];
  *CurPassword=0;

  if ((PswStart=strrchr(UserName,':'))!=NULL)
  {
    strcpy(CurPassword,PswStart+1);
    *PswStart=0;
  }
  if (*CurPassword==0)
    if (*Password==0 && *Opt.DefaultPassword && (*UserName==0 || stricmp(UserName,"anonymous")==0))
    {
      if (!AskLogin)
        strcpy(CurPassword,Opt.DefaultPassword);
    }
    else
      strcpy(CurPassword,Password);
  strcpy(Host,HostStart);

  *Home=0;
  char *HomePtr=strchr(Host,'/');
  if (HomePtr!=NULL)
  {
    if (*Home==0)
    {
      strcpy(Home,HomePtr);
      int Length=strlen(Home);
      if (Length>1 && Home[Length-1]=='/')
        Home[Length-1]=0;
    }
    *HomePtr=0;
  }
  if (*DirToRestore)
    strcpy(Home,DirToRestore);

  if (AskLogin)
    if (!GetLoginData(UserName,CurPassword))
    {
      Info.RestoreScreen(hScreen);
      return(FALSE);
    }

  FtpResetLastReply();
  if (*Host)
  {
    if (UseFirewall && *Opt.Firewall)
    {
      char FirewallHost[512],FirewallUser[NM];
      sprintf(FirewallUser,"%s@%s",*UserName ? UserName:"anonymous",Host);

      strcpy(FirewallHost,Opt.Firewall);
      char *PortPtr=strrchr(FirewallHost,':');
      if (PortPtr!=NULL && PortPtr!=FirewallHost && *(PortPtr-1)!='/')
        *PortPtr=' ';
      FtpConnect(&hConnect,FirewallHost,FirewallUser,CurPassword);
    }
    else
    {
      char *PortPtr=strrchr(Host,':');
      if (PortPtr!=NULL)
        *PortPtr=' ';
      FtpConnect(&hConnect,Host,*UserName ? UserName:"anonymous",CurPassword);
    }
  }
  else
    hConnect=NULL;
  Info.RestoreScreen(hScreen);

  if (!hConnect)
  {
    char Reply1[512],Reply2[512];
    FtpLastReply(Reply1,sizeof(Reply1),Reply2,sizeof(Reply2));
    if (*Reply1 && *Reply2)
    {
      char *MsgItems[]={GetMsg(MFtpTitle),GetMsg(MCannotConnectTo),HostName,"\x1",Reply1,Reply2,GetMsg(MOk)};
      Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
    }
    else
      if (*Reply2)
      {
        char *MsgItems[]={GetMsg(MFtpTitle),GetMsg(MCannotConnectTo),HostName,"\x1",Reply2,GetMsg(MOk)};
        Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
      }
      else
      {
        char *MsgItems[]={GetMsg(MFtpTitle),GetMsg(MCannotConnectTo),HostName,GetMsg(MOk)};
        Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
      }
    return(FALSE);
  }

  char Table[100];
  if (*HostTable)
    strcpy(Table,HostTable);
  else
    GetRegKey(HKEY_CURRENT_USER,"","CharTable",Table,"",sizeof(Table));
  ((Connection *)hConnect)->SetTable(TableNameToValue(Table));
  ((Connection *)hConnect)->PassiveMode=PassiveMode;

  *DirToRestore=0;
  ShowHosts=FALSE;

/*
  struct PanelRedrawInfo RInfo;
  RInfo.CurrentItem=RInfo.TopPanelItem=0;
*/
  Info.Control(this,FCTL_REDRAWANOTHERPANEL,NULL);

  if (StartViewMode>=0)
    SwitchingToFTP=TRUE;

  if (*Home)
    SetDirectory(Home,0);

  SwitchingToFTP=FALSE;

  return(TRUE);
}


int FTP::GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  WIN32_FIND_DATA FindData;
  *pPanelItem=NULL;
  *pItemsNumber=0;

  if (ShowHosts)
  {
    *pPanelItem=new PluginPanelItem[1];
    PluginPanelItem *CurItem=*pPanelItem;
    memset(CurItem,0,sizeof(*CurItem));
    strcpy(CurItem->FindData.cFileName,"..");
    CurItem->CustomColumnData=new LPSTR[1];
    CurItem->CustomColumnData[0]=new char[3];
    strcpy(CurItem->CustomColumnData[0],"..");
    CurItem->CustomColumnNumber=1;

    *pItemsNumber=1;

    EnumHost Enum(HKEY_CURRENT_USER,HostsPath);

    char RegKey[1024],HostName[512];
    FILETIME LastWrite;
    while (Enum.GetNextHost(RegKey,HostName,&LastWrite))
    {
      char HostDescr[256];
      int ItemsNumber=*pItemsNumber;

      GetRegKey(HKEY_CURRENT_USER,RegKey,"Description",HostDescr,"",sizeof(HostDescr));
      PluginPanelItem *NewPanelItem=(PluginPanelItem *)realloc(*pPanelItem,(ItemsNumber+1)*sizeof(PluginPanelItem));
      if (NewPanelItem==NULL)
        break;
      *pPanelItem=NewPanelItem;

      memset(&FindData,0,sizeof(FindData));

      strcpy(FindData.cFileName,HostName);
      FindData.ftLastWriteTime=LastWrite;
      if (GetRegKey(HKEY_CURRENT_USER,RegKey,"Folder",0))
        FindData.dwFileAttributes=FA_DIREC;

      PluginPanelItem *CurItem=&NewPanelItem[ItemsNumber];
      memset(CurItem,0,sizeof(PluginPanelItem));

      CurItem->FindData=FindData;
      if (*HostDescr)
      {
        CurItem->Description=new char[strlen(HostDescr)+1];
        strcpy(CurItem->Description,HostDescr);
      }

      char *HostPtr=strnicmp(HostName,"ftp://",6)==0 ? HostName+6:HostName;
      char *ChPtr=strrchr(HostPtr,'@');
      if (ChPtr!=NULL)
        HostPtr=ChPtr+1;
      CurItem->CustomColumnData=new LPSTR[1];
      CurItem->CustomColumnData[0]=new char[strlen(HostPtr)+1];
      strcpy(CurItem->CustomColumnData[0],HostPtr);
      CurItem->CustomColumnNumber=1;

      (*pItemsNumber)++;
    }
    strcpy(LastHostsPath,HostsPath);
    return(TRUE);
  }

  if (hConnect==NULL)
    if (!Connect() || hConnect==NULL)
    {
      if (ShowHosts)
      {
        Info.Control(this,FCTL_UPDATEPANEL,NULL);
        Info.Control(this,FCTL_REDRAWPANEL,NULL);
      }
      return(TRUE);
    }

  if (StartViewMode>=0)
    SwitchingToFTP=TRUE;

  if ((OpMode & OPM_FIND)==0)
  {
    strcpy(PanelTitle,GetMsg(MReadingFTPFolder));
    Info.Control(this,FCTL_REDRAWPANEL,NULL);
    Info.Text(0,0,0,NULL);
  }

  DWORD Mode;
  char  Owner[256];
  int ExitCode=FtpFindFirstFile(hConnect,"*",&FindData,ResetCache,&Mode,Owner);

  int ConnectionLost=FALSE;
  if (!ExitCode)
  {
    int LastError=GetLastError();
    if (LastError!=ERROR_NO_MORE_FILES && LastError!=ERROR_BAD_COMMAND &&
        LastError!=ERROR_ACCESS_DENIED)
      ConnectionLost=TRUE;
  }

  *PanelTitle=0;

  ResetCache=FALSE;

  if (StartViewMode>=0 && !ConnectionLost)
  {
    Info.Control(this,FCTL_SETVIEWMODE,&StartViewMode);
    SwitchingToFTP=FALSE;
    StartViewMode=-2;
  }

  if (!ExitCode)
  {
    if (!ConnectionLost)
    {
      *pItemsNumber=0;
      return(TRUE);
    }

    char *MsgItems[]={GetMsg(MFtpTitle),HostName,GetMsg(MConnectionLost),GetMsg(MRestore),GetMsg(MCancel)};
    while (Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)==0)
      if (Connect())
        return(GetFindData(pPanelItem,pItemsNumber,OpMode));
    FtpDisconnect(hConnect);
    hConnect=NULL;
    if (ShowHosts)
    {
      SwitchingToFTP=FALSE;
      RereadRequired=TRUE;
      return(GetFindData(pPanelItem,pItemsNumber,OpMode));
    }
    else
      if (StartViewMode>=0)
      {
        Info.Control(this,FCTL_SETVIEWMODE,&StartViewMode);
        SwitchingToFTP=FALSE;
        StartViewMode=-2;
      }
    return(TRUE);
  }

  while (ExitCode)
  {
    FindData.dwFileAttributes&=0x3f;
    FILETIME NewTime;
    LocalFileTimeToFileTime(&FindData.ftLastWriteTime,&NewTime);
    FindData.ftLastWriteTime=NewTime;
    if (FindData.dwFileAttributes & FA_DIREC)
      FindData.nFileSizeHigh=FindData.nFileSizeLow=0;
    PluginPanelItem *NewPanelItem=*pPanelItem;
    if ((*pItemsNumber & 1023)==0)
    {
      NewPanelItem=(PluginPanelItem *)realloc(NewPanelItem,(*pItemsNumber+1024+1)*sizeof(PluginPanelItem));
      if (NewPanelItem==NULL)
        break;
      *pPanelItem=NewPanelItem;
    }

    PluginPanelItem *CurItem=&NewPanelItem[*pItemsNumber];

    memset(CurItem,0,sizeof(PluginPanelItem));
    CurItem->FindData=FindData;
    CurItem->UserData=Mode;

    if (*Owner)
      CurItem->Owner = strdup(Owner);

    char ModeText[20];
    for (int I=0;I<9;I++)
      if (Mode & (1<<I))
      {
        static char *mtxt="xwr";
        ModeText[9-I]=mtxt[I%3];
      }
      else
        ModeText[9-I]='-';
    if (FindData.dwFileAttributes & FA_DIREC)
      ModeText[0]='d';
    else
      ModeText[0]=(Mode & 0x200) ? 'l':'-';
    ModeText[10]=0;

    CurItem->CustomColumnData=new LPSTR[1];
    CurItem->CustomColumnData[0]=strdup(ModeText);
    CurItem->CustomColumnNumber=1;

    (*pItemsNumber)++;
    ExitCode=FtpFindNextFile(hConnect,&FindData,&Mode,Owner);
  }

  return(TRUE);
}


void FTP::FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber)
{
  for (int I=0;I<ItemsNumber;I++)
  {
    if (ShowHosts)
    {
      if (PanelItem[I].Description!=NULL)
        delete PanelItem[I].Description;
    }
    else
      if (PanelItem[I].Owner)
        delete PanelItem[I].Owner;
    if (PanelItem[I].CustomColumnNumber==1)
    {
      delete PanelItem[I].CustomColumnData[0];
      delete PanelItem[I].CustomColumnData;
    }
  }
  delete PanelItem;
}


int FTP::GetHost(char *HostName,char *Password,char *HostDescr,int &AskLogin,
                 int &AsciiMode,int &PassiveMode,int &UseFirewall,
                 char *HostTable)
{
  const char *HostHistory="FTPHost";
  struct InitDialogItem InitItems[]=
  {
    DI_DOUBLEBOX,3,1,72,15,0,0,0,0,(char *)(*HostName ? MEditFtpTitle:MEnterFtpTitle),
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MFtpName,
    DI_EDIT,5,3,70,3,1,(DWORD)HostHistory,DIF_HISTORY,0,"",
    DI_TEXT,5,4,0,0,0,0,0,0,(char *)MUserPassword,
    DI_PSWEDIT,5,5,70,3,0,0,0,0,"",
    DI_TEXT,5,6,0,0,0,0,0,0,(char *)MHostDescr,
    DI_EDIT,5,7,70,3,0,0,0,0,"",
    DI_TEXT,3,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MAskLogin,
    DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MAsciiMode,
    DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MPassiveMode,
    DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)MUseFirewall,
    DI_TEXT,3,13,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,1,(char *)MFtpSave,
    DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFtpConnect,
    DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFtpSelectTable,
    DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  strcpy(DialogItems[2].Data,HostName);
  strcpy(DialogItems[4].Data,Password);
  strcpy(DialogItems[6].Data,HostDescr);
  DialogItems[8].Selected=AskLogin;
  DialogItems[9].Selected=AsciiMode;
  DialogItems[10].Selected=PassiveMode;
  DialogItems[11].Selected=UseFirewall;

  int AskCode;
  char TableName[100];
  strcpy(TableName,HostTable);
  while (1)
  {
    AskCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,17,"FTPConnect",DialogItems,
                        sizeof(DialogItems)/sizeof(DialogItems[0]));
    if (AskCode==15)
    {
      SelectHostTable(TableName);
      DialogItems[15].Focus=FALSE;
      DialogItems[2].Focus=TRUE;
    }
    else
      break;
  }
  if (AskCode!=13 && AskCode!=14 || *DialogItems[2].Data==0)
    return(FALSE);
  strcpy(HostName,DialogItems[2].Data);
  strcpy(Password,DialogItems[4].Data);
  strcpy(HostDescr,DialogItems[6].Data);
  AskLogin=DialogItems[8].Selected;
  AsciiMode=DialogItems[9].Selected;
  PassiveMode=DialogItems[10].Selected;
  UseFirewall=DialogItems[11].Selected;
  strcpy(HostTable,TableName);

  for (int I=0;HostName[I]!=0;I++)
    if (HostName[I]=='\\')
      HostName[I]='/';
  if (AskCode==14)
  {
    strcpy(FTP::HostName,HostName);
    strcpy(FTP::Password,Password);
    strcpy(FTP::HostDescr,HostDescr);
    FTP::AskLogin=AskLogin;
    FTP::AsciiMode=AsciiMode;
    FTP::PassiveMode=PassiveMode;
    FTP::UseFirewall=UseFirewall;
    strcpy(FTP::HostTable,HostTable);
    if (Connect())
    {
      SwitchingToFTP=TRUE;
      Info.Control(this,FCTL_UPDATEPANEL,NULL);
      SwitchingToFTP=FALSE;
      if (ShowHosts)
      {
        Info.Control(this,FCTL_UPDATEPANEL,NULL);
        Info.Control(this,FCTL_REDRAWPANEL,NULL);
      }
      else
      {
        struct PanelRedrawInfo RInfo;
        RInfo.CurrentItem=RInfo.TopPanelItem=0;
        Info.Control(this,FCTL_REDRAWPANEL,&RInfo);
      }
    }
    return(FALSE);
  }
  return(TRUE);
}


void FTP::EncryptPassword(char *Src,char *Dest)
{
  clock_t Random=clock();
  for (int I=0;I<256;I++)
  {
    char *CurDest=Dest;
    CurDest[0]=(Random+I)|0x80;
    CurDest[1]=(Random>>8)|0x80;
    strcpy(CurDest+2,Src);
    unsigned char XorMask=(CurDest[0]^CurDest[1])|80;
    CurDest+=2;
    int Done=TRUE;
    while (*CurDest)
    {
      *CurDest^=XorMask;
      if (*CurDest==0)
      {
        Done=FALSE;
        break;
      }
      CurDest++;
    }
    if (Done)
      break;
  }
}


void FTP::DecryptPassword(char *Src,char *Dest)
{
  unsigned char XorMask=(Src[0]^Src[1])|80;
  strcpy(Dest,Src+2);
  while (*Dest)
    *(Dest++)^=XorMask;
}


void FTP::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  struct PanelInfo PInfo;

  if (*PanelTitle==0)
    ::Info.Control(this,FCTL_GETANOTHERPANELINFO,&PInfo);

  if (StartViewMode==-1)
  {
    struct PanelInfo PInfo;
    ::Info.Control(this,FCTL_GETPANELINFO,&PInfo);
    StartViewMode=PInfo.ViewMode;
  }

  Info->StructSize=sizeof(*Info);
  if (ShowHosts)
    Info->Flags=OPIF_ADDDOTS|OPIF_SHOWPRESERVECASE|OPIF_USEATTRHIGHLIGHTING;
  else
    Info->Flags=OPIF_USEFILTER|OPIF_USESORTGROUPS|OPIF_USEHIGHLIGHTING|
                OPIF_ADDDOTS|OPIF_SHOWPRESERVECASE;
  Info->HostFile=NULL;
  if (ShowHosts)
    Info->CurDir=HostsPath;
  else
    if (hConnect)
    {
      static char CurDir[NM];
      DWORD Size=sizeof(CurDir);
      FtpGetCurrentDirectory(hConnect,CurDir,&Size);
      Info->CurDir=CurDir;
    }
    else
      Info->CurDir="";

  static char Title[512];
  Info->Format="FTP";
  if (*PanelTitle)
    sprintf(Title," %s ",PanelTitle);
  else
  {
    char *HostStart;
    if ((HostStart=strstr(HostName,"://"))==NULL)
      HostStart=HostName;
    else
      HostStart+=3;
    sprintf(Title," FTP:%s",HostStart);
    char *Slash=strchr(Title,'/');
    if (Slash!=NULL)
      *Slash=0;
    if (*Info->CurDir)
    {
      if (*Info->CurDir!='\\' && *Info->CurDir!='/')
        strcat(Title,"\\");
      strcat(Title,Info->CurDir);
    }
    strcat(Title," ");
  }

  Info->PanelTitle=Title;

  static struct InfoPanelLine InfoLines[8];

  memset(InfoLines,0,sizeof(InfoLines));
  strcpy(InfoLines[0].Text,GetMsg(MFtpInfoFTPClient));
  InfoLines[0].Separator=TRUE;
  strcpy(InfoLines[1].Text,GetMsg(MFtpInfoHostName));
  strcpy(InfoLines[1].Data,HostName);
  strcpy(InfoLines[2].Text,GetMsg(MFtpInfoHostDescr));
  strcpy(InfoLines[2].Data,HostDescr);
  strcpy(InfoLines[3].Text,GetMsg(MFtpInfoHostType));
  if (hConnect)
    FtpSystemInfo(hConnect,InfoLines[3].Data,sizeof(InfoLines[2].Data));
  strcpy(InfoLines[5].Text,GetMsg(MFtpInfoFtpTitle));
  InfoLines[5].Separator=TRUE;
  if (hConnect)
    FtpStartReply(hConnect,InfoLines[6].Data,sizeof(InfoLines[6].Data));

  int Width=PInfo.PanelRect.right-PInfo.PanelRect.left+1;

  int MaxWidth=Width-5-strlen(InfoLines[3].Text);

  if (MaxWidth<0)
    MaxWidth=0;

  if (MaxWidth>0 && Width>10 && strlen(InfoLines[3].Data)>MaxWidth)
  {
    sprintf(InfoLines[4].Data,"%-*s",Width-5,&InfoLines[3].Data[MaxWidth]);
    InfoLines[3].Data[MaxWidth]=0;
  }

  MaxWidth=Width-5;

  if (Width>10 && strlen(InfoLines[6].Data)>MaxWidth)
  {
    sprintf(InfoLines[7].Data,"%-*s",MaxWidth,&InfoLines[6].Data[MaxWidth]);
    InfoLines[6].Data[MaxWidth]=0;
  }
  Info->InfoLines=InfoLines;
  Info->InfoLinesNumber=sizeof(InfoLines)/sizeof(InfoLines[0]);
  static char *DescrFiles[32],DescrFilesString[256];
  strcpy(DescrFilesString,Opt.DescriptionNames);
  int DescrFilesNumber=0;
  char *NamePtr=DescrFilesString;
  while (DescrFilesNumber<sizeof(DescrFiles)/sizeof(DescrFiles[0]))
  {
    while (isspace(*NamePtr))
      NamePtr++;
    if (*NamePtr==0)
      break;
    DescrFiles[DescrFilesNumber++]=NamePtr;
    if ((NamePtr=strchr(NamePtr,','))==NULL)
      break;
    *(NamePtr++)=0;
  }
  Info->DescrFiles=DescrFiles;
  if (!Opt.ReadDescriptions)
    Info->DescrFilesNumber=0;
  else
    Info->DescrFilesNumber=DescrFilesNumber;
  static char ShortcutData[1024];
  sprintf(ShortcutData,"%s\x1%s\x2%d",HostName,Password,AskLogin);
  Info->ShortcutData=ShortcutData;

  static struct KeyBarTitles KeyBar;
  memset(&KeyBar,0,sizeof(KeyBar));

  if (ShowHosts || SwitchingToFTP)
  {
    int DizLength=0;

    EnumHost Enum(HKEY_CURRENT_USER,HostsPath);

    char RegKey[1024];
    while (Enum.GetNextHost(RegKey))
    {
      char HostDescr[256];
      GetRegKey(HKEY_CURRENT_USER,RegKey,"Description",HostDescr,"",sizeof(HostDescr));
      int Length=strlen(HostDescr);
      if (Length>DizLength)
        DizLength=Length;
    }

    static struct PanelMode PanelModesArray[10];

    static char *ColumnTitles[2];
    ColumnTitles[0]=GetMsg(MHostColumn);
    ColumnTitles[1]=NULL;

    PanelModesArray[1].ColumnTypes="N";
    PanelModesArray[1].ColumnWidths="0";
    if (DizLength==0)
      PanelModesArray[2].ColumnTypes="C0";
    else
      PanelModesArray[2].ColumnTypes="C0,Z";
    PanelModesArray[2].ColumnWidths="0,0";
    PanelModesArray[2].ColumnTitles=ColumnTitles;
    PanelModesArray[3].ColumnTypes="C0";
    PanelModesArray[3].ColumnWidths="0";
    PanelModesArray[3].ColumnTitles=ColumnTitles;

    struct PanelInfo PInfo;
    ::Info.Control(this,FCTL_GETPANELINFO,&PInfo);

    if ((PInfo.PanelRect.right-PInfo.PanelRect.left)/2>DizLength)
    {
      static char Lengths[50];
      sprintf(Lengths,"0,%d",DizLength);
      PanelModesArray[2].ColumnWidths=Lengths;
    }

    PanelModesArray[2].FullScreen=FALSE;

    Info->PanelModesArray=PanelModesArray;
    Info->PanelModesNumber=sizeof(PanelModesArray)/sizeof(PanelModesArray[0]);
    Info->StartPanelMode='2';

    KeyBar.ShiftTitles[1-1]=KeyBar.ShiftTitles[2-1]=KeyBar.ShiftTitles[3-1]="";
    KeyBar.ShiftTitles[4-1]=ShowHosts ? GetMsg(MShiftF4):NULL;
  }
  else
  {
    KeyBar.ShiftTitles[7-1]=GetMsg(MShiftF7);

    static struct PanelMode PanelModesArray[10];
    static char *ColumnTitles[3];
    ColumnTitles[0]=NULL;
    ColumnTitles[1]=NULL;
    ColumnTitles[2]=GetMsg(MFileMode);
    PanelModesArray[9].ColumnTypes="N,S,C0";
    PanelModesArray[9].ColumnWidths="0,0,10";
    PanelModesArray[9].ColumnTitles=ColumnTitles;
    Info->PanelModesArray=PanelModesArray;
    Info->PanelModesNumber=sizeof(PanelModesArray)/sizeof(PanelModesArray[0]);
  }
  KeyBar.AltTitles[6-1]=GetMsg(MAltF6);
  Info->KeyBar=&KeyBar;
}


int FTP::SetDirectory(char *Dir,int OpMode)
{
  if (ShowHosts)
  {
    if (strcmp(Dir,"..")==0)
    {
      char *Slash=strrchr(HostsPath,'\\');
      if (Slash!=NULL)
        *Slash=0;
      else
        *HostsPath=0;
      return(TRUE);
    }
    else
    {
      char RegKey[1024];
      GetFullKey(RegKey,Dir);
      if (GetRegKey(HKEY_CURRENT_USER,RegKey,"Folder",0))
      {
        if (*HostsPath)
          strcat(HostsPath,"\\");
        strcat(HostsPath,Dir);
        return(TRUE);
      }
    }
    return(FALSE);
  }
  if (!hConnect)
    return(FALSE);
  if (OpMode & OPM_FIND)
  {
    char OldDir[NM],NewDir[NM];
    DWORD Size=sizeof(OldDir);
    FtpGetCurrentDirectory(hConnect,OldDir,&Size);
    if (!FtpSetCurrentDirectory(hConnect,Dir))
      return(FALSE);
    FtpGetCurrentDirectory(hConnect,NewDir,&Size);
    return(strcmp(OldDir,NewDir)!=0);
  }
  else
  {
    strcpy(PanelTitle,GetMsg(MChangingFTPFolder));
    Info.Control(this,FCTL_REDRAWPANEL,NULL);
    Info.Text(0,0,0,NULL);
    int ExitCode=FtpSetCurrentDirectory(hConnect,Dir);
    if (ExitCode)
    {
      DWORD Size=sizeof(DirToRestore);
      FtpGetCurrentDirectory(hConnect,DirToRestore,&Size);
    }
    *PanelTitle=0;
    Info.Control(this,FCTL_REDRAWPANEL,NULL);
    Info.Text(0,0,0,NULL);
    return(ExitCode);
  }
}


static int TrafficCallback(int Size)
{
  if (CheckForEsc())
    return(FALSE);
  static clock_t PrevTime=0;
  clock_t CurTime=clock();

  if (TrafficInfo.FileSize==0)
  {
    TrafficInfo.StartFileSize=Size;
    TrafficInfo.TotalSkipped+=Size;
  }
  if (CurTime-PrevTime>300)
  {
    TruncStr(TrafficInfo.FileName,24);

    char ConsoleTitle[100];
    char *MsgTitle=TrafficInfo.Upload ? GetMsg(MStatusUpload):GetMsg(MStatusDownload);
    int TotalPercent=ToPercent(TrafficInfo.TotalSize,TrafficInfo.TotalFullSize);

    sprintf(ConsoleTitle,"%s {%d%%} - Far",MsgTitle,TotalPercent);
    if (strcmp(ConsoleTitle,TrafficInfo.ConsoleTitle)!=0)
    {
      if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
        OemToChar(ConsoleTitle,ConsoleTitle);
      SetConsoleTitle(ConsoleTitle);
      strcpy(TrafficInfo.ConsoleTitle,ConsoleTitle);
    }

    if (!TrafficInfo.ShowStatus)
    {
      char OutText[512];
      sprintf(OutText,"%s: %26s",GetMsg(MStatusDownload),TrafficInfo.FileName);
      Info.Text(2,0,LIGHTGRAY,OutText);
      return(TRUE);
    }
    PrevTime=CurTime;
    char Msg[10][200];
    char *MsgItems[]={MsgTitle,Msg[0],Msg[1],"\1",Msg[2],Msg[3]};

    clock_t dtime=(CurTime-TrafficInfo.FileStartTime)/1000;
    if (dtime<1) dtime=1;
    long FilePartSize=TrafficInfo.FileSize-TrafficInfo.StartFileSize;
    int CPS=(FilePartSize>0) ? FilePartSize/dtime:0;
    int Percent=ToPercent(TrafficInfo.FileSize,TrafficInfo.FullFileSize);

    sprintf(Msg[0]," %-24s %9d /%9d %8d %% %*s",TrafficInfo.FileName,
            TrafficInfo.FileSize,TrafficInfo.FullFileSize,Percent,
            strlen(GetMsg(MStatusCPS)),"");

    clock_t esttime=0;
    if (CPS!=0)
      esttime+=(TrafficInfo.FullFileSize-TrafficInfo.FileSize)/CPS;
    if (esttime>=0 && esttime<=3600*99)
      sprintf(Msg[1],"%-25s %02u:%02u:%02u / %02u:%02u:%02u %8d %s",
              GetMsg(MStatusTime),dtime/3600,(dtime/60)%60,dtime%60,
              esttime/3600,(esttime/60)%60,esttime%60,CPS,GetMsg(MStatusCPS));
    else
      *Msg[1]=0;

    dtime=(CurTime-TrafficInfo.TotalStartTime)/1000;
    if (dtime<1) dtime=1;
    FilePartSize=TrafficInfo.TotalSize-TrafficInfo.TotalSkipped;
    CPS=(FilePartSize>0) ? FilePartSize/dtime:0;

    sprintf(Msg[2]," %-24s %9d /%9d %8d %% %*s",GetMsg(MStatusTotal),
            TrafficInfo.TotalSize,TrafficInfo.TotalFullSize,TotalPercent,
            strlen(GetMsg(MStatusCPS)),"");

    esttime=0;
    if (CPS!=0)
      esttime+=(TrafficInfo.TotalFullSize-TrafficInfo.TotalSize)/CPS;
    if (esttime>=0 && esttime<=3600*99)
      sprintf(Msg[3],"%-25s %02u:%02u:%02u / %02u:%02u:%02u %8d %s",
              GetMsg(MStatusTime),dtime/3600,(dtime/60)%60,dtime%60,
              esttime/3600,(esttime/60)%60,esttime%60,CPS,GetMsg(MStatusCPS));
    else
      *Msg[3]=0;

    Info.Message(Info.ModuleNumber,TrafficInfo.FirstDraw ? 0:FMSG_KEEPBACKGROUND,
                 NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
    TrafficInfo.FirstDraw=FALSE;
  }
  int SizeInc=Size-TrafficInfo.LastFileSize;
  TrafficInfo.LastFileSize=Size;
  TrafficInfo.FileSize=Size;
  TrafficInfo.TotalSize+=SizeInc;
  return(TRUE);
}


int FTP::GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                  int Move,char *DestPath,int OpMode)
{
  if (ItemsNumber==0)
    return(0);
  if (*DestPath)
    AddEndSlash(DestPath);

  if (ShowHosts)
    return(GetHostFiles(PanelItem,ItemsNumber,Move,DestPath,OpMode));
  if (!hConnect)
    return(0);


  const char *GetHistory="FTPGet";
  struct InitDialogItem InitItems[]={
    DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MDownloadTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MDownloadTo,
    DI_EDIT,5,3,70,3,1,(DWORD)GetHistory,DIF_HISTORY,0,DestPath,
    DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MDownloadAscii,
    DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MDownload,
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MDownloadCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  int AsciiMode=FTP::AsciiMode;
  if ((OpMode & OPM_SILENT)==0)
  {
    DialogItems[4].Selected=AsciiMode;
    int AskCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,10,"FTPCmd",DialogItems,
                            sizeof(DialogItems)/sizeof(DialogItems[0]));
    if (AskCode!=6)
      return(-1);
    AsciiMode=DialogItems[4].Selected;
    strcpy(DestPath,DialogItems[2].Data);
  }
  if (*DestPath==0)
    return(-1);

  int Rename=ItemsNumber==1 && Move && strpbrk(DestPath,":\\/")==NULL;
  if (!Rename)
    AddEndSlash(DestPath);

  struct PluginPanelItem *FullItemsList;
  int FullItemsNumber;
  if (Rename)
  {
    FullItemsList=new PluginPanelItem[1];
    *FullItemsList=*PanelItem;
    FullItemsNumber=1;
  }
  else
    if (!ExpandList(PanelItem,ItemsNumber,&FullItemsList,&FullItemsNumber,TRUE))
      return(0);

  SaveConsoleTitle SaveTitle;
  memset(&TrafficInfo,0,sizeof(TrafficInfo));
  TrafficInfo.FirstDraw=TRUE;
  TrafficInfo.Upload=FALSE;
  TrafficInfo.ShowStatus=(OpMode & OPM_FIND)==0;
  TrafficInfo.TotalStartTime=clock();
  for (int I=0;I<FullItemsNumber;I++)
    if ((FullItemsList[I].FindData.dwFileAttributes & FA_DIREC)==0)
      TrafficInfo.TotalFullSize+=FullItemsList[I].FindData.nFileSizeLow;

  int OverwriteAll=FALSE,SkipAll=FALSE;
  for (int I=0;I<FullItemsNumber;I++)
  {
    char DestName[NM],*CurName;
    struct PluginPanelItem *CurPanelItem=&FullItemsList[I];
    CurName=CurPanelItem->FindData.cFileName;
    int Reget=FALSE;
    DWORD DestAttr=0xFFFFFFFF;
    if (Rename)
    {
      strcpy(DestName,DestPath);
      WIN32_FIND_DATA FindData;
      if (FtpFindFirstFile(hConnect,DestName,&FindData,FALSE,NULL,NULL))
        if (stricmp(FindData.cFileName,DestName)==0)
          DestAttr=FindData.dwFileAttributes;
    }
    else
    {
      sprintf(DestName,"%s%s",DestPath,CurName);
      if (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if (CreateDirectory(DestName,NULL))
          if (I<ItemsNumber && Opt.UpdateDescriptions)
            PanelItem[I].Flags|=PPIF_PROCESSDESCR;
        continue;
      }
      DestAttr=GetFileAttributes(DestName);
    }
    if ((OpMode & OPM_SILENT)==0 && !OverwriteAll && DestAttr!=0xFFFFFFFF)
    {
      if (SkipAll)
        continue;
      char *MsgItems[]={Rename ? GetMsg(MRenameTitle):GetMsg(MDownloadTitle),
        (DestAttr & FA_RDONLY) ? GetMsg(MAlreadyExistRO):GetMsg(MAlreadyExist),
        DestName,GetMsg(MAskOverwrite),GetMsg(MOverwrite),
        GetMsg(MOverwriteAll),GetMsg(MCopySkip),GetMsg(MCopySkipAll),
        GetMsg(MCopyResume),GetMsg(MCopyCancel)};
      int MsgCode=Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),6);
      switch(MsgCode)
      {
        case 1:
          OverwriteAll=TRUE;
          break;
        case 3:
          SkipAll=TRUE;
        case 2:
          TrafficInfo.TotalFullSize-=CurPanelItem->FindData.nFileSizeLow;
          continue;
        case 4:
          Reget=TRUE;
          break;
        case -1:
        case 5:
          delete FullItemsList;
          return(-1);
      }
      if (DestAttr & (FA_RDONLY|FA_HIDDEN))
        SetFileAttributes(DestName,DestAttr & ~(FA_RDONLY|FA_HIDDEN));
    }
    TrafficInfo.FullFileSize=CurPanelItem->FindData.nFileSizeLow;
    TrafficInfo.FileSize=0;
    TrafficInfo.LastFileSize=0;
    TrafficInfo.FileStartTime=clock();
    strcpy(TrafficInfo.FileName,CurName);
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    if (Rename)
    {
      int ExitCode=FtpRenameFile(hConnect,CurName,DestName);
      delete FullItemsList;
      return(ExitCode);
    }
    else
      if (FtpGetFile(hConnect,CurName,DestName,Reget,AsciiMode,TrafficCallback))
      {
        if (I<ItemsNumber)
        {
          PanelItem[I].Flags&=~PPIF_SELECTED;
          if (Opt.UpdateDescriptions)
            PanelItem[I].Flags|=PPIF_PROCESSDESCR;
        }
        FILE *SrcFile=fopen(DestName,"r+b");
        if (SrcFile!=NULL)
        {
          if (OpMode & OPM_DESCR)
          {
            int TableNum=((Connection *)hConnect)->TableNum;
            long FileSize=filelen(SrcFile);
            char *Buf=new char[FileSize];
            int ReadSize=fread(Buf,1,FileSize,SrcFile);
            int UseTable=FALSE;
            CharTableSet TableSet;
            if (TableNum==2)
            {
              int TabNum=Info.CharTable(FCT_DETECT,Buf,ReadSize);
              if (TabNum!=-1)
                if (Info.CharTable(TabNum,(char *)&TableSet,sizeof(TableSet))!=-1)
                {
                  UseTable=TRUE;
                  LastUsedTableNum=TabNum;
                }
            }
            if (TableNum>2)
              if (Info.CharTable(TableNum-3,(char *)&TableSet,sizeof(TableSet))!=-1)
              {
                UseTable=TRUE;
                LastUsedTableNum=TableNum-3;
              }
            switch(TableNum)
            {
              case 0:
                CharToOemBuff(Buf,Buf,ReadSize);
                break;
              case 1:
                break;
              default:
                if (UseTable)
                  for (int I=0;I<ReadSize;I++)
                    Buf[I]=TableSet.DecodeTable[Buf[I]];
                break;
            }
            fflush(SrcFile);
            fseek(SrcFile,0,SEEK_SET);
            fflush(SrcFile);
            fwrite(Buf,1,ReadSize,SrcFile);
            fflush(SrcFile);
            delete Buf;
          }
          HANDLE FileHandle=(HANDLE)_get_osfhandle(fileno(SrcFile));
          SetFileTime(FileHandle,NULL,NULL,&CurPanelItem->FindData.ftLastWriteTime);
          fclose(SrcFile);
        }
        if (Move)
          if (FtpDeleteFile(hConnect,CurName))
          {
            if (Opt.UpdateDescriptions && I<ItemsNumber)
              PanelItem[I].Flags|=PPIF_PROCESSDESCR;
          }
          else
          {
            char *MsgItems[]={GetMsg(MFtpTitle),GetMsg(MCannotDelete),CurName,GetMsg(MOk)};
            Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
            delete FullItemsList;
            return(0);
          }
      }
      else
      {
        if (GetLastError()!=ERROR_CANCELLED)
        {
          char *MsgItems[]={GetMsg(MError),GetMsg(MCannotDownload),CurName,GetMsg(MOk)};
          Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_DOWN|FMSG_ERRORTYPE,NULL,
                       MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
        }
        delete FullItemsList;
        return(0);
      }
  }
  if (Move)
    for (int I=FullItemsNumber-1;I>=0;I--)
    {
      if (CheckForEsc())
      {
        delete FullItemsList;
        return(-1);
      }
      char *CurName=FullItemsList[I].FindData.cFileName;
      if (FullItemsList[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        if (FtpRemoveDirectory(hConnect,CurName))
          if (I<ItemsNumber)
          {
            PanelItem[I].Flags&=~PPIF_SELECTED;
            if (Opt.UpdateDescriptions)
              PanelItem[I].Flags|=PPIF_PROCESSDESCR;
          }
    }
  delete FullItemsList;
  return(1);
}


int FTP::PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                  int Move,int OpMode)
{
  if (ItemsNumber==0)
    return(0);

  if (ShowHosts)
    return(PutHostsFiles(PanelItem,ItemsNumber,Move,OpMode));

  if (!hConnect)
    return(0);

  char StartPath[NM],DestPath[NM];
  DWORD Size=sizeof(StartPath);
  FtpGetCurrentDirectory(hConnect,StartPath,&Size);
  strcpy(DestPath,StartPath);

  const char *PutHistory="FTPPut";
  struct InitDialogItem InitItems[]={
    DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MUploadTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MUploadTo,
    DI_EDIT,5,3,70,3,1,(DWORD)PutHistory,DIF_HISTORY,0,DestPath,
    DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MUploadAscii,
    DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MUpload,
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MUploadCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  int AsciiMode=FTP::AsciiMode;
  if ((OpMode & OPM_SILENT)==0 && Opt.ShowUploadDialog)
  {
    DialogItems[4].Selected=AsciiMode;
    int AskCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,10,"FTPCmd",DialogItems,
                            sizeof(DialogItems)/sizeof(DialogItems[0]));
    if (AskCode!=6)
      return(-1);
    AsciiMode=DialogItems[4].Selected;
    strcpy(DestPath,DialogItems[2].Data);
  }

  if (strcmp(DestPath,StartPath)==0)
    *DestPath=0;
  if (*DestPath)
    AddEndSlash(DestPath);

  struct PluginPanelItem *FullItemsList;
  int FullItemsNumber;
  if (!ExpandList(PanelItem,ItemsNumber,&FullItemsList,&FullItemsNumber,FALSE))
    return(0);

  SaveConsoleTitle SaveTitle;
  memset(&TrafficInfo,0,sizeof(TrafficInfo));
  TrafficInfo.FirstDraw=TRUE;
  TrafficInfo.Upload=TRUE;
  TrafficInfo.ShowStatus=TRUE;
  TrafficInfo.TotalStartTime=clock();
  for (int I=0;I<FullItemsNumber;I++)
    if ((FullItemsList[I].FindData.dwFileAttributes & FA_DIREC)==0)
      TrafficInfo.TotalFullSize+=FullItemsList[I].FindData.nFileSizeLow;

  int OverwriteAll=FALSE,SkipAll=FALSE;
  for (int I=0;I<FullItemsNumber;I++)
  {
    char DestName[NM],*CurName;
    CurName=FullItemsList[I].FindData.cFileName;
    DWORD SrcAttr=FullItemsList[I].FindData.dwFileAttributes;
    if (Opt.UploadLowCase && (SrcAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
    {
      char *Name=PointToName(CurName);
      if (!IsCaseMixed(Name))
        LocalLower(Name);
    }
    sprintf(DestName,"%s%s",DestPath,CurName);
    if (strchr(StartPath,'\\')==NULL)
      for (int I=0;DestName[I];I++)
        if (DestName[I]=='\\')
          DestName[I]='/';
    if (SrcAttr & FILE_ATTRIBUTE_DIRECTORY)
    {
      if (FtpCreateDirectory(hConnect,DestName))
        if (I<ItemsNumber && Opt.UpdateDescriptions)
          PanelItem[I].Flags|=PPIF_PROCESSDESCR;
      continue;
    }
    int Reget=FALSE;
    DWORD DestAttr=0xFFFFFFFF,DestSize=0;
    WIN32_FIND_DATA FindData;
    if (FtpFindFirstFile(hConnect,DestName,&FindData,FALSE,NULL,NULL))
      if (stricmp(PointToName(FindData.cFileName),PointToName(DestName))==0)
      {
        DestAttr=FindData.dwFileAttributes;
        DestSize=FindData.nFileSizeLow;
      }

    if ((OpMode & OPM_SILENT)==0 && !OverwriteAll && DestAttr!=0xFFFFFFFF)
    {
      if (SkipAll)
        continue;
      char *MsgItems[]={GetMsg(MUploadTitle),
        (DestAttr & FA_RDONLY) ? GetMsg(MAlreadyExistRO):GetMsg(MAlreadyExist),
        DestName,GetMsg(MAskOverwrite),GetMsg(MOverwrite),
        GetMsg(MOverwriteAll),GetMsg(MCopySkip),GetMsg(MCopySkipAll),
        GetMsg(MCopyResume),GetMsg(MCopyCancel)};
      int MsgCode=Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,
                               sizeof(MsgItems)/sizeof(MsgItems[0]),6);
      switch(MsgCode)
      {
        case 1:
          OverwriteAll=TRUE;
          break;
        case 3:
          SkipAll=TRUE;
        case 2:
          TrafficInfo.TotalFullSize-=FullItemsList[I].FindData.nFileSizeLow;
          continue;
        case 4:
          Reget=TRUE;
          break;
        case -1:
        case 5:
          delete FullItemsList;
          return(-1);
      }
    }
    TrafficInfo.FullFileSize=FullItemsList[I].FindData.nFileSizeLow;
    TrafficInfo.FileSize=0;
    TrafficInfo.LastFileSize=0;
    TrafficInfo.FileStartTime=clock();
    strcpy(TrafficInfo.FileName,CurName);
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    if (OpMode & OPM_DESCR)
    {
      FILE *SrcFile=fopen(CurName,"r+b");
      if (SrcFile!=NULL)
      {
        int TableNum=((Connection *)hConnect)->TableNum;
        long FileSize=filelen(SrcFile);
        char *Buf=new char[FileSize];
        int ReadSize=fread(Buf,1,FileSize,SrcFile);
        switch(TableNum)
        {
          case 0:
            OemToCharBuff(Buf,Buf,ReadSize);
            break;
          case 1:
            break;
          default:
            {
              CharTableSet TableSet;
              if (Info.CharTable(LastUsedTableNum,(char *)&TableSet,sizeof(TableSet))!=-1)
                for (int I=0;I<ReadSize;I++)
                  Buf[I]=TableSet.EncodeTable[Buf[I]];
            }
            break;
        }
        fflush(SrcFile);
        fseek(SrcFile,0,SEEK_SET);
        fflush(SrcFile);
        fwrite(Buf,1,ReadSize,SrcFile);
        fflush(SrcFile);
        fclose(SrcFile);
        delete Buf;
      }
    }
    if (FtpPutFile(hConnect,CurName,DestName,Reget ? DestSize:0,
                   AsciiMode,TrafficCallback))
    {
      if (I<ItemsNumber)
      {
        PanelItem[I].Flags&=~PPIF_SELECTED;
        if (Opt.UpdateDescriptions)
          PanelItem[I].Flags|=PPIF_PROCESSDESCR;
      }
      if (Move)
      {
        SetFileAttributes(CurName,0);
        remove(CurName);
      }
    }
    else
    {
      if (GetLastError()!=ERROR_CANCELLED)
      {
        char *MsgItems[]={GetMsg(MError),GetMsg(MCannotUpload),CurName,GetMsg(MOk)};
        Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_DOWN|FMSG_ERRORTYPE,
                     NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
      }
      delete FullItemsList;
      return(0);
    }
  }

  if (Move)
    for (int I=FullItemsNumber-1;I>=0;I--)
    {
      if (CheckForEsc())
      {
        delete FullItemsList;
        return(-1);
      }
      char *CurName=FullItemsList[I].FindData.cFileName;
      if (FullItemsList[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        if (RemoveDirectory(CurName))
          if (I<ItemsNumber)
          {
            PanelItem[I].Flags&=~PPIF_SELECTED;
            if (Opt.UpdateDescriptions)
              PanelItem[I].Flags|=PPIF_PROCESSDESCR;
          }
    }

  delete FullItemsList;
  return(1);
}


int FTP::DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                     int OpMode)
{
  if (!hConnect && !ShowHosts)
    return(FALSE);

  if (ItemsNumber==0)
    return(FALSE);

  int MsgShown=TRUE;

  if ((OpMode & OPM_SILENT)==0)
  {
    char *MsgItems[]={ShowHosts ? GetMsg(MDeleteHostsTitle):GetMsg(MDeleteTitle),
                      ShowHosts ? GetMsg(MDeleteHosts):GetMsg(MDeleteFiles),
                      GetMsg(MDeleteDelete),GetMsg(MDeleteCancel)};
    if (Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)!=0)
      return(FALSE);
    ShellDeleteMsg("",0);
    MsgShown=TRUE;
    if (ItemsNumber>1)
    {
      char Msg[100];
      sprintf(Msg,ShowHosts ? GetMsg(MDeleteNumberOfHosts):GetMsg(MDeleteNumberOfFiles),ItemsNumber);
      MsgItems[1]=Msg;
      if (Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_DOWN,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)!=0)
        return(FALSE);
    }
  }

  struct PluginPanelItem *FullItemsList;
  int FullItemsNumber;
  if (!ExpandList(PanelItem,ItemsNumber,&FullItemsList,&FullItemsNumber,TRUE))
    return(FALSE);

  int DeleteAllFolders=FALSE;

  for (int I=0;I<FullItemsNumber;I++)
  {
    if (CheckForEsc())
    {
      delete FullItemsList;
      return(FALSE);
    }
    char *CurName=FullItemsList[I].FindData.cFileName;
    char *CurShowName=ShowHosts ? CurName+6:CurName;
    if (!MsgShown)
    {
      ShellDeleteMsg("",0);
      MsgShown=TRUE;
    }
    ShellDeleteMsg(CurShowName,FMSG_KEEPBACKGROUND);
    if (FullItemsList[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      if (!DeleteAllFolders && (OpMode & OPM_SILENT)==0)
      {
        char *MsgItems[]={ShowHosts ? GetMsg(MDeleteHostsTitle):GetMsg(MDeleteTitle),
              ShowHosts ? GetMsg(MDeleteHostFolder):GetMsg(MDeleteFolder),
              CurShowName,GetMsg(MDeleteGroupDelete),
              GetMsg(MDeleteGroupAll),GetMsg(MDeleteGroupCancel)};
        switch(Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_DOWN,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),3))
        {
          case -1:
          case 2:
            delete FullItemsList;
            return(FALSE);
          case 1:
            DeleteAllFolders=TRUE;
            break;
        }
      }
    }
    else
      if (ShowHosts)
        DeleteRegKey(HKEY_CURRENT_USER,CurName);
      else
        if (FtpDeleteFile(hConnect,CurName))
        {
          if (Opt.UpdateDescriptions && I<ItemsNumber)
            PanelItem[I].Flags|=PPIF_PROCESSDESCR;
        }
        else
        {
          char *MsgItems[]={GetMsg(MFtpTitle),GetMsg(MCannotDelete),CurShowName,GetMsg(MOk)};
          Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
          delete FullItemsList;
          return(FALSE);
        }
  }
  for (int I=FullItemsNumber-1;I>=0;I--)
  {
    if (CheckForEsc())
    {
      delete FullItemsList;
      return(FALSE);
    }
    char *CurName=FullItemsList[I].FindData.cFileName;
    if (!MsgShown)
    {
      ShellDeleteMsg("",0);
      MsgShown=TRUE;
    }
    ShellDeleteMsg(ShowHosts ? CurName+6:CurName,FMSG_KEEPBACKGROUND);
    if (FullItemsList[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      if (ShowHosts)
        DeleteRegKey(HKEY_CURRENT_USER,CurName);
      else
        if (FtpRemoveDirectory(hConnect,CurName))
          if (Opt.UpdateDescriptions && I<ItemsNumber)
            PanelItem[I].Flags|=PPIF_PROCESSDESCR;
  }
  delete FullItemsList;
  return(TRUE);
}


int FTP::ExpandList(struct PluginPanelItem *PanelItem,int ItemsNumber,
                    struct PluginPanelItem **pFullItemsList,
                    int *pFullItemsNumber,int FromPlugin)
{
  struct PluginPanelItem *FullItemsList=new PluginPanelItem[ItemsNumber];
  if (FullItemsList==NULL)
    return(FALSE);
  int FullItemsNumber=ItemsNumber;
  memcpy(FullItemsList,PanelItem,sizeof(*FullItemsList)*ItemsNumber);

  for (int I=0;I<ItemsNumber;I++)
    if (PanelItem[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      struct PluginPanelItem *DirPanelItem;
      int DirItemsNumber;
      if (FromPlugin && !Info.GetPluginDirList(Info.ModuleNumber,this,PanelItem[I].FindData.cFileName,&DirPanelItem,&DirItemsNumber) ||
          !FromPlugin && !Info.GetDirList(PanelItem[I].FindData.cFileName,&DirPanelItem,&DirItemsNumber))
      {
        delete FullItemsList;
        return(FALSE);
      }
      FullItemsList=(struct PluginPanelItem *)realloc(FullItemsList,sizeof(*FullItemsList)*(FullItemsNumber+DirItemsNumber+1));
      if (FullItemsList==NULL)
        return(FALSE);
      memcpy(FullItemsList+FullItemsNumber,DirPanelItem,sizeof(*FullItemsList)*DirItemsNumber);
      FullItemsNumber+=DirItemsNumber;
      Info.FreeDirList(DirPanelItem);
    }
  if (!ShowHosts && FromPlugin)
  {
    char CurDir[NM];
    DWORD Size=sizeof(CurDir);
    FtpGetCurrentDirectory(hConnect,CurDir,&Size);
    for (int I=0;I<FullItemsNumber;I++)
    {
      char *CurName=FullItemsList[I].FindData.cFileName;
      if (strchr(CurDir,'\\')==NULL)
        for (int I=0;CurName[I];I++)
          if (CurName[I]=='\\')
            CurName[I]='/';
    }
  }
  if (ShowHosts && FromPlugin)
    for (int I=0;I<FullItemsNumber;I++)
    {
      char FullKeyName[1024];
      strcpy(FullKeyName,"Hosts\\");
      if (*HostsPath)
      {
        strcat(FullKeyName,HostsPath);
        strcat(FullKeyName,"\\");
      }
      strcat(FullKeyName,FullItemsList[I].FindData.cFileName);
      strncpy(FullItemsList[I].FindData.cFileName,FullKeyName,NM);
    }
  *pFullItemsList=FullItemsList;
  *pFullItemsNumber=FullItemsNumber;
  return(TRUE);
}


void FTP::ShellDeleteMsg(char *Name,int Flags)
{
  char DelName[NM];
  CenterStr(Name,DelName,30);
  TruncStr(DelName,30);
  char *MsgItems[]={ShowHosts ? GetMsg(MDeleteHostsTitle):GetMsg(MDeleteTitle),
                    GetMsg(MDeleting),DelName};
  Info.Message(Info.ModuleNumber,Flags,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
}


int FTP::MakeDirectory(char *Name,int OpMode)
{
  const char *FolderHistory="NewFolder";
  if (!ShowHosts && !hConnect)
    return(FALSE);
  struct InitDialogItem InitItems[]={
    DI_DOUBLEBOX,3,1,72,4,0,0,0,1,(char *)MMkdirTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MMkdirName,
    DI_EDIT,5,3,70,3,1,(DWORD)FolderHistory,DIF_HISTORY,0,Name,
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  if ((OpMode & OPM_SILENT)==0)
  {
    int AskCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,6,"FTPCmd",DialogItems,
                            sizeof(DialogItems)/sizeof(DialogItems[0]));
    if (AskCode<0)
      return(-1);
    strcpy(Name,DialogItems[2].Data);
  }
  if (ShowHosts)
  {
    char FullKeyName[1024];
    GetFullKey(FullKeyName,Name);
    if (CheckRegKey(HKEY_CURRENT_USER,FullKeyName))
    {
      SetLastError(ERROR_ALREADY_EXISTS);
      return(FALSE);
    }
    SetRegKey(HKEY_CURRENT_USER,FullKeyName,"Folder",1);
    return(TRUE);
  }
  return(FtpCreateDirectory(hConnect,Name));
}


int FTP::ProcessKey(int Key,unsigned int ControlState)
{
  if (!ShowHosts && ControlState==PKF_CONTROL && Key==VK_INSERT)
  {
    char CmdLine[1024];
    Info.Control(this,FCTL_GETCMDLINE,CmdLine);
    if (*CmdLine==0)
      ControlState=PKF_CONTROL|PKF_SHIFT;
  }

  if (!ShowHosts && ControlState==(PKF_CONTROL|PKF_SHIFT) && Key==VK_INSERT)
  {
    CopyNamesToClipboard();
    return(TRUE);
  }

  if (!ShowHosts && ControlState==PKF_SHIFT && Key==VK_F7)
  {
    SelectTable();
    Info.Control(this,FCTL_UPDATEPANEL,NULL);
    Info.Control(this,FCTL_REDRAWPANEL,NULL);
    return(TRUE);
  }

  if (!ShowHosts && hConnect && ControlState==PKF_CONTROL && Key=='A')
  {
    SetAttributes();
    Info.Control(this,FCTL_UPDATEPANEL,NULL);
    Info.Control(this,FCTL_REDRAWPANEL,NULL);
    return(TRUE);
  }

  if (ControlState==PKF_CONTROL)
  {
    if (Key=='R')
    {
      ResetCache=TRUE;
      return(FALSE);
    }
    if (!ShowHosts && hConnect && Key=='F')
    {
      char FullName[512];
      struct PanelInfo PInfo;
      Info.Control(this,FCTL_GETPANELINFO,&PInfo);
      if (PInfo.CurrentItem>=PInfo.ItemsNumber)
        return(FALSE);
      PluginPanelItem *CurItem=&PInfo.PanelItems[PInfo.CurrentItem];
      GetFullFileName(FullName,CurItem->FindData.cFileName);
      Info.Control(this,FCTL_INSERTCMDLINE,FullName);
      return(TRUE);
    }
  }
  if (ControlState==0 && Key==VK_RETURN)
  {
    struct PanelInfo PInfo;
    Info.Control(this,FCTL_GETPANELINFO,&PInfo);
    if (PInfo.CurrentItem<PInfo.ItemsNumber)
    {
      DWORD Attr=PInfo.PanelItems[PInfo.CurrentItem].FindData.dwFileAttributes;
      if ((Attr & FILE_ATTRIBUTE_DIRECTORY)==0)
      {
        char *DirName=PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName;
        if (!ShowHosts && hConnect)
        {
          if (FtpSetCurrentDirectory(hConnect,DirName))
          {
            Info.Control(this,FCTL_UPDATEPANEL,NULL);
            struct PanelRedrawInfo RInfo;
            RInfo.CurrentItem=RInfo.TopPanelItem=0;
            Info.Control(this,FCTL_REDRAWPANEL,&RInfo);
            return(TRUE);
          }
        }
        else
          if (ShowHosts)
          {
            char RegKey[1024];
            strcpy(HostName,PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName);
            GetFullKey(RegKey,HostName);
            char CryptedPassword[256];
            GetRegKey(HKEY_CURRENT_USER,RegKey,"Description",HostDescr,"",sizeof(HostDescr));
            GetRegKey(HKEY_CURRENT_USER,RegKey,"Password",(BYTE *)CryptedPassword,(BYTE *)NULL,sizeof(CryptedPassword));
            DecryptPassword(CryptedPassword,Password);
            GetRegKey(HKEY_CURRENT_USER,RegKey,"AskLogin",AskLogin,0);
            GetRegKey(HKEY_CURRENT_USER,RegKey,"AsciiMode",AsciiMode,0);
            GetRegKey(HKEY_CURRENT_USER,RegKey,"PassiveMode",PassiveMode,Opt.PassiveMode);
            GetRegKey(HKEY_CURRENT_USER,RegKey,"UseFirewall",UseFirewall,*Opt.Firewall!=0);
            GetRegKey(HKEY_CURRENT_USER,RegKey,"CharTable",HostTable,"",sizeof(HostTable));
            if (Connect())
            {
              SwitchingToFTP=TRUE;
              Info.Control(this,FCTL_UPDATEPANEL,NULL);
              SwitchingToFTP=FALSE;
              if (ShowHosts)
              {
                Info.Control(this,FCTL_UPDATEPANEL,NULL);
                Info.Control(this,FCTL_REDRAWPANEL,NULL);
              }
              else
              {
                struct PanelRedrawInfo RInfo;
                RInfo.CurrentItem=RInfo.TopPanelItem=0;
                Info.Control(this,FCTL_REDRAWPANEL,&RInfo);
              }
            }
            else
              *HostName=0;
            return(TRUE);
          }
      }
    }
  }
  if (ShowHosts && (ControlState==PKF_SHIFT && Key==VK_F4 || ControlState==PKF_ALT && Key==VK_F6))
  {
    char HostName[NM],Password[256],HostDescr[256],HostTable[100];
    int AskLogin=0,AsciiMode=0,PassiveMode=Opt.PassiveMode,UseFirewall=(*Opt.Firewall!=0);
    *HostName=*Password=*HostDescr=*HostTable=0;
    if (GetHost(HostName,Password,HostDescr,AskLogin,AsciiMode,PassiveMode,UseFirewall,HostTable))
    {
      char FullKeyName[1024];
      GetFullKey(FullKeyName,HostName);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"Description",HostDescr);
      char CryptedPassword[256];
      EncryptPassword(Password,CryptedPassword);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"Password",(BYTE *)CryptedPassword,strlen(CryptedPassword)+1);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"AskLogin",AskLogin);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"AsciiMode",AsciiMode);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"PassiveMode",PassiveMode);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"UseFirewall",UseFirewall);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"CharTable",HostTable);
      Info.Control(this,FCTL_UPDATEPANEL,NULL);
      Info.Control(this,FCTL_REDRAWPANEL,NULL);
    }
    return(TRUE);
  }
  if (!ShowHosts && ControlState==PKF_ALT && Key==VK_F6)
  {
    SaveURL();
    return(TRUE);
  }
  if (ShowHosts && ControlState==0 && Key==VK_F4)
  {
    struct PanelInfo PInfo;
    Info.Control(this,FCTL_GETPANELINFO,&PInfo);
    if (PInfo.CurrentItem>=PInfo.ItemsNumber)
      return(TRUE);
    PluginPanelItem *CurItem=&PInfo.PanelItems[PInfo.CurrentItem];
    if (CurItem->FindData.dwFileAttributes & FA_DIREC)
      return(TRUE);

    char HostName[NM],Password[256],HostDescr[256],HostTable[100];
    int AskLogin,AsciiMode,PassiveMode,UseFirewall;

    strcpy(HostName,PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName);

    char FullKeyName[1024];
    GetFullKey(FullKeyName,HostName);

    GetRegKey(HKEY_CURRENT_USER,FullKeyName,"Description",HostDescr,"",sizeof(HostDescr));
    char CryptedPassword[256];
    GetRegKey(HKEY_CURRENT_USER,FullKeyName,"Password",(BYTE *)CryptedPassword,(BYTE *)NULL,sizeof(CryptedPassword));
    DecryptPassword(CryptedPassword,Password);
    GetRegKey(HKEY_CURRENT_USER,FullKeyName,"AskLogin",AskLogin,0);
    GetRegKey(HKEY_CURRENT_USER,FullKeyName,"AsciiMode",AsciiMode,0);
    GetRegKey(HKEY_CURRENT_USER,FullKeyName,"PassiveMode",PassiveMode,Opt.PassiveMode);
    GetRegKey(HKEY_CURRENT_USER,FullKeyName,"UseFirewall",UseFirewall,*Opt.Firewall!=0);
    GetRegKey(HKEY_CURRENT_USER,FullKeyName,"CharTable",HostTable,"",sizeof(HostTable));

    if (GetHost(HostName,Password,HostDescr,AskLogin,AsciiMode,PassiveMode,UseFirewall,HostTable))
    {
      DeleteRegKey(HKEY_CURRENT_USER,FullKeyName);
      GetFullKey(FullKeyName,HostName);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"Description",HostDescr);
      char CryptedPassword[256];
      EncryptPassword(Password,CryptedPassword);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"Password",(BYTE *)CryptedPassword,strlen(CryptedPassword)+1);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"AskLogin",AskLogin);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"AsciiMode",AsciiMode);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"PassiveMode",PassiveMode);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"UseFirewall",UseFirewall);
      SetRegKey(HKEY_CURRENT_USER,FullKeyName,"CharTable",HostTable);
      Info.Control(this,FCTL_UPDATEPANEL,NULL);
      Info.Control(this,FCTL_REDRAWPANEL,NULL);
    }
    return(TRUE);
  }
  return(FALSE);
}


void FTP::GetFullFileName(char *FullName,char *Name)
{
  char FileName[512];
  strcpy(FullName,"ftp://");
  if (strncmp(HostName,"ftp://",6)==0)
    strcat(FullName,HostName+6);
  else
    strcat(FullName,HostName);
  char *Slash=strchr(FullName+6,'/');
  if (Slash!=NULL)
    *Slash=0;
  DWORD Size=sizeof(FileName);
  FtpGetCurrentDirectory(hConnect,FileName,&Size);
  AddEndSlash(FileName);
  strcat(FileName,Name);
  for (int I=0;FileName[I]!=0;I++)
    if (FileName[I]=='\\')
      FileName[I]='/';
  if (*FileName && *FileName!='/')
    strcat(FullName,"/");
  strcat(FullName,FileName);
}


void FTP::CopyNamesToClipboard()
{
  char FullName[512];
  struct PanelInfo PInfo;
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
  char *CopyData=NULL;
  int CopySize=0;
  for (int I=0;I<PInfo.SelectedItemsNumber;I++)
  {
    PluginPanelItem *CurItem=&PInfo.SelectedItems[I];
    char FullName[512];
    GetFullFileName(FullName,CurItem->FindData.cFileName);
    strcat(FullName,"\r\n");
    int Length=strlen(FullName);
    char *NewData=(char *)realloc(CopyData,CopySize+Length+1);
    if (NewData==NULL)
    {
      delete CopyData;
      return;
    }
    CopyData=NewData;
    strcpy(CopyData+CopySize,FullName);
    CopySize+=Length;
  }
  if (CopyData!=NULL)
  {
    CopyToClipboard(CopyData);
    delete CopyData;
  }
}


int FTP::ProcessCommandLine(char *CommandLine)
{
  while (*CommandLine=='/')
    CommandLine++;
  strcpy(HostName,CommandLine);
  strcpy(Password,Opt.DefaultPassword);
  AskLogin=0;
  AsciiMode=0;
  PassiveMode=Opt.PassiveMode;
  UseFirewall=(*Opt.Firewall!=0);
  *HostTable=0;
  return(Connect());
}


int FTP::ProcessShortcutLine(char *Line)
{
  if (Line==NULL)
    return(FALSE);
  strcpy(HostName,Line);
  char *ChPtr=strchr(Line,'\x1');
  if (ChPtr==NULL)
    return(FALSE);
  AskLogin=0;
  AsciiMode=0;
  PassiveMode=Opt.PassiveMode;
  UseFirewall=(*Opt.Firewall!=0);
  *HostTable=0;
  HostName[ChPtr-Line]=0;
  ChPtr++;
  char UserName[NM];
  strcpy(UserName,ChPtr);
  ChPtr=strchr(ChPtr,'\x1');
  if (ChPtr==NULL)
  {
    strcpy(Password,UserName);
    if ((ChPtr=strchr(Password,'\x2'))!=NULL)
    {
      *ChPtr=0;
      AskLogin=(ChPtr[1]=='1');
    }
  }
  else
  {
    *strchr(UserName,'\x1')=0;
    strcat(UserName,"@");
    strcat(UserName,HostName);
    strcpy(HostName,UserName);
    ChPtr++;
    strcpy(Password,ChPtr);
  }
  return(Connect());
}


void FTP::GetNewKeyName(char *FullKeyName)
{
  MakeKeyName(FullKeyName,GetFreeKeyNumber());
}


int FTP::GetFreeKeyNumber()
{
  int Number=0;
  for (int I=0;;I++)
  {
    char FullKeyName[1024],HostName[NM];
    MakeKeyName(FullKeyName,I);
    GetRegKey(HKEY_CURRENT_USER,FullKeyName,"HostName",HostName,"",sizeof(HostName));
    if (*HostName==0)
      break;
    Number++;
  }
  return(Number);
}


void FTP::MakeKeyName(char *FullKeyName,int Number)
{
  char KeyName[100];
  sprintf(KeyName,"Host%d",Number);
  strcpy(FullKeyName,HostsPath);
  if (*FullKeyName)
    strcat(FullKeyName,"\\");
  strcat(FullKeyName,KeyName);
}


void FTP::GetFullKey(char *FullKeyName,char *Name)
{
  sprintf(FullKeyName,"Hosts\\%s",HostsPath);
  if (*HostsPath)
    strcat(FullKeyName,"\\");
  strcat(FullKeyName,Name);
}


int FTP::GetHostFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                      int Move,char *DestPath,int OpMode)
{
  struct InitDialogItem InitItems[]={
    DI_DOUBLEBOX,3,1,72,6,0,0,0,0,(char *)(Move ? MMoveHostTitle:MCopyHostTitle),
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)(Move ? MMoveHostTo:MCopyHostTo),
    DI_EDIT,5,3,70,3,1,0,0,0,DestPath,
    DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,1,(char *)MCopy,
    DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  if ((OpMode & OPM_SILENT)==0)
  {
    int AskCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,8,"FTPCmd",DialogItems,
                            sizeof(DialogItems)/sizeof(DialogItems[0]));
    if (AskCode!=4)
      return(-1);
    strcpy(DestPath,DialogItems[2].Data);
  }
  if (*DestPath==0)
    return(-1);

  struct PluginPanelItem *FullItemsList;
  int FullItemsNumber;
  if (!ExpandList(PanelItem,ItemsNumber,&FullItemsList,&FullItemsNumber,TRUE))
    return(0);

  int HostsPathLength=strlen(HostsPath);
  if (HostsPathLength>0)
    HostsPathLength++;


  int OverwriteAll=FALSE,SkipAll=FALSE,Rename=FALSE,NewDir=FALSE;

  char CheckKey[1024];
  if (strcmp(DestPath,"..")==0)
  {
    if (*HostsPath==0)
      return(0);
    else
    {
      sprintf(CheckKey,"Hosts\\%s",HostsPath);
      *(strrchr(CheckKey,'\\')+1)=0;
      Rename=TRUE;
    }
  }
  else
  {
    GetFullKey(CheckKey,DestPath);
    if (strpbrk(DestPath,":\\")==NULL && !CheckRegKey(HKEY_CURRENT_USER,CheckKey))
    {
      NewDir=TRUE;
      Rename=TRUE;
    }
    else
      if (GetRegKey(HKEY_CURRENT_USER,CheckKey,"Folder",0))
      {
        AddEndSlash(CheckKey);
        Rename=TRUE;
      }
  }

  AddEndSlash(DestPath);

  if (Rename)
  {
    for (int I=0;I<FullItemsNumber;I++)
    {
      char DestKey[NM],*CurName;
      struct PluginPanelItem *CurPanelItem=&FullItemsList[I];
      CurName=CurPanelItem->FindData.cFileName;
      if (strcmp(PointToName(CurName),"..")==0)
        continue;
      char *NamePtr=CurName+HostsPathLength+6;
      if (NewDir)
        while (*NamePtr && *NamePtr!='\\')
          NamePtr++;
      sprintf(DestKey,"%s%s",CheckKey,NamePtr);

      if (CheckRegKey(HKEY_CURRENT_USER,DestKey))
      {
        if ((OpMode & OPM_SILENT)==0 && !OverwriteAll)
        {
          if (SkipAll)
            continue;
          char *MsgItems[]={GetMsg(Move ? MMoveHostTitle:MCopyHostTitle),
            GetMsg(MHostAlreadyExist),DestKey+6,GetMsg(MHostAskOverwrite),
            GetMsg(MOverwrite),GetMsg(MOverwriteAll),GetMsg(MCopySkip),
            GetMsg(MCopySkipAll),GetMsg(MCopyCancel)};
          int MsgCode=Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),5);
          switch(MsgCode)
          {
            case 1:
              OverwriteAll=TRUE;
              break;
            case 3:
              SkipAll=TRUE;
            case 2:
              continue;
            case -1:
            case 4:
              delete FullItemsList;
              return(-1);
          }
        }
      }

      DeleteRegKey(HKEY_CURRENT_USER,DestKey);
      if (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        SetRegKey(HKEY_CURRENT_USER,DestKey,"Folder",1);
        continue;
      }
      char Password[256],HostDescr[256],HostTable[100];
      int AskLogin,AsciiMode,PassiveMode,UseFirewall;
      GetRegKey(HKEY_CURRENT_USER,CurName,"Description",HostDescr,"",sizeof(HostDescr));
      GetRegKey(HKEY_CURRENT_USER,CurName,"Password",(BYTE *)Password,(BYTE *)NULL,sizeof(Password));
      GetRegKey(HKEY_CURRENT_USER,CurName,"AskLogin",AskLogin,0);
      GetRegKey(HKEY_CURRENT_USER,CurName,"AsciiMode",AsciiMode,0);
      GetRegKey(HKEY_CURRENT_USER,CurName,"PassiveMode",PassiveMode,Opt.PassiveMode);
      GetRegKey(HKEY_CURRENT_USER,CurName,"UseFirewall",UseFirewall,(*Opt.Firewall!=0));
      GetRegKey(HKEY_CURRENT_USER,CurName,"CharTable",HostTable,"",sizeof(HostTable));
      SetRegKey(HKEY_CURRENT_USER,DestKey,"Description",HostDescr);
      SetRegKey(HKEY_CURRENT_USER,DestKey,"Password",(BYTE *)Password,strlen(Password)+1);
      SetRegKey(HKEY_CURRENT_USER,DestKey,"AskLogin",AskLogin);
      SetRegKey(HKEY_CURRENT_USER,DestKey,"AsciiMode",AsciiMode);
      SetRegKey(HKEY_CURRENT_USER,DestKey,"PassiveMode",PassiveMode);
      SetRegKey(HKEY_CURRENT_USER,DestKey,"UseFirewall",UseFirewall);
      SetRegKey(HKEY_CURRENT_USER,DestKey,"CharTable",HostTable);
      if (Move)
        DeleteRegKey(HKEY_CURRENT_USER,CurName);
      if (I<ItemsNumber)
        PanelItem[I].Flags&=~PPIF_SELECTED;
    }
  }
  else
    for (int I=0;I<FullItemsNumber;I++)
    {
      char DestName[NM],*CurName;
      struct PluginPanelItem *CurPanelItem=&FullItemsList[I];
      CurName=CurPanelItem->FindData.cFileName;
      if (strcmp(PointToName(CurName),"..")==0)
        continue;
      sprintf(DestName,"%s%s",DestPath,CurName+HostsPathLength+6);
      for (int J=strlen(DestPath);DestName[J]!=0;J++)
        if (DestName[J]==':' || DestName[J]=='/')
          DestName[J]='_';
      if (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        CreateDirectory(DestName,NULL);
        continue;
      }
      DWORD DestAttr=GetFileAttributes(DestName);
      if ((OpMode & OPM_SILENT)==0 && !OverwriteAll && DestAttr!=0xFFFFFFFF)
      {
        if (SkipAll)
          continue;
        char *MsgItems[]={GetMsg(Move ? MMoveHostTitle:MCopyHostTitle),
          (DestAttr & FA_RDONLY) ? GetMsg(MAlreadyExistRO):GetMsg(MAlreadyExist),
          DestName,GetMsg(MAskOverwrite),GetMsg(MOverwrite),
          GetMsg(MOverwriteAll),GetMsg(MCopySkip),GetMsg(MCopySkipAll),
          GetMsg(MCopyCancel)};
        int MsgCode=Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),5);
        switch(MsgCode)
        {
          case 1:
            OverwriteAll=TRUE;
            break;
          case 3:
            SkipAll=TRUE;
          case 2:
            continue;
          case -1:
          case 4:
            delete FullItemsList;
            return(-1);
        }
      }
      int WriteFailed=FALSE;
      if (DestAttr!=0xFFFFFFFF && remove(DestName)!=0)
        WriteFailed=TRUE;
      else
      {
        char Password[256],HostDescr[256],HostTanle[100];
        int AskLogin,AsciiMode,PassiveMode,UseFirewall;

        GetRegKey(HKEY_CURRENT_USER,CurName,"Description",HostDescr,"",sizeof(HostDescr));
        GetRegKey(HKEY_CURRENT_USER,CurName,"Password",(BYTE *)Password,(BYTE *)NULL,sizeof(Password));
        GetRegKey(HKEY_CURRENT_USER,CurName,"AskLogin",AskLogin,0);
        GetRegKey(HKEY_CURRENT_USER,CurName,"AsciiMode",AsciiMode,0);
        GetRegKey(HKEY_CURRENT_USER,CurName,"PassiveMode",PassiveMode,Opt.PassiveMode);
        GetRegKey(HKEY_CURRENT_USER,CurName,"UseFirewall",UseFirewall,*Opt.Firewall!=0);
        GetRegKey(HKEY_CURRENT_USER,CurName,"CharTable",HostTable,"",sizeof(HostTable));

        char AskLoginStr[10],AsciiModeStr[10],PassiveModeStr[10],UseFirewallStr[10];
        sprintf(AskLoginStr,"%d",AskLogin);
        sprintf(AsciiModeStr,"%d",AsciiMode);
        sprintf(PassiveModeStr,"%d",PassiveMode);
        sprintf(UseFirewallStr,"%d",UseFirewall);

        char HexStr[256];
        PasswordToHex(Password,HexStr);

        WritePrivateProfileString(NULL,NULL,NULL,DestName);

        if (!WritePrivateProfileString("FarFTP","URL",strrchr(CurName,'\\')+1,DestName) ||
            !WritePrivateProfileString("FarFTP","Password",HexStr,DestName) ||
            !WritePrivateProfileString("FarFTP","Description",HostDescr,DestName) ||
            !WritePrivateProfileString("FarFTP","AskLogin",AskLoginStr,DestName) ||
            !WritePrivateProfileString("FarFTP","AsciiMode",AsciiModeStr,DestName) ||
            !WritePrivateProfileString("FarFTP","PassiveMode",PassiveModeStr,DestName) ||
            !WritePrivateProfileString("FarFTP","UseFirewall",UseFirewallStr,DestName) ||
            !WritePrivateProfileString("FarFTP","CharTable",HostTable,DestName))
        {
          WriteFailed=TRUE;
          remove(DestName);
        }
        else
        {
          WritePrivateProfileString(NULL,NULL,NULL,DestName);
          if (Move)
            DeleteRegKey(HKEY_CURRENT_USER,CurName);
        }
      }
      if (WriteFailed)
      {
        char *MsgItems[]={GetMsg(MError),GetMsg(MCannotCopyHost),CurName+6,GetMsg(MOk)};
        Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_DOWN|FMSG_ERRORTYPE,NULL,
                     MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
        delete FullItemsList;
        return(0);
      }
    }

  if (Move)
    for (int I=FullItemsNumber-1;I>=0;I--)
    {
      if (CheckForEsc())
      {
        delete FullItemsList;
        return(-1);
      }
      char *CurName=FullItemsList[I].FindData.cFileName;
      if (FullItemsList[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        DeleteRegKey(HKEY_CURRENT_USER,CurName);
        if (I<ItemsNumber)
          PanelItem[I].Flags&=~PPIF_SELECTED;
      }
    }
  delete FullItemsList;
  return(1);
}


int FTP::PutHostsFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                       int Move,int OpMode)
{
  struct InitDialogItem InitItems[]={
    DI_DOUBLEBOX,3,1,72,6,0,0,0,0,(char *)(Move ? MMoveHostTitle:MCopyHostTitle),
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)(Move ? MMoveHostTo:MCopyHostTo),
    DI_EDIT,5,3,70,3,1,0,0,0,HostsPath,
    DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,1,(char *)MCopy,
    DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  char DestPath[NM];

  if ((OpMode & OPM_SILENT)==0)
  {
    int AskCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,8,"FTPCmd",DialogItems,
                            sizeof(DialogItems)/sizeof(DialogItems[0]));
    if (AskCode!=4)
      return(-1);
    strcpy(DestPath,DialogItems[2].Data);
  }

  if (*DestPath)
    AddEndSlash(DestPath);

  struct PluginPanelItem *FullItemsList;
  int FullItemsNumber;
  if (!ExpandList(PanelItem,ItemsNumber,&FullItemsList,&FullItemsNumber,FALSE))
    return(0);

  int OverwriteAll=FALSE,SkipAll=FALSE;
  for (int I=0;I<FullItemsNumber;I++)
  {
    char DestName[NM],CurName[512];
    strcpy(CurName,FullItemsList[I].FindData.cFileName);
    for (int J=0;CurName[J]!=0;J++)
      if (J>1 && CurName[J]==':' || CurName[J]=='/')
        CurName[J]='_';

    char HostName[512],Password[256],HostDescr[256],HostTable[100],SrcFile[NM];
    int AskLogin,AsciiMode,PassiveMode,UseFirewall;

    DWORD SrcAttr=GetFileAttributes(CurName);

    if (SrcAttr==0xFFFFFFFF)
      continue;
    if (SrcAttr & FILE_ATTRIBUTE_DIRECTORY)
    {
      strcpy(DestName,"Hosts\\");
      strcat(DestName,DestPath);
      strcat(DestName,CurName);

      char NewDirKey[512];
      strcpy(NewDirKey,DestName);
      *strrchr(NewDirKey,'\\')=0;
      SetRegKey(HKEY_CURRENT_USER,NewDirKey,"Folder",1);
      SetRegKey(HKEY_CURRENT_USER,DestName,"Folder",1);
      continue;
    }

    if (strchr(CurName,'\\')==NULL)
      sprintf(SrcFile,".\\%s",CurName);
    else
      strcpy(SrcFile,CurName);

    char HexStr[256];

    GetPrivateProfileString("FarFTP","URL","",HostName,sizeof(HostName),SrcFile);
    GetPrivateProfileString("FarFTP","Password","",HexStr,sizeof(HexStr),SrcFile);
    GetPrivateProfileString("FarFTP","Description","",HostDescr,sizeof(HostDescr),SrcFile);
    AskLogin=GetPrivateProfileInt("FarFTP","AskLogin",0,SrcFile);
    AsciiMode=GetPrivateProfileInt("FarFTP","AsciiMode",0,SrcFile);
    PassiveMode=GetPrivateProfileInt("FarFTP","PassiveMode",Opt.PassiveMode,SrcFile);
    UseFirewall=GetPrivateProfileInt("FarFTP","UseFirewall",*Opt.Firewall!=0,SrcFile);
    GetPrivateProfileString("FarFTP","CharTable","",HostTable,sizeof(HostTable),SrcFile);

    HexToPassword(HexStr,Password);

    if (*HostName==0)
    {
      char *MsgItems[]={GetMsg(MError),GetMsg(MCannotCopyHost),CurName,GetMsg(MOk)};
      Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_DOWN|FMSG_ERRORTYPE,
                    NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
      delete FullItemsList;
      return(0);
    }

    char FullHostName[NM],*NamePtr;
    strcpy(FullHostName,CurName);
    if ((NamePtr=strrchr(FullHostName,'\\'))==NULL)
      strcpy(FullHostName,HostName);
    else
      strcpy(NamePtr+1,HostName);

    strcpy(DestName,"Hosts\\");
    strcat(DestName,DestPath);
    strcat(DestName,FullHostName);

    DWORD DestAttr=0xFFFFFFFF;
    WIN32_FIND_DATA FindData;
    if (CheckRegKey(HKEY_CURRENT_USER,DestName))
      DestAttr=0;

    if ((OpMode & OPM_SILENT)==0 && !OverwriteAll && DestAttr!=0xFFFFFFFF)
    {
      if (SkipAll)
        continue;
      char *MsgItems[]={GetMsg(Move ? MMoveHostTitle:MCopyHostTitle),
        GetMsg(MHostAlreadyExist),DestName+6,GetMsg(MHostAskOverwrite),
        GetMsg(MOverwrite),GetMsg(MOverwriteAll),GetMsg(MCopySkip),
        GetMsg(MCopySkipAll),GetMsg(MCopyCancel)};
      int MsgCode=Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,
                               sizeof(MsgItems)/sizeof(MsgItems[0]),5);
      switch(MsgCode)
      {
        case 1:
          OverwriteAll=TRUE;
          break;
        case 3:
          SkipAll=TRUE;
        case 2:
          continue;
        case -1:
        case 4:
          delete FullItemsList;
          return(-1);
      }
    }

    SetRegKey(HKEY_CURRENT_USER,DestName,"Description",HostDescr);
    SetRegKey(HKEY_CURRENT_USER,DestName,"Password",(BYTE *)Password,strlen(Password)+1);
    SetRegKey(HKEY_CURRENT_USER,DestName,"AskLogin",AskLogin);
    SetRegKey(HKEY_CURRENT_USER,DestName,"AsciiMode",AsciiMode);
    SetRegKey(HKEY_CURRENT_USER,DestName,"PassiveMode",PassiveMode);
    SetRegKey(HKEY_CURRENT_USER,DestName,"UseFirewall",UseFirewall);
    SetRegKey(HKEY_CURRENT_USER,DestName,"CharTable",HostTable);

    if (Move)
    {
      SetFileAttributes(CurName,0);
      remove(CurName);
    }
    if (I<ItemsNumber)
      PanelItem[I].Flags&=~PPIF_SELECTED;
  }

  if (Move)
    for (int I=FullItemsNumber-1;I>=0;I--)
    {
      if (CheckForEsc())
      {
        delete FullItemsList;
        return(-1);
      }
      char *CurName=FullItemsList[I].FindData.cFileName;
      if (FullItemsList[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        if (RemoveDirectory(CurName))
          if (I<ItemsNumber)
            PanelItem[I].Flags&=~PPIF_SELECTED;
    }

  delete FullItemsList;
  return(1);
}


void FTP::SaveURL()
{
  const char *HostHistory="FTPHost";
  struct InitDialogItem InitItems[]=
  {
    DI_DOUBLEBOX,3,1,72,15,0,0,0,0,(char *)MSaveFTPTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MFtpName,
    DI_EDIT,5,3,70,3,1,(DWORD)HostHistory,DIF_HISTORY,0,"",
    DI_TEXT,5,4,0,0,0,0,0,0,(char *)MUserPassword,
    DI_PSWEDIT,5,5,70,3,0,0,0,0,"",
    DI_TEXT,5,6,0,0,0,0,0,0,(char *)MHostDescr,
    DI_EDIT,5,7,70,3,0,0,0,0,"",
    DI_TEXT,3,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MAskLogin,
    DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MAsciiMode,
    DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MPassiveMode,
    DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)MUseFirewall,
    DI_TEXT,3,13,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,1,(char *)MFtpSave,
    DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFtpSelectTable,
    DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  strcpy(DialogItems[2].Data,HostName);
  char *HostPtr=strnicmp(DialogItems[2].Data,"ftp://",6)==0 ? DialogItems[2].Data+6:DialogItems[2].Data;

  char CurDir[NM],*StartHome;
  DWORD Size=sizeof(CurDir);
  FtpGetCurrentDirectory(hConnect,CurDir,&Size);

  if ((StartHome=strchr(HostPtr,'/'))!=NULL && StartHome>DialogItems[2].Data &&
      *(StartHome-1)!='/' && *(StartHome-1)!=':')
  {
    if (*CurDir=='/' || *CurDir=='\\')
      strcpy(StartHome,CurDir);
    else
      strcpy(StartHome+1,CurDir);
  }
  else
    if (*CurDir)
    {
      if (*CurDir!='/' && *CurDir!='\\')
        strcat(DialogItems[2].Data,"/");
      strcat(DialogItems[2].Data,CurDir);
    }

  strcpy(DialogItems[4].Data,Password);
  strcpy(DialogItems[6].Data,HostDescr);
  DialogItems[8].Selected=FTP::AskLogin;
  DialogItems[9].Selected=FTP::AsciiMode;
  DialogItems[10].Selected=FTP::PassiveMode;
  DialogItems[11].Selected=FTP::UseFirewall;

  int AskCode;
  char TableName[100];
  strcpy(TableName,HostTable);
  while (1)
  {
    AskCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,17,"FTPConnect",DialogItems,
                        sizeof(DialogItems)/sizeof(DialogItems[0]));
    if (AskCode==14)
    {
      SelectHostTable(TableName);
      DialogItems[14].Focus=FALSE;
      DialogItems[2].Focus=TRUE;
    }
    else
      break;
  }
  if (AskCode!=13 || *DialogItems[2].Data==0)
    return;

  char *CurHostName=DialogItems[2].Data;
  char *CurPassword=DialogItems[4].Data;
  char *CurHostDescr=DialogItems[6].Data;
  int AskLogin=DialogItems[8].Selected;
  int AsciiCode=DialogItems[9].Selected;
  int PassiveMode=DialogItems[10].Selected;
  int UseFirewall=DialogItems[11].Selected;
  strcpy(HostTable,TableName);
  for (int I=0;CurHostName[I]!=0;I++)
    if (CurHostName[I]=='\\')
      CurHostName[I]='/';
  char FullKeyName[1024];
  sprintf(FullKeyName,"Hosts\\%s",CurHostName);
  SetRegKey(HKEY_CURRENT_USER,FullKeyName,"Description",CurHostDescr);
  char CryptedPassword[256];
  EncryptPassword(CurPassword,CryptedPassword);
  SetRegKey(HKEY_CURRENT_USER,FullKeyName,"Password",(BYTE *)CryptedPassword,strlen(CryptedPassword)+1);
  SetRegKey(HKEY_CURRENT_USER,FullKeyName,"AskLogin",AskLogin);
  SetRegKey(HKEY_CURRENT_USER,FullKeyName,"AsciiCode",AsciiCode);
  SetRegKey(HKEY_CURRENT_USER,FullKeyName,"PassiveMode",PassiveMode);
  SetRegKey(HKEY_CURRENT_USER,FullKeyName,"UseFirewall",UseFirewall);
  SetRegKey(HKEY_CURRENT_USER,FullKeyName,"CharTable",HostTable);
}


void ConnectMessage(int Msg,char *HostName)
{
  static char LastHost[NM];
  static int MaxLength;
  char *MsgItems[]={GetMsg(MFtpTitle),GetMsg(MConnectingTo),LastHost,NULL};
  char CenteredStr[512];
  switch(Msg)
  {
    case 0:
      {
        int Length;
        MaxLength=strlen(GetMsg(MWaitingForResponse));
        if ((Length=strlen(GetMsg(MSendingName)))>MaxLength)
          MaxLength=Length;
        if ((Length=strlen(GetMsg(MPasswordName)))>MaxLength)
          MaxLength=Length;
        if ((Length=strlen(GetMsg(MRequestingFolder)))>MaxLength)
          MaxLength=Length;
      }
      strcpy(LastHost,HostName);
      CenterStr(GetMsg(MWaitingForResponse),CenteredStr,MaxLength);
      MsgItems[3]=CenteredStr;
      break;
    case 1:
      CenterStr(GetMsg(MSendingName),CenteredStr,MaxLength);
      MsgItems[3]=CenteredStr;
      break;
    case 2:
      CenterStr(GetMsg(MPasswordName),CenteredStr,MaxLength);
      MsgItems[3]=CenteredStr;
      break;
    case 3:
      CenterStr(GetMsg(MRequestingFolder),CenteredStr,MaxLength);
      MsgItems[3]=CenteredStr;
      break;
  }
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
}


int FTP::GetLoginData(char *UserName,char *Password)
{
  struct InitDialogItem InitItems[]=
  {
    DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MLoginInfo,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MUserName,
    DI_EDIT,5,3,70,3,0,0,0,0,"",
    DI_TEXT,5,4,0,0,0,0,0,0,(char *)MUserPassword,
    DI_PSWEDIT,5,5,70,3,0,0,0,0,"",
    DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  if (*UserName==0 || *Password!=0)
    DialogItems[2].Focus=TRUE;
  else
    DialogItems[4].Focus=TRUE;

  strcpy(DialogItems[2].Data,UserName);
  strcpy(DialogItems[4].Data,Password);

  int AskCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,10,"FTPConnect",DialogItems,
                          sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (AskCode!=6 || *DialogItems[2].Data==0)
    return(FALSE);
  strcpy(UserName,DialogItems[2].Data);
  strcpy(Password,DialogItems[4].Data);
  return(TRUE);
}


int FTP::Compare(struct PluginPanelItem *Item1,struct PluginPanelItem *Item2,
                 unsigned int Mode)
{
  if (Item1->CustomColumnNumber>0 && ShowHosts)
    switch(Mode)
    {
      case SM_NAME:
      case SM_EXT:
        {
          if ((Item1->FindData.dwFileAttributes & FA_DIREC)>(Item2->FindData.dwFileAttributes & FA_DIREC))
            return(-1);
          if ((Item1->FindData.dwFileAttributes & FA_DIREC)<(Item2->FindData.dwFileAttributes & FA_DIREC))
            return(1);
          int RetCode=stricmp(Item1->CustomColumnData[0],Item2->CustomColumnData[0]);
          if (RetCode<0)
            return(-1);
          if (RetCode>0)
            return(1);
          return(0);
        }
    }
  return(-2);
}


void FTP::PasswordToHex(char *Password,char *HexStr)
{
  strcpy(HexStr,"hex:");
  HexStr+=4;
  while (*Password)
  {
    sprintf(HexStr,"%02x",*Password);
    Password++;
    HexStr+=2;
  }
}


void FTP::HexToPassword(char *HexStr,char *Password)
{
  if (strncmp(HexStr,"hex:",4)==0)
  {
    HexStr+=4;
    while (*HexStr)
    {
      *Password=HexToNum(HexStr[0])*16+HexToNum(HexStr[1]);
      Password++;
      HexStr+=2;
    }
    *Password=0;
  }
  else
    strcpy(Password,HexStr);
}


int FTP::HexToNum(int Hex)
{
  if (Hex>=0 && Hex<='9')
    return(Hex-'0');
  return(toupper(Hex)-'A'+10);
}


void FTP::SelectTable()
{
  struct FarMenuItem MenuItems[16];
  memset(MenuItems,0,sizeof(MenuItems));
  MenuItems[((Connection *)hConnect)->TableNum].Selected=TRUE;
  strcpy(MenuItems[0].Text,"Windows");
  strcpy(MenuItems[1].Text,"DOS");
  strcpy(MenuItems[2].Text,GetMsg(MTableAuto));
  int TableNum=3;
  while (TableNum<sizeof(MenuItems)/sizeof(MenuItems[0]))
  {
    CharTableSet TableSet;
    if (Info.CharTable(TableNum-3,(char *)&TableSet,sizeof(TableSet))==-1)
      break;
    strcpy(MenuItems[TableNum++].Text,TableSet.TableName);
  }

  int ExitCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT,
      GetMsg(MTableTitle),NULL,NULL,NULL,NULL,MenuItems,TableNum);
  if (ExitCode<0)
    return;
  if (hConnect)
    ((Connection *)hConnect)->SetTable(ExitCode);
  char TableName[100];
  switch(ExitCode)
  {
    case 0:
      strcpy(TableName,".win");
      break;
    case 1:
      strcpy(TableName,".dos");
      break;
    case 2:
      strcpy(TableName,".auto");
      break;
    default:
      strcpy(TableName,MenuItems[ExitCode].Text);
      break;
  }
  SetRegKey(HKEY_CURRENT_USER,"","CharTable",TableName);
}


void FTP::SelectHostTable(char *TableName)
{
  struct FarMenuItem MenuItems[16];
  memset(MenuItems,0,sizeof(MenuItems));
  strcpy(MenuItems[0].Text,GetMsg(MTableDefault));
  strcpy(MenuItems[1].Text,"Windows");
  strcpy(MenuItems[2].Text,"DOS");
  strcpy(MenuItems[3].Text,GetMsg(MTableAuto));
  if (strcmp(TableName,".win")==0)
    MenuItems[1].Selected=TRUE;
  if (strcmp(TableName,".dos")==0)
    MenuItems[2].Selected=TRUE;
  if (strcmp(TableName,".auto")==0)
    MenuItems[3].Selected=TRUE;

  int TableNum=4;
  while (TableNum<sizeof(MenuItems)/sizeof(MenuItems[0]))
  {
    CharTableSet TableSet;
    if (Info.CharTable(TableNum-4,(char *)&TableSet,sizeof(TableSet))==-1)
      break;
    if (strcmp(TableName,TableSet.TableName)==0)
      MenuItems[TableNum].Selected=TRUE;
    strcpy(MenuItems[TableNum++].Text,TableSet.TableName);
  }

  int ExitCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT,
      GetMsg(MTableTitle),NULL,NULL,NULL,NULL,MenuItems,TableNum);
  if (ExitCode<0)
    return;
  switch(ExitCode)
  {
    case 0:
      *TableName=0;
      break;
    case 1:
      strcpy(TableName,".win");
      break;
    case 2:
      strcpy(TableName,".dos");
      break;
    case 3:
      strcpy(TableName,".auto");
      break;
    default:
      strcpy(TableName,MenuItems[ExitCode].Text);
      break;
  }
}


int FTP::TableNameToValue(char *TableName)
{
  if (strcmp(TableName,".win")==0)
    return(0);
  if (strcmp(TableName,".dos")==0)
    return(1);
  if (strcmp(TableName,".auto")==0)
    return(2);
  for (int I=0;;I++)
  {
    CharTableSet TableSet;
    if (Info.CharTable(I,(char *)&TableSet,sizeof(TableSet))==-1)
      break;
    if (strcmp(TableName,TableSet.TableName)==0)
      return(I+3);
  }
  return(0);
}


void FTP::SetAttributes()
{
  struct InitDialogItem InitItems[]=
  {
    DI_DOUBLEBOX,3,1,35,6,0,0,0,0,(char *)MChmodTitle,
    DI_TEXT,6,2,0,0,0,0,0,0,(char *)"R  W  X   R  W  X   R  W  X",
    DI_CHECKBOX,5,3,0,0,1,0,0,0,"",
    DI_CHECKBOX,8,3,0,0,0,0,0,0,"",
    DI_CHECKBOX,11,3,0,0,0,0,0,0,"",
    DI_CHECKBOX,15,3,0,0,0,0,0,0,"",
    DI_CHECKBOX,18,3,0,0,0,0,0,0,"",
    DI_CHECKBOX,21,3,0,0,0,0,0,0,"",
    DI_CHECKBOX,25,3,0,0,0,0,0,0,"",
    DI_CHECKBOX,28,3,0,0,0,0,0,0,"",
    DI_CHECKBOX,31,3,0,0,0,0,0,0,"",
    DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,5,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  struct PanelInfo PInfo;
  Info.Control(this,FCTL_GETPANELINFO,&PInfo);
  if (PInfo.SelectedItemsNumber==1)
  {
    DWORD Mode=PInfo.SelectedItems[0].UserData;
    for (int I=2; I<=10; I++)
      if (Mode & (1<<(10-I)))
        DialogItems[I].Selected=TRUE;
  }

  int AskCode=Info.Dialog(Info.ModuleNumber,-1,-1,39,8,"FTPCmd",DialogItems,
                          sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (AskCode!=12)
    return;
  DWORD Mode=0;
  for (int I=2;I<=10;I++)
    Mode=(Mode<<1)|DialogItems[I].Selected;

  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);

  ShellChmodMsg("",0);
  for (int I=0;I<PInfo.SelectedItemsNumber;I++)
  {
    char *CurName=PInfo.SelectedItems[I].FindData.cFileName;
    ShellChmodMsg(CurName,FMSG_KEEPBACKGROUND);
    if (!FtpChmod(hConnect,CurName,Mode))
    {
      char *MsgItems[]={GetMsg(MFtpTitle),GetMsg(MCannotChmod),CurName,GetMsg(MSkip),GetMsg(MCancel)};
      if (Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_DOWN|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)!=0)
        break;
    }
  }
  Info.RestoreScreen(hScreen);
}


void FTP::ShellChmodMsg(char *Name,int Flags)
{
  char ChmodName[NM];
  CenterStr(Name,ChmodName,30);
  TruncStr(ChmodName,30);
  char *MsgItems[]={GetMsg(MChmodTitle),GetMsg(MChangingAttr),ChmodName};
  Info.Message(Info.ModuleNumber,Flags,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
}


int FTP::ProcessEvent(int Event,void *Param)
{
  extern int BusyMode;
  if (BusyMode && Event==FE_BREAK && (int)Param==CTRL_BREAK_EVENT && hConnect)
  {
    ((Connection *)hConnect)->AbortAllRequest(TRUE);
    BusyMode = FALSE;
    ShowHosts=(StartViewMode!=-2);
  }
  if (Event==FE_REDRAW && ShowHosts && RereadRequired)
  {
    static int Busy;
    if (!Busy)
    {
      Busy=TRUE;
      RereadRequired=FALSE;
      Info.Control(this,FCTL_UPDATEPANEL,NULL);
      Busy=FALSE;
    }
  }

  return(FALSE);
}
