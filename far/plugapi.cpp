/*
plugapi.cpp

API, доступное плагинам (диалоги, меню, ...)

*/

/* Revision: 1.131 10.05.2002 $ */

/*
Modify:
  10.05.2002 SVS
    - BugZ#502 - вызов Info.Control для из редактора far /e
      Запрещаем использование Control для CmdMode=1 (для /e и /v)
    + FCTL_CHECKPANELSEXIST - работаем в командном режиме (панели доступны?)
  30.04.2002 SVS
    - Снова Message :-(
  15.04.2002 SVS
    - Message(FMSG_ALLINONE,"\n\nFoobar") приводил к падению
  11.04.2002 SVS
    + FCTL_GET[ANOTHER]PANELSHORTINFO
    - Message(FMSG_ALLINONE,"\nFoobar") приводил к падению (первым '\n'
      пытались подавить вывод заголовка месагбокса)
  04.04.2002 SVS
    - Если в меню указали первым итемом сепаратор, но не указали
      selected, то получаем на экране лажу.
  30.03.2002 OT
    - После исправления бага №314 (патч 1250) отвалилось закрытие
      фара по кресту.
  26.03.2002 DJ
    ! разрешим GetMsg() при ManagerIsDown()
    ! ScanTree::GetNextName() принимает размер буфера для имени файла
  25.03.2002 SVS
    ! Запрет вызовов некторых функций АПИ, если ManagerIsDown() == FALSE
  22.03.2002 SVS
    - strcpy - Fuck!
  18.03.2002 SVS
    ! Уточнения, в связи с введением Opt.Dialogs
  17.03.2002 IS
    - FarCharTable: по возможности используем значение TableName (BugZ#331)
  01.03.2002 SVS
    ! Есть только одна функция создания временного файла - FarMkTempEx
  28.02.2002 SVS
    - Для ACTL_CONSOLEMODE+CONSOLE_GET_MODE
      Param=NULL - это... обычный (void*)0, так шта...
      К тому же "*(int*)Param" - это полная хрень.
    ! ACTL_*WINDOW* - Добавим проверку на FrameManager != NULL
    - ACTL_GETWINDOWINFO - забыли выставить корректное значение WindowInfo.Pos
  19.02.2002 SVS
    ! Добавим проверку на PluginNumber в Menu, Message & Dialog
  13.02.2002 SVS
    + MIF_USETEXTPTR - щоб юзать TextPtr
    ! Уборка варнингов
  11.02.2002 SVS
    + Добавка в меню - акселератор - решение BugZ#299
    ! Сепаратор может иметь лэйб только для варианта с Ex
  05.02.2002 SVS
    ! Технологический патч про отладку
  30.01.2002 DJ
    + ACTL_GETDESCSETTINGS
  22.01.2002 SVS
    ! Включим обратно Opt.ExceptRules в диалогах, ибо теперь я точно знаю,
      что падение было в ДИАЛОГЕ! ;-)
    ! немного косметики в сорцах.
  10.01.2002 SVS
    ! Немного SYSLOG_KEYMACRO
    ! Временная отмена новых редакторных флагов
  26.12.2001 SVS
    + EF_USEEXISTING, EF_BREAKIFOPEN - поведение при открытии редактора
      поменяли, а плагинам обломится что ли?
  24.12.2001 SVS
    - Бага с прорисовкой (при заходе в битый архив выводится месаг, но потом
      панели не обновляются, или просмотр HLF-файла - аналогично)
    ! Для LIF_SEPARATOR обнуляем пункт меню.
  22.12.2001 VVM
    + ACTL_GETWINDOWINFO: Если Pos == -1 то берем текущий фрейм
  07.12.2001 IS
    + FarInputBox - обертка вокруг GetString для плагинов - с меньшей
      функциональностью. Сделано для того, чтобы не дублировать код GetString.
  05.12.2001 SVS
    ! Временно отключаем обработку исключений на этом уровне.
  30.11.2001 DJ
    - не вылетаем, если GetPluginDirList() вызывается не на плагиновой панели
    - в GetPluginDirList() некорректно копировалась PPIF_USERDATA для каталогов
  28.11.2001 SVS
    + FIS_SHOWCOPYINGTIMEINFO
    ! Немного оптимизации для ACTL_GET*SETTINGS
  24.11.2001 IS
    + Обработка ACTL_GETSYSTEMSETTINGS, ACTL_GETPANELSETTINGS,
      ACTL_GETINTERFACESETTINGS, ACTL_GETCONFIRMATIONS,
  02.11.2001 SVS
    ! Выкинем ненужный код (а кое-где добавим :-))
    ! FCTL_GETCMDLINESELECTION -> FCTL_GETCMDLINESELECTEDTEXT
    + FCTL_GETCMDLINESELECTION - получить позиции выделения!
  28.10.2001 SVS
    ! Вернем обратно опрометчево убранный SaveScreen.
  26.10.2001 SVS
    - неверно инициализировался массив MsgItems в FarMessageFn()
  22.10.2001 SVS
    - уточнение (восстановление) поведения Message для плагинов
  22.10.2001 SVS
    ! Заюзаем вместо +16 константу ADDSPACEFORPSTRFORMESSAGE
  21.10.2001 SVS
    + CALLBACK-функция для избавления от BugZ#85
    ! Переработанный Message для плагинов (снято ограничение на 13 строк)
  11.10.2001 IS
    + обработка EF_DELETEONCLOSE
  08.10.2001 OT
    ! Запуск немодального фрейма в модальном режиме
  07.10.2001 SVS
    - Нету у нас немодальных диалогов!
  27.09.2001 IS
    - Левый размер при использовании strncpy
  22.09.2001 OT
    Вызов Viewer и Editor из меню плагина засовывает куда-то в background window
  21.09.2001 SVS
    - Бага. При старте ФАР, если плагин вызывает Message или Dialog, то
      ФАРу плохеет, т.к. Frame как такового нету. В общем введена проверка
      на NULL для Lock/Unlock
  20.09.2001 SVS
    ! ограничим диалоги и месагбоксы, вызванные из плагинов "скобками
      необновления нижнего экрана"
      А как на счет вызова меню из плагина???
  16.09.2001 SVS
    ! Отключаемые исключения
  15.09.2001 tran
    + ACTL_GETFARHWND
  09.09.2001 IS
    + Обработка VF_DISABLEHISTORY/EF_DISABLEHISTORY - плагин может запрещать
      добавление имени файла в историю.
  08.09.2001 IS
    ! Теперь имя файла добавляется в историю просмотра/редактирования
      при использовании функций апи Editor/Viewer
  13.08.2001 SKV
    + FCTL_GETCMDLINESELECTION,FCTL_SETCMDLINESELECTION
  13.08.2001 SVS
    - Забыл в прошлый раз проставить интернал PluginNumber в функции
      GetPluginDirList() для случая, если hPlugin=INVALID_HANDLE_VALUE
  07.08.2001 IS
    + Фича в FarCharTable: при неудаче считывания настроек по определенной
      символьной таблице структура CharTableSet заполняется данными для OEM.
    ! FarCharTable: второй параметр теперь не const, т.к. он меняется в функции.
  05.08.2001 SVS
    - Бага с вызовом хелпа в диалогах из плагинов
    ! Убираем из борланда ненужные (тупорылые) варнинги
  01.08.2001 SVS
    + Поменяем правила игры с новыми флагами (про тему помощи)!
  31.07.2001 IS
    + Изменения и новое в FarDialogEx, FarMenuFn, FarMessageFn:
      1. Учтем флаг F*_CUSTOMNAME
      2. Выкинем спецобработку для случая с *HelpTopic=='#'
      3. Считается, что помощи нет, если имя темы пустое.
  31.07.2001 IS
    - Баг: меню плагина закрывалось в случае нажатия на
      ctrl-key, alt-key или shift-key, даже если эти комбинации
      вовсе не были указаны в BreakKeys, а плагину было нужно
      отследить нажатие на просто key.
    ! Внедрение const (FarGetMsgFn)
  31.07.2001 SVS
    + Обработка хелпов по шаблону: ~Text~@#Path#Topic@
  27.07.2001 SVS
    + кусок для тестовых выкрутасов с ACTL_POSTKEYSEQUENCE
    + Дадим возможность плагина при вызове GetPluginDirList
      ссылаться на активную панель, т.е. hPlugin=INVALID_HANDLE_VALUE.
      По аналогии с Control
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
  19.07.2001 OT
    Мелкий баг с неотрисовкой
  18.07.2001 OT
    VFMenu
  16.07.2001 SVS
    + Обработка FarMenuItemEx в FarMenu
  26.06.2001 SVS
    ! __except -> EXCEPT
  26.06.2001 SKV
    + ACTL_COMMIT
  25.06.2001 IS
   ! Внедрение const
  21.06.2001 SVS
    ! ACTL_POSTSEQUENCEKEY -> ACTL_POSTKEYSEQUENCE - (с точки зрения eng)
    ! SequenceKey           -> KeySequence
    ! В ACTL_GETWINDOWINFO добавим проверку Param на NULL
  20.06.2001 SVS
    ! ACTL_PROCESSSEQUENCEKEY -> ACTL_POSTSEQUENCEKEY, в т.ч. механизм
      этой команды теперь основывается на Macro API
  19.06.2001 IS
    - Баг: не работало автоопределение кодировки после 268.
  06.06.2001 SVS
    - Во время исполнения макроса в меню игнорировались BreakKeys.
  05.06.2001 tran
    + ACTL_GETWINDOWCOUNT,ACTL_GETWINDOWINFO,ACTL_SETCURRENTWINDOW
  04.06.2001 SVS
    ! Фигня по поводу Checked символа в меню. Исправлено.
  03.06.2001 SVS
    ! Изменения в связи с переделкой UserData в VMenu
  31.05.2001 OT
    - ExitCode в соответствии с официальной документацией plugin.hlp
  27.05.2001 OT
    - Исправление вызовов конструкторов FileView и FileEdit в FarViewer() и FarEditor()
  26.05.2001 OT
    - Выпрямление логики вызовов в NFZ
  21.05.2001 DJ
    + FDLG_NONMODAL
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    ! Константы MENU_ - в морг
  16.05.2001 DJ
    ! proof-of-concept
  16.05.2001 SVS
    + #include "farexcpt.hpp"
  15.05.2001 OT
    ! NWZ -> NFZ
  14.05.2001 SVS
    ! FDLG_SMALLDILAOG -> FDLG_SMALLDIALOG
  12.05.2001 DJ
    + EF_ENABLE_F6, VF_ENABLE_F6
  12.05.2001 DJ
    ! убран SaveScr при вызове редактора/вьюера из плагина
  10.05.2001 SVS
    + В FarDialogEx() добавлена установка флагов FDLG_WARNING и FDLG_SMALLDILAOG.
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  01.05.2001 SVS
    - Вместо PluginNumber в FarDialogEx() залудил ItemsNumber :-(((
  28.04.2001 SVS
    + Обработка исключений в FarDialogEx() - равеновский чекер валил ФАР
      именно в диалогах, гад такой.
  11.09.2001 SVS
    - FarMessageFn(): если установлен флаг FMSG_ERRORTYPE, ItemsNumber равное 1,
      тоже должно позволять показывать сообщение.
    - FarMessageFn(): неверно сделанный патч #530 - исправляем.
  09.04.2001 SVS
    ! Избавимся от некоторых варнингов
  08.04.2001 SVS
    ! Уточнение в FarShowHelp
  28.03.2001 SVS
    ! ACTL_GETFARVERSION возвращает номер версии, а не TRUE - так практичнее
    ! ACTL_KEYMACRO, ACTL_EJECTMEDIA, ACTL_WAITKEY, ACTL_CONSOLEMODE -
      проверка Param на NULL.
    ! FarGetMsgFn - проверка на "готовность" CtrlObject (проверка на NULL)
  26.03.2001 SVS
    + добавлена обработка флага FHELP_USECONTENTS
  22.03.2001 tran 1.42
    ! мелкий баг в FarMessageFn/FMSG_ALLINONE
  21.03.2001 VVM
    + Обработка флага EF_CREATENEW
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  11.02.2001 SVS
    ! Сократим повторяющийся код в FarDialogEx
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  28.01.2001 SVS
    ! Конкретно обновим функцию FarMessageFn()
  23.01.2001 SVS
    ! Проверки параметров в FarDialogEx()
  21.01.2001 SVS
    + ACTL_PROCESSSEQUENCEKEY
  24.12.2000 SVS
    ! Отключаем "MCMD_PLAYSTRING"
    ! Уточнения MCMD_LOADALL и MCMD_SAVEALL - не работать во время записи
      или "воспроизведения макроса
  23.12.2000 SVS
    + MCMD_PLAYSTRING - "проиграть" строку (строка в том виде, как в реестре)
  21.12.2000 SVS
    + ACTL_KEYMACRO - зачатки будущего KeyMacro API
  18.12.2000 SVS
    ! Коррекции в FarShowHelp
  14.12.2000 SVS
    + ACTL_EJECTMEDIA
  13.12.2000 SVS
    ! FarDialogItem.Data - копирование strcpy заменено на memmove
      (терялись данные пользователя)
  04.12.2000 SVS
    + ACTL_GETCOLOR - получить определенный цвет
    + ACTL_GETARRAYCOLOR - получить весь массив цветов
  17.11.2000 SVS
    ! "Приколы нашего городка" - бага в функцию ShowHelp закралась :-(
  11.11.2000 SVS
    ! FarMkTemp() - убираем (как всегда - то ставим, то тут же убираем :-(((
  11.11.2000 SVS
    ! Используем конструкцию FarMkTemp()
  03.11.2000 OT
    ! Введение проверки возвращаемого значения
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  05.10.2000 SVS
   - бага с вызовом хелпа (FHELP_CUSTOMFILE)
  27.09.2000 SVS
   + FarViewerControl
  18.09.2000 SVS
    ! Функция FarDialogEx имеет 2 дополнительных параметра (Future)
  12.09.2000 SVS
    + Реализация флагов FHELP_* для вывода помощи.
  08.09.2000 VVM
    + Обработка команд
      FCTL_SETSORTMODE, FCTL_SETANOTHERSORTMODE
      FCTL_SETSORTORDER, FCTL_SETANOTHERSORTORDER
  30.08.2000 SVS
    ! Пал смертью храбрых флаг FMI_GETFARMSGID
  29.08.2000 SVS
    + Для диалога запомним номер плагина, вызвавшего этот диалог. Сейчас
      это для того, чтобы правильно отреагировать в Dialog API на DN_HELP
  29.08.2000 SVS
    ! Если PluginStartupInfo.GetMsg(?,N|FMI_GETFARMSGID), то подразумеваем, что
      хотим использовать "месаги" из САМОГО far*.lng
  24.08.2000 SVS
    + ACTL_WAITKEY - ожидать определенную (или любую) клавишу
  23.08.2000 SVS
    ! Все Flags приведены к одному виду -> DWORD.
      Модифицированы:
        * функции   FarMenuFn, FarMessageFn, FarShowHelp
        * структуры FarListItem, FarDialogItem
  22.08.2000 SVS
    ! Исключаем ненужные вызовы из FarText.
  18.08.2000 tran 1.12
    + Flags parameter in FarShowHelp
  09.08.2000 tran 1.11
    ! ACTL_GETSYSWORDDIV при Param==NULL просто возвращает длину строки
  03.08.2000 SVS
    + ACTL_GETSYSWORDDIV получить строку с символами разделителями слов
  01.08.2000 SVS
    ! FARDIALOGPROC -> FARWINDOWPROC
  28.07.2000 SVS
    ! В связи с появлением SendDlgMessage в классе Dialog
      вносим некоторые изменения!
  25.07.2000 SVS
    + Программое переключение FulScreen <-> Windowed (ACTL_CONSOLEMODE)
  23.07.2000 SVS
    + Функция FarDefDlgProc обработки диалога по умолчанию
    + Функция FarSendDlgMessage - посылка сообщения диалогу
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  12.07.2000 IS
    + Проверка флагов редактора в FarEditor (раньше они игнорировались) и
      открытие _немодального_ редактора, если есть соответствующий флаг.
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  05.07.2000 IS
    + Функция FarAdvControl
    + Команда ACTL_GETFARVERSION (в FarAdvControl)
  03.07.2000 IS
    + Функция вывода помощи
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "struct.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "scantree.hpp"
#include "rdrwdsk.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "plugins.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "frame.hpp"
#include "scrbuf.hpp"
#include "farexcpt.hpp"

// declare in plugins.cpp
extern int KeepUserScreen;
extern char DirToSet[NM];


void ScanPluginDir();


/* $ 07.12.2001 IS
   Обертка вокруг GetString для плагинов - с меньшей функциональностью.
   Сделано для того, чтобы не дублировать код GetString.
*/
int WINAPI FarInputBox(const char *Title,const char *Prompt,
                     const char *HistoryName,const char *SrcText,
    char *DestText,int DestLength,const char *HelpTopic,DWORD Flags)
{
  if (FrameManager->ManagerIsDown())
    return FALSE;
   return GetString(Title,Prompt,HistoryName,SrcText,DestText,DestLength,
     HelpTopic,Flags&~FIB_CHECKBOX,NULL,NULL);
}
/* IS $ */

