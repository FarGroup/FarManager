/*
findfile.cpp

Поиск (Alt-F7)

*/

/* Revision: 1.167 23.01.2005 $ */

/*
Modify:
  23.01.2005 SVS
    ! Продолжаем клеить поиск (см. 01916.search.txt)
  11.12.2004 WARP
    - Накосячил с обработкой ESC/F10 в поиске.
  09.12.2004 WARP
    ! Обертка из TRY/EXCEPT с посылом в fexcept в потоке поиска файлов (в потоке отрисовки уже есть обработчик).
  09.12.2004 WARP
    ! Устраняем падения поиска при попытке открывать файл,
      когда список файлов решил поменять размер.
    ! Чистим findfile.cpp (см. 01867.Search2.txt).
  08.12.2004 WARP
    ! Патч для поиска #1. Подробнее 01864.FindFile.txt
  01.12.2004 WARP
    - Устраняем глюки с показом окна поиска прямо поверх редактора (сам сотворил).
    ! Убираем лишнее перемергивание листа по окончании поиска (убираем DM_ENABLEREDRAW)
  19.11.2004 WARP
    ! "Нормальная" реализация UpdateRequired для VMenu,
      более интеллектальное обновление листа в поске.
  09.11.2004 WARP
    - Неверное позиционирование в архиве после просмотра файла во время поиска
  25.10.2004 SVS
    ! В процессе поиска: нажатие F3 не тормозит процесс, Enter (при активной кнопки [ View ]) - тормозит.
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  14.06.2004 KM
    ! Уточним действие поиска при использовании фильтра, в котором
      теперь существует обработка атрибута FILE_ATTRIBUTE_DIRECTORY
  08.06.2004 SVS
    ! Вместо GetDriveType теперь вызываем FAR_GetDriveType().
    ! Вместо "DriveType==DRIVE_CDROM" вызываем IsDriveTypeCDROM()
  02.03.2004 SVS
    - BugZ#1040 - неправильное форматирование сообщения
    + небольшая оптимизация коад - общий кусок вынесен в отдельный процедур.
  19.11.2003 IS
    ! теперь NamesList не требует указания размера списка
  26.10.2003 KM
    ! Если используем 16-ричный поиск, то поиск также должен искать
      0 байты: "00 00 00"
  23.10.2003 SVS
    ! вернем обратно вайлы и слипы
  16.10.2003 SVS
    ! Opt.FileSearchMode и Opt.FindFolders вынесены в отдельную структуру struct FindFileOptions
    + FindFileOptions.CollectFiles - собирать NamesList для поисковика (когда жмем F3 в диалоге результатов поиска)
    ! цикл со счетчиком и Sleep(10) убраны в пользу критических секций
  15.10.2003 KM
    - Обработаем переключение "горячих" клавиш
  10.10.2003 SVS
    - Из-за неверного условия в поисковике
       было:  else if (Param1>=19 || Param1<=24)
      не работал кусок кода, отвечающий за переключение строки ввода из одного состояния в другое
      (DI_EDIT <-> DI_FIXEDIT)
  09.10.2003 SVS
    ! В диалоге результатов поиска вместо "FindFile" заюзаем новый раздел помощи "FindFileResult"
  05.10.2003 KM
    ! Размер в результатах поиска выводится с учётом 64-битной математики.
    + В результатах поиска выводятся атрибуты найденных папок и файлов.
    + Чекбокс [ ] Use filter, включающий возможность фильтрования
      операции поиска при помощи использования нового класса FileFilter.
    + Кнопка Filter, позволяющая настраивать фильтр операций.
    ! При включенном фильтре операций тип даты и времени найденных файлов
      выводится в зависимости от установленных в фильтре:
      модификации, создания или последнего доступа.
    + Если используется фильтр операций, то во время поиска сообщаем об этом
  24.09.2003 KM
    - Маленькие правки на предыдущий патч.
  22.09.2003 KM
    + Добавлен поиск 16-ричного кода.
  20.09.2003 KM
    + Добавим в список стандартных таблиц поиска ANSI таблицу,
    а то как-то некузяво получается в редакторе и вьювере она
    есть, а в поиске нет.
  18.09.2003 KM
    ! Запоминание установок символьных таблиц
  12.09.2003 SVS
    ! Немного увеличим буфер для GetPathRootOne
  20.06.2003 SVS
    ! _beginthread(WriteDialogData... перемещен после запуска нитей-сборщиков
    ! Костыль:
      DM_SETFOCUS на 6 элемент производится после прихода эвента DN_DRAWDIALOGDONE
      т.о. мы избавляемся от двойного фокуса, т.к. DM_SETFOCUS будет идти в main-нити
    ! Применим класс ClevMutex (щёб голова не болела о том, что "забыл или не забыл поставть ReleaseMutex()")
  13.06.2003 SVS
    + В диалог поиска файлов добавлена опция "Искать в символических связях"
      ("Search in symbolic links"). Позволяет искать файлы в символических
      связях наравне с обычными подкаталогами. По умолчанию (при запуске
      Фара) ее значение равно значению опции Opt.ScanJunction.
  14.05.2003 SVS
    + _ALGO()
  21.04.2003 SVS
    ! Не "return FALSE;", но "DefDlgProc"
  07.04.2003 VVM
    ! Если во время поиска выбрать найденный файл мышью - вешаемся.
      Не надо посылать сообщение на закрытие диалога. Оно ни к чему...
  15.03.2003 VVM
    - Если диалог вызывается в цикле, то и DI_COMBOBOX надо инитить в цикле.
  20.02.2003 SVS
    ! Заменим strcmp(FooBar,"..") на TestParentFolderName(FooBar)
  04.02.2003 VVM
    ! Подкорректировал работу с мютексом для диалога.
  26.01.2003 IS
    ! FAR_DeleteFile вместо DeleteFile, FAR_RemoveDirectory вместо
      RemoveDirectory, просьба и впредь их использовать для удаления
      соответственно файлов и каталогов.
    ! FAR_CreateFile - обертка для CreateFile, просьба использовать именно
      ее вместо CreateFile
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  19.01.2003 KM
    - Падение в поиске на панели плагина YaCat с включенной
      опцией "Искать в архивах".
    - Неправильный переход на файл в панели плагина YaCat.
  03.12.2002 SVS
    - BugZ#680 - Несанкционированное запоминание настроек поиска
  28.10.2002 SVS
    ! При юзании PrepareTable*() параметр UseTableName=TRUE.
      Этим избавляемся от BugZ#689
  14.10.2002 VVM
    ! После использования плагин будем закрывать. (bug # 590)
  03.10.2002 SVS
    - перелудил в прошлом патче (PluginMode уже объявлена)
  01.10.2002 SVS
    - BugZ#665 - Новая кнопка [Drive]
  01.10.2002 SVS
    - BugZ#664 - лишние "&" в именах кодировок в диалоге поиска
  01.10.2002 VVM
    ! Переместим фокус на кнопку [Go To] при первом добавлении в список,
      а не при прорисовке.
  30.09.2002 SVS
    ! упростим процедуру обработки DN_CTLCOLORDLGLIST - нужно заменить только
      9-й индекс!
  12.09.2002 SVS
    - BugZ#593 - Падает при попытке поиска из меню
  31.08.2002 KM
    ! При поиске в плагине опции поиска "Search in all non-removable drives"
      и "Search in all, except removable and network drives" не
      имеют смысла и поэтому задизаблены.
  14.08.2002 VVM
    ! Уберем возможность переключения в активный редактор из окна поиска.
      Манагер это не умеет...
  28.06.2002 VVM
    + При начале поиска фокус находится на кнопке [New search]
      Как только что-нибудь найдем - сменим его на [Go To], если небыло
      движения по диалогу.
  02.06.2002 KM
    - Уточнение поиска по целым словам. Переработка алгоритма.
  27.05.2002 VVM
    - Большая бага - при поиске по словам выход за границы массива.
  25.05.2002 IS
    ! первый параметр у ConvertDate теперь ссылка на константу
  24.05.2002 SVS
    + Дублирование Numpad-клавиш
  18.05.2002 SVS
    ! Возможность компиляции под BC 5.5
  16.05.2002 SVS
    ! Нужно вместо GetPathRoot в поисковике юзать GetPathRootOne
  15.05.2002 SKV
    + зафиксируем вход в модальный редактор
  06.05.2002 SVS
    ! Перед показом диалога проинициализируем диалог путем вызова InitDialog()
  25.04.2002 IS
    ! внедрение const
  13.04.2002 KM
    ! Предыдущй патч не работал. Баг№445 был изничтожен ещё
      в 1347 патче.
  09.04.2002 SVS
    - BugZ#445 - Перетаскивание диалога поиска порождает мусор
  07.04.2002 KM
    - Вроде должен исчезнуть нестабильный баг с однократным
      отображением "||||" вместо строки "Искать в корня диска".
    - Неперерисовка подложки диалога поиска при выборе
      нового диска.
    - Сломался переход на каталог из списка найденных файлов
      во время починки попадания лишних каталогов во временную
      панель.
  05.04.2002 SVS
    ! MAX_READ -> Opt.PluginMaxReadData
  04.04.2002 KM
    - При смене диска на плагиновый и обратно не обновлялось
      состояние чекбокса "Искать в архивах".
  02.04.2002 KM
    + По просьбам трудящихся добавлен поиск по локальным дискам.
    + Давно хотел: в диалоге поиска при поиске с корня диска
      возможность смены драйва.
  26.03.2002 DJ
    ! ScanTree::GetNextName() принимает размер буфера для имени файла
  24.03.2002 KM
    - Неверно выкидывались во временную панель список файлов
      с папками. При поиске по маске, в которую каталоги не
      попадали, во временную панель попадали папки, в которых
      эти файлы были найдены, но отнюдь не найденные по маске,
      что не есть правильно.
    ! Восстановил поведение поиска в отношении добавления найденных
      каталогов в список к стандарту 1.65 и 3 беты. А речь идёт
      вот о чём. В старом фаре, если при поиске по маске в неё
      попадали и каталоги то они попадали в список найденного
      аналогично файлам:
      ===
      г======================== Find file =========================¬
      ¦C:\Work\FAR_SRC\Far\new\Debug\Plugins\                      ¦
      ¦  p_1.bat                                 0  24.03.02 04:31 ¦
      ¦  p1                                 Folder  24.03.02 04:31 ¦
      ¦  ProcList                           Folder  17.01.02 21:35 ¦
      ===
      При глобальной переделке поиска я это случайно убил.
    ! Сортировку в плагинах по именам папок делаем в случае,
      если имя файла содержит в себе путь, а не только при
      OPIF_REALNAMES, в противном случае в результатах поиска
      получается некрасивый визуальный эффект разбросанности
      одинаковых папок по списку.

  21.03.2002 VVM
    + Передаем имена каталогов на панель без заключительного "\"
  15.01.2002 VVM
    ! Неправильно передавались имена архивов в панель
  15.03.2002 KM
    - Заголовок окна показывался без амперсанда, но с подсветкой
      если в имени искомого файла был амперсанд.
  12.02.2002 VVM
    + Задействуем функцию AbortMessage()
  04.03.2002 DJ
    ! нажатие Esc в диалоге подтверждения прерывания поиска поиск _не_
    прерывает
  01.03.2002 SVS
    ! Есть только одна функция создания временного файла - FarMkTempEx
  21.02.2002 VVM
    ! заменим strcpy на strncpy. А то падает при поиске в слишком
      длинных путях.
    ! Отмена патча про ESC.
  21.02.2002 VVM
    ! При запросе на остановку поиска ESC аналогичен ответу NO
  19.02.2002 VVM
    ! Ошибка поиска в плагине. Забыл скобки поставить...
  11.02.2002 SVS
    ! Проинициализируем новые FarListItem в 0 (поелезно при изменении
      размера структуры FarListItem)
  30.01.2002 VVM
    ! Большой фикс - используем hPluginMutex во время работы с панелью
      плагина. Иначе возможен облом при просмотре найденного файла в
      момент поиска на панели этого-же плагина.
  23.01.2002 VVM
    + GetPluginFile() - получить файл для просмотра с панели плагина.
      В отличие от предыдущего подхода - учитывает вложенность папок
      и делает для них SetDirectory()
  18.01.2002 VVM
    ! При вызове просмотра забыли создать временный каталог.
  17.01.2002 VVM
    ! Выделять элементы в списке будем через ListBox, а не в структуре.
      Убираем возможность двойного выделения
    ! Поскольку работу с поиском в папках вынесли в диалог -
      флаг OPIF_FINDFOLDERS в плагине потерял свою актуальность
  16.01.2002 VVM
    ! В функцию AddMenuRecord не передается параметр Path, он там лишний...
      исправление бага № 236
  15.01.2002 VVM
    ! Исправление поиска в подкаталогах архивов
    + Информация о количестве найденных файлов и каталогов разделена.
  28.12.2001 SVS
    ! Правка с учетом изменений структур (про анонимный union)
  18.12.2001 KM
    - > IS "p.s. заметил на 1127 билде: если искать какую-нибудь (длинную?)
      строку, то при повторном поиске (выбрать "Новый поиск" в диалоге
      поиска) строка поиска будет содержать эту строчку, но усеченную
      по ширине.  Например, я искал "FROM_LEFT_2ND_BUTTON_PRESSED", после
      выбора "Новый поиск" получил "...PRESSED".... Кстати, и ищется
      файлах в обоих случаях не "FROM_LEFT_2ND_BUTTON_PRESSED", а
      "...PRESSED" (где-то ты там оригинальную строку поиска портишь)"
  16.12.2001 KM
    - Наконец-то удавлен баг с торможением клавиш курсора во время
      поиска в архивах. Дело оказалось в моей прошлогодней правке
      при поиске в запароленном архиве - IsPluginGetsFile, но так как
      поиск был переделан на Dialog API, то сей флаг теперь стал мешать.
    + Добавлена информативность в "Searching in:". Если ищем текст в файлах,
      то строка выглядит "Searching "text_to_find" in:".
    + Сделана проверка на существование открытого файла в редакторе при
      нажатии F4 во время поиска, при этом выдаётся запрос как в панелях
      (усечённый запрос).
      Артефакт: при нажатии на F4 во время поиска на файле уже открытом в
      редакторе заголовок консоли продолжает изменяться как-будто открыт
      диалог поиска (устал уже просто работать, может завтра найду :).
  11.12.2001 VVM
    - bugz#162 (* вызвав плагин для поиска в архиве и ничего там не найдя, FAR забыл,
         что поиск продолжается всё в том же каталоге и ещё раз вписал имя
         каталога в список результатов поиска. *)
  11.12.2001 VVM
    - Не рисуем диалог при активном скрин-сэйвере
  03.12.2001 DJ
    - корректный показ имен файлов с амперсандами
  01.12.2001 KM
    - [Bug#145] М-да... Такая невнимательность до добра не доведёт :(
      Перепутать Param1 с Param2 это надо суметь...
    + Добавлена в заголовок окна поиска информация о маске поиска:
      "=== Find files: mask*.ext ==="
      Сам много раз на длинных поисках забывал что ищу...
  23.11.2001 KM
    - БЛИН! Правил глюк и глюк добавил :-(
      Перестали работать чекбоксы и радиобатоны.
  23.11.2001 VVM
    ! Немного изменений, что-бы отрисовка диалога не перекрывала сообщение.
  22.11.2001 KM
    - Забыли однако, что DN_BTNCLICK работает со всеми "нажимающимися"
      контролами, только с кнопками немного по-другому, поэтому простое
      нажатие ENTER на кнопках Find или Cancel ни к чему не приводило.
    + Во время поиска нажали Esc - выдаётся стандартный запрос на останов
      операции (если опция включена) или просто поиск останавливается
      (раньше он ещё и выходил из него).
  22.11.2001 VVM
    ! Сбрасыватьсостояние FindFolders при вводе текста.
      Но только если не меняли этот состояние вручную
      Переделка предыдущего патча.
  15.11.2001 IS
    - ищем каталоги только, когда поле "текст" пустое, иначе ерунда получается
    + диалог поиска: не дадим включить поиск папок, если ищем текст
  13.11.2001 VVM
    ! Хм. Добавим папки на панель, если их искали...
  26.10.2001 KM
    + Добавлена опция "Искать каталоги", позволяющая искать не только файлы, но и каталоги.
    - Была попытка просматривать каталоги из архивов по F3.
  20.10.2001 KM
    ! Подчистки по тексту.
    ! Зачем-то при выбросе файлов во временную панель был сделан переход на файл,
      на котором стоял курсор в списке. Убрано.
  19.10.2001 KM
    - Маленький глючок в моём прошлом патче, перепутал параметры местами у memset.
    ! Использование DIF_SEPARATOR2 вмечто MakeSeparator.
    ! Обработка кнопки [ New search ] сделана через enum VVM для однообразности.
  19.10.2001 VVM
    + Вынесли GoTo за пределы диалоговой процедуры.Причины все те же, что и у [ Panel ]
    ! FindFileArcIndex нельзя использовать при обработке выделенных элементов списка.
      Он может быть и другой.
  19.10.2001 VVM
    ! Попытка номер 2. С учетом замечаний и пожеланий от KM и SVS ;)))
    ! Перетрях поиска, зачистка местности, удавливание багов. Далее по тексту...
  19.10.2001 KM
    ! С подачи VVM сделано добавление в список индекса LIST_INDEX_NONE на пустых строках.
    - Поправлен поиск во временной панели, он там не работал и выдавал "странные результаты",
      не позволял просматривать и редактировать файлы, хотя они и OPIF_REALNAMES.
    + Добавлена SortItems для сортировки PluginPanelItem для поиска во временной панели.
  18.10.2001 KM
    - Фар падал при попытке нажатия на кнопку "View", когда ничего не было найдено.
    ! Некоторые уточнения поведения кнопок и мыши при пустом списке.
  15.10.2001 VVM
    ! Отмена предыдущего патча про хост-файл. Факт отсутсвия хост-файла должен
      учитываться по-другому.
  15.10.2001 KM
    - Фар валился при попытке поиска в плагинах типа Temporary panel
      и Network browser. Не учитывался факт отсутствия хостфайла в этих
      плагинах.
    - Не происходил переход на файл при клике на нём мышой в списке.
    - Забыл обработать KEY_NUMPAD5 в списке найденных файлов.
  13.10.2001 VVM
    ! Баг при поиске в темп-панели.
    + Новая функция - очистить списки. Что-бы небыло перерасхода памяти
      после нажатия на [New search]
  12.10.2001 VVM
    ! Устраняем падение при поиске строки. Забыл, что
      INVALID_HANDLE_VALEU <> 0 ;)
  12.10.2001 VVM
    ! Очередной перетрях поиска. Избавляемся от глобальных переменных,
      которые могут конфликтовать. Список архивов теперь хранит хэндл
      архива и флаги для этого архива.
  09.10.2001 VVM
    ! Переделка поиска - ускорение неимеверное :))))
  07.10.2001 SVS
    ! Нафига юзать пвсевдографигу напрямую? ведь договаривались же -
      только коды!
  07.10.2001 SVS
    ! Небольшое ограничение области действия Dialog
  01.10.2001 VVM
    ! Проверять нада переменные на NULL
  01.10.2001 VVM
    ! После Alt-F7 с поиском в архивах не работали меню и диалоги плагинов
  27.09.2001 IS
    - Левый размер при использовании strncpy
  24.09.2001 OT
    Падение при открытии файла (F3,F4) из поиска
  13.09.2001 KM
    - Не просматривались файлы, найденные в архиве, если поиск начинался
      не с корня архива, а из подкаталогов.
  23.08.2001 VVM
    + В диалоге поиска строка с каталогом расширена на 20 символов - все равно пустовали.
    + Вызвать TruncPathStr() для каталога...
  17.08.2001 KM
    ! При редактировании найденного файла из архива функция "Сохранить"
      заменяется на функцию "Сохранить в", вызывая диалог для ввода имени.
    + Добавлена кнопка "View" в диалог поиска при поиске файла изнутри архива.
    - Поправлена реакция на клавиши серый + и - в просмотре файлов, теперь
      в списке просмотра присутствуют только не архивные файлы.
  15.08.2001 KM
    ! Глобальный перетрях поиска. 5-я серия
    + Добавлены возможности при поиске в архивах:
      1) Просматривать файлы найденные в архиве и находящиеся
         в списке по F3 и F4;
      2) Скидывать список архивов, в которых находятся найденные
         файлы во временную панель.
      3) Производить поиск в архивах не панели плагина, к примеру на
         временной панели, если панель создавалась с флагом OPIF_REALNAMES.
  11.08.2001 KM
    ! Вроде бы удалось нормально синхронизировать нити
      и теперь вся информация о ходе поиска выводится
      корректно.
  10.08.2001 KM
    + Изменение размеров диалога поиска при изменении размеров консоли.
  08.08.2001 KM
    ! Глобальный перетрях поиска. 3-я серия
    ! Кажется удалось избавится ещё от одного потенциально (и не только)
      падучего места при закрытии диалога поиска во время оного: просто
      я забыл перед закрытием диалога дождаться окончания работы второй нити.
    ! Переделан (пока без таблицы ANSI) выбор таблиц поиска.
  07.08.2001 IS
    ! Изменились параметры у FarCharTable
  07.08.2001 SVS
    ! удален FindSaveScr - нафиг, к теропевту. И без него жизнь полна прекрас.
    ! если идет режим поиска (все еще ищем), то отключим вывод помощи,
      ибо... артефакты с прорисовкой. Вот когда закончится поиск, тогде хелп
      будет доступен.
    ! запрещаем во время поиска юзать некоторые манагерные клавиши кроме
      KEY_CTRLALTSHIFTPRESS и KEY_ALTF9
      (2KM: про Alt-F9 - здесь нужно следить DN_RESIZECONSOLE)
    ! во время вызова редактора/вьювера разрешаем юзать клавиши:
      Alt-F9 и F11 :-)
    - ну и напоследок... используя "подвал" :-) корректно восстановим
      экран после F3/F4:
  01.08.2001 KM
    ! Глобальный перетрях поиска. 2-я серия
    ! С подачи OT синхронизацию процессов
      перевёл на Mutex.
    - Кажется удалось избавиться от иксепшенов.
      Артефакты:
        1. Если после F3 или F4 во время поиска
           нажать CAS, то под спрятавшимся
           вьювером/редактором будет видна тень
           от диалога.
        2. После F3 и F4 не восстанавливается
           изображение под диалогом.
        3. Не всегда рисуется количество и место
           поиска.
  31.07.2001 KM
    ! Глобальный перетрях поиска. 1-я серия
      Артефакты:
        1. После F3 и F4 не восстанавливается
           изображение под диалогом.
        2. При выходе из диалога поиска иногда
           выскакивает farexcpt.0xc0000005.
        3. Не всегда корректно синхронизируются нити,
           из-за чего не рисуется количество и место
           поиска.
  27.07.2001 SVS
    ! Опечатка (по наводке IS)
  24.07.2001 IS
    ! Замена проверки на ' ' и '\t' на вызов isspace
    ! Замена проверки на '\n' и '\r' на вызов iseol
  02.07.2001 IS
    ! FileMaskForFindFile.Set(NULL) -> FileMaskForFindFile.Free()
    + Вернул автоматическое добавление '*' к концу маски при определенных
      условиях (это было отменено предыдущим патчем).
  01.07.2001
    + Можно использовать маски исключения при установлении параметров поиска.
  25.06.2001 IS
    ! Внедрение const
  25.06.2001 SVS
    ! Юзаем SEARCHSTRINGBUFSIZE
  23.06.2001 OT
    - косметические исправления, чтобы VC не "предупреждал" :)
  18.06.2001 SVS
    - исправляем последствия предыдущего патча :-)
  18.06.2001 SVS
    - "Невхождение" в архивы - неверное условие на равенство (в 706-м забыл
      этот момент исправить)
  10.06.2001 IS
    + Покажем текущее имя кодовой таблицы в диалоге параметров поиска в файлах.
  09.06.2001 IS
    ! При переходе к найденному не меняем каталог, если мы уже в нем находимся.
      Тем самым добиваемся того, что выделение с элементов панели не
      сбрасывается.
  05.06.2001 SVS
    + Обработчик диалога (без него нажатие на пимпу "[View]" заваливает ФАР)
  04.06.2001 OT
     Подпорка для "естественного" обновления экрана
  03.06.2001 SVS
    ! Изменения в связи с переделкой UserData в VMenu
  30.05.2001 OT
    ! Процессор грузится на 100% после всех найденных файлов, возврат к старому :(... до лучших времен
  26.05.2001 OT
    ! Починка AltF7 в NFZ
  25.05.2001 DJ
    - ставим правильный цвет для disabled строчек
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    ! Константы MENU_ - в морг
  16.05.2001 DJ
    ! proof-of-concept
  15.05.2001 OT
    ! NWZ -> NFZ
  14.05.2001 DJ
    * дизейблим, а не прячем Search in archives на плагиновой панели
    * колесо чтоб работало :-)
  12.05.2001 DJ
    * курсор не останавливается на пустых строках между каталогами
  10.05.2001 DJ
    + поддержка F6 во вьюере/редакторе, вызванных из Find files
  06.05.2001 DJ
    ! перетрях #include
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  30.03.2001 SVS
    ! GetLogicalDrives заменен на FarGetLogicalDrives() в связи с началом
      компании по поддержке виндовой "полиции".
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  27.02.2001 VVM
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  14.12.2000 OT
    -  баг: поиск по Alt-F7 с очень длинной маской поиска/текста
  11.11.2000 SVS
    ! FarMkTemp() - убираем (как всегда - то ставим, то тут же убираем :-(((
  11.11.2000 SVS
    ! Используем конструкцию FarMkTemp()
  21.10.2000 SVS
    ! Добавка для поиска в FFFE-файлах.
  10.09.2000 SVS
    - Запрещаем двигать диалог результатов поиска!
  07.08.2000 KM
    - Глюк в поиске при запароленном архиве, если после получения
      запроса на ввод пароля понажимать долго стрелки вверх или вниз
      или пошевелить мышкой, диалог запроса пароля исчезал.
  05.08.2000 KM
    - Перерисовка каталога поиска в Alt-F7, если пользователь во время
      поиска нажимает стрелки вправо или влево.
  03.08.2000 KM
    + Добавлена возможность поиска по "Целым словам"
  01.08.2000 tran 1.03
    + |DIF_USELASTHISTORY
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "findfile.hpp"
#include "ClevMutex.hpp"
#include "plugin.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "editor.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "chgprior.hpp"
#include "namelist.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "CFileMask.hpp"
#include "filefilter.hpp"
#include "farexcpt.hpp"

#define DLG_HEIGHT 24
#define DLG_WIDTH 74
#define CHAR_TABLE_SIZE 5

#define LIST_DELTA  64
static DWORD LIST_INDEX_NONE = (DWORD)-1;

// Список найденных файлов. Индекс из списка хранится в меню.
static LPFINDLIST  FindList;
static DWORD       FindListCapacity;
static DWORD       FindListCount;
// Список архивов. Если файл найден в архиве, то FindList->ArcIndex указывает сюда.
static LPARCLIST   ArcList;
static DWORD       ArcListCapacity;
static DWORD       ArcListCount;

static CriticalSection ffCS; // чтобы не ресайзили список найденных файлов, пока мы из него читаем
static CriticalSection statusCS; // чтобы не писали в FindMessage/FindFileCount/FindDirCount пока мы оттуда читаем


static DWORD FindFileArcIndex;
// Используются для отправки файлов на временную панель.
// индекс текущего элемента в списке и флаг для отправки.
static DWORD FindExitIndex;
enum {FIND_EXIT_NONE, FIND_EXIT_SEARCHAGAIN, FIND_EXIT_GOTO, FIND_EXIT_PANEL};
static int FindExitCode;
//static char FindFileArcName[NM];

static char FindMask[NM],FindStr[SEARCHSTRINGBUFSIZE];
/* $ 30.07.2000 KM
   Добавлена переменная WholeWords для поиска по точному совпадению
*/
static int SearchMode,CmpCase,WholeWords,SearchInArchives,SearchInSymLink,SearchHex;
/* KM $ */
static int FindFoldersChanged;
static int SearchFromChanged;
static int DlgWidth,DlgHeight;
static volatile int StopSearch,PauseSearch,SearchDone,LastFoundNumber,FindFileCount,FindDirCount,WriteDataUsed,PrepareFilesListUsed;
static char FindMessage[200],LastDirName[2*NM];
static int FindMessageReady,FindCountReady,FindPositionChanged;
static char PluginSearchPath[2*NM];
static HANDLE hDlg;
static int RecurseLevel;
static int BreakMainThread;
static int PluginMode;

