Plist::Plist()
{
  LastUpdateTime=0;
}


int Plist::GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  int RetCode;
  if (NT)
    RetCode=GetListNT(pPanelItem,pItemsNumber);
  else
    RetCode=GetList95(pPanelItem,pItemsNumber);
  LastUpdateTime=clock();
  return(RetCode);
}


void Plist::FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber)
{
  for (int I=0;I<ItemsNumber;I++)
  {
    delete PanelItem[I].Description;
    delete (ProcessData *)PanelItem[I].UserData;
  }
  delete PanelItem;
}


void Plist::GetOpenPluginInfo(struct OpenPluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=OPIF_ADDDOTS;

  Info->CurDir="";

  static char Title[100];
  sprintf(Title," %s ",GetMsg(MPlistPanel));
  Info->PanelTitle=Title;

  static struct PanelMode PanelModesArray[10];
  PanelModesArray[1].ColumnTypes="N,S,Z";
  PanelModesArray[1].ColumnWidths="14,2,0";
  PanelModesArray[1].FullScreen=FALSE;
  static char *ColumnTitles1[3];
  ColumnTitles1[0]=GetMsg(MColumnModule);
  ColumnTitles1[1]=GetMsg(MColumnPriority);
  ColumnTitles1[2]=GetMsg(MColumnTitle);
  PanelModesArray[1].ColumnTitles=ColumnTitles1;

  Info->PanelModesArray=PanelModesArray;
  Info->PanelModesNumber=sizeof(PanelModesArray)/sizeof(PanelModesArray[0]);
  Info->StartPanelMode='1';
  Info->StartSortMode=SM_UNSORTED;
}


