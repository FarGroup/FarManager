/*
keyboard.cpp

Функций, имеющие отношение к клавитуре

*/

/* Revision: 1.121 20.04.2006 $ */

/*
Modify:
  20.04.2006 SVS
    + Opt.UseVk_oem_x - часть I
  17.04.2006 SVS
    + логирование
    ! отвалятся макросы AltShift/ и AltShift*
      т.е. теперь они должны зваться как AltShiftDivide и AltShiftMultiply (и еже с ними)
  10.10.2005 SVS
    - Неправльно кубики сложил - Modif в CalcKeyCode() не там стояло и вычислялось по предыдущим значениям.
  13.09.2005 SVS
    + модификаторы для Spec-клавиш
  09.09.2005 SVS
    + Добавлены (в т.ч. и имена) KEY_F13..KEY_F24 и несколько MM клавиш с
      VK-кодами от A6 до B7.
    + Специфические клавиши, у которых виртуальный код отличен от 0xFF,
      но имена которых FAR не знает, будут хранится как "OemXXXXX"
      (здесь XXXXX десятичное значение виртуального кода клавиши с
      ведущими нулями).
    + Специфические клавиши, у которых виртуальный код равен 0xFF,
      будут хранится как "SpecXXXXX"
      (здесь XXXXX десятичное значение сканкода клавиши с ведущими нулями).
  07.07.2005 SVS
    ! окончание темы, начатой в патче 1742 (01742.CAS.txt) - TechInfo #68
  24.04.2005 AY
    ! GCC
  03.12.2004 SVS
    - BugZ#1196 - Повторное Shift-Enter...
  09.11.2004 SVS
    - Макросы. Если есть автостартующие макросы, то
      Far -r
      заваливается, т.к. файловые панели еще не созданы, соотвественно Cp() вернет NULL.
      Проверим так же, что там Cp() вернет.
  21.08.2004 SVS
    ! Коррекция CheckForEscSilent() на предмет "завершенности" макроса - это когда последним в макросе стоит KEY_NONE.
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  31.05.2004 SVS
    ! выкинем нафиг MCODE_OP_SENDKEY - ненужен
    ! ReplaceStrings - последний параметр TRUE (не различать "высоту" букв)
  27.05.2004 SVS
    ! Для wVirtualKeyCode >= 0xFF возвращаем KEY_IDLE
  22.04.2004 SVS
    - BugZ#1061 - Ненадо учитывать CASE символов управляющих слов в записанных макросах в реестре
      вместо strstr() заюзаем новую функу LocalStrstri() - аналог strstr(), но с локалью и без учета регистра
  14.04.2004 SVS
    - "Ожил (или появился?) макрос"
  29.03.2004 SVS
    - Т.к. работаем в OEM + W-Input-функции, то... необходима перекодировка
      Unicode-символа из входного потока в OEM.
  01.03.2004 SVS
    ! Обертки FAR_OemTo* и FAR_CharTo* вокруг одноименных WinAPI-функций
      (задел на будущее + править впоследствии только 1 файл)
  18.12.2003 SVS
    ! Пробуем различать левый и правый CAS (попытка #1).
  24.10.2003 SVS
    ! уточнение FarNameToKey
  20.10.2003 SVS
    - преобразование Key = FarNameToKey ("AltCtrlБуква") -> FarKeyToName (Key) работает
      некорректно (возвращается Alt00000, вместо AltCtrlБуква
  15.10.2003 SVS
    + обработка вызова грабера при макросении
  13.10.2003 SVS
    ! RunGraber() вызываем в манагере, а в CalcKeyCode() просто вернем KEY_INS|KEY_ALT
  04.10.2003 SVS
    ! RunStartMacro() - "всегда"
  26.09.2003 SVS
    ! Изменения в названиях макроклавиш  25.07.2003 SVS
    ! учтем новые коды возврата для KeyMacro::GetCurRecord()
    ! учтем новый FARMACRO_KEY_EVENT в CalcKeyCode(), иначе плагины получат KEY_NONE
  17.06.2003 SVS
    - Alt-256 не давал нам символ с кодом 0x00. Пока закомментим кусочек один
  27.05.2003 SVS
    - Если CtrlObject->Macro.ProcessKey() вернул "УГУ", то обнулим EventType
      иначе все это барахло (имеется ввиду комбинация инициализации макроса)
      попрется в плагин, обрабатывающий ввод в редакторе.
    ! Вместо события "0x8001" применим осмысленный FARMACRO_KEY_EVENT
  23.05.2003 SVS
    ! Если макроклавиша - "обычная" клавиша, то...
       INPUT_RECORD.EventType=0x8001 - элемент Event содержит структуру
       KEY_EVENT_RECORD с информацией о сообщении с клавиатуры. Это
       сообщение специфичное для FAR manager (начиная с билда 1662) и
       приходит плагину во время "проигрывания" макрокоманды.
  06.05.2003 SVS
    ! очередные работы по DETECT_ALT_ENTER
  21.04.2003 SVS
    ! Автостартующие макросы запускаем только после загрузки всех плагинов!
    + Alt-Enter - пока в тестовой моде.
  05.03.2003 SVS
    ! Закоментим _SVS
  03.02.2003 SVS
    - BugZ#787 - Криво активируется QuickSearch при выполнении макроса.
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  15.01.2003 SVS
    ! У FARGetKeybLayoutName первый параметр может быть NULL
  10.01.2003 SVS
    - "Доисправим" BugZ#488 - Shift=enter
      Если ЭТО был макрос и у него нету KEY_SHIFT, то обнулим ShiftPressed
  06.01.2003 SVS
    ! в GetInputRecord сделаем Discard буферу сохранения в GlobalSaveScrPtr
      при обработке эвента WINDOW_BUFFER_SIZE_EVENT
    + FARGetKeybLayoutName, которая вызывает недокументированную
      KERNEL32.DLL::GetConsoleKeyboardLayoutNameA
  11.12.2002 SVS
    - BugZ#702 - Не обрабатывается изменение размера консоли во время копирования
      ВНИМАНИЕ! В обработку WINDOW_BUFFER_SIZE_EVENT включен механизм редрайва
      фреймов и вызов функции PreRedrawFunc!!!
  10.12.2002 SVS
    - BugZ#695 - Не работает прерывание по Esc
    ! УБРАН SaveScreen в функции ConfirmAbortOp(). МЕШАЕТ!
  18.11.2002 SVS
    - FSF.FarNameToKey("aaa") возвращает вместо -1 0x40000000.
  09.11.2002 SVS
    ! В связи с коррекцией "ReturnAltValue" вводим в макросах понятие
      AltXXXXX - т.е. то, что было введено как Alt-Num. Для того, чтобы
      не конвертировать код клавиши в редакторе!
  04.11.2002 SVS
    ! ReturnAltValue уехала из keyboard.cpp в global.cpp
  18.09.2002 VVM
    + SheckForEscSilent() - проверить на ESC безо всяких запросов пользователя
    + CinfirmAbortOp()    - спросить у пользователя подтверждение прерывания
    ! Соответственно CheckForEsc() теперь состоит из пары этих функций
  16.09.2002 SVS
    - "При драгндропе ярлыка с рабюочего стола его путь вставляется не
       полностью - интересно, почему?"
  21.08.2002 SVS
    ! WaitKey теперь возвращает код нажатой клавиши
  19.08.2002 SVS
    + Аля SKV "Пусть KEY_CTRL тоже будет IsShiftKey" :)
      (про дополнения в "BugZ#579 - сбрасывание выделения")
  17.08.2002 SVS
    - Траблы с Alt-Digit + NumLock=On под масдайкой.
    - В масдае нажатие на Gray-Del всегда приводил к клавише Del, независимо
      от состояния NumLock (при включенном NumLock ожидаем '.', но ее в
      масдае нету).
    - Наконец-то удавил гадину масдаевскую, вставляющую шифты - масдай
      успевает подсовывать Shift в очередь при нажатии курсорных расширенных
      клавиш... В т.ч. и NT... - ну.. "это совсем другая история, страна" ;-)
  23.07.2002 SKV
    + Пусть KEY_ALT тоже будет IsShiftKey :)
  30.05.2002 SVS
    ! Технологический патч для USE_WFUNC_IN - пока только эксперименты
      (чтоб не потерялось :-))
  24.05.2002 SVS
    ! Внедрение Numpad.
    ! исправлена ситуация, когда для прочих событий в клавиатурнике
      делается приведения рекорда к KEY_EVENT без предварительной очистки
      структуры (от чего Autowrap и ему подобным - колбасит по полной
      программе)
  16.05.2002 SVS
    - Фигня с мышью (пред.патч). Закомментим до лучших времен
      (сразу и не заметил, т.к. на LeftAlt на было назначено макроса)
    ! USE_WFUNC -> USE_WFUNC_IN, т.к. это совсем другая история
      Т.е. вывод на экран - это одно, а вот ввод - совсем другое!
      Если с выводом более менее ясно, то здесь полный бардак.
  15.05.2002 SVS
    ! Уточнения в WriteInput (особенно про Key > 255 :-)
    + В обработчике мыши... сравним Ctrl-keys и... если надо дадим сигнал на
      прорисовку, путем засыла в очередь VK_F16
  14.05.2002 VVM
    - Если крутим колесо мыши, то сбрасываем тип события EventType;
  05.04.2002 SVS
    + Prev?ButtonPressed
  01.04.2002 SVS
    ! Вместо KEY_FOCUS_CHANGED заведем KEY_KILLFOCUS и KEY_GOTFOCUS.
  19.03.2002 SVS
    - BugZ#364 - alt+break
  12.03.2002 VVM
    + Задействуем функцию AbortMessage()
  20.02.2002 OT
    - BugZ#314 - Shift-Enter на папке меняет путь заголовок окна
  14.02.2002 VVM
    ! UpdateIfChanged принимает не булевый Force, а варианты из UIC_*
  05.02.2002 SVS
    ! технологический патч - про сислоги
    + SpecKeyName - доступно только при отладке (SysLog)
  04.02.2002 SVS
    + IsNavKey(), IsShiftKey()
  29.01.2002 SVS
    - Была неверная отложенная установка заголовка консоли (неверное,
      старое значение выставлялось)
  10.01.2002 SVS
    ! Устем правило Opt.HotkeyRules для Alt-буква
  27.12.2001 KM
    ! Обнулим KeyText в KeyToText (как и было раньше), в противном случае
      в буфере клавиши возвращался мусор!
  26.12.2001 SVS
    ! При закрытии окна "по кресту"... теперь настраиваемо!
  25.12.2001 SVS
    ! В режиме WaitInMainLoop=1 Alt-символ и Alt-Shift-символ транслируем как
      есть (чтобы корректно работал FastFind). В остальных случаях - берем
      абс.значения кода клавиши.
    ! немного оптимизации
  21.12.2001 SVS
    + DOUBLE_CLICK as Macro
  08.12.2001 SVS
    - Бага в KeyNameToKey() - например, для "ShiftFooBar" выдаст KEY_SHIFT,
      Лечится путем сравнения Pos и Len, т.е. в нормальной ситуации должно
      быть Pos == Len
  26.11.2001 SVS
    + MouseEventFlags, PreMouseEventFlags - типы эвентов мыши
  20.11.2001 SVS
    ! В обработку FOCUS_EVENT добавим также обнуление LButtonPressed,
      RButtonPressed и MButtonPressed
  15.11.2001 SVS
    - BugZ#66 - порча командной строки после двойного AltF9
      Добавив немного Sleep(1) избавляемся от бага...
  24.10.2001 SVS
    ! CheckForEsc() - нажатие Esc в диалоге равносильно нажатию кнопки Yes
      Плюс - запрещение Alt-F9 и скринсавер.
  21.10.2001 SVS
    + PrevScrX,PrevScrY - предыдущие размеры консоли (для позиционирования
      диалогов)
  15.10.2001 SVS
    + _KEYMACRO()
  11.10.2001 SVS
    - BugZ#79. Некоректное преобразование имен клавиш в коды (при ошибках в
      написании названия клавиш)
  20.09.2001 SVS
    - бага с Alt-цифровая клавиша.
    ! Параметр у InputRecordToKey "const"
  18.09.2001 SVS
    ! временно отменим "...теперь даже для макроса корректно заполняется..."
  14.09.2001 SVS
    - Бага в TranslateKeyToVK() - неверно формировалось поле для Shift-клавиш
    ! теперь даже для макроса корректно заполняется структура INPUT_RECORD
      в функции GetInputRecord()
  30.08.2001 IS
    ! При принудительном закрытии Фара пытаемся вести себя так же, как и при
      нажатии на F10 в панелях, только не запрашиваем подтверждение закрытия,
      если это возможно.
  24.07.2001 SVS
    ! Если ждем KEY_CTRLALTSHIFTRELEASE, то гасим курсор (!)
  23.07.2001 SVS
    - Не работала комбинация Alt (не отпуская, в панеля) sp_
      (не набирался симол подчеркивания при нажатой Alt)
  01.07.2001 KM
    - При отпускании Shift-Enter в диалоге назначения
      вылазил Shift после отпускания клавиш.
  28.06.2001 SVS
    - Для Win9x при нажатом NumLock и юзании курсорных клавиш
      получаем в диалоге назначения ерундистику.
  25.06.2001 IS
    ! Внедрение const
  23.06.2001 OT
    - far -r
  21.06.2001 SVS
    ! Удалена функция WriteSequenceInput() за ненадобностью
  14.06.2001 OT
    ! "Бунт" ;-)
  08.06.2001 SVS
    ! Исправление ситуации (влоб) с блокировкой мыши при запуске некоторых
      PE приложений!
    ! ENABLE_MOUSE_INPUT -> ENABLE_WINDOW_INPUT с подаче ER ;-)
  06.06.2001 SVS
    ! Уточнение в функции WriteInput - теперь wVirtualScanCode
      корректно транслируется.
    ! W-функции юзаем пока только в режиме USE_WFUNC
  06.06.2001 SVS
    ! Уточнение в функции TranslateKeyToVK - теперь wVirtualScanCode
      корректно транслируется.
  23.05.2001 SVS
    ! Макрос на Alt+Shift+Цифра
  17.05.2001 OT
    + при изменении размера консоли генерим KEY_CONSOLE_BUFFER_RESIZE.
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  07.05.2001 SVS
    - При быстром поиске Alt-Shift-'-' не начинает поиск файлов, начинающихся
      с '_', а начинает с '-'. Раньше работало нормально.
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  28.04.2001 vvm
    + KEY_FOCUS_CHANGED для прорисовки кейбара.
  27.04.2001 SVS
    + Добавлены:
       "MsWheelDown" для KEY_MSWHEEL_DOWN и
       "MsWheelUp" для KEY_MSWHEEL_UP
      в массив FKeys1[]
    ! Не были учтены шифтовые клавиши при прокрутке колеса, из-за чего
      нельзя было использовать в макросах нечто вроде "ShiftMsWheelUp"
  26.04.2001 VVM
    - Выкинул нафиг MouseWheeled
    + Обработка спецклавиш KEY_MSWHEEL_XXXX
  24.04.2001 SVS
    + MouseWheeled - признак того, что крутанули колесо.
  16.04.2001 VVM
    + Прокрутка с шифтом подменяется на PgUp/PgDn
    + Opt.MouseWheelDelta - задает смещение для прокрутки. Сколько раз посылать UP/DOWN
  13.04.2001 VVM
    + Обработка колесика мышки под 2000.
  12.03.2001 SVS
    ! К терапевту предыдущие изменения - неудобно...
  07.03.2001 SVS
    ! Небольшое изменение - может быть и временное (как работает не пойму пока
      и на что потом повлияет - тоже не знаю, но Shift-F4 Shift-Enter, не
      отпуская Shift - работает корректно :-)
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  25.02.2001 SVS
    ! Уточнения для Alt-Shift-...
  09.02.2001 IS
    + Подтверждение нажатия Esc в CheckForEsc (опционально)
  06.02.2001 SVS
    - ОНИ... :-)
      Приведение в порядок "непослушных" Divide & Multiple на цифровой клаве.
  05.02.2001 SVS
    - Снова про клавиши... :-( Все переводилось в Upper.
  01.02.2001 SVS
    - бага - неверно конвертилось имя клавиши :-(
  01.02.2001 SVS
    - Неверное преобразование клавиш в VK_* в функции TranslateKeyToVK()
      из-за чего не отрабатывал ENTER, etc в плагинах.
    - Не отрабатывались комбинации Alt-Shift-* и Alt-Shift-? при быстром
      поиске в панелях.
  28.01.2001 SVS
    ! Снова про... WriteInput (с учетом данных на VK_F16)
  24.01.2001 SVS
    ! Вернем взад содержимое WriteInput
  23.01.2001 SVS
    ! CalcKeyCode - дополнительный параметр - проверка на макросы.
    ! Исключения вызовов макросов при указании "не использовать макросы"
  21.01.2001 SVS
    ! Уточнения в WriteInput!
    ! WriteInput теперь возвращает результат в виде FALASE/TRUE.
    + WriteSequenceInput
  17.01.2001 SVS
    + Opt.ShiftsKeyRules - Правило на счет выбора механизма трансляции
      Alt-Буква для нелатинским буковок
  09.01.2001 SVS
    ! Грабер должен быть раньше Alt-Number-ввода
  05.01.2001 SVS
    - И снова CalcKeyCode - не работал Alt-Ins
  05.01.2001 SVS
    - База в "вычислителе" клавиш :-(
  04.01.2001 SVS
    + Переделка алгоритмов декодирования клавиш...
    + TranslateKeyToVK
  26.12.2000 SVS
    + вызов KeyMacroToText() (функция определена в macro.cpp)
  22.12.2000 SVS
    + Выделение в качестве самостоятельного модуля, после чего можно смело
      ваять интерфейс пвсевдоклавиатурного драйвера :-)
*/

