//#define MAX_PASSW_LEN 256

struct PutDlgData
{
  PluginClass *Self;
  char ArcFormat[NM];
  //char OriginalName[512];   //$ AA 26.11.2001
  char Password1[256];
  //char Password2[256];      //$ AA 28.11.2001
  char DefExt[NM];
  BOOL DefaultPluginNotFound; //$ AA 2?.11.2001
  BOOL NoChangeArcName;       //$ AA 23.11.2001
  BOOL OldExactState;         //$ AA 26.11.2001
  //BOOL ArcNameChanged;        //$ AA 27.11.2001
};

#define MAM_SETDISABLE   DM_USER+1
#define MAM_ARCSWITCHES  DM_USER+2
//#define MAM_SETNAME      DM_USER+3
#define MAM_SELARC       DM_USER+4
#define MAM_ADDDEFEXT    DM_USER+5
#define MAM_DELDEFEXT    DM_USER+6

//­®¬¥à  í«¥¬¥­â®¢ ¤¨ «®£  PutFiles
#define PDI_DOUBLEBOX       0
#define PDI_ARCNAMECAPT     1
#define PDI_ARCNAMEEDT      2

//#define PDI_SELARCCAPT      3
//#define PDI_SELARCCOMB      4
//#define PDI_SWITCHESCAPT    5
//#define PDI_SWITCHESEDT     6
#define PDI_SWITCHESCAPT    3
#define PDI_SWITCHESEDT     4
#define PDI_SELARCCAPT      5
#define PDI_SELARCCOMB      6

#define PDI_SEPARATOR0      7
#define PDI_PASS0WCAPT      8
#define PDI_PASS0WEDT       9
#define PDI_PASS1WCAPT      10
#define PDI_PASS1WEDT       11
#define PDI_SEPARATOR1      12
#define PDI_ADDDELCHECK     13
#define PDI_EXACTNAMECHECK  14
#define PDI_BGROUNDCHECK    15
#define PDI_SEPARATOR2      16
#define PDI_ADDBTN          17
#define PDI_SELARCBTN       18
#define PDI_SAVEBTN         19
#define PDI_CANCELBTN       20

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

      //*Buffer=0; //$ AA á¡à®á¨âáï ¢ GetDefaultCommands
      ArcPlugin->GetDefaultCommands(i, j, CMD_ADD, Buffer);
      //å¨âàë© ä¨­â - ¯®¤áâ ­®¢ª  Buffer ¢ ª ç¥áâ¢¥ ¤¥ä®«â  ¤«ï á ¬®£® Buffer
      GetRegKey(HKEY_LOCAL_MACHINE, Format, CmdNames[CMD_ADD], Buffer, Buffer, sizeof(Buffer));
      if(*Buffer == 0)
        continue;

      NewItems=(FarListItem *)realloc(Items, (Count+1)*sizeof(FarListItem));
      if(NewItems==NULL)
      {
        free(Items);
        Items=NULL;
        return;
      }
      Items=NewItems;
      strncpy(Items[Count].Text, Format, sizeof(Items[Count].Text)-1);
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

  FSF.qsort(Items, Count, sizeof(struct FarListItem), (FCmp)Compare);

  DialogItem->ListItems=&ListItems;
}


