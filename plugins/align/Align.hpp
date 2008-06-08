struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  DWORD_PTR Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  const char *Data;
};

void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize);
const char *GetMsg(int MsgId);
void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber);


static struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;
char PluginRootKey[80];