#include "headers.hpp"
#pragma hdrstop

#include "keys.hpp"
#include "farqueue.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "plugin.hpp"
#include "lang.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "grabber.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"
#include "lockscrn.hpp"


//from WinUser.h
#define VK_BROWSER_BACK                  0xA6
#define VK_BROWSER_FORWARD               0xA7
#define VK_BROWSER_REFRESH               0xA8
#define VK_BROWSER_STOP                  0xA9
#define VK_BROWSER_SEARCH                0xAA
#define VK_BROWSER_FAVORITES             0xAB
#define VK_BROWSER_HOME                  0xAC
#define VK_VOLUME_MUTE                   0xAD
#define VK_VOLUME_DOWN                   0xAE
#define VK_VOLUME_UP                     0xAF
#define VK_MEDIA_NEXT_TRACK              0xB0
#define VK_MEDIA_PREV_TRACK              0xB1
#define VK_MEDIA_STOP                    0xB2
#define VK_MEDIA_PLAY_PAUSE              0xB3
#define VK_LAUNCH_MAIL                   0xB4
#define VK_LAUNCH_MEDIA_SELECT           0xB5
#define VK_LAUNCH_APP1                   0xB6
#define VK_LAUNCH_APP2                   0xB7
#define VK_OEM_1                         0xBA   // ';:' for US
#define VK_OEM_PLUS                      0xBB   // '+' any country
#define VK_OEM_COMMA                     0xBC   // ',' any country
#define VK_OEM_MINUS                     0xBD   // '-' any country
#define VK_OEM_PERIOD                    0xBE   // '.' any country
#define VK_OEM_2                         0xBF   // '/?' for US
#define VK_OEM_3                         0xC0   // '`~' for US
#define VK_OEM_4                         0xDB  //  '[{' for US
#define VK_OEM_5                         0xDC  //  '\|' for US
#define VK_OEM_6                         0xDD  //  ']}' for US
#define VK_OEM_7                         0xDE  //  ''"' for US
#define VK_OEM_8                         0xDF
#define VK_OEM_AX                        0xE1  //  'AX' key on Japanese AX kbd
#define VK_OEM_102                       0xE2  //  "<>" or "\|" on RT 102-key kbd.

static unsigned int AltValue=0;
static int KeyCodeForALT_LastPressed=0;

#if defined(MOUSEKEY)
static int PrePreMouseEventFlags;
#endif

static int ShiftPressedLast=FALSE,AltPressedLast=FALSE,CtrlPressedLast=FALSE;
static BOOL IsKeyCASPressed=FALSE; // CtrlAltShift - нажато или нет?

static int RightShiftPressedLast=FALSE,RightAltPressedLast=FALSE,RightCtrlPressedLast=FALSE;
static BOOL IsKeyRCASPressed=FALSE; // Right CtrlAltShift - нажато или нет?

static clock_t PressedLastTime,KeyPressedLastTime;
static int ShiftState=0;
static int AltEnter=-1;
static int LastShiftEnterPressed=FALSE;

/* ----------------------------------------------------------------- */
static struct TTable_KeyToVK{
  int Key;
  int VK;
} Table_KeyToVK[]={
//   {KEY_PGUP,          VK_PRIOR},
//   {KEY_PGDN,          VK_NEXT},
//   {KEY_END,           VK_END},
//   {KEY_HOME,          VK_HOME},
//   {KEY_LEFT,          VK_LEFT},
//   {KEY_UP,            VK_UP},
//   {KEY_RIGHT,         VK_RIGHT},
//   {KEY_DOWN,          VK_DOWN},
//   {KEY_INS,           VK_INSERT},
//   {KEY_DEL,           VK_DELETE},
//   {KEY_LWIN,          VK_LWIN},
//   {KEY_RWIN,          VK_RWIN},
//   {KEY_APPS,          VK_APPS},
//   {KEY_MULTIPLY,      VK_MULTIPLY},
//   {KEY_ADD,           VK_ADD},
//   {KEY_SUBTRACT,      VK_SUBTRACT},
//   {KEY_DIVIDE,        VK_DIVIDE},
//   {KEY_F1,            VK_F1},
//   {KEY_F2,            VK_F2},
//   {KEY_F3,            VK_F3},
//   {KEY_F4,            VK_F4},
//   {KEY_F5,            VK_F5},
//   {KEY_F6,            VK_F6},
//   {KEY_F7,            VK_F7},
//   {KEY_F8,            VK_F8},
//   {KEY_F9,            VK_F9},
//   {KEY_F10,           VK_F10},
//   {KEY_F11,           VK_F11},
//   {KEY_F12,           VK_F12},
   {KEY_BREAK,         VK_CANCEL},
   {KEY_BS,            VK_BACK},
   {KEY_TAB,           VK_TAB},
   {KEY_ENTER,         VK_RETURN},
   {KEY_ESC,           VK_ESCAPE},
   {KEY_SPACE,         VK_SPACE},
   {KEY_NUMPAD5,       VK_CLEAR},
};


struct TFKey3{
  DWORD Key;
  int   Len;
  char *Name;
};

static struct TFKey3 FKeys1[]={
  { KEY_RCTRLALTSHIFTRELEASE,24, "RightCtrlAltShiftRelease"},
  { KEY_RCTRLALTSHIFTPRESS,  22, "RightCtrlAltShiftPress"},
  { KEY_CTRLALTSHIFTRELEASE, 19, "CtrlAltShiftRelease"},
  { KEY_CTRLALTSHIFTPRESS,   17, "CtrlAltShiftPress"},
  { KEY_LAUNCH_MEDIA_SELECT, 17, "LaunchMediaSelect"},
  { KEY_BROWSER_FAVORITES,   16, "BrowserFavorites"},
  { KEY_MEDIA_PREV_TRACK,    14, "MediaPrevTrack"},
  { KEY_MEDIA_PLAY_PAUSE,    14, "MediaPlayPause"},
  { KEY_MEDIA_NEXT_TRACK,    14, "MediaNextTrack"},
  { KEY_BROWSER_REFRESH,     14, "BrowserRefresh"},
  { KEY_BROWSER_FORWARD,     14, "BrowserForward"},
  //{ KEY_HP_COMMUNITIES,      13, "HPCommunities"},
  { KEY_BROWSER_SEARCH,      13, "BrowserSearch"},
#if defined(MOUSEKEY)
  { KEY_MSLDBLCLICK,         11, "MsLDblClick"},
  { KEY_MSRDBLCLICK,         11, "MsRDblClick"},
#endif
  //{ KEY_AC_BOOKMARKS,        11, "ACBookmarks"},
  { KEY_MSWHEEL_DOWN,        11, "MsWheelDown"},
  { KEY_BROWSER_STOP,        11, "BrowserStop"},
  { KEY_BROWSER_HOME,        11, "BrowserHome"},
  { KEY_BROWSER_BACK,        11, "BrowserBack"},
  { KEY_VOLUME_MUTE,         10, "VolumeMute"},
  { KEY_VOLUME_DOWN,         10, "VolumeDown"},
  { KEY_LAUNCH_MAIL,         10, "LaunchMail"},
  { KEY_LAUNCH_APP2,         10, "LaunchApp2"},
  { KEY_LAUNCH_APP1,         10, "LaunchApp1"},
  //{ KEY_HP_INTERNET,         10, "HPInternet"},
  //{ KEY_AC_FORWARD,           9, "ACForward"},
  //{ KEY_AC_REFRESH,           9, "ACRefresh"},
  { KEY_MSWHEEL_UP,           9, "MsWheelUp"},
  { KEY_MEDIA_STOP,           9, "MediaStop"},
  { KEY_BACKSLASH,            9, "BackSlash"},
  //{ KEY_HP_MEETING,           9, "HPMeeting"},
  //{ KEY_HP_MARKET,            8, "HPMarket"},
  { KEY_VOLUME_UP,            8, "VolumeUp"},
  { KEY_SUBTRACT,             8, "Subtract"},
  { KEY_MULTIPLY,             8, "Multiply"},
  //{ KEY_HP_SEARCH,            8, "HPSearch"},
  //{ KEY_HP_HOME,              6, "HPHome"},
  //{ KEY_HP_MAIL,              6, "HPMail"},
  //{ KEY_HP_NEWS,              6, "HPNews"},
  //{ KEY_AC_BACK,              6, "ACBack"},
  //{ KEY_AC_STOP,              6, "ACStop"},
  { KEY_DIVIDE,               6, "Divide"},
  { KEY_SPACE,                5, "Space"},
  { KEY_RIGHT,                5, "Right"},
  { KEY_ENTER,                5, "Enter"},
  { KEY_CLEAR,                5, "Clear"},
  { KEY_BREAK,                5, "Break"},
  { KEY_PGUP,                 4, "PgUp"},
  { KEY_PGDN,                 4, "PgDn"},
  { KEY_LEFT,                 4, "Left"},
  { KEY_HOME,                 4, "Home"},
  { KEY_DOWN,                 4, "Down"},
  { KEY_APPS,                 4, "Apps"},
  { KEY_RWIN,                 4 ,"RWin"},
  { KEY_NUMPAD9,              4 ,"Num9"},
  { KEY_NUMPAD8,              4 ,"Num8"},
  { KEY_NUMPAD7,              4 ,"Num7"},
  { KEY_NUMPAD6,              4 ,"Num6"},
  { KEY_NUMPAD5,              4, "Num5"},
  { KEY_NUMPAD4,              4 ,"Num4"},
  { KEY_NUMPAD3,              4 ,"Num3"},
  { KEY_NUMPAD2,              4 ,"Num2"},
  { KEY_NUMPAD1,              4 ,"Num1"},
  { KEY_NUMPAD0,              4 ,"Num0"},
  { KEY_LWIN,                 4 ,"LWin"},
  { KEY_TAB,                  3, "Tab"},
  { KEY_INS,                  3, "Ins"},
  { KEY_F10,                  3, "F10"},
  { KEY_F11,                  3, "F11"},
  { KEY_F12,                  3, "F12"},
  { KEY_F13,                  3, "F13"},
  { KEY_F14,                  3, "F14"},
  { KEY_F15,                  3, "F15"},
  { KEY_F16,                  3, "F16"},
  { KEY_F17,                  3, "F17"},
  { KEY_F18,                  3, "F18"},
  { KEY_F19,                  3, "F19"},
  { KEY_F20,                  3, "F20"},
  { KEY_F21,                  3, "F21"},
  { KEY_F22,                  3, "F22"},
  { KEY_F23,                  3, "F23"},
  { KEY_F24,                  3, "F24"},
  { KEY_ESC,                  3, "Esc"},
  { KEY_END,                  3, "End"},
  { KEY_DEL,                  3, "Del"},
  { KEY_ADD,                  3, "Add"},
  { KEY_UP,                   2, "Up"},
  { KEY_F9,                   2, "F9"},
  { KEY_F8,                   2, "F8"},
  { KEY_F7,                   2, "F7"},
  { KEY_F6,                   2, "F6"},
  { KEY_F5,                   2, "F5"},
  { KEY_F4,                   2, "F4"},
  { KEY_F3,                   2, "F3"},
  { KEY_F2,                   2, "F2"},
  { KEY_F1,                   2, "F1"},
  { KEY_BS,                   2, "BS"},
  { KEY_BACKBRACKET,          1, "]"},
  { KEY_QUOTE,                1, "\""},
  { KEY_BRACKET,              1, "["},
  { KEY_COLON,                1, ":"},
  { KEY_SLASH,                1, "/"},
  { KEY_DOT,                  1, "."},
  { KEY_COMMA,                1, ","},
};


static struct TFKey3 ModifKeyName[]={
  { KEY_RCTRL  ,5 ,"RCtrl"},
  { KEY_SHIFT  ,5 ,"Shift"},
  { KEY_CTRL   ,4 ,"Ctrl"},
  { KEY_RALT   ,4 ,"RAlt"},
  { KEY_ALT    ,3 ,"Alt"},
  { KEY_M_SPEC ,4 ,"Spec"},
  { KEY_M_OEM  ,3 ,"Oem"},
//  { KEY_LCTRL  ,5 ,"LCtrl"},
//  { KEY_LALT   ,4 ,"LAlt"},
//  { KEY_LSHIFT ,6 ,"LShift"},
//  { KEY_RSHIFT ,6 ,"RShift"},
};

#if defined(SYSLOG)
static struct TFKey3 SpecKeyName[]={
  { KEY_CONSOLE_BUFFER_RESIZE,19, "ConsoleBufferResize"},
  { KEY_LOCKSCREEN           ,10, "LockScreen"},
  { KEY_KILLFOCUS             ,9, "KillFocus"},
  { KEY_GOTFOCUS              ,8, "GotFocus"},
  { KEY_DRAGCOPY             , 8, "DragCopy"},
  { KEY_DRAGMOVE             , 8, "DragMove"},
  { KEY_NONE                 , 4, "None"},
  { KEY_IDLE                 , 4, "Idle"},
};
#endif

/* ----------------------------------------------------------------- */

/* tran 31.08.2000 $
  FarInputRecordToKey */
int WINAPI InputRecordToKey(const INPUT_RECORD *r)
{
  if(r)
  {
    INPUT_RECORD Rec=*r; // НАДО!, т.к. внутри CalcKeyCode
                         //   структура INPUT_RECORD модифицируется!
    return CalcKeyCode(&Rec,TRUE);
  }
  return KEY_NONE;
}
/* tran 31.08.2000 $ */


int IsMouseButtonPressed()
{
  INPUT_RECORD rec;
  if (PeekInputRecord(&rec))
    GetInputRecord(&rec);
  Sleep(20);
  if (LButtonPressed)
    return(1);
  if (RButtonPressed)
    return(2);
  /* $ 23.08.2000 SVS
     + Дополнительно - для средней клавиши мыши
  */
  if(MButtonPressed)
    return(3);
  /* SVS $ */
  return(0);
}

