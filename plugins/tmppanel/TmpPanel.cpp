/*
TMPPANEL.CPP

Temporary panel main plugin code

*/

#include "TmpPanel.hpp"

char PluginRootKey[80];
char TMPPanelDir[NM*5];
unsigned int CurrentCommonPanel;
struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;
BOOL IsOldFAR;

PluginPanels CommonPanels[COMMONPANELSNUMBER];

#if defined(__GNUC__)
  #define DLLMAINFUNC DllMainCRTStartup
#else
  #define DLLMAINFUNC _DllMainCRTStartup
#endif

#ifdef __cplusplus
extern "C"{
#endif
BOOL WINAPI DLLMAINFUNC(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) lpReserved;
  if (DLL_PROCESS_ATTACH == dwReason && hDll)
  {
    char *pf;
    GetModuleFileName((HINSTANCE)hDll, TMPPanelDir, sizeof(TMPPanelDir));
    if (GetFullPathName(TMPPanelDir, sizeof(TMPPanelDir), TMPPanelDir, &pf))
      *pf = '\0';
  }
  return TRUE;
}
#ifdef __cplusplus
};
#endif

void ProcessList(HANDLE hPlugin, char *Name, int Mode, int ANSI);
void ShowMenuFromList(char *Name);
HANDLE OpenPanelFromOutput (char *argv, int ANSI);

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  IsOldFAR=TRUE;
  if((size_t)Info->StructSize>=sizeof(struct PluginStartupInfo))
  {
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;
    IsOldFAR=FALSE;

    FSF.sprintf(PluginRootKey,"%s\\TmpPanel",Info->RootKey);
    GetOptions();
    StartupOptFullScreenPanel=Opt.FullScreenPanel;
    StartupOptCommonPanel=Opt.CommonPanel;
    CurrentCommonPanel=0;
    memset(CommonPanels, 0, sizeof(CommonPanels));
    CommonPanels[0].Items=(PluginPanelItem*)malloc(sizeof(PluginPanelItem));
    Opt.LastSearchResultsPanel = 0;
  }
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,INT_PTR Item)
{
  if(IsOldFAR) return(INVALID_HANDLE_VALUE);

  HANDLE hPlugin=INVALID_HANDLE_VALUE;

  GetOptions();

  StartupOpenFrom=OpenFrom;
  if(OpenFrom==OPEN_COMMANDLINE)
  {
    char *argv=(char*)Item;
    int ANSI = FALSE;
    #define OPT_COUNT 6
    static const char ParamsStr[OPT_COUNT][8]=
      {"ansi","safe","any","replace","menu","full"};
    const int *ParamsOpt[OPT_COUNT]=
      {&ANSI,&Opt.SafeModePanel,&Opt.AnyInPanel,&Opt.Mode,
       &Opt.MenuForFilelist,&Opt.FullScreenPanel};

    while(argv && *argv==' ') argv++;
    while(lstrlen(argv)>1 && (*argv=='+' || *argv=='-'))
    {
      int k=0;
      while(argv && *argv!=' ')
      {
        k++;
        argv++;
      }
      static char TMP[MAX_PATH];
      lstrcpyn(TMP,argv-k,k+1);

      for(int i=0;i<OPT_COUNT;i++)
        if(lstrcmpi(TMP+1,ParamsStr[i])==0)
          *(int*)ParamsOpt[i] = *TMP=='+';;

      if(*(TMP+1)>='0' && *(TMP+1)<='9')
        CurrentCommonPanel=*(TMP+1)-'0';
      while(argv && *argv==' ') argv++;
    }

    FSF.Trim(argv);
    if(lstrlen(argv))
    {
      if(*argv=='<')
      {
        argv++;
        hPlugin = OpenPanelFromOutput (argv, ANSI);
      }
      else
      {
        FSF.Unquote(argv);
        char TMP[NM*5];
        FSF.ExpandEnvironmentStr(argv,TMP,sizeof(TMP));
        char *p;
        if(SearchPath(TMPPanelDir,TMP,NULL,sizeof(TMP),TMP,&p) || SearchPath(NULL,TMP,NULL,sizeof(TMP),TMP,&p))
        {
          if(Opt.MenuForFilelist)
          {
            ShowMenuFromList(TMP);
            return(INVALID_HANDLE_VALUE);
          }
          else
          {
            hPlugin=new TmpPanel();
            if(hPlugin==NULL) return(INVALID_HANDLE_VALUE);
            ProcessList(hPlugin, TMP,Opt.Mode,ANSI);
          }
        }
      }
    }
  }
  if(hPlugin==INVALID_HANDLE_VALUE)
  {
    hPlugin=new TmpPanel();
    if(hPlugin==NULL) return(INVALID_HANDLE_VALUE);
  }
  return(hPlugin);
}