long WINAPI PluginClass::PutDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  char Buffer[512];
  PutDlgData *pdd=(struct PutDlgData*)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);

  if(Msg == DN_INITDIALOG)
  {
    if(pdd->DefaultPluginNotFound)
      Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_SAVEBTN, 1);

    Info.SendDlgMessage(hDlg, DM_SETTEXTLENGTH, PDI_PASS0WEDT, 255);
    Info.SendDlgMessage(hDlg, DM_SETTEXTLENGTH, PDI_PASS1WEDT, 255);
    Info.SendDlgMessage(hDlg,MAM_SETDISABLE,0,0);
    Info.SendDlgMessage(hDlg,MAM_ARCSWITCHES,0,0);

    GetRegKey(HKEY_CURRENT_USER,pdd->ArcFormat,"AddSwitches",Buffer,"",sizeof(Buffer));
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR, PDI_SWITCHESEDT, (long)Buffer);

    FSF.sprintf(Buffer,GetMsg(MAddTitle),pdd->ArcFormat);
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,0,(long)Buffer);

    //Info.SendDlgMessage(hDlg,MAM_SETNAME,0,0);

    if(OLD_DIALOG_STYLE)
    {
      Info.SendDlgMessage(hDlg, DM_SHOWITEM, PDI_SELARCCOMB, 0);
      Info.SendDlgMessage(hDlg, DM_SHOWITEM, PDI_SELARCCAPT, 0);

      FarDialogItem Item;
      Info.SendDlgMessage(hDlg, DM_GETDLGITEM, PDI_SWITCHESCAPT, (DWORD)&Item);
      Item.X1=5;
      Info.SendDlgMessage(hDlg, DM_SETDLGITEM, PDI_SWITCHESCAPT, (DWORD)&Item);
      Info.SendDlgMessage(hDlg, DM_GETDLGITEM, PDI_SWITCHESEDT, (DWORD)&Item);
      Item.X1=5;
      Item.X2=70;
      Info.SendDlgMessage(hDlg, DM_SETDLGITEM, PDI_SWITCHESEDT, (DWORD)&Item);

      Info.SendDlgMessage(hDlg, DM_SHOWITEM, PDI_SELARCBTN, 1);
    }
    else
      Info.SendDlgMessage(hDlg, DM_SHOWITEM, PDI_SELARCBTN, 0);

    Info.SendDlgMessage(hDlg, DM_SETCHECK, PDI_EXACTNAMECHECK, pdd->OldExactState?BSTATE_CHECKED:BSTATE_UNCHECKED);
    //pdd->ArcNameChanged=0;
    if(pdd->NoChangeArcName)
      Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_EXACTNAMECHECK, 0);

    return TRUE;
  }
  else if(Msg == DN_EDITCHANGE)
  {
    if(Param1 == PDI_ARCNAMEEDT)
    {
      FarDialogItem *Item=(FarDialogItem *)Param2;
      Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_ADDBTN, Item->Data[0] != 0);
      //pdd->ArcNameChanged=TRUE;
      Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_EXACTNAMECHECK, 1);
    }
    else if(Param1 == PDI_SELARCCOMB)
    {
      strcpy(pdd->ArcFormat, ((FarDialogItem *)Param2)->Data);
      Info.SendDlgMessage(hDlg, MAM_SELARC, 0, 0);
    }
    else if(Param1 == PDI_SWITCHESEDT)
    {
      Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_SAVEBTN, 1);
      //return TRUE;
    }

  }
  else if(Msg == DN_KEY && Param2 == KEY_SHIFTF1) // Select archiver
  {
    if(OLD_DIALOG_STYLE)
    {
      Info.SendDlgMessage(hDlg, DN_BTNCLICK, PDI_SELARCBTN, 0);
    }
    else
    {
      ; // §¤¥áì ª®¤ ¤«ï à áªàëâ¨ï ª®¬¡®¡®ªá  ¢ë¡®à®à   àå¨¢ â®à 
    }
    return TRUE; // ­¥ ®¡à ¡ âë¢ âì íâã ª« ¢¨èã
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
      case PDI_SELARCBTN:
        if(pdd->Self->SelectFormat(pdd->ArcFormat,TRUE))
        {
          Info.SendDlgMessage(hDlg, MAM_SELARC, 0, 0);
#ifdef _NEW_ARC_SORT_
          int Rate=GetPrivateProfileInt("ChoiceRate", pdd->ArcFormat, 0, IniFile);
          WritePrivateProfileInt("ChoiceRate", pdd->ArcFormat, Rate+1, IniFile);
#endif //_NEW_ARC_SORT_
        }
        Info.SendDlgMessage(hDlg, DM_SETFOCUS, PDI_ARCNAMEEDT, 0);
        return TRUE;
      case PDI_EXACTNAMECHECK:
        if(Param2)
        {
          BOOL UnChanged=Info.SendDlgMessage(hDlg, DM_EDITUNCHANGEDFLAG, PDI_ARCNAMEEDT, -1);
          if(!pdd->OldExactState && /*!pdd->ArcNameChanged*/UnChanged) // 0->1
            Info.SendDlgMessage(hDlg, MAM_ADDDEFEXT, 0, 0);
        }
        else
          if(pdd->OldExactState)  // 1->0
            Info.SendDlgMessage(hDlg, MAM_DELDEFEXT, 0, 0);
        pdd->OldExactState=Param2;
        return TRUE;
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
    ArcPlugin->GetDefaultCommands(pdd->Self->ArcPluginNumber,pdd->Self->ArcPluginType,CMD_ADD,Buffer);
    GetRegKey(HKEY_LOCAL_MACHINE,pdd->ArcFormat,CmdNames[CMD_ADD],Buffer,Buffer,sizeof(Buffer));
    Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_ADDBTN, *Buffer != 0);
  }
  else if(Msg == MAM_ARCSWITCHES)
  {
    // ‚ëáâ ¢«ï¥¬ ¤ ­­ë¥ ¨§ AddSwitches
    GetRegKey(HKEY_CURRENT_USER,pdd->ArcFormat,"AddSwitches",Buffer,"",sizeof(Buffer));
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR, PDI_SWITCHESEDT, (long)Buffer);
    // ¥á«¨ AddSwitches ¯ãáâ®© ¨ î§ ¥âáï UseLastHistory, â®...
    static char SwHistoryName[NM];
    FSF.sprintf(SwHistoryName,"ArcSwitches\\%s",pdd->ArcFormat);
    // ...á«¥¤ãîé ï ª®¬ ­¤  § áâ ¢¨â ¢ëáâ ¢¨âì LastHistory
    Info.SendDlgMessage(hDlg, DM_SETHISTORY, PDI_SWITCHESEDT, (long)SwHistoryName);
    Info.SendDlgMessage(hDlg, DM_EDITUNCHANGEDFLAG, PDI_SWITCHESEDT, 1);
    return TRUE;

  }
  else if(Msg == MAM_SELARC)
  {
    Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_SAVEBTN, 1);
    Info.SendDlgMessage(hDlg, DM_ENABLE, PDI_EXACTNAMECHECK, 1);

    pdd->Self->FormatToPlugin(pdd->ArcFormat,pdd->Self->ArcPluginNumber,pdd->Self->ArcPluginType);

    BOOL IsDelOldDefExt=Info.SendDlgMessage(hDlg, MAM_DELDEFEXT, 0, 0);
    IsDelOldDefExt=IsDelOldDefExt && Info.SendDlgMessage(hDlg, DM_GETCHECK, PDI_EXACTNAMECHECK, 0);

    GetRegKey(HKEY_LOCAL_MACHINE,pdd->ArcFormat,"DefExt",Buffer,"",sizeof(Buffer));
    BOOL Ret=TRUE;
    if(*Buffer==0)
      Ret=ArcPlugin->GetFormatName(pdd->Self->ArcPluginNumber,pdd->Self->ArcPluginType,pdd->ArcFormat,Buffer);
    if(!Ret)
      pdd->DefExt[0]=0;
    else
      strncpy(pdd->DefExt,Buffer,sizeof(pdd->DefExt)-1);

    if(IsDelOldDefExt)
      Info.SendDlgMessage(hDlg, MAM_ADDDEFEXT, 0, 0);

    FSF.sprintf(Buffer,GetMsg(MAddTitle),pdd->ArcFormat);
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,0,(long)Buffer);

    Info.SendDlgMessage(hDlg,MAM_SETDISABLE,0,0);
    Info.SendDlgMessage(hDlg,MAM_ARCSWITCHES,0,0);
    //Info.SendDlgMessage(hDlg,MAM_SETNAME,0,0);
    Info.SendDlgMessage(hDlg,DM_SETFOCUS, PDI_ARCNAMEEDT, 0);
    return TRUE;
  }
  else if(Msg == MAM_ADDDEFEXT)
  {
    char Name[NM]/*, *Ext*/;
    Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, PDI_ARCNAMEEDT, (DWORD)Name);
    AddExt(Name, pdd->DefExt);
    Info.SendDlgMessage(hDlg,DM_SETTEXTPTR, PDI_ARCNAMEEDT, (DWORD)Name);
    return TRUE;
  }
  else if(Msg == MAM_DELDEFEXT)
  {
    char Name[NM]/*, *DefExt*/, *Ext;
    Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, PDI_ARCNAMEEDT, (DWORD)Name);
    if( SeekDefExtPoint(Name, pdd->DefExt, &Ext)!=NULL
        || (Ext!=NULL && !*(Ext+1)) )
    {
      *Ext=0;
      Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, PDI_ARCNAMEEDT, (DWORD)Name);
      return TRUE;
    }
    return FALSE;
  }

  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

