/*
Modify:
  13.04.2001 DJ
   - перенос сброса Opt.Background в PutFiles в более подходящее место
*/

void PluginClass::GetCommandFormat(int Command,char *Format,int FormatSize)
{
  *Format=0;
  char ArcFormat[100],DefExt[NM];
  if (!ArcPlugin->GetFormatName(ArcPluginNumber,ArcPluginType,ArcFormat,DefExt))
    return;
  GetRegKey(HKEY_LOCAL_MACHINE,ArcFormat,CmdNames[Command],Format,"",FormatSize);
  if (*Format==0)
    ArcPlugin->GetDefaultCommands(ArcPluginNumber,ArcPluginType,Command,Format);
}


int PluginClass::DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  char Command[512],AllFilesMask[32];
  if (ItemsNumber==0)
    return(FALSE);
  if ((OpMode & OPM_SILENT)==0)
  {
    const char *MsgItems[]={GetMsg(MDeleteTitle),GetMsg(MDeleteFiles),
                      GetMsg(MDeleteDelete),GetMsg(MDeleteCancel)};
    char Msg[512];
    if (ItemsNumber==1)
    {
      FSF.sprintf(Msg,GetMsg(MDeleteFile),PanelItem[0].FindData.cFileName);
      MsgItems[1]=Msg;
    }
    if (Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)!=0)
      return(FALSE);
    if (ItemsNumber>1)
    {
      char Msg[100];
      FSF.sprintf(Msg,GetMsg(MDeleteNumberOfFiles),ItemsNumber);
      MsgItems[1]=Msg;
      if (Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)!=0)
        return(FALSE);
    }
  }
  GetCommandFormat(CMD_DELETE,Command,sizeof(Command));
  GetCommandFormat(CMD_ALLFILESMASK,AllFilesMask,sizeof(AllFilesMask));
  int IgnoreErrors=(CurArcInfo.Flags & AF_IGNOREERRORS);
  ArcCommand ArcCmd(PanelItem,ItemsNumber,Command,ArcName,CurDir,"",AllFilesMask,IgnoreErrors,0,0,CurDir,CurArcInfo.Prefix);
  if (!IgnoreErrors && ArcCmd.GetExecCode()!=0)
    return(FALSE);
  if (Opt.UpdateDescriptions)
    for (int I=0;I<ItemsNumber;I++)
      PanelItem[I].Flags|=PPIF_PROCESSDESCR;
  return(TRUE);
}


