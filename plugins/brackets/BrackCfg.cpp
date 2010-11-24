#ifndef UNICODE
#define GetCheck(i) DialogItems[i].Param.Selected
#define GetDataPtr(i) DialogItems[i].Data.Data
#else
#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#define GetDataPtr(i) ((const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
#endif

int Config()
{
  static struct InitDialogItem InitItems[]={
  /* 0*/  {DI_DOUBLEBOX,3,1,57,19,0,0,0,0,(TCHAR *)MTitle},

  /* 1*/  {DI_SINGLEBOX,5,2,55,8,0,0,DIF_LEFTTEXT,0,(TCHAR *)MRules},
  /* 2*/  {DI_CHECKBOX,7,3,0,0,0,0,0,0,(TCHAR*)MIgnoreQuotation},
  /* 3*/  {DI_CHECKBOX,7,4,0,0,0,0,0,0,(TCHAR*)MIgnoreAfter},
  /* 4*/  {DI_CHECKBOX,7,5,0,0,0,0,0,0,(TCHAR*)MPriority},
  /* 5*/  {DI_CHECKBOX,7,6,0,0,0,0,0,0,(TCHAR*)MJumpToPair},
  /* 6*/  {DI_CHECKBOX,7,7,0,0,0,0,0,0,(TCHAR*)MBeep},

  /* 7*/  {DI_SINGLEBOX,5,9,55,16,0,0,DIF_LEFTTEXT,0,(TCHAR *)MDescriptions},
  /* 8*/  {DI_TEXT,7,10,0,0,0,0,0,0,(TCHAR*)MTypeQuotes},
  /* 9*/  {DI_FIXEDIT,7,11,26,0,0,0,0,0,_T("")},
  /*10*/  {DI_TEXT,7,12,0,0,0,0,0,0,(TCHAR*)MDescript1},
  /*11*/  {DI_FIXEDIT,7,13,26,0,0,0,0,0,_T("")},
  /*12*/  {DI_TEXT,7,14,0,0,0,0,0,0,(TCHAR*)MDescript2},
  /*13*/  {DI_FIXEDIT,7,15,46,0,0,0,0,0,_T("")},

  /*14*/  {DI_TEXT,-1,17,0,0,0,0,DIF_SEPARATOR,0,_T("")},
  /*15*/  {DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)MSave},
  /*16*/  {DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)MCancel}
  };

  struct FarDialogItem DialogItems[ARRAYSIZE(InitItems)];
  InitDialogItems(InitItems,DialogItems,ARRAYSIZE(InitItems));

  DialogItems[2].Param.Selected=Opt.IgnoreQuotes;
  DialogItems[3].Param.Selected=Opt.IgnoreAfter;
  DialogItems[4].Param.Selected=Opt.BracketPrior;
  DialogItems[5].Param.Selected=Opt.JumpToPair;
  DialogItems[6].Param.Selected=Opt.Beep;
#ifndef UNICODE
#define SET_DLGITEM(n,v)  lstrcpy(DialogItems[n].Data.Data, v)
#else
#define SET_DLGITEM(n,v)  DialogItems[n].PtrData = v
#endif
  SET_DLGITEM(9,Opt.QuotesType);
  SET_DLGITEM(11,Opt.Brackets1);
  SET_DLGITEM(13,Opt.Brackets2);

  BOOL result = FALSE;
#ifndef UNICODE
  int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,61,21,_T("Config"),
                           DialogItems,ARRAYSIZE(DialogItems));
#else
  HANDLE hDlg = Info.DialogInit(Info.ModuleNumber,-1,-1,61,21,_T("Config"),
                                DialogItems,ARRAYSIZE(DialogItems),0,0,NULL,0);

  if (hDlg == INVALID_HANDLE_VALUE)
    return(FALSE);

  int ExitCode=Info.DialogRun(hDlg);
#endif
  if (ExitCode!=15) goto done;

  Opt.IgnoreQuotes=GetCheck(2);
  Opt.IgnoreAfter=GetCheck(3);
  Opt.BracketPrior=GetCheck(4);
  Opt.JumpToPair=GetCheck(5);
  Opt.Beep=GetCheck(6);
  lstrcpy(Opt.QuotesType,GetDataPtr(9));
  lstrcpy(Opt.Brackets1,GetDataPtr(11));
  lstrcpy(Opt.Brackets2,GetDataPtr(13));

  HKEY hKey;
  DWORD Disposition;
  if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,
                  KEY_WRITE,NULL,&hKey,&Disposition)) == ERROR_SUCCESS)
  {
    RegSetValueEx(hKey,_T("Options"),0,REG_BINARY,(LPBYTE)&Opt,sizeof(Opt));
    RegCloseKey(hKey);
  }
  result = TRUE;
done:
#ifdef UNICODE
  Info.DialogFree(hDlg);
#endif
  return(result);
}
