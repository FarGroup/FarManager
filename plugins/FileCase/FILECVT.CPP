static int IsCaseMixed(char *Str);

void CaseConvertion()
{
  struct InitDialogItem InitItems[]={
    DI_DOUBLEBOX,3,1,62,11,0,0,0,0,(char *)MCvtTitle,
    DI_RADIOBUTTON,5,2,0,0,0,0,DIF_GROUP,0,"&Filename.ext",
    DI_RADIOBUTTON,5,3,0,0,0,0,0,0,"Filename.&Ext",
    DI_RADIOBUTTON,5,4,0,0,0,0,0,0,"file&name.ext",
    DI_RADIOBUTTON,5,5,0,0,0,0,0,0,"FI&LENAME.EXT",
    DI_RADIOBUTTON,5,6,0,0,0,0,0,0,"File N&ame.ext",
    DI_TEXT,5,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MCvtSkipMixedCase,
    DI_TEXT,5,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  DialogItems[1+Opt.ConvertMode].Focus=DialogItems[1+Opt.ConvertMode].Selected=TRUE;
  DialogItems[7].Selected=Opt.SkipMixedCase;
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,66,13,"Contents",DialogItems,
                           sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (ExitCode!=9)
    return;
  if (DialogItems[1].Selected)
    Opt.ConvertMode=0;
  else
    if (DialogItems[2].Selected)
      Opt.ConvertMode=1;
    else
      if (DialogItems[3].Selected)
        Opt.ConvertMode=2;
      else
        if (DialogItems[4].Selected)
          Opt.ConvertMode=3;
        else
          Opt.ConvertMode=4;
  Opt.SkipMixedCase=DialogItems[7].Selected;

  struct PanelInfo PInfo;
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&PInfo);

  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
  char *MsgItems[]={GetMsg(MCvtTitle),GetMsg(MConverting)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);

  for (int I=0;I<PInfo.SelectedItemsNumber;I++)
  {
    char NewName[NM];
    char *OldName=PInfo.SelectedItems[I].FindData.cFileName;
    OemToChar(OldName,NewName);
    if (*NewName==0 || Opt.SkipMixedCase && IsCaseMixed(NewName))
      continue;
    switch(Opt.ConvertMode)
    {
      case 0:
      case 1:
      case 4:
        {
          char *ExtPtr=strrchr(NewName,'.');
          *NewName=(char)CharUpper((LPTSTR)*NewName);
          CharLower(NewName+1);
          if (Opt.ConvertMode==1)
            if (ExtPtr!=NULL)
            {
              ExtPtr++;
              *ExtPtr=(char)CharUpper((LPTSTR)*ExtPtr);
            }
          if (Opt.ConvertMode==4 && NewName[1]!=0)
            for (int I=2;NewName[I]!=0 && &NewName[I]!=ExtPtr;I++)
              if (NewName[I-1]==' ' || NewName[I-1]=='_')
                NewName[I]=(char)CharUpper((LPTSTR)NewName[I]);
        }
        break;
      case 2:
        CharLower(NewName);
        break;
      case 3:
        CharUpper(NewName);
        break;
    }
    CharToOem(NewName,NewName);
    MoveFile(OldName,NewName);
  }
  SetRegKey(HKEY_CURRENT_USER,"","ConvertMode",Opt.ConvertMode);
  SetRegKey(HKEY_CURRENT_USER,"","SkipMixedCase",Opt.SkipMixedCase);
  Info.RestoreScreen(hScreen);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_UPDATEPANEL,NULL);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_REDRAWPANEL,NULL);
}


int IsCaseMixed(char *Str)
{
  while (*Str && !IsCharAlpha(*Str))
    Str++;
  int Case=IsCharLower(*Str);
  while (*(Str++))
    if (IsCharAlpha(*Str) && IsCharLower(*Str)!=Case)
      return(TRUE);
  return(FALSE);
}
