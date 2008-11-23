#ifndef __FARFUNC_HPP__
#define __FARFUNC_HPP__
/*
fn.hpp

Описания функций
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "farconst.hpp"
#include "global.hpp"
#include "plugin.hpp"
#include "filefilter.hpp"

#define countof(a) (sizeof(a)/sizeof(a[0]))

char *UnicodeToAnsi (const wchar_t *lpwszUnicodeString, int nMaxLength = -1);
void UnicodeToAnsi (const wchar_t *lpwszUnicodeString, char *lpDest, int nMaxLength = -1); //BUGBUG

void SetHighlighting();
void _export StartFAR();
void Box(int x1,int y1,int x2,int y2,int Color,int Type);
/*$ 14.02.2001 SKV
  Инитить ли палитру default значениями.
  По умолчанию - да.
  С 0 используется для ConsoleDetach.
*/
void InitConsole(int FirstInit=TRUE);
void InitRecodeOutTable(UINT cp=0);
void CloseConsole();
void SetFarConsoleMode(BOOL SetsActiveBuffer=FALSE);
void ChangeVideoMode(int NumLines,int NumColumns);
void ChangeVideoMode(int Maximized);
void SetVideoMode(int ConsoleMode);
void GetVideoMode(CONSOLE_SCREEN_BUFFER_INFO &csbi);
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

void Text(int X, int Y, int Color, const WCHAR *Str);
void Text(const WCHAR *Str);
void Text(int MsgId);
void VText(const WCHAR *Str);
void HiText(const WCHAR *Str,int HiColor,int isVertText=0);
#define HiVText(Str,HiColor) HiText(Str,HiColor,1)

void DrawLine(int Length,int Type, const wchar_t* UserSep=NULL);
#define ShowSeparator(Length,Type) DrawLine(Length,Type)
#define ShowUserSeparator(Length,Type,UserSep) DrawLine(Length,Type,UserSep)

WCHAR* MakeSeparator(int Length,WCHAR *DestStr,int Type=1, const wchar_t* UserSep=NULL);
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

void mprintf(const WCHAR *fmt,...);
void vmprintf(const WCHAR *fmt,...);

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

const wchar_t *GetShellAction(const wchar_t *FileName,DWORD& ImageSubsystem,DWORD& Error);
void ScrollScreen(int Count);
int ScreenSaver(int EnableExit);

string &FormatNumber(const wchar_t *Src, string &strDest, int NumDigits=0);
string &InsertCommas(unsigned __int64 li, string &strDest);

void DeleteDirTree(const wchar_t *Dir);
int GetClusterSize(const wchar_t *Root);

void InitDetectWindowedMode();
void DetectWindowedMode();
int IsWindowed();
void RestoreIcons();
void Log(char *fmt,...);
void BoxText(WORD Chr);
void BoxText(WCHAR *Str,int IsVert=0);
int FarColorToReal(int FarColor);
void ConvertCurrentPalette();
void ReopenConsole();

string &RemoveChar(string &strStr,wchar_t Target,BOOL Dup=TRUE);

wchar_t *InsertString(wchar_t *Str,int Pos,const wchar_t *InsStr,int InsSize=0);

int ReplaceStrings(string &strStr,const wchar_t *FindStr,const wchar_t *ReplStr,int Count=-1,BOOL IgnoreCase=FALSE);

#define RemoveHighlights(Str) RemoveChar(Str,L'&')

BOOL IsCaseMixed(const string &strStr);
BOOL IsCaseLower(const string &strStr);

int DeleteFileWithFolder(const wchar_t *FileName);



BOOL MoveFileThroughTemp(const wchar_t *Src, const wchar_t *Dest);

BOOL WINAPI FAR_OemToCharBuff(LPCSTR lpszSrc,LPSTR lpszDst,DWORD cchDstLength);
BOOL WINAPI FAR_CharToOemBuff(LPCSTR lpszSrc,LPSTR lpszDst,DWORD cchDstLength);
BOOL WINAPI FAR_OemToChar(LPCSTR lpszSrc,LPSTR lpszDst);
BOOL WINAPI FAR_CharToOem(LPCSTR lpszSrc,LPSTR lpszDst);

BOOL WINAPI FAR_GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer);

const wchar_t* GetLanguageString (int nID);

#define MSG(ID) GetLanguageString(ID)

int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2=NULL,const wchar_t *Str3=NULL,const wchar_t *Str4=NULL,
            INT_PTR PluginNumber=-1);
int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6=NULL,const wchar_t *Str7=NULL,
            INT_PTR PluginNumber=-1);
int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9=NULL,const wchar_t *Str10=NULL,
            INT_PTR PluginNumber=-1);
int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9,const wchar_t *Str10,
            const wchar_t *Str11,const wchar_t *Str12=NULL,const wchar_t *Str13=NULL,
            const wchar_t *Str14=NULL, INT_PTR PluginNumber=-1);

//int __cdecl MessageW (DWORD Flags,int Buttons,const char *Title, INT_PTR PluginNumber, ...);

int Message(DWORD Flags,int Buttons,const wchar_t *Title,const wchar_t * const *Items,
            int ItemsNumber,INT_PTR PluginNumber=-1);