static HANDLE hPluginMutex;

static int UseAllTables=FALSE,UseDecodeTable=FALSE,UseANSI=FALSE,UseUnicode=FALSE,TableNum=0,UseFilter=0;
static struct CharTableSet TableSet;

/* $ 01.07.2001 IS
   Объект "маска файлов". Именно его будем использовать для проверки имени
   файла на совпадение с искомым.
*/
static CFileMask FileMaskForFindFile;
/* IS $ */

/* $ 05.10.2003 KM
   Указатель на объект фильтра операций
*/
static FileFilter *Filter;
/* KM $*/

int _cdecl SortItems(const void *p1,const void *p2)
{
  PluginPanelItem *Item1=(PluginPanelItem *)p1;
  PluginPanelItem *Item2=(PluginPanelItem *)p2;
  char n1[NM*2],n2[NM*2];
  n1[0]=0;n2[0]=0;
  if (*Item1->FindData.cFileName)
    xstrncpy(n1,Item1->FindData.cFileName, sizeof(n1)-1);
  if (*Item2->FindData.cFileName)
    xstrncpy(n2,Item2->FindData.cFileName, sizeof(n1)-1);
  *(PointToName(n1))=0;
  *(PointToName(n2))=0;
  return LocalStricmp(n1,n2);
}

