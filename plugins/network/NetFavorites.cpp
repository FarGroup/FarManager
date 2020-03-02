#include "NetFavorites.hpp"
#include "NetCommon.hpp"

#define SZ_FAVORITES          L"Favorites"
#define SZ_FAVORITES_SUBKEY   SZ_FAVORITES L"\\%s"
#define SZ_USERNAME           L"UserName"
#define SZ_USERPASS           L"Password"

wchar_t szFavProv[] = L"Far Favorites Provider";

BOOL GetFavorites(LPNETRESOURCE pNR, NetResourceList *pList)
{
	NETRESOURCE tmp = {0};
	tmp.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;

	if (!pNR)
	{
		tmp.lpRemoteName = NULL;
		tmp.lpProvider = szFavProv;
		pList->Push(tmp);
	}
	else if (!lstrcmp(pNR->lpProvider, szFavProv))
	{
		wchar_t szKey[MAX_PATH*2];
		szKey[0] = 0;

		if (pNR->lpRemoteName)
		{
			wchar_t *p = wcschr(pNR->lpRemoteName, L'\\');

			if (p)
				p++;
			else
				p = pNR->lpRemoteName;

			FSF.sprintf(szKey, SZ_FAVORITES_SUBKEY, p);
		}
		else
			lstrcpy(szKey, SZ_FAVORITES);

#if 0
		HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, szKey, KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS);

		if (hKey)
		{
			wchar_t szSubKey[MAX_PATH];
			szSubKey[0] = 0;

			for (DWORD dwIndex = 0;
			        ERROR_SUCCESS == RegEnumKey(hKey,dwIndex,szSubKey,ARRAYSIZE(szSubKey));
			        dwIndex++)
			{
				int bTmp; LONG cData = sizeof(bTmp);
				bTmp = 0;
				wchar_t szSrc[MAX_PATH];
				szSrc[0] = 0;

				if (ERROR_SUCCESS == RegQueryValue(hKey, szSubKey, (wchar_t*)&bTmp, &cData) && bTmp == (int)L'1')
				{
					tmp.lpProvider = szFavProv;
					tmp.lpRemoteName = szSrc;
					tmp.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;

					if (pNR->lpRemoteName)
						lstrcpy(szSrc, pNR->lpRemoteName);
					else
						lstrcpy(szSrc, GetMsg(MFavorites));

					lstrcat(szSrc, L"\\");
					lstrcat(szSrc, szSubKey);
					pList->Push(tmp);
				}
				else
				{
					szSrc[0] = L'\\';
					szSrc[1] = L'\\';
					szSrc[2] = 0;
					lstrcat(szSrc, szSubKey);

					if (Opt.FavoritesFlags & FAVORITES_CHECK_RESOURCES)
					{
						tmp.lpProvider = NULL;
						tmp.lpRemoteName = szSrc;
						tmp.dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
						pList->Push(tmp);
					}
					else if (NetBrowser::GetResourceInfo(szSrc, &tmp))
					{
						pList->Push(tmp);
						NetResourceList::DeleteNetResource(tmp);
					}

					ZeroMemory(&tmp, sizeof(tmp));
				}
			}

			RegCloseKey(hKey);
		}

#endif
		return TRUE;
	}

	return FALSE;
}

BOOL CheckFavoriteItem(const LPNETRESOURCE pNR)
{
	return pNR && !lstrcmp(pNR->lpProvider, szFavProv);
}