DWORD GetInputRecord(INPUT_RECORD *rec)
{
  _KEYMACRO(CleverSysLog Clev("GetInputRecord - main"));
  static int LastEventIdle=FALSE;
  DWORD ReadCount;
  DWORD LoopCount=0,CalcKey;
  DWORD ReadKey=0;
  int NotMacros=FALSE;

  if (CtrlObject && CtrlObject->Cp())
  {
    _KEYMACRO(CleverSysLog SL("Macro) Get Next MacroKey"));
    int VirtKey,ControlState;
    CtrlObject->Macro.RunStartMacro();

    int MacroKey=CtrlObject->Macro.GetKey();
    _KEYMACRO(SysLog("Macro) [%d] MacroKey =%s",__LINE__,_FARKEY_ToName(MacroKey)));
    if (MacroKey)
    {
      ScrBuf.Flush();
      TranslateKeyToVK(MacroKey,VirtKey,ControlState,rec);
      rec->EventType=((MacroKey >= KEY_MACRO_BASE && MacroKey <= KEY_MACRO_ENDBASE) || (MacroKey&(~0xFF000000)) >= KEY_END_FKEY)?0:FARMACRO_KEY_EVENT;
      if(!(MacroKey&KEY_SHIFT))
        ShiftPressed=0;
      _KEYMACRO(SysLog("Macro) [%d] MacroKey1 =%s",__LINE__,_FARKEY_ToName(MacroKey)));
//      memset(rec,0,sizeof(*rec));
      return(MacroKey);
    }
  }

  if(KeyQueue && KeyQueue->Peek())
  {
    CalcKey=KeyQueue->Get();
    NotMacros=CalcKey&0x80000000?1:0;
    CalcKey&=~0x80000000;
    //???
    if(CtrlObject && CtrlObject->Macro.IsRecording() && (CalcKey == (KEY_ALT|KEY_NUMPAD0) || CalcKey == (KEY_ALT|KEY_INS)))
    {
      if(CtrlObject->Macro.ProcessKey(CalcKey))
      {
        RunGraber();
        rec->EventType=0;
        CalcKey=KEY_NONE;
      }
      return(CalcKey);
    }

    if (!NotMacros)
    {
      _KEYMACRO(CleverSysLog Clev("Macro) CALL(1) CtrlObject->Macro.ProcessKey()"));
      if (CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(CalcKey))
      {
        rec->EventType=0;
        CalcKey=KEY_NONE;
      }
    }
    return(CalcKey);
  }

  int EnableShowTime=Opt.Clock && (WaitInMainLoop || CtrlObject!=NULL &&
                     CtrlObject->Macro.GetMode()==MACRO_SEARCH);

  if (EnableShowTime)
    ShowTime(1);

  ScrBuf.Flush();

  if (!LastEventIdle)
    StartIdleTime=clock();
  LastEventIdle=FALSE;
  SetFarConsoleMode();
  while (1)
  {
#if defined(USE_WFUNC_IN)
    if(Opt.UseUnicodeConsole)
      PeekConsoleInputW(hConInp,rec,1,&ReadCount);
    else
      PeekConsoleInputA(hConInp,rec,1,&ReadCount);
#else
    PeekConsoleInput(hConInp,rec,1,&ReadCount);
#endif
    _SVS(SysLog("ReadCount=%d",ReadCount));
    /* $ 26.04.2001 VVM
       ! Убрал подмену колесика */
    if (ReadCount!=0)
    {
/*
      // При каптюренной мыши отдаем управление заданному объекту
      if (rec->EventType==MOUSE_EVENT && ScreenObject::CaptureMouseObject)
      {
        ScreenObject::CaptureMouseObject->ProcessMouse(&rec->Event.MouseEvent);
        ReadConsoleInput(hConInp,rec,1,&ReadCount);
        continue;
      }
*/
//_SVS(if(rec->EventType==KEY_EVENT)SysLog("Opt.UseUnicodeConsole=%d",Opt.UseUnicodeConsole));
//_SVS(if(rec->EventType==KEY_EVENT)SysLog("[%d] if(rec->EventType==KEY_EVENT) >>> %s",__LINE__,_INPUT_RECORD_Dump(rec)));

/*
#if defined(USE_WFUNC_IN)
    if(Opt.UseUnicodeConsole)
       MultiByteToWideChar(CP_OEMCP, MB_USEGLYPHCHARS, &Chr, 1, Oem2Unicode+I, 1);
#endif
*/

#if defined(USE_WFUNC_IN)
      if(Opt.UseUnicodeConsole && rec->EventType==KEY_EVENT)
      {
        WCHAR UnicodeChar=rec->Event.KeyEvent.uChar.UnicodeChar;
_SVS(SysLog("UnicodeChar= %C, 0x%04X",UnicodeChar,UnicodeChar));
_SVS(SysLog(">GetInputRecord= %s",_INPUT_RECORD_Dump(rec)));
        rec->Event.KeyEvent.uChar.UnicodeChar=0;
        UnicodeToOEM(&UnicodeChar,&rec->Event.KeyEvent.uChar.AsciiChar,sizeof(WCHAR));
_SVS(SysLog("<GetInputRecord= %s",_INPUT_RECORD_Dump(rec)));
      }
#endif

#if defined(DETECT_ALT_ENTER)

      /*
         Windowed -> FullScreen
           KEY_EVENT_RECORD:   Dn, Vk="VK_MENU"    FarAltEnter:  0
           FOCUS_EVENT_RECORD: FALSE               FarAltEnter:  1
             FOCUS_EVENT_RECORD: TRUE                FarAltEnter:  1
             FOCUS_EVENT_RECORD: TRUE                FarAltEnter:  1

         FullScreen -> Windowed
           KEY_EVENT_RECORD:   Dn, Vk="VK_MENU"    FarAltEnter:  1
           WINDOW_BUFFER_SIZE_RECORD: Size=[W,H]   FarAltEnter:  0
             FOCUS_EVENT_RECORD: FALSE               FarAltEnter:  0
             FOCUS_EVENT_RECORD: TRUE                FarAltEnter:  0
      */

      if(rec->EventType==KEY_EVENT && rec->Event.KeyEvent.wVirtualKeyCode == VK_MENU && rec->Event.KeyEvent.bKeyDown)
      {
        AltEnter=1;
      }
      else if(AltEnter &&
              (rec->EventType==FOCUS_EVENT && !rec->Event.FocusEvent.bSetFocus ||
               rec->EventType==WINDOW_BUFFER_SIZE_EVENT) &&
              PrevFarAltEnterMode != FarAltEnter(FAR_CONSOLE_GET_MODE))
      {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConOut,&csbi);
        _SVS(SysLog("AltEnter >>> dwSize={%d,%d} srWindow={%d,%d,%d,%d} dwMaximumWindowSize={%d,%d}  ScrX=%d (%d) ScrY=%d (%d)",csbi.dwSize.X,csbi.dwSize.Y,csbi.srWindow.Left, csbi.srWindow.Top,csbi.srWindow.Right,csbi.srWindow.Bottom,csbi.dwMaximumWindowSize.X,csbi.dwMaximumWindowSize.Y,ScrX,PrevScrX,ScrY,PrevScrY));

        if(rec->EventType==FOCUS_EVENT)
        {
          DWORD ReadCount2;
          INPUT_RECORD TmpRec2;
          TmpRec2.EventType=WINDOW_BUFFER_SIZE_EVENT;
          TmpRec2.Event.WindowBufferSizeEvent.dwSize.X=csbi.dwSize.X;
          TmpRec2.Event.WindowBufferSizeEvent.dwSize.Y=csbi.dwSize.Y;
          WriteConsoleInput(hConInp,&TmpRec2,1,&ReadCount2); // вернем самый первый!
        }
        else
          AltEnter=1;
        //PrevFarAltEnterMode=FarAltEnter(FAR_CONSOLE_GET_MODE);
      /*
        DWORD ReadCount2,ReadCount3;
        GetNumberOfConsoleInputEvents(hConInp,&ReadCount2);
        if(ReadCount2 >= 3)
        {
          INPUT_RECORD TmpRec2[2];
          ReadConsoleInput(hConInp,TmpRec2,2,&ReadCount3); // удалим 2, третья считается позже
        }
      */
      }
      else
        AltEnter=0;
#endif
      // в масдае хрен знает что творится с расширенными курсорными клавишами ;-(
      // Эта фигня нужна только в диалоге назначения макро - остальное по барабану - и так работает
      // ... иначе хреновень с эфектом залипшего шифта проскакивает
      if(rec->EventType==KEY_EVENT && IsProcessAssignMacroKey)
      {
        if(rec->Event.KeyEvent.wVirtualKeyCode == VK_SHIFT)
        {
          if(rec->Event.KeyEvent.dwControlKeyState&ENHANCED_KEY)
          {
            /*
            Left Right курсорные расширенные клавиши
            MustDie:
            Dn, 1, Vk=0x0025, Scan=0x004B Ctrl=0x00000120 (casa - cEcN)
            Dn, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000130 (caSa - cEcN)
                          ^^                          ^            ^ !!! вот такое и прокинем ;-)
            Up, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000120 (casa - cEcN)
            Up, 1, Vk=0x0025, Scan=0x004B Ctrl=0x00000120 (casa - cEcN)
            Dn, 1, Vk=0x0027, Scan=0x004D Ctrl=0x00000120 (casa - cEcN)
            Dn, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000130 (caSa - cEcN)
            Up, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000120 (casa - cEcN)
            Up, 1, Vk=0x0027, Scan=0x004D Ctrl=0x00000120 (casa - cEcN)
            --------------------------------------------------------------
            NT:
            Dn, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000030 (caSa - cecN)
            Dn, 1, Vk=0x0025, Scan=0x004B Ctrl=0x00000130 (caSa - cEcN)
            Up, 1, Vk=0x0025, Scan=0x004B Ctrl=0x00000130 (caSa - cEcN)
            Dn, 1, Vk=0x0027, Scan=0x004D Ctrl=0x00000130 (caSa - cEcN)
            Up, 1, Vk=0x0027, Scan=0x004D Ctrl=0x00000130 (caSa - cEcN)
            Up, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000020 (casa - cecN)
            */
            INPUT_RECORD pinp;
            DWORD nread;
            ReadConsoleInput(hConInp, &pinp, 1, &nread);
            continue;
          }

          /* коррекция шифта, т.к.
          NumLock=ON Shift-Numpad1
             Dn, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000030 (caSa - cecN)
             Dn, 1, Vk=0x0023, Scan=0x004F Ctrl=0x00000020 (casa - cecN)
             Up, 1, Vk=0x0023, Scan=0x004F Ctrl=0x00000020 (casa - cecN)
          >>>Dn, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000030 (caSa - cecN)
             Up, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000020 (casa - cecN)
          винда вставляет лишний шифт
          */
          if(rec->Event.KeyEvent.bKeyDown)
          {
            if(!ShiftState)
              ShiftState=TRUE;
            else // Здесь удалим из очереди... этот самый кривой шифт
            {
              INPUT_RECORD pinp;
              DWORD nread;
              ReadConsoleInput(hConInp, &pinp, 1, &nread);
              continue;
            }
          }
          else if(!rec->Event.KeyEvent.bKeyDown)
            ShiftState=FALSE;
        }

        if((rec->Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED) == 0 && ShiftState)
          rec->Event.KeyEvent.dwControlKeyState|=SHIFT_PRESSED;
      }

      // // _SVS(INPUT_RECORD_DumpBuffer());

#if 0
      if(rec->EventType==KEY_EVENT)
      {
        // берем количество оставшейся порции эвентов
        DWORD ReadCount2;
        GetNumberOfConsoleInputEvents(hConInp,&ReadCount2);
        // если их безобразно много, то просмотрим все на предмет KEY_EVENT
        if(ReadCount2 > 1)
        {
          INPUT_RECORD *TmpRec=(INPUT_RECORD*)xf_malloc(sizeof(INPUT_RECORD)*ReadCount2);
          if(TmpRec)
          {
            DWORD ReadCount3;
            INPUT_RECORD TmpRec2;
            int I;

            #if defined(USE_WFUNC_IN)
            if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
              PeekConsoleInputW(hConInp,TmpRec,ReadCount2,&ReadCount3);
            else
              PeekConsoleInputA(hConInp,TmpRec,ReadCount2,&ReadCount3);
            #else
            PeekConsoleInput(hConInp,TmpRec,ReadCount2,&ReadCount3);
            #endif
            for(I=0; I < ReadCount2; ++I)
            {
              if(TmpRec[I].EventType!=KEY_EVENT)
                break;

              // // _SVS(SysLog("%d> %s",I,_INPUT_RECORD_Dump(rec)));

              // удаляем из очереди
              #if defined(USE_WFUNC_IN)
              if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
                ReadConsoleInputW(hConInp,&TmpRec2,1,&ReadCount3);
              else
                ReadConsoleInputA(hConInp,&TmpRec2,1,&ReadCount3);
              #else
              ReadConsoleInput(hConInp,&TmpRec2,1,&ReadCount3);
              #endif

              if(TmpRec[I].Event.KeyEvent.bKeyDown==1)
              {
                if (TmpRec[I].Event.KeyEvent.uChar.AsciiChar != 0)
                  WriteInput(TmpRec[I].Event.KeyEvent.uChar.AsciiChar,0);
              }
              else if(TmpRec[I].Event.KeyEvent.wVirtualKeyCode==0x12)
              {
                if (TmpRec[I].Event.KeyEvent.uChar.AsciiChar != 0)
                  WriteInput(TmpRec[I].Event.KeyEvent.uChar.AsciiChar,0);
              }
            }
            // освободим память
            xf_free(TmpRec);
            return KEY_NONE;
          }
        }
      }
#endif
      break;
    }
    /* VVM $ */

    ScrBuf.Flush();

    Sleep(15);
    // Позволяет избежать ситуации блокирования мыши
    if(Opt.Mouse) // А нужно ли это условие???
      SetFarConsoleMode();

    if (CloseFAR)
    {
//      CloseFAR=FALSE;
      /* $ 30.08.2001 IS
         При принудительном закрытии Фара пытаемся вести себя так же, как и при
         нажатии на F10 в панелях, только не запрашиваем подтверждение закрытия,
         если это возможно.
      */
      if(!Opt.CloseConsoleRule)
        FrameManager->IsAnyFrameModified(TRUE);
      else
        FrameManager->ExitMainLoop(FALSE);
      return KEY_NONE;
      /* IS $ */
    }

    if ((LoopCount & 15)==0)
    {
      clock_t CurTime=clock();
      clock_t TimeAfterExec=CurTime-StartExecTime;
      if (EnableShowTime)
        ShowTime(0);
      if (WaitInMainLoop)
      {
        if (Opt.InactivityExit && Opt.InactivityExitTime>0 &&
            CurTime-StartIdleTime>Opt.InactivityExitTime*60000 &&
            FrameManager->GetFrameCount()==1)
        {
          FrameManager->ExitMainLoop(FALSE);
          return(KEY_NONE);
        }
        if ((LoopCount & 63)==0)
        {
          static int Reenter=0;
          if (!Reenter)
          {
            Reenter++;
            int X,Y;
            GetRealCursorPos(X,Y);
            if (X==0 && Y==ScrY && CtrlObject->CmdLine->IsVisible())
            {
              while (1)
              {
                INPUT_RECORD tmprec;
                int Key=GetInputRecord(&tmprec);
                if ((DWORD)Key==KEY_NONE || (DWORD)Key!=KEY_SHIFT && tmprec.Event.KeyEvent.bKeyDown)
                  break;
              }
              CtrlObject->Cp()->SetScreenPosition();
              ScrBuf.ResetShadow();
              ScrBuf.Flush();
            }
            Reenter--;
          }
          static int UpdateReenter=0;
          if (!UpdateReenter && CurTime-KeyPressedLastTime>700)
          {
            UpdateReenter=TRUE;
            CtrlObject->Cp()->LeftPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
            CtrlObject->Cp()->RightPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
            UpdateReenter=FALSE;
          }
        }
      }
      if (StartExecTime!=0 && TimeAfterExec>2000)
      {
        StartExecTime=0;
        if (!IsWindowed() && !Opt.Mouse)
        {
          SetConsoleMode(hConInp,ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT);
          SetConsoleMode(hConInp,ENABLE_WINDOW_INPUT);
        }
        SetFarTitle(NULL);//LastFarTitle);
      }
      if (Opt.ScreenSaver && Opt.ScreenSaverTime>0 &&
          CurTime-StartIdleTime>Opt.ScreenSaverTime*60000)
        if (!ScreenSaver(WaitInMainLoop))
          return(KEY_NONE);
      if (!WaitInMainLoop && LoopCount==64)
      {
        LastEventIdle=TRUE;
        memset(rec,0,sizeof(*rec));
        rec->EventType=KEY_EVENT;
        return(KEY_IDLE);
      }
    }
    LoopCount++;
  } // while (1)

  clock_t CurClock=clock();

  if (rec->EventType==FOCUS_EVENT)
  {
    /* $ 28.04.2001 VVM
      + Не только обработаем сами смену фокуса, но и передадим дальше */
    ShiftPressed=RightShiftPressedLast=ShiftPressedLast=FALSE;
    CtrlPressed=CtrlPressedLast=RightCtrlPressedLast=FALSE;
    AltPressed=AltPressedLast=RightAltPressedLast=FALSE;
    LButtonPressed=RButtonPressed=MButtonPressed=FALSE;
    ShiftState=FALSE;
    PressedLastTime=0;
    ReadConsoleInput(hConInp,rec,1,&ReadCount);
    CalcKey=rec->Event.FocusEvent.bSetFocus?KEY_GOTFOCUS:KEY_KILLFOCUS;
    memset(rec,0,sizeof(*rec)); // Иначе в ProcessEditorInput такая херь приходит - волосы дыбом становятся
    rec->EventType=KEY_EVENT;
    return CalcKey;
    /* VVM $ */
  }

  if (rec->EventType==KEY_EVENT)
  {
    /* коррекция шифта, т.к.
    NumLock=ON Shift-Numpad1
       Dn, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000030 (caSa - cecN)
       Dn, 1, Vk=0x0023, Scan=0x004F Ctrl=0x00000020 (casa - cecN)
       Up, 1, Vk=0x0023, Scan=0x004F Ctrl=0x00000020 (casa - cecN)
    >>>Dn, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000030 (caSa - cecN)
       Up, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000020 (casa - cecN)
    винда вставляет лишний шифт
    */
/*
    if(rec->Event.KeyEvent.wVirtualKeyCode == VK_SHIFT)
    {
      if(rec->Event.KeyEvent.bKeyDown)
      {
        if(!ShiftState)
          ShiftState=TRUE;
        else // Здесь удалим из очереди... этот самый кривой шифт
        {
          INPUT_RECORD pinp;
          DWORD nread;
          ReadConsoleInput(hConInp, &pinp, 1, &nread);
          return KEY_NONE;
        }
      }
      else if(!rec->Event.KeyEvent.bKeyDown)
        ShiftState=FALSE;
    }

    if((rec->Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED) == 0 && ShiftState)
      rec->Event.KeyEvent.dwControlKeyState|=SHIFT_PRESSED;
*/
//_SVS(if(rec->EventType==KEY_EVENT)SysLog("%s",_INPUT_RECORD_Dump(rec)));

    DWORD CtrlState=rec->Event.KeyEvent.dwControlKeyState;

    /* $ 28.06.2001 SVS
       Для Win9x при нажатом NumLock и юзании курсорных клавиш
       получаем в диалоге назначения ерундистику.
    */
_SVS(if(rec->EventType==KEY_EVENT)SysLog("[%d] if(rec->EventType==KEY_EVENT) >>> %s",__LINE__,_INPUT_RECORD_Dump(rec)));
    if(CtrlObject && CtrlObject->Macro.IsRecording())
    {
      static WORD PrevVKKeyCode=0; // NumLock+Cursor
      WORD PrevVKKeyCode2=PrevVKKeyCode;
      PrevVKKeyCode=rec->Event.KeyEvent.wVirtualKeyCode;

      if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
        (CtrlState&NUMLOCK_ON))
      {
        if((PrevVKKeyCode2 >= 0x21 && PrevVKKeyCode2 <= 0x28 ||
            PrevVKKeyCode2 >= 0x2D && PrevVKKeyCode2 <= 0x2E) &&
           PrevVKKeyCode == VK_SHIFT && rec->Event.KeyEvent.bKeyDown
           ||
           (PrevVKKeyCode >= 0x21 && PrevVKKeyCode <= 0x28 ||
            PrevVKKeyCode >= 0x2D && PrevVKKeyCode <= 0x2E) &&
           PrevVKKeyCode2 == VK_SHIFT && !rec->Event.KeyEvent.bKeyDown
          )
        {
          if(PrevVKKeyCode2 != VK_SHIFT)
          {
            INPUT_RECORD pinp;
            DWORD nread;
            // Удалим из очереди...
            ReadConsoleInput(hConInp, &pinp, 1, &nread);
            return KEY_NONE;
          }
        }
      }
      /* 1.07.2001 KM
        При отпускании Shift-Enter в диалоге назначения
        вылазил Shift после отпускания клавиш.
      */
      if((PrevVKKeyCode2==VK_SHIFT && PrevVKKeyCode==VK_RETURN &&
          rec->Event.KeyEvent.bKeyDown) ||
          (PrevVKKeyCode2==VK_RETURN && PrevVKKeyCode==VK_SHIFT &&
          !rec->Event.KeyEvent.bKeyDown))
      {
        if(PrevVKKeyCode2 != VK_SHIFT)
        {
          INPUT_RECORD pinp;
          DWORD nread;
          // Удалим из очереди...
          ReadConsoleInput(hConInp, &pinp, 1, &nread);
          return KEY_NONE;
        }
      }
      /* KM $ */
    }
    /* SVS $ */

    if (AltPressed && (CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))==0)
      DetectWindowedMode();
    CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
    AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
    ShiftPressed=(CtrlState & SHIFT_PRESSED);
    RightCtrlPressed=(CtrlState & RIGHT_CTRL_PRESSED);
    RightAltPressed=(CtrlState & RIGHT_ALT_PRESSED);
    RightShiftPressed=(CtrlState & SHIFT_PRESSED); //???
    KeyPressedLastTime=CurClock;
    /* $ 24.08.2000 SVS
       + Добавление на реакцию KEY_CTRLALTSHIFTRELEASE
    */
    if(IsKeyCASPressed && (Opt.CASRule&1) && (!CtrlPressed || !AltPressed || !ShiftPressed))
    {
      IsKeyCASPressed=FALSE;
      return KEY_CTRLALTSHIFTRELEASE;
    }

    if(IsKeyRCASPressed && (Opt.CASRule&2) && (!RightCtrlPressed || !RightAltPressed || !ShiftPressed))
    {
      IsKeyRCASPressed=FALSE;
      return KEY_RCTRLALTSHIFTRELEASE;
    }
