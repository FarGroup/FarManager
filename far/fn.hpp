#ifndef __FARFUNC_HPP__
#define __FARFUNC_HPP__
/*
fn.hpp

Описания функций

*/

/* Revision: 1.43 30.12.2000 $ */

/*
Modify:
  30.12.2000 SVS
   + Функции работы с атрибутами файлов "опубликованы"
  26.12.2000 SVS
   + KeyMacroToText()
  14.12.2000 SVS
   + EjectVolume()
  02.11.2000 OT
   ! Введение проверки на длину буфера, отведенного под имя файла.
  25.10.2000 IS
   ! Изменил имя параметра в FarMkTemp с Template на Prefix
  23.10.2000 SVS
   ! Узаконненая версия SysLog :-)
  20.10.2000 SVS
   ! ProcessName: Flags должен быть DWORD, а не int
  20.10.2000 SVS
   + SysLog
  16.10.2000 tran
   + PasteFromClipboardEx(int max);
  09.10.2000 IS
   + ProcessName
  27.09.2000 SVS
   + FarViewerControl
  27.09.2000 skv
   + DeleteBuffer. Удалять то, что вернул PasteFromClipboard.
  24.09.2000 SVS
   + Функция KeyNameToKey - получение кода клавиши по имени
     Если имя не верно или нет такого - возвращается -1
  20.09.2000 SVS
   ! удалил FolderPresent (блин, совсем крышу сорвало :-(
  19.09.2000 SVS
   + функция FolderPresent - "сужествует ли каталог"
  18.09.2000 SVS
   ! Функция FarDialogEx имеет 2 дополнительных параметра (Future)
   ! FarRecurseSearch -> FarRecursiveSearch
  15.09.2000 IS
   + Функция CheckRegValue - возвращает FALSE, если указанная переменная не
     содержит данные или размер данных равен нулю.
   + Функция DistrTableExist - проверяет, установлена ли таблица с
     распределением частот символов, возвращает TRUE в случае успеха
  14.09.2000 SVS
    + Функция FarMkTemp - получение имени временного файла с полным путем.
  12.09.2000 SVS
    ! FarShowHelp возвращает BOOL
  10.09.2000 SVS
    ! KeyToText возвращает BOOL
  10.09.2000 tran 1.23
    + FSF/FarRecurseSearch
  10.09.2000 SVS
    ! Наконец-то нашлось приемлемое имя для QWERTY -> Xlat.
  08.09.2000 SVS
    ! QWERTY -> Transliterate
  07.09.2000 SVS
    ! Функции GetFileOwner и GetNumberOfLinks имеют вызов WINAPI
    + FarBsearch
  05.09.2000 SVS
    + QWERTY-перекодировка!
      На основе плагина EditSwap by SVS :-)))
  31.08.2000 tran
    + FSF/FarInputRecordToKey
  29.08.2000 SVS
    + Дополнительный параметр у Message* - номер плагина.
  28.08.2000 SVS
    + Модификация вызова под WINAPI у функций Local*
    ! уточнение для FarQsort
    ! Не FarAtoa64, но FarAtoi64
    + FarItoa64
  24.08.2000 SVS
    + Пераметр у фунции WaitKey - возможность ожидать конкретную клавишу
  23.08.2000 SVS
    ! Все Flags приведены к одному виду -> DWORD.
      Модифицированы:
        * функции   FarMenuFn, FarMessageFn, FarShowHelp
        * структуры FarListItem, FarDialogItem
  23.08.2000 SVS
    + Уточнения (комментарий) для IsMouseButtonPressed()
  18.08.2000 tran
    + Flags parameter in FarShowHelp
  14.08.2000 SVS
    + Функции семейства seek под __int64
  01.08.2000 SVS
    ! Функция ввода строки GetString имеет один параметр для всех флагов
    ! дополнительный параметра у KeyToText - размер данных
  31.07.2000 SVS
    ! функция GetString имеет еще один параметр - расширение среды
  24.07.2000 SVS
    ! Все функции, попадающие в разряд FSF должны иметь WINAPI!!!
  23.07.2000 SVS
    + Функция FarDialogEx - расширенный диалог
    + Функция FarDefDlgProc обработки диалога по умолчанию
    + Функция FarSendDlgMessage - посылка сообщения диалогу
    + Text(int X, int Y, int Color, char *Str);
    + Text(int X, int Y, int Color, int MsgId);
  18.07.2000 tran 1.06
    ! изменил тип аргумента у ScrollBar с 'int' на 'unsigned long'
      нужно для Viewer
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  07.07.2000 IS
    + SetHighlighting из main.cpp
  07.07.2000 SVS
    + Дополнительная функция обработки строк: RemoveExternalSpaces
  06.07.2000 IS
    + Функция FarAdvControl
  05.07.2000 SVS
    + Функция ExpandEnvironmentStr
  03.07.2000 IS
    + Функция вывода помощи
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/


/* $ 07.07.2000 IS
   Функция перешла сюда из main.cpp
*/
void SetHighlighting();
/* IS $ */
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
void Text(int X, int Y, int Color, char *Str);
void Text(char *Str);
void Text(int X, int Y, int Color, int MsgId);
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
/* $ 24.08.2000 SVS
 + Пераметр у фунции WaitKey - возможность ожидать конкретную клавишу
*/
void WaitKey(int KeyWait=-1);
/* SVS $ */
int CopyKeyTree(char *Src,char *Dest,char *Skip);
void WriteInput(int Key);
void ShowTime(int ShowAlways);
int GetDateFormat();
int GetDateSeparator();
int GetTimeSeparator();
char* GetShellAction(char *FileName);
void ScrollScreen(int Count);
int ScreenSaver(int EnableExit);
void InsertCommas(unsigned long Number,char *Dest);
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
bool GetSubstName(char *LocalName,char *SubstName,int SubstSize);
void BoxText(char *Str);
int FarColorToReal(int FarColor);
void ReopenConsole();
void DeleteRegKey(char *Key);
void DeleteRegValue(char *Key,char *Value);
void DeleteKeyRecord(char *KeyMask,int Position);
void InsertKeyRecord(char *KeyMask,int Position,int TotalKeys);
void DeleteKeyTree(char *KeyName);
void _cdecl CheckReg(void *Param);
int CheckRegKey(char *Key);
/* 15.09.2000 IS
   Возвращает FALSE, если указанная переменная не содержит данные
   или размер данных равен нулю.
*/
int CheckRegValue(char *Key,char *ValueName);
/* IS $ */
int EnumRegKey(char *Key,DWORD Index,char *DestName,DWORD DestSize);
int IsFolderNotEmpty(char *Name);
void RemoveHighlights(char *Str);
int IsCaseMixed(char *Str);
int IsCaseLower(char *Str);
int DeleteFileWithFolder(char *FileName);
char* FarMSG(int MsgID);
/* $ 29.08.2000 SVS
   Дополнительный параметр у Message* - номер плагина.
*/
int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2,
            char *Str3,char *Str4,char *Str5,char *Str6=NULL,char *Str7=NULL,
            int PluginNumber=-1);
