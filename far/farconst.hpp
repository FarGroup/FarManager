#ifndef __FARCONST_HPP__
#define __FARCONST_HPP__
/*
farconst.hpp

содержит все enum, #define, etc

*/

/* Revision: 1.84 05.08.2004 $ */

/*
Modify:
  05.08.2004 SVS
    + MFLAGS_RUNAFTERFARSTARTED  - этот макрос уже запускался при старте ФАРа
  02.08.2004 SVS
    ! пропишем числовые значение енума...
  08.07.2004 SVS
    + MACRO_COMMON
  21.06.2004 SVS
    + DRIVE_DVD_RAM
    + CDROM_DeviceCaps
  09.06.2004 SVS
    + DRIVE_CD_RWDVD
  08.06.2004 SVS
    + DRIVE_*
  09.01.2004 SVS
    + ExcludeCmdHistoryType
  18.12.2003 SVS
    ! перечисления FSizeType и FDateType переехали из struct.hpp в farconst.hpp
  09.10.2003 SVS
    + APIS2ANSI и APIS2OEM для SetFileApisTo()
  04.10.2003 SVS
    ! Изменения во флагам макрокоманд. В принципе все описано в комметариях
  12.09.2003 SVS
    + CHKFLD_NOTFOUND - "нет такого каталога"
  03.09.2003 SVS
    + QUOTEDNAMETYPE
  02.09.2003 SVS
    + CHKFLD_ERROR
  25.08.2003 SVS
    ! Не SendKeysToPlugins, но NoSendKeysToPlugins, иначе нифига не получается.
  30.07.2003 SVS
    ! Вместо MFLAGS_INSIDEPLUGIN и MFLAGS_NOINSIDEPLUGIN введен один флаг MFLAGS_SENDKEYSTOPLUGINS,
      в соотвествии с NT в макросах
    ! немного документируем
  25.07.2003 SVS
    + MacroRecordAndExecuteType - коды возврата для KeyMacro::GetCurRecord()
  15.07.2003 SVS
    + MFLAGS_INSIDEPLUGIN и MFLAGS_NOINSIDEPLUGIN
  15.06.2003 SVS
    + До кучи добавлены макросы AsciiToUnicode и OEMToUnicode
  03.01.2003 SVS
    + CHAR_WCHAR
  10.12.2002 SVS
    + MSG_KILLSAVESCREEN
  14.07.2002 IS & SVS
    ! IS> PluginsFolderName, HelpFileMask и HelpFormatLinkModule теперь не
      IS> "#define", а "const char *" - для уменьшения размера данных
      SVS> ...и переехали в global.?pp
  18.06.2002 SVS
    + Коды возврата CHECKFOLDERCONST для CheckFolder()
  18.05.2002 SVS
    ! Возможность компиляции под BC 5.5
  15.05.2002 SVS
    + SKEY_IDLE - вместо VK_ послать FOCUS
  04.04.2002 SVS
    + FFTMODE
  22.02.2002 SVS
    ! Введение FAR_INT64
  10.01.2002 SVS
    + XC_EXISTS - код возврата для редактора
  25.12.2001 SVS
    + HelpFormatLinkModule
  24.12.2001 SVS
    - Bug - про хелп. Забыл исправить HelpFormatLink
  01.11.2001 SVS
    ! MakeDialogItems перехала в dialog.hpp из farconst.hpp
  26.10.2001 VVM
    + MOUSE_ANY_BUTTON_PRESSED
  16.10.2001 SVS
    + ADDSPACEFORPSTRFORMESSAGE - для Message (вместо числа 16)
  21.10.2001 SVS
    ! PREREDRAWFUNC и PISDEBUGGERPRESENT переехали из global.hpp
  16.10.2001 SVS
    + Макросы-преобразовалки: UnicodeToAscii() и UnicodeToOEM()
  16.09.2001 SVS
    ! Отключаемые исключения
  07.09.2001 SVS
    ! Флаги для макросов MFLAGS_* переехали из macro.cpp
  02.08.2001 IS
    + Для ассоциаций файлов:
      FILETYPE_ALTEXEC, FILETYPE_ALTVIEW, FILETYPE_ALTEDIT
  01.08.2001 SVS
    + HelpBeginLink, HelpFormatLink - формат для создания линков на темы помощи.
  31.07.2001 SVS
    + MACRO_MENU
  24.07.2001 SVS
    - проблемы компиляции под VC
  24.07.2001 IS
    ! isspace и iseol теперь не макросы, а inline функции
  22.07.2001 SVS
    + Оконстантим SysID для Network Browse плагина - SYSID_NETWORK
  26.06.2001 SVS
    ! __except -> EXCEPT
  25.06.2001 SVS
    + SEARCHSTRINGBUFSIZE - размер буфера для строки поиска.
  20.06.2001 SVS
    ! SKEY_NOTMACROS переехал из plugin.hpp
  06.06.2001 SVS
    + BOOKMARK_COUNT
    ! Min/Max заменены на inline-функции
  04.06.2001 SVS
    + HISTORY_COUNT - размер истории
  31.05.2001 OT
    ! Константы для FileEdit
  23.05.2001 SVS
    ! Перетасовка для MACRO_OTHER
  21.05.2001 SVS
    ! Константы MENU_ - в морг
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 SVS
    ! кривой комментарий был
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  28.04.2001 SVS
    + EXCEPT_OPENPLUGIN_FINDLIST, EXCEPT_FARDIALOG
  26.04.2001 DJ
    + новая константа для UserMenu EC_COMMAND_SELECTED
  25.04.2001 SVS
    + MODALTREE_FREE
  13.04.2001 VVM
    + Обработка колесика мышки под 2000.
  04.04.2001 SVS
    + MACRO_INFOPANEL,MACRO_QVIEWPANEL,MACRO_TREEPANEL,MACRO_LAST
      MACRO_LAST должен всегда стоять последним. Испрользуется в циклах.
  02.04.2001 SVS
    ! DRIVE_SUSTITUTE -> DRIVE_SUBSTITUTE
  02.04.2001 VVM
    + DRIVE_NOT_INIT. Используется при показе меню дисков.
  16.03.2001 SVS
    + FlagsNameAndPassword
  26.02.2001 VVM
    + STATUS_INVALIDFUNCTIONRESULT - когда функция вернула недопустимое значение
  25.02.2001 VVM
    + Флаги для ReadDiz() - RDF_*
  11.02.2001 SVS
    + *_MASK_SIZE - размеры для масок
  28.01.2001 SVS
    + SKEY_VK_KEYS - переехал из plugin.hpp
  25.01.2001 SVS
    + STATUS_STRUCTWRONGFILLED - когда Count>0 и **Ptr=NULL
  22.01.2001 SVS
    + EXCEPT_PROCESSVIEWERINPUT - на будущее.
  05.01.2001 SVS
    + DRIVE_SUSTITUTE - это про SUBST-диски.
  25.12.2000 SVS
    ! MACRO_ переехали из plugin.hpp :-)
  23.12.2000 SVS
    ! MACRO_ переехали в plugin.hpp
  21.12.2000 SVS
    ! Изменения в константах MACRO_*
  07.12.2000 SVS
    ! farversion.hpp исключен за ненадобностью
  31.10.2000 SVS
    + EXCEPT_GETOPENPLUGININFO_DATA
  23.10.2000 SVS
    + EXCEPT_GETPLUGININFO_DATA
  20.10.2000 SVS
    ! FILE_ATTRIBUTE_REPARSE_POINT перенесен в headers.hpp
  17.10.2000 SVS
    + ИСКЛЮЧЕНИЯ! Константы EXCEPT_* и макрос TRY
  27.09.2000 SVS
    +CASR_* Поведение Ctrl-Alt-Shift для AllCtrlAltShiftRule
  15.07.2000 SVS
    + Константы для Меню от VVM - вынесены из usermenu.cpp
  12.07.2000 SVS
    + Константы для WrapMode во вьювере.
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  10.07.2000 tran
    ! увеличено MAXSCRY с 120 до 300
      иначе под вин200 фар падает на Viewer
      там эта константа используется в качестве размера массива
      видимых строк.
      увеличивать выше не имеет смысла...
  29.06.2000 tran
    ! Версия берется из авто-генерируемого farversion.hpp
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define HelpBeginLink '<'
#define HelpEndLink '>'
#define HelpFormatLink "<%s\\>%s"

typedef unsigned char    UBYTE;
typedef unsigned short   UWORD;
typedef unsigned long    UDWORD;

typedef union {
  __int64 i64;
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } Part;
} FAR_INT64;


//#define  Min(x,y) (((x)<(y)) ? (x):(y))
template <class T>
inline const T&Min(const T &a, const T &b) { return a<b?a:b; }

//#define  Max(x,y) (((x)>(y)) ? (x):(y))
template <class T>
inline const T&Max(const T &a, const T &b) { return a>b?a:b; }

inline int IsSpace(int x) { return x==' ' || x=='\t'; }
inline int IsEol(int x)  { return x=='\r' || x=='\n'; }

#define  FALSE        0
#define  TRUE         1

#define  NM          260

#define  DEFAULT_SORT_GROUP 10000
#define  SEARCHSTRINGBUFSIZE 512

// типы рамок
enum {NO_BOX,SINGLE_BOX,SHORT_SINGLE_BOX,DOUBLE_BOX,SHORT_DOUBLE_BOX};

enum {
  MSG_WARNING        =0x00000001,
  MSG_ERRORTYPE      =0x00000002,
  MSG_KEEPBACKGROUND =0x00000004,
  MSG_DOWN           =0x00000008,
  MSG_LEFTALIGN      =0x00000010,
  MSG_KILLSAVESCREEN =0x00000020,
};

/* $ 02.08.2001 IS
     Новые константы для alt-f3, alt-f4 и ctrl-pgdn
*/
// Работа с ассоциациями файлов
enum {
  FILETYPE_EXEC,       // Enter
  FILETYPE_VIEW,       // F3
  FILETYPE_EDIT,       // F4
  FILETYPE_ALTEXEC,    // Ctrl-PgDn
  FILETYPE_ALTVIEW,    // Alt-F3
  FILETYPE_ALTEDIT     // Alt-F4
};
/* IS $ */

