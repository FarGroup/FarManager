#ifndef __PLUGIN_HPP__
#define __PLUGIN_HPP__

#ifndef FAR_USE_INTERNALS
#define FAR_USE_INTERNALS
#endif // END FAR_USE_INTERNALS
/*
  PLUGIN.HPP

  Plugin API for FAR Manager 1.70

  Copyright (c) 1996-2000 Eugene Roshal
  Copyrigth (c) 2000-2001 [ FAR group ]
*/
/* Revision: 1.146 26.09.2001 $ */

#ifdef FAR_USE_INTERNALS
/*
ВНИМАНИЕ!
В этом файле писать все изменения только в в этом блоке!!!!

Modify:
  26.09.2001 SVS 1.146
    ! AddEndSlash имеет возвращаемый тип BOOL
  24.09.2001 SVS 1.145
    + FSF.GetRepasePointInfo
  20.09.2001 SVS 1.144
    ! У функции FSF.FarInputRecordToKey параметр имеет сущность "const"!
  15.09.2001 tran 1.143
    + VE_READ, VE_CLOSE
  15.09.2001 tran 1.142
    + ACTL_GETFARHWND
  12.09.2001 SVS
    + FSF.ConvertNameToReal
  09.09.2001 IS
    + VF_DISABLEHISTORY, EF_DISABLEHISTORY
  31.08.2001 IS
    ! Все поля структуры CharTableSet кроме TableName теперь unsigned char.
  17.08.2001 VVM
    + PluginPanelItem.CRC32
  15.08.2001 SVS
    + DN_MOUSE, DM_SETNOTIFYMOUSEEVENT
  13.08.2001 SKV
    + FCTL_GETCMDLINESELECTION, FCTL_SETCMDLINESELECTION, struct CmdLineSelect.
  08.08.2001 SVS
    + DM_GETITEMPOSITION
  07.08.2001 IS
    + ESPT_SETTABLE
    ! FARAPICHARTABLE - второй параметр теперь не const, потому что он может
      изменяться в FarCharTable.
  07.08.2001 SVS
    + DN_RESIZECONSOLE
  01.08.2001 SVS
    ! Флаги FMENU_CUSTOMNAME, FDLG_CUSTOMNAME, FMSG_CUSTOMNAME в морг
  31.07.2001 IS
    + Флаги FMENU_CUSTOMNAME, FDLG_CUSTOMNAME, FMSG_CUSTOMNAME
  31.07.2001 SVS
    + Маркеры про FAR_USE_INTERNALS. Просьба не убирать! Это для скрипта,
      который будет генерить одноименный файл для дистрибутива. Если нужно
      исключить что либо от внешнего взгляда - заключить вот в такие "скобки":
      1 # ifdef FAR_USE_INTERNALS
      2   то, что должно быть скрыто
      3 # else // ELSE FAR_USE_INTERNALS
      4   замена!
      5 # endif // END FAR_USE_INTERNALS
  31.07.2001 IS
    + Внедрение const (FARAPIGETMSG)
  27.07.2001 SVS
    + DM_ALLKEYMODE - для нужд MacroBrowse (пока только для него :-)
  16.07.2001 SVS
    + FMENU_USEEXT & MENUITEMFLAGS & FarMenuItemEx
    + DM_SETHISTORY - управление наличием истории у DI_EDIT & DI_FIXEDIT
  11.07.2001 OT
    + Новое "техническое" сообщения диалогу - DM_KILLSAVESCREEN
  30.06.2001 KM
    ! Языковое уточнение: LIFIND_NOPATTER -> LIFIND_NOPATTERN
    + Новая структура FarListPos.
  29.06.2001 SVS
    ! Уточнение FarListFind.
    + LIFIND_NOPATTER - точное (без учета регистра букв) соответствие при
      поиске в списке
  26.06.2001 SKV
    + ACTL_COMMIT
  26.06.2001 SVS
    ! Перенесем DM_GETDROPDOWNOPENED и DM_SETDROPDOWNOPENED в "обычное"
      место и дадим ход в публику :-)
  25.06.2001 IS
    ! Внедрение const, чтобы было как можно меньше отличий от "официального"
     plugin.hpp
  23.06.2001 KM
    + DM_GETDROPDOWNOPENED - определить, открыт ли в диалоге комбобокс или хистори.
    + DM_SETDROPDOWNOPENED - открыть или закрыть программным путём комбобокс или хистори.
  21.06.2001 SVS
    ! ACTL_POSTSEQUENCEKEY  -> ACTL_POSTKEYSEQUENCE - (с точки зрения eng)
    ! SKFLAGS_DISABLEOUTPUT -> KSFLAGS_DISABLEOUTPUT
    ! SequenceKey           -> KeySequence
  20.06.2001 SVS
    ! ACTL_PROCESSSEQUENCEKEY -> ACTL_POSTSEQUENCEKEY
    ! SKEY_NOTMACROS -> SKFLAGS_DISABLEOUTPUT
  19.06.2001 SVS
    + DN_DRAGGED
  14.06.2001 SVS
    + Дополнение к ACTL_*WINDOW* - WTYPE_* - типы окон
      2AT: если что-то не так - изменяй.
  06.06.2001 SVS
    + EditorBookMark, ECTL_GETBOOKMARK
    + EditorInfo.BookMarkCount - дабы не вызывать несколько раз ECTL_GETBOOKMARK.
  05.06.2001 tran
    + ACTL_GETWINDOWCOUNT,ACTL_GETWINDOWINFO,ACTL_SETCURRENTWINDOW
    + struct WindowInfo
  04.06.2001 SVS
    ! выкинут LIF_PTRDATA - изжил себя как класс :-)
    ! Соответственно изменилась структура FarListItem
  03.06.2001 KM
    + Два новых сообщения:
      DM_LISTSETTITLE
      DM_LISTGETTITLE
      для установки/получения заголовков в DI_LISTBOX.
    + Вернулся флаг DIF_LISTAUTOHIGHLIGHT.
  03.06.2001 SVS
    ! Небольшое уточнение структуры FarListItemData (до 16 байт :-)
    + Пара макросов для листа
  03.06.2001 SVS
    + DM_LISTGETDATA, DM_LISTSETDATA, FarListItemData
  30.05.2001 SVS
    + MKLINKOP, FARSTDMKLINK
  29.05.2001 tran
    + макрос - MAKEFARVERSION
  21.05.2001 DJ
    + FDLG_NONMODAL
  21.05.2001 SVS
    + DM_RESIZEDIALOG
    + DM_SETITEMPOSITION
  18.05.2001 SVS
    + DM_LISTINSERT, DM_LISTINFO, DM_LISTFINDSTRING
    + DM_GETCHECK, DM_SETCHECK, DM_SET3STATE
    + BSTATE_*
    + Структуры FarListInsert, FarListInfo, FarListFind
  17.05.2001 SVS
    + DM_LISTUPDATE
    + FMENU_SHOWNOBOX (ЭТО НЕ ПУБЛИКУЕТСЯ, ЭТО ДЛЯ ВНУТРЕННЕГО ИСПОЛЬЗОВАНИЯ!)
    + структура FarListUpdate для DM_LISTUPDATE, по сути то же самое что и
      FarList - т.с. для лучшего понимания :-)
  15.05.2001 KM
    ! Убран флаг DIF_LISTHIGHLIGHT, так как его функцию
      уже выполнял DIF_LISTNOAMPERSAND, только наоборот.
  14.05.2001 SVS
    ! FDLG_SMALLDILAOG -> FDLG_SMALLDIALOG
  13.05.2001 SVS
    + DIF_LISTWRAPMODE, DIF_LISTHIGHLIGHT
    + DM_LISTADDSTR
  12.05.2001 DJ
    + VF_ENABLE_F6, EF_ENABLE_F6
  08.05.2001 SVS
    + FDLG_* - флаги для DialogEx
  07.05.2001 SVS
    + DM_LISTADD, DM_LISTDELETE, DM_LISTGET, DM_LISTSORT, DM_LISTGETCURPOS,
      DM_LISTSETCURPOS
    + DIF_LISTNOBOX - рамку для DI_LISTBOX не рисовать
    + struct FarListDelete
    + Макросы DlgList_*, DlgItem_*, DlgEdit_*, Dlg_* - типа windowsx.h ;-)
      Для затравки - глядишь и продолжат начинания :-)))
  04.05.2001 SVS
    ! Наконец то дошли руки до DI_LISTBOX ;-) - новый член FarDialogItem.ListPos
  24.04.2001 SVS
    + PanelInfo.Flags, флаги PANELINFOFLAGS.
  22.04.2001 SVS
    + EJECT_LOAD_MEDIA - работает только в NT/2000
  12.04.2001 SVS
    + DM_ADDHISTORY - добавить строку в историю
    + DIF_MANUALADDHISTORY - добавлять в историю только "ручками"
  03.04.2001 IS
    + ESPT_AUTOINDENT, ESPT_CURSORBEYONDEOL, ESPT_CHARCODEBASE
  26.03.2001 SVS
    + FHELP_USECONTENTS - если не найден требует топик, то отобразить "Contents"
  24.03.2001 tran
    + qsortex
  21.03.2001 VVM
    + Флаг EF_CREATENEW для редактора - создать новый файл (аналог SHIFT+F4)
  20.03.2001 tran 1.89
    + FarRecursiveSearch - добавлен void *param
  19.03.2001 SVS
    ! DN_CLOSE=DM_CLOSE, DN_KEY=DM_KEY - для наглядности. :-)
  16.02.2001 IS
    + Добавлена проверка правильности выравнивания на основе известного
      размера PluginPanelItem - он должен быть равным 366. Если это не так, то
      если определено STRICT, то компилирование вообще прекратится, иначе -
      будет выдан warning
  16.02.2001 IS
    + команда ECTL_SETPARAM - изменить некую настройку редактора
    + EDITOR_SETPARAMETER_TYPES - тип настройки
    + структура EditorSetParameter - информация о типе настройки и все
      остальном
  13.02.2001 SVS
    ! В связи с введением DIF_VAREDIT для DI_COMBOBOX изменена структура
      FarListItem и добавлен флаг LIF_PTRDATA
    ! Изменено значение флага LIF_DISABLE
    ! Изменены имена полей структуры FarDialogItemData - для "похожести"
      на диалог.
  11.02.2001 SVS
    ! FarDialogItem - изменения, касаемые Ptr
    + DIF_VAREDIT - флаг, указывающий на то, что будет использоваться
      FarDialogItem.Ptr вместо FarDialogItem.Data
  11.02.2001 SVS
    ! Изменения в LISTITEMFLAGS - флаги переехали в старшее слово
  28.01.2001 SVS
    ! SequenceKey.Sequence НЕ! "валяются" VK_* - только KEY_*
    + FMSG_ALLINONE - в качестве Items передается указатель на
      строку, в которой разделители строк - символ '\n'
    + FMSG_MB_* - дополнительно показать кнопки (в Items можно не указывать)
  25.01.2001 SVS
    ! Тип SequenceKey.Sequence изменен на DWORD
    + SKEY_VK_KEYS - в SequenceKey.Sequence "валяются" VK_* вместо KEY_*
  23.01.2001 SVS
    + SKEY_NOTMACROS - не использовать бинденные клавиши в SequenceKey
    + ViewerInfo.LeftPos и ViewerInfo.Reserved3;
  21.01.2001 SVS
    + struct SequenceKey
    + ACTL_PROCESSSEQUENCEKEY
  21.01.2001 IS
    ! Для однообразия с редактором изменил пару названий:
      VCTL_SETPOS -> VCTL_SETPOSITION
      AnsiText -> AnsiMode
  19.01.2001 SVS
    ! перестановки в командах VIEWER_CONTROL_COMMANDS
    + некоторые структуры для Viewer API: ViewerSelect, ViewerSetPosition и
      перечисление VIEWER_SETPOS_FLAGS
  03.01.2001 SVS
    + DIF_HIDDEN - элемент не видим
    + DM_SHOWITEM показать/скрыть элемент
  25.12.2000 SVS
    ! ACTL_KEYMACRO поддерживает только 2 команды: MCMD_LOADALL, MCMD_SAVEALL
  23.12.2000 SVS
    + MCMD_PLAYSTRING - "проиграть" строку.
    + MACRO_* - области действия макросов
    ! ActlKeyMacro - уточнения содержимого стркутуры для команды MCMD_PLAYSTRING
    + MFLAGS_ - флаги макроса
  21.12.2000 SVS
    + ACTL_KEYMACRO
    + структура ActlKeyMacro (с зарезервированными полями :-)
    + MacroCommand: MCMD_LOADALL, MCMD_SAVEALL (на этом пока остановимся,
      остальное будет опосля)
  21.12.2000 SVS
    + DM_GETTEXTPTR, DM_SETTEXTPTR
  18.12.2000 SVS
    + FHELP_NOSHOWERROR
  14.12.2000 SVS
    + ACTL_EJECTMEDIA & struct ActlEjectMedia & EJECT_NO_MESSAGE
  08.12.2000 SVS 1.70
    ! Оригинальный номер ревизии получился - 1.70 ;-) - символично.
      Для DM_SETTEXT, DM_GETTEXT в Param2 передается структура
      FarDialogItemData.
  07.12.2000 SVS
    ! Изменена константа FARMANAGERVERSION. Остальное читайте в
      DIFF.DOC\00300.FAR_VERSION.txt
  04.12.2000 SVS
    + DIF_3STATE - 3-х уровневый CheckBox
    + ACTL_GETCOLOR - получить определенный цвет
    + ACTL_GETARRAYCOLOR - получить весь массив цветов
  04.11.2000 SVS
    + XLAT_SWITCHKEYBBEEP - выдать звуковой сигнал при переключении
      клавиатуры
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  26.10.2000 SVS
    ! DM_SETEDITPOS/DM_GETEDITPOS -> DM_SETCURSORPOS/DM_GETCURSORPOS
  25.10.2000 IS
    + Изменил имя параметра в MkTemp с Template на Prefix
  23.10.2000 SVS
    + DM_SETEDITPOS, DM_GETEDITPOS -
      позиционирование курсора в строках редактирования.
  20.10.2000 SVS
    ! ProcessName: Flags должен быть DWORD, а не int
  20.10.2000 SVS
    + DM_GETFOCUS - получить ID элемента имеющего фокус ввода
  09.10.2000 IS
    + Флаги для ProcessName (PN_*)
    + Указатель в FARSTANDARDFUNCTIONS на ProcessName;
  27.09.2000 SVS
    + VCTL_QUIT      - закрыть вьювер
    + VCTL_GETINFO   - получение информации о Viewer
    + VCTL_SETKEYBAR - функция установки KeyBar Labels во вьювере
  27.09.2000 skv
    + DeleteBuffer
  26.09.2000 SVS
    ! FARSTDKEYTOTEXT -> FARSTDKEYTOKEYNAME
  24.09.2000 SVS
    ! Чистка файла от комментариев - писать только в этом блоке (Modify)!!!
    ! FarKeyToText -> FarKeyToName
    + FarNameToKey
  21.09.2000 SVS
    + OPEN_FILEPANEL открыт из файловой панели.
    + Поле PluginInfo.SysID - системный идентификатор плагина
  20.09.2000 SVS
    ! удалил FolderPresent (блин, совсем крышу сорвало :-(
  19.09.2000 SVS
    + выравнивание на 2 байта
    + функция FSF.FolderPresent - "сужествует ли каталог"
  18.09.2000 SVS
    + DIF_READONLY - флаг для строк редактирования
      (пока! для строк редактирования).
  18.09.2000 SVS
    ! Функция DialogEx имеет 2 дополнительных параметра (Future)
    ! переделки в struct PluginStartupInfo!!!!
    ! FarRecurseSearch -> FarRecursiveSearch
    ! FRS_RECURSE -> FRS_RECUR
  14.09.2000 SVS
    ! Ошибка в названии XLAT_SWITCHKEYBLAYOUT.
    + FSF.MkTemp
    + Флаг DIF_LISTNOAMPERSAND. По умолчанию для DI_LISTBOX
      выставляется флаг MENU_SHOWAMPERSAND. Этот флаг подавляет такое
      повдедение
  13.09.2000 skv
    + EEREDRAW_XXXXX defines
  12.09.2000 SVS
    + Флаги FHELP_* для функции ShowHelp
    ! FSF.ShowHelp возвращает BOOL
  10.09.2000 SVS
    ! KeyToText возвращает BOOL, если нет такой клавиши.
  10.09.2000 SVS 1.46
    + typedef struct _CHAR_INFO    CHAR_INFO;
      На тот случай, если wincon.h не был загружен.
  10.09.2000 tran 1.45
    + FSF/FarRecurseSearch
  10.09.2000 SVS 1.44
    ! Наконец-то нашлось приемлемое имя для QWERTY -> Xlat.
    + DIF_NOFOCUS - элемент не получает фокуса ввода (клавиатурой)
    + CHAR_INFO *VBuf; в элементах диалога
    + DIF_SELECTONENTRY - выделение Edit при получении фокуса
  08.09.2000 VVM
    + FCTL_SETSORTMODE, FCTL_SETANOTHERSORTMODE
      FCTL_SETSORTORDER, FCTL_SETANOTHERSORTORDER
      Смена сортировки на панели
  08.09.2000 SVS
    ! QWERTY -> Transliterate
    ! QWED_SWITCHKEYBLAYER -> EDTR_SWITCHKEYBLAYER
  08.09.2000 SVS
    + FARMANAGERVERSION
    ! FarStandardFunctions.Reserved* -> FarStandardFunctions.Reserved[10];
  07.09.2000 skv
    + ECTL_PROCESSKEY
  07.09.2000 VVM 1.39
    + PF_FULLCMDLINE флаг для передачи плагину всей строки вместе с
      префиксом
  07.09.2000 SVS 1.38
    + FSF.bsearch
    + FSF.GetFileOwner
    + FSF.GetNumberOfLinks;
  05.09.2000 SVS 1.37
    + QWERTY - перекодировщик - StandardFunctions.EDQwerty
  01.09.2000 SVS
    + конструкция (с подачи MY)
      #ifndef _WINCON_
      typedef struct _INPUT_RECORD INPUT_RECORD;
      #endif
  31.08.2000 tran 1.35
    + FSF: int FarInputRecordToKey(INPUT_RECORD*r);
  31.08.2000 SVS
    ! изменение FSF-функций
      FSF.RemoveLeadingSpaces =FSF.LTrim
      FSF.RemoveTrailingSpaces=FSF.RTrim
      FSF.RemoveExternalSpaces=FSF.Trim
    + DM_ENABLE
    + Флаг DIF_DISABLE переводящий элемент диалога в состояние Disable
    + Флаг LIF_DISABLE переводящий элемент списка в состояние Disable
  30.08.2000 SVS
    ! Пал смертью храбрых флаг FMI_GETFARMSGID
    + DM_MOVEDIALOG - переместить диалог.
  29.08.2000 SVS
    ! Вот и глючек вылез с unsigned char во внутренней структуре DialogItem,
      и из-за этого uchar элемент DI_USERCONTROL не может быть > 255 :-((((((
  29.08.2000 SVS
    + Плагин может запросить "месаг" из FAR*.LNG, для этого
      небходимо к MsgId (в функции GetMsg)добавить флаг FMI_GETFARMSGID
  28.08.2000 SVS
    + SFS-функции аля Local*
    ! уточнение для FARSTDQSORT - явное указание __cdecl для функции сравнения
    ! не FarStandardFunctions._atoi64, но FarStandardFunctions.atoi64
    + FARSTDITOA64
  25.08.2000 SVS
    + DM_GETDLGRECT - получить координаты диалогового окна
    + DM_USER - эт для юзеровских месагов :-)
  25.08.2000 SVS
    ! Удалены из FSF функции:
      memset, memcpy, memmove, memcmp,
      strchr, strrchr, strstr, strtok, strpbrk
    + Флаг FIB_BUTTONS - в функции InputBox если нужно - показываем
      кнопки <Ok> & <Cancel>
  24.08.2000 SVS
    + ACTL_WAITKEY - ожидать определенную (или любую) клавишу
    + Элемент DI_USERCONTROL - отрисовкой занимается плагин.
  23.08.2000 SVS
    ! Уточнения категорий DMSG_* -> DM_ (месаг) & DN_ (нотифи)
    + DM_KEY        - послать/получить клавишу(ы)
    + DM_GETDLGDATA - взять данные диалога.
    + DM_SETDLGDATA - установить данные диалога.
    + DM_SHOWDIALOG - показать/спрятать диалог
    ! Все Flags приведены к одному виду -> DWORD.
      Модифицированы:
        * функции   FarMenuFn, FarMessageFn, FarShowHelp
        * структуры FarListItem, FarDialogItem
  22.08.2000 SVS
    ! DMSG_PAINT -> DMSG_DRAWDIALOG
    ! DMSG_DRAWITEM -> DMSG_DRAWDLGITEM
    ! DMSG_CHANGELIST -> DMSG_LISTCHANGE
  21.08.2000 SVS 1.23
    ! DMSG_CHANGEITEM -> DMSG_EDITCHANGE
    + DMSG_BTNCLICK
  18.08.2000 tran
    + Flags in ShowHelp
  12.08.2000 KM 1.22
    + DIF_MASKEDIT - новый флаг, реализующий функциональность ввода
      по маске в строках ввода.
    ! В структуре FarDialogItem новое поле, включенное в union, char *Mask
  17.08.2000 SVS
    ! struct FarListItems -> struct FarList, а то совсем запутался :-)
    + Сообщения диалога: DMSG_ENABLEREDRAW, DMSG_MOUSECLICK,
    + Флаг для DI_BUTTON - DIF_BTNNOCLOSE - "кнопка не для закрытия диалога"
  17.08.2000 SVS
    ! Изменение номера весрии :-)
  09.08.2000 SVS
    + FIB_NOUSELASTHISTORY - флаг для использовании пред значения из
      истории задается отдельно!!!
  09.08.2000 tran
    + #define CONSOLE_*
  04.08.2000 SVS
    + ECTL_SETKEYBAR - функция установки KeyBar Labels в редакторе
  04.08.2000 SVS
    + FarListItems.CountItems -> FarListItems.ItemsNumber
  03.08.2000 SVS
    + Функция от AT: GetMinFarVersion
  03.08.2000 SVS
    + ACTL_GETSYSWORDDIV получить строку с символами разделителями слов
  02.08.2000 SVS
    + Дополнения для KeyBarTitles:
        CtrlShiftTitles
        AltShiftTitles,
        CtrlAltTitles
    + Добавка в OpenPluginInfo для того, чтобы различить FAR <= 1.65 и > 1.65
  01.08.2000 SVS
    ! Функция ввода строки имеет один параметр для всех флагов
    ! дополнительный параметра у KeyToText - размер данных
    + Флаг DIF_USELASTHISTORY для строк ввода.
      если у строки ввода есть история то начальное значение брать первым
      из истории
    ! Полная переделка структуры списка и "оболочка" вокруг списка
    + флаги для FarListItem.Flags
      LIF_SELECTED, LIF_CHECKED, LIF_SEPARATOR
    + Сообщения для обработки диалога, имееющий место быть :-)
      DMSG_SETDLGITEM, DMSG_CHANGELIST
    ! Изменено наименование типа функции обработчика на универсальное
      FARDIALOGPROC -> FARWINDOWPROC
  28.07.2000 SVS
    + Введен новый элемент DI_LISTBOX (зарезервировано место)
    + Сообщения для обработки диалога, имееющий место быть :-)
        DMSG_INITDIALOG, DMSG_ENTERIDLE, DMSG_HELP, DMSG_PAINT,
        DMSG_SETREDRAW, DMSG_DRAWITEM, DMSG_GETDLGITEM, DMSG_KILLFOCUS,
        DMSG_GOTFOCUS, DMSG_SETFOCUS, DMSG_GETTEXTLENGTH, DMSG_GETTEXT,
        DMSG_CTLCOLORDIALOG, DMSG_CTLCOLORDLGITEM, DMSG_CTLCOLORDLGLIST,
        DMSG_SETTEXTLENGTH, DMSG_SETTEXT, DMSG_CHANGEITEM, DMSG_HOTKEY,
        DMSG_CLOSE,
  25.07.2000 SVS
    ! Некоторое упорядочение в FarStandardFunctions
    + Программое переключение FulScreen <-> Windowed (ACTL_CONSOLEMODE)
    + FSF-функция KeyToText
    ! WINAPI для сервисных дополнительных функций
    + Функция-диалог ввода тестовой строки InputBox
  23.07.2000 SVS
    + DialogEx, SendDlgMessage, DefDlgProc,
    ! WINAPI для сервисных дополнительных функций
  18.07.2000 SVS
    + Введен новый элемент: DI_COMBOBOX и флаг DIF_DROPDOWNLIST
      (для нередактируемого DI_COMBOBOX - пока не реализовано!)
  12.07.2000 IS
    + Флаги  редактора:
      EF_NONMODAL - открытие немодального редактора
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  10.07.2000 IS
    ! Некоторые изменения с учетом голого C (по совету SVS)
  07.07.2000 IS
    + Указатели на функции в FarStandardFunctions:
      atoi, _atoi64, itoa, RemoveLeadingSpaces, RemoveTrailingSpaces,
      RemoveExternalSpaces, TruncStr, TruncPathStr, QuoteSpaceOnly,
      PointToName, GetPathRoot, AddEndSlash
  06.07.2000 IS
    + Функция AdvControl (PluginStartupInfo)
    + Команда ACTL_GETFARVERSION для AdvControl
    + Указатель на структуру FarStandardFunctions в PluginStartupInfo - она
      содержит указатели на полезные функции. Плагин должен обязательно
      скопировать ее себе, если хочет использовать в дальнейшем.
    + Указатели на функции в FarStandardFunctions:
      Unquote, ExpandEnvironmentStr,
      sprintf, sscanf, qsort, memcpy, memmove, memcmp, strchr, strrchr, strstr,
      strtok, memset, strpbrk
  05.06.2000 SVS
    + DI_EDIT имеет флаг DIF_EDITEXPAND - расширение переменных среды
      в enum FarDialogItemFlags
  03.07.2000 IS
    + Функция вывода помощи в api
  28.06.2000 SVS
    + Для MSVC тоже требуется extern "C" при декларации
      экспортируемых функций + коррекция на Borland C++ 5.5
  26.06.2000 SVS
    ! Подготовка Master Copy
*/
#endif // END FAR_USE_INTERNALS

