static BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam);
static HWND ProcWnd;

static BOOL InitToolhelp32();

typedef struct tagPINFO
{
  char  szModName[256];
  DWORD pid;
  DWORD dwPriorityClass;
  LONG  pcPriClassBase;
  DWORD cntThreads;
  UINT  uAppType;
  char  szFullPath[_MAX_PATH];
} PINFO, *PPINFO;

typedef struct tagTINFO
{
  DWORD tid;
  DWORD pidOwner;
  LONG  tpBasePri;
  LONG  tpDeltaPri;
} TINFO;


typedef BOOL (WINAPI *MODULEWALK)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
typedef BOOL (WINAPI *THREADWALK)(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
typedef BOOL (WINAPI *PROCESSWALK)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef HANDLE (WINAPI *CREATESNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);

static CREATESNAPSHOT pCreateToolhelp32Snapshot = NULL;
static MODULEWALK  pModule32First  = NULL;
static MODULEWALK  pModule32Next   = NULL;
static PROCESSWALK pProcess32First = NULL;
static PROCESSWALK pProcess32Next  = NULL;
static THREADWALK  pThread32First  = NULL;
static THREADWALK  pThread32Next   = NULL;

BOOL GetProcessModule (DWORD dwPID, DWORD dwModuleID, LPMODULEENTRY32 lpMe32,
                       DWORD cbMe32);
BOOL GetModuleNameFromExe (LPCSTR szFileName, LPSTR szModuleName, WORD cbLen);


int GetList95(PluginPanelItem **pPanelItem,int *pItemsNumber)
{
  if (pModule32First==NULL && !InitToolhelp32())
    return(FALSE);

  PROCESSENTRY32 pe32={0};
  HANDLE hProcessSnap;
  BOOL bRet;
  *pItemsNumber=0;

  hProcessSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
  if (hProcessSnap==(HANDLE)-1)
    return (FALSE);

  pe32.dwSize = sizeof(PROCESSENTRY32);

  if (pProcess32First(hProcessSnap, &pe32))
  {
    MODULEENTRY32 me32={0};
    PINFO pi={{0},0};
    BOOL bGotModule;
    do
    {
      bGotModule = GetProcessModule(pe32.th32ProcessID,pe32.th32ModuleID,
                                    &me32,sizeof(MODULEENTRY32));
      if (bGotModule)
      {
        pi.pcPriClassBase=pe32.pcPriClassBase;
        pi.pid           =pe32.th32ProcessID;
        pi.cntThreads    =pe32.cntThreads;
        strcpy(pi.szFullPath,pe32.szExeFile);
        if (!LocalStricmp(pe32.szExeFile,me32.szExePath))
        {
          strcpy(pi.szModName,me32.szModule);
          pi.uAppType=32;
        }
        else
        {
          if (!GetModuleNameFromExe(pe32.szExeFile,pi.szModName,sizeof(pi.szModName)))
            strcpy(pi.szModName,me32.szModule);
          pi.uAppType=16;
        }
        char Name[NM],Title[NM];
        strcpy(Name,pi.szModName);
        *Title=0;
        ProcWnd=NULL;
        EnumWindows(EnumWindowsProc,pi.pid);
        if (ProcWnd!=NULL)
          GetWindowText(ProcWnd,Title,sizeof(Title));
        int CurItemPos=*pItemsNumber;
        PluginPanelItem *NewPanelItem=(PluginPanelItem *)realloc(*pPanelItem,(CurItemPos+1)*sizeof(PluginPanelItem));
        if (NewPanelItem==NULL)
        {
          CloseHandle (hProcessSnap);
          return(FALSE);
        }
        *pPanelItem=NewPanelItem;
        PluginPanelItem *CurItem=&NewPanelItem[CurItemPos];
        memset(CurItem,0,sizeof(PluginPanelItem));
        CurItem->Flags|=PPIF_USERDATA;
        strcpy(CurItem->FindData.cFileName,Name);
        CurItem->FindData.nFileSizeLow=pe32.pcPriClassBase;
        CurItem->PackSize=pe32.th32ProcessID;
        CurItem->Description=new char[strlen(Title)+1];
        CharToOem(Title,Title);
        strcpy(CurItem->Description,Title);
        CurItem->UserData=(DWORD)(new ProcessData);
        ProcessData pdata;
        pdata.Size=sizeof(pdata);
        pdata.hwnd=ProcWnd;
        pdata.uAppType=pi.uAppType;
        pdata.Threads=pi.cntThreads;
        strcpy(pdata.FullPath,pi.szFullPath);
        memcpy((void *)CurItem->UserData,&pdata,sizeof(pdata));
        (*pItemsNumber)++;
      }
    } while (pProcess32Next(hProcessSnap,&pe32));
    bRet = TRUE;
  }
  else
    bRet = FALSE;

  CloseHandle (hProcessSnap);
  return (bRet);
}


BOOL InitToolhelp32()
{
  BOOL   bRet;
  HMODULE hKernel;

  hKernel = GetModuleHandle("KERNEL32.DLL");

  if (hKernel)
  {
    pCreateToolhelp32Snapshot =
        (CREATESNAPSHOT)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");

    pModule32First  = (MODULEWALK)GetProcAddress(hKernel,"Module32First");
    pModule32Next   = (MODULEWALK)GetProcAddress(hKernel,"Module32Next");
    pProcess32First = (PROCESSWALK)GetProcAddress(hKernel,"Process32First");
    pProcess32Next  = (PROCESSWALK)GetProcAddress(hKernel,"Process32Next");
    pThread32First  = (THREADWALK)GetProcAddress(hKernel,"Thread32First");
    pThread32Next   = (THREADWALK)GetProcAddress(hKernel,"Thread32Next");

    bRet =  pModule32First && pModule32Next  && pProcess32First &&
            pProcess32Next && pThread32First && pThread32Next &&
            pCreateToolhelp32Snapshot;
  }
  else
    bRet = FALSE;  // Couldn't even get a module handle to KERNEL.

  return bRet;
}



BOOL GetProcessModule(DWORD dwPID,DWORD dwModuleID,LPMODULEENTRY32 lpMe32,
                      DWORD cbMe32)
{
  MODULEENTRY32 me32={0};
  HANDLE hModuleSnap;
  BOOL bRet,bFound=FALSE;

  hModuleSnap=pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
  if (hModuleSnap==(HANDLE)-1)
    return (FALSE);
  me32.dwSize = sizeof(MODULEENTRY32);
  if (pModule32First(hModuleSnap, &me32))
  {
    do
    {
      if (me32.th32ModuleID==dwModuleID)
      {
        memcpy(lpMe32,&me32,cbMe32);
        bFound = TRUE;
      }
    } while (!bFound && pModule32Next(hModuleSnap, &me32));
    bRet = bFound;
  }
  else
    bRet = FALSE;
  CloseHandle (hModuleSnap);
  return (bRet);
}


BOOL KillProcess(DWORD pid)
{
  HANDLE hProcess;
  BOOL bRet;
  hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,pid);
  if (hProcess!=NULL)
  {
    bRet=TerminateProcess(hProcess,0xFFFFFFFF);
    if (bRet)
      WaitForSingleObject(hProcess,5000);
    CloseHandle(hProcess);
  }
  else
    bRet=FALSE;
  return(bRet);
}


