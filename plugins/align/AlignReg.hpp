void SetRegKey(const wchar_t *Key,const wchar_t *ValueName,DWORD ValueData);
void SetRegKey(const wchar_t *Key,const wchar_t *ValueName,wchar_t *ValueData);
int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,int &ValueData,DWORD Default);
int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,DWORD Default);
int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,wchar_t *ValueData,const wchar_t *Default,DWORD DataSize);