#define FARMANAGERVERSION 0x03B80146UL

#define MAKEFARVERSION(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))

#ifdef FAR_USE_INTERNALS
#else // ELSE FAR_USE_INTERNALS
#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
 #if defined(__GNUC__) || defined(_MSC_VER)
  #if !defined(_WINCON_H) && !defined(_WINCON_)
    #define _WINCON_H
    #define _WINCON_ // to prevent including wincon.h
    #if defined(_MSC_VER)
     #pragma pack(push,2)
    #else
     #pragma pack(2)
    #endif
    #include<windows.h>
    #if defined(_MSC_VER)
     #pragma pack(pop)
    #else
     #pragma pack()
    #endif
    #undef _WINCON_
    #undef  _WINCON_H

    #if defined(_MSC_VER)
     #pragma pack(push,8)
    #else
     #pragma pack(8)
    #endif
    #include<wincon.h>
    #if defined(_MSC_VER)
     #pragma pack(pop)
    #else
     #pragma pack()
    #endif
  #endif
  #define _WINCON_
 #else
   #include<windows.h>
 #endif
#endif
#endif // END FAR_USE_INTERNALS

#if defined(__BORLANDC__)
  #pragma option -a2
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(2)
  #if defined(__LCC__)
    #define _export __declspec(dllexport)
  #endif