BOOL GetModuleNameFromExe(LPCSTR szFileName,LPSTR szModuleName,WORD cbLen)
{
  PIMAGE_OS2_HEADER pNEHdr;
  PIMAGE_DOS_HEADER pDosExeHdr;
  HANDLE hFile,hFileMapping;
  BOOL bResult;

  hFile = CreateFile(szFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                     NULL,OPEN_EXISTING,0,NULL);

  if (hFile == INVALID_HANDLE_VALUE)
    return FALSE;

  hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
  if (hFileMapping == NULL)
  {
    CloseHandle(hFile);
    return FALSE;
  }

  pDosExeHdr=(PIMAGE_DOS_HEADER)MapViewOfFile(hFileMapping,FILE_MAP_READ,0,0,0);
  if (!pDosExeHdr)
  {
    CloseHandle(hFileMapping);
    CloseHandle(hFile);
    return FALSE;
  }

  pNEHdr=(PIMAGE_OS2_HEADER)((LPSTR)pDosExeHdr + pDosExeHdr -> e_lfanew);
  if (pDosExeHdr -> e_magic == IMAGE_DOS_SIGNATURE
      && pNEHdr -> ne_magic == IMAGE_OS2_SIGNATURE)
  {
    strncpy(szModuleName, (LPSTR)pNEHdr + pNEHdr->ne_restab +1,
            Min((BYTE)*((LPSTR)pNEHdr + pNEHdr -> ne_restab) + 1,cbLen));
    bResult=TRUE;
  }
  else
    bResult=FALSE;

  UnmapViewOfFile(pDosExeHdr);
  CloseHandle(hFileMapping);
  CloseHandle(hFile);

  return bResult;
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam)
{
  DWORD ProcID;
  GetWindowThreadProcessId(hwnd,&ProcID);
  if (ProcID==lParam && GetParent(hwnd)==NULL)
  {
    ProcWnd=hwnd;
    return(FALSE);
  }
  return(TRUE);
}
