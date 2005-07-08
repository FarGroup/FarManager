void NetBrowser::GetHideShareNT()
{
#ifdef NETWORK_LOGGING
	LogData("Entering NetBrowser::GetHideShareNT()");
#endif
  if(UsedNetFunctions)
  {
#ifdef NETWORK_LOGGING
	LogData("UsedNetFunctions = TRUE");
#endif
    char lpwsNetPath[NM];
    PSHARE_INFO_1 BufPtr, p;
    NET_API_STATUS res;
    if(PCurResource == NULL) return;

    LPTSTR    lpszServer = PCurResource->lpRemoteName;
    char szResPath [NM];
    LPTSTR pszSystem;
    NETRESOURCE pri;
    NETRESOURCE nr [256];
    DWORD er=0,tr=0,resume=0,rrsiz;

    MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,lpszServer,-1,(LPWSTR)lpwsNetPath,NM);
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
          strcpy(szResPath,lpszServer);
          strcat(szResPath,"\\");
          WideCharToMultiByte(CP_ACP,0,(LPWSTR)p->shi1_netname,-1,&szResPath[strlen(szResPath)],NM,NULL,NULL);

          if(szResPath[strlen(szResPath)-1] == '$' &&
           strcmp(&szResPath[strlen(szResPath)-4],"IPC$"))
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
	  LogData("UsedNetFunctions = FALSE");
	LogData("Leaving NetBrowser::GetHideShareNT()");
#endif
}

void NetBrowser::GetHideShare95()
{
#ifdef NETWORK_LOGGING
  fprintf (LogFile, "GetHideShare95()\n");
#endif
  if(UsedNetFunctions)
  {
    if(PCurResource == NULL) return;
    
    const int MAX_ENTRIES = 64;
    char szResPath [NM];
    LPTSTR pszSystem;
    NETRESOURCE nr [256];
    USHORT nEntriesRead, nTotalEntries;
    
    int cbBuffer = MAX_ENTRIES * sizeof (share_info_1);
    share_info_1 *pBuf = (share_info_1 *) my_malloc (cbBuffer);
    NET_API_STATUS nStatus = FNetShareEnum95 (PCurResource->lpRemoteName, 1, 
      (char *) pBuf, cbBuffer, &nEntriesRead, &nTotalEntries);

    if (nStatus == ERROR_MORE_DATA && nEntriesRead < nTotalEntries)
    {
      cbBuffer = nTotalEntries * sizeof (share_info_1);
      pBuf = (share_info_1 *) my_realloc (pBuf, cbBuffer);
      nStatus = FNetShareEnum95 (PCurResource->lpRemoteName, 1, (char *) pBuf,
        cbBuffer, &nEntriesRead, &nTotalEntries);
    }

    if (nStatus != NERR_Success && nStatus != ERROR_MORE_DATA)
    {
      my_free (pBuf);
#ifdef NETWORK_LOGGING
      fprintf (LogFile, "nStatus=%d\n", nStatus);
#endif
      return;
    }

    share_info_1 *p=pBuf;
    for(DWORD J=0; J < nEntriesRead ; J++)
    {
#ifdef NETWORK_LOGGING
      fprintf (LogFile, "shi1_netname %s\n", p->shi1_netname);
#endif
      NETRESOURCE pri;
      memset((void *)&pri,0,sizeof(pri));
      pri.dwScope = RESOURCE_GLOBALNET;
      pri.dwType = RESOURCETYPE_DISK;
      pri.lpLocalName = NULL;
      strcpy (szResPath, PCurResource->lpRemoteName);
      strcat (szResPath, "\\");
      strcat (szResPath, p->shi1_netname);
      
      if(szResPath[strlen(szResPath)-1] == '$' &&
        strcmp(&szResPath[strlen(szResPath)-4],"IPC$"))
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

    my_free (pBuf);
  }
#ifdef NETWORK_LOGGING
  else
    fprintf (LogFile, "Failed to load NET functions\n");
#endif
}
