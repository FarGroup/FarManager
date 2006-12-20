#ifndef __REG
#define __REG

HKEY CreateRegKey(HKEY hRoot, char *Key);
HKEY OpenRegKey(HKEY hRoot, char *Key);

void SetRegKey(HKEY hRoot, char *Key, char *ValueName, DWORD ValueData)
{
  HKEY hKey = CreateRegKey(hRoot, Key);

  RegSetValueEx(hKey, ValueName, 0, REG_DWORD, (BYTE *) & ValueData,
		sizeof(ValueData));
  RegCloseKey(hKey);
}


void SetRegKey(HKEY hRoot, char *Key, char *ValueName, char *ValueData)
{
  HKEY hKey = CreateRegKey(hRoot, Key);

  RegSetValueEx(hKey, ValueName, 0, REG_SZ, (CONST BYTE *) ValueData,
		strlen(ValueData) + 1);
  RegCloseKey(hKey);
}


int GetRegKey(HKEY hRoot, char *Key, char *ValueName, int &ValueData,
	      DWORD Default)
{
  HKEY hKey = OpenRegKey(hRoot, Key);
  DWORD Type, Size = sizeof(ValueData);
  int ExitCode =
    RegQueryValueEx(hKey, ValueName, 0, &Type, (BYTE *) & ValueData, &Size);
  RegCloseKey(hKey);
  if (hKey == NULL || ExitCode != ERROR_SUCCESS)
    {
      ValueData = Default;
      return (FALSE);
    }
  return (TRUE);
}


int GetRegKey(HKEY hRoot, char *Key, char *ValueName, DWORD Default)
{
  int ValueData;

  GetRegKey(hRoot, Key, ValueName, ValueData, Default);
  return (ValueData);
}


int GetRegKey(HKEY hRoot, char *Key, char *ValueName, char *ValueData,
	      char *Default, DWORD DataSize)
{
  HKEY hKey = OpenRegKey(hRoot, Key);
  DWORD Type;
  int ExitCode =
    RegQueryValueEx(hKey, ValueName, 0, &Type, (LPBYTE) ValueData,
		    &DataSize);

  RegCloseKey(hKey);
  if (hKey == NULL || ExitCode != ERROR_SUCCESS)
    {
      strcpy(ValueData, Default);
      return (FALSE);
    }
  return (TRUE);
}


HKEY CreateRegKey(HKEY hRoot, char *Key)
{
  HKEY hKey;
  DWORD Disposition;
  static char FullKeyName[512];

  g_FSF.sprintf(FullKeyName, REGStr.sss, PluginRootKey, *Key ? "\\" : "", Key);
  RegCreateKeyEx(hRoot, FullKeyName, 0, NULL, 0, KEY_WRITE, NULL,
		 &hKey, &Disposition);
  return (hKey);
}


HKEY OpenRegKey(HKEY hRoot, char *Key)
{
  HKEY hKey;
  static char FullKeyName[512];

  g_FSF.sprintf(FullKeyName, REGStr.sss, PluginRootKey, *Key ? "\\" : "", Key);
  if (RegOpenKeyEx(hRoot, FullKeyName, 0, KEY_QUERY_VALUE, &hKey) !=
      ERROR_SUCCESS)
    return (NULL);
  return (hKey);
}

void DeleteRegKey(HKEY hRoot, char *Key)
{
  static char FullKeyName[512];

  g_FSF.sprintf(FullKeyName, REGStr.sss, PluginRootKey, *Key ? "\\" : "", Key);
  RegDeleteKey(hRoot, FullKeyName);
}

#endif
