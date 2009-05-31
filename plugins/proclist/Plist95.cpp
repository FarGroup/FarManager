#include "proclist.hpp"
#include "proclng.hpp"
#include <tlhelp32.h>

static BOOL InitToolhelp32();

typedef BOOL (WINAPI *MODULEWALK)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
typedef BOOL (WINAPI *PROCESSWALK)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef HANDLE (WINAPI *CREATESNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL (WINAPI *HEAPLISTWALK)(HANDLE hSnapshot, LPHEAPLIST32 lphl);
typedef BOOL (WINAPI *HEAPFIRST)(LPHEAPENTRY32 lphe, DWORD th32ProcessID, ULONG_PTR th32HeapID);
typedef BOOL (WINAPI *HEAPNEXT)(LPHEAPENTRY32 lphe);

static CREATESNAPSHOT pCreateToolhelp32Snapshot;
static MODULEWALK  pModule32First, pModule32Next;
static PROCESSWALK pProcess32First, pProcess32Next;
static HEAPLISTWALK pHeap32ListFirst, pHeap32ListNext;
static HEAPFIRST pHeap32First;
static HEAPNEXT pHeap32Next;

BOOL GetProcessModule (DWORD dwPID, DWORD dwModuleID, LPMODULEENTRY32 lpMe32, DWORD& dwTotalSize);
BOOL GetModuleNameFromExe (LPCTSTR szFileName, LPTSTR szModuleName, WORD cbLen);
DWORD GetHeapSize(DWORD dwPID);

void GetPData95(ProcessData& pdata, PROCESSENTRY32& pe32)
{
    pdata.Size = sizeof(pdata);
    pdata.dwPID = pe32.th32ProcessID;
    pdata.dwParentPID = pe32.th32ParentProcessID;
    lstrcpy(pdata.FullPath, pe32.szExeFile);
    pdata.dwPrBase = pe32.pcPriClassBase;
    pdata.dwPID = pe32.th32ProcessID;
}

bool GetPData95(ProcessData& pdata)
{
  if (pModule32First==NULL && !InitToolhelp32())
    return false;

  PROCESSENTRY32 pe32 = {sizeof(pe32)};

  HANDLE hProcessSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
  if (hProcessSnap==INVALID_HANDLE_VALUE)
    return false;

  if (pProcess32First(hProcessSnap, &pe32))
    do {
        if(pdata.dwPID==pe32.th32ProcessID)
        {
            GetPData95(pdata, pe32);
            CloseHandle (hProcessSnap);
            return true;
        }
    } while (pProcess32Next(hProcessSnap,&pe32));
  CloseHandle (hProcessSnap);
  return false;
}

BOOL GetList95(PluginPanelItem*& pPanelItem,int &ItemsNumber)
{
  if (pModule32First==NULL && !InitToolhelp32())
    return FALSE;

  PROCESSENTRY32 pe32 = {sizeof(pe32)};
  BOOL bRet = FALSE;
#ifdef UNICODE
  wchar_t tmpStr[MAX_PATH];
#endif
  ItemsNumber = 0;

  HANDLE hProcessSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
  if (hProcessSnap==INVALID_HANDLE_VALUE)
    return FALSE;

  if (pProcess32First(hProcessSnap, &pe32))
  {
    do {
      DWORD dwTotalSize;
      MODULEENTRY32 me32;
      if (GetProcessModule(pe32.th32ProcessID,pe32.th32ModuleID,&me32,dwTotalSize) )
      {
        int CurItemPos = ItemsNumber;
        PluginPanelItem *pNewPanelItem=(PluginPanelItem *)realloc(pPanelItem,(CurItemPos+1)*sizeof(PluginPanelItem));

        if (pNewPanelItem==NULL)
          break;
        pPanelItem = pNewPanelItem;
        PluginPanelItem &CurItem = pNewPanelItem[CurItemPos];
        memset(&CurItem,0,sizeof(CurItem));
        CurItem.Flags |= PPIF_USERDATA;
        CurItem.UserData = (DWORD_PTR)(new ProcessData);
        ProcessData& pdata = *(ProcessData*)CurItem.UserData;
#ifdef UNICODE
#define PackSize      FindData.nPackSize
#define nFileSizeLow  nFileSize
#endif
        CurItem.PackSize = dwTotalSize;
        CurItem.FindData.nFileSizeLow = dwTotalSize;// + GetHeapSize(pe32.th32ProcessID);
#undef PackSize
#undef nFileSizeLow
        CurItem.NumberOfLinks = pe32.cntThreads;

        if (!FSF.LStricmp(pe32.szExeFile,me32.szExePath))
        {
          pdata.uAppType=32;
get_from_me32:
#ifndef UNICODE
          lstrcpy(CurItem.FindData.cFileName,me32.szModule);
#else
          CurItem.FindData.lpwszFileName = wcsdup(me32.szModule);
#endif
        }
        else
        {
          pdata.uAppType=16;
#ifndef UNICODE
#define GET CurItem.FindData.cFileName
#else
#define GET tmpStr
#endif
          if (!GetModuleNameFromExe(pe32.szExeFile,GET,ArraySize(GET)))
              goto get_from_me32;
#undef GET
#ifdef UNICODE
          CurItem.FindData.lpwszFileName = wcsdup(tmpStr);
#endif
        }
#ifndef UNICODE
        CharToOem(CurItem.FindData.cFileName, CurItem.FindData.cFileName);
#define PRT CurItem.FindData.cAlternateFileName
#else
#define PRT tmpStr
#endif
        FSF.sprintf(PRT, _T("%08X"), pe32.th32ProcessID);
#undef PRT
#ifdef UNICODE
        CurItem.FindData.lpwszAlternateFileName = wcsdup(tmpStr);
#endif
        GetPData95(pdata, pe32);

        ItemsNumber++;
      }
    } while (pProcess32Next(hProcessSnap,&pe32));
    bRet = TRUE;
  }
  CloseHandle (hProcessSnap);
  return bRet;
}


BOOL InitToolhelp32()
{
  BOOL   bRet;
  HMODULE hKernel;

  hKernel = GetModuleHandle(_T("KERNEL32.DLL"));

  if (hKernel)
  {
    pCreateToolhelp32Snapshot =
        (CREATESNAPSHOT)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");

    pModule32First  = (MODULEWALK)GetProcAddress(hKernel,"Module32First");
    pModule32Next   = (MODULEWALK)GetProcAddress(hKernel,"Module32Next");
    pProcess32First = (PROCESSWALK)GetProcAddress(hKernel,"Process32First");
    pProcess32Next  = (PROCESSWALK)GetProcAddress(hKernel,"Process32Next");
    pHeap32ListFirst= (HEAPLISTWALK)GetProcAddress(hKernel,"Heap32ListFirst");
    pHeap32ListNext = (HEAPLISTWALK)GetProcAddress(hKernel,"Heap32ListNext");
    pHeap32First    = (HEAPFIRST)GetProcAddress(hKernel, "Heap32First");
    pHeap32Next     = (HEAPNEXT)GetProcAddress(hKernel, "Heap32Next");

    bRet =  pModule32First && pModule32Next  && pProcess32First &&
            pProcess32Next && //pThread32First && pThread32Next &&
            pHeap32ListFirst && pHeap32Next &&
            pCreateToolhelp32Snapshot;
  }
  else
    bRet = FALSE;  // Couldn't even get a module handle to KERNEL.

  if(!bRet)
    pModule32First = 0;
  return bRet;
}

BOOL GetProcessModule(DWORD dwPID,DWORD dwModuleID,LPMODULEENTRY32 lpMe32,
                      DWORD& dwTotalSize)
{
  MODULEENTRY32 me32={sizeof(me32)};
  HANDLE hModuleSnap;
  BOOL bRet,bFound=FALSE;
  dwTotalSize = 0;

  hModuleSnap=pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
  if (hModuleSnap==INVALID_HANDLE_VALUE)
    return FALSE;
  if (pModule32First(hModuleSnap, &me32))
  {
    do
    {
      dwTotalSize += me32.modBaseSize;
      if (me32.th32ModuleID==dwModuleID)
      {
        *lpMe32 = me32;
        bFound = TRUE;
      }
    } while (pModule32Next(hModuleSnap, &me32));
    bRet = bFound;
  }
  else
    bRet = FALSE;
  CloseHandle (hModuleSnap);
  return bRet;
}

DWORD GetHeapSize(DWORD dwPID)
{
  HEAPLIST32 hl32 = { sizeof(hl32) };
  HEAPENTRY32 he32 = { sizeof(he32) };

  HANDLE hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, dwPID);
  if (hSnapshot==INVALID_HANDLE_VALUE)
    return 0;

  DWORD dwSize = 0;

  for(BOOL bRes = pHeap32ListFirst(hSnapshot, &hl32); bRes; bRes=pHeap32ListNext(hSnapshot, &hl32)) {
    for(BOOL bRes1 = pHeap32First(&he32, hl32.th32ProcessID, hl32.th32HeapID); bRes1; bRes1 = pHeap32Next(&he32))
      dwSize += (DWORD)he32.dwBlockSize;
  }
  CloseHandle(hSnapshot);
  return dwSize;
}

