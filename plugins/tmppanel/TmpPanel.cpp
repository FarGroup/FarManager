/*
TMPPANEL.CPP

Temporary panel main plugin code

*/

#include "TmpPanel.hpp"

TCHAR *PluginRootKey;
unsigned int CurrentCommonPanel;
struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;

PluginPanels CommonPanels[COMMONPANELSNUMBER];

#if defined(__GNUC__)

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) lpReserved;
  (void) dwReason;
  (void) hDll;
  return TRUE;
}
#endif

#ifndef UNICODE
#define WITH_ANSI_ARG     ,ANSI
#define WITH_ANSI_PARAM   ,int ANSI
#else
#define WITH_ANSI_ARG
#define WITH_ANSI_PARAM
#endif
static void ProcessList(HANDLE hPlugin, TCHAR *Name, int Mode WITH_ANSI_PARAM);
static void ShowMenuFromList(TCHAR *Name);
static HANDLE OpenPanelFromOutput (TCHAR *argv WITH_ANSI_PARAM);

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;

  PluginRootKey = (TCHAR *)malloc(lstrlen(Info->RootKey)*sizeof(TCHAR) + sizeof(_T("\\TmpPanel")));
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,_T("\\TmpPanel"));
  GetOptions();
  StartupOptFullScreenPanel=Opt.FullScreenPanel;
  StartupOptCommonPanel=Opt.CommonPanel;
  CurrentCommonPanel=0;
  memset(CommonPanels, 0, sizeof(CommonPanels));
  CommonPanels[0].Items=(PluginPanelItem*)calloc(1,sizeof(PluginPanelItem));
  Opt.LastSearchResultsPanel = 0;
}


HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,INT_PTR Item)
{
  HANDLE hPlugin=INVALID_HANDLE_VALUE;

  GetOptions();

  StartupOpenFrom=OpenFrom;
  if (OpenFrom==OPEN_COMMANDLINE)
  {
    TCHAR *argv=(TCHAR*)Item;
#ifndef UNICODE
    int ANSI = FALSE;
    #define OPT_COUNT 6
#else
    #define OPT_COUNT 5
#endif
    static const TCHAR ParamsStr[OPT_COUNT][8]=
    {
#ifndef UNICODE
      _T("ansi"),
#endif
      _T("safe"),_T("any"),_T("replace"),_T("menu"),_T("full")};
    const int *ParamsOpt[OPT_COUNT]=
    {
#ifndef UNICODE
      &ANSI,
#endif
      &Opt.SafeModePanel,&Opt.AnyInPanel,&Opt.Mode,
      &Opt.MenuForFilelist,&Opt.FullScreenPanel
    };

    while (argv && *argv==_T(' '))
      argv++;

    while (lstrlen(argv)>1 && (*argv==_T('+') || *argv==_T('-')))
    {
      int k=0;
      while (argv && *argv!=_T(' ') && *argv!=_T('<'))
      {
        k++;
        argv++;
      }

      StrBuf tmpTMP(k+1);
      TCHAR *TMP=tmpTMP;
      lstrcpyn(TMP,argv-k,k+1);

      for (int i=0;i<OPT_COUNT;i++)
        if (lstrcmpi(TMP+1,ParamsStr[i])==0)
          *(int*)ParamsOpt[i] = *TMP==_T('+');

      if (*(TMP+1)>=_T('0') && *(TMP+1)<=_T('9'))
        CurrentCommonPanel=*(TMP+1)-_T('0');

      while(argv && *argv==_T(' '))
        argv++;
    }

    FSF.Trim(argv);
    if (lstrlen(argv))
    {
      if (*argv==_T('<'))
      {
        argv++;
        hPlugin = OpenPanelFromOutput (argv WITH_ANSI_ARG);
        if (Opt.MenuForFilelist)
          return INVALID_HANDLE_VALUE;
      }
      else
      {
        FSF.Unquote(argv);
        StrBuf TmpIn;
        ExpandEnvStrs(argv,TmpIn);
        StrBuf TmpOut;
        if (FindListFile(TmpIn,TmpOut))
        {
          if (Opt.MenuForFilelist)
          {
            ShowMenuFromList(TmpOut);
            return INVALID_HANDLE_VALUE;
          }
          else
          {
            hPlugin=new TmpPanel();
            if (hPlugin==NULL)
              return INVALID_HANDLE_VALUE;

            ProcessList(hPlugin, TmpOut, Opt.Mode WITH_ANSI_ARG);
          }
        }
        else
        {
          return INVALID_HANDLE_VALUE;
        }
      }
    }
  }

  if (hPlugin==INVALID_HANDLE_VALUE)
  {
    hPlugin=new TmpPanel();
    if (hPlugin==NULL)
      return INVALID_HANDLE_VALUE;
  }

  return hPlugin;
}

