extern "C"
{
  void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info);
  HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item);
  void WINAPI _export GetPluginInfo(struct PluginInfo *Info);
};

static struct PluginStartupInfo Info;

char *GetMsg(int MsgId);

