struct PutDlgData
{
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

//­®¬¥à  í«¥¬¥­â®¢ ¤¨ «®£  PutFiles
#define PDI_DOUBLEBOX       0
#define PDI_ARCNAMECAPT     1
#define PDI_ARCNAMEEDT      2
#define PDI_SELARCCAPT      3
#define PDI_SELARCCOMB      4
#define PDI_SWITCHESCAPT    5
#define PDI_SWITCHESEDT     6
#define PDI_SEPARATOR0      7
#define PDI_PASS0WCAPT      8
#define PDI_PASS0WEDT       9
#define PDI_PASS1WCAPT      10
#define PDI_PASS1WEDT       11
#define PDI_SEPARATOR1      12
#define PDI_ADDDELCHECK     13
//#define PDI_NOCHNAMECHECK   14
#define PDI_BGROUNDCHECK    14
#define PDI_SEPARATOR2      15
#define PDI_ADDBTN          16
//#define PDI_SELARCBTN       17
#define PDI_SAVEBTN         17
#define PDI_CANCELBTN       18


class SelectFormatComboBox
{
private:
  static int __cdecl Compare(FarListItem *Item1, FarListItem *Item2);
  FarList ListItems;
public:
  SelectFormatComboBox(FarDialogItem *DialogItem, char *ArcFormat);
  ~SelectFormatComboBox(){ free(ListItems.Items); }
};

int SelectFormatComboBox::Compare(FarListItem *Item1, FarListItem *Item2)
{
  return FSF.LStricmp(Item1->Text, Item2->Text);
}

SelectFormatComboBox::SelectFormatComboBox(FarDialogItem *DialogItem, char *ArcFormat)
{
  typedef int (__cdecl *FCmp)(const void *, const void *);
  struct FarListItem *NewItems;
  int &Count=ListItems.ItemsNumber;
  FarListItem *&Items=ListItems.Items;
  char Format[100], DefExt[NM];

  DialogItem->ListItems=NULL;
  Items=NULL;
  Count=0;
  for(int i=0; i<ArcPlugin->FmtCount(); i++)
  {
    for(int j=0; ; j++)
    {
      if(!ArcPlugin->GetFormatName(i, j, Format, DefExt))
        break;

      char Buffer[512];
      GetRegKey(HKEY_LOCAL_MACHINE, Format, CmdNames[CMD_ADD], Buffer, "", sizeof(Buffer));
      if(*Buffer == 0)
      {
        ArcPlugin->GetDefaultCommands(i, j, CMD_ADD, Buffer);
        if(*Buffer == 0)
          continue;
      }

      NewItems=(FarListItem *)realloc(Items, (Count+1)*sizeof(FarListItem));
      if(NewItems==NULL)
      {
        free(Items);
        Items=NULL;
        return;
      }
      Items=NewItems;
      strncpy(Items[Count].Text, Format, sizeof(Items[Count].Text));
      Items[Count].Flags=(Count==0 && *ArcFormat==0 ||
                          !stricmp(ArcFormat, Format))?MIF_SELECTED:0;
      Count++;
    }
  }

  if(Count==0)
  {
    //free(Items);
    //Items=NULL;
    return;
  }

  FSF.qsort(Items, Count, sizeof(struct FarMenuItemEx), (FCmp)Compare);

  DialogItem->ListItems=&ListItems;
}


long WINAPI PluginClass::PutDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  char Buffer[512];
  struct PutDlgData *pdd=(struct PutDlgData*)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);

  if(Msg == DN_INITDIALOG)
  {
    Info.SendDlgMessage(hDlg, DM_SETTEXTLENGTH, PDI_PASS0WEDT, 255);
    Info.SendDlgMessage(hDlg, DM_SETTEXTLENGTH, PDI_PASS1WEDT, 255);
    Info.SendDlgMessage(hDlg,MAM_SETDISABLE,0,0);
    Info.SendDlgMessage(hDlg,MAM_ARCSWITCHES,0,0);

    GetRegKey(HKEY_CURRENT_USER,pdd->ArcFormat,"AddSwitches",Buffer,"",sizeof(Buffer));
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR, PDI_SWITCHESEDT, (long)Buffer);

    FSF.sprintf(Buffer,GetMsg(MAddTitle),pdd->ArcFormat);
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,0,(long)Buffer);

    Info.SendDlgMessage(hDlg,MAM_SETNAME,0,0);
    return TRUE;
  }
  else if(Msg == DN_EDITCHANGE)
  {
    if(Param1 == PDI_ARCNAMEEDT)
    {
      FarDialogItem *Item=(FarDialogItem *)Param2;
      Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_ADDBTN, Item->Data[0] != 0);
    }
    else if(Param1 == PDI_SELARCCOMB)
    {
      Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_SAVEBTN, 1);

      strcpy(pdd->ArcFormat, ((FarDialogItem *)Param2)->Data);
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
      Info.SendDlgMessage(hDlg,DM_SETFOCUS, PDI_ARCNAMEEDT, 0);
      //return TRUE;
    }
    else if(Param1 == PDI_SWITCHESEDT)
    {
      Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_SAVEBTN, 1);
      //return TRUE;
    }

  }
  else if(Msg == DN_BTNCLICK)
  {
    switch(Param1)
    {
      case PDI_CANCELBTN:
        break;

      case PDI_ADDBTN:
      {
        // ¯à®¢¥àª  á®¢¯ ¤¥­¨ï ¢¢¥¤¥­­®£® ¯ à®«ï ¨ ¯®¤â¢¥à¦¤¥­¨ï
        char Password1[256],Password2[256];
        Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, PDI_PASS0WEDT, (long)Password1);
        Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, PDI_PASS1WEDT, (long)Password2);
        if (strcmp(Password1,Password2))
        {
          const char *MsgItems[]={GetMsg(MError),GetMsg(MAddPswNotMatch),GetMsg(MOk)};
          Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,MsgItems,COUNT(MsgItems),1);
          return TRUE;
        }
        break;
      }

      case PDI_SAVEBTN:
      {
        SetRegKey(HKEY_CURRENT_USER,"","DefaultFormat",pdd->ArcFormat);
        Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, PDI_SWITCHESEDT, (long)Buffer);
        SetRegKey(HKEY_CURRENT_USER,pdd->ArcFormat,"AddSwitches",Buffer);

        Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_SAVEBTN, 0);
        Info.SendDlgMessage(hDlg, DM_SETFOCUS, PDI_ARCNAMEEDT, 0);

        return TRUE;
      }
    }
  }
  else if(Msg == DN_CLOSE)
  {
    return Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_ADDBTN, -1) == TRUE ||
           Param1 < 0 ||
           Param1 == PDI_CANCELBTN;
  }
  else if(Msg == MAM_SETDISABLE)
  {
    GetRegKey(HKEY_LOCAL_MACHINE,pdd->ArcFormat,CmdNames[CMD_ADD],Buffer,"",sizeof(Buffer));
    if(*Buffer == 0)
      ArcPlugin->GetDefaultCommands(pdd->Self->ArcPluginNumber,pdd->Self->ArcPluginType,CMD_ADD,Buffer);
    Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_ADDBTN, *Buffer != 0);
  }
  else if(Msg == MAM_ARCSWITCHES)
  {
    static char SwHistoryName[NM];
    FSF.sprintf(SwHistoryName,"ArcSwitches\\%s",pdd->ArcFormat);
    Info.SendDlgMessage(hDlg, DM_SETHISTORY, PDI_SWITCHESEDT, (long)SwHistoryName);
    Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, PDI_SWITCHESEDT, (long)"");
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
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR, PDI_ARCNAMEEDT, (long)Buffer);
  }

  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

