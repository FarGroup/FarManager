int ConfigGeneral()
{
  struct InitDialogItem InitItems[]={
  /*  0 */DI_DOUBLEBOX,3,1,72,16,0,0,0,0,(char *)MGConfigTitle,
  /*  1 */DI_RADIOBUTTON,5,2,0,0,1,0,DIF_GROUP,0,(char *)MGConfigHideNone,
  /*  2 */DI_RADIOBUTTON,5,3,0,0,0,0,0,0,(char *)MGConfigHideView,
  /*  3 */DI_RADIOBUTTON,5,4,0,0,0,0,0,0,(char *)MGConfigHideAlways,
  /*  4 */DI_TEXT,5,5,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /*  5 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MGConfigProcessShiftF1,
  /*  6 */DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MGConfigUseLastHistory,
  /*  7 */DI_TEXT,5,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /*  8 */DI_TEXT,5,9,0,0,0,0,0,0,(char *)MGConfigDizNames,
  /*  9 */DI_EDIT,5,10,70,10,0,0,0,0,"",
  /* 10 */DI_TEXT,5,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 11 */DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)MGConfigReadDiz,
  /* 12 */DI_CHECKBOX,5,13,0,0,0,0,0,0,(char *)MGConfigUpdateDiz,
  /* 13 */DI_TEXT,5,14,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 14 */DI_BUTTON,0,15,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 15 */DI_BUTTON,0,15,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };

  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  DialogItems[1].Selected=(Opt.HideOutput==0);
  DialogItems[2].Selected=(Opt.HideOutput==1);
  DialogItems[3].Selected=(Opt.HideOutput==2);
  DialogItems[5].Selected=Opt.ProcessShiftF1;
  DialogItems[6].Selected=Opt.UseLastHistory;
  strcpy(DialogItems[9].Data,Opt.DescriptionNames);
  DialogItems[11].Selected=Opt.ReadDescriptions;
  DialogItems[12].Selected=Opt.UpdateDescriptions;
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,18,"ArcSettings1",DialogItems,
                           sizeof(DialogItems)/sizeof(DialogItems[0]));
  if (ExitCode==15 || ExitCode < 0)
    return(FALSE);
  if (DialogItems[1].Selected)
    Opt.HideOutput=0;
  else
    Opt.HideOutput=(DialogItems[2].Selected) ? 1 : 2;
  Opt.ProcessShiftF1=DialogItems[5].Selected;
  Opt.UseLastHistory=DialogItems[6].Selected;
  strcpy(Opt.DescriptionNames,DialogItems[9].Data);
  Opt.ReadDescriptions=DialogItems[11].Selected;
  Opt.UpdateDescriptions=DialogItems[12].Selected;
  SetRegKey(HKEY_CURRENT_USER,"","HideOutput",Opt.HideOutput);
  SetRegKey(HKEY_CURRENT_USER,"","UseLastHistory",Opt.UseLastHistory);
  SetRegKey(HKEY_CURRENT_USER,"","ProcessShiftF1",Opt.ProcessShiftF1);
  SetRegKey(HKEY_CURRENT_USER,"","DescriptionNames",Opt.DescriptionNames);
  SetRegKey(HKEY_CURRENT_USER,"","ReadDescriptions",Opt.ReadDescriptions);
  SetRegKey(HKEY_CURRENT_USER,"","UpdateDescriptions",Opt.UpdateDescriptions);
  return(TRUE);
}


int ConfigCommands()
{
  char ArcFormat[100];
  *ArcFormat=0;
  while(PluginClass::SelectFormat(ArcFormat))
  {
    struct InitDialogItem InitItems[]={
      DI_DOUBLEBOX,3,1,72,19,0,0,0,0,ArcFormat,
      DI_TEXT,5,2,0,0,0,0,0,0,(char *)MArcSettingsExtract,
      DI_EDIT,25,2,70,3,1,0,0,0,"",
      DI_TEXT,5,3,0,0,0,0,0,0,(char *)MArcSettingsExtractWithoutPath,
      DI_EDIT,25,3,70,3,0,0,0,0,"",
      DI_TEXT,5,4,0,0,0,0,0,0,(char *)MArcSettingsTest,
      DI_EDIT,25,4,70,3,0,0,0,0,"",
      DI_TEXT,5,5,0,0,0,0,0,0,(char *)MArcSettingsDelete,
      DI_EDIT,25,5,70,3,0,0,0,0,"",
      DI_TEXT,5,6,0,0,0,0,0,0,(char *)MArcSettingsComment,
      DI_EDIT,25,6,70,3,0,0,0,0,"",
      DI_TEXT,5,7,0,0,0,0,0,0,(char *)MArcSettingsCommentFiles,
      DI_EDIT,25,7,70,3,0,0,0,0,"",
      DI_TEXT,5,8,0,0,0,0,0,0,(char *)MArcSettingsSFX,
      DI_EDIT,25,8,70,3,0,0,0,0,"",
      DI_TEXT,5,9,0,0,0,0,0,0,(char *)MArcSettingsLock,
      DI_EDIT,25,9,70,3,0,0,0,0,"",
      DI_TEXT,5,10,0,0,0,0,0,0,(char *)MArcSettingsProtect,
      DI_EDIT,25,10,70,3,0,0,0,0,"",
      DI_TEXT,5,11,0,0,0,0,0,0,(char *)MArcSettingsRecover,
      DI_EDIT,25,11,70,3,0,0,0,0,"",
      DI_TEXT,5,12,0,0,0,0,0,0,(char *)MArcSettingsAdd,
      DI_EDIT,25,12,70,3,0,0,0,0,"",
      DI_TEXT,5,13,0,0,0,0,0,0,(char *)MArcSettingsMove,
      DI_EDIT,25,13,70,3,0,0,0,0,"",
      DI_TEXT,5,14,0,0,0,0,0,0,(char *)MArcSettingsAddRecurse,
      DI_EDIT,25,14,70,3,0,0,0,0,"",
      DI_TEXT,5,15,0,0,0,0,0,0,(char *)MArcSettingsMoveRecurse,
      DI_EDIT,25,15,70,3,0,0,0,0,"",
      DI_TEXT,5,16,0,0,0,0,0,0,(char *)MArcSettingsAllFilesMask,
      DI_EDIT,25,16,70,3,0,0,0,0,"",
      DI_TEXT,3,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
      DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
    };

    struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
    InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
    int Length,MaxLength=0, I,J;
    for (I=1;I<=29;I+=2)
      if ((Length=strlen(DialogItems[I].Data))>MaxLength)
        MaxLength=Length;
    for (I=2,J=0;I<=30;I+=2,J++)
    {
      char *Command=DialogItems[I].Data;
      DialogItems[I].X1=6+MaxLength;
      GetRegKey(HKEY_LOCAL_MACHINE,ArcFormat,CmdNames[J],Command,"",sizeof(DialogItems[I].Data));
      if (*Command==0)
      {
        int PluginNumber,PluginType;
        if (PluginClass::FormatToPlugin(ArcFormat,PluginNumber,PluginType))
          ArcPlugin->GetDefaultCommands(PluginNumber,PluginType,J,Command);
      }
    }
    int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,76,21,"ArcSettings2",
                 DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
    if (ExitCode!=32)
      continue;
    for (I=2,J=0;I<=30;I+=2,J++)
      SetRegKey(HKEY_LOCAL_MACHINE,ArcFormat,CmdNames[J],DialogItems[I].Data);
  }
  return(TRUE);
}
