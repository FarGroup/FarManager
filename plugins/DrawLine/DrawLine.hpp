extern "C"
{
  void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info);
  HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item);
  void WINAPI _export GetPluginInfo(struct PluginInfo *Info);
};

static struct PluginStartupInfo Info;

void ProcessShiftKey(int KeyCode,int LineWidth);
void GetEnvType(char *NewString,int StringLength,struct EditorInfo *ei,
                int &LeftLine,int &UpLine,int &RightLine,int &DownLine);
void SetTitle(int LineWidth);
char *GetMsg(int MsgId);
