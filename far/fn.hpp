#ifndef __FARFUNC_HPP__
#define __FARFUNC_HPP__
/*
fn.hpp

Описания функций

*/

/* Revision: 1.163 02.07.2002 $ */

/*
Modify:
  02.07.2002 SVS
    + _PluginsStackItem_Dump() - дамп стека плагинов
  18.06.2002 SVS
    ! Функция IsFolderNotEmpty переименована в CheckFolder
  04.06.2002 SVS
    + TextToCharInfo
    ! Немного const
  31.05.2002 SVS
    ! GetVidChar стал inline в fn.hpp, код, ответственный за юникод
      перекочевал в GetVidCharW (из-за "for").
    ! SetVidChar стал inline
  30.05.2002 SVS
    + InitRecodeOutTable()
    ! GetVidChar и SetVidChar оперируют байтами
  30.05.2002 SVS
    + ShellUpdatePanels и CheckUpdateAnotherPanel
  28.05.2002 SVS
    + IsLocalPath()
  25.05.2002 IS
    ! первый параметр у ConvertDate теперь ссылка на константу
  24.05.2002 SVS
    + _INPUT_RECORD_Dump - вывод в лог информацию о INPUT_RECORD
    + _FCTLLOG
    + RunGraber()
  22.05.2002 SVS
    + _VCTL_ToName, _VCTLLOG
  22.05.2002 SVS
    + IsDiskInDrive()
  18.05.2002 SVS
    ! Возможность компиляции под BC 5.5
  16.05.2002 SVS
    ! Лишние описания функций
  29.04.2002 SVS
    ! немного const
  27.04.2002 SVS
    ! [G|S]etShortcutFolder могут теперь иметь в качестве некоторых параметров
      значение равное NULL
  25.04.2002 IS
    + const вариант PointToName
  05.04.2002 SVS
    + CheckShortcutFolder()
  04.04.2002 SVS
    ! WordWrap -> FarFormatText
    + _ACTL_ToName
  02.04.2002 SVS
    + SetInitialCursorType()
  28.03.2002 SVS
    + ClearScreen()
  26.03.2002 IS
    + void InitLCIDSort();
  26.03.2002 DJ
    ! вынесем CharBufferTooSmallWarn() на всеобщее обозрение
  20.03.2002 SVS
    + FarGetCurDir()
  20.03.2002 IS
    ! PrepareOSIfExist теперь принимает и возвращает const
    + PointToFolderNameIfFolder - аналог PointToName, только для строк типа
      "name\" (оканчивается на слеш) возвращает указатель на name, а не
      на пустую строку
  17.03.2002 IS
    + PrepareTable: параметр UseTableName - в качестве имени таблицы
      использовать не имя ключа реестра, а соответствующую переменную.
      По умолчанию - FALSE (использовать имя ключа).
  12.03.2002 VVM
    + Новая функция - пользователь попытался прервать операцию.
      Зададим вопрос.
  05.03.2002 DJ
    ! SubstFileName() получает размер буфера
  03.03.2002 SVS
    ! Если для VC вставить ключ /Gr, то видим кучу багов :-/
  03.03.2002 SVS
    ! Есть только одна функция создания временного файла - FarMkTempEx
      FarMkTemp - это для плагинов
    + ChangeBlockColor() - изменение цвета в блоке
  22.02.2002 SVS
    + Добавка функций ToPercent64() и filelen64()
    ! Коррекция fseek64 и ftell64 (в т.ч. снесен модификатор WINAPI)
  15.02.2002 IS
    + Новый параметр ChangeDir у FarChDir, если FALSE, то не меняем текущий
      диск, а только устанавливаем переменные окружения. По умолчанию - TRUE.
  13.02.2002 SVS
    + SysLogLastError()
  11.02.2002 SVS
    + _DLGMSG_ToName()
  05.02.2002 SVS
    ! У DeleteFileWithFolder параметр имеет суть const
    + _*_ToName() - для отладочных целей
  05.02.2002 SVS
    + IsNavKey(), IsShiftKey()
  25.01.2002 SVS
    + GetRegKeySize с уже открытым ключем HKEY hKey
  14.01.2002 IS
    + FarChDir - установка нужного диска и каталога и установление
      соответствующей переменной окружения. В случае успеха возвращается
      не ноль.
  11.01.2002 IS
    + InitKeysArray
  10.01.2002 SVS
    + SYSLOG_ECTL
  25.12.2001 SVS
    + AddEndSlash(char *Path,char TypeSlash) - с явно заданным слешем
  21.12.2001 SVS
    + CalcWordFromString - "вычисление" слова
  07.12.2001 SVS
    ! У Execute команда (первый параметр) - const
  07.12.2001 IS
    ! Два дополнительных параметра у GetString, которые используются
      при добавлении чек-бокса.
    + FarInputBox - обертка вокруг GetString для плагинов - с меньшей
      функциональностью. Сделано для того, чтобы не дублировать код GetString.
  06.12.2001 SVS
    ! PrepareDiskPath() - имеет доп.параметр - максимальный размер буфера
  02.12.2001 SVS
    ! PrepareDiskPath() имеет второй параметр по умолчанию TRUE
  27.11.2001 DJ
    + параметр Local у EditorConfig и ViewerConfig
  26.11.2001 SVS
    + PrepareDiskPath()
  22.11.2001 SVS
    + У Execute() добавлен параметр - SetUpDirs "Нужно устанавливать каталоги?"
      Это как раз про ту войну, когда Костя "отлучил" кусок кода про
      установку каталогов. Это понадобится гораздо позже.
  19.11.2001 SVS
    + ReplaceStrings - замена подстроки
  15.11.2001 OT
    - Исправление поведения cd c:\ на активном панельном плагине
  06.11.2001 SVS
    + EnumRegValue() - перечисление Values у ключа
    ! Немного const для функции работы с реестром
  18.10.2001 SVS
    ! У функций Message параметр Flags имеет суть "DWORD"
    + WordWrap()
  15.10.2001 SVS
    + Экспортируемые FarSysLog и FarSysLogDump только под SYSLOG_FARSYSLOG
    + _DIALOG & SYSLOG_DIALOG
  07.10.2001 SVS
    + InsertString()
  01.10.2001 IS
    + TruncStrFromEnd
  26.09.2001 SVS
    + DeleteEndSlash (с подачи IS)
  24.09.2001 SVS
    ! CleverSysLog - параметр у конструктора - "заголовок"
  20.09.2001 SVS
    ! Параметр у InputRecordToKey "const"
  18.09.2001 SVS
    + _ALGO & SYSLOG_ALGO - для "алгоритмов работы"
      Внимание! Лог в последствии будет большим!
  18.09.2001 SVS
    + класс CleverSysLog - что бы при выходе из функции делал SysLog(-1)
  12.09.2001 SVS
    + ConvertNameToReal()
  09.09.2001 SVS
    + GetMenuHotKey()
  07.08.2001 IS
    ! FarCharTable: второй параметр теперь не const, потому что он может
      меняться. в FarCharTable.
  03.08.2001 IS
    + InsertQuote
  31.07.2001 IS
    ! Внедрение const (FarGetMsgFn)
  25.07.2001 SVS
    ! Осмысленный параметр у InitConsole.
  11.07.2001 SVS
    ! HiStrlen и RemoveChar - дополнительный параметр - при дублях, типа '&&'
      "удалять ли все или оставлять только один символ"
  06.07.2001 IS
    ! Убрал CopyMaskStr, нефиг плодить сущности
  04.07.2001 SVS
    ! BoxText может рисовать вертикальный сепаратор
  04.07.2001 SVS
    + Функции про хип
  02.07.2001 IS
    + RawConvertShortNameToLongName
  25.06.2001 IS
    ! Внедрение const
  22.06.2001 SVS
    + StrFTime
  21.06.2001 SVS
    ! Удалена функция WriteSequenceInput() за ненадобностью
  18.06.2001 SVS
    + ExtractIfExistCommand()
  11.06.2001 SVS
    ! Новые параметры у GetSearchReplaceString()
  08.06.2001 SVS
    + GenerateWINDOW_BUFFER_SIZE_EVENT()
  06.06.2001 SVS
    ! функции получения символа юзаем пока только в режиме USE_WFUNC
  03.06.2001 SVS
    + GetRegKeySize() - получить размер данных
  30.05.2001 SVS
    ! ShellCopy::CreatePath выведена из класса в отдельню функцию CreatePath()
  21.05.2001 OT
    - Исправление поведения AltF9
  16.05.2001 SVS
   + _D(x) Для Мельникова!
    ! DumpExceptionInfo переименован в WriteEvent и переехал в farexcpt.hpp
    ! xfilter переехал в farexcpt.hpp
  09.05.2001 OT
   ! Макросы, аналогичные _D(x), которые зависят от разработчика или функционала
  07.05.2001 SVS
   ! _D(x) для SysLog
  07.05.2001 DJ
   + LocalUpperFast(), LocalLowerFast(), CopyMaskStr()
  06.05.2001 DJ
   ! перетрях #include
  29.04.2001 ОТ
   + Внедрение NWZ от Третьякова
  28.04.2001 SVS
   + xfilter
   + Новый параметр у DumpExceptionInfo - указатель на PluginItem.
   + Фунция печати дампа памяти SysLogDump()
  08.04.2001 SVS
   ! GetCommaWord() - дополнительный параметр - разделитель, по умолчанию = ','
   ! ExpandPATHEXT() выкинуты за ненадобностью.
  06.04.2001 SVS
   + ExpandPATHEXT()
  04.04.2001 SVS
   + MkRegKeyHighlightName
  03.04.2001 SVS
   + Add_PATHEXT()
  30.03.2001 SVS
   + FarGetLogicalDrives - оболочка вокруг GetLogicalDrives, с учетом
     скрытых логических дисков
  29.03.2001 IS
   ! void ViewerConfig() -> void ViewerConfig(struct ViewerOptions &ViOpt);
  24.03.2001 tran 1.69
   + FarQsortEx, qsortex
  20.03.2001 tran 1.67
   + FarRecursiveSearch - добавлен void *param
  20.03.2001 SVS
   + FileSizeToStr - функция преобразования размера файла в... удобочитаемый
     вид - выдрана из FileList::ShowList()
  16.03.2001 SVS
   + Функция DriveLocalToRemoteName() - Получить из имени диска RemoteName
   + GetNameAndPassword();
  13.03.2001 SVS
   ! GetPathRoot переехала в flink.hpp :-)
  07.03.2001 IS
   + DeleteEmptyKey
  06.03.2001 SVS
   ! InsertCommas возвращает знчение на Dest
  28.02.2001 SVS
   ! CenterStr возвращает указатель на Dest
  27.02.2001 SVS
   + BoxText(Char) - вывод одного символа
  22.02.2001 SVS
   + RemoveChar - удаляет символ из строки
   ! RemoveHighlights(Str) как макрос - вызывает RemoveChar(Str,'&')
  21.02.2001 IS
   + EditorConfig вызывается с ссылкой на EditorOptions
  20.02.2001 SVS
   ! ShowSeparator - дополнительный параметр - тип сепаратора
   + MakeSeparator - создание разделителя в памяти
  14.02.2001 SKV
   ! Параметр setpal для InitConsole, с default значением 1.
     Переинитить ли палитру.
  02.02.2001 IS
   + RemoveUnprintableCharacters
  28.01.2001 SVS
   ! DumpExeptionInfo -> DumpExceptionInfo ;-)
  27.01.2001 VVM
   + Дополнительный параметр у GetErrorString - резмер буфера
  25.01.2001 SVS
   ! WriteInput - имеет дополнительный параметр - флаги
   ! TranslateKeyToVK - имеет дополнительный параметр - указатель на эвенты.
  23.01.2001 SVS
   + DumpExeptionInfo()
  23.01.2001 SVS
   ! CalcKeyCode - новый параметр.
  22.01.2001 SVS
   ! ShellSetFileAttributes теперь возвращает результат в виде TRUE или FALSE
  20.01.2001 SVS
   + GetSearchReplaceString, WriteSequenceInput
   ! WriteInput теперь возвращает результат в виде FALASE/TRUE.
  14.01.2001 SVS
   + PrepareOSIfExist
  05.01.2001 SVS
   ! Функция GetSubstName переехала в flink.hpp
  04.01.2001 SVS
   + KeyNameMacroToKey() и TranslateKeyToVK()
  04.01.2001 SVS
   ! Описания MkLink, GetNumberOfLinks переехали в flink.hpp
  03.01.2001 SVS
   ! Дополнительный параметр у ConvertDate -
     "как взять формат даты - динамически или статически?"
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

#include "int64.hpp"
#include "farconst.hpp"
#include "global.hpp"

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
void InitRecodeOutTable(UINT cp);
/* SKV$*/
void CloseConsole();
void SetFarConsoleMode();
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
void Text(int X, int Y, int Color, int MsgId);
void Text(int MsgId);
void VText(const char *Str);
void HiText(const char *Str,int HiColor);
void ShowSeparator(int Length,int Type=1);
char* MakeSeparator(int Length,char *DestStr,int Type=1);
void SetScreen(int X1,int Y1,int X2,int Y2,int Ch,int Color);
void MakeShadow(int X1,int Y1,int X2,int Y2);
void ChangeBlockColor(int X1,int Y1,int X2,int Y2,int Color);
void SetColor(int Color);
void ClearScreen(int Color);
int  GetColor();
void GetText(int X1,int Y1,int X2,int Y2,void *Dest);
void PutText(int X1,int Y1,int X2,int Y2,const void *Src);
#if defined(USE_WFUNC)
void PutTextA(int X1,int Y1,int X2,int Y2,const void *Src);
#endif
void GetRealText(int X1,int Y1,int X2,int Y2,void *Dest);
void PutRealText(int X1,int Y1,int X2,int Y2,const void *Src);
void mprintf(char *fmt,...);
void mprintf(int MsgId,...);
void vmprintf(char *fmt,...);