/* $ 12.03.2002 VVM
  Новая функция - пользователь попытался прервать операцию.
  Зададим вопрос.
  Возвращает:
   FALSE - продолжить операцию
   TRUE  - прервать операцию
*/
int AbortMessage();

void SetMessageHelp(const wchar_t *Topic);
void GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2);
int ToPercent(unsigned long N1,unsigned long N2);
int ToPercent64(unsigned __int64 N1,unsigned __int64 N2);
// возвращает: 1 - LeftPressed, 2 - Right Pressed, 3 - Middle Pressed, 0 - none
int IsMouseButtonPressed();
int CmpName(const wchar_t *pattern,const wchar_t *str,int skippath=TRUE);
// обработать имя файла: сравнить с маской, масками, сгенерировать по маске
int WINAPI ProcessName(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags);

wchar_t* WINAPI QuoteSpace(wchar_t *Str);
string &QuoteSpace(string &strStr);


wchar_t* WINAPI InsertQuote(wchar_t *Str);
string& InsertQuote(string& strStr);

int ProcessGlobalFileTypes(const wchar_t *Name,int AlwaysWaitFinish);
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

int ConvertNameToFull(const wchar_t *lpwszSrc, string &strDest);
int WINAPI OldConvertNameToReal(const wchar_t *Src, string &strDest);
int WINAPI ConvertNameToReal(const wchar_t *Src, string &strDest, bool Internal=true);
void ConvertNameToShort(const wchar_t *Src, string &strDest); //BUGBUG, int
void ConvertNameToLong(const wchar_t *Src, string &strDest); //BUGBUG, int

void ChangeConsoleMode(int Mode);
void FlushInputBuffer();
void SystemSettings();
void PanelSettings();
void InterfaceSettings();
void DialogSettings();
void SetConfirmations();
void SetDizConfig();
int  IsLocalDrive(const wchar_t *Path);
void ViewerConfig(struct ViewerOptions &ViOpt,int Local=0);
void EditorConfig(struct EditorOptions &EdOpt,int Local=0);
void SetFolderInfoFiles();
void ReadConfig();
void SaveConfig(int Ask);
void SetColors();
int GetColorDialog(unsigned int &Color,bool bCentered=false,bool bAddTransparent=false);
int HiStrlen(const wchar_t *Str,BOOL Dup=TRUE);
int GetErrorString (string &strErrStr);
// Проверка на "продолжаемость" экспериментов по... например, удалению файла с разными именами!
BOOL CheckErrorForProcessed(DWORD Err);
void ShowProcessList();

wchar_t* PasteFormatFromClipboard(const wchar_t *Format);
int CopyFormatToClipboard(const wchar_t *Format,const wchar_t *Data);
wchar_t* PasteFormatFromClipboard(const wchar_t *Format);
wchar_t* WINAPI PasteFromClipboardEx(int max);
BOOL WINAPI FAR_EmptyClipboard(VOID);

int GetFileTypeByName(const wchar_t *Name);

bool CutToSlash(string &strStr, bool bInclude = false);
string &CutToNameUNC(string &strPath);
string &CutToFolderNameIfFolder(string &strPath);
const wchar_t *PointToNameUNC(const wchar_t *lpwszPath);

void SetFarTitle(const wchar_t *Title);
void LocalUpperInit();
void InitLCIDSort();
void InitKeysArray();
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
const char * __cdecl LocalStrstri(const char *str1, const char *str2);
const char * __cdecl LocalRevStrstri(const char *str1, const char *str2);

int __cdecl StrLength(const wchar_t *str);

const wchar_t * __cdecl StrStrI(const wchar_t *str1, const wchar_t *str2);
const wchar_t * __cdecl RevStrStrI(const wchar_t *str1, const wchar_t *str2);

void __cdecl UpperBuf(wchar_t *Buf, int Length);
void __cdecl LowerBuf(wchar_t *Buf, int Length);
void __cdecl StrUpper(wchar_t *s1);
void __cdecl StrLower(wchar_t *s1);

wchar_t __cdecl Upper(wchar_t Ch);
wchar_t __cdecl Lower(wchar_t Ch);
int __cdecl StrCmpNI(const wchar_t *s1, const wchar_t *s2, int n);
int __cdecl StrCmpI(const wchar_t *s1, const wchar_t *s2);
int __cdecl IsLower(wchar_t Ch);
int __cdecl IsUpper(wchar_t Ch);
int __cdecl IsAlpha(wchar_t Ch);
int __cdecl IsAlphaNum(wchar_t Ch);

int __cdecl StrCmp(const wchar_t *s1, const wchar_t *s2);
int __cdecl StrCmpN(const wchar_t *s1, const wchar_t *s2, int n);
int __cdecl NumStrCmp(const wchar_t *s1, const wchar_t *s2);
int __cdecl NumStrCmpI(const wchar_t *s1, const wchar_t *s2);

int LocalKeyToKey(int Key);
int GetShortcutFolder(int Key,string *pDestFolder, string *pPluginModule=NULL,
                      string *pPluginFile=NULL,string *pPluginData=NULL);
int SaveFolderShortcut(int Key,string *pSrcFolder,string *pPluginModule=NULL,
                       string *pPluginFile=NULL,string *pPluginData=NULL);