int PluginClass::ProcessHostFile(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  struct FarMenuItem MenuItems[7];
  memset(MenuItems,0,sizeof(MenuItems));
  MenuItems[0].Selected=TRUE;
  strcpy(MenuItems[0].Text,GetMsg(MArcCmdTest));
  strcpy(MenuItems[1].Text,GetMsg(MArcCmdComment));
  strcpy(MenuItems[2].Text,GetMsg(MArcCmdCommentFiles));
  strcpy(MenuItems[3].Text,GetMsg(MArcCmdSFX));
  strcpy(MenuItems[4].Text,GetMsg(MArcCmdRecover));
  strcpy(MenuItems[5].Text,GetMsg(MArcCmdProtect));
  strcpy(MenuItems[6].Text,GetMsg(MArcCmdLock));
  int ExitCode=Info.Menu(Info.ModuleNumber,-1,-1,0,0,GetMsg(MArcCmdTitle),NULL,
      "ArcCmd",NULL,NULL,MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));
  if (ExitCode<0)
    return(FALSE);
  FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
  char Command[512],AllFilesMask[32];
  const int Commands[]={CMD_TEST,CMD_COMMENT,CMD_COMMENTFILES,CMD_SFX,
                        CMD_RECOVER,CMD_PROTECT,CMD_LOCK};
  int CommandType=Commands[ExitCode];
  GetCommandFormat(CommandType,Command,sizeof(Command));
  GetCommandFormat(CMD_ALLFILESMASK,AllFilesMask,sizeof(AllFilesMask));
  int IgnoreErrors=(CurArcInfo.Flags & AF_IGNOREERRORS);
  char Password[256];
  *Password=0;

  int AskVolume=(OpMode & (OPM_FIND|OPM_VIEW|OPM_EDIT))==0 &&
                CurArcInfo.Volume && *CurDir==0 && ExitCode==0;
  struct PluginPanelItem MaskPanelItem;

  if (AskVolume)
  {
    char VolMsg[300];
    FSF.sprintf(VolMsg,GetMsg(MExtrVolume),FSF.PointToName(ArcName));
    const char *MsgItems[]={"",VolMsg,GetMsg(MExtrVolumeAsk1),
                      GetMsg(MExtrVolumeAsk2),GetMsg(MExtrVolumeSelFiles),
                      GetMsg(MExtrAllVolumes)};
    int MsgCode=Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2);
    if (MsgCode<0)
      return(-1);
    if (MsgCode==1)
    {
      memset(&MaskPanelItem,0,sizeof(MaskPanelItem));
      strcpy(MaskPanelItem.FindData.cFileName,AllFilesMask);
      strcpy(MaskPanelItem.FindData.cAlternateFileName,AllFilesMask);
      if (ItemsInfo.Encrypted)
        MaskPanelItem.Flags=F_ENCRYPTED;
      PanelItem=&MaskPanelItem;
      ItemsNumber=1;
    }
  }


  if (strstr(Command,"%%P")!=NULL)
    for (int I=0;I<ItemsNumber;I++)
      if ((PanelItem[I].Flags & F_ENCRYPTED) || ItemsInfo.Encrypted &&
          (PanelItem[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
        if (!GetPassword(Password,FSF.PointToName(ArcName)))
          return(FALSE);
        break;
      }

  ArcCommand ArcCmd(PanelItem,ItemsNumber,Command,ArcName,"",Password,AllFilesMask,
                    IgnoreErrors,CommandType==CMD_COMMENT || CommandType==CMD_COMMENTFILES ? 2:0,0,
                    CurDir,CurArcInfo.Prefix);
  return(IgnoreErrors || ArcCmd.GetExecCode()==0);
}


int PluginClass::GetFiles(struct PluginPanelItem *PanelItem,
                          int ItemsNumber,
                          int Move,
                          char *DestPath,
                          int OpMode)
{
  char SaveDir[NM];
  GetCurrentDirectory(sizeof(SaveDir),SaveDir);
  char Command[512],AllFilesMask[32];
  if (ItemsNumber==0)
    return(0);
  if (*DestPath)
    FSF.AddEndSlash(DestPath);
  const char *PathHistoryName="ExtrDestPath";
  struct InitDialogItem InitItems[]={
  /* 0 */  DI_DOUBLEBOX,3,1,72,12,0,0,0,0,(char *)MExtractTitle,
  /* 1 */  DI_TEXT,5,2,0,0,0,0,0,0,(char *)MExtractTo,
  /* 2 */  DI_EDIT,5,3,70,3,1,(DWORD)PathHistoryName,DIF_HISTORY,0,DestPath,
  /* 3 */  DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 4 */  DI_TEXT,5,5,0,0,0,0,0,0,(char *)MExtrPassword,
  /* 5 */  DI_PSWEDIT,5,6,35,5,0,0,0,0,"",
  /* 6 */  DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 7 */  DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MExtrWithoutPaths,
  /* 8 */  DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MBackground,
  /* 9 */  DI_TEXT,3,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /*10 */  DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,1,(char *)MExtrExtract,
  /*11 */  DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,0,(char *)MExtrCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  int AskVolume=(OpMode & (OPM_FIND|OPM_VIEW|OPM_EDIT))==0 &&
                CurArcInfo.Volume && *CurDir==0;

  if (!AskVolume)
  {
    DialogItems[7].Selected=TRUE;
    for (int I=0;I<ItemsNumber;I++)
      if (PanelItem[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        DialogItems[7].Selected=FALSE;
        break;
      }
  }

  /* $ 14.02.2001 raVen
     сброс галки "фоновая архивация" */
  Opt.UserBackground=0;
  /* raVen $ */

  DialogItems[8].Selected=Opt.UserBackground;

  if ((OpMode & OPM_SILENT)==0)
  {
    int AskCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,14,"ExtrFromArc",
                DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
    if (AskCode!=10)
      return(-1);
    strcpy(DestPath,DialogItems[2].Data);
    Opt.UserBackground=DialogItems[8].Selected;
    SetRegKey(HKEY_CURRENT_USER,"","Background",Opt.UserBackground);
  }

  LastWithoutPathsState=DialogItems[7].Selected;

  Opt.Background=OpMode & OPM_SILENT ? 0 : Opt.UserBackground;

  int SpaceOnly=TRUE;
  for (int I=0;DestPath[I]!=0;I++)
    if (DestPath[I]!=' ')
    {
      SpaceOnly=FALSE;
      break;
    }

  if (!SpaceOnly)
  {
    for (char *ChPtr=DestPath;*ChPtr!=0;ChPtr++)
      if (*ChPtr=='\\')
      {
        *ChPtr=0;
        CreateDirectory(DestPath,NULL);
        *ChPtr='\\';
      }
    CreateDirectory(DestPath,NULL);
  }


  if (*DestPath && DestPath[strlen(DestPath)-1]!=':')
    FSF.AddEndSlash(DestPath);
  GetCommandFormat(CMD_ALLFILESMASK,AllFilesMask,sizeof(AllFilesMask));

  struct PluginPanelItem MaskPanelItem;

  if (AskVolume)
  {
    char VolMsg[300];
    FSF.sprintf(VolMsg,GetMsg(MExtrVolume),FSF.PointToName(ArcName));
    const char *MsgItems[]={GetMsg(MExtractTitle),VolMsg,GetMsg(MExtrVolumeAsk1),
                      GetMsg(MExtrVolumeAsk2),GetMsg(MExtrVolumeSelFiles),
                      GetMsg(MExtrAllVolumes)};
    int MsgCode=Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2);
    if (MsgCode<0)
      return(-1);
    if (MsgCode==1)
    {
      memset(&MaskPanelItem,0,sizeof(MaskPanelItem));
      strcpy(MaskPanelItem.FindData.cFileName,AllFilesMask);
      strcpy(MaskPanelItem.FindData.cAlternateFileName,AllFilesMask);
      if (ItemsInfo.Encrypted)
        MaskPanelItem.Flags=F_ENCRYPTED;
      PanelItem=&MaskPanelItem;
      ItemsNumber=1;
    }
  }

  int CommandType=LastWithoutPathsState ? CMD_EXTRACTWITHOUTPATH:CMD_EXTRACT;
  GetCommandFormat(CommandType,Command,sizeof(Command));

  if (*DialogItems[5].Data==0 && strstr(Command,"%%P")!=NULL)
    for (int I=0;I<ItemsNumber;I++)
      if ((PanelItem[I].Flags & F_ENCRYPTED) || ItemsInfo.Encrypted &&
          (PanelItem[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
        if (!GetPassword(DialogItems[5].Data,FSF.PointToName(ArcName)))
          return(-1);
        break;
      }

  SetCurrentDirectory(DestPath);
  int SaveHideOut=Opt.HideOutput;
  if (OpMode & OPM_FIND)
    Opt.HideOutput=2;
  int IgnoreErrors=(CurArcInfo.Flags & AF_IGNOREERRORS);
  ArcCommand ArcCmd(PanelItem,ItemsNumber,Command,ArcName,CurDir,
             DialogItems[5].Data,AllFilesMask,IgnoreErrors,
             (OpMode & OPM_VIEW)!=0,(OpMode & OPM_FIND),CurDir,CurArcInfo.Prefix);
  Opt.HideOutput=SaveHideOut;
  SetCurrentDirectory(SaveDir);
  if (!IgnoreErrors && ArcCmd.GetExecCode()!=0)
    if (!(OpMode & OPM_VIEW))
      return(0);
  if (Move)
    DeleteFiles(PanelItem,ItemsNumber,TRUE);
  if (Opt.UpdateDescriptions)
    for (int I=0;I<ItemsNumber;I++)
      PanelItem[I].Flags|=PPIF_PROCESSDESCR;
  return(1);
}

struct PutDlgData{
  PluginClass *Self;
  char ArcFormat[NM];
  char OriginalName[512];
  char Password1[256];
  char Password2[256];
  char DefExt[NM];
};

#define MAM_SETDISABLE   DM_USER+1
#define MAM_ARCSWITCHES  DM_USER+2
#define MAM_SETNAME      DM_USER+3

long WINAPI PluginClass::PutDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  char Buffer[512];
  struct PutDlgData *pdd=(struct PutDlgData*)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);

  if(Msg == DN_INITDIALOG)
  {
    Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,7,255);
    Info.SendDlgMessage(hDlg,DM_SETTEXTLENGTH,9,255);
    Info.SendDlgMessage(hDlg,MAM_SETDISABLE,0,0);
    Info.SendDlgMessage(hDlg,MAM_ARCSWITCHES,0,0);

    GetRegKey(HKEY_CURRENT_USER,pdd->ArcFormat,"AddSwitches",Buffer,"",sizeof(Buffer));
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,4,(long)Buffer);

    FSF.sprintf(Buffer,GetMsg(MAddTitle),pdd->ArcFormat);
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,0,(long)Buffer);

    Info.SendDlgMessage(hDlg,MAM_SETNAME,0,0);
    return TRUE;
  }
  else if(Msg == DN_EDITCHANGE)
  {
    if(Param1 == 2)
    {
      FarDialogItem *Item=(FarDialogItem *)Param2;
      Info.SendDlgMessage(hDlg,DM_ENABLE,14,Item->Data[0] != 0);
    }
  }
  else if(Msg == DN_BTNCLICK)
  {
    switch(Param1)
    {
      case 15:
      {
        if (!pdd->Self->SelectFormat(pdd->ArcFormat,TRUE))
          Info.SendDlgMessage(hDlg,DM_CLOSE,17,0);
        else
        {
          pdd->Self->FormatToPlugin(pdd->ArcFormat,pdd->Self->ArcPluginNumber,pdd->Self->ArcPluginType);

          GetRegKey(HKEY_LOCAL_MACHINE,pdd->ArcFormat,"DefExt",Buffer,"",sizeof(Buffer));
          BOOL Ret=TRUE;
          if (*Buffer==0)
            Ret=ArcPlugin->GetFormatName(pdd->Self->ArcPluginNumber,pdd->Self->ArcPluginType,pdd->ArcFormat,Buffer);
          if(!Ret)
            pdd->DefExt[0]=0;
          else
            strncpy(pdd->DefExt,Buffer,sizeof(pdd->DefExt));

          FSF.sprintf(Buffer,GetMsg(MAddTitle),pdd->ArcFormat);
          Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,0,(long)Buffer);

          Info.SendDlgMessage(hDlg,MAM_SETDISABLE,0,0);
          Info.SendDlgMessage(hDlg,MAM_ARCSWITCHES,0,0);
          Info.SendDlgMessage(hDlg,MAM_SETNAME,0,0);
          Info.SendDlgMessage(hDlg,DM_SETFOCUS,2,0);
        }
        return TRUE;
      }

      case 17:
        break;

      case 14:
      {
        // проверка совпадения введенного пароля и подтверждения
        char Password1[256],Password2[256];
        Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,7,(long)Password1);
        Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,9,(long)Password2);
        if (strcmp(Password1,Password2))
        {
          const char *MsgItems[]={GetMsg(MError),GetMsg(MAddPswNotMatch),GetMsg(MOk)};
          Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
          return TRUE;
        }
        break;
      }

      case 16:
      {
        SetRegKey(HKEY_CURRENT_USER,"","DefaultFormat",pdd->ArcFormat);
        Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,4,(long)Buffer);
        SetRegKey(HKEY_CURRENT_USER,pdd->ArcFormat,"AddSwitches",Buffer);
        return TRUE;
      }
    }
  }
  else if(Msg == DN_CLOSE)
  {
    return Info.SendDlgMessage(hDlg,DM_ENABLE,14,-1) == TRUE ||
           Param1 < 0 ||
           Param1 == 17;
  }
  else if(Msg == MAM_SETDISABLE)
  {
    GetRegKey(HKEY_LOCAL_MACHINE,pdd->ArcFormat,CmdNames[CMD_ADD],Buffer,"",sizeof(Buffer));
    if(*Buffer == 0)
      ArcPlugin->GetDefaultCommands(pdd->Self->ArcPluginNumber,pdd->Self->ArcPluginType,CMD_ADD,Buffer);
    Info.SendDlgMessage(hDlg,DM_ENABLE,14,*Buffer != 0);
  }
  else if(Msg == MAM_ARCSWITCHES)
  {
    static char SwHistoryName[NM];
    FSF.sprintf(SwHistoryName,"ArcSwitches\\%s",pdd->ArcFormat);
    Info.SendDlgMessage(hDlg,DM_SETHISTORY,4,(long)SwHistoryName);
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,4,(long)"");
  }
  else if(Msg == MAM_SETNAME)
  {
    char ExtName[NM],*ExtPtr;
    strcpy(Buffer,pdd->OriginalName);
    if ((ExtPtr=strrchr(Buffer,'.'))==NULL ||
        (ExtPtr && stricmp(++ExtPtr,pdd->DefExt) != 0))
    {
      if(Opt.AddExtArchive)
      {
        FSF.sprintf(ExtName,"%s.%s",Buffer,pdd->DefExt);
        strcpy(Buffer,ExtName);
      }
    }
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(long)Buffer);
  }

  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int PluginClass::PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                          int Move,int OpMode)
{
  if (ItemsNumber==0)
    return(0);

  char Command[512],AllFilesMask[32];
  int ArcExitCode=1;
  struct PutDlgData pdd={0};
  char FullName[NM],ExtName[NM],*NamePtr,*ExtPtr;
  BOOL Ret=TRUE;

  pdd.Self=this;

  *pdd.Password1=*pdd.Password2=0;

  if (ArcPluginNumber==-1)
  {
    char DefaultFormat[100];
    GetRegKey(HKEY_CURRENT_USER,"","DefaultFormat",DefaultFormat,"",sizeof(DefaultFormat));
    if (!FormatToPlugin(DefaultFormat,ArcPluginNumber,ArcPluginType))
      ArcPluginNumber=ArcPluginType=0;
    strcpy(pdd.ArcFormat,DefaultFormat);
  }

  /* $ 14.02.2001 raVen
     сброс галки "фоновая архивация" */
  /* $ 13.04.2001 DJ
     перенесен в более подходящее место */
  Opt.UserBackground=0;
  /* DJ $ */
  /* raVen $ */
  while (1)
  {
    GetRegKey(HKEY_LOCAL_MACHINE,pdd.ArcFormat,"DefExt",pdd.DefExt,"",sizeof(pdd.DefExt));
    if (*pdd.DefExt==0)
      Ret=ArcPlugin->GetFormatName(ArcPluginNumber,ArcPluginType,pdd.ArcFormat,pdd.DefExt);
    if (!Ret)
      return(0);

    const char *ArcHistoryName="ArcName";
    struct InitDialogItem InitItems[]={
      /* 0*/DI_DOUBLEBOX,3,1,72,14,0,0,0,0,"",
      /* 1*/DI_TEXT,5,2,0,0,0,0,0,0,(char *)MAddToArc,
      /* 2*/DI_EDIT,5,3,70,3,1,(DWORD)ArcHistoryName,DIF_HISTORY,0,"",
      /* 3*/DI_TEXT,5,4,0,0,0,0,0,0,(char *)MAddSwitches,
      /* 4*/DI_EDIT,5,5,70,3,0,0,DIF_HISTORY,0,"",
      /* 5*/DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /* 6*/DI_TEXT,5,7,0,0,0,0,0,0,(char *)MAddPassword,
      /* 7*/DI_PSWEDIT,5,8,35,8,0,0,0,0,"",
      /* 8*/DI_TEXT,40,7,0,0,0,0,0,0,(char *)MAddReenterPassword,
      /* 9*/DI_PSWEDIT,40,8,70,8,0,0,0,0,"",
      /*10*/DI_TEXT,3,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /*11*/DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MAddDelete,
      /*12*/DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MBackground,
      /*13*/DI_TEXT,3,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /*14*/DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,1,(char *)MAddAdd,
      /*15*/DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(char *)MAddSelect,
      /*16*/DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(char *)MAddSave,
      /*17*/DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(char *)MAddCancel
    };
    struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
    InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

    if(Opt.UseLastHistory)
      DialogItems[4].Flags|=DIF_USELASTHISTORY;

    if (*ArcName)
      strcpy(DialogItems[2].Data,ArcName);
    else
    {
      if (ItemsNumber==1)
      {
        strcpy(DialogItems[2].Data,PanelItem->FindData.cFileName);
        if(Opt.DeleteExtFile)
        {
          char *Dot=strrchr(DialogItems[2].Data,'.');
          if (Dot!=NULL)
            *Dot=0;
        }
      }
      else
      {
        char CurDir[NM];
        GetCurrentDirectory(sizeof(CurDir),CurDir);
        strcpy(DialogItems[2].Data,FSF.PointToName(CurDir));
      }
      char AnsiName[NM];
      OemToAnsi(DialogItems[2].Data,AnsiName);
      if (!IsCaseMixed(AnsiName))
      {
        CharLower(AnsiName);
        AnsiToOem(AnsiName,DialogItems[2].Data);
      }
    }
    DialogItems[11].Selected=Move;
    /* $ 13.04.2001 DJ
       UserBackground instead of Background
    */
    DialogItems[12].Selected=Opt.UserBackground;
    /* DJ $ */
    strcpy(pdd.OriginalName,DialogItems[2].Data);

//    if(!Opt.DeleteExtFile)

    {
      if ((ExtPtr=strrchr(DialogItems[2].Data,'.'))==NULL ||
          (ExtPtr && stricmp(++ExtPtr,pdd.DefExt) != 0))
      {
        if(Opt.AddExtArchive)
        {
          FSF.sprintf(ExtName,"%s.%s",DialogItems[2].Data,pdd.DefExt);
          strcpy(DialogItems[2].Data,ExtName);
        }
      }
    }

    if ((OpMode & OPM_SILENT)==0)
    {
      int AskCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,16,"AddToArc",
                  DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),
                  0,0,PluginClass::PutDlgProc,(long)&pdd);

      strcpy(pdd.Password1,DialogItems[7].Data);
      strcpy(pdd.Password2,DialogItems[9].Data);
      Opt.UserBackground=DialogItems[12].Selected;
      if (AskCode!=14 || *DialogItems[2].Data==0)
        return(-1);
      SetRegKey(HKEY_CURRENT_USER,"","Background",Opt.UserBackground);
    }

    int Recurse=FALSE;
    for (int I=0;I<ItemsNumber;I++)
      if (PanelItem[I].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        Recurse=TRUE;
        break;
      }
    int CommandType;
    if (DialogItems[11].Selected)
      CommandType=Recurse ? CMD_MOVERECURSE:CMD_MOVE;
    else
      CommandType=Recurse ? CMD_ADDRECURSE:CMD_ADD;

    Opt.Background=OpMode & OPM_SILENT ? 0 : Opt.UserBackground;

    GetRegKey(HKEY_LOCAL_MACHINE,pdd.ArcFormat,CmdNames[CommandType],Command,"",sizeof(Command));
    if (*Command==0)
      ArcPlugin->GetDefaultCommands(ArcPluginNumber,ArcPluginType,CommandType,Command);
    GetRegKey(HKEY_LOCAL_MACHINE,pdd.ArcFormat,"AllFilesMask",AllFilesMask,"",sizeof(AllFilesMask));
    if (*AllFilesMask==0)
      ArcPlugin->GetDefaultCommands(ArcPluginNumber,ArcPluginType,CMD_ALLFILESMASK,AllFilesMask);
    if (*CurDir && strstr(Command,"%%R")==NULL)
    {
      const char *MsgItems[]={GetMsg(MWarning),GetMsg(MCannotPutToFolder),
                        GetMsg(MPutToRoot),GetMsg(MOk),GetMsg(MCancel)};
      if (Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)!=0)
        return(-1);
      else
        *CurDir=0;
    }
    char *SwPos=strstr(Command,"%%S");
    if (SwPos!=NULL)
    {
      char CmdRest[512];
      strcpy(CmdRest,SwPos[3]=='}' ? SwPos+4:SwPos+3);
      if (SwPos!=Command && *(SwPos-1)=='{')
        SwPos--;
      strcpy(SwPos,DialogItems[4].Data);
      strcat(Command,CmdRest);
    }
    else
      if (*DialogItems[4].Data)
      {
        strcat(Command," ");
        strcat(Command,DialogItems[4].Data);
      }

    int IgnoreErrors=(CurArcInfo.Flags & AF_IGNOREERRORS);
    ArcCommand ArcCmd(PanelItem,ItemsNumber,Command,DialogItems[2].Data,"",
                      pdd.Password1,AllFilesMask,IgnoreErrors,0,0,CurDir,
                      CurArcInfo.Prefix);
    if (!IgnoreErrors && ArcCmd.GetExecCode()!=0)
      ArcExitCode=0;
    if (ArcCmd.GetExecCode()==RETEXEC_ARCNOTFOUND)
      continue;

    if (GetFullPathName(DialogItems[2].Data,sizeof(FullName),FullName,&NamePtr))
      strcpy(ArcName,FullName);
    break;
  }
  if (Opt.UpdateDescriptions && ArcExitCode)
    for (int I=0;I<ItemsNumber;I++)
      PanelItem[I].Flags|=PPIF_PROCESSDESCR;
  return(ArcExitCode);
}


