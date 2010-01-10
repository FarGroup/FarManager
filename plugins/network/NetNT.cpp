#include "NetClass.hpp"
#include "NetCommon.hpp"

void NetBrowser::GetHideShareNT()
{
#ifdef NETWORK_LOGGING
  LogData(_T("Entering NetBrowser::GetHideShareNT()"));
#endif
  if(UsedNetFunctions)
  {
#ifdef NETWORK_LOGGING
  LogData(_T("UsedNetFunctions = TRUE"));
#endif
    TCHAR lpwsNetPath[MAX_PATH];
    PSHARE_INFO_1 BufPtr, p;
    NET_API_STATUS res;
    if(PCurResource == NULL) return;

    LPTSTR lpszServer = PCurResource->lpRemoteName;
    TCHAR szResPath [MAX_PATH];
    LPTSTR pszSystem;
    NETRESOURCE pri;
    NETRESOURCE nr [256];
    DWORD er=0,tr=0,resume=0,rrsiz;

#ifndef UNICODE
    MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,lpszServer,-1,(LPWSTR)lpwsNetPath,ArraySize(lpwsNetPath));
#else
    lstrcpyn(lpwsNetPath,lpszServer,ArraySize(lpwsNetPath));
#endif
    do
    {
      res = FNetShareEnum((LPWSTR)lpwsNetPath, 1, (LPBYTE *) &BufPtr, MAX_PREFERRED_LENGTH, &er, &tr, &resume);
      if(res == ERROR_SUCCESS || res == ERROR_MORE_DATA)
      {
        p=BufPtr;
        for(DWORD J=0; J < er; J++)
        {
          memset((void *)&pri,0,sizeof(pri));
          pri.dwScope = RESOURCE_GLOBALNET;
          pri.dwType = RESOURCETYPE_DISK;
          pri.lpLocalName = NULL;
          lstrcpy(szResPath,lpszServer);
          lstrcat(szResPath,_T("\\"));
          {
            size_t pos = lstrlen(szResPath);
#ifndef UNICODE
            WideCharToMultiByte(CP_ACP,0,(LPWSTR)p->shi1_netname,-1,&szResPath[pos],(int)(ArraySize(szResPath)-pos),NULL,NULL);
#else
            lstrcpyn(&szResPath[pos], p->shi1_netname, (int)(ArraySize(szResPath)-pos));
#endif
          }
          if(szResPath[lstrlen(szResPath)-1] == _T('$') &&
           lstrcmp(&szResPath[lstrlen(szResPath)-4],_T("IPC$")))
          {
            pri.lpRemoteName = szResPath;
            pri.dwUsage = RESOURCEUSAGE_CONTAINER;
            pri.lpProvider = NULL;
            rrsiz = sizeof(nr);

            // we need to provide buffer space for WNetGetResourceInformation
            int rc = FWNetGetResourceInformation(&pri,(void *)&nr [0],&rrsiz,&pszSystem);
            if (rc!=NO_ERROR)
            {
              p++;
              continue;
              //break; //?????
            }

            if (p->shi1_type == STYPE_DISKTREE)
              nr [0].dwType=RESOURCETYPE_DISK;
            if (p->shi1_type == STYPE_PRINTQ)
              nr [0].dwType=RESOURCETYPE_PRINT;
            if (p->shi1_type == STYPE_SPECIAL)
              nr [0].dwType=RESOURCETYPE_DISK;

            NetList.Push (nr [0]);
          }
          p++;
        }
        FNetApiBufferFree(BufPtr);
      }
      if(res == ERROR_SUCCESS)
        break;
    } while (res==ERROR_MORE_DATA);
  }
#ifdef NETWORK_LOGGING
  else
    LogData(_T("UsedNetFunctions = FALSE"));
  LogData(_T("Leaving NetBrowser::GetHideShareNT()"));
#endif
}

void NetBrowser::GetHideShare95()
{
#ifdef NETWORK_LOGGING
  LogData(_T("GetHideShare95()\n"));
#endif
  if(UsedNetFunctions)
  {
    if(PCurResource == NULL) return;

    const int MAX_ENTRIES = 64;
    TCHAR szResPath [MAX_PATH];
    LPTSTR pszSystem;
    NETRESOURCE nr [256];
    USHORT nEntriesRead, nTotalEntries;

    USHORT cbBuffer = MAX_ENTRIES * sizeof (share_info_1);
    share_info_1 *pBuf = (share_info_1 *) malloc (cbBuffer);
    NET_API_STATUS nStatus = FNetShareEnum95 (PCurResource->lpRemoteName, 1,
      (TCHAR *) pBuf, cbBuffer, &nEntriesRead, &nTotalEntries);

    if (nStatus == ERROR_MORE_DATA && nEntriesRead < nTotalEntries)
    {
      cbBuffer = nTotalEntries * sizeof (share_info_1);
      pBuf = (share_info_1 *) realloc (pBuf, cbBuffer);
      nStatus = FNetShareEnum95 (PCurResource->lpRemoteName, 1, (TCHAR *) pBuf,
        cbBuffer, &nEntriesRead, &nTotalEntries);
    }

    if (nStatus != NERR_Success && nStatus != ERROR_MORE_DATA)
    {
      free (pBuf);
#ifdef NETWORK_LOGGING
      _ftprintf (LogFile, _T("nStatus=%d\n"), nStatus);
#endif
      return;
    }

    share_info_1 *p=pBuf;
    for(DWORD J=0; J < nEntriesRead ; J++)
    {
#ifdef NETWORK_LOGGING
      _ftprintf (LogFile, _T("shi1_netname %s\n"), p->shi1_netname);
#endif
      NETRESOURCE pri;
      memset((void *)&pri,0,sizeof(pri));
      pri.dwScope = RESOURCE_GLOBALNET;
      pri.dwType = RESOURCETYPE_DISK;
      pri.lpLocalName = NULL;
      lstrcpy (szResPath, PCurResource->lpRemoteName);
      lstrcat (szResPath, _T("\\"));
      lstrcat (szResPath, p->shi1_netname);

      if(szResPath[lstrlen(szResPath)-1] == _T('$') &&
        lstrcmp(&szResPath[lstrlen(szResPath)-4],_T("IPC$")))
      {
        pri.lpRemoteName = szResPath;
        pri.dwUsage = RESOURCEUSAGE_CONTAINER;
        pri.lpProvider = NULL;
        DWORD rrsiz = sizeof(nr);

        // we need to provide buffer space for WNetGetResourceInformation
        int rc = FWNetGetResourceInformation(&pri,(void *)&nr [0],&rrsiz,&pszSystem);
        if (rc!=NO_ERROR)
        {
          p++;
          continue;
          //break; //?????
        }

        if (p->shi1_type == STYPE_DISKTREE)
          nr [0].dwType=RESOURCETYPE_DISK;
        if (p->shi1_type == STYPE_PRINTQ)
          nr [0].dwType=RESOURCETYPE_PRINT;
        if (p->shi1_type == STYPE_SPECIAL)
          nr [0].dwType=RESOURCETYPE_DISK;

        NetList.Push (nr [0]);
      }
      p++;
    }

    free (pBuf);
  }
#ifdef NETWORK_LOGGING
  else
    _ftprintf (LogFile, _T("Failed to load NET functions\n"));
#endif
}