/* $ 12.09.2000 SVS
  + Реализация флагов для вывода помощи.
*/
/* $ 18.08.2000 tran
   + Flags parameter */
/* $ 03.07.2000 IS
  Функция вывода помощи
*/
BOOL WINAPI FarShowHelp(const char *ModuleName,
                        const char *HelpTopic,DWORD Flags)
{
  if (FrameManager->ManagerIsDown())
    return FALSE;
  if (!HelpTopic)
    HelpTopic="Contents";

  DWORD OFlags=Flags;
  Flags&=~(FHELP_NOSHOWERROR|FHELP_USECONTENTS);
  char Path[2*NM],Topic[512];
  char *Mask=NULL;

  // двоеточие в начале топика надо бы игнорировать и в том случае,
  // если стоит FHELP_FARHELP...
  if((Flags&FHELP_FARHELP) || *HelpTopic==':')
    strncpy(Topic,HelpTopic+((*HelpTopic == ':')?1:0),sizeof(Topic)-1);
  else
  {
    if(ModuleName)
    {
      // FHELP_SELFHELP=0 - трактовать первый пар-р как Info.ModuleName
      //                   и показать топик из хелпа вызвавшего плагина
      /* $ 17.11.2000 SVS
         А значение FHELP_SELFHELP равно чему? Правильно - 0
         И фигля здесь удивлятся тому, что функция не работает :-(
      */
      if(Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE|FHELP_CUSTOMPATH)))
      {
        strncpy(Path,ModuleName,sizeof(Path)-1);
        if(Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE)))
        {
          Mask=PointToName(Path);
          if(Flags&FHELP_CUSTOMFILE)
          {
            memmove(Mask+1,Mask,strlen(Mask)+1);
            *Mask++=0;
          }
          else
          {
            *Mask=0;
            Mask=NULL;
          }
        }
      }
      else
        return FALSE;
      /* SVS $*/

      sprintf(Topic,HelpFormatLink,Path,HelpTopic);
    }
    else
      return FALSE;
  }
  {
    Help Hlp (Topic,Mask,OFlags);
    if(Hlp.GetError())
      return FALSE;
  }
  return TRUE;
}
/* IS $ */
/* tran 18.08.2000 $ */
/* SVS 12.09.2000 $ */


