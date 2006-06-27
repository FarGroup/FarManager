void CaseConvertion()
{
  static const char History[] = "FileCase_WordDiv";
  struct InitDialogItem InitItems[]={
  /* 00 */{DI_DOUBLEBOX,3,1,62,19,0,0,0,0,(char *)MFileCase},
  /* 01 */{DI_TEXT,5,2,0,0,0,0,0,0,(char *)MName},
  /* 02 */{DI_TEXT,34,2,0,0,0,0,0,0,(char *)MExtension},
  /* 03 */{DI_RADIOBUTTON,5,3,0,0,0,0,DIF_GROUP,0,(char *)MLower},
  /* 04 */{DI_RADIOBUTTON,5,4,0,0,0,0,0,0,(char *)MUpper},
  /* 05 */{DI_RADIOBUTTON,5,5,0,0,0,0,0,0,(char *)MFirst},
  /* 06 */{DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(char *)MTitle},
  /* 07 */{DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(char *)MNone},
  /* 08 */{DI_RADIOBUTTON,34,3,0,0,0,0,DIF_GROUP,0,(char *)MLowerExt},
  /* 09 */{DI_RADIOBUTTON,34,4,0,0,0,0,0,0,(char *)MUpperExt},
  /* 10 */{DI_RADIOBUTTON,34,5,0,0,0,0,0,0,(char *)MFirstExt},
  /* 11 */{DI_RADIOBUTTON,34,6,0,0,0,0,0,0,(char *)MTitleExt},
  /* 12 */{DI_RADIOBUTTON,34,7,0,0,0,0,0,0,(char *)MNoneExt},
  /* 13 */{DI_TEXT,5,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /* 14 */{DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MSkipMixedCase},
  /* 15 */{DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MProcessSubDir},
  /* 16 */{DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MProcessDir},
  /* 17 */{DI_TEXT,5,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /* 18 */{DI_CHECKBOX,5,13,0,0,0,0,0,0,(char *)MCurRun},
  /* 19 */{DI_TEXT,5,14,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /* 20 */{DI_TEXT,5,15,0,0,0,0,0,0,(char *)MWordDiv},
  /* 21 */{DI_EDIT,5,16,49,0,0,0,DIF_HISTORY,0,Opt.WordDiv},
  /* 22 */{DI_BUTTON,52,16,0,0,0,0,0,0,(char *)MReset},
  /* 23 */{DI_TEXT,5,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,""},
  /* 24 */{DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk},
  /* 25 */{DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel}
  };
  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));

  DialogItems[21].Param.History=History;
  DialogItems[3+Opt.ConvertMode].Focus=DialogItems[3+Opt.ConvertMode].Param.Selected=TRUE;
  DialogItems[8+Opt.ConvertModeExt].Param.Selected=TRUE;
  DialogItems[14].Param.Selected=Opt.SkipMixedCase;
  DialogItems[15].Param.Selected=Opt.ProcessSubDir;
  DialogItems[16].Param.Selected=Opt.ProcessDir;
  DialogItems[18].Param.Selected=0;

  while (1)
  {
    int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,66,21,
                            "Contents",DialogItems,
                            sizeof(DialogItems)/sizeof(DialogItems[0]));
    if (ExitCode==22)
      lstrcpy(DialogItems[21].Data.Data," _");
    else if (ExitCode!=24)
      return;
    else
      break;
  }

  int I, J;
  if (DialogItems[3].Param.Selected) I = MODE_LOWER;    else
  if (DialogItems[4].Param.Selected) I = MODE_UPPER;    else
  if (DialogItems[5].Param.Selected) I = MODE_N_WORD;   else
  if (DialogItems[6].Param.Selected) I = MODE_LN_WORD;  else
  if (DialogItems[7].Param.Selected) I = MODE_NONE;  else
    return;
  if (DialogItems[8].Param.Selected) J = MODE_LOWER;    else
  if (DialogItems[9].Param.Selected) J = MODE_UPPER;    else
  if (DialogItems[10].Param.Selected) J = MODE_N_WORD;   else
  if (DialogItems[11].Param.Selected) J = MODE_LN_WORD;  else
  if (DialogItems[12].Param.Selected) J = MODE_NONE;  else
    return;

  if (I==MODE_NONE && J==MODE_NONE)
    return;

  struct Options Backup;

  if(DialogItems[18].Param.Selected)
    memcpy(&Backup,&Opt,sizeof(Backup));

  lstrcpy(Opt.WordDiv,DialogItems[21].Data.Data);
  Opt.WordDivLen=lstrlen(Opt.WordDiv);
  Opt.ConvertMode=I;
  Opt.ConvertModeExt=J;
  Opt.SkipMixedCase=DialogItems[14].Param.Selected;
  Opt.ProcessSubDir=DialogItems[15].Param.Selected;
  Opt.ProcessDir=DialogItems[16].Param.Selected;

  struct PanelInfo PInfo;
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&PInfo);

  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
  const char *MsgItems[]={GetMsg(MFileCase),GetMsg(MConverting)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);

  char FullName[NM];

  for (I=0;I < PInfo.SelectedItemsNumber; I++)
  {
    GetFullName(FullName,PInfo.CurDir,
                PInfo.SelectedItems[I].FindData.cFileName);

    ProcessName(FullName,PInfo.SelectedItems[I].FindData.dwFileAttributes);
  }

  if(!DialogItems[18].Param.Selected)
  {
    SetRegKey(HKEY_CURRENT_USER,"","WordDiv",Opt.WordDiv);
    SetRegKey(HKEY_CURRENT_USER,"","ConvertMode",Opt.ConvertMode);
    SetRegKey(HKEY_CURRENT_USER,"","ConvertModeExt",Opt.ConvertModeExt);
    SetRegKey(HKEY_CURRENT_USER,"","SkipMixedCase",Opt.SkipMixedCase);
    SetRegKey(HKEY_CURRENT_USER,"","ProcessSubDir",Opt.ProcessSubDir);
    SetRegKey(HKEY_CURRENT_USER,"","ProcessDir",Opt.ProcessDir);
  }
  else
    memcpy(&Opt,&Backup,sizeof(Opt));

  Info.RestoreScreen(hScreen);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_UPDATEPANEL,NULL);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_REDRAWPANEL,NULL);
}
