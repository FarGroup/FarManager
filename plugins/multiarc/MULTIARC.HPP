#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <dir.h>
#include <time.h>
#include "d:\lang\bc5\far\plugin.hpp"
#include "marclng.hpp"
#include "formats\fmt.hpp"

#define MAX_COMMAND_LENGTH 32768

extern "C" {
void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info);
HANDLE WINAPI _export OpenFilePlugin(char *Name,const unsigned char *Data,unsigned int DataSize);
void WINAPI _export ClosePlugin(HANDLE hPlugin);
void WINAPI _export GetPluginInfo(struct PluginInfo *Info);
void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info);
int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
int WINAPI _export SetDirectory(HANDLE hPlugin,char *Dir,int OpMode);
int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
int WINAPI _export GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
int WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
int WINAPI _export ProcessHostFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
int WINAPI _export Configure(int ItemNumber);
void WINAPI _export ExitFAR();
}

typedef DWORD (WINAPI *PLUGINLOADFORMATMODULE)(char *ModuleName);
typedef BOOL (WINAPI *PLUGINISARCHIVE)(char *Name,const unsigned char *Data,int DataSize);
typedef BOOL (WINAPI *PLUGINOPENARCHIVE)(char *Name,int *Type);
typedef int (WINAPI *PLUGINGETARCITEM)(struct PluginPanelItem *Item,struct ArcItemInfo *Info);
typedef BOOL (WINAPI *PLUGINCLOSEARCHIVE)(struct ArcInfo *Info);
typedef BOOL (WINAPI *PLUGINGETFORMATNAME)(int Type,char *FormatName,char *DefaultExt);
typedef BOOL (WINAPI *PLUGINGETDEFAULTCOMMANDS)(int Type,int Command,char *Dest);

struct PluginItem
{
  char ModuleName[NM];
  HMODULE hModule;
  PLUGINLOADFORMATMODULE pLoadFormatModule;
  PLUGINISARCHIVE pIsArchive;
  PLUGINOPENARCHIVE pOpenArchive;
  PLUGINGETARCITEM pGetArcItem;
  PLUGINCLOSEARCHIVE pCloseArchive;
  PLUGINGETFORMATNAME pGetFormatName;
  PLUGINGETDEFAULTCOMMANDS pGetDefaultCommands;
};


class ArcPlugins
{
  private:
    struct PluginItem *PluginsData;
    int PluginsCount;
  public:
    ArcPlugins(char *ModuleName);
    ~ArcPlugins();
    int IsArchive(char *Name,const unsigned char *Data,int DataSize);
    BOOL OpenArchive(int PluginNumber,char *Name,int *Type);
    int GetArcItem(int PluginNumber,struct PluginPanelItem *Item,struct ArcItemInfo *Info);
    void CloseArchive(int PluginNumber,struct ArcInfo *Info);
    BOOL GetFormatName(int PluginNumber,int Type,char *FormatName,char *DefaultExt);
    BOOL GetDefaultCommands(int PluginNumber,int Type,int Command,char *Dest);
};


class PluginClass
{
  private:
    void GetCommandFormat(int Command,char *Format,int FormatSize);
    int GetPassword(char *Password,char *FileName);
    char ArcName[NM];
    char CurDir[NM];
    PluginPanelItem *ArcData;
    int ArcDataCount;
    WIN32_FIND_DATA ArcFindData;
    int ArcPluginNumber;
    int ArcPluginType;
    int LastTestState,LastWithoutPathsState;
    struct ArcItemInfo ItemsInfo;
    struct ArcInfo CurArcInfo;
    DWORD TotalSize;
    DWORD PackedSize;
    int DizPresent;
  public:
    PluginClass(int ArcPluginNumber);
    ~PluginClass();
    int ReadArchive(char *Name);
    int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
    void FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
    int SetDirectory(char *Dir,int OpMode);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    int DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    int ProcessHostFile(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    int GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                 int Move,char *DestPath,int OpMode);
    int PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                 int Move,int OpMode);
    int ProcessKey(int Key,unsigned int ControlState);
    static int SelectFormat(char *ArcFormat);
    static int FormatToPlugin(char *Format,int &PluginNumber,int &PluginType);
};


class ArcCommand
{
  private:
    int ProcessCommand(char *Command,int CommandType,int IgnoreErrors,int Silent);
    void DeleteBraces(char *Command);
    int ReplaceVar(char *Command,int &Length);
    int MakeListFile(char *ListFileName,int ShortNames,int QuoteName,
                     int UseSlash,int FolderName,int NameOnly,int PathOnly,
                     int FolderMask,char *LocalAllFilesMask,int AnsiCode);
    struct PluginPanelItem *PanelItem;
    int ItemsNumber;
    char ArcName[NM];
    char ArcDir[NM];
    char RealArcDir[NM];
    char Password[NM];
    char AllFilesMask[NM];
    char TempPath[NM];
    char NextFileName[NM];
    int NameNumber;
    int PrevFileNameNumber;
    char ListFileName[NM];
    unsigned int ExecCode;
    unsigned int MaxAllowedExitCode;
  public:
    ArcCommand(struct PluginPanelItem *PanelItem,int ItemsNumber,
               char *FormatString,char *ArcName,char *ArcDir,char *Password,
               char *AllFilesMask,int IgnoreErrors,int CommandType=0,
               int Silent=0,char *RealArcDir=NULL);
    int GetExecCode() {return(ExecCode);};
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

int ConfigGeneral();
int ConfigCommands();

char *GetMsg(int MsgId);
int Execute(HANDLE hPlugin,char *CmdStr,int HideOutput,int Silent,int ShowTitle);
void AddEndSlash(char *Path);
char* QuoteSpace(char *Str);
char* QuoteText(char *Str);
void ConvertNameToShort(char *Src,char *Dest);
void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                     int ItemsNumber);
char* PointToName(char *Path);
void InsertCommas(unsigned long Number,char *Dest);
int ToPercent(long N1,long N2);
int IsCaseMixed(char *Str);
int CheckForEsc();
int LocalStrnicmp(char *Str1,char *Str2,int Length);

void SetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData);
void SetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD ValueData);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD Default);

struct PluginStartupInfo Info;
class ArcPlugins *ArcPlugin=NULL;

enum {CMD_EXTRACT=0,CMD_EXTRACTWITHOUTPATH,CMD_TEST,CMD_DELETE,CMD_COMMENT,
      CMD_COMMENTFILES,CMD_SFX,CMD_LOCK,CMD_PROTECT,CMD_RECOVER,
      CMD_ADD,CMD_MOVE,CMD_ADDRECURSE,CMD_MOVERECURSE,CMD_ALLFILESMASK};

char *CmdNames[]={"Extract","ExtractWithoutPath","Test","Delete",
                  "Comment","CommentFiles","SFX","Lock","Protect","Recover",
                  "Add","Move","AddRecurse","MoveRecurse","AllFilesMask"};

#define F_ENCRYPTED 1

struct Options
{
  int HideOutput;
  int ProcessShiftF1;
  char DescriptionNames[NM];
  int ReadDescriptions;
  int UpdateDescriptions;
} Opt;

char PluginRootKey[80];
