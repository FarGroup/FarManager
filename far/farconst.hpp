#ifndef __FARCONST_HPP__
#define __FARCONST_HPP__
/*
farconst.hpp

содержит все enum, #define, etc

*/

/* Revision: 1.35 21.05.2001 $ */

/*
Modify:
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

#define PluginsFolderName "Plugins"
#define HelpFileMask "*.hlf"

typedef unsigned char    UBYTE;
typedef unsigned short   UWORD;
typedef unsigned long    UDWORD;

#define  Min(x,y) (((x)<(y)) ? (x):(y))
#define  Max(x,y) (((x)>(y)) ? (x):(y))
#undef isspace
#define  isspace(x) ((x)==' ' || (x)=='\t')
#define  iseol(x) ((x)=='\r' || (x)=='\n')

#define  FALSE        0
#define  TRUE         1

#define  NM          260

#define  DEFAULT_SORT_GROUP 10000

// типы рамок
enum {NO_BOX,SINGLE_BOX,SHORT_SINGLE_BOX,DOUBLE_BOX,SHORT_DOUBLE_BOX};
enum {MSG_WARNING=1,MSG_ERRORTYPE=2,MSG_KEEPBACKGROUND=4,MSG_DOWN=8,
      MSG_LEFTALIGN=16};
enum {FILETYPE_EXEC,FILETYPE_VIEW,FILETYPE_EDIT};

enum {DIZ_NOT_UPDATE,DIZ_UPDATE_IF_DISPLAYED,DIZ_UPDATE_ALWAYS};

enum {
  MACRO_SHELL,MACRO_VIEWER,MACRO_EDITOR,MACRO_DIALOG,MACRO_SEARCH,
  MACRO_DISKS,MACRO_MAINMENU,MACRO_HELP,
  MACRO_INFOPANEL,MACRO_QVIEWPANEL,MACRO_TREEPANEL,
  /* $ 05.04.2001 VVM
    ! Должен быть всегда последним. Используется в циклах */
  MACRO_LAST,
  /* VVM $ */
  MACRO_OTHER=0x1000
};

#define MakeDialogItems(Data,Item) \
  struct DialogItem Item[sizeof(Data)/sizeof(Data[0])]; \
  Dialog::DataToItem(Data,Item,sizeof(Data)/sizeof(Data[0]));

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
enum {PLUGIN_FARGETFILE,PLUGIN_FARGETFILES,PLUGIN_FARPUTFILES,
      PLUGIN_FARDELETEFILES,PLUGIN_FARMAKEDIRECTORY,PLUGIN_FAROTHER};

enum {MODALTREE_ACTIVE=1,MODALTREE_PASSIVE=2,MODALTREE_FREE=3};

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

#define SYSID_PRINTMANAGER	0x6E614D50


/* $ 25.02.2001 VVM
  + Флаги для ReadDiz() */
enum ReadDizFlags {
  RDF_NO_UPDATE		= 0x00000001UL,
};
/* VVM $ */

#define STATUS_STRUCTWRONGFILLED       0xE0001000
#define STATUS_INVALIDFUNCTIONRESULT   0xE0002000

#if defined(__BORLANDC__)
#define TRY	try
#else
#define TRY	__try
#endif
/* SVS $ */

#define DRIVE_SUBSTITUTE    0x0F
#define DRIVE_NOT_INIT      0xFF


#define SKEY_VK_KEYS        0x40000000


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

/* 01.08.2000 tran
   ExitCode values */
#define XC_WORKING        0xffff
#define XC_QUIT           0

/* $ 11.08.2000 tran
   мелочь для лучшей читабельности */
#define ENABLE_SWITCH TRUE
#define DISABLE_SWITCH FALSE
///

#endif // __FARCONST_HPP__