long WINAPI FindFiles::MainDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  Dialog* Dlg=(Dialog*)hDlg;
  char *FindText=MSG(MFindFileText),*FindHex=MSG(MFindFileHex),*FindCode=MSG(MFindFileCodePage);
  char DataStr[NM*2];

  switch(Msg)
  {
    case DN_INITDIALOG:
    {
      /* $ 21.09.2003 KM
         Переключение видимости строки ввода искомого текста
         в зависимости от Dlg->Item[13].Selected
      */
      if (Dlg->Item[13].Selected)
      {
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,FALSE);
      }
      else
      {
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,TRUE);
      }

      Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,5,1);
      Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,6,1);
      /* KM $ */

      unsigned int W=Dlg->Item[7].X1-Dlg->Item[4].X1-5;
      if (strlen((Dlg->Item[13].Selected?FindHex:FindText))>W)
      {
        xstrncpy(DataStr,(Dlg->Item[13].Selected?FindHex:FindText),W-3);
        DataStr[W-4]=0;
        strcat(DataStr,"...");
      }
      else
        xstrncpy(DataStr,(Dlg->Item[13].Selected?FindHex:FindText),sizeof(DataStr)-1);
      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,4,(long)DataStr);

      W=Dlg->Item[0].X2-Dlg->Item[7].X1-3;
      if (strlen(FindCode)>W)
      {
        xstrncpy(DataStr,FindCode,W-3);
        DataStr[W-4]=0;
        strcat(DataStr,"...");
      }
      else
        xstrncpy(DataStr,FindCode,sizeof(DataStr)-1);
      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,7,(long)DataStr);

      /* Установка запомненных ранее параметров */
      UseAllTables=Opt.CharTable.AllTables;
      UseANSI=Opt.CharTable.AnsiTable;
      UseUnicode=Opt.CharTable.UnicodeTable;
      UseDecodeTable=((UseAllTables==0) && (UseUnicode==0) && (UseANSI==0) && (Opt.CharTable.TableNum>0));
      if (UseDecodeTable)
        TableNum=Opt.CharTable.TableNum-1;
      else
        TableNum=0;
      /* -------------------------------------- */

      if (UseAllTables)
        xstrncpy(TableSet.TableName,MSG(MFindFileAllTables),sizeof(TableSet.TableName)-1);
      else if (UseUnicode)
        xstrncpy(TableSet.TableName,"Unicode",sizeof(TableSet.TableName)-1);
      /* $ 20.09.2003 KM
         Добавим поддержку ANSI таблицы
      */
      else if (UseANSI)
      {
        GetTable(&TableSet,TRUE,TableNum,UseUnicode);
        xstrncpy(TableSet.TableName,MSG(MGetTableWindowsText),sizeof(TableSet.TableName)-1);
      }
      /* KM $ */
      else if (!UseDecodeTable)
        xstrncpy(TableSet.TableName,MSG(MGetTableNormalText),sizeof(TableSet.TableName)-1);
      else
        PrepareTable(&TableSet,TableNum,TRUE);
      RemoveChar(TableSet.TableName,'&',TRUE);
      Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,8,(long)TableSet.TableName);

      FindFoldersChanged = FALSE;
      SearchFromChanged=FALSE;

      if (Dlg->Item[21].Selected==1)
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,29,TRUE);
      else
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,29,FALSE);

      return TRUE;
    }
    case DN_LISTCHANGE:
    {
      if (Param1==8)
      {
        /* $ 20.09.2003 KM
           Добавим поддержку ANSI таблицы
        */
        UseAllTables=(Param2==0);
        UseANSI=(Param2==3);
        UseUnicode=(Param2==4);
        UseDecodeTable=(Param2>=(CHAR_TABLE_SIZE+1));
        TableNum=Param2-(CHAR_TABLE_SIZE+1);
        if (UseAllTables)
          xstrncpy(TableSet.TableName,MSG(MFindFileAllTables),sizeof(TableSet.TableName)-1);
        else if (UseUnicode)
          xstrncpy(TableSet.TableName,"Unicode",sizeof(TableSet.TableName)-1);
        else if (UseANSI)
        {
          GetTable(&TableSet,TRUE,TableNum,UseUnicode);
          xstrncpy(TableSet.TableName,MSG(MGetTableWindowsText),sizeof(TableSet.TableName)-1);
        }
        else if (!UseDecodeTable)
          xstrncpy(TableSet.TableName,MSG(MGetTableNormalText),sizeof(TableSet.TableName)-1);
        else
          PrepareTable(&TableSet,TableNum,TRUE);
        /* KM $ */
      }
      return TRUE;
    }
    /* 22.11.2001 VVM
      ! Сбрасыватьсостояние FindFolders при вводе текста.
        Но только если не меняли этот состояние вручную */
    case DN_BTNCLICK:
    {
      Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
      char SearchFromRoot[128];
      struct FarDialogItemData ItemData;

      /* $ 23.11.2001 KM
         - БЛИН! Правил глюк и глюк добавил :-(
           Перестали работать чекбоксы и радиобатоны.
      */
      /* $ 22.11.2001 KM
         - Забыли однако, что DN_BTNCLICK работает со всеми "нажимающимися"
           контролами, только с кнопками немного по-другому, поэтому простое
           нажатие ENTER на кнопках Find или Cancel ни к чему не приводило.
      */
      if (Param1==28 || Param1==31) // [ Find ] или [ Cancel ]
        return FALSE;
      else if (Param1==29) // [ Drive ]
      {
        IsRedrawFramesInProcess++;
        ActivePanel->ChangeDisk();
        // Ну что ж, раз пошла такая пьянка рефрешить фреймы
        // будем таким способом.
        //FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
        FrameManager->ResizeAllFrame();
        IsRedrawFramesInProcess--;

        PrepareDriveNameStr(SearchFromRoot,sizeof(SearchFromRoot));
        ItemData.PtrLength=strlen(SearchFromRoot);
        ItemData.PtrData=SearchFromRoot;
        Dialog::SendDlgMessage(hDlg,DM_SETTEXT,21,(long)&ItemData);
        PluginMode=CtrlObject->Cp()->ActivePanel->GetMode()==PLUGIN_PANEL;
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,14,PluginMode?FALSE:TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,19,PluginMode?FALSE:TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,20,PluginMode?FALSE:TRUE);
      }
      else if (Param1==30) // Filter
        Filter->Configure();
      else if (Param1>=19 && Param1<=24)
      {
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,29,Param1==21?TRUE:FALSE);
        SearchFromChanged=TRUE;
      }
      else if (Param1==15) // [ ] Search for folders
        FindFoldersChanged = TRUE;
      else if (Param1==13) // [ ] Search for hexadecimal code
      {
        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        /* $ 21.09.2003 KM
           Переключение видимости строки ввода искомого текста
           в зависимости от установленного чекбокса hex mode
        */
        int LenDataStr=sizeof(DataStr);
        if (Param2)
        {
          Transform((unsigned char *)DataStr,LenDataStr,Dlg->Item[5].Data,'X');
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,6,(long)DataStr);

          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,FALSE);
        }
        else
        {
          Transform((unsigned char *)DataStr,LenDataStr,Dlg->Item[6].Data,'S');
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,5,(long)DataStr);

          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,5,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,6,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,11,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,12,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,15,TRUE);
        }
        /* KM $ */

        unsigned int W=Dlg->Item[7].X1-Dlg->Item[4].X1-5;
        if (strlen((Param2?FindHex:FindText))>W)
        {
          xstrncpy(DataStr,(Param2?FindHex:FindText),W-3);
          DataStr[W-4]=0;
          strcat(DataStr,"...");
        }
        else
          xstrncpy(DataStr,(Param2?FindHex:FindText),sizeof(DataStr)-1);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,4,(long)DataStr);

        if (strlen(DataStr)>0)
        {
          int UnchangeFlag=Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,5,-1);
          Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,6,UnchangeFlag);
        }

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      return TRUE;
      /* KM $ */
      /* KM $ */
    }
    case DN_EDITCHANGE:
    {
      FarDialogItem &Item=*reinterpret_cast<FarDialogItem*>(Param2);

      if ((Param1==5) && (!FindFoldersChanged))
      // Строка "Содержащий текст"
      {
        BOOL Checked = (*Item.Data.Data)?FALSE:Opt.FindOpt.FindFolders;
        if (Checked)
          Dialog::SendDlgMessage(hDlg, DM_SETCHECK, 15, BSTATE_CHECKED);
        else
          Dialog::SendDlgMessage(hDlg, DM_SETCHECK, 15, BSTATE_UNCHECKED);
      }
      return TRUE;
    }
    /* VVM $ */
    /* $ 15.10.2003 KM
        Обработаем "горячие" клавиши
    */
    case DN_HOTKEY:
    {
      if (Param1==4)
      {
        if (Dlg->Item[13].Selected)
          Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,6,0);
        else
          Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,5,0);
        return FALSE;
      }
    }
    /* KM $ */
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}


static void ShowTruncateMessage(int IDMField,int MaxSize)
{
  char Buf1 [128];
  char Buf2 [128];

  *Buf1='\'';
  xstrncpy (Buf1+1, MSG(IDMField), sizeof(Buf1)-3);
  strcat(Buf1,"'");
  RemoveHighlights(Buf1);
  sprintf (Buf2, MSG(MEditInputSize2), MaxSize);
  Message(MSG_WARNING,1,MSG(MWarning),MSG(MEditInputSize1),Buf1,Buf2,MSG(MOk));
}


FindFiles::FindFiles()
{
  _ALGO(CleverSysLog clv("FindFiles::FindFiles()"));
  static char LastFindMask[NM]="*.*",LastFindStr[SEARCHSTRINGBUFSIZE];
  // Статической структуре и статические переменные
  static char SearchFromRoot[128];
  /* $ 30.07.2000 KM
     Добавлена переменная LastWholeWords для поиска по точному совпадению
  */
  static int LastCmpCase=0,LastWholeWords=0,LastSearchInArchives=0,LastSearchInSymLink=-1,LastSearchHex=0;
  /* KM $ */
  int I;

  // Создадим объект фильтра
  Filter=new FileFilter;

  CmpCase=LastCmpCase;
  WholeWords=LastWholeWords;
  SearchInArchives=LastSearchInArchives;
  SearchHex=LastSearchHex;
  if(LastSearchInSymLink == -1)
    LastSearchInSymLink=Opt.ScanJunction;
  if (!RegVer)
    LastSearchInSymLink=0;
  SearchInSymLink=LastSearchInSymLink;
  SearchMode=Opt.FindOpt.FileSearchMode;
  xstrncpy(FindMask,LastFindMask,sizeof(FindMask)-1);
  xstrncpy(FindStr,LastFindStr,sizeof(FindStr)-1);
  BreakMainThread=0;

  xstrncpy(SearchFromRoot,MSG(MSearchFromRoot),sizeof(SearchFromRoot)-1);
  SearchFromRoot[sizeof(SearchFromRoot)-1]=0;


  FarList TableList;
  FarListItem *TableItem=(FarListItem *)xf_malloc(sizeof(FarListItem)*CHAR_TABLE_SIZE);
  TableList.Items=TableItem;
  TableList.ItemsNumber=CHAR_TABLE_SIZE;

  memset(TableItem,0,sizeof(FarListItem)*CHAR_TABLE_SIZE);
  xstrncpy(TableItem[0].Text,MSG(MFindFileAllTables),sizeof(TableItem[0].Text)-1);
  TableItem[1].Flags=LIF_SEPARATOR;
  xstrncpy(TableItem[2].Text,MSG(MGetTableNormalText),sizeof(TableItem[2].Text)-1);
  xstrncpy(TableItem[3].Text,MSG(MGetTableWindowsText),sizeof(TableItem[3].Text)-1);
  xstrncpy(TableItem[4].Text,"Unicode",sizeof(TableItem[4].Text)-1);

  for (I=0;;I++)
  {
    CharTableSet cts;
    int RetVal=FarCharTable(I,(char *)&cts,sizeof(cts));
    if (RetVal==-1)
      break;

    if (I==0)
    {
      TableItem=(FarListItem *)xf_realloc(TableItem,sizeof(FarListItem)*(CHAR_TABLE_SIZE+1));
      if (TableItem==NULL)
        return;
      memset(&TableItem[CHAR_TABLE_SIZE],0,sizeof(FarListItem));
      TableItem[CHAR_TABLE_SIZE].Flags=LIF_SEPARATOR;
      TableList.Items=TableItem;
      TableList.ItemsNumber++;
    }

    TableItem=(FarListItem *)xf_realloc(TableItem,sizeof(FarListItem)*(I+CHAR_TABLE_SIZE+2));
    if (TableItem==NULL)
      return;
    memset(&TableItem[I+CHAR_TABLE_SIZE+1],0,sizeof(FarListItem));
    xstrncpy(TableItem[I+CHAR_TABLE_SIZE+1].Text,cts.TableName,sizeof(TableItem[I+CHAR_TABLE_SIZE+1].Text)-1);
    RemoveChar(TableItem[I+CHAR_TABLE_SIZE+1].Text,'&',TRUE);
    TableList.Items=TableItem;
    TableList.ItemsNumber++;
  }

  FindList = NULL;
  ArcList = NULL;
  hPluginMutex=CreateMutex(NULL,FALSE,NULL);

  do
  {
    ClearAllLists();

    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

    PrepareDriveNameStr(SearchFromRoot,sizeof(SearchFromRoot));

    const char *MasksHistoryName="Masks",*TextHistoryName="SearchText";
    const char *HexMask="HH HH HH HH HH HH HH HH HH HH HH";

    /* $ 30.07.2000 KM
       Добавлен новый checkbox "Whole words" в диалог поиска
    */
    static struct DialogData FindAskDlgData[]=
    {
      /* 00 */DI_DOUBLEBOX,3,1,DLG_WIDTH,DLG_HEIGHT-2,0,0,0,0,(char *)MFindFileTitle,
      /* 01 */DI_TEXT,5,2,0,0,0,0,0,0,(char *)MFindFileMasks,
      /* 02 */DI_EDIT,5,3,72,3,1,(DWORD)MasksHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,"",
      /* 03 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR2,0,"",
      /* 04 */DI_TEXT,5,5,0,0,0,0,0,0,"",
      /* 05 */DI_EDIT,5,6,36,6,0,(DWORD)TextHistoryName,DIF_HISTORY,0,"",
      /* 06 */DI_FIXEDIT,5,6,36,6,0,(DWORD)HexMask,DIF_MASKEDIT,0,"",
      /* 07 */DI_TEXT,40,5,0,0,0,0,0,0,"",
      /* 08 */DI_COMBOBOX,40,6,72,16,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,"",
      /* 09 */DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /* 10 */DI_VTEXT,38,4,0,0,0,0,DIF_BOXCOLOR,0,"\xD1\xB3\xB3\xC1",
      /* 11 */DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MFindFileCase,
      /* 12 */DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MFindFileWholeWords,
      /* 13 */DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MSearchForHex,
      /* 14 */DI_CHECKBOX,40,8,0,0,0,0,0,0,(char *)MFindArchives,
      /* 15 */DI_CHECKBOX,40,9,0,0,0,0,0,0,(char *)MFindFolders,
      /* 16 */DI_CHECKBOX,40,10,0,0,0,0,0,0,(char *)MFindSymLinks,
      /* 17 */DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR2,0,"",
      /* 18 */DI_VTEXT,38,7,0,0,0,0,DIF_BOXCOLOR,0,"\xC5\xB3\xB3\xB3\xCF",
      /* 19 */DI_RADIOBUTTON,5,12,0,0,0,0,DIF_GROUP,0,(char *)MSearchAllDisks,
      /* 20 */DI_RADIOBUTTON,5,13,0,0,0,1,0,0,(char *)MSearchAllButNetwork,
      /* 21 */DI_RADIOBUTTON,5,14,0,0,0,1,0,0,"",
      /* 22 */DI_RADIOBUTTON,5,15,0,0,0,0,0,0,(char *)MSearchFromCurrent,
      /* 23 */DI_RADIOBUTTON,5,16,0,0,0,0,0,0,(char *)MSearchInCurrent,
      /* 24 */DI_RADIOBUTTON,5,17,0,0,0,0,0,0,(char *)MSearchInSelected,
      /* 25 */DI_TEXT,3,18,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /* 26 */DI_CHECKBOX,5,19,0,0,0,0,0,0,(char *)MFindUseFilter,
      /* 27 */DI_TEXT,3,20,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      /* 28 */DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,1,(char *)MFindFileFind,
      /* 29 */DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindFileDrive,
      /* 30 */DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,0,(char *)MFindFileSetFilter,
      /* 31 */DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel,
    };
    /* KM $ */
    FindAskDlgData[21].Data=SearchFromRoot;

    MakeDialogItems(FindAskDlgData,FindAskDlg);


    if (!*FindStr)
      FindAskDlg[15].Selected=Opt.FindOpt.FindFolders;
    for(I=19; I <= 24; ++I)
      FindAskDlg[I].Selected=0;
    FindAskDlg[19+SearchMode].Selected=1;

    {
      if (PluginMode)
      {
        struct OpenPluginInfo Info;
        HANDLE hPlugin=ActivePanel->GetPluginHandle();
        CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
        /* $ 14.05.2001 DJ
           дизейблим, а не прячем
        */
        if ((Info.Flags & OPIF_REALNAMES)==0)
          FindAskDlg[14].Flags |= DIF_DISABLE;
        /* DJ $ */
        if (FindAskDlg[19].Selected || FindAskDlg[20].Selected)
        {
          FindAskDlg[19].Selected=FindAskDlg[20].Selected=0;
          FindAskDlg[21].Selected=1;
        }
        FindAskDlg[19].Flags=FindAskDlg[20].Flags|=DIF_DISABLE;
      }
    }

    if (!RegVer || PluginMode)
    {
      FindAskDlg[16].Selected=0;
      FindAskDlg[16].Flags|=DIF_DISABLE;
    }
    else
      FindAskDlg[16].Selected=SearchInSymLink;

    /* $ 14.05.2001 DJ
       не селектим чекбокс, если нельзя искать в архивах
    */
    if (!(FindAskDlg[14].Flags & DIF_DISABLE))
      FindAskDlg[14].Selected=SearchInArchives;
    /* DJ $ */

    xstrncpy(FindAskDlg[2].Data,FindMask,sizeof(FindAskDlg[2].Data)-1);

    /* $ 26.10.2003 KM */
    if (SearchHex)
      xstrncpy(FindAskDlg[6].Data,FindStr,sizeof(FindAskDlg[6].Data)-1);
    else
      xstrncpy(FindAskDlg[5].Data,FindStr,sizeof(FindAskDlg[5].Data)-1);
    /* KM $ */

    FindAskDlg[11].Selected=CmpCase;
    FindAskDlg[12].Selected=WholeWords;
    FindAskDlg[13].Selected=SearchHex;

    // Использовать фильтр. KM
    FindAskDlg[26].Selected=UseFilter;

    while (1)
    {
      int ExitCode;
      {
        FindAskDlg[8].ListItems=&TableList;

        Dialog Dlg(FindAskDlg,sizeof(FindAskDlg)/sizeof(FindAskDlg[0]),MainDlgProc);

        Dlg.SetHelp("FindFile");
        Dlg.SetPosition(-1,-1,DLG_WIDTH+4,DLG_HEIGHT);
        Dlg.Process();
        ExitCode=Dlg.GetExitCode();
      }

      if (ExitCode!=28)
      {
        xf_free(TableItem);
        CloseHandle(hPluginMutex);
        return;
      }

      /* Запоминание установленных параметров */
      Opt.CharTable.AllTables=UseAllTables;
      Opt.CharTable.AnsiTable=UseANSI;
      Opt.CharTable.UnicodeTable=UseUnicode;
      if (UseDecodeTable)
        Opt.CharTable.TableNum=TableNum+1;
      else
        Opt.CharTable.TableNum=0;
      /****************************************/

      /* $ 01.07.2001 IS
         Проверим маску на корректность
      */
      if(!*FindAskDlg[2].Data)             // если строка с масками пуста,
         strcpy(FindAskDlg[2].Data, "*"); // то считаем, что маска есть "*"

      if(FileMaskForFindFile.Set(FindAskDlg[2].Data, FMF_ADDASTERISK))
           break;
      /* IS $ */
    }

    CmpCase=FindAskDlg[11].Selected;
    /* $ 30.07.2000 KM
       Добавлена переменная
    */
    WholeWords=FindAskDlg[12].Selected;
    /* KM $ */
    SearchHex=FindAskDlg[13].Selected;
    SearchInArchives=FindAskDlg[14].Selected;
    if (FindFoldersChanged)
      Opt.FindOpt.FindFolders=FindAskDlg[15].Selected;

    if (RegVer && !PluginMode)
      SearchInSymLink=FindAskDlg[16].Selected;

    // Запомнить признак использования фильтра. KM
    UseFilter=FindAskDlg[26].Selected;

    /* $ 14.12.2000 OT */

    if (strlen (FindAskDlg[2].Data) > sizeof(FindMask) )
      ShowTruncateMessage(MFindFileMasks,sizeof(FindMask)-1);

    xstrncpy(FindMask,*FindAskDlg[2].Data ? FindAskDlg[2].Data:"*",sizeof(FindMask)-1);

    if (strlen((SearchHex)?FindAskDlg[5].Data:FindAskDlg[6].Data) > sizeof(FindStr))
      ShowTruncateMessage(MFindFileText,sizeof(FindStr)-1);

    /* $ 26.10.2003 KM */
    if (SearchHex)
    {
      xstrncpy(FindStr,FindAskDlg[6].Data,sizeof(FindStr)-1);
      RemoveTrailingSpaces((char *)FindStr);
    }
    else
      xstrncpy(FindStr,FindAskDlg[5].Data,sizeof(FindStr)-1);
    /* KM $ */

    /* OT $ */

    if (*FindStr)
    {
      xstrncpy(GlobalSearchString,FindStr,sizeof(GlobalSearchString)-1);
      GlobalSearchCase=CmpCase;
      /* $ 30.07.2000 KM
         Добавлена переменная
      */
      GlobalSearchWholeWords=WholeWords;
      /* KM $ */
      GlobalSearchHex=SearchHex;
    }
    if (FindAskDlg[19].Selected)
      SearchMode=SEARCH_ALL;
    if (FindAskDlg[20].Selected)
      SearchMode=SEARCH_ALL_BUTNETWORK;
    if (FindAskDlg[21].Selected)
      SearchMode=SEARCH_ROOT;
    if (FindAskDlg[22].Selected)
      SearchMode=SEARCH_FROM_CURRENT;
    if (FindAskDlg[23].Selected)
      SearchMode=SEARCH_CURRENT_ONLY;
    if (FindAskDlg[24].Selected)
        SearchMode=SEARCH_SELECTED;
    if (SearchFromChanged)
    {
      Opt.FindOpt.FileSearchMode=SearchMode;
    }
    LastCmpCase=CmpCase;
    /* $ 30.07.2000 KM
       Добавлена переменная
    */
    LastWholeWords=WholeWords;
    /* KM $ */
    LastSearchHex=SearchHex;
    LastSearchInArchives=SearchInArchives;
    LastSearchInSymLink=SearchInSymLink;
    xstrncpy(LastFindMask,FindMask,sizeof(LastFindMask)-1);
    xstrncpy(LastFindStr,FindStr,sizeof(LastFindStr)-1);
    if (*FindStr)
      Editor::SetReplaceMode(FALSE);
  } while (FindFilesProcess());
  CloseHandle(hPluginMutex);
  xf_free(TableItem);
}


