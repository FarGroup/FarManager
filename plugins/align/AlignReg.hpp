void SetRegKey(const TCHAR *Key,const TCHAR *ValueName,DWORD ValueData);
void SetRegKey(const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData);
int GetRegKey(const TCHAR *Key,const TCHAR *ValueName,int &ValueData,DWORD Default);
int GetRegKey(const TCHAR *Key,const TCHAR *ValueName,DWORD Default);
int GetRegKey(const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData,const TCHAR *Default,DWORD DataSize);