//­®¢ë© áâ¨«ì ¤¨ «®£  "„®¡ ¢¨âì ª  àå¨¢ã"
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
    return 0;

  char Command[512],AllFilesMask[32];
  int ArcExitCode=1;
  struct PutDlgData pdd={0};
  char FullName[NM],/*ExtName[NM],*/*NamePtr/*,*ExtPtr*/;
  BOOL Ret=TRUE;

  pdd.Self=this;
  *pdd.Password1/*=*pdd.Password2*/=0;

  if (ArcPluginNumber==-1)
  {
    char DefaultFormat[100];
    GetRegKey(HKEY_CURRENT_USER,"","DefaultFormat",DefaultFormat,"",sizeof(DefaultFormat));
    if (!FormatToPlugin(DefaultFormat,ArcPluginNumber,ArcPluginType))
    {
      ArcPluginNumber=ArcPluginType=0;
      pdd.DefaultPluginNotFound=TRUE;
    }
    else
    {
      strcpy(pdd.ArcFormat,DefaultFormat);
      pdd.DefaultPluginNotFound=FALSE;
    }
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
      return 0;

    const char *ArcHistoryName="ArcName";

    struct InitDialogItem InitItems[]={
      /* 0*/{DI_DOUBLEBOX,3,1,72,15,0,0,0,0,""},
      /* 1*/{DI_TEXT,5,2,0,0,0,0,0,0,(char *)MAddToArc},
      /* 2*/{DI_EDIT,5,3,70,3,1,(DWORD)ArcHistoryName,DIF_HISTORY,0,""},

      //* 3*/{DI_TEXT,5,4,0,0,0,0,0,0,(char *)MAddSelect},
      //* 4*/{DI_COMBOBOX,5,5,25,3,0,0,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND,0,""},
      //* 5*/{DI_TEXT,28,4,0,0,0,0,0,0,(char *)MAddSwitches},
      //* 6*/{DI_EDIT,28,5,70,3,0,0,DIF_HISTORY,0,""},
      /* 3*/{DI_TEXT,5,4,0,0,0,0,0,0,(char *)MAddSwitches},
      /* 4*/{DI_EDIT,5,5,47,3,0,0,DIF_HISTORY,0,""},
      /* 5*/{DI_TEXT,50,4,0,0,0,0,0,0,(char *)MAddSelect},
      /* 6*/{DI_COMBOBOX,50,5,70,3,0,0,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTNOAMPERSAND,0,""},

      /* 7*/{DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
      /* 8*/{DI_TEXT,5,7,0,0,0,0,0,0,(char *)MAddPassword},
      /* 9*/{DI_PSWEDIT,5,8,36,8,0,0,0,0,""},
      /*10*/{DI_TEXT,39,7,0,0,0,0,0,0,(char *)MAddReenterPassword},
      /*11*/{DI_PSWEDIT,39,8,70,8,0,0,0,0,""},
      /*12*/{DI_TEXT,3,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
      /*13*/{DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MAddDelete},
      /*14*/{DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MExactArcName},
      /*15*/{DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)MBackground},
      /*16*/{DI_TEXT,3,13,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
      /*17*/{DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,1,(char *)MAddAdd},
      /*18*/{DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,0,(char *)MAddSelect},
      /*19*/{DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP|DIF_DISABLE,0,(char *)MAddSave},
      /*20*/{DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,0,(char *)MAddCancel},
    };
    struct FarDialogItem DialogItems[COUNT(InitItems)];
    InitDialogItems(InitItems,DialogItems,COUNT(InitItems));

/*    if(OLD_DIALOG_STYLE)
    {
      DialogItems[PDI_SWITCHESCAPT].X1=5;
      DialogItems[PDI_SWITCHESEDT].X1=5;
    }*/

    SelectFormatComboBox Box(&DialogItems[PDI_SELARCCOMB], pdd.ArcFormat);

    if(Opt.UseLastHistory)
      DialogItems[PDI_SWITCHESEDT].Flags|=DIF_USELASTHISTORY;

    if(*ArcName)
    {
      pdd.NoChangeArcName=TRUE;
      pdd.OldExactState=TRUE;
      strcpy(DialogItems[PDI_ARCNAMEEDT].Data, ArcName);
    }
    else
    {
#ifdef _ARC_UNDER_CURSOR_
      if(GetCursorName(DialogItems[PDI_ARCNAMEEDT].Data, pdd.ArcFormat, pdd.DefExt))
      {
        //pdd.NoChangeArcName=TRUE;
        pdd.OldExactState=TRUE;
      }
      else
      {
#endif //_ARC_UNDER_CURSOR_
        pdd.OldExactState=Opt.AutoResetExactArcName?FALSE:Opt.ExactArcName;
#ifdef _GROOP_NAME_
        GetGroopName(PanelItem, ItemsNumber, DialogItems[PDI_ARCNAMEEDT].Data);
#else //_GROOP_NAME_
        if(ItemsNumber==1)
        {
          strcpy(DialogItems[PDI_ARCNAMEEDT].Data.Data, PanelItem->FindData.cFileName);
          char *Dot=strrchr(DialogItems[PDI_ARCNAMEEDT].Data.Data,'.');
          if(Dot!=NULL)
            *Dot=0;
        }
        else
        {
          char CurDir[NM];
          GetCurrentDirectory(sizeof(CurDir),CurDir);
          strcpy(DialogItems[PDI_ARCNAMEEDT].Data.Data, FSF.PointToName(CurDir));
        }
#endif //else _GROOP_NAME_
        if(pdd.OldExactState && !*ArcName)
          AddExt(DialogItems[PDI_ARCNAMEEDT].Data, pdd.DefExt);
#ifdef _ARC_UNDER_CURSOR_
      }
#endif //_ARC_UNDER_CURSOR_
/*    $ AA 29.11.2001 //­ ä¨£  ­ ¬ ¨¬ï ãáà¥¤­ïâì?
      char AnsiName[NM];
      OemToAnsi(DialogItems[PDI_ARCNAMEEDT].Data,AnsiName);
      if(!IsCaseMixed(AnsiName))
      {
        CharLower(AnsiName);
        AnsiToOem(AnsiName, DialogItems[PDI_ARCNAMEEDT].Data);
      }
      AA 29.11.2001 $ */
    }

    DialogItems[PDI_ADDDELCHECK].Selected=Move;
    /* $ 13.04.2001 DJ
       UserBackground instead of Background
    */
    DialogItems[PDI_BGROUNDCHECK].Selected=Opt.UserBackground;
    /* DJ $ */
    //strcpy(pdd.OriginalName,DialogItems[PDI_ARCNAMEEDT].Data);


    if ((OpMode & OPM_SILENT)==0)
    {
      int AskCode=Info.DialogEx(Info.ModuleNumber,-1,-1,76,17,"AddToArc",
                  DialogItems,COUNT(DialogItems),
                  0,0,PluginClass::PutDlgProc,(long)&pdd);

      strcpy(pdd.Password1,DialogItems[PDI_PASS0WEDT].Data);
      //strcpy(pdd.Password2,DialogItems[PDI_PASS1WEDT].Data); //$ AA 28.11.2001
      Opt.UserBackground=DialogItems[PDI_BGROUNDCHECK].Selected;
      if (AskCode!=PDI_ADDBTN || *DialogItems[PDI_ARCNAMEEDT].Data==0)
        return -1;
      //SetRegKey(HKEY_CURRENT_USER,"","Background",Opt.UserBackground); // $ 06.02.2002 AA
      Opt.ExactArcName=DialogItems[PDI_EXACTNAMECHECK].Selected;
      SetRegKey(HKEY_CURRENT_USER, "", "ExactArcName", Opt.ExactArcName);
    }

    char *Ext;
    SeekDefExtPoint(DialogItems[PDI_ARCNAMEEDT].Data, pdd.DefExt, &Ext);
    if(DialogItems[PDI_EXACTNAMECHECK].Selected)
    {
      if(Ext==NULL)
        strcat(DialogItems[PDI_ARCNAMEEDT].Data, ".");
    }
    else
      AddExt(DialogItems[PDI_ARCNAMEEDT].Data, pdd.DefExt);

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

    ArcPlugin->GetDefaultCommands(ArcPluginNumber,ArcPluginType,CommandType,Command);
    GetRegKey(HKEY_LOCAL_MACHINE,pdd.ArcFormat,CmdNames[CommandType],Command,Command,sizeof(Command));
    ArcPlugin->GetDefaultCommands(ArcPluginNumber,ArcPluginType,CMD_ALLFILESMASK,AllFilesMask);
    GetRegKey(HKEY_LOCAL_MACHINE,pdd.ArcFormat,"AllFilesMask",AllFilesMask,AllFilesMask,sizeof(AllFilesMask));
    if (*CurDir && strstr(Command,"%%R")==NULL)
    {
      const char *MsgItems[]={GetMsg(MWarning),GetMsg(MCannotPutToFolder),
                        GetMsg(MPutToRoot),GetMsg(MOk),GetMsg(MCancel)};
      if (Info.Message(Info.ModuleNumber,0,NULL,MsgItems,COUNT(MsgItems),2)!=0)
        return -1;
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

    //¯®á«¥¤ãîé¨¥ ®¯¥à æ¨¨ (â¥áâ¨à®¢ ­¨¥ ¨ â¤) ­¥ ¤®«¦­ë ¡ëâì ä®­®¢ë¬¨
    Opt.Background=0; // $ 06.02.2002 AA

#ifdef _NEW_ARC_SORT_
    int Rate=GetPrivateProfileInt("RunRate", pdd.ArcFormat, 0, IniFile);
    WritePrivateProfileInt("RunRate", pdd.ArcFormat, Rate+1, IniFile);
#endif //_NEW_ARC_SORT_

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
  return ArcExitCode;
}

#ifdef _ARC_UNDER_CURSOR_
BOOL PluginClass::GetCursorName(char *ArcName, char *ArcFormat, char *ArcExt)
{
  if(!GetRegKey(HKEY_CURRENT_USER,"","ArcUnderCursor",0))
    return FALSE;

  PanelInfo pi;
  Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &pi);
  PluginPanelItem *Items=pi.PanelItems;
  PluginPanelItem *SelItems=pi.SelectedItems;
  PluginPanelItem *CurItem=Items+pi.CurrentItem;

  //¯®¤ ªãàá®à®¬ ¤®«¦­  ¡ëâì ­¥ ¯ ¯ª 
  if(CurItem->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
    return FALSE;

  //¤®«¦­® ¡ëâì ­¥¯ãáâ®¥ à áè¨à¥­¨¥
  char *Dot=strrchr(CurItem->FindData.cFileName, '.');
  if(!Dot || !*(++Dot))
    return FALSE;

  int i,j;

  //ªãàá®à ¤®«¦¥­ ¡ëâì ¢­¥ ¢ë¤¥«¥­¨ï
  for(i=0; i<pi.SelectedItemsNumber; i++)
    if(!strcmpi(CurItem->FindData.cFileName, SelItems[i].FindData.cFileName))
      return FALSE;

  //¯®¤ ªãàá®à®¬ ¤®«¦¥­ ¡ëâì ä ©« á à áè¨à¥­¨¥¬  àå¨¢ 
  char Format[100],DefExt[NM];
  for(i=0; i<ArcPlugin->FmtCount(); i++)
    for(j=0; ; j++)
    {
      if(!ArcPlugin->GetFormatName(i, j, Format, DefExt))
        break;
      //å¨âàë© å¨­â, çâ¥­¨¥ ª«îç  á ¤¥ä®«â®¬ ¨§ DefExt
      GetRegKey(HKEY_LOCAL_MACHINE, Format, "DefExt", DefExt, DefExt, sizeof(DefExt));

      if(!strcmpi(Dot, DefExt))
      {
        strcpy(ArcName, CurItem->FindData.cFileName);
        //int Len=Dot-CurItem->FindData.cFileName-1;
        //strncpy(ArcName, CurItem->FindData.cFileName, Len);
        //ArcName[Len]=0;

        //¢ë¡à âì á®®â¢¥âáâ¢ãîé¨©  àå¨¢ â®à
        ArcPluginNumber=i;
        ArcPluginType=j;
        strcpy(ArcFormat, Format);
        strcpy(ArcExt, DefExt);
        return TRUE;
      }
    }
  return FALSE;
}
#endif //_ARC_UNDER_CURSOR_

#ifdef _GROOP_NAME_
void PluginClass::GetGroopName(PluginPanelItem *Items, int Count, char *ArcName)
{
  BOOL NoGroop=!GetRegKey(HKEY_CURRENT_USER,"","GroopName",0);

  char *Name=Items->FindData.cFileName;
  char *Dot=strrchr(Name, '.');
  int Len=(Dot && !(Items->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
          ?(Dot-Name):strlen(Name);
  for(int i=1; i<Count; i++)
    if(NoGroop || FSF.LStrnicmp(Name, Items[i].FindData.cFileName, Len))
//    if(strncmpi(Name, Items[i].FindData.cFileName, Len))
    {
      //¢§ïâì ¨¬ï ¯ ¯ª¨
      char CurDir[NM];
      GetCurrentDirectory(sizeof(CurDir), CurDir);
      strcpy(ArcName, FSF.PointToName(CurDir));
      return;
    }
  strncpy(ArcName, Name, Len);
  ArcName[Len]=0;
}
#endif //_GROOP_NAME_
