#ifndef __FARCONST_HPP__
#define __FARCONST_HPP__
/*
farconst.hpp

содержит все enum, #define, etc

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

// версия FAR Manager
/*
   HIWORD:  HIBYTE = 0    ??
            LOBYTE = 1	- # betta
   LOWORD:  HIBYTE = 1  - version Hi
            LOBYTE = 65	- version Lo
*/
#define FAR_VERSION 0x00010141UL

#define FILE_ATTRIBUTE_REPARSE_POINT 0x400


#define TreeFileName "Tree.Far"
#define LocalMenuFileName "FarMenu.Ini"
#define TreeCacheFolderName "Tree.Cache"
#define PluginsFolderName "Plugins"
#define LangFileMask "*.lng"
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

#define  MAXSCRY     120

#define  NM          260

#define  DEFAULT_SORT_GROUP 10000

enum {FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL};
enum {UNSORTED,BY_NAME,BY_EXT,BY_MTIME,BY_CTIME,BY_ATIME,BY_SIZE,BY_DIZ,
      BY_OWNER,BY_COMPRESSEDSIZE,BY_NUMLINKS};
enum {VIEW_0=0,VIEW_1,VIEW_2,VIEW_3,VIEW_4,VIEW_5,VIEW_6,VIEW_7,VIEW_8,VIEW_9};
// типы рамок
enum {NO_BOX,SINGLE_BOX,SHORT_SINGLE_BOX,DOUBLE_BOX,SHORT_DOUBLE_BOX};
enum {MSG_WARNING=1,MSG_ERRORTYPE=2,MSG_KEEPBACKGROUND=4,MSG_DOWN=8,
      MSG_LEFTALIGN=16};
enum {FILETYPE_EXEC,FILETYPE_VIEW,FILETYPE_EDIT};
enum {DRIVE_SHOW_TYPE=1,DRIVE_SHOW_NETNAME=2,DRIVE_SHOW_LABEL=4,
      DRIVE_SHOW_FILESYSTEM=8,DRIVE_SHOW_SIZE=16,DRIVE_SHOW_REMOVABLE=32,
      DRIVE_SHOW_PLUGINS=64,DRIVE_SHOW_CDROM=128};
enum {UPDATE_KEEP_SELECTION=1,UPDATE_SECONDARY=2};
enum {SEARCH_ALL=0,SEARCH_ROOT,SEARCH_FROM_CURRENT,SEARCH_CURRENT_ONLY,
      SEARCH_SELECTED};
enum {MODALTYPE_VIEWER,MODALTYPE_EDITOR};

enum {DIZ_NOT_UPDATE,DIZ_UPDATE_IF_DISPLAYED,DIZ_UPDATE_ALWAYS};

// struct EditorUndoData
enum {UNDO_NONE=0,UNDO_EDIT,UNDO_INSSTR,UNDO_DELSTR};


#define MakeDialogItems(Data,Item) \
  struct DialogItem Item[sizeof(Data)/sizeof(Data[0])]; \
  Dialog::DataToItem(Data,Item,sizeof(Data)/sizeof(Data[0]));

// for filelist
enum {ARCHIVE_NONE,ARCHIVE_RAR,ARCHIVE_ZIP,ARCHIVE_ARJ,ARCHIVE_LZH};

#define MAX_MSG 5000

// for class KeyMacro
enum {MACRO_SHELL,MACRO_VIEWER,MACRO_EDITOR,MACRO_DIALOG,MACRO_SEARCH,
      MACRO_DISKS,MACRO_MAINMENU,MACRO_HELP,MACRO_OTHER};

// for class Panel
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

// for class Panel
enum {MODALTREE_ACTIVE=1,MODALTREE_PASSIVE=2};
enum {NORMAL_PANEL,PLUGIN_PANEL};



// from plugins.hpp
enum {PLUGIN_FARGETFILE,PLUGIN_FARGETFILES,PLUGIN_FARPUTFILES,
      PLUGIN_FARDELETEFILES,PLUGIN_FARMAKEDIRECTORY,PLUGIN_FAROTHER};


      // for class ShellCopy
enum COPY_CODES {COPY_CANCEL,COPY_NEXT,COPY_FAILURE,COPY_SUCCESS,
                 COPY_SUCCESS_MOVE};

// for class VMenu
enum {MENU_SHOWAMPERSAND=1,MENU_WRAPMODE=2,MENU_DISABLEDRAWBACKGROUND=4};


// from delete.cpp
enum {DELETE_SUCCESS,DELETE_YES,DELETE_SKIP,DELETE_CANCEL};


// from edit.cpp
enum {EOL_NONE,EOL_CR,EOL_LF,EOL_CRLF};

// from filelist.cpp
enum SELECT_MODES {SELECT_INVERT,SELECT_INVERTALL,SELECT_ADD,SELECT_REMOVE,
     SELECT_ADDEXT,SELECT_REMOVEEXT,SELECT_ADDNAME,SELECT_REMOVENAME};


// from flmodes.cpp
enum {NAME_COLUMN=0,SIZE_COLUMN,PACKED_COLUMN,DATE_COLUMN,TIME_COLUMN,
      MDATE_COLUMN,CDATE_COLUMN,ADATE_COLUMN,ATTR_COLUMN,DIZ_COLUMN,
      OWNER_COLUMN,NUMLINK_COLUMN,
      CUSTOM_COLUMN0,CUSTOM_COLUMN1,CUSTOM_COLUMN2,CUSTOM_COLUMN3,
      CUSTOM_COLUMN4,CUSTOM_COLUMN5,CUSTOM_COLUMN6,CUSTOM_COLUMN7,
      CUSTOM_COLUMN8,CUSTOM_COLUMN9};

// from scrsaver.cpp
enum {STAR_NONE,STAR_NORMAL,STAR_PLANET};

#endif // __FARCONST_HPP__
