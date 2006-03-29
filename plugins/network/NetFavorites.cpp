#include "NetFavorites.hpp"
#include "NetCommon.hpp"
#include "NetReg.hpp"

#define SZ_FAVORITES "Favorites"
#define SZ_FAVORITES_SUBKEY SZ_FAVORITES"\\%s"
#define SZ_USERNAME "UserName"
#define SZ_USERPASS "Password"

char* szFavProv = "Far Favorites Provider";

BOOL GetFavorites(LPNETRESOURCE pNR, NetResourceList *pList)
{
  NETRESOURCE tmp = {0};

  tmp.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
  if(!pNR)
  {
    tmp.lpRemoteName = NULL;
    tmp.lpProvider = szFavProv;
    pList->Push(tmp);
  }
  else if(!lstrcmp(pNR->lpProvider, szFavProv))
  {
    char szKey[NM*2] = {0};
    if(pNR->lpRemoteName)
    {
      char *p = strchr(pNR->lpRemoteName, '\\');
      if(p)
        p++;
      else
        p = pNR->lpRemoteName;
      FSF.sprintf(szKey, SZ_FAVORITES_SUBKEY, p);
    }else
      lstrcpy(szKey, SZ_FAVORITES);
    HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, szKey, KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS);
    if(hKey)
    {
      char szSubKey[NM] = {0};
      for(DWORD dwIndex = 0; ERROR_SUCCESS == RegEnumKey(hKey, dwIndex, szSubKey,
        sizeof(szSubKey)); dwIndex++)
      {
        int bTmp; LONG cData = sizeof(bTmp);
        bTmp = 0;
        char szSrc[NM] = {0};

        if(ERROR_SUCCESS == RegQueryValue(hKey, szSubKey, (char*)&bTmp, &cData) && bTmp == (int)'1')
        {
          tmp.lpProvider = szFavProv;
          tmp.lpRemoteName = szSrc;
          tmp.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
          if(pNR->lpRemoteName)
            lstrcpy(szSrc, pNR->lpRemoteName);
          else
            OemToChar(GetMsg(MFavorites), szSrc);
          lstrcat(szSrc, "\\");
          lstrcat(szSrc, szSubKey);
          pList->Push(tmp);
        }
        else
        {
          szSrc[0] = '\\';
          szSrc[1] = '\\';
          szSrc[2] = 0;
          lstrcat(szSrc, szSubKey);
          if(Opt.FavoritesFlags & FAVORITES_CHECK_RESOURCES)
          {
            tmp.lpProvider = NULL;
            tmp.lpRemoteName = szSrc;
            tmp.dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
            pList->Push(tmp);
          }
          else if(NetBrowser::GetResourceInfo(szSrc, &tmp))
          {
            pList->Push(tmp);
            NetResourceList::DeleteNetResource(tmp);
          }
          ZeroMemory(&tmp, sizeof(tmp));
        }
      }

      RegCloseKey(hKey);
    }
    return TRUE;
  }
  return FALSE;
}

BOOL CheckFavoriteItem(const LPNETRESOURCE pNR)
{
  if(pNR && !lstrcmp(pNR->lpProvider, szFavProv))
    return TRUE;
  return FALSE;
}

