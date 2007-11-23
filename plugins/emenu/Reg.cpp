#ifndef __REG
#define __REG
#include <windows.h>
#include <tchar.h>

static TCHAR g_PluginKey[256];

static HKEY CreateRegKey(void)
{
  HKEY hKey;
  DWORD Disposition;

  RegCreateKeyEx(HKEY_CURRENT_USER, g_PluginKey, 0, NULL, 0, KEY_WRITE, NULL,
                 &hKey, &Disposition);
  return (hKey);
}

static HKEY OpenRegKey(void)
{
  HKEY hKey;

  if (RegOpenKeyEx(HKEY_CURRENT_USER, g_PluginKey, 0, KEY_QUERY_VALUE, &hKey) !=
      ERROR_SUCCESS)
    return (NULL);
  return (hKey);
}

static void SetRegKey(const TCHAR *ValueName, DWORD ValueData)
{
  HKEY hKey = CreateRegKey();

  RegSetValueEx(hKey, ValueName, 0, REG_DWORD, (BYTE *) & ValueData,
                sizeof(ValueData));
  RegCloseKey(hKey);
}

static int GetRegKey(const TCHAR *ValueName, int &ValueData, DWORD Default)
{
  HKEY hKey = OpenRegKey();
  if(hKey != NULL) {
    DWORD Type, Size = sizeof(ValueData);
    int rc = RegQueryValueEx(hKey, ValueName, 0, &Type, (BYTE *) & ValueData, &Size);
    RegCloseKey(hKey);
    if(rc == ERROR_SUCCESS) return (TRUE);
  }
  ValueData = Default;
  return (FALSE);
}

static int GetRegKey(const TCHAR *ValueName, DWORD Default)
{
  int ValueData;

  GetRegKey(ValueName, ValueData, Default);
  return (ValueData);
}

#endif
