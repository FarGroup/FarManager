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
    return FALSE;
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
      return FALSE;
    if (ItemsNumber>1)
    {
      char Msg[100];
      FSF.sprintf(Msg,GetMsg(MDeleteNumberOfFiles),ItemsNumber);
      MsgItems[1]=Msg;
      if (Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2)!=0)
        return FALSE;
    }
  }
  GetCommandFormat(CMD_DELETE,Command,sizeof(Command));
  GetCommandFormat(CMD_ALLFILESMASK,AllFilesMask,sizeof(AllFilesMask));
  int IgnoreErrors=(CurArcInfo.Flags & AF_IGNOREERRORS);
  ArcCommand ArcCmd(PanelItem,ItemsNumber,Command,ArcName,CurDir,"",AllFilesMask,IgnoreErrors,0,0,CurDir,CurArcInfo.Prefix);
  if (!IgnoreErrors && ArcCmd.GetExecCode()!=0)
    return FALSE;
  if (Opt.UpdateDescriptions)
    for (int I=0;I<ItemsNumber;I++)
      PanelItem[I].Flags|=PPIF_PROCESSDESCR;
  return TRUE;
}


int PluginClass::ProcessHostFile(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
  struct ArcCmdMenuData{ int Msg, Cmd; };
  static const ArcCmdMenuData MenuData[]=
  {
    {MArcCmdTest,         CMD_TEST        },
    {MArcCmdComment,      CMD_COMMENT     },
    {MArcCmdCommentFiles, CMD_COMMENTFILES},
    {MArcCmdSFX,          CMD_SFX         },
    {MArcCmdRecover,      CMD_RECOVER     },
    {MArcCmdProtect,      CMD_PROTECT     },
    {MArcCmdLock,         CMD_LOCK        },
  };

  struct FarMenuItemEx MenuItems[COUNT(MenuData)];
  memset(MenuItems,0,sizeof(MenuItems));
  MenuItems[0].Flags=MIF_SELECTED;

  int Count=0;
  char Command[512],AllFilesMask[32];
  for(int i=0; i<COUNT(MenuData); i++)
  {
    GetCommandFormat(MenuData[i].Cmd, Command, sizeof(Command));
    if(*Command /*&& !(MenuData[i].Cmd==CMD_COMMENTFILES && ItemsNumber!=1)*/)
    {
      strcpy(MenuItems[Count].Text, GetMsg(MenuData[i].Msg));
      MenuItems[Count++].UserData=MenuData[i].Cmd;
    }
  }
  if(!Count)
    return FALSE;
  int ExitCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_USEEXT,
       GetMsg(MArcCmdTitle),NULL,"ArcCmd",NULL,NULL,(FarMenuItem *)MenuItems,
       Count);
  if(ExitCode<0)
    return FALSE;
  FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
  int CommandType=MenuItems[ExitCode].UserData;

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
      return -1;
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
          return FALSE;
        break;
      }

  ArcCommand ArcCmd(PanelItem,ItemsNumber,Command,ArcName,CurDir,Password,AllFilesMask,
                    IgnoreErrors,CommandType==CMD_COMMENT || CommandType==CMD_COMMENTFILES ? 2:0,0,
                    CurDir,CurArcInfo.Prefix);
  return IgnoreErrors || ArcCmd.GetExecCode()==0;
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
    return 0;
  if (*DestPath)
    FSF.AddEndSlash(DestPath);
  const char *PathHistoryName="ExtrDestPath";
  struct InitDialogItem InitItems[]={
  /* 0 */{DI_DOUBLEBOX,3,1,72,12,0,0,0,0,(char *)MExtractTitle},
  /* 1 */{DI_TEXT,5,2,0,0,0,0,0,0,(char *)MExtractTo},
  /* 2 */{DI_EDIT,5,3,70,3,1,(DWORD)PathHistoryName,DIF_HISTORY,0,DestPath},
  /* 3 */{DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /* 4 */{DI_TEXT,5,5,0,0,0,0,0,0,(char *)MExtrPassword},
  /* 5 */{DI_PSWEDIT,5,6,35,5,0,0,0,0,""},
  /* 6 */{DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /* 7 */{DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MExtrWithoutPaths},
  /* 8 */{DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MBackground},
  /* 9 */{DI_TEXT,3,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /*10 */{DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,1,(char *)MExtrExtract},
  /*11 */{DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,0,(char *)MExtrCancel},
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
      return -1;
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
      return -1;
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
          return -1;
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
      return 0;
  if (Move)
    DeleteFiles(PanelItem,ItemsNumber,TRUE);
  if (Opt.UpdateDescriptions)
    for (int I=0;I<ItemsNumber;I++)
      PanelItem[I].Flags|=PPIF_PROCESSDESCR;
  return 1;
}

int __cdecl FormatSort(struct FarMenuItemEx *Item1,struct FarMenuItemEx *Item2)
{
  #ifdef _NEW_ARC_SORT_
  int Temp=(int)Item2->UserData-(int)Item1->UserData;
  return Temp?Temp:(int)Item1->UserData==-1?0:FSF.LStricmp(Item1->Text,Item2->Text);
  #else
  return FSF.LStricmp(Item1->Text,Item2->Text);
  #endif
}

int PluginClass::SelectFormat(char *ArcFormat,int AddOnly)
{
  typedef int (__cdecl *FCmp)(const void *, const void *);
  struct FarMenuItemEx *MenuItems=NULL, *NewMenuItems;
  int MenuItemsNumber=0;
  char Format[100],DefExt[NM];
  int BreakCode;
  int BreakKeys[2]={VK_F4,0};
  int ExitCode;
  int I,J, Add;

  #ifdef _NEW_ARC_SORT_
  int SortModeIndex=GetPrivateProfileInt("MultiArc", "SortMode", 1, IniFile);
  char *SortMode;
  if(SortModeIndex<=1 || SortModeIndex>=COUNT(SortModes))
    SortMode=NULL;
  else
    SortMode=SortModes[SortModeIndex];
  #endif

  while(1)
  {
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
            return FALSE;
          }
          MenuItems=NewMenuItems;
          strncpy(MenuItems[MenuItemsNumber].Text,Format,sizeof(MenuItems[MenuItemsNumber].Text));
          MenuItems[MenuItemsNumber].Flags=(MenuItemsNumber==0 &&
                                            *ArcFormat==0 ||
                                            !stricmp(ArcFormat,Format))?
                                            MIF_SELECTED:0;

          #ifdef _NEW_ARC_SORT_
          if(SortMode)
            MenuItems[MenuItemsNumber].UserData=GetPrivateProfileInt(SortMode, Format, 0, IniFile);
          else
            MenuItems[MenuItemsNumber].UserData=SortModeIndex?0:-1;
          #endif

          MenuItemsNumber++;
        }
      }
    }
    if (MenuItemsNumber==0)
      return FALSE;

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
    MenuItems=NULL;
    MenuItemsNumber=0;
  }
  if(MenuItems)
    free(MenuItems);
  return ExitCode >= 0;
}


int PluginClass::FormatToPlugin(char *Format,int &PluginNumber,int &PluginType)
{
  char PluginFormat[100],DefExt[NM];
  for (int I=0; I < ArcPlugin->FmtCount(); I++)
  {
    /* $ AA 12.11.2001//заремлено, чтобы нормально работали нестандартные модули
    if (!ArcPlugin->GetFormatName(I,0,PluginFormat,DefExt))
      break;
    AA 12.11.2001 $ */
    for (int J=0;;J++)
    {
      if (!ArcPlugin->GetFormatName(I,J,PluginFormat,DefExt))
        break;
      if (stricmp(PluginFormat,Format)==0)
      {
        PluginNumber=I;
        PluginType=J;
        return TRUE;
      }
    }
  }
  return FALSE;
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
    return TRUE;
  }
  return FALSE;
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
    return TRUE;
  }
  else if (ControlState==(PKF_ALT|PKF_SHIFT) && Key==VK_F9)
  {
    Configure(0);
    return TRUE;
  }
  return FALSE;
}
