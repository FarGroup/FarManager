#include "Align.hpp"
#include "AlignReg.hpp"

HKEY CreateRegKey(const TCHAR *Key);
HKEY OpenRegKey(const TCHAR *Key);

void SetRegKey(const TCHAR *Key,const TCHAR *ValueName,DWORD ValueData)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  RegCloseKey(hKey);
}


void SetRegKey(const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_SZ,(CONST BYTE *)ValueData,(lstrlen(ValueData)+1)*sizeof(TCHAR));
  RegCloseKey(hKey);
}


int GetRegKey(const TCHAR *Key,const TCHAR *ValueName,int &ValueData,DWORD Default)
{
  HKEY hKey=OpenRegKey(Key);
  DWORD Type,Size=sizeof(ValueData);
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE *)&ValueData,&Size);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    ValueData=Default;
    return(FALSE);
  }
  return(TRUE);
}


int GetRegKey(const TCHAR *Key,const TCHAR *ValueName,DWORD Default)
{
  int ValueData;
  GetRegKey(Key,ValueName,ValueData,Default);
  return(ValueData);
}


int GetRegKey(const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData,const TCHAR *Default,DWORD DataSize)
{
  HKEY hKey=OpenRegKey(Key);
  DWORD Type;
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(LPBYTE)ValueData,&DataSize);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    lstrcpy(ValueData,Default);
    return(FALSE);
  }
  return(TRUE);
}

HKEY CreateRegKey(const TCHAR *Key)
{
  HKEY hKey=NULL;
  if (RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,0,0,KEY_WRITE,0,&hKey,0)==ERROR_SUCCESS)
  {
    if (Key && *Key)
    {
      HKEY hSubKey=NULL;
      RegCreateKeyEx(hKey,Key,0,0,0,KEY_WRITE,0,&hSubKey,0);
      RegCloseKey(hKey);
      return hSubKey;
    }
  }
  return hKey;
}

HKEY OpenRegKey(const TCHAR *Key)
{
  HKEY hKey=NULL;
  if (RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
  {
    if (Key && *Key)
    {
      HKEY hSubKey=NULL;
      RegOpenKeyEx(hKey,Key,0,KEY_QUERY_VALUE,&hSubKey);
      RegCloseKey(hKey);
      return hSubKey;
    }
  }
  return hKey;
}