enum DIZUPDATETYPE {
  DIZ_NOT_UPDATE,
  DIZ_UPDATE_IF_DISPLAYED,
  DIZ_UPDATE_ALWAYS
};

// *** Macros ***
// области действия макросов (начало исполнения) -  НЕ БОЛЕЕ 0xFF областей!
enum MACROMODEAREA {
  MACRO_OTHER        =   0, // Режим копирования текста с экрана, вертикальные меню
  MACRO_SHELL        =   1, // Файловые панели
  MACRO_VIEWER       =   2, // Внутренняя программа просмотра
  MACRO_EDITOR       =   3, // Редактор
  MACRO_DIALOG       =   4, // Диалоги
  MACRO_SEARCH       =   5, // Быстрый поиск в панелях
  MACRO_DISKS        =   6, // Меню выбора дисков
  MACRO_MAINMENU     =   7, // Основное меню
  MACRO_MENU         =   8, // Прочие меню
  MACRO_HELP         =   9, // Система помощи
  MACRO_INFOPANEL    =  10, // Информационная панель
  MACRO_QVIEWPANEL   =  11, // Панель быстрого просмотра
  MACRO_TREEPANEL    =  12, // Панель дерева папок

  MACRO_COMMON,             // ВЕЗДЕ! - должен быть предпоследним, т.к. приоритет самый низший !!!
  MACRO_LAST                // Должен быть всегда последним! Используется в циклах
};