#else
  #pragma pack(push,2)
  #if _MSC_VER
    #define _export
  #endif
#endif

#define NM 260

#ifndef _WINCON_
typedef struct _INPUT_RECORD INPUT_RECORD;
typedef struct _CHAR_INFO    CHAR_INFO;
#endif

struct PluginPanelItem
{
  WIN32_FIND_DATA FindData;
  DWORD           PackSizeHigh;
  DWORD           PackSize;
  DWORD           Flags;
  DWORD           NumberOfLinks;
  char           *Description;
  char           *Owner;
  char          **CustomColumnData;
  int             CustomColumnNumber;
  DWORD           UserData;
  DWORD           CRC32;
  DWORD           Reserved[2];
};

#if defined(__BORLANDC__)
#if sizeof(PluginPanelItem) != 366
#if defined(STRICT)
#error Incorrect alignment: sizeof(PluginPanelItem)!=366
#else
#pragma message Incorrect alignment: sizeof(PluginPanelItem)!=366
#endif
#endif
#endif

#define PPIF_PROCESSDESCR 0x80000000
#define PPIF_SELECTED     0x40000000
#define PPIF_USERDATA     0x20000000

enum {
  FMENU_SHOWAMPERSAND       =0x0001,
  FMENU_WRAPMODE            =0x0002,
  FMENU_AUTOHIGHLIGHT       =0x0004,
  FMENU_REVERSEAUTOHIGHLIGHT=0x0008,
#ifdef FAR_USE_INTERNALS
  FMENU_SHOWNOBOX           =0x0010,
#endif // END FAR_USE_INTERNALS
  FMENU_USEEXT              =0x0020,
};

