#ifndef __EMENU_HPP
#define __EMENU_HPP

extern "C"
{
  int WINAPI _export Configure(int ItemNumber);
  void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info);
  HANDLE WINAPI _export OpenPlugin(int OpenFrom, int Item);
  void WINAPI _export GetPluginInfo(struct PluginInfo *Info);
  void WINAPI _export ExitFAR();
};

struct PluginStartupInfo g_Info;
struct FarStandardFunctions g_FSF;
char PluginRootKey[80];
struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1, Y1, X2, Y2;
  unsigned char Focus;
  unsigned int Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  char *Data;
};
struct RegistryStr
{
  char *sss;
  char *WaitToContinue;
  char *UseGUI;
}
REGStr =
{
"%s%s%s", "WaitToContinue", "UseGUI"};

int UseGUI, WaitToContinue, IsOldFAR;

const char *GetMsg(int MsgId);
void InitDialogItems(struct InitDialogItem *Init, struct FarDialogItem *Item,
		     int ItemsNumber);
HKEY CreateRegKey(HKEY hRoot, char *Key);
HKEY OpenRegKey(HKEY hRoot, char *Key);
void SetRegKey(HKEY hRoot, char *Key, char *ValueName, DWORD ValueData);
int GetRegKey(HKEY hRoot, char *Key, char *ValueName, int &ValueData,
	      DWORD Default);
int GetRegKey(HKEY hRoot, char *Key, char *ValueName, DWORD Default);

#endif // __EMENU_HPP
