int Config()
{
  static struct InitDialogItem InitItems[]={
  /* 0*/  DI_DOUBLEBOX,3,1,57,19,0,0,0,0,(char *)MTitle,

  /* 1*/  DI_SINGLEBOX,5,2,55,8,0,0,DIF_LEFTTEXT,0,(char *)MRules,
  /* 2*/  DI_CHECKBOX,7,3,0,0,0,0,0,0,(char*)MIgnoreQuotation,
  /* 3*/  DI_CHECKBOX,7,4,0,0,0,0,0,0,(char*)MIgnoreAfter,
  /* 4*/  DI_CHECKBOX,7,5,0,0,0,0,0,0,(char*)MPriority,
  /* 5*/  DI_CHECKBOX,7,6,0,0,0,0,0,0,(char*)MJumpToPair,
  /* 6*/  DI_CHECKBOX,7,7,0,0,0,0,0,0,(char*)MBeep,

  /* 7*/  DI_SINGLEBOX,5,9,55,16,0,0,DIF_LEFTTEXT,0,(char *)MDescriptions,
  /* 8*/  DI_TEXT,7,10,0,0,0,0,0,0,(char*)MTypeQuotes,
  /* 9*/  DI_FIXEDIT,7,11,26,0,0,0,0,0,"",
  /*10*/  DI_TEXT,7,12,0,0,0,0,0,0,(char*)MDescript1,
  /*11*/  DI_FIXEDIT,7,13,26,0,0,0,0,0,"",
  /*12*/  DI_TEXT,7,14,0,0,0,0,0,0,(char*)MDescript2,
  /*13*/  DI_FIXEDIT,7,15,46,0,0,0,0,0,"",

  /*14*/  DI_TEXT,-1,17,0,0,0,0,DIF_SEPARATOR,0,"",
  /*15*/  DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)MSave,
  /*16*/  DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  DialogItems[2].Param.Selected=Opt.IgnoreQuotes;
  DialogItems[3].Param.Selected=Opt.IgnoreAfter;
  DialogItems[4].Param.Selected=Opt.BracketPrior;
  DialogItems[5].Param.Selected=Opt.JumpToPair;
  DialogItems[6].Param.Selected=Opt.Beep;
  lstrcpy(DialogItems[9].Data.Data,Opt.QuotesType);
  lstrcpy(DialogItems[11].Data.Data,Opt.Brackets1);
  lstrcpy(DialogItems[13].Data.Data,Opt.Brackets2);

  if (Info.Dialog(Info.ModuleNumber,-1,-1,61,21,"Config",
                  DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0])) != 15)
    return(FALSE);

  Opt.IgnoreQuotes=DialogItems[2].Param.Selected;
  Opt.IgnoreAfter=DialogItems[3].Param.Selected;
  Opt.BracketPrior=DialogItems[4].Param.Selected;
  Opt.JumpToPair=DialogItems[5].Param.Selected;
  Opt.Beep=DialogItems[6].Param.Selected;
  lstrcpy(Opt.QuotesType,DialogItems[9].Data.Data);
  lstrcpy(Opt.Brackets1,DialogItems[11].Data.Data);
  lstrcpy(Opt.Brackets2,DialogItems[13].Data.Data);

  HKEY hKey;
  DWORD Disposition;
  if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,
                  KEY_WRITE,NULL,&hKey,&Disposition)) == ERROR_SUCCESS)
  {
    RegSetValueEx(hKey,"Options",0,REG_BINARY,(LPBYTE)&Opt,sizeof(Opt));
    RegCloseKey(hKey);
  }
  return TRUE;
}