enum {
 PN_CMPNAME=0,
 PN_CMPNAMELIST=0x1000UL,
 PN_GENERATENAME=0x2000UL,
 PN_SKIPPATH=0x100000UL
};

typedef int (WINAPI *FARAPIMENU)(
  int                 PluginNumber,
  int                 X,
  int                 Y,
  int                 MaxHeight,
  DWORD               Flags,
  const char         *Title,
  const char         *Bottom,
  const char         *HelpTopic,
  const int          *BreakKeys,
  int                *BreakCode,
  const struct FarMenuItem *Item,
  int                 ItemsNumber
);

typedef long (WINAPI *FARWINDOWPROC)(
  HANDLE hDlg,
  int    Msg,
  int    Param1,
  long   Param2
);

typedef long (WINAPI *FARAPISENDDLGMESSAGE)(
  HANDLE hDlg,
  int    Msg,
  int    Param1,
  long   Param2
);

typedef long (WINAPI *FARAPIDEFDLGPROC)(
  HANDLE hDlg,
  int    Msg,
  int    Param1,
  long   Param2
);

typedef int (WINAPI *FARAPIDIALOG)(
  int                   PluginNumber,
  int                   X1,
  int                   Y1,
  int                   X2,
  int                   Y2,
  const char           *HelpTopic,
  struct FarDialogItem *Item,
  int                   ItemsNumber
);

typedef int (WINAPI *FARAPIDIALOGEX)(
  int                   PluginNumber,
  int                   X1,
  int                   Y1,
  int                   X2,
  int                   Y2,
  const char           *HelpTopic,
  struct FarDialogItem *Item,
  int                   ItemsNumber,
  DWORD                 Reserved,
  DWORD                 Flags,
  FARWINDOWPROC         DlgProc,
  long                  Param
);

enum {
  FDLG_WARNING             =0x00000001,
  FDLG_SMALLDIALOG         =0x00000002,
#ifdef FAR_USE_INTERNALS
  FDLG_NONMODAL            =0x00000004,
#endif // END FAR_USE_INTERNALS
};

enum {
  FMSG_WARNING             =0x00000001,
  FMSG_ERRORTYPE           =0x00000002,
  FMSG_KEEPBACKGROUND      =0x00000004,
  FMSG_DOWN                =0x00000008,
  FMSG_LEFTALIGN           =0x00000010,

  FMSG_ALLINONE            =0x00000020,

  FMSG_MB_OK               =0x00010000,
  FMSG_MB_OKCANCEL         =0x00020000,
  FMSG_MB_ABORTRETRYIGNORE =0x00030000,
  FMSG_MB_YESNO            =0x00040000,
  FMSG_MB_YESNOCANCEL      =0x00050000,
  FMSG_MB_RETRYCANCEL      =0x00060000,
};

typedef int (WINAPI *FARAPIMESSAGE)(
  int PluginNumber,
  DWORD Flags,
  const char *HelpTopic,
  const char * const *Items,
  int ItemsNumber,
  int ButtonsNumber
);


typedef const char* (WINAPI *FARAPIGETMSG)(
  int PluginNumber,
  int MsgId
);

enum DialogItemTypes {
  DI_TEXT,
  DI_VTEXT,
  DI_SINGLEBOX,
  DI_DOUBLEBOX,
  DI_EDIT,
  DI_PSWEDIT,
  DI_FIXEDIT,
  DI_BUTTON,
  DI_CHECKBOX,
  DI_RADIOBUTTON,
  DI_COMBOBOX,
  DI_LISTBOX,

  DI_USERCONTROL=255,
};

enum FarDialogItemFlags {
  DIF_COLORMASK         =0x000000ffUL,
  DIF_SETCOLOR          =0x00000100UL,
  DIF_BOXCOLOR          =0x00000200UL,
  DIF_GROUP             =0x00000400UL,
  DIF_LEFTTEXT          =0x00000800UL,
  DIF_MOVESELECT        =0x00001000UL,
  DIF_SHOWAMPERSAND     =0x00002000UL,
  DIF_CENTERGROUP       =0x00004000UL,
  DIF_NOBRACKETS        =0x00008000UL,
  DIF_MANUALADDHISTORY  =0x00008000UL,
  DIF_SEPARATOR         =0x00010000UL,
  DIF_VAREDIT           =0x00010000UL,
  DIF_EDITOR            =0x00020000UL,
  DIF_LISTNOAMPERSAND   =0x00020000UL,
  DIF_LISTNOBOX         =0x00040000UL,
  DIF_HISTORY           =0x00040000UL,
  DIF_BTNNOCLOSE        =0x00040000UL,
  DIF_EDITEXPAND        =0x00080000UL,
  DIF_DROPDOWNLIST      =0x00100000UL,
  DIF_USELASTHISTORY    =0x00200000UL,
  DIF_MASKEDIT          =0x00400000UL,
  DIF_SELECTONENTRY     =0x00800000UL,
  DIF_3STATE            =0x00800000UL,
  DIF_LISTWRAPMODE      =0x01000000UL,
  DIF_LISTAUTOHIGHLIGHT =0x02000000UL,
  DIF_HIDDEN            =0x10000000UL,
  DIF_READONLY          =0x20000000UL,
  DIF_NOFOCUS           =0x40000000UL,
  DIF_DISABLE           =0x80000000UL,
};

enum FarMessagesProc{
  DM_FIRST=0,
  DM_CLOSE,
  DM_ENABLE,
  DM_ENABLEREDRAW,
  DM_GETDLGDATA,
  DM_GETDLGITEM,
  DM_GETDLGRECT,
  DM_GETTEXT,
  DM_GETTEXTLENGTH,
  DM_KEY,
  DM_MOVEDIALOG,
  DM_SETDLGDATA,
  DM_SETDLGITEM,
  DM_SETFOCUS,
  DM_SETREDRAW,
  DM_SETTEXT,
  DM_SETTEXTLENGTH,
  DM_SHOWDIALOG,
  DM_GETFOCUS,
  DM_GETCURSORPOS,
  DM_SETCURSORPOS,
  DM_GETTEXTPTR,
  DM_SETTEXTPTR,
  DM_SHOWITEM,
  DM_ADDHISTORY,

  DM_GETCHECK,
  DM_SETCHECK,
  DM_SET3STATE,

  DM_LISTSORT,
  DM_LISTGET,
  DM_LISTGETCURPOS,
  DM_LISTSETCURPOS,
  DM_LISTDELETE,
  DM_LISTADD,
  DM_LISTADDSTR,
  DM_LISTUPDATE,
  DM_LISTINSERT,
  DM_LISTFINDSTRING,
  DM_LISTINFO,
  DM_LISTGETDATA,
  DM_LISTSETDATA,
  DM_LISTSETTITLE,
  DM_LISTGETTITLE,