#if defined(USE_WFUNC)
BYTE GetVidCharW(CHAR_INFO CI);
inline BYTE GetVidChar(CHAR_INFO CI)
{
  if(Opt.UseTTFFont)
    return GetVidCharW(CI);
  return CI.Char.AsciiChar;
}

inline void SetVidChar(CHAR_INFO& CI,BYTE Chr)
{
  extern WCHAR Oem2Unicode[];
  if(Opt.UseTTFFont)
    CI.Char.UnicodeChar = Oem2Unicode[Chr];
  else
    CI.Char.AsciiChar=Chr;
}

#else
#define GetVidChar(CI)     (CI).Char.AsciiChar
#define SetVidChar(CI,Chr) (CI).Char.AsciiChar=Chr
#endif


void ShowTime(int ShowAlways);
int GetDateFormat();
int GetDateSeparator();
int GetTimeSeparator();
char* GetShellAction(char *FileName);
void ScrollScreen(int Count);
int ScreenSaver(int EnableExit);
char* InsertCommas(unsigned long Number,char *Dest);
char* InsertCommas(int64 li,char *Dest);
void DeleteDirTree(char *Dir);
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
void BoxText(unsigned char Chr);
void BoxText(char *Str,int IsVert=0);
int FarColorToReal(int FarColor);
void ReopenConsole();
int CheckFolder(char *Name);
char *RemoveChar(char *Str,char Target,BOOL Dup=TRUE);
char *InsertString(char *Str,int Pos,const char *InsStr,int InsSize=0);
int ReplaceStrings(char *Str,const char *FindStr,const char *ReplStr,int Count=-1);
#define RemoveHighlights(Str) RemoveChar(Str,'&')
int IsCaseMixed(char *Str);
int IsCaseLower(char *Str);
int DeleteFileWithFolder(const char *FileName);
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
void EditFileTypes(int MenuPos);
void ProcessUserMenu(int EditMenu);
DWORD RawConvertShortNameToLongName(const char *src, char *dest, DWORD maxsize);
int ConvertNameToFull(const char *Src,char *Dest, int DestSize);
int WINAPI ConvertNameToReal(const char *Src,char *Dest, int DestSize);
void ConvertNameToShort(const char *Src,char *Dest);
void ChangeConsoleMode(int Mode);
void FlushInputBuffer();
void SystemSettings();
void PanelSettings();
void InterfaceSettings();
void SetConfirmations();
void SetDizConfig();
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
int GetColorDialog(unsigned int &Color);
int HiStrlen(const char *Str,BOOL Dup=TRUE);
/* $ 27.01.2001 VVM
   + Дополнительный параметр у GetErrorString - резмер буфера */