/* $ 05.07.2000 IS
  Функция, которая будет действовать и в редакторе, и в панелях, и...
*/
#ifndef _MSC_VER
#pragma warn -par
#endif
int WINAPI FarAdvControl(int ModuleNumber, int Command, void *Param)
{
  struct Opt2Flags{
    int *Opt;
    DWORD Flags;
  };
  int I;

  switch(Command)
  {
    case ACTL_GETFARVERSION:
    case ACTL_GETSYSWORDDIV:
    case ACTL_GETCOLOR:
    case ACTL_GETARRAYCOLOR:
    case ACTL_GETFARHWND:
    case ACTL_GETSYSTEMSETTINGS:
    case ACTL_GETPANELSETTINGS:
    case ACTL_GETINTERFACESETTINGS:
    case ACTL_GETCONFIRMATIONS:
    case ACTL_GETDESCSETTINGS:
      break;
    default:
     if (FrameManager && FrameManager->ManagerIsDown())
       return 0;
  }

  switch(Command)
  {
    case ACTL_GETFARVERSION:
    {
      if(Param)
        *(DWORD*)Param=FAR_VERSION;
      return FAR_VERSION;
    }

    /* $ 25.07.2000 SVS
       + Программое переключение FulScreen <-> Windowed (ACTL_CONSOLEMODE)
       mode = -2 - получить текущее состояние
              -1 - как тригер
               0 - Windowed
               1 - FulScreen
       Return
               0 - Windowed
               1 - FulScreen
    */
    case ACTL_CONSOLEMODE:
    {
      return FarAltEnter((int)Param);
    }
    /* SVS $ */

    /* $ 03.08.2000 SVS
       получение строки с разделителями слов
       Возвращает размер полученных данных без '\0'
       Максимальный размер приемного буфера = 80 с заключительным '\0'
       Строка выбирается не из реестра, а из Opt.
    */
    case ACTL_GETSYSWORDDIV:
    {
      int LenWordDiv=strlen(Opt.WordDiv);
      /* $ 09.08.2000 tran
       + if param==NULL, plugin хочет только узнать длину строки  */
      if (Param && !IsBadWritePtr(Param,LenWordDiv+1))
        strcpy((char *)Param,Opt.WordDiv);
      /* tran 09.08.2000 $ */
      return LenWordDiv;
    }
    /* SVS $ */

    /* $ 24.08.2000 SVS
       ожидать определенную (или любую) клавишу
       (int)Param - внутренний код клавиши, которую ожидаем, или -1
       если все равно какую клавишу ждать.
       возвращает 0;
    */
    case ACTL_WAITKEY:
    {
      WaitKey(Param?(int)Param:-1);
      return 0;
    }
    /* SVS $ */

    /* $ 04.12.2000 SVS
      ACTL_GETCOLOR - получить определенный цвет по индекс, определенному
       в farcolor.hpp
      (int)Param - индекс.
      Return - значение цвета или -1 если индекс неверен.
    */
    case ACTL_GETCOLOR:
    {
      if((int)Param < SizeArrayPalette && (int)Param >= 0)
        return (int)((unsigned int)Palette[(int)Param]);
      return -1;
    }
    /* SVS $ */

    /* $ 04.12.2000 SVS
      ACTL_GETARRAYCOLOR - получить весь массив цветов
      Param - указатель на массив или NULL - чтобы получить размер буфера
      Return - размер массива.
    */
    case ACTL_GETARRAYCOLOR:
    {
      if(Param && !IsBadWritePtr(Param,SizeArrayPalette))
        memmove(Param,Palette,SizeArrayPalette);
      return SizeArrayPalette;
    }
    /* SVS $ */

    /* $ 14.12.2000 SVS
      ACTL_EJECTMEDIA - извлечь диск из съемного накопителя
      Param - указатель на структуру ActlEjectMedia
      Return - TRUE - успешное извлечение, FALSE - ошибка.
    */
    case ACTL_EJECTMEDIA:
    {
      return Param?EjectVolume((char)((ActlEjectMedia*)Param)->Letter,
                               ((ActlEjectMedia*)Param)->Flags):FALSE;
    }
    /* $ 21.12.2000 SVS
       Macro API
    */
    case ACTL_KEYMACRO:
    {
      if(CtrlObject && Param) // все зависит от этой бадяги.
      {
        KeyMacro& Macro=CtrlObject->Macro; //??
        struct ActlKeyMacro *KeyMacro=(struct ActlKeyMacro*)Param;
        switch(KeyMacro->Command)
        {
          case MCMD_LOADALL: // из реестра в память ФАР с затиранием предыдущего
            if(Macro.IsRecording() || Macro.IsExecuting())
              return FALSE;
            return Macro.LoadMacros();

          case MCMD_SAVEALL: // из памяти ФАРа в реестра
            if(Macro.IsRecording() || Macro.IsExecuting())
              return FALSE;
            Macro.SaveMacros();
            return TRUE;
#if 0
          /* $ 23.12.2000 SVS
               MCMD_PLAYSTRING - "проиграть" строку (строка в том виде,
               как в реестре)
             Param - указатель на структуру struct ActlKeyMacro.
          */
          case MCMD_PLAYSTRING:
            if(KeyMacro->Str.KeyBuffer && *KeyMacro->Str.KeyBuffer)
               return Macro.PlayKeyMacro(KeyMacro->Str.KeyBuffer);
            return FALSE;
#endif
        }
      }
      return FALSE;
    }

    case ACTL_POSTKEYSEQUENCE:
    {
      if(CtrlObject && Param && ((struct KeySequence*)Param)->Count > 0)
      {
        struct MacroRecord MRec;
        MRec.Flags=(((struct KeySequence*)Param)->Flags)<<16;
        MRec.Key=0;
        MRec.BufferSize=((struct KeySequence*)Param)->Count;
        MRec.Buffer=((struct KeySequence*)Param)->Sequence;
        return CtrlObject->Macro.PostTempKeyMacro(&MRec);
#if 0
        // Этот кусок - для дальнейших экспериментов
        {
          //CtrlObject->Macro.PostTempKeyMacro(&MRec);
          for(int I=0; I < MRec.BufferSize; ++I)
          {
            int Key=MRec.Buffer[I];
            if(CtrlObject->Macro.ProcessKey(Key))
            {
              while((Key=CtrlObject->Macro.GetKey()) != 0)
              {
                FrameManager->ProcessKey(Key);
              }
            }
            else
              FrameManager->ProcessKey(Key);
            FrameManager->PluginCommit();
          }
          return TRUE;
        }
#endif
      }
      return FALSE;
    }

    /* $ 05.06.2001 tran
       новые ACTL_ для работы с фреймами */
    case ACTL_GETWINDOWINFO:
    {
      if(FrameManager && Param && !IsBadWritePtr(Param,sizeof(WindowInfo)))
      {
        WindowInfo *wi=(WindowInfo*)Param;
        Frame *f;
        /* $ 22.12.2001 VVM
          + Если Pos == -1 то берем текущий фрейм */
        if (wi->Pos == -1)
          f=FrameManager->GetCurrentFrame();
        else
          f=FrameManager->operator[](wi->Pos);
        /* VVM $ */
        if ( f==NULL )
          return FALSE;
        f->GetTypeAndName(wi->TypeName,wi->Name);
        wi->Pos=FrameManager->IndexOf(f);
        wi->Type=f->GetType();
        wi->Modified=f->IsFileModified();
        wi->Current=f==FrameManager->GetCurrentFrame();
        return TRUE;
      }
      return FALSE;
    }

    case ACTL_GETWINDOWCOUNT:
    {
      return FrameManager?FrameManager->GetFrameCount():0;
    }

    case ACTL_SETCURRENTWINDOW:
    {
      if (FrameManager && FrameManager->operator[]((int)Param)!=NULL )
      {
        FrameManager->ActivateFrame((int)Param);
        return TRUE;
      }
      return FALSE;
    }
    /* tran 05.06.2001 $ */
    /*$ 26.06.2001 SKV
      Для полноценной работы с ACTL_SETCURRENTWINDOW
      (и может еще для чего в будущем)
    */
    case ACTL_COMMIT:
    {
      return FrameManager?FrameManager->PluginCommit():FALSE;
    }
    /* SKV$*/
    /* $ 15.09.2001 tran
       пригодится плагинам */
    case ACTL_GETFARHWND:
    {
      return (int)hFarWnd;
    }
    /* tran $ */

    /* $ 24.11.2001 IS
       Ознакомим с настройками системными, панели, интерфейса, подтверждений
    */
    case ACTL_GETSYSTEMSETTINGS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.ClearReadOnly,FSS_CLEARROATTRIBUTE},
        {&Opt.DeleteToRecycleBin,FSS_DELETETORECYCLEBIN},
        {&Opt.UseSystemCopy,FSS_USESYSTEMCOPYROUTINE},
        {&Opt.CopyOpened,FSS_COPYFILESOPENEDFORWRITING},
        {&Opt.CreateUppercaseFolders,FSS_CREATEFOLDERSINUPPERCASE},
        {&Opt.SaveHistory,FSS_SAVECOMMANDSHISTORY},
        {&Opt.SaveFoldersHistory,FSS_SAVEFOLDERSHISTORY},
        {&Opt.SaveViewHistory,FSS_SAVEVIEWANDEDITHISTORY},
        {&Opt.UseRegisteredTypes,FSS_USEWINDOWSREGISTEREDTYPES},
        {&Opt.AutoSaveSetup,FSS_AUTOSAVESETUP},
      };
      for(I=0; I < sizeof(OSys)/sizeof(OSys[0]); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }

    case ACTL_GETPANELSETTINGS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.ShowHidden,FPS_SHOWHIDDENANDSYSTEMFILES},
        {&Opt.Highlight,FPS_HIGHLIGHTFILES},
        {&Opt.AutoChangeFolder,FPS_AUTOCHANGEFOLDER},
        {&Opt.SelectFolders,FPS_SELECTFOLDERS},
        {&Opt.ReverseSort,FPS_ALLOWREVERSESORTMODES},
        {&Opt.ShowColumnTitles,FPS_SHOWCOLUMNTITLES},
        {&Opt.ShowPanelStatus,FPS_SHOWSTATUSLINE},
        {&Opt.ShowPanelTotals,FPS_SHOWFILESTOTALINFORMATION},
        {&Opt.ShowPanelFree,FPS_SHOWFREESIZE},
        {&Opt.ShowPanelScrollbar,FPS_SHOWSCROLLBAR},
        {&Opt.ShowScreensNumber,FPS_SHOWBACKGROUNDSCREENSNUMBER},
        {&Opt.ShowSortMode,FPS_SHOWSORTMODELETTER},
      };
      for(I=0; I < sizeof(OSys)/sizeof(OSys[0]); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }

    case ACTL_GETINTERFACESETTINGS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.Clock,FIS_CLOCKINPANELS},
        {&Opt.ViewerEditorClock,FIS_CLOCKINVIEWERANDEDITOR},
        {&Opt.Mouse,FIS_MOUSE},
        {&Opt.ShowKeyBar,FIS_SHOWKEYBAR},
        {&Opt.ShowMenuBar,FIS_ALWAYSSHOWMENUBAR},
        {&Opt.Dialogs.EditHistory,FIS_HISTORYINDIALOGEDITCONTROLS},
        {&Opt.Dialogs.EditBlock,FIS_PERSISTENTBLOCKSINEDITCONTROLS},
        {&Opt.AltGr,FIS_USERIGHTALTASALTGR},
        {&Opt.CopyShowTotal,FIS_SHOWTOTALCOPYPROGRESSINDICATOR},
        {&Opt.CopyTimeRule,FIS_SHOWCOPYINGTIMEINFO},
        {&Opt.Dialogs.AutoComplete,FIS_AUTOCOMPLETEININPUTLINES},
        {&Opt.PgUpChangeDisk,FIS_USECTRLPGUPTOCHANGEDRIVE},
      };
      for(I=0; I < sizeof(OSys)/sizeof(OSys[0]); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }

    case ACTL_GETCONFIRMATIONS:
    {
      DWORD Options=0;
      static struct Opt2Flags OSys[]={
        {&Opt.Confirm.Copy,FCS_COPYOVERWRITE},
        {&Opt.Confirm.Move,FCS_MOVEOVERWRITE},
        {&Opt.Confirm.Drag,FCS_DRAGANDDROP},
        {&Opt.Confirm.Delete,FCS_DELETE},
        {&Opt.Confirm.DeleteFolder,FCS_DELETENONEMPTYFOLDERS},
        {&Opt.Confirm.Esc,FCS_INTERRUPTOPERATION},
        {&Opt.Confirm.RemoveConnection,FCS_DISCONNECTNETWORKDRIVE},
        {&Opt.Confirm.AllowReedit,FCS_RELOADEDITEDFILE},
        {&Opt.Confirm.HistoryClear,FCS_CLEARHISTORYLIST},
        {&Opt.Confirm.Exit,FCS_EXIT},
      };
      for(I=0; I < sizeof(OSys)/sizeof(OSys[0]); ++I)
        if(*OSys[I].Opt)
          Options|=OSys[I].Flags;
      return Options;
    }
    /* IS $ */

    /* $ 30.01.2002 DJ
       ACTL_GETDESCSETTINGS
    */
    case ACTL_GETDESCSETTINGS:
    {
      // опций мало - с массивом не заморачиваемся
      DWORD Options=0;
      if (Opt.Diz.UpdateMode == DIZ_UPDATE_IF_DISPLAYED)
        Options |= FDS_UPDATEIFDISPLAYED;
      else if (Opt.Diz.UpdateMode == DIZ_UPDATE_ALWAYS)
        Options |= FDS_UPDATEALWAYS;
      if (Opt.Diz.SetHidden)
        Options |= FDS_SETHIDDEN;
      if (Opt.Diz.ROUpdate)
        Options |= FDS_UPDATEREADONLY;
      return Options;
    }
    /* DJ $ */

  }
  return FALSE;
}
#ifndef _MSC_VER
#pragma warn +par
#endif
/* IS $ */