  DM_RESIZEDIALOG,
  DM_SETITEMPOSITION,

  DM_GETDROPDOWNOPENED,
  DM_SETDROPDOWNOPENED,

  DM_SETHISTORY,

  DM_GETITEMPOSITION,
  DM_SETNOTIFYMOUSEEVENT,

  DN_FIRST=0x1000,
  DN_BTNCLICK,
  DN_CTLCOLORDIALOG,
  DN_CTLCOLORDLGITEM,
  DN_CTLCOLORDLGLIST,
  DN_DRAWDIALOG,
  DN_DRAWDLGITEM,
  DN_EDITCHANGE,
  DN_ENTERIDLE,
  DN_GOTFOCUS,
  DN_HELP,
  DN_HOTKEY,
  DN_INITDIALOG,
  DN_KILLFOCUS,
  DN_LISTCHANGE,
  DN_MOUSECLICK,
  DN_DRAGGED,
  DN_RESIZECONSOLE,
  DN_MOUSEEVENT,

  DN_CLOSE=DM_CLOSE,
  DN_KEY=DM_KEY,

  DM_USER=0x4000,

#ifdef FAR_USE_INTERNALS
  DM_KILLSAVESCREEN=DN_FIRST-1,
  DM_ALLKEYMODE=DN_FIRST-2,
#endif // END FAR_USE_INTERNALS
};

enum LISTITEMFLAGS {
  LIF_SELECTED = 0x00010000UL,
  LIF_CHECKED  = 0x00020000UL,
  LIF_SEPARATOR= 0x00040000UL,
  LIF_DISABLE  = 0x00080000UL,
};

enum CHECKEDSTATE {
  BSTATE_UNCHECKED = 0,
  BSTATE_CHECKED   = 1,
  BSTATE_3STATE    = 2
};

struct FarListItem
{
  DWORD Flags;
  char Text[124];
};

struct FarList
{
  int ItemsNumber;
  struct FarListItem *Items;
};

struct FarListUpdate
{
  int Index;
  struct FarListItem *Items;
};

struct FarListInsert
{
  int Index;
  struct FarListItem *Item;
};

struct FarListPos
{
  int SelectPos;
  int TopPos;
};

enum{
  LIFIND_NOPATTERN = 0x00000001,
};

struct FarListFind
{
  int StartIndex;
  char *Pattern;
  DWORD Flags;
  DWORD Reserved;
};

struct FarListDelete
{
  int StartIndex;
  int Count;
};

struct FarListInfo
{
  DWORD Flags;
  int ItemsNumber;
  int SelectPos;
  int TopPos;
  int MaxHeight;
  int MaxLength;
  DWORD Reserved[6];
};

struct FarListItemData
{
  int   Index;
  int   DataSize;
  void *Data;
  DWORD Reserved;
};

struct FarListTitle
{
  int   TitleLen;
  char *Title;
  int   BottomLen;
  char *Bottom;
};

struct FarDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  union
  {
    int Selected;
    char *History;
    char *Mask;
    struct FarList *ListItems;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  int DefaultButton;
  union
  {
    char Data[512];
    int  ListPos;
    struct
    {
      DWORD PtrFlags;
      int   PtrLength;
      char *PtrData;
      char  PtrTail[1];
    } Ptr;
  };
};

struct FarDialogItemData
{
  int   PtrLength;
  char *PtrData;
};

#define Dlg_GetDlgData(Info,hDlg)              Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0)
#define Dlg_SetDlgData(Info,hDlg,Data)         Info.SendDlgMessage(hDlg,DM_SETDLGDATA,0,(long)Data)

#define DlgItem_GetFocus(Info,hDlg)            Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0)
#define DlgItem_SetFocus(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_SETFOCUS,ID,0)
#define DlgItem_Enable(Info,hDlg,ID)           Info.SendDlgMessage(hDlg,DM_ENABLE,ID,TRUE)
#define DlgItem_Disable(Info,hDlg,ID)          Info.SendDlgMessage(hDlg,DM_ENABLE,ID,FALSE)
#define DlgItem_IsEnable(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_ENABLE,ID,-1)
#define DlgItem_SetText(Info,hDlg,ID,Str)      Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,ID,(long)Str)

#define DlgItem_GetCheck(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_GETCHECK,ID,0)
#define DlgItem_SetCheck(Info,hDlg,ID,State)   Info.SendDlgMessage(hDlg,DM_SETCHECK,ID,State)

#define DlgEdit_AddHistory(Info,hDlg,ID,Str)   Info.SendDlgMessage(hDlg,DM_ADDHISTORY,ID,(long)Str)

#define DlgList_AddString(Info,hDlg,ID,Str)    Info.SendDlgMessage(hDlg,DM_LISTADDSTR,ID,(long)Str)
#define DlgList_GetCurPos(Info,hDlg,ID)        Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,ID,0)
#define DlgList_SetCurPos(Info,hDlg,ID,NewPos) Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID,NewPos)
#define DlgList_ClearList(Info,hDlg,ID)        Info.SendDlgMessage(hDlg,DM_LISTDELETE,ID,0)
#define DlgList_DeleteItem(Info,hDlg,ID,Index) {struct FarListDelete FLDItem={Index,1}; Info.SendDlgMessage(hDlg,DM_LISTDELETE,ID,(long)&FLDItem);}
#define DlgList_SortUp(Info,hDlg,ID)           Info.SendDlgMessage(hDlg,DM_LISTSORT,ID,0)
#define DlgList_SortDown(Info,hDlg,ID)         Info.SendDlgMessage(hDlg,DM_LISTSORT,ID,1)
#define DlgList_GetItemData(Info,hDlg,ID,Index)       Info.SendDlgMessage(hDlg,DM_LISTGETDATA,ID,Index)
#define DlgList_SetItemStr(Info,hDlg,ID,Index,Str)    {struct FarListItemData FLID{Index,0,Str,0}; Info.SendDlgMessage(hDlg,DM_LISTSETDATA,ID,(long)&FLID);}


struct FarMenuItem
{
  char Text[128];
  int Selected;
  int Checked;
  int Separator;
};

enum MENUITEMFLAGS {
  MIF_SELECTED = 0x00010000UL,
  MIF_CHECKED  = 0x00020000UL,
  MIF_SEPARATOR= 0x00040000UL,
  MIF_DISABLE  = 0x00080000UL,
};

struct FarMenuItemEx
{
  DWORD Flags;
  char Text[124];
};

struct CmdLineSelect
{
  int SelStart;
  int SelEnd;
};

enum {FCTL_CLOSEPLUGIN,FCTL_GETPANELINFO,FCTL_GETANOTHERPANELINFO,
      FCTL_UPDATEPANEL,FCTL_UPDATEANOTHERPANEL,
      FCTL_REDRAWPANEL,FCTL_REDRAWANOTHERPANEL,
      FCTL_SETANOTHERPANELDIR,FCTL_GETCMDLINE,FCTL_SETCMDLINE,
      FCTL_SETSELECTION,FCTL_SETANOTHERSELECTION,
      FCTL_SETVIEWMODE,FCTL_SETANOTHERVIEWMODE,FCTL_INSERTCMDLINE,
      FCTL_SETUSERSCREEN,FCTL_SETPANELDIR,FCTL_SETCMDLINEPOS,
      FCTL_GETCMDLINEPOS,
      FCTL_SETSORTMODE,FCTL_SETANOTHERSORTMODE,
      FCTL_SETSORTORDER,FCTL_SETANOTHERSORTORDER,
      FCTL_GETCMDLINESELECTION,FCTL_SETCMDLINESELECTION,
};

enum {PTYPE_FILEPANEL,PTYPE_TREEPANEL,PTYPE_QVIEWPANEL,PTYPE_INFOPANEL};

enum PANELINFOFLAGS {
  PFLAGS_SHOWHIDDEN           =0x00000001,
  PFLAGS_HIGHLIGHT            =0x00000002,
  PFLAGS_AUTOCHANGEFOLDER     =0x00000004,
  PFLAGS_SELECTFOLDERS        =0x00000008,
  PFLAGS_ALLOWREVERSESORT     =0x00000010,
  PFLAGS_REVERSESORTORDER     =0x00000020,
  PFLAGS_USESORTGROUPS        =0x00000040,
  PFLAGS_SELECTEDFIRST        =0x00000080,
};

struct PanelInfo
{
  int PanelType;
  int Plugin;
  RECT PanelRect;
  struct PluginPanelItem *PanelItems;
  int ItemsNumber;
  struct PluginPanelItem *SelectedItems;
  int SelectedItemsNumber;
  int CurrentItem;
  int TopPanelItem;
  int Visible;
  int Focus;
  int ViewMode;
  char ColumnTypes[80];
  char ColumnWidths[80];
  char CurDir[NM];
  int ShortNames;
  int SortMode;
  DWORD Flags;
  DWORD Reserved;
};


struct PanelRedrawInfo
{
  int CurrentItem;
  int TopPanelItem;
};