FindFiles::~FindFiles()
{
  /* $ 02.07.2001 IS
     Освободим память.
  */
  FileMaskForFindFile.Free();
  /* IS $ */
  ClearAllLists();
  ScrBuf.ResetShadow();

  // Уничтожим объект фильтра
  if(Filter)
    delete Filter;
}

int FindFiles::GetPluginFile(DWORD ArcIndex, struct PluginPanelItem *PanelItem,
                             char *DestPath, char *ResultName)
{
  _ALGO(CleverSysLog clv("FindFiles::GetPluginFile()"));
  HANDLE hPlugin = ArcList[ArcIndex].hPlugin;
  char SaveDir[NM];
  struct OpenPluginInfo Info;
  CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
  xstrncpy(SaveDir,Info.CurDir,sizeof(SaveDir)-1);
  AddEndSlash(SaveDir);

  CtrlObject->Plugins.SetDirectory(hPlugin,"\\",OPM_SILENT|OPM_FIND);
  SetPluginDirectory(ArcList[ArcIndex].RootPath,hPlugin);
  SetPluginDirectory(PanelItem->FindData.cFileName,hPlugin);

  PluginPanelItem NewItem = *PanelItem;
  char *FileName = PointToName(RemovePseudoBackSlash(NewItem.FindData.cFileName));
  if (FileName != NewItem.FindData.cFileName)
    xstrncpy(NewItem.FindData.cFileName, FileName, sizeof(NewItem.FindData.cFileName));
  int Result = CtrlObject->Plugins.GetFile(hPlugin,&NewItem,DestPath,ResultName,OPM_SILENT|OPM_FIND);

  CtrlObject->Plugins.SetDirectory(hPlugin,"\\",OPM_SILENT|OPM_FIND);
//  SetPluginDirectory(ArcList[ArcIndex].RootPath,hPlugin);
  SetPluginDirectory(SaveDir,hPlugin);
  return(Result);
}

long WINAPI FindFiles::FindDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  int Result;
  Dialog* Dlg=(Dialog*)hDlg;
  VMenu *ListBox=Dlg->Item[1].ListPtr;

  CriticalSectionLock Lock(Dlg->CS);

  switch(Msg)
  {
    case DN_DRAWDIALOGDONE:
    {
      Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
      // Переместим фокус на кнопку [Go To]
      if((FindDirCount || FindFileCount) && !FindPositionChanged)
      {
        FindPositionChanged=TRUE;
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,6/* [Go To] */,0);
      }
//      else
//        ScrBuf.Flush();
      return TRUE;
    }