/* SVS $ */
  }

  _SVS(if(rec->EventType==KEY_EVENT)SysLog("[%d] if(rec->EventType==KEY_EVENT) >>> %s",__LINE__,_INPUT_RECORD_Dump(rec)));

  ReturnAltValue=FALSE;
  CalcKey=CalcKeyCode(rec,TRUE,&NotMacros);
/*
  if(CtrlObject && CtrlObject->Macro.IsRecording() && (CalcKey == (KEY_ALT|KEY_NUMPAD0) || CalcKey == (KEY_ALT|KEY_INS)))
  {
    if(CtrlObject->Macro.ProcessKey(CalcKey))
    {
      RunGraber();
      rec->EventType=0;
      CalcKey=KEY_NONE;
    }
    return(CalcKey);
  }
*/
  _SVS(SysLog("1) CalcKey=%s",_FARKEY_ToName(CalcKey)));
  if (ReturnAltValue && !NotMacros)
  {
    _KEYMACRO(CleverSysLog Clev("CALL(2) CtrlObject->Macro.ProcessKey()"));
    if (CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(CalcKey))
    {
      rec->EventType=0;
      CalcKey=KEY_NONE;
    }
    return(CalcKey);
  }
  int GrayKey=(CalcKey==KEY_ADD || CalcKey==KEY_SUBTRACT || CalcKey==KEY_MULTIPLY);
  if ((CalcKey>=' ' && CalcKey<256 || CalcKey==KEY_BS || GrayKey) &&
      CalcKey!=KEY_DEL && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
  {
#if defined(USE_WFUNC_IN)
    if(Opt.UseUnicodeConsole)
    {
      ReadConsoleW(hConInp,&ReadKey,1,&ReadCount,NULL);

      //????????????????????????????????????????
      WCHAR UnicodeChar=ReadKey;
      UnicodeToOEM(&UnicodeChar,(char*)&ReadKey,sizeof(WCHAR));
      //????????????????????????????????????????
    }
    else
      ReadConsoleA(hConInp,&ReadKey,1,&ReadCount,NULL);
#else
    ReadConsole(hConInp,&ReadKey,1,&ReadCount,NULL);
#endif

    if (ReadKey==13 && CalcKey!=KEY_ENTER)
    {
#if defined(USE_WFUNC_IN)
      if(Opt.UseUnicodeConsole)
      {
        ReadConsoleW(hConInp,&ReadKey,1,&ReadCount,NULL);
        //????????????????????????????????????????
        WCHAR UnicodeChar=ReadKey;
        UnicodeToOEM(&UnicodeChar,(char*)&ReadKey,sizeof(WCHAR));
        //????????????????????????????????????????
      }
      else
        ReadConsoleA(hConInp,&ReadKey,1,&ReadCount,NULL);
#else
      ReadConsole(hConInp,&ReadKey,1,&ReadCount,NULL);
#endif
    }
    rec->Event.KeyEvent.uChar.AsciiChar=(char) ReadKey;
  }
  else
  {
#if defined(USE_WFUNC_IN)
    if(Opt.UseUnicodeConsole) //????????????????????????????????????????
    {
      ReadConsoleInputW(hConInp,rec,1,&ReadCount);
      if(rec->EventType==KEY_EVENT)
      {
        WCHAR UnicodeChar=rec->Event.KeyEvent.uChar.UnicodeChar;
        rec->Event.KeyEvent.uChar.UnicodeChar=0;
        UnicodeToOEM(&UnicodeChar,&rec->Event.KeyEvent.uChar.AsciiChar,sizeof(WCHAR));
      }
    }
    else
      ReadConsoleInputA(hConInp,rec,1,&ReadCount);
#else
    ReadConsoleInput(hConInp,rec,1,&ReadCount);

    // Эта фигня нужна только в диалоге назначения макро - остальное по барабану - и так работает
    // ... иначе хреновень с эфектом залипшего шифта проскакивает
    /* коррекция шифта, т.к.
    NumLock=ON Shift-Numpad1
       Dn, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000030 (caSa - cecN)
       Dn, 1, Vk=0x0023, Scan=0x004F Ctrl=0x00000020 (casa - cecN)
       Up, 1, Vk=0x0023, Scan=0x004F Ctrl=0x00000020 (casa - cecN)
    >>>Dn, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000030 (caSa - cecN)
       Up, 1, Vk=0x0010, Scan=0x002A Ctrl=0x00000020 (casa - cecN)
    винда вставляет лишний шифт
    */
    if(rec->Event.KeyEvent.wVirtualKeyCode == VK_SHIFT &&
       rec->EventType==KEY_EVENT &&
       IsProcessAssignMacroKey)
    {
      if(rec->Event.KeyEvent.bKeyDown)
      {
        if(!ShiftState)
          ShiftState=TRUE;
      }
      else if(!rec->Event.KeyEvent.bKeyDown)
        ShiftState=FALSE;
    }
#endif
  }


  if (EnableShowTime)
    ShowTime(1);

  /*& 17.05.2001 OT Изменился размер консоли, генерим клавишу*/
  if (rec->EventType==WINDOW_BUFFER_SIZE_EVENT)
  {
#if defined(DETECT_ALT_ENTER)
    _SVS(CleverSysLog Clev(""));
    _SVS(SysLog("ScrX=%d (%d) ScrY=%d (%d), AltEnter=%d",ScrX,PrevScrX,ScrY,PrevScrY,AltEnter));
    _SVS(SysLog("FarAltEnter -> %d",FarAltEnter(FAR_CONSOLE_GET_MODE)));
#endif
    int PScrX=ScrX;
    int PScrY=ScrY;
    //// // _SVS(SysLog(1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT)"));
    Sleep(1);
    GetVideoMode(CurScreenBufferInfo);
#if defined(DETECT_ALT_ENTER)
//    if((PScrX == PrevScrX && PScrY == PrevScrY) && PrevFarAltEnterMode == FarAltEnter(FAR_CONSOLE_GET_MODE))
    if (PScrX+1 == CurScreenBufferInfo.dwSize.X &&
        PScrY+1 == CurScreenBufferInfo.dwSize.Y &&
        PScrX+1 <= CurScreenBufferInfo.dwMaximumWindowSize.X &&
        PScrY+1 <= CurScreenBufferInfo.dwMaximumWindowSize.Y
        && PrevFarAltEnterMode == FarAltEnter(FAR_CONSOLE_GET_MODE)
      )
#else
    if (PScrX+1 == CurScreenBufferInfo.dwSize.X &&
        PScrY+1 == CurScreenBufferInfo.dwSize.Y)
#endif
    {
#if defined(DETECT_ALT_ENTER)
      _SVS(SysLog("return KEY_NONE"));
#endif
      return KEY_NONE;
    }
    else
    {
#if defined(DETECT_ALT_ENTER)
      _SVS(SysLog("return KEY_CONSOLE_BUFFER_RESIZE ScrX=%d (%d) ScrY=%d (%d)",ScrX,PrevScrX,ScrY,PrevScrY));
      if(FarAltEnter(FAR_CONSOLE_GET_MODE) == FAR_CONSOLE_FULLSCREEN)
      {
        _SVS(SysLog("call ChangeVideoMode"));
        PrevFarAltEnterMode=FarAltEnter(FAR_CONSOLE_GET_MODE);
        ChangeVideoMode(PScrY==24?50:25,80);
        GetVideoMode(CurScreenBufferInfo);
      }
      else
      {
        _SVS(SysLog("PrevScrX=PScrX"));
        PrevScrX=PScrX;
        PrevScrY=PScrY;
      }
#else
      PrevScrX=PScrX;
      PrevScrY=PScrY;
      //// // _SVS(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_CONSOLE_BUFFER_RESIZE"));
#endif
      Sleep(1);
      if(FrameManager)
      {
#if defined(DETECT_ALT_ENTER)
        _SVS(SysLog("if(FrameManager)"));
#endif
        // апдейтим панели (именно они сейчас!)
        LockScreen LckScr;
        if(GlobalSaveScrPtr)
          GlobalSaveScrPtr->Discard();
        FrameManager->ResizeAllFrame();
        FrameManager->GetCurrentFrame()->Show();
        //// // _SVS(SysLog("PreRedrawFunc = %p",PreRedrawFunc));
        if(PreRedrawFunc)
        {
          PreRedrawFunc();
        }
      }
      return(KEY_CONSOLE_BUFFER_RESIZE);
    }
  }
  /* 17.05.2001 $ */

  if (rec->EventType==KEY_EVENT)
  {
    DWORD CtrlState=rec->Event.KeyEvent.dwControlKeyState;
    DWORD KeyCode=rec->Event.KeyEvent.wVirtualKeyCode;
    CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
    AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
    RightCtrlPressed=(CtrlState & RIGHT_CTRL_PRESSED);
    RightAltPressed=(CtrlState & RIGHT_ALT_PRESSED);

    // Для NumPad!
    if((CalcKey&(KEY_CTRL|KEY_SHIFT|KEY_ALT|KEY_RCTRL|KEY_RALT)) == KEY_SHIFT &&
       (CalcKey&KEY_MASKF) >= KEY_NUMPAD0 && (CalcKey&KEY_MASKF) <= KEY_NUMPAD9)
      ShiftPressed=SHIFT_PRESSED;
    else
      ShiftPressed=(CtrlState & SHIFT_PRESSED);

    if (KeyCode==VK_F16 && ReadKey==VK_F16 || KeyCode==0)
      return(KEY_NONE);

    if (!rec->Event.KeyEvent.bKeyDown &&
        (KeyCode==VK_SHIFT || KeyCode==VK_CONTROL || KeyCode==VK_MENU) &&
        CurClock-PressedLastTime<500)
    {
      int Key=-1;
      if (ShiftPressedLast && KeyCode==VK_SHIFT)
      {
        if (ShiftPressedLast)
        {
          Key=KEY_SHIFT;
          //// // _SVS(SysLog("ShiftPressedLast, Key=KEY_SHIFT"));
        }
        else if (RightShiftPressedLast)
        {
          Key=KEY_RSHIFT;
          //// // _SVS(SysLog("RightShiftPressedLast, Key=KEY_RSHIFT"));
        }
      }
      if (KeyCode==VK_CONTROL)
      {
        if (CtrlPressedLast)
        {
          Key=KEY_CTRL;
          //// // _SVS(SysLog("CtrlPressedLast, Key=KEY_CTRL"));
        }
        else if (RightCtrlPressedLast)
        {
          Key=KEY_RCTRL;
          //// // _SVS(SysLog("CtrlPressedLast, Key=KEY_RCTRL"));
        }
      }

      if (KeyCode==VK_MENU)
      {
        if (AltPressedLast)
        {
          Key=KEY_ALT;
          //// // _SVS(SysLog("AltPressedLast, Key=KEY_ALT"));
        }
        else if (RightAltPressedLast)
        {
          Key=KEY_RALT;
          //// // _SVS(SysLog("RightAltPressedLast, Key=KEY_RALT"));
        }
      }

      {
        _KEYMACRO(CleverSysLog Clev("CALL(3) CtrlObject->Macro.ProcessKey()"));
        if (Key!=-1 && !NotMacros && CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(Key))
        {
          rec->EventType=0;
          Key=KEY_NONE;
        }
      }
      if (Key!=-1)
        return(Key);
    }

    ShiftPressedLast=RightShiftPressedLast=FALSE;
    CtrlPressedLast=RightCtrlPressedLast=FALSE;
    AltPressedLast=RightAltPressedLast=FALSE;

    ShiftPressedLast=(KeyCode==VK_SHIFT && rec->Event.KeyEvent.bKeyDown) ||
         (KeyCode==VK_RETURN && ShiftPressed && !rec->Event.KeyEvent.bKeyDown);

    if (!ShiftPressedLast)
      if (KeyCode==VK_CONTROL && rec->Event.KeyEvent.bKeyDown)
      {
        if (CtrlState & RIGHT_CTRL_PRESSED)
        {
          RightCtrlPressedLast=TRUE;
          //// // _SVS(SysLog("RightCtrlPressedLast=TRUE;"));
        }
        else
        {
          CtrlPressedLast=TRUE;
          //// // _SVS(SysLog("CtrlPressedLast=TRUE;"));
        }
      }

    if (!ShiftPressedLast && !CtrlPressedLast && !RightCtrlPressedLast)
    {
      if (KeyCode==VK_MENU && rec->Event.KeyEvent.bKeyDown)
      {
        if (CtrlState & RIGHT_ALT_PRESSED)
        {
          RightAltPressedLast=TRUE;
        }
        else
        {
          AltPressedLast=TRUE;
        }
        PressedLastTime=CurClock;
      }
    }
    else
      PressedLastTime=CurClock;

    if (KeyCode==VK_SHIFT || KeyCode==VK_MENU || KeyCode==VK_CONTROL ||
        KeyCode==VK_NUMLOCK || KeyCode==VK_SCROLL)
    {
      /* $ 24.08.2000 SVS
         + Добавление на реакцию KEY_CTRLALTSHIFTPRESS
      */
      switch(KeyCode)
      {
        case VK_SHIFT:
        case VK_MENU:
        case VK_CONTROL:
          if(!IsKeyCASPressed && CtrlPressed && AltPressed && ShiftPressed)
          {
            if(!IsKeyRCASPressed && RightCtrlPressed && RightAltPressed && RightShiftPressed)
            {
              if(Opt.CASRule&2)
              {
                IsKeyRCASPressed=TRUE;
                return (KEY_RCTRLALTSHIFTPRESS);
              }
            }
            else if(Opt.CASRule&1)
            {
              IsKeyCASPressed=TRUE;
              return (KEY_CTRLALTSHIFTPRESS);
            }
          }
          break;
        case VK_LSHIFT:
        case VK_LMENU:
        case VK_LCONTROL:
          if(!IsKeyRCASPressed && RightCtrlPressed && RightAltPressed && RightShiftPressed)
          {
            if((Opt.CASRule&2))
            {
              IsKeyRCASPressed=TRUE;
              return (KEY_RCTRLALTSHIFTPRESS);
            }
          }
          break;
      }
      /* SVS $ */
      return(KEY_NONE);
    }
    Panel::EndDrag();
  }

  if (rec->EventType==MOUSE_EVENT)
  {
    // проверка на Swap клавиш мыши
    static int SwapButton=GetSystemMetrics(SM_SWAPBUTTON);
#if defined(MOUSEKEY)
    PrePreMouseEventFlags=PreMouseEventFlags;
#endif
    PreMouseEventFlags=MouseEventFlags;
    MouseEventFlags=rec->Event.MouseEvent.dwEventFlags;

    DWORD CtrlState=rec->Event.MouseEvent.dwControlKeyState;

/*
    // Сигнал на прорисовку ;-) Помогает прорисовать кейбар при движении мышью
    if(CtrlState != (CtrlPressed|AltPressed|ShiftPressed))
    {
      static INPUT_RECORD TempRec[2]={
        {KEY_EVENT,{1,1,VK_F16,0,{0},0}},
        {KEY_EVENT,{0,1,VK_F16,0,{0},0}}
      };
      DWORD WriteCount;
      TempRec[0].Event.KeyEvent.dwControlKeyState=TempRec[1].Event.KeyEvent.dwControlKeyState=CtrlState;
      #if defined(USE_WFUNC_IN)
      if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
        WriteConsoleInputW(hConInp,TempRec,2,&WriteCount);
      else
        WriteConsoleInputA(hConInp,TempRec,2,&WriteCount);
      #else
      WriteConsoleInput(hConInp,TempRec,2,&WriteCount);
      #endif
    }
*/
    CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
    AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
    ShiftPressed=(CtrlState & SHIFT_PRESSED);
    RightCtrlPressed=(CtrlState & RIGHT_CTRL_PRESSED);
    RightAltPressed=(CtrlState & RIGHT_ALT_PRESSED);
    RightShiftPressed=(CtrlState & SHIFT_PRESSED);

    DWORD BtnState=rec->Event.MouseEvent.dwButtonState;
    if (SwapButton && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && !IsWindowed())
    {
      if (BtnState & FROM_LEFT_1ST_BUTTON_PRESSED)
        rec->Event.MouseEvent.dwButtonState|=RIGHTMOST_BUTTON_PRESSED;
      else
        rec->Event.MouseEvent.dwButtonState&=~RIGHTMOST_BUTTON_PRESSED;
      if (BtnState & RIGHTMOST_BUTTON_PRESSED)
        rec->Event.MouseEvent.dwButtonState|=FROM_LEFT_1ST_BUTTON_PRESSED;
      else
        rec->Event.MouseEvent.dwButtonState&=~FROM_LEFT_1ST_BUTTON_PRESSED;
    }

    if(MouseEventFlags != MOUSE_MOVED)
    {
//// // _SVS(SysLog("1. CtrlState=%X PrevRButtonPressed=%d,RButtonPressed=%d",CtrlState,PrevRButtonPressed,RButtonPressed));
      PrevLButtonPressed=LButtonPressed;
      PrevRButtonPressed=RButtonPressed;
      PrevMButtonPressed=MButtonPressed;
    }

    LButtonPressed=(BtnState & FROM_LEFT_1ST_BUTTON_PRESSED);
    RButtonPressed=(BtnState & RIGHTMOST_BUTTON_PRESSED);
    MButtonPressed=(BtnState & FROM_LEFT_2ND_BUTTON_PRESSED);
//// // _SVS(SysLog("2. BtnState=%X PrevRButtonPressed=%d,RButtonPressed=%d",BtnState,PrevRButtonPressed,RButtonPressed));

    PrevMouseX=MouseX;
    PrevMouseY=MouseY;
    MouseX=rec->Event.MouseEvent.dwMousePosition.X;
    MouseY=rec->Event.MouseEvent.dwMousePosition.Y;

#if defined(MOUSEKEY)
    if(PrePreMouseEventFlags == DOUBLE_CLICK)
    {
      memset(rec,0,sizeof(*rec)); // Иначе в ProcessEditorInput такая херь приходит - волосы дыбом становятся
      rec->EventType = KEY_EVENT;
      return(KEY_NONE);
    }
    if (MouseEventFlags == DOUBLE_CLICK && (LButtonPressed || RButtonPressed))
    {
      CalcKey=LButtonPressed?KEY_MSLDBLCLICK:KEY_MSRDBLCLICK;
      CalcKey |= (CtrlState&SHIFT_PRESSED?KEY_SHIFT:0)|
                 (CtrlState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)?KEY_CTRL:0)|
                 (CtrlState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)?KEY_ALT:0);
      memset(rec,0,sizeof(*rec)); // Иначе в ProcessEditorInput такая херь приходит - волосы дыбом становятся
      rec->EventType = KEY_EVENT;
    }
    else