typedef int (WINAPI *FARAPICONTROL)(
  HANDLE hPlugin,
  int Command,
  void *Param
);

typedef HANDLE (WINAPI *FARAPISAVESCREEN)(int X1, int Y1, int X2, int Y2);

typedef void (WINAPI *FARAPIRESTORESCREEN)(HANDLE hScreen);

typedef int (WINAPI *FARAPIGETDIRLIST)(
  const char *Dir,
  struct PluginPanelItem **pPanelItem,
  int *pItemsNumber
);

typedef int (WINAPI *FARAPIGETPLUGINDIRLIST)(
  int PluginNumber,
  HANDLE hPlugin,
  const char *Dir,
  struct PluginPanelItem **pPanelItem,
  int *pItemsNumber
);

typedef void (WINAPI *FARAPIFREEDIRLIST)(const struct PluginPanelItem *PanelItem);

enum VIEWER_FLAGS {
  VF_NONMODAL       =1,
  VF_DELETEONCLOSE  =2,
  VF_ENABLE_F6      =4,
  VF_DISABLEHISTORY =8,
};

enum EDITOR_FLAGS {
  EF_NONMODAL       =1,
  EF_CREATENEW      =2,
  EF_ENABLE_F6      =4,
  EF_DISABLEHISTORY =8,
};

typedef int (WINAPI *FARAPIVIEWER)(
  const char *FileName,
  const char *Title,
  int X1,
  int Y1,
  int X2,
  int Y2,
  DWORD Flags
);

typedef int (WINAPI *FARAPIEDITOR)(
  const char *FileName,
  const char *Title,
  int X1,
  int Y1,
  int X2,
  int Y2,
  DWORD Flags,
  int StartLine,
  int StartChar
);

typedef int (WINAPI *FARAPICMPNAME)(
  const char *Pattern,
  const char *String,
  int SkipPath
);


enum { FCT_DETECT=0x40000000 };

struct CharTableSet
{
  unsigned char DecodeTable[256];
  unsigned char EncodeTable[256];
  unsigned char UpperTable[256];
  unsigned char LowerTable[256];
  char TableName[128];
};

typedef int (WINAPI *FARAPICHARTABLE)(
  int Command,
  char *Buffer,
  int BufferSize
);

typedef void (WINAPI *FARAPITEXT)(
  int X,
  int Y,
  int Color,
  const char *Str
);


typedef int (WINAPI *FARAPIEDITORCONTROL)(
  int Command,
  void *Param
);

enum FarHelpFlags{
  FHELP_NOSHOWERROR =0x80000000,
  FHELP_SELFHELP    =0x00000000,
  FHELP_FARHELP     =0x00000001,
  FHELP_CUSTOMFILE  =0x00000002,
  FHELP_CUSTOMPATH  =0x00000004,
  FHELP_USECONTENTS =0x40000000,
};

typedef BOOL (WINAPI *FARAPISHOWHELP)(
  const char *ModuleName,
  const char *Topic,
  DWORD Flags
);

enum {
  ACTL_GETFARVERSION,
  ACTL_CONSOLEMODE,
  ACTL_GETSYSWORDDIV,
  ACTL_WAITKEY,
  ACTL_GETCOLOR,
  ACTL_GETARRAYCOLOR,
  ACTL_EJECTMEDIA,
  ACTL_KEYMACRO,
  ACTL_POSTKEYSEQUENCE,
  ACTL_GETWINDOWINFO,
  ACTL_GETWINDOWCOUNT,
  ACTL_SETCURRENTWINDOW,
  ACTL_COMMIT,
  ACTL_GETFARHWND,
};

#define CONSOLE_GET_MODE       (-2)
#define CONSOLE_TRIGGER        (-1)
#define CONSOLE_SET_WINDOWED   (0)
#define CONSOLE_SET_FULLSCREEN (1)
#define CONSOLE_WINDOWED       (0)
#define CONSOLE_FULLSCREEN     (1)

#define EJECT_NO_MESSAGE       0x00000001
#define EJECT_LOAD_MEDIA       0x00000002

struct ActlEjectMedia {
  DWORD Letter;
  DWORD Flags;
};


enum MacroCommand{
  MCMD_LOADALL, MCMD_SAVEALL
};

struct ActlKeyMacro{
  int Command;
  DWORD Reserved[3];
};

enum KeySequenceFlags {
  KSFLAGS_DISABLEOUTPUT       = 0x00000001,
};

struct KeySequence{
  DWORD Flags;
  int Count;
  DWORD *Sequence;
};

enum {
  WTYPE_VIRTUAL,
  WTYPE_PANELS,
  WTYPE_VIEWER,
  WTYPE_EDITOR,
  WTYPE_DIALOG,
  WTYPE_VMENU,
  WTYPE_HELP,
  WTYPE_USER,
};

struct WindowInfo
{
  int  Pos;
  int  Type;
  int  Modified;
  int  Current;
  char TypeName[64];
  char Name[NM];
};

typedef int (WINAPI *FARAPIADVCONTROL)(
  int ModuleNumber,
  int Command,
  void *Param
);


#ifdef FAR_USE_INTERNALS
enum VIEWER_CONTROL_COMMANDS {
  VCTL_GETINFO,
  VCTL_QUIT,
  VCTL_REDRAW,
  VCTL_SETKEYBAR,
  VCTL_SETPOSITION,
  VCTL_SELECT,
};

enum VIEWER_OPTIONS {
  VOPT_SAVEFILEPOSITION=1,
  VOPT_AUTODETECTTABLE=2,
};

struct ViewerSelect
{
   long  BlockStartPos;
   long  Reserved1;
   int   BlockLen;
};

enum VIEWER_SETPOS_FLAGS {
  VSP_NOREDRAW    = 0x0001,
  VSP_PERCENT     = 0x0002,
  VSP_RELATIVE    = 0x0004,
  VSP_NORETNEWPOS = 0x0008,
};

struct ViewerSetPosition
{
  DWORD Flags;
  long  StartPos;
  long  Reserved1;
  int   LeftPos;
};

struct ViewerMode{
  int UseDecodeTable;
  int TableNum;
  int AnsiMode;
  int Unicode;
  int Wrap;
  int TypeWrap;
  int Hex;
  DWORD Reserved[4];
};

struct ViewerInfo
{
  int    StructSize;
  int    ViewerID;
  char  *FileName;
  DWORD  FileSize;
  DWORD  Reserved1;
  DWORD  FilePos;
  DWORD  Reserved2;
  int    WindowSizeX;
  int    WindowSizeY;
  DWORD  Options;
  int    TabSize;
  struct ViewerMode CurMode;
  int    LeftPos;
  DWORD  Reserved3;
};

typedef int (WINAPI *FARAPIVIEWERCONTROL)(
  int Command,
  void *Param
);

#define VE_READ     0
#define VE_CLOSE    1

#endif // END FAR_USE_INTERNALS

enum EDITOR_EVENTS {
  EE_READ,EE_SAVE,EE_REDRAW,EE_CLOSE
};

#define EEREDRAW_ALL    (void*)0
#define EEREDRAW_CHANGE (void*)1
#define EEREDRAW_LINE   (void*)2

enum EDITOR_CONTROL_COMMANDS {
  ECTL_GETSTRING,ECTL_SETSTRING,ECTL_INSERTSTRING,ECTL_DELETESTRING,
  ECTL_DELETECHAR,ECTL_INSERTTEXT,ECTL_GETINFO,ECTL_SETPOSITION,
  ECTL_SELECT,ECTL_REDRAW,ECTL_EDITORTOOEM,ECTL_OEMTOEDITOR,
  ECTL_TABTOREAL,ECTL_REALTOTAB,ECTL_EXPANDTABS,ECTL_SETTITLE,
  ECTL_READINPUT,ECTL_PROCESSINPUT,ECTL_ADDCOLOR,ECTL_GETCOLOR,
  ECTL_SAVEFILE,ECTL_QUIT,ECTL_SETKEYBAR,ECTL_PROCESSKEY,ECTL_SETPARAM,
  ECTL_GETBOOKMARK,
};

enum EDITOR_SETPARAMETER_TYPES {
  ESPT_TABSIZE, ESPT_EXPANDTABS, ESPT_AUTOINDENT, ESPT_CURSORBEYONDEOL,
  ESPT_CHARCODEBASE, ESPT_SETTABLE,
};

struct EditorSetParameter
{
  int Type;
  union {
    int iParam;
    char *cParam;
    DWORD Reserved1;
  };
  DWORD Flags;
  DWORD Reserved2;
};

struct EditorGetString
{
  int StringNumber;
  char *StringText;
  char *StringEOL;
  int StringLength;
  int SelStart;
  int SelEnd;
};


struct EditorSetString
{
  int StringNumber;
  char *StringText;
  char *StringEOL;
  int StringLength;
};


enum EDITOR_OPTIONS {
  EOPT_EXPANDTABS=1,EOPT_PERSISTENTBLOCKS=2,EOPT_DELREMOVESBLOCKS=4,
  EOPT_AUTOINDENT=8,EOPT_SAVEFILEPOSITION=16,EOPT_AUTODETECTTABLE=32,
  EOPT_CURSORBEYONDEOL=64
};


