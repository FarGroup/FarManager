m4_include(`farversion.m4')m4_dnl
#hpp file name
lang.hpp
#number of languages
5
#id:0 language file name, language name, language description
FarRus.lng Russian "Russian (Русский)"
#id:1 language file name, language name, language description
FarEng.lng English "English"
#id:2 language file name, language name, language description
FarCze.lng Czech "Czech (Čeština)"
#id:3 language file name, language name, language description
FarGer.lng German "German (Deutsch)"
#id:4 language file name, language name, language description
FarHun.lng Hungarian "Hungarian (Magyar)"

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
#
#lng lines marked with "upd:" will cause a warning to be printed to the
#screen reminding that this line should be updated/translated

MYes=0
`l://Version: 'MAJOR`.'MINOR` build 'BUILD
l:
"Да"
"Yes"
"Ano"
"Ja"
"Igen"

MNo
"Нет"
"No"
"Ne"
"Nein"
"Nem"

MOk
"Продолжить"
"OK"
"Ok"
"OK"
"OK"

MHYes
l:
"&Да"
"&Yes"
"&Ano"
"&Ja"
"I&gen"

MHNo
"&Нет"
"&No"
"&Ne"
"&Nein"
"Ne&m"

MHOk
"&Продолжить"
"&OK"
"&Ok"
"&OK"
"&OK"

MCancel
l:
"Отменить"
"Cancel"
"Storno"
"Abbrechen"
"Mégsem"

MRetry
"Повторить"
"Retry"
"Znovu"
"Wiederholen"
"Újra"

MSkip
"Пропустить"
"Skip"
"Přeskočit"
"Überspringen"
"Kihagy"

MAbort
"Прервать"
"Abort"
"Zrušit"
"Abbrechen"
"Megszakít"

MIgnore
"Игнорировать"
"Ignore"
"Ignorovat"
"Ignorieren"
"Mégis"

MDelete
"Удалить"
"Delete"
"Smazat"
"Löschen"
"Töröl"

MSplit
"Разделить"
"Split"
"Rozdělit"
"Zerteilen"
"Feloszt"

MRemove
"Удалить"
"Remove"
"Odstranit"
"Entfernen"
"Eltávolít"

MHCancel
l:
"&Отменить"
"&Cancel"
"&Storno"
"&Abbrechen"
"Még&sem"

MHRetry
"&Повторить"
"&Retry"
"&Znovu"
"&Wiederholen"
"Ú&jra"

MHSkip
"П&ропустить"
"&Skip"
"&Přeskočit"
"Über&springen"
"Ki&hagy"

MHSkipAll
"Пропустить &все"
"S&kip all"
"Přeskočit &vše"
"Alle übersprin&gen"
"Kihagy &mind"

MHAbort
"Прер&вать"
"&Abort"
"Zr&ušit"
"&Abbrechen"
"Megsza&kít"

MHIgnore
"&Игнорировать"
"&Ignore"
"&Ignorovat"
"&Ignorieren"
"Mé&gis"

MHDelete
"&Удалить"
"&Delete"
"S&mazat"
"&Löschen"
"&Töröl"

MHRemove
"&Удалить"
"R&emove"
"&Odstranit"
"Ent&fernen"
"Eltá&volít"

MHSplit
"Раз&делить"
"Sp&lit"
"&Rozdělit"
"&Zerteilen"
"Fel&oszt"

MWarning
l:
"Предупреждение"
"Warning"
"Varování"
"Warnung"
"Figyelem"

MError
"Ошибка"
"Error"
"Chyba"
"Fehler"
"Hiba"

MQuit
l:
"Выход"
"Quit"
"Konec"
"Beenden"
"Kilépés"

MAskQuit
"Вы хотите завершить работу в FAR?"
"Do you want to quit FAR?"
"Opravdu chcete ukončit FAR?"
"Wollen Sie FAR beenden?"
"Biztosan kilép a FAR-ból?"

MF1
l:
l://functional keys - 6 characters max
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Súgó"

MF2
"ПользМ"
"UserMn"
"UživMn"
"BenuMn"
"FhMenü"

MF3
"Просм"
"View"
"Zobraz"
"Betr."
"Megnéz"

MF4
"Редакт"
"Edit"
"Edit"
"Bearb"
"Szerk."

MF5
"Копир"
"Copy"
"Kopír."
"Kopier"
"Másol"

MF6
"Перен"
"RenMov"
"PřjPřs"
"Versch"
"AtnMoz"

MF7
"Папка"
"MkFold"
"VytAdr"
"VerzEr"
"ÚjMapp"

MF8
"Удален"
"Delete"
"Smazat"
"Lösch."
"Töröl"

MF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"
"KonfMn"

MF10
"Выход"
"Quit"
"Konec"
"Beend."
"Kilép"

MF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"

MAltF1
l:
"Левая"
"Left"
"Levý"
"Links"
"Bal"

MAltF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"

MAltF3
"Смотр."
"View.."
"Zobr.."
"Betr.."
"Néző.."

MAltF4
"Редак."
"Edit.."
"Edit.."
"Bear.."
"Szrk.."

MAltF5
"Печать"
"Print"
"Tisk"
"Druck"
"Nyomt"

MAltF6
"Ссылка"
"MkLink"
"VytLnk"
"LinkEr"
"ÚjLink"

MAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"

MAltF8
"Истор"
"Histry"
"Histor"
"Histor"
"ParElő"

MAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"

MAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"

MAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"

MAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"BearHs"
"MapElő"

MCtrlF1
l:
"Левая"
"Left"
"Levý"
"Links"
"Bal"

MCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"

MCtrlF3
"Имя   "
"Name  "
"Název "
"Name  "
"Név"

MCtrlF4
"Расшир"
"Extens"
"Přípon"
"Erweit"
"Kiterj"

MCtrlF5
"Модиф"
"Modifn"
"Modifk"
"Veränd"
"MódIdő"

MCtrlF6
"Размер"
"Size"
"Veliko"
"Größe"
"Méret"

MCtrlF7
"Несорт"
"Unsort"
"Neřadi"
"Unsort"
"NincsR"

MCtrlF8
"Создан"
"Creatn"
"Vytvoř"
"Erstel"
"Keletk"

MCtrlF9
"Доступ"
"Access"
"Přístu"
"Zugrif"
"Hozzáf"

MCtrlF10
"Описан"
"Descr"
"Popis"
"Beschr"
"Megjgy"

MCtrlF11
"Владел"
"Owner"
"Vlastn"
"Besitz"
"Tulajd"

MCtrlF12
"Сорт"
"Sort"
"Třídit"
"Sort."
"RendMd"

MShiftF1
l:
"Добавл"
"Add"
"Přidat"
"Hinzu"
"Tömört"

MShiftF2
"Распак"
"Extrct"
"Rozbal"
"Extrah"
"Kibont"

MShiftF3
"АрхКом"
"ArcCmd"
"ArcPří"
"ArcBef"
"TömPar"

MShiftF4
"Редак."
"Edit.."
"Edit.."
"Erst.."
"ÚjFájl"

MShiftF5
"Копир"
"Copy"
"Kopír."
"Kopier"
"Másol"

MShiftF6
"Переим"
"Rename"
"Přejme"
"Umbene"
"ÁtnMoz"

MShiftF7
""
""
""
""
""

MShiftF8
"Удален"
"Delete"
"Smazat"
"Lösch."
"Töröl"

MShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"

MShiftF10
"Послдн"
"Last"
"Posled"
"Letzte"
"UtsMnü"

MShiftF11
"Группы"
"Group"
"Skupin"
"Gruppe"
"Csoprt"

MShiftF12
"Выбран"
"SelUp"
"VybPrv"
"AuswOb"
"KijFel"

MAltShiftF1
l:
l:// Main AltShift
""
""
""
""
""

MAltShiftF2
""
""
""
""
""

MAltShiftF3
""
""
""
""
""

MAltShiftF4
""
""
""
""
""

MAltShiftF5
""
""
""
""
""

MAltShiftF6
""
""
""
""
""

MAltShiftF7
""
""
""
""
""

MAltShiftF8
""
""
""
""
""

MAltShiftF9
"КонфПл"
"ConfPl"
"KonfPl"
"KonfPn"
"PluKnf"

MAltShiftF10
""
""
""
""
""

MAltShiftF11
""
""
""
""
""

MAltShiftF12
""
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
""

MCtrlShiftF2
""
""
""
""
""

MCtrlShiftF3
"Просм"
"View"
"Zobraz"
"Betr"
"Megnéz"

MCtrlShiftF4
"Редакт"
"Edit"
"Edit"
"Bearb"
"Szerk."

MCtrlShiftF5
""
""
""
""
""

MCtrlShiftF6
""
""
""
""
""

MCtrlShiftF7
""
""
""
""
""

MCtrlShiftF8
""
""
""
""
""

MCtrlShiftF9
""
""
""
""
""

MCtrlShiftF10
""
""
""
""
""

MCtrlShiftF11
""
""
""
""
""

MCtrlShiftF12
""
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
""

MCtrlAltF2
""
""
""
""
""

MCtrlAltF3
""
""
""
""
""

MCtrlAltF4
""
""
""
""
""

MCtrlAltF5
""
""
""
""
""

MCtrlAltF6
""
""
""
""
""

MCtrlAltF7
""
""
""
""
""

MCtrlAltF8
""
""
""
""
""

MCtrlAltF9
""
""
""
""
""

MCtrlAltF10
""
""
""
""
""

MCtrlAltF11
""
""
""
""
""

MCtrlAltF12
""
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
""

MCtrlAltShiftF2
""
""
""
""
""

MCtrlAltShiftF3
""
""
""
""
""

MCtrlAltShiftF4
""
""
""
""
""

MCtrlAltShiftF5
""
""
""
""
""

MCtrlAltShiftF6
""
""
""
""
""

MCtrlAltShiftF7
""
""
""
""
""

MCtrlAltShiftF8
""
""
""
""
""

MCtrlAltShiftF9
""
""
""
""
""

MCtrlAltShiftF10
""
""
""
""
""

MCtrlAltShiftF11
""
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
""

MHistoryTitle
l:
"История команд"
"History"
"Historie"
"Historie der letzten Befehle"
"Parancs előzmények"

MFolderHistoryTitle
"История папок"
"Folders history"
"Historie adresářů"
"Zuletzt besuchte Ordner"
"Mappa előzmények"

MViewHistoryTitle
"История просмотра"
"File view history"
"Historie prohlížení souborů"
"Zuletzt betrachtete Dateien"
"Fájl előzmények"

MViewHistoryIsCreate
"Создать файл?"
"Create file?"
"Vytvořit soubor?"
"Datei erstellen?"
"Fájl létrehozása?"

MHistoryView
"Просмотр"
"View"
"Zobrazit"
"Betr"
"Nézett"

MHistoryEdit
"Редактор"
"Edit"
"Editovat"
"Bearb"
"Szerk."

MHistoryExt
"Внешний "
"Ext."
"Rozšíření"
"Ext."
"Kit."

MHistoryClear
l:
"История будет полностью очищена. Продолжить?"
"All records in the history will be deleted. Continue?"
"Všechny záznamy v historii budou smazány. Pokračovat?"
"Die gesamte Historie wird gelöscht. Fortfahren?"
"Az előzmények minden eleme törlődik. Folytatja?"

MClear
"&Очистить"
"&Clear history"
"&Vymazat historii"
"Historie &löschen"
"Elő&zmények törlése"

MConfigSystemTitle
l:
"Системные параметры"
"System settings"
"Nastavení systému"
"Grundeinstellungen"
"Rendszer beállítások"

MConfigRO
"&Снимать атрибут R/O c CD файлов"
"&Clear R/O attribute from CD files"
"Z&rušit atribut R/O u souborů na CD"
"Schreibschutz von CD-Dateien ent&fernen"
"&Csak olvasható attr. törlése CD fájlokról"

MConfigRecycleBin
"Удалять в &Корзину"
"&Delete to Recycle Bin"
"&Mazat do Koše"
"In Papierkorb &löschen"
"&Törlés a Lomtárba"

MConfigRecycleBinLink
"У&далять символические ссылки"
"Delete symbolic &links"
"Mazat symbolické &linky"
"Symbolische L&inks löschen"
"Szimbolikus l&inkek törlése"

MConfigSystemCopy
"Использовать систе&мную функцию копирования"
"Use sys&tem copy routine"
"Používat kopírovací rutiny sys&tému"
"Sys&temeigene Kopierroutine verwenden"
"&Másoláshoz a rendszerrutin használata"

MConfigCopySharing
"Копировать открытые для &записи файлы"
"Copy files opened for &writing"
"Kopírovat soubory otevřené pro &zápis"
"Zum Schreiben geöffnete Dateien &kopieren"
"Írásra megnyitott &fájlok másolhatók"

MConfigScanJunction
"Ск&анировать символические ссылки"
"Scan s&ymbolic links"
"Prohledávat s&ymbolické linky"
"S&ymbolische Links scannen"
"Szimbolikus linkek &vizsgálata"

MConfigCreateUppercaseFolders
"Создавать &папки заглавными буквами"
"Create folders in &uppercase"
"Vytvářet adresáře &velkými písmeny"
"Ordner in Großschreib&ung erstellen"
"Mappák létrehozása &NAGYBETŰKKEL"

MConfigInactivity
"&Время бездействия"
"&Inactivity time"
"&Doba nečinnosti"
"Inaktivitäts&zeit"
"A FAR kilé&p"

MConfigInactivityMinutes
"минут"
"minutes"
"minut"
"Minuten"
"perc tétlenség után"

MConfigSaveHistory
"Сохранять &историю команд"
"Save commands &history"
"Ukládat historii &příkazů"
"&Befehlshistorie speichern"
"Parancs elő&zmények mentése"

MConfigSaveFoldersHistory
"Сохранять историю п&апок"
"Save &folders history"
"Ukládat historii &adresářů"
"&Ordnerhistorie speichern"
"M&appa előzmények mentése"

MConfigSaveViewHistory
"Сохранять историю п&росмотра и редактора"
"Save &view and edit history"
"Ukládat historii Zobraz a Editu&j"
"Betrachter/&Editor-Historie speichern"
"Nézőke és &Szerkesztő előzmények mentése"

MConfigRegisteredTypes
"Использовать стандартные &типы файлов"
"Use Windows &registered types"
"Používat regi&strované typy Windows"
"&Registrierte Windows-Dateitypen verwenden"
"&Windows reg. fájltípusok használata"

MConfigCloseCDGate
"Автоматически монтироват&ь CDROM"
"CD drive auto &mount"
"Automatické př&ipojení CD disků"
"CD-Laufwerk auto&matisch schließen"
"CD tálca a&utomatikus behúzása"

MConfigPersonalPath
"Путь к персональным п&лагинам:"
"&Path for personal plugins:"
"&Cesta k vlastním pluginům:"
"&Pfad für eigene Plugins:"
"Saját plu&ginek útvonala:"

MConfigAutoSave
"Автозапись кон&фигурации"
"Auto &save setup"
"Automatické ukládaní &nastavení"
"Setup automatisch &"speichern"
"B&eállítások automatikus mentése"

MConfigPanelTitle
l:
"Настройки панели"
"Panel settings"
"Nastavení panelů"
"Panels einrichten"
"Panel beállítások"

MConfigHidden
"Показывать скр&ытые и системные файлы"
"Show &hidden and system files"
"Ukázat &skryté a systémové soubory"
"&Versteckte und Systemdateien anzeigen"
"&Rejtett és rendszerfájlok mutatva"

MConfigHighlight
"&Раскраска файлов"
"Hi&ghlight files"
"Zvý&razňovat soubory"
"Dateien mark&ieren"
"Fá&jlok kiemelése"

MConfigAutoChange
"&Автосмена папки"
"&Auto change folder"
"&Automaticky měnit adresář"
"Ordner &automatisch wechseln (Baumansicht)"
"&Automatikus mappaváltás"

MConfigSelectFolders
"Пометка &папок"
"Select &folders"
"Vybírat a&dresáře"
"&Ordner auswählen"
"A ma&ppák is kijelölhetők"

MConfigSortFolderExt
"Сортировать имена папок по рас&ширению"
"Sort folder names by e&xtension"
"Řadit adresáře podle přípony"
"Ordner nach Er&weiterung sortieren"
"Mappák is rendezhetők &kiterjesztés szerint"

MConfigReverseSort
"Разрешить &обратную сортировку"
"Allow re&verse sort modes"
"Do&volit změnu směru řazení"
"&Umgekehrte Sortiermodi zulassen"
"Fordí&tott rendezés engedélyezése"

MConfigAutoUpdateLimit
"Отключать автооб&новление панелей,"
"&Disable automatic update of panels"
"Vypnout a&utomatickou aktualizaci panelů"
"Automatisches Panelupdate &deaktivieren"
"Pan&el automatikus frissítése kikapcsolva,"

MConfigAutoUpdateLimit2
"если объектов больше"
"if object count exceeds"
"jestliže počet objektů překročí"
"wenn mehr Objekte als"
"ha több elem van, mint:"

MConfigAutoUpdateRemoteDrive
"Автообновление с&етевых дисков"
"Network drives autor&efresh"
"Automatická obnova síťových disků"
"Netzw&erklauferke autom. aktualisieren"
"Hálózati meghajtók autom. &frissítése"

MConfigShowColumns
"Показывать &заголовки колонок"
"Show &column titles"
"Zobrazovat &nadpisy sloupců"
"S&paltentitel anzeigen"
"Oszlop&nevek mutatva"

MConfigShowStatus
"Показывать &строку статуса"
"Show &status line"
"Zobrazovat sta&vový řádek"
"&Statuszeile anzeigen"
"Á&llapotsor mutatva"

MConfigShowTotal
"Показывать су&ммарную информацию"
"Show files &total information"
"Zobrazovat &informace o velikosti souborů"
"&Gesamtzahl für Dateien anzeigen"
"Fájl össze&s információja mutatva"

MConfigShowFree
"Показывать с&вободное место"
"Show f&ree size"
"Zobrazovat vo&lné místo"
"&Freien Speicher anzeigen"
"Sza&bad lemezterület mutatva"

MConfigShowScrollbar
"Показывать по&лосу прокрутки"
"Show scroll&bar"
"Zobrazovat &posuvník"
"Scroll&balken anzeigen"
"Gördítősá&v mutatva"

MConfigShowScreensNumber
"Показывать количество &фоновых экранов"
"Show background screens &number"
"Zobrazovat počet &obrazovek na pozadí"
"&Nummer von Hintergrundseiten anzeigen"
"&Háttérképernyők száma mutatva"

MConfigShowSortMode
"Показывать букву режима сор&тировки"
"Show sort &mode letter"
"Zobrazovat písmeno &módu řazení"
"Buchstaben der Sortier&modi anzeigen"
"Rendezési mó&d betűjele mutatva"

MConfigInterfaceTitle
l:
"Настройки интерфейса"
"Interface settings"
"Nastavení rozhraní"
"Oberfläche einrichten"
"Kezelőfelület beállítások"

MConfigClock
"&Часы в панелях"
"&Clock in panels"
"&Hodiny v panelech"
"&Uhr in Panels anzeigen"
"Ór&a a paneleken"

MConfigViewerEditorClock
"Ч&асы при редактировании и просмотре"
"C&lock in viewer and editor"
"H&odiny v prohlížeči a editoru"
"U&hr in Betrachter und Editor anzeigen"
"Ó&ra a Nézőkében és Szerkesztőben"

MConfigMouse
"Мы&шь"
"M&ouse"
"M&yš"
"M&aus aktivieren"
"&Egér kezelése"

MConfigKeyBar
"Показывать &линейку клавиш"
"Show &key bar"
"Zobrazovat &zkratkové klávesy"
"Tast&enleiste anzeigen"
"&Funkcióbillentyűk sora mutatva"

MConfigMenuBar
"Всегда показывать &меню"
"Always show &menu bar"
"Vždy zobrazovat hlavní &menu"
"&Menüleiste immer anzeigen"
"A &menüsor mindig látszik"

MConfigSaver
"&Сохранение экрана"
"&Screen saver"
"Sp&ořič obrazovky"
"Bildschirm&schoner"
"&Képernyőpihentető"

MConfigSaverMinutes
"минут"
"minutes"
"minut"
"Minuten"
"perc tétlenség után"

MConfigUsePromptFormat
"Установить &формат командной строки"
"Set command line &prompt format"
"Nastavit formát &příkazového řádku"
"&Promptformat der Kommandozeile"
"Parancssori &prompt formátuma"

MConfigCopyTotal
"Показывать &общий индикатор копирования"
"Show &total copy progress indicator"
"Zobraz. ukazatel celkového stavu &kopírování"
"Zeige Gesamtfor&tschritt beim Kopieren"
"Másolás összesen folyamat&jelző"

MConfigCopyTimeRule
"Показывать информацию о времени &копирования"
"Show cop&ying time information"
"Zobrazovat informace o čase kopírování"
"Zeige Rest&zeit beim Kopieren"
"Má&solási idő mutatva"

MConfigPgUpChangeDisk
"Использовать Ctrl-PgUp для в&ыбора диска"
"Use Ctrl-Pg&Up to change drive"
"Použít Ctrl-Pg&Up pro změnu disku"
"Strg-Pg&Up wechselt das Laufwerk"
"A Ctrl-Pg&Up meghajtót vált"

MConfigDlgSetsTitle
l:
"Настройки диалогов"
"Dialog settings"
"Nastavení dialogů"
"Dialoge einrichten"
"Párbeszédablak beállítások"

MConfigDialogsEditHistory
"&История в строках ввода диалогов"
"&History in dialog edit controls"
"H&istorie v dialozích"
"&Historie in Eingabefelder von Dialogen"
"Pár&beszédablak előzmények"

MConfigDialogsEditBlock
"&Постоянные блоки в строках ввода"
"&Persistent blocks in edit controls"
"&Trvalé bloky v editačních polích"
"Dauer&hafte Markierungen in Eingabefelder"
"Maradó b&lokkok a párbeszédablakokban"

MConfigDialogsDelRemovesBlocks
"Del удаляет б&локи в строках ввода"
"&Del removes blocks in edit controls"
"&Del maže položky v editačních polích"
"&Entf löscht Markierungen"
"A &Del törli a szerk. ablak blokkjait"

MConfigDialogsAutoComplete
"&Автозавершение в строках ввода"
"&AutoComplete in edit controls"
"Automatické dokončování v editač&ních polích"
"&Automatisches Vervollständigen"
"A&utomatikus kiegészítés a beviteli mezőknél"

MConfigDialogsEULBsClear
"Backspace &удаляет неизмененный текст"
"&Backspace deletes unchanged text"
"&Backspace maže nezměněný text"
"&Rücktaste (BS) löscht unveränderten Text"
"A Ba&ckspace törli a változatlan szöveget"

MConfigDialogsMouseButton
"Клик мыши &вне диалога закрывает диалог"
"Mouse click &outside a dialog closes it"
"Kl&iknutí myší mimo dialog ho zavře"
"Dial&og schließen wenn Mausklick ausserhalb"
"&Egérkattintás a párb.ablakon kívül = bezárja"

MViewConfigTitle
l:
"Программа просмотра"
"Viewer"
"Prohlížeč"
"Betrachter"
"Nézőke"

MViewConfigExternal
"Внешняя программа просмотра:"
"External viewer:"
"Externí prohlížeč"
"Externer Betracher:"
"Külső Nézőke:"

MViewConfigExternalF3
"Запускать по F3"
"Use for F3"
"Použít pro F3"
"Starten mit F3"
"F3-ra"

MViewConfigExternalAltF3
"Запускать по Alt-F3"
"Use for Alt-F3"
"Použít pro Alt-F3"
"Starten mit Alt-F3"
"Alt-F3-ra"

MViewConfigExternalCommand
"&Команда просмотра:"
"&Viewer command:"
"&Příkaz prohlížeče:"
"Befehl für e&xternen Betracher:"
"Nézőke &parancs:"

MViewConfigInternal
"Встроенная программа просмотра:"
"Internal viewer:"
"Interní prohlížeč"
"Interner Betracher:"
"Belső Nézőke:"

MViewConfigSavePos
"&Сохранять позицию файла"
"&Save file position"
"&Ukládat pozici v souboru"
"Dateipositionen &speichern"
"&Fájlpozíció mentése"

MViewConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"
"Ukládat &záložky"
"&Lesezeichen speichern"
"Könyv&jelzők mentése"

MViewAutoDetectCodePage
"&Автоопределение кодовой страницы"
"&Autodetect code page"
upd:"&Autodetekovat znakovou sadu"
upd:"Zeichentabelle &automatisch erkennen"
"&Kódlap automatikus felismerése"

MViewConfigTabSize
"Раз&мер табуляции"
"Tab si&ze"
"Velikost &Tabulátoru"
"Ta&bulatorgröße"
"Ta&bulátor mérete"

MViewConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"
"Zobrazovat posu&vník"
"Scro&llbalken anzeigen"
"Gör&dítősáv mutatva"

MViewConfigArrows
"Показывать стрелки с&двига"
"Show scrolling arro&ws"
"Zobrazovat &skrolovací šipky"
"P&feile bei Scrollbalken zeigen"
"Gördítőn&yilak mutatva"

MViewConfigPersistentSelection
"Постоянное &выделение"
"&Persistent selection"
"Trvalé &výběry"
"Dauerhafte Text&markierungen"
"&Maradó blokkok"

MViewConfigAnsiCodePageAsDefault
"&Использовать кодовую страницу ANSI по умолчанию"
"Use ANS&I code page by default"
upd:"Automaticky otevírat soubory ve &WIN kódování"
upd:"Dateien standardmäßig mit Windows-Kod&ierung öffnen"
"Fájlok eredeti megnyitása ANS&I kódlappal"

MEditConfigTitle
l:
"Редактор"
"Editor"
"Editor"
"Editor"
"Szerkesztő"

MEditConfigExternal
"Внешний редактор:"
"External editor:"
"Externí editor"
"Externer Editor:"
"Külső Szerkesztő:"

MEditConfigEditorF4
"Запускать по F4"
"Use for F4"
"Použít pro F4"
"Starten mit F4"
"F4-re"

MEditConfigEditorAltF4
"Запускать по Alt-F4"
"Use for Alt-F4"
"Použít pro Alt-F4"
"Starten mit Alt-F4"
"Alt-F4-re"

MEditConfigEditorCommand
"&Команда редактирования:"
"&Editor command:"
"&Příkaz editoru:"
"Befehl für e&xternen Editor:"
"&Szerkesztő parancs:"

MEditConfigInternal
"Встроенный редактор:"
"Internal editor:"
"Interní editor"
"Interner Editor:"
"Belső Szerkesztő:"

MEditConfigExpandTabsTitle
"Преобразовывать &табуляцию:"
"Expand &tabs:"
"Rozšířit Ta&bulátory mezerami"
"&Tabs expandieren:"
"&Tabulátorból szóközök:"

MEditConfigDoNotExpandTabs
l:
"Не преобразовывать табуляцию"
"Do not expand tabs"
"Nerozšiřovat tabulátory mezerami"
"Tabs nicht expandieren"
"Ne helyettesítse a tabulátorokat"

MEditConfigExpandTabs
"Преобразовывать новые символы табуляции в пробелы"
"Expand newly entered tabs to spaces"
"Rozšířit nově zadané tabulátory mezerami"
"Neue Tabs zu Leerzeichen expandieren"
"Újonnan beírt tabulátorból szóközök"

MEditConfigConvertAllTabsToSpaces
"Преобразовывать все символы табуляции в пробелы"
"Expand all tabs to spaces"
"Rozšířit všechny tabulátory mezerami"
"Alle Tabs zu Leerzeichen expandieren"
"Minden tabulátorból szóközök"

MEditConfigPersistentBlocks
"&Постоянные блоки"
"&Persistent blocks"
"&Trvalé bloky"
"Dauerhafte Text&markierungen"
"&Maradó blokkok"

MEditConfigDelRemovesBlocks
l:
"Del удаляет б&локи"
"&Del removes blocks"
"&Del maže bloky"
"&Entf löscht Textmark."
"A &Del törli a blokkokat"

MEditConfigAutoIndent
"Авто&отступ"
"Auto &indent"
"Auto &Odsazování"
"Automatischer E&inzug"
"Automatikus &behúzás"

MEditConfigSavePos
"&Сохранять позицию файла"
"&Save file position"
"&Ukládat pozici v souboru"
"Dateipositionen &speichern"
"Fájl&pozíció mentése"

MEditConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"
"Ukládat zá&ložky"
"&Lesezeichen speichern"
"Könyv&jelzők mentése"

MEditAutoDetectCodePage
"&Автоопределение кодовой страницы"
"&Autodetect code page"
upd:"&Autodetekovat znakovou sadu"
upd:"Zeichentabelle &automatisch erkennen"
"&Kódlap automatikus felismerése"

MEditCursorBeyondEnd
"Ку&рсор за пределами строки"
"&Cursor beyond end of line"
"&Kurzor za koncem řádku"
"&Cursor hinter dem Ende der Zeile zulassen"
"Kurzor a sor&végjel után is"

MEditLockROFileModification
"Блокировать р&едактирование файлов с атрибутом R/O"
"Lock editing of read-only &files"
"&Zamknout editaci souborů určených jen pro čtení"
"Bearbeiten von &Dateien mit Schreibschutz verhindern"
"Csak olvasható fájlok s&zerkesztése tiltva"

MEditWarningBeforeOpenROFile
"Пре&дупреждать при открытии файла с атрибутом R/O"
"&Warn when opening read-only files"
"&Varovat při otevření souborů určených jen pro čtení"
"Beim Öffnen von Dateien mit Schreibschutz &warnen"
"Figyelmeztet &csak olvasható fájl megnyitásakor"

MEditConfigTabSize
"Раз&мер табуляции"
"Tab si&ze"
"Velikost &Tabulátoru"
"Ta&bulatorgröße"
"Tab&ulátor mérete"

MEditConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"
"Zobr&azovat posuvník"
"Scro&llbalken anzeigen"
"&Gördítősáv mutatva"

MEditConfigAnsiCodePageAsDefault
"&Использовать кодовую страницу ANSI по умолчанию"
"Use ANS&I code page by default"
upd:"Automaticky otevírat soubory ve &WIN kódování"
upd:"Dateien standardmäßig mit Windows-Kod&ierung öffnen"
"Fájlok eredeti megnyitása ANS&I kódlappal"

MEditConfigAnsiCodePageForNewFile
"Использо&вать кодовую страницу ANSI при создании файлов"
"Use ANSI code page when c&reating new files"
upd:"V&ytvářet nové soubory ve WIN kódování"
upd:"Neue Dateien mit Windows-Ko&dierung erstellen"
"Új &fájl létrehozása ANSI kódlappal"

MSaveSetupTitle
l:
"Конфигурация"
"Save setup"
"Uložit nastavení"
"Einstellungen speichern"
"Beállítások mentése"

MSaveSetupAsk1
"Вы хотите сохранить"
"Do you wish to save"
"Přejete si uložit"
"Wollen Sie die aktuellen Einstellungen"
"El akarja menteni a"

MSaveSetupAsk2
"текущую конфигурацию?"
"current setup?"
"aktuální nastavení?"
"speichern?"
"jelenlegi beállításokat?"

MSaveSetup
"Сохранить"
"Save"
"Uložit"
"Speichern"
"Mentés"

MCopyDlgTitle
l:
"Копирование"
"Copy"
"Kopírovat"
"Kopieren"
"Másolás"

MMoveDlgTitle
"Переименование/Перенос"
"Rename/Move"
"Přejmenovat/Přesunout"
"Verschieben/Umbenennen"
"Átnevezés/mozgatás"

MLinkDlgTitle
"Ссылка"
"Link"
"Link"
"Link erstellen"
"Link létrehozása"

MCopySecurity
"П&рава доступа:"
"&Access rights:"
"&Přístupová práva:"
"Zugriffsrecht&e:"
"Hozzáférési &jogok:"

MCopySecurityCopy
"Копироват&ь"
"Co&py"
"&Kopírovat"
"Ko&pieren"
"Más&ol"

MCopySecurityInherit
"Нас&ледовать"
"&Inherit"
"&Zdědit"
"Ve&rerben"
"Ö&rököl"

MCopySecurityLeave
"По умол&чанию"
"Defau&lt"
"Vých&ozí"
"A&utomat."
"Ala&pért."

MCopyIfFileExist
"Уже су&ществующие файлы:"
"Already e&xisting files:"
"Již e&xistující soubory:"
"&Dateien überschreiben:"
"Már &létező fájloknál:"

MCopyAsk
"&Запрос действия"
"&Ask"
"Ptát s&e"
"Fr&agen"
"Kér&dez"

MCopyAskRO
"Запрос подтверждения для &R/O файлов"
"Also ask on &R/O files"
"Ptát se také na &R/O soubory"
"Bei Dateien mit Sch&reibschutz fragen"
"&Csak olvasható fájloknál is kérdez"

MCopyOnlyNewerFiles
"Только &новые/обновленные файлы"
"Only ne&wer file(s)"
"Pouze &novější soubory"
"Nur &neuere Dateien"
"Cs&ak az újabb fájlokat"

MLinkType
"&Тип ссылки:"
"Link t&ype:"
"&Typ linku:"
"Linkt&yp:"
"Link &típusa:"

MLinkTypeJunction
"&связь каталогов"
"directory &junction"
"křížení a&dresářů"
"Ordner&knotenpunkt"
"Mappa &csomópont"

MLinkTypeHardlink
"&жёсткая ссылка"
"&hard link"
"&pevný link"
"&Hardlink"
"&Hardlink"

MLinkTypeSymlinkFile
"символическая ссылка (&файл)"
"symbolic link (&file)"
"symbolický link (&soubor)"
"Symbolischer Link (&Datei)"
"Szimbolikus link (&fájl)"

MLinkTypeSymlinkDirectory
"символическая ссылка (&папка)"
"symbolic link (fol&der)"
"symbolický link (&adresář)"
"Symbolischer Link (Or&dner)"
"Szimbolikus link (&mappa)"

MCopySymLinkContents
"Копировать содерж&имое символических ссылок"
"Cop&y contents of symbolic links"
"Kopírovat obsah sym&bolických linků"
"Inhalte von s&ymbolischen Links kopieren"
"Sz&imbolikus linkek másolása"

MCopyMultiActions
"Обр&абатывать несколько имен файлов"
"Process &multiple destinations"
"&Zpracovat více míst určení"
"&Mehrere Ziele verarbeiten"
"Tö&bbszörös cél létrehozása"

MCopyDlgCopy
"&Копировать"
"&Copy"
"&Kopírovat"
"&Kopieren"
"&Másolás"

MCopyDlgTree
"F10-&Дерево"
"F10-&Tree"
"F10-&Strom"
"F10-&Baum"
"F10-&Fa"

MCopyDlgCancel
"&Отменить"
"Ca&ncel"
"&Storno"
"Ab&bruch"
"Még&sem"

MCopyDlgRename
"&Переименовать"
"&Rename"
"Přej&menovat"
"&Umbenennen"
"Át&nevez"

MCopyDlgLink
"&Создать ссылку"
"&Link"
"&Linkovat"
"Ver&linken"
"&Linkel"

MCopyDlgTotal
"Всего"
"Total"
"Celkem"
"Gesamt"
"Összesen"

MCopyScanning
"Сканирование папок..."
"Scanning folders..."
"Načítání adresářů..."
"Scanne Ordner..."
"Mappák olvasása..."

MCopyPrepareSecury
"Применение прав доступа..."
"Applying access rights..."
"Nastavuji přístupová práva..."
"Anwenden der Zugriffsrechte..."
"Hozzáférési jogok alkalmazása..."

MCopyUseFilter
"Исполь&зовать фильтр"
"&Use filter"
"P&oužít filtr"
"Ben&utze Filter"
"Szűrő&vel"

MCopySetFilter
"&Фильтр"
"Filt&er"
"Filt&r"
"Filt&er"
"S&zűrő"

MCopyFile
l:
"Копировать \"%.55s\""
"Copy \"%.55s\""
"Kopírovat \"%.55s\""
"Kopiere \"%.55s\""
" \"%.55s\" másolása"

MMoveFile
"Переименовать или перенести \"%.55s\""
"Rename or move \"%.55s\""
"Přejmenovat nebo přesunout \"%.55s\""
"Verschiebe \"%.55s\""
" \"%.55s\" átnevezése/mozgatása"

MLinkFile
"Создать ссылку \"%.55s\""
"Link \"%.55s\""
"Linkovat \"%.55s\""
"Verlinke \"%.55s\""
" \"%.55s\" linkelése"

MCopyFiles
"Копировать %d элемент%s"
"Copy %d item%s"
"Kopírovat %d polož%s"
"Kopiere %d Objekt%s"
" %d elem másolása"

MMoveFiles
"Переименовать или перенести %d элемент%s"
"Rename or move %d item%s"
"Přejmenovat nebo přesunout %d polož%s"
"Verschiebe %d Objekt%s"
" %d elem átnevezése/mozgatása"

MLinkFiles
"Создать ссылки %d элемент%s"
"Link %d item%s"
"Linkovat %d polož%s"
"Verlinke %d Objekt%s"
" %d elem linkelése"

MCMLTargetTO
" &в:"
" t&o:"
" d&o:"
" na&ch:"
" ide:"

MCMLItems0
""
""
"u"
""
""

MCMLItemsA
"а"
"s"
"ek"
"e"
""

MCMLItemsS
"ов"
"s"
"ky"
"e"
""

MCopyIncorrectTargetList
l:
"Указан некорректный список целей!"
"Incorrect target list!"
"Nesprávný seznam cílů!"
"Ungültige Liste von Zielen!"
"Érvénytelen céllista!"

MCopyCopyingTitle
l:
"Копирование"
"Copying"
"Kopíruji"
"Kopieren"
"Másolás"

MCopyMovingTitle
"Перенос"
"Moving"
"Přesouvám"
"Verschieben"
"Mozgatás"

MCopyCannotFind
l:
"Файл не найден"
"Cannot find the file"
"Nelze nalézt soubor"
"Folgende Datei kann nicht gefunden werden:"
"A fájl nem található:"

MCannotCopyFolderToItself1
l:
"Нельзя копировать папку"
"Cannot copy the folder"
"Nelze kopírovat adresář"
"Folgender Ordner kann nicht kopiert werden:"
"A mappa:"

MCannotCopyFolderToItself2
"в саму себя"
"onto itself"
"sám na sebe"
"Ziel und Quelle identisch."
"nem másolható önmagába/önmagára"

MCannotCopyToTwoDot
l:
"Нельзя копировать файл или папку"
"You may not copy files or folders"
"Nelze kopírovat soubory nebo adresáře"
"Kopieren von Dateien oder Ordnern ist maximal"
"Nem másolhatja a fájlt vagy mappát"

MCannotMoveToTwoDot
"Нельзя перемещать файл или папку"
"You may not move files or folders"
"Nelze přesunout soubory nebo adresáře"
"Verschieben von Dateien oder Ordnern ist maximal"
"Nem mozgathatja a fájlt vagy mappát"

MCannotCopyMoveToTwoDot
"выше корневого каталога"
"higher than the root folder"
"na vyšší úroveň než kořenový adresář"
"bis zum Wurzelverzeichnis möglich."
"a gyökér fölé"

MCopyCannotCreateFolder
l:
"Ошибка создания папки"
"Cannot create the folder"
"Nelze vytvořit adresář"
"Folgender Ordner kann nicht erstellt werden:"
"A mappa nem hozható létre:"

MCopyCannotChangeFolderAttr
"Невозможно установить атрибуты для папки"
"Failed to set folder attributes"
"Nastavení atributů adresáře selhalo"
"Fehler beim Setzen der Ordnerattribute"
"A mappa attribútumok beállítása sikertelen"

MCopyCannotRenameFolder
"Невозможно переименовать папку"
"Cannot rename the folder"
"Nelze přejmenovat adresář"
"Folgender Ordner kann nicht umbenannt werden:"
"A mappa nem nevezhető át:"

MCopyIgnore
"&Игнорировать"
"&Ignore"
"&Ignorovat"
"&Ignorieren"
"Mé&gis"

MCopyIgnoreAll
"Игнорировать &все"
"Ignore &All"
"Ignorovat &vše"
"&Alle ignorieren"
"Min&d"

MCopyRetry
"&Повторить"
"&Retry"
"&Opakovat"
"Wiede&rholen"
"Ú&jra"

MCopySkip
"П&ропустить"
"&Skip"
"&Přeskočit"
"Ausla&ssen"
"&Kihagy"

MCopySkipAll
"&Пропустить все"
"S&kip all"
"Př&eskočit vše"
"Alle aus&lassen"
"Mi&nd"

MCopyCancel
"&Отменить"
"&Cancel"
"&Storno"
"Abb&rechen"
"Még&sem"

MCopyDecrypt
"Рас&шифровать"
"&Decrypt"
"&Dešifrovat"
"Ent&schlüsseln"
"&Titk. felold"

MCopyDecryptAll
"&Все"
"Decrypt &all"
"Deš&ifrovat vše"
"Alle e&ntschlüsseln"
"&Mind"

MCopyCannotCreateLink
l:
"Ошибка создания ссылки"
"Cannot create the link"
"Nelze vytvořit symbolický link"
"Folgender Link kann nicht erstellt werden:"
"A link nem hozható létre:"

MCopyFolderNotEmpty
"Папка назначения должна быть пустой"
"Target folder must be empty"
"Cílový adresář musí být prázdný"
"Zielordner muss leer sein."
"A célmappának üresnek kell lennie"

MCopyCannotCreateJunctionToFile
"Невозможно создать связь. Файл уже существует:"
"Cannot create junction. The file already exists:"
"Nelze vytvořit křížový odkaz. Soubor již existuje:"
"Knotenpunkt wurde nicht erstellt. Datei existiert bereits:"
"A csomópont nem hozható létre. A fájl már létezik:"

MCopyCannotCreateVolMount
l:
"Ошибка монтирования диска"
"Volume mount points error"
"Chyba připojovacích svazků"
"Fehler im Mountpoint des Datenträgers"
"Kötet mountpont hiba"

MCopyRetrVolFailed
"Невозможно получить информацию о '%s'"
"Retrieving volume name for '%s' failed"
"Získání jména zvazku pro '%s' selhalo"
"Fehler beim Lesen der Datenträgerbezeichnung von '%s'"
""%s" kötetnevének lekérése sikertelen"

MCopyMountVolFailed
"Ошибка при монтировании диска '%s'"
"Attempt to volume mount '%s'"
"Pokus o připojení svazku '%s'"
"Versuch Datenträger '%s' zu aktivieren"
""%s" kötet mountolása"

MCopyMountVolFailed2
"на '%s'"
"at '%s' failed"
"na '%s' selhal"
"fehlgeschlagen bei '%s'"
"nem sikerült: "%s""

MCopyCannotSupportVolMount
"Функция монтирования дисков не поддерживается"
"Volume mounting is not supported"
"Připojování svazku není podporováno"
"Datenträgeraktivierung wird nicht unterstützt"
"Kötet mountolása nem támogatott"

MCopyMountName
"disk_%c"
"Disk_%c"
"Disk_%c"
"Disk_%c"
"Disk_%c"

MCannotCopyFileToItself1
l:
"Нельзя копировать файл"
"Cannot copy the file"
"Nelze kopírovat soubor"
"Folgende Datei kann nicht kopiert werden:"
"A fájl:"

MCannotCopyFileToItself2
"в самого себя"
"onto itself"
"sám na sebe"
"Ziel und Quelle identisch."
"nem másolható önmagára"

MCopyStream1
l:
"Исходный файл содержит более одного потока данных,"
"The source file contains more than one data stream."
"Zdrojový soubor obsahuje více než jeden datový proud."
"Die Quelldatei enthält mehr als einen Datenstream"
"A forrásfájl több stream-et tartalmaz,"

MCopyStream2
"но вы не используете системную функцию копирования."
"but since you do not use a system copy routine."
"protože nepoužíváte systémovou kopírovací rutinu."
"aber Sie verwenden derzeit nicht die systemeigene Kopierroutine."
"de nem a rendszer másolórutinját használja."

MCopyStream3
"но том назначения не поддерживает этой возможности."
"but the destination volume does not support this feature."
"protože cílový svazek nepodporuje tuto vlastnost."
"aber der Zieldatenträger unterstützt diese Fähigkeit nicht."
"de a célkötet nem támogatja ezt a lehetőséget."

MCopyStream4
"Часть сведений не будет сохранена."
"Some data will not be preserved as a result."
"To bude mít za následek, že některá data nebudou uchována."
"Ein Teil der Daten bleiben daher nicht erhalten."
"Az adatok egy része el fog veszni."

MCopyFileExist
l:
"Файл уже существует"
"File already exists"
"Soubor již existuje"
"Datei existiert bereits"
"A fájl már létezik:"

MCopySource
"&Новый"
"&New"
"&Nový"
"&Neue Datei"
"Új verzió:"

MCopyDest
"Су&ществующий"
"E&xisting"
"E&xistující"
"Be&stehende Datei"
"Létező verzió:"

MCopyOverwrite
"В&место"
"&Overwrite"
"&Přepsat"
"Über&schr."
"&Felülír"

MCopyContinue
"П&родолжить"
"C&ontinue"
"P&okračovat"
"F&ortsetzen"
"M&égis"

MCopySkipOvr
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&spr."
"&Kihagy"

MCopyAppend
"&Дописать"
"A&ppend"
"Př&ipojit"
"&Anhängen"
"Hoz&záfűz"

MCopyResume
"Возоб&новить"
"&Resume"
"Po&kračovat"
"&Weiter"
"Fol&ytat"

MCopyCancelOvr
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Mégsem"

MCopyRememberChoice
"&Запомнить выбор"
"&Remember choice"
"Zapama&tovat volbu"
"Auswahl me&rken"
"Mind&ent a kiválasztott módon"

MCopyFileRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"
"Soubor je určen pouze pro čtení"
"Folgende Datei ist schreibgeschützt:"
"A fájl csak olvasható:"

MCopyAskDelete
"Вы хотите удалить его?"
"Do you wish to delete it?"
"Opravdu si ho přejete smazat?"
"Wollen Sie sie dennoch löschen?"
"Biztosan törölni akarja?"

MCopyDeleteRO
"&Удалить"
"&Delete"
"S&mazat"
"&Löschen"
"&Törli"

MCopyDeleteAllRO
"&Все"
"&All"
"&Vše"
"&Alle Löschen"
"Min&det"

MCopySkipRO
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&springen"
"&Kihagyja"

MCopySkipAllRO
"П&ропустить все"
"S&kip all"
"Př&eskočit vše"
"A&lle überspringen"
"Mind&et"

MCopyCancelRO
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Mégsem"

MCannotCopy
l:
"Ошибка копирования"
"Cannot copy"
"Nelze kopírovat"
"Konnte nicht kopieren"
"Nem másolható"

MCannotMove
"Ошибка переноса"
"Cannot move"
"Nelze přesunout"
"Konnte nicht verschieben"
"Nem mozgatható"

MCannotLink
"Ошибка создания ссылки"
"Cannot link"
"Nelze linkovat"
"Konnte nicht verlinken"
"Nem linkelhető"

MCannotCopyTo
"в"
"to"
"do"
"nach"
"ide:"

MCopyEncryptWarn1
"Файл"
"The file"
"Soubor"
"Die Datei"
"A fájl"

MCopyEncryptWarn2
"нельзя скопировать или переместить, не потеряв его шифрование."
"cannot be copied or moved without losing its encryption."
"nemůže být zkopírován nebo přesunut bez ztráty jeho šifrování."
"kann nicht bewegt werden ohne ihre Verschlüsselung zu verlieren."
"csak titkosítása elvesztésével másolható vagy mozgatható."

MCopyEncryptWarn3
"Можно пропустить эту ошибку или отменить операцию."
"You can choose to ignore this error and continue, or cancel."
"Můžete tuto chybu ignorovat a pokračovat, nebo operaci ukončit."
"Sie können dies ignorieren und fortfahren oder abbrechen."
"Ennek ellenére folytathatja vagy felfüggesztheti."

MCopyReadError
l:
"Ошибка чтения данных из"
"Cannot read data from"
"Nelze číst data z"
"Kann Daten nicht lesen von"
"Nem olvasható adat innen:"

MCopyWriteError
"Ошибка записи данных в"
"Cannot write data to"
"Nelze zapsat data do"
"Dann Daten nicht schreiben in"
"Nem írható adat ide:"

MCopyProcessed
l:
"Обработано файлов: %d"
"Files processed: %d"
"Zpracováno souborů: %d"
"Dateien verarbeitet: %d"
" %d fájl kész"

MCopyProcessedTotal
"Обработано файлов: %d из %d"
"Files processed: %d of %d"
"Zpracováno souborů: %d z %d"
"Dateien verarbeitet: %d von %d"
" %d fájl kész %d fájlból"

MCopyMoving
"Перенос файла"
"Moving the file"
"Přesunuji soubor"
"Verschiebe die Datei"
"Fájl mozgatása"

MCopyCopying
"Копирование файла"
"Copying the file"
"Kopíruji soubor"
"Kopiere die Datei"
"Fájl másolása"

MCopyTo
"в"
"to"
"do"
"nach"
"ide:"

MCopyErrorDiskFull
l:
"Диск заполнен. Вставьте следующий"
"Disk full. Insert next"
"Disk je plný. Vložte dalšíí"
"Datenträger voll. Bitte nächsten einlegen"
"A lemez megtelt, kérem a következőt!"

MDeleteTitle
l:
"Удаление"
"Delete"
"Smazat"
"Löschen"
"Törlés"

MAskDeleteFolder
"Вы хотите удалить папку"
"Do you wish to delete the folder"
"Přejete si smazat adresář"
"Wollen Sie den Ordner löschen"
"Törölni akarja a mappát?"

MAskDeleteFile
"Вы хотите удалить файл"
"Do you wish to delete the file"
"Přejete si smazat soubor"
"Wollen Sie die Datei löschen"
"Törölni akarja a fájlt?"

MAskDelete
"Вы хотите удалить"
"Do you wish to delete"
"Přejete si smazat"
"Wollen Sie folgendes Objekt löschen"
"Törölni akar"

MAskDeleteRecycleFolder
"Вы хотите поместить в Корзину папку"
"Do you wish to move to the Recycle Bin the folder"
"Přejete si přesunout do Koše adresář"
"Wollen Sie den Ordner in den Papierkorb verschieben"
"A Lomtárba akarja dobni a mappát?"

MAskDeleteRecycleFile
"Вы хотите поместить в Корзину файл"
"Do you wish to move to the Recycle Bin the file"
"Přejete si přesunout do Koše soubor"
"Wollen Sie die Datei in den Papierkorb verschieben"
"A Lomtárba akarja dobni a fájlt?"

MAskDeleteRecycle
"Вы хотите поместить в Корзину"
"Do you wish to move to the Recycle Bin"
"Přejete si přesunout do Koše"
"Wollen Sie das Objekt in den Papierkorb verschieben"
"A Lomtárba akar dobni"

MDeleteWipeTitle
"Уничтожение"
"Wipe"
"Vymazat"
"Sicheres Löschen"
"Kisöprés"

MAskWipeFolder
"Вы хотите уничтожить папку"
"Do you wish to wipe the folder"
"Přejete si vymazat adresář"
"Wollen Sie den Ordner sicher löschen"
"Ki akarja söpörni a mappát?"

MAskWipeFile
"Вы хотите уничтожить файл"
"Do you wish to wipe the file"
"Přejete si vymazat soubor"
"Wollen Sie die Datei sicher löschen"
"Ki akarja söpörni a fájlt?"

MAskWipe
"Вы хотите уничтожить"
"Do you wish to wipe"
"Přejete si vymazat"
"Wollen Sie das Objekt sicher löschen"
"Ki akar söpörni"

MDeleteLinkTitle
"Удаление ссылки"
"Delete link"
"Smazat link"
"Link löschen"
"Link törlése"

MAskDeleteLink
"является ссылкой на"
"is a symbolic link to"
"je symbolicky link na"
"ist ein symbolischer Link auf"
"szimlinkelve ide:"

MAskDeleteLinkFolder
"папку"
"folder"
"adresář"
"Ordner"
"mappa"

MAskDeleteLinkFile
"файл"
"file"
"soubor"
"Date"
"fájl"

MAskDeleteItems
"%d элемент%s"
"%d item%s"
"%d polož%s"
"%d Objekt%s"
"%d elemet?%s"

MAskDeleteItems0
""
""
"ku"
""
""

MAskDeleteItemsA
"а"
"s"
"ky"
"e"
""

MAskDeleteItemsS
"ов"
"s"
"ek"
"e"
""

MDeleteFolderTitle
l:
"Удаление папки "
"Delete folder"
"Smazat adresář"
"Ordner löschen"
"Mappa törlése"

MWipeFolderTitle
"Уничтожение папки "
"Wipe folder"
"Vymazat adresář"
"Ordner sicher löschen"
"Mappa kisöprése"

MDeleteFilesTitle
"Удаление файлов"
"Delete files"
"Smazat soubory"
"Dateien löschen"
"Fájlok törlése"

MWipeFilesTitle
"Уничтожение файлов"
"Wipe files"
"Vymazat soubory"
"Dateien sicher löschen"
"Fájlok kisöprése"

MDeleteFolderConfirm
"Данная папка будет удалена:"
"The following folder will be deleted:"
"Následující adresář bude smazán:"
"Folgender Ordner wird gelöscht:"
"A mappa törlődik:"

MWipeFolderConfirm
"Данная папка будет уничтожена:"
"The following folder will be wiped:"
"Následující adresář bude vymazán:"
"Folgender Ordner wird sicher gelöscht:"
"A mappa kisöprődik:"

MDeleteWipe
"Уничтожить"
"Wipe"
"Vymazat"
"Sicheres Löschen"
"Kisöpör"

MDeleteFileDelete
"&Удалить"
"&Delete"
"S&mazat"
"&Löschen"
"&Töröl"

MDeleteFileWipe
"&Уничтожить"
"&Wipe"
"V&ymazat"
"&Sicher löschen"
"Kisö&pör"

MDeleteFileAll
"&Все"
"&All"
"&Vše"
"&Alle"
"Min&det"

MDeleteFileSkip
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&springen"
"&Kihagy"

MDeleteFileSkipAll
"П&ропустить все"
"S&kip all"
"Př&eskočit vše"
"A&lle überspr."
"Mind&et"

MDeleteFileCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Mégsem"

MDeleteLinkDelete
l:
"Удалить ссылку"
"Delete link"
"Smazat link"
"Link löschen"
"Link törlése"

MDeleteLinkUnlink
"Разорвать ссылку"
"Break link"
"Poškozený link"
"Link auflösen"
"Link megszakítása"

MDeletingTitle
l:
"Удаление"
"Deleting"
"Mazání"
"Lösche"
"Törlés"

MDeleting
l:
"Удаление файла или папки"
"Deleting the file or folder"
"Mazání souboru nebo adresáře"
"Löschen von Datei oder Ordner"
"Fájl vagy mappa törlése"

MDeletingWiping
"Уничтожение файла или папки"
"Wiping the file or folder"
"Vymazávání souboru nebo adresáře"
"Sicheres löschen von Datei oder Ordner"
"Fájl vagy mappa kisöprése"

MDeleteRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"
"Soubor je určen pouze pro čtení"
"Folgende Datei ist schreibgeschützt:"
"A fájl csak olvasható:"

MAskDeleteRO
"Вы хотите удалить его?"
"Do you wish to delete it?"
"Opravdu si ho přejete smazat?"
"Wollen Sie sie dennoch löschen?"
"Mégis törölni akarja?"

MAskWipeRO
"Вы хотите уничтожить его?"
"Do you wish to wipe it?"
"Opravdu si ho přejete vymazat?"
"Wollen Sie sie dennoch sicher löschen?"
"Mégis ki akarja söpörni?"

MDeleteHardLink1
l:
"Файл имеет несколько жестких ссылок"
"Several hard links link to this file."
"Více pevných linků ukazuje na tento soubor."
"Mehrere Hardlinks zeigen auf diese Datei."
"Több hardlink kapcsolódik a fájlhoz, a fájl"

MDeleteHardLink2
"Уничтожение файла приведет к обнулению всех ссылающихся на него файлов."
"Wiping this file will void all files linking to it."
"Vymazání tohoto souboru zneplatní všechny soubory, které na něj linkují."
"Sicheres Löschen dieser Datei entfernt ebenfalls alle Links."
"kisöprése a linkelt fájlokat is megsemmisíti."

MDeleteHardLink3
"Уничтожать файл?"
"Do you wish to wipe this file?"
"Opravdu chcete vymazat tento soubor?"
"Wollen Sie diese Datei sicher löschen?"
"Biztosan kisöpri a fájlt?"

MCannotDeleteFile
l:
"Ошибка удаления файла"
"Cannot delete the file"
"Nelze smazat soubor"
"Datei konnte nicht gelöscht werden"
"A fájl nem törölhető"

MCannotDeleteFolder
"Ошибка удаления папки"
"Cannot delete the folder"
"Nelze smazat adresář"
"Ordner konnte nicht gelöscht werden"
"A mappa nem törölhető"

MDeleteRetry
"&Повторить"
"&Retry"
"&Znovu"
"Wiede&rholen"
"Ú&jra"

MDeleteSkip
"П&ропустить"
"&Skip"
"Přes&kočit"
"Über&springen"
"&Kihagy"

MDeleteSkipAll
"Пропустить &все"
"S&kip all"
"Přeskočit &vše"
"A&lle überspr."
"Min&d"

MDeleteCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Mégsem"

MCannotGetSecurity
l:
"Ошибка получения прав доступа к файлу"
"Cannot get file access rights for"
"Nemohu získat přístupová práva pro"
"Kann Zugriffsrechte nicht lesen für"
"A fájlhoz nincs hozzáférési joga:"

MCannotSetSecurity
"Ошибка установки прав доступа к файлу"
"Cannot set file access rights for"
"Nemohu nastavit přístupová práva pro"
"Kann Zugriffsrechte nicht setzen für"
"A fájl hozzáférési jogát nem állíthatja:"

MEditTitle
l:
"Редактор"
"Editor"
"Editor"
"Editor"
"Szerkesztő"

MAskReload
"уже загружен. Как открыть этот файл?"
"already loaded. How to open this file?"
"již otevřen. Jak otevřít tento soubor?"
"bereits geladen. Wie wollen Sie die Datei öffnen?"
"fájl már be van töltve. Hogyan szerkeszti?"

MCurrent
"&Текущий"
"&Current"
"&Stávající"
"A&ktuell"
"A mostanit &folytatja"

MReload
"Пере&грузить"
"R&eload"
"&Znovu načíst"
"Aktualisie&ren"
"Újra&tölti"

MNewOpen
"&Новая копия"
"&New instance"
"&Nová instance"
"&Neue Instanz"
"Ú&j példányban"

MEditCannotOpen
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"
"A fájl nem nyitható meg"

MEditReading
"Чтение файла"
"Reading the file"
"Načítám soubor"
"Lesen der Datei"
"Fájl olvasása"

MEditAskSave
"Файл был изменен"
"File has been modified"
"Soubor byl modifikován"
"Datei wurde verändert"
"A fájl megváltozott"

MEditAskSaveExt
"Файл был изменен внешней программой"
"The file was changed by an external program"
"Soubor byl změněný externím programem"
"Die Datei wurde durch ein externes Programm verändert"
"A fájlt egy másik program megváltoztatta"

MEditSave
l:
"&Сохранить"
"&Save"
"&Uložit"
"&Speichern"
"&Ment"

MEditNotSave
"&Не сохранять"
"Do &not save"
"&Neukládat"
"&Nicht speichern"
"&Nem ment"

MEditContinue
"&Продолжить редактирование"
"&Continue editing"
"&Pokračovat"
"Bearbeiten f&ortsetzen"
"&Szerkesztést folytat"

MEditBtnSaveAs
"Сохр&анить как"
"Save &as..."
"Ulož&it jako..."
"Speichern &als..."
"Mentés más&ként..."

MEditRO
l:
"имеет атрибут \"Только для чтения\""
"is a read-only file"
"je určen pouze pro čtení"
"ist eine schreibgeschützte Datei"
"csak olvasható fájl"

MEditExists
"уже существует"
"already exists"
"již existuje"
"ist bereits vorhanden"
"már létezik"

MEditOvr
"Вы хотите перезаписать его?"
"Do you wish to overwrite it?"
"Přejete si ho přepsat?"
"Wollen Sie die Datei überschreiben?"
"Felül akarja írni?"

MEditSaving
"Сохранение файла"
"Saving the file"
"Ukládám soubor"
"Speichere die Datei"
"Fájl mentése"

MEditStatusLine
"Строка"
"Line"
"Řádek"
"Zeile"
"Sor"

MEditStatusCol
"Кол"
"Col"
"Sloupec"
"Spal"
"Oszlop"

MEditRSH
l:
"предназначен только для чтения"
"is a read-only file"
"je určen pouze pro čtení"
"ist eine schreibgeschützte Datei"
"csak olvasható fájl"

MEditFileLong
"имеет размер %s,"
"has the size of %s,"
"má velikost %s,"
"hat eine Größe von %s,"
"mérete %s,"

MEditFileLong2
"что превышает заданное ограничение в %s."
"which exceeds the configured maximum size of %s."
"která překračuje nastavenou maximální velikost %s."
"die die konfiguierte Maximalgröße von %s überschreitet."
"meghaladja %s beállított maximumát."

MEditROOpen
"Вы хотите редактировать его?"
"Do you wish to edit it?"
"Opravdu si ho přejete upravit?"
"Wollen Sie sie dennoch bearbeiten?"
"Mégis szerkeszti?"

MEditCanNotEditDirectory
l:
"Невозможно редактировать папку"
"It is impossible to edit the folder"
"Nelze editovat adresář"
"Es ist nicht möglich den Ordner zu bearbeiten"
"A mappa nem szerkeszthető"

MEditSearchTitle
l:
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keresés"

MEditSearchFor
"&Искать"
"&Search for"
"&Hledat"
"&Suchen nach"
"&Keresés:"

MEditSearchCase
"&Учитывать регистр"
"&Case sensitive"
"&Rozlišovat velikost písmen"
"G&roß-/Kleinschrb."
"&Nagy/kisbetű érz."

MEditSearchWholeWords
"Только &целые слова"
"&Whole words"
"&Celá slova"
"&Ganze Wörter"
"E&gész szavakra"

MEditSearchReverse
"Обратн&ый поиск"
"Re&verse search"
"&Zpětné hledání"
"Richtung um&kehren"
"&Visszafelé keres"

MEditSearchSelFound
"&Выделять найденное"
"Se&lect found"
"Vy&ber nalezené"
"Treffer &markieren"
"&Találat kijelölése"

MEditSearchSearch
"Искать"
"Search"
"Hledat"
"Suchen"
"Kere&sés"

MEditSearchCancel
"Отменить"
"Cancel"
"Storno"
"Abbruch"
"&Mégsem"

MEditReplaceTitle
l:
"Замена"
"Replace"
"Nahradit"
"Ersetzen"
"Keresés és csere"

MEditReplaceWith
"Заменить &на"
"R&eplace with"
"Nahradit &s"
"&Ersetzen mit"
"&Erre cseréli:"

MEditReplaceReplace
"&Замена"
"&Replace"
"&Nahradit"
"E&rsetzen"
"&Csere"

MEditSearchingFor
l:
"Искать"
"Searching for"
"Vyhledávám"
"Suche nach"
"Keresett szöveg:"

MEditNotFound
"Строка не найдена"
"Could not find the string"
"Nemůžu najít řetězec"
"Konnte Zeichenkette nicht finden"
"A szöveg nem található:"

MEditAskReplace
l:
"Заменить"
"Replace"
"Nahradit"
"Ersetze"
"Ezt cseréli:"

MEditAskReplaceWith
"на"
"with"
"s"
"mit"
"erre a szövegre:"

MEditReplace
"&Заменить"
"&Replace"
"&Nahradit"
"E&rsetzen"
"&Csere"

MEditReplaceAll
"&Все"
"&All"
"&Vše"
"&Alle"
"&Mindet"

MEditSkip
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&springen"
"&Kihagy"

MEditCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"Mé&gsem"

MEditOpenCreateLabel
"&Открыть/создать файл:"
"&Open/create file:"
"Otevřít/vytvořit soubor:"
"Öffnen/datei erstellen:"
"Fájl megnyitása/&létrehozása:"

MEditOpenAutoDetect
"&Автоматическое определение"
"&Automatic detection"
upd:"Automatic detection"
upd:"Automatic detection"
"&Automatikus érzékelés"

MEditGoToLine
l:
"Перейти"
"Go to position"
"Jít na pozici"
"Gehe zu Zeile"
"Sorra ugrás"

MFolderShortcutsTitle
l:
"Ссылки на папки"
"Folder shortcuts"
"Adresářové zkratky"
"Ordnerschnellzugriff"
"Mappa gyorsbillentyűk"

MFolderShortcutBottom
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Bearb.: Entf,Einf,F4"
"Szerk.: Del,Ins,F4"

MShortcutNone
"<отсутствует>"
"<none>"
"<není>"
"<keiner>"
"<nincs>"

MShortcutPlugin
"<плагин>"
"<plugin>"
"<plugin>"
"<Plugin>"
"<plugin>"

MEnterShortcut
"Введите новую ссылку:"
"Enter new shortcut:"
"Zadejte novou zkratku:"
"Neue Verknüpfung:"
"Az új gyorsbillentyű:"

MNeedNearPath
"Перейти в ближайшую доступную папку?"
"Jump to the nearest existing folder?"
"Skočit na nejbližší existující adresář?"
"Zum nahesten existierenden Ordner springen?"
"Ugrás a legközelebbi létező mappára?"

MSaveThisShortcut
"Запомнить эту ссылку?"
"Save this shortcuts?"
"Uložit tyto zkratky?"
"Verknüpfung speichern?"
"Mentsem a gyorsbillentyűket?"

MEditF1
l:
l://functional keys - 6 characters max, 12 keys, "OEM" is F8 dupe!
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Súgó"

MEditF2
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"

MEditF3
""
""
""
""
""

MEditF4
""
""
""
""
""

MEditF5
""
""
""
""
""

MEditF6
"Просм"
"View"
"Zobraz"
"Betr."
"Megnéz"

MEditF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"

MEditF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MEditF9
""
""
""
""
""

MEditF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"

MEditF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MEditF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"

MEditF8DOS
le:// don't count this - it's a F8 another text
"OEM"
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
""

MEditShiftF2
"Сохр.в"
"SaveAs"
"UlJako"
"SpeiUn"
"Ment.."

MEditShiftF3
""
""
""
""
""

MEditShiftF4
"Редак."
"Edit.."
"Edit.."
"Bear.."
"Szrk.."

MEditShiftF5
""
""
""
""
""

MEditShiftF6
""
""
""
""
""

MEditShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"

MEditShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"

MEditShiftF9
""
""
""
""
""

MEditShiftF10
"СхрВых"
"SaveQ"
"UlKone"
"SaveQ"
"MentKi"

MEditShiftF11
""
""
""
""
""

MEditShiftF12
""
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
""

MEditAltF2
""
""
""
""
""

MEditAltF3
""
""
""
""
""

MEditAltF4
""
""
""
""
""

MEditAltF5
"Печать"
"Print"
"Tisk"
"Druck"
"Nyomt"

MEditAltF6
""
""
""
""
""

MEditAltF7
"Назад"
"Prev"
"Předch"
"Letzt"
"VisKer"

MEditAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"

MEditAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"

MEditAltF10
""
""
""
""
""

MEditAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"

MEditAltF12
""
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
""

MEditCtrlF2
""
""
""
""
""

MEditCtrlF3
""
""
""
""
""

MEditCtrlF4
""
""
""
""
""

MEditCtrlF5
""
""
""
""
""

MEditCtrlF6
""
""
""
""
""

MEditCtrlF7
"Замена"
"Replac"
"Nahraď"
"Ersetz"
"Csere"

MEditCtrlF8
""
""
""
""
""

MEditCtrlF9
""
""
""
""
""

MEditCtrlF10
"Позиц"
"GoFile"
"JdiSou"
"GehDat"
"FájlPz"

MEditCtrlF11
""
""
""
""
""

MEditCtrlF12
""
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
""

MEditAltShiftF2
""
""
""
""
""

MEditAltShiftF3
""
""
""
""
""

MEditAltShiftF4
""
""
""
""
""

MEditAltShiftF5
""
""
""
""
""

MEditAltShiftF6
""
""
""
""
""

MEditAltShiftF7
""
""
""
""
""

MEditAltShiftF8
""
""
""
""
""

MEditAltShiftF9
"Конфиг"
"Config"
"Nastav"
"Konfig"
"Beáll."

MEditAltShiftF10
""
""
""
""
""

MEditAltShiftF11
""
""
""
""
""

MEditAltShiftF12
""
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
""

MEditCtrlShiftF2
""
""
""
""
""

MEditCtrlShiftF3
""
""
""
""
""

MEditCtrlShiftF4
""
""
""
""
""

MEditCtrlShiftF5
""
""
""
""
""

MEditCtrlShiftF6
""
""
""
""
""

MEditCtrlShiftF7
""
""
""
""
""

MEditCtrlShiftF8
""
""
""
""
""

MEditCtrlShiftF9
""
""
""
""
""

MEditCtrlShiftF10
""
""
""
""
""

MEditCtrlShiftF11
""
""
""
""
""

MEditCtrlShiftF12
""
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
""

MEditCtrlAltF2
""
""
""
""
""

MEditCtrlAltF3
""
""
""
""
""

MEditCtrlAltF4
""
""
""
""
""

MEditCtrlAltF5
""
""
""
""
""

MEditCtrlAltF6
""
""
""
""
""

MEditCtrlAltF7
""
""
""
""
""

MEditCtrlAltF8
""
""
""
""
""

MEditCtrlAltF9
""
""
""
""
""

MEditCtrlAltF10
""
""
""
""
""

MEditCtrlAltF11
""
""
""
""
""

MEditCtrlAltF12
""
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
""

MEditCtrlAltShiftF2
""
""
""
""
""

MEditCtrlAltShiftF3
""
""
""
""
""

MEditCtrlAltShiftF4
""
""
""
""
""

MEditCtrlAltShiftF5
""
""
""
""
""

MEditCtrlAltShiftF6
""
""
""
""
""

MEditCtrlAltShiftF7
""
""
""
""
""

MEditCtrlAltShiftF8
""
""
""
""
""

MEditCtrlAltShiftF9
""
""
""
""
""

MEditCtrlAltShiftF10
""
""
""
""
""

MEditCtrlAltShiftF11
""
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
""

MSingleEditF1
l:
l://Single Editor: functional keys - 6 characters max, 12 keys, "OEM" is F8 dupe!
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Súgó"

MSingleEditF2
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"

MSingleEditF3
""
""
""
""
""

MSingleEditF4
""
""
""
""
""

MSingleEditF5
""
""
""
""
""

MSingleEditF6
"Просм"
"View"
"Zobraz"
"Betr."
"Megnéz"

MSingleEditF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"

MSingleEditF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MSingleEditF9
""
""
""
""
""

MSingleEditF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"

MSingleEditF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MSingleEditF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"

MSingleEditF8DOS
le:// don't count this - it's a F8 another text
"OEM"
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
""

MSingleEditShiftF2
"Сохр.в"
"SaveAs"
"UlJako"
"SpeiUn"
"Ment.."

MSingleEditShiftF3
""
""
""
""
""

MSingleEditShiftF4
""
""
""
""
""

MSingleEditShiftF5
""
""
""
""
""

MSingleEditShiftF6
""
""
""
""
""

MSingleEditShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"

MSingleEditShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"

MSingleEditShiftF9
""
""
""
""
""

MSingleEditShiftF10
"СхрВых"
"SaveQ"
"UlKone"
"SaveQ"
"MentKi"

MSingleEditShiftF11
""
""
""
""
""

MSingleEditShiftF12
""
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
""

MSingleEditAltF2
""
""
""
""
""

MSingleEditAltF3
""
""
""
""
""

MSingleEditAltF4
""
""
""
""
""

MSingleEditAltF5
"Печать"
"Print"
"Tisk"
"Druck"
"Nyomt"

MSingleEditAltF6
""
""
""
""
""

MSingleEditAltF7
""
""
""
""
""

MSingleEditAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"

MSingleEditAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"

MSingleEditAltF10
""
""
""
""
""

MSingleEditAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"

MSingleEditAltF12
""
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
""

MSingleEditCtrlF2
""
""
""
""
""

MSingleEditCtrlF3
""
""
""
""
""

MSingleEditCtrlF4
""
""
""
""
""

MSingleEditCtrlF5
""
""
""
""
""

MSingleEditCtrlF6
""
""
""
""
""

MSingleEditCtrlF7
"Замена"
"Replac"
"Nahraď"
"Ersetz"
"Csere"

MSingleEditCtrlF8
""
""
""
""
""

MSingleEditCtrlF9
""
""
""
""
""

MSingleEditCtrlF10
""
""
""
""
""

MSingleEditCtrlF11
""
""
""
""
""

MSingleEditCtrlF12
""
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
""

MSingleEditAltShiftF2
""
""
""
""
""

MSingleEditAltShiftF3
""
""
""
""
""

MSingleEditAltShiftF4
""
""
""
""
""

MSingleEditAltShiftF5
""
""
""
""
""

MSingleEditAltShiftF6
""
""
""
""
""

MSingleEditAltShiftF7
""
""
""
""
""

MSingleEditAltShiftF8
""
""
""
""
""

MSingleEditAltShiftF9
"Конфиг"
"Config"
"Nastav"
"Konfig"
"Beáll."

MSingleEditAltShiftF10
""
""
""
""
""

MSingleEditAltShiftF11
""
""
""
""
""

MSingleEditAltShiftF12
""
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
""

MSingleEditCtrlShiftF2
""
""
""
""
""

MSingleEditCtrlShiftF3
""
""
""
""
""

MSingleEditCtrlShiftF4
""
""
""
""
""

MSingleEditCtrlShiftF5
""
""
""
""
""

MSingleEditCtrlShiftF6
""
""
""
""
""

MSingleEditCtrlShiftF7
""
""
""
""
""

MSingleEditCtrlShiftF8
""
""
""
""
""

MSingleEditCtrlShiftF9
""
""
""
""
""

MSingleEditCtrlShiftF10
""
""
""
""
""

MSingleEditCtrlShiftF11
""
""
""
""
""

MSingleEditCtrlShiftF12
""
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
""

MSingleEditCtrlAltF2
""
""
""
""
""

MSingleEditCtrlAltF3
""
""
""
""
""

MSingleEditCtrlAltF4
""
""
""
""
""

MSingleEditCtrlAltF5
""
""
""
""
""

MSingleEditCtrlAltF6
""
""
""
""
""

MSingleEditCtrlAltF7
""
""
""
""
""

MSingleEditCtrlAltF8
""
""
""
""
""

MSingleEditCtrlAltF9
""
""
""
""
""

MSingleEditCtrlAltF10
""
""
""
""
""

MSingleEditCtrlAltF11
""
""
""
""
""

MSingleEditCtrlAltF12
""
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
""

MSingleEditCtrlAltShiftF2
""
""
""
""
""

MSingleEditCtrlAltShiftF3
""
""
""
""
""

MSingleEditCtrlAltShiftF4
""
""
""
""
""

MSingleEditCtrlAltShiftF5
""
""
""
""
""

MSingleEditCtrlAltShiftF6
""
""
""
""
""

MSingleEditCtrlAltShiftF7
""
""
""
""
""

MSingleEditCtrlAltShiftF8
""
""
""
""
""

MSingleEditCtrlAltShiftF9
""
""
""
""
""

MSingleEditCtrlAltShiftF10
""
""
""
""
""

MSingleEditCtrlAltShiftF11
""
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
""

MEditSaveAs
l:
"Сохранить &файл как"
"Save file &as"
"Uložit soubor jako"
"Speichern &als"
"Fá&jl mentése, mint:"

MEditCodePage
"&Кодовая страница:"
"&Code page:"
"Kódová stránka:"
"Codepage:"
"Kódlap:"

MEditAddSignature
"Добавить &сигнатуру (BOM)"
"Add &signature (BOM)"
"Přidat signaturu (BOM)"
"Sinatur hinzu (BOM)"
"Uni&code bájtsorrend jelzővel (BOM)"

MEditSaveAsFormatTitle
"Изменить перевод строки:"
"Change line breaks to:"
"Změnit zakončení řádků na:"
"Zeilenumbrüche setzen:"
"Sortörés konverzió:"

MEditSaveOriginal
"&исходный формат"
"Do n&ot change"
"&Beze změny"
"Nicht verä&ndern"
"Nincs &konverzió"

MEditSaveDOS
"в форма&те DOS/Windows (CR LF)"
"&Dos/Windows format (CR LF)"
"&Dos/Windows formát (CR LF)"
"&Dos/Windows Format (CR LF)"
"&DOS/Windows formátum (CR LF)"

MEditSaveUnix
"в формат&е UNIX (LF)"
"&Unix format (LF)"
"&Unix formát (LF)"
"&Unix Format (LF)"
"&UNIX formátum (LF)"

MEditSaveMac
"в фор&мате MAC (CR)"
"&Mac format (CR)"
"&Mac formát (CR)"
"&Mac Format (CR)"
"&Mac formátum (CR)"

MEditCannotSave
"Ошибка сохранения файла"
"Cannot save the file"
"Nelze uložit soubor"
"Kann die Datei nicht speichern"
"A fájl nem menthető"

MEditSavedChangedNonFile
"Файл изменен, но файл или папка, в которой он находился,"
"The file is changed but the file or the folder containing"
"Soubor je změněn, ale soubor, nebo adresář obsahující"
"Inhalt dieser Datei wurde verändert aber die Datei oder der Ordner, welche"
"A fájl megváltozott, de a fájlt vagy a mappáját"

MEditSavedChangedNonFile1
"Файл или папка, в которой он находился,"
"The file or the folder containing"
"Soubor nebo adresář obsahující"
"Die Datei oder der Ordner, welche"
"A fájlt vagy a mappáját"

MEditSavedChangedNonFile2
"был перемещен или удален."
"this file was moved or deleted."
"tento soubor byl přesunut, nebo smazán."
"diesen Inhalt enthält wurde verschoben oder gelöscht."
"időközben áthelyezte/átnevezte vagy törölte."

MEditNewPath1
"Путь к редактируемому файлу не существует,"
"The path to the edited file does not exist,"
"Cesta k editovanému souboru neexistuje,"
"Der Pfad zur bearbeiteten Datei existiert nicht,"
"A szerkesztendő fájl célmappája még"

MEditNewPath2
"но будет создан при сохранении файла."
"but will be created when the file is saved."
"ale bude vytvořena při uložení souboru."
"aber wird erstellt sobald die Datei gespeichert wird."
"nem létezik, de mentéskor létrejön."

MEditNewPath3
"Продолжать?"
"Continue?"
"Pokračovat?"
"Fortsetzen?"
"Folytatja?"

MEditNewPlugin1
"Имя редактируемого файла не может быть пустым"
"The name of the file to edit cannot be empty"
"Název souboru k editaci nesmí být prázdné"
"Der Name der zu editierenden Datei kann nicht leer sein"
"A szerkesztendő fájlnak nevet kell adni"

MEditDataLostWarn1
"Файл содержит символы, которые невозможно"
"This file contains characters, which cannot be"
"Tento soubor obsahuje znaky, které nemohou být"
"Die Datei enthält Zeichen, welche mit der aktuellen Codepage"
"A fájl olyan karaktereket tartalmaz, amelyek a"

MEditDataLostWarn2
"корректно преобразовать в выбранную кодировку."
"correctly translated using the selected codepage."
"korektně přeloženy do zvoleného kódování."
"nicht korrekt angezeigt werden können."
"kiválasztott kódlappal nem értelmezhetők helyesen."

MEditDataLostWarn3
"Продолжить?"
"Continue?"
"Pokračovat?"
"Fortsetzen?"
"Folytatja?"

MEditDataLostWarn4
"Сохранять файл не рекомендуется."
"It is not recommended to save this file."
"Není doporučeno uložit tento soubor."
"Es wird empfohlen, die Datei nicht zu speichern."
"A fájl mentése nem ajánlott."

MColumnName
l:
"Имя"
"Name"
"Název"
"Name"
"Név"

MColumnSize
"Размер"
"Size"
"Velikost"
"Größe"
"Méret"

MColumnPacked
"Упаков"
"Packed"
"Komprimovaný"
"Kompr."
"TMéret"

MColumnDate
"Дата"
"Date"
"Datum"
"Datum"
"Dátum"

MColumnTime
"Время"
"Time"
"Čas"
"Zeit"
"Idő"

MColumnModified
"Модификация"
"Modified"
"Modifikován"
"Verändert"
"Módosítva"

MColumnCreated
"Создание"
"Created"
"Vytvořen"
"Erstellt"
"Létrejött"

MColumnAccessed
"Доступ"
"Accessed"
"Přístup"
"Zugriff"
"Hozzáférés"

MColumnAttr
"Атриб"
"Attr"
"Attr"
"Attr"
"Attrib"

MColumnDescription
"Описание"
"Description"
"Popis"
"Beschreibung"
"Megjegyzés"

MColumnOwner
"Владелец"
"Owner"
"Vlastník"
"Besitzer"
"Tulajdonos"

MColumnMumLinks
"КлС"
"NmL"
"PočLn"
"AnL"
"Lnk"

MListUp
l:
"Вверх"
"  Up  "
"Nahoru"
" Hoch "
"  Fel  "

MListFolder
"Папка"
"Folder"
"Adresář"
"Ordner"
" Mappa "

MListSymLink
"Ссылка"
"Symlink"
"Link"
"Symlink"
"SzimLnk"

MListJunction
"Связь"
"Junction"
"Křížení"
"Knoten"
"Csomópt"

MListBytes
"Б"
"B"
"B"
"B"
"B"

MListKb
"К"
"K"
"K"
"K"
"K"

MListMb
"М"
"M"
"M"
"M"
"M"

MListGb
"Г"
"G"
"G"
"G"
"G"

MListTb
"Т"
"T"
"T"
"T"
"T"

MListFileSize
" %s байт в 1 файле "
" %s bytes in 1 file "
" %s bytů v 1 souboru "
" %s Bytes in 1 Datei "
" %s bájt 1 fájlban "

MListFilesSize1
" %s байт в %d файле "
" %s bytes in %d files "
" %s bytů v %d souborech "
" %s Bytes in %d Dateien "
" %s bájt %d fájlban "

MListFilesSize2
" %s байт в %d файлах "
" %s bytes in %d files "
" %s bytů v %d souborech "
" %s Bytes in %d Dateien "
" %s bájt %d fájlban "

MListFreeSize
" %s байт свободно "
" %s free bytes "
" %s volných bytů "
" %s freie Bytes "
" %s szabad bájt "

MDirInfoViewTitle
l:
"Просмотр"
"View"
"Zobraz"
"Betrachten"
"Vizsgálat"

MFileToEdit
"Редактировать файл:"
"File to edit:"
"Soubor k editaci:"
"Datei bearbeiten:"
"Szerkesztendő fájl:"

MUnselectTitle
l:
"Снять"
"Deselect"
"Odznačit"
"Abwählen"
"Kijelölést levesz"

MSelectTitle
"Пометить"
"Select"
"Označit"
"Auswählen"
"Kijelölés"

MSelectFilter
"&Фильтр"
"&Filter"
"&Filtr"
"&Filter"
"&Szűrő"

MCompareTitle
l:
"Сравнение"
"Compare"
"Porovnat"
"Vergleichen"
"Összehasonlítás"

MCompareFilePanelsRequired1
"Для сравнения папок требуются"
"Two file panels are required to perform"
"Pro provedení příkazu Porovnat adresáře"
"Zwei Dateipanels werden benötigt um"
"Mappák összehasonlításához"

MCompareFilePanelsRequired2
"две файловые панели"
"the Compare folders command"
"jsou nutné dva souborové panely"
"den Vergleich auszuführen."
"két fájlpanel szükséges"

MCompareSameFolders1
"Содержимое папок,"
"The folders contents seems"
"Obsahy adresářů jsou"
"Der Inhalt der beiden Ordner scheint"
"A mappák tartalma"

MCompareSameFolders2
"скорее всего, одинаково"
"to be identical"
"identické"
"identisch zu sein."
"azonosnak tűnik"

MSelectAssocTitle
l:
"Выберите ассоциацию"
"Select association"
"Vyber závislosti"
"Dateiverknüpfung auswählen"
"Válasszon társítást"

MAssocTitle
l:
"Ассоциации для файлов"
"File associations"
"Závislosti souborů"
"Dateiverknüpfungen"
"Fájltársítások"

MAssocBottom
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Bearb.: Entf,Einf,F4"
"Szerk.: Del,Ins,F4"

MAskDelAssoc
"Вы хотите удалить ассоциацию для"
"Do you wish to delete association for"
"Přejete si smazat závislost pro"
"Wollen Sie die Verknüpfung löschen für"
"Törölni szeretné a társítást?"

MAssocNeedMask
"Пожалуйста, укажите маску файлов"
"Please specify a file mask"
"Prosím zadejte masku souborů"
"Bitte geben Sie eine Dateimaske an"
"Kérem, adjon meg egy fájlmaszkot!"

MFileAssocTitle
l:
"Редактирование ассоциаций файлов"
"Edit file associations"
"Upravit závislosti souborů"
"Dateiverknüpfungen bearbeiten"
"Fájltársítások szerkesztése"

MFileAssocMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"
"&Maska nebo masky souborů:"
"Datei&maske (mehrere getrennt mit Komma):"
"F&ájlmaszk(ok, vesszővel elválasztva):"

MFileAssocDescr
"&Описание ассоциации:"
"&Description of the association:"
"&Popis asociací:"
"&Beschreibung der Verknüpfung:"
"A &társítás leírása:"

MFileAssocExec
"Команда, &выполняемая по Enter:"
"E&xecute command (used for Enter):"
"&Vykonat příkaz (použito pro Enter):"
"Befehl &ausführen (mit Enter):"
"&Végrehajtandó parancs (Enterre):"

MFileAssocAltExec
"Коман&да, выполняемая по Ctrl-PgDn:"
"Exec&ute command (used for Ctrl-PgDn):"
"V&ykonat příkaz (použito pro Ctrl-PgDn):"
"Befehl a&usführen (mit Strg-PgDn):"
"Vé&grehajtandó parancs (Ctrl-PgDown-ra):"

MFileAssocView
"Команда &просмотра, выполняемая по F3:"
"&View command (used for F3):"
"Příkaz &Zobraz (použito pro F3):"
"Be&trachten (mit F3):"
"&Nézőke parancs (F3-ra):"

MFileAssocAltView
"Команда просмотра, в&ыполняемая по Alt-F3:"
"V&iew command (used for Alt-F3):"
"Příkaz Z&obraz (použito pro Alt-F3):"
"Bet&rachten (mit Alt-F3):"
"N&ézőke parancs (Alt-F3-ra):"

MFileAssocEdit
"Команда &редактирования, выполняемая по F4:"
"&Edit command (used for F4):"
"Příkaz &Edituj (použito pro F4):"
"Bearb&eiten (mit F4):"
"S&zerkesztés parancs (F4-re):"

MFileAssocAltEdit
"Команда редактировани&я, выполняемая по Alt-F4:"
"Edit comm&and (used for Alt-F4):"
"Příkaz Editu&j (použito pro Alt-F4):"
"Bearbe&iten (mit Alt-F4):"
"Sze&rkesztés parancs (Alt-F4-re):"

MViewF1
l:
l://Viewer: functional keys, 12 keys, except F2 - 2 keys, and F8 - 2 keys
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Súgó"

MViewF2
le:// this is another text for F2
"Сверн"
"Wrap"
"Zalom"
"Umbr."
"SorTör"

MViewF3
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"

MViewF4
"Код"
"Hex"
"Hex"
"Hex"
"Hexa"

MViewF5
""
""
""
""
""

MViewF6
"Редакт"
"Edit"
"Edit"
"Bearb"
"Szerk."

MViewF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"

MViewF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MViewF9
""
""
""
""
""

MViewF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"

MViewF11
"Модули"
"Plugins"
"Plugin"
"Plugin"
"Plugin"

MViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"

MViewF2Unwrap
"Развер"
"Unwrap"
"Nezal"
"KeinUm"
"NemTör"

MViewF4Text
l:// this is another text for F4
"Текст"
"Text"
"Text"
"Text"
"Szöveg"

MViewF8DOS
"OEM"
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
""

MViewShiftF2
"Слова"
"WWrap"
"ZalSlo"
"WUmbr"
"SzóTör"

MViewShiftF3
""
""
""
""
""

MViewShiftF4
""
""
""
""
""

MViewShiftF5
""
""
""
""
""

MViewShiftF6
""
""
""
""
""

MViewShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"

MViewShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"

MViewShiftF9
""
""
""
""
""

MViewShiftF10
""
""
""
""
""

MViewShiftF11
""
""
""
""
""

MViewShiftF12
""
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
""

MViewAltF2
""
""
""
""
""

MViewAltF3
""
""
""
""
""

MViewAltF4
""
""
""
""
""

MViewAltF5
"Печать"
"Print"
"Tisk"
"Druck"
"Nyomt"

MViewAltF6
""
""
""
""
""

MViewAltF7
"Назад"
"Prev"
"Předch"
"Letzt"
"VisKer"

MViewAltF8
"Перейт"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"

MViewAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"

MViewAltF10
""
""
""
""
""

MViewAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"

MViewAltF12
""
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
""

MViewCtrlF2
""
""
""
""
""

MViewCtrlF3
""
""
""
""
""

MViewCtrlF4
""
""
""
""
""

MViewCtrlF5
""
""
""
""
""

MViewCtrlF6
""
""
""
""
""

MViewCtrlF7
""
""
""
""
""

MViewCtrlF8
""
""
""
""
""

MViewCtrlF9
""
""
""
""
""

MViewCtrlF10
"Позиц"
"GoFile"
"JítSou"
"GehDat"
"FájlPz"

MViewCtrlF11
""
""
""
""
""

MViewCtrlF12
""
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
""

MViewAltShiftF2
""
""
""
""
""

MViewAltShiftF3
""
""
""
""
""

MViewAltShiftF4
""
""
""
""
""

MViewAltShiftF5
""
""
""
""
""

MViewAltShiftF6
""
""
""
""
""

MViewAltShiftF7
""
""
""
""
""

MViewAltShiftF8
""
""
""
""
""

MViewAltShiftF9
"Конфиг"
"Config"
"Nastav"
"Konfig"
"Beáll."

MViewAltShiftF10
""
""
""
""
""

MViewAltShiftF11
""
""
""
""
""

MViewAltShiftF12
""
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
""

MViewCtrlShiftF2
""
""
""
""
""

MViewCtrlShiftF3
""
""
""
""
""

MViewCtrlShiftF4
""
""
""
""
""

MViewCtrlShiftF5
""
""
""
""
""

MViewCtrlShiftF6
""
""
""
""
""

MViewCtrlShiftF7
""
""
""
""
""

MViewCtrlShiftF8
""
""
""
""
""

MViewCtrlShiftF9
""
""
""
""
""

MViewCtrlShiftF10
""
""
""
""
""

MViewCtrlShiftF11
""
""
""
""
""

MViewCtrlShiftF12
""
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
""

MViewCtrlAltF2
""
""
""
""
""

MViewCtrlAltF3
""
""
""
""
""

MViewCtrlAltF4
""
""
""
""
""

MViewCtrlAltF5
""
""
""
""
""

MViewCtrlAltF6
""
""
""
""
""

MViewCtrlAltF7
""
""
""
""
""

MViewCtrlAltF8
""
""
""
""
""

MViewCtrlAltF9
""
""
""
""
""

MViewCtrlAltF10
""
""
""
""
""

MViewCtrlAltF11
""
""
""
""
""

MViewCtrlAltF12
""
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
""

MViewCtrlAltShiftF2
""
""
""
""
""

MViewCtrlAltShiftF3
""
""
""
""
""

MViewCtrlAltShiftF4
""
""
""
""
""

MViewCtrlAltShiftF5
""
""
""
""
""

MViewCtrlAltShiftF6
""
""
""
""
""

MViewCtrlAltShiftF7
""
""
""
""
""

MViewCtrlAltShiftF8
""
""
""
""
""

MViewCtrlAltShiftF9
""
""
""
""
""

MViewCtrlAltShiftF10
""
""
""
""
""

MViewCtrlAltShiftF11
""
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
""

MSingleViewF1
l:
l://Single Viewer: functional keys, 12 keys, except F2 - 2 keys, and F8 - 2 keys
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Súgó"

MSingleViewF2
"Сверн"
"Wrap"
"Zalom"
"Umbr."
"SorTör"

MSingleViewF3
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"

MSingleViewF4
"Код"
"Hex"
"Hex"
"Hex"
"Hexa"

MSingleViewF5
""
""
""
""
""

MSingleViewF6
"Редакт"
"Edit"
"Edit"
"Bearb"
"Szerk."

MSingleViewF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"

MSingleViewF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MSingleViewF9
""
""
""
""
""

MSingleViewF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"

MSingleViewF11
"Модули"
"Plugins"
"Plugin"
"Plugins"
"Plugin"

MSingleViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"

MSingleViewF2Unwrap
l:// this is another text for F2
"Развер"
"Unwrap"
"Nezal"
"KeinUm"
"NemTör"

MSingleViewF4Text
l:// this is another text for F4
"Текст"
"Text"
"Text"
"Text"
"Szöveg"

MSingleViewF8DOS
"OEM"
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
""

MSingleViewShiftF2
"Слова"
"WWrap"
"ZalSlo"
"WUmbr"
"SzóTör"

MSingleViewShiftF3
""
""
""
""
""

MSingleViewShiftF4
""
""
""
""
""

MSingleViewShiftF5
""
""
""
""
""

MSingleViewShiftF6
""
""
""
""
""

MSingleViewShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"

MSingleViewShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"

MSingleViewShiftF9
""
""
""
""
""

MSingleViewShiftF10
""
""
""
""
""

MSingleViewShiftF11
""
""
""
""
""

MSingleViewShiftF12
""
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
""

MSingleViewAltF2
""
""
""
""
""

MSingleViewAltF3
""
""
""
""
""

MSingleViewAltF4
""
""
""
""
""

MSingleViewAltF5
"Печать"
"Print"
"Tisk"
"Druck"
"Nyomt"

MSingleViewAltF6
""
""
""
""
""

MSingleViewAltF7
"Назад"
"Prev"
"Předch"
"Letzt"
"VisKer"

MSingleViewAltF8
"Перейт"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"

MSingleViewAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"

MSingleViewAltF10
""
""
""
""
""

MSingleViewAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"

MSingleViewAltF12
""
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
""

MSingleViewCtrlF2
""
""
""
""
""

MSingleViewCtrlF3
""
""
""
""
""

MSingleViewCtrlF4
""
""
""
""
""

MSingleViewCtrlF5
""
""
""
""
""

MSingleViewCtrlF6
""
""
""
""
""

MSingleViewCtrlF7
""
""
""
""
""

MSingleViewCtrlF8
""
""
""
""
""

MSingleViewCtrlF9
""
""
""
""
""

MSingleViewCtrlF10
""
""
""
""
""

MSingleViewCtrlF11
""
""
""
""
""

MSingleViewCtrlF12
""
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
""

MSingleViewAltShiftF2
""
""
""
""
""

MSingleViewAltShiftF3
""
""
""
""
""

MSingleViewAltShiftF4
""
""
""
""
""

MSingleViewAltShiftF5
""
""
""
""
""

MSingleViewAltShiftF6
""
""
""
""
""

MSingleViewAltShiftF7
""
""
""
""
""

MSingleViewAltShiftF8
""
""
""
""
""

MSingleViewAltShiftF9
"Конфиг"
"Config"
"Nastav"
"Konfig"
"Beáll."

MSingleViewAltShiftF10
""
""
""
""
""

MSingleViewAltShiftF11
""
""
""
""
""

MSingleViewAltShiftF12
""
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
""

MSingleViewCtrlShiftF2
""
""
""
""
""

MSingleViewCtrlShiftF3
""
""
""
""
""

MSingleViewCtrlShiftF4
""
""
""
""
""

MSingleViewCtrlShiftF5
""
""
""
""
""

MSingleViewCtrlShiftF6
""
""
""
""
""

MSingleViewCtrlShiftF7
""
""
""
""
""

MSingleViewCtrlShiftF8
""
""
""
""
""

MSingleViewCtrlShiftF9
""
""
""
""
""

MSingleViewCtrlShiftF10
""
""
""
""
""

MSingleViewCtrlShiftF11
""
""
""
""
""

MSingleViewCtrlShiftF12
""
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
""

MSingleViewCtrlAltF2
""
""
""
""
""

MSingleViewCtrlAltF3
""
""
""
""
""

MSingleViewCtrlAltF4
""
""
""
""
""

MSingleViewCtrlAltF5
""
""
""
""
""

MSingleViewCtrlAltF6
""
""
""
""
""

MSingleViewCtrlAltF7
""
""
""
""
""

MSingleViewCtrlAltF8
""
""
""
""
""

MSingleViewCtrlAltF9
""
""
""
""
""

MSingleViewCtrlAltF10
""
""
""
""
""

MSingleViewCtrlAltF11
""
""
""
""
""

MSingleViewCtrlAltF12
""
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
""

MSingleViewCtrlAltShiftF2
""
""
""
""
""

MSingleViewCtrlAltShiftF3
""
""
""
""
""

MSingleViewCtrlAltShiftF4
""
""
""
""
""

MSingleViewCtrlAltShiftF5
""
""
""
""
""

MSingleViewCtrlAltShiftF6
""
""
""
""
""

MSingleViewCtrlAltShiftF7
""
""
""
""
""

MSingleViewCtrlAltShiftF8
""
""
""
""
""

MSingleViewCtrlAltShiftF9
""
""
""
""
""

MSingleViewCtrlAltShiftF10
""
""
""
""
""

MSingleViewCtrlAltShiftF11
""
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
""

MInViewer
"просмотр %s"
"view %s"
"prohlížení %s"
"Betrachte %s"
"%s megnézése"

MInEditor
"редактирование %s"
"edit %s"
"editace %s"
"Bearbeite %s"
"%s szerkesztése"

MFilterTitle
l:
"Меню фильтров"
"Filters menu"
"Menu filtrů"
"Filtermenü"
"Szűrők menü"

MFilterBottom
"+,-,Пробел,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Space,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Mezera,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Leer,I,X,BS,UmschBS,Einf,Entf,F4,F5,StrgUp,StrgDn"
"+,-,Szóköz,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Fel,Ctrl-Le"

MPanelFileType
"Файлы панели"
"Panel file type"
"Typ panelu souborů"
"Dateityp in Panel"
"A panel fájltípusa"

MFolderFileType
"Папки"
"Folders"
"Adresáře"
"Ordner"
"Mappák"

MCanEditCustomFilterOnly
"Только пользовательский фильтр можно редактировать"
"Only custom filter can be edited"
"Jedině vlastní filtr může být upraven"
"Nur eigene Filter können editiert werden."
"Csak saját szűrő szerkeszthető"

MAskDeleteFilter
"Вы хотите удалить фильтр"
"Do you wish to delete the filter"
"Přejete si smazat filtr"
"Wollen Sie den eigenen Filter löschen"
"Törölni szeretné a szűrőt?"

MCanDeleteCustomFilterOnly
"Только пользовательский фильтр может быть удален"
"Only custom filter can be deleted"
"Jedině vlastní filtr může být smazán"
"Nur eigene Filter können gelöscht werden."
"Csak saját szűrő törölhető"

MFindFileTitle
l:
"Поиск файла"
"Find file"
"Hledat soubor"
"Nach Dateien suchen"
"Fájlkeresés"

MFindFileResultTitle
"Поиск файла - результат"
"Find file - result"
"Hledat soubor - výsledek"
"Suchergebnisse"
"Fájlkeresés eredménye"

MFindFileMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"
"Maska nebo masky souborů:"
"Datei&maske (mehrere getrennt mit Komma):"
"Fájlm&aszk(ok, vesszővel elválasztva):"

MFindFileText
"&Содержащих текст:"
"Con&taining text:"
"Obsahující te&xt:"
"Enthält &Text:"
"&Tartalmazza a szöveget:"

MFindFileHex
"&Содержащих 16-ричный код:"
"Con&taining hex:"
"Obsahující &hex:"
"En&thält Hex (xx xx ...):"
"Tartalmazza a he&xát:"

MFindFileCodePage
"Используя кодо&вую страницу:"
"Using code pa&ge:"
upd:"Použít &znakovou sadu:"
upd:"Zeichenta&belle verwenden:"
"Kó&dlap:"

MFindFileCase
"&Учитывать регистр"
"&Case sensitive"
"Roz&lišovat velikost písmen"
"Gr&oß-/Kleinschreibung"
"&Nagy/kisbetű érzékeny"

MFindFileWholeWords
"Только &целые слова"
"&Whole words"
"&Celá slova"
"Nur &ganze Wörter"
"Egés&z szavakra"

MFindFileAllCodePages
"Все кодовые страницы"
"All code pages"
upd:"Všechny znakové sady"
upd:"Alle Zeichentabellen"
"Minden kódlappal"

MFindArchives
"Искать в а&рхивах"
"Search in arch&ives"
"Hledat v a&rchívech"
"In Arch&iven suchen"
"Keresés t&ömörítettekben"

MFindFolders
"Искать п&апки"
"Search for f&olders"
"Hledat a&dresáře"
"Nach &Ordnern suchen"
"Keresés mapp&ákra"

MFindSymLinks
"Искать в символи&ческих ссылках"
"Search in symbolic lin&ks"
"Hledat v s&ymbolických lincích"
"In symbolischen Lin&ks suchen"
"Keresés sz&imbolikus linkekben"

MSearchForHex
"Искать 16-ричн&ый код"
"Search for &hex"
"Hledat &hex"
"Nach &Hex suchen"
"Keresés &hexákra"

MSearchWhere
"Выберите &область поиска:"
"Select search &area:"
upd:"Zvolte oblast hledání:"
upd:"Suchbereich:"
upd:"Keresés hatósugara:"

MSearchAllDisks
"На всех несъемных &дисках"
"In &all non-removable drives"
"Ve všech p&evných discích"
"Auf &allen festen Datenträger"
"Minden &fix meghajtón"

MSearchAllButNetwork
"На всех &локальных дисках"
"In all &local drives"
"Ve všech &lokálních discích"
"Auf allen &lokalen Datenträgern"
"Minden hel&yi meghajtón"

MSearchInPATH
"В PATH-катало&гах"
"In &PATH folders"
"V adresářích z &PATH"
"In &PATH-Ordnern"
"A &PATH mappáiban"

MSearchFromRootOfDrive
"С кор&ня диска"
"From the &root of"
"V &kořeni"
"Ab Wu&rzelverz. von"
"Megh. &gyökerétől:"

MSearchFromRootFolder
"С кор&невой папки"
"From the &root folder"
"V kořeno&vém adresáři"
"Ab Wu&rzelverzeichnis"
"A &gyökérmappától"

MSearchFromCurrent
"С &текущей папки"
"From the curre&nt folder"
"V tomto adresář&i"
"Ab dem aktuelle&n Ordner"
"Az akt&uális mappától"

MSearchInCurrent
"Только в теку&щей папке"
"The current folder onl&y"
"P&ouze v tomto adresáři"
"Nur im aktue&llen Ordner"
"&Csak az aktuális mappában"

MSearchInSelected
"В &отмеченных папках"
"&Selected folders"
"Ve vy&braných adresářích"
"In au&sgewählten Ordner"
"A ki&jelölt mappákban"

MFindUseFilter
"Исполь&зовать фильтр"
"&Use filter"
"Použít f&iltr"
"Ben&utze Filter"
"Sz&űrővel"

MFindAdvancedOptions
"Дополнит&ельные параметры"
"Advanced options"
"Pokročilé nastavení"
"Erweiterte Optionen"
"Haladó beáll&ítások"

MFindUsingFilter
"используя фильтр"
"using filter"
"používám filtr"
"mit Filter"
"szűrővel"

MFindFileFind
"&Искать"
"&Find"
"&Hledat"
"&Suchen"
"K&eres"

MFindFileDrive
"Дис&к"
"Dri&ve"
"D&isk"
"Lauf&werk"
"Meghajt&ó"

MFindFileSetFilter
"&Фильтр"
"Filt&er"
"&Filtr"
"Filt&er"
"Szű&rő"

MFindFileAdvanced
"До&полнительно"
"Advance&d"
"Pokr&očilé"
"Er&weitert"
"Ha&ladó"

MFindSearchingIn
"Поиск%s в:"
"Searching%s in:"
"Hledám%s v:"
"Suche%s in:"
"%s keresése:"

MFindNewSearch
"&Новый поиск"
"&New search"
"&Nové hledání"
"&Neue Suche"
"&Új keresés"

MFindGoTo
"Пе&рейти"
"&Go to"
"&Jdi na"
"&Gehe zu"
"U&grás"

MFindView
"&Смотреть"
"&View"
"Zo&braz"
"&Betrachten"
"Meg&néz"

MFindPanel
"Пане&ль"
"&Panel"
"&Panel"
"&Panel"
"&Panel"

MFindStop
"С&топ"
"&Stop"
"&Stop"
"&Stoppen"
"&Állj"

MFindDone
l:
"Поиск закончен. Найдено %d файл(ов) и %d папка(ок)"
"Search done. Found %d file(s) and %d folder(s)"
"Hledání ukončeno. Nalezeno %d soubor(ů) a %d adresář(ů)"
"Suche beendet. %d Datei(en) und %d Ordner gefunden."
"A keresés kész. %d fájlt és %d mappát találtam."

MFindCancel
"Отм&ена"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Mégsem"

MFindFound
l:
"Найдено"
"Found"
"Nalezeno"
"Gefunden"
"Találat"

MFindFileFolder
l:
"Папка"
"Folder"
"Adresář"
"Ordner"
"Mappa"

MFindFileAdvancedTitle
l:
"Дополнительные параметры поиска"
"Find file advanced options"
"Pokročilé nastavení vyhledávání souborů"
"Erweiterte Optionen"
"Fájlkeresés haladó beállításai"

MFindFileSearchFirst
"Проводить поиск в &первых:"
"Search only in the &first:"
"Hledat po&uze v prvních:"
"Nur &in den ersten x Bytes:"
"Keresés csak az első &x bájtban:"

MFoldTreeSearch
l:
"Поиск:"
"Search:"
"Hledat:"
"Suchen:"
"Keresés:"

MGetTableTitle
l:
"Кодовые страницы"
"Code pages"
upd:"Znakové sady:"
upd:"Tabellen"
"Kódlapok"

MGetTableBottomTitle
"Ctrl-H, Del, Ins"
"Ctrl-H, Del, Ins"
"Ctrl-H, Del, Ins"
"Strg-H, Entf, Einf"
"Ctrl-H,Del,Ins"

MHighlightTitle
l:
"Раскраска файлов"
"Files highlighting"
"Zvýrazňování souborů"
"Farbmarkierungen"
"Fájlkiemelések, rendezési csoportok"

MHighlightBottom
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Nahoru,Ctrl-Dolů"
"Einf,Entf,F4,F5,StrgUp,StrgDown"
"Ins,Del,F4,F5,Ctrl-Fel,Ctrl-Le"

MHighlightUpperSortGroup
"Верхняя группа сортировки"
"Upper sort group"
"Vzesupné řazení"
"Obere Sortiergruppen"
"Felsőbbrendű csoport"

MHighlightLowerSortGroup
"Нижняя группа сортировки"
"Lower sort group"
"Sestupné řazení"
"Untere Sortiergruppen"
"Alsóbbrendű csoport"

MHighlightLastGroup
"Наименее приоритетная группа раскраски"
"Lowest priority highlighting group"
"Zvýraznění nejnižší prority"
"Farbmarkierungen mit niedrigster Priorität"
"Legalacsonyabb rendű csoport"

MHighlightAskDel
"Вы хотите удалить раскраску для"
"Do you wish to delete highlighting for"
"Přejete si smazat zvýraznění pro"
"Wollen Sie Farbmarkierungen löschen für"
"Biztosan törli a kiemelést?"

MHighlightWarning
"Будут потеряны все Ваши настройки!"
"You will lose all changes!"
"Všechny změny budou ztraceny!"
"Sie verlieren jegliche Änderungen!"
"Minden változtatás elvész!"

MHighlightAskRestore
"Вы хотите восстановить раскраску файлов по умолчанию?"
"Do you wish to restore default highlighting?"
"Přejete si obnovit výchozí nastavení?"
"Wollen Sie Standard-Farbmarkierungen wiederherstellen?"
"Visszaállítja az alapértelmezett kiemeléseket?"

MHighlightEditTitle
l:
"Редактирование раскраски файлов"
"Edit files highlighting"
"Upravit zvýrazňování souborů"
"Farbmarkierungen bearbeiten"
"Fájlkiemelés szerkesztése"

MHighlightMarkChar
"Оп&циональный символ пометки,"
"Optional markin&g character,"
"Volitelný &znak pro označení určených souborů,"
"Optionale Markierun&g mit Zeichen,"
"Megadható &jelölő karakter"

MHighlightTransparentMarkChar
"прозра&чный"
"tra&nsparent"
"průh&ledný"
"tra&nsparent"
"át&látszó"

MHighlightColors
" Цвета файлов (\"черный на черном\" - цвет по умолчанию) "
" File name colors (\"black on black\" - default color) "
" Barva názvu souborů (\"černá na černé\" - výchozí barva) "
" Dateinamenfarben (\"Schwarz auf Schwarz\"=Standard) "
" Fájlnév színek (feketén fekete = alapértelmezett szín) "

MHighlightFileName1
"&1. Обычное имя файла                "
"&1. Normal file name               "
"&1. Normální soubor            "
"&1. Normaler Dateiname             "
"&1. Normál fájlnév                  "

MHighlightFileName2
"&3. Помеченное имя файла             "
"&3. Selected file name             "
"&3. Vybraný soubor             "
"&3. Markierter Dateiame            "
"&3. Kijelölt fájlnév                "

MHighlightFileName3
"&5. Имя файла под курсором           "
"&5. File name under cursor         "
"&5. Soubor pod kurzorem        "
"&5. Dateiname unter Cursor         "
"&5. Kurzor alatti fájlnév           "

MHighlightFileName4
"&7. Помеченное под курсором имя файла"
"&7. File name selected under cursor"
"&7. Vybraný soubor pod kurzorem"
"&7. Dateiname markiert unter Cursor"
"&7. Kurzor alatti kijelölt fájlnév  "

MHighlightMarking1
"&2. Пометка"
"&2. Marking"
"&2. Označení"
"&2. Markierung"
"&2. Jelölő kar.:"

MHighlightMarking2
"&4. Пометка"
"&4. Marking"
"&4. Označení"
"&4. Markierung"
"&4. Jelölő kar.:"

MHighlightMarking3
"&6. Пометка"
"&6. Marking"
"&6. Označení"
"&6. Markierung"
"&6. Jelölő kar.:"

MHighlightMarking4
"&8. Пометка"
"&8. Marking"
"&8. Označení"
"&8. Markierung"
"&8. Jelölő kar.:"

MHighlightExample1
"║filename.ext │"
"║filename.ext │"
"║filename.ext │"
"║dateinam.erw │"
"║fájlneve.kit │"

MHighlightExample2
"║ filename.ext│"
"║ filename.ext│"
"║ filename.ext│"
"║ dateinam.erw│"
"║ fájlneve.kit│"

MHighlightContinueProcessing
"Продолжать &обработку"
"C&ontinue processing"
"Pokračovat ve zpracová&ní"
"Verarbeitung f&ortsetzen"
"Folyamatos f&eldolgozás"

MInfoTitle
l:
"Информация"
"Information"
"Informace"
"Informationen"
"Információk"

MInfoCompName
"Имя компьютера"
"Computer name"
"Název počítače"
"Computername"
"Számítógép neve"

MInfoUserName
"Имя пользователя"
"User name"
"Jméno uživatele"
"Benutzername"
"Felhasználói név"

MInfoRemovable
"Сменный"
"Removable"
"Vyměnitelný"
"Austauschbares"
"Kivehető"

MInfoFixed
"Жесткий"
"Fixed"
"Pevný"
"Lokales"
"Fix"

MInfoNetwork
"Сетевой"
"Network"
"Síťový"
"Netzwerk"
"Hálózati"

MInfoCDROM
"CD-ROM"
"CD-ROM"
"CD-ROM"
"CD-ROM"
"CD-ROM"

MInfoCD_RW
"CD-RW"
"CD-RW"
"CD-RW"
"CD-RW"
"CD-RW"

MInfoCD_RWDVD
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD"

MInfoDVD_ROM
"DVD-ROM"
"DVD-ROM"
"DVD-ROM"
"DVD-ROM"
"DVD-ROM"

MInfoDVD_RW
"DVD-RW"
"DVD-RW"
"DVD-RW"
"DVD-RW"
"DVD-RW"

MInfoDVD_RAM
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"

MInfoRAM
"RAM"
"RAM"
"RAM"
"RAM"
"RAM"

MInfoSUBST
"SUBST"
"Subst"
"SUBST"
"Subst"
"Virtuális"

MInfoDisk
"диск"
"disk"
"disk"
"Laufwerk"
"lemez"

MInfoDiskTotal
"Всего байтов"
"Total bytes"
"Celkem bytů"
"Bytes gesamt"
"Összes bájt"

MInfoDiskFree
"Свободных байтов"
"Free bytes"
"Volných bytů"
"Bytes frei"
"Szabad bájt"

MInfoDiskLabel
"Метка тома"
"Volume label"
"Popisek disku"
"Laufwerksbezeichnung"
"Kötet címke"

MInfoDiskNumber
"Серийный номер"
"Serial number"
"Sériové číslo"
"Seriennummer"
"Sorozatszám"

MInfoMemory
" Память "
" Memory "
" Paměť "
" Speicher "
" Memória "

MInfoMemoryLoad
"Загрузка памяти"
"Memory load"
"Zatížení paměti"
"Speicherverbrauch"
"Használt memória"

MInfoMemoryTotal
"Всего памяти"
"Total memory"
"Celková paměť"
"Speicher gesamt"
"Összes memória"

MInfoMemoryFree
"Свободно памяти"
"Free memory"
"Volná paměť"
"Speicher frei"
"Szabad memória"

MInfoVirtualTotal
"Всего вирт. памяти"
"Total virtual"
"Celkem virtuální"
"Virtueller Speicher gesamt"
"Összes virtuális"

MInfoVirtualFree
"Свободно вирт. памяти"
"Free virtual"
"Volná virtuální"
"Virtueller Speicher frei"
"Szabad virtuális"

MInfoDizAbsent
"Файл описания папки отсутствует"
"Folder description file is absent"
"Soubor s popisem adresáře chybí"
"Keine Datei mit Ordnerbeschreibungen vorhanden."
"Mappa megjegyzésfájl nincs"

MErrorInvalidFunction
l:
"Некорректная функция"
"Incorrect function"
"Nesprávná funkce"
"Ungültige Funktion"
"Helytelen funkció"

MErrorBadCommand
"Команда не распознана"
"Command not recognized"
"Příkaz nebyl rozpoznán"
"Unbekannter Befehl"
"Ismeretlen parancs"

MErrorFileNotFound
"Файл не найден"
"File not found"
"Soubor nenalezen"
"Datei nicht gefunden"
"A fájl vagy mappa nem található"

MErrorPathNotFound
"Путь не найден"
"Path not found"
"Cesta nenalezena"
"Pfad nicht gefunden"
"Az elérési út nem található"

MErrorTooManyOpenFiles
"Слишком много открытых файлов"
"Too many open files"
"Příliš mnoho otevřených souborů"
"Zu viele geöffnete Dateien"
"Túl sok nyitott fájl"

MErrorAccessDenied
"Доступ запрещен"
"Access denied"
"Přístup odepřen"
"Zugriff verweigert"
"Hozzáférés megtagadva"

MErrorNotEnoughMemory
"Недостаточно памяти"
"Not enough memory"
"Nedostatek paměti"
"Nicht genügend Speicher"
"Nincs elég memória"

MErrorDiskRO
"Попытка записи на защищенный от записи диск"
"Cannot write to write protected disk"
"Nelze zapisovat na disk chráněný proti zápisu"
"Der Datenträger ist schreibgeschützt"
"Írásvédett lemezre nem lehet írni"

MErrorDeviceNotReady
"Устройство не готово"
"The device is not ready"
"Zařízení není připraveno"
"Das Gerät ist nicht bereit"
"Az eszköz nem kész"

MErrorCannotAccessDisk
"Доступ к диску невозможен"
"Disk cannot be accessed"
"Na disk nelze přistoupit"
"Auf Datenträger kann nicht zugegriffen werden"
"A lemez nem érhető el"

MErrorSectorNotFound
"Сектор не найден"
"Sector not found"
"Sektor nenalezen"
"Sektor nicht gefunden"
"Szektor nem található"

MErrorOutOfPaper
"В принтере нет бумаги"
"The printer is out of paper"
"V tiskárně došel papír"
"Der Drucker hat kein Papier mehr"
"A nyomtatóban nincs papír"

MErrorWrite
"Ошибка записи"
"Write fault error"
"Chyba zápisu"
"Fehler beim Schreibzugriff"
"Írási hiba"

MErrorRead
"Ошибка чтения"
"Read fault error"
"Chyba čtení"
"Fehler beim Lesezugriff"
"Olvasási hiba"

MErrorDeviceGeneral
"Общая ошибка устройства"
"Device general failure"
"Obecná chyba zařízení"
"Ein Gerätefehler ist aufgetreten"
"Eszköz általános hiba"

MErrorFileSharing
"Нарушение совместного доступа к файлу"
"File sharing violation"
"Narušeno sdílení souborů"
"Zugriffsverletzung"
"Fájlmegosztási hiba"

MErrorNetworkPathNotFound
"Сетевой путь не найден"
"The network path was not found"
"Síťová cesta nebyla nalezena"
"Der Netzwerkpfad wurde nicht gefunden"
"Hálózati útvonal nem található"

MErrorNetworkBusy
"Сеть занята"
"The network is busy"
"Síť je zaneprázdněna"
"Das Netzwerk ist beschäftigt"
"A hálózat foglalt"

MErrorNetworkAccessDenied
"Сетевой доступ запрещен"
"Network access is denied"
"Přístup na síť zakázán"
"Netzwerkzugriff wurde verweigert"
"Hálózati hozzáférés megtagadva"

MErrorNetworkWrite
"Ошибка записи в сети"
"A write fault occurred on the network"
"Na síti došlo k chybě v zápisu"
"Fehler beim Schreibzugriff auf das Netzwerk"
"Írási hiba a hálózaton"

MErrorDiskLocked
"Диск используется или заблокирован другим процессом"
"The disk is in use or locked by another process"
"Disk je používán nebo uzamčen jiným procesem"
"Datenträger wird verwendet oder ist durch einen anderen Prozess gesperrt"
"A lemezt használja vagy zárolja valami"

MErrorFileExists
"Файл или папка уже существует"
"File or folder already exists"
"Soubor nebo adresář již existuje"
"Die Datei oder der Ordner existiert bereits."
"A fájl vagy mappa már létezik"

MErrorInvalidName
"Указанное имя неверно"
"The specified name is invalid"
"Zadaný název je neplatný"
"Der angegebene Name ist ungültig"
"A megadott név érvénytelen"

MErrorInsufficientDiskSpace
"Нет места на диске"
"Insufficient disk space"
"Nedostatek místa na disku"
"Unzureichend Speicherplatz am Datenträger"
"Nincs elég hely a lemezen"

MErrorFolderNotEmpty
"Папка не пустая"
"The folder is not empty"
"Adresář není prázdný"
"Der Ordner ist nicht leer"
"A mappa nem üres"

MErrorIncorrectUserName
"Неверное имя пользователя"
"Incorrect user name"
"Neplatné jméno uživatele"
"Ungültiger Benutzername"
"Rossz felhasználói név"

MErrorIncorrectPassword
"Неверный пароль"
"Incorrect password"
"Neplatné heslo"
"Ungültiges Passwort"
"Rossz jelszó"

MErrorLoginFailure
"Ошибка регистрации"
"Login failure"
"Přihlášení selhalo"
"Login fehlgeschlagen"
"Sikertelen bejelentkezés"

MErrorConnectionAborted
"Соединение разорвано"
"Connection aborted"
"Spojení přerušeno"
"Verbindung abgebrochen"
"Kapcsolat bontva"

MErrorCancelled
"Операция отменена"
"Operation cancelled"
"Operace stornována"
"Vorgang abgebrochen"
"A művelet megszakítva"

MErrorNetAbsent
"Сеть отсутствует"
"No network present"
"Síť není k dispozici"
"Kein Netzwerk verfügbar"
"Nincs hálózat"

MErrorNetDeviceInUse
"Устройство используется и не может быть отсоединено"
"Device is in use and cannot be disconnected"
"Zařízení se používá a nemůže být odpojeno"
"Gerät wird gerade verwendet oder kann nicht getrennt werden"
"Az eszköz használatban van, nem választható le"

MErrorNetOpenFiles
"На сетевом диске есть открытые файлы"
"This network connection has open files"
"Přes toto síťové spojení jsou otevřeny soubory"
"Diese Netzwerkverbindung hat geöffnete Dateien"
"A hálózaton nyitott fájlok vannak"

MErrorAlreadyAssigned
"Имя локального устройства уже использовано"
"The local device name is already in use"
"Název lokálního zařízení je již používán"
"Der lokale Gerätename wird bereits verwendet"
"A helyi eszköznév már foglalt"

MErrorAlreadyRemebered
"Имя локального устройства уже находится в профиле пользователя"
"The local device is already in the user profile"
"Lokální zařízení je již v uživatelově profilu"
"Der lokale Datenträger ist bereits Teil des Benutzerprofils"
"A helyi eszköz már a felhasználói profilban van"

MErrorNotLoggedOn
"Пользователь не зарегистрирован в сети"
"User has not logged on to the network"
"Uživatel nebyl do sítě přihlášen"
"Benutzer hat sich nicht am Netzwerk angemeldet"
"A felhasználó nincs a hálózaton"

MErrorInvalidPassword
"Неверный пароль пользователя"
"The user password is invalid"
"Uživatelovo heslo není správné"
"Das Benutzerpasswort ist ungültig"
"Érvénytelen felhasználói jelszó"

MErrorNoRecoveryPolicy
"Для этой системы отсутствует политика надежного восстановления шифрования"
"There is no valid encryption recovery policy configured for this system"
"V tomto systému není nastaveno žádné platné pravidlo pro dešifrování"
"Auf diesem System ist keine gültige Richtlinie zum Wiederherstellen der Verschlüsselung konfiguriert."
"Nincs érvényes titkosítást feloldó szabály a házirendben"

MErrorEncryptionFailed
"Ошибка при попытке шифрования файла"
"The specified file could not be encrypted"
"Zadaný soubor nemohl být zašifrován"
"Die angegebene Datei konnte nicht verschlüsselt werden"
"A megadott fájl nem titkosítható"

MErrorDecryptionFailed
"Ошибка при попытке расшифровки файла"
"The specified file could not be decrypted"
"Zadaný soubor nemohl být dešifrován"
"Die angegebene Datei konnte nicht entschlüsselt werden"
"A megadott fájl titkosítása nem oldható fel"

MErrorFileNotEncrypted
"Указанный файл не зашифрован"
"The specified file is not encrypted"
"Zadaný soubor není zašifrován"
"Die angegebene Datei ist nicht verschlüsselt"
"A megadott fájl nem titkosított"

MErrorNoAssociation
"Указанному файлу не сопоставлено ни одно приложение для выполнения данной операции"
"No application is associated with the specified file for this operation"
"K zadanému souboru není asociována žádná aplikace pro tuto operaci"
"Diesem Dateityp und dieser Aktion ist kein Programm zugewiesen."
"A fájlhoz nincs társítva program"

MErrorFullPathNameLong
l:
"Полный путь к файлу имеет слишком большую длину"
"The full pathname is too long"
"Plná cesta k souboru je příliš dlouhá"
"Der volle Name des Pfades ist zu lang"
"A teljes elérési út túl hosszú"

MCannotExecute
l:
"Ошибка выполнения"
"Cannot execute"
"Nelze provést"
"Fehler beim Ausführen von"
"Nem végrehajtható:"

MScanningFolder
"Просмотр папки"
"Scanning the folder"
"Prohledávám adresář"
"Scanne den Ordner"
"Mappák olvasása..."

MMakeFolderTitle
l:
"Создание папки"
"Make folder"
"Vytvoření adresáře"
"Ordner erstellen"
"Új mappa létrehozása"

MCreateFolder
"Создать п&апку"
"Create the &folder"
"Vytvořit &adresář"
"Diesen &Ordner erstellen:"
"Mappa &neve:"

MMultiMakeDir
"Обрабатыват&ь несколько имен папок"
"Process &multiple names"
"Zpracovat &více názvů"
"&Mehrere Namen verarbeiten (getrennt durch Semikolon)"
"Töb&b név feldolgozása"

MIncorrectDirList
"Неправильный список папок"
"Incorrect folders list"
"Neplatný seznam adresářů"
"Fehlerhafte Ordnerliste"
"Hibás mappalista"

MCannotCreateFolder
"Ошибка создания папки"
"Cannot create the folder"
"Adresář nelze vytvořit"
"Konnte den Ordner nicht erstellen"
"A mappa nem hozható létre"

MMenuBriefView
l:
"&Краткий                  LCtrl-1"
"&Brief              LCtrl-1"
"&Stručný                  LCtrl-1"
"&Kurz                 LStrg-1"
"&Rövid              BalCtrl-1"

MMenuMediumView
"&Средний                  LCtrl-2"
"&Medium             LCtrl-2"
"S&třední                  LCtrl-2"
"&Mittel               LStrg-2"
"&Közepes            BalCtrl-2"

MMenuFullView
"&Полный                   LCtrl-3"
"&Full               LCtrl-3"
"&Plný                     LCtrl-3"
"&Voll                 LStrg-3"
"&Teljes             BalCtrl-3"

MMenuWideView
"&Широкий                  LCtrl-4"
"&Wide               LCtrl-4"
"Š&iroký                   LCtrl-4"
"B&reitformat          LStrg-4"
"&Széles             BalCtrl-4"

MMenuDetailedView
"&Детальный                LCtrl-5"
"Detai&led           LCtrl-5"
"Detai&lní                 LCtrl-5"
"Detai&lliert          LStrg-5"
"Rész&letes          BalCtrl-5"

MMenuDizView
"&Описания                 LCtrl-6"
"&Descriptions       LCtrl-6"
"P&opisky                  LCtrl-6"
"&Beschreibungen       LStrg-6"
"Fájl&megjegyzések   BalCtrl-6"

MMenuLongDizView
"Д&линные описания         LCtrl-7"
"Lon&g descriptions  LCtrl-7"
"&Dlouhé popisky           LCtrl-7"
"Lan&ge Beschreibungen LStrg-7"
"&Hosszú megjegyzés  BalCtrl-7"

MMenuOwnersView
"Вл&адельцы файлов         LCtrl-8"
"File own&ers        LCtrl-8"
"Vlastník so&uboru         LCtrl-8"
"B&esitzer             LStrg-8"
"Fájl tula&jdonos    BalCtrl-8"

MMenuLinksView
"Свя&зи файлов             LCtrl-9"
"File lin&ks         LCtrl-9"
"Souborové lin&ky          LCtrl-9"
"Dateilin&ks           LStrg-9"
"Fájl li&nkek        BalCtrl-9"

MMenuAlternativeView
"Аль&тернативный полный    LCtrl-0"
"&Alternative full   LCtrl-0"
"&Alternativní plný        LCtrl-0"
"&Alternativ voll      LStrg-0"
"&Alternatív teljes  BalCtrl-0"

MMenuInfoPanel
l:
"Панель ин&формации        Ctrl-L"
"&Info panel         Ctrl-L"
"Panel In&fo               Ctrl-L"
"&Infopanel            Strg-L"
"&Info panel         Ctrl-L"

MMenuTreePanel
"Де&рево папок             Ctrl-T"
"&Tree panel         Ctrl-T"
"Panel St&rom              Ctrl-T"
"Baumansich&t          Strg-T"
"&Fastruktúra        Ctrl-T"

MMenuQuickView
"Быстры&й просмотр         Ctrl-Q"
"Quick &view         Ctrl-Q"
"Z&běžné zobrazení         Ctrl-Q"
"Sc&hnellansicht       Strg-Q"
"&Gyorsnézet         Ctrl-Q"

MMenuSortModes
"Режим&ы сортировки        Ctrl-F12"
"&Sort modes         Ctrl-F12"
"Módy řaze&ní              Ctrl-F12"
"&Sortiermodi          Strg-F12"
"R&endezési elv      Ctrl-F12"

MMenuLongNames
"Показывать длинные &имена Ctrl-N"
"Show long &names    Ctrl-N"
"Zobrazit dlouhé názv&y    Ctrl-N"
"Lange Datei&namen     Strg-N"
"H&osszú fájlnevek   Ctrl-N"

MMenuTogglePanel
"Панель &Вкл/Выкл          Ctrl-F1"
"Panel &On/Off       Ctrl-F1"
"Panel &Zap/Vyp            Ctrl-F1"
"&Panel ein/aus        Strg-F1"
"&Panel be/ki        Ctrl-F1"

MMenuReread
"П&еречитать               Ctrl-R"
"&Re-read            Ctrl-R"
"Obno&vit                  Ctrl-R"
"Aktualisie&ren        Strg-R"
"Friss&ítés          Ctrl-R"

MMenuChangeDrive
"С&менить диск             Alt-F1"
"&Change drive       Alt-F1"
"Z&měnit jednotku          Alt-F1"
"Laufwerk we&chseln    Alt-F1"
"Meghajtó&váltás     Alt-F1"

MMenuView
l:
"&Просмотр              F3"
"&View               F3"
"&Zobrazit                   F3"
"&Betrachten           F3"
"&Megnéz               F3"

MMenuEdit
"&Редактирование        F4"
"&Edit               F4"
"&Editovat                   F4"
"B&earbeiten           F4"
"&Szerkeszt            F4"

MMenuCopy
"&Копирование           F5"
"&Copy               F5"
"&Kopírovat                  F5"
"&Kopieren             F5"
"Más&ol                F5"

MMenuMove
"П&еренос               F6"
"&Rename or move     F6"
"&Přejmenovat/Přesunout      F6"
"Ve&rschieben/Umben.   F6"
"Át&nevez/mozgat       F6"

MMenuCreateFolder
"&Создание папки        F7"
"&Make folder        F7"
"&Vytvořit adresář           F7"
"&Ordner erstellen     F7"
"Ú&j mappa             F7"

MMenuDelete
"&Удаление              F8"
"&Delete             F8"
"&Smazat                     F8"
"&Löschen              F8"
"&Töröl                F8"

MMenuWipe
"Уни&чтожение           Alt-Del"
"&Wipe               Alt-Del"
"&Vymazat                    Alt-Del"
"&Sicher löschen       Alt-Entf"
"&Kisöpör              Alt-Del"

MMenuAdd
"&Архивировать          Shift-F1"
"Add &to archive     Shift-F1"
"Přidat do &archívu          Shift-F1"
"Zu Archiv &hinzuf.    Umsch-F1"
"Tömörhöz ho&zzáad     Shift-F1"

MMenuExtract
"Распако&вать           Shift-F2"
"E&xtract files      Shift-F2"
"&Rozbalit soubory           Shift-F2"
"Archiv e&xtrahieren   Umsch-F2"
"Tömörből ki&bont      Shift-F2"

MMenuArchiveCommands
"Архивн&ые команды      Shift-F3"
"Arc&hive commands   Shift-F3"
"Příkazy arc&hívu            Shift-F3"
"Arc&hivbefehle        Umsch-F3"
"Tömörítő &parancsok   Shift-F3"

MMenuAttributes
"А&трибуты файлов       Ctrl-A"
"File &attributes    Ctrl-A"
"A&tributy souboru           Ctrl-A"
"Datei&attribute       Strg-A"
"Fájl &attribútumok    Ctrl-A"

MMenuApplyCommand
"Применить коман&ду     Ctrl-G"
"A&pply command      Ctrl-G"
"Ap&likovat příkaz           Ctrl-G"
"Befehl an&wenden      Strg-G"
"Parancs &végrehajtása Ctrl-G"

MMenuDescribe
"&Описание файлов       Ctrl-Z"
"Descri&be files     Ctrl-Z"
"Přidat popisek sou&borům    Ctrl-Z"
"Beschrei&bung ändern  Strg-Z"
"Fájlmegje&gyzések     Ctrl-Z"

MMenuSelectGroup
"Пометить &группу       Gray +"
"Select &group       Gray +"
"Oz&načit skupinu            Num +"
"&Gruppe auswählen     Num +"
"Csoport k&ijelölése   Szürke +"

MMenuUnselectGroup
"С&нять пометку         Gray -"
"U&nselect group     Gray -"
"O&dznačit skupinu           Num -"
"G&ruppe abwählen      Num -"
"Jelölést l&evesz      Szürke -"

MMenuInvertSelection
"&Инверсия пометки      Gray *"
"&Invert selection   Gray *"
"&Invertovat výběr           Num *"
"Auswah&l umkehren     Num *"
"Jelölést meg&fordít   Szürke *"

MMenuRestoreSelection
"Восстановить по&метку  Ctrl-M"
"Re&store selection  Ctrl-M"
"&Obnovit výběr              Ctrl-M"
"Auswahl wiederher&st. Strg-M"
"Jel&ölést visszatesz  Ctrl-M"

MMenuFindFile
l:
"&Поиск файла              Alt-F7"
"&Find file           Alt-F7"
"H&ledat soubor                  Alt-F7"
"Dateien &finden       Alt-F7"
"Fájl&keresés         Alt-F7"

MMenuHistory
"&История команд           Alt-F8"
"&History             Alt-F8"
"&Historie                       Alt-F8"
"&Historie             Alt-F8"
"Parancs &előzmények  Alt-F8"

MMenuVideoMode
"Видео&режим               Alt-F9"
"&Video mode          Alt-F9"
"&Video mód                      Alt-F9"
"Ansicht<->&Vollbild   Alt-F9"
"&Video mód           Alt-F9"

MMenuFindFolder
"Поис&к папки              Alt-F10"
"Fi&nd folder         Alt-F10"
"Hl&edat adresář                 Alt-F10"
"Ordner fi&nden        Alt-F10"
"&Mappakeresés        Alt-F10"

MMenuViewHistory
"Ис&тория просмотра        Alt-F11"
"File vie&w history   Alt-F11"
"Historie &zobrazení souborů     Alt-F11"
"Be&trachterhistorie   Alt-F11"
"Fáj&l előzmények     Alt-F11"

MMenuFoldersHistory
"Ист&ория папок            Alt-F12"
"F&olders history     Alt-F12"
"Historie &adresářů              Alt-F12"
"&Ordnerhistorie       Alt-F12"
"Ma&ppa előzmények    Alt-F12"

MMenuSwapPanels
"По&менять панели          Ctrl-U"
"&Swap panels         Ctrl-U"
"Prohodit panel&y                Ctrl-U"
"Panels tau&schen      Strg-U"
"Panel&csere          Ctrl-U"

MMenuTogglePanels
"Панели &Вкл/Выкл          Ctrl-O"
"&Panels On/Off       Ctrl-O"
"&Panely Zap/Vyp                 Ctrl-O"
"&Panels ein/aus       Strg-O"
"Panelek &be/ki       Ctrl-O"

MMenuCompareFolders
"&Сравнение папок"
"&Compare folders"
"Po&rovnat adresáře"
"Ordner verglei&chen"
"Mappák össze&hasonlítása"

MMenuUserMenu
"Меню пользовател&я"
"Edit user &menu"
"Upravit uživatelské &menu"
"Benutzer&menu editieren"
"Felhasználói m&enü szerk."

MMenuFileAssociations
"&Ассоциации файлов"
"File &associations"
"Asocia&ce souborů"
"Dat&eiverknüpfungen"
"Fájl&társítások"

MMenuFolderShortcuts
"Ссы&лки на папки"
"Fol&der shortcuts"
"A&dresářové zkratky"
"Or&dnerschnellzugriff"
"Mappa gyorsbillent&yűk"

MMenuFilter
"&Фильтр панели файлов     Ctrl-I"
"File panel f&ilter   Ctrl-I"
"F&iltr panelu souborů           Ctrl-I"
"Panelf&ilter          Strg-I"
"Fájlpanel &szűrők    Ctrl-I"

MMenuPluginCommands
"Команды внешних мо&дулей  F11"
"Pl&ugin commands     F11"
"Příkazy plu&ginů                F11"
"Pl&uginbefehle        F11"
"Pl&ugin parancsok    F11"

MMenuWindowsList
"Список экра&нов           F12"
"Sc&reens list        F12"
"Seznam obrazove&k               F12"
"Seite&nliste          F12"
"Képer&nyők           F12"

MMenuProcessList
"Список &задач             Ctrl-W"
"Task &list           Ctrl-W"
"Seznam úl&oh                    Ctrl-W"
"Task&liste            Strg-W"
"Futó p&rogramok      Ctrl-W"

MMenuHotPlugList
"Список Hotplug-&устройств"
"Ho&tplug devices list"
"Seznam v&yjímatelných zařízení"
"Sicheres En&tfernen"
"H&otplug eszközök"

MMenuSystemSettings
l:
"Систе&мные параметры"
"S&ystem settings"
"Nastavení S&ystému"
"&Grundeinstellungen"
"&Rendszer beállítások"

MMenuPanelSettings
"Настройки па&нели"
"&Panel settings"
"Nastavení &Panelů"
"&Panels einrichten"
"&Panel beállítások"

MMenuInterface
"Настройки &интерфейса"
"&Interface settings"
"Nastavení Ro&zhraní"
"Oberfläche einr&ichten"
"Kezelő&felület beállítások"

MMenuDialogSettings
"Настройки &диалогов"
"Di&alog settings"
"Nastavení Dialo&gů"
"Di&aloge einrichten"
"Pár&beszédablak beállítások"

MMenuLanguages
"&Языки"
"Lan&guages"
"Nastavení &Jazyka"
"Sprac&hen"
"N&yelvek"

MMenuPluginsConfig
"Параметры &внешних модулей"
"Pl&ugins configuration"
"Nastavení Plu&ginů"
"Konfiguration von Pl&ugins"
"Pl&ugin beállítások"

MMenuConfirmation
"&Подтверждения"
"Co&nfirmations"
"P&otvrzení"
"Bestätigu&ngen"
"Meg&erősítések"

MMenuFilePanelModes
"Режим&ы панели файлов"
"File panel &modes"
"&Módy souborových panelů"
"Anzeige&modi von Dateipanels"
"Fájlpanel mód&ok"

MMenuFileDescriptions
"&Описания файлов"
"File &descriptions"
"Popi&sy souborů"
"&Dateibeschreibungen"
"Fájl &megjegyzésfájlok"

MMenuFolderInfoFiles
"Файлы описания п&апок"
"&Folder description files"
"Soubory popisů &adresářů"
"O&rdnerbeschreibungen"
"M&appa megjegyzésfájlok"

MMenuViewer
"Настройки про&граммы просмотра"
"&Viewer settings"
"Nastavení P&rohlížeče"
"Be&trachter einrichten"
"&Nézőke beállítások"

MMenuEditor
"Настройки &редактора"
"&Editor settings"
"Nastavení &Editoru"
"&Editor einrichten"
"&Szerkesztő beállítások"

MMenuColors
"&Цвета"
"Co&lors"
"&Barvy"
"&Farben"
"S&zínek"

MMenuFilesHighlighting
"Раскраска &файлов и группы сортировки"
"Files &highlighting and sort groups"
"Z&výrazňování souborů a skupiny řazení"
"Farbmar&kierungen und Sortiergruppen"
"Fájlkiemelések, rendezési &csoportok"

MMenuSaveSetup
"&Сохранить параметры                  Shift-F9"
"&Save setup                         Shift-F9"
"&Uložit nastavení                      Shift-F9"
"Setup &speichern                     Umsch-F9"
"Beállítások men&tése                 Shift-F9"

MMenuTogglePanelRight
"Панель &Вкл/Выкл          Ctrl-F2"
"Panel &On/Off       Ctrl-F2"
"Panel &Zap/Vyp            Ctrl-F2"
"Panel &ein/aus        Strg-F2"
"Panel be/&ki        Ctrl-F2"

MMenuChangeDriveRight
"С&менить диск             Alt-F2"
"&Change drive       Alt-F2"
"Z&měnit jednotku          Alt-F2"
"Laufwerk &wechseln    Alt-F2"
"Meghajtó&váltás     Alt-F2"

MMenuLeftTitle
l:
"&Левая"
"&Left"
"&Levý"
"&Links"
"&Bal"

MMenuFilesTitle
"&Файлы"
"&Files"
"&Soubory"
"&Dateien"
"&Fájlok"

MMenuCommandsTitle
"&Команды"
"&Commands"
"Pří&kazy"
"&Befehle"
"&Parancsok"

MMenuOptionsTitle
"Па&раметры"
"&Options"
"&Nastavení"
"&Optionen"
"B&eállítások"

MMenuRightTitle
"&Правая"
"&Right"
"&Pravý"
"&Rechts"
"&Jobb"

MMenuSortTitle
l:
"Критерий сортировки"
"Sort by"
"Seřadit podle"
"Sortieren nach"
"Rendezési elv"

MMenuSortByName
"&Имя                              Ctrl-F3"
"&Name                 Ctrl-F3"
"&Názvu                     Ctrl-F3"
"&Name                   Strg-F3"
"&Név                  Ctrl-F3"

MMenuSortByExt
"&Расширение                       Ctrl-F4"
"E&xtension            Ctrl-F4"
"&Přípony                   Ctrl-F4"
"&Erweiterung            Strg-F4"
"Ki&terjesztés         Ctrl-F4"

MMenuSortByModification
"Время &модификации                Ctrl-F5"
"&Modification time    Ctrl-F5"
"Č&asu modifikace           Ctrl-F5"
"&Veränderungsdatum      Strg-F5"
"Módosítás &ideje      Ctrl-F5"

MMenuSortBySize
"Р&азмер                           Ctrl-F6"
"&Size                 Ctrl-F6"
"&Velikosti                 Ctrl-F6"
"&Größe                  Strg-F6"
"&Méret                Ctrl-F6"

MMenuUnsorted
"&Не сортировать                   Ctrl-F7"
"&Unsorted             Ctrl-F7"
"N&eřadit                   Ctrl-F7"
"&Unsortiert             Strg-F7"
"&Rendezetlen          Ctrl-F7"

MMenuSortByCreation
"Время &создания                   Ctrl-F8"
"&Creation time        Ctrl-F8"
"&Data vytvoření            Ctrl-F8"
"E&rstelldatum           Strg-F8"
"Ke&letkezés ideje     Ctrl-F8"

MMenuSortByAccess
"Время &доступа                    Ctrl-F9"
"&Access time          Ctrl-F9"
"Ča&su přístupu             Ctrl-F9"
"&Zugriffsdatum          Strg-F9"
"&Hozzáférés ideje     Ctrl-F9"

MMenuSortByDiz
"&Описания                         Ctrl-F10"
"&Descriptions         Ctrl-F10"
"P&opisků                   Ctrl-F10"
"&Beschreibungen         Strg-F10"
"Megjegyzé&sek         Ctrl-F10"

MMenuSortByOwner
"&Владельцы файлов                 Ctrl-F11"
"&Owner                Ctrl-F11"
"V&lastníka                 Ctrl-F11"
"Bes&itzer               Strg-F11"
"Tula&jdonos           Ctrl-F11"

MMenuSortByCompressedSize
"&Упакованный размер"
"Com&pressed size"
"&Komprimované velikosti"
"Kom&primierte Größe"
"Tömörített mér&et"

MMenuSortByNumLinks
"Ко&личество ссылок"
"Number of &hard links"
"Poč&tu pevných linků"
"Anzahl an &Links"
"Hardlinkek s&záma"

MMenuSortUseGroups
"Использовать &группы сортировки   Shift-F11"
"Use sort &groups      Shift-F11"
"Řazení podle skup&in       Shift-F11"
"Sortier&gruppen verw.   Umsch-F11"
"Rend. cs&oport haszn. Shift-F11"

MMenuSortSelectedFirst
"&Помеченные файлы вперед          Shift-F12"
"Show selected &first  Shift-F12"
"Nejdřív zobrazit vy&brané  Shift-F12"
"&Ausgewählte zuerst     Umsch-F12"
"Kijel&ölteket előre   Shift-F12"

MMenuSortUseNumeric
"Использовать &числовую сортировку"
"Use num&eric sort"
"Použít čí&selné řazení"
"Nu&merische Sortierung"
"N&umerikus rendezés"

MChangeDriveTitle
l:
"Диск"
"Drive"
"Jednotka   "
"Laufwerke"
"Meghajtók"

MChangeDriveRemovable
"сменный  "
"removable"
"vyměnitelná"
"wechsel. "
"kivehető "

MChangeDriveFixed
"жесткий  "
"fixed    "
"pevná      "
"fest     "
"fix      "

MChangeDriveNetwork
"сетевой  "
"network  "
"síťová     "
"Netzwerk "
"hálózati "

MChangeDriveCDROM
"CD-ROM   "
"CD-ROM   "
"CD-ROM     "
"CD-ROM   "
"CD-ROM   "

MChangeDriveCD_RW
"CD-RW    "
"CD-RW    "
"CD-RW      "
"CD-RW    "
"CD-RW    "

MChangeDriveCD_RWDVD
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD  "
"CD-RW/DVD"
"CD-RW/DVD"

MChangeDriveDVD_ROM
"DVD-ROM  "
"DVD-ROM  "
"DVD-ROM    "
"DVD-ROM  "
"DVD-ROM  "

MChangeDriveDVD_RW
"DVD-RW   "
"DVD-RW   "
"DWD-RW     "
"DVD-RW   "
"DVD-RW   "

MChangeDriveDVD_RAM
"DVD-RAM  "
"DVD-RAM  "
"DVD-RAM    "
"DVD-RAM  "
"DVD-RAM  "

MChangeDriveRAM
"RAM диск "
"RAM disk "
"RAM disk   "
"RAM-DISK "
"RAM lemez"

MChangeDriveSUBST
"SUBST    "
"subst    "
"SUBST      "
"Subst    "
"virtuális"

MChangeDriveLabelAbsent
"недоступен"
"not available"
"není k dispozici"
"nicht vorh.  "
"nem elérhető"

MChangeDriveCannotReadDisk
"Ошибка чтения диска в дисководе %c:"
"Cannot read the disk in drive %c:"
"Nelze přečíst disk v jednotce %c:"
"Datenträge in Laufwerk %c: kann nicht gelesen werden."
"%c: meghajtó lemeze nem olvasható"

MChangeDriveCannotDisconnect
"Не удается отсоединиться от %s"
"Cannot disconnect from %s"
"Nelze se odpojit od %s"
"Verbindung zu %s konnte nicht getrennt werden."
"Nem lehet leválni innen: %s"

MChangeDriveCannotDelSubst
"Не удается удалить виртуальный драйвер %s"
"Cannot delete a substituted drive %s"
"Nelze smazat substnutá jednotka %s"
"Substlaufwerk %s konnte nicht gelöscht werden."
"%s virtuális meghajtó nem törölhető"

MChangeDriveOpenFiles
"Если вы не закроете открытые файлы, данные могут быть утеряны"
"If you do not close the open files, data may be lost."
"Pokud neuzavřete otevřené soubory, mohou být tato data ztracena."
"Wenn Sie offene Dateien nicht schließen könnten Daten verloren gehen."
"Ha a nyitott fájlokat nem zárja be, az adatok elveszhetnek!"

MChangeSUBSTDisconnectDriveTitle
l:
"Отключение виртуального драйвера"
"Virtual device disconnection"
"Odpojování virtuálního zařízení"
"Virtuelles Gerät trennen"
"Virtuális meghajtó törlése"

MChangeSUBSTDisconnectDriveQuestion
"Отключить SUBST-диск %c:?"
"Disconnect SUBST-disk %c:?"
"Odpojit SUBST-disk %c:?"
"Substlaufwerk %c: trennen?"
"Törli %c: virtuális meghajtót?"

MChangeHotPlugDisconnectDriveTitle
l:
"Удаление устройства"
"Device Removal"
"Odpojování zařízení"
"Sicheres Entfernen"
"Eszköz biztonságos eltávolítása"

MChangeHotPlugDisconnectDriveQuestion
"Вы хотите удалить устройство"
"Do you want to remove the device"
"Opravdu si přejete odpojit zařízení"
"Wollen Sie folgendes Gerät sicher entfernen? "
"Eltávolítja az eszközt?"

MHotPlugDisks
"(диск(и): %s)"
"(disk(s): %s)"
"(disk(y): %s)"
"(Laufwerk(e): %s)"
"(%s meghajtó)"

MChangeCouldNotEjectHotPlugMedia
"Невозможно удалить устройство для диска %c:"
"Cannot remove a device for drive %c:"
"Zařízení %c: nemůže být odpojeno."
"Ein Gerät für Laufwerk %c: konnte nicht entfernt werden"
"%c: eszköz nem távolítható el"

MChangeCouldNotEjectHotPlugMedia2
"Невозможно удалить устройство:"
"Cannot remove a device:"
"Zařízení nemůže být odpojeno."
"Kann folgendes Geräte nicht entfernen:"
"Az eszköz nem távolítható el:"

MChangeHotPlugNotify1
"Теперь устройство" 
"The device" 
"Zařízení"
"Das Gerät"
"Az eszköz:"

MChangeHotPlugNotify2
"может быть безопасно извлечено из компьютера"
"can now be safely removed"
"může být nyní bezpečně odebráno"
"kann nun vom Computer getrennt werden."
"már biztonságosan eltávolítható!"

MHotPlugListTitle
"Hotplug-устройства"
"Hotplug devices list"
"Seznam vyjímatelných zařízení"
"Hardware sicher entfernen"
"Hotplug eszközök"

MHotPlugListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Tasten: Entf,StrgR,F1"
"Szerkesztés: Del,Ctrl-R"

MChangeDriveDisconnectTitle
l:
"Отключение сетевого устройства"
"Disconnect network drive"
"Odpojit síťovou jednotku"
"Netzwerklaufwerk trennen"
"Hálózati meghajtó leválasztása"

MChangeDriveDisconnectQuestion
"Вы хотите удалить соединение с устройством %c:?"
"Do you want to disconnect from the drive %c:?"
"Opravdu si přejete odpojit od jednotky %c:?"
"Wollen Sie die Verbindung zu Laufwerk %c: trennen?"
"Le akar válni %c: meghajtóról?"

MChangeDriveDisconnectMapped
"На устройство %c: отображена папка"
"The drive %c: is mapped to..."
"Jednotka %c: je namapována na..."
"Laufwerk %c: ist verknüpft zu..."
"%c: meghajtó hozzárendelve:"

MChangeDriveDisconnectReconnect
"&Восстанавливать при входе в систему"
"&Reconnect at Logon"
"&Znovu připojit při přihlášení"
"Bei Anmeldung &verbinden"
"&Bejelentkezésnél újracsatlakoztat"

MChangeDriveAskDisconnect
l:
"Вы хотите в любом случае отключиться от устройства?"
"Do you want to disconnect the device anyway?"
"Přejete si přesto zařízení odpojit?"
"Wollen Sie die Verbindung trotzdem trennen?"
"Mindenképpen leválasztja az eszközt?"

MChangeVolumeInUse
"Не удается извлечь диск из привода %c:"
"Volume %c: cannot be ejected."
"Jednotka %c: nemůže být vysunuta."
"Datenträger %c: kann nicht ausgeworfen werden."
"%c: kötet nem oldható ki"

MChangeVolumeInUse2
"Используется другим приложением"
"It is used by another application"
"Je používaná jinou aplikací"
"Andere Programme greifen momentan darauf zu"
"Másik program használja"

MChangeWaitingLoadDisk
ls:;"Ожидание загрузки диска..."
"Ожидание чтения диска..."
ls:;"Waiting for disk to load..."
"Waiting for disk to mount..."
ls:;"Nahrávání disku..."
"Čekám na disk k připojení..."
"Warte auf Datenträger..."
ls:;"Lemez betöltése..."
"Lemez betöltése..."

MChangeCouldNotEjectMedia
"Невозможно извлечь диск из привода %c:"
"Could not eject media from drive %c:"
"Nelze vysunout médium v jednotce %c:"
"Konnte Medium in Laufwerk %c: nicht auswerfen"
"%c: meghajtó lemeze nem oldható ki"

MAdditionalHotKey
"#!$%*+-/(),."
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
" Keresés "

MCannotCreateListFile
"Ошибка создания списка файлов"
"Cannot create list file"
"Nelze vytvořit soubor se seznamem"
"Dateiliste konnte nicht erstellt werden"
"Listafájl nem hozható létre"

MCannotCreateListTemp
"(невозможно создать временный файл для списка)"
"(cannot create temporary file for list)"
"(nemohu vytvořit dočasný soubor pro seznam)"
"(Fehler beim Anlegen einer temporären Datei für Liste)"
"(lista átmeneti fájl nem hozható létre)"

MCannotCreateListWrite
"(невозможно записать данные в файл)"
"(cannot write data in file)"
"(nemohu zapsat data do souboru)"
"(Fehler beim Schreiben der Daten)"
"(a fájlba nem írható adat)"

MDragFiles
l:
"%d файлов"
"%d files"
"%d souborů"
"%d Dateien"
"%d fájl"

MDragMove
"Перенос %s"
"Move %s"
"Přesunout %s"
"Verschiebe %s"
"%s mozgatása"

MDragCopy
"Копирование %s"
"Copy %s"
"Kopírovat %s"
"Kopiere %s"
"%s másolása"

MProcessListTitle
l:
"Список задач"
"Task list"
"Seznam úloh"
"Taskliste"
"Futó programok"

MProcessListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Tasten: Entf,StrgR"
"Szerk.: Del,Ctrl-R"

MKillProcessTitle
"Удаление задачи"
"Kill task"
"Zabít úlohu"
"Task beenden"
"Programkilövés"

MAskKillProcess
"Вы хотите удалить выбранную задачу?"
"Do you wish to kill selected task?"
"Přejete si zabít vybranou úlohu?"
"Wollen Sie den ausgewählten Task beenden?"
"Ki akarja lőni a kijelölt programot?"

MKillProcessWarning
"Вы потеряете всю несохраненную информацию этой программы"
"You will lose any unsaved information in this program"
"V tomto programu budou ztraceny neuložené informace"
"Alle ungespeicherten Daten dieses Programmes gehen verloren."
"A program minden mentetlen adata elvész!"

MKillProcessKill
"Удалить"
"Kill"
"Zabít"
"Beenden"
"Kilő"

MCannotKillProcess
"Указанную задачу удалить не удалось"
"Cannot kill the specified task"
"Nemohu ukončit zvolenou úlohu"
"Task konnte nicht beendet werden"
"A programot nem lehet kilőni"

MCannotKillProcessPerm
"Вы не имеет права удалить этот процесс."
"You have no permission to kill this process."
"Nemáte oprávnění zabít tento proces."
"Sie haben keine Rechte um diesen Prozess zu beenden."
"Nincs joga a program kilövésére"

MQuickViewTitle
l:
"Быстрый просмотр"
"Quick view"
"Zběžné zobrazení"
"Schnellansicht"
"Gyorsnézet"

MQuickViewFolder
"Папка \"%s\""
"Folder \"%s\""
"Adresář \"%s\""
"Verzeichnis \"%s\""
"\"%s\" mappa"

MQuickViewJunction
"Связь \"%s\""
"Junction \"%s\""
"Křížení \"%s\""
"Knotenpunkt \"%s\""
"\"%s\" csomópont"

MQuickViewSymlink
"Ссылка \"%s\""
"Symlink \"%s\""
"Symbolický link \"%s\""
"Symlink \"%s\""
"\"%s\" szimlink"

MQuickViewVolMount
"Том \"%s\""
"Volume \"%s\""
"Svazek \"%s\""
"Datenträger \"%s\""
"\"%s\" kötet"

MQuickViewContains
"Содержит:"
"Contains:"
"Obsah:"
"Enthält:"
"Tartalma:"

MQuickViewFolders
"Папок               "
"Folders          "
"Adresáře           "
"Ordner           "
"Mappák száma     "

MQuickViewFiles
"Файлов              "
"Files            "
"Soubory            "
"Dateien          "
"Fájlok száma     "

MQuickViewBytes
"Размер файлов       "
"Files size       "
"Velikost souborů   "
"Gesamtgröße      "
"Fájlok mérete    "

MQuickViewCompressed
"Упакованный размер  "
"Compressed size  "
"Komprim. velikost  "
"Komprimiert      "
"Tömörített méret "

MQuickViewRatio
"Степень сжатия      "
"Ratio            "
"Poměr              "
"Rate             "
"Tömörítés aránya "

MQuickViewCluster
"Размер кластера     "
"Cluster size     "
"Velikost clusteru  "
"Clustergröße     "
"Klaszterméret    "

MQuickViewRealSize
"Реальный размер     "
"Real files size  "
"Opravdová velikost "
"Tatsächlich      "
"Valódi fájlméret "

MQuickViewSlack
"Остатки кластеров   "
"Files slack      "
"Mrtvé místo        "
"Verlust          "
"Meddő adatok     "

MSetAttrTitle
l:
"Атрибуты"
"Attributes"
"Atributy"
"Attribute"
"Attribútumok"

MSetAttrFor
"Изменить файловые атрибуты"
"Change file attributes for"
"Změna atributů souboru pro"
"Ändere Dateiattribute für"
"Attribútumok megváltoztatása"

MSetAttrSelectedObjects
"выбранных объектов"
"selected objects"
"vybrané objekty"
"markierte Objekte"
"a kijelölt objektumokon"

MSetAttrHardLinks
"жестких ссылок (%d)"
"hard links (%d)"
"pevné linky (%d)"
"Hardlinks (%d)"
"hardlink (%d)"

MSetAttrJunction
"Связь \"%s\""
"Junction \"%s\""
"Křížení \"%s\""
"Knotenpunkte \"%s\""
"\"%s\" csomópont"

MSetAttrSymlink
"Ссылка \"%s\""
"Symlink \"%s\""
"Link \"%s\""
"Symlink \"%s\""
"\"%s\" szimlink"

MSetAttrVolMount
"Том \"%s\""
"Volume \"%s\""
"Svazek \"%s\""
"Datenträger \"%s\""
"\"%s\" kötet"

MSetAttrUnknownJunction
"(нет данных)"
"(data not available)"
"(data nejsou k dispozici)"
"(nicht verfügbar)"
"(adat nem elérhető)"

MSetAttrRO
"&Только для чтения"
"&Read only"
"&Pouze pro čtení"
"Sch&reibschutz"
"&Csak olvasható"

MSetAttrArchive
"&Архивный"
"&Archive"
"&Archivovat"
"&Archiv"
"&Archív"

MSetAttrHidden
"&Скрытый"
"&Hidden"
"&Skrytý"
"&Versteckt"
"&Rejtett"

MSetAttrSystem
"С&истемный"
"&System"
"S&ystémový"
"&System"
"Ren&dszer"

MSetAttrCompressed
"Сжаты&й"
"&Compressed"
"&Komprimovaný"
"&Komprimiert"
"&Tömörített"

MSetAttrEncrypted
"За&шифрованный"
"&Encrypted"
"&Šifrovaný"
"V&erschlüsselt"
"Tit&kosított"

MSetAttrNotIndexed
"Н&еиндексируемый"
"Not &Indexed"
"Neinde&xovaný"
"Nicht &indiziert"
"Nem inde&xelt"

MSetAttrSparse
"Разреженный"
"Sparse"
"Rozptýlený"
"Reserve"
"Ritkított"

MSetAttrTemp
"Временный"
"Temporary"
"Dočasný"
"Temporär"
"Átmeneti"

MSetAttrOffline
"Автономный"
"Offline"
"Offline"
"Offline"
"Offline"

MSetAttrVirtual
"Виртуальный"
"Virtual"
"Virtuální"
"Virtuell"
"Virtuális"

MSetAttrSubfolders
"Обрабатывать &вложенные папки"
"Process sub&folders"
"Zpracovat i po&dadresáře"
"Unterordner miteinbe&ziehen"
"Az almappákon is"

MSetAttrModification
"Время &модификации файла:"
"File &modification time:"
"Čas &modifikace souboru:"
"Datei &modifiziert:"
"&Módosítás dátuma/ideje:"

MSetAttrCreation
"Время со&здания файла:"
"File crea&tion time:"
"Čas v&ytvoření souboru:"
"Datei erstell&t:"
"&Létrehozás dátuma/ideje:"

MSetAttrLastAccess
"Время последнего &доступа к файлу:"
"&Last file access time:"
"Čas posledního pří&stupu:"
"&Letzter Zugriff:"
"&Utolsó hozzáférés dátuma/ideje:"

MSetAttrOriginal
"Исход&ное"
"&Original"
"&Originál"
"&Original"
"&Eredeti"

MSetAttrCurrent
"Те&кущее"
"Curre&nt"
"So&učasný"
"Akt&uell"
"Aktuál&is"

MSetAttrBlank
"Сбр&ос"
"&Blank"
"P&rázdný"
"L&eer"
"&Üres"

MSetAttrSet
"Установить"
"Set"
"Nastavit"
"Setzen"
"Alkalmaz"

MSetAttrTimeTitle1
l:
"ММ%cДД%cГГГГ чч%cмм%cсс"
"MM%cDD%cYYYY hh%cmm%css"
"MM%cDD%cRRRR hh%cmm%css"
"MM%cTT%cJJJJ hh%cmm%css"
"HH%cNN%cÉÉÉÉ óó%cpp%cmm"

MSetAttrTimeTitle2
"ДД%cММ%cГГГГ чч%cмм%cсс"
"DD%cMM%cYYYY hh%cmm%css"
"DD%cMM%cRRRR hh%cmm%css"
"TT%cMM%cJJJJ hh%cmm%css"
"NN%cHH%cÉÉÉÉ óó%cpp%cmm"

MSetAttrTimeTitle3
"ГГГГ%cММ%cДД чч%cмм%cсс"
"YYYY%cMM%cDD hh%cmm%css"
"RRRR%cMM%cDD hh%cmm%css"
"JJJJ%cMM%cTT hh%cmm%css"
"ÉÉÉÉ%cHH%cNN óó%cpp%cmm"

MSetAttrSetting
l:
"Установка файловых атрибутов для"
"Setting file attributes for"
"Nastavení atributů souboru pro"
"Setze Dateiattribute für"
"Attribútumok beállítása"

MSetAttrCannotFor
"Ошибка установки атрибутов для"
"Cannot set attributes for"
"Nelze nastavit atributy pro"
"Konnte Dateiattribute nicht setzen für"
"Az attribútumok nem állíthatók be:"

MSetAttrCompressedCannotFor
"Не удалось установить атрибут СЖАТЫЙ для"
"Cannot set attribute COMPRESSED for"
"Nelze nastavit atribut KOMPRIMOVANÝ pro"
"Konnte Komprimierung nicht setzen für"
"A TÖMÖRÍTETT attribútum nem állítható be:"

MSetAttrEncryptedCannotFor
"Не удалось установить атрибут ЗАШИФРОВАННЫЙ для"
"Cannot set attribute ENCRYPTED for"
"Nelze nastavit atribut ŠIFROVANÝ pro"
"Konnte Verschlüsselung nicht setzen für"
"A TITKOSÍTOTT attribútum nem állítható be:"

MSetAttrTimeCannotFor
"Не удалось установить время файла для"
"Cannot set file time for"
"Nelze nastavit čas souboru pro"
"Konnte Dateidatum nicht setzen für"
"A dátum nem állítható be:"

MSetColorPanel
l:
"&Панель"
"&Panel"
"&Panel"
"&Panel"
"&Panel"

MSetColorDialog
"&Диалог"
"&Dialog"
"&Dialog"
"&Dialog"
"Pár&beszédablak"

MSetColorWarning
"Пр&едупреждение"
"&Warning message"
"&Varovná zpráva"
"&Warnmeldung"
"&Figyelmeztetés"

MSetColorMenu
"&Меню"
"&Menu"
"&Menu"
"&Menü"
"&Menü"

MSetColorHMenu
"&Горизонтальное меню"
"Hori&zontal menu"
"Hori&zontální menu"
"Hori&zontales Menü"
"&Vízszintes menü"

MSetColorKeyBar
"&Линейка клавиш"
"&Key bar"
"&Řádek kláves"
"
"F&unkcióbill.sor"

MSetColorCommandLine
"&Командная строка"
"&Command line"
"Pří&kazový řádek"
"&Kommandozeile"
"P&arancssor"

MSetColorClock
"&Часы"
"C&lock"
"&Hodiny"
"U&hr"
"Ó&ra"

MSetColorViewer
"Про&смотрщик"
"&Viewer"
"P&rohlížeč"
"&Betrachter"
"&Nézőke"

MSetColorEditor
"&Редактор"
"&Editor"
"&Editor"
"&Editor"
"&Szerkesztő"

MSetColorHelp
"П&омощь"
"&Help"
"&Nápověda"
"&Hilfe"
"Sú&gó"

MSetDefaultColors
"&Установить стандартные цвета"
"Set de&fault colors"
"N&astavit výchozí barvy"
"Setze Standard&farben"
"Alapért. s&zínek"

MSetBW
"Черно-бел&ый режим"
"&Black and white mode"
"Černo&bílý mód"
"Schwarz && &Weiß"
"Fekete-fe&hér mód"

MSetColorPanelNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"

MSetColorPanelSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"

MSetColorPanelHighlightedInfo
"Выделенная информация"
"Highlighted info"
"Info zvýrazněné"
"Markierung"
"Kiemelt info"

MSetColorPanelDragging
"Перетаскиваемый текст"
"Dragging text"
"Tažený text"
"Drag && Drop Text"
"Vonszolt szöveg"

MSetColorPanelBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"

MSetColorPanelNormalCursor
"Обычный курсор"
"Normal cursor"
"Normální kurzor"
"Normale Auswahl"
"Normál kurzor"

MSetColorPanelSelectedCursor
"Выделенный курсор"
"Selected cursor"
"Vybraný kurzor"
"Markierte Auswahl"
"Kijelölt kurzor"

MSetColorPanelNormalTitle
"Обычный заголовок"
"Normal title"
"Normální nadpis"
"Normaler Titel"
"Normál név"

MSetColorPanelSelectedTitle
"Выделенный заголовок"
"Selected title"
"Vybraný nadpis"
"Markierter Titel"
"Kijelölt név"

MSetColorPanelColumnTitle
"Заголовок колонки"
"Column title"
"Nadpis sloupce"
"Spaltentitel"
"Oszlopnév"

MSetColorPanelTotalInfo
"Количество файлов"
"Total info"
"Info celkové"
"Gesamtinfo"
"Összes info"

MSetColorPanelSelectedInfo
"Количество выбранных файлов"
"Selected info"
"Info výběr"
"Markierungsinfo"
"Kijelölt info"

MSetColorPanelScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"

MSetColorPanelScreensNumber
"Количество фоновых экранов"
"Number of background screens"
"Počet obrazovek na pozadí"
"Anzahl an Hintergrundseiten"
"Háttérképernyők száma"

MSetColorDialogNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"

MSetColorDialogHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierter Text"
"Kiemelt szöveg"

MSetColorDialogDisabled
"Блокированный текст"
"Disabled text"
"Zakázaný text"
"Deaktivierter Text"
"Inaktív szöveg"

MSetColorDialogBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"

MSetColorDialogBoxTitle
"Заголовок рамки"
"Title"
"Nadpis"
"Titel"
"Keret neve"

MSetColorDialogHighlightedBoxTitle
"Выделенный заголовок рамки"
"Highlighted title"
"Zvýrazněný nadpis"
"Markierter Titel"
"Kiemelt keretnév"

MSetColorDialogTextInput
"Ввод текста"
"Text input"
"Textový vstup"
"Texteingabe"
"Beírt szöveg"

MSetColorDialogUnchangedTextInput
"Неизмененный текст"
"Unchanged text input"
"Nezměněný textový vstup"
"Unveränderte Texteingabe"
"Változatlan beírt szöveg"

MSetColorDialogSelectedTextInput
"Ввод выделенного текста"
"Selected text input"
"Vybraný textový vstup"
"Markierte Texteingabe"
"Beírt szöveg kijelölve"

MSetColorDialogEditDisabled
"Блокированное поле ввода"
"Disabled input line"
"Zakázaný vstupní řádek"
"Deaktivierte Eingabezeile"
"Inaktív beviteli sor"

MSetColorDialogButtons
"Кнопки"
"Buttons"
"Tlačítka"
"Schaltflächen"
"Gombok"

MSetColorDialogSelectedButtons
"Выбранные кнопки"
"Selected buttons"
"Vybraná tlačítka"
"Aktive Schaltflächen"
"Kijelölt gombok"

MSetColorDialogHighlightedButtons
"Выделенные кнопки"
"Highlighted buttons"
"Zvýrazněná tlačítka"
"Markierte Schaltflächen"
"Kiemelt gombok"

MSetColorDialogSelectedHighlightedButtons
"Выбранные выделенные кнопки"
"Selected highlighted buttons"
"Vybraná zvýrazněná tlačítka"
"Aktive markierte Schaltflächen"
"Kijelölt kiemelt gombok"

MSetColorDialogListBoxControl
"Список"
"List box"
"Seznam položek"
"Listenfelder"
"Listaablak"

MSetColorDialogComboBoxControl
"Комбинированный список"
"Combobox"
"Výběr položek"
"Kombinatiosfelder"
"Lenyíló szövegablak"

MSetColorDialogListText
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"

MSetColorDialogListSelectedText
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"

MSetColorDialogListHighLight
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"
"Kiemelt szöveg"

MSetColorDialogListSelectedHighLight
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"
"Aktive Markierung"
"Kijelölt kiemelt szöveg"

MSetColorDialogListDisabled
"Блокированный пункт"
"Disabled item"
"Naktivní položka"
"Deaktiviertes Element"
"Inaktív elem"

MSetColorDialogListBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"

MSetColorDialogListTitle
"Заголовок"
"Title"
"Nadpis"
"Titel"
"Keret neve"

MSetColorDialogListScrollBar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"

MSetColorDialogListArrows
"Индикаторы длинных строк"
"Long string indicators"
"Značka dlouhého řetězce"
"Indikator für lange Zeichenketten"
"Hosszú sztring jelzők"

MSetColorDialogListArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"
"Vybraná značka dlouhého řetězce"
"Aktiver Indikator"
"Kijelölt hosszú sztring jelzők"

MSetColorDialogListArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"
"Zakázaná značka dlouhého řetězce"
"Deaktivierter Indikator"
"Inaktív hosszú sztring jelzők"

MSetColorMenuNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"

MSetColorMenuSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"

MSetColorMenuHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"
"Kiemelt szöveg"

MSetColorMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"
"Aktive Markierung"
"Kijelölt kiemelt szöveg"

MSetColorMenuDisabled
"Недоступный пункт"
"Disabled text"
"Neaktivní text"
"Disabled text"
"Inaktív szöveg"

MSetColorMenuBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"

MSetColorMenuTitle
"Заголовок"
"Title"
"Nadpis"
"Titel"
"Keret neve"

MSetColorMenuScrollBar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"

MSetColorMenuArrows
"Индикаторы длинных строк"
"Long string indicators"
"Značka dlouhého řetězce"
"Long string indicators"
"Hosszú sztring jelzők"

MSetColorMenuArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"
"Vybraná značka dlouhého řetězce"
"Selected long string indicators"
"Kijelölt hosszú sztring jelzők"

MSetColorMenuArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"
"Zakázaná značka dlouhého řetězce"
"Disabled long string indicators"
"Inaktív hosszú sztring jelzők"

MSetColorHMenuNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"

MSetColorHMenuSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"

MSetColorHMenuHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"
"Kiemelt szöveg"

MSetColorHMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"
"Aktive Markierung"
"Kijelölt kiemelt szöveg"

MSetColorKeyBarNumbers
l:
"Номера клавиш"
"Key numbers"
"Čísla kláves"
"Tastenziffern"
"Funkció száma"

MSetColorKeyBarNames
"Названия клавиш"
"Key names"
"Názvy kláves"
"Tastennamen"
"Funkció neve"

MSetColorKeyBarBackground
"Фон"
"Background"
"Pozadí"
"Hintergrund"
"Háttere"

MSetColorCommandLineNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"

MSetColorCommandLineSelected
"Выделенный текст"
"Selected text input"
"Vybraný textový vstup"
"Markierte Texteingabe"
"Beírt szöveg kijelölve"

MSetColorCommandLinePrefix
"Текст префикса"
"Prefix text"
"Text předpony"
"Prefix Text"
"Előtag szövege"

MSetColorCommandLineUserScreen
"Пользовательский экран"
"User screen"
"Obrazovka uživatele"
"Benutzerseite"
"Konzol háttere"

MSetColorClockNormal
l:
"Обычный текст (панели)"
"Normal text (Panel)"
"Normální text (Panel)"
"Normaler Text (Panel)"
"Normál szöveg (Panelek)"

MSetColorClockNormalEditor
"Обычный текст (редактор)"
"Normal text (Editor)"
"Normální text (Editor)"
"Normaler Text (Editor)"
"Normál szöveg (Szerkesztő)"

MSetColorClockNormalViewer
"Обычный текст (вьювер)"
"Normal text (Viewer)"
"Normální text (Prohlížeč)"
"Normaler Text (Betrachter)"
"Normál szöveg (Nézőke)"

MSetColorViewerNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"

MSetColorViewerSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"

MSetColorViewerStatus
"Статус"
"Status line"
"Stavový řádek"
"Statuszeile"
"Állapotsor"

MSetColorViewerArrows
"Стрелки сдвига экрана"
"Screen scrolling arrows"
"Skrolovací šipky"
"Pfeile auf Scrollbalken"
"Képernyőgördítő nyilak"

MSetColorViewerScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"

MSetColorEditorNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"

MSetColorEditorSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"

MSetColorEditorStatus
"Статус"
"Status line"
"Stavový řádek"
"Statuszeile"
"Állapotsor"

MSetColorEditorScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"

MSetColorHelpNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"

MSetColorHelpHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"
"Kiemelt szöveg"

MSetColorHelpReference
"Ссылка"
"Reference"
"Odkaz"
"Referenz"
"Hivatkozás"

MSetColorHelpSelectedReference
"Выбранная ссылка"
"Selected reference"
"Vybraný odkaz"
"Ausgewählte Referenz"
"Kijelölt hivatkozás"

MSetColorHelpBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"

MSetColorHelpBoxTitle
"Заголовок рамки"
"Title"
"Nadpis"
"Titel"
"Keret neve"

MSetColorHelpScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"

MSetColorGroupsTitle
l:
"Цветовые группы"
"Color groups"
"Skupiny barev"
"Farbgruppen"
"Színcsoportok"

MSetColorItemsTitle
"Элементы группы"
"Group items"
"Položky skupin"
"Gruppeneinträge"
"Csoport elemei"

MSetColorTitle
l:
"Цвет"
"Color"
"Barva"
"Farbe"
"Színek"

MSetColorForeground
"&Текст"
"&Foreground"
"&Popředí"
"&Vordergrund"
"&Előtér"

MSetColorBackground
"&Фон"
"&Background"
"Po&zadí"
"&Hintergrund"
"&Háttér"

MSetColorForeTransparent
"&Прозрачный"
"&Transparent"
"Průhlednos&t"
"&Transparent"
"Átlá&tszó"

MSetColorBackTransparent
"П&розрачный"
"T&ransparent"
"Průhledno&st"
"T&ransparent"
"Átlát&szó"

MSetColorSample
"Текст Текст Текст Текст Текст Текст"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"

MSetColorSet
"Установить"
"Set"
"Nastavit"
"Setzen"
"A&lkalmaz"

MSetColorCancel
"Отменить"
"Cancel"
"Storno"
"Abbruch"
"&Mégsem"

MSetConfirmTitle
l:
"Подтверждения"
"Confirmations"
"Potvrzení"
"Bestätigungen"
"Megerősítések"

MSetConfirmCopy
"Перезапись файлов при &копировании"
"&Copy"
"&Kopírování"
"&Kopieren"
"&Másolás"

MSetConfirmMove
"Перезапись файлов при &переносе"
"&Move"
"&Přesouvání"
"&Verschieben"
"Moz&gatás"

MSetConfirmDrag
"Пере&таскивание"
"&Drag and drop"
"&Drag and drop"
"&Ziehen und Ablegen"
"&Húzd és ejtsd"

MSetConfirmDelete
"&Удаление"
"De&lete"
"&Mazání"
"&Löschen"
"&Törlés"

MSetConfirmDeleteFolders
"У&даление непустых папок"
"Delete non-empty &folders"
"Mazat &neprázdné adresáře"
"Löschen von Ordnern mit &Inhalt"
"Nem &üres mappák törlése"

MSetConfirmEsc
"Прерыва&ние операций (Esc)"
"&Interrupt operation"
"Pře&rušit operaci"
"&Unterbrechen von Vorgängen"
"Mű&velet megszakítása"

MSetConfirmRemoveConnection
"&Отключение сетевого устройства"
"Disconnect &network drive"
"Odpojení &síťové jednotky"
"Trennen von &Netzwerklaufwerken"
"Háló&zati meghajtó leválasztása"

MSetConfirmRemoveSUBST
"Отключение SUBST-диска"
"Disconnect &SUBST-disk"
"Odpojení SUBST-d&isku"
"Trennen von &Substlaufwerken"
"Virt&uális meghajtó törlése"

MSetConfirmRemoveHotPlug
"Отключение HotPlug-устройства"
"HotPlug-device removal"
"Odpojení vyjímatelného zařízení"
"Sicheres Entfernen von Hardware"
"H&otPlug eszköz eltávolítása"

MSetConfirmAllowReedit
"Повто&рное открытие файла в редакторе"
"&Reload edited file"
"&Obnovit upravovaný soubor"
"Bea&rbeitete Datei neu laden"
"&Szerkesztett fájl újratöltése"

MSetConfirmHistoryClear
"Очистка списка &истории"
"Clear &history list"
"Vymazat seznam &historie"
"&Historielisten löschen"
"&Előzménylista törlése"

MSetConfirmExit
"&Выход"
"E&xit"
"U&končení"
"Be&enden"
"K&ilépés a FAR-ból"

MFindFolderTitle
l:
"Поиск папки"
"Find folder"
"Najít adresář"
"Ordner finden"
"Mappakeresés"

MKBFolderTreeF1
l:
l:// Find folder Tree KeyBar
"Помощь"
"Help"
"Nápověda"
"Hilfe"
"Súgó"

MKBFolderTreeF2
"Обновить"
"Rescan"
"Obnovit"
"Aktual"
"FaFris"

MKBFolderTreeF5
"Размер"
"Zoom"
"Zoom"
"Vergr."
"Nagyít"

MKBFolderTreeF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"

MKBFolderTreeAltF9
"Видео"
"Video"
"Video"
"Vollb"
"Video"

MTreeTitle
"Дерево"
"Tree"
"Stromové zobrazení"
"Baum"
"Fa"

MCannotSaveTree
"Ошибка записи дерева папок в файл"
"Cannot save folders tree to file"
"Adresářový strom nelze uložit do souboru"
"Konnte Ordnerliste nicht in Datei speichern."
"A mappák fastruktúrája nem menthető fájlba"

MReadingTree
"Чтение дерева папок"
"Reading the folders tree"
"Načítám adresářový strom"
"Lese Ordnerliste"
"Mappaszerkezet újraolvasása..."

MUserMenuTitle
l:
"Пользовательское меню"
"User menu"
"Menu uživatele"
"Benutzermenü"
"Felhasználói menü szerkesztése"

MChooseMenuType
"Выберите тип пользовательского меню для редактирования"
"Choose user menu type to edit"
"Zvol typ menu uživatele pro úpravu"
"Wählen Sie den Typ des zu editierenden Benutzermenüs"
"Felhasználói menü típusa:"

MChooseMenuMain
"&Главное"
"&Main"
"&Hlavní"
"&Hauptmenü"
"&Főmenü"

MChooseMenuLocal
"&Местное"
"&Local"
"&Lokální"
"&Lokales Menü"
"&Helyi menü"

MMainMenuTitle
"Главное меню"
"Main menu"
"Hlavní menu"
"Hauptmenü"
"Főmenü"

MMainMenuFAR
"Папка FAR"
"FAR folder"
"Složka FARu"
"FAR Ordner"
"FAR mappa"

MMainMenuREG
l:
l:// <...menu (Registry)>
"Реестр"
"Registry"
"Registry"
"Reg."
"Registry"

MLocalMenuTitle
"Местное меню"
"Local menu"
"Lokalní menu"
"Lokales Menü"
"Helyi menü"

MMainMenuBottomTitle
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Bearb.: Entf,Einf,F4"
"Szerk.: Del,Ins,F4"

MAskDeleteMenuItem
"Вы хотите удалить пункт меню"
"Do you wish to delete the menu item"
"Přejete si smazat položku v menu"
"Do you wish to delete the menu item"
"Biztosan törli a menüelemet?"

MAskDeleteSubMenuItem
"Вы хотите удалить вложенное меню"
"Do you wish to delete the submenu"
"Přejete si smazat podmenu"
"Do you wish to delete the submenu"
"Biztosan törli az almenüt?"

MUserMenuInvalidInputLabel
"Неправильный формат метки меню!"
"Invalid format for UserMenu Label!"
"Neplatný formát pro název Uživatelského menu!"
"Invalid format for UserMenu Label!"
"A felhasználói menü névformátuma érvénytelen!"

MUserMenuInvalidInputHotKey
"Неправильный формат горячей клавиши!"
"Invalid format for Hot Key!"
"Neplatný formát pro klávesovou zkratku!"
"Invalid format for Hot Key!"
"A gyorsbillentyű formátuma érvénytelen!"

MEditMenuTitle
l:
"Редактирование пользовательского меню"
"Edit user menu"
"Editace uživatelského menu"
"Menübefehl bearbeiten"
"Parancs szerkesztése"

MEditMenuHotKey
"&Горячая клавиша:"
"&Hot key:"
"K&lávesová zkratka:"
"&Kurztaste:"
"&Gyorsbillentyű:"

MEditMenuLabel
"&Метка:"
"&Label:"
"&Popisek:"
"&Bezeichnung:"
"&Név:"

MEditMenuCommands
"&Команды:"
"&Commands:"
"Pří&kazy:"
"&Befehle:"
"&Parancsok:"

MAskInsertMenuOrCommand
l:
"Вы хотите вставить новую команду или новое меню?"
"Do you wish to insert a new command or a new menu?"
"Přejete si vložit nový příkaz nebo nové menu?"
"Wollen Sie einen neuen Menübefehl oder ein neues Menu erstellen?"
"Új parancs vagy új menü?"

MMenuInsertCommand
"Вставить команду"
"Insert command"
"Vložit příkaz"
"Neuer Befehl"
"Parancs"

MMenuInsertMenu
"Вставить меню"
"Insert menu"
"Vložit menu"
"Neues Menü"
"Menü"

MEditSubmenuTitle
l:
"Редактирование метки вложенного меню"
"Edit submenu label"
"Úprava popisku podmenu"
"Untermenü bearbeiten"
"Almenü szerkesztése"

MEditSubmenuHotKey
"&Горячая клавиша:"
"&Hot key:"
"Klávesová &zkratka:"
"&Kurztaste:"
"&Gyorsbillentyű:"

MEditSubmenuLabel
"&Метка:"
"&Label:"
"&Popisek:"
"&Bezeichnung:"
"&Név:"

MViewerTitle
l:
"Просмотр"
"Viewer"
"Prohlížeč"
"Betrachter"
"Nézőke"

MViewerCannotOpenFile
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"
"A fájl nem nyitható meg"

MViewerStatusCol
"Кол"
"Col"
"Sloupec"
"Spalte"
"Oszlop"

MViewSearchTitle
l:
"Поиск"
"Search"
"Hledat"
"Durchsuchen"
"Keresés"

MViewSearchFor
"&Искать"
"&Search for"
"H&ledat"
"&Suchen nach"
"&Keresés:"

MViewSearchForText
"Искать &текст"
"Search for &text"
"Hledat &text"
"Suchen nach &Text"
"&Szöveg keresése"

MViewSearchForHex
"Искать 16-ричный &код"
"Search for &hex"
"Hledat he&x"
"Suchen nach &Hex (xx xx ...)"
"&Hexa keresése"

MViewSearchCase
"&Учитывать регистр"
"&Case sensitive"
"&Rozlišovat velikost písmen"
"Gr&oß-/Kleinschreibung"
"&Nagy/kisbetű érzékeny"

MViewSearchWholeWords
"Только &целые слова"
"&Whole words"
"Celá &slova"
"Ganze &Wörter"
"E&gész szavakra"

MViewSearchReverse
"Обратн&ый поиск"
"Re&verse search"
"&Zpětné hledání"
"Richtung um&kehren"
"&Visszafelé keres"

MViewSearchSearch
"Искать"
"Search"
"Hledat"
"Suchen"
"Keres"

MViewSearchCancel
"Отменить"
"Cancel"
"Storno"
"Abbrechen"
"Mégsem"

MViewSearchingFor
l:
"Поиск"
"Searching for"
"Vyhledávám"
"Suche nach"
"Keresés:"

MViewSearchingHex
"Поиск байтов"
"Searching for bytes"
"Vyhledávám sekvenci bytů"
"Suche nach Bytes"
"Bájtok keresése:"

MViewSearchCannotFind
"Строка не найдена"
"Could not find the string"
"Nelze najít řetězec"
"Konnte Zeichenkette nicht finden"
"Nem találtam a szöveget:"

MViewSearchCannotFindHex
"Байты не найдены"
"Could not find the bytes"
"Nelze najít sekvenci bytů"
"Konnte Bytefolge nicht finden"
"Nem találtam a bájtokat:"

MViewSearchFromBegin
"Продолжить поиск с начала документа?"
"Continue the search from the beginning of the document?"
"Pokračovat s hledáním od začátku dokumentu?"
"Mit Suche am Anfang des Dokuments fortfahren?"
"Folytassam a keresést a dokumentum elejétől?"

MViewSearchFromEnd
"Продолжить поиск с конца документа?"
"Continue the search from the end of the document?"
"Pokračovat s hledáním od konce dokumentu?"
"Mit Suche am Ende des Dokuments fortfahren?"
"Folytassam a keresést a dokumentum végétől?"

MPrintTitle
l:
"Печать"
"Print"
"Tisk"
"Drucken"
"Nyomtatás"

MPrintTo
"Печатать %s на"
"Print %s to"
"Vytisknout %s na"
"Drucke %s nach"
"%s nyomtatása:"

MPrintFilesTo
"Печатать %d файлов на"
"Print %d files to"
"Vytisknout %d souborů na"
"Drucke %d Dateien mit"
"%d fájl nyomtatása:"

MPreparingForPrinting
"Подготовка файлов к печати"
"Preparing files for printing"
"Připravuji soubory pro tisk"
"Vorbereiten der Druckaufträge"
"Fájlok előkészítése nyomtatáshoz"

MJobs
"заданий"
"jobs"
"úlohy"
"Aufträge"
"feladat"

MCannotOpenPrinter
"Не удалось открыть принтер"
"Cannot open printer"
"Nelze otevřít tiskárnu"
"Fehler beim öffnen des Druckers"
"Nyomtató nem elérhető"

MCannotPrint
"Не удалось распечатать"
"Cannot print"
"Nelze tisknout"
"Fehler beim Drucken"
"Nem nyomtatható"

MDescribeFiles
l:
"Описание файла"
"Describe file"
"Popiskový soubor"
"Beschreibung ändern"
"Fájlmegjegyzés"

MEnterDescription
"Введите описание %s"
"Enter %s description"
"Zadejte popisek %s"
"Beschreibung für %s:"
"Írja be %s megjegyzését:"

MReadingDiz
l:
"Чтение описаний файлов"
"Reading file descriptions"
"Načítám popisky souboru"
"Lese Dateibeschreibungen"
"Fájlmegjegyzések olvasása"

MCannotUpdateDiz
"Не удалось обновить описания файлов"
"Cannot update file descriptions"
"Nelze aktualizovat popisky souboru"
"Dateibeschreibungen konnten nicht aktualisiert werden."
"A fájlmegjegyzések nem frissíthetők"

MCannotUpdateRODiz
"Файл описаний защищен от записи"
"The description file is read only"
"Popiskový soubor má atribut Jen pro čtení"
"Die Beschreibungsdatei ist schreibgeschützt."
"A megjegyzésfájl csak olvasható"

MCfgDizTitle
l:
"Описания файлов"
"File descriptions"
"Popisky souboru"
"Dateibeschreibungen"
"Fájl megjegyzésfájlok"

MCfgDizListNames
"Имена &списков описаний, разделенные запятыми:"
"Description &list names delimited with commas:"
"Seznam pop&isových souborů oddělených čárkami:"
"Beschreibungs&dateien, getrennt durch Komma:"
"Megjegyzés&fájlok nevei, vesszővel elválasztva:"

MCfgDizSetHidden
"Устанавливать &атрибут ""Hidden"" на новые списки описаний"
"Set ""&Hidden"" attribute to new description lists"
"Novým souborům s popisy nastavit atribut ""&Skrytý"""
"Setze das '&Versteckt'-Attribut für neu angelegte Dateien"
"Az új megjegyzésfájl ""&rejtett"" attribútumú legyen"

MCfgDizROUpdate
"Обновлять файл описаний с атрибутом ""Толь&ко для чтения"""
"Update &read only description file"
"Aktualizovat popisové soubory s atributem Jen pro čtení"
"Schreibgeschützte Dateien aktualisie&ren"
"&Csak olvasható megjegyzésfájlok frissítése"

MCfgDizStartPos
"&Позиция новых описаний в строке"
"&Position of new descriptions in the string"
"&Pozice nových popisů v řetězci"
"&Position neuer Beschreibungen in der Zeichenkette"
"Új megjegyzéseknél a szöveg tá&volsága"

MCfgDizNotUpdate
"&Не обновлять описания"
"Do &not update descriptions"
"&Neaktualizovat popisy"
"Beschreibungen &nie aktualisieren"
"Ne &frissítse a megjegyzéseket"

MCfgDizUpdateIfDisplayed
"&Обновлять, если они выводятся на экран"
"Update if &displayed"
"Aktualizovat, jestliže je &zobrazen"
"Aktualisieren &wenn angezeigt"
"Frissítsen, ha meg&jelenik"

MCfgDizAlwaysUpdate
"&Всегда обновлять"
"&Always update"
"&Vždy aktualizovat"
"Im&mer aktualisieren"
"&Mindig frissítsen"

MReadingTitleFiles
l:
"Обновление панелей"
"Update of panels"
"Aktualizace panelů"
"Aktualisiere Panels"
"Panelek frissítése"

MReadingFiles
"Чтение: %d файлов"
"Reading: %d files"
"Načítám: %d souborů"
"Lese: %d Dateien"
" %d fájl olvasása"

MUserBreakTitle
l:
"Прекращено пользователем"
"User break"
"Přerušeno uživatelem"
"Unterbochen durch Benutzer"
"A felhasználó megszakította"

MOperationNotCompleted
"Операция не завершена"
"Operation not completed"
"Operace není dokončena"
"Vorgang nicht abgeschlossen"
"A művelet félbeszakadt"

MEditPanelModes
l:
"Режимы панели"
"Edit panel modes"
"Editovat módy panelu"
"Anzeigemodi von Panels bearbeiten"
"Panel módok szerkesztése"

MEditPanelModesBrief
l:
"&Краткий режим"
"&Brief mode"
"&Stručný mód"
"&Kurz"
"&Rövid mód"

MEditPanelModesMedium
"&Средний режим"
"&Medium mode"
"S&třední mód"
"&Mittel"
"&Közepes mód"

MEditPanelModesFull
"&Полный режим"
"&Full mode"
"&Plný mód"
"&Voll"
"&Teljes mód"

MEditPanelModesWide
"&Широкий режим"
"&Wide mode"
"Š&iroký mód"
"B&reitformat"
"&Széles mód"

MEditPanelModesDetailed
"&Детальный режим"
"Detai&led mode"
"Detai&lní mód"
"Detai&lliert"
"Rés&zletes mód"

MEditPanelModesDiz
"&Описания"
"&Descriptions mode"
"P&opiskový mód"
"&Beschreibungen"
"&Fájlmegjegyzés mód"

MEditPanelModesLongDiz
"Д&линные описания"
"Lon&g descriptions mode"
"&Mód dlouhých popisků"
"Lan&ge Beschreibungen"
"&Hosszú megjegyzés mód"

MEditPanelModesOwners
"Вл&адельцы файлов"
"File own&ers mode"
"Mód vlastníka so&uborů"
"B&esitzer"
"T&ulajdonos mód"

MEditPanelModesLinks
"Свя&зи файлов"
"Lin&ks mode"
"Lin&kový mód"
"Dateilin&ks"
"Li&nkek mód"

MEditPanelModesAlternative
"Аль&тернативный полный режим"
"&Alternative full mode"
"&Alternativní plný mód"
"&Alternative Vollansicht"
"&Alternatív teljes mód"

MEditPanelModeTypes
l:
"&Типы колонок"
"Column &types"
"&Typ sloupců"
"Spalten&typen"
"Oszlop&típusok"

MEditPanelModeWidths
"&Ширина колонок"
"Column &widths"
"Šíř&ka sloupců"
"Spalten&breiten"
"Oszlop&szélességek"

MEditPanelModeStatusTypes
"Типы колонок строки ст&атуса"
"St&atus line column types"
"T&yp sloupců stavového řádku"
"St&atuszeile Spaltentypen"
"Állapotsor oszloptíp&usok"

MEditPanelModeStatusWidths
"Ширина колонок строки стат&уса"
"Status l&ine column widths"
"Šířka slo&upců stavového řádku"
"Statusze&ile Spaltenbreiten"
"Állapotsor &oszlopszélességek"

MEditPanelModeFullscreen
"&Полноэкранный режим"
"&Fullscreen view"
"&Celoobrazovkový režim"
"&Vollbild"
"Tel&jes képernyős nézet"

MEditPanelModeAlignExtensions
"&Выравнивать расширения файлов"
"Align file &extensions"
"Zarovnat příp&ony souborů"
"Datei&erweiterungen ausrichten"
"Fájlkiterjesztések &igazítása"

MEditPanelModeAlignFolderExtensions
"Выравнивать расширения пап&ок"
"Align folder e&xtensions"
"Zarovnat přípony adre&sářů"
"Ordnerer&weiterungen ausrichten"
"Mappakiterjesztések i&gazítása"

MEditPanelModeFoldersUpperCase
"Показывать папки &заглавными буквами"
"Show folders in &uppercase"
"Zobrazit adresáře &velkými písmeny"
"Ordner in Großb&uchstaben zeigen"
"Mappák NAG&YBETŰVEL mutatva"

MEditPanelModeFilesLowerCase
"Показывать файлы ст&рочными буквами"
"Show files in &lowercase"
"Zobrazit soubory ma&lými písmeny"
"Dateien in K&leinbuchstaben zeigen"
"Fájlok kis&betűvel mutatva"

MEditPanelModeUpperToLowerCase
"Показывать имена файлов из заглавных букв &строчными буквами"
"Show uppercase file names in lower&case"
"Zobrazit velké znaky ve jménech souborů jako &malá písmena"
"G&roßgeschriebene Dateinamen in Kleinbuchstaben zeigen"
"NAGYBETŰS fájl&nevek kisbetűvel"

MEditPanelModeCaseSensitiveSort
"Использовать р&егистрозависимую сортировку"
"Use case &sensitive sort"
"Použít řazení citlivé na velikost pí&smen"
"&Sortierung abhängig von Groß-/Kleinschreibung"
"N&agy/kisbetű érzékeny rendezés"

MEditPanelReadHelp
" Нажмите F1, чтобы получить информацию по настройке "
" Read online help for instructions "
" Pro instrukce si přečtěte online nápovědu "
" Siehe Hilfe für Anweisungen "
" Tanácsokat a Súgóban talál (F1) "

MSetFolderInfoTitle
l:
"Файлы информации о папках"
"Folder description files"
"Soubory s popiskem adresáře"
"Ordnerbeschreibungen"
"Mappa megjegyzésfájlok"

MSetFolderInfoNames
"Введите имена файлов, разделенные запятыми (допускаются маски)"
"Enter file names delimited with commas (wildcards are allowed)"
"Zadejte jména souborů oddělených čárkami (značky jsou povoleny)"
"Dateiliste, getrennt mit Komma (Jokerzeichen möglich):"
"Fájlnevek, vesszővel elválasztva (joker is használható)"

MScreensTitle
l:
"Экраны"
"Screens"
"Obrazovky"
"Seiten"
"Képernyők"

MScreensPanels
"Панели"
"Panels"
"Panely"
"Panels"
"Panelek"

MScreensView
"Просмотр"
"View"
"Zobrazit"
"Betr."
"Nézőke"

MScreensEdit
"Редактор"
"Edit"
"Editovat"
"Bearb"
"Szerkesztő"

MAskApplyCommandTitle
l:
"Применить команду"
"Apply command"
"Aplikovat příkaz"
"Befehl anwenden"
"Parancs végrehajtása"

MAskApplyCommand
"Введите команду для обработки выбранных файлов"
"Enter command to process selected files"
"Zadejte příkaz pro zpracování vybraných souborů"
"Befehlszeile auf ausgewählte Dateien anwenden:"
"Írja be a kijelölt fájlok parancsát:"

MPluginConfigTitle
l:
"Конфигурация модулей"
"Plugins configuration"
"Nastavení Pluginů"
"Konfiguration von Plugins"
"Plugin beállítások"

MPluginCommandsMenuTitle
"Команды внешних модулей"
"Plugin commands"
"Příkazy pluginů"
"Pluginbefehle"
"Plugin parancsok"

MPreparingList
l:
"Создание списка файлов"
"Preparing files list"
"Připravuji seznam souborů"
"Dateiliste wird vorbereitet"
"Fájllista elkészítése"

MLangTitle
l:
"Основной язык"
"Main language"
"Hlavní jazyk"
"Hauptsprache"
"A program nyelve"

MHelpLangTitle
"Язык помощи"
"Help language"
"Jazyk nápovědy"
"Sprache der Hilfedatei"
"A Súgó nyelve"

MDefineMacroTitle
l:
"Задание макрокоманды"
"Define macro"
"Definovat makro"
"Definiere Makro"
"Makró gyorsbillentyű"

MDefineMacro
"Нажмите желаемую клавишу"
"Press the desired key"
"Stiskněte požadovanou klávesu"
"Tastenkombination:"
"Nyomja le a billentyűt!"

MMacroReDefinedKey
"Макроклавиша '%s' уже определена."
"Macro key '%s' already defined."
"Klávesa makra '%s' již je definována."
"Makro '%s' bereits definiert."
""%s" makróbillentyű foglalt"

MMacroDeleteAssign
"Макроклавиша '%s' не активна."
"Macro key '%s' is not active."
"Klávesa makra '%s' není aktivní."
"Makro '%s' nicht aktiv."
""%s" makróbillentyű nem él"

MMacroDeleteKey
"Макроклавиша '%s' будет удалена."
"Macro key '%s' will be removed."
"Klávesa makra '%s' bude odstraněna."
"Makro '%s' wird entfernt und ersetzt:"
""%s" makróbillentyű törlődik"

MMacroCommonReDefinedKey
"Общая макроклавиша '%s' уже определена."
"Common macro key '%s' already defined."
"Klávesa pro běžné makro '%s' již je definována."
"Gemeinsames Makro '%s' bereits definiert."
""%s" közös makróbill. foglalt"

MMacroCommonDeleteAssign
"Общая макроклавиша '%s' не активна."
"Common macro key '%s' is not active."
"Klávesa pro běžné makro '%s' není aktivní."
"Gemeinsames Makro '%s' nicht aktiv."
""%s" közös makróbill. nem él"

MMacroCommonDeleteKey
"Общая макроклавиша '%s' будет удалена."
"Common macro key '%s' will be removed."
"Klávesa pro běžné makro '%s' bude odstraněna."
"Gemeinsames Makro '%s' wird entfernt und ersetzt:"
""%s" közös makróbill. törlődik"

MMacroSequence
"Последовательность:"
"Sequence:"
"Posloupnost:"
"Sequenz:"
"Szekvencia:"

MMacroReDefinedKey2
"Переопределить?"
"Redefine?"
"Předefinovat?"
"Neu definieren?"
"Újradefiniálja?"

MMacroDeleteKey2
"Удалить?"
"Delete?"
"Odstranit?"
"Löschen?"
"Törli?"

MMacroDisDisabledKey
"(макроклавиша не активна)"
"(macro key is not active)"
"(klávesa makra není aktivní)"
"(Makro inaktiv)"
"(makróbill. nem él)"

MMacroDisOverwrite
"Переопределить"
"Overwrite"
"Přepsat"
"Überschreiben"
"Felülírás"

MMacroDisAnotherKey
"Изменить клавишу"
"Try another key"
"Zkusit jinou klávesu"
"Neue Kombination"
"Adjon meg másik billentyűt!"

MMacroSettingsTitle
l:
"Параметры макрокоманды для '%s'"
"Macro settings for '%s'"
"Nastavení makra pro '%s'"
"Einstellungen für Makro '%s'"
""%s" makró beállításai"

MMacroSettingsEnableOutput
"Разрешить во время &выполнения вывод на экран"
"Allo&w screen output while executing macro"
"Povolit &výstup na obrazovku dokud se provádí makro"
"Bildschirmausgabe &während Makro abläuft"
"Képernyő&kimenet a makró futása közben"

MMacroSettingsRunAfterStart
"В&ыполнять после запуска FAR"
"Execute after FAR &start"
"&Spustit po spuštění FARu"
"Ausführen beim &Starten von FAR"
"Végrehajtás a FAR &indítása után"

MMacroSettingsActivePanel
"&Активная панель"
"&Active panel"
"&Aktivní panel"
"&Aktives Panel"
"&Aktív panel"

MMacroSettingsPassivePanel
"&Пассивная панель"
"&Passive panel"
"Pa&sivní panel"
"&Passives Panel"
"Passzí&v panel"

MMacroSettingsPluginPanel
"На панели пла&гина"
"P&lugin panel"
"Panel p&luginů"
"P&lugin Panel"
"Ha &plugin panel"

MMacroSettingsFolders
"Выполнять для папо&к"
"Execute for &folders"
"Spustit pro ad&resáře"
"Auf Ordnern aus&führen"
"Ha &mappa"

MMacroSettingsSelectionPresent
"&Отмечены файлы"
"Se&lection present"
"E&xistující výběr"
"Auswah&l vorhanden"
"Ha van ki&jelölés"

MMacroSettingsCommandLine
"Пустая командная &строка"
"Empty &command line"
"Prázdný pří&kazový řádek"
"Leere Befehls&zeile"
"Ha &üres a parancssor"

MMacroSettingsSelectionBlockPresent
"Отмечен б&лок"
"Selection &block present"
"Existující blok výběr&u"
"Mar&kierter Text vorhanden"
"Ha van kijelölt &blokk"

MMacroPErrUnrecognized_keyword
l:
"Неизвестное ключевое слово '%s'"
"Unrecognized keyword '%s'"
"Neznámé klíčové slovo '%s'"
"Unbekanntes Schlüsselwort '%s'"
"Ismeretlen kulcsszó "%s""

MMacroPErrUnrecognized_function
"Неизвестная функция '%s'"
"Unrecognized function '%s'"
"Neznámá funkce '%s'"
"Unbekannte Funktion '%s'"
"Ismeretlen funkció "%s""

MMacroPErrNot_expected_ELSE
"Неожиданное появление $Else"
"Unexpected $Else"
"Neočekávané $Else"
"Unerwartetes $Else"
"Váratlan $Else"

MMacroPErrNot_expected_END
"Неожиданное появление $End"
"Unexpected $End"
"Neočekávané $End"
"Unerwartetes $End"
"Váratlan $End"

MMacroPErrUnexpected_EOS
"Неожиданный конец строки"
"Unexpected end of source string"
"Neočekávaný konec zdrojového řetězce"
"Unerwartetes Ende der Zeichenkette"
"Váratlanul vége a forrássztringnek"

MMacroPErrExpected
"Ожидается '%s'"
"Expected '%s'"
"Očekávané '%s'"
"Erwartet '%s'"
"Várható "%s""

MMacroPErrBad_Hex_Control_Char
"Неизвестный шестнадцатеричный управляющий символ"
"Bad Hex Control Char"
"Chybný kontrolní znak Hex"
"Fehlerhaftes Hexzeichen"
"Rossz hexa vezérlőkarakter"

MMacroPErrBad_Control_Char
"Неправильный управляющий символ"
"Bad Control Char"
"Špatný kontrolní znak"
"Fehlerhaftes Kontrollzeichen"
"Rossz vezérlőkarakter"

MMacroPErrVar_Expected
"Переменная '%s' не найдена"
"Variable Expected '%s'"
"Očekávaná proměnná '%s'"
"Variable erwartet '%s'"
""%s" várható változó"

MMacroPErrExpr_Expected
"Ошибка синтаксиса"
"Expression Expected"
"Očekávaný výraz"
"Ausdruck erwartet"
"Szintaktikai hiba"

MCannotSaveFile
l:
"Ошибка сохранения файла"
"Cannot save file"
"Nelze uložit soubor"
"Kann Datei nicht speichern"
"A fájl nem menthető"

MTextSavedToTemp
"Отредактированный текст записан в"
"Edited text is stored in"
"Editovaný text je uložen v"
"Editierter Text ist gespeichert in"
"A szerkesztett szöveg elmentve:"

MMonthJan
l:
"Янв"
"Jan"
"Led"
"Jan"
"Jan"

MMonthFeb
"Фев"
"Feb"
"Úno"
"Feb"
"Feb"

MMonthMar
"Мар"
"Mar"
"Bře"
"Mär"
"Már"

MMonthApr
"Апр"
"Apr"
"Dub"
"Apr"
"Ápr"

MMonthMay
"Май"
"May"
"Kvě"
"Mai"
"Máj"

MMonthJun
"Июн"
"Jun"
"Čer"
"Jun"
"Jún"

MMonthJul
"Июл"
"Jul"
"Čec"
"Jul"
"Júl"

MMonthAug
"Авг"
"Aug"
"Srp"
"Aug"
"Aug"

MMonthSep
"Сен"
"Sep"
"Zář"
"Sep"
"Sze"

MMonthOct
"Окт"
"Oct"
"Říj"
"Okt"
"Okt"

MMonthNov
"Ноя"
"Nov"
"Lis"
"Nov"
"Nov"

MMonthDec
"Дек"
"Dec"
"Pro"
"Dez"
"Dec"

MPluginHotKeyTitle
l:
"Назначение горячей клавиши"
"Assign plugin hot key"
"Přidělit horkou klávesu pluginu"
"Dem Plugin eine Kurztaste zuweisen"
"Plugin gyorsbillentyű hozzárendelés"

MPluginHotKey
"Введите горячую клавишу (букву или цифру)"
"Enter hot key (letter or digit)"
"Zadejte horkou klávesu (písmeno nebo číslici)"
"Buchstabe oder Ziffer:"
"Nyomja le a billentyűt (betű vagy szám)!"

MPluginHotKeyBottom
"F4 - задать горячую клавишу"
"F4 - set hot key"
"F4 - nastavení horké klávesy"
"Kurztaste setzen: F4"
"F4 - gyorsbillentyű hozzárendelés"

MRightCtrl
l:
"ПравыйCtrl"
"RightCtrl"
"PravýCtrl"
"StrgRechts"
"JobbCtrl"

MViewerGoTo
l:
"Перейти"
"Go to"
"Jdi na"
"Gehe zu"
"Ugrás"

MGoToPercent
"&Процент"
"&Percent"
"&Procent"
"&Prozent"
"&Százalékban"

MGoToHex
"16-ричное &смещение"
"&Hex offset"
"&Hex offset"
"Position (&Hex)"
"&Hexában"

MGoToDecimal
"10-ичное с&мещение"
"&Decimal offset"
"&Desítkový offset"
"Position (&dezimal)"
"&Decimálisan"

MExceptTitleFAR
l:
"Внутренняя ошибка"
"Internal error"
"Vnitřní chyba"
"Interner Fehler"
"Belső hiba"

MExceptTitleLoad
"Ошибка загрузки плагина"
"Plugin load error"
"Chyba při načítání pluginu"
"Fehler beim Laden des Plugins"
"Plugin betöltési hiba"

MExceptTitle
"Ошибка вызова плагина"
"Plugin call error"
"Chyba při volání pluginu"
"Fehler beim Starten des Plugins"
"Plugin meghívási hiba"

MExcTrappedException
"Исключительная ситуация:"
"Exception occurred:"
"Vyskytla se výjimka:"
"Ausnahmefehler aufgetreten:"
"Kivétel történt:"

MExcCheckOnLousys
"Передана некорректная информация из модуля:"
"Incorrect information is passed from module:"
"Z modulu byla obdržena nekorektní informace:"
"Ungültige Informationen übergeben durch Modul:"
"Hibás információ jött a plugintől:"

MExcStructWrongFilled
"(некорректно заполнены поля структуры <%s>)"
"(the fields of structure <%s> are wrong filled)"
"(pole struktur <%s> jsou špatně vyplněna)"
"(Felder der Struktur <%s> wurden fehlerhaft gefüllt)"
"(<%s> struktúra mezői rosszul vannak kitöltve)"

MExcStructField
"(структура <%s>, поле <%s>)"
"(structure <%s>, field <%s>)"
"(struktura <%s>, položka <%s>)"
"(Struktur <%s>, Feld <%s>)"
"(<%s> struktúra, <%s> mező)"

MExcInvalidFuncResult
"Функция <%s> вернула недопустимое значение"
"Function <%s> has returned illegal value"
"Funkce <%s> vrátila nepovolenou hodnotu"
"Funktion <%s> lieferte ungültigen Rückgabewert"
"<%s> funkció érvénytelen értéket adott vissza"

MExcAddress
"Адрес исключения - 0x%p, модуль:"
"Exception address: 0x%p in module:"
"Výjimka na adrese: 0x%X v modulu:"
"Adresse des Fehlers: 0x%p in Modul:"
"Kivétel címe 0x%p, modul:"

MExcFARTerminateYes
"FAR Manager завершит работу!"
"FAR Manager will be terminated!"
"FAR Manager bude ukončen!"
"FAR Manager wird jetzt beendet!"
"A FAR Manager kilép!"

MExcUnloadYes
"Плагин будет выгружен!"
"The plugin will be Unloaded!"
"Plugin bude vyřazen!"
"Das Plugin wird jetzt entladen!"
"A plugin törlődik a memóriából!"

MExcRAccess
"\"Нарушение доступа (чтение из 0x%p)\""
"\"Access violation (read from 0x%p)\""
"\"Neplatná adresa (čtení z 0x%p)\""
"\"Zugriffsverletzung (Lesen von 0x%p)\""
"\"Hozzáférési jogsértés (olvasás 0x%p címről)\""

MExcWAccess
"\"Нарушение доступа (запись в 0x%p)\""
"\"Access violation (write to 0x%p)\""
"\"Neplatná adresa (zápis na 0x%p)\""
"\"Zugriffsverletzung (Schreiben nach 0x%p)\""
"\"Hozzáférési jogsértés (írás 0x%p címre)\""

MExcEAccess
"\"Нарушение доступа (исполнение кода из 0x%p)\""
"\"Access violation (execute at 0x%p)\""
"\"Neplatná adresa (spuštění na 0x%p)\""
"\"Zugriffsverletzung (Ausführen bei 0x%p)\""
"\"Hozzáférési jogsértés (végrehajtás 0x%p címen)\""

MExcOutOfBounds
"\"Попытка доступа к элементу за границами массива\""
"\"Array out of bounds\""
"\"Pole mimo hranice\""
"\"Arrayüberlauf\""
"\"A tömb határait meghaladta\""

MExcDivideByZero
"\"Деление на нуль\""
"\"Divide by zero\""
"\"Dělení nulou\""
"\"Division durch Null\""
"\"Nullával osztás\""

MExcStackOverflow
"\"Переполнение стека\""
"\"Stack Overflow\""
"\"Přetečení zásobníku\""
"\"Stacküberlauf\""
"\"Verem túlcsordulás\""

MExcBreakPoint
"\"Точка останова\""
"\"Breakpoint exception\""
"\"Výjimka přerušení\""
"\"Breakpoint exception\""
"\"Törésponti kivétel\""

MExcFloatDivideByZero
"\"Деление на нуль при операции с плавающей точкой\""
"\"Floating-point divide by zero\""
"\"Dělení nulou v pohyblivé čárce\""
"\"Fließkomma-Division durch Null\""
"\"Lebegőpontos szám osztása nullával\""

MExcFloatOverflow
"\"Переполнение при операции с плавающей точкой\""
"\"Floating point operation overflow\""
"\"Přetečení při operaci v pohyblivé čárce\""
"\"Fließkomma-Operation verursachte Überlauf\""
"\"Lebegőpontos művelet túlcsordulás\""

MExcFloatStackOverflow
"\"Стек регистров сопроцессора полон или пуст\""
"\"Floating point stack empty or full\""
"\"Prázdný nebo plný zásobník v pohyblivé čárce\""
"\"Fließkomma-Stack leer bzw. voll\""
"\"Lebegőpont verem üres vagy megtelt\""

MExcFloatUnderflow
"\"Потеря точности при операции с плавающей точкой\""
"\"Floating point operation underflow\""
"\"Podtečení při operaci v pohyblivé čárce\""
"\"Fließkomma-Operation verursachte Underflow\""
"\"Lebegőpontos művelet alulcsordulás\""

MExcBadInstruction
"\"Недопустимая инструкция\""
"\"Illegal instruction\""
"\"Neplatná instrukce\""
"\"Ungültige Anweisung\""
"\"Érvénytelen utasítás\""

MExcDatatypeMisalignment
"\"Попытка доступа к невыравненным данным\""
"\"Alignment fault\""
"\"Chyba zarovnání\""
"\"Fehler bei Datenausrichtung\""
"\"Adattípus illesztési hiba\""

MExcUnknown
"\"Неизвестное исключение\""
"\"Unknown exception\""
"\"Neznámá výjimka\""
"\"Unbekannte Ausnahme\""
"\"Ismeretlen kivétel\""

MExcDebugger
"Отладчик"
"Debugger"
"Debugger"
"Debugger"
"Debugger"

MNetUserName
l:
"Имя пользователя"
"User name"
"Jméno uživatele"
"Benutzername"
"Felhasználói név"

MNetUserPassword
"Пароль пользователя"
"User password"
"Heslo uživatele"
"Benutzerpasswort"
"Felhasználói jelszó"

MReadFolderError
l:
"Не удается прочесть содержимое папки"
"Cannot read folder contents"
"Nelze načíst obsah adresáře"
"Kann Ordnerinhalt nicht lesen"
"A mappa tartalma nem olvasható"

MPlgBadVers
l:
"Этот модуль требует FAR более высокой версии"
"This plugin requires higher FAR version"
"Tento plugin vyžaduje vyšší verzi FARu"
"Das Plugin benötigt eine aktuellere Version von FAR"
"A pluginhez újabb FAR verzió kell"

MPlgRequired
"Требуется версия FAR - %d.%d.%d."
"Required FAR version is %d.%d.%d."
"Požadovaná verze FARu je %d.%d.%d."
"Benötigte FAR-Version ist %d.%d.%d."
"A szükséges FAR verzió: %d.%d.%d."

MPlgRequired2
"Текущая версия FAR - %d.%d.%d."
"Current FAR version is %d.%d.%d"
"Nynější verze FARu je %d.%d.%d"
"Aktuelle FAR-Version ist %d.%d.%d"
"A jelenlegi FAR verzió: %d.%d.%d."

MPlgLoadPluginError
"Ошибка при загрузке плагина"
"Error loading plugin module"
"Chyba při nahrávání zásuvného modulu"
"Fehler beim Laden des Pluginmoduls"
"Plugin betöltési hiba"

MBuffSizeTooSmall_1
l:
"Буфер, размещенный под имя файла слишком мал."
"Buffer allocated for file name is too small."
"Buffer alokovaný pro jméno souboru je příliš malý."
"Reservierter Puffer für Dateiname ist zu klein."
"A fájlnév puffere túl kicsi."

MBuffSizeTooSmall_2
"Требуется %d байт, а имеется только %d"
"%d bytes are required, but only %d bytes were allocated."
"Požadováno %d bytů, ale alokováno pouze %d."
"%d Bytes werden benötigt aber nur %d Bytes wurden reserviert."
"%d bájt kell, de csak %d van lefoglalva."

MCheckBox2State
l:
"?"
"?"
"?"
"?"
"?"

MEditInputSize1
"Длина поля"
"Field"
"Pole"
"Feld"
"A mező"

MEditInputSize2
"будет уменьшена до %d байт."
"will be truncated to %d bytes."
"bude oseknuto na %d bytů."
"wird gekürzt auf %d Bytes."
" %d bájtra rövidül."

MHelpTitle
l:
"Помощь"
"Help"
"Nápověda"
"Hilfe"
"Súgó"

MHelpActivatorURL
"Эта ссылка запускает внешнее приложение:"
"This reference starts the external application:"
"Tento odkaz spouští externí aplikaci:"
"Diese Referenz startet folgendes externes Programm:"
"A hivatkozás által indított program:"

MHelpActivatorFormat
"с параметром:"
"with parameter:"
"s parametrem:"
"mit Parameter:"
"Paraméterei:"

MHelpActivatorQ
"Желаете запустить?"
"Do you wish to start it?"
"Přejete si ji spustit?"
"Wollen Sie jetzt starten?"
"El akarja indítani?"

MCannotOpenHelp
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"
"A fájl nem nyitható meg"

MHelpTopicNotFound
"Не найден запрошенный раздел помощи:"
"Requested help topic not found:"
"požadované téma nápovědy nebylo nalezeno"
"Angefordertes Hilfethema wurde nicht gefunden:"
"A kívánt Súgó témakör nem található:"

MPluginsHelpTitle
l:
"Внешние модули"
"Plugins help"
"Nápověda Pluginů"
"Pluginhilfe"
"Plugin Súgó"

MDocumentsHelpTitle
"Документы"
"Documents help"
"Nápověda Dokumentů"
"Dokumentenhilfe"
"Dokumentum Súgó"

MHelpSearchTitle
l:
"Поиск"
"Search"
"Hledání"
"Suchen"
"Keresés"

MHelpSearchingFor
"Поиск для"
"Searching for"
"Hledání"
"Suche nach"
"Keresés:"

MHelpSearchCannotFind
"Строка не найдена"
"Could not find the string"
"Nelze najít řetězec"
"Konnte Zeichenkette nicht finden"
"A szöveg nem található:"

MHelpF1
l:
l:// Help KeyBar F1-12
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Súgó"

MHelpF2
""
""
""
""
""

MHelpF3
""
""
""
""
""

MHelpF4
""
""
""
""
""

MHelpF5
"Размер"
"Zoom"
"Zoom"
"Vergr."
"Nagyít"

MHelpF6
""
""
""
""
""

MHelpF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"

MHelpF8
""
""
""
""
""

MHelpF9
""
""
""
""
""

MHelpF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"

MHelpF11
""
""
""
""
""

MHelpF12
""
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
"Tartlm"

MHelpShiftF2
"Плагин"
"Plugin"
"Plugin"
"Plugin"
"PlgSúg"

MHelpShiftF3
"Докум"
"Docums"
"Dokume"
"Dokume"
"DokSúg"

MHelpShiftF4
""
""
""
""
""

MHelpShiftF5
""
""
""
""
""

MHelpShiftF6
""
""
""
""
""

MHelpShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"Tovább"

MHelpShiftF8
""
""
""
""
""

MHelpShiftF9
""
""
""
""
""

MHelpShiftF10
""
""
""
""
""

MHelpShiftF11
""
""
""
""
""

MHelpShiftF12
""
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
"Vissza"

MHelpAltF2
""
""
""
""
""

MHelpAltF3
""
""
""
""
""

MHelpAltF4
""
""
""
""
""

MHelpAltF5
""
""
""
""
""

MHelpAltF6
""
""
""
""
""

MHelpAltF7
""
""
""
""
""

MHelpAltF8
""
""
""
""
""

MHelpAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"

MHelpAltF10
""
""
""
""
""

MHelpAltF11
""
""
""
""
""

MHelpAltF12
""
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
""

MHelpCtrlF2
""
""
""
""
""

MHelpCtrlF3
""
""
""
""
""

MHelpCtrlF4
""
""
""
""
""

MHelpCtrlF5
""
""
""
""
""

MHelpCtrlF6
""
""
""
""
""

MHelpCtrlF7
""
""
""
""
""

MHelpCtrlF8
""
""
""
""
""

MHelpCtrlF9
""
""
""
""
""

MHelpCtrlF10
""
""
""
""
""

MHelpCtrlF11
""
""
""
""
""

MHelpCtrlF12
""
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
""

MHelpCtrlShiftF2
""
""
""
""
""

MHelpCtrlShiftF3
""
""
""
""
""

MHelpCtrlShiftF4
""
""
""
""
""

MHelpCtrlShiftF5
""
""
""
""
""

MHelpCtrlShiftF6
""
""
""
""
""

MHelpCtrlShiftF7
""
""
""
""
""

MHelpCtrlShiftF8
""
""
""
""
""

MHelpCtrlShiftF9
""
""
""
""
""

MHelpCtrlShiftF10
""
""
""
""
""

MHelpCtrlShiftF11
""
""
""
""
""

MHelpCtrlShiftF12
""
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
""

MHelpCtrlAltF2
""
""
""
""
""

MHelpCtrlAltF3
""
""
""
""
""

MHelpCtrlAltF4
""
""
""
""
""

MHelpCtrlAltF5
""
""
""
""
""

MHelpCtrlAltF6
""
""
""
""
""

MHelpCtrlAltF7
""
""
""
""
""

MHelpCtrlAltF8
""
""
""
""
""

MHelpCtrlAltF9
""
""
""
""
""

MHelpCtrlAltF10
""
""
""
""
""

MHelpCtrlAltF11
""
""
""
""
""

MHelpCtrlAltF12
""
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
""

MHelpAltShiftF2
""
""
""
""
""

MHelpAltShiftF3
""
""
""
""
""

MHelpAltShiftF4
""
""
""
""
""

MHelpAltShiftF5
""
""
""
""
""

MHelpAltShiftF6
""
""
""
""
""

MHelpAltShiftF7
""
""
""
""
""

MHelpAltShiftF8
""
""
""
""
""

MHelpAltShiftF9
""
""
""
""
""

MHelpAltShiftF10
""
""
""
""
""

MHelpAltShiftF11
""
""
""
""
""

MHelpAltShiftF12
""
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
""

MHelpCtrlAltShiftF2
""
""
""
""
""

MHelpCtrlAltShiftF3
""
""
""
""
""

MHelpCtrlAltShiftF4
""
""
""
""
""

MHelpCtrlAltShiftF5
""
""
""
""
""

MHelpCtrlAltShiftF6
""
""
""
""
""

MHelpCtrlAltShiftF7
""
""
""
""
""

MHelpCtrlAltShiftF8
""
""
""
""
""

MHelpCtrlAltShiftF9
""
""
""
""
""

MHelpCtrlAltShiftF10
""
""
""
""
""

MHelpCtrlAltShiftF11
""
""
""
""
""

MHelpCtrlAltShiftF12
""
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
"Súgó"

MInfoF2
"Сверн"
"Wrap"
"Zalam"
"Umbr."
"SorTör"

MInfoF3
"СмОпис"
"VieDiz"
"Zobraz"
"BetDiz"
"MjMnéz"

MInfoF4
"РедОпи"
"EdtDiz"
"Edit"
"BeaDiz"
"MjSzrk"

MInfoF5
""
""
""
""
""

MInfoF6
""
""
""
""
""

MInfoF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"

MInfoF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MInfoF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"
"KonfMn"

MInfoF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"

MInfoF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MInfoF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"

MInfoShiftF1
l:
l:// InfoPanel KeyBar Shift-F1-F12
""
""
""
""
""

MInfoShiftF2
"Слова"
"WWrap"
"ZalSlo"
"WUmbr"
"SzóTör"

MInfoShiftF3
""
""
""
""
""

MInfoShiftF4
""
""
""
""
""

MInfoShiftF5
""
""
""
""
""

MInfoShiftF6
""
""
""
""
""

MInfoShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"

MInfoShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"

MInfoShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"

MInfoShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"
"UtsMnü"

MInfoShiftF11
""
""
""
""
""

MInfoShiftF12
""
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
"Bal"

MInfoAltF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"

MInfoAltF3
""
""
""
""
""

MInfoAltF4
""
""
""
""
""

MInfoAltF5
""
""
""
""
""

MInfoAltF6
""
""
""
""
""

MInfoAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"

MInfoAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"

MInfoAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"

MInfoAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"

MInfoAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"

MInfoAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"
"MapElő"

MInfoCtrlF1
l:
l:// InfoPanel KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"
"Links"
"Bal"

MInfoCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"

MInfoCtrlF3
""
""
""
""
""

MInfoCtrlF4
""
""
""
""
""

MInfoCtrlF5
""
""
""
""
""

MInfoCtrlF6
""
""
""
""
""

MInfoCtrlF7
""
""
""
""
""

MInfoCtrlF8
""
""
""
""
""

MInfoCtrlF9
""
""
""
""
""

MInfoCtrlF10
""
""
""
""
""

MInfoCtrlF11
""
""
""
""
""

MInfoCtrlF12
""
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
""

MInfoCtrlShiftF2
""
""
""
""
""

MInfoCtrlShiftF3
""
""
""
""
""

MInfoCtrlShiftF4
""
""
""
""
""

MInfoCtrlShiftF5
""
""
""
""
""

MInfoCtrlShiftF6
""
""
""
""
""

MInfoCtrlShiftF7
""
""
""
""
""

MInfoCtrlShiftF8
""
""
""
""
""

MInfoCtrlShiftF9
""
""
""
""
""

MInfoCtrlShiftF10
""
""
""
""
""

MInfoCtrlShiftF11
""
""
""
""
""

MInfoCtrlShiftF12
""
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
""

MInfoCtrlAltF2
""
""
""
""
""

MInfoCtrlAltF3
""
""
""
""
""

MInfoCtrlAltF4
""
""
""
""
""

MInfoCtrlAltF5
""
""
""
""
""

MInfoCtrlAltF6
""
""
""
""
""

MInfoCtrlAltF7
""
""
""
""
""

MInfoCtrlAltF8
""
""
""
""
""

MInfoCtrlAltF9
""
""
""
""
""

MInfoCtrlAltF10
""
""
""
""
""

MInfoCtrlAltF11
""
""
""
""
""

MInfoCtrlAltF12
""
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
""

MInfoAltShiftF2
""
""
""
""
""

MInfoAltShiftF3
""
""
""
""
""

MInfoAltShiftF4
""
""
""
""
""

MInfoAltShiftF5
""
""
""
""
""

MInfoAltShiftF6
""
""
""
""
""

MInfoAltShiftF7
""
""
""
""
""

MInfoAltShiftF8
""
""
""
""
""

MInfoAltShiftF9
""
""
""
""
""

MInfoAltShiftF10
""
""
""
""
""

MInfoAltShiftF11
""
""
""
""
""

MInfoAltShiftF12
""
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
""

MInfoCtrlAltShiftF2
""
""
""
""
""

MInfoCtrlAltShiftF3
""
""
""
""
""

MInfoCtrlAltShiftF4
""
""
""
""
""

MInfoCtrlAltShiftF5
""
""
""
""
""

MInfoCtrlAltShiftF6
""
""
""
""
""

MInfoCtrlAltShiftF7
""
""
""
""
""

MInfoCtrlAltShiftF8
""
""
""
""
""

MInfoCtrlAltShiftF9
""
""
""
""
""

MInfoCtrlAltShiftF10
""
""
""
""
""

MInfoCtrlAltShiftF11
""
""
""
""
""

MInfoCtrlAltShiftF12
""
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
"Súgó"

MQViewF2
"Сверн"
"Wrap"
"Zalam"
"Umbr."
"SorTör"

MQViewF3
"Просм"
"View"
"Zobraz"
"Betr."
"Megnéz"

MQViewF4
"Код"
"Hex"
"Hex"
"Hex"
"Hexa"

MQViewF5
""
""
""
""
""

MQViewF6
""
""
""
""
""

MQViewF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"

MQViewF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"

MQViewF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"
"KonfMn"

MQViewF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"

MQViewF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MQViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"

MQViewShiftF1
l:
l:// QView KeyBar Shift-F1-F12
""
""
""
""
""

MQViewShiftF2
"Слова"
"WWrap"
"ZalSlo"
"WUmbr"
"SzóTör"

MQViewShiftF3
""
""
""
""
""

MQViewShiftF4
""
""
""
""
""

MQViewShiftF5
""
""
""
""
""

MQViewShiftF6
""
""
""
""
""

MQViewShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"

MQViewShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"

MQViewShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"

MQViewShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"
"UtsMnü"

MQViewShiftF11
""
""
""
""
""

MQViewShiftF12
""
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
"Bal"

MQViewAltF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"

MQViewAltF3
""
""
""
""
""

MQViewAltF4
""
""
""
""
""

MQViewAltF5
""
""
""
""
""

MQViewAltF6
""
""
""
""
""

MQViewAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"

MQViewAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"

MQViewAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"

MQViewAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"

MQViewAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"

MQViewAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"
"MapElő"

MQViewCtrlF1
l:
l:// QView KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"
"Links"
"Bal"

MQViewCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"

MQViewCtrlF3
""
""
""
""
""

MQViewCtrlF4
""
""
""
""
""

MQViewCtrlF5
""
""
""
""
""

MQViewCtrlF6
""
""
""
""
""

MQViewCtrlF7
""
""
""
""
""

MQViewCtrlF8
""
""
""
""
""

MQViewCtrlF9
""
""
""
""
""

MQViewCtrlF10
""
""
""
""
""

MQViewCtrlF11
""
""
""
""
""

MQViewCtrlF12
""
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
""

MQViewCtrlShiftF2
""
""
""
""
""

MQViewCtrlShiftF3
""
""
""
""
""

MQViewCtrlShiftF4
""
""
""
""
""

MQViewCtrlShiftF5
""
""
""
""
""

MQViewCtrlShiftF6
""
""
""
""
""

MQViewCtrlShiftF7
""
""
""
""
""

MQViewCtrlShiftF8
""
""
""
""
""

MQViewCtrlShiftF9
""
""
""
""
""

MQViewCtrlShiftF10
""
""
""
""
""

MQViewCtrlShiftF11
""
""
""
""
""

MQViewCtrlShiftF12
""
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
""

MQViewCtrlAltF2
""
""
""
""
""

MQViewCtrlAltF3
""
""
""
""
""

MQViewCtrlAltF4
""
""
""
""
""

MQViewCtrlAltF5
""
""
""
""
""

MQViewCtrlAltF6
""
""
""
""
""

MQViewCtrlAltF7
""
""
""
""
""

MQViewCtrlAltF8
""
""
""
""
""

MQViewCtrlAltF9
""
""
""
""
""

MQViewCtrlAltF10
""
""
""
""
""

MQViewCtrlAltF11
""
""
""
""
""

MQViewCtrlAltF12
""
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
""

MQViewAltShiftF2
""
""
""
""
""

MQViewAltShiftF3
""
""
""
""
""

MQViewAltShiftF4
""
""
""
""
""

MQViewAltShiftF5
""
""
""
""
""

MQViewAltShiftF6
""
""
""
""
""

MQViewAltShiftF7
""
""
""
""
""

MQViewAltShiftF8
""
""
""
""
""

MQViewAltShiftF9
""
""
""
""
""

MQViewAltShiftF10
""
""
""
""
""

MQViewAltShiftF11
""
""
""
""
""

MQViewAltShiftF12
""
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
""

MQViewCtrlAltShiftF2
""
""
""
""
""

MQViewCtrlAltShiftF3
""
""
""
""
""

MQViewCtrlAltShiftF4
""
""
""
""
""

MQViewCtrlAltShiftF5
""
""
""
""
""

MQViewCtrlAltShiftF6
""
""
""
""
""

MQViewCtrlAltShiftF7
""
""
""
""
""

MQViewCtrlAltShiftF8
""
""
""
""
""

MQViewCtrlAltShiftF9
""
""
""
""
""

MQViewCtrlAltShiftF10
""
""
""
""
""

MQViewCtrlAltShiftF11
""
""
""
""
""

MQViewCtrlAltShiftF12
""
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
"Súgó"

MKBTreeF2
"ПользМ"
"UserMn"
"UživMn"
"BenuMn"
"FelhMn"

MKBTreeF3
""
""
""
""
""

MKBTreeF4
"Атриб"
"Attr"
"Attr"
"Attr"
"Attrib"

MKBTreeF5
"Копир"
"Copy"
"Kopír."
"Kopier"
"Másol"

MKBTreeF6
"Перен"
"RenMov"
"PřjPřs"
"RenMov"
"ÁtnMoz"

MKBTreeF7
"Папка"
"MkFold"
"VytAdr"
"VerzEr"
"ÚjMapp"

MKBTreeF8
"Удален"
"Delete"
"Smazat"
"Lösch"
"Törlés"

MKBTreeF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"
"KonfMn"

MKBTreeF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"

MKBTreeF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MKBTreeF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"

MKBTreeShiftF1
l:
l:// Tree KeyBar Shift-F1-F12
""
""
""
""
""

MKBTreeShiftF2
""
""
""
""
""

MKBTreeShiftF3
""
""
""
""
""

MKBTreeShiftF4
""
""
""
""
""

MKBTreeShiftF5
"Копир"
"Copy"
"Kopír."
"Kopier"
"Másol"

MKBTreeShiftF6
"Перен"
"Rename"
"Přejm."
"Umben"
"ÁtnMoz"

MKBTreeShiftF7
""
""
""
""
""

MKBTreeShiftF8
""
""
""
""
""

MKBTreeShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"

MKBTreeShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"
"UtsMnü"

MKBTreeShiftF11
"Группы"
"Group"
"Skupin"
"Gruppe"
"Csoprt"

MKBTreeShiftF12
"Выбран"
"SelUp"
"VybPrv"
"AuswOb"
"KijFel"

MKBTreeAltF1
l:
l:// Tree KeyBar Alt-F1-F12
"Левая"
"Left"
"Levý"
"Links"
"Bal"

MKBTreeAltF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"

MKBTreeAltF3
""
""
""
""
""

MKBTreeAltF4
""
""
""
""
""

MKBTreeAltF5
""
""
""
""
""

MKBTreeAltF6
""
""
""
""
""

MKBTreeAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"

MKBTreeAltF8
"Истор"
"Histry"
"Histor"
"Histor"
"ParElő"

MKBTreeAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"

MKBTreeAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"

MKBTreeAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"

MKBTreeAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"
"MapElő"

MKBTreeCtrlF1
l:
l:// Tree KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"
"Links"
"Bal"

MKBTreeCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"

MKBTreeCtrlF3
""
""
""
""
""

MKBTreeCtrlF4
""
""
""
""
""

MKBTreeCtrlF5
""
""
""
""
""

MKBTreeCtrlF6
""
""
""
""
""

MKBTreeCtrlF7
""
""
""
""
""

MKBTreeCtrlF8
""
""
""
""
""

MKBTreeCtrlF9
""
""
""
""
""

MKBTreeCtrlF10
""
""
""
""
""

MKBTreeCtrlF11
""
""
""
""
""

MKBTreeCtrlF12
""
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
""

MKBTreeCtrlShiftF2
""
""
""
""
""

MKBTreeCtrlShiftF3
""
""
""
""
""

MKBTreeCtrlShiftF4
""
""
""
""
""

MKBTreeCtrlShiftF5
""
""
""
""
""

MKBTreeCtrlShiftF6
""
""
""
""
""

MKBTreeCtrlShiftF7
""
""
""
""
""

MKBTreeCtrlShiftF8
""
""
""
""
""

MKBTreeCtrlShiftF9
""
""
""
""
""

MKBTreeCtrlShiftF10
""
""
""
""
""

MKBTreeCtrlShiftF11
""
""
""
""
""

MKBTreeCtrlShiftF12
""
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
""

MKBTreeCtrlAltF2
""
""
""
""
""

MKBTreeCtrlAltF3
""
""
""
""
""

MKBTreeCtrlAltF4
""
""
""
""
""

MKBTreeCtrlAltF5
""
""
""
""
""

MKBTreeCtrlAltF6
""
""
""
""
""

MKBTreeCtrlAltF7
""
""
""
""
""

MKBTreeCtrlAltF8
""
""
""
""
""

MKBTreeCtrlAltF9
""
""
""
""
""

MKBTreeCtrlAltF10
""
""
""
""
""

MKBTreeCtrlAltF11
""
""
""
""
""

MKBTreeCtrlAltF12
""
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
""

MKBTreeAltShiftF2
""
""
""
""
""

MKBTreeAltShiftF3
""
""
""
""
""

MKBTreeAltShiftF4
""
""
""
""
""

MKBTreeAltShiftF5
""
""
""
""
""

MKBTreeAltShiftF6
""
""
""
""
""

MKBTreeAltShiftF7
""
""
""
""
""

MKBTreeAltShiftF8
""
""
""
""
""

MKBTreeAltShiftF9
""
""
""
""
""

MKBTreeAltShiftF10
""
""
""
""
""

MKBTreeAltShiftF11
""
""
""
""
""

MKBTreeAltShiftF12
""
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
""

MKBTreeCtrlAltShiftF2
""
""
""
""
""

MKBTreeCtrlAltShiftF3
""
""
""
""
""

MKBTreeCtrlAltShiftF4
""
""
""
""
""

MKBTreeCtrlAltShiftF5
""
""
""
""
""

MKBTreeCtrlAltShiftF6
""
""
""
""
""

MKBTreeCtrlAltShiftF7
""
""
""
""
""

MKBTreeCtrlAltShiftF8
""
""
""
""
""

MKBTreeCtrlAltShiftF9
""
""
""
""
""

MKBTreeCtrlAltShiftF10
""
""
""
""
""

MKBTreeCtrlAltShiftF11
""
""
""
""
""

MKBTreeCtrlAltShiftF12
""
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
"Eltelt: %8.8s  Maradt: %8.8s  %5d%1.1sb/s"

MKeyESCWasPressed
l:
"Действие было прервано"
"Operation has been interrupted"
"Operace byla přerušena"
"Vorgang wurde unterbrochen"
"A műveletet megszakította"

MDoYouWantToStopWork
"Вы действительно хотите отменить действие?"
"Do you really want to cancel it?"
"Opravdu chcete operaci stornovat?"
"Wollen Sie den Vorgang wirklich abbrechen?"
"Valóban le akarja állítani?"

MDoYouWantToStopWork2
"Продолжить выполнение?"
"Continue work? "
"Pokračovat v práci?"
"Vorgang fortsetzen? "
"Folytatja?"

MCheckingFileInPlugin
l:
"Файл проверяется в плагине"
"The file is being checked by the plugin"
"Soubor je právě kontrolován pluginem"
"Datei wird von Plugin überprüft"
"A fájlt ez a plugin használja:"

MDialogType
l:
"Диалог"
"Dialog"
"Dialog"
"Dialog"
"Párbeszéd"

MHelpType
"Помощь"
"Help"
"Nápověda"
"Hilfe"
"Súgó"

MFolderTreeType
"ПоискКаталогов"
"FolderTree"
"StromAdresáře"
"Ordnerbaum"
"MappaFa"

MVMenuType
"Меню"
"Menu"
"Menu"
"Menü"
"Menü"

MIncorrectMask
l:
"Некорректная маска файлов!"
"File-mask string contains errors!"
"Řetězec masky souboru obsahuje chyby!"
"Zeichenkette mit Dateimaske enthält Fehler!"
"A fájlmaszk hibás!"

MPanelBracketsForLongName
l:
"{}"
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
"A %COMSPEC% környezeti változó nincs megadva!"

MExecuteErrorMessage
"'%s' не является внутренней или внешней командой, исполняемой программой или пакетным файлом.\n"
"'%s' is not recognized as an internal or external command, operable program or batch file.\n"
"'%s' nebylo nalezeno jako vniřní nebo externí příkaz, spustitelná aplikace nebo dávkový soubor.\n"
"'%s' nicht erkannt als interner oder externer Befehl, Programm oder Stapeldatei.\n"
""%s" nem azonítható külső vagy belső parancsként, futtatható programként vagy batch fájlként.\n"

MOpenPluginCannotOpenFile
l:
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"
"A fájl nem nyitható meg"

MFileFilterTitle
l:
"Фильтр"
"Filter"
"Filtr"
"Filter"
"Szűrő"

MFileHilightTitle
"Раскраска файлов"
"Files highlighting"
"Zvýrazňování souborů"
"Farbmarkierungen"
"Fájlkiemelés"

MFileFilterName
"Имя &фильтра:"
"Filter &name:"
"Jmé&no filtru:"
"Filter&name:"
"Szűrő &neve:"

MFileFilterMatchMask
"&Маска:"
"&Mask:"
"&Maska"
"&Maske:"
"&Maszk:"

MFileFilterSize
"Разм&ер:"
"Si&ze:"
"Vel&ikost"
"G&röße:"
"M&éret:"

MFileFilterSizeFromSign
">="
">="
">="
">="
">="

MFileFilterSizeToSign
"<="
"<="
"<="
"<="
"<="

MFileFilterDate
"&Дата/Время:"
"Da&te/Time:"
"Dat&um/Čas:"
"Da&tum/Zeit:"
"&Dátum/Idő:"

MFileFilterModified
"&модификации"
"&modification"
"&modifikace"
"&Modifikation"
"&Módosítás"

MFileFilterCreated
"&создания"
"&creation"
"&vytvoření"
"E&rstellung"
"&Létrehozás"

MFileFilterOpened
"&доступа"
"&access"
"&přístupu"
"Z&ugriff"
"&Hozzáférés"

MFileFilterDateRelative
"Относительна&я"
"Relat&ive"
"Relati&vní"
"Relat&iv"
"Relat&ív"

MFileFilterDateAfterSign
">="
">="
">="
">="
">="

MFileFilterDateBeforeSign
"<="
"<="
"<="
"<="
"<="

MFileFilterCurrent
"Теку&щая"
"C&urrent"
"Aktuá&lní"
"Akt&uell"
"&Jelenlegi"

MFileFilterBlank
"С&брос"
"B&lank"
"Práz&dný"
"&Leer"
"&Üres"

MFileFilterAttr
"Атрибут&ы"
"Attri&butes"
"Attri&buty"
"Attri&bute"
"Attri&bútumok"

MFileFilterAttrR
"&Только для чтения"
"&Read only"
"Jen pro čt&ení"
"Sch&reibschutz"
"&Csak olvasható"

MFileFilterAttrA
"&Архивный"
"&Archive"
"Arc&hivovat"
"&Archiv"
"&Archív"

MFileFilterAttrH
"&Скрытый"
"&Hidden"
"Skry&tý"
"&Versteckt"
"&Rejtett"

MFileFilterAttrS
"С&истемный"
"&System"
"Systémo&vý"
"&System"
"Re&ndszer"

MFileFilterAttrC
"С&жатый"
"&Compressed"
"Kompri&movaný"
"&Komprimiert"
"&Tömörített"

MFileFilterAttrE
"&Зашифрованный"
"&Encrypted"
"Ši&frovaný"
"V&erschlüsselt"
"T&itkosított"

MFileFilterAttrD
"&Каталог"
"&Directory"
"Adr&esář"
"Ver&zeichnis"
"Map&pa"

MFileFilterAttrNI
"&Неиндексируемый"
"Not inde&xed"
"Neinde&xovaný"
"Nicht in&diziert"
"Nem inde&xelt"

MFileFilterAttrSparse
"&Разреженный"
"S&parse"
"Říd&ký"
"Reserve"
"Ritk&ított"

MFileFilterAttrT
"&Временный"
"Temporar&y"
"Doča&sný"
"Temporär"
"Átm&eneti"

MFileFilterAttrReparse
"Симво&л. ссылка"
"Symbolic lin&k"
"Sybolický li&nk"
"Symbolischer Lin&k"
"S&zimbolikus link"

MFileFilterAttrOffline
"Автономны&й"
"O&ffline"
"O&ffline"
"O&ffline"
"O&ffline"

MFileFilterAttrVirtual
"Вирт&уальный"
"&Virtual"
"Virtuální"
"&Virtuell"
"&Virtuális"

MFileFilterReset
"Очистит&ь"
"Reset"
"Reset"
"Rücksetzen"
"Reset"

MFileFilterCancel
"Отмена"
"Cancel"
"Storno"
"Abbruch"
"Mégsem"

MFileFilterMakeTransparent
"Выставить прозрачность"
"Make transparent"
"Zprůhlednit"
"Transparent"
"Legyen átlátszó"

MBadFileSizeFormat
"Неправильно заполнено поле размера!"
"File size field is incorrectly filled!"
"Velikost souboru neobsahuje správnou hodnotu!"
"Angabe der Dateigröße ist fehlerhaft!"
"A fájlméret mező rosszul van kitöltve!"

#Must be the last
MNewFileName
l:
"?Новый файл?"
"?New File?"
"?Nový soubor?"
"?Neue Datei?"
"?Új fájl?"