int WINAPI FarMenuFn(int PluginNumber,int X,int Y,int MaxHeight,
           DWORD Flags,const char *Title,const char *Bottom,
           const char *HelpTopic, const int *BreakKeys,int *BreakCode,
           const struct FarMenuItem *Item, int ItemsNumber)
{
  int I;

  if (FrameManager->ManagerIsDown())
    return -1;

  if (DisablePluginsOutput)
    return(-1);

  if((DWORD)PluginNumber >= (DWORD)CtrlObject->Plugins.PluginsCount)
    return(-1); // к терапевту.

  int ExitCode;
  {
    VMenu FarMenu(Title,NULL,0,MaxHeight);
    FarMenu.SetPosition(X,Y,0,0);
    if (BreakCode!=NULL)
      *BreakCode=-1;

    {
      char Topic[512];
      if(Help::MkTopic(PluginNumber,HelpTopic,Topic))
        FarMenu.SetHelp(Topic);
    }

    if (Bottom!=NULL)
      FarMenu.SetBottomTitle(Bottom);

    struct MenuItem CurItem;
    memset(&CurItem,0,sizeof(CurItem));
    int Selected=0;

    if(Flags&FMENU_USEEXT)
    {
      struct FarMenuItemEx *ItemEx=(struct FarMenuItemEx*)Item;
      for (I=0; I < ItemsNumber; I++, ++ItemEx)
      {
        CurItem.Flags=ItemEx->Flags;
        CurItem.NamePtr=NULL; // за раз 4 байта в 0 :-)

        if(CurItem.Flags&MIF_USETEXTPTR)
          CurItem.NamePtr=(char*)ItemEx->Text.TextPtr;
        else
          strncpy(CurItem.Name,ItemEx->Text.Text,sizeof(CurItem.Name)-1);
        /*
        strncpy(CurItem.Name,
            ((ItemEx->Flags&MIF_USETEXTPTR) && ItemEx->Text.TextPtr)?ItemEx->Text.TextPtr:ItemEx->Text.Text,
            sizeof(CurItem.Name)-1);
        */
        CurItem.AccelKey=(CurItem.Flags&LIF_SEPARATOR)?0:ItemEx->AccelKey;
        if(!Selected && (CurItem.Flags&LIF_SELECTED) && !(CurItem.Flags&LIF_SEPARATOR))
          Selected++;
        FarMenu.AddItem(&CurItem);
      }
    }
    else
    {
      for (I=0;I<ItemsNumber;I++)
      {
        CurItem.Flags=Item[I].Checked?(LIF_CHECKED|(Item[I].Checked&0xFFFF)):0;
        CurItem.Flags|=Item[I].Selected?LIF_SELECTED:0;
        CurItem.Flags|=Item[I].Separator?LIF_SEPARATOR:0;
        if(Item[I].Separator)
          CurItem.Name[0]=0;
        else
          strncpy(CurItem.Name,Item[I].Text,sizeof(CurItem.Name)-1);
        if(!Selected && (CurItem.Flags&LIF_SELECTED) && !(CurItem.Flags&LIF_SEPARATOR))
          Selected++;
        FarMenu.AddItem(&CurItem);
      }
    }

    if(!Selected)
      FarMenu.SetSelectPos(0,1);

    DWORD MenuFlags=0;
    if (Flags & FMENU_SHOWAMPERSAND)
      MenuFlags|=VMENU_SHOWAMPERSAND;
    if (Flags & FMENU_WRAPMODE)
      MenuFlags|=VMENU_WRAPMODE;
    if (Flags & FMENU_AUTOHIGHLIGHT)
      FarMenu.AssignHighlights(FALSE);
    if (Flags & FMENU_REVERSEAUTOHIGHLIGHT)
      FarMenu.AssignHighlights(TRUE);
    FarMenu.SetFlags(MenuFlags);
    FarMenu.Show();
    while (!FarMenu.Done() && !CloseFARMenu)
    {
      INPUT_RECORD ReadRec;
      int ReadKey=GetInputRecord(&ReadRec);
      if (ReadRec.EventType==MOUSE_EVENT)
        FarMenu.ProcessMouse(&ReadRec.Event.MouseEvent);
      else
        if (ReadKey!=KEY_NONE)
        {
          if (BreakKeys!=NULL)
          {
            for (int I=0;BreakKeys[I]!=0;I++)
            {
              if(CtrlObject->Macro.IsExecuting())
              {
                int VirtKey,ControlState;
                TranslateKeyToVK(ReadKey,VirtKey,ControlState,&ReadRec);
              }
              if (ReadRec.Event.KeyEvent.wVirtualKeyCode==(BreakKeys[I] & 0xffff))
              {
                DWORD Flags=BreakKeys[I]>>16;
                /* $ 31.07.2001 IS
                   - Баг: меню плагина закрывалось в случае нажатия на
                     ctrl-key, alt-key или shift-key, даже если эти комбинации
                     вовсе не были указаны в BreakKeys, а плагину было нужно
                     отследить нажатие на просто key. Решение: переписан весь
                     кусок по анализу, т.к. предыдущий был полной лажей.
                */
                DWORD RealFlags=ReadRec.Event.KeyEvent.dwControlKeyState &
                      (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED|
                      LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED|SHIFT_PRESSED);

                int Accept;
                if(RealFlags) // нажаты shift, ctrl или alt
                {
                   Accept=FALSE; // т.к. пока ничего не известно
                   if(Flags) // должна быть проверка с учетом ctrl|alt|shift
                   {
                     if ((Flags & PKF_CONTROL) &&
                         (RealFlags & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)))
                       Accept=TRUE;
                     if ((Flags & PKF_ALT) &&
                         (RealFlags & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)))
                       Accept=TRUE;
                     if ((Flags & PKF_SHIFT) && (RealFlags & SHIFT_PRESSED))
                       Accept=TRUE;
                   }
                }
                else
                   Accept=!Flags;  // TRUE только, если нам не нужны сочетания
                                   // вместе с ctrl|alt|shift
                /* IS $ */
                if (Accept)
                {
                  if (BreakCode!=NULL)
                    *BreakCode=I;
                  FarMenu.Hide();
//                  CheckScreenLock();
                  return(FarMenu.GetSelectPos());
                }
              }
            }
          }
          FarMenu.ProcessKey(ReadKey);
        }
    }
    ExitCode=FarMenu.Modal::GetExitCode();
  }