// Флаги для макросов
#define MFLAGS_MODEMASK            0x000000FF // маска для выделения области действия (области начала исполнения) макроса

#define MFLAGS_DISABLEOUTPUT       0x00000100 // подавить обновление экрана во время выполнения макроса
#define MFLAGS_NOSENDKEYSTOPLUGINS 0x00000200 // НЕ передавать клавиши во время записи/воспроизведения макроса
#define MFLAGS_RUNAFTERFARSTARTED  0x00000400 // этот макрос уже запускался при старте ФАРа
#define MFLAGS_RUNAFTERFARSTART    0x00000800 // этот макрос запускается при старте ФАРа

#define MFLAGS_EMPTYCOMMANDLINE    0x00001000 // запускать, если командная линия пуста
#define MFLAGS_NOTEMPTYCOMMANDLINE 0x00002000 // запускать, если командная линия не пуста

#define MFLAGS_SELECTION           0x00004000 // активная:  запускать, если есть выделение
#define MFLAGS_NOSELECTION         0x00008000 // активная:  запускать, если есть нет выделения
#define MFLAGS_PSELECTION          0x00010000 // пассивная: запускать, если есть выделение
#define MFLAGS_PNOSELECTION        0x00020000 // пассивная: запускать, если есть нет выделения
#define MFLAGS_EDITSELECTION       0x00040000 // запускать, если есть выделение в редакторе
#define MFLAGS_EDITNOSELECTION     0x00080000 // запускать, если есть нет выделения в редакторе
#define MFLAGS_NOFILEPANELS        0x00100000 // активная:  запускать, если это плагиновая панель
#define MFLAGS_NOPLUGINPANELS      0x00200000 // активная:  запускать, если это файловая панель
#define MFLAGS_PNOFILEPANELS       0x00400000 // пассивная: запускать, если это плагиновая панель
#define MFLAGS_PNOPLUGINPANELS     0x00800000 // пассивная: запускать, если это файловая панель
#define MFLAGS_NOFOLDERS           0x01000000 // активная:  запускать, если текущий объект "файл"
#define MFLAGS_PNOFOLDERS          0x02000000 // пассивная: запускать, если текущий объект "файл"
#define MFLAGS_PNOFILES            0x04000000 // пассивная: запускать, если текущий объект "папка"
#define MFLAGS_NOFILES             0x08000000 // активная:  запускать, если текущий объект "папка"