int Plist::GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                  int Move,char *DestPath,int OpMode)
{
  if (ItemsNumber==0)
    return(0);
  for (int I=0;I<ItemsNumber;I++)
  {
    struct PluginPanelItem *CurItem=&PanelItem[I];
    struct ProcessData *pdata=(struct ProcessData *)CurItem->UserData;
    char FileName[NM];
    strcpy(FileName,DestPath);
    AddEndSlash(FileName);
    strcat(FileName,CurItem->FindData.cFileName);
    FILE *InfoFile=fopen(FileName,"wb");
    if (InfoFile==NULL)
      return(0);
    char AppType[100];
    if (!NT)
      sprintf(AppType,", %d%s",pdata->uAppType,GetMsg(MBits));
    else
      *AppType=0;

    fprintf(InfoFile,"%s %s%s\r\n",PrintTitle(MTitleModule),CurItem->FindData.cFileName,AppType);
    if (!NT)
      fprintf(InfoFile,"%s %s\r\n",PrintTitle(MTitleFullPath),pdata->FullPath);
    fprintf(InfoFile,"%s %08X\r\n",PrintTitle(MTitlePID),CurItem->PackSize);
    fprintf(InfoFile,"%s %d\r\n",PrintTitle(MTitlePriority),CurItem->FindData.nFileSizeLow);
    if (!NT)
      fprintf(InfoFile,"%s %d\r\n",PrintTitle(MTitleThreads),pdata->Threads);

    if (NT)
    {
      HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,CurItem->PackSize);
      if (hProcess!=NULL)
      {
        FILETIME Creation,Exit,Kernel,User;
        if (GetProcessTimes(hProcess,&Creation,&Exit,&Kernel,&User))
        {
          SYSTEMTIME Current,Compare;
          FILETIME CurFileTime;

          GetSystemTimeAsFileTime(&CurFileTime);

          GetLocalTime(&Current);
          FileTimeToSystemTime(&Creation,&Compare);
          SystemTimeToTzSpecificLocalTime(NULL,&Compare,&Compare);

          fprintf(InfoFile,"\r\n");

          char DateText[50],TimeText[50];
          if (Creation.dwHighDateTime!=0 || Creation.dwLowDateTime!=0)
          {
            ConvertDate(&Creation,DateText,TimeText,5);
  
            if (Current.wYear!=Compare.wYear || Current.wMonth!=Compare.wMonth ||
                Current.wDay!=Compare.wDay)
              fprintf(InfoFile,"%s %s %s\r\n",PrintTitle(MTitleStarted),DateText,TimeText);
            else
              fprintf(InfoFile,"%s %s\r\n",PrintTitle(MTitleStarted),TimeText);
  
            FileTimeToText(&CurFileTime,&Creation,TimeText);
            fprintf(InfoFile,"%s %s\r\n",PrintTitle(MTitleUptime),TimeText);
          }

          FILETIME CPUTime;
          CPUTime.dwHighDateTime=Kernel.dwHighDateTime+User.dwHighDateTime;
          CPUTime.dwLowDateTime=Kernel.dwLowDateTime+User.dwLowDateTime;
          if (CPUTime.dwLowDateTime<Kernel.dwLowDateTime)
            CPUTime.dwHighDateTime++;

          FileTimeToText(NULL,&CPUTime,TimeText);
          fprintf(InfoFile,"%s %s\r\n",PrintTitle(MTitleCPUTime),TimeText);

          FileTimeToText(NULL,&Kernel,TimeText);
          fprintf(InfoFile,"%s %s\r\n",PrintTitle(MTitleKernelTime),TimeText);

          FileTimeToText(NULL,&User,TimeText);
          fprintf(InfoFile,"%s %s\r\n",PrintTitle(MTitleUserTime),TimeText);

        }
        CloseHandle(hProcess);
      }
    }

    fprintf(InfoFile,"\r\n%s %s\r\n",PrintTitle(MTitleWindow),NullToEmpty(CurItem->Description));
    fprintf(InfoFile,"%-15s %08X\r\n","HWND:",pdata->hwnd);
    LONG Style=0,ExtStyle=0;
    if (pdata->hwnd!=NULL)
    {
      Style=GetWindowLong(pdata->hwnd,GWL_STYLE);
      ExtStyle=GetWindowLong(pdata->hwnd,GWL_EXSTYLE);
    }

    static int Styles[]={
      WS_POPUP,WS_CHILD,WS_MINIMIZE,WS_VISIBLE,WS_DISABLED,
      WS_CLIPSIBLINGS,WS_CLIPCHILDREN,WS_MAXIMIZE,WS_BORDER,WS_DLGFRAME,
      WS_VSCROLL,WS_HSCROLL,WS_SYSMENU,WS_THICKFRAME,WS_MINIMIZEBOX,
      WS_MAXIMIZEBOX
    };

    static char *StrStyles[]={
      "WS_POPUP","WS_CHILD","WS_MINIMIZE","WS_VISIBLE","WS_DISABLED",
      "WS_CLIPSIBLINGS","WS_CLIPCHILDREN","WS_MAXIMIZE","WS_BORDER","WS_DLGFRAME",
      "WS_VSCROLL","WS_HSCROLL","WS_SYSMENU","WS_THICKFRAME","WS_MINIMIZEBOX",
      "WS_MAXIMIZEBOX"
    };

    static int ExtStyles[]={
      WS_EX_DLGMODALFRAME,WS_EX_NOPARENTNOTIFY,WS_EX_TOPMOST,
      WS_EX_ACCEPTFILES,WS_EX_TRANSPARENT,WS_EX_MDICHILD,
      WS_EX_TOOLWINDOW,WS_EX_WINDOWEDGE,WS_EX_CLIENTEDGE,WS_EX_CONTEXTHELP,
      WS_EX_RIGHT,WS_EX_RTLREADING,WS_EX_LEFTSCROLLBAR,WS_EX_CONTROLPARENT,
      WS_EX_STATICEDGE,WS_EX_APPWINDOW
    };

    static char *StrExtStyles[]={
      "WS_EX_DLGMODALFRAME","WS_EX_NOPARENTNOTIFY","WS_EX_TOPMOST",
      "WS_EX_ACCEPTFILES","WS_EX_TRANSPARENT","WS_EX_MDICHILD",
      "WS_EX_TOOLWINDOW","WS_EX_WINDOWEDGE","WS_EX_CLIENTEDGE","WS_EX_CONTEXTHELP",
      "WS_EX_RIGHT","WS_EX_RTLREADING","WS_EX_LEFTSCROLLBAR","WS_EX_CONTROLPARENT",
      "WS_EX_STATICEDGE","WS_EX_APPWINDOW"
    };

    char StyleStr[1024],ExtStyleStr[1024];
    *StyleStr=*ExtStyleStr=0;
    for (int I=0;I<sizeof(Styles)/sizeof(Styles[0]);I++)
      if (Style & Styles[I])
      {
        strcat(StyleStr," ");
        strcat(StyleStr,StrStyles[I]);
      }
    for (int I=0;I<sizeof(ExtStyles)/sizeof(ExtStyles[0]);I++)
      if (Style & ExtStyles[I])
      {
        strcat(ExtStyleStr," ");
        strcat(ExtStyleStr,StrExtStyles[I]);
      }

    fprintf(InfoFile,"%s %08X %s\r\n",PrintTitle(MTitleStyle),Style,StyleStr);
    fprintf(InfoFile,"%s %08X %s\r\n",PrintTitle(MTitleExtStyle),ExtStyle,ExtStyleStr);

    fclose(InfoFile);
  }
  return(1);
}


