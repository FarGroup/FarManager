/*
vmenu.cpp

Обычное вертикальное меню
  а так же:
    * список в DI_COMBOBOX
    * список в DI_LISTBOX
    * ...
*/

/* Revision: 1.145 29.01.2005 $ */

/*
Modify:
  29.01.2005 WARP
    ! Небольшой cleanup (см. 01920.vmenu_dialog_cleanup.txt)
  27.12.2004 WARP
    ! Сделал критические секции при практически всех вызовах Dialog & VMenu
  12.12.2004 WARP
    ! Оптимизация с VMENU_DISABLEBACKGROUND не совсем удалась.
  08.12.2004 WARP
    ! Патч для поиска #1. Подробнее 01864.FindFile.txt
  01.12.2004 WARP
    ! Устраняем лишние перерисовки VMenu
  19.11.2004 WARP
    ! "Нормальная" реализация UpdateRequired для VMenu,
      более интеллектальное обновление листа в поске.
  11.11.2004 SVS
    + Обработка MCODE_V_ITEMCOUNT и MCODE_V_CURPOS
  27.08/2004 VVM
    - Некорректное исправление от 20.07.2004
      Перестало работать корректировка выделения при добавлении в пустой список.
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  02.08.2004 SVS
    - BugZ#1144 - Непонятки c CheckHotkey
      Забыл учесть AmpPos
  20.07.2004 KM
    - Падение меню в AdjustSelectPos при SelectPos=-1
  08.07.2004 SVS
    + обработка MCODE_OP_PLAINTEXT в VMenu
  07.07.2004 SVS
    ! Macro II
  30.06.2004 SVS
    + CheckHighlights() - проверка "есть ли такой хоткей в меню"
      (в "обычном" ФАРе не работает, т.к. ограничена дефайном MACRODRIVE2)
    + Небольшая добавка по поводу Macro II
      (в "обычном" ФАРе не работает, т.к. ограничена дефайном MACRODRIVE2)
  31.05.2004 SVS
    ! выкинем нафиг MCODE_OP_SENDKEY - ненужен
  11.05.2004 SVS
    ! Вызов VMenu::ParentDialog->ProcessKey() в VMenu::ProcessKey уберем, т.к.
      в этом случае получаем зацикливание.
    + Из VMenu сделаем нотификацию DN_LISTHOTKEY для диалоговой процедуры.
  24.02.2004 SVS
    ! Проверка на макро USE_WFUNC
  19.02.2004 SVS
    - Некорретная отрисовка меню в W-режиме в связи с применением Text() вместо BoxText()
  19.01.2004 SVS
    ! уточнение VMENU_SELECTPOSNONE
  12.01.2004 SVS
    - падение в VMenu::GetUserData,  если GetPosition(Position) вернул -1
    - BugZ#1010 -   не работает DM_LISTGETCURPOS
  05.01.2004 SVS
    + VMENU_SELECTPOSNONE - признак того, что SelectPos не выставлен (например, все элементы списка задисаблены)
    ! Корректировка SelectPos (по флагу VMENU_SELECTPOSNONE)
  25.11.2003 SVS
    ! ConsoleTitle должен создаваться по мере надобности, а не в конструкторе
  05.11.2003 SVS
    + VMENU_CHANGECONSOLETITLE, OldTitle
  27.10.2003 SVS
    + В VMenu::AddItem() засунем критические секции
  23.10.2003 SVS
    + Обработка KEY_MACRO_EMPTY, KEY_MACRO_SELECTED, KEY_MACRO_EOF и KEY_MACRO_BOF
    ! VMenuCSection -> CSection
    ! вернем обратно вайлы и слипы
  14.10.2003 SVS
    + VMenuCSection
    ! цикл со счетчиком и Sleep(10) убраны в пользу критических секций
  06.10.2003 SVS
    - BugZ#966 - У диалога съезжает рамка при его перещении
      Для VMENU_LISTBOX не делаем операцию "X2=ScrX-2;"
  26.09.2003 SVS
    ! Изменения в названиях макроклавиш
  17.06.2003
   - Bug: теперь в плагинах заголовки на листах появляются только на
          пустых листах, а на листах с элементами - нет.
   - Bug: при удалении из DI_LISTBOX после 1663 курсор скакал на поз.0
          сделаем более правильное решение - вычислим "правильную" позицию
   ! Вместо CallCount++ и CallCount-- применим InterlockedIncrement(&CallCount)
     и InterlockedDecrement(&CallCount)
  27.05.2003 SVS
    + VMenu::DrawTitles() - кусок кода, прорисовывающий заголовки вынесен в
      эту функцию. Это бага про
      "DM_LISTSETTITLES не работает, если в листе ни одного элемента."
    - DI_LISTBOX, Bugs: "При удалении элементов списка, если курсор стоит
      не на удаляемом элементе, то курсоров становится 2"
      Здесь сбросим все селекты сразу после удаления и "пересчитаем"
      по новой.
  10.05.2003 SVS
    ! В VMenu::SortItems() добавлен третий параметр SortForDataDWORD,
      который заставляет делать сортировку только по UserData. По умолчанию
      = FALSE
    - Для ListBox не рисовалось поле (Box), если ItemCount=0
  06.04.2003 SVS
    - "...хоткей '-' не срабатывает в меню "Команды внешних модулей" (F11),
      если набирать на цифровой клавиатуре..."
  17.03.2003 SVS
    - BugZ#814 - DI_LISTBOX высотой 1 не отображается
    ! Подложка для DI_LISTBOX и DI_COMBOBOX рисуется цветом COL_*DIALOGLISTTEXT
    - очередные уточнения позиций
  04.02.2003 SVS
    - BugZ#788 - Криво считается ширина меню.
  30.01.2003 KM
    - Нашёл ещё причину падения фара в поиске. Как выяснилось
      это происходило в SetSelectPos, если SelectPos был равен -1.
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  16.01.2003 KM
    ! Убрал свою правку, чтобы не загромождать код.
      После Валиной онп не имеет смысла (почти).
  13.01.2003 SVS
    - дополнение к двум предыдущим патчам :-)
  11.01.2003 KM
    - 100% падение фара в ShowMenu, скомпилированного VC, из
      диалога поиска при попытке отрисовки DI_LISTBOX.
  03.01.2003 SVS
    - Bug#757 - Несанкционированный доступ к настройкам
      (видимо не только в конструкторе нужно... надо подумать потом!!!)
  16.12.2002 SVS
    - BugZ#729 - некорректная отрисовка в диалоге найденных файлов
  04.11.2002 SVS
    - ListBox некорректно отрисовывается при отрисовки диалога,
      расположенного вплотную к по правому краю экрана
  29.10.2002 SVS
    ! скорректируем SelectPos после сортировки
  22.10.2002 SVS
    ! PrevCursorSize, PrevMacroMode - для пред.курсора
     > у меня сразу после выбора DrawLine из меню F11 пропадает курсор.
     > проявляется после практически любого нажатия на кнопки
  10.10.2002 SVS
    ! Уточнение BugZ#675.
  08.10.2002 SVS
    - BugZ#675 - Неправильно вычисляется ширина меню со списком окон
  04.10.2002 SVS
    ! Уточнение и расширение VMenu::SetColors()
  30.09.2002 SVS
    ! Цветовые истории (Colors не short, а BYTE и применим новую структуру
      FarListColors для SetColors/GetColors)
    ! небольшая оптимизация VMenu::SetColors
    - BugZ#400 - скролбар и проблемы из-за него
  20.09.2002 SVS
    - BugZ#635 - Плавающая ширина Plugin Configuration
    - BugZ#646 - Color groups\Dialog\*List* где они бывают?
  05.09.2002 SVS
    ! Уточнение BugZ#611
  03.09.2002 SVS
    - BugZ#611 - Нелепая сортировка в меню плагинов
    ! функция SortItems имеет доп параметр Offset
    - BugZ#601 - DM_LISTSETCURPOS & TopPos=-1
      Уточнение.
  27.08.2002 SVS
    - BugZ#601 - DM_LISTSETCURPOS & TopPos=-1
  25.06.2002 SVS
    ! Косметика:  BitFlags::Skip -> BitFlags::Clear
  18.06.2002 SVS
    ! В VMenu::Show() учтем факт того момента, что где-то когда то мы залочили
      отрисовку и... соответственно не станем делать эту саму отрисовку.
      Если этого не делать, то настырный манагер все равно отрисует менюху
      (даже если одна в состоянии Hide), при этом будем наблюдать (при выборе
      диска) визуальные противные эффекты (меню появилось, мать его).
      Эта побасенка про LockRefreshCount - в меню выбора дисков при извлечении
      CD-привода мы его (естественно меню :-) гасим и лочим.
  31.05.2002 SVS
    - BugZ#412 - bug в combobox'ах
  24.05.2002 SVS
    + Дублирование Numpad-клавиш
  18.05.2002 SVS
    - BugZ#517 - Пустое меню плагинов (и их конфигов) нельзя закрыть мышкой.
  18.05.2002 SVS
    - При апдейте данных в списке ЗАТИРАЛОСЬ ВСЕ!, потому то и нихрена не
      работало :-( Обновляем только те поля, которые нужно.
  18.05.2002 SVS
    ! MouseDown -> VMENU_MOUSEDOWN
  11.05.2002 SVS
    ! Меняем LIF_UPDATEKEEPUSERDATA на противоположный LIF_DELETEUSERDATA,
      делаем этот флаг доступным (соответственно меняется логика -
      при апдейте итема списка удаляем привязанные данные только в случае,
      если нас об этом попросили)
  28.04.2002 KM
    + GetTypeAndName возвращает тип MODALTYPE_VMENU и
      MODALTYPE_COMBOBOX.
  25.04.2002 SVS
    - BugZ#467 - Дублирующиеся хоткеи в Folders History
  18.04.2002 KM
    - Сбрасываем флаг "координаты установлены" у
      меню. Убивается баг:
      ===
      Новый прикол с DI_COMBOBOX
      3. Открывается (Ctrl-Down) и закрывается COMBOBOX (неважно как с
         выбором или по ESC).
      4. Теперь откравается EDIT (Ctrl-Down).
      5. Наблюдается глюк (открывшийся COMBOBOX).
      ===
  13.04.2002 KM
    - Добавим проверку на существование буфера сохранения,
      т.к. ResizeConsole вызывается теперь для всех экземпляров
      VMenu при AltF9.
  02.04.2002 SVS
    - BugZ#416 - Listbox with DIF_LISTNOBOX does not work correctly
  18.03.2002 SVS
    ! Про курсор
  03.03.2002 SVS
    ! Если для VC вставить ключ /Gr, то видим кучу багов :-/
  27.02.2002 SVS
    + LIF_UPDATEKEEPUSERDATA - "не убивать юзердату при обновлении"
    ! LIFIND_NOPATTERN -> LIFIND_EXACTMATCH
  23.02.2002 DJ
    - не перерисовывалось меню после DM_LISTDELETE (NULL)
    ! Show() также использует флаг VMENU_LISTHASFOCUS
    - после DM_LISTDELETE не ставим текущую позицию на сепаратор
    - VMenu::GetTitle() неправильно возвращало пустой заголовок
    - скорректирован алгоритм обрезания длинных заголовков меню
    - обработка LIF_SELECTED в UpdateItem
    - маскировка внутренних флагов в GetVMenuInfo
  21.02.2002 DJ
    ! корректная отрисовка списков, не имеющих фокуса
    - зависание, если в меню без WRAPMODE последний элемент - сепаратор
    + функция корректировки текущей позиции в меню
  19.02.2002 SVS
    - Забыли про Disable для хоткеев и акселераторов
  18.02.2002 SVS
    - Неверный расчет AmpPos
  13.02.2002 SVS
    ! Немного оптимизации...
    + Один интересный повторяющийся кусок вынесен в CheckKeyHighlighted()
    + MIF_USETEXTPTR - щоб юзать TextPtr
  11.02.2002 SVS
    + Член AccelKey в MenuData и MenuItem
    + BitFlags
    ! у функции UpdateItem() параметр должен быть типа FarListUpdate
    ! Сепаратор может иметь лэйб
  26.12.2001 SVS
    - если все пункты задисаблены, то нефига показывать селектед-пункт
      цветом курсора.
  21.12.2001 SVS
    - не учитывался флаг VMENU_LISTBOX при пересчете координат, из-за чего
      нельзя было спозиционировать список в X1=0
  02.12.2001 KM
    + Поелику VMENU_SHOWAMPERSAND сбрасывается в AssignHighlights
      для корректной работы ShowMenu сделаем сохранение энтого флага
      в переменной VMOldFlags, в противном случае если в диалоге
      использовался DI_LISTBOX без флага DIF_LISTNOAMPERSAND, то
      амперсанды отображались в списке только один раз до следующего
      ShowMenu.
  30.11.2001 DJ
    - не забудем инициализировать MaxLength
  14.11.2001 SVS
    ! Уточнение позиционирования при TopPos=-1 - возможно ошибся
  12.11.2001 SVS
    ! Небольшие уточнения.
  06.11.2001 SVS
    ! VMENU_REVERSIHLIGHT -> VMENU_REVERSEHIGHLIGHT
    ! struct FarListInsert.
         Если Index больше текущего количества элементов списка, то
         вставляемый пункт будет добавлен в конец списка.
         Если Index меньше нуля, то вставляемый пункт будет добавлен
         в начало списка.
  05.11.2001 SVS
    ! немного комментариев-разъяснений по поводу "прикрепленных" данных
  01.11.2001 SVS
    + немного про "типы" - GetType*()
  30.10.2001 SVS
    - Ошибка инженерной мысли :-) в VMenu::FindItem() - забыли передать
      флаги.
  13.10.2001 VVM
    ! Теперь меню не реагирует на отпускание клавиши мышки, если клавиша была нажата не в меню.
  12.10.2001 VVM
    ! Исправление ситуации со скроллбаром в DROPDOWNLIST-е
  10.10.2001 IS
    ! внедрение const
  27.09.2001 IS
    - Левый размер при использовании strncpy
  16.09.2001 SVS
    - BugZ#26: Динамическое изменение ширины списка истории
      (не вычищенные остатки предыдущих испытаний :-)
  12.09.2001 SVS
    - BugZ#10: F6, Ctrl-Down - "курсор" стоит не на верхней строке.
               Так и должно быть? Мне кажется - нет, должен "предлагаться"
               последний вариант из списка.
  09.09.2001 SVS
    ! Очредное уточнение на размер меню (вроде от глюков избавились!)
  05.09.2001 SVS
    ! небольшое уточнение на размер меню (вроде от глюков избавились?)
  14.08.2001 SVS
    ! уточнение пересчета координат при автоцентировании
  07.08.2001 SVS
    - неверно в прошлой серии велся пересчет координат.
  06.08.2001 SVS
    - бага с MaxLength - небыло контроля на длину титла
  01.08.2001 KM
    ! Ещё однго небольшое исправление.
      Добавлен инкремент/декремент CallCount
      в ShowMenu.
  31.07.2001 SVS
    ! Небольшое исправление (с подачи KM)
    ! MACRO_OTHER -> MACRO_MENU
  26.07.2001 OT
    - Поправлен ResizeConsole()
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
  22.07.2001 KM
    ! Исправление неточности перехода по PgUp/PgDn
      с установленным флагом VMENU_SHOWNOBOX (NO_BOX)
  22.07.2001 KM
    - Не устанавливался тип рамки при первом вызове
      ShowMenu с параметром TRUE, что давало неверную
      отрисовку меню.
  21.07.2001 KM
    ! Переработка отрисовки меню с флагом VMENU_SHOWNOBOX.
    ! Переработка обработки мыши в меню с флагом VMENU_SHOWNOBOX.

      Теперь DI_LISTBOX с выставленным флагом DIF_LISTNOBOX и меню
      с флагом VMENU_SHOWNOBOX рисуются без лишних окантовывающих пробелов
      вокруг списка, что автоматически позволяет использовать DI_LISTBOX в
      диалогах, не опасаясь затирания рамки самого диалога пустым местом от
      списка.
  19.07.2001 OT
    - VFMenu - продолжение исправления
  18.07.2001 OT
    + Новый класс VFMenu
  11.07.2001 SVS
    + Shift-F1 на равне с F1 может вызывать хелп. Это на тот случай, если мы
      в качестве хоткея назначили F1 (естественно хелп в такой ситуации не
      будет вызываться)
  30.06.2001 KM
    ! Языковое уточненение: LIFIND_NOPATTER -> LIFIND_NOPATTERN
    + GetSelectPos(struct FarListPos *)
    + SetSelectPos(struct FarListPos *)
    ! Небольшое изменение в функции UpdateRequired: теперь она возвращает
      TRUE и при условии, что выставлен флаг VMENU_UPDATEREQUIRED
  29.06.2001 SVS
    + Новый параметр у FindItem - флаги
    + LIFIND_NOPATTER - точное (без учета регистра букв) соответствие при
      поиске в списке
  25.06.2001 IS
    ! Внедрение const
  14.06.2001 SVS
    - Установка позиции всегда приводит к выбору 0 итема.
  13.06.2001 SVS
    - приведение типов.
  12.06.2001 KM
    - Некорректно работала функция GetUserData. Забыли, что данные
      в меню могут храниться не в UserData, а просто в Name, из-за
      чего не работал выбор из комбобокса (выбирался мусор).
  10.06.2001 SVS
    + FindItem с двумя параметрами - для будущего ручного финдера в списке.
    - не работал поиск, т.к. присутствовали символы '&'
  05.06.2001 KM
    - Избавление от маленькой дырочки в Set*Title.
  04.06.2001 SVS
    - Злостная бага в DeleteItem :-(
  04.06.2001 SVS
    ! "Фигня с пацанами вышла" - это про патч 706. В общем уточнения функций.
  03.06.2001 KM
    + Функции SetTitle, GetTitle, GetBottomTitle.
    ! Вернём DI_LISTBOX'у возможность задавать заголовок.
    ! Убрана дефолтная установка флага VMENU_WRAPMODE, в противном
      случае при создании меню прокрутка работала _ВСЕГДА_, что
      не всегда удобно.
  03.06.2001 SVS
    ! Изменения в связи с переделкой UserData в VMenu
    + GetPosition() - возвращает реальную позицию итема.
    + GetUserDataSize() - получить размер данных
    + SetUserData() - присовокупить данные к пункту меню
    ! GetUserData() - возвращает указатель на сами данные
  30.05.2001 OT
    - Проблемы с отрисовкой VMenu. В новом члене Frame *FrameFromLaunched
      запоминается тот фрейм, откуда это меню запускалось.
      Чтобы потом он не перерисовавался, когда его не просят :)
  25.05.2001 DJ
    + SetColor()
  23.05.2001 SVS
    - Проблемы с горячими клавишами в меню (Part II) - ни тебе инициализации
      AmpPos при добавлении пунктов, да еще и разницу между имененм и
      позицией... Срань.
  23.05.2001 SVS
    - Проблемы с горячими клавишами в меню.
  22.05.2001 SVS
    ! Не трогаем оригинал на предмет вставки '&', все делаем только при
      прорисовке.
  21.05.2001 DJ
    - Забыли вернуть значение в VMenu::ChangeFlags()
  21.05.2001 SVS
    ! VMENU_DRAWBACKGROUND -> VMENU_DISABLEDRAWBACKGROUND
    ! MENU_* выкинуты
    ! DialogStyle -> VMENU_WARNDIALOG
    ! struct MenuData
      Поля Selected, Checked и Separator преобразованы в DWORD Flags
    ! struct MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
  18.05.2001 SVS
    ! UpdateRequired -> VMENU_UPDATEREQUIRED
    ! DrawBackground -> VMENU_DRAWBACKGROUND
    ! WrapMode -> VMENU_WRAPMODE
    ! ShowAmpersand -> VMENU_SHOWAMPERSAND
    + Функции InsertItem(), FindItem(), UpdateRequired(), GetVMenuInfo()
  17.05.2001 SVS
    + UpdateItem() - обновить пункт
    + FarList2MenuItem() \ функции преобразования "туда-сюда"
    + MenuItem2FarList() /
    ! табуляции меняем только при показе - для сохранение оригинальной строки
    ! При автоназначении учтем факт того, что должны вернуть оригинал не тронутым
  15.05.2001 KM
    - Не работал флаг DIF_CHECKED в DI_LISTBOX
    + Добавлена возможность назначать в DI_LISTBOX с флагом DIF_CHECKED
      произвольный символ (в младшем слове Flags), который будет
      использоваться в качестве Check mark, следующим образом:
      MacroMenuItems[i].Flags|=(S[0]=='~')?LIF_CHECKED|'-':0;
  12.05.2001 SVS
    + AddItem(char *NewStrItem);
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
    + AddItem, отличается тем, что параметр типа struct FarList - для
      сокращения кода в диалогах :-)
    + SortItems() - опять же - для диалогов
    * Изменен тип возвращаемого значения для GetItemPtr() и убран первый
      параметр функции - Item
  06.05.2001 DJ
    ! перетрях #include
  27.04.2001 VVM
    + Обработка KEY_MSWHEEL_XXXX
    + В меню нажатие средней кнопки аналогично нажатию ЕНТЕР
  09.04.2001 SVS
    ! Избавимся от некоторых варнингов
  20.02.2001 SVS
    + Добавлена функция SetSelectPos() - переместить курсор с учетом
      Disabled & Separator
    ! Изменения в клавишнике и "мышнике" :-) с учетом введения SetSelectPos()
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
    ! Оптимизирован механизм отображения сепаратора - сначала формируем в
      памяти, потом выводим в виртуальный буфер
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  11.12.2000 tran
    + прокрутка мышью не должна врапить меню
  20.09.2000 SVS
    + Функция GetItemPtr - получить указатель на нужный Item.
  29.08.2000 tran 1.09
    - BUG с не записью \0 в конец строки в GetUserData
  01.08.2000 SVS
    + В ShowMenu добавлен параметр, сообщающий - вызвали ли функцию
      самостоятельно или из другой функции ;-)
    - Bug в конструкторе, если передали NULL для Title
    ! ListBoxControl -> VMFlags
    + функция удаления N пунктов меню
    + функция обработки меню (по умолчанию)
    + функция посылки сообщений меню
    ! Изменен вызов конструктора для указания функции-обработчика и родителя!
  28.07.2000 SVS
    + Добавлены цветовые атрибуты (в переменных) и функции, связанные с
      атрибутами:
      SetColors();
      GetColors();
  23.07.2000 SVS
    + Куча рамарок в исходниках :-)
    ! AlwaysScrollBar изменен на ListBoxControl
    ! Тень рисуется только для меню, для ListBoxControl она ненужна
  18.07.2000 SVS
    ! изменен вызов конструктора (пареметр isAlwaysScrollBar) с учетом
      необходимости scrollbar в DI_COMBOBOX (и в будущем - DI_LISTBOX)
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  06.07.2000 tran
    + mouse support for menu scrollbar
  29.06.2000 SVS
    ! Показывать ScrollBar в меню если включена опция ShowMenuScrollbar
  28.06.2000 tran
    + вертикальный скролбар в меню при необходимости
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "vmenu.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "macroopcode.hpp"
#include "colors.hpp"
#include "chgprior.hpp"
#include "dialog.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "constitle.hpp"

/* $ 18.07.2000 SVS
   ! изменен вызов конструктора (isListBoxControl) с учетом необходимости
     scrollbar в DI_LISTBOX & DI_COMBOBOX
*/
VMenu::VMenu(const char *Title,       // заголовок меню
             struct MenuData *Data, // пункты меню
             int ItemCount,     // количество пунктов меню
             int MaxHeight,     // максимальная высота
             DWORD Flags,       // нужен ScrollBar?
             FARWINDOWPROC Proc,    // обработчик
             Dialog *ParentDialog)  // родитель для ListBox
{
  int I;
  SetDynamicallyBorn(false);

  VMenu::VMFlags.Set(Flags);
  VMenu::VMFlags.Clear(VMENU_MOUSEDOWN);
/* SVS $ */

/*& 28.05.2001 OT Запретить перерисовку фрема во время запуска меню */
//  FrameFromLaunched=FrameManager->GetCurrentFrame();
//  FrameFromLaunched->LockRefresh();
/* OT &*/

  VMenu::ParentDialog=ParentDialog;
  /* $ 03.06.2001 KM
     ! Убрана дефолтная установка флага VMENU_WRAPMODE, в противном
       случае при создании меню прокрутка работала _ВСЕГДА_, что
       не всегда удобно.
  */
  VMFlags.Set(VMENU_UPDATEREQUIRED);
  /* KM $ */
  VMFlags.Clear(VMENU_SHOWAMPERSAND);
  TopPos=0;
  SaveScr=NULL;
  OldTitle=NULL;

  GetCursorType(PrevCursorVisible,PrevCursorSize);

  if(!Proc) // функция должна быть всегда!!!
    Proc=(FARWINDOWPROC)VMenu::DefMenuProc;
  VMenuProc=Proc;

  if (Title!=NULL)
    xstrncpy(VMenu::Title,Title, sizeof(VMenu::Title)-1);
  else
    *VMenu::Title=0;

  *BottomTitle=0;
  VMenu::Item=NULL;
  VMenu::ItemCount=0;

  VMenu::LastAddedItem = 0;

  /* $ 01.08.2000 SVS
   - Bug в конструкторе, если передали NULL для Title
  */
  /* $ 30.11.2001 DJ
     инициализируем перед тем, как добавлять айтема
  */
  MaxLength=strlen(VMenu::Title)+2;
  /* DJ $ */
  /* SVS $ */

  RLen[0]=RLen[1]=0; // реальные размеры 2-х половин

  struct MenuItem NewItem;
  for (I=0; I < ItemCount; I++)
  {
    memset(&NewItem,0,sizeof(NewItem));
    if ((unsigned int)Data[I].Name < MAX_MSG)
      xstrncpy(NewItem.Name,MSG((unsigned int)Data[I].Name),sizeof(NewItem.Name)+1);
    else
      xstrncpy(NewItem.Name,Data[I].Name,sizeof(NewItem.Name)+1);
    //NewItem.AmpPos=-1;
    NewItem.AccelKey=Data[I].AccelKey;
    NewItem.Flags=Data[I].Flags;
    AddItem(&NewItem);
  }

  BoxType=DOUBLE_BOX;
  for (SelectPos=-1,I=0;I<ItemCount;I++)
  {
    int Length=strlen(Item[I].Name);
    if (Length>MaxLength)
      MaxLength=Length;
    if ((Item[I].Flags&LIF_SELECTED) && !(Item[I].Flags&LIF_DISABLE))
      SelectPos=I;
  }

  VMFlags.Clear(VMENU_SELECTPOSNONE);
  if(SelectPos < 0)
    SelectPos=SetSelectPos(0,1);
  if(SelectPos < 0)
  {
    VMFlags.Set(VMENU_SELECTPOSNONE);
    SelectPos=0;
  }

  SetMaxHeight(MaxHeight);
  /* $ 28.07.2000 SVS
     Установим цвет по умолчанию
  */
  SetColors(NULL);
  /* SVS $*/
  if (!VMFlags.Check(VMENU_LISTBOX) && CtrlObject!=NULL)
  {
    PrevMacroMode=CtrlObject->Macro.GetMode();
    if (PrevMacroMode!=MACRO_MAINMENU &&
        PrevMacroMode!=MACRO_DIALOG)
      CtrlObject->Macro.SetMode(MACRO_MENU);
  }

  if (!VMFlags.Check(VMENU_LISTBOX))
    FrameManager->ModalizeFrame(this);
}


