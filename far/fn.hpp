void _export StartFAR();
void Box(int x1,int y1,int x2,int y2,int Color,int Type);
void InitConsole();
void CloseConsole();
void SetFarConsoleMode();
void ChangeVideoMode(int NumLines,int NumColumns=80);
void GetVideoMode();
void GotoXY(int X,int Y);
int WhereX();
int WhereY();
void MoveCursor(int X,int Y);
void GetCursorPos(int& X,int& Y);
void SetCursorType(int Visible,int Size);
void GetCursorType(int &Visible,int &Size);
void MoveRealCursor(int X,int Y);
void GetRealCursorPos(int& X,int& Y);
void SetRealCursorType(int Visible,int Size);
void GetRealCursorType(int &Visible,int &Size);
void Text(char *Str);
void Text(int MsgId);
void VText(char *Str);
void HiText(char *Str,int HiColor);
void SetScreen(int X1,int Y1,int X2,int Y2,int Ch,int Color);
void MakeShadow(int X1,int Y1,int X2,int Y2);
void SetColor(int Color);
int GetColor();
void GetText(int X1,int Y1,int X2,int Y2,void *Dest);
void PutText(int X1,int Y1,int X2,int Y2,void *Src);
void GetRealText(int X1,int Y1,int X2,int Y2,void *Dest);
void PutRealText(int X1,int Y1,int X2,int Y2,void *Src);
void mprintf(char *fmt,...);
void mprintf(int MsgId,...);
void vmprintf(char *fmt,...);
int GetInputRecord(INPUT_RECORD *rec);
int PeekInputRecord(INPUT_RECORD *rec);
int CalcKeyCode(INPUT_RECORD *rec,int RealKey);
void WaitKey();
void WriteInput(int Key);
void ShowTime(int ShowAlways);
UDWORD NTTimeToDos(FILETIME *ft);
void ConvertDate(FILETIME *ft,char *DateText,char *TimeText,int TimeLength,
                 int Brief=FALSE,int TextMonth=FALSE,int FullYear=FALSE);
int GetDateFormat();
int GetDateSeparator();
int GetTimeSeparator();
int Execute(char *CmdStr,int AlwaysWaitFinish,int SeparateWindow=FALSE,
            int DirectRun=FALSE);
char* GetShellAction(char *FileName);
void ScrollScreen(int Count);
void ShellMakeDir(Panel *SrcPanel);
void ShellDelete(Panel *SrcPanel,int Wipe);
void ShellSetFileAttributes(Panel *SrcPanel);
void ShellOptions(int LastCommand,MOUSE_EVENT_RECORD *MouseEvent);
int ScreenSaver(int EnableExit);
void InsertCommas(unsigned long Number,char *Dest);
void InsertCommas(int64 li,char *Dest);
int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2=NULL,
            char *Str3=NULL,char *Str4=NULL);
int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2,
            char *Str3,char *Str4,char *Str5,char *Str6=NULL,char *Str7=NULL);
int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2,
            char *Str3,char *Str4,char *Str5,char *Str6,
            char *Str7,char *Str8,char *Str9=NULL,char *Str10=NULL);
int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2,
            char *Str3,char *Str4,char *Str5,char *Str6,char *Str7,
            char *Str8,char *Str9,char *Str10,char *Str11,char *Str12=NULL,
            char *Str13=NULL,char *Str14=NULL);
void SetMessageHelp(char *Topic);
void GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2);
int ToPercent(unsigned long N1,unsigned long N2);
int IsMouseButtonPressed();
char* PointToName(char *Path);
int CmpName(char *pattern,char *string,int skippath=TRUE);
int CheckForEsc();
void ShowHeap();
void CheckHeap(int NumLine=0);
char* QuoteSpace(char *Str);
char* QuoteSpaceOnly(char *Str);
char* TruncStr(char *Str,int MaxLength);
char* TruncPathStr(char *Str,int MaxLength);
void RemoveLeadingSpaces(unsigned char *Str);
void RemoveTrailingSpaces(unsigned char *Str);
int ProcessGlobalFileTypes(char *Name,int AlwaysWaitFinish);
int ProcessLocalFileTypes(char *Name,char *ShortName,int Mode,int AlwaysWaitFinish);
void ProcessExternal(char *Command,char *Name,char *ShortName,int AlwaysWaitFinish);
int SubstFileName(char *Str,char *Name,char *ShortName,
                  char *ListName=NULL,char *ShortListName=NULL,
                  int IgnoreInput=FALSE,char *CmdLineDir=NULL);
void EditFileTypes(int MenuPos);
void ProcessUserMenu(int EditMenu);
void ConvertNameToFull(char *Src,char *Dest);
void ConvertNameToShort(char *Src,char *Dest);
void ChangeConsoleMode(int Mode);
void FlushInputBuffer();
void SystemSettings();
void PanelSettings();
void InterfaceSettings();
void SetConfirmations();
void SetDizConfig();
void ViewerConfig();
void EditorConfig();
void SetFolderInfoFiles();
void ReadConfig();
void SaveConfig(int Ask);
void SetColors();
int GetColorDialog(unsigned int &Color);
int HiStrlen(char *Str);
long filelen(FILE *FPtr);
int GetErrorString(char *ErrStr);
void ShowProcessList();
int CopyToClipboard(char *Data);
int CopyFormatToClipboard(char *Format,char *Data);
char* PasteFromClipboard();
char* PasteFormatFromClipboard(char *Format);
int GetFileTypeByName(char *Name);
void SetFarTitle(char *Title);
int GetDirInfo(char *Title,char *DirName,unsigned long &DirCount,
               unsigned long &FileCount,int64 &FileSize,
               int64 &CompressedFileSize,int64 &RealSize,
               unsigned long &ClusterSize,clock_t MsgWaitTime,
               int EnhBreak);