//²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²
//²                                                                            ²
//²   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Add to ZIP ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»   °
//²   º Add to archive                                                     º   °
//²   º backup°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°º   °
//²   º Select archiver        Switches                                    º   °
//²   º ZIP°°°°°°°°°°°°°°°°°° °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°º   °
//²   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   °
//²   º Archive password                  Reenter password                 º   °
//²   º °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°  °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° º   °
//²   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   °
//²   º [ ] Delete files after archiving                                   º   °
//²   º [ ] Background                                                     º   °
//²   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶   °
//²   º               [ Add ]  [ Save settings ]  [ Cancel ]               º   °
//²   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼   °
//²                                                                            °
//²²²°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
//²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²

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
     á¡à®á £ «ª¨ "ä®­®¢ ï  àå¨¢ æ¨ï" */
  /* $ 13.04.2001 DJ
     ¯¥à¥­¥á¥­ ¢ ¡®«¥¥ ¯®¤å®¤ïé¥¥ ¬¥áâ® */
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
      /* 0*/{DI_DOUBLEBOX,3,1,72,14,0,0,0,0,""},
      /* 1*/{DI_TEXT,5,2,0,0,0,0,0,0,(char *)MAddToArc},
      /* 2*/{DI_EDIT,5,3,70,3,1,(DWORD)ArcHistoryName,DIF_HISTORY,0,""},
      /* 3*/{DI_TEXT,5,4,0,0,0,0,0,0,(char *)MAddSelect},
      /* 4*/{DI_COMBOBOX,5,5,25,3,0,0,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND,0,""},
      /* 5*/{DI_TEXT,28,4,0,0,0,0,0,0,(char *)MAddSwitches},
      /* 6*/{DI_EDIT,28,5,70,3,0,0,DIF_HISTORY,0,""},
      /* 7*/{DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
      /* 8*/{DI_TEXT,5,7,0,0,0,0,0,0,(char *)MAddPassword},
      /* 9*/{DI_PSWEDIT,5,8,36,8,0,0,0,0,""},
      /*10*/{DI_TEXT,39,7,0,0,0,0,0,0,(char *)MAddReenterPassword},
      /*11*/{DI_PSWEDIT,39,8,70,8,0,0,0,0,""},
      /*12*/{DI_TEXT,3,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
      /*13*/{DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MAddDelete},
      //    {DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MNoChangeName},
      /*14*/{DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MBackground},
      /*15*/{DI_TEXT,3,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
      /*16*/{DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,1,(char *)MAddAdd},
      /*17*/{DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP|DIF_DISABLE,0,(char *)MAddSave},
      /*18*/{DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(char *)MAddCancel},
    };
    struct FarDialogItem DialogItems[COUNT(InitItems)];
    InitDialogItems(InitItems,DialogItems,COUNT(InitItems));

    SelectFormatComboBox Box(&DialogItems[PDI_SELARCCOMB], pdd.ArcFormat);

    if(Opt.UseLastHistory)
      DialogItems[PDI_SWITCHESEDT].Flags|=DIF_USELASTHISTORY;

    if (*ArcName)
      strcpy(DialogItems[PDI_ARCNAMEEDT].Data,ArcName);
    else
    {
      if (ItemsNumber==1)
      {
        strcpy(DialogItems[PDI_ARCNAMEEDT].Data,PanelItem->FindData.cFileName);
        if(Opt.DeleteExtFile)
        {
          char *Dot=strrchr(DialogItems[PDI_ARCNAMEEDT].Data,'.');
          if (Dot!=NULL)
            *Dot=0;
        }
      }
      else
      {
        char CurDir[NM];
        GetCurrentDirectory(sizeof(CurDir),CurDir);
        strcpy(DialogItems[PDI_ARCNAMEEDT].Data,FSF.PointToName(CurDir));
      }
      char AnsiName[NM];
      OemToAnsi(DialogItems[PDI_ARCNAMEEDT].Data,AnsiName);
      if (!IsCaseMixed(AnsiName))
      {
        CharLower(AnsiName);
        AnsiToOem(AnsiName,DialogItems[PDI_ARCNAMEEDT].Data);
      }
    }
    DialogItems[PDI_ADDDELCHECK].Selected=Move;
    /* $ 13.04.2001 DJ
       UserBackground instead of Background
    */
    DialogItems[PDI_BGROUNDCHECK].Selected=Opt.UserBackground;
    /* DJ $ */
    strcpy(pdd.OriginalName,DialogItems[PDI_ARCNAMEEDT].Data);

//    if(!Opt.DeleteExtFile)

    {
      if ((ExtPtr=strrchr(DialogItems[PDI_ARCNAMEEDT].Data,'.'))==NULL ||
          (ExtPtr && stricmp(++ExtPtr,pdd.DefExt) != 0))
      {
        if(Opt.AddExtArchive)
        {
          FSF.sprintf(ExtName,"%s.%s",DialogItems[PDI_ARCNAMEEDT].Data,pdd.DefExt);
          strcpy(DialogItems[PDI_ARCNAMEEDT].Data,ExtName);
        }
      }
    }

    if ((OpMode & OPM_SILENT)==0)
    {
      int AskCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,16,"AddToArc",
                  DialogItems,COUNT(DialogItems),
                  0,0,PluginClass::PutDlgProc,(long)&pdd);

      strcpy(pdd.Password1,DialogItems[PDI_PASS0WEDT].Data);
      strcpy(pdd.Password2,DialogItems[PDI_PASS1WEDT].Data);
      Opt.UserBackground=DialogItems[PDI_BGROUNDCHECK].Selected;
      if (AskCode!=PDI_ADDBTN || *DialogItems[PDI_ARCNAMEEDT].Data==0)
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
    if (DialogItems[PDI_ADDDELCHECK].Selected)
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
      if (Info.Message(Info.ModuleNumber,0,NULL,MsgItems,COUNT(MsgItems),2)!=0)
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
      strcpy(SwPos,DialogItems[PDI_SWITCHESEDT].Data);
      strcat(Command,CmdRest);
    }
    else
      if (*DialogItems[PDI_SWITCHESEDT].Data)
      {
        strcat(Command," ");
        strcat(Command,DialogItems[PDI_SWITCHESEDT].Data);
      }

    int IgnoreErrors=(CurArcInfo.Flags & AF_IGNOREERRORS);
    ArcCommand ArcCmd(PanelItem,ItemsNumber,Command,
                      DialogItems[PDI_ARCNAMEEDT].Data,"",pdd.Password1,
                      AllFilesMask,IgnoreErrors,0,0,CurDir,CurArcInfo.Prefix);
    if (!IgnoreErrors && ArcCmd.GetExecCode()!=0)
      ArcExitCode=0;
    if (ArcCmd.GetExecCode()==RETEXEC_ARCNOTFOUND)
      continue;

    if (GetFullPathName(DialogItems[PDI_ARCNAMEEDT].Data,sizeof(FullName),FullName,&NamePtr))
      strcpy(ArcName,FullName);
    break;
  }
  if (Opt.UpdateDescriptions && ArcExitCode)
    for (int I=0;I<ItemsNumber;I++)
      PanelItem[I].Flags|=PPIF_PROCESSDESCR;
  return(ArcExitCode);
}