#endif
    /* $ 26.04.2001 VVM
       + Обработка колесика мышки под 2000. */
    if (MouseEventFlags == MOUSE_WHEELED)
    { // Обработаем колесо и заменим на спец.клавиши
      short zDelta = (short)HIWORD(rec->Event.MouseEvent.dwButtonState);
      CalcKey = (zDelta>0)?KEY_MSWHEEL_UP:KEY_MSWHEEL_DOWN;
      /* $ 27.04.2001 SVS
         Не были учтены шифтовые клавиши при прокрутке колеса, из-за чего
         нельзя было использовать в макросах нечто вроде "ShiftMsWheelUp"
      */
      CalcKey |= (CtrlState&SHIFT_PRESSED?KEY_SHIFT:0)|
                 (CtrlState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)?KEY_CTRL:0)|
                 (CtrlState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)?KEY_ALT:0);
      /* SVS $ */
      /* $ 14.05.2002 VVM
        - сбросим тип евента вообще. Иначе бывают глюки (ProcessEditorInput) */
      memset(rec,0,sizeof(*rec)); // Иначе в ProcessEditorInput такая херь приходит - волосы дыбом становятся
      rec->EventType = KEY_EVENT;
      /* VVM $ */
    } /* if */
    /* VVM $ */
  }
  if (ReadKey!=0 && !GrayKey)
    CalcKey=ReadKey;

  {
    _KEYMACRO(CleverSysLog Clev("CALL(1) CtrlObject->Macro.ProcessKey()"));
    if (!NotMacros && CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(CalcKey))
    {
      rec->EventType=0;
      CalcKey=KEY_NONE;
    }
  }

  return(CalcKey);
}

DWORD PeekInputRecord(INPUT_RECORD *rec)
{
  DWORD ReadCount;
  DWORD Key;
  ScrBuf.Flush();
  if(KeyQueue && (Key=KeyQueue->Peek()) != 0)
  {
    int VirtKey,ControlState;
    ReadCount=TranslateKeyToVK(Key,VirtKey,ControlState,rec)?1:0;
  }
  else
  {
#if defined(USE_WFUNC_IN)
    if(Opt.UseUnicodeConsole)
    {
      PeekConsoleInputW(hConInp,rec,1,&ReadCount);
      if(rec->EventType==KEY_EVENT)
      {
        WCHAR UnicodeChar=rec->Event.KeyEvent.uChar.UnicodeChar;
        rec->Event.KeyEvent.uChar.UnicodeChar=0;
        UnicodeToOEM(&UnicodeChar,&rec->Event.KeyEvent.uChar.AsciiChar,sizeof(WCHAR));
      }
    }
    else
      PeekConsoleInputA(hConInp,rec,1,&ReadCount);
#else
    PeekConsoleInput(hConInp,rec,1,&ReadCount);
#endif
  }
  if (ReadCount==0)
    return(0);
  return(CalcKeyCode(rec,TRUE));
}


/* $ 24.08.2000 SVS
 + Пераметр у фунции WaitKey - возможность ожидать конкретную клавишу
     Если KeyWait = -1 - как и раньше
*/
DWORD WaitKey(DWORD KeyWait)
{
  int Visible=0,Size=0;
  if(KeyWait == KEY_CTRLALTSHIFTRELEASE || KeyWait == KEY_RCTRLALTSHIFTRELEASE)
  {
    GetCursorType(Visible,Size);
    SetCursorType(0,10);
  }

  DWORD Key;
  while (1)
  {
    INPUT_RECORD rec;
    Key=GetInputRecord(&rec);
    if(KeyWait == (DWORD)-1)
    {
      if (Key!=KEY_NONE && Key!=KEY_IDLE)
        break;
    }
    else if(Key == KeyWait)
      break;
  }

  if(KeyWait == KEY_CTRLALTSHIFTRELEASE || KeyWait == KEY_RCTRLALTSHIFTRELEASE)
    SetCursorType(Visible,Size);

  return Key;
}
/* SVS $ */

int WriteInput(int Key,DWORD Flags)
{
  if(Flags&(SKEY_VK_KEYS|SKEY_IDLE))
  {
    INPUT_RECORD Rec;
    DWORD WriteCount;

    if(Flags&SKEY_IDLE)
    {
      Rec.EventType=FOCUS_EVENT;
      Rec.Event.FocusEvent.bSetFocus=TRUE;
    }
    else
    {
      Rec.EventType=KEY_EVENT;
      Rec.Event.KeyEvent.bKeyDown=1;
      Rec.Event.KeyEvent.wRepeatCount=1;
      Rec.Event.KeyEvent.wVirtualKeyCode=Key;
      Rec.Event.KeyEvent.wVirtualScanCode=MapVirtualKey(
                    Rec.Event.KeyEvent.wVirtualKeyCode, 0);
      if (Key < 0x30 || Key > 0x5A) // 0-9:;<=>?@@ A..Z  //?????
        Key=0;
      Rec.Event.KeyEvent.uChar.UnicodeChar=Rec.Event.KeyEvent.uChar.AsciiChar=Key;
      Rec.Event.KeyEvent.dwControlKeyState=0;
    }
#if defined(USE_WFUNC_IN)
    if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
      return WriteConsoleInputW(hConInp,&Rec,1,&WriteCount);
    else
      return WriteConsoleInputA(hConInp,&Rec,1,&WriteCount);
#else
    return WriteConsoleInput(hConInp,&Rec,1,&WriteCount);
#endif
  }
  else if(KeyQueue)
  {
    return KeyQueue->Put(((DWORD)Key)|(Flags&SKEY_NOTMACROS?0x80000000:0));
  }
  else
    return 0;
}


int CheckForEscSilent()
{
  INPUT_RECORD rec;
  int Key;
  BOOL Processed=TRUE;

  /* TODO: Здесь, в общем то - ХЗ, т.к.
           по хорошему нужно проверять CtrlObject->Macro.PeekKey() на ESC или BREAK
           Но к чему это приведет - пока не могу дать ответ !!!
  */
  // если в "макросе"...
  if(CtrlObject->Macro.IsExecuting() != MACROMODE_NOMACRO && FrameManager->GetCurrentFrame())
  {
    // ...но ЭТО конец последовательности...
    if(CtrlObject->Macro.IsExecutingLastKey() && CtrlObject->Macro.PeekKey() == KEY_NONE)
      CtrlObject->Macro.GetKey(); // ...то "завершим" макрос
    else
      Processed=FALSE;
  }


  if (Processed && PeekInputRecord(&rec))
  {
    int MMode=CtrlObject->Macro.GetMode();
    CtrlObject->Macro.SetMode(MACRO_LAST);
    Key=GetInputRecord(&rec);
    CtrlObject->Macro.SetMode(MMode);
    /*
    if(Key == KEY_CONSOLE_BUFFER_RESIZE)
    {
      // апдейтим панели (именно они сейчас!)
      LockScreen LckScr;
      FrameManager->ResizeAllFrame();
      FrameManager->GetCurrentFrame()->Show();
      if(PreRedrawFunc)
        PreRedrawFunc();

    }
    else
    */
    if(Key==KEY_ESC || Key==KEY_BREAK)
      return(TRUE);
  }
  return(FALSE);
}

int ConfirmAbortOp()
{
//  SaveScreen SaveScr; // НУЖЕН! Избавляет от некоторых подводных багов
  BOOL rc=TRUE;
  IsProcessAssignMacroKey++; // запретим спец клавиши
                             // т.е. в этом диалоге нельзя нажать Alt-F9!
  if (Opt.Confirm.Esc)
    rc=AbortMessage();
  IsProcessAssignMacroKey--;
  return rc;
}

/* $ 09.02.2001 IS
     Подтверждение нажатия Esc
*/
int CheckForEsc()
{
  if (CheckForEscSilent())
    return(ConfirmAbortOp());
  else
    return(FALSE);
}
/* IS $ */

/* $ 25.07.2000 SVS
    ! Функция KeyToText сделана самосотоятельной - вошла в состав FSF
*/
/* $ 01.08.2000 SVS
   ! дополнительный параметра у KeyToText - размер данных
   Size=0 - по максимуму!
*/
/* $ 10.09.2000 SVS
  ! KeyToText возвращает BOOL
*/

static char *GetShiftKeyName(char *Name, DWORD Key,int& Len)
{
  if((Key&KEY_RCTRL) == KEY_RCTRL)   strcat(Name,ModifKeyName[0].Name);
  else if(Key&KEY_CTRL)              strcat(Name,ModifKeyName[2].Name);
//  else if(Key&KEY_LCTRL)             strcat(Name,ModifKeyName[3].Name);

  if((Key&KEY_RALT) == KEY_RALT)     strcat(Name,ModifKeyName[3].Name);
  else if(Key&KEY_ALT)               strcat(Name,ModifKeyName[4].Name);
//  else if(Key&KEY_LALT)    strcat(Name,ModifKeyName[6].Name);

  if(Key&KEY_SHIFT)                  strcat(Name,ModifKeyName[1].Name);
//  else if(Key&KEY_LSHIFT)  strcat(Name,ModifKeyName[0].Name);
//  else if(Key&KEY_RSHIFT)  strcat(Name,ModifKeyName[1].Name);
  if(Key&KEY_M_SPEC)                 strcat(Name,ModifKeyName[5].Name);
  else if(Key&KEY_M_OEM)             strcat(Name,ModifKeyName[6].Name);

  Len=strlen(Name);
  return Name;
}

/* $ 24.09.2000 SVS
 + Функция KeyNameToKey - получение кода клавиши по имени
   Если имя не верно или нет такого - возвращается -1
   Может и криво, но правильно и коротко!
*/
int WINAPI KeyNameToKey(const char *Name)
{
   if(!Name || !*Name)
     return -1;
   DWORD Key=0;

//// // _SVS(SysLog("KeyNameToKey('%s')",Name));

   // Это макроклавиша?
   if(Name[0] == '$' && Name[1])
     return KeyNameMacroToKey(Name);
//   if((Key=KeyNameMacroToKey(Name)) != (DWORD)-1)
//     return Key;

   int I, Pos, Len=strlen(Name);
   char TmpName[128];
   xstrncpy(TmpName,Name,sizeof(TmpName)-1);

   // пройдемся по всем модификаторам
   for(Pos=I=0; I < sizeof(ModifKeyName)/sizeof(ModifKeyName[0]); ++I)
   {
     if(LocalStrstri(TmpName,ModifKeyName[I].Name) && !(Key&ModifKeyName[I].Key))
     {
       ReplaceStrings(TmpName,ModifKeyName[I].Name,"",-1,TRUE);
       Key|=ModifKeyName[I].Key;
       Pos+=ModifKeyName[I].Len;
     }
   }
   //Pos=strlen(TmpName);
//// // _SVS(SysLog("Name=%s",Name));
   // если что-то осталось - преобразуем.
   if(Pos < Len)
   {
     // сначала - FKeys1
     const char* Ptr=Name+Pos;
     for (I=0;I<sizeof(FKeys1)/sizeof(FKeys1[0]);I++)
       if (!memicmp(Ptr,FKeys1[I].Name,FKeys1[I].Len))
       {
         Key|=FKeys1[I].Key;
         Pos+=FKeys1[I].Len;
         break;
       }
     if(I  == sizeof(FKeys1)/sizeof(FKeys1[0]))
     {
       if(Len == 1 || Pos == Len-1)
       {
         WORD Chr=(WORD)(BYTE)Name[Pos];
         if (Chr > 0 && Chr < 256)
         {
           if (Key&(KEY_ALT|KEY_RCTRL|KEY_CTRL|KEY_RALT))
           {
             if(Chr > 0x7F)
                Chr=LocalKeyToKey(Chr);
             Chr=LocalUpper(Chr);
           }
           Key|=Chr;
           if(Chr)
             Pos++;
         }
       }
       else if(Key & (KEY_ALT|KEY_RALT))
       {
         int K=atoi(Ptr);
         if(K != -1)
         {
           // Было введение Alt-Num
           Key=(Key|K|KEY_ALTDIGIT)&(~(KEY_ALT|KEY_RALT));
           Pos=Len;
         }
       }
       else if(Key & (KEY_M_SPEC|KEY_M_OEM))
       {
         int K=atoi(Ptr);
         if(K != -1)
         {
           if(Key & KEY_M_SPEC)
             Key=(Key|(K+KEY_VK_0xFF_BEGIN))&(~(KEY_M_SPEC|KEY_M_OEM));
           else if(Key & KEY_M_OEM)
             Key=(Key|(K+KEY_FKEY_BEGIN))&(~(KEY_M_SPEC|KEY_M_OEM));
           Pos=Len;
         }
       }
     }
   }
/*
   if(!(Key&(KEY_ALT|KEY_RCTRL|KEY_CTRL|KEY_RALT|KEY_ALTDIGIT)) && (Key&KEY_SHIFT) && LocalIsalpha(Key&(~KEY_CTRLMASK)))
   {
     Key&=~KEY_SHIFT;
     Key=LocalUpper(Key);
   }
*/
//// // _SVS(SysLog("Key=0x%08X (%c) => '%s'",Key,(Key?Key:' '),Name));

   return (!Key || Pos < Len)? -1: Key;
}
/* SVS $*/