/*
    case DN_HELP: // в режиме поиска отключим вывод помощи, ибо... артефакты с прорисовкой
    {
      return !SearchDone?NULL:Param2;
    }
*/
    case DN_MOUSECLICK:
    {
      /* $ 07.04.2003 VVM
         ! Если во время поиска выбрать найденный файл мышью - вешаемся.
         Не надо посылать сообщение на закрытие диалога. Оно ни к чему... */
      SMALL_RECT drect,rect;
      int Ret = FALSE;
      Dialog::SendDlgMessage(hDlg,DM_GETDLGRECT,0,(long)&drect);
      Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,1,(long)&rect);
      if (Param1==1 && ((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.X<drect.Left+rect.Right)
      {
        Ret = FindDlgProc(hDlg,DN_BTNCLICK,6,0); // emulates a [ Go to ] button pressing
      }
      /* VVM $ */
      return Ret;
    }

    case DN_KEY:
    {
      if (!StopSearch && ((Param2==KEY_ESC) || (Param2 == KEY_F10)) )
      {
        PauseSearch=TRUE;
        IsProcessAssignMacroKey++; // запретим спец клавиши
                                   // т.е. в этом диалоге нельзя нажать Alt-F9!
        int LocalRes=TRUE;
        if (Opt.Confirm.Esc)
          LocalRes=AbortMessage();
        IsProcessAssignMacroKey--;
        PauseSearch=FALSE;
        StopSearch=LocalRes;

        return TRUE;
      }

      if (!ListBox)
      {
        return TRUE;
      }

      if(Param2 == KEY_LEFT || Param2 == KEY_RIGHT)
        FindPositionChanged = TRUE;

      // некторые спец.клавиши всеже отбработаем.
      if(Param2 == KEY_CTRLALTSHIFTPRESS || Param2 == KEY_ALTF9)
      {
        IsProcessAssignMacroKey--;
        FrameManager->ProcessKey(Param2);
        IsProcessAssignMacroKey++;
        return TRUE;
      }

      if (Param1==9 && (Param2==KEY_RIGHT || Param2==KEY_TAB)) // [ Stop ] button
      {
        FindPositionChanged = TRUE;
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,5/* [ New search ] */,0);
        return TRUE;
      }
      else if (Param1==5 && (Param2==KEY_LEFT || Param2==KEY_SHIFTTAB)) // [ New search ] button
      {
        FindPositionChanged = TRUE;
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,9/* [ Stop ] */,0);
        return TRUE;
      }
      else if (Param2==KEY_UP || Param2==KEY_DOWN || Param2==KEY_PGUP ||
               Param2==KEY_PGDN || Param2==KEY_HOME || Param2==KEY_END ||
               Param2==KEY_MSWHEEL_UP || Param2==KEY_MSWHEEL_DOWN)
      {
        ListBox->ProcessKey(Param2);
        return TRUE;
      }
      else if (Param2==KEY_F3 || Param2==KEY_NUMPAD5 || Param2==KEY_SHIFTNUMPAD5 || Param2==KEY_F4 ||
               Param2==KEY_ENTER && Dialog::SendDlgMessage(hDlg,DM_GETFOCUS,0,0) == 7
              )
      {
        if (ListBox->GetItemCount()==0)
        {
          return TRUE;
        }

        if(Param2==KEY_ENTER && Dialog::SendDlgMessage(hDlg,DM_GETFOCUS,0,0) == 7)
          Param2=KEY_F3;

        ffCS.Enter ();

        DWORD ItemIndex = (DWORD)ListBox->GetUserData(NULL, 0);
        if (ItemIndex != LIST_INDEX_NONE)
        {
          int RemoveTemp=FALSE;
          int ClosePlugin=FALSE; // Плагины надо закрывать, если открыли.
          char SearchFileName[NM];
          char TempDir[NM];

          if (FindList[ItemIndex].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
          {
            ffCS.Leave ();

            return TRUE;
          }
          char *FileName=FindList[ItemIndex].FindData.cFileName;
          // FindFileArcIndex нельзя здесь использовать
          // Он может быть уже другой.
          if ((FindList[ItemIndex].ArcIndex != LIST_INDEX_NONE) &&
              (!(ArcList[FindList[ItemIndex].ArcIndex].Flags & OPIF_REALNAMES)))
          {
            char *FindArcName = ArcList[FindList[ItemIndex].ArcIndex].ArcName;
            if (ArcList[FindList[ItemIndex].ArcIndex].hPlugin == INVALID_HANDLE_VALUE)
            {
              char *Buffer=new char[Opt.PluginMaxReadData];
              FILE *ProcessFile=fopen(FindArcName,"rb");
              if (ProcessFile==NULL)
              {
                delete[] Buffer;

                ffCS.Leave ();
                return TRUE;
              }
              int ReadSize=fread(Buffer,1,Opt.PluginMaxReadData,ProcessFile);
              fclose(ProcessFile);

              int SavePluginsOutput=DisablePluginsOutput;
              DisablePluginsOutput=TRUE;

              WaitForSingleObject(hPluginMutex,INFINITE);
              ArcList[FindList[ItemIndex].ArcIndex].hPlugin = CtrlObject->Plugins.OpenFilePlugin(FindArcName,(unsigned char *)Buffer,ReadSize);
              ReleaseMutex(hPluginMutex);

              DisablePluginsOutput=SavePluginsOutput;

              delete[] Buffer;

              if (ArcList[FindList[ItemIndex].ArcIndex].hPlugin == (HANDLE)-2 ||
                  ArcList[FindList[ItemIndex].ArcIndex].hPlugin == INVALID_HANDLE_VALUE)
              {
                ArcList[FindList[ItemIndex].ArcIndex].hPlugin = INVALID_HANDLE_VALUE;

                ffCS.Leave ();
                return TRUE;
              }
              ClosePlugin = TRUE;
            }

            PluginPanelItem FileItem;
            memset(&FileItem,0,sizeof(FileItem));
            FileItem.FindData=FindList[ItemIndex].FindData;
            FarMkTempEx(TempDir); // А проверка на NULL???
            CreateDirectory(TempDir, NULL);

//            if (!CtrlObject->Plugins.GetFile(ArcList[FindList[ItemIndex].ArcIndex].hPlugin,&FileItem,TempDir,SearchFileName,OPM_SILENT|OPM_FIND))
            WaitForSingleObject(hPluginMutex,INFINITE);
            if (!GetPluginFile(FindList[ItemIndex].ArcIndex,&FileItem,TempDir,SearchFileName))
            {
              FAR_RemoveDirectory(TempDir);
              if (ClosePlugin)
              {
                CtrlObject->Plugins.ClosePlugin(ArcList[FindList[ItemIndex].ArcIndex].hPlugin);
                ArcList[FindList[ItemIndex].ArcIndex].hPlugin = INVALID_HANDLE_VALUE;
              }
              ReleaseMutex(hPluginMutex);

              ffCS.Leave ();
              return FALSE;
            }
            else
            {
              if (ClosePlugin)
              {
                CtrlObject->Plugins.ClosePlugin(ArcList[FindList[ItemIndex].ArcIndex].hPlugin);
                ArcList[FindList[ItemIndex].ArcIndex].hPlugin = INVALID_HANDLE_VALUE;
              }
              ReleaseMutex(hPluginMutex);
            }
            RemoveTemp=TRUE;
          }
          else
            xstrncpy(SearchFileName,FindList[ItemIndex].FindData.cFileName,sizeof(SearchFileName)-1);

          DWORD FileAttr;
          if ((FileAttr=GetFileAttributes(SearchFileName))!=(DWORD)-1)
          {
            char OldTitle[512];
            GetConsoleTitle(OldTitle,sizeof(OldTitle));

            if (Param2==KEY_F3 || Param2==KEY_NUMPAD5 || Param2==KEY_SHIFTNUMPAD5)
            {
              int ListSize=ListBox->GetItemCount();
              NamesList ViewList;
              // Возьмем все файлы, которые имеют реальные имена...
              if(Opt.FindOpt.CollectFiles)
              {
                DWORD Index;
                for (int I=0;I<ListSize;I++)
                {
                  Index = (DWORD)ListBox->GetUserData(NULL, 0, I);
                  LPFINDLIST PtrFindList=FindList+Index;
                  if ((Index != LIST_INDEX_NONE) &&
                      ((PtrFindList->ArcIndex == LIST_INDEX_NONE) ||
                       (ArcList[PtrFindList->ArcIndex].Flags & OPIF_REALNAMES)))
                  {
                    // Не учитывали файлы в архивах с OPIF_REALNAMES
                    if (*PtrFindList->FindData.cFileName && !(PtrFindList->FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
                      ViewList.AddName(PtrFindList->FindData.cFileName);
                  } /* if */
                } /* for */
                ViewList.SetCurName(FindList[ItemIndex].FindData.cFileName);
              }
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
              {
                FileViewer ShellViewer (SearchFileName,FALSE,FALSE,FALSE,-1,NULL,(FindList[ItemIndex].ArcIndex != LIST_INDEX_NONE)?NULL:(Opt.FindOpt.CollectFiles?&ViewList:NULL));
                ShellViewer.SetDynamicallyBorn(FALSE);
                ShellViewer.SetEnableF6(TRUE);
                // FindFileArcIndex нельзя здесь использовать
                // Он может быть уже другой.
                if ((FindList[ItemIndex].ArcIndex != LIST_INDEX_NONE) &&
                    (!(ArcList[FindList[ItemIndex].ArcIndex].Flags & OPIF_REALNAMES)))
                  ShellViewer.SetSaveToSaveAs(TRUE);
                //!Mutex.Unlock();

                ffCS.Leave (); // чтобы поиск продолжался, пока мы тут кнопки давим

                IsProcessVE_FindFile++;
                FrameManager->EnterModalEV();
                FrameManager->ExecuteModal();
                FrameManager->ExitModalEV();
                IsProcessVE_FindFile--;

                ffCS.Enter ();

                // заставляем рефрешится экран
                FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
              }

              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
            }
            else
            {
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,FALSE,0);
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
              {
/* $ 14.08.2002 VVM
  ! Пока-что запретим из поиска переключаться в активный редактор.
    К сожалению, манагер на это не способен сейчас
                int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR,SearchFileName);
                int SwitchTo=FALSE;
                if (FramePos!=-1)
                {
                  if (!(*FrameManager)[FramePos]->GetCanLoseFocus(TRUE) ||
                      Opt.Confirm.AllowReedit)
                  {
                    char MsgFullFileName[NM];
                    xstrncpy(MsgFullFileName,SearchFileName,sizeof(MsgFullFileName)-1);
                    int MsgCode=Message(0,2,MSG(MFindFileTitle),
                          TruncPathStr(MsgFullFileName,ScrX-16),
                          MSG(MAskReload),
                          MSG(MCurrent),MSG(MNewOpen));
                    if (MsgCode==0)
                    {
                      SwitchTo=TRUE;
                    }
                    else if (MsgCode==1)
                    {
                      SwitchTo=FALSE;
                    }
                    else
                    {
                      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
                      Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
                      return TRUE;
                    }
                  }
                  else
                  {
                    SwitchTo=TRUE;
                  }
                }
                if (SwitchTo)
                {
                  (*FrameManager)[FramePos]->SetCanLoseFocus(FALSE);
                  (*FrameManager)[FramePos]->SetDynamicallyBorn(FALSE);
                  FrameManager->ActivateFrame(FramePos);
                  IsProcessVE_FindFile++;
                  FrameManager->EnterModalEV();
                  FrameManager->ExecuteModal ();
                  FrameManager->ExitModalEV();
//                  FrameManager->ExecuteNonModal();
                  IsProcessVE_FindFile--;
                  // заставляем рефрешится экран
                  FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
                }
                else
*/
                {
                  FileEditor ShellEditor (SearchFileName,FALSE,FALSE);
                  ShellEditor.SetDynamicallyBorn(FALSE);
                  ShellEditor.SetEnableF6 (TRUE);
                  // FindFileArcIndex нельзя здесь использовать
                  // Он может быть уже другой.
                  if ((FindList[ItemIndex].ArcIndex != LIST_INDEX_NONE) &&
                      (!(ArcList[FindList[ItemIndex].ArcIndex].Flags & OPIF_REALNAMES)))
                    ShellEditor.SetSaveToSaveAs(TRUE);

                  ffCS.Leave (); // чтобы поиск продолжался, пока мы тут кнопки давим
                  IsProcessVE_FindFile++;
                  FrameManager->EnterModalEV();
                  FrameManager->ExecuteModal ();
                  FrameManager->ExitModalEV();
                  IsProcessVE_FindFile--;

                  ffCS.Enter ();
                  // заставляем рефрешится экран
                  FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
                }
              }
              Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
              Dialog::SendDlgMessage(hDlg,DM_SHOWDIALOG,TRUE,0);
            }
            SetConsoleTitle(OldTitle);
          }
          if (RemoveTemp)
            DeleteFileWithFolder(SearchFileName);
        }

        ffCS.Leave ();

        return TRUE;
      }
      Result = Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
      return Result;
    }
    case DN_BTNCLICK:
    {

      FindPositionChanged = TRUE;

      if (Param1==5) // [ New search ] button pressed
      {
        StopSearch=TRUE;
        FindExitCode=FIND_EXIT_SEARCHAGAIN;
        return FALSE;
      }
      else if (Param1==9) // [ Stop ] button pressed
      {
        if (StopSearch)
          return FALSE;
        StopSearch=TRUE;
        return TRUE;
      }
      else if (Param1==6) // [ Goto ] button pressed
      {
        if (!ListBox)
        {
          return FALSE;
        }

        // Переход будем делать так же после выхода из диалога.
        // Причину смотри для [ Panel ]
        if (ListBox->GetItemCount()==0)
        {
          return (TRUE);
        }
        FindExitIndex = (DWORD)ListBox->GetUserData(NULL, 0);
        if (FindExitIndex != LIST_INDEX_NONE)
          FindExitCode = FIND_EXIT_GOTO;
        return (FALSE);
      }
      else if (Param1==7) // [ View ] button pressed
      {
        FindDlgProc(hDlg,DN_KEY,1,KEY_F3);
        return TRUE;
      }
      else if (Param1==8) // [ Panel ] button pressed
      {
        if (!ListBox)
        {
          return FALSE;
        }

        // На панель будем посылать не в диалоге, а после.
        // После окончания поиска. Иначе возможна ситуация, когда мы
        // ищем на панели, потом ее грохаем и создаем новую (а поиск-то
        // идет!) и в результате ФАР трапается.
        if (ListBox->GetItemCount()==0)
        {
          return (TRUE);
        }
        FindExitCode = FIND_EXIT_PANEL;
        FindExitIndex = (DWORD)ListBox->GetUserData(NULL, 0);
        return (FALSE);
      }
    }
    case DN_CTLCOLORDLGLIST:
    {
      if (Param2)
        ((struct FarListColors *)Param2)->Colors[VMenuColorDisabled]=FarColorToReal(COL_DIALOGLISTTEXT);
      return TRUE;
    }
    case DN_CLOSE:
    {
      StopSearch=TRUE;
      return TRUE;
    }
    /* 10.08.2001 KM
       Изменение размеров диалога поиска при изменении размеров консоли.
    */
    case DN_RESIZECONSOLE:
    {
      COORD coord=(*(COORD*)Param2);
      SMALL_RECT rect;
      int IncY=coord.Y-DlgHeight-3;
      int I;

      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
      for (I=0;I<10;I++)
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,I,FALSE);

      Dialog::SendDlgMessage(hDlg,DM_GETDLGRECT,0,(long)&rect);
      coord.X=rect.Right-rect.Left+1;
      DlgHeight+=IncY;
      coord.Y=DlgHeight;

      if (IncY>0)
        Dialog::SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&coord);

      for (I=0;I<2;I++)
      {
        Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,I,(long)&rect);
        rect.Bottom+=(short)IncY;
        Dialog::SendDlgMessage(hDlg,DM_SETITEMPOSITION,I,(long)&rect);
      }

      for (I=2;I<10;I++)
      {
        Dialog::SendDlgMessage(hDlg,DM_GETITEMPOSITION,I,(long)&rect);
        if (I==2)
          rect.Left=-1;
        rect.Top+=(short)IncY;
        Dialog::SendDlgMessage(hDlg,DM_SETITEMPOSITION,I,(long)&rect);
      }

      if (!(IncY>0))
        Dialog::SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,(long)&coord);

      for (I=0;I<10;I++)
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,I,TRUE);
      Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);

      return TRUE;
    }
    /* KM $ */
  }
  Result = Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
  return Result;
}

int FindFiles::FindFilesProcess()
{
  _ALGO(CleverSysLog clv("FindFiles::FindFilesProcess()"));
  // В статической структуре нужны и статические переменные
  static char Title[2*NM]="";
  static char SearchStr[NM]="";

  /* $ 05.10.2003 KM
     Если используется фильтр операций, то во время поиска сообщаем об этом
  */
  if (*FindMask)
    if (UseFilter)
      sprintf(Title,"%s: %s (%s)",MSG(MFindFileTitle),FindMask,MSG(MFindUsingFilter));
    else
      sprintf(Title,"%s: %s",MSG(MFindFileTitle),FindMask);
  else
    if (UseFilter)
      sprintf(Title,"%s (%s)",MSG(MFindFileTitle),MSG(MFindUsingFilter));
    else
      sprintf(Title,"%s",MSG(MFindFileTitle));
  /* KM $ */
  if (*FindStr)
  {
    /* $ 26.10.2003 KM */
    /* $ 24.09.2003 KM */
    char Temp[NM],FStr[NM*2];

    xstrncpy(FStr,FindStr,sizeof(FStr)-1);
    sprintf(Temp," \"%s\"",TruncStrFromEnd(FStr,10));
    sprintf(SearchStr,MSG(MFindSearchingIn),Temp);
    /* KM $ */
    /* KM $ */
  }
  else
    sprintf(SearchStr,MSG(MFindSearchingIn),"");

  /* $ 15.03.2002 KM
     Заголовок окна показывался без амперсанда.
  /*
  /* $ 03.12.2001 DJ
     корректный показ имен файлов с амперсандами
  */
  static struct DialogData FindDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,DLG_WIDTH,DLG_HEIGHT-4,0,0,DIF_SHOWAMPERSAND,0,Title,
  /* 01 */DI_LISTBOX,4,2,73,DLG_HEIGHT-9,0,0,DIF_LISTNOBOX,0,(char*)0,
  /* 02 */DI_TEXT,-1,DLG_HEIGHT-8,0,DLG_HEIGHT-8,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 03 */DI_TEXT,5,DLG_HEIGHT-7,0,DLG_HEIGHT-7,0,0,DIF_SHOWAMPERSAND,0,SearchStr,
  /* 04 */DI_TEXT,3,DLG_HEIGHT-6,0,DLG_HEIGHT-6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 05 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,1,0,DIF_CENTERGROUP,1,(char *)MFindNewSearch,
  /* 06 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(char *)MFindGoTo,
  /* 07 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(char *)MFindView,
  /* 08 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(char *)MFindPanel,
  /* 09 */DI_BUTTON,0,DLG_HEIGHT-5,0,DLG_HEIGHT-5,0,0,DIF_CENTERGROUP,0,(char *)MFindStop
  };
  /* DJ $ */
  /* KM $ */
  MakeDialogItems(FindDlgData,FindDlg);

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  DlgHeight=DLG_HEIGHT-2;
  DlgWidth=FindDlg[0].X2-FindDlg[0].X1-4;

  int IncY=ScrY>24 ? ScrY-24:0;
  FindDlg[0].Y2+=IncY;
  FindDlg[1].Y2+=IncY;
  FindDlg[2].Y1+=IncY;
  FindDlg[3].Y1+=IncY;
  FindDlg[4].Y1+=IncY;
  FindDlg[5].Y1+=IncY;
  FindDlg[6].Y1+=IncY;
  FindDlg[7].Y1+=IncY;
  FindDlg[8].Y1+=IncY;
  FindDlg[9].Y1+=IncY;

  DlgHeight+=IncY;

  if (PluginMode)
  {
    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    HANDLE hPlugin=ActivePanel->GetPluginHandle();
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

    FindFileArcIndex = AddArcListItem(Info.HostFile, hPlugin, Info.Flags, Info.CurDir);
    if (FindFileArcIndex == LIST_INDEX_NONE)
      return(FALSE);

    if ((Info.Flags & OPIF_REALNAMES)==0)
    {
      FindDlg[8].Type=DI_TEXT;
      *FindDlg[8].Data=0;
    }
  }

  Dialog Dlg=Dialog(FindDlg,sizeof(FindDlg)/sizeof(FindDlg[0]),FindDlgProc);
  hDlg=(HANDLE)&Dlg;