#define MFLAGS_LABELPRESENT        0x10000000 // есть "метка" для F? клавиш
#define MFLAGS_REUSEMACRO          0x20000000 // повторное использование макросов (вызов макроса из макроса)
#define MFLAGS_NEEDSAVEMACRO       0x40000000 // необходимо этот макрос запомнить
#define MFLAGS_DISABLEMACRO        0x80000000 // этот макрос отключен



// коды возврата для KeyMacro::GetCurRecord()
enum MACRORECORDANDEXECUTETYPE{
  MACROMODE_NOMACRO          =0,  // не в режиме макро
  MACROMODE_EXECUTING        =1,  // исполнение: без передачи плагину пимп
  MACROMODE_EXECUTING_COMMON =2,  // исполнение: с передачей плагину пимп
  MACROMODE_RECORDING        =3,  // запись: без передачи плагину пимп
  MACROMODE_RECORDING_COMMON =4,  // запись: с передачей плагину пимп
};

// **************************************************

// for filelist
enum {ARCHIVE_NONE,ARCHIVE_RAR,ARCHIVE_ZIP,ARCHIVE_ARJ,ARCHIVE_LZH};

#define MAX_MSG 5000

enum {
  COLUMN_MARK        =  0x80000000,
  COLUMN_NAMEONLY    =  0x40000000,
  COLUMN_RIGHTALIGN  =  0x20000000,
  COLUMN_FORMATTED   =  0x10000000,
  COLUMN_COMMAS      =  0x08000000,
  COLUMN_THOUSAND    =  0x04000000,
  COLUMN_BRIEF       =  0x02000000,
  COLUMN_MONTH       =  0x01000000,
};

// from plugins.hpp
enum {
  PLUGIN_FARGETFILE,
  PLUGIN_FARGETFILES,
  PLUGIN_FARPUTFILES,
  PLUGIN_FARDELETEFILES,
  PLUGIN_FARMAKEDIRECTORY,
  PLUGIN_FAROTHER
};

enum {
  MODALTREE_ACTIVE  =1,
  MODALTREE_PASSIVE =2,
  MODALTREE_FREE    =3
};

