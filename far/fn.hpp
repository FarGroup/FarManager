#ifndef __FARFUNC_HPP__
#define __FARFUNC_HPP__
/*
fn.hpp

Описания функций

*/

#include "farconst.hpp"
#include "global.hpp"
#include "filefilter.hpp"

/* $ 07.07.2000 IS
   Функция перешла сюда из main.cpp
*/
void SetHighlighting();
/* IS $ */
void _export StartFAR();
void Box(int x1,int y1,int x2,int y2,int Color,int Type);
/*$ 14.02.2001 SKV
  Инитить ли палитру default значениями.
  По умолчанию - да.
  С 0 используется для ConsoleDetach.
*/
void InitConsole(int FirstInit=TRUE);
void InitRecodeOutTable(UINT cp=0);
/* SKV$*/
void CloseConsole();
void SetFarConsoleMode(BOOL SetsActiveBuffer=FALSE);
//OT
void ChangeVideoMode(int NumLines,int NumColumns);
void ChangeVideoMode(int Maximized);
void SetVideoMode(int ConsoleMode);
void GetVideoMode(CONSOLE_SCREEN_BUFFER_INFO &csbi);
//OT
void GotoXY(int X,int Y);
int WhereX();
int WhereY();
void MoveCursor(int X,int Y);
void GetCursorPos(int& X,int& Y);
void SetCursorType(int Visible,int Size);
void SetInitialCursorType();
void GetCursorType(int &Visible,int &Size);
void MoveRealCursor(int X,int Y);
void GetRealCursorPos(int& X,int& Y);
void SetRealCursorType(int Visible,int Size);
void GetRealCursorType(int &Visible,int &Size);
void Text(int X, int Y, int Color, const char *Str);
void Text(const char *Str);
#if defined(USE_WFUNC)
void TextW(int X, int Y, int Color, const WCHAR *Str);
void TextW(const WCHAR *Str);
#endif
void Text(int X, int Y, int Color, int MsgId);
void Text(int MsgId);
void VText(const char *Str);
#if defined(USE_WFUNC)
void VTextW(const WCHAR *Str);
#endif
void HiText(const char *Str,int HiColor,int isVertText=0);
#define HiVText(Str,HiColor) HiText(Str,HiColor,1)
#if defined(USE_WFUNC)
void HiTextW(const WCHAR *Str,int HiColor,int isVertText=0);
#define HiVTextW(Str,HiColor) HiTextW(Str,HiColor,1)
#endif

void DrawLine(int Length,int Type);
#define ShowSeparator(Length,Type) DrawLine(Length,Type)
#define ShowUserSeparator(Length,Type,BoxSymbols) DrawLine(Length,Type)

char* MakeSeparator(int Length,char *DestStr,int Type=1);
#if defined(USE_WFUNC)
WCHAR* MakeSeparatorW(int Length,WCHAR *DestStr,int Type=1);
#endif
void SetScreen(int X1,int Y1,int X2,int Y2,int Ch,int Color);
void MakeShadow(int X1,int Y1,int X2,int Y2);
void ChangeBlockColor(int X1,int Y1,int X2,int Y2,int Color);
void SetColor(int Color);
void SetRealColor(int Color);
void ClearScreen(int Color);
int  GetColor();
void GetText(int X1,int Y1,int X2,int Y2,void *Dest,int DestSize);
void PutText(int X1,int Y1,int X2,int Y2,const void *Src);
#if defined(USE_WFUNC)
void PutTextA(int X1,int Y1,int X2,int Y2,const void *Src);
#endif
void GetRealText(int X1,int Y1,int X2,int Y2,void *Dest);
void PutRealText(int X1,int Y1,int X2,int Y2,const void *Src);
void _GetRealText(HANDLE hConsoleOutput,int X1,int Y1,int X2,int Y2,const void *Src,int BufX,int BufY);
void _PutRealText(HANDLE hConsoleOutput,int X1,int Y1,int X2,int Y2,const void *Src,int BufX,int BufY);

#if defined(USE_WFUNC)
void mprintfW(CHAR *fmt,...);
void vmprintfW(CHAR *fmt,...);
#endif
void mprintf(char *fmt,...);
void mprintf(int MsgId,...);
void vmprintf(char *fmt,...);

#if defined(USE_WFUNC)
WORD GetVidCharW(CHAR_INFO CI);
inline WORD GetVidChar(CHAR_INFO CI)
{
  if(Opt.UseUnicodeConsole)
    return GetVidCharW(CI);
  return CI.Char.AsciiChar;
}

inline void SetVidChar(CHAR_INFO& CI,WORD Chr)
{
  extern WCHAR Oem2Unicode[];
  extern BYTE RecodeOutTable[];

  if(Opt.UseUnicodeConsole)
    CI.Char.UnicodeChar = Oem2Unicode[Chr];
  else
    CI.Char.AsciiChar=RecodeOutTable[Chr];
}

#else
#define GetVidChar(CI)     (CI).Char.AsciiChar
#define SetVidChar(CI,Chr) (CI).Char.AsciiChar=Chr
#endif


void ShowTime(int ShowAlways);
int GetDateFormat();
int GetDateSeparator();
int GetTimeSeparator();
char* GetShellAction(const char *FileName,DWORD& ImageSubsystem,DWORD& Error);
void ScrollScreen(int Count);
int ScreenSaver(int EnableExit);
char *FormatNumber(const char *Src, char *Dest, int Size, int NumDigits=0);
char* InsertCommas(const unsigned long &Number,char *Dest,int Size);
char* InsertCommas(const unsigned __int64 &li,char *Dest,int Size);
void DeleteDirTree(const char *Dir);
int GetClusterSize(char *Root);

void __cdecl CheckVersion(void *Param);
void __cdecl ErrRegFn(void *Param);
void __cdecl CheckReg(void *Param);
void Register();

