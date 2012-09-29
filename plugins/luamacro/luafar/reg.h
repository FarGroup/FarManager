#include <windows.h>

HKEY CreateRegKey(HKEY hRoot, wchar_t *Key);
HKEY OpenRegKey(HKEY hRoot, wchar_t *Key);

void SetRegKeyStr(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, wchar_t *ValueData);
void SetRegKeyDword(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, DWORD ValueData);
void SetRegKeyArr(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, BYTE *ValueData, DWORD ValueSize);

int GetRegKeyStr(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, wchar_t *ValueData, wchar_t *Default, DWORD DataSize);
int GetRegKeyInt(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, int *ValueData, DWORD Default);
int GetRegKeyDword(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, DWORD Default);
int GetRegKeyArr(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, BYTE *ValueData, BYTE *Default, DWORD DataSize);

void DeleteRegKey(HKEY hRoot, wchar_t *Key);
