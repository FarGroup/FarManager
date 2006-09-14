#ifndef __FARFUNC_HPP__
#define __FARFUNC_HPP__
/*
fn.hpp

Описания функций

*/

/* Revision: 1.279 04.07.2006 $ */

#include "farconst.hpp"
#include "global.hpp"
#include "plugin.hpp"

char *UnicodeToAnsi (const wchar_t *lpwszUnicodeString, int nMaxLength = -1);
void UnicodeToAnsi (const wchar_t *lpwszUnicodeString, char *lpDest, int nMaxLength = -1); //BUGBUG
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
void TextW(int MsgId);
#endif
void Text(int X, int Y, int Color, int MsgId);
void Text(int MsgId);
void VText(const char *Str);
#if defined(USE_WFUNC)
void VTextW(const WCHAR *Str);
#endif
void HiText(const char *Str,int HiColor);
#if defined(USE_WFUNC)
void HiTextW(const WCHAR *Str,int HiColor);
#endif

void DrawLine(int Length,int Type);
#define ShowSeparator(Length,Type) DrawLine(Length,Type)

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
void GetRealText(int X1,int Y1,int X2,int Y2,void *Dest);
void PutRealText(int X1,int Y1,int X2,int Y2,const void *Src);
void _GetRealText(HANDLE hConsoleOutput,int X1,int Y1,int X2,int Y2,const void *Src,int BufX,int BufY);
void _PutRealText(HANDLE hConsoleOutput,int X1,int Y1,int X2,int Y2,const void *Src,int BufX,int BufY);

#if defined(USE_WFUNC)
void mprintfW(WCHAR *fmt,...);
void vmprintfW(WCHAR *fmt,...);
#endif
void mprintf(char *fmt,...);
void mprintf(int MsgId,...);
void vmprintf(char *fmt,...);

inline WORD GetVidChar(CHAR_INFO CI)
{
  return CI.Char.UnicodeChar;
}

inline void SetVidChar(CHAR_INFO& CI,WORD Chr)
{
  CI.Char.UnicodeChar = Chr;
}

void ShowTime(int ShowAlways);
int GetDateFormat();
int GetDateSeparator();
int GetTimeSeparator();

char* GetShellAction(const char *FileName,DWORD& ImageSubsystem,DWORD& Error);
void ScrollScreen(int Count);
int ScreenSaver(int EnableExit);

string &InsertCommasW(unsigned __int64 li, string &strDest);

void DeleteDirTree(const wchar_t *Dir);
int GetClusterSizeW(const wchar_t *Root);

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
string &RemoveCharW(string &strStr,wchar_t Target,BOOL Dup=TRUE);

wchar_t *InsertStringW(wchar_t *Str,int Pos,const wchar_t *InsStr,int InsSize=0);

int ReplaceStrings(char *Str,const char *FindStr,const char *ReplStr,int Count=-1,BOOL IgnoreCase=FALSE);
int ReplaceStringsW(string &strStr,const wchar_t *FindStr,const wchar_t *ReplStr,int Count=-1,BOOL IgnoreCase=FALSE);

#define RemoveHighlights(Str) RemoveChar(Str,'&')
#define RemoveHighlightsW(Str) RemoveCharW(Str,L'&')

int IsCaseMixed(char *Str);
int IsCaseLower(char *Str);

#if defined(USE_WFUNC)
#if defined(__UNICODESTRING_HPP__)
BOOL IsCaseMixedW(const string &strStr);
BOOL IsCaseLowerW(const string &strStr);
#endif
#endif

int DeleteFileWithFolderW(const wchar_t *FileName);


/* $ 26.01.2003 IS
    + FAR_DeleteFile, FAR_RemoveDirectory просьба только их использовать
      для удаления соответственно файлов и каталогов.
    + FAR_CreateFile - обертка для CreateFile, просьба использовать именно
      ее вместо CreateFile
*/
// удалить файл, код возврата аналогичен DeleteFile
BOOL WINAPI FAR_DeleteFileW(const wchar_t *FileName);
// удалить каталог, код возврата аналогичен RemoveDirectory
BOOL WINAPI FAR_RemoveDirectoryW(const wchar_t *DirName);


HANDLE WINAPI FAR_CreateFileW(
    const wchar_t *lpwszFileName,     // pointer to name of the file
    DWORD dwDesiredAccess,  // access (read-write) mode
    DWORD dwShareMode,      // share mode
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, // pointer to security attributes
    DWORD dwCreationDistribution, // how to create
    DWORD dwFlagsAndAttributes,   // file attributes
    HANDLE hTemplateFile          // handle to file with attributes to copy
   );

/* IS $ */

BOOL FAR_CopyFileW(
    const wchar_t *lpwszExistingFileName, // pointer to name of an existing file
    const wchar_t *lpwszNewFileName,  // pointer to filename to copy to
    BOOL bFailIfExists  // flag for operation if file exists
   );


BOOL Init_CopyFileEx(void);