//  CheckScreenLock();
  return(ExitCode);
}

/* $ 23.07.2000 SVS
   Функции для расширенного диалога
*/
// Функция FarDefDlgProc обработки диалога по умолчанию
long WINAPI FarDefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  if(hDlg)  // исключаем лишний вызов для hDlg=0
    return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
  return 0;
}

/* $ 28.07.2000 SVS
    ! В связи с появлением SendDlgMessage в классе Dialog
      вносим некоторые изменения!
*/
// Посылка сообщения диалогу
long WINAPI FarSendDlgMessage(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  if(hDlg) // исключаем лишний вызов для hDlg=0
    return Dialog::SendDlgMessage(hDlg,Msg,Param1,Param2);
  return 0;
}
/* SVS $ */

int WINAPI FarDialogFn(int PluginNumber,int X1,int Y1,int X2,int Y2,
           const char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber)
{
  return FarDialogEx(PluginNumber,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,0,0,NULL,0);
}

/* $   13.12.2000 SVS
   ! FarDialogItem.Data - копирование strcpy заменено на memmove
   (терялись данные пользователя)
*/
#ifndef _MSC_VER
#pragma warn -par
#endif
/* Цель данной функции - выставить флаг Flags - признак того, что
   мы упали где то в плагине
*/
static int Except_FarDialogEx(struct DialogItem *InternalItem)
{
  if(CtrlObject)
    CtrlObject->Plugins.Flags.Set(PSIF_DIALOG);

  // Окончание
  delete[] InternalItem;

  Frame *frame;
  if((frame=FrameManager->GetBottomFrame()) != NULL)
    frame->UnlockRefresh(); // теперь можно :-)
//  CheckScreenLock();
  FrameManager->RefreshFrame(); //??

  return EXCEPTION_CONTINUE_SEARCH; // продолжим исполнения цепочки исключений!
}


int WINAPI FarDialogEx(int PluginNumber,int X1,int Y1,int X2,int Y2,
           const char *HelpTopic,struct FarDialogItem *Item,int ItemsNumber,
           DWORD Reserved, DWORD Flags,
           FARWINDOWPROC DlgProc,long Param)

{
  if (FrameManager->ManagerIsDown())
    return -1;

  if (DisablePluginsOutput ||
      ItemsNumber <= 0 ||
      !Item ||
      IsBadReadPtr(Item,sizeof(struct FarDialogItem)*ItemsNumber))
    return(-1);

  if((DWORD)PluginNumber >= (DWORD)CtrlObject->Plugins.PluginsCount)
    return(-1); // к терапевту.

  // ФИЧА! нельзя указывать отрицательные X2 и Y2
  if(X2 < 0 || Y2 < 0)
  {
    return -1;
  }

  struct DialogItem *InternalItem=new DialogItem[ItemsNumber];

  if(!InternalItem)
    return -1;

  int ExitCode=-1;

  //struct PluginItem *CurPlugin=&CtrlObject->Plugins.PluginsData[PluginNumber];

  memset(InternalItem,0,sizeof(DialogItem)*ItemsNumber);

  Dialog::ConvertItem(CVTITEM_FROMPLUGIN,Item,InternalItem,ItemsNumber);

  Frame *frame;
  if((frame=FrameManager->GetBottomFrame()) != NULL)
    frame->LockRefresh(); // отменим прорисовку фрейма

  {
    Dialog FarDialog(InternalItem,ItemsNumber,DlgProc,Param);
    FarDialog.SetPosition(X1,Y1,X2,Y2);

    if(Flags & FDLG_WARNING)
      FarDialog.SetDialogMode(DMODE_WARNINGSTYLE);
    if(Flags & FDLG_SMALLDIALOG)
      FarDialog.SetDialogMode(DMODE_SMALLDIALOG);
    if(Flags & FDLG_NONMODAL)
      FarDialog.SetCanLoseFocus(TRUE);
    FarDialog.SetOwnsItems(TRUE);
    FarDialog.SetHelp(HelpTopic);

    /* IS $ */
    /* $ 29.08.2000 SVS
       Запомним номер плагина - сейчас в основном для формирования HelpTopic
    */
    FarDialog.SetPluginNumber(PluginNumber);
    /* SVS $ */

    if(Opt.ExceptRules)
    {
      CtrlObject->Plugins.Flags.Skip(PSIF_DIALOG);
      TRY
      {
        FarDialog.Process();
        Dialog::ConvertItem(CVTITEM_TOPLUGIN,Item,InternalItem,ItemsNumber);
        ExitCode=FarDialog.GetExitCode();
      }
      EXCEPT (Except_FarDialogEx(InternalItem))
      {
        ;
      }
    }
    else
    {
      FarDialog.Process();
      Dialog::ConvertItem(CVTITEM_TOPLUGIN,Item,InternalItem,ItemsNumber);
      ExitCode=FarDialog.GetExitCode();
    }
  }

  delete[] InternalItem;

  if((frame=FrameManager->GetBottomFrame()) != NULL)
    frame->UnlockRefresh(); // теперь можно :-)
//  CheckScreenLock();
  FrameManager->RefreshFrame(); //??
  return(ExitCode);
}
#ifndef _MSC_VER
#pragma warn +par
#endif
/* SVS 13.12.2000 $ */
/* SVS $ */

const char* WINAPI FarGetMsgFn(int PluginNumber,int MsgId)
{
  return(CtrlObject?CtrlObject->Plugins.FarGetMsg(PluginNumber,MsgId):"");
}

char* PluginsSet::FarGetMsg(int PluginNumber,int MsgId)
{
  if (PluginNumber<PluginsCount)
  {
    struct PluginItem *CurPlugin=&PluginsData[PluginNumber];
    char Path[NM];
    strncpy(Path,CurPlugin->ModuleName,sizeof(Path)-1);
    *PointToName(Path)=0;
    if (CurPlugin->Lang.Init(Path))
      return(CurPlugin->Lang.GetMsg(MsgId));
  }
  return("");
}

/* $ 28.01.2001 SVS
   ! Конкретно обновим функцию FarMessageFn()
*/

int WINAPI FarMessageFn(int PluginNumber,DWORD Flags,const char *HelpTopic,
                        const char * const *Items,int ItemsNumber,
                        int ButtonsNumber)
{
  if (FrameManager->ManagerIsDown())
    return -1;

  if (DisablePluginsOutput)
    return(-1);

  if ((!(Flags&(FMSG_ALLINONE|FMSG_ERRORTYPE)) && ItemsNumber<2) || !Items)
    return(-1);

  if((DWORD)PluginNumber >= (DWORD)CtrlObject->Plugins.PluginsCount)
    return(-1); // к терапевту.

  char *SingleItems=NULL;
  char *Msg;
  int I;

  // анализ количества строк для FMSG_ALLINONE
  if(Flags&FMSG_ALLINONE)
  {
    ItemsNumber=0;
    I=strlen((char *)Items)+2;
    if((SingleItems=(char *)malloc(I)) == NULL)
      return -1;

    Msg=strcpy(SingleItems,(char *)Items);
    while ((Msg = strchr(Msg, '\n')) != NULL)
    {
//      *Msg='\0';

      if(*++Msg == '\0')
        break;
      ++ItemsNumber;
    }
    ItemsNumber++; //??
  }

  const char **MsgItems=(const char **)malloc(sizeof(char*)*(ItemsNumber+ADDSPACEFORPSTRFORMESSAGE));
  if(!MsgItems)
  {
    free(SingleItems);
    return(-1);
  }

  memset(MsgItems,0,sizeof(char*)*(ItemsNumber+ADDSPACEFORPSTRFORMESSAGE));

  if(Flags&FMSG_ALLINONE)
  {
    I=0;
    Msg=SingleItems;

    // анализ количества строк и разбивка на пункты
    char *MsgTemp;
    while ((MsgTemp = strchr(Msg, '\n')) != NULL)
    {
      *MsgTemp='\0';
      MsgItems[I]=Msg;
      Msg+=strlen(Msg)+1;

      if(*Msg == '\0')
        break;
      ++I;
    }
    if(*Msg)
    {
      MsgItems[I]=Msg;
    }
  }
  else
  {
    for (I=0; I < ItemsNumber; I++)
      MsgItems[I]=Items[I];
  }

  // ограничение на строки
  if(ItemsNumber > ScrY-2)
  {
    ItemsNumber=ScrY-2-(Flags&0x000F0000?1:0);
  }

  /* $ 22.03.2001 tran
     ItemsNumber++ -> ++ItemsNumber
     тереялся последний элемент */
  switch(Flags&0x000F0000)
  {
    case FMSG_MB_OK:
      ButtonsNumber=1;
      MsgItems[ItemsNumber++]=MSG(MOk);
      break;
    case FMSG_MB_OKCANCEL:
      ButtonsNumber=2;
      MsgItems[ItemsNumber++]=MSG(MOk);
      MsgItems[ItemsNumber++]=MSG(MCancel);
      break;
    case FMSG_MB_ABORTRETRYIGNORE:
      ButtonsNumber=3;
      MsgItems[ItemsNumber++]=MSG(MAbort);
      MsgItems[ItemsNumber++]=MSG(MRetry);
      MsgItems[ItemsNumber++]=MSG(MIgnore);
      break;
    case FMSG_MB_YESNO:
      ButtonsNumber=2;
      MsgItems[ItemsNumber++]=MSG(MYes);
      MsgItems[ItemsNumber++]=MSG(MNo);
      break;
    case FMSG_MB_YESNOCANCEL:
      ButtonsNumber=3;
      MsgItems[ItemsNumber++]=MSG(MYes);
      MsgItems[ItemsNumber++]=MSG(MNo);
      MsgItems[ItemsNumber++]=MSG(MCancel);
      break;
    case FMSG_MB_RETRYCANCEL:
      ButtonsNumber=2;
      MsgItems[ItemsNumber++]=MSG(MRetry);
      MsgItems[ItemsNumber++]=MSG(MCancel);
      break;
  }
  /* tran $ */

  // запоминаем топик
  {
    char Topic[512];
    if(Help::MkTopic(PluginNumber,HelpTopic,Topic))
      SetMessageHelp(Topic);
  }

  // непосредственно... вывод
  Frame *frame;
  if((frame=FrameManager->GetBottomFrame()) != NULL)
    frame->LockRefresh(); // отменим прорисовку фрейма
  int MsgCode=Message(Flags,ButtonsNumber,MsgItems[0],MsgItems+1,ItemsNumber-1,PluginNumber);
  if((frame=FrameManager->GetBottomFrame()) != NULL)
    frame->UnlockRefresh(); // теперь можно :-)
  //CheckScreenLock();

  if(SingleItems)
    free(SingleItems);
  free(MsgItems);
  return(MsgCode);
}

