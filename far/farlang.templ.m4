m4_include(`farversion.m4')m4_dnl
#hpp file name
lang.hpp
#number of languages
2
#id:0 language file name, language name, language description
FarRus.lng Russian "Russian (Русский)"
#id:1 language file name, language name, language description
FarEng.lng English "English"

#head of the hpp file
hhead:#ifndef __FARLANG_HPP__
hhead:#define __FARLANG_HPP__

#tail of the hpp file
htail:
htail:#endif  // __FARLANG_HPP__
#и так сколько нужно

#--------------------------------------------------------------------
#теперь идут сами lng фиды
#--------------------------------------------------------------------
#первым идёт имя элемнта в enum а перед ним могут идти коменты для hpp
#файла
#h://коммент до записи самого MShareware
#he://коммент после записи самого MShareware
#MShareware
#теперь идут lng строки для всех языков по очереди а перед ними коменты
#l://это запишеться во все lng файлы перед строкой текста
#le://это запишеться во все lng файлы после строки текста
#ls://это только перед рус строкой
#lse://это только после рус строки
#"Пожалуйста, зарегистрируйте Вашу копию"
#ls://это только перед енг строкой
#lse://это только после енг строки
#"Evaluation copy, please register."

MShareware=0
`l://Version: 'MAJOR`.'MINOR` build 'BUILD
l:
"Пожалуйста, зарегистрируйте Вашу копию"
"Evaluation copy, please register."

MRegistered
"Зарегистрирован"
"Registered to"

MYes
"Да"
"Yes"

MNo
"Нет"
"No"

MOk
"Продолжить"
"OK"

MHYes
l:
"&Да"
"&Yes"

MHNo
"&Нет"
"&No"

MHOk
"&Продолжить"
"&OK"

MCancel
l:
"Отменить"
"Cancel"

MRetry
"Повторить"
"Retry"

MSkip
"Пропустить"
"Skip"

MAbort
"Прервать"
"Abort"

MIgnore
"Игнорировать"
"Ignore"

MDelete
"Удалить"
"Delete"

MSplit
"Разделить"
"Split"

MRemove
"Удалить"
"Remove"

MHCancel
l:
"&Отменить"
"&Cancel"

MHRetry
"&Повторить"
"&Retry"

MHSkip
"П&ропустить"
"&Skip"

MHSkipAll
"Пропустить &все"
"S&kip all"

MHAbort
"Прер&вать"
"&Abort"

MHIgnore
"&Игнорировать"
"&Ignore"

MHDelete
"&Удалить"
"&Delete"

MHRemove
"&Удалить"
"R&emove"

MHSplit
"Раз&делить"
"Sp&lit"

MWarning
l:
"Предупреждение"
"Warning"

MError
"Ошибка"
"Error"

MQuit
l:
"Выход"
"Quit"

MAskQuit
"Вы хотите завершить работу в FAR?"
"Do you want to quit FAR?"

MF1
l:
l://functional keys - 6 characters max
"Помощь"
"Help"

MF2
"ПользМ"
"UserMn"

MF3
"Просм"
"View"

MF4
"Редакт"
"Edit"

MF5
"Копир"
"Copy"

MF6
"Перен"
"RenMov"

MF7
"Папка"
"MkFold"

MF8
"Удален"
"Delete"

MF9
"КонфМн"
"ConfMn"

MF10
"Выход"
"Quit"

MF11
"Модули"
"Plugin"

MF12
"Экраны"
"Screen"

MAltF1
l:
"Левая"
"Left"

MAltF2
"Правая"
"Right"

MAltF3
"Смотр."
"View.."

MAltF4
"Редак."
"Edit.."

MAltF5
"Печать"
"Print"

MAltF6
"Ссылка"
"MkLink"

MAltF7
"Искать"
"Find"

MAltF8
"Истор"
"Histry"

MAltF9
"Видео"
"Video"

MAltF10
"Дерево"
"Tree"

MAltF11
"ИстПр"
"ViewHs"

MAltF12
"ИстПап"
"FoldHs"

MCtrlF1
l:
"Левая"
"Left"

MCtrlF2
"Правая"
"Right"

MCtrlF3
"Имя   "
"Name  "

MCtrlF4
"Расшир"
"Extens"

MCtrlF5
"Модиф"
"Modifn"

MCtrlF6
"Размер"
"Size"

MCtrlF7
"Несорт"
"Unsort"

MCtrlF8
"Создан"
"Creatn"

MCtrlF9
"Доступ"
"Access"

MCtrlF10
"Описан"
"Descr"

MCtrlF11
"Владел"
"Owner"

MCtrlF12
"Сорт"
"Sort"

MShiftF1
l:
"Добавл"
"Add"

MShiftF2
"Распак"
"Extrct"

MShiftF3
"АрхКом"
"ArcCmd"

MShiftF4
"Редак."
"Edit.."

MShiftF5
"Копир"
"Copy"

MShiftF6
"Переим"
"Rename"

MShiftF7
""
""

MShiftF8
"Удален"
"Delete"

MShiftF9
"Сохран"
"Save"

MShiftF10
"Послдн"
"Last"

MShiftF11
"Группы"
"Group"

MShiftF12
"Выбран"
"SelUp"

MAltShiftF1
l:
l:// Main AltShift
""
""

MAltShiftF2
""
""

MAltShiftF3
""
""

MAltShiftF4
""
""

MAltShiftF5
""
""

MAltShiftF6
""
""

MAltShiftF7
""
""

MAltShiftF8
""
""

MAltShiftF9
"КонфПл"
"ConfPl"

MAltShiftF10
""
""

MAltShiftF11
""
""

MAltShiftF12
""
""

MCtrlShiftF1
l:
l://Main CtrlShift
""
""

MCtrlShiftF2
""
""

MCtrlShiftF3
"Просм"
"View"

MCtrlShiftF4
"Редакт"
"Edit"

MCtrlShiftF5
""
""

MCtrlShiftF6
""
""

MCtrlShiftF7
""
""

MCtrlShiftF8
""
""

MCtrlShiftF9
""
""

MCtrlShiftF10
""
""

MCtrlShiftF11
""
""

MCtrlShiftF12
""
""

MCtrlAltF1
l:
l:// Main CtrlAlt
""
""

MCtrlAltF2
""
""

MCtrlAltF3
""
""

MCtrlAltF4
""
""

MCtrlAltF5
""
""

MCtrlAltF6
""
""

MCtrlAltF7
""
""

MCtrlAltF8
""
""

MCtrlAltF9
""
""

MCtrlAltF10
""
""

MCtrlAltF11
""
""

MCtrlAltF12
""
""

MCtrlAltShiftF1
l:
l:// Main CtrlAltShift
""
""

MCtrlAltShiftF2
""
""

MCtrlAltShiftF3
""
""

MCtrlAltShiftF4
""
""

MCtrlAltShiftF5
""
""

MCtrlAltShiftF6
""
""

MCtrlAltShiftF7
""
""

MCtrlAltShiftF8
""
""

MCtrlAltShiftF9
""
""

MCtrlAltShiftF10
""
""

MCtrlAltShiftF11
""
""

MCtrlAltShiftF12
le://End of functional keys
""
""

MHistoryTitle
l:
"История команд"
"History"

MFolderHistoryTitle
"История папок"
"Folders history"

MViewHistoryTitle
"История просмотра"
"File view history"

MViewHistoryIsCreate
"Создать файл?"
"Create file?"

MHistoryView
"Просмотр"
"View"

MHistoryEdit
"Редактор"
"Edit"

MHistoryExt
"Внешний "
"Ext."

MHistoryClear
l:
"История будет полностью очищена. Продолжить?"
"All records in the history will be deleted. Continue?"

MClear
"&Очистить"
"&Clear history"

MConfigSystemTitle
l:
"Системные параметры"
"System settings"

MConfigRO
"&Снимать атрибут R/O c CD файлов"
"&Clear R/O attribute from CD files"

MConfigRecycleBin
"Удалять в &Корзину"
"&Delete to Recycle Bin"

MConfigRecycleBinLink
"У&далять символические ссылки"
"Delete symbolic &links"

MConfigSystemCopy
"Использовать систе&мную функцию копирования"
"Use sys&tem copy routine"

MConfigCopySharing
"Копировать открытые для &записи файлы"
"Copy files opened for &writing"

MConfigScanJunction
"Ск&анировать символические ссылки"
"Scan s&ymbolic links"

MConfigCreateUppercaseFolders
"Создавать &папки заглавными буквами"
"Create folders in &uppercase"

MConfigInactivity
"&Время бездействия"
"&Inactivity time"

MConfigInactivityMinutes
"минут"
"minutes"

MConfigSaveHistory
"Сохранять &историю команд"
"Save commands &history"

MConfigSaveFoldersHistory
"Сохранять историю п&апок"
"Save &folders history"

MConfigSaveViewHistory
"Сохранять историю п&росмотра и редактора"
"Save &view and edit history"

MConfigRegisteredTypes
"Использовать стандартные &типы файлов"
"Use Windows &registered types"

MConfigCloseCDGate
"Автоматически монтироват&ь CDROM"
"CD drive auto &mount"

MConfigPersonalPath
"Путь к персональным п&лагинам:"
"&Path for personal plugins:"

MConfigAutoSave
"Автозапись кон&фигурации"
"Auto &save setup"

MConfigPanelTitle
l:
"Настройки панели"
"Panel settings"

MConfigHidden
"Показывать скр&ытые и системные файлы"
"Show &hidden and system files"

MConfigHighlight
"&Раскраска файлов"
"Hi&ghlight files"

MConfigAutoChange
"&Автосмена папки"
"&Auto change folder"

MConfigSelectFolders
"Пометка &папок"
"Select &folders"

MConfigSortFolderExt
"Сортировать имена папок по рас&ширению"
"Sort folder names by e&xtension"

MConfigReverseSort
"Разрешить &обратную сортировку"
"Allow re&verse sort modes"

MConfigAutoUpdateLimit
"Отключать автооб&новление панелей,"
"&Disable automatic update of panels"

MConfigAutoUpdateLimit2
"если объектов больше"
"if object count exceeds"

MConfigAutoUpdateRemoteDrive
"Автообновление с&етевых дисков"
"Network drives autor&efresh"

MConfigShowColumns
"Показывать &заголовки колонок"
"Show &column titles"

MConfigShowStatus
"Показывать &строку статуса"
"Show &status line"

MConfigShowTotal
"Показывать су&ммарную информацию"
"Show files &total information"

MConfigShowFree
"Показывать с&вободное место"
"Show f&ree size"

MConfigShowScrollbar
"Показывать по&лосу прокрутки"
"Show scroll&bar"

MConfigShowScreensNumber
"Показывать количество &фоновых экранов"
"Show background screens &number"

MConfigShowSortMode
"Показывать букву режима сор&тировки"
"Show sort &mode letter"

MConfigInterfaceTitle
l:
"Настройки интерфейса"
"Interface settings"

MConfigClock
"&Часы в панелях"
"&Clock in panels"

MConfigViewerEditorClock
"Ч&асы при редактировании и просмотре"
"C&lock in viewer and editor"

MConfigMouse
"Мы&шь"
"M&ouse"

MConfigKeyBar
"Показывать &линейку клавиш"
"Show &key bar"

MConfigMenuBar
"Всегда показывать &меню"
"Always show &menu bar"

MConfigSaver
"&Сохранение экрана"
"&Screen saver"

MConfigSaverMinutes
"минут"
"minutes"

MConfigUsePromptFormat
"Установить &формат командной строки"
"Set command line &prompt format"

MConfigAltGr
"Использовать &правый Alt как AltGr"
"Use right Alt as &AltGr"

MConfigCopyTotal
"Показывать &общий индикатор копирования"
"Show &total copy progress indicator"

MConfigCopyTimeRule
"Показывать информацию о времени &копирования"
"Show cop&ying time information"

MConfigPgUpChangeDisk
"Использовать Ctrl-PgUp для в&ыбора диска"
"Use Ctrl-Pg&Up to change drive"

MConfigDlgSetsTitle
l:
"Настройки диалогов"
"Dialog settings"

MConfigDialogsEditHistory
"&История в строках ввода диалогов"
"&History in dialog edit controls"

MConfigDialogsEditBlock
"&Постоянные блоки в строках ввода"
"&Persistent blocks in edit controls"

MConfigDialogsDelRemovesBlocks
"Del удаляет б&локи в строках ввода"
"&Del removes blocks in edit controls"

MConfigDialogsAutoComplete
"&Автозавершение в строках ввода"
"&AutoComplete in edit controls"

MConfigDialogsEULBsClear
"Backspace &удаляет неизмененный текст"
"&Backspace deletes unchanged text"

MConfigDialogsMouseButton
"Клик мыши &вне диалога закрывает диалог"
"Mouse click &outside a dialog closes it"

MViewConfigTitle
l:
"Программа просмотра"
"Viewer"

MViewConfigExternal
"Внешняя программа просмотра:"
"External viewer:"

MViewConfigExternalF3
"Запускать по F3"
"Use for F3"

MViewConfigExternalAltF3
"Запускать по Alt-F3"
"Use for Alt-F3"

MViewConfigExternalCommand
"&Команда просмотра:"
"&Viewer command:"

MViewConfigInternal
"Встроенная программа просмотра:"
"Internal viewer:"

MViewConfigSavePos
"&Сохранять позицию файла"
"&Save file position"

MViewConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"

MViewAutoDetectTable
"&Автоопределение таблицы символов"
"&Autodetect character table"

MViewConfigTabSize
"Раз&мер табуляции"
"Tab si&ze"

MViewConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"

MViewConfigArrows
"Показывать стрелки с&двига"
"Show scrolling arro&ws"

MViewConfigPersistentSelection
"Постоянное &выделение"
"&Persistent selection"

MViewConfigAnsiTableAsDefault
"&Изначально открывать файлы в WIN кодировке"
"&Initially open files in WIN encoding"

MEditConfigTitle
l:
"Редактор"
"Editor"

MEditConfigExternal
"Внешний редактор:"
"External editor:"

MEditConfigEditorF4
"Запускать по F4"
"Use for F4"

MEditConfigEditorAltF4
"Запускать по Alt-F4"
"Use for Alt-F4"

MEditConfigEditorCommand
"&Команда редактирования:"
"&Editor command:"

MEditConfigInternal
"Встроенный редактор:"
"Internal editor:"

MEditConfigExpandTabsTitle
"Преобразовывать &табуляцию:"
"Expand &tabs:"

MEditConfigDoNotExpandTabs
l:
"Не преобразовывать табуляцию"
"Do not expand tabs"

MEditConfigExpandTabs
"Преобразовывать новые символы табуляции в пробелы"
"Expand newly entered tabs to spaces"

MEditConfigConvertAllTabsToSpaces
"Преобразовывать все символы табуляции в пробелы"
"Expand all tabs to spaces"

MEditConfigPersistentBlocks
"&Постоянные блоки"
"&Persistent blocks"

MEditConfigDelRemovesBlocks
l:
"Del удаляет б&локи"
"&Del removes blocks"

MEditConfigAutoIndent
"Авто&отступ"
"Auto &indent"

MEditConfigSavePos
"&Сохранять позицию файла"
"&Save file position"

MEditConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"

MEditAutoDetectTable
"&Автоопределение таблицы символов"
"&Autodetect character table"

MEditCursorBeyondEnd
"Ку&рсор за пределами строки"
"&Cursor beyond end of line"

MEditLockROFileModification
"Блокировать р&едактирование файлов с атрибутом R/O"
"Lock editing of read-only &files"

MEditWarningBeforeOpenROFile
"Пре&дупреждать при открытии файла с атрибутом R/O"
"&Warn when opening read-only files"

MEditConfigTabSize
"Раз&мер табуляции"
"Tab si&ze"

MEditConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"

MEditConfigAnsiTableAsDefault
"&Изначально открывать файлы в WIN кодировке"
"I&nitially open files in WIN encoding"

MEditConfigAnsiTableForNewFile
"Созда&вать новые файлы в WIN кодировке"
"C&reate new files in WIN encoding"

MDistributionTableWasNotFound
l:
"Таблица с распределением частот символов не обнаружена!"
"Table with the character frequency distribution was not found!"

MAutoDetectWillNotWork
"Опция \"Автоопределение таблицы символов\" отключена."
"Option \"Autodetect character table\" is off."

MSaveSetupTitle
l:
"Конфигурация"
"Save setup"

MSaveSetupAsk1
"Вы хотите сохранить"
"Do you wish to save"

MSaveSetupAsk2
"текущую конфигурацию?"
"current setup?"

MSaveSetup
"Сохранить"
"Save"

MCopyDlgTitle
l:
"Копирование"
"Copy"

MMoveDlgTitle
"Переименование/Перенос"
"Rename/Move"

MLinkDlgTitle
"Ссылка"
"Link"

MCopySecurity
"П&рава доступа:"
"&Access rights:"

MCopySecurityCopy
"Копироват&ь"
"Co&py"

MCopySecurityInherit
"Нас&ледовать"
"&Inherit"

MCopySecurityLeave
"По умол&чанию"
"Defau&lt"

MCopyIfFileExist
"Уже су&ществующие файлы:"
"Already e&xisting files:"

MCopyAsk
"&Запрос действия"
"&Ask"

MCopyAskRO
"Запрос подтверждения для &R/O файлов"
"Also ask on &R/O files"

MCopyOnlyNewerFiles
"Только &новые/обновленные файлы"
"Only ne&wer file(s)"

MLinkType
"&Тип ссылки:"
"Link t&ype:"

MLinkTypeJunction
"&связь каталогов"
"directory &junction"

MLinkTypeHardlink
"&жёсткая ссылка"
"&hard link"

MLinkTypeSymlinkFile
"символическая ссылка (&файл)"
"symbolic link (&file)"

MLinkTypeSymlinkDirectory
"символическая ссылка (&папка)"
"symbolic link (fol&der)"

MCopySymLinkContents
"Копировать содерж&имое символических ссылок"
"Cop&y contents of symbolic links"

MCopyMultiActions
"Обр&абатывать несколько имен файлов"
"Process &multiple destinations"

MCopyDlgCopy
"&Копировать"
"&Copy"

MCopyDlgTree
"F10-&Дерево"
"F10-&Tree"

MCopyDlgCancel
"&Отменить"
"Ca&ncel"

MCopyDlgRename
"&Переименовать"
"&Rename"

MCopyDlgLink
"&Создать ссылку"
"&Link"

MCopyDlgTotal
"Всего"
"Total"

MCopyScanning
"Сканирование папок..."
"Scanning folders..."

MCopyPrepareSecury
"Применение прав доступа..."
"Applying access rights..."

MCopyUseFilter
"Исполь&зовать фильтр"
"&Use filter"

MCopySetFilter
"&Фильтр"
"Filt&er"

MCopyFile
l:
"Копировать \"%.55s\""
"Copy \"%.55s\""

MMoveFile
"Переименовать или перенести \"%.55s\""
"Rename or move \"%.55s\""

MLinkFile
"Создать ссылку \"%.55s\""
"Link \"%.55s\""

MCopyFiles
"Копировать %d элемент%s"
"Copy %d item%s"

MMoveFiles
"Переименовать или перенести %d элемент%s"
"Rename or move %d item%s"

MLinkFiles
"Создать ссылки %d элемент%s"
"Link %d item%s"

MCMLTargetTO
" &в:"
" t&o:"

MCMLItems0
""
""

MCMLItemsA
"а"
"s"

MCMLItemsS
"ов"
"s"

MCopyIncorrectTargetList
l:
"Указан некорректный список целей!"
"Incorrect target list!"

MCopyNotSupportLink1
l:
"Функция создания ссылок"
"The link creation feature"

MCopyNotSupportLink2
"доступна только в семействе Windows NT"
"is available only under Windows NT"

MCopyCopyingTitle
l:
"Копирование"
"Copying"

MCopyMovingTitle
"Перенос"
"Moving"

MCopyCannotFind
l:
"Файл не найден"
"Cannot find the file"

MCannotCopyFolderToItself1
l:
"Нельзя копировать папку"
"Cannot copy the folder"

MCannotCopyFolderToItself2
"в саму себя"
"onto itself"

MCannotCopyToTwoDot
l:
"Нельзя копировать файл или папку"
"You may not copy files or folders"

MCannotMoveToTwoDot
"Нельзя перемещать файл или папку"
"You may not move files or folders"

MCannotCopyMoveToTwoDot
"выше корневого каталога"
"higher than the root folder"

MCopyCannotCreateFolder
l:
"Ошибка создания папки"
"Cannot create the folder"

MCopyCannotChangeFolderAttr
"Невозможно установить атрибуты для папки"
"Failed to set folder attributes"

MCopyCannotRenameFolder
"Невозможно переименовать папку"
"Cannot rename the folder"

MCopyIgnore
"&Игнорировать"
"&Ignore"

MCopyIgnoreAll
"Игнорировать &все"
"Ignore &All"

MCopyRetry
"&Повторить"
"&Retry"

MCopySkip
"П&ропустить"
"&Skip"

MCopySkipAll
"&Пропустить все"
"S&kip all"

MCopyCancel
"&Отменить"
"&Cancel"

MCopyDecrypt
"Рас&шифровать"
"&Decrypt"

MCopyDecryptAll
"&Все"
"Decrypt &all"

MCopyCannotCreateLink
l:
"Ошибка создания ссылки"
"Cannot create the link"

MCopyFolderNotEmpty
"Папка назначения должна быть пустой"
"Target folder must be empty"

MCopyCannotCreateJunctionToFile
"Невозможно создать связь. Файл уже существует:"
"Cannot create junction. The file already exists:"

MCopyCannotCreateVolMount
l:
"Ошибка монтирования диска"
"Volume mount points error"

MCopyRetrVolFailed
"Невозможно получить информацию о '%s'"
"Retrieving volume name for '%s' failed"

MCopyMountVolFailed
"Ошибка при монтировании диска '%s'"
"Attempt to volume mount '%s'"

MCopyMountVolFailed2
"на '%s'"
"at '%s' failed"

MCopyCannotSupportVolMount
"Функция монтирования дисков не поддерживается"
"Volume mounting is not supported"

MCopyMountName
"disk_%c"
"Disk_%c"

MCannotCopyFileToItself1
l:
"Нельзя копировать файл"
"Cannot copy the file"

MCannotCopyFileToItself2
"в самого себя"
"onto itself"

MCopyStream1
l:
"Исходный файл содержит более одного потока данных,"
"The source file contains more than one data stream."

MCopyStream2
"но вы не используете системную функцию копирования."
"but since you do not use a system copy routine."

MCopyStream3
"но том назначения не поддерживает этой возможности."
"but the destination volume does not support this feature."

MCopyStream4
"Часть сведений не будет сохранена."
"Some data will not be preserved as a result."

MCopyFileExist
l:
"Файл уже существует"
"File already exists"

MCopySource
"&Новый"
"&New"

MCopyDest
"Су&ществующий"
"E&xisting"

MCopyOverwrite
"В&место"
"&Overwrite"

MCopyContinue
"П&родолжить"
"C&ontinue"

MCopySkipOvr
"&Пропустить"
"&Skip"

MCopyAppend
"&Дописать"
"A&ppend"

MCopyResume
"Возоб&новить"
"&Resume"

MCopyCancelOvr
"&Отменить"
"&Cancel"

MCopyRememberChoice
"&Запомнить выбор"
"&Remember choice"

MCopyFileRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"

MCopyAskDelete
"Вы хотите удалить его?"
"Do you wish to delete it?"

MCopyDeleteRO
"&Удалить"
"&Delete"

MCopyDeleteAllRO
"&Все"
"&All"

MCopySkipRO
"&Пропустить"
"&Skip"

MCopySkipAllRO
"П&ропустить все"
"S&kip all"

MCopyCancelRO
"&Отменить"
"&Cancel"

MCannotCopy
l:
"Ошибка копирования"
"Cannot copy"

MCannotMove
"Ошибка переноса"
"Cannot move"

MCannotLink
"Ошибка создания ссылки"
"Cannot link"

MCannotCopyTo
"в"
"to"

MCopyEncryptWarn1
"Файл"
"The file"

MCopyEncryptWarn2
"нельзя скопировать или переместить, не потеряв его шифрование."
"cannot be copied or moved without losing its encryption."

MCopyEncryptWarn3
"Можно пропустить эту ошибку или отменить операцию."
"You can choose to ignore this error and continue, or cancel."

MCopyReadError
l:
"Ошибка чтения данных из"
"Cannot read data from"

MCopyWriteError
"Ошибка записи данных в"
"Cannot write data to"

MCopyProcessed
l:
"Обработано файлов: %d"
"Files processed: %d"

MCopyProcessedTotal
"Обработано файлов: %d из %d"
"Files processed: %d of %d"

MCopyMoving
"Перенос файла"
"Moving the file"

MCopyCopying
"Копирование файла"
"Copying the file"

MCopyTo
"в"
"to"

MCopyErrorDiskFull
l:
"Диск заполнен. Вставьте следующий"
"Disk full. Insert next"

MDeleteTitle
l:
"Удаление"
"Delete"

MAskDeleteFolder
"Вы хотите удалить папку"
"Do you wish to delete the folder"

MAskDeleteFile
"Вы хотите удалить файл"
"Do you wish to delete the file"

MAskDelete
"Вы хотите удалить"
"Do you wish to delete"

MAskDeleteRecycleFolder
"Вы хотите поместить в Корзину папку"
"Do you wish to move to the Recycle Bin the folder"

MAskDeleteRecycleFile
"Вы хотите поместить в Корзину файл"
"Do you wish to move to the Recycle Bin the file"

MAskDeleteRecycle
"Вы хотите поместить в Корзину"
"Do you wish to move to the Recycle Bin"

MDeleteWipeTitle
"Уничтожение"
"Wipe"

MAskWipeFolder
"Вы хотите уничтожить папку"
"Do you wish to wipe the folder"

MAskWipeFile
"Вы хотите уничтожить файл"
"Do you wish to wipe the file"

MAskWipe
"Вы хотите уничтожить"
"Do you wish to wipe"

MDeleteLinkTitle
"Удаление ссылки"
"Delete link"

MAskDeleteLink
"является ссылкой на"
"is a symbolic link to"

MAskDeleteLinkFolder
"папку"
"folder"

MAskDeleteLinkFile
"файл"
"file"

MAskDeleteItems
"%d элемент%s"
"%d item%s"

MAskDeleteItems0
""
""

MAskDeleteItemsA
"а"
"s"

MAskDeleteItemsS
"ов"
"s"

MDeleteFolderTitle
l:
"Удаление папки "
"Delete folder"

MWipeFolderTitle
"Уничтожение папки "
"Wipe folder"

MDeleteFilesTitle
"Удаление файлов"
"Delete files"

MWipeFilesTitle
"Уничтожение файлов"
"Wipe files"

MDeleteFolderConfirm
"Данная папка будет удалена:"
"The following folder will be deleted:"

MWipeFolderConfirm
"Данная папка будет уничтожена:"
"The following folder will be wiped:"

MDeleteWipe
"Уничтожить"
"Wipe"

MDeleteFileDelete
"&Удалить"
"&Delete"

MDeleteFileWipe
"&Уничтожить"
"&Wipe"

MDeleteFileAll
"&Все"
"&All"

MDeleteFileSkip
"&Пропустить"
"&Skip"

MDeleteFileSkipAll
"П&ропустить все"
"S&kip all"

MDeleteFileCancel
"&Отменить"
"&Cancel"

MDeleteLinkDelete
l:
"Удалить ссылку"
"Delete link"

MDeleteLinkUnlink
"Разорвать ссылку"
"Break link"

MDeletingTitle
l:
"Удаление"
"Deleting"

MDeleting
l:
"Удаление файла или папки"
"Deleting the file or folder"

MDeletingWiping
"Уничтожение файла или папки"
"Wiping the file or folder"

MDeleteRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"

MAskDeleteRO
"Вы хотите удалить его?"
"Do you wish to delete it?"

MAskWipeRO
"Вы хотите уничтожить его?"
"Do you wish to wipe it?"

MDeleteHardLink1
l:
"Файл имеет несколько жестких ссылок"
"Several hard links link to this file."

MDeleteHardLink2
"Уничтожение файла приведет к обнулению всех ссылающихся на него файлов."
"Wiping this file will void all files linking to it."

MDeleteHardLink3
"Уничтожать файл?"
"Do you wish to wipe this file?"

MCannotDeleteFile
l:
"Ошибка удаления файла"
"Cannot delete the file"

MCannotDeleteFolder
"Ошибка удаления папки"
"Cannot delete the folder"

MDeleteRetry
"&Повторить"
"&Retry"

MDeleteSkip
"П&ропустить"
"&Skip"

MDeleteSkipAll
"Пропустить &все"
"S&kip all"

MDeleteCancel
"&Отменить"
"&Cancel"

MCannotGetSecurity
l:
"Ошибка получения прав доступа к файлу"
"Cannot get file access rights for"

MCannotSetSecurity
"Ошибка установки прав доступа к файлу"
"Cannot set file access rights for"

MEditTitle
l:
"Редактор"
"Editor"

MAskReload
"уже загружен. Как открыть этот файл?"
"already loaded. How to open this file?"

MCurrent
"&Текущий"
"&Current"

MReload
"Пере&грузить"
"R&eload"

MNewOpen
"&Новая копия"
"&New instance"

MEditCannotOpen
"Ошибка открытия файла"
"Cannot open the file"

MEditReading
"Чтение файла"
"Reading the file"

MEditAskSave
"Файл был изменен"
"File has been modified"

MEditAskSaveExt
"Файл был изменен внешней программой"
"The file was changed by an external program"

MEditSave
l:
"&Сохранить"
"&Save"

MEditNotSave
"&Не сохранять"
"Do &not save"

MEditContinue
"&Продолжить редактирование"
"&Continue editing"

MEditBtnSaveAs
"Сохр&анить как"
"Save &as..."

MEditRO
l:
"имеет атрибут \"Только для чтения\""
"is a read-only file"

MEditExists
"уже существует"
"already exists"

MEditOvr
"Вы хотите перезаписать его?"
"Do you wish to overwrite it?"

MEditSaving
"Сохранение файла"
"Saving the file"

MEditStatusLine
"Строка"
"Line"

MEditStatusCol
"Кол"
"Col"

MEditRSH
l:
"предназначен только для чтения"
"is a read-only file"

MEditFileGetSizeError
"Не удалось определить размер."
"File size could not be determined."

MEditFileLong
"имеет размер %s,"
"has the size of %s,"

MEditFileLong2
"что превышает заданное ограничение в %s."
"which exceeds the configured maximum size of %s."

MEditROOpen
"Вы хотите редактировать его?"
"Do you wish to edit it?"

MEditCanNotEditDirectory
l:
"Невозможно редактировать папку"
"It is impossible to edit the folder"

MEditSearchTitle
l:
"Поиск"
"Search"

MEditSearchFor
"&Искать"
"&Search for"

MEditSearchCase
"&Учитывать регистр"
"&Case sensitive"

MEditSearchWholeWords
"Только &целые слова"
"&Whole words"

MEditSearchReverse
"Обратн&ый поиск"
"Re&verse search"

MEditSearchSearch
"Искать"
"Search"

MEditSearchCancel
"Отменить"
"Cancel"

MEditReplaceTitle
l:
"Замена"
"Replace"

MEditReplaceWith
"Заменить &на"
"R&eplace with"

MEditReplaceReplace
"&Замена"
"&Replace"

MEditSearchingFor
l:
"Искать"
"Searching for"

MEditNotFound
"Строка не найдена"
"Could not find the string"

MEditAskReplace
l:
"Заменить"
"Replace"

MEditAskReplaceWith
"на"
"with"

MEditReplace
"&Заменить"
"&Replace"

MEditReplaceAll
"&Все"
"&All"

MEditSkip
"&Пропустить"
"&Skip"

MEditCancel
"&Отменить"
"&Cancel"

MEditGoToLine
l:
"Перейти"
"Go to position"

MFolderShortcutsTitle
l:
"Ссылки на папки"
"Folder shortcuts"

MFolderShortcutBottom
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"

MShortcutNone
"<отсутствует>"
"<none>"

MShortcutPlugin
"<плагин>"
"<plugin>"

MEnterShortcut
"Введите новую ссылку:"
"Enter new shortcut:"

MNeedNearPath
"Перейти в ближайшую доступную папку?"
"Jump to the nearest existing folder?"

MSaveThisShortcut
"Запомнить эту ссылку?"
"Save this shortcuts?"

MEditF1
l:
l://functional keys - 6 characters max, 12 keys, "DOS" is F8 dupe!
"Помощь"
"Help"

MEditF2
"Сохран"
"Save"

MEditF3
""
""

MEditF4
""
""

MEditF5
""
""

MEditF6
"Просм"
"View"

MEditF7
"Поиск"
"Search"

MEditF8
"Win"
"Win"

MEditF9
""
""

MEditF10
"Выход"
"Quit"

MEditF11
"Модули"
"Plugin"

MEditF12
"Экраны"
"Screen"

MEditF8DOS
le:// don't count this - it's a F8 another text
"DOS"
"DOS"

MEditShiftF1
l:
l://Editor: Shift
""
""

MEditShiftF2
"Сохр.в"
"SaveAs"

MEditShiftF3
""
""

MEditShiftF4
"Редак."
"Edit.."

MEditShiftF5
""
""

MEditShiftF6
""
""

MEditShiftF7
"Дальше"
"Next"

MEditShiftF8
"Таблиц"
"Table"

MEditShiftF9
""
""

MEditShiftF10
"СхрВых"
"SaveQ"

MEditShiftF11
""
""

MEditShiftF12
""
""

MEditAltF1
l:
l://Editor: Alt
""
""

MEditAltF2
""
""

MEditAltF3
""
""

MEditAltF4
""
""

MEditAltF5
"Печать"
"Print"

MEditAltF6
""
""

MEditAltF7
""
""

MEditAltF8
"Строка"
"Goto"

MEditAltF9
"Видео"
"Video"

MEditAltF10
""
""

MEditAltF11
"ИстПр"
"ViewHs"

MEditAltF12
""
""

MEditCtrlF1
l:
l://Editor: Ctrl
""
""

MEditCtrlF2
""
""

MEditCtrlF3
""
""

MEditCtrlF4
""
""

MEditCtrlF5
""
""

MEditCtrlF6
""
""

MEditCtrlF7
"Замена"
"Replac"

MEditCtrlF8
""
""

MEditCtrlF9
""
""

MEditCtrlF10
"Позиц"
"GoFile"

MEditCtrlF11
""
""

MEditCtrlF12
""
""

MEditAltShiftF1
l:
l://Editor: AltShift
""
""

MEditAltShiftF2
""
""

MEditAltShiftF3
""
""

MEditAltShiftF4
""
""

MEditAltShiftF5
""
""

MEditAltShiftF6
""
""

MEditAltShiftF7
""
""

MEditAltShiftF8
""
""

MEditAltShiftF9
"Конфиг"
"Config"

MEditAltShiftF10
""
""

MEditAltShiftF11
""
""

MEditAltShiftF12
""
""

MEditCtrlShiftF1
l:
l://Editor: CtrlShift
""
""

MEditCtrlShiftF2
""
""

MEditCtrlShiftF3
""
""

MEditCtrlShiftF4
""
""

MEditCtrlShiftF5
""
""

MEditCtrlShiftF6
""
""

MEditCtrlShiftF7
""
""

MEditCtrlShiftF8
""
""

MEditCtrlShiftF9
""
""

MEditCtrlShiftF10
""
""

MEditCtrlShiftF11
""
""

MEditCtrlShiftF12
""
""

MEditCtrlAltF1
l:
l:// Editor: CtrlAlt
""
""

MEditCtrlAltF2
""
""

MEditCtrlAltF3
""
""

MEditCtrlAltF4
""
""

MEditCtrlAltF5
""
""

MEditCtrlAltF6
""
""

MEditCtrlAltF7
""
""

MEditCtrlAltF8
""
""

MEditCtrlAltF9
""
""

MEditCtrlAltF10
""
""

MEditCtrlAltF11
""
""

MEditCtrlAltF12
""
""

MEditCtrlAltShiftF1
l:
l:// Editor: CtrlAltShift
""
""

MEditCtrlAltShiftF2
""
""

MEditCtrlAltShiftF3
""
""

MEditCtrlAltShiftF4
""
""

MEditCtrlAltShiftF5
""
""

MEditCtrlAltShiftF6
""
""

MEditCtrlAltShiftF7
""
""

MEditCtrlAltShiftF8
""
""

MEditCtrlAltShiftF9
""
""

MEditCtrlAltShiftF10
""
""

MEditCtrlAltShiftF11
""
""

MEditCtrlAltShiftF12
le://End of functional keys (Editor)
""
""

MSingleEditF1
l:
l://Single Editor: functional keys - 6 characters max, 12 keys, "DOS" is F8 dupe!
"Помощь"
"Help"

MSingleEditF2
"Сохран"
"Save"

MSingleEditF3
""
""

MSingleEditF4
""
""

MSingleEditF5
""
""

MSingleEditF6
"Просм"
"View"

MSingleEditF7
"Поиск"
"Search"

MSingleEditF8
"Win"
"Win"

MSingleEditF9
""
""

MSingleEditF10
"Выход"
"Quit"

MSingleEditF11
"Модули"
"Plugin"

MSingleEditF12
"Экраны"
"Screen"

MSingleEditF8DOS
le:// don't count this - it's a F8 another text
"DOS"
"DOS"

MSingleEditShiftF1
l:
l://Single Editor: Shift
""
""

MSingleEditShiftF2
"Сохр.в"
"SaveAs"

MSingleEditShiftF3
""
""

MSingleEditShiftF4
""
""

MSingleEditShiftF5
""
""

MSingleEditShiftF6
""
""

MSingleEditShiftF7
"Дальше"
"Next"

MSingleEditShiftF8
"Таблиц"
"Table"

MSingleEditShiftF9
""
""

MSingleEditShiftF10
"СхрВых"
"SaveQ"

MSingleEditShiftF11
""
""

MSingleEditShiftF12
""
""

MSingleEditAltF1
l:
l://Single Editor: Alt
""
""

MSingleEditAltF2
""
""

MSingleEditAltF3
""
""

MSingleEditAltF4
""
""

MSingleEditAltF5
"Печать"
"Print"

MSingleEditAltF6
""
""

MSingleEditAltF7
""
""

MSingleEditAltF8
"Строка"
"Goto"

MSingleEditAltF9
"Видео"
"Video"

MSingleEditAltF10
""
""

MSingleEditAltF11
"ИстПр"
"ViewHs"

MSingleEditAltF12
""
""

MSingleEditCtrlF1
l:
l://Single Editor: Ctrl
""
""

MSingleEditCtrlF2
""
""

MSingleEditCtrlF3
""
""

MSingleEditCtrlF4
""
""

MSingleEditCtrlF5
""
""

MSingleEditCtrlF6
""
""

MSingleEditCtrlF7
"Замена"
"Replac"

MSingleEditCtrlF8
""
""

MSingleEditCtrlF9
""
""

MSingleEditCtrlF10
""
""

MSingleEditCtrlF11
""
""

MSingleEditCtrlF12
""
""

MSingleEditAltShiftF1
l:
l://Single Editor: AltShift
""
""

MSingleEditAltShiftF2
""
""

MSingleEditAltShiftF3
""
""

MSingleEditAltShiftF4
""
""

MSingleEditAltShiftF5
""
""

MSingleEditAltShiftF6
""
""

MSingleEditAltShiftF7
""
""

MSingleEditAltShiftF8
""
""

MSingleEditAltShiftF9
"Конфиг"
"Config"

MSingleEditAltShiftF10
""
""

MSingleEditAltShiftF11
""
""

MSingleEditAltShiftF12
""
""

MSingleEditCtrlShiftF1
l:
l://Single Editor: CtrlShift
""
""

MSingleEditCtrlShiftF2
""
""

MSingleEditCtrlShiftF3
""
""

MSingleEditCtrlShiftF4
""
""

MSingleEditCtrlShiftF5
""
""

MSingleEditCtrlShiftF6
""
""

MSingleEditCtrlShiftF7
""
""

MSingleEditCtrlShiftF8
""
""

MSingleEditCtrlShiftF9
""
""

MSingleEditCtrlShiftF10
""
""

MSingleEditCtrlShiftF11
""
""

MSingleEditCtrlShiftF12
""
""

MSingleEditCtrlAltF1
l:
l://Single Editor: CtrlAlt
""
""

MSingleEditCtrlAltF2
""
""

MSingleEditCtrlAltF3
""
""

MSingleEditCtrlAltF4
""
""

MSingleEditCtrlAltF5
""
""

MSingleEditCtrlAltF6
""
""

MSingleEditCtrlAltF7
""
""

MSingleEditCtrlAltF8
""
""

MSingleEditCtrlAltF9
""
""

MSingleEditCtrlAltF10
""
""

MSingleEditCtrlAltF11
""
""

MSingleEditCtrlAltF12
""
""

MSingleEditCtrlAltShiftF1
l:
l://Single Editor: CtrlAltShift
""
""

MSingleEditCtrlAltShiftF2
""
""

MSingleEditCtrlAltShiftF3
""
""

MSingleEditCtrlAltShiftF4
""
""

MSingleEditCtrlAltShiftF5
""
""

MSingleEditCtrlAltShiftF6
""
""

MSingleEditCtrlAltShiftF7
""
""

MSingleEditCtrlAltShiftF8
""
""

MSingleEditCtrlAltShiftF9
""
""

MSingleEditCtrlAltShiftF10
""
""

MSingleEditCtrlAltShiftF11
""
""

MSingleEditCtrlAltShiftF12
le://End of functional keys (Single Editor)
""
""

MEditSaveAs
l:
"Сохранить &файл как"
"Save file &as"

MEditSaveAsFormatTitle
"Изменить перевод строки:"
"Change line breaks to:"

MEditSaveOriginal
"&исходный формат"
"Do n&ot change"

MEditSaveDOS
"в форма&те DOS/Windows (CR LF)"
"&Dos/Windows format (CR LF)"

MEditSaveUnix
"в формат&е UNIX (LF)"
"&Unix format (LF)"

MEditSaveMac
"в фор&мате MAC (CR)"
"&Mac format (CR)"

MEditCannotSave
"Ошибка сохранения файла"
"Cannot save the file"

MEditSavedChangedNonFile
"Файл изменен, но файл или папка, в которой он находился,"
"The file is changed but the file or the folder containing"

MEditSavedChangedNonFile1
"Файл или папка, в которой он находился,"
"The file or the folder containing"

MEditSavedChangedNonFile2
"был перемещен или удален."
"this file was moved or deleted."

MEditNewPath1
"Путь к редактируемому файлу не существует,"
"The path to the edited file does not exist,"

MEditNewPath2
"но будет создан при сохранении файла."
"but will be created when the file is saved."

MEditNewPath3
"Продолжать?"
"Continue?"

MEditNewPlugin1
"Имя редактируемого файла не может быть пустым"
"The name of the file to edit cannot be empty"

MColumnName
l:
"Имя"
"Name"

MColumnSize
"Размер"
"Size"

MColumnPacked
"Упаков"
"Packed"

MColumnDate
"Дата"
"Date"

MColumnTime
"Время"
"Time"

MColumnModified
"Модификация"
"Modified"

MColumnCreated
"Создание"
"Created"

MColumnAccessed
"Доступ"
"Accessed"

MColumnAttr
"Атриб"
"Attr"

MColumnDescription
"Описание"
"Description"

MColumnOwner
"Владелец"
"Owner"

MColumnMumLinks
"КлС"
"NmL"

MListUp
l:
"Вверх"
"  Up  "

MListFolder
"Папка"
"Folder"

MListSymLink
"Ссылка"
"Symlink"

MListJunction
"Связь"
"Junction"

MListBytes
"Б"
"B"

MListKb
"К"
"K"

MListMb
"М"
"M"

MListGb
"Г"
"G"

MListTb
"Т"
"T"

MListFileSize
" %s байт в 1 файле "
" %s bytes in 1 file "

MListFilesSize1
" %s байт в %d файле "
" %s bytes in %d files "

MListFilesSize2
" %s байт в %d файлах "
" %s bytes in %d files "

MListFreeSize
" %s байт свободно "
" %s free bytes "

MDirInfoViewTitle
l:
"Просмотр"
"View"

MFileToEdit
"Редактировать файл:"
"File to edit:"

MUnselectTitle
l:
"Снять"
"Deselect"

MSelectTitle
"Пометить"
"Select"

MSelectFilter
"&Фильтр"
"&Filter"

MCompareTitle
l:
"Сравнение"
"Compare"

MCompareFilePanelsRequired1
"Для сравнения папок требуются"
"Two file panels are required to perform"

MCompareFilePanelsRequired2
"две файловые панели"
"the Compare folders command"

MCompareSameFolders1
"Содержимое папок,"
"The folders contents seems"

MCompareSameFolders2
"скорее всего, одинаково"
"to be identical"

MSelectAssocTitle
l:
"Выберите ассоциацию"
"Select association"

MAssocTitle
l:
"Ассоциации для файлов"
"File associations"

MAssocBottom
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"

MAskDelAssoc
"Вы хотите удалить ассоциацию для"
"Do you wish to delete association for"

MAssocNeedMask
"Пожалуйста, укажите маску файлов"
"Please specify a file mask"

MFileAssocTitle
l:
"Редактирование ассоциаций файлов"
"Edit file associations"

MFileAssocMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"

MFileAssocDescr
"&Описание ассоциации:"
"&Description of the association:"

MFileAssocExec
"Команда, &выполняемая по Enter:"
"E&xecute command (used for Enter):"

MFileAssocAltExec
"Коман&да, выполняемая по Ctrl-PgDn:"
"Exec&ute command (used for Ctrl-PgDn):"

MFileAssocView
"Команда &просмотра, выполняемая по F3:"
"&View command (used for F3):"

MFileAssocAltView
"Команда просмотра, в&ыполняемая по Alt-F3:"
"V&iew command (used for Alt-F3):"

MFileAssocEdit
"Команда &редактирования, выполняемая по F4:"
"&Edit command (used for F4):"

MFileAssocAltEdit
"Команда редактировани&я, выполняемая по Alt-F4:"
"Edit comm&and (used for Alt-F4):"

MViewF1
l:
l://Viewer: functional keys, 12 keys, except F2 - 2 keys, and F8 - 2 keys
"Помощь"
"Help"

MViewF2
le:// this is another text for F2
"Сверн"
"Wrap"

MViewF3
"Выход"
"Quit"

MViewF4
"Код"
"Hex"

MViewF5
""
""

MViewF6
"Редакт"
"Edit"

MViewF7
"Поиск"
"Search"

MViewF8
"Win"
"Win"

MViewF9
""
""

MViewF10
"Выход"
"Quit"

MViewF11
"Модули"
"Plugins"

MViewF12
"Экраны"
"Screen"

MViewF2Unwrap
"Развер"
"Unwrap"

MViewF4Text
l:// this is another text for F4
"Текст"
"Text"

MViewF8DOS
"DOS"
"DOS"

MViewShiftF1
l:
l://Viewer: Shift
""
""

MViewShiftF2
"Слова"
"WWrap"

MViewShiftF3
""
""

MViewShiftF4
""
""

MViewShiftF5
""
""

MViewShiftF6
""
""

MViewShiftF7
"Дальше"
"Next"

MViewShiftF8
"Таблиц"
"Table"

MViewShiftF9
""
""

MViewShiftF10
""
""

MViewShiftF11
""
""

MViewShiftF12
""
""

MViewAltF1
l:
l://Viewer: Alt
""
""

MViewAltF2
""
""

MViewAltF3
""
""

MViewAltF4
""
""

MViewAltF5
"Печать"
"Print"

MViewAltF6
""
""

MViewAltF7
"Назад"
"Prev"

MViewAltF8
"Перейт"
"Goto"

MViewAltF9
"Видео"
"Video"

MViewAltF10
""
""

MViewAltF11
"ИстПр"
"ViewHs"

MViewAltF12
""
""

MViewCtrlF1
l:
l://Viewer: Ctrl
""
""

MViewCtrlF2
""
""

MViewCtrlF3
""
""

MViewCtrlF4
""
""

MViewCtrlF5
""
""

MViewCtrlF6
""
""

MViewCtrlF7
""
""

MViewCtrlF8
""
""

MViewCtrlF9
""
""

MViewCtrlF10
"Позиц"
"GoFile"

MViewCtrlF11
""
""

MViewCtrlF12
""
""

MViewAltShiftF1
l:
l://Viewer: AltShift
""
""

MViewAltShiftF2
""
""

MViewAltShiftF3
""
""

MViewAltShiftF4
""
""

MViewAltShiftF5
""
""

MViewAltShiftF6
""
""

MViewAltShiftF7
""
""

MViewAltShiftF8
""
""

MViewAltShiftF9
"Конфиг"
"Config"

MViewAltShiftF10
""
""

MViewAltShiftF11
""
""

MViewAltShiftF12
""
""

MViewCtrlShiftF1
l:
l://Viewer: CtrlShift
""
""

MViewCtrlShiftF2
""
""

MViewCtrlShiftF3
""
""

MViewCtrlShiftF4
""
""

MViewCtrlShiftF5
""
""

MViewCtrlShiftF6
""
""

MViewCtrlShiftF7
""
""

MViewCtrlShiftF8
""
""

MViewCtrlShiftF9
""
""

MViewCtrlShiftF10
""
""

MViewCtrlShiftF11
""
""

MViewCtrlShiftF12
""
""

MViewCtrlAltF1
l:
l://Viewer: CtrlAlt
""
""

MViewCtrlAltF2
""
""

MViewCtrlAltF3
""
""

MViewCtrlAltF4
""
""

MViewCtrlAltF5
""
""

MViewCtrlAltF6
""
""

MViewCtrlAltF7
""
""

MViewCtrlAltF8
""
""

MViewCtrlAltF9
""
""

MViewCtrlAltF10
""
""

MViewCtrlAltF11
""
""

MViewCtrlAltF12
""
""

MViewCtrlAltShiftF1
l:
l://Viewer: CtrlAltShift
""
""

MViewCtrlAltShiftF2
""
""

MViewCtrlAltShiftF3
""
""

MViewCtrlAltShiftF4
""
""

MViewCtrlAltShiftF5
""
""

MViewCtrlAltShiftF6
""
""

MViewCtrlAltShiftF7
""
""

MViewCtrlAltShiftF8
""
""

MViewCtrlAltShiftF9
""
""

MViewCtrlAltShiftF10
""
""

MViewCtrlAltShiftF11
""
""

MViewCtrlAltShiftF12
le://end of functional keys (Viewer)
""
""

MSingleViewF1
l:
l://Single Viewer: functional keys, 12 keys, except F2 - 2 keys, and F8 - 2 keys
"Помощь"
"Help"

MSingleViewF2
"Сверн"
"Wrap"

MSingleViewF3
"Выход"
"Quit"

MSingleViewF4
"Код"
"Hex"

MSingleViewF5
""
""

MSingleViewF6
"Редакт"
"Edit"

MSingleViewF7
"Поиск"
"Search"

MSingleViewF8
"Win"
"Win"

MSingleViewF9
""
""

MSingleViewF10
"Выход"
"Quit"

MSingleViewF11
"Модули"
"Plugins"

MSingleViewF12
"Экраны"
"Screen"

MSingleViewF2Unwrap
l:// this is another text for F2
"Развер"
"Unwrap"

MSingleViewF4Text
l:// this is another text for F4
"Текст"
"Text"

MSingleViewF8DOS
"DOS"
"DOS"

MSingleViewShiftF1
l:
l://Single Viewer: Shift
""
""

MSingleViewShiftF2
"Слова"
"WWrap"

MSingleViewShiftF3
""
""

MSingleViewShiftF4
""
""

MSingleViewShiftF5
""
""

MSingleViewShiftF6
""
""

MSingleViewShiftF7
"Дальше"
"Next"

MSingleViewShiftF8
"Таблиц"
"Table"

MSingleViewShiftF9
""
""

MSingleViewShiftF10
""
""

MSingleViewShiftF11
""
""

MSingleViewShiftF12
""
""

MSingleViewAltF1
l:
l://Single Viewer: Alt
""
""

MSingleViewAltF2
""
""

MSingleViewAltF3
""
""

MSingleViewAltF4
""
""

MSingleViewAltF5
"Печать"
"Print"

MSingleViewAltF6
""
""

MSingleViewAltF7
"Назад"
"Prev"

MSingleViewAltF8
"Перейт"
"Goto"

MSingleViewAltF9
"Видео"
"Video"

MSingleViewAltF10
""
""

MSingleViewAltF11
"ИстПр"
"ViewHs"

MSingleViewAltF12
""
""

MSingleViewCtrlF1
l:
l://Single Viewer: Ctrl
""
""

MSingleViewCtrlF2
""
""

MSingleViewCtrlF3
""
""

MSingleViewCtrlF4
""
""

MSingleViewCtrlF5
""
""

MSingleViewCtrlF6
""
""

MSingleViewCtrlF7
""
""

MSingleViewCtrlF8
""
""

MSingleViewCtrlF9
""
""

MSingleViewCtrlF10
""
""

MSingleViewCtrlF11
""
""

MSingleViewCtrlF12
""
""

MSingleViewAltShiftF1
l:
l://Single Viewer: AltShift
""
""

MSingleViewAltShiftF2
""
""

MSingleViewAltShiftF3
""
""

MSingleViewAltShiftF4
""
""

MSingleViewAltShiftF5
""
""

MSingleViewAltShiftF6
""
""

MSingleViewAltShiftF7
""
""

MSingleViewAltShiftF8
""
""

MSingleViewAltShiftF9
"Конфиг"
"Config"

MSingleViewAltShiftF10
""
""

MSingleViewAltShiftF11
""
""

MSingleViewAltShiftF12
""
""

MSingleViewCtrlShiftF1
l:
l://Single Viewer: CtrlShift
""
""

MSingleViewCtrlShiftF2
""
""

MSingleViewCtrlShiftF3
""
""

MSingleViewCtrlShiftF4
""
""

MSingleViewCtrlShiftF5
""
""

MSingleViewCtrlShiftF6
""
""

MSingleViewCtrlShiftF7
""
""

MSingleViewCtrlShiftF8
""
""

MSingleViewCtrlShiftF9
""
""

MSingleViewCtrlShiftF10
""
""

MSingleViewCtrlShiftF11
""
""

MSingleViewCtrlShiftF12
""
""

MSingleViewCtrlAltF1
l:
l://Single Viewer: CtrlAlt
""
""

MSingleViewCtrlAltF2
""
""

MSingleViewCtrlAltF3
""
""

MSingleViewCtrlAltF4
""
""

MSingleViewCtrlAltF5
""
""

MSingleViewCtrlAltF6
""
""

MSingleViewCtrlAltF7
""
""

MSingleViewCtrlAltF8
""
""

MSingleViewCtrlAltF9
""
""

MSingleViewCtrlAltF10
""
""

MSingleViewCtrlAltF11
""
""

MSingleViewCtrlAltF12
""
""

MSingleViewCtrlAltShiftF1
l:
l://Single Viewer: CtrlAltShift
""
""

MSingleViewCtrlAltShiftF2
""
""

MSingleViewCtrlAltShiftF3
""
""

MSingleViewCtrlAltShiftF4
""
""

MSingleViewCtrlAltShiftF5
""
""

MSingleViewCtrlAltShiftF6
""
""

MSingleViewCtrlAltShiftF7
""
""

MSingleViewCtrlAltShiftF8
""
""

MSingleViewCtrlAltShiftF9
""
""

MSingleViewCtrlAltShiftF10
""
""

MSingleViewCtrlAltShiftF11
""
""

MSingleViewCtrlAltShiftF12
le://end of functional keys (Single Viewer)
""
""

MInViewer
"просмотр %s"
"view %s"

MInEditor
"редактирование %s"
"edit %s"

MFilterTitle
l:
"Меню фильтров"
"Filters menu"

MFilterBottom
"+,-,Пробел,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Space,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"

MPanelFileType
"Файлы панели"
"Panel file type"

MFolderFileType
"Папки"
"Folders"

MCanEditCustomFilterOnly
"Только пользовательский фильтр можно редактировать"
"Only custom filter can be edited"

MAskDeleteFilter
"Вы хотите удалить фильтр"
"Do you wish to delete the filter"

MCanDeleteCustomFilterOnly
"Только пользовательский фильтр может быть удален"
"Only custom filter can be deleted"

MFindFileTitle
l:
"Поиск файла"
"Find file"

MFindFileResultTitle
"Поиск файла - результат"
"Find file - result"

MFindFileMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"

MFindFileText
"&Содержащих текст:"
"Containing &text:"

MFindFileHex
"&Содержащих 16-ричный код:"
"Con&taining hex:"

MFindFileCodePage
"Используя таблицу сим&волов:"
"Using character ta&ble:"

MFindFileCase
"&Учитывать регистр"
"&Case sensitive"

MFindFileWholeWords
"Только &целые слова"
"&Whole words"

MFindFileAllTables
"Все таблицы символов"
"All character tables"

MFindArchives
"Искать в а&рхивах"
"Search in arch&ives"

MFindFolders
"Искать п&апки"
"Search for f&olders"

MFindSymLinks
"Искать в символи&ческих ссылках"
"Search in symbolic lin&ks"

MSearchForHex
"Искать 16-ричн&ый код"
"Search for &hex"

MSearchWhere
"Выберите область поиска:"
"Select search area:"

MSearchAllDisks
"На всех несъемных &дисках"
"In &all non-removable drives"

MSearchAllButNetwork
"На всех &локальных дисках"
"In all &local drives"

MSearchInPATH
"В PATH-катало&гах"
"In &PATH folders"

MSearchFromRootOfDrive
"С кор&ня диска"
"From the &root of"

MSearchFromRootFolder
"С кор&невой папки"
"From the &root folder"

MSearchFromCurrent
"С &текущей папки"
"From the curre&nt folder"

MSearchInCurrent
"Только в теку&щей папке"
"The current folder onl&y"

MSearchInSelected
"В &отмеченных папках"
"&Selected folders"

MFindUseFilter
"Исполь&зовать фильтр"
"&Use filter"

MFindAdvancedOptions
"Дополнит&ельные параметры"
"Advanced options"

MFindUsingFilter
"используя фильтр"
"using filter"

MFindFileFind
"&Искать"
"&Find"

MFindFileDrive
"Дис&к"
"Dri&ve"

MFindFileSetFilter
"&Фильтр"
"Filt&er"

MFindFileAdvanced
"До&полнительно"
"Advance&d"

MFindFileTable
"Т&аблица"
"Ta&ble"

MFindSearchingIn
"Поиск%s в:"
"Searching%s in:"

MFindNewSearch
"&Новый поиск"
"&New search"

MFindGoTo
"Пе&рейти"
"&Go to"

MFindView
"&Смотреть"
"&View"

MFindPanel
"Пане&ль"
"&Panel"

MFindStop
"С&топ"
"&Stop"

MFindDone
l:
"Поиск закончен. Найдено %d файл(ов) и %d папка(ок)"
"Search done. Found %d file(s) and %d folder(s)"

MFindCancel
"Отм&ена"
"&Cancel"

MFindFound
l:
"Найдено"
"Found"

MFindFileFolder
l:
"Папка"
"Folder"

MFindFileAdvancedTitle
l:
"Дополнительные параметры поиска"
"Find file advanced options"

MFindFileSearchFirst
"Проводить поиск в &первых:"
"Search only in the &first:"

MFoldTreeSearch
l:
"Поиск:"
"Search:"

MGetTableTitle
l:
"Таблицы"
"Tables"

MGetTableNormalText
"DOS текст"
"DOS text"

MGetTableWindowsText
"Windows текст"
"Windows text"

MHighlightTitle
l:
"Раскраска файлов"
"Files highlighting"

MHighlightBottom
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"

MHighlightUpperSortGroup
"Верхняя группа сортировки"
"Upper sort group"

MHighlightLowerSortGroup
"Нижняя группа сортировки"
"Lower sort group"

MHighlightLastGroup
"Наименее приоритетная группа раскраски"
"Lowest priority highlighting group"

MHighlightAskDel
"Вы хотите удалить раскраску для"
"Do you wish to delete highlighting for"

MHighlightWarning
"Будут потеряны все Ваши настройки!"
"You will lose all changes!"

MHighlightAskRestore
"Вы хотите восстановить раскраску файлов по умолчанию?"
"Do you wish to restore default highlighting?"

MHighlightEditTitle
l:
"Редактирование раскраски файлов"
"Edit files highlighting"

MHighlightMarkChar
"Оп&циональный символ пометки,"
"Optional markin&g character,"

MHighlightTransparentMarkChar
"прозра&чный"
"tra&nsparent"

MHighlightColors
" Цвета файлов (\"черный на черном\" - цвет по умолчанию) "
" File name colors (\"black on black\" - default color) "

MHighlightFileName1
"&1. Обычное имя файла                "
"&1. Normal file name               "

MHighlightFileName2
"&3. Помеченное имя файла             "
"&3. Selected file name             "

MHighlightFileName3
"&5. Имя файла под курсором           "
"&5. File name under cursor         "

MHighlightFileName4
"&7. Помеченное под курсором имя файла"
"&7. File name selected under cursor"

MHighlightMarking1
"&2. Пометка"
"&2. Marking"

MHighlightMarking2
"&4. Пометка"
"&4. Marking"

MHighlightMarking3
"&6. Пометка"
"&6. Marking"

MHighlightMarking4
"&8. Пометка"
"&8. Marking"

MHighlightExample1
"║filename.ext │"
"║filename.ext │"

MHighlightExample2
"║ filename.ext│"
"║ filename.ext│"

MHighlightContinueProcessing
"Продолжать &обработку"
"C&ontinue processing"

MInfoTitle
l:
"Информация"
"Information"

MInfoCompName
"Имя компьютера"
"Computer name"

MInfoUserName
"Имя пользователя"
"User name"

MInfoRemovable
"Сменный"
"Removable"

MInfoFixed
"Жесткий"
"Fixed"

MInfoNetwork
"Сетевой"
"Network"

MInfoCDROM
"CD-ROM"
"CD-ROM"

MInfoCD_RW
"CD-RW"
"CD-RW"

MInfoCD_RWDVD
"CD-RW/DVD"
"CD-RW/DVD"

MInfoDVD_ROM
"DVD-ROM"
"DVD-ROM"

MInfoDVD_RW
"DVD-RW"
"DVD-RW"

MInfoDVD_RAM
"DVD-RAM"
"DVD-RAM"

MInfoRAM
"RAM"
"RAM"

MInfoSUBST
"SUBST"
"Subst"

MInfoDisk
"диск"
"disk"

MInfoDiskTotal
"Всего байтов"
"Total bytes"

MInfoDiskFree
"Свободных байтов"
"Free bytes"

MInfoDiskLabel
"Метка тома"
"Volume label"

MInfoDiskNumber
"Серийный номер"
"Serial number"

MInfoMemory
" Память "
" Memory "

MInfoMemoryLoad
"Загрузка памяти"
"Memory load"

MInfoMemoryTotal
"Всего памяти"
"Total memory"

MInfoMemoryFree
"Свободно памяти"
"Free memory"

MInfoVirtualTotal
"Всего вирт. памяти"
"Total virtual"

MInfoVirtualFree
"Свободно вирт. памяти"
"Free virtual"

MInfoDizAbsent
"Файл описания папки отсутствует"
"Folder description file is absent"

MErrorInvalidFunction
l:
"Некорректная функция"
"Incorrect function"

MErrorBadCommand
"Команда не распознана"
"Command not recognized"

MErrorFileNotFound
"Файл не найден"
"File not found"

MErrorPathNotFound
"Путь не найден"
"Path not found"

MErrorTooManyOpenFiles
"Слишком много открытых файлов"
"Too many open files"

MErrorAccessDenied
"Доступ запрещен"
"Access denied"

MErrorNotEnoughMemory
"Недостаточно памяти"
"Not enough memory"

MErrorDiskRO
"Попытка записи на защищенный от записи диск"
"Cannot write to write protected disk"

MErrorDeviceNotReady
"Устройство не готово"
"The device is not ready"

MErrorCannotAccessDisk
"Доступ к диску невозможен"
"Disk cannot be accessed"

MErrorSectorNotFound
"Сектор не найден"
"Sector not found"

MErrorOutOfPaper
"В принтере нет бумаги"
"The printer is out of paper"

MErrorWrite
"Ошибка записи"
"Write fault error"

MErrorRead
"Ошибка чтения"
"Read fault error"

MErrorDeviceGeneral
"Общая ошибка устройства"
"Device general failure"

MErrorFileSharing
"Нарушение совместного доступа к файлу"
"File sharing violation"

MErrorNetworkPathNotFound
"Сетевой путь не найден"
"The network path was not found"

MErrorNetworkBusy
"Сеть занята"
"The network is busy"

MErrorNetworkAccessDenied
"Сетевой доступ запрещен"
"Network access is denied"

MErrorNetworkWrite
"Ошибка записи в сети"
"A write fault occurred on the network"

MErrorDiskLocked
"Диск используется или заблокирован другим процессом"
"The disk is in use or locked by another process"

MErrorFileExists
"Файл или папка уже существует"
"File or folder already exists"

MErrorInvalidName
"Указанное имя неверно"
"The specified name is invalid"

MErrorInsufficientDiskSpace
"Нет места на диске"
"Insufficient disk space"

MErrorFolderNotEmpty
"Папка не пустая"
"The folder is not empty"

MErrorIncorrectUserName
"Неверное имя пользователя"
"Incorrect user name"

MErrorIncorrectPassword
"Неверный пароль"
"Incorrect password"

MErrorLoginFailure
"Ошибка регистрации"
"Login failure"

MErrorConnectionAborted
"Соединение разорвано"
"Connection aborted"

MErrorCancelled
"Операция отменена"
"Operation cancelled"

MErrorNetAbsent
"Сеть отсутствует"
"No network present"

MErrorNetDeviceInUse
"Устройство используется и не может быть отсоединено"
"Device is in use and cannot be disconnected"

MErrorNetOpenFiles
"На сетевом диске есть открытые файлы"
"This network connection has open files"

MErrorAlreadyAssigned
"Имя локального устройства уже использовано"
"The local device name is already in use"

MErrorAlreadyRemebered
"Имя локального устройства уже находится в профиле пользователя"
"The local device is already in the user profile"

MErrorNotLoggedOn
"Пользователь не зарегистрирован в сети"
"User has not logged on to the network"

MErrorInvalidPassword
"Неверный пароль пользователя"
"The user password is invalid"

MErrorNoRecoveryPolicy
"Для этой системы отсутствует политика надежного восстановления шифрования"
"There is no valid encryption recovery policy configured for this system"

MErrorEncryptionFailed
"Ошибка при попытке шифрования файла"
"The specified file could not be encrypted"

MErrorDecryptionFailed
"Ошибка при попытке расшифровки файла"
"The specified file could not be decrypted"

MErrorFileNotEncrypted
"Указанный файл не зашифрован"
"The specified file is not encrypted"

MErrorNoAssociation
"Указанному файлу не сопоставлено ни одно приложение для выполнения данной операции"
"No application is associated with the specified file for this operation"

MErrorFullPathNameLong
l:
"Полный путь к файлу имеет слишком большую длину"
"The full pathname is too long"

MCannotExecute
l:
"Ошибка выполнения"
"Cannot execute"

MScanningFolder
"Просмотр папки"
"Scanning the folder"

MMakeFolderTitle
l:
"Создание папки"
"Make folder"

MCreateFolder
"Создать п&апку"
"Create the &folder"

MMultiMakeDir
"Обрабатыват&ь несколько имен папок"
"Process &multiple names"

MIncorrectDirList
"Неправильный список папок"
"Incorrect folders list"

MCannotCreateFolder
"Ошибка создания папки"
"Cannot create the folder"

MMenuBriefView
l:
"&Краткий                  LCtrl-1"
"&Brief              LCtrl-1"

MMenuMediumView
"&Средний                  LCtrl-2"
"&Medium             LCtrl-2"

MMenuFullView
"&Полный                   LCtrl-3"
"&Full               LCtrl-3"

MMenuWideView
"&Широкий                  LCtrl-4"
"&Wide               LCtrl-4"

MMenuDetailedView
"&Детальный                LCtrl-5"
"Detai&led           LCtrl-5"

MMenuDizView
"&Описания                 LCtrl-6"
"&Descriptions       LCtrl-6"

MMenuLongDizView
"Д&линные описания         LCtrl-7"
"Lon&g descriptions  LCtrl-7"

MMenuOwnersView
"Вл&адельцы файлов         LCtrl-8"
"File own&ers        LCtrl-8"

MMenuLinksView
"Свя&зи файлов             LCtrl-9"
"File lin&ks         LCtrl-9"

MMenuAlternativeView
"Аль&тернативный полный    LCtrl-0"
"&Alternative full   LCtrl-0"

MMenuInfoPanel
l:
"Панель ин&формации        Ctrl-L"
"&Info panel         Ctrl-L"

MMenuTreePanel
"Де&рево папок             Ctrl-T"
"&Tree panel         Ctrl-T"

MMenuQuickView
"Быстры&й просмотр         Ctrl-Q"
"Quick &view         Ctrl-Q"

MMenuSortModes
"Режим&ы сортировки        Ctrl-F12"
"&Sort modes         Ctrl-F12"

MMenuLongNames
"Показывать длинные &имена Ctrl-N"
"Show long &names    Ctrl-N"

MMenuTogglePanel
"Панель &Вкл/Выкл          Ctrl-F1"
"Panel &On/Off       Ctrl-F1"

MMenuReread
"П&еречитать               Ctrl-R"
"&Re-read            Ctrl-R"

MMenuChangeDrive
"С&менить диск             Alt-F1"
"&Change drive       Alt-F1"

MMenuView
l:
"&Просмотр              F3"
"&View               F3"

MMenuEdit
"&Редактирование        F4"
"&Edit               F4"

MMenuCopy
"&Копирование           F5"
"&Copy               F5"

MMenuMove
"П&еренос               F6"
"&Rename or move     F6"

MMenuCreateFolder
"&Создание папки        F7"
"&Make folder        F7"

MMenuDelete
"&Удаление              F8"
"&Delete             F8"

MMenuWipe
"Уни&чтожение           Alt-Del"
"&Wipe               Alt-Del"

MMenuAdd
"&Архивировать          Shift-F1"
"Add &to archive     Shift-F1"

MMenuExtract
"Распако&вать           Shift-F2"
"E&xtract files      Shift-F2"

MMenuArchiveCommands
"Архивн&ые команды      Shift-F3"
"Arc&hive commands   Shift-F3"

MMenuAttributes
"А&трибуты файлов       Ctrl-A"
"File &attributes    Ctrl-A"

MMenuApplyCommand
"Применить коман&ду     Ctrl-G"
"A&pply command      Ctrl-G"

MMenuDescribe
"&Описание файлов       Ctrl-Z"
"Descri&be files     Ctrl-Z"

MMenuSelectGroup
"Пометить &группу       Gray +"
"Select &group       Gray +"

MMenuUnselectGroup
"С&нять пометку         Gray -"
"U&nselect group     Gray -"

MMenuInvertSelection
"&Инверсия пометки      Gray *"
"&Invert selection   Gray *"

MMenuRestoreSelection
"Восстановить по&метку  Ctrl-M"
"Re&store selection  Ctrl-M"

MMenuFindFile
l:
"&Поиск файла              Alt-F7"
"&Find file           Alt-F7"

MMenuHistory
"&История команд           Alt-F8"
"&History             Alt-F8"

MMenuVideoMode
"Видео&режим               Alt-F9"
"&Video mode          Alt-F9"

MMenuFindFolder
"Поис&к папки              Alt-F10"
"Fi&nd folder         Alt-F10"

MMenuViewHistory
"Ис&тория просмотра        Alt-F11"
"File vie&w history   Alt-F11"

MMenuFoldersHistory
"Ист&ория папок            Alt-F12"
"F&olders history     Alt-F12"

MMenuSwapPanels
"По&менять панели          Ctrl-U"
"&Swap panels         Ctrl-U"

MMenuTogglePanels
"Панели &Вкл/Выкл          Ctrl-O"
"&Panels On/Off       Ctrl-O"

MMenuCompareFolders
"&Сравнение папок"
"&Compare folders"

MMenuUserMenu
"Меню пользовател&я"
"Edit user &menu"

MMenuFileAssociations
"&Ассоциации файлов"
"File &associations"

MMenuFolderShortcuts
"Ссы&лки на папки"
"Fol&der shortcuts"

MMenuFilter
"&Фильтр панели файлов     Ctrl-I"
"File panel f&ilter   Ctrl-I"

MMenuPluginCommands
"Команды внешних мо&дулей  F11"
"Pl&ugin commands     F11"

MMenuWindowsList
"Список экра&нов           F12"
"Sc&reens list        F12"

MMenuProcessList
"Список &задач             Ctrl-W"
"Task &list           Ctrl-W"

MMenuHotPlugList
"Список Hotplug-&устройств"
"Ho&tplug devices list"

MMenuSystemSettings
l:
"Систе&мные параметры"
"S&ystem settings"

MMenuPanelSettings
"Настройки па&нели"
"&Panel settings"

MMenuInterface
"Настройки &интерфейса"
"&Interface settings"

MMenuDialogSettings
"Настройки &диалогов"
"Di&alog settings"

MMenuLanguages
"&Языки"
"Lan&guages"

MMenuPluginsConfig
"Параметры &внешних модулей"
"Pl&ugins configuration"

MMenuConfirmation
"&Подтверждения"
"Co&nfirmations"

MMenuFilePanelModes
"Режим&ы панели файлов"
"File panel &modes"

MMenuFileDescriptions
"&Описания файлов"
"File &descriptions"

MMenuFolderInfoFiles
"Файлы описания п&апок"
"&Folder description files"

MMenuViewer
"Настройки про&граммы просмотра"
"&Viewer settings"

MMenuEditor
"Настройки &редактора"
"&Editor settings"

MMenuColors
"&Цвета"
"Co&lors"

MMenuFilesHighlighting
"Раскраска &файлов и группы сортировки"
"Files &highlighting and sort groups"

MMenuSaveSetup
"&Сохранить параметры                  Shift-F9"
"&Save setup                         Shift-F9"

MMenuTogglePanelRight
"Панель &Вкл/Выкл          Ctrl-F2"
"Panel &On/Off       Ctrl-F2"

MMenuChangeDriveRight
"С&менить диск             Alt-F2"
"&Change drive       Alt-F2"

MMenuLeftTitle
l:
"&Левая"
"&Left"

MMenuFilesTitle
"&Файлы"
"&Files"

MMenuCommandsTitle
"&Команды"
"&Commands"

MMenuOptionsTitle
"Па&раметры"
"&Options"

MMenuRightTitle
"&Правая"
"&Right"

MMenuSortTitle
l:
"Критерий сортировки"
"Sort by"

MMenuSortByName
"&Имя                              Ctrl-F3"
"&Name                 Ctrl-F3"

MMenuSortByExt
"&Расширение                       Ctrl-F4"
"E&xtension            Ctrl-F4"

MMenuSortByModification
"Время &модификации                Ctrl-F5"
"&Modification time    Ctrl-F5"

MMenuSortBySize
"Р&азмер                           Ctrl-F6"
"&Size                 Ctrl-F6"

MMenuUnsorted
"&Не сортировать                   Ctrl-F7"
"&Unsorted             Ctrl-F7"

MMenuSortByCreation
"Время &создания                   Ctrl-F8"
"&Creation time        Ctrl-F8"

MMenuSortByAccess
"Время &доступа                    Ctrl-F9"
"&Access time          Ctrl-F9"

MMenuSortByDiz
"&Описания                         Ctrl-F10"
"&Descriptions         Ctrl-F10"

MMenuSortByOwner
"&Владельцы файлов                 Ctrl-F11"
"&Owner                Ctrl-F11"

MMenuSortByCompressedSize
"&Упакованный размер"
"Com&pressed size"

MMenuSortByNumLinks
"Ко&личество ссылок"
"Number of &hard links"

MMenuSortUseGroups
"Использовать &группы сортировки   Shift-F11"
"Use sort &groups      Shift-F11"

MMenuSortSelectedFirst
"&Помеченные файлы вперед          Shift-F12"
"Show selected &first  Shift-F12"

MMenuSortUseNumeric
"Использовать &числовую сортировку"
"Use num&eric sort"

MChangeDriveTitle
l:
"Диск"
"Drive"

MChangeDriveRemovable
"сменный  "
"removable"

MChangeDriveFixed
"жесткий  "
"fixed    "

MChangeDriveNetwork
"сетевой  "
"network  "

MChangeDriveCDROM
"CD-ROM   "
"CD-ROM   "

MChangeDriveCD_RW
"CD-RW    "
"CD-RW    "

MChangeDriveCD_RWDVD
"CD-RW/DVD"
"CD-RW/DVD"

MChangeDriveDVD_ROM
"DVD-ROM  "
"DVD-ROM  "

MChangeDriveDVD_RW
"DVD-RW   "
"DVD-RW   "

MChangeDriveDVD_RAM
"DVD-RAM  "
"DVD-RAM  "

MChangeDriveRAM
"RAM диск "
"RAM disk "

MChangeDriveSUBST
"SUBST    "
"subst    "

MChangeDriveLabelAbsent
"недоступен"
"not available"

MChangeDriveCannotReadDisk
"Ошибка чтения диска в дисководе %c:"
"Cannot read the disk in drive %c:"

MChangeDriveCannotDisconnect
"Не удается отсоединиться от %s"
"Cannot disconnect from %s"

MChangeDriveCannotDelSubst
"Не удается удалить виртуальный драйвер %s"
"Cannot delete a substituted drive %s"

MChangeDriveOpenFiles
"Если вы не закроете открытые файлы, данные могут быть утеряны"
"If you do not close the open files, data may be lost."

MChangeSUBSTDisconnectDriveTitle
l:
"Отключение виртуального драйвера"
"Virtual device disconnection"

MChangeSUBSTDisconnectDriveQuestion
"Отключить SUBST-диск %c:?"
"Disconnect SUBST-disk %c:?"

MChangeHotPlugDisconnectDriveTitle
l:
"Удаление устройства"
"Device Removal"

MChangeHotPlugDisconnectDriveQuestion
"Вы хотите удалить устройство"
"Do you want to remove the device"

MHotPlugDisks
"(диск(и): %s)"
"(disk(s): %s)"

MChangeCouldNotEjectHotPlugMedia
"Невозможно удалить устройство для диска %c:"
"Cannot remove a device for drive %c:"

MChangeCouldNotEjectHotPlugMedia2
"Невозможно удалить устройство:"
"Cannot remove a device:"

MChangeHotPlugNotify1
"Теперь устройство" 
"The device" 

MChangeHotPlugNotify2
"может быть безопасно извлечено из компьютера"
"can now be safely removed"

MHotPlugListTitle
"Hotplug-устройства"
"Hotplug devices list"

MHotPlugListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"

MChangeDriveDisconnectTitle
l:
"Отключение сетевого устройства"
"Disconnect network drive"

MChangeDriveDisconnectQuestion
"Вы хотите удалить соединение с устройством %c:?"
"Do you want to disconnect from the drive %c:?"

MChangeDriveDisconnectMapped
"На устройство %c: отображена папка"
"The drive %c: is mapped to..."

MChangeDriveDisconnectReconnect
"&Восстанавливать при входе в систему"
"&Reconnect at Logon"

MChangeDriveAskDisconnect
l:
"Вы хотите в любом случае отключиться от устройства?"
"Do you want to disconnect the device anyway?"

MChangeVolumeInUse
"Не удается извлечь диск из привода %c:"
"Volume %c: cannot be ejected."

MChangeVolumeInUse2
"Используется другим приложением"
"It is used by another application"

MChangeWaitingLoadDisk
ls:;"Ожидание загрузки диска..."
"Ожидание чтения диска..."
ls:;"Waiting for disk to load..."
"Waiting for disk to mount..."

MChangeCouldNotUnlockMedia
"Невозможно разблокировать привод %c:"
"Could not unlock media from drive %c:"

MChangeCouldNotEjectMedia
"Невозможно извлечь диск из привода %c:"
"Could not eject media from drive %c:"

MAdditionalHotKey
"#!$%*+-/(),."
"#!$%*+-/(),."

MSearchFileTitle
l:
" Поиск "
" Search "

MCannotCreateListFile
"Ошибка создания списка файлов"
"Cannot create list file"

MCannotCreateListTemp
"(невозможно создать временный файл для списка)"
"(cannot create temporary file for list)"

MCannotCreateListWrite
"(невозможно записать данные в файл)"
"(cannot write data in file)"

MDragFiles
l:
"%d файлов"
"%d files"

MDragMove
"Перенос %s"
"Move %s"

MDragCopy
"Копирование %s"
"Copy %s"

MProcessListTitle
l:
"Список задач"
"Task list"

MProcessListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"

MKillProcessTitle
"Удаление задачи"
"Kill task"

MAskKillProcess
"Вы хотите удалить выбранную задачу?"
"Do you wish to kill selected task?"

MKillProcessWarning
"Вы потеряете всю несохраненную информацию этой программы"
"You will lose any unsaved information in this program"

MKillProcessKill
"Удалить"
"Kill"

MCannotKillProcess
"Указанную задачу удалить не удалось"
"Cannot kill the specified task"

MCannotKillProcessPerm
"Вы не имеет права удалить этот процесс."
"You have no permission to kill this process."

MQuickViewTitle
l:
"Быстрый просмотр"
"Quick view"

MQuickViewFolder
"Папка \"%s\""
"Folder \"%s\""

MQuickViewJunction
"Связь \"%s\""
"Junction \"%s\""

MQuickViewSymlink
"Ссылка \"%s\""
"Symlink \"%s\""

MQuickViewVolMount
"Том \"%s\""
"Volume \"%s\""

MQuickViewContains
"Содержит:"
"Contains:"

MQuickViewFolders
"Папок               "
"Folders          "

MQuickViewFiles
"Файлов              "
"Files            "

MQuickViewBytes
"Размер файлов       "
"Files size       "

MQuickViewCompressed
"Упакованный размер  "
"Compressed size  "

MQuickViewRatio
"Степень сжатия      "
"Ratio            "

MQuickViewCluster
"Размер кластера     "
"Cluster size     "

MQuickViewRealSize
"Реальный размер     "
"Real files size  "

MQuickViewSlack
"Остатки кластеров   "
"Files slack      "

MSetAttrTitle
l:
"Атрибуты"
"Attributes"

MSetAttrFor
"Изменить файловые атрибуты для"
"Change file attributes for"

MSetAttrSelectedObjects
"выбранных объектов"
"selected objects"

MSetAttrJunction
"Связь \"%s\""
"Junction \"%s\""

MSetAttrSymlink
"Ссылка \"%s\""
"Symlink \"%s\""

MSetAttrVolMount
"Том \"%s\""
"Volume \"%s\""

MSetAttrUnknownJunction
"(нет данных)"
"(data not available)"

MSetAttrRO
"&Только для чтения"
"&Read only"

MSetAttrArchive
"&Архивный"
"&Archive"

MSetAttrHidden
"&Скрытый"
"&Hidden"

MSetAttrSystem
"С&истемный"
"&System"

MSetAttrCompressed
"Сжаты&й"
"&Compressed"

MSetAttrEncrypted
"За&шифрованный"
"&Encrypted"

MSetAttrNotIndexed
"Н&еиндексируемый"
"Not &Indexed"

MSetAttrSparse
"Разреженный"
"Sparse"

MSetAttrTemp
"Временный"
"Temporary"

MSetAttrOffline
"Автономный"
"Offline"

MSetAttrVirtual
"Виртуальный"
"Virtual"

MSetAttrSubfolders
"Обрабатывать &вложенные папки"
"Process sub&folders"

MSetAttrModification
"Время &модификации файла:"
"File &modification time:"

MSetAttrCreation
"Время со&здания файла:"
"File crea&tion time:"

MSetAttrLastAccess
"Время последнего &доступа к файлу:"
"&Last file access time:"

MSetAttrOriginal
"Исход&ное"
"&Original"

MSetAttrCurrent
"Те&кущее"
"Curre&nt"

MSetAttrBlank
"Сбр&ос"
"&Blank"

MSetAttrSet
"Установить"
"Set"

MSetAttrTimeTitle1
l:
"ММ%cДД%cГГГГ чч%cмм%cсс"
"MM%cDD%cYYYY hh%cmm%css"

MSetAttrTimeTitle2
"ДД%cММ%cГГГГ чч%cмм%cсс"
"DD%cMM%cYYYY hh%cmm%css"

MSetAttrTimeTitle3
"ГГГГ%cММ%cДД чч%cмм%cсс"
"YYYY%cMM%cDD hh%cmm%css"

MSetAttrSetting
l:
"Установка файловых атрибутов для"
"Setting file attributes for"

MSetAttrCannotFor
"Ошибка установки атрибутов для"
"Cannot set attributes for"

MSetAttrCompressedCannotFor
"Не удалось установить атрибут СЖАТЫЙ для"
"Cannot set attribute COMPRESSED for"

MSetAttrEncryptedCannotFor
"Не удалось установить атрибут ЗАШИФРОВАННЫЙ для"
"Cannot set attribute ENCRYPTED for"

MSetAttrTimeCannotFor
"Не удалось установить время файла для"
"Cannot set file time for"

MSetColorPanel
l:
"&Панель"
"&Panel"

MSetColorDialog
"&Диалог"
"&Dialog"

MSetColorWarning
"Пр&едупреждение"
"&Warning message"

MSetColorMenu
"&Меню"
"&Menu"

MSetColorHMenu
"&Горизонтальное меню"
"Hori&zontal menu"

MSetColorKeyBar
"&Линейка клавиш"
"&Key bar"

MSetColorCommandLine
"&Командная строка"
"&Command line"

MSetColorClock
"&Часы"
"C&lock"

MSetColorViewer
"Про&смотрщик"
"&Viewer"

MSetColorEditor
"&Редактор"
"&Editor"

MSetColorHelp
"П&омощь"
"&Help"

MSetDefaultColors
"&Установить стандартные цвета"
"Set de&fault colors"

MSetBW
"Черно-бел&ый режим"
"&Black and white mode"

MSetColorPanelNormal
l:
"Обычный текст"
"Normal text"

MSetColorPanelSelected
"Выбранный текст"
"Selected text"

MSetColorPanelHighlightedInfo
"Выделенная информация"
"Highlighted info"

MSetColorPanelDragging
"Перетаскиваемый текст"
"Dragging text"

MSetColorPanelBox
"Рамка"
"Border"

MSetColorPanelNormalCursor
"Обычный курсор"
"Normal cursor"

MSetColorPanelSelectedCursor
"Выделенный курсор"
"Selected cursor"

MSetColorPanelNormalTitle
"Обычный заголовок"
"Normal title"

MSetColorPanelSelectedTitle
"Выделенный заголовок"
"Selected title"

MSetColorPanelColumnTitle
"Заголовок колонки"
"Column title"

MSetColorPanelTotalInfo
"Количество файлов"
"Total info"

MSetColorPanelSelectedInfo
"Количество выбранных файлов"
"Selected info"

MSetColorPanelScrollbar
"Полоса прокрутки"
"Scrollbar"

MSetColorPanelScreensNumber
"Количество фоновых экранов"
"Number of background screens"

MSetColorDialogNormal
l:
"Обычный текст"
"Normal text"

MSetColorDialogHighlighted
"Выделенный текст"
"Highlighted text"

MSetColorDialogDisabled
"Блокированный текст"
"Disabled text"

MSetColorDialogBox
"Рамка"
"Border"

MSetColorDialogBoxTitle
"Заголовок рамки"
"Title"

MSetColorDialogHighlightedBoxTitle
"Выделенный заголовок рамки"
"Highlighted title"

MSetColorDialogTextInput
"Ввод текста"
"Text input"

MSetColorDialogUnchangedTextInput
"Неизмененный текст"
"Unchanged text input"

MSetColorDialogSelectedTextInput
"Ввод выделенного текста"
"Selected text input"

MSetColorDialogEditDisabled
"Блокированное поле ввода"
"Disabled input line"

MSetColorDialogButtons
"Кнопки"
"Buttons"

MSetColorDialogSelectedButtons
"Выбранные кнопки"
"Selected buttons"

MSetColorDialogHighlightedButtons
"Выделенные кнопки"
"Highlighted buttons"

MSetColorDialogSelectedHighlightedButtons
"Выбранные выделенные кнопки"
"Selected highlighted buttons"

MSetColorDialogListBoxControl
"Список"
"List box"

MSetColorDialogComboBoxControl
"Комбинированный список"
"Combobox"

MSetColorDialogListText
l:
"Обычный текст"
"Normal text"

MSetColorDialogListSelectedText
"Выбранный текст"
"Selected text"

MSetColorDialogListHighLight
"Выделенный текст"
"Highlighted text"

MSetColorDialogListSelectedHighLight
"Выбранный выделенный текст"
"Selected highlighted text"

MSetColorDialogListDisabled
"Блокированный пункт"
"Disabled item"

MSetColorDialogListBox
"Рамка"
"Border"

MSetColorDialogListTitle
"Заголовок"
"Title"

MSetColorDialogListGrayed
"Серый текст списка"
"Grayed list text"

MSetColorDialogSelectedListGrayed
"Выбранный серый текст списка"
"Selected grayed list text"

MSetColorDialogListScrollBar
"Полоса прокрутки"
"Scrollbar"

MSetColorDialogListArrows
"Индикаторы длинных строк"
"Long string indicators"

MSetColorDialogListArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"

MSetColorDialogListArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"

MSetColorMenuNormal
l:
"Обычный текст"
"Normal text"

MSetColorMenuSelected
"Выбранный текст"
"Selected text"

MSetColorMenuHighlighted
"Выделенный текст"
"Highlighted text"

MSetColorMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"

MSetColorMenuDisabled
"Недоступный пункт"
"Disabled text"

MSetColorMenuGrayed
"Серый текст"
"Grayed text"

MSetColorMenuSelectedGrayed
"Выбранный серый текст"
"Selected grayed text"

MSetColorMenuBox
"Рамка"
"Border"

MSetColorMenuTitle
"Заголовок"
"Title"

MSetColorMenuScrollBar
"Полоса прокрутки"
"Scrollbar"

MSetColorMenuArrows
"Индикаторы длинных строк"
"Long string indicators"

MSetColorMenuArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"

MSetColorMenuArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"

MSetColorHMenuNormal
l:
"Обычный текст"
"Normal text"

MSetColorHMenuSelected
"Выбранный текст"
"Selected text"

MSetColorHMenuHighlighted
"Выделенный текст"
"Highlighted text"

MSetColorHMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"

MSetColorKeyBarNumbers
l:
"Номера клавиш"
"Key numbers"

MSetColorKeyBarNames
"Названия клавиш"
"Key names"

MSetColorKeyBarBackground
"Фон"
"Background"

MSetColorCommandLineNormal
l:
"Обычный текст"
"Normal text"

MSetColorCommandLineSelected
"Выделенный текст"
"Selected text input"

MSetColorCommandLinePrefix
"Текст префикса"
"Prefix text"

MSetColorCommandLineUserScreen
"Пользовательский экран"
"User screen"

MSetColorClockNormal
l:
"Обычный текст (панели)"
"Normal text (Panel)"

MSetColorClockNormalEditor
"Обычный текст (редактор)"
"Normal text (Editor)"

MSetColorClockNormalViewer
"Обычный текст (вьювер)"
"Normal text (Viewer)"

MSetColorViewerNormal
l:
"Обычный текст"
"Normal text"

MSetColorViewerSelected
"Выбранный текст"
"Selected text"

MSetColorViewerStatus
"Статус"
"Status line"

MSetColorViewerArrows
"Стрелки сдвига экрана"
"Screen scrolling arrows"

MSetColorViewerScrollbar
"Полоса прокрутки"
"Scrollbar"

MSetColorEditorNormal
l:
"Обычный текст"
"Normal text"

MSetColorEditorSelected
"Выбранный текст"
"Selected text"

MSetColorEditorStatus
"Статус"
"Status line"

MSetColorEditorScrollbar
"Полоса прокрутки"
"Scrollbar"

MSetColorHelpNormal
l:
"Обычный текст"
"Normal text"

MSetColorHelpHighlighted
"Выделенный текст"
"Highlighted text"

MSetColorHelpReference
"Ссылка"
"Reference"

MSetColorHelpSelectedReference
"Выбранная ссылка"
"Selected reference"

MSetColorHelpBox
"Рамка"
"Border"

MSetColorHelpBoxTitle
"Заголовок рамки"
"Title"

MSetColorHelpScrollbar
"Полоса прокрутки"
"Scrollbar"

MSetColorGroupsTitle
l:
"Цветовые группы"
"Color groups"

MSetColorItemsTitle
"Элементы группы"
"Group items"

MSetColorTitle
l:
"Цвет"
"Color"

MSetColorForeground
"&Текст"
"&Foreground"

MSetColorBackground
"&Фон"
"&Background"

MSetColorForeTransparent
"&Прозрачный"
"&Transparent"

MSetColorBackTransparent
"П&розрачный"
"T&ransparent"

MSetColorSample
"Текст Текст Текст Текст Текст Текст"
"Text Text Text Text Text Text Text"

MSetColorSet
"Установить"
"Set"

MSetColorCancel
"Отменить"
"Cancel"

MSetConfirmTitle
l:
"Подтверждения"
"Confirmations"

MSetConfirmCopy
"Перезапись файлов при &копировании"
"&Copy"

MSetConfirmMove
"Перезапись файлов при &переносе"
"&Move"

MSetConfirmDrag
"Пере&таскивание"
"&Drag and drop"

MSetConfirmDelete
"&Удаление"
"De&lete"

MSetConfirmDeleteFolders
"У&даление непустых папок"
"Delete non-empty &folders"

MSetConfirmEsc
"Прерыва&ние операций"
"&Interrupt operation"

MSetConfirmRemoveConnection
"&Отключение сетевого устройства"
"Disconnect &network drive"

MSetConfirmRemoveSUBST
"Отключение SUBST-диска"
"Disconnect &SUBST-disk"

MSetConfirmRemoveHotPlug
"Отключение HotPlug-устройства"
"HotPlug-device removal"

MSetConfirmAllowReedit
"Повто&рное открытие файла в редакторе"
"&Reload edited file"

MSetConfirmHistoryClear
"Очистка списка &истории"
"Clear &history list"

MSetConfirmExit
"&Выход"
"E&xit"

MFindFolderTitle
l:
"Поиск папки"
"Find folder"

MKBFolderTreeF1
l:
l:// Find folder Tree KeyBar
"Помощь"
"Help"

MKBFolderTreeF2
"Обновить"
"Rescan"

MKBFolderTreeF5
"Размер"
"Zoom"

MKBFolderTreeF10
"Выход"
"Quit"

MKBFolderTreeAltF9
"Видео"
"Video"

MTreeTitle
"Дерево"
"Tree"

MCannotSaveTree
"Ошибка записи дерева папок в файл"
"Cannot save folders tree to file"

MReadingTree
"Чтение дерева папок"
"Reading the folders tree"

MUserMenuTitle
l:
"Пользовательское меню"
"User menu"

MChooseMenuType
"Выберите тип пользовательского меню для редактирования"
"Choose user menu type to edit"

MChooseMenuMain
"&Главное"
"&Main"

MChooseMenuLocal
"&Местное"
"&Local"

MMainMenuTitle
"Главное меню"
"Main menu"

MMainMenuFAR
"Папка FAR"
"FAR folder"

MMainMenuREG
l:
l:// <...menu (Registry)>
"Реестр"
"Registry"

MLocalMenuTitle
"Местное меню"
"Local menu"

MMainMenuBottomTitle
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"

MAskDeleteMenuItem
"Вы хотите удалить пункт меню"
"Do you wish to delete the menu item"

MAskDeleteSubMenuItem
"Вы хотите удалить вложенное меню"
"Do you wish to delete the submenu"

MUserMenuInvalidInputLabel
"Неправильный формат метки меню!"
"Invalid format for UserMenu Label!"

MUserMenuInvalidInputHotKey
"Неправильный формат горячей клавиши!"
"Invalid format for Hot Key!"

MEditMenuTitle
l:
"Редактирование пользовательского меню"
"Edit user menu"

MEditMenuHotKey
"&Горячая клавиша:"
"&Hot key:"

MEditMenuLabel
"&Метка:"
"&Label:"

MEditMenuCommands
"&Команды:"
"&Commands:"

MAskInsertMenuOrCommand
l:
"Вы хотите вставить новую команду или новое меню?"
"Do you wish to insert a new command or a new menu?"

MMenuInsertCommand
"Вставить команду"
"Insert command"

MMenuInsertMenu
"Вставить меню"
"Insert menu"

MEditSubmenuTitle
l:
"Редактирование метки вложенного меню"
"Edit submenu label"

MEditSubmenuHotKey
"&Горячая клавиша:"
"&Hot key:"

MEditSubmenuLabel
"&Метка:"
"&Label:"

MViewerTitle
l:
"Просмотр"
"Viewer"

MViewerCannotOpenFile
"Ошибка открытия файла"
"Cannot open the file"

MViewerStatusCol
"Кол"
"Col"

MViewSearchTitle
l:
"Поиск"
"Search"

MViewSearchFor
"&Искать"
"&Search for"

MViewSearchForText
"Искать &текст"
"Search for &text"

MViewSearchForHex
"Искать 16-ричный &код"
"Search for &hex"

MViewSearchCase
"&Учитывать регистр"
"&Case sensitive"

MViewSearchWholeWords
"Только &целые слова"
"&Whole words"

MViewSearchReverse
"Обратн&ый поиск"
"Re&verse search"

MViewSearchSearch
"Искать"
"Search"

MViewSearchCancel
"Отменить"
"Cancel"

MViewSearchingFor
l:
"Поиск"
"Searching for"

MViewSearchingHex
"Поиск байтов"
"Searching for bytes"

MViewSearchCannotFind
"Строка не найдена"
"Could not find the string"

MViewSearchCannotFindHex
"Байты не найдены"
"Could not find the bytes"

MViewSearchFromBegin
"Продолжить поиск с начала документа?"
"Continue the search from the beginning of the document?"

MViewSearchFromEnd
"Продолжить поиск с конца документа?"
"Continue the search from the end of the document?"

MPrintTitle
l:
"Печать"
"Print"

MPrintTo
"Печатать %s на"
"Print %s to"

MPrintFilesTo
"Печатать %d файлов на"
"Print %d files to"

MPreparingForPrinting
"Подготовка файлов к печати"
"Preparing files for printing"

MJobs
"заданий"
"jobs"

MCannotOpenPrinter
"Не удалось открыть принтер"
"Cannot open printer"

MCannotPrint
"Не удалось распечатать"
"Cannot print"

MDescribeFiles
l:
"Описание файла"
"Describe file"

MEnterDescription
"Введите описание %s"
"Enter %s description"

MReadingDiz
l:
"Чтение описаний файлов"
"Reading file descriptions"

MCannotUpdateDiz
"Не удалось обновить описания файлов"
"Cannot update file descriptions"

MCannotUpdateRODiz
"Файл описаний защищен от записи"
"The description file is read only"

MCfgDizTitle
l:
"Описания файлов"
"File descriptions"

MCfgDizListNames
"Имена &списков описаний, разделенные запятыми:"
"Description &list names delimited with commas:"

MCfgDizSetHidden
"Устанавливать &атрибут ""Hidden"" на новые списки описаний"
"Set ""&Hidden"" attribute to new description lists"

MCfgDizROUpdate
"Обновлять файл описаний с атрибутом ""Толь&ко для чтения"""
"Update &read only description file"

MCfgDizStartPos
"&Позиция новых описаний в строке"
"&Position of new descriptions in the string"

MCfgDizNotUpdate
"&Не обновлять описания"
"Do &not update descriptions"

MCfgDizUpdateIfDisplayed
"&Обновлять, если они выводятся на экран"
"Update if &displayed"

MCfgDizAlwaysUpdate
"&Всегда обновлять"
"&Always update"

MReadingTitleFiles
l:
"Обновление панелей"
"Update of panels"

MReadingFiles
"Чтение: %d файлов"
"Reading: %d files"

MUserBreakTitle
l:
"Прекращено пользователем"
"User break"

MOperationNotCompleted
"Операция не завершена"
"Operation not completed"

MEditPanelModes
l:
"Режимы панели"
"Edit panel modes"

MEditPanelModesBrief
l:
"&Краткий режим"
"&Brief mode"

MEditPanelModesMedium
"&Средний режим"
"&Medium mode"

MEditPanelModesFull
"&Полный режим"
"&Full mode"

MEditPanelModesWide
"&Широкий режим"
"&Wide mode"

MEditPanelModesDetailed
"&Детальный режим"
"Detai&led mode"

MEditPanelModesDiz
"&Описания"
"&Descriptions mode"

MEditPanelModesLongDiz
"Д&линные описания"
"Lon&g descriptions mode"

MEditPanelModesOwners
"Вл&адельцы файлов"
"File own&ers mode"

MEditPanelModesLinks
"Свя&зи файлов"
"Lin&ks mode"

MEditPanelModesAlternative
"Аль&тернативный полный режим"
"&Alternative full mode"

MEditPanelModeTypes
l:
"&Типы колонок"
"Column &types"

MEditPanelModeWidths
"&Ширина колонок"
"Column &widths"

MEditPanelModeStatusTypes
"Типы колонок строки ст&атуса"
"St&atus line column types"

MEditPanelModeStatusWidths
"Ширина колонок строки стат&уса"
"Status l&ine column widths"

MEditPanelModeFullscreen
"&Полноэкранный режим"
"&Fullscreen view"

MEditPanelModeAlignExtensions
"&Выравнивать расширения файлов"
"Align file &extensions"

MEditPanelModeAlignFolderExtensions
"Выравнивать расширения пап&ок"
"Align folder e&xtensions"

MEditPanelModeFoldersUpperCase
"Показывать папки &заглавными буквами"
"Show folders in &uppercase"

MEditPanelModeFilesLowerCase
"Показывать файлы ст&рочными буквами"
"Show files in &lowercase"

MEditPanelModeUpperToLowerCase
"Показывать имена файлов из заглавных букв &строчными буквами"
"Show uppercase file names in lower&case"

MEditPanelModeCaseSensitiveSort
"Использовать р&егистрозависимую сортировку"
"Use case &sensitive sort"

MEditPanelReadHelp
" Нажмите F1, чтобы получить информацию по настройке "
" Read online help for instructions "

MSetFolderInfoTitle
l:
"Файлы информации о папках"
"Folder description files"

MSetFolderInfoNames
"Введите имена файлов, разделенные запятыми (допускаются маски)"
"Enter file names delimited with commas (wildcards are allowed)"

MScreensTitle
l:
"Экраны"
"Screens"

MScreensPanels
"Панели"
"Panels"

MScreensView
"Просмотр"
"View"

MScreensEdit
"Редактор"
"Edit"

MAskApplyCommandTitle
l:
"Применить команду"
"Apply command"

MAskApplyCommand
"Введите команду для обработки выбранных файлов"
"Enter command to process selected files"

MPluginConfigTitle
l:
"Конфигурация модулей"
"Plugins configuration"

MPluginCommandsMenuTitle
"Команды внешних модулей"
"Plugin commands"

MPreparingList
l:
"Создание списка файлов"
"Preparing files list"

MLangTitle
l:
"Основной язык"
"Main language"

MHelpLangTitle
"Язык помощи"
"Help language"

MDefineMacroTitle
l:
"Задание макрокоманды"
"Define macro"

MDefineMacro
"Нажмите желаемую клавишу"
"Press the desired key"

MMacroReDefinedKey
"Макроклавиша '%s' уже определена."
"Macro key '%s' already defined."

MMacroDeleteAssign
"Макроклавиша '%s' не активна."
"Macro key '%s' is not active."

MMacroDeleteKey
"Макроклавиша '%s' будет удалена."
"Macro key '%s' will be removed."

MMacroCommonReDefinedKey
"Общая макроклавиша '%s' уже определена."
"Common macro key '%s' already defined."

MMacroCommonDeleteAssign
"Общая макроклавиша '%s' не активна."
"Common macro key '%s' is not active."

MMacroCommonDeleteKey
"Общая макроклавиша '%s' будет удалена."
"Common macro key '%s' will be removed."

MMacroSequence
"Последовательность:"
"Sequence:"

MMacroReDefinedKey2
"Переопределить?"
"Redefine?"

MMacroDeleteKey2
"Удалить?"
"Delete?"

MMacroDisDisabledKey
"(макроклавиша не активна)"
"(macro key is not active)"

MMacroDisOverwrite
"Переопределить"
"Overwrite"

MMacroDisAnotherKey
"Изменить клавишу"
"Try another key"

MMacroSettingsTitle
l:
"Параметры макрокоманды для '%s'"
"Macro settings for '%s'"

MMacroSettingsEnableOutput
"Разрешить во время &выполнения вывод на экран"
"Allo&w screen output while executing macro"

MMacroSettingsRunAfterStart
"В&ыполнять после запуска FAR"
"Execute after FAR &start"

MMacroSettingsActivePanel
"&Активная панель"
"&Active panel"

MMacroSettingsPassivePanel
"&Пассивная панель"
"&Passive panel"

MMacroSettingsPluginPanel
"На панели пла&гина"
"P&lugin panel"

MMacroSettingsFolders
"Выполнять для папо&к"
"Execute for &folders"

MMacroSettingsSelectionPresent
"&Отмечены файлы"
"Se&lection present"

MMacroSettingsCommandLine
"Пустая командная &строка"
"Empty &command line"

MMacroSettingsSelectionBlockPresent
"Отмечен б&лок"
"Selection &block present"

MMacroPErrUnrecognized_keyword
l:
"Неизвестное ключевое слово '%s'"
"Unrecognized keyword '%s'"

MMacroPErrUnrecognized_function
"Неизвестная функция '%s'"
"Unrecognized function '%s'"

MMacroPErrFuncParam
"Неверное количество параметров у функции '%s'"
"Incorrect number of arguments for function '%s'"

MMacroPErrNot_expected_ELSE
"Неожиданное появление $Else"
"Unexpected $Else"

MMacroPErrNot_expected_END
"Неожиданное появление $End"
"Unexpected $End"

MMacroPErrUnexpected_EOS
"Неожиданный конец строки"
"Unexpected end of source string"

MMacroPErrExpected
"Ожидается '%s'"
"Expected '%s'"

MMacroPErrBad_Hex_Control_Char
"Неизвестный шестнадцатеричный управляющий символ"
"Bad Hex Control Char"

MMacroPErrBad_Control_Char
"Неправильный управляющий символ"
"Bad Control Char"

MMacroPErrVar_Expected
"Переменная '%s' не найдена"
"Variable Expected '%s'"

MMacroPErrExpr_Expected
"Ошибка синтаксиса"
"Expression Expected"

MCannotSaveFile
l:
"Ошибка сохранения файла"
"Cannot save file"

MTextSavedToTemp
"Отредактированный текст записан в"
"Edited text is stored in"

MMonthJan
l:
"Янв"
"Jan"

MMonthFeb
"Фев"
"Feb"

MMonthMar
"Мар"
"Mar"

MMonthApr
"Апр"
"Apr"

MMonthMay
"Май"
"May"

MMonthJun
"Июн"
"Jun"

MMonthJul
"Июл"
"Jul"

MMonthAug
"Авг"
"Aug"

MMonthSep
"Сен"
"Sep"

MMonthOct
"Окт"
"Oct"

MMonthNov
"Ноя"
"Nov"

MMonthDec
"Дек"
"Dec"

MPluginHotKeyTitle
l:
"Назначение горячей клавиши"
"Assign plugin hot key"

MPluginHotKey
"Введите горячую клавишу (букву или цифру)"
"Enter hot key (letter or digit)"

MPluginHotKeyBottom
"F4 - задать горячую клавишу"
"F4 - set hot key"

MRightCtrl
l:
"ПравыйCtrl"
"RightCtrl"

MViewerGoTo
l:
"Перейти"
"Go to"

MGoToPercent
"&Процент"
"&Percent"

MGoToHex
"16-ричное &смещение"
"&Hex offset"

MGoToDecimal
"10-ичное с&мещение"
"&Decimal offset"

MExceptTitleFAR
l:
"Внутренняя ошибка"
"Internal error"

MExceptTitleLoad
"Ошибка загрузки плагина"
"Plugin load error"

MExceptTitle
"Ошибка вызова плагина"
"Plugin call error"

MExcTrappedException
"Исключительная ситуация:"
"Exception occurred:"

MExcCheckOnLousys
"Передана некорректная информация из модуля:"
"Incorrect information is passed from module:"

MExcStructWrongFilled
"(некорректно заполнены поля структуры <%s>)"
"(the fields of structure <%s> are wrong filled)"

MExcStructField
"(структура <%s>, поле <%s>)"
"(structure <%s>, field <%s>)"

MExcInvalidFuncResult
"Функция <%s> вернула недопустимое значение"
"Function <%s> has returned illegal value"

MExcAddress
"Адрес исключения - 0x%p, модуль:"
"Exception address: 0x%p in module:"

MExcFARTerminateYes
"FAR Manager завершит работу!"
"FAR Manager will be terminated!"

MExcUnloadYes
"Плагин будет выгружен!"
"The plugin will be Unloaded!"

MExcRAccess
"\"Нарушение доступа (чтение из 0x%p)\""
"\"Access violation (read from 0x%p)\""

MExcWAccess
"\"Нарушение доступа (запись в 0x%p)\""
"\"Access violation (write to 0x%p)\""

MExcEAccess
"\"Нарушение доступа (исполнение кода из 0x%p)\""
"\"Access violation (execute at 0x%p)\""

MExcOutOfBounds
"\"Попытка доступа к элементу за границами массива\""
"\"Array out of bounds\""

MExcDivideByZero
"\"Деление на нуль\""
"\"Divide by zero\""

MExcStackOverflow
"\"Переполнение стека\""
"\"Stack Overflow\""

MExcBreakPoint
"\"Точка останова\""
"\"Breakpoint exception\""

MExcFloatDivideByZero
"\"Деление на нуль при операции с плавающей точкой\""
"\"Floating-point divide by zero\""

MExcFloatOverflow
"\"Переполнение при операции с плавающей точкой\""
"\"Floating point operation overflow\""

MExcFloatStackOverflow
"\"Стек регистров сопроцессора полон или пуст\""
"\"Floating point stack empty or full\""

MExcFloatUnderflow
"\"Потеря точности при операции с плавающей точкой\""
"\"Floating point operation underflow\""

MExcBadInstruction
"\"Недопустимая инструкция\""
"\"Illegal instruction\""

MExcDatatypeMisalignment
"\"Попытка доступа к невыравненным данным\""
"\"Alignment fault\""

MExcUnknown
"\"Неизвестное исключение\""
"\"Unknown exception\""

MExcDebugger
"Отладчик"
"Debugger"

MNetUserName
l:
"Имя пользователя"
"User name"

MNetUserPassword
"Пароль пользователя"
"User password"

MReadFolderError
l:
"Не удается прочесть содержимое папки"
"Cannot read folder contents"

MPlgBadVers
l:
"Этот модуль требует FAR более высокой версии"
"This plugin requires higher FAR version"

MPlgRequired
"Требуется версия FAR - %d.%d.%d."
"Required FAR version is %d.%d.%d."

MPlgRequired2
"Текущая версия FAR - %d.%d.%d."
"Current FAR version is %d.%d.%d"

MPlgLoadPluginError
"Ошибка при загрузке плагина"
"Error loading plugin module"

MBuffSizeTooSmall_1
l:
"Буфер, размещенный под имя файла слишком мал."
"Buffer allocated for file name is too small."

MBuffSizeTooSmall_2
"Требуется %d байт, а имеется только %d"
"%d bytes are required, but only %d bytes were allocated."

MFileNameExceedSystem
"Длина имени файла превышает системные ограничения."
"File name length exceeds system limitations."

MCheckBox2State
l:
"?"
"?"

MEditInputSize1
"Длина поля"
"Field"

MEditInputSize2
"будет уменьшена до %d байт."
"will be truncated to %d bytes."

MHelpTitle
l:
"Помощь"
"Help"

MHelpActivatorURL
"Эта ссылка запускает внешнее приложение:"
"This reference starts the external application:"

MHelpActivatorFormat
"с параметром:"
"with parameter:"

MHelpActivatorQ
"Желаете запустить?"
"Do you wish to start it?"

MCannotOpenHelp
"Ошибка открытия файла"
"Cannot open the file"

MHelpTopicNotFound
"Не найден запрошенный раздел помощи:"
"Requested help topic not found:"

MPluginsHelpTitle
l:
"Внешние модули"
"Plugins help"

MDocumentsHelpTitle
"Документы"
"Documents help"

MHelpSearchTitle
l:
"Поиск"
"Search"

MHelpSearchingFor
"Поиск для"
"Searching for"

MHelpSearchCannotFind
"Строка не найдена"
"Could not find the string"

MHelpF1
l:
l:// Help KeyBar F1-12
"Помощь"
"Help"

MHelpF2
""
""

MHelpF3
""
""

MHelpF4
""
""

MHelpF5
"Размер"
"Zoom"

MHelpF6
""
""

MHelpF7
"Поиск"
"Search"

MHelpF8
""
""

MHelpF9
""
""

MHelpF10
"Выход"
"Quit"

MHelpF11
""
""

MHelpF12
""
""

MHelpShiftF1
l:
l:// Help KeyBar Shift-F1-12
"Содерж"
"Index"

MHelpShiftF2
"Плагин"
"Plugin"

MHelpShiftF3
"Докум"
"Docums"

MHelpShiftF4
""
""

MHelpShiftF5
""
""

MHelpShiftF6
""
""

MHelpShiftF7
"Дальше"
"Next"

MHelpShiftF8
""
""

MHelpShiftF9
""
""

MHelpShiftF10
""
""

MHelpShiftF11
""
""

MHelpShiftF12
""
""

MHelpAltF1
l:
l:// Help KeyBar Alt-F1-12
"Пред."
"Prev"

MHelpAltF2
""
""

MHelpAltF3
""
""

MHelpAltF4
""
""

MHelpAltF5
""
""

MHelpAltF6
""
""

MHelpAltF7
""
""

MHelpAltF8
""
""

MHelpAltF9
"Видео"
"Video"

MHelpAltF10
""
""

MHelpAltF11
""
""

MHelpAltF12
""
""

MHelpCtrlF1
l:
l:// Help KeyBar Ctrl-F1-12
""
""

MHelpCtrlF2
""
""

MHelpCtrlF3
""
""

MHelpCtrlF4
""
""

MHelpCtrlF5
""
""

MHelpCtrlF6
""
""

MHelpCtrlF7
""
""

MHelpCtrlF8
""
""

MHelpCtrlF9
""
""

MHelpCtrlF10
""
""

MHelpCtrlF11
""
""

MHelpCtrlF12
""
""

MHelpCtrlShiftF1
l:
l:// Help KeyBar CtrlShiftF1-12
""
""

MHelpCtrlShiftF2
""
""

MHelpCtrlShiftF3
""
""

MHelpCtrlShiftF4
""
""

MHelpCtrlShiftF5
""
""

MHelpCtrlShiftF6
""
""

MHelpCtrlShiftF7
""
""

MHelpCtrlShiftF8
""
""

MHelpCtrlShiftF9
""
""

MHelpCtrlShiftF10
""
""

MHelpCtrlShiftF11
""
""

MHelpCtrlShiftF12
""
""

MHelpCtrlAltF1
l:
l:// Help KeyBar CtrlAltF1-12
""
""

MHelpCtrlAltF2
""
""

MHelpCtrlAltF3
""
""

MHelpCtrlAltF4
""
""

MHelpCtrlAltF5
""
""

MHelpCtrlAltF6
""
""

MHelpCtrlAltF7
""
""

MHelpCtrlAltF8
""
""

MHelpCtrlAltF9
""
""

MHelpCtrlAltF10
""
""

MHelpCtrlAltF11
""
""

MHelpCtrlAltF12
""
""

MHelpAltShiftF1
l:
l:// Help KeyBar AltShiftF1-12
""
""

MHelpAltShiftF2
""
""

MHelpAltShiftF3
""
""

MHelpAltShiftF4
""
""

MHelpAltShiftF5
""
""

MHelpAltShiftF6
""
""

MHelpAltShiftF7
""
""

MHelpAltShiftF8
""
""

MHelpAltShiftF9
""
""

MHelpAltShiftF10
""
""

MHelpAltShiftF11
""
""

MHelpAltShiftF12
""
""

MHelpCtrlAltShiftF1
l:
l:// Help KeyBar CtrlAltShiftF1-12
""
""

MHelpCtrlAltShiftF2
""
""

MHelpCtrlAltShiftF3
""
""

MHelpCtrlAltShiftF4
""
""

MHelpCtrlAltShiftF5
""
""

MHelpCtrlAltShiftF6
""
""

MHelpCtrlAltShiftF7
""
""

MHelpCtrlAltShiftF8
""
""

MHelpCtrlAltShiftF9
""
""

MHelpCtrlAltShiftF10
""
""

MHelpCtrlAltShiftF11
""
""

MHelpCtrlAltShiftF12
""
""

MInfoF1
l:
l:// InfoPanel KeyBar F1-F12
"Помощь"
"Help"

MInfoF2
"Сверн"
"Wrap"

MInfoF3
"СмОпис"
"VieDiz"

MInfoF4
"РедОпи"
"EdtDiz"

MInfoF5
""
""

MInfoF6
""
""

MInfoF7
"Поиск"
"Search"

MInfoF8
"Win"
"Win"

MInfoF9
"КонфМн"
"ConfMn"

MInfoF10
"Выход"
"Quit"

MInfoF11
"Модули"
"Plugin"

MInfoF12
"Экраны"
"Screen"

MInfoShiftF1
l:
l:// InfoPanel KeyBar Shift-F1-F12
""
""

MInfoShiftF2
"Слова"
"WWrap"

MInfoShiftF3
""
""

MInfoShiftF4
""
""

MInfoShiftF5
""
""

MInfoShiftF6
""
""

MInfoShiftF7
"Дальше"
"Next"

MInfoShiftF8
"Таблиц"
"Table"

MInfoShiftF9
"Сохран"
"Save"

MInfoShiftF10
"Послдн"
"Last"

MInfoShiftF11
""
""

MInfoShiftF12
""
""

MInfoAltF1
l:
l:// InfoPanel KeyBar Alt-F1-F12
"Левая"
"Left"

MInfoAltF2
"Правая"
"Right"

MInfoAltF3
""
""

MInfoAltF4
""
""

MInfoAltF5
""
""

MInfoAltF6
""
""

MInfoAltF7
"Искать"
"Find"

MInfoAltF8
"Строка"
"Goto"

MInfoAltF9
"Видео"
"Video"

MInfoAltF10
"Дерево"
"Tree"

MInfoAltF11
"ИстПр"
"ViewHs"

MInfoAltF12
"ИстПап"
"FoldHs"

MInfoCtrlF1
l:
l:// InfoPanel KeyBar Ctrl-F1-F12
"Левая"
"Left"

MInfoCtrlF2
"Правая"
"Right"

MInfoCtrlF3
""
""

MInfoCtrlF4
""
""

MInfoCtrlF5
""
""

MInfoCtrlF6
""
""

MInfoCtrlF7
""
""

MInfoCtrlF8
""
""

MInfoCtrlF9
""
""

MInfoCtrlF10
""
""

MInfoCtrlF11
""
""

MInfoCtrlF12
""
""

MInfoCtrlShiftF1
l:
l:// InfoPanel KeyBar CtrlShiftF1-12
""
""

MInfoCtrlShiftF2
""
""

MInfoCtrlShiftF3
""
""

MInfoCtrlShiftF4
""
""

MInfoCtrlShiftF5
""
""

MInfoCtrlShiftF6
""
""

MInfoCtrlShiftF7
""
""

MInfoCtrlShiftF8
""
""

MInfoCtrlShiftF9
""
""

MInfoCtrlShiftF10
""
""

MInfoCtrlShiftF11
""
""

MInfoCtrlShiftF12
""
""

MInfoCtrlAltF1
l:
l:// InfoPanel KeyBar CtrlAltF1-12
""
""

MInfoCtrlAltF2
""
""

MInfoCtrlAltF3
""
""

MInfoCtrlAltF4
""
""

MInfoCtrlAltF5
""
""

MInfoCtrlAltF6
""
""

MInfoCtrlAltF7
""
""

MInfoCtrlAltF8
""
""

MInfoCtrlAltF9
""
""

MInfoCtrlAltF10
""
""

MInfoCtrlAltF11
""
""

MInfoCtrlAltF12
""
""

MInfoAltShiftF1
l:
l:// InfoPanel KeyBar AltShiftF1-12
""
""

MInfoAltShiftF2
""
""

MInfoAltShiftF3
""
""

MInfoAltShiftF4
""
""

MInfoAltShiftF5
""
""

MInfoAltShiftF6
""
""

MInfoAltShiftF7
""
""

MInfoAltShiftF8
""
""

MInfoAltShiftF9
""
""

MInfoAltShiftF10
""
""

MInfoAltShiftF11
""
""

MInfoAltShiftF12
""
""

MInfoCtrlAltShiftF1
l:
l:// InfoPanel KeyBar CtrlAltShiftF1-12
""
""

MInfoCtrlAltShiftF2
""
""

MInfoCtrlAltShiftF3
""
""

MInfoCtrlAltShiftF4
""
""

MInfoCtrlAltShiftF5
""
""

MInfoCtrlAltShiftF6
""
""

MInfoCtrlAltShiftF7
""
""

MInfoCtrlAltShiftF8
""
""

MInfoCtrlAltShiftF9
""
""

MInfoCtrlAltShiftF10
""
""

MInfoCtrlAltShiftF11
""
""

MInfoCtrlAltShiftF12
""
""

MQViewF1
l:
l:// QView KeyBar F1-F12
"Помощь"
"Help"

MQViewF2
"Сверн"
"Wrap"

MQViewF3
"Просм"
"View"

MQViewF4
"Код"
"Hex"

MQViewF5
""
""

MQViewF6
""
""

MQViewF7
"Поиск"
"Search"

MQViewF8
"Win"
"Win"

MQViewF9
"КонфМн"
"ConfMn"

MQViewF10
"Выход"
"Quit"

MQViewF11
"Модули"
"Plugin"

MQViewF12
"Экраны"
"Screen"

MQViewShiftF1
l:
l:// QView KeyBar Shift-F1-F12
""
""

MQViewShiftF2
"Слова"
"WWrap"

MQViewShiftF3
""
""

MQViewShiftF4
""
""

MQViewShiftF5
""
""

MQViewShiftF6
""
""

MQViewShiftF7
"Дальше"
"Next"

MQViewShiftF8
"Таблиц"
"Table"

MQViewShiftF9
"Сохран"
"Save"

MQViewShiftF10
"Послдн"
"Last"

MQViewShiftF11
""
""

MQViewShiftF12
""
""

MQViewAltF1
l:
l:// QView KeyBar Alt-F1-F12
"Левая"
"Left"

MQViewAltF2
"Правая"
"Right"

MQViewAltF3
""
""

MQViewAltF4
""
""

MQViewAltF5
""
""

MQViewAltF6
""
""

MQViewAltF7
"Искать"
"Find"

MQViewAltF8
"Строка"
"Goto"

MQViewAltF9
"Видео"
"Video"

MQViewAltF10
"Дерево"
"Tree"

MQViewAltF11
"ИстПр"
"ViewHs"

MQViewAltF12
"ИстПап"
"FoldHs"

MQViewCtrlF1
l:
l:// QView KeyBar Ctrl-F1-F12
"Левая"
"Left"

MQViewCtrlF2
"Правая"
"Right"

MQViewCtrlF3
""
""

MQViewCtrlF4
""
""

MQViewCtrlF5
""
""

MQViewCtrlF6
""
""

MQViewCtrlF7
""
""

MQViewCtrlF8
""
""

MQViewCtrlF9
""
""

MQViewCtrlF10
""
""

MQViewCtrlF11
""
""

MQViewCtrlF12
""
""

MQViewCtrlShiftF1
l:
l:// QView KeyBar CtrlShiftF1-12
""
""

MQViewCtrlShiftF2
""
""

MQViewCtrlShiftF3
""
""

MQViewCtrlShiftF4
""
""

MQViewCtrlShiftF5
""
""

MQViewCtrlShiftF6
""
""

MQViewCtrlShiftF7
""
""

MQViewCtrlShiftF8
""
""

MQViewCtrlShiftF9
""
""

MQViewCtrlShiftF10
""
""

MQViewCtrlShiftF11
""
""

MQViewCtrlShiftF12
""
""

MQViewCtrlAltF1
l:
l:// QView KeyBar CtrlAltF1-12
""
""

MQViewCtrlAltF2
""
""

MQViewCtrlAltF3
""
""

MQViewCtrlAltF4
""
""

MQViewCtrlAltF5
""
""

MQViewCtrlAltF6
""
""

MQViewCtrlAltF7
""
""

MQViewCtrlAltF8
""
""

MQViewCtrlAltF9
""
""

MQViewCtrlAltF10
""
""

MQViewCtrlAltF11
""
""

MQViewCtrlAltF12
""
""

MQViewAltShiftF1
l:
l:// QView KeyBar AltShiftF1-12
""
""

MQViewAltShiftF2
""
""

MQViewAltShiftF3
""
""

MQViewAltShiftF4
""
""

MQViewAltShiftF5
""
""

MQViewAltShiftF6
""
""

MQViewAltShiftF7
""
""

MQViewAltShiftF8
""
""

MQViewAltShiftF9
""
""

MQViewAltShiftF10
""
""

MQViewAltShiftF11
""
""

MQViewAltShiftF12
""
""

MQViewCtrlAltShiftF1
l:
l:// QView KeyBar CtrlAltShiftF1-12
""
""

MQViewCtrlAltShiftF2
""
""

MQViewCtrlAltShiftF3
""
""

MQViewCtrlAltShiftF4
""
""

MQViewCtrlAltShiftF5
""
""

MQViewCtrlAltShiftF6
""
""

MQViewCtrlAltShiftF7
""
""

MQViewCtrlAltShiftF8
""
""

MQViewCtrlAltShiftF9
""
""

MQViewCtrlAltShiftF10
""
""

MQViewCtrlAltShiftF11
""
""

MQViewCtrlAltShiftF12
""
""

MKBTreeF1
l:
l:// Tree KeyBar F1-F12
"Помощь"
"Help"

MKBTreeF2
"ПользМ"
"UserMn"

MKBTreeF3
""
""

MKBTreeF4
"Атриб"
"Attr"

MKBTreeF5
"Копир"
"Copy"

MKBTreeF6
"Перен"
"RenMov"

MKBTreeF7
"Папка"
"MkFold"

MKBTreeF8
"Удален"
"Delete"

MKBTreeF9
"КонфМн"
"ConfMn"

MKBTreeF10
"Выход"
"Quit"

MKBTreeF11
"Модули"
"Plugin"

MKBTreeF12
"Экраны"
"Screen"

MKBTreeShiftF1
l:
l:// Tree KeyBar Shift-F1-F12
""
""

MKBTreeShiftF2
""
""

MKBTreeShiftF3
""
""

MKBTreeShiftF4
""
""

MKBTreeShiftF5
"Копир"
"Copy"

MKBTreeShiftF6
"Перен"
"Rename"

MKBTreeShiftF7
""
""

MKBTreeShiftF8
""
""

MKBTreeShiftF9
"Сохран"
"Save"

MKBTreeShiftF10
"Послдн"
"Last"

MKBTreeShiftF11
"Группы"
"Group"

MKBTreeShiftF12
"Выбран"
"SelUp"

MKBTreeAltF1
l:
l:// Tree KeyBar Alt-F1-F12
"Левая"
"Left"

MKBTreeAltF2
"Правая"
"Right"

MKBTreeAltF3
""
""

MKBTreeAltF4
""
""

MKBTreeAltF5
""
""

MKBTreeAltF6
""
""

MKBTreeAltF7
"Искать"
"Find"

MKBTreeAltF8
"Истор"
"Histry"

MKBTreeAltF9
"Видео"
"Video"

MKBTreeAltF10
"Дерево"
"Tree"

MKBTreeAltF11
"ИстПр"
"ViewHs"

MKBTreeAltF12
"ИстПап"
"FoldHs"

MKBTreeCtrlF1
l:
l:// Tree KeyBar Ctrl-F1-F12
"Левая"
"Left"

MKBTreeCtrlF2
"Правая"
"Right"

MKBTreeCtrlF3
""
""

MKBTreeCtrlF4
""
""

MKBTreeCtrlF5
""
""

MKBTreeCtrlF6
""
""

MKBTreeCtrlF7
""
""

MKBTreeCtrlF8
""
""

MKBTreeCtrlF9
""
""

MKBTreeCtrlF10
""
""

MKBTreeCtrlF11
""
""

MKBTreeCtrlF12
""
""

MKBTreeCtrlShiftF1
l:
l:// Tree KeyBar CtrlShiftF1-12
""
""

MKBTreeCtrlShiftF2
""
""

MKBTreeCtrlShiftF3
""
""

MKBTreeCtrlShiftF4
""
""

MKBTreeCtrlShiftF5
""
""

MKBTreeCtrlShiftF6
""
""

MKBTreeCtrlShiftF7
""
""

MKBTreeCtrlShiftF8
""
""

MKBTreeCtrlShiftF9
""
""

MKBTreeCtrlShiftF10
""
""

MKBTreeCtrlShiftF11
""
""

MKBTreeCtrlShiftF12
""
""

MKBTreeCtrlAltF1
l:
l:// Tree KeyBar CtrlAltF1-12
""
""

MKBTreeCtrlAltF2
""
""

MKBTreeCtrlAltF3
""
""

MKBTreeCtrlAltF4
""
""

MKBTreeCtrlAltF5
""
""

MKBTreeCtrlAltF6
""
""

MKBTreeCtrlAltF7
""
""

MKBTreeCtrlAltF8
""
""

MKBTreeCtrlAltF9
""
""

MKBTreeCtrlAltF10
""
""

MKBTreeCtrlAltF11
""
""

MKBTreeCtrlAltF12
""
""

MKBTreeAltShiftF1
l:
l:// Tree KeyBar AltShiftF1-12
""
""

MKBTreeAltShiftF2
""
""

MKBTreeAltShiftF3
""
""

MKBTreeAltShiftF4
""
""

MKBTreeAltShiftF5
""
""

MKBTreeAltShiftF6
""
""

MKBTreeAltShiftF7
""
""

MKBTreeAltShiftF8
""
""

MKBTreeAltShiftF9
""
""

MKBTreeAltShiftF10
""
""

MKBTreeAltShiftF11
""
""

MKBTreeAltShiftF12
""
""

MKBTreeCtrlAltShiftF1
l:
l:// Tree KeyBar CtrlAltShiftF1-12
""
""

MKBTreeCtrlAltShiftF2
""
""

MKBTreeCtrlAltShiftF3
""
""

MKBTreeCtrlAltShiftF4
""
""

MKBTreeCtrlAltShiftF5
""
""

MKBTreeCtrlAltShiftF6
""
""

MKBTreeCtrlAltShiftF7
""
""

MKBTreeCtrlAltShiftF8
""
""

MKBTreeCtrlAltShiftF9
""
""

MKBTreeCtrlAltShiftF10
""
""

MKBTreeCtrlAltShiftF11
""
""

MKBTreeCtrlAltShiftF12
""
""

MRegTitle
l:
"Регистрация FAR"
"Register FAR"

MRegUser
"Регистрационное имя"
"Registration name"

MRegCode
"Регистрационный код"
"Registration code"

MRegFailed
"Некорректный код"
"Registration failed"

MRegThanks
"Успешная регистрация"
"Thank you for support!"

MRegOnly
"Эта функция доступна только в зарегистрированной версии"
"This function is available in registered version only"

MRegOnlyShort
"доступно после регистрации"
"available in registered version"

MCopyTimeInfo
l:
"Время: %8.8s  Осталось: %8.8s  %5d%1.1sб/с"
"Time: %8.8s  Remaining: %8.8s  %5d%1.1sb/s"

MKeyESCWasPressed
l:
"Действие было прервано"
"Operation has been interrupted"

MDoYouWantToStopWork
"Вы действительно хотите отменить действие?"
"Do you really want to cancel it?"

MDoYouWantToStopWork2
"Продолжить выполнение?"
"Continue work? "

MCheckingFileInPlugin
l:
"Файл проверяется в плагине"
"The file is being checked by the plugin"

MDialogType
l:
"Диалог"
"Dialog"

MHelpType
"Помощь"
"Help"

MFolderTreeType
"ПоискКаталогов"
"FolderTree"

MVMenuType
"Меню"
"Menu"

MIncorrectMask
l:
"Некорректная маска файлов!"
"File-mask string contains errors!"

MPanelBracketsForLongName
l:
"{}"
"{}"

MComspecNotFound
l:
"Переменная окружения %COMSPEC% не определена!"
"Environment variable %COMSPEC% not defined!"

MExecuteErrorMessage
"'%s' не является внутренней или внешней командой, исполняемой программой или пакетным файлом.\n"
"'%s' is not recognized as an internal or external command, operable program or batch file.\n"

MOpenPluginCannotOpenFile
l:
"Ошибка открытия файла"
"Cannot open the file"

MFileFilterTitle
l:
"Фильтр"
"Filter"

MFileHilightTitle
"Раскраска файлов"
"Files highlighting"

MFileFilterName
"Имя &фильтра:"
"Filter &name:"

MFileFilterMatchMask
"&Маска:"
"&Mask:"

MFileFilterSize
"Разм&ер:"
"Si&ze:"

MFileFilterSizeFromSign
">="
">="

MFileFilterSizeToSign
"<="
"<="

MFileFilterDate
"&Дата/Время:"
"Da&te/Time:"

MFileFilterModified
"&модификации"
"&modification"

MFileFilterCreated
"&создания"
"&creation"

MFileFilterOpened
"&доступа"
"&access"

MFileFilterDateRelative
"Относительна&я"
"Relat&ive"

MFileFilterDateAfterSign
">="
">="

MFileFilterDateBeforeSign
"<="
"<="

MFileFilterCurrent
"Теку&щая"
"C&urrent"

MFileFilterBlank
"С&брос"
"B&lank"

MFileFilterAttr
"Атрибут&ы"
"Attri&butes"

MFileFilterAttrR
"&Только для чтения"
"&Read only"

MFileFilterAttrA
"&Архивный"
"&Archive"

MFileFilterAttrH
"&Скрытый"
"&Hidden"

MFileFilterAttrS
"С&истемный"
"&System"

MFileFilterAttrC
"С&жатый"
"&Compressed"

MFileFilterAttrE
"&Зашифрованный"
"&Encrypted"

MFileFilterAttrD
"&Каталог"
"&Directory"

MFileFilterAttrNI
"&Неиндексируемый"
"Not inde&xed"

MFileFilterAttrSparse
"&Разреженный"
"S&parse"

MFileFilterAttrT
"&Временный"
"Temporar&y"

MFileFilterAttrReparse
"Симво&л. ссылка"
"Symbolic lin&k"

MFileFilterAttrOffline
"Автономны&й"
"O&ffline"

MFileFilterAttrVirtual
"Вирт&уальный"
"&Virtual"

MFileFilterReset
"Очистит&ь"
"Reset"

MFileFilterCancel
"Отмена"
"Cancel"

MFileFilterMakeTransparent
"Выставить прозрачность"
"Make transparent"

MBadFileSizeFormat
"Неправильно заполнено поле размера!"
"File size field is incorrectly filled!"

MNewFileName
l:
"?Новый файл?"
"?New File?"

MListEval
l:
"Shareware версия"
"Evaluation version"