BOOL GetResourceKey(wchar_t* lpRemoteName, const wchar_t* rootKey, wchar_t* lpResourceKey, size_t *cSize)
{
	return FALSE;
#if 0
	// We should be sure that "Favorites" is a folder
	SetRegKey(HKEY_CURRENT_USER, SZ_FAVORITES, NULL, L"1");

	if (!lpResourceKey || !cSize || !*cSize)
		return FALSE;

	wchar_t *p = NULL;
	wchar_t szKey[MAX_PATH];

	if (lpRemoteName)
	{
		while (*lpRemoteName==L'\\') lpRemoteName++;

		if (0 != (p = wcschr(lpRemoteName, L'\\')))
			*p = 0;

		if (!FSF.LStricmp(lpRemoteName, GetMsg(MFavorites)))
		{
			if (p) *p = L'\\', p++;

			if (p)
				FSF.sprintf(szKey, SZ_FAVORITES_SUBKEY, p);
			else
				lstrcpy(szKey, SZ_FAVORITES);

			HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, szKey);

			if (hKey)
			{
				RegCloseKey(hKey);
				lstrcpyn(lpResourceKey, szKey, (int)*cSize);
				return TRUE;
			}
		}
		else
		{
			if (p) *p = L'\\';
		}
	}

	if (!lpRemoteName)
	{
		if (*cSize < (size_t)(lstrlen(rootKey)+1))
		{
			*cSize = 0;
		}
		else
		{
			lstrcpy(lpResourceKey, rootKey);
			*cSize = lstrlen(lpResourceKey);
		}

		return FALSE;
	}

	int buffLen = lstrlen(lpRemoteName) + lstrlen(SZ_FAVORITES) + MAX_PATH;
	wchar_t* buff = new wchar_t[buffLen];
	p = wcschr(lpRemoteName, L'\\');

	if (p)
		*p = 0;

	FSF.sprintf(buff, L"%s\\%s", rootKey, lpRemoteName);

	if (p)
		*p = L'\\';

	BOOL res = TRUE;
	HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, buff);

	if (hKey)
	{
		RegCloseKey(hKey);
		FSF.sprintf(lpResourceKey, L"%s\\%s", rootKey, lpRemoteName);
		*cSize = lstrlen(lpResourceKey);
	}
	else
	{
		res = FALSE;

		if (0 != (hKey = OpenRegKey(HKEY_CURRENT_USER, rootKey,
		                            KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS)))
		{
			for (DWORD dwIndex = 0;
			        ERROR_SUCCESS == RegEnumKey(hKey, dwIndex, szKey, ARRAYSIZE(szKey));
			        dwIndex++)
			{
				int bTmp = 0;
				LONG cData = sizeof(bTmp);

				if (ERROR_SUCCESS == RegQueryValue(hKey, szKey, (wchar_t*)&bTmp, &cData) && bTmp == (int)L'1')
				{
					FSF.sprintf(buff, L"%s\\%s", rootKey, szKey);

					if (GetResourceKey(lpRemoteName, buff, lpResourceKey, cSize))
					{
						res = TRUE;
						break;
					}
				}
			}

			RegCloseKey(hKey);
		}
	}

	if (!res)
	{
		FSF.sprintf(lpResourceKey, L"%s\\%s", rootKey, lpRemoteName);
	}

	delete(buff);
	return res;
#endif
}

void WriteFavoriteItem(LPFAVORITEITEM lpFavItem, wchar_t* /*szFolder*/)
{
#if 0
	wchar_t szResourceKey[MAX_PATH];
	size_t cSize = ARRAYSIZE(szResourceKey);
	GetResourceKey(lpFavItem->lpRemoteName, SZ_FAVORITES, szResourceKey, &cSize);
	SetRegKey(HKEY_CURRENT_USER, szResourceKey, SZ_USERNAME, lpFavItem->lpUserName);
	SetRegKey(HKEY_CURRENT_USER, szResourceKey, SZ_USERPASS, lpFavItem->lpPassword);
#endif
}

BOOL ReadFavoriteItem(LPFAVORITEITEM lpFavItem)
{
#if 0
	wchar_t resKey[MAX_PATH];
	size_t cData = ARRAYSIZE(resKey);

	if (lpFavItem && GetResourceKey(lpFavItem->lpRemoteName, SZ_FAVORITES, resKey, &cData))
	{
		GetRegKey(HKEY_CURRENT_USER, resKey, SZ_USERNAME, lpFavItem->lpUserName,
		          L"", lpFavItem->ccUserName);
		GetRegKey(HKEY_CURRENT_USER, resKey, SZ_USERPASS, lpFavItem->lpPassword,
		          L"", lpFavItem->ccPassword);
		return TRUE;
	}

#endif
	return FALSE;
}

