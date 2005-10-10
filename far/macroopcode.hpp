/*
macroopcode.hpp

OpCode для макросов

*/

/* Revision: 1.17 07.10.2005 $ */

/*
Modify:
  07.10.2005 SVS
    ! Editor.CurStr -> Editor.Value. так точнее будет
    + Dlg.GetValue()
  05.10.2005 SVS
    + Editor.CurStr - содержимое текущей строки
  19.09.2005 SVS
    + MCODE_V_PPANEL_DRIVETYPE, MCODE_V_APANEL_DRIVETYPE
  05.07.2005 SVS
    + Добавка в макросы - OldVal=Editor.Set(Index,NewVal)
    + Editor.FileName - имя редактируемого файла
    + Viewer.FileName - имя просматриваемого файла
  06.04.2005 SVS
    + MCODE_F_MSAVE       // b=msave(var)
    + MCODE_C_APANEL_LFN  // на активной панели длинные имена?
    + MCODE_C_PPANEL_LFN  // на пассивной панели длинные имена?
  01.04.2005 SVS
    + MCODE_F_PANELITEM
  02.03.2005 SVS
    + MCODE_V_FAR_WIDTH
  15.02.2005 SVS
    + MCODE_F_ITOA
  14.02.2005 SVS
    + MCODE_V_APANEL_OPIFLAGS, MCODE_V_PPANEL_OPIFLAGS, MCODE_V_CMDLINE_VALUE
  08.12.2004 SVS
    + Dlg.ItemCount, Dlg.CurPos, CmdLine.ItemCount, CmdLine.CurPos
  11.11.2004 SVS
    + [A|P]Panel.UNCPath
    ! [A|P]Panel.Count переименован в [A|P]Panel.ItemCount
    + В дополнении к [A|P]Panel.ItemCount 2 новых слова:
        "ItemCount" - число элементов в текущем объекте
        "CurPos" - текущий индекс в текущем объекте (начиная с 1)
    + Три обособленных, для редактора:
        "Editor.CurLine"  - текущая строка в редакторе (начиная с 1)
        "Editor.Lines"    - количество строк
        "Editor.CurPos"   - текущая позиция курсора в строке (начиная с 1)
  10.11.2004 SVS
    + [A|P]Panel.Count, [A|P]Panel.CurPos
  09.11.2004 SVS
    + MCODE_V_APANEL_TYPE, MCODE_V_PPANEL_TYPE, MCODE_C_APANEL_FILEPANEL, MCODE_C_PPANEL_FILEPANEL
  10.09.2004 SVS
    + UCase (MCODE_F_UCASE), LCase (MCODE_F_UCASE)
  05.08.2004 SVS
    + MCODE_V_VIEWERSTATE, MCODE_F_FSPLIT, MCODE_F_MSGBOX, MCODE_C_CMDLINE_EMPTY, MCODE_C_CMDLINE_SELECTED, MCODE_V_DLGITEMTYPE
  02.08.2004 SVS
    + MCODE_C_CMDLINE_BOF, MCODE_C_CMDLINE_EOF
  07.07.2004 SVS & AN
    + Адд
*/

#ifndef __MACROOPCODE_HPP__
#define __MACROOPCODE_HPP__

#include "keys.hpp"

enum MACRO_OP_CODE {
  /* ************************************************************************* */
  MCODE_OP_EXIT=KEY_MACRO_OP_BASE,  // принудительно закончить выполнение макропоследовательности

  MCODE_OP_JMP,                     // Jumps..
  MCODE_OP_JZ,
  MCODE_OP_JNZ,
  MCODE_OP_JLT,
  MCODE_OP_JLE,
  MCODE_OP_JGT,
  MCODE_OP_JGE,

  MCODE_OP_EXPR,                    // Признак начала (инициализация стека)
  MCODE_OP_DOIT,                    // Признак конца (вершина стека - Result)
  MCODE_OP_SAVE,                    // Присваивание переменной. Имя переменной следующие DWORD (как в $Text).
  MCODE_OP_SAVEREPCOUNT,
  MCODE_OP_PUSHINT,                 // Положить значение на стек. Само
  MCODE_OP_PUSHSTR,                 // значение - следующий DWORD
  MCODE_OP_PUSHVAR,                 // или несколько таковых (как в $Text)

  MCODE_OP_REP,                     // $rep - признак начала цикла
  MCODE_OP_END,                     // $end - признак конца цикла/условия

  MCODE_OP_IF,                      // Вообще-то эта группа в байткод
  MCODE_OP_ELSE,                    // не попадет никогда :)
  MCODE_OP_WHILE,

  // Одноместные операции
  MCODE_OP_NEGATE,
  MCODE_OP_NOT,