char ToHex(char Ch);
void InitDetectWindowedMode();
void DetectWindowedMode();
int IsWindowed();
void RestoreIcons();
void Log(char *fmt,...);
void BoxText(WORD Chr);
void BoxText(char *Str,int IsVert=0);
#if defined(USE_WFUNC)
void BoxTextW(WCHAR *Str,int IsVert=0);
void BoxTextW2(const char *Str,int IsVert);
#endif
int FarColorToReal(int FarColor);
void ConvertCurrentPalette();
void ReopenConsole();
char *RemoveChar(char *Str,char Target,BOOL Dup=TRUE);
char *InsertString(char *Str,int Pos,const char *InsStr,int InsSize=0);
int ReplaceStrings(char *Str,const char *FindStr,const char *ReplStr,int Count=-1,BOOL IgnoreCase=FALSE);
#define RemoveHighlights(Str) RemoveChar(Str,'&')
int IsCaseMixed(char *Str);
int IsCaseLower(char *Str);
int DeleteFileWithFolder(const char *FileName);


/* $ 26.01.2003 IS
    + FAR_DeleteFile, FAR_RemoveDirectory просьба только их использовать
      для удаления соответственно файлов и каталогов.
    + FAR_CreateFile - обертка для CreateFile, просьба использовать именно
      ее вместо CreateFile
*/
// удалить файл, код возврата аналогичен DeleteFile
BOOL WINAPI FAR_DeleteFile(const char *FileName);
// удалить каталог, код возврата аналогичен RemoveDirectory
BOOL WINAPI FAR_RemoveDirectory(const char *DirName);

// открыть файл, вод возврата аналогичен CreateFile
HANDLE WINAPI FAR_CreateFile(
    LPCTSTR lpFileName,     // pointer to name of the file
    DWORD dwDesiredAccess,  // access (read-write) mode
    DWORD dwShareMode,      // share mode
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, // pointer to security attributes
    DWORD dwCreationDistribution, // how to create
    DWORD dwFlagsAndAttributes,   // file attributes
    HANDLE hTemplateFile          // handle to file with attributes to copy
   );
/* IS $ */

HANDLE FAR_FindFirstFile(const char *FileName,LPWIN32_FIND_DATA lpFindFileData,bool ScanSymLink=true);
BOOL FAR_FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData);
BOOL FAR_FindClose(HANDLE hFindFile);

BOOL GetFileWin32FindData(const char *Name,WIN32_FIND_DATA *FInfo=NULL,bool ScanSymLink=true);

BOOL FAR_CopyFile(
    LPCTSTR lpExistingFileName, // pointer to name of an existing file
    LPCTSTR lpNewFileName,  // pointer to filename to copy to
    BOOL bFailIfExists  // flag for operation if file exists
   );

BOOL Init_CopyFileEx(void);
BOOL FAR_CopyFileEx(LPCTSTR lpExistingFileName,
            LPCTSTR lpNewFileName,void *lpProgressRoutine,
            LPVOID lpData,LPBOOL pbCancel,DWORD dwCopyFlags);
BOOL FAR_MoveFile(
    LPCTSTR lpExistingFileName, // address of name of the existing file
    LPCTSTR lpNewFileName   // address of new name for the file
   );
BOOL FAR_MoveFileEx(
    LPCTSTR lpExistingFileName, // address of name of the existing file
    LPCTSTR lpNewFileName,   // address of new name for the file
    DWORD dwFlags   // flag to determine how to move file
   );
BOOL MoveFileThroughTemp(const char *Src, const char *Dest);

BOOL FAR_GetFileSize (HANDLE hFile, unsigned __int64 *pSize);

BOOL WINAPI FAR_SetFilePointerEx(HANDLE hFile,LARGE_INTEGER liDistanceToMove,PLARGE_INTEGER lpNewFilePointer,DWORD dwMoveMethod);

void WINAPI SetFileApisTo(int Type);
BOOL WINAPI FAR_OemToCharBuff(LPCSTR lpszSrc,LPTSTR lpszDst,DWORD cchDstLength);
BOOL WINAPI FAR_CharToOemBuff(LPCSTR lpszSrc,LPTSTR lpszDst,DWORD cchDstLength);
BOOL WINAPI FAR_OemToChar(LPCSTR lpszSrc,LPTSTR lpszDst);
BOOL WINAPI FAR_CharToOem(LPCSTR lpszSrc,LPTSTR lpszDst);

BOOL WINAPI FAR_GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer);


char* FarMSG(int MsgID);
#define MSG(ID) FarMSG(ID)

/* $ 29.08.2000 SVS
   Дополнительный параметр у Message* - номер плагина.
*/
int Message(DWORD Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2=NULL,const char *Str3=NULL,const char *Str4=NULL,
            int PluginNumber=-1);
int Message(DWORD Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2,const char *Str3,const char *Str4,
            const char *Str5,const char *Str6=NULL,const char *Str7=NULL,
            int PluginNumber=-1);
int Message(DWORD Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2,const char *Str3,const char *Str4,
            const char *Str5,const char *Str6,const char *Str7,
            const char *Str8,const char *Str9=NULL,const char *Str10=NULL,
            int PluginNumber=-1);
int Message(DWORD Flags,int Buttons,const char *Title,const char *Str1,
            const char *Str2,const char *Str3,const char *Str4,
            const char *Str5,const char *Str6,const char *Str7,
            const char *Str8,const char *Str9,const char *Str10,
            const char *Str11,const char *Str12=NULL,const char *Str13=NULL,
            const char *Str14=NULL, int PluginNumber=-1);
int Message(DWORD Flags,int Buttons,const char *Title,const char * const *Items,
            int ItemsNumber,int PluginNumber=-1);
/* SVS $*/
/* $ 12.03.2002 VVM
  Новая функция - пользователь попытался прервать операцию.
  Зададим вопрос.
  Возвращает:
   FALSE - продолжить операцию
   TRUE  - прервать операцию
*/
int AbortMessage();
/* VVM $ */
void SetMessageHelp(const char *Topic);
void GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2);
int ToPercent(unsigned long N1,unsigned long N2);
int ToPercent64(__int64 N1,__int64 N2);
// возвращает: 1 - LeftPressed, 2 - Right Pressed, 3 - Middle Pressed, 0 - none
int IsMouseButtonPressed();
int CmpName(const char *pattern,const char *string,int skippath=TRUE);
/* $ 09.10.2000 IS
    + Новая функция для обработки имени файла
*/
// обработать имя файла: сравнить с маской, масками, сгенерировать по маске
int WINAPI ProcessName(const char *param1, char *param2, DWORD flags);
/* IS $ */
char* QuoteSpace(char *Str);
/* $ 03.08.2001 IS функция заключения строки в кавычки */
char *InsertQuote(char *Str);
/* IS $ */
int ProcessGlobalFileTypes(char *Name,int AlwaysWaitFinish);
int ProcessLocalFileTypes(char *Name,char *ShortName,int Mode,int AlwaysWaitFinish);
void ProcessExternal(char *Command,char *Name,char *ShortName,int AlwaysWaitFinish);
int SubstFileName(char *Str,int StrSize, char *Name,char *ShortName,
                  char *ListName=NULL,char *ShortListName=NULL,
                  int IgnoreInput=FALSE,char *CmdLineDir=NULL);