//  pDlg->SetDynamicallyBorn();
  Dlg.SetHelp("FindFileResult");
  Dlg.SetPosition(-1,-1,DLG_WIDTH+4,DLG_HEIGHT-2+IncY);
  // Надо бы показать диалог, а то инициализация элементов запаздывает
  // иногда при поиске и первые элементы не добавляются
  Dlg.InitDialog();
  Dlg.Show();

  LastFoundNumber=0;
  SearchDone=FALSE;
  StopSearch=FALSE;
  PauseSearch=FALSE;
  WriteDataUsed=FALSE;
  PrepareFilesListUsed=0;
  FindFileCount=FindDirCount=0;
  FindExitIndex = LIST_INDEX_NONE;
  FindExitCode = FIND_EXIT_NONE;
  *FindMessage=*LastDirName=FindMessageReady=FindCountReady=FindPositionChanged=0;

  if (PluginMode)
  {
    if (_beginthread(PreparePluginList,0,NULL)==(unsigned long)-1)
      return FALSE;
  }
  else
  {
    if (_beginthread(PrepareFilesList,0,NULL)==(unsigned long)-1)
      return FALSE;
  }

  // Нитка для вывода в диалоге информации о ходе поиска
  if (_beginthread(WriteDialogData,0,NULL)==(unsigned long)-1)
    return FALSE;

  IsProcessAssignMacroKey++; // отключим все спец. клавиши
  Dlg.Process();
  IsProcessAssignMacroKey--;

  while(WriteDataUsed || PrepareFilesListUsed > 0)
    Sleep(10);

  ::hDlg=NULL;

  switch (FindExitCode)
  {
    case FIND_EXIT_SEARCHAGAIN:
    {
      return TRUE;
    }
    case FIND_EXIT_PANEL:
    // Отработаем переброску на временную панель
    {
      int ListSize = FindListCount;
      PluginPanelItem *PanelItems=new PluginPanelItem[ListSize];
      if (PanelItems==NULL)
        ListSize=0;
      int ItemsNumber=0;
      for (int i=0;i<ListSize;i++)
      {
        if (strlen(FindList[i].FindData.cFileName)>0 && FindList[i].Used)
        // Добавляем всегда, если имя задано
        {
          // Для плагинов с виртуальными именами заменим имя файла на имя архива.
          // панель сама уберет лишние дубли.
          int IsArchive = ((FindList[i].ArcIndex != LIST_INDEX_NONE) &&
                          !(ArcList[FindList[i].ArcIndex].Flags&OPIF_REALNAMES));
          // Добавляем только файлы или имена архивов
          /* $ 24.09.2003 KM
             Если включен режим поиска hex-кодов, тогда папки в поиск не включаем
          */
          /* $ 13.11.2001 VVM
            ! Хм. Добавим папки, если их искали... */
          if (IsArchive || Opt.FindOpt.FindFolders ||
              !(FindList[i].FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) &&
              !SearchHex)
          /* VVM $ */
          /* KM $ */
          {
            if (IsArchive)
              xstrncpy(FindList[i].FindData.cFileName, ArcList[FindList[i].ArcIndex].ArcName,
                      sizeof(FindList[i].FindData.cFileName)-1);
            PluginPanelItem *pi=&PanelItems[ItemsNumber++];
            memset(pi,0,sizeof(*pi));
            pi->FindData=FindList[i].FindData;
            if (IsArchive)
              pi->FindData.dwFileAttributes = 0;
            /* $ 21.03.2002 VVM
              + Передаем имена каталогов без заключительного "\" */
            if (pi->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
              int Length = strlen(pi->FindData.cFileName);
              if ((Length) && (pi->FindData.cFileName[Length-1]=='\\'))
                pi->FindData.cFileName[Length-1] = 0;
            }
            /* VVM $ */
          }
        } /* if */
      } /* for */

      HANDLE hNewPlugin=CtrlObject->Plugins.OpenFindListPlugin(PanelItems,ItemsNumber);
      if (hNewPlugin!=INVALID_HANDLE_VALUE)
      {
        Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
        Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hNewPlugin,"");
        NewPanel->SetVisible(TRUE);
        NewPanel->Update(0);
//        if (FindExitIndex != LIST_INDEX_NONE)
//          NewPanel->GoToFile(FindList[FindExitIndex].FindData.cFileName);
        NewPanel->Show();
        NewPanel->SetFocus();
      }
      /* $ 13.07.2000 SVS
         использовали new[]
      */
      delete[] PanelItems;
      break;
    } /* case FIND_EXIT_PANEL */
    case FIND_EXIT_GOTO:
    {
      char *FileName=FindList[FindExitIndex].FindData.cFileName;
      Panel *FindPanel=CtrlObject->Cp()->ActivePanel;

      if ((FindList[FindExitIndex].ArcIndex != LIST_INDEX_NONE) &&
          (!(ArcList[FindList[FindExitIndex].ArcIndex].Flags & OPIF_REALNAMES)))
      {
        HANDLE hPlugin = ArcList[FindList[FindExitIndex].ArcIndex].hPlugin;
        if (hPlugin == INVALID_HANDLE_VALUE)
        {
          char ArcName[NM],ArcPath[NM];
          xstrncpy(ArcName,ArcList[FindList[FindExitIndex].ArcIndex].ArcName,sizeof(ArcName)-1);
          if (FindPanel->GetType()!=FILE_PANEL)
            FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
          xstrncpy(ArcPath,ArcName,sizeof(ArcPath)-1);
          *PointToName(ArcPath)=0;
          FindPanel->SetCurDir(ArcPath,TRUE);
          hPlugin=((FileList *)FindPanel)->OpenFilePlugin(ArcName,FALSE);
          if (hPlugin==(HANDLE)-2)
            hPlugin = INVALID_HANDLE_VALUE;
        } /* if */
        if (hPlugin != INVALID_HANDLE_VALUE)
        {
          struct OpenPluginInfo Info;
          CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

          /* $ 19.01.2003 KM
             Уточнение перехода в нужный каталог плагина.
          */
          if (SearchMode==SEARCH_ROOT ||
              SearchMode==SEARCH_ALL ||
              SearchMode==SEARCH_ALL_BUTNETWORK)
            CtrlObject->Plugins.SetDirectory(hPlugin,"\\",OPM_FIND);
          SetPluginDirectory(FileName,hPlugin,TRUE);
          /* KM $ */
        }
      } /* if */
      else
      {
        char SetName[NM];
        int Length;
        if ((Length=strlen(FileName))==0)
          break;
        if (Length>1 && FileName[Length-1]=='\\' && FileName[Length-2]!=':')
          FileName[Length-1]=0;
        if (GetFileAttributes(FileName)==(DWORD)-1)
          break;
        {
          char *NamePtr;
          NamePtr=PointToName(FileName);
          xstrncpy(SetName,NamePtr,sizeof(SetName)-1);
          *NamePtr=0;
          Length=strlen(FileName);
          if (Length>1 && FileName[Length-1]=='\\' && FileName[Length-2]!=':')
            FileName[Length-1]=0;
        }
        if (*FileName==0)
          break;
        if (FindPanel->GetType()!=FILE_PANEL &&
            CtrlObject->Cp()->GetAnotherPanel(FindPanel)->GetType()==FILE_PANEL)
          FindPanel=CtrlObject->Cp()->GetAnotherPanel(FindPanel);
        if ((FindPanel->GetType()!=FILE_PANEL) || (FindPanel->GetMode()!=NORMAL_PANEL))
        // Сменим панель на обычную файловую...
        {
          FindPanel=CtrlObject->Cp()->ChangePanel(FindPanel,FILE_PANEL,TRUE,TRUE);
          FindPanel->SetVisible(TRUE);
          FindPanel->Update(0);
        }
        /* $ 09.06.2001 IS
           ! Не меняем каталог, если мы уже в нем находимся. Тем самым
             добиваемся того, что выделение с элементов панели не сбрасывается.
        */
        {
          char DirTmp[NM];
          FindPanel->GetCurDir(DirTmp);
          Length=strlen(DirTmp);
          if (Length>1 && DirTmp[Length-1]=='\\' && DirTmp[Length-2]!=':')
            DirTmp[Length-1]=0;
          if(0!=LocalStricmp(FileName, DirTmp))
            FindPanel->SetCurDir(FileName,TRUE);
        }
        /* IS $ */
        if (*SetName)
          FindPanel->GoToFile(SetName);
        FindPanel->Show();
        FindPanel->SetFocus();
      }
      break;
    } /* case FIND_EXIT_GOTO */
  } /* switch */

  return FALSE;
}


void FindFiles::SetPluginDirectory(char *DirName,HANDLE hPlugin,int UpdatePanel)
{
  _ALGO(CleverSysLog clv("FindFiles::SetPluginDirectory()"));
  _ALGO(SysLog("DirName='%s', hPlugin=0x%08X, UpdatePanel=%d",(DirName?DirName:"NULL"),hPlugin,UpdatePanel));
  char Name[NM],*StartName,*EndName;
  int IsPluginDir;

  /* $ 19.01.2003 KM
     Восстановлю поведение до 4 беты. Если в DirName есть
     символ '\x1' значит это путь из плагина. Таким образом
     легче определить плагиновые пути и, соответственно,
     сделать правильный переход.
  */
  if (strlen(DirName)>0)
    IsPluginDir=strchr(DirName,'\x1')!=NULL;
  else
    IsPluginDir=FALSE;

  xstrncpy(Name,DirName,sizeof(Name)-1);
  StartName=Name;
  while(IsPluginDir?(EndName=strchr(StartName,'\x1'))!=NULL:(EndName=strchr(StartName,'\\'))!=NULL)
  /* KM $ */
  {
    *EndName=0;
    // RereadPlugin
    {
      int FileCount=0;
      PluginPanelItem *PanelData=NULL;
      if (CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&FileCount,OPM_FIND))
        CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,FileCount);
    }
    CtrlObject->Plugins.SetDirectory(hPlugin,StartName,OPM_FIND);
    StartName=EndName+1;
  }
  /* $ 19.01.2003 KM
     Отрисуем панель при необходимости.
  */
  if (UpdatePanel)
  {
    CtrlObject->Cp()->ActivePanel->Update(UPDATE_KEEP_SELECTION);
    if (!CtrlObject->Cp()->ActivePanel->GoToFile(StartName))
      CtrlObject->Cp()->ActivePanel->GoToFile(DirName);
    CtrlObject->Cp()->ActivePanel->Show();
  }
  /* KM $ */
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
void _cdecl FindFiles::PrepareFilesList(void *Param)
{
  WIN32_FIND_DATA FindData;
  char FullName[NM],Root[NM*2];

  TRY {

    PrepareFilesListUsed++;
    DWORD DiskMask=FarGetLogicalDrives();
    CtrlObject->CmdLine->GetCurDir(Root);

    for (int CurrentDisk=0;DiskMask!=0;CurrentDisk++,DiskMask>>=1)
    {
      if (SearchMode==SEARCH_ALL ||
          SearchMode==SEARCH_ALL_BUTNETWORK)
      {
        if ((DiskMask & 1)==0)
          continue;
        sprintf(Root,"%c:\\",'A'+CurrentDisk);
        int DriveType=FAR_GetDriveType(Root);
        if (DriveType==DRIVE_REMOVABLE || IsDriveTypeCDROM(DriveType) ||
           (DriveType==DRIVE_REMOTE && SearchMode==SEARCH_ALL_BUTNETWORK))
          if (DiskMask==1)
            break;
          else
            continue;
      }
      else if (SearchMode==SEARCH_ROOT)
        GetPathRootOne(Root,Root);

      {
        ScanTree ScTree(FALSE,SearchMode!=SEARCH_CURRENT_ONLY,SearchInSymLink);

        char SelName[NM];
        int FileAttr;
        if (SearchMode==SEARCH_SELECTED)
          CtrlObject->Cp()->ActivePanel->GetSelName(NULL,FileAttr);

        while (1)
        {
          char CurRoot[2*NM];
          if (SearchMode==SEARCH_SELECTED)
          {
            if (!CtrlObject->Cp()->ActivePanel->GetSelName(SelName,FileAttr))
              break;
            if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0 || TestParentFolderName(SelName) ||
                strcmp(SelName,".")==0)
              continue;
            xstrncpy(CurRoot,Root,sizeof(CurRoot)-1);
            AddEndSlash(CurRoot);
            strcat(CurRoot,SelName);
          }
          else
            xstrncpy(CurRoot,Root,sizeof(CurRoot)-1);

          ScTree.SetFindPath(CurRoot,"*.*");

          statusCS.Enter ();

          xstrncpy(FindMessage,CurRoot,sizeof(FindMessage)-1);
          FindMessage[sizeof(FindMessage)-1]=0;
          FindMessageReady=TRUE;

          statusCS.Leave ();

          while (!StopSearch && ScTree.GetNextName(&FindData,FullName, sizeof (FullName)-1))
          {
            while (PauseSearch)
              Sleep(10);

            /* $ 30.09.2003 KM
              Отфильтруем файлы не попадающие в действующий фильтр
            */
            int IsFile;
            if (UseFilter)
              IsFile=Filter->FileInFilter(&FindData);
            else
              IsFile=TRUE;

            if (IsFile)
            {
              /* $ 14.06.2004 KM
                Уточнение действия при обработке каталогов
              */
              if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
              {
                statusCS.Enter();

                xstrncpy(FindMessage,FullName,sizeof(FindMessage)-1);
                FindMessage[sizeof(FindMessage)-1]=0;
                FindMessageReady=TRUE;

                statusCS.Leave();
              }
              /* KM $ */


              if (IsFileIncluded(NULL,FullName,FindData.dwFileAttributes))
                AddMenuRecord(FullName,&FindData);

              if (SearchInArchives)
                ArchiveSearch(FullName);
            }
            /* KM $ */
          }
          if (SearchMode!=SEARCH_SELECTED)
            break;
        }
        if (SearchMode!=SEARCH_ALL && SearchMode!=SEARCH_ALL_BUTNETWORK)
          break;
      }
    }

    statusCS.Enter();
    sprintf(FindMessage,MSG(MFindDone),FindFileCount,FindDirCount);
    SetFarTitle (FindMessage);
    statusCS.Leave();

    while (!StopSearch && FindMessageReady)
      Sleep(10);
  //  sprintf(FindMessage,MSG(MFindDone),FindFileCount,FindDirCount);

    statusCS.Enter ();

    SearchDone=TRUE;
    FindMessageReady=TRUE;

    statusCS.Leave ();

    PrepareFilesListUsed--;
  }
  EXCEPT (xfilter((int)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
  {
    TerminateProcess( GetCurrentProcess(), 1);
  }
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


void FindFiles::ArchiveSearch(char *ArcName)
{
  _ALGO(CleverSysLog clv("FindFiles::ArchiveSearch()"));
  _ALGO(SysLog("ArcName='%s'",(ArcName?ArcName:"NULL")));
  char *Buffer=new char[Opt.PluginMaxReadData];
  FILE *ProcessFile=fopen(ArcName,"rb");
  if (ProcessFile==NULL)
  {
    /* $ 13.07.2000 SVS
       использовали new[]
    */
    delete[] Buffer;
    /* SVS $ */
    return;
  }
  int ReadSize=fread(Buffer,1,Opt.PluginMaxReadData,ProcessFile);
  fclose(ProcessFile);

  int SavePluginsOutput=DisablePluginsOutput;
  DisablePluginsOutput=TRUE;
  HANDLE hArc=CtrlObject->Plugins.OpenFilePlugin(ArcName,(unsigned char *)Buffer,ReadSize);
  /* $ 01.10.2001 VVM */
  DisablePluginsOutput=SavePluginsOutput;
  /* VVM $ */


  /* $ 13.07.2000 SVS
     использовали new[]
  */
  delete[] Buffer;
  /* SVS $ */

  if (hArc==(HANDLE)-2)
  {
    BreakMainThread=TRUE;
    return;
  }
  if (hArc==INVALID_HANDLE_VALUE)
    return;

  int SaveSearchMode=SearchMode;
  DWORD SaveArcIndex = FindFileArcIndex;
  {
    SearchMode=SEARCH_FROM_CURRENT;
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hArc,&Info);
    FindFileArcIndex = AddArcListItem(ArcName, hArc, Info.Flags, Info.CurDir);
    /* $ 11.12.2001 VVM
      - Запомним каталог перед поиском в архиве.
        И если ничего не нашли - не рисуем его снова */
    {
      char SaveDirName[NM],SaveSearchPath[2*NM];
      int SaveListCount = FindListCount;
      /* $ 19.01.2003 KM
         Запомним пути поиска в плагине, они могут измениться.
      */
      xstrncpy(SaveSearchPath,PluginSearchPath,2*NM);
      /* KM $ */
      xstrncpy(SaveDirName, LastDirName, NM);
      *LastDirName = 0;
      PreparePluginList((void *)1);
      xstrncpy(PluginSearchPath,SaveSearchPath,2*NM);
      WaitForSingleObject(hPluginMutex,INFINITE);
      CtrlObject->Plugins.ClosePlugin(ArcList[FindFileArcIndex].hPlugin);
      ArcList[FindFileArcIndex].hPlugin = INVALID_HANDLE_VALUE;
      ReleaseMutex(hPluginMutex);
      if (SaveListCount == FindListCount)
        xstrncpy(LastDirName, SaveDirName, NM);
    }
    /* VVM $ */
  }
  FindFileArcIndex = SaveArcIndex;
  SearchMode=SaveSearchMode;
}

/* $ 01.07.2001 IS
   Используем FileMaskForFindFile вместо GetCommaWord
*/
int FindFiles::IsFileIncluded(PluginPanelItem *FileItem,char *FullName,DWORD FileAttr)
{
  int FileFound=FileMaskForFindFile.Compare(FullName);
  HANDLE hPlugin=INVALID_HANDLE_VALUE;
  if (FindFileArcIndex != LIST_INDEX_NONE)
    hPlugin = ArcList[FindFileArcIndex].hPlugin;
  while(FileFound)
  {
    /* $ 24.09.2003 KM
       Если включен режим поиска hex-кодов, тогда папки в поиск не включаем
    */
    /* $ 17.01.2002 VVM
      ! Поскольку работу с поиском в папках вынесли в диалог -
        флаг в плагине потерял свою актуальность */
    if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && ((Opt.FindOpt.FindFolders==0) || SearchHex))
//        ((hPlugin == INVALID_HANDLE_VALUE) ||
//        (ArcList[FindFileArcIndex].Flags & OPIF_FINDFOLDERS)==0))
      return FALSE;
    /* VVM $ */
    /* KM $ */

    if (*FindStr && FileFound)
    {
      FileFound=FALSE;
      if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
        break;
      char SearchFileName[NM];
      int RemoveTemp=FALSE;
      if ((hPlugin != INVALID_HANDLE_VALUE) && (ArcList[FindFileArcIndex].Flags & OPIF_REALNAMES)==0)
      {
        char TempDir[NM];
        FarMkTempEx(TempDir); // А проверка на NULL???
        CreateDirectory(TempDir,NULL);
        WaitForSingleObject(hPluginMutex,INFINITE);
        if (!CtrlObject->Plugins.GetFile(hPlugin,FileItem,TempDir,SearchFileName,OPM_SILENT|OPM_FIND))
        {
          ReleaseMutex(hPluginMutex);
          FAR_RemoveDirectory(TempDir);
          break;
        }
        else
          ReleaseMutex(hPluginMutex);
        RemoveTemp=TRUE;
      }
      else
        xstrncpy(SearchFileName,FullName,sizeof(SearchFileName)-1);
      if (LookForString(SearchFileName))
        FileFound=TRUE;
      if (RemoveTemp)
        DeleteFileWithFolder(SearchFileName);
    }
    break;
  }
  return(FileFound);
}
/* IS $ */