BOOL FAR_CopyFileExW(
        const wchar_t *lpExistingFileName,
        const wchar_t *lpNewFileName,
        void *lpProgressRoutine,
        LPVOID lpData,
        LPBOOL pbCancel,
        DWORD dwCopyFlags
        );


BOOL FAR_MoveFileW(
    const wchar_t *lpwszExistingFileName, // address of name of the existing file
    const wchar_t *lpwszNewFileName   // address of new name for the file
   );

BOOL FAR_MoveFileExW(
    const wchar_t *lpwszExistingFileName, // address of name of the existing file
    const wchar_t *lpwszNewFileName,   // address of new name for the file
    DWORD dwFlags   // flag to determine how to move file
   );


BOOL MoveFileThroughTempW(const wchar_t *Src, const wchar_t *Dest);


void WINAPI SetFileApisTo(int Type);
BOOL WINAPI FAR_OemToCharBuff(LPCSTR lpszSrc,LPTSTR lpszDst,DWORD cchDstLength);
BOOL WINAPI FAR_CharToOemBuff(LPCSTR lpszSrc,LPTSTR lpszDst,DWORD cchDstLength);
BOOL WINAPI FAR_OemToChar(LPCSTR lpszSrc,LPTSTR lpszDst);
BOOL WINAPI FAR_CharToOem(LPCSTR lpszSrc,LPTSTR lpszDst);

BOOL WINAPI FAR_GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer);

char* GetAnsiLanguageString (int nID);
wchar_t* GetUnicodeLanguageString (int nID);

#define MSG(ID) GetAnsiLanguageString(ID)
#define UMSG(ID) GetUnicodeLanguageString(ID)

/* $ 29.08.2000 SVS
   Дополнительный параметр у Message* - номер плагина.
*/

int MessageW(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2=NULL,const wchar_t *Str3=NULL,const wchar_t *Str4=NULL,
            int PluginNumber=-1);
int MessageW(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6=NULL,const wchar_t *Str7=NULL,
            int PluginNumber=-1);
int MessageW(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9=NULL,const wchar_t *Str10=NULL,
            int PluginNumber=-1);
int MessageW(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9,const wchar_t *Str10,
            const wchar_t *Str11,const wchar_t *Str12=NULL,const wchar_t *Str13=NULL,
            const wchar_t *Str14=NULL, int PluginNumber=-1);

//int __cdecl MessageW (DWORD Flags,int Buttons,const char *Title, int PluginNumber, ...);

int MessageW(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t * const *Items,
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
void SetMessageHelp(const wchar_t *Topic);
void GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2);
int ToPercent(unsigned long N1,unsigned long N2);
int ToPercent64(unsigned __int64 N1,unsigned __int64 N2);
// возвращает: 1 - LeftPressed, 2 - Right Pressed, 3 - Middle Pressed, 0 - none
int IsMouseButtonPressed();
int CmpName(const char *pattern,const char *str,int skippath=TRUE);
int CmpNameW(const wchar_t *pattern,const wchar_t *str,int skippath=TRUE);
/* $ 09.10.2000 IS
    + Новая функция для обработки имени файла
*/
// обработать имя файла: сравнить с маской, масками, сгенерировать по маске
int WINAPI ProcessName(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags);
/* IS $ */

wchar_t* WINAPI QuoteSpaceW (wchar_t *Str);
string &QuoteSpaceW(string &strStr);


wchar_t* WINAPI InsertQuoteW (wchar_t *Str);
string& InsertQuoteW(string& strStr);
/* IS $ */
//int ProcessGlobalFileTypes(char *Name,int AlwaysWaitFinish);
int ProcessGlobalFileTypesW(const wchar_t *Name,int AlwaysWaitFinish);

int ProcessLocalFileTypes(const wchar_t *Name,const wchar_t *ShortName,int Mode,int AlwaysWaitFinish);
void ProcessExternal(const wchar_t *Command,const wchar_t *Name,const wchar_t *ShortName,int AlwaysWaitFinish);

int SubstFileName(string &strStr, const wchar_t *Name, const wchar_t *ShortName,
                  string *strListName=NULL,
                  string *strAnotherListName = NULL,
                  string *strShortListName=NULL,
                  string *strAnotherShortListName=NULL,
                  int IgnoreInput=FALSE,const wchar_t *CmdLineDir=NULL);
BOOL ExtractIfExistCommand(string &strCommandText);
void EditFileTypes();
void ProcessUserMenu(int EditMenu);

DWORD RawConvertShortNameToLongNameW(const wchar_t *src, string &strDest);

int ConvertNameToFullW(const wchar_t *lpwszSrc, string &strDest);
int WINAPI ConvertNameToRealW(const wchar_t *Src, string &strDest);
void ConvertNameToShortW(const wchar_t *Src, string &strDest);