BOOL ExtractIfExistCommand(char *CommandText);
void EditFileTypes();
void ProcessUserMenu(int EditMenu);
DWORD RawConvertShortNameToLongName(const char *src, char *dest, DWORD maxsize);
int ConvertNameToFull(const char *Src,char *Dest, int DestSize);
int WINAPI OldConvertNameToReal(const char *Src,char *Dest, int DestSize);
int WINAPI ConvertNameToReal(const char *Src,char *Dest, int DestSize,bool Internal=true);
void ConvertNameToShort(const char *Src,char *Dest,int DestSize);
void ChangeConsoleMode(int Mode);
void FlushInputBuffer();
void SystemSettings();
void PanelSettings();
void InterfaceSettings();
void DialogSettings();
void SetConfirmations();
void SetDizConfig();
int  IsLocalDrive(const char *Path);
/* $ 27.11.2001 DJ
   параметр Local
*/
void ViewerConfig(struct ViewerOptions &ViOpt,int Local=0);
void EditorConfig(struct EditorOptions &EdOpt,int Local=0);
/* DJ $ */
void SetFolderInfoFiles();
void ReadConfig();
void SaveConfig(int Ask);
void SetColors();
int GetColorDialog(unsigned int &Color,bool bCentered=false,bool bAddTransparent=false);
int HiStrlen(const char *Str);
int HiFindRealPos(const char *Str, int Pos, BOOL ShowAmp);
char *HiText2Str(char *Dest, int DestSize, const char *Str);

/* $ 27.01.2001 VVM
   + Дополнительный параметр у GetErrorString - резмер буфера */
int GetErrorString(char *ErrStr, DWORD StrSize);
/* VVM $ */
// Проверка на "продолжаемость" экспериментов по... например, удалению файла с разными именами!
BOOL CheckErrorForProcessed(DWORD Err);
void ShowProcessList();
int CopyFormatToClipboard(const char *Format,char *Data);
char* PasteFormatFromClipboard(const char *Format);
/* $ 16.10.2000 tran
  параметер - ограничение по длины */
char* WINAPI PasteFromClipboardEx(int max);
/* tran $ */
BOOL WINAPI FAR_EmptyClipboard(VOID);

int GetFileTypeByName(const char *Name);
void SetFarTitle(const char *Title);
void LocalUpperInit();
/* $ 11.01.2002 IS инициализация массива клавиш */
void InitKeysArray();
/* IS $ */
void InitLCIDSort();
/* $ 28.08.2000 SVS
   Модификация вызова под WINAPI
*/
int WINAPI LocalIslower(unsigned Ch);
int WINAPI LocalIsupper(unsigned Ch);
int WINAPI LocalIsalpha(unsigned Ch);
int WINAPI LocalIsalphanum(unsigned Ch);
unsigned WINAPI LocalUpper(unsigned LowerChar);
void WINAPI LocalUpperBuf(char *Buf,int Length);
void WINAPI LocalLowerBuf(char *Buf,int Length);
unsigned WINAPI LocalLower(unsigned UpperChar);
void WINAPI LocalStrupr(char *s1);
void WINAPI LocalStrlwr(char *s1);
int WINAPI LStricmp(const char *s1,const char *s2);
int WINAPI LStrnicmp(const char *s1,const char *s2,int n);
/* SVS $ */
const char * __cdecl LocalStrstri(const char *str1, const char *str2);
const char * __cdecl LocalRevStrstri(const char *str1, const char *str2);
int __cdecl LocalStricmp(const char *s1,const char *s2);
int __cdecl LocalStrnicmp(const char *s1,const char *s2,int n);
int __cdecl LCStricmp(const char *s1,const char *s2);
int __cdecl LCNumStricmp(const char *s1,const char *s2);

int LocalKeyToKey(int Key);
int GetShortcutFolder(int Key,char *DestFolder,int DestSize,char *PluginModule=NULL,
                      char *PluginFile=NULL,char *PluginData=NULL);
int SaveFolderShortcut(int Key,char *SrcFolder,char *PluginModule=NULL,
                       char *PluginFile=NULL,char *PluginData=NULL);
int GetShortcutFolderSize(int Key);
void ShowFolderShortcut();
void ShowFilter();
/* 15.09.2000 IS
   Проверяет, установлена ли таблица с распределением частот символов
*/
int DistrTableExist(void);
/* IS $ */
int GetTable(struct CharTableSet *TableSet,int AnsiText,int &TableNum,
             int &UseUnicode);
void DecodeString(char *Str,unsigned char *DecodeTable,int Length=-1);
void EncodeString(char *Str,unsigned char *EncodeTable,int Length=-1);
//char *NullToEmpty(char *Str);
const char *NullToEmpty(const char *Str);
char* CenterStr(char *Src,char *Dest,int Length);
const char *GetCommaWord(const char *Src,char *Word,char Separator=',');
void ScrollBar(int X1,int Y1,int Length,unsigned long Current,unsigned long Total);
int WINAPI GetFileOwner(const char *Computer,const char *Name,char *Owner);
void SIDCacheFlush(void);

/* $ 26.10.2003 KM
   Изменение входных параметров
*/
/* $ 21.09.2003 KM
   Трансформирует строку по заданному типу.
*/
void Transform(unsigned char *Buffer,int &BufLen,const char *ConvStr,char TransformType);
/* KM $ */
/* KM $ */

void ConvertDate(const FILETIME &ft,char *DateText,char *TimeText,int TimeLength,
        int Brief=FALSE,int TextMonth=FALSE,int FullYear=FALSE,int DynInit=FALSE);
