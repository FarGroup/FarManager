/*
dialog.cpp

Класс диалога

*/

/* Revision: 1.90 28.04.2001 $ */

/*
Modify:
  04.05.2001 SVS
    ! Наконец то дошли руки до DI_LISTBOX ;-) - новый член FarDialogItem.ListPos
    - Бага с DN_INIT* - переведен "на уровень выше", т.е. до запоминания
      области отображения диалога.
    - какого хрена засунули DialogEdit->FastShow() в функцию инициализации?
  28.04.2001 SVS
   + Функция GetItemRect() получения геометрии итема.
   - DN_MOUSECLICK фактически не работал, если рамка стояла первой в списке.
  27.04.2001 SVS
   - Не работали позиционирование в хистори и в комбобоксах - вместо
     MaxLen, стояло sizeof(MaxLen) :-((
  27.04.2001 VVM
   + Обработка KEY_MSWHEEL_XXXX
   - Убрал подмену клавиш при прокрутке.
  26.04.2001 SVS
   - Проблема с DM_SETTEXTLENGTH: выставляем максимальный размер в том
     случае, если он еще не выставлен.
  25.04.2001 SVS
   + уточнения по поводу DM_SETTEXTLENGTH & DIF_VAREDIT
  24.04.2001 SVS
   ! Подмена клавиш при прокрутке колеса.
   ! Если в ответ на событие DN_MOUSECLICK (Param=-1) обработчик диалога
     вернет TRUE, то диалог не будет закрыт. Если игнорировать
     {DN_MOUSECLICK,-1} или вернуть FALSE, то диалог не закроется.
   ! Клик мышой вне диталога - обрабатываются только левая и правая клавиши
     остальные клавиши не имеют значения
  23.04.2001 SVS
   - Забыл послать месаг DN_EDITCHANGE при выборе из хистори.
  22.04.2001 SVS
   + DIF_SHOWAMPERSAND для DI_BUTTON, DI_CHECKBOX, DI_RADIOBUTTON
  16.04.2001 SVS
   + Tab в списке хистори - аналог Enter
   ! Перерисовка в автодополнении должна идти после DN_EDITCHANGE (imho)
  12.04.2001 SVS
   + DM_ADDHISTORY - добавить строку в историю
   + DIF_MANUALADDHISTORY - добавлять в историю только "ручками"
   ! функция AddToEditHistory теперь возвращает результат операции
     добавления строки в историю
  12.04.2001 SVS
   ! Дополнительная проверка для DM_SETDLGITEM на смену типа котрола.
     Нефига пока менять - ядро еще не готово к метаморфозам. Потом как нить.
   + CheckDialogCoord() - проверка и корректировка координат диалога
   ! не был реализован механизм центрирования диалога для DM_MOVEDIALOG
     это когда координаты = -1, и задано абсолютное смещение.
  02.04.2001 SVS
   + исключим смену режима RO для поля ввода с клавиатуры
  23.03.2001 SVS
   ! У функции ConvertItem() новый параметр InternalCall - сейчас
     используется только для DN_EDITCHANGE
  22.03.2001 SVS
   - немного подправлен механизЪм DN_EDITCHANGE
  05.03.2001 SVS
   - Бага в хоткеях :-(
  21.02.2001 IS
   ! Opt.EditorPersistentBlocks -> Opt.EdOpt.PersistentBlocks
  21.02.2001 IS
   - Избавился от утечки памяти в SelectFromEditHistory (проявлялось не у всех,
     но проявлялось же!)
  20.02.2001 SVS
   ! Пересмотр алгоритма IsKeyHighlighted с добавками Alt- на
     сколько это возможно
  20.02.2001 SVS
   ! Уточнение поведения горячих клавиш с учетом Disabled & Hidden...
  20.02.2001 SVS
   - Бага в SelectFromComboBox
  13.02.2001 SVS
   ! HISTORY_COUNT - константа для размера истории.
   + Дополнительный параметр у FindInEditForAC, SelectFromEditHistory,
     AddToEditHistory и SelectFromComboBox - MaxLen - максимальный размер
     строки назначения.
   + DI_COMBOBOX тоже мАгет работать с DIF_VAREDIT
   ! Рисование "салазок" перенесено в DefDlgProc(DN_DRAWDIALOG,1)
   ! Если функция обработчик в ответ на DN_DRAWDIALOG вернула FALSE,
     то диалог отрисовываться не будет
   ! Если функция обработчик в ответ на DN_DRAWDLGITEM вернула FALSE,
     то элемент диалога отрисовываться не будет
  12.02.2001 SVS
   + Добавка на реакцию DOUBLE_CLICK - чекбоксы стали шустрее переключаться :-)
     AN> ...Пробелом те же чекбоксы переключаются очень шустро.
     AN> Это именно отсутствие реакции на (MouseEvent.dwEventFlags==DOUBLE_CLICK).
  11.02.2001 SVS
   ! DIF_VAREDIT - только для DI_EDIT!!!
  11.02.2001 SVS
   ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  23.01.2001 SVS
   ! Изменены функции DeleteDialogObjects() и GetDialogObjectsData()
   + Введены ексепшины в DataToItem и ConvertItem
   ! Введена проверка на NULL объектов ObjPtr
  04.01.2001 SVS
   - Bug - DM_KEY не реагировал на то, что возвращает пользовательская функция
  04.01.2001 SVS
   - Bug при использовании DM_SETDLGITEM - неверно устанавливался фокус ввода
  28.12.2000 SVS
   + добавлена обработка Opt.HotkeyRules
  25.12.2000 SVS
   - Забыл сделать возврат из функции для DM_GETTEXTPTR
  21.12.2000 SVS
   ! Ctr-Break теперь недействителен, т.е. все зависит от того
     что вернет обработчик.
   + DIF_SHOWAMPERSAND для DI_*BOX
   + DM_GETTEXTPTR, DM_SETTEXTPTR
  15.12.2000 SVS
   ! При перемещении диалога повторяем поведение "бормандовых" сред.
   ! Новый движок мышиного перемещения
  14.12.2000 SVS
   ! Дополнение цветов (пропустил :-)
   + Отмена реакции хоткеев для DIF_DISABLE
  08.12.2000 SVS
   ! DM_SET(GET)TEXT - Param2 указатель на структуру FarDialogItemData
  06.12.2000 SVS
   - Охрененная бага с SelectOnEntry() - эта функция вызывалась
     до создания редактора!!!
   ! Обработка кликания мыши вне диалога вынесена в DefDlgProc
   ! Уточним логику работы:
     1) DI_BUTTON: Если не старый стиль и кнопка "не для закрытия"
                   (DIF_BTNNOCLOSE), то ничего не делаем, иначе -
                   предлагаем закрыть диалог.
     2) Если нажали Enter не на DI_BUTTON, то отрабатываем как и раньше
        при условии, что есть кнопка с Default=1 и у этой кнопки не
        выставлен флаг DIF_BTNNOCLOSE.
  05.12.2000 IS
   ! Все удалил в блоке, где проверяем, автодополнять или нет,
     и написал заново
  04.12.2000 SVS
   - перебор с DIF_NOFOCUS :-(
   - При удаление в предыдущем патче - прихватил нужную строку :-(
   ! Для DI_USERCONTROL - для копирования Data используется memmove()
  04.12.2000 SVS
   ! Проблемы с фокусом ввода:
     "Ctrl-A Alt-N Enter - фокус ввода не устанавливался в поле даты."
   ! Автодополнение - чтобы не работало во время проигрывания макросов.
   ! Оптимизация функций ConvertItem() и DataToItem() - с указателями
     будет генериться компактный и быстрый код (MSVC - это сам делает :-(
   + DIF_3STATE - 3-х уровневый CheckBox
   ! Уточним на что влияет флаг DIF_DROPDOWNLIST - только для DI_COMBOBOX.
   - Исключаем из списка оповещаемых о мыши недоступные элементы
   ! Для DI_PSWEDIT и DI_FIXEDIT обработка DIF_EDITEXPAND не нужна
   ! Отрисовка Disabled
  03.12.2000 IS
   ! Не автодополнять, если после курсора есть невыделенные символы.
     Работает это правило, естественно, только с постоянными блоками.
  21.11.2000 SVS
   - Не стиралась последняя строка в многострочном редакторе
  08.11.2000 SVS
   ! Изменен пересчет кодов клавишь для hotkey (используются сканкоды)
  04.11.2000 SVS
   + Проверка на альтернативную клавишу при XLat-перекодировке
  26.10.2000 SVS
   ! DM_SETEDITPOS/DM_GETEDITPOS -> DM_SETCURSORPOS/DM_GETCURSORPOS
   + Работа с курсором для DI_USERCONTROL в ограниченном варианте.
  23.10.2000 SVS
   + DM_SETEDITPOS, DM_GETEDITPOS -
      позиционирование курсора в строках редактирования.
  20.10.2000 SVS
   + DM_GETFOCUS - получить ID элемента имеющего фокус ввода
  16.10.2000 tran 1.47
   + для EDIT полей выставляется ограничение в 511 символов
  27.09.2000 SVS
   ! Alt-Up/Down/Left/Right - убрал (чтобы в будущем не пересекались
     с MultiEdit)
   ! Ctrl-Alt-Shift - реагируем, если надо.
  24.09.2000 SVS
   + Движение диалога - Alt-стрелки
   + вызов функции Xlat
  22.09.2000 SVS
   ! Уточнение AutoComplete при постоянных блоках.
  20.09.2000 SVS
   ! Enter в строках ввода (кроме DIF_EDITOR) завершает диалог.
  18.09.2000 SVS
   + DIF_READONLY - флаг для строк редактирования
      (пока! для строк редактирования).
  18.09.2000 SVS
   ! Уточнения для SelectOnEntry
   ! Маска не должна быть пустой (строка из пробелов не учитывается)!
  14.09.2000 SVS
   + Флаг DIF_LISTNOAMPERSAND. По умолчанию для DI_LISTBOX
      выставляется флаг MENU_SHOWAMPERSAND. Этот флаг подавляет такое
      поведение
  12.09.2000 SVS
   ! Задаем поведение для кнопки с DefaultButton=1:
     Такая кнопка независимо от стиля диалога инициирует сообщение DM_CLOSE.
   ! Исправляем ситуацию с BackSpace в DIF_EDITOR
   ! Решаем проблему, если Del нажали в позиции большей, чем длина
     строки (DIF_EDITOR)
  11.09.2000 SVS
   + Ctrl-U в строках ввода снимает пометку блока
  09.09.2000 SVS
   + DIF_NOFOCUS - элемент не получает фокуса ввода (клавиатурой)
   + Стиль диалога DMODE_OLDSTYLE - диалог в старом стиле.
   ! вывалить из диалога, ткнув вне диалога, сможем только в старом стиле.
   + Функция SelectOnEntry - выделение строки редактирования
     (Обработка флага DIF_SELECTONENTRY)
  08.09.2000 SVS
   - Если коротко, то DM_SETFOCUS вроде как и работал :-)
   ! Уточнение для DN_MOUSECLICK
  31.08.2000 SVS
   + DM_ENABLE (не полностью готов :-)
   - Бага с вызовом файлов помощи.
  30.08.2000 SVS
   + Метод Hide()
   + Режим диалога DMODE_SHOW - Диалог виден?
   ! уточнения для IsEnableRedraw
   + DM_MOVEDIALOG - перемещение диалога.
   ! Изменение цветов для ComboBox(DowpDownList)
  29.08.2000 SVS
   ! При подмене темы помощи из диаловой процедуры...
     короче, нужно вновь формировать контент!
  29.08.2000 SVS
   - Первый официальный альфа-баг - функция ProcessHighlighting
     MY> Работа с диалогами стала ГЛЮЧНАЯ. Я имею в виду горячие клавиши.
     MY> Входим в настройку чего угодно, жмем Alt-нужную букву и
     MY> наблюдаем разнообразные глюки.
  28.08.2000 SVS
   - баг рук кривых (или не внимательности!) :-)
  25.08.2000 SVS
   + DM_GETDLGRECT - получить координаты диалогового окна
   ! Уточнение для DN_MOUSECLICK
  25.08.2000 SVS
   ! Уточнения, относительно фокуса ввода - ох уж эти сказочки, блин.
  24.08.2000 SVS
   + InitDialogObjects имеет параметр - для выборочной реинициализации
     элементов
  24.08.2000 SVS
   ! Охрененная дыра!!!! (дальше только матом)... это про ComboBox
     Этого лучше не знать...
   + CtrlAltShift - спрятать/показать диалог...
   + Элемент DI_USERCONTROL - отрисовкой занимается плагин.
  24.08.2000 SVS
   ! Критическая ошибка - с фокусов не порядки...
     перестарался с ChangeFocus()
  23.08.2000 SVS
   ! Уточнения категорий DMSG_* -> DM_ (месаг) & DN_ (нотифи)
   + DM_KEY        - послать/получить клавишу(ы)
   + DM_GETDLGDATA - взять данные диалога.
   + DM_SETDLGDATA - установить данные диалога.
   + DM_SHOWDIALOG - показать/спрятать диалог
   + Переменная класса FocusPos - всегда известно какой элемент в фокусе
   ! Переменные IsCanMove, InitObjects, CreateObjects, WarningStyle, Dragged
     удалены -> битовые флаги в DialogMode
   + множество мелких уточнений ;-)
  22.08.2000 SVS
   ! С моим английским вообще ни как :-((
     IsMovedDialog -> IsCanMove
   ! DMSG_PAINT -> DMSG_DRAWDIALOG
   ! DMSG_DRAWITEM -> DMSG_DRAWDLGITEM
   ! DMSG_CHANGELIST -> DMSG_LISTCHANGE
   ! ShowDialog - дополнительный параметр - какой элемент отрисовывать
  21.08.2000 SVS 1.23
   ! Описание сообщений убрано - смотрите в хелпе :-)))
   ! Самыми последними в этом файле должны быть DefDlgProc и SendDlgMessage
     (мне так удобно :-)
   + Перед отрисовкой DI_LISTBOX спросим об изменении цветовых атрибутов
   ! DIF_HISTORY имеет более высокий приоритет, чем DIF_MASKEDIT
   - глюк в AutoComplete при включенных постоянных блоках
   ! DMSG_CHANGEITEM -> DMSG_EDITCHANGE
   + DMSG_BTNCLICK
  12.08.2000 KM 1.20
   + Добавление работы DI_FIXEDIT с флагом DIF_MASKEDIT для установки
     маски в строку ввода.
  18.08.2000 SVS
   + DialogMode - Флаги текущего режима диалога
   + Флаг IsEnableRedraw - разрешающий/запрещающий перерисовку диалога
   + Сообщения: DMSG_ENABLEREDRAW, DMSG_MOUSECLICK
   + DI_BUTTON тоже теперь может иметь DIF_SETCOLOR
   + Флаг для DI_BUTTON - DIF_BTNNOCLOSE - "кнопка не для закрытия диалога"
   - Если на выход процедура ответила "НЕТ", то диалог зацикливался, т.к.
     не был сброшен флаг выхода.
  15.08.2000 SVS
   ! Для DropDownList цвета обрабатываем по иному.
   + Сделаем так, чтобы ткнув мышкой в DropDownList список раскрывался сам.
  11.08.2000 SVS
   + Данные, специфические для конкретного экземпляра диалога
   + Для того, чтобы послать DMSG_CLOSE нужно переопределить Process
   ! Уточнение для DMSG_CLOSE
  11.08.2000 KM 1.18
   ! Убран дублирующий код, исправляющий некорректное перемещение диалога
     мышкой. Оказывается мы с Андреем сделали патчик в один день :)
   ! Чуть-чуть переделано таскание мышкой диалога. Было: после первого нажатия
     мышкой на диалоге отображение режима перемещения не включалось до тех пор
     пока не начиналось само движение диалога, после чего и происходило
     отображение начала этого режима, что есть в корне идеологически неверно :)
   ! Артефакт: если после первого запуска фара, не пермещая мышь
     нажать левую кнопку, чтобы переместить диалог (диалог находится под мышью),
     то из-за того, что к этому моменту PrevMouseX и PrevMouseY ещё не определены,
     также неопределённым получался прыжок диалога.
  10.08.2000 SVS
   + переменная IsMovedDialog - можно ли двигать диалог :-)
   + функция установки IsMovedDialog
  09.08.2000 tran 1.16
   - убраны "салазки"
  09.08.2000 KM
   ! При включении режима перемещения диалога добавлено
     отключение мигающего курсора. Так косметика.
   ! Поправлено перемещение диалога мышкой, Андрей-то не захотел
     исправлять :). Теперь перемещение стало корректным, без прокатывания
     диалога по экрану.
   ! При выходе за край экрана диалогом, тень, по возможности, продолжает
     рисоваться.

     Номер редакции остался прежним - здесь было
     забегание вперёд.
  07.08.2000 SVS
   + В Функции выравнивания кооржинат про ListBox забыли!
  04.08.2000 SVS
    + FarListItems.CountItems -> FarListItems.ItemsNumber
  03.08.2000 tran
   + мышиный перетаск диалога - хватание за пустое место
     внимание - ограничение невыхода за границы экрана посталено не зря
     иначе ползут глюки...
     изменена индикация - по углам красные палки
     с MOVE в угла могут быть глюки в изображении.
     строки про MOVE закоментарены "//# "
  01.08.2000 SVS
   ! History теперь ВСЕГДА имеет ScrollBar, т.к. этот элемент ближе
     к ComboBox`у, нежели к меню.
   - переменная класса lastKey удалена за ненадобностью :-)
   + Обычный ListBox
   - Небольшой глючек с AutoComplete
   ! В History должно заносится значение (для DIF_EXPAND...) перед
     расширением среды!
   + Еже ли стоит флаг DIF_USELASTHISTORY и непустая строка ввода,
     то подстанавливаем первое значение из History
   - Отключена возможность для DI_PSWEDIT иметь History...
     ...Что бы небыло повадно... и для повыщения защиты, т.с.
  31.07.2000 tran & SVS
   + перемещение диалога по экрану клавишами. Ctrl-F5 включает режим
     перемещения. Индикация перемещения - "Move" в левом верхнем углу
  28.07.2000 SVS
   ! Переметр Edit *EditLine в функции FindInEditForAC нафиг ненужен!
   - Небольшой баг с автозавершением:
       ...Есть в хистори на F7 - "templates". Жму F7, нажимаю shift+t=T и ...
       получаю маленькую t. В итоге большую добился только набиранием её
       после маленькой и стиранием оной...
   - Если плагин не выставил ни одного эелемента с фокусом,
     то придется самому об этом позаботиться, и выставить
     фокус на первом вразумительном элементе ;-)
   + AutoComplite: Для DI_COMBOBOX.
   ! SelectFromComboBox имеет дополнительный параметр с тем, чтобы
     позиционировать item в меню со списком в соответсвии со строкой ввода
   ! FindInEditHistory -> FindInEditForAC
     Поиск как в истории, так и в ComboBox`е (чтобы не пладить кода)
   + Функция IsFocused, определяющая - "Может ли элемент диалога
     иметь фокус ввода"
   + Функция ConvertItem - преобразования из внутреннего представления
     в FarDialogItem и обратно
   + Некоторое количество сообщений:
        DMSG_INITDIALOG, DMSG_ENTERIDLE, DMSG_HELP, DMSG_PAINT,
        DMSG_SETREDRAW, DMSG_DRAWITEM, DMSG_GETDLGITEM, DMSG_KILLFOCUS,
        DMSG_GOTFOCUS, DMSG_SETFOCUS, DMSG_GETTEXTLENGTH, DMSG_GETTEXT,
        DMSG_CTLCOLORDIALOG, DMSG_CTLCOLORDLGITEM, DMSG_CTLCOLORDLGLIST,
        DMSG_SETTEXTLENGTH, DMSG_SETTEXT, DMSG_CHANGEITEM, DMSG_HOTKEY,
        DMSG_CLOSE,
  26.07.2000 SVS
   + Ну наконец-то - долгожданный нередактируемый ComboBox
  26.07.2000 SVS
   + AutoComplite: Для DIF_HISTORY.
  25.07.2000 SVS
   + Новый параметр в конструкторе
  23.07.2000 SVS
   + Куча ремарок в исходниках :-)
   + Изменен вызов конструтора - добавка в виде функции обработки
   ! Строковые константы "SavedDialogHistory\\%s",
     "Locked%d" и "Line%d" сделаны поименованными.
   + Функция обработки диалога (по умолчанию) DefDlgProc() - забито место :-)
  19.07.2000 SVS
   ! "...В редакторе команд меню нажмите home shift+end del
     блок не удаляется..."
     DEL у итемов, имеющих DIF_EDITOR, работал без учета выделения...
  18.07.2000 SVS
   + Обработка элемента DI_COMBOBOX (пока все еще редактируемого)
   + Функция-обработчик выбора из списка - SelectFromComboBox
  11.07.2000 SVS
   ! Изменения для возможности компиляции под BC & VC
  05.07.2000 SVS
   + добавлена проверка на флаг DIF_EDITEXPAND - расширение переменных
     среды в элементе диалога DI_EDIT
  25.06.2000 SVS
   ! Подготовка Master Copy
   ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */


