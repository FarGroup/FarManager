int ResetButtonID;
int WordDivEditID;

INT_PTR WINAPI DlgProc(HANDLE hDlg,int Msg,int Param1,void *Param2)
{
  switch(Msg)
  {
    case DN_BTNCLICK:
      if (Param1==22)
      {
        Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,21,(void *)L" _");
        return TRUE;
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void CaseConvertion()
{
  struct Options Backup;
  memcpy(&Backup,&Opt,sizeof(Backup));

  PluginDialogBuilder Builder(Info, MainGuid, DialogGuid, MFileCase, L"Contents", DlgProc);

  Builder.StartColumns();

  Builder.AddText(MName);
  const int NameIDs[] = {MLower, MUpper, MFirst, MTitle, MNone};
  Builder.AddRadioButtons(&Opt.ConvertMode, ARRAYSIZE(NameIDs), NameIDs, true);

  Builder.ColumnBreak();

  Builder.AddText(MExtension);
  const int ExtIDs[] = {MLowerExt, MUpperExt, MFirstExt, MTitleExt, MNoneExt};
  Builder.AddRadioButtons(&Opt.ConvertModeExt, ARRAYSIZE(ExtIDs), ExtIDs);

  Builder.EndColumns();

  Builder.AddSeparator();

  Builder.AddCheckbox(MSkipMixedCase, &Opt.SkipMixedCase);
  Builder.AddCheckbox(MProcessSubDir, &Opt.ProcessSubDir);
  Builder.AddCheckbox(MProcessDir, &Opt.ProcessDir);

  Builder.AddSeparator();

  int CurRun = 0;
  Builder.AddCheckbox(MCurRun, &CurRun);

  Builder.AddSeparator();

  Builder.AddText(MWordDiv);
  FarDialogItem *Edit = Builder.AddEditField(Opt.WordDiv,ARRAYSIZE(Opt.WordDiv),20,L"FileCase_WordDiv");
  WordDivEditID = Builder.GetLastID();
  Builder.AddButtonAfter(Edit, MReset);
  ResetButtonID = Builder.GetLastID();

  Builder.AddOKCancel(MOk, MCancel);

  if (Builder.ShowDialog())
  {
    if (Opt.ConvertMode!=MODE_NONE || Opt.ConvertModeExt!=MODE_NONE)
    {
      Opt.WordDivLen=lstrlen(Opt.WordDiv);

      struct PanelInfo PInfo;
      Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&PInfo);
      HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
      const wchar_t *MsgItems[]={GetMsg(MFileCase),GetMsg(MConverting)};
      Info.Message(&MainGuid,0,NULL,MsgItems,ARRAYSIZE(MsgItems),0);

      wchar_t FullName[MAX_PATH];

      int Size=(int)Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIR,0,0);
      wchar_t* CurDir=new wchar_t[Size];
      Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIR,Size,CurDir);

      for (int I=0;I < PInfo.SelectedItemsNumber; I++)
      {
        PluginPanelItem* PPI=(PluginPanelItem*)malloc(Info.PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,I,0));
        if(PPI)
        {
          Info.PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,I,PPI);
          GetFullName(FullName,CurDir,PPI->FileName);
          ProcessName(FullName,PPI->FileAttributes);
          free(PPI);
        }
      }
      delete[] CurDir;

      if (!CurRun)
      {
        PluginSettings settings(MainGuid, Info.SettingsControl);
        settings.Set(0,L"WordDiv",Opt.WordDiv);
        settings.Set(0,L"ConvertMode",Opt.ConvertMode);
        settings.Set(0,L"ConvertModeExt",Opt.ConvertModeExt);
        settings.Set(0,L"SkipMixedCase",Opt.SkipMixedCase);
        settings.Set(0,L"ProcessSubDir",Opt.ProcessSubDir);
        settings.Set(0,L"ProcessDir",Opt.ProcessDir);
      }

      Info.RestoreScreen(hScreen);
      Info.PanelControl(PANEL_ACTIVE,FCTL_UPDATEPANEL,0,0);
      Info.PanelControl(PANEL_ACTIVE,FCTL_REDRAWPANEL,0,0);
    }

    if (CurRun)
      memcpy(&Opt,&Backup,sizeof(Opt));
  }
}