int __cdecl FormatSort(struct FarMenuItemEx *Item1,struct FarMenuItemEx *Item2)
{
  return FSF.LStricmp(Item1->Text,Item2->Text);
}

int PluginClass::SelectFormat(char *ArcFormat,int AddOnly)
{
  typedef int (__cdecl *FCmp)(const void *, const void *);
  struct FarMenuItemEx *MenuItems=NULL, *NewMenuItems;
  int MenuItemsNumber;
  char Format[100],DefExt[NM];
  int BreakCode;
  int BreakKeys[2]={VK_F4,0};
  int ExitCode;
  int I,J, Add;

  while(1)
  {
    MenuItems=NULL;
    MenuItemsNumber=0;
    for (I=0;I < ArcPlugin->FmtCount();I++)
    {
//      if (!ArcPlugin->GetFormatName(I,0,Format,DefExt))
//        continue;

      for (J=0;;J++)
      {
        if (!ArcPlugin->GetFormatName(I,J,Format,DefExt))
          break;

        Add=TRUE;

        if(AddOnly) // Only add to archive?
        {
          char Buffer[512];
          GetRegKey(HKEY_LOCAL_MACHINE,Format,CmdNames[CMD_ADD],Buffer,"",sizeof(Buffer));
          if(*Buffer == 0)
          {
            ArcPlugin->GetDefaultCommands(I,J,CMD_ADD,Buffer);
            if(*Buffer == 0)
            {
              Add=FALSE;
            }
          }
        }

        if(Add) //
        {
          NewMenuItems=(struct FarMenuItemEx *)realloc(MenuItems,
                               (MenuItemsNumber+1)*sizeof(struct FarMenuItemEx));
          if (NewMenuItems==NULL)
          {
            free(MenuItems);
            return(FALSE);
          }
          MenuItems=NewMenuItems;
          strncpy(MenuItems[MenuItemsNumber].Text,Format,sizeof(MenuItems[MenuItemsNumber].Text));
          MenuItems[MenuItemsNumber].Flags=(MenuItemsNumber==0 &&
                                            *ArcFormat==0 ||
                                            !stricmp(ArcFormat,Format))?
                                            MIF_SELECTED:0;
          MenuItemsNumber++;
        }
      }
    }
    if (MenuItemsNumber==0)
      return(FALSE);

    FSF.qsort(MenuItems,MenuItemsNumber,sizeof(struct FarMenuItemEx),(FCmp)FormatSort);

    ExitCode=Info.Menu(Info.ModuleNumber,-1,-1,0,
                         FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE|FMENU_USEEXT,
                         GetMsg(MSelectArchiver),GetMsg(MSelectF4),NULL,
                         BreakKeys,&BreakCode,
                         (struct FarMenuItem*)MenuItems,MenuItemsNumber);
    if (ExitCode>=0)
    {
      strcpy(ArcFormat,MenuItems[ExitCode].Text);
      if(BreakCode == 0)  // F4 pressed
        ConfigCommands(ArcFormat);
      else
        break;
    }
    else
      break;
    free(MenuItems);
  }
  if(MenuItems)
    free(MenuItems);
  return(ExitCode>=0);
}


