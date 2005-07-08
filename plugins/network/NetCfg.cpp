int Config()
{
  struct InitDialogItem InitItems[]={
  /*  0 */{DI_DOUBLEBOX,3,1,72,10,0,0,0,0,(char *)MConfigTitle},
  /*  1 */{DI_CHECKBOX,5,2,0,0,0,0,0,0,(char *)MConfigAddToDisksMenu},
  /*  2 */{DI_FIXEDIT,7,3,7,3,1,0,0,0,""},
  /*  3 */{DI_TEXT,9,3,0,0,0,0,0,0,(char *)MConfigDisksMenuDigit},
  /*  4 */{DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)MConfigAddToPluginMenu},
  /*  5 */{DI_TEXT,5,5,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /*  6 */{DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MConfigLocalNetwork},
  /*  7 */{DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MNTGetHideShare},
  /*  8 */{DI_TEXT,5,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /*  9 */{DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk},
  /* 10 */{DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel}
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  DialogItems[1].Selected=Opt.AddToDisksMenu;
  DialogItems[4].Selected=Opt.AddToPluginsMenu;
  if (Opt.DisksMenuDigit)
    FSF.sprintf(DialogItems[2].Data,"%d",Opt.DisksMenuDigit);
  DialogItems[6].Selected=Opt.LocalNetwork;
  DialogItems[7].Selected=Opt.NTGetHideShare;

  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,12,"Config",DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (ExitCode!=9)
    return(FALSE);
  Opt.AddToDisksMenu=DialogItems[1].Selected;
  Opt.AddToPluginsMenu=DialogItems[4].Selected;
  Opt.DisksMenuDigit=FSF.atoi(DialogItems[2].Data);
  Opt.LocalNetwork=DialogItems[6].Selected;
  Opt.NTGetHideShare=DialogItems[7].Selected;

  SetRegKey(HKEY_CURRENT_USER,"",StrAddToDisksMenu,Opt.AddToDisksMenu);
  SetRegKey(HKEY_CURRENT_USER,"",StrAddToPluginsMenu,Opt.AddToPluginsMenu);
  SetRegKey(HKEY_CURRENT_USER,"",StrDisksMenuDigit,Opt.DisksMenuDigit);
  SetRegKey(HKEY_CURRENT_USER,"",StrLocalNetwork,Opt.LocalNetwork);
  SetRegKey(HKEY_CURRENT_USER,"",StrNTHiddenShare,Opt.NTGetHideShare);
  return(TRUE);
}