static HANDLE OpenPanelFromOutput (TCHAR *argv WITH_ANSI_PARAM)
{
  TCHAR *tempDir=ParseParam(argv);

  BOOL allOK=FALSE;

  StrBuf tempfilename(NT_MAX_PATH); //BUGBUG
  StrBuf cmdparams(NT_MAX_PATH); //BUGBUG
  StrBuf fullcmd;

  FSF.MkTemp(tempfilename,
#ifdef UNICODE
             tempfilename.Size(),
#endif
             _T("FARTMP"));

  lstrcpy(cmdparams,_T("%COMSPEC% /c "));
  lstrcat(cmdparams,argv);
  ExpandEnvStrs(cmdparams,fullcmd);

  SECURITY_ATTRIBUTES sa;
  memset(&sa, 0, sizeof(sa));
  sa.nLength=sizeof(sa);
  sa.bInheritHandle=TRUE;

  HANDLE FileHandle;
  FileHandle=CreateFile(tempfilename,GENERIC_WRITE,FILE_SHARE_READ,
    &sa,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

  if (FileHandle!=INVALID_HANDLE_VALUE)
  {
    STARTUPINFO si;
    memset(&si,0,sizeof(si));
    si.cb=sizeof(si);
    si.dwFlags=STARTF_USESTDHANDLES;
    si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput=FileHandle;
    si.hStdError=FileHandle;

    PROCESS_INFORMATION pi;
    memset(&pi,0,sizeof(pi));

    StrBuf workDir(1); //make empty string just in case

    if (tempDir)
    {
      workDir.Reset(NT_MAX_PATH);  //BUGBUG
      ExpandEnvStrs(tempDir,workDir);
    }
    else
    {
#ifdef UNICODE
      DWORD Size=FSF.GetCurrentDirectory(0,NULL);
      if (Size)
      {
        workDir.Reset(Size);
        FSF.GetCurrentDirectory(Size,workDir);
      }
#else
      workDir.Reset(MAX_PATH);
      GetCurrentDirectory(workDir.Size(),workDir);
#endif
    }

    TCHAR consoleTitle[255];
    DWORD tlen = GetConsoleTitle(consoleTitle, ArraySize(consoleTitle));
    SetConsoleTitle(argv);

    BOOL Created=CreateProcess(NULL,fullcmd,NULL,NULL,TRUE,0,NULL,workDir,&si,&pi);

    if (Created)
    {
      WaitForSingleObject(pi.hProcess,INFINITE);
      CloseHandle(pi.hThread);
      CloseHandle(pi.hProcess);
      allOK=TRUE;
    }

    CloseHandle(FileHandle);

    if (tlen)
      SetConsoleTitle(consoleTitle);
  }

  HANDLE hPlugin = INVALID_HANDLE_VALUE;

  if (allOK)
  {
    if (Opt.MenuForFilelist)
    {
      ShowMenuFromList(tempfilename);
    }
    else
    {
      hPlugin=new TmpPanel();
      if (hPlugin==NULL)
        return INVALID_HANDLE_VALUE;
      ProcessList(hPlugin, tempfilename, Opt.Mode WITH_ANSI_ARG);
    }
  }

  DeleteFile(tempfilename);
  return hPlugin;
}

void ReadFileLines(HANDLE hFileMapping, DWORD FileSizeLow, TCHAR **argv, TCHAR *args,
                   UINT *numargs, UINT *numchars WITH_ANSI_PARAM)
{
  *numchars = 0;
  *numargs = 0;

  char *FileData=(char *)MapViewOfFile(hFileMapping,FILE_MAP_READ,0,0,FileSizeLow);
  if (!FileData)
    return;

  StrBuf TMP(NT_MAX_PATH); //BUGBUG
  DWORD Len,Pos=0,Size=FileSizeLow;

#ifdef UNICODE
  bool  ucs2 = false;
  if (Size >= 2) switch(*(LPWORD)FileData) {
    case (BOM_UTF8 & 0xFFFF):
      if (Size >= 3 && ((LPBYTE)FileData)[2] == (BYTE)(BOM_UTF8>>16)) {
    case BOM_UCS2_BE:
        goto done;
      }
    default:
      break;
    case BOM_UCS2:
      ucs2 = true;
      Pos += 2;
      break;
  }
#endif

  while (Pos<Size)
  {
#ifdef UNICODE
    if (!ucs2) {
#endif
      char c;
      while (Pos<Size && ((c = FileData[Pos]) == '\r' || c == '\n'))
        Pos++;
      DWORD Off = Pos;
      while (Pos<Size && (c = FileData[Pos]) != '\r' && c != '\n')
        Pos++;
      Len = Pos - Off;
#ifndef UNICODE
      if (Len >= (DWORD)TMP.Size())
        Len = TMP.Size()-1;
      memcpy(TMP.Ptr(), &FileData[Off], Len);
#else // UNICODE
      Len = MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED|MB_ERR_INVALID_CHARS,
                                &FileData[Off], Len, TMP, TMP.Size()-1);
    } else {
      wchar_t c;
      --Size;
      while (Pos<Size && ((c = *(wchar_t*)&FileData[Pos]) == L'\r' || c == L'\n'))
        Pos += sizeof(wchar_t);
      DWORD Off = Pos;
      while (Pos<Size && (c = *(wchar_t*)&FileData[Pos]) != L'\r' && c != L'\n')
        Pos += sizeof(wchar_t);
      if (Pos < Size)
        ++Size;
      Len = (Pos-Off)/sizeof(wchar_t);
      if (Len >= (DWORD)TMP.Size())
        Len = TMP.Size()-1;
      memcpy(TMP.Ptr(), &FileData[Off], Len*sizeof(wchar_t));
    }
#endif
    if (!Len)
      continue;

    TMP.Ptr()[Len]=0;

    FSF.Trim(TMP);
    FSF.Unquote(TMP);

#ifndef UNICODE
    if(ANSI)
      CharToOem(TMP,TMP);
#endif

    Len = lstrlen(TMP);
    if (!Len)
      continue;

    if (argv)
      *argv++ = args;

    if (args)
    {
      lstrcpy(args,TMP);
      args+=Len+1;
    }

    (*numchars)+=Len+1;
    ++*numargs;
  }
#ifdef UNICODE
done:
#endif
  UnmapViewOfFile((LPVOID)FileData);
}