BOOL KillProcess(DWORD dwPID)
{
  HANDLE hProcess;
  BOOL bRet;
  hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,dwPID);
  if (hProcess!=NULL)
  {
    bRet=TerminateProcess(hProcess,0xFFFFFFFF);
    if (bRet)
      WaitForSingleObject(hProcess,5000);
    CloseHandle(hProcess);
  }
  else
    bRet=FALSE;
  return bRet;
}

BOOL GetModuleNameFromExe(LPCTSTR szFileName,LPTSTR szModuleName,WORD cbLen)
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
  bResult = FALSE;
  if (pDosExeHdr -> e_magic == IMAGE_DOS_SIGNATURE && pNEHdr -> ne_magic == IMAGE_OS2_SIGNATURE)
  {
#ifndef UNICODE
    lstrcpyn(szModuleName, (LPSTR)pNEHdr + pNEHdr->ne_restab +1,Min((BYTE)*((LPSTR)pNEHdr + pNEHdr -> ne_restab) + 1,cbLen)+1);
    bResult=TRUE;
#else
    int n = MultiByteToWideChar(CP_ACP, 0,
                                (const char*)pNEHdr + pNEHdr->ne_restab + 1,
                                (BYTE)*((const LPBYTE)pNEHdr + pNEHdr->ne_restab) + 1,
                                szModuleName, cbLen);
    if (n) {
      szModuleName[n] = 0;
      bResult = TRUE;
    }
#endif
  }

  UnmapViewOfFile(pDosExeHdr);
  CloseHandle(hFileMapping);
  CloseHandle(hFile);

  return bResult;
}