/* $ 27.09.2000 SVS
  +CASR_* Поведение Ctrl-Alt-Shift для AllCtrlAltShiftRule
*/
enum {
  CASR_PANEL  = 0x0001,
  CASR_EDITOR = 0x0002,
  CASR_VIEWER = 0x0004,
  CASR_HELP   = 0x0008,
  CASR_DIALOG = 0x0010,
};
/* SVS $ */

#define SYSID_PRINTMANAGER      0x6E614D50
#define SYSID_NETWORK           0x5774654E


/* $ 25.02.2001 VVM
  + Флаги для ReadDiz() */
enum ReadDizFlags {
  RDF_NO_UPDATE         = 0x00000001UL,
};
/* VVM $ */

#define STATUS_STRUCTWRONGFILLED       0xE0001000
#define STATUS_INVALIDFUNCTIONRESULT   0xE0002000

#if defined(__BORLANDC__)
#define TRY      try
#else
#define TRY   __try
#endif
#define EXCEPT __except

#define DRIVE_SUBSTITUTE            15
#define DRIVE_REMOTE_NOT_CONNECTED  16
#define DRIVE_CD_RW                 18
#define DRIVE_CD_RWDVD              19
#define DRIVE_DVD_ROM               20
#define DRIVE_DVD_RW                21
#define DRIVE_DVD_RAM               22
#define DRIVE_NOT_INIT             255

enum CDROM_DeviceCaps
{
  CDDEV_CAPS_NONE               = 0x00000000,

  CDDEV_CAPS_READ_CDROM         = 0x00000001,
  CDDEV_CAPS_READ_CDR           = 0x00000002,
  CDDEV_CAPS_READ_CDRW          = 0x00000004,

  CDDEV_CAPS_READ_DVDROM        = 0x00000008,
  CDDEV_CAPS_READ_DVDR          = 0x00000010,
  CDDEV_CAPS_READ_DVDRW         = 0x00000020,
  CDDEV_CAPS_READ_DVDRAM        = 0x00000040,


  CDDEV_CAPS_WRITE_CDR          = 0x00020000,
  CDDEV_CAPS_WRITE_CDRW         = 0x00040000,

  CDDEV_CAPS_WRITE_DVDR         = 0x00100000,
  CDDEV_CAPS_WRITE_DVDRW        = 0x00200000,
  CDDEV_CAPS_WRITE_DVDRAM       = 0x00400000,


  CDDEV_CAPS_GENERIC_CD         = CDDEV_CAPS_READ_CDROM | CDDEV_CAPS_READ_CDR | CDDEV_CAPS_READ_CDRW,
  CDDEV_CAPS_GENERIC_CDRW       = CDDEV_CAPS_GENERIC_CD | CDDEV_CAPS_WRITE_CDR | CDDEV_CAPS_WRITE_CDRW,
  CDDEV_CAPS_GENERIC_DVD        = CDDEV_CAPS_GENERIC_CD | CDDEV_CAPS_READ_DVDROM | CDDEV_CAPS_READ_DVDR | CDDEV_CAPS_READ_DVDRW,
  CDDEV_CAPS_GENERIC_COMBO      = CDDEV_CAPS_GENERIC_DVD | CDDEV_CAPS_WRITE_CDR | CDDEV_CAPS_WRITE_CDRW,
  CDDEV_CAPS_GENERIC_DVDRW      = CDDEV_CAPS_GENERIC_COMBO | CDDEV_CAPS_WRITE_DVDR | CDDEV_CAPS_WRITE_DVDRW,
  CDDEV_CAPS_GENERIC_DVDRAM     = CDDEV_CAPS_GENERIC_DVDRW | CDDEV_CAPS_WRITE_DVDRAM
};


enum {
  SKEY_VK_KEYS           = 0x40000000,
  SKEY_IDLE              = 0x80000000,
  SKEY_NOTMACROS         = 0x00000001,
};

// размеры для полей ввода
#define PANELFILTER_MASK_SIZE    2048

// для диалога GetNameAndPassword()
enum FlagsNameAndPassword{
  GNP_USELAST      = 0x00000001UL, // использовать последние введенные данные
  GNP_NOOEMTOCHAR  = 0x00000002UL, // не конвертировать имя и пароль OEM->CHAR
};

