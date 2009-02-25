m4_include(`farversion.m4')m4_dnl
#hpp file name
lang.hpp
#number of languages
3
#id:0 language file name, language name, language description
FarRus.lng Russian "Russian (Русский)"
#id:1 language file name, language name, language description
FarEng.lng English "English"
#id:2 language file name, language name, language description
FarCze.lng Czech "Czech (Čeština)"

#head of the hpp file
hhead:#ifndef __FARLANG_HPP__
hhead:#define __FARLANG_HPP__

#tail of the hpp file
htail:
htail:#endif  // __FARLANG_HPP__
#and so on as much as needed

#--------------------------------------------------------------------
#now come the lng feeds
#--------------------------------------------------------------------
#first comes the text name from the enum which can be preceded with
#comments that will go to the hpp file
#h://This comment will appear before MYes
#he://This comment will appear after MYes
#MYes
#now come the lng lines for all the languages in the order defined
#above, they can be preceded with comments as shown below
#l://This comment will appear in all the lng files before the lng line
#le://This comment will appear in all the lng files after the lng line
#ls://This comment will appear only in Russian lng file before the lng line
#lse://This comment will appear only in Russian lng file after the lng line
#"Да"
#ls://This comment will appear only in English lng file before the lng line
#lse://This comment will appear only in English lng file after the lng line
#"Yes"
#ls://This comment will appear only in Czech lng file before the lng line
#lse://This comment will appear only in Czech lng file after the lng line
#upd:"Ano"
#lng lines marked with "upd:" will cause a warning to be printed to the
#screen reminding that this line should be updated/translated

MYes=0
`l://Version: 'MAJOR`.'MINOR` build 'BUILD
l:
"Да"
"Yes"
"Ano"

MNo
"Нет"
"No"
"Ne"

MOk
"Продолжить"
"OK"
"Ok"

MHYes
l:
"&Да"
"&Yes"
"&Ano"

MHNo
"&Нет"
"&No"
"&Ne"

MHOk
"&Продолжить"
"&OK"
"&Ok"

MCancel
l:
"Отменить"
"Cancel"
"Storno"

MRetry
"Повторить"
"Retry"
"Znovu"

MSkip
"Пропустить"
"Skip"
"Přeskočit"

MAbort
"Прервать"
"Abort"
"Zrušit"

MIgnore
"Игнорировать"
"Ignore"
"Ignorovat"

MDelete
"Удалить"
"Delete"
"Smazat"

MSplit
"Разделить"
"Split"
"Rozdělit"

MRemove
"Удалить"
"Remove"
"Odstranit"

MHCancel
l:
"&Отменить"
"&Cancel"
"&Storno"

MHRetry
"&Повторить"
"&Retry"
"&Znovu"

MHSkip
"П&ропустить"
"&Skip"
"&Přeskočit"

MHSkipAll
"Пропустить &все"
"S&kip all"
"Přeskočit &vše"

MHAbort
"Прер&вать"
"&Abort"
"Zr&ušit"

MHIgnore
"&Игнорировать"
"&Ignore"
"&Ignorovat"

MHDelete
"&Удалить"
"&Delete"
"S&mazat"

MHRemove
"&Удалить"
"R&emove"
"&Odstranit"

MHSplit
"Раз&делить"
"Sp&lit"
"&Rozdělit"

MWarning
l:
"Предупреждение"
"Warning"
"Varování"

MError
"Ошибка"
"Error"
"Chyba"

MQuit
l:
"Выход"
"Quit"
"Konec"

MAskQuit
"Вы хотите завершить работу в FAR?"
"Do you want to quit FAR?"
"Opravdu chcete ukončit FAR?"

MF1
l:
l://functional keys - 6 characters max
"Помощь"
"Help"
"Pomoc"

MF2
"ПользМ"
"UserMn"
"UživMn"

MF3
"Просм"
"View"
"Zobraz"

MF4
"Редакт"
"Edit"
"Edit"

MF5
"Копир"
"Copy"
"Kopír."

MF6
"Перен"
"RenMov"
"PřjPřs"

MF7
"Папка"
"MkFold"
"VytAdr"

MF8
"Удален"
"Delete"
"Smazat"

MF9
"КонфМн"
"ConfMn"
"KonfMn"

MF10
"Выход"
"Quit"
"Konec"

MF11
"Модули"
"Plugin"
"Plugin"

MF12
"Экраны"
"Screen"
"Obraz."

MAltF1
l:
"Левая"
"Left"
"Levý"

MAltF2
"Правая"
"Right"
"Pravý"

MAltF3
"Смотр."
"View.."
"Zobr.."

MAltF4
"Редак."
"Edit.."
"Edit.."

MAltF5
"Печать"
"Print"
"Tisk"

MAltF6
"Ссылка"
"MkLink"
"VytLnk"

MAltF7
"Искать"
"Find"
"Hledat"

MAltF8
"Истор"
"Histry"
"Histor"

MAltF9
"Видео"
"Video"
"Video"

MAltF10
"Дерево"
"Tree"
"Strom"

MAltF11
"ИстПр"
"ViewHs"
"ProhHs"

MAltF12
"ИстПап"
"FoldHs"
"AdrsHs"

MCtrlF1
l:
"Левая"
"Left"
"Levý"

MCtrlF2
"Правая"
"Right"
"Pravý"

MCtrlF3
"Имя   "
"Name  "
"Název "

MCtrlF4
"Расшир"
"Extens"
"Přípon"

MCtrlF5
"Модиф"
"Modifn"
"Modifk"

MCtrlF6
"Размер"
"Size"
"Veliko"

MCtrlF7
"Несорт"
"Unsort"
"Neřadi"

MCtrlF8
"Создан"
"Creatn"
"Vytvoř"

MCtrlF9
"Доступ"
"Access"
"Přístu"

MCtrlF10
"Описан"
"Descr"
"Popis"

MCtrlF11
"Владел"
"Owner"
"Vlastn"

MCtrlF12
"Сорт"
"Sort"
"Třídit"

MShiftF1
l:
"Добавл"
"Add"
"Přidat"

MShiftF2
"Распак"
"Extrct"
"Rozbal"

MShiftF3
"АрхКом"
"ArcCmd"
"ArcPří"

MShiftF4
"Редак."
"Edit.."
"Edit.."

MShiftF5
"Копир"
"Copy"
"Kopír."

MShiftF6
"Переим"
"Rename"
"Přejme"

MShiftF7
""
""
""

MShiftF8
"Удален"
"Delete"
"Smazat"

MShiftF9
"Сохран"
"Save"
"Uložit"

MShiftF10
"Послдн"
"Last"
"Posled"

MShiftF11
"Группы"
"Group"
"Skupin"

MShiftF12
"Выбран"
"SelUp"
"VybPrv"

MAltShiftF1
l:
l:// Main AltShift
""
""
""

MAltShiftF2
""
""
""

MAltShiftF3
""
""
""

MAltShiftF4
""
""
""

MAltShiftF5
""
""
""

MAltShiftF6
""
""
""

MAltShiftF7
""
""
""

MAltShiftF8
""
""
""

MAltShiftF9
"КонфПл"
"ConfPl"
"KonfPl"

MAltShiftF10
""
""
""

MAltShiftF11
""
""
""

MAltShiftF12
""
""
""

MCtrlShiftF1
l:
l://Main CtrlShift
""
""
""

MCtrlShiftF2
""
""
""

MCtrlShiftF3
"Просм"
"View"
"Zobraz"

MCtrlShiftF4
"Редакт"
"Edit"
"Edit"

MCtrlShiftF5
""
""
""

MCtrlShiftF6
""
""
""

MCtrlShiftF7
""
""
""

MCtrlShiftF8
""
""
""

MCtrlShiftF9
""
""
""

MCtrlShiftF10
""
""
""

MCtrlShiftF11
""
""
""

MCtrlShiftF12
""
""
""

MCtrlAltF1
l:
l:// Main CtrlAlt
""
""
""

MCtrlAltF2
""
""
""

MCtrlAltF3
""
""
""

MCtrlAltF4
""
""
""

MCtrlAltF5
""
""
""

MCtrlAltF6
""
""
""

MCtrlAltF7
""
""
""

MCtrlAltF8
""
""
""

MCtrlAltF9
""
""
""

MCtrlAltF10
""
""
""

MCtrlAltF11
""
""
""

MCtrlAltF12
""
""
""

MCtrlAltShiftF1
l:
l:// Main CtrlAltShift
""
""
""

MCtrlAltShiftF2
""
""
""

MCtrlAltShiftF3
""
""
""

MCtrlAltShiftF4
""
""
""

MCtrlAltShiftF5
""
""
""

MCtrlAltShiftF6
""
""
""

MCtrlAltShiftF7
""
""
""

MCtrlAltShiftF8
""
""
""

MCtrlAltShiftF9
""
""
""

MCtrlAltShiftF10
""
""
""

MCtrlAltShiftF11
""
""
""

MCtrlAltShiftF12
le://End of functional keys
""
""
""

MHistoryTitle
l:
"История команд"
"History"
"Historie"

MFolderHistoryTitle
"История папок"
"Folders history"
"Historie adresářů"

MViewHistoryTitle
"История просмотра"
"File view history"
"Historie prohlížení souborů"

MViewHistoryIsCreate
"Создать файл?"
"Create file?"
"Vytvořit soubor?"

MHistoryView
"Просмотр"
"View"
"Zobrazit"

MHistoryEdit
"Редактор"
"Edit"
"Editovat"

MHistoryExt
"Внешний "
"Ext."
"Rozšíření"

MHistoryClear
l:
"История будет полностью очищена. Продолжить?"
"All records in the history will be deleted. Continue?"
"Všechny záznamy v historii budou smazány. Pokračovat?"

MClear
"&Очистить"
"&Clear history"
"&Vymazat historii"

MConfigSystemTitle
l:
"Системные параметры"
"System settings"
"Nastavení systému"

MConfigRO
"&Снимать атрибут R/O c CD файлов"
"&Clear R/O attribute from CD files"
"Z&rušit atribut R/O u souborů na CD"

MConfigRecycleBin
"Удалять в &Корзину"
"&Delete to Recycle Bin"
"&Mazat do Koše"

MConfigRecycleBinLink
"У&далять символические ссылки"
"Delete symbolic &links"
"Mazat symbolické &linky"

MConfigSystemCopy
"Использовать систе&мную функцию копирования"
"Use sys&tem copy routine"
"Používat kopírovací rutiny sys&tému"

MConfigCopySharing
"Копировать открытые для &записи файлы"
"Copy files opened for &writing"
"Kopírovat soubory otevřené pro &zápis"

MConfigScanJunction
"Ск&анировать символические ссылки"
"Scan s&ymbolic links"
"Prohledávat s&ymbolické linky"

MConfigCreateUppercaseFolders
"Создавать &папки заглавными буквами"
"Create folders in &uppercase"
"Vytvářet adresáře &velkými písmeny"

MConfigInactivity
"&Время бездействия"
"&Inactivity time"
"&Doba nečinnosti"

MConfigInactivityMinutes
"минут"
"minutes"
"minut"

MConfigSaveHistory
"Сохранять &историю команд"
"Save commands &history"
"Ukládat historii &příkazů"

MConfigSaveFoldersHistory
"Сохранять историю п&апок"
"Save &folders history"
"Ukládat historii &adresářů"

MConfigSaveViewHistory
"Сохранять историю п&росмотра и редактора"
"Save &view and edit history"
"Ukládat historii Zobraz a Editu&j"

MConfigRegisteredTypes
"Использовать стандартные &типы файлов"
"Use Windows &registered types"
"Používat regi&strované typy Windows"

MConfigCloseCDGate
"Автоматически монтироват&ь CDROM"
"CD drive auto &mount"
"Automatické př&ipojení CD disků"

MConfigPersonalPath
"Путь к персональным п&лагинам:"
"&Path for personal plugins:"
"&Cesta k vlastním pluginům:"

MConfigAutoSave
"Автозапись кон&фигурации"
"Auto &save setup"
"Automatické ukládaní &nastavení"

MConfigPanelTitle
l:
"Настройки панели"
"Panel settings"
"Nastavení panelů"

MConfigHidden
"Показывать скр&ытые и системные файлы"
"Show &hidden and system files"
"Ukázat &skryté a systémové soubory"

MConfigHighlight
"&Раскраска файлов"
"Hi&ghlight files"
"Zvý&razňovat soubory"

MConfigAutoChange
"&Автосмена папки"
"&Auto change folder"
"&Automaticky měnit adresář"

MConfigSelectFolders
"Пометка &папок"
"Select &folders"
"Vybírat a&dresáře"

MConfigSortFolderExt
"Сортировать имена папок по рас&ширению"
"Sort folder names by e&xtension"
"Řadit adresáře podle přípony"

MConfigReverseSort
"Разрешить &обратную сортировку"
"Allow re&verse sort modes"
"Do&volit změnu směru řazení"

MConfigAutoUpdateLimit
"Отключать автооб&новление панелей,"
"&Disable automatic update of panels"
"Vypnout a&utomatickou aktualizaci panelů"

MConfigAutoUpdateLimit2
"если объектов больше"
"if object count exceeds"
"jestliže počet objektů překročí"

MConfigAutoUpdateRemoteDrive
"Автообновление с&етевых дисков"
"Network drives autor&efresh"
"Automatická obnova síťových disků"

MConfigShowColumns
"Показывать &заголовки колонок"
"Show &column titles"
"Zobrazovat &nadpisy sloupců"

MConfigShowStatus
"Показывать &строку статуса"
"Show &status line"
"Zobrazovat sta&vový řádek"

MConfigShowTotal
"Показывать су&ммарную информацию"
"Show files &total information"
"Zobrazovat &informace o velikosti souborů"

MConfigShowFree
"Показывать с&вободное место"
"Show f&ree size"
"Zobrazovat vo&lné místo"

MConfigShowScrollbar
"Показывать по&лосу прокрутки"
"Show scroll&bar"
"Zobrazovat &posuvník"

MConfigShowScreensNumber
"Показывать количество &фоновых экранов"
"Show background screens &number"
"Zobrazovat počet &obrazovek na pozadí"

MConfigShowSortMode
"Показывать букву режима сор&тировки"
"Show sort &mode letter"
"Zobrazovat písmeno &módu řazení"

MConfigInterfaceTitle
l:
"Настройки интерфейса"
"Interface settings"
"Nastavení rozhraní"

MConfigClock
"&Часы в панелях"
"&Clock in panels"
"&Hodiny v panelech"

MConfigViewerEditorClock
"Ч&асы при редактировании и просмотре"
"C&lock in viewer and editor"
"H&odiny v prohlížeči a editoru"

MConfigMouse
"Мы&шь"
"M&ouse"
"M&yš"

MConfigMousePanelMClickRule
"В панелях ср&едняя кнопка равна Enter"
"Middle &button equals Enter in panels"
"Prostřední tlač. znamená v panelech Enter"

MConfigKeyBar
"Показывать &линейку клавиш"
"Show &key bar"
"Zobrazovat &zkratkové klávesy"

MConfigMenuBar
"Всегда показывать &меню"
"Always show &menu bar"
"Vždy zobrazovat hlavní &menu"

MConfigSaver
"&Сохранение экрана"
"&Screen saver"
"Sp&ořič obrazovky"

MConfigSaverMinutes
"минут"
"minutes"
"minut"

MConfigUsePromptFormat
"Установить &формат командной строки"
"Set command line &prompt format"
"Nastavit formát &příkazového řádku"

MConfigCopyTotal
"Показывать &общий индикатор копирования"
"Show &total copy progress indicator"
"Zobraz. ukazatel celkového stavu &kopírování"

MConfigCopyTimeRule
"Показывать информацию о времени &копирования"
"Show cop&ying time information"
"Zobrazovat informace o čase kopírování"

MConfigPgUpChangeDisk
"Использовать Ctrl-PgUp для в&ыбора диска"
"Use Ctrl-Pg&Up to change drive"
"Použít Ctrl-Pg&Up pro změnu disku"

MConfigDlgSetsTitle
l:
"Настройки диалогов"
"Dialog settings"
"Nastavení dialogů"

MConfigDialogsEditHistory
"&История в строках ввода диалогов"
"&History in dialog edit controls"
"H&istorie v dialozích"

MConfigDialogsEditBlock
"&Постоянные блоки в строках ввода"
"&Persistent blocks in edit controls"
"&Trvalé bloky v editačních polích"

MConfigDialogsDelRemovesBlocks
"Del удаляет б&локи в строках ввода"
"&Del removes blocks in edit controls"
"&Del maže položky v editačních polích"

MConfigDialogsAutoComplete
"&Автозавершение в строках ввода"
"&AutoComplete in edit controls"
"Automatické dokončování v editač&ních polích"

MConfigDialogsEULBsClear
"Backspace &удаляет неизмененный текст"
"&Backspace deletes unchanged text"
"&Backspace maže nezměněný text"

MConfigDialogsMouseButton
"Клик мыши &вне диалога закрывает диалог"
"Mouse click &outside a dialog closes it"
"Kl&iknutí myší mimo dialog ho zavře"

MViewConfigTitle
l:
"Программа просмотра"
"Viewer"
"Prohlížeč"

MViewConfigExternal
"Внешняя программа просмотра:"
"External viewer:"
"Externí prohlížeč"

MViewConfigExternalF3
"Запускать по F3"
"Use for F3"
"Použít pro F3"

MViewConfigExternalAltF3
"Запускать по Alt-F3"
"Use for Alt-F3"
"Použít pro Alt-F3"

MViewConfigExternalCommand
"&Команда просмотра:"
"&Viewer command:"
"&Příkaz prohlížeče:"

MViewConfigInternal
"Встроенная программа просмотра:"
"Internal viewer:"
"Interní prohlížeč"

MViewConfigSavePos
"&Сохранять позицию файла"
"&Save file position"
"&Ukládat pozici v souboru"

MViewConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"
"Ukládat &záložky"

MViewAutoDetectTable
"&Автоопределение таблицы символов"
"&Autodetect character table"
"&Autodetekovat znakovou sadu"

MViewConfigTabSize
"Раз&мер табуляции"
"Tab si&ze"
"Velikost &Tabulátoru"

MViewConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"
"Zobrazovat posu&vník"

MViewConfigArrows
"Показывать стрелки с&двига"
"Show scrolling arro&ws"
"Zobrazovat &skrolovací šipky"

MViewConfigPersistentSelection
"Постоянное &выделение"
"&Persistent selection"
"Trvalé &výběry"

MViewConfigAnsiTableAsDefault
"&Изначально открывать файлы в WIN кодировке"
"&Initially open files in WIN encoding"
"Automaticky otevírat soubory ve &WIN kódování"

MEditConfigTitle
l:
"Редактор"
"Editor"
"Editor"

MEditConfigExternal
"Внешний редактор:"
"External editor:"
"Externí editor"

MEditConfigEditorF4
"Запускать по F4"
"Use for F4"
"Použít pro F4"

MEditConfigEditorAltF4
"Запускать по Alt-F4"
"Use for Alt-F4"
"Použít pro Alt-F4"

MEditConfigEditorCommand
"&Команда редактирования:"
"&Editor command:"
"&Příkaz editoru:"

MEditConfigInternal
"Встроенный редактор:"
"Internal editor:"
"Interní editor"

MEditConfigExpandTabsTitle
"Преобразовывать &табуляцию:"
"Expand &tabs:"
"Rozšířit Ta&bulátory mezerami"

MEditConfigDoNotExpandTabs
l:
"Не преобразовывать табуляцию"
"Do not expand tabs"
"Nerozšiřovat tabulátory mezerami"

MEditConfigExpandTabs
"Преобразовывать новые символы табуляции в пробелы"
"Expand newly entered tabs to spaces"
"Rozšířit nově zadané tabulátory mezerami"

MEditConfigConvertAllTabsToSpaces
"Преобразовывать все символы табуляции в пробелы"
"Expand all tabs to spaces"
"Rozšířit všechny tabulátory mezerami"

MEditConfigPersistentBlocks
"&Постоянные блоки"
"&Persistent blocks"
"&Trvalé bloky"

MEditConfigDelRemovesBlocks
l:
"Del удаляет б&локи"
"&Del removes blocks"
"&Del maže bloky"

MEditConfigAutoIndent
"Авто&отступ"
"Auto &indent"
"Auto &Odsazování"

MEditConfigSavePos
"&Сохранять позицию файла"
"&Save file position"
"&Ukládat pozici v souboru"

MEditConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"
"Ukládat zá&ložky"

MEditAutoDetectTable
"&Автоопределение таблицы символов"
"&Autodetect character table"
"&Autodetekovat znakovou sadu"

MEditCursorBeyondEnd
"Ку&рсор за пределами строки"
"&Cursor beyond end of line"
"&Kurzor za koncem řádku"

MEditLockROFileModification
"Блокировать р&едактирование файлов с атрибутом R/O"
"Lock editing of read-only &files"
"&Zamknout editaci souborů určených jen pro čtení"

MEditWarningBeforeOpenROFile
"Пре&дупреждать при открытии файла с атрибутом R/O"
"&Warn when opening read-only files"
"&Varovat při otevření souborů určených jen pro čtení"

MEditConfigTabSize
"Раз&мер табуляции"
"Tab si&ze"
"Velikost &Tabulátoru"

MEditConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"
"Zobr&azovat posuvník"

MEditConfigAnsiTableAsDefault
"&Изначально открывать файлы в WIN кодировке"
"I&nitially open files in WIN encoding"
"Automaticky otevírat soubory ve &WIN kódování"

MEditConfigAnsiTableForNewFile
"Созда&вать новые файлы в WIN кодировке"
"C&reate new files in WIN encoding"
"V&ytvářet nové soubory ve WIN kódování"

MSaveSetupTitle
l:
"Конфигурация"
"Save setup"
"Uložit nastavení"

MSaveSetupAsk1
"Вы хотите сохранить"
"Do you wish to save"
"Přejete si uložit"

MSaveSetupAsk2
"текущую конфигурацию?"
"current setup?"
"aktuální nastavení?"

MSaveSetup
"Сохранить"
"Save"
"Uložit"

MCopyDlgTitle
l:
"Копирование"
"Copy"
"Kopírovat"

MMoveDlgTitle
"Переименование/Перенос"
"Rename/Move"
"Přejmenovat/Přesunout"

MLinkDlgTitle
"Ссылка"
"Link"
"Link"

MCopySecurity
"П&рава доступа:"
"&Access rights:"
"&Přístupová práva:"

MCopySecurityCopy
"Копироват&ь"
"Co&py"
"&Kopírovat"

MCopySecurityInherit
"Нас&ледовать"
"&Inherit"
"&Zdědit"

MCopySecurityLeave
"По умол&чанию"
"Defau&lt"
"Vých&ozí"

MCopyIfFileExist
"Уже су&ществующие файлы:"
"Already e&xisting files:"
"Již e&xistující soubory:"

MCopyAsk
"&Запрос действия"
"&Ask"
"Ptát s&e"

MCopyAskRO
"Запрос подтверждения для &R/O файлов"
"Also ask on &R/O files"
"Ptát se také na &R/O soubory"

MCopyOnlyNewerFiles
"Только &новые/обновленные файлы"
"Only ne&wer file(s)"
"Pouze &novější soubory"

MLinkType
"&Тип ссылки:"
"Link t&ype:"
"&Typ linku:"

MLinkTypeJunction
"&связь каталогов"
"directory &junction"
"křížení a&dresářů"

MLinkTypeHardlink
"&жёсткая ссылка"
"&hard link"
"&pevný link"

MLinkTypeSymlinkFile
"символическая ссылка (&файл)"
"symbolic link (&file)"
"symbolický link (&soubor)"

MLinkTypeSymlinkDirectory
"символическая ссылка (&папка)"
"symbolic link (fol&der)"
"symbolický link (&adresář)"

MCopySymLinkContents
"Копировать содерж&имое символических ссылок"
"Cop&y contents of symbolic links"
"Kopírovat obsah sym&bolických linků"

MCopyMultiActions
"Обр&абатывать несколько имен файлов"
"Process &multiple destinations"
"&Zpracovat více míst určení"

MCopyDlgCopy
"&Копировать"
"&Copy"
"&Kopírovat"

MCopyDlgTree
"F10-&Дерево"
"F10-&Tree"
"F10-&Strom"

MCopyDlgCancel
"&Отменить"
"Ca&ncel"
"&Storno"

MCopyDlgRename
"&Переименовать"
"&Rename"
"Přej&menovat"

MCopyDlgLink
"&Создать ссылку"
"&Link"
"&Linkovat"

MCopyDlgTotal
"Всего"
"Total"
"Celkem"

MCopyScanning
"Сканирование папок..."
"Scanning folders..."
"Načítání adresářů..."

MCopyPrepareSecury
"Применение прав доступа..."
"Applying access rights..."
"Nastavuji přístupová práva..."

MCopyUseFilter
"Исполь&зовать фильтр"
"&Use filter"
"P&oužít filtr"

MCopySetFilter
"&Фильтр"
"Filt&er"
"Filt&r"

MCopyFile
l:
"Копировать \"%.55s\""
"Copy \"%.55s\""
"Kopírovat \"%.55s\""

MMoveFile
"Переименовать или перенести \"%.55s\""
"Rename or move \"%.55s\""
"Přejmenovat nebo přesunout \"%.55s\""

MLinkFile
"Создать ссылку \"%.55s\""
"Link \"%.55s\""
"Linkovat \"%.55s\""

MCopyFiles
"Копировать %d элемент%s"
"Copy %d item%s"
"Kopírovat %d polož%s"

MMoveFiles
"Переименовать или перенести %d элемент%s"
"Rename or move %d item%s"
"Přejmenovat nebo přesunout %d polož%s"

MLinkFiles
"Создать ссылки %d элемент%s"
"Link %d item%s"
"Linkovat %d polož%s"

MCMLTargetTO
" &в:"
" t&o:"
" d&o:"

MCMLItems0
""
""
"u"

MCMLItemsA
"а"
"s"
"ek"

MCMLItemsS
"ов"
"s"
"ky"

MCopyIncorrectTargetList
l:
"Указан некорректный список целей!"
"Incorrect target list!"
"Nesprávný seznam cílů!"

MCopyCopyingTitle
l:
"Копирование"
"Copying"
"Kopíruji"

MCopyMovingTitle
"Перенос"
"Moving"
"Přesouvám"

MCopyCannotFind
l:
"Файл не найден"
"Cannot find the file"
"Nelze nalézt soubor"

MCannotCopyFolderToItself1
l:
"Нельзя копировать папку"
"Cannot copy the folder"
"Nelze kopírovat adresář"

MCannotCopyFolderToItself2
"в саму себя"
"onto itself"
"sám na sebe"

MCannotCopyToTwoDot
l:
"Нельзя копировать файл или папку"
"You may not copy files or folders"
"Nelze kopírovat soubory nebo adresáře"

MCannotMoveToTwoDot
"Нельзя перемещать файл или папку"
"You may not move files or folders"
"Nelze přesunout soubory nebo adresáře"

MCannotCopyMoveToTwoDot
"выше корневого каталога"
"higher than the root folder"
"na vyšší úroveň než kořenový adresář"

MCopyCannotCreateFolder
l:
"Ошибка создания папки"
"Cannot create the folder"
"Nelze vytvořit adresář"

MCopyCannotChangeFolderAttr
"Невозможно установить атрибуты для папки"
"Failed to set folder attributes"
"Nastavení atributů adresáře selhalo"

MCopyCannotRenameFolder
"Невозможно переименовать папку"
"Cannot rename the folder"
"Nelze přejmenovat adresář"

MCopyIgnore
"&Игнорировать"
"&Ignore"
"&Ignorovat"

MCopyIgnoreAll
"Игнорировать &все"
"Ignore &All"
"Ignorovat &vše"

MCopyRetry
"&Повторить"
"&Retry"
"&Opakovat"

MCopySkip
"П&ропустить"
"&Skip"
"&Přeskočit"

MCopySkipAll
"&Пропустить все"
"S&kip all"
"Př&eskočit vše"

MCopyCancel
"&Отменить"
"&Cancel"
"&Storno"

MCopyDecrypt
"Рас&шифровать"
"&Decrypt"
"&Dešifrovat"

MCopyDecryptAll
"&Все"
"Decrypt &all"
"Deš&ifrovat vše"

MCopyCannotCreateLink
l:
"Ошибка создания ссылки"
"Cannot create the link"
"Nelze vytvořit symbolický link"

MCopyFolderNotEmpty
"Папка назначения должна быть пустой"
"Target folder must be empty"
"Cílový adresář musí být prázdný"

MCopyCannotCreateJunctionToFile
"Невозможно создать связь. Файл уже существует:"
"Cannot create junction. The file already exists:"
"Nelze vytvořit křížový odkaz. Soubor již existuje:"

MCopyCannotCreateVolMount
l:
"Ошибка монтирования диска"
"Volume mount points error"
"Chyba připojovacích svazků"

MCopyRetrVolFailed
"Невозможно получить информацию о '%s'"
"Retrieving volume name for '%s' failed"
"Získání jména zvazku pro '%s' selhalo"

MCopyMountVolFailed
"Ошибка при монтировании диска '%s'"
"Attempt to volume mount '%s'"
"Pokus o připojení svazku '%s'"

MCopyMountVolFailed2
"на '%s'"
"at '%s' failed"
"na '%s' selhal"

MCopyCannotSupportVolMount
"Функция монтирования дисков не поддерживается"
"Volume mounting is not supported"
"Připojování svazku není podporováno"

MCopyMountName
"disk_%c"
"Disk_%c"
"Disk_%c"

MCannotCopyFileToItself1
l:
"Нельзя копировать файл"
"Cannot copy the file"
"Nelze kopírovat soubor"

MCannotCopyFileToItself2
"в самого себя"
"onto itself"
"sám na sebe"

MCopyStream1
l:
"Исходный файл содержит более одного потока данных,"
"The source file contains more than one data stream."
"Zdrojový soubor obsahuje více než jeden datový proud."

MCopyStream2
"но вы не используете системную функцию копирования."
"but since you do not use a system copy routine."
"protože nepoužíváte systémovou kopírovací rutinu."

MCopyStream3
"но том назначения не поддерживает этой возможности."
"but the destination volume does not support this feature."
"protože cílový svazek nepodporuje tuto vlastnost."

MCopyStream4
"Часть сведений не будет сохранена."
"Some data will not be preserved as a result."
"To bude mít za následek, že některá data nebudou uchována."

MCopyFileExist
l:
"Файл уже существует"
"File already exists"
"Soubor již existuje"

MCopySource
"&Новый"
"&New"
"&Nový"

MCopyDest
"Су&ществующий"
"E&xisting"
"E&xistující"

MCopyOverwrite
"В&место"
"&Overwrite"
"&Přepsat"

MCopyContinue
"П&родолжить"
"C&ontinue"
"P&okračovat"

MCopySkipOvr
"&Пропустить"
"&Skip"
"Přes&kočit"

MCopyAppend
"&Дописать"
"A&ppend"
"Př&ipojit"

MCopyResume
"Возоб&новить"
"&Resume"
"Po&kračovat"

MCopyCancelOvr
"&Отменить"
"&Cancel"
"&Storno"

MCopyRememberChoice
"&Запомнить выбор"
"&Remember choice"
"Zapama&tovat volbu"

MCopyFileRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"
"Soubor je určen pouze pro čtení"

MCopyAskDelete
"Вы хотите удалить его?"
"Do you wish to delete it?"
"Opravdu si ho přejete smazat?"

MCopyDeleteRO
"&Удалить"
"&Delete"
"S&mazat"

MCopyDeleteAllRO
"&Все"
"&All"
"&Vše"

MCopySkipRO
"&Пропустить"
"&Skip"
"Přes&kočit"

MCopySkipAllRO
"П&ропустить все"
"S&kip all"
"Př&eskočit vše"

MCopyCancelRO
"&Отменить"
"&Cancel"
"&Storno"

MCannotCopy
l:
"Ошибка копирования"
"Cannot copy"
"Nelze kopírovat"

MCannotMove
"Ошибка переноса"
"Cannot move"
"Nelze přesunout"

MCannotLink
"Ошибка создания ссылки"
"Cannot link"
"Nelze linkovat"

MCannotCopyTo
"в"
"to"
"do"

MCopyEncryptWarn1
"Файл"
"The file"
"Soubor"

MCopyEncryptWarn2
"нельзя скопировать или переместить, не потеряв его шифрование."
"cannot be copied or moved without losing its encryption."
"nemůže být zkopírován nebo přesunut bez ztráty jeho šifrování."

MCopyEncryptWarn3
"Можно пропустить эту ошибку или отменить операцию."
"You can choose to ignore this error and continue, or cancel."
"Můžete tuto chybu ignorovat a pokračovat, nebo operaci ukončit."

MCopyReadError
l:
"Ошибка чтения данных из"
"Cannot read data from"
"Nelze číst data z"

MCopyWriteError
"Ошибка записи данных в"
"Cannot write data to"
"Nelze zapsat data do"

MCopyProcessed
l:
"Обработано файлов: %d"
"Files processed: %d"
"Zpracováno souborů: %d"

MCopyProcessedTotal
"Обработано файлов: %d из %d"
"Files processed: %d of %d"
"Zpracováno souborů: %d z %d"

MCopyMoving
"Перенос файла"
"Moving the file"
"Přesunuji soubor"

MCopyCopying
"Копирование файла"
"Copying the file"
"Kopíruji soubor"

MCopyTo
"в"
"to"
"do"

MCopyErrorDiskFull
l:
"Диск заполнен. Вставьте следующий"
"Disk full. Insert next"
"Disk je plný. Vložte dalšíí"

MDeleteTitle
l:
"Удаление"
"Delete"
"Smazat"

MAskDeleteFolder
"Вы хотите удалить папку"
"Do you wish to delete the folder"
"Přejete si smazat adresář"

MAskDeleteFile
"Вы хотите удалить файл"
"Do you wish to delete the file"
"Přejete si smazat soubor"

MAskDelete
"Вы хотите удалить"
"Do you wish to delete"
"Přejete si smazat"

MAskDeleteRecycleFolder
"Вы хотите поместить в Корзину папку"
"Do you wish to move to the Recycle Bin the folder"
"Přejete si přesunout do Koše adresář"

MAskDeleteRecycleFile
"Вы хотите поместить в Корзину файл"
"Do you wish to move to the Recycle Bin the file"
"Přejete si přesunout do Koše soubor"

MAskDeleteRecycle
"Вы хотите поместить в Корзину"
"Do you wish to move to the Recycle Bin"
"Přejete si přesunout do Koše"

MDeleteWipeTitle
"Уничтожение"
"Wipe"
"Vymazat"

MAskWipeFolder
"Вы хотите уничтожить папку"
"Do you wish to wipe the folder"
"Přejete si vymazat adresář"

MAskWipeFile
"Вы хотите уничтожить файл"
"Do you wish to wipe the file"
"Přejete si vymazat soubor"

MAskWipe
"Вы хотите уничтожить"
"Do you wish to wipe"
"Přejete si vymazat"

MDeleteLinkTitle
"Удаление ссылки"
"Delete link"
"Smazat link"

MAskDeleteLink
"является ссылкой на"
"is a symbolic link to"
"je symbolicky link na"

MAskDeleteLinkFolder
"папку"
"folder"
"adresář"

MAskDeleteLinkFile
"файл"
"file"
"soubor"

MAskDeleteItems
"%d элемент%s"
"%d item%s"
"%d polož%s"

MAskDeleteItems0
""
""
"ku"

MAskDeleteItemsA
"а"
"s"
"ky"

MAskDeleteItemsS
"ов"
"s"
"ek"

MDeleteFolderTitle
l:
"Удаление папки "
"Delete folder"
"Smazat adresář"

MWipeFolderTitle
"Уничтожение папки "
"Wipe folder"
"Vymazat adresář"

MDeleteFilesTitle
"Удаление файлов"
"Delete files"
"Smazat soubory"

MWipeFilesTitle
"Уничтожение файлов"
"Wipe files"
"Vymazat soubory"

MDeleteFolderConfirm
"Данная папка будет удалена:"
"The following folder will be deleted:"
"Následující adresář bude smazán:"

MWipeFolderConfirm
"Данная папка будет уничтожена:"
"The following folder will be wiped:"
"Následující adresář bude vymazán:"

MDeleteWipe
"Уничтожить"
"Wipe"
"Vymazat"

MDeleteFileDelete
"&Удалить"
"&Delete"
"S&mazat"

MDeleteFileWipe
"&Уничтожить"
"&Wipe"
"V&ymazat"

MDeleteFileAll
"&Все"
"&All"
"&Vše"

MDeleteFileSkip
"&Пропустить"
"&Skip"
"Přes&kočit"

MDeleteFileSkipAll
"П&ропустить все"
"S&kip all"
"Př&eskočit vše"

MDeleteFileCancel
"&Отменить"
"&Cancel"
"&Storno"

MDeleteLinkDelete
l:
"Удалить ссылку"
"Delete link"
"Smazat link"

MDeleteLinkUnlink
"Разорвать ссылку"
"Break link"
"Poškozený link"

MDeletingTitle
l:
"Удаление"
"Deleting"
"Mazání"

MDeleting
l:
"Удаление файла или папки"
"Deleting the file or folder"
"Mazání souboru nebo adresáře"

MDeletingWiping
"Уничтожение файла или папки"
"Wiping the file or folder"
"Vymazávání souboru nebo adresáře"

MDeleteRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"
"Soubor je určen pouze pro čtení"

MAskDeleteRO
"Вы хотите удалить его?"
"Do you wish to delete it?"
"Opravdu si ho přejete smazat?"

MAskWipeRO
"Вы хотите уничтожить его?"
"Do you wish to wipe it?"
"Opravdu si ho přejete vymazat?"

MDeleteHardLink1
l:
"Файл имеет несколько жестких ссылок"
"Several hard links link to this file."
"Více pevných linků ukazuje na tento soubor."

MDeleteHardLink2
"Уничтожение файла приведет к обнулению всех ссылающихся на него файлов."
"Wiping this file will void all files linking to it."
"Vymazání tohoto souboru zneplatní všechny soubory, které na něj linkují."

MDeleteHardLink3
"Уничтожать файл?"
"Do you wish to wipe this file?"
"Opravdu chcete vymazat tento soubor?"

MCannotDeleteFile
l:
"Ошибка удаления файла"
"Cannot delete the file"
"Nelze smazat soubor"

MCannotDeleteFolder
"Ошибка удаления папки"
"Cannot delete the folder"
"Nelze smazat adresář"

MDeleteRetry
"&Повторить"
"&Retry"
"&Znovu"

MDeleteSkip
"П&ропустить"
"&Skip"
"Přes&kočit"

MDeleteSkipAll
"Пропустить &все"
"S&kip all"
"Přeskočit &vše"

MDeleteCancel
"&Отменить"
"&Cancel"
"&Storno"

MCannotGetSecurity
l:
"Ошибка получения прав доступа к файлу"
"Cannot get file access rights for"
"Nemohu získat přístupová práva pro"

MCannotSetSecurity
"Ошибка установки прав доступа к файлу"
"Cannot set file access rights for"
"Nemohu nastavit přístupová práva pro"

MEditTitle
l:
"Редактор"
"Editor"
"Editor"

MAskReload
"уже загружен. Как открыть этот файл?"
"already loaded. How to open this file?"
"již otevřen. Jak otevřít tento soubor?"

MCurrent
"&Текущий"
"&Current"
"&Stávající"

MReload
"Пере&грузить"
"R&eload"
"&Znovu načíst"

MNewOpen
"&Новая копия"
"&New instance"
"&Nová instance"

MEditCannotOpen
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"

MEditReading
"Чтение файла"
"Reading the file"
"Načítám soubor"

MEditAskSave
"Файл был изменен"
"File has been modified"
"Soubor byl modifikován"

MEditAskSaveExt
"Файл был изменен внешней программой"
"The file was changed by an external program"
"Soubor byl změněný externím programem"

MEditSave
l:
"&Сохранить"
"&Save"
"&Uložit"

MEditNotSave
"&Не сохранять"
"Do &not save"
"&Neukládat"

MEditContinue
"&Продолжить редактирование"
"&Continue editing"
"&Pokračovat"

MEditBtnSaveAs
"Сохр&анить как"
"Save &as..."
"Ulož&it jako..."

MEditRO
l:
"имеет атрибут \"Только для чтения\""
"is a read-only file"
"je určen pouze pro čtení"

MEditExists
"уже существует"
"already exists"
"již existuje"

MEditOvr
"Вы хотите перезаписать его?"
"Do you wish to overwrite it?"
"Přejete si ho přepsat?"

MEditSaving
"Сохранение файла"
"Saving the file"
"Ukládám soubor"

MEditStatusLine
"Строка"
"Line"
"Řádek"

MEditStatusCol
"Кол"
"Col"
"Sloupec"

MEditRSH
l:
"предназначен только для чтения"
"is a read-only file"
"je určen pouze pro čtení"

MEditFileLong
"имеет размер %s,"
"has the size of %s,"
"má velikost %s,"

MEditFileLong2
"что превышает заданное ограничение в %s."
"which exceeds the configured maximum size of %s."
"která překračuje nastavenou maximální velikost %s."

MEditROOpen
"Вы хотите редактировать его?"
"Do you wish to edit it?"
"Opravdu si ho přejete upravit?"

MEditCanNotEditDirectory
l:
"Невозможно редактировать папку"
"It is impossible to edit the folder"
"Nelze editovat adresář"

MEditSearchTitle
l:
"Поиск"
"Search"
"Hledat"

MEditSearchFor
"&Искать"
"&Search for"
"&Hledat"

MEditSearchCase
"&Учитывать регистр"
"&Case sensitive"
"&Rozlišovat velikost písmen"

MEditSearchWholeWords
"Только &целые слова"
"&Whole words"
"&Celá slova"

MEditSearchReverse
"Обратн&ый поиск"
"Re&verse search"
"&Zpětné hledání"

MEditSearchSelFound
"&Выделять найденное"
"Se&lect found"
"Vy&ber nalezené"

MEditSearchSearch
"Искать"
"Search"
"Hledat"

MEditSearchCancel
"Отменить"
"Cancel"
"Storno"

MEditReplaceTitle
l:
"Замена"
"Replace"
"Nahradit"

MEditReplaceWith
"Заменить &на"
"R&eplace with"
"Nahradit &s"

MEditReplaceReplace
"&Замена"
"&Replace"
"&Nahradit"

MEditSearchingFor
l:
"Искать"
"Searching for"
"Vyhledávám"

MEditNotFound
"Строка не найдена"
"Could not find the string"
"Nemůžu najít řetězec"

MEditAskReplace
l:
"Заменить"
"Replace"
"Nahradit"

MEditAskReplaceWith
"на"
"with"
"s"

MEditReplace
"&Заменить"
"&Replace"
"&Nahradit"

MEditReplaceAll
"&Все"
"&All"
"&Vše"

MEditSkip
"&Пропустить"
"&Skip"
"Přes&kočit"

MEditCancel
"&Отменить"
"&Cancel"
"&Storno"

MEditOpenCreateLabel
"Открыть/создать файл:"
"Open/create file:"
"Open/create file:"

MEditGoToLine
l:
"Перейти"
"Go to position"
"Jít na pozici"

MFolderShortcutsTitle
l:
"Ссылки на папки"
"Folder shortcuts"
"Adresářové zkratky"

MFolderShortcutBottom
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"

MShortcutNone
"<отсутствует>"
"<none>"
"<není>"

MShortcutPlugin
"<плагин>"
"<plugin>"
"<plugin>"

MEnterShortcut
"Введите новую ссылку:"
"Enter new shortcut:"
"Zadejte novou zkratku:"

MNeedNearPath
"Перейти в ближайшую доступную папку?"
"Jump to the nearest existing folder?"
"Skočit na nejbližší existující adresář?"

MSaveThisShortcut
"Запомнить эту ссылку?"
"Save this shortcuts?"
"Uložit tyto zkratky?"

MEditF1
l:
l://functional keys - 6 characters max, 12 keys, "OEM" is F8 dupe!
"Помощь"
"Help"
"Pomoc"

MEditF2
"Сохран"
"Save"
"Uložit"

MEditF3
""
""
""

MEditF4
""
""
""

MEditF5
""
""
""

MEditF6
"Просм"
"View"
"Zobraz"

MEditF7
"Поиск"
"Search"
"Hledat"

MEditF8
"ANSI"
"ANSI"
"ANSI"

MEditF9
""
""
""

MEditF10
"Выход"
"Quit"
"Konec"

MEditF11
"Модули"
"Plugin"
"Plugin"

MEditF12
"Экраны"
"Screen"
"Obraz."

MEditF8DOS
le:// don't count this - it's a F8 another text
"OEM"
"OEM"
"OEM"

MEditShiftF1
l:
l://Editor: Shift
""
""
""

MEditShiftF2
"Сохр.в"
"SaveAs"
"UlJako"

MEditShiftF3
""
""
""

MEditShiftF4
"Редак."
"Edit.."
"Edit.."

MEditShiftF5
""
""
""

MEditShiftF6
""
""
""

MEditShiftF7
"Дальше"
"Next"
"Další"

MEditShiftF8
"Таблиц"
"Table"
"ZnSady"

MEditShiftF9
""
""
""

MEditShiftF10
"СхрВых"
"SaveQ"
"UlKone"

MEditShiftF11
""
""
""

MEditShiftF12
""
""
""

MEditAltF1
l:
l://Editor: Alt
""
""
""

MEditAltF2
""
""
""

MEditAltF3
""
""
""

MEditAltF4
""
""
""

MEditAltF5
"Печать"
"Print"
"Tisk"

MEditAltF6
""
""
""

MEditAltF7
"Назад"
"Prev"
"Předch"

MEditAltF8
"Строка"
"Goto"
"Jít na"

MEditAltF9
"Видео"
"Video"
"Video"

MEditAltF10
""
""
""

MEditAltF11
"ИстПр"
"ViewHs"
"ProhHs"

MEditAltF12
""
""
""

MEditCtrlF1
l:
l://Editor: Ctrl
""
""
""

MEditCtrlF2
""
""
""

MEditCtrlF3
""
""
""

MEditCtrlF4
""
""
""

MEditCtrlF5
""
""
""

MEditCtrlF6
""
""
""

MEditCtrlF7
"Замена"
"Replac"
"Nahraď"

MEditCtrlF8
""
""
""

MEditCtrlF9
""
""
""

MEditCtrlF10
"Позиц"
"GoFile"
"JdiSou"

MEditCtrlF11
""
""
""

MEditCtrlF12
""
""
""

MEditAltShiftF1
l:
l://Editor: AltShift
""
""
""

MEditAltShiftF2
""
""
""

MEditAltShiftF3
""
""
""

MEditAltShiftF4
""
""
""

MEditAltShiftF5
""
""
""

MEditAltShiftF6
""
""
""

MEditAltShiftF7
""
""
""

MEditAltShiftF8
""
""
""

MEditAltShiftF9
"Конфиг"
"Config"
"Nastav"

MEditAltShiftF10
""
""
""

MEditAltShiftF11
""
""
""

MEditAltShiftF12
""
""
""

MEditCtrlShiftF1
l:
l://Editor: CtrlShift
""
""
""

MEditCtrlShiftF2
""
""
""

MEditCtrlShiftF3
""
""
""

MEditCtrlShiftF4
""
""
""

MEditCtrlShiftF5
""
""
""

MEditCtrlShiftF6
""
""
""

MEditCtrlShiftF7
""
""
""

MEditCtrlShiftF8
""
""
""

MEditCtrlShiftF9
""
""
""

MEditCtrlShiftF10
""
""
""

MEditCtrlShiftF11
""
""
""

MEditCtrlShiftF12
""
""
""

MEditCtrlAltF1
l:
l:// Editor: CtrlAlt
""
""
""

MEditCtrlAltF2
""
""
""

MEditCtrlAltF3
""
""
""

MEditCtrlAltF4
""
""
""

MEditCtrlAltF5
""
""
""

MEditCtrlAltF6
""
""
""

MEditCtrlAltF7
""
""
""

MEditCtrlAltF8
""
""
""

MEditCtrlAltF9
""
""
""

MEditCtrlAltF10
""
""
""

MEditCtrlAltF11
""
""
""

MEditCtrlAltF12
""
""
""

MEditCtrlAltShiftF1
l:
l:// Editor: CtrlAltShift
""
""
""

MEditCtrlAltShiftF2
""
""
""

MEditCtrlAltShiftF3
""
""
""

MEditCtrlAltShiftF4
""
""
""

MEditCtrlAltShiftF5
""
""
""

MEditCtrlAltShiftF6
""
""
""

MEditCtrlAltShiftF7
""
""
""

MEditCtrlAltShiftF8
""
""
""

MEditCtrlAltShiftF9
""
""
""

MEditCtrlAltShiftF10
""
""
""

MEditCtrlAltShiftF11
""
""
""

MEditCtrlAltShiftF12
le://End of functional keys (Editor)
""
""
""

MSingleEditF1
l:
l://Single Editor: functional keys - 6 characters max, 12 keys, "OEM" is F8 dupe!
"Помощь"
"Help"
"Pomoc"

MSingleEditF2
"Сохран"
"Save"
"Uložit"

MSingleEditF3
""
""
""

MSingleEditF4
""
""
""

MSingleEditF5
""
""
""

MSingleEditF6
"Просм"
"View"
"Zobraz"

MSingleEditF7
"Поиск"
"Search"
"Hledat"

MSingleEditF8
"ANSI"
"ANSI"
"ANSI"

MSingleEditF9
""
""
""

MSingleEditF10
"Выход"
"Quit"
"Konec"

MSingleEditF11
"Модули"
"Plugin"
"Plugin"

MSingleEditF12
"Экраны"
"Screen"
"Obraz."

MSingleEditF8DOS
le:// don't count this - it's a F8 another text
"OEM"
"OEM"
"OEM"

MSingleEditShiftF1
l:
l://Single Editor: Shift
""
""
""

MSingleEditShiftF2
"Сохр.в"
"SaveAs"
"UlJako"

MSingleEditShiftF3
""
""
""

MSingleEditShiftF4
""
""
""

MSingleEditShiftF5
""
""
""

MSingleEditShiftF6
""
""
""

MSingleEditShiftF7
"Дальше"
"Next"
"Další"

MSingleEditShiftF8
"Таблиц"
"Table"
"ZnSady"

MSingleEditShiftF9
""
""
""

MSingleEditShiftF10
"СхрВых"
"SaveQ"
"UlKone"

MSingleEditShiftF11
""
""
""

MSingleEditShiftF12
""
""
""

MSingleEditAltF1
l:
l://Single Editor: Alt
""
""
""

MSingleEditAltF2
""
""
""

MSingleEditAltF3
""
""
""

MSingleEditAltF4
""
""
""

MSingleEditAltF5
"Печать"
"Print"
"Tisk"

MSingleEditAltF6
""
""
""

MSingleEditAltF7
""
""
""

MSingleEditAltF8
"Строка"
"Goto"
"Jít na"

MSingleEditAltF9
"Видео"
"Video"
"Video"

MSingleEditAltF10
""
""
""

MSingleEditAltF11
"ИстПр"
"ViewHs"
"ProhHs"

MSingleEditAltF12
""
""
""

MSingleEditCtrlF1
l:
l://Single Editor: Ctrl
""
""
""

MSingleEditCtrlF2
""
""
""

MSingleEditCtrlF3
""
""
""

MSingleEditCtrlF4
""
""
""

MSingleEditCtrlF5
""
""
""

MSingleEditCtrlF6
""
""
""

MSingleEditCtrlF7
"Замена"
"Replac"
"Nahraď"

MSingleEditCtrlF8
""
""
""

MSingleEditCtrlF9
""
""
""

MSingleEditCtrlF10
""
""
""

MSingleEditCtrlF11
""
""
""

MSingleEditCtrlF12
""
""
""

MSingleEditAltShiftF1
l:
l://Single Editor: AltShift
""
""
""

MSingleEditAltShiftF2
""
""
""

MSingleEditAltShiftF3
""
""
""

MSingleEditAltShiftF4
""
""
""

MSingleEditAltShiftF5
""
""
""

MSingleEditAltShiftF6
""
""
""

MSingleEditAltShiftF7
""
""
""

MSingleEditAltShiftF8
""
""
""

MSingleEditAltShiftF9
"Конфиг"
"Config"
"Nastav"

MSingleEditAltShiftF10
""
""
""

MSingleEditAltShiftF11
""
""
""

MSingleEditAltShiftF12
""
""
""

MSingleEditCtrlShiftF1
l:
l://Single Editor: CtrlShift
""
""
""

MSingleEditCtrlShiftF2
""
""
""

MSingleEditCtrlShiftF3
""
""
""

MSingleEditCtrlShiftF4
""
""
""

MSingleEditCtrlShiftF5
""
""
""

MSingleEditCtrlShiftF6
""
""
""

MSingleEditCtrlShiftF7
""
""
""

MSingleEditCtrlShiftF8
""
""
""

MSingleEditCtrlShiftF9
""
""
""

MSingleEditCtrlShiftF10
""
""
""

MSingleEditCtrlShiftF11
""
""
""

MSingleEditCtrlShiftF12
""
""
""

MSingleEditCtrlAltF1
l:
l://Single Editor: CtrlAlt
""
""
""

MSingleEditCtrlAltF2
""
""
""

MSingleEditCtrlAltF3
""
""
""

MSingleEditCtrlAltF4
""
""
""

MSingleEditCtrlAltF5
""
""
""

MSingleEditCtrlAltF6
""
""
""

MSingleEditCtrlAltF7
""
""
""

MSingleEditCtrlAltF8
""
""
""

MSingleEditCtrlAltF9
""
""
""

MSingleEditCtrlAltF10
""
""
""

MSingleEditCtrlAltF11
""
""
""

MSingleEditCtrlAltF12
""
""
""

MSingleEditCtrlAltShiftF1
l:
l://Single Editor: CtrlAltShift
""
""
""

MSingleEditCtrlAltShiftF2
""
""
""

MSingleEditCtrlAltShiftF3
""
""
""

MSingleEditCtrlAltShiftF4
""
""
""

MSingleEditCtrlAltShiftF5
""
""
""

MSingleEditCtrlAltShiftF6
""
""
""

MSingleEditCtrlAltShiftF7
""
""
""

MSingleEditCtrlAltShiftF8
""
""
""

MSingleEditCtrlAltShiftF9
""
""
""

MSingleEditCtrlAltShiftF10
""
""
""

MSingleEditCtrlAltShiftF11
""
""
""

MSingleEditCtrlAltShiftF12
le://End of functional keys (Single Editor)
""
""
""

MEditSaveAs
l:
"Сохранить &файл как"
"Save file &as"
"Uložit soubor jako"

MEditCodePage
"Кодовая страница:"
"Code page:"
"Kódová stránka:"

MEditAddSignature
"Добавить сигнатуру (BOM)"
"Add signature (BOM)"
"Přidat signaturu (BOM)"

MEditSaveAsFormatTitle
"Изменить перевод строки:"
"Change line breaks to:"
"Změnit zakončení řádků na:"

MEditSaveOriginal
"&исходный формат"
"Do n&ot change"
"&Beze změny"

MEditSaveDOS
"в форма&те DOS/Windows (CR LF)"
"&Dos/Windows format (CR LF)"
"&Dos/Windows formát (CR LF)"

MEditSaveUnix
"в формат&е UNIX (LF)"
"&Unix format (LF)"
"&Unix formát (LF)"

MEditSaveMac
"в фор&мате MAC (CR)"
"&Mac format (CR)"
"&Mac formát (CR)"

MEditCannotSave
"Ошибка сохранения файла"
"Cannot save the file"
"Nelze uložit soubor"

MEditSavedChangedNonFile
"Файл изменен, но файл или папка, в которой он находился,"
"The file is changed but the file or the folder containing"
"Soubor je změněn, ale soubor, nebo adresář obsahující"

MEditSavedChangedNonFile1
"Файл или папка, в которой он находился,"
"The file or the folder containing"
"Soubor nebo adresář obsahující"

MEditSavedChangedNonFile2
"был перемещен или удален."
"this file was moved or deleted."
"tento soubor byl přesunut, nebo smazán."

MEditNewPath1
"Путь к редактируемому файлу не существует,"
"The path to the edited file does not exist,"
"Cesta k editovanému souboru neexistuje,"

MEditNewPath2
"но будет создан при сохранении файла."
"but will be created when the file is saved."
"ale bude vytvořena při uložení souboru."

MEditNewPath3
"Продолжать?"
"Continue?"
"Pokračovat?"

MEditNewPlugin1
"Имя редактируемого файла не может быть пустым"
"The name of the file to edit cannot be empty"
"Název souboru k editaci nesmí být prázdné"

MEditDataLostWarn1
"Файл содержит символы, которые невозможно"
"This file contains characters, which cannot be"
"Tento soubor obsahuje znaky, které nemohou být"

MEditDataLostWarn2
"корректно преобразовать в выбранную кодировку."
"correctly translated using the selected codepage."
"korektně přeloženy do zvoleného kódování."

MEditDataLostWarn3
"Продолжить?"
"Continue?"
"Pokračovat?"

MEditDataLostWarn4
"Сохранять файл не рекомендуется."
"It is not recommended to save this file."
"Není doporučeno uložit tento soubor."

MColumnName
l:
"Имя"
"Name"
"Název"

MColumnSize
"Размер"
"Size"
"Velikost"

MColumnPacked
"Упаков"
"Packed"
"Komprimovaný"

MColumnDate
"Дата"
"Date"
"Datum"

MColumnTime
"Время"
"Time"
"Čas"

MColumnModified
"Модификация"
"Modified"
"Modifikován"

MColumnCreated
"Создание"
"Created"
"Vytvořen"

MColumnAccessed
"Доступ"
"Accessed"
"Přístup"

MColumnAttr
"Атриб"
"Attr"
"Attr"

MColumnDescription
"Описание"
"Description"
"Popis"

MColumnOwner
"Владелец"
"Owner"
"Vlastník"

MColumnMumLinks
"КлС"
"NmL"
"PočLn"

MListUp
l:
"Вверх"
"  Up  "
"Nahoru"

MListFolder
"Папка"
"Folder"
"Adresář"

MListSymLink
"Ссылка"
"Symlink"
"Link"

MListJunction
"Связь"
"Junction"
"Křížení"

MListBytes
"Б"
"B"
"B"

MListKb
"К"
"K"
"K"

MListMb
"М"
"M"
"M"

MListGb
"Г"
"G"
"G"

MListTb
"Т"
"T"
"T"

MListFileSize
" %s байт в 1 файле "
" %s bytes in 1 file "
" %s bytů v 1 souboru "

MListFilesSize1
" %s байт в %d файле "
" %s bytes in %d files "
" %s bytů v %d souborech "

MListFilesSize2
" %s байт в %d файлах "
" %s bytes in %d files "
" %s bytů v %d souborech "

MListFreeSize
" %s байт свободно "
" %s free bytes "
" %s volných bytů "

MDirInfoViewTitle
l:
"Просмотр"
"View"
"Zobraz"

MFileToEdit
"Редактировать файл:"
"File to edit:"
"Soubor k editaci:"

MUnselectTitle
l:
"Снять"
"Deselect"
"Odznačit"

MSelectTitle
"Пометить"
"Select"
"Označit"

MSelectFilter
"&Фильтр"
"&Filter"
"&Filtr"

MCompareTitle
l:
"Сравнение"
"Compare"
"Porovnat"

MCompareFilePanelsRequired1
"Для сравнения папок требуются"
"Two file panels are required to perform"
"Pro provedení příkazu Porovnat adresáře"

MCompareFilePanelsRequired2
"две файловые панели"
"the Compare folders command"
"jsou nutné dva souborové panely"

MCompareSameFolders1
"Содержимое папок,"
"The folders contents seems"
"Obsahy adresářů jsou"

MCompareSameFolders2
"скорее всего, одинаково"
"to be identical"
"identické"

MSelectAssocTitle
l:
"Выберите ассоциацию"
"Select association"
"Vyber závislosti"

MAssocTitle
l:
"Ассоциации для файлов"
"File associations"
"Závislosti souborů"

MAssocBottom
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"

MAskDelAssoc
"Вы хотите удалить ассоциацию для"
"Do you wish to delete association for"
"Přejete si smazat závislost pro"

MAssocNeedMask
"Пожалуйста, укажите маску файлов"
"Please specify a file mask"
"Prosím zadejte masku souborů"

MFileAssocTitle
l:
"Редактирование ассоциаций файлов"
"Edit file associations"
"Upravit závislosti souborů"

MFileAssocMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"
"&Maska nebo masky souborů:"

MFileAssocDescr
"&Описание ассоциации:"
"&Description of the association:"
"&Popis asociací:"

MFileAssocExec
"Команда, &выполняемая по Enter:"
"E&xecute command (used for Enter):"
"&Vykonat příkaz (použito pro Enter):"

MFileAssocAltExec
"Коман&да, выполняемая по Ctrl-PgDn:"
"Exec&ute command (used for Ctrl-PgDn):"
"V&ykonat příkaz (použito pro Ctrl-PgDn):"

MFileAssocView
"Команда &просмотра, выполняемая по F3:"
"&View command (used for F3):"
"Příkaz &Zobraz (použito pro F3):"

MFileAssocAltView
"Команда просмотра, в&ыполняемая по Alt-F3:"
"V&iew command (used for Alt-F3):"
"Příkaz Z&obraz (použito pro Alt-F3):"

MFileAssocEdit
"Команда &редактирования, выполняемая по F4:"
"&Edit command (used for F4):"
"Příkaz &Edituj (použito pro F4):"

MFileAssocAltEdit
"Команда редактировани&я, выполняемая по Alt-F4:"
"Edit comm&and (used for Alt-F4):"
"Příkaz Editu&j (použito pro Alt-F4):"

MViewF1
l:
l://Viewer: functional keys, 12 keys, except F2 - 2 keys, and F8 - 2 keys
"Помощь"
"Help"
"Pomoc"

MViewF2
le:// this is another text for F2
"Сверн"
"Wrap"
"Zalom"

MViewF3
"Выход"
"Quit"
"Konec"

MViewF4
"Код"
"Hex"
"Hex"

MViewF5
""
""
""

MViewF6
"Редакт"
"Edit"
"Edit"

MViewF7
"Поиск"
"Search"
"Hledat"

MViewF8
"ANSI"
"ANSI"
"ANSI"

MViewF9
""
""
""

MViewF10
"Выход"
"Quit"
"Konec"

MViewF11
"Модули"
"Plugins"
"Plugin"

MViewF12
"Экраны"
"Screen"
"Obraz."

MViewF2Unwrap
"Развер"
"Unwrap"
"Nezal"

MViewF4Text
l:// this is another text for F4
"Текст"
"Text"
"Text"

MViewF8DOS
"OEM"
"OEM"
"OEM"

MViewShiftF1
l:
l://Viewer: Shift
""
""
""

MViewShiftF2
"Слова"
"WWrap"
"ZalSlo"

MViewShiftF3
""
""
""

MViewShiftF4
""
""
""

MViewShiftF5
""
""
""

MViewShiftF6
""
""
""

MViewShiftF7
"Дальше"
"Next"
"Další"

MViewShiftF8
"Таблиц"
"Table"
"ZnSady"

MViewShiftF9
""
""
""

MViewShiftF10
""
""
""

MViewShiftF11
""
""
""

MViewShiftF12
""
""
""

MViewAltF1
l:
l://Viewer: Alt
""
""
""

MViewAltF2
""
""
""

MViewAltF3
""
""
""

MViewAltF4
""
""
""

MViewAltF5
"Печать"
"Print"
"Tisk"

MViewAltF6
""
""
""

MViewAltF7
"Назад"
"Prev"
"Předch"

MViewAltF8
"Перейт"
"Goto"
"Jít na"

MViewAltF9
"Видео"
"Video"
"Video"

MViewAltF10
""
""
""

MViewAltF11
"ИстПр"
"ViewHs"
"ProhHs"

MViewAltF12
""
""
""

MViewCtrlF1
l:
l://Viewer: Ctrl
""
""
""

MViewCtrlF2
""
""
""

MViewCtrlF3
""
""
""

MViewCtrlF4
""
""
""

MViewCtrlF5
""
""
""

MViewCtrlF6
""
""
""

MViewCtrlF7
""
""
""

MViewCtrlF8
""
""
""

MViewCtrlF9
""
""
""

MViewCtrlF10
"Позиц"
"GoFile"
"JítSou"

MViewCtrlF11
""
""
""

MViewCtrlF12
""
""
""

MViewAltShiftF1
l:
l://Viewer: AltShift
""
""
""

MViewAltShiftF2
""
""
""

MViewAltShiftF3
""
""
""

MViewAltShiftF4
""
""
""

MViewAltShiftF5
""
""
""

MViewAltShiftF6
""
""
""

MViewAltShiftF7
""
""
""

MViewAltShiftF8
""
""
""

MViewAltShiftF9
"Конфиг"
"Config"
"Nastav"

MViewAltShiftF10
""
""
""

MViewAltShiftF11
""
""
""

MViewAltShiftF12
""
""
""

MViewCtrlShiftF1
l:
l://Viewer: CtrlShift
""
""
""

MViewCtrlShiftF2
""
""
""

MViewCtrlShiftF3
""
""
""

MViewCtrlShiftF4
""
""
""

MViewCtrlShiftF5
""
""
""

MViewCtrlShiftF6
""
""
""

MViewCtrlShiftF7
""
""
""

MViewCtrlShiftF8
""
""
""

MViewCtrlShiftF9
""
""
""

MViewCtrlShiftF10
""
""
""

MViewCtrlShiftF11
""
""
""

MViewCtrlShiftF12
""
""
""

MViewCtrlAltF1
l:
l://Viewer: CtrlAlt
""
""
""

MViewCtrlAltF2
""
""
""

MViewCtrlAltF3
""
""
""

MViewCtrlAltF4
""
""
""

MViewCtrlAltF5
""
""
""

MViewCtrlAltF6
""
""
""

MViewCtrlAltF7
""
""
""

MViewCtrlAltF8
""
""
""

MViewCtrlAltF9
""
""
""

MViewCtrlAltF10
""
""
""

MViewCtrlAltF11
""
""
""

MViewCtrlAltF12
""
""
""

MViewCtrlAltShiftF1
l:
l://Viewer: CtrlAltShift
""
""
""

MViewCtrlAltShiftF2
""
""
""

MViewCtrlAltShiftF3
""
""
""

MViewCtrlAltShiftF4
""
""
""

MViewCtrlAltShiftF5
""
""
""

MViewCtrlAltShiftF6
""
""
""

MViewCtrlAltShiftF7
""
""
""

MViewCtrlAltShiftF8
""
""
""

MViewCtrlAltShiftF9
""
""
""

MViewCtrlAltShiftF10
""
""
""

MViewCtrlAltShiftF11
""
""
""

MViewCtrlAltShiftF12
le://end of functional keys (Viewer)
""
""
""

MSingleViewF1
l:
l://Single Viewer: functional keys, 12 keys, except F2 - 2 keys, and F8 - 2 keys
"Помощь"
"Help"
"Pomoc"

MSingleViewF2
"Сверн"
"Wrap"
"Zalom"

MSingleViewF3
"Выход"
"Quit"
"Konec"

MSingleViewF4
"Код"
"Hex"
"Hex"

MSingleViewF5
""
""
""

MSingleViewF6
"Редакт"
"Edit"
"Edit"

MSingleViewF7
"Поиск"
"Search"
"Hledat"

MSingleViewF8
"ANSI"
"ANSI"
"ANSI"

MSingleViewF9
""
""
""

MSingleViewF10
"Выход"
"Quit"
"Konec"

MSingleViewF11
"Модули"
"Plugins"
"Plugin"

MSingleViewF12
"Экраны"
"Screen"
"Obraz."

MSingleViewF2Unwrap
l:// this is another text for F2
"Развер"
"Unwrap"
"Nezal"

MSingleViewF4Text
l:// this is another text for F4
"Текст"
"Text"
"Text"

MSingleViewF8DOS
"OEM"
"OEM"
"OEM"

MSingleViewShiftF1
l:
l://Single Viewer: Shift
""
""
""

MSingleViewShiftF2
"Слова"
"WWrap"
"ZalSlo"

MSingleViewShiftF3
""
""
""

MSingleViewShiftF4
""
""
""

MSingleViewShiftF5
""
""
""

MSingleViewShiftF6
""
""
""

MSingleViewShiftF7
"Дальше"
"Next"
"Další"

MSingleViewShiftF8
"Таблиц"
"Table"
"ZnSady"

MSingleViewShiftF9
""
""
""

MSingleViewShiftF10
""
""
""

MSingleViewShiftF11
""
""
""

MSingleViewShiftF12
""
""
""

MSingleViewAltF1
l:
l://Single Viewer: Alt
""
""
""

MSingleViewAltF2
""
""
""

MSingleViewAltF3
""
""
""

MSingleViewAltF4
""
""
""

MSingleViewAltF5
"Печать"
"Print"
"Tisk"

MSingleViewAltF6
""
""
""

MSingleViewAltF7
"Назад"
"Prev"
"Předch"

MSingleViewAltF8
"Перейт"
"Goto"
"Jít na"

MSingleViewAltF9
"Видео"
"Video"
"Video"

MSingleViewAltF10
""
""
""

MSingleViewAltF11
"ИстПр"
"ViewHs"
"ProhHs"

MSingleViewAltF12
""
""
""

MSingleViewCtrlF1
l:
l://Single Viewer: Ctrl
""
""
""

MSingleViewCtrlF2
""
""
""

MSingleViewCtrlF3
""
""
""

MSingleViewCtrlF4
""
""
""

MSingleViewCtrlF5
""
""
""

MSingleViewCtrlF6
""
""
""

MSingleViewCtrlF7
""
""
""

MSingleViewCtrlF8
""
""
""

MSingleViewCtrlF9
""
""
""

MSingleViewCtrlF10
""
""
""

MSingleViewCtrlF11
""
""
""

MSingleViewCtrlF12
""
""
""

MSingleViewAltShiftF1
l:
l://Single Viewer: AltShift
""
""
""

MSingleViewAltShiftF2
""
""
""

MSingleViewAltShiftF3
""
""
""

MSingleViewAltShiftF4
""
""
""

MSingleViewAltShiftF5
""
""
""

MSingleViewAltShiftF6
""
""
""

MSingleViewAltShiftF7
""
""
""

MSingleViewAltShiftF8
""
""
""

MSingleViewAltShiftF9
"Конфиг"
"Config"
"Nastav"

MSingleViewAltShiftF10
""
""
""

MSingleViewAltShiftF11
""
""
""

MSingleViewAltShiftF12
""
""
""

MSingleViewCtrlShiftF1
l:
l://Single Viewer: CtrlShift
""
""
""

MSingleViewCtrlShiftF2
""
""
""

MSingleViewCtrlShiftF3
""
""
""

MSingleViewCtrlShiftF4
""
""
""

MSingleViewCtrlShiftF5
""
""
""

MSingleViewCtrlShiftF6
""
""
""

MSingleViewCtrlShiftF7
""
""
""

MSingleViewCtrlShiftF8
""
""
""

MSingleViewCtrlShiftF9
""
""
""

MSingleViewCtrlShiftF10
""
""
""

MSingleViewCtrlShiftF11
""
""
""

MSingleViewCtrlShiftF12
""
""
""

MSingleViewCtrlAltF1
l:
l://Single Viewer: CtrlAlt
""
""
""

MSingleViewCtrlAltF2
""
""
""

MSingleViewCtrlAltF3
""
""
""

MSingleViewCtrlAltF4
""
""
""

MSingleViewCtrlAltF5
""
""
""

MSingleViewCtrlAltF6
""
""
""

MSingleViewCtrlAltF7
""
""
""

MSingleViewCtrlAltF8
""
""
""

MSingleViewCtrlAltF9
""
""
""

MSingleViewCtrlAltF10
""
""
""

MSingleViewCtrlAltF11
""
""
""

MSingleViewCtrlAltF12
""
""
""

MSingleViewCtrlAltShiftF1
l:
l://Single Viewer: CtrlAltShift
""
""
""

MSingleViewCtrlAltShiftF2
""
""
""

MSingleViewCtrlAltShiftF3
""
""
""

MSingleViewCtrlAltShiftF4
""
""
""

MSingleViewCtrlAltShiftF5
""
""
""

MSingleViewCtrlAltShiftF6
""
""
""

MSingleViewCtrlAltShiftF7
""
""
""

MSingleViewCtrlAltShiftF8
""
""
""

MSingleViewCtrlAltShiftF9
""
""
""

MSingleViewCtrlAltShiftF10
""
""
""

MSingleViewCtrlAltShiftF11
""
""
""

MSingleViewCtrlAltShiftF12
le://end of functional keys (Single Viewer)
""
""
""

MInViewer
"просмотр %s"
"view %s"
"prohlížení %s"

MInEditor
"редактирование %s"
"edit %s"
"editace %s"

MFilterTitle
l:
"Меню фильтров"
"Filters menu"
"Menu filtrů"

MFilterBottom
"+,-,Пробел,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Space,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Mezera,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"

MPanelFileType
"Файлы панели"
"Panel file type"
"Typ panelu souborů"

MFolderFileType
"Папки"
"Folders"
"Adresáře"

MCanEditCustomFilterOnly
"Только пользовательский фильтр можно редактировать"
"Only custom filter can be edited"
"Jedině vlastní filtr může být upraven"

MAskDeleteFilter
"Вы хотите удалить фильтр"
"Do you wish to delete the filter"
"Přejete si smazat filtr"

MCanDeleteCustomFilterOnly
"Только пользовательский фильтр может быть удален"
"Only custom filter can be deleted"
"Jedině vlastní filtr může být smazán"

MFindFileTitle
l:
"Поиск файла"
"Find file"
"Hledat soubor"

MFindFileResultTitle
"Поиск файла - результат"
"Find file - result"
"Hledat soubor - výsledek"

MFindFileMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"
"Maska nebo masky souborů:"

MFindFileText
"&Содержащих текст:"
"Containing &text:"
"Obsahující te&xt:"

MFindFileHex
"&Содержащих 16-ричный код:"
"Con&taining hex:"
"Obsahující &hex:"

MFindFileCodePage
"Используя таблицу сим&волов:"
"Using character ta&ble:"
"Použít &znakovou sadu:"

MFindFileCase
"&Учитывать регистр"
"&Case sensitive"
"Roz&lišovat velikost písmen"

MFindFileWholeWords
"Только &целые слова"
"&Whole words"
"&Celá slova"

MFindFileAllTables
"Все таблицы символов"
"All character tables"
"Všechny znakové sady"

MFindArchives
"Искать в а&рхивах"
"Search in arch&ives"
"Hledat v a&rchívech"

MFindFolders
"Искать п&апки"
"Search for f&olders"
"Hledat a&dresáře"

MFindSymLinks
"Искать в символи&ческих ссылках"
"Search in symbolic lin&ks"
"Hledat v s&ymbolických lincích"

MSearchForHex
"Искать 16-ричн&ый код"
"Search for &hex"
"Hledat &hex"

MSearchWhere
"Выберите область поиска:"
"Select search area:"
"Zvolte oblast hledání:"

MSearchAllDisks
"На всех несъемных &дисках"
"In &all non-removable drives"
"Ve všech p&evných discích"

MSearchAllButNetwork
"На всех &локальных дисках"
"In all &local drives"
"Ve všech &lokálních discích"

MSearchInPATH
"В PATH-катало&гах"
"In &PATH folders"
"V adresářích z &PATH"

MSearchFromRootOfDrive
"С кор&ня диска"
"From the &root of"
"V &kořeni"

MSearchFromRootFolder
"С кор&невой папки"
"From the &root folder"
"V kořeno&vém adresáři"

MSearchFromCurrent
"С &текущей папки"
"From the curre&nt folder"
"V tomto adresář&i"

MSearchInCurrent
"Только в теку&щей папке"
"The current folder onl&y"
"P&ouze v tomto adresáři"

MSearchInSelected
"В &отмеченных папках"
"&Selected folders"
"Ve vy&braných adresářích"

MFindUseFilter
"Исполь&зовать фильтр"
"&Use filter"
"Použít f&iltr"

MFindAdvancedOptions
"Дополнит&ельные параметры"
"Advanced options"
"Pokročilé nastavení"

MFindUsingFilter
"используя фильтр"
"using filter"
"používám filtr"

MFindFileFind
"&Искать"
"&Find"
"&Hledat"

MFindFileDrive
"Дис&к"
"Dri&ve"
"D&isk"

MFindFileSetFilter
"&Фильтр"
"Filt&er"
"&Filtr"

MFindFileAdvanced
"До&полнительно"
"Advance&d"
"Pokr&očilé"

MFindFileTable
"Т&аблица"
"Ta&ble"
"Zna&ková sada"

MFindSearchingIn
"Поиск%s в:"
"Searching%s in:"
"Hledám%s v:"

MFindNewSearch
"&Новый поиск"
"&New search"
"&Nové hledání"

MFindGoTo
"Пе&рейти"
"&Go to"
"&Jdi na"

MFindView
"&Смотреть"
"&View"
"Zo&braz"

MFindPanel
"Пане&ль"
"&Panel"
"&Panel"

MFindStop
"С&топ"
"&Stop"
"&Stop"

MFindDone
l:
"Поиск закончен. Найдено %d файл(ов) и %d папка(ок)"
"Search done. Found %d file(s) and %d folder(s)"
"Hledání ukončeno. Nalezeno %d soubor(ů) a %d adresář(ů)"

MFindCancel
"Отм&ена"
"&Cancel"
"&Storno"

MFindFound
l:
"Найдено"
"Found"
"Nalezeno"

MFindFileFolder
l:
"Папка"
"Folder"
"Adresář"

MFindFileAdvancedTitle
l:
"Дополнительные параметры поиска"
"Find file advanced options"
"Pokročilé nastavení vyhledávání souborů"

MFindFileSearchFirst
"Проводить поиск в &первых:"
"Search only in the &first:"
"Hledat po&uze v prvních:"

MFoldTreeSearch
l:
"Поиск:"
"Search:"
"Hledat:"

MGetTableTitle
l:
"Таблицы"
"Tables"
"Znakové sady:"

MGetTableBottomTitle
"CtrlH, Ins"
"CtrlH, Ins"
"CtrlH, Ins"

MHighlightTitle
l:
"Раскраска файлов"
"Files highlighting"
"Zvýrazňování souborů"

MHighlightBottom
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Nahoru,Ctrl-Dolů"

MHighlightUpperSortGroup
"Верхняя группа сортировки"
"Upper sort group"
"Vzesupné řazení"

MHighlightLowerSortGroup
"Нижняя группа сортировки"
"Lower sort group"
"Sestupné řazení"

MHighlightLastGroup
"Наименее приоритетная группа раскраски"
"Lowest priority highlighting group"
"Zvýraznění nejnižší prority"

MHighlightAskDel
"Вы хотите удалить раскраску для"
"Do you wish to delete highlighting for"
"Přejete si smazat zvýraznění pro"

MHighlightWarning
"Будут потеряны все Ваши настройки!"
"You will lose all changes!"
"Všechny změny budou ztraceny!"

MHighlightAskRestore
"Вы хотите восстановить раскраску файлов по умолчанию?"
"Do you wish to restore default highlighting?"
"Přejete si obnovit výchozí nastavení?"

MHighlightEditTitle
l:
"Редактирование раскраски файлов"
"Edit files highlighting"
"Upravit zvýrazňování souborů"

MHighlightMarkChar
"Оп&циональный символ пометки,"
"Optional markin&g character,"
"Volitelný &znak pro označení určených souborů,"

MHighlightTransparentMarkChar
"прозра&чный"
"tra&nsparent"
"průh&ledný"

MHighlightColors
" Цвета файлов (\"черный на черном\" - цвет по умолчанию) "
" File name colors (\"black on black\" - default color) "
" Barva názvu souborů (\"černá na černé\" - výchozí barva) "

MHighlightFileName1
"&1. Обычное имя файла                "
"&1. Normal file name               "
"&1. Normální soubor"

MHighlightFileName2
"&3. Помеченное имя файла             "
"&3. Selected file name             "
"&3. Vybraný soubor"

MHighlightFileName3
"&5. Имя файла под курсором           "
"&5. File name under cursor         "
"&5. Soubor pod kurzorem"

MHighlightFileName4
"&7. Помеченное под курсором имя файла"
"&7. File name selected under cursor"
"&7. Vybraný soubor pod kurzorem"

MHighlightMarking1
"&2. Пометка"
"&2. Marking"
"&2. Označení"

MHighlightMarking2
"&4. Пометка"
"&4. Marking"
"&4. Označení"

MHighlightMarking3
"&6. Пометка"
"&6. Marking"
"&6. Označení"

MHighlightMarking4
"&8. Пометка"
"&8. Marking"
"&8. Označení"

MHighlightExample1
"║filename.ext │"
"║filename.ext │"
"║filename.ext │"

MHighlightExample2
"║ filename.ext│"
"║ filename.ext│"
"║ filename.ext│"

MHighlightContinueProcessing
"Продолжать &обработку"
"C&ontinue processing"
"Pokračovat ve zpracová&ní"

MInfoTitle
l:
"Информация"
"Information"
"Informace"

MInfoCompName
"Имя компьютера"
"Computer name"
"Název počítače"

MInfoUserName
"Имя пользователя"
"User name"
"Jméno uživatele"

MInfoRemovable
"Сменный"
"Removable"
"Vyměnitelný"

MInfoFixed
"Жесткий"
"Fixed"
"Pevný"

MInfoNetwork
"Сетевой"
"Network"
"Síťový"

MInfoCDROM
"CD-ROM"
"CD-ROM"
"CD-ROM"

MInfoCD_RW
"CD-RW"
"CD-RW"
"CD-RW"

MInfoCD_RWDVD
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD"

MInfoDVD_ROM
"DVD-ROM"
"DVD-ROM"
"DVD-ROM"

MInfoDVD_RW
"DVD-RW"
"DVD-RW"
"DVD-RW"

MInfoDVD_RAM
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"

MInfoRAM
"RAM"
"RAM"
"RAM"

MInfoSUBST
"SUBST"
"Subst"
"SUBST"

MInfoDisk
"диск"
"disk"
"disk"

MInfoDiskTotal
"Всего байтов"
"Total bytes"
"Celkem bytů"

MInfoDiskFree
"Свободных байтов"
"Free bytes"
"Volných bytů"

MInfoDiskLabel
"Метка тома"
"Volume label"
"Popisek disku"

MInfoDiskNumber
"Серийный номер"
"Serial number"
"Sériové číslo"

MInfoMemory
" Память "
" Memory "
" Paměť "

MInfoMemoryLoad
"Загрузка памяти"
"Memory load"
"Zatížení paměti"

MInfoMemoryTotal
"Всего памяти"
"Total memory"
"Celková paměť"

MInfoMemoryFree
"Свободно памяти"
"Free memory"
"Volná paměť"

MInfoVirtualTotal
"Всего вирт. памяти"
"Total virtual"
"Celkem virtuální"

MInfoVirtualFree
"Свободно вирт. памяти"
"Free virtual"
"Volná virtuální"

MInfoDizAbsent
"Файл описания папки отсутствует"
"Folder description file is absent"
"Soubor s popisem adresáře chybí"

MErrorInvalidFunction
l:
"Некорректная функция"
"Incorrect function"
"Nesprávná funkce"

MErrorBadCommand
"Команда не распознана"
"Command not recognized"
"Příkaz nebyl rozpoznán"

MErrorFileNotFound
"Файл не найден"
"File not found"
"Soubor nenalezen"

MErrorPathNotFound
"Путь не найден"
"Path not found"
"Cesta nenalezena"

MErrorTooManyOpenFiles
"Слишком много открытых файлов"
"Too many open files"
"Příliš mnoho otevřených souborů"

MErrorAccessDenied
"Доступ запрещен"
"Access denied"
"Přístup odepřen"

MErrorNotEnoughMemory
"Недостаточно памяти"
"Not enough memory"
"Nedostatek paměti"

MErrorDiskRO
"Попытка записи на защищенный от записи диск"
"Cannot write to write protected disk"
"Nelze zapisovat na disk chráněný proti zápisu"

MErrorDeviceNotReady
"Устройство не готово"
"The device is not ready"
"Zařízení není připraveno"

MErrorCannotAccessDisk
"Доступ к диску невозможен"
"Disk cannot be accessed"
"Na disk nelze přistoupit"

MErrorSectorNotFound
"Сектор не найден"
"Sector not found"
"Sektor nenalezen"

MErrorOutOfPaper
"В принтере нет бумаги"
"The printer is out of paper"
"V tiskárně došel papír"

MErrorWrite
"Ошибка записи"
"Write fault error"
"Chyba zápisu"

MErrorRead
"Ошибка чтения"
"Read fault error"
"Chyba čtení"

MErrorDeviceGeneral
"Общая ошибка устройства"
"Device general failure"
"Obecná chyba zařízení"

MErrorFileSharing
"Нарушение совместного доступа к файлу"
"File sharing violation"
"Narušeno sdílení souborů"

MErrorNetworkPathNotFound
"Сетевой путь не найден"
"The network path was not found"
"Síťová cesta nebyla nalezena"

MErrorNetworkBusy
"Сеть занята"
"The network is busy"
"Síť je zaneprázdněna"

MErrorNetworkAccessDenied
"Сетевой доступ запрещен"
"Network access is denied"
"Přístup na síť zakázán"

MErrorNetworkWrite
"Ошибка записи в сети"
"A write fault occurred on the network"
"Na síti došlo k chybě v zápisu"

MErrorDiskLocked
"Диск используется или заблокирован другим процессом"
"The disk is in use or locked by another process"
"Disk je používán nebo uzamčen jiným procesem"

MErrorFileExists
"Файл или папка уже существует"
"File or folder already exists"
"Soubor nebo adresář již existuje"

MErrorInvalidName
"Указанное имя неверно"
"The specified name is invalid"
"Zadaný název je neplatný"

MErrorInsufficientDiskSpace
"Нет места на диске"
"Insufficient disk space"
"Nedostatek místa na disku"

MErrorFolderNotEmpty
"Папка не пустая"
"The folder is not empty"
"Adresář není prázdný"

MErrorIncorrectUserName
"Неверное имя пользователя"
"Incorrect user name"
"Neplatné jméno uživatele"

MErrorIncorrectPassword
"Неверный пароль"
"Incorrect password"
"Neplatné heslo"

MErrorLoginFailure
"Ошибка регистрации"
"Login failure"
"Přihlášení selhalo"

MErrorConnectionAborted
"Соединение разорвано"
"Connection aborted"
"Spojení přerušeno"

MErrorCancelled
"Операция отменена"
"Operation cancelled"
"Operace stornována"

MErrorNetAbsent
"Сеть отсутствует"
"No network present"
"Síť není k dispozici"

MErrorNetDeviceInUse
"Устройство используется и не может быть отсоединено"
"Device is in use and cannot be disconnected"
"Zařízení se používá a nemůže být odpojeno"

MErrorNetOpenFiles
"На сетевом диске есть открытые файлы"
"This network connection has open files"
"Přes toto síťové spojení jsou otevřeny soubory"

MErrorAlreadyAssigned
"Имя локального устройства уже использовано"
"The local device name is already in use"
"Název lokálního zařízení je již používán"

MErrorAlreadyRemebered
"Имя локального устройства уже находится в профиле пользователя"
"The local device is already in the user profile"
"Lokální zařízení je již v uživatelově profilu"

MErrorNotLoggedOn
"Пользователь не зарегистрирован в сети"
"User has not logged on to the network"
"Uživatel nebyl do sítě přihlášen"

MErrorInvalidPassword
"Неверный пароль пользователя"
"The user password is invalid"
"Uživatelovo heslo není správné"

MErrorNoRecoveryPolicy
"Для этой системы отсутствует политика надежного восстановления шифрования"
"There is no valid encryption recovery policy configured for this system"
"V tomto systému není nastaveno žádné platné pravidlo pro dešifrování"

MErrorEncryptionFailed
"Ошибка при попытке шифрования файла"
"The specified file could not be encrypted"
"Zadaný soubor nemohl být zašifrován"

MErrorDecryptionFailed
"Ошибка при попытке расшифровки файла"
"The specified file could not be decrypted"
"Zadaný soubor nemohl být dešifrován"

MErrorFileNotEncrypted
"Указанный файл не зашифрован"
"The specified file is not encrypted"
"Zadaný soubor není zašifrován"

MErrorNoAssociation
"Указанному файлу не сопоставлено ни одно приложение для выполнения данной операции"
"No application is associated with the specified file for this operation"
"K zadanému souboru není asociována žádná aplikace pro tuto operaci"

MErrorFullPathNameLong
l:
"Полный путь к файлу имеет слишком большую длину"
"The full pathname is too long"
"Plná cesta k souboru je příliš dlouhá"

MCannotExecute
l:
"Ошибка выполнения"
"Cannot execute"
"Nelze provést"

MScanningFolder
"Просмотр папки"
"Scanning the folder"
"Prohledávám adresář"

MMakeFolderTitle
l:
"Создание папки"
"Make folder"
"Vytvoření adresáře"

MCreateFolder
"Создать п&апку"
"Create the &folder"
"Vytvořit &adresář"

MMultiMakeDir
"Обрабатыват&ь несколько имен папок"
"Process &multiple names"
"Zpracovat &více názvů"

MIncorrectDirList
"Неправильный список папок"
"Incorrect folders list"
"Neplatný seznam adresářů"

MCannotCreateFolder
"Ошибка создания папки"
"Cannot create the folder"
"Adresář nelze vytvořit"

MMenuBriefView
l:
"&Краткий                  LCtrl-1"
"&Brief              LCtrl-1"
"&Stručný                  LCtrl-1"

MMenuMediumView
"&Средний                  LCtrl-2"
"&Medium             LCtrl-2"
"S&třední                  LCtrl-2"

MMenuFullView
"&Полный                   LCtrl-3"
"&Full               LCtrl-3"
"&Plný                     LCtrl-3"

MMenuWideView
"&Широкий                  LCtrl-4"
"&Wide               LCtrl-4"
"Š&iroký                   LCtrl-4"

MMenuDetailedView
"&Детальный                LCtrl-5"
"Detai&led           LCtrl-5"
"Detai&lní                 LCtrl-5"

MMenuDizView
"&Описания                 LCtrl-6"
"&Descriptions       LCtrl-6"
"P&opisky                  LCtrl-6"

MMenuLongDizView
"Д&линные описания         LCtrl-7"
"Lon&g descriptions  LCtrl-7"
"&Dlouhé popisky           LCtrl-7"

MMenuOwnersView
"Вл&адельцы файлов         LCtrl-8"
"File own&ers        LCtrl-8"
"Vlastník so&uboru         LCtrl-8"

MMenuLinksView
"Свя&зи файлов             LCtrl-9"
"File lin&ks         LCtrl-9"
"Souborové lin&ky          LCtrl-9"

MMenuAlternativeView
"Аль&тернативный полный    LCtrl-0"
"&Alternative full   LCtrl-0"
"&Alternativní plný        LCtrl-0"

MMenuInfoPanel
l:
"Панель ин&формации        Ctrl-L"
"&Info panel         Ctrl-L"
"Panel In&fo               Ctrl-L"

MMenuTreePanel
"Де&рево папок             Ctrl-T"
"&Tree panel         Ctrl-T"
"Panel St&rom              Ctrl-T"

MMenuQuickView
"Быстры&й просмотр         Ctrl-Q"
"Quick &view         Ctrl-Q"
"Z&běžné zobrazení         Ctrl-Q"

MMenuSortModes
"Режим&ы сортировки        Ctrl-F12"
"&Sort modes         Ctrl-F12"
"Módy řaze&ní              Ctrl-F12"

MMenuLongNames
"Показывать длинные &имена Ctrl-N"
"Show long &names    Ctrl-N"
"Zobrazit dlouhé názv&y    Ctrl-N"

MMenuTogglePanel
"Панель &Вкл/Выкл          Ctrl-F1"
"Panel &On/Off       Ctrl-F1"
"Panel &Zap/Vyp            Ctrl-F1"

MMenuReread
"П&еречитать               Ctrl-R"
"&Re-read            Ctrl-R"
"Obno&vit                  Ctrl-R"

MMenuChangeDrive
"С&менить диск             Alt-F1"
"&Change drive       Alt-F1"
"Z&měnit jednotku          Alt-F1"

MMenuView
l:
"&Просмотр              F3"
"&View               F3"
"&Zobrazit                   F3"

MMenuEdit
"&Редактирование        F4"
"&Edit               F4"
"&Editovat                   F4"

MMenuCopy
"&Копирование           F5"
"&Copy               F5"
"&Kopírovat                  F5"

MMenuMove
"П&еренос               F6"
"&Rename or move     F6"
"&Přejmenovat/Přesunout      F6"

MMenuCreateFolder
"&Создание папки        F7"
"&Make folder        F7"
"&Vytvořit adresář           F7"

MMenuDelete
"&Удаление              F8"
"&Delete             F8"
"&Smazat                     F8"

MMenuWipe
"Уни&чтожение           Alt-Del"
"&Wipe               Alt-Del"
"&Vymazat                    Alt-Del"

MMenuAdd
"&Архивировать          Shift-F1"
"Add &to archive     Shift-F1"
"Přidat do &archívu          Shift-F1"

MMenuExtract
"Распако&вать           Shift-F2"
"E&xtract files      Shift-F2"
"&Rozbalit soubory           Shift-F2"

MMenuArchiveCommands
"Архивн&ые команды      Shift-F3"
"Arc&hive commands   Shift-F3"
"Příkazy arc&hívu            Shift-F3"

MMenuAttributes
"А&трибуты файлов       Ctrl-A"
"File &attributes    Ctrl-A"
"A&tributy souboru           Ctrl-A"

MMenuApplyCommand
"Применить коман&ду     Ctrl-G"
"A&pply command      Ctrl-G"
"Ap&likovat příkaz           Ctrl-G"

MMenuDescribe
"&Описание файлов       Ctrl-Z"
"Descri&be files     Ctrl-Z"
"Přidat popisek sou&borům    Ctrl-Z"

MMenuSelectGroup
"Пометить &группу       Gray +"
"Select &group       Gray +"
"Oz&načit skupinu            Num +"

MMenuUnselectGroup
"С&нять пометку         Gray -"
"U&nselect group     Gray -"
"O&dznačit skupinu           Num -"

MMenuInvertSelection
"&Инверсия пометки      Gray *"
"&Invert selection   Gray *"
"&Invertovat výběr           Num *"

MMenuRestoreSelection
"Восстановить по&метку  Ctrl-M"
"Re&store selection  Ctrl-M"
"&Obnovit výběr              Ctrl-M"

MMenuFindFile
l:
"&Поиск файла              Alt-F7"
"&Find file           Alt-F7"
"H&ledat soubor                  Alt-F7"

MMenuHistory
"&История команд           Alt-F8"
"&History             Alt-F8"
"&Historie                       Alt-F8"

MMenuVideoMode
"Видео&режим               Alt-F9"
"&Video mode          Alt-F9"
"&Video mód                      Alt-F9"

MMenuFindFolder
"Поис&к папки              Alt-F10"
"Fi&nd folder         Alt-F10"
"Hl&edat adresář                 Alt-F10"

MMenuViewHistory
"Ис&тория просмотра        Alt-F11"
"File vie&w history   Alt-F11"
"Historie &zobrazení souborů     Alt-F11"

MMenuFoldersHistory
"Ист&ория папок            Alt-F12"
"F&olders history     Alt-F12"
"Historie &adresářů              Alt-F12"

MMenuSwapPanels
"По&менять панели          Ctrl-U"
"&Swap panels         Ctrl-U"
"Prohodit panel&y                Ctrl-U"

MMenuTogglePanels
"Панели &Вкл/Выкл          Ctrl-O"
"&Panels On/Off       Ctrl-O"
"&Panely Zap/Vyp                 Ctrl-O"

MMenuCompareFolders
"&Сравнение папок"
"&Compare folders"
"Po&rovnat adresáře"

MMenuUserMenu
"Меню пользовател&я"
"Edit user &menu"
"Upravit uživatelské &menu"

MMenuFileAssociations
"&Ассоциации файлов"
"File &associations"
"Asocia&ce souborů"

MMenuFolderShortcuts
"Ссы&лки на папки"
"Fol&der shortcuts"
"A&dresářové zkratky"

MMenuFilter
"&Фильтр панели файлов     Ctrl-I"
"File panel f&ilter   Ctrl-I"
"F&iltr panelu souborů           Ctrl-I"

MMenuPluginCommands
"Команды внешних мо&дулей  F11"
"Pl&ugin commands     F11"
"Příkazy plu&ginů                F11"

MMenuWindowsList
"Список экра&нов           F12"
"Sc&reens list        F12"
"Seznam obrazove&k               F12"

MMenuProcessList
"Список &задач             Ctrl-W"
"Task &list           Ctrl-W"
"Seznam úl&oh                    Ctrl-W"

MMenuHotPlugList
"Список Hotplug-&устройств"
"Ho&tplug devices list"
"Seznam v&yjímatelných zařízení"

MMenuSystemSettings
l:
"Систе&мные параметры"
"S&ystem settings"
"Nastavení S&ystému"

MMenuPanelSettings
"Настройки па&нели"
"&Panel settings"
"Nastavení &Panelů"

MMenuInterface
"Настройки &интерфейса"
"&Interface settings"
"Nastavení Ro&zhraní"

MMenuDialogSettings
"Настройки &диалогов"
"Di&alog settings"
"Nastavení Dialo&gů"

MMenuLanguages
"&Языки"
"Lan&guages"
"Nastavení &Jazyka"

MMenuPluginsConfig
"Параметры &внешних модулей"
"Pl&ugins configuration"
"Nastavení Plu&ginů"

MMenuConfirmation
"&Подтверждения"
"Co&nfirmations"
"P&otvrzení"

MMenuFilePanelModes
"Режим&ы панели файлов"
"File panel &modes"
"&Módy souborových panelů"

MMenuFileDescriptions
"&Описания файлов"
"File &descriptions"
"Popi&sy souborů"

MMenuFolderInfoFiles
"Файлы описания п&апок"
"&Folder description files"
"Soubory popisů &adresářů"

MMenuViewer
"Настройки про&граммы просмотра"
"&Viewer settings"
"Nastavení P&rohlížeče"

MMenuEditor
"Настройки &редактора"
"&Editor settings"
"Nastavení &Editoru"

MMenuColors
"&Цвета"
"Co&lors"
"&Barvy"

MMenuFilesHighlighting
"Раскраска &файлов и группы сортировки"
"Files &highlighting and sort groups"
"Z&výrazňování souborů a skupiny řazení"

MMenuSaveSetup
"&Сохранить параметры      Shift-F9"
"&Save setup        Shift-F9"
"&Uložit nastavení                         Shift-F9"

MMenuTogglePanelRight
"Панель &Вкл/Выкл          Ctrl-F2"
"Panel &On/Off       Ctrl-F2"
"Panel &Zap/Vyp            Ctrl-F2"

MMenuChangeDriveRight
"С&менить диск             Alt-F2"
"&Change drive       Alt-F2"
"Z&měnit jednotku          Alt-F2"

MMenuLeftTitle
l:
"&Левая"
"&Left"
"&Levý"

MMenuFilesTitle
"&Файлы"
"&Files"
"&Soubory"

MMenuCommandsTitle
"&Команды"
"&Commands"
"Pří&kazy"

MMenuOptionsTitle
"Па&раметры"
"&Options"
"&Nastavení"

MMenuRightTitle
"&Правая"
"&Right"
"&Pravý"

MMenuSortTitle
l:
"Критерий сортировки"
"Sort by"
"Seřadit podle"

MMenuSortByName
"&Имя                              Ctrl-F3"
"&Name                 Ctrl-F3"
"&Názvu                     Ctrl-F3"

MMenuSortByExt
"&Расширение                       Ctrl-F4"
"E&xtension            Ctrl-F4"
"&Přípony                   Ctrl-F4"

MMenuSortByModification
"Время &модификации                Ctrl-F5"
"&Modification time    Ctrl-F5"
"Č&asu modifikace           Ctrl-F5"

MMenuSortBySize
"Р&азмер                           Ctrl-F6"
"&Size                 Ctrl-F6"
"&Velikosti                 Ctrl-F6"

MMenuUnsorted
"&Не сортировать                   Ctrl-F7"
"&Unsorted             Ctrl-F7"
"N&eřadit                   Ctrl-F7"

MMenuSortByCreation
"Время &создания                   Ctrl-F8"
"&Creation time        Ctrl-F8"
"&Data vytvoření            Ctrl-F8"

MMenuSortByAccess
"Время &доступа                    Ctrl-F9"
"&Access time          Ctrl-F9"
"Ča&su přístupu             Ctrl-F9"

MMenuSortByDiz
"&Описания                         Ctrl-F10"
"&Descriptions         Ctrl-F10"
"P&opisků                   Ctrl-F10"

MMenuSortByOwner
"&Владельцы файлов                 Ctrl-F11"
"&Owner                Ctrl-F11"
"V&lastníka                 Ctrl-F11"

MMenuSortByCompressedSize
"&Упакованный размер"
"Com&pressed size"
"&Komprimované velikosti"

MMenuSortByNumLinks
"Ко&личество ссылок"
"Number of &hard links"
"Poč&tu pevných linků"

MMenuSortUseGroups
"Использовать &группы сортировки   Shift-F11"
"Use sort &groups      Shift-F11"
"Řazení podle skup&in       Shift-F11"

MMenuSortSelectedFirst
"&Помеченные файлы вперед          Shift-F12"
"Show selected &first  Shift-F12"
"Nejdřív zobrazit vy&brané  Shift-F12"

MMenuSortUseNumeric
"Использовать &числовую сортировку"
"Use num&eric sort"
"Použít čí&selné řazení"

MChangeDriveTitle
l:
"Диск"
"Drive"
"Jednotka   "

MChangeDriveRemovable
"сменный  "
"removable"
"vyměnitelná"

MChangeDriveFixed
"жесткий  "
"fixed    "
"pevná      "

MChangeDriveNetwork
"сетевой  "
"network  "
"síťová     "

MChangeDriveCDROM
"CD-ROM   "
"CD-ROM   "
"CD-ROM     "

MChangeDriveCD_RW
"CD-RW    "
"CD-RW    "
"CD-RW      "

MChangeDriveCD_RWDVD
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD  "

MChangeDriveDVD_ROM
"DVD-ROM  "
"DVD-ROM  "
"DVD-ROM    "

MChangeDriveDVD_RW
"DVD-RW   "
"DVD-RW   "
"DWD-RW     "

MChangeDriveDVD_RAM
"DVD-RAM  "
"DVD-RAM  "
"DVD-RAM    "

MChangeDriveRAM
"RAM диск "
"RAM disk "
"RAM disk   "

MChangeDriveSUBST
"SUBST    "
"subst    "
"SUBST      "

MChangeDriveLabelAbsent
"недоступен"
"not available"
"není k dispozici"

MChangeDriveCannotReadDisk
"Ошибка чтения диска в дисководе %c:"
"Cannot read the disk in drive %c:"
"Nelze přečíst disk v jednotce %c:"

MChangeDriveCannotDisconnect
"Не удается отсоединиться от %s"
"Cannot disconnect from %s"
"Nelze se odpojit od %s"

MChangeDriveCannotDelSubst
"Не удается удалить виртуальный драйвер %s"
"Cannot delete a substituted drive %s"
"Nelze smazat substnutá jednotka %s"

MChangeDriveOpenFiles
"Если вы не закроете открытые файлы, данные могут быть утеряны"
"If you do not close the open files, data may be lost."
"Pokud neuzavřete otevřené soubory, mohou být tato data ztracena."

MChangeSUBSTDisconnectDriveTitle
l:
"Отключение виртуального драйвера"
"Virtual device disconnection"
"Odpojování virtuálního zařízení"

MChangeSUBSTDisconnectDriveQuestion
"Отключить SUBST-диск %c:?"
"Disconnect SUBST-disk %c:?"
"Odpojit SUBST-disk %c:?"

MChangeHotPlugDisconnectDriveTitle
l:
"Удаление устройства"
"Device Removal"
"Odpojování zařízení"

MChangeHotPlugDisconnectDriveQuestion
"Вы хотите удалить устройство"
"Do you want to remove the device"
"Opravdu si přejete odpojit zařízení"

MHotPlugDisks
"(диск(и): %s)"
"(disk(s): %s)"
"(disk(y): %s)"

MChangeCouldNotEjectHotPlugMedia
"Невозможно удалить устройство для диска %c:"
"Cannot remove a device for drive %c:"
"Zařízení %c: nemůže být odpojeno."

MChangeCouldNotEjectHotPlugMedia2
"Невозможно удалить устройство:"
"Cannot remove a device:"
"Zařízení nemůže být odpojeno."

MChangeHotPlugNotify1
"Теперь устройство" 
"The device" 
"Zařízení"

MChangeHotPlugNotify2
"может быть безопасно извлечено из компьютера"
"can now be safely removed"
"může být nyní bezpečně odebráno"

MHotPlugListTitle
"Hotplug-устройства"
"Hotplug devices list"
"Seznam vyjímatelných zařízení"

MHotPlugListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Edit: Del,Ctrl-R"

MChangeDriveDisconnectTitle
l:
"Отключение сетевого устройства"
"Disconnect network drive"
"Odpojit síťovou jednotku"

MChangeDriveDisconnectQuestion
"Вы хотите удалить соединение с устройством %c:?"
"Do you want to disconnect from the drive %c:?"
"Opravdu si přejete odpojit od jednotky %c:?"

MChangeDriveDisconnectMapped
"На устройство %c: отображена папка"
"The drive %c: is mapped to..."
"Jednotka %c: je namapována na..."

MChangeDriveDisconnectReconnect
"&Восстанавливать при входе в систему"
"&Reconnect at Logon"
"&Znovu připojit při přihlášení"

MChangeDriveAskDisconnect
l:
"Вы хотите в любом случае отключиться от устройства?"
"Do you want to disconnect the device anyway?"
"Přejete si přesto zařízení odpojit?"

MChangeVolumeInUse
"Не удается извлечь диск из привода %c:"
"Volume %c: cannot be ejected."
"Jednotka %c: nemůže být vysunuta."

MChangeVolumeInUse2
"Используется другим приложением"
"It is used by another application"
"Je používaná jinou aplikací"

MChangeWaitingLoadDisk
ls:;"Ожидание загрузки диска..."
"Ожидание чтения диска..."
ls:;"Waiting for disk to load..."
"Waiting for disk to mount..."
ls:;"Nahrávání disku..."
"Čekám na disk k připojení..."

MChangeCouldNotEjectMedia
"Невозможно извлечь диск из привода %c:"
"Could not eject media from drive %c:"
"Nelze vysunout médium v jednotce %c:"

MAdditionalHotKey
"#!$%*+-/(),."
"#!$%*+-/(),."
"#!$%*+-/(),."

MSearchFileTitle
l:
" Поиск "
" Search "
" Hledat "

MCannotCreateListFile
"Ошибка создания списка файлов"
"Cannot create list file"
"Nelze vytvořit soubor se seznamem"

MCannotCreateListTemp
"(невозможно создать временный файл для списка)"
"(cannot create temporary file for list)"
"(nemohu vytvořit dočasný soubor pro seznam)"

MCannotCreateListWrite
"(невозможно записать данные в файл)"
"(cannot write data in file)"
"(nemohu zapsat data do souboru)"

MDragFiles
l:
"%d файлов"
"%d files"
"%d souborů"

MDragMove
"Перенос %s"
"Move %s"
"Přesunout %s"

MDragCopy
"Копирование %s"
"Copy %s"
"Kopírovat %s"

MProcessListTitle
l:
"Список задач"
"Task list"
"Seznam úloh"

MProcessListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Edit: Del,Ctrl-R"

MKillProcessTitle
"Удаление задачи"
"Kill task"
"Zabít úlohu"

MAskKillProcess
"Вы хотите удалить выбранную задачу?"
"Do you wish to kill selected task?"
"Přejete si zabít vybranou úlohu?"

MKillProcessWarning
"Вы потеряете всю несохраненную информацию этой программы"
"You will lose any unsaved information in this program"
"V tomto programu budou ztraceny neuložené informace"

MKillProcessKill
"Удалить"
"Kill"
"Zabít"

MCannotKillProcess
"Указанную задачу удалить не удалось"
"Cannot kill the specified task"
"Nemohu ukončit zvolenou úlohu"

MCannotKillProcessPerm
"Вы не имеет права удалить этот процесс."
"You have no permission to kill this process."
"Nemáte oprávnění zabít tento proces."

MQuickViewTitle
l:
"Быстрый просмотр"
"Quick view"
"Zběžné zobrazení"

MQuickViewFolder
"Папка \"%s\""
"Folder \"%s\""
"Adresář \"%s\""

MQuickViewJunction
"Связь \"%s\""
"Junction \"%s\""
"Křížení \"%s\""

MQuickViewSymlink
"Ссылка \"%s\""
"Symlink \"%s\""
"Symbolický link \"%s\""

MQuickViewVolMount
"Том \"%s\""
"Volume \"%s\""
"Svazek \"%s\""

MQuickViewContains
"Содержит:"
"Contains:"
"Obsah:"

MQuickViewFolders
"Папок               "
"Folders          "
"Adresáře           "

MQuickViewFiles
"Файлов              "
"Files            "
"Soubory            "

MQuickViewBytes
"Размер файлов       "
"Files size       "
"Velikost souborů   "

MQuickViewCompressed
"Упакованный размер  "
"Compressed size  "
"Komprim. velikost  "

MQuickViewRatio
"Степень сжатия      "
"Ratio            "
"Poměr              "

MQuickViewCluster
"Размер кластера     "
"Cluster size     "
"Velikost clusteru  "

MQuickViewRealSize
"Реальный размер     "
"Real files size  "
"Opravdová velikost "

MQuickViewSlack
"Остатки кластеров   "
"Files slack      "
"Mrtvé místo        "

MSetAttrTitle
l:
"Атрибуты"
"Attributes"
"Atributy"

MSetAttrFor
"Изменить файловые атрибуты"
"Change file attributes for"
"Změna atributů souboru pro"

MSetAttrSelectedObjects
"выбранных объектов"
"selected objects"
"vybrané objekty"

MSetAttrHardLinks
"жестких ссылок (%d)"
"hard links (%d)"
"pevné linky (%d)"

MSetAttrJunction
"Связь \"%s\""
"Junction \"%s\""
"Křížení \"%s\""

MSetAttrSymlink
"Ссылка \"%s\""
"Symlink \"%s\""
"Link \"%s\""

MSetAttrVolMount
"Том \"%s\""
"Volume \"%s\""
"Svazek \"%s\""

MSetAttrUnknownJunction
"(нет данных)"
"(data not available)"
"(data nejsou k dispozici)"

MSetAttrRO
"&Только для чтения"
"&Read only"
"&Pouze pro čtení"

MSetAttrArchive
"&Архивный"
"&Archive"
"&Archivovat"

MSetAttrHidden
"&Скрытый"
"&Hidden"
"&Skrytý"

MSetAttrSystem
"С&истемный"
"&System"
"S&ystémový"

MSetAttrCompressed
"Сжаты&й"
"&Compressed"
"&Komprimovaný"

MSetAttrEncrypted
"За&шифрованный"
"&Encrypted"
"&Šifrovaný"

MSetAttrNotIndexed
"Н&еиндексируемый"
"Not &Indexed"
"Neinde&xovaný"

MSetAttrSparse
"Разреженный"
"Sparse"
"Rozptýlený"

MSetAttrTemp
"Временный"
"Temporary"
"Dočasný"

MSetAttrOffline
"Автономный"
"Offline"
"Offline"

MSetAttrVirtual
"Виртуальный"
"Virtual"
"Virtuální"

MSetAttrSubfolders
"Обрабатывать &вложенные папки"
"Process sub&folders"
"Zpracovat i po&dadresáře"

MSetAttrModification
"Время &модификации файла:"
"File &modification time:"
"Čas &modifikace souboru:"

MSetAttrCreation
"Время со&здания файла:"
"File crea&tion time:"
"Čas v&ytvoření souboru:"

MSetAttrLastAccess
"Время последнего &доступа к файлу:"
"&Last file access time:"
"Čas posledního pří&stupu:"

MSetAttrOriginal
"Исход&ное"
"&Original"
"&Originál"

MSetAttrCurrent
"Те&кущее"
"Curre&nt"
"So&učasný"

MSetAttrBlank
"Сбр&ос"
"&Blank"
"P&rázdný"

MSetAttrSet
"Установить"
"Set"
"Nastavit"

MSetAttrTimeTitle1
l:
"ММ%cДД%cГГГГ чч%cмм%cсс"
"MM%cDD%cYYYY hh%cmm%css"
"MM%cDD%cRRRR hh%cmm%css"

MSetAttrTimeTitle2
"ДД%cММ%cГГГГ чч%cмм%cсс"
"DD%cMM%cYYYY hh%cmm%css"
"DD%cMM%cRRRR hh%cmm%css"

MSetAttrTimeTitle3
"ГГГГ%cММ%cДД чч%cмм%cсс"
"YYYY%cMM%cDD hh%cmm%css"
"RRRR%cMM%cDD hh%cmm%css"

MSetAttrSetting
l:
"Установка файловых атрибутов для"
"Setting file attributes for"
"Nastavení atributů souboru pro"

MSetAttrCannotFor
"Ошибка установки атрибутов для"
"Cannot set attributes for"
"Nelze nastavit atributy pro"

MSetAttrCompressedCannotFor
"Не удалось установить атрибут СЖАТЫЙ для"
"Cannot set attribute COMPRESSED for"
"Nelze nastavit atribut KOMPRIMOVANÝ pro"

MSetAttrEncryptedCannotFor
"Не удалось установить атрибут ЗАШИФРОВАННЫЙ для"
"Cannot set attribute ENCRYPTED for"
"Nelze nastavit atribut ŠIFROVANÝ pro"

MSetAttrTimeCannotFor
"Не удалось установить время файла для"
"Cannot set file time for"
"Nelze nastavit čas souboru pro"

MSetColorPanel
l:
"&Панель"
"&Panel"
"&Panel"

MSetColorDialog
"&Диалог"
"&Dialog"
"&Dialog"

MSetColorWarning
"Пр&едупреждение"
"&Warning message"
"&Varovná zpráva"

MSetColorMenu
"&Меню"
"&Menu"
"&Menu"

MSetColorHMenu
"&Горизонтальное меню"
"Hori&zontal menu"
"Hori&zontální menu"

MSetColorKeyBar
"&Линейка клавиш"
"&Key bar"
"&Řádek kláves"

MSetColorCommandLine
"&Командная строка"
"&Command line"
"Pří&kazový řádek"

MSetColorClock
"&Часы"
"C&lock"
"&Hodiny"

MSetColorViewer
"Про&смотрщик"
"&Viewer"
"P&rohlížeč"

MSetColorEditor
"&Редактор"
"&Editor"
"&Editor"

MSetColorHelp
"П&омощь"
"&Help"
"&Nápověda"

MSetDefaultColors
"&Установить стандартные цвета"
"Set de&fault colors"
"N&astavit výchozí barvy"

MSetBW
"Черно-бел&ый режим"
"&Black and white mode"
"Černo&bílý mód"

MSetColorPanelNormal
l:
"Обычный текст"
"Normal text"
"Normální text"

MSetColorPanelSelected
"Выбранный текст"
"Selected text"
"Vybraný text"

MSetColorPanelHighlightedInfo
"Выделенная информация"
"Highlighted info"
"Info zvýrazněné"

MSetColorPanelDragging
"Перетаскиваемый текст"
"Dragging text"
"Tažený text"

MSetColorPanelBox
"Рамка"
"Border"
"Okraj"

MSetColorPanelNormalCursor
"Обычный курсор"
"Normal cursor"
"Normální kurzor"

MSetColorPanelSelectedCursor
"Выделенный курсор"
"Selected cursor"
"Vybraný kurzor"

MSetColorPanelNormalTitle
"Обычный заголовок"
"Normal title"
"Normální nadpis"

MSetColorPanelSelectedTitle
"Выделенный заголовок"
"Selected title"
"Vybraný nadpis"

MSetColorPanelColumnTitle
"Заголовок колонки"
"Column title"
"Nadpis sloupce"

MSetColorPanelTotalInfo
"Количество файлов"
"Total info"
"Info celkové"

MSetColorPanelSelectedInfo
"Количество выбранных файлов"
"Selected info"
"Info výběr"

MSetColorPanelScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"

MSetColorPanelScreensNumber
"Количество фоновых экранов"
"Number of background screens"
"Počet obrazovek na pozadí"

MSetColorDialogNormal
l:
"Обычный текст"
"Normal text"
"Normální text"

MSetColorDialogHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"

MSetColorDialogDisabled
"Блокированный текст"
"Disabled text"
"Zakázaný text"

MSetColorDialogBox
"Рамка"
"Border"
"Okraj"

MSetColorDialogBoxTitle
"Заголовок рамки"
"Title"
"Nadpis"

MSetColorDialogHighlightedBoxTitle
"Выделенный заголовок рамки"
"Highlighted title"
"Zvýrazněný nadpis"

MSetColorDialogTextInput
"Ввод текста"
"Text input"
"Textový vstup"

MSetColorDialogUnchangedTextInput
"Неизмененный текст"
"Unchanged text input"
"Nezměněný textový vstup"

MSetColorDialogSelectedTextInput
"Ввод выделенного текста"
"Selected text input"
"Vybraný textový vstup"

MSetColorDialogEditDisabled
"Блокированное поле ввода"
"Disabled input line"
"Zakázaný vstupní řádek"

MSetColorDialogButtons
"Кнопки"
"Buttons"
"Tlačítka"

MSetColorDialogSelectedButtons
"Выбранные кнопки"
"Selected buttons"
"Vybraná tlačítka"

MSetColorDialogHighlightedButtons
"Выделенные кнопки"
"Highlighted buttons"
"Zvýrazněná tlačítka"

MSetColorDialogSelectedHighlightedButtons
"Выбранные выделенные кнопки"
"Selected highlighted buttons"
"Vybraná zvýrazněná tlačítka"

MSetColorDialogListBoxControl
"Список"
"List box"
"Seznam položek"

MSetColorDialogComboBoxControl
"Комбинированный список"
"Combobox"
"Výběr položek"

MSetColorDialogListText
l:
"Обычный текст"
"Normal text"
"Normální text"

MSetColorDialogListSelectedText
"Выбранный текст"
"Selected text"
"Vybraný text"

MSetColorDialogListHighLight
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"

MSetColorDialogListSelectedHighLight
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"

MSetColorDialogListDisabled
"Блокированный пункт"
"Disabled item"
"Naktivní položka"

MSetColorDialogListBox
"Рамка"
"Border"
"Okraj"

MSetColorDialogListTitle
"Заголовок"
"Title"
"Nadpis"

MSetColorDialogListScrollBar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"

MSetColorDialogListArrows
"Индикаторы длинных строк"
"Long string indicators"
"Značka dlouhého řetězce"

MSetColorDialogListArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"
"Vybraná značka dlouhého řetězce"

MSetColorDialogListArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"
"Zakázaná značka dlouhého řetězce"

MSetColorMenuNormal
l:
"Обычный текст"
"Normal text"
"Normální text"

MSetColorMenuSelected
"Выбранный текст"
"Selected text"
"Vybraný text"

MSetColorMenuHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"

MSetColorMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"

MSetColorMenuDisabled
"Недоступный пункт"
"Disabled text"
"Neaktivní text"

MSetColorMenuBox
"Рамка"
"Border"
"Okraj"

MSetColorMenuTitle
"Заголовок"
"Title"
"Nadpis"

MSetColorMenuScrollBar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"

MSetColorMenuArrows
"Индикаторы длинных строк"
"Long string indicators"
"Značka dlouhého řetězce"

MSetColorMenuArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"
"Vybraná značka dlouhého řetězce"

MSetColorMenuArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"
"Zakázaná značka dlouhého řetězce"

MSetColorHMenuNormal
l:
"Обычный текст"
"Normal text"
"Normální text"

MSetColorHMenuSelected
"Выбранный текст"
"Selected text"
"Vybraný text"

MSetColorHMenuHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"

MSetColorHMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"

MSetColorKeyBarNumbers
l:
"Номера клавиш"
"Key numbers"
"Čísla kláves"

MSetColorKeyBarNames
"Названия клавиш"
"Key names"
"Názvy kláves"

MSetColorKeyBarBackground
"Фон"
"Background"
"Pozadí"

MSetColorCommandLineNormal
l:
"Обычный текст"
"Normal text"
"Normální text"

MSetColorCommandLineSelected
"Выделенный текст"
"Selected text input"
"Vybraný textový vstup"

MSetColorCommandLinePrefix
"Текст префикса"
"Prefix text"
"Text předpony"

MSetColorCommandLineUserScreen
"Пользовательский экран"
"User screen"
"Obrazovka uživatele"

MSetColorClockNormal
l:
"Обычный текст (панели)"
"Normal text (Panel)"
"Normální text (Panel)"

MSetColorClockNormalEditor
"Обычный текст (редактор)"
"Normal text (Editor)"
"Normální text (Editor)"

MSetColorClockNormalViewer
"Обычный текст (вьювер)"
"Normal text (Viewer)"
"Normální text (Prohlížeč)"

MSetColorViewerNormal
l:
"Обычный текст"
"Normal text"
"Normální text"

MSetColorViewerSelected
"Выбранный текст"
"Selected text"
"Vybraný text"

MSetColorViewerStatus
"Статус"
"Status line"
"Stavový řádek"

MSetColorViewerArrows
"Стрелки сдвига экрана"
"Screen scrolling arrows"
"Skrolovací šipky"

MSetColorViewerScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"

MSetColorEditorNormal
l:
"Обычный текст"
"Normal text"
"Normální text"

MSetColorEditorSelected
"Выбранный текст"
"Selected text"
"Vybraný text"

MSetColorEditorStatus
"Статус"
"Status line"
"Stavový řádek"

MSetColorEditorScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"

MSetColorHelpNormal
l:
"Обычный текст"
"Normal text"
"Normální text"

MSetColorHelpHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"

MSetColorHelpReference
"Ссылка"
"Reference"
"Odkaz"

MSetColorHelpSelectedReference
"Выбранная ссылка"
"Selected reference"
"Vybraný odkaz"

MSetColorHelpBox
"Рамка"
"Border"
"Okraj"

MSetColorHelpBoxTitle
"Заголовок рамки"
"Title"
"Nadpis"

MSetColorHelpScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"

MSetColorGroupsTitle
l:
"Цветовые группы"
"Color groups"
"Skupiny barev"

MSetColorItemsTitle
"Элементы группы"
"Group items"
"Položky skupin"

MSetColorTitle
l:
"Цвет"
"Color"
"Barva"

MSetColorForeground
"&Текст"
"&Foreground"
"&Popředí"

MSetColorBackground
"&Фон"
"&Background"
"Po&zadí"

MSetColorForeTransparent
"&Прозрачный"
"&Transparent"
"Průhlednos&t"

MSetColorBackTransparent
"П&розрачный"
"T&ransparent"
"Průhledno&st"

MSetColorSample
"Текст Текст Текст Текст Текст Текст"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"

MSetColorSet
"Установить"
"Set"
"Nastavit"

MSetColorCancel
"Отменить"
"Cancel"
"Storno"

MSetConfirmTitle
l:
"Подтверждения"
"Confirmations"
"Potvrzení"

MSetConfirmCopy
"Перезапись файлов при &копировании"
"&Copy"
"&Kopírování"

MSetConfirmMove
"Перезапись файлов при &переносе"
"&Move"
"&Přesouvání"

MSetConfirmDrag
"Пере&таскивание"
"&Drag and drop"
"&Drag and drop"

MSetConfirmDelete
"&Удаление"
"De&lete"
"&Mazání"

MSetConfirmDeleteFolders
"У&даление непустых папок"
"Delete non-empty &folders"
"Mazat &neprázdné adresáře"

MSetConfirmEsc
"Прерыва&ние операций (Esc)"
"&Interrupt operation"
"Pře&rušit operaci"

MSetConfirmRemoveConnection
"&Отключение сетевого устройства"
"Disconnect &network drive"
"Odpojení &síťové jednotky"

MSetConfirmRemoveSUBST
"Отключение SUBST-диска"
"Disconnect &SUBST-disk"
"Odpojení SUBST-d&isku"

MSetConfirmRemoveHotPlug
"Отключение HotPlug-устройства"
"HotPlug-device removal"
"Odpojení vyjímatelného zařízení"

MSetConfirmAllowReedit
"Повто&рное открытие файла в редакторе"
"&Reload edited file"
"&Obnovit upravovaný soubor"

MSetConfirmHistoryClear
"Очистка списка &истории"
"Clear &history list"
"Vymazat seznam &historie"

MSetConfirmExit
"&Выход"
"E&xit"
"U&končení"

MFindFolderTitle
l:
"Поиск папки"
"Find folder"
"Najít adresář"

MKBFolderTreeF1
l:
l:// Find folder Tree KeyBar
"Помощь"
"Help"
"Nápověda"

MKBFolderTreeF2
"Обновить"
"Rescan"
"Obnovit"

MKBFolderTreeF5
"Размер"
"Zoom"
"Zoom"

MKBFolderTreeF10
"Выход"
"Quit"
"Konec"

MKBFolderTreeAltF9
"Видео"
"Video"
"Video"

MTreeTitle
"Дерево"
"Tree"
"Stromové zobrazení"

MCannotSaveTree
"Ошибка записи дерева папок в файл"
"Cannot save folders tree to file"
"Adresářový strom nelze uložit do souboru"

MReadingTree
"Чтение дерева папок"
"Reading the folders tree"
"Načítám adresářový strom"

MUserMenuTitle
l:
"Пользовательское меню"
"User menu"
"Menu uživatele"

MChooseMenuType
"Выберите тип пользовательского меню для редактирования"
"Choose user menu type to edit"
"Zvol typ menu uživatele pro úpravu"

MChooseMenuMain
"&Главное"
"&Main"
"&Hlavní"

MChooseMenuLocal
"&Местное"
"&Local"
"&Lokální"

MMainMenuTitle
"Главное меню"
"Main menu"
"Hlavní menu"

MMainMenuFAR
"Папка FAR"
"FAR folder"
"Složka FARu"

MMainMenuREG
l:
l:// <...menu (Registry)>
"Реестр"
"Registry"
"Registry"

MLocalMenuTitle
"Местное меню"
"Local menu"
"Lokalní menu"

MMainMenuBottomTitle
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"

MAskDeleteMenuItem
"Вы хотите удалить пункт меню"
"Do you wish to delete the menu item"
"Přejete si smazat položku v menu"

MAskDeleteSubMenuItem
"Вы хотите удалить вложенное меню"
"Do you wish to delete the submenu"
"Přejete si smazat podmenu"

MUserMenuInvalidInputLabel
"Неправильный формат метки меню!"
"Invalid format for UserMenu Label!"
"Neplatný formát pro název Uživatelského menu!"

MUserMenuInvalidInputHotKey
"Неправильный формат горячей клавиши!"
"Invalid format for Hot Key!"
"Neplatný formát pro klávesovou zkratku!"

MEditMenuTitle
l:
"Редактирование пользовательского меню"
"Edit user menu"
"Editace uživatelského menu"

MEditMenuHotKey
"&Горячая клавиша:"
"&Hot key:"
"K&lávesová zkratka:"

MEditMenuLabel
"&Метка:"
"&Label:"
"&Popisek:"

MEditMenuCommands
"&Команды:"
"&Commands:"
"Pří&kazy:"

MAskInsertMenuOrCommand
l:
"Вы хотите вставить новую команду или новое меню?"
"Do you wish to insert a new command or a new menu?"
"Přejete si vložit nový příkaz nebo nové menu?"

MMenuInsertCommand
"Вставить команду"
"Insert command"
"Vložit příkaz"

MMenuInsertMenu
"Вставить меню"
"Insert menu"
"Vložit menu"

MEditSubmenuTitle
l:
"Редактирование метки вложенного меню"
"Edit submenu label"
"Úprava popisku podmenu"

MEditSubmenuHotKey
"&Горячая клавиша:"
"&Hot key:"
"Klávesová &zkratka:"

MEditSubmenuLabel
"&Метка:"
"&Label:"
"&Popisek:"

MViewerTitle
l:
"Просмотр"
"Viewer"
"Prohlížeč"

MViewerCannotOpenFile
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"

MViewerStatusCol
"Кол"
"Col"
"Sloupec"

MViewSearchTitle
l:
"Поиск"
"Search"
"Hledat"

MViewSearchFor
"&Искать"
"&Search for"
"H&ledat"

MViewSearchForText
"Искать &текст"
"Search for &text"
"Hledat &text"

MViewSearchForHex
"Искать 16-ричный &код"
"Search for &hex"
"Hledat he&x"

MViewSearchCase
"&Учитывать регистр"
"&Case sensitive"
"&Rozlišovat velikost písmen"

MViewSearchWholeWords
"Только &целые слова"
"&Whole words"
"Celá &slova"

MViewSearchReverse
"Обратн&ый поиск"
"Re&verse search"
"&Zpětné hledání"

MViewSearchSearch
"Искать"
"Search"
"Hledat"

MViewSearchCancel
"Отменить"
"Cancel"
"Storno"

MViewSearchingFor
l:
"Поиск"
"Searching for"
"Vyhledávám"

MViewSearchingHex
"Поиск байтов"
"Searching for bytes"
"Vyhledávám sekvenci bytů"

MViewSearchCannotFind
"Строка не найдена"
"Could not find the string"
"Nelze najít řetězec"

MViewSearchCannotFindHex
"Байты не найдены"
"Could not find the bytes"
"Nelze najít sekvenci bytů"

MViewSearchFromBegin
"Продолжить поиск с начала документа?"
"Continue the search from the beginning of the document?"
"Pokračovat s hledáním od začátku dokumentu?"

MViewSearchFromEnd
"Продолжить поиск с конца документа?"
"Continue the search from the end of the document?"
"Pokračovat s hledáním od konce dokumentu?"

MPrintTitle
l:
"Печать"
"Print"
"Tisk"

MPrintTo
"Печатать %s на"
"Print %s to"
"Vytisknout %s na"

MPrintFilesTo
"Печатать %d файлов на"
"Print %d files to"
"Vytisknout %d souborů na"

MPreparingForPrinting
"Подготовка файлов к печати"
"Preparing files for printing"
"Připravuji soubory pro tisk"

MJobs
"заданий"
"jobs"
"úlohy"

MCannotOpenPrinter
"Не удалось открыть принтер"
"Cannot open printer"
"Nelze otevřít tiskárnu"

MCannotPrint
"Не удалось распечатать"
"Cannot print"
"Nelze tisknout"

MDescribeFiles
l:
"Описание файла"
"Describe file"
"Popiskový soubor"

MEnterDescription
"Введите описание %s"
"Enter %s description"
"Zadejte popisek %s"

MReadingDiz
l:
"Чтение описаний файлов"
"Reading file descriptions"
"Načítám popisky souboru"

MCannotUpdateDiz
"Не удалось обновить описания файлов"
"Cannot update file descriptions"
"Nelze aktualizovat popisky souboru"

MCannotUpdateRODiz
"Файл описаний защищен от записи"
"The description file is read only"
"Popiskový soubor má atribut Jen pro čtení"

MCfgDizTitle
l:
"Описания файлов"
"File descriptions"
"Popisky souboru"

MCfgDizListNames
"Имена &списков описаний, разделенные запятыми:"
"Description &list names delimited with commas:"
"Seznam pop&isových souborů oddělených čárkami:"

MCfgDizSetHidden
"Устанавливать &атрибут ""Hidden"" на новые списки описаний"
"Set ""&Hidden"" attribute to new description lists"
"Novým souborům s popisy nastavit atribut ""&Skrytý"""

MCfgDizROUpdate
"Обновлять файл описаний с атрибутом ""Толь&ко для чтения"""
"Update &read only description file"
"Aktualizovat popisové soubory s atributem Jen pro čtení"

MCfgDizStartPos
"&Позиция новых описаний в строке"
"&Position of new descriptions in the string"
"&Pozice nových popisů v řetězci"

MCfgDizNotUpdate
"&Не обновлять описания"
"Do &not update descriptions"
"&Neaktualizovat popisy"

MCfgDizUpdateIfDisplayed
"&Обновлять, если они выводятся на экран"
"Update if &displayed"
"Aktualizovat, jestliže je &zobrazen"

MCfgDizAlwaysUpdate
"&Всегда обновлять"
"&Always update"
"&Vždy aktualizovat"

MReadingTitleFiles
l:
"Обновление панелей"
"Update of panels"
"Aktualizace panelů"

MReadingFiles
"Чтение: %d файлов"
"Reading: %d files"
"Načítám: %d souborů"

MUserBreakTitle
l:
"Прекращено пользователем"
"User break"
"Přerušeno uživatelem"

MOperationNotCompleted
"Операция не завершена"
"Operation not completed"
"Operace není dokončena"

MEditPanelModes
l:
"Режимы панели"
"Edit panel modes"
"Editovat módy panelu"

MEditPanelModesBrief
l:
"&Краткий режим"
"&Brief mode"
"&Stručný mód"

MEditPanelModesMedium
"&Средний режим"
"&Medium mode"
"S&třední mód"

MEditPanelModesFull
"&Полный режим"
"&Full mode"
"&Plný mód"

MEditPanelModesWide
"&Широкий режим"
"&Wide mode"
"Š&iroký mód"

MEditPanelModesDetailed
"&Детальный режим"
"Detai&led mode"
"Detai&lní mód"

MEditPanelModesDiz
"&Описания"
"&Descriptions mode"
"P&opiskový mód"

MEditPanelModesLongDiz
"Д&линные описания"
"Lon&g descriptions mode"
"&Mód dlouhých popisků"

MEditPanelModesOwners
"Вл&адельцы файлов"
"File own&ers mode"
"Mód vlastníka so&uborů"

MEditPanelModesLinks
"Свя&зи файлов"
"Lin&ks mode"
"Lin&kový mód"

MEditPanelModesAlternative
"Аль&тернативный полный режим"
"&Alternative full mode"
"&Alternativní plný mód"

MEditPanelModeTypes
l:
"&Типы колонок"
"Column &types"
"&Typ sloupců"

MEditPanelModeWidths
"&Ширина колонок"
"Column &widths"
"Šíř&ka sloupců"

MEditPanelModeStatusTypes
"Типы колонок строки ст&атуса"
"St&atus line column types"
"T&yp sloupců stavového řádku"

MEditPanelModeStatusWidths
"Ширина колонок строки стат&уса"
"Status l&ine column widths"
"Šířka slo&upců stavového řádku"

MEditPanelModeFullscreen
"&Полноэкранный режим"
"&Fullscreen view"
"&Celoobrazovkový režim"

MEditPanelModeAlignExtensions
"&Выравнивать расширения файлов"
"Align file &extensions"
"Zarovnat příp&ony souborů"

MEditPanelModeAlignFolderExtensions
"Выравнивать расширения пап&ок"
"Align folder e&xtensions"
"Zarovnat přípony adre&sářů"

MEditPanelModeFoldersUpperCase
"Показывать папки &заглавными буквами"
"Show folders in &uppercase"
"Zobrazit adresáře &velkými písmeny"

MEditPanelModeFilesLowerCase
"Показывать файлы ст&рочными буквами"
"Show files in &lowercase"
"Zobrazit soubory ma&lými písmeny"

MEditPanelModeUpperToLowerCase
"Показывать имена файлов из заглавных букв &строчными буквами"
"Show uppercase file names in lower&case"
"Zobrazit velké znaky ve jménech souborů jako &malá písmena"

MEditPanelModeCaseSensitiveSort
"Использовать р&егистрозависимую сортировку"
"Use case &sensitive sort"
"Použít řazení citlivé na velikost pí&smen"

MEditPanelReadHelp
" Нажмите F1, чтобы получить информацию по настройке "
" Read online help for instructions "
" Pro instrukce si přečtěte online nápovědu "

MSetFolderInfoTitle
l:
"Файлы информации о папках"
"Folder description files"
"Soubory s popiskem adresáře"

MSetFolderInfoNames
"Введите имена файлов, разделенные запятыми (допускаются маски)"
"Enter file names delimited with commas (wildcards are allowed)"
"Zadejte jména souborů oddělených čárkami (značky jsou povoleny)"

MScreensTitle
l:
"Экраны"
"Screens"
"Obrazovky"

MScreensPanels
"Панели"
"Panels"
"Panely"

MScreensView
"Просмотр"
"View"
"Zobrazit"

MScreensEdit
"Редактор"
"Edit"
"Editovat"

MAskApplyCommandTitle
l:
"Применить команду"
"Apply command"
"Aplikovat příkaz"

MAskApplyCommand
"Введите команду для обработки выбранных файлов"
"Enter command to process selected files"
"Zadejte příkaz pro zpracování vybraných souborů"

MPluginConfigTitle
l:
"Конфигурация модулей"
"Plugins configuration"
"Nastavení Pluginů"

MPluginCommandsMenuTitle
"Команды внешних модулей"
"Plugin commands"
"Příkazy pluginů"

MPreparingList
l:
"Создание списка файлов"
"Preparing files list"
"Připravuji seznam souborů"

MLangTitle
l:
"Основной язык"
"Main language"
"Hlavní jazyk"

MHelpLangTitle
"Язык помощи"
"Help language"
"Jazyk nápovědy"

MDefineMacroTitle
l:
"Задание макрокоманды"
"Define macro"
"Definovat makro"

MDefineMacro
"Нажмите желаемую клавишу"
"Press the desired key"
"Stiskněte požadovanou klávesu"

MMacroReDefinedKey
"Макроклавиша '%s' уже определена."
"Macro key '%s' already defined."
"Klávesa makra '%s' již je definována."

MMacroDeleteAssign
"Макроклавиша '%s' не активна."
"Macro key '%s' is not active."
"Klávesa makra '%s' není aktivní."

MMacroDeleteKey
"Макроклавиша '%s' будет удалена."
"Macro key '%s' will be removed."
"Klávesa makra '%s' bude odstraněna."

MMacroCommonReDefinedKey
"Общая макроклавиша '%s' уже определена."
"Common macro key '%s' already defined."
"Klávesa pro běžné makro '%s' již je definována."

MMacroCommonDeleteAssign
"Общая макроклавиша '%s' не активна."
"Common macro key '%s' is not active."
"Klávesa pro běžné makro '%s' není aktivní."

MMacroCommonDeleteKey
"Общая макроклавиша '%s' будет удалена."
"Common macro key '%s' will be removed."
"Klávesa pro běžné makro '%s' bude odstraněna."

MMacroSequence
"Последовательность:"
"Sequence:"
"Posloupnost:"

MMacroReDefinedKey2
"Переопределить?"
"Redefine?"
"Předefinovat?"

MMacroDeleteKey2
"Удалить?"
"Delete?"
"Odstranit?"

MMacroDisDisabledKey
"(макроклавиша не активна)"
"(macro key is not active)"
"(klávesa makra není aktivní)"

MMacroDisOverwrite
"Переопределить"
"Overwrite"
"Přepsat"

MMacroDisAnotherKey
"Изменить клавишу"
"Try another key"
"Zkusit jinou klávesu"

MMacroSettingsTitle
l:
"Параметры макрокоманды для '%s'"
"Macro settings for '%s'"
"Nastavení makra pro '%s'"

MMacroSettingsEnableOutput
"Разрешить во время &выполнения вывод на экран"
"Allo&w screen output while executing macro"
"Povolit &výstup na obrazovku dokud se provádí makro"

MMacroSettingsRunAfterStart
"В&ыполнять после запуска FAR"
"Execute after FAR &start"
"&Spustit po spuštění FARu"

MMacroSettingsActivePanel
"&Активная панель"
"&Active panel"
"&Aktivní panel"

MMacroSettingsPassivePanel
"&Пассивная панель"
"&Passive panel"
"Pa&sivní panel"

MMacroSettingsPluginPanel
"На панели пла&гина"
"P&lugin panel"
"Panel p&luginů"

MMacroSettingsFolders
"Выполнять для папо&к"
"Execute for &folders"
"Spustit pro ad&resáře"

MMacroSettingsSelectionPresent
"&Отмечены файлы"
"Se&lection present"
"E&xistující výběr"

MMacroSettingsCommandLine
"Пустая командная &строка"
"Empty &command line"
"Prázdný pří&kazový řádek"

MMacroSettingsSelectionBlockPresent
"Отмечен б&лок"
"Selection &block present"
"Existující blok výběr&u"

MMacroPErrUnrecognized_keyword
l:
"Неизвестное ключевое слово '%s'"
"Unrecognized keyword '%s'"
"Neznámé klíčové slovo '%s'"

MMacroPErrUnrecognized_function
"Неизвестная функция '%s'"
"Unrecognized function '%s'"
"Neznámá funkce '%s'"

MMacroPErrNot_expected_ELSE
"Неожиданное появление $Else"
"Unexpected $Else"
"Neočekávané $Else"

MMacroPErrNot_expected_END
"Неожиданное появление $End"
"Unexpected $End"
"Neočekávané $End"

MMacroPErrUnexpected_EOS
"Неожиданный конец строки"
"Unexpected end of source string"
"Neočekávaný konec zdrojového řetězce"

MMacroPErrExpected
"Ожидается '%s'"
"Expected '%s'"
"Očekávané '%s'"

MMacroPErrBad_Hex_Control_Char
"Неизвестный шестнадцатеричный управляющий символ"
"Bad Hex Control Char"
"Chybný kontrolní znak Hex"

MMacroPErrBad_Control_Char
"Неправильный управляющий символ"
"Bad Control Char"
"Špatný kontrolní znak"

MMacroPErrVar_Expected
"Переменная '%s' не найдена"
"Variable Expected '%s'"
"Očekávaná proměnná '%s'"

MMacroPErrExpr_Expected
"Ошибка синтаксиса"
"Expression Expected"
"Očekávaný výraz"

MCannotSaveFile
l:
"Ошибка сохранения файла"
"Cannot save file"
"Nelze uložit soubor"

MTextSavedToTemp
"Отредактированный текст записан в"
"Edited text is stored in"
"Editovaný text je uložen v"

MMonthJan
l:
"Янв"
"Jan"
"Led"

MMonthFeb
"Фев"
"Feb"
"Úno"

MMonthMar
"Мар"
"Mar"
"Bře"

MMonthApr
"Апр"
"Apr"
"Dub"

MMonthMay
"Май"
"May"
"Kvě"

MMonthJun
"Июн"
"Jun"
"Čer"

MMonthJul
"Июл"
"Jul"
"Čec"

MMonthAug
"Авг"
"Aug"
"Srp"

MMonthSep
"Сен"
"Sep"
"Zář"

MMonthOct
"Окт"
"Oct"
"Říj"

MMonthNov
"Ноя"
"Nov"
"Lis"

MMonthDec
"Дек"
"Dec"
"Pro"

MPluginHotKeyTitle
l:
"Назначение горячей клавиши"
"Assign plugin hot key"
"Přidělit horkou klávesu pluginu"

MPluginHotKey
"Введите горячую клавишу (букву или цифру)"
"Enter hot key (letter or digit)"
"Zadejte horkou klávesu (písmeno nebo číslici)"

MPluginHotKeyBottom
"F4 - задать горячую клавишу"
"F4 - set hot key"
"F4 - nastavení horké klávesy"

MRightCtrl
l:
"ПравыйCtrl"
"RightCtrl"
"PravýCtrl"

MViewerGoTo
l:
"Перейти"
"Go to"
"Jdi na"

MGoToPercent
"&Процент"
"&Percent"
"&Procent"

MGoToHex
"16-ричное &смещение"
"&Hex offset"
"&Hex offset"

MGoToDecimal
"10-ичное с&мещение"
"&Decimal offset"
"&Desítkový offset"

MExceptTitleFAR
l:
"Внутренняя ошибка"
"Internal error"
"Vnitřní chyba"

MExceptTitleLoad
"Ошибка загрузки плагина"
"Plugin load error"
"Chyba při načítání pluginu"

MExceptTitle
"Ошибка вызова плагина"
"Plugin call error"
"Chyba při volání pluginu"

MExcTrappedException
"Исключительная ситуация:"
"Exception occurred:"
"Vyskytla se výjimka:"

MExcCheckOnLousys
"Передана некорректная информация из модуля:"
"Incorrect information is passed from module:"
"Z modulu byla obdržena nekorektní informace:"

MExcStructWrongFilled
"(некорректно заполнены поля структуры <%s>)"
"(the fields of structure <%s> are wrong filled)"
"(pole struktur <%s> jsou špatně vyplněna)"

MExcStructField
"(структура <%s>, поле <%s>)"
"(structure <%s>, field <%s>)"
"(struktura <%s>, položka <%s>)"

MExcInvalidFuncResult
"Функция <%s> вернула недопустимое значение"
"Function <%s> has returned illegal value"
"Funkce <%s> vrátila nepovolenou hodnotu"

MExcAddress
"Адрес исключения - 0x%p, модуль:"
"Exception address: 0x%p in module:"
"Výjimka na adrese: 0x%X v modulu:"

MExcFARTerminateYes
"FAR Manager завершит работу!"
"FAR Manager will be terminated!"
"FAR Manager bude ukončen!"

MExcUnloadYes
"Плагин будет выгружен!"
"The plugin will be Unloaded!"
"Plugin bude vyřazen!"

MExcRAccess
"\"Нарушение доступа (чтение из 0x%p)\""
"\"Access violation (read from 0x%p)\""
"\"Neplatná adresa (čtení z 0x%p)\""

MExcWAccess
"\"Нарушение доступа (запись в 0x%p)\""
"\"Access violation (write to 0x%p)\""
"\"Neplatná adresa (zápis na 0x%p)\""

MExcEAccess
"\"Нарушение доступа (исполнение кода из 0x%p)\""
"\"Access violation (execute at 0x%p)\""
"\"Neplatná adresa (spuštění na 0x%p)\""

MExcOutOfBounds
"\"Попытка доступа к элементу за границами массива\""
"\"Array out of bounds\""
"\"Pole mimo hranice\""

MExcDivideByZero
"\"Деление на нуль\""
"\"Divide by zero\""
"\"Dělení nulou\""

MExcStackOverflow
"\"Переполнение стека\""
"\"Stack Overflow\""
"\"Přetečení zásobníku\""

MExcBreakPoint
"\"Точка останова\""
"\"Breakpoint exception\""
"\"Výjimka přerušení\""

MExcFloatDivideByZero
"\"Деление на нуль при операции с плавающей точкой\""
"\"Floating-point divide by zero\""
"\"Dělení nulou v pohyblivé čárce\""

MExcFloatOverflow
"\"Переполнение при операции с плавающей точкой\""
"\"Floating point operation overflow\""
"\"Přetečení při operaci v pohyblivé čárce\""

MExcFloatStackOverflow
"\"Стек регистров сопроцессора полон или пуст\""
"\"Floating point stack empty or full\""
"\"Prázdný nebo plný zásobník v pohyblivé čárce\""

MExcFloatUnderflow
"\"Потеря точности при операции с плавающей точкой\""
"\"Floating point operation underflow\""
"\"Podtečení při operaci v pohyblivé čárce\""

MExcBadInstruction
"\"Недопустимая инструкция\""
"\"Illegal instruction\""
"\"Neplatná instrukce\""

MExcDatatypeMisalignment
"\"Попытка доступа к невыравненным данным\""
"\"Alignment fault\""
"\"Chyba zarovnání\""

MExcUnknown
"\"Неизвестное исключение\""
"\"Unknown exception\""
"\"Neznámá výjimka\""

MExcDebugger
"Отладчик"
"Debugger"
"Debugger"

MNetUserName
l:
"Имя пользователя"
"User name"
"Jméno uživatele"

MNetUserPassword
"Пароль пользователя"
"User password"
"Heslo uživatele"

MReadFolderError
l:
"Не удается прочесть содержимое папки"
"Cannot read folder contents"
"Nelze načíst obsah adresáře"

MPlgBadVers
l:
"Этот модуль требует FAR более высокой версии"
"This plugin requires higher FAR version"
"Tento plugin vyžaduje vyšší verzi FARu"

MPlgRequired
"Требуется версия FAR - %d.%d.%d."
"Required FAR version is %d.%d.%d."
"Požadovaná verze FARu je %d.%d.%d."

MPlgRequired2
"Текущая версия FAR - %d.%d.%d."
"Current FAR version is %d.%d.%d"
"Nynější verze FARu je %d.%d.%d"

MPlgLoadPluginError
"Ошибка при загрузке плагина"
"Error loading plugin module"
"Chyba při nahrávání zásuvného modulu"

MBuffSizeTooSmall_1
l:
"Буфер, размещенный под имя файла слишком мал."
"Buffer allocated for file name is too small."
"Buffer alokovaný pro jméno souboru je příliš malý."

MBuffSizeTooSmall_2
"Требуется %d байт, а имеется только %d"
"%d bytes are required, but only %d bytes were allocated."
"Požadováno %d bytů, ale alokováno pouze %d."

MCheckBox2State
l:
"?"
"?"
"?"

MEditInputSize1
"Длина поля"
"Field"
"Pole"

MEditInputSize2
"будет уменьшена до %d байт."
"will be truncated to %d bytes."
"bude oseknuto na %d bytů."

MHelpTitle
l:
"Помощь"
"Help"
"Nápověda"

MHelpActivatorURL
"Эта ссылка запускает внешнее приложение:"
"This reference starts the external application:"
"Tento odkaz spouští externí aplikaci:"

MHelpActivatorFormat
"с параметром:"
"with parameter:"
"s parametrem:"

MHelpActivatorQ
"Желаете запустить?"
"Do you wish to start it?"
"Přejete si ji spustit?"

MCannotOpenHelp
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"

MHelpTopicNotFound
"Не найден запрошенный раздел помощи:"
"Requested help topic not found:"
"požadované téma nápovědy nebylo nalezeno"

MPluginsHelpTitle
l:
"Внешние модули"
"Plugins help"
"Nápověda Pluginů"

MDocumentsHelpTitle
"Документы"
"Documents help"
"Nápověda Dokumentů"

MHelpSearchTitle
l:
"Поиск"
"Search"
"Hledání"

MHelpSearchingFor
"Поиск для"
"Searching for"
"Hledání"

MHelpSearchCannotFind
"Строка не найдена"
"Could not find the string"
"Nelze najít řetězec"

MHelpF1
l:
l:// Help KeyBar F1-12
"Помощь"
"Help"
"Pomoc"

MHelpF2
""
""
""

MHelpF3
""
""
""

MHelpF4
""
""
""

MHelpF5
"Размер"
"Zoom"
"Zoom"

MHelpF6
""
""
""

MHelpF7
"Поиск"
"Search"
"Hledat"

MHelpF8
""
""
""

MHelpF9
""
""
""

MHelpF10
"Выход"
"Quit"
"Konec"

MHelpF11
""
""
""

MHelpF12
""
""
""

MHelpShiftF1
l:
l:// Help KeyBar Shift-F1-12
"Содерж"
"Index"
"Index"

MHelpShiftF2
"Плагин"
"Plugin"
"Plugin"

MHelpShiftF3
"Докум"
"Docums"
"Dokume"

MHelpShiftF4
""
""
""

MHelpShiftF5
""
""
""

MHelpShiftF6
""
""
""

MHelpShiftF7
"Дальше"
"Next"
"Další"

MHelpShiftF8
""
""
""

MHelpShiftF9
""
""
""

MHelpShiftF10
""
""
""

MHelpShiftF11
""
""
""

MHelpShiftF12
""
""
""

MHelpAltF1
l:
l:// Help KeyBar Alt-F1-12
"Пред."
"Prev"
"Předch"

MHelpAltF2
""
""
""

MHelpAltF3
""
""
""

MHelpAltF4
""
""
""

MHelpAltF5
""
""
""

MHelpAltF6
""
""
""

MHelpAltF7
""
""
""

MHelpAltF8
""
""
""

MHelpAltF9
"Видео"
"Video"
"Video"

MHelpAltF10
""
""
""

MHelpAltF11
""
""
""

MHelpAltF12
""
""
""

MHelpCtrlF1
l:
l:// Help KeyBar Ctrl-F1-12
""
""
""

MHelpCtrlF2
""
""
""

MHelpCtrlF3
""
""
""

MHelpCtrlF4
""
""
""

MHelpCtrlF5
""
""
""

MHelpCtrlF6
""
""
""

MHelpCtrlF7
""
""
""

MHelpCtrlF8
""
""
""

MHelpCtrlF9
""
""
""

MHelpCtrlF10
""
""
""

MHelpCtrlF11
""
""
""

MHelpCtrlF12
""
""
""

MHelpCtrlShiftF1
l:
l:// Help KeyBar CtrlShiftF1-12
""
""
""

MHelpCtrlShiftF2
""
""
""

MHelpCtrlShiftF3
""
""
""

MHelpCtrlShiftF4
""
""
""

MHelpCtrlShiftF5
""
""
""

MHelpCtrlShiftF6
""
""
""

MHelpCtrlShiftF7
""
""
""

MHelpCtrlShiftF8
""
""
""

MHelpCtrlShiftF9
""
""
""

MHelpCtrlShiftF10
""
""
""

MHelpCtrlShiftF11
""
""
""

MHelpCtrlShiftF12
""
""
""

MHelpCtrlAltF1
l:
l:// Help KeyBar CtrlAltF1-12
""
""
""

MHelpCtrlAltF2
""
""
""

MHelpCtrlAltF3
""
""
""

MHelpCtrlAltF4
""
""
""

MHelpCtrlAltF5
""
""
""

MHelpCtrlAltF6
""
""
""

MHelpCtrlAltF7
""
""
""

MHelpCtrlAltF8
""
""
""

MHelpCtrlAltF9
""
""
""

MHelpCtrlAltF10
""
""
""

MHelpCtrlAltF11
""
""
""

MHelpCtrlAltF12
""
""
""

MHelpAltShiftF1
l:
l:// Help KeyBar AltShiftF1-12
""
""
""

MHelpAltShiftF2
""
""
""

MHelpAltShiftF3
""
""
""

MHelpAltShiftF4
""
""
""

MHelpAltShiftF5
""
""
""

MHelpAltShiftF6
""
""
""

MHelpAltShiftF7
""
""
""

MHelpAltShiftF8
""
""
""

MHelpAltShiftF9
""
""
""

MHelpAltShiftF10
""
""
""

MHelpAltShiftF11
""
""
""

MHelpAltShiftF12
""
""
""

MHelpCtrlAltShiftF1
l:
l:// Help KeyBar CtrlAltShiftF1-12
""
""
""

MHelpCtrlAltShiftF2
""
""
""

MHelpCtrlAltShiftF3
""
""
""

MHelpCtrlAltShiftF4
""
""
""

MHelpCtrlAltShiftF5
""
""
""

MHelpCtrlAltShiftF6
""
""
""

MHelpCtrlAltShiftF7
""
""
""

MHelpCtrlAltShiftF8
""
""
""

MHelpCtrlAltShiftF9
""
""
""

MHelpCtrlAltShiftF10
""
""
""

MHelpCtrlAltShiftF11
""
""
""

MHelpCtrlAltShiftF12
""
""
""

MInfoF1
l:
l:// InfoPanel KeyBar F1-F12
"Помощь"
"Help"
"Pomoc"

MInfoF2
"Сверн"
"Wrap"
"Zalam"

MInfoF3
"СмОпис"
"VieDiz"
"Zobraz"

MInfoF4
"РедОпи"
"EdtDiz"
"Edit"

MInfoF5
""
""
""

MInfoF6
""
""
""

MInfoF7
"Поиск"
"Search"
"Hledat"

MInfoF8
"ANSI"
"ANSI"
"ANSI"

MInfoF9
"КонфМн"
"ConfMn"
"KonfMn"

MInfoF10
"Выход"
"Quit"
"Konec"

MInfoF11
"Модули"
"Plugin"
"Plugin"

MInfoF12
"Экраны"
"Screen"
"Obraz."

MInfoShiftF1
l:
l:// InfoPanel KeyBar Shift-F1-F12
""
""
""

MInfoShiftF2
"Слова"
"WWrap"
"ZalSlo"

MInfoShiftF3
""
""
""

MInfoShiftF4
""
""
""

MInfoShiftF5
""
""
""

MInfoShiftF6
""
""
""

MInfoShiftF7
"Дальше"
"Next"
"Další"

MInfoShiftF8
"Таблиц"
"Table"
"ZnSady"

MInfoShiftF9
"Сохран"
"Save"
"Uložit"

MInfoShiftF10
"Послдн"
"Last"
"Posled"

MInfoShiftF11
""
""
""

MInfoShiftF12
""
""
""

MInfoAltF1
l:
l:// InfoPanel KeyBar Alt-F1-F12
"Левая"
"Left"
"Levý"

MInfoAltF2
"Правая"
"Right"
"Pravý"

MInfoAltF3
""
""
""

MInfoAltF4
""
""
""

MInfoAltF5
""
""
""

MInfoAltF6
""
""
""

MInfoAltF7
"Искать"
"Find"
"Hledat"

MInfoAltF8
"Строка"
"Goto"
"Jít na"

MInfoAltF9
"Видео"
"Video"
"Video"

MInfoAltF10
"Дерево"
"Tree"
"Strom"

MInfoAltF11
"ИстПр"
"ViewHs"
"ProhHs"

MInfoAltF12
"ИстПап"
"FoldHs"
"AdrsHs"

MInfoCtrlF1
l:
l:// InfoPanel KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"

MInfoCtrlF2
"Правая"
"Right"
"Pravý"

MInfoCtrlF3
""
""
""

MInfoCtrlF4
""
""
""

MInfoCtrlF5
""
""
""

MInfoCtrlF6
""
""
""

MInfoCtrlF7
""
""
""

MInfoCtrlF8
""
""
""

MInfoCtrlF9
""
""
""

MInfoCtrlF10
""
""
""

MInfoCtrlF11
""
""
""

MInfoCtrlF12
""
""
""

MInfoCtrlShiftF1
l:
l:// InfoPanel KeyBar CtrlShiftF1-12
""
""
""

MInfoCtrlShiftF2
""
""
""

MInfoCtrlShiftF3
""
""
""

MInfoCtrlShiftF4
""
""
""

MInfoCtrlShiftF5
""
""
""

MInfoCtrlShiftF6
""
""
""

MInfoCtrlShiftF7
""
""
""

MInfoCtrlShiftF8
""
""
""

MInfoCtrlShiftF9
""
""
""

MInfoCtrlShiftF10
""
""
""

MInfoCtrlShiftF11
""
""
""

MInfoCtrlShiftF12
""
""
""

MInfoCtrlAltF1
l:
l:// InfoPanel KeyBar CtrlAltF1-12
""
""
""

MInfoCtrlAltF2
""
""
""

MInfoCtrlAltF3
""
""
""

MInfoCtrlAltF4
""
""
""

MInfoCtrlAltF5
""
""
""

MInfoCtrlAltF6
""
""
""

MInfoCtrlAltF7
""
""
""

MInfoCtrlAltF8
""
""
""

MInfoCtrlAltF9
""
""
""

MInfoCtrlAltF10
""
""
""

MInfoCtrlAltF11
""
""
""

MInfoCtrlAltF12
""
""
""

MInfoAltShiftF1
l:
l:// InfoPanel KeyBar AltShiftF1-12
""
""
""

MInfoAltShiftF2
""
""
""

MInfoAltShiftF3
""
""
""

MInfoAltShiftF4
""
""
""

MInfoAltShiftF5
""
""
""

MInfoAltShiftF6
""
""
""

MInfoAltShiftF7
""
""
""

MInfoAltShiftF8
""
""
""

MInfoAltShiftF9
""
""
""

MInfoAltShiftF10
""
""
""

MInfoAltShiftF11
""
""
""

MInfoAltShiftF12
""
""
""

MInfoCtrlAltShiftF1
l:
l:// InfoPanel KeyBar CtrlAltShiftF1-12
""
""
""

MInfoCtrlAltShiftF2
""
""
""

MInfoCtrlAltShiftF3
""
""
""

MInfoCtrlAltShiftF4
""
""
""

MInfoCtrlAltShiftF5
""
""
""

MInfoCtrlAltShiftF6
""
""
""

MInfoCtrlAltShiftF7
""
""
""

MInfoCtrlAltShiftF8
""
""
""

MInfoCtrlAltShiftF9
""
""
""

MInfoCtrlAltShiftF10
""
""
""

MInfoCtrlAltShiftF11
""
""
""

MInfoCtrlAltShiftF12
""
""
""

MQViewF1
l:
l:// QView KeyBar F1-F12
"Помощь"
"Help"
"Pomoc"

MQViewF2
"Сверн"
"Wrap"
"Zalam"

MQViewF3
"Просм"
"View"
"Zobraz"

MQViewF4
"Код"
"Hex"
"Hex"

MQViewF5
""
""
""

MQViewF6
""
""
""

MQViewF7
"Поиск"
"Search"
"Hledat"

MQViewF8
"ANSI"
"ANSI"
"ANSI"

MQViewF9
"КонфМн"
"ConfMn"
"KonfMn"

MQViewF10
"Выход"
"Quit"
"Konec"

MQViewF11
"Модули"
"Plugin"
"Plugin"

MQViewF12
"Экраны"
"Screen"
"Obraz."

MQViewShiftF1
l:
l:// QView KeyBar Shift-F1-F12
""
""
""

MQViewShiftF2
"Слова"
"WWrap"
"ZalSlo"

MQViewShiftF3
""
""
""

MQViewShiftF4
""
""
""

MQViewShiftF5
""
""
""

MQViewShiftF6
""
""
""

MQViewShiftF7
"Дальше"
"Next"
"Další"

MQViewShiftF8
"Таблиц"
"Table"
"ZnSady"

MQViewShiftF9
"Сохран"
"Save"
"Uložit"

MQViewShiftF10
"Послдн"
"Last"
"Posled"

MQViewShiftF11
""
""
""

MQViewShiftF12
""
""
""

MQViewAltF1
l:
l:// QView KeyBar Alt-F1-F12
"Левая"
"Left"
"Levý"

MQViewAltF2
"Правая"
"Right"
"Pravý"

MQViewAltF3
""
""
""

MQViewAltF4
""
""
""

MQViewAltF5
""
""
""

MQViewAltF6
""
""
""

MQViewAltF7
"Искать"
"Find"
"Hledat"

MQViewAltF8
"Строка"
"Goto"
"Jít na"

MQViewAltF9
"Видео"
"Video"
"Video"

MQViewAltF10
"Дерево"
"Tree"
"Strom"

MQViewAltF11
"ИстПр"
"ViewHs"
"ProhHs"

MQViewAltF12
"ИстПап"
"FoldHs"
"AdrsHs"

MQViewCtrlF1
l:
l:// QView KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"

MQViewCtrlF2
"Правая"
"Right"
"Pravý"

MQViewCtrlF3
""
""
""

MQViewCtrlF4
""
""
""

MQViewCtrlF5
""
""
""

MQViewCtrlF6
""
""
""

MQViewCtrlF7
""
""
""

MQViewCtrlF8
""
""
""

MQViewCtrlF9
""
""
""

MQViewCtrlF10
""
""
""

MQViewCtrlF11
""
""
""

MQViewCtrlF12
""
""
""

MQViewCtrlShiftF1
l:
l:// QView KeyBar CtrlShiftF1-12
""
""
""

MQViewCtrlShiftF2
""
""
""

MQViewCtrlShiftF3
""
""
""

MQViewCtrlShiftF4
""
""
""

MQViewCtrlShiftF5
""
""
""

MQViewCtrlShiftF6
""
""
""

MQViewCtrlShiftF7
""
""
""

MQViewCtrlShiftF8
""
""
""

MQViewCtrlShiftF9
""
""
""

MQViewCtrlShiftF10
""
""
""

MQViewCtrlShiftF11
""
""
""

MQViewCtrlShiftF12
""
""
""

MQViewCtrlAltF1
l:
l:// QView KeyBar CtrlAltF1-12
""
""
""

MQViewCtrlAltF2
""
""
""

MQViewCtrlAltF3
""
""
""

MQViewCtrlAltF4
""
""
""

MQViewCtrlAltF5
""
""
""

MQViewCtrlAltF6
""
""
""

MQViewCtrlAltF7
""
""
""

MQViewCtrlAltF8
""
""
""

MQViewCtrlAltF9
""
""
""

MQViewCtrlAltF10
""
""
""

MQViewCtrlAltF11
""
""
""

MQViewCtrlAltF12
""
""
""

MQViewAltShiftF1
l:
l:// QView KeyBar AltShiftF1-12
""
""
""

MQViewAltShiftF2
""
""
""

MQViewAltShiftF3
""
""
""

MQViewAltShiftF4
""
""
""

MQViewAltShiftF5
""
""
""

MQViewAltShiftF6
""
""
""

MQViewAltShiftF7
""
""
""

MQViewAltShiftF8
""
""
""

MQViewAltShiftF9
""
""
""

MQViewAltShiftF10
""
""
""

MQViewAltShiftF11
""
""
""

MQViewAltShiftF12
""
""
""

MQViewCtrlAltShiftF1
l:
l:// QView KeyBar CtrlAltShiftF1-12
""
""
""

MQViewCtrlAltShiftF2
""
""
""

MQViewCtrlAltShiftF3
""
""
""

MQViewCtrlAltShiftF4
""
""
""

MQViewCtrlAltShiftF5
""
""
""

MQViewCtrlAltShiftF6
""
""
""

MQViewCtrlAltShiftF7
""
""
""

MQViewCtrlAltShiftF8
""
""
""

MQViewCtrlAltShiftF9
""
""
""

MQViewCtrlAltShiftF10
""
""
""

MQViewCtrlAltShiftF11
""
""
""

MQViewCtrlAltShiftF12
""
""
""

MKBTreeF1
l:
l:// Tree KeyBar F1-F12
"Помощь"
"Help"
"Pomoc"

MKBTreeF2
"ПользМ"
"UserMn"
"UživMn"

MKBTreeF3
""
""
""

MKBTreeF4
"Атриб"
"Attr"
"Attr"

MKBTreeF5
"Копир"
"Copy"
"Kopír."

MKBTreeF6
"Перен"
"RenMov"
"PřjPřs"

MKBTreeF7
"Папка"
"MkFold"
"VytAdr"

MKBTreeF8
"Удален"
"Delete"
"Smazat"

MKBTreeF9
"КонфМн"
"ConfMn"
"KonfMn"

MKBTreeF10
"Выход"
"Quit"
"Konec"

MKBTreeF11
"Модули"
"Plugin"
"Plugin"

MKBTreeF12
"Экраны"
"Screen"
"Obraz."

MKBTreeShiftF1
l:
l:// Tree KeyBar Shift-F1-F12
""
""
""

MKBTreeShiftF2
""
""
""

MKBTreeShiftF3
""
""
""

MKBTreeShiftF4
""
""
""

MKBTreeShiftF5
"Копир"
"Copy"
"Kopír."

MKBTreeShiftF6
"Перен"
"Rename"
"Přejm."

MKBTreeShiftF7
""
""
""

MKBTreeShiftF8
""
""
""

MKBTreeShiftF9
"Сохран"
"Save"
"Uložit"

MKBTreeShiftF10
"Послдн"
"Last"
"Posled"

MKBTreeShiftF11
"Группы"
"Group"
"Skupin"

MKBTreeShiftF12
"Выбран"
"SelUp"
"VybPrv"

MKBTreeAltF1
l:
l:// Tree KeyBar Alt-F1-F12
"Левая"
"Left"
"Levý"

MKBTreeAltF2
"Правая"
"Right"
"Pravý"

MKBTreeAltF3
""
""
""

MKBTreeAltF4
""
""
""

MKBTreeAltF5
""
""
""

MKBTreeAltF6
""
""
""

MKBTreeAltF7
"Искать"
"Find"
"Hledat"

MKBTreeAltF8
"Истор"
"Histry"
"Histor"

MKBTreeAltF9
"Видео"
"Video"
"Video"

MKBTreeAltF10
"Дерево"
"Tree"
"Strom"

MKBTreeAltF11
"ИстПр"
"ViewHs"
"ProhHs"

MKBTreeAltF12
"ИстПап"
"FoldHs"
"AdrsHs"

MKBTreeCtrlF1
l:
l:// Tree KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"

MKBTreeCtrlF2
"Правая"
"Right"
"Pravý"

MKBTreeCtrlF3
""
""
""

MKBTreeCtrlF4
""
""
""

MKBTreeCtrlF5
""
""
""

MKBTreeCtrlF6
""
""
""

MKBTreeCtrlF7
""
""
""

MKBTreeCtrlF8
""
""
""

MKBTreeCtrlF9
""
""
""

MKBTreeCtrlF10
""
""
""

MKBTreeCtrlF11
""
""
""

MKBTreeCtrlF12
""
""
""

MKBTreeCtrlShiftF1
l:
l:// Tree KeyBar CtrlShiftF1-12
""
""
""

MKBTreeCtrlShiftF2
""
""
""

MKBTreeCtrlShiftF3
""
""
""

MKBTreeCtrlShiftF4
""
""
""

MKBTreeCtrlShiftF5
""
""
""

MKBTreeCtrlShiftF6
""
""
""

MKBTreeCtrlShiftF7
""
""
""

MKBTreeCtrlShiftF8
""
""
""

MKBTreeCtrlShiftF9
""
""
""

MKBTreeCtrlShiftF10
""
""
""

MKBTreeCtrlShiftF11
""
""
""

MKBTreeCtrlShiftF12
""
""
""

MKBTreeCtrlAltF1
l:
l:// Tree KeyBar CtrlAltF1-12
""
""
""

MKBTreeCtrlAltF2
""
""
""

MKBTreeCtrlAltF3
""
""
""

MKBTreeCtrlAltF4
""
""
""

MKBTreeCtrlAltF5
""
""
""

MKBTreeCtrlAltF6
""
""
""

MKBTreeCtrlAltF7
""
""
""

MKBTreeCtrlAltF8
""
""
""

MKBTreeCtrlAltF9
""
""
""

MKBTreeCtrlAltF10
""
""
""

MKBTreeCtrlAltF11
""
""
""

MKBTreeCtrlAltF12
""
""
""

MKBTreeAltShiftF1
l:
l:// Tree KeyBar AltShiftF1-12
""
""
""

MKBTreeAltShiftF2
""
""
""

MKBTreeAltShiftF3
""
""
""

MKBTreeAltShiftF4
""
""
""

MKBTreeAltShiftF5
""
""
""

MKBTreeAltShiftF6
""
""
""

MKBTreeAltShiftF7
""
""
""

MKBTreeAltShiftF8
""
""
""

MKBTreeAltShiftF9
""
""
""

MKBTreeAltShiftF10
""
""
""

MKBTreeAltShiftF11
""
""
""

MKBTreeAltShiftF12
""
""
""

MKBTreeCtrlAltShiftF1
l:
l:// Tree KeyBar CtrlAltShiftF1-12
""
""
""

MKBTreeCtrlAltShiftF2
""
""
""

MKBTreeCtrlAltShiftF3
""
""
""

MKBTreeCtrlAltShiftF4
""
""
""

MKBTreeCtrlAltShiftF5
""
""
""

MKBTreeCtrlAltShiftF6
""
""
""

MKBTreeCtrlAltShiftF7
""
""
""

MKBTreeCtrlAltShiftF8
""
""
""

MKBTreeCtrlAltShiftF9
""
""
""

MKBTreeCtrlAltShiftF10
""
""
""

MKBTreeCtrlAltShiftF11
""
""
""

MKBTreeCtrlAltShiftF12
""
""
""

MCopyTimeInfo
l:
"Время: %8.8s  Осталось: %8.8s  %5d%1.1sб/с"
"Time: %8.8s  Remaining: %8.8s  %5d%1.1sb/s"
"Čas: %8.8s  Zbývá: %8.8s  %5d%1.1sb/s"

MKeyESCWasPressed
l:
"Действие было прервано"
"Operation has been interrupted"
"Operace byla přerušena"

MDoYouWantToStopWork
"Вы действительно хотите отменить действие?"
"Do you really want to cancel it?"
"Opravdu chcete operaci stornovat?"

MDoYouWantToStopWork2
"Продолжить выполнение?"
"Continue work? "
"Pokračovat v práci?"

MCheckingFileInPlugin
l:
"Файл проверяется в плагине"
"The file is being checked by the plugin"
"Soubor je právě kontrolován pluginem"

MDialogType
l:
"Диалог"
"Dialog"
"Dialog"

MHelpType
"Помощь"
"Help"
"Nápověda"

MFolderTreeType
"ПоискКаталогов"
"FolderTree"
"StromAdresáře"

MVMenuType
"Меню"
"Menu"
"Menu"

MIncorrectMask
l:
"Некорректная маска файлов!"
"File-mask string contains errors!"
"Řetězec masky souboru obsahuje chyby!"

MPanelBracketsForLongName
l:
"{}"
"{}"
"{}"

MComspecNotFound
l:
"Переменная окружения %COMSPEC% не определена!"
"Environment variable %COMSPEC% not defined!"
"Proměnná prostředí %COMSPEC% není definována!"

MExecuteErrorMessage
"'%s' не является внутренней или внешней командой, исполняемой программой или пакетным файлом.\n"
"'%s' is not recognized as an internal or external command, operable program or batch file.\n"
"'%s' nebylo nalezeno jako vniřní nebo externí příkaz, spustitelná aplikace nebo dávkový soubor.\n"

MOpenPluginCannotOpenFile
l:
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"

MFileFilterTitle
l:
"Фильтр"
"Filter"
"Filtr"

MFileHilightTitle
"Раскраска файлов"
"Files highlighting"
"Zvýrazňování souborů"

MFileFilterName
"Имя &фильтра:"
"Filter &name:"
"Jmé&no filtru:"

MFileFilterMatchMask
"&Маска:"
"&Mask:"
"&Maska"

MFileFilterSize
"Разм&ер:"
"Si&ze:"
"Vel&ikost"

MFileFilterSizeFromSign
">="
">="
"<="

MFileFilterSizeToSign
"<="
"<="
"<="

MFileFilterDate
"&Дата/Время:"
"Da&te/Time:"
"Dat&um/Čas:"

MFileFilterModified
"&модификации"
"&modification"
"&modifikace"

MFileFilterCreated
"&создания"
"&creation"
"&vytvoření"

MFileFilterOpened
"&доступа"
"&access"
"&přístupu"

MFileFilterDateRelative
"Относительна&я"
"Relat&ive"
"Relati&vní"

MFileFilterDateAfterSign
">="
">="
">="

MFileFilterDateBeforeSign
"<="
"<="
"<="

MFileFilterCurrent
"Теку&щая"
"C&urrent"
"Aktuá&lní"

MFileFilterBlank
"С&брос"
"B&lank"
"Práz&dný"

MFileFilterAttr
"Атрибут&ы"
"Attri&butes"
"Attri&buty"

MFileFilterAttrR
"&Только для чтения"
"&Read only"
"Jen pro čt&ení"

MFileFilterAttrA
"&Архивный"
"&Archive"
"Arc&hivovat"

MFileFilterAttrH
"&Скрытый"
"&Hidden"
"Skry&tý"

MFileFilterAttrS
"С&истемный"
"&System"
"Systémo&vý"

MFileFilterAttrC
"С&жатый"
"&Compressed"
"Kompri&movaný"

MFileFilterAttrE
"&Зашифрованный"
"&Encrypted"
"Ši&frovaný"

MFileFilterAttrD
"&Каталог"
"&Directory"
"Adr&esář"

MFileFilterAttrNI
"&Неиндексируемый"
"Not inde&xed"
"Neinde&xovaný"

MFileFilterAttrSparse
"&Разреженный"
"S&parse"
"Říd&ký"

MFileFilterAttrT
"&Временный"
"Temporar&y"
"Doča&sný"

MFileFilterAttrReparse
"Симво&л. ссылка"
"Symbolic lin&k"
"Sybolický li&nk"

MFileFilterAttrOffline
"Автономны&й"
"O&ffline"
"O&ffline"

MFileFilterAttrVirtual
"Вирт&уальный"
"&Virtual"
"Virtuální"

MFileFilterReset
"Очистит&ь"
"Reset"
"Reset"

MFileFilterCancel
"Отмена"
"Cancel"
"Storno"

MFileFilterMakeTransparent
"Выставить прозрачность"
"Make transparent"
"Zprůhlednit"

MBadFileSizeFormat
"Неправильно заполнено поле размера!"
"File size field is incorrectly filled!"
"Velikost souboru neobsahuje správnou hodnotu!"

#Must be the last
MNewFileName
l:
"?Новый файл?"
"?New File?"
"?Nový soubor?"