int GetErrorString(char *ErrStr, DWORD StrSize);
/* VVM $ */
void ShowProcessList();
int CopyFormatToClipboard(const char *Format,char *Data);
char* PasteFormatFromClipboard(const char *Format);
/* $ 16.10.2000 tran
  параметер - ограничение по длины */
char* WINAPI PasteFromClipboardEx(int max);
/* tran $ */

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
int __cdecl LocalStricmp(const char *s1,const char *s2);
int __cdecl LocalStrnicmp(const char *s1,const char *s2,int n);
int __cdecl LCStricmp(const char *s1,const char *s2);
int LocalKeyToKey(int Key);
int GetShortcutFolder(int Key,char *DestFolder,char *PluginModule=NULL,
                      char *PluginFile=NULL,char *PluginData=NULL);
int SaveFolderShortcut(int Key,char *SrcFolder,char *PluginModule=NULL,
                       char *PluginFile=NULL,char *PluginData=NULL);
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
const char *NullToEmpty(const char *Str);
char* CenterStr(char *Src,char *Dest,int Length);
const char *GetCommaWord(const char *Src,char *Word,char Separator=',');
void ScrollBar(int X1,int Y1,int Length,unsigned long Current,unsigned long Total);
int WINAPI GetFileOwner(const char *Computer,const char *Name,char *Owner);