static char fmtLocked[]="Locked%d";
static char fmtLine[]  ="Line%d";
static char fmtSavedDialogHistory[]="SavedDialogHistory\\%s";
static char *CheckBox3State=NULL;

//////////////////////////////////////////////////////////////////////////
/* Public:
   Конструктор класса Dialog
*/
Dialog::Dialog(struct DialogItem *Item,int ItemCount,
               FARWINDOWPROC DlgProc,long InitParam)
{
  /* $ 04.12.2000 SVS
     Если надо - загрузим символ, представляющий 3-е состояние CheckBox
  */
  if(!CheckBox3State)
    CheckBox3State=MSG(MCheckBox2State);
  /* SVS $ */
  /* $ 29.08.2000 SVS
    Номер плагина, вызвавшего диалог (-1 = Main)
  */
  PluginNumber=-1;
  /* SVS $ */
  /* $ 23.08.2000 SVS
    Режимы диалога.
  */
  DialogMode=0;
  /* SVS $ */
  /* $ 11.08.2000 SVS
    + Данные, специфические для конкретного экземпляра диалога
  */
  Dialog::DataDialog=InitParam;
  /* SVS $ */
  DialogTooLong=0;
  /* $ 10.08.2000 SVS
     Изначально диалоги можно таскать
  */
  SetDialogMode(DMODE_ISCANMOVE);
  /* SVS $ */
  /* $ 18.08.2000 SVS
  */
  /*
    + Флаг IsEnableRedraw - разрешающий/запрещающий перерисовку диалога
      =0 - разрешена, другие значение - не перерисовывать
        Когда посылается сообщение DMSG_ENABLEREDRAW, то этот флаг
        при Param1=TRUE увеличивается, при Param1 = FALSE - уменьшается
  */
  IsEnableRedraw=0;

  FocusPos=-1;
  PrevFocusPos=-1;

  if(!DlgProc) // функция должна быть всегда!!!
  {
    DlgProc=(FARWINDOWPROC)Dialog::DefDlgProc;
    // знать диалог в старом стиле - учтем этот факт!
    SetDialogMode(DMODE_OLDSTYLE);
  }
  Dialog::DlgProc=DlgProc;

  Dialog::Item=Item;
  Dialog::ItemCount=ItemCount;

  if (CtrlObject!=NULL)
  {
    // запомним пред. режим макро.
    PrevMacroMode=CtrlObject->Macro.GetMode();
    // макросить будет в диалогах :-)
    CtrlObject->Macro.SetMode(MACRO_DIALOG);
  }

  // запоминаем предыдущий заголовок консоли
  GetConsoleTitle(OldConsoleTitle,sizeof(OldConsoleTitle));
}


//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Деструктор класса Dialog
*/
Dialog::~Dialog()
{
  INPUT_RECORD rec;

  GetDialogObjectsData();
  DeleteDialogObjects();

  if (CtrlObject!=NULL)
    CtrlObject->Macro.SetMode(PrevMacroMode);

  Hide();
  ScrBuf.Flush();

  PeekInputRecord(&rec);
  SetConsoleTitle(OldConsoleTitle);
}


//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Расчет значений координат окна диалога и вызов функции
   ScreenObject::Show() для вывода диалога на экран.
*/
void Dialog::Show()
{
  if (!CheckDialogMode(DMODE_INITOBJECTS))      // самодостаточный вариант, когда
  {                      //  элементы инициализируются при первом вызове.
    /* $ 28.07.2000 SVS
       Укажем процедуре, что у нас все Ок!
    */
    CheckDialogCoord();
    if(DlgProc((HANDLE)this,DN_INITDIALOG,InitDialogObjects(),DataDialog))
    {
      // еще разок, т.к. данные могли быть изменены
      InitDialogObjects();
    }
    // все объекты проинициализированы!
    SetDialogMode(DMODE_INITOBJECTS);
  }
  CheckDialogCoord();
  ScreenObject::Show();
}

void Dialog::CheckDialogCoord(void)
{
  if (X1 == -1) // задано центрирование диалога по горизонтали?
  {             //   X2 при этом = ширине диалога.
    X1=(ScrX - X2 + 1)/2;

    if (X1 <= 0) // ширина диалога больше ширины экрана?
    {
      DialogTooLong=X2-1;
      X1=0;
      X2=ScrX;
    }
    else
      X2+=X1-1;
  }

  if (Y1 == -1) // задано центрирование диалога по вертикали?
  {             //   Y2 при этом = высоте диалога.
    Y1=(ScrY - Y2 + 1)/2;

    if (Y1>1)
      Y1--;
    if (Y1>5)
      Y1--;
    if (Y1<0)
    {
       Y1=0;
       Y2=ScrY;
    }
    else
      Y2+=Y1-1;
  }
}

/* $ 30.08.2000 SVS
  Цель перехвата данной функции - управление видимостью...
*/
void Dialog::Hide()
{
  ScreenObject::Hide();
  SkipDialogMode(DMODE_SHOW);
}
/* SVS $*/

//////////////////////////////////////////////////////////////////////////
/* Private, Virtual:
   Инициализация объектов и вывод диалога на экран.
*/
void Dialog::DisplayObject()
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  Shadow();              // "наводим" тень
  ShowDialog();          // "нарисуем" диалог.
}


//////////////////////////////////////////////////////////////////////////
/* Public:
   Инициализация элементов диалога.
*/
/* $ 28.07.2000 SVS
   Теперь InitDialogObjects возвращает ID элемента
   с фокусом ввода
*/
/* $ 24.08.2000 SVS
  InitDialogObjects имеет параметр - для выборочной реинициализации
  элементов. ID = -1 - касаемо всех объектов
*/
int Dialog::InitDialogObjects(int ID)
{
  int I, J, TitleSet;
  int Length,StartX;
  int Type;
  struct DialogItem *CurItem;
  int InitItemCount;
  DWORD ItemFlags;

  if(ID+1 > ItemCount)
    return -1;

  if(ID == -1) // инициализируем все?
  {
    ID=0;
    InitItemCount=ItemCount;
  }
  else
  {
    InitItemCount=ID+1;
  }

  /* 04.01.2001 SVS
     если FocusPos в пределах и элемент задисаблен, то ищем сначала. */
  if(FocusPos >= 0 && FocusPos < ItemCount &&
     (Item[FocusPos].Flags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
    FocusPos = -1; // будем искать сначала!
  /* SVS $ */

  // предварительный цикл по поводу кнопок и заголовка консоли
  for(I=ID, TitleSet=0; I < InitItemCount; I++)
  {
    CurItem=&Item[I];
    ItemFlags=CurItem->Flags;

    // для кнопок не имеющи стиля "Показывает заголовок кнопки без скобок"
    //  добавим энти самые скобки
    if (CurItem->Type==DI_BUTTON &&
        (ItemFlags & DIF_NOBRACKETS)==0 &&
        *CurItem->Data != '[')
    {
      char BracketedTitle[200];
      sprintf(BracketedTitle,"[ %s ]",CurItem->Data);
      strcpy(CurItem->Data,BracketedTitle);
    }

    // по первому попавшемуся "тексту" установим заголовок консоли!
    if (!TitleSet &&             // при условии, что еще не устанавливали
         (CurItem->Type==DI_TEXT ||
          CurItem->Type==DI_DOUBLEBOX ||
          CurItem->Type==DI_SINGLEBOX))
      for (J=0;CurItem->Data[J]!=0;J++)
        if (LocalIsalpha(CurItem->Data[J]))
        {
          SetFarTitle(CurItem->Data+J);
          TitleSet=TRUE;
          break;
        }

     // предварительный поик фокуса
     if(FocusPos == -1 &&
        IsFocused(CurItem->Type) &&
        CurItem->Focus &&
        !(ItemFlags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
       FocusPos=I; // запомним первый фокусный элемент
     CurItem->Focus=0; // сбросим для всех, чтобы не оказалось,
                       //   что фокусов - как у дурочка фантиков
  }

  // Опять про фокус ввода - теперь, если "чудо" забыло выставить
  // хотя бы один, то ставим на первый подходящий
  if(FocusPos == -1)
  {
    for (I=0; I < ItemCount; I++) // по всем!!!!
    {
      CurItem=&Item[I];
      if(IsFocused(CurItem->Type) &&
         !(CurItem->Flags&(DIF_DISABLE|DIF_NOFOCUS|DIF_HIDDEN)))
      {
        FocusPos=I;
        break;
      }
    }
  }
  if(FocusPos == -1) // ну ни хрена себе - нет ни одного
  {                  //   элемента с возможностью фокуса
     FocusPos=0;     // убится, блин
  }

  // ну вот и добрались до!
  Item[FocusPos].Focus=1;

  // а теперь все сначала и по полной программе...
  for (I=ID; I < InitItemCount; I++)
  {
    CurItem=&Item[I];
    Type=CurItem->Type;
    ItemFlags=CurItem->Flags;

    // Последовательно объявленные элементы с флагом DIF_CENTERGROUP
    // и одинаковой вертикальной позицией будут отцентрированы в диалоге.
    // Их координаты X не важны. Удобно использовать для центрирования
    // групп кнопок.
    if ((ItemFlags & DIF_CENTERGROUP) &&
        (I==0 ||
        (Item[I-1].Flags & DIF_CENTERGROUP)==0 ||
        Item[I-1].Y1!=CurItem->Y1))
    {
      Length=0;

      for (J=I; J < ItemCount &&
                (Item[J].Flags & DIF_CENTERGROUP) &&
                Item[J].Y1==Item[I].Y1; J++)
      {
        Length+=HiStrlen(Item[J].Data);

        if (Item[J].Type==DI_BUTTON && *Item[J].Data!=' ')
          Length+=2;
      }

      if (Item[I].Type==DI_BUTTON && *Item[I].Data!=' ')
        Length-=2;

      StartX=(X2-X1+1-Length)/2;

      if (StartX<0)
        StartX=0;

      for (J=I; J < ItemCount &&
                (Item[J].Flags & DIF_CENTERGROUP) &&
                Item[J].Y1==Item[I].Y1; J++)
      {
        Item[J].X1=StartX;
        StartX+=HiStrlen(Item[J].Data);

        if (Item[J].Type==DI_BUTTON && *Item[J].Data!=' ')
          StartX+=2;
      }
    }
    /* $ 01.08.2000 SVS
       Обычный ListBox
    */
    if (Type==DI_LISTBOX)
    {
      if (!CheckDialogMode(DMODE_CREATEOBJECTS))
        CurItem->ObjPtr=new VMenu(NULL,NULL,0,CurItem->Y2-CurItem->Y1+1,
                               VMENU_ALWAYSSCROLLBAR|VMENU_LISTBOX,NULL/*,this*/);

      VMenu *ListBox=(VMenu *)CurItem->ObjPtr;

      if(ListBox)
      {
        // удалим все итемы
        ListBox->DeleteItems();

        struct MenuItem ListItem={0};
        /* $ 13.09.2000 SVS
           + Флаг DIF_LISTNOAMPERSAND. По умолчанию для DI_LISTBOX &
             DI_COMBOBOX выставляется флаг MENU_SHOWAMPERSAND. Этот флаг
             подавляет такое поведение
        */
        if(!(ItemFlags&DIF_LISTNOAMPERSAND))
          ListBox->SetFlags(MENU_SHOWAMPERSAND);
        /* SVS $*/
        ListBox->SetPosition(X1+CurItem->X1,Y1+CurItem->Y1,
                             X1+CurItem->X2,Y1+CurItem->Y2);
        ListBox->SetBoxType(SHORT_SINGLE_BOX);

        struct FarList *List=CurItem->ListItems;
        if(List && List->Items)
        {
          struct FarListItem *Items=List->Items;
          for (J=0; J < List->ItemsNumber; J++)
          {
            ListItem.Separator=Items[J].Flags&LIF_SEPARATOR;
            ListItem.Selected=Items[J].Flags&LIF_SELECTED;
            ListItem.Checked=Items[J].Flags&LIF_CHECKED;
            ListItem.Disabled=Items[J].Flags&LIF_DISABLE;
            // здесь нужно добавить проверку на LIF_PTRDATA!!!
            ListItem.Flags=0;
            ListItem.PtrData=NULL;
            if(Items[J].Flags&LIF_PTRDATA)
            {
              strncpy(ListItem.Name,Items[J].Ptr.PtrData,sizeof(ListItem.Name));
              ListItem.UserDataSize=Items[J].Ptr.PtrLength;
              if(Items[J].Ptr.PtrLength > sizeof(ListItem.UserData))
              {
                ListItem.PtrData=Items[J].Ptr.PtrData;
                ListItem.Flags=1;
              }
              else
                memmove(ListItem.UserData,Items[J].Ptr.PtrData,Items[J].Ptr.PtrLength);
            }
            else
            {
              strncpy(ListItem.Name,Items[J].Text,sizeof(ListItem.Name));
              strncpy(ListItem.UserData,Items[J].Text,sizeof(ListItem.UserData));
              ListItem.UserDataSize=strlen(Items[J].Text);
            }

            ListBox->AddItem(&ListItem);
          }
        }
      }
    }
    /* SVS $*/
    // "редакторы" - разговор особый...
    if (IsEdit(Type))
    {
      if (!CheckDialogMode(DMODE_CREATEOBJECTS))
        CurItem->ObjPtr=new Edit;

      Edit *DialogEdit=(Edit *)CurItem->ObjPtr;
      /* $ 26.07.2000 SVS
         Ну наконец-то - долгожданный нередактируемый ComboBox
      */
      /* $ 30.11.200 SVS
         Уточним на что влияет флаг DIF_DROPDOWNLIST
      */
      if ((ItemFlags & DIF_DROPDOWNLIST) && Type == DI_COMBOBOX)
      {
         DialogEdit->DropDownBox=1;
      }
      /* SVS $ */
      /* SVS $ */
      /* $ 18.09.2000 SVS
         ReadOnly!
      */
      if (ItemFlags & DIF_READONLY)
      {
         DialogEdit->ReadOnly=1;
      }
      /* SVS $ */
      /* $ 15.10.2000 tran
        строка редакторирование должна иметь максимум в 511 символов */
      // выставляем максимальный размер в том случае, если он еще не выставлен
      if(DialogEdit->GetMaxLength() == -1)
      {
        if((CurItem->Type==DI_EDIT || CurItem->Type==DI_COMBOBOX) &&
           (ItemFlags&DIF_VAREDIT))
          DialogEdit->SetMaxLength(CurItem->Ptr.PtrLength);
        else
          DialogEdit->SetMaxLength(511);
      }
      /* tran $ */
      DialogEdit->SetPosition(X1+CurItem->X1,Y1+CurItem->Y1,
                              X1+CurItem->X2,Y1+CurItem->Y2);
      DialogEdit->SetObjectColor(
         FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ?
             ((ItemFlags&DIF_DISABLE)?COL_WARNDIALOGEDITDISABLED:COL_WARNDIALOGEDIT):
             ((ItemFlags&DIF_DISABLE)?COL_DIALOGEDITDISABLED:COL_DIALOGEDIT)),
         FarColorToReal((ItemFlags&DIF_DISABLE)?COL_DIALOGEDITDISABLED:COL_DIALOGEDITSELECTED));
      if (CurItem->Type==DI_PSWEDIT)
      {
        DialogEdit->SetPasswordMode(TRUE);
        /* $ 01.08.2000 SVS
          ...Что бы небыло повадно... и для повыщения защиты, т.с.
        */
        ItemFlags&=~DIF_HISTORY;
        /* SVS $ */
      }

      if (Type==DI_FIXEDIT)
      {
        /* $ 21.08.2000 SVS
           DIF_HISTORY имеет более высокий приоритет, чем DIF_MASKEDIT
        */
        if(ItemFlags&DIF_HISTORY)
          ItemFlags&=~DIF_MASKEDIT;
        /* SVS $ */
        // если DI_FIXEDIT, то курсор сразу ставится на замену...
        //   ай-ай - было недокументированно :-)
        DialogEdit->SetMaxLength(CurItem->X2-CurItem->X1+1);
        DialogEdit->SetOvertypeMode(TRUE);
        /* $ 12.08.2000 KM
           Если тип строки ввода DI_FIXEDIT и установлен флаг DIF_MASKEDIT
           и непустой параметр CurItem->Mask, то вызываем новую функцию
           для установки маски в объект Edit.
        */
        /* $ 18.09.2000 SVS
          Маска не должна быть пустой (строка из пробелов не учитывается)!
        */
        if ((ItemFlags & DIF_MASKEDIT) && CurItem->Mask)
        {
          char *Ptr=CurItem->Mask;
          while(*Ptr && *Ptr == ' ') ++Ptr;
          if(*Ptr)
            DialogEdit->SetInputMask(CurItem->Mask);
          else
          {
            CurItem->Mask=NULL;
            ItemFlags&=~DIF_MASKEDIT;
          }
        }
        /* SVS $ */
        /* KM $ */
      }
      else
        // "мини-редактор"
        // Последовательно определенные поля ввода (edit controls),
        // имеющие этот флаг группируются в редактор с возможностью
        // вставки и удаления строк
        if (!(ItemFlags & DIF_EDITOR))
        {
          DialogEdit->SetEditBeyondEnd(FALSE);
          DialogEdit->SetClearFlag(1);
        }

      /* $ 01.08.2000 SVS
         Еже ли стоит флаг DIF_USELASTHISTORY и непустая строка ввода,
         то подстанавливаем первое значение из History
      */
      if((ItemFlags&(DIF_HISTORY|DIF_USELASTHISTORY)) == (DIF_HISTORY|DIF_USELASTHISTORY))
      {
        char RegKey[80];
        char *PtrData;
        int PtrLength;
        if((CurItem->Type==DI_EDIT || CurItem->Type==DI_COMBOBOX) && (ItemFlags&DIF_VAREDIT))
        {
          PtrData  =(char *)CurItem->Ptr.PtrData;
          PtrLength=CurItem->Ptr.PtrLength;
        }
        else
        {
          PtrData  =CurItem->Data;
          PtrLength=sizeof(CurItem->Data);
        }
        if(!PtrData[0])
        {
          sprintf(RegKey,fmtSavedDialogHistory,(char*)CurItem->History);
          GetRegKey(RegKey,"Line0",PtrData,"",PtrLength);
        }
      }
      /* SVS $ */
      if((ItemFlags&DIF_MANUALADDHISTORY) && !(ItemFlags&DIF_HISTORY))
        ItemFlags&=~DIF_MANUALADDHISTORY; // сбросим нафиг.

      /* $ 18.03.2000 SVS
         Если это ComBoBox и данные не установлены, то берем из списка
         при условии, что хоть один из пунктов имеет Selected != 0
      */
      if (Type==DI_COMBOBOX &&
          (!(ItemFlags&DIF_VAREDIT) && CurItem->Data[0] == 0 ||
            (ItemFlags&DIF_VAREDIT) && *(char*)CurItem->Ptr.PtrData == 0) &&
          CurItem->ListItems)
      {
        struct FarListItem *ListItems=CurItem->ListItems->Items;
        int Length=CurItem->ListItems->ItemsNumber;

        for (J=0; J < Length; J++)
        {
          if(ListItems[J].Flags & LIF_SELECTED)
          {
            // берем только первый пункт для области редактирования
            if(ItemFlags&DIF_VAREDIT)
            {
              if(ListItems[J].Flags&LIF_PTRDATA)
                strncpy((char *)CurItem->Ptr.PtrData, ListItems[J].Ptr.PtrData,CurItem->Ptr.PtrLength);
              else
                strncpy((char *)CurItem->Ptr.PtrData, ListItems[J].Text,CurItem->Ptr.PtrLength);
            }
            else
            {
              if(ListItems[J].Flags&LIF_PTRDATA)
                strncpy((char *)CurItem->Data, ListItems[J].Ptr.PtrData,sizeof(CurItem->Data));
              else
                strcpy(CurItem->Data, ListItems[J].Text);
            }
            break;
          }
        }
      }
      /* SVS $ */
      if((CurItem->Type==DI_EDIT || CurItem->Type==DI_COMBOBOX) &&
         (ItemFlags&DIF_VAREDIT))
        DialogEdit->SetString((char *)CurItem->Ptr.PtrData);
      else
        DialogEdit->SetString(CurItem->Data);

      if (Type==DI_FIXEDIT)
        DialogEdit->SetCurPos(0);
    }
    if (Type == DI_USERCONTROL)
    {
      if (!CheckDialogMode(DMODE_CREATEOBJECTS))
        CurItem->ObjPtr=new COORD; // пока ограничимся хранением координат курсора
      ((COORD *)(CurItem->ObjPtr))->X=-1;
      ((COORD *)(CurItem->ObjPtr))->Y=-1;
    }

    CurItem->Flags=ItemFlags;
  }
  // если будет редактор, то обязательно будет выделен.
  SelectOnEntry(FocusPos);

  // все объекты созданы!
  SetDialogMode(DMODE_CREATEOBJECTS);
  return FocusPos;
}
/* 24.08.2000 SVS $ */


BOOL Dialog::GetItemRect(int I,RECT& Rect)
{
  if(I >= ItemCount)
    return FALSE;

  struct DialogItem *CurItem=&Item[I];
  DWORD ItemFlags=CurItem->Flags;
  int Type=CurItem->Type;

  Rect.left=(int)CurItem->X1;
  Rect.top=(int)CurItem->Y1;
  Rect.right=(int)CurItem->X2;
  Rect.bottom=(int)CurItem->Y2;

  switch(Type)
  {
    case DI_TEXT:
      if (CurItem->X1==(unsigned char)-1)
        Rect.left=(X2-X1+1-((ItemFlags & DIF_SHOWAMPERSAND)?
                                 strlen(CurItem->Data):
                                 HiStrlen(CurItem->Data)))/2;
      if(Rect.left < 0)
        Rect.left=0;

      if (CurItem->Y1==(unsigned char)-1)
        Rect.top=(Y2-Y1+1)/2;

      if(Rect.top < 0)
        Rect.top=0;

      if (ItemFlags & DIF_SEPARATOR)
      {
        Rect.bottom=Rect.top;
        Rect.left=3;
        Rect.right=X2-X1-5; //???
        break;
      }

    case DI_BUTTON:
      Rect.bottom=Rect.top;
      Rect.right=Rect.left+((ItemFlags & DIF_SHOWAMPERSAND)?
                                 strlen(CurItem->Data):
                                 HiStrlen(CurItem->Data));
      break;

    case DI_CHECKBOX:
    case DI_RADIOBUTTON:
      Rect.bottom=Rect.top;
      Rect.right=Rect.left+((ItemFlags & DIF_SHOWAMPERSAND)?
                                 strlen(CurItem->Data):
                                 HiStrlen(CurItem->Data))+
                                 (Type == DI_CHECKBOX?4:
                                   (ItemFlags & DIF_MOVESELECT?3:4)
                                 );
      break;

    case DI_VTEXT:
      Rect.right=Rect.left;
      Rect.bottom=Rect.top+strlen(CurItem->Data);
      break;

    case DI_COMBOBOX:
    case DI_EDIT:
    case DI_FIXEDIT:
    case DI_PSWEDIT:
      Rect.bottom=Rect.top;
      break;
  }
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////
/* Private:
   Получение данных и удаление "редакторов"
*/
void Dialog::DeleteDialogObjects()
{
  int I;
  struct DialogItem *CurItem;

  for (I=0; I < ItemCount; I++)
  {
    if((CurItem=Item+I)->ObjPtr)
      switch(CurItem->Type)
      {
        case DI_EDIT:
        case DI_FIXEDIT:
        case DI_PSWEDIT:
        case DI_COMBOBOX:
          delete (Edit *)(CurItem->ObjPtr);
          break;
        case DI_LISTBOX:
          delete (VMenu *)(CurItem->ObjPtr);
          break;
        case DI_USERCONTROL:
          delete (COORD *)(CurItem->ObjPtr);
          break;
      }
   }
}


//////////////////////////////////////////////////////////////////////////
/* Public:
   Сохраняет значение из полей редактирования.
   При установленном флаге DIF_HISTORY, сохраняет данные в реестре.
*/
void Dialog::GetDialogObjectsData()
{
  int I;
  struct DialogItem *CurItem;

  for (I=0; I < ItemCount; I++)
  {
    if((CurItem=Item+I)->ObjPtr)
      switch(CurItem->Type)
      {
        case DI_EDIT:
        case DI_FIXEDIT:
        case DI_PSWEDIT:
        case DI_COMBOBOX:
        {
          char *PtrData;
          int PtrLength;
          Edit *EditPtr=(Edit *)(CurItem->ObjPtr);
          // подготовим данные
          if((CurItem->Type==DI_EDIT || CurItem->Type==DI_COMBOBOX) &&
             (CurItem->Flags&DIF_VAREDIT))
          {
            PtrData  =(char *)CurItem->Ptr.PtrData;
            PtrLength=CurItem->Ptr.PtrLength;
          }
          else
          {
            PtrData  =CurItem->Data;
            PtrLength=sizeof(CurItem->Data);
          }

          // получим данные
          EditPtr->GetString(PtrData,PtrLength);

          if (ExitCode>=0 &&
              (CurItem->Flags & DIF_HISTORY) &&
              !(CurItem->Flags & DIF_MANUALADDHISTORY) && // при мануале не добавляем
              CurItem->History &&
              Opt.DialogsEditHistory)
            AddToEditHistory(PtrData,CurItem->History,PtrLength);
          /* $ 01.08.2000 SVS
             ! В History должно заносится значение (для DIF_EXPAND...) перед
              расширением среды!
          */
          /*$ 05.07.2000 SVS $
          Проверка - этот элемент предполагает расширение переменных среды?
          т.к. функция GetDialogObjectsData() может вызываться самостоятельно
          Но надо проверить!*/
          /* $ 04.12.2000 SVS
            ! Для DI_PSWEDIT и DI_FIXEDIT обработка DIF_EDITEXPAND не нужна
             (DI_FIXEDIT допускается для случая если нету маски)
          */
          if((CurItem->Flags&DIF_EDITEXPAND) &&
              CurItem->Type != DI_PSWEDIT &&
              CurItem->Type != DI_FIXEDIT)
             ExpandEnvironmentStr(PtrData, PtrData,PtrLength-1);
          /* SVS $ */
          /* SVS $ */
          /* 01.08.2000 SVS $ */
          break;
        }

        case DI_LISTBOX:
        {
          VMenu *VMenuPtr=(VMenu *)(CurItem->ObjPtr);
          CurItem->ListPos=VMenuPtr->GetSelectPos();
          break;
        }

        /**/
      }
   }
}


//////////////////////////////////////////////////////////////////////////
/* $ 22.08.2000 SVS
  ! ShowDialog - дополнительный параметр - какой элемент отрисовывать
*/
/* Private:
   Отрисовка элементов диалога на экране.
*/
void Dialog::ShowDialog(int ID)
{
  struct DialogItem *CurItem;
  int X,Y;
  int I,DrawItemCount;
  unsigned long Attr;

  /* $ 18.08.2000 SVS
     Если не разрешена отрисовка, то вываливаем.
  */
  if(IsEnableRedraw ||                 // разрешена прорисовка ?
     (ID+1 > ItemCount) ||             // а номер в рамках дозволенного?
     CheckDialogMode(DMODE_DRAWING) || // диалог рисуется?
     !CheckDialogMode(DMODE_INITOBJECTS))
    return;
  /* SVS $ */

  SetDialogMode(DMODE_DRAWING);  // диалог рисуется!!!

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  if(ID == -1) // рисуем все?
  {
    /* $ 28.07.2000 SVS
       Перед прорисовкой диалога посылаем сообщение в обработчик
    */
    if(!DlgProc((HANDLE)this,DN_DRAWDIALOG,0,0))
    {
      SkipDialogMode(DMODE_DRAWING);  // конец отрисовки диалога!!!
      return;
    }
    /* SVS $ */

    /* $ 28.07.2000 SVS
       перед прорисовкой подложки окна диалога...
    */
    Attr=DlgProc((HANDLE)this,DN_CTLCOLORDIALOG,0,
        CheckDialogMode(DMODE_WARNINGSTYLE) ? COL_WARNDIALOGTEXT:COL_DIALOGTEXT);
    SetScreen(X1,Y1,X2,Y2,' ',Attr);
    /* SVS $ */
  }

  if(ID == -1) // рисуем все?
  {
    ID=0;
    DrawItemCount=ItemCount;
  }
  else
  {
    DrawItemCount=ID+1;
  }

  for (I=ID; I < DrawItemCount; I++)
  {
    CurItem=&Item[I];

    if(CurItem->Flags&DIF_HIDDEN)
      continue;

    /* $ 28.07.2000 SVS
       Перед прорисовкой каждого элемента посылаем сообщение
       посредством функции SendDlgMessage - в ней делается все!
    */
    if(!Dialog::SendDlgMessage((HANDLE)this,DN_DRAWDLGITEM,I,0))
       continue;
    /* SVS $ */
    /* $ 28.07.2000 SVS
       перед прорисовкой каждого элемента диалога выясним атритубы отрисовки
    */
    switch(CurItem->Type)
    {
/* ***************************************************************** */
      case DI_USERCONTROL:
        if(CurItem->VBuf)
        {
          PutText(X1+CurItem->X1,Y1+CurItem->Y1,X1+CurItem->X2,Y1+CurItem->Y2,CurItem->VBuf);
          // не забудим переместить курсор, если он спозиционирован.
          if(((COORD *)(CurItem->ObjPtr))->X != -1 &&
             ((COORD *)(CurItem->ObjPtr))->Y != -1 &&
             FocusPos == I)
          {
             MoveCursor(
                ((COORD *)(CurItem->ObjPtr))->X+CurItem->X1+X1,
                ((COORD *)(CurItem->ObjPtr))->Y+CurItem->Y1+Y1
             );
          }
        }
        break; //уже наприсовали :-)))

/* ***************************************************************** */
      case DI_SINGLEBOX:
      case DI_DOUBLEBOX:
      {
        Attr=MAKELONG(
          MAKEWORD(FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ?
                      ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOXTITLE):
                      ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGBOXTITLE)), // Title LOBYTE
                 FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ?
                      ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTTEXT):
                      ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTTEXT))),// HiText HIBYTE
          MAKEWORD(FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ?
                  ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX):
                  ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGBOX)), // Box LOBYTE
                 0)                                               // HIBYTE
        );
        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,Attr);

        Box(X1+CurItem->X1,Y1+CurItem->Y1,X1+CurItem->X2,Y1+CurItem->Y2,
            LOBYTE(HIWORD(Attr)),
            (CurItem->Type==DI_SINGLEBOX) ? SINGLE_BOX:DOUBLE_BOX);

        if (*CurItem->Data)
        {
          char Title[200];
          int XB;

          sprintf(Title," %s ",CurItem->Data);
          XB=X1+CurItem->X1+(CurItem->X2-CurItem->X1+1-HiStrlen(Title))/2;

          if (CurItem->Flags & DIF_LEFTTEXT && X1+CurItem->X1+1 < XB)
            XB=X1+CurItem->X1+1;

          SetColor(Attr&0xFF);
          GotoXY(XB,Y1+CurItem->Y1);
          //HiText(Title,HIBYTE(LOWORD(Attr)));
          if (CurItem->Flags & DIF_SHOWAMPERSAND)
            Text(Title);
          else
            HiText(Title,HIBYTE(LOWORD(Attr)));
        }
        break;
      }