static void ReadFileList (TCHAR *filename, int *argc, TCHAR ***argv WITH_ANSI_PARAM)
{
  *argc = 0;
  *argv = NULL;

#ifdef UNICODE
  StrBuf FullPath;
  GetFullPath(filename, FullPath);
  StrBuf NtPath;
  FormNtPath(FullPath, NtPath);
#else
  const char* NtPath = filename;
#endif

  HANDLE hFile=CreateFile(NtPath,GENERIC_READ,FILE_SHARE_READ,NULL,
    OPEN_EXISTING,0,NULL);

  if(hFile != INVALID_HANDLE_VALUE)
  {
    DWORD FileSizeLow=GetFileSize(hFile,NULL);
    HANDLE hFileMapping=CreateFileMapping(hFile,NULL,PAGE_READONLY,0,0,NULL);
    CloseHandle(hFile);

    if(hFileMapping != NULL)
    {
      UINT i;
      ReadFileLines(hFileMapping, FileSizeLow, NULL, NULL, (UINT*)argc, &i WITH_ANSI_ARG);
      *argv = (TCHAR**)malloc(*argc*sizeof(TCHAR*) + i*sizeof(TCHAR));
      ReadFileLines(hFileMapping, FileSizeLow, *argv, (TCHAR*)&(*argv)[*argc],
                    (UINT*)argc, &i WITH_ANSI_ARG);
      CloseHandle(hFileMapping);
    }
  }
}

