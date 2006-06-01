/*
TMPCFG.CPP

Temporary panel configuration

*/

enum
{
  AddToDisksMenu,
  AddToPluginsMenu,
  CommonPanel,
  SafeModePanel,
  AnyInPanel,
  CopyContens,
  Mode,
  MenuForFilelist,
  NewPanelForSearchResults,
  FullScreenPanel,
  ColumnTypes,
  ColumnWidths,
  StatusColumnTypes,
  StatusColumnWidths,
  DisksMenuDigit,
  Mask,
  Prefix
};

static const char REGStr[17][8]=
{
 "InDisks","InPlug",
 "Common","Safe","Any","Contens",
 "Mode","Menu","NewP",
 "Full",
 "ColT","ColW","StatT","StatW",
 "DigitV","Mask","Prefix"
};

struct COptionsList
{
  void *Option;
  char *pStr;
  unsigned int DialogItem;
};

static const struct COptionsList OptionsList[]={
  {&Opt.AddToDisksMenu    , ""         ,  1},
  {&Opt.AddToPluginsMenu  , ""         ,  4},

  {&Opt.CommonPanel       , ""         ,  5},
  {&Opt.SafeModePanel     , NULL       ,  6},
  {&Opt.AnyInPanel        , NULL       ,  7},
  {&Opt.CopyContents      , NULL       ,  8},
  {&Opt.Mode              , ""         ,  9},
  {&Opt.MenuForFilelist   , NULL       , 10},
  {&Opt.NewPanelForSearchResults, NULL , 11},

  {&Opt.FullScreenPanel   , NULL       , 22},

  {Opt.ColumnTypes        , "N,S"      , 15},
  {Opt.ColumnWidths       , "0,8"      , 17},
  {Opt.StatusColumnTypes  , "NR,SC,D,T", 19},
  {Opt.StatusColumnWidths , "0,8,0,5"  , 21},

  {Opt.DisksMenuDigit     , "1"        ,  2},
  {Opt.Mask               , "*.temp"   , 25},
  {Opt.Prefix             , "tmp"      , 27},
};

void GetOptions(void)
{
  for(int i=AddToDisksMenu;i<=Prefix;i++)
  {
    DWORD Type,Size,IntValueData;
    static char StrValueData[256];
    HKEY hKey;
    RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey);
    if (i<ColumnTypes)
    {
      Size=sizeof(IntValueData);
      Type=REG_DWORD;
      (*(int*)(OptionsList[i]).Option)=RegQueryValueEx(hKey,
        REGStr[i],0,&Type,(BYTE *)&IntValueData,&Size)==ERROR_SUCCESS?
        IntValueData:(OptionsList[i].pStr?1:0);
    }
    else
    {
      Type=REG_SZ;
      Size=sizeof(StrValueData);
      lstrcpy((char*)OptionsList[i].Option,
        RegQueryValueEx(hKey,REGStr[i],0,&Type,(BYTE*)StrValueData,&Size)==ERROR_SUCCESS?
        (char*)StrValueData:OptionsList[i].pStr);
    }
    RegCloseKey(hKey);
  }

  OutputDebugString("AnyInPanel is");
  OutputDebugString(Opt.AnyInPanel?"true":"false");
}

const int DIALOG_WIDTH = 78;
const int DIALOG_HEIGHT = 22;
const int DC = DIALOG_WIDTH/2-1;

