#include <windows.h>

HKEY CreateRegKey(HKEY hRoot, wchar_t *Key, REGSAM samDesired);
HKEY OpenRegKey(HKEY hRoot, wchar_t *Key, REGSAM samDesired);

BOOL SetRegKeyStr(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, wchar_t *ValueData, REGSAM samDesired);
BOOL SetRegKeyDword(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, DWORD ValueData, REGSAM samDesired);
BOOL SetRegKeyArr(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, BYTE *ValueData, DWORD ValueSize, REGSAM samDesired);

int GetRegKeyStr(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, wchar_t *ValueData, wchar_t *Default, DWORD DataSize, REGSAM samDesired);
int GetRegKeyInt(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, int *ValueData, DWORD Default, REGSAM samDesired);
int GetRegKeyArr(HKEY hRoot, wchar_t *Key, wchar_t *ValueName, BYTE *ValueData, BYTE *Default, DWORD DataSize, REGSAM samDesired);