int WINAPI FarControl(HANDLE hPlugin,int Command,void *Param)
{
  if(Command == FCTL_CHECKPANELSEXIST)
    return CmdMode == FALSE?TRUE:FALSE;

  if (CmdMode || FrameManager->ManagerIsDown())
    return 0;

  if (CtrlObject->Cp()->LeftPanel==NULL || CtrlObject->Cp()->RightPanel==NULL)
    return(0);

  switch(Command)
  {
    case FCTL_CLOSEPLUGIN:
      strcpy(DirToSet,NullToEmpty((char *)Param));
    case FCTL_GETPANELINFO:
    case FCTL_GETANOTHERPANELINFO:
    case FCTL_GETPANELSHORTINFO:
    case FCTL_GETANOTHERPANELSHORTINFO:
    case FCTL_UPDATEPANEL:
    case FCTL_UPDATEANOTHERPANEL:
    case FCTL_REDRAWPANEL:
    case FCTL_REDRAWANOTHERPANEL:
    case FCTL_SETPANELDIR:
    case FCTL_SETANOTHERPANELDIR:
    case FCTL_SETSELECTION:
    case FCTL_SETANOTHERSELECTION:
    case FCTL_SETVIEWMODE:
    case FCTL_SETANOTHERVIEWMODE:
/* $ VVM 08.09.2000
   + Смена сортировки из плагина
*/
    case FCTL_SETSORTMODE:
    case FCTL_SETANOTHERSORTMODE:
    case FCTL_SETSORTORDER:
    case FCTL_SETANOTHERSORTORDER:
/* VVM $ */
      {
        if (hPlugin==INVALID_HANDLE_VALUE)
        {
          CtrlObject->Cp()->ActivePanel->SetPluginCommand(Command,Param);
          return(TRUE);
        }
        HANDLE hInternal;
        Panel *LeftPanel=CtrlObject->Cp()->LeftPanel;
        Panel *RightPanel=CtrlObject->Cp()->RightPanel;
        int Processed=FALSE;
        if (LeftPanel!=NULL && LeftPanel->GetMode()==PLUGIN_PANEL)
        {
          hInternal=((struct PluginHandle *)LeftPanel->GetPluginHandle())->InternalHandle;
          if (hPlugin==hInternal)
          {
            LeftPanel->SetPluginCommand(Command,Param);
            Processed=TRUE;
          }
        }
        if (RightPanel!=NULL && RightPanel->GetMode()==PLUGIN_PANEL)
        {
          hInternal=((struct PluginHandle *)RightPanel->GetPluginHandle())->InternalHandle;
          if (hPlugin==hInternal)
          {
            RightPanel->SetPluginCommand(Command,Param);
            Processed=TRUE;
          }
        }
        return(Processed);
      }
    case FCTL_GETCMDLINE:
      CtrlObject->CmdLine->GetString((char *)Param,1024);
      return(TRUE);
    case FCTL_SETCMDLINE:
    case FCTL_INSERTCMDLINE:
      if (Command==FCTL_SETCMDLINE)
        CtrlObject->CmdLine->SetString(NullToEmpty((char *)Param));
      else
        CtrlObject->CmdLine->InsertString(NullToEmpty((char *)Param));
      CtrlObject->CmdLine->Redraw();
      return(TRUE);
    case FCTL_SETCMDLINEPOS:
      CtrlObject->CmdLine->SetCurPos(*(int *)Param);
      CtrlObject->CmdLine->Redraw();
      return(TRUE);
    case FCTL_GETCMDLINEPOS:
      *(int *)Param=CtrlObject->CmdLine->GetCurPos();
      return(TRUE);
    case FCTL_GETCMDLINESELECTEDTEXT:
      CtrlObject->CmdLine->GetSelString((char *)Param,1024);
      return TRUE;
    case FCTL_GETCMDLINESELECTION:
    {
      CmdLineSelect *sel=(CmdLineSelect*)Param;
      if(sel)
      {
        CtrlObject->CmdLine->GetSelection(sel->SelStart,sel->SelEnd);
        return TRUE;
      }
      break;
    }
    case FCTL_SETCMDLINESELECTION:
    {
      CmdLineSelect *sel=(CmdLineSelect*)Param;
      if(sel)
      {
        CtrlObject->CmdLine->Select(sel->SelStart,sel->SelEnd);
        CtrlObject->CmdLine->Redraw();
        return TRUE;
      }
      break;
    }
    case FCTL_SETUSERSCREEN:
      if (CtrlObject->Cp()->LeftPanel==NULL || CtrlObject->Cp()->RightPanel==NULL)
        return(FALSE);
      KeepUserScreen++;
      CtrlObject->Cp()->LeftPanel->ProcessingPluginCommand++;
      CtrlObject->Cp()->RightPanel->ProcessingPluginCommand++;
      ScrBuf.FillBuf();
      SaveScreen SaveScr;
      {
        RedrawDesktop Redraw;
        CtrlObject->CmdLine->Hide();
        SaveScr.RestoreArea(FALSE);
      }
      KeepUserScreen--;
      CtrlObject->Cp()->LeftPanel->ProcessingPluginCommand--;
      CtrlObject->Cp()->RightPanel->ProcessingPluginCommand--;
      return(TRUE);
  }
  return(FALSE);
}


HANDLE WINAPI FarSaveScreen(int X1,int Y1,int X2,int Y2)
{
  if (FrameManager->ManagerIsDown())
    return NULL;

  if (X2==-1)
    X2=ScrX;
  if (Y2==-1)
    Y2=ScrY;
  return((HANDLE)(new SaveScreen(X1,Y1,X2,Y2,FALSE)));
}


void WINAPI FarRestoreScreen(HANDLE hScreen)
{
  if (FrameManager->ManagerIsDown())
    return;
  if (hScreen==NULL)
    ScrBuf.FillBuf();
  if (hScreen)
    delete (SaveScreen *)hScreen;
}


static void PR_FarGetDirListMsg(void)
{
  Message(MSG_DOWN,0,"",MSG(MPreparingList));
}

int WINAPI FarGetDirList(const char *Dir,struct PluginPanelItem **pPanelItem,
                  int *pItemsNumber)
{
  if (FrameManager->ManagerIsDown())
    return 0;

  PluginPanelItem *ItemsList=NULL;
  int ItemsNumber=0;
  SaveScreen SaveScr;
  clock_t StartTime=clock();
  int MsgOut=0;

  *pItemsNumber=0;
  *pPanelItem=NULL;

  ScanTree ScTree(FALSE);
  WIN32_FIND_DATA FindData;
  char FullName[NM],DirName[NM];
//  ConvertNameToFull(Dir,DirName, sizeof(DirName));
  if (ConvertNameToFull(Dir,DirName, sizeof(DirName)) >= sizeof(DirName)){
    return FALSE;
  }
  ScTree.SetFindPath(DirName,"*.*");
  *PointToName(DirName)=0;
  int DirLength=strlen(DirName);
  while (ScTree.GetNextName(&FindData,FullName, sizeof (FullName)-1))
  {
    if ((ItemsNumber & 31)==0)
    {
      if (CheckForEsc())
      {
        if(ItemsList)
          free(ItemsList);
        SetPreRedrawFunc(NULL);
        return(FALSE);
      }
      if (!MsgOut && clock()-StartTime > 500)
      {
        SetCursorType(FALSE,0);
        SetPreRedrawFunc(PR_FarGetDirListMsg);
        PR_FarGetDirListMsg();
        MsgOut=1;
      }
      ItemsList=(PluginPanelItem *)realloc(ItemsList,sizeof(*ItemsList)*(ItemsNumber+32+1));
      if (ItemsList==NULL)
      {
        *pItemsNumber=0;
        SetPreRedrawFunc(NULL);
        return(FALSE);
      }
    }
    memset(&ItemsList[ItemsNumber],0,sizeof(*ItemsList));
    ItemsList[ItemsNumber].FindData=FindData;
    strcpy(ItemsList[ItemsNumber].FindData.cFileName,FullName+DirLength);
    ItemsNumber++;
  }
  SetPreRedrawFunc(NULL);
  *pPanelItem=ItemsList;
  *pItemsNumber=ItemsNumber;
  return(TRUE);
}


