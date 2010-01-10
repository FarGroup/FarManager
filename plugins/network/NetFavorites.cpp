#include "NetFavorites.hpp"
#include "NetCommon.hpp"
#include "NetReg.hpp"

#define SZ_FAVORITES          _T("Favorites")
#define SZ_FAVORITES_SUBKEY   SZ_FAVORITES _T("\\%s")
#define SZ_USERNAME           _T("UserName")
#define SZ_USERPASS           _T("Password")

TCHAR szFavProv[] = _T("Far Favorites Provider");

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
    TCHAR szKey[MAX_PATH*2];
    szKey[0] = 0;
    if(pNR->lpRemoteName)
    {
      TCHAR *p = _tcschr(pNR->lpRemoteName, _T('\\'));
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
      TCHAR szSubKey[MAX_PATH];
      szSubKey[0] = 0;
      for(DWORD dwIndex = 0;
          ERROR_SUCCESS == RegEnumKey(hKey,dwIndex,szSubKey,ArraySize(szSubKey));
          dwIndex++)
      {
        int bTmp; LONG cData = sizeof(bTmp);
        bTmp = 0;
        TCHAR szSrc[MAX_PATH];
        szSrc[0] = 0;

        if(ERROR_SUCCESS == RegQueryValue(hKey, szSubKey, (TCHAR*)&bTmp, &cData) && bTmp == (int)_T('1'))
        {
          tmp.lpProvider = szFavProv;
          tmp.lpRemoteName = szSrc;
          tmp.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
          if(pNR->lpRemoteName)
            lstrcpy(szSrc, pNR->lpRemoteName);
          else
            OEMToChar(GetMsg(MFavorites), szSrc);
          lstrcat(szSrc, _T("\\"));
          lstrcat(szSrc, szSubKey);
          pList->Push(tmp);
        }
        else
        {
          szSrc[0] = _T('\\');
          szSrc[1] = _T('\\');
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
  return pNR && !lstrcmp(pNR->lpProvider, szFavProv);
}

BOOL GetResourceKey(TCHAR* lpRemoteName, const TCHAR* rootKey, TCHAR* lpResourceKey, size_t *cSize)
{
  // We should be sure that "Favorites" is a folder
  SetRegKey(HKEY_CURRENT_USER, SZ_FAVORITES, NULL, _T("1"));
  if(!lpResourceKey || !cSize || !*cSize)
    return FALSE;

  TCHAR *p = NULL;
  TCHAR szKey[MAX_PATH];
  if(lpRemoteName)
  {
    while (*lpRemoteName==_T('\\')) lpRemoteName++;
    if(0 != (p = _tcschr(lpRemoteName, _T('\\'))))
      *p = 0;
    TCHAR szFavoritesName[MAX_PATH];
    OEMToChar(GetMsg(MFavorites), szFavoritesName);
    if(!lstrcmpi(lpRemoteName, szFavoritesName))
    {
      if(p) *p = _T('\\'), p++;
      if(p)
        FSF.sprintf(szKey, SZ_FAVORITES_SUBKEY, p);
      else
        lstrcpy(szKey, SZ_FAVORITES);
      HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, szKey);
      if(hKey)
      {
        RegCloseKey(hKey);
        lstrcpyn(lpResourceKey, szKey, (int)*cSize);
        return TRUE;
      }
    }
    else
      if(p) *p = _T('\\');
  }
  if(!lpRemoteName)
  {
    if(*cSize < (size_t)(lstrlen(rootKey)+1))
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
  TCHAR* buff = new TCHAR[buffLen];

  p = _tcschr(lpRemoteName, _T('\\'));
  if(p)
    *p = 0;

  FSF.sprintf(buff, _T("%s\\%s"), rootKey, lpRemoteName);

  if(p)
    *p = _T('\\');
  BOOL res = TRUE;
  HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, buff);
  if(hKey)
  {
    RegCloseKey(hKey);
    FSF.sprintf(lpResourceKey, _T("%s\\%s"), rootKey, lpRemoteName);

    *cSize = lstrlen(lpResourceKey);
  }
  else
  {
    res = FALSE;
    if(0 != (hKey = OpenRegKey(HKEY_CURRENT_USER, rootKey,
      KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS)))
    {
      for(DWORD dwIndex = 0;
          ERROR_SUCCESS == RegEnumKey(hKey, dwIndex, szKey, ArraySize(szKey));
          dwIndex++)
      {
        int bTmp = 0;
        LONG cData = sizeof(bTmp);
        if(ERROR_SUCCESS == RegQueryValue(hKey, szKey, (TCHAR*)&bTmp, &cData) && bTmp == (int)_T('1'))
        {
          FSF.sprintf(buff, _T("%s\\%s"), rootKey, szKey);
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
    FSF.sprintf(lpResourceKey, _T("%s\\%s"), rootKey, lpRemoteName);
  }

  delete(buff);
  return res;
}

void WriteFavoriteItem(LPFAVORITEITEM lpFavItem, TCHAR* /*szFolder*/)
{
  TCHAR szResourceKey[MAX_PATH];
  size_t cSize = ArraySize(szResourceKey);
  GetResourceKey(lpFavItem->lpRemoteName, SZ_FAVORITES, szResourceKey, &cSize);
  SetRegKey(HKEY_CURRENT_USER, szResourceKey, SZ_USERNAME, lpFavItem->lpUserName);
  SetRegKey(HKEY_CURRENT_USER, szResourceKey, SZ_USERPASS, lpFavItem->lpPassword);
}

BOOL ReadFavoriteItem(LPFAVORITEITEM lpFavItem)
{
  TCHAR resKey[MAX_PATH];
  size_t cData = ArraySize(resKey);
  if(lpFavItem && GetResourceKey(lpFavItem->lpRemoteName, SZ_FAVORITES, resKey, &cData))
  {
    GetRegKey(HKEY_CURRENT_USER, resKey, SZ_USERNAME, lpFavItem->lpUserName,
      _T(""), lpFavItem->ccUserName);
    GetRegKey(HKEY_CURRENT_USER, resKey, SZ_USERPASS, lpFavItem->lpPassword,
      _T(""), lpFavItem->ccPassword);
    return TRUE;
  }
  return FALSE;
}

BOOL GetFavoritesParent(NETRESOURCE& SrcRes, LPNETRESOURCE lpParent)
{
  TCHAR* p;
  NETRESOURCE nr = {0};
  nr.lpProvider = szFavProv;
  nr.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
  if(!lstrcmp(SrcRes.lpProvider, szFavProv))
  {
    p = (TCHAR*)PointToName(SrcRes.lpRemoteName);
    if(p && p != SrcRes.lpRemoteName)
    {
      p--;
      *p = 0;
    }
    else
      return FALSE;
    if(lpParent)
    {
      if(_tcschr(SrcRes.lpRemoteName, _T('\\')))
        nr.lpRemoteName = SrcRes.lpRemoteName;
      NetResourceList::CopyNetResource(*lpParent, nr);
    }

    if(p)
      *p = _T('\\');
    return TRUE;
  }
  if(SrcRes.dwDisplayType == RESOURCEDISPLAYTYPE_SHARE)
    return FALSE;


  TCHAR szResourceKey[MAX_PATH];
  size_t cSize = ArraySize(szResourceKey);
  if(GetResourceKey(SrcRes.lpRemoteName, SZ_FAVORITES, szResourceKey, &cSize))
  {
    p = (TCHAR*)PointToName(szResourceKey);
    if(!p || !*p)
    {
      return FALSE;
    }
    if(lpParent)
    {
      *--p = 0;
      TCHAR res[MAX_PATH];
      res[0] = 0;
      p = _tcschr(szResourceKey, _T('\\'));
      if(p)
      {
        OEMToChar(GetMsg(MFavorites), res);
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

BOOL GetFavoriteResource(TCHAR *SrcName, LPNETRESOURCE DstNetResource)
{
  NETRESOURCE nr = {0};
  TCHAR *p1, *p = SrcName;
  while(*p == _T('\\')) ++p;
  TCHAR szKey[MAX_PATH];
  size_t cSize = ArraySize(szKey);
  int dwKey = 0;
  szKey[0] = 0;
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
    if(GetRegKey(HKEY_CURRENT_USER, szKey, NULL, (TCHAR*)&dwKey, _T("0"), sizeof(dwKey))
      && dwKey == _T('1'))
    {
      p = _tcschr(szKey, _T('\\'));
      if(p)
      {
        if(DstNetResource)
        {
          nr.lpProvider = szFavProv;
          nr.dwDisplayType = RESOURCEDISPLAYTYPE_DOMAIN;
          TCHAR szOutName[MAX_PATH];
          OEMToChar(GetMsg(MFavorites), szOutName);
          lstrcat(szOutName, p);
          nr.lpRemoteName = szOutName;
          NetResourceList::CopyNetResource(*DstNetResource, nr);
        }

        return TRUE;
      }
    }
    else
    {
      p1 = (TCHAR*)PointToName(szKey);
      if(szKey != p1)
      {
        *(p1-1) = 0;
        if(GetRegKey(HKEY_CURRENT_USER, szKey, NULL, (TCHAR*)&dwKey, _T("0"), sizeof(dwKey))
          && dwKey == _T('1'))
        {
          p = p1;
        }
        else
        {
          p = (TCHAR*)PointToName(szKey);
          *(p1-1) = _T('\\');
          if(p != szKey)
          {
            *(p - 1) = 0;
            if(!GetRegKey(HKEY_CURRENT_USER, szKey, NULL, (TCHAR*)&dwKey, _T("0"), sizeof(dwKey))
              || dwKey != _T('1'))
              return FALSE;
          }
        }
        if(DstNetResource)
        {
          lstrcpy(szKey, _T("\\\\"));
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

BOOL RemoveFromFavorites(TCHAR *SrcName, LPREMOVEFROMFAVCB /*pUserCallBack*/, LPVOID /*pUserData*/)
{
  TCHAR *p = _tcschr(SrcName, _T('\\'));
  if(p)
    while(*++p == _T('\\'));
    else
      p = SrcName;


    TCHAR szKey[MAX_PATH];

    FSF.sprintf(szKey, SZ_FAVORITES_SUBKEY, p);

    HKEY hKey = OpenRegKey(HKEY_CURRENT_USER, _T(""), MAXIMUM_ALLOWED);
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
      TCHAR Buffer[MAX_PATH];
      DWORD dw = ArraySize(Buffer);
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

BOOL RecursiveSetValue (HKEY hKey, TCHAR* lpSubKey)
{
  if(!lpSubKey || !*lpSubKey)
    return FALSE;
  TCHAR *p = _tcschr(lpSubKey, _T('\\'));
  if(p)
  {
    *p = 0;
    while(*++p == _T('\\'));
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
  return ERROR_SUCCESS == RegSetValue(hKey, lpSubKey, REG_SZ, _T("1"), 2);
}

const TCHAR *g_szInvalidChars=_T("\\/");

BOOL ValidatePath(const TCHAR *szPath)
{
  if(!szPath||!*szPath)
    return FALSE;
  for(const TCHAR *p=szPath;*p;p++)
    for(const TCHAR *p1=g_szInvalidChars;*p1;p1++)
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
  bool init(RegParser* parentItem, const TCHAR* subKey, bool createSubKey=false)
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
            RegSetValue(m_hKey, NULL, REG_SZ, _T("1"), 2);
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


  bool parsePath(TCHAR* path, bool createFolders, RegParser** ppResult)
  {
    RegParser* childItem;
    TCHAR* pRestOfPath;

    if (!path)
    {
      if (ppResult)
      {
        *ppResult = this;
        AddRef();
      }
      return true;
    }

    pRestOfPath = _tcschr(path, _T('\\'));
    if (pRestOfPath)
      *pRestOfPath++ = _T('\0');

    bool res = true;
    if (!lstrcmp(path, _T("..")))
    {
      childItem = m_parent;
      if (childItem)
        childItem->AddRef();
      else
        res = false;
    }
    else if (!lstrcmp(path, _T(".")))
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
      pRestOfPath[-1] = _T('\\');
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

BOOL CreateSubFolder(TCHAR *szRoot, TCHAR *szSubFolder)
{
  TCHAR szFavorites[] = SZ_FAVORITES;

  if (szRoot && !FSF.LStrnicmp(szRoot, szFavorites, lstrlen(szFavorites)))
  {
    szRoot = _tcschr(szRoot, _T('\\'));
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
