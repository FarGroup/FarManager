void _fastcall TMacroView::ReadConfig()
{
  if (Reg->OpenKey(PluginRootKey))
  {
    Conf.AddDescription=Reg->ReadInteger(_T("AddDescription"));
    Conf.AutomaticSave=Reg->ReadInteger(_T("AutomaticSave"));
    Conf.UseHighlight=Reg->ReadInteger(_T("UseHighlight"));
    Conf.StartDependentSort=Reg->ReadInteger(_T("StartDependentSort"));
    Conf.LongGroupNames=Reg->ReadInteger(_T("LongGroupNames"));
    Conf.MenuCycle=Reg->ReadInteger(_T("MenuCycle"));
    Conf.DblClick=Reg->ReadInteger(_T("DblClick"));
    Conf.GroupDivider=Reg->ReadInteger(_T("GroupDivider"));
    Conf.SaveOnStart=Reg->ReadInteger(_T("SaveOnStart"));
    Conf.ViewShell=Reg->ReadInteger(_T("ViewShell"));
    Conf.ViewViewer=Reg->ReadInteger(_T("ViewViewer"));
    Conf.ViewEditor=Reg->ReadInteger(_T("ViewEditor"));

    if (Conf.AddDescription<0)
    {
      Reg->WriteInteger(_T("AddDescription"),1);
      Conf.AddDescription=1;
    }

    if (Conf.AutomaticSave<0)
      Conf.AutomaticSave=0;

    if (Conf.UseHighlight<0)
    {
      Reg->WriteInteger(_T("UseHighlight"),1);
      Conf.UseHighlight=1;
    }

    if (Conf.StartDependentSort<0)
      Conf.StartDependentSort=0;

    if (Conf.LongGroupNames<0)
    {
      Reg->WriteInteger(_T("LongGroupNames"),1);
      Conf.LongGroupNames=1;
    }

    if (Conf.MenuCycle<0)
    {
      Reg->WriteInteger(_T("MenuCycle"),1);
      Conf.MenuCycle=1;
    }

    if (Conf.DblClick<0)
    {
      Reg->WriteInteger(_T("DblClick"),1);
      Conf.DblClick=1;
    }

    if (Conf.GroupDivider<0)
    {
      Reg->WriteInteger(_T("GroupDivider"),1);
      Conf.GroupDivider=1;
    }

    if (Conf.SaveOnStart<0)
    {
      Reg->WriteInteger(_T("SaveOnStart"),1);
      Conf.SaveOnStart=1;
    }

    if (Conf.ViewShell<0)
      Conf.ViewShell=0;

    if (Conf.ViewViewer<0)
      Conf.ViewViewer=0;

    if (Conf.ViewEditor<0)
      Conf.ViewEditor=0;
    Reg->CloseKey();
  }
  else
  {
    Conf.AddDescription=1;
    Conf.AutomaticSave=0;
    Conf.UseHighlight=1;
    Conf.StartDependentSort=0;
    Conf.LongGroupNames=1;
    Conf.MenuCycle=1;
    Conf.DblClick=1;
    Conf.GroupDivider=1;
    Conf.SaveOnStart=1;
    Conf.ViewShell=0;
    Conf.ViewViewer=0;
    Conf.ViewEditor=0;
  }
}