int GetShortcutFolderSize(int Key);
void ShowFolderShortcut();
void ShowFilter();
int DistrTableExist(void);
int GetTable(struct CharTableSet *TableSet,int AnsiText,int &TableNum,
             int &UseUnicode);

int GetTableEx ();
void DecodeStringEx (wchar_t *Str, DWORD dwCP, int Length=-1);
void EncodeStringEx (wchar_t *Str, DWORD dwCP, int Length=-1);

void DecodeString(char *Str,unsigned char *DecodeTable,int Length=-1);
void EncodeString(char *Str,unsigned char *EncodeTable,int Length=-1);
#define NullToEmpty(s) (s?s:L"")

string& CenterStr(const wchar_t *Src, string &strDest,int Length);

const wchar_t *GetCommaWord(const wchar_t *Src,string &strWord,wchar_t Separator=L',');

void ScrollBar(int X1,int Y1,int Length,unsigned long Current,unsigned long Total);

int WINAPI GetFileOwner(const wchar_t *Computer,const wchar_t *Name, string &strOwner);

void SIDCacheFlush(void);

void TransformA(unsigned char *Buffer,int &BufLen,const char *ConvStr,char TransformType);
void Transform(string &strBuffer,const wchar_t *ConvStr,wchar_t TransformType);

void GetFileDateAndTime(const wchar_t *Src,unsigned *Dst,int Separator);
void StrToDateTime(const wchar_t *CDate, const wchar_t *CTime, FILETIME &ft, int DateFormat, int DateSeparator, int TimeSeparator);

void ConvertDate(const FILETIME &ft,string &strDateText, string &strTimeText,int TimeLength,
        int Brief=FALSE,int TextMonth=FALSE,int FullYear=FALSE,int DynInit=FALSE);

void ShellOptions(int LastCommand,MOUSE_EVENT_RECORD *MouseEvent);

// Registry
void SetRegRootKey(HKEY hRootKey);

LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,const wchar_t * const ValueData, int SizeData, DWORD Type);
LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,const wchar_t * const ValueData);
LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,DWORD ValueData);
LONG SetRegKey(const wchar_t *Key,const wchar_t *ValueName,const BYTE *ValueData,DWORD ValueSize);
int GetRegKey(const wchar_t *Key,const wchar_t *ValueName, string &strValueData,const wchar_t *Default,DWORD *pType=NULL);
int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,BYTE *ValueData,const BYTE *Default,DWORD DataSize,DWORD *pType=NULL);
int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,int &ValueData,DWORD Default);
int GetRegKey(const wchar_t *Key,const wchar_t *ValueName,DWORD Default);
HKEY CreateRegKey(const wchar_t *Key);
HKEY OpenRegKey(const wchar_t *Key);
int GetRegKeySize(const wchar_t *Key,const wchar_t *ValueName);
int GetRegKeySize(HKEY hKey,const wchar_t *ValueName);
int EnumRegValue(const wchar_t *Key,DWORD Index, string &strDestName, LPBYTE SData,DWORD SDataSize,LPDWORD IData=NULL,__int64* IData64=NULL);
int EnumRegValueEx(const wchar_t *Key,DWORD Index, string &strDestName, string &strData, LPDWORD IData=NULL,__int64* IData64=NULL);
LONG SetRegKey64(const wchar_t *Key,const wchar_t *ValueName,unsigned __int64 ValueData);
int GetRegKey64(const wchar_t *Key,const wchar_t *ValueName,__int64 &ValueData,unsigned __int64 Default);
__int64 GetRegKey64(const wchar_t *Key,const wchar_t *ValueName,unsigned __int64 Default);
void DeleteRegKey(const wchar_t *Key);
void DeleteRegValue(const wchar_t *Key,const wchar_t *Value);
void DeleteKeyRecord(const wchar_t *KeyMask,int Position);
void InsertKeyRecord(const wchar_t *KeyMask,int Position,int TotalKeys);
void RenumKeyRecord(const wchar_t *KeyRoot,const wchar_t *KeyMask,const wchar_t *KeyMask0);
void DeleteKeyTree(const wchar_t *KeyName);
int CheckRegKey(const wchar_t *Key);
int CheckRegValue(const wchar_t *Key,const wchar_t *ValueName);
int DeleteEmptyKey(HKEY hRoot, const wchar_t *FullKeyName);
int EnumRegKey(const wchar_t *Key,DWORD Index,string &strDestName);
int CopyKeyTree(const wchar_t *Src,const wchar_t *Dest,const wchar_t *Skip);

void UseSameRegKey();
void CloseSameRegKey();

int RegQueryStringValueEx (HKEY hKey, const wchar_t *lpwszValueName, string &strData, const wchar_t *lpwszDefault = L"");
int RegQueryStringValue (HKEY hKey, const wchar_t *lpwszSubKey, string &strData, const wchar_t *lpwszDefault = L"");


int CheckFolder(const wchar_t *Name);
int CheckShortcutFolder(string *pTestPath,int IsHostFile, BOOL Silent=FALSE);