int Config()
{
  static const struct MyInitDialogItem InitItems[]={
  /* 0*/ {DI_DOUBLEBOX, 3, 1,  DIALOG_WIDTH-4,DIALOG_HEIGHT-2, 0, MConfigTitle},

  /* 1*/ {DI_CHECKBOX,  5, 2,  0, 0, 0, MConfigAddToDisksMenu},
  /* 2*/ {DI_FIXEDIT,   7, 3,  7, 3, 0, -1},
  /* 3*/ {DI_TEXT,      9, 3,  0, 0, 0, MConfigDisksMenuDigit},
  /* 4*/ {DI_CHECKBOX, DC, 2,  0, 0, 0, MConfigAddToPluginsMenu},
  /* 5*/ {DI_TEXT,      5, 4,  0, 0, DIF_BOXCOLOR|DIF_SEPARATOR, -1},

  /* 6*/ {DI_CHECKBOX,  5, 5,  0, 0, 0, MConfigCommonPanel},
  /* 7*/ {DI_CHECKBOX,  5, 6,  0, 0, 0, MSafeModePanel},
  /* 8*/ {DI_CHECKBOX,  5, 7,  0, 0, 0, MAnyInPanel},
  /* 9*/ {DI_CHECKBOX,  5, 8,  0, 0, DIF_3STATE, MCopyContens},
  /*10*/ {DI_CHECKBOX, DC, 5,  0, 0, 0, MReplaceInFilelist},
  /*11*/ {DI_CHECKBOX, DC, 6,  0, 0, 0, MMenuForFilelist},
  /*12*/ {DI_CHECKBOX, DC, 7,  0, 0, 0, MNewPanelForSearchResults},

  /*13*/ {DI_TEXT,      5, 9,  0, 0, DIF_BOXCOLOR|DIF_SEPARATOR, -1},

  /*14*/ {DI_TEXT,      5,10,  0, 0, 0, MColumnTypes},
  /*15*/ {DI_EDIT,      5,11, 36,11, 0, -1},
  /*16*/ {DI_TEXT,      5,12,  0, 0, 0, MColumnWidths},
  /*17*/ {DI_EDIT,      5,13, 36,13, 0, -1},
  /*18*/ {DI_TEXT,     DC,10,  0, 0, 0, MStatusColumnTypes},
  /*19*/ {DI_EDIT,     DC,11, 72,11, 0, -1},
  /*20*/ {DI_TEXT,     DC,12,  0, 0, 0, MStatusColumnWidths},
  /*21*/ {DI_EDIT,     DC,13, 72,13, 0, -1},
  /*22*/ {DI_CHECKBOX,  5,14,  0, 0, 0, MFullScreenPanel},

  /*23*/ {DI_TEXT,      5,15,  0, 0, DIF_BOXCOLOR|DIF_SEPARATOR, -1},

  /*24*/ {DI_TEXT,      5,16,  0, 0, 0, MMask},
  /*25*/ {DI_EDIT,      5,17, 36,17, 0, -1},
  /*26*/ {DI_TEXT,     DC,16,  0, 0, 0, MPrefix},
  /*27*/ {DI_EDIT,     DC,17, 72,17, 0, -1},

  /*28*/ {DI_TEXT,      5,18,  0, 0, DIF_BOXCOLOR|DIF_SEPARATOR, -1},

  /*29*/ {DI_BUTTON,    0,19,  0, 0, DIF_CENTERGROUP,  MOk},
  /*30*/ {DI_BUTTON,    0,19,  0, 0, DIF_CENTERGROUP,  MCancel}
  };

  int i;
  static struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];

  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  DialogItems[29].DefaultButton=1;
  DialogItems[2].Focus=1;

  GetOptions();
  for(i=AddToDisksMenu;i<=Prefix;i++)
    if (i<ColumnTypes)
      DialogItems[OptionsList[i].DialogItem].Selected=*(int*)(OptionsList[i].Option);
    else
      lstrcpy(DialogItems[OptionsList[i].DialogItem].Data,(char*)OptionsList[i].Option);

  int Ret=Info.Dialog(Info.ModuleNumber,-1,-1,DIALOG_WIDTH,DIALOG_HEIGHT,"Config",
    DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));

  if(Ret==sizeof(InitItems)/sizeof(InitItems[0]) || Ret<0)
    return(FALSE);

  for(i=AddToDisksMenu;i<=Prefix;i++)
  {
    HKEY hKey;
    DWORD Disposition;
    RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,
                   &hKey,&Disposition);
    if (i<ColumnTypes)
    {
      *((int*)OptionsList[i].Option)=DialogItems[OptionsList[i].DialogItem].Selected;
      RegSetValueEx(hKey,REGStr[i],0,REG_DWORD,(BYTE*)OptionsList[i].Option,
        sizeof(OptionsList[i].Option));
    }
    else
    {
      FSF.Trim(lstrcpy((char*)OptionsList[i].Option,DialogItems[OptionsList[i].DialogItem].Data));
      RegSetValueEx(hKey,REGStr[i],0,REG_SZ,(BYTE*)OptionsList[i].Option,
        lstrlen((char*)OptionsList[i].Option)+1);
    }
    RegCloseKey(hKey);
  }
  if(StartupOptFullScreenPanel!=Opt.FullScreenPanel ||
     StartupOptCommonPanel!=Opt.CommonPanel)
  {
    const char *MsgItems[]={GetMsg(MTempPanel),GetMsg(MConfigNewOption),GetMsg(MOk)};
    Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
  }
  return(TRUE);
}
