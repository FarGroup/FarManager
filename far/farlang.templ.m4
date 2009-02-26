m4_include(`farversion.m4')m4_dnl
#hpp file name
lang.hpp
#number of languages
4
#id:0 language file name, language name, language description
FarRus.lng Russian "Russian (Русский)"
#id:1 language file name, language name, language description
FarEng.lng English "English"
#id:2 language file name, language name, language description
FarCze.lng Czech "Czech (Čeština)"
#id:3 language file name, language name, language description
FarGer.lng German "German (Deutsch)"

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
"Ja"

MNo
"Нет"
"No"
"Ne"
"Nein"

MOk
"Продолжить"
"OK"
"Ok"
"OK"

MHYes
l:
"&Да"
"&Yes"
"&Ano"
"&Ja"

MHNo
"&Нет"
"&No"
"&Ne"
"&Nein"

MHOk
"&Продолжить"
"&OK"
"&Ok"
"&OK"

MCancel
l:
"Отменить"
"Cancel"
"Storno"
"Abbrechen"

MRetry
"Повторить"
"Retry"
"Znovu"
"Wiederholen"

MSkip
"Пропустить"
"Skip"
"Přeskočit"
"Überspringen"

MAbort
"Прервать"
"Abort"
"Zrušit"
"Abbrechen"

MIgnore
"Игнорировать"
"Ignore"
"Ignorovat"
"Ignorieren"

MDelete
"Удалить"
"Delete"
"Smazat"
"Löschen"

MSplit
"Разделить"
"Split"
"Rozdělit"
"Zerteilen"

MRemove
"Удалить"
"Remove"
"Odstranit"
"Entfernen"

MHCancel
l:
"&Отменить"
"&Cancel"
"&Storno"
"&Abbrechen"

MHRetry
"&Повторить"
"&Retry"
"&Znovu"
"&Wiederholen"

MHSkip
"П&ропустить"
"&Skip"
"&Přeskočit"
"Über&springen"

MHSkipAll
"Пропустить &все"
"S&kip all"
"Přeskočit &vše"
"Alle übersprin&gen"

MHAbort
"Прер&вать"
"&Abort"
"Zr&ušit"
"&Abbrechen"

MHIgnore
"&Игнорировать"
"&Ignore"
"&Ignorovat"
"&Ignorieren"

MHDelete
"&Удалить"
"&Delete"
"S&mazat"
"&Löschen"

MHRemove
"&Удалить"
"R&emove"
"&Odstranit"
"Ent&fernen"

MHSplit
"Раз&делить"
"Sp&lit"
"&Rozdělit"
"&Zerteilen"

MWarning
l:
"Предупреждение"
"Warning"
"Varování"
"Warnung"

MError
"Ошибка"
"Error"
"Chyba"
"Fehler"

MQuit
l:
"Выход"
"Quit"
"Konec"
"Beenden"

MAskQuit
"Вы хотите завершить работу в FAR?"
"Do you want to quit FAR?"
"Opravdu chcete ukončit FAR?"
"Wollen Sie FAR beenden?"

MF1
l:
l://functional keys - 6 characters max
"Помощь"
"Help"
"Pomoc"
"Hilfe"

MF2
"ПользМ"
"UserMn"
"UživMn"
"BenuMn"

MF3
"Просм"
"View"
"Zobraz"
"Betr."

MF4
"Редакт"
"Edit"
"Edit"
"Bearb"

MF5
"Копир"
"Copy"
"Kopír."
"Kopier"

MF6
"Перен"
"RenMov"
"PřjPřs"
"Versch"

MF7
"Папка"
"MkFold"
"VytAdr"
"VerzEr"

MF8
"Удален"
"Delete"
"Smazat"
"Lösch."

MF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"

MF10
"Выход"
"Quit"
"Konec"
"Beend."

MF11
"Модули"
"Plugin"
"Plugin"
"Plugin"

MF12
"Экраны"
"Screen"
"Obraz."
"Seiten"

MAltF1
l:
"Левая"
"Left"
"Levý"
"Links"

MAltF2
"Правая"
"Right"
"Pravý"
"Rechts"

MAltF3
"Смотр."
"View.."
"Zobr.."
"Betr.."

MAltF4
"Редак."
"Edit.."
"Edit.."
"Bear.."

MAltF5
"Печать"
"Print"
"Tisk"
"Druck"

MAltF6
"Ссылка"
"MkLink"
"VytLnk"
"LinkEr"

MAltF7
"Искать"
"Find"
"Hledat"
"Suchen"

MAltF8
"Истор"
"Histry"
"Histor"
"Histor"

MAltF9
"Видео"
"Video"
"Video"
"Ansich"

MAltF10
"Дерево"
"Tree"
"Strom"
"Baum"

MAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"

MAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"BearHs"

MCtrlF1
l:
"Левая"
"Left"
"Levý"
"Links"

MCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"

MCtrlF3
"Имя   "
"Name  "
"Název "
"Name  "

MCtrlF4
"Расшир"
"Extens"
"Přípon"
"Erweit"

MCtrlF5
"Модиф"
"Modifn"
"Modifk"
"Veränd"

MCtrlF6
"Размер"
"Size"
"Veliko"
"Größe"

MCtrlF7
"Несорт"
"Unsort"
"Neřadi"
"Unsort"

MCtrlF8
"Создан"
"Creatn"
"Vytvoř"
"Erstel"

MCtrlF9
"Доступ"
"Access"
"Přístu"
"Zugrif"

MCtrlF10
"Описан"
"Descr"
"Popis"
"Beschr"

MCtrlF11
"Владел"
"Owner"
"Vlastn"
"Besitz"

MCtrlF12
"Сорт"
"Sort"
"Třídit"
"Sort."

MShiftF1
l:
"Добавл"
"Add"
"Přidat"
"Hinzu"

MShiftF2
"Распак"
"Extrct"
"Rozbal"
"Extrah"

MShiftF3
"АрхКом"
"ArcCmd"
"ArcPří"
"ArcBef"

MShiftF4
"Редак."
"Edit.."
"Edit.."
"Erst.."

MShiftF5
"Копир"
"Copy"
"Kopír."
"Kopier"

MShiftF6
"Переим"
"Rename"
"Přejme"
"Umbene"

MShiftF7
""
""
""
""

MShiftF8
"Удален"
"Delete"
"Smazat"
"Lösch."

MShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"

MShiftF10
"Послдн"
"Last"
"Posled"
"Letzte"

MShiftF11
"Группы"
"Group"
"Skupin"
"Gruppe"

MShiftF12
"Выбран"
"SelUp"
"VybPrv"
"AuswOb"

MAltShiftF1
l:
l:// Main AltShift
""
""
""
""

MAltShiftF2
""
""
""
""

MAltShiftF3
""
""
""
""

MAltShiftF4
""
""
""
""

MAltShiftF5
""
""
""
""

MAltShiftF6
""
""
""
""

MAltShiftF7
""
""
""
""

MAltShiftF8
""
""
""
""

MAltShiftF9
"КонфПл"
"ConfPl"
"KonfPl"
"KonfPn"

MAltShiftF10
""
""
""
""

MAltShiftF11
""
""
""
""

MAltShiftF12
""
""
""
""

MCtrlShiftF1
l:
l://Main CtrlShift
""
""
""
""

MCtrlShiftF2
""
""
""
""

MCtrlShiftF3
"Просм"
"View"
"Zobraz"
"Betr"

MCtrlShiftF4
"Редакт"
"Edit"
"Edit"
"Bearb"

MCtrlShiftF5
""
""
""
""

MCtrlShiftF6
""
""
""
""

MCtrlShiftF7
""
""
""
""

MCtrlShiftF8
""
""
""
""

MCtrlShiftF9
""
""
""
""

MCtrlShiftF10
""
""
""
""

MCtrlShiftF11
""
""
""
""

MCtrlShiftF12
""
""
""
""

MCtrlAltF1
l:
l:// Main CtrlAlt
""
""
""
""

MCtrlAltF2
""
""
""
""

MCtrlAltF3
""
""
""
""

MCtrlAltF4
""
""
""
""

MCtrlAltF5
""
""
""
""

MCtrlAltF6
""
""
""
""

MCtrlAltF7
""
""
""
""

MCtrlAltF8
""
""
""
""

MCtrlAltF9
""
""
""
""

MCtrlAltF10
""
""
""
""

MCtrlAltF11
""
""
""
""

MCtrlAltF12
""
""
""
""

MCtrlAltShiftF1
l:
l:// Main CtrlAltShift
""
""
""
""

MCtrlAltShiftF2
""
""
""
""

MCtrlAltShiftF3
""
""
""
""

MCtrlAltShiftF4
""
""
""
""

MCtrlAltShiftF5
""
""
""
""

MCtrlAltShiftF6
""
""
""
""

MCtrlAltShiftF7
""
""
""
""

MCtrlAltShiftF8
""
""
""
""

MCtrlAltShiftF9
""
""
""
""

MCtrlAltShiftF10
""
""
""
""

MCtrlAltShiftF11
""
""
""
""

MCtrlAltShiftF12
le://End of functional keys
""
""
""
""

MHistoryTitle
l:
"История команд"
"History"
"Historie"
"Historie der letzten Befehle"

MFolderHistoryTitle
"История папок"
"Folders history"
"Historie adresářů"
"Zuletzt besuchte Ordner"

MViewHistoryTitle
"История просмотра"
"File view history"
"Historie prohlížení souborů"
"Zuletzt betrachtete Dateien"

MViewHistoryIsCreate
"Создать файл?"
"Create file?"
"Vytvořit soubor?"
"Datei erstellen?"

MHistoryView
"Просмотр"
"View"
"Zobrazit"
"Betr"

MHistoryEdit
"Редактор"
"Edit"
"Editovat"
"Bearb"

MHistoryExt
"Внешний "
"Ext."
"Rozšíření"
"Ext."

MHistoryClear
l:
"История будет полностью очищена. Продолжить?"
"All records in the history will be deleted. Continue?"
"Všechny záznamy v historii budou smazány. Pokračovat?"
"Die gesamte Historie wird gelöscht. Fortfahren?"

MClear
"&Очистить"
"&Clear history"
"&Vymazat historii"
"Historie &löschen"

MConfigSystemTitle
l:
"Системные параметры"
"System settings"
"Nastavení systému"
"Grundeinstellungen"

MConfigRO
"&Снимать атрибут R/O c CD файлов"
"&Clear R/O attribute from CD files"
"Z&rušit atribut R/O u souborů na CD"
"Schreibschutz von CD-Dateien ent&fernen"

MConfigRecycleBin
"Удалять в &Корзину"
"&Delete to Recycle Bin"
"&Mazat do Koše"
"In Papierkorb &löschen"

MConfigRecycleBinLink
"У&далять символические ссылки"
"Delete symbolic &links"
"Mazat symbolické &linky"
"Symbolische L&inks löschen"

MConfigSystemCopy
"Использовать систе&мную функцию копирования"
"Use sys&tem copy routine"
"Používat kopírovací rutiny sys&tému"
"Sys&temeigene Kopierroutine verwenden"

MConfigCopySharing
"Копировать открытые для &записи файлы"
"Copy files opened for &writing"
"Kopírovat soubory otevřené pro &zápis"
"Zum Schreiben geöffnete Dateien &kopieren"

MConfigScanJunction
"Ск&анировать символические ссылки"
"Scan s&ymbolic links"
"Prohledávat s&ymbolické linky"
"S&ymbolische Links scannen"

MConfigCreateUppercaseFolders
"Создавать &папки заглавными буквами"
"Create folders in &uppercase"
"Vytvářet adresáře &velkými písmeny"
"Ordner in Großschreib&ung erstellen"

MConfigInactivity
"&Время бездействия"
"&Inactivity time"
"&Doba nečinnosti"
"Inaktivitäts&zeit"

MConfigInactivityMinutes
"минут"
"minutes"
"minut"
"Minuten"

MConfigSaveHistory
"Сохранять &историю команд"
"Save commands &history"
"Ukládat historii &příkazů"
"&Befehlshistorie speichern"

MConfigSaveFoldersHistory
"Сохранять историю п&апок"
"Save &folders history"
"Ukládat historii &adresářů"
"&Ordnerhistorie speichern"

MConfigSaveViewHistory
"Сохранять историю п&росмотра и редактора"
"Save &view and edit history"
"Ukládat historii Zobraz a Editu&j"
"Betrachter/&Editor-Historie speichern"

MConfigRegisteredTypes
"Использовать стандартные &типы файлов"
"Use Windows &registered types"
"Používat regi&strované typy Windows"
"&Registrierte Windows-Dateitypen verwenden"

MConfigCloseCDGate
"Автоматически монтироват&ь CDROM"
"CD drive auto &mount"
"Automatické př&ipojení CD disků"
"CD-Laufwerk auto&matisch schließen"

MConfigPersonalPath
"Путь к персональным п&лагинам:"
"&Path for personal plugins:"
"&Cesta k vlastním pluginům:"
"&Pfad für eigene Plugins:"

MConfigAutoSave
"Автозапись кон&фигурации"
"Auto &save setup"
"Automatické ukládaní &nastavení"
"Setup automatisch &"speichern"

MConfigPanelTitle
l:
"Настройки панели"
"Panel settings"
"Nastavení panelů"
"Panels einrichten"

MConfigHidden
"Показывать скр&ытые и системные файлы"
"Show &hidden and system files"
"Ukázat &skryté a systémové soubory"
"&Versteckte und Systemdateien anzeigen"

MConfigHighlight
"&Раскраска файлов"
"Hi&ghlight files"
"Zvý&razňovat soubory"
"Dateien mark&ieren"

MConfigAutoChange
"&Автосмена папки"
"&Auto change folder"
"&Automaticky měnit adresář"
"Ordner &automatisch wechseln (Baumansicht)"

MConfigSelectFolders
"Пометка &папок"
"Select &folders"
"Vybírat a&dresáře"
"&Ordner auswählen"

MConfigSortFolderExt
"Сортировать имена папок по рас&ширению"
"Sort folder names by e&xtension"
"Řadit adresáře podle přípony"
"Ordner nach Er&weiterung sortieren"

MConfigReverseSort
"Разрешить &обратную сортировку"
"Allow re&verse sort modes"
"Do&volit změnu směru řazení"
"&Umgekehrte Sortiermodi zulassen"

MConfigAutoUpdateLimit
"Отключать автооб&новление панелей,"
"&Disable automatic update of panels"
"Vypnout a&utomatickou aktualizaci panelů"
"Automatisches Panelupdate &deaktivieren"

MConfigAutoUpdateLimit2
"если объектов больше"
"if object count exceeds"
"jestliže počet objektů překročí"
"wenn mehr Objekte als"

MConfigAutoUpdateRemoteDrive
"Автообновление с&етевых дисков"
"Network drives autor&efresh"
"Automatická obnova síťových disků"
"Netzw&erklauferke autom. aktualisieren"

MConfigShowColumns
"Показывать &заголовки колонок"
"Show &column titles"
"Zobrazovat &nadpisy sloupců"
"S&paltentitel anzeigen"

MConfigShowStatus
"Показывать &строку статуса"
"Show &status line"
"Zobrazovat sta&vový řádek"
"&Statuszeile anzeigen"

MConfigShowTotal
"Показывать су&ммарную информацию"
"Show files &total information"
"Zobrazovat &informace o velikosti souborů"
"&Gesamtzahl für Dateien anzeigen"

MConfigShowFree
"Показывать с&вободное место"
"Show f&ree size"
"Zobrazovat vo&lné místo"
"&Freien Speicher anzeigen"

MConfigShowScrollbar
"Показывать по&лосу прокрутки"
"Show scroll&bar"
"Zobrazovat &posuvník"
"Scroll&balken anzeigen"

MConfigShowScreensNumber
"Показывать количество &фоновых экранов"
"Show background screens &number"
"Zobrazovat počet &obrazovek na pozadí"
"&Nummer von Hintergrundseiten anzeigen"

MConfigShowSortMode
"Показывать букву режима сор&тировки"
"Show sort &mode letter"
"Zobrazovat písmeno &módu řazení"
"Buchstaben der Sortier&modi anzeigen"

MConfigInterfaceTitle
l:
"Настройки интерфейса"
"Interface settings"
"Nastavení rozhraní"
"Oberfläche einrichten"

MConfigClock
"&Часы в панелях"
"&Clock in panels"
"&Hodiny v panelech"
"&Uhr in Panels anzeigen"

MConfigViewerEditorClock
"Ч&асы при редактировании и просмотре"
"C&lock in viewer and editor"
"H&odiny v prohlížeči a editoru"
"U&hr in Betrachter und Editor anzeigen"

MConfigMouse
"Мы&шь"
"M&ouse"
"M&yš"
"M&aus aktivieren"

MConfigMousePanelMClickRule
"В панелях ср&едняя кнопка равна Enter"
"Middle &button equals Enter in panels"
"Prostřední tlač. znamená v panelech Enter"
"Mittlere &Taste als Enter in Panels"

MConfigKeyBar
"Показывать &линейку клавиш"
"Show &key bar"
"Zobrazovat &zkratkové klávesy"
"Tast&enleiste anzeigen"

MConfigMenuBar
"Всегда показывать &меню"
"Always show &menu bar"
"Vždy zobrazovat hlavní &menu"
"&Menüleiste immer anzeigen"

MConfigSaver
"&Сохранение экрана"
"&Screen saver"
"Sp&ořič obrazovky"
"Bildschirm&schoner"

MConfigSaverMinutes
"минут"
"minutes"
"minut"
"Minuten"

MConfigUsePromptFormat
"Установить &формат командной строки"
"Set command line &prompt format"
"Nastavit formát &příkazového řádku"
"&Promptformat der Kommandozeile"

MConfigCopyTotal
"Показывать &общий индикатор копирования"
"Show &total copy progress indicator"
"Zobraz. ukazatel celkového stavu &kopírování"
"Zeige Gesamtfor&tschritt beim Kopieren"

MConfigCopyTimeRule
"Показывать информацию о времени &копирования"
"Show cop&ying time information"
"Zobrazovat informace o čase kopírování"
"Zeige Rest&zeit beim Kopieren"

MConfigPgUpChangeDisk
"Использовать Ctrl-PgUp для в&ыбора диска"
"Use Ctrl-Pg&Up to change drive"
"Použít Ctrl-Pg&Up pro změnu disku"
"Strg-Pg&Up wechselt das Laufwerk"

MConfigDlgSetsTitle
l:
"Настройки диалогов"
"Dialog settings"
"Nastavení dialogů"
"Dialoge einrichten"

MConfigDialogsEditHistory
"&История в строках ввода диалогов"
"&History in dialog edit controls"
"H&istorie v dialozích"
"&Historie in Eingabefelder von Dialogen"

MConfigDialogsEditBlock
"&Постоянные блоки в строках ввода"
"&Persistent blocks in edit controls"
"&Trvalé bloky v editačních polích"
"Dauer&hafte Markierungen in Eingabefelder"

MConfigDialogsDelRemovesBlocks
"Del удаляет б&локи в строках ввода"
"&Del removes blocks in edit controls"
"&Del maže položky v editačních polích"
"&Entf löscht Markierungen"

MConfigDialogsAutoComplete
"&Автозавершение в строках ввода"
"&AutoComplete in edit controls"
"Automatické dokončování v editač&ních polích"
"&Automatisches Vervollständigen"

MConfigDialogsEULBsClear
"Backspace &удаляет неизмененный текст"
"&Backspace deletes unchanged text"
"&Backspace maže nezměněný text"
"&Rücktaste (BS) löscht unveränderten Text"

MConfigDialogsMouseButton
"Клик мыши &вне диалога закрывает диалог"
"Mouse click &outside a dialog closes it"
"Kl&iknutí myší mimo dialog ho zavře"
"Dial&og schließen wenn Mausklick ausserhalb"

MViewConfigTitle
l:
"Программа просмотра"
"Viewer"
"Prohlížeč"
"Betrachter"

MViewConfigExternal
"Внешняя программа просмотра:"
"External viewer:"
"Externí prohlížeč"
"Externer Betracher:"

MViewConfigExternalF3
"Запускать по F3"
"Use for F3"
"Použít pro F3"
"Starten mit F3"

MViewConfigExternalAltF3
"Запускать по Alt-F3"
"Use for Alt-F3"
"Použít pro Alt-F3"
"Starten mit Alt-F3"

MViewConfigExternalCommand
"&Команда просмотра:"
"&Viewer command:"
"&Příkaz prohlížeče:"
"Befehl für e&xternen Betracher:"

MViewConfigInternal
"Встроенная программа просмотра:"
"Internal viewer:"
"Interní prohlížeč"
"Interner Betracher:"

MViewConfigSavePos
"&Сохранять позицию файла"
"&Save file position"
"&Ukládat pozici v souboru"
"Dateipositionen &speichern"

MViewConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"
"Ukládat &záložky"
"&Lesezeichen speichern"

MViewAutoDetectTable
"&Автоопределение таблицы символов"
"&Autodetect character table"
"&Autodetekovat znakovou sadu"
"Zeichentabelle &automatisch erkennen"

MViewConfigTabSize
"Раз&мер табуляции"
"Tab si&ze"
"Velikost &Tabulátoru"
"Ta&bulatorgröße"

MViewConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"
"Zobrazovat posu&vník"
"Scro&llbalken anzeigen"

MViewConfigArrows
"Показывать стрелки с&двига"
"Show scrolling arro&ws"
"Zobrazovat &skrolovací šipky"
"P&feile bei Scrollbalken zeigen"

MViewConfigPersistentSelection
"Постоянное &выделение"
"&Persistent selection"
"Trvalé &výběry"
"Dauerhafte Text&markierungen"

MViewConfigAnsiTableAsDefault
"&Изначально открывать файлы в WIN кодировке"
"&Initially open files in WIN encoding"
"Automaticky otevírat soubory ve &WIN kódování"
"Dateien standardmäßig mit Windows-Kod&ierung öffnen"

MEditConfigTitle
l:
"Редактор"
"Editor"
"Editor"
"Editor"

MEditConfigExternal
"Внешний редактор:"
"External editor:"
"Externí editor"
"Externer Editor:"

MEditConfigEditorF4
"Запускать по F4"
"Use for F4"
"Použít pro F4"
"Starten mit F4"

MEditConfigEditorAltF4
"Запускать по Alt-F4"
"Use for Alt-F4"
"Použít pro Alt-F4"
"Starten mit Alt-F4"

MEditConfigEditorCommand
"&Команда редактирования:"
"&Editor command:"
"&Příkaz editoru:"
"Befehl für e&xternen Editor:"

MEditConfigInternal
"Встроенный редактор:"
"Internal editor:"
"Interní editor"
"Interner Editor:"

MEditConfigExpandTabsTitle
"Преобразовывать &табуляцию:"
"Expand &tabs:"
"Rozšířit Ta&bulátory mezerami"
"&Tabs expandieren:"

MEditConfigDoNotExpandTabs
l:
"Не преобразовывать табуляцию"
"Do not expand tabs"
"Nerozšiřovat tabulátory mezerami"
"Tabs nicht expandieren"

MEditConfigExpandTabs
"Преобразовывать новые символы табуляции в пробелы"
"Expand newly entered tabs to spaces"
"Rozšířit nově zadané tabulátory mezerami"
"Neue Tabs zu Leerzeichen expandieren"

MEditConfigConvertAllTabsToSpaces
"Преобразовывать все символы табуляции в пробелы"
"Expand all tabs to spaces"
"Rozšířit všechny tabulátory mezerami"
"Alle Tabs zu Leerzeichen expandieren"

MEditConfigPersistentBlocks
"&Постоянные блоки"
"&Persistent blocks"
"&Trvalé bloky"
"Dauerhafte Text&markierungen"

MEditConfigDelRemovesBlocks
l:
"Del удаляет б&локи"
"&Del removes blocks"
"&Del maže bloky"
"&Entf löscht Textmark."

MEditConfigAutoIndent
"Авто&отступ"
"Auto &indent"
"Auto &Odsazování"
"Automatischer E&inzug"

MEditConfigSavePos
"&Сохранять позицию файла"
"&Save file position"
"&Ukládat pozici v souboru"
"Dateipositionen &speichern"

MEditConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"
"Ukládat zá&ložky"
"&Lesezeichen speichern"

MEditAutoDetectTable
"&Автоопределение таблицы символов"
"&Autodetect character table"
"&Autodetekovat znakovou sadu"
"Zeichentabelle &automatisch erkennen"

MEditCursorBeyondEnd
"Ку&рсор за пределами строки"
"&Cursor beyond end of line"
"&Kurzor za koncem řádku"
"&Cursor hinter dem Ende der Zeile zulassen"

MEditLockROFileModification
"Блокировать р&едактирование файлов с атрибутом R/O"
"Lock editing of read-only &files"
"&Zamknout editaci souborů určených jen pro čtení"
"Bearbeiten von &Dateien mit Schreibschutz verhindern"

MEditWarningBeforeOpenROFile
"Пре&дупреждать при открытии файла с атрибутом R/O"
"&Warn when opening read-only files"
"&Varovat při otevření souborů určených jen pro čtení"
"Beim Öffnen von Dateien mit Schreibschutz &warnen"

MEditConfigTabSize
"Раз&мер табуляции"
"Tab si&ze"
"Velikost &Tabulátoru"
"Ta&bulatorgröße"

MEditConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"
"Zobr&azovat posuvník"
"Scro&llbalken anzeigen"

MEditConfigAnsiTableAsDefault
"&Изначально открывать файлы в WIN кодировке"
"I&nitially open files in WIN encoding"
"Automaticky otevírat soubory ve &WIN kódování"
"Dateien standardmäßig mit Windows-Kod&ierung öffnen"

MEditConfigAnsiTableForNewFile
"Созда&вать новые файлы в WIN кодировке"
"C&reate new files in WIN encoding"
"V&ytvářet nové soubory ve WIN kódování"
"Neue Dateien mit Windows-Ko&dierung erstellen"

MSaveSetupTitle
l:
"Конфигурация"
"Save setup"
"Uložit nastavení"
"Einstellungen speichern"

MSaveSetupAsk1
"Вы хотите сохранить"
"Do you wish to save"
"Přejete si uložit"
"Wollen Sie die aktuellen Einstellungen"

MSaveSetupAsk2
"текущую конфигурацию?"
"current setup?"
"aktuální nastavení?"
"speichern?"

MSaveSetup
"Сохранить"
"Save"
"Uložit"
"Speichern"

MCopyDlgTitle
l:
"Копирование"
"Copy"
"Kopírovat"
"Kopieren"

MMoveDlgTitle
"Переименование/Перенос"
"Rename/Move"
"Přejmenovat/Přesunout"
"Verschieben/Umbenennen"

MLinkDlgTitle
"Ссылка"
"Link"
"Link"
"Link erstellen"

MCopySecurity
"П&рава доступа:"
"&Access rights:"
"&Přístupová práva:"
"Zugriffsrecht&e:"

MCopySecurityCopy
"Копироват&ь"
"Co&py"
"&Kopírovat"
"Ko&pieren"

MCopySecurityInherit
"Нас&ледовать"
"&Inherit"
"&Zdědit"
"Ve&rerben"

MCopySecurityLeave
"По умол&чанию"
"Defau&lt"
"Vých&ozí"
"A&utomat."

MCopyIfFileExist
"Уже су&ществующие файлы:"
"Already e&xisting files:"
"Již e&xistující soubory:"
"&Dateien überschreiben:"

MCopyAsk
"&Запрос действия"
"&Ask"
"Ptát s&e"
"Fr&agen"

MCopyAskRO
"Запрос подтверждения для &R/O файлов"
"Also ask on &R/O files"
"Ptát se také na &R/O soubory"
"Bei Dateien mit Sch&reibschutz fragen"

MCopyOnlyNewerFiles
"Только &новые/обновленные файлы"
"Only ne&wer file(s)"
"Pouze &novější soubory"
"Nur &neuere Dateien"

MLinkType
"&Тип ссылки:"
"Link t&ype:"
"&Typ linku:"
"Linkt&yp:"

MLinkTypeJunction
"&связь каталогов"
"directory &junction"
"křížení a&dresářů"
"Ordner&knotenpunkt"

MLinkTypeHardlink
"&жёсткая ссылка"
"&hard link"
"&pevný link"
"&Hardlink"

MLinkTypeSymlinkFile
"символическая ссылка (&файл)"
"symbolic link (&file)"
"symbolický link (&soubor)"
"Symbolischer Link (&Datei)"

MLinkTypeSymlinkDirectory
"символическая ссылка (&папка)"
"symbolic link (fol&der)"
"symbolický link (&adresář)"
"Symbolischer Link (Or&dner)"

MCopySymLinkContents
"Копировать содерж&имое символических ссылок"
"Cop&y contents of symbolic links"
"Kopírovat obsah sym&bolických linků"
"Inhalte von s&ymbolischen Links kopieren"

MCopyMultiActions
"Обр&абатывать несколько имен файлов"
"Process &multiple destinations"
"&Zpracovat více míst určení"
"&Mehrere Ziele verarbeiten"

MCopyDlgCopy
"&Копировать"
"&Copy"
"&Kopírovat"
"&Kopieren"

MCopyDlgTree
"F10-&Дерево"
"F10-&Tree"
"F10-&Strom"
"F10-&Baum"

MCopyDlgCancel
"&Отменить"
"Ca&ncel"
"&Storno"
"Ab&bruch"

MCopyDlgRename
"&Переименовать"
"&Rename"
"Přej&menovat"
"&Umbenennen"

MCopyDlgLink
"&Создать ссылку"
"&Link"
"&Linkovat"
"Ver&linken"

MCopyDlgTotal
"Всего"
"Total"
"Celkem"
"Gesamt"

MCopyScanning
"Сканирование папок..."
"Scanning folders..."
"Načítání adresářů..."
"Scanne Ordner..."

MCopyPrepareSecury
"Применение прав доступа..."
"Applying access rights..."
"Nastavuji přístupová práva..."
"Anwenden der Zugriffsrechte..."

MCopyUseFilter
"Исполь&зовать фильтр"
"&Use filter"
"P&oužít filtr"
"Ben&utze Filter"

MCopySetFilter
"&Фильтр"
"Filt&er"
"Filt&r"
"Filt&er"

MCopyFile
l:
"Копировать \"%.55s\""
"Copy \"%.55s\""
"Kopírovat \"%.55s\""
"Kopiere \"%.55s\""

MMoveFile
"Переименовать или перенести \"%.55s\""
"Rename or move \"%.55s\""
"Přejmenovat nebo přesunout \"%.55s\""
"Verschiebe \"%.55s\""

MLinkFile
"Создать ссылку \"%.55s\""
"Link \"%.55s\""
"Linkovat \"%.55s\""
"Verlinke \"%.55s\""

MCopyFiles
"Копировать %d элемент%s"
"Copy %d item%s"
"Kopírovat %d polož%s"
"Kopiere %d Objekt%s"

MMoveFiles
"Переименовать или перенести %d элемент%s"
"Rename or move %d item%s"
"Přejmenovat nebo přesunout %d polož%s"
"Verschiebe %d Objekt%s"

MLinkFiles
"Создать ссылки %d элемент%s"
"Link %d item%s"
"Linkovat %d polož%s"
"Verlinke %d Objekt%s"

MCMLTargetTO
" &в:"
" t&o:"
" d&o:"
" na&ch:"

MCMLItems0
""
""
"u"
""

MCMLItemsA
"а"
"s"
"ek"
"e"

MCMLItemsS
"ов"
"s"
"ky"
"e"

MCopyIncorrectTargetList
l:
"Указан некорректный список целей!"
"Incorrect target list!"
"Nesprávný seznam cílů!"
"Ungültige Liste von Zielen!"

MCopyCopyingTitle
l:
"Копирование"
"Copying"
"Kopíruji"
"Kopieren"

MCopyMovingTitle
"Перенос"
"Moving"
"Přesouvám"
"Verschieben"

MCopyCannotFind
l:
"Файл не найден"
"Cannot find the file"
"Nelze nalézt soubor"
"Folgende Datei kann nicht gefunden werden:"

MCannotCopyFolderToItself1
l:
"Нельзя копировать папку"
"Cannot copy the folder"
"Nelze kopírovat adresář"
"Folgender Ordner kann nicht kopiert werden:"

MCannotCopyFolderToItself2
"в саму себя"
"onto itself"
"sám na sebe"
"Ziel und Quelle identisch."

MCannotCopyToTwoDot
l:
"Нельзя копировать файл или папку"
"You may not copy files or folders"
"Nelze kopírovat soubory nebo adresáře"
"Kopieren von Dateien oder Ordnern ist maximal"

MCannotMoveToTwoDot
"Нельзя перемещать файл или папку"
"You may not move files or folders"
"Nelze přesunout soubory nebo adresáře"
"Verschieben von Dateien oder Ordnern ist maximal"

MCannotCopyMoveToTwoDot
"выше корневого каталога"
"higher than the root folder"
"na vyšší úroveň než kořenový adresář"
"bis zum Wurzelverzeichnis möglich."

MCopyCannotCreateFolder
l:
"Ошибка создания папки"
"Cannot create the folder"
"Nelze vytvořit adresář"
"Folgender Ordner kann nicht erstellt werden:"

MCopyCannotChangeFolderAttr
"Невозможно установить атрибуты для папки"
"Failed to set folder attributes"
"Nastavení atributů adresáře selhalo"
"Fehler beim Setzen der Ordnerattribute"

MCopyCannotRenameFolder
"Невозможно переименовать папку"
"Cannot rename the folder"
"Nelze přejmenovat adresář"
"Folgender Ordner kann nicht umbenannt werden:"

MCopyIgnore
"&Игнорировать"
"&Ignore"
"&Ignorovat"
"&Ignorieren"

MCopyIgnoreAll
"Игнорировать &все"
"Ignore &All"
"Ignorovat &vše"
"&Alle ignorieren"

MCopyRetry
"&Повторить"
"&Retry"
"&Opakovat"
"Wiede&rholen"

MCopySkip
"П&ропустить"
"&Skip"
"&Přeskočit"
"Ausla&ssen"

MCopySkipAll
"&Пропустить все"
"S&kip all"
"Př&eskočit vše"
"Alle aus&lassen"

MCopyCancel
"&Отменить"
"&Cancel"
"&Storno"
"Abb&rechen"

MCopyDecrypt
"Рас&шифровать"
"&Decrypt"
"&Dešifrovat"
"Ent&schlüsseln"

MCopyDecryptAll
"&Все"
"Decrypt &all"
"Deš&ifrovat vše"
"Alle e&ntschlüsseln"

MCopyCannotCreateLink
l:
"Ошибка создания ссылки"
"Cannot create the link"
"Nelze vytvořit symbolický link"
"Folgender Link kann nicht erstellt werden:"

MCopyFolderNotEmpty
"Папка назначения должна быть пустой"
"Target folder must be empty"
"Cílový adresář musí být prázdný"
"Zielordner muss leer sein."

MCopyCannotCreateJunctionToFile
"Невозможно создать связь. Файл уже существует:"
"Cannot create junction. The file already exists:"
"Nelze vytvořit křížový odkaz. Soubor již existuje:"
"Knotenpunkt wurde nicht erstellt. Datei existiert bereits:"

MCopyCannotCreateVolMount
l:
"Ошибка монтирования диска"
"Volume mount points error"
"Chyba připojovacích svazků"
"Fehler im Mountpoint des Datenträgers"

MCopyRetrVolFailed
"Невозможно получить информацию о '%s'"
"Retrieving volume name for '%s' failed"
"Získání jména zvazku pro '%s' selhalo"
"Fehler beim Lesen der Datenträgerbezeichnung von '%s'"

MCopyMountVolFailed
"Ошибка при монтировании диска '%s'"
"Attempt to volume mount '%s'"
"Pokus o připojení svazku '%s'"
"Versuch Datenträger '%s' zu aktivieren"

MCopyMountVolFailed2
"на '%s'"
"at '%s' failed"
"na '%s' selhal"
"fehlgeschlagen bei '%s'"

MCopyCannotSupportVolMount
"Функция монтирования дисков не поддерживается"
"Volume mounting is not supported"
"Připojování svazku není podporováno"
"Datenträgeraktivierung wird nicht unterstützt"

MCopyMountName
"disk_%c"
"Disk_%c"
"Disk_%c"
"Disk_%c"

MCannotCopyFileToItself1
l:
"Нельзя копировать файл"
"Cannot copy the file"
"Nelze kopírovat soubor"
"Folgende Datei kann nicht kopiert werden:"

MCannotCopyFileToItself2
"в самого себя"
"onto itself"
"sám na sebe"
"Ziel und Quelle identisch."

MCopyStream1
l:
"Исходный файл содержит более одного потока данных,"
"The source file contains more than one data stream."
"Zdrojový soubor obsahuje více než jeden datový proud."
"Die Quelldatei enthält mehr als einen Datenstream"

MCopyStream2
"но вы не используете системную функцию копирования."
"but since you do not use a system copy routine."
"protože nepoužíváte systémovou kopírovací rutinu."
"aber Sie verwenden derzeit nicht die systemeigene Kopierroutine."

MCopyStream3
"но том назначения не поддерживает этой возможности."
"but the destination volume does not support this feature."
"protože cílový svazek nepodporuje tuto vlastnost."
"aber der Zieldatenträger unterstützt diese Fähigkeit nicht."

MCopyStream4
"Часть сведений не будет сохранена."
"Some data will not be preserved as a result."
"To bude mít za následek, že některá data nebudou uchována."
"Ein Teil der Daten bleiben daher nicht erhalten."

MCopyFileExist
l:
"Файл уже существует"
"File already exists"
"Soubor již existuje"
"Datei existiert bereits"

MCopySource
"&Новый"
"&New"
"&Nový"
"&Neue Datei"

MCopyDest
"Су&ществующий"
"E&xisting"
"E&xistující"
"Be&stehende Datei"

MCopyOverwrite
"В&место"
"&Overwrite"
"&Přepsat"
"Über&schr."

MCopyContinue
"П&родолжить"
"C&ontinue"
"P&okračovat"
"F&ortsetzen"

MCopySkipOvr
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&spr."

MCopyAppend
"&Дописать"
"A&ppend"
"Př&ipojit"
"&Anhängen"

MCopyResume
"Возоб&новить"
"&Resume"
"Po&kračovat"
"&Weiter"

MCopyCancelOvr
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"

MCopyRememberChoice
"&Запомнить выбор"
"&Remember choice"
"Zapama&tovat volbu"
"Auswahl me&rken"

MCopyFileRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"
"Soubor je určen pouze pro čtení"
"Folgende Datei ist schreibgeschützt:"

MCopyAskDelete
"Вы хотите удалить его?"
"Do you wish to delete it?"
"Opravdu si ho přejete smazat?"
"Wollen Sie sie dennoch löschen?"

MCopyDeleteRO
"&Удалить"
"&Delete"
"S&mazat"
"&Löschen"

MCopyDeleteAllRO
"&Все"
"&All"
"&Vše"
"&Alle Löschen"

MCopySkipRO
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&springen"

MCopySkipAllRO
"П&ропустить все"
"S&kip all"
"Př&eskočit vše"
"A&lle überspringen"

MCopyCancelRO
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"

MCannotCopy
l:
"Ошибка копирования"
"Cannot copy"
"Nelze kopírovat"
"Konnte nicht kopieren"

MCannotMove
"Ошибка переноса"
"Cannot move"
"Nelze přesunout"
"Konnte nicht verschieben"

MCannotLink
"Ошибка создания ссылки"
"Cannot link"
"Nelze linkovat"
"Konnte nicht verlinken"

MCannotCopyTo
"в"
"to"
"do"
"nach"

MCopyEncryptWarn1
"Файл"
"The file"
"Soubor"
"Die Datei"

MCopyEncryptWarn2
"нельзя скопировать или переместить, не потеряв его шифрование."
"cannot be copied or moved without losing its encryption."
"nemůže být zkopírován nebo přesunut bez ztráty jeho šifrování."
"kann nicht bewegt werden ohne ihre Verschlüsselung zu verlieren."

MCopyEncryptWarn3
"Можно пропустить эту ошибку или отменить операцию."
"You can choose to ignore this error and continue, or cancel."
"Můžete tuto chybu ignorovat a pokračovat, nebo operaci ukončit."
"Sie können dies ignorieren und fortfahren oder abbrechen."

MCopyReadError
l:
"Ошибка чтения данных из"
"Cannot read data from"
"Nelze číst data z"
"Kann Daten nicht lesen von"

MCopyWriteError
"Ошибка записи данных в"
"Cannot write data to"
"Nelze zapsat data do"
"Dann Daten nicht schreiben in"

MCopyProcessed
l:
"Обработано файлов: %d"
"Files processed: %d"
"Zpracováno souborů: %d"
"Dateien varbeitet: %d"

MCopyProcessedTotal
"Обработано файлов: %d из %d"
"Files processed: %d of %d"
"Zpracováno souborů: %d z %d"
"Dateien varbeitet: %d von %d"

MCopyMoving
"Перенос файла"
"Moving the file"
"Přesunuji soubor"
"Verschiebe die Datei"

MCopyCopying
"Копирование файла"
"Copying the file"
"Kopíruji soubor"
"Kopiere die Datei"

MCopyTo
"в"
"to"
"do"
"nach"

MCopyErrorDiskFull
l:
"Диск заполнен. Вставьте следующий"
"Disk full. Insert next"
"Disk je plný. Vložte dalšíí"
"Datenträger voll. Bitte nächsten einlegen"

MDeleteTitle
l:
"Удаление"
"Delete"
"Smazat"
"Löschen"

MAskDeleteFolder
"Вы хотите удалить папку"
"Do you wish to delete the folder"
"Přejete si smazat adresář"
"Wollen Sie den Ordner löschen"

MAskDeleteFile
"Вы хотите удалить файл"
"Do you wish to delete the file"
"Přejete si smazat soubor"
"Wollen Sie die Datei löschen"

MAskDelete
"Вы хотите удалить"
"Do you wish to delete"
"Přejete si smazat"
"Wollen Sie folgendes Objekt löschen"

MAskDeleteRecycleFolder
"Вы хотите поместить в Корзину папку"
"Do you wish to move to the Recycle Bin the folder"
"Přejete si přesunout do Koše adresář"
"Wollen Sie den Ordner in den Papierkorb verschieben"

MAskDeleteRecycleFile
"Вы хотите поместить в Корзину файл"
"Do you wish to move to the Recycle Bin the file"
"Přejete si přesunout do Koše soubor"
"Wollen Sie die Datei in den Papierkorb verschieben"

MAskDeleteRecycle
"Вы хотите поместить в Корзину"
"Do you wish to move to the Recycle Bin"
"Přejete si přesunout do Koše"
"Wollen Sie das Objekt in den Papierkorb verschieben"

MDeleteWipeTitle
"Уничтожение"
"Wipe"
"Vymazat"
"Sicheres Löschen"

MAskWipeFolder
"Вы хотите уничтожить папку"
"Do you wish to wipe the folder"
"Přejete si vymazat adresář"
"Wollen Sie den Ordner sicher löschen"

MAskWipeFile
"Вы хотите уничтожить файл"
"Do you wish to wipe the file"
"Přejete si vymazat soubor"
"Wollen Sie die Datei sicher löschen"

MAskWipe
"Вы хотите уничтожить"
"Do you wish to wipe"
"Přejete si vymazat"
"Wollen Sie das Objekt sicher löschen"

MDeleteLinkTitle
"Удаление ссылки"
"Delete link"
"Smazat link"
"Link löschen"

MAskDeleteLink
"является ссылкой на"
"is a symbolic link to"
"je symbolicky link na"
"ist ein symbolischer Link auf"

MAskDeleteLinkFolder
"папку"
"folder"
"adresář"
"Ordner"

MAskDeleteLinkFile
"файл"
"file"
"soubor"
"Date"

MAskDeleteItems
"%d элемент%s"
"%d item%s"
"%d polož%s"
"%d Objekt%s"

MAskDeleteItems0
""
""
"ku"
""

MAskDeleteItemsA
"а"
"s"
"ky"
"e"

MAskDeleteItemsS
"ов"
"s"
"ek"
"e"

MDeleteFolderTitle
l:
"Удаление папки "
"Delete folder"
"Smazat adresář"
"Ordner löschen"

MWipeFolderTitle
"Уничтожение папки "
"Wipe folder"
"Vymazat adresář"
"Ordner sicher löschen"

MDeleteFilesTitle
"Удаление файлов"
"Delete files"
"Smazat soubory"
"Dateien löschen"

MWipeFilesTitle
"Уничтожение файлов"
"Wipe files"
"Vymazat soubory"
"Dateien sicher löschen"

MDeleteFolderConfirm
"Данная папка будет удалена:"
"The following folder will be deleted:"
"Následující adresář bude smazán:"
"Folgender Ordner wird gelöscht:"

MWipeFolderConfirm
"Данная папка будет уничтожена:"
"The following folder will be wiped:"
"Následující adresář bude vymazán:"
"Folgender Ordner wird sicher gelöscht:"

MDeleteWipe
"Уничтожить"
"Wipe"
"Vymazat"
"Sicheres Löschen"

MDeleteFileDelete
"&Удалить"
"&Delete"
"S&mazat"
"&Löschen"

MDeleteFileWipe
"&Уничтожить"
"&Wipe"
"V&ymazat"
"&Sicher löschen"

MDeleteFileAll
"&Все"
"&All"
"&Vše"
"&Alle"

MDeleteFileSkip
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&springen"

MDeleteFileSkipAll
"П&ропустить все"
"S&kip all"
"Př&eskočit vše"
"A&lle überspr."

MDeleteFileCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"

MDeleteLinkDelete
l:
"Удалить ссылку"
"Delete link"
"Smazat link"
"Link löschen"

MDeleteLinkUnlink
"Разорвать ссылку"
"Break link"
"Poškozený link"
"Link auflösen"

MDeletingTitle
l:
"Удаление"
"Deleting"
"Mazání"
"Lösche"

MDeleting
l:
"Удаление файла или папки"
"Deleting the file or folder"
"Mazání souboru nebo adresáře"
"Löschen von Datei oder Ordner"

MDeletingWiping
"Уничтожение файла или папки"
"Wiping the file or folder"
"Vymazávání souboru nebo adresáře"
"Sicheres löschen von Datei oder Ordner"

MDeleteRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"
"Soubor je určen pouze pro čtení"
"Folgende Datei ist schreibgeschützt:"

MAskDeleteRO
"Вы хотите удалить его?"
"Do you wish to delete it?"
"Opravdu si ho přejete smazat?"
"Wollen Sie sie dennoch löschen?"

MAskWipeRO
"Вы хотите уничтожить его?"
"Do you wish to wipe it?"
"Opravdu si ho přejete vymazat?"
"Wollen Sie sie dennoch sicher löschen?"

MDeleteHardLink1
l:
"Файл имеет несколько жестких ссылок"
"Several hard links link to this file."
"Více pevných linků ukazuje na tento soubor."
"Mehrere Hardlinks zeigen auf diese Datei."

MDeleteHardLink2
"Уничтожение файла приведет к обнулению всех ссылающихся на него файлов."
"Wiping this file will void all files linking to it."
"Vymazání tohoto souboru zneplatní všechny soubory, které na něj linkují."
"Sicheres Löschen dieser Datei entfernt ebenfalls alle Links."

MDeleteHardLink3
"Уничтожать файл?"
"Do you wish to wipe this file?"
"Opravdu chcete vymazat tento soubor?"
"Wollen Sie diese Datei sicher löschen?"

MCannotDeleteFile
l:
"Ошибка удаления файла"
"Cannot delete the file"
"Nelze smazat soubor"
"Datei konnte nicht gelöscht werden"

MCannotDeleteFolder
"Ошибка удаления папки"
"Cannot delete the folder"
"Nelze smazat adresář"
"Ordner konnte nicht gelöscht werden"

MDeleteRetry
"&Повторить"
"&Retry"
"&Znovu"
"Wiede&rholen"

MDeleteSkip
"П&ропустить"
"&Skip"
"Přes&kočit"
"Über&springen"

MDeleteSkipAll
"Пропустить &все"
"S&kip all"
"Přeskočit &vše"
"A&lle überspr."

MDeleteCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"

MCannotGetSecurity
l:
"Ошибка получения прав доступа к файлу"
"Cannot get file access rights for"
"Nemohu získat přístupová práva pro"
"Kann Zugriffsrechte nicht lesen für"

MCannotSetSecurity
"Ошибка установки прав доступа к файлу"
"Cannot set file access rights for"
"Nemohu nastavit přístupová práva pro"
"Kann Zugriffsrechte nicht setzen für"

MEditTitle
l:
"Редактор"
"Editor"
"Editor"
"Editor"

MAskReload
"уже загружен. Как открыть этот файл?"
"already loaded. How to open this file?"
"již otevřen. Jak otevřít tento soubor?"
"bereits geladen. Wie wollen Sie die Datei öffnen?"

MCurrent
"&Текущий"
"&Current"
"&Stávající"
"A&ktuell"

MReload
"Пере&грузить"
"R&eload"
"&Znovu načíst"
"Aktualisie&ren"

MNewOpen
"&Новая копия"
"&New instance"
"&Nová instance"
"&Neue Instanz"

MEditCannotOpen
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"

MEditReading
"Чтение файла"
"Reading the file"
"Načítám soubor"
"Lesen der Datei"

MEditAskSave
"Файл был изменен"
"File has been modified"
"Soubor byl modifikován"
"Datei wurde verändert"

MEditAskSaveExt
"Файл был изменен внешней программой"
"The file was changed by an external program"
"Soubor byl změněný externím programem"
"Die Datei wurde durch ein externes Programm verändert"

MEditSave
l:
"&Сохранить"
"&Save"
"&Uložit"
"&Speichern"

MEditNotSave
"&Не сохранять"
"Do &not save"
"&Neukládat"
"&Nicht speichern"

MEditContinue
"&Продолжить редактирование"
"&Continue editing"
"&Pokračovat"
"Bearbeiten f&ortsetzen"

MEditBtnSaveAs
"Сохр&анить как"
"Save &as..."
"Ulož&it jako..."
"Speichern &als..."

MEditRO
l:
"имеет атрибут \"Только для чтения\""
"is a read-only file"
"je určen pouze pro čtení"
"ist eine schreibgeschützte Datei"

MEditExists
"уже существует"
"already exists"
"již existuje"
"ist bereits vorhanden"

MEditOvr
"Вы хотите перезаписать его?"
"Do you wish to overwrite it?"
"Přejete si ho přepsat?"
"Wollen Sie die Datei überschreiben?"

MEditSaving
"Сохранение файла"
"Saving the file"
"Ukládám soubor"
"Speichere die Datei"

MEditStatusLine
"Строка"
"Line"
"Řádek"
"Zeile"

MEditStatusCol
"Кол"
"Col"
"Sloupec"
"Spal"

MEditRSH
l:
"предназначен только для чтения"
"is a read-only file"
"je určen pouze pro čtení"
"ist eine schreibgeschützte Datei"

MEditFileLong
"имеет размер %s,"
"has the size of %s,"
"má velikost %s,"
"hat eine Größe von %s,"

MEditFileLong2
"что превышает заданное ограничение в %s."
"which exceeds the configured maximum size of %s."
"která překračuje nastavenou maximální velikost %s."
"die die konfiguierte Maximalgröße von %s überschreitet."

MEditROOpen
"Вы хотите редактировать его?"
"Do you wish to edit it?"
"Opravdu si ho přejete upravit?"
"Wollen Sie sie dennoch bearbeiten?"

MEditCanNotEditDirectory
l:
"Невозможно редактировать папку"
"It is impossible to edit the folder"
"Nelze editovat adresář"
"Es ist nicht möglich den Ordner zu bearbeiten"

MEditSearchTitle
l:
"Поиск"
"Search"
"Hledat"
"Suchen"

MEditSearchFor
"&Искать"
"&Search for"
"&Hledat"
"&Suchen nach"

MEditSearchCase
"&Учитывать регистр"
"&Case sensitive"
"&Rozlišovat velikost písmen"
"G&roß-/Kleinschrb."

MEditSearchWholeWords
"Только &целые слова"
"&Whole words"
"&Celá slova"
"&Ganze Wörter"

MEditSearchReverse
"Обратн&ый поиск"
"Re&verse search"
"&Zpětné hledání"
"Richtung um&kehren"

MEditSearchSelFound
"&Выделять найденное"
"Se&lect found"
"Vy&ber nalezené"
"Treffer &markieren"

MEditSearchSearch
"Искать"
"Search"
"Hledat"
"Suchen"

MEditSearchCancel
"Отменить"
"Cancel"
"Storno"
"Abbruch"

MEditReplaceTitle
l:
"Замена"
"Replace"
"Nahradit"
"Ersetzen"

MEditReplaceWith
"Заменить &на"
"R&eplace with"
"Nahradit &s"
"&Ersetzen mit"

MEditReplaceReplace
"&Замена"
"&Replace"
"&Nahradit"
"E&rsetzen"

MEditSearchingFor
l:
"Искать"
"Searching for"
"Vyhledávám"
"Suche nach"

MEditNotFound
"Строка не найдена"
"Could not find the string"
"Nemůžu najít řetězec"
"Konnte Zeichenkette nicht finden"

MEditAskReplace
l:
"Заменить"
"Replace"
"Nahradit"
"Ersetze"

MEditAskReplaceWith
"на"
"with"
"s"
"mit"

MEditReplace
"&Заменить"
"&Replace"
"&Nahradit"
"E&rsetzen"

MEditReplaceAll
"&Все"
"&All"
"&Vše"
"&Alle"

MEditSkip
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&springen"

MEditCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"

MEditOpenCreateLabel
"Открыть/создать файл:"
"Open/create file:"
"Otevřít/vytvořit soubor:"
"Öffnen/datei erstellen:"

MEditGoToLine
l:
"Перейти"
"Go to position"
"Jít na pozici"
"Gehe zu Zeile"

MFolderShortcutsTitle
l:
"Ссылки на папки"
"Folder shortcuts"
"Adresářové zkratky"
"Ordnerschnellzugriff"

MFolderShortcutBottom
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Bearb.: Entf,Einf,F4"

MShortcutNone
"<отсутствует>"
"<none>"
"<není>"
"<keiner>"

MShortcutPlugin
"<плагин>"
"<plugin>"
"<plugin>"
"<Plugin>"

MEnterShortcut
"Введите новую ссылку:"
"Enter new shortcut:"
"Zadejte novou zkratku:"
"Neue Verknüpfung:"

MNeedNearPath
"Перейти в ближайшую доступную папку?"
"Jump to the nearest existing folder?"
"Skočit na nejbližší existující adresář?"
"Zum nahesten existierenden Ordner springen?"

MSaveThisShortcut
"Запомнить эту ссылку?"
"Save this shortcuts?"
"Uložit tyto zkratky?"
"Verknüpfung speichern?"

MEditF1
l:
l://functional keys - 6 characters max, 12 keys, "OEM" is F8 dupe!
"Помощь"
"Help"
"Pomoc"
"Hilfe"

MEditF2
"Сохран"
"Save"
"Uložit"
"Speich"

MEditF3
""
""
""
""

MEditF4
""
""
""
""

MEditF5
""
""
""
""

MEditF6
"Просм"
"View"
"Zobraz"
"Betr."

MEditF7
"Поиск"
"Search"
"Hledat"
"Suchen"

MEditF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MEditF9
""
""
""
""

MEditF10
"Выход"
"Quit"
"Konec"
"Ende"

MEditF11
"Модули"
"Plugin"
"Plugin"
"Plugin"

MEditF12
"Экраны"
"Screen"
"Obraz."
"Seiten"

MEditF8DOS
le:// don't count this - it's a F8 another text
"OEM"
"OEM"
"OEM"
"OEM"

MEditShiftF1
l:
l://Editor: Shift
""
""
""
""

MEditShiftF2
"Сохр.в"
"SaveAs"
"UlJako"
"SpeiUn"

MEditShiftF3
""
""
""
""

MEditShiftF4
"Редак."
"Edit.."
"Edit.."
"Bear.."

MEditShiftF5
""
""
""
""

MEditShiftF6
""
""
""
""

MEditShiftF7
"Дальше"
"Next"
"Další"
"Nächst"

MEditShiftF8
"Таблиц"
"Table"
"ZnSady"
"Tabell"

MEditShiftF9
""
""
""
""

MEditShiftF10
"СхрВых"
"SaveQ"
"UlKone"
"SaveQ"

MEditShiftF11
""
""
""
""

MEditShiftF12
""
""
""
""

MEditAltF1
l:
l://Editor: Alt
""
""
""
""

MEditAltF2
""
""
""
""

MEditAltF3
""
""
""
""

MEditAltF4
""
""
""
""

MEditAltF5
"Печать"
"Print"
"Tisk"
"Druck"

MEditAltF6
""
""
""
""

MEditAltF7
"Назад"
"Prev"
"Předch"
"Letzt"

MEditAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"

MEditAltF9
"Видео"
"Video"
"Video"
"Ansich"

MEditAltF10
""
""
""
""

MEditAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"

MEditAltF12
""
""
""
""

MEditCtrlF1
l:
l://Editor: Ctrl
""
""
""
""

MEditCtrlF2
""
""
""
""

MEditCtrlF3
""
""
""
""

MEditCtrlF4
""
""
""
""

MEditCtrlF5
""
""
""
""

MEditCtrlF6
""
""
""
""

MEditCtrlF7
"Замена"
"Replac"
"Nahraď"
"Ersetz"

MEditCtrlF8
""
""
""
""

MEditCtrlF9
""
""
""
""

MEditCtrlF10
"Позиц"
"GoFile"
"JdiSou"
"GehDat"

MEditCtrlF11
""
""
""
""

MEditCtrlF12
""
""
""
""

MEditAltShiftF1
l:
l://Editor: AltShift
""
""
""
""

MEditAltShiftF2
""
""
""
""

MEditAltShiftF3
""
""
""
""

MEditAltShiftF4
""
""
""
""

MEditAltShiftF5
""
""
""
""

MEditAltShiftF6
""
""
""
""

MEditAltShiftF7
""
""
""
""

MEditAltShiftF8
""
""
""
""

MEditAltShiftF9
"Конфиг"
"Config"
"Nastav"
"Konfig"

MEditAltShiftF10
""
""
""
""

MEditAltShiftF11
""
""
""
""

MEditAltShiftF12
""
""
""
""

MEditCtrlShiftF1
l:
l://Editor: CtrlShift
""
""
""
""

MEditCtrlShiftF2
""
""
""
""

MEditCtrlShiftF3
""
""
""
""

MEditCtrlShiftF4
""
""
""
""

MEditCtrlShiftF5
""
""
""
""

MEditCtrlShiftF6
""
""
""
""

MEditCtrlShiftF7
""
""
""
""

MEditCtrlShiftF8
""
""
""
""

MEditCtrlShiftF9
""
""
""
""

MEditCtrlShiftF10
""
""
""
""

MEditCtrlShiftF11
""
""
""
""

MEditCtrlShiftF12
""
""
""
""

MEditCtrlAltF1
l:
l:// Editor: CtrlAlt
""
""
""
""

MEditCtrlAltF2
""
""
""
""

MEditCtrlAltF3
""
""
""
""

MEditCtrlAltF4
""
""
""
""

MEditCtrlAltF5
""
""
""
""

MEditCtrlAltF6
""
""
""
""

MEditCtrlAltF7
""
""
""
""

MEditCtrlAltF8
""
""
""
""

MEditCtrlAltF9
""
""
""
""

MEditCtrlAltF10
""
""
""
""

MEditCtrlAltF11
""
""
""
""

MEditCtrlAltF12
""
""
""
""

MEditCtrlAltShiftF1
l:
l:// Editor: CtrlAltShift
""
""
""
""

MEditCtrlAltShiftF2
""
""
""
""

MEditCtrlAltShiftF3
""
""
""
""

MEditCtrlAltShiftF4
""
""
""
""

MEditCtrlAltShiftF5
""
""
""
""

MEditCtrlAltShiftF6
""
""
""
""

MEditCtrlAltShiftF7
""
""
""
""

MEditCtrlAltShiftF8
""
""
""
""

MEditCtrlAltShiftF9
""
""
""
""

MEditCtrlAltShiftF10
""
""
""
""

MEditCtrlAltShiftF11
""
""
""
""

MEditCtrlAltShiftF12
le://End of functional keys (Editor)
""
""
""
""

MSingleEditF1
l:
l://Single Editor: functional keys - 6 characters max, 12 keys, "OEM" is F8 dupe!
"Помощь"
"Help"
"Pomoc"
"Hilfe"

MSingleEditF2
"Сохран"
"Save"
"Uložit"
"Speich"

MSingleEditF3
""
""
""
""

MSingleEditF4
""
""
""
""

MSingleEditF5
""
""
""
""

MSingleEditF6
"Просм"
"View"
"Zobraz"
"Betr."

MSingleEditF7
"Поиск"
"Search"
"Hledat"
"Suchen"

MSingleEditF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MSingleEditF9
""
""
""
""

MSingleEditF10
"Выход"
"Quit"
"Konec"
"Ende"

MSingleEditF11
"Модули"
"Plugin"
"Plugin"
"Plugin"

MSingleEditF12
"Экраны"
"Screen"
"Obraz."
"Seiten"

MSingleEditF8DOS
le:// don't count this - it's a F8 another text
"OEM"
"OEM"
"OEM"
"OEM"

MSingleEditShiftF1
l:
l://Single Editor: Shift
""
""
""
""

MSingleEditShiftF2
"Сохр.в"
"SaveAs"
"UlJako"
"SpeiUn"

MSingleEditShiftF3
""
""
""
""

MSingleEditShiftF4
""
""
""
""

MSingleEditShiftF5
""
""
""
""

MSingleEditShiftF6
""
""
""
""

MSingleEditShiftF7
"Дальше"
"Next"
"Další"
"Nächst"

MSingleEditShiftF8
"Таблиц"
"Table"
"ZnSady"
"Tabell"

MSingleEditShiftF9
""
""
""
""

MSingleEditShiftF10
"СхрВых"
"SaveQ"
"UlKone"
"SaveQ"

MSingleEditShiftF11
""
""
""
""

MSingleEditShiftF12
""
""
""
""

MSingleEditAltF1
l:
l://Single Editor: Alt
""
""
""
""

MSingleEditAltF2
""
""
""
""

MSingleEditAltF3
""
""
""
""

MSingleEditAltF4
""
""
""
""

MSingleEditAltF5
"Печать"
"Print"
"Tisk"
"Druck"

MSingleEditAltF6
""
""
""
""

MSingleEditAltF7
""
""
""
""

MSingleEditAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"

MSingleEditAltF9
"Видео"
"Video"
"Video"
"Ansich"

MSingleEditAltF10
""
""
""
""

MSingleEditAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"

MSingleEditAltF12
""
""
""
""

MSingleEditCtrlF1
l:
l://Single Editor: Ctrl
""
""
""
""

MSingleEditCtrlF2
""
""
""
""

MSingleEditCtrlF3
""
""
""
""

MSingleEditCtrlF4
""
""
""
""

MSingleEditCtrlF5
""
""
""
""

MSingleEditCtrlF6
""
""
""
""

MSingleEditCtrlF7
"Замена"
"Replac"
"Nahraď"
"Ersetz"

MSingleEditCtrlF8
""
""
""
""

MSingleEditCtrlF9
""
""
""
""

MSingleEditCtrlF10
""
""
""
""

MSingleEditCtrlF11
""
""
""
""

MSingleEditCtrlF12
""
""
""
""

MSingleEditAltShiftF1
l:
l://Single Editor: AltShift
""
""
""
""

MSingleEditAltShiftF2
""
""
""
""

MSingleEditAltShiftF3
""
""
""
""

MSingleEditAltShiftF4
""
""
""
""

MSingleEditAltShiftF5
""
""
""
""

MSingleEditAltShiftF6
""
""
""
""

MSingleEditAltShiftF7
""
""
""
""

MSingleEditAltShiftF8
""
""
""
""

MSingleEditAltShiftF9
"Конфиг"
"Config"
"Nastav"
"Konfig"

MSingleEditAltShiftF10
""
""
""
""

MSingleEditAltShiftF11
""
""
""
""

MSingleEditAltShiftF12
""
""
""
""

MSingleEditCtrlShiftF1
l:
l://Single Editor: CtrlShift
""
""
""
""

MSingleEditCtrlShiftF2
""
""
""
""

MSingleEditCtrlShiftF3
""
""
""
""

MSingleEditCtrlShiftF4
""
""
""
""

MSingleEditCtrlShiftF5
""
""
""
""

MSingleEditCtrlShiftF6
""
""
""
""

MSingleEditCtrlShiftF7
""
""
""
""

MSingleEditCtrlShiftF8
""
""
""
""

MSingleEditCtrlShiftF9
""
""
""
""

MSingleEditCtrlShiftF10
""
""
""
""

MSingleEditCtrlShiftF11
""
""
""
""

MSingleEditCtrlShiftF12
""
""
""
""

MSingleEditCtrlAltF1
l:
l://Single Editor: CtrlAlt
""
""
""
""

MSingleEditCtrlAltF2
""
""
""
""

MSingleEditCtrlAltF3
""
""
""
""

MSingleEditCtrlAltF4
""
""
""
""

MSingleEditCtrlAltF5
""
""
""
""

MSingleEditCtrlAltF6
""
""
""
""

MSingleEditCtrlAltF7
""
""
""
""

MSingleEditCtrlAltF8
""
""
""
""

MSingleEditCtrlAltF9
""
""
""
""

MSingleEditCtrlAltF10
""
""
""
""

MSingleEditCtrlAltF11
""
""
""
""

MSingleEditCtrlAltF12
""
""
""
""

MSingleEditCtrlAltShiftF1
l:
l://Single Editor: CtrlAltShift
""
""
""
""

MSingleEditCtrlAltShiftF2
""
""
""
""

MSingleEditCtrlAltShiftF3
""
""
""
""

MSingleEditCtrlAltShiftF4
""
""
""
""

MSingleEditCtrlAltShiftF5
""
""
""
""

MSingleEditCtrlAltShiftF6
""
""
""
""

MSingleEditCtrlAltShiftF7
""
""
""
""

MSingleEditCtrlAltShiftF8
""
""
""
""

MSingleEditCtrlAltShiftF9
""
""
""
""

MSingleEditCtrlAltShiftF10
""
""
""
""

MSingleEditCtrlAltShiftF11
""
""
""
""

MSingleEditCtrlAltShiftF12
le://End of functional keys (Single Editor)
""
""
""
""

MEditSaveAs
l:
"Сохранить &файл как"
"Save file &as"
"Uložit soubor jako"
"Speichern &als"

MEditCodePage
"Кодовая страница:"
"Code page:"
"Kódová stránka:"
"Codepage:"

MEditAddSignature
"Добавить сигнатуру (BOM)"
"Add signature (BOM)"
"Přidat signaturu (BOM)"
"Sinatur hinzu (BOM)"

MEditSaveAsFormatTitle
"Изменить перевод строки:"
"Change line breaks to:"
"Změnit zakončení řádků na:"
"Zeilenumbrüche setzen:"

MEditSaveOriginal
"&исходный формат"
"Do n&ot change"
"&Beze změny"
"Nicht verä&ndern"

MEditSaveDOS
"в форма&те DOS/Windows (CR LF)"
"&Dos/Windows format (CR LF)"
"&Dos/Windows formát (CR LF)"
"&Dos/Windows Format (CR LF)"

MEditSaveUnix
"в формат&е UNIX (LF)"
"&Unix format (LF)"
"&Unix formát (LF)"
"&Unix Format (LF)"

MEditSaveMac
"в фор&мате MAC (CR)"
"&Mac format (CR)"
"&Mac formát (CR)"
"&Mac Format (CR)"

MEditCannotSave
"Ошибка сохранения файла"
"Cannot save the file"
"Nelze uložit soubor"
"Kann die Datei nicht speichern"

MEditSavedChangedNonFile
"Файл изменен, но файл или папка, в которой он находился,"
"The file is changed but the file or the folder containing"
"Soubor je změněn, ale soubor, nebo adresář obsahující"
"Inhalt dieser Datei wurde verändert aber die Datei oder der Ordner, welche"

MEditSavedChangedNonFile1
"Файл или папка, в которой он находился,"
"The file or the folder containing"
"Soubor nebo adresář obsahující"
"Die Datei oder der Ordner, welche"

MEditSavedChangedNonFile2
"был перемещен или удален."
"this file was moved or deleted."
"tento soubor byl přesunut, nebo smazán."
"diesen Inhalt enthält wurde verschoben oder gelöscht."

MEditNewPath1
"Путь к редактируемому файлу не существует,"
"The path to the edited file does not exist,"
"Cesta k editovanému souboru neexistuje,"
"Der Pfad zur bearbeiteten Datei existiert nicht,"

MEditNewPath2
"но будет создан при сохранении файла."
"but will be created when the file is saved."
"ale bude vytvořena při uložení souboru."
"aber wird erstellt sobald die Datei gespeichert wird."

MEditNewPath3
"Продолжать?"
"Continue?"
"Pokračovat?"
"Fortsetzen?"

MEditNewPlugin1
"Имя редактируемого файла не может быть пустым"
"The name of the file to edit cannot be empty"
"Název souboru k editaci nesmí být prázdné"
"Der Name der zu editierenden Datei kann nicht leer sein"

MEditDataLostWarn1
"Файл содержит символы, которые невозможно"
"This file contains characters, which cannot be"
"Tento soubor obsahuje znaky, které nemohou být"
"Die Datei enthält Zeichen, welche mit der aktuellen Codepage"

MEditDataLostWarn2
"корректно преобразовать в выбранную кодировку."
"correctly translated using the selected codepage."
"korektně přeloženy do zvoleného kódování."
"nicht korrekt angezeigt werden können."

MEditDataLostWarn3
"Продолжить?"
"Continue?"
"Pokračovat?"
"Fortsetzen?"

MEditDataLostWarn4
"Сохранять файл не рекомендуется."
"It is not recommended to save this file."
"Není doporučeno uložit tento soubor."
"Es wird empfohlen, die Datei nicht zu speichern."

MColumnName
l:
"Имя"
"Name"
"Název"
"Name"

MColumnSize
"Размер"
"Size"
"Velikost"
"Größe"

MColumnPacked
"Упаков"
"Packed"
"Komprimovaný"
"Kompr."

MColumnDate
"Дата"
"Date"
"Datum"
"Datum"

MColumnTime
"Время"
"Time"
"Čas"
"Zeit"

MColumnModified
"Модификация"
"Modified"
"Modifikován"
"Verändert"

MColumnCreated
"Создание"
"Created"
"Vytvořen"
"Erstellt"

MColumnAccessed
"Доступ"
"Accessed"
"Přístup"
"Zugriff"

MColumnAttr
"Атриб"
"Attr"
"Attr"
"Attr"

MColumnDescription
"Описание"
"Description"
"Popis"
"Beschreibung"

MColumnOwner
"Владелец"
"Owner"
"Vlastník"
"Besitzer"

MColumnMumLinks
"КлС"
"NmL"
"PočLn"
"AnL"

MListUp
l:
"Вверх"
"  Up  "
"Nahoru"
" Hoch "

MListFolder
"Папка"
"Folder"
"Adresář"
"Ordner"

MListSymLink
"Ссылка"
"Symlink"
"Link"
"Symlink"

MListJunction
"Связь"
"Junction"
"Křížení"
"Knoten"

MListBytes
"Б"
"B"
"B"
"B"

MListKb
"К"
"K"
"K"
"K"

MListMb
"М"
"M"
"M"
"M"

MListGb
"Г"
"G"
"G"
"G"

MListTb
"Т"
"T"
"T"
"T"

MListFileSize
" %s байт в 1 файле "
" %s bytes in 1 file "
" %s bytů v 1 souboru "
" %s Bytes in 1 Datei "

MListFilesSize1
" %s байт в %d файле "
" %s bytes in %d files "
" %s bytů v %d souborech "
" %s Bytes in %d Dateien "

MListFilesSize2
" %s байт в %d файлах "
" %s bytes in %d files "
" %s bytů v %d souborech "
" %s Bytes in %d Dateien "

MListFreeSize
" %s байт свободно "
" %s free bytes "
" %s volných bytů "
" %s freie Bytes "

MDirInfoViewTitle
l:
"Просмотр"
"View"
"Zobraz"
"Betrachten"

MFileToEdit
"Редактировать файл:"
"File to edit:"
"Soubor k editaci:"
"Datei bearbeiten:"

MUnselectTitle
l:
"Снять"
"Deselect"
"Odznačit"
"Abwählen"

MSelectTitle
"Пометить"
"Select"
"Označit"
"Auswählen"

MSelectFilter
"&Фильтр"
"&Filter"
"&Filtr"
"&Filter"

MCompareTitle
l:
"Сравнение"
"Compare"
"Porovnat"
"Vergleichen"

MCompareFilePanelsRequired1
"Для сравнения папок требуются"
"Two file panels are required to perform"
"Pro provedení příkazu Porovnat adresáře"
"Zwei Dateipanels werden benötigt um"

MCompareFilePanelsRequired2
"две файловые панели"
"the Compare folders command"
"jsou nutné dva souborové panely"
"den Vergleich auszuführen."

MCompareSameFolders1
"Содержимое папок,"
"The folders contents seems"
"Obsahy adresářů jsou"
"Der Inhalt der beiden Ordner scheint"

MCompareSameFolders2
"скорее всего, одинаково"
"to be identical"
"identické"
"identisch zu sein."

MSelectAssocTitle
l:
"Выберите ассоциацию"
"Select association"
"Vyber závislosti"
"Dateiverknüpfung auswählen"

MAssocTitle
l:
"Ассоциации для файлов"
"File associations"
"Závislosti souborů"
"Dateiverknüpfungen"

MAssocBottom
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Bearb.: Entf,Einf,F4"

MAskDelAssoc
"Вы хотите удалить ассоциацию для"
"Do you wish to delete association for"
"Přejete si smazat závislost pro"
"Wollen Sie die Verknüpfung löschen für"

MAssocNeedMask
"Пожалуйста, укажите маску файлов"
"Please specify a file mask"
"Prosím zadejte masku souborů"
"Bitte geben Sie eine Dateimaske an"

MFileAssocTitle
l:
"Редактирование ассоциаций файлов"
"Edit file associations"
"Upravit závislosti souborů"
"Dateiverknüpfungen bearbeiten"

MFileAssocMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"
"&Maska nebo masky souborů:"
"Datei&maske (mehrere getrennt mit Komma):"

MFileAssocDescr
"&Описание ассоциации:"
"&Description of the association:"
"&Popis asociací:"
"&Beschreibung der Verknüpfung:"

MFileAssocExec
"Команда, &выполняемая по Enter:"
"E&xecute command (used for Enter):"
"&Vykonat příkaz (použito pro Enter):"
"Befehl &ausführen (mit Enter):"

MFileAssocAltExec
"Коман&да, выполняемая по Ctrl-PgDn:"
"Exec&ute command (used for Ctrl-PgDn):"
"V&ykonat příkaz (použito pro Ctrl-PgDn):"
"Befehl a&usführen (mit Strg-PgDn):"

MFileAssocView
"Команда &просмотра, выполняемая по F3:"
"&View command (used for F3):"
"Příkaz &Zobraz (použito pro F3):"
"Be&trachten (mit F3):"

MFileAssocAltView
"Команда просмотра, в&ыполняемая по Alt-F3:"
"V&iew command (used for Alt-F3):"
"Příkaz Z&obraz (použito pro Alt-F3):"
"Bet&rachten (mit Alt-F3):"

MFileAssocEdit
"Команда &редактирования, выполняемая по F4:"
"&Edit command (used for F4):"
"Příkaz &Edituj (použito pro F4):"
"Bearb&eiten (mit F4):"

MFileAssocAltEdit
"Команда редактировани&я, выполняемая по Alt-F4:"
"Edit comm&and (used for Alt-F4):"
"Příkaz Editu&j (použito pro Alt-F4):"
"Bearbe&iten (mit Alt-F4):"

MViewF1
l:
l://Viewer: functional keys, 12 keys, except F2 - 2 keys, and F8 - 2 keys
"Помощь"
"Help"
"Pomoc"
"Hilfe"

MViewF2
le:// this is another text for F2
"Сверн"
"Wrap"
"Zalom"
"Umbr."

MViewF3
"Выход"
"Quit"
"Konec"
"Ende"

MViewF4
"Код"
"Hex"
"Hex"
"Hex"

MViewF5
""
""
""
""

MViewF6
"Редакт"
"Edit"
"Edit"
"Bearb"

MViewF7
"Поиск"
"Search"
"Hledat"
"Suchen"

MViewF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MViewF9
""
""
""
""

MViewF10
"Выход"
"Quit"
"Konec"
"Ende"

MViewF11
"Модули"
"Plugins"
"Plugin"
"Plugin"

MViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"

MViewF2Unwrap
"Развер"
"Unwrap"
"Nezal"
"KeinUm"

MViewF4Text
l:// this is another text for F4
"Текст"
"Text"
"Text"
"Text"

MViewF8DOS
"OEM"
"OEM"
"OEM"
"OEM"

MViewShiftF1
l:
l://Viewer: Shift
""
""
""
""

MViewShiftF2
"Слова"
"WWrap"
"ZalSlo"
"WUmbr"

MViewShiftF3
""
""
""
""

MViewShiftF4
""
""
""
""

MViewShiftF5
""
""
""
""

MViewShiftF6
""
""
""
""

MViewShiftF7
"Дальше"
"Next"
"Další"
"Nächst"

MViewShiftF8
"Таблиц"
"Table"
"ZnSady"
"Tabell"

MViewShiftF9
""
""
""
""

MViewShiftF10
""
""
""
""

MViewShiftF11
""
""
""
""

MViewShiftF12
""
""
""
""

MViewAltF1
l:
l://Viewer: Alt
""
""
""
""

MViewAltF2
""
""
""
""

MViewAltF3
""
""
""
""

MViewAltF4
""
""
""
""

MViewAltF5
"Печать"
"Print"
"Tisk"
"Druck"

MViewAltF6
""
""
""
""

MViewAltF7
"Назад"
"Prev"
"Předch"
"Letzt"

MViewAltF8
"Перейт"
"Goto"
"Jít na"
"GeheZu"

MViewAltF9
"Видео"
"Video"
"Video"
"Ansich"

MViewAltF10
""
""
""
""

MViewAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"

MViewAltF12
""
""
""
""

MViewCtrlF1
l:
l://Viewer: Ctrl
""
""
""
""

MViewCtrlF2
""
""
""
""

MViewCtrlF3
""
""
""
""

MViewCtrlF4
""
""
""
""

MViewCtrlF5
""
""
""
""

MViewCtrlF6
""
""
""
""

MViewCtrlF7
""
""
""
""

MViewCtrlF8
""
""
""
""

MViewCtrlF9
""
""
""
""

MViewCtrlF10
"Позиц"
"GoFile"
"JítSou"
"GehDat"

MViewCtrlF11
""
""
""
""

MViewCtrlF12
""
""
""
""

MViewAltShiftF1
l:
l://Viewer: AltShift
""
""
""
""

MViewAltShiftF2
""
""
""
""

MViewAltShiftF3
""
""
""
""

MViewAltShiftF4
""
""
""
""

MViewAltShiftF5
""
""
""
""

MViewAltShiftF6
""
""
""
""

MViewAltShiftF7
""
""
""
""

MViewAltShiftF8
""
""
""
""

MViewAltShiftF9
"Конфиг"
"Config"
"Nastav"
"Konfig"

MViewAltShiftF10
""
""
""
""

MViewAltShiftF11
""
""
""
""

MViewAltShiftF12
""
""
""
""

MViewCtrlShiftF1
l:
l://Viewer: CtrlShift
""
""
""
""

MViewCtrlShiftF2
""
""
""
""

MViewCtrlShiftF3
""
""
""
""

MViewCtrlShiftF4
""
""
""
""

MViewCtrlShiftF5
""
""
""
""

MViewCtrlShiftF6
""
""
""
""

MViewCtrlShiftF7
""
""
""
""

MViewCtrlShiftF8
""
""
""
""

MViewCtrlShiftF9
""
""
""
""

MViewCtrlShiftF10
""
""
""
""

MViewCtrlShiftF11
""
""
""
""

MViewCtrlShiftF12
""
""
""
""

MViewCtrlAltF1
l:
l://Viewer: CtrlAlt
""
""
""
""

MViewCtrlAltF2
""
""
""
""

MViewCtrlAltF3
""
""
""
""

MViewCtrlAltF4
""
""
""
""

MViewCtrlAltF5
""
""
""
""

MViewCtrlAltF6
""
""
""
""

MViewCtrlAltF7
""
""
""
""

MViewCtrlAltF8
""
""
""
""

MViewCtrlAltF9
""
""
""
""

MViewCtrlAltF10
""
""
""
""

MViewCtrlAltF11
""
""
""
""

MViewCtrlAltF12
""
""
""
""

MViewCtrlAltShiftF1
l:
l://Viewer: CtrlAltShift
""
""
""
""

MViewCtrlAltShiftF2
""
""
""
""

MViewCtrlAltShiftF3
""
""
""
""

MViewCtrlAltShiftF4
""
""
""
""

MViewCtrlAltShiftF5
""
""
""
""

MViewCtrlAltShiftF6
""
""
""
""

MViewCtrlAltShiftF7
""
""
""
""

MViewCtrlAltShiftF8
""
""
""
""

MViewCtrlAltShiftF9
""
""
""
""

MViewCtrlAltShiftF10
""
""
""
""

MViewCtrlAltShiftF11
""
""
""
""

MViewCtrlAltShiftF12
le://end of functional keys (Viewer)
""
""
""
""

MSingleViewF1
l:
l://Single Viewer: functional keys, 12 keys, except F2 - 2 keys, and F8 - 2 keys
"Помощь"
"Help"
"Pomoc"
"Hilfe"

MSingleViewF2
"Сверн"
"Wrap"
"Zalom"
"Umbr."

MSingleViewF3
"Выход"
"Quit"
"Konec"
"Ende"

MSingleViewF4
"Код"
"Hex"
"Hex"
"Hex"

MSingleViewF5
""
""
""
""

MSingleViewF6
"Редакт"
"Edit"
"Edit"
"Bearb"

MSingleViewF7
"Поиск"
"Search"
"Hledat"
"Suchen"

MSingleViewF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MSingleViewF9
""
""
""
""

MSingleViewF10
"Выход"
"Quit"
"Konec"
"Ende"

MSingleViewF11
"Модули"
"Plugins"
"Plugin"
"Plugins"

MSingleViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"

MSingleViewF2Unwrap
l:// this is another text for F2
"Развер"
"Unwrap"
"Nezal"
"KeinUm"

MSingleViewF4Text
l:// this is another text for F4
"Текст"
"Text"
"Text"
"Text"

MSingleViewF8DOS
"OEM"
"OEM"
"OEM"
"OEM"

MSingleViewShiftF1
l:
l://Single Viewer: Shift
""
""
""
""

MSingleViewShiftF2
"Слова"
"WWrap"
"ZalSlo"
"WUmbr"

MSingleViewShiftF3
""
""
""
""

MSingleViewShiftF4
""
""
""
""

MSingleViewShiftF5
""
""
""
""

MSingleViewShiftF6
""
""
""
""

MSingleViewShiftF7
"Дальше"
"Next"
"Další"
"Nächst"

MSingleViewShiftF8
"Таблиц"
"Table"
"ZnSady"
"Tabell"

MSingleViewShiftF9
""
""
""
""

MSingleViewShiftF10
""
""
""
""

MSingleViewShiftF11
""
""
""
""

MSingleViewShiftF12
""
""
""
""

MSingleViewAltF1
l:
l://Single Viewer: Alt
""
""
""
""

MSingleViewAltF2
""
""
""
""

MSingleViewAltF3
""
""
""
""

MSingleViewAltF4
""
""
""
""

MSingleViewAltF5
"Печать"
"Print"
"Tisk"
"Druck"

MSingleViewAltF6
""
""
""
""

MSingleViewAltF7
"Назад"
"Prev"
"Předch"
"Letzt"

MSingleViewAltF8
"Перейт"
"Goto"
"Jít na"
"GeheZu"

MSingleViewAltF9
"Видео"
"Video"
"Video"
"Ansich"

MSingleViewAltF10
""
""
""
""

MSingleViewAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"

MSingleViewAltF12
""
""
""
""

MSingleViewCtrlF1
l:
l://Single Viewer: Ctrl
""
""
""
""

MSingleViewCtrlF2
""
""
""
""

MSingleViewCtrlF3
""
""
""
""

MSingleViewCtrlF4
""
""
""
""

MSingleViewCtrlF5
""
""
""
""

MSingleViewCtrlF6
""
""
""
""

MSingleViewCtrlF7
""
""
""
""

MSingleViewCtrlF8
""
""
""
""

MSingleViewCtrlF9
""
""
""
""

MSingleViewCtrlF10
""
""
""
""

MSingleViewCtrlF11
""
""
""
""

MSingleViewCtrlF12
""
""
""
""

MSingleViewAltShiftF1
l:
l://Single Viewer: AltShift
""
""
""
""

MSingleViewAltShiftF2
""
""
""
""

MSingleViewAltShiftF3
""
""
""
""

MSingleViewAltShiftF4
""
""
""
""

MSingleViewAltShiftF5
""
""
""
""

MSingleViewAltShiftF6
""
""
""
""

MSingleViewAltShiftF7
""
""
""
""

MSingleViewAltShiftF8
""
""
""
""

MSingleViewAltShiftF9
"Конфиг"
"Config"
"Nastav"
"Konfig"

MSingleViewAltShiftF10
""
""
""
""

MSingleViewAltShiftF11
""
""
""
""

MSingleViewAltShiftF12
""
""
""
""

MSingleViewCtrlShiftF1
l:
l://Single Viewer: CtrlShift
""
""
""
""

MSingleViewCtrlShiftF2
""
""
""
""

MSingleViewCtrlShiftF3
""
""
""
""

MSingleViewCtrlShiftF4
""
""
""
""

MSingleViewCtrlShiftF5
""
""
""
""

MSingleViewCtrlShiftF6
""
""
""
""

MSingleViewCtrlShiftF7
""
""
""
""

MSingleViewCtrlShiftF8
""
""
""
""

MSingleViewCtrlShiftF9
""
""
""
""

MSingleViewCtrlShiftF10
""
""
""
""

MSingleViewCtrlShiftF11
""
""
""
""

MSingleViewCtrlShiftF12
""
""
""
""

MSingleViewCtrlAltF1
l:
l://Single Viewer: CtrlAlt
""
""
""
""

MSingleViewCtrlAltF2
""
""
""
""

MSingleViewCtrlAltF3
""
""
""
""

MSingleViewCtrlAltF4
""
""
""
""

MSingleViewCtrlAltF5
""
""
""
""

MSingleViewCtrlAltF6
""
""
""
""

MSingleViewCtrlAltF7
""
""
""
""

MSingleViewCtrlAltF8
""
""
""
""

MSingleViewCtrlAltF9
""
""
""
""

MSingleViewCtrlAltF10
""
""
""
""

MSingleViewCtrlAltF11
""
""
""
""

MSingleViewCtrlAltF12
""
""
""
""

MSingleViewCtrlAltShiftF1
l:
l://Single Viewer: CtrlAltShift
""
""
""
""

MSingleViewCtrlAltShiftF2
""
""
""
""

MSingleViewCtrlAltShiftF3
""
""
""
""

MSingleViewCtrlAltShiftF4
""
""
""
""

MSingleViewCtrlAltShiftF5
""
""
""
""

MSingleViewCtrlAltShiftF6
""
""
""
""

MSingleViewCtrlAltShiftF7
""
""
""
""

MSingleViewCtrlAltShiftF8
""
""
""
""

MSingleViewCtrlAltShiftF9
""
""
""
""

MSingleViewCtrlAltShiftF10
""
""
""
""

MSingleViewCtrlAltShiftF11
""
""
""
""

MSingleViewCtrlAltShiftF12
le://end of functional keys (Single Viewer)
""
""
""
""

MInViewer
"просмотр %s"
"view %s"
"prohlížení %s"
"Betrachte %s"

MInEditor
"редактирование %s"
"edit %s"
"editace %s"
"Bearbeite %s"

MFilterTitle
l:
"Меню фильтров"
"Filters menu"
"Menu filtrů"
"Filtermenü"

MFilterBottom
"+,-,Пробел,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Space,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Mezera,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Leer,I,X,BS,UmschBS,Einf,Entf,F4,F5,StrgUp,StrgDn"

MPanelFileType
"Файлы панели"
"Panel file type"
"Typ panelu souborů"
"Dateityp in Panel"

MFolderFileType
"Папки"
"Folders"
"Adresáře"
"Ordner"

MCanEditCustomFilterOnly
"Только пользовательский фильтр можно редактировать"
"Only custom filter can be edited"
"Jedině vlastní filtr může být upraven"
"Nur eigene Filter können editiert werden."

MAskDeleteFilter
"Вы хотите удалить фильтр"
"Do you wish to delete the filter"
"Přejete si smazat filtr"
"Wollen Sie den eigenen Filter löschen"

MCanDeleteCustomFilterOnly
"Только пользовательский фильтр может быть удален"
"Only custom filter can be deleted"
"Jedině vlastní filtr může být smazán"
"Nur eigene Filter können gelöscht werden."

MFindFileTitle
l:
"Поиск файла"
"Find file"
"Hledat soubor"
"Nach Dateien suchen"

MFindFileResultTitle
"Поиск файла - результат"
"Find file - result"
"Hledat soubor - výsledek"
"Suchergebnisse"

MFindFileMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"
"Maska nebo masky souborů:"
"Datei&maske (mehrere getrennt mit Komma):"

MFindFileText
"&Содержащих текст:"
"Containing &text:"
"Obsahující te&xt:"
"Enthält &Text:"

MFindFileHex
"&Содержащих 16-ричный код:"
"Con&taining hex:"
"Obsahující &hex:"
"En&thält Hex (xx xx ...):"

MFindFileCodePage
"Используя таблицу сим&волов:"
"Using character ta&ble:"
"Použít &znakovou sadu:"
"Zeichenta&belle verwenden:"

MFindFileCase
"&Учитывать регистр"
"&Case sensitive"
"Roz&lišovat velikost písmen"
"Gr&oß-/Kleinschreibung"

MFindFileWholeWords
"Только &целые слова"
"&Whole words"
"&Celá slova"
"Nur &ganze Wörter"

MFindFileAllTables
"Все таблицы символов"
"All character tables"
"Všechny znakové sady"
"Alle Zeichentabellen"

MFindArchives
"Искать в а&рхивах"
"Search in arch&ives"
"Hledat v a&rchívech"
"In Arch&iven suchen"

MFindFolders
"Искать п&апки"
"Search for f&olders"
"Hledat a&dresáře"
"Nach &Ordnern suchen"

MFindSymLinks
"Искать в символи&ческих ссылках"
"Search in symbolic lin&ks"
"Hledat v s&ymbolických lincích"
"In symbolischen Lin&ks suchen"

MSearchForHex
"Искать 16-ричн&ый код"
"Search for &hex"
"Hledat &hex"
"Nach &Hex suchen"

MSearchWhere
"Выберите область поиска:"
"Select search area:"
"Zvolte oblast hledání:"
"Suchbereich:"

MSearchAllDisks
"На всех несъемных &дисках"
"In &all non-removable drives"
"Ve všech p&evných discích"
"Auf &allen festen Datenträger"

MSearchAllButNetwork
"На всех &локальных дисках"
"In all &local drives"
"Ve všech &lokálních discích"
"Auf allen &lokalen Datenträgern"

MSearchInPATH
"В PATH-катало&гах"
"In &PATH folders"
"V adresářích z &PATH"
"In &PATH-Ordnern"

MSearchFromRootOfDrive
"С кор&ня диска"
"From the &root of"
"V &kořeni"
"Ab Wu&rzelverz. von"

MSearchFromRootFolder
"С кор&невой папки"
"From the &root folder"
"V kořeno&vém adresáři"
"Ab Wu&rzelverzeichnis"

MSearchFromCurrent
"С &текущей папки"
"From the curre&nt folder"
"V tomto adresář&i"
"Ab dem aktuelle&n Ordner"

MSearchInCurrent
"Только в теку&щей папке"
"The current folder onl&y"
"P&ouze v tomto adresáři"
"Nur im aktue&llen Ordner"

MSearchInSelected
"В &отмеченных папках"
"&Selected folders"
"Ve vy&braných adresářích"
"In au&sgewählten Ordner"

MFindUseFilter
"Исполь&зовать фильтр"
"&Use filter"
"Použít f&iltr"
"Ben&utze Filter"

MFindAdvancedOptions
"Дополнит&ельные параметры"
"Advanced options"
"Pokročilé nastavení"
"Erweiterte Optionen"

MFindUsingFilter
"используя фильтр"
"using filter"
"používám filtr"
"mit Filter"

MFindFileFind
"&Искать"
"&Find"
"&Hledat"
"&Suchen"

MFindFileDrive
"Дис&к"
"Dri&ve"
"D&isk"
"Lauf&werk"

MFindFileSetFilter
"&Фильтр"
"Filt&er"
"&Filtr"
"Filt&er"

MFindFileAdvanced
"До&полнительно"
"Advance&d"
"Pokr&očilé"
"Er&weitert"

MFindFileTable
"Т&аблица"
"Ta&ble"
"Zna&ková sada"
"Ta&belle"

MFindSearchingIn
"Поиск%s в:"
"Searching%s in:"
"Hledám%s v:"
"Suche%s in:"

MFindNewSearch
"&Новый поиск"
"&New search"
"&Nové hledání"
"&Neue Suche"

MFindGoTo
"Пе&рейти"
"&Go to"
"&Jdi na"
"&Gehe zu"

MFindView
"&Смотреть"
"&View"
"Zo&braz"
"&Betrachten"

MFindPanel
"Пане&ль"
"&Panel"
"&Panel"
"&Panel"

MFindStop
"С&топ"
"&Stop"
"&Stop"
"&Stoppen"

MFindDone
l:
"Поиск закончен. Найдено %d файл(ов) и %d папка(ок)"
"Search done. Found %d file(s) and %d folder(s)"
"Hledání ukončeno. Nalezeno %d soubor(ů) a %d adresář(ů)"
"Suche beendet. %d Datei(en) und %d Ordner gefunden."

MFindCancel
"Отм&ена"
"&Cancel"
"&Storno"
"Ab&bruch"

MFindFound
l:
"Найдено"
"Found"
"Nalezeno"
"Gefunden"

MFindFileFolder
l:
"Папка"
"Folder"
"Adresář"
"Ordner"

MFindFileAdvancedTitle
l:
"Дополнительные параметры поиска"
"Find file advanced options"
"Pokročilé nastavení vyhledávání souborů"
"Erweiterte Optionen"

MFindFileSearchFirst
"Проводить поиск в &первых:"
"Search only in the &first:"
"Hledat po&uze v prvních:"
"Nur &in den ersten x Bytes:"

MFoldTreeSearch
l:
"Поиск:"
"Search:"
"Hledat:"
"Suchen:"

MGetTableTitle
l:
"Таблицы"
"Tables"
"Znakové sady:"
"Tabellen"

MGetTableBottomTitle
"CtrlH, Ins"
"CtrlH, Ins"
"CtrlH, Ins"
"StrgH, Einf"

MHighlightTitle
l:
"Раскраска файлов"
"Files highlighting"
"Zvýrazňování souborů"
"Farbmarkierungen"

MHighlightBottom
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Nahoru,Ctrl-Dolů"
"Einf,Entf,F4,F5,StrgUp,StrgDown"

MHighlightUpperSortGroup
"Верхняя группа сортировки"
"Upper sort group"
"Vzesupné řazení"
"Obere Sortiergruppen"

MHighlightLowerSortGroup
"Нижняя группа сортировки"
"Lower sort group"
"Sestupné řazení"
"Untere Sortiergruppen"

MHighlightLastGroup
"Наименее приоритетная группа раскраски"
"Lowest priority highlighting group"
"Zvýraznění nejnižší prority"
"Farbmarkierungen mit niedrigster Priorität"

MHighlightAskDel
"Вы хотите удалить раскраску для"
"Do you wish to delete highlighting for"
"Přejete si smazat zvýraznění pro"
"Wollen Sie Farbmarkierungen löschen für"

MHighlightWarning
"Будут потеряны все Ваши настройки!"
"You will lose all changes!"
"Všechny změny budou ztraceny!"
"Sie verlieren jegliche Änderungen!"

MHighlightAskRestore
"Вы хотите восстановить раскраску файлов по умолчанию?"
"Do you wish to restore default highlighting?"
"Přejete si obnovit výchozí nastavení?"
"Wollen Sie Standard-Farbmarkierungen wiederherstellen?"

MHighlightEditTitle
l:
"Редактирование раскраски файлов"
"Edit files highlighting"
"Upravit zvýrazňování souborů"
"Farbmarkierungen bearbeiten"

MHighlightMarkChar
"Оп&циональный символ пометки,"
"Optional markin&g character,"
"Volitelný &znak pro označení určených souborů,"
"Optionale Markierun&g mit Zeichen,"

MHighlightTransparentMarkChar
"прозра&чный"
"tra&nsparent"
"průh&ledný"
"tra&nsparent"

MHighlightColors
" Цвета файлов (\"черный на черном\" - цвет по умолчанию) "
" File name colors (\"black on black\" - default color) "
" Barva názvu souborů (\"černá na černé\" - výchozí barva) "
" Dateinamenfarben (\"Schwarz auf Blau\"=Standard) "

MHighlightFileName1
"&1. Обычное имя файла                "
"&1. Normal file name               "
"&1. Normální soubor"
"&1. Normaler Dateiname             "

MHighlightFileName2
"&3. Помеченное имя файла             "
"&3. Selected file name             "
"&3. Vybraný soubor"
"&3. Markierter Dateiame            "

MHighlightFileName3
"&5. Имя файла под курсором           "
"&5. File name under cursor         "
"&5. Soubor pod kurzorem"
"&5. Dateiname unter Cursor         "

MHighlightFileName4
"&7. Помеченное под курсором имя файла"
"&7. File name selected under cursor"
"&7. Vybraný soubor pod kurzorem"
"&7. Dateiname markiert unter Cursor"

MHighlightMarking1
"&2. Пометка"
"&2. Marking"
"&2. Označení"
"&2. Markierung"

MHighlightMarking2
"&4. Пометка"
"&4. Marking"
"&4. Označení"
"&4. Markierung"

MHighlightMarking3
"&6. Пометка"
"&6. Marking"
"&6. Označení"
"&6. Markierung"

MHighlightMarking4
"&8. Пометка"
"&8. Marking"
"&8. Označení"
"&8. Markierung"

MHighlightExample1
"║filename.ext │"
"║filename.ext │"
"║filename.ext │"
"║dateinam.erw │"

MHighlightExample2
"║ filename.ext│"
"║ filename.ext│"
"║ filename.ext│"
"║ dateinam.erw│"

MHighlightContinueProcessing
"Продолжать &обработку"
"C&ontinue processing"
"Pokračovat ve zpracová&ní"
"Verarbeitung f&ortsetzen"

MInfoTitle
l:
"Информация"
"Information"
"Informace"
"Informationen"

MInfoCompName
"Имя компьютера"
"Computer name"
"Název počítače"
"Computername"

MInfoUserName
"Имя пользователя"
"User name"
"Jméno uživatele"
"Benutzername"

MInfoRemovable
"Сменный"
"Removable"
"Vyměnitelný"
"Austauschbares"

MInfoFixed
"Жесткий"
"Fixed"
"Pevný"
"Lokales"

MInfoNetwork
"Сетевой"
"Network"
"Síťový"
"Netzwerk"

MInfoCDROM
"CD-ROM"
"CD-ROM"
"CD-ROM"
"CD-ROM"

MInfoCD_RW
"CD-RW"
"CD-RW"
"CD-RW"
"CD-RW"

MInfoCD_RWDVD
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD"

MInfoDVD_ROM
"DVD-ROM"
"DVD-ROM"
"DVD-ROM"
"DVD-ROM"

MInfoDVD_RW
"DVD-RW"
"DVD-RW"
"DVD-RW"
"DVD-RW"

MInfoDVD_RAM
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"

MInfoRAM
"RAM"
"RAM"
"RAM"
"RAM"

MInfoSUBST
"SUBST"
"Subst"
"SUBST"
"Subst"

MInfoDisk
"диск"
"disk"
"disk"
"Laufwerk"

MInfoDiskTotal
"Всего байтов"
"Total bytes"
"Celkem bytů"
"Bytes gesamt"

MInfoDiskFree
"Свободных байтов"
"Free bytes"
"Volných bytů"
"Bytes frei"

MInfoDiskLabel
"Метка тома"
"Volume label"
"Popisek disku"
"Laufwerksbezeichnung"

MInfoDiskNumber
"Серийный номер"
"Serial number"
"Sériové číslo"
"Seriennummer"

MInfoMemory
" Память "
" Memory "
" Paměť "
" Speicher "

MInfoMemoryLoad
"Загрузка памяти"
"Memory load"
"Zatížení paměti"
"Speicherverbrauch"

MInfoMemoryTotal
"Всего памяти"
"Total memory"
"Celková paměť"
"Speicher gesamt"

MInfoMemoryFree
"Свободно памяти"
"Free memory"
"Volná paměť"
"Speicher frei"

MInfoVirtualTotal
"Всего вирт. памяти"
"Total virtual"
"Celkem virtuální"
"Virtueller Speicher gesamt"

MInfoVirtualFree
"Свободно вирт. памяти"
"Free virtual"
"Volná virtuální"
"Virtueller Speicher frei"

MInfoDizAbsent
"Файл описания папки отсутствует"
"Folder description file is absent"
"Soubor s popisem adresáře chybí"
"Keine Datei mit Ordnerbeschreibungen vorhanden."

MErrorInvalidFunction
l:
"Некорректная функция"
"Incorrect function"
"Nesprávná funkce"
"Ungültige Funktion"

MErrorBadCommand
"Команда не распознана"
"Command not recognized"
"Příkaz nebyl rozpoznán"
"Unbekannter Befehl"

MErrorFileNotFound
"Файл не найден"
"File not found"
"Soubor nenalezen"
"Datei nicht gefunden"

MErrorPathNotFound
"Путь не найден"
"Path not found"
"Cesta nenalezena"
"Pfad nicht gefunden"

MErrorTooManyOpenFiles
"Слишком много открытых файлов"
"Too many open files"
"Příliš mnoho otevřených souborů"
"Zu viele geöffnete Dateien"

MErrorAccessDenied
"Доступ запрещен"
"Access denied"
"Přístup odepřen"
"Zugriff verweigert"

MErrorNotEnoughMemory
"Недостаточно памяти"
"Not enough memory"
"Nedostatek paměti"
"Nicht genügend Speicher"

MErrorDiskRO
"Попытка записи на защищенный от записи диск"
"Cannot write to write protected disk"
"Nelze zapisovat na disk chráněný proti zápisu"
"Der Datenträger ist schreibgeschützt"

MErrorDeviceNotReady
"Устройство не готово"
"The device is not ready"
"Zařízení není připraveno"
"Das Gerät ist nicht bereit"

MErrorCannotAccessDisk
"Доступ к диску невозможен"
"Disk cannot be accessed"
"Na disk nelze přistoupit"
"Auf Datenträger kann nicht zugegriffen werden"

MErrorSectorNotFound
"Сектор не найден"
"Sector not found"
"Sektor nenalezen"
"Sektor nicht gefunden"

MErrorOutOfPaper
"В принтере нет бумаги"
"The printer is out of paper"
"V tiskárně došel papír"
"Der Drucker hat kein Papier mehr"

MErrorWrite
"Ошибка записи"
"Write fault error"
"Chyba zápisu"
"Fehler beim Schreibzugriff"

MErrorRead
"Ошибка чтения"
"Read fault error"
"Chyba čtení"
"Fehler beim Lesezugriff"

MErrorDeviceGeneral
"Общая ошибка устройства"
"Device general failure"
"Obecná chyba zařízení"
"Ein Gerätefehler ist aufgetreten"

MErrorFileSharing
"Нарушение совместного доступа к файлу"
"File sharing violation"
"Narušeno sdílení souborů"
"Zugriffsverletzung"

MErrorNetworkPathNotFound
"Сетевой путь не найден"
"The network path was not found"
"Síťová cesta nebyla nalezena"
"Der Netzwerkpfad wurde nicht gefunden"

MErrorNetworkBusy
"Сеть занята"
"The network is busy"
"Síť je zaneprázdněna"
"Das Netzwerk ist beschäftigt"

MErrorNetworkAccessDenied
"Сетевой доступ запрещен"
"Network access is denied"
"Přístup na síť zakázán"
"Netzwerkzugriff wurde verweigert"

MErrorNetworkWrite
"Ошибка записи в сети"
"A write fault occurred on the network"
"Na síti došlo k chybě v zápisu"
"Fehler beim Schreibzugriff auf das Netzwerk"

MErrorDiskLocked
"Диск используется или заблокирован другим процессом"
"The disk is in use or locked by another process"
"Disk je používán nebo uzamčen jiným procesem"
"Datenträger wird verwendet oder ist durch einen anderen Prozess gesperrt"

MErrorFileExists
"Файл или папка уже существует"
"File or folder already exists"
"Soubor nebo adresář již existuje"
"Die Datei oder der Ordner existiert bereits."

MErrorInvalidName
"Указанное имя неверно"
"The specified name is invalid"
"Zadaný název je neplatný"
"Der angegebene Name ist ungültig"

MErrorInsufficientDiskSpace
"Нет места на диске"
"Insufficient disk space"
"Nedostatek místa na disku"
"Unzureichend Speicherplatz am Datenträger"

MErrorFolderNotEmpty
"Папка не пустая"
"The folder is not empty"
"Adresář není prázdný"
"Der Ordner ist nicht leer"

MErrorIncorrectUserName
"Неверное имя пользователя"
"Incorrect user name"
"Neplatné jméno uživatele"
"Ungültiger Benutzername"

MErrorIncorrectPassword
"Неверный пароль"
"Incorrect password"
"Neplatné heslo"
"Ungültiges Passwort"

MErrorLoginFailure
"Ошибка регистрации"
"Login failure"
"Přihlášení selhalo"
"Login fehlgeschlagen"

MErrorConnectionAborted
"Соединение разорвано"
"Connection aborted"
"Spojení přerušeno"
"Verbindung abgebrochen"

MErrorCancelled
"Операция отменена"
"Operation cancelled"
"Operace stornována"
"Vorgang abgebrochen"

MErrorNetAbsent
"Сеть отсутствует"
"No network present"
"Síť není k dispozici"
"Kein Netzwerk verfügbar"

MErrorNetDeviceInUse
"Устройство используется и не может быть отсоединено"
"Device is in use and cannot be disconnected"
"Zařízení se používá a nemůže být odpojeno"
"Gerät wird gerade verwendet oder kann nicht getrennt werden"

MErrorNetOpenFiles
"На сетевом диске есть открытые файлы"
"This network connection has open files"
"Přes toto síťové spojení jsou otevřeny soubory"
"Diese Netzwerkverbindung hat geöffnete Dateien"

MErrorAlreadyAssigned
"Имя локального устройства уже использовано"
"The local device name is already in use"
"Název lokálního zařízení je již používán"
"Der lokale Gerätename wird bereits verwendet"

MErrorAlreadyRemebered
"Имя локального устройства уже находится в профиле пользователя"
"The local device is already in the user profile"
"Lokální zařízení je již v uživatelově profilu"
"Der lokale Datenträger ist bereits Teil des Benutzerprofils"

MErrorNotLoggedOn
"Пользователь не зарегистрирован в сети"
"User has not logged on to the network"
"Uživatel nebyl do sítě přihlášen"
"Benutzer hat sich nicht am Netzwerk angemeldet"

MErrorInvalidPassword
"Неверный пароль пользователя"
"The user password is invalid"
"Uživatelovo heslo není správné"
"Das Benutzerpasswort ist ungültig"

MErrorNoRecoveryPolicy
"Для этой системы отсутствует политика надежного восстановления шифрования"
"There is no valid encryption recovery policy configured for this system"
"V tomto systému není nastaveno žádné platné pravidlo pro dešifrování"
"Auf diesem System ist keine gültige Richtlinie zum Wiederherstellen der Verschlüsselung konfiguriert."

MErrorEncryptionFailed
"Ошибка при попытке шифрования файла"
"The specified file could not be encrypted"
"Zadaný soubor nemohl být zašifrován"
"Die angegebene Datei konnte nicht verschlüsselt werden"

MErrorDecryptionFailed
"Ошибка при попытке расшифровки файла"
"The specified file could not be decrypted"
"Zadaný soubor nemohl být dešifrován"
"Die angegebene Datei konnte nicht entschlüsselt werden"

MErrorFileNotEncrypted
"Указанный файл не зашифрован"
"The specified file is not encrypted"
"Zadaný soubor není zašifrován"
"Die angegebene Datei ist nicht verschlüsselt"

MErrorNoAssociation
"Указанному файлу не сопоставлено ни одно приложение для выполнения данной операции"
"No application is associated with the specified file for this operation"
"K zadanému souboru není asociována žádná aplikace pro tuto operaci"
"Diesem Dateityp und dieser Aktion ist kein Programm zugewiesen."

MErrorFullPathNameLong
l:
"Полный путь к файлу имеет слишком большую длину"
"The full pathname is too long"
"Plná cesta k souboru je příliš dlouhá"
"Der volle Name des Pfades ist zu lang"

MCannotExecute
l:
"Ошибка выполнения"
"Cannot execute"
"Nelze provést"
"Fehler beim Ausführen von"

MScanningFolder
"Просмотр папки"
"Scanning the folder"
"Prohledávám adresář"
"Scanne den Ordner"

MMakeFolderTitle
l:
"Создание папки"
"Make folder"
"Vytvoření adresáře"
"Ordner erstellen"

MCreateFolder
"Создать п&апку"
"Create the &folder"
"Vytvořit &adresář"
"Diesen &Ordner erstellen:"

MMultiMakeDir
"Обрабатыват&ь несколько имен папок"
"Process &multiple names"
"Zpracovat &více názvů"
"&Mehrere Namen verarbeiten (getrennt durch Semikolon)"

MIncorrectDirList
"Неправильный список папок"
"Incorrect folders list"
"Neplatný seznam adresářů"
"Fehlerhafte Ordnerliste"

MCannotCreateFolder
"Ошибка создания папки"
"Cannot create the folder"
"Adresář nelze vytvořit"
"Konnte den Ordner nicht erstellen"

MMenuBriefView
l:
"&Краткий                  LCtrl-1"
"&Brief              LCtrl-1"
"&Stručný                  LCtrl-1"
"&Kurz                 LStrg-1"

MMenuMediumView
"&Средний                  LCtrl-2"
"&Medium             LCtrl-2"
"S&třední                  LCtrl-2"
"&Mittel               LStrg-2"

MMenuFullView
"&Полный                   LCtrl-3"
"&Full               LCtrl-3"
"&Plný                     LCtrl-3"
"&Voll                 LStrg-3"

MMenuWideView
"&Широкий                  LCtrl-4"
"&Wide               LCtrl-4"
"Š&iroký                   LCtrl-4"
"B&reitformat          LStrg-4"

MMenuDetailedView
"&Детальный                LCtrl-5"
"Detai&led           LCtrl-5"
"Detai&lní                 LCtrl-5"
"Detai&lliert          LStrg-5"

MMenuDizView
"&Описания                 LCtrl-6"
"&Descriptions       LCtrl-6"
"P&opisky                  LCtrl-6"
"&Beschreibungen       LStrg-6"

MMenuLongDizView
"Д&линные описания         LCtrl-7"
"Lon&g descriptions  LCtrl-7"
"&Dlouhé popisky           LCtrl-7"
"Lan&ge Beschreibungen LStrg-7"

MMenuOwnersView
"Вл&адельцы файлов         LCtrl-8"
"File own&ers        LCtrl-8"
"Vlastník so&uboru         LCtrl-8"
"B&esitzer             LStrg-8"

MMenuLinksView
"Свя&зи файлов             LCtrl-9"
"File lin&ks         LCtrl-9"
"Souborové lin&ky          LCtrl-9"
"Dateilin&ks           LStrg-9"

MMenuAlternativeView
"Аль&тернативный полный    LCtrl-0"
"&Alternative full   LCtrl-0"
"&Alternativní plný        LCtrl-0"
"&Alternativ voll      LStrg-0"

MMenuInfoPanel
l:
"Панель ин&формации        Ctrl-L"
"&Info panel         Ctrl-L"
"Panel In&fo               Ctrl-L"
"&Infopanel            Strg-L"

MMenuTreePanel
"Де&рево папок             Ctrl-T"
"&Tree panel         Ctrl-T"
"Panel St&rom              Ctrl-T"
"Baumansich&t          Strg-T"

MMenuQuickView
"Быстры&й просмотр         Ctrl-Q"
"Quick &view         Ctrl-Q"
"Z&běžné zobrazení         Ctrl-Q"
"Sc&hnellansicht       Strg-Q"

MMenuSortModes
"Режим&ы сортировки        Ctrl-F12"
"&Sort modes         Ctrl-F12"
"Módy řaze&ní              Ctrl-F12"
"&Sortiermodi          Strg-F12"

MMenuLongNames
"Показывать длинные &имена Ctrl-N"
"Show long &names    Ctrl-N"
"Zobrazit dlouhé názv&y    Ctrl-N"
"Lange Datei&namen     Strg-N"

MMenuTogglePanel
"Панель &Вкл/Выкл          Ctrl-F1"
"Panel &On/Off       Ctrl-F1"
"Panel &Zap/Vyp            Ctrl-F1"
"&Panel ein/aus        Strg-F1"

MMenuReread
"П&еречитать               Ctrl-R"
"&Re-read            Ctrl-R"
"Obno&vit                  Ctrl-R"
"Aktualisie&ren        Strg-R"

MMenuChangeDrive
"С&менить диск             Alt-F1"
"&Change drive       Alt-F1"
"Z&měnit jednotku          Alt-F1"
"Laufwerk we&chseln    Alt-F1"

MMenuView
l:
"&Просмотр              F3"
"&View               F3"
"&Zobrazit                   F3"
"&Betrachten           F3"

MMenuEdit
"&Редактирование        F4"
"&Edit               F4"
"&Editovat                   F4"
"B&earbeiten           F4"

MMenuCopy
"&Копирование           F5"
"&Copy               F5"
"&Kopírovat                  F5"
"&Kopieren             F5"

MMenuMove
"П&еренос               F6"
"&Rename or move     F6"
"&Přejmenovat/Přesunout      F6"
"Ve&rschieben/Umben.   F6"

MMenuCreateFolder
"&Создание папки        F7"
"&Make folder        F7"
"&Vytvořit adresář           F7"
"&Ordner erstellen     F7"

MMenuDelete
"&Удаление              F8"
"&Delete             F8"
"&Smazat                     F8"
"&Löschen              F8"

MMenuWipe
"Уни&чтожение           Alt-Del"
"&Wipe               Alt-Del"
"&Vymazat                    Alt-Del"
"&Sicher löschen       Alt-Entf"

MMenuAdd
"&Архивировать          Shift-F1"
"Add &to archive     Shift-F1"
"Přidat do &archívu          Shift-F1"
"Zu Archiv &hinzuf.    Umsch-F1"

MMenuExtract
"Распако&вать           Shift-F2"
"E&xtract files      Shift-F2"
"&Rozbalit soubory           Shift-F2"
"Archiv e&xtrahieren   Umsch-F2"

MMenuArchiveCommands
"Архивн&ые команды      Shift-F3"
"Arc&hive commands   Shift-F3"
"Příkazy arc&hívu            Shift-F3"
"Arc&hivbefehle        Umsch-F3"

MMenuAttributes
"А&трибуты файлов       Ctrl-A"
"File &attributes    Ctrl-A"
"A&tributy souboru           Ctrl-A"
"Datei&attribute       Strg-A"

MMenuApplyCommand
"Применить коман&ду     Ctrl-G"
"A&pply command      Ctrl-G"
"Ap&likovat příkaz           Ctrl-G"
"Befehl an&wenden      Strg-G"

MMenuDescribe
"&Описание файлов       Ctrl-Z"
"Descri&be files     Ctrl-Z"
"Přidat popisek sou&borům    Ctrl-Z"
"Beschrei&bung ändern  Strg-Z"

MMenuSelectGroup
"Пометить &группу       Gray +"
"Select &group       Gray +"
"Oz&načit skupinu            Num +"
"&Gruppe auswählen     Num +"

MMenuUnselectGroup
"С&нять пометку         Gray -"
"U&nselect group     Gray -"
"O&dznačit skupinu           Num -"
"G&ruppe abwählen      Num -"

MMenuInvertSelection
"&Инверсия пометки      Gray *"
"&Invert selection   Gray *"
"&Invertovat výběr           Num *"
"Auswah&l umkehren     Num *"

MMenuRestoreSelection
"Восстановить по&метку  Ctrl-M"
"Re&store selection  Ctrl-M"
"&Obnovit výběr              Ctrl-M"
"Auswahl wiederher&st. Strg-M"

MMenuFindFile
l:
"&Поиск файла              Alt-F7"
"&Find file           Alt-F7"
"H&ledat soubor                  Alt-F7"
"Dateien &finden       Alt-F7"

MMenuHistory
"&История команд           Alt-F8"
"&History             Alt-F8"
"&Historie                       Alt-F8"
"&Historie             Alt-F8"

MMenuVideoMode
"Видео&режим               Alt-F9"
"&Video mode          Alt-F9"
"&Video mód                      Alt-F9"
"Ansicht<->&Vollbild   Alt-F9"

MMenuFindFolder
"Поис&к папки              Alt-F10"
"Fi&nd folder         Alt-F10"
"Hl&edat adresář                 Alt-F10"
"Ordner fi&nden        Alt-F10"

MMenuViewHistory
"Ис&тория просмотра        Alt-F11"
"File vie&w history   Alt-F11"
"Historie &zobrazení souborů     Alt-F11"
"Be&trachterhistorie   Alt-F11"

MMenuFoldersHistory
"Ист&ория папок            Alt-F12"
"F&olders history     Alt-F12"
"Historie &adresářů              Alt-F12"
"&Ordnerhistorie       Alt-F12"

MMenuSwapPanels
"По&менять панели          Ctrl-U"
"&Swap panels         Ctrl-U"
"Prohodit panel&y                Ctrl-U"
"Panels tau&schen      Strg-U"

MMenuTogglePanels
"Панели &Вкл/Выкл          Ctrl-O"
"&Panels On/Off       Ctrl-O"
"&Panely Zap/Vyp                 Ctrl-O"
"&Panels ein/aus       Strg-O"

MMenuCompareFolders
"&Сравнение папок"
"&Compare folders"
"Po&rovnat adresáře"
"Ordner verglei&chen"

MMenuUserMenu
"Меню пользовател&я"
"Edit user &menu"
"Upravit uživatelské &menu"
"Benutzer&menu editieren"

MMenuFileAssociations
"&Ассоциации файлов"
"File &associations"
"Asocia&ce souborů"
"Dat&eiverknüpfungen"

MMenuFolderShortcuts
"Ссы&лки на папки"
"Fol&der shortcuts"
"A&dresářové zkratky"
"Or&dnerschnellzugriff"

MMenuFilter
"&Фильтр панели файлов     Ctrl-I"
"File panel f&ilter   Ctrl-I"
"F&iltr panelu souborů           Ctrl-I"
"Panelf&ilter          Strg-I"

MMenuPluginCommands
"Команды внешних мо&дулей  F11"
"Pl&ugin commands     F11"
"Příkazy plu&ginů                F11"
"Pl&uginbefehle        F11"

MMenuWindowsList
"Список экра&нов           F12"
"Sc&reens list        F12"
"Seznam obrazove&k               F12"
"Seite&nliste          F12"

MMenuProcessList
"Список &задач             Ctrl-W"
"Task &list           Ctrl-W"
"Seznam úl&oh                    Ctrl-W"
"Task&liste            Strg-W"

MMenuHotPlugList
"Список Hotplug-&устройств"
"Ho&tplug devices list"
"Seznam v&yjímatelných zařízení"
"Sicheres En&tfernen"

MMenuSystemSettings
l:
"Систе&мные параметры"
"S&ystem settings"
"Nastavení S&ystému"
"&Grundeinstellungen"

MMenuPanelSettings
"Настройки па&нели"
"&Panel settings"
"Nastavení &Panelů"
"&Panels einrichten"

MMenuInterface
"Настройки &интерфейса"
"&Interface settings"
"Nastavení Ro&zhraní"
"Oberfläche einr&ichten"

MMenuDialogSettings
"Настройки &диалогов"
"Di&alog settings"
"Nastavení Dialo&gů"
"Di&aloge einrichten"

MMenuLanguages
"&Языки"
"Lan&guages"
"Nastavení &Jazyka"
"Sprac&hen"

MMenuPluginsConfig
"Параметры &внешних модулей"
"Pl&ugins configuration"
"Nastavení Plu&ginů"
"Konfiguration von Pl&ugins"

MMenuConfirmation
"&Подтверждения"
"Co&nfirmations"
"P&otvrzení"
"Bestätigu&ngen"

MMenuFilePanelModes
"Режим&ы панели файлов"
"File panel &modes"
"&Módy souborových panelů"
"Anzeige&modi von Dateipanels"

MMenuFileDescriptions
"&Описания файлов"
"File &descriptions"
"Popi&sy souborů"
"&Dateibeschreibungen"

MMenuFolderInfoFiles
"Файлы описания п&апок"
"&Folder description files"
"Soubory popisů &adresářů"
"O&rdnerbeschreibungen"

MMenuViewer
"Настройки про&граммы просмотра"
"&Viewer settings"
"Nastavení P&rohlížeče"
"Be&trachter einrichten"

MMenuEditor
"Настройки &редактора"
"&Editor settings"
"Nastavení &Editoru"
"&Editor einrichten"

MMenuColors
"&Цвета"
"Co&lors"
"&Barvy"
"&Farben"

MMenuFilesHighlighting
"Раскраска &файлов и группы сортировки"
"Files &highlighting and sort groups"
"Z&výrazňování souborů a skupiny řazení"
"Farbmar&kierungen und Sortiergruppen"

MMenuSaveSetup
"&Сохранить параметры      Shift-F9"
"&Save setup        Shift-F9"
"&Uložit nastavení                         Shift-F9"
"Setup &speichern   Umsch-F9"

MMenuTogglePanelRight
"Панель &Вкл/Выкл          Ctrl-F2"
"Panel &On/Off       Ctrl-F2"
"Panel &Zap/Vyp            Ctrl-F2"
"Panel &ein/aus        Strg-F2"

MMenuChangeDriveRight
"С&менить диск             Alt-F2"
"&Change drive       Alt-F2"
"Z&měnit jednotku          Alt-F2"
"Laufwerk &wechseln    Alt-F2"

MMenuLeftTitle
l:
"&Левая"
"&Left"
"&Levý"
"&Links"

MMenuFilesTitle
"&Файлы"
"&Files"
"&Soubory"
"&Dateien"

MMenuCommandsTitle
"&Команды"
"&Commands"
"Pří&kazy"
"&Befehle"

MMenuOptionsTitle
"Па&раметры"
"&Options"
"&Nastavení"
"&Optionen"

MMenuRightTitle
"&Правая"
"&Right"
"&Pravý"
"&Rechts"

MMenuSortTitle
l:
"Критерий сортировки"
"Sort by"
"Seřadit podle"
"Sortieren nach"

MMenuSortByName
"&Имя                              Ctrl-F3"
"&Name                 Ctrl-F3"
"&Názvu                     Ctrl-F3"
"&Name                   Strg-F3"

MMenuSortByExt
"&Расширение                       Ctrl-F4"
"E&xtension            Ctrl-F4"
"&Přípony                   Ctrl-F4"
"&Erweiterung            Strg-F4"

MMenuSortByModification
"Время &модификации                Ctrl-F5"
"&Modification time    Ctrl-F5"
"Č&asu modifikace           Ctrl-F5"
"&Veränderungsdatum      Strg-F5"

MMenuSortBySize
"Р&азмер                           Ctrl-F6"
"&Size                 Ctrl-F6"
"&Velikosti                 Ctrl-F6"
"&Größe                  Strg-F6"

MMenuUnsorted
"&Не сортировать                   Ctrl-F7"
"&Unsorted             Ctrl-F7"
"N&eřadit                   Ctrl-F7"
"&Unsortiert             Strg-F7"

MMenuSortByCreation
"Время &создания                   Ctrl-F8"
"&Creation time        Ctrl-F8"
"&Data vytvoření            Ctrl-F8"
"E&rstelldatum           Strg-F8"

MMenuSortByAccess
"Время &доступа                    Ctrl-F9"
"&Access time          Ctrl-F9"
"Ča&su přístupu             Ctrl-F9"
"&Zugriffsdatum          Strg-F9"

MMenuSortByDiz
"&Описания                         Ctrl-F10"
"&Descriptions         Ctrl-F10"
"P&opisků                   Ctrl-F10"
"&Beschreibungen         Strg-F10"

MMenuSortByOwner
"&Владельцы файлов                 Ctrl-F11"
"&Owner                Ctrl-F11"
"V&lastníka                 Ctrl-F11"
"Bes&itzer               Strg-F11"

MMenuSortByCompressedSize
"&Упакованный размер"
"Com&pressed size"
"&Komprimované velikosti"
"Kom&primierte Größe"

MMenuSortByNumLinks
"Ко&личество ссылок"
"Number of &hard links"
"Poč&tu pevných linků"
"Anzahl an &Links"

MMenuSortUseGroups
"Использовать &группы сортировки   Shift-F11"
"Use sort &groups      Shift-F11"
"Řazení podle skup&in       Shift-F11"
"Sortier&gruppen verw.   Umsch-F11"

MMenuSortSelectedFirst
"&Помеченные файлы вперед          Shift-F12"
"Show selected &first  Shift-F12"
"Nejdřív zobrazit vy&brané  Shift-F12"
"&Ausgewählte zuerst     Umsch-F12"

MMenuSortUseNumeric
"Использовать &числовую сортировку"
"Use num&eric sort"
"Použít čí&selné řazení"
"Nu&merische Sortierung"

MChangeDriveTitle
l:
"Диск"
"Drive"
"Jednotka   "
"Laufwerke"

MChangeDriveRemovable
"сменный  "
"removable"
"vyměnitelná"
"wechsel. "

MChangeDriveFixed
"жесткий  "
"fixed    "
"pevná      "
"fest     "

MChangeDriveNetwork
"сетевой  "
"network  "
"síťová     "
"Netzwerk "

MChangeDriveCDROM
"CD-ROM   "
"CD-ROM   "
"CD-ROM     "
"CD-ROM   "

MChangeDriveCD_RW
"CD-RW    "
"CD-RW    "
"CD-RW      "
"CD-RW    "

MChangeDriveCD_RWDVD
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD  "
"CD-RW/DVD"

MChangeDriveDVD_ROM
"DVD-ROM  "
"DVD-ROM  "
"DVD-ROM    "
"DVD-ROM  "

MChangeDriveDVD_RW
"DVD-RW   "
"DVD-RW   "
"DWD-RW     "
"DVD-RW   "

MChangeDriveDVD_RAM
"DVD-RAM  "
"DVD-RAM  "
"DVD-RAM    "
"DVD-RAM  "

MChangeDriveRAM
"RAM диск "
"RAM disk "
"RAM disk   "
"RAM-DISK "

MChangeDriveSUBST
"SUBST    "
"subst    "
"SUBST      "
"Subst    "

MChangeDriveLabelAbsent
"недоступен"
"not available"
"není k dispozici"
"nicht vorh.  "

MChangeDriveCannotReadDisk
"Ошибка чтения диска в дисководе %c:"
"Cannot read the disk in drive %c:"
"Nelze přečíst disk v jednotce %c:"
"Datenträge in Laufwerk %c: kann nicht gelesen werden."

MChangeDriveCannotDisconnect
"Не удается отсоединиться от %s"
"Cannot disconnect from %s"
"Nelze se odpojit od %s"
"Verbindung zu %s konnte nicht getrennt werden."

MChangeDriveCannotDelSubst
"Не удается удалить виртуальный драйвер %s"
"Cannot delete a substituted drive %s"
"Nelze smazat substnutá jednotka %s"
"Substlaufwerk %s konnte nicht gelöscht werden."

MChangeDriveOpenFiles
"Если вы не закроете открытые файлы, данные могут быть утеряны"
"If you do not close the open files, data may be lost."
"Pokud neuzavřete otevřené soubory, mohou být tato data ztracena."
"Wenn Sie offene Dateien nicht schließen könnten Daten verloren gehen."

MChangeSUBSTDisconnectDriveTitle
l:
"Отключение виртуального драйвера"
"Virtual device disconnection"
"Odpojování virtuálního zařízení"
"Virtuelles Gerät trennen"

MChangeSUBSTDisconnectDriveQuestion
"Отключить SUBST-диск %c:?"
"Disconnect SUBST-disk %c:?"
"Odpojit SUBST-disk %c:?"
"Substlaufwerk %c: trennen?"

MChangeHotPlugDisconnectDriveTitle
l:
"Удаление устройства"
"Device Removal"
"Odpojování zařízení"
"Sicheres Entfernen"

MChangeHotPlugDisconnectDriveQuestion
"Вы хотите удалить устройство"
"Do you want to remove the device"
"Opravdu si přejete odpojit zařízení"
"Wollen Sie folgendes Gerät sicher entfernen? "

MHotPlugDisks
"(диск(и): %s)"
"(disk(s): %s)"
"(disk(y): %s)"
"(Laufwerk(e): %s)"

MChangeCouldNotEjectHotPlugMedia
"Невозможно удалить устройство для диска %c:"
"Cannot remove a device for drive %c:"
"Zařízení %c: nemůže být odpojeno."
"Ein Gerät für Laufwerk %c: konnte nicht entfernt werden"

MChangeCouldNotEjectHotPlugMedia2
"Невозможно удалить устройство:"
"Cannot remove a device:"
"Zařízení nemůže být odpojeno."
"Kann folgendes Geräte nicht entfernen:"

MChangeHotPlugNotify1
"Теперь устройство" 
"The device" 
"Zařízení"
"Das Gerät"

MChangeHotPlugNotify2
"может быть безопасно извлечено из компьютера"
"can now be safely removed"
"může být nyní bezpečně odebráno"
"kann nun vom Computer getrennt werden."

MHotPlugListTitle
"Hotplug-устройства"
"Hotplug devices list"
"Seznam vyjímatelných zařízení"
"Hardware sicher entfernen"

MHotPlugListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Tasten: Entf,StrgR,F1"

MChangeDriveDisconnectTitle
l:
"Отключение сетевого устройства"
"Disconnect network drive"
"Odpojit síťovou jednotku"
"Netzwerklaufwerk trennen"

MChangeDriveDisconnectQuestion
"Вы хотите удалить соединение с устройством %c:?"
"Do you want to disconnect from the drive %c:?"
"Opravdu si přejete odpojit od jednotky %c:?"
"Wollen Sie die Verbindung zu Laufwerk %c: trennen?"

MChangeDriveDisconnectMapped
"На устройство %c: отображена папка"
"The drive %c: is mapped to..."
"Jednotka %c: je namapována na..."
"Laufwerk %c: ist verknüpft zu..."

MChangeDriveDisconnectReconnect
"&Восстанавливать при входе в систему"
"&Reconnect at Logon"
"&Znovu připojit při přihlášení"
"Bei Anmeldung &verbinden"

MChangeDriveAskDisconnect
l:
"Вы хотите в любом случае отключиться от устройства?"
"Do you want to disconnect the device anyway?"
"Přejete si přesto zařízení odpojit?"
"Wollen Sie die Verbindung trotzdem trennen?"

MChangeVolumeInUse
"Не удается извлечь диск из привода %c:"
"Volume %c: cannot be ejected."
"Jednotka %c: nemůže být vysunuta."
"Datenträger %c: kann nicht ausgeworfen werden."

MChangeVolumeInUse2
"Используется другим приложением"
"It is used by another application"
"Je používaná jinou aplikací"
"Andere Programme greifen momentan darauf zu"

MChangeWaitingLoadDisk
ls:;"Ожидание загрузки диска..."
"Ожидание чтения диска..."
ls:;"Waiting for disk to load..."
"Waiting for disk to mount..."
ls:;"Nahrávání disku..."
"Čekám na disk k připojení..."
"Warte auf Datenträger..."

MChangeCouldNotEjectMedia
"Невозможно извлечь диск из привода %c:"
"Could not eject media from drive %c:"
"Nelze vysunout médium v jednotce %c:"
"Konnte Medium in Laufwerk %c: nicht auswerfen"

MAdditionalHotKey
"#!$%*+-/(),."
"#!$%*+-/(),."
"#!$%*+-/(),."
"#!$%*+-/(),."

MSearchFileTitle
l:
" Поиск "
" Search "
" Hledat "
" Suchen "

MCannotCreateListFile
"Ошибка создания списка файлов"
"Cannot create list file"
"Nelze vytvořit soubor se seznamem"
"Dateiliste konnte nicht erstellt werden"

MCannotCreateListTemp
"(невозможно создать временный файл для списка)"
"(cannot create temporary file for list)"
"(nemohu vytvořit dočasný soubor pro seznam)"
"(Fehler beim Anlegen einer temporären Datei für Liste)"

MCannotCreateListWrite
"(невозможно записать данные в файл)"
"(cannot write data in file)"
"(nemohu zapsat data do souboru)"
"(Fehler beim Schreiben der Daten)"

MDragFiles
l:
"%d файлов"
"%d files"
"%d souborů"
"%d Dateien"

MDragMove
"Перенос %s"
"Move %s"
"Přesunout %s"
"Verschiebe %s"

MDragCopy
"Копирование %s"
"Copy %s"
"Kopírovat %s"
"Kopiere %s"

MProcessListTitle
l:
"Список задач"
"Task list"
"Seznam úloh"
"Taskliste"

MProcessListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Tasten: Entf,StrgR"

MKillProcessTitle
"Удаление задачи"
"Kill task"
"Zabít úlohu"
"Task beenden"

MAskKillProcess
"Вы хотите удалить выбранную задачу?"
"Do you wish to kill selected task?"
"Přejete si zabít vybranou úlohu?"
"Wollen Sie den ausgewählten Task beenden?"

MKillProcessWarning
"Вы потеряете всю несохраненную информацию этой программы"
"You will lose any unsaved information in this program"
"V tomto programu budou ztraceny neuložené informace"
"Alle ungespeicherten Daten dieses Programmes gehen verloren."

MKillProcessKill
"Удалить"
"Kill"
"Zabít"
"Beenden"

MCannotKillProcess
"Указанную задачу удалить не удалось"
"Cannot kill the specified task"
"Nemohu ukončit zvolenou úlohu"
"Task konnte nicht beendet werden"

MCannotKillProcessPerm
"Вы не имеет права удалить этот процесс."
"You have no permission to kill this process."
"Nemáte oprávnění zabít tento proces."
"Sie haben keine Rechte um diesen Prozess zu beenden."

MQuickViewTitle
l:
"Быстрый просмотр"
"Quick view"
"Zběžné zobrazení"
"Schnellansicht"

MQuickViewFolder
"Папка \"%s\""
"Folder \"%s\""
"Adresář \"%s\""
"Verzeichnis \"%s\""

MQuickViewJunction
"Связь \"%s\""
"Junction \"%s\""
"Křížení \"%s\""
"Knotenpunkt \"%s\""

MQuickViewSymlink
"Ссылка \"%s\""
"Symlink \"%s\""
"Symbolický link \"%s\""
"Symlink \"%s\""

MQuickViewVolMount
"Том \"%s\""
"Volume \"%s\""
"Svazek \"%s\""
"Datenträger \"%s\""

MQuickViewContains
"Содержит:"
"Contains:"
"Obsah:"
"Enthält:"

MQuickViewFolders
"Папок               "
"Folders          "
"Adresáře           "
"Ordner           "

MQuickViewFiles
"Файлов              "
"Files            "
"Soubory            "
"Dateien          "

MQuickViewBytes
"Размер файлов       "
"Files size       "
"Velikost souborů   "
"Gesamtgröße      "

MQuickViewCompressed
"Упакованный размер  "
"Compressed size  "
"Komprim. velikost  "
"Komprimiert      "

MQuickViewRatio
"Степень сжатия      "
"Ratio            "
"Poměr              "
"Rate             "

MQuickViewCluster
"Размер кластера     "
"Cluster size     "
"Velikost clusteru  "
"Clustergröße     "

MQuickViewRealSize
"Реальный размер     "
"Real files size  "
"Opravdová velikost "
"Tatsächlich      "

MQuickViewSlack
"Остатки кластеров   "
"Files slack      "
"Mrtvé místo        "
"Verlust          "

MSetAttrTitle
l:
"Атрибуты"
"Attributes"
"Atributy"
"Attribute"

MSetAttrFor
"Изменить файловые атрибуты"
"Change file attributes for"
"Změna atributů souboru pro"
"Ändere Dateiattribute für"

MSetAttrSelectedObjects
"выбранных объектов"
"selected objects"
"vybrané objekty"
"markierte Objekte"

MSetAttrHardLinks
"жестких ссылок (%d)"
"hard links (%d)"
"pevné linky (%d)"
"Hardlinks (%d)"

MSetAttrJunction
"Связь \"%s\""
"Junction \"%s\""
"Křížení \"%s\""
"Knotenpunkte \"%s\""

MSetAttrSymlink
"Ссылка \"%s\""
"Symlink \"%s\""
"Link \"%s\""
"Symlink \"%s\""

MSetAttrVolMount
"Том \"%s\""
"Volume \"%s\""
"Svazek \"%s\""
"Datenträger \"%s\""

MSetAttrUnknownJunction
"(нет данных)"
"(data not available)"
"(data nejsou k dispozici)"
"(nicht verfügbar)"

MSetAttrRO
"&Только для чтения"
"&Read only"
"&Pouze pro čtení"
"Sch&reibschutz"

MSetAttrArchive
"&Архивный"
"&Archive"
"&Archivovat"
"&Archiv"

MSetAttrHidden
"&Скрытый"
"&Hidden"
"&Skrytý"
"&Versteckt"

MSetAttrSystem
"С&истемный"
"&System"
"S&ystémový"
"&System"

MSetAttrCompressed
"Сжаты&й"
"&Compressed"
"&Komprimovaný"
"&Komprimiert"

MSetAttrEncrypted
"За&шифрованный"
"&Encrypted"
"&Šifrovaný"
"V&erschlüsselt"

MSetAttrNotIndexed
"Н&еиндексируемый"
"Not &Indexed"
"Neinde&xovaný"
"Nicht &indiziert"

MSetAttrSparse
"Разреженный"
"Sparse"
"Rozptýlený"
"Reserve"

MSetAttrTemp
"Временный"
"Temporary"
"Dočasný"
"Temporär"

MSetAttrOffline
"Автономный"
"Offline"
"Offline"
"Offline"

MSetAttrVirtual
"Виртуальный"
"Virtual"
"Virtuální"
"Virtuell"

MSetAttrSubfolders
"Обрабатывать &вложенные папки"
"Process sub&folders"
"Zpracovat i po&dadresáře"
"Unterordner miteinbe&ziehen"

MSetAttrModification
"Время &модификации файла:"
"File &modification time:"
"Čas &modifikace souboru:"
"Datei &modifiziert:"

MSetAttrCreation
"Время со&здания файла:"
"File crea&tion time:"
"Čas v&ytvoření souboru:"
"Datei erstell&t:"

MSetAttrLastAccess
"Время последнего &доступа к файлу:"
"&Last file access time:"
"Čas posledního pří&stupu:"
"&Letzter Zugriff:"

MSetAttrOriginal
"Исход&ное"
"&Original"
"&Originál"
"&Original"

MSetAttrCurrent
"Те&кущее"
"Curre&nt"
"So&učasný"
"Akt&uell"

MSetAttrBlank
"Сбр&ос"
"&Blank"
"P&rázdný"
"L&eer"

MSetAttrSet
"Установить"
"Set"
"Nastavit"
"Setzen"

MSetAttrTimeTitle1
l:
"ММ%cДД%cГГГГ чч%cмм%cсс"
"MM%cDD%cYYYY hh%cmm%css"
"MM%cDD%cRRRR hh%cmm%css"
"MM%cTT%cJJJJ hh%cmm%css"

MSetAttrTimeTitle2
"ДД%cММ%cГГГГ чч%cмм%cсс"
"DD%cMM%cYYYY hh%cmm%css"
"DD%cMM%cRRRR hh%cmm%css"
"TT%cMM%cJJJJ hh%cmm%css"

MSetAttrTimeTitle3
"ГГГГ%cММ%cДД чч%cмм%cсс"
"YYYY%cMM%cDD hh%cmm%css"
"RRRR%cMM%cDD hh%cmm%css"
"JJJJ%cMM%cTT hh%cmm%css"

MSetAttrSetting
l:
"Установка файловых атрибутов для"
"Setting file attributes for"
"Nastavení atributů souboru pro"
"Setze Dateiattribute für"

MSetAttrCannotFor
"Ошибка установки атрибутов для"
"Cannot set attributes for"
"Nelze nastavit atributy pro"
"Konnte Dateiattribute nicht setzen für"

MSetAttrCompressedCannotFor
"Не удалось установить атрибут СЖАТЫЙ для"
"Cannot set attribute COMPRESSED for"
"Nelze nastavit atribut KOMPRIMOVANÝ pro"
"Konnte Komprimierung nicht setzen für"

MSetAttrEncryptedCannotFor
"Не удалось установить атрибут ЗАШИФРОВАННЫЙ для"
"Cannot set attribute ENCRYPTED for"
"Nelze nastavit atribut ŠIFROVANÝ pro"
"Konnte Verschlüsselung nicht setzen für"

MSetAttrTimeCannotFor
"Не удалось установить время файла для"
"Cannot set file time for"
"Nelze nastavit čas souboru pro"
"Konnte Dateidatum nicht setzen für"

MSetColorPanel
l:
"&Панель"
"&Panel"
"&Panel"
"&Panel"

MSetColorDialog
"&Диалог"
"&Dialog"
"&Dialog"
"&Dialog"

MSetColorWarning
"Пр&едупреждение"
"&Warning message"
"&Varovná zpráva"
"&Warnmeldung"

MSetColorMenu
"&Меню"
"&Menu"
"&Menu"
"&Menü"

MSetColorHMenu
"&Горизонтальное меню"
"Hori&zontal menu"
"Hori&zontální menu"
"Hori&zontales Menü"

MSetColorKeyBar
"&Линейка клавиш"
"&Key bar"
"&Řádek kláves"
"

MSetColorCommandLine
"&Командная строка"
"&Command line"
"Pří&kazový řádek"
"&Kommandozeile"

MSetColorClock
"&Часы"
"C&lock"
"&Hodiny"
"U&hr"

MSetColorViewer
"Про&смотрщик"
"&Viewer"
"P&rohlížeč"
"&Betrachter"

MSetColorEditor
"&Редактор"
"&Editor"
"&Editor"
"&Editor"

MSetColorHelp
"П&омощь"
"&Help"
"&Nápověda"
"&Hilfe"

MSetDefaultColors
"&Установить стандартные цвета"
"Set de&fault colors"
"N&astavit výchozí barvy"
"Setze Standard&farben"

MSetBW
"Черно-бел&ый режим"
"&Black and white mode"
"Černo&bílý mód"
"Schwarz && &Weiß"

MSetColorPanelNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"

MSetColorPanelSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"

MSetColorPanelHighlightedInfo
"Выделенная информация"
"Highlighted info"
"Info zvýrazněné"
"Markierung"

MSetColorPanelDragging
"Перетаскиваемый текст"
"Dragging text"
"Tažený text"
"Drag && Drop Text"

MSetColorPanelBox
"Рамка"
"Border"
"Okraj"
"Rahmen"

MSetColorPanelNormalCursor
"Обычный курсор"
"Normal cursor"
"Normální kurzor"
"Normale Auswahl"

MSetColorPanelSelectedCursor
"Выделенный курсор"
"Selected cursor"
"Vybraný kurzor"
"Markierte Auswahl"

MSetColorPanelNormalTitle
"Обычный заголовок"
"Normal title"
"Normální nadpis"
"Normaler Titel"

MSetColorPanelSelectedTitle
"Выделенный заголовок"
"Selected title"
"Vybraný nadpis"
"Markierter Titel"

MSetColorPanelColumnTitle
"Заголовок колонки"
"Column title"
"Nadpis sloupce"
"Spaltentitel"

MSetColorPanelTotalInfo
"Количество файлов"
"Total info"
"Info celkové"
"Gesamtinfo"

MSetColorPanelSelectedInfo
"Количество выбранных файлов"
"Selected info"
"Info výběr"
"Markierungsinfo"

MSetColorPanelScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"

MSetColorPanelScreensNumber
"Количество фоновых экранов"
"Number of background screens"
"Počet obrazovek na pozadí"
"Anzahl an Hintergrundseiten"

MSetColorDialogNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"

MSetColorDialogHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierter Text"

MSetColorDialogDisabled
"Блокированный текст"
"Disabled text"
"Zakázaný text"
"Deaktivierter Text"

MSetColorDialogBox
"Рамка"
"Border"
"Okraj"
"Rahmen"

MSetColorDialogBoxTitle
"Заголовок рамки"
"Title"
"Nadpis"
"Titel"

MSetColorDialogHighlightedBoxTitle
"Выделенный заголовок рамки"
"Highlighted title"
"Zvýrazněný nadpis"
"Markierter Titel"

MSetColorDialogTextInput
"Ввод текста"
"Text input"
"Textový vstup"
"Texteingabe"

MSetColorDialogUnchangedTextInput
"Неизмененный текст"
"Unchanged text input"
"Nezměněný textový vstup"
"Unveränderte Texteingabe"

MSetColorDialogSelectedTextInput
"Ввод выделенного текста"
"Selected text input"
"Vybraný textový vstup"
"Markierte Texteingabe"

MSetColorDialogEditDisabled
"Блокированное поле ввода"
"Disabled input line"
"Zakázaný vstupní řádek"
"Deaktivierte Eingabezeile"

MSetColorDialogButtons
"Кнопки"
"Buttons"
"Tlačítka"
"Schaltflächen"

MSetColorDialogSelectedButtons
"Выбранные кнопки"
"Selected buttons"
"Vybraná tlačítka"
"Aktive Schaltflächen"

MSetColorDialogHighlightedButtons
"Выделенные кнопки"
"Highlighted buttons"
"Zvýrazněná tlačítka"
"Markierte Schaltflächen"

MSetColorDialogSelectedHighlightedButtons
"Выбранные выделенные кнопки"
"Selected highlighted buttons"
"Vybraná zvýrazněná tlačítka"
"Aktive markierte Schaltflächen"

MSetColorDialogListBoxControl
"Список"
"List box"
"Seznam položek"
"Listenfelder"

MSetColorDialogComboBoxControl
"Комбинированный список"
"Combobox"
"Výběr položek"
"Kombinatiosfelder"

MSetColorDialogListText
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"

MSetColorDialogListSelectedText
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"

MSetColorDialogListHighLight
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"

MSetColorDialogListSelectedHighLight
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"
"Aktive Markierung"

MSetColorDialogListDisabled
"Блокированный пункт"
"Disabled item"
"Naktivní položka"
"Deaktiviertes Element"

MSetColorDialogListBox
"Рамка"
"Border"
"Okraj"
"Rahmen"

MSetColorDialogListTitle
"Заголовок"
"Title"
"Nadpis"
"Titel"

MSetColorDialogListScrollBar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"

MSetColorDialogListArrows
"Индикаторы длинных строк"
"Long string indicators"
"Značka dlouhého řetězce"
"Indikator für lange Zeichenketten"

MSetColorDialogListArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"
"Vybraná značka dlouhého řetězce"
"Aktiver Indikator"

MSetColorDialogListArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"
"Zakázaná značka dlouhého řetězce"
"Deaktivierter Indikator"

MSetColorMenuNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"

MSetColorMenuSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"

MSetColorMenuHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"

MSetColorMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"
"Aktive Markierung"

MSetColorMenuDisabled
"Недоступный пункт"
"Disabled text"
"Neaktivní text"
"Disabled text"

MSetColorMenuBox
"Рамка"
"Border"
"Okraj"
"Rahmen"

MSetColorMenuTitle
"Заголовок"
"Title"
"Nadpis"
"Titel"

MSetColorMenuScrollBar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"

MSetColorMenuArrows
"Индикаторы длинных строк"
"Long string indicators"
"Značka dlouhého řetězce"
"Long string indicators"

MSetColorMenuArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"
"Vybraná značka dlouhého řetězce"
"Selected long string indicators"

MSetColorMenuArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"
"Zakázaná značka dlouhého řetězce"
"Disabled long string indicators"

MSetColorHMenuNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"

MSetColorHMenuSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"

MSetColorHMenuHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"

MSetColorHMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"
"Aktive Markierung"

MSetColorKeyBarNumbers
l:
"Номера клавиш"
"Key numbers"
"Čísla kláves"
"Tastenziffern"

MSetColorKeyBarNames
"Названия клавиш"
"Key names"
"Názvy kláves"
"Tastennamen"

MSetColorKeyBarBackground
"Фон"
"Background"
"Pozadí"
"Hintergrund"

MSetColorCommandLineNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"

MSetColorCommandLineSelected
"Выделенный текст"
"Selected text input"
"Vybraný textový vstup"
"Markierte Texteingabe"

MSetColorCommandLinePrefix
"Текст префикса"
"Prefix text"
"Text předpony"
"Prefix Text"

MSetColorCommandLineUserScreen
"Пользовательский экран"
"User screen"
"Obrazovka uživatele"
"Benutzerseite"

MSetColorClockNormal
l:
"Обычный текст (панели)"
"Normal text (Panel)"
"Normální text (Panel)"
"Normaler Text (Panel)"

MSetColorClockNormalEditor
"Обычный текст (редактор)"
"Normal text (Editor)"
"Normální text (Editor)"
"Normaler Text (Editor)"

MSetColorClockNormalViewer
"Обычный текст (вьювер)"
"Normal text (Viewer)"
"Normální text (Prohlížeč)"
"Normaler Text (Betrachter)"

MSetColorViewerNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"

MSetColorViewerSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"

MSetColorViewerStatus
"Статус"
"Status line"
"Stavový řádek"
"Statuszeile"

MSetColorViewerArrows
"Стрелки сдвига экрана"
"Screen scrolling arrows"
"Skrolovací šipky"
"Pfeile auf Scrollbalken"

MSetColorViewerScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"

MSetColorEditorNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"

MSetColorEditorSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"

MSetColorEditorStatus
"Статус"
"Status line"
"Stavový řádek"
"Statuszeile"

MSetColorEditorScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"

MSetColorHelpNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"

MSetColorHelpHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"

MSetColorHelpReference
"Ссылка"
"Reference"
"Odkaz"
"Referenz"

MSetColorHelpSelectedReference
"Выбранная ссылка"
"Selected reference"
"Vybraný odkaz"
"Ausgewählte Referenz"

MSetColorHelpBox
"Рамка"
"Border"
"Okraj"
"Rahmen"

MSetColorHelpBoxTitle
"Заголовок рамки"
"Title"
"Nadpis"
"Titel"

MSetColorHelpScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"

MSetColorGroupsTitle
l:
"Цветовые группы"
"Color groups"
"Skupiny barev"
"Farbgruppen"

MSetColorItemsTitle
"Элементы группы"
"Group items"
"Položky skupin"
"Gruppeneinträge"

MSetColorTitle
l:
"Цвет"
"Color"
"Barva"
"Farbe"

MSetColorForeground
"&Текст"
"&Foreground"
"&Popředí"
"&Vordergrund"

MSetColorBackground
"&Фон"
"&Background"
"Po&zadí"
"&Hintergrund"

MSetColorForeTransparent
"&Прозрачный"
"&Transparent"
"Průhlednos&t"
"&Transparent"

MSetColorBackTransparent
"П&розрачный"
"T&ransparent"
"Průhledno&st"
"T&ransparent"

MSetColorSample
"Текст Текст Текст Текст Текст Текст"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"

MSetColorSet
"Установить"
"Set"
"Nastavit"
"Setzen"

MSetColorCancel
"Отменить"
"Cancel"
"Storno"
"Abbruch"

MSetConfirmTitle
l:
"Подтверждения"
"Confirmations"
"Potvrzení"
"Bestätigungen"

MSetConfirmCopy
"Перезапись файлов при &копировании"
"&Copy"
"&Kopírování"
"&Kopieren"

MSetConfirmMove
"Перезапись файлов при &переносе"
"&Move"
"&Přesouvání"
"&Verschieben"

MSetConfirmDrag
"Пере&таскивание"
"&Drag and drop"
"&Drag and drop"
"&Ziehen und Ablegen"

MSetConfirmDelete
"&Удаление"
"De&lete"
"&Mazání"
"&Löschen"

MSetConfirmDeleteFolders
"У&даление непустых папок"
"Delete non-empty &folders"
"Mazat &neprázdné adresáře"
"Löschen von Ordnern mit &Inhalt"

MSetConfirmEsc
"Прерыва&ние операций (Esc)"
"&Interrupt operation"
"Pře&rušit operaci"
"&Unterbrechen von Vorgängen"

MSetConfirmRemoveConnection
"&Отключение сетевого устройства"
"Disconnect &network drive"
"Odpojení &síťové jednotky"
"Trennen von &Netzwerklaufwerken"

MSetConfirmRemoveSUBST
"Отключение SUBST-диска"
"Disconnect &SUBST-disk"
"Odpojení SUBST-d&isku"
"Trennen von &Substlaufwerken"

MSetConfirmRemoveHotPlug
"Отключение HotPlug-устройства"
"HotPlug-device removal"
"Odpojení vyjímatelného zařízení"
"Sicheres Entfernen von Hardware"

MSetConfirmAllowReedit
"Повто&рное открытие файла в редакторе"
"&Reload edited file"
"&Obnovit upravovaný soubor"
"Bea&rbeitete Datei neu laden"

MSetConfirmHistoryClear
"Очистка списка &истории"
"Clear &history list"
"Vymazat seznam &historie"
"&Historielisten löschen"

MSetConfirmExit
"&Выход"
"E&xit"
"U&končení"
"Be&enden"

MFindFolderTitle
l:
"Поиск папки"
"Find folder"
"Najít adresář"
"Ordner finden"

MKBFolderTreeF1
l:
l:// Find folder Tree KeyBar
"Помощь"
"Help"
"Nápověda"
"Hilfe"

MKBFolderTreeF2
"Обновить"
"Rescan"
"Obnovit"
"Aktual"

MKBFolderTreeF5
"Размер"
"Zoom"
"Zoom"
"Vergr."

MKBFolderTreeF10
"Выход"
"Quit"
"Konec"
"Ende"

MKBFolderTreeAltF9
"Видео"
"Video"
"Video"
"Vollb"

MTreeTitle
"Дерево"
"Tree"
"Stromové zobrazení"
"Baum"

MCannotSaveTree
"Ошибка записи дерева папок в файл"
"Cannot save folders tree to file"
"Adresářový strom nelze uložit do souboru"
"Konnte Ordnerliste nicht in Datei speichern."

MReadingTree
"Чтение дерева папок"
"Reading the folders tree"
"Načítám adresářový strom"
"Lese Ordnerliste"

MUserMenuTitle
l:
"Пользовательское меню"
"User menu"
"Menu uživatele"
"Benutzermenü"

MChooseMenuType
"Выберите тип пользовательского меню для редактирования"
"Choose user menu type to edit"
"Zvol typ menu uživatele pro úpravu"
"Wählen Sie den Typ des zu editierenden Benutzermenüs"

MChooseMenuMain
"&Главное"
"&Main"
"&Hlavní"
"&Hauptmenü"

MChooseMenuLocal
"&Местное"
"&Local"
"&Lokální"
"&Lokales Menü"

MMainMenuTitle
"Главное меню"
"Main menu"
"Hlavní menu"
"Hauptmenü"

MMainMenuFAR
"Папка FAR"
"FAR folder"
"Složka FARu"
"FAR Ordner"

MMainMenuREG
l:
l:// <...menu (Registry)>
"Реестр"
"Registry"
"Registry"
"Reg."

MLocalMenuTitle
"Местное меню"
"Local menu"
"Lokalní menu"
"Lokales Menü"

MMainMenuBottomTitle
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Bearb.: Entf,Einf,F4"

MAskDeleteMenuItem
"Вы хотите удалить пункт меню"
"Do you wish to delete the menu item"
"Přejete si smazat položku v menu"
"Do you wish to delete the menu item"

MAskDeleteSubMenuItem
"Вы хотите удалить вложенное меню"
"Do you wish to delete the submenu"
"Přejete si smazat podmenu"
"Do you wish to delete the submenu"

MUserMenuInvalidInputLabel
"Неправильный формат метки меню!"
"Invalid format for UserMenu Label!"
"Neplatný formát pro název Uživatelského menu!"
"Invalid format for UserMenu Label!"

MUserMenuInvalidInputHotKey
"Неправильный формат горячей клавиши!"
"Invalid format for Hot Key!"
"Neplatný formát pro klávesovou zkratku!"
"Invalid format for Hot Key!"

MEditMenuTitle
l:
"Редактирование пользовательского меню"
"Edit user menu"
"Editace uživatelského menu"
"Menübefehl bearbeiten"

MEditMenuHotKey
"&Горячая клавиша:"
"&Hot key:"
"K&lávesová zkratka:"
"&Kurztaste:"

MEditMenuLabel
"&Метка:"
"&Label:"
"&Popisek:"
"&Bezeichnung:"

MEditMenuCommands
"&Команды:"
"&Commands:"
"Pří&kazy:"
"&Befehle:"

MAskInsertMenuOrCommand
l:
"Вы хотите вставить новую команду или новое меню?"
"Do you wish to insert a new command or a new menu?"
"Přejete si vložit nový příkaz nebo nové menu?"
"Wollen Sie einen neuen Menübefehl oder ein neues Menu erstellen?"

MMenuInsertCommand
"Вставить команду"
"Insert command"
"Vložit příkaz"
"Neuer Befehl"

MMenuInsertMenu
"Вставить меню"
"Insert menu"
"Vložit menu"
"Neues Menü"

MEditSubmenuTitle
l:
"Редактирование метки вложенного меню"
"Edit submenu label"
"Úprava popisku podmenu"
"Untermenü bearbeiten"

MEditSubmenuHotKey
"&Горячая клавиша:"
"&Hot key:"
"Klávesová &zkratka:"
"&Kurztaste:"

MEditSubmenuLabel
"&Метка:"
"&Label:"
"&Popisek:"
"&Bezeichnung:"

MViewerTitle
l:
"Просмотр"
"Viewer"
"Prohlížeč"
"Betrachter"

MViewerCannotOpenFile
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"

MViewerStatusCol
"Кол"
"Col"
"Sloupec"
"Spalte"

MViewSearchTitle
l:
"Поиск"
"Search"
"Hledat"
"Durchsuchen"

MViewSearchFor
"&Искать"
"&Search for"
"H&ledat"
"&Suchen nach"

MViewSearchForText
"Искать &текст"
"Search for &text"
"Hledat &text"
"Suchen nach &Text"

MViewSearchForHex
"Искать 16-ричный &код"
"Search for &hex"
"Hledat he&x"
"Suchen nach &Hex (xx xx ...)"

MViewSearchCase
"&Учитывать регистр"
"&Case sensitive"
"&Rozlišovat velikost písmen"
"Gr&oß-/Kleinschreibung"

MViewSearchWholeWords
"Только &целые слова"
"&Whole words"
"Celá &slova"
"Ganze &Wörter"

MViewSearchReverse
"Обратн&ый поиск"
"Re&verse search"
"&Zpětné hledání"
"Richtung um&kehren"

MViewSearchSearch
"Искать"
"Search"
"Hledat"
"Suchen"

MViewSearchCancel
"Отменить"
"Cancel"
"Storno"
"Abbrechen"

MViewSearchingFor
l:
"Поиск"
"Searching for"
"Vyhledávám"
"Suche nach"

MViewSearchingHex
"Поиск байтов"
"Searching for bytes"
"Vyhledávám sekvenci bytů"
"Suche nach Bytes"

MViewSearchCannotFind
"Строка не найдена"
"Could not find the string"
"Nelze najít řetězec"
"Konnte Zeichenkette nicht finden"

MViewSearchCannotFindHex
"Байты не найдены"
"Could not find the bytes"
"Nelze najít sekvenci bytů"
"Konnte Bytefolge nicht finden"

MViewSearchFromBegin
"Продолжить поиск с начала документа?"
"Continue the search from the beginning of the document?"
"Pokračovat s hledáním od začátku dokumentu?"
"Mit Suche am Anfang des Dokuments fortfahren?"

MViewSearchFromEnd
"Продолжить поиск с конца документа?"
"Continue the search from the end of the document?"
"Pokračovat s hledáním od konce dokumentu?"
"Mit Suche am Ende des Dokuments fortfahren?"

MPrintTitle
l:
"Печать"
"Print"
"Tisk"
"Drucken"

MPrintTo
"Печатать %s на"
"Print %s to"
"Vytisknout %s na"
"Drucke %s nach"

MPrintFilesTo
"Печатать %d файлов на"
"Print %d files to"
"Vytisknout %d souborů na"
"Drucke %d Dateien mit"

MPreparingForPrinting
"Подготовка файлов к печати"
"Preparing files for printing"
"Připravuji soubory pro tisk"
"Vorbereiten der Druckaufträge"

MJobs
"заданий"
"jobs"
"úlohy"
"Aufträge"

MCannotOpenPrinter
"Не удалось открыть принтер"
"Cannot open printer"
"Nelze otevřít tiskárnu"
"Fehler beim öffnen des Druckers"

MCannotPrint
"Не удалось распечатать"
"Cannot print"
"Nelze tisknout"
"Fehler beim Drucken"

MDescribeFiles
l:
"Описание файла"
"Describe file"
"Popiskový soubor"
"Beschreibung ändern"

MEnterDescription
"Введите описание %s"
"Enter %s description"
"Zadejte popisek %s"
"Beschreibung für %s:"

MReadingDiz
l:
"Чтение описаний файлов"
"Reading file descriptions"
"Načítám popisky souboru"
"Lese Dateibeschreibungen"

MCannotUpdateDiz
"Не удалось обновить описания файлов"
"Cannot update file descriptions"
"Nelze aktualizovat popisky souboru"
"Dateibeschreibungen konnten nicht aktualisiert werden."

MCannotUpdateRODiz
"Файл описаний защищен от записи"
"The description file is read only"
"Popiskový soubor má atribut Jen pro čtení"
"Die Beschreibungsdatei ist schreibgeschützt."

MCfgDizTitle
l:
"Описания файлов"
"File descriptions"
"Popisky souboru"
"Dateibeschreibungen"

MCfgDizListNames
"Имена &списков описаний, разделенные запятыми:"
"Description &list names delimited with commas:"
"Seznam pop&isových souborů oddělených čárkami:"
"Beschreibungs&dateien, getrennt durch Komma:"

MCfgDizSetHidden
"Устанавливать &атрибут ""Hidden"" на новые списки описаний"
"Set ""&Hidden"" attribute to new description lists"
"Novým souborům s popisy nastavit atribut ""&Skrytý"""
"Setze das '&Versteckt'-Attribut für neu angelegte Dateien"

MCfgDizROUpdate
"Обновлять файл описаний с атрибутом ""Толь&ко для чтения"""
"Update &read only description file"
"Aktualizovat popisové soubory s atributem Jen pro čtení"
"Schreibgeschützte Dateien aktualisie&ren"

MCfgDizStartPos
"&Позиция новых описаний в строке"
"&Position of new descriptions in the string"
"&Pozice nových popisů v řetězci"
"&Position neuer Beschreibungen in der Zeichenkette"

MCfgDizNotUpdate
"&Не обновлять описания"
"Do &not update descriptions"
"&Neaktualizovat popisy"
"Beschreibungen &nie aktualisieren"

MCfgDizUpdateIfDisplayed
"&Обновлять, если они выводятся на экран"
"Update if &displayed"
"Aktualizovat, jestliže je &zobrazen"
"Aktualisieren &wenn angezeigt"

MCfgDizAlwaysUpdate
"&Всегда обновлять"
"&Always update"
"&Vždy aktualizovat"
"Im&mer aktualisieren"

MReadingTitleFiles
l:
"Обновление панелей"
"Update of panels"
"Aktualizace panelů"
"Aktualisiere Panels"

MReadingFiles
"Чтение: %d файлов"
"Reading: %d files"
"Načítám: %d souborů"
"Lese: %d Dateien"

MUserBreakTitle
l:
"Прекращено пользователем"
"User break"
"Přerušeno uživatelem"
"Unterbochen durch Benutzer"

MOperationNotCompleted
"Операция не завершена"
"Operation not completed"
"Operace není dokončena"
"Vorgang nicht abgeschlossen"

MEditPanelModes
l:
"Режимы панели"
"Edit panel modes"
"Editovat módy panelu"
"Anzeigemodi von Panels bearbeiten"

MEditPanelModesBrief
l:
"&Краткий режим"
"&Brief mode"
"&Stručný mód"
"&Kurz"

MEditPanelModesMedium
"&Средний режим"
"&Medium mode"
"S&třední mód"
"&Mittel"

MEditPanelModesFull
"&Полный режим"
"&Full mode"
"&Plný mód"
"&Voll"

MEditPanelModesWide
"&Широкий режим"
"&Wide mode"
"Š&iroký mód"
"B&reitformat"

MEditPanelModesDetailed
"&Детальный режим"
"Detai&led mode"
"Detai&lní mód"
"Detai&lliert"

MEditPanelModesDiz
"&Описания"
"&Descriptions mode"
"P&opiskový mód"
"&Beschreibungen"

MEditPanelModesLongDiz
"Д&линные описания"
"Lon&g descriptions mode"
"&Mód dlouhých popisků"
"Lan&ge Beschreibungen"

MEditPanelModesOwners
"Вл&адельцы файлов"
"File own&ers mode"
"Mód vlastníka so&uborů"
"B&esitzer"

MEditPanelModesLinks
"Свя&зи файлов"
"Lin&ks mode"
"Lin&kový mód"
"Dateilin&ks"

MEditPanelModesAlternative
"Аль&тернативный полный режим"
"&Alternative full mode"
"&Alternativní plný mód"
"&Alternative Vollansicht"

MEditPanelModeTypes
l:
"&Типы колонок"
"Column &types"
"&Typ sloupců"
"Spalten&typen"

MEditPanelModeWidths
"&Ширина колонок"
"Column &widths"
"Šíř&ka sloupců"
"Spalten&breiten"

MEditPanelModeStatusTypes
"Типы колонок строки ст&атуса"
"St&atus line column types"
"T&yp sloupců stavového řádku"
"St&atuszeile Spaltentypen"

MEditPanelModeStatusWidths
"Ширина колонок строки стат&уса"
"Status l&ine column widths"
"Šířka slo&upců stavového řádku"
"Statusze&ile Spaltenbreiten"

MEditPanelModeFullscreen
"&Полноэкранный режим"
"&Fullscreen view"
"&Celoobrazovkový režim"
"&Vollbild"

MEditPanelModeAlignExtensions
"&Выравнивать расширения файлов"
"Align file &extensions"
"Zarovnat příp&ony souborů"
"Datei&erweiterungen ausrichten"

MEditPanelModeAlignFolderExtensions
"Выравнивать расширения пап&ок"
"Align folder e&xtensions"
"Zarovnat přípony adre&sářů"
"Ordnerer&weiterungen ausrichten"

MEditPanelModeFoldersUpperCase
"Показывать папки &заглавными буквами"
"Show folders in &uppercase"
"Zobrazit adresáře &velkými písmeny"
"Ordner in Großb&uchstaben zeigen"

MEditPanelModeFilesLowerCase
"Показывать файлы ст&рочными буквами"
"Show files in &lowercase"
"Zobrazit soubory ma&lými písmeny"
"Dateien in K&leinbuchstaben zeigen"

MEditPanelModeUpperToLowerCase
"Показывать имена файлов из заглавных букв &строчными буквами"
"Show uppercase file names in lower&case"
"Zobrazit velké znaky ve jménech souborů jako &malá písmena"
"G&roßgeschriebene Dateinamen in Kleinbuchstaben zeigen"

MEditPanelModeCaseSensitiveSort
"Использовать р&егистрозависимую сортировку"
"Use case &sensitive sort"
"Použít řazení citlivé na velikost pí&smen"
"&Sortierung abhängig von Groß-/Kleinschreibung"

MEditPanelReadHelp
" Нажмите F1, чтобы получить информацию по настройке "
" Read online help for instructions "
" Pro instrukce si přečtěte online nápovědu "
" Siehe Hilfe für Anweisungen "

MSetFolderInfoTitle
l:
"Файлы информации о папках"
"Folder description files"
"Soubory s popiskem adresáře"
"Ordnerbeschreibungen"

MSetFolderInfoNames
"Введите имена файлов, разделенные запятыми (допускаются маски)"
"Enter file names delimited with commas (wildcards are allowed)"
"Zadejte jména souborů oddělených čárkami (značky jsou povoleny)"
"Dateiliste, getrennt mit Komma (Jokerzeichen möglich):"

MScreensTitle
l:
"Экраны"
"Screens"
"Obrazovky"
"Seiten"

MScreensPanels
"Панели"
"Panels"
"Panely"
"Panels"

MScreensView
"Просмотр"
"View"
"Zobrazit"
"Betr."

MScreensEdit
"Редактор"
"Edit"
"Editovat"
"Bearb"

MAskApplyCommandTitle
l:
"Применить команду"
"Apply command"
"Aplikovat příkaz"
"Befehl anwenden"

MAskApplyCommand
"Введите команду для обработки выбранных файлов"
"Enter command to process selected files"
"Zadejte příkaz pro zpracování vybraných souborů"
"Befehlszeile auf ausgewählte Dateien anwenden:"

MPluginConfigTitle
l:
"Конфигурация модулей"
"Plugins configuration"
"Nastavení Pluginů"
"Konfiguration von Plugins"

MPluginCommandsMenuTitle
"Команды внешних модулей"
"Plugin commands"
"Příkazy pluginů"
"Pluginbefehle"

MPreparingList
l:
"Создание списка файлов"
"Preparing files list"
"Připravuji seznam souborů"
"Dateiliste wird vorbereitet"

MLangTitle
l:
"Основной язык"
"Main language"
"Hlavní jazyk"
"Hauptsprache"

MHelpLangTitle
"Язык помощи"
"Help language"
"Jazyk nápovědy"
"Sprache der Hilfedatei"

MDefineMacroTitle
l:
"Задание макрокоманды"
"Define macro"
"Definovat makro"
"Definiere Makro"

MDefineMacro
"Нажмите желаемую клавишу"
"Press the desired key"
"Stiskněte požadovanou klávesu"
"Tastenkombination:"

MMacroReDefinedKey
"Макроклавиша '%s' уже определена."
"Macro key '%s' already defined."
"Klávesa makra '%s' již je definována."
"Makro '%s' bereits definiert."

MMacroDeleteAssign
"Макроклавиша '%s' не активна."
"Macro key '%s' is not active."
"Klávesa makra '%s' není aktivní."
"Makro '%s' nicht aktiv."

MMacroDeleteKey
"Макроклавиша '%s' будет удалена."
"Macro key '%s' will be removed."
"Klávesa makra '%s' bude odstraněna."
"Makro '%s' wird entfernt und ersetzt:"

MMacroCommonReDefinedKey
"Общая макроклавиша '%s' уже определена."
"Common macro key '%s' already defined."
"Klávesa pro běžné makro '%s' již je definována."
"Gemeinsames Makro '%s' bereits definiert."

MMacroCommonDeleteAssign
"Общая макроклавиша '%s' не активна."
"Common macro key '%s' is not active."
"Klávesa pro běžné makro '%s' není aktivní."
"Gemeinsames Makro '%s' nicht aktiv."

MMacroCommonDeleteKey
"Общая макроклавиша '%s' будет удалена."
"Common macro key '%s' will be removed."
"Klávesa pro běžné makro '%s' bude odstraněna."
"Gemeinsames Makro '%s' wird entfernt und ersetzt:"

MMacroSequence
"Последовательность:"
"Sequence:"
"Posloupnost:"
"Sequenz:"

MMacroReDefinedKey2
"Переопределить?"
"Redefine?"
"Předefinovat?"
"Neu definieren?"

MMacroDeleteKey2
"Удалить?"
"Delete?"
"Odstranit?"
"Löschen?"

MMacroDisDisabledKey
"(макроклавиша не активна)"
"(macro key is not active)"
"(klávesa makra není aktivní)"
"(Makro inaktiv)"

MMacroDisOverwrite
"Переопределить"
"Overwrite"
"Přepsat"
"Überschreiben"

MMacroDisAnotherKey
"Изменить клавишу"
"Try another key"
"Zkusit jinou klávesu"
"Neue Kombination"

MMacroSettingsTitle
l:
"Параметры макрокоманды для '%s'"
"Macro settings for '%s'"
"Nastavení makra pro '%s'"
"Einstellungen für Makro '%s'"

MMacroSettingsEnableOutput
"Разрешить во время &выполнения вывод на экран"
"Allo&w screen output while executing macro"
"Povolit &výstup na obrazovku dokud se provádí makro"
"Bildschirmausgabe &während Makro abläuft"

MMacroSettingsRunAfterStart
"В&ыполнять после запуска FAR"
"Execute after FAR &start"
"&Spustit po spuštění FARu"
"Ausführen beim &Starten von FAR"

MMacroSettingsActivePanel
"&Активная панель"
"&Active panel"
"&Aktivní panel"
"&Aktives Panel"

MMacroSettingsPassivePanel
"&Пассивная панель"
"&Passive panel"
"Pa&sivní panel"
"&Passives Panel"

MMacroSettingsPluginPanel
"На панели пла&гина"
"P&lugin panel"
"Panel p&luginů"
"P&lugin Panel"

MMacroSettingsFolders
"Выполнять для папо&к"
"Execute for &folders"
"Spustit pro ad&resáře"
"Auf Ordnern aus&führen"

MMacroSettingsSelectionPresent
"&Отмечены файлы"
"Se&lection present"
"E&xistující výběr"
"Auswah&l vorhanden"

MMacroSettingsCommandLine
"Пустая командная &строка"
"Empty &command line"
"Prázdný pří&kazový řádek"
"Leere Befehls&zeile"

MMacroSettingsSelectionBlockPresent
"Отмечен б&лок"
"Selection &block present"
"Existující blok výběr&u"
"Mar&kierter Text vorhanden"

MMacroPErrUnrecognized_keyword
l:
"Неизвестное ключевое слово '%s'"
"Unrecognized keyword '%s'"
"Neznámé klíčové slovo '%s'"
"Unbekanntes Schlüsselwort '%s'"

MMacroPErrUnrecognized_function
"Неизвестная функция '%s'"
"Unrecognized function '%s'"
"Neznámá funkce '%s'"
"Unbekannte Funktion '%s'"

MMacroPErrNot_expected_ELSE
"Неожиданное появление $Else"
"Unexpected $Else"
"Neočekávané $Else"
"Unerwartetes $Else"

MMacroPErrNot_expected_END
"Неожиданное появление $End"
"Unexpected $End"
"Neočekávané $End"
"Unerwartetes $End"

MMacroPErrUnexpected_EOS
"Неожиданный конец строки"
"Unexpected end of source string"
"Neočekávaný konec zdrojového řetězce"
"Unerwartetes Ende der Zeichenkette"

MMacroPErrExpected
"Ожидается '%s'"
"Expected '%s'"
"Očekávané '%s'"
"Erwartet '%s'"

MMacroPErrBad_Hex_Control_Char
"Неизвестный шестнадцатеричный управляющий символ"
"Bad Hex Control Char"
"Chybný kontrolní znak Hex"
"Fehlerhaftes Hexzeichen"

MMacroPErrBad_Control_Char
"Неправильный управляющий символ"
"Bad Control Char"
"Špatný kontrolní znak"
"Fehlerhaftes Kontrollzeichen"

MMacroPErrVar_Expected
"Переменная '%s' не найдена"
"Variable Expected '%s'"
"Očekávaná proměnná '%s'"
"Variable erwartet '%s'"

MMacroPErrExpr_Expected
"Ошибка синтаксиса"
"Expression Expected"
"Očekávaný výraz"
"Ausdruck erwartet"

MCannotSaveFile
l:
"Ошибка сохранения файла"
"Cannot save file"
"Nelze uložit soubor"
"Kann Datei nicht speichern"

MTextSavedToTemp
"Отредактированный текст записан в"
"Edited text is stored in"
"Editovaný text je uložen v"
"Editierter Text ist gespeichert in"

MMonthJan
l:
"Янв"
"Jan"
"Led"
"Jan"

MMonthFeb
"Фев"
"Feb"
"Úno"
"Feb"

MMonthMar
"Мар"
"Mar"
"Bře"
"Mär"

MMonthApr
"Апр"
"Apr"
"Dub"
"Apr"

MMonthMay
"Май"
"May"
"Kvě"
"Mai"

MMonthJun
"Июн"
"Jun"
"Čer"
"Jun"

MMonthJul
"Июл"
"Jul"
"Čec"
"Jul"

MMonthAug
"Авг"
"Aug"
"Srp"
"Aug"

MMonthSep
"Сен"
"Sep"
"Zář"
"Sep"

MMonthOct
"Окт"
"Oct"
"Říj"
"Okt"

MMonthNov
"Ноя"
"Nov"
"Lis"
"Nov"

MMonthDec
"Дек"
"Dec"
"Pro"
"Dez"

MPluginHotKeyTitle
l:
"Назначение горячей клавиши"
"Assign plugin hot key"
"Přidělit horkou klávesu pluginu"
"Dem Plugin eine Kurztaste zuweisen"

MPluginHotKey
"Введите горячую клавишу (букву или цифру)"
"Enter hot key (letter or digit)"
"Zadejte horkou klávesu (písmeno nebo číslici)"
"Buchstabe oder Ziffer:"

MPluginHotKeyBottom
"F4 - задать горячую клавишу"
"F4 - set hot key"
"F4 - nastavení horké klávesy"
"Kurztaste setzen: F4"

MRightCtrl
l:
"ПравыйCtrl"
"RightCtrl"
"PravýCtrl"
"StrgRechts"

MViewerGoTo
l:
"Перейти"
"Go to"
"Jdi na"
"Gehe zu"

MGoToPercent
"&Процент"
"&Percent"
"&Procent"
"&Prozent"

MGoToHex
"16-ричное &смещение"
"&Hex offset"
"&Hex offset"
"Position (&Hex)"

MGoToDecimal
"10-ичное с&мещение"
"&Decimal offset"
"&Desítkový offset"
"Position (&dezimal)"

MExceptTitleFAR
l:
"Внутренняя ошибка"
"Internal error"
"Vnitřní chyba"
"Interner Fehler"

MExceptTitleLoad
"Ошибка загрузки плагина"
"Plugin load error"
"Chyba při načítání pluginu"
"Fehler beim Laden des Plugins"

MExceptTitle
"Ошибка вызова плагина"
"Plugin call error"
"Chyba při volání pluginu"
"Fehler beim Starten des Plugins"

MExcTrappedException
"Исключительная ситуация:"
"Exception occurred:"
"Vyskytla se výjimka:"
"Ausnahmefehler aufgetreten:"

MExcCheckOnLousys
"Передана некорректная информация из модуля:"
"Incorrect information is passed from module:"
"Z modulu byla obdržena nekorektní informace:"
"Ungültige Informationen übergeben durch Modul:"

MExcStructWrongFilled
"(некорректно заполнены поля структуры <%s>)"
"(the fields of structure <%s> are wrong filled)"
"(pole struktur <%s> jsou špatně vyplněna)"
"(Felder der Struktur <%s> wurden fehlerhaft gefüllt)"

MExcStructField
"(структура <%s>, поле <%s>)"
"(structure <%s>, field <%s>)"
"(struktura <%s>, položka <%s>)"
"(Struktur <%s>, Feld <%s>)"

MExcInvalidFuncResult
"Функция <%s> вернула недопустимое значение"
"Function <%s> has returned illegal value"
"Funkce <%s> vrátila nepovolenou hodnotu"
"Funktion <%s> lieferte ungültigen Rückgabewert"

MExcAddress
"Адрес исключения - 0x%p, модуль:"
"Exception address: 0x%p in module:"
"Výjimka na adrese: 0x%X v modulu:"
"Adresse des Fehlers: 0x%p in Modul:"

MExcFARTerminateYes
"FAR Manager завершит работу!"
"FAR Manager will be terminated!"
"FAR Manager bude ukončen!"
"FAR Manager wird jetzt beendet!"

MExcUnloadYes
"Плагин будет выгружен!"
"The plugin will be Unloaded!"
"Plugin bude vyřazen!"
"Das Plugin wird jetzt entladen!"

MExcRAccess
"\"Нарушение доступа (чтение из 0x%p)\""
"\"Access violation (read from 0x%p)\""
"\"Neplatná adresa (čtení z 0x%p)\""
"\"Zugriffsverletzung (Lesen von 0x%p)\""

MExcWAccess
"\"Нарушение доступа (запись в 0x%p)\""
"\"Access violation (write to 0x%p)\""
"\"Neplatná adresa (zápis na 0x%p)\""
"\"Zugriffsverletzung (Schreiben nach 0x%p)\""

MExcEAccess
"\"Нарушение доступа (исполнение кода из 0x%p)\""
"\"Access violation (execute at 0x%p)\""
"\"Neplatná adresa (spuštění na 0x%p)\""
"\"Zugriffsverletzung (Ausführen bei 0x%p)\""

MExcOutOfBounds
"\"Попытка доступа к элементу за границами массива\""
"\"Array out of bounds\""
"\"Pole mimo hranice\""
"\"Arrayüberlauf\""

MExcDivideByZero
"\"Деление на нуль\""
"\"Divide by zero\""
"\"Dělení nulou\""
"\"Division durch Null\""

MExcStackOverflow
"\"Переполнение стека\""
"\"Stack Overflow\""
"\"Přetečení zásobníku\""
"\"Stacküberlauf\""

MExcBreakPoint
"\"Точка останова\""
"\"Breakpoint exception\""
"\"Výjimka přerušení\""
"\"Breakpoint exception\""

MExcFloatDivideByZero
"\"Деление на нуль при операции с плавающей точкой\""
"\"Floating-point divide by zero\""
"\"Dělení nulou v pohyblivé čárce\""
"\"Fließkomma-Division durch Null\""

MExcFloatOverflow
"\"Переполнение при операции с плавающей точкой\""
"\"Floating point operation overflow\""
"\"Přetečení při operaci v pohyblivé čárce\""
"\"Fließkomma-Operation verursachte Überlauf\""

MExcFloatStackOverflow
"\"Стек регистров сопроцессора полон или пуст\""
"\"Floating point stack empty or full\""
"\"Prázdný nebo plný zásobník v pohyblivé čárce\""
"\"Fließkomma-Stack leer bzw. voll\""

MExcFloatUnderflow
"\"Потеря точности при операции с плавающей точкой\""
"\"Floating point operation underflow\""
"\"Podtečení při operaci v pohyblivé čárce\""
"\"Fließkomma-Operation verursachte Underflow\""

MExcBadInstruction
"\"Недопустимая инструкция\""
"\"Illegal instruction\""
"\"Neplatná instrukce\""
"\"Ungültige Anweisung\""

MExcDatatypeMisalignment
"\"Попытка доступа к невыравненным данным\""
"\"Alignment fault\""
"\"Chyba zarovnání\""
"\"Fehler bei Datenausrichtung\""

MExcUnknown
"\"Неизвестное исключение\""
"\"Unknown exception\""
"\"Neznámá výjimka\""
"\"Unbekannte Ausnahme\""

MExcDebugger
"Отладчик"
"Debugger"
"Debugger"
"Debugger"

MNetUserName
l:
"Имя пользователя"
"User name"
"Jméno uživatele"
"Benutzername"

MNetUserPassword
"Пароль пользователя"
"User password"
"Heslo uživatele"
"Benutzerpasswort"

MReadFolderError
l:
"Не удается прочесть содержимое папки"
"Cannot read folder contents"
"Nelze načíst obsah adresáře"
"Kann Ordnerinhalt nicht lesen"

MPlgBadVers
l:
"Этот модуль требует FAR более высокой версии"
"This plugin requires higher FAR version"
"Tento plugin vyžaduje vyšší verzi FARu"
"Das Plugin benötigt eine aktuellere Version von FAR"

MPlgRequired
"Требуется версия FAR - %d.%d.%d."
"Required FAR version is %d.%d.%d."
"Požadovaná verze FARu je %d.%d.%d."
"Benötigte FAR-Version ist %d.%d.%d."

MPlgRequired2
"Текущая версия FAR - %d.%d.%d."
"Current FAR version is %d.%d.%d"
"Nynější verze FARu je %d.%d.%d"
"Aktuelle FAR-Version ist %d.%d.%d"

MPlgLoadPluginError
"Ошибка при загрузке плагина"
"Error loading plugin module"
"Chyba při nahrávání zásuvného modulu"
"Fehler beim Laden des Pluginmoduls"

MBuffSizeTooSmall_1
l:
"Буфер, размещенный под имя файла слишком мал."
"Buffer allocated for file name is too small."
"Buffer alokovaný pro jméno souboru je příliš malý."
"Reservierter Puffer für Dateiname ist zu klein."

MBuffSizeTooSmall_2
"Требуется %d байт, а имеется только %d"
"%d bytes are required, but only %d bytes were allocated."
"Požadováno %d bytů, ale alokováno pouze %d."
"%d Bytes werden benötigt aber nur %d Bytes wurden reserviert."

MCheckBox2State
l:
"?"
"?"
"?"
"?"

MEditInputSize1
"Длина поля"
"Field"
"Pole"
"Feld"

MEditInputSize2
"будет уменьшена до %d байт."
"will be truncated to %d bytes."
"bude oseknuto na %d bytů."
"wird gekürzt auf %d Bytes."

MHelpTitle
l:
"Помощь"
"Help"
"Nápověda"
"Hilfe"

MHelpActivatorURL
"Эта ссылка запускает внешнее приложение:"
"This reference starts the external application:"
"Tento odkaz spouští externí aplikaci:"
"Diese Referenz startet folgendes externes Programm:"

MHelpActivatorFormat
"с параметром:"
"with parameter:"
"s parametrem:"
"mit Parameter:"

MHelpActivatorQ
"Желаете запустить?"
"Do you wish to start it?"
"Přejete si ji spustit?"
"Wollen Sie jetzt starten?"

MCannotOpenHelp
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"

MHelpTopicNotFound
"Не найден запрошенный раздел помощи:"
"Requested help topic not found:"
"požadované téma nápovědy nebylo nalezeno"
"Angefordertes Hilfethema wurde nicht gefunden:"

MPluginsHelpTitle
l:
"Внешние модули"
"Plugins help"
"Nápověda Pluginů"
"Pluginhilfe"

MDocumentsHelpTitle
"Документы"
"Documents help"
"Nápověda Dokumentů"
"Dokumentenhilfe"

MHelpSearchTitle
l:
"Поиск"
"Search"
"Hledání"
"Suchen"

MHelpSearchingFor
"Поиск для"
"Searching for"
"Hledání"
"Suche nach"

MHelpSearchCannotFind
"Строка не найдена"
"Could not find the string"
"Nelze najít řetězec"
"Konnte Zeichenkette nicht finden"

MHelpF1
l:
l:// Help KeyBar F1-12
"Помощь"
"Help"
"Pomoc"
"Hilfe"

MHelpF2
""
""
""
""

MHelpF3
""
""
""
""

MHelpF4
""
""
""
""

MHelpF5
"Размер"
"Zoom"
"Zoom"
"Vergr."

MHelpF6
""
""
""
""

MHelpF7
"Поиск"
"Search"
"Hledat"
"Suchen"

MHelpF8
""
""
""
""

MHelpF9
""
""
""
""

MHelpF10
"Выход"
"Quit"
"Konec"
"Ende"

MHelpF11
""
""
""
""

MHelpF12
""
""
""
""

MHelpShiftF1
l:
l:// Help KeyBar Shift-F1-12
"Содерж"
"Index"
"Index"
"Index"

MHelpShiftF2
"Плагин"
"Plugin"
"Plugin"
"Plugin"

MHelpShiftF3
"Докум"
"Docums"
"Dokume"
"Dokume"

MHelpShiftF4
""
""
""
""

MHelpShiftF5
""
""
""
""

MHelpShiftF6
""
""
""
""

MHelpShiftF7
"Дальше"
"Next"
"Další"
"Nächst"

MHelpShiftF8
""
""
""
""

MHelpShiftF9
""
""
""
""

MHelpShiftF10
""
""
""
""

MHelpShiftF11
""
""
""
""

MHelpShiftF12
""
""
""
""

MHelpAltF1
l:
l:// Help KeyBar Alt-F1-12
"Пред."
"Prev"
"Předch"
"Letzt"

MHelpAltF2
""
""
""
""

MHelpAltF3
""
""
""
""

MHelpAltF4
""
""
""
""

MHelpAltF5
""
""
""
""

MHelpAltF6
""
""
""
""

MHelpAltF7
""
""
""
""

MHelpAltF8
""
""
""
""

MHelpAltF9
"Видео"
"Video"
"Video"
"Ansich"

MHelpAltF10
""
""
""
""

MHelpAltF11
""
""
""
""

MHelpAltF12
""
""
""
""

MHelpCtrlF1
l:
l:// Help KeyBar Ctrl-F1-12
""
""
""
""

MHelpCtrlF2
""
""
""
""

MHelpCtrlF3
""
""
""
""

MHelpCtrlF4
""
""
""
""

MHelpCtrlF5
""
""
""
""

MHelpCtrlF6
""
""
""
""

MHelpCtrlF7
""
""
""
""

MHelpCtrlF8
""
""
""
""

MHelpCtrlF9
""
""
""
""

MHelpCtrlF10
""
""
""
""

MHelpCtrlF11
""
""
""
""

MHelpCtrlF12
""
""
""
""

MHelpCtrlShiftF1
l:
l:// Help KeyBar CtrlShiftF1-12
""
""
""
""

MHelpCtrlShiftF2
""
""
""
""

MHelpCtrlShiftF3
""
""
""
""

MHelpCtrlShiftF4
""
""
""
""

MHelpCtrlShiftF5
""
""
""
""

MHelpCtrlShiftF6
""
""
""
""

MHelpCtrlShiftF7
""
""
""
""

MHelpCtrlShiftF8
""
""
""
""

MHelpCtrlShiftF9
""
""
""
""

MHelpCtrlShiftF10
""
""
""
""

MHelpCtrlShiftF11
""
""
""
""

MHelpCtrlShiftF12
""
""
""
""

MHelpCtrlAltF1
l:
l:// Help KeyBar CtrlAltF1-12
""
""
""
""

MHelpCtrlAltF2
""
""
""
""

MHelpCtrlAltF3
""
""
""
""

MHelpCtrlAltF4
""
""
""
""

MHelpCtrlAltF5
""
""
""
""

MHelpCtrlAltF6
""
""
""
""

MHelpCtrlAltF7
""
""
""
""

MHelpCtrlAltF8
""
""
""
""

MHelpCtrlAltF9
""
""
""
""

MHelpCtrlAltF10
""
""
""
""

MHelpCtrlAltF11
""
""
""
""

MHelpCtrlAltF12
""
""
""
""

MHelpAltShiftF1
l:
l:// Help KeyBar AltShiftF1-12
""
""
""
""

MHelpAltShiftF2
""
""
""
""

MHelpAltShiftF3
""
""
""
""

MHelpAltShiftF4
""
""
""
""

MHelpAltShiftF5
""
""
""
""

MHelpAltShiftF6
""
""
""
""

MHelpAltShiftF7
""
""
""
""

MHelpAltShiftF8
""
""
""
""

MHelpAltShiftF9
""
""
""
""

MHelpAltShiftF10
""
""
""
""

MHelpAltShiftF11
""
""
""
""

MHelpAltShiftF12
""
""
""
""

MHelpCtrlAltShiftF1
l:
l:// Help KeyBar CtrlAltShiftF1-12
""
""
""
""

MHelpCtrlAltShiftF2
""
""
""
""

MHelpCtrlAltShiftF3
""
""
""
""

MHelpCtrlAltShiftF4
""
""
""
""

MHelpCtrlAltShiftF5
""
""
""
""

MHelpCtrlAltShiftF6
""
""
""
""

MHelpCtrlAltShiftF7
""
""
""
""

MHelpCtrlAltShiftF8
""
""
""
""

MHelpCtrlAltShiftF9
""
""
""
""

MHelpCtrlAltShiftF10
""
""
""
""

MHelpCtrlAltShiftF11
""
""
""
""

MHelpCtrlAltShiftF12
""
""
""
""

MInfoF1
l:
l:// InfoPanel KeyBar F1-F12
"Помощь"
"Help"
"Pomoc"
"Hilfe"

MInfoF2
"Сверн"
"Wrap"
"Zalam"
"Umbr."

MInfoF3
"СмОпис"
"VieDiz"
"Zobraz"
"BetDiz"

MInfoF4
"РедОпи"
"EdtDiz"
"Edit"
"BeaDiz"

MInfoF5
""
""
""
""

MInfoF6
""
""
""
""

MInfoF7
"Поиск"
"Search"
"Hledat"
"Suchen"

MInfoF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MInfoF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"

MInfoF10
"Выход"
"Quit"
"Konec"
"Ende"

MInfoF11
"Модули"
"Plugin"
"Plugin"
"Plugin"

MInfoF12
"Экраны"
"Screen"
"Obraz."
"Seiten"

MInfoShiftF1
l:
l:// InfoPanel KeyBar Shift-F1-F12
""
""
""
""

MInfoShiftF2
"Слова"
"WWrap"
"ZalSlo"
"WUmbr"

MInfoShiftF3
""
""
""
""

MInfoShiftF4
""
""
""
""

MInfoShiftF5
""
""
""
""

MInfoShiftF6
""
""
""
""

MInfoShiftF7
"Дальше"
"Next"
"Další"
"Nächst"

MInfoShiftF8
"Таблиц"
"Table"
"ZnSady"
"Tabell"

MInfoShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"

MInfoShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"

MInfoShiftF11
""
""
""
""

MInfoShiftF12
""
""
""
""

MInfoAltF1
l:
l:// InfoPanel KeyBar Alt-F1-F12
"Левая"
"Left"
"Levý"
"Links"

MInfoAltF2
"Правая"
"Right"
"Pravý"
"Rechts"

MInfoAltF3
""
""
""
""

MInfoAltF4
""
""
""
""

MInfoAltF5
""
""
""
""

MInfoAltF6
""
""
""
""

MInfoAltF7
"Искать"
"Find"
"Hledat"
"Suchen"

MInfoAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"

MInfoAltF9
"Видео"
"Video"
"Video"
"Ansich"

MInfoAltF10
"Дерево"
"Tree"
"Strom"
"Baum"

MInfoAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"

MInfoAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"

MInfoCtrlF1
l:
l:// InfoPanel KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"
"Links"

MInfoCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"

MInfoCtrlF3
""
""
""
""

MInfoCtrlF4
""
""
""
""

MInfoCtrlF5
""
""
""
""

MInfoCtrlF6
""
""
""
""

MInfoCtrlF7
""
""
""
""

MInfoCtrlF8
""
""
""
""

MInfoCtrlF9
""
""
""
""

MInfoCtrlF10
""
""
""
""

MInfoCtrlF11
""
""
""
""

MInfoCtrlF12
""
""
""
""

MInfoCtrlShiftF1
l:
l:// InfoPanel KeyBar CtrlShiftF1-12
""
""
""
""

MInfoCtrlShiftF2
""
""
""
""

MInfoCtrlShiftF3
""
""
""
""

MInfoCtrlShiftF4
""
""
""
""

MInfoCtrlShiftF5
""
""
""
""

MInfoCtrlShiftF6
""
""
""
""

MInfoCtrlShiftF7
""
""
""
""

MInfoCtrlShiftF8
""
""
""
""

MInfoCtrlShiftF9
""
""
""
""

MInfoCtrlShiftF10
""
""
""
""

MInfoCtrlShiftF11
""
""
""
""

MInfoCtrlShiftF12
""
""
""
""

MInfoCtrlAltF1
l:
l:// InfoPanel KeyBar CtrlAltF1-12
""
""
""
""

MInfoCtrlAltF2
""
""
""
""

MInfoCtrlAltF3
""
""
""
""

MInfoCtrlAltF4
""
""
""
""

MInfoCtrlAltF5
""
""
""
""

MInfoCtrlAltF6
""
""
""
""

MInfoCtrlAltF7
""
""
""
""

MInfoCtrlAltF8
""
""
""
""

MInfoCtrlAltF9
""
""
""
""

MInfoCtrlAltF10
""
""
""
""

MInfoCtrlAltF11
""
""
""
""

MInfoCtrlAltF12
""
""
""
""

MInfoAltShiftF1
l:
l:// InfoPanel KeyBar AltShiftF1-12
""
""
""
""

MInfoAltShiftF2
""
""
""
""

MInfoAltShiftF3
""
""
""
""

MInfoAltShiftF4
""
""
""
""

MInfoAltShiftF5
""
""
""
""

MInfoAltShiftF6
""
""
""
""

MInfoAltShiftF7
""
""
""
""

MInfoAltShiftF8
""
""
""
""

MInfoAltShiftF9
""
""
""
""

MInfoAltShiftF10
""
""
""
""

MInfoAltShiftF11
""
""
""
""

MInfoAltShiftF12
""
""
""
""

MInfoCtrlAltShiftF1
l:
l:// InfoPanel KeyBar CtrlAltShiftF1-12
""
""
""
""

MInfoCtrlAltShiftF2
""
""
""
""

MInfoCtrlAltShiftF3
""
""
""
""

MInfoCtrlAltShiftF4
""
""
""
""

MInfoCtrlAltShiftF5
""
""
""
""

MInfoCtrlAltShiftF6
""
""
""
""

MInfoCtrlAltShiftF7
""
""
""
""

MInfoCtrlAltShiftF8
""
""
""
""

MInfoCtrlAltShiftF9
""
""
""
""

MInfoCtrlAltShiftF10
""
""
""
""

MInfoCtrlAltShiftF11
""
""
""
""

MInfoCtrlAltShiftF12
""
""
""
""

MQViewF1
l:
l:// QView KeyBar F1-F12
"Помощь"
"Help"
"Pomoc"
"Hilfe"

MQViewF2
"Сверн"
"Wrap"
"Zalam"
"Umbr."

MQViewF3
"Просм"
"View"
"Zobraz"
"Betr."

MQViewF4
"Код"
"Hex"
"Hex"
"Hex"

MQViewF5
""
""
""
""

MQViewF6
""
""
""
""

MQViewF7
"Поиск"
"Search"
"Hledat"
"Suchen"

MQViewF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MQViewF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"

MQViewF10
"Выход"
"Quit"
"Konec"
"Ende"

MQViewF11
"Модули"
"Plugin"
"Plugin"
"Plugin"

MQViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"

MQViewShiftF1
l:
l:// QView KeyBar Shift-F1-F12
""
""
""
""

MQViewShiftF2
"Слова"
"WWrap"
"ZalSlo"
"WUmbr"

MQViewShiftF3
""
""
""
""

MQViewShiftF4
""
""
""
""

MQViewShiftF5
""
""
""
""

MQViewShiftF6
""
""
""
""

MQViewShiftF7
"Дальше"
"Next"
"Další"
"Nächst"

MQViewShiftF8
"Таблиц"
"Table"
"ZnSady"
"Tabell"

MQViewShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"

MQViewShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"

MQViewShiftF11
""
""
""
""

MQViewShiftF12
""
""
""
""

MQViewAltF1
l:
l:// QView KeyBar Alt-F1-F12
"Левая"
"Left"
"Levý"
"Links"

MQViewAltF2
"Правая"
"Right"
"Pravý"
"Rechts"

MQViewAltF3
""
""
""
""

MQViewAltF4
""
""
""
""

MQViewAltF5
""
""
""
""

MQViewAltF6
""
""
""
""

MQViewAltF7
"Искать"
"Find"
"Hledat"
"Suchen"

MQViewAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"

MQViewAltF9
"Видео"
"Video"
"Video"
"Ansich"

MQViewAltF10
"Дерево"
"Tree"
"Strom"
"Baum"

MQViewAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"

MQViewAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"

MQViewCtrlF1
l:
l:// QView KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"
"Links"

MQViewCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"

MQViewCtrlF3
""
""
""
""

MQViewCtrlF4
""
""
""
""

MQViewCtrlF5
""
""
""
""

MQViewCtrlF6
""
""
""
""

MQViewCtrlF7
""
""
""
""

MQViewCtrlF8
""
""
""
""

MQViewCtrlF9
""
""
""
""

MQViewCtrlF10
""
""
""
""

MQViewCtrlF11
""
""
""
""

MQViewCtrlF12
""
""
""
""

MQViewCtrlShiftF1
l:
l:// QView KeyBar CtrlShiftF1-12
""
""
""
""

MQViewCtrlShiftF2
""
""
""
""

MQViewCtrlShiftF3
""
""
""
""

MQViewCtrlShiftF4
""
""
""
""

MQViewCtrlShiftF5
""
""
""
""

MQViewCtrlShiftF6
""
""
""
""

MQViewCtrlShiftF7
""
""
""
""

MQViewCtrlShiftF8
""
""
""
""

MQViewCtrlShiftF9
""
""
""
""

MQViewCtrlShiftF10
""
""
""
""

MQViewCtrlShiftF11
""
""
""
""

MQViewCtrlShiftF12
""
""
""
""

MQViewCtrlAltF1
l:
l:// QView KeyBar CtrlAltF1-12
""
""
""
""

MQViewCtrlAltF2
""
""
""
""

MQViewCtrlAltF3
""
""
""
""

MQViewCtrlAltF4
""
""
""
""

MQViewCtrlAltF5
""
""
""
""

MQViewCtrlAltF6
""
""
""
""

MQViewCtrlAltF7
""
""
""
""

MQViewCtrlAltF8
""
""
""
""

MQViewCtrlAltF9
""
""
""
""

MQViewCtrlAltF10
""
""
""
""

MQViewCtrlAltF11
""
""
""
""

MQViewCtrlAltF12
""
""
""
""

MQViewAltShiftF1
l:
l:// QView KeyBar AltShiftF1-12
""
""
""
""

MQViewAltShiftF2
""
""
""
""

MQViewAltShiftF3
""
""
""
""

MQViewAltShiftF4
""
""
""
""

MQViewAltShiftF5
""
""
""
""

MQViewAltShiftF6
""
""
""
""

MQViewAltShiftF7
""
""
""
""

MQViewAltShiftF8
""
""
""
""

MQViewAltShiftF9
""
""
""
""

MQViewAltShiftF10
""
""
""
""

MQViewAltShiftF11
""
""
""
""

MQViewAltShiftF12
""
""
""
""

MQViewCtrlAltShiftF1
l:
l:// QView KeyBar CtrlAltShiftF1-12
""
""
""
""

MQViewCtrlAltShiftF2
""
""
""
""

MQViewCtrlAltShiftF3
""
""
""
""

MQViewCtrlAltShiftF4
""
""
""
""

MQViewCtrlAltShiftF5
""
""
""
""

MQViewCtrlAltShiftF6
""
""
""
""

MQViewCtrlAltShiftF7
""
""
""
""

MQViewCtrlAltShiftF8
""
""
""
""

MQViewCtrlAltShiftF9
""
""
""
""

MQViewCtrlAltShiftF10
""
""
""
""

MQViewCtrlAltShiftF11
""
""
""
""

MQViewCtrlAltShiftF12
""
""
""
""

MKBTreeF1
l:
l:// Tree KeyBar F1-F12
"Помощь"
"Help"
"Pomoc"
"Hilfe"

MKBTreeF2
"ПользМ"
"UserMn"
"UživMn"
"BenuMn"

MKBTreeF3
""
""
""
""

MKBTreeF4
"Атриб"
"Attr"
"Attr"
"Attr"

MKBTreeF5
"Копир"
"Copy"
"Kopír."
"Kopier"

MKBTreeF6
"Перен"
"RenMov"
"PřjPřs"
"RenMov"

MKBTreeF7
"Папка"
"MkFold"
"VytAdr"
"VerzEr"

MKBTreeF8
"Удален"
"Delete"
"Smazat"
"Lösch"

MKBTreeF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"

MKBTreeF10
"Выход"
"Quit"
"Konec"
"Ende"

MKBTreeF11
"Модули"
"Plugin"
"Plugin"
"Plugin"

MKBTreeF12
"Экраны"
"Screen"
"Obraz."
"Seiten"

MKBTreeShiftF1
l:
l:// Tree KeyBar Shift-F1-F12
""
""
""
""

MKBTreeShiftF2
""
""
""
""

MKBTreeShiftF3
""
""
""
""

MKBTreeShiftF4
""
""
""
""

MKBTreeShiftF5
"Копир"
"Copy"
"Kopír."
"Kopier"

MKBTreeShiftF6
"Перен"
"Rename"
"Přejm."
"Umben"

MKBTreeShiftF7
""
""
""
""

MKBTreeShiftF8
""
""
""
""

MKBTreeShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"

MKBTreeShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"

MKBTreeShiftF11
"Группы"
"Group"
"Skupin"
"Gruppe"

MKBTreeShiftF12
"Выбран"
"SelUp"
"VybPrv"
"AuswOb"

MKBTreeAltF1
l:
l:// Tree KeyBar Alt-F1-F12
"Левая"
"Left"
"Levý"
"Links"

MKBTreeAltF2
"Правая"
"Right"
"Pravý"
"Rechts"

MKBTreeAltF3
""
""
""
""

MKBTreeAltF4
""
""
""
""

MKBTreeAltF5
""
""
""
""

MKBTreeAltF6
""
""
""
""

MKBTreeAltF7
"Искать"
"Find"
"Hledat"
"Suchen"

MKBTreeAltF8
"Истор"
"Histry"
"Histor"
"Histor"

MKBTreeAltF9
"Видео"
"Video"
"Video"
"Ansich"

MKBTreeAltF10
"Дерево"
"Tree"
"Strom"
"Baum"

MKBTreeAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"

MKBTreeAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"

MKBTreeCtrlF1
l:
l:// Tree KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"
"Links"

MKBTreeCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"

MKBTreeCtrlF3
""
""
""
""

MKBTreeCtrlF4
""
""
""
""

MKBTreeCtrlF5
""
""
""
""

MKBTreeCtrlF6
""
""
""
""

MKBTreeCtrlF7
""
""
""
""

MKBTreeCtrlF8
""
""
""
""

MKBTreeCtrlF9
""
""
""
""

MKBTreeCtrlF10
""
""
""
""

MKBTreeCtrlF11
""
""
""
""

MKBTreeCtrlF12
""
""
""
""

MKBTreeCtrlShiftF1
l:
l:// Tree KeyBar CtrlShiftF1-12
""
""
""
""

MKBTreeCtrlShiftF2
""
""
""
""

MKBTreeCtrlShiftF3
""
""
""
""

MKBTreeCtrlShiftF4
""
""
""
""

MKBTreeCtrlShiftF5
""
""
""
""

MKBTreeCtrlShiftF6
""
""
""
""

MKBTreeCtrlShiftF7
""
""
""
""

MKBTreeCtrlShiftF8
""
""
""
""

MKBTreeCtrlShiftF9
""
""
""
""

MKBTreeCtrlShiftF10
""
""
""
""

MKBTreeCtrlShiftF11
""
""
""
""

MKBTreeCtrlShiftF12
""
""
""
""

MKBTreeCtrlAltF1
l:
l:// Tree KeyBar CtrlAltF1-12
""
""
""
""

MKBTreeCtrlAltF2
""
""
""
""

MKBTreeCtrlAltF3
""
""
""
""

MKBTreeCtrlAltF4
""
""
""
""

MKBTreeCtrlAltF5
""
""
""
""

MKBTreeCtrlAltF6
""
""
""
""

MKBTreeCtrlAltF7
""
""
""
""

MKBTreeCtrlAltF8
""
""
""
""

MKBTreeCtrlAltF9
""
""
""
""

MKBTreeCtrlAltF10
""
""
""
""

MKBTreeCtrlAltF11
""
""
""
""

MKBTreeCtrlAltF12
""
""
""
""

MKBTreeAltShiftF1
l:
l:// Tree KeyBar AltShiftF1-12
""
""
""
""

MKBTreeAltShiftF2
""
""
""
""

MKBTreeAltShiftF3
""
""
""
""

MKBTreeAltShiftF4
""
""
""
""

MKBTreeAltShiftF5
""
""
""
""

MKBTreeAltShiftF6
""
""
""
""

MKBTreeAltShiftF7
""
""
""
""

MKBTreeAltShiftF8
""
""
""
""

MKBTreeAltShiftF9
""
""
""
""

MKBTreeAltShiftF10
""
""
""
""

MKBTreeAltShiftF11
""
""
""
""

MKBTreeAltShiftF12
""
""
""
""

MKBTreeCtrlAltShiftF1
l:
l:// Tree KeyBar CtrlAltShiftF1-12
""
""
""
""

MKBTreeCtrlAltShiftF2
""
""
""
""

MKBTreeCtrlAltShiftF3
""
""
""
""

MKBTreeCtrlAltShiftF4
""
""
""
""

MKBTreeCtrlAltShiftF5
""
""
""
""

MKBTreeCtrlAltShiftF6
""
""
""
""

MKBTreeCtrlAltShiftF7
""
""
""
""

MKBTreeCtrlAltShiftF8
""
""
""
""

MKBTreeCtrlAltShiftF9
""
""
""
""

MKBTreeCtrlAltShiftF10
""
""
""
""

MKBTreeCtrlAltShiftF11
""
""
""
""

MKBTreeCtrlAltShiftF12
""
""
""
""

MCopyTimeInfo
l:
"Время: %8.8s  Осталось: %8.8s  %5d%1.1sб/с"
"Time: %8.8s  Remaining: %8.8s  %5d%1.1sb/s"
"Čas: %8.8s  Zbývá: %8.8s  %5d%1.1sb/s"
"Zeit: %8.8s  Verbleibend: %8.8s  %5d%1.1sb/s"

MKeyESCWasPressed
l:
"Действие было прервано"
"Operation has been interrupted"
"Operace byla přerušena"
"Vorgang wurde unterbrochen"

MDoYouWantToStopWork
"Вы действительно хотите отменить действие?"
"Do you really want to cancel it?"
"Opravdu chcete operaci stornovat?"
"Wollen Sie den Vorgang wirklich abbrechen?"

MDoYouWantToStopWork2
"Продолжить выполнение?"
"Continue work? "
"Pokračovat v práci?"
"Vorgang fortsetzen? "

MCheckingFileInPlugin
l:
"Файл проверяется в плагине"
"The file is being checked by the plugin"
"Soubor je právě kontrolován pluginem"
"Datei wird von Plugin überprüft"

MDialogType
l:
"Диалог"
"Dialog"
"Dialog"
"Dialog"

MHelpType
"Помощь"
"Help"
"Nápověda"
"Hilfe"

MFolderTreeType
"ПоискКаталогов"
"FolderTree"
"StromAdresáře"
"Ordnerbaum"

MVMenuType
"Меню"
"Menu"
"Menu"
"Menü"

MIncorrectMask
l:
"Некорректная маска файлов!"
"File-mask string contains errors!"
"Řetězec masky souboru obsahuje chyby!"
"Zeichenkette mit Dateimaske enthält Fehler!"

MPanelBracketsForLongName
l:
"{}"
"{}"
"{}"
"{}"

MComspecNotFound
l:
"Переменная окружения %COMSPEC% не определена!"
"Environment variable %COMSPEC% not defined!"
"Proměnná prostředí %COMSPEC% není definována!"
"Umgebungsvariable %COMSPEC% nicht definiert!"

MExecuteErrorMessage
"'%s' не является внутренней или внешней командой, исполняемой программой или пакетным файлом.\n"
"'%s' is not recognized as an internal or external command, operable program or batch file.\n"
"'%s' nebylo nalezeno jako vniřní nebo externí příkaz, spustitelná aplikace nebo dávkový soubor.\n"
"'%s' nicht erkannt als interner oder externer Befehl, Programm oder Stapeldatei.\n"

MOpenPluginCannotOpenFile
l:
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"

MFileFilterTitle
l:
"Фильтр"
"Filter"
"Filtr"
"Filter"

MFileHilightTitle
"Раскраска файлов"
"Files highlighting"
"Zvýrazňování souborů"
"Farbmarkierungen"

MFileFilterName
"Имя &фильтра:"
"Filter &name:"
"Jmé&no filtru:"
"Filter&name:"

MFileFilterMatchMask
"&Маска:"
"&Mask:"
"&Maska"
"&Maske:"

MFileFilterSize
"Разм&ер:"
"Si&ze:"
"Vel&ikost"
"G&röße:"

MFileFilterSizeFromSign
">="
">="
"<="
">="

MFileFilterSizeToSign
"<="
"<="
"<="
"<="

MFileFilterDate
"&Дата/Время:"
"Da&te/Time:"
"Dat&um/Čas:"
"Da&tum/Zeit:"

MFileFilterModified
"&модификации"
"&modification"
"&modifikace"
"&Modifikation"

MFileFilterCreated
"&создания"
"&creation"
"&vytvoření"
"E&rstellung"

MFileFilterOpened
"&доступа"
"&access"
"&přístupu"
"Z&ugriff"

MFileFilterDateRelative
"Относительна&я"
"Relat&ive"
"Relati&vní"
"Relat&iv"

MFileFilterDateAfterSign
">="
">="
">="
">="

MFileFilterDateBeforeSign
"<="
"<="
"<="
"<="

MFileFilterCurrent
"Теку&щая"
"C&urrent"
"Aktuá&lní"
"Akt&uell"

MFileFilterBlank
"С&брос"
"B&lank"
"Práz&dný"
"&Leer"

MFileFilterAttr
"Атрибут&ы"
"Attri&butes"
"Attri&buty"
"Attri&bute"

MFileFilterAttrR
"&Только для чтения"
"&Read only"
"Jen pro čt&ení"
"Sch&reibschutz"

MFileFilterAttrA
"&Архивный"
"&Archive"
"Arc&hivovat"
"&Archiv"

MFileFilterAttrH
"&Скрытый"
"&Hidden"
"Skry&tý"
"&Versteckt"

MFileFilterAttrS
"С&истемный"
"&System"
"Systémo&vý"
"&System"

MFileFilterAttrC
"С&жатый"
"&Compressed"
"Kompri&movaný"
"&Komprimiert"

MFileFilterAttrE
"&Зашифрованный"
"&Encrypted"
"Ši&frovaný"
"V&erschlüsselt"

MFileFilterAttrD
"&Каталог"
"&Directory"
"Adr&esář"
"Ver&zeichnis"

MFileFilterAttrNI
"&Неиндексируемый"
"Not inde&xed"
"Neinde&xovaný"
"Nicht in&diziert"

MFileFilterAttrSparse
"&Разреженный"
"S&parse"
"Říd&ký"
"Reserve"

MFileFilterAttrT
"&Временный"
"Temporar&y"
"Doča&sný"
"Temporär"

MFileFilterAttrReparse
"Симво&л. ссылка"
"Symbolic lin&k"
"Sybolický li&nk"
"Symbolischer Lin&k"

MFileFilterAttrOffline
"Автономны&й"
"O&ffline"
"O&ffline"
"O&ffline"

MFileFilterAttrVirtual
"Вирт&уальный"
"&Virtual"
"Virtuální"
"&Virtuell"

MFileFilterReset
"Очистит&ь"
"Reset"
"Reset"
"Rücksetzen"

MFileFilterCancel
"Отмена"
"Cancel"
"Storno"
"Abbruch"

MFileFilterMakeTransparent
"Выставить прозрачность"
"Make transparent"
"Zprůhlednit"
"Transparent"

MBadFileSizeFormat
"Неправильно заполнено поле размера!"
"File size field is incorrectly filled!"
"Velikost souboru neobsahuje správnou hodnotu!"
"Angabe der Dateigröße ist fehlerhaft!"

#Must be the last
MNewFileName
l:
"?Новый файл?"
"?New File?"
"?Nový soubor?"
"?Neue Datei?"
