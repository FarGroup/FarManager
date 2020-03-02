#include "MultiArc.hpp"

HKEY CreateRegKey(HKEY hRoot,const char *Key);
HKEY OpenRegKey(HKEY hRoot,const char *Key);

void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData)
{
  HKEY hKey=CreateRegKey(hRoot,Key);
  RegSetValueEx(hKey,ValueName,0,REG_SZ,(BYTE*)ValueData,lstrlen(ValueData)+1);
  RegCloseKey(hKey);
}


void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD ValueData)
{
  HKEY hKey=CreateRegKey(hRoot,Key);
  RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
  RegCloseKey(hKey);
}


int GetRegKey(const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize)
{
  int Ret;
  if(0==(Ret=GetRegKey(HKEY_CURRENT_USER, Key,ValueName,ValueData,Default,DataSize)))
    Ret=GetRegKey(HKEY_LOCAL_MACHINE,Key,ValueName,ValueData,Default,DataSize);
  return Ret;
}


int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize)
{
  HKEY hKey=OpenRegKey(hRoot,Key);
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


int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,int &ValueData,DWORD Default)
{
  HKEY hKey=OpenRegKey(hRoot,Key);
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


int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD Default)
{
  int ValueData;
  GetRegKey(hRoot,Key,ValueName,ValueData,Default);
  return ValueData;
}

static char *CreateKeyName(char *FullKeyName, const char *Key)
{
  FSF.sprintf(FullKeyName,"%s%s%s",PluginRootKey,*Key ? "\\":"",Key);
  return FullKeyName;
}

HKEY CreateRegKey(HKEY hRoot,const char *Key)
{
  HKEY hKey;
  DWORD Disposition;
  char FullKeyName[512];
  RegCreateKeyEx(hRoot,CreateKeyName(FullKeyName,Key),0,NULL,0,KEY_WRITE,NULL,
                 &hKey,&Disposition);
  return hKey;
}


HKEY OpenRegKey(HKEY hRoot,const char *Key)
{
  HKEY hKey;
  char FullKeyName[512];
  if (RegOpenKeyEx(hRoot,CreateKeyName(FullKeyName,Key),0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
    return NULL;
  return hKey;
}

void DeleteRegKey(HKEY hRoot,const char *Key)
{
  char FullKeyName[512];
  RegDeleteKey(hRoot,CreateKeyName(FullKeyName,Key));
}


void DeleteRegValue(HKEY hRoot,const char *Key,const char *ValueName)
{
  HKEY hKey;
  char FullKeyName[512];
  if (RegOpenKeyEx(hRoot,CreateKeyName(FullKeyName,Key),0,KEY_WRITE,&hKey)==ERROR_SUCCESS)
  {
    RegDeleteValue(hKey,ValueName);
    RegCloseKey(hKey);
  }
}