int PluginClass::FormatToPlugin(char *Format,int &PluginNumber,int &PluginType)
{
  char PluginFormat[100],DefExt[NM];
  for (int I=0;;I++)
  {
    if (!ArcPlugin->GetFormatName(I,0,PluginFormat,DefExt))
      break;
    for (int J=0;;J++)
    {
      if (!ArcPlugin->GetFormatName(I,J,PluginFormat,DefExt))
        break;
      if (stricmp(PluginFormat,Format)==0)
      {
        PluginNumber=I;
        PluginType=J;
        return(TRUE);
      }
    }
  }
  return(FALSE);
}


int PluginClass::GetPassword(char *Password,const char *FileName)
{
  char Prompt[NM],InPass[80];
  FSF.sprintf(Prompt,GetMsg(MGetPasswordForFile),FileName);
  if(Info.InputBox((const char*)GetMsg(MGetPasswordTitle),
                  (const char*)Prompt,NULL,NULL,
                  InPass,sizeof(InPass),NULL,FIB_PASSWORD|FIB_ENABLEEMPTY))
  {
    strcpy(Password,InPass);
    return(TRUE);
  }
  return(FALSE);
}


int PluginClass::ProcessKey(int Key,unsigned int ControlState)
{
  if ((ControlState & PKF_ALT) && Key==VK_F6)
  {
//    HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
    if (strstr(ArcName,"FarTmp")==NULL)
    {
      char CurDir[NM];
      strcpy(CurDir,ArcName);
      char *Slash=strrchr(CurDir,'\\');
      if (Slash!=NULL)
      {
        if (Slash!=CurDir && *(Slash-1)==':')
          Slash[1]=0;
        else
          *Slash=0;
        SetCurrentDirectory(CurDir);
      }
    }
    struct PanelInfo PInfo;
    Info.Control(this,FCTL_GETPANELINFO,&PInfo);
    GetFiles(PInfo.SelectedItems,PInfo.SelectedItemsNumber,FALSE,PInfo.CurDir,OPM_SILENT);
//    Info.RestoreScreen(hScreen);
    Info.Control(this,FCTL_UPDATEPANEL,NULL);
    Info.Control(this,FCTL_REDRAWPANEL,NULL);
    Info.Control(this,FCTL_UPDATEANOTHERPANEL,NULL);
    Info.Control(this,FCTL_REDRAWANOTHERPANEL,NULL);
    return(TRUE);
  }
  else if (ControlState==(PKF_ALT|PKF_SHIFT) && Key==VK_F9)
  {
    Configure(0);
    return TRUE;
  }
  return(FALSE);
}
