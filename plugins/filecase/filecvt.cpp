#ifndef UNICODE
#define GetCheck(i) DialogItems[i].Param.Selected
#define GetDataPtr(i) DialogItems[i].Data.Data
#else
#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#define GetDataPtr(i) ((const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
#endif

LONG_PTR WINAPI DlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  switch(Msg)
  {
    case DN_BTNCLICK:
      if(Param1==22)
      {
        Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,21,(LONG_PTR)TEXT(" _"));
        return TRUE;
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

void CaseConvertion()
{
  static const TCHAR History[] = _T("FileCase_WordDiv");
  struct InitDialogItem InitItems[]={
  /* 00 */{DI_DOUBLEBOX,3,1,62,19,0,0,0,0,(TCHAR *)MFileCase},
  /* 01 */{DI_TEXT,5,2,0,0,0,0,0,0,(TCHAR *)MName},
  /* 02 */{DI_TEXT,34,2,0,0,0,0,0,0,(TCHAR *)MExtension},
  /* 03 */{DI_RADIOBUTTON,5,3,0,0,0,0,DIF_GROUP,0,(TCHAR *)MLower},
  /* 04 */{DI_RADIOBUTTON,5,4,0,0,0,0,0,0,(TCHAR *)MUpper},
  /* 05 */{DI_RADIOBUTTON,5,5,0,0,0,0,0,0,(TCHAR *)MFirst},
  /* 06 */{DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(TCHAR *)MTitle},
  /* 07 */{DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(TCHAR *)MNone},
  /* 08 */{DI_RADIOBUTTON,34,3,0,0,0,0,DIF_GROUP,0,(TCHAR *)MLowerExt},
  /* 09 */{DI_RADIOBUTTON,34,4,0,0,0,0,0,0,(TCHAR *)MUpperExt},
  /* 10 */{DI_RADIOBUTTON,34,5,0,0,0,0,0,0,(TCHAR *)MFirstExt},
  /* 11 */{DI_RADIOBUTTON,34,6,0,0,0,0,0,0,(TCHAR *)MTitleExt},
  /* 12 */{DI_RADIOBUTTON,34,7,0,0,0,0,0,0,(TCHAR *)MNoneExt},
  /* 13 */{DI_TEXT,5,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /* 14 */{DI_CHECKBOX,5,9,0,0,0,0,0,0,(TCHAR *)MSkipMixedCase},
  /* 15 */{DI_CHECKBOX,5,10,0,0,0,0,0,0,(TCHAR *)MProcessSubDir},
  /* 16 */{DI_CHECKBOX,5,11,0,0,0,0,0,0,(TCHAR *)MProcessDir},
  /* 17 */{DI_TEXT,5,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /* 18 */{DI_CHECKBOX,5,13,0,0,0,0,0,0,(TCHAR *)MCurRun},
  /* 19 */{DI_TEXT,5,14,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /* 20 */{DI_TEXT,5,15,0,0,0,0,0,0,(TCHAR *)MWordDiv},
  /* 21 */{DI_EDIT,5,16,49,0,0,0,DIF_HISTORY,0,Opt.WordDiv},
  /* 22 */{DI_BUTTON,52,16,0,0,0,0,DIF_BTNNOCLOSE,0,(TCHAR *)MReset},
  /* 23 */{DI_TEXT,5,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
  /* 24 */{DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)MOk},
  /* 25 */{DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)MCancel}
  };
  struct FarDialogItem DialogItems[ArraySize(InitItems)];
  InitDialogItems(InitItems,DialogItems,ArraySize(InitItems));

  DialogItems[21].Param.History=History;
  DialogItems[3+Opt.ConvertMode].Focus=DialogItems[3+Opt.ConvertMode].Param.Selected=TRUE;
  DialogItems[8+Opt.ConvertModeExt].Param.Selected=TRUE;
  DialogItems[14].Param.Selected=Opt.SkipMixedCase;
  DialogItems[15].Param.Selected=Opt.ProcessSubDir;
  DialogItems[16].Param.Selected=Opt.ProcessDir;
  DialogItems[18].Param.Selected=0;

  int I, J;

#ifndef UNICODE
  I=Info.DialogEx
#else
  HANDLE hDlg=Info.DialogInit
#endif
    (Info.ModuleNumber,-1,-1,66,21,_T("Contents"),DialogItems,ArraySize(DialogItems),0,0,DlgProc,0);
#ifdef UNICODE
  if ( hDlg == INVALID_HANDLE_VALUE )
    return;
  I=Info.DialogRun(hDlg);
#endif

  if (I!=24)
   goto done;

  if (GetCheck(3)) I = MODE_LOWER;
  else if (GetCheck(4)) I = MODE_UPPER;
  else if (GetCheck(5)) I = MODE_N_WORD;
  else if (GetCheck(6)) I = MODE_LN_WORD;
  else if (GetCheck(7)) I = MODE_NONE;
  else goto done;

  if (GetCheck(8)) J = MODE_LOWER;
  else if (GetCheck(9)) J = MODE_UPPER;
  else if (GetCheck(10)) J = MODE_N_WORD;
  else if (GetCheck(11)) J = MODE_LN_WORD;
  else if (GetCheck(12)) J = MODE_NONE;
  else goto done;

  if (I==MODE_NONE && J==MODE_NONE)
    goto done;

  struct Options Backup;

  if (GetCheck(18))
    memcpy(&Backup,&Opt,sizeof(Backup));

  lstrcpy(Opt.WordDiv,GetDataPtr(21));
  Opt.WordDivLen=lstrlen(Opt.WordDiv);
  Opt.ConvertMode=I;
  Opt.ConvertModeExt=J;
  Opt.SkipMixedCase=GetCheck(14);
  Opt.ProcessSubDir=GetCheck(15);
  Opt.ProcessDir=GetCheck(16);

  struct PanelInfo PInfo;
#ifndef UNICODE
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&PInfo);
#else
  Info.Control(PANEL_ACTIVE,FCTL_GETPANELINFO,0,(LONG_PTR)&PInfo);
#endif
  HANDLE hScreen=Info.SaveScreen(0,0,-1,-1);
  const TCHAR *MsgItems[]={GetMsg(MFileCase),GetMsg(MConverting)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ArraySize(MsgItems),0);

  TCHAR FullName[NM];

#ifndef UNICODE
#define CurDir PInfo.CurDir
#else
	int Size=Info.Control(PANEL_ACTIVE,FCTL_GETCURRENTDIRECTORY,0,NULL);
	wchar_t* CurDir=new wchar_t[Size];
	Info.Control(PANEL_ACTIVE,FCTL_GETCURRENTDIRECTORY,Size,(LONG_PTR)CurDir);
#endif

#ifndef UNICODE
#define FileName PInfo.SelectedItems[I].FindData.cFileName
#define FileAttr PInfo.SelectedItems[I].FindData.dwFileAttributes
#else
#define FileName PPI->FindData.lpwszFileName
#define FileAttr PPI->FindData.dwFileAttributes
#endif
  for (I=0;I < PInfo.SelectedItemsNumber; I++)
  {
#ifdef UNICODE
    PluginPanelItem* PPI=(PluginPanelItem*)malloc(Info.Control(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,I,0));
    if(PPI)
    {
      Info.Control(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,I,(LONG_PTR)PPI);
#endif
    GetFullName(FullName,CurDir,FileName);
    ProcessName(FullName,FileAttr);
#ifdef UNICODE
      free(PPI);
    }
#endif
  }
#ifdef UNICODE
  delete[] CurDir;
#endif

  if (!GetCheck(18))
  {
    SetRegKey(HKEY_CURRENT_USER,_T(""),_T("WordDiv"),Opt.WordDiv);
    SetRegKey(HKEY_CURRENT_USER,_T(""),_T("ConvertMode"),Opt.ConvertMode);
    SetRegKey(HKEY_CURRENT_USER,_T(""),_T("ConvertModeExt"),Opt.ConvertModeExt);
    SetRegKey(HKEY_CURRENT_USER,_T(""),_T("SkipMixedCase"),Opt.SkipMixedCase);
    SetRegKey(HKEY_CURRENT_USER,_T(""),_T("ProcessSubDir"),Opt.ProcessSubDir);
    SetRegKey(HKEY_CURRENT_USER,_T(""),_T("ProcessDir"),Opt.ProcessDir);
  }
  else
    memcpy(&Opt,&Backup,sizeof(Opt));

  Info.RestoreScreen(hScreen);
#ifndef UNICODE
  Info.Control(INVALID_HANDLE_VALUE,FCTL_UPDATEPANEL,NULL);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_REDRAWPANEL,NULL);
#else
  Info.Control(PANEL_ACTIVE,FCTL_UPDATEPANEL,0,NULL);
  Info.Control(PANEL_ACTIVE,FCTL_REDRAWPANEL,0,NULL);
#endif

done:
#ifdef UNICODE
  Info.DialogFree(hDlg);
#endif
  return;
}