int Plist::DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                     int OpMode)
{
  if (ItemsNumber==0)
    return(FALSE);
  char *MsgItems[]={GetMsg(MDeleteTitle),GetMsg(MDeleteProcesses),
                    GetMsg(MDeleteDelete),GetMsg(MCancel)};
  char Msg[512];
  if (ItemsNumber==1)
  {
    sprintf(Msg,GetMsg(MDeleteProcess),PanelItem[0].FindData.cFileName);
    MsgItems[1]=Msg;
  }
  if (Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)!=0)
    return(FALSE);
  if (ItemsNumber>1)
  {
    char Msg[512];
    sprintf(Msg,GetMsg(MDeleteNumberOfProcesses),ItemsNumber);
    MsgItems[1]=Msg;
    if (Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)!=0)
      return(FALSE);
  }
  for (int I=0;I<ItemsNumber;I++)
  {
    struct PluginPanelItem *CurItem=&PanelItem[I];
    struct ProcessData *pdata=(struct ProcessData *)CurItem->UserData;
    int Success;
    if (NT)
      Success=KillProcessNT(CurItem->PackSize,pdata->hwnd);
    else
      Success=KillProcess(CurItem->PackSize);
    if (!Success)
    {
      char Msg[512];
      sprintf(Msg,GetMsg(MCannotDelete),CurItem->FindData.cFileName);
      char *MsgItems[]={GetMsg(MDeleteTitle),Msg,GetMsg(MOk)};
      Info.Message(Info.ModuleNumber,FMSG_WARNING|FMSG_ERRORTYPE,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
      return(FALSE);
    }
  }
  return(TRUE);
}


int Plist::ProcessEvent(int Event,void *Param)
{
  if (Event==FE_IDLE && clock()-LastUpdateTime>1000)
  {
    Info.Control(this,FCTL_UPDATEPANEL,(void *)1);
    Info.Control(this,FCTL_REDRAWPANEL,NULL);

    struct PanelInfo PInfo;
    Info.Control(this,FCTL_GETANOTHERPANELINFO,&PInfo);
    if (PInfo.PanelType==PTYPE_QVIEWPANEL)
    {
      Info.Control(this,FCTL_UPDATEANOTHERPANEL,(void *)1);
      Info.Control(this,FCTL_REDRAWANOTHERPANEL,NULL);
    }
  }
  return(FALSE);
}


int Plist::ProcessKey(int Key,unsigned int ControlState)
{
  if (ControlState==0 && Key==VK_RETURN)
  {
    struct PanelInfo PInfo;
    Info.Control(this,FCTL_GETPANELINFO,&PInfo);
    if (PInfo.CurrentItem<PInfo.ItemsNumber)
    {
      PluginPanelItem *CurItem=&PInfo.PanelItems[PInfo.CurrentItem];
      struct ProcessData *pdata=(struct ProcessData *)CurItem->UserData;
      if (pdata==NULL)
        return(FALSE);
      if (pdata->hwnd!=NULL)
      {
        SetForegroundWindow(pdata->hwnd);

        WINDOWPLACEMENT wp;
        wp.length=sizeof(wp);
        if (!GetWindowPlacement(pdata->hwnd,&wp) || wp.showCmd!=SW_SHOWMAXIMIZED)
          ShowWindowAsync(pdata->hwnd,SW_RESTORE);
      }
    }
    return(TRUE);
  }
  return(FALSE);
}


char *Plist::PrintTitle(int MsgId)
{
  static char FullStr[256];
  char Str[256];
  sprintf(Str,"%s:",GetMsg(MsgId));
  sprintf(FullStr,"%-15s",Str);
  return(FullStr);
}


void Plist::FileTimeToText(FILETIME *CurFileTime,FILETIME *SrcTime,char *TimeText)
{
  FILETIME Uptime;
  if (CurFileTime==NULL)
  {
    Uptime.dwHighDateTime=SrcTime->dwHighDateTime;
    Uptime.dwLowDateTime=SrcTime->dwLowDateTime;
  }
  else
  {
    Uptime.dwHighDateTime=CurFileTime->dwHighDateTime-SrcTime->dwHighDateTime;
    if (CurFileTime->dwLowDateTime<SrcTime->dwLowDateTime)
      Uptime.dwHighDateTime--;
    Uptime.dwLowDateTime=CurFileTime->dwLowDateTime-SrcTime->dwLowDateTime;
  }

  SYSTEMTIME st;
  FileTimeToSystemTime(&Uptime,&st);
  int Days=st.wDay-1;
  for (int I=1;I<st.wMonth;I++)
  {
    static int MonthDays[12]={31,29,31,30,31,30,31,31,30,31,30,31};
    Days+=MonthDays[I-1];
  }
  if (Days>0)
    sprintf(TimeText,"%d %02d:%02d:%02d",Days,st.wHour,st.wMinute,st.wSecond);
  else
    sprintf(TimeText,"%02d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);
}