void FindFiles::AddMenuRecord(char *FullName, WIN32_FIND_DATA *FindData)
{
  char MenuText[NM],FileText[NM],SizeText[30];
  char Attr[30],Date[30],DateStr[30],TimeStr[30];
  struct MenuItem ListItem;

  Dialog* Dlg=(Dialog*)hDlg;
  if(!Dlg)
  {
    return;
  }

  VMenu *ListBox=Dlg->Item[1].ListPtr;

  if (!ListBox)
  {
    return;
  }

  memset(&ListItem,0,sizeof(ListItem));

  if (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    sprintf(SizeText,"%13s",MSG(MFindFileFolder));
  else
  {
    // Изменим отображение размера с учётом 64-битности размера файла
    __int64 fsize=(unsigned __int64)FindData->nFileSizeLow|((unsigned __int64)FindData->nFileSizeHigh<<32);
    _ui64toa(fsize,SizeText,10);
  }
  char *DisplayName=FindData->cFileName;
  /* $ 24.03.2002 KM
     В плагинах принудительно поставим указатель в имени на имя
     для корректного его отображения в списке, отбросив путь,
     т.к. некоторые плагины возвращают имя вместе с полным путём,
     к примеру временная панель.
  */
  if (FindFileArcIndex != LIST_INDEX_NONE)
    DisplayName=PointToName(DisplayName);
  /* KM $ */

  sprintf(FileText," %-26.26s %13.13s",DisplayName,SizeText);

  /* $ 05.10.2003 KM
     При использовании фильтра по дате будем отображать тот тип
     даты, который задан в фильтре.
  */
  if (UseFilter && Filter->GetParams().FDate.Used)
  {
    switch(Filter->GetParams().FDate.DateType)
    {
      case FDATE_MODIFIED:
        // Отображаем дату последнего изменения
        ConvertDate(FindData->ftLastWriteTime,DateStr,TimeStr,5);
        break;
      case FDATE_CREATED:
        // Отображаем дату создания
        ConvertDate(FindData->ftCreationTime,DateStr,TimeStr,5);
        break;
      case FDATE_OPENED:
        // Отображаем дату последнего доступа
        ConvertDate(FindData->ftLastAccessTime,DateStr,TimeStr,5);
        break;
    }
  }
  else
    // Отображаем дату последнего изменения
    ConvertDate(FindData->ftLastWriteTime,DateStr,TimeStr,5);
  /* KM $ */
  sprintf(Date,"  %s  %s",DateStr,TimeStr);
  strcat(FileText,Date);

  /* $ 05.10.2003 KM
     Отобразим в панели поиска атрибуты найденных файлов
  */
  char AttrStr[8];
  DWORD FileAttr=FindData->dwFileAttributes;
  AttrStr[0]=(FileAttr & FILE_ATTRIBUTE_COMPRESSED) ? 'C':((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)?'E':' ');
  AttrStr[1]=(FileAttr & FILE_ATTRIBUTE_ARCHIVE) ? 'A':' ';
  AttrStr[2]=(FileAttr & FILE_ATTRIBUTE_SYSTEM) ? 'S':' ';
  AttrStr[3]=(FileAttr & FILE_ATTRIBUTE_HIDDEN) ? 'H':' ';
  AttrStr[4]=(FileAttr & FILE_ATTRIBUTE_READONLY) ? 'R':' ';
  AttrStr[5]=0;

  sprintf(Attr," %s",AttrStr);
  strcat(FileText,Attr);
  /* KM $ */
  sprintf(MenuText," %-*.*s",DlgWidth-3,DlgWidth-3,FileText);

  char PathName[2*NM];
  xstrncpy(PathName,FullName,sizeof(PathName)-1);
  PathName[sizeof(PathName)-1]=0;

  RemovePseudoBackSlash(PathName);

  *PointToName(PathName)=0;
  if (*PathName==0)
    strcpy(PathName,".\\");

  AddEndSlash(PathName);

  if (LocalStricmp(PathName,LastDirName)!=0)
  {
    if (*LastDirName)
    {
      /* $ 12.05.2001 DJ
         курсор не останавливается на пустых строках между каталогами
      */
      ListItem.Flags|=LIF_DISABLE;
      // С подачи VVM сделано добавление в список индекса LIST_INDEX_NONE на пустых строках
      ListBox->SetUserData((void*)LIST_INDEX_NONE,sizeof(LIST_INDEX_NONE),ListBox->AddItem(&ListItem));
      ListItem.Flags&=~LIF_DISABLE;
      /* DJ $ */
    }
    xstrncpy(LastDirName,PathName,sizeof(LastDirName)-1);
    if ((FindFileArcIndex != LIST_INDEX_NONE) &&
        (!(ArcList[FindFileArcIndex].Flags & OPIF_REALNAMES)) &&
        (ArcList[FindFileArcIndex].ArcName) &&
        (*ArcList[FindFileArcIndex].ArcName))
    {
      char ArcPathName[NM*2];
      sprintf(ArcPathName,"%s:%s",ArcList[FindFileArcIndex].ArcName,*PathName=='.' ? "\\":PathName);
      xstrncpy(PathName,ArcPathName,sizeof(PathName)-1);
    }
    xstrncpy(SizeText,MSG(MFindFileFolder),sizeof(SizeText)-1);
    sprintf(FileText,"%-50.50s  <%6.6s>",TruncPathStr(PathName,50),SizeText);
    sprintf(ListItem.Name,"%-*.*s",DlgWidth-2,DlgWidth-2,FileText);

    ffCS.Enter ();

    DWORD ItemIndex = AddFindListItem(FindData);
    if (ItemIndex != LIST_INDEX_NONE)
    {
      // Сбросим данные в FindData. Они там от файла
      memset(&FindList[ItemIndex].FindData,0,sizeof(FindList[ItemIndex].FindData));
      // Используем LastDirName, т.к. PathName уже может быть искажена
      xstrncpy(FindList[ItemIndex].FindData.cFileName, LastDirName,
              sizeof(FindList[ItemIndex].FindData.cFileName)-1);
      /* $ 07.04.2002 KM
        - Вместо пустого имени используем флаг, в противном
          случае не работает переход на каталог из списка.
          Used=0 - Имя не попададёт во временную панель.
      */
      FindList[ItemIndex].Used=0;
      /* KM $ */
      // Поставим атрибут у каталога, что-бы он не был файлом :)
      FindList[ItemIndex].FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
      if (FindFileArcIndex != LIST_INDEX_NONE)
        FindList[ItemIndex].ArcIndex = FindFileArcIndex;

      ListBox->SetUserData((void*)ItemIndex,sizeof(ItemIndex),
                           ListBox->AddItem(&ListItem));
    }

    ffCS.Leave ();
  }

  /* $ 24.03.2002 KM
     Дополнительно добавляем в список найденного
     папки. Так было в 1.65 и в 3 бете, но при
     полной переделке поиска я это где-то отломил,
     теперь возвращаю на место.
  */

  ffCS.Enter ();

  DWORD ItemIndex = AddFindListItem(FindData);
  if (ItemIndex != LIST_INDEX_NONE)
  {
    xstrncpy(FindList[ItemIndex].FindData.cFileName, FullName,
            sizeof(FindList[ItemIndex].FindData.cFileName)-1);
    /* $ 07.04.2002 KM
       Used=1 - Имя попададёт во временную панель.
    */
    FindList[ItemIndex].Used=1;
    /* KM $ */
    if (FindFileArcIndex != LIST_INDEX_NONE)
      FindList[ItemIndex].ArcIndex = FindFileArcIndex;
  }
  xstrncpy(ListItem.Name,MenuText,sizeof(ListItem.Name)-1);

  ffCS.Leave ();
  /* $ 17.01.2002 VVM
    ! Выделять будем не в структуре, а в списке. Дабы не двоилось выделение */
//  ListItem.SetSelect(!FindFileCount);

  int ListPos = ListBox->AddItem(&ListItem);
  ListBox->SetUserData((void*)ItemIndex,sizeof(ItemIndex), ListPos);
  // Выделим как положено - в списке.
  if (!FindFileCount && !FindDirCount)
    ListBox->SetSelectPos(ListPos, -1);
  /* VVM $ */

  statusCS.Enter();

  if (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    FindDirCount++;
  else
    FindFileCount++;

  statusCS.Leave();
  /* KM $ */

  LastFoundNumber++;
  FindCountReady=TRUE;

}


int FindFiles::LookForString(char *Name)
{
  FILE *SrcFile;
  char Buf[32768],SaveBuf[32768],CmpStr[sizeof(FindStr)];
  int Length,ReadSize;
  if ((Length=strlen(FindStr))==0)
    return(TRUE);
  HANDLE FileHandle=FAR_CreateFile(Name,GENERIC_READ|GENERIC_WRITE,
         FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  if (FileHandle==INVALID_HANDLE_VALUE)
    FileHandle=FAR_CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,0,NULL);
  if (FileHandle==INVALID_HANDLE_VALUE)
    return(FALSE);
  int Handle=_open_osfhandle((long)FileHandle,O_BINARY);
  if (Handle==-1)
    return(FALSE);
  if ((SrcFile=fdopen(Handle,"rb"))==NULL)
    return(FALSE);

  FILETIME LastAccess;
  int TimeRead=GetFileTime(FileHandle,NULL,&LastAccess,NULL);

  /* $ 26.10.2003 KM */
  if (SearchHex)
  {
    int LenCmpStr=sizeof(CmpStr);

    Transform((unsigned char *)CmpStr,LenCmpStr,(char *)FindStr,'S');
    Length=LenCmpStr;
  }
  else
    xstrncpy(CmpStr,FindStr,sizeof(CmpStr)-1);
  /* KM $ */

  if (!CmpCase && !SearchHex)
    LocalStrupr(CmpStr);
  /* $ 30.07.2000 KM
     Добавочные переменные
  */
  int FirstIteration=TRUE;
  /* KM $ */
  int ReverseBOM=FALSE;
  int IsFirst=FALSE;
  while (!StopSearch && (ReadSize=fread(Buf,1,sizeof(Buf),SrcFile))>0)
  {
    int DecodeTableNum=0;
    int UnicodeSearch=UseUnicode;
    int RealReadSize=ReadSize;

    /* $ 22.09.2003 KM
       Поиск по hex-кодам
    */
    if (!SearchHex)
    {
      if (UseAllTables || UseUnicode)
      {
        memcpy(SaveBuf,Buf,ReadSize);

        /* $ 21.10.2000 SVS
           Хреново получилось, в лоб так сказать, но ищет в FFFE-файлах
        */
        if(!IsFirst)
        {
          IsFirst=TRUE;
          if(*(WORD*)Buf == 0xFFFE) // The text contains the Unicode
             ReverseBOM=TRUE;       // byte-reversed byte-order mark
                                    // (Reverse BOM) 0xFFFE as its first character.
        }

        if(ReverseBOM)
        {
          BYTE Chr;
          for(int I=0; I < ReadSize; I+=2)
          {
            Chr=SaveBuf[I];
            SaveBuf[I]=SaveBuf[I+1];
            SaveBuf[I+1]=Chr;
          }
        }
        /* SVS $ */
      }
    }
    /* KM $ */

    while (1)
    {
      /* $ 22.09.2003 KM
         Поиск по hex-кодам
      */
      if (!SearchHex)
      {
        if (DecodeTableNum>0 && !UnicodeSearch)
          memcpy(Buf,SaveBuf,ReadSize);
        if (UnicodeSearch)
        {
          WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)SaveBuf,ReadSize/2,Buf,ReadSize,NULL,NULL);
          ReadSize/=2;
        }
        else
          /* $ 20.09.2003 KM
             Добавим поддержку ANSI таблицы
          */
          if (UseDecodeTable || UseANSI || DecodeTableNum>0)
            for (int I=0;I<ReadSize;I++)
              Buf[I]=TableSet.DecodeTable[Buf[I]];
        /* KM $ */
        if (!CmpCase)
          LocalUpperBuf(Buf,ReadSize);
      }
      /* KM $ */

      int CheckSize=ReadSize-Length+1;
      /* $ 30.07.2000 KM
         Обработка "Whole words" в поиске
      */
      /* $ 26.05.2002 KM
          Исправлены ошибки в поиске по целым словам.
      */
      for (int I=0;I<CheckSize;I++)
      {
        int cmpResult;
        int locResultLeft=FALSE;
        int locResultRight=FALSE;

        /* $ 22.09.2003 KM
           Поиск по hex-кодам
        */
        if (WholeWords && !SearchHex)
        {
          if (!FirstIteration)
          {
            if (IsSpace(Buf[I-1]) || IsEol(Buf[I-1]) ||
               (strchr(Opt.WordDiv,Buf[I-1])!=NULL))
              locResultLeft=TRUE;
          }
          else
          {
            FirstIteration=FALSE;
            locResultLeft=TRUE;
          }

          if (RealReadSize!=sizeof(Buf) && I+Length>=RealReadSize)
            locResultRight=TRUE;
          else
            if (I+Length<RealReadSize &&
               (IsSpace(Buf[I+Length]) || IsEol(Buf[I+Length]) ||
               (strchr(Opt.WordDiv,Buf[I+Length])!=NULL)))
              locResultRight=TRUE;
        }
        /* KM $ */
        else
        {
          locResultLeft=TRUE;
          locResultRight=TRUE;
        }

        cmpResult=locResultLeft && locResultRight && CmpStr[0]==Buf[I] &&
          (Length==1 || CmpStr[1]==Buf[I+1] &&
          (Length==2 || memcmp(CmpStr+2,&Buf[I+2],Length-2)==0));

        if (cmpResult)
        {
          if (TimeRead)
            SetFileTime(FileHandle,NULL,&LastAccess,NULL);
          fclose(SrcFile);
          return(TRUE);
        }
      }
      /* KM $ */
      /* KM $ */
      /* $ 22.09.2003 KM
         Поиск по hex-кодам
      */
      if (UseAllTables && !SearchHex)
      {
        if (PrepareTable(&TableSet,DecodeTableNum++,TRUE))
        {
          xstrncpy(CmpStr,FindStr,sizeof(CmpStr)-1);
          if (!CmpCase)
            LocalStrupr(CmpStr);
        }
        else
          if (!UnicodeSearch)
            UnicodeSearch=true;
          else
            break;
      }
      else
        break;
      /* KM $ */
    }

    if (RealReadSize==sizeof(Buf))
    {
      /* $ 22.09.2003 KM
         Поиск по hex-кодам
      */
      /* $ 30.07.2000 KM
         Изменение offset при чтении нового блока с учётом WordDiv
      */
      int NewPos;
      if (UnicodeSearch && !SearchHex)
        NewPos=ftell(SrcFile)-2*(Length+1);
      else
        NewPos=ftell(SrcFile)-(Length+1);
      fseek(SrcFile,Max(NewPos,0),SEEK_SET);
      /* KM $ */
      /* KM $ */
    }
  }
  if (TimeRead)
    SetFileTime(FileHandle,NULL,&LastAccess,NULL);
  fclose(SrcFile);
  return(FALSE);
}