void ConvertRelativeDate(const FILETIME &ft,char *DaysText,char *TimeText);
void ShellOptions(int LastCommand,MOUSE_EVENT_RECORD *MouseEvent);

// Registry
void SetRegRootKey(HKEY hRootKey);
HKEY GetRegRootKey(void);
LONG SetRegKey(const char *Key,const char *ValueName,const char * const ValueData);
LONG SetRegKey(const char *Key,const char *ValueName,const char * const ValueData,int SizeData, DWORD Type);
LONG SetRegKey(const char *Key,const char *ValueName,DWORD ValueData);
LONG SetRegKey64(const char *Key,const char *ValueName,unsigned __int64 ValueData);
LONG SetRegKey(const char *Key,const char *ValueName,const BYTE *ValueData,DWORD ValueSize);

int GetRegKey(const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize,DWORD *pType=NULL);
int GetRegKey(const char *Key,const char *ValueName,BYTE *ValueData,const BYTE *Default,DWORD DataSize,DWORD *pType=NULL);
int GetRegKey(const char *Key,const char *ValueName,int &ValueData,DWORD Default);
int GetRegKey64(const char *Key,const char *ValueName,__int64 &ValueData,unsigned __int64 Default);
int GetRegKey(const char *Key,const char *ValueName,DWORD Default);
__int64 GetRegKey64(const char *Key,const char *ValueName,unsigned __int64 Default);
HKEY CreateRegKey(const char *Key);
HKEY OpenRegKey(const char *Key);
int GetRegKeySize(const char *Key,const char *ValueName);
int GetRegKeySize(HKEY hKey,const char *ValueName);
int EnumRegValue(const char *Key,DWORD Index,char *DestName,DWORD DestSize,LPBYTE SData,DWORD SDataSize,DWORD *NeedSDataSize=NULL,LPDWORD IData=NULL,__int64* IData64=NULL);
void DeleteRegKey(const char *Key);
void DeleteRegValue(const char *Key,const char *Value);
void DeleteKeyRecord(const char *KeyMask,int Position);
void InsertKeyRecord(const char *KeyMask,int Position,int TotalKeys);
void RenumKeyRecord(const char *KeyRoot,const char *KeyMask,const char *KeyMask0);
void DeleteKeyTree(const char *KeyName);
int CheckRegKey(const char *Key);
int CheckRegValue(const char *Key,const char *ValueName);
int DeleteEmptyKey(HKEY hRoot, const char *FullKeyName);
int EnumRegKey(const char *Key,DWORD Index,char *DestName,DWORD DestSize);
int CopyKeyTree(const char *Src,const char *Dest,const char *Skip);
void UseSameRegKey();
void CloseSameRegKey();


int CheckFolder(const char *Name);
int CheckShortcutFolder(char *TestPath,int LengthPath,int IsHostFile, BOOL Silent=FALSE);

#if defined(__FARCONST_HPP__) && (defined(_INC_WINDOWS) || defined(_WINDOWS_) || defined(_WINDOWS_H))
UDWORD NTTimeToDos(FILETIME *ft);
int Execute(const char *CmdStr,int AlwaysWaitFinish,int SeparateWindow=FALSE,int DirectRun=FALSE,int FolderRun=FALSE);
#endif

class Panel;
void ShellMakeDir(Panel *SrcPanel);
void ShellDelete(Panel *SrcPanel,int Wipe);
int  ShellSetFileAttributes(Panel *SrcPanel);
void PrintFiles(Panel *SrcPanel);
void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir=FALSE);
int  CheckUpdateAnotherPanel(Panel *SrcPanel,const char *SelName);

BOOL GetDiskSize(char *Root,unsigned __int64 *TotalSize,unsigned __int64 *TotalFree,unsigned __int64 *UserFree);
int GetDirInfo(char *Title,const char *DirName,unsigned long &DirCount,
               unsigned long &FileCount,unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize,unsigned __int64 &RealSize,
               unsigned long &ClusterSize,clock_t MsgWaitTime,
               FileFilter *Filter,
               DWORD Flags=GETDIRINFO_SCANSYMLINKDEF);
int GetPluginDirInfo(HANDLE hPlugin,char *DirName,unsigned long &DirCount,
               unsigned long &FileCount,unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize);

int DetectTable(FILE *SrcFile,struct CharTableSet *TableSet,int &TableNum);

#ifdef __PLUGIN_HPP__
/* $ 17.03.2002 IS
   Параметр UseTableName - в качестве имени таблицы использовать не имя ключа
   реестра, а соответствующую переменную.
   По умолчанию - FALSE (использовать имя ключа).
*/
int PrepareTable(struct CharTableSet *TableSet,int TableNum,BOOL UseTableName=FALSE);
/* IS $ */
#endif



#if defined(_INC_WINDOWS) || defined(_WINDOWS_)
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
#endif

#ifdef __PLUGIN_HPP__
// эти функции _были_ как static
int WINAPI FarGetPluginDirList(INT_PTR PluginNumber,HANDLE hPlugin,
                  const char *Dir,struct PluginPanelItem **pPanelItem,
                  int *pItemsNumber);
int WINAPI FarMenuFn(INT_PTR PluginNumber,int X,int Y,int MaxHeight,
           DWORD Flags,const char *Title,const char *Bottom,
           const char *HelpTopic,const int *BreakKeys,int *BreakCode,
           const struct FarMenuItem *Item, int ItemsNumber);
int WINAPI FarDialogFn(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,
           const char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber);
const char* WINAPI FarGetMsgFn(INT_PTR PluginNumber,int MsgId);
int WINAPI FarMessageFn(INT_PTR PluginNumber,DWORD Flags,
           const char *HelpTopic,const char * const *Items,int ItemsNumber,
           int ButtonsNumber);
int WINAPI FarControl(HANDLE hPlugin,int Command,void *Param);
HANDLE WINAPI FarSaveScreen(int X1,int Y1,int X2,int Y2);
void WINAPI FarRestoreScreen(HANDLE hScreen);
int WINAPI FarGetDirList(const char *Dir,struct PluginPanelItem **pPanelItem,
           int *pItemsNumber);
void WINAPI FarFreeDirList(const struct PluginPanelItem *PanelItem);
int WINAPI FarViewer(const char *FileName,const char *Title,
                     int X1,int Y1,int X2,int Y2,DWORD Flags);