  // Двуместные операции
  MCODE_OP_LT,
  MCODE_OP_LE,
  MCODE_OP_GT,
  MCODE_OP_GE,
  MCODE_OP_EQ,
  MCODE_OP_NE,

  MCODE_OP_ADD,
  MCODE_OP_SUB,
  MCODE_OP_MUL,
  MCODE_OP_DIV,

  MCODE_OP_AND,
  MCODE_OP_OR,
  MCODE_OP_BITAND,
  MCODE_OP_BITOR,
  MCODE_OP_BITXOR,


  /* ************************************************************************* */
  MCODE_OP_XLAT,
  MCODE_OP_DATE,
  MCODE_OP_PLAINTEXT,

  MCODE_OP_MACROMODE,               // сменить режим блокировки вывода на экран
  MCODE_OP_SWITCHKBD,               // переключить раскладку клавиатуры
  MCODE_OP_ICLIP,                   // внутренний или внешний клипборд

#if defined(MOUSEKEY)
  MCODE_OP_SELWORD,
#endif


  /* ************************************************************************* */
  // функции
  MCODE_F_NOFUNC=KEY_MACRO_F_BASE,
  MCODE_F_ABS,
  MCODE_F_MIN,
  MCODE_F_MAX,
  MCODE_F_IIF,
  MCODE_F_SUBSTR,
  MCODE_F_INDEX,
  MCODE_F_RINDEX,
  MCODE_F_LEN,
  MCODE_F_STRING,
  MCODE_F_INT,
  MCODE_F_DATE,
  MCODE_F_XLAT,                     // вызывать XLat: Param=0 или 1
  MCODE_F_MENU_CHECKHOTKEY,         // есть такой сивол?
  MCODE_F_ENVIRON,                  // получить значение переменной среды
  MCODE_F_FEXIST,                   // проверка существования файла/каталога
  MCODE_F_FATTR,                    // возвращает атрибуты файловго объекта
  MCODE_F_FSPLIT,                   // возвращает заданную компоненту пути
  MCODE_F_MSGBOX,                   // MsgBox("Title","Text",flags)
  MCODE_F_UCASE,                    // UpperCase
  MCODE_F_LCASE,                    // LowerCase
  MCODE_F_ITOA,                     //
  MCODE_F_PANELITEM,                // V=PanelItem(Panel,Index,TypeInfo)
  MCODE_F_MSAVE,                    // b=msave(var)
  MCODE_F_EDITOR_SET,               // N=Editor.Set(N,Var)
  MCODE_F_DLG_GETVALUE,             // V=Dlg.GetValue(ID,N)

  /* ************************************************************************* */
  // булевые переменные - различные состояния
  MCODE_C_DISABLEOUTPUT=KEY_MACRO_C_BASE,// вывод запрещен?
  MCODE_C_WINDOWEDMODE,             // оконный режим?
  MCODE_C_BOF,                      // начало файла/активного каталога?
  MCODE_C_EOF,                      // конец файла/активного каталога?
  MCODE_C_EMPTY,                    // ком.строка пуста?
  MCODE_C_SELECTED,                 // выделенный блок есть?
  MCODE_C_ICLIP,                    // внутренний или внешний клипборд
  MCODE_C_ROOTFOLDER,               // аналог MCODE_C_APANEL_ROOT для активной панели

  MCODE_C_APANEL_BOF,               // начало активного  каталога?
  MCODE_C_PPANEL_BOF,               // начало пассивного каталога?
  MCODE_C_APANEL_EOF,               // конец активного  каталога?
  MCODE_C_PPANEL_EOF,               // конец пассивного каталога?
  MCODE_C_APANEL_ISEMPTY,           // активная панель:  пуста?
  MCODE_C_PPANEL_ISEMPTY,           // пассивная панель: пуста?
  MCODE_C_APANEL_SELECTED,          // активная панель:  выделенные элементы есть?
  MCODE_C_PPANEL_SELECTED,          // пассивная панель: выделенные элементы есть?
  MCODE_C_APANEL_ROOT,              // это корневой каталог активной панели?
  MCODE_C_PPANEL_ROOT,              // это корневой каталог пассивной панели?
  MCODE_C_APANEL_VISIBLE,           // активная панель:  видима?
  MCODE_C_PPANEL_VISIBLE,           // пассивная панель: видима?
  MCODE_C_APANEL_PLUGIN,            // активная панель:  плагиновая?
  MCODE_C_PPANEL_PLUGIN,            // пассивная панель: плагиновая?
  MCODE_C_APANEL_FILEPANEL,         // активная панель:  файловая?
  MCODE_C_PPANEL_FILEPANEL,         // пассивная панель: файловая?
  MCODE_C_APANEL_FOLDER,            // активная панель:  текущий элемент каталог?
  MCODE_C_PPANEL_FOLDER,            // пассивная панель: текущий элемент каталог?
  MCODE_C_APANEL_LEFT,              // активная панель левая?
  MCODE_C_PPANEL_LEFT,              // пассивная панель левая?
  MCODE_C_APANEL_LFN,               // на активной панели длинные имена?
  MCODE_C_PPANEL_LFN,               // на пассивной панели длинные имена?

