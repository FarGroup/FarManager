#include "NetReg.hpp"
#include "NetCommon.hpp"

HKEY CreateRegKey(HKEY hRoot,const TCHAR *Key);
HKEY OpenRegKey(HKEY hRoot,const TCHAR *Key);

void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,const TCHAR *ValueData)
{
  HKEY hKey=CreateRegKey(hRoot,Key);
  RegSetValueEx(hKey,ValueName,0,REG_SZ,(BYTE*)ValueData,(lstrlen(ValueData)+1)*sizeof(TCHAR));
  RegCloseKey(hKey);
}


void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD ValueData)
{
  HKEY hKey=CreateRegKey(hRoot,Key);
  RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  RegCloseKey(hKey);
}


void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,BYTE *ValueData,DWORD ValueSize)
{
  HKEY hKey=CreateRegKey(hRoot,Key);
  RegSetValueEx(hKey,ValueName,0,REG_BINARY,ValueData,ValueSize);
  RegCloseKey(hKey);
}


int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData,const TCHAR *Default,DWORD DataSize)
{
  HKEY hKey=OpenRegKey(hRoot,Key);
  DWORD Type;
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE*)ValueData,&DataSize);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    lstrcpy(ValueData,Default);
    return(FALSE);
  }
  return(TRUE);
}


int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,int &ValueData,DWORD Default)
{
  HKEY hKey=OpenRegKey(hRoot,Key);
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


int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD Default)
{
  int ValueData;
  GetRegKey(hRoot,Key,ValueName,ValueData,Default);
  return(ValueData);
}


int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize)
{
  HKEY hKey=OpenRegKey(hRoot,Key);
  DWORD Type;
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,ValueData,&DataSize);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    if (Default!=NULL)
      memcpy(ValueData,Default,DataSize);
    else
      memset(ValueData,0,DataSize);
    return(FALSE);
  }
  return(TRUE);
}


HKEY CreateRegKey(HKEY hRoot,const TCHAR *Key)
{
  HKEY hKey=NULL;
  if (RegCreateKeyEx(hRoot,PluginRootKey,0,0,0,KEY_WRITE,0,&hKey,0)==ERROR_SUCCESS)
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

HKEY OpenRegKey(HKEY hRoot, const TCHAR *Key, REGSAM samDesired)
{
  HKEY hKey=NULL;
  if (RegOpenKeyEx(hRoot,PluginRootKey,0,samDesired,&hKey)!=ERROR_SUCCESS)
  {
    if (Key && *Key)
    {
      HKEY hSubKey=NULL;
      RegOpenKeyEx(hKey,Key,0,samDesired,&hSubKey);
      RegCloseKey(hKey);
      return hSubKey;
    }
  }
  return hKey;
}

HKEY OpenRegKey(HKEY hRoot,const TCHAR *Key)
{
  return OpenRegKey(hRoot, Key, KEY_QUERY_VALUE);
}