int WINAPI FarEditor(const char *FileName,const char *Title,
                     int X1,int Y1,int X2, int Y2,DWORD Flags,
                     int StartLine,int StartChar);
int WINAPI FarCmpName(const char *pattern,const char *string,int skippath);
int WINAPI FarCharTable(int Command,char *Buffer,int BufferSize);
void WINAPI FarText(int X,int Y,int Color,const char *Str);
#if defined(USE_WFUNC)
int WINAPI TextToCharInfo(const char *Text,WORD Attr, CHAR_INFO *CharInfo, int Length, DWORD Reserved);
#endif
int WINAPI FarEditorControl(int Command,void *Param);

int WINAPI FarViewerControl(int Command,void *Param);

/* $ 18.08.2000 tran
   add Flags parameter */
/* $ 03.07.2000 IS
  Функция вывода помощи
*/
BOOL WINAPI FarShowHelp(const char *ModuleName,
                        const char *HelpTopic,DWORD Flags);
/* IS $ */
/* tran 18.08.2000 $ */
/* $ 07.12.2001 IS
   Обертка вокруг GetString для плагинов - с меньшей функциональностью.
   Сделано для того, чтобы не дублировать код GetString.
*/
int WINAPI FarInputBox(const char *Title,const char *Prompt,
                       const char *HistoryName,const char *SrcText,
                       char *DestText,int DestLength,
                       const char *HelpTopic,DWORD Flags);
/* IS $ */
/* $ 06.07.2000 IS
  Функция, которая будет действовать и в редакторе, и в панелях, и...
*/
INT_PTR WINAPI FarAdvControl(INT_PTR ModuleNumber, int Command, void *Param);
/* IS $ */
/* $ 23.07.2000 IS
   Функции для расширенного диалога
*/
//  Функция расширенного диалога
int WINAPI FarDialogEx(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,
      const char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber,
      DWORD Reserved, DWORD Flags,
      FARWINDOWPROC Proc,LONG_PTR Param);
//  Функция обработки диалога по умолчанию
LONG_PTR WINAPI FarDefDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
// Посылка сообщения диалогу
LONG_PTR WINAPI FarSendDlgMessage(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);

/* SVS $ */
#endif


/* $ 24.07.2000 SVS
   Те функции, которые попадают в FSF
   Должны иметь WINAPI
*/
/* $ 05.07.2000 SVS
   Расширение переменной среды
*/
DWORD WINAPI ExpandEnvironmentStr(const char *src, char *dst, size_t size=8192);
/* SVS $ */

BOOL UnExpandEnvString(const char *Path, const char *EnvVar, char* Dest, int DestSize);
BOOL PathUnExpandEnvStr(const char *Path, char* Dest, int DestSize);

void WINAPI Unquote(char *Str);
void UnquoteExternal(char *Str);

/* $ 07.07.2000 SVS
   + удалить пробелы снаружи
   ! изменен тип возврата
*/
char* WINAPI RemoveLeadingSpaces(char *Str);
char* WINAPI RemoveTrailingSpaces(char *Str);
char* WINAPI RemoveExternalSpaces(char *Str);
/* SVS $ */
/* $ 02.02.2001 IS
  + Новая функция: заменяет пробелами непечатные символы в строке
*/
char* WINAPI RemoveUnprintableCharacters(char *Str);
/* IS $ */
char* WINAPI TruncStr(char *Str,int MaxLength);
char* WINAPI TruncStrFromEnd(char *Str, int MaxLength);
char* WINAPI TruncPathStr(char *Str,int MaxLength);
char* WINAPI QuoteSpaceOnly(char *Str);
char* WINAPI PointToName(char *Path);
char* WINAPI PointToNameUNC(char *Path);
/* $ 12.01.2004 IS
   + Функция для сверки символа с разделителями слова с учетом текущей
     кодировки
*/
// Проверяет - является ли символ разделителем слова (вернет TRUE, если да)
// Параметры:
//   TableSet - указатель на таблицы перекодировки (если отсутствует,
//              то кодировка - OEM)
//   WordDiv  - набор разделителей слова в кодировке OEM
//   Chr      - проверяемый символ
BOOL IsWordDiv(const struct CharTableSet *TableSet, const char *WordDiv, unsigned char Chr);
/* IS $ */
const char* WINAPI PointToName(const char *Path);
/* $ 20.03.2002 IS
    + PointToFolderNameIfFolder - аналог PointToName, только для строк типа
      "name\" (оканчивается на слеш) возвращает указатель на name, а не
      на пустую строку
*/
char* WINAPI PointToFolderNameIfFolder(const char *Path);
/* IS $ */
BOOL  TestParentFolderName(const char *Name);
BOOL  AddEndSlash(char *Path,char TypeSlash);
BOOL  WINAPI AddEndSlash(char *Path);
BOOL  WINAPI DeleteEndSlash(char *Path,bool allendslash=false);
int __cdecl NumStrcmp(const char *s1, const char *s2);
int __digit_cnt_0(const char* s, const char** beg);
char *WINAPI FarItoa(int value, char *string, int radix);
__int64 WINAPI FarAtoi64(const char *s);
char *WINAPI FarItoa64(__int64 value, char *string, int radix);
int WINAPI FarAtoi(const char *s);
void WINAPI FarQsort(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
void WINAPI FarQsortEx(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *),void*);
#if defined(__BORLANDC__)
int WINAPIV FarSprintf(char *buffer,const char *format,...);
int WINAPIV FarSnprintf(char *buffer,size_t sizebuf,const char *format,...);
#ifndef FAR_MSVCRT
int WINAPIV FarSscanf(const char *buffer, const char *format,...);
#endif
#endif // defined(__BORLANDC__)
int WINAPI CopyToClipboard(const char *Data);
char* WINAPI PasteFromClipboard(void);