/* ***************************************************************** */
      case DI_TEXT:
      {
        if (CurItem->X1==(unsigned char)-1)
          X=(X2-X1+1-((CurItem->Flags & DIF_SHOWAMPERSAND)?HiStrlen(CurItem->Data):strlen(CurItem->Data)))/2;
        else
          X=CurItem->X1;

        if (CurItem->Y1==(unsigned char)-1)
          Y=(Y2-Y1+1)/2;
        else
          Y=CurItem->Y1;

        if (CurItem->Flags & DIF_SETCOLOR)
          Attr=CurItem->Flags & DIF_COLORMASK;
        else
          if (CurItem->Flags & DIF_BOXCOLOR)
            Attr=CheckDialogMode(DMODE_WARNINGSTYLE) ?
                  ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX):
                  ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGBOX);
          else
            Attr=CheckDialogMode(DMODE_WARNINGSTYLE) ?
                  ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT):
                  ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGTEXT);

        Attr=MAKELONG(
           MAKEWORD(FarColorToReal(Attr),
                   FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ?
                      ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTTEXT):
                      ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTTEXT))), // HIBYTE HiText
             0);
        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,Attr);
        SetColor(Attr&0xFF);

        if (CurItem->Flags & DIF_SEPARATOR)
        {
          GotoXY(X1+3,Y1+Y);
          if (DialogTooLong)
            ShowSeparator(DialogTooLong-5);
          else
            ShowSeparator(X2-X1-5);
        }

        GotoXY(X1+X,Y1+Y);

        if (CurItem->Flags & DIF_SHOWAMPERSAND)
          Text(CurItem->Data);
        else
          HiText(CurItem->Data,HIBYTE(LOWORD(Attr)));

        break;
      }

/* ***************************************************************** */
      case DI_VTEXT:
      {
        if (CurItem->Flags & DIF_BOXCOLOR)
          Attr=CheckDialogMode(DMODE_WARNINGSTYLE) ?
                   ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBOX):
                   ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGBOX);
        else
          if (CurItem->Flags & DIF_SETCOLOR)
            Attr=(CurItem->Flags & DIF_COLORMASK);
          else
            Attr=(CheckDialogMode(DMODE_WARNINGSTYLE) ?
                   ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT):
                   ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGTEXT));

        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,FarColorToReal(Attr));
        SetColor(Attr&0xFF);
        GotoXY(X1+CurItem->X1,Y1+CurItem->Y1);
        VText(CurItem->Data);
        break;
      }

/* ***************************************************************** */
      /* $ 18.07.2000 SVS
         + обработка элемента DI_COMBOBOX
      */
      case DI_EDIT:
      case DI_FIXEDIT:
      case DI_PSWEDIT:
      case DI_COMBOBOX:
      {
        Edit *EditPtr=(Edit *)(CurItem->ObjPtr);
        if(!EditPtr)
          break;

        /* $ 15.08.2000 SVS
           ! Для DropDownList цвета обрабатываем по иному
        */
        /* $ 30.08.2000 SVS
           ! "Цвета, видите ли, ему не понравились" :-)
        */
        Attr=EditPtr->GetObjectColor();
        if(CurItem->Type == DI_COMBOBOX && (CurItem->Flags & DIF_DROPDOWNLIST))
        {
          DWORD AAA=Attr&0xFF;
          Attr=MAKEWORD(FarColorToReal(AAA),
                 FarColorToReal((!CurItem->Focus)?
                  (CheckDialogMode(DMODE_WARNINGSTYLE) ?
                    ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGEDITDISABLED:COL_WARNDIALOGEDIT):
                    ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGEDITDISABLED:COL_DIALOGEDIT)
                  ):
                  (CheckDialogMode(DMODE_WARNINGSTYLE) ?
                    ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGEDITDISABLED:COL_DIALOGEDITSELECTED):
                    ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGEDITDISABLED:COL_DIALOGEDITSELECTED)
                  )
                 )
                );
          Attr=MAKELONG(Attr, // EditLine (Lo=Color, Hi=Selected)
            MAKEWORD(FarColorToReal(AAA), // EditLine - UnChanched Color
            FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ?
               ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT):
               ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGTEXT)
              )) // HistoryLetter
           );
        }
        else
        {
          Attr=MAKEWORD(FarColorToReal(Attr&0xFF),
            (CheckDialogMode(DMODE_WARNINGSTYLE) ?
              FarColorToReal(((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGEDITDISABLED:COL_WARNDIALOGEDIT)):
              FarColorToReal(((CurItem->Flags&DIF_DISABLE)?COL_DIALOGEDITDISABLED:COL_DIALOGEDITSELECTED))
            )
          );
          Attr=MAKELONG(Attr, // EditLine (Lo=Color, Hi=Selected)
             MAKEWORD(FarColorToReal(EditPtr->GetObjectColorUnChanged()), // EditLine - UnChanched Color
             FarColorToReal(((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGTEXT)) // HistoryLetter
             ));
        }
        /* SVS $ */
        /* SVS $ */
        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,Attr);

        EditPtr->SetObjectColor(Attr&0xFF,HIBYTE(LOWORD(Attr)),LOBYTE(HIWORD(Attr)));

        if (CurItem->Focus)
        {
          /* $ 09.08.2000 KM
             Отключение мигающего курсора при перемещении диалога
          */
          if (!CheckDialogMode(DMODE_DRAGGED))
            SetCursorType(1,-1);
          SelectOnEntry(I);
          EditPtr->Show();
          /* KM $ */
        }
        else
        {
          EditPtr->FastShow();
          EditPtr->SetLeftPos(0);
        }

        /* $ 09.08.2000 KM
           Отключение мигающего курсора при перемещении диалога
        */
        if (CheckDialogMode(DMODE_DRAGGED))
          SetCursorType(FALSE,0);
        /* KM $ */

        if (CurItem->History &&
             ((CurItem->Flags & DIF_HISTORY) &&
              Opt.DialogsEditHistory
              || CurItem->Type == DI_COMBOBOX))
        {
          int EditX1,EditY1,EditX2,EditY2;

          EditPtr->GetPosition(EditX1,EditY1,EditX2,EditY2);
          //Text((CurItem->Type == DI_COMBOBOX?"\x1F":"\x19"));
          Text(EditX2+1,EditY1,HIBYTE(HIWORD(Attr)),"\x19");
        }
        break;
        /* SVS $ */
      }

/* ***************************************************************** */
      /* $ 01.08.2000 SVS
         Обычный ListBox
      */
      case DI_LISTBOX:
      {
        VMenu *ListBox=(VMenu *)(CurItem->ObjPtr);
        if(ListBox)
        {
          /* $ 21.08.2000 SVS
             Перед отрисовкой спросим об изменении цветовых атрибутов
          */
          short Colors[9];
          ListBox->GetColors(Colors);
          if(DlgProc((HANDLE)this,DN_CTLCOLORDLGLIST,
                          sizeof(Colors)/sizeof(Colors[0]),(long)Colors))
            ListBox->SetColors(Colors);
          /* SVS $ */
          if (CurItem->Focus)
            ListBox->Show();
          else
            ListBox->FastShow();
        }
        break;
      }
      /* 01.08.2000 SVS $ */