BOOL WINAPI KeyToText(int Key0,char *KeyText0,int Size)
{
  if(!KeyText0)
     return FALSE;

  char KeyText[256];
  int I, Len;
  DWORD Key=(DWORD)Key0, FKey=(DWORD)Key0&0xFFFF;

  if(Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE)
    return KeyMacroToText(Key0,KeyText0,Size);

  if(Key&KEY_ALTDIGIT)
  {
    sprintf(KeyText,"Alt%05d",Key&FKey);
  }
  else
  {
    /* $ 27.12.2001 KM
      ! Обнулим KeyText (как и было раньше), в противном случае
        в буфере возвращался мусор!
    */
    memset(KeyText,0,sizeof(KeyText));
    /* KM $ */

    GetShiftKeyName(KeyText,Key,Len);

    for (I=0;I<sizeof(FKeys1)/sizeof(FKeys1[0]);I++)
    {
      if (FKey==FKeys1[I].Key)
      {
        strcat(KeyText,FKeys1[I].Name);
        break;
      }
    }

    if(I  == sizeof(FKeys1)/sizeof(FKeys1[0]))
    {
      if(FKey >= KEY_VK_0xFF_BEGIN && FKey <= KEY_VK_0xFF_END)
      {
        sprintf(KeyText+strlen(KeyText),"Spec%05d",FKey-KEY_VK_0xFF_BEGIN);
      }
      else if(FKey > KEY_LAUNCH_APP2 && FKey < KEY_CTRLALTSHIFTPRESS)
      {
        sprintf(KeyText+strlen(KeyText),"Oem%05d",FKey-KEY_FKEY_BEGIN);
      }
      else
      {
    #if defined(SYSLOG)
        for (I=0;I<sizeof(SpecKeyName)/sizeof(SpecKeyName[0]);I++)
          if (FKey==SpecKeyName[I].Key)
          {
            strcat(KeyText,SpecKeyName[I].Name);
            break;
          }
        if(I  == sizeof(SpecKeyName)/sizeof(SpecKeyName[0]))
    #endif
        {
          FKey=(Key&0xFF)&(~0x20);
          if (FKey >= 'A' && FKey <= 'Z')
            KeyText[Len]=(char)(Key&0xFF)&((Key&(KEY_RCTRL|KEY_CTRL|KEY_ALT|KEY_RCTRL))?(~0x20):0xFF);
          else if ((Key&0xFF) > 0 && (Key&0xFF) < 256)
            KeyText[Len]=(char)Key&0xFF;
        }
      }
    }
    if(!KeyText[0])
    {
      *KeyText0='\0';
      return FALSE;
    }
  }

  if(Size > 0)
    xstrncpy(KeyText0,KeyText,Size);
  else
    strcpy(KeyText0,KeyText);
//_D(SysLog("KeyToText() 0x%08X %s",Key,KeyText));
  return TRUE;
}
/* SVS 10.09.2000 $ */
/* SVS $ */


int TranslateKeyToVK(int Key,int &VirtKey,int &ControlState,INPUT_RECORD *Rec)
{
  int FKey  =Key&0x0000FFFF;
  int FShift=Key&0x7F000000; // старший бит используется в других целях!
  int I;

  VirtKey=0;
  ControlState=(FShift&KEY_SHIFT?PKF_SHIFT:0)|
               (FShift&KEY_ALT?PKF_ALT:0)|
               (FShift&KEY_CTRL?PKF_CONTROL:0);

  for(I=0; I < sizeof(Table_KeyToVK)/sizeof(Table_KeyToVK[0]); ++I)
    if (FKey==Table_KeyToVK[I].Key)
    {
      VirtKey=Table_KeyToVK[I].VK;
      break;
    }

  if(I  == sizeof(Table_KeyToVK)/sizeof(Table_KeyToVK[0]))
  {
    if (FKey>='0' && FKey<='9' || FKey>='A' && FKey<='Z')
      VirtKey=FKey;
    else if(FKey > 0x100 && FKey < KEY_END_FKEY)
      VirtKey=FKey-0x100;
    else if(FKey < 0x100)
      VirtKey=VkKeyScan(FKey)&0xFF;
    else
      VirtKey=FKey;
  }
  if(Rec && VirtKey!=0)
  {
    Rec->EventType=KEY_EVENT;
    Rec->Event.KeyEvent.bKeyDown=1;
    Rec->Event.KeyEvent.wRepeatCount=1;
    Rec->Event.KeyEvent.wVirtualKeyCode=VirtKey;
    Rec->Event.KeyEvent.wVirtualScanCode = MapVirtualKey(
                    Rec->Event.KeyEvent.wVirtualKeyCode, 0);
    if (Key>255)
      Key=0;
    Rec->Event.KeyEvent.uChar.UnicodeChar=
        Rec->Event.KeyEvent.uChar.AsciiChar=Key;
    // здесь подход к Shift-клавишам другой, нежели для ControlState
    Rec->Event.KeyEvent.dwControlKeyState=
               (FShift&KEY_SHIFT?SHIFT_PRESSED:0)|
               (FShift&KEY_ALT?LEFT_ALT_PRESSED:0)|
               (FShift&KEY_CTRL?LEFT_CTRL_PRESSED:0)|
               (FShift&KEY_RALT?RIGHT_ALT_PRESSED:0)|
               (FShift&KEY_RCTRL?RIGHT_CTRL_PRESSED:0);
  }
  return(VirtKey!=0);
}


int IsNavKey(DWORD Key)
{
  static DWORD NavKeys[][2]={
    {0,KEY_CTRLC},
    {0,KEY_INS},      {0,KEY_NUMPAD0},
    {0,KEY_CTRLINS},  {0,KEY_CTRLNUMPAD0},

    {1,KEY_LEFT},     {1,KEY_NUMPAD4},
    {1,KEY_RIGHT},    {1,KEY_NUMPAD6},
    {1,KEY_HOME},     {1,KEY_NUMPAD7},
    {1,KEY_END},      {1,KEY_NUMPAD1},
    {1,KEY_UP},       {1,KEY_NUMPAD8},
    {1,KEY_DOWN},     {1,KEY_NUMPAD2},
    {1,KEY_PGUP},     {1,KEY_NUMPAD9},
    {1,KEY_PGDN},     {1,KEY_NUMPAD3},
    //!!!!!!!!!!!
  };

  for (int I=0; I < sizeof(NavKeys)/sizeof(NavKeys[0]); I++)
    if(!NavKeys[I][0] && Key==NavKeys[I][1] ||
       NavKeys[I][0] && (Key&0x00FFFFFF)==(NavKeys[I][1]&0x00FFFFFF))
      return TRUE;
  return FALSE;
}

int IsShiftKey(DWORD Key)
{
  static DWORD ShiftKeys[]={
     KEY_SHIFTLEFT,          KEY_SHIFTNUMPAD4,
     KEY_SHIFTRIGHT,         KEY_SHIFTNUMPAD6,
     KEY_SHIFTHOME,          KEY_SHIFTNUMPAD7,
     KEY_SHIFTEND,           KEY_SHIFTNUMPAD1,
     KEY_SHIFTUP,            KEY_SHIFTNUMPAD8,
     KEY_SHIFTDOWN,          KEY_SHIFTNUMPAD2,
     KEY_SHIFTPGUP,          KEY_SHIFTNUMPAD9,
     KEY_SHIFTPGDN,          KEY_SHIFTNUMPAD3,
     KEY_CTRLSHIFTHOME,      KEY_CTRLSHIFTNUMPAD7,
     KEY_CTRLSHIFTPGUP,      KEY_CTRLSHIFTNUMPAD9,
     KEY_CTRLSHIFTEND,       KEY_CTRLSHIFTNUMPAD1,
     KEY_CTRLSHIFTPGDN,      KEY_CTRLSHIFTNUMPAD3,
     KEY_CTRLSHIFTLEFT,      KEY_CTRLSHIFTNUMPAD4,
     KEY_CTRLSHIFTRIGHT,     KEY_CTRLSHIFTNUMPAD6,
     KEY_ALTSHIFTDOWN,       KEY_ALTSHIFTNUMPAD2,
     KEY_ALTSHIFTLEFT,       KEY_ALTSHIFTNUMPAD4,
     KEY_ALTSHIFTRIGHT,      KEY_ALTSHIFTNUMPAD6,
     KEY_ALTSHIFTUP,         KEY_ALTSHIFTNUMPAD8,
     KEY_ALTSHIFTEND,        KEY_ALTSHIFTNUMPAD1,
     KEY_ALTSHIFTHOME,       KEY_ALTSHIFTNUMPAD7,
     KEY_ALTSHIFTPGDN,       KEY_ALTSHIFTNUMPAD3,
     KEY_ALTSHIFTPGUP,       KEY_ALTSHIFTNUMPAD9,
     KEY_CTRLALTPGUP,        KEY_CTRLALTNUMPAD9,
     KEY_CTRLALTHOME,        KEY_CTRLALTNUMPAD7,
     KEY_CTRLALTPGDN,        KEY_CTRLALTNUMPAD2,
     KEY_CTRLALTEND,         KEY_CTRLALTNUMPAD1,
     KEY_CTRLALTLEFT,        KEY_CTRLALTNUMPAD4,
     KEY_CTRLALTRIGHT,       KEY_CTRLALTNUMPAD6,
     KEY_ALTUP,
     KEY_ALTLEFT,
     KEY_ALTDOWN,
     KEY_ALTRIGHT,
     KEY_ALTHOME,
     KEY_ALTEND,
     KEY_ALTPGUP,
     KEY_ALTPGDN,
     KEY_ALT,
     KEY_CTRL,
  };

  for (int I=0;I<sizeof(ShiftKeys)/sizeof(ShiftKeys[0]);I++)
    if (Key==ShiftKeys[I])
      return TRUE;
  return FALSE;
}


char *FARGetKeybLayoutName(char *Dest,int DestSize)
{
  typedef BOOL (WINAPI *PGETCONSOLEKEYBOARDLAYOUTNAMEA)(LPSTR);
  static PGETCONSOLEKEYBOARDLAYOUTNAMEA pGetConsoleKeyboardLayoutNameA=NULL;
  static int LoadedGCKLM=0;
  static char Buffer[64];

#if defined(USE_WFUNC_IN)
  typedef BOOL (WINAPI *PGETCONSOLEKEYBOARDLAYOUTNAMEW)(WCHAR*);
  static PGETCONSOLEKEYBOARDLAYOUTNAMEW pGetConsoleKeyboardLayoutNameW=NULL;
  static WCHAR WBuffer[100];
#endif

  if(!LoadedGCKLM) // && WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT
  {
    LoadedGCKLM=-1;
    if(!pGetConsoleKeyboardLayoutNameA)
    {
      pGetConsoleKeyboardLayoutNameA = (PGETCONSOLEKEYBOARDLAYOUTNAMEA)GetProcAddress(GetModuleHandle("KERNEL32.DLL"),"GetConsoleKeyboardLayoutNameA");
      if(pGetConsoleKeyboardLayoutNameA)
        LoadedGCKLM=1;
    }
#if defined(USE_WFUNC_IN)
    else if(!pGetConsoleKeyboardLayoutNameW)
    {
      pGetConsoleKeyboardLayoutNameW = (PGETCONSOLEKEYBOARDLAYOUTNAMEW)GetProcAddress(GetModuleHandle("KERNEL32.DLL"),"GetConsoleKeyboardLayoutNameW");
      if(pGetConsoleKeyboardLayoutNameW)
        LoadedGCKLM=2;
    }
#endif
  }

  switch(LoadedGCKLM)
  {
    case 1:
    {
      if(pGetConsoleKeyboardLayoutNameA(Buffer))
      {
        if(Dest)
        {
          xstrncpy(Dest,Buffer,DestSize);
          return Dest;
        }
        else
          return Buffer;
      }
      break;
    }

#if defined(USE_WFUNC_IN)
    case 2:
    {
      if(pGetConsoleKeyboardLayoutNameW(WBuffer))
      {
        UnicodeToOEM(WBuffer,Dest,DestSize);
        return Dest;
      }
      break;
    }
#endif
  }
  return NULL;
}