BOOL GetFavoritesParent(NETRESOURCE& SrcRes, LPNETRESOURCE lpParent)
{
	wchar_t* p;
	NETRESOURCE nr = {0};
	nr.lpProvider = szFavProv;
	nr.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;

	if (!lstrcmp(SrcRes.lpProvider, szFavProv))
	{
		p = (wchar_t*)FSF.PointToName(SrcRes.lpRemoteName);

		if (p && p != SrcRes.lpRemoteName)
		{
			p--;
			*p = 0;
		}
		else
			return FALSE;

		if (lpParent)
		{
			if (wcschr(SrcRes.lpRemoteName, L'\\'))
				nr.lpRemoteName = SrcRes.lpRemoteName;

			NetResourceList::CopyNetResource(*lpParent, nr);
		}

		if (p)
			*p = L'\\';

		return TRUE;
	}

	if (SrcRes.dwDisplayType == RESOURCEDISPLAYTYPE_SHARE)
		return FALSE;

	wchar_t szResourceKey[MAX_PATH];
	size_t cSize = ARRAYSIZE(szResourceKey);

	if (GetResourceKey(SrcRes.lpRemoteName, SZ_FAVORITES, szResourceKey, &cSize))
	{
		p = (wchar_t*)FSF.PointToName(szResourceKey);

		if (!p || !*p)
		{
			return FALSE;
		}

		if (lpParent)
		{
			*--p = 0;
			wchar_t res[MAX_PATH];
			res[0] = 0;
			p = wcschr(szResourceKey, L'\\');

			if (p)
			{
				lstrcpy(res, GetMsg(MFavorites));
				lstrcat(res, p);
				p = res;
			}

			nr.lpRemoteName = p;
			NetResourceList::CopyNetResource(*lpParent, nr);
		}

		return TRUE;
	}

	return FALSE;
}

#if 0
STDAPI
EliminateSubKey(HKEY hkey, LPTSTR strSubKey);
#endif

BOOL GetFavoriteResource(const wchar_t *SrcName, LPNETRESOURCE DstNetResource)
{
#if 0
	NETRESOURCE nr = {0};
	wchar_t *p1, *p = SrcName;

	while (*p == L'\\') ++p;

	wchar_t szKey[MAX_PATH];
	size_t cSize = ARRAYSIZE(szKey);
	int dwKey = 0;
	szKey[0] = 0;

	if (GetResourceKey(p, SZ_FAVORITES, szKey, &cSize))
	{
		if (!lstrcmpi(szKey, SZ_FAVORITES))
		{
			if (DstNetResource)
			{
				nr.lpProvider = szFavProv;
				nr.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
				NetResourceList::CopyNetResource(*DstNetResource, nr);
			}

			return TRUE;
		}

		if (GetRegKey(HKEY_CURRENT_USER, szKey, NULL, (wchar_t*)&dwKey, L"0", sizeof(dwKey))
		        && dwKey == L'1')
		{
			p = wcschr(szKey, L'\\');

			if (p)
			{
				if (DstNetResource)
				{
					nr.lpProvider = szFavProv;
					nr.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
					wchar_t szOutName[MAX_PATH];
					lstrcpy(szOutName, GetMsg(MFavorites));
					lstrcat(szOutName, p);
					nr.lpRemoteName = szOutName;
					NetResourceList::CopyNetResource(*DstNetResource, nr);
				}

				return TRUE;
			}
		}
		else
		{
			p1 = (wchar_t*)FSF.PointToName(szKey);

			if (szKey != p1)
			{
				*(p1-1) = 0;

				if (GetRegKey(HKEY_CURRENT_USER, szKey, NULL, (wchar_t*)&dwKey, L"0", sizeof(dwKey))
				        && dwKey == L'1')
				{
					p = p1;
				}
				else
				{
					p = (wchar_t*)FSF.PointToName(szKey);
					*(p1-1) = L'\\';

					if (p != szKey)
					{
						*(p - 1) = 0;

						if (!GetRegKey(HKEY_CURRENT_USER, szKey, NULL, (wchar_t*)&dwKey, L"0", sizeof(dwKey))
						        || dwKey != L'1')
							return FALSE;
					}
				}

				if (DstNetResource)
				{
					lstrcpy(szKey, L"\\\\");
					lstrcat(szKey, p);
					nr.lpRemoteName = szKey;
					nr.dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
					NetResourceList::CopyNetResource(*DstNetResource, nr);
				}

				return TRUE;
			}
		}
	}

#endif
	return FALSE;
}