#if defined(__FARCONST_HPP__) && (defined(_INC_WINDOWS) || defined(_WINDOWS_) || defined(_WINDOWS_H))
DWORD NTTimeToDos(FILETIME *ft);
int Execute(const wchar_t *CmdStr,int AlwaysWaitFinish,int SeparateWindow=FALSE,int DirectRun=FALSE,int FolderRun=FALSE);
#endif

class Panel;
void ShellMakeDir(Panel *SrcPanel);
void ShellDelete(Panel *SrcPanel,int Wipe);
int  ShellSetFileAttributes(Panel *SrcPanel);
void PrintFiles(Panel *SrcPanel);
void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir=FALSE);
int  CheckUpdateAnotherPanel(Panel *SrcPanel,const wchar_t *SelName);

BOOL GetDiskSize(const wchar_t *Root,unsigned __int64 *TotalSize, unsigned __int64 *TotalFree, unsigned __int64 *UserFree);

int GetDirInfo(const wchar_t *Title,const wchar_t *DirName,unsigned long &DirCount,
               unsigned long &FileCount,unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize,unsigned __int64 &RealSize,
               unsigned long &ClusterSize,clock_t MsgWaitTime,
               FileFilter *Filter,
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
#endif


#ifdef __PLUGIN_HPP__

//----------- PLUGIN API/FSF ---------------------------------------------------
//все эти функции, за исключение sprintf/sscanf имеют тип вызова __stdcall

void __stdcall farUpperBuf(wchar_t *Buf, int Length);
void __stdcall farLowerBuf(wchar_t *Buf, int Length);
void __stdcall farStrUpper(wchar_t *s1);
void __stdcall farStrLower(wchar_t *s1);
wchar_t __stdcall farUpper(wchar_t Ch);
wchar_t __stdcall farLower(wchar_t Ch);
int __stdcall farStrCmpNI(const wchar_t *s1, const wchar_t *s2, int n);
int __stdcall farStrCmpI(const wchar_t *s1, const wchar_t *s2);
int __stdcall farIsLower(wchar_t Ch);
int __stdcall farIsUpper(wchar_t Ch);
int __stdcall farIsAlpha(wchar_t Ch);
int __stdcall farIsAlphaNum(wchar_t Ch);

int WINAPI farGetFileOwner(const wchar_t *Computer,const wchar_t *Name, wchar_t *Owner);

int WINAPI FarGetPluginDirList(INT_PTR PluginNumber,HANDLE hPlugin,
                  const wchar_t *Dir,struct PluginPanelItem **pPanelItem,
                  int *pItemsNumber);
void WINAPI FarFreePluginDirList(PluginPanelItem *PanelItem, int ItemsNumber);

int WINAPI FarMenuFn(INT_PTR PluginNumber,int X,int Y,int MaxHeight,
           DWORD Flags,const wchar_t *Title,const wchar_t *Bottom,
           const wchar_t *HelpTopic,const int *BreakKeys,int *BreakCode,
           const struct FarMenuItem *Item, int ItemsNumber);
const wchar_t* WINAPI FarGetMsgFn(INT_PTR PluginHandle,int MsgId);
int WINAPI FarMessageFn(INT_PTR PluginNumber,DWORD Flags,
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
int WINAPI TextToCharInfo(const char *Text,WORD Attr, CHAR_INFO *CharInfo, int Length, DWORD Reserved);
int WINAPI FarEditorControl(int Command,void *Param);

int WINAPI FarViewerControl(int Command,void *Param);

/* Функция вывода помощи */
BOOL WINAPI FarShowHelp(const wchar_t *ModuleName,
                        const wchar_t *HelpTopic,DWORD Flags);

/* Обертка вокруг GetString для плагинов - с меньшей функциональностью.
   Сделано для того, чтобы не дублировать код GetString.*/

int WINAPI FarInputBox(const wchar_t *Title,const wchar_t *Prompt,
                       const wchar_t *HistoryName,const wchar_t *SrcText,
                       wchar_t *DestText,int DestLength,
                       const wchar_t *HelpTopic,DWORD Flags);
/* Функция, которая будет действовать и в редакторе, и в панелях, и... */
INT_PTR WINAPI FarAdvControl(INT_PTR ModuleNumber, int Command, void *Param);
//  Функция расширенного диалога
HANDLE WINAPI FarDialogInit(INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2,
                       const wchar_t *HelpTopic, struct FarDialogItem *Item,
                       unsigned int ItemsNumber, DWORD Reserved, DWORD Flags,
                       FARWINDOWPROC Proc, LONG_PTR Param);
int WINAPI FarDialogRun(HANDLE hDlg);
void WINAPI FarDialogFree(HANDLE hDlg);
//  Функция обработки диалога по умолчанию
LONG_PTR WINAPI FarDefDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
// Посылка сообщения диалогу
LONG_PTR WINAPI FarSendDlgMessage(HANDLE hDlg,int Msg,int Param1, LONG_PTR Param2);

#endif


BOOL UnExpandEnvString(const char *Path, const char *EnvVar, char* Dest, int DestSize);
BOOL PathUnExpandEnvStr(const char *Path, char* Dest, int DestSize);

void WINAPI Unquote(string &strStr);
void WINAPI Unquote(wchar_t *Str);

void UnquoteExternal(string &strStr);

wchar_t* WINAPI RemoveLeadingSpaces(wchar_t *Str);
string& WINAPI RemoveLeadingSpaces(string &strStr);

char* WINAPI RemoveTrailingSpacesA(char *Str);
wchar_t *WINAPI RemoveTrailingSpaces(wchar_t *Str);
string& WINAPI RemoveTrailingSpaces(string &strStr);

wchar_t* WINAPI RemoveExternalSpaces(wchar_t *Str);
string & WINAPI RemoveExternalSpaces(string &strStr);
string & WINAPI RemoveUnprintableCharacters(string &strStr);

wchar_t* __stdcall TruncStr(wchar_t *Str,int MaxLength);
string& __stdcall TruncStr(string &strStr,int MaxLength);

wchar_t* WINAPI TruncStrFromEnd(wchar_t *Str,int MaxLength);
string& __stdcall TruncStrFromEnd(string &strStr, int MaxLength);

wchar_t* __stdcall TruncPathStr(wchar_t *Str, int MaxLength);
string& __stdcall TruncPathStr(string &strStr, int MaxLength);

wchar_t* WINAPI QuoteSpaceOnly(wchar_t *Str);
string& WINAPI QuoteSpaceOnly(string &strStr);
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
BOOL IsWordDiv(const struct CharTableSet *TableSet, const wchar_t *WordDiv, wchar_t Chr);

const wchar_t* __stdcall PointToName(const wchar_t *lpwszPath);
const wchar_t* __stdcall PointToFolderNameIfFolder(const wchar_t *lpwszPath);
const wchar_t* PointToExt(const wchar_t *lpwszPath);

BOOL  TestParentFolderName(const wchar_t *Name);

BOOL  AddEndSlash(string &strPath, wchar_t TypeSlash);
BOOL  AddEndSlash(string &strPath);

BOOL  AddEndSlash(wchar_t *Path, wchar_t TypeSlash);
BOOL  WINAPI AddEndSlash(wchar_t *Path);

BOOL  WINAPI DeleteEndSlash(string &strPath,bool allendslash=false);

string& ReplaceSlashToBSlash(string& strStr);

int __digit_cnt_0(const wchar_t* s, const wchar_t** beg);
wchar_t *WINAPI FarItoa(int value, wchar_t *string, int radix);
__int64 WINAPI FarAtoi64(const wchar_t *s);
wchar_t *WINAPI FarItoa64(__int64 value, wchar_t *string, int radix);
int WINAPI FarAtoi(const wchar_t *s);
void WINAPI FarQsort(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
void WINAPI FarQsortEx(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *),void*);

int WINAPI CopyToClipboard(const wchar_t *Data);
wchar_t* WINAPI PasteFromClipboard(void);


wchar_t* InternalPasteFromClipboard(int AnsiMode);
wchar_t* InternalPasteFromClipboardEx(int max,int AnsiMode);
int InternalCopyToClipboard(const wchar_t *Data,int AnsiMode);


int __stdcall GetString(
		const wchar_t *Title,
		const wchar_t *SubTitle,
		const wchar_t *HistoryName,
		const wchar_t *SrcText,
		string &strDestText,
		int DestLength,
		const wchar_t *HelpTopic = NULL,
		DWORD Flags = 0,
		int *CheckBoxValue = NULL,
		const wchar_t *CheckBoxText = NULL
		);

int WINAPI GetNameAndPassword(const wchar_t *Title,string &strUserName, string &strPassword, const wchar_t *HelpTopic,DWORD Flags);

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


char* WINAPI XlatA(char *Line,
                    int StartPos,
                    int EndPos,
                    const struct CharTableSet *TableSet,
                    DWORD Flags);

wchar_t* WINAPI Xlat(wchar_t *Line,
                    int StartPos,
                    int EndPos,
                    const struct CharTableSet *TableSet,
                    DWORD Flags);

#ifdef __cplusplus
extern "C" {
#endif

void *WINAPI FarBsearch(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));

typedef int  (WINAPI *FRSUSERFUNCW)(const FAR_FIND_DATA *FData,const wchar_t *FullName,void *param);
void WINAPI FarRecursiveSearch(const wchar_t *initdir,const wchar_t *mask,FRSUSERFUNCW func,DWORD flags,void *param);

wchar_t* __stdcall FarMkTemp(wchar_t *Dest, DWORD size, const wchar_t *Prefix);
string& FarMkTempEx(string &strDest, const wchar_t *Prefix=NULL, BOOL WithPath=TRUE);

void CreatePath(string &strPath);

/* $ 15.02.2002 IS
   Установка нужного диска и каталога и установление соответствующей переменной
   окружения. В случае успеха возвращается не ноль.
   Если ChangeDir==FALSE, то не меняем текущий  диск, а только устанавливаем
   переменные окружения.
*/
BOOL FarChDir(const wchar_t *NewDir,BOOL ChangeDir=TRUE);

// обертка вокруг функции получения текущего пути.
// для локального пути делает букву диска в uppercase
DWORD FarGetCurDir(string &strBuffer);

void WINAPI DeleteBuffer(void* Buffer);

#ifdef __cplusplus
};
#endif

/* <Логи ***************************************************
*/
void SysLog(int l);
void SysLog(const wchar_t *fmt,...);
void SysLog(int l,const wchar_t *fmt,...); ///
void SysLogLastError(void);
void ShowHeap();
void CheckHeap(int NumLine);

string __FARKEY_ToName(int Key);
#define _FARKEY_ToName(K) (const wchar_t*)__FARKEY_ToName(K)
string __MCODE_ToName(int OpCode);
#define _MCODE_ToName(K) (const wchar_t*)__MCODE_ToName(K)
string __VK_KEY_ToName(int VkKey);
#define _VK_KEY_ToName(K) (const wchar_t*)__VK_KEY_ToName(K)
string __ECTL_ToName(int Command);
#define _ECTL_ToName(K) (const wchar_t*)__ECTL_ToName(K)
string __EE_ToName(int Command);
#define _EE_ToName(K) (const wchar_t*)__EE_ToName(K)
string __EEREDRAW_ToName(int Command);
#define _EEREDRAW_ToName(K) (const wchar_t*)__EEREDRAW_ToName(K)
string __ESPT_ToName(int Command);
#define _ESPT_ToName(K) (const wchar_t*)__ESPT_ToName(K)
string __VE_ToName(int Command);
#define _VE_ToName(K) (const wchar_t*)__VE_ToName(K)
string __FCTL_ToName(int Command);
#define _FCTL_ToName(K) (const wchar_t*)__FCTL_ToName(K)
string __DLGMSG_ToName(int Msg);
#define _DLGMSG_ToName(K) (const wchar_t*)__DLGMSG_ToName(K)
string __ACTL_ToName(int Command);
#define _ACTL_ToName(K) (const wchar_t*)__ACTL_ToName(K)
string __VCTL_ToName(int Command);
#define _VCTL_ToName(K) (const wchar_t*)__VCTL_ToName(K)
string __INPUT_RECORD_Dump(INPUT_RECORD *Rec);
#define _INPUT_RECORD_Dump(K) (const wchar_t*)__INPUT_RECORD_Dump(K)
string __MOUSE_EVENT_RECORD_Dump(MOUSE_EVENT_RECORD *Rec);
#define _MOUSE_EVENT_RECORD_Dump(K) (const wchar_t*)__MOUSE_EVENT_RECORD_Dump(K)
string __SysLog_LinearDump(LPBYTE Buf,int SizeBuf);
#define _SysLog_LinearDump(B,S) (const wchar_t*)__SysLog_LinearDump((B),(S))

void GetOpenPluginInfo_Dump(const wchar_t *Title,const struct OpenPluginInfo *Info,FILE *fp);
void INPUT_RECORD_DumpBuffer(FILE *fp=NULL);
void PanelViewSettings_Dump(const wchar_t *Title,const struct PanelViewSettings &ViewSettings,FILE *fp=NULL);
void PluginsStackItem_Dump(const wchar_t *Title,const struct PluginsStackItem *StackItems,int ItemNumber,FILE *fp=NULL);
void SaveScreenDumpBuffer(const wchar_t *Title,const CHAR_INFO *Buffer,int X1,int Y1,int X2,int Y2,int RealScreen,FILE *fp=NULL);
class Manager;
void ManagerClass_Dump(const wchar_t *Title,const Manager *m=NULL,FILE *fp=NULL);
void GetVolumeInformation_Dump(const wchar_t *Title,LPCWSTR lpRootPathName,LPCWSTR lpVolumeNameBuffer,DWORD nVolumeNameSize,
                                           DWORD lpVolumeSerialNumber, DWORD lpMaximumComponentLength, DWORD lpFileSystemFlags,
                                           LPCWSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize,FILE *fp=NULL);

void WIN32_FIND_DATA_Dump(const wchar_t *Title,const WIN32_FIND_DATA &fd,FILE *fp=NULL);

#if defined(SYSLOG_FARSYSLOG)
#ifdef __cplusplus
extern "C" {
#endif
void WINAPIV _export FarSysLog(const wchar_t *ModuleName,int Level,char *fmt,...);
void WINAPI  _export FarSysLogDump(const wchar_t *ModuleName,DWORD StartAddress,LPBYTE Buf,int SizeBuf);
void WINAPI _export FarSysLog_INPUT_RECORD_Dump(const wchar_t *ModuleName,INPUT_RECORD *rec);
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

void SysLogDump(const wchar_t *Title,DWORD StartAddress,LPBYTE Buf,int SizeBuf,FILE *fp=NULL);

FILE *OpenLogStream(const wchar_t *file);

#define L_ERR      1
#define L_WARNING  2
#define L_INFO     3
#define L_DEBUG1   4
#define L_DEBUG2   5
#define L_DEBUG3   6

class CleverSysLog{ // ;-)
  public:
    CleverSysLog(const wchar_t *Title=NULL);
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

int GetEncryptFunctions(void);

int ESetFileAttributes(const wchar_t *Name,DWORD Attr,int SkipMode=-1);
int ESetFileCompression(const wchar_t *Name,int State,DWORD FileAttr,int SkipMode=-1);
int ESetFileEncryption(const wchar_t *Name,int State,DWORD FileAttr,int SkipMode=-1,int Silent=0);
#define ESetFileEncryptionSilent(Name,State,FileAttr,SkipMode) ESetFileEncryptionW(Name,State,FileAttr,SkipMode,1)
int ESetFileTime(const wchar_t *Name,FILETIME *LastWriteTime,
                  FILETIME *CreationTime,FILETIME *LastAccessTime,
                  DWORD FileAttr,int SkipMode=-1);

//int ConvertWildcards(const char *Src,char *Dest, int SelectedFolderNameLength);
int ConvertWildcards(const wchar_t *SrcName,string &strDest, int SelectedFolderNameLength);

const wchar_t* WINAPI PrepareOSIfExist(const wchar_t *CmdLine);
BOOL IsBatchExtType(const wchar_t *ExtPtr);
#ifdef ADD_GUI_CHECK
BOOL BatchFileExist(const char *FileName,char *DestName,int SizeDestName);
#endif

int WINAPI GetSearchReplaceString (
         int IsReplaceMode,
         string *pSearchStr,
         string *pReplaceStr,
         const wchar_t *TextHistoryName,
         const wchar_t *ReplaceHistoryName,
         int *Case,
         int *WholeWords,
         int *Reverse,
         int *SelectFound);


BOOL WINAPI KeyMacroToText(int Key,string &strKeyText0);
int WINAPI KeyNameMacroToKey(const wchar_t *Name);
int TranslateKeyToVK(int Key,int &VirtKey,int &ControlState,INPUT_RECORD *rec=NULL);
int WINAPI KeyNameToKey(const wchar_t *Name);
BOOL WINAPI KeyToText (int Key, string &strKeyText);
int WINAPI InputRecordToKey(const INPUT_RECORD *Rec);
DWORD GetInputRecord(INPUT_RECORD *rec,bool ExcludeMacro=false);
DWORD PeekInputRecord(INPUT_RECORD *rec);
DWORD CalcKeyCode(INPUT_RECORD *rec,int RealKey,int *NotMacros=NULL);
DWORD WaitKey(DWORD KeyWait=(DWORD)-1,DWORD delayMS=0);
int SetFLockState(UINT vkKey, int State);
BOOL FARGetKeybLayoutNameW (string &strDest);
int WriteInput(int Key,DWORD Flags=0);
int IsNavKey(DWORD Key);
int IsShiftKey(DWORD Key);
int CheckForEsc();
int CheckForEscSilent();
int ConfirmAbortOp();

// Получить из имени диска RemoteName
string &DriveLocalToRemoteName(int DriveType,wchar_t Letter,string &strDest);

void __PrepareKMGTbStr(void);
string& __stdcall FileSizeToStr(string &strDestStr, unsigned __int64 Size, int Width=-1, int ViewFlags=COLUMN_COMMAS);


DWORD WINAPI FarGetLogicalDrives(void);

string &Add_PATHEXT(string &strDest);

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl qsortex(char *base, size_t nel, size_t width,
            int (__cdecl *comp_fp)(const void *, const void *,void*), void *user);

char * __cdecl farmktemp(char *temp);
char * __cdecl xstrncat (char * dest,const char * src,size_t maxlen);
wchar_t * __cdecl xwcsncat (wchar_t * dest,const wchar_t * src,size_t maxlen);
char * __cdecl xstrncpy (char * dest,const char * src,size_t maxlen);
wchar_t * __cdecl xwcsncpy (wchar_t * dest,const wchar_t * src,size_t maxlen);
char * __cdecl xf_strdup (const char * string);
wchar_t * __cdecl xf_wcsdup (const wchar_t * string);
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

void GenerateWINDOW_BUFFER_SIZE_EVENT(int Sx=-1, int Sy=-1);

void PrepareStrFTime();
size_t WINAPI StrFTime(string &strDest, const wchar_t *Format,const tm *t);
size_t MkStrFTime(string &strDest, const wchar_t *Fmt=NULL);

BOOL WINAPI GetMenuHotKey(string &strHotKey,int LenHotKey,
                          const wchar_t *DlgHotKeyTitle,
                          const wchar_t *DlgHotKeyText,
                          const wchar_t *DlgPluginTitle,  // заголовок
                          const wchar_t *HelpTopic,
                          const wchar_t *RegKey,
                          const wchar_t *RegValueName);

string& WINAPI FarFormatText(const wchar_t *SrcText, int Width, string &strDestText, const wchar_t* Break, DWORD Flags);


int PathMayBeAbsolute(const wchar_t *Src);

string& PrepareDiskPath(string &strPath, BOOL CheckFullPath=TRUE);

//   TableSet - указатель на таблицы перекодировки (если отсутствует,
//              то кодировка - OEM)
//   WordDiv  - набор разделителей слова в кодировке OEM
// возвращает указатель на начало слова
const wchar_t * const CalcWordFromString(const wchar_t *Str,int CurPos,int *Start,int *End,const struct CharTableSet *TableSet, const wchar_t *WordDiv);

long filelen(FILE *FPtr);
__int64 filelen64(FILE *FPtr);
__int64 ftell64(FILE *fp);
int fseek64 (FILE *fp, __int64 offset, int whence);

BOOL IsDiskInDrive(const wchar_t *Drive);

CDROM_DeviceCaps GetCDDeviceCaps(HANDLE hDevice);
UINT GetCDDeviceTypeByCaps(CDROM_DeviceCaps caps);
BOOL IsDriveTypeCDROM(UINT DriveType);
UINT FAR_GetDriveType(const wchar_t *RootDir,CDROM_DeviceCaps *caps=NULL,DWORD Detect=0);

bool PathPrefix(const wchar_t *Path);
BOOL IsNetworkPath(const wchar_t *Path);
BOOL IsLocalPath(const wchar_t *Path);
BOOL IsLocalRootPath(const wchar_t *Path);
BOOL IsLocalPrefixPath(const wchar_t *Path);
BOOL IsLocalVolumePath(const wchar_t *Path);
BOOL IsLocalVolumeRootPath(const wchar_t *Path);

BOOL RunGraber(void);

BOOL ProcessOSAliases(string &strStr);

int PartCmdLine(const wchar_t *CmdStr, string &strNewCmdStr, string &strNewCmdPar);

void initMacroVarTable(int global);
void doneMacroVarTable(int global);
bool checkMacroConst(const wchar_t *name);
const wchar_t *eStackAsString(int Pos=0);

int __parseMacroString(DWORD *&CurMacroBuffer, int &CurMacroBufferSize, const wchar_t *BufPtr);
BOOL __getMacroParseError(string *strErrMessage1, string *strErrMessage2,string *strErrMessage3);
int  __getMacroErrorCode(int *nErr=NULL);

int _MakePath1(DWORD Key,string &strPathName, const wchar_t *Param2,int ShortNameAsIs=TRUE);

string &CurPath2ComputerName(const wchar_t *CurDir, string &strComputerName);
void ConvertNameToUNC(string &strFileName);
int CheckDisksProps(const wchar_t *SrcPath,const wchar_t *DestPath,int CheckedType);


#define CP_UNICODE 1200 //MSDN
#define CP_REVERSEBOM 65534
#define CP_AUTODETECT -1

int GetFileFormat (FILE *file, bool *pSignatureFound = NULL);


//winapi wrappers

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

HANDLE apiFindFirstFile (const wchar_t *lpwszFileName, FAR_FIND_DATA_EX *pFindFileData,bool ScanSymLink=true);
BOOL apiFindNextFile (HANDLE hFindFile, FAR_FIND_DATA_EX *pFindFileData);
BOOL apiFindClose(HANDLE hFindFile);

void apiFindDataToDataEx (const FAR_FIND_DATA *pSrc, FAR_FIND_DATA_EX *pDest);
void apiFindDataExToData (const FAR_FIND_DATA_EX *pSrc, FAR_FIND_DATA *pDest);
void apiFreeFindData (FAR_FIND_DATA *pData);

BOOL apiGetFindDataEx (const wchar_t *lpwszFileName, FAR_FIND_DATA_EX *pFindData,bool ScanSymLink=true);
BOOL apiGetFileSize (HANDLE hFile, unsigned __int64 *pSize);

BOOL apiSetFilePointerEx (
		HANDLE hFile,
		LARGE_INTEGER liDistanceToMove,
		PLARGE_INTEGER lpNewFilePointer,
		DWORD dwMoveMethod
		);

//junk

BOOL apiDeleteFile (const wchar_t *lpwszFileName);
BOOL apiRemoveDirectory (const wchar_t *DirName);

HANDLE apiCreateFile (
		const wchar_t *lpwszFileName,     // pointer to name of the file
		DWORD dwDesiredAccess,  // access (read-write) mode
		DWORD dwShareMode,      // share mode
		LPSECURITY_ATTRIBUTES lpSecurityAttributes, // pointer to security attributes
		DWORD dwCreationDistribution, // how to create
		DWORD dwFlagsAndAttributes,   // file attributes
		HANDLE hTemplateFile          // handle to file with attributes to copy
		);

BOOL apiCopyFile (
		const wchar_t *lpwszExistingFileName, // pointer to name of an existing file
		const wchar_t *lpwszNewFileName,  // pointer to filename to copy to
		BOOL bFailIfExists  // flag for operation if file exists
		);

BOOL apiCopyFileEx (
		const wchar_t *lpExistingFileName,
		const wchar_t *lpNewFileName,
		void *lpProgressRoutine,
		LPVOID lpData,
		LPBOOL pbCancel,
		DWORD dwCopyFlags
		);


BOOL apiMoveFile (
		const wchar_t *lpwszExistingFileName, // address of name of the existing file
		const wchar_t *lpwszNewFileName   // address of new name for the file
		);

BOOL apiMoveFileEx (
		const wchar_t *lpwszExistingFileName, // address of name of the existing file
		const wchar_t *lpwszNewFileName,   // address of new name for the file
		DWORD dwFlags   // flag to determine how to move file
		);

string& HiText2Str(string& strDest, const wchar_t *Str);

__int64 FileTimeDifference(const FILETIME *a, const FILETIME* b);
unsigned __int64 FileTimeToUI64(const FILETIME *ft);

#endif  // __FARFUNC_HPP__
