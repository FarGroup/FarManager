m4_include(`farversion.m4')m4_dnl
#hpp file name
bootstrap\lang.inc

#number of languages
8

#language file name, language name, language description
FarRus.lng Russian "Russian (Русский)"
FarEng.lng English "English"
FarCze.lng Czech "Czech (Čeština)"
FarGer.lng German "German (Deutsch)"
FarHun.lng Hungarian "Hungarian (Magyar)"
FarPol.lng Polish "Polish (Polski)"
FarSpa.lng Spanish "Spanish (Español)"
FarSky.lng Slovak "Slovak (Slovenčina)"

#enum name
enum:LNGID:int

#head of the hpp file
#hhead:
#hhead:

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
"Si"
"Áno"

MNo
"Нет"
"No"
"Ne"
"Nein"
"Nem"
"Nie"
"No"
"Nie"

MOk
"OK"
"OK"
"Ok"
"OK"
"OK"
"OK"
"Aceptar"
"OK"

MHYes
l:
"&Да"
"&Yes"
"&Ano"
"&Ja"
"I&gen"
"&Tak"
"&Si"
"&Áno"

MHNo
"&Нет"
"&No"
"&Ne"
"&Nein"
"Ne&m"
"&Nie"
"&No"
"&Nie"

MHOk
"&OK"
"&OK"
"&Ok"
"&OK"
"&OK"
"&OK"
"&Aceptar"
"&OK"

MCancel
l:
"Отмена"
"Cancel"
"Storno"
"Abbrechen"
"Mégsem"
"Anuluj"
"Cancelar"
"Storno"

MRetry
"Повторить"
"Retry"
"Znovu"
"Wiederholen"
"Újra"
"Ponów"
"Reiterar"
"Znova"

MSkip
"Пропустить"
"Skip"
"Přeskočit"
"Überspringen"
"Kihagy"
"Omiń"
"Omitir"
"Preskočiť"

MAbort
"Прервать"
"Abort"
"Zrušit"
"Abbrechen"
"Megszakít"
"Zaniechaj"
"Abortar"
"Zrušiť"

MIgnore
"Игнорировать"
"Ignore"
"Ignorovat"
"Ignorieren"
"Mégis"
"Zignoruj"
"Ignorar"
"Ignorovať"

MDelete
"Удалить"
"Delete"
"Smazat"
"Löschen"
"Töröl"
"Usuń"
"Borrar"
"Zmazať"

MSplit
"Разделить"
"Split"
"Rozdělit"
"Zerteilen"
"Feloszt"
"Podziel"
"Dividir"
"Rozdeliť"

MRemove
"Удалить"
"Remove"
"Odstranit"
"Entfernen"
"Eltávolít"
"Usuń"
"Remover"
"Odstrániť"

MHCancel
l:
"&Отмена"
"&Cancel"
"&Storno"
"&Abbrechen"
"Még&sem"
"&Anuluj"
"&Cancelar"
"&Storno"

MHRetry
"&Повторить"
"&Retry"
"&Znovu"
"&Wiederholen"
"Ú&jra"
"&Ponów"
"&Reiterar"
"&Znova"

MHSkip
"П&ропустить"
"&Skip"
"&Přeskočit"
"Über&springen"
"Ki&hagy"
"&Omiń"
"&Omitir"
"&Preskočiť"

MHSkipAll
"Пропустить &все"
"S&kip all"
"Přeskočit &vše"
"Alle übersprin&gen"
"Kihagy &mind"
"Omiń &wszystkie"
"Omitir &Todo"
"Preskočiť &všetko"

MHAbort
"Прер&вать"
"&Abort"
"Zr&ušit"
"&Abbrechen"
"Megsza&kít"
"&Zaniechaj"
"Ab&ortar"
"Zr&ušiť"

MHIgnore
"&Игнорировать"
"&Ignore"
"&Ignorovat"
"&Ignorieren"
"Mé&gis"
"Z&ignoruj"
"&Ignorar"
"&Ignorovať"

MHDelete
"&Удалить"
"&Delete"
"S&mazat"
"&Löschen"
"&Töröl"
"&Usuń"
"&Borrar"
"Z&mazať"

MHRemove
"&Удалить"
"R&emove"
"&Odstranit"
"Ent&fernen"
"Eltá&volít"
"U&suń"
"R&emover"
"&Odstrániť"

MHSplit
"Раз&делить"
"Sp&lit"
"&Rozdělit"
"&Zerteilen"
"Fel&oszt"
"Po&dziel"
"Dividir"
"&Rozdeliť"

MWarning
l:
"Предупреждение"
"Warning"
"Varování"
"Warnung"
"Figyelem"
"Ostrzeżenie"
"Advertencia"
"Varovanie"

MError
"Ошибка"
"Error"
"Chyba"
"Fehler"
"Hiba"
"Błąd"
"Error"
"Chyba"

MQuit
l:
"Выход"
"Quit"
"Konec"
"Beenden"
"Kilépés"
"Zakończ"
"Salir"
"Koniec"

MAskQuit
"Вы хотите завершить работу в FAR?"
"Do you want to quit FAR?"
"Opravdu chcete ukončit FAR?"
"Wollen Sie FAR beenden?"
"Biztosan kilép a FAR-ból?"
"Czy chcesz zakończyć pracę z FARem?"
"Desea salir de FAR?"
"Chcete ukončiť FAR?"

MF1
l:
l://functional keys - 6 characters max
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Súgó"
"Pomoc"
"Ayuda"
"Pomoc"

MF2
"ПользМ"
"UserMn"
"UživMn"
"BenuMn"
"FhMenü"
"Menu"
"Menú "
"Menu"

MF3
"Просм"
"View"
"Zobraz"
"Betr."
"Megnéz"
"Zobacz"
"Ver "
"Zobraz"

MF4
"Редакт"
"Edit"
"Edit"
"Bearb"
"Szerk."
"Edytuj"
"Editar"
"Uprav"

MF5
"Копир"
"Copy"
"Kopír."
"Kopier"
"Másol"
"Kopiuj"
"Copiar"
"Kopír."

MF6
"Перен"
"RenMov"
"PřjPřs"
"Versch"
"AtnMoz"
"ZmNazw"
"RenMov"
"Premen"

MF7
"Папка"
"MkFold"
"VytAdr"
"VerzEr"
"ÚjMapp"
"UtwKat"
"CrDIR "
"VytvAd"

MF8
"Удален"
"Delete"
"Smazat"
"Lösch."
"Töröl"
"Usuń"
"Borrar"
"Zmazať"

MF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"
"KonfMn"
"Konfig"
"BarMnú"
"KonfMn"

MF10
"Выход"
"Quit"
"Konec"
"Beend."
"Kilép"
"Koniec"
"Salir"
"Koniec"

MF11
"Плагины"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Comple"
"Moduly"

MF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"
"Ekran"
"Pantal"
"Okno"

MAltF1
l:
"Левая"
"Left"
"Levý"
"Links"
"Bal"
"Lewy"
"Izqda "
"Ľavý"

MAltF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"
"Prawy"
"Drcha "
"Pravý"

MAltF3
"Смотр."
"View.."
"Zobr.."
"Betr.."
"Néző.."
"Zobacz"
"Ver..."
"Zobr.."

MAltF4
"Редак."
"Edit.."
"Edit.."
"Bear.."
"Szrk.."
"Edytuj"
"Edita."
"Uprav."

MAltF5
"Печать"
"Print"
"Tisk"
"Druck"
"Nyomt"
"Drukuj"
"Imprim"
"Tlač"

MAltF6
"Ссылка"
"MkLink"
"VytLnk"
"LinkEr"
"ÚjLink"
"Dowiąż"
"CrEnlc"
"Prepoj"

MAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"
"Znajdź"
"Buscar"
"Hľadať"

MAltF8
"Истор"
"Histry"
"Histor"
"Histor"
"ParElő"
"Histor"
"Histor"
"Histor"

MAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Tryb"
"Video"
"Video"

MAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"
"Drzewo"
"Arbol"
"Strom"

MAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"
"HsPodg"
"VerHis"
"HsZobr"

MAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"BearHs"
"MapElő"
"HsKat"
"HisDir"
"HsAdrs"

MCtrlF1
l:
"Левая"
"Left"
"Levý"
"Links"
"Bal"
"Lewy"
"Izqda "
"Ľavý"

MCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"
"Prawy"
"Drcha "
"Pravý"

MCtrlF3
"Имя   "
"Name  "
"Název "
"Name  "
"Név"
"Nazwa"
"Nombre"
"Názov "

MCtrlF4
"Расшир"
"Extens"
"Přípon"
"Erweit"
"Kiterj"
"Rozsz"
"Extens"
"Prípon"

MCtrlF5
"Запись"
"Write"
upd:"Write"
upd:"Write"
upd:"Write"
upd:"Write"
"Fecha"
"Zmena"

MCtrlF6
"Размер"
"Size"
"Veliko"
"Größe"
"Méret"
"Rozm"
"Tamaño"
"Veľkos"

MCtrlF7
"Несорт"
"Unsort"
"Neřadi"
"Unsort"
"NincsR"
"BezSor"
"SinOrd"
"Netrie"

MCtrlF8
"Создан"
"Creatn"
"Vytvoř"
"Erstel"
"Keletk"
"Utworz"
"Creado"
"Vytvor"

MCtrlF9
"Доступ"
"Access"
"Přístu"
"Zugrif"
"Hozzáf"
"Użycie"
"Acceso"
"Prístu"

MCtrlF10
"Описан"
"Descr"
"Popis"
"Beschr"
"Megjgy"
"Opis"
"Descr"
"Popis"

MCtrlF11
"Владел"
"Owner"
"Vlastn"
"Besitz"
"Tulajd"
"Właśc"
"Dueño"
"Vlastn"

MCtrlF12
"Сорт"
"Sort"
"Třídit"
"Sort."
"RendMd"
"Sortuj"
"Orden"
"Tried"

MShiftF1
l:
"Добавл"
"Add"
"Přidat"
"Hinzu"
"Tömört"
"Dodaj"
"Añadir"
"Spakov"

MShiftF2
"Распак"
"Extrct"
"Rozbal"
"Extrah"
"Kibont"
"Rozpak"
"Extrae"
"Rozpak"

MShiftF3
"АрхКом"
"ArcCmd"
"ArcPří"
"ArcBef"
"TömPar"
"Polec"
"ArcCmd"
"PríkPk"

MShiftF4
"Редак."
"Edit.."
"Edit.."
"Erst.."
"ÚjFájl"
"Edytuj"
"Editar"
"Úpravy"

MShiftF5
"Копир"
"Copy"
"Kopír."
"Kopier"
"Másol"
"Kopiuj"
"Copiar"
"Kopír."

MShiftF6
"Переим"
"Rename"
"Přejme"
"Umbene"
"ÁtnMoz"
"ZmNazw"
"RenMov"
"Premen"

MShiftF7
""
""
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
"Lösch."
"Töröl"
"Usuń"
"Borrar"
"Zmazať"

MShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"
"Zapisz"
"Guarda"
"Uložiť"

MShiftF10
"Послдн"
"Last"
"Posled"
"Letzte"
"UtsMnü"
"Ostatn"
"Ultimo"
"Posled"

MShiftF11
"Группы"
"Group"
"Skupin"
"Gruppe"
"Csoprt"
"Grupa"
"Grupo"
"Skupin"

MShiftF12
"Выбран"
"SelUp"
"VybPrv"
"AuswOb"
"KijFel"
"SelUp"
"SelUp"
"VybPrv"

MAltShiftF1
l:
l:// Main AltShift
""
""
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
""
""

MAltShiftF3
""
""
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
""
""

MAltShiftF5
""
""
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
""
""

MAltShiftF7
""
""
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
""
""

MAltShiftF9
"КонфПл"
"ConfPl"
"KonfPl"
"KonfPn"
"PluKnf"
"KonfPl"
"ConfPl"
"KonfPl"

MAltShiftF10
""
""
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
""
""

MAltShiftF12
""
""
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
""
""

MCtrlShiftF2
""
""
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
"Megnéz"
"Podgląd"
"Ver "
"Zobraziť"

MCtrlShiftF4
"Редакт"
"Edit"
"Edit"
"Bearb"
"Szerk."
"Edycja"
"Editar"
"Upraviť"

MCtrlShiftF5
""
""
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
""
""

MCtrlShiftF7
""
""
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
""
""

MCtrlShiftF9
""
""
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
""
""

MCtrlShiftF11
""
""
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
""
""

MCtrlAltF2
""
""
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
""
""

MCtrlAltF4
""
""
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
""
""

MCtrlAltF6
""
""
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
""
""

MCtrlAltF8
""
""
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
""
""

MCtrlAltF10
""
""
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
""
""

MCtrlAltF12
""
""
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
""
""

MCtrlAltShiftF2
""
""
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
""
""

MCtrlAltShiftF4
""
""
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
""
""

MCtrlAltShiftF6
""
""
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
""
""

MCtrlAltShiftF8
""
""
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
""
""

MCtrlAltShiftF10
""
""
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
""
""

MHistoryTitle
l:
"История команд"
"History"
"Historie"
"Historie der letzten Befehle"
"Parancs előzmények"
"Historia"
"Historial"
"História"

MFolderHistoryTitle
"История папок"
"Folders history"
"Historie adresářů"
"Zuletzt besuchte Ordner"
"Mappa előzmények"
"Historia katalogów"
"Historial directorios"
"História priečinkov"

MViewHistoryTitle
"История просмотра и редактирования"
"File view and edit history"
upd:"Historie prohlížení souborů"
upd:"Zuletzt betrachtete Dateien"
upd:"Fájl előzmények"
upd:"Historia podglądu plików"
"Historial de visor y editor de archivos"
"História zobrazení a úprav súborov"

MViewHistoryIsCreate
"Создать файл?"
"Create file?"
"Vytvořit soubor?"
"Datei erstellen?"
"Fájl létrehozása?"
"Utworzyć plik?"
"Crear archivo?"
"Vytvoriť súbor?"

MHistoryView
"Просмотр"
"View"
"Zobrazit"
"Betr"
"Nézett"
"Zobacz"
"Ver   "
"Zobrazenie"

MHistoryEdit
"Редактор"
"Edit"
"Editovat"
"Bearb"
"Szerk."
"Edytuj"
"Editar"
"Úpravy"

MHistoryExt
"Внешний"
"Ext."
"Rozšíření"
"Ext."
"Kit."
"Ext."
"Ext."
"Prípona"

MHistoryClear
l:
"История будет полностью очищена. Продолжить?"
"All records in the history will be deleted. Continue?"
"Všechny záznamy v historii budou smazány. Pokračovat?"
"Die gesamte Historie wird gelöscht. Fortfahren?"
"Az előzmények minden eleme törlődik. Folytatja?"
"Wszystkie wpisy historii będą usunięte. Kontynuować?"
"Los datos en el historial serán borrados. Continuar?"
"Všetky záznamy v histórii budú zmazané. Pokračovať?"

MClear
"&Очистить"
"&Clear history"
"&Vymazat historii"
"Historie &löschen"
"Elő&zmények törlése"
"&Czyść historię"
"&Limpiar historial"
"&Vymazať históriu"

MConfigSystemTitle
l:
"Системные параметры"
"System settings"
"Nastavení systému"
"Grundeinstellungen"
"Rendszer beállítások"
"Ustawienia systemowe"
"Configuración de sistema"
"Systémové nastavenia"

MConfigRO
"&Снимать атрибут R/O c CD файлов"
"&Clear R/O attribute from CD files"
"Z&rušit atribut R/O u souborů na CD"
"Schreibschutz von CD-Dateien ent&fernen"
"&Csak olvasható attr. törlése CD fájlokról"
"Wyczyść atrybut &R/O przy kopiowaniu z CD"
"&Borrar atributos R/O de archivos de CD"
"Z&rušiť atribút R/O pri súboroch z CD"

MConfigRecycleBin
"Удалять в &Корзину"
"&Delete to Recycle Bin"
"&Mazat do Koše"
"In Papierkorb &löschen"
"&Törlés a Lomtárba"
"&Usuwaj do Kosza"
"Borrar hacia &papelera de reciclaje"
"&Mazať do Koša"

MConfigRecycleBinLink
"У&далять символические ссылки"
"Delete symbolic &links"
"Mazat symbolické &linky"
"Symbolische L&inks löschen"
"Szimbolikus l&inkek törlése"
"Usuń &linki symboliczne"
"Borrar en&laces simbólicos"
"Mazať symbo&lické odkazy"

MConfigSystemCopy
"Использовать систе&мную функцию копирования"
"Use sys&tem copy routine"
"Používat kopírovací rutiny sys&tému"
"Sys&temeigene Kopierroutine verwenden"
"&Másoláshoz a rendszerrutin használata"
"Używaj &systemowej procedury kopiowania"
"Usar rutina de copia del sis&tema"
"Kopírovať pomocou sys&témovej rutiny"

MConfigCopySharing
"Копировать открытые для &записи файлы"
"Copy files opened for &writing"
"Kopírovat soubory otevřené pro &zápis"
"Zum Schreiben geöffnete Dateien &kopieren"
"Írásra megnyitott &fájlok másolhatók"
"Kopiuj pliki otwarte do zap&isu"
"Copiar archivos abiertos para &escritura"
"Kopírovať súbory otvorené pre &zápis"

MConfigScanJunction
"Ск&анировать символические ссылки"
"Scan s&ymbolic links"
"Prohledávat s&ymbolické linky"
"S&ymbolische Links scannen"
"Szimbolikus linkek &vizsgálata"
"Skanuj linki s&ymboliczne"
"Explorar enlaces simbólicos"
"Prehľadávať s&ymbolické odkazy"

MConfigCreateUppercaseFolders
"Создавать &папки заглавными буквами"
"Create folders in &uppercase"
"Vytvářet adresáře &velkými písmeny"
"Ordner in Großschreib&ung erstellen"
"Mappák létrehozása &NAGYBETŰKKEL"
"Nazwy katalogów &WIELKIMI LITERAMI"
"Crear directorios en ma&yúsculas"
"Vytvárať priečinky s &veľkými písmenami"

MConfigSaveHistory
"Сохранять &историю команд"
"Save commands &history"
"Ukládat historii &příkazů"
"&Befehlshistorie speichern"
"Parancs elő&zmények mentése"
"Zapisz historię &poleceń"
"Guardar &historial de comandos"
"Ukladať históriu &príkazov"

MConfigSaveFoldersHistory
"Сохра&нять историю папок"
"Save &folders history"
"Ukládat historii &adresářů"
"&Ordnerhistorie speichern"
"M&appa előzmények mentése"
"Zapisz historię &katalogów"
"Guardar historial de directorios"
"Ukladať históriu prie&činkov"

MConfigSaveViewHistory
"Сохранять историю п&росмотра и редактора"
"Save &view and edit history"
"Ukládat historii Zobraz a Editu&j"
"Betrachter/&Editor-Historie speichern"
"Nézőke és &szerkesztő előzmények mentése"
"Zapisz historię podglądu i &edycji"
"Guardar historial de &visor y editor"
"Ukladať históriu &zobrazení a úprav"

MConfigRegisteredTypes
"Использовать стандартные &типы файлов"
"Use Windows &registered types"
"Používat regi&strované typy Windows"
"&Registrierte Windows-Dateitypen verwenden"
"&Windows reg. fájltípusok használata"
"Użyj zare&jestrowanych typów Windows"
"Usar extensiones &registradas de Windows"
"Použiť typy súborov regi&strované vo Windows"

MConfigCloseCDGate
"Автоматически монтироват&ь CDROM"
"CD drive auto &mount"
"Automatické př&ipojení CD disků"
"CD-Laufwerk auto&matisch schließen"
"CD tálca a&utomatikus behúzása"
"&Montuj CD automatycznie"
"CD-ROM: automontar unidad"
"Automaticky pr&ipojiť jednotku CD"

MConfigUpdateEnvironment
"Автообновл&ение переменных окружения"
"Automatic update of environment variables"
upd:"Automatic update of environment variables"
upd:"Automatic update of environment variables"
upd:"Automatic update of environment variables"
upd:"Automatic update of environment variables"
"Actualización automática de variables de entorno"
"Automaticky aktualizovať premenné prostredia"

MConfigElevation
"Запрос прав администратора:"
"Request administrator rights:"
upd:"request administrator rights:"
upd:"request administrator rights:"
upd:"request administrator rights:"
upd:"request administrator rights:"
"Requerir derechos de administrador:"
"Vyžiadať práva správcu:"

MConfigElevationModify
"д&ля изменения"
"for modification"
upd:"for modification"
upd:"for modification"
upd:"for modification"
upd:"for modification"
"para modificación"
"na úpravy"

MConfigElevationRead
"для &чтения"
"for read"
upd:"for read"
upd:"for read"
upd:"for read"
upd:"for read"
"para lectura"
"na čítanie"

MConfigElevationUsePrivileges
"исп&ользовать дополнительные привилегии"
"use additional privileges"
upd:"use additional privileges"
upd:"use additional privileges"
upd:"use additional privileges"
upd:"use additional privileges"
"usar privilegios adicionales"
"použiť ďalšie privilégiá"

MConfigAutoSave
"Автозапись кон&фигурации"
"Auto &save setup"
"Automatické ukládaní &nastavení"
"Setup automatisch &"speichern"
"B&eállítások automatikus mentése"
"Automatycznie &zapisuj ustawienia"
"Auto&guardar configuración"
"Automaticky ukladať &nastavenia"

MConfigTreeTitle
l:
"Настройки дерева папок"
"Tree settings"
upd:"Tree settings"
upd:"Tree settings"
upd:"Tree settings"
upd:"Tree settings"
"Arbol de directorios"
"Nastavenia stromu"

MConfigTreeAutoChange
"&Автосмена папки"
"&Auto change folder"
"&Automaticky měnit adresář"
"Ordner &automatisch wechseln (Baumansicht)"
"&Automatikus mappaváltás"
"&Automatycznie zmieniaj katalog"
"&Auto cambiar directorio"
"&Automaticky meniť priečinok"

MConfigTreeLabelMinFolder
"&Минимальное количество папок:"
upd:"&Minimum number of folders:"
upd:"&Minimum number of folders:"
upd:"&Minimum number of folders:"
upd:"&Minimum number of folders:"
upd:"&Minimum number of folders:"
"&Mínimo número de carpetas:"
"&Minimálny počet priečinkov:"

#MConfigTreeLabel1
#"Хранить файл структуры папок для:"
#upd:"Хранить файл структуры папок для:"
#upd:"Хранить файл структуры папок для:"
#upd:"Хранить файл структуры папок для:"
#upd:"Хранить файл структуры папок для:"
#upd:"Хранить файл структуры папок для:"
#upd:"Хранить файл структуры папок для:"

#MConfigTreeLabelLocalDisk
#"локальных дисков"
#upd:"локальных дисков"
#upd:"локальных дисков"
#upd:"локальных дисков"
#upd:"локальных дисков"
#upd:"локальных дисков"
#upd:"локальных дисков"

#MConfigTreeLabelNetDisk
#"сетевых дисков"
#upd:"сетевых дисков"
#upd:"сетевых дисков"
#upd:"сетевых дисков"
#upd:"сетевых дисков"
#upd:"сетевых дисков"
#upd:"сетевых дисков"

#MConfigTreeLabelNetPath
#"сетевых путей"
#upd:"сетевых путей"
#upd:"сетевых путей"
#upd:"сетевых путей"
#upd:"сетевых путей"
#upd:"сетевых путей"
#upd:"сетевых путей"

#MConfigTreeLabelRemovableDisk
#"сменных дисков"
#upd:"сменных дисков"
#upd:"сменных дисков"
#upd:"сменных дисков"
#upd:"сменных дисков"
#upd:"сменных дисков"
#upd:"сменных дисков"

#MConfigTreeLabelCDDisk
#"CD-дисков"
#upd:"CD-дисков"
#upd:"CD-дисков"
#upd:"CD-дисков"
#upd:"CD-дисков"
#upd:"CD-дисков"
#upd:"CD-дисков"

#MConfigTreeLabelExceptPath
#"Не создавать файл для следующих дисков:"
#upd:"Не создавать файл для следующих дисков:"
#upd:"Не создавать файл для следующих дисков:"
#upd:"Не создавать файл для следующих дисков:"
#upd:"Не создавать файл для следующих дисков:"
#upd:"Не создавать файл для следующих дисков:"
#upd:"Не создавать файл для следующих дисков:"

#MConfigTreeLabelSaveLocalPath
#"Путь для локальных дисков:"
#upd:"Путь для локальных дисков:"
#upd:"Путь для локальных дисков:"
#upd:"Путь для локальных дисков:"
#upd:"Путь для локальных дисков:"
#upd:"Путь для локальных дисков:"
#upd:"Путь для локальных дисков:"

#MConfigTreeLabelSaveNetPath
#"Путь для сетевых дисков и путей:"
#upd:"Путь для сетевых дисков и путей:"
#upd:"Путь для сетевых дисков и путей:"
#upd:"Путь для сетевых дисков и путей:"
#upd:"Путь для сетевых дисков и путей:"
#upd:"Путь для сетевых дисков и путей:"
#upd:"Путь для сетевых дисков и путей:"

MConfigPanelTitle
l:
"Настройки панели"
"Panel settings"
"Nastavení panelů"
"Panels einrichten"
"Panel beállítások"
"Ustawienia panelu"
"Configuración de paneles"
"Nastavenie panelov"

MConfigHidden
"Показывать скр&ытые и системные файлы"
"Show &hidden and system files"
"Ukázat &skryté a systémové soubory"
"&Versteckte und Systemdateien anzeigen"
"&Rejtett és rendszerfájlok mutatva"
"Pokazuj pliki &ukryte i systemowe"
"Mostrar archivos ocultos y de sistema"
"Zobraziť &skryté a systémové súbory"

MConfigHighlight
"&Раскраска файлов"
"Hi&ghlight files"
"Zvý&razňovat soubory"
"Dateien mark&ieren"
"Fá&jlok kiemelése"
"W&yróżniaj pliki"
"Resaltar archivos"
"Zv&ýrazňovať súbory"

MConfigSelectFolders
"Пометка &папок"
"Select &folders"
"Vybírat a&dresáře"
"&Ordner auswählen"
"A ma&ppák is kijelölhetők"
"Zaznaczaj katalo&gi"
"Seleccionar &directorios"
"&Zvýrazňovať priečinky"

MConfigSortFolderExt
"Сортировать имена папок по рас&ширению"
"Sort folder names by e&xtension"
"Řadit adresáře podle přípony"
"Ordner nach Er&weiterung sortieren"
"Mappák is rendezhetők &kiterjesztés szerint"
"Sortuj nazwy katalogów wg r&ozszerzeń"
"Ordenar directorios por extensión"
"Triediť priečinky podľa prípony"

MConfigReverseSort
"Разрешить &обратную сортировку"
"Allow re&verse sort modes"
"Do&volit změnu směru řazení"
"&Umgekehrte Sortiermodi zulassen"
"Fordí&tott rendezés engedélyezése"
"Włącz &możliwość odwrotnego sortowania"
"Permitir modo de orden in&verso"
"Po&voliť opačné triedenie"

MConfigAutoUpdateLimit
"Отключать автооб&новление панелей,"
"&Disable automatic update of panels"
"Vypnout a&utomatickou aktualizaci panelů"
"Automatisches Panelupdate &deaktivieren"
"Pan&el automatikus frissítése kikapcsolva,"
"&Wyłącz automatyczną aktualizację paneli"
"Desactiva actualización automát. de &paneles"
"Zrušiť a&utomatickú aktualizáciu panelov"

MConfigAutoUpdateLimit2
"если объектов больше"
"if object count exceeds"
"jestliže počet objektů překročí"
"wenn mehr Objekte als"
"ha több elem van, mint:"
"jeśli zawierają więcej obiektów niż"
"si conteo de objetos es excedido"
"ak počet objektov prekročí"

MConfigAutoUpdateRemoteDrive
"Автообновление с&етевых дисков"
"Network drives autor&efresh"
"Automatická obnova síťových disků"
"Netzw&erklauferke autom. aktualisieren"
"Hálózati meghajtók autom. &frissítése"
"Auto&odświeżanie dysków sieciowych"
"Actualizar unidad&es de Red"
"Automaticky obnovovať si&eťové disky"

MConfigShowColumns
"Показывать &заголовки колонок"
"Show &column titles"
"Zobrazovat &nadpisy sloupců"
"S&paltentitel anzeigen"
"Oszlop&nevek mutatva"
"Wyświetl tytuły &kolumn"
"Mostrar títulos de &columnas"
"Zobrazovať &názvy stĺpcov"

MConfigShowStatus
"Показывать &строку статуса"
"Show &status line"
"Zobrazovat sta&vový řádek"
"&Statuszeile anzeigen"
"Á&llapotsor mutatva"
"Wyświetl &linię statusu"
"Mostrar línea de e&stado"
"Zobrazovať sta&vový riadok"

MConfigDetailedJunction
"Определять точки монтирования &диска"
"Determine Volume Mount &point"
upd:"Determine Volume Mount point"
upd:"Determine Volume Mount point"
upd:"Determine Volume Mount point"
upd:"Determine Volume Mount point"
upd:"Determine Volume Mount point"
upd:"Determine Volume Mount point"

MConfigShowTotal
"Показывать су&ммарную информацию"
"Show files &total information"
"Zobrazovat &informace o velikosti souborů"
"&Gesamtzahl für Dateien anzeigen"
"Fájl össze&s információja mutatva"
"Wyświetl &całkowitą informację o plikach"
"Mostrar información comple&ta de archivos"
"Zobrazovať &informácie o súboroch"

MConfigShowFree
"Показывать с&вободное место"
"Show f&ree size"
"Zobrazovat vo&lné místo"
"&Freien Speicher anzeigen"
"Sza&bad lemezterület mutatva"
"Wyświetl ilość &wolnego miejsca"
"Mostrar espacio lib&re"
"Zobrazovať vo&ľné miesto"

MConfigShowScrollbar
"Показывать по&лосу прокрутки"
"Show scroll&bar"
"Zobrazovat &posuvník"
"Scroll&balken anzeigen"
"Gördítősá&v mutatva"
"Wyświetl &suwak"
"Mostrar &barra de desplazamiento"
"Zobrazovať &posuvník"

MConfigShowScreensNumber
"Показывать количество &фоновых экранов"
"Show background screens &number"
"Zobrazovat počet &obrazovek na pozadí"
"&Nummer von Hintergrundseiten anzeigen"
"&Háttérképernyők száma mutatva"
"Wyświetl ilość &ekranów w tle"
"Mostrar &número de pantallas de fondo"
"Zobrazovať počet &okien na pozadí"

MConfigShowSortMode
"Показывать букву режима сор&тировки"
"Show sort &mode letter"
"Zobrazovat písmeno &módu řazení"
"Buchstaben der Sortier&modi anzeigen"
"Rendezési mó&d betűjele mutatva"
"Wyświetl l&iterę trybu sortowania"
"Mostrar letra para &modo de orden"
"Zobrazovať písmeno &módu triedenia"

MConfigShowDotsInRoot
"Показывать \"..\" в корневых каталогах"
"Show \"..\" in root folders"
"Show \"..\" in root folders"
"Show \"..\" in root folders"
"Show \"..\" in root folders"
"Show \"..\" in root folders"
"Mostrar \"..\" en directorio raíz"
"Zobrazovať \"..\" v koreňových priečinkoch"

MConfigHighlightColumnSeparator
"Подсвечивать разделители колонок"
"Highlight column separators"
upd:"Highlight column separators"
upd:"Highlight column separators"
upd:"Highlight column separators"
upd:"Highlight column separators"
"Resaltar separador de columnas"
"Zvýrazňovať oddeľovače stĺpcov"

MConfigDoubleGlobalColumnSeparator
"Удваивать глобальные разделители колонок"
"Double global column separators"
upd:"Double global column separators"
upd:"Double global column separators"
upd:"Double global column separators"
upd:"Double global column separators"
"Separador de columna doble"
"Dvojité oddeľovače hlavných stĺpcov"

MConfigInterfaceTitle
l:
"Настройки интерфейса"
"Interface settings"
"Nastavení rozhraní"
"Oberfläche einrichten"
"Kezelőfelület beállítások"
"Ustawienia interfejsu"
"Configuración de interfaz"
"Nastavenie rozhrania"

MConfigClock
"&Часы в панелях"
"&Clock in panels"
"&Hodiny v panelech"
"&Uhr in Panels anzeigen"
"Ór&a a paneleken"
"&Zegar"
"&Reloj en paneles"
"&Hodiny v paneloch"

MConfigViewerEditorClock
"Ч&асы при редактировании и просмотре"
"C&lock in viewer and editor"
"H&odiny v prohlížeči a editoru"
"U&hr in Betrachter und Editor anzeigen"
"Ó&ra a nézőkében és szerkesztőben"
"Zegar w &podglądzie i edytorze"
"Re&loj en visor y editor"
"H&odiny v zobrazovači a editore"

MConfigMouse
"Мы&шь"
"M&ouse"
"M&yš"
"M&aus aktivieren"
"&Egér kezelése"
"M&ysz"
"Rat&ón"
"M&yš"

MConfigKeyBar
"Показывать &линейку клавиш"
"Show &key bar"
"Zobrazovat &zkratkové klávesy"
"Tast&enleiste anzeigen"
"&Funkcióbillentyűk sora mutatva"
"Wyświetl pasek &klawiszy"
"Mostrar barra de &funciones"
"Zobrazovať tlačidlá &funkčných klávesov"

MConfigMenuBar
"Всегда показывать &меню"
"Always show &menu bar"
"Vždy zobrazovat hlavní &menu"
"&Menüleiste immer anzeigen"
"A &menüsor mindig látszik"
"Zawsze pokazuj pasek &menu"
"Mostrar siempre barra de &menú"
"Zobrazovať horné &menu"

MConfigSaver
"&Сохранение экрана"
"&Screen saver"
"Sp&ořič obrazovky"
"Bildschirm&schoner"
"&Képernyőpihentető"
"&Wygaszacz ekranu"
"&Salvapantallas"
"Š&etrič obrazovky"

MConfigSaverMinutes
"минут"
"minutes"
"minut"
"Minuten"
"perc tétlenség után"
"m&inut"
"minutos"
"minút"

MConfigCopyTotal
"Показывать &общий индикатор копирования"
"Show &total copy progress indicator"
"Zobraz. ukazatel celkového stavu &kopírování"
"Zeige Gesamtfor&tschritt beim Kopieren"
"Másolás összesen folyamat&jelző"
"Pokaż &całkowity postęp kopiowania"
"Mostrar indicador de progreso de copia &total"
"Zobraziť indikátor celkového priebehu &kopírovania"

MConfigCopyTimeRule
"Показывать информацию о времени &копирования"
"Show cop&ying time information"
"Zobrazovat informace o čase kopírování"
"Zeige Rest&zeit beim Kopieren"
"Má&solási idő mutatva"
"Pokaż informację o c&zasie kopiowania"
"Mostrar información de tiempo de copiado"
"Zobraziť informáciu o dobe kopírovania"

MConfigDeleteTotal
"Показывать общий индикатор &удаления"
"Show total &delete progress indicator"
upd:"Show total delete progress indicator"
upd:"Show total delete progress indicator"
upd:"Show total delete progress indicator"
upd:"Show total delete progress indicator"
"Mostrar indicador de progreso de borrado total"
"Zobraziť indikátor celkového priebehu ma&zania"

MConfigPgUpChangeDisk
"Использовать Ctrl-PgUp для в&ыбора диска"
"Use Ctrl-Pg&Up to change drive"
"Použít Ctrl-Pg&Up pro změnu disku"
"Strg-Pg&Up wechselt das Laufwerk"
"A Ctrl-Pg&Up meghajtót vált"
"Użyj Ctrl-Pg&Up do zmiany napędu"
"Usar Ctrl-Pg&Up para cambiar unidad"
"Použiť Ctrl-Pg&Up na zmenu disku"

MConfigClearType
"Уч&итывать состояние ClearType"
"Cl&earType friendly redraw (can be slow)"
upd:"ClearType friendly redraw (can be slow)"
upd:"ClearType friendly redraw (can be slow)"
upd:"ClearType friendly redraw (can be slow)"
upd:"ClearType friendly redraw (can be slow)"
"Limpiar rediseño (puede ser lento)"
"Použiť &ClearType (pomalšie)"

MConfigTitleAddons
"Дополнительная информация для &заголовка окна:"
"Far &window title addons:"
upd:"Far window title addons:"
upd:"Far window title addons:"
upd:"Far window title addons:"
upd:"Far window title addons:"
"Título de ventana de FAR:"
"Far &window title addons:"

MConfigDlgSetsTitle
l:
"Настройки диалогов"
"Dialog settings"
"Nastavení dialogů"
"Dialoge einrichten"
"Párbeszédablak beállítások"
"Ustawienia okien dialogowych"
"Configuración de diálogo"
"Nastavenia dialógov"

MConfigDialogsEditHistory
"&История в строках ввода диалогов"
"&History in dialog edit controls"
"H&istorie v dialozích"
"&Historie in Eingabefelder von Dialogen"
"&Beviteli sor előzmények mentése"
"&Historia w polach edycyjnych"
"&Historial en controles de diálogo de edición"
"H&istória v dialógoch"

MConfigDialogsEditBlock
"&Постоянные блоки в строках ввода"
"&Persistent blocks in edit controls"
"&Trvalé bloky v editačních polích"
"Dauer&hafte Markierungen in Eingabefelder"
"Maradó b&lokkok a beviteli sorokban"
"&Trwałe bloki podczas edycji"
"&Bloques persistentes en controles de edición"
"&Trvalé bloky v editačných poliach"

MConfigDialogsDelRemovesBlocks
"Del удаляет б&локи в строках ввода"
"&Del removes blocks in edit controls"
"&Del maže položky v editačních polích"
"&Entf löscht Markierungen"
"A &Del törli a beviteli sorok blokkjait"
"&Del usuwa blok podczas edycji"
"&Del remueve bloques en controles de edición"
"&Del vymaže položky v editačných poliach"

MConfigDialogsAutoComplete
"&Автозавершение в строках ввода"
"&AutoComplete in edit controls"
"Automatické dokončování v editač&ních polích"
"&Automatisches Vervollständigen"
"Beviteli sor a&utomatikus kiegészítése"
"&Autouzupełnianie podczas edycji"
"Autocompl&etar en controles de edición"
"Automatické dokončovanie v editač&ných poliach"

MConfigDialogsEULBsClear
"Backspace &удаляет неизмененный текст"
"&Backspace deletes unchanged text"
"&Backspace maže nezměněný text"
"&Rücktaste (BS) löscht unveränderten Text"
"A Ba&ckspace törli a változatlan szöveget"
"&Backspace usuwa nie zmieniony tekst"
"&Backspace elimina texto no cambiado"
"&Backspace vymaže nezmenený text"

MConfigDialogsMouseButton
"Клик мыши &вне диалога закрывает диалог"
"Mouse click &outside a dialog closes it"
"Kl&iknutí myší mimo dialog ho zavře"
"Dial&og schließen wenn Mausklick ausserhalb"
"&Egérkattintás a párb.ablakon kívül: bezárja"
"&Kliknięcie myszy poza oknem zamyka je"
"Click de ratón fuera del diálogo, lo cierra"
"Kl&iknutie myšou mimo dialógového okna ho zavrie"

MConfigVMenuTitle
l:
"Настройки меню"
"Menu settings"
upd:"Menu settings"
upd:"Menu settings"
upd:"Menu settings"
upd:"Menu settings"
"Configuración de menú"
"Nastavenia menu"

MConfigVMenuLBtnClick
"Клик левой кнопки мыши вне меню"
"Left mouse click outside a menu"
upd:"Left mouse click outside a menu"
upd:"Left mouse click outside a menu"
upd:"Left mouse click outside a menu"
upd:"Left mouse click outside a menu"
"Click izquierdo de ratón fuera de menú"
"Kliknutie ľavým tlačidlom myši mimo menu"

MConfigVMenuRBtnClick
"Клик правой кнопки мыши вне меню"
"Right mouse click outside a menu"
upd:"Right mouse click outside a menu"
upd:"Right mouse click outside a menu"
upd:"Right mouse click outside a menu"
upd:"Right mouse click outside a menu"
"Click derecho de ratón fuera de menú"
"Kliknutie pravým tlačidlom myši mimo menu"

MConfigVMenuMBtnClick
"Клик средней кнопки мыши вне меню"
"Middle mouse click outside a menu"
upd:"Middle mouse click outside a menu"
upd:"Middle mouse click outside a menu"
upd:"Middle mouse click outside a menu"
upd:"Middle mouse click outside a menu"
"Click medio de ratón fuera de menú"
"Kliknutie prostredným tlačidlom myši mimo menu"

MConfigVMenuClickCancel
"Закрыть с отменой"
"Cancel menu"
upd:"Cancel menu"
upd:"Cancel menu"
upd:"Cancel menu"
upd:"Cancel menu"
"Cancelar menú"
"Zrušiť menu"

MConfigVMenuClickApply
"Выполнить текущий пункт"
"Execute selected item"
upd:"Execute selected item"
upd:"Execute selected item"
upd:"Execute selected item"
upd:"Execute selected item"
"Ejecutar ítem seleccionado"
"Spustiť zvolenú položku"

MConfigVMenuClickIgnore
"Ничего не делать"
"Do nothing"
upd:"Do nothing"
upd:"Do nothing"
upd:"Do nothing"
upd:"Do nothing"
"No hacer nada"
"Neurobiť nič"

MConfigCmdlineTitle
l:
"Настройки командной строки"
"Command line settings"
upd:"Command line settings"
upd:"Command line settings"
upd:"Command line settings"
upd:"Command line settings"
"Configuración de línea de comando"
"Nastavenia príkazového riadka"

MConfigCmdlineEditBlock
"&Постоянные блоки"
"&Persistent blocks"
upd:"Persistent blocks"
upd:"Persistent blocks"
upd:"Persistent blocks"
upd:"Persistent blocks"
"&Bloques persistentes"
"Trvalé bloky"

MConfigCmdlineDelRemovesBlocks
"Del удаляет б&локи"
"&Del removes blocks"
upd:"Del removes blocks"
upd:"Del removes blocks"
upd:"Del removes blocks"
upd:"Del removes blocks"
"&Del remueve bloques"
"Del vymaže bloky"

MConfigCmdlineAutoComplete
"&Автозавершение"
"&AutoComplete"
upd:"AutoComplete"
upd:"AutoComplete"
upd:"AutoComplete"
upd:"AutoComplete"
"&AutoCompletar"
"AutoComplete"

MConfigCmdlineUsePromptFormat
"Установить &формат командной строки"
"Set command line &prompt format"
"Nastavit formát &příkazového řádku"
"&Promptformat der Kommandozeile"
"Parancssori &prompt formátuma"
"Wy&gląd znaku zachęty linii poleceń"
"Formato para línea de comando (&prompt)"
"Nastaviť formát &príkazového riadka"

MConfigCmdlineUseHomeDir
"Использовать &домашний каталог"
"Use &home dir"
upd:"Use &home dir"
upd:"Use &home dir"
upd:"Use &home dir"
upd:"Use &home dir"
upd:"Use &home dir"
upd:"Use &home dir"

MConfigCmdlinePromptFormatAdmin
"Администратор"
"Administrator"
upd:"Administrator"
upd:"Administrator"
upd:"Administrator"
upd:"Administrator"
"Administrador"
"Administrátor"

MConfigAutoCompleteTitle
l:
"Настройка автозавершения"
"AutoComplete settings"
upd:"AutoComplete settings"
upd:"AutoComplete settings"
upd:"AutoComplete settings"
upd:"AutoComplete settings"
"Configuración de autocompletar"
"Nastavenia AutoComplete"

MConfigAutoCompleteShowList
l:
"Показывать &список"
"&Show list"
upd:"&Show list"
upd:"&Show list"
upd:"&Show list"
upd:"&Show list"
"Mo&strar lista"
"&Zobraziť zoznam"

MConfigAutoCompleteModalList
l:
"&Модальный режим"
"&Modal mode"
upd:"&Modal mode"
upd:"&Modal mode"
upd:"&Modal mode"
upd:"&Modal mode"
"Clase de &Modo"
"&Modal mode"

MConfigAutoCompleteAutoAppend
l:
"&Подставлять первый подходящий вариант"
"&Append first matched item"
upd:"&Append first matched item"
upd:"&Append first matched item"
upd:"&Append first matched item"
upd:"&Append first matched item"
"&Agregar primer ítem coincidente"
"&Append first matched item"

MConfigInfoPanelTitle
l:
"Настройка информационной панели"
"InfoPanel settings"
upd:"InfoPanel settings"
upd:"InfoPanel settings"
upd:"InfoPanel settings"
upd:"InfoPanel settings"
"Configuración de panel de información"
"Nastavenia InfoPanela"

MConfigInfoPanelCNTitle
"Формат вывода имени &компьютера"
upd:"&ComputerName format"
upd:"&ComputerName format"
upd:"&ComputerName format"
upd:"&ComputerName format"
upd:"&ComputerName format"
"Formato nombre &computadora"
"&ComputerName format"

MConfigInfoPanelCNPhysicalNetBIOS
upd:"Physical NetBIOS"
upd:"Physical NetBIOS"
upd:"Physical NetBIOS"
upd:"Physical NetBIOS"
upd:"Physical NetBIOS"
upd:"Physical NetBIOS"
"NetBios físico"
"Physical NetBIOS"

MConfigInfoPanelCNPhysicalDnsHostname
upd:"Physical DNS hostname"
upd:"Physical DNS hostname"
upd:"Physical DNS hostname"
upd:"Physical DNS hostname"
upd:"Physical DNS hostname"
upd:"Physical DNS hostname"
"DNS hostname físico"
"Physical DNS hostname"

MConfigInfoPanelCNPhysicalDnsDomain
upd:"Physical DNS domain"
upd:"Physical DNS domain"
upd:"Physical DNS domain"
upd:"Physical DNS domain"
upd:"Physical DNS domain"
upd:"Physical DNS domain"
"Dominio DNS físico"
"Physical DNS domain"

MConfigInfoPanelCNPhysicalDnsFullyQualified
upd:"Physical DNS fully-qualified"
upd:"Physical DNS fully-qualified"
upd:"Physical DNS fully-qualified"
upd:"Physical DNS fully-qualified"
upd:"Physical DNS fully-qualified"
upd:"Physical DNS fully-qualified"
"DNS calificado físico"
"Physical DNS fully-qualified"

MConfigInfoPanelCNNetBIOS
upd:"NetBIOS"
upd:"NetBIOS"
upd:"NetBIOS"
upd:"NetBIOS"
upd:"NetBIOS"
upd:"NetBIOS"
"NetBios"
"NetBIOS"

MConfigInfoPanelCNDnsHostname
upd:"DNS hostname"
upd:"DNS hostname"
upd:"DNS hostname"
upd:"DNS hostname"
upd:"DNS hostname"
upd:"DNS hostname"
"DNS hostname"
"DNS hostname"

MConfigInfoPanelCNDnsDomain
upd:"DNS domain"
upd:"DNS domain"
upd:"DNS domain"
upd:"DNS domain"
upd:"DNS domain"
upd:"DNS domain"
"Dominio DNS"
"DNS domain"

MConfigInfoPanelCNDnsFullyQualified
upd:"DNS fully-qualified"
upd:"DNS fully-qualified"
upd:"DNS fully-qualified"
upd:"DNS fully-qualified"
upd:"DNS fully-qualified"
upd:"DNS fully-qualified"
"DNS Calificado"
"DNS fully-qualified"

MConfigInfoPanelUNTitle
"Формат вывода имени &пользователя"
upd:"&UserName format"
upd:"&UserName format"
upd:"&UserName format"
upd:"&UserName format"
upd:"&UserName format"
"Formato nombre de &usuario"
"&UserName format"

MConfigInfoPanelUNUnknown
"По умолчанию"
"Default"
upd:"Default"
upd:"Default"
upd:"Default"
upd:"Default"
"Por defecto"
"Default"

MConfigInfoPanelUNFullyQualifiedDN
"Полностью определённое имя домена"
"Fully Qualified Domain Name"
upd:"Fully Qualified Domain Name"
upd:"Fully Qualified Domain Name"
upd:"Fully Qualified Domain Name"
upd:"Fully Qualified Domain Name"
"Nombre dominio completamente calificado"
"Fully Qualified Domain Name"

MConfigInfoPanelUNSamCompatible
upd:"Sam Compatible"
upd:"Sam Compatible"
upd:"Sam Compatible"
upd:"Sam Compatible"
upd:"Sam Compatible"
upd:"Sam Compatible"
"Compatible con Sam"
"Sam Compatible"

MConfigInfoPanelUNDisplay
"Отображаемое имя"
upd:"Display Name"
upd:"Display Name"
upd:"Display Name"
upd:"Display Name"
upd:"Display Name"
"Mostrar Nombre"
"Display Name"

MConfigInfoPanelUNUniqueId
"Уникальный идентификатор"
upd:"Unique Id"
upd:"Unique Id"
upd:"Unique Id"
upd:"Unique Id"
upd:"Unique Id"
"ID único"
"Unique Id"

MConfigInfoPanelUNCanonical
"Канонический вид"
"Canonical Name"
upd:"Canonical Name"
upd:"Canonical Name"
upd:"Canonical Name"
upd:"Canonical Name"
"Nombre Canónico"
"Canonical Name"

MConfigInfoPanelUNUserPrincipal
"Основное имя пользователя"
upd:"User Principial Name"
upd:"User Principial Name"
upd:"User Principial Name"
upd:"User Principial Name"
upd:"User Principial Name"
"Nombre principal usuario"
"User Principial Name"

MConfigInfoPanelUNServicePrincipal
upd:"Service Principal"
upd:"Service Principal"
upd:"Service Principal"
upd:"Service Principal"
upd:"Service Principal"
upd:"Service Principal"
"Servicio principal"
"Service Principal"

MConfigInfoPanelUNDnsDomain
upd:"Dns Domain"
upd:"Dns Domain"
upd:"Dns Domain"
upd:"Dns Domain"
upd:"Dns Domain"
upd:"Dns Domain"
"Dominio DNS"
"Dns Domain"

MConfigInfoPanelShowPowerStatus
"Показывать состояние п&итания"
"Show &power status"
upd:"Show &power status"
upd:"Show &power status"
upd:"Show &power status"
upd:"Show &power status"
"Mostrar estado de &encendido"
"Zobraziť stav na&pájania"

MMenuInfoShowModeTitle
l:
"Показать информацию"
"Show Info"
upd:"Show Info"
upd:"Show Info"
upd:"Show Info"
upd:"Show Info"
"Mostrar Info"
"Zobraziť Info"

MMenuInfoShowModeDisk
"&Диск"
"&Disk"
upd:"&Disk"
upd:"&Disk"
upd:"&Disk"
upd:"&Disk"
"&Disco"
"&Disk"

MMenuInfoShowModeMemory
"&Память"
"&Memory"
upd:"&Memory"
upd:"&Memory"
upd:"&Memory"
upd:"&Memory"
"&Memoria"
"Pa&mäť"

MMenuInfoShowModeDirDiz
"&Описание папки"
"&Description"
upd:"&Description"
upd:"&Description"
upd:"&Description"
upd:"&Description"
"&Descripción"
"&Popis"

MMenuInfoShowModePluginDiz
"Пла&гиновая панель"
"Plu&gin panel"
upd:"Plu&gin panel"
upd:"Plu&gin panel"
upd:"Plu&gin panel"
upd:"Plu&gin panel"
"Panel de Complemento"
"Panel modu&lov"

MMenuInfoShowModePower
"Состояние п&итания"
"&Power status"
upd:"&Power status"
upd:"&Power status"
upd:"&Power status"
upd:"&Power status"
"Estado &encendido"
"Stav na&pájania"

MViewConfigTitle
l:
"Программа просмотра"
"Viewer"
"Prohlížeč"
"Betrachter"
"Nézőke"
"Podgląd"
"Visor"
"Zobrazovač"

MViewConfigExternalF3
"Запускать внешнюю программу просмотра по F3 вместо Alt-F3"
"Use external viewer for F3 instead of Alt-F3"
upd:"Use external viewer for F3 instead of Alt-F3"
upd:"Use external viewer for F3 instead of Alt-F3"
"Alt-F3 helyett F3 indítja a külső nézőkét"
upd:"Use external viewer for F3 instead of Alt-F3"
"Usar visor externo con F3 en lugar de Alt-F3"
"Použiť externý zobrazovač pre F3 namiesto Alt-F3"

MViewConfigExternalCommand
"&Команда просмотра:"
"&Viewer command:"
"&Příkaz prohlížeče:"
"Befehl für e&xternen Betracher:"
"Nézőke &parancs:"
"&Polecenie:"
"Comando &visor:"
"&Príkaz zobrazovača:"

MViewConfigInternal
" Встроенная программа просмотра "
" Internal viewer "
" Interní prohlížeč "
" Interner Betracher "
" Belső nézőke "
" Podgląd wbudowany "
" Visor interno "
" Interný zobrazovač "

MViewConfigSavePos
"&Сохранять позицию файла"
"&Save file position"
"&Ukládat pozici v souboru"
"Dateipositionen &speichern"
"&Fájlpozíció mentése"
"&Zapamiętaj pozycję w pliku"
"&Guardar posición de archivo"
"Pamä&tať si pozíciu v súbore"

MViewConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"
"Ukládat &záložky"
"&Lesezeichen speichern"
"Könyv&jelzők mentése"
"Zapisz z&akładki"
"Guardar &marcadores"
"Ukladať &záložky"

MViewAutoDetectCodePage
"&Автоопределение кодовой страницы"
"&Autodetect code page"
upd:"&Autodetekovat znakovou sadu"
upd:"Zeichentabelle &automatisch erkennen"
"&Kódlap automatikus felismerése"
"Rozpozn&aj tablicę znaków"
"&Autodetectar página de códigos"
"&Automaticky zistiť tabuľku znakov"

MViewConfigTabSize
"Размер &табуляции"
"&Tab size"
"Velikost &Tabulátoru"
"Ta&bulatorgröße"
"Ta&bulátor mérete"
"Rozmiar &tabulatora"
"&Tamaño de tabulación"
"Veľkosť &tabulátora"

MViewConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"
"Zobrazovat posu&vník"
"Scro&llbalken anzeigen"
"Gör&dítősáv mutatva"
"Pokaż &pasek przewijania"
"Mostrar &barra de desplazamiento"
"Zobraziť posu&vník"

MViewConfigArrows
"Показывать стрелки с&двига"
"Show scrolling arro&ws"
"Zobrazovat &skrolovací šipky"
"P&feile bei Scrollbalken zeigen"
"Gördítőn&yilak mutatva"
"Pokaż strzał&ki przewijania"
"Mostrar &flechas de desplazamiento"
"Zobraziť &šípky posuvníka"

MViewConfigPersistentSelection
"Постоянное &выделение"
"&Persistent selection"
"Trvalé &výběry"
"Dauerhafte Text&markierungen"
"&Maradó blokkok"
"T&rwałe zaznaczenie"
"Selección &persistente"
"Trvalý &výber"

MViewConfigAnsiCodePageAsDefault
"&Использовать кодовую страницу ANSI по умолчанию"
"Use ANS&I code page by default"
upd:"Automaticky otevírat soubory ve &WIN kódování"
upd:"Dateien standardmäßig mit Windows-Kod&ierung öffnen"
"Fájlok eredeti megnyitása ANS&I kódlappal"
"&Otwieraj pliki w kodowaniu Windows"
"Usar página de código ANS&I por defecto"
"Predvolená tabuľka znakov ANS&I"

MViewConfigMaxLineSize
"&Максимальная ширина строки"
"&Maximum line width"
upd:"Maximum line width"
upd:"Maximum line width"
upd:"Maximum line width"
upd:"Maximum line width"
"Ancho máximo de &línea"
"&Maximálna šírka čiary"

MViewConfigVisible0x00
"Показывать '\\&0'"
"Visible '\\&0'"
upd:"Visible '\\0'"
upd:"Visible '\\0'"
upd:"Visible '\\0'"
upd:"Visible '\\0'"
"Visible '\\0'"
"Viditeľné '\\0'"

MViewConfigEditAutofocus
"Авто-&фокус в диалоге поиска"
"Search dialog auto-&focus"
upd:"Search dialog auto-focus"
upd:"Search dialog auto-focus"
upd:"Search dialog auto-focus"
upd:"Search dialog auto-focus"
"Auto-enfocar búsqueda en &diálogo"
"Search dialog auto-focus"

MEditConfigTitle
l:
"Редактор"
"Editor"
"Editor"
"Editor"
"Szerkesztő"
"Edytor"
"Editor"
"Editor"

MEditConfigEditorF4
"Запускать внешний редактор по F4 вместо Alt-F4"
"Use external editor for F4 instead of Alt-F4"
upd:"Use external editor for F4 instead of Alt-F4"
upd:"Use external editor for F4 instead of Alt-F4"
"Alt-F4 helyett F4 indítja a külső szerkesztőt"
upd:"Use external editor for F4 instead of Alt-F4"
"Usar editor externo con F4 en lugar de Alt-F4"
"Použiť externý editor pre F4 namiesto Alt-F4"

MEditConfigEditorCommand
"&Команда редактирования:"
"&Editor command:"
"&Příkaz editoru:"
"Befehl für e&xternen Editor:"
"&Szerkesztő parancs:"
"&Polecenie:"
"Comando &editor:"
"&Príkaz editora:"

MEditConfigInternal
" Встроенный редактор "
" Internal editor "
" Interní editor "
" Interner Editor "
" Belső szerkesztő "
" Edytor wbudowany "
" Editor interno "
" Interný editor "

MEditConfigExpandTabsTitle
"Преобразовывать &табуляцию:"
"Expand &tabs:"
"Rozšířit Ta&bulátory mezerami"
"&Tabs expandieren:"
"&Tabulátorból szóközök:"
"Zamiana znaków &tabulacji:"
"E&xpandir tabulación a espacios"
"Rozšíriť ta&bulátory medzerami"

MEditConfigDoNotExpandTabs
l:
"Не преобразовывать табуляцию"
"Do not expand tabs"
"Nerozšiřovat tabulátory mezerami"
"Tabs nicht expandieren"
"Ne helyettesítse a tabulátorokat"
"Nie zamieniaj znaków tabulacji"
"No expandir tabulaciones"
"Nerozšíriť tabulátory medzerami"

MEditConfigExpandTabs
"Преобразовывать новые символы табуляции в пробелы"
"Expand newly entered tabs to spaces"
"Rozšířit nově zadané tabulátory mezerami"
"Neue Tabs zu Leerzeichen expandieren"
"Újonnan beírt tabulátorból szóközök"
"Zamień nowo dodane znaki tabulacji na spacje"
"Expandir nuevas tabulaciones ingresadas a espacios"
"Rozšíriť nové tabulátory medzerami"

MEditConfigConvertAllTabsToSpaces
"Преобразовывать все символы табуляции в пробелы"
"Expand all tabs to spaces"
"Rozšířit všechny tabulátory mezerami"
"Alle Tabs zu Leerzeichen expandieren"
"Minden tabulátorból szóközök"
"Zastąp wszystkie tabulatory spacjami"
"Expandir todas las tabulaciones a espacios"
"Rozšíriť všetky tabulátory medzerami"

MEditConfigPersistentBlocks
"&Постоянные блоки"
"&Persistent blocks"
"&Trvalé bloky"
"Dauerhafte Text&markierungen"
"&Maradó blokkok"
"T&rwałe bloki"
"&Bloques persistente"
"&Trvalé bloky"

MEditConfigDelRemovesBlocks
l:
"Del удаляет б&локи"
"&Del removes blocks"
"&Del maže bloky"
"&Entf löscht Textmark."
"A &Del törli a blokkokat"
"&Del usuwa bloki"
"Del &remueve bloques"
"&Del maže bloky"

MEditConfigAutoIndent
"Авто&отступ"
"Auto &indent"
"Auto &Odsazování"
"Automatischer E&inzug"
"Automatikus &behúzás"
"Automatyczne &wcięcia"
"Poner &sangría automáticamente"
"Automatické &odsadenie"

MEditConfigSavePos
"&Сохранять позицию файла"
"&Save file position"
"&Ukládat pozici v souboru"
"Dateipositionen &speichern"
"Fájl&pozíció mentése"
"&Zapamiętaj pozycję kursora w pliku"
"&Guardar posición de archivo"
"&Ukladať pozíciu v súbore"

MEditConfigSaveShortPos
"Сохранять &закладки"
"Save &bookmarks"
"Ukládat zá&ložky"
"&Lesezeichen speichern"
"Könyv&jelzők mentése"
"Zapisz &zakładki"
"Guardar &marcadores"
"Ukladať zá&ložky"

MEditCursorBeyondEnd
"Ку&рсор за пределами строки"
"&Cursor beyond end of line"
"&Kurzor za koncem řádku"
upd:"&Cursor hinter dem Ende"
"Kurzor a sor&végjel után is"
"&Kursor za końcem linii"
"&Cursor después de fin de línea"
"&Kurzor za koncom riadka"

MEditAutoDetectCodePage
"&Автоопределение кодовой страницы"
"&Autodetect code page"
upd:"&Autodetekovat znakovou sadu"
upd:"Zeichentabelle &automatisch erkennen"
"&Kódlap automatikus felismerése"
"Rozpozn&aj tablicę znaków"
"&Autodetectar página de códigos"
"&Automaticky zistiť tabuľku znakov"

MEditShareWrite
"Разрешить редактирование открытых для записи &файлов"
"Allow editing files ope&ned for writing"
upd:"Allow editing files opened for &writing"
upd:"Allow editing files opened for &writing"
"Írásra m&egnyitott fájlok szerkeszthetők"
upd:"Allow editing files opened for &writing"
"Permitir escrit&ura de archivos abiertos para edición"
"Umožniť úpravu súborov otvorených &pre zápis"

MEditLockROFileModification
"Блокировать р&едактирование файлов с атрибутом R/O"
"Lock editing of read-only &files"
"&Zamknout editaci souborů určených jen pro čtení"
"Bearbeiten von &Dateien mit Schreibschutz verhindern"
"Csak olvasható fájlok s&zerkesztése tiltva"
"Nie edytuj plików tylko do odczytu"
"Bloquear edición de archivos de só&lo lectura"
"&Zamknúť úpravy súborov určených len na čítanie"

MEditWarningBeforeOpenROFile
"Пре&дупреждать при открытии файла с атрибутом R/O"
"&Warn when opening read-only files"
"&Varovat při otevření souborů určených jen pro čtení"
"Beim Öffnen von Dateien mit Schreibschutz &warnen"
"Figyelmeztet &csak olvasható fájl megnyitásakor"
"&Ostrzeż przed otwieraniem plików tylko do odczytu"
"Advertencia al abrir archivos de sól&o lectura"
"&Varovať pri otvorení súborov určených len na čítanie"

MEditConfigTabSize
"Раз&мер табуляции"
"Tab si&ze"
"Velikost &Tabulátoru"
"Ta&bulatorgröße"
"Tab&ulátor mérete"
"Rozmiar ta&bulatora"
"&Tamaño de tabulación"
"Veľkosť &tabulátora"

MEditConfigScrollbar
"Показывать &полосу прокрутки"
"Show scro&llbar"
"Zobr&azovat posuvník"
"Scro&llbalken anzeigen"
"&Gördítősáv mutatva"
"Pokaż &pasek przewijania"
"Mostrar barra de despla&zamiento"
"Zobr&azovať posuvník"

MEditShowWhiteSpace
"Пробельные символы"
"Sh&ow white space"
upd:"Show white space"
upd:"Show white space"
upd:"Show white space"
upd:"Show white space"
"Mostrar espacios en bla&nco"
"Show white space"

MEditConfigPickUpWord
"Cлово под к&урсором"
"Pick &up the word"
upd:"Pick &up the word"
upd:"Pick &up the word"
upd:"Pick &up the word"
upd:"Pick &up the word"
"Tomar la &palabra"
"Pick &up the word"

MEditConfigAnsiCodePageAsDefault
"&Использовать кодовую страницу ANSI по умолчанию"
"Use ANS&I code page by default"
upd:"Automaticky otevírat soubory ve &WIN kódování"
upd:"Dateien standardmäßig mit Windows-Kod&ierung öffnen"
"Fájlok eredeti megnyitása ANS&I kódlappal"
"&Próbuj otwierać pliki w kodowaniu Windows"
"Usar página de códigos ANS&I por defecto"
"Automaticky otvárať súbory kódovaní W&IN"

MEditConfigAnsiCodePageForNewFile
"Использо&вать кодовую страницу ANSI при создании файлов"
"Use ANSI code page when c&reating new files"
upd:"V&ytvářet nové soubory ve WIN kódování"
upd:"Neue Dateien mit Windows-Ko&dierung erstellen"
"Új &fájl létrehozása ANSI kódlappal"
"&Twórz nowe pliki w kodowaniu Windows"
"Usar página de códigos ANSI al crear archi&vos nuevos"
"V&ytvárať nové súbory v kódovaní WIN"

MSaveSetupTitle
l:
"Конфигурация"
"Save setup"
"Uložit nastavení"
"Einstellungen speichern"
"Beállítások mentése"
"Zapisz ustawienia"
"Guardar configuración"
"Uloženie nastavení"

MSaveSetupAsk1
"Вы хотите сохранить"
"Do you wish to save"
"Přejete si uložit"
"Wollen Sie die aktuellen Einstellungen"
"Elmenti a jelenlegi"
"Czy chcesz zapisać"
"Desea guardar la configuración"
"Chcete uložiť"

MSaveSetupAsk2
"текущую конфигурацию?"
"current setup?"
"aktuální nastavení?"
"speichern?"
"beállításokat?"
"bieżące ustawienia?"
"actual de FAR?"
"aktuálne nastavenia?"

MSaveSetup
"Сохранить"
"Save"
"Uložit"
"Speichern"
"Mentés"
"Zapisz"
"Guardar"
"Uložiť"

MCopyDlgTitle
l:
"Копирование"
"Copy"
"Kopírovat"
"Kopieren"
"Másolás"
"Kopiuj"
"Copiar"
"Kopírovanie"

MMoveDlgTitle
"Переименование/Перенос"
"Rename/Move"
"Přejmenovat/Přesunout"
"Verschieben/Umbenennen"
"Átnevezés-Mozgatás"
"Zmień nazwę/przenieś"
"Renombrar/Mover"
"Premenovanie/Presunutie"

MLinkDlgTitle
"Ссылка"
"Link"
"Link"
"Link erstellen"
"Link létrehozása"
"Dowiąż"
"Enlace"
"Prepojenie"

MCopySecurity
"П&рава доступа:"
"&Access rights:"
"&Přístupová práva:"
"Zugriffsrecht&e:"
"Hozzáférési &jogok:"
"&Prawa dostępu:"
"&Permisos de acceso:"
"&Prístupové práva:"

MCopySecurityCopy
"Копироват&ь"
"Co&py"
"&Kopírovat"
"Ko&pieren"
"Más&ol"
"Kopiu&j"
"Cop&iar"
"S&kopírovať"

MCopySecurityInherit
"Нас&ледовать"
"&Inherit"
"&Zdědit"
"Ve&rerben"
"Ö&rököl"
"&Dziedzicz"
"H&eredar"
"&Zdediť"

MCopySecurityLeave
"По умол&чанию"
"Defau&lt"
"Vých&ozí"
"A&utomat."
"Ala&pért."
"Do&myślne"
"Por defecto"
"Vých&odiskové"

MCopyIfFileExist
"Уже су&ществующие файлы:"
"Already e&xisting files:"
"Již e&xistující soubory:"
"&Dateien überschreiben:"
"Már &létező fájloknál:"
"Dla już &istniejących:"
"Archivos ya e&xistentes:"
"Už e&xistujúce súbory:"

MCopyAsk
"&Запрос действия"
"&Ask"
"Ptát s&e"
"Fr&agen"
"Kér&dez"
"&Zapytaj"
"Pregunt&ar"
"&Opýtať sa"

MCopyAskRO
"Запрос подтверждения для &R/O файлов"
"Also ask on &R/O files"
"Ptát se také na &R/O soubory"
"Bei Dateien mit Sch&reibschutz fragen"
"&Csak olvasható fájloknál is kérdez"
"&Pytaj także o pliki tylko do odczytu"
"Preguntar también archivos Sólo Lectu&ra"
"Opýtať sa aj pri súboroch &R/O"

MCopyOnlyNewerFiles
"Только &новые/обновлённые файлы"
"Only ne&wer file(s)"
"Pouze &novější soubory"
"Nur &neuere Dateien"
"Cs&ak az újabb fájlokat"
"Tylko &nowsze pliki"
"Sólo archivo(s) más nuev&os"
"Len &novšie súbory"

MLinkType
"&Тип ссылки:"
"Link t&ype:"
"&Typ linku:"
"Linkt&yp:"
"Link &típusa:"
"&Typ linku:"
"Tipo de &enlace"
"&Typ prepojenia:"

MLinkTypeJunction
"&связь каталогов"
"directory &junction"
"křížení a&dresářů"
"Ordner&knotenpunkt"
"Mappa &csomópont"
"directory &junction"
"unión de directorio"
"kríženie pr&iečinkov"

MLinkTypeHardlink
"&жёсткая ссылка"
"&hard link"
"&pevný link"
"&Hardlink"
"&Hardlink"
"link &trwały"
"enlace duro"
"&pevné prepojenie"

MLinkTypeSymlink
"си&мволическая ссылка"
"&symbolic link"
"symbolický link"
"Symbolischer Link"
"Szimbolikus link"
"link symboliczny"
"enlace simbólico"
"symbolické prepojenie"

MLinkTypeSymlinkFile
"символическая ссылка (&файл)"
"symbolic link (&file)"
"symbolický link (&soubor)"
"Symbolischer Link (&Datei)"
"Szimbolikus link (&fájl)"
"link symboliczny (do &pliku)"
"enlace simbólico (&archivo)"
"symbolické prepoj. (&súbor)"

MLinkTypeSymlinkDirectory
"символическая ссылка (&папка)"
"symbolic link (fol&der)"
"symbolický link (&adresář)"
"Symbolischer Link (Or&dner)"
"Szimbolikus link (&mappa)"
"link symboliczny (do &folderu)"
"enlace simbólico (&directorios)"
"symbolické prepoj. (prie&činok)"

MCopySymLinkContents
"Копировать содерж&имое символических ссылок"
"Cop&y contents of symbolic links"
"Kopírovat obsah sym&bolických linků"
"Inhalte von s&ymbolischen Links kopieren"
"Sz&imbolikus linkek másolása"
"&Kopiuj zawartość linków symbolicznych"
"Copiar contenidos de enlaces simbólicos"
"Kopírovať obsah sym&bolic. prepoj."

MCopyMultiActions
"Обр&абатывать несколько имён файлов"
"Process &multiple destinations"
"&Zpracovat více míst určení"
"&Mehrere Ziele verarbeiten"
"Tö&bbszörös cél létrehozása"
"Przetwarzaj &wszystkie cele"
"Procesar &múltiples destinos"
"Spra&covať viacero miest určenia"

MCopyDlgCopy
"&Копировать"
"&Copy"
"&Kopírovat"
"&Kopieren"
"&Másolás"
"&Kopiuj"
"&Copiar"
"S&kopíruj"

MCopyDlgTree
"F10-&Дерево"
"F10-&Tree"
"F10-&Strom"
"F10-&Baum"
"F10-&Fa"
"F10-&Drzewo"
"F10-&Arbol"
"F10-&Strom"

MCopyDlgCancel
"&Отменить"
"Ca&ncel"
"&Storno"
"Ab&bruch"
"Még&sem"
"&Anuluj"
"Ca&ncelar"
"Zruši&ť"

MCopyDlgRename
"&Переименовать"
"&Rename"
"Přej&menovat"
"&Umbenennen"
"Át&nevez-Mozgat"
"&Zmień nazwę"
"&Renombrar"
"Pre&menuj"

MCopyDlgLink
"&Создать ссылку"
"Create &link"
upd:"Create &link"
upd:"Create &link"
upd:"Create &link"
upd:"Create &link"
"Crear &enlace"
"Vytvor prepo&jenie"

MCopyDlgTotal
"Всего"
"Total"
"Celkem"
"Gesamt"
"Összesen"
"Razem"
"Total"
"Spolu"

MCopyScanning
"Сканирование папок..."
"Scanning folders..."
"Načítání adresářů..."
"Scanne Ordner..."
"Mappák olvasása..."
"Przeszukuję katalogi..."
"Explorando directorios..."
"Načítavam priečinky..."

MCopyPrepareSecury
"Применение прав доступа..."
"Applying access rights..."
"Nastavuji přístupová práva..."
"Anwenden der Zugriffsrechte..."
"Hozzáférési jogok alkalmazása..."
"Ustawianie praw dostępu..."
"Aplicando derechos de acceso..."
"Aplikujem prístupové práva..."

MCopyUseFilter
"Исполь&зовать фильтр"
"&Use filter"
"P&oužít filtr"
"Ben&utze Filter"
"Szűrő&vel"
"&Użyj filtra"
"&Usar filtros"
"Pou&žiť filter"

MCopySetFilter
"&Фильтр"
"Filt&er"
"Filt&r"
"Filt&er"
"S&zűrő"
"Filt&r"
"Fi&ltro"
"Filte&r"

MCopyFile
l:
"Копировать"
"Copy"
"Kopírovat"
"Kopiere"
upd:"másolása"
"Skopiuj"
"Copiar"
"Kopírovať"

MMoveFile
"Переименовать или перенести"
"Rename or move"
"Přejmenovat nebo přesunout"
"Verschiebe"
upd:"átnevezése-mozgatása"
"Zmień nazwę lub przenieś"
"Renombrar o mover"
"Premenovať alebo presunúť"

MLinkFile
"Создать ссылку на"
"Create link to"
upd:"Create link to"
upd:"Create link to"
upd:"Create link to"
upd:"Create link to"
"Crear enlace a"
"Vytvoriť prepojenie na"

MCopyFiles
"Копировать %1 элемент%2"
"Copy %1 item%2"
"Kopírovat %1 polož%2"
"Kopiere %1 Objekt%2"
" %1 elem másolása%2"
"Skopiuj %1 plików%2"
"Copiar %1 ítem%2"
"Kopírovať %1 polož%2"

MMoveFiles
"Переименовать или перенести %1 элемент%2"
"Rename or move %1 item%2"
"Přejmenovat nebo přesunout %1 polož%2"
"Verschiebe %1 Objekt%2"
" %1 elem átnevezése-mozgatása%2"
"Zmień nazwę lub przenieś %1 plików%2"
"Renombrar o mover %1 ítem%2"
"Premenovať alebo presunúť %1 polož%2"

MLinkFiles
"Создать ссылки на %1 элемент%2"
"Create links to %1 item%2"
upd:"Create links to %1 item%2"
upd:"Create links to %1 item%2"
upd:"Create links to %1 item%2"
upd:"Create links to %1 item%2"
"Crear enlaces a %1 ítem%2"
"Vytvoriť prepojenia na %1 polož%2"

MCMLTargetTO
" &в:"
" t&o:"
" d&o:"
" na&ch:"
" ide:"
" d&o:"
" &hacia:"
" d&o:"

MCMLTargetIN
" &в:"
" in:"
upd:" &in:"
upd:" &in:"
upd:" &in:"
upd:" &in:"
" &en:"
" &v:"

MCMLItems0
""
""
"u"
""
""
""
""
"ku"

MCMLItemsA
"а"
"s"
"ek"
"e"
""
"s"
"s"
"iek"

MCMLItemsS
"ов"
"s"
"ky"
"e"
""
"s"
"s"
"ky"

MCopyIncorrectTargetList
l:
"Указан некорректный список целей"
"Incorrect target list"
"Nesprávný seznam cílů"
"Ungültige Liste von Zielen"
"Érvénytelen céllista"
"Błędna lista wynikowa"
"Lista destino incorrecta"
"Nesprávny zoznam cieľov"

MCopyCopyingTitle
l:
"Копирование"
"Copying"
"Kopíruji"
"Kopieren"
"Másolás"
"Kopiowanie"
"Copiando"
"Kopírujem"

MCopyMovingTitle
"Перенос"
"Moving"
"Přesouvám"
"Verschieben"
"Mozgatás"
"Przenoszenie"
"Moviendo"
"Premiestňujem"

MCopyCannotFind
l:
"Файл не найден"
"Cannot find the file"
"Nelze nalézt soubor"
"Folgende Datei kann nicht gefunden werden:"
"A fájl nem található:"
"Nie mogę odnaleźć pliku"
"No se puede encontrar el archivo"
"Nemôžem nájsť súbor"

MCannotCopyFolderToItself1
l:
"Нельзя копировать папку"
"Cannot copy the folder"
"Nelze kopírovat adresář"
"Folgender Ordner kann nicht kopiert werden:"
"A mappa:"
"Nie można skopiować katalogu"
"No se puede copiar el directorio"
"Nemôžem skopírovať priečinok"

MCannotCopyFolderToItself2
"в саму себя"
"onto itself"
"sám na sebe"
"Ziel und Quelle identisch."
"nem másolható önmagába/önmagára"
"do niego samego"
"en sí mismo"
"sám do seba"

MCannotCopyToTwoDot
l:
"Нельзя копировать файл или папку"
"You may not copy files or folders"
"Nelze kopírovat soubory nebo adresáře"
"Kopieren von Dateien oder Ordnern ist maximal"
"Nem másolhatja a fájlt vagy mappát"
"Nie można skopiować plików"
"Usted no puede copiar archivos o directorios"
"Nemôžete kopírovať súbory alebo priečinky"

MCannotMoveToTwoDot
"Нельзя перемещать файл или папку"
"You may not move files or folders"
"Nelze přesunout soubory nebo adresáře"
"Verschieben von Dateien oder Ordnern ist maximal"
"Nem mozgathatja a fájlt vagy mappát"
"Nie można przenieść plików"
"Usted no puede mover archivos o directorios"
"Nemôžete presunúť súbory alebo priečinky"

MCannotCopyMoveToTwoDot
"выше корневого каталога"
"higher than the root folder"
"na vyšší úroveň než kořenový adresář"
"bis zum Wurzelverzeichnis möglich."
"a gyökér fölé"
"na poziom wyższy niż do korzenia"
"más alto que el directorio raíz"
"vyššie než do koreňového priečinka"

MCopyCannotCreateFolder
l:
"Ошибка создания папки"
"Cannot create the folder"
"Nelze vytvořit adresář"
"Folgender Ordner kann nicht erstellt werden:"
"A mappa nem hozható létre:"
"Nie udało się utworzyć katalogu"
"No se puede crear el directorio"
"Nemôžem vytvoriť priečinok"

MCopyCannotChangeFolderAttr
"Невозможно установить атрибуты для папки"
"Failed to set folder attributes"
"Nastavení atributů adresáře selhalo"
"Fehler beim Setzen der Ordnerattribute"
"A mappa attribútumok beállítása sikertelen"
"Nie udało się ustawić atrybutów folderu"
"Error al poner atributos en directorio"
"Nepodarilo sa nastaviť atribúty priečinka"

MCopyCannotRenameFolder
"Невозможно переименовать папку"
"Cannot rename the folder"
"Nelze přejmenovat adresář"
"Folgender Ordner kann nicht umbenannt werden:"
"A mappa nem nevezhető át:"
"Nie udało się zmienić nazwy katalogu"
"No se puede renombrar el directorio"
"Nemôžem premenovať priečinok"

MCopyIgnore
"&Игнорировать"
"&Ignore"
"&Ignorovat"
"&Ignorieren"
"Mé&gis"
"&Ignoruj"
"&Ignorar"
"&Ignorovať"

MCopyIgnoreAll
"Игнорировать &все"
"Ignore &All"
"Ignorovat &vše"
"&Alle ignorieren"
"Min&d"
"Ignoruj &wszystko"
"Ignorar &Todo"
"Ignorovať &všetko"

MCopyRetry
"&Повторить"
"&Retry"
"&Opakovat"
"Wiede&rholen"
"Ú&jra"
"&Ponów"
"&Reiterar"
"Z&opakovať"

MCopySkip
"П&ропустить"
"&Skip"
"&Přeskočit"
"Ausla&ssen"
"&Kihagy"
"&Omiń"
"&Omitir"
"&Preskočiť"

MCopySkipAll
"&Пропустить все"
"S&kip all"
"Př&eskočit vše"
"Alle aus&lassen"
"Mi&nd"
"Omiń w&szystkie"
"O&mitir todos"
"Pr&eskočiť všetko"

MCopyCancel
"&Отменить"
"&Cancel"
"&Storno"
"Abb&rechen"
"Még&sem"
"&Anuluj"
"&Cancelar"
"&Zrušiť"

MCopyCannotCreateLink
l:
"Ошибка создания ссылки"
"Cannot create the link"
"Nelze vytvořit symbolický link"
"Folgender Link kann nicht erstellt werden:"
"A link nem hozható létre:"
"Nie udało się utworzyć linku"
"No se puede crear el enlace simbólico"
"Nemôžem vytvoriť symbolické prepojenie"

MCopyFolderNotEmpty
"Папка назначения должна быть пустой"
"Target folder must be empty"
"Cílový adresář musí být prázdný"
"Zielordner muss leer sein."
"A célmappának üresnek kell lennie"
"Folder wynikowy musi być pusty"
"Directorio destino debe estar vacío"
"Cieľový priečinok musí byť prázdny"

MCopyCannotCreateJunctionToFile
"Невозможно создать связь. Файл уже существует:"
"Cannot create junction. The file already exists:"
"Nelze vytvořit křížový odkaz. Soubor již existuje:"
"Knotenpunkt wurde nicht erstellt. Datei existiert bereits:"
"A csomópont nem hozható létre. A fájl már létezik:"
"Nie można utworzyć połączenia - plik już istnieje:"
"No se puede unir. El archivo ya existe:"
"Nemôžem vytvoriť krížový odkaz. Súbor už existuje:"

MCopyCannotCreateVolMount
l:
"Ошибка монтирования диска"
"Volume mount points error"
"Chyba připojovacích svazků"
"Fehler im Mountpoint des Datenträgers"
"Kötet mountpont hiba"
"Błąd montowania napędu"
"Error en puntos de montaje de volumen"
"Chyba pripojovania"

MCopyMountVolFailed
"Ошибка при монтировании диска '%1'"
"Attempt to volume mount '%1'"
"Pokus o připojení svazku '%1'"
"Versuch Datenträger '%1' zu aktivieren"
""%1" kötet mountolása"
"Nie udało się zamontować woluminu '%1'"
"Intento de montaje de volumen '%1'"
"Pokus o pripojenie '%1'"

MCopyMountVolFailed2
"на '%1'"
"at '%1' failed"
"na '%1' selhal"
"fehlgeschlagen bei '%1'"
"nem sikerült: "%1""
"w '%1' nie udało się"
"a '%1' ha fallado"
"na '%1' zlyhal"

MCopyMountName
"Disk_"
"Disk_"
"Disk_"
"Disk_"
"Disk_"
"Disk_"
"Disco_"
"Disk_"

MCannotCopyFileToItself1
l:
"Нельзя копировать файл"
"Cannot copy the file"
"Nelze kopírovat soubor"
"Folgende Datei kann nicht kopiert werden:"
"A fájl"
"Nie można skopiować pliku"
"Imposible copiar el archivo"
"Nemôžem kopírovať súbor"

MCannotCopyFileToItself2
"в самого себя"
"onto itself"
"sám na sebe"
"Ziel und Quelle identisch."
"nem másolható önmagára"
"do niego samego"
"en sí mismo"
"sám na seba"

MCopyStream1
l:
"Исходный файл содержит более одного потока данных,"
"The source file contains more than one data stream."
"Zdrojový soubor obsahuje více než jeden datový proud."
"Die Quelldatei enthält mehr als einen Datenstream"
"A forrásfájl több stream-et tartalmaz,"
"Plik źródłowy zawiera więcej niż jeden strumień danych."
"El archivo origen contiene más de un flujo de datos."
"Zdrojový súbor obsahuje viac než jeden dátový prúd."

MCopyStream2
"но вы не используете системную функцию копирования."
"but since you do not use a system copy routine."
"protože nepoužíváte systémovou kopírovací rutinu."
"aber Sie verwenden derzeit nicht die systemeigene Kopierroutine."
"de nem a rendszer másolórutinját használja."
"ale ze względu na rezygnację z systemowej procedury kopiowania."
"pero desde que usted no usa la rutina de copia del sistema."
"lebo nepoužívate systémovú kopírovaciu rutinu."

MCopyStream3
"но том назначения не поддерживает этой возможности."
"but the destination volume does not support this feature."
"protože cílový svazek nepodporuje tuto vlastnost."
"aber der Zieldatenträger unterstützt diese Fähigkeit nicht."
"de a célkötet nem támogatja ezt a lehetőséget."
"ale napęd docelowy nie obsługuje tej funkcji."
"pero el volumen de destino no soporta esta opción."
"lebo cieľová jednotka nepodporuje túto funkciu."

MCopyStream4
"Часть сведений не будет сохранена."
"Some data will not be preserved as a result."
"To bude mít za následek, že některá data nebudou uchována."
"Ein Teil der Daten bleiben daher nicht erhalten."
"Az adatok egy része el fog veszni."
"Nie wszystkie dane zostaną zachowane."
"Algunos datos no serán preservados como un resultado."
"To bude mať za následok, že sa niekteré dáta nezachovajú."

MCopyDirectoryOrFile
l:
"Подразумевается имя папки или файла?"
"Does it specify a folder name or file name?"
upd:"Does it specify a folder name or file name?"
upd:"Does it specify a folder name or file name?"
upd:"Does it specify a folder name or file name?"
upd:"Does it specify a folder name or file name?"
"Si especifica nombre de carpeta o nombre de archivo?"
"Je to názov priečinka alebo súboru?"

MCopyDirectoryOrFileDirectory
"Папка"
"Folder"
upd:"Folder"
upd:"Folder"
upd:"Folder"
upd:"Folder"
"Carpeta"
"Priečinka"

MCopyDirectoryOrFileFile
"Файл"
"File"
upd:"File"
upd:"File"
upd:"File"
upd:"File"
"Archivo"
"Súboru"

MCopyFileExist
l:
"Файл уже существует"
"File already exists"
"Soubor již existuje"
"Datei existiert bereits"
"A fájl már létezik:"
"Plik już istnieje"
"El archivo ya existe"
"Súbor už existuje"

MCopySource
"&Новый"
"&New"
"&Nový"
"&Neue Datei"
"Ú&j verzió:"
"&Nowy"
"&Nuevo"
"&Nový"

MCopyDest
"Су&ществующий"
"E&xisting"
"E&xistující"
"Be&stehende Datei"
"Létező &verzió:"
"&Istniejący"
"E&xistente"
"E&xistujúci"

MCopyOverwrite
"В&место"
"&Overwrite"
"&Přepsat"
"Über&schr."
"&Felülír"
"N&adpisz"
"&Sobrescribir"
"&Prepísať"

MCopySkipOvr
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&spr."
"&Kihagy"
"&Omiń"
"&Omitir"
"Pres&kočiť"

MCopyAppend
"&Дописать"
"A&ppend"
"Př&ipojit"
"&Anhängen"
"Hoz&záfűz"
"&Dołącz"
"A&gregar"
"Pr&ipojiť"

MCopyResume
"Возоб&новить"
"&Resume"
"Po&kračovat"
"&Weiter"
"Fol&ytat"
"Ponó&w"
"&Resumir"
"Pok&račovať"

MCopyRename
"&Имя"
"R&ename"
upd:"R&ename"
upd:"R&ename"
"Á&tnevez"
upd:"R&ename"
"Renombrar"
"Pr&emenovať"

MCopyCancelOvr
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Mégsem"
"&Anuluj"
"&Cancelar"
"&Zrušiť"

MCopyRememberChoice
"&Запомнить выбор"
"&Remember choice"
"Zapama&tovat volbu"
"Auswahl me&rken"
"Mind&ent a kiválasztott módon"
"&Zapamiętaj ustawienia"
"&Recordar elección"
"Zapamä&tať si voľbu"

MCopyRenameTitle
"Переименование"
"Rename"
upd:"Rename"
upd:"Rename"
"Átnevezés"
upd:"Rename"
"Renombrar"
"Premenovať"

MCopyRenameText
"&Новое имя:"
"&New name:"
upd:"&New name:"
upd:"&New name:"
"Ú&j név:"
upd:"&New name:"
"&Nuevo nombre:"
"N&ový názov:"

MCopyFileRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"
"Soubor je určen pouze pro čtení"
"Folgende Datei ist schreibgeschützt:"
"A fájl csak olvasható:"
"Ten plik jest tylko-do-odczytu"
"El archivo es de sólo lectura"
"Súbor je len na čítanie"

MCopyAskDelete
"Вы хотите удалить его?"
"Do you wish to delete it?"
"Opravdu si ho přejete smazat?"
"Wollen Sie sie dennoch löschen?"
"Biztosan törölni akarja?"
"Czy chcesz go usunąć?"
"Desea borrarlo igual?"
"Chcete ho zmazať?"

MCopyDeleteRO
"&Удалить"
"&Delete"
"S&mazat"
"&Löschen"
"&Törli"
"&Usuń"
"&Borrar"
"Z&mazať"

MCopyDeleteAllRO
"&Все"
"&All"
"&Vše"
"&Alle Löschen"
"Min&det"
"&Wszystkie"
"&Todos"
"&Všetko"

MCopySkipRO
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&springen"
"&Kihagyja"
"&Omiń"
"&Omitir"
"Pres&kočiť"

MCopySkipAllRO
"П&ропустить все"
"S&kip all"
"Př&eskočit vše"
"A&lle überspringen"
"Mind&et"
"O&miń wszystkie"
"O&mitir todos"
"Pr&eskočiť všetky"

MCopyCancelRO
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Mégsem"
"&Anuluj"
"&Cancelar"
"&Storno"

MCannotCopy
l:
"Ошибка копирования"
"Cannot copy"
"Nelze kopírovat"
"Konnte nicht kopieren"
"Nem másolható"
"Nie mogę skopiować"
"No se puede copiar"
"Nemôžem kopírovať"

MCannotMove
"Ошибка переноса"
"Cannot move"
"Nelze přesunout"
"Konnte nicht verschieben"
"Nem mozgatható"
"Nie mogę przenieść"
"No se puede mover"
"Nemôžem presunúť"

MCannotLink
"Ошибка создания ссылки"
"Cannot link"
"Nelze linkovat"
"Konnte nicht verlinken"
"Nem linkelhető"
"Nie mogę dowiązać"
"No se puede enlazar"
"Nemôžem prepojiť"

MCannotCopyTo
"в"
"to"
"do"
"nach"
"ide:"
"do"
"hacia"
"do"

MCopyEncryptWarn1
"Файл"
"The file"
"Soubor"
"Die Datei"
"A fájl"
"Plik"
"El archivo"
"Súbor"

MCopyEncryptWarn2
"нельзя скопировать или переместить, не потеряв его шифрование."
"cannot be copied or moved without losing its encryption."
"nemůže být zkopírován nebo přesunut bez ztráty jeho šifrování."
"kann nicht bewegt werden ohne ihre Verschlüsselung zu verlieren."
"csak titkosítása elvesztésével másolható vagy mozgatható."
"nie może zostać skopiowany/przeniesiony bez utraty szyfrowania"
"no puede copiarse o moverse sin perder el cifrado."
"sa nedá skopírovať ani presunúť bez straty jeho šifrovania."

MCopyEncryptWarn3
"Можно пропустить эту ошибку или отменить операцию."
"You can choose to ignore this error and continue, or cancel."
"Můžete tuto chybu ignorovat a pokračovat, nebo operaci ukončit."
"Sie können dies ignorieren und fortfahren oder abbrechen."
"Ennek ellenére folytathatja vagy felfüggesztheti."
"Możesz zignorować błąd i kontynuować lub anulować operację."
"Usted puede ignorar este error y continuar, o cancelar."
"Môžete túto chybu ignorovať a pokračovať, alebo operáciu zrušiť."

MCopyReadError
l:
"Ошибка чтения данных из"
"Cannot read data from"
"Nelze číst data z"
"Kann Daten nicht lesen von"
"Nem olvasható adat innen:"
"Nie mogę odczytać danych z"
"No se puede leer datos desde"
"Nemôžem čítať dáta z"

MCopyWriteError
"Ошибка записи данных в"
"Cannot write data to"
"Nelze zapsat data do"
"Dann Daten nicht schreiben in"
"Nem írható adat ide:"
"Nie mogę zapisać danych do"
"No se puede escribir datos hacia"
"Nemôžem zapísať dáta do"

MCopyProcessed
l:
"Обработано файлов: %1"
"Files processed: %1"
"Zpracováno souborů: %1"
"Dateien verarbeitet: %1"
" %1 fájl kész"
"Przetworzonych plików: %1"
"Archivos procesados: %1"
"Spracované súbory: %1"

MCopyProcessedTotal
"Обработано файлов: %1 из %2"
"Files processed: %1 of %2"
"Zpracováno souborů: %1 z %2"
"Dateien verarbeitet: %1 von %2"
" %1 fájl kész %2 fájlból"
"Przetworzonych plików: %1 z %2"
"Archivos procesados: %1 de %2"
"Spracované súbory: %1 z %2"

MCopyMoving
"Перенос файла"
"Moving the file"
"Přesunuji soubor"
"Verschiebe die Datei"
"Fájl mozgatása"
"Przenoszę plik"
"Moviendo el archivo"
"Presúvam súbor"

MCopyCopying
"Копирование файла"
"Copying the file"
"Kopíruji soubor"
"Kopiere die Datei"
"Fájl másolása"
"Kopiuję plik"
"Copiando el archivo"
"Kopírujem súbor"

MCopyTo
"в"
"to"
"do"
"nach"
"ide:"
"do"
"Hacia"
"do"

MCopyErrorDiskFull
l:
"Диск заполнен. Вставьте следующий"
"Disk full. Insert next"
"Disk je plný. Vložte dalšíí"
"Datenträger voll. Bitte nächsten einlegen"
"A lemez megtelt, kérem a következőt"
"Dysk pełny. Włóż następny"
"Disco lleno. Inserte el próximo"
"Disk je plný. Vložte ďalší"

MDeleteTitle
l:
"Удаление"
"Delete"
"Smazat"
"Löschen"
"Törlés"
"Usuń"
"Borrar"
"Zmazať"

MAskDeleteFolder
"Вы хотите удалить папку"
"Do you wish to delete the folder"
"Přejete si smazat adresář"
"Wollen Sie den Ordner löschen"
"Törölni akarja a mappát?"
"Czy chcesz wymazać katalog"
"Desea borrar el directorio"
"Chcete zmazať priečinok"

MAskDeleteFile
"Вы хотите удалить файл"
"Do you wish to delete the file"
"Přejete si smazat soubor"
"Wollen Sie die Datei löschen"
"Törölni akarja a fájlt?"
"Czy chcesz usunąć plik"
"Desea borrar el archivo"
"Chcete zmazať súbor"

MAskDelete
"Вы хотите удалить"
"Do you wish to delete"
"Přejete si smazat"
"Wollen Sie folgendes Objekt löschen"
"Törölni akar"
"Czy chcesz usunąć"
"Desea borrar"
"Chcete zmazať"

MAskDeleteRecycleFolder
"Вы хотите переместить в Корзину папку"
"Do you wish to move to the Recycle Bin the folder"
"Přejete si přesunout do Koše adresář"
"Wollen Sie den Ordner in den Papierkorb verschieben"
"A Lomtárba akarja dobni a mappát?"
"Czy chcesz przenieść katalog do Kosza"
"Desea mover hacia la papelera de reciclaje el directorio"
"Chcete presunúť do Koša priečinok"

MAskDeleteRecycleFile
"Вы хотите переместить в Корзину файл"
"Do you wish to move to the Recycle Bin the file"
"Přejete si přesunout do Koše soubor"
"Wollen Sie die Datei in den Papierkorb verschieben"
"A Lomtárba akarja dobni a fájlt?"
"Czy chcesz przenieść plik do Kosza"
"Desea mover hacia la papelera de reciclaje el archivo"
"Chcete presunúť do Koša súbor"

MAskDeleteRecycle
"Вы хотите переместить в Корзину"
"Do you wish to move to the Recycle Bin"
"Přejete si přesunout do Koše"
"Wollen Sie das Objekt in den Papierkorb verschieben"
"A Lomtárba akar dobni"
"Czy chcesz przenieść do Kosza"
"Desea mover hacia la papelera de reciclaje"
"Chcete presunúť do Koša"

MDeleteWipeTitle
"Уничтожение"
"Wipe"
"Vymazat"
"Sicheres Löschen"
"Kisöprés"
"Wymaż"
"Destruir"
"Vymazať"

MAskWipeFolder
"Вы хотите уничтожить папку"
"Do you wish to wipe the folder"
"Přejete si vymazat adresář"
"Wollen Sie den Ordner sicher löschen"
"Ki akarja söpörni a mappát?"
"Czy chcesz wymazać katalog"
"Desea destruir el directorio"
"Chcete vymazať priečinok"

MAskWipeFile
"Вы хотите уничтожить файл"
"Do you wish to wipe the file"
"Přejete si vymazat soubor"
"Wollen Sie die Datei sicher löschen"
"Ki akarja söpörni a fájlt?"
"Czy chcesz wymazać plik"
"Desea destruir el archivo"
"Chcete vymazať súbor"

MAskWipe
"Вы хотите уничтожить"
"Do you wish to wipe"
"Přejete si vymazat"
"Wollen Sie das Objekt sicher löschen"
"Ki akar söpörni"
"Czy chcesz wymazać"
"Desea destruir"
"Chcete vymazať"

MDeleteLinkTitle
"Удаление ссылки"
"Delete link"
"Smazat link"
"Link löschen"
"Link törlése"
"Usuń link"
"Borrar enlace"
"Zmazať prepojenie"

MAskDeleteLink
"является ссылкой на"
"is a link to"
"je link na"
"ist ein Link auf"
"linkelve ide:"
"jest linkiem do"
"es un enlace al"
"je prepojenie na"

MAskDeleteLinkFolder
"папку"
"folder"
"adresář"
"Ordner"
"mappa"
"folder"
"directorio"
"priečinok"

MAskDeleteLinkFile
"файл"
"file"
"soubor"
"Date"
"fájl"
"plik"
"archivo"
"súbor"

MAskDeleteItems
"%1 элемент%2"
"%1 item%2"
"%1 polož%2"
"%1 Objekt%2"
"%1 elemet%2"
"%1 plik%2"
"%1 ítem%2"
"%1 polož%2"

MAskDeleteItems0
""
""
"ku"
""
""
""
""
"ku"

MAskDeleteItemsA
"а"
"s"
"ky"
"e"
""
"i"
"s"
"ky"

MAskDeleteItemsS
"ов"
"s"
"ek"
"e"
""
"ów"
"s"
"iek"

MDeleteFolderTitle
l:
"Удаление папки "
"Delete folder"
"Smazat adresář"
"Ordner löschen"
"Mappa törlése"
"Usuń folder"
"Borrar directorio"
"Zmazať priečinok"

MWipeFolderTitle
"Уничтожение папки "
"Wipe folder"
"Vymazat adresář"
"Ordner sicher löschen"
"Mappa kisöprése"
"Wymaż folder"
"Destruir directorio"
"Vymazať priečinok"

MDeleteFilesTitle
"Удаление файлов"
"Delete files"
"Smazat soubory"
"Dateien löschen"
"Fájlok törlése"
"Usuń pliki"
"Borrar archivos"
"Zmazať súbory"

MWipeFilesTitle
"Уничтожение файлов"
"Wipe files"
"Vymazat soubory"
"Dateien sicher löschen"
"Fájlok kisöprése"
"Wymaż pliki"
"Destruir archivos"
"Vymazať súbory"

MDeleteFolderConfirm
"Данная папка будет удалена:"
"The following folder will be deleted:"
"Následující adresář bude smazán:"
"Folgender Ordner wird gelöscht:"
"A mappa törlődik:"
"Następujący folder zostanie usunięty:"
"El siguiente directorio será borrado:"
"Nasledujúci priečinok bude zmazaný:"

MWipeFolderConfirm
"Данная папка будет уничтожена:"
"The following folder will be wiped:"
"Následující adresář bude vymazán:"
"Folgender Ordner wird sicher gelöscht:"
"A mappa kisöprődik:"
"Następujący folder zostanie wymazany:"
"El siguiente directorio será destruído:"
"Nasledujúci priečinok bude vymazaný:"

MDeleteWipe
"Уничтожить"
"Wipe"
"Vymazat"
"Sicheres Löschen"
"Kisöpör"
"Wymaż"
"Limpiar"
"Vymazať"

MDeleteRecycle
"Переместить"
"Move"
upd:"Move"
upd:"Move"
upd:"Move"
upd:"Move"
"Mover"
"Presunúť"

MDeleteFileDelete
"&Удалить"
"&Delete"
"S&mazat"
"&Löschen"
"&Töröl"
"&Usuń"
"&Borrar"
"Z&mazať"

MDeleteFileWipe
"&Уничтожить"
"&Wipe"
"V&ymazat"
"&Sicher löschen"
"Kisö&pör"
"&Wymaż"
"Destruir"
"V&ymazať"

MDeleteFileAll
"&Все"
"&All"
"&Vše"
"&Alle"
"Min&det"
"&wszystkie"
"&Todos"
"&Všetky"

MDeleteFileSkip
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&springen"
"&Kihagy"
"&Omiń"
"&Omitir"
"Pres&kočiť"

MDeleteFileSkipAll
"П&ропустить все"
"S&kip all"
"Př&eskočit vše"
"A&lle überspr."
"Mind&et"
"O&miń wszystkie"
"O&mitir todos"
"Pr&eskočiť všetky"

MDeleteFileCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Mégsem"
"&Anuluj"
"&Cancelar"
"&Storno"

MDeleteLinkDelete
l:
"Удалить ссылку"
"Delete link"
"Smazat link"
"Link löschen"
"Link törlése"
"Usuń link"
"Borrar enlace"
"Zmazať prepojenie"

MDeleteLinkUnlink
"Разорвать ссылку"
"Break link"
"Poškozený link"
"Link auflösen"
"Link megszakítása"
"Przerwij link"
"Romper enlace"
"Poškodené prepojenie"

MDeletingTitle
l:
"Удаление"
"Deleting"
"Mazání"
"Lösche"
"Törlés"
"Usuwam"
"Borrando"
"Mazanie"

MDeleting
l:
"Удаление файла или папки"
"Deleting the file or folder"
"Mazání souboru nebo adresáře"
"Löschen von Datei oder Ordner"
"Fájl vagy mappa törlése"
"Usuwam plik/katalog"
"Borrando el archivo o directorio"
"Mazanie súboru alebo priečinka"

MDeletingWiping
"Уничтожение файла или папки"
"Wiping the file or folder"
"Vymazávání souboru nebo adresáře"
"Sicheres löschen von Datei oder Ordner"
"Fájl vagy mappa kisöprése"
"Wymazuję plik/katalog"
"Destruyendo el archivo o directorio"
"Vymazávanie súboru alebo priečinka"

MDeleteRO
l:
"Файл имеет атрибут \"Только для чтения\""
"The file is read only"
"Soubor je určen pouze pro čtení"
"Folgende Datei ist schreibgeschützt:"
"A fájl csak olvasható:"
"Ten plik jest tylko do odczytu"
"El archivo es de sólo lectura"
"Súbor je len na čítanie"

MAskDeleteRO
"Вы хотите удалить его?"
"Do you wish to delete it?"
"Opravdu si ho přejete smazat?"
"Wollen Sie sie dennoch löschen?"
"Mégis törölni akarja?"
"Czy chcesz go usunąć?"
"Desea borrarlo?"
"Chcete ho zmazať?"

MAskWipeRO
"Вы хотите уничтожить его?"
"Do you wish to wipe it?"
"Opravdu si ho přejete vymazat?"
"Wollen Sie sie dennoch sicher löschen?"
"Mégis ki akarja söpörni?"
"Czy chcesz go wymazać?"
"Desea destruirlo?"
"Chcete ho vymazať?"

MDeleteHardLink1
l:
"Файл имеет несколько жёстких ссылок"
"Several hard links link to this file."
"Více pevných linků ukazuje na tento soubor."
"Mehrere Hardlinks zeigen auf diese Datei."
"Több hardlink kapcsolódik a fájlhoz, a fájl"
"Do tego pliku prowadzi wiele linków trwałych."
"Demasiados enlaces rígidos a este archivo."
"Niekoľko pevných prepojení ukazuje na tento súbor."

MDeleteHardLink2
"Уничтожение файла приведёт к обнулению всех ссылающихся на него файлов."
"Wiping this file will void all files linking to it."
"Vymazání tohoto souboru zneplatní všechny soubory, které na něj linkují."
"Sicheres Löschen dieser Datei entfernt ebenfalls alle Links."
"kisöprése a linkelt fájlokat is megsemmisíti."
"Wymazanie tego pliku wymaże wszystkie pliki dolinkowane."
"Limpiando este archivo invalidará todos los archivos enlazados."
"Vymazanie tohto súboru zneplatní všetky súbory, ktoré naň prepojené."

MDeleteHardLink3
"Уничтожать файл?"
"Do you wish to wipe this file?"
"Opravdu chcete vymazat tento soubor?"
"Wollen Sie diese Datei sicher löschen?"
"Biztosan kisöpri a fájlt?"
"Czy wymazać plik?"
"Desea destruir este archivo"
"Naozaj chcete vymazať tento súbor?"

MCannotDeleteFile
l:
"Ошибка удаления файла"
"Cannot delete the file"
"Nelze smazat soubor"
"Datei konnte nicht gelöscht werden"
"A fájl nem törölhető"
"Nie mogę usunąć pliku"
"No se puede borrar el archivo"
"Nemôžem zmazať súbor"

MCannotDeleteFolder
"Ошибка удаления папки"
"Cannot delete the folder"
"Nelze smazat adresář"
"Ordner konnte nicht gelöscht werden"
"A mappa nem törölhető"
"Nie mogę usunąć katalogu"
"No se puede borrar el directorio"
"Nemôžem zmazať priečinok"

MDeleteRetry
"&Повторить"
"&Retry"
"&Znovu"
"Wiede&rholen"
"Ú&jra"
"&Ponów"
"&Reiterar"
"&Znova"

MDeleteSkip
"П&ропустить"
"&Skip"
"Přes&kočit"
"Über&springen"
"&Kihagy"
"Po&miń"
"&Omitir"
"Pres&kočiť"

MDeleteSkipAll
"Пропустить &все"
"S&kip all"
"Přeskočit &vše"
"A&lle überspr."
"Min&d"
"Pomiń &wszystkie"
"Omitir &Todo"
"Preskočiť &všetky"

MDeleteCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Mégsem"
"&Anuluj"
"&Cancelar"
"&Storno"

MCannotGetSecurity
l:
"Ошибка получения прав доступа к файлу"
"Cannot get file access rights for"
"Nemohu získat přístupová práva pro"
"Kann Zugriffsrechte nicht lesen für"
"A fájlhoz nincs hozzáférési joga:"
"Nie mogę pobrać praw dostępu dla"
"No se puede tener permisos de acceso a archivo"
"Nemôžem získať prístupové práva pre"

MCannotSetSecurity
"Ошибка установки прав доступа к файлу"
"Cannot set file access rights for"
"Nemohu nastavit přístupová práva pro"
"Kann Zugriffsrechte nicht setzen für"
"A fájl hozzáférési jogát nem állíthatja:"
"Nie mogę ustawić praw dostępu dla"
"No se puede poner permisos de acceso a archivo"
"Nemôžem nastaviť prístupové práva pre"

MEditTitle
l:
"Редактор"
"Editor"
"Editor"
"Editor"
"Szerkesztő"
"Edytor"
"Editor"
"Editor"

MAskReload
"уже загружен. Как открыть этот файл?"
"already loaded. How to open this file?"
"již otevřen. Jak otevřít tento soubor?"
"bereits geladen. Wie wollen Sie die Datei öffnen?"
"fájl már be van töltve. Hogyan szerkeszti?"
"został już załadowany. Załadować ponownie?"
"ya está cargado. Como abrir este archivo?"
"je už spustený. Ako otvoriť tento súbor?"

MCurrent
"&Текущий"
"&Current"
"&Stávající"
"A&ktuell"
"A mostanit &folytatja"
"&Bieżący"
"A&ctual"
"A&ktuálny"

MReload
"Пере&грузить"
"R&eload"
"&Znovu načíst"
"Aktualisie&ren"
"Újra&tölti"
"&Załaduj"
"R&ecargar"
"&Znova načítať"

MNewOpen
"&Новая копия"
"&New instance"
"&Nová instance"
"&Neue Instanz"
"Ú&j példányban"
"&Nowa instancja"
"&Nueva instancia"
"&Nová inštancia"

MEditCannotOpen
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"
"A fájl nem nyitható meg"
"Nie mogę otworzyć pliku"
"No se puede abrir el archivo"
"Nemôžem otvoriť súbor"

MEditReading
"Чтение файла"
"Reading the file"
"Načítám soubor"
"Lesen der Datei"
"Fájl olvasása"
"Czytam plik"
"Leyendo el archivo"
"Načítavam súbor"

MEditAskSave
"Файл был изменён. Сохранить?"
"File has been modified. Save?"
upd:"Soubor byl modifikován. Save?"
upd:"Datei wurde verändert. Save?"
upd:"A fájl megváltozott. Save?"
upd:"Plik został zmodyfikowany. Save?"
"El archivo ha sido modificado. Desea guardarlo?"
"Súbor bol upravený. Uložiť?"

MEditAskSaveExt
"Файл был изменён внешней программой. Сохранить?"
"The file was changed by an external program. Save?"
upd:"Soubor byl změněný externím programem. Save?"
upd:"Die Datei wurde durch ein externes Programm verändert. Save?"
upd:"A fájlt egy külső program megváltoztatta. Save?"
upd:"Plik został zmieniony przez inny program. Save?"
"El archivo ha sido cambiado por un programa externo. Desea guardarlo?"
"Súbor bol zmenený externým programom. Uložiť?"

MEditBtnSaveAs
"Сохр&анить как..."
"Save &as..."
"Ulož&it jako..."
"Speichern &als..."
"Mentés más&ként..."
"Zapisz &jako..."
"Guardar como..."
"Ulož&iť ako..."

MEditRO
l:
"имеет атрибут \"Только для чтения\""
"is a read-only file"
"je určen pouze pro čtení"
"ist eine schreibgeschützte Datei"
"csak olvasható fájl"
"jest plikiem tylko do odczytu"
"es un archivo de sólo lectura"
"je len na čítanie"

MEditExists
"уже существует"
"already exists"
"již existuje"
"ist bereits vorhanden"
"már létezik"
"już istnieje"
"ya existe"
"už existuje"

MEditOvr
"Вы хотите перезаписать его?"
"Do you wish to overwrite it?"
"Přejete si ho přepsat?"
"Wollen Sie die Datei überschreiben?"
"Felül akarja írni?"
"Czy chcesz go nadpisać?"
"Desea sobrescribirlo?"
"Chcete ho prepísať?"

MEditSaving
"Сохранение файла"
"Saving the file"
"Ukládám soubor"
"Speichere die Datei"
"Fájl mentése"
"Zapisuję plik"
"Guardando el archivo"
"Ukladám súbor"

# 3 max
MEditStatusLine
"Стр"
"Ln"
upd:"Řádek"
upd:"Zeile"
upd:"Sor"
upd:"linia"
"Línea"
"Riadok"

# 3 max
MEditStatusCol
"Кол"
"Col"
upd:"Sloupec"
upd:"Spal"
upd:"Oszlop"
upd:"kolumna"
"Col"
"Stĺpec"

# 2 max
MEditStatusChar
"С"
"Ch"
upd:"Ch"
upd:"Ch"
upd:"Ch"
upd:"Ch"
"Ch"
"Znak"

MEditRSH
l:
"предназначен только для чтения"
"is a read-only file"
"je určen pouze pro čtení"
"ist eine schreibgeschützte Datei"
"csak olvasható fájl"
"jest plikiem tylko do odczytu"
"es un archivo de sólo lectura"
"je súbor len na čítanie"

MEditFileGetSizeError
"Не удалось определить размер."
"File size could not be determined."
upd:"File size could not be determined."
upd:"File size could not be determined."
"A fájlméret megállapíthatatlan."
upd:"File size could not be determined."
"Tamaño de archivo no puede ser determinado"
"Veľkosť súboru sa nedá určiť."

MEditFileLong
"имеет размер %1,"
"has the size of %1,"
"má velikost %1,"
"hat eine Größe von %1,"
"mérete %1,"
"ma wielkość %1,"
"tiene el tamaño de %1,"
"má veľkosť %1,"

MEditFileLong2
"что превышает заданное ограничение в %1."
"which exceeds the configured maximum size of %1."
"která překračuje nastavenou maximální velikost %1."
"die die konfiguierte Maximalgröße von %1 überschreitet."
"meghaladja %1 beállított maximumát."
"przekraczającą ustalone maksimum %1."
"cual excede el tamaño máximo configurado de %1."
"ktorá prekračuje nastavenú maximálnu veľkosť %1."

MEditROOpen
"Вы хотите редактировать его?"
"Do you wish to edit it?"
"Opravdu si ho přejete upravit?"
"Wollen Sie sie dennoch bearbeiten?"
"Mégis szerkeszti?"
"Czy chcesz go edytować?"
"Desea editarlo?"
"Chcete ho upraviť?"

MEditCanNotEditDirectory
l:
"Невозможно редактировать папку"
"It is impossible to edit the folder"
"Nelze editovat adresář"
"Es ist nicht möglich den Ordner zu bearbeiten"
"A mappa nem szerkeszthető"
"Nie można edytować folderu"
"Es imposible editar el directorio"
"Nemôžem upraviť priečinok"

MEditSearchTitle
l:
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keresés"
"Szukaj"
"Buscar"
"Hľadanie"

MEditSearchFor
"&Искать"
"&Search for"
"&Hledat"
"&Suchen nach"
"&Keresés:"
"&Znajdź"
"&Buscar por"
"&Hľadať"

MEditSearchCase
"&Учитывать регистр"
"&Case sensitive"
"&Rozlišovat velikost písmen"
"G&roß-/Kleinschrb."
"&Nagy/kisbetű érz."
"&Uwzględnij wielkość liter"
"Sensible min/ma&y"
"&malé a VEĽKÉ"

MEditSearchWholeWords
"Только &целые слова"
"&Whole words"
"&Celá slova"
"&Ganze Wörter"
"Csak e&gész szavak"
"Tylko całe słowa"
"&Palabras completas"
"&Celé slová"

MEditSearchReverse
"Обратн&ый поиск"
"Re&verse search"
"&Zpětné hledání"
"Richtung um&kehren"
"&Visszafelé keres"
"Szukaj w &odwrotnym kierunku"
"Búsqueda in&versa"
"Opačné h&ľadanie"

MEditSearchSelFound
"&Выделять найденное"
"Se&lect found"
"Vy&ber nalezené"
"Treffer &markieren"
"&Találat kijelölése"
"W&ybierz znalezione"
"Se&leccionado encontrado"
"Vy&brať nájdené"

MEditSearchRegexp
"&Регулярные выражения"
"Re&gular expressions"
upd:"Re&gular expressions"
upd:"Re&gular expressions"
upd:"Re&gular expressions"
upd:"Re&gular expressions"
"Expresiones re&gulares"
"Re&gulárne výrazy"

MEditSearchSearch
"Искать"
"Search"
"Hledat"
"Suchen"
"Kere&sés"
"&Szukaj"
"Buscar"
"Hľadať"

MEditSearchAll
"Вс&ё"
"&All"
upd:"&All"
upd:"&All"
upd:"&All"
upd:"&All"
"&Todo"
"Všetk&y"

MEditSearchCancel
"Отменить"
"Cancel"
"Storno"
"Abbruch"
"&Mégsem"
"&Anuluj"
"Cancelar"
"Storno"

MEditReplaceTitle
l:
"Замена"
"Replace"
"Nahradit"
"Ersetzen"
"Keresés és csere"
"Zamień"
"Reemplazar"
"Nahradiť"

MEditReplaceWith
"Заменить &на"
"R&eplace with"
"Nahradit &s"
"&Ersetzen mit"
"&Erre cseréli:"
"Zamień &na"
"R&eemplazar con"
"Nahradiť &čím"

MEditReplaceReplace
"&Замена"
"&Replace"
"&Nahradit"
"E&rsetzen"
"&Csere"
"Za&mień"
"&Reemplazar"
"&Nahradiť"

MEditSearchingFor
l:
"Искать"
"Searching for"
"Vyhledávám"
"Suche nach"
"Keresett szöveg:"
"Szukam"
"Buscando por"
"Vyhľadávam"

MEditSearchStatistics
"Вхождений: %1, строк: %2"
"Occurrences: %1, lines: %2"
upd:"Occurrences: %1, lines: %2"
upd:"Occurrences: %1, lines: %2"
upd:"Occurrences: %1, lines: %2"
upd:"Occurrences: %1, lines: %2"
"Frecuencia: %1, líneas: %2"
"Nájdených: %1, riadky: %2"

MEditNotFound
"Строка не найдена"
"Could not find the string"
"Nemůžu najít řetězec"
"Konnte Zeichenkette nicht finden"
"A szöveg nem található:"
"Nie mogę odnaleźć ciągu"
"No se puede encontrar la cadena"
"Nemôžem nájsť reťazec"

MEditAskReplace
l:
"Заменить"
"Replace"
"Nahradit"
"Ersetze"
"Ezt cseréli:"
"Zamienić"
"Reemplazar"
"Nahradiť"

MEditAskReplaceWith
"на"
"with"
"s"
"mit"
"erre a szövegre:"
"na"
"con"
"čím"

MEditReplace
"&Заменить"
"&Replace"
"&Nahradit"
"E&rsetzen"
"&Csere"
"&Zamień"
"&Reemplazar"
"&Nahradiť"

MEditReplaceAll
"&Все"
"&All"
"&Vše"
"&Alle"
"&Mindet"
"&Wszystkie"
"&Todos"
"&Všetky"

MEditSkip
"&Пропустить"
"&Skip"
"Přes&kočit"
"Über&springen"
"&Kihagy"
"&Omiń"
"&Omitir"
"Pres&kočiť"

MEditCancel
"&Отменить"
"&Cancel"
"&Storno"
"Ab&bruch"
"Mé&gsem"
"&Anuluj"
"&Cancelar"
"&Storno"

MEditOpenCreateLabel
"&Открыть/создать файл:"
"&Open/create file:"
"Otevřít/vytvořit soubor:"
"Öffnen/datei erstellen:"
"Fájl megnyitása/&létrehozása:"
"&Otwórz/utwórz plik:"
"&Abrir/crear archivo:"
"Otvoriť/vytvoriť súbor:"

MEditOpenAutoDetect
"&Автоматическое определение"
"&Automatic detection"
upd:"Automatic detection"
upd:"Automatic detection"
"&Automatikus felismerés"
"&Wykryj automatycznie"
"Detección &automática"
"Automatické zistenie"

MDefaultCP
"По умолчанию"
"Default"
upd:"Default"
upd:"Default"
upd:"Default"
upd:"Default"
"Por defecto"
"Východiskové"

MEditGoToLine
l:
"Перейти"
"Go to position"
"Jít na pozici"
"Gehe zu Zeile"
"Sorra ugrás"
"Idź do linii"
"Ir a posición"
"Ísť na pozíciu"

MFolderShortcutsTitle
l:
"Ссылки на папки"
"Folder shortcuts"
"Adresářové zkratky"
"Ordnerschnellzugriff"
"Mappa gyorsbillentyűk"
"Skróty katalogów"
"Atajos a directorios"
"Skratky priečinkov"

MFolderShortcutBottom
"Редактирование: Del,Ins,ShiftIns,F4"
"Edit: Del,Ins,ShiftIns,F4"
"Edit: Del,Ins,ShiftIns,F4"
"Bearb.: Entf,Einf,ShiftEinf,F4"
"Szerk.: Del,Ins,ShiftIns,F4"
"Edycja: Del,Ins,ShiftIns,F4"
"Editar: Del,Ins,ShiftIns,F4"
"Upraviť: Del,Ins,ShiftIns,F4"

MFolderShortcutBottomSub
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Bearb.: Entf,Einf,F4"
"Szerk.: Del,Ins,F4"
"Edycja: Del,Ins,F4"
"Editar: Del,Ins,F4"
"Upraviť: Del,Ins,F4"

MShortcutNone
"<отсутствует>"
"<none>"
"<není>"
"<keiner>"
"<nincs>"
"<brak>"
"<nada>"
"<nie je>"

MShortcutPlugin
"<плагин>"
"<plugin>"
"<plugin>"
"<Plugin>"
"<plugin>"
"<plugin>"
"<complemento>"
"<modul>"

MFSShortcutName
"Название:"
"Title:"
upd:"Title:"
upd:"Title:"
upd:"Title:"
upd:"Title:"
"Título:"
"Názov:"

MFSShortcutPath
"Путь:"
"Path:"
upd:"Path:"
upd:"Path:"
upd:"Path:"
upd:"Path:"
"Ruta:"
"Cesta:"

MNeedNearPath
"Перейти в ближайшую доступную папку?"
"Jump to the nearest existing folder?"
"Skočit na nejbližší existující adresář?"
"Zum nahesten existierenden Ordner springen?"
"Ugrás a legközelebbi létező mappára?"
"Przejść do najbliższego istniejącego folderu?"
"Saltar al próximo directorio existente"
"Skočiť na najbližší existujúci priečinok?"

MSaveThisShortcut
"Запомнить эту ссылку?"
"Save this shortcut?"
"Uložit tyto zkratky?"
"Verknüpfung speichern?"
"Mentsem a gyorsbillentyűket?"
"Zapisać skróty?"
"Guardar este atajo"
"Uložiť tieto skratky?"

MEditF1
l:
l://functional keys - 6 characters max, 12 keys, "OEM" is F8 dupe!
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Súgó"
"Pomoc"
"Ayuda"
"Pomocník"

MEditF2
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"
"Zapisz"
"Guarda"
"Uložiť"

MEditF3
""
""
""
""
""
""
""
""

MEditF4
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"
"Koniec"
"Salir"
"Koniec"

MEditF5
""
""
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
"Megnéz"
"Zobacz"
"Ver "
"Zobraziť"

MEditF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"
"Szukaj"
"Buscar"
"Hľadať"

MEditF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"Latin 2"
"ANSI"
"WIN"

MEditF9
""
""
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
"Kilép"
"Koniec"
"Salir"
"Koniec"

MEditF11
"Плагины"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Comple"
"Modul"

MEditF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"
"Ekran"
"Pantal"
"Okno"

MEditF8DOS
le:// don't count this - it's a F8 another text
"OEM"
"OEM"
"OEM"
"OEM"
"OEM"
"CP-1250"
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
"Grdcom"
"UložAko"

MEditShiftF3
""
""
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
"Edita."
"Uprav."

MEditShiftF5
""
""
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
""
""

MEditShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"
"Następ"
"Próxim"
"Nasl."

MEditShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"
"Tabela"
"PágCód"
"TabZn"

MEditShiftF9
""
""
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
"GrdySl"
"UložKon"

MEditShiftF11
""
""
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
""
""

MEditAltF2
""
""
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
""
""

MEditAltF4
""
""
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
"Imprim"
"Tlač"

MEditAltF6
""
""
""
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
"Poprz"
"Previo"
"Predch"

MEditAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"
"IdźDo"
"Ir a.."
"Ísť na"

MEditAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"
"Video"
"Video"

MEditAltF10
""
""
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
"NézElő"
"Historia"
"VerHis"
"ZobrHs"

MEditAltF12
""
""
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
""
""

MEditCtrlF2
""
""
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
""
""

MEditCtrlF4
""
""
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
""
""

MEditCtrlF6
""
""
""
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
"Zamień"
"Remplz"
"Nahraď"

MEditCtrlF8
""
""
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
""
""

MEditCtrlF10
"Позиц"
"GoFile"
"JdiSou"
"GehDat"
"FájlPz"
"GoFile"
"IrArch"
"ÍsťSúb"

MEditCtrlF11
""
""
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
""
""

MEditAltShiftF2
""
""
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
""
""

MEditAltShiftF4
""
""
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
""
""

MEditAltShiftF6
""
""
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
""
""

MEditAltShiftF8
""
""
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
"Beáll."
"Konfig"
"Config"
"Konfig"

MEditAltShiftF10
""
""
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
""
""

MEditAltShiftF12
""
""
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
""
""

MEditCtrlShiftF2
""
""
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
""
""

MEditCtrlShiftF4
""
""
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
""
""

MEditCtrlShiftF6
""
""
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
""
""

MEditCtrlShiftF8
""
""
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
""
""

MEditCtrlShiftF10
""
""
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
""
""

MEditCtrlShiftF12
""
""
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
""
""

MEditCtrlAltF2
""
""
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
""
""

MEditCtrlAltF4
""
""
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
""
""

MEditCtrlAltF6
""
""
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
""
""

MEditCtrlAltF8
""
""
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
""
""

MEditCtrlAltF10
""
""
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
""
""

MEditCtrlAltF12
""
""
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
""
""

MEditCtrlAltShiftF2
""
""
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
""
""

MEditCtrlAltShiftF4
""
""
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
""
""

MEditCtrlAltShiftF6
""
""
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
""
""

MEditCtrlAltShiftF8
""
""
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
""
""

MEditCtrlAltShiftF10
""
""
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
"Pomoc"
"Ayuda"
"Pomocn"

MSingleEditF2
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"
"Zapisz"
"Guarda"
"Uložiť"

MSingleEditF3
""
""
""
""
""
""
""
""

MSingleEditF4
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"
"Koniec"
"Salir"
"Koniec"

MSingleEditF5
""
""
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
"Megnéz"
"Zobacz"
"Ver"
"Zobraz"

MSingleEditF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"
"Szukaj"
"Buscar"
"Hľadať"

MSingleEditF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"Latin 2"
"ANSI"
"WIN"

MSingleEditF9
""
""
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
"Kilép"
"Koniec"
"Salir"
"Koniec"

MSingleEditF11
"Плагины"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Comple"
"Modul"

MSingleEditF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"
"Ekran"
"Pantal"
"Okno"

MSingleEditF8DOS
le:// don't count this - it's a F8 another text
"OEM"
"OEM"
"OEM"
"OEM"
"OEM"
"CP 1250"
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
"GrdCom"
"UložAko"

MSingleEditShiftF3
""
""
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
""
""

MSingleEditShiftF5
""
""
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
""
""

MSingleEditShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"
"Następ"
"Próxim"
"Nasl."

MSingleEditShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"
"Tabela"
"PágCód"
"TabZn"

MSingleEditShiftF9
""
""
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
"GrdySl"
"UložKon"

MSingleEditShiftF11
""
""
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
""
""

MSingleEditAltF2
""
""
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
""
""

MSingleEditAltF4
""
""
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
"Imprim"
"Tlač"

MSingleEditAltF6
""
""
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
""
""

MSingleEditAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"
"IdźDo"
"Ir a.."
"Ísť na"

MSingleEditAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Ekran"
"Video"
"Video"

MSingleEditAltF10
""
""
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
"NézElő"
"ZobHs"
"VerHis"
"ZobrHs"

MSingleEditAltF12
""
""
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
""
""

MSingleEditCtrlF2
""
""
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
""
""

MSingleEditCtrlF4
""
""
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
""
""

MSingleEditCtrlF6
""
""
""
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
"Zastąp"
"Remplz"
"Nahraď"

MSingleEditCtrlF8
""
""
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
""
""

MSingleEditCtrlF10
""
""
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
""
""

MSingleEditCtrlF12
""
""
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
""
""

MSingleEditAltShiftF2
""
""
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
""
""

MSingleEditAltShiftF4
""
""
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
""
""

MSingleEditAltShiftF6
""
""
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
""
""

MSingleEditAltShiftF8
""
""
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
"Beáll."
"Konfig"
"Config"
"Konfig"

MSingleEditAltShiftF10
""
""
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
""
""

MSingleEditAltShiftF12
""
""
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
""
""

MSingleEditCtrlShiftF2
""
""
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
""
""

MSingleEditCtrlShiftF4
""
""
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
""
""

MSingleEditCtrlShiftF6
""
""
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
""
""

MSingleEditCtrlShiftF8
""
""
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
""
""

MSingleEditCtrlShiftF10
""
""
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
""
""

MSingleEditCtrlShiftF12
""
""
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
""
""

MSingleEditCtrlAltF2
""
""
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
""
""

MSingleEditCtrlAltF4
""
""
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
""
""

MSingleEditCtrlAltF6
""
""
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
""
""

MSingleEditCtrlAltF8
""
""
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
""
""

MSingleEditCtrlAltF10
""
""
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
""
""

MSingleEditCtrlAltF12
""
""
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
""
""

MSingleEditCtrlAltShiftF2
""
""
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
""
""

MSingleEditCtrlAltShiftF4
""
""
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
""
""

MSingleEditCtrlAltShiftF6
""
""
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
""
""

MSingleEditCtrlAltShiftF8
""
""
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
""
""

MSingleEditCtrlAltShiftF10
""
""
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
""
""

MEditSaveAs
l:
"Сохранить &файл как"
"Save file &as"
"Uložit soubor jako"
"Speichern &als"
"Fá&jl mentése, mint:"
"Zapisz plik &jako"
"Guardar archivo &como"
"Uložiť súbor ako"

MEditCodePage
"&Кодовая страница:"
"&Code page:"
"Kódová stránka:"
"Codepage:"
"Kódlap:"
"&Strona kodowa:"
"Página de &códigos:"
"Tabuľka znakov:"

MEditAddSignature
"Добавить &сигнатуру (BOM)"
"Add &signature (BOM)"
"Přidat signaturu (BOM)"
"Sinatur hinzu (BOM)"
"Uni&code bájtsorrend jelzővel (BOM)"
"Dodaj &znacznik BOM"
"Añadir &signatura (BOM)"
"Pridať podpis (BOM)"

MEditSaveAsFormatTitle
"Изменить перевод строки:"
"Change line breaks to:"
"Změnit zakončení řádků na:"
"Zeilenumbrüche setzen:"
"Sortörés konverzió:"
"Zamień znaki końca linii na:"
"Cambiar fin de líneas a:"
"Zmeniť konce riadkov na:"

MEditSaveOriginal
"&исходный формат"
"Do n&ot change"
"&Beze změny"
"Nicht verä&ndern"
"Nincs &konverzió"
"&Nie zmieniaj"
"N&o cambiar"
"&Bez zmeny"

MEditSaveDOS
"в форма&те DOS/Windows (CR LF)"
"&Dos/Windows format (CR LF)"
"&Dos/Windows formát (CR LF)"
"&Dos/Windows Format (CR LF)"
"&DOS/Windows formátum (CR LF)"
"Format &Dos/Windows (CR LF)"
"Formato &DOS/Windows (CR LF)"
"Formát &DOS/Windows (CR LF)"

MEditSaveUnix
"в формат&е UNIX (LF)"
"&Unix format (LF)"
"&Unix formát (LF)"
"&Unix Format (LF)"
"&UNIX formátum (LF)"
"Format &Unix (LF)"
"Formato &Unix (LF)"
"Formát &Unix (LF)"

MEditSaveMac
"в фор&мате MAC (CR)"
"&Mac format (CR)"
"&Mac formát (CR)"
"&Mac Format (CR)"
"&Mac formátum (CR)"
"Format &Mac (CR)"
"Formato &Mac (CR)"
"Formát &Mac (CR)"

MEditCannotSave
"Ошибка сохранения файла"
"Cannot save the file"
"Nelze uložit soubor"
"Kann die Datei nicht speichern"
"A fájl nem menthető"
"Nie mogę zapisać pliku"
"No se puede guardar archivo"
"Nemôžem uložiť súbor"

MEditSavedChangedNonFile
"Файл изменён, но файл или папка, в которой он находился,"
"The file is changed but the file or the folder containing"
"Soubor je změněn, ale soubor, nebo adresář obsahující"
"Inhalt dieser Datei wurde verändert aber die Datei oder der Ordner, welche"
"A fájl megváltozott, de a fájlt vagy a mappáját"
"Plik został zmieniony, ale plik lub folder zawierający"
"El archivo es cambiado pero el archivo o el directorio que contiene"
"Súbor je zmenený, ale súbor alebo priečinok obsahujúci"

MEditSavedChangedNonFile1
"Файл или папка, в которой он находился,"
"The file or the folder containing"
"Soubor nebo adresář obsahující"
"Die Datei oder der Ordner, welche"
"A fájlt vagy a mappáját"
"Plik lub folder zawierający"
"El archivo o el directorio conteniendo"
"Súbor alebo priečinok obsahujúci"

MEditSavedChangedNonFile2
"был перемещён или удалён. Сохранить?"
"this file was moved or deleted. Save?"
upd:"tento soubor byl přesunut, nebo smazán. Save?"
upd:"diesen Inhalt enthält wurde verschoben oder gelöscht. Save?"
upd:"időközben áthelyezte/átnevezte vagy törölte. Save?"
upd:"ten plik został przeniesiony lub usunięty. Save?"
"este archivo ha sido movido o borrado. Desea guardarlo?"
"tento súbor bol presunutý alebo zmazaný. Uložiť?"

MEditNewPath1
"Путь к редактируемому файлу не существует,"
"The path to the edited file does not exist,"
"Cesta k editovanému souboru neexistuje,"
"Der Pfad zur bearbeiteten Datei existiert nicht,"
"A szerkesztendő fájl célmappája még"
"Ścieżka do edytowanego pliku nie istnieje,"
"La ruta del archivo editado no existe,"
"Cesta k upravenému súboru neexistuje,"

MEditNewPath2
"но будет создан при сохранении файла."
"but will be created when the file is saved."
"ale bude vytvořena při uložení souboru."
"aber wird erstellt sobald die Datei gespeichert wird."
"nem létezik, de mentéskor létrejön."
"ale zostanie utworzona po zapisaniu pliku."
"pero será creada cuando el archivo sea guardado."
"ale vytvorí sa pri uložení súboru."

MEditNewPath3
"Продолжать?"
"Continue?"
"Pokračovat?"
"Fortsetzen?"
"Folytatja?"
"Kontynuować?"
"Continuar?"
"Pokračovať?"

MEditNewPlugin1
"Имя редактируемого файла не может быть пустым"
"The name of the file to edit cannot be empty"
"Název souboru k editaci nesmí být prázdné"
"Der Name der zu editierenden Datei kann nicht leer sein"
"A szerkesztendő fájlnak nevet kell adni"
"Nazwa pliku do edycji nie może być pusta"
"El nombre del archivo a editar no puede estar vacío"
"Názov súboru na úpravu nesmie byť prázdny"

MEditorLoadCPWarn1
"Файл содержит символы, которые невозможно"
"File contains characters, which cannot be"
upd:"File contains characters, which cannot be"
upd:"File contains characters, which cannot be"
upd:"File contains characters, which cannot be"
upd:"File contains characters, which cannot be"
"El archivo contiene caracteres que no pueden ser"
"Súbor obsahuje znaky, ktoré sa nedajú"

MEditorLoadCPWarn2
"корректно прочитать, используя выбранную кодовую страницу."
"correctly read using selected codepage."
upd:"correctly read using selected codepage."
upd:"correctly read using selected codepage."
upd:"correctly read using selected codepage."
upd:"correctly read using selected codepage."
"correctamente leídos con la página de códigos (codepage) seleccionada."
"správne prečítať so zvolenou tabuľkou znakov."

MEditorSaveCPWarn1
"Редактор содержит символы, которые невозможно"
"Editor contains characters, which cannot be"
upd:"Editor contains characters, which cannot be"
upd:"Editor contains characters, which cannot be"
upd:"Editor contains characters, which cannot be"
upd:"Editor contains characters, which cannot be"
"El editor contiene caracteres que no pueden ser"
"Editor obsahuje znaky, ktoré sa nedajú"

MEditorSaveCPWarn2
"корректно сохранить, используя выбранную кодовую страницу."
"correctly saved using selected codepage."
upd:"correctly saved using selected codepage."
upd:"correctly saved using selected codepage."
upd:"correctly saved using selected codepage."
upd:"correctly saved using selected codepage."
"correctamente guardados con página de códigos (codepage) seleccionada."
"správne uložiť so zvolenou tabuľkou znakov."

MEditorSwitchCPWarn1
"Редактор содержит символы, которые невозможно"
"Editor contains characters, which cannot be"
upd:"Editor contains characters, which cannot be"
upd:"Editor contains characters, which cannot be"
upd:"Editor contains characters, which cannot be"
upd:"Editor contains characters, which cannot be"
"El editor contiene caracteres que no pueden ser"
"Editor obsahuje znaky, ktoré sa nedajú"

MEditorSwitchCPWarn2
"корректно преобразовать, используя выбранную кодовую страницу."
"correctly translated using selected codepage."
upd:"correctly translated using selected codepage."
upd:"correctly translated using selected codepage."
upd:"correctly translated using selected codepage."
upd:"correctly translated using selected codepage."
"correctamente traducidos con la página de códigos (codepage) seleccionada."
"správne preložiť so zvolenou tabuľkou znakov."

MEditorSwitchCPConfirm
"Продолжить?"
"Continue?"
upd:"Continue?"
upd:"Continue?"
upd:"Continue?"
upd:"Continue?"
"Continuar?"
"Pokračovať?"

MEditDataLostWarn
"Во время редактирования файла некоторые данные были утеряны."
"During file editing some data were lost."
upd:"During file editing some data were lost."
upd:"During file editing some data were lost."
upd:"During file editing some data were lost."
upd:"During file editing some data were lost."
"Durante la edición del archivo algunos datos se perdieron."
"Počas úpravy súboru sa niektoré dáta stratili."

MEditorSaveNotRecommended
"Сохранять файл не рекомендуется."
"It is not recommended to save this file."
"Není doporučeno uložit tento soubor."
"Es wird empfohlen, die Datei nicht zu speichern."
"A fájl mentése nem ajánlott."
"Odradzamy zapis pliku."
"No se recomienda guardar este archivo."
"Neodporúčame uložiť tento súbor."

MEditorSaveCPWarnShow
"Показать"
"Show"
upd:"Show"
upd:"Show"
upd:"Show"
upd:"Show"
"Mostrar"
"Zobraziť"

MEditorSwitchUnicodeCPDisabled
"Нельзя изменить юникодную кодовую страницу"
"Unicode codepage switch is not supported"
upd:"Unicode codepage switch is not supported"
upd:"Unicode codepage switch is not supported"
upd:"Unicode codepage switch is not supported"
upd:"Unicode codepage switch is not supported"
"Cambio a página de códigos Unicode no está soportado"
"Prepínač Unicode nie je podporovaný"

MEditorTryReloadFile
"Можно перечитать файл, указав нужную кодовую страницу (ShiftF4)"
"You can reload file using custom codepage (ShiftF4)"
upd:"You can reload file using custom codepage (ShiftF4)"
upd:"You can reload file using custom codepage (ShiftF4)"
upd:"You can reload file using custom codepage (ShiftF4)"
upd:"You can reload file using custom codepage (ShiftF4)"
"Usted puede recargar el archivo usando página de códigos a elección (ShiftF4)"
"Môžete znova načítať súbor s užívateľskou tabuľkou znakov (ShiftF4)"

MEditorSwitchToUnicodeCPDisabled
"Переключение на юникодную кодовую страницу %d не поддерживается"
"Switch to unicode codepage %d is not supported"
upd:"Switch to unicode codepage %d is not supported"
upd:"Switch to unicode codepage %d is not supported"
upd:"Switch to unicode codepage %d is not supported"
upd:"Switch to unicode codepage %d is not supported"
"Cambio a página de códigos unicode %d no está soportado"
"Prepínač na tabuľku Unicode %d nie je podporovaný"

MEditorCPNotSupported
"Кодовая страница %d не поддерживается вашей системой"
"Codepage %d is not supported in your system"
upd:"Codepage %d is not supported in your system"
upd:"Codepage %d is not supported in your system"
upd:"Codepage %d is not supported in your system"
upd:"Codepage %d is not supported in your system"
"Página de códigos %d no está soportado en su sistema"
"Tabuľka znakov %d nie je podporovaná vo vašom systéme"

MEditorCPNotDetected
"Не удалось определить кодовую страницу"
"Codepage wasn't detected"
upd:"Codepage wasn't detected"
upd:"Codepage wasn't detected"
upd:"Codepage wasn't detected"
upd:"Codepage wasn't detected"
"No se detectó página de códigos"
"Nebola zistená tabuľka znakov"

MColumnName
l:
"Имя"
"Name"
"Název"
"Name"
"Név"
"Nazwa"
"Nombre"
"Názov"

MColumnExtension
"Расширение"
"Extension"
upd:"Extension"
upd:"Extension"
upd:"Extension"
upd:"Extension"
"Extensión"
"Prípona"

MColumnSize
"Размер"
"Size"
"Velikost"
"Größe"
"Méret"
"Rozmiar"
"Tamaño"
"Veľkosť"

MColumnAlocatedSize
"Выделено"
"Allocated"
upd:"Allocated"
upd:"Allocated"
upd:"Allocated"
upd:"Allocated"
"Compresión"
"Allocated"

MColumnDate
"Дата"
"Date"
"Datum"
"Datum"
"Dátum"
"Data"
"Fecha"
"Dátum"

MColumnTime
"Время"
"Time"
"Čas"
"Zeit"
"Idő"
"Czas"
"Hora"
"Čas"

MColumnWrited
"Запись"
"Write"
upd:"Write"
upd:"Write"
upd:"Write"
upd:"Write"
"Escrit"
"Zápis"

MColumnCreated
"Создание"
"Created"
"Vytvořen"
"Erstellt"
"Létrejött"
"Utworzenie"
"Creado "
"Vytvorený"

MColumnAccessed
"Доступ"
"Accessed"
"Přístup"
"Zugriff"
"Hozzáférés"
"Użycie"
"Acceso  "
"Prístup"

MColumnChanged
"Изменение"
upd:"Change"
upd:"Change"
upd:"Change"
upd:"Change"
upd:"Change"
"Cambio"
"Zmenený"

MColumnAttr
"Атриб"
"Attr"
"Attr"
"Attr"
"Attrib"
"Atrybuty"
"Atrib"
"Atrib."

MColumnDescription
"Описание"
"Description"
"Popis"
"Beschreibung"
"Megjegyzés"
"Opis"
"Descripción"
"Popis"

MColumnOwner
"Владелец"
"Owner"
"Vlastník"
"Besitzer"
"Tulajdonos"
"Właściciel"
"Dueño"
"Vlastník"

MColumnMumLinks
"КлС"
"NmL"
"PočLn"
"AnL"
"Lnk"
"NmL"
"NmL"
"NmL"

MColumnNumStreams
"КлП"
"NmS"
upd:"NmS"
upd:"NmS"
"Stm"
upd:"NmS"
"NmS"
"NmS"

MColumnStreamsSize
"РазмПт"
"StrmSz"
upd:"StrmSz"
upd:"StrmSz"
"StmMér"
upd:"StrmSz"
"StrmSz"
"StrmSz"

MColumnUnknown
"???"
"???"
"???"
"???"
"???"
"???"
"???"
"???"

MListUp
l:
"Вверх"
"  Up  "
"Nahoru"
" Hoch "
"  Fel  "
"W górę"
"UP-DIR"
"Nahor"

MListFolder
"Папка"
"Folder"
"Adresář"
"Ordner"
" Mappa "
"Folder"
"SUB-DIR"
"Priečinok"

MListSymlink
"Ссылка"
"Symlink"
"Link"
"Symlink"
"SzimLnk"
"LinkSym"
"EnlSimb"
"Prepojenie"

MListJunction
"Связь"
"Junction"
"Křížení"
"Knoten"
"Csomópt"
"Dowiązania"
" Unir "
"Kríženie"

MListVolMount
"Том"
"Volume"
"Svazek"
upd:"Volume"
upd:"Volume"
upd:"Volume"
"Volumen"
"Zväzok"

MListDFS
upd:"DFS"
"DFS"
upd:"DFS"
upd:"DFS"
upd:"DFS"
upd:"DFS"
upd:"DFS"
upd:"DFS"

MListDFSR
upd:"DFSR"
"DFSR"
upd:"DFSR"
upd:"DFSR"
upd:"DFSR"
upd:"DFSR"
upd:"DFSR"
upd:"DFSR"

MListHSM
upd:"HSM"
"HSM"
upd:"HSM"
upd:"HSM"
upd:"HSM"
upd:"HSM"
upd:"HSM"
upd:"HSM"

MListHSM2
upd:"HSM2"
"HSM2"
upd:"HSM2"
upd:"HSM2"
upd:"HSM2"
upd:"HSM2"
upd:"HSM2"
upd:"HSM2"

MListSIS
upd:"SIS"
"SIS"
upd:"SIS"
upd:"SIS"
upd:"SIS"
upd:"SIS"
upd:"SIS"
upd:"SIS"

MListWIM
upd:"WIM"
"WIM"
upd:"WIM"
upd:"WIM"
upd:"WIM"
upd:"WIM"
upd:"WIM"
upd:"WIM"

MListCSV
upd:"CSV"
"CSV"
upd:"CSV"
upd:"CSV"
upd:"CSV"
upd:"CSV"
upd:"CSV"
upd:"CSV"

MListUnknownReparsePoint
"?Ссылка?"
"?Symlink?"
"?Link?"
"?Symlink?"
"?SzimLnk?"
"?LinkSym?"
"?EnlSimb?"
"?Prepojenie?"

MListBrackets
"<>"
"<>"
"<>"
"<>"
"<>"
"<>"
"<>"
"<>"

MListBytes
"Б"
"B"
"B"
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
"K"
"k"

MListMb
"М"
"M"
"M"
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
"G"
"G"

MListTb
"Т"
"T"
"T"
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
"P"
"P"

MListEb
"Э"
"E"
"E"
"E"
"E"
"E"
"E"
"E"

MListFileSize
" %1 байт в 1 файле "
" %1 bytes in 1 file "
" %1 bytů v 1 souboru "
" %1 Bytes in 1 Datei "
" %1 bájt 1 fájlban "
" %1 bajtów w 1 pliku "
" %1 bytes en 1 archivo "
" %1 bajtov v 1 súbore "

MListFilesSize1
" %1 байт в %2 файле "
" %1 bytes in %2 files "
" %1 bytů v %2 souborech "
" %1 Bytes in %2 Dateien "
" %1 bájt %2 fájlban "
" %1 bajtów w %2 plikach "
" %1 bytes en %2 archivos "
" %1 bajtov v %2 súboroch "

MListFilesSize2
" %1 байт в %2 файлах "
" %1 bytes in %2 files "
" %1 bytů v %2 souborech "
" %1 Bytes in %2 Dateien "
" %1 bájt %2 fájlban "
" %1 bajtów w %2 plikach "
" %1 bytes en %2 archivos "
" %1 bajtov v %2 súboroch "

MListFreeSize
" %1 байт свободно "
" %1 free bytes "
" %1 volných bytů "
" %1 freie Bytes "
" %1 szabad bájt "
" %1 wolnych bajtów "
" %1 bytes libres "
" %1 voľných bajtov "

MDirInfoViewTitle
l:
"Просмотр"
"View"
"Zobraz"
"Betrachten"
"Vizsgálat"
"Podgląd"
"Ver "
"Zobraziť"

MFileToEdit
"Редактировать файл:"
"File to edit:"
"Soubor k editaci:"
"Datei bearbeiten:"
"Szerkesztendő fájl:"
"Plik do edycji:"
"Archivo a editar:"
"Súbor na úpravu:"

MUnselectTitle
l:
"Снять"
"Deselect"
"Odznačit"
"Abwählen"
"Kijelölést levesz"
"Odznacz"
"Deseleccionar"
"Odznačiť"

MSelectTitle
"Пометить"
"Select"
"Označit"
"Auswählen"
"Kijelölés"
"Zaznacz"
"Seleccionar"
"Označiť"

MSelectFilter
"&Фильтр"
"&Filter"
"&Filtr"
"&Filter"
"&Szűrő"
"&Filtruj"
"&Filtro"
"&Filter"

MCompareTitle
l:
"Сравнение"
"Compare"
"Porovnat"
"Vergleichen"
"Összehasonlítás"
"Porównaj"
"Comparar"
"Porovnať"

MCompareFilePanelsRequired1
"Для сравнения папок требуются"
"Two file panels are required to perform"
"Pro provedení příkazu Porovnat adresáře"
"Zwei Dateipanels werden benötigt um"
"Mappák összehasonlításához"
"Aby porównać katalogi konieczne są"
"Dos paneles de archivos son necesarios para poder"
"Na vykonanie príkazu Porovnať priečinky"

MCompareFilePanelsRequired2
"две файловые панели"
"the Compare folders command"
"jsou nutné dva souborové panely"
"den Vergleich auszuführen."
"két fájlpanel szükséges"
"dwa zwykłe panele plików"
"utilizar el comando comparar directorios"
"treba dva súborové panely"

MCompareSameFolders1
"Содержимое папок,"
"The folders contents seems"
"Obsahy adresářů jsou"
"Der Inhalt der beiden Ordner scheint"
"A mappák tartalma"
"Zawartość katalogów"
"El contenido de los directorios parecen"
"Obsahy priečinkov sú"

MCompareSameFolders2
"скорее всего, одинаково"
"to be identical"
"identické"
"identisch zu sein."
"azonosnak tűnik"
"wydaje się być identyczna"
"ser idénticos"
"zhodné"

MSelectAssocTitle
l:
"Выберите ассоциацию"
"Select association"
"Vyber závislosti"
"Dateiverknüpfung auswählen"
"Válasszon társítást"
"Wybierz przypisanie"
"Seleccionar asociaciones"
"Výber asociácie"

MAssocTitle
l:
"Ассоциации для файлов"
"File associations"
"Závislosti souborů"
"Dateiverknüpfungen"
"Fájltársítások"
"Przypisania plików"
"Asociación de archivos"
"Asociácie súborov"

MAssocBottom
"Редактирование: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Edit: Del,Ins,F4"
"Bearb.: Entf,Einf,F4"
"Szerk.: Del,Ins,F4"
"Edycja: Del,Ins,F4"
"Editar: Del,Ins,F4"
"Upraviť: Del,Ins,F4"

MAskDelAssoc
"Вы хотите удалить ассоциацию для"
"Do you wish to delete association for"
"Přejete si smazat závislost pro"
"Wollen Sie die Verknüpfung löschen für"
"Törölni szeretné a társítást?"
"Czy chcesz usunąć przypisanie dla"
"Desea borrar la asociación para"
"Chcete zmazať asociáciu pre"

MFileAssocTitle
l:
"Редактирование ассоциаций файлов"
"Edit file associations"
"Upravit závislosti souborů"
"Dateiverknüpfungen bearbeiten"
"Fájltársítások szerkesztése"
"Edytuj przypisania pliku"
"Editar asociación de archivos"
"Upraviť asociácie súborov"

MFileAssocMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"
"&Maska nebo masky souborů:"
"Datei&maske (mehrere getrennt mit Komma):"
"F&ájlmaszk(ok, vesszővel elválasztva):"
"&Maska pliku lub kilka masek oddzielonych przecinkami:"
"&Máscara de archivo o múltiples máscaras de archivos:"
"&Maska alebo masky súborov:"

MFileAssocDescr
"&Описание ассоциации:"
"&Description of the association:"
"&Popis asociací:"
"&Beschreibung der Verknüpfung:"
"A &társítás leírása:"
"&Opis przypisania:"
"&Descripción de la asociación:"
"&Popis asociácií:"

MFileAssocExec
"Команда, &выполняемая по Enter:"
"E&xecute command (used for Enter):"
"&Vykonat příkaz (použito pro Enter):"
"Befehl &ausführen (mit Enter):"
"&Végrehajtandó parancs (Enterre):"
"Polecenie (po naciśnięciu &Enter):"
"E&jecutar comando (usado por Enter):"
"&Vykonať príkaz (platí pre Enter):"

MFileAssocAltExec
"Коман&да, выполняемая по Ctrl-PgDn:"
"Exec&ute command (used for Ctrl-PgDn):"
"V&ykonat příkaz (použito pro Ctrl-PgDn):"
"Befehl a&usführen (mit Strg-PgDn):"
"Vé&grehajtandó parancs (Ctrl-PgDown-ra):"
"Polecenie (po naciśnięciu &Ctrl-PgDn):"
"Ejecutar comando (usado por Ctrl-PgDn):"
"V&ykonať príkaz (platí pre Ctrl-PgDn):"

MFileAssocView
"Команда &просмотра, выполняемая по F3:"
"&View command (used for F3):"
"Příkaz &Zobraz (použito pro F3):"
"Be&trachten (mit F3):"
"&Nézőke parancs (F3-ra):"
"&Podgląd (po naciśnięciu F3):"
"Comando de &visor (usado por F3):"
"Príkaz &Zobraz (platí pre F3):"

MFileAssocAltView
"Команда просмотра, в&ыполняемая по Alt-F3:"
"V&iew command (used for Alt-F3):"
"Příkaz Z&obraz (použito pro Alt-F3):"
"Bet&rachten (mit Alt-F3):"
"N&ézőke parancs (Alt-F3-ra):"
"Podg&ląd (po naciśnięciu Alt-F3):"
"Comando de visor (usado por Alt-F3):"
"Príkaz Z&obraz (platí pre Alt-F3):"

MFileAssocEdit
"Команда &редактирования, выполняемая по F4:"
"&Edit command (used for F4):"
"Příkaz &Edituj (použito pro F4):"
"Bearb&eiten (mit F4):"
"S&zerkesztés parancs (F4-re):"
"&Edycja  (po naciśnięciu F4):"
"Comando de &editor (usado por F4):"
"Príkaz &Uprav (platí pre F4):"

MFileAssocAltEdit
"Команда редактировани&я, выполняемая по Alt-F4:"
"Edit comm&and (used for Alt-F4):"
"Příkaz Editu&j (použito pro Alt-F4):"
"Bearbe&iten (mit Alt-F4):"
"Sze&rkesztés parancs (Alt-F4-re):"
"E&dycja  (po naciśnięciu Alt-F4):"
"Comando de editor (usado por Alt-F4):"
"Príkaz Upra&v (platí pre Alt-F4):"

MViewF1
l:
l://Viewer: functional keys, 12 keys, except F2 - 2 keys, and F8 - 2 keys
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Súgó"
"Pomoc"
"Ayuda"
"Pomocník"

MViewF2
le:// this is another text for F2
"Сверн"
"Wrap"
"Zalom"
"Umbr."
"SorTör"
"Zawiń"
"Divide"
"Zalomiť"

MViewF3
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"
"Koniec"
"Quitar"
"Koniec"

MViewF4
"Код"
"Hex"
"Hex"
"Hex"
"Hexa"
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
""
""

MViewF6
"Редакт"
"Edit"
"Edit"
"Bearb"
"Szerk."
"Edytuj"
"Editar"
"Upraviť"

MViewF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"
"Szukaj"
"Buscar"
"Hľadať"

MViewF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"Latin 2"
"ANSI"
"WIN"

MViewF9
""
""
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
"Kilép"
"Koniec"
"Quitar"
"Koniec"

MViewF11
"Плагины"
"Plugins"
"Plugin"
"Plugin"
"Plugin"
"Pluginy"
"Complementos"
"Moduly"

MViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"
"Ekran"
"Pantal"
"Okno"

MViewF2Unwrap
"Развер"
"Unwrap"
"Nezal"
"KeinUm"
"NemTör"
"Unwrap"
"NoDiv"
"Nezalom"

MViewF4Text
l:// this is another text for F4
"Текст"
"Text"
"Text"
"Text"
"Szöveg"
"Tekst"
"Texto"
"Text"

MViewF4Dump
"Дамп"
"Dump"
upd:"Dump"
upd:"Dump"
upd:"Dump"
upd:"Dump"
"Volcar"
"Dump"

MViewMode
"Режим просмотра"
"View mode"
upd:"View mode"
upd:"View mode"
upd:"View mode"
upd:"View mode"
"Modo de vista"
"View mode"

MViewF8DOS
"OEM"
"OEM"
"OEM"
"OEM"
"OEM"
"CP 1250"
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
""
""
""

MViewShiftF2
"Слова"
"WWrap"
"ZalSlo"
"WUmbr"
"SzóTör"
"ZawińS"
"ConDiv"
"ZalSlo"

MViewShiftF3
""
""
""
""
""
""
""
""

MViewShiftF4
"Режим"
"Mode"
upd:"Mode"
upd:"Mode"
upd:"Mode"
upd:"Mode"
"Modo"
"Mode"

MViewShiftF5
""
""
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
""
""

MViewShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"
"Następ"
"Próxim"
"Nasl."

MViewShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"
"Tabela"
"PágCód"
"Tabuľka"

MViewShiftF9
""
""
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
""
""

MViewShiftF11
""
""
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
""
""

MViewAltF2
""
""
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
""
""

MViewAltF4
""
""
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
"Imprim"
"Tlač"

MViewAltF6
""
""
""
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
"Poprz"
"Previo"
"Predch."

MViewAltF8
"Перейт"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"
"IdźDo"
"Ir a.."
"Ísť na"

MViewAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"
"Video"
"Video"

MViewAltF10
""
""
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
"NézElő"
"Historia"
"VerHis"
"ZobrHs"

MViewAltF12
""
""
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
""
""

MViewCtrlF2
""
""
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
""
""

MViewCtrlF4
""
""
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
""
""

MViewCtrlF6
""
""
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
""
""

MViewCtrlF8
""
""
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
""
""

MViewCtrlF10
"Позиц"
"GoFile"
"JítSou"
"GehDat"
"FájlPz"
"DoPlik"
"IrArch"
"ÍsťSúbor"

MViewCtrlF11
""
""
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
""
""

MViewAltShiftF2
""
""
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
""
""

MViewAltShiftF4
""
""
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
""
""

MViewAltShiftF6
""
""
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
""
""

MViewAltShiftF8
""
""
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
"Beáll."
"Konfig"
"Config"
"Konfig"

MViewAltShiftF10
""
""
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
""
""

MViewAltShiftF12
""
""
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
""
""

MViewCtrlShiftF2
""
""
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
""
""

MViewCtrlShiftF4
""
""
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
""
""

MViewCtrlShiftF6
""
""
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
""
""

MViewCtrlShiftF8
""
""
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
""
""

MViewCtrlShiftF10
""
""
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
""
""

MViewCtrlShiftF12
""
""
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
""
""

MViewCtrlAltF2
""
""
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
""
""

MViewCtrlAltF4
""
""
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
""
""

MViewCtrlAltF6
""
""
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
""
""

MViewCtrlAltF8
""
""
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
""
""

MViewCtrlAltF10
""
""
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
""
""

MViewCtrlAltF12
""
""
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
""
""

MViewCtrlAltShiftF2
""
""
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
""
""

MViewCtrlAltShiftF4
""
""
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
""
""

MViewCtrlAltShiftF6
""
""
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
""
""

MViewCtrlAltShiftF8
""
""
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
""
""

MViewCtrlAltShiftF10
""
""
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
"Pomoc"
"Ayuda"
"Pomocník"

MSingleViewF2
"Сверн"
"Wrap"
"Zalom"
"Umbr."
"SorTör"
"Zawiń"
"Divide"
"Zalomiť"

MSingleViewF3
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"
"Koniec"
"Quitar"
"Koniec"

MSingleViewF4
"Код"
"Hex"
"Hex"
"Hex"
"Hexa"
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
""
""

MSingleViewF6
"Редакт"
"Edit"
"Edit"
"Bearb"
"Szerk."
"Edytuj"
"Editar"
"Upraviť"

MSingleViewF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"
"Szukaj"
"Buscar"
"Hľadať"

MSingleViewF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"Latin 2"
"ANSI"
"WIN"

MSingleViewF9
""
""
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
"Kilép"
"Koniec"
"Quitar"
"Koniec"

MSingleViewF11
"Плагины"
"Plugins"
"Plugin"
"Plugins"
"Plugin"
"Pluginy"
"Complementos"
"Moduly"

MSingleViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"
"Ekran"
"Pantal"
"Okno"

MSingleViewF2Unwrap
l:// this is another text for F2
"Развер"
"Unwrap"
"Nezal"
"KeinUm"
"NemTör"
"Rozwij"
"Unwrap"
"Nezalom."

MSingleViewF4Text
l:// this is another text for F4
"Текст"
"Text"
"Text"
"Text"
"Szöveg"
"Tekst"
"Texto"
"Text"

MSingleViewF8DOS
"OEM"
"OEM"
"OEM"
"OEM"
"OEM"
"CP 1250"
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
""
""
""

MSingleViewShiftF2
"Слова"
"WWrap"
"ZalSlo"
"WUmbr"
"SzóTör"
"ZawińS"
"ConDiv"
"ZalSlo"

MSingleViewShiftF3
""
""
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
""
""

MSingleViewShiftF5
""
""
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
""
""

MSingleViewShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"
"Nast."
"Próxim"
"Nasl."

MSingleViewShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"
"Tabela"
"PágCód"
"TabZn"

MSingleViewShiftF9
""
""
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
""
""

MSingleViewShiftF11
""
""
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
""
""

MSingleViewAltF2
""
""
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
""
""

MSingleViewAltF4
""
""
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
"Imprim"
"Tlač"

MSingleViewAltF6
""
""
""
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
"Poprz"
"Prev"
"Predch"

MSingleViewAltF8
"Перейт"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"
"IdźDo"
"Ir a.."
"Ísť na"

MSingleViewAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"
"Video"
"Video"

MSingleViewAltF10
""
""
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
"NézElő"
"Historia"
"VerHis"
"ZobrHs"

MSingleViewAltF12
""
""
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
""
""

MSingleViewCtrlF2
""
""
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
""
""

MSingleViewCtrlF4
""
""
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
""
""

MSingleViewCtrlF6
""
""
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
""
""

MSingleViewCtrlF8
""
""
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
""
""

MSingleViewCtrlF10
""
""
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
""
""

MSingleViewCtrlF12
""
""
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
""
""

MSingleViewAltShiftF2
""
""
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
""
""

MSingleViewAltShiftF4
""
""
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
""
""

MSingleViewAltShiftF6
""
""
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
""
""

MSingleViewAltShiftF8
""
""
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
"Beáll."
"Konfig"
"Config"
"Konfig"

MSingleViewAltShiftF10
""
""
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
""
""

MSingleViewAltShiftF12
""
""
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
""
""

MSingleViewCtrlShiftF2
""
""
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
""
""

MSingleViewCtrlShiftF4
""
""
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
""
""

MSingleViewCtrlShiftF6
""
""
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
""
""

MSingleViewCtrlShiftF8
""
""
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
""
""

MSingleViewCtrlShiftF10
""
""
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
""
""

MSingleViewCtrlShiftF12
""
""
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
""
""

MSingleViewCtrlAltF2
""
""
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
""
""

MSingleViewCtrlAltF4
""
""
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
""
""

MSingleViewCtrlAltF6
""
""
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
""
""

MSingleViewCtrlAltF8
""
""
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
""
""

MSingleViewCtrlAltF10
""
""
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
""
""

MSingleViewCtrlAltF12
""
""
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
""
""

MSingleViewCtrlAltShiftF2
""
""
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
""
""

MSingleViewCtrlAltShiftF4
""
""
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
""
""

MSingleViewCtrlAltShiftF6
""
""
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
""
""

MSingleViewCtrlAltShiftF8
""
""
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
""
""

MSingleViewCtrlAltShiftF10
""
""
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
""
""

MInViewer
"просмотр %1"
"view %1"
"prohlížení %1"
"Betrachte %1"
"%1 megnézése"
"podgląd %1"
"ver %1"
"zobraziť %1"

MInEditor
"редактирование %1"
"edit %1"
"editace %1"
"Bearbeite %1"
"%1 szerkesztése"
"edycja %1"
"editar %1"
"upraviť %1"

MFilterTitle
l:
"Меню фильтров"
"Filters menu"
"Menu filtrů"
"Filtermenü"
"Szűrők menü"
"Filtry"
"Menú de Filtros"
"Menu filtrov"

MFilterBottom
"+,-,Пробел,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Space,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Mezera,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Leer,I,X,BS,UmschBS,Einf,Entf,F4,F5,StrgUp,StrgDn"
"+,-,Szóköz,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Fel,Ctrl-Le"
"+,-,Spacja,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Space,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"
"+,-,Medzera,I,X,BS,Shift-BS,Ins,Del,F4,F5,Ctrl-Up,Ctrl-Dn"

MPanelFileType
"Файлы панели"
"Panel file type"
"Typ panelu souborů"
"Dateityp in Panel"
"A panel fájltípusa"
"Typ plików w panelu"
"Tipo de panel de archivo"
"Typ panela súborov"

MFolderFileType
"Папки"
"Folders"
"Adresáře"
"Ordner"
"Mappák"
"Foldery"
"Directorios"
"Priečinky"

MCanEditCustomFilterOnly
"Только пользовательский фильтр можно редактировать"
"Only custom filter can be edited"
"Jedině vlastní filtr může být upraven"
"Nur eigene Filter können editiert werden."
"Csak saját szűrő szerkeszthető"
"Tylko filtr użytkownika może być edytowany"
"Sólo filtro personalizado puede ser editado"
"Len vlastný filter sa dá upraviť"

MAskDeleteFilter
"Вы хотите удалить фильтр"
"Do you wish to delete the filter"
"Přejete si smazat filtr"
"Wollen Sie den eigenen Filter löschen"
"Törölni szeretné a szűrőt?"
"Czy chcesz usunąć filtr"
"Desea borrar el filtro"
"Chcete zmazať filter"

MCanDeleteCustomFilterOnly
"Только пользовательский фильтр может быть удалён"
"Only custom filter can be deleted"
"Jedině vlastní filtr může být smazán"
"Nur eigene Filter können gelöscht werden."
"Csak saját szűrő törölhető"
"Tylko filtr użytkownika może być usunięty"
"Sólo filtro personalizado puede ser borrado"
"Len vlastný filter sa dá zmazať"

MFindFileTitle
l:
"Поиск файла"
"Find file"
"Hledat soubor"
"Nach Dateien suchen"
"Fájlkeresés"
"Znajdź plik"
"Encontrar archivo"
"Hľadať súbor"

MFindFileMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"
"Maska nebo masky souborů:"
"Datei&maske (mehrere getrennt mit Komma):"
"Fájlm&aszk(ok, vesszővel elválasztva):"
"&Maska pliku lub kilka masek oddzielonych przecinkami:"
"&Máscara de archivo o múltiples máscaras de archivos:"
"Maska alebo masky súborov:"

MFindFileText
"&Содержащих текст:"
"Con&taining text:"
"Obsahující te&xt:"
"Enthält &Text:"
"&Tartalmazza a szöveget:"
"Zawierający &tekst:"
"Conteniendo &texto:"
"Obsahujúci te&xt:"

MFindFileHex
"&Содержащих 16-ричный код:"
"Con&taining hex:"
"Obsahující &hex:"
"En&thält Hex (xx xx ...):"
"Tartalmazza a he&xát:"
"Zawierający wartość &szesnastkową:"
"Conteniendo Hexa:"
"Obsahujúci &hex:"

MFindFileCodePage
"Используя кодо&вую страницу:"
"Using code pa&ge:"
upd:"Použít &znakovou sadu:"
upd:"Zeichenta&belle verwenden:"
"Kó&dlap:"
"Użyj tablicy znaków:"
"Usando pá&gina de códigos:"
"Použiť tabuľku &znakov:"

MFindFileCodePageBottom
"Space, Ins"
"Space, Ins"
"Space, Ins"
"Space, Ins"
"Space, Ins"
"Space, Ins"
"Espacio, Ins"
"Medzera, Ins"

MFindFileCase
"&Учитывать регистр"
"&Case sensitive"
"Roz&lišovat velikost písmen"
"Gr&oß-/Kleinschreibung"
"&Nagy/kisbetű érzékeny"
"&Uwzględnij wielkość liter"
"Sensible min/ma&yúsc."
"ma&lé a VEĽKÉ písmená"

MFindFileWholeWords
"Только &целые слова"
"&Whole words"
"&Celá slova"
"Nur &ganze Wörter"
"Csak egés&z szavak"
"Tylko &całe słowa"
"&Palabras completas"
"&Celé slová"

MFindFileAllCodePages
"Все кодовые страницы"
"All code pages"
upd:"Všechny znakové sady"
upd:"Alle Zeichentabellen"
"Minden kódlappal"
"Wszystkie zainstalowane"
"Todas las páginas de códigos"
"Všetky tabuľky znakov"

MFindArchives
"Искать в а&рхивах"
"Search in arch&ives"
"Hledat v a&rchívech"
"In Arch&iven suchen"
"Keresés t&ömörítettekben"
"Szukaj w arc&hiwach"
"Buscar en archivos &comprimidos"
"Hľadať v a&rchívoch"

MFindFolders
"Искать п&апки"
"Search for f&olders"
"Hledat a&dresáře"
"Nach &Ordnern suchen"
"Keresés mapp&ákra"
"Szukaj &folderów"
"Buscar por &directorios"
"Hľadač pr&iečinky"

MFindSymLinks
"Искать в символи&ческих ссылках"
"Search in symbolic lin&ks"
"Hledat v s&ymbolických lincích"
"In symbolischen Lin&ks suchen"
"Keresés sz&imbolikus linkekben"
"Szukaj w &linkach"
"Buscar en enlaces &simbólicos"
"Hľadať v s&ymbol. prepoj."

MSearchForHex
"Искать 16-ричн&ый код"
"Search for &hex"
"Hledat &hex"
"Nach &Hex suchen"
"Keresés &hexákra"
"Szukaj wartości &szesnastkowej"
"Buscar cadena &hexa"
"Hľadať &hex"

MSearchWhere
"Выберите &область поиска:"
"Select search &area:"
upd:"Zvolte oblast hledání:"
upd:"Suchbereich:"
"Keresés hatós&ugara:"
"Obszar wyszukiwania:"
"Seleccionar área de búsqueda:"
"Zvoľte oblasť hľadania:"

MSearchAllDisks
"На всех несъёмных &дисках"
"In &all non-removable drives"
"Ve všech p&evných discích"
"Auf &allen festen Datenträger"
"Minden &fix meghajtón"
"Na dyskach &stałych"
"Buscar en todas las unidades no-removibles"
"na všetkých p&evných diskoch"

MSearchAllButNetwork
"На всех &локальных дисках"
"In all &local drives"
"Ve všech &lokálních discích"
"Auf allen &lokalen Datenträgern"
"Minden hel&yi meghajtón"
"Na dyskach &lokalnych"
"Buscar en todas las unidades locales"
"na všetkých &lokálnych diskoch"

MSearchInPATH
"В PATH-катало&гах"
"In &PATH folders"
"V adresářích z &PATH"
"In &PATH-Ordnern"
"A &PATH mappáiban"
"W folderach zmiennej &PATH"
"En directorios de variable &PATH"
"v adresároch z &PATH"

MSearchFromRootOfDrive
"С кор&ня диска"
"From the &root of"
"V &kořeni"
"Ab Wu&rzelverz. von"
"Meghajtó &gyökerétől:"
"Od &korzenia"
"Buscar desde &raíz de"
"od &koreňa"

MSearchFromRootFolder
"С кор&невой папки"
"From the &root folder"
"V kořeno&vém adresáři"
"Ab Wu&rzelverzeichnis"
"A &gyökérmappától"
"Od katalogu &głównego"
"Buscar desde el directorio &raíz"
"od koreňo&vého priečinka"

MSearchFromCurrent
"С &текущей папки"
"From the curre&nt folder"
"V tomto adresář&i"
"Ab dem aktuelle&n Ordner"
"Az akt&uális mappától"
"Od &bieżącego katalogu"
"Buscar desde directorio actual"
"od tohto prie&činka"

MSearchInCurrent
"Только в теку&щей папке"
"The current folder onl&y"
"P&ouze v tomto adresáři"
"Nur im aktue&llen Ordner"
"&Csak az aktuális mappában"
"&Tylko w bieżącym katalogu"
"Buscar en el directorio actua&l solamente"
"len v &tomto priečinku"

MSearchInSelected
"В &отмеченных папках"
"&Selected folders"
"Ve vy&braných adresářích"
"In au&sgewählten Ordner"
"A ki&jelölt mappákban"
"W &zaznaczonych katalogach"
"Directorios &seleccionados"
"vo vy&braných priečinkoch"

MFindUseFilter
"Исполь&зовать фильтр"
"&Use filter"
"Použít f&iltr"
"Ben&utze Filter"
"Sz&űrővel"
"&Filtruj"
"Usar &filtro"
"Použiť f&ilter"

MFindUsingFilter
"используя фильтр"
"using filter"
"používám filtr"
"mit Filter"
"szűrővel"
"używając filtra"
"usando filtro"
"používam filter"

MFindFileFind
"&Искать"
"&Find"
"&Hledat"
"&Suchen"
"K&eres"
"Szuka&j"
"&Encontrar"
"&Hľadať"

MFindFileDrive
"Дис&к"
"Dri&ve"
"D&isk"
"Lauf&werk"
"Meghajt&ó"
"&Dysk"
"&Unidad"
"D&isk"

MFindFileSetFilter
"&Фильтр"
"Filt&er"
"&Filtr"
"Filt&er"
"Szű&rő"
"&Filtr"
"F&iltro"
"&Filter"

MFindFileAdvanced
"До&полнительно"
"Advance&d"
"Pokr&očilé"
"Er&weitert"
"Ha&ladó"
"&Zaawansowane"
"Avan&zada"
"Pokr&očilé"

MFindSearchingIn
"Поиск%1 в"
"Searching%1 in"
"Hledám%1 v"
"Suche%1 in"
"%1 keresése"
"Szukam%1 w"
"Buscando%1 en:"
"Hľadám%1 v"

MFindNewSearch
"&Новый поиск"
"&New search"
"&Nové hledání"
"&Neue Suche"
"&Új keresés"
"&Od nowa..."
"&Nueva búsqueda"
"&Nové hľadanie"

MFindGoTo
"Пе&рейти"
"&Go to"
"&Jdi na"
"&Gehe zu"
"U&grás"
"&Idź do"
"&Ir a"
"&Ísť na"

MFindView
"&Смотреть"
"&View"
"Zo&braz"
"&Betrachten"
"Meg&néz"
"&Podgląd"
"&Ver "
"Zo&braziť"

MFindPanel
"Пане&ль"
"&Panel"
"&Panel"
"&Panel"
"&Panel"
"&Do panelu"
"&Panel"
"&Panel"

MFindStop
"С&топ"
"&Stop"
"&Stop"
"&Stoppen"
"&Állj"
"&Stop"
"D&etener"
"&Stop"

MFindDone
l:
"Поиск закончен. Найдено файлов: %1, папок: %2"
"Search done. Found files: %1, folders: %2"
"Hledání ukončeno. Nalezeno %1 soubor(ů) a %2 adresář(ů)"
"Suche beendet. %1 Datei(en) und %2 Ordner gefunden."
"A keresés kész. %1 fájlt és %2 mappát találtam."
"Wyszukiwanie zakończone (znalazłem %1 plików i %2 folderów)"
"Búsqueda finalizada. Encontrados %1 archivo(s) y %2 directorio(s)"
"Hľadanie skončilo. Nájdených %1 súbor(ov) a %2 priečink(ov)"

MFindCancel
"Отм&ена"
"&Cancel"
"&Storno"
"Ab&bruch"
"&Mégsem"
"&Anuluj"
"&Cancelar"
"&Storno"

MFindFound
l:
" Файлов: %1, папок: %2 "
" Files: %1, folders: %2 "
" Souborů: %1, adresářů: %2 "
" Dateien: %1, Ordner: %2 "
" Fájlt: %1, mappát: %2 "
" Plików: %1, folderów: %2 "
" Archivos: %1, carpetas: %2 "
" Súborov: %1, priečinkov: %2 "

MFindFileFolder
l:
"Папка"
"Folder"
"Adresář"
"Ordner"
"Mappa"
"Katalog"
" DIR  "
"Priečinok"

MFindFileSymLink
"Ссылка"
"Symlink"
"Link"
"Symlink"
"SzimLnk"
"LinkSym"
"EnlSimb"
"Prepojenie"

MFindFileJunction
"Связь"
"Junction"
"Křížení"
"Knoten"
"Csomópt"
"Dowiązania"
"Unión"
"Kríženie"

MFindFileAdvancedTitle
l:
"Дополнительные параметры поиска"
"Find file advanced options"
"Pokročilé nastavení vyhledávání souborů"
"Erweiterte Optionen"
"Fájlkeresés haladó beállításai"
"Zaawansowane opcje wyszukiwania"
"Opciones avanzada de búsqueda de archivo"
"Pokročilé nastavenie vyhľadávania súborov"

MFindFileSearchFirst
"Проводить поиск в &первых:"
"Search only in the &first:"
"Hledat po&uze v prvních:"
"Nur &in den ersten x Bytes:"
"Keresés csak az első &x bájtban:"
"Szukaj wyłącznie w &pierwszych:"
"Buscar solamente en el &primer:"
"Hľadať &len v prvých:"

MFindFileSearchOutputFormat
"&Формат вывода:"
"&Output format:"
upd:"&Output format:"
upd:"&Output format:"
upd:"&Output format:"
upd:"&Output format:"
"F&ormato de salida:"
"Vý&stupný formát:"

MFindAlternateStreams
"Обрабатывать &альтернативные потоки данных"
"Process &alternate data streams"
upd:"Process &alternate data streams"
upd:"Process &alternate data streams"
"&Alternatív adatsávok (stream) feldolgozása"
upd:"Process &alternate data streams"
"Procesar flujo alternativo de datos"
"Spracovať stried&avé dátové toky"

MFindAlternateModeTypes
"&Типы колонок"
"Column &types"
"&Typ sloupců"
"Spalten&typen"
"Oszlop&típusok"
"&Typy kolumn"
"&Tipos de columna"
"&Typ stĺpcov"

MFindAlternateModeWidths
"&Ширина колонок"
"Column &widths"
"Šíř&ka sloupců"
"Spalten&breiten"
"Oszlop&szélességek"
"&Szerokości kolumn"
"Anc&ho de columna"
"Šír&ka stĺpcov"

MFoldTreeSearch
l:
"Поиск:"
"Search:"
"Hledat:"
"Suchen:"
"Keresés:"
"Wyszukiwanie:"
"Buscar:"
"Hľadať:"

MGetCodePageTitle
l:
"Кодовые страницы"
"Code pages"
upd:"Znakové sady:"
upd:"Tabellen"
"Kódlapok"
"Strony kodowe"
"Página de códigos"
"Tabuľky znakov:"

MGetCodePageSystem
"Системные"
"System"
upd:"System"
upd:"System"
"Rendszer"
upd:"System"
"Sistema"
"Systém"

MGetCodePageUnicode
"Юникод"
"Unicode"
upd:"Unicode"
upd:"Unicode"
"Unicode"
upd:"Unicode"
"Unicode"
"Unicode"

MGetCodePageFavorites
"Избранные"
"Favorites"
upd:"Favorites"
upd:"Favorites"
"Kedvencek"
upd:"Favorites"
"Favoritos"
"Obľúbené"

MGetCodePageOther
"Прочие"
"Other"
upd:"Other"
upd:"Other"
"Egyéb"
upd:"Other"
"Otro"
"Iné"

MGetCodePageBottomTitle
"Ctrl-H, Del, Ins, F4"
"Ctrl-H, Del, Ins, F4"
"Ctrl-H, Del, Ins, F4"
"Strg-H, Entf, Einf, F4"
"Ctrl-H, Del, Ins, F4"
"Ctrl-H, Del, Ins, F4"
"Ctrl-H, Del, Ins, F4"
"Ctrl-H, Del, Ins, F4"

MGetCodePageBottomShortTitle
"Ctrl-H, Del, F4"
"Ctrl-H, Del, F4"
"Ctrl-H, Del, F4"
"Strg-H, Entf, F4"
"Ctrl-H, Del, F4"
"Ctrl-H, Del, F4"
"Ctrl-H, Del, F4"
"Ctrl-H, Del, F4"

MGetCodePageEditCodePageName
"Изменить имя кодовой страницы"
"Edit code page name"
upd:"Edit code page name"
upd:"Edit code page name"
upd:"Edit code page name"
upd:"Edit code page name"
"Editar página de códigos"
"Upraviť názov tabuľky znakov"

MGetCodePageResetCodePageName
"&Сбросить"
"&Reset"
upd:"&Reset"
upd:"&Reset"
upd:"&Reset"
upd:"&Reset"
"&Reiniciar"
"&Reset"

MHighlightTitle
l:
"Раскраска файлов"
"Files highlighting"
"Zvýrazňování souborů"
"Farbmarkierungen"
"Fájlkiemelések, rendezési csoportok"
"Wyróżnianie plików"
"Resaltado de archivos"
"Zvýrazňovanie súborov"

MHighlightBottom
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Nahoru,Ctrl-Dolů"
"Einf,Entf,F4,F5,StrgUp,StrgDown"
"Ins,Del,F4,F5,Ctrl-Fel,Ctrl-Le"
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Up,Ctrl-Down"
"Ins,Del,F4,F5,Ctrl-Nahor,Ctrl-Nadol"

MHighlightUpperSortGroup
"Верхняя группа сортировки"
"Upper sort group"
"Vzesupné řazení"
"Obere Sortiergruppen"
"Felsőbbrendű csoport"
"Górna grupa sortowania"
"Grupo de ordenamiento de arriba"
"Vzostupné triedenie"

MHighlightLowerSortGroup
"Нижняя группа сортировки"
"Lower sort group"
"Sestupné řazení"
"Untere Sortiergruppen"
"Alsóbbrendű csoport"
"Dolna grupa sortowania"
"Grupo de ordenamiento de abajo"
"Zostupné triedenie"

MHighlightLastGroup
"Наименее приоритетная группа раскраски"
"Lowest priority highlighting group"
"Zvýraznění nejnižší prority"
"Farbmarkierungen mit niedrigster Priorität"
"Legalacsonyabb rendű csoport"
"Grupa wyróżniania o najniższym priorytecie"
"Resaltado de grupo con baja prioridad"
"Zvýraznenie najnižšej priority"

MHighlightAskDel
"Вы хотите удалить раскраску для"
"Do you wish to delete highlighting for"
"Přejete si smazat zvýraznění pro"
"Wollen Sie Farbmarkierungen löschen für"
"Biztosan törli a kiemelést?"
"Czy chcesz usunąć wyróżnianie dla"
"Desea borrar resaltado para"
"Chcete zmazať zvýraznenie pre"

MHighlightWarning
"Будут потеряны все Ваши настройки"
"You will lose all changes"
"Všechny změny budou ztraceny"
"Sie verlieren jegliche Änderungen"
"Minden változtatás elvész"
"Wszystkie zmiany zostaną utracone"
"Usted perderá todos los cambios"
"Všetky zmeny sa stratia"

MHighlightAskRestore
"Вы хотите восстановить раскраску файлов по умолчанию?"
"Do you wish to restore default highlighting?"
"Přejete si obnovit výchozí nastavení?"
"Wollen Sie Standard-Farbmarkierungen wiederherstellen?"
"Visszaállítja az alapértelmezett kiemeléseket?"
"Czy przywrócić wyróżnianie domyślne?"
"Desea restablecer resaltado por defecto?"
"Chcete obnoviť východiskové zvýrazňovanie?"

MHighlightEditTitle
l:
"Редактирование раскраски файлов"
"Edit files highlighting"
"Upravit zvýrazňování souborů"
"Farbmarkierungen bearbeiten"
"Fájlkiemelés szerkesztése"
"Edytuj wyróżnianie plików"
"Editar resaltado de archivos"
"Upraviť zvýrazňovanie súborov"

MHighlightMarkChar
"Оп&циональный символ пометки,"
"Optional markin&g character,"
"Volitelný &znak pro označení určených souborů,"
"Optionale Markierun&g mit Zeichen,"
"Megadható &jelölő karakter"
"Opcjonalny znak &wyróżniający zaznaczone pliki,"
"Ca&racter opcional para marcar,"
"Voliteľný &znak označovania,"

MHighlightTransparentMarkChar
"прозра&чный"
"tra&nsparent"
"průh&ledný"
"tra&nsparent"
"át&látszó"
"prze&zroczyste"
"tra&nsparente"
"prieh&ľadný"

MHighlightColors
" Цвета файлов (\"чёрный на чёрном\" - цвет по умолчанию) "
" File name colors (\"black on black\" - default color) "
" Barva názvu souborů (\"černá na černé\" - výchozí barva) "
" Dateinamenfarben (\"Schwarz auf Schwarz\"=Standard) "
" Fájlnév színek (feketén fekete = alapértelmezett szín) "
" Kolory nazw plików (domyślny - \"czarny na czarnym\") "
" Colores de archivos (\"negro en negro\" - color por defecto) "
" Farba názvov súborov (\"čierna na čiernej\" - východisková) "

MHighlightFileName1
"&1. Обычное имя файла                "
"&1. Normal file name               "
"&1. Normální soubor            "
"&1. Normaler Dateiname             "
"&1. Normál fájlnév                  "
"&1. Nazwa pliku bez zaznaczenia    "
"&1. Normal                  "
"&1. Normálny súbor            "

MHighlightFileName2
"&3. Помеченное имя файла             "
"&3. Selected file name             "
"&3. Vybraný soubor             "
"&3. Markierter Dateiame            "
"&3. Kijelölt fájlnév                "
"&3. Zaznaczenie                    "
"&3. Seleccionado            "
"&3. Vybraný súbor             "

MHighlightFileName3
"&5. Имя файла под курсором           "
"&5. File name under cursor         "
"&5. Soubor pod kurzorem        "
"&5. Dateiname unter Cursor         "
"&5. Kurzor alatti fájlnév           "
"&5. Nazwa pliku pod kursorem       "
"&5. Bajo cursor             "
"&5. Súbor pod kurzorom        "

MHighlightFileName4
"&7. Помеченное под курсором имя файла"
"&7. File name selected under cursor"
"&7. Vybraný soubor pod kurzorem"
"&7. Dateiname markiert unter Cursor"
"&7. Kurzor alatti kijelölt fájlnév  "
"&7. Zaznaczony plik pod kursorem   "
"&7. Seleccionado bajo cursor"
"&7. Vybraný súbor pod kurzorom"

MHighlightMarking1
"&2. Пометка"
"&2. Marking"
"&2. Označení"
"&2. Markierung"
"&2. Jelölő kar.:"
"&2. Zaznaczenie"
"&2. Marcado"
"&2. Označenie"

MHighlightMarking2
"&4. Пометка"
"&4. Marking"
"&4. Označení"
"&4. Markierung"
"&4. Jelölő kar.:"
"&4. Zaznaczenie"
"&4. Marcado"
"&4. Označenie"

MHighlightMarking3
"&6. Пометка"
"&6. Marking"
"&6. Označení"
"&6. Markierung"
"&6. Jelölő kar.:"
"&6. Zaznaczenie"
"&6. Marcado"
"&6. Označenie"

MHighlightMarking4
"&8. Пометка"
"&8. Marking"
"&8. Označení"
"&8. Markierung"
"&8. Jelölő kar.:"
"&8. Zaznaczenie"
"&8. Marcado"
"&8. Označenie"

MHighlightExample1
"║filename.ext │"
"║filename.ext │"
"║filename.ext │"
"║dateinam.erw │"
"║fájlneve.kit │"
"║nazwa.roz    │"
"║nombre.ext   │"
"║filename.ext │"

MHighlightExample2
"║ filename.ext│"
"║ filename.ext│"
"║ filename.ext│"
"║ dateinam.erw│"
"║ fájlneve.kit│"
"║ nazwa.roz   │"
"║ nombre.ext  │"
"║ filename.ext│"

MHighlightContinueProcessing
"Продолжать &обработку"
"C&ontinue processing"
"Pokračovat ve zpracová&ní"
"Verarbeitung f&ortsetzen"
"Folyamatos f&eldolgozás"
"K&ontynuuj przetwarzanie"
"Contin&uar procesando"
"Pokračovať v spracúva&ní"

MInfoTitle
l:
"Информация"
"Information"
"Informace"
"Informationen"
"Információk"
"Informacja"
"Información"
"Informácie"

MInfoCompName
"Имя компьютера"
"Computer name"
"Název počítače"
"Computername"
"Számítógép neve"
"Nazwa komputera"
"Nombre computadora"
"Názov počítača"

MInfoCompDescription
"Описание компьютера"
"Computer description"
upd:"Computer description"
upd:"Computer description"
upd:"Computer description"
upd:"Computer description"
"Descripción de computadora"
"Opis počítača"

MInfoUserName
"Имя пользователя"
"User name"
"Jméno uživatele"
"Benutzername"
"Felhasználói név"
"Nazwa użytkownika"
"Nombre de usuario"
"Meno užívateľa"

MInfoUserDescription
"Описание пользователя"
"User description"
upd:"User description"
upd:"User description"
upd:"User description"
upd:"User description"
"Descripción de usuario"
"Opis užívateľa"

MInfoUserAccessLevel
"Уровень доступа"
"Access level"
upd:"Access level"
upd:"Access level"
upd:"Access level"
upd:"Access level"
"Nivel de acceso"
"Úroveň prístupu"

MInfoUserAccessLevelGuest
"Гость"
"Guest"
upd:"Guest"
upd:"Guest"
upd:"Guest"
upd:"Guest"
"Invitado"
"Hosť"

MInfoUserAccessLevelUser
"Пользователь"
"User"
upd:"User"
upd:"User"
upd:"User"
upd:"User"
"Usuario"
"Užívateľ"

MInfoUserAccessLevelAdministrator
"Администратор"
"Administrator"
upd:"Administrator"
upd:"Administrator"
upd:"Administrator"
upd:"Administrator"
"Administrador"
"Správca"

MInfoUserAccessLevelUnknown
"Неизвестно"
"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
"Desconocido"
"Neznámy"

MInfoDiskTitle
" Диск "
" Disk "
" Disk "
" Laufwerk "
" Lemez "
" Dysk "
" Disco "
" Disk "

MInfoRemovable
"Сменный"
"Removable"
"Vyměnitelný"
"Austauschbares"
"Kivehető"
"Wyjmowalny"
"Removible"
"Vymeniteľný"

MInfoFixed
"Жёсткий"
"Fixed"
"Pevný"
"Lokales"
"Fix"
"Stały"
"Rígido"
"Pevný"

MInfoNetwork
"Сетевой"
"Network"
"Síťový"
"Netzwerk"
"Hálózati"
"Sieciowy"
"Red"
"Sieťový"

MInfoCDROM
"CD-ROM"
"CD-ROM"
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
"CD-RW"
"CD-RW"

MInfoCD_RWDVD
"CD-RW/DVD"
"CD-RW/DVD"
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
"DVD-ROM"
"DVD-ROM"

MInfoDVD_RW
"DVD-RW"
"DVD-RW"
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
"DVD-RAM"
"DVD-RAM"

MInfoRAM
"RAM"
"RAM"
"RAM"
"RAM"
"RAM"
"RAM"
"RAM"
"RAM"

MInfoSUBST
"Subst"
"Subst"
"Subst"
"Subst"
"Subst"
"Subst"
"Subst"
"Subst"

MInfoVirtual
"Виртуальный"
"Virtual"
upd:"Virtual"
upd:"Virtual"
upd:"Virtual"
upd:"Virtual"
"Virtual"
"Virtuálny"

MInfoUnknown
"Неизвестный"
"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
"Desconocido"
"Neznámy"

MInfoDisk
"диск"
"disk"
"disk"
"Laufwerk"
"lemez"
"dysk"
"disco"
"disk"

MInfoDiskTotal
"Всего байтов"
"Total bytes"
"Celkem bytů"
"Bytes gesamt"
"Összes bájt"
"Razem bajtów"
"Total de bytes"
"Bajtov spolu"

MInfoDiskFree
"Свободных байтов"
"Free bytes"
"Volných bytů"
"Bytes frei"
"Szabad bájt"
"Wolnych bajtów"
"Bytes libres"
"Bajtov voľných"

MInfoDiskLabel
"Метка тома"
"Volume label"
"Popisek disku"
"Laufwerksbezeichnung"
"Kötet címke"
"Etykieta woluminu"
"Etiqueta de volumen"
"Názov disku"

MInfoDiskNumber
"Серийный номер"
"Serial number"
"Sériové číslo"
"Seriennummer"
"Sorozatszám"
"Numer seryjny"
"Número de serie"
"Sériové číslo"

MInfoMemory
" Память "
" Memory "
" Paměť "
" Speicher "
" Memória "
" Pamięć "
" Memoria "
" Pamäť "

MInfoMemoryLoad
"Загрузка памяти"
"Memory load"
"Zatížení paměti"
"Speicherverbrauch"
"Használt memória"
"Użycie pamięci"
"Carga en Memoria"
"Obsadenie pamäte"

MInfoMemoryInstalled
"Установлено памяти"
"Installed memory"
upd:"Installed memory"
upd:"Installed memory"
upd:"Installed memory"
upd:"Installed memory"
"Memoria instalada"
"Nainštalovaná pamäť"

MInfoMemoryTotal
"Всего памяти"
"Total memory"
"Celková paměť"
"Speicher gesamt"
"Összes memória"
"Całkowita pamięć"
"Total memoria"
"Pamäť spolu"

MInfoMemoryFree
"Свободно памяти"
"Free memory"
"Volná paměť"
"Speicher frei"
"Szabad memória"
"Wolna pamięć"
"Memoria libre"
"Pamäť voľná"

MInfoVirtualTotal
"Всего вирт. памяти"
"Total virtual"
"Celkem virtuální"
"Virtueller Speicher gesamt"
"Összes virtuális"
"Całkowita wirtualna"
"Total virtual"
"Virtuálna spolu"

MInfoVirtualFree
"Свободно вирт. памяти"
"Free virtual"
"Volná virtuální"
"Virtueller Speicher frei"
"Szabad virtuális"
"Wolna wirtualna"
"Virtual libre"
"Virtuálna voľná"

MInfoPageFileTotal
"Всего файла подкачки"
"Total paging file"
upd:"Total paging file"
upd:"Total paging file"
upd:"Total paging file"
upd:"Total paging file"
"Archivo de paginación total"
"Stránkovací súbor spolu"

MInfoPageFileFree
"Свободно файла подкачки"
"Free paging file"
upd:"Free paging file"
upd:"Free paging file"
upd:"Free paging file"
upd:"Free paging file"
"Archivo de paginación libre"
"Stránkovací súbor voľný"

MInfoDescription
" Описание "
" Description "
" Popis "
" Beschreibung "
" Megjegyzés "
" Opis "
" Descripción "
" Popis "

MInfoDizAbsent
"Файл описания папки отсутствует"
"Folder description file is absent"
"Soubor s popisem adresáře chybí"
"Keine Datei mit Ordnerbeschreibungen vorhanden."
"Mappa megjegyzésfájl nincs"
"Plik opisu katalogu nie istnieje"
"archivo descripción del directorio está ausente"
"Chýba popisný súbor priečinka"

MInfoPlugin
" Плагин "
" Plugin "
" Plugin "
" Plugin "
" Plugin "
" Plugin "
" Complemento "
" Modul "

MInfoPowerStatus
" Питание "
" Power Status"
upd:" Power Status"
upd:" Power Status"
upd:" Power Status"
upd:" Power Status"
" Estado de encendido"
" Stav napájania"

MInfoPowerStatusAC
"Подключения к сети"
"AC power status"
upd:"AC power status"
upd:"AC power status"
upd:"AC power status"
upd:"AC power status"
"Estado encendido AC"
"Stav sieťového napájania"

MInfoPowerStatusACOffline
"Отсутствует"
"Offline"
upd:"Offline"
upd:"Offline"
upd:"Offline"
upd:"Offline"
"Offline"
"Offline"

MInfoPowerStatusACOnline
"Подключено"
"Online"
upd:"Online"
upd:"Online"
upd:"Online"
upd:"Online"
"Online"
"Online"

MInfoPowerStatusACBackUp
upd:"Backup power"
"Backup power"
upd:"Backup power"
upd:"Backup power"
upd:"Backup power"
upd:"Backup power"
"Backup power"
"Záložné napájanie"

MInfoPowerStatusACUnknown
"Не определено"
"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
"Unknown"
"Neznámy"

MInfoPowerStatusBCLifePercent
"Заряд батареи"
"Battery life percent"
upd:"Battery life percent"
upd:"Battery life percent"
upd:"Battery life percent"
upd:"Battery life percent"
"Battery life percent"
"Stav akumulátora v percentách"

MInfoPowerStatusBCLifePercentUnknown
"Не определено"
"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
"Unknown"
"Neznámy"

MInfoPowerStatusBC
"Статус заряда батареи"
"Battery charge status"
upd:"Battery charge status"
upd:"Battery charge status"
upd:"Battery charge status"
upd:"Battery charge status"
"Battery charge status"
"Kapacita akumulátora"

MInfoPowerStatusBCHigh
"Высокий"
"High"
upd:"High"
upd:"High"
upd:"High"
upd:"High"
"High"
"Vysoká"

MInfoPowerStatusBCLow
"Низкий"
"Low"
upd:"Low"
upd:"Low"
upd:"Low"
upd:"Low"
"Low"
"Nízka"

MInfoPowerStatusBCCritical
"Критичный"
"Critical"
upd:"Critical"
upd:"Critical"
upd:"Critical"
upd:"Critical"
"Critical"
"Kritická"

MInfoPowerStatusBCCharging
"Зарядка"
"Charging"
upd:"Charging"
upd:"Charging"
upd:"Charging"
upd:"Charging"
"Charging"
"Nabíjanie"

MInfoPowerStatusBCNoSysBat
"Батареи нет"
"No system battery"
upd:"No system battery"
upd:"No system battery"
upd:"No system battery"
upd:"No system battery"
"No system battery"
"Chýba systémový akumulátor"

MInfoPowerStatusBCUnknown
"Не определено"
"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
"Unknown"
"Neznámy"

MInfoPowerStatusBCTimeRem
"Время до разряда"
"Battery life time"
upd:"Battery life time"
upd:"Battery life time"
upd:"Battery life time"
upd:"Battery life time"
"Battery life time"
"Doba práce z akumulátora"

MInfoPowerStatusBCTMUnknown
"Не определено"
"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
"Unknown"
"Neznámy"

MInfoPowerStatusBCFullTimeRem
"Полное время работы"
"Battery full time"
upd:"Battery full time"
upd:"Battery full time"
upd:"Battery full time"
upd:"Battery full time"
"Battery full time"
"Plná doba práce z akumulátora"

MInfoPowerStatusBCFTMUnknown
"Не определено"
"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
upd:"Unknown"
"Unknown"
"Neznámy"

MAccessDenied
"Доступ запрещён"
"Access denied"
"Přístup odepřen"
"Zugriff verweigert"
"Hozzáférés megtagadva"
"Dostęp zabroniony"
"Acceso denegado"
"Prístup odmietnutý"

MCannotExecute
l:
"Ошибка выполнения"
"Cannot execute"
"Nelze provést"
"Fehler beim Ausführen von"
"Nem végrehajtható:"
"Nie mogę wykonać"
"No se puede ejecutar"
"Nedá sa vykonať"

MCannotInvokeComspec
"Ошибка вызова командного интерпретатора"
"Cannot invoke command interpreter"
upd:"Cannot invoke command interpreter"
upd:"Cannot invoke command interpreter"
upd:"Cannot invoke command interpreter"
upd:"Cannot invoke command interpreter"
"No se puede invocar intérprete de comando"
"Nemôžem vyvolať interpreta príkazov"

MCheckComspecVar
"Проверьте переменную окружения COMSPEC"
"Check COMSPEC environment variable"
upd:"Check COMSPEC environment variable"
upd:"Check COMSPEC environment variable"
upd:"Check COMSPEC environment variable"
upd:"Check COMSPEC environment variable"
"Compruebe variable de entorno COMSPEC"
"Skontrolujte premennú prostredia COMSPEC"

MScanningFolder
"Просмотр папки"
"Scanning the folder"
"Prohledávám adresář"
"Scanne den Ordner"
"Mappák olvasása..."
"Przeszukuję katalog"
"Explorando el directorio"
"Prehľadávam priečinok"

MMakeFolderTitle
l:
"Создание папки"
"Make folder"
"Vytvoření adresáře"
"Ordner erstellen"
"Új mappa létrehozása"
"Utwórz katalog"
"Crear directorio"
"Vytvorenie priečinka"

MCreateFolder
"Создать п&апку:"
"Create the &folder:"
"Vytvořit &adresář:"
"Diesen &Ordner erstellen:"
"Mappa &neve:"
"Nazwa katalogu:"
"Nombre del directorio:"
"Vytvoriť &priečinok:"

MMakeFolderLinkType
"Тип ссылки:"
"Link type:"
"Typ linku:"
"Linktyp:"
"Link típusa:"
"Typ linku:"
"Tipo de enlace:"
"Typ prepojenia:"

MMakeFolderLinkNone
"Нет"
"None"
upd:"None"
upd:"None"
upd:"None"
upd:"None"
"Nada"
"Žiadny"

MMakeFolderLinkJunction
"связь каталогов"
"directory junction"
"křížení adresářů"
"Ordnerknotenpunkt"
"Mappa csomópont"
"directory junction"
"unión de directorio"
"kríženie priečinkov"

MMakeFolderLinkSymlink
"символическая ссылка"
"symbolic link"
"symbolický link"
"Symbolischer Link"
"Szimbolikus link"
"link symboliczny"
"enlace simbólico"
"symbolické prepojenie"

MMakeFolderLinkTarget
"Цель:"
"Target:"
upd:"Target:"
upd:"Target:"
upd:"Target:"
upd:"Target:"
"Objetivo:"
"Cieľ:"

MMultiMakeDir
"Обрабатыват&ь несколько имён папок"
"Process &multiple names"
"Zpracovat &více názvů"
"&Mehrere Namen verarbeiten (getrennt durch Semikolon)"
"Töb&b név feldolgozása"
"Przetwarzaj &wiele nazw"
"Procesar &múltiples nombres"
"Spracovať &viac názvov"

MIncorrectDirList
"Неправильный список папок"
"Incorrect folders list"
"Neplatný seznam adresářů"
"Fehlerhafte Ordnerliste"
"Hibás mappalista"
"Błędna lista folderów"
"Listado de directorios incorrecto"
"Neplatný zoznam priečinkov"

MCannotCreateFolder
"Ошибка создания папки"
"Cannot create the folder"
"Adresář nelze vytvořit"
"Konnte den Ordner nicht erstellen"
"A mappa nem hozható létre"
"Nie mogę utworzyć katalogu"
"No se puede crear el directorio"
"Nemôžem vytvoriť priečinok"

MMenuBriefView
l:
"&Краткий                  LCtrl-1"
"&Brief              LCtrl-1"
"&Stručný                  LCtrl-1"
"&Kurz                 LStrg-1"
"&Rövid              BalCtrl-1"
"&Skrótowy             LCtrl-1"
"&Breve                 LCtrl-1"
"&Stručný                  LCtrl-1"

MMenuMediumView
"&Средний                  LCtrl-2"
"&Medium             LCtrl-2"
"S&třední                  LCtrl-2"
"&Mittel               LStrg-2"
"&Közepes            BalCtrl-2"
"Ś&redni               LCtrl-2"
"&Medio                 LCtrl-2"
"S&tredný                  LCtrl-2"

MMenuFullView
"&Полный                   LCtrl-3"
"&Full               LCtrl-3"
"&Plný                     LCtrl-3"
"&Voll                 LStrg-3"
"&Teljes             BalCtrl-3"
"&Pełny                LCtrl-3"
"&Completo              LCtrl-3"
"Ú&plný                    LCtrl-3"

MMenuWideView
"&Широкий                  LCtrl-4"
"&Wide               LCtrl-4"
"Š&iroký                   LCtrl-4"
"B&reitformat          LStrg-4"
"&Széles             BalCtrl-4"
"S&zeroki              LCtrl-4"
"&Amplio                LCtrl-4"
"Š&iroký                   LCtrl-4"

MMenuDetailedView
"&Детальный                LCtrl-5"
"Detai&led           LCtrl-5"
"Detai&lní                 LCtrl-5"
"Detai&lliert          LStrg-5"
"Rész&letes          BalCtrl-5"
"Ze sz&czegółami       LCtrl-5"
"De&tallado             LCtrl-5"
"Pod&robný                 LCtrl-5"

MMenuDizView
"&Описания                 LCtrl-6"
"&Descriptions       LCtrl-6"
"P&opisky                  LCtrl-6"
"&Beschreibungen       LStrg-6"
"Fájl&megjegyzések   BalCtrl-6"
"&Opisy                LCtrl-6"
"&Descripción           LCtrl-6"
"S p&opismi                LCtrl-6"

MMenuLongDizView
"Д&линные описания         LCtrl-7"
"Lon&g descriptions  LCtrl-7"
"&Dlouhé popisky           LCtrl-7"
"Lan&ge Beschreibungen LStrg-7"
"&Hosszú megjegyzés  BalCtrl-7"
"&Długie opisy         LCtrl-7"
"Descripción lar&ga     LCtrl-7"
"S &dlhými popismi         LCtrl-7"

MMenuOwnersView
"Вл&адельцы файлов         LCtrl-8"
"File own&ers        LCtrl-8"
"Vlastník so&uboru         LCtrl-8"
"B&esitzer             LStrg-8"
"Fájl tula&jdonos    BalCtrl-8"
"&Właściciele          LCtrl-8"
"Du&eños de archivos    LCtrl-8"
"Vlastníci s&úboru         LCtrl-8"

MMenuLinksView
"Свя&зи файлов             LCtrl-9"
"File lin&ks         LCtrl-9"
"Souborové lin&ky          LCtrl-9"
"Dateilin&ks           LStrg-9"
"Fájl li&nkek        BalCtrl-9"
"Dowiąza&nia           LCtrl-9"
"En&laces               LCtrl-9"
"Prepo&jenia súboru        LCtrl-9"

MMenuAlternativeView
"Аль&тернативный полный    LCtrl-0"
"&Alternative full   LCtrl-0"
"&Alternativní plný        LCtrl-0"
"&Alternativ voll      LStrg-0"
"&Alternatív teljes  BalCtrl-0"
"&Alternatywny         LCtrl-0"
"Alternativo com&pleto  LCtrl-0"
"&Alternatívny úplný       LCtrl-0"

MMenuInfoPanel
l:
"Панель ин&формации        Ctrl-L"
"&Info panel         Ctrl-L"
"Panel In&fo               Ctrl-L"
"&Infopanel            Strg-L"
"&Info panel         Ctrl-L"
"Panel informacy&jny   Ctrl-L"
"Panel &información     Ctrl-L"
"Panel In&fo               Ctrl-L"

MMenuTreePanel
"Де&рево папок             Ctrl-T"
"&Tree panel         Ctrl-T"
"Panel St&rom              Ctrl-T"
"Baumansich&t          Strg-T"
"&Fastruktúra        Ctrl-T"
"Drz&ewo               Ctrl-T"
"Panel árbol           Ctrl-T"
"Pa&nel Strom              Ctrl-T"

MMenuQuickView
"Быстры&й просмотр         Ctrl-Q"
"Quick &view         Ctrl-Q"
"Z&běžné zobrazení         Ctrl-Q"
"Sc&hnellansicht       Strg-Q"
"&Gyorsnézet         Ctrl-Q"
"Sz&ybki podgląd       Ctrl-Q"
"&Vista rápida          Ctrl-Q"
"R&ýchle info              Ctrl-Q"

MMenuSortModes
"Режим&ы сортировки        Ctrl-F12"
"&Sort modes         Ctrl-F12"
"Módy řaze&ní              Ctrl-F12"
"&Sortiermodi          Strg-F12"
"R&endezési elv      Ctrl-F12"
"Try&by sortowania     Ctrl-F12"
"&Ordenar por...        Ctrl-F12"
"Triediť pod&ľa            Ctrl-F12"

MMenuLongNames
"Показывать длинные &имена Ctrl-N"
"Show long &names    Ctrl-N"
"Zobrazit dlouhé názv&y    Ctrl-N"
"Lange Datei&namen     Strg-N"
"H&osszú fájlnevek   Ctrl-N"
"Po&każ długie nazwy   Ctrl-N"
"Ver &nombres largos    Ctrl-N"
"Zobraziť dlhé názv&y      Ctrl-N"

MMenuTogglePanel
"Панель &Вкл/Выкл          Ctrl-F1"
"Panel &On/Off       Ctrl-F1"
"Panel &Zap/Vyp            Ctrl-F1"
"&Panel ein/aus        Strg-F1"
"&Panel be/ki        Ctrl-F1"
"Włącz/Wyłącz pane&l   Ctrl-F1"
"Panel &Si/No           Ctrl-F1"
"Panel &Zap/Vyp            Ctrl-F1"

MMenuReread
"П&еречитать               Ctrl-R"
"&Re-read            Ctrl-R"
"Obno&vit                  Ctrl-R"
"Aktualisie&ren        Strg-R"
"Friss&ítés          Ctrl-R"
"Odśw&ież              Ctrl-R"
"&Releer                Ctrl-R"
"O&bnoviť                  Ctrl-R"

MMenuChangeDrive
"С&менить диск             Alt-F1"
"&Change drive       Alt-F1"
"Z&měnit jednotku          Alt-F1"
"Laufwerk we&chseln    Alt-F1"
"Meghajtó&váltás     Alt-F1"
"Z&mień napęd          Alt-F1"
"Cambiar &unidad        Alt-F1"
"Z&meniť jednotku          Alt-F1"

MMenuView
l:
"&Просмотр              F3"
"&View               F3"
"&Zobrazit                   F3"
"&Betrachten           F3"
"&Megnéz               F3"
"&Podgląd                   F3"
"&Ver                           F3"
"&Zobraziť                   F3"

MMenuEdit
"&Редактирование        F4"
"&Edit               F4"
"&Editovat                   F4"
"B&earbeiten           F4"
"&Szerkeszt            F4"
"&Edytuj                    F4"
"&Editar                        F4"
"&Upraviť                    F4"

MMenuCopy
"&Копирование           F5"
"&Copy               F5"
"&Kopírovat                  F5"
"&Kopieren             F5"
"Más&ol                F5"
"&Kopiuj                    F5"
"&Copiar                        F5"
"&Kopírovať                  F5"

MMenuMove
"П&еренос               F6"
"&Rename or move     F6"
"&Přejmenovat/Přesunout      F6"
"Ve&rschieben/Umben.   F6"
"Át&nevez-Mozgat       F6"
"&Zmień nazwę lub przenieś  F6"
"&Renombrar o mover             F6"
"&Premenovať/Presunúť        F6"

MMenuLink
"Ссы&лка                Alt-F6"
"Lin&k               Alt-F6"
upd:"Link               Alt-F6"
upd:"Link               Alt-F6"
upd:"Link               Alt-F6"
upd:"Link               Alt-F6"
upd:"Link               Alt-F6"
upd:"Link               Alt-F6"

MMenuCreateFolder
"&Создание папки        F7"
"&Make folder        F7"
"&Vytvořit adresář           F7"
"&Ordner erstellen     F7"
"Ú&j mappa             F7"
"U&twórz katalog            F7"
"Crear &directorio              F7"
"&Vytvoriť priečinok         F7"

MMenuDelete
"&Удаление              F8"
"&Delete             F8"
"&Smazat                     F8"
"&Löschen              F8"
"&Töröl                F8"
"&Usuń                      F8"
"&Borrar                        F8"
"Z&mazať                     F8"

MMenuWipe
"Уни&чтожение           Alt-Del"
"&Wipe               Alt-Del"
"&Vymazat                    Alt-Del"
"&Sicher löschen       Alt-Entf"
"&Kisöpör              Alt-Del"
"&Wymaż                     Alt-Del"
"Destr&uir                      Alt-Del"
"&Vymazať                    Alt-Del"

MMenuAdd
"&Архивировать          Shift-F1"
"Add &to archive     Shift-F1"
"Přidat do &archívu          Shift-F1"
"Zu Archiv &hinzuf.    Umsch-F1"
"Tömörhöz ho&zzáad     Shift-F1"
"&Dodaj do archiwum         Shift-F1"
"A&gregar a archivo comprimido  Shift-F1"
"Pridať do &archívu          Shift-F1"

MMenuExtract
"Распако&вать           Shift-F2"
"E&xtract files      Shift-F2"
"&Rozbalit soubory           Shift-F2"
"Archiv e&xtrahieren   Umsch-F2"
"Tömörből ki&bont      Shift-F2"
"&Rozpakuj archiwum         Shift-F2"
"E&xtraer archivos              Shift-F2"
"&Rozbaliť súbory            Shift-F2"

MMenuArchiveCommands
"Архивн&ые команды      Shift-F3"
"Arc&hive commands   Shift-F3"
"Příkazy arc&hívu            Shift-F3"
"Arc&hivbefehle        Umsch-F3"
"Tömörítő &parancsok   Shift-F3"
"Po&lecenie archiwizera     Shift-F3"
"Co&mandos archivo comprimido   Shift-F3"
"Príkaz&y pakovača           Shift-F3"

MMenuAttributes
"А&трибуты файлов       Ctrl-A"
"File &attributes    Ctrl-A"
"A&tributy souboru           Ctrl-A"
"Datei&attribute       Strg-A"
"Fájl &attribútumok    Ctrl-A"
"&Atrybuty pliku            Ctrl-A"
"Cambiar a&tributos             Ctrl-A"
"A&tribúty súboru            Ctrl-A"

MMenuApplyCommand
"Применить коман&ду     Ctrl-G"
"A&pply command      Ctrl-G"
"Ap&likovat příkaz           Ctrl-G"
"Befehl an&wenden      Strg-G"
"Parancs &végrehajtása Ctrl-G"
"Zastosuj pole&cenie        Ctrl-G"
"A&plicar comando               Ctrl-G"
"Po&užiť príkaz              Ctrl-G"

MMenuDescribe
"&Описание файлов       Ctrl-Z"
"Descri&be files     Ctrl-Z"
"Přidat popisek sou&borům    Ctrl-Z"
"Beschrei&bung ändern  Strg-Z"
"Fájlmegje&gyzés       Ctrl-Z"
"&Opisz pliki               Ctrl-Z"
"Describir archi&vos            Ctrl-Z"
"Pridať popis k sú&borom     Ctrl-Z"

MMenuSelectGroup
"Пометить &группу       Gray +"
"Select &group       Gray +"
"Oz&načit skupinu            Num +"
"&Gruppe auswählen     Num +"
"Csoport k&ijelölése   Szürke +"
"Zaznacz &grupę             Szary +"
"&Seleccionar grupo             Gray +"
"Oz&načiť skupinu            Num +"

MMenuUnselectGroup
"С&нять пометку         Gray -"
"U&nselect group     Gray -"
"O&dznačit skupinu           Num -"
"G&ruppe abwählen      Num -"
"Jelölést l&evesz      Szürke -"
"Odz&nacz grupę             Szary -"
"Deseleccio&nar grupo           Gray -"
"O&dznačiť skupinu           Num -"

MMenuInvertSelection
"&Инверсия пометки      Gray *"
"&Invert selection   Gray *"
"&Invertovat výběr           Num *"
"Auswah&l umkehren     Num *"
"Jelölést meg&fordít   Szürke *"
"Od&wróć zaznaczenie        Szary *"
"&Invertir selección            Gray *"
"&Invertovať výber           Num *"

MMenuRestoreSelection
"Восстановить по&метку  Ctrl-M"
"Re&store selection  Ctrl-M"
"&Obnovit výběr              Ctrl-M"
"Auswahl wiederher&st. Strg-M"
"Jel&ölést visszatesz  Ctrl-M"
"Odtwórz zaznaczen&ie       Ctrl-M"
"Rest&ablecer selección         Ctrl-M"
"&Obnoviť výber              Ctrl-M"

MMenuFindFile
l:
"&Поиск файла              Alt-F7"
"&Find file           Alt-F7"
"H&ledat soubor                  Alt-F7"
"Dateien &finden       Alt-F7"
"Fájl&keresés         Alt-F7"
"&Znajdź plik               Alt-F7"
"&Buscar archivos           Alt-F7"
"Ná&jsť súbor                    Alt-F7"

MMenuHistory
"&История команд           Alt-F8"
"&History             Alt-F8"
"&Historie                       Alt-F8"
"&Historie             Alt-F8"
"Parancs &előzmények  Alt-F8"
"&Historia                  Alt-F8"
"&Historial                 Alt-F8"
"&História                       Alt-F8"

MMenuVideoMode
"Видео&режим               Alt-F9"
"&Video mode          Alt-F9"
"&Video mód                      Alt-F9"
"Ansicht<->&Vollbild   Alt-F9"
"&Video mód           Alt-F9"
"&Tryb wyświetlania         Alt-F9"
"Modo de video             Alt-F9"
"&Video mód                      Alt-F9"

MMenuFindFolder
"Поис&к папки              Alt-F10"
"Fi&nd folder         Alt-F10"
"Hl&edat adresář                 Alt-F10"
"Ordner fi&nden        Alt-F10"
"&Mappakeresés        Alt-F10"
"Znajdź kata&log            Alt-F10"
"Buscar &directorio         Alt-F10"
"Hľ&adať priečinok               Alt-F10"

MMenuViewHistory
"Ис&тория просмотра        Alt-F11"
"File vie&w history   Alt-F11"
"Historie &zobrazení souborů     Alt-F11"
"Be&trachterhistorie   Alt-F11"
"Fáj&l előzmények     Alt-F11"
"Historia &podglądu plików  Alt-F11"
"Historial &visor y editor  Alt-F11"
"História &zobrazení súborov     Alt-F11"

MMenuFoldersHistory
"Ист&ория папок            Alt-F12"
"F&olders history     Alt-F12"
"Historie &adresářů              Alt-F12"
"&Ordnerhistorie       Alt-F12"
"Ma&ppa előzmények    Alt-F12"
"Historia &katalogów        Alt-F12"
"Histo&rial directorios     Alt-F12"
"História &priečinkov            Alt-F12"

MMenuSwapPanels
"По&менять панели          Ctrl-U"
"&Swap panels         Ctrl-U"
"Prohodit panel&y                Ctrl-U"
"Panels tau&schen      Strg-U"
"Panel&csere          Ctrl-U"
"Z&amień panele             Ctrl-U"
"I&nvertir paneles          Ctrl-U"
"Vymeniť panel&y                 Ctrl-U"

MMenuTogglePanels
"Панели &Вкл/Выкл          Ctrl-O"
"&Panels On/Off       Ctrl-O"
"&Panely Zap/Vyp                 Ctrl-O"
"&Panels ein/aus       Strg-O"
"Panelek &be/ki       Ctrl-O"
"&Włącz/Wyłącz panele       Ctrl-O"
"&Paneles Si/No             Ctrl-O"
"Pane&ly Zap/Vyp                 Ctrl-O"

MMenuCompareFolders
"&Сравнение папок"
"&Compare folders"
"Po&rovnat adresáře"
"Ordner verglei&chen"
"Mappák össze&hasonlítása"
"Porówna&j katalogi"
"&Compara directorios"
"Po&rovnať priečinky"

MMenuUserMenu
"Меню пользовател&я"
"Edit user &menu"
"Upravit uživatelské &menu"
"Benutzer&menu editieren"
"Felhasználói m&enü szerk."
"Edytuj &menu użytkownika"
"Editar &menú usuario"
"Upraviť užívateľské &menu"

MMenuFileAssociations
"&Ассоциации файлов"
"File &associations"
"Asocia&ce souborů"
"Dat&eiverknüpfungen"
"Fájl&társítások"
"Prz&ypisania plików"
"&Asociar archivos"
"Asociá&cie súborov"

MMenuFolderShortcuts
"Ссы&лки на папки"
"Fol&der shortcuts"
"A&dresářové zkratky"
"Or&dnerschnellzugriff"
"Mappa gyorsbillent&yűk"
"&Skróty katalogów"
"Atajos a dir&ectorios"
"Skratk&y priečinkov"

MMenuFilter
"&Фильтр панели файлов     Ctrl-I"
"File panel f&ilter   Ctrl-I"
"F&iltr panelu souborů           Ctrl-I"
"Panelf&ilter          Strg-I"
"Fájlpanel &szűrők    Ctrl-I"
"&Filtr panelu plików       Ctrl-I"
"F&iltro de paneles         Ctrl-I"
"F&ilter panela súborov          Ctrl-I"

MMenuPluginCommands
"Команды внешних мо&дулей  F11"
"Pl&ugin commands     F11"
"Příkazy plu&ginů                F11"
"Pl&uginbefehle        F11"
"Pl&ugin parancsok    F11"
"Pl&uginy                   F11"
"C&omandos de complemento   F11"
"Príkazy mo&dulov                F11"

MMenuWindowsList
"Список экра&нов           F12"
"Sc&reens list        F12"
"Seznam obrazove&k               F12"
"Seite&nliste          F12"
"Képer&nyők           F12"
"L&ista ekranów             F12"
"&Listado ventanas          F12"
"Zoznam okie&n                   F12"

MMenuProcessList
"Список &задач             Ctrl-W"
"Task &list           Ctrl-W"
"Seznam úl&oh                    Ctrl-W"
"Task&liste            Strg-W"
"Futó p&rogramok      Ctrl-W"
"Lista za&dań               Ctrl-W"
"Lista de &tareas           Ctrl-W"
"Zoznam úl&oh                    Ctrl-W"

MMenuHotPlugList
"Список Hotplug-&устройств"
"Ho&tplug devices list"
"Seznam v&yjímatelných zařízení"
"Sicheres En&tfernen"
"H&otplug eszközök"
"Lista urządzeń Ho&tplug"
"Lista dispositivos hotplu&g"
"Zoznam vy&berateľných zariadení"

MMenuSystemSettings
l:
"Систе&мные параметры"
"S&ystem settings"
"Nastavení S&ystému"
"&Grundeinstellungen"
"&Rendszer beállítások"
"Ustawienia &systemowe"
"&Sistema      "
"S&ystémové nastavenia"

MMenuPanelSettings
"Настройки па&нели"
"&Panel settings"
"Nastavení &Panelů"
"&Panels einrichten"
"&Panel beállítások"
"Ustawienia &panelu"
"&Paneles      "
"Nastavenia &panelov"

MMenuTreeSettings
"Настройки д&ерева папок"
"&Tree settings"
upd:"Tree settings"
upd:"Tree settings"
upd:"Tree settings"
upd:"Tree settings"
"Ar&bol de directorios"
"Nastavenia s&tromu"

MMenuInterface
"Настройки &интерфейса"
"&Interface settings"
"Nastavení Ro&zhraní"
"Oberfläche einr&ichten"
"Kezelő&felület beállítások"
"Ustawienia &interfejsu"
"Interfa&z     "
"Nastavenia ro&zhrania"

MMenuLanguages
"&Языки"
"Lan&guages"
"Nastavení &Jazyka"
"Sprac&hen"
"N&yelvek (Languages)"
"&Język"
"&Idiomas"
"Nastavenia &jazyka"

MMenuPluginsConfig
"Параметры плагино&в"
"Pl&ugins configuration"
"Nastavení Plu&ginů"
"Konfiguration von Pl&ugins"
"Pl&ugin beállítások"
"Konfiguracja p&luginów"
"Configuración de complementos"
"Nastavenia mo&dulov"

MMenuPluginsManagerSettings
"Параметры менеджера плагинов"
"Plugins manager settings"
upd:"Plugins manager settings"
upd:"Plugins manager settings"
upd:"Plugins manager settings"
upd:"Plugins manager settings"
"E&xplorador de complementos"
"Nastavenia správcu modulov"

MMenuDialogSettings
"Настройки &диалогов"
"Di&alog settings"
"Nastavení Dialo&gů"
"Di&aloge einrichten"
"Pár&beszédablak beállítások"
"Ustawienia okna &dialogowego"
"Diá&logo"
"Nastavenia dialó&gov"

MMenuVMenuSettings
"Настройки меню"
"Menu settings"
upd:"Menu settings"
upd:"Menu settings"
upd:"Menu settings"
upd:"Menu settings"
"Menú"
"Nastavenia menu"

MMenuCmdlineSettings
"Настройки &командной строки"
"&Command line settings"
upd:"Command line settings"
upd:"Command line settings"
upd:"Command line settings"
upd:"Command line settings"
"Línea de comando"
"Nastavenia príkazového riadka"

MMenuAutoCompleteSettings
"На&стройки автозавершения"
"AutoComplete settings"
upd:"AutoComplete settings"
upd:"AutoComplete settings"
upd:"AutoComplete settings"
upd:"AutoComplete settings"
"A&utocompletar"
"Nastavenia AutoComplete"

MMenuInfoPanelSettings
"Нас&тройки информационной панели"
"Inf&oPanel settings"
upd:"Inf&oPanel settings"
upd:"Inf&oPanel settings"
upd:"Inf&oPanel settings"
upd:"Inf&oPanel settings"
"Panel de in&formación"
"Nastavenia Inf&oPanela"

MMenuMaskGroups
"Группы масок файлов"
"Groups of file masks"
upd:"Groups of file masks"
upd:"Groups of file masks"
upd:"Groups of file masks"
upd:"Groups of file masks"
"Grupos de máscara de archivos"
"Skupiny masiek súborov"

MMenuConfirmation
"&Подтверждения"
"Co&nfirmations"
"P&otvrzení"
"Bestätigu&ngen"
"Meg&erősítések"
"P&otwierdzenia"
"Co&nfirmaciones"
"P&otvrzovanie"

MMenuFilePanelModes
"Режим&ы панели файлов"
"File panel &modes"
"&Módy souborových panelů"
"Anzeige&modi von Dateipanels"
"Fájlpanel mód&ok"
"&Tryby wyświetlania panelu plików"
"M&odo de paneles de archivos"
"&Módy súborových panelov"

MMenuFileDescriptions
"&Описания файлов"
"File &descriptions"
"Popi&sy souborů"
"&Dateibeschreibungen"
"Fájl &megjegyzésfájlok"
"Opis&y plików"
"&Descripción de archivos"
"Popi&sy súborov"

MMenuFolderInfoFiles
"Файлы описания п&апок"
"&Folder description files"
"Soubory popisů &adresářů"
"O&rdnerbeschreibungen"
"M&appa megjegyzésfájlok"
"Pliki opisu &katalogu"
"&Archivo de descripción de directorios"
"Popisy pr&iečinkov"

MMenuViewer
"Настройки про&граммы просмотра"
"&Viewer settings"
"Nastavení P&rohlížeče"
"Be&trachter einrichten"
"&Nézőke beállítások"
"Ustawienia pod&glądu"
"&Visor "
"Nastavenia zob&razovača"

MMenuEditor
"Настройки &редактора"
"&Editor settings"
"Nastavení &Editoru"
"&Editor einrichten"
"&Szerkesztő beállítások"
"Ustawienia &edytora"
"&Editor "
"Nastavenia &editora"

MMenuCodePages
"Кодов&ые страницы"
upd:"Cod&e pages"
upd:"Znakové sady:"
upd:"Tabellen"
upd:"Kódlapok"
upd:"Strony kodowe"
"Página de códigos"
"Tabuľky znakov:"

MMenuColors
"&Цвета"
"Co&lors"
"&Barvy"
"&Farben"
"S&zínek"
"Kolo&ry"
"&Colores"
"&Farby"

MMenuFilesHighlighting
"Раскраска &файлов и группы сортировки"
"Files &highlighting and sort groups"
"Z&výrazňování souborů a skupiny řazení"
"Farbmar&kierungen und Sortiergruppen"
"Fájlkiemelések, rendezési &csoportok"
"&Wyróżnianie plików"
"&Resaltar archivos y ordenar grupos"
"Z&výrazňovanie súborov a skupiny triedenia"

MMenuSaveSetup
"&Сохранить параметры                  Shift-F9"
"&Save setup                         Shift-F9"
"&Uložit nastavení                      Shift-F9"
"Setup &speichern                     Umsch-F9"
"Beállítások men&tése                 Shift-F9"
"&Zapisz ustawienia       Shift-F9"
"&Guardar configuración          Shift-F9"
"&Uložiť nastavenia        Shift-F9"

MMenuTogglePanelRight
"Панель &Вкл/Выкл          Ctrl-F2"
"Panel &On/Off       Ctrl-F2"
"Panel &Zap/Vyp            Ctrl-F2"
"Panel &ein/aus        Strg-F2"
"Panel be/&ki        Ctrl-F2"
"Włącz/wyłącz pane&l   Ctrl-F2"
"Panel &Si/No           Ctrl-F2"
"Panel &Zap/Vyp            Ctrl-F2"

MMenuChangeDriveRight
"С&менить диск             Alt-F2"
"&Change drive       Alt-F2"
"Z&měnit jednotku          Alt-F2"
"Laufwerk &wechseln    Alt-F2"
"Meghajtó&váltás     Alt-F2"
"Z&mień napęd          Alt-F2"
"Cambiar &unidad        Alt-F2"
"Z&meniť disk              Alt-F2"

MMenuLeftTitle
l:
"&Левая"
"&Left"
"&Levý"
"&Links"
"&Bal"
"&Lewy"
"&Izquierdo"
"&Ľavý"

MMenuFilesTitle
"&Файлы"
"&Files"
"&Soubory"
"&Dateien"
"&Fájlok"
"Pl&iki"
"&Archivos"
"&Súbory"

MMenuCommandsTitle
"&Команды"
"&Commands"
"Pří&kazy"
"&Befehle"
"&Parancsok"
"Pol&ecenia"
"&Comandos"
"Prí&kazy"

MMenuOptionsTitle
"Па&раметры"
"&Options"
"&Nastavení"
"&Optionen"
"B&eállítások"
"&Opcje"
"&Opciones"
"&Nastavenia"

MMenuRightTitle
"&Правая"
"&Right"
"&Pravý"
"&Rechts"
"&Jobb"
"&Prawy"
"&Derecho"
"&Pravý"

MMenuSortTitle
l:
"Критерий сортировки"
"Sort by"
"Seřadit podle"
"Sortieren nach"
"Rendezési elv"
"Sortuj według..."
"Ordenar por"
"Triediť podľa"

MMenuSortByName
"&Имя                              Ctrl-F3"
"&Name                   Ctrl-F3"
"&Názvu                     Ctrl-F3"
"&Name                   Strg-F3"
"&Név                  Ctrl-F3"
"&nazwy                       Ctrl-F3"
"&Nombre                         Ctrl-F3"
"&Názvu                     Ctrl-F3"

MMenuSortByExt
"&Расширение                       Ctrl-F4"
"E&xtension              Ctrl-F4"
"&Přípony                   Ctrl-F4"
"&Erweiterung            Strg-F4"
"Ki&terjesztés         Ctrl-F4"
"ro&zszerzenia                Ctrl-F4"
"E&xtensión                      Ctrl-F4"
"&Prípony                   Ctrl-F4"

MMenuSortByWrite
"Время &записи                     Ctrl-F5"
"&Write time             Ctrl-F5"
upd:"&Write time             Ctrl-F5"
upd:"&Write time             Ctrl-F5"
upd:"&Write time             Ctrl-F5"
upd:"&Write time             Ctrl-F5"
"Fecha &modificación             Ctrl-F5"
"Č&asu zápisu               Ctrl-F5"

MMenuSortBySize
"Р&азмер                           Ctrl-F6"
"&Size                   Ctrl-F6"
"&Velikosti                 Ctrl-F6"
"&Größe                  Strg-F6"
"&Méret                Ctrl-F6"
"&rozmiaru                    Ctrl-F6"
"&Tamaño                         Ctrl-F6"
"&Veľkosti                  Ctrl-F6"

MMenuUnsorted
"&Не сортировать                   Ctrl-F7"
"&Unsorted               Ctrl-F7"
"N&eřadit                   Ctrl-F7"
"&Unsortiert             Strg-F7"
"&Rendezetlen          Ctrl-F7"
"&bez sortowania              Ctrl-F7"
"&Sin ordenar                    Ctrl-F7"
"N&etriediť                 Ctrl-F7"

MMenuSortByCreation
"Время &создания                   Ctrl-F8"
"&Creation time          Ctrl-F8"
"&Data vytvoření            Ctrl-F8"
"E&rstelldatum           Strg-F8"
"Ke&letkezés ideje     Ctrl-F8"
"czasu u&tworzenia            Ctrl-F8"
"Fecha de &creación              Ctrl-F8"
"&Času vytvorenia           Ctrl-F8"

MMenuSortByAccess
"Время &доступа                    Ctrl-F9"
"&Access time            Ctrl-F9"
"Ča&su přístupu             Ctrl-F9"
"&Zugriffsdatum          Strg-F9"
"&Hozzáférés ideje     Ctrl-F9"
"czasu &użycia                Ctrl-F9"
"Fecha de &acceso                Ctrl-F9"
"Ča&su prístupu             Ctrl-F9"

MMenuSortByChange
"Время из&менения"
"Chan&ge time"
upd:"Change time"
upd:"Change time"
upd:"Change time"
upd:"Change time"
"Cambiar &hora"
"Času zmeny"

MMenuSortByDiz
"&Описания                         Ctrl-F10"
"&Descriptions           Ctrl-F10"
"P&opisků                   Ctrl-F10"
"&Beschreibungen         Strg-F10"
"Megjegyzé&sek         Ctrl-F10"
"&opisu                       Ctrl-F10"
"&Descripciones                  Ctrl-F10"
"P&opisov                   Ctrl-F10"

MMenuSortByOwner
"&Владельцы файлов                 Ctrl-F11"
"&Owner                  Ctrl-F11"
"V&lastníka                 Ctrl-F11"
"Bes&itzer               Strg-F11"
"Tula&jdonos           Ctrl-F11"
"&właściciela                 Ctrl-F11"
"Dueñ&o                          Ctrl-F11"
"V&lastníka                 Ctrl-F11"

MMenuSortByAllocatedSize
"Выделенный размер"
"Allocated size"
upd:"Allocated size"
upd:"Allocated size"
upd:"Allocated size"
upd:"Allocated size"
"Tamaño de com&presión"
"Alokovanej veľkosti"

MMenuSortByNumLinks
"Ко&личество ссылок"
"Number of &hard links"
"Poč&tu pevných linků"
"Anzahl an &Links"
"Hardlinkek s&záma"
"&liczby dowiązań"
"Número de enlaces &rígidos"
"Poč&tu pevných prepojení"

MMenuSortByNumStreams
"Количество &потоков"
"Number of st&reams"
upd:"Number of st&reams"
upd:"Number of st&reams"
"Stream-e&k száma"
upd:"Number of st&reams"
"Número de flujos"
"Počtu toko&v"

MMenuSortByStreamsSize
"Размер по&токов"
"Si&ze of streams"
upd:"Si&ze of streams"
upd:"Si&ze of streams"
"Stream-ek m&érete"
upd:"Si&ze of streams"
"Tamaño de flujos"
"Veľkos&ti tokov"

MMenuSortByFullName
"&Полное имя"
"&Full name"
upd:"&Full name"
upd:"&Full name"
upd:"&Full name"
upd:"&Full name"
"Nombre completo"
"&Úplneho názvu"

MMenuSortByCustomData
upd:"Cus&tom data"
"Cus&tom data"
upd:"Cus&tom data"
upd:"Cus&tom data"
upd:"Cus&tom data"
upd:"Cus&tom data"
"Datos opcionales"
"U&žívateľských dát"

MMenuSortUseGroups
"Использовать &группы сортировки   Shift-F11"
"Use sort &groups        Shift-F11"
"Řazení podle skup&in       Shift-F11"
"Sortier&gruppen verw.   Umsch-F11"
"Rend. cs&oport haszn. Shift-F11"
"użyj &grup sortowania        Shift-F11"
"Usar orden/&grupo               Shift-F11"
"Skup&inového triedenia     Shift-F11"

MMenuSortSelectedFirst
"Помеченные &файлы вперёд          Shift-F12"
"Show selected f&irst    Shift-F12"
"Nejdřív zobrazit vy&brané  Shift-F12"
"&Ausgewählte zuerst     Umsch-F12"
"Kijel&ölteket előre   Shift-F12"
"zazna&czone najpierw         Shift-F12"
"Mostrar seleccionados primero  Shift-F12"
"Najprv zobraziť vy&brané   Shift-F12"

MMenuSortDirectoriesFirst
"&Каталоги вперёд"
"Sho&w directories first"
upd:"Sho&w directories first"
upd:"Sho&w directories first"
upd:"Sho&w directories first"
upd:"Sho&w directories first"
"Mostrar directorios primero"
"Najpr&v zobraziť priečinky"

MMenuSortUseNumeric
"&Числовая сортировка"
"Num&eric sort"
"Použít čí&selné řazení"
"Nu&merische Sortierung"
"N&umerikus rendezés"
"Sortuj num&erycznie"
"Usar orden num&érico"
"Použiť čí&selné triedenie"

MMenuSortUseCaseSensitive
"Сортировка с учётом регистра"
"Use case sensitive sort"
"Použít řazení citlivé na velikost písmen"
"Sortierung abhängig von Groß-/Kleinschreibung"
"Nagy/kisbetű érzékeny rendezés"
"Sortuj uwzględniając wielkość liter"
"Usar orden sensible min/mayúsc."
"Triediť podľa malých a VEĽKÝCH písmen"

MMaskGroupBottom
"Редактирование: Ins, Del, F4, F7, Ctrl-R"
"Edit: Ins, Del, F4, F7, Ctrl-R"
"Edit: Ins, Del, F4, F7, Ctrl-R"
"Tasten: Einf, Entf, F4, F7, StrgR"
"Szerk.: Ins, Del, F4, F7, Ctrl-R"
"Edycja: Ins, Del, F4, F7, Ctrl-R"
"Editar: Ins, Del, F4, F7, Ctrl-R"
"Upraviť: Ins, Del, F4, F7, Ctrl-R"

MMaskGroupName
"&Имя:"
"&Name:"
"Jmé&no:"
"&Name:"
"&Neve:"
"&Nazwa:"
"&Nombre:"
"Me&no:"

MMaskGroupMasks
"Одна или несколько &масок файлов:"
"A file &mask or several file masks:"
"&Maska nebo masky souborů:"
"Datei&maske (mehrere getrennt mit Komma):"
"F&ájlmaszk(ok, vesszővel elválasztva):"
"&Maska pliku lub kilka masek oddzielonych przecinkami:"
"&Máscara de archivo o múltiples máscaras de archivos:"
"&Maska alebo masky súborov:"

MMaskGroupAskDelete
"Вы хотите удалить"
"Do you wish to delete"
"Přejete si smazat"
"Wollen Sie folgendes Objekt löschen"
"Törölni akar"
"Czy chcesz usunąć"
"Quiere borrar"
"Chcete zmazať"

MMaskGroupRestore
"Вы хотите восстановить наборы масок по умолчанию?"
"Do you wish to restore default mask sets?"
upd:"Do you wish to restore default mask sets?"
upd:"Do you wish to restore default mask sets?"
upd:"Do you wish to restore default mask sets?"
upd:"Do you wish to restore default mask sets?"
"Quiere restablecer por defecto máscaras?"
"Chcete obnoviť predvolené zostavy masiek?"

MMaskGroupFindMask
"&Маска:"
"&Mask:"
"&Maska"
"&Maske:"
"&Maszk:"
"&Maska:"
"&Máscara:"
"&Maska"

MMaskGroupTotal
"Всего: %1"
"Total: %1"
"Celkem: %1"
"Gesamt: %1"
"Összesen: %1"
"Razem: %1"
"Total: %1"
"Spolu: %1"

MChangeDriveTitle
l:
"Диск"
"Drive"
"Jednotka"
"Laufwerke"
"Meghajtók"
"Napęd"
"Unidad"
"Disk"

MChangeDriveRemovable
"сменный"
"removable"
"vyměnitelná"
"wechsel."
"kivehető"
"wyjmowalny"
"removible"
"vymeniteľný"

MChangeDriveFixed
"жёсткий"
"fixed"
"pevná"
"fest"
"fix"
"stały"
"rígido   "
"pevný"

MChangeDriveNetwork
"сетевой"
"network"
"síťová"
"Netzwerk"
"hálózati"
"sieciowy"
"red      "
"sieťový"

MChangeDriveDisconnectedNetwork
"отключенный"
"disconnected"
upd:"disconnected"
upd:"disconnected"
"leválasztva"
upd:"disconnected"
"desconectado"
"odpojený"

MChangeDriveCDROM
"CD-ROM"
"CD-ROM"
"CD-ROM"
"CD-ROM"
"CD-ROM"
"CD-ROM"
"CD-ROM   "
"CD-ROM"

MChangeDriveCD_RW
"CD-RW"
"CD-RW"
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
"CD-RW/DVD"
"CD-RW/DVD"

MChangeDriveDVD_ROM
"DVD-ROM"
"DVD-ROM"
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
"DVD-RW"
"DWD-RW"

MChangeDriveDVD_RAM
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"
"DVD-RAM"

MChangeDriveBD_ROM
"BD-ROM"
"BD-ROM"
"BD-ROM"
"BD-ROM"
"BD-ROM"
"BD-ROM"
"BD-ROM"
"BD-ROM"

MChangeDriveBD_RW
"BD-RW"
"BD-RW"
"BD-RW"
"BD-RW"
"BD-RW"
"BD-RW"
"BD-RW"
"BD-RW"

MChangeDriveHDDVD_ROM
"HDDVD-ROM"
"HDDVD-ROM"
"HDDVD-ROM"
"HDDVD-ROM"
"HDDVD-ROM"
"HDDVD-ROM"
"HDDVD-ROM"
"HDDVD-ROM"

MChangeDriveHDDVD_RW
"HDDVD-RW"
"HDDVD-RW"
"HDDVD-RW"
"HDDVD-RW"
"HDDVD-RW"
"HDDVD-RW"
"HDDVD-RW"
"HDDVD-RW"


MChangeDriveRAM
"RAM диск"
"RAM disk"
"RAM disk"
"RAM-DISK"
"RAM lemez"
"RAM-dysk"
"Disco RAM"
"RAM disk"

MChangeDriveSUBST
"SUBST"
"subst"
"SUBST"
"Subst"
"virtuális"
"subst"
"subst    "
"SUBST"

MChangeDriveVirtual
"виртуальный"
"virtual"
upd:"virtual"
upd:"virtual"
upd:"virtual"
upd:"virtual"
"virtual"
"virtuálny"

MChangeDriveLabelAbsent
"недоступен"
"not available"
"není k dispozici"
"nicht vorh."
"nem elérhető"
"niedostępny"
"no disponible"
"nie je k dispozícii"

MChangeDriveCannotReadDisk
"Ошибка чтения диска в дисководе"
"Cannot read the disk in drive"
"Nelze přečíst disk v jednotce"
"Kann nicht gelesen werden datenträge in Laufwerk"
"Meghajtó lemeze nem olvasható"
"Nie mogę odczytać dysku w napędzie"
"No se puede leer el disco en unidad"
"Nemôžem prečítať disk v jednotke"

MChangeDriveCannotDisconnect
"Не удаётся отсоединиться от %1"
"Cannot disconnect from %1"
"Nelze se odpojit od %1"
"Verbindung zu %1 konnte nicht getrennt werden."
"Nem lehet leválni innen: %1"
"Nie mogę odłączyć się od %1"
"No se puede desconectar desde %1"
"Nemôžem sa odpojiť od %1"

MChangeDriveCannotDelSubst
"Не удаётся удалить виртуальный диск %1"
"Cannot delete a substituted disk %1"
"Nelze smazat substnutá jednotka %1"
"Substlaufwerk %1 konnte nicht gelöscht werden."
"%1 virtuális meghajtó nem törölhető"
"Nie można usunąć dysku SUBST %1"
"No se puede borrar una unidad sustituida %1"
"Nemôžem zmazať substnutú jednotku %1"

MChangeDriveOpenFiles
"Если вы не закроете открытые файлы, данные могут быть утеряны"
"If you do not close the open files, data may be lost"
"Pokud neuzavřete otevřené soubory, mohou být tato data ztracena"
"Wenn Sie offene Dateien nicht schließen könnten Daten verloren gehen"
"Ha a nyitott fájlokat nem zárja be, az adatok elveszhetnek"
"Jeśli nie zamkniesz otwartych plików, możesz utracić dane"
"Si no cierra los archivos abiertos, los datos se pueden perder."
"Ak nezatvoríte otvorené súbory, môžete stratiť dáta"

MChangeSUBSTDisconnectDriveTitle
l:
"Отключение виртуального устройства"
"Virtual device disconnection"
"Odpojování virtuálního zařízení"
"Virtuelles Gerät trennen"
"Virtuális meghajtó törlése"
"Odłączanie napędu wirtualnego"
"Desconexión de dispositivo virtual"
"Odpojenie virtuálneho zariadenia"

MChangeSUBSTDisconnectDriveQuestion
"Отключить SUBST-диск %1?"
"Disconnect SUBST-disk %1?"
"Odpojit SUBST-disk %1?"
"Substlaufwerk %1 trennen?"
"Törli %1 virtuális meghajtót?"
"Odłączyć dysk SUBST %1?"
"Desconectarse de disco sustituido %1?"
"Odpojiť SUBST-disk %1?"

MChangeVHDDisconnectDriveTitle
"Отсоединение виртуального диска"
"Virtual disk detaching"
upd:"Virtual disk detaching"
upd:"Virtual disk detaching"
upd:"Virtual disk detaching"
upd:"Virtual disk detaching"
"Desconexión de disco virtual"
"Odpojenie virtuálneho disku"

MChangeVHDDisconnectDriveQuestion
"Отсоединить виртуальный диск %1?"
"Detach virtual disk %1?"
upd:"Detach virtual disk %1?"
upd:"Detach virtual disk %1?"
upd:"Detach virtual disk %1?"
upd:"Detach virtual disk %1?"
"Desconectar disco virtual %1?"
"Odpojiť virtuálny disk %1?"

MChangeHotPlugDisconnectDriveTitle
l:
"Удаление устройства"
"Device Removal"
"Odpojování zařízení"
"Sicheres Entfernen"
"Eszköz biztonságos eltávolítása"
"Odłączanie urządzenia"
"Remover dispositivo"
"Odstránenie zariadenia"

MChangeHotPlugDisconnectDriveQuestion
"Вы хотите удалить устройство"
"Do you want to remove the device"
"Opravdu si přejete odpojit zařízení"
"Wollen Sie folgendes Gerät sicher entfernen? "
"Eltávolítja az eszközt?"
"Czy odłączyć urządzenie"
"Desea remover el dispositivo"
"Chcete odstrániť zariadenie"

MHotPlugDisks
"(диск(и): %1)"
"(disk(s): %1)"
"(disk(y): %1)"
"(Laufwerk(e): %1)"
"(%1 meghajtó)"
"(dysk(i): %1)"
"(disco(s): %1)"
"(disk(y): %1)"

MChangeCouldNotEjectHotPlugMedia
"Невозможно удалить устройство для диска %1:"
"Cannot remove a device for drive %1:"
"Zařízení %1: nemůže být odpojeno."
"Ein Gerät für Laufwerk %1: konnte nicht entfernt werden"
"%1: eszköz nem távolítható el"
"Nie udało się odłączyć dysku %1:"
"No se puede remover dispositivo para unidad %1:"
"Nemôžem odstrániť zariadenie pre jednotku %1:"

MChangeCouldNotEjectHotPlugMedia2
"Невозможно удалить устройство:"
"Cannot remove a device:"
"Zařízení nemůže být odpojeno."
"Kann folgendes Geräte nicht entfernen:"
"Az eszköz nem távolítható el:"
"Nie udało się odłączyć urządzenia:"
"No se puede remover el dispositivo:"
"Nemôžem odstrániť zariadenie."

MChangeHotPlugNotify1
"Теперь устройство"
"The device"
"Zařízení"
"Das Gerät"
"Az eszköz:"
"Urządzenie"
"El dispositivo"
"Zariadenie"

MChangeHotPlugNotify2
"может быть безопасно извлечено из компьютера"
"can now be safely removed"
"může být nyní bezpečně odebráno"
"kann nun vom Computer getrennt werden"
"már biztonságosan eltávolítható"
"można teraz bezpiecznie odłączyć"
"ahora puede ser removido de forma segura"
"môže byť teraz bezpečne odstránené"

MHotPlugListTitle
"Hotplug-устройства"
"Hotplug devices list"
"Seznam vyjímatelných zařízení"
"Hardware sicher entfernen"
"Hotplug eszközök"
"Lista urządzeń Hotplug"
"Lista de conexión de dispositivos"
"Zoznam vyberateľných zariadení"

MHotPlugListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Tasten: Entf,StrgR,F1"
"Szerkesztés: Del,Ctrl-R"
"Edycja: Del,Ctrl-R"
"Editar: Del,Ctrl-R"
"Edit: Del,Ctrl-R"

MChangeDriveDisconnectTitle
l:
"Отключение сетевого устройства"
"Disconnect network drive"
"Odpojit síťovou jednotku"
"Netzwerklaufwerk trennen"
"Hálózati meghajtó leválasztása"
"Odłączanie dysku sieciowego"
"Desconectar unidad de red"
"Odpojiť sieťový disk"

MChangeDriveDisconnectQuestion
"Вы хотите удалить соединение с устройством %1:?"
"Do you want to disconnect from the drive %1:?"
"Opravdu si přejete odpojit od jednotky %1:?"
"Wollen Sie die Verbindung zu Laufwerk %1: trennen?"
"Le akar válni %1: meghajtóról?"
"Czy odłączyć dysk %1:?"
"Quiere desconectarse desde la unidad %1:?"
"Chcete odpojiť od jednotky %1:?"

MChangeDriveDisconnectMapped
"На устройство %1: отображена папка:"
"The drive %1: is mapped to:"
"Jednotka %1: je namapována na:"
"Laufwerk %1: ist verknüpft zu:"
"%1: meghajtó hozzárendelve:"
"Dysk %1: jest skojarzony z:"
"La unidad %c: es mapeada hacia:"
"Jednotka %1: je namapovaná na:"

MChangeDriveDisconnectReconnect
"&Восстанавливать при входе в систему"
"&Reconnect at logon"
"&Znovu připojit při přihlášení"
"Bei Anmeldung &verbinden"
"&Bejelentkezésnél újracsatlakoztat"
"&Podłącz ponownie przy logowaniu"
"&Reconectar al desconectar"
"&Znova pripojiť pri prihlásení"

MChangeDriveAskDisconnect
l:
"Вы хотите в любом случае отключиться от устройства?"
"Do you want to disconnect the device anyway?"
"Přejete si přesto zařízení odpojit?"
"Wollen Sie die Verbindung trotzdem trennen?"
"Mindenképpen leválasztja az eszközt?"
"Czy chcesz mimo to odłączyć urządzenie?"
"Quiere desconectar el dispositivo de cualquier forma?"
"Aj tak chcete odpojiť zariadenie?"

MChangeWaitingLoadDisk
"Ожидание чтения диска..."
"Waiting for disk to mount..."
"Čekám na disk k připojení..."
"Warte auf Datenträger..."
"Lemez betöltése..."
"Trwa montowanie dysku..."
"Esperando para montar el disco..."
"Čakám na disk na pripojenie..."

MChangeCouldNotEjectMedia
"Невозможно извлечь диск из привода %1:"
"Could not eject media from drive %1:"
"Nelze vysunout médium v jednotce %1:"
"Konnte Medium in Laufwerk %1: nicht auswerfen"
"%1: meghajtó lemeze nem oldható ki"
"Nie można wysunąć dysku z napędu %1:"
"No se puede expulsar medio de la unidad %1:"
"Nemôžem vysunúť médium v jednotke %1:"

MChangeDriveConfigure
"Настройка меню выбора диска"
"Change Drive Menu Options"
upd:"Change Drive Menu Options"
upd:"Change Drive Menu Options"
upd:"Change Drive Menu Options"
upd:"Change Drive Menu Options"
"Cambiar opciones de menú de unidades"
"Možnosti menu zmeny disku"

MChangeDriveShowDiskType
"Показывать &тип диска"
"Show disk &type"
upd:"Show disk type"
upd:"Show disk type"
upd:"Show disk type"
upd:"Show disk type"
"Mostrar &tipo de disco"
"Zobraziť typ disku"

MChangeDriveShowNetworkName
"Показывать &сетевое имя/путь SUBST/имя VHD"
"Show &network name/SUBST path/VHD name"
upd:"Show &network name/SUBST path/VHD name"
upd:"Show &network name/SUBST path/VHD name"
upd:"Show &network name/SUBST path/VHD name"
upd:"Show &network name/SUBST path/VHD name"
"Mostrar nombre/SUBST ruta/VHD de nombre de &red"
"Zobraziť sieťový &názov/cesta SUBST/názov VHD"

MChangeDriveShowLabel
"Показывать &метку диска"
"Show disk &label"
upd:"Show disk &label"
upd:"Show disk &label"
upd:"Show disk &label"
upd:"Show disk &label"
"Mostrar etiqueta"
"Zobraziť názov disk&u"

MChangeDriveShowFileSystem
"Показывать тип &файловой системы"
"Show &file system type"
upd:"Show &file system type"
upd:"Show &file system type"
upd:"Show &file system type"
upd:"Show &file system type"
"Mostrar sistema de archivos"
"Zobraziť ty&p systému súborov"

MChangeDriveShowSize
"Показывать &размер"
"Show &size"
upd:"Show &size"
upd:"Show &size"
upd:"Show &size"
upd:"Show &size"
"Mostrar tamaño"
"Zobrazi&ť veľkosť"

MChangeDriveShowSizeFloat
"Показывать ра&змер в стиле Windows Explorer"
"Show size in &Windows Explorer style"
upd:"Show size in &Windows Explorer style"
upd:"Show size in &Windows Explorer style"
upd:"Show size in &Windows Explorer style"
upd:"Show size in &Windows Explorer style"
"Mostrar tamaño estilo &Windows Explorer"
"Zobraziť veľkosť v štýle Prieskumníka &Windows"

MChangeDriveShowRemovableDrive
"Показывать параметры см&енных дисков"
"Show &removable drive parameters"
upd:"Show &removable drive parameters"
upd:"Show &removable drive parameters"
upd:"Show &removable drive parameters"
upd:"Show &removable drive parameters"
"Mostrar parámetros de unidad removible"
"Zobraziť paramet&re vymeniteľnej jednotky"

MChangeDriveShowPlugins
"Показывать &плагины"
"Show &plugins"
upd:"Show &plugins"
upd:"Show &plugins"
upd:"Show &plugins"
upd:"Show &plugins"
"Mostrar com&plementos"
"Zobraziť mo&duly"

MChangeDriveSortPluginsByHotkey
"Сортировать плагины по горячей клавише"
"Sort plugins by hotkey"
upd:"Sort plugins by hotkey"
upd:"Sort plugins by hotkey"
upd:"Sort plugins by hotkey"
upd:"Sort plugins by hotkey"
"Ordenar complementos por tecla de atajo"
"Triediť moduly podľa skratky"

MChangeDriveShowCD
"Показывать параметры &компакт-дисков"
"Show &CD drive parameters"
upd:"Show &CD drive parameters"
upd:"Show &CD drive parameters"
upd:"Show &CD drive parameters"
upd:"Show &CD drive parameters"
"Mostrar parámetros unidad de &CD"
"Zobraziť parametre jwednotky &CD"

MChangeDriveShowNetworkDrive
"Показывать параметры се&тевых дисков"
"Show n&etwork drive parameters"
upd:"Show ne&twork drive parameters"
upd:"Show ne&twork drive parameters"
upd:"Show ne&twork drive parameters"
upd:"Show ne&twork drive parameters"
"Mostrar parámetros unidades de red"
"Zobraziť parametre sie&ťovej jednotky"

MChangeDriveMenuFooter
"Del,Shift-Del,F3,F4,F9"
"Del,Shift-Del,F3,F4,F9"
"Del,Shift-Del,F3,F4,F9"
"Del,Shift-Del,F3,F4,F9"
"Del,Shift-Del,F3,F4,F9"
"Del,Shift-Del,F3,F4,F9"
"Del,Shift-Del,F3,F4,F9"
"Del,Shift-Del,F3,F4,F9"

MSearchFileTitle
l:
" Поиск "
" Search "
" Hledat "
" Suchen "
" Keresés "
" Szukaj "
" Buscar "
" Hľadať "

MCannotCreateListFile
"Ошибка создания списка файлов"
"Cannot create list file"
"Nelze vytvořit soubor se seznamem"
"Dateiliste konnte nicht erstellt werden"
"A listafájl nem hozható létre"
"Nie mogę utworzyć listy plików"
"No se puede crear archivo de lista"
"Nemôžem vytvoriť súbor so zoznamom"

MCannotCreateListTemp
"(невозможно создать временный файл для списка)"
"(cannot create temporary file for list)"
"(nemohu vytvořit dočasný soubor pro seznam)"
"(Fehler beim Anlegen einer temporären Datei für Liste)"
"(a lista átmeneti fájl nem hozható létre)"
"(nie można utworzyć pliku tymczasowego dla listy)"
"(no se puede crear archivo temporal para lista)"
"(nemôžem vytvoriť dočasný súbor pre zoznam)"

MCannotCreateListWrite
"(невозможно записать данные в файл)"
"(cannot write data in file)"
"(nemohu zapsat data do souboru)"
"(Fehler beim Schreiben der Daten)"
"(a fájlba nem írható adat)"
"(nie można zapisać danych do pliku)"
"(no se puede escribir datos en el archivo)"
"(nemôžem zapísať dáta do súboru)"

MDragFiles
l:
"%1 файлов"
"%1 files"
"%1 souborů"
"%1 Dateien"
"%1 fájl"
"%1 plików"
"%1 archivos"
"%1 úborov"

MDragMove
"Перенос %1"
"Move %1"
"Přesunout %1"
"Verschiebe %1"
"%1 mozgatása"
"Przenieś %1"
"Mover %1"
"Presunúť %1"

MDragCopy
"Копирование %1"
"Copy %1"
"Kopírovat %1"
"Kopiere %1"
"%1 másolása"
"Kopiuj %1"
"Copiar %1"
"Kopírovať %1"

MProcessListTitle
l:
"Список задач"
"Task list"
"Seznam úloh"
"Taskliste"
"Futó programok"
"Lista zadań"
"Lista de tareas"
"Zoznam úloh"

MProcessListBottom
"Редактирование: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Edit: Del,Ctrl-R"
"Tasten: Entf,StrgR"
"Szerk.: Del,Ctrl-R"
"Edycja: Del,Ctrl-R"
"Editar: Del,Ctrl-R"
"Upraviť: Del,Ctrl-R"

MKillProcessTitle
"Удаление задачи"
"Kill task"
"Zabít úlohu"
"Task beenden"
"Programkilövés"
"Zakończ zadanie"
"Terminar tarea"
"Zrušiť úlohu"

MAskKillProcess
"Вы хотите удалить выбранную задачу?"
"Do you wish to kill selected task?"
"Přejete si zabít vybranou úlohu?"
"Wollen Sie den ausgewählten Task beenden?"
"Ki akarja lőni a kijelölt programot?"
"Czy chcesz zakończyć wybrane zadanie?"
"Desea terminar la tarea seleccionada?"
"Chcete zrušiť zvolenú úlohu?"

MKillProcessWarning
"Вы потеряете всю несохраненную информацию этой программы"
"You will lose any unsaved information in this program"
"V tomto programu budou ztraceny neuložené informace"
"Alle ungespeicherten Daten dieses Programmes gehen verloren"
"A program minden mentetlen adata elvész"
"Utracisz wszystkie niezapisane dane w tym programie"
"Usted perder cualquier información no grabada de este programa"
"V tomto programe sa stratia neuložené informácie"

MKillProcessKill
"Удалить"
"Kill"
"Zabít"
"Beenden"
"Kilő"
"Zakończ"
"Terminar"
"Zrušiť"

MCannotKillProcess
"Указанную задачу удалить не удалось"
"Cannot kill the specified task"
"Nemohu ukončit zvolenou úlohu"
"Task konnte nicht beendet werden"
"A programot nem lehet kilőni"
"Nie mogę zakończyć wybranego zadania"
"No se puede terminar la tarea seleccionada"
"Nemôžem ukončiť zvolenú úlohu"

MCannotKillProcessPerm
"Вы не имеет права удалить этот процесс."
"You have no permission to kill this process."
"Nemáte oprávnění zabít tento proces."
"Sie haben keine Rechte um diesen Prozess zu beenden."
"Nincs joga a program kilövésére"
"Nie masz wystarczających uprawnień do zakończenia procesu."
"Usted no tiene permiso para terminar este proceso."
"Nemáte oprávnenie zrušiť tento proces."

MQuickViewTitle
l:
"Быстрый просмотр"
"Quick view"
"Zběžné zobrazení"
"Schnellansicht"
"Gyorsnézet"
"Szybki podgląd"
"Vista rápida"
"Rýchle info"

MQuickViewFolder
"Папка"
"Folder"
"Adresář"
"Verzeichnis"
"Mappa"
"Folder"
"Directorio"
"Priečinok"

MQuickViewJunction
"Связь"
"Junction"
"Křížení"
"Knotenpunkt"
"Csomópont"
"Powiązanie"
"Unión"
"Kríženie"

MQuickViewSymlink
"Ссылка"
"Symlink"
"Symbolický link"
"Symlink"
"Szimlink"
"Link"
"EnlcSimb"
"Symbolické prepojenie"

MQuickViewVolMount
"Том"
"Volume"
"Svazek"
"Datenträger"
"Kötet"
"Napęd"
"Volumen"
"Zväzok"

MQuickViewDFS
upd:"DFS"
"DFS"
upd:"DFS"
upd:"DFS"
upd:"DFS"
upd:"DFS"
upd:"DFS"
upd:"DFS"

MQuickViewDFSR
upd:"DFSR"
"DFSR"
upd:"DFSR"
upd:"DFSR"
upd:"DFSR"
upd:"DFSR"
upd:"DFSR"
upd:"DFSR"

MQuickViewHSM
upd:"HSM"
"HSM"
upd:"HSM"
upd:"HSM"
upd:"HSM"
upd:"HSM"
upd:"HSM"
upd:"HSM"

MQuickViewHSM2
upd:"HSM2"
"HSM2"
upd:"HSM2"
upd:"HSM2"
upd:"HSM2"
upd:"HSM2"
upd:"HSM2"
upd:"HSM2"

MQuickViewSIS
upd:"SIS"
"SIS"
upd:"SIS"
upd:"SIS"
upd:"SIS"
upd:"SIS"
upd:"SIS"
upd:"SIS"

MQuickViewWIM
upd:"WIM"
"WIM"
upd:"WIM"
upd:"WIM"
upd:"WIM"
upd:"WIM"
upd:"WIM"
upd:"WIM"

MQuickViewCSV
upd:"CSV"
"CSV"
upd:"CSV"
upd:"CSV"
upd:"CSV"
upd:"CSV"
upd:"CSV"
upd:"CSV"

MQuickViewUnknownReparsePoint
upd:"Unknown reparse point"
"Unknown reparse point"
upd:"Unknown reparse point"
upd:"Unknown reparse point"
upd:"Unknown reparse point"
upd:"Unknown reparse point"
upd:"Unknown reparse point"
upd:"Unknown reparse point"

MQuickViewNoData
"(нет данных)"
"(data not available)"
"(data nejsou k dispozici)"
"(nicht verfügbar)"
"(adat nem elérhető)"
"(dane niedostępne)"
"(dato no disponible)"
"(dáta nie sú k dispozícii)"

MQuickViewContains
"Содержит:"
"Contains:"
"Obsah:"
"Enthält:"
"Tartalma:"
"Zawiera:"
"Contiene:"
"Obsahuje:"

MQuickViewFolders
"Папок               "
"Folders          "
"Adresáře           "
"Ordner           "
"Mappák száma     "
"Katalogi            "
"Directorios      "
"Priečinky         "

MQuickViewFiles
"Файлов              "
"Files            "
"Soubory            "
"Dateien          "
"Fájlok száma     "
"Pliki               "
"Archivos         "
"Súbory            "

MQuickViewBytes
"Размер файлов       "
"Files size       "
"Velikost souborů   "
"Gesamtgröße      "
"Fájlok mérete    "
"Rozmiar plików      "
"Tamaño archivos  "
"Veľkosť súborov   "

MQuickViewAllocated
"Выделенный размер   "
"Allocated size   "
upd:"Allocated size   "
upd:"Allocated size   "
upd:"Allocated size   "
upd:"Allocated size   "
"Tamaño alojado   "
"Alokovaná veľkosť "

MQuickViewCluster
"Размер кластера     "
"Cluster size     "
"Velikost clusteru  "
"Clustergröße     "
"Klaszterméret    "
"Rozmiar klastra     "
"Tamaño cluster   "
"Veľkosť klastra   "

MQuickViewSlack
"Остатки кластеров   "
"Files slack      "
"Mrtvé místo        "
"Verlust          "
"Meddő terület    "
"Przestrzeń stracona "
"Desperdiciado    "
"Mŕtve miesto      "

MQuickViewMFTOverhead
upd:"MFT overhead        "
"MFT overhead     "
upd:"MFT overhead        "
upd:"MFT overhead        "
upd:"MFT overhead        "
upd:"MFT overhead        "
"Gasto adicional de MFT"
"MFT overhead      "

MSetAttrTitle
l:
"Атрибуты"
"Attributes"
"Atributy"
"Attribute"
"Attribútumok"
"Atrybuty"
"Atributos"
"Atribúty"

MSetAttrFor
"Изменить файловые атрибуты"
"Change file attributes for"
"Změna atributů souboru pro"
"Ändere Dateiattribute für"
"Attribútumok megváltoztatása"
"Zmień atrybuty dla"
"Cambiar atributos del archivo"
"Zmena atributov súboru pre"

MSetAttrSelectedObjects
"выбранных объектов"
"selected objects"
"vybrané objekty"
"markierte Objekte"
"a kijelölt objektumokon"
"wybranych obiektów"
"objetos seleccionados"
"vybrané objekty"

MSetAttrHardLinks
"жёстких ссылок"
"hard links"
"pevné linky"
"Hardlinks"
"hardlink"
"linków trwałych"
"Enlace rígido"
"pevné prepojenia"

MSetAttrJunction
"Связь:"
"Junction:"
"Křížení:"
"Knotenpunkte:"
"Сsomópont:"
"Powiązanie:"
"Empalmar:"
"Kríženie:"

MSetAttrSymlink
"Ссылка:"
"Symlink:"
"Link:"
"Symlink:"
"Szimlink:"
"Link:"
"Enlace:"
"Prepojenie:"

MSetAttrVolMount
"Том:"
"Volume:"
"Svazek:"
"Datenträger:"
"Kötet:"
"Punkt zamontowania:"
"Volumen:"
"Zväzok:"

MSetAttrUnknownJunction
"(нет данных)"
"(data not available)"
"(data nejsou k dispozici)"
"(nicht verfügbar)"
"(adat nem elérhető)"
"(dane niedostępne)"
"(dato no disponible)"
"(dáta nie sú k dispozícii)"

MSetAttrRO
"&Только для чтения"
"&Read only"
"&Pouze pro čtení"
"Sch&reibschutz"
"&Csak olvasható"
"&Tylko do odczytu"
"Sólo &lectura"
"&Len na čítanie"

MSetAttrArchive
"&Архивный"
"&Archive"
"&Archivovat"
"&Archiv"
"&Archív"
"&Archiwalny"
"&Archivo"
"&Archivovať"

MSetAttrHidden
"&Скрытый"
"&Hidden"
"&Skrytý"
"&Versteckt"
"&Rejtett"
"&Ukryty"
"&Oculto"
"&Skrytý"

MSetAttrSystem
"С&истемный"
"&System"
"S&ystémový"
"&System"
"Ren&dszer"
"&Systemowy"
"&Sistema"
"S&ystémový"

MSetAttrCompressed
"Сжаты&й"
"&Compressed"
"&Komprimovaný"
"&Komprimiert"
"&Tömörített"
"S&kompresowany"
"&Comprimido"
"&Komprimovaný"

MSetAttrEncrypted
"За&шифрованный"
"&Encrypted"
"&Šifrovaný"
"V&erschlüsselt"
"Tit&kosított"
"Zaszy&frowany"
"Ci&frado"
"&Šifrovaný"

MSetAttrNotIndexed
"Н&еиндексируемый"
"Not &Indexed"
"Neinde&xovaný"
"Nicht &indiziert"
"Nem inde&xelt"
"Nie z&indeksowany"
"No &Indexar"
"Neinde&xovaný"

MSetAttrSparse
"Разре&женный"
"S&parse"
upd:"Rozptýlený"
upd:"Reserve"
"Ritk&ított"
upd:"Sparse"
"Dis&perso"
"Rozptýlený"

MSetAttrTemp
"Временный"
"Temporary"
"Dočasný"
"Temporär"
"&Átmeneti"
"&Tymczasowy"
"Temporal"
"Dočasný"

MSetAttrOffline
"Автономный"
"Offline"
"Offline"
"Offline"
"O&ffline"
"Offline"
"Desconectado"
"Offline"

MSetAttrReparsePoint
"Точка повторной обработки"
"Reparse point"
upd:"Reparse point"
upd:"Reparse point"
upd:"Reparse point"
upd:"Reparse point"
"Reanalizar punto"
"Reparse point"

MSetAttrVirtual
"Виртуальный"
"Virtual"
"Virtuální"
"Virtuell"
"&Virtuális"
"Wirtualny"
"Virtual"
"Virtuálny"

MSetAttrSubfolders
"Обрабатывать &вложенные папки"
"Process sub&folders"
"Zpracovat i po&dadresáře"
"Unterordner miteinbe&ziehen"
"Az almappákon is"
"Przetwarzaj &podkatalogi"
"Procesar sub&directorios"
"Spracovať aj po&dpriečinky"

MSetAttrOwner
"Владелец:"
"Owner:"
"Vlastník:"
"Besitzer:"
"Tulajdonos:"
"Właściciel:"
"Dueño:"
"Vlastník:"

MSetAttrOwnerMultiple
"(несколько значений)"
"(multiple values)"
upd:"(multiple values)"
upd:"(multiple values)"
upd:"(multiple values)"
upd:"(multiple values)"
"(valores múltiples)"
"(viacero hodnôt)"

MSetAttrModification
"Время последней &записи:"
"Last &write time:"
upd:"Last &write time:"
upd:"Last &write time:"
upd:"Last &write time:"
upd:"Last &write time:"
"&Ultima hora escritura:"
"Čas posle&dného zápisu:"

MSetAttrCreation
"Время со&здания:"
"Crea&tion time:"
"Čas v&ytvoření:"
"Datei erstell&t:"
"&Létrehozás dátuma/ideje:"
"Czas u&tworzenia:"
"Hora de cr&eación:"
"Čas v&ytvorenia:"

MSetAttrLastAccess
"Время последнего &доступа:"
"&Last access time:"
"Čas posledního pří&stupu:"
"&Letzter Zugriff:"
"&Utolsó hozzáférés dátuma/ideje:"
"Czas ostatniego &dostępu:"
"Hora de últi&mo acceso:"
"Čas posledného prí&stupu:"

MSetAttrChange
"Время из&менения:"
"Chan&ge time:"
upd:"Change time:"
upd:"Change time:"
upd:"Change time:"
upd:"Change time:"
"Cambiar &hora:"
"Čas zmeny:"

MSetAttrOriginal
"Исход&ное"
"&Original"
"&Originál"
"&Original"
"&Eredeti"
"Wstaw &oryginalny"
"Ori&ginal"
"&Originálny"

MSetAttrCurrent
"Те&кущее"
"Curre&nt"
"So&učasný"
"Akt&uell"
"Aktuál&is"
"Wstaw &bieżący"
"Ac&tual"
"Akt&uálny"

MSetAttrBlank
"Сбр&ос"
"&Blank"
"P&rázdný"
"L&eer"
"&Üres"
"&Wyczyść"
"&Vaciar"
"P&rázdny"

MSetAttrSet
"Установить"
"Set"
"Nastavit"
"Setzen"
"Alkalmaz"
"Usta&w"
"Poner"
"Nastaviť"

MSetAttrTimeTitle1
l:
"ММ%1ДД%2ГГГГГ чч%3мм%4сс%5мс"
"MM%1DD%2YYYYY hh%3mm%4ss%5ms"
upd:"MM%1DD%2RRRRR hh%3mm%4ss%5ms"
upd:"MM%1TT%2JJJJJ hh%3mm%4ss%5ms"
upd:"HH%1NN%2ÉÉÉÉÉ óó%3pp%4mm%5ms"
upd:"MM%1DD%2RRRRR gg%3mm%4ss%5ms"
"MM%1DD%2AAAAA hh%3mm%4ss%5ms"
"MM%1DD%2RRRRR hh%3mm%4ss%5ms"

MSetAttrTimeTitle2
"ДД%1ММ%2ГГГГГ чч%3мм%4сс%5мс"
"DD%1MM%2YYYYY hh%3mm%4ss%5ms"
upd:"DD%1MM%2RRRRR hh%3mm%4ss%5ms"
upd:"TT%1MM%2JJJJJ hh%3mm%4ss%5ms"
upd:"NN%1HH%2ÉÉÉÉÉ óó%3pp%4mm%5ms"
upd:"DD%1MM%2RRRRR gg%3mm%4ss%5ms"
"DD%1MM%2AAAAA hh%3mm%4ss%5ms"
"DD%1MM%2RRRRR hh%3mm%4ss%5ms"

MSetAttrTimeTitle3
"ГГГГГ%1ММ%2ДД чч%3мм%4сс%5мс"
"YYYYY%1MM%2DD hh%3mm%4ss%5ms"
upd:"RRRRR%1MM%2DD hh%3mm%4ss%5ms"
upd:"JJJJJ%1MM%2TT hh%3mm%4ss%5ms"
upd:"ÉÉÉÉÉ%1HH%2NN óó%3pp%4mm%5ms"
upd:"RRRRR%1MM%2DD gg%3mm%4ss%5ms"
"AAAAA%1MM%2DD hh%3mm%4ss%5ms"
"RRRRR%1MM%2DD hh%3mm%4ss%5ms"

MSetAttrSystemDialog
"Системные свойства"
"System properties"
upd:"System properties"
upd:"System properties"
upd:"System properties"
upd:"System properties"
"Propiedades del sistema"
"Vlastnosti systému"

MSetAttrSetting
l:
"Установка файловых атрибутов для"
"Setting file attributes for"
"Nastavení atributů souboru pro"
"Setze Dateiattribute für"
"Attribútumok beállítása"
"Ustawiam atrybuty"
"Poniendo atributos de archivo para"
"Nastavenie atribútov súboru pro"

MSetAttrCannotFor
"Ошибка установки атрибутов для"
"Cannot set attributes for"
"Nelze nastavit atributy pro"
"Konnte Dateiattribute nicht setzen für"
"Az attribútumok nem állíthatók be:"
"Nie mogę ustawić atrybutów dla"
"No se pueden poner atributos para"
"Nemôžem nastaviť atribúty pre"

MSetAttrCompressedCannotFor
"Не удалось установить атрибут СЖАТЫЙ для"
"Cannot set attribute COMPRESSED for"
"Nelze nastavit atribut KOMPRIMOVANÝ pro"
"Konnte Komprimierung nicht setzen für"
"A TÖMÖRÍTETT attribútum nem állítható be:"
"Nie mogę ustawić atrybutu SKOMPRESOWANY dla"
"No se puede poner atributo COMPRIMIDO a"
"Nemôžem nastaviť atribút KOMPRIMOVANÝ pre"

MSetAttrEncryptedCannotFor
"Не удалось установить атрибут ЗАШИФРОВАННЫЙ для"
"Cannot set attribute ENCRYPTED for"
"Nelze nastavit atribut ŠIFROVANÝ pro"
"Konnte Verschlüsselung nicht setzen für"
"A TITKOSÍTOTT attribútum nem állítható be:"
"Nie mogę ustawić atrybutu ZASZYFROWANY dla"
"No se puede poner atributo CIFRADO a"
"Nemôžem nastaviť atribút ŠIFROVANÝ pre"

MSetAttrSparseCannotFor
"Не удалось установить атрибут РАЗРЕЖЁННЫЙ для"
"Cannot set attribute SPARSE for"
upd:"Cannot set attribute SPARSE for"
upd:"Cannot set attribute SPARSE for"
"A RITKÍTOTT attribútum nem állítható be:"
upd:"Cannot set attribute SPARSE for"
"No se puede poner atributo DISPERSO para"
"Nemôžem nastaviť atribút SPARSE pre"

MSetAttrTimeCannotFor
"Не удалось установить время файла для"
"Cannot set file time for"
"Nelze nastavit čas souboru pro"
"Konnte Dateidatum nicht setzen für"
"A dátum nem állítható be:"
"Nie mogę ustawić czasu pliku dla"
"No se puede poner hora de archivo para"
"Nemôžem nastaviť čas súboru pre"

MSetAttrOwnerCannotFor
"Не удалось установить владельца для"
"Cannot set owner for"
upd:"Cannot set owner for"
upd:"Cannot set owner for"
upd:"Cannot set owner for"
upd:"Cannot set owner for"
"No se puede poner como dueño para"
"Nemôžem nastaviť vlastníka pre"


MSetColorPanel
l:
"&Панель"
"&Panel"
"&Panel"
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
"Pár&beszédablak"
"Okno &dialogowe"
"&Diálogo"
"&Dialóg"

MSetColorWarning
"Пр&едупреждение"
"&Warning message"
"&Varovná zpráva"
"&Warnmeldung"
"&Figyelmeztetés"
"&Ostrzeżenie"
"Mensa&je de advertencia"
"&Varovné hlásenie"

MSetColorMenu
"&Меню"
"&Menu"
"&Menu"
"&Menü"
"&Menü"
"&Menu"
"&Menú"
"&Menu"

MSetColorHMenu
"&Горизонтальное меню"
"Hori&zontal menu"
"Hori&zontální menu"
"Hori&zontales Menü"
"&Vízszintes menü"
"Pa&sek menu"
"Menú &horizontal"
"Hori&zontálne menu"

MSetColorKeyBar
"&Линейка клавиш"
"&Key bar"
"&Řádek kláves"
upd:"&Key bar"
"F&unkcióbill.sor"
"Pasek &klawiszy"
"&Barra de menú"
"&Lišta klávesov"

MSetColorCommandLine
"&Командная строка"
"&Command line"
"Pří&kazový řádek"
"&Kommandozeile"
"P&arancssor"
"&Linia poleceń"
"&Línea de comando"
"Prí&kazový riadok"

MSetColorClock
"&Часы"
"C&lock"
"&Hodiny"
"U&hr"
"Ó&ra"
"&Zegar"
"&Reloj"
"&Hodiny"

MSetColorViewer
"Про&смотрщик"
"&Viewer"
"P&rohlížeč"
"&Betrachter"
"&Nézőke"
"Pod&gląd"
"&Visor"
"Zob&razovač"

MSetColorEditor
"&Редактор"
"&Editor"
"&Editor"
"&Editor"
"&Szerkesztő"
"&Edytor"
"&Editor"
"&Editor"

MSetColorHelp
"П&омощь"
"&Help"
"&Nápověda"
"&Hilfe"
"Sú&gó"
"P&omoc"
"&Ayuda"
"Pomoc&ník"

MSetDefaultColors
"&Установить стандартные цвета"
"Set de&fault colors"
"N&astavit výchozí barvy"
"Setze Standard&farben"
"Alapért. s&zínek"
"&Ustaw kolory domyślne"
"Poner colores prede&terminados"
"N&astaviť východiskové farby"

MSetBW
"Чёрно-бел&ый режим"
"&Black and white mode"
"Černo&bílý mód"
"Schwarz && &Weiß"
"Fekete-fe&hér mód"
"&Tryb czarno-biały"
"Modo blanco y &negro"
"Čierno&biely režim"

MSetColorPanelNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"
"Normalny tekst"
"Texto normal"
"Normálny text"

MSetColorPanelSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"
"Wybrany tekst"
"Texto seleccionado"
"Vybraný text"

MSetColorPanelHighlightedInfo
"Выделенная информация"
"Highlighted info"
"Info zvýrazněné"
"Markierung"
"Kiemelt info"
"Podświetlone info"
"Info resaltados"
"Zvýraznené info"

MSetColorPanelDragging
"Перетаскиваемый текст"
"Dragging text"
"Tažený text"
"Drag && Drop Text"
"Vonszolt szöveg"
"Przeciągany tekst"
"Texto arrastrado"
"Potiahnutý text"

MSetColorPanelBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"
"Ramka"
"Borde"
"Okraj"

MSetColorPanelNormalCursor
"Обычный курсор"
"Normal cursor"
"Normální kurzor"
"Normale Auswahl"
"Normál kurzor"
"Normalny kursor"
"Cursor normal"
"Normálny kurzor"

MSetColorPanelSelectedCursor
"Выделенный курсор"
"Selected cursor"
"Vybraný kurzor"
"Markierte Auswahl"
"Kijelölt kurzor"
"Wybrany kursor"
"Cursor seleccionado"
"Vybraný kurzor"

MSetColorPanelNormalTitle
"Обычный заголовок"
"Normal title"
"Normální nadpis"
"Normaler Titel"
"Normál név"
"Normalny tytuł"
"Título normal"
"Normálny názov"

MSetColorPanelSelectedTitle
"Выделенный заголовок"
"Selected title"
"Vybraný nadpis"
"Markierter Titel"
"Kijelölt név"
"Wybrany tytuł"
"Título seleccionado"
"Vybraný názov"

MSetColorPanelColumnTitle
"Заголовок колонки"
"Column title"
"Nadpis sloupce"
"Spaltentitel"
"Oszlopnév"
"Tytuł kolumny"
"Título de columna"
"Názov stĺpca"

MSetColorPanelTotalInfo
"Количество файлов"
"Total info"
"Info celkové"
"Gesamtinfo"
"Összes info"
"Całkowite info"
"Info total"
"Celkové info"

MSetColorPanelSelectedInfo
"Количество выбранных файлов"
"Selected info"
"Info výběr"
"Markierungsinfo"
"Kijelölt info"
"Wybrane info"
"Info seleccionados"
"vybrané info"

MSetColorPanelScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"
"Suwak"
"Barra desplazamiento"
"Posuvník"

MSetColorPanelScreensNumber
"Количество фоновых экранов"
"Number of background screens"
"Počet obrazovek na pozadí"
"Anzahl an Hintergrundseiten"
"Háttérképernyők száma"
"Ilość ekranów w tle "
"Número de pantallas de fondo"
"Počet okien na pozadí"

MSetColorDialogNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"
"Tekst zwykły"
"Texto normal"
"Normálny text"

MSetColorDialogHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierter Text"
"Kiemelt szöveg"
"Tekst podświetlony"
"Texto resaltado"
"Zvýraznený text"

MSetColorDialogDisabled
"Блокированный текст"
"Disabled text"
"Zakázaný text"
"Deaktivierter Text"
"Inaktív szöveg"
"Tekst nieaktywny"
"Texto desactivado"
"Zakázaný text"

MSetColorDialogBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"
"Ramka"
"Borde"
"Okraj"

MSetColorDialogBoxTitle
"Заголовок рамки"
"Title"
"Nadpis"
"Titel"
"Keret neve"
"Tytuł"
"Título"
"Názov"

MSetColorDialogHighlightedBoxTitle
"Выделенный заголовок рамки"
"Highlighted title"
"Zvýrazněný nadpis"
"Markierter Titel"
"Kiemelt keretnév"
"Podświetlony tytuł"
"Título resaltado"
"Zvýraznený názov"

MSetColorDialogTextInput
"Ввод текста"
"Text input"
"Textový vstup"
"Texteingabe"
"Beírt szöveg"
"Wpisywany tekst"
"Entrada de texto"
"Textový vstup"

MSetColorDialogUnchangedTextInput
"Неизмененный текст"
"Unchanged text input"
"Nezměněný textový vstup"
"Unveränderte Texteingabe"
"Változatlan beírt szöveg"
"Niezmieniony wpisywany tekst "
"Entrada de texto sin cambiar"
"Nezmenený textový vstup"

MSetColorDialogSelectedTextInput
"Ввод выделенного текста"
"Selected text input"
"Vybraný textový vstup"
"Markierte Texteingabe"
"Beírt szöveg kijelölve"
"Zaznaczony wpisywany tekst"
"Entrada de texto seleccionada"
"Označený textový vstup"

MSetColorDialogEditDisabled
"Блокированное поле ввода"
"Disabled input line"
"Zakázaný vstupní řádek"
"Deaktivierte Eingabezeile"
"Inaktív beviteli sor"
"Nieaktywna linia wprowadzania danych"
"Línea de entrada desactivada"
"Zakázaný vstupný riadok"

MSetColorDialogButtons
"Кнопки"
"Buttons"
"Tlačítka"
"Schaltflächen"
"Gombok"
"Przyciski"
"Botones"
"Tlačidlá"

MSetColorDialogSelectedButtons
"Выбранные кнопки"
"Selected buttons"
"Vybraná tlačítka"
"Aktive Schaltflächen"
"Kijelölt gombok"
"Wybrane przyciski"
"Botones seleccionados"
"Vybrané tlačidlá"

MSetColorDialogHighlightedButtons
"Выделенные кнопки"
"Highlighted buttons"
"Zvýrazněná tlačítka"
"Markierte Schaltflächen"
"Kiemelt gombok"
"Podświetlone przyciski"
"Botones resaltados"
"Zvýraznené tlačidlá"

MSetColorDialogSelectedHighlightedButtons
"Выбранные выделенные кнопки"
"Selected highlighted buttons"
"Vybraná zvýrazněná tlačítka"
"Aktive markierte Schaltflächen"
"Kijelölt kiemelt gombok"
"Wybrane podświetlone przyciski "
"Botones resaltados seleccionados"
"Vybrané zvýraznené tlačidlá"

MSetColorDialogDefaultButton
"Кнопка по умолчанию"
"Default button"
upd:"Default button"
upd:"Default button"
upd:"Default button"
upd:"Default button"
"Botón por defecto"
"Predvolené tlačidlo"

MSetColorDialogSelectedDefaultButton
"Выбранная кнопка по умолчанию"
"Selected default button"
upd:"Selected default button"
upd:"Selected default button"
upd:"Selected default button"
upd:"Selected default button"
"Botón por defecto seleccionado"
"Vybrané predvolené tlačidlo"

MSetColorDialogHighlightedDefaultButton
"Выделенная кнопка по умолчанию"
"Highlighted default button"
upd:"Highlighted default button"
upd:"Highlighted default button"
upd:"Highlighted default button"
upd:"Highlighted default button"
"Botón por defecto resaltado"
"Zvýraznené predvolené tlačidlo"

MSetColorDialogSelectedHighlightedDefaultButton
"Выбранная выделенная кнопка по умолчанию"
"Selected highlighted default button"
upd:"Selected highlighted default button"
upd:"Selected highlighted default button"
upd:"Selected highlighted default button"
upd:"Selected highlighted default button"
"Botón por defecto resaltado seleccionado"
"Vybrané zvýraznené predvolené tlačidlo"

MSetColorDialogListBoxControl
"Список"
"List box"
"Seznam položek"
"Listenfelder"
"Listaablak"
"Lista"
"Cuadro de lista"
"Zoznam položiek"

MSetColorDialogComboBoxControl
"Комбинированный список"
"Combobox"
"Výběr položek"
"Kombinatiosfelder"
"Lenyíló szövegablak"
"Pole combo"
"Cuadro combo"
"Výber položiek"

MSetColorDialogListText
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"
"Tekst zwykły"
"Texto normal"
"Normály text"

MSetColorDialogListSelectedText
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"
"Tekst wybrany"
"Texto seleccionado"
"Vybraný text"

MSetColorDialogListHighLight
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"
"Kiemelt szöveg"
"Tekst podświetlony"
"Texto resaltado"
"Zvýraznený text"

MSetColorDialogListSelectedHighLight
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"
"Aktive Markierung"
"Kijelölt kiemelt szöveg"
"Tekst wybrany i podświetlony"
"Texto resaltado seleccionado"
"Vybraný zvýraznený text"

MSetColorDialogListDisabled
"Блокированный пункт"
"Disabled item"
"Naktivní položka"
"Deaktiviertes Element"
"Inaktív elem"
"Pole nieaktywne"
"Item desactivado"
"Neaktívna položka"

MSetColorDialogListBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"
"Ramka"
"Borde"
"Okraj"

MSetColorDialogListTitle
"Заголовок"
"Title"
"Nadpis"
"Titel"
"Keret neve"
"Tytuł"
"Título"
"Názov"

MSetColorDialogListGrayed
"Серый текст списка"
"Grayed list text"
upd:"Grayed list text"
upd:"Grayed list text"
"Szürke listaszöveg"
upd:"Grayed list text"
"Texto de listado en gris"
"Grayed list text"

MSetColorDialogSelectedListGrayed
"Выбранный серый текст списка"
"Selected grayed list text"
upd:"Selected grayed list text"
upd:"Selected grayed list text"
"Kijelölt szürke listaszöveg"
upd:"Selected grayed list text"
"Texto de listado en gris seleccionado"
"Selected grayed list text"

MSetColorDialogListScrollBar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"
"Suwak"
"Barra desplazamiento"
"Posuvník"

MSetColorDialogListArrows
"Индикаторы длинных строк"
"Long string indicators"
"Značka dlouhého řetězce"
"Indikator für lange Zeichenketten"
"Hosszú sztring jelzők"
"Znacznik długiego napisu"
"Indicadores de cadena larga"
"Značka dlhého reťazca"

MSetColorDialogListArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"
"Vybraná značka dlouhého řetězce"
"Aktiver Indikator"
"Kijelölt hosszú sztring jelzők"
"Zaznaczone znacznik długiego napisu"
"Indicadores de cadena larga seleccionados"
"Vybraná značka dlhého reťazca"

MSetColorDialogListArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"
"Zakázaná značka dlouhého řetězce"
"Deaktivierter Indikator"
"Inaktív hosszú sztring jelzők"
"Nieaktywny znacznik długiego napisu"
"Indicadores de cadena larga desactivados"
"Zakázaná značka dlhého reťazca"

MSetColorMenuNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"
"Normalny tekst"
"Texto normal"
"Normálny text"

MSetColorMenuSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"
"Wybrany tekst"
"Texto seleccionado"
"Vybraný text"

MSetColorMenuHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"
"Kiemelt szöveg"
"Podświetlony tekst"
"Texto resaltado"
"Zvýraznený text"

MSetColorMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"
"Aktive Markierung"
"Kijelölt kiemelt szöveg"
"Wybrany podświetlony tekst "
"Texto resaltado seleccionado"
"Vybraný zvýraznený text"

MSetColorMenuDisabled
"Недоступный пункт"
"Disabled text"
"Neaktivní text"
"Disabled text"
"Inaktív szöveg"
"Tekst nieaktywny"
"Texto desactivado"
"Neaktívny text"

MSetColorMenuGrayed
"Серый текст"
"Grayed text"
upd:"Grayed text"
upd:"Grayed text"
"Szürke szöveg"
upd:"Grayed text"
"Texto en gris"
"Grayed text"

MSetColorMenuSelectedGrayed
"Выбранный серый текст"
"Selected grayed text"
upd:"Selected grayed text"
upd:"Selected grayed text"
"Kijelölt szürke szöveg"
upd:"Selected grayed text"
"Texto en gris seleccionado"
"Selected grayed text"

MSetColorMenuBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"
"Ramka"
"Borde"
"Okraj"

MSetColorMenuTitle
"Заголовок"
"Title"
"Nadpis"
"Titel"
"Keret neve"
"Tytuł"
"Título"
"Názov"

MSetColorMenuScrollBar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"
"Suwak"
"Barra desplazamiento"
"Posuvník"

MSetColorMenuArrows
"Индикаторы длинных строк"
"Long string indicators"
"Značka dlouhého řetězce"
"Long string indicators"
"Hosszú sztring jelzők"
"Znacznik długiego napisu"
"Indicadores de cadena larga"
"Značka dlhého reťazca"

MSetColorMenuArrowsSelected
"Выбранные индикаторы длинных строк"
"Selected long string indicators"
"Vybraná značka dlouhého řetězce"
"Selected long string indicators"
"Kijelölt hosszú sztring jelzők"
"Zaznaczone znacznik długiego napisu"
"Indicadores de cadena larga seleccionados"
"Vybraná značka dlhého reťazca"

MSetColorMenuArrowsDisabled
"Блокированные индикаторы длинных строк"
"Disabled long string indicators"
"Zakázaná značka dlouhého řetězce"
"Disabled long string indicators"
"Inaktív hosszú sztring jelzők"
"Nieaktywny znacznik długiego napisu"
"Indicadores de cadena larga desactivados"
"Zakázaná značka dlhého reťazca"

MSetColorHMenuNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"
"Normalny tekst"
"Texto normal"
"Normálny text"

MSetColorHMenuSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"
"Wybrany tekst"
"Texto seleccionado"
"Vybraný text"

MSetColorHMenuHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"
"Kiemelt szöveg"
"Podświetlony tekst"
"Texto resaltado"
"Zvýraznený text"

MSetColorHMenuSelectedHighlighted
"Выбранный выделенный текст"
"Selected highlighted text"
"Vybraný zvýrazněný text"
"Aktive Markierung"
"Kijelölt kiemelt szöveg"
"Wybrany podświetlony tekst "
"Texto resaltado seleccionado"
"Vybraný zvýraznený text"

MSetColorKeyBarNumbers
l:
"Номера клавиш"
"Key numbers"
"Čísla kláves"
"Tastenziffern"
"Funkció száma"
"Numery klawiszy"
"Números de teclas"
"Čísla klávesov"

MSetColorKeyBarNames
"Названия клавиш"
"Key names"
"Názvy kláves"
"Tastennamen"
"Funkció neve"
"Nazwy klawiszy"
"Nombres de teclas"
"Názvy klávesov"

MSetColorKeyBarBackground
"Фон"
"Background"
"Pozadí"
"Hintergrund"
"Háttere"
"Tło"
"Fondo"
"Pozadie"

MSetColorCommandLineNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"
"Normalny tekst"
"Texto normal"
"Normálny text"

MSetColorCommandLineSelected
"Выделенный текст"
"Selected text input"
"Vybraný textový vstup"
"Markierte Texteingabe"
"Beírt szöveg kijelölve"
"Zaznaczony wpisany tekst"
"Entrada de texto seleccionada"
"Vybraný textový vstup"

MSetColorCommandLinePrefix
"Текст префикса"
"Prefix text"
"Text předpony"
"Prefix Text"
"Előtag szövege"
"Tekst prefiksu"
"Texto prefijado"
"Text predpony"

MSetColorCommandLineUserScreen
"Пользовательский экран"
"User screen"
"Obrazovka uživatele"
"Benutzerseite"
"Konzol háttere"
"Ekran użytkownika"
"Pantalla de usuario"
"Okno užívateľa"

MSetColorClockNormal
l:
"Обычный текст (панели)"
"Normal text (Panel)"
"Normální text (Panel)"
"Normaler Text (Panel)"
"Normál szöveg (panelek)"
"Normalny tekst (Panel)"
"Texto normal (Panel)"
"Normálny text (panel)"

MSetColorClockNormalEditor
"Обычный текст (редактор)"
"Normal text (Editor)"
"Normální text (Editor)"
"Normaler Text (Editor)"
"Normál szöveg (szerkesztő)"
"Normalny tekst (Edytor)"
"Texto normal (Editor)"
"Normálny text (editor)"

MSetColorClockNormalViewer
"Обычный текст (вьювер)"
"Normal text (Viewer)"
"Normální text (Prohlížeč)"
"Normaler Text (Betrachter)"
"Normál szöveg (nézőke)"
"Normalny tekst (Podgląd)"
"Texto normal (Visor)"
"Normálny text (zobrazovač)"

MSetColorViewerNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"
"Normalny tekst"
"Texto normal"
"Normálny text"

MSetColorViewerSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"
"Zaznaczony tekst"
"Texto seleccionado"
"Vybraný text"

MSetColorViewerStatus
"Статус"
"Status line"
"Stavový řádek"
"Statuszeile"
"Állapotsor"
"Linia statusu"
"Línea de estado"
"Stavový riadok"

MSetColorViewerArrows
"Стрелки сдвига экрана"
"Screen scrolling arrows"
"Skrolovací šipky"
"Pfeile auf Scrollbalken"
"Képernyőgördítő nyilak"
"Strzałki przesuwające ekran"
"Flechas desplazamiento de pantalla"
"Šípky posuvníka"

MSetColorViewerScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"
"Suwak"
"Barras desplazamiento"
"Posuvník"

MSetColorEditorNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"
"Normalny tekst"
"Texto normal"
"Normálny text"

MSetColorEditorSelected
"Выбранный текст"
"Selected text"
"Vybraný text"
"Markierter Text"
"Kijelölt szöveg"
"Zaznaczony tekst"
"Texto seleccionado"
"Vybraný text"

MSetColorEditorStatus
"Статус"
"Status line"
"Stavový řádek"
"Statuszeile"
"Állapotsor"
"Linia statusu"
"Línea de estado"
"Stavový riadok"

MSetColorEditorScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"
"Suwak"
"Barra de desplazamiento"
"Posuvník"

MSetColorHelpNormal
l:
"Обычный текст"
"Normal text"
"Normální text"
"Normaler Text"
"Normál szöveg"
"Normalny tekst"
"Texto normal"
"Normálny text"

MSetColorHelpHighlighted
"Выделенный текст"
"Highlighted text"
"Zvýrazněný text"
"Markierung"
"Kiemelt szöveg"
"Podświetlony tekst"
"Texto resaltado"
"Zvýraznený text"

MSetColorHelpReference
"Ссылка"
"Reference"
"Odkaz"
"Referenz"
"Hivatkozás"
"Odniesienie"
"Referencia"
"Odkaz"

MSetColorHelpSelectedReference
"Выбранная ссылка"
"Selected reference"
"Vybraný odkaz"
"Ausgewählte Referenz"
"Kijelölt hivatkozás"
"Wybrane odniesienie "
"Referencia seleccionada"
"Vybraný odkaz"

MSetColorHelpBox
"Рамка"
"Border"
"Okraj"
"Rahmen"
"Keret"
"Ramka"
"Borde"
"Okraj"

MSetColorHelpBoxTitle
"Заголовок рамки"
"Title"
"Nadpis"
"Titel"
"Keret neve"
"Tytuł"
"Título"
"Názov"

MSetColorHelpScrollbar
"Полоса прокрутки"
"Scrollbar"
"Posuvník"
"Scrollbalken"
"Gördítősáv"
"Suwak"
"Barra desplazamiento"
"Posuvník"

MSetColorGroupsTitle
l:
"Цветовые группы"
"Color groups"
"Skupiny barev"
"Farbgruppen"
"Színcsoportok"
"Grupy kolorów"
"Grupos de colores"
"Skupiny farieb"

MSetColorItemsTitle
"Элементы группы"
"Group items"
"Položky skupin"
"Gruppeneinträge"
"A színcsoport elemei"
"Elementy grupy"
"Grupos de ítems"
"Položky skupín"

MSetColorTitle
l:
"Цвет"
"Color"
"Barva"
"Farbe"
"Színek"
"Kolor"
"Color"
"Farba"

MSetColorForeground
"&Текст"
"&Foreground"
"&Popředí"
"&Vordergrund"
"&Előtér"
"&Pierwszy plan"
"&Caracteres"
"&Popredie"

MSetColorBackground
"&Фон"
"&Background"
"Po&zadí"
"&Hintergrund"
"&Háttér"
"&Tło"
"&Fondo     "
"Po&zadie"

MSetColorForeTransparent
"&Прозрачный"
"&Transparent"
"Průhlednos&t"
"&Transparent"
"Átlá&tszó"
"P&rzezroczyste"
"&Transparente"
"Priehľadnos&ť"

MSetColorBackTransparent
"П&розрачный"
"T&ransparent"
"Průhledno&st"
"T&ransparent"
"Átlát&szó"
"Pr&zezroczyste"
"T&ransparente"
"Priehľadno&sť"

MSetColorSample
"Текст Текст Текст Текст Текст Текст"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"
"Text Text Text Text Text Text Text"
"Tekst Tekst Tekst Tekst Tekst Tekst"
"Texto Texto Texto Texto Texto"
"Text Text Text Text Text Text Text"

MSetColorSet
"Установить"
"Set"
"Nastavit"
"Setzen"
"A&lkalmaz"
"Ustaw"
"Poner"
"Nastaviť"

MSetColorCancel
"Отменить"
"Cancel"
"Storno"
"Abbruch"
"&Mégsem"
"Anuluj"
"Cancelar"
"Storno"

MSetConfirmTitle
l:
"Подтверждения"
"Confirmations"
"Potvrzení"
"Bestätigungen"
"Megerősítések"
"Potwierdzenia"
"Confirmaciones"
"Potvrdiť"

MSetConfirmCopy
"Перезапись файлов при &копировании"
"&Copy"
"&Kopírování"
"&Kopieren"
"&Másolás"
"&Kopiowanie"
"&Copiar"
"&Kopírovanie"

MSetConfirmMove
"Перезапись файлов при &переносе"
"&Move"
"&Přesouvání"
"&Verschieben"
"Moz&gatás"
"&Przenoszenie"
"&Mover"
"&Presunutie"

MSetConfirmRO
"Перезапись и удаление R/O &файлов"
"&Overwrite and delete R/O files"
upd:"&Overwrite and delete R/O files"
upd:"&Overwrite and delete R/O files"
"&Csak olv. fájlok felülírása/törlése"
upd:"&Overwrite and delete R/O files"
"S&obrescribir y borrar archivos Sólo/Lectura"
"Pr&epísanie a zmazanie súborov R/O"


MSetConfirmDrag
"Пере&таскивание"
"&Drag and drop"
"&Drag and drop"
"&Ziehen und Ablegen"
"&Húzd és ejtsd"
"P&rzeciąganie i upuszczanie"
"&Arrastrar y soltar"
"Po&tiahni a pusť"

MSetConfirmDelete
"&Удаление"
"De&lete"
"&Mazání"
"&Löschen"
"&Törlés"
"&Usuwanie"
"&Borrar"
"Z&mazanie"

MSetConfirmDeleteFolders
"У&даление непустых папок"
"Delete non-empty &folders"
"Mazat &neprázdné adresáře"
"Löschen von Ordnern mit &Inhalt"
"Nem &üres mappák törlése"
"Usuwanie &niepustych katalogów"
"Borrar &directorios no-vacíos"
"Zmazanie &neprázdnych priečinkov"

MSetConfirmEsc
"Прерыва&ние операций"
"&Interrupt operation"
"Pře&rušit operaci"
"&Unterbrechen von Vorgängen"
"Mű&velet megszakítása"
"&Przerwanie operacji"
"&Interrumpir operación"
"Pre&rušenie operácie"

MSetConfirmRemoveConnection
"&Отключение сетевого устройства"
"Disconnect &network drive"
"Odpojení &síťové jednotky"
"Trennen von &Netzwerklaufwerken"
"Háló&zati meghajtó leválasztása"
"Odłączenie dysku &sieciowego"
"Desconectar u&nidad de red"
"Odpojenie &sieťovej jednotky"

MSetConfirmRemoveSUBST
"Отключение SUBST-диска"
"Disconnect &SUBST-disk"
"Odpojení SUBST-d&isku"
"Trennen von &Substlaufwerken"
"Virt&uális meghajtó törlése"
"Odłączenie dysku &SUBST"
"Desconectar disco s&ustituido"
"Odpojenie d&isku SUBST"

MSetConfirmDetachVHD
"Отсоедиение виртуального диска"
"Detach virtual disk"
upd:"Detach virtual disk"
upd:"Detach virtual disk"
upd:"Detach virtual disk"
upd:"Detach virtual disk"
"Desmontar disco virtual"
"Odpojenie virtuálneho disku"

MSetConfirmRemoveHotPlug
"Отключение HotPlug-у&стройства"
"Hot&Plug-device removal"
"Odpojení vyjímatelného zařízení"
"Sicheres Entfernen von Hardware"
"H&otPlug eszköz eltávolítása"
"Odłączanie urządzenia HotPlug"
"Remover dispositivo Hot&Plug"
"Odpojenie vyberateľného zariadenia"

MSetConfirmAllowReedit
"Повто&рное открытие файла в редакторе"
"&Reload edited file"
"&Obnovit upravovaný soubor"
"Bea&rbeitete Datei neu laden"
"&Szerkesztett fájl újratöltése"
"&Załaduj edytowany plik"
"&Recargar archivo editado"
"&Obnovenie upravovaného súboru"

MSetConfirmHistoryClear
"Очистка списка &истории"
"Clear &history list"
"Vymazat seznam &historie"
"&Historielisten löschen"
"&Előzménylista törlése"
"Czyszczenie &historii"
"Limpiar listado de &historial"
"Vymazanie zoznamu &histórie"

MSetConfirmExit
"&Выход"
"E&xit"
"U&končení"
"Be&enden"
"K&ilépés a FAR-ból"
"&Wyjście"
"&Salir"
"Uk&ončenie"

MPluginsManagerSettingsTitle
l:
"Параметры менеджера плагинов"
"Plugins manager settings"
upd:"Plugins manager settings"
upd:"Plugins manager settings"
upd:"Plugins manager settings"
upd:"Plugins manager settings"
"Configuración de gestor de complementos"
"Nastavenia správcu modulov"

MPluginsManagerOEMPluginsSupport
"Поддержка OEM-плагинов"
"OEM plugins support"
upd:"OEM plugins support"
upd:"OEM plugins support"
upd:"OEM plugins support"
upd:"OEM plugins support"
"Soporte de complementos OEM"
"Podpora modulov OEM"

MPluginsManagerScanSymlinks
"Ск&анировать символические ссылки"
"Scan s&ymbolic links"
"Prohledávat s&ymbolické linky"
"S&ymbolische Links scannen"
"Szimbolikus linkek &vizsgálata"
"Skanuj linki s&ymboliczne"
"Explorar enlaces simbólicos"
"Prehľadávať s&ymbolické prepojenia"

MPluginsManagerOFP
"Обработка &файла"
"&File processing"
upd:"&File processing"
upd:"&File processing"
"&Fájl feldolgozása"
upd:"&File processing"
"Proceso de archivo"
"Spra&covanie súboru"

MPluginsManagerStdAssoc
"Пункт вызова стандартной &ассоциации"
"Show standard &association item"
upd:"Show standard &association item"
upd:"Show standard &association item"
"Szabvány társítás megjelenítése"
upd:"Show standard &association item"
"Mostrar asociaciones normales de ítems"
"Zobrazi&ť položku štandardnej asociácie"

MPluginsManagerEvenOne
"Даже если найден всего &один плагин"
"Even if only &one plugin found"
upd:"Even if only &one plugin found"
upd:"Even if only &one plugin found"
"Akkor is, ha csak egy plugin van"
upd:"Even if only &one plugin found"
"Aún si solo se encontró un complemento"
"aj keď sa náj&de len jeden modul"

MPluginsManagerSFL
"&Результаты поиска (SetFindList)"
"Search &results (SetFindList)"
upd:"Search &results (SetFindList)"
upd:"Search &results (SetFindList)"
"Keresés eredménye (SetFindList)"
upd:"Search &results (SetFindList)"
"Resultados de búsqueda (SetFindList)"
"Výs&ledky hľadania (SetFindList)"

MPluginsManagerPF
"Обработка &префикса"
"&Prefix processing"
upd:"&Prefix processing"
upd:"&Prefix processing"
"Előtag feldolgozása"
upd:"&Prefix processing"
"Proceso de prefijo"
"S&pracovanie predpony"

MPluginConfirmationTitle
"Выбор плагина"
"Plugin selection"
upd:"Plugin selection"
upd:"Plugin selection"
"Plugin választás"
upd:"Plugin selection"
" Selección de complemento "
"Výber modulu"

MMenuPluginStdAssociation
"Стандартная ассоциация"
"Standard association"
upd:"Standard association"
upd:"Standard association"
"Szabvány társítás"
upd:"Standard association"
"Asociación normal"
"Štandardné asociácie"

MFindFolderTitle
l:
"Поиск папки"
"Find folder"
"Najít adresář"
"Ordner finden"
"Mappakeresés"
"Znajdź folder"
"Encontrar directorio"
"Nájsť priečinok"

MKBFolderTreeF1
l:
l:// Find folder Tree KeyBar
"Помощь"
"Help"
"Nápověda"
"Hilfe"
"Súgó"
"Pomoc"
"Ayuda"
"Pomocník"

MKBFolderTreeF2
"Обновить"
"Rescan"
"Obnovit"
"Aktual"
"FaFris"
"Czytaj ponownie"
"ReExpl"
"Obnoviť"

MKBFolderTreeF5
"Размер"
"Zoom"
"Zoom"
"Vergr."
"Nagyít"
"Powiększ"
"Zoom"
"Zoom"

MKBFolderTreeF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"
"Koniec"
"Salir"
"Koniec"

MKBFolderTreeAltF9
"Видео"
"Video"
"Video"
"Vollb"
"Video"
"Video"
"Video"
"Video"

MTreeTitle
"Дерево"
"Tree"
"Stromové zobrazení"
"Baum"
"Fa"
"Drzewo"
"Arbol"
"Stromové zobrazenie"

MCannotSaveTree
"Ошибка записи дерева папок в файл"
"Cannot save folders tree to file"
"Adresářový strom nelze uložit do souboru"
"Konnte Ordnerliste nicht in Datei speichern."
"A mappák fastruktúrája nem menthető fájlba"
"Nie mogę zapisać drzewa katalogów do pliku"
"No se puede guardar árbol de directorios al archivo"
"Strom priečinkov sa nedá uložiť do súboru"

MReadingTree
"Чтение дерева папок"
"Reading the folders tree"
"Načítám adresářový strom"
"Lese Ordnerliste"
"Mappaszerkezet újraolvasása..."
"Odczytuję drzewo katalogów"
"Leyendo árbol de directorios"
"Načítavam strom priečinkov"

MUserMenuTitle
l:
"Пользовательское меню"
"User menu"
"Menu uživatele"
"Benutzermenü"
"Felhasználói menü szerkesztése"
"Menu użytkownika"
"Menú de usuario"
"Užívateľské menu"

MChooseMenuType
"Выберите тип пользовательского меню для редактирования"
"Choose user menu type to edit"
"Zvol typ menu uživatele pro úpravu"
"Wählen Sie den Typ des zu editierenden Benutzermenüs"
"Felhasználói menü típusa:"
"Wybierz typ menu do edycji"
"Elija tipo de menú usuario a editar"
"Zvoľte typ užívateľského menu na úpravu"

MChooseMenuMain
"&Главное"
"&Main"
"&Hlavní"
"&Hauptmenü"
"&Főmenü"
"Główne"
"&Principal"
"&Hlavné"

MChooseMenuLocal
"&Местное"
"&Local"
"&Lokální"
"&Lokales Menü"
"&Helyi menü"
"Lokalne"
"&Local"
"&Miestne"

MMainMenuTitle
"Главное меню"
"Main menu"
"Hlavní menu"
"Hauptmenü"
"Főmenü"
"Menu główne"
"Menú principal"
"Hlavné menu"

MMainMenuUser
l:
l:// <...menu (User)>
"Пользовательское"
"User"
upd:"User"
upd:"User"
upd:"User"
upd:"User"
"Usuario"
"Užívateľ"

MMainMenuGlobal
"Глобальное"
"Global"
upd:"Global"
upd:"Global"
upd:"Global"
upd:"Global"
"Global"
"Global"

MLocalMenuTitle
"Местное меню"
"Local menu"
"Lokalní menu"
"Lokales Menü"
"Helyi menü"
"Menu lokalne"
"Menú local"
"Miestne menu"

MMainMenuBottomTitle
"Редактирование: Del,Ins,F4,Alt-F4"
"Edit: Del,Ins,F4,Alt-F4"
"Edit: Del,Ins,F4,Alt-F4"
"Bearb.: Entf,Einf,F4,Alt-F4"
"Szerk.: Del,Ins,F4,Alt-F4"
"Edycja: Del,Ins,F4,Alt-F4"
"Editar: Del,Ins,F4"
"Upraviť: Del,Ins,F4,Alt-F4"

MAskDeleteMenuItem
"Вы хотите удалить пункт меню"
"Do you wish to delete the menu item"
"Přejete si smazat položku v menu"
"Do you wish to delete the menu item"
"Biztosan törli a menüelemet?"
"Czy usunąć pozycję menu"
"Desea borrar el ítem del menú"
"chcete zmazať položku v menu"

MAskDeleteSubMenuItem
"Вы хотите удалить вложенное меню"
"Do you wish to delete the submenu"
"Přejete si smazat podmenu"
"Do you wish to delete the submenu"
"Biztosan törli az almenüt?"
"Czy usunąć podmenu"
"Desea borrar el submenú"
"Chcete zmazať podmenu"

MUserMenuInvalidInputLabel
"Неправильный формат метки меню"
"Invalid format for UserMenu Label"
"Neplatný formát pro název Uživatelského menu"
"Invalid format for UserMenu Label"
"A felhasználói menü névformátuma érvénytelen"
"Błędny format etykiety menu użytkownika"
"Formato inválido para etiqueta de menú usuario"
"Neplatný formát názvu užívateľského menu"

MUserMenuInvalidInputHotKey
"Неправильный формат горячей клавиши"
"Invalid format for Hot Key"
"Neplatný formát pro klávesovou zkratku"
"Invalid format for Hot Key"
"A gyorsbillentyű formátuma érvénytelen"
"Błędny format klawisza skrótu"
"Formato inválido para tecla de atajo"
"Neplatný formát klávesovej skratky"

MEditMenuTitle
l:
"Редактирование пользовательского меню"
"Edit user menu"
"Editace uživatelského menu"
"Menübefehl bearbeiten"
"Parancs szerkesztése"
"Edytuj menu użytkownika"
"Editar menú de usuario"
"Úprava užívateľského menu"

MEditMenuHotKey
"&Горячая клавиша:"
"&Hot key:"
"K&lávesová zkratka:"
"&Kurztaste:"
"&Gyorsbillentyű:"
"&Klawisz skrótu:"
"&Tecla de atajo:"
"K&lávesová skratka:"

MEditMenuLabel
"&Метка:"
"&Label:"
"&Popisek:"
"&Bezeichnung:"
"&Név:"
"&Etykieta:"
"&Etiqueta:"
"&Názov:"

MEditMenuCommands
"&Команды:"
"&Commands:"
"Pří&kazy:"
"&Befehle:"
"&Parancsok:"
"&Polecenia:"
"&Comandos:"
"Prí&kazy:"

MAskInsertMenuOrCommand
l:
"Вы хотите вставить новую команду или новое меню?"
"Do you wish to insert a new command or a new menu?"
"Přejete si vložit nový příkaz nebo nové menu?"
"Wollen Sie einen neuen Menübefehl oder ein neues Menu erstellen?"
"Új parancs vagy új menü?"
"Czy chcesz wstawić nowe polecenie lub nowe menu?"
"Desea insertar un nuevo comando o un nuevo menú?"
"Chcete vložiť nový príkaz alebo nové menu?"

MMenuInsertCommand
"Вставить команду"
"Insert command"
"Vložit příkaz"
"Neuer Befehl"
"Parancs"
"Wstaw polecenie"
"Insertar comando"
"Vložiť príkaz"

MMenuInsertMenu
"Вставить меню"
"Insert menu"
"Vložit menu"
"Neues Menü"
"Menü"
"Wstaw menu"
"Insertar menú"
"Vložiť menu"

MEditSubmenuTitle
l:
"Редактирование метки вложенного меню"
"Edit submenu label"
"Úprava popisku podmenu"
"Untermenü bearbeiten"
"Almenü szerkesztése"
"Edytuj etykietę podmenu"
"Editar etiqueta de submenú"
"Úprava názvu podmenu"

MEditSubmenuHotKey
"&Горячая клавиша:"
"&Hot key:"
"Klávesová &zkratka:"
"&Kurztaste:"
"&Gyorsbillentyű:"
"&Klawisz skrótu:"
"&Tecla de atajo:"
"Klávesová &skratka:"

MEditSubmenuLabel
"&Метка:"
"&Label:"
"&Popisek:"
"&Bezeichnung:"
"&Név:"
"&Etykieta:"
"&Etiqueta:"
"&Názov:"

MViewerTitle
l:
"Просмотр"
"Viewer"
"Prohlížeč"
"Betrachter"
"Nézőke"
"Podgląd"
"Visor"
"Zobrazovač"

MViewerCannotOpenFile
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"
"A fájl nem nyitható meg"
"Nie mogę otworzyć pliku"
"No se puede abrir el archivo"
"Nemôžem otvoriť súbor"

MViewerStatusCol
"Кол"
"Col"
"Sloupec"
"Spalte"
"Oszlop"
"Kolumna"
"Col"
"Stĺpec"

MViewSearchTitle
l:
"Поиск"
"Search"
"Hledat"
"Durchsuchen"
"Keresés"
"Szukaj"
"Buscar"
"Hľadanie"

MViewSearchFor
"&Искать"
"&Search for"
"H&ledat"
"&Suchen nach"
"&Keresés:"
"&Znajdź"
"&Buscar por"
"H&ľadať"

MViewSearchForText
"Искать &текст"
"Search for &text"
"Hledat &text"
"Suchen nach &Text"
"&Szöveg keresése"
"Szukaj &tekstu"
"Buscar cadena de &texto"
"Hľadať &text"

MViewSearchForHex
"Искать 16-ричный &код"
"Search for &hex"
"Hledat he&x"
"Suchen nach &Hex (xx xx ...)"
"&Hexa keresése"
"Szukaj &wartości szesnastkowych"
"Buscar cadena &hexadecimal"
"Hľadať he&x"

MViewSearchCase
"&Учитывать регистр"
"&Case sensitive"
"&Rozlišovat velikost písmen"
"Gr&oß-/Kleinschreibung"
"&Nagy/kisbetű érzékeny"
"&Uwzględnij wielkość liter"
"Sensible min/ma&yúsculas"
"&malé a VEĽKÉ"

MViewSearchWholeWords
"Только &целые слова"
"&Whole words"
"Celá &slova"
"Ganze &Wörter"
"Csak e&gész szavak"
"Tylko całe słowa"
"&Palabras completas"
"Celé &slová"

MViewSearchReverse
"Обратн&ый поиск"
"Re&verse search"
"&Zpětné hledání"
"Richtung um&kehren"
"&Visszafelé keres"
"Szukaj w &odwrotnym kierunku"
"Buscar al in&verso"
"&Opačné hľadanie"

MViewSearchRegexp
"&Регулярные выражения"
"Re&gular expressions"
upd:"Re&gular expressions"
upd:"Re&gular expressions"
upd:"Re&gular expressions"
upd:"Re&gular expressions"
"Expresiones re&gulares"
"Re&gulárne výrazy"

MViewSearchSearch
"Искать"
"Search"
"Hledat"
"Suchen"
"Keres"
"&Szukaj"
"Buscar"
"Hľadať"

MViewSearchCancel
"Отменить"
"Cancel"
"Storno"
"Abbrechen"
"Mégsem"
"&Anuluj"
"Cancelar"
"Storno"

MViewSearchingFor
l:
"Поиск"
"Searching for"
"Vyhledávám"
"Suche nach"
"Keresés:"
"Szukam"
"Buscando por"
"Vyhľadávam"

MViewSearchingHex
"Поиск байтов"
"Searching for bytes"
"Vyhledávám sekvenci bytů"
"Suche nach Bytes"
"Bájtok keresése:"
"Szukam bajtów"
"Buscando por bytes"
"Vyhľadávam sled bajtov"

MViewSearchCannotFind
"Строка не найдена"
"Could not find the string"
"Nelze najít řetězec"
"Konnte Zeichenkette nicht finden"
"Nem találtam a szöveget:"
"Nie mogę odnaleźć ciągu znaków"
"No se puede encontrar la cadena"
"Nemôžem nájsť reťazec"

MViewSearchCannotFindHex
"Байты не найдены"
"Could not find the bytes"
"Nelze najít sekvenci bytů"
"Konnte Bytefolge nicht finden"
"Nem találtam a bájtokat:"
"Nie mogę odnaleźć bajtów"
"No se puede encontrar los bytes"
"Nemôžem nájsť sekvenciu bajtov"

MViewSearchFromBegin
"Продолжить поиск с начала документа?"
"Continue the search from the beginning of the document?"
"Pokračovat s hledáním od začátku dokumentu?"
"Mit Suche am Anfang des Dokuments fortfahren?"
"Folytassam a keresést a dokumentum elejétől?"
"Kontynuować wyszukiwanie od początku dokumentu?"
"Continuar búsqueda desde el comienzo del documento"
"Pokračovať v hľadaní od začiatku dokumentu?"

MViewSearchFromEnd
"Продолжить поиск с конца документа?"
"Continue the search from the end of the document?"
"Pokračovat s hledáním od konce dokumentu?"
"Mit Suche am Ende des Dokuments fortfahren?"
"Folytassam a keresést a dokumentum végétől?"
"Kontynuować wyszukiwanie od końca dokumentu?"
"Continuar búsqueda desde el final del documento"
"Pokračovať v hľadaní od konca dokumentu?"

MPrintTitle
l:
"Печать"
"Print"
"Tisk"
"Drucken"
"Nyomtatás"
"Drukuj"
"Imprimir"
"Tlač"

MPrintTo
"Печатать %1 на"
"Print %1 to"
"Vytisknout %1 na"
"Drucke %1 nach"
"%1 nyomtatása:"
"Drukuj %1 do"
"Imprimir %1 a"
"Vytlačiť %1 na"

MPrintFilesTo
"Печатать %1 файлов на"
"Print %1 files to"
"Vytisknout %1 souborů na"
"Drucke %1 Dateien mit"
"%1 fájl nyomtatása:"
"Drukuj %1 pliki(ów) do"
"Imprimir %1 archivos a"
"Vytlačiť %1 súborov na"

MPreparingForPrinting
"Подготовка файлов к печати"
"Preparing files for printing"
"Připravuji soubory pro tisk"
"Vorbereiten der Druckaufträge"
"Fájlok előkészítése nyomtatáshoz"
"Przygotowuję plik(i) do drukowania"
"Preparando archivos para imprimir"
"Pripravujem súbory na tlač"

MCannotEnumeratePrinters
"Не удалось получить список доступных принтеров"
"Cannot enumerate available printers list"
upd:"Cannot enumerate available printers list"
upd:"Cannot enumerate available printers list"
"Az elérhető nyomtatók listája nem állítható össze"
upd:"Cannot enumerate available printers list"
"No se puede enumerar lista de impresoras disponibles"
"Cannot enumerate available printers list"

MCannotOpenPrinter
"Не удалось открыть принтер"
"Cannot open printer"
"Nelze otevřít tiskárnu"
"Fehler beim öffnen des Druckers"
"Nyomtató nem elérhető"
"Nie mogę połączyć się z drukarką"
"No se puede abrir impresora"
"Nemôžem otvoriť tlačiareň"

MCannotPrint
"Не удалось распечатать"
"Cannot print"
"Nelze tisknout"
"Fehler beim Drucken"
"Nem nyomtatható"
"Nie mogę drukować"
"No se puede imprimir"
"Nemôžem tlačiť"

MDescribeFiles
l:
"Описание файла"
"Describe file"
"Popiskový soubor"
"Beschreibung ändern"
"Fájlmegjegyzés"
"Opisz plik"
"Describir archivos"
"Súbor popisov"

MEnterDescription
"Введите описание для"
"Enter description for"
"Zadejte popisek"
"Beschreibung für"
upd:"Írja be megjegyzését:"
"Wprowadź opis"
"Entrar descripción de"
"Zadajte popis"

MReadingDiz
l:
"Чтение описаний файлов"
"Reading file descriptions"
"Načítám popisky souboru"
"Lese Dateibeschreibungen"
"Fájlmegjegyzések olvasása"
"Odczytuję opisy plików"
"Leyendo descripción de archivos"
"Načítavam popisy súboru"

MCannotUpdateDiz
"Не удалось обновить описания файлов"
"Cannot update file descriptions"
"Nelze aktualizovat popisky souboru"
"Dateibeschreibungen konnten nicht aktualisiert werden."
"A fájlmegjegyzések nem frissíthetők"
"Nie moge aktualizować opisów plików"
"No se puede actualizar descripción de archivos"
"Nemôžem aktualizovať popisy súboru"

MCannotUpdateRODiz
"Файл описаний защищён от записи"
"The description file is read only"
"Popiskový soubor má atribut Jen pro čtení"
"Die Beschreibungsdatei ist schreibgeschützt."
"A megjegyzésfájl csak olvasható"
"Opis jest plikiem tylko do odczytu"
"El archivo descripción es de sólo lectura"
"Súbor popisov je len na čítanie"

MCfgDizTitle
l:
"Описания файлов"
"File descriptions"
"Popisky souboru"
"Dateibeschreibungen"
"Fájl megjegyzésfájlok"
"Opisy plików"
"Descripción de archivos"
"Popisy súborov"

MCfgDizListNames
"Имена &списков описаний, разделённые запятыми:"
"Description &list names delimited with commas:"
"Seznam pop&isových souborů oddělených čárkami:"
"Beschreibungs&dateien, getrennt durch Komma:"
"Megjegyzés&fájlok nevei, vesszővel elválasztva:"
"Nazwy &plików z opisami oddzielone przecinkami:"
"Nombres de &listas de descripción delimitado con comas:"
"Zoznam pop&isov súborov oddelených čiarkami:"

MCfgDizSetHidden
"Устанавливать &атрибут ""Скрытый"" на новые списки описаний"
"Set ""&Hidden"" attribute to new description lists"
"Novým souborům s popisy nastavit atribut ""&Skrytý"""
"Setze das '&Versteckt'-Attribut für neu angelegte Dateien"
"Az új megjegyzésfájl ""&rejtett"" attribútumú legyen"
"Ustaw atrybut ""&Ukryty"" dla nowych plików z opisami"
"Poner atributo ""&Oculto"" a las nuevas listas de descripción"
"Nové súbory s popismi nastaviť s atribútom ""&Skrytý"""

MCfgDizROUpdate
"Обновлять файл описаний с атрибутом ""Толь&ко для чтения"""
"Update &read only description file"
"Aktualizovat popisové soubory s atributem Jen pro čtení"
"Schreibgeschützte Dateien aktualisie&ren"
"&Csak olvasható megjegyzésfájlok frissítése"
"Aktualizuj plik opisu tylko do odczytu"
"&Actualizar archivo descripción de sólo lectura"
"Aktualizovať popisové súbory s atribútom Len na čítanie"

MCfgDizStartPos
"&Позиция новых описаний в строке"
"&Position of new descriptions in the string"
"&Pozice nových popisů v řetězci"
"&Position neuer Beschreibungen in der Zeichenkette"
"Új megjegyzéseknél a szöveg &kezdete"
"Pozy&cja nowych opisów w linii"
"&Posición de nueva descripciones en la cadena"
"&Pozícia nových popisov v reťazci"

MCfgDizNotUpdate
"&Не обновлять описания"
"Do &not update descriptions"
"&Neaktualizovat popisy"
"Beschreibungen &nie aktualisieren"
"N&e frissítse a megjegyzéseket"
"&Nie aktualizuj opisów"
"&No actualizar descripciones"
"&Neaktualizovať popisy"

MCfgDizUpdateIfDisplayed
"&Обновлять, если они выводятся на экран"
"Update if &displayed"
"Aktualizovat, jestliže je &zobrazen"
"Aktualisieren &wenn angezeigt"
"Frissítsen, ha meg&jelenik"
"Aktualizuj jeśli &widoczne"
"Actualizar si es &visualizado"
"Aktualizovať, ak sa &zobrazuje"

MCfgDizAlwaysUpdate
"&Всегда обновлять"
"&Always update"
"&Vždy aktualizovat"
"Im&mer aktualisieren"
"&Mindig frissítsen"
"&Zawsze aktualizuj"
"Actualizar &siempre"
"&Vždy aktualizovať"

MCfgDizAnsiByDefault
"&Использовать кодовую страницу ANSI по умолчанию"
"Use ANS&I code page by default"
upd:"Automaticky otevírat soubory ve &WIN kódování"
upd:"Dateien standardmäßig mit Windows-Kod&ierung öffnen"
"Fájlok eredeti megnyitása ANS&I kódlappal"
"&Otwieraj pliki w kodowaniu Windows"
"Usar página de códigos ANS&I por defecto"
"Automaticky otvárať súbory v kódovaní &WIN"

MCfgDizSaveInUTF
"Сохранять в UTF8"
"Save in UTF8"
upd:"Save in UTF8"
upd:"Save in UTF8"
upd:"Save in UTF8"
upd:"Save in UTF8"
"Guardar en UTF8"
"Uložiť v UTF8"

MReadingTitleFiles
l:
"Обновление панелей"
"Update of panels"
"Aktualizace panelů"
"Aktualisiere Panels"
"Panelek frissítése"
"Aktualizacja panelu"
"Actualización de paneles"
"Aktualizácia panelov"

MReadingFiles
"Чтение: %1 файлов"
"Reading: %1 files"
"Načítám: %1 souborů"
"Lese: %1 Dateien"
" %1 fájl olvasása"
"Czytam: %1 plików"
"Leyendo: %1 archivos"
"Načítavam: %1 súborov"

MOperationNotCompleted
"Операция не завершена"
"Operation not completed"
"Operace není dokončena"
"Vorgang nicht abgeschlossen"
"A művelet félbeszakadt"
"Operacja nie doprowadzona do końca"
"Operación no completada"
"Operácia sa nedokončila"

MEditPanelModes
l:
"Режимы панели"
"Edit panel modes"
"Editovat módy panelu"
"Anzeigemodi von Panels bearbeiten"
"Panel módok szerkesztése"
"Edytuj tryby wyświetlania paneli"
"Editar modo de paneles"
"Upraviť módy panelov"

MEditPanelModesBrief
l:
"&Краткий режим"
"&Brief mode"
"&Stručný mód"
"&Kurz"
"&Rövid mód"
"&Skrótowy"
"&Breve"
"&Stručný mód"

MEditPanelModesMedium
"&Средний режим"
"&Medium mode"
"S&třední mód"
"&Mittel"
"&Közepes mód"
"Ś&redni"
"&Medio"
"S&tredný mód"

MEditPanelModesFull
"&Полный режим"
"&Full mode"
"&Plný mód"
"&Voll"
"&Teljes mód"
"&Pełny"
"&Completo"
"Ú&plný mód"

MEditPanelModesWide
"&Широкий режим"
"&Wide mode"
"Š&iroký mód"
"B&reitformat"
"&Széles mód"
"S&zeroki"
"&Amplio"
"Š&iroký mód"

MEditPanelModesDetailed
"&Детальный режим"
"Detai&led mode"
"Detai&lní mód"
"Detai&lliert"
"Rés&zletes mód"
"Ze sz&czegółami"
"De&tallado"
"Podro&bný mód"

MEditPanelModesDiz
"&Описания"
"&Descriptions mode"
"P&opiskový mód"
"&Beschreibungen"
"&Fájlmegjegyzés mód"
"&Opisy"
"&Descripción"
"S p&opismi"

MEditPanelModesLongDiz
"Д&линные описания"
"Lon&g descriptions mode"
"&Mód dlouhých popisků"
"Lan&ge Beschreibungen"
"&Hosszú megjegyzés mód"
"&Długie opisy"
"Descripción lar&ga"
"S dlhý&mi popismi"

MEditPanelModesOwners
"Вл&адельцы файлов"
"File own&ers mode"
"Mód vlastníka so&uborů"
"B&esitzer"
"T&ulajdonos mód"
"&Właściciele"
"Du&eños de archivos"
"Mód vlastníkov s&úborov"

MEditPanelModesLinks
"Свя&зи файлов"
"Lin&ks mode"
"Lin&kový mód"
"Dateilin&ks"
"Li&nkek mód"
"Dowiąza&nia"
"En&laces"
"Mód pr&epojení"

MEditPanelModesAlternative
"Аль&тернативный полный режим"
"&Alternative full mode"
"&Alternativní plný mód"
"&Alternative Vollansicht"
"&Alternatív teljes mód"
"&Alternatywny"
"Alternativo com&pleto"
"&Alternatívny úplný mód"

MEditPanelModeTypes
l:
"&Типы колонок"
"Column &types"
"&Typ sloupců"
"Spalten&typen"
"Oszlop&típusok"
"&Typy kolumn"
"&Tipos de columna"
"&Typ stĺpcov"

MEditPanelModeWidths
"&Ширина колонок"
"Column &widths"
"Šíř&ka sloupců"
"Spalten&breiten"
"Oszlop&szélességek"
"&Szerokości kolumn"
"&Ancho de columna"
"Šír&ka stĺpcov"

MEditPanelModeStatusTypes
"Типы колонок строки ст&атуса"
"St&atus line column types"
"T&yp sloupců stavového řádku"
"St&atuszeile Spaltentypen"
"Állapotsor oszloptíp&usok"
"Typy kolumn &linii statusu"
"Tipos de columnas &línea de estado"
"T&yp stĺpcov stavového riadka"

MEditPanelModeStatusWidths
"Ширина колонок строки стат&уса"
"Status l&ine column widths"
"Šířka slo&upců stavového řádku"
"Statusze&ile Spaltenbreiten"
"Állapotsor &oszlopszélességek"
"Szerokości kolumn l&inii statusu"
"Ancho de columnas lí&nea de estado"
"Šírka stĺ&pcov stavového riadka"

MEditPanelModeFullscreen
"&Полноэкранный режим"
"&Fullscreen view"
"&Celoobrazovkový režim"
"&Vollbild"
"Tel&jes képernyős nézet"
"Widok &pełnoekranowy"
"&Vista pantalla completa"
"&Celoobrazovkový režim"

MEditPanelModeAlignExtensions
"&Выравнивать расширения файлов"
"Align file &extensions"
"Zarovnat příp&ony souborů"
"Datei&erweiterungen ausrichten"
"Fájlkiterjesztések &igazítása"
"W&yrównaj rozszerzenia plików"
"Alinear &extensiones de archivos"
"Zarovnať príp&ony súborov"

MEditPanelModeAlignFolderExtensions
"Выравнивать расширения пап&ок"
"Align folder e&xtensions"
"Zarovnat přípony adre&sářů"
"Ordnerer&weiterungen ausrichten"
"Mappakiterjesztések i&gazítása"
"Wyrównaj rozszerzenia &folderów"
"Alinear e&xtensiones de directorios"
"Zarovnať prípony prieči&nkov"

MEditPanelModeFoldersUpperCase
"Показывать папки &заглавными буквами"
"Show folders in &uppercase"
"Zobrazit adresáře &velkými písmeny"
"Ordner in Großb&uchstaben zeigen"
"Mappák NAG&YBETŰVEL mutatva"
"Nazwy katalogów &WIELKIMI LITERAMI"
"&Directorios en mayúsculas"
"Zobraziť priečinky &VEĽKÝMI písmenami"

MEditPanelModeFilesLowerCase
"Показывать файлы ст&рочными буквами"
"Show files in &lowercase"
"Zobrazit soubory ma&lými písmeny"
"Dateien in K&leinbuchstaben zeigen"
"Fájlok kis&betűvel mutatva"
"&Nazwy plików małymi literami"
"Archivos en m&inúsculas"
"Zobraziť súbory ma&lými písmenami"

MEditPanelModeUpperToLowerCase
"Показывать имена файлов из заглавных букв &строчными буквами"
"Show uppercase file names in lower&case"
"Zobrazit velké znaky ve jménech souborů jako &malá písmena"
"G&roßgeschriebene Dateinamen in Kleinbuchstaben zeigen"
"NAGYBETŰS fájl&nevek kisbetűvel"
"Wyświetl NAZWY_PLIKÓW &jako nazwy_plików"
"Archivos en ma&yúsculas mostrarlos con minúsculas"
"Zobraziť veľké znaky v názvoch súborov ako &malé písmená"

MEditPanelReadHelp
" Нажмите F1, чтобы получить информацию по настройке "
" Read online help for instructions "
" Pro instrukce si přečtěte online nápovědu "
" Siehe Hilfe für Anweisungen "
" Tanácsokat a súgóban talál (F1) "
" Instrukcje zawarte są w pomocy podręcznej "
" Para instrucciones leer ayuda en línea "
" Pokyny nájdete v online Pomocníkovi "

MSetFolderInfoTitle
l:
"Файлы информации о папках"
"Folder description files"
"Soubory s popiskem adresáře"
"Ordnerbeschreibungen"
"Mappa megjegyzésfájlok"
"Pliki opisu katalogu"
"Descripciones de directorio"
"Súbory s popisom priečinka"

MSetFolderInfoNames
"Введите имена файлов, разделённые запятыми (допускаются маски)"
"Enter file names delimited with commas (wildcards are allowed)"
"Zadejte jména souborů oddělených čárkami (značky jsou povoleny)"
"Dateiliste, getrennt mit Komma (Jokerzeichen möglich):"
"Fájlnevek, vesszővel elválasztva (joker is használható)"
"Nazwy plików oddzielone przecinkami (znaki ? i * dopuszczalne)"
"Ingrese nombre de archivo delimitado con comas (comodines permitidos)"
"Zadajte názvy súborov oddelené čiarkami (* je povolené)"

MScreensTitle
l:
"Экраны"
"Screens"
"Obrazovky"
"Seiten"
"Képernyők"
"Ekrany"
"Pantallas"
"Okná"

MScreensPanels
"Панели"
"Panels"
"Panely"
"Panels"
"Panelek"
"Panele"
"Paneles"
"Panely"

MScreensView
"Просмотр"
"View"
"Zobrazit"
"Betr."
"Nézőke"
"Podgląd"
"Ver"
"Zobraziť"

MScreensEdit
"Редактор"
"Edit"
"Editovat"
"Bearb"
"Szerkesztő"
"Edycja"
"Editar"
"Upraviť"

MAskApplyCommandTitle
l:
"Применить команду"
"Apply command"
"Aplikovat příkaz"
"Befehl anwenden"
"Parancs végrehajtása"
"Zastosuj polecenie"
"Aplicar comando"
"Použiť príkaz"

MAskApplyCommand
"Введите команду для обработки выбранных файлов"
"Enter command to process selected files"
"Zadejte příkaz pro zpracování vybraných souborů"
"Befehlszeile auf ausgewählte Dateien anwenden:"
"Írja be a kijelölt fájlok parancsát:"
"Wprowadź polecenie do przetworzenia wybranych plików"
"Ingrese comando para procesar archivos seleccionados"
"Zadajte príkaz na spracovanie vybraných súborov"

MPluginConfigTitle
l:
"Конфигурация плагинов"
"Plugins configuration"
"Nastavení Pluginů"
"Konfiguration von Plugins"
"Plugin beállítások"
"Konfiguracja pluginów"
"Configuración de complementos"
"Nastavenie modulov"

MPluginCommandsMenuTitle
"Команды плагинов"
"Plugin commands"
"Příkazy pluginů"
"Pluginbefehle"
"Plugin parancsok"
"Dostępne pluginy"
"Comandos de complemento"
"Príkazy modulov"

MPreparingList
l:
"Создание списка файлов"
"Preparing files list"
"Připravuji seznam souborů"
"Dateiliste wird vorbereitet"
"Fájllista elkészítése"
"Przygotowuję listę plików"
"Preparando lista de archivos"
"Pripravujem zoznam súborov"

MLangTitle
l:
"Основной язык"
"Main language"
"Hlavní jazyk"
"Hauptsprache"
"A program nyelve"
"Język programu"
"Idioma principal"
"Hlavný jazyk"

MHelpLangTitle
"Язык помощи"
"Help language"
"Jazyk nápovědy"
"Sprache der Hilfedatei"
"A súgó nyelve"
"Język pomocy"
"Idioma de ayuda"
"Jazyk Pomocníka"

MDefineMacroTitle
l:
"Задание макрокоманды"
"Define macro"
"Definovat makro"
"Definiere Makro"
"Makró gyorsbillentyű"
"Zdefiniuj makro"
"Definir macro"
"Definovať makro"

MDefineMacro
"Нажмите желаемую клавишу"
"Press the desired key"
"Stiskněte požadovanou klávesu"
"Tastenkombination:"
"Nyomja le a billentyűt"
"Naciśnij żądany klawisz"
"Pulse la tecla deseada"
"Stlačte požadovaný kláves"

MMacroReDefinedKey
"Макроклавиша '%1' уже определена."
"Macro key '%1' already defined."
"Klávesa makra '%1' již je definována."
"Makro '%1' bereits definiert."
""%1" makróbillentyű foglalt"
"Skrót '%1' jest już zdefiniowany."
"Macro '%1' ya está definido."
"Kláves makra '%1' je už definovaný."

MMacroDeleteAssign
"Макроклавиша '%1' не активна."
"Macro key '%1' is not active."
"Klávesa makra '%1' není aktivní."
"Makro '%1' nicht aktiv."
""%1" makróbillentyű nem él"
"Skrót '%1' jest nieaktywny."
"Macro '%1' no está activo."
"Kláves makra '%1' nie je aktívny."

MMacroDeleteKey
"Макроклавиша '%1' будет удалена."
"Macro key '%1' will be removed."
"Klávesa makra '%1' bude odstraněna."
"Makro '%1' wird entfernt und ersetzt:"
""%1" makróbillentyű törlődik"
"Skrót '%1' zostanie usunięty."
"Macro '%1' será removido."
"Kláves makra '%1' bude odstránený."

MMacroCommonReDefinedKey
"Общая макроклавиша '%1' уже определена."
"Common macro key '%1' already defined."
"Klávesa pro běžné makro '%1' již je definována."
"Gemeinsames Makro '%1' bereits definiert."
""%1" közös makróbill. foglalt"
"Skrót '%1' jest już zdefiniowany."
"Tecla de macro '%1' ya ha sido definida."
"Kláves pre bežné makro '%1' je už definovaný."

MMacroCommonDeleteAssign
"Общая макроклавиша '%1' не активна."
"Common macro key '%1' is not active."
"Klávesa pro běžné makro '%1' není aktivní."
"Gemeinsames Makro '%1' nicht aktiv."
""%1" közös makróbill. nem él"
"Skrót '%1' jest nieaktywny."
"Tecla de macro '%1' no está activada."
"Kláves pre bežné makro '%1' nie je aktívny."

MMacroCommonDeleteKey
"Общая макроклавиша '%1' будет удалена."
"Common macro key '%1' will be removed."
"Klávesa pro běžné makro '%1' bude odstraněna."
"Gemeinsames Makro '%1' wird entfernt und ersetzt:"
""%1" közös makróbill. törlődik"
"Skrót '%1' zostanie usunięty."
"Tecla de macro '%1' será removida."
"Kláves pre bežné makro '%1' bude odstránený."

MMacroSequence
"Последовательность:"
"Sequence:"
"Posloupnost:"
"Sequenz:"
"Szekvencia:"
"Sekwencja:"
"Secuencia:"
"Postupnosť:"

MMacroDescription
"Описание:"
"Description:"
"Popis:"
"Beschreibung:"
"Megjegyzés:"
"Opis:"
"Descripción:"
"Popis:"

MMacroReDefinedKey2
"Переопределить?"
"Redefine?"
"Předefinovat?"
"Neu definieren?"
"Újradefiniálja?"
"Zdefiniować powtórnie?"
"Redefinir?"
"Predefinovať?"

MMacroDeleteKey2
"Удалить?"
"Delete?"
"Odstranit?"
"Löschen?"
"Törli?"
"Usunąć?"
"Borrar?"
"Odstrániť?"

MMacroDisDisabledKey
"(макроклавиша не активна)"
"(macro key is not active)"
"(klávesa makra není aktivní)"
"(Makro inaktiv)"
"(makróbill. nem él)"
"(skrót jest nieaktywny)"
"(macro no está activo)"
"(kláves makra nie je aktívny)"

MMacroDisOverwrite
"Переопределить"
"Overwrite"
"Přepsat"
"Überschreiben"
"Felülírás"
"Zastąpić"
"Sobrescribir"
"Prepísať"

MMacroDisAnotherKey
"Изменить клавишу"
"Try another key"
"Zkusit jinou klávesu"
"Neue Kombination"
"Adjon meg másik billentyűt"
"Spróbuj inny klawisz"
"Intente otra tecla"
"Skúsiť iný kláves"

MMacroEditKey
"Изменить"
"Change"
upd:"Change"
upd:"Change"
upd:"Change"
upd:"Change"
"Cambiar?"
"Zmeniť"

MMacroSettingsTitle
l:
"Параметры макрокоманды для '%1'"
"Macro settings for '%1'"
"Nastavení makra pro '%1'"
"Einstellungen für Makro '%1'"
""%1" makró beállításai"
"Ustawienia makra dla '%1'"
"Configurar macro para '%1'"
"Nastavenia makra pre '%1'"

MMacroSettingsEnableOutput
"Разрешить во время &выполнения вывод на экран"
"Allo&w screen output while executing macro"
"Povolit &výstup na obrazovku dokud se provádí makro"
"Bildschirmausgabe &während Makro abläuft"
"Képernyő&kimenet a makró futása közben"
"&Wyłącz zapis na ekran podczas wykonywania makra"
"Permitir salida pantalla mientras se ejecut&an los macros"
"Povoliť &výstup na obrazovku, kým sa vykonáva makro"

MMacroSettingsRunAfterStart
"В&ыполнять после запуска FAR"
"Execute after FAR &start"
"&Spustit po spuštění FARu"
"Ausführen beim &Starten von FAR"
"Végrehajtás a FAR &indítása után"
"Wykonaj po &starcie FAR-a"
"Ejecutar luego de &iniciar FAR"
"&Spustiť po spustení FARu"

MMacroSettingsActivePanel
"&Активная панель"
"&Active panel"
"&Aktivní panel"
"&Aktives Panel"
"&Aktív panel"
"Panel &aktywny"
"Panel &activo"
"&Aktívny panel"

MMacroSettingsPassivePanel
"&Пассивная панель"
"&Passive panel"
"Pa&sivní panel"
"&Passives Panel"
"Passzí&v panel"
"Panel &pasywny"
"Panel &pasivo"
"Pa&sívny panel"

MMacroSettingsPluginPanel
"На панели пла&гина"
"P&lugin panel"
"Panel p&luginů"
"P&lugin Panel"
"Ha &plugin panel"
"Panel p&luginów"
"Panel de comp&lementos"
"Panel modu&lov"

MMacroSettingsFolders
"Выполнять для папо&к"
"Execute for &folders"
"Spustit pro ad&resáře"
"Auf Ordnern aus&führen"
"Ha &mappa"
"Wykonaj dla &folderów"
"Ejecutar para &directorios"
"Spustiť pre p&riečinky"

MMacroSettingsSelectionPresent
"&Отмечены файлы"
"Se&lection present"
"E&xistující výběr"
"Auswah&l vorhanden"
"Ha van ki&jelölés"
"Zaznaczenie &obecne"
"Selección presente"
"E&xistujúci výber"

MMacroSettingsCommandLine
"Пустая командная &строка"
"Empty &command line"
"Prázdný pří&kazový řádek"
"Leere Befehls&zeile"
"Ha &üres a parancssor"
"Pusta &linia poleceń"
"Vaciar línea de &comandos"
"Prázdny prí&kazový riadok"

MMacroSettingsSelectionBlockPresent
"Отмечен б&лок"
"Selection &block present"
"Existující blok výběr&u"
"Mar&kierter Text vorhanden"
"Ha van kijelölt &blokk"
"Obecny &blok zaznaczenia"
"Selección de bloque presente"
"Existujúci blok výber&u"

MMacroPErrorTitle
"Ошибка при разборе макроса"
"Error parsing macro"
upd:"Error parsing macro"
upd:"Error parsing macro"
upd:"Error parsing macro"
upd:"Error parsing macro"
"Error analizando macro"
"Error parsing macro"

MMacroPErrorPosition
"Строка %1, позиция %2"
"Line %1, Pos %2"
upd:"Line %1, Pos %2"
upd:"Line %1, Pos %2"
upd:"Line %1, Pos %2"
upd:"Line %1, Pos %2"
"Línea %1, Pos %2"
"Riadok %1, Pol. %2"

MMacroPErrUnrecognized_keyword
l:
"Неизвестное ключевое слово '%1'"
"Unrecognized keyword '%1'"
"Neznámé klíčové slovo '%1'"
"Unbekanntes Schlüsselwort '%1'"
"Ismeretlen kulcsszó "%1""
"Nie rozpoznano słowa kluczowego '%1'"
"Unrecognized keyword '%1'"
"Neznáme kľúčové slovo '%1'"

MMacroPErrUnrecognized_function
"Неизвестная функция '%1'"
"Unrecognized function '%1'"
"Neznámá funkce '%1'"
"Unbekannte Funktion '%1'"
"Ismeretlen funkció "%1""
"Nie rozpoznano funkcji'%1'"
"Unrecognized function '%1'"
"Neznáma funkcia '%1'"

MMacroPErrFuncParam
"Неверное количество параметров у функции '%1'"
"Incorrect number of arguments for function '%1'"
upd:"Incorrect number of arguments for function '%1'"
upd:"Incorrect number of arguments for function '%1'"
"'%1' funkció paramétereinek száma helytelen"
upd:"Incorrect number of arguments for function '%1'"
"Incorrect number of arguments for function '%1'"
"Nesprávny počet argumentov pre funkciu '%1'"

MMacroPErrNot_expected_ELSE
"Неожиданное появление $Else"
"Unexpected $Else"
"Neočekávané $Else"
"Unerwartetes $Else"
"Váratlan $Else"
"$Else w niewłaściwym miejscu"
"Unexpected $Else"
"Neočakávané $Else"

MMacroPErrNot_expected_END
"Неожиданное появление $End"
"Unexpected $End"
"Neočekávané $End"
"Unerwartetes $End"
"Váratlan $End"
"$End w niewłaściwym miejscu"
"Unexpected $End"
"Neočakávané $End"

MMacroPErrUnexpected_EOS
"Неожиданный конец строки"
"Unexpected end of source string"
"Neočekávaný konec zdrojového řetězce"
"Unerwartetes Ende der Zeichenkette"
"Váratlanul vége a forrássztringnek"
"Nie spodziewano się końca ciągu"
"Unexpected end of source string"
"Neočakávaný koniec zdrojového reťazca"

MMacroPErrExpected
"Ожидается '%1'"
"Expected '%1'"
"Očekávané '%1'"
"Erwartet '%1'"
"Várható "%1""
"Oczekiwano '%1'"
"Expected '%1'"
"Očakávané '%1'"

MMacroPErrBad_Hex_Control_Char
"Неизвестный шестнадцатеричный управляющий символ"
"Bad Hex Control Char"
"Chybný kontrolní znak Hex"
"Fehlerhaftes Hexzeichen"
"Rossz hexa vezérlőkarakter"
"Błędny szesnastkowy znak sterujący"
"Bad Hex Control Char"
"Chybný kontrolný znak Hex"

MMacroPErrBad_Control_Char
"Неправильный управляющий символ"
"Bad Control Char"
"Špatný kontrolní znak"
"Fehlerhaftes Kontrollzeichen"
"Rossz vezérlőkarakter"
"Błędny znak sterujący"
"Bad Control Char"
"Chybný kontrolný znak"

MMacroPErrVar_Expected
"Переменная '%1' не найдена"
"Variable Expected '%1'"
"Očekávaná proměnná '%1'"
"Variable erwartet '%1'"
""%1" várható változó"
"Oczekiwano zmiennej '%1'"
"Variable Expected '%1'"
"Očakávaná premenná '%1'"

MMacroPErrExpr_Expected
"Ошибка синтаксиса"
"Expression Expected"
"Očekávaný výraz"
"Ausdruck erwartet"
"Szintaktikai hiba"
"Oczekiwano wyrażenia"
"Expression Expected"
"Očakávaný výraz"

MMacroPErr_ZeroLengthMacro
"Пустая макропоследовательность"
"Zero-length macro"
upd:"Zero-length macro"
upd:"Zero-length macro"
"Nulla hosszúságú makró"
upd:"Zero-length macro"
"macro de longitud 0"
"Makro s nulovou dĺžkou"

MMacroPErrIntParserError
"Внутренняя ошибка парсера"
"Internal parser error"
upd:"Internal parser error"
upd:"Internal parser error"
upd:"Internal parser error"
upd:"Internal parser error"
"Macro parsing error"
"Internal parser error"

MMacroPErrContinueOutsideTheLoop
"Оператор $Continue вне цикла"
upd:"$Continue outside the loop"
upd:"$Continue outside the loop"
upd:"$Continue outside the loop"
upd:"$Continue outside the loop"
upd:"$Continue outside the loop"
"$Continuar por fuera del loop"
"$Pokračovať mimo slučky"

MCannotSaveFile
l:
"Ошибка сохранения файла"
"Cannot save file"
"Nelze uložit soubor"
"Kann Datei nicht speichern"
"A fájl nem menthető"
"Nie mogę zapisać pliku"
"No se puede guardar archivo"
"Nemôžem uložiť súbor"

MTextSavedToTemp
"Отредактированный текст записан в"
"Edited text is stored in"
"Editovaný text je uložen v"
"Editierter Text ist gespeichert in"
"A szerkesztett szöveg elmentve:"
"Edytowany tekst został zachowany w"
"Texto editado es almacenado en"
"Upravený text je uložený v"

MMonthJan
l:
"Янв"
"Jan"
"Led"
"Jan"
"Jan"
"Sty"
"Ene"
"jan"

MMonthFeb
"Фев"
"Feb"
"Úno"
"Feb"
"Feb"
"Lut"
"Feb"
"feb"

MMonthMar
"Мар"
"Mar"
"Bře"
"Mär"
"Már"
"Mar"
"Mar"
"mar"

MMonthApr
"Апр"
"Apr"
"Dub"
"Apr"
"Ápr"
"Kwi"
"Abr"
"apr"

MMonthMay
"Май"
"May"
"Kvě"
"Mai"
"Máj"
"Maj"
"May"
"máj"

MMonthJun
"Июн"
"Jun"
"Čer"
"Jun"
"Jún"
"Cze"
"Jun"
"ján"

MMonthJul
"Июл"
"Jul"
"Čec"
"Jul"
"Júl"
"Lip"
"Jul"
"júl"

MMonthAug
"Авг"
"Aug"
"Srp"
"Aug"
"Aug"
"Sie"
"Ago"
"aug"

MMonthSep
"Сен"
"Sep"
"Zář"
"Sep"
"Sze"
"Wrz"
"Sep"
"sep"

MMonthOct
"Окт"
"Oct"
"Říj"
"Okt"
"Okt"
"Paź"
"Oct"
"okt"

MMonthNov
"Ноя"
"Nov"
"Lis"
"Nov"
"Nov"
"Lis"
"Nov"
"nov"

MMonthDec
"Дек"
"Dec"
"Pro"
"Dez"
"Dec"
"Gru"
"Dic"
"dec"

MPluginHotKeyTitle
l:
"Назначение горячей клавиши"
"Assign plugin hot key"
"Přidělit horkou klávesu pluginu"
"Dem Plugin eine Kurztaste zuweisen"
"Plugin gyorsbillentyű hozzárendelés"
"Przypisz klawisz skrótu do pluginu"
"Asignar tecla de atajo a complemento"
"Prideliť skratkový kláves modulu"

MPluginHotKey
"Введите горячую клавишу (букву или цифру)"
"Enter hot key (letter or digit)"
"Zadejte horkou klávesu (písmeno nebo číslici)"
"Buchstabe oder Ziffer:"
"Nyomja le a billentyűt (betű vagy szám)"
"Podaj klawisz skrótu (litera lub cyfra)"
"Entrar tecla rápida (letra o dígito)"
"Zadajte skratkový kláves (písmeno alebo číslicu)"

MPluginInformation
"Информация о плагине"
"Plugin information"
upd:"Plugin information"
upd:"Plugin information"
upd:"Plugin information"
upd:"Plugin information"
"Información de complemento"
"Informácie o module"

MPluginModuleTitle
"&Название:"
"&Title:"
upd:"&Title:"
upd:"&Title:"
upd:"&Title:"
upd:"&Title:"
"&Título:"
"&Názov:"

MPluginDescription
"&Описание:"
"&Description:"
upd:"&Description:"
upd:"&Description:"
upd:"&Description:"
upd:"&Description:"
"&Descripción:"
"&Opis:"

MPluginAuthor
"&Автор:"
"&Author:"
upd:"&Author:"
upd:"&Author:"
upd:"&Author:"
upd:"&Author:"
"&Autor:"
"&Autor:"

MPluginVersion
"&Версия:"
"&Version:"
upd:"&Version:"
upd:"&Version:"
upd:"&Version:"
upd:"&Version:"
"&Versión:"
"&Verzia:"

MPluginModulePath
"&Файл плагина:"
"&Module path:"
upd:"&Module path:"
upd:"&Module path:"
upd:"&Module path:"
upd:"&Module path:"
"Ruta de &Modulo:"
"Cesta k &modulu:"

MPluginGUID
"&GUID плагина:"
"Plugin &GUID:"
upd:"Plugin &GUID:"
upd:"Plugin &GUID:"
upd:"Plugin &GUID:"
upd:"Plugin &GUID:"
"&GUID complemento:"
"&GUID modulu:"

MPluginItemGUID
"GU&ID пункта:"
"Plugin &item GUID:"
upd:"Plugin &item GUID:"
upd:"Plugin &item GUID:"
upd:"Plugin &item GUID:"
upd:"Plugin &item GUID:"
"&Item GUID complemento:"
"GUID položky m&odulu:"

MPluginPrefix
"&Префикс плагина:"
"Plugin &prefix:"
upd:"Plugin &prefix:"
upd:"Plugin &prefix:"
upd:"Plugin &prefix:"
upd:"Plugin &prefix:"
"&Prefijo de complemento:"
"Predpona mo&dulu:"

MPluginHotKeyBottom
"F4 - задать горячую клавишу, F3 - информация"
"F4 - set hot key, F3 - information"
upd:"F4 - nastavení horké klávesy, F3 - information"
upd:"Kurztaste setzen: F4, information: F3"
upd:"F4 - gyorsbillentyű hozzárendelés, F3 - information"
upd:"F4 - ustaw klawisz skrótu, F3 - information"
"F4 - tecla de atajo, F3 - información"
"F4 - nastavenie skratkového klávesu, F3 - informácia"


MRightCtrl
l:
"ПравыйCtrl"
"RightCtrl"
"PravýCtrl"
"StrgRechts"
"JobbCtrl"
"PrawyCtrl"
"CtrlDrcho"
"PravýCtrl"

MViewerGoTo
l:
"Перейти"
"Go to"
"Jdi na"
"Gehe zu"
"Ugrás"
"Idź do"
"Ir a:"
"Ísť na"

MGoToPercent
"&Процент"
"&Percent"
"&Procent"
"&Prozent"
"&Százalékban"
"&Procent"
"&Porcentaje"
"&Percent"

MGoToHex
"16-ричное &смещение"
"&Hex offset"
"&Hex offset"
"Position (&Hex)"
"&Hexában"
"Pozycja (&szesnastkowo)"
"Dirección &Hexa"
"&Hex offset"

MGoToDecimal
"10-ичное с&мещение"
"&Decimal offset"
"&Desítkový offset"
"Position (&dezimal)"
"&Decimálisan"
"Pozycja (&dziesiętnie)"
"Dirección &Decimal"
"&Desiatkový offset"

MExcTrappedException
"Исключительная ситуация"
"Exception occurred"
"Vyskytla se výjimka"
"Ausnahmefehler aufgetreten"
"Kivétel történt"
"Wystąpił wyjątek"
"Error de excepción"
"Vyskytla sa výnimka"

MExcRAccess
"Нарушение доступа (чтение из %1)"
"Access violation (read from %1)"
"Neplatná adresa (čtení z %1)"
"Zugriffsverletzung (Lesen von %1)"
"Hozzáférési jogsértés (olvasás %1 címről)"
"Błąd dostępu (odczyt z %1)"
"Violación de acceso (leído desde %1)"
"Neplatná adresa (čítanie z %1)"

MExcWAccess
"Нарушение доступа (запись в %1)"
"Access violation (write to %1)"
"Neplatná adresa (zápis na %1)"
"Zugriffsverletzung (Schreiben nach %1)"
"Hozzáférési jogsértés (írás %1 címre)"
"Błąd dostępu (zapis do %1)"
"Violación de acceso (escrito a %1)"
"Neplatná adresa (zápis na %1)"

MExcEAccess
"Нарушение доступа (исполнение кода из %1)"
"Access violation (execute at %1)"
"Neplatná adresa (spuštění na %1)"
"Zugriffsverletzung (Ausführen bei %1)"
"Hozzáférési jogsértés (végrehajtás %1 címen)"
"Błąd dostępu (wykonanie w %1)"
"Violación de acceso (ejecutado en %1)"
"Neplatná adresa (spustenie na %1)"

MExcOutOfBounds
"Попытка доступа к элементу за границами массива"
"Array out of bounds"
"Pole mimo hranice"
"Arrayüberlauf"
"A tömb határait meghaladta"
"Przekroczenie granic tabeli"
"Array out of bounds"
"Pole mimo hranice"

MExcDivideByZero
"Деление на нуль"
"Divide by zero"
"Dělení nulou"
"Division durch Null"
"Nullával osztás"
"Dzielenie przez zero"
"División por cero"
"Delenie nulou"

MExcStackOverflow
"Переполнение стека"
"Stack Overflow"
"Přetečení zásobníku"
"Stacküberlauf"
"Verem túlcsordulás"
"Przepełnienie stosu"
"Stack overflow"
"Pretečenie zásobníka"

MExcBreakPoint
"Точка останова"
"Breakpoint exception"
"Výjimka přerušení"
"Breakpoint exception"
"Törésponti kivétel"
"Wyjątek punktu przerwania"
"Excepción de punto de quiebre"
"Výnimka prerušenia"

MExcFloatDivideByZero
"Деление на нуль при операции с плавающей точкой"
"Floating-point divide by zero"
"Dělení nulou v pohyblivé čárce"
"Fließkomma-Division durch Null"
"Lebegőpontos szám osztása nullával"
"Błąd zmiennoprzecinkowego dzielenia przez zero"
"Punto flotante dividido por cero"
"Delenie nulou v pohyblivej čiarke"

MExcFloatOverflow
"Переполнение при операции с плавающей точкой"
"Floating point operation overflow"
"Přetečení při operaci v pohyblivé čárce"
"Fließkomma-Operation verursachte Überlauf"
"Lebegőpontos művelet túlcsordulás"
"Przepełnienie przy operacji zmiennnoprzecinkowej"
"Operación de punto flotante desbordada"
"Pretečenie pri operácii s pohyblivou čiarkou"

MExcFloatStackOverflow
"Стек регистров сопроцессора полон или пуст"
"Floating point stack empty or full"
"Prázdný nebo plný zásobník v pohyblivé čárce"
"Fließkomma-Stack leer bzw. voll"
"Lebegőpont verem üres vagy megtelt"
"Stos operacji zmiennoprzecinkowych pusty lub pełny"
"Pila de punto flotante vacía o llena"
"Prázdny alebo plný zásobník v pohyblivej čiarke"

MExcFloatUnderflow
"Потеря точности при операции с плавающей точкой"
"Floating point operation underflow"
"Podtečení při operaci v pohyblivé čárce"
"Fließkomma-Operation verursachte Underflow"
"Lebegőpontos művelet alulcsordulás"
"Błąd niedomiaru przy operacji zmiennoprzecinkowej"
"Operación de punto flotante underflow"
"Podtečenie pri operácii s pohyblivou čiarkou"

MExcBadInstruction
"Недопустимая инструкция"
"Illegal instruction"
"Neplatná instrukce"
"Ungültige Anweisung"
"Érvénytelen utasítás"
"Błędna instrukcja"
"Instrucción ilegal"
"Neplatná inštrukcia"

MExcDatatypeMisalignment
"Попытка доступа к невыравненным данным"
"Alignment fault"
"Chyba zarovnání"
"Fehler bei Datenausrichtung"
"Adattípus illesztési hiba"
"Błąd ustawienia"
"Falta de alineamiento"
"Chyba zarovnania"

MExcUnknown
"Неизвестное исключение"
"Unknown exception"
"Neznámá výjimka"
"Unbekannte Ausnahme"
"Ismeretlen kivétel"
"Nieznany wyjątek"
"Excepción desconocida"
"Neznáma výnimka"

MExcException
"Исключение:"
"Exception:"
upd:"Exception:"
upd:"Exception:"
upd:"Exception:"
upd:"Exception:"
"Excepción:"
"Výnimka:"

MExcAddress
"Адрес:"
"Address:"
upd:"Address:"
upd:"Address:"
upd:"Address:"
upd:"Address:"
"Dirección:"
"Adresa:"

MExcFunction
"Функция:"
"Function:"
upd:"Function:"
upd:"Function:"
upd:"Function:"
upd:"Function:"
"Función:"
"Funkcia:"

MExcModule
"Модуль:"
"Module:"
upd:"Module:"
upd:"Module:"
upd:"Module:"
upd:"Module:"
"Módulo:"
"Modul:"

MExcTerminate
"Завершить Far"
"Terminate Far"
upd:"Terminate Far"
upd:"Terminate Far"
upd:"Terminate Far"
upd:"Terminate Far"
"Far se dará por terminado"
"Ukončiť FAR"

MExcUnload
"Выгрузить плагин"
"Unload plugin"
upd:"Unload plugin"
upd:"Unload plugin"
upd:"Unload plugin "
upd:"Unload plugin"
"El complemento será descargado"
"Unload plugin"

MExcDebugger
"Отладка"
"Debug"
upd:"Debug"
upd:"Debug"
upd:"Debug"
upd:"Debug"
"Depurador"
"Debug"

MNetUserName
l:
"Имя пользователя"
"User name"
"Jméno uživatele"
"Benutzername"
"Felhasználói név"
"Nazwa użytkownika"
"Nombre de usuario"
"Meno užívateľa"

MNetUserPassword
"Пароль пользователя"
"User password"
"Heslo uživatele"
"Benutzerpasswort"
"Felhasználói jelszó"
"Hasło użytkownika"
"Clave de usuario"
"Heslo užívateľa"

MReadFolderError
l:
"Не удаётся прочесть содержимое папки"
"Cannot read folder contents"
"Nelze načíst obsah adresáře"
"Kann Ordnerinhalt nicht lesen"
"A mappa tartalma nem olvasható"
"Nie udało się odczytać zawartości folderu"
"No se puede leer contenidos de directorios"
"Nemôžem načítať obsah priečinka"

MPlgBadVers
l:
"Для работы этого плагина требуется более новая версия Far"
"This plugin requires more recent version of Far"
"Tento plugin vyžaduje vyšší verzi FARu"
"Das Plugin benötigt eine aktuellere Version von FAR"
"A pluginhez újabb FAR verzió kell"
"Do uruchomienia pluginu wymagana jest wyższa wersja FAR-a"
"Este complemento requiere versión más actual de FAR"
"Tento modul si vyžaduje vyššiu verziu FARu"

MPlgRequired
"Требуемая версия Far - %1,"
"Required Far version is %1,"
"Požadovaná verze Faru je %1,"
"Benötigte Far-Version ist %1,"
"A szükséges Far verzió: %1,"
"Wymagana wersja Far-a to %1,"
"Requiere la versión Far %1,"
"Požadovaná verzia FARu je %1,"

MPlgRequired2
"текущая версия Far - %1."
"current Far version is %1."
"nynější verze FARu je %1."
"aktuelle Far-Version ist %1."
"a jelenlegi Far verzió: %1."
"bieżąca wersja Far-a: %1."
"versión actual de Far es %1."
"terajšia verzia FARu je %1."

MPlgLoadPluginError
"Ошибка при загрузке плагина"
"Error loading plugin"
upd:"Chyba při nahrávání zásuvného modulu"
upd:"Fehler beim Laden des Pluginmoduls"
upd:"Plugin betöltési hiba"
upd:"Błąd ładowania modułu plugina"
"Error cargando módulo complemento"
"Chyba pri nahrávaní zásuvného modulu"

MCheckBox2State
l:
"?"
"?"
"?"
"?"
"?"
"?"
"?"
"?"

MHelpTitle
l:
"Помощь"
"Help"
"Nápověda"
"Hilfe"
"Súgó"
"Pomoc"
"Ayuda"
"Pomocník"

MHelpActivatorURL
"Эта ссылка запускает внешнее приложение:"
"This reference starts the external application:"
"Tento odkaz spouští externí aplikaci:"
"Diese Referenz startet folgendes externes Programm:"
"A hivatkozás által indított program:"
"To wywołanie uruchomi aplikację zewnętrzną:"
"Esta referencia inicia la aplicación externa:"
"Tento odkaz spúšťa externú aplikáciu:"

MHelpActivatorFormat
"с параметром:"
"with parameter:"
"s parametrem:"
"mit Parameter:"
"Paraméterei:"
"z parametrem:"
"con parámetro:"
"s parametrom:"

MHelpActivatorQ
"Желаете запустить?"
"Do you wish to start it?"
"Přejete si ji spustit?"
"Wollen Sie jetzt starten?"
"El akarja indítani?"
"Czy chcesz ją uruchomić?"
"Desea comenzar la aplicación?"
"Chcete ju spustiť?"

MCannotOpenHelp
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"
"A fájl nem nyitható meg"
"Nie można otworzyć pliku"
"No se puede abrir el archivo"
"Nemôžem otvoriť súbor"

MHelpTopicNotFound
"Не найден запрошенный раздел помощи:"
"Requested help topic not found:"
"požadované téma nápovědy nebylo nalezeno"
"Angefordertes Hilfethema wurde nicht gefunden:"
"A kívánt súgó témakör nem található:"
"Nie znaleziono tematu pomocy:"
"Tema de ayuda requerido no encontrado:"
"Požadovaná téma Pomocníka nenájdená:"

MPluginsHelpTitle
l:
"Плагины"
"Plugins help"
"Nápověda Pluginů"
"Pluginhilfe"
"Pluginek súgói"
"Pomoc dla pluginów"
"Ayuda de complementos"
"Pomocník k modulom"

MDocumentsHelpTitle
"Документы"
"Documents help"
"Nápověda Dokumentů"
"Dokumentenhilfe"
"Dokumentumok súgói"
"Pomoc dla dokumentów"
"Ayuda documentos"
"Pomocník k dokumentom"

MHelpSearchTitle
l:
"Поиск"
"Search"
"Hledání"
"Suchen"
"Keresés"
"Szukaj"
"Buscar"
"Hľadanie"

MHelpSearchingFor
"Поиск для"
"Searching for"
"Hledání"
"Suche nach"
"Keresés:"
"Znajdź"
"Buscando por"
"Hľadám"

MHelpSearchCannotFind
"Строка не найдена"
"Could not find the string"
"Nelze najít řetězec"
"Konnte Zeichenkette nicht finden"
"A szöveg nem található:"
"Nie mogę odnaleźć ciągu znaków"
"No se encontró la cadena"
"Nemôžem nájsť reťazec"

MHelpF1
l:
l:// Help KeyBar F1-12
"Помощь"
"Help"
"Pomoc"
"Hilfe"
"Súgó"
"Pomoc"
"Ayuda"
"Pomocník"

MHelpF2
""
""
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
""
""

MHelpF4
""
""
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
"Nagyít"
"Powiększ"
"Zoom"
"Zoom"

MHelpF6
""
""
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
"Buscar"
"Hľadať"

MHelpF8
""
""
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
""
""

MHelpF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"
"Koniec"
"Salir"
"Koniec"

MHelpF11
""
""
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
"Indice"
"Index"

MHelpShiftF2
"Плагин"
"Plugin"
"Plugin"
"Plugin"
"PlgSúg"
"Plugin"
"Comple"
"Modul"

MHelpShiftF3
"Докум"
"Docums"
"Dokume"
"Dokume"
"DokSúg"
"Dokumenty"
"Docums"
"Dokume"

MHelpShiftF4
""
""
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
""
""

MHelpShiftF6
""
""
""
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
"Nast."
"Próxim"
"Nasl."

MHelpShiftF8
""
""
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
""
""

MHelpShiftF10
""
""
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
""
""

MHelpShiftF12
""
""
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
"Předch"
"Letzt"
"Vissza"
"Poprz."
"Previo"
"Predch"

MHelpAltF2
""
""
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
""
""

MHelpAltF4
""
""
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
""
""

MHelpAltF6
""
""
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
""
""

MHelpAltF8
""
""
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
"Video"
"Video"

MHelpAltF10
""
""
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
""
""

MHelpAltF12
""
""
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
""
""

MHelpCtrlF2
""
""
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
""
""

MHelpCtrlF4
""
""
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
""
""

MHelpCtrlF6
""
""
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
""
""

MHelpCtrlF8
""
""
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
""
""

MHelpCtrlF10
""
""
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
""
""

MHelpCtrlF12
""
""
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
""
""

MHelpCtrlShiftF2
""
""
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
""
""

MHelpCtrlShiftF4
""
""
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
""
""

MHelpCtrlShiftF6
""
""
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
""
""

MHelpCtrlShiftF8
""
""
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
""
""

MHelpCtrlShiftF10
""
""
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
""
""

MHelpCtrlShiftF12
""
""
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
""
""

MHelpCtrlAltF2
""
""
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
""
""

MHelpCtrlAltF4
""
""
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
""
""

MHelpCtrlAltF6
""
""
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
""
""

MHelpCtrlAltF8
""
""
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
""
""

MHelpCtrlAltF10
""
""
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
""
""

MHelpCtrlAltF12
""
""
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
""
""

MHelpAltShiftF2
""
""
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
""
""

MHelpAltShiftF4
""
""
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
""
""

MHelpAltShiftF6
""
""
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
""
""

MHelpAltShiftF8
""
""
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
""
""

MHelpAltShiftF10
""
""
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
""
""

MHelpAltShiftF12
""
""
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
""
""

MHelpCtrlAltShiftF2
""
""
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
""
""

MHelpCtrlAltShiftF4
""
""
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
""
""

MHelpCtrlAltShiftF6
""
""
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
""
""

MHelpCtrlAltShiftF8
""
""
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
""
""

MHelpCtrlAltShiftF10
""
""
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
""
""

MHelpCtrlAltShiftF12
""
""
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
"Súgó"
"Pomoc"
"Ayuda"
"Pomoc"

MInfoF2
"Сверн"
"Wrap"
"Zalam"
"Umbr."
"SorTör"
"Zawiń"
"Divide"
"Zalam"

MInfoF3
"СмОпис"
"VieDiz"
"Zobraz"
"BetDiz"
"MjMnéz"
"VieDiz"
"VerDiz"
"Zobraz"

MInfoF4
"РедОпи"
"EdtDiz"
"Edit"
"BeaDiz"
"MjSzrk"
"EdtDiz"
"EdtDiz"
"Uprav."

MInfoF5
""
""
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
""
""

MInfoF7
"Поиск"
"Search"
"Hledat"
"Suchen"
"Keres"
"Search"
"Buscar"
"Hľadať"

MInfoF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"WIN"

MInfoF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"
"KonfMn"
"ConfMn"
"BarMnú"
"KonfMn"

MInfoF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"
"Koniec"
"Quitar"
"Koniec"

MInfoF11
"Плагины"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Comple"
"Modul"

MInfoF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"
"Ekran"
"Pantal"
"Okno"

MInfoShiftF1
l:
l:// InfoPanel KeyBar Shift-F1-F12
""
""
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
"SzóTör"
"ZawijS"
"ConDiv"
"ZalSlo"

MInfoShiftF3
""
""
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
""
""

MInfoShiftF5
""
""
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
""
""

MInfoShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"
"Nast."
"Próxim"
"Nasl."

MInfoShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"
"StrKod"
"PágCód"
"TabZn"

MInfoShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"
"Zapisz"
"Guarda"
"Uložiť"

MInfoShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"
"UtsMnü"
"Ostat."
"Ultimo"
"Posled"

MInfoShiftF11
""
""
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
"Lewy"
"Izqda"
"Ľavý"

MInfoAltF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"
"Prawy"
"Drcha"
"Pravý"

MInfoAltF3
""
""
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
""
""

MInfoAltF5
""
""
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
""
""

MInfoAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"
"Znajdź"
"Encont"
"Hľadať"

MInfoAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"
"IdźDo"
"Ir a.."
"Ísť na"

MInfoAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"
"Video"
"Video"

MInfoAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"
"Drzewo"
"Arbol"
"Strom"

MInfoAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"
"Historia"
"VerHis"
"HsZobr"

MInfoAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"
"MapElő"
"FoldHs"
"HisDir"
"HsPrie"

MInfoCtrlF1
l:
l:// InfoPanel KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"
"Links"
"Bal"
"Lewy"
"Izqda"
"Ľavý"

MInfoCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"
"Prawy"
"Drcha"
"Pravý"

MInfoCtrlF3
""
""
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
""
""

MInfoCtrlF5
""
""
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
""
""

MInfoCtrlF7
""
""
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
""
""

MInfoCtrlF9
""
""
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
""
""

MInfoCtrlF11
""
""
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
""
""

MInfoCtrlShiftF2
""
""
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
""
""

MInfoCtrlShiftF4
""
""
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
""
""

MInfoCtrlShiftF6
""
""
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
""
""

MInfoCtrlShiftF8
""
""
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
""
""

MInfoCtrlShiftF10
""
""
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
""
""

MInfoCtrlShiftF12
""
""
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
""
""

MInfoCtrlAltF2
""
""
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
""
""

MInfoCtrlAltF4
""
""
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
""
""

MInfoCtrlAltF6
""
""
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
""
""

MInfoCtrlAltF8
""
""
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
""
""

MInfoCtrlAltF10
""
""
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
""
""

MInfoCtrlAltF12
""
""
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
""
""

MInfoAltShiftF2
""
""
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
""
""

MInfoAltShiftF4
""
""
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
""
""

MInfoAltShiftF6
""
""
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
""
""

MInfoAltShiftF8
""
""
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
""
""

MInfoAltShiftF10
""
""
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
""
""

MInfoAltShiftF12
""
""
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
""
""

MInfoCtrlAltShiftF2
""
""
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
""
""

MInfoCtrlAltShiftF4
""
""
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
""
""

MInfoCtrlAltShiftF6
""
""
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
""
""

MInfoCtrlAltShiftF8
""
""
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
""
""

MInfoCtrlAltShiftF10
""
""
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
""
""

MInfoCtrlAltShiftF12
""
""
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
"Súgó"
"Pomoc"
"Ayuda"
"Pomoc"

MQViewF2
"Сверн"
"Wrap"
"Zalam"
"Umbr."
"SorTör"
"Zawiń"
"Divide"
"Zalam"

MQViewF3
"Просм"
"View"
"Zobraz"
"Betr."
"Megnéz"
"Zobacz"
"Ver"
"Zobraz"

MQViewF4
"Код"
"Hex"
"Hex"
"Hex"
"Hexa"
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
""
""

MQViewF6
""
""
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
"Buscar"
"Hľadať"

MQViewF8
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"ANSI"
"WIN"

MQViewF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"
"KonfMn"
"ConfMn"
"BarMnú"
"KonfMn"

MQViewF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"
"Koniec"
"Quitar"
"Koniec"

MQViewF11
"Плагины"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Comple"
"Modul"

MQViewF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"
"Ekran"
"Pantal"
"Okno"

MQViewShiftF1
l:
l:// QView KeyBar Shift-F1-F12
""
""
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
"SzóTör"
"WWrap"
"ConDiv"
"ZalSlo"

MQViewShiftF3
""
""
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
""
""

MQViewShiftF5
""
""
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
""
""

MQViewShiftF7
"Дальше"
"Next"
"Další"
"Nächst"
"TovKer"
"Nast."
"Próxim"
"Nasl."

MQViewShiftF8
"КодСтр"
"CodePg"
upd:"ZnSady"
upd:"Tabell"
"Kódlap"
"StrKod"
"PágCód"
"TabZn"

MQViewShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"
"Zapisz"
"Guarda"
"Uložiť"

MQViewShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"
"UtsMnü"
"Ostat."
"Ultimo"
"Posled"

MQViewShiftF11
""
""
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
"Lewy"
"Izqda"
"Ľavý"

MQViewAltF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"
"Prawy"
"Drcha"
"Pravý"

MQViewAltF3
""
""
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
""
""

MQViewAltF5
""
""
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
""
""

MQViewAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"
"Znajdź"
"Encont"
"Hľadať"

MQViewAltF8
"Строка"
"Goto"
"Jít na"
"GeheZu"
"Ugrás"
"IdźDo"
"Ir a."
"Ísť na"

MQViewAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"
"Video"
"Video"

MQViewAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"
"Drzewo"
"Arbol"
"Strom"

MQViewAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"
"Historia"
"VerHis"
"HsZobr"

MQViewAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"
"MapElő"
"FoldHs"
"HisDir"
"HsPrieč"

MQViewCtrlF1
l:
l:// QView KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"
"Links"
"Bal"
"Lewy"
"Izqda"
"Ľavý"

MQViewCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"
"Prawy"
"Drcha"
"Pravý"

MQViewCtrlF3
""
""
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
""
""

MQViewCtrlF5
""
""
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
""
""

MQViewCtrlF7
""
""
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
""
""

MQViewCtrlF9
""
""
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
""
""

MQViewCtrlF11
""
""
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
""
""

MQViewCtrlShiftF2
""
""
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
""
""

MQViewCtrlShiftF4
""
""
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
""
""

MQViewCtrlShiftF6
""
""
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
""
""

MQViewCtrlShiftF8
""
""
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
""
""

MQViewCtrlShiftF10
""
""
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
""
""

MQViewCtrlShiftF12
""
""
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
""
""

MQViewCtrlAltF2
""
""
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
""
""

MQViewCtrlAltF4
""
""
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
""
""

MQViewCtrlAltF6
""
""
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
""
""

MQViewCtrlAltF8
""
""
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
""
""

MQViewCtrlAltF10
""
""
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
""
""

MQViewCtrlAltF12
""
""
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
""
""

MQViewAltShiftF2
""
""
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
""
""

MQViewAltShiftF4
""
""
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
""
""

MQViewAltShiftF6
""
""
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
""
""

MQViewAltShiftF8
""
""
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
""
""

MQViewAltShiftF10
""
""
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
""
""

MQViewAltShiftF12
""
""
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
""
""

MQViewCtrlAltShiftF2
""
""
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
""
""

MQViewCtrlAltShiftF4
""
""
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
""
""

MQViewCtrlAltShiftF6
""
""
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
""
""

MQViewCtrlAltShiftF8
""
""
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
""
""

MQViewCtrlAltShiftF10
""
""
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
""
""

MQViewCtrlAltShiftF12
""
""
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
"Súgó"
"Pomoc"
"Ayuda"
"Pomoc"

MKBTreeF2
"ПользМ"
"UserMn"
"UživMn"
"BenuMn"
"FelhMn"
"UserMn"
"Menú"
"UžívMn"

MKBTreeF3
""
""
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
"Atrib"
"Atr"

MKBTreeF5
"Копир"
"Copy"
"Kopír."
"Kopier"
"Másol"
"Kopiuj"
"Copiar"
"Kopír."

MKBTreeF6
"Перен"
"RenMov"
"PřjPřs"
"RenMov"
"ÁtnMoz"
"Zamień"
"RenMov"
"PrmPrs"

MKBTreeF7
"Папка"
"MkFold"
"VytAdr"
"VerzEr"
"ÚjMapp"
"NowyFldr"
"CrDIR "
"VytPri"

MKBTreeF8
"Удален"
"Delete"
"Smazat"
"Lösch"
"Törlés"
"Usuń"
"Borrar"
"Zmazať"

MKBTreeF9
"КонфМн"
"ConfMn"
"KonfMn"
"KonfMn"
"KonfMn"
"KonfMenu"
"BarMnú"
"KonfMn"

MKBTreeF10
"Выход"
"Quit"
"Konec"
"Ende"
"Kilép"
"Koniec"
"Quitar"
"Koniec"

MKBTreeF11
"Плагины"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Plugin"
"Comple"
"Modul"

MKBTreeF12
"Экраны"
"Screen"
"Obraz."
"Seiten"
"Képrny"
"Ekran"
"Pantal"
"Okno"

MKBTreeShiftF1
l:
l:// Tree KeyBar Shift-F1-F12
""
""
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
""
""

MKBTreeShiftF3
""
""
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
""
""

MKBTreeShiftF5
"Копир"
"Copy"
"Kopír."
"Kopier"
"Másol"
"Kopiuj"
"Copiar"
"Kopír."

MKBTreeShiftF6
"Перен"
"Rename"
"Přejm."
"Umben"
"ÁtnMoz"
"Zamień"
"RenMov"
"Premen"

MKBTreeShiftF7
""
""
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
""
""

MKBTreeShiftF9
"Сохран"
"Save"
"Uložit"
"Speich"
"Mentés"
"Zapisz"
"Guarda"
"Uložiť"

MKBTreeShiftF10
"Послдн"
"Last"
"Posled"
"Letzt"
"UtsMnü"
"Ostat."
"Ultimo"
"Posled"

MKBTreeShiftF11
"Группы"
"Group"
"Skupin"
"Gruppe"
"Csoprt"
"Grupa"
"Grupo"
"Skupin"

MKBTreeShiftF12
"Выбран"
"SelUp"
"VybPrv"
"AuswOb"
"KijFel"
"SelUp"
"SelUp"
"VybPrv"

MKBTreeAltF1
l:
l:// Tree KeyBar Alt-F1-F12
"Левая"
"Left"
"Levý"
"Links"
"Bal"
"Lewy"
"Izqda"
"Ľavý"

MKBTreeAltF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"
"Prawy"
"Drcha"
"Pravý"

MKBTreeAltF3
""
""
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
""
""

MKBTreeAltF5
""
""
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
""
""

MKBTreeAltF7
"Искать"
"Find"
"Hledat"
"Suchen"
"Keres"
"Znajdź"
"Encont"
"Hľadať"

MKBTreeAltF8
"Истор"
"Histry"
"Histor"
"Histor"
"ParElő"
"Historia"
"Histor"
"Histór"

MKBTreeAltF9
"Видео"
"Video"
"Video"
"Ansich"
"Video"
"Video"
"Video"
"Video"

MKBTreeAltF10
"Дерево"
"Tree"
"Strom"
"Baum"
"MapKer"
"Drzewo"
"Arbol"
"Strom"

MKBTreeAltF11
"ИстПр"
"ViewHs"
"ProhHs"
"BetrHs"
"NézElő"
"Historia"
"VerHis"
"HsZobr"

MKBTreeAltF12
"ИстПап"
"FoldHs"
"AdrsHs"
"OrdnHs"
"MapElő"
"FoldHs"
"HisDir"
"HsPrieč"

MKBTreeCtrlF1
l:
l:// Tree KeyBar Ctrl-F1-F12
"Левая"
"Left"
"Levý"
"Links"
"Bal"
"Lewy"
"Izqda"
"Ľavý"

MKBTreeCtrlF2
"Правая"
"Right"
"Pravý"
"Rechts"
"Jobb"
"Prawy"
"Drcha"
"Pravý"

MKBTreeCtrlF3
""
""
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
""
""

MKBTreeCtrlF5
""
""
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
""
""

MKBTreeCtrlF7
""
""
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
""
""

MKBTreeCtrlF9
""
""
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
""
""

MKBTreeCtrlF11
""
""
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
""
""

MKBTreeCtrlShiftF2
""
""
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
""
""

MKBTreeCtrlShiftF4
""
""
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
""
""

MKBTreeCtrlShiftF6
""
""
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
""
""

MKBTreeCtrlShiftF8
""
""
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
""
""

MKBTreeCtrlShiftF10
""
""
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
""
""

MKBTreeCtrlShiftF12
""
""
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
""
""

MKBTreeCtrlAltF2
""
""
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
""
""

MKBTreeCtrlAltF4
""
""
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
""
""

MKBTreeCtrlAltF6
""
""
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
""
""

MKBTreeCtrlAltF8
""
""
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
""
""

MKBTreeCtrlAltF10
""
""
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
""
""

MKBTreeCtrlAltF12
""
""
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
""
""

MKBTreeAltShiftF2
""
""
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
""
""

MKBTreeAltShiftF4
""
""
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
""
""

MKBTreeAltShiftF6
""
""
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
""
""

MKBTreeAltShiftF8
""
""
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
""
""

MKBTreeAltShiftF10
""
""
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
""
""

MKBTreeAltShiftF12
""
""
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
""
""

MKBTreeCtrlAltShiftF2
""
""
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
""
""

MKBTreeCtrlAltShiftF4
""
""
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
""
""

MKBTreeCtrlAltShiftF6
""
""
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
""
""

MKBTreeCtrlAltShiftF8
""
""
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
""
""

MKBTreeCtrlAltShiftF10
""
""
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
""
""

MKBTreeCtrlAltShiftF12
""
""
""
""
""
""
""
""

MCopyTimeInfo
l:
"Время: %1    Осталось: %2    %3Б/с"
"Time: %1    Remaining: %2    %3B/s"
"Čas: %1      Zbývá: %2      %3B/s"
"Zeit: %1   Verbleibend: %2   %3B/s"
"Eltelt: %1    Maradt: %2    %3B/s"
"Czas: %1    Pozostało: %2    %3B/s"
"Tiempo: %1    Restante: %2    %3B/s"
"Čas: %1      Zostáva: %2      %3B/s"

MKeyESCWasPressed
l:
"Действие было прервано"
"Operation has been interrupted"
"Operace byla přerušena"
"Vorgang wurde unterbrochen"
"A műveletet megszakította"
"Operacja została przerwana"
"Operación ha sido interrumpida"
"Operácia bola prerušená"

MDoYouWantToStopWork
"Вы действительно хотите отменить действие?"
"Do you really want to cancel it?"
"Opravdu chcete operaci stornovat?"
"Wollen Sie den Vorgang wirklich abbrechen?"
"Valóban le akarja állítani?"
"Czy naprawdę chcesz ją anulować?"
"Desea realmente cancelar la operación?"
"Chcete operáciu zrušiť?"

MDoYouWantToStopWork2
"Продолжить выполнение?"
"Continue work? "
"Pokračovat v práci?"
"Vorgang fortsetzen? "
"Folytatja?"
"Kontynuować? "
"Continuar trabajo? "
"Pokračovať v práci?"

MCheckingFileInPlugin
l:
"Файл проверяется в плагине"
"The file is being checked by the plugin"
"Soubor je právě kontrolován pluginem"
"Datei wird von Plugin überprüft"
"A fájlt ez a plugin használja:"
"Plugin sprawdza plik"
"El archivo está siendo chequeado por el complemento"
"Súbor je práve kontrolovaný modulom"

MDialogType
l:
"Диалог"
"Dialog"
"Dialog"
"Dialog"
"Párbeszéd"
"Dialog"
"Diálogo"
"Dialóg"

MHelpType
"Помощь"
"Help"
"Nápověda"
"Hilfe"
"Súgó"
"Pomoc"
"Ayuda"
"Pomocník"

MFolderTreeType
"ПоискКаталогов"
"FolderTree"
"StromAdresáře"
"Ordnerbaum"
"MappaFa"
"Drzewo folderów"
"ArbolDirectorio"
"StromPriečinkov"

MVMenuType
"Меню"
"Menu"
"Menu"
"Menü"
"Menü"
"Menu"
"Menú"
"Menu"

MIncorrectMask
l:
"Некорректная маска файлов"
"File-mask string contains errors"
"Řetězec masky souboru obsahuje chyby"
"Zeichenkette mit Dateimaske enthält Fehler"
"A fájlmaszk hibás"
"Maska pliku zawiera błędy"
"Cadena de máscara de archivos contiene errores"
"Reťazec masky súboru obsahuje chyby"

MPanelBracketsForLongName
l:
"{}"
"{}"
"{}"
"{}"
"{}"
"{}"
"{}"
"{}"

MComspecNotFound
l:
"Переменная окружения %COMSPEC% не определена"
"Environment variable %COMSPEC% not defined"
"Proměnná prostředí %COMSPEC% není definována"
"Umgebungsvariable %COMSPEC% nicht definiert"
"A %COMSPEC% környezeti változó nincs definiálva"
"Nie zdefiniowano zmiennej środowiskowej %COMSPEC%"
"Variable de entorno %COMSPEC% no definida"
"Premenná prostredia %COMSPEC% nie je definovaná"

MOpenPluginCannotOpenFile
l:
"Ошибка открытия файла"
"Cannot open the file"
"Nelze otevřít soubor"
"Kann Datei nicht öffnen"
"A fájl nem nyitható meg"
"Nie można otworzyć pliku"
"No se puede abrir el archivo"
"Nemôžem otvoriť súbor"

MFileFilterTitle
l:
"Фильтр"
"Filter"
"Filtr"
"Filter"
"Felhasználói szűrő"
"Filtr wyszukiwania"
"Filtro"
"Filter"

MFileHilightTitle
"Раскраска файлов"
"Files highlighting"
"Zvýrazňování souborů"
"Farbmarkierungen"
"Fájlkiemelés"
"Zaznaczanie plików"
"Resaltado de archivos"
"Zvýrazňovanie súborov"

MFileFilterName
"Имя &фильтра:"
"Filter &name:"
"Jmé&no filtru:"
"Filter&name:"
"Szűrő &neve:"
"Nazwa &filtra:"
"&Nombre filtro:"
"&Názov filtra:"

MFileFilterMatchMask
"&Маска:"
"&Mask:"
"&Maska"
"&Maske:"
"&Maszk:"
"&Maska:"
"&Máscara:"
"&Maska"

MFileFilterSize
"Разм&ер:"
"Si&ze:"
"Vel&ikost"
"G&röße:"
"M&éret:"
"Ro&zmiar:"
"&Tamaño:"
"Ve&ľkosť"

MFileFilterSizeFromSign
">="
">="
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
"<="
"<="

MFileHardLinksCount
"Болee одной жёсткой ссылки"
"Has more than one hardlink"
upd:"Has more than one hardlink"
upd:"Has more than one hardlink"
upd:"Has more than one hardlink"
upd:"Has more than one hardlink"
"Tiene mas de un enlace rígido"
"Má viac než jedno pevné prepojenie"

MFileFilterDate
"&Дата/Время:"
"Da&te/Time:"
"Dat&um/Čas:"
"Da&tum/Zeit:"
"&Dátum/Idő:"
"Da&ta/Czas:"
"&Fecha/Hora:"
"Dát&um/Čas:"

MFileFilterWrited
"&записи"
upd:"&write"
upd:"&write"
upd:"&write"
upd:"&write"
upd:"&write"
"&modificación"
"&zápisu"

MFileFilterCreated
"&создания"
"&creation"
"&vytvoření"
"E&rstellung"
"&Létrehozás"
"&utworzenia"
"&creación"
"&vytvorenia"

MFileFilterOpened
"&доступа"
"&access"
"&přístupu"
"Z&ugriff"
"&Hozzáférés"
"&dostępu"
"&acceso"
"&prístupu"

MFileFilterChanged
"&изменения"
"c&hange"
upd:"c&hange"
upd:"c&hange"
upd:"c&hange"
upd:"c&hange"
"c&ambio"
"z&meny"

MFileFilterDateRelative
"Относительна&я"
"Relat&ive"
"Relati&vní"
"Relat&iv"
"Relat&ív"
"Relat&ive"
"Relat&ivo"
"Relatí&vny"

MFileFilterDateAfterSign
">="
">="
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
"<="
"<="

MFileFilterCurrent
"Теку&щая"
"C&urrent"
"Aktuá&lní"
"Akt&uell"
"&Jelenlegi"
"&Bieżący"
"Act&ual"
"Aktuá&lny"

MFileFilterBlank
"С&брос"
"B&lank"
"Práz&dný"
"&Leer"
"&Üres"
"&Wyczyść"
"En b&lanco"
"Práz&dny"

MFileFilterAttr
"Атрибут&ы"
"Attri&butes"
"Attri&buty"
"Attri&bute"
"Attri&bútumok"
"&Atrybuty"
"Atri&butos"
"Atri&búty"

MFileFilterAttrR
"&Только для чтения"
"&Read only"
"Jen pro čt&ení"
"Sch&reibschutz"
"&Csak olvasható"
"&Do odczytu"
"Sólo &Lectura"
"Len na čít&anie"

MFileFilterAttrA
"&Архивный"
"&Archive"
"Arc&hivovat"
"&Archiv"
"&Archív"
"&Archiwalny"
"&Archivo"
"Arc&hivovať"

MFileFilterAttrH
"&Скрытый"
"&Hidden"
"Skry&tý"
"&Versteckt"
"&Rejtett"
"&Ukryty"
"&Oculto"
"Skry&tý"

MFileFilterAttrS
"С&истемный"
"&System"
"Systémo&vý"
"&System"
"Re&ndszer"
"&Systemowy"
"&Sistema"
"Systémo&vý"

MFileFilterAttrC
"С&жатый"
"&Compressed"
"Kompri&movaný"
"&Komprimiert"
"&Tömörített"
"S&kompresowany"
"&Comprimido"
"Kompri&movaný"

MFileFilterAttrE
"&Зашифрованный"
"&Encrypted"
"Ši&frovaný"
"V&erschlüsselt"
"T&itkosított"
"&Zaszyfrowany"
"C&ifrado"
"Ši&frovaný"

MFileFilterAttrD
"&Каталог"
"&Directory"
"Adr&esář"
"Ver&zeichnis"
"Map&pa"
"&Katalog"
"&Directorio"
"Pri&ečinok"

MFileFilterAttrNI
"&Неиндексируемый"
"Not inde&xed"
"Neinde&xovaný"
"Nicht in&diziert"
"Nem inde&xelt"
"Nie z&indeksowany"
"No inde&xado"
"Neinde&xovaný"

MFileFilterAttrSparse
"&Разрежённый"
"S&parse"
"Říd&ký"
"Reserve"
"Ritk&ított"
"S&parse"
"Escas&o"
"Ried&ky"

MFileFilterAttrT
"&Временный"
"Temporar&y"
"Doča&sný"
"Temporär"
"Átm&eneti"
"&Tymczasowy"
"Tem&poral"
"Doča&sný"

MFileFilterAttrReparse
"Симво&л. ссылка"
"Symbolic lin&k"
"Sybolický li&nk"
"Symbolischer Lin&k"
"S&zimbolikus link"
"Link &symboliczny"
"&Enlace simbólico"
"Sybolické prepoje&nie"

MFileFilterAttrOffline
"Автономны&й"
"O&ffline"
"O&ffline"
"O&ffline"
"O&ffline"
"O&ffline"
"Desconectado"
"O&ffline"

MFileFilterAttrVirtual
"Вирт&уальный"
"&Virtual"
"Virtuální"
"&Virtuell"
"&Virtuális"
"&Wirtualny"
"&Virtual"
"Virtuálny"

MFileFilterReset
"Очистит&ь"
"Reset"
"Reset"
"Rücksetzen"
"Reset"
"Wy&czyść"
"Reinicio"
"Reset"

MFileFilterCancel
"Отмена"
"Cancel"
"Storno"
"Abbruch"
"Mégsem"
"&Anuluj"
"Cancelar"
"Storno"

MFileFilterMakeTransparent
"Выставить прозрачность"
"Make transparent"
"Zprůhlednit"
"Transparent"
"Legyen átlátszó"
"Ustaw jako przezroczysty"
"Hacer transparente"
"Spriehľadniť"

MBadFileSizeFormat
"Неправильно заполнено поле размера"
"File size field is incorrectly filled"
"Velikost souboru neobsahuje správnou hodnotu"
"Angabe der Dateigröße ist fehlerhaft"
"A fájlméret mező rosszul van kitöltve"
"Rozmiar pliku jest niepoprawny"
"Campo de tamaño de archivo no está correctamente llenado"
"Veľkosť súboru neobsahuje správnu hodnotu"

MFarTitleAddonsAdmin
l:
"Администратор"
"Administrator"
upd:"Administrator"
upd:"Administrator"
upd:"Administrator"
upd:"Administrator"
"Administrador"
"Správca"

MElevationRequired
"Нужно обладать правами администратора"
"You need to provide administrator permission"
upd:"You need to provide administrator permission"
upd:"You need to provide administrator permission"
upd:"You need to provide administrator permission"
upd:"You need to provide administrator permission"
"Usted necesita permisos de administrador"
"Musíte poskytnúť práva správcu"

MElevationRequiredPrivileges
"Требуются дополнительные привилегии"
"Additional privileges required"
upd:"Additional privileges required"
upd:"Additional privileges required"
upd:"Additional privileges required"
upd:"Additional privileges required"
"Privilegios adicionales requeridos"
"Ďalšie práva sa vyžadujú na"

MElevationRequiredProcess
"для обработки этого объекта:"
"to process this object:"
upd:"to process this object:"
upd:"to process this object:"
upd:"to process this object:"
upd:"to process this object:"
"para procesar este objeto:"
"spracovanie tohto objektu:"

MElevationRequiredCreate
"для создания этого объекта:"
"to create this object:"
upd:"to create this object:"
upd:"to create this object:"
upd:"to create this object:"
upd:"to create this object:"
"para crear este objeto:"
"vytvorenie tohto objektu:"

MElevationRequiredDelete
"для удаления этого объекта:"
"to delete this object:"
upd:"to delete this object:"
upd:"to delete this object:"
upd:"to delete this object:"
upd:"to delete this object:"
"para eliminar este objeto:"
"zmazanie tohto objektu:"

MElevationRequiredCopy
"для копирования этого объекта:"
"to copy this object:"
upd:"to copy this object:"
upd:"to copy this object:"
upd:"to copy this object:"
upd:"to copy this object:"
"para copiar este objeto:"
"kopírovanie tohto objektu:"

MElevationRequiredMove
"для перемещения этого объекта:"
"to move this object:"
upd:"to move this object:"
upd:"to move this object:"
upd:"to move this object:"
upd:"to move this object:"
"para mover este objeto:"
"presunutie tohto objektu:"

MElevationRequiredGetAttributes
"для получения атрибутов этого объекта:"
"to get attributes of this object:"
upd:"to get attributes of this object:"
upd:"to get attributes of this object:"
upd:"to get attributes of this object:"
upd:"to get attributes of this object:"
"para obtener atributos de este objeto:"
"získanie atribútov tohto objektu:"

MElevationRequiredSetAttributes
"для установки атрибутов этого объекта:"
"to set attributes of this object:"
upd:"to set attributes of this object:"
upd:"to set attributes of this object:"
upd:"to set attributes of this object:"
upd:"to set attributes of this object:"
"para poner atributos a este objeto:"
"nastavenie atribútov tohto objektu:"

MElevationRequiredHardLink
"для создания этой жёсткой ссылки:"
"to create this hard link:"
upd:"to create this hard link:"
upd:"to create this hard link:"
upd:"to create this hard link:"
upd:"to create this hard link:"
"para crear este enlace rígido:"
"vytvorenie tohto pevného prepojenia:"

MElevationRequiredSymLink
"для создания этой символической ссылки:"
"to create this symbolic link:"
upd:"to create this symbolic link:"
upd:"to create this symbolic link:"
upd:"to create this symbolic link:"
upd:"to create this symbolic link:"
"para crear este enlace simbólico:"
"vytvorenie tohto symbolického prepojenia:"

MElevationRequiredRecycle
"для перемещения этого объекта в корзину:"
"to move this object to recycle bin:"
upd:"to move this object to recycle bin:"
upd:"to move this object to recycle bin:"
upd:"to move this object to recycle bin:"
upd:"to move this object to recycle bin:"
"para mover este objeto a la papelera:"
"presunutie tohto objektu do Koša:"

MElevationRequiredList
"для просмотра этого объекта:"
"to list this object:"
upd:"to list this object:"
upd:"to list this object:"
upd:"to list this object:"
upd:"to list this object:"
"para listar este objeto:"
"zobrazenie tohto objektu:"

MElevationRequiredSetOwner
"для установки владельца этого объекта:"
"to set owner of this object:"
upd:"to set owner of this object:"
upd:"to set owner of this object:"
upd:"to set owner of this object:"
upd:"to set owner of this object:"
"para poner como dueño de este objeto:"
"nastavenie vlastníka tohto objektu:"

MElevationRequiredOpen
"для открытия этого объекта:"
"to open this object:"
upd:"to open this object:"
upd:"to open this object:"
upd:"to open this object:"
upd:"to open this object:"
"para abrir este objeto:"
"otvorenie tohto objektu:"

MElevationRequiredEncryptFile
"для шифрования этого объекта:"
"to encrypt this object:"
upd:"to encrypt this object:"
upd:"to encrypt this object:"
upd:"to encrypt this object:"
upd:"to encrypt this object:"
"para cifrar este objeto:"
"zašifrovanie tohto objektu:"

MElevationRequiredDecryptFile
"для расшифрования этого объекта:"
"to decrypt this object:"
upd:"to decrypt this object:"
upd:"to decrypt this object:"
upd:"to decrypt this object:"
upd:"to decrypt this object:"
"para descifrar este objeto:"
"dešifrovanie tohto objektu:"

MElevationDoForAll
"Выполнить это действие для &всех текущих объектов"
"Do this for &all current objects"
upd:"Do this for &all current objects"
upd:"Do this for &all current objects"
upd:"Do this for &all current objects"
upd:"Do this for &all current objects"
"Hacer esto para todos los objetos actuales"
"Urobiť to so všetkými &aktuálnymi objektmi"

MElevationDoNotAskAgainInTheCurrentSession
"Больше не спрашивать в текущей сессии"
"Do not ask again in the current session"
upd:"Do not ask again in the current session"
upd:"Do not ask again in the current session"
upd:"Do not ask again in the current session"
upd:"Do not ask again in the current session"
"No preguntar nuevamente en la sesión actual"
"Nepýtať sa znova v aktuálnej relácii"

MCompletionHistoryTitle
"История"
"History"
upd:"History"
upd:"History"
upd:"History"
upd:"History"
"Historial"
"História"

MCompletionFilesTitle
"Файлы"
"Files"
upd:"Files"
upd:"Files"
upd:"Files"
upd:"Files"
"Archivos"
"Súbory"

MObjectLockedReason
"Объект %1 в:"
"Object is being %1 in:"
upd:"Object is being %1 in:"
upd:"Object is being %1 in:"
upd:"Object is being %1 in:"
upd:"Object is being %1 in:"
"Objecto comienza %1 en:"
"Object is being %1 in:"

MObjectLockedReasonPlayed
"воспроизводится"
"played"
upd:"played"
upd:"played"
upd:"played"
upd:"played"
"reproducido"
"played"

MObjectLockedReasonEdited
"редактируется"
"edited"
upd:"edited"
upd:"edited"
upd:"edited"
upd:"edited"
"editado"
"edited"

MObjectLockedReasonOpened
"открыт"
"opened"
upd:"opened"
upd:"opened"
upd:"opened"
upd:"opened"
"abierto"
"opened"

MObjectLockedSwitchTo
"Переключиться"
"Switch to:"
upd:"Switch to"
upd:"Switch to"
upd:"Switch to"
upd:"Switch to"
"Cambiar a"
"Prepnúť na"

MObjectLockedClose
"Закрыть файл"
"Close file"
upd:"Close file"
upd:"Close file"
upd:"Close file"
upd:"Close file"
"Cerrar archivo"
"Zatvoriť súbor"

MProblemDb
"Проблемы с БД настроек"
"Problem configuration DB"
upd:"Problem configuration DB"
upd:"Problem configuration DB"
upd:"Problem configuration DB"
upd:"Problem configuration DB"
"Problema de configuración DB"
"Problém s konfiguráciou DB"

MShowConfigFolders
"Показать каталоги настроек"
"Show configuration folders"
upd:"Show configuration folders"
upd:"Show configuration folders"
upd:"Show configuration folders"
upd:"Show configuration folders"
"Mostrar carpetas de configuración"
"Zobraziť konfiguračné priečinky"

#Must be the last
MNewFileName
l:
"?Новый файл?"
"?New File?"
"?Nový soubor?"
"?Neue Datei?"
"?Új fájl?"
"?Nowy plik?"
"?Nuevo Archivo?"
"?Nový súbor?"
