int Config()
{
  struct InitDialogItem InitItems[]={
    DI_DOUBLEBOX,3,1,72,19,0,0,0,0,(char *)MConfigTitle,
    DI_CHECKBOX,5,2,0,0,0,0,0,0,(char *)MConfigAddToDisksMenu,
    DI_FIXEDIT,7,3,7,3,1,0,0,0,"",
    DI_TEXT,9,3,0,0,0,0,0,0,(char *)MConfigDisksMenuDigit,
    DI_TEXT,5,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_TEXT,5,5,0,0,0,0,0,0,(char *)MConfigDizNames,
    DI_EDIT,5,6,70,6,0,0,0,0,"",
    DI_TEXT,5,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MConfigReadDiz,
    DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MConfigUpdateDiz,
    DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MConfigUploadLowCase,
    DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MConfigUploadDialog,
    DI_TEXT,5,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_TEXT,5,13,0,0,0,0,0,0,(char *)MConfigDefPassword,
    DI_PSWEDIT,5,14,34,11,0,0,0,0,"",
    DI_TEXT,5,15,0,0,0,0,0,0,(char *)MConfigFirewall,
    DI_EDIT,5,16,34,11,0,0,0,0,"",
    DI_CHECKBOX,37,16,0,0,0,0,0,0,(char *)MConfigPassiveMode,
    DI_TEXT,5,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  DialogItems[1].Selected=Opt.AddToDisksMenu;
  if (Opt.DisksMenuDigit)
    sprintf(DialogItems[2].Data,"%d",Opt.DisksMenuDigit);
  strcpy(DialogItems[6].Data,Opt.DescriptionNames);
  DialogItems[8].Selected=Opt.ReadDescriptions;
  DialogItems[9].Selected=Opt.UpdateDescriptions;
  DialogItems[10].Selected=Opt.UploadLowCase;
  DialogItems[11].Selected=Opt.ShowUploadDialog;
  strcpy(DialogItems[14].Data,Opt.DefaultPassword);
  strcpy(DialogItems[16].Data,Opt.Firewall);
  DialogItems[17].Selected=Opt.PassiveMode;
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,21,"FTPCfg",DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (ExitCode!=19)
    return(FALSE);
  Opt.AddToDisksMenu=DialogItems[1].Selected;
  Opt.DisksMenuDigit=atoi(DialogItems[2].Data);
  strcpy(Opt.DescriptionNames,DialogItems[6].Data);
  Opt.ReadDescriptions=DialogItems[8].Selected;
  Opt.UpdateDescriptions=DialogItems[9].Selected;
  Opt.UploadLowCase=DialogItems[10].Selected;
  Opt.ShowUploadDialog=DialogItems[11].Selected;
  strcpy(Opt.DefaultPassword,DialogItems[14].Data);
  strcpy(Opt.Firewall,DialogItems[16].Data);
  Opt.PassiveMode=DialogItems[17].Selected;
  SetRegKey(HKEY_CURRENT_USER,"","AddToDisksMenu",Opt.AddToDisksMenu);
  SetRegKey(HKEY_CURRENT_USER,"","DisksMenuDigit",Opt.DisksMenuDigit);
  SetRegKey(HKEY_CURRENT_USER,"","DescriptionNames",Opt.DescriptionNames);
  SetRegKey(HKEY_CURRENT_USER,"","ReadDescriptions",Opt.ReadDescriptions);
  SetRegKey(HKEY_CURRENT_USER,"","UpdateDescriptions",Opt.UpdateDescriptions);
  SetRegKey(HKEY_CURRENT_USER,"","UploadLowCase",Opt.UploadLowCase);
  SetRegKey(HKEY_CURRENT_USER,"","ShowUploadDialog",Opt.ShowUploadDialog);
  char CryptedPassword[256];
  FTP::EncryptPassword(Opt.DefaultPassword,CryptedPassword);
  SetRegKey(HKEY_CURRENT_USER,"","DefaultPassword",(BYTE *)CryptedPassword,strlen(CryptedPassword)+1);
  SetRegKey(HKEY_CURRENT_USER,"","Firewall",Opt.Firewall);
  SetRegKey(HKEY_CURRENT_USER,"","PassiveMode",Opt.PassiveMode);
  return(TRUE);
}

