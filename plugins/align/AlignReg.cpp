#include "Align.hpp"
#include "AlignReg.hpp"

HKEY CreateRegKey(const wchar_t *Key);
HKEY OpenRegKey(const wchar_t *Key);

void SetRegKey(const wchar_t *Key,const wchar_t *ValueName,DWORD ValueData)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  RegCloseKey(hKey);
}


void SetRegKey(const wchar_t *Key,const wchar_t *ValueName,wchar_t *ValueData)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_SZ,(CONST BYTE *)ValueData,(lstrlen(ValueData)+1)*sizeof(wchar_t));
  RegCloseKey(hKey);
}


int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,int &ValueData,DWORD Default)
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


int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,DWORD Default)
{
  int ValueData;
  GetRegKey(Key,ValueName,ValueData,Default);
  return(ValueData);
}


int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,wchar_t *ValueData,const wchar_t *Default,DWORD DataSize)
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

HKEY CreateRegKey(const wchar_t *Key)
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

HKEY OpenRegKey(const wchar_t *Key)
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
