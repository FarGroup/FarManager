extern "C"
{
  void   WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info);
  HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item);
  void   WINAPI _export GetPluginInfo(struct PluginInfo *Info);
};

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  char *Data;
};

void SetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData,char *Default,DWORD DataSize);
char *GetMsg(int MsgId);
void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                     int ItemsNumber);


static struct PluginStartupInfo Info;
char PluginRootKey[80];
