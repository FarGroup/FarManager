m4_include(`farversion.m4')m4_dnl
#hpp file name
lang.hpp
#number of languages
6
#id:0 language file name, language name, language description
FarRus.lng Russian "Russian (Русский)"
#id:1 language file name, language name, language description
FarEng.lng English "English"
#id:2 language file name, language name, language description
FarCze.lng Czech "Czech (Cestina)"
#id:3 language file name, language name, language description
FarGer.lng German "German (Deutsch)"
#id:4 language file name, language name, language description
FarHun.lng Hungarian "Hungarian (Magyar)"
#id:5 language file name, language name, language description
FarPol.lng Polish "Polish (Polski)"

#head of the hpp file
hhead:#pragma once
hhead:

#tail of the hpp file
#htail:
#htail:
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
"Tak"

MNo
"Нет"
"No"
"Ne"
"Nein"
"Nem"
"Nie"

MOk
"Продолжить"
"OK"
"Ok"
"OK"
"OK"
"OK"

MHYes
l:
"&Да"
"&Yes"
"&Ano"
"&Ja"
"I&gen"
"&Tak"

MHNo
"&Нет"
"&No"
"&Ne"
"&Nein"
"Ne&m"
"&Nie"

MHOk
"&Продолжить"
"&OK"
"&Ok"
"&OK"
"&OK"
"&OK"

MCancel
l:
"Отменить"
"Cancel"
"Storno"
"Abbrechen"
"Megsem"
"Anuluj"

MRetry
"Повторить"
"Retry"
"Znovu"
"Wiederholen"
"Ujra"
"Ponow"

MSkip
"Пропустить"
"Skip"
"Preskocit"
"Uberspringen"
"Kihagy"
"Omin"

MAbort
"Прервать"
"Abort"
"Zrusit"
"Abbrechen"
"Megszakit"
"Zaniechaj"

MIgnore
"Игнорировать"
"Ignore"
"Ignorovat"
"Ignorieren"
"Megis"
"Zignoruj"

MDelete
"Удалить"
"Delete"
"Smazat"
"Loschen"
"Torol"
"Usun"

MSplit
"Разделить"
"Split"
"Rozdelit"
"Zerteilen"
"Feloszt"
"Podziel"

MRemove
"Удалить"
"Remove"
"Odstranit"
"Entfernen"
"Eltavolit"
"Usun"

MHCancel
l:
"&Отменить"
"&Cancel"
"&Storno"
"&Abbrechen"
"Meg&sem"
"&Anuluj"

MHRetry
"&Повторить"
"&Retry"
"&Znovu"
"&Wiederholen"
"U&jra"
"&Ponow"

MHSkip
"П&ропустить"
"&Skip"
"&Preskocit"
"Uber&springen"
"Ki&hagy"
"&Omin"

MHSkipAll
"Пропустить &все"
"S&kip all"
"Preskocit &vse"
"Alle ubersprin&gen"
"Kihagy &mind"
"Omin &wszystkie"

MHAbort
"Прер&вать"
"&Abort"
"Zr&usit"
"&Abbrechen"
"Megsza&kit"
"&Zaniechaj"

MHIgnore
"&Игнорировать"
"&Ignore"
"&Ignorovat"
"&Ignorieren"
"Me&gis"
"Z&ignoruj"

MHDelete
"&Удалить"
"&Delete"
"S&mazat"
"&Loschen"
"&Torol"
"&Usun"

MHRemove
"&Удалить"
"R&emove"
"&Odstranit"
"Ent&fernen"
"Elta&volit"
"U&sun"

MHSplit
"Раз&делить"
"Sp&lit"
"&Rozdelit"
"&Zerteilen"
"Fel&oszt"
"Po&dziel"

MWarning
l:
"Предупреждение"
"Warning"
"Varovani"
"Warnung"
"Figyelem"
"Ostrzezenie"

MError
"Ошибка"
"Error"
"Chyba"
"Fehler"
"Hiba"
"Blad"

MQuit
l:
"Выход"
"Quit"
"Konec"
"Beenden"
"Kilepes"
"Zakoncz"

MAskQuit
"Вы хотите завершить работу в FAR?"
"Do you want to quit FAR?"
"Opravdu chcete ukoncit FAR?"
"Wollen Sie FAR beenden?"
"Biztosan kilep a FAR-bol?"
"Czy chcesz zakonczyc prace z FARem?"

MF1
l:
l://functional keys - 6 characters max
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Sugo"
"Pomoc"

MF2
"ПользМ"
"UserMn"
"UzivMn"
"BenuMn"
"FhMenu"
"Menu"

MF3
"Просм"
"View"
"Zobraz"
"Betr."
"Megnez"
"Zobacz"

MF4
"Редакт"
"Edit"
"Edit"
"Bearb"
"Szerk."
"Edytuj"

MF5
"Копир"
"Copy"
"Kopir."
"Kopier"
"Masol"
"Kopiuj"

MF6
"Перен"
"RenMov"
"PrjPrs"
"Versch"
"AtnMoz"
"ZmNazw"

MF7
"Папка"
"MkFold"
"VytAdr"
"VerzEr"
"UjMapp"
"UtwKat"

MF8
"Удален"
"Delete"
"Smazat"
"Losch."
"Torol"
"Usun"

MF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"
"KonfMn"
"Konfig"

MF10
"Выход"
"Quit"
"Konec"
"Beend."
"Kilep"
"Koniec"

MF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Keprny"
"Ekran"

MAltF1
l:
"Левая"
"Left"
"Levy"
"Links"
"Bal"
"Lewy"

MAltF2
"Правая"
"Right"
"Pravy"
"Rechts"
"Jobb"
"Prawy"

MAltF3
"Смотр."
"View.."
"Zobr.."
"Betr.."
"Nezo.."
"Zobacz"

MAltF4
"Редак."
"Edit.."
"Edit.."
"Bear.."
"Szrk.."
"Edytuj"

MAltF5
"Печать"
"Print"
"Tisk"
"Druck"
"Nyomt"
"Drukuj"

MAltF6
"Ссылка"
"MkLink"
"VytLnk"
"LinkEr"
"UjLink"
"Dowiaz"

MAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"
"Znajdz"

MAltF8
"Истор"
"Histry"
"Histor"
"Histor"
"ParElo"
"Histor"

MAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Tryb"

MAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"
"Drzewo"

MAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NezElo"
"HsPodg"

MAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"BearHs"
"MapElo"
"HsKat"

MCtrlF1
l:
"Левая"
"Left"
"Levy"
"Links"
"Bal"
"Lewy"

MCtrlF2
"Правая"
"Right"
"Pravy"
"Rechts"
"Jobb"
"Prawy"

MCtrlF3
"Имя   "
"Name  "
"Nazev "
"Name  "
"Nev"
"Nazwa"

MCtrlF4
"Расшир"
"Extens"
"Pripon"
"Erweit"
"Kiterj"
"Rozsz"

MCtrlF5
"Модиф"
"Modifn"
"Modifk"
"Verand"
"ModIdo"
"Modyf"

MCtrlF6
"Размер"
"Size"
"Veliko"
"Gro?e"
"Meret"
"Rozm"

MCtrlF7
"Несорт"
"Unsort"
"Neradi"
"Unsort"
"NincsR"
"BezSor"

MCtrlF8
"Создан"
"Creatn"
"Vytvor"
"Erstel"
"Keletk"
"Utworz"

MCtrlF9
"Доступ"
"Access"
"Pristu"
"Zugrif"
"Hozzaf"
"Uzycie"

MCtrlF10
"Описан"
"Descr"
"Popis"
"Beschr"
"Megjgy"
"Opis"

MCtrlF11
"Владел"
"Owner"
"Vlastn"
"Besitz"
"Tulajd"
"Wlasc"

MCtrlF12
"Сорт"
"Sort"
"Tridit"
"Sort."
"RendMd"
"Sortuj"

MShiftF1
l:
"Добавл"
"Add"
"Pridat"
"Hinzu"
"Tomort"
"Dodaj"

MShiftF2
"Распак"
"Extrct"
"Rozbal"
"Extrah"
"Kibont"
"Rozpak"

MShiftF3
"АрхКом"
"ArcCmd"
"ArcPri"
"ArcBef"
"TomPar"
"Polec"

MShiftF4
"Редак."
"Edit.."
"Edit.."
"Erst.."
"UjFajl"
"Edytuj"

MShiftF5
"Копир"
"Copy"
"Kopir."
"Kopier"
"Masol"
"Kopiuj"

MShiftF6
"Переим"
"Rename"
"Prejme"
"Umbene"
"AtnMoz"
"ZmNazw"

MShiftF7
""
""
""
""
""
""

MShiftF8
"Удален"
"Delete"
"Smazat"
"Losch."
"Torol"
"Usun"

MShiftF9
"Сохран"
"Save"
"Ulozit"
"Speich"
"Mentes"
"Zapisz"

MShiftF10
"Послдн"
"Last"
"Posled"
"Letzte"
"UtsMnu"
"Ostatn"

MShiftF11
"Группы"
"Group"
"Skupin"
"Gruppe"
"Csoprt"
"Grupa"

MShiftF12
"Выбран"
"SelUp"
"VybPrv"
"AuswOb"
"KijFel"
"SelUp"

MAltShiftF1
l:
l:// Main AltShift
""
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
""

MAltShiftF3
""
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
""

MAltShiftF5
""
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
""

MAltShiftF7
""
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
""

MAltShiftF9
"КонфПл"
"ConfPl"
"KonfPl"
"KonfPn"
"PluKnf"
"KonfPl"

MAltShiftF10
""
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
""

MAltShiftF12
""
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
""

MCtrlShiftF2
""
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
"Megnez"
"Podglad"

MCtrlShiftF4
"Редакт"
"Edit"
"Edit"
"Bearb"
"Szerk."
"Edycja"

MCtrlShiftF5
""
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
""

MCtrlShiftF7
""
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
""

MCtrlShiftF9
""
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
""

MCtrlShiftF11
""
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
""

MCtrlAltF1
l:
l:// Main CtrlAlt
""
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
""

MCtrlAltF3
""
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
""

MCtrlAltF5
""
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
""

MCtrlAltF7
""
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
""

MCtrlAltF9
""
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
""

MCtrlAltF11
""
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
""

MCtrlAltShiftF1
l:
l:// Main CtrlAltShift
""
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
""

MCtrlAltShiftF3
""
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
""

MCtrlAltShiftF5
""
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
""

MCtrlAltShiftF7
""
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
""

MCtrlAltShiftF9
""
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
""

MCtrlAltShiftF11
""
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
""

MHistoryTitle
l:
"История команд"
"History"
"Historie"
"Historie der letzten Befehle"
"Parancs elozmenyek"
"Historia"

MFolderHistoryTitle
"История папок"
"Folders history"
"Historie adresaru"
"Zuletzt besuchte Ordner"
"Mappa elozmenyek"
"Historia katalogow"

MViewHistoryTitle
"История просмотра"
"File view history"
"Historie prohlizeni souboru"
"Zuletzt betrachtete Dateien"
"Fajl elozmenyek"
"Historia podgladu plikow"

MViewHistoryIsCreate
"Создать файл?"
"Create file?"
"Vytvorit soubor?"
"Datei erstellen?"
"Fajl letrehozasa?"
"Utworzyc plik?"

MHistoryView
"Просмотр"
"View"
"Zobrazit"
"Betr"
"Nezett"
"Zobacz"

MHistoryEdit
"Редактор"
"Edit"
"Editovat"
"Bearb"
"Szerk."
"Edytuj"

MHistoryExt
"Внешний "
"Ext."
"Rozsireni"
"Ext."
"Kit."
"Ext."

MHistoryClear
l:
"История будет полностью очищена. Продолжить?"
"All records in the history will be deleted. Continue?"
"Vsechny zaznamy v historii budou smazany. Pokracovat?"
"Die gesamte Historie wird geloscht. Fortfahren?"
"Az elozmenyek minden eleme torlodik. Folytatja?"
"Wszystkie wpisy historii beda usuniete. Kontynuowac?"

MClear
"&Очистить"
"&Clear history"
"&Vymazat historii"
"Historie &loschen"
"Elo&zmenyek torlese"
"&Czysc historie"

MConfigSystemTitle
l:
"Системные параметры"
"System settings"
"Nastaveni systemu"
"Grundeinstellungen"
"Rendszer beallitasok"
"Ustawienia systemowe"

MConfigRO
"&Снимать атрибут R/O c CD файлов"
"&Clear R/O attribute from CD files"
"Z&rusit atribut R/O u souboru na CD"
"Schreibschutz von CD-Dateien ent&fernen"
"&Csak olvashato attr. torlese CD fajlokrol"
"Wyczysc atrybut &R/O przy kopiowaniu z CD"

MConfigRecycleBin
"Удалять в &Корзину"
"&Delete to Recycle Bin"
"&Mazat do Kose"
"In Papierkorb &loschen"
"&Torles a Lomtarba"
"&Usuwaj do Kosza"

MConfigRecycleBinLink
"У&далять символические ссылки"
"Delete symbolic &links"
"Mazat symbolicke &linky"
"Symbolische L&inks loschen"
"Szimbolikus l&inkek torlese"
"Usun &linki symboliczne"

MConfigSystemCopy
"Использовать систе&мную функцию копирования"
"Use sys&tem copy routine"
"Pouzivat kopirovaci rutiny sys&temu"
"Sys&temeigene Kopierroutine verwenden"
"&Masolashoz a rendszerrutin hasznalata"
"Uzywaj &systemowej procedury kopiowania"

MConfigCopySharing
"Копировать открытые для &записи файлы"
"Copy files opened for &writing"
"Kopirovat soubory otevrene pro &zapis"
"Zum Schreiben geoffnete Dateien &kopieren"
"Irasra megnyitott &fajlok masolhatok"
"Kopiuj pliki otwarte do zap&isu"

MConfigScanJunction
"Ск&анировать символические ссылки"
"Scan s&ymbolic links"
"Prohledavat s&ymbolicke linky"
"S&ymbolische Links scannen"
"Szimbolikus linkek &vizsgalata"
"Skanuj linki s&ymboliczne"

MConfigCreateUppercaseFolders
"Создавать &папки заглавными буквами"
"Create folders in &uppercase"
"Vytvaret adresare &velkymi pismeny"
"Ordner in Gro?schreib&ung erstellen"
"Mappak letrehozasa &NAGYBETUKKEL"
"Nazwy katalogow &WIELKIMI LITERAMI"

MConfigInactivity
"&Время бездействия"
"&Inactivity time"
"&Doba necinnosti"
"Inaktivitats&zeit"
"A FAR kile&p"
"Czas &bezczynnosci"

MConfigInactivityMinutes
"минут"
"minutes"
"minut"
"Minuten"
"perc tetlenseg utan"
"&minut"

MConfigSaveHistory
"Сохранять &историю команд"
"Save commands &history"
"Ukladat historii &prikazu"
"&Befehlshistorie speichern"
"Parancs elo&zmenyek mentese"
"Zapisz historie &polecen"

MConfigSaveFoldersHistory
"Сохранять историю п&апок"
"Save &folders history"
"Ukladat historii &adresaru"
"&Ordnerhistorie speichern"
"M&appa elozmenyek mentese"
"Zapisz historie &katalogow"

MConfigSaveViewHistory
"Сохранять историю п&росмотра и редактора"
"Save &view and edit history"
"Ukladat historii Zobraz a Editu&j"
"Betrachter/&Editor-Historie speichern"
"Nezoke es &szerkeszto elozmenyek mentese"
"Zapisz historie podgladu i &edycji"

MConfigRegisteredTypes
"Использовать стандартные &типы файлов"
"Use Windows &registered types"
"Pouzivat regi&strovane typy Windows"
"&Registrierte Windows-Dateitypen verwenden"
"&Windows reg. fajltipusok hasznalata"
"Uzyj zare&jestrowanych typow Windows"

MConfigCloseCDGate
"Автоматически монтироват&ь CDROM"
"CD drive auto &mount"
"Automaticke pr&ipojeni CD disku"
"CD-Laufwerk auto&matisch schlie?en"
"CD talca a&utomatikus behuzasa"
"&Montuj CD automatycznie"

MConfigPersonalPath
"Путь к персональным п&лагинам:"
"&Path for personal plugins:"
"&Cesta k vlastnim pluginum:"
"&Pfad fur eigene Plugins:"
"Sajat plu&ginek utvonala:"
"S&ciezka do wlasnych pluginow:"

MConfigAutoSave
"Автозапись кон&фигурации"
"Auto &save setup"
"Automaticke ukladani &nastaveni"
"Setup automatisch &"speichern"
"B&eallitasok automatikus mentese"
"Automatycznie &zapisuj ustawienia"

MConfigPanelTitle
l:
"Настройки панели"
"Panel settings"
"Nastaveni panelu"
"Panels einrichten"
"Panel beallitasok"
"Ustawienia panelu"

MConfigHidden
"Показывать скр&ытые и системные файлы"
"Show &hidden and system files"
"Ukazat &skryte a systemove soubory"
"&Versteckte und Systemdateien anzeigen"
"&Rejtett es rendszerfajlok mutatva"
"Pokazuj pliki &ukryte i systemowe"

MConfigHighlight
"&Раскраска файлов"
"Hi&ghlight files"
"Zvy&raznovat soubory"
"Dateien mark&ieren"
"Fa&jlok kiemelese"
"W&yrozniaj pliki"

MConfigAutoChange
"&Автосмена папки"
"&Auto change folder"
"&Automaticky menit adresar"
"Ordner &automatisch wechseln (Baumansicht)"
"&Automatikus mappavaltas"
"&Automatycznie zmieniaj katalog"

MConfigSelectFolders
"Пометка &папок"
"Select &folders"
"Vybirat a&dresare"
"&Ordner auswahlen"
"A ma&ppak is kijelolhetok"
"Zaznaczaj katalo&gi"

MConfigSortFolderExt
"Сортировать имена папок по рас&ширению"
"Sort folder names by e&xtension"
"Radit adresare podle pripony"
"Ordner nach Er&weiterung sortieren"
"Mappak is rendezhetok &kiterjesztes szerint"
"Sortuj nazwy katalogow wg r&ozszerzen"

MConfigReverseSort
"Разрешить &обратную сортировку"
"Allow re&verse sort modes"
"Do&volit zmenu smeru razeni"
"&Umgekehrte Sortiermodi zulassen"
"Fordi&tott rendezes engedelyezese"
"Wlacz &mozliwosc odwrotnego sortowania"

MConfigAutoUpdateLimit
"Отключать автооб&новление панелей,"
"&Disable automatic update of panels"
"Vypnout a&utomatickou aktualizaci panelu"
"Automatisches Panelupdate &deaktivieren"
"Pan&el automatikus frissitese kikapcsolva,"
"&Wylacz automatyczna aktualizacje paneli"

MConfigAutoUpdateLimit2
"если объектов больше"
"if object count exceeds"
"jestlize pocet objektu prekroci"
"wenn mehr Objekte als"
"ha tobb elem van, mint:"
"jesli zawieraja wiecej obiektow niz"

MConfigAutoUpdateRemoteDrive
"Автообновление с&етевых дисков"
"Network drives autor&efresh"
"Automaticka obnova sitovych disku"
"Netzw&erklauferke autom. aktualisieren"
"Halozati meghajtok autom. &frissitese"
"Auto&odswiezanie dyskow sieciowych"

MConfigShowColumns
"Показывать &заголовки колонок"
"Show &column titles"
"Zobrazovat &nadpisy sloupcu"
"S&paltentitel anzeigen"
"Oszlop&nevek mutatva"
"Wyswietl tytuly &kolumn"

MConfigShowStatus
"Показывать &строку статуса"
"Show &status line"
"Zobrazovat sta&vovy radek"
"&Statuszeile anzeigen"
"A&llapotsor mutatva"
"Wyswietl &linie statusu"

MConfigShowTotal
"Показывать су&ммарную информацию"
"Show files &total information"
"Zobrazovat &informace o velikosti souboru"
"&Gesamtzahl fur Dateien anzeigen"
"Fajl ossze&s informacioja mutatva"
"Wyswietl &calkowita informacje o plikach"

MConfigShowFree
"Показывать с&вободное место"
"Show f&ree size"
"Zobrazovat vo&lne misto"
"&Freien Speicher anzeigen"
"Sza&bad lemezterulet mutatva"
"Wyswietl ilosc &wolnego miejsca"

MConfigShowScrollbar
"Показывать по&лосу прокрутки"
"Show scroll&bar"
"Zobrazovat &posuvnik"
"Scroll&balken anzeigen"
"Gorditosa&v mutatva"
"Wyswietl &suwak"

MConfigShowScreensNumber
"Показывать количество &фоновых экранов"
"Show background screens &number"
"Zobrazovat pocet &obrazovek na pozadi"
"&Nummer von Hintergrundseiten anzeigen"
"&Hatterkepernyok szama mutatva"
"Wyswietl ilosc &ekranow w tle"

MConfigShowSortMode
"Показывать букву режима сор&тировки"
"Show sort &mode letter"
"Zobrazovat pismeno &modu razeni"
"Buchstaben der Sortier&modi anzeigen"
"Rendezesi mo&d betujele mutatva"
"Wyswietl l&itere trybu sortowania"

MConfigInterfaceTitle
l:
"Настройки интерфейса"
"Interface settings"
"Nastaveni rozhrani"
"Oberflache einrichten"
"Kezelofelulet beallitasok"
"Ustawienia interfejsu"

MConfigClock
"&Часы в панелях"
"&Clock in panels"
"&Hodiny v panelech"
"&Uhr in Panels anzeigen"
"Or&a a paneleken"
"&Zegar"

MConfigViewerEditorClock
"Ч&асы при редактировании и просмотре"
"C&lock in viewer and editor"
"H&odiny v prohlizeci a editoru"
"U&hr in Betrachter und Editor anzeigen"
"O&ra a nezokeben es szerkesztoben"
"Zegar w &podgladzie i edytorze"

MConfigMouse
"Мы&шь"
"M&ouse"
"M&ys"
"M&aus aktivieren"
"&Eger kezelese"
"M&ysz"

MConfigKeyBar
"Показывать &линейку клавиш"
"Show &key bar"
"Zobrazovat &zkratkove klavesy"
"Tast&enleiste anzeigen"
"&Funkciobillentyuk sora mutatva"
"Wyswietl pasek &klawiszy"

MConfigMenuBar
"Всегда показывать &меню"
"Always show &menu bar"
"Vzdy zobrazovat hlavni &menu"
"&Menuleiste immer anzeigen"
"A &menusor mindig latszik"
"Zawsze pokazuj pasek &menu"

MConfigSaver
"&Сохранение экрана"
"&Screen saver"
"Sp&oric obrazovky"
"Bildschirm&schoner"
"&Kepernyopihenteto"
"&Wygaszacz ekranu"

MConfigSaverMinutes
"минут"
"minutes"
"minut"
"Minuten"
"perc tetlenseg utan"
"m&inut"

MConfigUsePromptFormat
"Установить &формат командной строки"
"Set command line &prompt format"
"Nastavit format &prikazoveho radku"
"&Promptformat der Kommandozeile"
"Parancssori &prompt formatuma"
"Wy&glad znaku zachety linii polecen"

MConfigCopyTotal
"Показывать &общий индикатор копирования"
"Show &total copy progress indicator"
"Zobraz. ukazatel celkoveho stavu &kopirovani"
"Zeige Gesamtfor&tschritt beim Kopieren"
"Masolas osszesen folyamat&jelzo"
"Pokaz &calkowity postep kopiowania"

MConfigCopyTimeRule
"Показывать информацию о времени &копирования"
"Show cop&ying time information"
"Zobrazovat informace o case kopirovani"
"Zeige Rest&zeit beim Kopieren"
"Ma&solasi ido mutatva"
"Pokaz informacje o c&zasie kopiowania"

MConfigPgUpChangeDisk
"Использовать Ctrl-PgUp для в&ыбора диска"
"Use Ctrl-Pg&Up to change drive"
"Pouzit Ctrl-Pg&Up pro zmenu disku"
"Strg-Pg&Up wechselt das Laufwerk"
"A Ctrl-Pg&Up meghajtot valt"
"Uzyj Ctrl-Pg&Up do zmiany napedu"

MConfigClearType
upd:"ClearType friendly redraw (can be slow)"
"ClearType friendly redraw (can be slow)"
upd:"ClearType friendly redraw (can be slow)"
upd:"ClearType friendly redraw (can be slow)"
upd:"ClearType friendly redraw (can be slow)"
upd:"ClearType friendly redraw (can be slow)"

MConfigDlgSetsTitle
l:
"Настройки диалогов"
"Dialog settings"
"Nastaveni dialogu"
"Dialoge einrichten"
"Parbeszedablak beallitasok"
"Ustawienia okien dialogowych"

MConfigDialogsEditHistory
"&История в строках ввода диалогов"
"&History in dialog edit controls"
"H&istorie v dialozich"
"&Historie in Eingabefelder von Dialogen"
"&Beviteli sor elozmenyek mentese"
"&Historia w polach edycyjnych"

MConfigDialogsEditBlock
"&Постоянные блоки в строках ввода"
"&Persistent blocks in edit controls"
"&Trvale bloky v editacnich polich"
"Dauer&hafte Markierungen in Eingabefelder"
"Marado b&lokkok a beviteli sorokban"
"&Trwale bloki podczas edycji"

MConfigDialogsDelRemovesBlocks
"Del удаляет б&локи в строках ввода"
"&Del removes blocks in edit controls"
"&Del maze polozky v editacnich polich"
"&Entf loscht Markierungen"
"A &Del torli a beviteli sorok blokkjait"
"&Del usuwa blok podczas edycji"

MConfigDialogsAutoComplete
"&Автозавершение в строках ввода"
"&AutoComplete in edit controls"
"Automaticke dokoncovani v editac&nich polich"
"&Automatisches Vervollstandigen"
"Beviteli sor a&utomatikus kiegeszitese"
"&Autouzupelnianie podczas edycji"

MConfigDialogsEULBsClear
"Backspace &удаляет неизмененный текст"
"&Backspace deletes unchanged text"
"&Backspace maze nezmeneny text"
"&Rucktaste (BS) loscht unveranderten Text"
"A Ba&ckspace torli a valtozatlan szoveget"
"&Backspace usuwa nie zmieniony tekst"

MConfigDialogsMouseButton
"Клик мыши &вне диалога закрывает диалог"
"Mouse click &outside a dialog closes it"
"Kl&iknuti mysi mimo dialog ho zavre"
"Dial&og schlie?en wenn Mausklick ausserhalb"
"&Egerkattintas a parb.ablakon kivul: bezarja"
"&Klikniecie myszy poza oknem zamyka je"

MViewConfigTitle
l:
"Программа просмотра"
"Viewer"
"Prohlizec"
"Betrachter"
"Nezoke"
"Podglad"

MViewConfigExternalF3
"Запускать внешнюю программу просмотра по F3 вместо Alt-F3"
"Use external viewer for F3 instead of Alt-F3"
upd:"Use external viewer for F3 instead of Alt-F3"
upd:"Use external viewer for F3 instead of Alt-F3"
"Alt-F3 helyett F3 inditja a kulso nezoket"
upd:"Use external viewer for F3 instead of Alt-F3"

MViewConfigExternalCommand
"&Команда просмотра:"
"&Viewer command:"
"&Prikaz prohlizece:"
"Befehl fur e&xternen Betracher:"
"Nezoke &parancs:"
"&Polecenie:"

MViewConfigInternal
" Встроенная программа просмотра "
" Internal viewer "
" Interni prohlizec "
" Interner Betracher "
" Belso nezoke "
" Podglad wbudowany "

MViewConfigSavePos
"&Сохранять позицию файла"
"&Save file position"
"&Ukladat pozici v souboru"
"Dateipositionen &speichern"
"&Fajlpozicio mentese"
"&Zapamietaj pozycje w pliku"

MViewConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"
"Ukladat &zalozky"
"&Lesezeichen speichern"
"Konyv&jelzok mentese"
"Zapisz z&akladki"

MViewAutoDetectCodePage
"&Автоопределение кодовой страницы"
"&Autodetect code page"
upd:"&Autodetekovat znakovou sadu"
upd:"Zeichentabelle &automatisch erkennen"
"&Kodlap automatikus felismerese"
"Rozpozn&aj tablice znakow"

MViewConfigTabSize
"Раз&мер табуляции"
"Tab si&ze"
"Velikost &Tabulatoru"
"Ta&bulatorgro?e"
"Ta&bulator merete"
"Rozmiar &tabulatora"

MViewConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"
"Zobrazovat posu&vnik"
"Scro&llbalken anzeigen"
"Gor&ditosav mutatva"
"Pokaz &pasek przewijania"

MViewConfigArrows
"Показывать стрелки с&двига"
"Show scrolling arro&ws"
"Zobrazovat &skrolovaci sipky"
"P&feile bei Scrollbalken zeigen"
"Gorditon&yilak mutatva"
"Pokaz strzal&ki przewijania"

MViewConfigPersistentSelection
"Постоянное &выделение"
"&Persistent selection"
"Trvale &vybery"
"Dauerhafte Text&markierungen"
"&Marado blokkok"
"T&rwale zaznaczenie"

MViewConfigAnsiCodePageAsDefault
"&Использовать кодовую страницу ANSI по умолчанию"
"Use ANS&I code page by default"
upd:"Automaticky otevirat soubory ve &WIN kodovani"
upd:"Dateien standardma?ig mit Windows-Kod&ierung offnen"
"Fajlok eredeti megnyitasa ANS&I kodlappal"
"&Otwieraj pliki w kodowaniu Windows"

MEditConfigTitle
l:
"Редактор"
"Editor"
"Editor"
"Editor"
"Szerkeszto"
"Edytor"

MEditConfigEditorF4
"Запускать внешний редактор по F4 вместо Alt-F4"
"Use external editor for F4 instead of Alt-F4"
upd:"Use external editor for F4 instead of Alt-F4"
upd:"Use external editor for F4 instead of Alt-F4"
"Alt-F4 helyett F4 inditja a kulso szerkesztot"
upd:"Use external editor for F4 instead of Alt-F4"

MEditConfigEditorCommand
"&Команда редактирования:"
"&Editor command:"
"&Prikaz editoru:"
"Befehl fur e&xternen Editor:"
"&Szerkeszto parancs:"
"&Polecenie:"

MEditConfigInternal
" Встроенный редактор "
" Internal editor "
" Interni editor "
" Interner Editor "
" Belso szerkeszto "
" Edytor wbudowany "

MEditConfigExpandTabsTitle
"Преобразовывать &табуляцию:"
"Expand &tabs:"
"Rozsirit Ta&bulatory mezerami"
"&Tabs expandieren:"
"&Tabulatorbol szokozok:"
"Zamiana znakow &tabulacji:"

MEditConfigDoNotExpandTabs
l:
"Не преобразовывать табуляцию"
"Do not expand tabs"
"Nerozsirovat tabulatory mezerami"
"Tabs nicht expandieren"
"Ne helyettesitse a tabulatorokat"
"Nie zamieniaj znakow tabulacji"

MEditConfigExpandTabs
"Преобразовывать новые символы табуляции в пробелы"
"Expand newly entered tabs to spaces"
"Rozsirit nove zadane tabulatory mezerami"
"Neue Tabs zu Leerzeichen expandieren"
"Ujonnan beirt tabulatorbol szokozok"
"Zamien nowo dodane znaki tabulacji na spacje"

MEditConfigConvertAllTabsToSpaces
"Преобразовывать все символы табуляции в пробелы"
"Expand all tabs to spaces"
"Rozsirit vsechny tabulatory mezerami"
"Alle Tabs zu Leerzeichen expandieren"
"Minden tabulatorbol szokozok"
"Zastap wszystkie tabulatory spacjami"

MEditConfigPersistentBlocks
"&Постоянные блоки"
"&Persistent blocks"
"&Trvale bloky"
"Dauerhafte Text&markierungen"
"&Marado blokkok"
"T&rwale bloki"

MEditConfigDelRemovesBlocks
l:
"Del удаляет б&локи"
"&Del removes blocks"
"&Del maze bloky"
"&Entf loscht Textmark."
"A &Del torli a blokkokat"
"&Del usuwa bloki"

MEditConfigAutoIndent
"Авто&отступ"
"Auto &indent"
"Auto &Odsazovani"
"Automatischer E&inzug"
"Automatikus &behuzas"
"Automatyczne &wciecia"

MEditConfigSavePos
"&Сохранять позицию файла"
"&Save file position"
"&Ukladat pozici v souboru"
"Dateipositionen &speichern"
"Fajl&pozicio mentese"
"&Zapamietaj pozycje kursora w pliku"

MEditConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"
"Ukladat za&lozky"
"&Lesezeichen speichern"
"Konyv&jelzok mentese"
"Zapisz &zakladki"

MEditCursorBeyondEnd
"Ку&рсор за пределами строки"
"&Cursor beyond end of line"
"&Kurzor za koncem radku"
upd:"&Cursor hinter dem Ende"
"Kurzor a sor&vegjel utan is"
"&Kursor za koncem linii"

MEditAutoDetectCodePage
"&Автоопределение кодовой страницы"
"&Autodetect code page"
upd:"&Autodetekovat znakovou sadu"
upd:"Zeichentabelle &automatisch erkennen"
"&Kodlap automatikus felismerese"
"Rozpozn&aj tablice znakow"

MEditShareWrite
"Разрешить редактирование открытых для записи &файлов"
"Allow editing files ope&ned for writing"
upd:"Allow editing files opened for &writing"
upd:"Allow editing files opened for &writing"
"Irasra m&egnyitott fajlok szerkeszthetok"
upd:"Allow editing files opened for &writing"

MEditLockROFileModification
"Блокировать р&едактирование файлов с атрибутом R/O"
"Lock editing of read-only &files"
"&Zamknout editaci souboru urcenych jen pro cteni"
"Bearbeiten von &Dateien mit Schreibschutz verhindern"
"Csak olvashato fajlok s&zerkesztese tiltva"
"Nie edytuj plikow tylko do odczytu"

MEditWarningBeforeOpenROFile
"Пре&дупреждать при открытии файла с атрибутом R/O"
"&Warn when opening read-only files"
"&Varovat pri otevreni souboru urcenych jen pro cteni"
"Beim Offnen von Dateien mit Schreibschutz &warnen"
"Figyelmeztet &csak olvashato fajl megnyitasakor"
"&Ostrzez przed otwieraniem plikow tylko do odczytu"

MEditConfigTabSize
"Раз&мер табуляции"
"Tab si&ze"
"Velikost &Tabulatoru"
"Ta&bulatorgro?e"
"Tab&ulator merete"
"Rozmiar ta&bulatora"

MEditConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"
"Zobr&azovat posuvnik"
"Scro&llbalken anzeigen"
"&Gorditosav mutatva"
"Pokaz %pasek przewijania"

MEditConfigPickUpWord
"Cлово под к&урсором"
"Pick &up the word"
upd:"Pick &up the word"
upd:"Pick &up the word"
upd:"Pick &up the word"
upd:"Pick &up the word"

MEditConfigAnsiCodePageAsDefault
"&Использовать кодовую страницу ANSI по умолчанию"
"Use ANS&I code page by default"
upd:"Automaticky otevirat soubory ve &WIN kodovani"
upd:"Dateien standardma?ig mit Windows-Kod&ierung offnen"
"Fajlok eredeti megnyitasa ANS&I kodlappal"
"&Probuj otwierac pliki w kodowaniu Windows"

MEditConfigAnsiCodePageForNewFile
"Использо&вать кодовую страницу ANSI при создании файлов"
"Use ANSI code page when c&reating new files"
upd:"V&ytvaret nove soubory ve WIN kodovani"
upd:"Neue Dateien mit Windows-Ko&dierung erstellen"
"Uj &fajl letrehozasa ANSI kodlappal"
"&Tworz nowe pliki w kodowaniu Windows"

MSaveSetupTitle
l:
"Конфигурация"
"Save setup"
"Ulozit nastaveni"
"Einstellungen speichern"
"Beallitasok mentese"
"Zapisz ustawienia"

MSaveSetupAsk1
"Вы хотите сохранить"
"Do you wish to save"
"Prejete si ulozit"
"Wollen Sie die aktuellen Einstellungen"
"Elmenti a jelenlegi"
"Czy chcesz zapisac"

MSaveSetupAsk2
"текущую конфигурацию?"
"current setup?"
"aktualni nastaveni?"
"speichern?"
"beallitasokat?"
"biezace ustawienia?"

MSaveSetup
"Сохранить"
"Save"
"Ulozit"
"Speichern"
"Mentes"
"Zapisz"

MCopyDlgTitle
l:
"Копирование"
"Copy"
"Kopirovat"
"Kopieren"
"Masolas"
"Kopiuj"

MMoveDlgTitle
"Переименование/Перенос"
"Rename/Move"
"Prejmenovat/Presunout"
"Verschieben/Umbenennen"
"Atnevezes-Mozgatas"
"Zmien nazwe/przenies"

MLinkDlgTitle
"Ссылка"
"Link"
"Link"
"Link erstellen"
"Link letrehozasa"
"Dowiaz"

MCopySecurity
"П&рава доступа:"
"&Access rights:"
"&Pristupova prava:"
"Zugriffsrecht&e:"
"Hozzaferesi &jogok:"
"&Prawa dostepu:"

MCopySecurityCopy
"Копироват&ь"
"Co&py"
"&Kopirovat"
"Ko&pieren"
"Mas&ol"
"Kopiu&j"

MCopySecurityInherit
"Нас&ледовать"
"&Inherit"
"&Zdedit"
"Ve&rerben"
"O&rokol"
"&Dziedzicz"

MCopySecurityLeave
"По умол&чанию"
"Defau&lt"
"Vych&ozi"
"A&utomat."
"Ala&pert."
"Do&myslne"

MCopyIfFileExist
"Уже су&ществующие файлы:"
"Already e&xisting files:"
"Jiz e&xistujici soubory:"
"&Dateien uberschreiben:"
"Mar &letezo fajloknal:"
"Dla juz &istniejacych:"

MCopyAsk
"&Запрос действия"
"&Ask"
"Ptat s&e"
"Fr&agen"
"Ker&dez"
"&Zapytaj"

MCopyAskRO
"Запрос подтверждения для &R/O файлов"
"Also ask on &R/O files"
"Ptat se take na &R/O soubory"
"Bei Dateien mit Sch&reibschutz fragen"
"&Csak olvashato fajloknal is kerdez"
"&Pytaj takze o pliki tylko do odczytu"

MCopyOnlyNewerFiles
"Только &новые/обновленные файлы"
"Only ne&wer file(s)"
"Pouze &novejsi soubory"
"Nur &neuere Dateien"
"Cs&ak az ujabb fajlokat"
"Tylko &nowsze pliki"

MLinkType
"&Тип ссылки:"
"Link t&ype:"
"&Typ linku:"
"Linkt&yp:"
"Link &tipusa:"
"&Typ linku:"

MLinkTypeJunction
"&связь каталогов"
"directory &junction"
"krizeni a&dresaru"
"Ordner&knotenpunkt"
"Mappa &csomopont"
"directory &junction"

MLinkTypeHardlink
"&жёсткая ссылка"
"&hard link"
"&pevny link"
"&Hardlink"
"&Hardlink"
"link &trwaly"

MLinkTypeSymlinkFile
"символическая ссылка (&файл)"
"symbolic link (&file)"
"symbolicky link (&soubor)"
"Symbolischer Link (&Datei)"
"Szimbolikus link (&fajl)"
"link symboliczny (do &pliku)"

MLinkTypeSymlinkDirectory
"символическая ссылка (&папка)"
"symbolic link (fol&der)"
"symbolicky link (&adresar)"
"Symbolischer Link (Or&dner)"
"Szimbolikus link (&mappa)"
"link symboliczny (do &folderu)"

MCopySymLinkContents
"Копировать содерж&имое символических ссылок"
"Cop&y contents of symbolic links"
"Kopirovat obsah sym&bolickych linku"
"Inhalte von s&ymbolischen Links kopieren"
"Sz&imbolikus linkek masolasa"
"&Kopiuj zawartosc linkow symbolicznych"

MCopyMultiActions
"Обр&абатывать несколько имен файлов"
"Process &multiple destinations"
"&Zpracovat vice mist urceni"
"&Mehrere Ziele verarbeiten"
"To&bbszoros cel letrehozasa"
"Przetwarzaj &wszystkie cele"

MCopyDlgCopy
"&Копировать"
"&Copy"
"&Kopirovat"
"&Kopieren"
"&Masolas"
"&Kopiuj"

MCopyDlgTree
"F10-&Дерево"
"F10-&Tree"
"F10-&Strom"
"F10-&Baum"
"F10-&Fa"
"F10-&Drzewo"

MCopyDlgCancel
"&Отменить"
"Ca&ncel"
"&Storno"
"Ab&bruch"
"Meg&sem"
"&Anuluj"

MCopyDlgRename
"&Переименовать"
"&Rename"
"Prej&menovat"
"&Umbenennen"
"At&nevez-Mozgat"
"&Zmien nazwe"

MCopyDlgLink
"&Создать ссылку"
"&Link"
"&Linkovat"
"Ver&linken"
"&Linkel"
"D&owiaz"

MCopyDlgTotal
"Всего"
"Total"
"Celkem"
"Gesamt"
"Osszesen"
"Razem"

MCopyScanning
"Сканирование папок..."
"Scanning folders..."
"Nacitani adresaru..."
"Scanne Ordner..."
"Mappak olvasasa..."
"Przeszukuje katalogi..."

MCopyPrepareSecury
"Применение прав доступа..."
"Applying access rights..."
"Nastavuji pristupova prava..."
"Anwenden der Zugriffsrechte..."
"Hozzaferesi jogok alkalmazasa..."
"Ustawianie praw dostepu..."

MCopyUseFilter
"Исполь&зовать фильтр"
"&Use filter"
"P&ouzit filtr"
"Ben&utze Filter"
"Szuro&vel"
"&Uzyj filtra"

MCopySetFilter
"&Фильтр"
"Filt&er"
"Filt&r"
"Filt&er"
"S&zuro"
"Filt&r"

MCopyFile
l:
"Копировать"
"Copy"
"Kopirovat"
"Kopiere"
upd:"masolasa"
"Skopiuj"

MMoveFile
"Переименовать или перенести"
"Rename or move"
"Prejmenovat nebo presunout"
"Verschiebe"
upd:"atnevezese-mozgatasa"
"Zmien nazwe lub przenies"

MLinkFile
"Создать ссылку"
"Link"
"Linkovat"
"Verlinke"
upd:"linkelese"
"Dowiaz"

MCopyFiles
"Копировать %d элемент%s"
"Copy %d item%s"
"Kopirovat %d poloz%s"
"Kopiere %d Objekt%s"
" %d elem masolasa"
"Skopiuj %d plikow"

MMoveFiles
"Переименовать или перенести %d элемент%s"
"Rename or move %d item%s"
"Prejmenovat nebo presunout %d poloz%s"
"Verschiebe %d Objekt%s"
" %d elem atnevezese-mozgatasa"
"Zmien nazwe lub przenies %d plikow"

MLinkFiles
"Создать ссылки %d элемент%s"
"Link %d item%s"
"Linkovat %d poloz%s"
"Verlinke %d Objekt%s"
" %d elem linkelese"
"Dowiaz %d plikow"

MCMLTargetTO
" &в:"
" t&o:"
" d&o:"
" na&ch:"
" ide:"
" d&o:"

MCMLItems0
""
""
"u"
""
""
""

MCMLItemsA
"а"
"s"
"ek"
"e"
""
"s"

MCMLItemsS
"ов"
"s"
"ky"
"e"
""
"s"

MCopyIncorrectTargetList
l:
"Указан некорректный список целей!"
"Incorrect target list!"
"Nespravny seznam cilu!"
"Ungultige Liste von Zielen!"
"Ervenytelen cellista!"
"Bledna lista wynikowa!"

MCopyCopyingTitle
l:
"Копирование"
"Copying"
"Kopiruji"
"Kopieren"
"Masolas"
"Kopiowanie"

MCopyMovingTitle
"Перенос"
"Moving"
"Presouvam"
"Verschieben"
"Mozgatas"
"Przenoszenie"

MCopyCannotFind
l:
"Файл не найден"
"Cannot find the file"
"Nelze nalezt soubor"
"Folgende Datei kann nicht gefunden werden:"
"A fajl nem talalhato:"
"Nie moge odnalezc pliku"

MCannotCopyFolderToItself1
l:
"Нельзя копировать папку"
"Cannot copy the folder"
"Nelze kopirovat adresar"
"Folgender Ordner kann nicht kopiert werden:"
"A mappa:"
"Nie mozna skopiowac katalogu"

MCannotCopyFolderToItself2
"в саму себя"
"onto itself"
"sam na sebe"
"Ziel und Quelle identisch."
"nem masolhato onmagaba/onmagara"
"do niego samego"

MCannotCopyToTwoDot
l:
"Нельзя копировать файл или папку"
"You may not copy files or folders"
"Nelze kopirovat soubory nebo adresare"
"Kopieren von Dateien oder Ordnern ist maximal"
"Nem masolhatja a fajlt vagy mappat"
"Nie mozna skopiowac plikow"

MCannotMoveToTwoDot
"Нельзя перемещать файл или папку"
"You may not move files or folders"
"Nelze presunout soubory nebo adresare"
"Verschieben von Dateien oder Ordnern ist maximal"
"Nem mozgathatja a fajlt vagy mappat"
"Nie mozna przeniesc plikow"

MCannotCopyMoveToTwoDot
"выше корневого каталога"
"higher than the root folder"
"na vyssi uroven nez korenovy adresar"
"bis zum Wurzelverzeichnis moglich."
"a gyoker fole"
"na poziom wyzszy niz do korzenia"

MCopyCannotCreateFolder
l:
"Ошибка создания папки"
"Cannot create the folder"
"Nelze vytvorit adresar"
"Folgender Ordner kann nicht erstellt werden:"
"A mappa nem hozhato letre:"
"Nie udalo sie utworzyc katalogu"

MCopyCannotChangeFolderAttr
"Невозможно установить атрибуты для папки"
"Failed to set folder attributes"
"Nastaveni atributu adresare selhalo"
"Fehler beim Setzen der Ordnerattribute"
"A mappa attributumok beallitasa sikertelen"
"Nie udalo sie ustawic atrybutow folderu"

MCopyCannotRenameFolder
"Невозможно переименовать папку"
"Cannot rename the folder"
"Nelze prejmenovat adresar"
"Folgender Ordner kann nicht umbenannt werden:"
"A mappa nem nevezheto at:"
"Nie udalo sie zmienic nazwy katalogu"

MCopyIgnore
"&Игнорировать"
"&Ignore"
"&Ignorovat"
"&Ignorieren"
"Me&gis"
"&Ignoruj"

MCopyIgnoreAll
"Игнорировать &все"
"Ignore &All"
"Ignorovat &vse"
"&Alle ignorieren"
"Min&d"
"Ignoruj &wszystko"

MCopyRetry
"&Повторить"
"&Retry"
"&Opakovat"
"Wiede&rholen"
"U&jra"
"&Ponow"

MCopySkip
"П&ропустить"
"&Skip"
"&Preskocit"
"Ausla&ssen"
"&Kihagy"
"&Omin"

MCopySkipAll
"&Пропустить все"
"S&kip all"
"Pr&eskocit vse"
"Alle aus&lassen"
"Mi&nd"
"Omin w&szystkie"

MCopyCancel
"&Отменить"
"&Cancel"
"&Storno"
"Abb&rechen"
"Meg&sem"
"&Anuluj"

MCopyDecrypt
"Рас&шифровать"
"&Decrypt"
"&Desifrovat"
"Ent&schlusseln"
"&Titk. felold"
"&Deszyfruj"

MCopyDecryptAll
"&Все"
"Decrypt &all"
"Des&ifrovat vse"
"Alle e&ntschlusseln"
"&Mind"
"Deszyfruj ws&zystkie"

MCopyCannotCreateLink
l:
"Ошибка создания ссылки"
"Cannot create the link"
"Nelze vytvorit symbolicky link"
"Folgender Link kann nicht erstellt werden:"
"A link nem hozhato letre:"
"Nie udalo sie utworzyc linku"

MCopyFolderNotEmpty
"Папка назначения должна быть пустой"
"Target folder must be empty"
"Cilovy adresar musi byt prazdny"
"Zielordner muss leer sein."
"A celmappanak uresnek kell lennie"
"Folder wynikowy musi byc pusty"

MCopyCannotCreateJunctionToFile
"Невозможно создать связь. Файл уже существует:"
"Cannot create junction. The file already exists:"
"Nelze vytvorit krizovy odkaz. Soubor jiz existuje:"
"Knotenpunkt wurde nicht erstellt. Datei existiert bereits:"
"A csomopont nem hozhato letre. A fajl mar letezik:"
"Nie mozna utworzyc polaczenia - plik juz istnieje:"

MCopyCannotCreateVolMount
l:
"Ошибка монтирования диска"
"Volume mount points error"
"Chyba pripojovacich svazku"
"Fehler im Mountpoint des Datentragers"
"Kotet mountpont hiba"
"Blad montowania napedu"

MCopyMountVolFailed
"Ошибка при монтировании диска '%s'"
"Attempt to volume mount '%s'"
"Pokus o pripojeni svazku '%s'"
"Versuch Datentrager '%s' zu aktivieren"
""%s" kotet mountolasa"
"Nie udalo sie zamontowac woluminu '%s'"

MCopyMountVolFailed2
"на '%s'"
"at '%s' failed"
"na '%s' selhal"
"fehlgeschlagen bei '%s'"
"nem sikerult: "%s""
"w '%s' nie udalo sie"

MCopyMountName
"disk_%c"
"Disk_%c"
"Disk_%c"
"Disk_%c"
"Disk_%c"
"Disk_%c"

MCannotCopyFileToItself1
l:
"Нельзя копировать файл"
"Cannot copy the file"
"Nelze kopirovat soubor"
"Folgende Datei kann nicht kopiert werden:"
"A fajl"
"Nie mozna skopiowac pliku"

MCannotCopyFileToItself2
"в самого себя"
"onto itself"
"sam na sebe"
"Ziel und Quelle identisch."
"nem masolhato onmagara"
"do niego samego"

MCopyStream1
l:
"Исходный файл содержит более одного потока данных,"
"The source file contains more than one data stream."
"Zdrojovy soubor obsahuje vice nez jeden datovy proud."
"Die Quelldatei enthalt mehr als einen Datenstream"
"A forrasfajl tobb stream-et tartalmaz,"
"Plik zrodlowy zawiera wiecej niz jeden strumien danych."

MCopyStream2
"но вы не используете системную функцию копирования."
"but since you do not use a system copy routine."
"protoze nepouzivate systemovou kopirovaci rutinu."
"aber Sie verwenden derzeit nicht die systemeigene Kopierroutine."
"de nem a rendszer masolorutinjat hasznalja."
"ale ze wzgledu na rezygnacje z systemowej procedury kopiowania."

MCopyStream3
"но том назначения не поддерживает этой возможности."
"but the destination volume does not support this feature."
"protoze cilovy svazek nepodporuje tuto vlastnost."
"aber der Zieldatentrager unterstutzt diese Fahigkeit nicht."
"de a celkotet nem tamogatja ezt a lehetoseget."
"ale naped docelowy nie obsluguje tej funkcji."

MCopyStream4
"Часть сведений не будет сохранена."
"Some data will not be preserved as a result."
"To bude mit za nasledek, ze nektera data nebudou uchovana."
"Ein Teil der Daten bleiben daher nicht erhalten."
"Az adatok egy resze el fog veszni."
"Nie wszystkie dane zostana zachowane."

MCopyDirectoryOrFile
l:
"Подразумевается имя папки или файла?"
"Does it specify a folder name or file name?"
upd:"Does it specify a folder name or file name?"
upd:"Does it specify a folder name or file name?"
upd:"Does it specify a folder name or file name?"
upd:"Does it specify a folder name or file name?"

MCopyDirectoryOrFileDirectory
"Папка"
"Folder"
upd:"Folder"
upd:"Folder"
upd:"Folder"
upd:"Folder"

MCopyDirectoryOrFileFile
"Файл"
"File"
upd:"File"
upd:"File"
upd:"File"
upd:"File"

MCopyFileExist
l:
"Файл уже существует"
"File already exists"
"Soubor jiz existuje"
"Datei existiert bereits"
"A fajl mar letezik:"
"Plik juz istnieje"

MCopySource
"&Новый"
"&New"
"&Novy"
"&Neue Datei"
"U&j verzio:"
"&Nowy"

MCopyDest
"Су&ществующий"
"E&xisting"
"E&xistujici"
"Be&stehende Datei"
"Letezo &verzio:"
"&Istniejacy"

MCopyOverwrite
"В&место"
"&Overwrite"
"&Prepsat"
"Uber&schr."
"&Felulir"
"N&adpisz"

MCopySkipOvr
"&Пропустить"
"&Skip"
"Pres&kocit"
"Uber&spr."
"&Kihagy"
"&Omin"

MCopyAppend
"&Дописать"
"A&ppend"
"Pr&ipojit"
"&Anhangen"
"Hoz&zafuz"
"&Dolacz"

MCopyResume
"Возоб&новить"
"&Resume"
"Po&kracovat"
"&Weiter"
"Fol&ytat"
"Pono&w"

MCopyRename
"&Имя"
"R&ename"
upd:"R&ename"
upd:"R&ename"
"A&tnevez"
upd:"R&ename"

MCopyCancelOvr
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Megsem"
"&Anuluj"

MCopyRememberChoice
"&Запомнить выбор"
"&Remember choice"
"Zapama&tovat volbu"
"Auswahl me&rken"
"Mind&ent a kivalasztott modon"
"&Zapamietaj ustawienia"

MCopyRenameTitle
"Переименование"
"Rename"
upd:"Rename"
upd:"Rename"
"Atnevezes"
upd:"Rename"

MCopyRenameText
"&Новое имя:"
"&New name:"
upd:"&New name:"
upd:"&New name:"
"U&j nev:"
upd:"&New name:"

MCopyFileRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"
"Soubor je urcen pouze pro cteni"
"Folgende Datei ist schreibgeschutzt:"
"A fajl csak olvashato:"
"Ten plik jest tylko-do-odczytu"

MCopyAskDelete
"Вы хотите удалить его?"
"Do you wish to delete it?"
"Opravdu si ho prejete smazat?"
"Wollen Sie sie dennoch loschen?"
"Biztosan torolni akarja?"
"Czy chcesz go usunac?"

MCopyDeleteRO
"&Удалить"
"&Delete"
"S&mazat"
"&Loschen"
"&Torli"
"&Usun"

MCopyDeleteAllRO
"&Все"
"&All"
"&Vse"
"&Alle Loschen"
"Min&det"
"&Wszystkie"

MCopySkipRO
"&Пропустить"
"&Skip"
"Pres&kocit"
"Uber&springen"
"&Kihagyja"
"&Omin"

MCopySkipAllRO
"П&ропустить все"
"S&kip all"
"Pr&eskocit vse"
"A&lle uberspringen"
"Mind&et"
"O&min wszystkie"

MCopyCancelRO
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Megsem"
"&Anuluj"

MCannotCopy
l:
"Ошибка копирования"
"Cannot copy"
"Nelze kopirovat"
"Konnte nicht kopieren"
"Nem masolhato"
"Nie moge skopiowac"

MCannotMove
"Ошибка переноса"
"Cannot move"
"Nelze presunout"
"Konnte nicht verschieben"
"Nem mozgathato"
"Nie moge przeniesc"

MCannotLink
"Ошибка создания ссылки"
"Cannot link"
"Nelze linkovat"
"Konnte nicht verlinken"
"Nem linkelheto"
"Nie moge dowiazac"

MCannotCopyTo
"в"
"to"
"do"
"nach"
"ide:"
"do"

MCopyEncryptWarn1
"Файл"
"The file"
"Soubor"
"Die Datei"
"A fajl"
"Plik"

MCopyEncryptWarn2
"нельзя скопировать или переместить, не потеряв его шифрование."
"cannot be copied or moved without losing its encryption."
"nemuze byt zkopirovan nebo presunut bez ztraty jeho sifrovani."
"kann nicht bewegt werden ohne ihre Verschlusselung zu verlieren."
"csak titkositasa elvesztesevel masolhato vagy mozgathato."
"nie moze zostac skopiowany/przeniesiony bez utraty szyfrowania"

MCopyEncryptWarn3
"Можно пропустить эту ошибку или отменить операцию."
"You can choose to ignore this error and continue, or cancel."
"Muzete tuto chybu ignorovat a pokracovat, nebo operaci ukoncit."
"Sie konnen dies ignorieren und fortfahren oder abbrechen."
"Ennek ellenere folytathatja vagy felfuggesztheti."
"Mozesz zignorowac blad i kontynuowac lub anulowac operacje."

MCopyReadError
l:
"Ошибка чтения данных из"
"Cannot read data from"
"Nelze cist data z"
"Kann Daten nicht lesen von"
"Nem olvashato adat innen:"
"Nie moge odczytac danych z"

MCopyWriteError
"Ошибка записи данных в"
"Cannot write data to"
"Nelze zapsat data do"
"Dann Daten nicht schreiben in"
"Nem irhato adat ide:"
"Nie moge zapisac danych do"

MCopyProcessed
l:
"Обработано файлов: %d"
"Files processed: %d"
"Zpracovano souboru: %d"
"Dateien verarbeitet: %d"
" %d fajl kesz"
"Przetworzonych plikow: %d"

MCopyProcessedTotal
"Обработано файлов: %d из %d"
"Files processed: %d of %d"
"Zpracovano souboru: %d z %d"
"Dateien verarbeitet: %d von %d"
" %d fajl kesz %d fajlbol"
"Przetworzonych plikow: %d z %d"

MCopyMoving
"Перенос файла"
"Moving the file"
"Presunuji soubor"
"Verschiebe die Datei"
"Fajl mozgatasa"
"Przenosze plik"

MCopyCopying
"Копирование файла"
"Copying the file"
"Kopiruji soubor"
"Kopiere die Datei"
"Fajl masolasa"
"Kopiuje plik"

MCopyTo
"в"
"to"
"do"
"nach"
"ide:"
"do"

MCopyErrorDiskFull
l:
"Диск заполнен. Вставьте следующий"
"Disk full. Insert next"
"Disk je plny. Vlozte dalsii"
"Datentrager voll. Bitte nachsten einlegen"
"A lemez megtelt, kerem a kovetkezot!"
"Dysk pelny. Wloz nastepny"

MDeleteTitle
l:
"Удаление"
"Delete"
"Smazat"
"Loschen"
"Torles"
"Usun"

MAskDeleteFolder
"Вы хотите удалить папку"
"Do you wish to delete the folder"
"Prejete si smazat adresar"
"Wollen Sie den Ordner loschen"
"Torolni akarja a mappat?"
"Czy chcesz wymazac katalog"

MAskDeleteFile
"Вы хотите удалить файл"
"Do you wish to delete the file"
"Prejete si smazat soubor"
"Wollen Sie die Datei loschen"
"Torolni akarja a fajlt?"
"Czy chcesz usunac plik"

MAskDelete
"Вы хотите удалить"
"Do you wish to delete"
"Prejete si smazat"
"Wollen Sie folgendes Objekt loschen"
"Torolni akar"
"Czy chcesz usunac"

MAskDeleteRecycleFolder
"Вы хотите поместить в Корзину папку"
"Do you wish to move to the Recycle Bin the folder"
"Prejete si presunout do Kose adresar"
"Wollen Sie den Ordner in den Papierkorb verschieben"
"A Lomtarba akarja dobni a mappat?"
"Czy chcesz przeniesc katalog do Kosza"

MAskDeleteRecycleFile
"Вы хотите поместить в Корзину файл"
"Do you wish to move to the Recycle Bin the file"
"Prejete si presunout do Kose soubor"
"Wollen Sie die Datei in den Papierkorb verschieben"
"A Lomtarba akarja dobni a fajlt?"
"Czy chcesz przeniesc plik do Kosza"

MAskDeleteRecycle
"Вы хотите поместить в Корзину"
"Do you wish to move to the Recycle Bin"
"Prejete si presunout do Kose"
"Wollen Sie das Objekt in den Papierkorb verschieben"
"A Lomtarba akar dobni"
"Czy chcesz przeniesc do Kosza"

MDeleteWipeTitle
"Уничтожение"
"Wipe"
"Vymazat"
"Sicheres Loschen"
"Kisopres"
"Wymaz"

MAskWipeFolder
"Вы хотите уничтожить папку"
"Do you wish to wipe the folder"
"Prejete si vymazat adresar"
"Wollen Sie den Ordner sicher loschen"
"Ki akarja soporni a mappat?"
"Czy chcesz wymazac katalog"

MAskWipeFile
"Вы хотите уничтожить файл"
"Do you wish to wipe the file"
"Prejete si vymazat soubor"
"Wollen Sie die Datei sicher loschen"
"Ki akarja soporni a fajlt?"
"Czy chcesz wymazac plik"

MAskWipe
"Вы хотите уничтожить"
"Do you wish to wipe"
"Prejete si vymazat"
"Wollen Sie das Objekt sicher loschen"
"Ki akar soporni"
"Czy chcesz wymazac"

MDeleteLinkTitle
"Удаление ссылки"
"Delete link"
"Smazat link"
"Link loschen"
"Link torlese"
"Usun link"

MAskDeleteLink
"является ссылкой на"
"is a symbolic link to"
"je symbolicky link na"
"ist ein symbolischer Link auf"
"szimlinkelve ide:"
"jest linkiem symbolicznym do"

MAskDeleteLinkFolder
"папку"
"folder"
"adresar"
"Ordner"
"mappa"
"folder"

MAskDeleteLinkFile
"файл"
"file"
"soubor"
"Date"
"fajl"
"plik"

MAskDeleteItems
"%d элемент%s"
"%d item%s"
"%d poloz%s"
"%d Objekt%s"
"%d elemet%s"
"%d plik%s"

MAskDeleteItems0
""
""
"ku"
""
""
""

MAskDeleteItemsA
"а"
"s"
"ky"
"e"
""
"i"

MAskDeleteItemsS
"ов"
"s"
"ek"
"e"
""
"ow"

MDeleteFolderTitle
l:
"Удаление папки "
"Delete folder"
"Smazat adresar"
"Ordner loschen"
"Mappa torlese"
"Usun folder"

MWipeFolderTitle
"Уничтожение папки "
"Wipe folder"
"Vymazat adresar"
"Ordner sicher loschen"
"Mappa kisoprese"
"Wymaz folder"

MDeleteFilesTitle
"Удаление файлов"
"Delete files"
"Smazat soubory"
"Dateien loschen"
"Fajlok torlese"
"Usun pliki"

MWipeFilesTitle
"Уничтожение файлов"
"Wipe files"
"Vymazat soubory"
"Dateien sicher loschen"
"Fajlok kisoprese"
"Wymaz pliki"

MDeleteFolderConfirm
"Данная папка будет удалена:"
"The following folder will be deleted:"
"Nasledujici adresar bude smazan:"
"Folgender Ordner wird geloscht:"
"A mappa torlodik:"
"Nastepujacy folder zostanie usuniety:"

MWipeFolderConfirm
"Данная папка будет уничтожена:"
"The following folder will be wiped:"
"Nasledujici adresar bude vymazan:"
"Folgender Ordner wird sicher geloscht:"
"A mappa kisoprodik:"
"Nastepujacy folder zostanie wymazany:"

MDeleteWipe
"Уничтожить"
"Wipe"
"Vymazat"
"Sicheres Loschen"
"Kisopor"
"Wymaz"

MDeleteFileDelete
"&Удалить"
"&Delete"
"S&mazat"
"&Loschen"
"&Torol"
"&Usun"

MDeleteFileWipe
"&Уничтожить"
"&Wipe"
"V&ymazat"
"&Sicher loschen"
"Kiso&por"
"&Wymaz"

MDeleteFileAll
"&Все"
"&All"
"&Vse"
"&Alle"
"Min&det"
"&wszystkie"

MDeleteFileSkip
"&Пропустить"
"&Skip"
"Pres&kocit"
"Uber&springen"
"&Kihagy"
"&Omin"

MDeleteFileSkipAll
"П&ропустить все"
"S&kip all"
"Pr&eskocit vse"
"A&lle uberspr."
"Mind&et"
"O&min wszystkie"

MDeleteFileCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Megsem"
"&Anuluj"

MDeleteLinkDelete
l:
"Удалить ссылку"
"Delete link"
"Smazat link"
"Link loschen"
"Link torlese"
"Usun link"

MDeleteLinkUnlink
"Разорвать ссылку"
"Break link"
"Poskozeny link"
"Link auflosen"
"Link megszakitasa"
"Przerwij link"

MDeletingTitle
l:
"Удаление"
"Deleting"
"Mazani"
"Losche"
"Torles"
"Usuwam"

MDeleting
l:
"Удаление файла или папки"
"Deleting the file or folder"
"Mazani souboru nebo adresare"
"Loschen von Datei oder Ordner"
"Fajl vagy mappa torlese"
"Usuwam plik/katalog"

MDeletingWiping
"Уничтожение файла или папки"
"Wiping the file or folder"
"Vymazavani souboru nebo adresare"
"Sicheres loschen von Datei oder Ordner"
"Fajl vagy mappa kisoprese"
"Wymazuje plik/katalog"

MDeleteRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"
"Soubor je urcen pouze pro cteni"
"Folgende Datei ist schreibgeschutzt:"
"A fajl csak olvashato:"
"Ten plik jest tylko do odczytu"

MAskDeleteRO
"Вы хотите удалить его?"
"Do you wish to delete it?"
"Opravdu si ho prejete smazat?"
"Wollen Sie sie dennoch loschen?"
"Megis torolni akarja?"
"Czy chcesz go usunac?"

MAskWipeRO
"Вы хотите уничтожить его?"
"Do you wish to wipe it?"
"Opravdu si ho prejete vymazat?"
"Wollen Sie sie dennoch sicher loschen?"
"Megis ki akarja soporni?"
"Czy chcesz go wymazac?"

MDeleteHardLink1
l:
"Файл имеет несколько жестких ссылок"
"Several hard links link to this file."
"Vice pevnych linku ukazuje na tento soubor."
"Mehrere Hardlinks zeigen auf diese Datei."
"Tobb hardlink kapcsolodik a fajlhoz, a fajl"
"Do tego pliku prowadzi wiele linkow trwalych."

MDeleteHardLink2
"Уничтожение файла приведет к обнулению всех ссылающихся на него файлов."
"Wiping this file will void all files linking to it."
"Vymazani tohoto souboru zneplatni vsechny soubory, ktere na nej linkuji."
"Sicheres Loschen dieser Datei entfernt ebenfalls alle Links."
"kisoprese a linkelt fajlokat is megsemmisiti."
"Wymazanie tego pliku wymaze wszystkie pliki dolinkowane."

MDeleteHardLink3
"Уничтожать файл?"
"Do you wish to wipe this file?"
"Opravdu chcete vymazat tento soubor?"
"Wollen Sie diese Datei sicher loschen?"
"Biztosan kisopri a fajlt?"
"Czy wymazac plik?"

MCannotDeleteFile
l:
"Ошибка удаления файла"
"Cannot delete the file"
"Nelze smazat soubor"
"Datei konnte nicht geloscht werden"
"A fajl nem torolheto"
"Nie moge usunac pliku"

MCannotDeleteFolder
"Ошибка удаления папки"
"Cannot delete the folder"
"Nelze smazat adresar"
"Ordner konnte nicht geloscht werden"
"A mappa nem torolheto"
"Nie moge usunac katalogu"

MDeleteRetry
"&Повторить"
"&Retry"
"&Znovu"
"Wiede&rholen"
"U&jra"
"&Ponow"

MDeleteSkip
"П&ропустить"
"&Skip"
"Pres&kocit"
"Uber&springen"
"&Kihagy"
"Po&min"

MDeleteSkipAll
"Пропустить &все"
"S&kip all"
"Preskocit &vse"
"A&lle uberspr."
"Min&d"
"Pomin &wszystkie"

MDeleteCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Megsem"
"&Anuluj"

MCannotGetSecurity
l:
"Ошибка получения прав доступа к файлу"
"Cannot get file access rights for"
"Nemohu ziskat pristupova prava pro"
"Kann Zugriffsrechte nicht lesen fur"
"A fajlhoz nincs hozzaferesi joga:"
"Nie moge pobrac praw dostepu dla"

MCannotSetSecurity
"Ошибка установки прав доступа к файлу"
"Cannot set file access rights for"
"Nemohu nastavit pristupova prava pro"
"Kann Zugriffsrechte nicht setzen fur"
"A fajl hozzaferesi jogat nem allithatja:"
"Nie moge ustawic praw dostepu dla"

MEditTitle
l:
"Редактор"
"Editor"
"Editor"
"Editor"
"Szerkeszto"
"Edytor"

MAskReload
"уже загружен. Как открыть этот файл?"
"already loaded. How to open this file?"
"jiz otevren. Jak otevrit tento soubor?"
"bereits geladen. Wie wollen Sie die Datei offnen?"
"fajl mar be van toltve. Hogyan szerkeszti?"
"zostal juz zaladowany. Zaladowac ponownie?"

MCurrent
"&Текущий"
"&Current"
"&Stavajici"
"A&ktuell"
"A mostanit &folytatja"
"&Biezacy"

MReload
"Пере&грузить"
"R&eload"
"&Znovu nacist"
"Aktualisie&ren"
"Ujra&tolti"
"&Zaladuj"

MNewOpen
"&Новая копия"
"&New instance"
"&Nova instance"
"&Neue Instanz"
"U&j peldanyban"
"&Nowa instancja"

MEditCannotOpen
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevrit soubor"
"Kann Datei nicht offnen"
"A fajl nem nyithato meg"
"Nie moge otworzyc pliku"

MEditReading
"Чтение файла"
"Reading the file"
"Nacitam soubor"
"Lesen der Datei"
"Fajl olvasasa"
"Czytam plik"

MEditAskSave
"Файл был изменен"
"File has been modified"
"Soubor byl modifikovan"
"Datei wurde verandert"
"A fajl megvaltozott"
"Plik zostal zmodyfikowany"

MEditAskSaveExt
"Файл был изменен внешней программой"
"The file was changed by an external program"
"Soubor byl zmeneny externim programem"
"Die Datei wurde durch ein externes Programm verandert"
"A fajlt egy kulso program megvaltoztatta"
"Plik zostal zmieniony przez inny program"

MEditSave
l:
"&Сохранить"
"&Save"
"&Ulozit"
"&Speichern"
"&Ment"
"&Zapisz"

MEditNotSave
"&Не сохранять"
"Do &not save"
"&Neukladat"
"&Nicht speichern"
"&Nem ment"
"&Nie zapisuj"

MEditContinue
"&Продолжить редактирование"
"&Continue editing"
"&Pokracovat"
"Bearbeiten f&ortsetzen"
"&Szerkesztest folytat"
"&Kontynuuj edycje"

MEditBtnSaveAs
"Сохр&анить как"
"Save &as..."
"Uloz&it jako..."
"Speichern &als..."
"Mentes mas&kent..."
"Zapisz &jako..."

MEditRO
l:
"имеет атрибут \"Только для чтения\""
"is a read-only file"
"je urcen pouze pro cteni"
"ist eine schreibgeschutzte Datei"
"csak olvashato fajl"
"jest plikiem tylko do odczytu"

MEditExists
"уже существует"
"already exists"
"jiz existuje"
"ist bereits vorhanden"
"mar letezik"
"juz istnieje"

MEditOvr
"Вы хотите перезаписать его?"
"Do you wish to overwrite it?"
"Prejete si ho prepsat?"
"Wollen Sie die Datei uberschreiben?"
"Felul akarja irni?"
"Czy chcesz go nadpisac?"

MEditSaving
"Сохранение файла"
"Saving the file"
"Ukladam soubor"
"Speichere die Datei"
"Fajl mentese"
"Zapisuje plik"

MEditStatusLine
"Строка"
"Line"
"Radek"
"Zeile"
"Sor"
"linia"

MEditStatusCol
"Кол"
"Col"
"Sloupec"
"Spal"
"Oszlop"
"kolumna"

MEditRSH
l:
"предназначен только для чтения"
"is a read-only file"
"je urcen pouze pro cteni"
"ist eine schreibgeschutzte Datei"
"csak olvashato fajl"
"jest plikiem tylko do odczytu"

MEditFileGetSizeError
"Не удалось определить размер."
"File size could not be determined."
upd:"File size could not be determined."
upd:"File size could not be determined."
"A fajlmeret megallapithatatlan."
upd:"File size could not be determined."

MEditFileLong
"имеет размер %s,"
"has the size of %s,"
"ma velikost %s,"
"hat eine Gro?e von %s,"
"merete %s,"
"ma wielkosc %s,"

MEditFileLong2
"что превышает заданное ограничение в %s."
"which exceeds the configured maximum size of %s."
"ktera prekracuje nastavenou maximalni velikost %s."
"die die konfiguierte Maximalgro?e von %s uberschreitet."
"meghaladja %s beallitott maximumat."
"przekraczajaca ustalone maksimum %s."

MEditROOpen
"Вы хотите редактировать его?"
"Do you wish to edit it?"
"Opravdu si ho prejete upravit?"
"Wollen Sie sie dennoch bearbeiten?"
"Megis szerkeszti?"
"Czy chcesz go edytowac?"

MEditCanNotEditDirectory
l:
"Невозможно редактировать папку"
"It is impossible to edit the folder"
"Nelze editovat adresar"
"Es ist nicht moglich den Ordner zu bearbeiten"
"A mappa nem szerkesztheto"
"Nie mozna edytowac folderu"

MEditSearchTitle
l:
"Поиск"
"Search"
"Hledat"
"Suchen"
"Kereses"
"Szukaj"

MEditSearchFor
"&Искать"
"&Search for"
"&Hledat"
"&Suchen nach"
"&Kereses:"
"&Znajdz"

MEditSearchCase
"&Учитывать регистр"
"&Case sensitive"
"&Rozlisovat velikost pismen"
"G&ro?-/Kleinschrb."
"&Nagy/kisbetu erz."
"&Uwzglednij wielkosc liter"

MEditSearchWholeWords
"Только &целые слова"
"&Whole words"
"&Cela slova"
"&Ganze Worter"
"Csak e&gesz szavak"
"Tylko cale slowa"

MEditSearchReverse
"Обратн&ый поиск"
"Re&verse search"
"&Zpetne hledani"
"Richtung um&kehren"
"&Visszafele keres"
"Szukaj w &odwrotnym kierunku"

MEditSearchSelFound
"&Выделять найденное"
"Se&lect found"
"Vy&ber nalezene"
"Treffer &markieren"
"&Talalat kijelolese"
"W&ybierz znalezione"

MEditSearchRegexp
"&Регулярные выражения"
"Re&gular expressions"
upd:"Re&gular expressions"
upd:"Re&gular expressions"
upd:"Re&gular expressions"
upd:"Re&gular expressions"

MEditSearchSearch
"Искать"
"Search"
"Hledat"
"Suchen"
"Kere&ses"
"&Szukaj"

MEditSearchCancel
"Отменить"
"Cancel"
"Storno"
"Abbruch"
"&Megsem"
"&Anuluj"

MEditReplaceTitle
l:
"Замена"
"Replace"
"Nahradit"
"Ersetzen"
"Kereses es csere"
"Zamien"

MEditReplaceWith
"Заменить &на"
"R&eplace with"
"Nahradit &s"
"&Ersetzen mit"
"&Erre csereli:"
"Zamien &na"

MEditReplaceReplace
"&Замена"
"&Replace"
"&Nahradit"
"E&rsetzen"
"&Csere"
"Za&mien"

MEditSearchingFor
l:
"Искать"
"Searching for"
"Vyhledavam"
"Suche nach"
"Keresett szoveg:"
"Szukam"

MEditNotFound
"Строка не найдена"
"Could not find the string"
"Nemuzu najit retezec"
"Konnte Zeichenkette nicht finden"
"A szoveg nem talalhato:"
"Nie moge odnalezc ciagu"

MEditAskReplace
l:
"Заменить"
"Replace"
"Nahradit"
"Ersetze"
"Ezt csereli:"
"Zamienic"

MEditAskReplaceWith
"на"
"with"
"s"
"mit"
"erre a szovegre:"
"na"

MEditReplace
"&Заменить"
"&Replace"
"&Nahradit"
"E&rsetzen"
"&Csere"
"&Zamien"

MEditReplaceAll
"&Все"
"&All"
"&Vse"
"&Alle"
"&Mindet"
"&Wszystkie"

MEditSkip
"&Пропустить"
"&Skip"
"Pres&kocit"
"Uber&springen"
"&Kihagy"
"&Omin"

MEditCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"Me&gsem"
"&Anuluj"

MEditOpenCreateLabel
"&Открыть/создать файл:"
"&Open/create file:"
"Otevrit/vytvorit soubor:"
"Offnen/datei erstellen:"
"Fajl megnyitasa/&letrehozasa:"
"&Otworz/utworz plik:"

MEditOpenAutoDetect
"&Автоматическое определение"
"&Automatic detection"
upd:"Automatic detection"
upd:"Automatic detection"
"&Automatikus felismeres"
"&Wykryj automatycznie"

MEditGoToLine
l:
"Перейти"
"Go to position"
"Jit na pozici"
"Gehe zu Zeile"
"Sorra ugras"
"Idz do linii"

MFolderShortcutsTitle
l:
"Ссылки на папки"
"Folder shortcuts"
"Adresarove zkratky"
"Ordnerschnellzugriff"
"Mappa gyorsbillentyuk"
"Skroty katalogow"

MFolderShortcutBottom
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Bearb.: Entf,Einf,F4"
"Szerk.: Del,Ins,F4"
"Edycja: Del,Ins,F4"

MShortcutNone
"<отсутствует>"
"<none>"
"<neni>"
"<keiner>"
"<nincs>"
"<brak>"

MShortcutPlugin
"<плагин>"
"<plugin>"
"<plugin>"
"<Plugin>"
"<plugin>"
"<plugin>"

MEnterShortcut
"Введите новую ссылку:"
"Enter new shortcut:"
"Zadejte novou zkratku:"
"Neue Verknupfung:"
"A gyorsbillentyuhoz rendelt mappa:"
"Wprowadz nowy skrot:"

MNeedNearPath
"Перейти в ближайшую доступную папку?"
"Jump to the nearest existing folder?"
"Skocit na nejblizsi existujici adresar?"
"Zum nahesten existierenden Ordner springen?"
"Ugras a legkozelebbi letezo mappara?"
"Przejsc do najblizszego istniejacego folderu?"

MSaveThisShortcut
"Запомнить эту ссылку?"
"Save this shortcuts?"
"Ulozit tyto zkratky?"
"Verknupfung speichern?"
"Mentsem a gyorsbillentyuket?"
"Zapisac skroty?"

MEditF1
l:
l://functional keys - 6 characters max, 12 keys, "OEM" is F8 dupe!
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Sugo"
"Pomoc"

MEditF2
"Сохран"
"Save"
"Ulozit"
"Speich"
"Mentes"
"Zapisz"

MEditF3
""
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
""

MEditF5
""
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
"Megnez"
"Zobacz"

MEditF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"
"Szukaj"

MEditF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"Latin 2"

MEditF9
""
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
"Kilep"
"Koniec"

MEditF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MEditF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Keprny"
"Ekran"

MEditF8DOS
le:// don't count this - it's a F8 another text
"OEM"
"OEM"
"OEM"
"OEM"
"OEM"
"CP-1250"

MEditShiftF1
l:
l://Editor: Shift
""
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
"Zapisz"

MEditShiftF3
""
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
"Edytuj"

MEditShiftF5
""
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
""

MEditShiftF7
"Дальше"
"Next"
"Dalsi"
"Nachst"
"TovKer"
"Nastep"

MEditShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kodlap"
"Tabela"

MEditShiftF9
""
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
"ZapKon"

MEditShiftF11
""
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
""

MEditAltF1
l:
l://Editor: Alt
""
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
""

MEditAltF3
""
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
""

MEditAltF5
"Печать"
"Print"
"Tisk"
"Druck"
"Nyomt"
"Drukuj"

MEditAltF6
""
""
""
""
""
""

MEditAltF7
"Назад"
"Prev"
"Predch"
"Letzt"
"VisKer"
"Poprz"

MEditAltF8
"Строка"
"Goto"
"Jit na"
"GeheZu"
"Ugras"
"IdzDo"

MEditAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"

MEditAltF10
""
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
"NezElo"
"Historia"

MEditAltF12
""
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
""

MEditCtrlF2
""
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
""

MEditCtrlF4
""
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
""

MEditCtrlF6
""
""
""
""
""
""

MEditCtrlF7
"Замена"
"Replac"
"Nahrad"
"Ersetz"
"Csere"
"Zamien"

MEditCtrlF8
""
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
""

MEditCtrlF10
"Позиц"
"GoFile"
"JdiSou"
"GehDat"
"FajlPz"
"GoFile"

MEditCtrlF11
""
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
""

MEditAltShiftF1
l:
l://Editor: AltShift
""
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
""

MEditAltShiftF3
""
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
""

MEditAltShiftF5
""
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
""

MEditAltShiftF7
""
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
""

MEditAltShiftF9
"Конфиг"
"Config"
"Nastav"
"Konfig"
"Beall."
"Konfig"

MEditAltShiftF10
""
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
""

MEditAltShiftF12
""
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
""

MEditCtrlShiftF2
""
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
""

MEditCtrlShiftF4
""
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
""

MEditCtrlShiftF6
""
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
""

MEditCtrlShiftF8
""
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
""

MEditCtrlShiftF10
""
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
""

MEditCtrlShiftF12
""
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
""

MEditCtrlAltF2
""
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
""

MEditCtrlAltF4
""
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
""

MEditCtrlAltF6
""
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
""

MEditCtrlAltF8
""
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
""

MEditCtrlAltF10
""
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
""

MEditCtrlAltF12
""
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
""

MEditCtrlAltShiftF2
""
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
""

MEditCtrlAltShiftF4
""
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
""

MEditCtrlAltShiftF6
""
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
""

MEditCtrlAltShiftF8
""
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
""

MEditCtrlAltShiftF10
""
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
""

MEditCtrlAltShiftF12
le://End of functional keys (Editor)
""
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
"Sugo"
"Pomoc"

MSingleEditF2
"Сохран"
"Save"
"Ulozit"
"Speich"
"Mentes"
"Zapisz"

MSingleEditF3
""
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
""

MSingleEditF5
""
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
"Megnez"
"Zobacz"

MSingleEditF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"
"Szukaj"

MSingleEditF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"Latin 2"

MSingleEditF9
""
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
"Kilep"
"Koniec"

MSingleEditF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MSingleEditF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Keprny"
"Ekran"

MSingleEditF8DOS
le:// don't count this - it's a F8 another text
"OEM"
"OEM"
"OEM"
"OEM"
"OEM"
"CP 1250"

MSingleEditShiftF1
l:
l://Single Editor: Shift
""
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
"Zapisz"

MSingleEditShiftF3
""
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
""

MSingleEditShiftF5
""
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
""

MSingleEditShiftF7
"Дальше"
"Next"
"Dalsi"
"Nachst"
"TovKer"
"Nastep"

MSingleEditShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kodlap"
"Tabela"

MSingleEditShiftF9
""
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
"ZapKon"

MSingleEditShiftF11
""
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
""

MSingleEditAltF1
l:
l://Single Editor: Alt
""
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
""

MSingleEditAltF3
""
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
""

MSingleEditAltF5
"Печать"
"Print"
"Tisk"
"Druck"
"Nyomt"
"Drukuj"

MSingleEditAltF6
""
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
""

MSingleEditAltF8
"Строка"
"Goto"
"Jit na"
"GeheZu"
"Ugras"
"IdzDo"

MSingleEditAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Ekran"

MSingleEditAltF10
""
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
"NezElo"
"ZobHs"

MSingleEditAltF12
""
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
""

MSingleEditCtrlF2
""
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
""

MSingleEditCtrlF4
""
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
""

MSingleEditCtrlF6
""
""
""
""
""
""

MSingleEditCtrlF7
"Замена"
"Replac"
"Nahrad"
"Ersetz"
"Csere"
"Zastap"

MSingleEditCtrlF8
""
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
""

MSingleEditCtrlF10
""
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
""

MSingleEditCtrlF12
""
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
""

MSingleEditAltShiftF2
""
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
""

MSingleEditAltShiftF4
""
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
""

MSingleEditAltShiftF6
""
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
""

MSingleEditAltShiftF8
""
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
"Beall."
"Konfig"

MSingleEditAltShiftF10
""
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
""

MSingleEditAltShiftF12
""
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
""

MSingleEditCtrlShiftF2
""
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
""

MSingleEditCtrlShiftF4
""
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
""

MSingleEditCtrlShiftF6
""
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
""

MSingleEditCtrlShiftF8
""
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
""

MSingleEditCtrlShiftF10
""
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
""

MSingleEditCtrlShiftF12
""
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
""

MSingleEditCtrlAltF2
""
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
""

MSingleEditCtrlAltF4
""
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
""

MSingleEditCtrlAltF6
""
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
""

MSingleEditCtrlAltF8
""
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
""

MSingleEditCtrlAltF10
""
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
""

MSingleEditCtrlAltF12
""
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
""

MSingleEditCtrlAltShiftF2
""
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
""

MSingleEditCtrlAltShiftF4
""
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
""

MSingleEditCtrlAltShiftF6
""
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
""

MSingleEditCtrlAltShiftF8
""
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
""

MSingleEditCtrlAltShiftF10
""
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
""

MSingleEditCtrlAltShiftF12
le://End of functional keys (Single Editor)
""
""
""
""
""
""

MEditSaveAs
l:
"Сохранить &файл как"
"Save file &as"
"Ulozit soubor jako"
"Speichern &als"
"Fa&jl mentese, mint:"
"Zapisz plik &jako"

MEditCodePage
"&Кодовая страница:"
"&Code page:"
"Kodova stranka:"
"Codepage:"
"Kodlap:"
"&Strona kodowa:"

MEditAddSignature
"Добавить &сигнатуру (BOM)"
"Add &signature (BOM)"
"Pridat signaturu (BOM)"
"Sinatur hinzu (BOM)"
"Uni&code bajtsorrend jelzovel (BOM)"
"Dodaj &znacznik BOM"

MEditSaveAsFormatTitle
"Изменить перевод строки:"
"Change line breaks to:"
"Zmenit zakonceni radku na:"
"Zeilenumbruche setzen:"
"Sortores konverzio:"
"Zamien znaki konca linii na:"

MEditSaveOriginal
"&исходный формат"
"Do n&ot change"
"&Beze zmeny"
"Nicht vera&ndern"
"Nincs &konverzio"
"&Nie zmieniaj"

MEditSaveDOS
"в форма&те DOS/Windows (CR LF)"
"&Dos/Windows format (CR LF)"
"&Dos/Windows format (CR LF)"
"&Dos/Windows Format (CR LF)"
"&DOS/Windows formatum (CR LF)"
"Format &Dos/Windows (CR LF)"

MEditSaveUnix
"в формат&е UNIX (LF)"
"&Unix format (LF)"
"&Unix format (LF)"
"&Unix Format (LF)"
"&UNIX formatum (LF)"
"Format &Unix (LF)"

MEditSaveMac
"в фор&мате MAC (CR)"
"&Mac format (CR)"
"&Mac format (CR)"
"&Mac Format (CR)"
"&Mac formatum (CR)"
"Format &Mac (CR)"

MEditCannotSave
"Ошибка сохранения файла"
"Cannot save the file"
"Nelze ulozit soubor"
"Kann die Datei nicht speichern"
"A fajl nem mentheto"
"Nie moge zapisac pliku"

MEditSavedChangedNonFile
"Файл изменен, но файл или папка, в которой он находился,"
"The file is changed but the file or the folder containing"
"Soubor je zmenen, ale soubor, nebo adresar obsahujici"
"Inhalt dieser Datei wurde verandert aber die Datei oder der Ordner, welche"
"A fajl megvaltozott, de a fajlt vagy a mappajat"
"Plik zostal zmieniony, ale plik lub folder zawierajacy"

MEditSavedChangedNonFile1
"Файл или папка, в которой он находился,"
"The file or the folder containing"
"Soubor nebo adresar obsahujici"
"Die Datei oder der Ordner, welche"
"A fajlt vagy a mappajat"
"Plik lub folder zawierajacy"

MEditSavedChangedNonFile2
"был перемещен или удален."
"this file was moved or deleted."
"tento soubor byl presunut, nebo smazan."
"diesen Inhalt enthalt wurde verschoben oder geloscht."
"idokozben athelyezte/atnevezte vagy torolte."
"ten plik zostal przeniesiony lub usuniety."

MEditNewPath1
"Путь к редактируемому файлу не существует,"
"The path to the edited file does not exist,"
"Cesta k editovanemu souboru neexistuje,"
"Der Pfad zur bearbeiteten Datei existiert nicht,"
"A szerkesztendo fajl celmappaja meg"
"Sciezka do edytowanego pliku nie istnieje,"

MEditNewPath2
"но будет создан при сохранении файла."
"but will be created when the file is saved."
"ale bude vytvorena pri ulozeni souboru."
"aber wird erstellt sobald die Datei gespeichert wird."
"nem letezik, de menteskor letrejon."
"ale zostanie utworzona po zapisaniu pliku."

MEditNewPath3
"Продолжать?"
"Continue?"
"Pokracovat?"
"Fortsetzen?"
"Folytatja?"
"Kontynuowac?"

MEditNewPlugin1
"Имя редактируемого файла не может быть пустым"
"The name of the file to edit cannot be empty"
"Nazev souboru k editaci nesmi byt prazdne"
"Der Name der zu editierenden Datei kann nicht leer sein"
"A szerkesztendo fajlnak nevet kell adni"
"Nazwa pliku do edycji nie moze byc pusta"

MEditDataLostWarn1
"Файл содержит символы, которые невозможно"
"This file contains characters, which cannot be"
"Tento soubor obsahuje znaky, ktere nemohou byt"
"Die Datei enthalt Zeichen, welche mit der aktuellen Codepage"
"A fajl olyan karaktereket tartalmaz, amelyek a"
"Plik zawiera znaki, ktore nie moga byc"

MEditDataLostWarn2
"корректно преобразовать в выбранную кодировку."
"correctly translated using the selected codepage."
"korektne prelozeny do zvoleneho kodovani."
"nicht korrekt angezeigt werden konnen."
"kivalasztott kodlappal nem ertelmezhetok helyesen."
"poprawnie zapisane w wybranej stronie kodowej."

MEditDataLostWarn3
"Продолжить?"
"Continue?"
"Pokracovat?"
"Fortsetzen?"
"Folytatja?"
"Kontynuowac?"

MEditDataLostWarn4
"Сохранять файл не рекомендуется."
"It is not recommended to save this file."
"Neni doporuceno ulozit tento soubor."
"Es wird empfohlen, die Datei nicht zu speichern."
"A fajl mentese nem ajanlott."
"Odradzamy zapis pliku."

MColumnName
l:
"Имя"
"Name"
"Nazev"
"Name"
"Nev"
"Nazwa"

MColumnSize
"Размер"
"Size"
"Velikost"
"Gro?e"
"Meret"
"Rozmiar"

MColumnPacked
"Упаков"
"Packed"
"Komprimovany"
"Kompr."
"TMeret"
"Spakowany"

MColumnDate
"Дата"
"Date"
"Datum"
"Datum"
"Datum"
"Data"

MColumnTime
"Время"
"Time"
"Cas"
"Zeit"
"Ido"
"Czas"

MColumnModified
"Модификация"
"Modified"
"Modifikovan"
"Verandert"
"Modositva"
"Modyfikacja"

MColumnCreated
"Создание"
"Created"
"Vytvoren"
"Erstellt"
"Letrejott"
"Utworzenie"

MColumnAccessed
"Доступ"
"Accessed"
"Pristup"
"Zugriff"
"Hozzaferes"
"Uzycie"

MColumnAttr
"Атриб"
"Attr"
"Attr"
"Attr"
"Attrib"
"Atrybuty"

MColumnDescription
"Описание"
"Description"
"Popis"
"Beschreibung"
"Megjegyzes"
"Opis"

MColumnOwner
"Владелец"
"Owner"
"Vlastnik"
"Besitzer"
"Tulajdonos"
"Wlasciciel"

MColumnMumLinks
"КлС"
"NmL"
"PocLn"
"AnL"
"Lnk"
"NmL"

MColumnNumStreams
"КлП"
"NmS"
upd:"NmS"
upd:"NmS"
"Stm"
upd:"NmS"

MColumnStreamsSize
"РазмПт"
"StrmSz"
upd:"StrmSz"
upd:"StrmSz"
"StmMer"
upd:"StrmSz"

MListUp
l:
"Вверх"
"  Up  "
"Nahoru"
" Hoch "
"  Fel  "
"W gore"

MListFolder
"Папка"
"Folder"
"Adresar"
"Ordner"
" Mappa "
"Folder"

MListSymLink
"Ссылка"
"Symlink"
"Link"
"Symlink"
"SzimLnk"
"LinkSym"

MListJunction
"Связь"
"Junction"
"Krizeni"
"Knoten"
"Csomopt"
"Dowiazania"

MListVolMount
"Том"
"Volume"
"Svazek"
"Datentrager"
"Kotet"
"Naped"

MListBytes
"Б"
"B"
"B"
"B"
"B"
"B"

MListKb
"К"
"K"
"K"
"K"
"k"
"K"

MListMb
"М"
"M"
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
"G"

MListTb
"Т"
"T"
"T"
"T"
"T"
"T"

MListPb
"П"
"P"
"P"
"P"
"P"
"P"

MListEb
"Э"
"E"
"E"
"E"
"E"
"E"

MListFileSize
" %s байт в 1 файле "
" %s bytes in 1 file "
" %s bytu v 1 souboru "
" %s Bytes in 1 Datei "
" %s bajt 1 fajlban "
" %s bajtow w 1 pliku "

MListFilesSize1
" %s байт в %d файле "
" %s bytes in %d files "
" %s bytu v %d souborech "
" %s Bytes in %d Dateien "
" %s bajt %d fajlban "
" %s bajtow w %d plikach "

MListFilesSize2
" %s байт в %d файлах "
" %s bytes in %d files "
" %s bytu v %d souborech "
" %s Bytes in %d Dateien "
" %s bajt %d fajlban "
" %s bajtow w %d plikach "

MListFreeSize
" %s байт свободно "
" %s free bytes "
" %s volnych bytu "
" %s freie Bytes "
" %s szabad bajt "
" %s wolnych bajtow "

MDirInfoViewTitle
l:
"Просмотр"
"View"
"Zobraz"
"Betrachten"
"Vizsgalat"
"Podglad"

MFileToEdit
"Редактировать файл:"
"File to edit:"
"Soubor k editaci:"
"Datei bearbeiten:"
"Szerkesztendo fajl:"
"Plik do edycji:"

MUnselectTitle
l:
"Снять"
"Deselect"
"Odznacit"
"Abwahlen"
"Kijelolest levesz"
"Odznacz"

MSelectTitle
"Пометить"
"Select"
"Oznacit"
"Auswahlen"
"Kijeloles"
"Zaznacz"

MSelectFilter
"&Фильтр"
"&Filter"
"&Filtr"
"&Filter"
"&Szuro"
"&Filtruj"

MCompareTitle
l:
"Сравнение"
"Compare"
"Porovnat"
"Vergleichen"
"Osszehasonlitas"
"Porownaj"

MCompareFilePanelsRequired1
"Для сравнения папок требуются"
"Two file panels are required to perform"
"Pro provedeni prikazu Porovnat adresare"
"Zwei Dateipanels werden benotigt um"
"Mappak osszehasonlitasahoz"
"Aby porownac katalogi konieczne sa"

MCompareFilePanelsRequired2
"две файловые панели"
"the Compare folders command"
"jsou nutne dva souborove panely"
"den Vergleich auszufuhren."
"ket fajlpanel szukseges"
"dwa zwykle panele plikow"

MCompareSameFolders1
"Содержимое папок,"
"The folders contents seems"
"Obsahy adresaru jsou"
"Der Inhalt der beiden Ordner scheint"
"A mappak tartalma"
"Zawartosc katalogow"

MCompareSameFolders2
"скорее всего, одинаково"
"to be identical"
"identicke"
"identisch zu sein."
"azonosnak tunik"
"wydaje sie byc identyczna"

MSelectAssocTitle
l:
"Выберите ассоциацию"
"Select association"
"Vyber zavislosti"
"Dateiverknupfung auswahlen"
"Valasszon tarsitast"
"Wybierz przypisanie"

MAssocTitle
l:
"Ассоциации для файлов"
"File associations"
"Zavislosti souboru"
"Dateiverknupfungen"
"Fajltarsitasok"
"Przypisania plikow"

MAssocBottom
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Bearb.: Entf,Einf,F4"
"Szerk.: Del,Ins,F4"
"Edycja: Del,Ins,F4"

MAskDelAssoc
"Вы хотите удалить ассоциацию для"
"Do you wish to delete association for"
"Prejete si smazat zavislost pro"
"Wollen Sie die Verknupfung loschen fur"
"Torolni szeretne a tarsitast?"
"Czy chcesz usunac przypisanie dla"

MFileAssocTitle
l:
"Редактирование ассоциаций файлов"
"Edit file associations"
"Upravit zavislosti souboru"
"Dateiverknupfungen bearbeiten"
"Fajltarsitasok szerkesztese"
"Edytuj przypisania pliku"

MFileAssocMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"
"&Maska nebo masky souboru:"
"Datei&maske (mehrere getrennt mit Komma):"
"F&ajlmaszk(ok, vesszovel elvalasztva):"
"&Maska pliku lub kilka masek oddzielonych przecinkami:"

MFileAssocDescr
"&Описание ассоциации:"
"&Description of the association:"
"&Popis asociaci:"
"&Beschreibung der Verknupfung:"
"A &tarsitas leirasa:"
"&Opis przypisania:"

MFileAssocExec
"Команда, &выполняемая по Enter:"
"E&xecute command (used for Enter):"
"&Vykonat prikaz (pouzito pro Enter):"
"Befehl &ausfuhren (mit Enter):"
"&Vegrehajtando parancs (Enterre):"
"Polecenie (po nacisnieciu &Enter):"

MFileAssocAltExec
"Коман&да, выполняемая по Ctrl-PgDn:"
"Exec&ute command (used for Ctrl-PgDn):"
"V&ykonat prikaz (pouzito pro Ctrl-PgDn):"
"Befehl a&usfuhren (mit Strg-PgDn):"
"Ve&grehajtando parancs (Ctrl-PgDown-ra):"
"Polecenie (po nacisnieciu &Ctrl-PgDn):"

MFileAssocView
"Команда &просмотра, выполняемая по F3:"
"&View command (used for F3):"
"Prikaz &Zobraz (pouzito pro F3):"
"Be&trachten (mit F3):"
"&Nezoke parancs (F3-ra):"
"&Podglad (po nacisnieciu F3):"

MFileAssocAltView
"Команда просмотра, в&ыполняемая по Alt-F3:"
"V&iew command (used for Alt-F3):"
"Prikaz Z&obraz (pouzito pro Alt-F3):"
"Bet&rachten (mit Alt-F3):"
"N&ezoke parancs (Alt-F3-ra):"
"Podg&lad (po nacisnieciu Alt-F3):"

MFileAssocEdit
"Команда &редактирования, выполняемая по F4:"
"&Edit command (used for F4):"
"Prikaz &Edituj (pouzito pro F4):"
"Bearb&eiten (mit F4):"
"S&zerkesztes parancs (F4-re):"
"&Edycja  (po nacisnieciu F4):"

MFileAssocAltEdit
"Команда редактировани&я, выполняемая по Alt-F4:"
"Edit comm&and (used for Alt-F4):"
"Prikaz Editu&j (pouzito pro Alt-F4):"
"Bearbe&iten (mit Alt-F4):"
"Sze&rkesztes parancs (Alt-F4-re):"
"E&dycja  (po nacisnieciu Alt-F4):"

MViewF1
l:
l://Viewer: functional keys, 12 keys, except F2 - 2 keys, and F8 - 2 keys
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Sugo"
"Pomoc"

MViewF2
le:// this is another text for F2
"Сверн"
"Wrap"
"Zalom"
"Umbr."
"SorTor"
"Zawin"

MViewF3
"Выход"
"Quit"
"Konec"
"Ende"
"Kilep"
"Koniec"

MViewF4
"Код"
"Hex"
"Hex"
"Hex"
"Hexa"
"Hex"

MViewF5
""
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
"Edytuj"

MViewF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"
"Szukaj"

MViewF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"Latin 2"

MViewF9
""
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
"Kilep"
"Koniec"

MViewF11
"Модули"
"Plugins"
"Plugin"
"Plugin"
"Plugin"
"Pluginy"

MViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Keprny"
"Ekran"

MViewF2Unwrap
"Развер"
"Unwrap"
"Nezal"
"KeinUm"
"NemTor"
"Unwrap"

MViewF4Text
l:// this is another text for F4
"Текст"
"Text"
"Text"
"Text"
"Szoveg"
"Tekst"

MViewF8DOS
"OEM"
"OEM"
"OEM"
"OEM"
"OEM"
"CP 1250"

MViewShiftF1
l:
l://Viewer: Shift
""
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
"SzoTor"
"ZawinS"

MViewShiftF3
""
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
""

MViewShiftF5
""
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
""

MViewShiftF7
"Дальше"
"Next"
"Dalsi"
"Nachst"
"TovKer"
"Nastep"

MViewShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kodlap"
"Tabela"

MViewShiftF9
""
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
""

MViewShiftF11
""
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
""

MViewAltF1
l:
l://Viewer: Alt
""
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
""

MViewAltF3
""
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
""

MViewAltF5
"Печать"
"Print"
"Tisk"
"Druck"
"Nyomt"
"Drukuj"

MViewAltF6
""
""
""
""
""
""

MViewAltF7
"Назад"
"Prev"
"Predch"
"Letzt"
"VisKer"
"Poprz"

MViewAltF8
"Перейт"
"Goto"
"Jit na"
"GeheZu"
"Ugras"
"IdzDo"

MViewAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"

MViewAltF10
""
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
"NezElo"
"Historia"

MViewAltF12
""
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
""

MViewCtrlF2
""
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
""

MViewCtrlF4
""
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
""

MViewCtrlF6
""
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
""

MViewCtrlF8
""
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
""

MViewCtrlF10
"Позиц"
"GoFile"
"JitSou"
"GehDat"
"FajlPz"
"DoPlik"

MViewCtrlF11
""
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
""

MViewAltShiftF1
l:
l://Viewer: AltShift
""
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
""

MViewAltShiftF3
""
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
""

MViewAltShiftF5
""
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
""

MViewAltShiftF7
""
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
""

MViewAltShiftF9
"Конфиг"
"Config"
"Nastav"
"Konfig"
"Beall."
"Konfig"

MViewAltShiftF10
""
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
""

MViewAltShiftF12
""
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
""

MViewCtrlShiftF2
""
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
""

MViewCtrlShiftF4
""
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
""

MViewCtrlShiftF6
""
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
""

MViewCtrlShiftF8
""
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
""

MViewCtrlShiftF10
""
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
""

MViewCtrlShiftF12
""
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
""

MViewCtrlAltF2
""
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
""

MViewCtrlAltF4
""
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
""

MViewCtrlAltF6
""
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
""

MViewCtrlAltF8
""
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
""

MViewCtrlAltF10
""
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
""

MViewCtrlAltF12
""
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
""

MViewCtrlAltShiftF2
""
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
""

MViewCtrlAltShiftF4
""
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
""

MViewCtrlAltShiftF6
""
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
""

MViewCtrlAltShiftF8
""
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
""

MViewCtrlAltShiftF10
""
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
""

MViewCtrlAltShiftF12
le://end of functional keys (Viewer)
""
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
"Sugo"
"Pomoc"

MSingleViewF2
"Сверн"
"Wrap"
"Zalom"
"Umbr."
"SorTor"
"Zawin"

MSingleViewF3
"Выход"
"Quit"
"Konec"
"Ende"
"Kilep"
"Koniec"

MSingleViewF4
"Код"
"Hex"
"Hex"
"Hex"
"Hexa"
"Hex"

MSingleViewF5
""
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
"Edytuj"

MSingleViewF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"
"Szukaj"

MSingleViewF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"Latin 2"

MSingleViewF9
""
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
"Kilep"
"Koniec"

MSingleViewF11
"Модули"
"Plugins"
"Plugin"
"Plugins"
"Plugin"
"Pluginy"

MSingleViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Keprny"
"Ekran"

MSingleViewF2Unwrap
l:// this is another text for F2
"Развер"
"Unwrap"
"Nezal"
"KeinUm"
"NemTor"
"Rozwij"

MSingleViewF4Text
l:// this is another text for F4
"Текст"
"Text"
"Text"
"Text"
"Szoveg"
"Tekst"

MSingleViewF8DOS
"OEM"
"OEM"
"OEM"
"OEM"
"OEM"
"CP 1250"

MSingleViewShiftF1
l:
l://Single Viewer: Shift
""
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
"SzoTor"
"ZawinS"

MSingleViewShiftF3
""
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
""

MSingleViewShiftF5
""
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
""

MSingleViewShiftF7
"Дальше"
"Next"
"Dalsi"
"Nachst"
"TovKer"
"Nast."

MSingleViewShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kodlap"
"Tabela"

MSingleViewShiftF9
""
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
""

MSingleViewShiftF11
""
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
""

MSingleViewAltF1
l:
l://Single Viewer: Alt
""
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
""

MSingleViewAltF3
""
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
""

MSingleViewAltF5
"Печать"
"Print"
"Tisk"
"Druck"
"Nyomt"
"Drukuj"

MSingleViewAltF6
""
""
""
""
""
""

MSingleViewAltF7
"Назад"
"Prev"
"Predch"
"Letzt"
"VisKer"
"Poprz"

MSingleViewAltF8
"Перейт"
"Goto"
"Jit na"
"GeheZu"
"Ugras"
"IdzDo"

MSingleViewAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"

MSingleViewAltF10
""
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
"NezElo"
"Historia"

MSingleViewAltF12
""
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
""

MSingleViewCtrlF2
""
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
""

MSingleViewCtrlF4
""
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
""

MSingleViewCtrlF6
""
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
""

MSingleViewCtrlF8
""
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
""

MSingleViewCtrlF10
""
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
""

MSingleViewCtrlF12
""
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
""

MSingleViewAltShiftF2
""
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
""

MSingleViewAltShiftF4
""
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
""

MSingleViewAltShiftF6
""
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
""

MSingleViewAltShiftF8
""
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
"Beall."
"Konfig"

MSingleViewAltShiftF10
""
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
""

MSingleViewAltShiftF12
""
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
""

MSingleViewCtrlShiftF2
""
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
""

MSingleViewCtrlShiftF4
""
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
""

MSingleViewCtrlShiftF6
""
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
""

MSingleViewCtrlShiftF8
""
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
""

MSingleViewCtrlShiftF10
""
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
""

MSingleViewCtrlShiftF12
""
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
""

MSingleViewCtrlAltF2
""
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
""

MSingleViewCtrlAltF4
""
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
""

MSingleViewCtrlAltF6
""
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
""

MSingleViewCtrlAltF8
""
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
""

MSingleViewCtrlAltF10
""
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
""

MSingleViewCtrlAltF12
""
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
""

MSingleViewCtrlAltShiftF2
""
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
""

MSingleViewCtrlAltShiftF4
""
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
""

MSingleViewCtrlAltShiftF6
""
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
""

MSingleViewCtrlAltShiftF8
""
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
""

MSingleViewCtrlAltShiftF10
""
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
""

MSingleViewCtrlAltShiftF12
le://end of functional keys (Single Viewer)
""
""
""
""
""
""

MInViewer
"просмотр %s"
"view %s"
"prohlizeni %s"
"Betrachte %s"
"%s megnezese"
"podglad %s"

MInEditor
"редактирование %s"
"edit %s"
"editace %s"
"Bearbeite %s"
"%s szerkesztese"
"edycja %s"

MFilterTitle
l:
"Меню фильтров"
"Filters menu"
"Menu filtru"
"Filtermenu"
"Szurok menu"
"Filtry"

MFilterBottom
"+,-,Пробел,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Space,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Mezera,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Leer,I,X,BS,UmschBS,Einf,Entf,F4,F5,StrgUp,StrgDn"
"+,-,Szokoz,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Fel,Ctrl-Le"
"+,-,Spacja,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"

MPanelFileType
"Файлы панели"
"Panel file type"
"Typ panelu souboru"
"Dateityp in Panel"
"A panel fajltipusa"
"Typ plikow w panelu"

MFolderFileType
"Папки"
"Folders"
"Adresare"
"Ordner"
"Mappak"
"Foldery"

MCanEditCustomFilterOnly
"Только пользовательский фильтр можно редактировать"
"Only custom filter can be edited"
"Jedine vlastni filtr muze byt upraven"
"Nur eigene Filter konnen editiert werden."
"Csak sajat szuro szerkesztheto"
"Tylko filtr uzytkownika moze byc edytowany"

MAskDeleteFilter
"Вы хотите удалить фильтр"
"Do you wish to delete the filter"
"Prejete si smazat filtr"
"Wollen Sie den eigenen Filter loschen"
"Torolni szeretne a szurot?"
"Czy chcesz usunac filtr"

MCanDeleteCustomFilterOnly
"Только пользовательский фильтр может быть удален"
"Only custom filter can be deleted"
"Jedine vlastni filtr muze byt smazan"
"Nur eigene Filter konnen geloscht werden."
"Csak sajat szuro torolheto"
"Tylko filtr uzytkownika moze byc usuniety"

MFindFileTitle
l:
"Поиск файла"
"Find file"
"Hledat soubor"
"Nach Dateien suchen"
"Fajlkereses"
"Znajdz plik"

MFindFileResultTitle
"Поиск файла - результат"
"Find file - result"
"Hledat soubor - vysledek"
"Suchergebnisse"
"Fajlkereses eredmenye"
"Wynik poszukiwania"

MFindFileMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"
"Maska nebo masky souboru:"
"Datei&maske (mehrere getrennt mit Komma):"
"Fajlm&aszk(ok, vesszovel elvalasztva):"
"&Maska pliku lub kilka masek oddzielonych przecinkami:"

MFindFileText
"&Содержащих текст:"
"Con&taining text:"
"Obsahujici te&xt:"
"Enthalt &Text:"
"&Tartalmazza a szoveget:"
"Zawierajacy &tekst:"

MFindFileHex
"&Содержащих 16-ричный код:"
"Con&taining hex:"
"Obsahujici &hex:"
"En&thalt Hex (xx xx ...):"
"Tartalmazza a he&xat:"
"Zawierajacy wartosc &szesnastkowa:"

MFindFileCodePage
"Используя кодо&вую страницу:"
"Using code pa&ge:"
upd:"Pouzit &znakovou sadu:"
upd:"Zeichenta&belle verwenden:"
"Ko&dlap:"
"Uzyj tablicy znakow:"

MFindFileCodePageBottom
"Space, Ins"
"Space, Ins"
"Space, Ins"
"Space, Ins"
"Space, Ins"
"Space, Ins"

MFindFileCase
"&Учитывать регистр"
"&Case sensitive"
"Roz&lisovat velikost pismen"
"Gr&o?-/Kleinschreibung"
"&Nagy/kisbetu erzekeny"
"&Uwzglednij wielkosc liter"

MFindFileWholeWords
"Только &целые слова"
"&Whole words"
"&Cela slova"
"Nur &ganze Worter"
"Csak eges&z szavak"
"Tylko &cale slowa"

MFindFileAllCodePages
"Все кодовые страницы"
"All code pages"
upd:"Vsechny znakove sady"
upd:"Alle Zeichentabellen"
"Minden kodlappal"
"Wszystkie zainstalowane"

MFindArchives
"Искать в а&рхивах"
"Search in arch&ives"
"Hledat v a&rchivech"
"In Arch&iven suchen"
"Kereses t&omoritettekben"
"Szukaj w arc&hiwach"

MFindFolders
"Искать п&апки"
"Search for f&olders"
"Hledat a&dresare"
"Nach &Ordnern suchen"
"Kereses mapp&akra"
"Szukaj &folderow"

MFindSymLinks
"Искать в символи&ческих ссылках"
"Search in symbolic lin&ks"
"Hledat v s&ymbolickych lincich"
"In symbolischen Lin&ks suchen"
"Kereses sz&imbolikus linkekben"
"Szukaj w &linkach"

MSearchForHex
"Искать 16-ричн&ый код"
"Search for &hex"
"Hledat &hex"
"Nach &Hex suchen"
"Kereses &hexakra"
"Szukaj wartosci &szesnastkowej"

MSearchWhere
"Выберите &область поиска:"
"Select search &area:"
upd:"Zvolte oblast hledani:"
upd:"Suchbereich:"
"Kereses hatos&ugara:"
"Obszar wyszukiwania:"

MSearchAllDisks
"На всех несъемных &дисках"
"In &all non-removable drives"
"Ve vsech p&evnych discich"
"Auf &allen festen Datentrager"
"Minden &fix meghajton"
"Na dyskach &stalych"

MSearchAllButNetwork
"На всех &локальных дисках"
"In all &local drives"
"Ve vsech &lokalnich discich"
"Auf allen &lokalen Datentragern"
"Minden hel&yi meghajton"
"Na dyskach &lokalnych"

MSearchInPATH
"В PATH-катало&гах"
"In &PATH folders"
"V adresarich z &PATH"
"In &PATH-Ordnern"
"A &PATH mappaiban"
"W folderach zmiennej &PATH"

MSearchFromRootOfDrive
"С кор&ня диска"
"From the &root of"
"V &koreni"
"Ab Wu&rzelverz. von"
"Meghajto &gyokeretol:"
"Od &korzenia"

MSearchFromRootFolder
"С кор&невой папки"
"From the &root folder"
"V koreno&vem adresari"
"Ab Wu&rzelverzeichnis"
"A &gyokermappatol"
"Od katalogu &glownego"

MSearchFromCurrent
"С &текущей папки"
"From the curre&nt folder"
"V tomto adresar&i"
"Ab dem aktuelle&n Ordner"
"Az akt&ualis mappatol"
"Od &biezacego katalogu"

MSearchInCurrent
"Только в теку&щей папке"
"The current folder onl&y"
"P&ouze v tomto adresari"
"Nur im aktue&llen Ordner"
"&Csak az aktualis mappaban"
"&Tylko w biezacym katalogu"

MSearchInSelected
"В &отмеченных папках"
"&Selected folders"
"Ve vy&branych adresarich"
"In au&sgewahlten Ordner"
"A ki&jelolt mappakban"
"W &zaznaczonych katalogach"

MFindUseFilter
"Исполь&зовать фильтр"
"&Use filter"
"Pouzit f&iltr"
"Ben&utze Filter"
"Sz&urovel"
"&Filtruj"

MFindUsingFilter
"используя фильтр"
"using filter"
"pouzivam filtr"
"mit Filter"
"szurovel"
"uzywajac filtra"

MFindFileFind
"&Искать"
"&Find"
"&Hledat"
"&Suchen"
"K&eres"
"Szuka&j"

MFindFileDrive
"Дис&к"
"Dri&ve"
"D&isk"
"Lauf&werk"
"Meghajt&o"
"&Dysk"

MFindFileSetFilter
"&Фильтр"
"Filt&er"
"&Filtr"
"Filt&er"
"Szu&ro"
"&Filtr"

MFindFileAdvanced
"До&полнительно"
"Advance&d"
"Pokr&ocile"
"Er&weitert"
"Ha&lado"
"&Zaawansowane"

MFindSearchingIn
"Поиск%s в:"
"Searching%s in:"
"Hledam%s v:"
"Suche%s in:"
"%s keresese:"
"Szukam w:"

MFindNewSearch
"&Новый поиск"
"&New search"
"&Nove hledani"
"&Neue Suche"
"&Uj kereses"
"&Od nowa..."

MFindGoTo
"Пе&рейти"
"&Go to"
"&Jdi na"
"&Gehe zu"
"U&gras"
"&Idz do"

MFindView
"&Смотреть"
"&View"
"Zo&braz"
"&Betrachten"
"Meg&nez"
"&Podglad"

MFindPanel
"Пане&ль"
"&Panel"
"&Panel"
"&Panel"
"&Panel"
"&Do panelu"

MFindStop
"С&топ"
"&Stop"
"&Stop"
"&Stoppen"
"&Allj"
"&Stop"

MFindDone
l:
"Поиск закончен. Найдено %d файл(ов) и %d папка(ок)"
"Search done. Found %d file(s) and %d folder(s)"
"Hledani ukonceno. Nalezeno %d soubor(u) a %d adresar(u)"
"Suche beendet. %d Datei(en) und %d Ordner gefunden."
"A kereses kesz. %d fajlt es %d mappat talaltam."
"Wyszukiwanie zakonczone (znalazlem %d plikow i %d folderow)"

MFindCancel
"Отм&ена"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Megsem"
"&Anuluj"

MFindFound
l:
" Файлов: %d, папок: %d "
" Files: %d, folders: %d "
" Souboru: %d, adresaru: %d "
" Dateien: %d, Ordner: %d "
" Fajlt: %d, mappat: %d "
" Plikow: %d, folderow: %d "

MFindFileFolder
l:
"Папка"
"Folder"
"Adresar"
"Ordner"
"Mappa"
"Katalog"

MFindFileAdvancedTitle
l:
"Дополнительные параметры поиска"
"Find file advanced options"
"Pokrocile nastaveni vyhledavani souboru"
"Erweiterte Optionen"
"Fajlkereses halado beallitasai"
"Zaawansowane opcje wyszukiwania"

MFindFileSearchFirst
"Проводить поиск в &первых:"
"Search only in the &first:"
"Hledat po&uze v prvnich:"
"Nur &in den ersten x Bytes:"
"Kereses csak az elso &x bajtban:"
"Szukaj wylacznie w &pierwszych:"

MFindAlternateStreams
"Обрабатывать &альтернативные потоки данных"
"Process &alternate data streams"
upd:"Process &alternate data streams"
upd:"Process &alternate data streams"
"&Alternativ adatsavok (stream) feldolgozasa"
upd:"Process &alternate data streams"

MFoldTreeSearch
l:
"Поиск:"
"Search:"
"Hledat:"
"Suchen:"
"Kereses:"
"Wyszukiwanie:"

MGetCodePageTitle
l:
"Кодовые страницы"
"Code pages"
upd:"Znakove sady:"
upd:"Tabellen"
"Kodlapok"
"Strony kodowe"

MGetCodePageSystem
"Системные"
"System"
upd:"System"
upd:"System"
"Rendszer"
upd:"System"

MGetCodePageUnicode
"Юникод"
"Unicode"
upd:"Unicode"
upd:"Unicode"
"Unicode"
upd:"Unicode"

MGetCodePageFavorites
"Избранные"
"Favorites"
upd:"Favorites"
upd:"Favorites"
"Kedvencek"
upd:"Favorites"

MGetCodePageOther
"Прочие"
"Other"
upd:"Other"
upd:"Other"
"Egyeb"
upd:"Other"

MGetCodePageBottomTitle
"Ctrl-H, Del, Ins"
"Ctrl-H, Del, Ins"
"Ctrl-H, Del, Ins"
"Strg-H, Entf, Einf"
"Ctrl-H, Del, Ins"
"Ctrl-H, Del, Ins"

MGetCodePageBottomShortTitle
"Ctrl-H, Del"
"Ctrl-H, Del"
"Ctrl-H, Del"
"Strg-H, Entf"
"Ctrl-H, Del"
"Ctrl-H, Del"

MHighlightTitle
l:
"Раскраска файлов"
"Files highlighting"
"Zvyraznovani souboru"
"Farbmarkierungen"
"Fajlkiemelesek, rendezesi csoportok"
"Wyroznianie plikow"

MHighlightBottom
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Nahoru,Ctrl-Dolu"
"Einf,Entf,F4,F5,StrgUp,StrgDown"
"Ins,Del,F4,F5,Ctrl-Fel,Ctrl-Le"
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"

MHighlightUpperSortGroup
"Верхняя группа сортировки"
"Upper sort group"
"Vzesupne razeni"
"Obere Sortiergruppen"
"Felsobbrendu csoport"
"Gorna grupa sortowania"

MHighlightLowerSortGroup
"Нижняя группа сортировки"
"Lower sort group"
"Sestupne razeni"
"Untere Sortiergruppen"
"Alsobbrendu csoport"
"Dolna grupa sortowania"

MHighlightLastGroup
"Наименее приоритетная группа раскраски"
"Lowest priority highlighting group"
"Zvyrazneni nejnizsi prority"
"Farbmarkierungen mit niedrigster Prioritat"
"Legalacsonyabb rendu csoport"
"Grupa wyrozniania o najnizszym priorytecie"

MHighlightAskDel
"Вы хотите удалить раскраску для"
"Do you wish to delete highlighting for"
"Prejete si smazat zvyrazneni pro"
"Wollen Sie Farbmarkierungen loschen fur"
"Biztosan torli a kiemelest?"
"Czy chcesz usunac wyroznianie dla"

MHighlightWarning
"Будут потеряны все Ваши настройки!"
"You will lose all changes!"
"Vsechny zmeny budou ztraceny!"
"Sie verlieren jegliche Anderungen!"
"Minden valtoztatas elvesz!"
"Wszystkie zmiany zostana utracone!"

MHighlightAskRestore
"Вы хотите восстановить раскраску файлов по умолчанию?"
"Do you wish to restore default highlighting?"
"Prejete si obnovit vychozi nastaveni?"
"Wollen Sie Standard-Farbmarkierungen wiederherstellen?"
"Visszaallitja az alapertelmezett kiemeleseket?"
"Czy przywrocic wyroznianie domyslne?"

MHighlightEditTitle
l:
"Редактирование раскраски файлов"
"Edit files highlighting"
"Upravit zvyraznovani souboru"
"Farbmarkierungen bearbeiten"
"Fajlkiemeles szerkesztese"
"Edytuj wyroznianie plikow"

MHighlightMarkChar
"Оп&циональный символ пометки,"
"Optional markin&g character,"
"Volitelny &znak pro oznaceni urcenych souboru,"
"Optionale Markierun&g mit Zeichen,"
"Megadhato &jelolo karakter"
"Opcjonalny znak &wyrozniajacy zaznaczone pliki,"

MHighlightTransparentMarkChar
"прозра&чный"
"tra&nsparent"
"pruh&ledny"
"tra&nsparent"
"at&latszo"
"prze&zroczyste"

MHighlightColors
" Цвета файлов (\"черный на черном\" - цвет по умолчанию) "
" File name colors (\"black on black\" - default color) "
" Barva nazvu souboru (\"cerna na cerne\" - vychozi barva) "
" Dateinamenfarben (\"Schwarz auf Schwarz\"=Standard) "
" Fajlnev szinek (feketen fekete = alapertelmezett szin) "
" Kolory nazw plikow (domyslny - \"czarny na czarnym\") "

MHighlightFileName1
"&1. Обычное имя файла                "
"&1. Normal file name               "
"&1. Normalni soubor            "
"&1. Normaler Dateiname             "
"&1. Normal fajlnev                  "
"&1. Nazwa pliku bez zaznaczenia    "

MHighlightFileName2
"&3. Помеченное имя файла             "
"&3. Selected file name             "
"&3. Vybrany soubor             "
"&3. Markierter Dateiame            "
"&3. Kijelolt fajlnev                "
"&3. Zaznaczenie                    "

MHighlightFileName3
"&5. Имя файла под курсором           "
"&5. File name under cursor         "
"&5. Soubor pod kurzorem        "
"&5. Dateiname unter Cursor         "
"&5. Kurzor alatti fajlnev           "
"&5. Nazwa pliku pod kursorem       "

MHighlightFileName4
"&7. Помеченное под курсором имя файла"
"&7. File name selected under cursor"
"&7. Vybrany soubor pod kurzorem"
"&7. Dateiname markiert unter Cursor"
"&7. Kurzor alatti kijelolt fajlnev  "
"&7. Zaznaczony plik pod kursorem   "

MHighlightMarking1
"&2. Пометка"
"&2. Marking"
"&2. Oznaceni"
"&2. Markierung"
"&2. Jelolo kar.:"
"&2. Zaznaczenie"

MHighlightMarking2
"&4. Пометка"
"&4. Marking"
"&4. Oznaceni"
"&4. Markierung"
"&4. Jelolo kar.:"
"&4. Zaznaczenie"

MHighlightMarking3
"&6. Пометка"
"&6. Marking"
"&6. Oznaceni"
"&6. Markierung"
"&6. Jelolo kar.:"
"&6. Zaznaczenie"

MHighlightMarking4
"&8. Пометка"
"&8. Marking"
"&8. Oznaceni"
"&8. Markierung"
"&8. Jelolo kar.:"
"&8. Zaznaczenie"

MHighlightExample1
"║filename.ext │"
"║filename.ext │"
"║filename.ext │"
"║dateinam.erw │"
"║fajlneve.kit │"
"║nazwa.roz │"

MHighlightExample2
"║ filename.ext│"
"║ filename.ext│"
"║ filename.ext│"
"║ dateinam.erw│"
"║ fajlneve.kit│"
"║ nazwa.roz│"

MHighlightContinueProcessing
"Продолжать &обработку"
"C&ontinue processing"
"Pokracovat ve zpracova&ni"
"Verarbeitung f&ortsetzen"
"Folyamatos f&eldolgozas"
"K&ontynuuj przetwarzanie"

MInfoTitle
l:
"Информация"
"Information"
"Informace"
"Informationen"
"Informaciok"
"Informacja"

MInfoCompName
"Имя компьютера"
"Computer name"
"Nazev pocitace"
"Computername"
"Szamitogep neve"
"Nazwa komputera"

MInfoUserName
"Имя пользователя"
"User name"
"Jmeno uzivatele"
"Benutzername"
"Felhasznaloi nev"
"Nazwa uzytkownika"

MInfoRemovable
"Сменный"
"Removable"
"Vymenitelny"
"Austauschbares"
"Kiveheto"
"Wyjmowalny"

MInfoFixed
"Жесткий"
"Fixed"
"Pevny"
"Lokales"
"Fix"
"Staly"

MInfoNetwork
"Сетевой"
"Network"
"Sitovy"
"Netzwerk"
"Halozati"
"Sieciowy"

MInfoCDROM
"CD-ROM"
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
"CD-RW"

MInfoCD_RWDVD
"CD-RW/DVD"
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
"DVD-ROM"

MInfoDVD_RW
"DVD-RW"
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
"DVD-RAM"

MInfoRAM
"RAM"
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
"Virtualis"
"Subst"

MInfoDisk
"диск"
"disk"
"disk"
"Laufwerk"
"lemez"
"dysk"

MInfoDiskTotal
"Всего байтов"
"Total bytes"
"Celkem bytu"
"Bytes gesamt"
"Osszes bajt"
"Razem bajtow"

MInfoDiskFree
"Свободных байтов"
"Free bytes"
"Volnych bytu"
"Bytes frei"
"Szabad bajt"
"Wolnych bajtow"

MInfoDiskLabel
"Метка тома"
"Volume label"
"Popisek disku"
"Laufwerksbezeichnung"
"Kotet cimke"
"Etykieta woluminu"

MInfoDiskNumber
"Серийный номер"
"Serial number"
"Seriove cislo"
"Seriennummer"
"Sorozatszam"
"Numer seryjny"

MInfoMemory
" Память "
" Memory "
" Pamet "
" Speicher "
" Memoria "
" Pamiec "

MInfoMemoryLoad
"Загрузка памяти"
"Memory load"
"Zatizeni pameti"
"Speicherverbrauch"
"Hasznalt memoria"
"Uzycie pamieci"

MInfoMemoryTotal
"Всего памяти"
"Total memory"
"Celkova pamet"
"Speicher gesamt"
"Osszes memoria"
"Calkowita pamiec"

MInfoMemoryFree
"Свободно памяти"
"Free memory"
"Volna pamet"
"Speicher frei"
"Szabad memoria"
"Wolna pamiec"

MInfoVirtualTotal
"Всего вирт. памяти"
"Total virtual"
"Celkem virtualni"
"Virtueller Speicher gesamt"
"Osszes virtualis"
"Calkowita wirtualna"

MInfoVirtualFree
"Свободно вирт. памяти"
"Free virtual"
"Volna virtualni"
"Virtueller Speicher frei"
"Szabad virtualis"
"Wolna wirtualna"

MInfoDizAbsent
"Файл описания папки отсутствует"
"Folder description file is absent"
"Soubor s popisem adresare chybi"
"Keine Datei mit Ordnerbeschreibungen vorhanden."
"Mappa megjegyzesfajl nincs"
"Plik opisu katalogu nie istnieje"

MErrorInvalidFunction
l:
"Некорректная функция"
"Incorrect function"
"Nespravna funkce"
"Ungultige Funktion"
"Helytelen funkcio"
"Niewlasciwa funkcja"

MErrorBadCommand
"Команда не распознана"
"Command not recognized"
"Prikaz nebyl rozpoznan"
"Unbekannter Befehl"
"Ismeretlen parancs"
"Nieznane polecenie"

MErrorFileNotFound
"Файл не найден"
"File not found"
"Soubor nenalezen"
"Datei nicht gefunden"
"A fajl vagy mappa nem talalhato"
"Nie odnaleziono pliku"

MErrorPathNotFound
"Путь не найден"
"Path not found"
"Cesta nenalezena"
"Pfad nicht gefunden"
"Az eleresi ut nem talalhato"
"Nie odnaleziono sciezki"

MErrorTooManyOpenFiles
"Слишком много открытых файлов"
"Too many open files"
"Prilis mnoho otevrenych souboru"
"Zu viele geoffnete Dateien"
"Tul sok nyitott fajl"
"Zbyt wiele otwartych plikow"

MErrorAccessDenied
"Доступ запрещен"
"Access denied"
"Pristup odepren"
"Zugriff verweigert"
"Hozzaferes megtagadva"
"Dostep zabroniony"

MErrorNotEnoughMemory
"Недостаточно памяти"
"Not enough memory"
"Nedostatek pameti"
"Nicht genugend Speicher"
"Nincs eleg memoria"
"Za malo pamieci"

MErrorDiskRO
"Попытка записи на защищенный от записи диск"
"Cannot write to write protected disk"
"Nelze zapisovat na disk chraneny proti zapisu"
"Der Datentrager ist schreibgeschutzt"
"Irasvedett lemezre nem lehet irni"
"Nie moge zapisac na zabezpieczony dysk"

MErrorDeviceNotReady
"Устройство не готово"
"The device is not ready"
"Zarizeni neni pripraveno"
"Das Gerat ist nicht bereit"
"Az eszkoz nem kesz"
"Urzadzenie nie jest gotowe"

MErrorCannotAccessDisk
"Доступ к диску невозможен"
"Disk cannot be accessed"
"Na disk nelze pristoupit"
"Auf Datentrager kann nicht zugegriffen werden"
"A lemez nem erheto el"
"Brak dostepu do dysku"

MErrorSectorNotFound
"Сектор не найден"
"Sector not found"
"Sektor nenalezen"
"Sektor nicht gefunden"
"Szektor nem talalhato"
"Nie odnaleziono sektora"

MErrorOutOfPaper
"В принтере нет бумаги"
"The printer is out of paper"
"V tiskarne dosel papir"
"Der Drucker hat kein Papier mehr"
"A nyomtatoban nincs papir"
"Brak papieru w drukarce"

MErrorWrite
"Ошибка записи"
"Write fault error"
"Chyba zapisu"
"Fehler beim Schreibzugriff"
"Irasi hiba"
"Blad zapisu"

MErrorRead
"Ошибка чтения"
"Read fault error"
"Chyba cteni"
"Fehler beim Lesezugriff"
"Olvasasi hiba"
"Blad odczytu"

MErrorDeviceGeneral
"Общая ошибка устройства"
"Device general failure"
"Obecna chyba zarizeni"
"Ein Geratefehler ist aufgetreten"
"Eszkoz altalanos hiba"
"Ogolny blad urzadzenia"

MErrorFileSharing
"Нарушение совместного доступа к файлу"
"File sharing violation"
"Naruseno sdileni souboru"
"Zugriffsverletzung"
"Fajlmegosztasi hiba"
"Naruszenie zasad wspoluzytkowania pliku"

MErrorNetworkPathNotFound
"Сетевой путь не найден"
"The network path was not found"
"Sitova cesta nebyla nalezena"
"Der Netzwerkpfad wurde nicht gefunden"
"Halozati utvonal nem talalhato"
"Nie odnaleziono sciezki sieciowej"

MErrorNetworkBusy
"Сеть занята"
"The network is busy"
"Sit je zaneprazdnena"
"Das Netzwerk ist beschaftigt"
"A halozat zsufolt"
"Siec jest zajeta"

MErrorNetworkAccessDenied
"Сетевой доступ запрещен"
"Network access is denied"
"Pristup na sit zakazan"
"Netzwerkzugriff wurde verweigert"
"Halozati hozzaferes megtagadva"
"Dostep do sieci zabroniony"

MErrorNetworkWrite
"Ошибка записи в сети"
"A write fault occurred on the network"
"Na siti doslo k chybe v zapisu"
"Fehler beim Schreibzugriff auf das Netzwerk"
"Irasi hiba a halozaton"
"Wystapil blad zapisu w sieci"

MErrorDiskLocked
"Диск используется или заблокирован другим процессом"
"The disk is in use or locked by another process"
"Disk je pouzivan nebo uzamcen jinym procesem"
"Datentrager wird verwendet oder ist durch einen anderen Prozess gesperrt"
"A lemezt hasznalja vagy zarolja egy folyamat"
"Dysk jest w uzyciu lub zablokowany przez inny proces"

MErrorFileExists
"Файл или папка уже существует"
"File or folder already exists"
"Soubor nebo adresar jiz existuje"
"Die Datei oder der Ordner existiert bereits."
"A fajl vagy mappa mar letezik"
"Plik lub katalog juz istnieje"

MErrorInvalidName
"Указанное имя неверно"
"The specified name is invalid"
"Zadany nazev je neplatny"
"Der angegebene Name ist ungultig"
"A megadott nev ervenytelen"
"Podana nazwa jest niewlasciwa"

MErrorInsufficientDiskSpace
"Нет места на диске"
"Insufficient disk space"
"Nedostatek mista na disku"
"Unzureichend Speicherplatz am Datentrager"
"Nincs eleg hely a lemezen"
"Za malo miejsca na dysku"

MErrorFolderNotEmpty
"Папка не пустая"
"The folder is not empty"
"Adresar neni prazdny"
"Der Ordner ist nicht leer"
"A mappa nem ures"
"Katalog nie jest pusty"

MErrorIncorrectUserName
"Неверное имя пользователя"
"Incorrect user name"
"Neplatne jmeno uzivatele"
"Ungultiger Benutzername"
"Ervenytelen felhasznaloi nev"
"Niewlasciwa nazwa uzytkownika"

MErrorIncorrectPassword
"Неверный пароль"
"Incorrect password"
"Neplatne heslo"
"Ungultiges Passwort"
"Ervenytelen jelszo"
"Niewlasciwe haslo"

MErrorLoginFailure
"Ошибка регистрации"
"Login failure"
"Prihlaseni selhalo"
"Login fehlgeschlagen"
"Sikertelen bejelentkezes"
"Logowanie nie powiodlo sie"

MErrorConnectionAborted
"Соединение разорвано"
"Connection aborted"
"Spojeni preruseno"
"Verbindung abgebrochen"
"Kapcsolat bontva"
"Polaczenie zerwane"

MErrorCancelled
"Операция отменена"
"Operation cancelled"
"Operace stornovana"
"Vorgang abgebrochen"
"A muvelet megszakitva"
"Operacja przerwana"

MErrorNetAbsent
"Сеть отсутствует"
"No network present"
"Sit neni k dispozici"
"Kein Netzwerk verfugbar"
"Nincs halozat"
"Brak sieci"

MErrorNetDeviceInUse
"Устройство используется и не может быть отсоединено"
"Device is in use and cannot be disconnected"
"Zarizeni se pouziva a nemuze byt odpojeno"
"Gerat wird gerade verwendet oder kann nicht getrennt werden"
"Az eszkoz hasznalatban van, nem valaszthato le"
"Urzadzenie jest w uzyciu i nie mozna go odlaczyc"

MErrorNetOpenFiles
"На сетевом диске есть открытые файлы"
"This network connection has open files"
"Pres toto sitove spojeni jsou otevreny soubory"
"Diese Netzwerkverbindung hat geoffnete Dateien"
"A halozaton nyitott fajlok vannak"
"To polaczenie sieciowe posiada otwarte pliki"

MErrorAlreadyAssigned
"Имя локального устройства уже использовано"
"The local device name is already in use"
"Nazev lokalniho zarizeni je jiz pouzivan"
"Der lokale Geratename wird bereits verwendet"
"A helyi eszkoznev mar foglalt"
"Nazwa urzadzenia lokalnego jest juz uzywana"

MErrorAlreadyRemebered
"Имя локального устройства уже находится в профиле пользователя"
"The local device is already in the user profile"
"Lokalni zarizeni je jiz v uzivatelove profilu"
"Der lokale Datentrager ist bereits Teil des Benutzerprofils"
"A helyi eszkoz mar a felhasznaloi profilban van"
"Lokalne urzadzenie znajduje sie juz w profilu uzytkownika"

MErrorNotLoggedOn
"Пользователь не зарегистрирован в сети"
"User has not logged on to the network"
"Uzivatel nebyl do site prihlasen"
"Benutzer hat sich nicht am Netzwerk angemeldet"
"A felhasznalo nincs a halozaton"
"Uzytkownik nie jest zalogowany do sieci"

MErrorInvalidPassword
"Неверный пароль пользователя"
"The user password is invalid"
"Uzivatelovo heslo neni spravne"
"Das Benutzerpasswort ist ungultig"
"Ervenytelen felhasznaloi jelszo"
"Haslo uzytkownika jest niewlasciwe"

MErrorNoRecoveryPolicy
"Для этой системы отсутствует политика надежного восстановления шифрования"
"There is no valid encryption recovery policy configured for this system"
"V tomto systemu neni nastaveno zadne platne pravidlo pro desifrovani"
"Auf diesem System ist keine gultige Richtlinie zum Wiederherstellen der Verschlusselung konfiguriert."
"Nincs ervenyes titkositast feloldo szabaly a hazirendben"
"Polityka odzyskiwania szyfrowania nie jest skonfigurowana"

MErrorEncryptionFailed
"Ошибка при попытке шифрования файла"
"The specified file could not be encrypted"
"Zadany soubor nemohl byt zasifrovan"
"Die angegebene Datei konnte nicht verschlusselt werden"
"A megadott fajl nem titkosithato"
"Nie udalo sie zaszyfrowac pliku"

MErrorDecryptionFailed
"Ошибка при попытке расшифровки файла"
"The specified file could not be decrypted"
"Zadany soubor nemohl byt desifrovan"
"Die angegebene Datei konnte nicht entschlusselt werden"
"A megadott fajl titkositasa nem oldhato fel"
"Nie udalo sie odszyfrowac pliku"

MErrorFileNotEncrypted
"Указанный файл не зашифрован"
"The specified file is not encrypted"
"Zadany soubor neni zasifrovan"
"Die angegebene Datei ist nicht verschlusselt"
"A megadott fajl nem titkositott"
"Plik nie jest zaszyfrowany"

MErrorNoAssociation
"Указанному файлу не сопоставлено ни одно приложение для выполнения данной операции"
"No application is associated with the specified file for this operation"
"K zadanemu souboru neni asociovana zadna aplikace pro tuto operaci"
"Diesem Dateityp und dieser Aktion ist kein Programm zugewiesen."
"A fajlhoz nincs tarsitva program"
"Z ta operacja dla pliku nie jest skojarzona zadna aplikacja"

MErrorFullPathNameLong
l:
"Полный путь к файлу имеет слишком большую длину"
"The full pathname is too long"
"Plna cesta k souboru je prilis dlouha"
"Der volle Name des Pfades ist zu lang"
"A teljes eleresi ut tul hosszu"
"Pelna sciezka jest zbyt dluga"

MCannotExecute
l:
"Ошибка выполнения"
"Cannot execute"
"Nelze provest"
"Fehler beim Ausfuhren von"
"Nem vegrehajthato:"
"Nie moge wykonac"

MScanningFolder
"Просмотр папки"
"Scanning the folder"
"Prohledavam adresar"
"Scanne den Ordner"
"Mappak olvasasa..."
"Przeszukuje katalog"

MMakeFolderTitle
l:
"Создание папки"
"Make folder"
"Vytvoreni adresare"
"Ordner erstellen"
"Uj mappa letrehozasa"
"Utworz katalog"

MCreateFolder
"Создать п&апку"
"Create the &folder"
"Vytvorit &adresar"
"Diesen &Ordner erstellen:"
"Mappa &neve:"
"Nazwa katalogu"

MMultiMakeDir
"Обрабатыват&ь несколько имен папок"
"Process &multiple names"
"Zpracovat &vice nazvu"
"&Mehrere Namen verarbeiten (getrennt durch Semikolon)"
"Tob&b nev feldolgozasa"
"Przetwarzaj &wiele nazw"

MIncorrectDirList
"Неправильный список папок"
"Incorrect folders list"
"Neplatny seznam adresaru"
"Fehlerhafte Ordnerliste"
"Hibas mappalista"
"Bledna lista folderow"

MCannotCreateFolder
"Ошибка создания папки"
"Cannot create the folder"
"Adresar nelze vytvorit"
"Konnte den Ordner nicht erstellen"
"A mappa nem hozhato letre"
"Nie moge utworzyc katalogu"

MMenuBriefView
l:
"&Краткий                  LCtrl-1"
"&Brief              LCtrl-1"
"&Strucny                  LCtrl-1"
"&Kurz                 LStrg-1"
"&Rovid              BalCtrl-1"
"&Skrotowy             LCtrl-1"

MMenuMediumView
"&Средний                  LCtrl-2"
"&Medium             LCtrl-2"
"S&tredni                  LCtrl-2"
"&Mittel               LStrg-2"
"&Kozepes            BalCtrl-2"
"S&redni               LCtrl-2"

MMenuFullView
"&Полный                   LCtrl-3"
"&Full               LCtrl-3"
"&Plny                     LCtrl-3"
"&Voll                 LStrg-3"
"&Teljes             BalCtrl-3"
"&Pelny                LCtrl-3"

MMenuWideView
"&Широкий                  LCtrl-4"
"&Wide               LCtrl-4"
"S&iroky                   LCtrl-4"
"B&reitformat          LStrg-4"
"&Szeles             BalCtrl-4"
"S&zeroki              LCtrl-4"

MMenuDetailedView
"&Детальный                LCtrl-5"
"Detai&led           LCtrl-5"
"Detai&lni                 LCtrl-5"
"Detai&lliert          LStrg-5"
"Resz&letes          BalCtrl-5"
"Ze sz&czegolami       LCtrl-5"

MMenuDizView
"&Описания                 LCtrl-6"
"&Descriptions       LCtrl-6"
"P&opisky                  LCtrl-6"
"&Beschreibungen       LStrg-6"
"Fajl&megjegyzesek   BalCtrl-6"
"&Opisy                LCtrl-6"

MMenuLongDizView
"Д&линные описания         LCtrl-7"
"Lon&g descriptions  LCtrl-7"
"&Dlouhe popisky           LCtrl-7"
"Lan&ge Beschreibungen LStrg-7"
"&Hosszu megjegyzes  BalCtrl-7"
"&Dlugie opisy         LCtrl-7"

MMenuOwnersView
"Вл&адельцы файлов         LCtrl-8"
"File own&ers        LCtrl-8"
"Vlastnik so&uboru         LCtrl-8"
"B&esitzer             LStrg-8"
"Fajl tula&jdonos    BalCtrl-8"
"&Wlasciciele          LCtrl-8"

MMenuLinksView
"Свя&зи файлов             LCtrl-9"
"File lin&ks         LCtrl-9"
"Souborove lin&ky          LCtrl-9"
"Dateilin&ks           LStrg-9"
"Fajl li&nkek        BalCtrl-9"
"Dowiaza&nia           LCtrl-9"

MMenuAlternativeView
"Аль&тернативный полный    LCtrl-0"
"&Alternative full   LCtrl-0"
"&Alternativni plny        LCtrl-0"
"&Alternativ voll      LStrg-0"
"&Alternativ teljes  BalCtrl-0"
"&Alternatywny         LCtrl-0"

MMenuInfoPanel
l:
"Панель ин&формации        Ctrl-L"
"&Info panel         Ctrl-L"
"Panel In&fo               Ctrl-L"
"&Infopanel            Strg-L"
"&Info panel         Ctrl-L"
"Panel informacy&jny   Ctrl-L"

MMenuTreePanel
"Де&рево папок             Ctrl-T"
"&Tree panel         Ctrl-T"
"Panel St&rom              Ctrl-T"
"Baumansich&t          Strg-T"
"&Fastruktura        Ctrl-T"
"Drz&ewo               Ctrl-T"

MMenuQuickView
"Быстры&й просмотр         Ctrl-Q"
"Quick &view         Ctrl-Q"
"Z&bezne zobrazeni         Ctrl-Q"
"Sc&hnellansicht       Strg-Q"
"&Gyorsnezet         Ctrl-Q"
"Sz&ybki podglad       Ctrl-Q"

MMenuSortModes
"Режим&ы сортировки        Ctrl-F12"
"&Sort modes         Ctrl-F12"
"Mody raze&ni              Ctrl-F12"
"&Sortiermodi          Strg-F12"
"R&endezesi elv      Ctrl-F12"
"Try&by sortowania     Ctrl-F12"

MMenuLongNames
"Показывать длинные &имена Ctrl-N"
"Show long &names    Ctrl-N"
"Zobrazit dlouhe nazv&y    Ctrl-N"
"Lange Datei&namen     Strg-N"
"H&osszu fajlnevek   Ctrl-N"
"Po&kaz dlugie nazwy   Ctrl-N"

MMenuTogglePanel
"Панель &Вкл/Выкл          Ctrl-F1"
"Panel &On/Off       Ctrl-F1"
"Panel &Zap/Vyp            Ctrl-F1"
"&Panel ein/aus        Strg-F1"
"&Panel be/ki        Ctrl-F1"
"Wlacz/Wylacz pane&l   Ctrl-F1"

MMenuReread
"П&еречитать               Ctrl-R"
"&Re-read            Ctrl-R"
"Obno&vit                  Ctrl-R"
"Aktualisie&ren        Strg-R"
"Friss&ites          Ctrl-R"
"Odsw&iez              Ctrl-R"

MMenuChangeDrive
"С&менить диск             Alt-F1"
"&Change drive       Alt-F1"
"Z&menit jednotku          Alt-F1"
"Laufwerk we&chseln    Alt-F1"
"Meghajto&valtas     Alt-F1"
"Z&mien naped          Alt-F1"

MMenuView
l:
"&Просмотр              F3"
"&View               F3"
"&Zobrazit                   F3"
"&Betrachten           F3"
"&Megnez               F3"
"&Podglad                   F3"

MMenuEdit
"&Редактирование        F4"
"&Edit               F4"
"&Editovat                   F4"
"B&earbeiten           F4"
"&Szerkeszt            F4"
"&Edytuj                    F4"

MMenuCopy
"&Копирование           F5"
"&Copy               F5"
"&Kopirovat                  F5"
"&Kopieren             F5"
"Mas&ol                F5"
"&Kopiuj                    F5"

MMenuMove
"П&еренос               F6"
"&Rename or move     F6"
"&Prejmenovat/Presunout      F6"
"Ve&rschieben/Umben.   F6"
"At&nevez-Mozgat       F6"
"&Zmien nazwe lub przenies  F6"

MMenuCreateFolder
"&Создание папки        F7"
"&Make folder        F7"
"&Vytvorit adresar           F7"
"&Ordner erstellen     F7"
"U&j mappa             F7"
"U&tworz katalog            F7"

MMenuDelete
"&Удаление              F8"
"&Delete             F8"
"&Smazat                     F8"
"&Loschen              F8"
"&Torol                F8"
"&Usun                      F8"

MMenuWipe
"Уни&чтожение           Alt-Del"
"&Wipe               Alt-Del"
"&Vymazat                    Alt-Del"
"&Sicher loschen       Alt-Entf"
"&Kisopor              Alt-Del"
"&Wymaz                     Alt-Del"

MMenuAdd
"&Архивировать          Shift-F1"
"Add &to archive     Shift-F1"
"Pridat do &archivu          Shift-F1"
"Zu Archiv &hinzuf.    Umsch-F1"
"Tomorhoz ho&zzaad     Shift-F1"
"&Dodaj do archiwum         Shift-F1"

MMenuExtract
"Распако&вать           Shift-F2"
"E&xtract files      Shift-F2"
"&Rozbalit soubory           Shift-F2"
"Archiv e&xtrahieren   Umsch-F2"
"Tomorbol ki&bont      Shift-F2"
"&Rozpakuj archiwum         Shift-F2"

MMenuArchiveCommands
"Архивн&ые команды      Shift-F3"
"Arc&hive commands   Shift-F3"
"Prikazy arc&hivu            Shift-F3"
"Arc&hivbefehle        Umsch-F3"
"Tomorito &parancsok   Shift-F3"
"Po&lecenie archiwizera     Shift-F3"

MMenuAttributes
"А&трибуты файлов       Ctrl-A"
"File &attributes    Ctrl-A"
"A&tributy souboru           Ctrl-A"
"Datei&attribute       Strg-A"
"Fajl &attributumok    Ctrl-A"
"&Atrybuty pliku            Ctrl-A"

MMenuApplyCommand
"Применить коман&ду     Ctrl-G"
"A&pply command      Ctrl-G"
"Ap&likovat prikaz           Ctrl-G"
"Befehl an&wenden      Strg-G"
"Parancs &vegrehajtasa Ctrl-G"
"Zastosuj pole&cenie        Ctrl-G"

MMenuDescribe
"&Описание файлов       Ctrl-Z"
"Descri&be files     Ctrl-Z"
"Pridat popisek sou&borum    Ctrl-Z"
"Beschrei&bung andern  Strg-Z"
"Fajlmegje&gyzes       Ctrl-Z"
"&Opisz pliki               Ctrl-Z"

MMenuSelectGroup
"Пометить &группу       Gray +"
"Select &group       Gray +"
"Oz&nacit skupinu            Num +"
"&Gruppe auswahlen     Num +"
"Csoport k&ijelolese   Szurke +"
"Zaznacz &grupe             Szary +"

MMenuUnselectGroup
"С&нять пометку         Gray -"
"U&nselect group     Gray -"
"O&dznacit skupinu           Num -"
"G&ruppe abwahlen      Num -"
"Jelolest l&evesz      Szurke -"
"Odz&nacz grupe             Szary -"

MMenuInvertSelection
"&Инверсия пометки      Gray *"
"&Invert selection   Gray *"
"&Invertovat vyber           Num *"
"Auswah&l umkehren     Num *"
"Jelolest meg&fordit   Szurke *"
"Od&wroc zaznaczenie        Szary *"

MMenuRestoreSelection
"Восстановить по&метку  Ctrl-M"
"Re&store selection  Ctrl-M"
"&Obnovit vyber              Ctrl-M"
"Auswahl wiederher&st. Strg-M"
"Jel&olest visszatesz  Ctrl-M"
"Odtworz zaznaczen&ie       Ctrl-M"

MMenuFindFile
l:
"&Поиск файла              Alt-F7"
"&Find file           Alt-F7"
"H&ledat soubor                  Alt-F7"
"Dateien &finden       Alt-F7"
"Fajl&kereses         Alt-F7"
"&Znajdz plik               Alt-F7"

MMenuHistory
"&История команд           Alt-F8"
"&History             Alt-F8"
"&Historie                       Alt-F8"
"&Historie             Alt-F8"
"Parancs &elozmenyek  Alt-F8"
"&Historia                  Alt-F8"

MMenuVideoMode
"Видео&режим               Alt-F9"
"&Video mode          Alt-F9"
"&Video mod                      Alt-F9"
"Ansicht<->&Vollbild   Alt-F9"
"&Video mod           Alt-F9"
"&Tryb wyswietlania         Alt-F9"

MMenuFindFolder
"Поис&к папки              Alt-F10"
"Fi&nd folder         Alt-F10"
"Hl&edat adresar                 Alt-F10"
"Ordner fi&nden        Alt-F10"
"&Mappakereses        Alt-F10"
"Znajdz kata&log            Alt-F10"

MMenuViewHistory
"Ис&тория просмотра        Alt-F11"
"File vie&w history   Alt-F11"
"Historie &zobrazeni souboru     Alt-F11"
"Be&trachterhistorie   Alt-F11"
"Faj&l elozmenyek     Alt-F11"
"Historia &podgladu plikow  Alt-F11"

MMenuFoldersHistory
"Ист&ория папок            Alt-F12"
"F&olders history     Alt-F12"
"Historie &adresaru              Alt-F12"
"&Ordnerhistorie       Alt-F12"
"Ma&ppa elozmenyek    Alt-F12"
"Historia &katalogow        Alt-F12"

MMenuSwapPanels
"По&менять панели          Ctrl-U"
"&Swap panels         Ctrl-U"
"Prohodit panel&y                Ctrl-U"
"Panels tau&schen      Strg-U"
"Panel&csere          Ctrl-U"
"Z&amien panele             Ctrl-U"

MMenuTogglePanels
"Панели &Вкл/Выкл          Ctrl-O"
"&Panels On/Off       Ctrl-O"
"&Panely Zap/Vyp                 Ctrl-O"
"&Panels ein/aus       Strg-O"
"Panelek &be/ki       Ctrl-O"
"&Wlacz/Wylacz panele       Ctrl-O"

MMenuCompareFolders
"&Сравнение папок"
"&Compare folders"
"Po&rovnat adresare"
"Ordner verglei&chen"
"Mappak ossze&hasonlitasa"
"Porowna&j katalogi"

MMenuUserMenu
"Меню пользовател&я"
"Edit user &menu"
"Upravit uzivatelske &menu"
"Benutzer&menu editieren"
"Felhasznaloi m&enu szerk."
"Edytuj &menu uzytkownika"

MMenuFileAssociations
"&Ассоциации файлов"
"File &associations"
"Asocia&ce souboru"
"Dat&eiverknupfungen"
"Fajl&tarsitasok"
"Prz&ypisania plikow"

MMenuFolderShortcuts
"Ссы&лки на папки"
"Fol&der shortcuts"
"A&dresarove zkratky"
"Or&dnerschnellzugriff"
"Mappa gyorsbillent&yuk"
"&Skroty katalogow"

MMenuFilter
"&Фильтр панели файлов     Ctrl-I"
"File panel f&ilter   Ctrl-I"
"F&iltr panelu souboru           Ctrl-I"
"Panelf&ilter          Strg-I"
"Fajlpanel &szurok    Ctrl-I"
"&Filtr panelu plikow       Ctrl-I"

MMenuPluginCommands
"Команды внешних мо&дулей  F11"
"Pl&ugin commands     F11"
"Prikazy plu&ginu                F11"
"Pl&uginbefehle        F11"
"Pl&ugin parancsok    F11"
"Pl&uginy                   F11"

MMenuWindowsList
"Список экра&нов           F12"
"Sc&reens list        F12"
"Seznam obrazove&k               F12"
"Seite&nliste          F12"
"Keper&nyok           F12"
"L&ista ekranow             F12"

MMenuProcessList
"Список &задач             Ctrl-W"
"Task &list           Ctrl-W"
"Seznam ul&oh                    Ctrl-W"
"Task&liste            Strg-W"
"Futo p&rogramok      Ctrl-W"
"Lista za&dan               Ctrl-W"

MMenuHotPlugList
"Список Hotplug-&устройств"
"Ho&tplug devices list"
"Seznam v&yjimatelnych zarizeni"
"Sicheres En&tfernen"
"H&otplug eszkozok"
"Lista urzadzen Ho&tplug"

MMenuSystemSettings
l:
"Систе&мные параметры"
"S&ystem settings"
"Nastaveni S&ystemu"
"&Grundeinstellungen"
"&Rendszer beallitasok"
"Ustawienia &systemowe"

MMenuPanelSettings
"Настройки па&нели"
"&Panel settings"
"Nastaveni &Panelu"
"&Panels einrichten"
"&Panel beallitasok"
"Ustawienia &panelu"

MMenuInterface
"Настройки &интерфейса"
"&Interface settings"
"Nastaveni Ro&zhrani"
"Oberflache einr&ichten"
"Kezelo&felulet beallitasok"
"Ustawienia &interfejsu"

MMenuDialogSettings
"Настройки &диалогов"
"Di&alog settings"
"Nastaveni Dialo&gu"
"Di&aloge einrichten"
"Par&beszedablak beallitasok"
"Ustawienia okna &dialogowego"

MMenuLanguages
"&Языки"
"Lan&guages"
"Nastaveni &Jazyka"
"Sprac&hen"
"N&yelvek (Languages)"
"&Jezyk"

MMenuPluginsConfig
"Параметры &внешних модулей"
"Pl&ugins configuration"
"Nastaveni Plu&ginu"
"Konfiguration von Pl&ugins"
"Pl&ugin beallitasok"
"Konfiguracja p&luginow"

MMenuConfirmation
"&Подтверждения"
"Co&nfirmations"
"P&otvrzeni"
"Bestatigu&ngen"
"Meg&erositesek"
"P&otwierdzenia"

MMenuPluginConfirmation
"Выбор плагина"
"Plugin selection"
upd:"Plugin selection"
upd:"Plugin selection"
"Plugin valasztas"
upd:"Plugin selection"

MMenuPluginStdAssociation
"Стандартная ассоциация"
"Standard association"
upd:"Standard association"
upd:"Standard association"
"Szabvany tarsitas"
upd:"Standard association"

MMenuFilePanelModes
"Режим&ы панели файлов"
"File panel &modes"
"&Mody souborovych panelu"
"Anzeige&modi von Dateipanels"
"Fajlpanel mod&ok"
"&Tryby wyswietlania panelu plikow"

MMenuFileDescriptions
"&Описания файлов"
"File &descriptions"
"Popi&sy souboru"
"&Dateibeschreibungen"
"Fajl &megjegyzesfajlok"
"Opis&y plikow"

MMenuFolderInfoFiles
"Файлы описания п&апок"
"&Folder description files"
"Soubory popisu &adresaru"
"O&rdnerbeschreibungen"
"M&appa megjegyzesfajlok"
"Pliki opisu &katalogu"

MMenuViewer
"Настройки про&граммы просмотра"
"&Viewer settings"
"Nastaveni P&rohlizece"
"Be&trachter einrichten"
"&Nezoke beallitasok"
"Ustawienia pod&gladu"

MMenuEditor
"Настройки &редактора"
"&Editor settings"
"Nastaveni &Editoru"
"&Editor einrichten"
"&Szerkeszto beallitasok"
"Ustawienia &edytora"

MMenuColors
"&Цвета"
"Co&lors"
"&Barvy"
"&Farben"
"S&zinek"
"Kolo&ry"

MMenuFilesHighlighting
"Раскраска &файлов и группы сортировки"
"Files &highlighting and sort groups"
"Z&vyraznovani souboru a skupiny razeni"
"Farbmar&kierungen und Sortiergruppen"
"Fajlkiemelesek, rendezesi &csoportok"
"&Wyroznianie plikow"

MMenuSaveSetup
"&Сохранить параметры                  Shift-F9"
"&Save setup                         Shift-F9"
"&Ulozit nastaveni                      Shift-F9"
"Setup &speichern                     Umsch-F9"
"Beallitasok men&tese                 Shift-F9"
"&Zapisz ustawienia       Shift-F9"

MMenuTogglePanelRight
"Панель &Вкл/Выкл          Ctrl-F2"
"Panel &On/Off       Ctrl-F2"
"Panel &Zap/Vyp            Ctrl-F2"
"Panel &ein/aus        Strg-F2"
"Panel be/&ki        Ctrl-F2"
"Wlacz/wylacz pane&l   Ctrl-F2"

MMenuChangeDriveRight
"С&менить диск             Alt-F2"
"&Change drive       Alt-F2"
"Z&menit jednotku          Alt-F2"
"Laufwerk &wechseln    Alt-F2"
"Meghajto&valtas     Alt-F2"
"Z&mien naped          Alt-F2"

MMenuLeftTitle
l:
"&Левая"
"&Left"
"&Levy"
"&Links"
"&Bal"
"&Lewy"

MMenuFilesTitle
"&Файлы"
"&Files"
"&Soubory"
"&Dateien"
"&Fajlok"
"Pl&iki"

MMenuCommandsTitle
"&Команды"
"&Commands"
"Pri&kazy"
"&Befehle"
"&Parancsok"
"Pol&ecenia"

MMenuOptionsTitle
"Па&раметры"
"&Options"
"&Nastaveni"
"&Optionen"
"B&eallitasok"
"&Opcje"

MMenuRightTitle
"&Правая"
"&Right"
"&Pravy"
"&Rechts"
"&Jobb"
"&Prawy"

MMenuSortTitle
l:
"Критерий сортировки"
"Sort by"
"Seradit podle"
"Sortieren nach"
"Rendezesi elv"
"Sortuj wedlug..."

MMenuSortByName
"&Имя                              Ctrl-F3"
"&Name                 Ctrl-F3"
"&Nazvu                     Ctrl-F3"
"&Name                   Strg-F3"
"&Nev                  Ctrl-F3"
"&nazwy                       Ctrl-F3"

MMenuSortByExt
"&Расширение                       Ctrl-F4"
"E&xtension            Ctrl-F4"
"&Pripony                   Ctrl-F4"
"&Erweiterung            Strg-F4"
"Ki&terjesztes         Ctrl-F4"
"ro&zszerzenia                Ctrl-F4"

MMenuSortByModification
"Время &модификации                Ctrl-F5"
"&Modification time    Ctrl-F5"
"C&asu modifikace           Ctrl-F5"
"&Veranderungsdatum      Strg-F5"
"Modositas &ideje      Ctrl-F5"
"czasu &modyfikacji           Ctrl-F5"

MMenuSortBySize
"Р&азмер                           Ctrl-F6"
"&Size                 Ctrl-F6"
"&Velikosti                 Ctrl-F6"
"&Gro?e                  Strg-F6"
"&Meret                Ctrl-F6"
"&rozmiaru                    Ctrl-F6"

MMenuUnsorted
"&Не сортировать                   Ctrl-F7"
"&Unsorted             Ctrl-F7"
"N&eradit                   Ctrl-F7"
"&Unsortiert             Strg-F7"
"&Rendezetlen          Ctrl-F7"
"&bez sortowania              Ctrl-F7"

MMenuSortByCreation
"Время &создания                   Ctrl-F8"
"&Creation time        Ctrl-F8"
"&Data vytvoreni            Ctrl-F8"
"E&rstelldatum           Strg-F8"
"Ke&letkezes ideje     Ctrl-F8"
"czasu u&tworzenia            Ctrl-F8"

MMenuSortByAccess
"Время &доступа                    Ctrl-F9"
"&Access time          Ctrl-F9"
"Ca&su pristupu             Ctrl-F9"
"&Zugriffsdatum          Strg-F9"
"&Hozzaferes ideje     Ctrl-F9"
"czasu &uzycia                Ctrl-F9"

MMenuSortByDiz
"&Описания                         Ctrl-F10"
"&Descriptions         Ctrl-F10"
"P&opisku                   Ctrl-F10"
"&Beschreibungen         Strg-F10"
"Megjegyze&sek         Ctrl-F10"
"&opisu                       Ctrl-F10"

MMenuSortByOwner
"&Владельцы файлов                 Ctrl-F11"
"&Owner                Ctrl-F11"
"V&lastnika                 Ctrl-F11"
"Bes&itzer               Strg-F11"
"Tula&jdonos           Ctrl-F11"
"&wlasciciela                 Ctrl-F11"

MMenuSortByCompressedSize
"&Упакованный размер"
"Com&pressed size"
"&Komprimovane velikosti"
"Kom&primierte Gro?e"
"Tomoritett mer&et"
"rozmiaru po &kompresji"

MMenuSortByNumLinks
"Ко&личество ссылок"
"Number of &hard links"
"Poc&tu pevnych linku"
"Anzahl an &Links"
"Hardlinkek s&zama"
"&liczby dowiazan"

MMenuSortByNumStreams
"Количество &потоков"
"Number of s&treams"
upd:"Number of s&treams"
upd:"Number of s&treams"
"Stream-e&k szama"
upd:"Number of s&treams"

MMenuSortByStreamsSize
"Размер по&токов"
"Size of st&reams"
upd:"Size of st&reams"
upd:"Size of st&reams"
"Stream-ek m&erete"
upd:"Size of st&reams"

MMenuSortUseGroups
"Использовать &группы сортировки   Shift-F11"
"Use sort &groups      Shift-F11"
"Razeni podle skup&in       Shift-F11"
"Sortier&gruppen verw.   Umsch-F11"
"Rend. cs&oport haszn. Shift-F11"
"uzyj &grup sortowania        Shift-F11"

MMenuSortSelectedFirst
"&Помеченные файлы вперед          Shift-F12"
"Show selected &first  Shift-F12"
"Nejdriv zobrazit vy&brane  Shift-F12"
"&Ausgewahlte zuerst     Umsch-F12"
"Kijel&olteket elore   Shift-F12"
"zazna&czone najpierw         Shift-F12"

MMenuSortUseNumeric
"Использовать &числовую сортировку"
"Use num&eric sort"
"Pouzit ci&selne razeni"
"Nu&merische Sortierung"
"N&umerikus rendezes"
"Sortuj num&erycznie"

MChangeDriveTitle
l:
"Диск"
"Drive"
"Jednotka"
"Laufwerke"
"Meghajtok"
"Naped"

MChangeDriveRemovable
"сменный"
"removable"
"vymenitelna"
"wechsel."
"kiveheto"
"wyjmowalny"

MChangeDriveFixed
"жесткий"
"fixed"
"pevna"
"fest"
"fix"
"staly"

MChangeDriveNetwork
"сетевой"
"network"
"sitova"
"Netzwerk"
"halozati"
"sieciowy"

MChangeDriveDisconnectedNetwork
"не подключенный"
"disconnected"
upd:"disconnected"
upd:"disconnected"
"levalasztva"
upd:"disconnected"

MChangeDriveCDROM
"CD-ROM"
"CD-ROM"
"CD-ROM"
"CD-ROM"
"CD-ROM"
"CD-ROM"

MChangeDriveCD_RW
"CD-RW"
"CD-RW"
"CD-RW"
"CD-RW"
"CD-RW"
"CD-RW"

MChangeDriveCD_RWDVD
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD"
"CD-RW/DVD"

MChangeDriveDVD_ROM
"DVD-ROM"
"DVD-ROM"
"DVD-ROM"
"DVD-ROM"
"DVD-ROM"
"DVD-ROM"

MChangeDriveDVD_RW
"DVD-RW"
"DVD-RW"
"DWD-RW"
"DVD-RW"
"DVD-RW"
"DVD-RW"

MChangeDriveDVD_RAM
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"

MChangeDriveRAM
"RAM диск"
"RAM disk"
"RAM disk"
"RAM-DISK"
"RAM lemez"
"RAM-dysk"

MChangeDriveSUBST
"SUBST"
"subst"
"SUBST"
"Subst"
"virtualis"
"subst"

MChangeDriveLabelAbsent
"недоступен"
"not available"
"neni k dispozici"
"nicht vorh."
"nem elerheto"
"niedostepny"

MChangeDriveCannotReadDisk
"Ошибка чтения диска в дисководе %c:"
"Cannot read the disk in drive %c:"
"Nelze precist disk v jednotce %c:"
"Datentrage in Laufwerk %c: kann nicht gelesen werden."
"%c: meghajto lemeze nem olvashato"
"Nie moge odczytac dysku w napedzie %c:"

MChangeDriveCannotDisconnect
"Не удается отсоединиться от %s"
"Cannot disconnect from %s"
"Nelze se odpojit od %s"
"Verbindung zu %s konnte nicht getrennt werden."
"Nem lehet levalni innen: %s"
"Nie moge odlaczyc sie od %s"

MChangeDriveCannotDelSubst
"Не удается удалить виртуальный драйвер %s"
"Cannot delete a substituted drive %s"
"Nelze smazat substnuta jednotka %s"
"Substlaufwerk %s konnte nicht geloscht werden."
"%s virtualis meghajto nem torolheto"
"Nie mozna usunac dysku SUBST %s"

MChangeDriveOpenFiles
"Если вы не закроете открытые файлы, данные могут быть утеряны"
"If you do not close the open files, data may be lost."
"Pokud neuzavrete otevrene soubory, mohou byt tato data ztracena."
"Wenn Sie offene Dateien nicht schlie?en konnten Daten verloren gehen."
"Ha a nyitott fajlokat nem zarja be, az adatok elveszhetnek!"
"Jesli nie zamkniesz otwartych plikow, mozesz utracic dane."

MChangeSUBSTDisconnectDriveTitle
l:
"Отключение виртуального устройства"
"Virtual device disconnection"
"Odpojovani virtualniho zarizeni"
"Virtuelles Gerat trennen"
"Virtualis meghajto torlese"
"Odlaczanie napedu wirtualnego"

MChangeSUBSTDisconnectDriveQuestion
"Отключить SUBST-диск %c:?"
"Disconnect SUBST-disk %c:?"
"Odpojit SUBST-disk %c:?"
"Substlaufwerk %c: trennen?"
"Torli %c: virtualis meghajtot?"
"Odlaczyc dysk SUBST %c:?"

MChangeHotPlugDisconnectDriveTitle
l:
"Удаление устройства"
"Device Removal"
"Odpojovani zarizeni"
"Sicheres Entfernen"
"Eszkoz biztonsagos eltavolitasa"
"Odlaczanie urzadzenia"

MChangeHotPlugDisconnectDriveQuestion
"Вы хотите удалить устройство"
"Do you want to remove the device"
"Opravdu si prejete odpojit zarizeni"
"Wollen Sie folgendes Gerat sicher entfernen? "
"Eltavolitja az eszkozt?"
"Czy odlaczyc urzadzenie"

MHotPlugDisks
"(диск(и): %s)"
"(disk(s): %s)"
"(disk(y): %s)"
"(Laufwerk(e): %s)"
"(%s meghajto)"
"(dysk(i): %s)"

MChangeCouldNotEjectHotPlugMedia
"Невозможно удалить устройство для диска %c:"
"Cannot remove a device for drive %c:"
"Zarizeni %c: nemuze byt odpojeno."
"Ein Gerat fur Laufwerk %c: konnte nicht entfernt werden"
"%c: eszkoz nem tavolithato el"
"Nie udalo sie odlaczyc dysku %c:"

MChangeCouldNotEjectHotPlugMedia2
"Невозможно удалить устройство:"
"Cannot remove a device:"
"Zarizeni nemuze byt odpojeno."
"Kann folgendes Gerate nicht entfernen:"
"Az eszkoz nem tavolithato el:"
"Nie udalo sie odlaczyc urzadzenia:"

MChangeHotPlugNotify1
"Теперь устройство" 
"The device" 
"Zarizeni"
"Das Gerat"
"Az eszkoz:"
"Urzadzenie"

MChangeHotPlugNotify2
"может быть безопасно извлечено из компьютера"
"can now be safely removed"
"muze byt nyni bezpecne odebrano"
"kann nun vom Computer getrennt werden."
"mar biztonsagosan eltavolithato!"
"mozna teraz bezpiecznie odlaczyc"

MHotPlugListTitle
"Hotplug-устройства"
"Hotplug devices list"
"Seznam vyjimatelnych zarizeni"
"Hardware sicher entfernen"
"Hotplug eszkozok"
"Lista urzadzen Hotplug"

MHotPlugListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Tasten: Entf,StrgR,F1"
"Szerkesztes: Del,Ctrl-R"
"Edycja: Del,Ctrl-R"

MChangeDriveDisconnectTitle
l:
"Отключение сетевого устройства"
"Disconnect network drive"
"Odpojit sitovou jednotku"
"Netzwerklaufwerk trennen"
"Halozati meghajto levalasztasa"
"Odlaczanie dysku sieciowego"

MChangeDriveDisconnectQuestion
"Вы хотите удалить соединение с устройством %c:?"
"Do you want to disconnect from the drive %c:?"
"Opravdu si prejete odpojit od jednotky %c:?"
"Wollen Sie die Verbindung zu Laufwerk %c: trennen?"
"Le akar valni %c: meghajtorol?"
"Czy odlaczyc dysk %c:?"

MChangeDriveDisconnectMapped
"На устройство %c: отображена папка"
"The drive %c: is mapped to..."
"Jednotka %c: je namapovana na..."
"Laufwerk %c: ist verknupft zu..."
"%c: meghajto hozzarendelve:"
"Dysk %c: jest skojarzony z..."

MChangeDriveDisconnectReconnect
"&Восстанавливать при входе в систему"
"&Reconnect at Logon"
"&Znovu pripojit pri prihlaseni"
"Bei Anmeldung &verbinden"
"&Bejelentkezesnel ujracsatlakoztat"
"&Podlacz ponownie przy logowaniu"

MChangeDriveAskDisconnect
l:
"Вы хотите в любом случае отключиться от устройства?"
"Do you want to disconnect the device anyway?"
"Prejete si presto zarizeni odpojit?"
"Wollen Sie die Verbindung trotzdem trennen?"
"Mindenkeppen levalasztja az eszkozt?"
"Czy chcesz mimo to odlaczyc urzadzenie?"

MChangeVolumeInUse
"Не удается извлечь диск из привода %c:"
"Volume %c: cannot be ejected."
"Jednotka %c: nemuze byt vysunuta."
"Datentrager %c: kann nicht ausgeworfen werden."
"%c: kotet nem oldhato ki"
"Nie mozna wysunac dysku %c."

MChangeVolumeInUse2
"Используется другим приложением"
"It is used by another application"
"Je pouzivana jinou aplikaci"
"Andere Programme greifen momentan darauf zu"
"Masik program hasznalja"
"Jest uzywany przez inna aplikacje"

MChangeWaitingLoadDisk
"Ожидание чтения диска..."
"Waiting for disk to mount..."
"Cekam na disk k pripojeni..."
"Warte auf Datentrager..."
"Lemez betoltese..."
"Trwa montowanie dysku..."

MChangeCouldNotEjectMedia
"Невозможно извлечь диск из привода %c:"
"Could not eject media from drive %c:"
"Nelze vysunout medium v jednotce %c:"
"Konnte Medium in Laufwerk %c: nicht auswerfen"
"%c: meghajto lemeze nem oldhato ki"
"Nie mozna wysunac dysku z napedu %c:"

MAdditionalHotKey
"#!$%*+-/(),."
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
" Kereses "
" Szukaj "

MCannotCreateListFile
"Ошибка создания списка файлов"
"Cannot create list file"
"Nelze vytvorit soubor se seznamem"
"Dateiliste konnte nicht erstellt werden"
"A listafajl nem hozhato letre"
"Nie moge utworzyc listy plikow"

MCannotCreateListTemp
"(невозможно создать временный файл для списка)"
"(cannot create temporary file for list)"
"(nemohu vytvorit docasny soubor pro seznam)"
"(Fehler beim Anlegen einer temporaren Datei fur Liste)"
"(a lista atmeneti fajl nem hozhato letre)"
"(nie mozna utworzyc pliku tymczasowego dla listy)"

MCannotCreateListWrite
"(невозможно записать данные в файл)"
"(cannot write data in file)"
"(nemohu zapsat data do souboru)"
"(Fehler beim Schreiben der Daten)"
"(a fajlba nem irhato adat)"
"(nie mozna zapisac danych do pliku)"

MDragFiles
l:
"%d файлов"
"%d files"
"%d souboru"
"%d Dateien"
"%d fajl"
"%d plikow"

MDragMove
"Перенос %s"
"Move %s"
"Presunout %s"
"Verschiebe %s"
"%s mozgatasa"
"Przenies %s"

MDragCopy
"Копирование %s"
"Copy %s"
"Kopirovat %s"
"Kopiere %s"
"%s masolasa"
"Kopiuj %s"

MProcessListTitle
l:
"Список задач"
"Task list"
"Seznam uloh"
"Taskliste"
"Futo programok"
"Lista zadan"

MProcessListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Tasten: Entf,StrgR"
"Szerk.: Del,Ctrl-R"
"Edycja: Del,Ctrl-R"

MKillProcessTitle
"Удаление задачи"
"Kill task"
"Zabit ulohu"
"Task beenden"
"Programkiloves"
"Zakoncz zadanie"

MAskKillProcess
"Вы хотите удалить выбранную задачу?"
"Do you wish to kill selected task?"
"Prejete si zabit vybranou ulohu?"
"Wollen Sie den ausgewahlten Task beenden?"
"Ki akarja loni a kijelolt programot?"
"Czy chcesz zakonczyc wybrane zadanie?"

MKillProcessWarning
"Вы потеряете всю несохраненную информацию этой программы"
"You will lose any unsaved information in this program"
"V tomto programu budou ztraceny neulozene informace"
"Alle ungespeicherten Daten dieses Programmes gehen verloren."
"A program minden mentetlen adata elvesz!"
"Utracisz wszystkie niezapisane dane w tym programie"

MKillProcessKill
"Удалить"
"Kill"
"Zabit"
"Beenden"
"Kilo"
"Zakoncz"

MCannotKillProcess
"Указанную задачу удалить не удалось"
"Cannot kill the specified task"
"Nemohu ukoncit zvolenou ulohu"
"Task konnte nicht beendet werden"
"A programot nem lehet kiloni"
"Nie moge zakonczyc wybranego zadania"

MCannotKillProcessPerm
"Вы не имеет права удалить этот процесс."
"You have no permission to kill this process."
"Nemate opravneni zabit tento proces."
"Sie haben keine Rechte um diesen Prozess zu beenden."
"Nincs joga a program kilovesere"
"Nie masz wystarczajacych uprawnien do zakonczenia procesu."

MQuickViewTitle
l:
"Быстрый просмотр"
"Quick view"
"Zbezne zobrazeni"
"Schnellansicht"
"Gyorsnezet"
"Szybki podglad"

MQuickViewFolder
"Папка"
"Folder"
"Adresar"
"Verzeichnis"
"Mappa"
"Folder"

MQuickViewJunction
"Связь"
"Junction"
"Krizeni"
"Knotenpunkt"
"Csomopont"
"Powiazanie"

MQuickViewSymlink
"Ссылка"
"Symlink"
"Symbolicky link"
"Symlink"
"Szimlink"
"Link"

MQuickViewVolMount
"Том"
"Volume"
"Svazek"
"Datentrager"
"Kotet"
"Naped"

MQuickViewContains
"Содержит:"
"Contains:"
"Obsah:"
"Enthalt:"
"Tartalma:"
"Zawiera:"

MQuickViewFolders
"Папок               "
"Folders          "
"Adresare           "
"Ordner           "
"Mappak szama     "
"Katalogi            "

MQuickViewFiles
"Файлов              "
"Files            "
"Soubory            "
"Dateien          "
"Fajlok szama     "
"Pliki               "

MQuickViewBytes
"Размер файлов       "
"Files size       "
"Velikost souboru   "
"Gesamtgro?e      "
"Fajlok merete    "
"Rozmiar plikow      "

MQuickViewCompressed
"Упакованный размер  "
"Compressed size  "
"Komprim. velikost  "
"Komprimiert      "
"Tomoritett meret "
"Po kompresji        "

MQuickViewRatio
"Степень сжатия      "
"Ratio            "
"Pomer              "
"Rate             "
"Tomorites aranya "
"Procent             "

MQuickViewCluster
"Размер кластера     "
"Cluster size     "
"Velikost clusteru  "
"Clustergro?e     "
"Klasztermeret    "
"Rozmiar klastra     "

MQuickViewRealSize
"Реальный размер     "
"Real files size  "
"Opravdova velikost "
"Tatsachlich      "
"Valodi fajlmeret "
"Wlasciwy rozmiar    "

MQuickViewSlack
"Остатки кластеров   "
"Files slack      "
"Mrtve misto        "
"Verlust          "
"Meddo terulet    "
"Przestrzen stracona "

MSetAttrTitle
l:
"Атрибуты"
"Attributes"
"Atributy"
"Attribute"
"Attributumok"
"Atrybuty"

MSetAttrFor
"Изменить файловые атрибуты"
"Change file attributes for"
"Zmena atributu souboru pro"
"Andere Dateiattribute fur"
"Attributumok megvaltoztatasa"
"Zmien atrybuty dla"

MSetAttrSelectedObjects
"выбранных объектов"
"selected objects"
"vybrane objekty"
"markierte Objekte"
"a kijelolt objektumokon"
"wybranych obiektow"

MSetAttrHardLinks
"жестких ссылок (%d)"
"hard links (%d)"
"pevne linky (%d)"
"Hardlinks (%d)"
"hardlink (%d)"
"linkow trwalych (%d)"

MSetAttrJunction
"Связь:"
"Junction:"
"Krizeni:"
"Knotenpunkte:"
"Сsomopont:"
"Powiazanie:"

MSetAttrSymlink
"Ссылка:"
"Symlink:"
"Link:"
"Symlink:"
"Szimlink:"
"Link:"

MSetAttrVolMount
"Том:"
"Volume:"
"Svazek:"
"Datentrager:"
"Kotet:"
"Punkt zamontowania:"

MSetAttrUnknownJunction
"(нет данных)"
"(data not available)"
"(data nejsou k dispozici)"
"(nicht verfugbar)"
"(adat nem elerheto)"
"(dane niedostepne)"

MSetAttrRO
"&Только для чтения"
"&Read only"
"&Pouze pro cteni"
"Sch&reibschutz"
"&Csak olvashato"
"&Tylko do odczytu"

MSetAttrArchive
"&Архивный"
"&Archive"
"&Archivovat"
"&Archiv"
"&Archiv"
"&Archiwalny"

MSetAttrHidden
"&Скрытый"
"&Hidden"
"&Skryty"
"&Versteckt"
"&Rejtett"
"&Ukryty"

MSetAttrSystem
"С&истемный"
"&System"
"S&ystemovy"
"&System"
"Ren&dszer"
"&Systemowy"

MSetAttrCompressed
"Сжаты&й"
"&Compressed"
"&Komprimovany"
"&Komprimiert"
"&Tomoritett"
"S&kompresowany"

MSetAttrEncrypted
"За&шифрованный"
"&Encrypted"
"&Sifrovany"
"V&erschlusselt"
"Tit&kositott"
"Zaszy&frowany"

MSetAttrNotIndexed
"Н&еиндексируемый"
"Not &Indexed"
"Neinde&xovany"
"Nicht &indiziert"
"Nem inde&xelt"
"Nie z&indeksowany"

MSetAttrSparse
"Разре&женный"
"S&parse"
upd:"Rozptyleny"
upd:"Reserve"
"Ritk&itott"
upd:"Sparse"

MSetAttrTemp
"Временный"
"Temporary"
"Docasny"
"Temporar"
"&Atmeneti"
"&Tymczasowy"

MSetAttrOffline
"Автономный"
"Offline"
"Offline"
"Offline"
"O&ffline"
"Offline"

MSetAttrVirtual
"Виртуальный"
"Virtual"
"Virtualni"
"Virtuell"
"&Virtualis"
"Wirtualny"

MSetAttrSubfolders
"Обрабатывать &вложенные папки"
"Process sub&folders"
"Zpracovat i po&dadresare"
"Unterordner miteinbe&ziehen"
"Az almappakon is"
"Przetwarzaj &podkatalogi"

MSetAttrModification
"Время &модификации файла:"
"File &modification time:"
"Cas &modifikace souboru:"
"Datei &modifiziert:"
"&Modositas datuma/ideje:"
"Czas ostatniej &modyfikacji:"

MSetAttrCreation
"Время со&здания файла:"
"File crea&tion time:"
"Cas v&ytvoreni souboru:"
"Datei erstell&t:"
"&Letrehozas datuma/ideje:"
"Czas u&tworzenia:"

MSetAttrLastAccess
"Время последнего &доступа к файлу:"
"&Last file access time:"
"Cas posledniho pri&stupu:"
"&Letzter Zugriff:"
"&Utolso hozzaferes datuma/ideje:"
"Czas ostatniego &dostepu:"

MSetAttrOriginal
"Исход&ное"
"&Original"
"&Original"
"&Original"
"&Eredeti"
"Wstaw &oryginalny"

MSetAttrCurrent
"Те&кущее"
"Curre&nt"
"So&ucasny"
"Akt&uell"
"Aktual&is"
"Wstaw &biezacy"

MSetAttrBlank
"Сбр&ос"
"&Blank"
"P&razdny"
"L&eer"
"&Ures"
"&Wyczysc"

MSetAttrSet
"Установить"
"Set"
"Nastavit"
"Setzen"
"Alkalmaz"
"Usta&w"

MSetAttrTimeTitle1
l:
"ММ%cДД%cГГГГ чч%cмм%cсс%cмс"
"MM%cDD%cYYYY hh%cmm%css%cms"
upd:"MM%cDD%cRRRR hh%cmm%css%cms"
upd:"MM%cTT%cJJJJ hh%cmm%css%cms"
upd:"HH%cNN%cEEEE oo%cpp%cmm%cms"
upd:"MM%cDD%cRRRR gg%cmm%css%cms"

MSetAttrTimeTitle2
"ДД%cММ%cГГГГ чч%cмм%cсс%cмс"
"DD%cMM%cYYYY hh%cmm%css%cms"
upd:"DD%cMM%cRRRR hh%cmm%css%cms"
upd:"TT%cMM%cJJJJ hh%cmm%css%cms"
upd:"NN%cHH%cEEEE oo%cpp%cmm%cms"
upd:"DD%cMM%cRRRR gg%cmm%css%cms"

MSetAttrTimeTitle3
"ГГГГ%cММ%cДД чч%cмм%cсс%cмс"
"YYYY%cMM%cDD hh%cmm%css%cms"
upd:"RRRR%cMM%cDD hh%cmm%css%cms"
upd:"JJJJ%cMM%cTT hh%cmm%css%cms"
upd:"EEEE%cHH%cNN oo%cpp%cmm%cms"
upd:"RRRR%cMM%cDD gg%cmm%css%cms"

MSetAttrSetting
l:
"Установка файловых атрибутов для"
"Setting file attributes for"
"Nastaveni atributu souboru pro"
"Setze Dateiattribute fur"
"Attributumok beallitasa"
"Ustawiam atrybuty"

MSetAttrCannotFor
"Ошибка установки атрибутов для"
"Cannot set attributes for"
"Nelze nastavit atributy pro"
"Konnte Dateiattribute nicht setzen fur"
"Az attributumok nem allithatok be:"
"Nie moge ustawic atrybutow dla"

MSetAttrCompressedCannotFor
"Не удалось установить атрибут СЖАТЫЙ для"
"Cannot set attribute COMPRESSED for"
"Nelze nastavit atribut KOMPRIMOVANY pro"
"Konnte Komprimierung nicht setzen fur"
"A TOMORITETT attributum nem allithato be:"
"Nie moge ustawic atrybutu SKOMPRESOWANY dla"

MSetAttrEncryptedCannotFor
"Не удалось установить атрибут ЗАШИФРОВАННЫЙ для"
"Cannot set attribute ENCRYPTED for"
"Nelze nastavit atribut SIFROVANY pro"
"Konnte Verschlusselung nicht setzen fur"
"A TITKOSITOTT attributum nem allithato be:"
"Nie moge ustawic atrybutu ZASZYFROWANY dla"

MSetAttrSparseCannotFor
"Не удалось установить атрибут РАЗРЕЖЕННЫЙ для"
"Cannot set attribute SPARSE for"
upd:"Cannot set attribute SPARSE for"
upd:"Cannot set attribute SPARSE for"
"A RITKITOTT attributum nem allithato be:"
upd:"Cannot set attribute SPARSE for"

MSetAttrTimeCannotFor
"Не удалось установить время файла для"
"Cannot set file time for"
"Nelze nastavit cas souboru pro"
"Konnte Dateidatum nicht setzen fur"
"A datum nem allithato be:"
"Nie moge ustawic czasu pliku dla"

MSetColorPanel
l:
"&Панель"
"&Panel"
"&Panel"
"&Panel"
"&Panel"
"&Panel"

MSetColorDialog
"&Диалог"
"&Dialog"
"&Dialog"
"&Dialog"
"Par&beszedablak"
"Okno &dialogowe"

MSetColorWarning
"Пр&едупреждение"
"&Warning message"
"&Varovna zprava"
"&Warnmeldung"
"&Figyelmeztetes"
"&Ostrzezenie"

MSetColorMenu
"&Меню"
"&Menu"
"&Menu"
"&Menu"
"&Menu"
"&Menu"

MSetColorHMenu
"&Горизонтальное меню"
"Hori&zontal menu"
"Hori&zontalni menu"
"Hori&zontales Menu"
"&Vizszintes menu"
"Pa&sek menu"

MSetColorKeyBar
"&Линейка клавиш"
"&Key bar"
"&Radek klaves"
upd:"&Key bar"
"F&unkciobill.sor"
"Pasek &klawiszy"

MSetColorCommandLine
"&Командная строка"
"&Command line"
"Pri&kazovy radek"
"&Kommandozeile"
"P&arancssor"
"&Linia polecen"

MSetColorClock
"&Часы"
"C&lock"
"&Hodiny"
"U&hr"
"O&ra"
"&Zegar"

MSetColorViewer
"Про&смотрщик"
"&Viewer"
"P&rohlizec"
"&Betrachter"
"&Nezoke"
"Pod&glad"

MSetColorEditor
"&Редактор"
"&Editor"
"&Editor"
"&Editor"
"&Szerkeszto"
"&Edytor"

MSetColorHelp
"П&омощь"
"&Help"
"&Napoveda"
"&Hilfe"
"Su&go"
"P&omoc"

MSetDefaultColors
"&Установить стандартные цвета"
"Set de&fault colors"
"N&astavit vychozi barvy"
"Setze Standard&farben"
"Alapert. s&zinek"
"&Ustaw kolory domyslne"

MSetBW
"Черно-бел&ый режим"
"&Black and white mode"
"Cerno&bily mod"
"Schwarz && &Wei?"
"Fekete-fe&her mod"
"&Tryb czarno-bialy"

MSetColorPanelNormal
l:
"Обычный текст"
"Normal text"
"Normalni text"
"Normaler Text"
"Normal szoveg"
"Normalny tekst"

MSetColorPanelSelected
"Выбранный текст"
"Selected text"
"Vybrany text"
"Markierter Text"
"Kijelolt szoveg"
"Wybrany tekst"

MSetColorPanelHighlightedInfo
"Выделенная информация"
"Highlighted info"
"Info zvyraznene"
"Markierung"
"Kiemelt info"
"Podswietlone info"

MSetColorPanelDragging
"Перетаскиваемый текст"
"Dragging text"
"Tazeny text"
"Drag && Drop Text"
"Vonszolt szoveg"
"Przeciagany tekst"

MSetColorPanelBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"
"Ramka"

MSetColorPanelNormalCursor
"Обычный курсор"
"Normal cursor"
"Normalni kurzor"
"Normale Auswahl"
"Normal kurzor"
"Normalny kursor"

MSetColorPanelSelectedCursor
"Выделенный курсор"
"Selected cursor"
"Vybrany kurzor"
"Markierte Auswahl"
"Kijelolt kurzor"
"Wybrany kursor"

MSetColorPanelNormalTitle
"Обычный заголовок"
"Normal title"
"Normalni nadpis"
"Normaler Titel"
"Normal nev"
"Normalny tytul"

MSetColorPanelSelectedTitle
"Выделенный заголовок"
"Selected title"
"Vybrany nadpis"
"Markierter Titel"
"Kijelolt nev"
"Wybrany tytul"

MSetColorPanelColumnTitle
"Заголовок колонки"
"Column title"
"Nadpis sloupce"
"Spaltentitel"
"Oszlopnev"
"Tytul kolumny"

MSetColorPanelTotalInfo
"Количество файлов"
"Total info"
"Info celkove"
"Gesamtinfo"
"Osszes info"
"Calkowite info"

MSetColorPanelSelectedInfo
"Количество выбранных файлов"
"Selected info"
"Info vyber"
"Markierungsinfo"
"Kijelolt info"
"Wybrane info"

MSetColorPanelScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvnik"
"Scrollbalken"
"Gorditosav"
"Suwak"

MSetColorPanelScreensNumber
"Количество фоновых экранов"
"Number of background screens"
"Pocet obrazovek na pozadi"
"Anzahl an Hintergrundseiten"
"Hatterkepernyok szama"
"Ilosc ekranow w tle "

MSetColorDialogNormal
l:
"Обычный текст"
"Normal text"
"Normalni text"
"Normaler Text"
"Normal szoveg"
"Tekst zwykly"

MSetColorDialogHighlighted
"Выделенный текст"
"Highlighted text"
"Zvyrazneny text"
"Markierter Text"
"Kiemelt szoveg"
"Tekst podswietlony"

MSetColorDialogDisabled
"Блокированный текст"
"Disabled text"
"Zakazany text"
"Deaktivierter Text"
"Inaktiv szoveg"
"Tekst nieaktywny"

MSetColorDialogBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"
"Ramka"

MSetColorDialogBoxTitle
"Заголовок рамки"
"Title"
"Nadpis"
"Titel"
"Keret neve"
"Tytul"

MSetColorDialogHighlightedBoxTitle
"Выделенный заголовок рамки"
"Highlighted title"
"Zvyrazneny nadpis"
"Markierter Titel"
"Kiemelt keretnev"
"Podswietlony tytul"

MSetColorDialogTextInput
"Ввод текста"
"Text input"
"Textovy vstup"
"Texteingabe"
"Beirt szoveg"
"Wpisywany tekst"

MSetColorDialogUnchangedTextInput
"Неизмененный текст"
"Unchanged text input"
"Nezmeneny textovy vstup"
"Unveranderte Texteingabe"
"Valtozatlan beirt szoveg"
"Niezmieniony wpisywany tekst "

MSetColorDialogSelectedTextInput
"Ввод выделенного текста"
"Selected text input"
"Vybrany textovy vstup"
"Markierte Texteingabe"
"Beirt szoveg kijelolve"
"Zaznaczony wpisywany tekst"

MSetColorDialogEditDisabled
"Блокированное поле ввода"
"Disabled input line"
"Zakazany vstupni radek"
"Deaktivierte Eingabezeile"
"Inaktiv beviteli sor"
"Nieaktywna linia wprowadzania danych"

MSetColorDialogButtons
"Кнопки"
"Buttons"
"Tlacitka"
"Schaltflachen"
"Gombok"
"Przyciski"

MSetColorDialogSelectedButtons
"Выбранные кнопки"
"Selected buttons"
"Vybrana tlacitka"
"Aktive Schaltflachen"
"Kijelolt gombok"
"Wybrane przyciski"

MSetColorDialogHighlightedButtons
"Выделенные кнопки"
"Highlighted buttons"
"Zvyraznena tlacitka"
"Markierte Schaltflachen"
"Kiemelt gombok"
"Podswietlone przyciski"

MSetColorDialogSelectedHighlightedButtons
"Выбранные выделенные кнопки"
"Selected highlighted buttons"
"Vybrana zvyraznena tlacitka"
"Aktive markierte Schaltflachen"
"Kijelolt kiemelt gombok"
"Wybrane podswietlone przyciski "

MSetColorDialogListBoxControl
"Список"
"List box"
"Seznam polozek"
"Listenfelder"
"Listaablak"
"Lista"

MSetColorDialogComboBoxControl
"Комбинированный список"
"Combobox"
"Vyber polozek"
"Kombinatiosfelder"
"Lenyilo szovegablak"
"Pole combo"

MSetColorDialogListText
l:
"Обычный текст"
"Normal text"
"Normalni text"
"Normaler Text"
"Normal szoveg"
"Tekst zwykly"

MSetColorDialogListSelectedText
"Выбранный текст"
"Selected text"
"Vybrany text"
"Markierter Text"
"Kijelolt szoveg"
"Tekst wybrany"

MSetColorDialogListHighLight
"Выделенный текст"
"Highlighted text"
"Zvyrazneny text"
"Markierung"
"Kiemelt szoveg"
"Tekst podswietlony"

MSetColorDialogListSelectedHighLight
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybrany zvyrazneny text"
"Aktive Markierung"
"Kijelolt kiemelt szoveg"
"Tekst wybrany i podswietlony"

MSetColorDialogListDisabled
"Блокированный пункт"
"Disabled item"
"Naktivni polozka"
"Deaktiviertes Element"
"Inaktiv elem"
"Pole nieaktywne"

MSetColorDialogListBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"
"Ramka"

MSetColorDialogListTitle
"Заголовок"
"Title"
"Nadpis"
"Titel"
"Keret neve"
"Tytul"

MSetColorDialogListGrayed
"Серый текст списка"
"Grayed list text"
upd:"Grayed list text"
upd:"Grayed list text"
"Szurke listaszoveg"
upd:"Grayed list text"

MSetColorDialogSelectedListGrayed
"Выбранный серый текст списка"
"Selected grayed list text"
upd:"Selected grayed list text"
upd:"Selected grayed list text"
"Kijelolt szurke listaszoveg"
upd:"Selected grayed list text"

MSetColorDialogListScrollBar
"Полоса прокрутки"
"Scrollbar"
"Posuvnik"
"Scrollbalken"
"Gorditosav"
"Suwak"

MSetColorDialogListArrows
"Индикаторы длинных строк"
"Long string indicators"
"Znacka dlouheho retezce"
"Indikator fur lange Zeichenketten"
"Hosszu sztring jelzok"
"Znacznik dlugiego napisu"

MSetColorDialogListArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"
"Vybrana znacka dlouheho retezce"
"Aktiver Indikator"
"Kijelolt hosszu sztring jelzok"
"Zaznaczone znacznik dlugiego napisu"

MSetColorDialogListArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"
"Zakazana znacka dlouheho retezce"
"Deaktivierter Indikator"
"Inaktiv hosszu sztring jelzok"
"Nieaktywny znacznik dlugiego napisu"

MSetColorMenuNormal
l:
"Обычный текст"
"Normal text"
"Normalni text"
"Normaler Text"
"Normal szoveg"
"Normalny tekst"

MSetColorMenuSelected
"Выбранный текст"
"Selected text"
"Vybrany text"
"Markierter Text"
"Kijelolt szoveg"
"Wybrany tekst"

MSetColorMenuHighlighted
"Выделенный текст"
"Highlighted text"
"Zvyrazneny text"
"Markierung"
"Kiemelt szoveg"
"Podswietlony tekst"

MSetColorMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybrany zvyrazneny text"
"Aktive Markierung"
"Kijelolt kiemelt szoveg"
"Wybrany podswietlony tekst "

MSetColorMenuDisabled
"Недоступный пункт"
"Disabled text"
"Neaktivni text"
"Disabled text"
"Inaktiv szoveg"
"Tekst nieaktywny"

MSetColorMenuGrayed
"Серый текст"
"Grayed text"
upd:"Grayed text"
upd:"Grayed text"
"Szurke szoveg"
upd:"Grayed text"

MSetColorMenuSelectedGrayed
"Выбранный серый текст"
"Selected grayed text"
upd:"Selected grayed text"
upd:"Selected grayed text"
"Kijelolt szurke szoveg"
upd:"Selected grayed text"

MSetColorMenuBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"
"Ramka"

MSetColorMenuTitle
"Заголовок"
"Title"
"Nadpis"
"Titel"
"Keret neve"
"Tytul"

MSetColorMenuScrollBar
"Полоса прокрутки"
"Scrollbar"
"Posuvnik"
"Scrollbalken"
"Gorditosav"
"Suwak"

MSetColorMenuArrows
"Индикаторы длинных строк"
"Long string indicators"
"Znacka dlouheho retezce"
"Long string indicators"
"Hosszu sztring jelzok"
"Znacznik dlugiego napisu"

MSetColorMenuArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"
"Vybrana znacka dlouheho retezce"
"Selected long string indicators"
"Kijelolt hosszu sztring jelzok"
"Zaznaczone znacznik dlugiego napisu"

MSetColorMenuArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"
"Zakazana znacka dlouheho retezce"
"Disabled long string indicators"
"Inaktiv hosszu sztring jelzok"
"Nieaktywny znacznik dlugiego napisu"

MSetColorHMenuNormal
l:
"Обычный текст"
"Normal text"
"Normalni text"
"Normaler Text"
"Normal szoveg"
"Normalny tekst"

MSetColorHMenuSelected
"Выбранный текст"
"Selected text"
"Vybrany text"
"Markierter Text"
"Kijelolt szoveg"
"Wybrany tekst"

MSetColorHMenuHighlighted
"Выделенный текст"
"Highlighted text"
"Zvyrazneny text"
"Markierung"
"Kiemelt szoveg"
"Podswietlony tekst"

MSetColorHMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybrany zvyrazneny text"
"Aktive Markierung"
"Kijelolt kiemelt szoveg"
"Wybrany podswietlony tekst "

MSetColorKeyBarNumbers
l:
"Номера клавиш"
"Key numbers"
"Cisla klaves"
"Tastenziffern"
"Funkcio szama"
"Numery klawiszy"

MSetColorKeyBarNames
"Названия клавиш"
"Key names"
"Nazvy klaves"
"Tastennamen"
"Funkcio neve"
"Nazwy klawiszy"

MSetColorKeyBarBackground
"Фон"
"Background"
"Pozadi"
"Hintergrund"
"Hattere"
"Tlo"

MSetColorCommandLineNormal
l:
"Обычный текст"
"Normal text"
"Normalni text"
"Normaler Text"
"Normal szoveg"
"Normalny tekst"

MSetColorCommandLineSelected
"Выделенный текст"
"Selected text input"
"Vybrany textovy vstup"
"Markierte Texteingabe"
"Beirt szoveg kijelolve"
"Zaznaczony wpisany tekst"

MSetColorCommandLinePrefix
"Текст префикса"
"Prefix text"
"Text predpony"
"Prefix Text"
"Elotag szovege"
"Tekst prefiksu"

MSetColorCommandLineUserScreen
"Пользовательский экран"
"User screen"
"Obrazovka uzivatele"
"Benutzerseite"
"Konzol hattere"
"Ekran uzytkownika"

MSetColorClockNormal
l:
"Обычный текст (панели)"
"Normal text (Panel)"
"Normalni text (Panel)"
"Normaler Text (Panel)"
"Normal szoveg (panelek)"
"Normalny tekst (Panel)"

MSetColorClockNormalEditor
"Обычный текст (редактор)"
"Normal text (Editor)"
"Normalni text (Editor)"
"Normaler Text (Editor)"
"Normal szoveg (szerkeszto)"
"Normalny tekst (Edytor)"

MSetColorClockNormalViewer
"Обычный текст (вьювер)"
"Normal text (Viewer)"
"Normalni text (Prohlizec)"
"Normaler Text (Betrachter)"
"Normal szoveg (nezoke)"
"Normalny tekst (Podglad)"

MSetColorViewerNormal
l:
"Обычный текст"
"Normal text"
"Normalni text"
"Normaler Text"
"Normal szoveg"
"Normalny tekst"

MSetColorViewerSelected
"Выбранный текст"
"Selected text"
"Vybrany text"
"Markierter Text"
"Kijelolt szoveg"
"Zaznaczony tekst"

MSetColorViewerStatus
"Статус"
"Status line"
"Stavovy radek"
"Statuszeile"
"Allapotsor"
"Linia statusu"

MSetColorViewerArrows
"Стрелки сдвига экрана"
"Screen scrolling arrows"
"Skrolovaci sipky"
"Pfeile auf Scrollbalken"
"Kepernyogordito nyilak"
"Strzalki przesuwajace ekran"

MSetColorViewerScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvnik"
"Scrollbalken"
"Gorditosav"
"Suwak"

MSetColorEditorNormal
l:
"Обычный текст"
"Normal text"
"Normalni text"
"Normaler Text"
"Normal szoveg"
"Normalny tekst"

MSetColorEditorSelected
"Выбранный текст"
"Selected text"
"Vybrany text"
"Markierter Text"
"Kijelolt szoveg"
"Zaznaczony tekst"

MSetColorEditorStatus
"Статус"
"Status line"
"Stavovy radek"
"Statuszeile"
"Allapotsor"
"Linia statusu"

MSetColorEditorScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvnik"
"Scrollbalken"
"Gorditosav"
"Suwak"

MSetColorHelpNormal
l:
"Обычный текст"
"Normal text"
"Normalni text"
"Normaler Text"
"Normal szoveg"
"Normalny tekst"

MSetColorHelpHighlighted
"Выделенный текст"
"Highlighted text"
"Zvyrazneny text"
"Markierung"
"Kiemelt szoveg"
"Podswietlony tekst"

MSetColorHelpReference
"Ссылка"
"Reference"
"Odkaz"
"Referenz"
"Hivatkozas"
"Odniesienie"

MSetColorHelpSelectedReference
"Выбранная ссылка"
"Selected reference"
"Vybrany odkaz"
"Ausgewahlte Referenz"
"Kijelolt hivatkozas"
"Wybrane odniesienie "

MSetColorHelpBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"
"Ramka"

MSetColorHelpBoxTitle
"Заголовок рамки"
"Title"
"Nadpis"
"Titel"
"Keret neve"
"Tytul"

MSetColorHelpScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvnik"
"Scrollbalken"
"Gorditosav"
"Suwak"

MSetColorGroupsTitle
l:
"Цветовые группы"
"Color groups"
"Skupiny barev"
"Farbgruppen"
"Szincsoportok"
"Grupy kolorow"

MSetColorItemsTitle
"Элементы группы"
"Group items"
"Polozky skupin"
"Gruppeneintrage"
"A szincsoport elemei"
"Elementy grupy"

MSetColorTitle
l:
"Цвет"
"Color"
"Barva"
"Farbe"
"Szinek"
"Kolor"

MSetColorForeground
"&Текст"
"&Foreground"
"&Popredi"
"&Vordergrund"
"&Eloter"
"&Pierwszy plan"

MSetColorBackground
"&Фон"
"&Background"
"Po&zadi"
"&Hintergrund"
"&Hatter"
"&Tlo"

MSetColorForeTransparent
"&Прозрачный"
"&Transparent"
"Pruhlednos&t"
"&Transparent"
"Atla&tszo"
"P&rzezroczyste"

MSetColorBackTransparent
"П&розрачный"
"T&ransparent"
"Pruhledno&st"
"T&ransparent"
"Atlat&szo"
"Pr&zezroczyste"

MSetColorSample
"Текст Текст Текст Текст Текст Текст"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"
"Tekst Tekst Tekst Tekst Tekst Tekst"

MSetColorSet
"Установить"
"Set"
"Nastavit"
"Setzen"
"A&lkalmaz"
"Ustaw"

MSetColorCancel
"Отменить"
"Cancel"
"Storno"
"Abbruch"
"&Megsem"
"Anuluj"

MSetConfirmTitle
l:
"Подтверждения"
"Confirmations"
"Potvrzeni"
"Bestatigungen"
"Megerositesek"
"Potwierdzenia"

MSetConfirmCopy
"Перезапись файлов при &копировании"
"&Copy"
"&Kopirovani"
"&Kopieren"
"&Masolas"
"&Kopiowanie"

MSetConfirmMove
"Перезапись файлов при &переносе"
"&Move"
"&Presouvani"
"&Verschieben"
"Moz&gatas"
"&Przenoszenie"

MSetConfirmRO
"Перезапись и удаление R/O &файлов"
"&Overwrite and delete R/O files"
upd:"&Overwrite and delete R/O files"
upd:"&Overwrite and delete R/O files"
"&Csak olv. fajlok felulirasa/torlese"
upd:"&Overwrite and delete R/O files"


MSetConfirmDrag
"Пере&таскивание"
"&Drag and drop"
"&Drag and drop"
"&Ziehen und Ablegen"
"&Huzd es ejtsd"
"P&rzeciaganie i upuszczanie"

MSetConfirmDelete
"&Удаление"
"De&lete"
"&Mazani"
"&Loschen"
"&Torles"
"&Usuwanie"

MSetConfirmDeleteFolders
"У&даление непустых папок"
"Delete non-empty &folders"
"Mazat &neprazdne adresare"
"Loschen von Ordnern mit &Inhalt"
"Nem &ures mappak torlese"
"Usuwanie &niepustych katalogow"

MSetConfirmEsc
"Прерыва&ние операций"
"&Interrupt operation"
"Pre&rusit operaci"
"&Unterbrechen von Vorgangen"
"Mu&velet megszakitasa"
"&Przerwanie operacji"

MSetConfirmRemoveConnection
"&Отключение сетевого устройства"
"Disconnect &network drive"
"Odpojeni &sitove jednotky"
"Trennen von &Netzwerklaufwerken"
"Halo&zati meghajto levalasztasa"
"Odlaczenie dysku &sieciowego"

MSetConfirmRemoveSUBST
"Отключение SUBST-диска"
"Disconnect &SUBST-disk"
"Odpojeni SUBST-d&isku"
"Trennen von &Substlaufwerken"
"Virt&ualis meghajto torlese"
"Odlaczenie dysku &SUBST"

MSetConfirmRemoveHotPlug
"Отключение HotPlug-у&стройства"
"Hot&Plug-device removal"
"Odpojeni vyjimatelneho zarizeni"
"Sicheres Entfernen von Hardware"
"H&otPlug eszkoz eltavolitasa"
"Odlaczanie urzadzenia HotPlug"

MSetConfirmAllowReedit
"Повто&рное открытие файла в редакторе"
"&Reload edited file"
"&Obnovit upravovany soubor"
"Bea&rbeitete Datei neu laden"
"&Szerkesztett fajl ujratoltese"
"&Zaladuj edytowany plik"

MSetConfirmHistoryClear
"Очистка списка &истории"
"Clear &history list"
"Vymazat seznam &historie"
"&Historielisten loschen"
"&Elozmenylista torlese"
"Czyszczenie &historii"

MSetConfirmExit
"&Выход"
"E&xit"
"U&konceni"
"Be&enden"
"K&ilepes a FAR-bol"
"&Wyjscie"

MSetPluginConfirmationTitle
l:
"Выбор плагина"
"Plugin selection"
upd:"Plugin selection"
upd:"Plugin selection"
"Plugin valasztas"
upd:"Plugin selection"

MSetPluginConfirmationOFP
"Обработка файла (OpenFilePlugin)"
"File processing (OpenFilePlugin)"
upd:"File processing (OpenFilePlugin)"
upd:"File processing (OpenFilePlugin)"
"Fajl feldolgozasa (OpenFilePlugin)"
upd:"File processing (OpenFilePlugin)"

MSetPluginConfirmationStdAssoc
"Пункт вызова стандартной ассоциации"
"Show standard association item"
upd:"Show standard association item"
upd:"Show standard association item"
"Szabvany tarsitas megjelenitese"
upd:"Show standard association item"

MSetPluginConfirmationEvenOne
"Даже если найден всего один плагин"
"Even if only one plugin found"
upd:"Even if only one plugin found"
upd:"Even if only one plugin found"
"Akkor is, ha csak egy plugin van"
upd:"Even if only one plugin found"

MSetPluginConfirmationSFL
"Результаты поиска (SetFindList)"
"Search results (SetFindList)"
upd:"Search results (SetFindList)"
upd:"Search results (SetFindList)"
"Kereses eredmenye (SetFindList)"
upd:"Search results (SetFindList)"

MSetPluginConfirmationPF
"Обработка префикса"
"Prefix processing"
upd:"Prefix processing"
upd:"Prefix processing"
"Elotag feldolgozasa"
upd:"Prefix processing"

MFindFolderTitle
l:
"Поиск папки"
"Find folder"
"Najit adresar"
"Ordner finden"
"Mappakereses"
"Znajdz folder"

MKBFolderTreeF1
l:
l:// Find folder Tree KeyBar
"Помощь"
"Help"
"Napoveda"
"Hilfe"
"Sugo"
"Pomoc"

MKBFolderTreeF2
"Обновить"
"Rescan"
"Obnovit"
"Aktual"
"FaFris"
"Czytaj ponownie"

MKBFolderTreeF5
"Размер"
"Zoom"
"Zoom"
"Vergr."
"Nagyit"
"Powieksz"

MKBFolderTreeF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilep"
"Koniec"

MKBFolderTreeAltF9
"Видео"
"Video"
"Video"
"Vollb"
"Video"
"Video"

MTreeTitle
"Дерево"
"Tree"
"Stromove zobrazeni"
"Baum"
"Fa"
"Drzewo"

MCannotSaveTree
"Ошибка записи дерева папок в файл"
"Cannot save folders tree to file"
"Adresarovy strom nelze ulozit do souboru"
"Konnte Ordnerliste nicht in Datei speichern."
"A mappak fastrukturaja nem mentheto fajlba"
"Nie moge zapisac drzewa katalogow do pliku"

MReadingTree
"Чтение дерева папок"
"Reading the folders tree"
"Nacitam adresarovy strom"
"Lese Ordnerliste"
"Mappaszerkezet ujraolvasasa..."
"Odczytuje drzewo katalogow"

MUserMenuTitle
l:
"Пользовательское меню"
"User menu"
"Menu uzivatele"
"Benutzermenu"
"Felhasznaloi menu szerkesztese"
"Menu uzytkownika"

MChooseMenuType
"Выберите тип пользовательского меню для редактирования"
"Choose user menu type to edit"
"Zvol typ menu uzivatele pro upravu"
"Wahlen Sie den Typ des zu editierenden Benutzermenus"
"Felhasznaloi menu tipusa:"
"Wybierz typ menu do edycji"

MChooseMenuMain
"&Главное"
"&Main"
"&Hlavni"
"&Hauptmenu"
"&Fomenu"
"Glowne"

MChooseMenuLocal
"&Местное"
"&Local"
"&Lokalni"
"&Lokales Menu"
"&Helyi menu"
"Lokalne"

MMainMenuTitle
"Главное меню"
"Main menu"
"Hlavni menu"
"Hauptmenu"
"Fomenu"
"Menu glowne"

MMainMenuFAR
"Папка FAR"
"FAR folder"
"Slozka FARu"
"FAR Ordner"
"FAR mappa"
"Folder FAR-a"

MMainMenuREG
l:
l:// <...menu (Registry)>
"Реестр"
"Registry"
"Registry"
"Reg."
"Registry"
"Rejestr"

MLocalMenuTitle
"Местное меню"
"Local menu"
"Lokalni menu"
"Lokales Menu"
"Helyi menu"
"Menu lokalne"

MMainMenuBottomTitle
"Редактирование: Del,Ins,F4,Alt-F4"
"Edit: Del,Ins,F4,Alt-F4"
"Edit: Del,Ins,F4,Alt-F4"
"Bearb.: Entf,Einf,F4,Alt-F4"
"Szerk.: Del,Ins,F4,Alt-F4"
"Edycja: Del,Ins,F4,Alt-F4"

MAskDeleteMenuItem
"Вы хотите удалить пункт меню"
"Do you wish to delete the menu item"
"Prejete si smazat polozku v menu"
"Do you wish to delete the menu item"
"Biztosan torli a menuelemet?"
"Czy usunac pozycje menu"

MAskDeleteSubMenuItem
"Вы хотите удалить вложенное меню"
"Do you wish to delete the submenu"
"Prejete si smazat podmenu"
"Do you wish to delete the submenu"
"Biztosan torli az almenut?"
"Czy usunac podmenu"

MUserMenuInvalidInputLabel
"Неправильный формат метки меню!"
"Invalid format for UserMenu Label!"
"Neplatny format pro nazev Uzivatelskeho menu!"
"Invalid format for UserMenu Label!"
"A felhasznaloi menu nevformatuma ervenytelen!"
"Bledny format etykiety menu uzytkownika!"

MUserMenuInvalidInputHotKey
"Неправильный формат горячей клавиши!"
"Invalid format for Hot Key!"
"Neplatny format pro klavesovou zkratku!"
"Invalid format for Hot Key!"
"A gyorsbillentyu formatuma ervenytelen!"
"Bledny format klawisza skrotu!"

MEditMenuTitle
l:
"Редактирование пользовательского меню"
"Edit user menu"
"Editace uzivatelskeho menu"
"Menubefehl bearbeiten"
"Parancs szerkesztese"
"Edytuj menu uzytkownika"

MEditMenuHotKey
"&Горячая клавиша:"
"&Hot key:"
"K&lavesova zkratka:"
"&Kurztaste:"
"&Gyorsbillentyu:"
"&Klawisz skrotu:"

MEditMenuLabel
"&Метка:"
"&Label:"
"&Popisek:"
"&Bezeichnung:"
"&Nev:"
"&Etykieta:"

MEditMenuCommands
"&Команды:"
"&Commands:"
"Pri&kazy:"
"&Befehle:"
"&Parancsok:"
"&Polecenia:"

MAskInsertMenuOrCommand
l:
"Вы хотите вставить новую команду или новое меню?"
"Do you wish to insert a new command or a new menu?"
"Prejete si vlozit novy prikaz nebo nove menu?"
"Wollen Sie einen neuen Menubefehl oder ein neues Menu erstellen?"
"Uj parancs vagy uj menu?"
"Czy chcesz wstawic nowe polecenie lub nowe menu?"

MMenuInsertCommand
"Вставить команду"
"Insert command"
"Vlozit prikaz"
"Neuer Befehl"
"Parancs"
"Wstaw polecenie"

MMenuInsertMenu
"Вставить меню"
"Insert menu"
"Vlozit menu"
"Neues Menu"
"Menu"
"Wstaw menu"

MEditSubmenuTitle
l:
"Редактирование метки вложенного меню"
"Edit submenu label"
"Uprava popisku podmenu"
"Untermenu bearbeiten"
"Almenu szerkesztese"
"Edytuj etykiete podmenu"

MEditSubmenuHotKey
"&Горячая клавиша:"
"&Hot key:"
"Klavesova &zkratka:"
"&Kurztaste:"
"&Gyorsbillentyu:"
"&Klawisz skrotu:"

MEditSubmenuLabel
"&Метка:"
"&Label:"
"&Popisek:"
"&Bezeichnung:"
"&Nev:"
"&Etykieta:"

MViewerTitle
l:
"Просмотр"
"Viewer"
"Prohlizec"
"Betrachter"
"Nezoke"
"Podglad"

MViewerCannotOpenFile
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevrit soubor"
"Kann Datei nicht offnen"
"A fajl nem nyithato meg"
"Nie moge otworzyc pliku"

MViewerStatusCol
"Кол"
"Col"
"Sloupec"
"Spalte"
"Oszlop"
"Kolumna"

MViewSearchTitle
l:
"Поиск"
"Search"
"Hledat"
"Durchsuchen"
"Kereses"
"Szukaj"

MViewSearchFor
"&Искать"
"&Search for"
"H&ledat"
"&Suchen nach"
"&Kereses:"
"&Znajdz"

MViewSearchForText
"Искать &текст"
"Search for &text"
"Hledat &text"
"Suchen nach &Text"
"&Szoveg keresese"
"Szukaj &tekstu"

MViewSearchForHex
"Искать 16-ричный &код"
"Search for &hex"
"Hledat he&x"
"Suchen nach &Hex (xx xx ...)"
"&Hexa keresese"
"Szukaj &wartosci szesnastkowych"

MViewSearchCase
"&Учитывать регистр"
"&Case sensitive"
"&Rozlisovat velikost pismen"
"Gr&o?-/Kleinschreibung"
"&Nagy/kisbetu erzekeny"
"&Uwzglednij wielkosc liter"

MViewSearchWholeWords
"Только &целые слова"
"&Whole words"
"Cela &slova"
"Ganze &Worter"
"Csak e&gesz szavak"
"Tylko cale slowa"

MViewSearchReverse
"Обратн&ый поиск"
"Re&verse search"
"&Zpetne hledani"
"Richtung um&kehren"
"&Visszafele keres"
"Szukaj w &odwrotnym kierunku"

MViewSearchRegexp
"&Регулярные выражения"
"&Regular expressions"
upd:"&Regular expressions"
upd:"&Regular expressions"
upd:"&Regular expressions"
upd:"&Regular expressions"

MViewSearchSearch
"Искать"
"Search"
"Hledat"
"Suchen"
"Keres"
"&Szukaj"

MViewSearchCancel
"Отменить"
"Cancel"
"Storno"
"Abbrechen"
"Megsem"
"&Anuluj"

MViewSearchingFor
l:
"Поиск"
"Searching for"
"Vyhledavam"
"Suche nach"
"Kereses:"
"Szukam"

MViewSearchingHex
"Поиск байтов"
"Searching for bytes"
"Vyhledavam sekvenci bytu"
"Suche nach Bytes"
"Bajtok keresese:"
"Szukam bajtow"

MViewSearchCannotFind
"Строка не найдена"
"Could not find the string"
"Nelze najit retezec"
"Konnte Zeichenkette nicht finden"
"Nem talaltam a szoveget:"
"Nie moge odnalezc ciagu znakow"

MViewSearchCannotFindHex
"Байты не найдены"
"Could not find the bytes"
"Nelze najit sekvenci bytu"
"Konnte Bytefolge nicht finden"
"Nem talaltam a bajtokat:"
"Nie moge odnalezc bajtow"

MViewSearchFromBegin
"Продолжить поиск с начала документа?"
"Continue the search from the beginning of the document?"
"Pokracovat s hledanim od zacatku dokumentu?"
"Mit Suche am Anfang des Dokuments fortfahren?"
"Folytassam a keresest a dokumentum elejetol?"
"Kontynuowac wyszukiwanie od poczatku dokumentu?"

MViewSearchFromEnd
"Продолжить поиск с конца документа?"
"Continue the search from the end of the document?"
"Pokracovat s hledanim od konce dokumentu?"
"Mit Suche am Ende des Dokuments fortfahren?"
"Folytassam a keresest a dokumentum vegetol?"
"Kontynuowac wyszukiwanie od konca dokumentu?"

MPrintTitle
l:
"Печать"
"Print"
"Tisk"
"Drucken"
"Nyomtatas"
"Drukuj"

MPrintTo
"Печатать %s на"
"Print %s to"
"Vytisknout %s na"
"Drucke %s nach"
"%s nyomtatasa:"
"Drukuj %s do"

MPrintFilesTo
"Печатать %d файлов на"
"Print %d files to"
"Vytisknout %d souboru na"
"Drucke %d Dateien mit"
"%d fajl nyomtatasa:"
"Drukuj %d pliki(ow) do"

MPreparingForPrinting
"Подготовка файлов к печати"
"Preparing files for printing"
"Pripravuji soubory pro tisk"
"Vorbereiten der Druckauftrage"
"Fajlok elokeszitese nyomtatashoz"
"Przygotowuje plik(i) do drukowania"

MCannotEnumeratePrinters
"Не удалось получить список доступных принтеров"
"Cannot enumerate available printers list"
upd:"Cannot enumerate available printers list"
upd:"Cannot enumerate available printers list"
"Az elerheto nyomtatok listaja nem allithato ossze"
upd:"Cannot enumerate available printers list"

MCannotOpenPrinter
"Не удалось открыть принтер"
"Cannot open printer"
"Nelze otevrit tiskarnu"
"Fehler beim offnen des Druckers"
"Nyomtato nem elerheto"
"Nie moge polaczyc sie z drukarka"

MCannotPrint
"Не удалось распечатать"
"Cannot print"
"Nelze tisknout"
"Fehler beim Drucken"
"Nem nyomtathato"
"Nie moge drukowac"

MDescribeFiles
l:
"Описание файла"
"Describe file"
"Popiskovy soubor"
"Beschreibung andern"
"Fajlmegjegyzes"
"Opisz plik"

MEnterDescription
"Введите описание для"
"Enter description for"
"Zadejte popisek"
"Beschreibung fur"
upd:"Irja be megjegyzeset:"
"Wprowadz opis"

MReadingDiz
l:
"Чтение описаний файлов"
"Reading file descriptions"
"Nacitam popisky souboru"
"Lese Dateibeschreibungen"
"Fajlmegjegyzesek olvasasa"
"Odczytuje opisy plikow"

MCannotUpdateDiz
"Не удалось обновить описания файлов"
"Cannot update file descriptions"
"Nelze aktualizovat popisky souboru"
"Dateibeschreibungen konnten nicht aktualisiert werden."
"A fajlmegjegyzesek nem frissithetok"
"Nie moge aktualizowac opisow plikow"

MCannotUpdateRODiz
"Файл описаний защищен от записи"
"The description file is read only"
"Popiskovy soubor ma atribut Jen pro cteni"
"Die Beschreibungsdatei ist schreibgeschutzt."
"A megjegyzesfajl csak olvashato"
"Opis jest plikiem tylko do odczytu"

MCfgDizTitle
l:
"Описания файлов"
"File descriptions"
"Popisky souboru"
"Dateibeschreibungen"
"Fajl megjegyzesfajlok"
"Opisy plikow"

MCfgDizListNames
"Имена &списков описаний, разделенные запятыми:"
"Description &list names delimited with commas:"
"Seznam pop&isovych souboru oddelenych carkami:"
"Beschreibungs&dateien, getrennt durch Komma:"
"Megjegyzes&fajlok nevei, vesszovel elvalasztva:"
"Nazwy &plikow z opisami oddzielone przecinkami:"

MCfgDizSetHidden
"Устанавливать &атрибут ""Hidden"" на новые списки описаний"
"Set ""&Hidden"" attribute to new description lists"
"Novym souborum s popisy nastavit atribut ""&Skryty"""
"Setze das '&Versteckt'-Attribut fur neu angelegte Dateien"
"Az uj megjegyzesfajl ""&rejtett"" attributumu legyen"
"Ustaw atrybut ""&Ukryty"" dla nowych plikow z opisami"

MCfgDizROUpdate
"Обновлять файл описаний с атрибутом ""Толь&ко для чтения"""
"Update &read only description file"
"Aktualizovat popisove soubory s atributem Jen pro cteni"
"Schreibgeschutzte Dateien aktualisie&ren"
"&Csak olvashato megjegyzesfajlok frissitese"
"Aktualizuj plik opisu tylko do odczytu"

MCfgDizStartPos
"&Позиция новых описаний в строке"
"&Position of new descriptions in the string"
"&Pozice novych popisu v retezci"
"&Position neuer Beschreibungen in der Zeichenkette"
"Uj megjegyzeseknel a szoveg &kezdete"
"Pozy&cja nowych opisow w linii"

MCfgDizNotUpdate
"&Не обновлять описания"
"Do &not update descriptions"
"&Neaktualizovat popisy"
"Beschreibungen &nie aktualisieren"
"N&e frissitse a megjegyzeseket"
"&Nie aktualizuj opisow"

MCfgDizUpdateIfDisplayed
"&Обновлять, если они выводятся на экран"
"Update if &displayed"
"Aktualizovat, jestlize je &zobrazen"
"Aktualisieren &wenn angezeigt"
"Frissitsen, ha meg&jelenik"
"Aktualizuj jesli &widoczne"

MCfgDizAlwaysUpdate
"&Всегда обновлять"
"&Always update"
"&Vzdy aktualizovat"
"Im&mer aktualisieren"
"&Mindig frissitsen"
"&Zawsze aktualizuj"

MReadingTitleFiles
l:
"Обновление панелей"
"Update of panels"
"Aktualizace panelu"
"Aktualisiere Panels"
"Panelek frissitese"
"Aktualizacja panelu"

MReadingFiles
"Чтение: %d файлов"
"Reading: %d files"
"Nacitam: %d souboru"
"Lese: %d Dateien"
" %d fajl olvasasa"
"Czytam: %d plikow"

MUserBreakTitle
l:
"Прекращено пользователем"
"User break"
"Preruseno uzivatelem"
"Unterbochen durch Benutzer"
"A felhasznalo megszakitotta"
"Przerwane przez uzytkownika"

MOperationNotCompleted
"Операция не завершена"
"Operation not completed"
"Operace neni dokoncena"
"Vorgang nicht abgeschlossen"
"A muvelet felbeszakadt"
"Operacja nie doprowadzona do konca"

MEditPanelModes
l:
"Режимы панели"
"Edit panel modes"
"Editovat mody panelu"
"Anzeigemodi von Panels bearbeiten"
"Panel modok szerkesztese"
"Edytuj tryby wyswietlania paneli"

MEditPanelModesBrief
l:
"&Краткий режим"
"&Brief mode"
"&Strucny mod"
"&Kurz"
"&Rovid mod"
"&Skrotowy"

MEditPanelModesMedium
"&Средний режим"
"&Medium mode"
"S&tredni mod"
"&Mittel"
"&Kozepes mod"
"S&redni"

MEditPanelModesFull
"&Полный режим"
"&Full mode"
"&Plny mod"
"&Voll"
"&Teljes mod"
"&Pelny"

MEditPanelModesWide
"&Широкий режим"
"&Wide mode"
"S&iroky mod"
"B&reitformat"
"&Szeles mod"
"S&zeroki"

MEditPanelModesDetailed
"&Детальный режим"
"Detai&led mode"
"Detai&lni mod"
"Detai&lliert"
"Res&zletes mod"
"Ze sz&czegolami"

MEditPanelModesDiz
"&Описания"
"&Descriptions mode"
"P&opiskovy mod"
"&Beschreibungen"
"&Fajlmegjegyzes mod"
"&Opisy"

MEditPanelModesLongDiz
"Д&линные описания"
"Lon&g descriptions mode"
"&Mod dlouhych popisku"
"Lan&ge Beschreibungen"
"&Hosszu megjegyzes mod"
"&Dlugie opisy"

MEditPanelModesOwners
"Вл&адельцы файлов"
"File own&ers mode"
"Mod vlastnika so&uboru"
"B&esitzer"
"T&ulajdonos mod"
"&Wlasciciele"

MEditPanelModesLinks
"Свя&зи файлов"
"Lin&ks mode"
"Lin&kovy mod"
"Dateilin&ks"
"Li&nkek mod"
"Dowiaza&nia"

MEditPanelModesAlternative
"Аль&тернативный полный режим"
"&Alternative full mode"
"&Alternativni plny mod"
"&Alternative Vollansicht"
"&Alternativ teljes mod"
"&Alternatywny"

MEditPanelModeTypes
l:
"&Типы колонок"
"Column &types"
"&Typ sloupcu"
"Spalten&typen"
"Oszlop&tipusok"
"&Typy kolumn"

MEditPanelModeWidths
"&Ширина колонок"
"Column &widths"
"Sir&ka sloupcu"
"Spalten&breiten"
"Oszlop&szelessegek"
"&Szerokosci kolumn"

MEditPanelModeStatusTypes
"Типы колонок строки ст&атуса"
"St&atus line column types"
"T&yp sloupcu stavoveho radku"
"St&atuszeile Spaltentypen"
"Allapotsor oszloptip&usok"
"Typy kolumn &linii statusu"

MEditPanelModeStatusWidths
"Ширина колонок строки стат&уса"
"Status l&ine column widths"
"Sirka slo&upcu stavoveho radku"
"Statusze&ile Spaltenbreiten"
"Allapotsor &oszlopszelessegek"
"Szerokosci kolumn l&inii statusu"

MEditPanelModeFullscreen
"&Полноэкранный режим"
"&Fullscreen view"
"&Celoobrazovkovy rezim"
"&Vollbild"
"Tel&jes kepernyos nezet"
"Widok &pelnoekranowy"

MEditPanelModeAlignExtensions
"&Выравнивать расширения файлов"
"Align file &extensions"
"Zarovnat prip&ony souboru"
"Datei&erweiterungen ausrichten"
"Fajlkiterjesztesek &igazitasa"
"W&yrownaj rozszerzenia plikow"

MEditPanelModeAlignFolderExtensions
"Выравнивать расширения пап&ок"
"Align folder e&xtensions"
"Zarovnat pripony adre&saru"
"Ordnerer&weiterungen ausrichten"
"Mappakiterjesztesek i&gazitasa"
"Wyrownaj rozszerzenia &folderow"

MEditPanelModeFoldersUpperCase
"Показывать папки &заглавными буквами"
"Show folders in &uppercase"
"Zobrazit adresare &velkymi pismeny"
"Ordner in Gro?b&uchstaben zeigen"
"Mappak NAG&YBETUVEL mutatva"
"Nazwy katalogow &WIELKIMI LITERAMI"

MEditPanelModeFilesLowerCase
"Показывать файлы ст&рочными буквами"
"Show files in &lowercase"
"Zobrazit soubory ma&lymi pismeny"
"Dateien in K&leinbuchstaben zeigen"
"Fajlok kis&betuvel mutatva"
"&Nazwy plikow malymi literami"

MEditPanelModeUpperToLowerCase
"Показывать имена файлов из заглавных букв &строчными буквами"
"Show uppercase file names in lower&case"
"Zobrazit velke znaky ve jmenech souboru jako &mala pismena"
"G&ro?geschriebene Dateinamen in Kleinbuchstaben zeigen"
"NAGYBETUS fajl&nevek kisbetuvel"
"Wyswietl NAZWY_PLIKOW &jako nazwy_plikow"

MEditPanelModeCaseSensitiveSort
"Использовать р&егистрозависимую сортировку"
"Use case &sensitive sort"
"Pouzit razeni citlive na velikost pi&smen"
"&Sortierung abhangig von Gro?-/Kleinschreibung"
"N&agy/kisbetu erzekeny rendezes"
"S&ortuj uwzgledniajac wielkosc liter"

MEditPanelReadHelp
" Нажмите F1, чтобы получить информацию по настройке "
" Read online help for instructions "
" Pro instrukce si prectete online napovedu "
" Siehe Hilfe fur Anweisungen "
" Tanacsokat a sugoban talal (F1) "
" Instrukcje zawarte sa w pomocy podrecznej "

MSetFolderInfoTitle
l:
"Файлы информации о папках"
"Folder description files"
"Soubory s popiskem adresare"
"Ordnerbeschreibungen"
"Mappa megjegyzesfajlok"
"Pliki opisu katalogu"

MSetFolderInfoNames
"Введите имена файлов, разделенные запятыми (допускаются маски)"
"Enter file names delimited with commas (wildcards are allowed)"
"Zadejte jmena souboru oddelenych carkami (znacky jsou povoleny)"
"Dateiliste, getrennt mit Komma (Jokerzeichen moglich):"
"Fajlnevek, vesszovel elvalasztva (joker is hasznalhato)"
"Nazwy plikow oddzielone przecinkami (znaki ? i * dopuszczalne)"

MScreensTitle
l:
"Экраны"
"Screens"
"Obrazovky"
"Seiten"
"Kepernyok"
"Ekrany"

MScreensPanels
"Панели"
"Panels"
"Panely"
"Panels"
"Panelek"
"Panele"

MScreensView
"Просмотр"
"View"
"Zobrazit"
"Betr."
"Nezoke"
"Podglad"

MScreensEdit
"Редактор"
"Edit"
"Editovat"
"Bearb"
"Szerkeszto"
"Edycja"

MAskApplyCommandTitle
l:
"Применить команду"
"Apply command"
"Aplikovat prikaz"
"Befehl anwenden"
"Parancs vegrehajtasa"
"Zastosuj polecenie"

MAskApplyCommand
"Введите команду для обработки выбранных файлов"
"Enter command to process selected files"
"Zadejte prikaz pro zpracovani vybranych souboru"
"Befehlszeile auf ausgewahlte Dateien anwenden:"
"Irja be a kijelolt fajlok parancsat:"
"Wprowadz polecenie do przetworzenia wybranych plikow"

MPluginConfigTitle
l:
"Конфигурация модулей"
"Plugins configuration"
"Nastaveni Pluginu"
"Konfiguration von Plugins"
"Plugin beallitasok"
"Konfiguracja pluginow"

MPluginCommandsMenuTitle
"Команды внешних модулей"
"Plugin commands"
"Prikazy pluginu"
"Pluginbefehle"
"Plugin parancsok"
"Dostepne pluginy"

MPreparingList
l:
"Создание списка файлов"
"Preparing files list"
"Pripravuji seznam souboru"
"Dateiliste wird vorbereitet"
"Fajllista elkeszitese"
"Przygotowuje liste plikow"

MLangTitle
l:
"Основной язык"
"Main language"
"Hlavni jazyk"
"Hauptsprache"
"A program nyelve"
"Jezyk programu"

MHelpLangTitle
"Язык помощи"
"Help language"
"Jazyk napovedy"
"Sprache der Hilfedatei"
"A sugo nyelve"
"Jezyk pomocy"

MDefineMacroTitle
l:
"Задание макрокоманды"
"Define macro"
"Definovat makro"
"Definiere Makro"
"Makro gyorsbillentyu"
"Zdefiniuj makro"

MDefineMacro
"Нажмите желаемую клавишу"
"Press the desired key"
"Stisknete pozadovanou klavesu"
"Tastenkombination:"
"Nyomja le a billentyut!"
"Nacisnij zadany klawisz"

MMacroReDefinedKey
"Макроклавиша '%s' уже определена."
"Macro key '%s' already defined."
"Klavesa makra '%s' jiz je definovana."
"Makro '%s' bereits definiert."
""%s" makrobillentyu foglalt"
"Skrot '%s' jest juz zdefiniowany."

MMacroDeleteAssign
"Макроклавиша '%s' не активна."
"Macro key '%s' is not active."
"Klavesa makra '%s' neni aktivni."
"Makro '%s' nicht aktiv."
""%s" makrobillentyu nem el"
"Skrot '%s' jest nieaktywny."

MMacroDeleteKey
"Макроклавиша '%s' будет удалена."
"Macro key '%s' will be removed."
"Klavesa makra '%s' bude odstranena."
"Makro '%s' wird entfernt und ersetzt:"
""%s" makrobillentyu torlodik"
"Skrot '%s' zostanie usuniety."

MMacroCommonReDefinedKey
"Общая макроклавиша '%s' уже определена."
"Common macro key '%s' already defined."
"Klavesa pro bezne makro '%s' jiz je definovana."
"Gemeinsames Makro '%s' bereits definiert."
""%s" kozos makrobill. foglalt"
"Skrot '%s' jest juz zdefiniowany."

MMacroCommonDeleteAssign
"Общая макроклавиша '%s' не активна."
"Common macro key '%s' is not active."
"Klavesa pro bezne makro '%s' neni aktivni."
"Gemeinsames Makro '%s' nicht aktiv."
""%s" kozos makrobill. nem el"
"Skrot '%s' jest nieaktywny."

MMacroCommonDeleteKey
"Общая макроклавиша '%s' будет удалена."
"Common macro key '%s' will be removed."
"Klavesa pro bezne makro '%s' bude odstranena."
"Gemeinsames Makro '%s' wird entfernt und ersetzt:"
""%s" kozos makrobill. torlodik"
"Skrot '%s' zostanie usuniety."

MMacroSequence
"Последовательность:"
"Sequence:"
"Posloupnost:"
"Sequenz:"
"Szekvencia:"
"Sekwencja:"

MMacroReDefinedKey2
"Переопределить?"
"Redefine?"
"Predefinovat?"
"Neu definieren?"
"Ujradefinialja?"
"Zdefiniowac powtornie?"

MMacroDeleteKey2
"Удалить?"
"Delete?"
"Odstranit?"
"Loschen?"
"Torli?"
"Usunac?"

MMacroDisDisabledKey
"(макроклавиша не активна)"
"(macro key is not active)"
"(klavesa makra neni aktivni)"
"(Makro inaktiv)"
"(makrobill. nem el)"
"(skrot jest nieaktywny)"

MMacroDisOverwrite
"Переопределить"
"Overwrite"
"Prepsat"
"Uberschreiben"
"Feluliras"
"Zastapic"

MMacroDisAnotherKey
"Изменить клавишу"
"Try another key"
"Zkusit jinou klavesu"
"Neue Kombination"
"Adjon meg masik billentyut!"
"Sprobuj inny klawisz"

MMacroSettingsTitle
l:
"Параметры макрокоманды для '%s'"
"Macro settings for '%s'"
"Nastaveni makra pro '%s'"
"Einstellungen fur Makro '%s'"
""%s" makro beallitasai"
"Ustawienia makra dla '%s'"

MMacroSettingsEnableOutput
"Разрешить во время &выполнения вывод на экран"
"Allo&w screen output while executing macro"
"Povolit &vystup na obrazovku dokud se provadi makro"
"Bildschirmausgabe &wahrend Makro ablauft"
"Kepernyo&kimenet a makro futasa kozben"
"&Wylacz zapis na ekran podczas wykonywania makra"

MMacroSettingsRunAfterStart
"В&ыполнять после запуска FAR"
"Execute after FAR &start"
"&Spustit po spusteni FARu"
"Ausfuhren beim &Starten von FAR"
"Vegrehajtas a FAR &inditasa utan"
"Wykonaj po &starcie FAR-a"

MMacroSettingsActivePanel
"&Активная панель"
"&Active panel"
"&Aktivni panel"
"&Aktives Panel"
"&Aktiv panel"
"Panel &aktywny"

MMacroSettingsPassivePanel
"&Пассивная панель"
"&Passive panel"
"Pa&sivni panel"
"&Passives Panel"
"Passzi&v panel"
"Panel &pasywny"

MMacroSettingsPluginPanel
"На панели пла&гина"
"P&lugin panel"
"Panel p&luginu"
"P&lugin Panel"
"Ha &plugin panel"
"Panel p&luginow"

MMacroSettingsFolders
"Выполнять для папо&к"
"Execute for &folders"
"Spustit pro ad&resare"
"Auf Ordnern aus&fuhren"
"Ha &mappa"
"Wykonaj dla &folderow"

MMacroSettingsSelectionPresent
"&Отмечены файлы"
"Se&lection present"
"E&xistujici vyber"
"Auswah&l vorhanden"
"Ha van ki&jeloles"
"Zaznaczenie &obecne"

MMacroSettingsCommandLine
"Пустая командная &строка"
"Empty &command line"
"Prazdny pri&kazovy radek"
"Leere Befehls&zeile"
"Ha &ures a parancssor"
"Pusta &linia polecen"

MMacroSettingsSelectionBlockPresent
"Отмечен б&лок"
"Selection &block present"
"Existujici blok vyber&u"
"Mar&kierter Text vorhanden"
"Ha van kijelolt &blokk"
"Obecny &blok zaznaczenia"

MMacroOutputFormatForHelpSz
l:
l:// for <!Macro:Vars!> and <!Macro:Consts!>, count formats = 1
""%s""
""%s""
""%s""
""%s""
""%s""
""%s""

MMacroOutputFormatForHelpDWord
l:// for <!Macro:Vars!> and <!Macro:Consts!>, count formats = 2
"%d / 0x%X"
"%d / 0x%X"
"%d / 0x%X"
"%d / 0x%X"
"%d / 0x%X"
"%d / 0x%X"

MMacroOutputFormatForHelpQWord
l:// for <!Macro:Vars!> and <!Macro:Consts!>, count formats = 2
"%I64d / 0x%I64X"
"%I64d / 0x%I64X"
"%I64d / 0x%I64X"
"%I64d / 0x%I64X"
"%I64d / 0x%I64X"
"%I64d / 0x%I64X"

MMacroOutputFormatForHelpDouble
l:// for <!Macro:Vars!> and <!Macro:Consts!>, count formats = 2
"%g"
"%g"
"%g"
"%g"
"%g"
"%g"

MMacroPErrUnrecognized_keyword
l:
"Неизвестное ключевое слово '%s'"
"Unrecognized keyword '%s'"
"Nezname klicove slovo '%s'"
"Unbekanntes Schlusselwort '%s'"
"Ismeretlen kulcsszo "%s""
"Nie rozpoznano slowa kluczowego '%s'"

MMacroPErrUnrecognized_function
"Неизвестная функция '%s'"
"Unrecognized function '%s'"
"Neznama funkce '%s'"
"Unbekannte Funktion '%s'"
"Ismeretlen funkcio "%s""
"Nie rozpoznano funkcji'%s'"

MMacroPErrFuncParam
"Неверное количество параметров у функции '%s'"
"Incorrect number of arguments for function '%s'"
upd:"Incorrect number of arguments for function '%s'"
upd:"Incorrect number of arguments for function '%s'"
"'%s' funkcio parametereinek szama helytelen"
upd:"Incorrect number of arguments for function '%s'"

MMacroPErrNot_expected_ELSE
"Неожиданное появление $Else"
"Unexpected $Else"
"Neocekavane $Else"
"Unerwartetes $Else"
"Varatlan $Else"
"$Else w niewlasciwym miejscu"

MMacroPErrNot_expected_END
"Неожиданное появление $End"
"Unexpected $End"
"Neocekavane $End"
"Unerwartetes $End"
"Varatlan $End"
"$End w niewlasciwym miejscu"

MMacroPErrUnexpected_EOS
"Неожиданный конец строки"
"Unexpected end of source string"
"Neocekavany konec zdrojoveho retezce"
"Unerwartetes Ende der Zeichenkette"
"Varatlanul vege a forrassztringnek"
"Nie spodziewano sie konca ciagu"

MMacroPErrExpected
"Ожидается '%s'"
"Expected '%s'"
"Ocekavane '%s'"
"Erwartet '%s'"
"Varhato "%s""
"Oczekiwano '%s'"

MMacroPErrBad_Hex_Control_Char
"Неизвестный шестнадцатеричный управляющий символ"
"Bad Hex Control Char"
"Chybny kontrolni znak Hex"
"Fehlerhaftes Hexzeichen"
"Rossz hexa vezerlokarakter"
"Bledny szesnastkowy znak sterujacy"

MMacroPErrBad_Control_Char
"Неправильный управляющий символ"
"Bad Control Char"
"Spatny kontrolni znak"
"Fehlerhaftes Kontrollzeichen"
"Rossz vezerlokarakter"
"Bledny znak sterujacy"

MMacroPErrVar_Expected
"Переменная '%s' не найдена"
"Variable Expected '%s'"
"Ocekavana promenna '%s'"
"Variable erwartet '%s'"
""%s" varhato valtozo"
"Oczekiwano zmiennej '%s'"

MMacroPErrExpr_Expected
"Ошибка синтаксиса"
"Expression Expected"
"Ocekavany vyraz"
"Ausdruck erwartet"
"Szintaktikai hiba"
"Oczekiwano wyrazenia"

MMacroPErr_ZeroLengthMacro
"Пустая макропоследовательность"
"Zero-length macro"
upd:"Zero-length macro"
upd:"Zero-length macro"
"Nulla hosszusagu makro"
upd:"Zero-length macro"

MMacroPErrorTitle
"Ошибка при разборе макроса"
"Macro parsing error"
"Macro parsing error"
"Macro parsing error"
"Makroelemzesi hiba"
"Macro parsing error"

MCannotSaveFile
l:
"Ошибка сохранения файла"
"Cannot save file"
"Nelze ulozit soubor"
"Kann Datei nicht speichern"
"A fajl nem mentheto"
"Nie moge zapisac pliku"

MTextSavedToTemp
"Отредактированный текст записан в"
"Edited text is stored in"
"Editovany text je ulozen v"
"Editierter Text ist gespeichert in"
"A szerkesztett szoveg elmentve:"
"Edytowany tekst zostal zachowany w"

MMonthJan
l:
"Янв"
"Jan"
"Led"
"Jan"
"Jan"
"Sty"

MMonthFeb
"Фев"
"Feb"
"Uno"
"Feb"
"Feb"
"Lut"

MMonthMar
"Мар"
"Mar"
"Bre"
"Mar"
"Mar"
"Mar"

MMonthApr
"Апр"
"Apr"
"Dub"
"Apr"
"Apr"
"Kwi"

MMonthMay
"Май"
"May"
"Kve"
"Mai"
"Maj"
"Maj"

MMonthJun
"Июн"
"Jun"
"Cer"
"Jun"
"Jun"
"Cze"

MMonthJul
"Июл"
"Jul"
"Cec"
"Jul"
"Jul"
"Lip"

MMonthAug
"Авг"
"Aug"
"Srp"
"Aug"
"Aug"
"Sie"

MMonthSep
"Сен"
"Sep"
"Zar"
"Sep"
"Sze"
"Wrz"

MMonthOct
"Окт"
"Oct"
"Rij"
"Okt"
"Okt"
"Paz"

MMonthNov
"Ноя"
"Nov"
"Lis"
"Nov"
"Nov"
"Lis"

MMonthDec
"Дек"
"Dec"
"Pro"
"Dez"
"Dec"
"Gru"

MPluginHotKeyTitle
l:
"Назначение горячей клавиши"
"Assign plugin hot key"
"Pridelit horkou klavesu pluginu"
"Dem Plugin eine Kurztaste zuweisen"
"Plugin gyorsbillentyu hozzarendeles"
"Przypisz klawisz skrotu do pluginu"

MPluginHotKey
"Введите горячую клавишу (букву или цифру)"
"Enter hot key (letter or digit)"
"Zadejte horkou klavesu (pismeno nebo cislici)"
"Buchstabe oder Ziffer:"
"Nyomja le a billentyut (betu vagy szam)!"
"Podaj klawisz skrotu (litera lub cyfra)"

MPluginHotKeyBottom
"F4 - задать горячую клавишу"
"F4 - set hot key"
"F4 - nastaveni horke klavesy"
"Kurztaste setzen: F4"
"F4 - gyorsbillentyu hozzarendeles"
"F4 - ustaw klawisz skrotu"

MRightCtrl
l:
"ПравыйCtrl"
"RightCtrl"
"PravyCtrl"
"StrgRechts"
"JobbCtrl"
"PrawyCtrl"

MViewerGoTo
l:
"Перейти"
"Go to"
"Jdi na"
"Gehe zu"
"Ugras"
"Idz do"

MGoToPercent
"&Процент"
"&Percent"
"&Procent"
"&Prozent"
"&Szazalekban"
"&Procent"

MGoToHex
"16-ричное &смещение"
"&Hex offset"
"&Hex offset"
"Position (&Hex)"
"&Hexaban"
"Pozycja (&szesnastkowo)"

MGoToDecimal
"10-ичное с&мещение"
"&Decimal offset"
"&Desitkovy offset"
"Position (&dezimal)"
"&Decimalisan"
"Pozycja (&dziesietnie)"

MExceptTitleFAR
l:
"Внутренняя ошибка"
"Internal error"
"Vnitrni chyba"
"Interner Fehler"
"Belso hiba"
"Blad wewnetrzny"

MExceptTitleLoad
"Ошибка загрузки плагина"
"Plugin load error"
"Chyba pri nacitani pluginu"
"Fehler beim Laden des Plugins"
"Plugin betoltesi hiba"
"Blad ladowania pluginu"

MExceptTitle
"Ошибка вызова плагина"
"Plugin call error"
"Chyba pri volani pluginu"
"Fehler beim Starten des Plugins"
"Plugin meghivasi hiba"
"Blad wywolania pluginu"

MExcTrappedException
"Исключительная ситуация:"
"Exception occurred:"
"Vyskytla se vyjimka:"
"Ausnahmefehler aufgetreten:"
"Kivetel tortent:"
"Wystapil wyjatek:"

MExcCheckOnLousys
"Передана некорректная информация из модуля:"
"Incorrect information is passed from module:"
"Z modulu byla obdrzena nekorektni informace:"
"Ungultige Informationen ubergeben durch Modul:"
"Hibas informacio jott a plugintol:"
"Blad przekazywania informacji z modulu:"

MExcStructWrongFilled
"(некорректно заполнены поля структуры <%s>)"
"(the fields of structure <%s> are wrong filled)"
"(pole struktur <%s> jsou spatne vyplnena)"
"(Felder der Struktur <%s> wurden fehlerhaft gefullt)"
"(<%s> struktura mezoi rosszul vannak kitoltve)"
"(pola struktury <%s> sa nieprawidlowo wypelnione)"

MExcStructField
"(структура <%s>, поле <%s>)"
"(structure <%s>, field <%s>)"
"(struktura <%s>, polozka <%s>)"
"(Struktur <%s>, Feld <%s>)"
"(<%s> struktura, <%s> mezo)"
"(struktura <%s>, pole <%s>)"

MExcInvalidFuncResult
"Функция <%s> вернула недопустимое значение"
"Function <%s> has returned illegal value"
"Funkce <%s> vratila nepovolenou hodnotu"
"Funktion <%s> lieferte ungultigen Ruckgabewert"
"<%s> funkcio ervenytelen erteket adott vissza"
"Funkcja <%s> zwrocila nieprawidlowa wartosc"

MExcAddress
"Адрес исключения - 0x%p, модуль:"
"Exception address: 0x%p in module:"
"Vyjimka na adrese: 0x%X v modulu:"
"Adresse des Fehlers: 0x%p in Modul:"
"Kivetel cime 0x%p, modul:"
"Adres wyjatku: 0x%p w module:"

MExcFARTerminateYes
"FAR Manager завершит работу!"
"FAR Manager will be terminated!"
"FAR Manager bude ukoncen!"
"FAR Manager wird jetzt beendet!"
"A FAR Manager kilep!"
"FAR zostanie wylaczony!"

MExcUnloadYes
"Плагин будет выгружен!"
"The plugin will be Unloaded!"
"Plugin bude vyrazen!"
"Das Plugin wird jetzt entladen!"
"A plugin torlodik a memoriabol!"
"Plugin zostanie usuniety z pamieci!"

MExcRAccess
"\"Нарушение доступа (чтение из 0x%p)\""
"\"Access violation (read from 0x%p)\""
"\"Neplatna adresa (cteni z 0x%p)\""
"\"Zugriffsverletzung (Lesen von 0x%p)\""
"\"Hozzaferesi jogsertes (olvasas 0x%p cimrol)\""
"\"Blad dostepu (odczyt z 0x%p)\""

MExcWAccess
"\"Нарушение доступа (запись в 0x%p)\""
"\"Access violation (write to 0x%p)\""
"\"Neplatna adresa (zapis na 0x%p)\""
"\"Zugriffsverletzung (Schreiben nach 0x%p)\""
"\"Hozzaferesi jogsertes (iras 0x%p cimre)\""
"\"Blad dostepu (zapis do 0x%p)\""

MExcEAccess
"\"Нарушение доступа (исполнение кода из 0x%p)\""
"\"Access violation (execute at 0x%p)\""
"\"Neplatna adresa (spusteni na 0x%p)\""
"\"Zugriffsverletzung (Ausfuhren bei 0x%p)\""
"\"Hozzaferesi jogsertes (vegrehajtas 0x%p cimen)\""
"\"Blad dostepu (wykonanie w 0x%p)\""

MExcOutOfBounds
"\"Попытка доступа к элементу за границами массива\""
"\"Array out of bounds\""
"\"Pole mimo hranice\""
"\"Arrayuberlauf\""
"\"A tomb hatarait meghaladta\""
"\"Przekroczenie granic tabeli\""

MExcDivideByZero
"\"Деление на нуль\""
"\"Divide by zero\""
"\"Deleni nulou\""
"\"Division durch Null\""
"\"Nullaval osztas\""
"\"Dzielenie przez zero\""

MExcStackOverflow
"\"Переполнение стека\""
"\"Stack Overflow\""
"\"Preteceni zasobniku\""
"\"Stackuberlauf\""
"\"Verem tulcsordulas\""
"\"Przepelnienie stosu\""

MExcBreakPoint
"\"Точка останова\""
"\"Breakpoint exception\""
"\"Vyjimka preruseni\""
"\"Breakpoint exception\""
"\"Toresponti kivetel\""
"\"Wyjatek punktu przerwania\""

MExcFloatDivideByZero
"\"Деление на нуль при операции с плавающей точкой\""
"\"Floating-point divide by zero\""
"\"Deleni nulou v pohyblive carce\""
"\"Flie?komma-Division durch Null\""
"\"Lebegopontos szam osztasa nullaval\""
"\"Blad zmiennoprzecinkowego dzielenia przez zero\""

MExcFloatOverflow
"\"Переполнение при операции с плавающей точкой\""
"\"Floating point operation overflow\""
"\"Preteceni pri operaci v pohyblive carce\""
"\"Flie?komma-Operation verursachte Uberlauf\""
"\"Lebegopontos muvelet tulcsordulas\""
"\"Przepelnienie przy operacji zmiennnoprzecinkowej\""

MExcFloatStackOverflow
"\"Стек регистров сопроцессора полон или пуст\""
"\"Floating point stack empty or full\""
"\"Prazdny nebo plny zasobnik v pohyblive carce\""
"\"Flie?komma-Stack leer bzw. voll\""
"\"Lebegopont verem ures vagy megtelt\""
"\"Stos operacji zmiennoprzecinkowych pusty lub pelny\""

MExcFloatUnderflow
"\"Потеря точности при операции с плавающей точкой\""
"\"Floating point operation underflow\""
"\"Podteceni pri operaci v pohyblive carce\""
"\"Flie?komma-Operation verursachte Underflow\""
"\"Lebegopontos muvelet alulcsordulas\""
"\"Blad niedomiaru przy operacji zmiennoprzecinkowej\""

MExcBadInstruction
"\"Недопустимая инструкция\""
"\"Illegal instruction\""
"\"Neplatna instrukce\""
"\"Ungultige Anweisung\""
"\"Ervenytelen utasitas\""
"\"Bledna instrukcja\""

MExcDatatypeMisalignment
"\"Попытка доступа к невыравненным данным\""
"\"Alignment fault\""
"\"Chyba zarovnani\""
"\"Fehler bei Datenausrichtung\""
"\"Adattipus illesztesi hiba\""
"\"Blad ustawienia\""

MExcUnknown
"\"Неизвестное исключение\""
"\"Unknown exception\""
"\"Neznama vyjimka\""
"\"Unbekannte Ausnahme\""
"\"Ismeretlen kivetel\""
"\"Nieznany wyjatek\""

MExcDebugger
"Отладчик"
"Debugger"
"Debugger"
"Debugger"
"Debugger"
"Debugger"

MNetUserName
l:
"Имя пользователя"
"User name"
"Jmeno uzivatele"
"Benutzername"
"Felhasznaloi nev"
"Nazwa uzytkownika"

MNetUserPassword
"Пароль пользователя"
"User password"
"Heslo uzivatele"
"Benutzerpasswort"
"Felhasznaloi jelszo"
"Haslo uzytkownika"

MReadFolderError
l:
"Не удается прочесть содержимое папки"
"Cannot read folder contents"
"Nelze nacist obsah adresare"
"Kann Ordnerinhalt nicht lesen"
"A mappa tartalma nem olvashato"
"Nie udalo sie odczytac zawartosci folderu"

MPlgBadVers
l:
"Этот модуль требует FAR более высокой версии"
"This plugin requires higher FAR version"
"Tento plugin vyzaduje vyssi verzi FARu"
"Das Plugin benotigt eine aktuellere Version von FAR"
"A pluginhez ujabb FAR verzio kell"
"Do uruchomienia pluginu wymagana jest wyzsza wersja FAR-a"

MPlgRequired
"Требуется версия FAR - %d.%d.%d."
"Required FAR version is %d.%d.%d."
"Pozadovana verze FARu je %d.%d.%d."
"Benotigte FAR-Version ist %d.%d.%d."
"A szukseges FAR verzio: %d.%d.%d."
"Wymagana wersja FAR-a to %d.%d.%d."

MPlgRequired2
"Текущая версия FAR - %d.%d.%d."
"Current FAR version is %d.%d.%d"
"Nynejsi verze FARu je %d.%d.%d"
"Aktuelle FAR-Version ist %d.%d.%d"
"A jelenlegi FAR verzio: %d.%d.%d."
"Biezaca wersja FAR-a: %d.%d.%d"

MPlgLoadPluginError
"Ошибка при загрузке плагина"
"Error loading plugin module"
"Chyba pri nahravani zasuvneho modulu"
"Fehler beim Laden des Pluginmoduls"
"Plugin betoltesi hiba"
"Blad ladowania modulu plugina"

MBuffSizeTooSmall_1
l:
"Буфер, размещенный под имя файла слишком мал."
"Buffer allocated for file name is too small."
"Buffer alokovany pro jmeno souboru je prilis maly."
"Reservierter Puffer fur Dateiname ist zu klein."
"A fajlnev puffere tul kicsi."
"Bufor zaalokowany dla nazwy pliku jest zbyt maly."

MBuffSizeTooSmall_2
"Требуется %d байт, а имеется только %d"
"%d bytes are required, but only %d bytes were allocated."
"Pozadovano %d bytu, ale alokovano pouze %d."
"%d Bytes werden benotigt aber nur %d Bytes wurden reserviert."
"%d bajt kell, de csak %d van lefoglalva."
"Wymagano %d bajtow, a zaalokowano tylko %d."

MCheckBox2State
l:
"?"
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
"A mezo"
"Pole"

MEditInputSize2
"будет уменьшена до %d байт."
"will be truncated to %d bytes."
"bude oseknuto na %d bytu."
"wird gekurzt auf %d Bytes."
" %d bajtra rovidul."
"bedzie obciete do %d bajtow."

MHelpTitle
l:
"Помощь"
"Help"
"Napoveda"
"Hilfe"
"Sugo"
"Pomoc"

MHelpActivatorURL
"Эта ссылка запускает внешнее приложение:"
"This reference starts the external application:"
"Tento odkaz spousti externi aplikaci:"
"Diese Referenz startet folgendes externes Programm:"
"A hivatkozas altal inditott program:"
"To wywolanie uruchomi aplikacje zewnetrzna:"

MHelpActivatorFormat
"с параметром:"
"with parameter:"
"s parametrem:"
"mit Parameter:"
"Parameterei:"
"z parametrem:"

MHelpActivatorQ
"Желаете запустить?"
"Do you wish to start it?"
"Prejete si ji spustit?"
"Wollen Sie jetzt starten?"
"El akarja inditani?"
"Czy chcesz ja uruchomic?"

MCannotOpenHelp
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevrit soubor"
"Kann Datei nicht offnen"
"A fajl nem nyithato meg"
"Nie mozna otworzyc pliku"

MHelpTopicNotFound
"Не найден запрошенный раздел помощи:"
"Requested help topic not found:"
"pozadovane tema napovedy nebylo nalezeno"
"Angefordertes Hilfethema wurde nicht gefunden:"
"A kivant sugo temakor nem talalhato:"
"Nie znaleziono tematu pomocy:"

MPluginsHelpTitle
l:
"Внешние модули"
"Plugins help"
"Napoveda Pluginu"
"Pluginhilfe"
"Pluginek sugoi"
"Pomoc dla pluginow"

MDocumentsHelpTitle
"Документы"
"Documents help"
"Napoveda Dokumentu"
"Dokumentenhilfe"
"Dokumentumok sugoi"
"Pomoc dla dokumentow"

MHelpSearchTitle
l:
"Поиск"
"Search"
"Hledani"
"Suchen"
"Kereses"
"Szukaj"

MHelpSearchingFor
"Поиск для"
"Searching for"
"Hledani"
"Suche nach"
"Kereses:"
"Znajdz"

MHelpSearchCannotFind
"Строка не найдена"
"Could not find the string"
"Nelze najit retezec"
"Konnte Zeichenkette nicht finden"
"A szoveg nem talalhato:"
"Nie moge odnalezc ciagu znakow"

MHelpF1
l:
l:// Help KeyBar F1-12
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Sugo"
"Pomoc"

MHelpF2
""
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
""

MHelpF4
""
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
"Nagyit"
"Powieksz"

MHelpF6
""
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
"Szukaj"

MHelpF8
""
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
""

MHelpF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilep"
"Koniec"

MHelpF11
""
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
""

MHelpShiftF1
l:
l:// Help KeyBar Shift-F1-12
"Содерж"
"Index"
"Index"
"Index"
"Tartlm"
"Indeks"

MHelpShiftF2
"Плагин"
"Plugin"
"Plugin"
"Plugin"
"PlgSug"
"Plugin"

MHelpShiftF3
"Докум"
"Docums"
"Dokume"
"Dokume"
"DokSug"
"Dokumenty"

MHelpShiftF4
""
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
""

MHelpShiftF6
""
""
""
""
""
""

MHelpShiftF7
"Дальше"
"Next"
"Dalsi"
"Nachst"
"Tovabb"
"Nast."

MHelpShiftF8
""
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
""

MHelpShiftF10
""
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
""

MHelpShiftF12
""
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
"Predch"
"Letzt"
"Vissza"
"Poprz."

MHelpAltF2
""
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
""

MHelpAltF4
""
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
""

MHelpAltF6
""
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
""

MHelpAltF8
""
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
"Video"

MHelpAltF10
""
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
""

MHelpAltF12
""
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
""

MHelpCtrlF2
""
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
""

MHelpCtrlF4
""
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
""

MHelpCtrlF6
""
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
""

MHelpCtrlF8
""
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
""

MHelpCtrlF10
""
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
""

MHelpCtrlF12
""
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
""

MHelpCtrlShiftF2
""
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
""

MHelpCtrlShiftF4
""
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
""

MHelpCtrlShiftF6
""
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
""

MHelpCtrlShiftF8
""
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
""

MHelpCtrlShiftF10
""
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
""

MHelpCtrlShiftF12
""
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
""

MHelpCtrlAltF2
""
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
""

MHelpCtrlAltF4
""
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
""

MHelpCtrlAltF6
""
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
""

MHelpCtrlAltF8
""
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
""

MHelpCtrlAltF10
""
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
""

MHelpCtrlAltF12
""
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
""

MHelpAltShiftF2
""
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
""

MHelpAltShiftF4
""
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
""

MHelpAltShiftF6
""
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
""

MHelpAltShiftF8
""
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
""

MHelpAltShiftF10
""
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
""

MHelpAltShiftF12
""
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
""

MHelpCtrlAltShiftF2
""
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
""

MHelpCtrlAltShiftF4
""
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
""

MHelpCtrlAltShiftF6
""
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
""

MHelpCtrlAltShiftF8
""
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
""

MHelpCtrlAltShiftF10
""
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
""

MHelpCtrlAltShiftF12
""
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
"Sugo"
"Pomoc"

MInfoF2
"Сверн"
"Wrap"
"Zalam"
"Umbr."
"SorTor"
"Zawin"

MInfoF3
"СмОпис"
"VieDiz"
"Zobraz"
"BetDiz"
"MjMnez"
"VieDiz"

MInfoF4
"РедОпи"
"EdtDiz"
"Edit"
"BeaDiz"
"MjSzrk"
"EdtDiz"

MInfoF5
""
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
""

MInfoF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"
"Search"

MInfoF8
"ANSI"
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
"ConfMn"

MInfoF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilep"
"Koniec"

MInfoF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MInfoF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Keprny"
"Ekran"

MInfoShiftF1
l:
l:// InfoPanel KeyBar Shift-F1-F12
""
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
"SzoTor"
"ZawijS"

MInfoShiftF3
""
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
""

MInfoShiftF5
""
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
""

MInfoShiftF7
"Дальше"
"Next"
"Dalsi"
"Nachst"
"TovKer"
"Nast."

MInfoShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kodlap"
"StrKod"

MInfoShiftF9
"Сохран"
"Save"
"Ulozit"
"Speich"
"Mentes"
"Zapisz"

MInfoShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"
"UtsMnu"
"Ostat."

MInfoShiftF11
""
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
""

MInfoAltF1
l:
l:// InfoPanel KeyBar Alt-F1-F12
"Левая"
"Left"
"Levy"
"Links"
"Bal"
"Lewy"

MInfoAltF2
"Правая"
"Right"
"Pravy"
"Rechts"
"Jobb"
"Prawy"

MInfoAltF3
""
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
""

MInfoAltF5
""
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
""

MInfoAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"
"Znajdz"

MInfoAltF8
"Строка"
"Goto"
"Jit na"
"GeheZu"
"Ugras"
"IdzDo"

MInfoAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"

MInfoAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"
"Drzewo"

MInfoAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NezElo"
"Historia"

MInfoAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"
"MapElo"
"FoldHs"

MInfoCtrlF1
l:
l:// InfoPanel KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levy"
"Links"
"Bal"
"Lewy"

MInfoCtrlF2
"Правая"
"Right"
"Pravy"
"Rechts"
"Jobb"
"Prawy"

MInfoCtrlF3
""
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
""

MInfoCtrlF5
""
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
""

MInfoCtrlF7
""
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
""

MInfoCtrlF9
""
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
""

MInfoCtrlF11
""
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
""

MInfoCtrlShiftF1
l:
l:// InfoPanel KeyBar CtrlShiftF1-12
""
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
""

MInfoCtrlShiftF3
""
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
""

MInfoCtrlShiftF5
""
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
""

MInfoCtrlShiftF7
""
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
""

MInfoCtrlShiftF9
""
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
""

MInfoCtrlShiftF11
""
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
""

MInfoCtrlAltF1
l:
l:// InfoPanel KeyBar CtrlAltF1-12
""
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
""

MInfoCtrlAltF3
""
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
""

MInfoCtrlAltF5
""
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
""

MInfoCtrlAltF7
""
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
""

MInfoCtrlAltF9
""
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
""

MInfoCtrlAltF11
""
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
""

MInfoAltShiftF1
l:
l:// InfoPanel KeyBar AltShiftF1-12
""
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
""

MInfoAltShiftF3
""
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
""

MInfoAltShiftF5
""
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
""

MInfoAltShiftF7
""
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
""

MInfoAltShiftF9
""
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
""

MInfoAltShiftF11
""
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
""

MInfoCtrlAltShiftF1
l:
l:// InfoPanel KeyBar CtrlAltShiftF1-12
""
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
""

MInfoCtrlAltShiftF3
""
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
""

MInfoCtrlAltShiftF5
""
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
""

MInfoCtrlAltShiftF7
""
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
""

MInfoCtrlAltShiftF9
""
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
""

MInfoCtrlAltShiftF11
""
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
""

MQViewF1
l:
l:// QView KeyBar F1-F12
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Sugo"
"Pomoc"

MQViewF2
"Сверн"
"Wrap"
"Zalam"
"Umbr."
"SorTor"
"Zawin"

MQViewF3
"Просм"
"View"
"Zobraz"
"Betr."
"Megnez"
"Zobacz"

MQViewF4
"Код"
"Hex"
"Hex"
"Hex"
"Hexa"
"Hex"

MQViewF5
""
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
""

MQViewF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"
"Szukaj"

MQViewF8
"ANSI"
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
"ConfMn"

MQViewF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilep"
"Koniec"

MQViewF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MQViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Keprny"
"Ekran"

MQViewShiftF1
l:
l:// QView KeyBar Shift-F1-F12
""
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
"SzoTor"
"WWrap"

MQViewShiftF3
""
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
""

MQViewShiftF5
""
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
""

MQViewShiftF7
"Дальше"
"Next"
"Dalsi"
"Nachst"
"TovKer"
"Nast."

MQViewShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kodlap"
"StrKod"

MQViewShiftF9
"Сохран"
"Save"
"Ulozit"
"Speich"
"Mentes"
"Zapisz"

MQViewShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"
"UtsMnu"
"Ostat."

MQViewShiftF11
""
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
""

MQViewAltF1
l:
l:// QView KeyBar Alt-F1-F12
"Левая"
"Left"
"Levy"
"Links"
"Bal"
"Lewy"

MQViewAltF2
"Правая"
"Right"
"Pravy"
"Rechts"
"Jobb"
"Prawy"

MQViewAltF3
""
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
""

MQViewAltF5
""
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
""

MQViewAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"
"Znajdz"

MQViewAltF8
"Строка"
"Goto"
"Jit na"
"GeheZu"
"Ugras"
"IdzDo"

MQViewAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"

MQViewAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"
"Drzewo"

MQViewAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NezElo"
"Historia"

MQViewAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"
"MapElo"
"FoldHs"

MQViewCtrlF1
l:
l:// QView KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levy"
"Links"
"Bal"
"Lewy"

MQViewCtrlF2
"Правая"
"Right"
"Pravy"
"Rechts"
"Jobb"
"Prawy"

MQViewCtrlF3
""
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
""

MQViewCtrlF5
""
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
""

MQViewCtrlF7
""
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
""

MQViewCtrlF9
""
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
""

MQViewCtrlF11
""
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
""

MQViewCtrlShiftF1
l:
l:// QView KeyBar CtrlShiftF1-12
""
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
""

MQViewCtrlShiftF3
""
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
""

MQViewCtrlShiftF5
""
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
""

MQViewCtrlShiftF7
""
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
""

MQViewCtrlShiftF9
""
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
""

MQViewCtrlShiftF11
""
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
""

MQViewCtrlAltF1
l:
l:// QView KeyBar CtrlAltF1-12
""
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
""

MQViewCtrlAltF3
""
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
""

MQViewCtrlAltF5
""
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
""

MQViewCtrlAltF7
""
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
""

MQViewCtrlAltF9
""
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
""

MQViewCtrlAltF11
""
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
""

MQViewAltShiftF1
l:
l:// QView KeyBar AltShiftF1-12
""
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
""

MQViewAltShiftF3
""
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
""

MQViewAltShiftF5
""
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
""

MQViewAltShiftF7
""
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
""

MQViewAltShiftF9
""
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
""

MQViewAltShiftF11
""
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
""

MQViewCtrlAltShiftF1
l:
l:// QView KeyBar CtrlAltShiftF1-12
""
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
""

MQViewCtrlAltShiftF3
""
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
""

MQViewCtrlAltShiftF5
""
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
""

MQViewCtrlAltShiftF7
""
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
""

MQViewCtrlAltShiftF9
""
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
""

MQViewCtrlAltShiftF11
""
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
""

MKBTreeF1
l:
l:// Tree KeyBar F1-F12
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Sugo"
"Pomoc"

MKBTreeF2
"ПользМ"
"UserMn"
"UzivMn"
"BenuMn"
"FelhMn"
"UserMn"

MKBTreeF3
""
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
"Atryb."

MKBTreeF5
"Копир"
"Copy"
"Kopir."
"Kopier"
"Masol"
"Kopiuj"

MKBTreeF6
"Перен"
"RenMov"
"PrjPrs"
"RenMov"
"AtnMoz"
"Zamien"

MKBTreeF7
"Папка"
"MkFold"
"VytAdr"
"VerzEr"
"UjMapp"
"NowyFldr"

MKBTreeF8
"Удален"
"Delete"
"Smazat"
"Losch"
"Torles"
"Usun"

MKBTreeF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"
"KonfMn"
"KonfMenu"

MKBTreeF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilep"
"Koniec"

MKBTreeF11
"Модули"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"

MKBTreeF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Keprny"
"Ekran"

MKBTreeShiftF1
l:
l:// Tree KeyBar Shift-F1-F12
""
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
""

MKBTreeShiftF3
""
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
""

MKBTreeShiftF5
"Копир"
"Copy"
"Kopir."
"Kopier"
"Masol"
"Kopiuj"

MKBTreeShiftF6
"Перен"
"Rename"
"Prejm."
"Umben"
"AtnMoz"
"Zamien"

MKBTreeShiftF7
""
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
""

MKBTreeShiftF9
"Сохран"
"Save"
"Ulozit"
"Speich"
"Mentes"
"Zapisz"

MKBTreeShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"
"UtsMnu"
"Ostat."

MKBTreeShiftF11
"Группы"
"Group"
"Skupin"
"Gruppe"
"Csoprt"
"Grupa"

MKBTreeShiftF12
"Выбран"
"SelUp"
"VybPrv"
"AuswOb"
"KijFel"
"SelUp"

MKBTreeAltF1
l:
l:// Tree KeyBar Alt-F1-F12
"Левая"
"Left"
"Levy"
"Links"
"Bal"
"Lewy"

MKBTreeAltF2
"Правая"
"Right"
"Pravy"
"Rechts"
"Jobb"
"Prawy"

MKBTreeAltF3
""
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
""

MKBTreeAltF5
""
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
""

MKBTreeAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"
"Znajdz"

MKBTreeAltF8
"Истор"
"Histry"
"Histor"
"Histor"
"ParElo"
"Historia"

MKBTreeAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"

MKBTreeAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"
"Drzewo"

MKBTreeAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NezElo"
"Historia"

MKBTreeAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"
"MapElo"
"FoldHs"

MKBTreeCtrlF1
l:
l:// Tree KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levy"
"Links"
"Bal"
"Lewy"

MKBTreeCtrlF2
"Правая"
"Right"
"Pravy"
"Rechts"
"Jobb"
"Prawy"

MKBTreeCtrlF3
""
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
""

MKBTreeCtrlF5
""
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
""

MKBTreeCtrlF7
""
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
""

MKBTreeCtrlF9
""
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
""

MKBTreeCtrlF11
""
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
""

MKBTreeCtrlShiftF1
l:
l:// Tree KeyBar CtrlShiftF1-12
""
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
""

MKBTreeCtrlShiftF3
""
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
""

MKBTreeCtrlShiftF5
""
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
""

MKBTreeCtrlShiftF7
""
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
""

MKBTreeCtrlShiftF9
""
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
""

MKBTreeCtrlShiftF11
""
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
""

MKBTreeCtrlAltF1
l:
l:// Tree KeyBar CtrlAltF1-12
""
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
""

MKBTreeCtrlAltF3
""
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
""

MKBTreeCtrlAltF5
""
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
""

MKBTreeCtrlAltF7
""
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
""

MKBTreeCtrlAltF9
""
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
""

MKBTreeCtrlAltF11
""
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
""

MKBTreeAltShiftF1
l:
l:// Tree KeyBar AltShiftF1-12
""
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
""

MKBTreeAltShiftF3
""
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
""

MKBTreeAltShiftF5
""
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
""

MKBTreeAltShiftF7
""
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
""

MKBTreeAltShiftF9
""
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
""

MKBTreeAltShiftF11
""
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
""

MKBTreeCtrlAltShiftF1
l:
l:// Tree KeyBar CtrlAltShiftF1-12
""
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
""

MKBTreeCtrlAltShiftF3
""
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
""

MKBTreeCtrlAltShiftF5
""
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
""

MKBTreeCtrlAltShiftF7
""
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
""

MKBTreeCtrlAltShiftF9
""
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
""

MKBTreeCtrlAltShiftF11
""
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
""

MCopyTimeInfo
l:
"Время: %8.8s    Осталось: %8.8s    %8.8sб/с"
"Time: %8.8s    Remaining: %8.8s    %8.8sb/s"
"Cas: %8.8s      Zbyva: %8.8s      %8.8sb/s"
"Zeit: %8.8s   Verbleibend: %8.8s   %8.8sb/s"
"Eltelt: %8.8s    Maradt: %8.8s    %8.8sb/s"
"Czas: %8.8s    Pozostalo: %8.8s    %8.8sb/s"

MKeyESCWasPressed
l:
"Действие было прервано"
"Operation has been interrupted"
"Operace byla prerusena"
"Vorgang wurde unterbrochen"
"A muveletet megszakitotta"
"Operacja zostala przerwana"

MDoYouWantToStopWork
"Вы действительно хотите отменить действие?"
"Do you really want to cancel it?"
"Opravdu chcete operaci stornovat?"
"Wollen Sie den Vorgang wirklich abbrechen?"
"Valoban le akarja allitani?"
"Czy naprawde chcesz ja anulowac?"

MDoYouWantToStopWork2
"Продолжить выполнение?"
"Continue work? "
"Pokracovat v praci?"
"Vorgang fortsetzen? "
"Folytatja?"
"Kontynuowac? "

MCheckingFileInPlugin
l:
"Файл проверяется в плагине"
"The file is being checked by the plugin"
"Soubor je prave kontrolovan pluginem"
"Datei wird von Plugin uberpruft"
"A fajlt ez a plugin hasznalja:"
"Plugin sprawdza plik"

MDialogType
l:
"Диалог"
"Dialog"
"Dialog"
"Dialog"
"Parbeszed"
"Dialog"

MHelpType
"Помощь"
"Help"
"Napoveda"
"Hilfe"
"Sugo"
"Pomoc"

MFolderTreeType
"ПоискКаталогов"
"FolderTree"
"StromAdresare"
"Ordnerbaum"
"MappaFa"
"Drzewo folderow"

MVMenuType
"Меню"
"Menu"
"Menu"
"Menu"
"Menu"
"Menu"

MIncorrectMask
l:
"Некорректная маска файлов!"
"File-mask string contains errors!"
"Retezec masky souboru obsahuje chyby!"
"Zeichenkette mit Dateimaske enthalt Fehler!"
"A fajlmaszk hibas!"
"Maska pliku zawiera bledy!"

MPanelBracketsForLongName
l:
"{}"
"{}"
"{}"
"{}"
"{}"
"{}"

MComspecNotFound
l:
"Переменная окружения %COMSPEC% не определена!"
"Environment variable %COMSPEC% not defined!"
"Promenna prostredi %COMSPEC% neni definovana!"
"Umgebungsvariable %COMSPEC% nicht definiert!"
"A %COMSPEC% kornyezeti valtozo nincs definialva!"
"Nie zdefiniowano zmiennej srodowiskowej %COMSPEC%!"

MExecuteErrorMessage
"'%s' не является внутренней или внешней командой, исполняемой программой или пакетным файлом.\n"
"'%s' is not recognized as an internal or external command, operable program or batch file.\n"
"'%s' nebylo nalezeno jako vnirni nebo externi prikaz, spustitelna aplikace nebo davkovy soubor.\n"
"'%s' nicht erkannt als interner oder externer Befehl, Programm oder Stapeldatei.\n"
""%s" nem azonithato kulso vagy belso parancskent, futtathato programkent vagy batch fajlkent.\n"
"Nie rozpoznano '%s' jako polecenia, programu ani skryptu.\n"

MOpenPluginCannotOpenFile
l:
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevrit soubor"
"Kann Datei nicht offnen"
"A fajl nem nyithato meg"
"Nie mozna otworzyc pliku"

MFileFilterTitle
l:
"Фильтр"
"Filter"
"Filtr"
"Filter"
"Felhasznaloi szuro"
"Filtr wyszukiwania"

MFileHilightTitle
"Раскраска файлов"
"Files highlighting"
"Zvyraznovani souboru"
"Farbmarkierungen"
"Fajlkiemeles"
"Zaznaczanie plikow"

MFileFilterName
"Имя &фильтра:"
"Filter &name:"
"Jme&no filtru:"
"Filter&name:"
"Szuro &neve:"
"Nazwa &filtra:"

MFileFilterMatchMask
"&Маска:"
"&Mask:"
"&Maska"
"&Maske:"
"&Maszk:"
"&Maska:"

MFileFilterSize
"Разм&ер:"
"Si&ze:"
"Vel&ikost"
"G&ro?e:"
"M&eret:"
"Ro&zmiar:"

MFileFilterSizeFromSign
">="
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
"<="

MFileFilterDate
"&Дата/Время:"
"Da&te/Time:"
"Dat&um/Cas:"
"Da&tum/Zeit:"
"&Datum/Ido:"
"Da&ta/Czas:"

MFileFilterModified
"&модификации"
"&modification"
"&modifikace"
"&Modifikation"
"&Modositas"
"&modyfikacji"

MFileFilterCreated
"&создания"
"&creation"
"&vytvoreni"
"E&rstellung"
"&Letrehozas"
"&utworzenia"

MFileFilterOpened
"&доступа"
"&access"
"&pristupu"
"Z&ugriff"
"&Hozzaferes"
"&dostepu"

MFileFilterDateRelative
"Относительна&я"
"Relat&ive"
"Relati&vni"
"Relat&iv"
"Relat&iv"
"Relat&ive"

MFileFilterDateAfterSign
">="
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
"<="

MFileFilterCurrent
"Теку&щая"
"C&urrent"
"Aktua&lni"
"Akt&uell"
"&Jelenlegi"
"&Biezacy"

MFileFilterBlank
"С&брос"
"B&lank"
"Praz&dny"
"&Leer"
"&Ures"
"&Wyczysc"

MFileFilterAttr
"Атрибут&ы"
"Attri&butes"
"Attri&buty"
"Attri&bute"
"Attri&butumok"
"&Atrybuty"

MFileFilterAttrR
"&Только для чтения"
"&Read only"
"Jen pro ct&eni"
"Sch&reibschutz"
"&Csak olvashato"
"&Do odczytu"

MFileFilterAttrA
"&Архивный"
"&Archive"
"Arc&hivovat"
"&Archiv"
"&Archiv"
"&Archiwalny"

MFileFilterAttrH
"&Скрытый"
"&Hidden"
"Skry&ty"
"&Versteckt"
"&Rejtett"
"&Ukryty"

MFileFilterAttrS
"С&истемный"
"&System"
"Systemo&vy"
"&System"
"Re&ndszer"
"&Systemowy"

MFileFilterAttrC
"С&жатый"
"&Compressed"
"Kompri&movany"
"&Komprimiert"
"&Tomoritett"
"S&kompresowany"

MFileFilterAttrE
"&Зашифрованный"
"&Encrypted"
"Si&frovany"
"V&erschlusselt"
"T&itkositott"
"&Zaszyfrowany"

MFileFilterAttrD
"&Каталог"
"&Directory"
"Adr&esar"
"Ver&zeichnis"
"Map&pa"
"&Katalog"

MFileFilterAttrNI
"&Неиндексируемый"
"Not inde&xed"
"Neinde&xovany"
"Nicht in&diziert"
"Nem inde&xelt"
"Nie z&indeksowany"

MFileFilterAttrSparse
"&Разреженный"
"S&parse"
"Rid&ky"
"Reserve"
"Ritk&itott"
"S&parse"

MFileFilterAttrT
"&Временный"
"Temporar&y"
"Doca&sny"
"Temporar"
"Atm&eneti"
"&Tymczasowy"

MFileFilterAttrReparse
"Симво&л. ссылка"
"Symbolic lin&k"
"Sybolicky li&nk"
"Symbolischer Lin&k"
"S&zimbolikus link"
"Link &symboliczny"

MFileFilterAttrOffline
"Автономны&й"
"O&ffline"
"O&ffline"
"O&ffline"
"O&ffline"
"O&ffline"

MFileFilterAttrVirtual
"Вирт&уальный"
"&Virtual"
"Virtualni"
"&Virtuell"
"&Virtualis"
"&Wirtualny"

MFileFilterReset
"Очистит&ь"
"Reset"
"Reset"
"Rucksetzen"
"Reset"
"Wy&czysc"

MFileFilterCancel
"Отмена"
"Cancel"
"Storno"
"Abbruch"
"Megsem"
"&Anuluj"

MFileFilterMakeTransparent
"Выставить прозрачность"
"Make transparent"
"Zpruhlednit"
"Transparent"
"Legyen atlatszo"
"Ustaw jako przezroczysty"

MBadFileSizeFormat
"Неправильно заполнено поле размера!"
"File size field is incorrectly filled!"
"Velikost souboru neobsahuje spravnou hodnotu!"
"Angabe der Dateigro?e ist fehlerhaft!"
"A fajlmeret mezo rosszul van kitoltve!"
"Rozmiar pliku jest niepoprawny!"

#Must be the last
MNewFileName
l:
"?Новый файл?"
"?New File?"
"?Novy soubor?"
"?Neue Datei?"
"?Uj fajl?"
"?Nowy plik?"