void ChangeConsoleMode(int Mode);
void FlushInputBuffer();
void SystemSettings();
void PanelSettings();
void InterfaceSettings();
void DialogSettings();
void SetConfirmations();
void SetDizConfig();
int  IsLocalDriveW(const wchar_t *Path);
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
int GetColorDialog(unsigned int &Color,bool bCentered=false);
int HiStrlenW(const wchar_t *Str,BOOL Dup=TRUE);
/* $ 27.01.2001 VVM
   + Дополнительный параметр у GetErrorString - резмер буфера */
int GetErrorString(char *ErrStr, DWORD StrSize);
int GetErrorStringW (string &strErrStr);
/* VVM $ */
// Проверка на "продолжаемость" экспериментов по... например, удалению файла с разными именами!
BOOL CheckErrorForProcessed(DWORD Err);
void ShowProcessList();

wchar_t* PasteFormatFromClipboardW(const wchar_t *Format);
int CopyFormatToClipboardW(const wchar_t *Format,const wchar_t *Data);
wchar_t* PasteFormatFromClipboard(const wchar_t *Format);
wchar_t* WINAPI PasteFromClipboardExW(int max);
/* tran $ */
BOOL WINAPI FAR_EmptyClipboard(VOID);

int GetFileTypeByNameW(const wchar_t *Name);

string &CutToSlashW (string &strStr, bool bInclude = false);
string &CutToNameUNCW (string &strPath);
const wchar_t *PointToNameUNCW (const wchar_t *lpwszPath);

void SetFarTitleW (const wchar_t *Title);
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

const wchar_t * __cdecl StrstriW(const wchar_t *str1, const wchar_t *str2);
const wchar_t * __cdecl RevStrstriW(const wchar_t *str1, const wchar_t *str2);

int __cdecl LocalStricmp(const char *s1,const char *s2);
int __cdecl LocalStrnicmp(const char *s1,const char *s2,int n);
int __cdecl LCStricmp(const char *s1,const char *s2);
int __cdecl LCNumStricmp(const char *s1,const char *s2);


void WINAPI LocalUpperBufW(wchar_t *Buf,int Length);
void WINAPI LocalLowerBufW(wchar_t *Buf,int Length);
void WINAPI LocalStruprW(wchar_t *s1);
void WINAPI LocalStrlwrW(wchar_t *s1);

wchar_t WINAPI LocalUpperW (wchar_t Ch);
wchar_t WINAPI LocalLowerW (wchar_t Ch);
int WINAPI LocalStrnicmpW (const wchar_t *s1, const wchar_t *s2, int n);
int WINAPI LocalStricmpW (const wchar_t *s1, const wchar_t *s2);
int WINAPI LocalIslowerW (wchar_t Ch);
int WINAPI LocalIsupperW (wchar_t Ch);
int WINAPI LocalIsalphaW (wchar_t Ch);
int WINAPI LocalIsalphanumW (wchar_t Ch);

int LocalKeyToKey(int Key);
int GetShortcutFolder(int Key,string *pDestFolder, string *pPluginModule=NULL,
                      string *pPluginFile=NULL,string *pPluginData=NULL);
int SaveFolderShortcut(int Key,string *pSrcFolder,string *pPluginModule=NULL,
                       string *pPluginFile=NULL,string *pPluginData=NULL);
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

int GetTableEx ();
void DecodeStringEx (wchar_t *Str, DWORD dwCP, int Length=-1);
void EncodeStringEx (wchar_t *Str, DWORD dwCP, int Length=-1);

void DecodeString(char *Str,unsigned char *DecodeTable,int Length=-1);
void EncodeString(char *Str,unsigned char *EncodeTable,int Length=-1);
//char *NullToEmpty(char *Str);
const char *NullToEmpty(const char *Str);
#define NullToEmptyW(s) (s?s:L"")

string& CenterStrW(const wchar_t *Src, string &strDest,int Length);

const char *GetCommaWord(const char *Src,char *Word,char Separator=',');
const wchar_t *GetCommaWordW(const wchar_t *Src,string &strWord,wchar_t Separator=L',');

void ScrollBar(int X1,int Y1,int Length,unsigned long Current,unsigned long Total);

int WINAPI GetFileOwnerW(const wchar_t *Computer,const wchar_t *Name, string &strOwner);

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

void ConvertDateW(const FILETIME &ft,string &strDateText, string &strTimeText,int TimeLength,
        int Brief=FALSE,int TextMonth=FALSE,int FullYear=FALSE,int DynInit=FALSE);


void ShellOptions(int LastCommand,MOUSE_EVENT_RECORD *MouseEvent);

// Registry
void SetRegRootKey(HKEY hRootKey);