void PrintModuleVersion(HANDLE InfoFile, LPTSTR pVersion, LPTSTR pDesc, int len);

void PrintModules95(HANDLE InfoFile, DWORD dwPID, _Opt& Opt)
{
    if (pModule32First==NULL && !InitToolhelp32())
        return;

    MODULEENTRY32 me32;
    HANDLE hModuleSnap;

    hModuleSnap=pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (hModuleSnap==INVALID_HANDLE_VALUE)
        return;
    me32.dwSize = sizeof(me32);
    for(BOOL bRet = pModule32First(hModuleSnap, &me32); bRet; bRet = pModule32Next (hModuleSnap, &me32)) {
        int len = 0;
        fprintf2(len, InfoFile, _T(" %p  %6X  %s"), me32.modBaseAddr, me32.modBaseSize,OUT_STRING(me32.szExePath));
        TCHAR   *pVersion, *pDesc;
        LPBYTE  pBuf;
        if(Opt.ExportModuleVersion && Plist::GetVersionInfo(me32.szExePath, pBuf, pVersion, pDesc)) {
            PrintModuleVersion(InfoFile, pVersion, pDesc, len);
            delete pBuf;
        }
        fputc(_T('\n'),InfoFile);
    }
    CloseHandle (hModuleSnap);
}