HANDLE OpenPanelFromOutput (char *argv, int ANSI)
{
  char *tempDir=ParseParam(argv);

  BOOL allOK=FALSE;
  static char tempfilename[NM*5],cmdparams[NM*5],fullcmd[NM*5];
  FSF.MkTemp(tempfilename, "FARTMP");
  lstrcpy(cmdparams,"%COMSPEC% /c ");
  lstrcat(cmdparams,argv);
  FSF.ExpandEnvironmentStr(cmdparams,fullcmd,sizeof(fullcmd));

  static SECURITY_ATTRIBUTES sa;
  memset(&sa, 0, sizeof(sa));
  sa.nLength=sizeof(sa);
  sa.bInheritHandle=TRUE;

  HANDLE FileHandle;
  FileHandle=CreateFile(tempfilename,GENERIC_WRITE,FILE_SHARE_READ,
    &sa,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

  if(FileHandle!=INVALID_HANDLE_VALUE)
  {
    static STARTUPINFO si;
    memset(&si,0,sizeof(si));
    si.cb=sizeof(si);
    si.dwFlags=STARTF_USESTDHANDLES;
    si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput=FileHandle;
    si.hStdError=FileHandle;

    static PROCESS_INFORMATION pi;
    memset(&pi,0,sizeof(pi));

    static char SaveDir[NM],workDir[NM];

    if(tempDir)
    {
      GetCurrentDirectory(sizeof(SaveDir),SaveDir);
      FSF.ExpandEnvironmentStr(tempDir,workDir,sizeof(workDir));
      SetCurrentDirectory(workDir);
    }

    static char consoleTitle[255];
    DWORD tlen = GetConsoleTitle(consoleTitle, 255);
    SetConsoleTitle(argv);

    if(CreateProcess(NULL,fullcmd,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
    {
      WaitForSingleObject(pi.hProcess,INFINITE);
      CloseHandle(pi.hThread);
      CloseHandle(pi.hProcess);
      allOK=TRUE;
    }
    CloseHandle(FileHandle);

    if(tlen) SetConsoleTitle(consoleTitle);
    if(tempDir) SetCurrentDirectory(SaveDir);
  }
  HANDLE hPlugin = INVALID_HANDLE_VALUE;
  if(allOK)
  {
    hPlugin=new TmpPanel();
    if(hPlugin==NULL)
      return(INVALID_HANDLE_VALUE);
    ProcessList(hPlugin,tempfilename,Opt.Mode,ANSI);
  }
  DeleteFile(tempfilename);
  return hPlugin;
}

void ReadFileLines(HANDLE hFileMapping, DWORD FileSizeLow, char **argv, char *args,
                   UINT *numargs, UINT *numchars, int ANSI)
{
  *numchars = 0;
  *numargs = 0;

  char *FileData=(char *)MapViewOfFile(hFileMapping,FILE_MAP_READ,0,0,FileSizeLow);

  if(FileData!=NULL)
  {
    static char TMP[MAX_PATH];
    int Len,Pos=0,Size=FileSizeLow;

    while(Pos<Size)
    {
      while(Pos<Size && (FileData[Pos]=='\r' || FileData[Pos]=='\n')) Pos++;
      Len=0;
      while(Pos<Size && FileData[Pos]!='\r' && FileData[Pos]!='\n')
      {
        Len++;
        Pos++;
      }

      Len=(Len<MAX_PATH)?Len:(MAX_PATH-1);
      memcpy(&TMP,FileData+Pos-Len,Len);
      TMP[Len]=0;
      FSF.Trim(TMP);
      FSF.Unquote(TMP);

      if(ANSI) CharToOem(TMP,TMP);

      if(argv) *argv++ = args;
      if(args)
      {
        lstrcpy(args,&TMP[0]);
        args+=Len+1;
      }

      (*numchars)+=Len+1;
      ++*numargs;
    }
    UnmapViewOfFile((LPVOID)FileData);
  }
}

void ReadFileList (char *filename, int *argc, char ***argv, int ANSI)
{
  *argc = 0;
  *argv = NULL;

  HANDLE hFile=CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,
    OPEN_EXISTING,0,NULL);

  if(hFile != INVALID_HANDLE_VALUE)
  {
    DWORD FileSizeLow=GetFileSize(hFile,NULL);
    HANDLE hFileMapping=CreateFileMapping(hFile,NULL,PAGE_READONLY,0,0,NULL);
    CloseHandle(hFile);

    if(hFileMapping != NULL)
    {
      UINT i;
      ReadFileLines (hFileMapping, FileSizeLow, NULL,NULL,(UINT *) argc,&i,ANSI);
      *argv = (char**)malloc(*argc * sizeof(char*) + i * sizeof(char) + 1);
      ReadFileLines (hFileMapping, FileSizeLow,
          *argv, ((char*)*argv) + sizeof(char*) * *argc,(UINT*)argc, &i,ANSI);
      CloseHandle(hFileMapping);
    }
  }
}

void ProcessList(HANDLE hPlugin, char *Name, int Mode, int ANSI)
{
  if(Mode)
  {
    FreePanelItems(CommonPanels[CurrentCommonPanel].Items,
                   CommonPanels[CurrentCommonPanel].ItemsNumber);
    CommonPanels[CurrentCommonPanel].Items=(PluginPanelItem*)malloc(sizeof(PluginPanelItem));
    CommonPanels[CurrentCommonPanel].ItemsNumber=0;
  }
  TmpPanel *Panel=(TmpPanel*)hPlugin;

  int argc;
  char **argv;
  ReadFileList (Name, &argc, &argv, ANSI);

  HANDLE hScreen = Panel->BeginPutFiles();

  static struct PluginPanelItem ppi;
  memset(&ppi,0,sizeof(ppi));
  for(UINT i=0;(int)i<argc;i++)
    if(Panel->CheckForCorrect(argv[i],&ppi.FindData,Opt.AnyInPanel))
      Panel->PutOneFile(ppi);

  Panel->CommitPutFiles (hScreen, TRUE);
  if (argv)
    free(argv);
}


void ShowMenuFromList(char *Name)
{
  int argc;
  char **argv;
  ReadFileList (Name, &argc, &argv, FALSE);

  FarMenuItem *fmi=(FarMenuItem*)malloc(argc*sizeof(FarMenuItem));
  if(fmi)
  {
    static char TMP[MAX_PATH];
    for(int i=0;i<argc;++i)
    {
      char *param,*p=TMP;
      FSF.ExpandEnvironmentStr(argv[i],TMP,sizeof(TMP));
      param=ParseParam(p);
      FSF.TruncStr(param?param:p,67);
      lstrcpy(fmi[i].Text,param?param:p);
      fmi[i].Separator=!lstrcmp(param,"-");
    }
//    fmi[0].Selected=TRUE;

    static char Title[MAX_PATH];
    FSF.ProcessName(FSF.PointToName(Name),lstrcpy(Title,"*."),PN_GENERATENAME);
    FSF.TruncPathStr(Title,64);

    int BreakCode;
    static const int BreakKeys[2]={MAKELONG(VK_RETURN,PKF_SHIFT),0};
    int ExitCode=Info.Menu(Info.ModuleNumber, -1, -1, 0,
      FMENU_WRAPMODE, Title, NULL, "Contents",
      &BreakKeys[0], &BreakCode, fmi, argc);
    free(fmi);
    if(ExitCode>-1 && ExitCode<argc)
    {
      FSF.ExpandEnvironmentStr(argv[ExitCode],TMP,sizeof(TMP));
      char *p=TMP;
      ParseParam(p);

      FAR_FIND_DATA FindData;
      int bShellExecute=BreakCode!=-1;

      if(!bShellExecute)
        if(TmpPanel::CheckForCorrect(p,&FindData,FALSE))
          if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            Info.Control(INVALID_HANDLE_VALUE,FCTL_SETPANELDIR,p);
          else
            bShellExecute=TRUE;
        else
          Info.Control(INVALID_HANDLE_VALUE,FCTL_SETCMDLINE,p);
      if(bShellExecute)
        ShellExecute(NULL,"open",p,NULL,NULL,SW_SHOW);
    }
  }
  if (argv)
    free(argv);
}


HANDLE WINAPI _export OpenFilePlugin(char *Name,const unsigned char *,int DataSize)
{
  if(IsOldFAR || !DataSize || !FSF.ProcessName(Opt.Mask,Name,PN_CMPNAMELIST))
    return(INVALID_HANDLE_VALUE);

  if(!Opt.MenuForFilelist)
  {
    HANDLE hPlugin=new TmpPanel();
    if(hPlugin == NULL) return(INVALID_HANDLE_VALUE);
    ProcessList(hPlugin, Name, Opt.Mode, FALSE);
    return(hPlugin);
  }
  else
  {
    ShowMenuFromList(Name);
    return(INVALID_HANDLE_VALUE);
  }
}

void WINAPI _export ClosePlugin(HANDLE hPlugin)
{
  delete(TmpPanel *)hPlugin;
}


void WINAPI _export ExitFAR()
{
  for(int i=0;i<COMMONPANELSNUMBER;++i)
    FreePanelItems(CommonPanels[i].Items, CommonPanels[i].ItemsNumber);
}


int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->GetFindData(pPanelItem,pItemsNumber,OpMode));
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_PRELOAD;
  static const char *DiskMenuStrings[1];
  DiskMenuStrings[0]=GetMsg(MDiskMenuString);
  Info->DiskMenuStrings=DiskMenuStrings;
  static int DiskMenuNumbers[1];
  DiskMenuNumbers[0]=FSF.atoi(Opt.DisksMenuDigit);
  Info->DiskMenuNumbers=DiskMenuNumbers;
  Info->DiskMenuStringsNumber=Opt.AddToDisksMenu?1:0;
  static const char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MTempPanel);
  Info->PluginMenuStrings=Opt.AddToPluginsMenu?PluginMenuStrings:NULL;
  Info->PluginMenuStringsNumber=Opt.AddToPluginsMenu?sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]):0;
  static const char *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MTempPanel);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=sizeof(PluginCfgStrings)/sizeof(PluginCfgStrings[0]);
  Info->CommandPrefix=Opt.Prefix;
}


void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  Panel->GetOpenPluginInfo(Info);
}


int WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->SetDirectory(Dir,OpMode));
}


int WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,
                            int ItemsNumber,int Move,int OpMode)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->PutFiles(PanelItem,ItemsNumber,Move,OpMode));
}


int WINAPI _export SetFindList(HANDLE hPlugin,const struct PluginPanelItem *PanelItem,int ItemsNumber)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->SetFindList(PanelItem,ItemsNumber));
}


int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->ProcessEvent(Event,Param));
}


int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  TmpPanel *Panel=(TmpPanel *)hPlugin;
  return(Panel->ProcessKey(Key,ControlState));
}


int WINAPI _export Configure(int ItemNumber)
{
  return(!IsOldFAR && !ItemNumber?Config():FALSE);
}

int WINAPI _export GetMinFarVersion(void)
{
  return(MAKEFARVERSION(1,70,1024));
}