void ConvertDate(const FILETIME &ft,char *DateText,char *TimeText,int TimeLength,
        int Brief=FALSE,int TextMonth=FALSE,int FullYear=FALSE,int DynInit=FALSE);
void ShellOptions(int LastCommand,MOUSE_EVENT_RECORD *MouseEvent);

// Registry
void SetRegRootKey(HKEY hRootKey);
void SetRegKey(const char *Key,const char *ValueName,const char * const ValueData);
void SetRegKey(const char *Key,const char *ValueName,DWORD ValueData);
void SetRegKey(const char *Key,const char *ValueName,const BYTE *ValueData,DWORD ValueSize);
int GetRegKey(const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize);
int GetRegKey(const char *Key,const char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(const char *Key,const char *ValueName,DWORD Default);
int GetRegKey(const char *Key,const char *ValueName,BYTE *ValueData,const BYTE *Default,DWORD DataSize);
HKEY CreateRegKey(const char *Key);
HKEY OpenRegKey(const char *Key);
int GetRegKeySize(const char *Key,const char *ValueName);
int GetRegKeySize(HKEY hKey,const char *ValueName);
int EnumRegValue(const char *Key,DWORD Index,char *DestName,DWORD DestSize,LPBYTE Data,DWORD DataSize);
void DeleteRegKey(const char *Key);
void DeleteRegValue(const char *Key,const char *Value);
void DeleteKeyRecord(const char *KeyMask,int Position);
void InsertKeyRecord(const char *KeyMask,int Position,int TotalKeys);
void DeleteKeyTree(const char *KeyName);
int CheckRegKey(const char *Key);
int CheckRegValue(const char *Key,const char *ValueName);
int DeleteEmptyKey(HKEY hRoot, const char *FullKeyName);
int EnumRegKey(const char *Key,DWORD Index,char *DestName,DWORD DestSize);
int CopyKeyTree(const char *Src,const char *Dest,const char *Skip);
void UseSameRegKey();
void CloseSameRegKey();


int CheckShortcutFolder(char *TestPath,int LengthPath,int IsHostFile);

#if defined(__FARCONST_HPP__) && (defined(_INC_WINDOWS) || defined(_WINDOWS_))
UDWORD NTTimeToDos(FILETIME *ft);
int Execute(const char *CmdStr,int AlwaysWaitFinish,int SeparateWindow=FALSE,
            int DirectRun=FALSE,int SetUpDirs=FALSE);
#endif

class Panel;
void ShellMakeDir(Panel *SrcPanel);
void ShellDelete(Panel *SrcPanel,int Wipe);
int  ShellSetFileAttributes(Panel *SrcPanel);
void PrintFiles(Panel *SrcPanel);
void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir=FALSE);
int  CheckUpdateAnotherPanel(Panel *SrcPanel,const char *SelName);