static struct PluginPanelItem *PluginDirList;
static int DirListItemsNumber;
static char PluginSearchPath[NM*16];
static int StopSearch;
static HANDLE hDirListPlugin;
static int PluginSearchMsgOut;
static struct
{
  PluginPanelItem *Addr;
  int ItemsNumber;
} DirListNumbers[16];

static void FarGetPluginDirListMsg(char *Name,DWORD Flags)
{
  Message(Flags,0,"",MSG(MPreparingList),Name);
  PreRedrawParam.Flags=Flags;
  PreRedrawParam.Param1=Name;
}

static void PR_FarGetPluginDirListMsg(void)
{
  FarGetPluginDirListMsg((char *)PreRedrawParam.Param1,PreRedrawParam.Flags&(~MSG_KEEPBACKGROUND));
}

int WINAPI FarGetPluginDirList(int PluginNumber,HANDLE hPlugin,
                  const char *Dir,struct PluginPanelItem **pPanelItem,
                  int *pItemsNumber)
{
  if (FrameManager->ManagerIsDown())
    return FALSE;

  {
    if (strcmp(Dir,".")==0 || strcmp(Dir,"..")==0)
      return(FALSE);

    static struct PluginHandle DirListPlugin;

    // А не хочет ли плагин посмотреть на текущую панель?
    if (hPlugin==INVALID_HANDLE_VALUE)
    {
      /* $ 30.11.2001 DJ
         А плагиновая ли это панель?
      */
      DWORD Handle = (DWORD) CtrlObject->Cp()->ActivePanel->GetPluginHandle();
      if (!Handle || Handle == 0xffffffff)
        return FALSE;

      DirListPlugin=*((struct PluginHandle *)Handle);
      /* DJ $ */
    }
    else
    {
      DirListPlugin.PluginNumber=PluginNumber;
      DirListPlugin.InternalHandle=hPlugin;
    }

    SaveScreen SaveScr;

    SetPreRedrawFunc(NULL);
    {
      char DirName[512];
      strncpy(DirName,Dir,sizeof(DirName)-1);
      TruncStr(DirName,30);
      CenterStr(DirName,DirName,30);
      SetCursorType(FALSE,0);

      SetPreRedrawFunc(PR_FarGetPluginDirListMsg);
      FarGetPluginDirListMsg(DirName,0);
      PluginSearchMsgOut=FALSE;

      hDirListPlugin=(HANDLE)&DirListPlugin;
      StopSearch=FALSE;
      *pItemsNumber=DirListItemsNumber=0;
      *pPanelItem=PluginDirList=NULL;
      struct OpenPluginInfo Info;
      CtrlObject->Plugins.GetOpenPluginInfo(hDirListPlugin,&Info);
      char PrevDir[NM];
      strcpy(PrevDir,Info.CurDir);
      if (CtrlObject->Plugins.SetDirectory(hDirListPlugin,Dir,OPM_FIND))
      {
        strcpy(PluginSearchPath,Dir);
        strcat(PluginSearchPath,"\x1");
        ScanPluginDir();
        *pPanelItem=PluginDirList;
        *pItemsNumber=DirListItemsNumber;
        CtrlObject->Plugins.SetDirectory(hDirListPlugin,"..",OPM_FIND);
        PluginPanelItem *PanelData=NULL;
        int ItemCount=0;
        if (CtrlObject->Plugins.GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND))
          CtrlObject->Plugins.FreeFindData(hDirListPlugin,PanelData,ItemCount);
        struct OpenPluginInfo NewInfo;
        CtrlObject->Plugins.GetOpenPluginInfo(hDirListPlugin,&NewInfo);
        if (LocalStricmp(PrevDir,NewInfo.CurDir)!=0)
          CtrlObject->Plugins.SetDirectory(hDirListPlugin,PrevDir,OPM_FIND);
      }
    }
    SetPreRedrawFunc(NULL);
  }

  if (!StopSearch)
    for (int I=0;I<sizeof(DirListNumbers)/sizeof(DirListNumbers[0]);I++)
      if (DirListNumbers[I].Addr==NULL)
      {
        DirListNumbers[I].Addr=*pPanelItem;
        DirListNumbers[I].ItemsNumber=*pItemsNumber;
        break;
      }
  return(!StopSearch);
}

/* $ 30.11.2001 DJ
   вытащим в функцию общий код для копирования айтема в ScanPluginDir()
*/

static void CopyPluginDirItem (PluginPanelItem *CurPanelItem)
{
  char FullName[2*NM+1];
  sprintf(FullName,"%.*s%.*s",NM,PluginSearchPath,NM,CurPanelItem->FindData.cFileName);
  for (int I=0;FullName[I]!=0;I++)
    if (FullName[I]=='\x1')
      FullName[I]='\\';
  PluginPanelItem *DestItem=PluginDirList+DirListItemsNumber;
  *DestItem=*CurPanelItem;
  if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
  {
    DWORD Size=*(DWORD *)CurPanelItem->UserData;
    /* $ 13.07.2000 SVS
       вместо new будем использовать malloc
    */
    DestItem->UserData=(DWORD)malloc(Size);
    /* SVS $*/
    memcpy((void *)DestItem->UserData,(void *)CurPanelItem->UserData,Size);
  }

  strncpy(DestItem->FindData.cFileName,FullName,sizeof(DestItem->FindData.cFileName)-1);
  DirListItemsNumber++;
}

/* DJ $ */


void ScanPluginDir()
{
  int I;
  PluginPanelItem *PanelData=NULL;
  int ItemCount=0;
  if (CheckForEsc())
    StopSearch=TRUE;

  char DirName[NM];
  strncpy(DirName,PluginSearchPath,sizeof(DirName)-1);
  DirName[sizeof(DirName)-1]=0;
  for (I=0;DirName[I]!=0;I++)
    if (DirName[I]=='\x1')
      DirName[I]=DirName[I+1]==0 ? 0:'\\';
  TruncStr(DirName,30);
  CenterStr(DirName,DirName,30);

  FarGetPluginDirListMsg(DirName,MSG_KEEPBACKGROUND);

  if (StopSearch || !CtrlObject->Plugins.GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND))
    return;
  struct PluginPanelItem *NewList=(struct PluginPanelItem *)realloc(PluginDirList,1+sizeof(*PluginDirList)*(DirListItemsNumber+ItemCount));
  if (NewList==NULL)
  {
    StopSearch=TRUE;
    return;
  }
  PluginDirList=NewList;
  for (I=0;I<ItemCount && !StopSearch;I++)
  {
    PluginPanelItem *CurPanelItem=PanelData+I;
    /* $ 30.11.2001 DJ
       вытащим копирование айтема в функцию
    */
    if ((CurPanelItem->FindData.dwFileAttributes & FA_DIREC)==0)
      CopyPluginDirItem (CurPanelItem);
    /* DJ $ */
  }
  for (I=0;I<ItemCount && !StopSearch;I++)
  {
    PluginPanelItem *CurPanelItem=PanelData+I;
    if ((CurPanelItem->FindData.dwFileAttributes & FA_DIREC) &&
        strcmp(CurPanelItem->FindData.cFileName,".")!=0 &&
        strcmp(CurPanelItem->FindData.cFileName,"..")!=0)

    {
      struct PluginPanelItem *NewList=(struct PluginPanelItem *)realloc(PluginDirList,sizeof(*PluginDirList)*(DirListItemsNumber+1));
      if (NewList==NULL)
      {
        StopSearch=TRUE;
        return;
      }
      PluginDirList=NewList;
      /* $ 30.11.2001 DJ
         используем общую функцию для копирования FindData (не забываем
         обработать PPIF_USERDATA)
      */
      CopyPluginDirItem (CurPanelItem);
      /* DJ $ */
      if (CtrlObject->Plugins.SetDirectory(hDirListPlugin,CurPanelItem->FindData.cFileName,OPM_FIND))
      {
        strcat(PluginSearchPath,CurPanelItem->FindData.cFileName);
        strcat(PluginSearchPath,"\x1");
        if (strlen(PluginSearchPath)<sizeof(PluginSearchPath)-NM)
          ScanPluginDir();
        *strrchr(PluginSearchPath,'\x1')=0;
        char *NamePtr=strrchr(PluginSearchPath,'\x1');
        if (NamePtr!=NULL)
          *(NamePtr+1)=0;
        else
          *PluginSearchPath=0;
        if (!CtrlObject->Plugins.SetDirectory(hDirListPlugin,"..",OPM_FIND))
        {
          StopSearch=TRUE;
          break;
        }
      }
    }
  }
  CtrlObject->Plugins.FreeFindData(hDirListPlugin,PanelData,ItemCount);
}


