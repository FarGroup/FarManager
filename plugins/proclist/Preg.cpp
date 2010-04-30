#include "Proclist.hpp"
#include "Proclng.hpp"

static HKEY CreateRegKey(LPCTSTR Key);
static HKEY OpenRegKey(LPCTSTR Key);

void SetRegKey(LPCTSTR Key,LPCTSTR ValueName,LPCTSTR ValueData)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_SZ,(BYTE*)ValueData,(lstrlen(ValueData)+1)*sizeof(TCHAR));
  RegCloseKey(hKey);
}


void SetRegKey(LPCTSTR Key,LPCTSTR ValueName,DWORD ValueData)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  RegCloseKey(hKey);
}


void SetRegKey(LPCTSTR Key,LPCTSTR ValueName,BYTE *ValueData,DWORD ValueSize)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_BINARY,ValueData,ValueSize);
  RegCloseKey(hKey);
}


int GetRegKey(LPCTSTR Key,LPCTSTR ValueName,LPTSTR ValueData,LPCTSTR Default,DWORD DataSize)
{
  HKEY hKey=OpenRegKey(Key);
  DWORD Type;
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE*)ValueData,&DataSize);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    lstrcpy(ValueData,Default);
    return FALSE;
  }
  return TRUE;
}


int GetRegKey(LPCTSTR Key,LPCTSTR ValueName,int &ValueData,DWORD Default)
{
  HKEY hKey=OpenRegKey(Key);
  DWORD Type,Size=sizeof(ValueData);
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(BYTE *)&ValueData,&Size);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    ValueData=Default;
    return FALSE;
  }
  return TRUE;
}


int GetRegKey(LPCTSTR Key,LPCTSTR ValueName,DWORD Default)
{
  int ValueData;
  GetRegKey(Key,ValueName,ValueData,Default);
  return ValueData;
}


int GetRegKey(LPCTSTR Key,LPCTSTR ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize)
{
  HKEY hKey=OpenRegKey(Key);
  DWORD Type;
  int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,ValueData,&DataSize);
  RegCloseKey(hKey);
  if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
  {
    if (Default!=NULL)
      memcpy(ValueData,Default,DataSize);
    else
      memset(ValueData,0,DataSize);
    return FALSE;
  }
  return TRUE;
}


void DeleteRegKey(LPCTSTR Key)
{
  if (Key && *Key)
  {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
    {
      RegDeleteKey(hKey,Key);
      RegCloseKey(hKey);
    }
  }
  else
  {
    RegDeleteKey(HKEY_CURRENT_USER,PluginRootKey);
  }
}


static HKEY CreateRegKey(LPCTSTR Key)
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


static HKEY OpenRegKey(LPCTSTR Key)
{
  HKEY hKey=NULL;
  if (RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
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
