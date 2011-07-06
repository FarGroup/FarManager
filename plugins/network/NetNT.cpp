#include "NetClass.hpp"
#include "NetCommon.hpp"

void NetBrowser::GetHideShareNT()
{
#ifdef NETWORK_LOGGING
	LogData(L"Entering NetBrowser::GetHideShareNT()");
#endif

	if (UsedNetFunctions)
	{
#ifdef NETWORK_LOGGING
		LogData(L"UsedNetFunctions = TRUE");
#endif
		wchar_t lpwsNetPath[MAX_PATH];
		PSHARE_INFO_1 BufPtr, p;
		NET_API_STATUS res;

		if (PCurResource == NULL) return;

		LPTSTR lpszServer = PCurResource->lpRemoteName;
		wchar_t szResPath [MAX_PATH];
		LPTSTR pszSystem;
		NETRESOURCE pri;
		NETRESOURCE nr [256];
		DWORD er=0,tr=0,resume=0,rrsiz;
		lstrcpyn(lpwsNetPath,lpszServer,ARRAYSIZE(lpwsNetPath));

		do
		{
			res = FNetShareEnum((LPWSTR)lpwsNetPath, 1, (LPBYTE *) &BufPtr, MAX_PREFERRED_LENGTH, &er, &tr, &resume);

			if (res == ERROR_SUCCESS || res == ERROR_MORE_DATA)
			{
				p=BufPtr;

				for (DWORD J=0; J < er; J++)
				{
					memset((void *)&pri,0,sizeof(pri));
					pri.dwScope = RESOURCE_GLOBALNET;
					pri.dwType = RESOURCETYPE_DISK;
					pri.lpLocalName = NULL;
					lstrcpy(szResPath,lpszServer);
					lstrcat(szResPath,L"\\");
					{
						size_t pos = lstrlen(szResPath);
						lstrcpyn(&szResPath[pos], p->shi1_netname, (int)(ARRAYSIZE(szResPath)-pos));
					}

					if (szResPath[lstrlen(szResPath)-1] == L'$' &&
					        lstrcmp(&szResPath[lstrlen(szResPath)-4],L"IPC$"))
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

						NetList.Push(nr [0]);
					}

					p++;
				}

				FNetApiBufferFree(BufPtr);
			}

			if (res == ERROR_SUCCESS)
				break;
		}
		while (res==ERROR_MORE_DATA);
	}

#ifdef NETWORK_LOGGING
	else
		LogData(L"UsedNetFunctions = FALSE");

	LogData(L"Leaving NetBrowser::GetHideShareNT()");
#endif
}
