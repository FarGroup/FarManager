#ifndef __REG
#define __REG

HKEY CreateRegKey(HKEY hRoot,const TCHAR *Key);
HKEY OpenRegKey(HKEY hRoot,const TCHAR *Key);

static TCHAR FullKeyName[512];

void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,BYTE *ValueData,DWORD ValueSize)
{
	HKEY hKey=CreateRegKey(hRoot, Key);
	RegSetValueEx(hKey,ValueName,0,REG_BINARY,ValueData,ValueSize);
	RegCloseKey(hKey);
}

void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD ValueData)
{
	HKEY hKey=CreateRegKey(hRoot,Key);
	RegSetValueEx(hKey,ValueName,0,REG_DWORD,(BYTE *)&ValueData,sizeof(ValueData));
	RegCloseKey(hKey);
}


void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData)
{
	HKEY hKey=CreateRegKey(hRoot,Key);
	RegSetValueEx(hKey,ValueName,0,REG_SZ,(CONST BYTE *)ValueData,(lstrlen(ValueData)+1)*sizeof(TCHAR));
	RegCloseKey(hKey);
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


int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData,const TCHAR *Default,DWORD DataSize)
{
	HKEY hKey=OpenRegKey(hRoot,Key);
	DWORD Type;
	DataSize *= sizeof(TCHAR);
	int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,(LPBYTE)ValueData,&DataSize);
	RegCloseKey(hKey);

	if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
	{
		lstrcpy(ValueData,Default);
		return(FALSE);
	}
	else
		ValueData[DataSize] = 0;

	return(TRUE);
}

int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize)
{
	HKEY hKey=OpenRegKey(hRoot,Key);
	DWORD Type,Required=DataSize;
	int ExitCode=RegQueryValueEx(hKey,ValueName,0,&Type,ValueData,&Required);
	RegCloseKey(hKey);

	if (hKey==NULL || ExitCode!=ERROR_SUCCESS)
	{
		if (Default!=NULL)
			memcpy(ValueData,Default,DataSize);
		else
			memset(ValueData,0,DataSize);

		return(FALSE);
	}

	return(DataSize);
}


HKEY CreateRegKey(HKEY hRoot,const TCHAR *Key)
{
	HKEY hKey;
	DWORD Disposition;
	FarSprintf(FullKeyName,REGStr.sss,PluginRootKey,*Key ? _T("\\"):_T(""),Key);
	RegCreateKeyEx(hRoot,FullKeyName,0,NULL,0,KEY_WRITE,NULL,
	               &hKey,&Disposition);
	return(hKey);
}


HKEY OpenRegKey(HKEY hRoot,const TCHAR *Key)
{
	HKEY hKey;
	FarSprintf(FullKeyName,REGStr.sss,PluginRootKey,*Key ? _T("\\"):_T(""),Key);

	if (RegOpenKeyEx(hRoot,FullKeyName,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
		return(NULL);

	return(hKey);
}

void DeleteRegKey(HKEY hRoot,const TCHAR *Key)
{
	FarSprintf(FullKeyName,REGStr.sss,PluginRootKey,*Key ? _T("\\"):_T(""),Key);
	RegDeleteKey(hRoot,FullKeyName);
}

#endif