int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2,
            char *Str3,char *Str4,char *Str5,char *Str6,
            char *Str7,char *Str8,char *Str9=NULL,char *Str10=NULL,
            int PluginNumber=-1);
int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2,
            char *Str3,char *Str4,char *Str5,char *Str6,char *Str7,
            char *Str8,char *Str9,char *Str10,char *Str11,char *Str12=NULL,
            char *Str13=NULL,char *Str14=NULL,
            int PluginNumber=-1);
int Message(int Flags,int Buttons,char *Title,char *Str1,char *Str2=NULL,
            char *Str3=NULL,char *Str4=NULL,
            int PluginNumber=-1);
/* SVS $*/
void SetMessageHelp(char *Topic);
void GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2);
int ToPercent(unsigned long N1,unsigned long N2);
// возвращает: 1 - LeftPressed, 2 - Right Pressed, 3 - Middle Pressed, 0 - none
int IsMouseButtonPressed();
int CmpName(char *pattern,char *string,int skippath=TRUE);
int CheckForEsc();
/* $ 09.10.2000 IS
    + Новая функция для обработки имени файла
*/
// обработать имя файла: сравнить с маской, масками, сгенерировать по маске
int WINAPI ProcessName(char *param1, char *param2, DWORD flags);
/* IS $ */
void ShowHeap();
void CheckHeap(int NumLine=0);
char* QuoteSpace(char *Str);
int ProcessGlobalFileTypes(char *Name,int AlwaysWaitFinish);
int ProcessLocalFileTypes(char *Name,char *ShortName,int Mode,int AlwaysWaitFinish);
void ProcessExternal(char *Command,char *Name,char *ShortName,int AlwaysWaitFinish);
int SubstFileName(char *Str,char *Name,char *ShortName,
                  char *ListName=NULL,char *ShortListName=NULL,
                  int IgnoreInput=FALSE,char *CmdLineDir=NULL);