BOOL RemoveFromFavorites(wchar_t *SrcName, LPREMOVEFROMFAVCB /*pUserCallBack*/, LPVOID /*pUserData*/)
{
	return FALSE;
#if 0
	wchar_t *p = wcschr(SrcName, L'\\');

	if (p)
		while (*++p == L'\\');
	else
		p = SrcName;

	wchar_t szKey[MAX_PATH];
	FSF.sprintf(szKey, SZ_FAVORITES_SUBKEY, p);
	HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, L"", MAXIMUM_ALLOWED);

	if (!hKey)
		return FALSE;

	BOOL res = SUCCEEDED(EliminateSubKey(hKey, szKey));
	RegCloseKey(hKey);
	return res;
#endif
}

//---------------------------------------------------------------------------
//
// EliminateSubKey
//
// Try to enumerate all keys under this one.
// if we find anything, delete it completely.
// Otherwise just delete it.
//
//---------------------------------------------------------------------------
#if 0
STDAPI
EliminateSubKey(HKEY hkey, LPTSTR strSubKey)
{
	HKEY hk;

	if (0 == lstrlen(strSubKey))
	{
		// defensive approach
		return E_FAIL;
	}

	LONG lreturn = RegOpenKeyEx(hkey
	                            , strSubKey
	                            , 0
	                            , MAXIMUM_ALLOWED
	                            , &hk);

	if (ERROR_SUCCESS == lreturn)
	{
		// Keep on enumerating the first (zero-th)
		// key and deleting that
		for (; ;)
		{
			wchar_t Buffer[MAX_PATH];
			DWORD dw = ARRAYSIZE(Buffer);
			FILETIME ft;
			lreturn = RegEnumKeyEx(hk
			                       , 0
			                       , Buffer
			                       , &dw
			                       , NULL
			                       , NULL
			                       , NULL
			                       , &ft);

			if (ERROR_SUCCESS == lreturn)
			{
				EliminateSubKey(hk, Buffer);
			}
			else
			{
				break;
			}
		}

		RegCloseKey(hk);
		RegDeleteKey(hkey, strSubKey);
	}

	return NOERROR;
}

BOOL RecursiveSetValue(HKEY hKey, wchar_t* lpSubKey)
{
	if (!lpSubKey || !*lpSubKey)
		return FALSE;

	wchar_t *p = wcschr(lpSubKey, L'\\');

	if (p)
	{
		*p = 0;

		while (*++p == L'\\');

		if (*p)
		{
			HKEY hSubKey;

			if (ERROR_SUCCESS != RegCreateKeyEx(hKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE,
			                                    KEY_ALL_ACCESS, NULL, &hSubKey, NULL))
				return FALSE;

			BOOL res = RecursiveSetValue(hSubKey, p);
			RegCloseKey(hSubKey);

			if (!res)
				return FALSE;
		}
	}

	return ERROR_SUCCESS == RegSetValue(hKey, lpSubKey, REG_SZ, L"1", 2);
}
#endif

const wchar_t *g_szInvalidChars=L"\\/";

BOOL ValidatePath(const wchar_t *szPath)
{
	if (!szPath||!*szPath)
		return FALSE;

	for (const wchar_t *p=szPath; *p; p++)
		for (const wchar_t *p1=g_szInvalidChars; *p1; p1++)
			if (*p1==*p)
				return FALSE;

	return TRUE;
}

#if 0
class Unknown
{
	private:
		LONG m_refCount;
	protected:
		virtual ~Unknown() {}
	public:
		Unknown():m_refCount(1) {}
		virtual LONG AddRef()
		{
			return InterlockedIncrement(&m_refCount);
		}
		virtual LONG Release()
		{
			LONG res;

			if (0 >= (res = InterlockedDecrement(&m_refCount)))
				delete this;

			return res;
		}
};