// GetAsyncKeyState(VK_RSHIFT)
DWORD CalcKeyCode(INPUT_RECORD *rec,int RealKey,int *NotMacros)
{
  _SVS(CleverSysLog Clev("CalcKeyCode()"));
  _SVS(SysLog("Param: %s| RealKey=%d  *NotMacros=%d",_INPUT_RECORD_Dump(rec),RealKey,(NotMacros?*NotMacros:0)));
  CHAR_WCHAR Char;

  unsigned int ScanCode,KeyCode,CtrlState;
  CtrlState=rec->Event.KeyEvent.dwControlKeyState;
  ScanCode=rec->Event.KeyEvent.wVirtualScanCode;
  KeyCode=rec->Event.KeyEvent.wVirtualKeyCode;
  Char.UnicodeChar=rec->Event.KeyEvent.uChar.UnicodeChar;
  //// // _SVS(if(KeyCode == VK_DECIMAL || KeyCode == VK_DELETE) SysLog("CalcKeyCode -> CtrlState=%04X KeyCode=%s ScanCode=%08X AsciiChar=%02X ShiftPressed=%d ShiftPressedLast=%d",CtrlState,_VK_KEY_ToName(KeyCode), ScanCode, Char.AsciiChar,ShiftPressed,ShiftPressedLast));

  if(NotMacros)
    *NotMacros=CtrlState&0x80000000?TRUE:FALSE;
//  CtrlState&=~0x80000000;

  if (!(rec->EventType==KEY_EVENT || rec->EventType == FARMACRO_KEY_EVENT))
  {
    _SVS(SysLog("[%d] return KEY_NONE; [!(rec->EventType==KEY_EVENT || rec->EventType == FARMACRO_KEY_EVENT)]", __LINE__));
    return(KEY_NONE);
  }

  if (!RealKey)
  {
    CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
    AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
    ShiftPressed=(CtrlState & SHIFT_PRESSED);
    RightCtrlPressed=(CtrlState & RIGHT_CTRL_PRESSED);
    RightAltPressed=(CtrlState & RIGHT_ALT_PRESSED);
    RightShiftPressed=(CtrlState & SHIFT_PRESSED);
  }

  DWORD Modif=(CtrlPressed?KEY_CTRL:0)|(AltPressed?KEY_ALT:0)|(ShiftPressed?KEY_SHIFT:0);

  if(rec->Event.KeyEvent.wVirtualKeyCode >= 0xFF && RealKey)
  {
    //VK_?=0x00FF, Scan=0x0013 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
    //VK_?=0x00FF, Scan=0x0014 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
    //VK_?=0x00FF, Scan=0x0015 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
    //VK_?=0x00FF, Scan=0x001A uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
    //VK_?=0x00FF, Scan=0x001B uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
    //VK_?=0x00FF, Scan=0x001E uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
    //VK_?=0x00FF, Scan=0x001F uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
    //VK_?=0x00FF, Scan=0x0023 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
    if(!rec->Event.KeyEvent.bKeyDown && (CtrlState&(ENHANCED_KEY|NUMLOCK_ON)))
    {
      _SVS(SysLog("[%d] return Modif|(KEY_VK_0xFF_BEGIN+ScanCode)=%X (%d)", __LINE__,Modif|(KEY_VK_0xFF_BEGIN+ScanCode),Modif|(KEY_VK_0xFF_BEGIN+ScanCode)));
      return Modif|(KEY_VK_0xFF_BEGIN+ScanCode);
    }
    _SVS(SysLog("[%d] return KEY_IDLE"));
    return KEY_IDLE;
  }

  if (!rec->Event.KeyEvent.bKeyDown)
  {
    KeyCodeForALT_LastPressed=0;
    if (KeyCode==VK_MENU && AltValue!=0)
    {
      //FlushInputBuffer();//???
      INPUT_RECORD TempRec;
      DWORD ReadCount;
      ReadConsoleInput(hConInp,&TempRec,1,&ReadCount);

      ReturnAltValue=TRUE;
      _SVS(SysLog("0 AltNumPad -> AltValue=0x%0X CtrlState=%X",AltValue,CtrlState));
//#if defined(USE_WFUNC_IN)
//      AltValue&=0xFFFF;
//      rec->Event.KeyEvent.uChar.UnicodeChar=AltValue;
//#else
      AltValue&=0x00FF;
      rec->Event.KeyEvent.uChar.AsciiChar=AltValue;
//#endif
      //// // _SVS(SysLog("KeyCode==VK_MENU -> AltValue=%X (%c)",AltValue,AltValue));
      _SVS(SysLog("[%d] return AltValue=%d",__LINE__,AltValue));
      return(AltValue);
    }
    else
    {
      _SVS(SysLog("[%d] return KEY_NONE",__LINE__));
      return(KEY_NONE);
    }
  }

  if ((CtrlState & 9)==9)
    if (Char.AsciiChar!=0)
    {
      _SVS(SysLog("[%d] return Char.AsciiChar",__LINE__));
      return(Char.AsciiChar);
    }
    else
      CtrlPressed=0;

  if (Opt.AltGr && CtrlPressed && (rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED))
    if (Char.AsciiChar=='\\')
    {
      _SVS(SysLog("[%d] return KEY_CTRLBACKSLASH",__LINE__));
      return(KEY_CTRLBACKSLASH);
    }

  if (KeyCode==VK_MENU)
    AltValue=0;

  if (Char.AsciiChar==0 && !CtrlPressed && !AltPressed)
  {
    if (KeyCode==VK_OEM_3 && !Opt.UseVk_oem_x)
    {
      _SVS(SysLog("[%d] return '%c' [KeyCode==VK_OEM_3]",__LINE__,ShiftPressed ? '~':'`'));
      return(ShiftPressed ? '~':'`');
    }

    if (KeyCode==VK_OEM_7 && !Opt.UseVk_oem_x)
    {
    // ES:
    // KEY_EVENT_RECORD: Dn, 1, Vk="VK_OEM_7" [222/0x00DE], Scan=0x0028 uChar=[U=''' (0x0027): A=''' (0x27)] Ctrl=0x00000040 (casac - ecnS) (Widowed)
      _SVS(SysLog("[%d] return '%c' [KeyCode==VK_OEM_7]",__LINE__,ShiftPressed ? '"':'\''));
      return(ShiftPressed ? '"':'\'');
    }
  }

  if (Char.AsciiChar<32 && (CtrlPressed || AltPressed))
  {
    switch(KeyCode)
    {
      case VK_OEM_COMMA:
        Char.AsciiChar=',';
        break;
      case VK_OEM_PERIOD:
        Char.AsciiChar='.';
        break;
      case VK_OEM_4:
        if(!Opt.UseVk_oem_x)
          Char.AsciiChar='[';
        break;
      case VK_OEM_5:
        //Char.AsciiChar=ScanCode==0x29?0x15:'\\'; //???
        if(!Opt.UseVk_oem_x)
          Char.AsciiChar='\\';
        break;
      case VK_OEM_6:
        if(!Opt.UseVk_oem_x)
          Char.AsciiChar=']';
        break;
      case VK_OEM_7:
        if(!Opt.UseVk_oem_x)
          Char.AsciiChar='\"';
        break;
    }
  }
  /* $ 24.08.2000 SVS
     "Персональные 100 грамм" :-)
  */
  if(CtrlPressed && AltPressed && ShiftPressed)
  {
    switch(KeyCode)
    {
      case VK_SHIFT:
      case VK_MENU:
      case VK_CONTROL:
      {
        if(RightCtrlPressed && RightAltPressed && RightShiftPressed)
        {
          if((Opt.CASRule&2))
          {
            _SVS(SysLog("[%d] return %s",__LINE__,_FARKEY_ToName(IsKeyRCASPressed?KEY_RCTRLALTSHIFTPRESS:KEY_RCTRLALTSHIFTRELEASE)));
            return (IsKeyRCASPressed?KEY_RCTRLALTSHIFTPRESS:KEY_RCTRLALTSHIFTRELEASE);
          }
        }
        else if(Opt.CASRule&1)
        {
          _SVS(SysLog("[%d] return %s",__LINE__,_FARKEY_ToName(IsKeyCASPressed?KEY_CTRLALTSHIFTPRESS:KEY_CTRLALTSHIFTRELEASE)));
          return (IsKeyCASPressed?KEY_CTRLALTSHIFTPRESS:KEY_CTRLALTSHIFTRELEASE);
        }
      }
    }
  }
  if(RightCtrlPressed && RightAltPressed && RightShiftPressed)
  {
    switch(KeyCode)
    {
      case VK_RSHIFT:
      case VK_RMENU:
      case VK_RCONTROL:
        if(Opt.CASRule&2)
        {
          _SVS(SysLog("[%d] return %s",__LINE__,_FARKEY_ToName(IsKeyRCASPressed?KEY_RCTRLALTSHIFTPRESS:KEY_RCTRLALTSHIFTRELEASE)));
          return (IsKeyRCASPressed?KEY_RCTRLALTSHIFTPRESS:KEY_RCTRLALTSHIFTRELEASE);
        }
        break;
    }
  }
  /* SVS $*/

  if (KeyCode>=VK_F1 && KeyCode<=VK_F24)
  {
    _SVS(SysLog("[%d] return %s [if (KeyCode>=VK_F1 && KeyCode<=VK_F24)]",__LINE__,_FARKEY_ToName(Modif+KEY_F1+((KeyCode-VK_F1)))));
//    return(Modif+KEY_F1+((KeyCode-VK_F1)<<8));
    return(Modif+KEY_F1+((KeyCode-VK_F1)));
  }
  int NotShift=!CtrlPressed && !AltPressed && !ShiftPressed;

  // Здесь полное шаманство, т.к. при включенном NumLock Статус Shift`а
  // куда то нахрен пропадает (утомился выводить "формулу любви")
  DWORD Modif2=0;

  if(!(CtrlState&ENHANCED_KEY)) //(CtrlState&NUMLOCK_ON) // не затрагиваем серые клавиши.
  {
    Modif2=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)?KEY_CTRL:0)|
                (CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)?KEY_ALT:0);

    if(CtrlState&NUMLOCK_ON)
    {
      Modif2|=KEY_SHIFT;
      if(KeyCode >= VK_NUMPAD0 && KeyCode <= VK_NUMPAD9 || KeyCode == VK_DECIMAL)
      {
        Modif2&=~KEY_SHIFT;
      }
    }
    else
      Modif2|=GetAsyncKeyState(VK_SHIFT) < 0?KEY_SHIFT:0;
  }
  _SVS(SysLog("[%d] Modif2=%08X",__LINE__,Modif2));

  if (AltPressed && !CtrlPressed && !ShiftPressed)
  {
#if 0
    if (AltValue==0 && (CtrlObject->Macro.IsRecording() == MACROMODE_NOMACRO || !Opt.UseNumPad))
    {
      // VK_INSERT  = 0x2D       AS-0 = 0x2D
      // VK_NUMPAD0 = 0x60       A-0  = 0x60
      /*
        С грабером не все понятно - что, где и когда вызывать,
        посему его оставим пока в покое.
      */
      if(//(CtrlState&NUMLOCK_ON)  && KeyCode==VK_NUMPAD0 && !(CtrlState&ENHANCED_KEY) ||
         (Opt.UseNumPad && KeyCode==VK_INSERT && (CtrlState&ENHANCED_KEY)) ||
         (!Opt.UseNumPad && (KeyCode==VK_INSERT || KeyCode==VK_NUMPAD0))
        )
      {   // CtrlObject->Macro.IsRecording()
      //// // _SVS(SysLog("IsProcessAssignMacroKey=%d",IsProcessAssignMacroKey));
        if(IsProcessAssignMacroKey && Opt.UseNumPad)
        {
          return KEY_INS|KEY_ALT;
        }
        else
        {
          RunGraber();
        }
        return(KEY_NONE);
      }
    }
#else
    if (AltValue==0)
    {
      if(KeyCode==VK_INSERT || KeyCode==VK_NUMPAD0)
      {
        if(CtrlObject && CtrlObject->Macro.IsRecording())
          CtrlObject->Macro.ProcessKey(KEY_INS|KEY_ALT);
        RunGraber();
        _SVS(SysLog("[%d] return KEY_NONE",__LINE__));
        return(KEY_NONE);
      }
    }
#endif

    //// // _SVS(SysLog("1 AltNumPad -> CalcKeyCode -> KeyCode=%s  ScanCode=0x%0X AltValue=0x%0X CtrlState=%X GetAsyncKeyState(VK_SHIFT)=%X",_VK_KEY_ToName(KeyCode),ScanCode,AltValue,CtrlState,GetAsyncKeyState(VK_SHIFT)));
    if((CtrlState & ENHANCED_KEY)==0
      //(CtrlState&NUMLOCK_ON) && KeyCode >= VK_NUMPAD0 && KeyCode <= VK_NUMPAD9 ||
      // !(CtrlState&NUMLOCK_ON) && KeyCode < VK_NUMPAD0
      )
    {
    //// // _SVS(SysLog("2 AltNumPad -> CalcKeyCode -> KeyCode=%s  ScanCode=0x%0X AltValue=0x%0X CtrlState=%X GetAsyncKeyState(VK_SHIFT)=%X",_VK_KEY_ToName(KeyCode),ScanCode,AltValue,CtrlState,GetAsyncKeyState(VK_SHIFT)));
      static unsigned int ScanCodes[]={82,79,80,81,75,76,77,71,72,73};
      for (int I=0;I<sizeof(ScanCodes)/sizeof(ScanCodes[0]);I++)
      {
        if (ScanCodes[I]==ScanCode)
        {
          if (RealKey && KeyCodeForALT_LastPressed != KeyCode)
          {
            AltValue=AltValue*10+I;
            KeyCodeForALT_LastPressed=KeyCode;
          }
          _SVS(SysLog("(AltDigit) [%d] AltNumPad -> AltValue=0x%0X CtrlState=%X",__LINE__,AltValue,CtrlState));
          if(AltValue!=0)
          {
            _SVS(SysLog("[%d] return KEY_NONE [if(AltValue!=0)]",__LINE__));
            return(KEY_NONE);
          }
        }
      }
    }
  }

  /*
  NumLock=Off
    Down
      CtrlState=0100 KeyCode=0028 ScanCode=00000050 AsciiChar=00         ENHANCED_KEY
      CtrlState=0100 KeyCode=0028 ScanCode=00000050 AsciiChar=00
    Num2
      CtrlState=0000 KeyCode=0028 ScanCode=00000050 AsciiChar=00
      CtrlState=0000 KeyCode=0028 ScanCode=00000050 AsciiChar=00

    Ctrl-8
      CtrlState=0008 KeyCode=0026 ScanCode=00000048 AsciiChar=00
    Ctrl-Shift-8               ^^!!!
      CtrlState=0018 KeyCode=0026 ScanCode=00000048 AsciiChar=00

  ------------------------------------------------------------------------
  NumLock=On

    Down
      CtrlState=0120 KeyCode=0028 ScanCode=00000050 AsciiChar=00         ENHANCED_KEY
      CtrlState=0120 KeyCode=0028 ScanCode=00000050 AsciiChar=00
    Num2
      CtrlState=0020 KeyCode=0062 ScanCode=00000050 AsciiChar=32
      CtrlState=0020 KeyCode=0062 ScanCode=00000050 AsciiChar=32

    Ctrl-8
      CtrlState=0028 KeyCode=0068 ScanCode=00000048 AsciiChar=00
    Ctrl-Shift-8               ^^!!!
      CtrlState=0028 KeyCode=0026 ScanCode=00000048 AsciiChar=00
  */

  /* ------------------------------------------------------------- */
  switch(KeyCode)
  {
    case VK_INSERT:
    case VK_NUMPAD0:
      if(CtrlState&ENHANCED_KEY)
      {
        return(Modif|KEY_INS);
      }
      else if((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD0)
        return '0';
      return Modif|(Opt.UseNumPad?Modif2|KEY_NUMPAD0:KEY_INS);
    case VK_DOWN:
    case VK_NUMPAD2:
      if(CtrlState&ENHANCED_KEY)
      {
        return(Modif|KEY_DOWN);
      }
      else if((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD2)
        return '2';
      return Modif|(Opt.UseNumPad?Modif2|KEY_NUMPAD2:KEY_DOWN);
    case VK_LEFT:
    case VK_NUMPAD4:
      if(CtrlState&ENHANCED_KEY)
      {
        return(Modif|KEY_LEFT);
      }
      else if((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD4)
        return '4';
      return Modif|(Opt.UseNumPad?Modif2|KEY_NUMPAD4:KEY_LEFT);
    case VK_RIGHT:
    case VK_NUMPAD6:
      if(CtrlState&ENHANCED_KEY)
      {
        return(Modif|KEY_RIGHT);
      }
      else if((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD6)
        return '6';
      return Modif|(Opt.UseNumPad?Modif2|KEY_NUMPAD6:KEY_RIGHT);
    case VK_UP:
    case VK_NUMPAD8:
      if(CtrlState&ENHANCED_KEY)
      {
        return(Modif|KEY_UP);
      }
      else if((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD8)
        return '8';
      return Modif|(Opt.UseNumPad?Modif2|KEY_NUMPAD8:KEY_UP);
    case VK_END:
    case VK_NUMPAD1:
      if(CtrlState&ENHANCED_KEY)
      {
        return(Modif|KEY_END);
      }
      else if((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD1)
        return '1';
      return Modif|(Opt.UseNumPad?Modif2|KEY_NUMPAD1:KEY_END);
    case VK_HOME:
    case VK_NUMPAD7:
      if(CtrlState&ENHANCED_KEY)
      {
        return(Modif|KEY_HOME);
      }
      else if((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD7)
        return '7';
      return Modif|(Opt.UseNumPad?Modif2|KEY_NUMPAD7:KEY_HOME);
    case VK_NEXT:
    case VK_NUMPAD3:
      if(CtrlState&ENHANCED_KEY)
      {
        return(Modif|KEY_PGDN);
      }
      else if((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD3)
        return '3';
      return Modif|(Opt.UseNumPad?Modif2|KEY_NUMPAD3:KEY_PGDN);
    case VK_PRIOR:
    case VK_NUMPAD9:
      if(CtrlState&ENHANCED_KEY)
      {
        return(Modif|KEY_PGUP);
      }
      else if((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD9)
        return '9';
      return Modif|(Opt.UseNumPad?Modif2|KEY_NUMPAD9:KEY_PGUP);
    case VK_CLEAR:
    case VK_NUMPAD5:
      if(CtrlState&ENHANCED_KEY)
      {
        return(Modif|KEY_NUMPAD5);
      }
      else if((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD5)
        return '5';
      return Modif|(Opt.UseNumPad?Modif2:0)|KEY_NUMPAD5;

    case VK_DECIMAL:
    case VK_DELETE:
//      // // _SVS(SysLog("case VK_DELETE:  Opt.UseNumPad=%08X CtrlState=%X GetAsyncKeyState(VK_SHIFT)=%X",Opt.UseNumPad,CtrlState,GetAsyncKeyState(VK_SHIFT)));
      if(CtrlState&ENHANCED_KEY)
      {
        return(Modif|KEY_DEL);
      }
      else if(NotShift && (CtrlState&NUMLOCK_ON) && KeyCode == VK_DECIMAL ||
              (CtrlState&NUMLOCK_ON) && KeyCode == VK_DELETE && (WinVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)) // МАСДАЙ!
        return '.';
      return Modif| (Opt.UseNumPad?Modif2:0)| KEY_DEL;
  }

  switch(KeyCode)
  {
    case VK_BROWSER_BACK:
      return Modif|KEY_BROWSER_BACK;
    case VK_BROWSER_FORWARD:
      return Modif|KEY_BROWSER_FORWARD;
    case VK_BROWSER_REFRESH:
      return Modif|KEY_BROWSER_REFRESH;
    case VK_BROWSER_STOP:
      return Modif|KEY_BROWSER_STOP;
    case VK_BROWSER_SEARCH:
      return Modif|KEY_BROWSER_SEARCH;
    case VK_BROWSER_FAVORITES:
      return Modif|KEY_BROWSER_FAVORITES;
    case VK_BROWSER_HOME:
      return Modif|KEY_BROWSER_HOME;
    case VK_VOLUME_MUTE:
      return Modif|KEY_VOLUME_MUTE;
    case VK_VOLUME_DOWN:
      return Modif|KEY_VOLUME_DOWN;
    case VK_VOLUME_UP:
      return Modif|KEY_VOLUME_UP;
    case VK_MEDIA_NEXT_TRACK:
      return Modif|KEY_MEDIA_NEXT_TRACK;
    case VK_MEDIA_PREV_TRACK:
      return Modif|KEY_MEDIA_PREV_TRACK;
    case VK_MEDIA_STOP:
      return Modif|KEY_MEDIA_STOP;
    case VK_MEDIA_PLAY_PAUSE:
      return Modif|KEY_MEDIA_PLAY_PAUSE;
    case VK_LAUNCH_MAIL:
      return Modif|KEY_LAUNCH_MAIL;
    case VK_LAUNCH_MEDIA_SELECT:
      return Modif|KEY_LAUNCH_MEDIA_SELECT;
    case VK_LAUNCH_APP1:
      return Modif|KEY_LAUNCH_APP1;
    case VK_LAUNCH_APP2:
      return Modif|KEY_LAUNCH_APP2;

    case VK_APPS:
      return(Modif|KEY_APPS);
    case VK_LWIN:
      return(Modif|KEY_LWIN);
    case VK_RWIN:
      return(Modif|KEY_RWIN);
    case VK_RETURN:
      //  !!!!!!!!!!!!! - Если "!ShiftPressed", то Shift-F4 Shift-Enter, не
      //                  отпуская Shift...
      _SVS(SysLog("(VK_RETURN) ShiftPressed=%d RealKey=%d !ShiftPressedLast=%d !CtrlPressed=%d !AltPressed=%d (%d)",ShiftPressed,RealKey,ShiftPressedLast,CtrlPressed,AltPressed,(ShiftPressed && RealKey && !ShiftPressedLast && !CtrlPressed && !AltPressed)));
      if (ShiftPressed && RealKey && !ShiftPressedLast && !CtrlPressed && !AltPressed && !LastShiftEnterPressed)
        return(KEY_ENTER);
      LastShiftEnterPressed=Modif&KEY_SHIFT?TRUE:FALSE;
      return(Modif|KEY_ENTER);
    case VK_BACK:
      return(Modif|KEY_BS);
    case VK_SPACE:
      if(Char.AsciiChar == ' ')
        return(Modif|KEY_SPACE);
      return Char.AsciiChar;
    case VK_TAB:
      return(Modif|KEY_TAB);
    case VK_ADD:
      return(Modif|KEY_ADD);
    case VK_SUBTRACT:
      return(Modif|KEY_SUBTRACT);
    case VK_ESCAPE:
      return(Modif|KEY_ESC);
  }

  /* ------------------------------------------------------------- */
  if (CtrlPressed && AltPressed && ShiftPressed)
  {
    _SVS(if(KeyCode!=VK_CONTROL && KeyCode!=VK_MENU) SysLog("CtrlAltShift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_SHIFT|KEY_CTRL|KEY_ALT+KeyCode);
    if(Opt.ShiftsKeyRules) //???
      switch(KeyCode)
      {
        case VK_OEM_3:
          //if(!Opt.UseVk_oem_x)
            return(KEY_SHIFT+KEY_CTRL+KEY_ALT+'~');
          //break;
        case VK_OEM_MINUS:
          return(KEY_SHIFT+KEY_CTRL+KEY_ALT+'-');
        case VK_OEM_PLUS:
          return(KEY_SHIFT+KEY_CTRL+KEY_ALT+'=');
        case VK_OEM_5:
          return(KEY_SHIFT+KEY_CTRL+KEY_ALT+KEY_BACKSLASH);
        case VK_OEM_6:
          return(KEY_SHIFT+KEY_CTRL+KEY_ALT+KEY_BACKBRACKET);
        case VK_OEM_4:
          return(KEY_SHIFT+KEY_CTRL+KEY_ALT+KEY_BRACKET);
        case VK_OEM_7:
          return(KEY_SHIFT+KEY_CTRL+KEY_ALT+KEY_QUOTE);
        case VK_OEM_1:
          return(KEY_SHIFT+KEY_CTRL+KEY_ALT+KEY_COLON);
        case VK_OEM_2:
          return(KEY_SHIFT+KEY_CTRL+KEY_ALT+KEY_SLASH);
        case VK_OEM_PERIOD:
          return(KEY_SHIFT+KEY_CTRL+KEY_ALT+KEY_DOT);
        case VK_OEM_COMMA:
          return(KEY_SHIFT+KEY_CTRL+KEY_ALT+KEY_COMMA);
      }
    switch(KeyCode)
    {
      case VK_DIVIDE:
        return(KEY_SHIFT|KEY_CTRLALT|KEY_DIVIDE);
      case VK_MULTIPLY:
        return(KEY_SHIFT|KEY_CTRLALT|KEY_MULTIPLY);
      case VK_PAUSE:
        return(KEY_SHIFT|KEY_CTRLALT|KEY_BREAK);
    }
    if (Char.AsciiChar)
      return(KEY_SHIFT|KEY_CTRL|KEY_ALT+Char.AsciiChar);
    if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_MENU))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_SHIFT|KEY_CTRL|KEY_ALT+KeyCode);
  }

  /* ------------------------------------------------------------- */
  if (CtrlPressed && AltPressed)
  {
    _SVS(if(KeyCode!=VK_CONTROL && KeyCode!=VK_MENU) SysLog("CtrlAlt -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRL|KEY_ALT+KeyCode);
    if(Opt.ShiftsKeyRules) //???
      switch(KeyCode)
      {
        case VK_OEM_3:
          return(KEY_CTRL+KEY_ALT+'~');
        case VK_OEM_MINUS:
          return(KEY_CTRL+KEY_ALT+'-');
        case VK_OEM_PLUS:
          return(KEY_CTRL+KEY_ALT+'=');
        case VK_OEM_5:
          return(KEY_CTRL+KEY_ALT+KEY_BACKSLASH);
        case VK_OEM_6:
          return(KEY_CTRL+KEY_ALT+KEY_BACKBRACKET);
        case VK_OEM_4:
          return(KEY_CTRL+KEY_ALT+KEY_BRACKET);
        case VK_OEM_7:
          return(KEY_CTRL+KEY_ALT+KEY_QUOTE);
        case VK_OEM_1:
          return(KEY_CTRL+KEY_ALT+KEY_COLON);
        case VK_OEM_2:
          return(KEY_CTRL+KEY_ALT+KEY_SLASH);
        case VK_OEM_PERIOD:
          return(KEY_CTRL+KEY_ALT+KEY_DOT);
        case VK_OEM_COMMA:
          return(KEY_CTRL+KEY_ALT+KEY_COMMA);
      }
    switch(KeyCode)
    {
      case VK_DIVIDE:
        return(KEY_CTRLALT|KEY_DIVIDE);
      case VK_MULTIPLY:
        return(KEY_CTRLALT|KEY_MULTIPLY);
      case VK_PAUSE:
        return(KEY_CTRLALT|KEY_BREAK);
    }
    if (Char.AsciiChar)
      return(KEY_CTRL|KEY_ALT+Char.AsciiChar);
    if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_MENU))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_CTRL|KEY_ALT+KeyCode);
  }


  /* ------------------------------------------------------------- */
  if (AltPressed && ShiftPressed)
  {
    _SVS(if(KeyCode!=VK_MENU && KeyCode!=VK_SHIFT) SysLog("AltShift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));
    if (KeyCode>='0' && KeyCode<='9')
    {
      if(WaitInFastFind > 0 &&
        CtrlObject->Macro.GetCurRecord(NULL,NULL) < MACROMODE_RECORDING &&
        CtrlObject->Macro.GetIndex(KEY_ALTSHIFT0+KeyCode-'0',-1) == -1)
      {
        return(KEY_ALT+KEY_SHIFT+Char.AsciiChar);
      }
      else
        return(KEY_ALTSHIFT0+KeyCode-'0');
    }
    if (!WaitInMainLoop && KeyCode>='A' && KeyCode<='Z')
      return(KEY_ALTSHIFT+KeyCode);
    if(Opt.ShiftsKeyRules) //???
      switch(KeyCode)
      {
        case VK_OEM_3:
          return(KEY_ALT+KEY_SHIFT+'~');
        case VK_OEM_MINUS:
          return(KEY_ALT+KEY_SHIFT+'_');
        case VK_OEM_PLUS:
          return(KEY_ALT+KEY_SHIFT+'=');
        case VK_OEM_5:
          return(KEY_ALT+KEY_SHIFT+KEY_BACKSLASH);
        case VK_OEM_6:
          return(KEY_ALT+KEY_SHIFT+KEY_BACKBRACKET);
        case VK_OEM_4:
          return(KEY_ALT+KEY_SHIFT+KEY_BRACKET);
        case VK_OEM_7:
          return(KEY_ALT+KEY_SHIFT+KEY_QUOTE);
        case VK_OEM_1:
          return(KEY_ALT+KEY_SHIFT+KEY_COLON);
        case VK_OEM_2:
          if(WaitInFastFind)
            return(KEY_ALT+KEY_SHIFT+'?');
          else
            return(KEY_ALT+KEY_SHIFT+KEY_SLASH);
        case VK_OEM_PERIOD:
          return(KEY_ALT+KEY_SHIFT+KEY_DOT);
        case VK_OEM_COMMA:
          return(KEY_ALT+KEY_SHIFT+KEY_COMMA);
      }
    switch(KeyCode)
    {
      case VK_DIVIDE:
        //if(WaitInFastFind)
        //  return(KEY_ALT+KEY_SHIFT+'/');
        //else
          return(KEY_ALTSHIFT|KEY_DIVIDE);
      case VK_MULTIPLY:
        //if(WaitInFastFind)
        //{
       //   return(KEY_ALT+KEY_SHIFT+'*');
       // }
        //else
          return(KEY_ALTSHIFT|KEY_MULTIPLY);
      case VK_CAPITAL:
        return(KEY_NONE);
      case VK_PAUSE:
        return(KEY_ALTSHIFT|KEY_BREAK);
    }
    if (Char.AsciiChar)
    {
      if (Opt.AltGr && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
        if (rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)
          return(Char.AsciiChar);
      return(KEY_ALT+KEY_SHIFT+Char.AsciiChar);
    }
    if (!RealKey && (KeyCode==VK_MENU || KeyCode==VK_SHIFT))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_ALT+KEY_SHIFT+KeyCode);
  }


  /* ------------------------------------------------------------- */
  if (CtrlPressed && ShiftPressed)
  {
    _SVS(if(KeyCode!=VK_CONTROL && KeyCode!=VK_SHIFT) SysLog("CtrlShift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_CTRLSHIFT0+KeyCode-'0');
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRLSHIFTA+KeyCode-'A');
    switch(KeyCode)
    {
      case VK_OEM_PERIOD:
        return(KEY_CTRLSHIFTDOT);
      case VK_OEM_4:
        return(KEY_CTRLSHIFTBRACKET);
      case VK_OEM_6:
        return(KEY_CTRLSHIFTBACKBRACKET);
      case VK_OEM_2:
        return(KEY_CTRLSHIFTSLASH);
      case VK_OEM_5:
        return(KEY_CTRLSHIFTBACKSLASH);
      case VK_DIVIDE:
        return(KEY_CTRLSHIFT|KEY_DIVIDE);
      case VK_MULTIPLY:
        return(KEY_CTRLSHIFT|KEY_MULTIPLY);
    }
    if(Opt.ShiftsKeyRules) //???
      switch(KeyCode)
      {
        case VK_OEM_3:
          return(KEY_CTRL+KEY_SHIFT+'~');
        case VK_OEM_MINUS:
          return(KEY_CTRL+KEY_SHIFT+'-');
        case VK_OEM_PLUS:
          return(KEY_CTRL+KEY_SHIFT+'=');
        case VK_OEM_7:
          return(KEY_CTRL+KEY_SHIFT+KEY_QUOTE);
        case VK_OEM_1:
          return(KEY_CTRL+KEY_SHIFT+KEY_COLON);
        case VK_OEM_COMMA:
          return(KEY_CTRL+KEY_SHIFT+KEY_COMMA);
      }
    if (Char.AsciiChar)
      return(KEY_CTRL|KEY_SHIFT+Char.AsciiChar);
    if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_SHIFT))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_CTRL|KEY_SHIFT+KeyCode);
  }


  /* ------------------------------------------------------------- */
  if ((CtrlState & RIGHT_CTRL_PRESSED)==RIGHT_CTRL_PRESSED)
  {
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_RCTRL0+KeyCode-'0');
  }


  /* ------------------------------------------------------------- */
  if (!CtrlPressed && !AltPressed && !ShiftPressed)
  {
    switch(KeyCode)
    {
      case VK_DIVIDE:
        return(KEY_DIVIDE);
      case VK_CANCEL:
        return(KEY_BREAK);
      case VK_MULTIPLY:
        return(KEY_MULTIPLY);
    }
  }

  /* ------------------------------------------------------------- */
  if (CtrlPressed)
  {
    _SVS(if(KeyCode!=VK_CONTROL) SysLog("Ctrl -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_CTRL0+KeyCode-'0');
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRL+KeyCode);
    switch(KeyCode)
    {
      case VK_OEM_COMMA:
        return(KEY_CTRLCOMMA);
      case VK_OEM_PERIOD:
        return(KEY_CTRLDOT);
      case VK_OEM_2:
        return(KEY_CTRLSLASH);
      case VK_OEM_4:
        return(KEY_CTRLBRACKET);
      case VK_OEM_5:
        return(KEY_CTRLBACKSLASH);
      case VK_OEM_6:
        return(KEY_CTRLBACKBRACKET);
      case VK_OEM_7:
        return(KEY_CTRLQUOTE);
      case VK_MULTIPLY:
        return(KEY_CTRL|KEY_MULTIPLY);
      case VK_DIVIDE:
        return(KEY_CTRL|KEY_DIVIDE);
    }

    if(Opt.ShiftsKeyRules) //???
      switch(KeyCode)
      {
        case VK_OEM_3:
          return(KEY_CTRL+'~');
        case VK_OEM_MINUS:
          return(KEY_CTRL+'-');
        case VK_OEM_PLUS:
          return(KEY_CTRL+'=');
        case VK_OEM_1:
          return(KEY_CTRL+KEY_COLON);
      }

    if (KeyCode)
    {
      if (!RealKey && KeyCode==VK_CONTROL)
        return(KEY_NONE);
      return(KEY_CTRL+KeyCode);
    }
  }

  /* ------------------------------------------------------------- */
  if (AltPressed)
  {
    _SVS(if(KeyCode!=VK_MENU) SysLog("Alt -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));
    if(Opt.ShiftsKeyRules) //???
      switch(KeyCode)
      {
        case VK_OEM_3:
          return(KEY_ALT+'~');
        case VK_OEM_MINUS:
          if(WaitInFastFind)
            return(KEY_ALT+KEY_SHIFT+'_');
          else
            return(KEY_ALT+'-');
        case VK_OEM_PLUS:
          return(KEY_ALT+'=');
        case VK_OEM_5:
          return(KEY_ALT+KEY_BACKSLASH);
        case VK_OEM_6:
          return(KEY_ALT+KEY_BACKBRACKET);
        case VK_OEM_4:
          return(KEY_ALT+KEY_BRACKET);
        case VK_OEM_7:
          return(KEY_ALT+KEY_QUOTE);
        case VK_OEM_1:
          return(KEY_ALT+KEY_COLON);
        case VK_OEM_2:
          return(KEY_ALT+KEY_SLASH);
      }
    switch(KeyCode)
    {
      case VK_OEM_COMMA:
        return(KEY_ALTCOMMA);
      case VK_OEM_PERIOD:
        return(KEY_ALTDOT);
      case VK_DIVIDE:
        //if(WaitInFastFind)
        //  return(KEY_ALT+KEY_SHIFT+'/');
        //else
          return(KEY_ALT|KEY_DIVIDE);
      case VK_MULTIPLY:
//        if(WaitInFastFind)
//          return(KEY_ALT+KEY_SHIFT+'*');
//        else
          return(KEY_ALT|KEY_MULTIPLY);
      case VK_PAUSE:
        return(KEY_ALT+KEY_BREAK);
    }
    if (Char.AsciiChar)
    {
      if (Opt.AltGr && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
      {
        _SVS(SysLog("if (Opt.AltGr... %x",Char.AsciiChar));
        if (rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)
        {
          _SVS(SysLog("[%d] return %s",__LINE__,_FARKEY_ToName(Char.AsciiChar)));
          return(Char.AsciiChar);
        }
      }
      if(!Opt.ShiftsKeyRules || WaitInFastFind > 0)
        return(LocalUpper(Char.AsciiChar)+KEY_ALT);
      else if(WaitInMainLoop ||
              !Opt.HotkeyRules //????
           )
      {
        _SVS(SysLog("[%d] return %s",__LINE__,_FARKEY_ToName(KEY_ALT+Char.AsciiChar)));
        return(KEY_ALT+Char.AsciiChar);
      }
    }
    if (KeyCode==VK_CAPITAL)
    {
      _SVS(SysLog("[%d] return KEY_NONE",__LINE__));
      return(KEY_NONE);
    }
    if (!RealKey && KeyCode==VK_MENU)
    {
      _SVS(SysLog("[%d] return KEY_NONE",__LINE__));
      return(KEY_NONE);
    }
    _SVS(SysLog("[%d] return %s",__LINE__,_FARKEY_ToName(KEY_ALT+KeyCode)));
    return(KEY_ALT+KeyCode);
  }

  if(ShiftPressed)
  {
    _SVS(if(KeyCode!=VK_SHIFT) SysLog("Shift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));
    switch(KeyCode)
    {
      case VK_DIVIDE:
        return(KEY_SHIFT|KEY_DIVIDE);
      case VK_MULTIPLY:
        return(KEY_SHIFT|KEY_MULTIPLY);
    }

  }

  if (Char.AsciiChar)
  {
    _SVS(SysLog("[%d] return Char.AsciiChar=%x",__LINE__,Char.AsciiChar));
    return(Char.AsciiChar);
  }
  _SVS(SysLog("[%d] return KEY_NONE",__LINE__));
  return(KEY_NONE);
}