void EditFileTypes(int MenuPos);
void ProcessUserMenu(int EditMenu);
int ConvertNameToFull(char *Src,char *Dest, int DestSize);
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
int GetErrorString(char *ErrStr);
void ShowProcessList();
int CopyFormatToClipboard(char *Format,char *Data);
char* PasteFormatFromClipboard(char *Format);
/* $ 16.10.2000 tran
  параметер - ограничение по длины */
char* WINAPI PasteFromClipboardEx(int max);
/* tran $ */

int GetFileTypeByName(char *Name);
void SetFarTitle(char *Title);
void LocalUpperInit();
/* $ 28.08.2000 SVS
   Модификация вызова под WINAPI
*/
int WINAPI LocalIslower(int Ch);
int WINAPI LocalIsupper(int Ch);
int WINAPI LocalIsalpha(int Ch);
int WINAPI LocalIsalphanum(int Ch);
int WINAPI LocalUpper(int LowerChar);
void WINAPI LocalUpperBuf(char *Buf,int Length);
void WINAPI LocalLowerBuf(char *Buf,int Length);
int WINAPI LocalLower(int UpperChar);
void WINAPI LocalStrupr(char *s1);
void WINAPI LocalStrlwr(char *s1);
int WINAPI LStricmp(char *s1,char *s2);
int WINAPI LStrnicmp(char *s1,char *s2,int n);
/* SVS $ */
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
/* 15.09.2000 IS
   Проверяет, установлена ли таблица с распределением частот символов
*/
int DistrTableExist(void);
/* IS $ */
int GetTable(struct CharTableSet *TableSet,int AnsiText,int &TableNum,
             int &UseUnicode);
void DecodeString(char *Str,unsigned char *DecodeTable,int Length=-1);
void EncodeString(char *Str,unsigned char *EncodeTable,int Length=-1);
char *NullToEmpty(char *Str);
void CenterStr(char *Src,char *Dest,int Length);
char *GetCommaWord(char *Src,char *Word);
void ScrollBar(int X1,int Y1,int Length,unsigned long Current,unsigned long Total);
int WINAPI GetFileOwner(char *Computer,char *Name,char *Owner);
int WINAPI GetNumberOfLinks(char *Name);
void ShowSeparator(int Length);
void UseSameRegKey();
void CloseSameRegKey();

#if defined(_INC_WINDOWS) || defined(_WINDOWS_)
int GetInputRecord(INPUT_RECORD *rec);
int PeekInputRecord(INPUT_RECORD *rec);
int CalcKeyCode(INPUT_RECORD *rec,int RealKey);
void ConvertDate(FILETIME *ft,char *DateText,char *TimeText,int TimeLength,
                 int Brief=FALSE,int TextMonth=FALSE,int FullYear=FALSE);
void ShellOptions(int LastCommand,MOUSE_EVENT_RECORD *MouseEvent);
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
#endif


#if defined(__FARCONST_HPP__) && (defined(_INC_WINDOWS) || defined(_WINDOWS_))
UDWORD NTTimeToDos(FILETIME *ft);
int Execute(char *CmdStr,int AlwaysWaitFinish,int SeparateWindow=FALSE,
            int DirectRun=FALSE);
#endif


#ifdef __PANEL_HPP__
void ShellMakeDir(Panel *SrcPanel);
void ShellDelete(Panel *SrcPanel,int Wipe);
void ShellSetFileAttributes(Panel *SrcPanel);
void PrintFiles(Panel *SrcPanel);
#endif

#ifdef __INT64_HPP__
BOOL GetDiskSize(char *Root,int64 *TotalSize,int64 *TotalFree,int64 *UserFree);
void InsertCommas(int64 li,char *Dest);
int GetDirInfo(char *Title,char *DirName,unsigned long &DirCount,
               unsigned long &FileCount,int64 &FileSize,
               int64 &CompressedFileSize,int64 &RealSize,
               unsigned long &ClusterSize,clock_t MsgWaitTime,
               int EnhBreak);