LONG SetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,const wchar_t * const ValueData);
LONG SetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,DWORD ValueData);
LONG SetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,const BYTE *ValueData,DWORD ValueSize);
int GetRegKeyW(const wchar_t *Key,const wchar_t *ValueName, string &strValueData,const wchar_t *Default);
int GetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,int &ValueData,DWORD Default);
int GetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,DWORD Default);
int GetRegKeyW(const wchar_t *Key,const wchar_t *ValueName,BYTE *ValueData,const BYTE *Default,DWORD DataSize);
HKEY CreateRegKeyW(const wchar_t *Key);
HKEY OpenRegKeyW(const wchar_t *Key);
int GetRegKeySizeW(const wchar_t *Key,const wchar_t *ValueName);
int GetRegKeySizeW(HKEY hKey,const wchar_t *ValueName);
int EnumRegValueW(const wchar_t *Key,DWORD Index, string &strDestName, LPBYTE SData,DWORD SDataSize,LPDWORD IData=NULL,__int64* IData64=NULL);
int EnumRegValueExW(const wchar_t *Key,DWORD Index, string &strDestName, string strData, LPDWORD IData=NULL,__int64* IData64=NULL);
LONG SetRegKey64W(const wchar_t *Key,const wchar_t *ValueName,unsigned __int64 ValueData);
int GetRegKey64W(const wchar_t *Key,const wchar_t *ValueName,__int64 &ValueData,unsigned __int64 Default);
__int64 GetRegKey64W(const wchar_t *Key,const wchar_t *ValueName,unsigned __int64 Default);
void DeleteRegKeyW(const wchar_t *Key);
void DeleteRegValueW(const wchar_t *Key,const wchar_t *Value);
void DeleteKeyRecordW(const wchar_t *KeyMask,int Position);
void InsertKeyRecordW(const wchar_t *KeyMask,int Position,int TotalKeys);
void RenumKeyRecordW(const wchar_t *KeyRoot,const wchar_t *KeyMask,const wchar_t *KeyMask0);
void DeleteKeyTreeW(const wchar_t *KeyName);
int CheckRegKeyW(const wchar_t *Key);
int CheckRegValueW(const wchar_t *Key,const wchar_t *ValueName);
int DeleteEmptyKeyW(HKEY hRoot, const wchar_t *FullKeyName);
int EnumRegKeyW(const wchar_t *Key,DWORD Index,string &strDestName);
int CopyKeyTreeW(const wchar_t *Src,const wchar_t *Dest,const wchar_t *Skip);

void UseSameRegKey();
void CloseSameRegKey();

int RegQueryStringValueEx (HKEY hKey, const wchar_t *lpwszValueName, string &strData, const wchar_t *lpwszDefault = L"");
int RegQueryStringValue (HKEY hKey, const wchar_t *lpwszSubKey, string &strData, const wchar_t *lpwszDefault = L"");


int CheckFolderW(const wchar_t *Name);
int CheckShortcutFolderW(string *pTestPath,int IsHostFile, BOOL Silent=FALSE);

#if defined(__FARCONST_HPP__) && (defined(_INC_WINDOWS) || defined(_WINDOWS_) || defined(_WINDOWS_H))
UDWORD NTTimeToDos(FILETIME *ft);
int Execute(const wchar_t *CmdStr,int AlwaysWaitFinish,int SeparateWindow=FALSE,int DirectRun=FALSE,int FolderRun=FALSE);
#endif

class Panel;
void ShellMakeDir(Panel *SrcPanel);
void ShellDelete(Panel *SrcPanel,int Wipe);
int  ShellSetFileAttributes(Panel *SrcPanel);
void PrintFiles(Panel *SrcPanel);
void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir=FALSE);
int  CheckUpdateAnotherPanel(Panel *SrcPanel,const wchar_t *SelName);

BOOL GetDiskSizeW(const wchar_t *Root,unsigned __int64 *TotalSize, unsigned __int64 *TotalFree, unsigned __int64 *UserFree);

int GetDirInfo(const wchar_t *Title,const wchar_t *DirName,unsigned long &DirCount,
               unsigned long &FileCount,unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize,unsigned __int64 &RealSize,
               unsigned long &ClusterSize,clock_t MsgWaitTime,
               DWORD Flags=GETDIRINFO_SCANSYMLINKDEF);