  MCODE_C_CMDLINE_BOF,              // курсор в начале cmd-строки редактирования?
  MCODE_C_CMDLINE_EOF,              // курсор в конце cmd-строки редактирования?
  MCODE_C_CMDLINE_EMPTY,            // ком.строка пуста?
  MCODE_C_CMDLINE_SELECTED,         // в ком.строке есть выделение блока?

  /* ************************************************************************* */
  // не булевые переменные
  MCODE_V_APANEL_CURRENT=KEY_MACRO_V_BASE,// APanel.Current - имя файла на активной панели
  MCODE_V_PPANEL_CURRENT,           // PPanel.Current - имя файла на пассивной панели
  MCODE_V_APANEL_SELCOUNT,          // APanel.SelCount - активная панель:  число выделенных элементов
  MCODE_V_PPANEL_SELCOUNT,          // PPanel.SelCount - пассивная панель: число выделенных элементов
  MCODE_V_APANEL_PATH,              // APanel.Path - активная панель:  путь на панели
  MCODE_V_PPANEL_PATH,              // PPanel.Path - пассивная панель: путь на панели
  MCODE_V_APANEL_UNCPATH,           // APanel.UNCPath - активная панель:  UNC-путь на панели
  MCODE_V_PPANEL_UNCPATH,           // PPanel.UNCPath - пассивная панель: UNC-путь на панели
  MCODE_V_APANEL_WIDTH,             // APanel.Width - активная панель:  ширина панели
  MCODE_V_PPANEL_WIDTH,             // PPanel.Width - пассивная панель: ширина панели
  MCODE_V_APANEL_TYPE,              // тип активной панели
  MCODE_V_PPANEL_TYPE,              // тип пассивной панели
  MCODE_V_APANEL_ITEMCOUNT,         // APanel.ItemCount - активная панель:  число элементов
  MCODE_V_PPANEL_ITEMCOUNT,         // PPanel.ItemCount - пассивная панель: число элементов
  MCODE_V_APANEL_CURPOS,            // APanel.CurPos - активная панель:  текущий индекс
  MCODE_V_PPANEL_CURPOS,            // PPanel.CurPos - пассивная панель: текущий индекс
  MCODE_V_APANEL_OPIFLAGS,          // APanel.OPIFlags - активная панель: флаги открытого плагина
  MCODE_V_PPANEL_OPIFLAGS,          // PPanel.OPIFlags - пассивная панель: флаги открытого плагина
  MCODE_V_APANEL_DRIVETYPE,         // APanel.DriveType - активная панель: тип привода
  MCODE_V_PPANEL_DRIVETYPE,         // PPanel.DriveType - пассивная панель: тип привода
  MCODE_V_FAR_WIDTH,                // Far.Width - ширина консольного окна
  MCODE_V_ITEMCOUNT,                // ItemCount - число элементов в текущем объекте
  MCODE_V_CURPOS,                   // CurPos - текущий индекс в текущем объекте
  MCODE_V_EDITORFILENAME,           // Editor.FileName - имя редактируемого файла
  MCODE_V_EDITORLINES,              // Editor.Lines - количество строк в редакторе
  MCODE_V_EDITORCURLINE,            // Editor.CurLine - текущая линия в редакторе (в дополнении к Count)
  MCODE_V_EDITORCURPOS,             // Editor.CurPos - текущая поз. в редакторе
  MCODE_V_EDITORSTATE,              // Editor.State
  MCODE_V_EDITORVALUE,              // Editor.Value - содержимое текущей строки
  MCODE_V_DLGITEMTYPE,              // Dlg.ItemType
  MCODE_V_DLGITEMCOUNT,             // Dlg.ItemCount
  MCODE_V_DLGCURPOS,                // Dlg.CurPos
  MCODE_V_VIEWERFILENAME,           // Viewer.FileName - имя просматриваемого файла
  MCODE_V_VIEWERSTATE,              // Viewer.State

  MCODE_V_CMDLINE_ITEMCOUNT,        // CmdLine.ItemCount
  MCODE_V_CMDLINE_CURPOS,           // CmdLine.CurPos
  MCODE_V_CMDLINE_VALUE,            // CmdLine.Value
};

typedef enum MACRO_OP_CODE TFunction;


#endif // __MACROOPCODE_HPP__
