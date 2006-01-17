#ifndef __NETREG_HPP__
#define __NETREG_HPP__
#include <windows.h>

HKEY CreateRegKey(HKEY hRoot,const char *Key);
HKEY OpenRegKey(HKEY hRoot,const char *Key);

void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData);
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,BYTE *ValueData,DWORD ValueSize);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);
HKEY CreateRegKey(HKEY hRoot,const char *Key);
HKEY OpenRegKey(HKEY hRoot, const char *Key, REGSAM samDesired);
HKEY OpenRegKey(HKEY hRoot,const char *Key);


#endif //! __NETREG_HPP__