static void ProcessList(HANDLE hPlugin, TCHAR *Name, int Mode WITH_ANSI_PARAM)
{
  if(Mode)
  {
    FreePanelItems(CommonPanels[CurrentCommonPanel].Items,
                   CommonPanels[CurrentCommonPanel].ItemsNumber);
    CommonPanels[CurrentCommonPanel].Items=(PluginPanelItem*)calloc(1,sizeof(PluginPanelItem));
    CommonPanels[CurrentCommonPanel].ItemsNumber=0;
  }
  TmpPanel *Panel=(TmpPanel*)hPlugin;

  int argc;
  TCHAR **argv;
  ReadFileList (Name, &argc, &argv WITH_ANSI_ARG);

  HANDLE hScreen = Panel->BeginPutFiles();

  for(UINT i=0;(int)i<argc;i++)
    Panel->PutOneFile(argv[i]);

  Panel->CommitPutFiles (hScreen, TRUE);
  if (argv)
    free(argv);
}


void ShowMenuFromList(TCHAR *Name)
{
  int argc;
  TCHAR **argv;
#ifndef UNICODE
  static const int ANSI = FALSE;
#endif
  ReadFileList (Name, &argc, &argv WITH_ANSI_ARG);

  FarMenuItem *fmi=(FarMenuItem*)malloc(argc*sizeof(FarMenuItem));
  if (fmi)
  {
    StrBuf TMP(NT_MAX_PATH); //BUGBUG
    for (int i=0;i<argc;++i)
    {
      TCHAR *param,*p=TMP;
      ExpandEnvStrs(argv[i],TMP);
      param=ParseParam(p);
      FSF.TruncStr(param?param:p,67);

#ifndef UNICODE
      strncpy(fmi[i].Text,param?param:p,sizeof(fmi[i].Text)-1);
#else
      fmi[i].Text = wcsdup(param?param:p);
#endif

      fmi[i].Separator=!lstrcmp(param,_T("-"));
    }
//    fmi[0].Selected=TRUE;

    TCHAR Title[128]; //BUGBUG
    FSF.ProcessName(FSF.PointToName(Name),lstrcpy(Title,_T("*.")),
#ifdef UNICODE
                    ArraySize(Title),
#endif
                    PN_GENERATENAME);
    FSF.TruncPathStr(Title,64);

    int BreakCode;
    static const int BreakKeys[2]={MAKELONG(VK_RETURN,PKF_SHIFT),0};

    int ExitCode=Info.Menu(Info.ModuleNumber, -1, -1, 0,
      FMENU_WRAPMODE, Title, NULL, _T("Contents"),
      &BreakKeys[0], &BreakCode, fmi, argc);

#ifdef UNICODE
    for (int i=0;i<argc;++i)
      if (fmi[i].Text)
        free((void*)fmi[i].Text);
#endif

    free(fmi);

    if ((unsigned)ExitCode<(unsigned)argc)
    {
      ExpandEnvStrs(argv[ExitCode],TMP);
      TCHAR *p=TMP;
      ParseParam(p);

      FAR_FIND_DATA FindData;
      int bShellExecute=BreakCode!=-1;

      if (!bShellExecute)
      {
        if (TmpPanel::GetFileInfoAndValidate(p,&FindData,FALSE))
        {
          if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
          {
#ifndef UNICODE
            Info.Control(INVALID_HANDLE_VALUE,FCTL_SETPANELDIR,p);
#else
            Info.Control(INVALID_HANDLE_VALUE,FCTL_SETPANELDIR,0,(LONG_PTR)p);
#endif
          }
          else
          {
            bShellExecute=TRUE;
          }
        }
        else
        {
#ifndef UNICODE
          Info.Control(INVALID_HANDLE_VALUE,FCTL_SETCMDLINE,p);
#else
          Info.Control(PANEL_ACTIVE,FCTL_SETCMDLINE,0,(LONG_PTR)p);
#endif
        }
      }

      if (bShellExecute)
        ShellExecute(NULL,_T("open"),p,NULL,NULL,SW_SHOW);
    }
  }

  if (argv)
    free(argv);
}