VMenu::~VMenu()
{
  if (!VMFlags.Check(VMENU_LISTBOX) && CtrlObject!=NULL)
    CtrlObject->Macro.SetMode(PrevMacroMode);
  Hide();
  DeleteItems();
/*& 28.05.2001 OT Разрешить перерисовку фрейма, в котором создавалось это меню */
//  FrameFromLaunched->UnlockRefresh();
/* OT &*/
  SetCursorType(PrevCursorVisible,PrevCursorSize);

  if (!VMFlags.Check(VMENU_LISTBOX))
  {
    FrameManager->UnmodalizeFrame(this);
    FrameManager->RefreshFrame();
  }
}

void VMenu::Hide()
{
  CriticalSectionLock Lock(CS);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  if(!VMFlags.Check(VMENU_LISTBOX) && SaveScr)
  {
    delete SaveScr;
    SaveScr=NULL;
    ScreenObject::Hide();
  }

  Y2=-1;
//  X2=-1;

  VMFlags.Set(VMENU_UPDATEREQUIRED);
  if(OldTitle)
    delete OldTitle;
}


void VMenu::Show()
{
  CriticalSectionLock Lock(CS);

  int OldX1 = X1, OldY1 = Y1, OldX2 = X2, OldY2 = Y2;

  if(LockRefreshCount)
    return;

  /* $ 23.02.2002 DJ
     здесь тоже используем флаг LISTHASFOCUS
  */
  if(VMFlags.Check(VMENU_LISTBOX))
  {
    if (VMFlags.Check(VMENU_SHOWNOBOX))
      BoxType=NO_BOX;
    else if (VMFlags.Check (VMENU_LISTHASFOCUS))
      BoxType=SHORT_DOUBLE_BOX;
    else
      BoxType=SHORT_SINGLE_BOX;
  }
  /* DJ $ */

  int AutoCenter=FALSE,AutoHeight=FALSE;

  if (X1==-1)
  {
    X1=(ScrX-MaxLength-4)/2;
    AutoCenter=TRUE;
  }
  if(VMFlags.Check(VMENU_LISTBOX))
  {
    if(X1<0)
      X1=0;
    if (X2<=0)
      X2=X1+MaxLength+4;
  }
  else
  {
    if(X1<2)
      X1=2;
    if (X2<=0)
      X2=X1+MaxLength+4;
  }
  if (!AutoCenter && X2 > ScrX-(VMFlags.Check(VMENU_LISTBOX)?2:4)+2*(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
  {
    X1+=ScrX-4-X2;
    X2=ScrX-4;
    if (X1<2)
    {
      X1=2;
      X2=ScrX-2;
    }
  }
  if (!VMFlags.Check(VMENU_LISTBOX) && X2>ScrX-2)
    X2=ScrX-2;

  if (Y1==-1)
  {
    if (MaxHeight!=0 && MaxHeight<ItemCount)
      Y1=(ScrY-MaxHeight-2)/2;
    else
      if ((Y1=(ScrY-ItemCount-2)/2)<0)
        Y1=0;
    AutoHeight=TRUE;
  }
  if (Y2<=0)
    if (MaxHeight!=0 && MaxHeight<ItemCount)
      Y2=Y1+MaxHeight+1;
    else
      Y2=Y1+ItemCount+1;
  if (Y2>ScrY)
    Y2=ScrY;
  if (AutoHeight && Y1<3 && Y2>ScrY-3)
  {
    Y1=2;
    Y2=ScrY-2;
  }
  if (X2>X1 && Y2+(VMFlags.Check(VMENU_SHOWNOBOX)?1:0)>Y1)
  {
  /*  if ( (OldX1 != X1) ||
         (OldY1 != Y1) ||
         (OldX2 != X2) ||
         (OldY2 != Y2) )
       VMFlags.Clear (VMENU_DISABLEDRAWBACKGROUND);*/

    if(SelectPos == -1)
      SelectPos=SetSelectPos(0,1);
//_SVS(SysLog("VMenu::Show()"));
    if(!VMFlags.Check(VMENU_LISTBOX))
      ScreenObject::Show();
//      Show();
    else
    {
      VMFlags.Set(VMENU_UPDATEREQUIRED);
      DisplayObject();
    }
  }
}

/* $ 28.07.2000 SVS
   Переработка функции с учетом VMenu::Colors[] -
      заменены константы на VMenu::Colors[]
*/
void VMenu::DisplayObject()
{
  CriticalSectionLock Lock(CS);
//_SVS(SysLog("VMFlags&VMENU_UPDATEREQUIRED=%d",VMFlags.Check(VMENU_UPDATEREQUIRED)));
//  if (!(VMFlags&VMENU_UPDATEREQUIRED))
//    return;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  VMFlags.Clear(VMENU_UPDATEREQUIRED);
  Modal::ExitCode=-1;

  SetCursorType(0,10);

  if (!VMFlags.Check(VMENU_LISTBOX) && SaveScr==NULL)
  {
    if (!VMFlags.Check(VMENU_DISABLEDRAWBACKGROUND) && !(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
      SaveScr=new SaveScreen(X1-2,Y1-1,X2+4,Y2+2);
    else
      SaveScr=new SaveScreen(X1,Y1,X2+2,Y2+1);
  }
  if (!VMFlags.Check(VMENU_DISABLEDRAWBACKGROUND))
  {
    /* $ 23.07.2000 SVS
       Тень для ListBox ненужна
    */
    if (BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX)
    {
      Box(X1,Y1,X2,Y2,VMenu::Colors[VMenuColorBox],BoxType);
      if(!VMFlags.Check(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR))
      {
        MakeShadow(X1+2,Y2+1,X2+1,Y2+1);
        MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
      }
    }
    else
    {
      /* $ 21.07.2001 KM
       ! Переработка отрисовки меню с флагом VMENU_SHOWNOBOX.
      */
      if (BoxType!=NO_BOX)
        SetScreen(X1-2,Y1-1,X2+2,Y2+1,' ',VMenu::Colors[VMenuColorBody]);
      else
        SetScreen(X1,Y1,X2,Y2,' ',VMenu::Colors[VMenuColorBody]);
      /* KM $ */
      if(!VMFlags.Check(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR))
      {
        MakeShadow(X1,Y2+2,X2+3,Y2+2);
        MakeShadow(X2+3,Y1,X2+4,Y2+2);
      }
      if (BoxType!=NO_BOX)
        Box(X1,Y1,X2,Y2,VMenu::Colors[VMenuColorBox],BoxType);
    }
    /* SVS $*/

//    VMFlags.Set (VMENU_DISABLEDRAWBACKGROUND);
  }
  /* $ 03.06.2001 KM
     ! Вернём DI_LISTBOX'у возможность задавать заголовок.
  */
  /* $ 23.02.2002 DJ
     обрезаем длину заголовка не по длине заголовка, а по реальной ширине меню!
  */
  if(!VMFlags.Check(VMENU_LISTBOX))
    DrawTitles();
  /* DJ $ */
  /* KM $ */
  ShowMenu(TRUE);
}
/* SVS $ */

void VMenu::DrawTitles()
{
  CriticalSectionLock Lock(CS);

  int MaxTitleLength = X2-X1-2;
  int WidthTitle;
  if (*Title)
  {
    if((WidthTitle=strlen(Title)) > MaxTitleLength)
      WidthTitle=MaxTitleLength-1;
    GotoXY(X1+(X2-X1-1-WidthTitle)/2,Y1);
    SetColor(VMenu::Colors[VMenuColorTitle]);
    mprintf(" %*.*s ",WidthTitle,WidthTitle,Title);
  }
  if (*BottomTitle)
  {
    if((WidthTitle=strlen(BottomTitle)) > MaxTitleLength)
      WidthTitle=MaxTitleLength-1;
    GotoXY(X1+(X2-X1-1-WidthTitle)/2,Y2);
    SetColor(VMenu::Colors[VMenuColorTitle]);
    mprintf(" %*.*s ",WidthTitle,WidthTitle,BottomTitle);
  }
}

/* $ 28.07.2000 SVS
   Переработка функции с учетом VMenu::Colors[] -
      заменены константы на VMenu::Colors[]
*/
void VMenu::ShowMenu(int IsParent)
{
  CriticalSectionLock Lock(CS);

  char TmpStr[1024];
  unsigned char BoxChar[2],BoxChar2[2];
  int Y,I;
  /* $ 23.02.2002 DJ
     если в меню нету пунктов - это не значит, что не надо рисовать фон!
  */
  if (/*ItemCount==0 ||*/ X2<=X1 || Y2<=Y1)  /* DJ $ */
  {
    if(!(VMFlags.Check(VMENU_SHOWNOBOX) && Y2==Y1))
      return;
  }
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  BoxChar2[1]=BoxChar[1]=0;

  /* $ 11.01.2003 KM
     ! Иногда TopPos=-1, что в дальнейшем приводило к отрицательному
       индексу и, естественно, приводило к exception.
  */
  if (TopPos<0)
      TopPos=0;
  /* KM $ */

  /* $ 22.07.2001 KM
   - Не устанавливался тип рамки при первом вызове
     ShowMenu с параметром TRUE, что давало неверную
     отрисовку меню.
  */
  /* $ 21.02.2002 DJ
     а для списков, не имеющих фокуса, нужна одиночная рамка!
  */
  if (VMFlags.Check(VMENU_LISTBOX))
  {
    if (VMFlags.Check(VMENU_SHOWNOBOX))
      BoxType = NO_BOX;
    else if (VMFlags.Check (VMENU_LISTHASFOCUS))
      BoxType = SHORT_DOUBLE_BOX;
    else
      BoxType = SHORT_SINGLE_BOX;
  }
  /* DJ $ */
  /* KM $ */
  if(VMFlags.Check(VMENU_LISTBOX))
  {
    if((!IsParent || !ItemCount))
    {
      if(ItemCount)
        BoxType=VMFlags.Check(VMENU_SHOWNOBOX)?NO_BOX:SHORT_SINGLE_BOX;
      SetScreen(X1,Y1,X2,Y2,' ',VMenu::Colors[VMenuColorBody]);
    }
    if (BoxType!=NO_BOX)
      Box(X1,Y1,X2,Y2,VMenu::Colors[VMenuColorBox],BoxType);
    DrawTitles();
  }

  switch(BoxType)
  {
    case NO_BOX:
      *BoxChar=' ';
      break;
    case SINGLE_BOX:
    case SHORT_SINGLE_BOX:
      *BoxChar=0x0B3; // |
      break;
    case DOUBLE_BOX:
    case SHORT_DOUBLE_BOX:
      *BoxChar=0x0BA; // ||
      break;
  }

  if (ItemCount <= 0)
    return;

  if (SelectPos<ItemCount)
  {
    if(Item[SelectPos].Flags&LIF_DISABLE)
      Item[SelectPos].Flags&=~LIF_SELECTED;
    else
      Item[SelectPos].Flags|=LIF_SELECTED;
  }

  /* $ 02.12.2001 KM
     ! Предварительно, если нужно, настроим "горячие" клавиши.
  */
  if(VMFlags.Check(VMENU_AUTOHIGHLIGHT|VMENU_REVERSEHIGHLIGHT))
    AssignHighlights(VMFlags.Check(VMENU_REVERSEHIGHLIGHT));
  /* KM $ */

  /* $ 21.07.2001 KM
   ! Переработка отрисовки меню с флагом VMENU_SHOWNOBOX.
  */
  if (SelectPos>TopPos+((BoxType!=NO_BOX)?Y2-Y1-2:Y2-Y1))
    TopPos=SelectPos-((BoxType!=NO_BOX)?Y2-Y1-2:Y2-Y1);
  if (SelectPos<TopPos)
    TopPos=SelectPos;
  if(TopPos<0)
    TopPos=0;

  for (Y=Y1+((BoxType!=NO_BOX)?1:0),I=TopPos;Y<((BoxType!=NO_BOX)?Y2:Y2+1);Y++,I++)
  /* KM $ */
  {
    GotoXY(X1,Y);
    if (I<ItemCount)
    {
      if (Item[I].Flags&LIF_SEPARATOR)
      {
        int SepWidth=X2-X1+1;
        char *Ptr=TmpStr+1;
        MakeSeparator(SepWidth,TmpStr,
          BoxType==NO_BOX?0:(BoxType==SINGLE_BOX||BoxType==SHORT_SINGLE_BOX?2:1));

        if (I>0 && I<ItemCount-1 && SepWidth>3)
          for (unsigned int J=0;Ptr[J+3]!=0;J++)
          {
            if (Item[I-1][J]==0)
              break;
            if (Item[I-1][J]==0x0B3)
            {
              int Correction=0;
              if (!VMFlags.Check(VMENU_SHOWAMPERSAND) && memchr(Item[I-1].PtrName(),'&',J)!=NULL)
                Correction=1;
              if (strlen(Item[I+1].PtrName())>=J && Item[I+1][J]==0x0B3)
                Ptr[J-Correction+2]=0x0C5;
              else
                Ptr[J-Correction+2]=0x0C1;
            }
          }
        //Text(X1,Y,VMenu::Colors[VMenuColorSeparator],TmpStr); // VMenuColorBox
        SetColor(VMenu::Colors[VMenuColorSeparator]);
#if defined(USE_WFUNC)
        if(Opt.UseUnicodeConsole)
          BoxTextW2(TmpStr,FALSE);
        else
#endif
          BoxText(TmpStr,FALSE);

        if (*Item[I].PtrName())
        {
          int ItemWidth=strlen(Item[I].PtrName());
          GotoXY(X1+(X2-X1-1-ItemWidth)/2,Y);
          mprintf(" %*.*s ",ItemWidth,ItemWidth,Item[I].PtrName());
        }
      }
      else
      {
        /* $ 21.07.2001 KM
         ! Переработка отрисовки меню с флагом VMENU_SHOWNOBOX.
        */
        if (BoxType!=NO_BOX)
        {
          SetColor(VMenu::Colors[VMenuColorBox]);
          BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[*BoxChar-0x0B0]:*BoxChar)); //Text((char*)BoxChar);
          GotoXY(X2,Y);
          BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[*BoxChar-0x0B0]:*BoxChar)); //Text((char*)BoxChar);
        }
        if ((Item[I].Flags&LIF_SELECTED) && !(Item[I].Flags&LIF_DISABLE))
          SetColor(VMenu::Colors[VMenuColorSelected]);
        else
          SetColor(VMenu::Colors[(Item[I].Flags&LIF_DISABLE?9:3)]);
        if (BoxType!=NO_BOX)
          GotoXY(X1+1,Y);
        else
          GotoXY(X1,Y);
        /* KM $ */
        char Check=' ';
        if (Item[I].Flags&LIF_CHECKED)
          if (!(Item[I].Flags&0x0000FFFF))
            Check=0x0FB;
          else
            Check=(char)Item[I].Flags&0x0000FFFF;

        if(HiStrlen(Item[I].PtrName(),TRUE) > X2-X1-3)
          sprintf(TmpStr,"%c %.*s",Check,X2-X1-3,Item[I].PtrName());
        else
          sprintf(TmpStr,"%c %s",Check,Item[I].PtrName());
        { // табуляции меняем только при показе!!!
          // для сохранение оригинальной строки!!!
          char *TabPtr;
          while ((TabPtr=strchr(TmpStr,'\t'))!=NULL)
            *TabPtr=' ';
        }
        int Col;

        if(!(Item[I].Flags&LIF_DISABLE))
        {
          if (Item[I].Flags&LIF_SELECTED)
              Col=VMenu::Colors[VMenuColorHSelect];
          else
              Col=VMenu::Colors[VMenuColorHilite];
        }
        else
          Col=VMenu::Colors[VMenuColorDisabled];
        if(VMFlags.Check(VMENU_SHOWAMPERSAND))
          Text(TmpStr);
        else
        {
          short AmpPos=Item[I].AmpPos+2;
//_SVS(SysLog(">>> AmpPos=%d (%d) TmpStr='%s'",AmpPos,Item[I].AmpPos,TmpStr));
          if(AmpPos >= 2 && AmpPos < sizeof(TmpStr) && TmpStr[AmpPos] != '&')
          {
            memmove(TmpStr+AmpPos+1,TmpStr+AmpPos,strlen(TmpStr+AmpPos)+1);
            TmpStr[AmpPos]='&';
          }
//_SVS(SysLog("<<< AmpPos=%d TmpStr='%s'",AmpPos,TmpStr));
          HiText(TmpStr,Col);
        }
        // сделаем добавочку для NO_BOX
        mprintf("%*s",X2-WhereX()+(BoxType==NO_BOX?1:0),"");
      }
    }
    else
    {
      /* $ 21.07.2001 KM
       ! Переработка отрисовки меню с флагом VMENU_SHOWNOBOX.
      */
      if (BoxType!=NO_BOX)
      {
        SetColor(VMenu::Colors[VMenuColorBox]);
        BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[*BoxChar-0x0B0]:*BoxChar)); //Text((char*)BoxChar);
        GotoXY(X2,Y);
        BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[*BoxChar-0x0B0]:*BoxChar)); //Text((char*)BoxChar);
        GotoXY(X1+1,Y);
      }
      else
        GotoXY(X1,Y);
      SetColor(VMenu::Colors[VMenuColorText]);
                                                     // сделаем добавочку для NO_BOX
      mprintf("%*s",((BoxType!=NO_BOX)?X2-X1-1:X2-X1)+(BoxType==NO_BOX?1:0),"");
      /* KM $ */
    }
  }
  /* $ 28.06.2000 tran
       показываем скролбар если пунктов в меню больше чем
       его высота
     $ 29.06.2000 SVS
       Показывать ScrollBar в меню если включена опция Opt.ShowMenuScrollbar
     $ 18.07.2000 SVS
       + всегда покажет scrollbar для DI_LISTBOX & DI_COMBOBOX и опционально
         для вертикального меню
  */
  /* $ 21.07.2001 KM
   ! Переработка отрисовки меню с флагом VMENU_SHOWNOBOX.
  */
  if (VMFlags.Check(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Opt.ShowMenuScrollbar)
  {
    if (((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1+1)<ItemCount)
    {
      SetColor(VMenu::Colors[VMenuColorScrollBar]);
      if (BoxType!=NO_BOX)
        ScrollBar(X2,Y1+1,Y2-Y1-1,SelectPos,ItemCount);
      else
        ScrollBar(X2,Y1,Y2-Y1+1,SelectPos,ItemCount);
    }
  }
}
/* 28.07.2000 SVS $ */

BOOL VMenu::UpdateRequired(void)
{
  CriticalSectionLock Lock(CS);

  return ((LastAddedItem>=TopPos && LastAddedItem<(TopPos+Y2-Y1)+2) || VMFlags.Check(VMENU_UPDATEREQUIRED));

  //+1 for real diff
  //+2 for scrollbar
}

BOOL VMenu::CheckKeyHiOrAcc(DWORD Key,int Type,int Translate)
{
  CriticalSectionLock Lock(CS);

  int I;
  struct MenuItem *CurItem;

  if(VMFlags.Check(VMENU_LISTBOX)) // не забудем сбросить EndLoop для листбокса,
    EndLoop=FALSE;                 // иначе не будут работать хоткеи в активном списке

  for (CurItem=Item,I=0; I < ItemCount; I++, ++CurItem)
  {
    if(!(CurItem->Flags&LIF_DISABLE) &&
       (
         (!Type && CurItem->AccelKey && Key == CurItem->AccelKey) ||
         (Type && Dialog::IsKeyHighlighted(CurItem->PtrName(),Key,Translate,CurItem->AmpPos))
       )
      )
    {
      Item[SelectPos].Flags&=~LIF_SELECTED;
      CurItem->Flags|=LIF_SELECTED;
      SelectPos=I;
      ShowMenu(TRUE);
      if(!VMenu::ParentDialog)
      {
        Modal::ExitCode=I;
        EndLoop=TRUE;
      }
      break;
    }
  }
  return EndLoop==TRUE;
}

int VMenu::ProcessKey(int Key)
{
  CriticalSectionLock Lock(CS);

  int I;

  if (Key==KEY_NONE || Key==KEY_IDLE)
    return(FALSE);

  switch(Key)
  {
    case MCODE_OP_PLAINTEXT:
    {
      const char *str = eStackAsString();
      if (!*str)
        return FALSE;
      Key=*str;
      break;
    }
    case MCODE_C_EMPTY:
      return ItemCount<=0;
    case MCODE_C_EOF:
      return SelectPos==ItemCount-1;
    case MCODE_C_BOF:
      return SelectPos==0;
    case MCODE_C_SELECTED:
      return ItemCount > 0 && SelectPos >= 0;
    case MCODE_V_ITEMCOUNT:
      return ItemCount;
    case MCODE_V_CURPOS:
      return SelectPos+1;
    case MCODE_F_MENU_CHECKHOTKEY:
    {
      const char *str = eStackAsString(1);
      if ( *str )
        return CheckHighlights(*str);
      return FALSE;
    }
  }

  VMFlags.Set(VMENU_UPDATEREQUIRED);
  if (ItemCount==0)
    if (Key!=KEY_F1 && Key!=KEY_SHIFTF1 && Key!=KEY_F10 && Key!=KEY_ESC)
    {
      Modal::ExitCode=-1;
      return(FALSE);
    }

  if(!(Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE))
  {
    DWORD S=Key&(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT);
    DWORD K=Key&(~(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT));

    if (K==KEY_MULTIPLY)
      Key='*'|S;
    else if (K==KEY_ADD)
      Key='+'|S;
    else if (K==KEY_SUBTRACT)
      Key='-'|S;
    else if (K==KEY_DIVIDE)
      Key='/'|S;
  }

  switch(Key)
  {
    case KEY_ENTER:
    {
      if(!VMenu::ParentDialog)
      {
        if(!(Item[SelectPos].Flags&LIF_DISABLE))
        {
          EndLoop=TRUE;
          Modal::ExitCode=SelectPos;
        }
      }
      break;
    }

    case KEY_ESC:
    case KEY_F10:
    {
      if(!VMenu::ParentDialog)
      {
        EndLoop=TRUE;
        Modal::ExitCode=-1;
      }
      break;
    }

    case KEY_HOME:         case KEY_NUMPAD7:
    case KEY_CTRLHOME:     case KEY_CTRLNUMPAD7:
    case KEY_CTRLPGUP:     case KEY_CTRLNUMPAD9:
    {
      SelectPos=SetSelectPos(0,1);
      ShowMenu(TRUE);
      break;
    }

    case KEY_END:          case KEY_NUMPAD1:
    case KEY_CTRLEND:      case KEY_CTRLNUMPAD1:
    case KEY_CTRLPGDN:     case KEY_CTRLNUMPAD3:
    {
      SelectPos=SetSelectPos(ItemCount-1,-1);
      ShowMenu(TRUE);
      break;
    }

    case KEY_PGUP:         case KEY_NUMPAD9:
    {
      /* $ 22.07.2001 KM
       ! Исправление неточности перехода по PgUp/PgDn
         с установленным флагом VMENU_SHOWNOBOX (NO_BOX)
      */
      if((I=SelectPos-((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1)) < 0)
        I=0;
      SelectPos=SetSelectPos(I,1);
      ShowMenu(TRUE);
      break;
    }

    case KEY_PGDN:         case KEY_NUMPAD3:
    {
      if((I=SelectPos+((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1)) >= ItemCount)
        I=ItemCount-1;
      SelectPos=SetSelectPos(I,-1);
      ShowMenu(TRUE);
      break;
    }

      /* KM $ */
    /* $ 27.04.2001 VVM
      + Обработка KEY_MSWHEEL_XXXX */
    case KEY_MSWHEEL_UP:
    /* VVM $ */
    case KEY_LEFT:         case KEY_NUMPAD4:
    case KEY_UP:           case KEY_NUMPAD8:
    {
      SelectPos=SetSelectPos(SelectPos-1,-1);
      ShowMenu(TRUE);
      break;
    }

    /* $ 27.04.2001 VVM
      + Обработка KEY_MSWHEEL_XXXX */
    case KEY_MSWHEEL_DOWN:
    /* VVM $ */
    case KEY_RIGHT:        case KEY_NUMPAD6:
    case KEY_DOWN:         case KEY_NUMPAD2:
    {
      SelectPos=SetSelectPos(SelectPos+1,1);
      ShowMenu(TRUE);
      break;
    }

    case KEY_TAB:
    case KEY_SHIFTTAB:
    default:
    {
      int OldSelectPos=SelectPos;

      if(!CheckKeyHiOrAcc(Key,0,0))
      {
        if(Key == KEY_SHIFTF1 || Key == KEY_F1)
        {
          if(VMenu::ParentDialog)
            ;//VMenu::ParentDialog->ProcessKey(Key);
          else
            ShowHelp();
          break;
        }
        else
        {
          if(!CheckKeyHiOrAcc(Key,1,FALSE))
            CheckKeyHiOrAcc(Key,1,TRUE);
        }
      }

      if(VMenu::ParentDialog && OldSelectPos!=SelectPos && Dialog::SendDlgMessage((HANDLE)ParentDialog,DN_LISTHOTKEY,DialogItemID,SelectPos))
      {
        Item[SelectPos].Flags&=~LIF_SELECTED;
        Item[OldSelectPos].Flags|=LIF_SELECTED;
        SelectPos=OldSelectPos;
        ShowMenu(TRUE);
      }

      return(FALSE);
    }
  }
  return(TRUE);
}

int VMenu::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  CriticalSectionLock Lock(CS);

  int MsPos,MsX,MsY;
  int XX2;

  VMFlags.Set(VMENU_UPDATEREQUIRED);
  if (ItemCount==0)
  {
    if(MouseEvent->dwButtonState && MouseEvent->dwEventFlags==0)
      EndLoop=TRUE;
    Modal::ExitCode=-1;
    return(FALSE);
  }

  /* $ 27.04.2001 VVM
    + Считать нажатие средней кнопки за ЕНТЕР */
  if (MouseEvent->dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
  {
    ProcessKey(KEY_ENTER);
    return(TRUE);
  }
  /* VVM $ */

  MsX=MouseEvent->dwMousePosition.X;
  MsY=MouseEvent->dwMousePosition.Y;
  /* $ 06.07.2000 tran
     + mouse support for menu scrollbar
  */

  /* $ 21.07.2001 KM
   ! Переработка обработки мыши в меню с флагом VMENU_SHOWNOBOX.
  */
  int SbY1=((BoxType!=NO_BOX)?Y1+1:Y1), SbY2=((BoxType!=NO_BOX)?Y2-1:Y2);

  XX2=X2;

  /* $ 12.10.2001 VVM
    ! Есть ли у нас скроллбар? */
  int ShowScrollBar = FALSE;
  if (VMFlags.Check(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Opt.ShowMenuScrollbar)
    ShowScrollBar = TRUE;
  /* VVM $ */

  if (ShowScrollBar && ((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1+1)<ItemCount)
    XX2--;  // уменьшает площадь, в которой меню следит за мышью само

  if (ShowScrollBar && MsX==X2 && ((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1+1)<ItemCount &&
      (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) )
  /* KM $ */
  {
    if (MsY==SbY1)
    {
      while (IsMouseButtonPressed())
      {
        /* $ 11.12.2000 tran
           прокрутка мышью не должна врапить меню
        */
        if (SelectPos!=0)
            ProcessKey(KEY_UP);
        ShowMenu(TRUE);
        /* tran $ */
      }
      return(TRUE);
    }
    if (MsY==SbY2)
    {
      while (IsMouseButtonPressed())
      {
        /* $ 11.12.2000 tran
           прокрутка мышью не должна врапить меню
        */
        if (SelectPos!=ItemCount-1)
            ProcessKey(KEY_DOWN);
        /* tran $ */
        ShowMenu(TRUE);
      }
      return(TRUE);
    }
    if (MsY>SbY1 && MsY<SbY2)
    {
      int SbHeight;
      int Delta=0;
      while (IsMouseButtonPressed())
      {
        SbHeight=Y2-Y1-2;
        MsPos=(ItemCount-1)*(MouseY-Y1)/(SbHeight);
        if(MsPos >= ItemCount)
        {
          MsPos=ItemCount-1;
          Delta=-1;
        }
        if(MsPos < 0)
        {
          MsPos=0;
          Delta=1;
        }
        if(!(Item[MsPos].Flags&LIF_SEPARATOR) && !(Item[MsPos].Flags&LIF_DISABLE))
          SelectPos=SetSelectPos(MsPos,Delta); //??
        ShowMenu(TRUE);
      }
      return(TRUE);
    }
  }
  /* tran 06.07.2000 $ */

  // dwButtonState & 3 - Left & Right button
  if (BoxType!=NO_BOX && (MouseEvent->dwButtonState & 3) && MsX>X1 && MsX<X2)
  {
    if (MsY==Y1)
    {
      while (MsY==Y1 && SelectPos>0 && IsMouseButtonPressed())
        ProcessKey(KEY_UP);
      return(TRUE);
    }
    if (MsY==Y2)
    {
      while (MsY==Y2 && SelectPos<ItemCount-1 && IsMouseButtonPressed())
        ProcessKey(KEY_DOWN);
      return(TRUE);
    }
  }

  /* $ 06.07.2000 tran
     + mouse support for menu scrollbar
     */
  /* $ 21.07.2001 KM
   ! Переработка обработки мыши в меню с флагом VMENU_SHOWNOBOX.
  */
  if ((BoxType!=NO_BOX)?
      (MsX>X1 && MsX<XX2 && MsY>Y1 && MsY<Y2):
      (MsX>=X1 && MsX<=XX2 && MsY>=Y1 && MsY<=Y2))
  /* KM $ */
  /* tran 06.07.2000 $ */
  {
    /* $ 21.07.2001 KM
     ! Переработка обработки мыши в меню с флагом VMENU_SHOWNOBOX.
    */
    MsPos=TopPos+((BoxType!=NO_BOX)?MsY-Y1-1:MsY-Y1);
    /* KM $ */
    if (MsPos<ItemCount && !(Item[MsPos].Flags&LIF_SEPARATOR) && !(Item[MsPos].Flags&LIF_DISABLE))
    {
      if (MouseX!=PrevMouseX || MouseY!=PrevMouseY || MouseEvent->dwEventFlags==0)
      {
/* TODO:

   Это заготовка для управления поведением листов "не в стиле меню" - когда текущий
   указатель списка (позиция) следит за мышой...

        if(!CheckFlags(VMENU_LISTBOX|VMENU_COMBOBOX) && MouseEvent->dwEventFlags==MOUSE_MOVED ||
            CheckFlags(VMENU_LISTBOX|VMENU_COMBOBOX) && MouseEvent->dwEventFlags!=MOUSE_MOVED)
*/
        {
          Item[SelectPos].Flags&=~LIF_SELECTED;
          Item[MsPos].Flags|=LIF_SELECTED;
          SelectPos=MsPos;
        }
        ShowMenu(TRUE);
      }
      /* $ 13.10.2001 VVM
        + Запомнить нажатие клавиши мышки и только в этом случае реагировать при отпускании */
      if (MouseEvent->dwEventFlags==0 &&
         (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
        VMenu::VMFlags.Set(VMENU_MOUSEDOWN);
      if (MouseEvent->dwEventFlags==0 &&
         (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED))==0 &&
          VMenu::VMFlags.Check(VMENU_MOUSEDOWN))
      {
        VMenu::VMFlags.Clear(VMENU_MOUSEDOWN);
        ProcessKey(KEY_ENTER);
      }
      /* VVM $ */
    }
    return(TRUE);
  }
  else
    if (BoxType!=NO_BOX && (MouseEvent->dwButtonState & 3) && MouseEvent->dwEventFlags==0)
    {
      ProcessKey(KEY_ESC);
      return(TRUE);
    }

  return(FALSE);
}


void VMenu::DeleteItems()
{
  CriticalSectionLock Lock(CS);

  /* $ 13.07.2000 SVS
     ни кто не вызывал запрос памяти через new :-)
  */
  if(Item)
  {
    for(int I=0; I < ItemCount; ++I)
      if(Item[I].UserDataSize > sizeof(Item[I].UserData) && Item[I].UserData)
        xf_free(Item[I].UserData);
    xf_free(Item);
  }
  /* SVS $ */
  Item=NULL;
  ItemCount=0;
  SelectPos=-1;
  TopPos=0;
  MaxLength=strlen(VMenu::Title)+2;
  if(MaxLength > ScrX-8)
    MaxLength=ScrX-8;
  /* $ 23.02.2002 DJ
     а перерисовать?
  */
  VMFlags.Set(VMENU_UPDATEREQUIRED);
  /* DJ $ */
}


/* $ 01.08.2000 SVS
   функция удаления N пунктов меню
*/
int VMenu::DeleteItem(int ID,int Count)
{
  CriticalSectionLock Lock(CS);

  int I;

  if(ID < 0 || ID >= ItemCount || Count <= 0)
    return ItemCount;
  if(ID+Count > ItemCount)
    Count=ItemCount-ID; //???
  if(Count <= 0)
    return ItemCount;

  int OldItemSelected=-1;
  for(I=0; I < ItemCount; ++I)
  {
    if(Item[I].Flags & LIF_SELECTED)
      OldItemSelected=I;
    Item [I].Flags&=~LIF_SELECTED;
  }

  if(OldItemSelected >= ID)
  {
    if(ID+Count >= ItemCount)
      OldItemSelected=ID-1;
  }

  if(OldItemSelected < 0)
    OldItemSelected=0;

  // Надобно удалить данные, чтоб потери по памяти не были
  for(I=0; I < Count; ++I)
  {
    struct MenuItem *PtrItem=Item+ID+I;
    if(PtrItem->UserDataSize > sizeof(PtrItem->UserData) && PtrItem->UserData)
      xf_free(PtrItem->UserData);
  }

  // а вот теперь перемещения
  if(ItemCount > 1)
    memmove(Item+ID,Item+ID+Count,sizeof(struct MenuItem)*(ItemCount-(ID+Count))); //???

  // коррекция текущей позиции
  if(SelectPos >= ID && SelectPos < ID+Count)
  {
    SelectPos=ID;
    if(ID+Count == ItemCount)
      SelectPos--;
    /* $ 23.02.2002 DJ
       постараемся не ставить выделение на сепаратор
    */
    while (SelectPos > 0 &&(Item [SelectPos].Flags & (LIF_SEPARATOR | LIF_DISABLE)))
      SelectPos--;
    /* DJ $ */
  }

  VMFlags.Clear(VMENU_SELECTPOSNONE);
  if(SelectPos < 0)
  {
    VMFlags.Set(VMENU_SELECTPOSNONE);
    SelectPos=0;
  }

  ItemCount-=Count;

  if(SelectPos < TopPos || SelectPos > TopPos+Y2-Y1 || TopPos >= ItemCount)
  {
    TopPos=0;
    VMFlags.Set(VMENU_UPDATEREQUIRED);
  }

  // Нужно ли обновить экран?
  if ((ID >= TopPos && ID < TopPos+Y2-Y1) ||
      (ID+Count >= TopPos && ID+Count < TopPos+Y2-Y1)) //???
  {
    VMFlags.Set(VMENU_UPDATEREQUIRED);
  }

  SelectPos=SetSelectPos(OldItemSelected,1);
  if (Item[SelectPos].Flags & (LIF_SEPARATOR | LIF_DISABLE))
    VMFlags.Set(VMENU_SELECTPOSNONE);

  if(SelectPos > -1)
    Item[SelectPos].Flags|=LIF_SELECTED;

  return(ItemCount);
}
/* SVS $ */

int VMenu::AddItem(const struct MenuItem *NewItem,int PosAdd)
{
  CriticalSectionLock Lock(CS);

  if(!NewItem)
    return -1; //???

  struct MenuItem *NewPtr;
  int Length;

  if(PosAdd >= ItemCount)
    PosAdd=ItemCount;

  if (UpdateRequired())
    VMFlags.Set(VMENU_UPDATEREQUIRED);

  if ((ItemCount & 255)==0)
  {
    if ((NewPtr=(struct MenuItem *)xf_realloc(Item,sizeof(struct MenuItem)*(ItemCount+256+1)))==NULL)
      return(0);
    Item=NewPtr;
  }

  // Если < 0 - однозначно ставим в нулевую позицию, т.е добавка сверху
  if(PosAdd < 0)
    PosAdd=0;

  if(PosAdd < ItemCount)
    memmove(Item+PosAdd+1,Item+PosAdd,sizeof(struct MenuItem)*(ItemCount-PosAdd)); //??

  Item[PosAdd]=*NewItem;

  if(VMFlags.Check(VMENU_SHOWAMPERSAND))
    Length=strlen(Item[PosAdd].PtrName());
  else
    Length=HiStrlen(Item[PosAdd].PtrName(),TRUE);

  if (Length>MaxLength)
    MaxLength=Length;
  if(MaxLength > ScrX-8)
    MaxLength=ScrX-8;
  if (Item[PosAdd].Flags&LIF_SELECTED)
    SelectPos=PosAdd;
  if(Item[PosAdd].Flags&0x0000FFFF)
  {
    Item[PosAdd].Flags|=LIF_CHECKED;
    if((Item[PosAdd].Flags&0x0000FFFF) == 1)
      Item[PosAdd].Flags&=0xFFFF0000;
  }
  Item[PosAdd].AmpPos=-1;

  VMFlags.Clear(VMENU_SELECTPOSNONE);
  if(SelectPos < 0)
  {
    VMFlags.Set(VMENU_SELECTPOSNONE);
    //SelectPos=0;
  }

  // Вычисление размеров
  int I=0, J=0;
  char Chr;
  const char *NamePtr=Item[PosAdd].PtrName();
  while((Chr=NamePtr[I]) != 0)
  {
    if(Chr != '&' && Chr != '\t')
      J++;
    else
    {
      if(Chr != '&')
        Item[PosAdd].Idx2=++J;
      else if(Item[PosAdd].AmpPos != -1)
        Item[PosAdd].AmpPos=J;
    }
    ++I;
  }

  Item[PosAdd].Len[0]=strlen(NamePtr)-Item[PosAdd].Idx2; //??
  if(Item[PosAdd].Idx2)
    Item[PosAdd].Len[1]=strlen(&NamePtr[Item[PosAdd].Idx2]);

  // Уточнение общих размеров
  if(RLen[0] < Item[PosAdd].Len[0])
    RLen[0]=Item[PosAdd].Len[0];
  if(RLen[1] < Item[PosAdd].Len[1])
    RLen[1]=Item[PosAdd].Len[1];

  if(VMFlags.Check(VMENU_AUTOHIGHLIGHT|VMENU_REVERSEHIGHLIGHT))
    AssignHighlights(VMFlags.Check(VMENU_REVERSEHIGHLIGHT));
//  if(VMFlags.Check(VMENU_LISTBOXSORT))
//    SortItems(0);
  LastAddedItem = PosAdd;
  return(ItemCount++);
}

int  VMenu::AddItem(const char *NewStrItem)
{
  CriticalSectionLock Lock(CS);

  struct FarList FarList0;
  struct FarListItem FarListItem0;

  memset(&FarListItem0,0,sizeof(FarListItem0));
  if(!NewStrItem || NewStrItem[0] == 0x1)
  {
    FarListItem0.Flags=LIF_SEPARATOR;
    xstrncpy(FarListItem0.Text,NewStrItem+1,sizeof(FarListItem0.Text)-2);
  }
  else
  {
    xstrncpy(FarListItem0.Text,NewStrItem,sizeof(FarListItem0.Text)-1);
  }
  FarList0.ItemsNumber=1;
  FarList0.Items=&FarListItem0;
  return VMenu::AddItem(&FarList0);
}

int VMenu::AddItem(const struct FarList *List)
{
  CriticalSectionLock Lock(CS);

  if(List && List->Items)
  {
    struct MenuItem MItem;
    struct FarListItem *FItem=List->Items;

    for (int J=0; J < List->ItemsNumber; J++, ++FItem)
      AddItem(FarList2MenuItem(FItem,&MItem));
  }
  return ItemCount;
}

int VMenu::UpdateItem(const struct FarListUpdate *NewItem)
{
  CriticalSectionLock Lock(CS);

  if(NewItem && (DWORD)NewItem->Index < (DWORD)ItemCount)
  {
    struct MenuItem MItem;
    // Освободим память... от ранее занятого ;-)
    struct MenuItem *PItem=Item+NewItem->Index;
    if(PItem->UserDataSize > sizeof(PItem->UserData) && PItem->UserData && (NewItem->Item.Flags&LIF_DELETEUSERDATA))
    {
      xf_free(PItem->UserData);
      PItem->UserData=NULL;
      PItem->UserDataSize=0;
    }

    FarList2MenuItem(&NewItem->Item,&MItem);
    PItem->Flags=MItem.Flags;
    memcpy(PItem->Name,MItem.Name,sizeof(PItem->Name));

    /* $ 23.02.2002 DJ
       если элемент selected - поставим на него выделение
    */
    if (PItem->Flags & LIF_SELECTED && !(PItem->Flags & (LIF_SEPARATOR | LIF_DISABLE)))
      SelectPos = NewItem->Index;
    AdjustSelectPos();
    /* DJ $ */

    return TRUE;
  }
  return FALSE;
}

int VMenu::InsertItem(const struct FarListInsert *NewItem)
{
  CriticalSectionLock Lock(CS);

  if(NewItem)
  {
    struct MenuItem MItem;
    return AddItem(FarList2MenuItem(&NewItem->Item,&MItem),NewItem->Index);
  }
  return -1;
}

int VMenu::GetUserDataSize(int Position)
{
  CriticalSectionLock Lock(CS);

  if (ItemCount==0)
    return(0);

  int DataSize=Item[GetPosition(Position)].UserDataSize;

  return(DataSize);
}

int VMenu::_SetUserData(struct MenuItem *PItem,
                       const void *Data,   // Данные
                       int Size)     // Размер, если =0 то предполагается, что в Data-строка
{
  if(PItem->UserDataSize > sizeof(PItem->UserData) && PItem->UserData)
    xf_free(PItem->UserData);

  PItem->UserDataSize=0;
  PItem->UserData=NULL;

  if(Data)
  {
    int SizeReal=Size;

    // Если Size=0, то подразумевается, что в Data находится ASCIIZ строка
    if(!Size)
      SizeReal=strlen((const char*)Data)+1;

    // если размер данных Size=0 или Size больше 4 байт (sizeof(void*))
    if(!Size ||
        Size > sizeof(PItem->UserData)) // если в 4 байта не влезаем, то...
    {
      // размер больше 4 байт?
      if(SizeReal > sizeof(PItem->UserData))
      {
        // ...значит выделяем нужную память.
        if((PItem->UserData=(char*)xf_malloc(SizeReal)) != NULL)
        {
          PItem->UserDataSize=SizeReal;
          memcpy(PItem->UserData,Data,SizeReal);
        }
      }
      else // ЭТА СТРОКА ПОМЕЩАЕТСЯ В 4 БАЙТА!
      {
        PItem->UserDataSize=SizeReal;
        memcpy(PItem->Str4,Data,SizeReal);
      }
    }
    else // Ок. данные помещаются в 4 байта...
    {
      PItem->UserDataSize=0;         // признак того, что данных либо нет, либо
      PItem->UserData=(char*)Data;   // они помещаются в 4 байта
    }
  }
  return(PItem->UserDataSize);
}

void* VMenu::_GetUserData(struct MenuItem *PItem,void *Data,int Size)
{
  int DataSize=PItem->UserDataSize;
  char *PtrData=PItem->UserData; // PtrData содержит: либо указатель на что-то либо
                                 // 4 байта!
  /* $ 12.06.2001 KM
     - Некорректно работала функция. Забыли, что данные в меню
       могут быть в простом MenuItem.Name
  */
  if (Size > 0 && Data != NULL)
  {
    if (PtrData) // данные есть?
    {
      // размерчик больше 4 байт?
      if(DataSize > sizeof(PItem->UserData))
      {
        memmove(Data,PtrData,Min(Size,DataSize));
      }
      else if(DataSize > 0) // а данные то вообще есть? Т.е. если в UserData
      {                     // есть строка из 4 байт (UserDataSize при этом > 0)
        memmove(Data,PItem->Str4,Min(Size,DataSize));
      }
      // else а иначе... в PtrData уже указатель сидит!
    }
    else // ... данных нет, значит лудим имя пункта!
    {
      PtrData=PItem->PtrName();
      if(PItem->Flags&MIF_USETEXTPTR)
        memmove(Data,PtrData,Min(Size,(int)strlen(PtrData)));
      else
        memmove(Data,PItem->Name,Min(Size,(int)sizeof(PItem->Name)));
    }
  }
  /* KM $ */
  return(PtrData);
}

struct FarListItem *VMenu::MenuItem2FarList(const struct MenuItem *MItem,
                                            struct FarListItem *FItem)
{
  if(FItem && MItem)
  {
    memset(FItem,0,sizeof(struct FarListItem));
    FItem->Flags=MItem->Flags&(~MIF_USETEXTPTR); //??
    xstrncpy(FItem->Text,((struct MenuItem *)MItem)->PtrName(),sizeof(FItem->Text)-1);
//    FItem->AccelKey=MItem->AccelKey;
    //??????????????????
    //   FItem->UserData=MItem->UserData;
    //   FItem->UserDataSize=MItem->UserDataSize;
    //??????????????????
    return FItem;
  }
  return NULL;
}

struct MenuItem *VMenu::FarList2MenuItem(const struct FarListItem *FItem,
                                         struct MenuItem *MItem)
{
  if(FItem && MItem)
  {
    memset(MItem,0,sizeof(struct MenuItem));
    MItem->Flags=FItem->Flags;
//    MItem->AccelKey=FItem->AccelKey;
    xstrncpy(MItem->Name,FItem->Text,sizeof(MItem->Name)-1);
    MItem->Flags&=~MIF_USETEXTPTR;
    //MItem->Flags|=LIF_DELETEUSERDATA; //???????????????????
    //VMenu::_SetUserData(MItem,FItem->UserData,FItem->UserDataSize); //???
    // А здесь надо вычислять AmpPos????
    return MItem;
  }
  return NULL;
}

// получить позицию курсора и верхнюю позицию итема
int VMenu::GetSelectPos(struct FarListPos *ListPos)
{
  CriticalSectionLock Lock(CS);

  ListPos->SelectPos=GetSelectPos();
  if(VMFlags.Check(VMENU_SELECTPOSNONE))
    ListPos->SelectPos=-1;
  ListPos->TopPos=TopPos;
  return ListPos->SelectPos;
}

void VMenu::SetMaxHeight(int NewMaxHeight)
{
  CriticalSectionLock Lock(CS);

  VMenu::MaxHeight=NewMaxHeight;
  if(MaxLength > ScrX-8) //
    MaxLength=ScrX-8;
}

// установить курсор и верхний итем
int VMenu::SetSelectPos(struct FarListPos *ListPos)
{
  CriticalSectionLock Lock(CS);

  int Ret=SetSelectPos(ListPos->SelectPos,1);
  int OldTopPos=TopPos;
  if(Ret > -1)
  {
    TopPos=ListPos->TopPos;
    if(ListPos->TopPos == -1)
    {
      if(ItemCount < MaxHeight)
        TopPos=0;
      else
      {

        //TopPos=Ret-MaxHeight/2;               //?????????
        TopPos = (ListPos->SelectPos-ListPos->TopPos+1) > MaxHeight?ListPos->TopPos+1:ListPos->TopPos;
        if(TopPos+MaxHeight > ItemCount)
          TopPos=ItemCount-MaxHeight;

      }
    }

    if(TopPos < 0)
      TopPos = 0;
  }
  return Ret;
}

// переместить курсор с учетом Disabled & Separator
int VMenu::SetSelectPos(int Pos,int Direct)
{
  CriticalSectionLock Lock(CS);

  if(!Item || !ItemCount)
    return -1;

  int OrigPos=Pos, Pass=0, I=0;

  do
  {
    /* $ 21.02.2002 DJ
       в меню без WRAPMODE условие OrigPos == Pos никогда не выполнится =>
       нужно использовать немного другую логику для выхода из цикла
    */
    if (Pos<0)
    {
      if (VMFlags.Check(VMENU_WRAPMODE))
        Pos=ItemCount-1;
      else
      {
        Pos=0;
        Pass++;
      }
    }

    if (Pos>=ItemCount)
    {
      if (VMFlags.Check(VMENU_WRAPMODE))
        Pos=0;
      else
      {
        Pos=ItemCount-1;
        Pass++;
      }
    }
    /* DJ $ */

    if(!(Item[Pos].Flags&LIF_SEPARATOR) && !(Item[Pos].Flags&LIF_DISABLE))
      break;

    Pos+=Direct;

    if(Pass)
      return SelectPos;

    if (I>=ItemCount) // круг пройден - ничего не найдено :-(
      Pass++;

    ++I;
  } while (1);

  /* $ 30.01.2003 KM
     - Иногда фар падал. Как выяснилось если SelectPos был равен -1.
  */
  if (SelectPos!=-1)
    Item[SelectPos].Flags&=~LIF_SELECTED;
  /* KM $ */
  Item[Pos].Flags|=LIF_SELECTED;
  SelectPos=Pos;

  if (SelectPos!=-1)
    VMFlags.Clear(VMENU_SELECTPOSNONE);
  /* $ 01.07.2001 KM
    Дадим знать, что позиция изменилась для перерисовки (диалог
    иногда не "замечал", что позиция изменилась).
  */
  VMFlags.Set(VMENU_UPDATEREQUIRED);
  /* KM $ */
  return Pos;
}

/* $ 21.02.2002 DJ
   корректировка текущей позиции (чтобы не было двух выделенных элементов,
   или чтобы выделенный элемент не был сепаратором)
*/

void VMenu::AdjustSelectPos()
{
  CriticalSectionLock Lock(CS);

  if (!Item || !ItemCount)
    return;

  /* $ 20.07.2004 KM
     Добавим проверку на -1, в противном случае падает меню
     из Dialog API.
  */
  /* $ 27.07.2004 VVM
     Перенесем проверку в другое место :)
  */
//  if (SelectPos!=-1)
//  {
    int OldSelectPos = SelectPos;
    // если selection стоит в некорректном месте - сбросим его
    if (SelectPos >= 0 && Item [SelectPos].Flags & (LIF_SEPARATOR | LIF_DISABLE))
      SelectPos = -1;

    for (int i=0; i<ItemCount; i++)
    {
      if (Item [i].Flags & (LIF_SEPARATOR | LIF_DISABLE))
        Item [i].SetSelect (FALSE);
      else
      {
        if (SelectPos == -1)
        {
          Item [i].SetSelect (TRUE);
          SelectPos = i;
        }
        else if (SelectPos >= 0 && SelectPos != i)
        {
          // если уже есть выделенный элемент - оставим как было
          Item [i].SetSelect (FALSE);
        }
      }
    }

    // если ничего не нашли - оставим как было
    if (SelectPos == -1)
    {
      SelectPos = OldSelectPos;
      if (SelectPos >= 0)
        Item [SelectPos].SetSelect (TRUE);
    }
//  }
  /* KM $ */

  if (SelectPos == -1)
    VMFlags.Set(VMENU_SELECTPOSNONE); //??
  else
    VMFlags.Clear(VMENU_SELECTPOSNONE);
}

/* DJ $ */

void VMenu::SetTitle(const char *Title)
{
  CriticalSectionLock Lock(CS);

  int Length;
  VMFlags.Set(VMENU_UPDATEREQUIRED);
  Title=NullToEmpty(Title);
  xstrncpy(VMenu::Title,Title,sizeof(VMenu::Title)-1);
  Length=strlen(Title)+2;
  if (Length > MaxLength)
    MaxLength=Length;
  if(MaxLength > ScrX-8)
    MaxLength=ScrX-8;

  if(VMFlags.Check(VMENU_CHANGECONSOLETITLE))
  {
    if(*VMenu::Title)
    {
      if(!OldTitle)
        OldTitle=new ConsoleTitle;
      SetFarTitle(VMenu::Title);
    }
    else
    {
      if(OldTitle)
      {
        delete OldTitle;
        OldTitle=NULL;
      }
    }
  }
}


char *VMenu::GetTitle(char *Dest,int Size)
{
  CriticalSectionLock Lock(CS);

  /* $ 23.02.2002 DJ
     Если заголовок пустой - это не значит, что его нельзя вернуть!
  */
  if (Dest /*&& *VMenu::Title*/)
    return xstrncpy(Dest,VMenu::Title,Size-1);
  /* DJ $ */
  return NULL;
}


void VMenu::SetBottomTitle(const char *BottomTitle)
{
  CriticalSectionLock Lock(CS);

  int Length;
  VMFlags.Set(VMENU_UPDATEREQUIRED);
  BottomTitle=NullToEmpty(BottomTitle);
  xstrncpy(VMenu::BottomTitle,BottomTitle,sizeof(VMenu::BottomTitle)-1);
  Length=strlen(BottomTitle)+2;
  if (Length > MaxLength)
    MaxLength=Length;
  if(MaxLength > ScrX-8)
    MaxLength=ScrX-8;
}


char *VMenu::GetBottomTitle(char *Dest,int Size)
{
  CriticalSectionLock Lock(CS);

  if (Dest && *VMenu::BottomTitle)
    return xstrncpy(Dest,VMenu::BottomTitle,Size-1);
  return NULL;
}


void VMenu::SetBoxType(int BoxType)
{
  CriticalSectionLock Lock(CS);

  VMenu::BoxType=BoxType;
}

int VMenu::GetPosition(int Position)
{
  CriticalSectionLock Lock(CS);

  int DataPos=(Position==-1) ? SelectPos : Position;
  if (DataPos>=ItemCount)
    DataPos=ItemCount-1;
  return DataPos;
}


int VMenu::GetSelection(int Position)
{
  CriticalSectionLock Lock(CS);

  if (ItemCount==0)
    return(0);

  int DataPos=GetPosition(Position);
  if (Item[DataPos].Flags&LIF_SEPARATOR)
    return(0);
  int Checked=Item[DataPos].Flags&0xFFFF;
  return((Item[DataPos].Flags&LIF_CHECKED)?(Checked?Checked:1):0);
}


void VMenu::SetSelection(int Selection,int Position)
{
  CriticalSectionLock Lock(CS);

  if (ItemCount==0)
    return;
  Item[GetPosition(Position)].SetCheck(Selection);
}

// Функция GetItemPtr - получить указатель на нужный Item.
struct MenuItem *VMenu::GetItemPtr(int Position)
{
  CriticalSectionLock Lock(CS);

  if (ItemCount==0)
    return NULL;
  return Item+GetPosition(Position);
}

BOOL VMenu::CheckHighlights(BYTE CheckSymbol)
{
  CriticalSectionLock Lock(CS);

  for (int I=0; I < ItemCount; I++)
  {
    char Ch=0;
    const char *Name=Item[I].PtrName();
    const char *ChPtr=strchr(Name,'&');

    if (ChPtr || Item[I].AmpPos > -1)
    {
      if (!ChPtr && Item[I].AmpPos > -1)
      {
        ChPtr=Name+Item[I].AmpPos;
        Ch=*ChPtr;
      }
      else
        Ch=ChPtr[1];

      if(VMFlags.Check(VMENU_SHOWAMPERSAND))
      {
        ChPtr=strchr(ChPtr+1,'&');
        if(ChPtr)
          Ch=ChPtr[1];
      }
    }

    if(Ch && LocalUpper(CheckSymbol) == LocalUpper(Ch))
      return TRUE;
  }
  return FALSE;
}

void VMenu::AssignHighlights(int Reverse)
{
  CriticalSectionLock Lock(CS);

  BYTE Used[256];
  memset(Used,0,sizeof(Used));

  /* $ 02.12.2001 KM
     + Поелику VMENU_SHOWAMPERSAND сбрасывается для корректной
       работы ShowMenu сделаем сохранение энтого флага, в противном
       случае если в диалоге использовался DI_LISTBOX без флага
       DIF_LISTNOAMPERSAND, то амперсанды отображались в списке
       только один раз до следующего ShowMenu.
  */
  if (VMFlags.Check(VMENU_SHOWAMPERSAND))
    VMOldFlags.Set(VMENU_SHOWAMPERSAND);
  if (VMOldFlags.Check(VMENU_SHOWAMPERSAND))
    VMFlags.Set(VMENU_SHOWAMPERSAND);
  /* KM $ */
  int I, Delta=Reverse ? -1:1;
  for (I=(Reverse ? ItemCount-1:0); I >= 0 && I < ItemCount; I+=Delta)
  {
    char Ch=0;
    const char *Name=Item[I].PtrName();
    const char *ChPtr=strchr(Name,'&');

    Item[I].AmpPos=-1;
    if (ChPtr)
    {
      Ch=ChPtr[1];
      if(VMFlags.Check(VMENU_SHOWAMPERSAND))
      {
        ChPtr=strchr(ChPtr+1,'&');
        if(ChPtr)
          Ch=ChPtr[1];
      }
    }

    if(Ch && !Used[LocalUpper(Ch)] && !Used[LocalLower(Ch)])
    {
      Used[LocalUpper(Ch)]++;
      Used[LocalLower(Ch)]++;
      Item[I].AmpPos=ChPtr-Name;
    }
  }
//_SVS(SysLogDump("Used Pre",0,Used,sizeof(Used),NULL));

  // TODO:  ЭТОТ цикл нужно уточнить - возможно вылезут артефакты (хотя не уверен)
  for (I=Reverse ? ItemCount-1:0;I>=0 && I<ItemCount;I+=Reverse ? -1:1)
  {
    const char *Name=Item[I].PtrName();
    const char *ChPtr=strchr(Name,'&');
    if (ChPtr==NULL || VMFlags.Check(VMENU_SHOWAMPERSAND))
    {
      for (int J=0; Name[J]; J++)
      {
        char Ch=Name[J];
        if((Ch =='&' || LocalIsalpha(Ch) || (Ch >= '0' && Ch <='9')) &&
             !Used[LocalUpper(Ch)] && !Used[LocalLower(Ch)])
        {
          Used[LocalUpper(Ch)]++;
          Used[LocalLower(Ch)]++;
          Item[I].AmpPos=J;
          break;
        }
      }
    }
  }
//_SVS(SysLogDump("Used Post",0,Used,sizeof(Used),NULL));
  VMFlags.Set(VMENU_AUTOHIGHLIGHT|(Reverse?VMENU_REVERSEHIGHLIGHT:0));
  VMFlags.Clear(VMENU_SHOWAMPERSAND);
}

/* $ 28.07.2000 SVS

*/
void VMenu::SetColors(struct FarListColors *Colors)
{
  CriticalSectionLock Lock(CS);

  if(Colors)
    memmove(VMenu::Colors,Colors->Colors,sizeof(VMenu::Colors));
  else
  {
    static short StdColor[2][3][VMENU_COLOR_COUNT]=
    {
      // Not VMENU_WARNDIALOG
      {
        { // VMENU_LISTBOX
          COL_DIALOGLISTTEXT,                        // подложка
          COL_DIALOGLISTBOX,                         // рамка
          COL_DIALOGLISTTITLE,                       // заголовок - верхний и нижний
          COL_DIALOGLISTTEXT,                        // Текст пункта
          COL_DIALOGLISTHIGHLIGHT,                   // HotKey
          COL_DIALOGLISTBOX,                         // separator
          COL_DIALOGLISTSELECTEDTEXT,                // Выбранный
          COL_DIALOGLISTSELECTEDHIGHLIGHT,           // Выбранный - HotKey
          COL_DIALOGLISTSCROLLBAR,                   // ScrollBar
          COL_DIALOGLISTDISABLED,                    // Disabled
        },
        { // VMENU_COMBOBOX
          COL_DIALOGCOMBOTEXT,                        // подложка
          COL_DIALOGCOMBOBOX,                         // рамка
          COL_DIALOGCOMBOTITLE,                       // заголовок - верхний и нижний
          COL_DIALOGCOMBOTEXT,                        // Текст пункта
          COL_DIALOGCOMBOHIGHLIGHT,                   // HotKey
          COL_DIALOGCOMBOBOX,                         // separator
          COL_DIALOGCOMBOSELECTEDTEXT,                // Выбранный
          COL_DIALOGCOMBOSELECTEDHIGHLIGHT,           // Выбранный - HotKey
          COL_DIALOGCOMBOSCROLLBAR,                   // ScrollBar
          COL_DIALOGCOMBODISABLED,                    // Disabled
        },
        { // VMenu
          COL_MENUBOX,                               // подложка
          COL_MENUBOX,                               // рамка
          COL_MENUTITLE,                             // заголовок - верхний и нижний
          COL_MENUTEXT,                              // Текст пункта
          COL_MENUHIGHLIGHT,                         // HotKey
          COL_MENUBOX,                               // separator
          COL_MENUSELECTEDTEXT,                      // Выбранный
          COL_MENUSELECTEDHIGHLIGHT,                 // Выбранный - HotKey
          COL_MENUSCROLLBAR,                         // ScrollBar
          COL_MENUDISABLEDTEXT,                      // Disabled
        }
      },

      // == VMENU_WARNDIALOG
      {
        { // VMENU_LISTBOX
          COL_WARNDIALOGLISTTEXT,                    // подложка
          COL_WARNDIALOGLISTBOX,                     // рамка
          COL_WARNDIALOGLISTTITLE,                   // заголовок - верхний и нижний
          COL_WARNDIALOGLISTTEXT,                    // Текст пункта
          COL_WARNDIALOGLISTHIGHLIGHT,               // HotKey
          COL_WARNDIALOGLISTBOX,                     // separator
          COL_WARNDIALOGLISTSELECTEDTEXT,            // Выбранный
          COL_WARNDIALOGLISTSELECTEDHIGHLIGHT,       // Выбранный - HotKey
          COL_WARNDIALOGLISTSCROLLBAR,               // ScrollBar
          COL_WARNDIALOGLISTDISABLED,                // Disabled
        },
        { // VMENU_COMBOBOX
          COL_WARNDIALOGCOMBOTEXT,                   // подложка
          COL_WARNDIALOGCOMBOBOX,                    // рамка
          COL_WARNDIALOGCOMBOTITLE,                  // заголовок - верхний и нижний
          COL_WARNDIALOGCOMBOTEXT,                   // Текст пункта
          COL_WARNDIALOGCOMBOHIGHLIGHT,              // HotKey
          COL_WARNDIALOGCOMBOBOX,                    // separator
          COL_WARNDIALOGCOMBOSELECTEDTEXT,           // Выбранный
          COL_WARNDIALOGCOMBOSELECTEDHIGHLIGHT,      // Выбранный - HotKey
          COL_WARNDIALOGCOMBOSCROLLBAR,              // ScrollBar
          COL_WARNDIALOGCOMBODISABLED,               // Disabled
        },
        { // VMenu
          COL_MENUBOX,                               // подложка
          COL_MENUBOX,                               // рамка
          COL_MENUTITLE,                             // заголовок - верхний и нижний
          COL_MENUTEXT,                              // Текст пункта
          COL_MENUHIGHLIGHT,                         // HotKey
          COL_MENUBOX,                               // separator
          COL_MENUSELECTEDTEXT,                      // Выбранный
          COL_MENUSELECTEDHIGHLIGHT,                 // Выбранный - HotKey
          COL_MENUSCROLLBAR,                         // ScrollBar
          COL_MENUDISABLEDTEXT,                      // Disabled
        }
      }
    };
    int TypeMenu  = CheckFlags(VMENU_LISTBOX)?0:(CheckFlags(VMENU_COMBOBOX)?1:2);
    int StyleMenu = CheckFlags(VMENU_WARNDIALOG)?1:0;
    if(CheckFlags(VMENU_DISABLED))
    {
      VMenu::Colors[0]=FarColorToReal(StyleMenu?COL_WARNDIALOGDISABLED:COL_DIALOGDISABLED);
      for(int I=1; I < VMENU_COLOR_COUNT; ++I)
        VMenu::Colors[I]=VMenu::Colors[0];
    }
    else
    {
      for(int I=0; I < VMENU_COLOR_COUNT; ++I)
        VMenu::Colors[I]=FarColorToReal(StdColor[StyleMenu][TypeMenu][I]);
    }
  }
}

void VMenu::GetColors(struct FarListColors *Colors)
{
  CriticalSectionLock Lock(CS);

  memmove(Colors->Colors,VMenu::Colors,sizeof(VMenu::Colors));
}
/* SVS $*/

/* $ 25.05.2001 DJ
   установка одного цвета
*/
void VMenu::SetOneColor (int Index, short Color)
{
  CriticalSectionLock Lock(CS);

  if ((DWORD)Index < sizeof(Colors) / sizeof (Colors [0]))
    Colors [Index]=FarColorToReal(Color);
}

/* DJ $ */

struct SortItemParam{
  int Direction;
  int Offset;
};

static int __cdecl  SortItem(const struct MenuItem *el1,
                           const struct MenuItem *el2,
                           const struct SortItemParam *Param)
{
  char Name1[2*NM],Name2[2*NM];
  xstrncpy(Name1,((struct MenuItem *)el1)->PtrName(),sizeof(Name1));
  RemoveChar(Name1,'&',TRUE);
  xstrncpy(Name2,((struct MenuItem *)el2)->PtrName(),sizeof(Name2));
  RemoveChar(Name2,'&',TRUE);
  int Res=LocalStricmp(Name1+Param->Offset,Name2+Param->Offset);
  return(Param->Direction==0?Res:(Res<0?1:(Res>0?-1:0)));
}

static int __cdecl  SortItemDataDWORD(const struct MenuItem *el1,
                           const struct MenuItem *el2,
                           const struct SortItemParam *Param)
{
  int Res;
  DWORD Dw1=(DWORD)(((struct MenuItem *)el1)->UserData);
  DWORD Dw2=(DWORD)(((struct MenuItem *)el2)->UserData);
  if(Dw1 == Dw2)
    Res=0;
  else if(Dw1 > Dw2)
    Res=1;
  else
    Res=-1;
  return(Param->Direction==0?Res:(Res<0?1:(Res>0?-1:0)));
}

// Сортировка элементов списка
// Offset - начало сравнения! по умолчанию =0
void VMenu::SortItems(int Direction,int Offset,BOOL SortForDataDWORD)
{
  CriticalSectionLock Lock(CS);

  typedef int (__cdecl *qsortex_fn)(const void*,const void*,void*);
  struct SortItemParam Param;
  Param.Direction=Direction;
  Param.Offset=Offset;

  int I;
  //_SVS(for(I=0; I < ItemCount; ++I)SysLog("%2d) 0x%08X - '%s'",I,Item[I].Flags,Item[I].Name));
  if(!SortForDataDWORD) // обычная сортировка
    qsortex((char *)Item,
          ItemCount,
          sizeof(struct MenuItem),
          (qsortex_fn)SortItem,
          &Param);
  else
    qsortex((char *)Item,
          ItemCount,
          sizeof(struct MenuItem),
          (qsortex_fn)SortItemDataDWORD,
          &Param);
  //_SVS(for(I=0; I < ItemCount; ++I)SysLog("%2d) 0x%08X - '%s'",I,Item[I].Flags,Item[I].Name));

  // скорректируем SelectPos
  for(I=0; I < ItemCount; ++I)
    if (Item[I].Flags & LIF_SELECTED && !(Item[I].Flags & (LIF_SEPARATOR | LIF_DISABLE)))
    {
      SelectPos=I;
      break;
    }

  VMFlags.Set(VMENU_UPDATEREQUIRED);
}

// return Pos || -1
int VMenu::FindItem(const struct FarListFind *FItem)
{
  return FindItem(FItem->StartIndex,FItem->Pattern,FItem->Flags);
}

int VMenu::FindItem(int StartIndex,const char *Pattern,DWORD Flags)
{
  CriticalSectionLock Lock(CS);

  char TmpBuf[130];
  if((DWORD)StartIndex < (DWORD)ItemCount)
  {
    const char *NamePtr;
    int LenPattern=strlen(Pattern);
    for(int I=StartIndex;I < ItemCount;I++)
    {
      NamePtr=Item[I].PtrName();
      int LenNamePtr=strlen(NamePtr);
      memcpy(TmpBuf,NamePtr,Min((int)LenNamePtr+1,(int)sizeof(TmpBuf)));
      if(Flags&LIFIND_EXACTMATCH)
      {
        if(!LocalStrnicmp(RemoveChar(TmpBuf,'&'),Pattern,Max(LenPattern,LenNamePtr)))
          return I;
      }
      else
      {
        if(CmpName(Pattern,RemoveChar(TmpBuf,'&'),1))
          return I;
      }
    }
  }
  return -1;
}

BOOL VMenu::GetVMenuInfo(struct FarListInfo* Info)
{
  CriticalSectionLock Lock(CS);

  if(Info)
  {
    /* $ 23.02.2002 DJ
       нефиг показывать наши внутренние флаги
    */
    Info->Flags=VMFlags.Flags & (LINFO_SHOWNOBOX | LINFO_AUTOHIGHLIGHT
      | LINFO_REVERSEHIGHLIGHT | LINFO_WRAPMODE | LINFO_SHOWAMPERSAND);
    /* DJ $ */
    Info->ItemsNumber=ItemCount;
    Info->SelectPos=SelectPos;
    Info->TopPos=TopPos;
    Info->MaxHeight=MaxHeight;
    Info->MaxLength=MaxLength;
    memset(&Info->Reserved,0,sizeof(Info->Reserved));
    return TRUE;
  }
  return FALSE;
}

// Присовокупить к итему данные.
int VMenu::SetUserData(void *Data,   // Данные
                       int Size,     // Размер, если =0 то предполагается, что в Data-строка
                       int Position) // номер итема
{
  CriticalSectionLock Lock(CS);

  if (ItemCount==0 || Position < 0)
    return(0);

  int DataSize=VMenu::_SetUserData(Item+GetPosition(Position),Data,Size);

  return DataSize;
}

// Получить данные
void* VMenu::GetUserData(void *Data,int Size,int Position)
{
  CriticalSectionLock Lock(CS);

  void *PtrData=NULL;
  if (ItemCount || Position < 0)
  {
    if((Position=GetPosition(Position)) >= 0)
      PtrData=VMenu::_GetUserData(Item+Position,Data,Size);
  }
  return(PtrData);
}

void VMenu::Process()
{
  Modal::Process();
}

void VMenu::ResizeConsole()
{
  CriticalSectionLock Lock(CS);

  /* $ 13.04.2002 KM
    - Добавим проверку на существование буфера сохранения,
      т.к. ResizeConsole вызывается теперь для всех экземпляров
      VMenu при AltF9.
  */
  if (SaveScr)
  {
    SaveScr->Discard();
    delete SaveScr;
    SaveScr=NULL;
  }
  /* KM $ */
  if (this->CheckFlags(VMENU_NOTCHANGE))
  {
    return;
  }
  ObjWidth=ObjHeight=0;
  if (!this->CheckFlags(VMENU_NOTCENTER))
  {
    Y2=X2=Y1=X1=-1;
  }
  else
  {
    X1=5;
    if (!this->CheckFlags(VMENU_LEFTMOST) && ScrX>40)
    {
      X1=(ScrX+1)/2+5;
    }
    Y1=(ScrY+1-(this->ItemCount+5))/2;
    if (Y1<1) Y1=1;
    X2=Y2=0;
  }
}

int VMenu::GetTypeAndName(char *Type,char *Name)
{
  CriticalSectionLock Lock(CS);

  if(Type)
    strcpy(Type,MSG(MVMenuType));
  if(Name)
    strcpy(Name,Title);
  return(CheckFlags(VMENU_COMBOBOX)?MODALTYPE_COMBOBOX:MODALTYPE_VMENU);
}


#ifndef _MSC_VER
#pragma warn -par
#endif
// функция обработки меню (по умолчанию)
long WINAPI VMenu::DefMenuProc(HANDLE hVMenu,int Msg,int Param1,long Param2)
{
  return 0;
}
#ifndef _MSC_VER
#pragma warn +par
#endif

#ifndef _MSC_VER
#pragma warn -par
#endif
// функция посылки сообщений меню
long WINAPI VMenu::SendMenuMessage(HANDLE hVMenu,int Msg,int Param1,long Param2)
{
  CriticalSectionLock Lock(((VMenu*)hVMenu)->CS);

  if(hVMenu)
    return ((VMenu*)hVMenu)->VMenuProc(hVMenu,Msg,Param1,Param2);
  return 0;
}
#ifndef _MSC_VER
#pragma warn +par
#endif

char MenuItem::operator[](int Pos) const
{
  if(Flags&MIF_USETEXTPTR)
    return (!NamePtr || Pos > strlen(NamePtr))?0:NamePtr[Pos];
  return (Pos > strlen(Name))?0:Name[Pos];
}

char* MenuItem::PtrName()
{
  return ((Flags&MIF_USETEXTPTR)!=0&&NamePtr)?NamePtr:Name;
}