int GetPluginDirInfo(HANDLE hPlugin,char *DirName,unsigned long &DirCount,
               unsigned long &FileCount,int64 &FileSize,
               int64 &CompressedFileSize);
void AddEndSlash(char *Path);
void LocalUpperInit();
int LocalIslower(int Ch);
int LocalIsupper(int Ch);
int LocalIsalpha(int Ch);
int LocalIsalphanum(int Ch);
int LocalUpper(int LowerChar);
void LocalUpperBuf(char *Buf,int Length);
int LocalLower(int UpperChar);
void LocalStrupr(char *s1);
void LocalStrlwr(char *s1);
int LocalStricmp(char *s1,char *s2);
int LocalStrnicmp(char *s1,char *s2,int n);
int LCStricmp(char *s1,char *s2);
int LocalKeyToKey(int Key);
int GetShortcutFolder(int Key,char *DestFolder,char *PluginModule,
                      char *PluginFile,char *PluginData);
int SaveFolderShortcut(int Key,char *SrcFolder,char *PluginModule,
                       char *PluginFile,char *PluginData);
void ShowFolderShortcut();
void ShowFilter();
int GetTable(struct CharTableSet *TableSet,int AnsiText,int &TableNum,
             int &UseUnicode);
void DecodeString(char *Str,unsigned char *DecodeTable,int Length=-1);
void EncodeString(char *Str,unsigned char *EncodeTable,int Length=-1);
int DetectTable(FILE *SrcFile,struct CharTableSet *TableSet,int &TableNum);
int PrepareTable(struct CharTableSet *TableSet,int TableNum);
void GetPathRoot(char *Path,char *Root);
void PrintFiles(Panel *SrcPanel);
char *NullToEmpty(char *Str);
void CenterStr(char *Src,char *Dest,int Length);
char *GetCommaWord(char *Src,char *Word);
int GetString(char *Title,char *SubTitle,char *HistoryName,char *SrcText,
    char *DestText,int DestLength,char *HelpTopic=NULL,int EnableEmpty=FALSE,
    int Password=FALSE);
void ScrollBar(int X1,int Y1,int Length,int Current,int Total);
int GetFileOwner(char *Computer,char *Name,char *Owner);
int GetNumberOfLinks(char *Name);
void ShowSeparator(int Length);
void UseSameRegKey();
void CloseSameRegKey();
void SetRegRootKey(HKEY hRootKey);
void SetRegKey(char *Key,char *ValueName,char *ValueData);
void SetRegKey(char *Key,char *ValueName,DWORD ValueData);
void SetRegKey(char *Key,char *ValueName,BYTE *ValueData,DWORD ValueSize);
int GetRegKey(char *Key,char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey(char *Key,char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(char *Key,char *ValueName,DWORD Default);
int GetRegKey(char *Key,char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);
HKEY CreateRegKey(char *Key);
HKEY OpenRegKey(char *Key);
void DeleteRegKey(char *Key);
void DeleteRegValue(char *Key,char *Value);
void DeleteKeyRecord(char *KeyMask,int Position);
void InsertKeyRecord(char *KeyMask,int Position,int TotalKeys);
void DeleteKeyTree(char *KeyName);
int CheckRegKey(char *Key);
int EnumRegKey(char *Key,DWORD Index,char *DestName,DWORD DestSize);
int IsFolderNotEmpty(char *Name);
void RemoveHighlights(char *Str);
int IsCaseMixed(char *Str);
int IsCaseLower(char *Str);
int DeleteFileWithFolder(char *FileName);
char* FarMSG(int MsgID);
BOOL GetDiskSize(char *Root,int64 *TotalSize,int64 *TotalFree,int64 *UserFree);
void DeleteDirTree(char *Dir);
int MkLink(char *Src,char *Dest);
int GetClusterSize(char *Root);
void _cdecl CheckVersion(void *Param);
void _cdecl ErrRegFn(void *Param);
void Register();
char ToHex(char Ch);
void InitDetectWindowedMode();
void DetectWindowedMode();
int IsWindowed();
void RestoreIcons();
void Log(char *fmt,...);
void Unquote(char *Str);
bool GetSubstName(char *LocalName,char *SubstName,int SubstSize);
void BoxText(char *Str);
int FarColorToReal(int FarColor);
void ReopenConsole();

ULARGE_INTEGER operator - (ULARGE_INTEGER &s1,unsigned int s2);
ULARGE_INTEGER operator + (ULARGE_INTEGER &s1,unsigned int s2);
ULARGE_INTEGER operator - (ULARGE_INTEGER &s1,ULARGE_INTEGER &s2);
ULARGE_INTEGER operator + (ULARGE_INTEGER &s1,ULARGE_INTEGER &s2);
ULARGE_INTEGER operator -= (ULARGE_INTEGER &s1,unsigned int s2);
ULARGE_INTEGER operator += (ULARGE_INTEGER &s1,unsigned int s2);
ULARGE_INTEGER operator -= (ULARGE_INTEGER &s1,ULARGE_INTEGER &s2);
ULARGE_INTEGER operator += (ULARGE_INTEGER &s1,ULARGE_INTEGER &s2);
unsigned int operator / (ULARGE_INTEGER d1,unsigned int d2);
ULARGE_INTEGER operator >> (ULARGE_INTEGER c1,unsigned int c2);
BOOL operator < (ULARGE_INTEGER &c1,int c2);
BOOL operator >= (ULARGE_INTEGER &c1,int c2);
BOOL operator >= (ULARGE_INTEGER &c1,ULARGE_INTEGER &c2);