class RegParser : public Unknown
{
	private:
		HKEY m_hKey;
		RegParser* m_parent;
	protected:
		~RegParser()
		{
			if (m_parent)
				m_parent->Release();

			if (m_hKey)
				RegCloseKey(m_hKey);
		}
	public:
		RegParser(): Unknown(), m_hKey(0), m_parent(0) {}
		/**
		 * @brief Function initialize the parser.
		 *
		 * It sets up the parser's internal m_hKey allowing to perform operations on it.
		 */
		bool init(RegParser* parentItem, const wchar_t* subKey, bool createSubKey=false)
		{
			if (m_parent)
			{
				m_parent->Release();
				m_parent = NULL;
			}

			if (m_hKey)
			{
				RegCloseKey(m_hKey);
				m_hKey = 0;
			}

			if (createSubKey)
			{
				if (parentItem && parentItem->m_hKey)
				{
					DWORD dwDispositon;

					if (ERROR_SUCCESS == RegCreateKeyEx(parentItem->m_hKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dwDispositon))
					{
						if (dwDispositon == REG_CREATED_NEW_KEY)
						{
							RegSetValue(m_hKey, NULL, REG_SZ, L"1", 2);
						}
					}
				}
				else
					m_hKey = CreateRegKey(HKEY_CURRENT_USER, subKey);
			}
			else
			{
				if (parentItem && parentItem->m_hKey)
					RegOpenKeyEx(parentItem->m_hKey, subKey,0, KEY_ALL_ACCESS, &m_hKey);
				else
					m_hKey = OpenRegKey(HKEY_CURRENT_USER, subKey, KEY_ALL_ACCESS);
			}

			if (m_hKey)
			{
				m_parent = parentItem;

				if (m_parent)
					m_parent->AddRef();

				return true;
			}

			return false;
		}


		bool parsePath(wchar_t* path, bool createFolders, RegParser** ppResult)
		{
			RegParser* childItem;
			wchar_t* pRestOfPath;

			if (!path)
			{
				if (ppResult)
				{
					*ppResult = this;
					AddRef();
				}

				return true;
			}

			pRestOfPath = wcschr(path, L'\\');

			if (pRestOfPath)
				*pRestOfPath++ = L'\0';

			bool res = true;

			if (!lstrcmp(path, L".."))
			{
				childItem = m_parent;

				if (childItem)
					childItem->AddRef();
				else
					res = false;
			}
			else if (!lstrcmp(path, L"."))
			{
				childItem = this;
				childItem->AddRef();
			}
			else
			{
				childItem = new RegParser;
				res = childItem->init(this, path, createFolders);
			}

			if (pRestOfPath)
				pRestOfPath[-1] = L'\\';

			if (!childItem)
				return false;

			if (!res)
			{
				childItem->Release();
				return false;
			}

			if (!pRestOfPath)
			{
				if (ppResult)
					*ppResult = childItem;
				else
					childItem->Release();

				return true;
			}

			res = childItem->parsePath(pRestOfPath, createFolders, ppResult);
			childItem->Release();
			return res;
		}
};

bool GetFavoriteRoot(RegParser** favoritesRoot)
{
	if (!favoritesRoot)
		return false;

	RegParser* root = new RegParser;

	if (!root->init(NULL, SZ_FAVORITES))
	{
		root->Release();
		return false;
	}

	*favoritesRoot = root;
	return true;
}
#endif

BOOL CreateSubFolder(wchar_t *szRoot, wchar_t *szSubFolder)
{
	return FALSE;
#if 0
	wchar_t szFavorites[] = SZ_FAVORITES;

	if (szRoot && !FSF.LStrnicmp(szRoot, szFavorites, lstrlen(szFavorites)))
	{
		szRoot = wcschr(szRoot, L'\\');

		if (szRoot)
			szRoot++;
	}

	if (!szSubFolder)
		return FALSE;

	RegParser* root;

	if (!GetFavoriteRoot(&root))
		return FALSE;

	bool res;
	RegParser* currFolder;
	res = root->parsePath(szRoot, false, &currFolder);
	root->Release();

	if (!res)
		return FALSE;

	FSF.Unquote(szSubFolder);
	res = currFolder->parsePath(szSubFolder, true, NULL);
	currFolder->Release();
	return res?TRUE:FALSE;
#endif
}