/* ***************************************************************** */
      case DI_CHECKBOX:
      case DI_RADIOBUTTON:
      {
        if (CurItem->Flags & DIF_SETCOLOR)
          Attr=(CurItem->Flags & DIF_COLORMASK);
        else
          Attr=(CheckDialogMode(DMODE_WARNINGSTYLE) ?
                  ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGTEXT):
                  ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGTEXT));

        Attr=MAKEWORD(FarColorToReal(Attr),
             FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ?
                   ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTTEXT):
                   ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTTEXT))); // HiText
        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,Attr);

        SetColor(Attr&0xFF);

        GotoXY(X1+CurItem->X1,Y1+CurItem->Y1);

        if (CurItem->Type==DI_CHECKBOX)
          mprintf("[%c] ",CurItem->Selected ?
             (((CurItem->Flags&DIF_3STATE) && CurItem->Selected == 2)?
                *CheckBox3State:'x'):' ');
        else
          if (CurItem->Flags & DIF_MOVESELECT)
            mprintf(" %c ",CurItem->Selected ? '\07':' ');
          else
            mprintf("(%c) ",CurItem->Selected ? '\07':' ');

        if (CurItem->Flags & DIF_SHOWAMPERSAND)
          Text(CurItem->Data);
        else
          HiText(CurItem->Data,HIBYTE(LOWORD(Attr)));

        if (CurItem->Focus)
        {
          /* $ 09.08.2000 KM
             Отключение мигающего курсора при перемещении диалога
          */
          if (!CheckDialogMode(DMODE_DRAGGED))
            SetCursorType(1,-1);
          MoveCursor(X1+CurItem->X1+1,Y1+CurItem->Y1);
          /* KM $ */
        }

        break;
      }

/* ***************************************************************** */
      case DI_BUTTON:
      {
        GotoXY(X1+CurItem->X1,Y1+CurItem->Y1);

        /* $ 18.08.2000 SVS
           + DI_BUTTON тоже теперь может иметь DIF_SETCOLOR
        */
        if (CurItem->Focus)
        {
          SetCursorType(0,10);
          Attr=MAKEWORD(
             (CurItem->Flags & DIF_SETCOLOR)?(CurItem->Flags & DIF_COLORMASK):
               FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ?
                   ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGSELECTEDBUTTON):
                   ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGSELECTEDBUTTON)), // TEXT
             FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ?
                   ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON):
                   ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTSELECTEDBUTTON))); // HiText
        }
        else
        {
          Attr=MAKEWORD(
             (CurItem->Flags & DIF_SETCOLOR)?(CurItem->Flags & DIF_COLORMASK):
               FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ?
                      ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGBUTTON):
                      ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGBUTTON)), // TEXT
             FarColorToReal(CheckDialogMode(DMODE_WARNINGSTYLE) ?
                      ((CurItem->Flags&DIF_DISABLE)?COL_WARNDIALOGDISABLED:COL_WARNDIALOGHIGHLIGHTBUTTON):
                      ((CurItem->Flags&DIF_DISABLE)?COL_DIALOGDISABLED:COL_DIALOGHIGHLIGHTBUTTON))); // HiText
        }
        /* SVS $ */
        Attr=DlgProc((HANDLE)this,DN_CTLCOLORDLGITEM,I,Attr);
        SetColor(Attr&0xFF);
        if (CurItem->Flags & DIF_SHOWAMPERSAND)
          Text(CurItem->Data);
        else
          HiText(CurItem->Data,HIBYTE(LOWORD(Attr)));
        break;
      }

/* ***************************************************************** */
    } // end switch(...
    /* 28.07.2000 SVS $ */
  } // end for (I=...

  /* $ 31.07.2000 SVS
     Включим индикатор перемещения...
  */
  if ( CheckDialogMode(DMODE_DRAGGED) ) // если диалог таскается
  {
    DlgProc((HANDLE)this,DN_DRAWDIALOG,1,0);
  }
  /* SVS $ */

  SkipDialogMode(DMODE_DRAWING);  // конец отрисовки диалога!!!
  SetDialogMode(DMODE_SHOW); // диалог на экране!
}
/* SVS 22.08.2000 $ */


//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Обработка данных от клавиатуры.
   Перекрывает BaseInput::ProcessKey.
*/
int Dialog::ProcessKey(int Key)
{
  int I,J;
  char Str[1024];
  char *PtrStr;
  Edit *CurEditLine;

  if (Key==KEY_NONE || Key==KEY_IDLE)
  {
    /* $ 28.07.2000 SVS
       Передадим этот факт в обработчик :-)
    */
    DlgProc((HANDLE)this,DN_ENTERIDLE,0,0);
    /* SVS $ */
    return(FALSE);
  }

  /* $ 31.07.2000 tran
     + перемещение диалога по экрану */
  if (CheckDialogMode(DMODE_DRAGGED)) // если диалог таскается
  {
    int rr=1;
    /* $ 15.12.2000 SVS
       При перемещении диалога повторяем поведение "бормандовых" сред.
    */
    switch (Key)
    {
        case KEY_CTRLLEFT:
        case KEY_CTRLHOME:
        case KEY_HOME:
            rr=Key == KEY_CTRLLEFT?10:X1;
        case KEY_LEFT:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( X1>0 )
                {
                    X1--;
                    X2--;
                    AdjustEditPos(-1,0);
                }
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
            break;
        case KEY_CTRLRIGHT:
        case KEY_CTRLEND:
        case KEY_END:
            rr=Key == KEY_CTRLRIGHT?10:abs(X1-(ScrX - (X2-X1+1)))+1;
        case KEY_RIGHT:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( X2<ScrX )
                {
                    X1++;
                    X2++;
                    AdjustEditPos(1,0);
                }
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
            break;
        case KEY_PGUP:
        case KEY_CTRLPGUP:
        case KEY_CTRLUP:
            rr=Key == KEY_CTRLUP?5:Y1;
        case KEY_UP:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( Y1>0 )
                {
                    Y1--;
                    Y2--;
                    AdjustEditPos(0,-1);
                }
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
            break;
        case KEY_CTRLDOWN:
        case KEY_CTRLPGDN:
        case KEY_PGDN:
            rr=Key == KEY_CTRLDOWN? 5: abs(Y1-(ScrY - (Y2-Y1+1)))+1;
        case KEY_DOWN:
            Hide();
            for ( I=0; I<rr; I++ )
                if ( Y2<ScrY )
                {
                    Y1++;
                    Y2++;
                    AdjustEditPos(0,1);
                }
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
            break;
        case KEY_ENTER:
        case KEY_CTRLF5:
            SkipDialogMode(DMODE_DRAGGED); // закончим движение!
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
            break;
        case KEY_ESC:
            Hide();
            AdjustEditPos(OldX1-X1,OldY1-Y1);
            X1=OldX1;
            X2=OldX2;
            Y1=OldY1;
            Y2=OldY2;
            SkipDialogMode(DMODE_DRAGGED);
            if(!CheckDialogMode(DMODE_ALTDRAGGED)) Show();
            break;
    }
    /* SVS $ */
    if(CheckDialogMode(DMODE_ALTDRAGGED))
    {
      SkipDialogMode(DMODE_DRAGGED|DMODE_ALTDRAGGED);
      Show();
    }
    return (TRUE);
  }
  /* $ 10.08.2000 SVS
     Двигаем, если разрешено! (IsCanMove)
  */
  if (Key == KEY_CTRLF5 && CheckDialogMode(DMODE_ISCANMOVE))
  /* SVS 10.08.2000 $*/
  {
    // включаем флаг и запоминаем координаты
    SetDialogMode(DMODE_DRAGGED);
    OldX1=X1; OldX2=X2; OldY1=Y1; OldY2=Y2;
    //# GetText(0,0,3,0,LV);
    Show();
    return (TRUE);
  }
  /* tran 31.07.2000 $ */

  // "ХАчу глянуть на то, что под диалогом..."
  if(Key == KEY_CTRLALTSHIFTPRESS && CheckDialogMode(DMODE_SHOW))
  {
    if(Opt.AllCtrlAltShiftRule & CASR_DIALOG)
    {
      Hide();
      WaitKey(KEY_CTRLALTSHIFTRELEASE);
      Show();
    }
    return(TRUE);
  }

  int Type=Item[FocusPos].Type;

  if(!CheckDialogMode(DMODE_KEY))
    if(DlgProc((HANDLE)this,DM_KEY,FocusPos,Key))
      return TRUE;

  // небольшая оптимизация
  if(Type==DI_CHECKBOX)
  {
    if(!(Item[FocusPos].Flags&DIF_3STATE))
    {
      if((Key == KEY_ADD      && !Item[FocusPos].Selected) ||
         (Key == KEY_SUBTRACT &&  Item[FocusPos].Selected))
       Key=KEY_SPACE;
    }
    /*
      блок else не нужен, т.к. ниже клавиши будут обработаны...
    */
  }
  else if(Key == KEY_ADD)
    Key='+';
  else if(Key == KEY_SUBTRACT)
    Key='-';
  else if(Key == KEY_MULTIPLY)
    Key='*';

  if(Type == DI_LISTBOX)
  {
    switch(Key)
    {
      case KEY_HOME:
      case KEY_LEFT:
      case KEY_RIGHT:
      case KEY_UP:
      case KEY_DOWN:
      case KEY_PGUP:
      case KEY_PGDN:
        VMenu *List=(VMenu *)Item[FocusPos].ObjPtr;
        int CurListPos=List->GetSelectPos();
        int CheckedListItem=List->GetSelection(-1);
        List->ProcessKey(Key);
        int NewListPos=List->GetSelectPos();
        if(NewListPos != CurListPos)
          if(!DlgProc((HANDLE)this,DN_LISTCHANGE,FocusPos,NewListPos))
          {
            List->SetSelection(CheckedListItem,CurListPos);
            ShowDialog(FocusPos);
          }
        return(TRUE);
    }
  }

  switch(Key)
  {
    case KEY_F1:
      /* $ 28.07.2000 SVS
         Перед выводом диалога посылаем сообщение в обработчик
         и если вернули что надо, то выводим подсказку
      */
      PtrStr=(char*)DlgProc((HANDLE)this,DN_HELP,FocusPos,(long)&HelpTopic[0]);
      if(PtrStr && *PtrStr)
      {
        /* $ 31.08.2000 SVS
           - Бага с вызовом файлов помощи.
        */
        if(PluginNumber != -1)
        {
          /* $ 29.08.2000 SVS
             ! При подмене темы помощи из диаловой процедуры...
               короче, нужно вновь формировать контент!
          */
          if (*PtrStr==':')       // Main Topic?
            strcpy(Str,PtrStr+1);
          else if (*PtrStr=='#')  // уже сформировано?
            strcpy(Str,PtrStr);
          else                    // надо формировать...
          {
            strcpy(&Str[512],CtrlObject->Plugins.PluginsData[PluginNumber].ModuleName);
            *PointToName(&Str[512])=0;
            sprintf(Str,"#%s#%s",&Str[512],PtrStr);
          }
          /* SVS $ */
        }
        else
          strcpy(Str,PtrStr);

        SetHelp(Str);
        /* SVS $ */
        ShowHelp();
      }
      /* SVS $ */
      return(TRUE);

    case KEY_TAB:
    case KEY_SHIFTTAB:
// Здесь с фокусом ОООЧЕНЬ ТУМАННО!!!
      if (Item[FocusPos].Flags & DIF_EDITOR)
      {
        I=FocusPos;
        while (Item[I].Flags & DIF_EDITOR)
          I=ChangeFocus(I,(Key==KEY_TAB) ? 1:-1,TRUE);
      }
      else
      {
        I=ChangeFocus(FocusPos,(Key==KEY_TAB) ? 1:-1,TRUE);
        if (Key==KEY_SHIFTTAB)
          while (I>0 && (Item[I].Flags & DIF_EDITOR)!=0 &&
                 (Item[I-1].Flags & DIF_EDITOR)!=0 &&
                 ((Edit *)Item[I].ObjPtr)->GetLength()==0)
            I--;
      }
      ChangeFocus2(FocusPos,I);
      ShowDialog();
      return(TRUE);

    case KEY_CTRLENTER:
      EndLoop=TRUE;
      for (I=0;I<ItemCount;I++)
        if (Item[I].DefaultButton)
        {
          if (!IsEdit(Item[I].Type))
            Item[I].Selected=1;
          ExitCode=I;
          return(TRUE);
        }
      if(!CheckDialogMode(DMODE_OLDSTYLE))
      {
        EndLoop=FALSE; // только если есть
        return TRUE; // делать больше не чего
      }

    case KEY_ENTER:
      if (Item[FocusPos].Flags & DIF_EDITOR)
      {
        int EditorLastPos;
        for (EditorLastPos=I=FocusPos;I<ItemCount;I++)
          if (IsEdit(Item[I].Type) && (Item[I].Flags & DIF_EDITOR))
            EditorLastPos=I;
          else
            break;
        if (((Edit *)(Item[EditorLastPos].ObjPtr))->GetLength()!=0)
          return(TRUE);
        for (I=EditorLastPos;I>FocusPos;I--)
        {
          int CurPos;
          if (I==FocusPos+1)
            CurPos=((Edit *)(Item[I-1].ObjPtr))->GetCurPos();
          else
            CurPos=0;
          ((Edit *)(Item[I-1].ObjPtr))->GetString(Str,sizeof(Str));
          int Length=strlen(Str);
          ((Edit *)(Item[I].ObjPtr))->SetString(CurPos>=Length ? "":Str+CurPos);
          if (CurPos<Length)
            Str[CurPos]=0;
          ((Edit *)(Item[I].ObjPtr))->SetCurPos(0);
          ((Edit *)(Item[I-1].ObjPtr))->SetString(Str);
          /* $ 28.07.2000 SVS
            При изменении состояния каждого элемента посылаем сообщение
            посредством функции SendDlgMessage - в ней делается все!
          */
          Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,I-1,0);
          Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,I,0);
          /* SVS $ */
        }
        if (EditorLastPos>FocusPos)
        {
          ((Edit *)(Item[FocusPos].ObjPtr))->SetCurPos(0);
          ProcessKey(KEY_DOWN);
        }
        else
          ShowDialog();
        return(TRUE);
      }
      else if (Type==DI_BUTTON)
      {
        /* $ 21.08.2000 SVS
           Нет срабатывания, если давим на кнопку
        */
        Item[FocusPos].Selected=1;
        // сообщение - "Кнокна кликнута"
        Dialog::SendDlgMessage((HANDLE)this,DN_BTNCLICK,FocusPos,0);
        /* $ 06.12.2000 SVS
           Если не старый стиль и кнопка "не для закрытия" (DIF_BTNNOCLOSE), то
           вываливаемся, иначе - предлагаем закрыть диалог.
        */
        if(!CheckDialogMode(DMODE_OLDSTYLE) && (Item[FocusPos].Flags&DIF_BTNNOCLOSE))
          return(TRUE);

        ExitCode=FocusPos;
        EndLoop=TRUE;
        /* SVS $ */
        /* SVS $ */
      }
#if 0
      else if(IsEdit(Type) || CheckDialogMode(DMODE_OLDSTYLE))
      {
        for (I=0;I<ItemCount;I++)
          if (Item[I].DefaultButton)
          {
            if (!IsEdit(Item[I].Type))
              Item[I].Selected=1;
            ExitCode=I;
          }

        EndLoop=TRUE;
        if (ExitCode==-1)
          ExitCode=FocusPos;
      }
#else
      else
      {
        ExitCode=-1;
        for (I=0;I<ItemCount;I++)
          if (Item[I].DefaultButton && !(Item[I].Flags&DIF_BTNNOCLOSE))
          {
            if (!IsEdit(Item[I].Type))
              Item[I].Selected=1;
            ExitCode=I;
          }
      }

      EndLoop=TRUE;
      if (ExitCode==-1)
        ExitCode=FocusPos;
