int Config()
{
  struct InitDialogItem InitItems[]={
    DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MConfigTitle,
    DI_CHECKBOX,5,2,0,0,0,0,0,0,(char *)MConfigAddToDisksMenu,
    DI_FIXEDIT,7,3,7,3,1,0,0,0,"",
    DI_TEXT,9,3,0,0,0,0,0,0,(char *)MConfigDisksMenuDigit,
    DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MConfigCommonPanel,
    DI_TEXT,5,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  DialogItems[1].Selected=Opt.AddToDisksMenu;
  if (Opt.DisksMenuDigit)
    sprintf(DialogItems[2].Data,"%d",Opt.DisksMenuDigit);
  DialogItems[5].Selected=GetRegKey(HKEY_CURRENT_USER,"","CommonPanel",1);
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,10,"TempCfg",DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (ExitCode!=7)
    return(FALSE);
  Opt.AddToDisksMenu=DialogItems[1].Selected;
  Opt.DisksMenuDigit=atoi(DialogItems[2].Data);
  SetRegKey(HKEY_CURRENT_USER,"","AddToDisksMenu",Opt.AddToDisksMenu);
  SetRegKey(HKEY_CURRENT_USER,"","DisksMenuDigit",Opt.DisksMenuDigit);
  SetRegKey(HKEY_CURRENT_USER,"","CommonPanel",DialogItems[5].Selected);
  if (Opt.CommonPanel!=DialogItems[5].Selected)
  {
    char *MsgItems[]={GetMsg(MTempPanel),GetMsg(MConfigNewOption),GetMsg(MOk)};
    Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
  }
  return(TRUE);
}