/* $ 13.04.2001 VVM
  + 2000 посылает консольному окну событие с dwEventFlags == MOUSE_WHEELED */
#define MOUSE_WHEELED 0x0004
/* VVM $ */

///

/* $ 27.05.2001 DJ         30.05.2001 OT
   константы для ExitCode - перенесены из
*/

enum {
    XC_QUIT                = (unsigned long) -777,
    XC_OPEN_ERROR          = 0,
    XC_MODIFIED            = 1,
    XC_NOT_MODIFIED        = 2,
    XC_LOADING_INTERRUPTED = 3,
    XC_EXISTS              = 4,
};

/* DJ $ */


/* $ 11.08.2000 tran
   мелочь для лучшей читабельности */
#define ENABLE_SWITCH TRUE
#define DISABLE_SWITCH FALSE
///

// Размер истории - как для класса History, так и для диалогов!
#define HISTORY_COUNT    64
// Количество закладок в редакторе/вьювере на одну позицию
#define BOOKMARK_COUNT   10

#define UnicodeToAscii(src,dst,lendst)  WideCharToMultiByte(CP_ACP,0,(src),-1,(dst),(lendst),NULL,FALSE)
#define UnicodeToOEM(src,dst,lendst)    WideCharToMultiByte(CP_OEMCP,0,(src),-1,(dst),(lendst),NULL,FALSE)
#define AsciiToUnicode(src,dst,lendst)  MultiByteToWideChar(CP_ACP,0,(src),-1,(dst),(lendst))
#define OEMToUnicode(src,dst,lendst)    MultiByteToWideChar(CP_OEMCP,0,(src),-1,(dst),(lendst))

typedef void (*PREREDRAWFUNC)(void);
typedef BOOL (WINAPI *PISDEBUGGERPRESENT)(VOID);

#define ADDSPACEFORPSTRFORMESSAGE 16

#define MOUSE_ANY_BUTTON_PRESSED (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED|FROM_LEFT_2ND_BUTTON_PRESSED|FROM_LEFT_3RD_BUTTON_PRESSED|FROM_LEFT_4TH_BUTTON_PRESSED)

enum FFTMODE{
  FFTM_BREAKLONGWORD = 0x00000001,
};

enum CHECKFOLDERCONST{ // for CheckFolder()
  CHKFLD_ERROR     = -2,
  CHKFLD_NOTACCESS = -1,
  CHKFLD_EMPTY     =  0,
  CHKFLD_NOTEMPTY  =  1,
  CHKFLD_NOTFOUND  =  2,
};

typedef union {
  WCHAR UnicodeChar;
  CHAR  AsciiChar;
} CHAR_WCHAR;

// для Opt.QuotedName
enum QUOTEDNAMETYPE{
  QUOTEDNAME_INSERT         = 0x00000001,            // кавычить при сбросе в командную строку, в диалогах и редакторе
  QUOTEDNAME_CLIPBOARD      = 0x00000002,            // кавычить при помещении в буфер обмена
};

enum{
  APIS2OEM,
  APIS2ANSI,
};

// for FileFilter
enum FDateType
{
  FDATE_MODIFIED=0,
  FDATE_CREATED,
  FDATE_OPENED,
};

enum FSizeType
{
  FSIZE_INBYTES=0,
  FSIZE_INKBYTES,
  FSIZE_INMBYTES,
  FSIZE_INGBYTES,

  FSIZE_IN_LAST, // всегда последний !!!
};

enum ExcludeCmdHistoryType{
  EXCLUDECMDHISTORY_NOTWINASS    = 0x00000001,  // не помещать в историю команды ассоциаций Windows
  EXCLUDECMDHISTORY_NOTFARASS    = 0x00000002,  // не помещать в историю команды выполнения ассоциаций файлов
  EXCLUDECMDHISTORY_NOTPANEL     = 0x00000004,  // не помещать в историю команды выполнения с панели
  EXCLUDECMDHISTORY_NOTCMDLINE   = 0x00000008,  // не помещать в историю команды выполнения с ком.строки
};

#endif // __FARCONST_HPP__