BOOL GetResourceKey(char* lpRemoteName, char* rootKey, char* lpResourceKey, int *cSize)
{
  // We should be sure that "Favorites" is a folder
  SetRegKey(HKEY_CURRENT_USER, SZ_FAVORITES, NULL, "1");
  if(!lpResourceKey || !cSize || !*cSize)
    return FALSE;

  char *p = NULL;
  char szKey[NM];
  if(lpRemoteName)
  {
    while (*lpRemoteName=='\\') lpRemoteName++;
    if(0 != (p = strchr(lpRemoteName, '\\')))
      *p = 0;
    char szFavoritesName[NM]; OemToChar(GetMsg(MFavorites), szFavoritesName);
    if(!lstrcmpi(lpRemoteName, szFavoritesName))
    {
      if(p) *p = '\\', p++;
      if(p)
        FSF.sprintf(szKey, SZ_FAVORITES_SUBKEY, p);
      else
        lstrcpy(szKey, SZ_FAVORITES);
      HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, szKey);
      if(hKey)
      {
        RegCloseKey(hKey);
        lstrcpyn(lpResourceKey, szKey, *cSize);
        return TRUE;
      }
    }
    else
      if(p) *p = '\\';

  }
  if(!lpRemoteName)
  {
    if(*cSize < lstrlen(rootKey)+1)
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
  int buffLen = lstrlen(lpRemoteName) + lstrlen(SZ_FAVORITES) + NM;
  char* buff = new(char[buffLen]);

  p = strchr(lpRemoteName, '\\');
  if(p)
    *p = 0;

  FSF.sprintf(buff, "%s\\%s", rootKey, lpRemoteName);

  if(p)
    *p = '\\';
  BOOL res = TRUE;
  HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, buff);
  if(hKey)
  {
    RegCloseKey(hKey);
    FSF.sprintf(lpResourceKey, "%s\\%s", rootKey, lpRemoteName);

    *cSize = lstrlen(lpResourceKey);
  }
  else
  {
    res = FALSE;
    if(0 != (hKey = OpenRegKey(HKEY_CURRENT_USER, rootKey,
      KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS)))
    {
      for(DWORD dwIndex = 0; ERROR_SUCCESS == RegEnumKey(hKey, dwIndex,
        szKey, sizeof(szKey)); dwIndex++)
      {
        int bTmp = 0;
        LONG cData = sizeof(bTmp);
        if(ERROR_SUCCESS == RegQueryValue(hKey, szKey, (char *)&bTmp, &cData) && bTmp == (int)'1')
        {
          FSF.sprintf(buff, "%s\\%s", rootKey, szKey);
          if(GetResourceKey(lpRemoteName, buff, lpResourceKey, cSize))
          {
            res = TRUE;
            break;
          }
        }
      }

      RegCloseKey(hKey);
    }
  }

  if(!res)
  {
    FSF.sprintf(lpResourceKey, "%s\\%s", rootKey, lpRemoteName);
  }

  delete(buff), buff = NULL;
  return res;
}

void WriteFavoriteItem(LPFAVORITEITEM lpFavItem, char* /*szFolder*/)
{
  char szResourceKey[NM];
  int cSize = sizeof(szResourceKey);
  GetResourceKey(lpFavItem->lpRemoteName, SZ_FAVORITES, szResourceKey, &cSize);
  SetRegKey(HKEY_CURRENT_USER, szResourceKey, SZ_USERNAME, lpFavItem->lpUserName);
  SetRegKey(HKEY_CURRENT_USER, szResourceKey, SZ_USERPASS, lpFavItem->lpPassword);
}

BOOL ReadFavoriteItem(LPFAVORITEITEM lpFavItem)
{
  char resKey[MAX_PATH];
  int cData = sizeof(resKey);
  if(lpFavItem && GetResourceKey(lpFavItem->lpRemoteName, SZ_FAVORITES, resKey, &cData))
  {
    GetRegKey(HKEY_CURRENT_USER, resKey, SZ_USERNAME, lpFavItem->lpUserName,
      "", lpFavItem->ccUserName);
    GetRegKey(HKEY_CURRENT_USER, resKey, SZ_USERPASS, lpFavItem->lpPassword,
      "", lpFavItem->ccPassword);
    return TRUE;
  }
  return FALSE;
}