int GetPluginDirInfo(HANDLE hPlugin,char *DirName,unsigned long &DirCount,
               unsigned long &FileCount,int64 &FileSize,
               int64 &CompressedFileSize);
#endif

#if defined(__BORLANDC__)
 #ifdef __STDIO_H
 long filelen(FILE *FPtr);
 int DetectTable(FILE *SrcFile,struct CharTableSet *TableSet,int &TableNum);
 #endif
#else
 long filelen(FILE *FPtr);
 int DetectTable(FILE *SrcFile,struct CharTableSet *TableSet,int &TableNum);
#endif

#ifdef __PLUGIN_HPP__
int PrepareTable(struct CharTableSet *TableSet,int TableNum);
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
int WINAPI FarGetPluginDirList(int PluginNumber,HANDLE hPlugin,
                  char *Dir,struct PluginPanelItem **pPanelItem,
                  int *pItemsNumber);
int WINAPI FarMenuFn(int PluginNumber,int X,int Y,int MaxHeight,
           DWORD Flags,char *Title,char *Bottom,char *HelpTopic,
           int *BreakKeys,int *BreakCode,struct FarMenuItem *Item,
           int ItemsNumber);
int WINAPI FarDialogFn(int PluginNumber,int X1,int Y1,int X2,int Y2,
           char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber);
char* WINAPI FarGetMsgFn(int PluginNumber,int MsgId);
int WINAPI FarMessageFn(int PluginNumber,DWORD Flags,
           char *HelpTopic,char **Items,int ItemsNumber,
           int ButtonsNumber);
int WINAPI FarControl(HANDLE hPlugin,int Command,void *Param);
HANDLE WINAPI FarSaveScreen(int X1,int Y1,int X2,int Y2);
void WINAPI FarRestoreScreen(HANDLE hScreen);
int WINAPI FarGetDirList(char *Dir,struct PluginPanelItem **pPanelItem,
           int *pItemsNumber);
void WINAPI FarFreeDirList(struct PluginPanelItem *PanelItem);
int WINAPI FarViewer(char *FileName,char *Title,int X1,int Y1,int X2,
           int Y2,DWORD Flags);
int WINAPI FarEditor(char *FileName,char *Title,int X1,int Y1,int X2,
           int Y2,DWORD Flags,int StartLine,int StartChar);
int WINAPI FarCmpName(char *pattern,char *string,int skippath);
int WINAPI FarCharTable(int Command,char *Buffer,int BufferSize);
void WINAPI FarText(int X,int Y,int Color,char *Str);
int WINAPI FarEditorControl(int Command,void *Param);

int WINAPI FarViewerControl(int Command,void *Param);

/* $ 18.08.2000 tran
   add Flags parameter */
/* $ 03.07.2000 IS
  Функция вывода помощи
*/
BOOL WINAPI FarShowHelp(char *ModuleName, char *HelpTopic,DWORD Flags);
/* IS $ */
/* tran 18.08.2000 $ */

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
      char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber,
      DWORD Reserved, DWORD Flags,
      FARWINDOWPROC Proc,long Param);
//  Функция обработки диалога по умолчанию
long WINAPI FarDefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
// Посылка сообщения диалогу
long WINAPI FarSendDlgMessage(HANDLE hDlg,int Msg,int Param1, long Param2);

/* SVS $ */
#endif


/* $ 24.07.2000 SVS
   Те функции, которые попадают в FSF
   Должны иметь WINAPI
*/
/* $ 05.07.2000 SVS
   Расширение переменной среды
*/
DWORD WINAPI ExpandEnvironmentStr(char *src, char *dst, size_t size=8192);
/* SVS $ */
void WINAPI Unquote(char *Str);