char* InternalPasteFromClipboard(int AnsiMode);
char* InternalPasteFromClipboardEx(int max,int AnsiMode);
int InternalCopyToClipboard(const char *Data,int AnsiMode);
/* $ 01.08.2000 SVS
   ! Функция ввода строки GetString имеет один параметр для всех флагов
*/
/* $ 31.07.2000 SVS
    ! функция GetString имеет еще один параметр - расширение среды
*/
/* $ 07.12.2001 IS
   ! Два дополнительных параметра, которые используются при добавлении
     чек-бокса
*/
int WINAPI GetString(const char *Title,const char *SubTitle,
                     const char *HistoryName,const char *SrcText,
    char *DestText,int DestLength,const char *HelpTopic=NULL,DWORD Flags=0,
    int *CheckBoxValue=NULL,const char *CheckBoxText=NULL);
/* IS $ */
/* SVS $ */
int WINAPI GetNameAndPassword(char *Title,char *UserName,char *Password,char *HelpTopic,DWORD Flags);

/* Программое переключение FulScreen <-> Windowed
   (с подачи "Vasily V. Moshninov" <vmoshninov@newmail.ru>)
   mode = -2 - GetMode
          -1 - как тригер
           0 - Windowed
           1 - FulScreen
   Return
           0 - Windowed
           1 - FulScreen
*/
int FarAltEnter(int mode);


/* $ 05.09.2000 SVS
  XLat-перекодировка!
  На основе плагина EditSwap by SVS :-)))
*/
char* WINAPI Xlat(char *Line,
                    int StartPos,
                    int EndPos,
                    const struct CharTableSet *TableSet,
                    DWORD Flags);
/* SVS $ */

/* $ 14.08.2000 SVS
    + Функции семейства seek под __int64
*/
#ifdef __cplusplus
extern "C" {
#endif

void *WINAPI FarBsearch(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));

/* $ 10.09.2000 tran
   FSF/FarRecurseSearch*/
typedef int  (WINAPI *FRSUSERFUNC)(const WIN32_FIND_DATA *FData,const char *FullName,void *param);
void WINAPI FarRecursiveSearch(const char *initdir,const char *mask,FRSUSERFUNC func,DWORD flags,void *param);
/* tran 10.09.2000 $ */
/* $ 14.09.2000 SVS
 + Функция FarMkTemp - получение имени временного файла с полным путем.
*/
/* $ 25.10.2000 IS
 ! Изменил имя параметра с Template на Prefix
*/
char* WINAPI FarMkTemp(char *Dest, const char *Prefix);
char* FarMkTempEx(char *Dest, const char *Prefix=NULL, BOOL WithPath=TRUE);
/* IS $*/
/* SVS $*/

void CreatePath(char *Path);

/* $ 15.02.2002 IS
   Установка нужного диска и каталога и установление соответствующей переменной
   окружения. В случае успеха возвращается не ноль.
   Если ChangeDir==FALSE, то не меняем текущий  диск, а только устанавливаем
   переменные окружения.
*/
BOOL FarChDir(const char *NewDir,BOOL ChangeDir=TRUE);
/* IS $ */

// обертка вокруг функции получения текущего пути.
// для локального пути делает букву диска в uppercase
DWORD FarGetCurDir(DWORD Length,char *Buffer);

void WINAPI DeleteBuffer(void* Buffer);

#ifdef __cplusplus
};
#endif

/* <Логи ***************************************************
*/
void SysLog(int l);
void SysLog(char *fmt,...);
void SysLog(int l,char *fmt,...); ///
void SysLogLastError(void);
void ShowHeap();
void CheckHeap(int NumLine);

const char *_FARKEY_ToName(int Key);
const char *_MCODE_ToName(int OpCode);
const char *_VK_KEY_ToName(int VkKey);
const char *_ECTL_ToName(int Command);
const char *_EE_ToName(int Command);
const char *_EEREDRAW_ToName(int Command);
const char *_ESPT_ToName(int Command);
const char *_VE_ToName(int Command);
const char *_FCTL_ToName(int Command);
const char *_DLGMSG_ToName(int Msg);
const char *_ACTL_ToName(int Command);
const char *_VCTL_ToName(int Command);
const char *_INPUT_RECORD_Dump(INPUT_RECORD *Rec);
const char *_MOUSE_EVENT_RECORD_Dump(MOUSE_EVENT_RECORD *Rec);
// после вызова этой функции нужно освободить память!!!
const char *_SysLog_LinearDump(LPBYTE Buf,int SizeBuf);
void GetOpenPluginInfo_Dump(char *Title,const struct OpenPluginInfo *Info,FILE *fp);
void INPUT_RECORD_DumpBuffer(FILE *fp=NULL);
void PanelViewSettings_Dump(char *Title,const struct PanelViewSettings &ViewSettings,FILE *fp=NULL);
void PluginsStackItem_Dump(char *Title,const struct PluginsStackItem *StackItems,int ItemNumber,FILE *fp=NULL);
void SaveScreenDumpBuffer(const char *Title,const CHAR_INFO *Buffer,int X1,int Y1,int X2,int Y2,int RealScreen,FILE *fp=NULL);
class Manager;
void ManagerClass_Dump(char *Title,const Manager *m=NULL,FILE *fp=NULL);
void GetVolumeInformation_Dump(char *Title,LPCTSTR lpRootPathName,LPTSTR lpVolumeNameBuffer,DWORD nVolumeNameSize,
                                           DWORD lpVolumeSerialNumber, DWORD lpMaximumComponentLength, DWORD lpFileSystemFlags,
                                           LPTSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize,FILE *fp=NULL);

void WIN32_FIND_DATA_Dump(char *Title,const WIN32_FIND_DATA &fd,FILE *fp=NULL);

#if defined(SYSLOG_FARSYSLOG)
#ifdef __cplusplus
extern "C" {
#endif
void WINAPIV _export FarSysLog(char *ModuleName,int Level,char *fmt,...);
void WINAPI  _export FarSysLogDump(const char *ModuleName,DWORD StartAddress,LPBYTE Buf,int SizeBuf);
void WINAPI _export FarSysLog_INPUT_RECORD_Dump(char *ModuleName,INPUT_RECORD *rec);
#ifdef __cplusplus
};
#endif
#endif

#if defined(_DEBUG) && defined(SYSLOG)
#define _D(x)  x
#else
#define _D(x)
#endif