enum EDITOR_BLOCK_TYPES {
  BTYPE_NONE,BTYPE_STREAM,BTYPE_COLUMN
};


struct EditorInfo
{
  int EditorID;
  char *FileName;
  int WindowSizeX;
  int WindowSizeY;
  int TotalLines;
  int CurLine;
  int CurPos;
  int CurTabPos;
  int TopScreenLine;
  int LeftPos;
  int Overtype;
  int BlockType;
  int BlockStartLine;
  int AnsiMode;
  int TableNum;
  DWORD Options;
  int TabSize;
  int BookMarkCount;
  DWORD Reserved[7];
};

struct EditorBookMark
{
  long *Line;
  long *Cursor;
  long *ScreenLine;
  long *LeftPos;
  DWORD Reserved[4];
};

struct EditorSetPosition
{
  int CurLine;
  int CurPos;
  int CurTabPos;
  int TopScreenLine;
  int LeftPos;
  int Overtype;
};


struct EditorSelect
{
  int BlockType;
  int BlockStartLine;
  int BlockStartPos;
  int BlockWidth;
  int BlockHeight;
};


struct EditorConvertText
{
  char *Text;
  int TextLength;
};


struct EditorConvertPos
{
  int StringNumber;
  int SrcPos;
  int DestPos;
};


struct EditorColor
{
  int StringNumber;
  int ColorItem;
  int StartPos;
  int EndPos;
  int Color;
};

struct EditorSaveFile
{
  char FileName[NM];
  char *FileEOL;
};

typedef void (WINAPI *FARSTDUNQUOTE)(char *Str);

typedef DWORD (WINAPI *FARSTDEXPANDENVIRONMENTSTR)(
  const char *src,
  char *dst,
  size_t size
);

enum INPUTBOXFLAGS{
  FIB_ENABLEEMPTY      = 0x0001,
  FIB_PASSWORD         = 0x0002,
  FIB_EXPANDENV        = 0x0004,
  FIB_NOUSELASTHISTORY = 0x0008,
  FIB_BUTTONS          = 0x0010,
};

typedef int (WINAPI *FARAPIINPUTBOX)(
  const char *Title,
  const char *SubTitle,
  const char *HistoryName,
  const char *SrcText,
  char *DestText,
  int   DestLength,
  const char *HelpTopic,
  DWORD Flags
);