void WINAPI FarFreeDirList(const struct PluginPanelItem *PanelItem)
{
  if (PanelItem==NULL)
    return;
  int ItemsNumber=0;
  int I;
  for (I=0;I<sizeof(DirListNumbers)/sizeof(DirListNumbers[0]);I++)
    if (DirListNumbers[I].Addr==PanelItem)
    {
      DirListNumbers[I].Addr=NULL;
      ItemsNumber=DirListNumbers[I].ItemsNumber;
      break;
    }

  for (I=0;I<ItemsNumber;I++)
  {
    const PluginPanelItem *CurPanelItem=PanelItem+I;
    if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
      /* $ 13.07.2000 SVS
        для запроса использовали malloc
      */
      free((void *)CurPanelItem->UserData);
      /* SVS $*/
  }
  /* $ 13.07.2000 SVS
    для запроса использовали realloc
  */
  // Что ЭТО ЗА ИЗВРАТ??????????
  //free(static_cast<void*>(const_cast<PluginPanelItem *>(PanelItem)));
  free((void*)PanelItem);
  /* SVS $*/
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
int WINAPI FarViewer(const char *FileName,const char *Title,
                     int X1,int Y1,int X2, int Y2,DWORD Flags)
{
  if (FrameManager->ManagerIsDown())
    return FALSE;
  if (X2==-1)
    X2=ScrX;
  if (Y2==-1)
    Y2=ScrY;
  char OldTitle[512];
  GetConsoleTitle(OldTitle,sizeof(OldTitle));
  /* $ 09.09.2001 IS */
  int DisableHistory=(Flags & VF_DISABLEHISTORY)?TRUE:FALSE;
  /* IS $ */
  if (Flags & VF_NONMODAL)
  {
    /* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
    FileViewer *Viewer=new FileViewer(FileName,TRUE,DisableHistory,Title,X1,Y1,X2,Y2);
    /* IS $ */
    if(!Viewer)
      return FALSE;
    if (Flags & VF_DELETEONCLOSE)
      Viewer->SetTempViewName(FileName);
    /* $ 12.05.2001 DJ */
    Viewer->SetEnableF6 ((Flags & VF_ENABLE_F6) != 0);
    /* DJ $ */
    FrameManager->ExecuteNonModal();
  }
  else
  {
    /* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
    FileViewer Viewer (FileName,FALSE,DisableHistory,Title,X1,Y1,X2,Y2);
    /* IS $ */
    /* $ 28.05.2001 По умолчанию Вьюер, поэтому нужно здесь признак выставиль явно */
    Viewer.SetDynamicallyBorn(false);
    FrameManager->ExecuteModal();
    if (Flags & VF_DELETEONCLOSE)
      Viewer.SetTempViewName(FileName);
    /* $ 12.05.2001 DJ */
    Viewer.SetEnableF6 ((Flags & VF_ENABLE_F6) != 0);
    /* DJ $ */
    SetConsoleTitle(OldTitle);
    if (!Viewer.GetExitCode()){
      return FALSE;
    }
  }
  return(TRUE);
}


int WINAPI FarEditor(const char *FileName,const char *Title,
                     int X1,int Y1,int X2,
                     int Y2,DWORD Flags,int StartLine,int StartChar)
{
  if (FrameManager->ManagerIsDown())
    return EEC_OPEN_ERROR;
  if (X2==-1)
    X2=ScrX;
  if (Y2==-1)
    Y2=ScrY;
  char OldTitle[512];
  GetConsoleTitle(OldTitle,sizeof(OldTitle));
  /* $ 12.07.2000 IS
   Проверка флагов редактора (раньше они игнорировались) и открытие
   немодального редактора, если есть соответствующий флаг
  */
  int ExitCode;
  /* $ 21.03.2001 VVM
    + Обработка флага EF_CREATENEW */
  int CreateNew = (Flags & EF_CREATENEW)?TRUE:FALSE;
  /* VVM $ */
  /* $ 09.09.2001 IS */
  int DisableHistory=(Flags & EF_DISABLEHISTORY)?TRUE:FALSE;
  int DeleteOnClose=(Flags & EF_DELETEONCLOSE)?TRUE:FALSE;
  /* IS $ */
  int OpMode=FEOPMODE_QUERY;
#if 0
  if((Flags&(EF_USEEXISTING|EF_BREAKIFOPEN)) != (EF_USEEXISTING|EF_BREAKIFOPEN))
    OpMode=(Flags&EF_USEEXISTING)?FEOPMODE_USEEXISTING:FEOPMODE_BREAKIFOPEN;
#endif

  if (Flags & EF_NONMODAL)
  {
    ExitCode=FALSE;
    /* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
    FileEditor *Editor=new FileEditor(FileName,CreateNew,TRUE,
                                      StartLine,StartChar,Title,
                                      X1,Y1,X2,Y2,DisableHistory,
                                      DeleteOnClose,OpMode);
    /* IS $ */
    if (Editor)
    {
      /* $ 12.05.2001 DJ */
      Editor->SetEnableF6 ((Flags & EF_ENABLE_F6) != 0);
      /* DJ $ */
      FrameManager->ExecuteNonModal();
      ExitCode=TRUE;
    }
  }
  else
  {
   /* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
   FileEditor Editor(FileName,CreateNew,FALSE,
                     StartLine,StartChar,Title,
                     X1,Y1,X2,Y2,DisableHistory,
                     DeleteOnClose,OpMode);
   /* IS $ */
   Editor.SetDynamicallyBorn(false);
   /* $ 12.05.2001 DJ */
   Editor.SetEnableF6 ((Flags & EF_ENABLE_F6) != 0);
   /* DJ $ */
   SetConsoleTitle(OldTitle);
   FrameManager->ExecuteModal();
   ExitCode = Editor.GetExitCode();
   if (ExitCode && ExitCode != XC_LOADING_INTERRUPTED)
   {
#if 0
     if(OpMode==FEOPMODE_BREAKIFOPEN && ExitCode==XC_QUIT)
       ExitCode = XC_OPEN_ERROR;
     else
#endif
       ExitCode = Editor.IsFileChanged()?XC_MODIFIED:XC_NOT_MODIFIED;
   }
  }
  return ExitCode;
  /* IS $ */
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


int WINAPI FarCmpName(const char *pattern,const char *string,int skippath)
{
  return(CmpName(pattern,string,skippath));
}


int WINAPI FarCharTable(int Command,char *Buffer,int BufferSize)
{
  if (FrameManager->ManagerIsDown())
    return -1;
  if (Command==FCT_DETECT)
  {
    char DataFileName[NM];
    FILE *DataFile;
    /* $ 19.06.2001
       - Баг: не работало автоопределение.
         Эх, Валя, зачем же ты return -1 закомментарил в 268??
    */
    if (!FarMkTempEx(DataFileName) || (DataFile=fopen(DataFileName,"w+b"))==NULL)
      return(-1);
    /* IS $ */
    fwrite(Buffer,1,BufferSize,DataFile);
    fseek(DataFile,0,SEEK_SET);
    CharTableSet TableSet;
    int TableNum;
    int DetectCode=DetectTable(DataFile,&TableSet,TableNum);
    fclose(DataFile);
    remove(DataFileName);
    return(DetectCode ? TableNum-1:-1);
  }
  if (BufferSize!=sizeof(CharTableSet))
    return(-1);
  /* $ 07.08.2001 IS
       При неудаче заполним структуру данными для OEM
  */
  CharTableSet *CTS=reinterpret_cast<CharTableSet*>(Buffer);
  /* $ 17.03.2002 IS По возможности используем значение TableName */
  if (!PrepareTable(CTS,Command,TRUE))
  /* IS $ */
  {
    for(unsigned int i=0;i<256;++i)
    {
      CTS->EncodeTable[i]=CTS->DecodeTable[i]=i;
      CTS->UpperTable[i]=LocalUpper(i);
      CTS->LowerTable[i]=LocalLower(i);
    }
    strcpy(CTS->TableName,MSG(MGetTableNormalText));
    return(-1);
  }
  /* IS $ */
  return(Command);
}


void WINAPI FarText(int X,int Y,int Color,const char *Str)
{
  if (FrameManager->ManagerIsDown())
    return;
  if (Str==NULL)
  {
    int PrevLockCount=ScrBuf.GetLockCount();
    ScrBuf.SetLockCount(0);
    ScrBuf.Flush();
    ScrBuf.SetLockCount(PrevLockCount);
  }
  else
  {
    /* $ 22.08.2000 SVS
       Исключаем ненужные вызовы из FarText.
    */
    Text(X,Y,Color,Str);
    /* SVS $ */
  }
}


int WINAPI FarEditorControl(int Command,void *Param)
{
  if (CtrlObject->Plugins.CurEditor==NULL || FrameManager->ManagerIsDown())
    return(0);
#if defined(SYSLOG_KEYMACRO) || defined(SYSLOG_ECTL)
  {
#if defined(SYSLOG_KEYMACRO)
    if(Command == ECTL_READINPUT || Command == ECTL_PROCESSINPUT)
    {
#endif
      _KEYMACRO(CleverSysLog SL("FarEditorControl()"));
      _KEYMACRO(SysLog("Command=%s (%d) Param=0x%08X Macro.IsExecuting()=%d",_ECTL_ToName(Command),Command,Param,CtrlObject->Macro.IsExecuting()));
#if defined(SYSLOG_KEYMACRO)
    }
#endif
    return(CtrlObject->Plugins.CurEditor->EditorControl(Command,Param));
  }
#else
  return(CtrlObject->Plugins.CurEditor->EditorControl(Command,Param));
#endif
}

/* $ 27.09.2000 SVS
  Управление вьювером
*/
int WINAPI FarViewerControl(int Command,void *Param)
{
  if (CtrlObject->Plugins.CurViewer==NULL || FrameManager->ManagerIsDown())
    return(0);
  return(CtrlObject->Plugins.CurViewer->ViewerControl(Command,Param));
}
/* SVS $ */
