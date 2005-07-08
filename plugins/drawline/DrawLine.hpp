static struct PluginStartupInfo Info;

void ProcessShiftKey(int KeyCode,int LineWidth);
void GetEnvType(char *NewString,int StringLength,struct EditorInfo *ei,
                int &LeftLine,int &UpLine,int &RightLine,int &DownLine);
void SetTitle(int LineWidth,int IDTitle);
const char *GetMsg(int MsgId);
