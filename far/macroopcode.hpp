/*
macroopcode.hpp

OpCode для макросов

*/

/* Revision: 1.01 02.08.2004 $ */

/*
Modify:
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
  MCODE_OP_SAVE,                    // Присваивание переменной. Имя переменной
                                    // следующие DWORD (как в $Text).
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


  /* ************************************************************************* */
  // булевые переменные - различные состояния
  MCODE_C_DISABLEOUTPUT=KEY_MACRO_C_BASE,// вывод запрещен?
  MCODE_C_WINDOWEDMODE,             // оконный режим?
  MCODE_C_SELECTED,                 // выделенный блок есть?
  MCODE_C_EOF,                      // конец файла/активного каталога?
  MCODE_C_BOF,                      // начало файла/активного каталога?
  MCODE_C_EMPTY,                    // ком.строка пуста?
  MCODE_C_ICLIP,                    // внутренний или внешний клипборд
  MCODE_C_ROOTFOLDER,               // аналог MCODE_C_APANEL_ROOT для активной панели

  MCODE_C_APANEL_ISEMPTY,           // активная панель:  пуста?
  MCODE_C_PPANEL_ISEMPTY,           // пассивная панель: пуста?
  MCODE_C_APANEL_BOF,               // начало активного  каталога?
  MCODE_C_PPANEL_BOF,               // начало пассивного каталога?
  MCODE_C_APANEL_EOF,               // конец активного  каталога?
  MCODE_C_PPANEL_EOF,               // конец пассивного каталога?
  MCODE_C_APANEL_ROOT,              // это корневой каталог активной панели?
  MCODE_C_PPANEL_ROOT,              // это корневой каталог пассивной панели?
  MCODE_C_APANEL_VISIBLE,           // активная панель:  видима?
  MCODE_C_PPANEL_VISIBLE,           // пассивная панель: видима?
  MCODE_C_APANEL_PLUGIN,            // активная панель:  плагиновая?
  MCODE_C_PPANEL_PLUGIN,            // пассивная панель: плагиновая?
  MCODE_C_APANEL_FOLDER,            // активная панель:  текущий элемент каталог?
  MCODE_C_PPANEL_FOLDER,            // пассивная панель: текущий элемент каталог?
  MCODE_C_APANEL_SELECTED,          // активная панель:  выделенные элементы есть?
  MCODE_C_PPANEL_SELECTED,          // пассивная панель: выделенные элементы есть?
  MCODE_C_APANEL_LEFT,              // активная панель левая?
  MCODE_C_PPANEL_LEFT,              // пассивная панель левая?

  MCODE_C_CMDLINE_BOF,              // курсор в начале cmd-строки редактирования?
  MCODE_C_CMDLINE_EOF,              // курсор в конеце cmd-строки редактирования?

  /* ************************************************************************* */
  // не булевые переменные
  MCODE_V_APANEL_CURRENT=KEY_MACRO_V_BASE,// APanel.Current - имя файла на активной панели
  MCODE_V_PPANEL_CURRENT,           // PPanel.Current - имя файла на пассивной панели
  MCODE_V_APANEL_SELCOUNT,          // APanel.SelСount - активная панель:  число выделенных элементов
  MCODE_V_PPANEL_SELCOUNT,          // PPanel.SelСount - пассивная панель: число выделенных элементов
  MCODE_V_APANEL_PATH,              // APanel.Path - активная панель:  путь на панели
  MCODE_V_PPANEL_PATH,              // PPanel.Path - пассивная панель: путь на панели
  MCODE_V_APANEL_WIDTH,             // APanel.Path - активная панель:  путь на панели
  MCODE_V_PPANEL_WIDTH,             // PPanel.Path - пассивная панель: путь на панели
  MCODE_V_EDITORSTATE,
};

typedef enum MACRO_OP_CODE TFunction;


#endif // __MACROOPCODE_HPP__