int GetPluginDirInfo(HANDLE hPlugin,const wchar_t *DirName,unsigned long &DirCount,
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


#ifdef __PLUGIN_HPP__
// эти функции _были_ как static
int WINAPI FarGetPluginDirList(int PluginNumber,HANDLE hPlugin,
                  const wchar_t *Dir,struct PluginPanelItemW **pPanelItem,
                  int *pItemsNumber);
void WINAPI FarFreePluginDirList(PluginPanelItemW *PanelItem, int ItemsNumber);

int WINAPI FarMenuFn(int PluginNumber,int X,int Y,int MaxHeight,
           DWORD Flags,const wchar_t *Title,const wchar_t *Bottom,
           const wchar_t *HelpTopic,const int *BreakKeys,int *BreakCode,
           const struct FarMenuItem *Item, int ItemsNumber);
int WINAPI FarDialogFn(int PluginNumber,int X1,int Y1,int X2,int Y2,
           const wchar_t *HelpTopic,struct FarDialogItem *Item,int ItemsNumber);
const char* WINAPI FarGetMsgFn(int PluginNumber,int MsgId);
int WINAPI FarMessageFn(int PluginNumber,DWORD Flags,
           const wchar_t *HelpTopic,const wchar_t * const *Items,int ItemsNumber,
           int ButtonsNumber);
int WINAPI FarControl(HANDLE hPlugin,int Command,void *Param);
HANDLE WINAPI FarSaveScreen(int X1,int Y1,int X2,int Y2);
void WINAPI FarRestoreScreen(HANDLE hScreen);

int WINAPI FarGetDirList(const wchar_t *Dir, FAR_FIND_DATA **pPanelItem, int *pItemsNumber);
void WINAPI FarFreeDirList(FAR_FIND_DATA *PanelItem, int nItemsNumber);

int WINAPI FarViewer(const wchar_t *FileName,const wchar_t *Title,
                     int X1,int Y1,int X2,int Y2,DWORD Flags);
int WINAPI FarEditor(const wchar_t *FileName,const wchar_t *Title,
                     int X1,int Y1,int X2, int Y2,DWORD Flags,
                     int StartLine,int StartChar);
int WINAPI FarCmpName(const wchar_t *pattern,const wchar_t *string,int skippath);
int WINAPI FarCharTable(int Command,char *Buffer,int BufferSize);
void WINAPI FarText(int X,int Y,int Color,const wchar_t *Str);
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
BOOL WINAPI FarShowHelp(const wchar_t *ModuleName,
                        const wchar_t *HelpTopic,DWORD Flags);
/* IS $ */
/* tran 18.08.2000 $ */
/* $ 07.12.2001 IS
   Обертка вокруг GetString для плагинов - с меньшей функциональностью.
   Сделано для того, чтобы не дублировать код GetString.
*/
int WINAPI FarInputBox(const wchar_t *Title,const wchar_t *Prompt,
                       const wchar_t *HistoryName,const wchar_t *SrcText,
                       wchar_t *DestText,int DestLength,
                       const wchar_t *HelpTopic,DWORD Flags);
/* IS $ */
/* $ 06.07.2000 IS
  Функция, которая будет действовать и в редакторе, и в панелях, и...
*/
int WINAPI FarAdvControl(int ModuleNumber, int Command, void *Param);
/* IS $ */
/* $ 23.07.2000 IS
   Функции для расширенного диалога
*/
//  Функция расширенного диалога
int WINAPI FarDialogEx(int PluginNumber,int X1,int Y1,int X2,int Y2,
      const wchar_t *HelpTopic,struct FarDialogItem *Item,int ItemsNumber,
      DWORD Reserved, DWORD Flags,
      FARWINDOWPROC Proc,long Param);
//  Функция обработки диалога по умолчанию
long WINAPI FarDefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
// Посылка сообщения диалогу
long WINAPI FarSendDlgMessage(HANDLE hDlg,int Msg,int Param1, long Param2);

/* SVS $ */
#endif


BOOL UnExpandEnvString(const char *Path, const char *EnvVar, char* Dest, int DestSize);
BOOL PathUnExpandEnvStr(const char *Path, char* Dest, int DestSize);

void WINAPI UnquoteW(string &strStr);
void WINAPI UnquoteW(wchar_t *Str);

void UnquoteExternalW(string &strStr);

/* $ 07.07.2000 SVS
   + удалить пробелы снаружи
   ! изменен тип возврата
*/
wchar_t* WINAPI RemoveLeadingSpacesW(wchar_t *Str);
string& WINAPI RemoveLeadingSpacesW(string &strStr);

char* WINAPI RemoveTrailingSpaces(char *Str);
wchar_t *WINAPI RemoveTrailingSpacesW(wchar_t *Str);
string& WINAPI RemoveTrailingSpacesW(string &strStr);

wchar_t* WINAPI RemoveExternalSpacesW(wchar_t *Str);
string & WINAPI RemoveExternalSpacesW(string &strStr);
/* SVS $ */
/* $ 02.02.2001 IS
  + Новая функция: заменяет пробелами непечатные символы в строке
*/
string & WINAPI RemoveUnprintableCharactersW(string &strStr);

wchar_t* __stdcall TruncStrW (wchar_t *Str,int MaxLength);
string& __stdcall TruncStrW (string &strStr,int MaxLength);

string& __stdcall TruncStrFromEndW (string &strStr, int MaxLength);

wchar_t* __stdcall TruncPathStrW (wchar_t *Str, int MaxLength);
string& __stdcall TruncPathStrW (string &strStr, int MaxLength);

wchar_t* WINAPI QuoteSpaceOnlyW(wchar_t *Str);
string& WINAPI QuoteSpaceOnlyW(string &strStr);
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
BOOL IsWordDivW(const struct CharTableSet *TableSet, const wchar_t *WordDiv, wchar_t Chr);
/* IS $ */
char* __stdcall PointToName (char *Path);
const char* __stdcall PointToName (const char *Path);
const wchar_t* __stdcall PointToNameW (const wchar_t *lpwszPath);

const wchar_t* __stdcall PointToFolderNameIfFolderW (const wchar_t *lpwszPath);

BOOL  TestParentFolderNameW(const wchar_t *Name);

BOOL  AddEndSlashW (string &strPath, wchar_t TypeSlash);
BOOL  AddEndSlashW (string &strPath);

BOOL  AddEndSlashW (wchar_t *Path, wchar_t TypeSlash);
BOOL  WINAPI AddEndSlashW (wchar_t *Path);

BOOL  WINAPI DeleteEndSlash(char *Path);
BOOL  WINAPI DeleteEndSlashW(string &strPath);

int __cdecl NumStrcmp(const char *s1, const char *s2);
int __digit_cnt_0(const char* s, const char** beg);
wchar_t *WINAPI FarItoa(int value, wchar_t *string, int radix);
__int64 WINAPI FarAtoi64(const wchar_t *s);
wchar_t *WINAPI FarItoa64(__int64 value, wchar_t *string, int radix);
int WINAPI FarAtoi(const wchar_t *s);
void WINAPI FarQsort(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
void WINAPI FarQsortEx(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *),void*);
int WINAPI CopyToClipboard(const char *Data);
char* WINAPI PasteFromClipboard(void);

int WINAPI CopyToClipboardW(const wchar_t *Data);
wchar_t* WINAPI PasteFromClipboardW(void);


char* InternalPasteFromClipboard(int AnsiMode);
wchar_t* InternalPasteFromClipboardW(int AnsiMode);
char* InternalPasteFromClipboardEx(int max,int AnsiMode);
wchar_t* InternalPasteFromClipboardExW(int max,int AnsiMode);
int InternalCopyToClipboard(const char *Data,int AnsiMode);
int InternalCopyToClipboardW(const wchar_t *Data,int AnsiMode);


int WINAPI GetStringW(const wchar_t *Title,const wchar_t *SubTitle,
                     const wchar_t *HistoryName,const wchar_t *SrcText,
    string &strDestText,int DestLength,const wchar_t *HelpTopic=NULL,DWORD Flags=0,
    int *CheckBoxValue=NULL,const wchar_t *CheckBoxText=NULL);

/* IS $ */
/* SVS $ */
int WINAPI GetNameAndPasswordW(const wchar_t *Title,string &strUserName, string &strPassword, const wchar_t *HelpTopic,DWORD Flags);

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

wchar_t* WINAPI XlatW(wchar_t *Line,
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

// $ 10.09.2000 tran - FSF/FarRecurseSearch
typedef int  (WINAPI *FRSUSERFUNCW)(const FAR_FIND_DATA *FData,const wchar_t *FullName,void *param);
void WINAPI FarRecursiveSearch(const wchar_t *initdir,const wchar_t *mask,FRSUSERFUNCW func,DWORD flags,void *param);

/* $ 14.09.2000 SVS
 + Функция FarMkTemp - получение имени временного файла с полным путем.
*/
/* $ 25.10.2000 IS
 ! Изменил имя параметра с Template на Prefix
*/
//char* WINAPI FarMkTemp(char *Dest, const char *Prefix);

wchar_t* __stdcall FarMkTemp(wchar_t *Dest, DWORD size, const wchar_t *Prefix);
string& FarMkTempExW(string &strDest, const wchar_t *Prefix=NULL, BOOL WithPath=TRUE);
/* IS $*/
/* SVS $*/

void CreatePathW(string &strPath);

/* $ 15.02.2002 IS
   Установка нужного диска и каталога и установление соответствующей переменной
   окружения. В случае успеха возвращается не ноль.
   Если ChangeDir==FALSE, то не меняем текущий  диск, а только устанавливаем
   переменные окружения.
*/
BOOL FarChDirW(const wchar_t *NewDir,BOOL ChangeDir=TRUE);
/* IS $ */

// обертка вокруг функции получения текущего пути.
// для локального пути делает букву диска в uppercase
DWORD FarGetCurDirW (string &strBuffer);

class UserDefinedListW;
UserDefinedListW *SaveAllCurDir(void);
void RestoreAllCurDir(UserDefinedListW *DirList);

/*$ 27.09.2000 skv
*/
void WINAPI DeleteBuffer(char* Buffer);
/* skv$*/

#ifdef __cplusplus
};
#endif
/* SVS $ */

/* <Логи ***************************************************
*/
void SysLog(int l);
void SysLog(char *fmt,...);
void SysLog(int l,char *fmt,...); ///
void SysLogLastError(void);
void ShowHeap();
void CheckHeap(int NumLine);

string _FARKEY_ToName(int Key);
const char *_VK_KEY_ToName(int VkKey);
const char *_ECTL_ToName(int Command);
const char *_EE_ToName(int Command);
const char *_EEREDRAW_ToName(int Command);
const char *_ESPT_ToName(int Command);
const char *_FCTL_ToName(int Command);
const char *_DLGMSG_ToName(int Msg);
const char *_ACTL_ToName(int Command);
const char *_VCTL_ToName(int Command);
const char *_INPUT_RECORD_Dump(INPUT_RECORD *Rec);
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
void WINAPI  _export FarSysLogDump(char *ModuleName,DWORD StartAddress,LPBYTE Buf,int SizeBuf);
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


void OpenSysLog();
void CloseSysLog();

struct TUserLog
{
    FILE *Stream;
    int   Level;
};

void SysLogDump(char *Title,DWORD StartAddress,LPBYTE Buf,int SizeBuf,FILE *fp=NULL);

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


BOOL EjectVolume(wchar_t Letter,DWORD Flags);
BOOL RemoveUSBDrive(char Letter,DWORD Flags);
BOOL IsEjectableMedia(wchar_t Letter,UINT DriveType=DRIVE_NOT_INIT,BOOL ForceCDROM=FALSE);
BOOL IsDriveUsb(wchar_t DriveName,void *pDevInst);
int  ProcessRemoveHotplugDevice (wchar_t Drive, DWORD Flags);

bool InitializeSetupAPI ();
bool CheckInitSetupAPI ();
void FinalizeSetupAPI ();
void ShowHotplugDevice ();


/* $ 30.12.2000 SVS
   Функции работы с атрибутами файлов "опубликованы"
*/
int GetEncryptFunctions(void);

int ESetFileAttributesW(const wchar_t *Name,int Attr);
int ESetFileCompressionW(const wchar_t *Name,int State,int FileAttr);
int ESetFileEncryptionW(const wchar_t *Name,int State,int FileAttr,int Silent=0);
#define ESetFileEncryptionSilentW(Name,State,FileAttr) ESetFileEncryptionW(Name,State,FileAttr,1)
int ESetFileTimeW(const wchar_t *Name,FILETIME *LastWriteTime,
                  FILETIME *CreationTime,FILETIME *LastAccessTime,
                  int FileAttr);

/* SVS $ */
//int ConvertWildcards(const char *Src,char *Dest, int SelectedFolderNameLength);
int ConvertWildcardsW (const wchar_t *SrcName,string &strDest, int SelectedFolderNameLength);

const wchar_t* WINAPI PrepareOSIfExist(const wchar_t *CmdLine);
BOOL IsBatchExtTypeW(const wchar_t *ExtPtr);
#ifdef ADD_GUI_CHECK
BOOL BatchFileExist(const char *FileName,char *DestName,int SizeDestName);
#endif

int WINAPI GetSearchReplaceStringW (
         int IsReplaceMode,
         string *pSearchStr,
         string *pReplaceStr,
         const wchar_t *TextHistoryName,
         const wchar_t *ReplaceHistoryName,
         int *Case,
         int *WholeWords,
         int *Reverse);


BOOL WINAPI KeyMacroToText(int Key,string &strKeyText0);
int WINAPI KeyNameMacroToKey(const wchar_t *Name);
int TranslateKeyToVK(int Key,int &VirtKey,int &ControlState,INPUT_RECORD *rec=NULL);
/* $ 24.09.2000 SVS
 + Функция KeyNameToKey - получение кода клавиши по имени
   Если имя не верно или нет такого - возвращается -1
*/
int WINAPI KeyNameToKey(const wchar_t *Name);
BOOL WINAPI KeyToText (int Key, string &strKeyText);
/* SVS $ */
/* 01.08.2000 SVS $ */
/* $ 31.08.2000 tran
   FSF/FarInputRecordToKey */
int WINAPI InputRecordToKey(const INPUT_RECORD *Rec);
/* tran 31.08.2000 $ */
DWORD GetInputRecord(INPUT_RECORD *rec);
DWORD PeekInputRecord(INPUT_RECORD *rec);
DWORD CalcKeyCode(INPUT_RECORD *rec,int RealKey,int *NotMacros=NULL);
/* $ 24.08.2000 SVS
 + Пераметр у фунции WaitKey - возможность ожидать конкретную клавишу
*/
DWORD WaitKey(DWORD KeyWait=(DWORD)-1);
/* SVS $ */
BOOL FARGetKeybLayoutNameW (string &strDest);
int WriteInput(int Key,DWORD Flags=0);
int IsNavKey(DWORD Key);
int IsShiftKey(DWORD Key);
int CheckForEsc();
int CheckForEscSilent();
int ConfirmAbortOp();

// Получить из имени диска RemoteName
string &DriveLocalToRemoteNameW(int DriveType,wchar_t Letter,string &strDest);

void __PrepareKMGTbStr(void);
string& __stdcall FileSizeToStrW(string &strDestStr, unsigned __int64 Size, int Width=-1, int ViewFlags=COLUMN_COMMAS);


DWORD WINAPI FarGetLogicalDrives(void);

string &Add_PATHEXT(string &strDest);

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl qsortex(char *base, unsigned int nel, unsigned int width,
            int (__cdecl *comp_fp)(const void *, const void *,void*), void *user);

char * __cdecl farmktemp(char *temp);
char * __cdecl xstrncpy (char * dest,const char * src,size_t maxlen);
wchar_t * __cdecl xwcsncpy (wchar_t * dest,const wchar_t * src,size_t maxlen);
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
int MkStrFTimeW(string &strDest, const wchar_t *Fmt=NULL);

BOOL WINAPI GetMenuHotKeyW(string &strHotKey,int LenHotKey,
                          const wchar_t *DlgHotKeyTitle,
                          const wchar_t *DlgHotKeyText,
                          const wchar_t *DlgPluginTitle,  // заголовок
                          const wchar_t *HelpTopic,
                          const wchar_t *RegKey,
                          const wchar_t *RegValueName);

string& WINAPI FarFormatTextW(const wchar_t *SrcText, int Width, string &strDestText, const wchar_t* Break, DWORD Flags);


void SetPreRedrawFunc(PREREDRAWFUNC Func);

int PathMayBeAbsoluteW(const wchar_t *Src);

string& PrepareDiskPathW(string &strPath, BOOL CheckFullPath=TRUE);

#if defined(MOUSEKEY)
//   TableSet - указатель на таблицы перекодировки (если отсутствует,
//              то кодировка - OEM)
//   WordDiv  - набор разделителей слова в кодировке OEM
// возвращает указатель на начало слова
const char * const CalcWordFromString(const char *Str,int CurPos,int *Start,int *End,const struct CharTableSet *TableSet, const char *WordDiv);
#endif

long filelen(FILE *FPtr);
__int64 filelen64(FILE *FPtr);
__int64 ftell64(FILE *fp);
int fseek64 (FILE *fp, __int64 offset, int whence);

BOOL IsDiskInDriveW(const wchar_t *Drive);

CDROM_DeviceCaps GetCDDeviceCaps(HANDLE hDevice);
UINT GetCDDeviceTypeByCaps(CDROM_DeviceCaps caps);
BOOL IsDriveTypeCDROM(UINT DriveType);
UINT FAR_GetDriveTypeW(const wchar_t *RootDir,CDROM_DeviceCaps *caps=NULL,DWORD Detect=0);

BOOL IsLocalPathW(const wchar_t *Path);
BOOL IsLocalRootPathW(const wchar_t *Path);

BOOL RunGraber(void);

BOOL ProcessOSAliasesW(string &strStr);

int PartCmdLineW(const wchar_t *CmdStr, string &strNewCmdStr, string &strNewCmdPar);

void initMacroVarTable(int global);
void doneMacroVarTable(int global);
const wchar_t *eStackAsString(int Pos=0);

BOOL GetMacroParseError(string *strErrMessage1, string *strErrMessage2,string *strErrMessage3);

int _MakePath1W(DWORD Key,string &strPathName, const wchar_t *Param2,int ShortNameAsIs=TRUE);



#define CP_UNICODE		65535
#define CP_REVERSEBOM	65534

int GetFileFormat (FILE *file, bool *pSignatureFound = NULL);

DWORD apiGetEnvironmentVariable (const wchar_t *lpwszName, string &strBuffer);
DWORD apiGetCurrentDirectory (string &strCurDir);
DWORD apiGetTempPath (string &strBuffer);
DWORD apiGetModuleFileName (HMODULE hModule, string &strFileName);
DWORD apiExpandEnvironmentStrings (const wchar_t *src, string &strDest);
DWORD apiGetConsoleTitle (string &strConsoleTitle);
DWORD apiWNetGetConnection (const wchar_t *lpwszLocalName, string &strRemoteName);
BOOL apiGetVolumeInformation (
   const wchar_t *lpwszRootPathName,
   string *pVolumeName,
   LPDWORD lpVolumeSerialNumber,
   LPDWORD lpMaximumComponentLength,
   LPDWORD lpFileSystemFlags,
   string *pFileSystemName
   );

HANDLE apiFindFirstFile (const wchar_t *lpwszFileName, FAR_FIND_DATA_EX *pFindFileData);
BOOL apiFindNextFile (HANDLE hFindFile, FAR_FIND_DATA_EX *pFindFileData);

void apiFindDataToDataEx (const FAR_FIND_DATA *pSrc, FAR_FIND_DATA_EX *pDest);
void apiFindDataExToData (const FAR_FIND_DATA_EX *pSrc, FAR_FIND_DATA *pDest);
void apiFreeFindData (FAR_FIND_DATA *pData);

BOOL apiGetFindDataEx (const wchar_t *lpwszFileName, FAR_FIND_DATA_EX *pFindData);

string &CurPath2ComputerName(const wchar_t *CurDir, string &strComputerName);

int CheckDisksPropsW(const wchar_t *SrcPath,const wchar_t *DestPath,int CheckedType);

#endif  // __FARFUNC_HPP__