// <C&C++>
typedef int     (WINAPIV *FARSTDSPRINTF)(char *Buffer,const char *Format,...);
typedef int     (WINAPIV *FARSTDSSCANF)(const char *Buffer, const char *Format,...);
// </C&C++>
typedef void    (WINAPI *FARSTDQSORT)(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
typedef void    (WINAPI *FARSTDQSORTEX)(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *userparam),void *userparam);
typedef void   *(WINAPI *FARSTDBSEARCH)(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
typedef int     (WINAPI *FARSTDGETFILEOWNER)(const char *Computer,const char *Name,char *Owner);
typedef int     (WINAPI *FARSTDGETNUMBEROFLINKS)(const char *Name);
typedef int     (WINAPI *FARSTDATOI)(const char *s);
typedef __int64 (WINAPI *FARSTDATOI64)(const char *s);
typedef char   *(WINAPI *FARSTDITOA64)(__int64 value, char *string, int radix);
typedef char   *(WINAPI *FARSTDITOA)(int value, char *string, int radix);
typedef char   *(WINAPI *FARSTDLTRIM)(char *Str);
typedef char   *(WINAPI *FARSTDRTRIM)(char *Str);
typedef char   *(WINAPI *FARSTDTRIM)(char *Str);
typedef char   *(WINAPI *FARSTDTRUNCSTR)(char *Str,int MaxLength);
typedef char   *(WINAPI *FARSTDTRUNCPATHSTR)(char *Str,int MaxLength);
typedef char   *(WINAPI *FARSTDQUOTESPACEONLY)(char *Str);
typedef char   *(WINAPI *FARSTDPOINTTONAME)(char *Path);
typedef void    (WINAPI *FARSTDGETPATHROOT)(const char *Path,char *Root);
typedef BOOL    (WINAPI *FARSTDADDENDSLASH)(char *Path);
typedef int     (WINAPI *FARSTDCOPYTOCLIPBOARD)(const char *Data);
typedef char   *(WINAPI *FARSTDPASTEFROMCLIPBOARD)(void);
typedef int     (WINAPI *FARSTDINPUTRECORDTOKEY)(const INPUT_RECORD *r);
typedef int     (WINAPI *FARSTDLOCALISLOWER)(unsigned Ch);
typedef int     (WINAPI *FARSTDLOCALISUPPER)(unsigned Ch);
typedef int     (WINAPI *FARSTDLOCALISALPHA)(unsigned Ch);
typedef int     (WINAPI *FARSTDLOCALISALPHANUM)(unsigned Ch);
typedef unsigned (WINAPI *FARSTDLOCALUPPER)(unsigned LowerChar);
typedef unsigned (WINAPI *FARSTDLOCALLOWER)(unsigned UpperChar);
typedef void    (WINAPI *FARSTDLOCALUPPERBUF)(char *Buf,int Length);
typedef void    (WINAPI *FARSTDLOCALLOWERBUF)(char *Buf,int Length);
typedef void    (WINAPI *FARSTDLOCALSTRUPR)(char *s1);
typedef void    (WINAPI *FARSTDLOCALSTRLWR)(char *s1);
typedef int     (WINAPI *FARSTDLOCALSTRICMP)(const char *s1,const char *s2);
typedef int     (WINAPI *FARSTDLOCALSTRNICMP)(const char *s1,const char *s2,int n);
typedef int     (WINAPI *FARSTDPROCESSNAME)(const char *param1, char *param2, DWORD flags);

enum XLATMODE{
  XLAT_SWITCHKEYBLAYOUT = 0x0000001UL,
  XLAT_SWITCHKEYBBEEP   = 0x0000002UL,
};

typedef char*   (WINAPI *FARSTDXLAT)(char *Line,int StartPos,int EndPos,const struct CharTableSet *TableSet,DWORD Flags);
typedef BOOL    (WINAPI *FARSTDKEYTOKEYNAME)(int Key,char *KeyText,int Size);
typedef int     (WINAPI *FARSTDKEYNAMETOKEY)(const char *Name);
typedef int     (WINAPI *FRSUSERFUNC)(const WIN32_FIND_DATA *FData,const char *FullName,void *Param);
typedef void    (WINAPI *FARSTDRECURSIVESEARCH)(const char *InitDir,const char *Mask,FRSUSERFUNC Func,DWORD Flags,void *Param);
typedef char*   (WINAPI *FARSTDMKTEMP)(char *Dest,const char *Prefix);
typedef void    (WINAPI *FARSTDDELETEBUFFER)(char *Buffer);

enum MKLINKOP{
  FLINK_HARDLINK  = 1,
  FLINK_SYMLINK   = 2,
  FLINK_VOLMOUNT  = 3,

  FLINK_SHOWERRMSG= 0x10000,
};
typedef int     (WINAPI *FARSTDMKLINK)(char *Src,char *Dest,DWORD Flags);
typedef int     (WINAPI *FARCONVERTNAMETOREAL)(const char *Src,char *Dest, int DestSize);

enum FRSMODE{
  FRS_RETUPDIR = 0x0001,
  FRS_RECUR    = 0x0002
};

typedef int     (WINAPI *FARGETREPASEPOINTINFO)(const char *Src,char *Dest,int DestSize);


typedef struct FarStandardFunctions
{
  int StructSize;

  FARSTDATOI                 atoi;
  FARSTDATOI64               atoi64;
  FARSTDITOA                 itoa;
  FARSTDITOA64               itoa64;
  // <C&C++>
  FARSTDSPRINTF              sprintf;
  FARSTDSSCANF               sscanf;
  // </C&C++>
  FARSTDQSORT                qsort;
  FARSTDBSEARCH              bsearch;
  FARSTDQSORTEX              qsortex;

  DWORD                      Reserved[9];

  FARSTDLOCALISLOWER         LIsLower;
  FARSTDLOCALISUPPER         LIsUpper;
  FARSTDLOCALISALPHA         LIsAlpha;
  FARSTDLOCALISALPHANUM      LIsAlphanum;
  FARSTDLOCALUPPER           LUpper;
  FARSTDLOCALLOWER           LLower;
  FARSTDLOCALUPPERBUF        LUpperBuf;
  FARSTDLOCALLOWERBUF        LLowerBuf;
  FARSTDLOCALSTRUPR          LStrupr;
  FARSTDLOCALSTRLWR          LStrlwr;
  FARSTDLOCALSTRICMP         LStricmp;
  FARSTDLOCALSTRNICMP        LStrnicmp;

  FARSTDUNQUOTE              Unquote;
  FARSTDEXPANDENVIRONMENTSTR ExpandEnvironmentStr;
  FARSTDLTRIM                LTrim;
  FARSTDRTRIM                RTrim;
  FARSTDTRIM                 Trim;
  FARSTDTRUNCSTR             TruncStr;
  FARSTDTRUNCPATHSTR         TruncPathStr;
  FARSTDQUOTESPACEONLY       QuoteSpaceOnly;
  FARSTDPOINTTONAME          PointToName;
  FARSTDGETPATHROOT          GetPathRoot;
  FARSTDADDENDSLASH          AddEndSlash;
  FARSTDCOPYTOCLIPBOARD      CopyToClipboard;
  FARSTDPASTEFROMCLIPBOARD   PasteFromClipboard;
  FARSTDKEYTOKEYNAME         FarKeyToName;
  FARSTDKEYNAMETOKEY         FarNameToKey;
  FARSTDINPUTRECORDTOKEY     FarInputRecordToKey;
  FARSTDXLAT                 XLat;
  FARSTDGETFILEOWNER         GetFileOwner;
  FARSTDGETNUMBEROFLINKS     GetNumberOfLinks;
  FARSTDRECURSIVESEARCH      FarRecursiveSearch;
  FARSTDMKTEMP               MkTemp;
  FARSTDDELETEBUFFER         DeleteBuffer;
  FARSTDPROCESSNAME          ProcessName;
  FARSTDMKLINK               MkLink;
  FARCONVERTNAMETOREAL       ConvertNameToReal;
  FARGETREPASEPOINTINFO      GetRepasePointInfo;
} FARSTANDARDFUNCTIONS;

struct PluginStartupInfo
{
  int StructSize;
  char ModuleName[NM];
  int ModuleNumber;
  char *RootKey;
  FARAPIMENU             Menu;
  FARAPIDIALOG           Dialog;
  FARAPIMESSAGE          Message;
  FARAPIGETMSG           GetMsg;
  FARAPICONTROL          Control;
  FARAPISAVESCREEN       SaveScreen;
  FARAPIRESTORESCREEN    RestoreScreen;
  FARAPIGETDIRLIST       GetDirList;
  FARAPIGETPLUGINDIRLIST GetPluginDirList;
  FARAPIFREEDIRLIST      FreeDirList;
  FARAPIVIEWER           Viewer;
  FARAPIEDITOR           Editor;
  FARAPICMPNAME          CmpName;
  FARAPICHARTABLE        CharTable;
  FARAPITEXT             Text;
  FARAPIEDITORCONTROL    EditorControl;

  FARSTANDARDFUNCTIONS  *FSF;

  FARAPISHOWHELP         ShowHelp;
  FARAPIADVCONTROL       AdvControl;
  FARAPIINPUTBOX         InputBox;
  FARAPIDIALOGEX         DialogEx;
  FARAPISENDDLGMESSAGE   SendDlgMessage;
  FARAPIDEFDLGPROC       DefDlgProc;
#ifdef FAR_USE_INTERNALS
  DWORD                  Reserved;
  FARAPIVIEWERCONTROL    ViewerControl;
#else // ELSE FAR_USE_INTERNALS
  DWORD                  Reserved[2];
#endif // END FAR_USE_INTERNALS
};


enum PLUGIN_FLAGS {
  PF_PRELOAD        = 0x0001,
  PF_DISABLEPANELS  = 0x0002,
  PF_EDITOR         = 0x0004,
  PF_VIEWER         = 0x0008,
  PF_FULLCMDLINE    = 0x0010,
};


struct PluginInfo
{
  int StructSize;
  DWORD Flags;
  const char * const *DiskMenuStrings;
  int *DiskMenuNumbers;
  int DiskMenuStringsNumber;
  const char * const *PluginMenuStrings;
  int PluginMenuStringsNumber;
  const char * const *PluginConfigStrings;
  int PluginConfigStringsNumber;
  const char *CommandPrefix;
#ifdef FAR_USE_INTERNALS
  DWORD SysID;
#else // ELSE FAR_USE_INTERNALS
  DWORD Reserved;
#endif // END FAR_USE_INTERNALS
};


struct InfoPanelLine
{
  char Text[80];
  char Data[80];
  int Separator;
};


struct PanelMode
{
  char *ColumnTypes;
  char *ColumnWidths;
  char **ColumnTitles;
  int FullScreen;
  int DetailedStatus;
  int AlignExtensions;
  int CaseConversion;
  char *StatusColumnTypes;
  char *StatusColumnWidths;
  DWORD Reserved[2];
};


enum OPENPLUGININFO_FLAGS {
  OPIF_USEFILTER               = 0x0001,
  OPIF_USESORTGROUPS           = 0x0002,
  OPIF_USEHIGHLIGHTING         = 0x0004,
  OPIF_ADDDOTS                 = 0x0008,
  OPIF_RAWSELECTION            = 0x0010,
  OPIF_REALNAMES               = 0x0020,
  OPIF_SHOWNAMESONLY           = 0x0040,
  OPIF_SHOWRIGHTALIGNNAMES     = 0x0080,
  OPIF_SHOWPRESERVECASE        = 0x0100,
  OPIF_FINDFOLDERS             = 0x0200,
  OPIF_COMPAREFATTIME          = 0x0400,
  OPIF_EXTERNALGET             = 0x0800,
  OPIF_EXTERNALPUT             = 0x1000,
  OPIF_EXTERNALDELETE          = 0x2000,
  OPIF_EXTERNALMKDIR           = 0x4000,
  OPIF_USEATTRHIGHLIGHTING     = 0x8000
};


enum OPENPLUGININFO_SORTMODES {
  SM_DEFAULT,SM_UNSORTED,SM_NAME,SM_EXT,SM_MTIME,SM_CTIME,
  SM_ATIME,SM_SIZE,SM_DESCR,SM_OWNER,SM_COMPRESSEDSIZE,SM_NUMLINKS
};


struct KeyBarTitles
{
  char *Titles[12];
  char *CtrlTitles[12];
  char *AltTitles[12];
  char *ShiftTitles[12];

  char *CtrlShiftTitles[12];
  char *AltShiftTitles[12];
  char *CtrlAltTitles[12];
};


struct OpenPluginInfo
{
  int                   StructSize;
  DWORD                 Flags;
  char                 *HostFile;
  char                 *CurDir;
  char                 *Format;
  const char           *PanelTitle;
  struct InfoPanelLine *InfoLines;
  int                   InfoLinesNumber;
  char                **DescrFiles;
  int                   DescrFilesNumber;
  struct PanelMode     *PanelModesArray;
  int                   PanelModesNumber;
  int                   StartPanelMode;
  int                   StartSortMode;
  int                   StartSortOrder;
  struct KeyBarTitles  *KeyBar;
  char                 *ShortcutData;
  long                  Reserverd;
};

enum {
  OPEN_DISKMENU,
  OPEN_PLUGINSMENU,
  OPEN_FINDLIST,
  OPEN_SHORTCUT,
  OPEN_COMMANDLINE,
  OPEN_EDITOR,
  OPEN_VIEWER,
#ifdef FAR_USE_INTERNALS
  OPEN_FILEPANEL,
#endif // END FAR_USE_INTERNALS
};

enum {PKF_CONTROL=1,PKF_ALT=2,PKF_SHIFT=4};

enum FAR_EVENTS {
  FE_CHANGEVIEWMODE,
  FE_REDRAW,
  FE_IDLE,
  FE_CLOSE,
  FE_BREAK,
  FE_COMMAND
};

enum OPERATION_MODES {
  OPM_SILENT=1,
  OPM_FIND=2,
  OPM_VIEW=4,
  OPM_EDIT=8,
  OPM_TOPLEVEL=16,
  OPM_DESCR=32
};

#if defined(__BORLANDC__) || defined(_MSC_VER) || defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
// Exported Functions

void   WINAPI _export ClosePlugin(HANDLE hPlugin);
int    WINAPI _export Compare(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode);
int    WINAPI _export Configure(int ItemNumber);
int    WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
void   WINAPI _export ExitFAR(void);
void   WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
void   WINAPI _export FreeVirtualFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
int    WINAPI _export GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
int    WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
int    WINAPI _export GetMinFarVersion(void);
void   WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info);
void   WINAPI _export GetPluginInfo(struct PluginInfo *Info);
int    WINAPI _export GetVirtualFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,const char *Path);
int    WINAPI _export MakeDirectory(HANDLE hPlugin,char *Name,int OpMode);
HANDLE WINAPI _export OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize);
HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item);
int    WINAPI _export ProcessEditorEvent(int Event,void *Param);
int    WINAPI _export ProcessEditorInput(const INPUT_RECORD *Rec);
int    WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param);
int    WINAPI _export ProcessHostFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
int    WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
int    WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
int    WINAPI _export SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode);
int    WINAPI _export SetFindList(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
void   WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info);

#ifdef FAR_USE_INTERNALS
int    WINAPI _export ProcessViewerEvent(int Event,void *Param);
#endif // END FAR_USE_INTERNALS

#ifdef __cplusplus
};
#endif
#endif

#if defined(__BORLANDC__)
  #pragma option -a.
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack()
#else
  #pragma pack(pop)
#endif

#endif /* __PLUGIN_HPP__ */