/* $ 07.07.2000 SVS
   + удалить пробелы снаружи
   ! изменен тип возврата
*/
char* WINAPI RemoveLeadingSpaces(char *Str);
char* WINAPI RemoveTrailingSpaces(char *Str);
char* WINAPI RemoveExternalSpaces(char *Str);
/* SVS $ */
char* WINAPI TruncStr(char *Str,int MaxLength);
char* WINAPI TruncPathStr(char *Str,int MaxLength);
char* WINAPI QuoteSpaceOnly(char *Str);
char* WINAPI PointToName(char *Path);
void  WINAPI GetPathRoot(char *Path,char *Root);
int  WINAPI AddEndSlash(char *Path);
char *WINAPI FarItoa(int value, char *string, int radix);
__int64 WINAPI FarAtoi64(const char *s);
char *WINAPI FarItoa64(__int64 value, char *string, int radix);
int WINAPI FarAtoi(const char *s);
void WINAPI FarQsort(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
int WINAPIV FarSprintf(char *buffer,const char *format,...);
int WINAPIV FarSscanf(const char *buffer, const char *format,...);
int WINAPI CopyToClipboard(char *Data);
char* WINAPI PasteFromClipboard(void);
/* $ 01.08.2000 SVS
   ! Функция ввода строки GetString имеет один параметр для всех флагов
*/
/* $ 31.07.2000 SVS
    ! функция GetString имеет еще один параметр - расширение среды
*/
int WINAPI GetString(char *Title,char *SubTitle,char *HistoryName,char *SrcText,
    char *DestText,int DestLength,char *HelpTopic=NULL,DWORD Flags=0);
/* SVS $ */
// ! дополнительный параметра у KeyToText - размер данных
//   Size=0 - по максимуму!
BOOL WINAPI KeyToText(int Key,char *KeyText,int Size=0);
/* SVS $ */
/* 01.08.2000 SVS $ */
/* $ 31.08.2000 tran
   FSF/FarInputRecordToKey */
int WINAPI InputRecordToKey(INPUT_RECORD *r);
/* tran 31.08.2000 $ */


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
                    struct CharTableSet *TableSet,
                    DWORD Flags);
/* SVS $ */

/* $ 14.08.2000 SVS
    + Функции семейства seek под __int64
*/
#ifdef __cplusplus
extern "C" {
#endif

__int64 WINAPI ftell64(FILE *fp);
int WINAPI fseek64 (FILE *fp, __int64 offset, int whence);

void *WINAPI FarBsearch(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));

/* $ 10.09.2000 tran
   FSF/FarRecurseSearch*/
typedef int  (WINAPI *FRSUSERFUNC)(WIN32_FIND_DATA *FData,char *FullName);
void WINAPI FarRecursiveSearch(char *initdir,char *mask,FRSUSERFUNC func,DWORD flags);
/* tran 10.09.2000 $ */
/* $ 14.09.2000 SVS
 + Функция FarMkTemp - получение имени временного файла с полным путем.
*/
/* $ 25.10.2000 IS
 ! Изменил имя параметра с Template на Prefix
*/
char* WINAPI FarMkTemp(char *Dest, char *Prefix);
/* IS $*/
/* SVS $*/


/* $ 24.09.2000 SVS
 + Функция KeyNameToKey - получение кода клавиши по имени
   Если имя не верно или нет такого - возвращается -1
*/
int WINAPI KeyNameToKey(char *Name);
/* SVS $*/

/*$ 27.09.2000 skv
*/
void WINAPI DeleteBuffer(char* Buffer);
/* skv$*/

#ifdef __cplusplus
};
#endif
/* SVS $ */

void SysLog(int l);
void SysLog(char *msg,...);

void OpenSysLog();
void CloseSysLog();

struct TUserLog
{
    FILE *Stream;
    int   Level;
};


FILE *OpenLogStream(char *file);

#define L_ERR      1
#define L_WARNING  2
#define L_INFO     3
#define L_DEBUG1   4
#define L_DEBUG2   5
#define L_DEBUG3   6

#define MAX_ARG_LEN   4096
#define MAX_LOG_LINE 10240

#define MAX_FILE 260


BOOL EjectVolume(char Letter,DWORD Flags);

BOOL WINAPI KeyMacroToText(int Key,char *KeyText0,int Size);

/* $ 30.12.2000 SVS
   Функции работы с атрибутами файлов "опубликованы"
*/
int GetEncryptFunctions(void);
int ESetFileAttributes(const char *Name,int Attr);
int ESetFileCompression(const char *Name,int State,int FileAttr);
int ESetFileEncryption(const char *Name,int State,int FileAttr);
int ESetFileTime(const char *Name,FILETIME *LastWriteTime,
                  FILETIME *CreationTime,FILETIME *LastAccessTime,
                  int FileAttr);
/* SVS $ */

#endif  // __FARFUNC_HPP__
