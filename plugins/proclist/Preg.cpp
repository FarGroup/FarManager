#include "proclist.hpp"
#include "proclng.hpp"

HKEY CreateRegKey(const char *Key);
HKEY OpenRegKey(const char *Key);

void SetRegKey(const char *Key,const char *ValueName,char *ValueData)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_SZ,(BYTE*)ValueData,lstrlen(ValueData)+1);
  RegCloseKey(hKey);
}


void SetRegKey(const char *Key,const char *ValueName,DWORD ValueData)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  RegCloseKey(hKey);
}


void SetRegKey(const char *Key,const char *ValueName,BYTE *ValueData,DWORD ValueSize)
{
  HKEY hKey=CreateRegKey(Key);
  RegSetValueEx(hKey,ValueName,0,REG_BINARY,ValueData,ValueSize);
  RegCloseKey(hKey);
}


int GetRegKey(const char *Key,const char *ValueName,char *ValueData,char *Default,DWORD DataSize)
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


int GetRegKey(const char *Key,const char *ValueName,int &ValueData,DWORD Default)
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


int GetRegKey(const char *Key,const char *ValueName,DWORD Default)
{
  int ValueData;
  GetRegKey(Key,ValueName,ValueData,Default);
  return ValueData;
}


int GetRegKey(const char *Key,const char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize)
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


void DeleteRegKey(char *Key)
{
  char FullKeyName[80];
  FSF.sprintf(FullKeyName,"%s%s%s",PluginRootKey,*Key ? "\\":"",Key);
  RegDeleteKey(HKEY_CURRENT_USER,FullKeyName);
}


HKEY CreateRegKey(const char *Key)
{
  HKEY hKey;
  char FullKeyName[MAX_PATH];
  if(Key && *Key)
    FSF.sprintf(FullKeyName,"%s\\%s",PluginRootKey,Key);
  else
    lstrcpy(FullKeyName, PluginRootKey);
  RegCreateKeyEx(HKEY_CURRENT_USER,FullKeyName,0,0,0,KEY_WRITE,0,&hKey,0);
  return hKey;
}


HKEY OpenRegKey(const char *Key)
{
  HKEY hKey;
  char FullKeyName[MAX_PATH];
  if(Key && *Key)
    FSF.sprintf(FullKeyName,"%s\\%s",PluginRootKey,Key);
  else
    lstrcpy(FullKeyName, PluginRootKey);
  if (RegOpenKeyEx(HKEY_CURRENT_USER,FullKeyName,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
    return NULL;
  return hKey;
}