#ifdef __FAR_INT64_HPP__
BOOL GetDiskSize(char *Root,int64 *TotalSize,int64 *TotalFree,int64 *UserFree);
int GetDirInfo(char *Title,char *DirName,unsigned long &DirCount,
               unsigned long &FileCount,int64 &FileSize,
               int64 &CompressedFileSize,int64 &RealSize,
               unsigned long &ClusterSize,clock_t MsgWaitTime,
               int EnhBreak);
int GetPluginDirInfo(HANDLE hPlugin,char *DirName,unsigned long &DirCount,
               unsigned long &FileCount,int64 &FileSize,
               int64 &CompressedFileSize);
#endif

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
int WINAPI FarGetPluginDirList(int PluginNumber,HANDLE hPlugin,
                  const char *Dir,struct PluginPanelItem **pPanelItem,
                  int *pItemsNumber);
int WINAPI FarMenuFn(int PluginNumber,int X,int Y,int MaxHeight,
           DWORD Flags,const char *Title,const char *Bottom,
           const char *HelpTopic,const int *BreakKeys,int *BreakCode,
           const struct FarMenuItem *Item, int ItemsNumber);
int WINAPI FarDialogFn(int PluginNumber,int X1,int Y1,int X2,int Y2,
           const char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber);
const char* WINAPI FarGetMsgFn(int PluginNumber,int MsgId);
int WINAPI FarMessageFn(int PluginNumber,DWORD Flags,
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
int WINAPI FarAdvControl(int ModuleNumber, int Command, void *Param);
/* IS $ */
/* $ 23.07.2000 IS
   Функции для расширенного диалога
*/
//  Функция расширенного диалога
int WINAPI FarDialogEx(int PluginNumber,int X1,int Y1,int X2,int Y2,
      const char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber,
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
DWORD WINAPI ExpandEnvironmentStr(const char *src, char *dst, size_t size=8192);
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
const char* WINAPI PointToName(const char *Path);
/* $ 20.03.2002 IS
    + PointToFolderNameIfFolder - аналог PointToName, только для строк типа
      "name\" (оканчивается на слеш) возвращает указатель на name, а не
      на пустую строку
*/
char* WINAPI PointToFolderNameIfFolder(const char *Path);
/* IS $ */
BOOL  AddEndSlash(char *Path,char TypeSlash);
BOOL  WINAPI AddEndSlash(char *Path);
BOOL  WINAPI DeleteEndSlash(char *Path);
char *WINAPI FarItoa(int value, char *string, int radix);
__int64 WINAPI FarAtoi64(const char *s);
char *WINAPI FarItoa64(__int64 value, char *string, int radix);
int WINAPI FarAtoi(const char *s);
void WINAPI FarQsort(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
void WINAPI FarQsortEx(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *),void*);
int WINAPIV FarSprintf(char *buffer,const char *format,...);
int WINAPIV FarSscanf(const char *buffer, const char *format,...);
int WINAPI CopyToClipboard(const char *Data);
char* WINAPI PasteFromClipboard(void);
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

const char *_FARKEY_ToName(int Key);
const char *_VK_KEY_ToName(int VkKey);
const char *_ECTL_ToName(int Command);
const char *_FCTL_ToName(int Command);
const char *_DLGMSG_ToName(int Msg);
const char *_ACTL_ToName(int Command);
const char *_VCTL_ToName(int Command);
const char *_INPUT_RECORD_Dump(INPUT_RECORD *Rec);
void PluginsStackItem_Dump(char *Title,const struct PluginsStackItem *StackItems,int ItemNumber,FILE *fp=NULL);

#if defined(SYSLOG_FARSYSLOG)
#ifdef __cplusplus
extern "C" {
#endif
void WINAPIV _export FarSysLog(char *ModuleName,int Level,char *fmt,...);
void WINAPI  _export FarSysLogDump(char *ModuleName,DWORD StartAddress,LPBYTE Buf,int SizeBuf);
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
    CleverSysLog(char *Title=NULL);
    ~CleverSysLog();
};


#define MAX_ARG_LEN   4096
#define MAX_LOG_LINE 10240

#define MAX_FILE 260


BOOL EjectVolume(char Letter,DWORD Flags);

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
int ConvertWildcards(const char *Src,char *Dest, int SelectedFolderNameLength);

const char* WINAPI PrepareOSIfExist(const char *CmdLine);

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
int GetInputRecord(INPUT_RECORD *rec);
int PeekInputRecord(INPUT_RECORD *rec);
int CalcKeyCode(INPUT_RECORD *rec,int RealKey,int *NotMacros=NULL);
/* $ 24.08.2000 SVS
 + Пераметр у фунции WaitKey - возможность ожидать конкретную клавишу
*/
void WaitKey(int KeyWait=-1);
/* SVS $ */
int WriteInput(int Key,DWORD Flags=0);
int IsNavKey(DWORD Key);
int IsShiftKey(DWORD Key);
int CheckForEsc();

// Получить из имени диска RemoteName
char* DriveLocalToRemoteName(int DriveType,char Letter,char *Dest);
char* WINAPI FileSizeToStr(char *DestStr,DWORD SizeHigh, DWORD Size,
                                int Width=-1, int ViewFlags=COLUMN_COMMAS);

DWORD WINAPI FarGetLogicalDrives(void);

char *Add_PATHEXT(char *Dest);

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl qsortex(char *base, unsigned int nel, unsigned int width,
            int (__cdecl *comp_fp)(const void *, const void *,void*), void *user);

#if __BORLANDC__ < 0x550 || defined(_MSC_VER)
char * __cdecl mktemp(char *temp);
#endif

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

BOOL WINAPI GetMenuHotKey(char *HotKey,int LenHotKey,
                          char *DlgHotKeyTitle,
                          char *DlgHotKeyText,
                          char *HelpTopic,
                          char *RegKey,
                          char *RegValueName);

char *WINAPI FarFormatText(const char *SrcText,int Width,
                      char *DestText,int MaxLen,
                      const char* Break, DWORD Flags);

void SetPreRedrawFunc(PREREDRAWFUNC Func);

int PathMayBeAbsolute(const char *Src);
char* PrepareDiskPath(char *Path,int MaxSize,BOOL CheckFullPath=TRUE);

#if defined(MOUSEKEY)
const char * const CalcWordFromString(const char *Str,int CurPos,int *Start,int *End);
#endif

void CharBufferTooSmallWarn(int BufSize, int FileNameSize);

long filelen(FILE *FPtr);
__int64 filelen64(FILE *FPtr);
__int64 ftell64(FILE *fp);
int fseek64 (FILE *fp, __int64 offset, int whence);

BOOL IsDiskInDrive(const char *Drive);

BOOL IsLocalPath(const char *Path);

BOOL RunGraber(void);

#endif  // __FARFUNC_HPP__