// для "алгоритмов работы" - внимание! лог будет большим!
#if defined(_DEBUG) && defined(SYSLOG_ALGO)
#define _ALGO(x)  x
#else
#define _ALGO(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_DIALOG)
#define _DIALOG(x)  x
#else
#define _DIALOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_KEYMACRO)
#define _KEYMACRO(x)  x
#else
#define _KEYMACRO(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_KEYMACRO_PARSE)
#define _KEYMACRO_PARSE(x)  x
#else
#define _KEYMACRO_PARSE(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_ECTL)
#define _ECTLLOG(x)  x
#else
#define _ECTLLOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_EE_REDRAW)
#define _SYS_EE_REDRAW(x)  x
#else
#define _SYS_EE_REDRAW(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_FCTL)
#define _FCTLLOG(x)  x
#else
#define _FCTLLOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_MANAGERLOG)
#define _MANAGERLOG(x)  x
#else
#define _MANAGERLOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_ACTL)
#define _ACTLLOG(x)  x
#else
#define _ACTLLOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_VCTL)
#define _VCTLLOG(x)  x
#else
#define _VCTLLOG(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_OT)
#define _OT(x)  x
#else
#define _OT(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_SVS)
#define _SVS(x)  x
#else
#define _SVS(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_DJ)
#define _DJ(x)  x
#else
#define _DJ(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_WARP)
#define _WARP(x)  x
#else
#define _WARP(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_VVM)
#define _VVM(x)  x
#else
#define _VVM(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_IS)
#define _IS(x)  x
#else
#define _IS(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_AT)
#define _AT(x)  x
#else
#define _AT(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_tran)
#define _tran(x)  x
#else
#define _tran(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_SKV)
#define _SKV(x)  x
#else
#define _SKV(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_KM)
#define _KM(x)  x
#else
#define _KM(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_NWZ)
#define _NWZ(x)  x
#else
#define _NWZ(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_COPYR)
#define _LOGCOPYR(x)  x
#else
#define _LOGCOPYR(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_TREX)
#define _TREX(x)  x
#else
#define _TREX(x)
#endif

#if defined(_DEBUG) && defined(SYSLOG_YJH)
#define _YJH(x)  x
#else
#define _YJH(x)
#endif

void OpenSysLog();
void CloseSysLog();

struct TUserLog
{
    FILE *Stream;
    int   Level;
};

void SysLogDump(const char *Title,DWORD StartAddress,LPBYTE Buf,int SizeBuf,FILE *fp=NULL);

FILE *OpenLogStream(char *file);

#define L_ERR      1
#define L_WARNING  2
#define L_INFO     3
#define L_DEBUG1   4
#define L_DEBUG2   5
#define L_DEBUG3   6

class CleverSysLog{ // ;-)
  public:
    CleverSysLog(const char *Title=NULL);
    ~CleverSysLog();
};


#define MAX_ARG_LEN   4096
#define MAX_LOG_LINE 10240

#define MAX_FILE 260


BOOL EjectVolume(char Letter,DWORD Flags);
BOOL RemoveUSBDrive(char Letter,DWORD Flags);
BOOL IsEjectableMedia(char Letter,UINT DriveType=DRIVE_NOT_INIT,BOOL ForceCDROM=FALSE);
BOOL IsDriveUsb(char DriveName,void *pDevInst);
int  ProcessRemoveHotplugDevice (char Drive, DWORD Flags);

bool InitializeSetupAPI ();
bool CheckInitSetupAPI ();
void FinalizeSetupAPI ();
void ShowHotplugDevice ();


/* $ 30.12.2000 SVS
   Функции работы с атрибутами файлов "опубликованы"
*/
int GetEncryptFunctions(void);
int ESetFileAttributes(const char *Name,int Attr,int SkipMode=-1);
int ESetFileCompression(const char *Name,int State,int FileAttr,int SkipMode=-1);
int ESetFileEncryption(const char *Name,int State,int FileAttr,int SkipMode=-1,int Silent=0);
#define ESetFileEncryptionSilent(Name,State,FileAttr,SkipMode) ESetFileEncryption(Name,State,FileAttr,SkipMode,1)
int ESetFileTime(const char *Name,FILETIME *LastWriteTime,
                  FILETIME *CreationTime,FILETIME *LastAccessTime,
                  int FileAttr,int SkipMode=-1);
/* SVS $ */
int ConvertWildcards(const char *Src,char *Dest, int SelectedFolderNameLength);

const char* WINAPI PrepareOSIfExist(const char *CmdLine);
BOOL IsBatchExtType(const char *ExtPtr);
BOOL BatchFileExist(const char *FileName,char *DestName,int SizeDestName);

int WINAPI GetSearchReplaceString(
         int IsReplaceMode,
         unsigned char *SearchStr,
         int LenSearchStr,
         unsigned char *ReplaceStr,
         int LenReplaceStr,
         const char *TextHistoryName,
         const char *ReplaceHistoryName,
         int *Case,
         int *WholeWords,
         int *Reverse);

BOOL WINAPI KeyMacroToText(int Key,char *KeyText0,int Size);
int WINAPI KeyNameMacroToKey(const char *Name);
int TranslateKeyToVK(int Key,int &VirtKey,int &ControlState,INPUT_RECORD *rec=NULL);
/* $ 24.09.2000 SVS
 + Функция KeyNameToKey - получение кода клавиши по имени
   Если имя не верно или нет такого - возвращается -1
*/
int WINAPI KeyNameToKey(const char *Name);
/* SVS $*/
// ! дополнительный параметра у KeyToText - размер данных
//   Size=0 - по максимуму!
BOOL WINAPI KeyToText(int Key,char *KeyText,int Size=0);
/* SVS $ */
/* 01.08.2000 SVS $ */
/* $ 31.08.2000 tran
   FSF/FarInputRecordToKey */
int WINAPI InputRecordToKey(const INPUT_RECORD *Rec);
/* tran 31.08.2000 $ */
DWORD GetInputRecord(INPUT_RECORD *rec,bool ExcludeMacro=false,bool ProcessMouse=false);
DWORD PeekInputRecord(INPUT_RECORD *rec);
DWORD CalcKeyCode(INPUT_RECORD *rec,int RealKey,int *NotMacros=NULL);
/* $ 24.08.2000 SVS
 + Пераметр у фунции WaitKey - возможность ожидать конкретную клавишу
*/
DWORD WaitKey(DWORD KeyWait=(DWORD)-1,DWORD delayMS=0);
/* SVS $ */
int SetFLockState(UINT vkKey, int State);
char *FARGetKeybLayoutName(char *Dest,int DestSize);
int WriteInput(int Key,DWORD Flags=0);
int IsNavKey(DWORD Key);
int IsShiftKey(DWORD Key);
int CheckForEsc();
int CheckForEscSilent();
int ConfirmAbortOp();