#ifndef UNICODE
#define _CONST
#define _OPARG
#else
#define _CONST const
#define _OPARG ,int
#endif
HANDLE WINAPI EXP_NAME(OpenFilePlugin)(_CONST TCHAR *Name,const unsigned char *,int DataSize _OPARG)
#undef _OPARG
#undef _CONST
{
#ifndef UNICODE
#define PNAME_ARG Name
#define pName     Name
#else
  StrBuf pName(NT_MAX_PATH); //BUGBUG
  lstrcpy(pName, Name);
#define PNAME_ARG pName, pName.Size()
#endif

  if (!DataSize || !FSF.ProcessName(Opt.Mask,PNAME_ARG,PN_CMPNAMELIST))
    return INVALID_HANDLE_VALUE;
#undef PNAME_ARG

  if (!Opt.MenuForFilelist)
  {
    HANDLE hPlugin=new TmpPanel();
    if (hPlugin == NULL)
      return INVALID_HANDLE_VALUE;
#ifndef UNICODE
    static const int ANSI = FALSE;
#endif
    ProcessList(hPlugin, pName, Opt.Mode WITH_ANSI_ARG);
    return hPlugin;
  }
  else
  {
    ShowMenuFromList(pName);
    return INVALID_HANDLE_VALUE;
  }
#undef pName
}

void WINAPI EXP_NAME(ClosePlugin)(HANDLE hPlugin)
{
  delete (TmpPanel *)hPlugin;
}


void WINAPI EXP_NAME(ExitFAR)()
{
  free(PluginRootKey);
  for (int i=0;i<COMMONPANELSNUMBER;++i)
    FreePanelItems(CommonPanels[i].Items, CommonPanels[i].ItemsNumber);
}


int WINAPI EXP_NAME(GetFindData)(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return Panel->GetFindData(pPanelItem,pItemsNumber,OpMode);
}


void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_PRELOAD;
  static const TCHAR *DiskMenuStrings[1];
  DiskMenuStrings[0]=GetMsg(MDiskMenuString);
  Info->DiskMenuStrings=DiskMenuStrings;
#ifndef UNICODE
  static int DiskMenuNumbers[1];
  DiskMenuNumbers[0]=FSF.atoi(Opt.DisksMenuDigit);
  Info->DiskMenuNumbers=DiskMenuNumbers;
#endif
  Info->DiskMenuStringsNumber=Opt.AddToDisksMenu?ArraySize(DiskMenuStrings):0;
  static const TCHAR *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MTempPanel);
  Info->PluginMenuStrings=Opt.AddToPluginsMenu?PluginMenuStrings:NULL;
  Info->PluginMenuStringsNumber=Opt.AddToPluginsMenu?ArraySize(PluginMenuStrings):0;
  static const TCHAR *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MTempPanel);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=ArraySize(PluginCfgStrings);
  Info->CommandPrefix=Opt.Prefix;
}


void WINAPI EXP_NAME(GetOpenPluginInfo)(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  Panel->GetOpenPluginInfo(Info);
}


int WINAPI EXP_NAME(SetDirectory)(HANDLE hPlugin,const TCHAR *Dir,int OpMode)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->SetDirectory(Dir,OpMode));
}


int WINAPI EXP_NAME(PutFiles)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                              int ItemsNumber,int Move,
#ifdef UNICODE
                              const wchar_t* SrcPath,
#endif
                              int OpMode)
{
#ifndef UNICODE
  StrBuf SrcPath(MAX_PATH);
  GetCurrentDirectory(SrcPath.Size(), SrcPath);
#endif
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return Panel->PutFiles(PanelItem,ItemsNumber,Move,SrcPath,OpMode);
}


int WINAPI EXP_NAME(SetFindList)(HANDLE hPlugin,const struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->SetFindList(PanelItem,ItemsNumber));
}


int WINAPI EXP_NAME(ProcessEvent)(HANDLE hPlugin,int Event,void *Param)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->ProcessEvent(Event,Param));
}


int WINAPI EXP_NAME(ProcessKey)(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->ProcessKey(Key,ControlState));
}


int WINAPI EXP_NAME(Configure)(int ItemNumber)
{
  return (!ItemNumber?Config():FALSE);
}

int WINAPI EXP_NAME(GetMinFarVersion)()
{
  return FARMANAGERVERSION;
}