#endif
      return(TRUE);

    case KEY_ESC:
    case KEY_BREAK:
    case KEY_F10:
      EndLoop=TRUE;
      ExitCode=(Key==KEY_BREAK) ? -2:-1;
      return(TRUE);

    /* $ 04.12.2000 SVS
       3-х уровневое состояние
       Для чекбокса сюда попадем только в случае, если контрол
       имеет флаг DIF_3STATE
    */
    case KEY_ADD:
    case KEY_SUBTRACT:
    case KEY_MULTIPLY:
      if (Type==DI_CHECKBOX)
      {
        int CHKState=
           (Key == KEY_ADD?1:
            (Key == KEY_SUBTRACT?0:
             ((Key == KEY_MULTIPLY)?2:
              Item[FocusPos].Selected)));
        if(Item[FocusPos].Selected != CHKState)
          if(Dialog::SendDlgMessage((HANDLE)this,DN_BTNCLICK,FocusPos,CHKState))
          {
             Item[FocusPos].Selected=CHKState;
             ShowDialog();
          }
      }
      return(TRUE);
    /* SVS 22.11.2000 $ */

    case KEY_SPACE:
      if (Type==DI_BUTTON)
        return(ProcessKey(KEY_ENTER));
      if (Type==DI_CHECKBOX)
      {
        /* $ 04.12.2000 SVS
           3-х уровневое состояние
        */
        int OldSelected=Item[FocusPos].Selected;

        if(Item[FocusPos].Flags&DIF_3STATE)
          (++Item[FocusPos].Selected)%=3;
        else
          Item[FocusPos].Selected = !Item[FocusPos].Selected;
        /* $ 28.07.2000 SVS
          При изменении состояния каждого элемента посылаем сообщение
           посредством функции SendDlgMessage - в ней делается все!
        */
        if(!Dialog::SendDlgMessage((HANDLE)this,DN_BTNCLICK,FocusPos,Item[FocusPos].Selected))
          Item[FocusPos].Selected = OldSelected;
        /* SVS $ */
        /* SVS 04.12.2000 $ */
        ShowDialog();
        return(TRUE);
      }
      if (Type==DI_RADIOBUTTON)
      {
        int PrevRB;
        for (I=FocusPos;;I--)
          if (Item[I].Type==DI_RADIOBUTTON && (Item[I].Flags & DIF_GROUP) ||
              I==0 || Item[I-1].Type!=DI_RADIOBUTTON)
            break;
        do
        {
          /* $ 28.07.2000 SVS
            При изменении состояния каждого элемента посылаем сообщение
            посредством функции SendDlgMessage - в ней делается все!
          */
          J=Item[I].Selected;
          Item[I].Selected=0;
          if(J)
          {
            PrevRB=I;
          }
          ++I;
          /* SVS $ */
        } while (I<ItemCount && Item[I].Type==DI_RADIOBUTTON &&
                 (Item[I].Flags & DIF_GROUP)==0);

        Item[FocusPos].Selected=1;
        /* $ 28.07.2000 SVS
          При изменении состояния каждого элемента посылаем сообщение
          посредством функции SendDlgMessage - в ней делается все!
        */
        if(!Dialog::SendDlgMessage((HANDLE)this,DN_BTNCLICK,FocusPos,PrevRB))
        {
           // вернем назад, если пользователь не захотел...
           Item[PrevRB].Selected=1;
           Item[FocusPos].Selected=0;
        }
        /* SVS $ */
        ShowDialog();
        return(TRUE);
      }
      if (IsEdit(Type))
      {
        /* $ 28.07.2000 SVS
          При изменении состояния каждого элемента посылаем сообщение
          посредством функции SendDlgMessage - в ней делается все!
        */
        if(((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key))
          Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
        /* SVS $ */
        return(TRUE);
      }
      return(TRUE);

    case KEY_HOME:
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

      if (IsEdit(Type))
      {
        ((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }

      for (I=0;I<ItemCount;I++)
        if (IsFocused(Item[I].Type))
        {
          ChangeFocus2(FocusPos,I);
          /* $ 28.07.2000 SVS
            При изменении состояния каждого элемента посылаем сообщение
            посредством функции SendDlgMessage - в ней делается все!
          */
          //Dialog::SendDlgMessage((HANDLE)this,DN_CHANGEITEM,FocusPos,0);
          //Dialog::SendDlgMessage((HANDLE)this,DN_CHANGEITEM,I,0);
          /* SVS $ */
          ShowDialog();
          return(TRUE);
        }
      return(TRUE);

    case KEY_LEFT:
    case KEY_RIGHT:
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

      if (IsEdit(Type))
      {
        ((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      {
        int MinDist=1000,MinPos;
        for (I=0;I<ItemCount;I++)
          if (I!=FocusPos && (IsEdit(Item[I].Type) || Item[I].Type==DI_CHECKBOX ||
              Item[I].Type==DI_RADIOBUTTON) && Item[I].Y1==Item[FocusPos].Y1)
          {
            int Dist=Item[I].X1-Item[FocusPos].X1;
            if (Key==KEY_LEFT && Dist<0 || Key==KEY_RIGHT && Dist>0)
              if (abs(Dist)<MinDist)
              {
                MinDist=abs(Dist);
                MinPos=I;
              }
          }
          if (MinDist<1000)
          {
            ChangeFocus2(FocusPos,MinPos);
            if (Item[MinPos].Flags & DIF_MOVESELECT)
              ProcessKey(KEY_SPACE);
            else
              ShowDialog();
            return(TRUE);
          }
      }

    case KEY_UP:
    case KEY_DOWN:
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

      {
        int PrevPos=0;
        if (Item[FocusPos].Flags & DIF_EDITOR)
          PrevPos=((Edit *)(Item[FocusPos].ObjPtr))->GetCurPos();
        I=ChangeFocus(FocusPos,(Key==KEY_LEFT || Key==KEY_UP) ? -1:1,FALSE);
        Item[FocusPos].Focus=0;
        Item[I].Focus=1;
        ChangeFocus2(FocusPos,I);
        if (Item[I].Flags & DIF_EDITOR)
          ((Edit *)(Item[I].ObjPtr))->SetCurPos(PrevPos);
        if (Item[I].Flags & DIF_MOVESELECT)
          ProcessKey(KEY_SPACE);
        else
          ShowDialog();
      }
      return(TRUE);

    case KEY_END:
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

      if (IsEdit(Type))
      {
        ((Edit *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
    case KEY_PGDN:
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

      if (!(Item[FocusPos].Flags & DIF_EDITOR))
      {
        for (I=0;I<ItemCount;I++)
          if (Item[I].DefaultButton)
          {
            ChangeFocus2(FocusPos,I);
            ShowDialog();
            return(TRUE);
          }
      }
      else
      {
        ProcessKey(KEY_TAB);
        ProcessKey(KEY_UP);
      }
      return(TRUE);

    /* $ 27.04.2001 VVM
      + Обработка колеса мышки */
    case KEY_MSWHEEL_UP:
    case KEY_MSWHEEL_DOWN:
    /* VVM $ */
    case KEY_CTRLUP:
    case KEY_CTRLDOWN:
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

      CurEditLine=((Edit *)(Item[FocusPos].ObjPtr));
      if (IsEdit(Type) &&
           (Item[FocusPos].Flags & DIF_HISTORY) &&
           Opt.DialogsEditHistory &&
           Item[FocusPos].History)
      /* $ 26.07.2000 SVS
         Передаем то, что в строке ввода в функцию выбора из истории
         для выделения нужного пункта в истории.
      */
      {
        char *PStr=Str;
        int MaxLen=sizeof(Item[FocusPos].Data);
        if(Item[FocusPos].Flags&DIF_VAREDIT)
        {
          MaxLen=Item[FocusPos].Ptr.PtrLength;
          if((PStr=(char*)malloc(MaxLen+1)) == NULL)
            return TRUE;//???
        }
        /* $ 27.04.2001 SVS
           Оху%$@#&^%$&$%*%^$*^%$*^%$*^%$&*
           Было: sizeof(MaxLen) ;-( - это типа размер данных.
        */
        CurEditLine->GetString(PStr,MaxLen);
        /* SVS $ */
        SelectFromEditHistory(CurEditLine,Item[FocusPos].History,PStr,MaxLen);
        Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
        if(Item[FocusPos].Flags&DIF_VAREDIT)
          free(PStr);
      }
      /* SVS $ */
      /* $ 18.07.2000 SVS
         + обработка DI_COMBOBOX - выбор из списка!
      */
      else if(Type == DI_COMBOBOX && Item[FocusPos].ListItems)
      {
        char *PStr=Str;
        int MaxLen=sizeof(Item[FocusPos].Data);
        if(Item[FocusPos].Flags&DIF_VAREDIT)
        {
          MaxLen=Item[FocusPos].Ptr.PtrLength;
          if((PStr=(char*)malloc(MaxLen+1)) == NULL)
            return TRUE;//???
        }
        CurEditLine->GetString(PStr,MaxLen);
        SelectFromComboBox(CurEditLine,
                      Item[FocusPos].ListItems,PStr,MaxLen);
        Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
        if(Item[FocusPos].Flags&DIF_VAREDIT)
          free(PStr);
      }
      /* SVS $ */
      return(TRUE);

    default:
      // для user-типа вываливаем
      if(Type == DI_USERCONTROL)
        return TRUE;

      /* $ 01.08.2000 SVS
         Обычный ListBox
      */
      if(Type == DI_LISTBOX)
      {
        ((VMenu *)(Item[FocusPos].ObjPtr))->ProcessKey(Key);
        return(TRUE);
      }
      /* SVS $ */

      /* $ 21.08.2000 SVS
         Autocomplete при постоянных блоках и немного оптимизации ;-)
      */
      if (IsEdit(Type))
      {
        Edit *edt=(Edit *)Item[FocusPos].ObjPtr;
        int SelStart, SelEnd;

        if(Key == KEY_CTRLL) // исключим смену режима RO для поля ввода с клавиатуры
          return TRUE;

        /* $ 11.09.2000 SVS
           Ctrl-U в строках ввода снимает пометку блока
        */
        if(Key == KEY_CTRLU)
        {
          edt->SetClearFlag(0);
          edt->Select(-1,0);
          edt->Show();
          return TRUE;
        }
        /* SVS $ */

        if (Item[FocusPos].Flags & DIF_EDITOR)
          switch(Key)
          {
            /* $ 12.09.2000 SVS
              Исправляем ситуацию с BackSpace в DIF_EDITOR
            */
            case KEY_BS:
            {
              int CurPos=edt->GetCurPos();
              /* $ 21.11.2000 SVS
                 Не стиралась последняя строка в многострочном редакторе
              */
              // В начале строки????
              if(!edt->GetCurPos())
              {
                // а "выше" тоже DIF_EDITOR?
                if(FocusPos > 0 && (Item[FocusPos-1].Flags&DIF_EDITOR))
                {
                  // добавляем к предыдущему и...
                  Edit *edt_1=(Edit *)Item[FocusPos-1].ObjPtr;
                  edt_1->GetString(Str,sizeof(Str));
                  CurPos=strlen(Str);
                  edt->GetString(Str+CurPos,sizeof(Str)-CurPos);
                  edt_1->SetString(Str);

                  for (I=FocusPos+1;I<ItemCount;I++)
                  {
                    if (Item[I].Flags & DIF_EDITOR)
                    {
                      if (I>FocusPos)
                      {
                        ((Edit *)(Item[I].ObjPtr))->GetString(Str,sizeof(Str));
                        ((Edit *)(Item[I-1].ObjPtr))->SetString(Str);
                      }
                      ((Edit *)(Item[I].ObjPtr))->SetString("");
                    }
                    else // ага, значит  FocusPos это есть последний из DIF_EDITOR
                    {
                      ((Edit *)(Item[I-1].ObjPtr))->SetString("");
                      break;
                    }
                  }
                  ProcessKey(KEY_UP);
                  edt_1->SetCurPos(CurPos);
                }
              }
              /* SVS $ */
              else
              {
                edt->ProcessKey(Key);
              }
              Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
              ShowDialog();
              return(TRUE);
            }
            /* SVS $ */

            case KEY_CTRLY:
              for (I=FocusPos;I<ItemCount;I++)
                if (Item[I].Flags & DIF_EDITOR)
                {
                  if (I>FocusPos)
                  {
                    ((Edit *)(Item[I].ObjPtr))->GetString(Str,sizeof(Str));
                    ((Edit *)(Item[I-1].ObjPtr))->SetString(Str);
                  }
                  ((Edit *)(Item[I].ObjPtr))->SetString("");
                }
                else
                  break;
              /* $ 28.07.2000 SVS
                При изменении состояния каждого элемента посылаем сообщение
                посредством функции SendDlgMessage - в ней делается все!
              */
              Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
              /* SVS $ */
              ShowDialog();
              return(TRUE);

            case KEY_DEL:
              /* $ 19.07.2000 SVS
                 ! "...В редакторе команд меню нажмите home shift+end del
                   блок не удаляется..."
                   DEL у итемов, имеющих DIF_EDITOR, работал без учета
                   выделения...
              */
              if (FocusPos<ItemCount+1 && (Item[FocusPos+1].Flags & DIF_EDITOR))
              {
                int CurPos=edt->GetCurPos();
                int Length=edt->GetLength();
                int SelStart, SelEnd;

                edt->GetSelection(SelStart, SelEnd);
                edt->GetString(Str,sizeof(Str));
                int LengthStr=strlen(Str);
                if(SelStart > -1)
                {
                  memmove(&Str[SelStart],&Str[SelEnd],Length-SelEnd+1);
                  edt->SetString(Str);
                  edt->SetCurPos(SelStart);
                  /* $ 28.07.2000 SVS
                    При изменении состояния каждого элемента посылаем сообщение
                    посредством функции SendDlgMessage - в ней делается все!
                  */
                  Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
                  /* SVS $ */
                  ShowDialog();
                  return(TRUE);
                }
                else if (CurPos>=Length)
                {
                  Edit *edt_1=(Edit *)Item[FocusPos+1].ObjPtr;
                  /* $ 12.09.2000 SVS
                     Решаем проблему, если Del нажали в позиции
                     большей, чем длина строки
                  */
                  if (CurPos > Length)
                  {
                    LengthStr=CurPos;
                    memset(Str+Length,' ',CurPos-Length);
                  }
                  /* SVS $*/
                  edt_1->GetString(Str+LengthStr,sizeof(Str)-LengthStr);
                  edt_1->SetString(Str);
                  ProcessKey(KEY_CTRLY);
                  edt->SetCurPos(CurPos);
                  ShowDialog();
                  return(TRUE);
                }
              }
              break;
              /* SVS $*/
            case KEY_PGUP:
              ProcessKey(KEY_SHIFTTAB);
              ProcessKey(KEY_DOWN);
              return(TRUE);
          }

        /* $ 24.09.2000 SVS
           Вызов функции Xlat
        */
        /* $ 04.11.2000 SVS
           Проверка на альтернативную клавишу
        */
        if(Opt.XLat.XLatDialogKey && Key == Opt.XLat.XLatDialogKey ||
           Opt.XLat.XLatAltDialogKey && Key == Opt.XLat.XLatAltDialogKey)
        {
          edt->Xlat();
          Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
          return TRUE;
        }
        /* SVS $ */
        /* SVS $ */

        if (edt->ProcessKey(Key))
        {
          int RedrawNeed=FALSE;
          /* $ 26.07.2000 SVS
             AutoComplite: Если установлен DIF_HISTORY
                 и разрешено автозавершение!.
          */
          /* $ 04.12.2000 SVS
            Автодополнение - чтобы не работало во время проигрывания макросов.
          */
          if(!CtrlObject->Macro.IsExecuting() &&
             Opt.AutoComplete && Key < 256 && Key != KEY_BS && Key != KEY_DEL &&
             ((Item[FocusPos].Flags & DIF_HISTORY) || Type == DI_COMBOBOX)
            )
          {
            /* $ 05.12.2000 IS
               Все удалил и написал заново ;)
            */
            int MaxLen=sizeof(Item[FocusPos].Data);
            char *PStr=Str;
            if(Item[FocusPos].Flags & DIF_VAREDIT)
            {
              MaxLen=Item[FocusPos].Ptr.PtrLength;
              if((PStr=(char*)malloc(MaxLen+1)) == NULL)
                return TRUE; //???
            }
            int DoAutoComplete=TRUE;
            int CurPos=edt->GetCurPos();
            edt->GetString(PStr,MaxLen);
            int len=strlen(PStr);
            edt->GetSelection(SelStart,SelEnd);
            if(SelStart < 0 || SelStart==SelEnd)
                SelStart=len;
            else
                SelStart++;

            if(CurPos<SelStart) DoAutoComplete=FALSE;
            if(SelStart<SelEnd && SelEnd<len) DoAutoComplete=FALSE;

            if(Opt.EdOpt.PersistentBlocks)
            {
              if(DoAutoComplete && CurPos <= SelEnd)
              {
                PStr[CurPos]=0;
                edt->Select(CurPos,MaxLen); //select the appropriate text
                edt->DeleteBlock();
                edt->FastShow();
              }
            }
            /* IS $ */

            SelEnd=strlen(PStr);

            //find the string in the list
            /* $ 03.12.2000 IS
                 Учитываем флаг DoAutoComplete
            */
            if (DoAutoComplete && FindInEditForAC(Type == DI_COMBOBOX,
                         (void *)Item[FocusPos].Selected,PStr,MaxLen))
            /* IS $ */
            {
//SysLog("Coplete: Str=%s SelStart=%d SelEnd=%d CurPos=%d",Str,SelStart,SelEnd, CurPos);
              edt->SetString(PStr);
              edt->Select(SelEnd,MaxLen); //select the appropriate text
              //edt->Select(CurPos,sizeof(Str)); //select the appropriate text
              /* $ 01.08.2000 SVS
                 Небольшой глючек с AutoComplete
              */
              edt->SetCurPos(CurPos); // SelEnd
              RedrawNeed=TRUE;
            }
            if(Item[FocusPos].Flags & DIF_VAREDIT)
              free(PStr);
          }
          /* SVS 03.12.2000 $ */
          Dialog::SendDlgMessage((HANDLE)this,DN_EDITCHANGE,FocusPos,0);
          /* SVS $ */
          if(RedrawNeed)
            Redraw(); // Перерисовка должна идти после DN_EDITCHANGE (imho)
          return(TRUE);
        }
        /* SVS 21.08.2000 $ */
      }

      if (ProcessHighlighting(Key,FocusPos,FALSE))
        return(TRUE);

      return(ProcessHighlighting(Key,FocusPos,TRUE));
  }
}


//////////////////////////////////////////////////////////////////////////
/* Public, Virtual:
   Обработка данных от "мыши".
   Перекрывает BaseInput::ProcessMouse.
*/
/* $ 18.08.2000 SVS
   + DN_MOUSECLICK
*/
int Dialog::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int I, J;
  int MsX,MsY;
  int Type;
  RECT Rect;

  if (MouseEvent->dwButtonState==0)
    return(FALSE);
  if(!CheckDialogMode(DMODE_SHOW))
    return FALSE;

  MsX=MouseEvent->dwMousePosition.X;
  MsY=MouseEvent->dwMousePosition.Y;

  if (MsX<X1 || MsY<Y1 || MsX>X2 || MsY>Y2)
  {
    if(!DlgProc((HANDLE)this,DN_MOUSECLICK,-1,(long)MouseEvent))
    {
      if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
        ProcessKey(KEY_ESC);
      else if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
        ProcessKey(KEY_ENTER);
    }
    return(TRUE);
  }

//SysLog("Ms (%d,%d)",MsX,MsY);
  if (MouseEvent->dwEventFlags==0 || MouseEvent->dwEventFlags==DOUBLE_CLICK)
  {
    // первый цикл - все за исключением рамок.
    for (I=0; I < ItemCount;I++)
    {
      if(Item[I].Flags&(DIF_DISABLE|DIF_HIDDEN))
        continue;

      GetItemRect(I,Rect);
      Rect.left+=X1;  Rect.top+=Y1;
      Rect.right+=X1; Rect.bottom+=Y1;
//SysLog("? %2d) Rect (%2d,%2d) (%2d,%2d) '%s'",I,Rect.left,Rect.top,Rect.right,Rect.bottom,Item[I].Data);

      if(MsX >= Rect.left && MsY >= Rect.top && MsX <= Rect.right && MsY <= Rect.bottom)
      {
//SysLog("+ %2d) Rect (%2d,%2d) (%2d,%2d) '%s'",I,Rect.left,Rect.top,Rect.right,Rect.bottom,Item[I].Data);
        // для прозрачных :-)
        if(Item[I].Type == DI_SINGLEBOX || Item[I].Type == DI_DOUBLEBOX)
        {
          // если на рамке, то...
          if(((MsX == Rect.left || MsX == Rect.right) && MsY >= Rect.top && MsY <= Rect.bottom) || // vert
             ((MsY == Rect.top  || MsY == Rect.bottom) && MsX >= Rect.left && MsX <= Rect.right) )   // hor
          {
            if(DlgProc((HANDLE)this,DN_MOUSECLICK,I,(long)MouseEvent))
              return TRUE;
          }
          else
            continue;
        }

        if(Item[I].Type == DI_USERCONTROL)
        {
          // для user-типа подготовим координаты мыши
          MouseEvent->dwMousePosition.X-=Rect.left;
          MouseEvent->dwMousePosition.Y-=Rect.top;
        }

        if(DlgProc((HANDLE)this,DN_MOUSECLICK,I,(long)MouseEvent))
          return TRUE;

        if(Item[I].Type == DI_USERCONTROL)
        {
           ChangeFocus2(FocusPos,I);
           ShowDialog();
           return(TRUE);
        }
        break;
      }
    }

    if((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
    {
      for (I=0;I<ItemCount;I++)
      {
        /* $ 04.12.2000 SVS
           Исключаем из списка оповещаемых о мыши недоступные элементы
        */
        if(Item[I].Flags&(DIF_DISABLE|DIF_HIDDEN))
          continue;
        /* SVS $ */
        Type=Item[I].Type;
        if (MsX>=X1+Item[I].X1)
        {
          /* $ 01.08.2000 SVS
             Обычный ListBox
          */
          if(Type == DI_LISTBOX    &&
              MsY >= Y1+Item[I].Y1 &&
              MsY <= Y1+Item[I].Y2 &&
              MsX <= X1+Item[I].X2)
          {
            if(FocusPos != I)
              ChangeFocus2(FocusPos,I);
            ShowDialog();
            ((VMenu *)(Item[I].ObjPtr))->ProcessMouse(MouseEvent);
            return(TRUE);
          }
          /* SVS $ */

          if (IsEdit(Type))
          {
            /* $ 15.08.2000 SVS
               + Сделаем так, чтобы ткнув мышкой в DropDownList
                 список раскрывался сам.
               Есть некоторая глюкавость - когда список раскрыт и мы
               мышой переваливаем на другой элемент, то список закрывается
               но перехода реального на указанный элемент диалога не происходит
            */
            int EditX1,EditY1,EditX2,EditY2;
            Edit *EditLine=(Edit *)(Item[I].ObjPtr);
            EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);

            if(MsY==EditY1 && Type == DI_COMBOBOX &&
               (Item[I].Flags & DIF_DROPDOWNLIST) &&
               MsX >= EditX1 && MsX <= EditX2+1)
            {
              EditLine->SetClearFlag(0);
              ChangeFocus2(FocusPos,I);
              ShowDialog();
              ProcessKey(KEY_CTRLDOWN);
              return(TRUE);
            }
            /* SVS $ */

            if (EditLine->ProcessMouse(MouseEvent))
            {
              EditLine->SetClearFlag(0);
              ChangeFocus2(FocusPos,I);
              ShowDialog();
              return(TRUE);
            }
            else
            {
              /* $ 18.07.2000 SVS
                 + Проверка на тип элемента DI_COMBOBOX
              */
              if (MsX==EditX2+1 && MsY==EditY1 && Item[I].History &&
                  ((Item[I].Flags & DIF_HISTORY) && Opt.DialogsEditHistory
                   || Type == DI_COMBOBOX))
              /* SVS $ */
              {
                ChangeFocus2(FocusPos,I);
                ProcessKey(KEY_CTRLDOWN);
                return(TRUE);
              }
            }
          }
          if (Type==DI_BUTTON &&
              MsY==Y1+Item[I].Y1 &&
              MsX < X1+Item[I].X1+HiStrlen(Item[I].Data))
          {
            ChangeFocus2(FocusPos,I);
            ShowDialog();
            while (IsMouseButtonPressed())
              ;
            if (MouseX <  X1 ||
                MouseX >  X1+Item[I].X1+HiStrlen(Item[I].Data)+4 ||
                MouseY != Y1+Item[I].Y1)
            {
              ChangeFocus2(FocusPos,I);
              ShowDialog();
              return(TRUE);
            }
            ProcessKey(KEY_ENTER);
            return(TRUE);
          }

          if ((Type == DI_CHECKBOX ||
               Type == DI_RADIOBUTTON) &&
              MsY==Y1+Item[I].Y1 &&
              MsX < (X1+Item[I].X1+HiStrlen(Item[I].Data)+4-((Item[I].Flags & DIF_MOVESELECT)!=0)))
          {
            ChangeFocus2(FocusPos,I);
            ProcessKey(KEY_SPACE);
            return(TRUE);
          }
        }
      } // for (I=0;I<ItemCount;I++)
      // ДЛЯ MOUSE-Перемещалки:
      //   Сюда попадаем в том случае, если мышь не попала на активные элементы
      //
      /* $ 10.08.2000 SVS
         Двигаем, если разрешено! (IsCanMove)
      */
      if (CheckDialogMode(DMODE_ISCANMOVE))
      {
        /* $ 03.08.2000 tran
           ну раз попадаем - то будем перемещать */
        //SetDialogMode(DMODE_DRAGGED);
        OldX1=X1; OldX2=X2; OldY1=Y1; OldY2=Y2;
        // запомним delta места хватания и Left-Top диалогового окна
        MsX=abs(X1-MouseX);
        MsY=abs(Y1-MouseY);
        while (1)
        {
            int mb=IsMouseButtonPressed();
            /* $ 15.12.2000 SVS
               Новый движок мышиного перемещения
            */
            int mx,my,X0,Y0;
            if ( mb==1 ) // left key, still dragging
            {
                Hide();
                X0=X1;
                Y0=Y1;
                if(MouseX==PrevMouseX)
                  mx=X1;
                else
                  mx=MouseX-MsX;
                if(MouseY==PrevMouseY)
                  my=Y1;
                else
                  my=MouseY-MsY;

                if(mx >= 0 && mx+(X2-X1)<=ScrX)
                {
                  X2=mx+(X2-X1);
                  X1=mx;
                  AdjustEditPos(X1-X0,0); //?
                }
                if(my >= 0 && my+(Y2-Y1)<=ScrY)
                {
                  Y2=my+(Y2-Y1);
                  Y1=my;
                  AdjustEditPos(0,Y1-Y0); //?
                }
                Show();
            }
            /* SVS $ */
            else if (mb==2) // right key, abort
            {
                Hide();
                AdjustEditPos(OldX1-X1,OldY1-Y1);
                X1=OldX1;
                X2=OldX2;
                Y1=OldY1;
                Y2=OldY2;
                SkipDialogMode(DMODE_DRAGGED);
                Show();
                break;
            }
            else  // release key, drop dialog
            {
                SkipDialogMode(DMODE_DRAGGED);
                Show();
                break;
            }
        }// while (1)
        /* tran 03.08.2000 $ */
      }
      /* SVS 10.08.2000 $*/
    }
  }
  return(FALSE);
}
/* SVS 18.08.2000 $ */


//////////////////////////////////////////////////////////////////////////
/* Private:
   Изменяет фокус ввода (воздействие клавишами
     KEY_TAB, KEY_SHIFTTAB, KEY_UP, KEY_DOWN,
   а так же Alt-HotKey)
*/
/* $ 28.07.2000 SVS
   Довесок для сообщений DN_KILLFOCUS & DN_SETFOCUS
*/
/* $ 24.08.2000 SVS
   Добавка для DI_USERCONTROL
*/
int Dialog::ChangeFocus(int FocusPos,int Step,int SkipGroup)
{
  int Type,OrigFocusPos=FocusPos;
//  int FucusPosNeed=-1;
  // В функцию обработки диалога здесь передаем сообщение,
  //   что элемент - LostFocus() - теряет фокус ввода.
//  if(CheckDialogMode(DMODE_INITOBJECTS))
//    FucusPosNeed=DlgProc((HANDLE)this,DN_KILLFOCUS,FocusPos,0);
//  if(FucusPosNeed != -1 && IsFocused(Item[FucusPosNeed].Type))
//    FocusPos=FucusPosNeed;
//  else
  {
    while (1)
    {
      FocusPos+=Step;
      if (FocusPos>=ItemCount)
        FocusPos=0;
      if (FocusPos<0)
        FocusPos=ItemCount-1;

      Type=Item[FocusPos].Type;

      if(!(Item[FocusPos].Flags&(DIF_NOFOCUS|DIF_DISABLE|DIF_HIDDEN)))
      {
        if (Type==DI_LISTBOX || Type==DI_BUTTON || Type==DI_CHECKBOX || IsEdit(Type) || Type==DI_USERCONTROL)
          break;
        if (Type==DI_RADIOBUTTON && (!SkipGroup || Item[FocusPos].Selected))
          break;
      }
      // убираем зацикливание с последующим подвисанием :-)
      if(OrigFocusPos == FocusPos)
        break;
    }
  }

//  Dialog::FocusPos=FocusPos;
  // В функцию обработки диалога здесь передаем сообщение,
  //   что элемент GotFocus() - получил фокус ввода.
  // Игнорируем возвращаемое функцией диалога значение
//  if(CheckDialogMode(DMODE_INITOBJECTS))
//    DlgProc((HANDLE)this,DN_GOTFOCUS,FocusPos,0);
  return(FocusPos);
}
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   Private:
   Изменяет фокус ввода между двумя элементами.
   Вынесен отдельно с тем, чтобы обработать DN_KILLFOCUS & DM_SETFOCUS
*/
int Dialog::ChangeFocus2(int KillFocusPos,int SetFocusPos)
{
  int FucusPosNeed=-1;
  if(!(Item[SetFocusPos].Flags&(DIF_NOFOCUS|DIF_DISABLE|DIF_HIDDEN)))
  {
    if(CheckDialogMode(DMODE_INITOBJECTS))
      FucusPosNeed=DlgProc((HANDLE)this,DN_KILLFOCUS,KillFocusPos,0);

    if(FucusPosNeed != -1 && IsFocused(Item[FucusPosNeed].Type))
      SetFocusPos=FucusPosNeed;

    if(Item[SetFocusPos].Flags&DIF_NOFOCUS)
       SetFocusPos=KillFocusPos;

    Item[KillFocusPos].Focus=0;
    Item[SetFocusPos].Focus=1;

    Dialog::PrevFocusPos=Dialog::FocusPos;
    Dialog::FocusPos=SetFocusPos;
    if(CheckDialogMode(DMODE_INITOBJECTS))
      DlgProc((HANDLE)this,DN_GOTFOCUS,SetFocusPos,0);
  }
  else
    SetFocusPos=KillFocusPos;

  return(SetFocusPos);
}
/* SVS $ */

/* $ 08.09.2000 SVS
  Функция SelectOnEntry - выделение строки редактирования
  Обработка флага DIF_SELECTONENTRY
*/
void Dialog::SelectOnEntry(int Pos)
{
  if(IsEdit(Item[Pos].Type) &&
     (Item[Pos].Flags&DIF_SELECTONENTRY)
//     && PrevFocusPos != -1 && PrevFocusPos != Pos
    )
  {
    Edit *edt=(Edit *)Item[Pos].ObjPtr;
    if(edt)
      edt->Select(0,edt->GetLength());
  }
}
/* SVS $ */

/* $ 04.12.2000 SVS
   ! Оптимизация функций ConvertItem() и DataToItem() - с указателями
     будет генериться компактный и быстрый код (MSVC - это сам делает :-(
*/

//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   Public, Static:
   + функция ConvertItem - обратное преобразование элементов диалога из
   внутреннего представления во внешние
*/
void Dialog::ConvertItem(int FromPlugin,
                         struct FarDialogItem *Item,struct DialogItem *Data,
                         int Count,BOOL InternalCall)
{
  int I;
  if(!Item || !Data)
    return;

  char *PtrData;
  int PtrLength;
  Edit *EditPtr;

  if(FromPlugin == CVTITEM_TOPLUGIN)
    for (I=0; I < Count; I++, ++Item, ++Data)
    {
      Item->Type=Data->Type;
      Item->X1=Data->X1;
      Item->Y1=Data->Y1;
      Item->X2=Data->X2;
      Item->Y2=Data->Y2;
      Item->Focus=Data->Focus;
      Item->Selected=Data->Selected;
      Item->Flags=Data->Flags;
      Item->DefaultButton=Data->DefaultButton;
      if(InternalCall)
      {
        if(Dialog::IsEdit(Data->Type) && (EditPtr=(Edit *)(Data->ObjPtr)) != NULL)
        {
          // Заполним значения
          if((Data->Type==DI_EDIT || Data->Type==DI_COMBOBOX) &&
             (Data->Flags&DIF_VAREDIT))
          {
            PtrData  =(char *)Data->Ptr.PtrData;
            PtrLength=Data->Ptr.PtrLength;
          }
          else
          {
            PtrData  =Data->Data;
            PtrLength=sizeof(Data->Data);
          }
          EditPtr->GetString(PtrData,PtrLength);
        }
      }
      {
          TRY{
            memmove(Item->Data,Data->Data,sizeof(Item->Data));
          }
          __except (EXCEPTION_EXECUTE_HANDLER)
          {
            ;
          }
      }
    }
  else
    for (I=0; I < Count; I++, ++Item, ++Data)
    {
      Data->Type=Item->Type;
      Data->X1=Item->X1;
      Data->Y1=Item->Y1;
      Data->X2=Item->X2;
      Data->Y2=Item->Y2;
      Data->Focus=Item->Focus;
      Data->Selected=Item->Selected;
      Data->Flags=Item->Flags;
      Data->DefaultButton=Item->DefaultButton;
      {
         TRY{
           memmove(Data->Data,Item->Data,sizeof(Data->Data));
         }
         __except (EXCEPTION_EXECUTE_HANDLER)
         {
           ;
         }
      }
      /* Этот кусок будет работать после тчательной проверки.
      Он позволит менять данные в ответ на DN_EDITCHANGE
      if(InternalCall)
      {
        if(Dialog::IsEdit(Data->Type) && (EditPtr=(Edit *)(Data->ObjPtr)) != NULL)
        {
          // обновим
          if((Data->Type==DI_EDIT || Data->Type==DI_COMBOBOX) &&
             (Data->Flags&DIF_VAREDIT))
          {
            PtrData  =(char *)Data->Ptr.PtrData;
            PtrLength=Data->Ptr.PtrLength;
          }
          else
          {
            PtrData  =Data->Data;
            PtrLength=sizeof(Data->Data);
          }
          EditPtr->SetString(PtrData);
        }
      }
      */
    }
}
/* SVS $ */

//////////////////////////////////////////////////////////////////////////
/* Public, Static:
   преобразует данные об элементах диалога во внутреннее
   представление. Аналогичен функции InitDialogItems (см. "Far PlugRinG
   Russian Help Encyclopedia of Developer")
*/
void Dialog::DataToItem(struct DialogData *Data,struct DialogItem *Item,
                        int Count)
{
  int I;

  if(!Item || !Data)
    return;

  for (I=0;I<Count;I++, ++Item, ++Data)
  {
    Item->Type=Data->Type;
    Item->X1=Data->X1;
    Item->Y1=Data->Y1;
    Item->X2=Data->X2;
    Item->Y2=Data->Y2;
    Item->Focus=Data->Focus;
    Item->Selected=Data->Selected;
    Item->Flags=Data->Flags;
    Item->DefaultButton=Data->DefaultButton;
    if ((unsigned int)Data->Data<MAX_MSG)
      strcpy(Item->Data,MSG((unsigned int)Data->Data));
    else
    {
      TRY{
        memmove(Item->Data,Data->Data,sizeof(Item->Data));
      }
      __except (EXCEPTION_EXECUTE_HANDLER)
      {
        ;
      }
    }
    Item->ObjPtr=NULL;
  }
}
/* SVS 04.12.2000 $ */


//////////////////////////////////////////////////////////////////////////
/* Private:
   Проверяет тип элемента диалога на предмет строки ввода
   (DI_EDIT, DI_FIXEDIT, DI_PSWEDIT) и в случае успеха возвращает TRUE
*/
/* $ 18.07.2000 SVS
   ! элемент DI_COMBOBOX относится к категории строковых редакторов...
*/
int Dialog::IsEdit(int Type)
{
  return(Type==DI_EDIT ||
         Type==DI_FIXEDIT ||
         Type==DI_PSWEDIT ||
         Type == DI_COMBOBOX);
}
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   Функция, определяющая - "Может ли элемент диалога иметь фокус ввода"
*/
/* $ 24.08.2000 SVS
   Добавка для DI_USERCONTROL
*/
int Dialog::IsFocused(int Type)
{
  return(Type==DI_EDIT ||
         Type==DI_FIXEDIT ||
         Type==DI_PSWEDIT ||
         Type==DI_COMBOBOX ||
         Type==DI_BUTTON ||
         Type==DI_CHECKBOX ||
         Type==DI_RADIOBUTTON ||
         Type==DI_LISTBOX ||
         Type==DI_USERCONTROL);
}
/* 24.08.2000 SVS $ */
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
/* $ 26.07.2000 SVS
   AutoComplite: Поиск входжение подстроки в истории
*/
/* $ 28.07.2000 SVS
   ! Переметр Edit *EditLine нафиг ненужен!
*/
int Dialog::FindInEditForAC(int TypeFind,void *HistoryName,char *FindStr,int MaxLen)
{
  char *Str;
  int I, Count, LenFindStr=strlen(FindStr);

  if(!TypeFind)
  {
    char RegKey[80],KeyValue[80];
    if((Str=(char*)malloc(MaxLen+1)) == NULL)
      return FALSE;
    sprintf(RegKey,fmtSavedDialogHistory,(char*)HistoryName);
    // просмотр пунктов истории
    for (I=0; I < HISTORY_COUNT; I++)
    {
      sprintf(KeyValue,fmtLine,I);
      GetRegKey(RegKey,KeyValue,Str,"",MaxLen);
      if (!LocalStrnicmp(Str,FindStr,LenFindStr))
        break;
    }
    if (I == HISTORY_COUNT)
    {
      free(Str);
      return FALSE;
    }
    /* $ 28.07.2000 SVS
       Введенные буковки не затрагиваем, а дополняем недостающее.
    */
//SysLog("FindInEditForAC()  FindStr=%s Str=%s",FindStr,&Str[strlen(FindStr)]);
    strncat(FindStr,&Str[LenFindStr],MaxLen-LenFindStr);
    /* SVS $ */
    free(Str);
  }
  else
  {
    struct FarListItem *ListItems=((struct FarList *)HistoryName)->Items;
    int Count=((struct FarList *)HistoryName)->ItemsNumber;

    for (I=0; I < Count ;I++)
    {
      if (!LocalStrnicmp(
        ((ListItems[I].Flags&LIF_PTRDATA)?
            ListItems[I].Ptr.PtrData:
            ListItems[I].Text),
        FindStr,LenFindStr))
        break;
    }
    if (I  == Count)
      return FALSE;

    if(ListItems[I].Flags&LIF_PTRDATA)
    {
      // проверим "переполнение" - чтобы не вылезти за пределы строки.
      if(ListItems[I].Ptr.PtrLength < LenFindStr)
        strncat(FindStr,&ListItems[I].Ptr.PtrData[LenFindStr],MaxLen-LenFindStr);
    }
    else
    {
      if(sizeof(ListItems[I].Text) < LenFindStr)
        strncat(FindStr,&ListItems[I].Text[LenFindStr],MaxLen-LenFindStr);
    }
  }
  return TRUE;
}
/*  SVS $ */

//////////////////////////////////////////////////////////////////////////
/* Private:
   Заполняем выпадающий список для ComboBox
*/
/*
   $ 18.07.2000 SVS
   Функция-обработчик выбора из списка и установки...
*/
void Dialog::SelectFromComboBox(
         Edit *EditLine,                   // строка редактирования
         struct FarList *List,    // список строк
         char *IStr,
         int MaxLen)
{
  char *Str;
  struct MenuItem ComboBoxItem={0};
  struct FarListItem *ListItems=List->Items;
  int EditX1,EditY1,EditX2,EditY2;
  int I,Dest;

  if((Str=(char*)malloc(MaxLen)) != NULL)
  {
    // создание пустого вертикального меню
    //  с обязательным показом ScrollBar
    VMenu ComboBoxMenu("",NULL,0,8,VMENU_ALWAYSSCROLLBAR,NULL/*,this*/);

    EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);
    if (EditX2-EditX1<20)
      EditX2=EditX1+20;
    if (EditX2>ScrX)
      EditX2=ScrX;
  #if 0
    if(!(Item[FocusPos].Flags&DIF_LISTNOAMPERSAND))
  #endif
      ComboBoxMenu.SetFlags(MENU_SHOWAMPERSAND);
    ComboBoxMenu.SetPosition(EditX1,EditY1+1,EditX2,0);
    ComboBoxMenu.SetBoxType(SHORT_SINGLE_BOX);

    // заполнение пунктов меню
    /* Последний пункт списка - ограничититель - в нем Tetx[0]
       должен быть равен '\0'
    */
    for (Dest=I=0;I < List->ItemsNumber;I++)
    {
      memset(&ComboBoxItem,0,sizeof(ComboBoxItem));

      /* $ 28.07.2000 SVS
         Выставим Selected при полном совпадении строки ввода и списка
      */

      if(IStr && *IStr && !(ListItems[I].Flags&LIF_DISABLE))
      {
        if((ComboBoxItem.Selected=(!Dest &&
         !strncmp(IStr,
           ((ListItems[I].Flags&LIF_PTRDATA)?ListItems[I].Ptr.PtrData:ListItems[I].Text),
           (ListItems[I].Flags&LIF_PTRDATA)?ListItems[I].Ptr.PtrLength:sizeof(ListItems[I].Text)))?
           TRUE:FALSE) == TRUE)
           Dest++;
      }
      else
         ComboBoxItem.Selected=ListItems[I].Flags&LIF_SELECTED;

      ComboBoxItem.Separator=ListItems[I].Flags&LIF_SEPARATOR;
      ComboBoxItem.Checked=ListItems[I].Flags&LIF_CHECKED;
      ComboBoxItem.Disabled=ListItems[I].Flags&LIF_DISABLE;
      /* 01.08.2000 SVS $ */
      /* SVS $ */
      if(ListItems[I].Flags&LIF_PTRDATA)
      {
        // а может все таки влезим в нужные размеры?
        if(ListItems[I].Ptr.PtrLength < sizeof(ComboBoxItem.UserData))
        {
          strncpy(ComboBoxItem.Name,ListItems[I].Ptr.PtrData,sizeof(ComboBoxItem.Name)-1);
          strcpy(ComboBoxItem.UserData,ListItems[I].Ptr.PtrData);
        }
        else
        {
          ComboBoxItem.PtrData=ListItems[I].Ptr.PtrData;
          ComboBoxItem.Flags=1;
        }
        ComboBoxItem.UserDataSize=strlen(ListItems[I].Ptr.PtrData);
      }
      else
      {
        strcpy(ComboBoxItem.Name,ListItems[I].Text);
        strcpy(ComboBoxItem.UserData,ListItems[I].Text);
        ComboBoxItem.UserDataSize=strlen(ListItems[I].Text);
      }
      ComboBoxMenu.AddItem(&ComboBoxItem);
    }

    /* $ 28.07.2000 SVS
       Перед отрисовкой спросим об изменении цветовых атрибутов
    */
    short Colors[9];
    ComboBoxMenu.GetColors(Colors);
    if(DlgProc((HANDLE)this,DN_CTLCOLORDLGLIST,
                    sizeof(Colors)/sizeof(Colors[0]),(long)Colors))
      ComboBoxMenu.SetColors(Colors);
    /* SVS $ */

    ComboBoxMenu.Show();

    Dest=ComboBoxMenu.GetSelectPos();
    while (!ComboBoxMenu.Done())
    {
      int Key=ComboBoxMenu.ReadInput();
      // здесь можно добавить что-то свое, например,
      I=ComboBoxMenu.GetSelectPos();
      if(I != Dest)
      {
        if(!DlgProc((HANDLE)this,DN_LISTCHANGE,FocusPos,I))
          ComboBoxMenu.SetSelectPos(Dest,Dest<I?-1:1); //????
        else
          Dest=I;
      }
      //  обработку multiselect ComboBox

      ComboBoxMenu.ProcessInput();
    }

    int ExitCode=ComboBoxMenu.GetExitCode();
    if (ExitCode<0)
      return;
    /* Запомним текущее состояние */
    for (I=0; I < List->ItemsNumber; I++)
      ListItems[I].Flags&=~LIF_SELECTED;
    ListItems[ExitCode].Flags|=LIF_SELECTED;
    ComboBoxMenu.GetUserData(Str,MaxLen,ExitCode);

    EditLine->SetString(Str);
    EditLine->SetLeftPos(0);
    Redraw();
    free(Str);
  }
}
/* SVS $ */

//////////////////////////////////////////////////////////////////////////
/* Private:
   Заполняем выпадающий список из истории
*/
/* $ 26.07.2000 SVS
  + Дополнительный параметр в SelectFromEditHistory для выделения
   нужной позиции в истории (если она соответствует строке ввода)
*/
void Dialog::SelectFromEditHistory(Edit *EditLine,
                                   char *HistoryName,
                                   char *IStr,
                                   int MaxLen)
/* SVS $ */
/* $ 21.02.2001 IS
     Избавился от утечки памяти (проявлялось не у всех, но проявлялось же!)
*/
{
  char RegKey[80],KeyValue[80],*Str[HISTORY_COUNT]={0};
  int I,Dest;
  int Checked;
  int Final=FALSE;

  if(!EditLine)
    return;

  sprintf(RegKey,fmtSavedDialogHistory,HistoryName);
  while(1)
  {
    // создание пустого вертикального меню
    VMenu HistoryMenu("",NULL,0,8,VMENU_ALWAYSSCROLLBAR);

    struct MenuItem HistoryItem;
    int EditX1,EditY1,EditX2,EditY2;
    int ItemsCount;

    EditLine->GetPosition(EditX1,EditY1,EditX2,EditY2);
    if (EditX2-EditX1<20)
      EditX2=EditX1+20;
    if (EditX2>ScrX)
      EditX2=ScrX;

    memset(&HistoryItem,0,sizeof(HistoryItem));
    HistoryMenu.SetFlags(MENU_SHOWAMPERSAND);
    HistoryMenu.SetPosition(EditX1,EditY1+1,EditX2,0);
    HistoryMenu.SetBoxType(SHORT_SINGLE_BOX);

    // заполнение пунктов меню
    ItemsCount=0;
    for (Dest=I=0; I < HISTORY_COUNT; I++)
    {
      if((Str[I]=(char*)malloc(MaxLen+1)) == NULL)
        break;

      memset(&HistoryItem,0,sizeof(HistoryItem));

      sprintf(KeyValue,fmtLine,I);
      GetRegKey(RegKey,KeyValue,Str[I],"",MaxLen);
      if (*Str[I]==0)
        continue;

      sprintf(KeyValue,fmtLocked,I);

      GetRegKey(RegKey,KeyValue,(int)Checked,0);
      HistoryItem.Checked=Checked;
      /* $ 26.07.2000 SVS
         Выставим Selected при полном совпадении строки ввода и истории
      */
      if((HistoryItem.Selected=(!Dest && !strcmp(IStr,Str[I]))?TRUE:FALSE) == TRUE)
         Dest++;
      /* SVS $ */
      strncpy(HistoryItem.Name,Str[I],sizeof(HistoryItem.Name)-1);
      if(MaxLen < sizeof(HistoryItem.UserData))
        strncpy(HistoryItem.UserData,Str[I],sizeof(HistoryItem.UserData));
      else
      {
        HistoryItem.PtrData=Str[I];
        HistoryItem.Flags=1;
      }
      HistoryItem.UserDataSize=strlen(Str[I])+1;
      HistoryMenu.AddItem(&HistoryItem);
      ItemsCount++;
    }
    if (ItemsCount==0)
      {
        Final=TRUE;
        break;
      }

    /* $ 28.07.2000 SVS
       Перед отрисовкой спросим об изменении цветовых атрибутов
    */
    short Colors[9];
    HistoryMenu.GetColors(Colors);
    if(DlgProc((HANDLE)this,DN_CTLCOLORDLGLIST,
                    sizeof(Colors)/sizeof(Colors[0]),(long)Colors))
      HistoryMenu.SetColors(Colors);
    /* SVS $ */
    HistoryMenu.Show();
    while (!HistoryMenu.Done())
    {
      int Key=HistoryMenu.ReadInput();

      // Del очищает историю команд.
      if (Key==KEY_DEL)
      {
        int Locked;
        for (I=0,Dest=0; I < HISTORY_COUNT;I++)
        {
          sprintf(KeyValue,fmtLine,I);
          GetRegKey(RegKey,KeyValue,Str[I],"",MaxLen);
          DeleteRegValue(RegKey,KeyValue);
          sprintf(KeyValue,fmtLocked,I);
          GetRegKey(RegKey,KeyValue,Locked,0);
          DeleteRegValue(RegKey,KeyValue);

          // залоченные пункты истории не удаляются
          if (Locked)
          {
            sprintf(KeyValue,fmtLine,Dest);
            SetRegKey(RegKey,KeyValue,Str[I]);
            sprintf(KeyValue,fmtLocked,Dest);
            SetRegKey(RegKey,KeyValue,TRUE);
            Dest++;
          }
        }
        HistoryMenu.Hide();
        SelectFromEditHistory(EditLine,HistoryName,IStr,MaxLen);
        Final=TRUE;
        break;
      }

      // Ins защищает пункт истории от удаления.
      if (Key==KEY_INS)
      {
        sprintf(KeyValue,fmtLocked,HistoryMenu.GetSelectPos());
        if (!HistoryMenu.GetSelection())
        {
          HistoryMenu.SetSelection(TRUE);
          SetRegKey(RegKey,KeyValue,1);
        }
        else
        {
          HistoryMenu.SetSelection(FALSE);
          DeleteRegValue(RegKey,KeyValue);
        }
        HistoryMenu.SetUpdateRequired(TRUE);
        HistoryMenu.Redraw();
        continue;
      }

      // Tab в списке хистори - аналог Enter
      if (Key==KEY_TAB)
      {
        HistoryMenu.ProcessKey(KEY_ENTER);
        continue;
      }

      // Сюды надо добавить DN_LISTCHANGE

      HistoryMenu.ProcessInput();
    }

    if(Final) break;

    int ExitCode=HistoryMenu.GetExitCode();
    if (ExitCode<0)
      {
        Final=TRUE;
        break;
      }

    HistoryMenu.GetUserData(Str[0],MaxLen,ExitCode);

    break;
  }

  if(!Final && Str[0])
  {
    EditLine->SetString(Str[0]);
    EditLine->SetLeftPos(0);
    Redraw();
  }

  for (I=0; I < HISTORY_COUNT; I++)
    if(Str[I])
      free(Str[I]);
}
/* IS $ */

//////////////////////////////////////////////////////////////////////////
/* Private:
   Работа с историей - добавление и reorder списка
*/
int Dialog::AddToEditHistory(char *AddStr,char *HistoryName,int MaxLen)
{
  int LastLine=HISTORY_COUNT-1,FirstLine=HISTORY_COUNT, I, Locked;
  char *Str;

  if (*AddStr==0)
    return FALSE;

  if((Str=(char*)malloc(MaxLen+1)) == NULL)
    return FALSE;

  char RegKey[80],SrcKeyValue[80],DestKeyValue[80];
  sprintf(RegKey,fmtSavedDialogHistory,HistoryName);

  for (I=0; I < HISTORY_COUNT; I++)
  {
    sprintf(SrcKeyValue,fmtLocked,I);
    GetRegKey(RegKey,SrcKeyValue,Locked,0);
    if (!Locked)
    {
      FirstLine=I;
      break;
    }
  }

  for (I=0; I < HISTORY_COUNT; I++)
  {
    sprintf(SrcKeyValue,fmtLine,I);
    GetRegKey(RegKey,SrcKeyValue,Str,"",MaxLen);
    if (strcmp(Str,AddStr)==0)
    {
      LastLine=I;
      break;
    }
  }

  if (FirstLine<=LastLine)
  {
    for (int Src=LastLine-1;Src>=FirstLine;Src--)
    {
      sprintf(SrcKeyValue,fmtLocked,Src);
      GetRegKey(RegKey,SrcKeyValue,Locked,0);
      if (Locked)
        continue;
      for (int Dest=Src+1;Dest<=LastLine;Dest++)
      {
        sprintf(DestKeyValue,fmtLocked,Dest);
        GetRegKey(RegKey,DestKeyValue,Locked,0);
        if (!Locked)
        {
          sprintf(SrcKeyValue,fmtLine,Src);
          GetRegKey(RegKey,SrcKeyValue,Str,"",MaxLen);
          sprintf(DestKeyValue,fmtLine,Dest);
          SetRegKey(RegKey,DestKeyValue,Str);
          break;
        }
      }
    }
    char FirstLineKeyValue[20];
    sprintf(FirstLineKeyValue,fmtLine,FirstLine);
    SetRegKey(RegKey,FirstLineKeyValue,AddStr);
  }
  free(Str);
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////
/* Public, Static:
   Проверка на HotKey
*/
/* $ 20.02.2001 SVS
   Пересмотр алгоритма IsKeyHighlighted с добавками Alt- на
   сколько это возможно*/
int Dialog::IsKeyHighlighted(char *Str,int Key,int Translate)
{
  if ((Str=strchr(Str,'&'))==NULL)
    return(FALSE);
  int UpperStrKey=LocalUpper(Str[1]);
  /* $ 08.11.2000 SVS
     Изменен пересчет кодов клавиш для hotkey (используются сканкоды)
  */
  /* 28.12.2000 SVS
    + добавлена обработка Opt.HotkeyRules */
  if (Key < 256)
  {
    int KeyToKey=LocalKeyToKey(Key);
    return(UpperStrKey == LocalUpper(Key) ||
      Translate &&
      (!Opt.HotkeyRules && UpperStrKey==LocalUpper(KeyToKey) ||
        Opt.HotkeyRules && LocalKeyToKey(UpperStrKey)==KeyToKey));
  }

  if(Key&KEY_ALT)
  {
    int AltKey=Key&(~KEY_ALT);
    if(AltKey < 256)
    {
      if (AltKey >= '0' && AltKey <= '9')
        return(AltKey==UpperStrKey);

      int AltKeyToKey=LocalKeyToKey(AltKey);
      if (AltKey > ' ' && AltKey <= 255)
  //         (AltKey=='-'  || AltKey=='/' || AltKey==','  || AltKey=='.' ||
  //          AltKey=='\\' || AltKey=='=' || AltKey=='['  || AltKey==']' ||
  //          AltKey==':'  || AltKey=='"' || AltKey=='~'))
      {
        return(UpperStrKey==LocalUpper(AltKey) ||
               Translate &&
               (!Opt.HotkeyRules && UpperStrKey==LocalUpper(AltKeyToKey) ||
                  Opt.HotkeyRules && LocalKeyToKey(UpperStrKey)==AltKeyToKey));
      }
    }
  }
  /* SVS $*/
  /* SVS $*/
  return(FALSE);
}
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
/* Private:
   Если жмакнули Alt-???
*/
int Dialog::ProcessHighlighting(int Key,int FocusPos,int Translate)
{
  int I, Type;
  DWORD Flags;
  for (I=0;I<ItemCount;I++)
  {
    Type=Item[I].Type;
    Flags=Item[I].Flags;

    if ((!IsEdit(Type) || (Type == DI_COMBOBOX && (Flags&DIF_DROPDOWNLIST))) &&
        (Flags & (DIF_SHOWAMPERSAND|DIF_DISABLE|DIF_HIDDEN))==0)
      if (IsKeyHighlighted(Item[I].Data,Key,Translate))
      {
        int DisableSelect=FALSE;

        // Если ЭТО: Edit(пред контрол) и DI_TEXT в одну строку, то...
        if (I>0 &&
            Type==DI_TEXT &&                              // DI_TEXT
            IsEdit(Item[I-1].Type) &&                     // и редактор
            Item[I].Y1==Item[I-1].Y1 &&                   // и оба в одну строку
            (I+1 < ItemCount && Item[I].Y1!=Item[I+1].Y1)) // ...и следующий контрол в другой строке
        {
          if((Item[I-1].Flags&(DIF_DISABLE|DIF_HIDDEN)) != 0) // и не задисаблен
             break;
          // Сообщим о случивщемся факте процедуре обработки диалога
          if(!DlgProc((HANDLE)this,DN_HOTKEY,I,Key))
            break; // сказали не продолжать обработку...
          I=ChangeFocus(I,-1,FALSE);
          DisableSelect=TRUE;
        }
        else if (Item[I].Type==DI_TEXT      || Item[I].Type==DI_VTEXT ||
                 Item[I].Type==DI_SINGLEBOX || Item[I].Type==DI_DOUBLEBOX)
        {
          if(I+1 < ItemCount && // ...и следующий контрол
            (Item[I+1].Flags&(DIF_DISABLE|DIF_HIDDEN)) != 0) // и не задисаблен
             break;
          // Сообщим о случивщемся факте процедуре обработки диалога
          if(!DlgProc((HANDLE)this,DN_HOTKEY,I,Key))
            break; // сказали не продолжать обработку...
          I=ChangeFocus(I,1,FALSE);
          DisableSelect=TRUE;
        }
        /* $ 29.08.2000 SVS
           - Первый официальный альфа-баг - функция ProcessHighlighting
           MY> Работа с диалогами стала ГЛЮЧНАЯ. Я имею в виду горячие клавиши.
           MY> Входим в настройку чего угодно, жмем Alt-нужную букву и
           MY> наблюдаем разнообразные глюки.

           А ларчик просто открывался :-)))
        */
        // Сообщим о случивщемся факте процедуре обработки диалога
        if(!DlgProc((HANDLE)this,DN_HOTKEY,I,Key))
          break; // сказали не продолжать обработку...
        ChangeFocus2(FocusPos,I);
        /* SVS $ */
        if ((Item[I].Type==DI_CHECKBOX || Item[I].Type==DI_RADIOBUTTON) &&
            (!DisableSelect || (Item[I].Flags & DIF_MOVESELECT)))
        {
          ProcessKey(KEY_SPACE);
          return(TRUE);
        }
        else if (Item[I].Type==DI_BUTTON)
        {
          ProcessKey(KEY_ENTER);
          return(TRUE);
        }
        // при ComboBox`е - "вываливаем" последний //????
        else if (Item[I].Type==DI_COMBOBOX)
        {
          ProcessKey(KEY_CTRLDOWN);
          return(TRUE);
        }
        ShowDialog();
        return(TRUE);
      }
  }
  return(FALSE);
}


//////////////////////////////////////////////////////////////////////////
/* $ 31.07.2000 tran
   + функция подравнивания координат edit классов */
/* $ 07.08.2000 SVS
   + а про ListBox забыли?*/
void Dialog::AdjustEditPos(int dx, int dy)
{
  struct DialogItem *CurItem;
  int I;
  int x1,x2,y1,y2;

  if(!CheckDialogMode(DMODE_CREATEOBJECTS))
    return;

  ScreenObject *DialogEdit;
  for (I=0; I < ItemCount; I++)
  {
    CurItem=&Item[I];
    if (CurItem->ObjPtr && (IsEdit(CurItem->Type) || CurItem->Type == DI_LISTBOX))
    {
       DialogEdit=(ScreenObject *)CurItem->ObjPtr;
       DialogEdit->GetPosition(x1,y1,x2,y2);
       x1+=dx;
       x2+=dx;
       y1+=dy;
       y2+=dy;
       DialogEdit->SetPosition(x1,y1,x2,y2);
    }
  }
}
/* SVS $ */
/* tran 31.07.2000 $ */


//////////////////////////////////////////////////////////////////////////
/* $ 11.08.2000 SVS
   Работа с доп. данными экземпляра диалога
   Пока простое копирование (присвоение)
*/
void Dialog::SetDialogData(long NewDataDialog)
{
  DataDialog=NewDataDialog;
}
/* SVS $ */

//////////////////////////////////////////////////////////////////////////
/* $ 11.08.2000 SVS
   + Для того, чтобы послать DM_CLOSE нужно переопределить Process
*/
void Dialog::Process()
{
  do{
    Modal::Process();
    /* $ 21.12.2000 SVS
       Ctr-Break теперь недействителен, т.е. все зависит от того
       что вернет обработчик.
    */
    if(DlgProc((HANDLE)this,DM_CLOSE,ExitCode,0))// || ExitCode == -2)
      break;
    /* SVS $ */
    /* $ 18.08.2000 SVS
       Вах-вах, а сбросить-то флаг забыли 8-=(((
    */
    ClearDone();
    /* SVS $ */
  }while(1);
}
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   функция обработки диалога (по умолчанию)
   Вот именно эта функция и является последним рубежом обработки диалога.
   Т.е. здесь должна быть ВСЯ обработка ВСЕХ сообщений!!!
*/
long WINAPI Dialog::DefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
  struct DialogItem *CurItem;
  char *Ptr, Str[1024];
  int Len, Type, I;

  if(!Dlg)
    return 0;

  switch(Msg)
  {
    case DN_INITDIALOG:
      return FALSE; // изменений не было!

    case DM_CLOSE:
      return TRUE;  // согласен с закрытием

    case DN_KILLFOCUS:
      return -1;    // "Согласен с потерей фокуса"

    case DN_GOTFOCUS:
      return 0;     // always 0

    case DN_HELP:
      return Param2; // что передали, то и...

    case DN_DRAWDIALOG:
    {
      if(Param1 == 1)  // Нужно отрисовать "салазки"?
      {
        /* $ 03.08.2000 tran
           вывод текста в углу может приводить к ошибкам изображения
           1) когда диалог перемещается в угол
           2) когда диалог перемещается из угла
           сделал вывод красных палочек по углам */
        Text(Dlg->X1,Dlg->Y1,0xCE,"\\");
        Text(Dlg->X1,Dlg->Y2,0xCE,"/");
        Text(Dlg->X2,Dlg->Y1,0xCE,"/");
        Text(Dlg->X2,Dlg->Y2,0xCE,"\\");
      }
      return TRUE;
    }

    case DN_CTLCOLORDIALOG:
      return Param2;

    case DN_CTLCOLORDLGITEM:
      return Param2;

    case DN_CTLCOLORDLGLIST:
      return FALSE;

    case DN_ENTERIDLE:
      return 0;     // always 0
  }

  // предварительно проверим...
  if(Param1 >= Dlg->ItemCount)
    return 0;

  CurItem=&Dlg->Item[Param1];
  Type=CurItem->Type;

  Ptr=CurItem->Data;

  switch(Msg)
  {
    case DN_MOUSECLICK:
      return FALSE;

    case DN_DRAWDLGITEM:
      return TRUE;

    case DN_HOTKEY:
      return TRUE;

    case DN_EDITCHANGE:
      return TRUE;

    case DN_BTNCLICK:
      return TRUE;

    case DN_LISTCHANGE:
      return TRUE;

    /* $ 23.08.2000 SVS
       + получить клавишу(ы)
    */
    case DM_KEY:
      return FALSE;
    /* SVS $ */
  }

  return 0;
}
/* SVS $ */


//////////////////////////////////////////////////////////////////////////
/* $ 28.07.2000 SVS
   Посылка сообщения диалогу
   Некоторые сообщения эта функция обрабатывает сама, не передавая управление
   обработчику диалога.
*/
long WINAPI Dialog::SendDlgMessage(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
  struct DialogItem *CurItem;
  char *Ptr, Str[1024];
  int Len, Type, I;
  struct FarDialogItem PluginDialogItem;

  if(!Dlg)
    return 0;
  // предварительно проверим...
  if(Param1 >= Dlg->ItemCount)
    return 0;

//  CurItem=&Dlg->Item[Param1];
  CurItem=Dlg->Item+Param1;
  Type=CurItem->Type;
  Ptr=CurItem->Data;

  switch(Msg)
  {
    case DM_ADDHISTORY:
      if(Param2 &&
         (Type==DI_EDIT || Type==DI_FIXEDIT) &&
         (CurItem->Flags & DIF_HISTORY))
      {
        return Dlg->AddToEditHistory((char*)Param2,CurItem->History,strlen((char*)Param2)+1);
      }
      return FALSE;
    /* $ 23.10.2000 SVS
       Получить/установить позицию в строках редактирования
    */
    case DM_GETCURSORPOS:
      if(!CurItem->ObjPtr || !Param2)
        return FALSE;
      if (IsEdit(Type))
      {
        ((COORD*)Param2)->X=((Edit *)(CurItem->ObjPtr))->GetCurPos();
        ((COORD*)Param2)->Y=0;
        return TRUE;
      }
      else if(Type == DI_USERCONTROL)
      {
        ((COORD*)Param2)->X=((COORD*)(CurItem->ObjPtr))->X;
        ((COORD*)Param2)->Y=((COORD*)(CurItem->ObjPtr))->Y;
        return TRUE;
      }
      return FALSE;

    case DM_SETCURSORPOS:
      if(!CurItem->ObjPtr)
        return FALSE;
      if (IsEdit(Type))
      {
        ((Edit *)(CurItem->ObjPtr))->SetCurPos(((COORD*)Param2)->X);
        return TRUE;
      }
      else if(Type == DI_USERCONTROL)
      {
        // учтем, что координаты для этого элемента всегда относительные!
        //  и начинаются с 0,0
        COORD Coord=*(COORD*)Param2;
        Coord.X+=CurItem->X1;
        if(Coord.X > CurItem->X2)
          Coord.X=CurItem->X2;

        Coord.Y+=CurItem->Y1;
        if(Coord.Y > CurItem->Y2)
          Coord.Y=CurItem->Y2;

        // Запомним
        ((COORD*)(CurItem->ObjPtr))->X=Coord.X-CurItem->X1;
        ((COORD*)(CurItem->ObjPtr))->Y=Coord.Y-CurItem->Y1;
        // переместим если надо
        if(Dlg->CheckDialogMode(DMODE_SHOW) && Dlg->FocusPos == Param1)
        {
           // что-то одно надо убрать :-)
           MoveCursor(Coord.X+Dlg->X1,Coord.Y+Dlg->Y1); // ???
           Dlg->ShowDialog(); //???
        }
        return TRUE;
      }
      return FALSE;
    /* SVS $ */


    case DN_LISTCHANGE:
    {
      return Dlg->DlgProc(hDlg,Msg,Param1,Param2);
    }

    case DN_EDITCHANGE:
    {
      Dialog::ConvertItem(CVTITEM_TOPLUGIN,&PluginDialogItem,CurItem,1,TRUE);
      if((I=Dlg->DlgProc(hDlg,Msg,Param1,(long)&PluginDialogItem)) == TRUE)
        Dialog::ConvertItem(CVTITEM_FROMPLUGIN,&PluginDialogItem,CurItem,1,TRUE);
      return I;
    }

    case DN_BTNCLICK:
      return Dlg->DlgProc(hDlg,Msg,Param1,Param2);

    case DN_DRAWDLGITEM:
      // преобразуем данные для!
      Dialog::ConvertItem(CVTITEM_TOPLUGIN,&PluginDialogItem,CurItem,1);
      I=Dlg->DlgProc(hDlg,Msg,Param1,(long)&PluginDialogItem);
      Dialog::ConvertItem(CVTITEM_FROMPLUGIN,&PluginDialogItem,CurItem,1);
      return I;

    case DM_SETREDRAW:
      if(Dlg->CheckDialogMode(DMODE_INITOBJECTS))
        Dlg->Show();
      return 0;

    /* $ 08.09.2000 SVS
      - Если коротко, то DM_SETFOCUS вроде как и работал :-)
    */
    case DM_SETFOCUS:
//      if(!Dialog::IsFocused(Dlg->Item[Param1].Type))
//        return FALSE;
      if(Dlg->ChangeFocus2(Dlg->FocusPos,Param1) == Param1)
      {
        Dlg->ShowDialog();
        return TRUE;
      }
      return FALSE;
    /* SVS $ */

    /* $ 20.10.2000 SVS
      Получить ID фокуса
    */
    case DM_GETFOCUS:
      return Dlg->FocusPos;
    /* SVS $ */

    case DM_GETTEXTPTR:
      if(Param2)
      {
        struct FarDialogItemData IData;
        IData.PtrData=(char *)Param2;
        IData.PtrLength=0;
        return Dialog::SendDlgMessage(hDlg,DM_GETTEXT,Param1,(long)&IData);
      }

    case DM_GETTEXT:
      if(Param2) // если здесь NULL, то это еще один способ получить размер
      {
        struct FarDialogItemData *did=(struct FarDialogItemData*)Param2;
        Len=0;
        switch(Type)
        {
          case DI_COMBOBOX:
          case DI_EDIT:
          case DI_PSWEDIT:
          case DI_FIXEDIT:
            if(!CurItem->ObjPtr)
              break;
            ((Edit *)(CurItem->ObjPtr))->GetString(Str,sizeof(Str));
            Ptr=Str;

          case DI_TEXT:
          case DI_VTEXT:
          case DI_SINGLEBOX:
          case DI_DOUBLEBOX:
          case DI_CHECKBOX:
          case DI_RADIOBUTTON:
          case DI_BUTTON:

            Len=strlen(Ptr)+1;
            if (!(CurItem->Flags & DIF_NOBRACKETS) && Type == DI_BUTTON)
            {
              Ptr+=2;
              Len-=4;
            }

            if(!did->PtrLength)
              did->PtrLength=Len;
            else if(Len > did->PtrLength)
              Len=did->PtrLength;

            if(Len > 0 && did->PtrData)
            {
              memmove(did->PtrData,Ptr,Len);
              did->PtrData[Len]=0;
            }
            break;

          case DI_USERCONTROL:
            did->PtrLength=CurItem->Ptr.PtrLength;
            did->PtrData=(char*)CurItem->Ptr.PtrData;
            break;

          case DI_LISTBOX: // пока не трогаем - не реализован
          {
            if(!CurItem->ObjPtr)
              break;
            VMenu *VMenuPtr=(VMenu *)(CurItem->ObjPtr);
            did->PtrLength=VMenuPtr->GetUserData(did->PtrData,did->PtrLength,-1);
            break;
          }

          default:  // подразумеваем, что остались
            did->PtrLength=0;
            break;
        }
        return Len;
      }
      // здесь умышленно не ставим return, т.к. хотим получить размер
      // следовательно сразу должен идти "case DM_GETTEXTLENGTH"!!!

    case DM_GETTEXTLENGTH:
      switch(Type)
      {
        case DI_BUTTON:
          Len=strlen(Ptr)+1;
          if (!(CurItem->Flags & DIF_NOBRACKETS))
            Len-=4;
          break;

        case DI_USERCONTROL:
          Len=CurItem->Ptr.PtrLength;
          break;

        case DI_TEXT:
        case DI_VTEXT:
        case DI_SINGLEBOX:
        case DI_DOUBLEBOX:
        case DI_CHECKBOX:
        case DI_RADIOBUTTON:
          Len=strlen(Ptr)+1;
          break;

        case DI_COMBOBOX:
        case DI_EDIT:
        case DI_PSWEDIT:
        case DI_FIXEDIT:
          if(CurItem->ObjPtr)
            Len=((Edit *)(CurItem->ObjPtr))->GetLength();

        case DI_LISTBOX:
          Len=0;
          if(CurItem->ObjPtr)
          {
            VMenu *VMenuPtr=(VMenu *)(CurItem->ObjPtr);
            Len=VMenuPtr->GetUserData(NULL,0,-1);
          }
          break;

        default:
          Len=0;
          break;
      }
      return Len;

    case DM_SETTEXTPTR:
    {
      if(!Param2)
        return 0;

      struct FarDialogItemData IData;
      IData.PtrData=(char *)Param2;
      IData.PtrLength=strlen(IData.PtrData);
      return Dialog::SendDlgMessage(hDlg,DM_SETTEXT,Param1,(long)&IData);
    }


    case DM_SETTEXT:
      if(Param2)
      {
        struct FarDialogItemData *did=(struct FarDialogItemData*)Param2;
        Len=0;
        switch(Type)
        {
          case DI_USERCONTROL:
            CurItem->Ptr.PtrLength=did->PtrLength;
            CurItem->Ptr.PtrData=did->PtrData;
            return CurItem->Ptr.PtrLength;

          case DI_TEXT:
          case DI_VTEXT:
          case DI_SINGLEBOX:
          case DI_DOUBLEBOX:
            if((Len=did->PtrLength) == NULL)
            {
              strncpy(Ptr,(char *)did->PtrData,511);
              Len=strlen(Ptr)+1;
            }
            else
            {
              if((unsigned)did->PtrLength > 511)
                Len=511;
              memmove(Ptr,(char *)did->PtrData,Len);
              Ptr[Len]=0;
            }
            if(Dlg->CheckDialogMode(DMODE_SHOW))
            {
              Dlg->ShowDialog(Param1);
              ScrBuf.Flush();
            }
            return Len;

          case DI_BUTTON:
          case DI_CHECKBOX:
          case DI_RADIOBUTTON:
            if((Len=did->PtrLength) == NULL)
            {
              strncpy(Ptr,(char *)did->PtrData,511);
              Len=strlen(Ptr)+1;
            }
            else
            {
              if((unsigned)did->PtrLength > 511)
                Len=511;
              memmove(Ptr,(char *)did->PtrData,Len);
              Ptr[Len]=0;
            }
            break;

          case DI_COMBOBOX:
          case DI_EDIT:
          case DI_PSWEDIT:
          case DI_FIXEDIT:
            if((Len=did->PtrLength) == NULL)
            {
              strncpy(Ptr,(char *)did->PtrData,511);
              Len=strlen(Ptr)+1;
            }
            else
            {
              if((unsigned)did->PtrLength > 511)
                Len=511;
              memmove(Ptr,(char *)did->PtrData,Len);
              Ptr[Len]=0;
            }
            if(CurItem->ObjPtr)
            {
              ((Edit *)(CurItem->ObjPtr))->SetString((char *)Ptr);
              ((Edit *)(CurItem->ObjPtr))->Select(-1,-1); // снимаем выделение
            }
            break;

          case DI_LISTBOX: // пока не трогаем - не реализован
            return 0;

          default:  // подразумеваем, что остались
            return 0;
        }
        Dlg->InitDialogObjects(Param1); // переинициализируем элементы диалога
        if(Dlg->CheckDialogMode(DMODE_SHOW)) // достаточно ли этого????!!!!
        {
          Dlg->ShowDialog(Param1);
          ScrBuf.Flush();
        }
        return strlen((char *)Ptr)+1; //???
      }
      return 0;

    case DM_SETTEXTLENGTH:
      if((Type==DI_EDIT || Type==DI_PSWEDIT ||
          (Type==DI_COMBOBOX && !(CurItem->Flags & DIF_DROPDOWNLIST))) &&
         CurItem->ObjPtr)
      {
        int MaxLen=((Edit *)(CurItem->ObjPtr))->GetMaxLength();

        if((CurItem->Type==DI_EDIT || CurItem->Type==DI_COMBOBOX) &&
           (CurItem->Flags&DIF_VAREDIT))
          CurItem->Ptr.PtrLength=Param2; //???
        else if(Param2 > 511)
          Param2=511;

        ((Edit *)(CurItem->ObjPtr))->SetMaxLength(Param2);

        //if (CheckDialogMode(DMODE_INITOBJECTS)) //???
        Dlg->InitDialogObjects(Param1); // переинициализируем элементы диалога
        return MaxLen;
      }
      return 0;

    case DM_GETDLGITEM:
      if(Param2)
      {
        Dialog::ConvertItem(CVTITEM_TOPLUGIN,(struct FarDialogItem *)Param2,CurItem,1);
/*
        if(IsEdit(Type))
        {
          ((Edit *)(CurItem->ObjPtr))->GetString(Str,sizeof(Str));
          strcpy((char *)Param2,Str);
        }
        else
          strcpy(((struct FarDialogItem *)Param2)->Data,CurItem->Data);
*/
        return TRUE;
      }
      return FALSE;

    case DM_SETDLGITEM:
      if(Param2 &&
         Type == ((struct FarDialogItem *)Param2)->Type) // пока нефига менять тип
      {
        Dialog::ConvertItem(CVTITEM_FROMPLUGIN,(struct FarDialogItem *)Param2,CurItem,1);
        CurItem->Type=Type;
        // еще разок, т.к. данные могли быть изменены
        Dlg->InitDialogObjects(Param1);
        if(Dlg->CheckDialogMode(DMODE_SHOW))
        {
          Dlg->ShowDialog(Param1);
          ScrBuf.Flush();
        }
        return TRUE;
      }
      return FALSE;


    /* $ 18.08.2000 SVS
       + Разрешение/запрещение отрисовки диалога
    */
    case DM_ENABLEREDRAW:
      if(Param1)
        Dlg->IsEnableRedraw++;
      else
        Dlg->IsEnableRedraw--;

      if(!Dlg->IsEnableRedraw)
        if(Dlg->CheckDialogMode(DMODE_INITOBJECTS))
          Dlg->Show();
      return 0;
    /* SVS $ */

    /* $ 23.08.2000 SVS
       + показать/спрятать диалог.
    */
    case DM_SHOWDIALOG:
//      if(!Dlg->IsEnableRedraw)
      {
        if(Param1)
        {
          if(!Dlg->IsVisible())
            Dlg->Show();
        }
        else
        {
          if(Dlg->IsVisible())
            Dlg->Hide();
        }
      }
      return 0;
    /* SVS $ */

    /* $ 23.08.2000 SVS
       + установить/взять данные диалога.
    */
    case DM_SETDLGDATA:
    {
      long PrewDataDialog=Dlg->DataDialog;
      Dlg->DataDialog=Param2;
      return PrewDataDialog;
    }

    case DM_GETDLGDATA:
    {
      return Dlg->DataDialog;
    }
    /* SVS $ */

    /* $ 23.08.2000 SVS
       + послать клавишу(ы)
    */
    case DM_KEY:
    {
      int *KeyArray=(int*)Param2;
      Dlg->SetDialogMode(DMODE_KEY);
      for(I=0; I < Param1; ++I)
        Dlg->ProcessKey(KeyArray[I]);
      Dlg->SkipDialogMode(DMODE_KEY);
      return 0;
    }
    /* SVS $ */

    /* $ 23.08.2000 SVS
       + принудительно закрыть диалог
    */
    case DM_CLOSE:
      if(Param1 == -1)
        Dlg->ExitCode=Dlg->FocusPos;
      else
        Dlg->ExitCode=Param1;
      Dlg->EndLoop=TRUE;
      return TRUE;  // согласен с закрытием
    /* SVS $ */

    /* $ 25.08.2000 SVS
        + получить координаты диалогового окна
    */
    case DM_GETDLGRECT:
    {
      if(Param2)
      {
        int x1,y1,x2,y2;
        Dlg->GetPosition(x1,y1,x2,y2);
        ((SMALL_RECT*)Param2)->Left=x1;
        ((SMALL_RECT*)Param2)->Top=y1;
        ((SMALL_RECT*)Param2)->Right=x2;
        ((SMALL_RECT*)Param2)->Bottom=y2;
        return TRUE;
      }
      return FALSE;
    }
    /* SVS $ */

    /* $ 30.08.2000 SVS
        + программное перемещение диалога
    */
    case DM_MOVEDIALOG:
    {
      int W1,H1;

      W1=Dlg->X2-Dlg->X1;
      H1=Dlg->Y2-Dlg->Y1;
      // сохранили
      Dlg->OldX1=Dlg->X1;
      Dlg->OldY1=Dlg->Y1;
      Dlg->OldX2=Dlg->X2;
      Dlg->OldY2=Dlg->Y2;
      // переместили
      if(Param1)   // абсолютно?
      {
        Dlg->X1=((COORD*)Param2)->X;
        Dlg->Y1=((COORD*)Param2)->Y;
//        if(Dlg->X1 == -1 || Dlg->Y1 == -1)
        Dlg->CheckDialogCoord();
      }
      else         // значит относительно
      {
        Dlg->X1+=((COORD*)Param2)->X;
        Dlg->Y1+=((COORD*)Param2)->Y;
      }

      // проверили и скорректировали
      if(Dlg->X1 < 0)         Dlg->X1=0;
      if(Dlg->Y1 < 0)         Dlg->Y1=0;
      if(Dlg->X1+W1 >= ScrX)  Dlg->X1=ScrX-W1; //?
      if(Dlg->Y1+H1 >= ScrY)  Dlg->Y1=ScrY-H1; //?
      Dlg->X2=Dlg->X1+W1;
      Dlg->Y2=Dlg->Y1+H1;
      ((COORD*)Param2)->X=Dlg->X1;
      ((COORD*)Param2)->Y=Dlg->Y1;

      I=Dlg->IsVisible();// && Dlg->CheckDialogMode(DMODE_INITOBJECTS);
      if(I) Dlg->Hide();
      // приняли.
      Dlg->AdjustEditPos(Dlg->X1-Dlg->OldX1,Dlg->Y1-Dlg->OldY1);
      if(I) Dlg->Show(); // только если диалог был виден

      return Param2;
    }
    /* SVS $ */

    /* $ 31.08.2000 SVS
        + переключение/получение состояния Enable/Disable элемента
    */
    case DM_ENABLE:
    {
      DWORD PrevFlags=CurItem->Flags;
      if(Param2 != -1)
      {
         if(Param2)
           CurItem->Flags&=~DIF_DISABLE;
         else
           CurItem->Flags|=DIF_DISABLE;
      }
      return (PrevFlags&DIF_DISABLE)?FALSE:TRUE;
    }
    /* SVS $ */

    /* $ 03.01.2001 SVS
        + показать/скрыть элемент
        Param2: -1 - получить состояние
                 0 - погасить
                 1 - показать
        Return:  предыдущее состояние
    */
    case DM_SHOWITEM:
    {
      DWORD PrevFlags=CurItem->Flags;
      if(Param2 != -1)
      {
         if(Param2)
           CurItem->Flags&=~DIF_HIDDEN;
         else
           CurItem->Flags|=DIF_HIDDEN;
      }
      return (PrevFlags&DIF_HIDDEN)?FALSE:TRUE;
    }
    /* SVS $ */
  }

  // Все, что сами не отрабатываем - посылаем на обработку обработчику.
  return Dlg->DlgProc(hDlg,Msg,Param1,Param2);
}
/* SVS $ */

//////////////////////////////////////////////////////////////////////////