#if defined(__BORLANDC__)
#pragma warn -par
#endif
void _cdecl FindFiles::PreparePluginList(void *Param)
{
  char SaveDir[NM];

  TRY {

    Sleep(200);
    *PluginSearchPath=0;
    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
    /* $ 15.10.2001 VVM */
    HANDLE hPlugin=ArcList[FindFileArcIndex].hPlugin;
    struct OpenPluginInfo Info;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    xstrncpy(SaveDir,Info.CurDir,sizeof(SaveDir)-1);
    WaitForSingleObject(hPluginMutex,INFINITE);
    if (SearchMode==SEARCH_ROOT ||
        SearchMode==SEARCH_ALL ||
        SearchMode==SEARCH_ALL_BUTNETWORK)
      CtrlObject->Plugins.SetDirectory(hPlugin,"\\",OPM_FIND);
    ReleaseMutex(hPluginMutex);
    RecurseLevel=0;
    ScanPluginTree(hPlugin,ArcList[FindFileArcIndex].Flags);
    /* VVM $ */
    WaitForSingleObject(hPluginMutex,INFINITE);
    if (SearchMode==SEARCH_ROOT ||
        SearchMode==SEARCH_ALL ||
        SearchMode==SEARCH_ALL_BUTNETWORK)
      CtrlObject->Plugins.SetDirectory(hPlugin,SaveDir,OPM_FIND);
    ReleaseMutex(hPluginMutex);
    while (!StopSearch && FindMessageReady)
      Sleep(10);
    if (Param==NULL)
    {
      statusCS.Enter();

      sprintf(FindMessage,MSG(MFindDone),FindFileCount,FindDirCount);
      FindMessageReady=TRUE;
      SearchDone=TRUE;

      statusCS.Leave();
    }
  }
  EXCEPT (xfilter((int)INVALID_HANDLE_VALUE,GetExceptionInformation(),NULL,1))
  {
    TerminateProcess( GetCurrentProcess(), 1);
  }
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif

void FindFiles::ScanPluginTree(HANDLE hPlugin, DWORD Flags)
{
  PluginPanelItem *PanelData=NULL;
  int ItemCount=0;

  WaitForSingleObject(hPluginMutex,INFINITE);
  if (StopSearch || !CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&ItemCount,OPM_FIND))
  {
    ReleaseMutex(hPluginMutex);
    return;
  }
  else
    ReleaseMutex(hPluginMutex);
  RecurseLevel++;

  /* $ 19.01.2003 KM
     Отключу пока сортировку, что-то результаты получаются
     не совсем те, которые я ожидал.
  */
  /* $ 24.03.2002 KM
     Сортировку в плагинах по именам папок делаем в случае,
     если имя файла содержит в себе путь, а не только при
     OPIF_REALNAMES, в противном случае в результатах поиска
     получается некрасивый визуальный эффект разбросанности
     одинаковых папок по списку.
  */
//  if (PanelData && strlen(PointToName(PanelData->FindData.cFileName))>0)
//    far_qsort((void *)PanelData,ItemCount,sizeof(*PanelData),SortItems);
  /* KM $ */
  /* KM $ */

  if (SearchMode!=SEARCH_SELECTED || RecurseLevel!=1)
  {
    for (int I=0;I<ItemCount && !StopSearch;I++)
    {
      while (PauseSearch)
        Sleep(10);

      PluginPanelItem *CurPanelItem=PanelData+I;
      char *CurName=CurPanelItem->FindData.cFileName;
      char FullName[2*NM];
      if (strcmp(CurName,".")==0 || TestParentFolderName(CurName))
        continue;
//      char AddPath[2*NM];
      if (Flags & OPIF_REALNAMES)
      {
        xstrncpy(FullName,CurName,sizeof(FullName)-1);
//        strcpy(AddPath,CurName);
//        *PointToName(AddPath)=0;
      }
      else
      {
        sprintf(FullName,"%s%s",PluginSearchPath,CurName);
//        strcpy(AddPath,PluginSearchPath);
      }

      /* $ 30.09.2003 KM
        Отфильтруем файлы не попадающие в действующий фильтр
      */
      int IsFile;
      if (UseFilter)
        IsFile=Filter->FileInFilter(&CurPanelItem->FindData);
      else
        IsFile=TRUE;

      if (IsFile)
      {
        /* $ 14.06.2004 KM
          Уточнение действия при обработке каталогов
        */
        if (CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          statusCS.Enter();

          xstrncpy(FindMessage,FullName,sizeof(FindMessage)-1);
          FindMessage[sizeof(FindMessage)-1]=0;
          RemovePseudoBackSlash(FindMessage);
          FindMessageReady=TRUE;

          statusCS.Leave();

        }
        /* KM $ */

        if (IsFileIncluded(CurPanelItem,CurName,CurPanelItem->FindData.dwFileAttributes))
          AddMenuRecord(FullName,&CurPanelItem->FindData);

        if (SearchInArchives && (hPlugin != INVALID_HANDLE_VALUE) && (Flags & OPIF_REALNAMES))
          ArchiveSearch(FullName);
      }
      /* KM $ */
    }
  }
  if (SearchMode!=SEARCH_CURRENT_ONLY)
  {
    for (int I=0;I<ItemCount && !StopSearch;I++)
    {
      PluginPanelItem *CurPanelItem=PanelData+I;
      char *CurName=CurPanelItem->FindData.cFileName;
      if ((CurPanelItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
          strcmp(CurName,".")!=0 && !TestParentFolderName(CurName) &&
          (SearchMode!=SEARCH_SELECTED || RecurseLevel!=1 ||
          CtrlObject->Cp()->ActivePanel->IsSelected(CurName)))
      {
        WaitForSingleObject(hPluginMutex,INFINITE);
        if (strchr(CurName,'\x1')==NULL && CtrlObject->Plugins.SetDirectory(hPlugin,CurName,OPM_FIND))
        {
          ReleaseMutex(hPluginMutex);
          /* $ 19.01.2003 KM
             Хотя здесь и шла проверка не переполнение
             PluginSearchPath, но бывает дописывалась только
             часть в пути в конец переменной, что не есть гуд.
          */
          int SearchPathLen=strlen(PluginSearchPath);
          int CurNameLen=strlen(CurName);
          if (SearchPathLen+CurNameLen<NM-2)
          {
            strcat(PluginSearchPath,CurName);
            strcat(PluginSearchPath,"\x1");
            ScanPluginTree(hPlugin, Flags);
            char *Ptr=strrchr(PluginSearchPath,'\x1');
            if (Ptr!=NULL)
              *Ptr=0;
          }
          /* KM $ */
          char *NamePtr=strrchr(PluginSearchPath,'\x1');
          if (NamePtr!=NULL)
            *(NamePtr+1)=0;
          else
            *PluginSearchPath=0;
          WaitForSingleObject(hPluginMutex,INFINITE);
          if (!CtrlObject->Plugins.SetDirectory(hPlugin,"..",OPM_FIND))
            StopSearch=TRUE;
          ReleaseMutex(hPluginMutex);
          if (StopSearch) break;
        }
        else
          ReleaseMutex(hPluginMutex);
      }
    }
  }
  CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,ItemCount);
  RecurseLevel--;
}

void _cdecl FindFiles::WriteDialogData(void *Param)
{
  FarDialogItemData ItemData;
  char DataStr[NM];

  WriteDataUsed=TRUE;

  while( true )
  {
    Dialog* Dlg=(Dialog*)hDlg;

    if( !Dlg )
      break;

    VMenu *ListBox=Dlg->Item[1].ListPtr;

    if (ListBox && !PauseSearch && !ScreenSaverActive)
    {
      if (BreakMainThread)
        StopSearch=TRUE;

      if (FindCountReady)
      {
        statusCS.Enter ();

        sprintf(DataStr," %s: %d ",MSG(MFindFound),FindFileCount+FindDirCount);
        ItemData.PtrData=DataStr;
        ItemData.PtrLength=strlen(DataStr);

        Dialog::SendDlgMessage(hDlg,DM_SETTEXT,2,(long)&ItemData);

        FindCountReady=FALSE;

        statusCS.Leave ();
      }

      if (FindMessageReady)
      {
        statusCS.Enter ();

        char SearchStr[NM];

        if (*FindStr)
        {
          char Temp[NM],FStr[NM*2];

          xstrncpy(FStr,FindStr,sizeof(FStr)-1);
          sprintf(Temp," \"%s\"",TruncStrFromEnd(FStr,10));
          sprintf(SearchStr,MSG(MFindSearchingIn),Temp);
        }
        else
          sprintf(SearchStr,MSG(MFindSearchingIn),"");

        int Wid1=strlen(SearchStr);
        int Wid2=DlgWidth-strlen(SearchStr)-1;

        if (SearchDone)
        {
          Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

          xstrncpy(DataStr,MSG(MFindCancel),sizeof(DataStr)-1);
          ItemData.PtrData=DataStr;
          ItemData.PtrLength=strlen(DataStr);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXT,9,(long)&ItemData);

          sprintf(DataStr,"%-*.*s",DlgWidth,DlgWidth,FindMessage);
          ItemData.PtrData=DataStr;
          ItemData.PtrLength=strlen(DataStr);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXT,3,(long)&ItemData);

          ItemData.PtrData="";
          ItemData.PtrLength=strlen(DataStr);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXT,2,(long)&ItemData);

          Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
          SetFarTitle(FindMessage);
          StopSearch=TRUE;
        }
        else
        {
          sprintf(DataStr,"%-*.*s %-*.*s",Wid1,Wid1,SearchStr,Wid2,Wid2,TruncPathStr(FindMessage,Wid2));
          ItemData.PtrData=DataStr;
          ItemData.PtrLength=strlen(DataStr);
          Dialog::SendDlgMessage(hDlg,DM_SETTEXT,3,(long)&ItemData);
        }

        FindMessageReady=FALSE;

        statusCS.Leave ();
      }

      if (LastFoundNumber && ListBox)
      {
        LastFoundNumber=0;

        if ( ListBox->UpdateRequired () )
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,1,1);
      }
    }

    if (StopSearch && SearchDone && !FindMessageReady && !FindCountReady && !LastFoundNumber)
        break;

    Sleep(20);
  }

  WriteDataUsed=FALSE;
}

BOOL FindFiles::FindListGrow()
{
  DWORD Delta = (FindListCapacity < 256)?LIST_DELTA:FindListCapacity/2;
  LPFINDLIST NewList = (LPFINDLIST)xf_realloc(FindList, (FindListCapacity + Delta) * sizeof(FINDLIST));
  if (NewList)
  {
    FindList = NewList;
    FindListCapacity+= Delta;
    return(TRUE);
  }
  return(FALSE);
}

BOOL FindFiles::ArcListGrow()
{
  DWORD Delta = (ArcListCapacity < 256)?LIST_DELTA:ArcListCapacity/2;
  LPARCLIST NewList = (LPARCLIST)xf_realloc(ArcList, (ArcListCapacity + Delta) * sizeof(ARCLIST));
  if (NewList)
  {
    ArcList = NewList;
    ArcListCapacity+= Delta;
    return(TRUE);
  }
  return(FALSE);
}

DWORD FindFiles::AddFindListItem(WIN32_FIND_DATA *FindData)
{
  if ((FindListCount == FindListCapacity) &&
      (!FindListGrow()))
    return(LIST_INDEX_NONE);
  FindList[FindListCount].FindData = *FindData;
  FindList[FindListCount].ArcIndex = LIST_INDEX_NONE;
  return(FindListCount++);
}

DWORD FindFiles::AddArcListItem(const char *ArcName, HANDLE hPlugin,
                                DWORD dwFlags, const char *RootPath)
{
  if ((ArcListCount == ArcListCapacity) &&
      (!ArcListGrow()))
    return(LIST_INDEX_NONE);
  xstrncpy(ArcList[ArcListCount].ArcName, NullToEmpty(ArcName),
          sizeof(ArcList[ArcListCount].ArcName)-1);
  ArcList[ArcListCount].hPlugin = hPlugin;
  ArcList[ArcListCount].Flags = dwFlags;
  xstrncpy(ArcList[ArcListCount].RootPath, NullToEmpty(RootPath),
          sizeof(ArcList[ArcListCount].RootPath)-1);
  AddEndSlash(ArcList[ArcListCount].RootPath);
  return(ArcListCount++);
}

void FindFiles::ClearAllLists()
{
  CriticalSectionLock Lock(ffCS);

  FindFileArcIndex = LIST_INDEX_NONE;

  if (FindList)
    xf_free(FindList);
  FindList = NULL;
  FindListCapacity = FindListCount = 0;

  if (ArcList)
    xf_free(ArcList);
  ArcList = NULL;
  ArcListCapacity = ArcListCount = 0;
}

char *FindFiles::PrepareDriveNameStr(char *SearchFromRoot,size_t sz)
{
  char CurDir[NM*2],DriveName[64];

  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  PluginMode=ActivePanel->GetMode()==PLUGIN_PANEL && ActivePanel->IsVisible();

  CtrlObject->CmdLine->GetCurDir(CurDir);
  GetPathRootOne(CurDir,CurDir);
  if (CurDir[strlen(CurDir)-1]=='\\')
    CurDir[strlen(CurDir)-1]=0;

  if (*CurDir==0 || PluginMode)
  {
    xstrncpy(SearchFromRoot,MSG(MSearchFromRoot),sz-1);
  }
  else
  {
    sprintf(DriveName,MSG(MSearchFromRootOfDrive),CurDir);
    sprintf(SearchFromRoot,"%s %s",MSG(MSearchFromRoot),DriveName);
  }
  SearchFromRoot[sz-1]=0;

  if (strlen(SearchFromRoot)>DLG_WIDTH-10)
    TruncStrFromEnd(SearchFromRoot,DLG_WIDTH-10);

  return SearchFromRoot;
}

char *FindFiles::RemovePseudoBackSlash(char *FileName)
{
  for (int i=0;FileName[i]!=0;i++)
  {
    if (FileName[i]=='\x1')
      FileName[i]='\\';
  }
  return FileName;
}
