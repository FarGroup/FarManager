/*
macroopcode.hpp

OpCode для макросов

*/

#ifndef __MACROOPCODE_HPP__
#define __MACROOPCODE_HPP__

#include "keys.hpp"

/*
  ВНИМАНИЕ!
  При добавлении сюда...
  ... так же то же необходимо добавлять и в syslog.cpp (функция __MCODE_ToName)

*/

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

  // <TODO>
  MCODE_OP_DISCARD,                 // убрать значение с вершины стека
  MCODE_OP_POP,                     // присвоить значение переменной и убрать из вершины стека
  MCODE_OP_COPY,                    // %a=%d
  // </TODO>

  MCODE_OP_IF,                      // Вообще-то эта группа в байткод
  MCODE_OP_ELSE,                    // не попадет никогда :)
  MCODE_OP_WHILE,

  /* ************************************************************************* */
  MCODE_OP_XLAT,
  MCODE_OP_DATE,
  MCODE_OP_PLAINTEXT,

  MCODE_OP_MACROMODE,               // сменить режим блокировки вывода на экран
  MCODE_OP_SWITCHKBD,               // переключить раскладку клавиатуры
  MCODE_OP_ICLIP,                   // внутренний или внешний клипборд
  MCODE_OP_AKEY,                    // $AKey - клавиша, которой вызвали макрос
  MCODE_OP_SELWORD,                 // $SelWord - выделить "слово"


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
  MCODE_F_PANEL_SETPOS,             // N=Panel.SetPos(panelType,fileName)
  MCODE_F_MSAVE,                    // b=msave(var)
  MCODE_F_EDITOR_SET,               // N=Editor.Set(N,Var)
  MCODE_F_DLG_GETVALUE,             // V=Dlg.GetValue(ID,N)
  MCODE_F_CLIP,                     // V=clip(N,S)
  MCODE_F_SLEEP,                    // Sleep(N)
  MCODE_F_PANEL_FEXIST,             // N=Panel.FExist(panelType,fileMask)
  MCODE_F_PANEL_FATTR,              // N=Panel.FAttr(panelType,fileMask)

  /* ************************************************************************* */
  // булевые переменные - различные состояния
  MCODE_C_DISABLEOUTPUT=KEY_MACRO_C_BASE,// вывод запрещен?

  MCODE_C_AREA_OTHER,               // Режим копирования текста с экрана, вертикальные меню
  MCODE_C_AREA_SHELL,               // Файловые панели
  MCODE_C_AREA_VIEWER,              // Внутренняя программа просмотра
  MCODE_C_AREA_EDITOR,              // Редактор
  MCODE_C_AREA_DIALOG,              // Диалоги
  MCODE_C_AREA_SEARCH,              // Быстрый поиск в панелях
  MCODE_C_AREA_DISKS,               // Меню выбора дисков
  MCODE_C_AREA_MAINMENU,            // Основное меню
  MCODE_C_AREA_MENU,                // Прочие меню
  MCODE_C_AREA_HELP,                // Система помощи
  MCODE_C_AREA_INFOPANEL,           // Информационная панель
  MCODE_C_AREA_QVIEWPANEL,          // Панель быстрого просмотра
  MCODE_C_AREA_TREEPANEL,           // Панель дерева папок
  MCODE_C_AREA_FINDFOLDER,          // Поиск папок
  MCODE_C_AREA_USERMENU,            // Меню пользователя

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
  MCODE_V_FAR_WIDTH=KEY_MACRO_V_BASE,// Far.Width - ширина консольного окна
  MCODE_V_FAR_HEIGHT,               // Far.Height - высота консольного окна
  MCODE_V_FAR_TITLE,                // Far.Title - текущий заголовок консольного окна

  MCODE_V_APANEL_CURRENT,           // APanel.Current - имя файла на активной панели
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
  MCODE_V_APANEL_HEIGHT,            // APanel.Height - активная панель:  высота панели
  MCODE_V_PPANEL_HEIGHT,            // PPanel.Height - пассивная панель: высота панели
  MCODE_V_APANEL_COLUMNCOUNT,       // APanel.ColumnCount - активная панель:  количество колонок
  MCODE_V_PPANEL_COLUMNCOUNT,       // PPanel.ColumnCount - пассивная панель: количество колонок

  MCODE_V_ITEMCOUNT,                // ItemCount - число элементов в текущем объекте
  MCODE_V_CURPOS,                   // CurPos - текущий индекс в текущем объекте
  MCODE_V_TITLE,                    // Title - заголовок текущего объекта
  MCODE_V_HEIGHT,                   // Height - высота текущего объекта
  MCODE_V_WIDTH,                    // Width - ширина текущего объекта

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

  MCODE_V_DRVSHOWPOS,               // Drv.ShowPos - меню выбора дисков отображено: 1=слева (Alt-F1), 2=справа (Alt-F2), 0="нету его"
  MCODE_V_DRVSHOWMODE,              // Drv.ShowMode - режимы отображения меню выбора дисков

  MCODE_V_HELPFILENAME,             // Help.FileName
  MCODE_V_HELPTOPIC,                // Help.Topic
  MCODE_V_HELPSELTOPIC,             // Help.SelTopic
};

typedef enum MACRO_OP_CODE TFunction;


#endif // __MACROOPCODE_HPP__