BOOL _fastcall TMacroView::Configure()
{
  BOOL Result;
  int OutCode;
  TCHAR *ItemsErrorWrite[]=
  {
    GetMsg(MMacroError),GetMsg(MMacroErrorWrite),
    S,GetMsg(MMacroOk),
  };

  struct InitDialogItem InitItems[]=
  {
  /* 0  */ {DI_DOUBLEBOX,3,1,70,18,0,0,0,0,(TCHAR *)MMacroConfig},
  /* 1  */ {DI_CHECKBOX,5,2,0,0,1,0,0,0,(TCHAR *)MMacroAddDescription},
  /* 2  */ {DI_CHECKBOX,5,3,0,0,0,0,0,0,(TCHAR *)MMacroAutomaticSave},
  /* 3  */ {DI_CHECKBOX,5,4,0,0,0,0,0,0,(TCHAR *)MMacroUseHighlight},
  /* 4  */ {DI_CHECKBOX,5,5,0,0,0,0,0,0,(TCHAR *)MMacroStartDependentSort},
  /* 5  */ {DI_CHECKBOX,5,6,0,0,0,0,0,0,(TCHAR *)MMacroLongGroupNames},
  /* 6  */ {DI_CHECKBOX,5,7,0,0,0,0,0,0,(TCHAR *)MMacroMenuCycle},
  /* 7  */ {DI_CHECKBOX,5,8,0,0,0,0,0,0,(TCHAR *)MMacroDblClick},
  /* 8  */ {DI_CHECKBOX,5,9,0,0,0,0,0,0,(TCHAR *)MMacroGroupDivider},
  /* 9  */ {DI_TEXT,5,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /* 10 */ {DI_CHECKBOX,5,11,0,0,0,0,0,0,(TCHAR *)MMacroSaveOnStart},
  /* 11 */ {DI_TEXT,5,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /* 12 */ {DI_CHECKBOX,5,13,0,0,0,0,0,0,(TCHAR *)MMacroViewShell},
  /* 13 */ {DI_CHECKBOX,5,14,0,0,0,0,0,0,(TCHAR *)MMacroViewViewer},
  /* 14 */ {DI_CHECKBOX,5,15,0,0,0,0,0,0,(TCHAR *)MMacroViewEditor},
  /* 15 */ {DI_TEXT,5,16,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /* 16 */ {DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)MMacroSave},
  /* 17 */ {DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)MMacroCancel},
  };

  int size=ArraySize(InitItems);

  FarDialogItem *DialogItems=new FarDialogItem[size];
  InitDialogItems(InitItems,DialogItems,size);

  Reg->CreateKey(PluginRootKey);
  ReadConfig();

  DialogItems[1].Param.Selected=Conf.AddDescription;
  DialogItems[2].Param.Selected=Conf.AutomaticSave;
  DialogItems[3].Param.Selected=Conf.UseHighlight;
  DialogItems[4].Param.Selected=Conf.StartDependentSort;
  DialogItems[5].Param.Selected=Conf.LongGroupNames;
  DialogItems[6].Param.Selected=Conf.MenuCycle;
  DialogItems[7].Param.Selected=Conf.DblClick;
  DialogItems[8].Param.Selected=Conf.GroupDivider;
  DialogItems[10].Param.Selected=Conf.SaveOnStart;
  DialogItems[12].Param.Selected=Conf.ViewShell;
  DialogItems[13].Param.Selected=Conf.ViewViewer;
  DialogItems[14].Param.Selected=Conf.ViewEditor;
  OutCode=Info.Dialog(Info.ModuleNumber,-1,-1,74,20,_T("Config"),DialogItems,size
#ifdef UNICODE
                      ,NULL
#endif
                     );

  if (OutCode==16) // кнопка [Сохранить]
  {
    Reg->OpenKey(PluginRootKey);
    if ((Reg->WriteInteger(_T("AddDescription"),DialogItems[1].Param.Selected)) &&
        (Reg->WriteInteger(_T("AutomaticSave"),DialogItems[2].Param.Selected)) &&
        (Reg->WriteInteger(_T("UseHighlight"),DialogItems[3].Param.Selected)) &&
        (Reg->WriteInteger(_T("StartDependentSort"),DialogItems[4].Param.Selected)) &&
        (Reg->WriteInteger(_T("LongGroupNames"),DialogItems[5].Param.Selected)) &&
        (Reg->WriteInteger(_T("MenuCycle"),DialogItems[6].Param.Selected)) &&
        (Reg->WriteInteger(_T("DblClick"),DialogItems[7].Param.Selected)) &&
        (Reg->WriteInteger(_T("GroupDivider"),DialogItems[8].Param.Selected)) &&
        (Reg->WriteInteger(_T("SaveOnStart"),DialogItems[10].Param.Selected)) &&
        (Reg->WriteInteger(_T("ViewShell"),DialogItems[12].Param.Selected)) &&
        (Reg->WriteInteger(_T("ViewViewer"),DialogItems[13].Param.Selected)) &&
        (Reg->WriteInteger(_T("ViewEditor"),DialogItems[14].Param.Selected)))
    {
      Conf.AddDescription=DialogItems[1].Param.Selected;
      Conf.AutomaticSave=DialogItems[2].Param.Selected;
      Conf.UseHighlight=DialogItems[3].Param.Selected;
      Conf.StartDependentSort=DialogItems[4].Param.Selected;
      Conf.LongGroupNames=DialogItems[5].Param.Selected;
      Conf.MenuCycle=DialogItems[6].Param.Selected;
      Conf.DblClick=DialogItems[7].Param.Selected;
      Conf.GroupDivider=DialogItems[8].Param.Selected;
      Conf.SaveOnStart=DialogItems[10].Param.Selected;
      Conf.ViewShell=DialogItems[12].Param.Selected;
      Conf.ViewViewer=DialogItems[13].Param.Selected;
      Conf.ViewEditor=DialogItems[14].Param.Selected;
      Result=TRUE;
    }
    else
    {
      lstrcpy(ItemsErrorWrite[2],PluginRootKey);
      QuoteText(ItemsErrorWrite[2]);
      Info.Message(Info.ModuleNumber,FMSG_WARNING,NULL,ItemsErrorWrite,
                   ArraySize(ItemsErrorWrite),1);
      Result=FALSE;
    }
  }
  else
    Result=FALSE;
  delete[] DialogItems;
  Reg->CloseKey();
  return Result;
}