BOOL GetFavoritesParent(NETRESOURCE& SrcRes, LPNETRESOURCE lpParent)
{
  char* p;
  NETRESOURCE nr = {0};
  nr.lpProvider = szFavProv;
  nr.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
  if(!lstrcmp(SrcRes.lpProvider, szFavProv))
  {
    p = PointToName(SrcRes.lpRemoteName);
    if(p && p != SrcRes.lpRemoteName)
    {
      p--;
      *p = 0;
    }
    else
      return FALSE;
    if(lpParent)
    {
      if(strchr(SrcRes.lpRemoteName, '\\'))
        nr.lpRemoteName = SrcRes.lpRemoteName;
      NetResourceList::CopyNetResource(*lpParent, nr);
    }

    if(p)
      *p = '\\';
    return TRUE;
  }
  if(SrcRes.dwDisplayType == RESOURCEDISPLAYTYPE_SHARE)
    return FALSE;


  char szResourceKey[NM];
  int cSize = sizeof(szResourceKey);
  if(GetResourceKey(SrcRes.lpRemoteName, SZ_FAVORITES, szResourceKey, &cSize))
  {
    p = PointToName(szResourceKey);
    if(!p || !*p)
    {
      return FALSE;
    }
    if(lpParent)
    {
      *--p = 0;
      char res[NM] = {0};
      p = strchr(szResourceKey, '\\');
      if(p)
      {
        OemToChar(GetMsg(MFavorites), res);
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

STDAPI
EliminateSubKey( HKEY hkey, LPTSTR strSubKey );

BOOL GetFavoriteResource(char *SrcName, LPNETRESOURCE DstNetResource)
{
  NETRESOURCE nr = {0};
  char *p1, *p = SrcName;
  while(*p == '\\' || *p == '\\') p++;
  char szKey[NM] = {0};
  int cSize = sizeof(szKey);
  int dwKey = 0;
  if(GetResourceKey(p, SZ_FAVORITES, szKey, &cSize))
  {
    if(!lstrcmpi(szKey, SZ_FAVORITES))
    {
      if(DstNetResource)
      {
        nr.lpProvider = szFavProv;
        nr.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
        NetResourceList::CopyNetResource(*DstNetResource, nr);
      }
      return TRUE;
    }
    if(GetRegKey(HKEY_CURRENT_USER, szKey, NULL, (char*)&dwKey, "0", sizeof(dwKey))
      && dwKey == '1')
    {
      p = strchr(szKey, '\\');
      if(p)
      {
        if(DstNetResource)
        {
          nr.lpProvider = szFavProv;
          nr.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
          char szOutName[NM]; OemToChar(GetMsg(MFavorites), szOutName);
          lstrcat(szOutName, p);
          nr.lpRemoteName = szOutName;
          NetResourceList::CopyNetResource(*DstNetResource, nr);
        }

        return TRUE;
      }
    }
    else
    {
      p1 = PointToName(szKey);
      if(szKey != p1)
      {
        *(p1-1) = 0;
        if(GetRegKey(HKEY_CURRENT_USER, szKey, NULL, (char*)&dwKey, "0", sizeof(dwKey))
          && dwKey == '1')
        {
          p = p1;
        }
        else
        {
          p = PointToName(szKey);
          *(p1-1) = '\\';
          if(p != szKey)
          {
            *(p - 1) = 0;
            if(!GetRegKey(HKEY_CURRENT_USER, szKey, NULL, (char*)&dwKey, "0", sizeof(dwKey))
              || dwKey != '1')
              return FALSE;
          }
        }
        if(DstNetResource)
        {
          *(int*)&szKey[0] = 0x00005C5C; // "\\\\\x0\x0"
          lstrcat(szKey, p);
          nr.lpRemoteName = szKey;
          nr.dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
          NetResourceList::CopyNetResource(*DstNetResource, nr);
        }
        return TRUE;
      }
    }
  }
  return FALSE;
}

BOOL RemoveFromFavorites(char *SrcName, LPREMOVEFROMFAVCB /*pUserCallBack*/, LPVOID /*pUserData*/)
{
  char *p = strchr(SrcName, '\\');
  if(p)
    while(*++p == '\\');
    else
      p = SrcName;


    char szKey[MAX_PATH];

    FSF.sprintf(szKey, SZ_FAVORITES_SUBKEY, p);

    HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, "", MAXIMUM_ALLOWED);
    if(!hKey)
      return FALSE;

    BOOL res = SUCCEEDED(EliminateSubKey( hKey, szKey ));

    RegCloseKey(hKey);
    return res;
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

STDAPI
EliminateSubKey( HKEY hkey, LPTSTR strSubKey )
{
  HKEY hk;
  if (0 == lstrlen(strSubKey) ) {
    // defensive approach
    return E_FAIL;
  }

  LONG lreturn = RegOpenKeyEx( hkey
    , strSubKey
    , 0
    , MAXIMUM_ALLOWED
    , &hk );

  if( ERROR_SUCCESS == lreturn )
  {
    // Keep on enumerating the first (zero-th)
    // key and deleting that

    for( ; ; )
    {
      CHAR Buffer[MAX_PATH];
      DWORD dw = MAX_PATH;
      FILETIME ft;

      lreturn = RegEnumKeyEx( hk
        , 0
        , Buffer
        , &dw
        , NULL
        , NULL
        , NULL
        , &ft);

      if( ERROR_SUCCESS == lreturn )
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

BOOL RecursiveSetValue (HKEY hKey, char* lpSubKey)
{
  if(!lpSubKey || !*lpSubKey)
    return FALSE;
  char *p = strchr(lpSubKey, '\\');
  if(p)
  {
    *p = 0;
    while(*++p == '\\');
    if(*p)
    {
      HKEY hSubKey;
      if(ERROR_SUCCESS != RegCreateKeyEx(hKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL, &hSubKey, NULL))
        return FALSE;
      BOOL res = RecursiveSetValue(hSubKey, p);
      RegCloseKey(hSubKey);
      if(!res)
        return FALSE;
    }
  }
  return ERROR_SUCCESS == RegSetValue(hKey, lpSubKey, REG_SZ, "1", 2);
}

char *g_szInvalidChars="\\/";

BOOL ValidatePath(const char *szPath)
{
  if(!szPath||!*szPath)
    return FALSE;
  for(const char *p=szPath;*p;p++)
    for(char *p1=g_szInvalidChars;*p1;p1++)
      if(*p1==*p)
        return FALSE;
  return TRUE;
}

class Unknown
{
private:
  LONG m_refCount;
protected:
  virtual ~Unknown(){}
public:
  Unknown():m_refCount(1){}
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
    if(m_hKey)
      RegCloseKey(m_hKey);
  }
public:
  RegParser(): Unknown(), m_hKey(0), m_parent(0){}
  /**
   * @brief Function initialize the parser.
   *
   * It sets up the parser's internal m_hKey allowing to perform operations on it.
   */
  bool init(RegParser* parentItem, const char* subKey, bool createSubKey=false)
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
    if(createSubKey)
    {
      if(parentItem && parentItem->m_hKey)
      {
        DWORD dwDispositon;
        if (ERROR_SUCCESS == RegCreateKeyEx(parentItem->m_hKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hKey, &dwDispositon))
        {
          if (dwDispositon == REG_CREATED_NEW_KEY)
          {
            RegSetValue(m_hKey, NULL, REG_SZ, "1", 2);
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


  bool parsePath(char* path, bool createFolders, RegParser** ppResult)
  {
    RegParser* childItem;
    char* pRestOfPath;

    if (!path)
    {
      if (ppResult)
      {
        *ppResult = this;
        AddRef();
      }
      return true;
    }

    pRestOfPath = strchr(path, '\\');
    if (pRestOfPath)
      *pRestOfPath++ = '\0';

    bool res = true;
    if (!lstrcmp(path, ".."))
    {
      childItem = m_parent;
      if (childItem)
        childItem->AddRef();
      else
        res = false;
    }
    else if (!lstrcmp(path, "."))
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
      pRestOfPath[-1] = '\\';
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

BOOL CreateSubFolder(char *szRoot, char *szSubFolder)
{
  char szFavorites[] = SZ_FAVORITES;

  if (szRoot && !FSF.LStrnicmp(szRoot, szFavorites, lstrlen(szFavorites)))
  {
    szRoot = strchr(szRoot, '\\');
    if (szRoot)
      szRoot++;
  }

  if(!szSubFolder)
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
}