// Получить из имени диска RemoteName
char* DriveLocalToRemoteName(int DriveType,char Letter,char *Dest);
void __PrepareKMGTbStr(void);
char* WINAPI FileSizeToStr(char *DestStr,unsigned __int64 Size, int Width=-1, int ViewFlags=COLUMN_COMMAS);

DWORD WINAPI FarGetLogicalDrives(void);

char *Add_PATHEXT(char *Dest);

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl qsortex(char *base, size_t nel, size_t width,
            int (__cdecl *comp_fp)(const void *, const void *,void*), void *user);

char * __cdecl farmktemp(char *temp);
char * __cdecl xstrncpy (char * dest,const char * src,size_t maxlen);
char * __cdecl xstrncat (char * dest,const char * src,size_t maxlen);
char * __cdecl xf_strdup (const char * string);
void __cdecl far_qsort (
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
    );

void  __cdecl xf_free(void *__block);
void *__cdecl xf_malloc(size_t __size);
void *__cdecl xf_realloc(void *__block, size_t __size);

#ifdef __cplusplus
}
#endif

/* $ 01.05.2001 DJ
   inline-функции для быстрой конвертации
*/

inline char LocalUpperFast (char c)
{
  extern unsigned char LowerToUpper[256];  // in local.cpp
  return LowerToUpper [c];
}

inline char LocalLowerFast (char c)
{
  extern unsigned char UpperToLower[256];  // in local.cpp
  return UpperToLower [c];
}
/* DJ $ */

void GenerateWINDOW_BUFFER_SIZE_EVENT(int Sx=-1, int Sy=-1);

void PrepareStrFTime(void);
int WINAPI StrFTime(char *Dest, size_t MaxSize, const char *Format,const struct tm *t);
int MkStrFTime(char *Dest,int DestSize,const char *Fmt=NULL);

BOOL WINAPI GetMenuHotKey(char *HotKey,int LenHotKey,
                          char *DlgHotKeyTitle,
                          char *DlgHotKeyText,
                          char *DlgPluginTitle,  // заголовок
                          char *HelpTopic,
                          char *RegKey,
                          char *RegValueName);

char *WINAPI FarFormatText(const char *SrcText,int Width,
                      char *DestText,int MaxLen,
                      const char* Break, DWORD Flags);

int PathMayBeAbsolute(const char *Src);
char* PrepareDiskPath(char *Path,int MaxSize,BOOL CheckFullPath=TRUE);

//   TableSet - указатель на таблицы перекодировки (если отсутствует,
//              то кодировка - OEM)
//   WordDiv  - набор разделителей слова в кодировке OEM
// возвращает указатель на начало слова
const char * const CalcWordFromString(const char *Str,int CurPos,int *Start,int *End,const struct CharTableSet *TableSet, const char *WordDiv);

void CharBufferTooSmallWarn(int BufSize, int FileNameSize);

long filelen(FILE *FPtr);
__int64 filelen64(FILE *FPtr);
__int64 ftell64(FILE *fp);
int fseek64 (FILE *fp, __int64 offset, int whence);

BOOL IsDiskInDrive(const char *Drive);

CDROM_DeviceCaps GetCDDeviceCaps(HANDLE hDevice);
UINT GetCDDeviceTypeByCaps(CDROM_DeviceCaps caps);
BOOL IsDriveTypeCDROM(UINT DriveType);
UINT FAR_GetDriveType(LPCTSTR RootDir,CDROM_DeviceCaps *caps=NULL,DWORD Detect=0);

bool PathPrefix(const char *Path);
BOOL IsNetworkPath(const char *Path);
BOOL IsLocalPath(const char *Path);
BOOL IsLocalRootPath(const char *Path);
BOOL IsLocalPrefixPath(const char *Path);
BOOL IsLocalVolumePath(const char *Path);
BOOL IsLocalVolumeRootPath(const char *Path);

BOOL RunGraber(void);

BOOL ProcessOSAliases(char *Str,int SizeStr);
int PartCmdLine(const char *CmdStr,char *NewCmdStr,int SizeNewCmdStr,char *NewCmdPar,int SizeNewCmdPar);

void initMacroVarTable(int global);
void doneMacroVarTable(int global);
bool checkMacroConst(const char *name);

const char *eStackAsString(int Pos=0);
int __parseMacroString(DWORD *&CurMacroBuffer, int &CurMacroBufferSize, const char *BufPtr);
BOOL __getMacroParseError(char *ErrMsg1,char *ErrMsg2,char *ErrMsg3);
int  __getMacroErrorCode(int *nErr=NULL);

int _MakePath1(DWORD Key,char *PathName,int PathNameSize, const char *Param2,int ShortNameAsIs=TRUE);

const char *CurPath2ComputerName(const char *CurDir, char *ComputerName,int SizeName);
void ConvertNameToUNC(char *FileName, int Size);

int CheckDisksProps(const char *SrcPath,const char *DestPath,int CheckedType);

BOOL GetFileDateAndTime(const char *Src,unsigned *Dst,int Separator);
BOOL StrToDateTime(const char *CDate,const char *CTime,FILETIME &ft, int DateFormat, int DateSeparator, int TimeSeparator, bool bRelative=false);
int ReadFileTime(int Type,const char *Name,DWORD FileAttr,FILETIME *FileTime,char *OSrcDate,char *OSrcTime);

bool CheckFileSizeStringFormat(const char *FileSizeStr);
unsigned __int64 ConvertFileSizeString(const char *FileSizeStr);

bool GetShellType(const char *Ext, char *Type, LONG Size,ASSOCIATIONTYPE aType=AT_FILEEXTENSION);

// Проверка типа файловой системы. Если TargetFS не задан (NULL или ""), то проверяем на "NTFS"
BOOL CheckFileSystem(const char *CurDir,const char *TargetFS=NULL);

#endif  // __FARFUNC_HPP__
