m4_include(`farversion.m4')m4_dnl
.Language=Hungarian,Hungarian (Magyar)
.Options CtrlColorChar=\
.Options CtrlStartPosChar=^<wrap>

@Contents
$^#Fájl- és archívumkezelő program#
`$^#'FULLVERSIONNOBRACES`#'
$^#Copyright © 1996-2000 Eugene Roshal#
`$^#Copyright ©' COPYRIGHTYEARS `FAR Group#'
$^(help file last translated for build 882)
   ~A súgó betűrendes tartalomjegyzéke~@Index@
   ~A súgó használata~@Help@

   ~A programról~@About@
   ~FAR license~@License@

   ~A FAR parancssori kapcsolói~@CmdLine@
   ~Billentyűparancsok~@KeyRef@
   ~Pluginek támogatása~@Plugins@
   ~A pluginek képességeinek áttekintése~@PluginsReviews@

   ~Panelek:~@Panels@ ~Fájlpanel~@FilePanel@
            ~Fastruktúra panel~@TreePanel@
            ~Info panel~@InfoPanel@
            ~Gyorsnézet panel~@QViewPanel@
            ~Húzd és ejtsd művelet fájlokon~@DragAndDrop@
            ~A fájlpanel nézetek testreszabása~@PanelViewModes@
            ~Fájlok kijelölése~@SelectFiles@

   ~Menük:~@Menus@   ~Bal és Jobb menü~@LeftRightMenu@
            ~Fájlok menü~@FilesMenu@
            ~Parancsok menü~@CmdMenu@
            ~Beállítások menü~@OptMenu@

   ~Fájlkeresés~@FindFile@
   ~Parancs elözmények~@History@
   ~Mappakeresés~@FindFolder@
   ~Mappák összehasonlítása~@CompFolders@
   ~Felhasználói menü~@UserMenu@
   ~Meghajtóváltás (Meghajtók menü)~@DriveDlg@

   ~Fájltársítások~@FileAssoc@
   ~Operációs rendszer parancsok~@OSCommands@
   ~Mappa gyorsbillentyűk~@FolderShortcuts@
   ~Szűrők menü~@FiltersMenu@
   ~Képernyők váltása~@ScrSwitch@
   ~Futó programok~@TaskList@
   ~Hotplug eszközök~@HotPlugList@

   ~Rendszer beállítások~@SystemSettings@
   ~Panel beállítások~@PanelSettings@
   ~Fastruktúra beállítások~@TreeSettings@
   ~Kezelőfelület beállítások~@InterfSettings@
   ~Párbeszédablak beállítások~@DialogSettings@

   ~Fájlkiemelések, rendezési csoportok~@Highlight@
   ~Fájlmegjegyzések~@FileDiz@
   ~Nézőke beállítások~@ViewerSettings@
   ~Szerkesztő beállítások~@EditorSettings@

   ~Másolás, mozgatás, átnevezés és linkek létrehozása~@CopyFiles@

   ~Belső nézőke~@Viewer@
   ~Belső szerkesztő~@Editor@

   ~Fájlmaszkok~@FileMasks@
   ~Makrók~@KeyMacro@


@Help
$ # FAR: a súgó használata#
    A súgó oldalain hivatkozásokat találunk, amelyek további súgóoldalakra
mutatnak. Szintén a főoldalon található ~A súgó betűrendes tartalomjegyzéke~@Index@,
is, rajta az összes témakör listája, ami egyes esetekben megkönnyíti a
szükséges információk gyorsabb megtalálását.

    A #Tab# és a #Shift-Tab# billentyűk a kurzort egyik hivatkozásról a másikra
léptetik és az #Enter# vagy az egérgomb lenyomásával juthatunk el a
hivatkozott oldalra.

    Ha a szöveg túlnyúlik a súgó ablakán, függőleges gördítősáv jelenik meg,
ebben az esetben a #kurzorvezérlő billentyűkkel# görgethetjük a szöveget.

    Az #Alt-F1# vagy a #BackSpace# segítségével léphetünk visszafelé a bejárt
súgóoldalakon, #Shift-F1#-re pedig a súgó tartalma jelenik meg.

    A #Shift-F2# a ~pluginek~@Plugins@ súgóját hívja meg.

    A #súgó# alapértelmezés szerint csökkentett méretű ablakban jelenik meg,
amit az #F5# #(Nagyít)# funkcióbillentyűvel maximalizálhatunk. Az #F5# újbóli
lenyomása az előző méretre állítja vissza a súgó ablakát.


@About
$ # FAR: a programról#
    A #FAR# szövegmódú fájl- és tömörítettkezelő program Windows 2000,
XP, 2003, Vista és 2008 operációs rendszerhez. Kezeli a #hosszú fájlneveket#,
valamint a fájlokon és mappákon elvégezhető műveletek széles skáláját kínálja.

    A #FAR# #ingyenes# és #nyílt forráskódú# program, terjesztése
az átdolgozott BSD ~liszensz~@License@ szövegének megfelelően történik.

    A #FAR# teljesen átláthatóvá teszi a #tömörített fájlokat#. Ugyanolyan
egyszerűen kezelhetjük az állományokat, mintha mappákban lennének, mivel a
a FAR a parancsokat a tömörítőprogramok számára értelmezhető külső hívásokká
alakítja át.

    A #FAR# számos szolgáltatással is rendelkezik.


@License
$ # FAR: License#

 Copyright © 1996 Eugene Roshal
 Copyright © 2000 Far Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
 1. ^<wrap>Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 3. The name of the authors may not be used to endorse or promote products
derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


@CmdLine
$ # FAR: a FAR parancssori kapcsolói#
  A FAR a következő parancssori kapcsolókkal indítható:

  #/a#    ^<wrap>Letiltja a 0-31-ig terjedő és a 255-ös ASCII kódú
karakterek megjelenítését. Hasznos lehet, ha a FAR-t telnet alól használjuk.

  #/ag#   Letiltja a pszeudografikus karakterek megjelenítését.
        ^<wrap>
  #/e[<sor>[:<pozíció>]] <fájlnév>#  A megadott fájlt szerkesztésre
nyitja meg. A /e után megadható, hogy melyik sor hányadik karakterhelyére
álljon a kurzor.

          Például: far /e70:2 readme.

  #/p[<path>]#  A "fő" plugineket a <path>-ben megadott elérési út
          ^<wrap>mappáiban keresi. Több keresési útvonal is megadható,
";"-vel elválasztva.

          Példa: #far /p%USERPROFILE%\\Far\\Plugins#

  #/co#   ^<wrap>A FAR pluginjei csak a gyorsítótárból töltődhetnek be.
Bár így a betöltésük gyorsabb, az új vagy megváltozott plugineket a FAR nem
érzékeli. CSAK állandó pluginek esetén használjuk! Pluginek hozzáadása,
cseréje vagy törlése után a FAR-t a kapcsoló nélkül kell elindítani. Ha a
gyorsítótár üres, nem töltődik be plugin.

          Megjegyzések a /p és /co kapcsolókhoz:

        - ^<wrap>ha a /p után nincs érték, a FAR pluginek nélkül
fog elindulni;
        - ha /p-nek adtunk <path> értéket, csak a megadott útvonalról
töltődnek be pluginek;
        - ha csak a /co kapcsolóval indítjuk és a plugin gyorsítótár
nem üres, a gyorsítótárból töltődnek be a pluginek;
        - a /co kapcsolót a FAR figyelmen kívül hagyja, ha /p is áll
mellette;
        - ha sem a /p, sem a /co kapcsoló nem szerepel a parancssorban,
akkor a pluginek csak az alapértelmezett plugin mappából, valamint a
~saját pluginek elérési útvonala~@SystemSettings@ által meghatározott
mappákból töltődnek be.

  #/m#    ^<wrap>A FAR induláskor nem tölti be a registryből a makróit.

  #/ma#   ^<wrap>A "Futtatás a FAR indítása után" opciójú makrók nem
indulnak el a FAR-ral.
          ^<wrap>
  #/u <felhasználónév>#  Lehetővé teszi, hogy a felhasználóknak saját
FAR beállításaik legyenek.

          Például: far /u guest

        ^<wrap>A FAR Manager a "FARUSER" ~környezeti változónak~@FAREnv@
a <felhasználónév> értéket adja.
        ^<wrap>
  #/v <fájlnév>#  Megnézi a megadott fájlt. Ha a <fájlnév> #-#, akkor az
stdin adatát olvassa ki.

        ^<wrap>Például a "dir|far /v -" a dir parancs kimenetét fogja
kiolvasni.

        ^<wrap>Ha a bemenő adatfolyam üres, amikor a fájlnév "-" (ha
az előző példánál maradva a "dir" parancsot elhagyjuk), akkor a FAR a
végtelenségig fog várakozni a bemenő adatfolyam végére. Ezt a hibát a FAR
egy későbbi verziójában a szerzők valószínűleg ki fogják javítani.

  #/w#
  Stretch to console window instead of console buffer.

  #/x#    ^<wrap>Letiltja a kivételek kezelését. Ezt a lehetőséget
a szerzők a pluginek fejlesztői részére tervezték, ezért nem ajánlott normál
használat közben alkalmazni.

    A parancssorban megadható legfeljebb két olyan elérési út, ami mappára,
fájlra vagy tömörített fájlra mutat. Az első elérési út az aktív, a második a
passzív panelre vonatkozik:

  - ^<wrap>ha az elérési út mappára vagy tömörített fájlra mutat, a FAR
megjeleníti a tartalmát;

  - ^<wrap>ha az elérési út fájlra mutat, a FAR belép a mappába, ahol
a fájl található és ráállítja a kurzort (ha a fájl létezik).


@KeyRef
$ #Billentyűparancsok#

 ~Panelvezérlő parancsok~@PanelCmd@

 ~Parancssor~@CmdLineCmd@

 ~Fájlkezelő és rendszerparancsok~@FuncCmd@

 ~Egér: görgő támogatása~@MsWheel@

 ~Egyebek~@MiscCmd@


@PanelCmd
$ #Panelvezérlő parancsok#
  #Általános panelparancsok#

  Aktív panel váltó                                              #Tab#
  Panelek megcserélése                                        #Ctrl-U#
  Panel frissítése                                            #Ctrl-R#
  Info panel be/ki                                            #Ctrl-L#
  ~Gyorsnézet~@QViewPanel@ panel be/ki                                      #Ctrl-Q#
  Fastruktúra panel be/ki                                     #Ctrl-T#
  Mindkét panelt elrejti/megmutatja                           #Ctrl-O#
  Átmenetileg elrejti mindkét panelt
    (amíg a billentyűk le vannak nyomva)              #Ctrl-Alt-Shift#
  Elrejti/megmutatja az inaktív panelt                        #Ctrl-P#
  Elrejti/megmutatja a bal panelt                            #Ctrl-F1#
  Elrejti/megmutatja a jobb panelt                           #Ctrl-F2#
  A panelek magasságán változtat                    #Ctrl-Fel,Ctrl-Le#
  A panelek szélességén változtat
    (ha a parancssor üres)                        #Ctrl-Jobb,Ctrl-Bal#
  Visszaállítja a panelszélességek alapértékét          #Ctrl-Numpad5#
  Visszaállítja a panelmagasságok alapértékét       #Ctrl-Alt-Numpad5#
  Megmutatja/elrejti a funkcióbillentyűk sorát
    a képernyő alján                                          #Ctrl-B#

  #Fájlpanel parancsok#

  Fájlokat kijelöl/kijelölést levesz           #Ins,Shift-Kurzorbill.#
  Csoport kijelölése                                        #Szürke +#
  Csoportkijelölést levesz                                  #Szürke -#
  Kijelölést megfordít                                      #Szürke *#
  Kijelöli az aktuális fájl kiterjesztésével
    megegyező fájlokat                               #Ctrl-<Szürke +>#
  A kijelölést leveszi az aktuális fájl
    kiterjesztésével megegyező fájlokról             #Ctrl-<Szürke ->#
  A kijelölést megfordítja a mappákon is
    (a parancssor állapotától és a mappák
    kijelölhetőségi opciójától függetlenül)          #Ctrl-<Szürke *>#
  Kijelöli az aktuális fájllal azonos nevű fájlokat   #Alt-<Szürke +>#
  A kijelölést leveszi az aktuális fájllal
    megegyező nevű fájlokról                          #Alt-<Szürke ->#
  Minden fájl kijelölése                            #Shift-<Szürke +>#
  Minden fájlról leveszi a kijelölést               #Shift-<Szürke ->#
  Visszaállítja az előző kijelölést                           #Ctrl-M#

  Túl hosszú fájlnevek és megjegyzések gördítése    #Alt-Bal,Alt-Jobb#
                                                    #Alt-Home,Alt-End#

  Rövid nézet módra vált                                   #BalCtrl-1#
  Közepes nézet módra vált                                 #BalCtrl-2#
  Teljes nézet módra vált                                  #BalCtrl-3#
  Széles nézet módra vált                                  #BalCtrl-4#
  Részletes nézet módra vált                               #BalCtrl-5#
  Fájlmegjegyzések módra vált                              #BalCtrl-6#
  Hosszú megjegyzések módra vált                           #BalCtrl-7#
  Fájltulajdonos nézet módra vált                          #BalCtrl-8#
  Fájl linkek nézet módra vált                             #BalCtrl-9#
  Alternatív teljes nézet módra vált                       #BalCtrl-0#

  A rejtett és rendszerfájlokat megmutatja/elrejti            #Ctrl-H#
  Hosszú és rövid fájlnév nézet között vált                   #Ctrl-N#

  Elrejti/megmutatja a bal panelt                            #Ctrl-F1#
  Elrejti/megmutatja a jobb panelt                           #Ctrl-F2#

  Név szerint rendezi az aktív panel fájljait                #Ctrl-F3#
  Kiterjesztés szerint rendezi az aktív panel fájljait       #Ctrl-F4#
  Módosítás ideje szerint rendezi az aktív panel fájljait    #Ctrl-F5#
  Méret szerint rendezi az aktív panel fájljait              #Ctrl-F6#
  Rendezetlenül mutatja az aktív panel fájljait              #Ctrl-F7#
  Keletkezésük ideje szerint rendezi az aktív panel fájljait #Ctrl-F8#
  Hozzáférésük ideje szerint rendezi az aktív panel fájljait #Ctrl-F9#
  Megjegyzéseik szerint rendezi az aktív panel fájljait     #Ctrl-F10#
  Fájltulajdonos szerint rendezi az aktív panel fájljait    #Ctrl-F11#
  Megjeleníti a ~rendezési elv~@PanelCmdSort@ menüt                         #Ctrl-F12#
  Rendezési csoportok szerint rendez                       #Shift-F11#
  A kijelölt fájlokat előre helyezi                        #Shift-F12#

  ~Mappa gyorsbillentyűt~@FolderShortcuts@ készít                      #Ctrl-Shift-0...9#
  Gyorsbillentyűvel mappára ugrik                     #JobbCtrl-0...9#

    Ha az aktív panel ~gyorsnézet panel~@QViewPanel@, ~fastruktúra panel~@TreePanel@ vagy
    ~info panel~@InfoPanel@, nem az aktív, hanem a passzív panel vált a
    megfelelő mappára.

  A kijelölt fájlok neveit a vágólapra másolja
    (ha a parancssor üres)                                  #Ctrl-Ins#
  A kijelölt fájlok neveit a vágólapra másolja
    (a parancssor állapotától függetlenül)            #Ctrl-Shift-Ins#
  A kijelölt fájlok neveit elérési úttal a vágólapra
    másolja (a parancssor állapotától függetlenül)     #Alt-Shift-Ins#
  A kijelölt fájlok hálózati (UNC) neveit a vágólapra
    másolja (a parancssor állapotától függetlenül)      #Ctrl-Alt-Ins#

  Megjegyzések:

  1. ^<wrap>Ha a ~Panel beállítások~@PanelSettings@ menü "Fordított rendezés
engedélyezése" opciója be van kapcsolva, akkor az aktuális rendezési elv
másodszori használata a rendezést növekvőről csökkenőre állítja át és
fordítva.

  2. ^<wrap>A túl hosszú fájlneveket és megjegyzéseket jobbra-balra görgető
#Alt-Bal# és #Alt-Jobb# billentyűkombináció csak a normál #Bal# és #Jobb#
kurzornyilakkal működik, a numerikus billentyűzet kurzorvezérlőivel nem. Ez
azért van így, mert lenyomott #Alt#-nál a numerikus billentyűzet számai
karakterek bevitelére szolgálnak, a karakterek decimális kódjaival.

  3. ^<wrap>A #Ctrl-Alt-Ins# billentyűkombináció a következő szabály szerint
másol szöveget a vágólapra:

     - ^<wrap>hálózati meghajtóknál a fájl hálózati (UNC) nevét másolja ki;
     - helyi meghajtóknál a fájl helyi nevét másolja ki, a
~szimbolikus linkjeivel~@HardSymLink@ együtt.

  4. ^<wrap>Ha az #Alt-Shift-Ins# vagy a #Ctrl-Alt-Ins# kombináció
használatánál a kurzor a #..# nevű elem felett áll, akkor az aktuális mappa
neve másolódik a vágólapra.


@PanelCmdSort
$ #Rendezési elv#
    A Rendezési elv menü a #Ctrl-F12#-vel hívható meg és az éppen
aktív panelre fog vonatkozni. A következő rendezési módok használhatók:

  Nevük szerint rendezi a fájlokat                           #Ctrl-F3#
  Kiterjesztésük szerint rendezi a fájlokat                  #Ctrl-F4#
  Módosításuk ideje szerint rendezi a fájlokat               #Ctrl-F5#
  Méretük szerint rendezi a fájlokat                         #Ctrl-F6#
  Rendezetlenül mutatja a fájlokat                           #Ctrl-F7#
  Keletkezésük ideje szerint rendezi a fájlokat              #Ctrl-F8#
  Hozzáférésük ideje szerint rendezi a fájlokat              #Ctrl-F9#
  Megjegyzéseik szerint rendezi a fájlokat                  #Ctrl-F10#
  Tulajdonosuk szerint rendezi a fájlokat                   #Ctrl-F11#

  Rendezési csoport használata                             #Shift-F11#
  A kijelölt fájlok előre kerülnek                         #Shift-F12#
  Numerikus rendezés

  #Megjegyzések a numerikus rendezéshez#

    Az operációs rendszer fájlrendezési algoritmusa a Windows XP-ben
megváltozott: a sztring alapú rendezés helyett numerikus rendezést vezettek be.
Az XP-hez hasonlóan a FAR is tud numerikusan rendezni, vagyis a fájlnevek
kezdő nulláit képes figyelmen kívül hagyni. A következő példa szemlélteti a
kétféle rendezés eltéréseit:

    Numerikus rendezés                 Sztring alapú rendezés
    (Windows XP)                       (Windows 2000)

    Ie4_01                             Ie4_01
    Ie4_128                            Ie4_128
    Ie5                                Ie401sp2
    Ie6                                Ie5
    Ie401sp2                           Ie501sp2
    Ie501sp2                           Ie6
    5.txt                              11.txt
    11.txt                             5.txt
    88.txt                             88.txt


@FastFind
$ #Gyorskeresés a paneleken#
    A fájlok gyors megkeresésére használhatjuk a #gyorskeresés# műveletet, a
fájlnév karaktereinek begépelésével. A használatához először le kell
nyomni és nyomva tartani az #Alt#-ot (vagy az #Alt-Shift#-et), majd addig kell
beírni a keresett fájl nevének karaktereit, amíg a kurzor rá nem áll a
fájlra.

    Ha a keresőablak aktív, a #Ctrl-Enter# lenyomására a kurzor
sorban végiglép a beírt karaktereknek megfelelő fájlneveken. A
#Ctrl-Shift-Enter# hasonlóan működik, de visszafelé léptet.

    A karakterek beírásánál joker (#*# és #?#) karakter is használható.

    A keresőablakba a vágólapról is beilleszthető szöveg (#Ctrl-V# vagy
#Shift-Ins#), ebben az esetben az első találatig keres.

    A keresőablakban idegen ábécé betűi is használhatók. Ha alkalmazzuk a
transzliteráló funkciót, a bevitt szöveg átíródik és az új szövegnek megfelelő
következő találatig tart a keresés. A TechInfo ##10-ben leírtak szerint
állítható be a transzliteráció gyorsbillentyűje.


@CmdLineCmd
$ #A parancssor parancsai#
 #Általános parancssori parancsok#

  Egy karakterrel balra                                   #Bal,Ctrl-S#
  Egy karakterrel jobbra                                 #Jobb,Ctrl-D#
  Egy szóval balra                                          #Ctrl-Bal#
  Egy szóval jobbra                                        #Ctrl-Jobb#
  A sor elejére                                            #Ctrl-Home#
  A sor végére                                              #Ctrl-End#
  Karakter törlése (jobbra)                                      #Del#
  Karakter törlése balra                                   #BackSpace#
  A sor végéig töröl (jobbra)                                 #Ctrl-K#
  Szó törlése balra                                   #Ctrl-BackSpace#
  Szó törlése jobbra                                        #Ctrl-Del#
  Másolás a vágólapra                                       #Ctrl-Ins#
  Beillesztés a vágólapról                                 #Shift-Ins#
  Előző utasítás                                              #Ctrl-E#
  Következő utasítás                                          #Ctrl-X#
  Parancssor törlése                                          #Ctrl-Y#

 #Beszúró parancsok#

  Beszúrja a parancssorba az akív panel
  aktuális fájljának nevét                         #Ctrl-J,Ctrl-Enter#

     ~Gyorskeresés~@FastFind@ módban a #Ctrl-Enter# nem fájlnevet szúr be,
     hanem a találatnak megfelelő fájlokon lépked végig.

  Beszúrja a passzív panel aktuális fájlnevét       #Ctrl-Shift-Enter#
  Beszúrja az aktív panel fájlnevét, elérési úttal            #Ctrl-F#
  Beszúrja a passzív panel fájlnevét, elérési úttal           #Ctrl-;#
  Beszúrja az aktív panel hálózati (UNC) fájlnevét        #Ctrl-Alt-F#
  Beszúrja a passzív panel hálózati (UNC) fájlnevét       #Ctrl-Alt-;#

  Beszúrja a bal panel elérési útvonalát                      #Ctrl-[#
  Beszúrja a jobb panel elérési útvonalát                     #Ctrl-]#
  Beszúrja a bal panel hálózati (UNC) elérési útvonalát   #Ctrl-Alt-[#
  Beszúrja a jobb panel hálózati (UNC) elérési útvonalát  #Ctrl-Alt-]#

  Beszúrja az aktív panel elérési útvonalát             #Ctrl-Shift-[#
  Beszúrja a passzív panel elérési útvonalát            #Ctrl-Shift-]#
  Beszúrja az aktív panel hálózati (UNC) elérési útját   #Alt-Shift-[#
  Beszúrja a passzív panel hálózati (UNC) elérési útját  #Alt-Shift-]#

  Megjegyzések:

  1. ^<wrap>Ha a parancssor üres, a #Ctrl-Ins# ugyanúgy átmásolja a kijelölt
fájlneveket a panelről a vágólapra, mint a #Ctrl-Shift-Ins# (lásd
~Panelvezérlő parancsok~@PanelCmd@);

  2. ^<wrap>A #Ctrl-End# lenyomása a parancssor végén azt eredményezi,
hogy a parancssor jelenlegi tartalma helyére a parancssori
~előzmények~@History@ első olyan parancsa kerül, ami a parancssor aktuális
tartalmával kezdődik, ha ilyen létezik. A #Ctrl-End# többszöri lenyomása
sorban előhívja a többi ilyen parancsot.

  3. ^<wrap>A fent leírt parancsok többsége működik minden
szerkesztőfunkcióban, beleértve a párbeszédablakok beviteli sorait és a belső
szerkesztőt is.

  4. ^<wrap>Az #Alt-Shift-Bal#, #Alt-Shift-Jobb#, #Alt-Shift-Home# és
az #Alt-Shift-End# kijelölik a blokkot a parancssorban akkor is, ha a panelek
be vannak kapcsolva.

  5. ^<wrap>A helyi meghajtókra kiadott "Fájl hálózati (UNC) nevének
beszúrása" parancs a fájlok helyi nevét illeszti be, elérési útjukkal és
~szimbolikus linkjeikkel~@HardSymLink@ együtt.


@FuncCmd
$ #Panelvezérlő parancsok - rendszerparancsok#
  Súgó                                                            #F1#

  ~Felhasználói menü~@UserMenu@ megjelenítése                                 #F2#

  Megnéz                                 #Ctrl-Shift-F3, Numpad 5, F3#

    Fájlon lenyomva a #Numpad 5# vagy az #F3# meghívja a ~belső~@Viewer@, a külső
    vagy a ~társított~@FileAssoc@ nézőkét, a fájl típusától és a
    ~külső nézőke beállításaitól~@ViewerSettings@ függően. A #Ctrl-Shift-F3# mindig a
    belső nézőkét hívja meg, a fájl kiterjesztésétől függetlenül.
    Mappán lenyomva kiszámítja és megmutatja a kijelölt mappák
    méretét.

  Szerkeszt                                        #Ctrl-Shift-F4, F4#

    Az #F4# meghívja a ~belső~@Editor@, a külső vagy a ~társított~@FileAssoc@ szerkesztőt,
    a fájl típusától és a ~külső szerkesztő beállításaitól~@EditorSettings@ függően.
    A #Ctrl-Shift-F4# mindig a belső szerkesztőt hívja meg, a fájlok
    társításaitól függetlenül. Az #F4# és a #Ctrl-Shift-F4# a mappákon
    az ~attribútumok~@FileAttrDlg@ megváltoztatása párbeszédablakot hívja elő.

  ~Másol~@CopyFiles@                                                           #F5#

    Fájlokat és mappákat másol. Ha másolás előtt szeretnénk létre-
    hozni a célmappát, a név végére tegyünk lezáró \\-jelet.

  ~Átnevezés vagy mozgatás~@CopyFiles@                                         #F6#

    Fájlokat és mappákat nevez át vagy helyez át. Ha a mozgatás előtt
    szeretnénk létrehozni a célmappát, a név végére tegyünk lezáró
    \\-jelet.

  ~Új mappát hoz létre~@MakeFolder@                                             #F7#

  ~Töröl~@DeleteFile@                                      #Shift-Del, Shift-F8, F8#

  ~Kisöpör~@DeleteFile@                                                    #Alt-Del#

  ~Menüsor~@Menus@ megjelenítése                                           #F9#

  Kilépés a FAR-ból                                              #F10#

  ~Plugin parancsok~@Plugins@ megjelenítése                                 #F11#

  Bal panel meghajtóváltás                                    #Alt-F1#

  Jobb panel meghajtóváltás                                   #Alt-F2#

  Belső/külső nézőke váltó                                    #Alt-F3#

    Ha a belső nézőke az alapértelmezett, meghívja a Beállítások
    ~nézőke beállításokban~@ViewerSettings@ megadott külső nézőkét vagy a fájl
    típusához ~társított nézőkét~@FileAssoc@. Ha a külső nézőke alapértelmezett,
    meghívja a belső nézőkét.

  Belső/külső szerkesztő váltó                                #Alt-F4#

    Ha a belső szerkesztő az alapértelmezett, meghívja a Beállítások
    ~szerkesztő beállításokban~@EditorSettings@ megadott külső szerkesztőt vagy a fájl
    típusához ~társított szerkesztőt~@FileAssoc@. Ha a külső szerkesztő az
    alapértelmezett, akkor meghívja a belső szerkesztőt.

  Fájlok nyomtatása                                           #Alt-F5#

    Ha a "Print Manager" plugin telepítve van, a kijelölt fájlok
    nyomtatása ezen a nyomtatóvezérlő pluginen keresztül történik.
    Ha nincs telepítve, akkor a belső lehetőségek szerint.

  ~Fájl linkek~@HardSymLink@ létrehozása (csak NTFS)                         #Alt-F6#

    A hardlinkek segítségével különböző fájlnevekkel hivatkozhatunk
    ugyanarra az adatra.

  ~Fájlkeresés~@FindFile@ parancs végrehajtása                            #Alt-F7#

  ~Parancs előzmények~@History@ megjelenítése                            #Alt-F8#

  A FAR konzolablak méretének átkapcsolása                    #Alt-F9#

    Ablakban futtatás módban ezzel a paranccsal váltogatni lehet az
    aktuális ablakméret és a konzolablak lehető legnagyobb mérete
    között. Teljes képernyős módban az #Alt-F9# váltogat a 25 soros
    és az 50 soros függőleges felbontás között. Részletek a TechInfo
    ##38-nál.

  ~Plugin beállítások~@Plugins@                                    #Alt-Shift-F9#

  ~Mappakeresés~@FindFolder@ parancs végrehajtása                          #Alt-F10#

  ~Megnézett és szerkesztett fájlok~@HistoryViews@ előzménye                 #Alt-F11#

  ~Mappa előzmények~@HistoryFolders@ megjelenítése                             #Alt-F12#

  Fájlok hozzáadása tömörített fájlhoz                      #Shift-F1#
  Fájlok kibontása tömörített fájlból                       #Shift-F2#
  Tömörítettkezelő parancsok végrehajtása                   #Shift-F3#
  ~Új fájl~@FileOpenCreate@ szerkesztése                                      #Shift-F4#

    Új fájl megnyitásánál a fájl ugyanazt a kódlapot kapja, mint amit
    a szerkesztőben utoljára használtunk. A FAR szerkesztője első
    megnyitásakor az alapértelmezett kódlapot fogja használni.

  A kurzor alatti fájl másolása                             #Shift-F5#
  A kurzor alatti fájl átnevezése/áthelyezése               #Shift-F6#

    Mappákon: ha a megadott (abszolút vagy relatív) elérési út létező
    mappára mutat, akkor a forrásmappát e célmappa belsejébe mozgatja.
    Ha nem, akkor a forrásmappát az új elérési útra nevezi át (vagy
    helyezi át).

    Példaként #c:\mappa1\#-et mozgassuk #d:\mappa2\#-re:

    - ha #d:\mappa2\# létezik, akkor #c:\mappa1\# tartalma átkerül
      #d:\mappa2\mappa1\# mappába;
    - ha nem létezik, akkor #c:\mappa1\# áthelyeződik (átneveződik)
      az újonnan létrehozott #d:\mappa2\# mappába (mappára).

  A kurzor alatti ~fájl törlése~@DeleteFile@                              #Shift-F8#
  Beállítások mentése                                       #Shift-F9#
  A kurzor rááll a legutóbb végrehajtott menüelemre        #Shift-F10#

  Végrehajtás, belépés mappába vagy tömörített fájlba          #Enter#
  Végrehajtás külön ablakban                             #Shift-Enter#

    A #Shift-Enter# mappán lenyomva meghívja a Windows Intézőt és
    megjeleníti a kijelölt mappa tartalmát. Ha egy meghajtó gyökerét
    szeretnénk látni az Intézőben, használjuk a #Shift-Enter#-t a
    ~meghajtók~@DriveDlg@ menü megfelelő meghajtóján. A #Shift-Enter# a mappák
    #..# elemén lenyomva az aktuális mappát nyitja meg az Intézőben.

  Belépés a gyökérmappába                                     #Ctrl-\\#

  Belépés mappába vagy tömörített fájlba (SFX-be is)       #Ctrl-PgDn#

    Ha a kurzor mappán áll, a #Ctrl-PgDn# beléptet a mappába. Ha a
    kurzor fájlon áll, a fájl típusához ~társított parancsot~@FileAssoc@ hajtja
    végre, vagy belép a tömörítettbe.

  Visszalépés a szülőmappába                               #Ctrl-PgUp#

    Ha a "Kezelőfelület beállítások" ~A Ctrl-PgUp meghajtót vált~@InterfSettings@
    opcióját engedélyeztük, a meghajtók gyökerében a #Ctrl-PgUp#
    lenyomása a hálózati plugint hívja meg, vagy a ~Meghajtók~@DriveDlg@ menüt.

  Gyorsbillentyűt rendel az aktuális mappához       #Ctrl-Shift-0...9#

  Mappa gyorsbillentyű használata                     #JobbCtrl-0...9#

  ~Fájl attribútumok~@FileAttrDlg@ beállítása                                #Ctrl-A#
  ~Parancs végrehajtása~@ApplyCmd@ a kijelölt fájlokon                    #Ctrl-G#
  ~Megjegyzést~@FileDiz@ fűz a kijelölt fájlokhoz vagy mappákhoz         #Ctrl-Z#


@DeleteFile
$ #Fájlok és mappák törlése és kisöprése#
    ^<wrap>A következő billentyűket használhatjuk fájlok és mappák
törlésére és kisöprésére:

    #F8#         - ^<wrap>ha vannak a panelen kijelölt fájlok vagy mappák,
törlődnek, egyébként csak a kurzor alatti fájl vagy mappa törlődik;

    #Shift-F8#   - ^<wrap>csak a kurzor alatti elem törlődik, függetlenül
attól, hogy van-e kijelölt fájl vagy mappa;

    #Shift-Del#  - ^<wrap>végleg törli a kijelölt elemeket, ezért a Lomtárban
sem jelennek meg;

    #Alt-Del#    - kisöpri a fájlokat és mappákat (biztonsági törlés).


    Megjegyzések:

    1. A ~rendszer beállításaitól~@SystemSettings@ függ, hogy az #F8# és
a #Shift-F8# a törölt fájlokat a Lomtárba teszi-e vagy sem. Ezzel szemben
a #Shift-Del# mindig a Lomtár kihagyásával töröl.

    2. Kisöprésnél (#Alt-Del#) a FAR a fájl adatait törlés előtt nullákkal
írja felül (a TechInfo##29-ben elolvasható, hogyan lehet zéró helyett más
felülíró karaktert megadni), ezután a fájl méretét nulla hosszúságúra
állítja, átmeneti nevet ad neki, végül törli.


@ErrCannotExecute
$ #Hiba: nem végrehajtható#
    A program, amit megpróbáltunk futtatni, nem értelmezhető sem belső, sem
külső parancsként, sem futtatható programként, sem batch fájlként.

    Amikor a FAR megpróbálja végrehajtani a parancssor tartalmát, a
következő sorrendben kutatja át a mappákat futtatható fájlok után,
a %PATHEXT% környezeti változóban definiált sorrendben behelyettesítve minden
kiterjesztést, kezdve a ".BAT;.CMD;.EXE;.COM;" kiterjesztésekkel:

  1. Az aktuális mappában keres.
  2. A %PATH% környezeti változó elérési útvonalaiban keres.
  3. A 32 bites Windows rendszermappában keres (SYSTEM32).
  4. A 16 bites Windows rendszermappában keres (SYSTEM).
  5. A Windows mappájában keres.


@MiscCmd
$ #Panelvezérlő parancsok - egyebek#
  Képernyőgrabber                                            #Alt-Ins#

    A képernyőgrabberrel bármelyik képernyőterület kijelölhető és a
    vágólapra másolható. A kurzor a #kurzornyilakkal# vagy az #egérrel#
    mozgatható. A szöveg a #Shift-kurzornyilakkal# vagy az egérrel,
    lenyomott #bal gombnál# mozgatással jelölhető ki. Az #Enter#, a
    #Ctrl-Ins#, a #jobb egérgomb# vagy a #kettős kattintás# a kijelölt
    szöveget a vágólapra másolja, a #Ctrl-<Szürke +># hozzáfűzi a
    vágólap aktuális tartalmához, az #Esc# pedig kiléptet a grab
    módból. A #Ctrl-U# leveszi a kijelölést a blokkról.

  ~Makró~@KeyMacro@ rögzítése                                           #Ctrl-<.>#

  Párbeszédablak előzmények                        #Ctrl-Fel, Ctrl-Le#

    A párbeszédablak szerkesztési előzményeiből az #Enterrel#
    másolhatjuk ki a lista kivánt elemét a beviteli mezőbe.
    Az #Ins# billentyű megjelöli vagy leveszi a jelölést egy-egy
    elemről. Az így megjelölt előzményeket a később hozzáadódó
    elemek nem tudják lelökni a listáról, így a gyakran használt
    sztringek mindig megmaradnak.

  Párbeszédablak szerkesztési előzményeinek törlése              #Del#

  Párbeszédablak szerkesztési előzmény aktuális elemének törlése
    a listából (ha az elem nincs rögzítve)                 #Shift-Del#

  A párbeszédablak alapértelmezett elemére állítja a kurzort    #PgDn#

  A kurzor alatti fájlnevet a párbeszédablakba illeszti  #Shift-Enter#

  A paszív panel kurzora alatti fájlnevet
    illeszti a párbeszédablakba                     #Ctrl-Shift-Enter#

    Ez a billentyűkombináció minden szerkesztett sornál működik,
    beleértve a párbeszédablakokat és a ~belső szerkesztőt~@Editor@ is,
    kivéve a parancssort.

    A #Ctrl-Enter# a párbeszédablakok alapértelmezett műveletét
    hajtja végre (lenyomja az alapértelmezett gombot vagy más
    hasonló dolgot művel).

    Párbeszédablakok jelölőnégyzeteinek billentyűparancsai:

    - bejelöli (#[x]#)                                        #Szürke +#
    - a jelölést kiveszi (#[ ]#)                              #Szürke -#
    - határozatlanul hagyja (#[?]#)                           #Szürke *#
      (utóbbi csak a háromállapotú jelölőnégyzeteknél
      működik)

    A #bal egérkattintás# a párbeszédablakon kívül eső területen
    egyenértékű az #Esc# lenyomásával.

    A #jobb egérkattintás# a párbeszédablakon kívül eső területen
    egyenértékű az #Enter# lenyomásával.

    A FAR Manager kezeli az ~egér görgőjét~@MsWheel@.

    A párbeszédablakok elmozdíthatók, ha az egérrel megfogjuk őket
    és húzzuk, vagy mozgathatók a #Ctrl-F5# lenyomása után a
    #kurzornyilakkal# is.


@MsWheel
$ #Egér: görgő támogatása#

   #Panelek#      ^<wrap>A görgő forgatása a fájlok listáját gördíti, a kurzor
helyzete változatlan marad.

   #Szerkesztő#   ^<wrap>A görgő a szöveget gördíti, a kurzor helyzete nem
változik (hasonló, mint a #Ctrl-Fel#/#Ctrl-Le#).

   #Nézőke#       A görgő a szöveget gördíti.

   #Súgó#         A görgő a szöveget gördíti.

   #Menük#        ^<wrap>A görgő #Fel#/#Le# billentyűként viselkedik, így
kurzorvezérlők nélkül is választhatunk menüpontot.

   #Párbeszéd-#
   #ablakok#      ^<wrap>A párbeszédablakokban, ha a szerkesztett sornak van
előzménylistája vagy lenyíló ablaka, a görgő lenyitja a legördülő listát és
ezután a görgő úgy viselkedik, mint a menükben.

    A görgő elfordításával arányos elmozduló sorok száma beállítható
a panelekre, a szerkesztőre és a nézőkére (lásd TechInfo ##33).

@Plugins
$ #Pluginek támogatása#
    A külső DLL modulok (pluginek) segítségével a FAR-t új parancsokkal
és emulált fájlrendszerekkel bővíthetjük. Például a tömörített fájlok
kezeléséről, valamint az FTP kliens, az ideiglenes panel és a hálózati
böngésző működéséről fájlrendszert emuláló pluginek gondoskodnak.

    Minden pluginnek saját mappája van a FAR.EXE-vel azonos mappából nyíló
Plugins mappán belül. Ha a FAR új plugint érzékel, elmenti az adatait és
később a plugint csak szükség esetén tölti be, így a nem használt pluginek
feleslegesen nem foglalnak memóriát. Ha biztosak vagyunk benne, hogy egyes
plugineket soha nem használunk, törlésükkel lemezterületet takaríthatunk meg.

    A pluginek meghívhatók a ~Meghajtók~@DriveDlg@ menüből, a
#Plugin parancsok# menüből az #F11# leütésével és a ~Parancsok~@CmdMenu@
menüből is. A "Plugin parancsok" menüben az #F4#-gyel
gyorsbillentyűt rendelhetünk a menü elemeihez, ezáltal egyszerűbben,
~makrókból~@KeyMacro@ hívhatók meg. A pluginek meghívhatók a
fájlpanelekről, valamint (csak az #F11#-gyel) a belső nézőkéből és a
szerkesztőből is, de a nézőkében és a szerkesztőben csak az
oda tervezett pluginek listája jelenik meg.

    A pluginek jellemzőit a ~Beállítások menüben~@OptMenu@, a
~Plugin beállítások~@PluginsConfig@ almenüben állíthatjuk be.

    A fájlműveletek, mint a másolás, áthelyezés, törlés, szerkesztés,
átnevezés vagy ~fájlkeresés~@FindFile@ működnek fájlrendszert emuláló
pluginekkel is, ha erre a plugin is képes. A "Fájlkeresés" parancs
az aktuális mappától kiadva kevesebb funkcionalitást vár el a plugintől,
mintha a gyökértől kezdve kerestetnénk, tehát ha az utóbbi módszer
nem működik megfelelően, használjuk az előbbit!

    A pluginek saját üzenetekkel és saját súgóval rendelkeznek. A
pluginek súgóinak listája a következők szerint jeleníthető meg:

    #Shift-F2# - a teljes FAR súgórendszerből használható

    #Shift-F1# - ^<wrap>a pluginek listájában használható
(helyzetérzékeny súgó).

    Ha a pluginnek nincs súgója, a helyzetérzékeny súgó ablaka nem
jelenik meg.

    Ha az aktív panel pluginnel emulált fájlrendszert mutat, a
parancssorból kiadott "CD" parancs a pluginnel emulált fájlrendszer mappái
közt vált. A "CD"-vel ellentétben a "CHDIR" parancs mindig valódi mappanévként
kezeli a megadott paramétert, függetlenül a fájlpanel típusától.

    Az #Alt-Shift-F9# billentyűkombináció meghívja a
~plugin beállítások~@PluginsConfig@ menüt.


@PluginsConfig
$ #Plugin beállítások#
    A telepített ~plugineket~@Plugins@ a ~Beállítások menü~@OptMenu@
#Plugin beállítások# menüpontjában konfigurálhatjuk. A menüt elérhetjük a
~Meghajtók~@DriveDlg@ menüből is, az #Alt-Shift-F9# leütésével.

    A "Plugin beállításokban" a kijelölt plugin helyzetérzékeny súgójához
a #Shift-F1#-gyel jutunk. Ha a pluginhez nem tartozik súgófájl, akkor a
helyzetérzékeny súgó nem jelenik meg.

    Ha a helyzetérzékeny súgót meghívjuk, a FAR megpróbálja megjeleníteni a
súgó #Config# témakörét. Ha a súgófájlban Config témakör nem létezik, akkor
a kijelölt plugin súgójának fő témaköre jelenik meg.

    A #Plugin beállítások# menüben az #F4#-gyel gyorsbillentyűt rendelhetünk
a menü elemeihez, így később ~makrókkal~@KeyMacro@ hívhatjuk
meg őket.


@PluginsReviews
$ #A pluginek képességeinek áttekintése#
    A FAR manager olyan szorosan összetartozik pluginjeivel, hogy egyszerűen
értelmetlen említésük nélkül beszélni a FAR-ról. A pluginek szinte korlátlan
bővítési lehetőséget biztosítanak a FAR-nak.

    Néhány lehetőség, a részletek és a teljesség igénye nélkül:

  * Szintaktikai szövegkiemelés a programok forrásszövegeiben.
  * ^<wrap>Együttműködés az FTP szerverekkel (beleértve a proxyn
keresztüli elérést).
  * Keresés és csere egyszerre több fájlban, reguláris
kifejezésekkel.
  * Fájlok csoportos átnevezése, a helyettesítő szimbólumok és
a sablonok összetett, vegyes alkalmazásával a maszkokban.
  * NNTP/SMTP/POP3/IMAP4 kliensek, üzenet küldése személyhívóra.
  * Nem szabványos felbontású szöveges képernyőmódok.
  * Szövegek kódlapkonverziója (nemzeti karakterek).
  * A Lomtár tartalmának kezelése.
  * A futó folyamatok prioritásainak szabályozása helyi és hálózati
számítógépeken.
  * Szavak automatikus kiegészítése a szerkesztőben, sablonok
kezelése.
  * A Windows regisztrációs adatbázisának szerkesztése.
  * Windows gyorsbillentyűk létrehozása, módosítása.
  * A FidoNetes fájlok és szövegek kényelmesebb kezelése.
  * Fájlok UU kódolása és dekódolása.
  * A WinAmp vezérlése, MP3 tag-ek szerkesztése.
  * A Quake (nevű játékprogram) PAK fájljainak kezelése.
  * Nyomtatóvezérlés, helyi és hálózati egyaránt.
  * ODBC kompatibilis adatbázisok lekérdezéseihez kapcsolódás,
hibakeresés.
  * RAS szolgáltatás kezelése.
  * Külső programok (compilerek, konverterek stb.) futtatása, miközben
a FAR szerkesztőjében szöveget szerkesztünk.
  * A windows-os súgófájlok (.hlp és .chm) tartalmának
megjelenítése.
  * Számológépek, különböző képességekkel.
  * Játékok :-)
  * Helyesírás-ellenőrző funkciók a FAR szövegszerkesztőben.
  * Cserélhető (lemezes) meghajtókhoz katalógus készítése és
sok más lehetőség...

    A letölthető pluginekről tájékoztató linkek:

  - FAR Manager official site
    ~http://www.farmanager.com~@http://www.farmanager.com@
  - Online fórum
    ~http://forum.farmanager.com~@http://forum.farmanager.com@
  - PlugRinG honlap
    ~http://plugring.farmanager.com~@http://plugring.farmanager.com@
  - Ingyenes levelezőcsoport-szolgáltatás
    ~http://groups.google.com/group/fardev/~@http://groups.google.com/group/fardev@
  - USENET echo konferencia
    ~news:fido7.far.support~@news:fido7.far.support@
    ~news:fido7.far.development~@news:fido7.far.development@
  - FidoNet echo konferencia
    far.support
    far.development


@Panels
$ #Panelek#
    A FAR rendszerint két fájlpanelt jelenít meg (bal és jobb panelt), rajtuk
különféle adatokkal. Ha változtatni szeretnénk a paneleken megjelenő
adatok jellegén, használjuk a ~panel menüket~@LeftRightMenu@ vagy a megfelelő
~billentyűparancsot~@KeyRef@!

    A következő témakörök további információval szolgálnak:

      ~Fájlpanel~@FilePanel@                  ~Fastruktúra panel~@TreePanel@
      ~Info panel~@InfoPanel@                 ~Gyorsnézet panel~@QViewPanel@

      ~Húzd és ejtsd művelet fájlokon~@DragAndDrop@
      ~Fájlok kijelölése~@SelectFiles@
      ~A fájlpanel nézetek testreszabása~@PanelViewModes@


@FilePanel
$ #Panelek: fájlpanel#
    A fájlpanelek az aktuális mappát jelenítik meg. Kijelölhetünk fájlokat és
mappákat vagy a jelölést levehetjük róluk, különböző fájl- és tömörítő
műveleteket hajthatunk végre rajtuk. A parancsok listája a
~billentyűparancsok~@KeyRef@ témakörben olvasható.

    A fájlpanelek alapértelmezett nézet módjai a következők:

 #Rövid#              Fájlnevek három oszlopban.

 #Közepes#            Fájlnevek két oszlopban.

 #Teljes#             A fájlok neve, mérete, dátuma és ideje.

 #Széles#             A fájlok neve és mérete.

 #Részletes#          ^<wrap>A fájlok neve, mérete, tömörített mérete; utolsó
módosítás, a létrehozás és hozzáférés dátuma/ideje és az attribútumok. Teljes
képernyős mód.

 #Megjegyzések#       A fájlnevek és a ~fájlmegjegyzések~@FileDiz@.

 #Hosszú megjegyzés#  ^<wrap>A fájlok neve, mérete és megjegyzése.
Teljes képernyős mód.

 #Fájl tulajdonos#    Fájlnevek, méretük és tulajdonosuk.

 #Fájl linkek#        Fájlnevek, méretük, hardlinkjeik száma.

 #Alternatív teljes#  ^<wrap>Fájlok neve, mérete (számjegyei ezresenként elkülönítve)
és a fájlok dátuma.

    A ~fájlpanel nézetek módjai~@PanelViewModes@ testreszabhatók.

    A "Tömörített méret" (TMéret) az NTFS tömörített fájljainál és az egyéb
tömörített fájloknál értelmezett. A "Fájltulajdonos" és a "Fájl linkek" csak
NTFS fájlrendszerben értelmezettek. Előfordulhat, hogy néhány fájlrendszer nem
kezeli a fájlok létrehozásának és hozzáférésének dátumát.

    Ha panel nézet módot szeretnénk váltani, válasszunk a
~panel menüben~@LeftRightMenu@. Ha a panel jellege eredetileg nem fájlpanel
volt, a módváltás vagy a meghajtóváltás után automatikusan fájlpanel módra
vált.

    A ~gyorskeresés~@FastFind@ művelet a keresett fájlnév karaktereinek
begépelésével a megfelelő fájlra állítja a kurzort.


@TreePanel
$ #Panelek: fastruktúra panel#
    A fastruktúra panel az aktív panel mappaszerkezetét egy fa ágaihoz
hasonlóan ábrázolja. Ebben a nézetben gyorsan lehet mappát váltani és
a mappákkat kezelni.

    A FAR a mappák szerkezeti adatainak tárolásához minden meghajtó
gyökérmappájában létrehoz egy-egy #Tree3.Far# nevű fájlt, a csak olvasható
meghajtók adatait pedig egy Tree3.Cache nevű rejtett mappába menti el,
a FAR.EXE mappáján belül. A Tree3.Far fájl eredetileg nem létezik, a
#Fastruktúra panel# vagy a #Mappa keresése# funkció első használata után jön
létre automatikusan. Ha a Tree3.Far már létezik és ha változik a fa, a FAR
frissíti a megváltozott szerkezet adatait. Ha a szerkezeti
változások akkor történtek, amikor a FAR nem futott és a Tree3.Far tartalma
már nem teljesen időszerű, a #Ctrl-R# leütésével frissíthetjük.

    A #gyorskeresés# segítségével hamarabb megtalálhatjuk a mappákat. Tartsuk
nyomva az Alt billentyűt és addig írjuk be a keresett mappa nevét, amíg rá nem
áll a sávkurzor. A #Ctrl-Enter# lenyomásával a további találatokon
lépkedhetünk.

    A #Szürke +# és a #Szürke -# billentyűkkel fel-le mozoghatunk ugyanannak
a szintnek a mappáin.

@InfoPanel
$ #Panelek: info panel#
    Az információs panelről a következő adatok olvashatók le:

    - ^<wrap>a számítógép és az aktuális felhasználó #hálózati# neve;

    - ^<wrap>az #aktuális lemez# betűjele és fajtája, a fájlrendszer típusa,
a hálózat neve, a teljes és a szabad lemezterület, a kötet címkéje és
sorozatszáma;

    - ^<wrap>a #memória# foglaltsága százalékban (a 100% a teljes rendelkezésre
álló memória foglaltságát jelenti), a fizikai és a virtuális memória
teljes és szabad mérete;

    - #mappa megjegyzésfájl#.

    A mappa megjegyzésfájlok tartalmát teljes képernyőn tekinthetjük meg az
#F3#-mal vagy a #bal egérgombbal#. A megjegyzés megnézéséhez vagy
szerkesztéséhez nyomjuk le az #F4#-et vagy a #jobb egérgombot#. Több
~nézőke parancs~@Viewer@ (keresés, kódlap kiválasztása és mások) használható
a mappák megjegyzéseinek megtekintésénél is.

    Hogy a FAR mely fájlokat kezelje mappa megjegyzésként, megadhatjuk a
~Beállítások menü~@OptMenu@ "Mappa megjegyzésfájlok" menüjének listájában.

    A FAR igyekszik felismerni az összes rendszerbe csatlakozó CD meghajtó
típusát. A felismert típusok: CD-ROM, CD-RW, CD-RW/DVD, DVD-ROM, DVD-RW és
DVD-RAM. Ez a funkció csak Windows NT/XP-n elérhető; a rendszergazda
jogokkal rendelkező felhasználóknál és a helyi felhasználóknál működik,
ha a "Helyi biztonsági beállítások" szerkesztőjében a
#Helyi házirend/Biztonsági beállítások/Eszközök:#-ben
#A CD-ROM használatához kötelező bejelentkezni a helyi számítógépre#
paramétert engedélyeztük. Az említett biztonsági szerkesztőprogramot a
parancssorból a #secpol.msc# parancs kiadásával is elindíthatjuk.

    Virtuális lemezeknél (SUBST-disk) az info panelről a gazdalemez
jellemzőit olvashatjuk le.

@InfoPanelShowMode
$ #Режимы отображения информационной панели#

@InfoPanelSettings
$ #Настройка информационной панели#

@QViewPanel
$ #Panelek: gyorsnézet panel#
    A gyorsnézet panel a ~fájlpanel~@FilePanel@ vagy a ~fastruktúra panel~@TreePanel@
kiválasztott elemének információit mutatja meg.

    Ha a választott elem fájl, a tartalma jelenik meg. A ~belső nézőke~@Viewer@ több
parancsa használható a panelen megjelenített fájlra. Ha a fájl a Windowsban
regisztrált fájltípus, akkor ez a paraméter is megjelenik.

    Mappákon a gyorsnézet panel a teljes méretet, a teljes tömörített méretet,
a fájlok és az almappák számát, az aktuális lemez klaszterméretét, a fájlok
valódi méretét, beleértve a meddő lemezterületet (a kihasználatlan
klaszterrészek összegét) jeleníti meg. A tömörített méretnek csak NTFS
meghajtókon van értelme.

    Reparse pontoknál a forrásmappa elérési útja is megjelenik.

    Mappáknál a teljes méret értéke eltérhet a valóságostól, ha:

    1. A mappa vagy a mappa almappái szimbolikus linkeket tartalmaz(nak) és
a ~rendszer beállítások~@SystemSettings@ párbeszédablakban a "Szimbolikus
linkek vizsgálata" opciót engedélyeztük.

    2. A mappa vagy a mappa almappái ugyanarra a fájlra mutató
hardlinkeket tartalmaznak.


@DragAndDrop
$ #Másolás: húzd és ejtsd művelet fájlokon#
    Fájlok #másolásához# és #áthelyezéséhez# használhatjuk a "húzd és ejtsd"
módszert. Nyomjuk le és tartsuk nyomva az egér bal gombját a forrásfájlokon
és mappákon, húzzuk át a másik panelre, majd engedjük fel az egér gombját.

    Ha fájlok vagy mappák csoportját szeretnénk másolni (vagy áthelyezni),
előbb jelöljük ki őket, majd a fent leírtak szerint fogjuk meg és húzzuk át.

    Megfogás vagy húzás közben átkapcsolhatunk másolásról áthelyezésre,
ha lenyomjuk az egér jobb gombját. Áthelyezhetünk úgy is, ha a bal egérgomb
lenyomása előtt lenyomjuk és nyomva tartjuk a #Shift# billentyűt.


@Menus
$ #Menük#
    A menüsort az F9 lenyomásával vagy a konzolképernyő felső részén
egérkattintással jeleníthetjük meg.

    Ha a menüsort aktiváljuk az #F9#-cel, automatikusan az aktív paneloldal
menüje jelölődik ki, a #Tab# pedig váltogat a Jobb és Bal menü közt. Ha a
"Fájlok", "Parancsok" vagy "Beállítások" menü volt aktív, először a
passzív panel aktivizálódik.

    A #Shift-F10# billentyűkombináció a menü utoljára használt elemére ugrik.

    A következő témakörök részletezik a menük szolgáltatásait:

     ~Bal és Jobb menü~@LeftRightMenu@              ~Fájlok menü~@FilesMenu@

     ~Parancsok menü~@CmdMenu@                ~Beállítások menü~@OptMenu@


@LeftRightMenu
$ #Menük: Bal és Jobb menü#
   A #Bal# és #Jobb# menüben a két panel jellemzőit egymástól függetlenül
állíthatjuk be. A menük elemei:

   #Rövid#                A fájlok három oszlopban jelennek meg.

   #Közepes#              A fájlok két oszlopban jelennek meg.

   #Teljes#               A fájlok neve, mérete, dátuma és ideje.

   #Széles#               A fájlok neve és mérete jelenik meg.

   #Részletes#            ^<wrap>A fájlok neve, mérete, tömörített mérete,
a módosítás, létrehozás és hozzáférés ideje, attribútumok. Teljes képernyős
mód.

   #Fájlmegjegyzések#     A fájlok neve, ~megjegyzésükkel~@FileDiz@.

   #Hosszú megjegyzés#    A fájlok neve, mérete és megjegyzése.
                        Teljes képernyős mód.

   #Fájltulajdonos#       A fájlok neve, mérete és tulajdonosa.

   #Fájl linkek#          A fájlok neve, mérete, hardlinkek száma.

   #Alternatív teljes#    ^<wrap>A fájlok neve, mérete (rendezett formátumú
számokkal) és dátuma.

   #Info panel#           A panelt ~info panel~@InfoPanel@ módra váltja.

   #Fastruktúra#          A panelt ~fastruktúra~@TreePanel@ módra váltja.

   #Gyorsnézet#           A panelt ~gyorsnézet~@QViewPanel@ módra váltja.

   #Rendezési elv#        A lehetséges rendezési módok megjelenítése.

   #Hosszú fájlnevek#     A fájlnevek hosszú/rövid módja közt vált.

   #Panel be/ki#          Megmutatja vagy elrejti a panelt.

   #Frissítés#            Újraolvassa a panel tartalmát.

   #Meghajtóváltás#       ^<wrap>Az aktuális meghajtóról másikra válthatunk a
Meghajtók menüben.

@FilesMenu
$ #Menük: Fájlok menü#

   #Megnéz#                 ^<wrap>~Fájlokba néz bele~@Viewer@ vagy megméri a
mappák tartalmának méretét.

   #Szerkeszt#              Fájlok ~szerkesztése~@Editor@.

   #Másol#                  Fájlokat és mappákat ~másol~@CopyFiles@.

   #Átnevez-Mozgat#         ^<wrap>Fájlokat és mappákat
~nevez át vagy mozgat~@CopyFiles@.

   #Új mappa#               ~Új mappát~@MakeFolder@ hoz létre.

   #Töröl#                  Fájlokat és mappákat töröl.

   #Kisöpör#                ^<wrap>Fájlokat és mappákat söpör ki (biztonsági
törlés). Az adatokat nullákkal írja felül, majd a fájlt megcsonkítja,
átmeneti nevet ad neki, végül törli.

   #Tömörhöz hozzáad#       A kijelölt fájlokat tömöríti.

   #Tömörből kibont#        Fájlokat csomagol ki tömörített fájlból.

   #Tömörítő parancsok#     Parancsokat hajt végre tömörített fájlokon.

   #Fájl attribútumok#      ^<wrap>A ~fájlok attribútumait~@FileAttrDlg@ és
dátumát/idejét változtatja meg.

   #Parancs végrehajtása#   ^<wrap>~Parancsot hajt végre~@ApplyCmd@ a kijelölt
fájlokon.

   #Fájlmegjegyzések#       ~Megjegyzést fűz~@FileDiz@ a kijelölt fájlokhoz.

   #Csoport kijelölése#     ^<wrap>Fájlok csoportját ~jelöli ki~@SelectFiles@,
joker karakterek segítségével.

   #Jelölést levesz#        ^<wrap>Fájlok csoportjáról
~leveszi a jelölést~@SelectFiles@, joker karakterek segítségével.

   #Jelölést megfordít#     ~Megfordítja~@SelectFiles@ a fájlok kijelölését.

   #Jelölést visszatesz#    ^<wrap>~Visszateszi~@SelectFiles@ a fájlok előző
kijelölését fájlművelet vagy csoport kijelölése után.

   A menü néhány parancsáról további leírást olvashatunk a
~Fájlkezelő és rendszerparancsok~@FuncCmd@ témakörben.


@CmdMenu
$ #Menük: Parancsok menü#

   #Fájlkeresés#          ^<wrap>Fájlokat keres a mappák fáiban, joker
karakterek is használhatók. Bővebben a ~fájlkeresés~@FindFile@ témakörben.

   #Parancs előzmények#   ^<wrap>Kilistázza a korábbi parancsokat. Bővebb
információ a ~parancs előzmények~@History@ témakörben található.

   #Video mód#            ^<wrap>Teljes képernyőn 25 és 50 soros felbontás
között vált, konzolablakban az ablak eredeti mérete és maximális mérete
között vált.

   #Mappakeresés#         ^<wrap>Mappát keres a mappák fáiban. További
információk a ~mappakeresés~@FindFolder@ témakörben.

   #Fájl előzmények#      ^<wrap>A korábban
~megnézett vagy szerkesztett~@HistoryViews@ fájlok listáját mutatja meg.

   #Mappa előzmények#     ^<wrap>A bejárt ~mappák előzményeit~@HistoryFolders@
jeleníti meg. A "Mappa előzmények" és a "Fájl előzmények" listák elemei
kiválasztás után a lista aljára kerülnek. Ezt elkerülhetjük, ha Enter helyett
#Shift-Enterrel# választunk közülük.

   #Panelcsere#           Megcseréli a bal és jobb panelt.

   #Panelek be/ki#        Megjeleníti/elrejti mindkét panelt.

   #Mappák#               Összeveti a mappák tartalmát.
   #összehasonlítása#     ^<wrap>A ~mappák összehasonlítása~@CompFolders@
témakör részletezi a funkciót.

   #Felhasználói menü#    Fő vagy helyi ~felhasználói menüt~@UserMenu@
   #szerkesztése#         szerkeszthetünk. Az #Ins# billentyű beszúrja,
                        a #Del# törli, az #F4# pedig szerkeszti a menü
                        elemeit.

   #Fájltársítások#       ^<wrap>Megjeleníti a ~fájltársításokat~@FileAssoc@.
Az #Ins# billentyűvel beszúrhatjuk, a #Del#-lel törölhetjük, az #F4#-gyel
pedig szerkeszthetjük a fájlok társításait.

   #Mappa#                A ~mappa gyorsbillentyűk~@FolderShortcuts@ aktuális
   #gyorsbillentyűk#      összerendeléseit jeleníti meg.

   #Fájlpanel szűrő#      A fájlpanelek tartalmát szűrhetjük.
                        ^<wrap>A ~Szűrők menü~@FiltersMenu@ témakör
részletezi a lehetőségeit.

   #Plugin parancsok#     A ~plugin parancsok~@Plugins@ listája.

   #Képernyők#            A megnyitott ~képernyők listája~@ScrSwitch@.

   #Futó programok#       A ~futó programok listája~@TaskList@.

   #Hotplug eszközök#     A ~hotplug eszközök listája~@HotPlugList@.


@OptMenu
$ #Menük: Beállítások menü#
   #Rendszer#             Megjeleníti a ~rendszer beállítások~@SystemSettings@
   #beállítások#          párbeszédablakot.

   #Panel beállítások#    A ~panel beállítások~@PanelSettings@ párbeszédablak.

   #Fastruktúra#          A ~fastruktúra beállítások~@TreeSettings@
   #beállítások#          párbeszédablakot jeleníti meg.

   #Kezelőfelület#        A ~kezelőfelület beállítások~@InterfSettings@
   #beállítások#          párbeszédablakot jeleníti meg.

   #Párbeszédablak#       A ~párbeszédablak beállítások~@DialogSettings@
   #beállítások#          párbeszédablakot jeleníti meg.

   #Nyelvek#              A program és a súgó nyelve választható ki.
                        ^<wrap>Használjuk a "Beállítások mentése" funkciót!

   #Plugin#               A ~pluginek~@Plugins@ működése állítható be, a pluginek
   #beállítások#          beállítási párbeszédablakaiban.

   #Megerősítések#        ^<wrap>Egyes műveletek végrehajtására
~megerősítés~@ConfirmDlg@ (rákérdezés) kapcsolható ki vagy be.

   #Fájlpanel módok#      ^<wrap>A
~fájlpanel nézet módok testreszabása~@PanelViewModes@ végezhető el a
funkcióval.

   #Fájl megjegyzés-#     Megadható, hogy a ~fájlok megjegyzéseit~@FileDiz@
   #fájlok#               mely fájlokból olvassa ki a FAR. Beállíthatók
                        a megjelenítés és frissítés jellemzői is.

   #Mappa megjegyzés-#    Megadható, hogy az ~info panel~@InfoPanel@ mely
   #fájlok#               fájlokat jelenítse meg mappa megjegyzésként
                        (~joker~@FileMasks@ karakter is megengedett).

   #Nézőke beállítások#   A külső és belső ~nézőke beállításai~@ViewerSettings@.

   #Szerkesztő#           A külső és belső ~szerkesztő beállításai~@EditorSettings@.
   #beállítások#

   #Színek#               ^<wrap>Kiválasztható minden egyes képernyőelem
színe, a FAR teljes palettája fekete-fehérre cserélhető vagy visszaállítható
az eredeti színkombináció.

   #Fájlkiemelések,#      A ~fájlkiemelések, rendezési csoportok~@Highlight@
   #rendezési csoportok#  beállításai.

   #Beállítások#          Elmenti a jelenlegi beállításokat, színeket
   #mentése#              és a képernyő elrendezését.


@ConfirmDlg
$ #Megerősítések#
    A #megerősítések# párbeszédablakban a következő műveletek rákérdezését
kapcsolhatjuk ki vagy be:

    - a célfájlok felülírása fájlok másolásakor;

    - a célfájlok felülírása fájlok áthelyezésekor;

    - fájlok ~húzd és ejtsd~@DragAndDrop@ mozgatása;

    - fájlok törlése;

    - nem üres mappák törlése;

    - művelet megszakítása;

    - ^<wrap>~hálózati meghajtók leválasztása~@DisconnectDrive@ a Meghajtók
menüből;

    - virtuális (SUBST) meghajtók törlése a Meghajtók menüből;

    - USB tárolóeszközök eltávolítása a Meghajtók menüből;

    - fájlok ~újratöltése~@EditorReload@ a szerkesztőbe;

    - a nézőke/szerkesztő, mappa és parancs előzménylisták törlése;

    - kilépés a FAR-ból.


@MakeFolder
$ #Új mappa#
    Ezzel a funkcióval új mappákat hozhatunk létre. Környezeti változókat is
megadhatunk a parancssorban, helyükön a mappa létrehozásakor az értékük fog
megjelenni. Egy lépésben hozhatunk létre mélyebbre ágyazott almappákat, ha a
mappák neveit #\\#-karakterrel választjuk el. Példa:

    #%USERDOMAIN%\\%USERNAME%\\Mappa3#

    Ha a #Több név feldolgozása# opciót engedélyezzük, egyszerre több mappát
készíthetünk. Ebben az esetben a mappák neveit "#;#" vagy "#,#" karakterrel kell
elválasztani. Ha a fent említett opciót engedélyeztük és a mappa nevében van
"#;#" (vagy "#,#") karakter, akkor a nevet idézőjelek közé kell tenni. Például
a következő soron Entert ütve:

    #C:\\Foo1;"E:\\foo,2;";D:\\foo3#

 a #C:\\Foo1#, az #E:\\foo,2;# és a #D:\\foo3# nevű mappák jönnek létre.


@FindFile
$ #Fájlkeresés#
    A parancs segítségével a megadott keresési feltételeknek megfelelő
fájlokat vagy mappákat kereshetünk meg a mappák fastruktúrájában.
Több ~joker~@FileMasks@ karakteres maszk is használható (vesszővel elválasztva)
és a keresés ~pluginnel~@Plugins@ emulált fájlrendszerekben is működik.

    Megadhatunk szöveget feltételként, ekkor csak a szöveget tartalmazó
fájlokat keresi meg, valamint bekapcsolható a #Nagy/kisbetű érzékeny# keresés
is.

    A #Csak egész szavak# opció csak akkor értékeli találatnak a megtalált
szövegrészeket, ha a megadott karaktersort a többitől a szóköz, tabulátor vagy
soremelés karakter választja el, vagy a szabványos határoló karakterek,
amelyek alapértelmezés szerint: #!%^&*()+|{}:"<>?`-=\\[];',./#.

    A #Keresés hexákra# opcióval hexadecimális számsorokat adhatunk meg
keresési feltételként. Ebben az esetben a #Nagy/kisbetű érzékeny#, a
#Csak egész szavak#, a #Kódlap# és a #Keresés mappákra# opciók
lehetőségét a FAR kikapcsolja és a korábban beállított értéküket sem veszi
figyelembe a keresés során.

    A #Kódlap# legördülő listájában a szöveg kereséséhez kiválaszthatunk
egy konkrét kódlapot, vagy megjelölhető a #Minden kódlappal# lehetőség is,
utóbbi esetben a FAR a szabványos és a #Kedvenc# kódlapok szerint keresi a
szövegeket a fájlokban. A #Kedvenc# kódlapokat a nézőke vagy a szerkesztő
kódlapválasztó párbeszédablakában (Shift-F8) jelölhetjük ki. Ha a
#Minden kódlappal# opciót választottuk, de a kódlapok kínálatát túlzóan
bőségesnek találjuk, az #Ins# vagy a #Space# billentyűvel leszűkíthetjük
a szabványos és #Kedvenc# kódlapok körét, így kizárólag a megjelölt
kódlapok szerint megy végbe a keresés.

    A #Keresés tömörítettekben# opció hatására a FAR a számára ismert
tömörítési formátumú archívumokba is belenéz, bár ez jelentősen csökkenti a
keresés hatékonyságát. A FAR nem tud keresni egymásba ágyazott tömörített
fájlokban.

    A #Keresés mappákra# opció a keresett nevek hatókörét kiterjeszti a
mappák neveire és siker esetén ezekkel is nő a találatszám.

    A #Keresés szimbolikus linkekben# opció hatására a keresés ugyanúgy
zajlik le a ~szimbolikus linkekben~@HardSymLink@, mintha szabályos almappák
lennének.

    #Keresés hatósugara#

    A keresés kiadható

    - minden fix meghajtóra;

    - ^<wrap>minden helyi meghajtóra, a kivehető és hálózati meghajtók
kivételével;

    - ^<wrap>A %PATH% környezeti változóban megadott összes mappára
(a belőlük nyíló almappákra nem);

    - ^<wrap>az aktuális meghajtó vagy a #Meghajtó# gombbal
definiált meghajtó gyökeréből nyíló összes mappára (a keresés
párbeszédablak lekérdezi az aktív panel meghajtójának betűjelét, ebből
adódik a #Meghajtó gyökerétől:# opció);

    - az aktuális mappára;

    - ^<wrap>csak az aktuális mappában vagy a kijelölt mappákban (a FAR
jelenlegi verziója nem keres olyan mappákban, amelyek
~szimbolikus linkek~@HardSymLink@).

    A keresés beállításai a többi beállítással mentődnek.

    A #Szűrővel# opció segítségével a felhasználó által megadott feltételeknek
megfelelő keresést végez. A #Szűrő# gomb lenyomásával a ~szűrők menüt~@FiltersMenu@
hívhatjuk be.

    A #Haladó# gomb meghívja a ~fájlkeresés haladó beállításai~@FindFileAdvanced@
párbeszédablakot, ahol bővíthető a keresési feltételek rendszere. A "Haladó"
opció gyors engedélyezését vagy tiltását a #Haladó beállítások# jelölőnégyzettel
végezhetjük el.


@FindFileAdvanced
$ #Fájlkeresés haladó beállításai#
    A #Tartalmazza a szöveget# (vagy a #Tartalmazza a hexát#) mezőben beírt
karaktersorozatot nem csak az egész fájlban, hanem a fájl elejének megadott
tartományában is kerestethetjük a #Keresés csak az első x bájtban# opcióval.
Ha a megadott érték kisebb a fájl méreténél, hiába tartalmazza a fájl
tartományon túli része a szöveget, oda nem terjed ki a keresés.

    A keresési tartomány mértékegységeként a következő utótagok használhatók:

    B - bájt (ha nincs utótag, az is bájtot jelent);
    K - kilobájt;
    M - megabájt;
    G - gigabájt;
    T - terabájt.

@FindFileResult
$ #Fájlkeresés: vezérlőgombok#
    A #Fájlkeresés# ablakban - akár ~keresés~@FindFile@ közben, akár annak
befejeztével - a kurzorvezérlő billentyűkkel görhethetjük a találatok listáját,
vagy lépkedhetünk a műveleti gombokon és aktiválhatjuk őket.

    Keresés közben vagy a keresés után a következő gombok használhatók:

    #Új keresés#      Új keresést indít el.

    #Ugrás#           ^<wrap>Megszakítja a keresést (ha még tart), átvált a
mappák paneljére és a kurzort a kiválasztott fájlra állítja.

    #Megnéz#          ^<wrap>Megnézi a kiválasztott fájlt. Ha a keresés még
nincs kész, a fájl megnézése közben a keresés a háttérben folytatódik.

    #Panel#           ^<wrap>A megtalált fájlok listáját ideiglenes panelen
jeleníti meg.

    #Állj#            ^<wrap>Megszakítja a keresést. Csak a keresés folyamán
aktív.

    #Mégsem#          Bezárja a keresés párbeszédablakát.

    Az #F3# és az #F4# a megtalált fájlok megnézésére és szerkesztésére
szolgál és pluginnel emulált fájlrendszerekben is használható. Érdemes
megjegyezni, hogy ha emulált fájlrendszerben mentjük a szerkesztés változásait
az #F2#-vel, egyszerű #Mentés# helyett #Mentés másként# művelet történik.


@FindFolder
$ #Mappakeresés#
    A paranccsal gyorsan megtalálhatjuk a mappákat a fastruktúrában.

    A mappákat kiválaszthatjuk a kurzorvezérlő billentyűkkel vagy nevük
karaktereinek begépelésével, joker karakterek is használhatók.

    Az #Enter# lenyomásával a kiválasztott mappára ugorhatunk.

    A #Ctrl-R# vagy az #F2# újraolvassa a fastruktúrát.

    A #Szürke +# és #Szürke -# fel-le léptet a jelenlegi mappával azonos
szinten lévő mappákon.

    Az #F5# a keresőablak teljes és eredeti mérete közt váltogat.

    A #Ctrl-Enter# azokon a mappákon lépked lefelé, amelyeknek neve az addig
begépelt karaktereknek megfelel. A #Ctrl-Shift-Enter# hasonlóan működik, de
felfelé léptet.

@Filter
$ #Szűrő#
   A műveleti szűrővel az általunk megadott szűrőfeltételeknek megfelelő
fájlok csoportján végeztethetjük el a kívánt műveletet. Egy-egy szűrő több
különböző szabálykészletet is tartalmazhat.

   A Szűrő párbeszédablak elemei:

   #Szűrő neve#      ^<wrap>A szűrő neve, ami majd a szűrő menüben látszik.
Ez a mező üres is lehet.

                   ^<wrap>A szűrők neve nem érhető el, ha a szűrőt a
~Fájlkiemelések, rendezési csoportok~@Highlight@ menüből nyitottuk meg.

   #Maszk#           Egy vagy több ~fájlmaszk~@FileMasks@.

                   ^<wrap>A szűrőfeltételek akkor teljesülnek, ha a
fájlmaszkelemzés be van kapcsolva és a név megfelel valamelyik maszknak. Ha a
maszkok elemzése ki van kapcsolva, a fájl nevét a FAR nem veszi figyelembe.

   #Méret#           A fájlméret minimális és maximális értéke.
                   A következő fájlméret utótagok használhatók:

                   B - bájt (ha nincs utótag, az is bájtot jelent);
                   K - kilobájt;
                   M - megabájt;
                   G - gigabájt;
                   T - terabájt.

                   ^<wrap>A szűrőfeltételek akkor teljesülnek, ha a
fájlméretelemzés be van kapcsolva és a fájl mérete a tartományon belül esik.
Ahol valamelyik értéket nem korlátozzuk (üresen hagyott sorral), ott a fájl
mérete bármilyen értéket felvehet.

   #Dátum/Idő#       A fájl dátumának/idejének tartománya.
                   ^<wrap>Választhatunk az utolsó #módosítás#, a #létrehozás#
vagy a #hozzáférés# dátuma és időpontja közt.

                   ^<wrap>A #Jelenlegi# gomb az aktuális dátummal és idővel
tölti fel a dátum/idő mezőt, ezután a mezők értékeit módosíthatjuk, például
akár külön a hónap vagy a perc értékét is. Az #Üres# gomb törli a dátum- és
időmezők tartalmát.

                   ^<wrap>A szűrőfeltételek akkor teljesülnek, ha a
dátum/időanalízis be van kapcsolva és a fájl dátuma/ideje a kiválasztott
időtípus (módosítás, létrehozás vagy hozzáférés) megadott tartományán belül
van. Ha egyik vagy mindkét időmezőt üresen hagyjuk, a kitöltetlen sor nem lesz
szűrőfeltétel.

   #Attribútumok#    Befoglaló és kizáró attribútumok.

                   ^<wrap>A szűrőfeltételek akkor teljesülnek, ha az
attribútumelemzés be van kapcsolva és a fájl minden megadott befoglaló
attribútummal rendelkezik, de nincs egyetlen kizáró attribútuma sem:

                   #[x]# - ^<wrap>befoglaló attribútum - a fájlnak
rendelkeznie kell az attribútummal;
                   #[ ]# - kizáró attribútum - a fájlnak nem
lehet ilyen attribútuma;
                   #[?]# - az attribútum értéke nem számít.

                   ^<wrap>A #Tömörített#, #Titkosított#, #Nem indexelt#,
#Ritkított#, #Átmeneti# és #Offline# attribútum csak NTFS fájlrendszerű
lemezeken létezik. A #Virtuális# attribútumot csak a Windows Vista/2008
operációs rendszerek használják.

   A megfelelő jelölőnégyzetekkel egyszerűen tilthatunk le vagy
engedélyezhetünk egy-egy szűrőfeltételt, a #Reset# gomb pedig minden
szűrőfeltételt töröl.


@History
$ #Parancs előzmények#
    A Parancs előzmények menü a korábban végrehajtott parancsok
  listáját jeleníti meg. A listán a kurzorvezérlőkön kívül a
  következő billentyűket használhatjuk:

  Előzőleg kiadott parancs végrehajtása                         #Enter#

  Előzőleg kiadott parancs végrehajtása új ablakban       #Shift-Enter#

  Parancs másolása a parancssorba                          #Ctrl-Enter#

  Az előzménylista törlése                                        #Del#

  Az aktuális előzményelem törlése                          #Shift-Del#

  Zárolja vagy megengedi az előzményelem módosítását              #Ins#

  A kiválasztott parancsot a vágólapra másolja,                #Ctrl-C#
    a lista legördítve marad                            vagy #Ctrl-Ins#

    Ha az előző vagy a következő parancsot közvetlenül a
  parancssorból szeretnénk meghívni, használjuk a #Ctrl-E# vagy
  a #Ctrl-X# billentyűket.

    Ha parancsot szeretnénk választani a listából, a kurzorvezérlőkön
  és az #Enteren# kívül használhatjuk közvetlenül a parancs kiemelt
  betűjelét is.

    Ha azt szeretnénk, hogy a FAR kilépéskor elmentse a parancsok
  előzményét, jelöljük be a megfelelő opciót a ~Rendszer beállítások~@SystemSettings@
  párbeszédablakban.

    A zárolt előzményelemek nem törlődnek az előzménylista módosulása
  vagy törlése esetén sem.


@HistoryViews
$ #Előzmények: megnézett és szerkesztett fájlok előzménye#
    A "Fájl előzmények" az utoljára megnézett vagy szerkesztett
  fájlok listáját mutatja, elérési útvonalukkal. A kurzorvezérlőkön
  kívül a következő billentyűkombinációkat használhatjuk:

  Újbóli megnyitás a nézőkében vagy a szerkesztőben             #Enter#

  Fájlnév másolása a parancssorba                          #Ctrl-Enter#

  Előzménylista törlése                                           #Del#

  Az aktuális előzményelem törlése                          #Shift-Del#

  Zárolja vagy megengedi az előzményelem módosítását              #Ins#

  Lista frissítése, a már nem élő bejegyzések törlése          #Ctrl-R#

  A lista kiválasztott elemének vágólapra másolása,            #Ctrl-C#
    a lista legördítve marad                            vagy #Ctrl-Ins#

  Fájl megnyitása a ~szerkesztőben~@Editor@                                  #F4#

  Fájl megnyitása a ~nézőkében~@Viewer@                                      #F3#
                                                        vagy #Numpad 5#

    A lista fájljainak megnyitását a kurzorvezérlőkön és az #Enteren#
  kívül elvégezhetjük közvetlenül a kiemelt betűjelükkel is.

    Az előzménylista aktivizált elemei a lista végére kerülnek. Ezt a
  hatást elkerülhetjük, ha a #Shift-Enterrel# nyitjuk meg őket.

    Ha azt szeretnénk, hogy a FAR kilépéskor elmentse a fájlok
  előzményeit, jelöljük be a megfelelő opciót a ~Rendszer beállítások~@SystemSettings@
  párbeszédablakban.

  Megjegyzések:

     1. A lista frissítése (Ctrl-R) hosszú időt vehet igénybe, ha
        jelenleg nem elérhető távoli helyeket kell vizsgálnia.

     2. A zárolt előzményelemek nem törlődnek az előzménylista
        módosulása vagy törlése esetén sem.


@HistoryFolders
$ #Előzmények: mappa előzmények#
    A mappák előzménye a korábban bejárt mappák listáját jeleníti
  meg. A kurzorvezérlőkön kívül a következő billentyűkombinációkat
  használhatjuk:

  Belépés a listán kiválasztott mappába                         #Enter#

  Kiválasztott mappa megnyitása a passzív panelen    #Ctrl-Shift-Enter#

  Mappa nevének másolása a parancssorba                    #Ctrl-Enter#

  Előzménylista törlése                                           #Del#

  Az aktuális előzményelem törlése                          #Shift-Del#

  Zárolja vagy megengedi az előzményelem módosítását              #Ins#

  Lista frissítése, a már nem élő bejegyzések törlése          #Ctrl-R#

  A kiválasztott mappa nevét a vágólapra másolja,              #Ctrl-C#
  a lista legördítve marad                              vagy #Ctrl-Ins#

    A lista mappáinak megnyitására a kurzorvezérlőkön és az #Enteren#
  kívül használatjuk a kiemelt betűjelüket is.

    Az előzménylista újraaktivált elemei a lista végére kerülnek. Ezt
  a hatást elkerülhetjük, ha a #Shift-Enterrel# nyitjuk meg őket.

    Ha azt szeretnénk, hogy a FAR kilépéskor elmentse a mappák
  előzményeit, jelöljük be a megfelelő opciót a ~Rendszer beállítások~@SystemSettings@
  párbeszédablakban.

  Megjegyzések:

    1. A lista frissítése (Ctrl-R) hosszú időt vehet igénybe, ha
       pillanatnyilag nem elérhető távoli helyeket kell vizsgálnia.

    2. A zárolt előzményelemek nem törlődnek az előzménylista
       módosulása vagy törlése esetén sem.


@TaskList
$ #Futó programok#
    A Futó programok menü a jelenleg működő programokat listázza ki. A lista
sorai megegyeznek a futó programok ablakainak megnevezéseivel.

    A listán átválthatunk a futó program ablakára vagy a programokat
"kilőhetjük" a memóriából a #Del# billentyűvel. Utóbbi művelettel bánjunk
óvatosan, mert azonnal leállítja a kiválasztott program futását és annak
minden elmentetlen adata elvész, ezért ezt a funkciót csak végszükség esetén
használjuk, például a nem válaszoló programoknál.

    A Futó programokat a ~Parancsok menüből~@CmdMenu@ vagy a #Ctrl-W#
billentyűkombinációval hívhatjuk meg. A #Ctrl-W# kombináció a nézőkében és a
szerkesztőben is működik.

    A #Ctrl-R# frissíti a programok listáját.


@HotPlugList
$ #Hotplug eszközök#
    A Hotplug eszközök menü a PC kártyaolvasók és a számítógéphez
csatlakoztatott egyéb analóg eszközök listáját jeleníti meg.

    A leválasztandó eszköz nevét ki kell választani a listából és #Del#-t ütni
rajta. Ezután a Windows gondoskodik az eszköz biztonságos eltávolításáról és
értesítést kapunk, ha az eltávolítás már biztosan nem jár adatvesztéssel.

    A #Ctrl-R# frissíti a csatlakozó eszközök listáját.


@CompFolders
$ #Mappák összehasonlítása#
    A Mappák összehasonlítása parancs csak akkor használható, ha mindkét
megjelenített panel ~fájlpanel~@FilePanel@ típusú. A parancs összehasonlítja
a két panel mappáinak tartalmát. Azok a fájlok kapnak jelölést, amelyek csak
az egyik panelen léteznek, vagy a dátumuk újabb, mint a másik panel mappájában
az azonos nevű fájloké.

    A parancs az almappák neveit és tartalmát nem hasonlítja össze, a fájlokat
is csupán nevük, méretük és dátumuk alapján, így tartalmi eltéréseik nem hatnak
a művelet eredményére.


@UserMenu
$ #Felhasználói menü#
    A Felhasználói menüvel a sűrűn használt parancsokat egyszerűbben hívhatjuk
meg. Számos általunk definiálható parancs és parancssorozat létezik, amit a
felhasználói menüből hajthatunk végre. A menü tartalmazhat almenüket is. A
~különleges szimbólumok~@MetaSymbols@ a parancsokban és a parancsok nevében is
használhatók. Jegyezzük meg, hogy a #!?<név>?<alapérték>!# szimbólum
segítségével olyan párbeszédablakot készíthetünk, amelynek beviteli mezőiben
közvetlenül a végrehajtás előtt paraméterezhetjük a parancsokat.

    A ~Parancsok menü~@CmdMenu@ #Felhasználói menü szerkesztése# menüpontjában
szerkeszthetjük vagy létrehozhatjuk fő és helyi felhasználói menüinket.
Főmenüből csak egy lehet, helyi menüje bármelyik mappának lehet. Ha egy
mappának nincs helyi menüje, F2-re a főmenü aktivizálódik. A fő- és a helyi
menük között #Shift-F2#-vel váltogathatunk. A #BackSpace#-szel a helyi menüből
visszafelé haladhatunk a főmenü felé.

    A felhasználói menü parancsai vagy almenüi közé választóvonalat is
tehetünk: az #Ins# billentyűvel szúrjunk be egy új parancsot vagy menüt,
adjunk #-# értéket a gyorsbillentyűnek és a többi mezőt hagyjuk üresen. A
létrehozott választóvonalakat törölhetjük, ha az #Alt-F4#-gyel fájl módba
kapcsolunk és a szerkesztőben eltávolítjuk a felesleges #-:# sorokat.

    A felhasználói menü parancsait úgy hajthatjuk végre, ha kiválasztjuk a
kurzorvezérlő billentyűvel és #Enter#-t ütünk rajta, vagy a hozzárendelt
gyorsbillentyű segítségével.

    Az #Ins# billentyűvel új menüelemeket, almenüket hozhatunk létre, az #F4#
bilentyűvel szerkeszthetjük a jellemzőiket, a #Del#-lel pedig törölhetjük a
menü elemeit. #Alt-F4#-gyel a menüket szövegfájl formátumban szerkeszthetjük.

    A felhasználói menü elemeinek gyorsbillentyűjeként megadhatunk számokat,
betűket vagy funkcióbillentyűket (#F1#..#F12#), utóbbiakat csak beírás útján
fogadja el. Ha az #F1# vagy #F4# eredetileg más művelethez volt rendelve,
a felhasználói menü felülbírálja eredeti funkcióikat, bár a #Shift-F4#-gyel
ezután is szerkeszthetjük a menüt.

    Ha a szerkesztett menüelemünk parancs, meg kell adnunk a gyorsbillentyűjét,
a nevét (ami majd a menüben jelenik meg) és a végrehajtandó parancs sorát.

    Almenü szerkesztésénél csak a gyorsbillentyűt és a nevet kell megadni.

    A helyi felhasználói menü az adott mappában egy #FarMenu.Ini# nevű fájlba
mentődik. A főmenüt a FAR alaphelyzetben a regisztrációs adatbázisba menti, de
fájlba is menthető. Ha helyi menüt készítettünk a FAR mappájában, a FAR ezt
fogja használni a registrybe mentett főmenü helyett.

    A #Shift-F10# azonnal bezárja a felhasználói menü ablakát, akár főmenü,
akár almenü volt megnyitva benne.


@FileAssoc
$ #Fájltársítások#
    A FAR Manager támogatja a fájlok társításait, így a megadott
~maszkoknak~@FileMasks@ megfelelő fájlokhoz különféle műveleteket társíthatunk,
amelyekkel megnézhetjük vagy szerkeszthetjük őket.

    A ~Parancsok menü~@CmdMenu@ #Fájltársítások# menüpontjában új
összerendeléseket definiálhatunk.

    Azonos fájltípushoz is többféle műveletet rendelhetünk és a szükséges
társításokat menüből választhatjuk ki.

    A társítások menüjében a következő szerkesztőfunkciókat használhatjuk:

    #Ins#        - Új ~társítás~@FileAssocModify@ létrehozása

    #F4#         - Meglévő társítás ~szerkesztése~@FileAssocModify@

    #Del#        - Jelenlegi társítás törlése

    Ha egy fájltípushoz nincs definiált társítás és a
~Rendszer beállítások~@SystemSettings@ menüben a #Windows regisztrált#
#fájltípusainak használata# opció be van kapcsolva, a FAR megpróbálja a
Windows társításait alkalmazni.


@FileAssocModify
$ #Fájltársítások: szerkesztés#
    A FAR minden ~maszkkal~@FileMasks@ definiált fájltípusához hat
végrehajtható parancsot lehet hozzárendelni:

    #Végrehajtandó parancs#         #Enter#-re indul
    #(Enterre)#

    #Végrehajtandó parancs#         #Ctrl-PgDn#-ra indul
    #(Ctrl-PgDn-ra)#

    #Nézőke parancs#                #F3#-ra indul
    #(F3-ra)#

    #Nézőke parancs#                #Alt-F3#-ra indul
    #(Alt-F3-ra)#

    #Szerkesztés parancs#           #F4#-re indul
    #(F4-re)#

    #Szerkesztés parancs#           #Alt-F4#-re indul
    #(Alt-F4-re)#

    A társítások tulajdonságait néhány szóval jellemezhetjük
#A társítás leírása# mezőben.

    Ha nem szeretnénk, hogy a társított program végrehajtása előtt a FAR
paneljei kikapcsolódjanak, kezdjük a parancssort #@@# karakterrel.

    A parancssorban ~különleges szimbólumokat~@MetaSymbols@ is használhatunk.

  Megjegyzések:

  1. ^<wrap>Ha egy fájltípushoz nincs definiált társítás és a
~Rendszer beállítások~@SystemSettings@ menüben a #Windows regisztrált#
#fájltípusainak használata# opció be van kapcsolva, a FAR megpróbálja a
Windowsban definiált társításokat alkalmazni.

  2. ^<wrap>Az operációs rendszer "IF EXIST" és "IF DEFINED"
~parancsaival~@OSCommands@ a társításoknak kifinomultabb feltételrendszert
szabhatunk. Ha azonos fájltípushoz több különböző társítást adtunk meg, az
említett szabályok hatására a menüben csak a feltételeknek megfelelő
társítások jelennek meg.


@MetaSymbols
$ #Különleges szimbólumok#
    A ~fájltársításoknál~@FileAssoc@, a ~Felhasználói menüben~@UserMenu@ és a
~Parancs végrehajtása~@ApplyCmd@ menüben a következő különleges szimbólumokat
használhatjuk:

    #!!#       "!" karakter
    #!#        Hosszú fájlnév, kiterjesztés nélkül
    #!~~#       Rövid fájlnév, kiterjesztés nélkül
    #!`#       Hosszú kiterjesztés fájlnév nélkül (ext)
    #!`~~#      Rövid kiterjesztés fájlnév nélkül (ext)
    #!.!#      Hosszú fájlnév, kiterjesztéssel
    #!-!#      Rövid fájlnév, kiterjesztéssel
    #!+!#      ^<wrap>Hasonló a !-!-hoz, de ha a hosszú fájlnév elveszett
a parancs végrehajtása után, a FAR visszaállítja
    #!@@!#      A fájl neve, a kijelölt fájlnevek listájával
    #!$!#      A fájl neve, a kijelölt rövid fájlnevek listájával
    #!&#       A kijelölt fájlok listája
    #!&~~#      A kijelölt rövid fájlnevek listája
    #!:#       Az aktuális meghajtó, "C:" formátumban, távoli mappáknál
"\\\\server\\share"
    #!\\#       Az aktuális elérési útvonal
    #!/#       Az aktuális elérési út rövid neve
    #!=\\#      Az aktuális elérési út, a ~szimbolikus linkeket~@HardSymLink@
is figyelembe véve.
    #!=/#      Az aktuális elérési út rövid neve,
a ~szimbolikus linkeket~@HardSymLink@ is figyelembe véve.
    #!?<név>?<alapérték>!#   A parancs végrehajtásakor e szimbólum helyén
egy felhasználói beviteli ablak jelenik meg. A <név> a beviteli mező neve, az
<alapérték> pedig a szerkesztőmezőbe eredetileg kerülő karaktersor.

             Több ilyen szimbólum lehet egy sorban, például:

               grep !?Keresés:?! !?Maszk:?*.*!|c:\\far\\far.exe -v -

             ^<wrap>A <név> mező kiegészülhet az <alapérték> sztringre
vonatkozó előzménnyel (az <előzmény> változó nevével).
             Ebben az esetben a parancssor formátuma:

               #!?$<előzmény>$<név>?<alapérték>!#

             Példa:

               grep !?#$GrepHist$#Keresés:?! !?Maszk:?*.*!|far.exe -v -

             ^<wrap>A <név> és az <alapérték> beírásánál más különleges
szimbólumot is használhatunk, zárójelek között.

             Példa:

               grep !?Maszk (!.!):?! |far.exe -v -)

    #!###       ^<wrap>A fájlokra hivatkozó szimbólumok elé írt "!##"
módosítóval elérhetjük, hogy a szimbólum (és minden utána következő karakter)
a passzív panelre vonatkozzon (lásd 4-es számú megjegyzés). Például a !##!.! a
passzív panel aktuális fájlnevére utal.

    #!^#       ^<wrap>A fájlokra hivatkozó szimbólumok elé írt "!^" módosító
hatására a szimbólum (és az összes azt követő karakter) az aktív panelre
vonatkozik (lásd 4-es számú megjegyzés). Például a !^!.! szimbólum az aktív
panel aktuális fájljára utal, a !##!\\!^!.! pedig a passzív panelen lévő fájl
nevére és az aktív panel aktuális, azonos nevű fájljára utal.

    Megjegyzések:

    1. ^<wrap>A FAR a különleges szimbólumok kezelésénél kizárólag a
szimbólumnak megfelelő sztringet helyettesíti be, semmiféle további karaktert
(például idézőjelet) nem tesz hozzá, így ha szükséges, erről nekünk kell
gondoskodni. Például ha egy program idézőjelek közé tett fájlmaszkot igényel,
akkor #program.exe !.!# helyett #program.exe "!.!"# kifejezést kell írni.

    2. A !@@! és !$! szimbólummal a következő módosítók használhatók:

         Q - a szóközöket tartalmazó neveket idézőjelek közé teszi;
         S - elérési utaknál '/' jelet használ '\\'-helyett;
         F - teljes elérési utat használ;
         A - ANSI kódolást használ.

       ^<wrap>Például a #!@@AFQ!# azt jelenti, hogy "fájlnév a kijelölt fájlok
neveinek listájával, ANSI kódolással, teljes elérési útvonalakkal, szóközt
tartalmazó fájlnevek idézőjelben".

    3. ^<wrap>Ha összetett hivatkozást adtunk meg, a !@@! és !$! metakarakter
eredeti alakjában jelenik meg a menüben. Ezeket a FAR a parancs
végrehajtásakor fogja értelmezni.

    4. ^<wrap>A "!##" és a "!^" előtag a hivatkozások paneloldali
átkapcsolójaként szolgál. Az ilyen előtag hatóköre a sorban utána következő,
szintén átkapcsoló előtagig terjed.

       Például:

         if exist !##!\\!^!.! diff -c -p !##!\\!^!.! !\\!.!

       ^<wrap>"Ha a passzív panelen létezik ugyanolyan nevű fájl, mint amin az
aktív panel sávkurzora áll, mutassa meg a két fájl különbségét, függetlenül
attól, hogy a passzív panelen mi a jelenleg aktív fájl neve."

    5. ^<wrap>Ha valamelyik program a név megadásánál lezáró \\-jelet igényel,
használjuk a #!.\# metaszimbólumot. Például, ha egy RAR-ral tömörített fájlt
szeretnénk a fájllal azonos nevű mappába kibontani, a parancs:

         winrar x "!.!" "!.\"

@SystemSettings
$ #Beállítások: rendszer beállítások#
  #Törlés a Lomtárba#       ^<wrap>A fájlok vagy mappák törlésénél
közbeiktatja a Lomtárat. A "Törlés a Lomtárba" művelet csak helyi
merevlemezeken működik.

  #Szimbolikus linkek#      Megkeresi és törli a mappák szimbolikus
  #törlése#                 linkjeit, mielőtt a Lomtárba dobná őket.

  #Másoláshoz a rendszer-#  A FAR beépített másolórutinja helyett az
  #rutin használata#        operációs rendszer rutinját használja.
                          ^<wrap>Alkalmazása NTFS fájlrendszerben hasznos
lehet, mert a CopyFileEx rendszerfunkció ésszerűbb lemezfoglalási módszert
használ, azonkívül a fájlokat bővített attribútumkészletükkel együtt másolja
át. Másrészt viszont az operációs rendszer metódusa meggátolja a fájlok
feloszthatóságát, ha a ~másolás~@CopyFiles@ vagy mozgatás nem használható.

  #Írásra megnyitott#       Megengedi más programokban írásra
  #fájlok másolhatók#       megnyitott fájlok másolását.
                          ^<wrap>A módszer praktikus lehet a hosszú időre
megnyitott fájlok másolására, de veszélyessé is válhat, ha a fájl a másolás
ideje alatt módosul.

  #Szimbolikus linkek#      Ha ez az opció be van kapcsolva, akkor a
  #vizsgálata#              a mappák fastruktúrájának feltérképezése
                          ^<wrap>során a normál mappák mérete, valamint a
~szimbolikus linkjeik~@HardSymLink@ mérete együttesen fogják meghatározni a
mappákban található fájlok méretének összegét.

  #Mappák létrehozása#      Ha az új mappa nevét csupa kisbetűvel
  #NAGYBETŰKKEL#            írjuk be és ez az opció be van kapcsolva,
                          a mappa neve nagybetűs lesz.

  #A FAR kilép x perc#      A FAR futása abbamarad, ha a megadott
  #tétlenség után#          időtartam alatt nem történik billentyű-
                          vagy egérművelet.
                          ^<wrap>A funkció csak akkor működik, ha a FAR-nak
csupán parancssori bevitelre kell várnia és nincs a háttérben megnyitott
nézőke vagy szerkesztő.

  #Parancs előzmények#      A FAR kilépés előtt elmenti, indításnál
  #mentése#                 visszatölti a ~parancs előzményeket~@History@.

  #Mappa előzmények#        A FAR kilépés előtt elmenti, indításnál
  #mentése#                 visszatölti a ~mappa előzményeket~@HistoryFolders@.
                          ^<wrap>A mappa előzmények listája az #Alt-F12#-vel
is megjeleníthető.

  #Nézőke és szerkesztő#    A FAR kilépés előtt elmenti, indításnál
  #előzmények mentése#      pedig betölti a ~nézőke és a szerkesztő~@HistoryViews@
                          ~előzményeit~@HistoryViews@.
                          ^<wrap>Az előzmények listáját az #Alt-F11#-gyel is
megjeleníthetjük.

  #Windows regisztrált#     Ha az opció be van kapcsolva és #Enter#-t
  #fájltípusok használata#  ütünk egy olyan típusú fájlon, amit a
                          ^<wrap>Windows ismer és a típus nem szerepel a FAR
~fájltársítások~@FileAssoc@ listáján, a Windows a saját társítású programjával
próbálja megnyitni.

  #CD tálca automatikus#    Ha CD-ROM típusú meghajtót választottunk a
  #behúzása#                ~Meghajtók menüben~@DriveDlg@, a FAR megpróbálja
                          behúzni a meghajtó nyitott tálcáját.
                          ^<wrap>Kapcsoljuk ki az opciót, ha nem működik
megfelelően (néhány CD-ROM meghajtó hibás drivere miatt ez előfordulhat).

  #Saját pluginek#          Itt adhatjuk meg "saját" pluginjeink
  #útvonala#                mappáinak elérési útvonalait, ahol a
                          ^<wrap>FAR-nak a "fő" plugineken túl modulokat
kell keresnie. Több útvonalat beírhatunk, ";"-vel elválasztva, környezeti
változók is használhatók. A saját pluginek nem töltődnek be, ha a FAR a /p
vagy /co ~parancssori~@CmdLine@ kapcsolóval indul.

  #Beállítások automatikus# Ha az opció be van kapcsolva, kilépéskor
  #mentése#                 a FAR önműködően menti a beállításait, a
                          panelek aktuális helyzetével együtt.


@PanelSettings
$ #Beállítások: panel beállítások#
  #Rejtett és rendszer-#    Megjeleníti a rejtett és rendszer
  #fájlok mutatva#          attribútumú fájlokat. Ez az opció
                          a #Ctrl-H#-val is átkapcsolható.

  #Fájlok kiemelése#        A ~fájlkiemelések~@Highlight@ engedélyezése.

  #A mappák is#             A #Szürke +# és a #Szürke *# nem csak a
  #kijelölhetők#            fájlokat, hanem a mappákat is kijelöli.
                          ^<wrap>Kikapcsolt opciónál csak a fájlok kapnak
jelölést.

  #Mappák is rendezhetők#   A kiterjesztés szerinti rendezés nem csak
  #kiterjesztés szerint#    fájlokra, hanem mappákra is lehetséges.
                          ^<wrap>Bekapcsolt opciónál a FAR 1.65-ös
verziójában alkalmazott rendezés lesz érvényes. Kikapcsolt opciónál a
mappákat akkor is név szerint rendezi, ha a fájlokat kiterjesztésük szerint.

  #Fordított rendezés#      Bekapcsolt opciónál az adott rendezési
  #engedélyezése#           elv másodszori aktiválása megfordítja
                          a rendezés irányát.

  #Panelek automatikus#     Korlátozó funkció: ha a mappában a
  #frissítése kikapcsolva,# fájlok száma meghaladja a megadott
  #ha több elem van, mint:# értéket, a fájlszerkezet változása
                          nem vonja maga után a panel automatikus
                          frissítését.

                          ^<wrap>Az automatikus frissítés csak FAT,
FAT32 és NTFS fájlrendszerben működik. A "0" érték azt jelenti, hogy
"mindig frissít". A frissítés kézzel is elvégezhető a #Ctrl-R#-rel.

  #Hálózati meghajtók#      Engedélyezi a panelek automatikus
  #autom. frissítése#       frissítését, ha a hálózati meghajtók
                          fájlrendszerének állapota megváltozik.
                          ^<wrap>Lassú hálózatoknál célszerűbb lehet az
opciót kikapcsolni.

  #Oszlopnevek mutatva#     ^<wrap>Megjeleníti a ~fájlpanelek~@FilePanel@
oszlopainak neveit.

  #Állapotsor mutatva#      Megjeleníti a fájlpanel állapotsorát.

  #Fájlok összes#           Megjeleníti a mappa fájljainak számát és
  #információja mutatva#    méretösszegét a fájlpanel alsó sorában.

  #Szabad lemezterület#     Megjeleníti az aktuális meghajtó
  #mutatva#                 szabad lemezterületének méretét.

  #Gördítősáv mutatva#      ^<wrap>Megjeleníti a fájl- és ~fa panel~@TreePanel@
oldalsó gördítősávját.

  #Háttérképernyők száma#   Megmutatja a ~háttérképernyők~@ScrSwitch@
  #mutatva#                 számát.

  #Rendezési mód betűjele#  Megmutatja az aktuális rendezési elv
  #mutatva#                 betűjelét a bal felső sarokban.


@TreeSettings
$ #Fastruktúra beállítások#
  #Automatikus#             Ha engedélyezett, a ~fastruktúra panelen~@TreePanel@
  #mappaváltás#             a kurzor mozgatására a másik panel is
                          ^<wrap>mappát vált. Ha nincs engedélyezve,
a fastruktúrán a mappaváltáshoz #Entert# kell ütni.


@InterfSettings
$ #Beállítások: kezelőfelület beállítások#

  #Óra a paneleken#         ^<wrap>Megjeleníti az órát a képernyő jobb felső
sarkában.

  #Óra a nézőkében#         Megjeleníti az órát a nézőkében és a
  #és a szerkesztőben#      szerkesztőben is.

  #Egér kezelése#           A FAR egérrel is vezérelhető.

  #Funkcióbillentyűk#       Megjeleníti a funkcióbillentyűk sorát a
  #sora mutatva#            ^<wrap>képernyő alján. Ez az opció #Ctrl-B#-vel
is átváltható.

  #A menüsor mindig#        A felső menüsor mindig látható, akkor
  #látszik#                 is, ha nem aktív.

  #Képernyőpihentető X#     Elindul a képernyőpihentető, ha nem
  #perc tétlenség után#     történt egér- vagy billentyűművelet
                          a percben megadott időtartam alatt.
                          ^<wrap>Ha az opció be van kapcsolva, a pihentető
akkor is elindul, ha a FAR képernyő jobb felső sarka fölé visszük az egér
kurzorát.

  #Parancssori prompt#      Itt állíthatjuk be a FAR alapértelmezett
  #formátuma#               ~parancssori prompt~@CommandPrompt@ formátumát.

  #Másolás összesen#        Fájlok másolása során nem csak az egyes
  #folyamatjelző#           fájlra mutatja meg analóg sávon, hogy hol
                          tart a folyamat, hanem az összes fájlra.
                          ^<wrap>Valamivel több időt igényelhet a másolások
beindulása, mivel folyton figyelnie kell a fájlok összméretét.

  #Másolási idő mutatva#    Tájékoztat az átlagos másolási sebességről,
                          ^<wrap>a másolás eltelt idejéről és a becsült
hátralévő időről a Másolás párbeszédablakban.

                          ^<wrap>Mivel a becslések kiszámítása némi időt
igényel, ezért ha a "Másolás összesen folyamatjelző" ki van kapcsolva és
sok apró fájlt kell másolni, valószínűleg semmilyen információ nem jelenik
meg.

  #A Ctrl-PgUp#             A #Ctrl-PgUp# leütése egy meghajtó
  #meghajtót vált#          gyökérmappájában:
                        - ^<wrap>helyi meghajtónál megjeleníti a Meghajtók
menüt;
                        - hálózati meghajtónál elindítja a Hálózat
plugint (ha lehetséges) vagy meghívja a Meghajtók menüt (ha a Hálózat plugin
nem elérhető).

@DialogSettings
$ #Beállítások: párbeszédablak beállítások#
  #Beviteli sorok#          Megőrzi egyes FAR párbeszédablakok
  #előzménykövetése#        beviteli sorainak előzményeit.
                          ^<wrap>Az előzőleg beírt sztringek listáját
legördíthetjük az egérrel vagy a #Ctrl-Fel# és #Ctrl-Le# billentyűvel. Ha nem
szeretnénk ezt a fajta előzménykövetést használni (például biztonsági
megfontolásból), kapcsoljuk ki.

  #Maradó blokkok a#        Nem veszi le a kijelölt szövegekről a
  #beviteli sorokban#       kijelölést, ha a kurzort megmozdítjuk;
                          ^<wrap>sem a párbeszédablakok beviteli soraiban,
sem a párbeszédablakok parancssoraiban.

  #A Del törli a bevite-#   Ha van kijelölt szövegrész, a Del nem a
  #li sorok blokkjait#      kurzor alatti karaktert, hanem előbb a
                          kijelölt szöveget törli.

  #Beviteli sorok auto-#    Engedélyezi az előzménylistával rendelkező
  #matikus kiegészítése#    szövegbeviteli sorok és lenyíló ablakok
                          önműködő kiegészítését beírásnál.
                          ^<wrap>Ha az opciót letiltottuk, a #Ctrl-End#-del
ideiglenesen feloldható a tiltás egy-egy sorra. A makrók rögzítése vagy
végrehajtása átmenetileg letiltja az automatikus kiegészítést.

  #A Backspace törli a#     Ha engedélyezzük, a #BackSpace# ugyanúgy
  #változatlan szöveget#    kitörli a beviteli mezőkben az egész vál-
                          tozatlan szöveget, mint a #Del#.

  #Egérkattintás a párb.#   Ha a #bal/jobb egérgombot# lenyomjuk egy
  #ablakon kívül=bezárja#   párbeszédablak területén túl, bezárul az
                          ^<wrap>ablak (lásd ~egyebek~@MiscCmd@). Az
opcióval letilthatjuk ezt a működést.


@CommandPrompt
$ #A parancssori prompt formátuma#
   A FAR-ban megváltoztatható a parancssori prompt formátuma.
Ehhez a ~kezelőfelület beállítások~@InterfSettings@ párbeszédablak
#Parancssori prompt formátuma# beviteli mezőjében be kell írni a változók és
speciális kódszavak megfelelő sorrendjét, így a prompt további adatokat
jeleníthet meg.

   A környezeti változókon túl ezeket a speciális kódszavakat használhatjuk:

     $a - & karakter
     $b - | karakter
     $c - ( karakter
     $d - az aktuális dátum (a rendszer beállításaitól függ)
     $f - ) karakter
     $g - > karakter
     $h - törli az előző karaktert
     $l - < karakter
     $n - az aktuális meghajtó betűjele
     $p - az aktuális meghajtó és elérési út
     $q - = karakter
     $s - szóköz
     $t - az aktuális idő óó:pp:mm formátumban
     $$ - $ karakter

   Alapértelmezett a #$p$g# formátum - az aktuális meghajtó és az elérési
útvonal (#C:\>#).

   Példák:

   1. ^<wrap>A #[%COMPUTERNAME%]$S$P$G# formátumú prompt a számítógép nevét,
az aktuális meghajtó betűjelét és az elérési utat tartalmazza (ehhez a
%COMPUTERNAME% környezeti változónak is definiáltnak kell lennie).

   2. ^<wrap>A #[$T$H$H$H]$S$P$G# formátumú promptban az aktuális idő
(óó:pp) után a meghajtó betűjele és az elérési út áll.


@Viewer
$ #Nézőke: vezérlőbillentyűk#
   Nézőke parancsok

    #Bal#                Egy karakterrel balra
    #Jobb#               Egy karakterrel jobbra
    #Fel#                Egy sorral fel
    #Le#                 Egy sorral le
    #Ctrl-Bal#           ^<wrap>20 karakterrel balra, hexa módban 1 hellyel
balra
    #Ctrl-Jobb#          20 karakterrel jobbra, hexa módban 1 hellyel
jobbra
    #PgUp#               Egy oldallal fel
    #PgDn#               Egy oldallal le
    #Ctrl-Shift-Bal#     A sorok kezdő pozíciójára ugrik (ha a sortörés
nincs bekapcsolva és a sorok túlnyúlnak a kép méretén)
    #Ctrl-Shift-Jobb#    A sorok végső pozíciójára ugrik (ha a sortörés
nincs bekapcsolva és a sorok túlnyúlnak a kép méretén)
    #Home, Ctrl-Home#    A fájl elejére ugrik
    #End, Ctrl-End#      A fájl végére ugrik

    #F1#                 Súgó
    #F2#                 Sortörés be/ki
    #Shift-F2#           Betűtörés/szótörés átkapcsoló
    #F4#                 Szöveg/hexa mód átkapcsoló
    #F6#                 Átvált ~szerkesztésre~@Editor@
    #Alt-F5#             ^<wrap>Fájl nyomtatása (a Nyomtatóvezérlő plugin
segítségével)
    #F7#                 ~Keresés~@ViewerSearch@
    #Shift-F7, Szóköz#   Tovább keres
    #Alt-F7#             Tovább keres, de visszafelé
    #F8#                 OEM/ANSI kódlap váltó
    #Shift-F8#           Kódlap kiválasztása
    #Alt-F8#             ~Ugrás~@ViewerGotoPos@ a jelenlegi
szövegpozícióból másik pozícióba
    #Alt-F9#             Átváltja a FAR konzolablak méretét (video)
    #Alt-Shift-F9#       Meghívja a
~nézőke beállítások~@ViewerSettings@ párbeszédablakot
    #Numpad5,F3,F10,Esc# Kilépés
    #Ctrl-F10#           Megállapítja a megnézett fájl helyét
(a konzolablak fejlécének szövegét - "X fájl megnézése" - a "(meghajtó+elérési
út)" információra cseréli, tehát megmutatja a fájl helyét a fastruktúrán - a
ford.)
    #F11#                Meghívja a ~plugin parancsok~@Plugins@ menüt
    #Alt-F11#            Megjeleníti a ~fájl előzményeket~@HistoryViews@
    #+#                  A mappa következő fájlját nyitja meg
    #-#                  A mappa előző fájlját nyitja meg
    #Ctrl-O#             Megjeleníti a konzolhátteret
    #Ctrl-Alt-Shift#     Átmenetileg megmutatja a konzolhátteret
(amíg a billentyűket lenyomva tartjuk)
    #Ctrl-B#             Elrejti vagy megmutatja a képernyő alján
a funkcióbillentyűsort
    #Ctrl-Shift-B#       Megmutatja/elrejti az állapotsort
    #Ctrl-S#             Megmutatja/elrejti a gördítősávot
    #Alt-BS, Ctrl-Z#     Visszalép a fájlban az előző pozícióra
    #JobbCtrl-0..9#      0-tól 9-ig könyvjelzőt tesz a pozícióba
    #Ctrl-Shift-0..9#    0-tól 9-ig könyvjelzőt tesz a pozícióba
    #LeftCtrl-0..9#      A 0...9-es könyvjelzőre ugrik

    #Ctrl-Ins, Ctrl-C#   ^<wrap>A keresés találataként kiemelt szöveget
a vágólapra másolja.
    #Ctrl-U#             Leveszi a keresés találatáról a kiemelést.

    Lásd még a ~nézőke makróinak~@KeyMacroViewerList@ listáját.

    Megjegyzések:

    1. ^<wrap>A keresőablak meghívásához a nézőkében az is elég, ha elkezdjük
begépelni a keresett szöveget.

    2. ^<wrap>Az, hogy a nézőkében megnyitunk egy fájlt, nem zárja ki,
hogy közben egy másik folyamat ne törölhetné azt. Annak ellenére, hogy a fájl
valójában csak a nézőke bezárásakor törlődik, a törölt fájlra irányuló további
műveletek hibával fognak leállni - ez Windows sajátosság.

    3. ^<wrap>A FAR jelenlegi verziója korlátozza a belső nézőkében megnyitott
fájlok oszlopainak egy sorban megjeleníthető maximális számát: értéke nem
haladhatja meg a 2048-at. Ha valamelyik sor túllépi ezt, a FAR akkor is
több sorban jeleníti meg, ha a sortörés ki van kapcsolva. (Oszlopok száma =
karakterek száma.)

    4. ^<wrap>A FAR nézőke ~keresője~@ViewerSearch@ (#F7#) a fájl képernyőn
megjelenő részének kezdetétől az első előfordulásig keresi a sztringet.

    5. ^<wrap>Ha automatikusan szeretnénk gördíteni egy folyamatosan változó
tartalmú fájlt, vigyük a kurzort a fájl végére (az End billentyűvel).


@ViewerGotoPos
$ #Nézőke: ugrás a megadott pozícióba#
    A párbeszédablak segítségével a nézőkében megnyitott fájl megadott részére
ugorhatunk.

    Megadhatunk decimális eltolást, százalékot vagy hexadecimális eltolást.
Az értékeket beírhatjuk olyan specifikus formában, ahol a számok elé
vagy mögé írt kiegészítő elemek határozzák meg az értékek kezelését. Ha nem
adunk meg ilyen kiegészítőket, csak számokat, az értékek kezelési
módját a rádiógombokkal jelölhetjük ki.

    A számok elé írt + vagy - jel relatív eltolást eredményez.

    A hexadecimális eltolást a következő formátumokban adhatjuk meg:
       0xNNNN, NNNNh, $NNNN

    A decimális eltolás (nem százalékos) formátuma NNNNd.

   Példák:

     #50%#           A fájl közepére ugrik (50%).
     #-10%#          ^<wrap>A jelenlegi helyzettől 10%-ot lép
visszafelé (ha 50%-on álltunk, az új pozíció 40%-on lesz).
     #0x100#         A 0x100 (256) bájtpozícióba lép.
     #+0x300#        0x300 (768) bájtot lép előre.

    Ha a számértéket az említett formátumleírók valamelyikével ("%", "0x",
"h", "$", "d") kiegészítve adtuk meg, a rádiógombok állapotát a FAR nem veszi
figyelembe.


@ViewerSearch
$ #Nézőke: keresés#
    A ~nézőke~@Viewer@ keresőjében a következő keresési módok és lehetőségek
közül választhatunk:

    #Szöveg keresése#

    ^<wrap>A #Keresés# szerkesztősorába beírt bármilyen szöveg keresése.

      #Nagy/kisbetű érzékeny#  - ^<wrap>Keresésnél a szöveg betűinek mérete
is számít (ha például a #text#-et keressük, a fájlban előforduló #Text#
szöveget a FAR nem értékeli találatnak)

      #Csak egész szavak#      - ^<wrap>A megadott szöveg előfordulásait csak
akkor veszi találatnak, ha soremelések, tabulátorok vagy szóközök határolják,
vagy a szabványos elválasztó karakterek: #!%^&*()+|{}:"<>?`-=\\[];',./#

    #Hexa keresése#

    ^<wrap>A #Keresés# mezőben beírt szöveget
hexadecimális kóddá alakítja és ezt fogja keresni (a "Nagy/kisbetű érzékeny"
és a "Csak egész szavak" opció ez esetben nem jelölhető be).

      #Visszafelé keres#       - ^<wrap>Megfordítja a keresés irányát, a fájl
végétől keres a fájl elejéig.


@Editor
$ #Szerkesztő#
    A kurzor alatti fájl szerkesztéséhez nyomjuk le az #F4# billentyűt.
Így vagy a belső szerkesztővel, vagy bármelyik, előzőleg a
~szerkesztő beállítások~@EditorSettings@ menüben beállított külső
szerkesztővel nyithatjuk meg.

    #Új fájl létrehozása a szerkesztővel#

    Ha a #Shift-F4# billentyűk lenyomása után egy nem létező fájl nevét
adjuk meg, ~új fájlt~@FileOpenCreate@ hozhatunk létre.

    Megjegyzések:

    1. ^<wrap>Ha az új fájl létrehozása során egy nem létező mappa nevét is
beírjuk, ~"A szerkesztendő fájl célmappája még nem létezik,~@WarnEditorPath@
~de mentéskor létrejön. Folytatja?"~@WarnEditorPath@ figyelmeztetést kapjuk.

    2. ^<wrap>Ha olyan fájlt próbálunk szerkesztésre megnyitni, ami a
szerkesztőben jelenleg is nyitva van,
"~a szerkesztett fájl újbóli megnyitása~@EditorReload@" ablakban választhatunk
a lehetőségek közül.

    3. ^<wrap>Az új fájl létrehozásakor alapértelmezés szerint a Windows
kódlapját kapja, de ez az opció a ~szerkesztő beállítások~@EditorSettings@
menüben kikapcsolható.

  #Vezérlőbillentyűk#

  Kurzorvezérlés

   #Bal#                     Egy karakterrel balra
   #Ctrl-S#                  ^<wrap>Egy karakterrel balra viszi a kurzort,
de ha eléri a sor elejét, nem lép fel az előző sorra
   #Jobb#                    Egy karakterrel jobbra
   #Fel#                     Egy sort fel
   #Le#                      Egy sort le
   #Ctrl-Bal#                Egy szóval balra
   #Ctrl-Jobb#               Egy szóval jobbra
   #Ctrl-Fel#                Egy sorral felfelé gördít
   #Ctrl-Le#                 Egy sorral lefelé gördít
   #PgUp#                    Egy lappal fel
   #PgDn#                    Egy lappal le
   #Home#                    A sor elejére
   #End#                     A sor végére
   #Ctrl-Home, Ctrl-PgUp#    A fájl elejére
   #Ctrl-End, Ctrl-PgDn#     A fájl végére
   #Ctrl-N#                  A képernyő tetejére
   #Ctrl-E#                  A képernyő aljára

  Törlő műveletek

   #Del#                     ^<wrap>A kurzor alatti karakter törlése (a
kijelölt szöveget is törölheti, a
~szerkesztő beállításaitól~@EditorSettings@ függően)
   #BackSpace#               Egy karakterrel balra töröl
   #Ctrl-Y#                  Egy sort töröl
   #Ctrl-K#                  Törlés a sor végéig
   #Ctrl-BackSpace#          Szó törlése balra
   #Ctrl-T, Ctrl-Del#        Szó törlése jobbra

  Blokkműveletek

   #Shift-Kurzorbill.#       Blokk kijelölése
   #Ctrl-Shift-Kurzorbill.#  Blokk kijelölése
   #Alt-Szürke kurzorbill.#  Függőleges blokk kijelölése
   #Alt-Shift-Kurzorbill.#   Függőleges blokk kijelölése
   #Ctrl-Alt-Szürke bill.#   Függőleges blokk kijelölése
   #Ctrl-A#                  Az egész szöveg kijelölése
   #Ctrl-U#                  Leveszi a blokk kijelölését
   #Shift-Ins, Ctrl-V#       Blokk beillesztése a vágólapról
   #Shift-Del, Ctrl-X#       Blokk kivágása
   #Ctrl-Ins, Ctrl-C#        Blokk másolása a vágólapra
   #Ctrl-<Szürke +>#         Blokk hozzáfűzése a vágólaphoz
   #Ctrl-D#                  Blokk törlése
   #Ctrl-P#                  ^<wrap>Blokk másolása a jelenlegi
kurzorpozícióba (csak maradó blokk módban)
   #Ctrl-M#                  Blokk mozgatása a jelenlegi
kurzorpozícióba (csak maradó blokk módban)
   #Alt-U#                   Blokk eltolása balra
   #Alt-I#                   Blokk eltolása jobbra

  Egyéb műveletek

   #F1#                      Súgó
   #F2#                      Fájl mentése
   #Shift-F2#                ~Fájl mentése másként~@FileSaveAs@
   #Shift-F4#                ~Új fájl~@FileOpenCreate@ szerkesztése
   #Alt-F5#                  ^<wrap>Fájl vagy kijelölt blokk nyomtatása
(a Nyomtatásvezérlő pluginnel)
   #F6#                      ~Nézőke~@Viewer@ módba kapcsol
   #F7#                      ~Keresés~@EditorSearch@
   #Ctrl-F7#                 ~Keresés és csere~@EditorSearch@
   #Shift-F7#                Keresés és csere folytatása
   #Alt-F7#                  Keresés és csere folytatása, visszafelé
   #F8#                      OEM/ANSI kódlap váltó
   #Shift-F8#                Kódlap kiválasztása
   #Alt-F8#                  ~Ugrás~@EditorGotoPos@ megadott sorra és oszlopra
   #Alt-F9#                  A FAR konzolablak méretének átváltása
   #Alt-Shift-F9#            A ~szerkesztő beállítások~@EditorSettings@
párbeszédablakot jeleníti meg
   #F10, Esc#                Kilépés
   #Shift-F10#               Mentés és kilépés
   #Ctrl-F10#                A szerkesztett fájl helyzete a fán
   #F11#                     A ~plugin parancsok~@Plugins@ menü (a
szerkesztőhöz tervezett pluginekkel)
   #Alt-F11#                 ~Szerkesztő előzmények~@HistoryViews@
megjelenítése
   #Alt-BackSpace, Ctrl-Z#   Utolsó művelet visszavonása
   #Ctrl-L#                  A szerkesztett szöveg módosítását tiltja
   #Ctrl-O#                  A konzolablak hátterére vált
   #Ctrl-Alt-Shift#          A konzolablak háttere (amíg a billentyűk
le vannak nyomva)
   #Ctrl-Q#                  A következő billentyűkombinációt
karakterkódként kezeli
   #JobbCtrl-0...9#          0-tól 9-ig könyvjelzőt tesz az aktuális
kurzorpozícióba
   #Ctrl-Shift-0...9#        0-tól 9-ig könyvjelzőt tesz az aktuális
kurzorpozícióba
   #BalCtrl-0...9#           A 0...9-es könyvjelzőre ugrik
   #Shift-Enter#             Beszúrja az aktív panelről az aktuális
fájlnevet a kurzorpozícióba
   #Ctrl-Shift-Enter#        Beszúrja a passzív panelről az aktuális
fájlnevet a kurzorpozícióba
   #Ctrl-F#                  Beszúrja a most szerkesztett fájl elérési
útját és nevét a kurzorpozícióba
   #Ctrl-B#                  Megmutatja/elrejti a funkcióbillentyűsort
a képernyő alján
   #Ctrl-Shift-B#            Megmutatja/elrejti a felső állapotsort

    Lásd még a ~szerkesztő makróinak~@KeyMacroEditList@ listáját.


    Megjegyzések:

    1. ^<wrap>Az #Alt-U#/#Alt-I# a sor behúzását állítja, ha nincs
kijelölt blokk.

    2. ^<wrap>Lenyomott #Alt#-tal a karaktereket decimális kódjukkal
írhatjuk be a numerikus billentyűzeten (0-65535).

    3. ^<wrap>Ha nincs kijelölt blokk, a #Ctrl-Ins#/#Ctrl-C# kijelöli
a teljes aktuális sort és a vágólapra másolja.

@EditorSearch
$ #Editor: search/replace#


@FileOpenCreate
$ #Szerkesztő: fájl megnyitása/létrehozása#
    A #Shift-F4# billentyűkombinációval létező vagy új fájlt nyithatunk meg
szerkesztésre.

    A ~szerkesztő beállításaitól~@EditorSettings@ függően az új fájl OEM
vagy ANSI kódolású lesz, de szükség esetén a kódlapok #listájából# más
kódlapot is választhatunk.

    Létező fájlnál is szükség lehet a #Кódlap# paraméter átállítására,
ha például az "Automatikus felismerés" funkció rossz kódlapot állít be.


@FileSaveAs
$ #Szerkesztő: fájl mentése másként#
    A #Shift-F2# billentyűkombinációval a jelenleg szerkesztett fájlt új
néven, másik kódlappal és más sortörés karakterrel menthetjük el.

    Ha UTF-8, UNICODE vagy REVERSEBOM kódlapot választottunk,
a #Unicode bájtsorrend jelzővel (BOM)# opció bekapcsolása speciális jelzőt
állít be a fájlban, lehetővé téve a programok számára a formátumon belüli
egyedi formátumazonosító felismerését.

    ^<wrap>Az új néven és a kódlapon kívül megadhatjuk a sortörés karakterek
formátumát is:

    #Nincs konverzió#
    ^<wrap>A sortörés karakterek nem változnak.

    #DOS/Windows formátum (CR LF)#
    ^<wrap>A sortöréseket két tagból álló szekvenciává, Carriage Return
(CR), azaz "kocsi vissza" és Line Feed (LF), azaz "soremelés" karakterré
konvertálja, a DOS/Windows formátumnak megfelelően.

    #Unix formátum (LF)#
    ^<wrap>A sortöréseket Line Feed (LF) karakterré konvertálja, a UNIX
formátumnak megfelelően.

    #Mac formátum (CR)#
    ^<wrap>A sortöréseket Carriage Return (CR) karakterré konvertálja, a
Mac OS formátumnak megfelelően.


@EditorGotoPos
$ #Szerkesztő: ugrás megadott sorra és oszlopra#
    A párbeszédablakban a belső szerkesztőbe betöltött fájl pozícióiba
ugrást definiálhatjuk.

    Megadhatjuk a #sort#, az #oszlopot# (col) vagy mindkettőt.

    Az első érték a sor számát, a második az oszlop számát jelképezi.
A számokat a következő karakterek valamelyikével kell elválasztani: "," "."
";" ":" vagy szóköz.

    Ha az értéket ",Col" formátumban adjuk meg, a szerkesztő a jelenlegi sor
megadott oszlopára ugrik.

    Ha a sorszám után "%"-ot írunk, a szerkesztő a fájl megadott százalékú
pozíciójába lép, például #50%#-ot beírva a fájl közepére ugrik.


@EditorReload
$ #Szerkesztő: szerkesztett fájl újbóli betöltése#
    A FAR Manager minden olyan kísérletet nyomon követ, amikor egy jelenleg
már szerkesztett fájlt próbálunk meg ismét megnyitni szerkesztésre. A fájl
újratöltésének szabályai:

    1. ^<wrap>Ha a fájl nem változott és a ~Megerősítések~@ConfirmDlg@
párbeszédablak "Szerkesztett fájl újratöltése" opciója nincs engedélyezve,
a FAR minden további figyelmeztetés nélkül a megnyitott példányra vált.

    2. ^<wrap>Ha a fájl a szerkesztés során megváltozott vagy a "Szerkesztett
fájl újratöltése" opció engedélyezve volt, az előugró ablak szerkesztési
módra vonatkozó kérdésére háromféle módon válaszolhatunk:

    #A mostanit#     Folytatja a jelenleg megnyitott fájl
    #folytatja#      szerkesztését.

    #Új példányban#  ^<wrap>A fájlt a szerkesztő új példányában nyitja meg.
Ebben az esetben gondosan ügyeljünk arra, hogy a mentésnél mindig az
utolsóként bezárt példány aktuális állapota fogja eldönteni a mentett fájl
végleges tartalmát!

    #Újratölti#      ^<wrap>Az eddigi változtatások elvesznek és a fájlt
eredeti állapotában tölti be a lemezről a szerkesztőbe.

@WarnEditorPath
$ #Figyelem: A szerkesztendő fájl célmappája még nem létezik...#
    Ezt az üzenetet akkor kapjuk, ha a ~szerkesztőben~@Editor@ megnyitott új
fájl elérési útvonalaként nem létező mappát adunk meg. Mentés előtt a FAR
létrehozza a mappát, feltéve, ha az elérési út helyes (például elfogadhatatlan,
ha nem létező meghajtó nevével kezdődik az elérési út), valamint kellő
jogosultsággal rendelkezünk a mappa létrehozásához.

@WarnEditorPluginName
$ #Figyelem: A szerkesztendő fájlnak nevet kell adni#
    Ha a szerkesztővel pluginnel emulált fájlrendszerben szeretnénk új fájlt
létrehozni, elkerülhetetlen a fájlnév megadása.

@WarnEditorSavedEx
$ #Figyelem: A fájlt egy külső program megváltoztatta#
    A fájl lemezen található példányának módosítási dátuma és ideje nem
egyezik azzal, amit a FAR az utolsó hozzáféréskor mentett. Ez annyit jelent,
hogy egy másik program vagy másik felhasználó (vagy akár mi módosítottuk, a
szerkesztő másik példányával) a szerkesztés közben megváltoztatta a fájlt.

    Ha a "Mentés" gombot nyomjuk le, a fájl tartalmát a szerkesztőben
megnyitott példány aktuális állapota írja felül és a külső program által
végrehajtott összes módosítás elvész.


@DriveDlg
$ #Meghajtóváltás (Meghajtók menü)#
    A Meghajtók menüben másik meghajtót választhatunk a panelhez,
leválaszthatjuk a hálózati meghajtókat vagy új ~plugin~@Plugins@ panelt
nyithatunk meg.

    A meghajtók és a pluginek közül sávkurzorral vagy a hozzájuk rendelt
betűjelekkel és számokkal választhatunk. Ha a panel típusa eredetileg
nem ~fájlpanel~@FilePanel@ volt, meghajtóváltás után az lesz.

    #Ctrl-A#, #F4# nyílt a Windows meghajtó tulajdonságai párbeszédpanelen.

    A #Del# billentyűvel:

    - ~leválaszthatjuk~@DisconnectDrive@ a hálózati meghajtókat;

    - ^<wrap>törölhetjük a SUBST paranccsal létrehozott virtuális
meghajtókat;

    - ^<wrap>kiadathatjuk a CD-ROM-ok vagy más cserélhető lemezes meghajtók
lemezeit.

      ^<wrap>A ZIP meghajtók lemezének kiadásához rendszergazda jogosultság
szükséges. A CD-ROM-ok tálcáját az #Ins# billentyűvel tolhatjuk be.

    A #Shift-Del# billentyűkombinációval biztonságosan eltávolíthatjuk az USB
portra csatlakoztatott tárolóeszközöket. Ha olyan kártyaolvasóba
helyezett flash memóriakártyára adtuk ki a ~biztonságos eltávolítás~@HotPlugList@
parancsot, ahol a kártyalvasó több lemez kezelésére képes, a parancs a
kártyaolvasót választja le.

    A Meghajtók menüben a #Ctrl-1 - Ctrl-9# billentyűkkel a meghajtókra
vonatkozó különféle információk megjelenítését kapcsolhatjuk ki vagy be:

    Ctrl-1 - a lemez típusa;
    Ctrl-2 - ^<wrap>a hálózat neve (és a SUBST meghajtó gazdalemezén annak a
mappának az elérési útvonala, amihez a virtuális meghajtót hozzárendeltük);
    Ctrl-3 - a lemez címkéje;
    Ctrl-4 - a fájlrendszer;
    Ctrl-5 - a teljes és a szabad lemezterület mérete (kétféle
megjelenítési módja van, nyomjuk le kétszer);
    Ctrl-6 - a kivehető lemez paraméterei;
    Ctrl-7 - pluginek megjelenítése;
    Ctrl-8 - a CD meghajtók fajtái;
    Ctrl-9 - a hálózat jellemzői.

    A #Meghajtók# menü beállításait a FAR a többi konfigurációs adattal együtt
menti.

    Ha ~A Ctrl-PgUp meghajtót vált~@InterfSettings@ opciót engedélyeztük, a
#Ctrl-PgUp# ugyanúgy működik, mint az #Esc#: kilép a Meghajtók menüből és
bezárja az ablakot.

    A #Shift-Enter# meghívja a Windows Explorert, megjelenítve benne a
kiválasztott meghajtó gyökerét (csak "valódi" meghajtóknál működik,
pluginnel emulált fájlrendszereknél nem).

    A #Ctrl-R# frissíti a Meghajtók menü tartalmát.

    Ha a #CD meghajtó típusa# mód engedélyezve van (#Ctrl-8#), a FAR
igyekszik felismerni az összes rendszerbe csatlakozó CD meghajtó
típusát. A felismert típusok: CD-ROM, CD-RW, CD-RW/DVD, DVD-ROM, DVD-RW és
DVD-RAM. Ez a funkció csak a rendszergazda jogokkal rendelkező
felhasználóknál és helyi felhasználóknál működik, ha a "Helyi biztonsági
beállítások" szerkesztőjében a
#Helyi házirend/Biztonsági beállítások/Eszközök:# tételnél
#A CD-ROM használatához kötelező bejelentkezni a helyi számítógépre#
szabályt engedélyeztük. A biztonsági szerkesztőprogramot a
parancssorból a #secpol.msc# parancs kiadásával is elindíthatjuk.

    A Meghajtók menüben az #Alt-Shift-F9# meghívja a
~plugin beállítások~@PluginsConfig@ menüt (ha #Ctrl-7#-tel engedélyeztük
a pluginek megjelenítését).

    A #Shift-F9# pluginen lenyomva meghívja a plugin beállításainak
párbeszédablakát.

    A #Shift-F1# pluginen lenyomva meghívja a plugin helyzetérzékeny
súgóját (ha a súgófájl létezik).

@ChangeDriveMode
$ #Change Drive Menu Options#


@DisconnectDrive
$ #Hálózati meghajtó leválasztása#
    A ~Meghajtók~@DriveDlg@ menüben a #Del# lenyomásával leválaszthatjuk
a hálózati meghajtókat.

    A #[x] Belépéskor újracsatlakoztat# opció csak az állandóan csatlakozó
hálózati meghajtóknál engedélyezett.

    A leválasztás jóváhagyása a ~megerősítések~@ConfirmDlg@
párbeszédablakban kapcsolható ki/be.


@Highlight
$ #Fájlkiemelések, rendezési csoportok#
    A panelek fájljai és mappái kényelmesebb és áttekinthetőbb formában
jeleníthetők meg a FAR Manager színes kiemelési lehetőségével.
A fájlokat különböző feltételek szerint (~fájlmaszk~@FileMasks@,
attribútumok) csoportosíthatjuk és a létrehozott csoportokhoz színeket
rendelhetünk.

    A kiemelések megjelenítését a Beállítások menü ~Panel beállítások~@PanelSettings@
párbeszédablakában a "Fájlok kiemelése" opcióval engedélyezhetjük vagy
tilthatjuk le.

    Bármelyik csoport kiemelésének jellemzőit ~szerkeszthetjük~@HighlightEdit@
a ~Beállítások~@OptMenu@ menü "Fájlkiemelések, rendezési csoportok"
menüpontjában.


@HighlightList
$ #Fájlkiemelések, rendezési csoportok: vezérlőbillentyűk#
    A ~Fájlkiemelések, rendezési csoportok~@Highlight@ menüben különféle
műveleteket hajthatunk végre a csoportok listáján, a következő billentyűkkel:

  #Ins#            - Új kiemelési csoport létrehozása

  #F5#             - Az aktuális csoport duplikálása

  #Del#            - Az aktuális csoport törlése

  #Enter# vagy #F4#  - Az aktuális kiemelési csoport ~szerkesztése~@HighlightEdit@

  #Ctrl-R#         - ^<wrap>Visszaállítja az alapértelmezett kiemelési
csoportokat

  #Ctrl-Fel#       - A csoportot felfelé mozgatja

  #Ctrl-Le#        - A csoportot lefelé mozgatja

    A FAR a csoportkiemeléseket felülről lefelé haladva vizsgálja. Ha érzékeli,
hogy a fájl valamelyik csoport tagja, további hovatartozását nem vizsgálja.


@HighlightEdit
$ #Fájlkiemelések, rendezési csoportok: szerkesztés#
    A ~Beállítások menü~@OptMenu@ #Fájlkiemelések, rendezési csoportok#
párbeszédablakában hozhatunk létre fájlkiemelési csoportokat. Minden
csoportdefiníció tartalmazhat:

     - egy vagy több ~fájlmaszkot~@FileMasks@;

     - méretbeli és dátum/idő korlátokat;

     - befoglaló vagy kizáró attribútumokat:
       #[x]# - ^<wrap>befoglaló attribútum - a fájlnak rendelkeznie kell
az attribútummal
       #[ ]# - kizáró attribútum - a fájlnak nem lehet ilyen
attribútuma
       #[?]# - az attribútum értéke nem számít;

     - ^<wrap>a normál fájlnév, a kijelölt fájlnév, a kurzor alatti
fájlnév és a kurzor alatti kijelölt fájlnév színét. Ha egy elemre az
alapértelmezett színeket szeretnénk használni, a színeket állítsuk "feketén
fekete", azaz fekete háttéren fekete szöveg színösszetételre;

     - ^<wrap>megadható fájljelölő karaktert. A jelölő karaktert használhatjuk
színkiemeléssel együtt vagy helyette.

    Ha a "Maszk" opció ki van kapcsolva, a FAR a maszkokat nem elemzi, csak a
többi bekapcsolt analízis számít (méret, dátum/idő, attribútum).

    Egy fájl akkor tartozik egy kiemelési csoportba, ha:

     - ^<wrap>a fájlmaszkelemzés engedélyezve van és a fájl megfelel
legalább egy maszknak (kikapcsolt maszkelemzésnél a fájlnév nem számít);

     - a méret és a dátum/idő határértékeinek megfelel;

     - megvan minden szükséges attribútuma;

     - nincs egyetlen kizárt attribútuma sem.

    A Tömörített, Titkosított, Nem indexelt, Ritkított és Átmeneti
attribútumok, valamint a szimbolikus linkek csak NTFS fájlrendszerben
értelmezettek.


@ViewerSettings
$ #Beállítások: nézőke beállítások#
    Ebben a párbeszédablakban a külső és ~belső nézőke~@Viewer@ alapértelmezett
beállításait változtathatjuk meg.


    Külső nézőke

  #Alt-F3 helyett F3 in-#   Az #Alt-F3# helyett #F3# hívja meg
  #dítja a külső nézőkét#   a külső nézőkét.

  #Nézőke parancs#          A külső nézőkét elindító parancssor.
                          ^<wrap>A parancssorban a megnézendő fájlnevek
megadásához alkalmazhatunk ~különleges szimbólumokat~@MetaSymbols@ is. Ha nem
szeretnénk, hogy a külső nézőke futtatása előtt a FAR paneljei
kikapcsolódjanak, kezdjük a parancsot #@@# karakterrel.

    Belső nézőke

  #Maradó blokkok#          ^<wrap>Nem veszi le a kijelölést a blokkokról,
ha megmozdítjuk a kurzort.

  #Gördítőnyilak mutatva#   ^<wrap>Kikapcsolt sortörésnél a vízszintesen
túlnyúló sorok végein gördítőnyilak jelennek meg.

  #Fájlpozíció mentése#     ^<wrap>Elmenti és visszatölti a legutóbb
megnézett fájlok szöveghelyzetét, vele a kódlapot is (ha "kézzel" választottuk
ki), valamint a nézet módját (normál vagy hexadecimális).

  #Könyvjelzők mentése#     ^<wrap>Elmenti és visszatölti az utoljára
megnézett fájlokban a #JobbCtrl-0...9# vagy a #Ctrl-Shift-0...9# leütésével
elhelyezett könyvjelzőinket.

  #Tabulátor mérete#        A tabulátor szóközökben mért hossza.

  #Gördítősáv mutatva#      ^<wrap>Az oldalsó gördítősáv megjelenítése a
belső nézőkében. Ezt a lehetőséget a #Ctrl-S# leütésével is bekapcsolhatjuk.

  #Kódlap automatikus#      ~Automatikusan felismeri~@CodePage@ a megnézett
  #felismerése#             szöveg kódlapját.

  #Fájlok eredeti meg-#     A megnyitott fájlok alapértelmezett
  #nyitása ANSI kódlappal#  kódlapja OEM helyett ANSI lesz.

    Ha az #F3# billentyűhöz rendeltük a külső nézőkét, az csak akkor indul el,
ha az aktuális fájltípushoz nincs ~társítva~@FileAssoc@ nézőke.

    A párbeszédablakban a beállítások módosítása nincs hatással az előzőleg
megnyitott belső nézőke ablakokra.

    A nézőke beállításainak párbeszédablakát meghívhatjuk úgy is, ha a
~belső nézőkében~@Viewer@ #Alt-Shift-F9#-et ütünk. Ebben az esetben a
változtatások rögtön életbe lépnek, de csak az aktuális munkafolyamatra
érvényesek.


@EditorSettings
$ #Beállítások: szerkesztő beállítások#
    Ebben a párbeszédablakban a külső és ~belső szerkesztő~@Editor@
alapértelmezett beállításait változtathatjuk meg.

    Külső szerkesztő

  #Alt-F4 helyett F4#       Az #Alt-F4# helyett #F4# hívja meg a külső
  #indítja a külső#         szerkesztőt.
  #szerkesztőt#

  #Szerkesztő parancs#      ^<wrap>A külső szerkesztőt indító parancssor.
~Különleges szimbólumokat~@MetaSymbols@ is használhatunk a szerkesztendő fájl
megadásánál. Ha nem szeretnénk, hogy a külső szerkesztő futtatása előtt a FAR
paneljei kikapcsolódjanak, kezdjük a parancssort #@@# karakterrel.

    Belső szerkesztő

    Tabulátorból szóközök:

    #Ne helyettesítse a#    A tabulátorokat nem konvertálja szóközzé
    #tabulátorokat#         a szerkesztés során.

    #Újonnan beírt tabu-#   Szövegszerkesztés közben minden beírt
    #látorokból szóközök#   ^<wrap>#Tab# karaktert megfelelő számú szóközzel
helyettesít, de a korábbi tabulátorokat nem konvertálja.

    #Minden tabulátorból#   A szöveg megnyitásakor automatikusan
    #szóközök#              minden tabulátort szóközzé alakít.


  #Maradó blokkok#          ^<wrap>Nem veszi le a blokkokról a kijelölést,
ha megmozdítjuk a kurzort.

  #A Del törli#             Ha van kijelölt blokk, a #Del# nem a
  #a blokkokat#             ^<wrap>kurzor alatti karaktert, hanem a blokkot
törli.

  #Fájlpozíció mentése#     ^<wrap>Elmenti és visszatölti a legutóbb
szerkesztett fájlok szöveghelyzetét és a kódlapot is, ha utóbbit kézzel
választottuk ki.

  #Könyvjelzők mentése#     ^<wrap>Elmenti és visszatölti az utoljára
szerkesztett fájlokban a #JobbCtrl-0...9# vagy a #Ctrl-Shift-0...9# leütésével
elhelyezett könyvjelzőinket.

  #Automatikus behúzás#     ^<wrap>Szöveg beírásánál engedélyezi az
önműködő behúzást.

  #Kurzor a sorvégjel#      A szerkesztőben a kurzor a sorvégjel
  #után is#                 mögé is vihető.

  #Tabulátor mérete#        A tabulátor hossza, szóközökben.

  #Gördítősáv mutatva#      ^<wrap>Az oldalsó gördítősáv megjelenítése a
a belső szerkesztőben.


  #Írásra megnyitott fáj-#  Lehetővé teszi a más programokban írásra
  #lok szerkeszthetők#      ^<wrap>megnyitott fájlok szerkesztését. Ez a
funkció praktikus, ha hosszú időre megnyitott fájlt szeretnénk szerkeszteni,
de veszélyessé válhat, ha a fájl szerkesztés közben módosul.

  #Csak olvasható fájlok#   Ha "csak olvasható" attribútumú fájlt
  #szerkesztése tiltva#     ^<wrap>nyitottunk meg szerkesztésre, a
szerkesztő ugyanúgy letiltja a szöveg módosítását, mintha #Ctrl-L#-t ütnénk.

  #Figyelmeztet csak#       Ha "csak olvasható" attribútumú fájlt
  #olvasható fájl#          próbálunk megnyitni szerkesztésre,
  #megnyitásakor#           előtte figyelmeztető üzenetet kapunk.

  #Kódlap automatikus#      ~Automatikusan felismeri~@CodePage@ a
  #felismerése#             szerkesztendő szöveg kódlapját.

  #Fájlok eredeti megnyi-#  A fájlokat OEM helyett ANSI kódlappal
  #tása ANSI kódlappal#     nyitja meg.

  #Új fájl létrehozása#     Az új fájlokat OEM helyett ANSI kódlappal
  #ANSI kódlappal#          hozza létre.

    Ha külső szerkesztőt rendeltünk az #F4# billentyűhöz, csak akkor indul el,
ha az aktuális fájltípushoz nincs ~társítva~@FileAssoc@ szerkesztő.

    A párbeszédablakban a beállítások módosítása nincs hatással az előzőleg
megnyitott belső szerkesztő ablakokra.

    A szerkesztő beállításainak párbeszédablakát meghívhatjuk úgy is, hogy a
~belső szerkesztőben~@Editor@ #Alt-Shift-F9#-et ütünk. Ebben az esetben a
változtatások rögtön életbe lépnek, de csak az aktuális munkafolyamatra
érvényesek.


@Codepage
$ #Kódlapok automatikus felismerése#
    A FAR megpróbálja megállapítani a fájl megnézéséhez vagy szerkesztéséhez
megfelelő kódlapot. Ne feledjük azonban, hogy a helyes felismerés nem
garantálható, különösen, ha rövid vagy nem tipikus szövegfájlt nyitunk meg.


@FileAttrDlg
$ #Fájl attribútumok párbeszédablak#
    A párbeszédablakban a fájlobjektumok attribútumait, valamint dátumát és
idejét változtathatjuk meg. Használhatjuk egyetlen fájlra vagy fájlok
csoportjára is. Ha nem szeretnénk, hogy a változtatások almappákban is
végbemenjenek, "Az almappákon is" opciót ne kapcsoljuk be.

  #Fájl attribútumok#

    A párbeszédablak jelölőnégyzetei három állapotot vehetnek fel:

     #[x]# - ^<wrap>minden kijelölt elemnek van ilyen attribútuma (minden
kijelölt elemre ráteszi az attribútumot)

     #[ ]# - ^<wrap>egyetlen kijelölt elemnek sincs ilyen attribútuma (minden
kijelölt elemről leveszi az attribútumot)

     #[?]# - ^<wrap>nincs minden kijelölt elemnek ilyen attribútuma (ne
változtasson az attribútumon)

    Azok a jelölőnégyzetek, ahol minden kijelölt fájlnak megegyeznek az
attribútumai, kétállapotúra változnak: csak bejelölni vagy törölni lehet az
értéküket. Ha csoportos változtatásnál a kijelölt elemek közt mappák is
vannak, minden jelölőnégyzet háromállapotú lesz.

    Csak azok az attribútumok fognak megváltozni, ahol a jelölőnégyzetekben
az eredeti állapothoz képest változtatás történt.

    A "Tömörített", "Titkosított", "Nem indexelt", "Ritkított", "Átmeneti" és
az "Offline" attribútum csak NTFS fájlrendszerű meghajtókon használható.
A "Virtuális" attribútum csak Windows Vista/2008 alatt használható. A
"Tömörített" (C) és a "Titkosított" (E) attribútum kizárja egymást: vagy
az egyik, vagy a másik adható meg.

    ~Mappa linkek~@HardSymLink@ esetében a párbeszédablak az eredeti mappa
adatait fogja megjeleníteni (csak NTFS fájlrendszerben). Ha az eredeti mappa
adatai nem állnak rendelkezésre (különösen távoli mappák szimbolikus
linkjeinél), az "#(adat nem elérhető)#" üzenet jelenik meg.

  #Fájlok dátuma és ideje#

    A fájlobjektumoknak három időtípusa van:

    - az utolsó módosítás időpontja;

    - a létrehozás időpontja;

    - az utolsó hozzáférés időpontja.

    FAT fájlrendszerű meghajtókon az utolsó hozzáférés óra, perc és másodperc
értéke mindig nulla.

    Ha nem akarunk változtatni a fájl dátumán vagy idején, hagyjuk a megfelelő
mezőket üresen vagy eredeti állapotukban. Az #Üres# gomb megnyomása minden
dátum- és időértéket töröl, ezután a megfelelő mezőkbe beírhatjuk a változtatni
kívánt értéket. Nem kötelező mindent kitölteni, mert a nem változtatott mezők
eredeti értékei megmaradnak.

    Az #Aktuális# gomb a jelenlegi idővel tölti fel a dátum/idő mezőket.

    Az #Eredeti# gomb a fájl vagy mappa eredeti időértékeivel tölti fel a
dátum/idő mezőket. Csak egy kijelölt fájlra vagy mappára használható,
csoportra nem.

    ~Szimbolikus linkek~@HardSymLink@ dátuma és ideje nem állítható.


@FolderShortcuts
$ #Mappa gyorsbillentyűk#
    A mappa gyorsbillentyűkkel a sűrűn használt mappákat egyszerűbben érhetjük
el, ha a #Ctrl-Shift-0...9# kombináció lenyomásával gyorsbillentyűt rendelünk
az aktív panel aktuális mappájához. A gyorsbillentyűkhöz rendelt mappákra a
#JobbCtrl-0...9# lenyomásával válthatunk. Ha a gyorsbillentyűket szövegbeviteli
sorban használjuk, a FAR beilleszti a sorba a mappa elérési útvonalát.

    A ~Parancsok menü~@CmdMenu@ #Mappa gyorsbillentyűk# menüpontjában
megnézhetjük, beállíthatjuk, szerkeszthetjük vagy törölhetjük a mappákhoz
rendelt gyorsbillentyűket. A menüben az #Ins# lenyomása a FAR aktív paneljének
elérési útvonalát rendeli hozzá az aktuális gyorsbillentyűhöz.

    Szerkesztéssel (#F4#) pluginnel emulált panelekhez nem rendelhetünk
gyorsbillentyűt.


@FiltersMenu
$ #Szűrők menü#
    A #Szűrők menüben# saját szabályokkal korlátozhatjuk a fájltípusok
körét, hogy csak a megadott paraméterekkel rendelkező fájlokon mehessen végbe
az a művelet, amelyből a Szűrők menüt meghívtuk.

    A Szűrők menü két részből áll. A felső részben jelennek meg a saját
#Felhasználói szűrők#, az alsó rész pedig az aktuális panel fájljainak
minden előforduló kiterjesztését listázza ki betűrendben. (Akkor is megjelenik
minden kiterjesztés, ha a szűrőt meghívó menüben megadott maszknak az aktuális
panelen egyetlen fájl sem felel meg.)

    A #Felhasználói szűrők# menüben a következő parancsokat használhatjuk:

   #Ins#        ^<wrap>Új szűrő létrehozása (egy üres ~szűrő~@Filter@
párbeszédablakot kapunk, amit nekünk kell beállítani).

   #F4#         Meglévő ~szűrő~@Filter@ szerkesztése.

   #F5#         Meglévő ~szűrő~@Filter@ duplikálása.

   #Del#        Szűrő törlése.

   #Ctrl-Fel#   A szűrőt egy hellyel feljebb mozgatja.

   #Ctrl-Le#    A szűrőt egy hellyel lejjebb mozgatja.


    A #Felhasználói szűrőkre# és az automatikusan generált szűrőkre (azaz a
fájlmaszkokra) egyaránt alkalmazhatók a következő parancsok:

   #Szóköz#, #Plusz#       ^<wrap>Amelyik menüelemen #Szóközt# vagy #+#-t
ütünk, "+" jelet kap. Ha vannak ilyen elemek, a művelet csak az ezeknek
megfelelő fájlokon megy végbe.

   #Minusz#              ^<wrap>A #-# billentyűvel kijelölt elemek "-" jelet
kapnak. Az így megjelölt szűrőknek megfelelő fájlokon nem megy végbe a
művelet.

   #I# vagy #X#            ^<wrap>Szerepük hasonló a #Plusz# és a #Minusz#
funkciójához, de találat esetén magasabb a prioritásuk.

   #Backspace#           Az aktuális elem jelölését törli.

   #Shift-Backspace#     Minden elem jelölését törli.

    A #Szűrők menü# párbeszédablakban a szűrőjelöléseken végzett
változtatások akkor lépnek életbe, ha #Enterrel# jóváhagyjuk őket.

    A szűrőket és jelöléseiket a FAR a többi konfigurációs adattal együtt
menti.

    \1B╔════════ C:\\\-    Ha egy panelen szűrő működik, a panel bal
    \1B║\1Ei*  Név     \-    felső sarkában a rendezési mód betűjele
    \1B║\1F..          \-    mellett #*# karakter jelenik meg.

    A szűrőfunkció a következő területeken működik:

    ~Fájlpanelek~@FilePanel@
    ~Másolás, mozgatás, átnevezés és linkek létrehozása~@CopyFiles@
    ~Fájlkeresés~@FindFile@


@FileDiz
$ #Fájlmegjegyzések#
    A fájlmegjegyzések segítségével szöveges információkkal láthatjuk el a
fájlokat. A megjegyzéseket a FAR a fájlok mappájában egy listafájlba
menti el. A megjegyzésfájl soronkénti formátuma: a fájlnév, minimum egy
szóköz, majd a megjegyzés szövege.

    Alapértelmezés szerint a megjegyzéseket két ~fájlpanel nézet mód~@PanelViewModes@
képes megjeleníteni, a #Fájlmegjegyzések# és a #Hosszú megjegyzés# mód.

    A ~Fájlok menü~@FilesMenu@ #Fájlmegjegyzés# (#Ctrl-Z#) menüpontjának
segítségével írhatjuk be a kijelölt fájlok megjegyzéseit.

    A megjegyzésfájlok neveit a ~Beállítások menü~@OptMenu@ #Fájl megjegyzésfájlok#
párbeszédablakában változtathatjuk meg. Itt állíthatjuk be a megjegyzések
frissítési módját is: letilthatjuk, vagy beállíthatjuk úgy, hogy csak akkor
frissüljön, ha a nézet mód megjeleníti a megjegyzéseket, vagy hogy mindig
frissüljön a lista. Alapértelmezés szerint a FAR "Rejtett" attribútumot ad
ezeknek a fájloknak, de a párbeszédablakban "Az új megjegyzésfájl rejtett
attribútumú legyen" opciót ki is kapcsolhatjuk. Ugyanitt állítható be az is,
hogy az új megjegyzésfájlok soraiban a megjegyzések hányadik karakterhelyen
kezdődjenek.

    Ha egy megjegyzésfájlnak "csak olvasható" attribútuma van, a FAR nem
próbálja frissíteni a megjegyzéseket és a fájlobjektumok mozgatása vagy
törlése után hibaüzenetet küld. Ha a
#Csak olvasható megjegyzésfájlok frissítése# opció engedélyezett, a FAR
a szokott módon igyekszik frissíteni a megjegyzéseket.

    Ha a beállításokban engedélyeztük, a FAR a másolás, mozgatás vagy törlés
során frissíti a megjegyzéseket, de ha a művelet almappákban lévő fájlokon
megy végbe, az almappák fájljainak megjegyzései nem frissülnek.


@PanelViewModes
$ #Fájlpanel nézet módok testreszabása#
    A ~fájlpanelek~@FilePanel@ 10 előre definiált nézet módban jeleníthetik
meg az információkat: rövid, közepes, teljes, széles, részletes,
fájlmegjegyzések, hosszú megjegyzés, fájl tulajdonos, fájl linkek és alternatív
teljes. Ennyi nézet általában elég, de ha akarjuk, módosíthatjuk a meglévőket
vagy lecserélhetjük őket teljesen újakra.

    A ~Beállítások menü~@OptMenu@ #Fájlpanel módok# almenüjében az említett
nézet módokat megváltoztathatjuk. Először ki kell választani a módosítani
kívánt nézetet a felkínált listából, ahol a "Rövid mód" megfelel a fájlpanelek
"Rövid" módjának (#BalCtrl-1#), a "Közepes mód" megfelel a fájlpanelek
"Közepes" módjának (#BalCtrl-2#) és így tovább az utolsó elemig, a
#BalCtrl-0#-val meghívható "Alternatív teljes" nézet módig bezárólag.

    Ha választottunk, a párbeszédablakban a következő jellemzőket állíthatjuk
be:

    #Oszloptípusok# - az oszloptípusok, amelyeket egy vagy több (vesszővel
elválasztott) karakter jelképez, a következők lehetnek:

    N[M,O,R]   - fájlnév
                 ahol:  M - jelölő karakter mutatva;
                        O - ^<wrap>nevek, elérési út nélkül (elsősorban
pluginekhez);
                        R - jobbra igazított nevek;
                 A módosítókat kombinálni is lehet, például NMR.

    S[C,T,F,E] - fájlméret
    P[C,T,F,E] - tömörített fájlméret
    G[C,T,F,E] - a fájlstream-ek mérete
                 ahol:  C - rendezett fájlméret formátum;
                        T - 1024 helyett 1000 az osztó;
                        F - ^<wrap>a fájlméretek a Windows Exploreréhez
hasonló formátumúak (például 999 bájt 999-ként, de 1000 bájt 0.97 kB formában
jelenik meg);
                        E - takarékos mód, nincs szóköz a fájlméret
és az utótag közt;

    D          - a fájl módosításának dátuma
    T          - a fájl módosításának ideje

    DM[B,M]    - a fájl módosításának dátuma és ideje
    DC[B,M]    - a fájl létrehozásának dátuma és ideje
    DA[B,M]    - a fájl utolsó hozzáférésének dátuma és ideje
                 ahol:  B - rövid (Unix stílusú) fájl időformátum;
                        M - szöveges hónapnevek;

    A          - fájlattribútumok
    Z          - fájlmegjegyzések

    O[L]       - a fájl tulajdonosa
                 ahol:  L - tartománynév mutatva (domain);

    LN         - hardlinkek száma

    F          - stream-ek száma

    Ha az oszloptípusok leírójában több fájlnévoszlop szerepel, a fájlpanel
többoszlopos formában jelenik meg.

    Az attribútumok betűjeleinek jelentése:

       #R#         - Read only (Csak olvasható)
       #S#         - System (Rendszer)
       #H#         - Hidden (Rejtett)
       #A#         - Archive (Archiv)
       #L#         - Mappa csomópont vagy szimbolikus link
       #C# vagy #E#  - ^<wrap>Compressed vagy Encrypted (Tömörített vagy
Titkosított)
       #$#         - Sparse (Ritkított)
       #T#         - Temporary (Átmeneti)
       #I#         - Nem (tartalom)indexelt
       #O#         - Offline
       #V#         - Virtuális

    Az attribútumok megjelenítési sorrendje: RSHALCTIOV. A "Ritkított"
attribútum csak fájlokra vonatkozhat és az "L" helyén jelenik meg. A
"Titkosított" (E) attribútum a "C" helyén jelenik meg, mivel a
fájloknak/mappáknak nem lehet egyszerre "Tömörített" és "Titkosított"
attribútuma. Az attribútumok oszlopa alapértelmezés szerint 6 karakter széles.
A többi attribútum megjelenítéséhez (T, I, O és V) kézzel kell 10
karakteresre állítani az oszlopszélességet.

    #Oszlopszélességek# - a panelek oszlopszélességét állíthatjuk be vele.
A "0" szélesség az alapértelmezett szélességet jelenti. Ha a Név, a
Megjegyzés vagy a Tulajdonos oszlop értéke "0", a FAR automatikusan állítja be
a szélességét a panel szélességéhez. Hogy az oszlopok a különféle szélességű
képernyőkön helyesen jelenjenek meg, feltétlenül ajánlott legalább egy
oszlopszélességet automatikusra állítani.

    Ha a fájl Idő vagy Dátum/Idő oszlopának alapértelmezett szélességét 1-gyel
növeljük, a megjelenítést 12 órás formátumra állíthatjuk át. A szélesség
további növelésével a másodperc és ezredmásodperc értéke is megjelenik.

    Ha a Dátum oszlop szélességét kettővel növeljük, az évszám négyszámjegyű
formában jelenik meg.

    Az #Állapotsor oszloptípusok# és az #Állapotsor oszlopszélességek#
beállítása hasonlóan működik az "Oszloptípusokhoz" és "Oszlopszélességekhez",
de a panelek állapotsorára hat.

    #Teljes képernyős nézet# - osztott képernyő helyett a fájlpanel elfoglalja
a teljes képernyőszélességet.

    #Fájlkiterjesztések igazítása# - a fájlok kiterjesztéseit igazítva mutatja
meg.

    #Mappakiterjesztések igazítása# - a mappák kiterjesztéseit igazítva mutatja
meg.

    #Mappák NAGYBETŰVEL mutatva# - minden mappa neve nagybetűsen jelenik meg,
eredeti betűméreteiktől függetlenül.

    #Fájlok kisbetűvel mutatva# - minden fájl neve kisbetűvel jelenik meg,
eredeti betűméreteiktől függetlenül.

    #NAGYBETŰS fájlnevek kisbetűvel# - minden nagybetűs fájlnév kisbetűvel
jelenik meg. Alapértelmezésben az opció be van kapcsolva. Ha a fájlok és
mappák neveit eredeti méretükben szeretnénk látni, kapcsoljuk ki, a "Mappák
NAGYBETŰVEL mutatva" és a "Fájlok kisbetűvel mutatva" opcióval együtt. Minden
említett betűméret beállítás csak a megjelenítésre hat, mivel a FAR eredeti
állapotukban hagyja és úgy is kezeli a fájlok és mappák betűméreteit.

    #Nagy/kisbetű érzékeny rendezés# - a rendezésnél figyelembe veszi a
fájlnevek betűméreteit.


@SortGroups
$ #Rendezési csoportok#
    A fájlok akkor rendezhetők csoportba, ha a ~fájlpanel~@FilePanel@
#név szerint# vagy #kiterjesztés szerint# rendezett. A csoportos rendezést a
#Shift-F11# kapcsolja ki vagy be. Rendezési csoportok definiálásával új
rendezési szabályokkal egészíthetjük ki a már létezőket.

    Minden csoport tartalmaz egy (vagy több, vesszővel elválasztott)
~fájlmaszkot~@FileMasks@. Ha egy rendezési csoport helyzete magasabb vagy
alacsonyabb egy másikénál, a fájlcsoport tagjai a fájlpanelen szintén feljebb
vagy lejjebb kerülnek, ugyanazt a hierarchiát követve, ahogyan helyzetük a
rendezési menüben az alattuk és fölöttük lévő csoportokhoz viszonyul.

    A rendezési szabályokat szerkeszthetjük, rendezhetjük, törölhetjük vagy új
szabályokat hozhatunk létre a ~Beállítások menü~@OptMenu@
~Fájlkiemelések, rendezési csoportok~@Highlight@ almenüjében.


@FileMasks
$ #Fájlmaszkok#
    A fájlmaszkokat a FAR parancsaiban gyakran használjuk fájlok, mappák vagy
ezek csoportjainak kijelölésére. A maszkok egyaránt tartalmazhatnak általános
érvényű fájlnév szimbólumokat, joker (* és ?) karaktereket és különleges
kifejezéseket:

    #*#           ^<wrap>bármilyen hosszúságú és tartalmú karaktersor (vagy
akár semmilyen karakter);

    #?#           egyetlen helyiértéknyi karakter;

    #[c,x-z]#     ^<wrap>a szögletes zárójelek közt álló bármelyik karakter.
Lehet egyedüli, lehet tartomány vagy a kettő kombinációja.

    Például az ftp.exe, fc.exe és az f.ext fájl az f*.ex? maszkkal írható le,
a *co* maszba belefér a color.ini és az edit.com is, a [c-f,t]*.txt maszknak
pedig a config.txt, demo.txt, faq.txt és a tips.txt egyaránt megfelel.

    Sok FAR parancs megengedi egyidejűleg több különféle maszk használatát,
vesszővel vagy pontosvesszővel elválasztva. Például a "Fájlok" menü "Csoport
kijelölése" parancsával kiválaszthatjuk a dokumentumokat, ha a
#*.doc,*.txt,*.wri# maszkot használjuk.

    Bármelyik maszkot idézőjelek közé lehet tenni, de a maszkok listáját nem.
Például ha a maszkban elválasztó karaktert szeretnénk használni ("," vagy ";"),
az idézőjelek használata elkerülhetetlen, nehogy a FAR maszkok listájaként
próbálja meg értelmezni a definíciót.

    Néhány parancsban (ilyen a ~fájlkeresés~@FindFile@, a ~szűrő~@Filter@,a
~szűrők menü~@FiltersMenu@, a fájlok ~kijelölése~@SelectFiles@, a fájlok
~társítása~@FileAssoc@ és a ~fájlkiemelések, rendezési csoportok~@Highlight@)
használhatunk kizáró maszkokat is. A #kizáró maszk# olyan fájlmaszk (vagy
maszkok csoportja), amivel a befoglaló maszknak megfelelő fájlok közül
kizárhatjuk egy másik maszk vagy maszkrendszer fájljait. A kizáró maszkoknak a
befoglaló maszkok után kell állniuk, #|# karakterrel elválasztva.

    Néhány példa a befoglaló és kizáró maszkok használatára:

    1. *.cpp
       Minden #cpp# kiterjesztésű fájl.
    2. *.*|*.bak,*.tmp
       Minden fájl, a #bak# és #tmp# kiterjesztésűeket kivéve.
    3. *.*|
       Ez a maszk hibát generál, mert a | karakter után nincs maszk.
    4. *.*|*.bak|*.tmp
       ^<wrap>Szintén hibás szintakszis, mert a | karakter egy sorban csak
egyszer szerepelhet.
    5. |*.bak
       Ugyanaz, mint a *|*.bak

    A "," (vagy ";") az egyes maszkokat, a "|" karakter pedig a befoglaló és a
kizáró maszkok csoportját választja el egymástól.


@SelectFiles
$ #Fájlok kijelölése#
    A ~fájlpanelek~@FilePanel@ fájljai és mappái kijelölésére többféle módszer
kínálkozik. Az #Ins# billentyű kijelöli a kurzor alatti fájlnevet, majd egyet
lefelé léptet, a #Shift-Kurzorvezérlőkkel# (kurzornyilak, PgUp, PgDn, Home,
End) pedig az adott kurzorvezérlő egységnyi lépéséig jelölhetünk ki fájlokat.

    A #Szürke +# és a #Szürke -# használatával csoportokat jelölhetünk ki vagy
a kijelölést levehetjük a csoportokról, ~fájlmaszkok~@FileMasks@
segítségével. A #Szürke *# megfordítja az aktuális kijelölést. A
#Jelölést visszatesz# parancs (#Ctrl-M#) visszaállítja az előző kijelölést.

    A #Ctrl-<Szürke +># és a #Ctrl-<Szürke -># kijelöli a kurzor alatti fájl
kiterjesztésével megegyező fájlokat, illetve leveszi róluk a kijelölést.

    Az #Alt-<Szürke +># és az #Alt-<Szürke -># kijelöli a kurzor alatti fájl
nevével megegyező fájlokat, illetve leveszi a kijelölést.

    A #Ctrl-<Szürke *># megfordítja a kijelöléseket a mappákon is. Ha a
~Panel beállítások~@PanelSettings@ menü #A mappák is kijelölhetők# opciója
be van kapcsolva, ugyanaz a hatása, mint a #Szürke *#-nak.

    A #Shift-<Szürke +># minden fájlt kijelöl, a #Shift-<Szürke -># minden
kijelölést levesz.

    Ha nincsenek kjelölt fájlok, a műveletek csak a kurzor alatti fájlra
hatnak.


@CopyFiles
$ #Másolás, mozgatás, átnevezés és linkek létrehozása#
    Fájlok és mappák másolására, mozgatására és átnevezésére a
 következő parancsokat használhatjuk:

  A ~kijelölt~@SelectFiles@ fájlok másolása                                    #F5#

  A kurzor alatti fájl másolása,
  a kijelöléstől függetlenül                              #Shift-F5#

  A kijelölt fájlok átnevezése vagy mozgatása                   #F6#

  A kurzor alatti fájl átnevezése vagy mozgatása,
  a kijelöléstől függetlenül                              #Shift-F6#

    Mappákon: ha a megadott (abszolút vagy relatív) elérési út
 létező mappára mutat, akkor a forrásmappát a célmappa belsejébe
 mozgatja. Ha nem, akkor a forrásmappát az új elérési útra nevezi
 át (vagy helyezi át).

    Példaként #c:\mappa1\#-et mozgassuk #d:\mappa2\#-re:
    - ha #d:\mappa2\# létezik, akkor #c:\mappa1\# tartalma átkerül
      #d:\mappa2\mappa1\# mappába;
    - ha nem létezik, akkor #c:\mappa1\# áthelyeződik (átneveződik)
      az újonnan létrehozott #d:\mappa2\# mappába (mappára).

  ~Fájl linkek~@HardSymLink@ létrehozása                                   #Alt-F6#

    Ha a "#Többszörös cél létrehozása#" opciót engedélyeztük, a beviteli
mezőben másolási vagy mozgatási célként több elérési utat is megadhatunk,
#;# vagy #,# karakterrel elválasztva egymástól. Ha a cél neve tartalmaz
";" vagy "," karaktert, idézőjelek közé kell tenni az elérési útjukat.

    Ha nem létező célmappákat adunk meg, amit szeretnénk létrehozni, a
mappák nevei közt és után mindig álljon \\-jel. A Másolás párbeszédablakban
#F10#-zel az aktív panel, #Alt-F10#-zel a passzív panel fastruktúrájának mappái
között kereshetünk célmappát. A #Shift-F10# a beviteli sorba beírt elérési út
fastruktúráját bontja ki (ha több útvonalat adtunk meg, azokból csak az elsőt).
Ha a "Többszörös cél létrehozása" opció engedélyezett, a fastruktúrán
kiválasztott mappa elérési útját a FAR hozzáfűzi a szerkesztett sorhoz.

    Pluginnel emulált fájlrendszereknél a másolás, mozgatás és átnevezés
lehetősége a plugin képességeitől függ.

    Ha a célfájl már létezik, felülírhatjuk, kihagyhatjuk vagy a
forrásfájl tartalmát hozzáfűzhetjük a célfájl végéhez.

    Ha a másolás vagy mozgatás során a céllemez megtelik, felfüggeszthetjük
a másolást vagy kicserélhetjük a lemezt és alkalmazhatjuk a "Felosztást".
Utóbbi esetben a FAR a másolt fájlt a céllemez méretére szeleteli fel. Ez az
opció csak akkor jelenik meg, ha a ~Rendszer beállítások~@SystemSettings@ menüben a
"Másoláshoz a rendszerrutin használata" opció ki van kapcsolva.

    A "Hozzáférési jogok" opció csak NTFS fájlrendszernél működik, vele a
fájlok hozzáférési információit is átmásolhatjuk. A másolás és mozgatás
alapértelmezett beállítása az "Alapértelmezett" opció: a FAR a háttérül
szolgáló rendszerre bízza a hozzáférési jogok kezelését.
Ha a "Másol" opciót választjuk, a fájlok és mappák az eredeti hozzáférési
jogaikat viszik magukkal. Az "Örököl" opcióval a másolt vagy mozgatott fájlok
és mappák a célmappa hozzáférési jogait öröklik.

    A #Már létező fájloknál# opcióban megadhatjuk, hogy mit tegyen a
FAR, ha azonos nevű fájllal találkozik a célhelyen.

    A legördülő listában a következő lehetőségek közül választhatunk:

    #Kérdez#    - ^<wrap>megjeleníti a
~Figyelem, a fájl már létezik...~@CopyAskOverwrite@ figyelmeztető színű
párbeszédablakot;
    #Felülír#   - a létező fájlt felülírja;
    #Kihagy#    - a létező fájlt nem írja felül;
    #Hozzáfűz#  - a létező fájl végéhez hozzáfűzi az új fájl
tartalmát;
    #Csak az újabb fájlokat# - csak a frissebb módosítási dátumú
fájlok írják felül a célhelyen létezőket;
    #Csak olvasható fájloknál is kérdez# - ha a célhelyen "csak
olvasható" attribútumú fájllal találkozik, a
felülírás előtt újabb megerősítést kér.

    A ~Rendszer beállítások~@SystemSettings@ menü "Másoláshoz a rendszerrutin
használata" opciójával a FAR a Windows operációs rendszer CopyFileEx nevű
(vagy CopyFile, ha a CopyFileEx nem áll rendelkezésre) másolórutinját fogja
használni a saját másolórutinja helyett. Ez előnyökkel járhat NTFS
fájlrendszernél, mert a CopyFileEx ésszerűbb lemezfoglalási metódust használ
és a fájlokat bővített attribútumkészletükkel együtt másolja át. A rendszer
másolórutinja nem használható akkor, ha a fájl titkosított és nem az
aktuális meghajtóra másolunk fájlt.

    A "Szimbolikus linkek másolása" opcióval megadhatjuk, hogy a FAR másolás
vagy mozgatás során milyen ~szabályok~@CopyRule@ szerint kezelje a
~szimbolikus linkeket~@HardSymLink@.

    Amikor a fájlok mozgatásánál a FAR megállapítja, hogy a művelethez
szükséges-e utólagos törlés, vagy elég a könyvtárbejegyzések módosítása
(ha azonos a forrás- és a célmeghajtó), figyelembe veszi a
~szimbolikus linkjeiket~@HardSymLink@ is.

    A FAR képes a #con#, a #nul# és a #\\\\.\\nul# eszközre másolni. Ez annyit
jelent, hogy adatokat ugyan olvas a lemezről, azokat mégsem írja ki sehova.

    Ha a #nul#, #\\\\.\\nul# vagy #con# eszközre mozgatunk fájlokat, a művelet
nem törli a forrásfájlokat a lemezről.

    A "Hozzáférési jogok" és a "Csak az újabb fájlokat" opciók beállításai csak
az aktuális másolási feladatra érvényesek, értéküket a FAR nem őrzi meg.

    A #Szűrővel# opció bekapcsolásával a szűrőfeltételeknek megfelelő fájlokat
másolhatjuk, a #Szűrő# gombbal pedig a ~Szűrők menüt~@FiltersMenu@ nyithatjuk
meg. Vegyük figyelembe, hogy ha olyan mappát másolnánk, amelynek fájljai közül
egyik sem felel meg a szűrőfeltételeknek, az üres mappát a FAR nem másolja át.


@CopyAskOverwrite
$ #Másolás: megerősítés felülírás előtt#
    Ha a másolni kívánt fájl a célhelyen már létezik, a megjelenő
párbeszédablakban a következő lehetőségek közül választhatunk:

    #Felülír#    - a forrással azonos nevű fájlt felülírja;

    #Kihagy#     - a forrással azonos nevű fájlt nem írja felül;

    #Hozzáfűz#   - ^<wrap>a létező fájl végéhez hozzáfűzi a forrásfájl
tartalmát.

    Ha másolás során bekapcsoljuk a #Mindent a kiválasztott módon# opciót,
a FAR megjegyzi választásainkat és az adott másolási feladat minden azonos
helyzetére alkalmazza.

    Ha úgy érezzük, hogy az új vagy a létező fájl mérete és dátuma
nem szolgál elég információval a helyes döntéshez, tartalmi eltéréseiket a
párbeszédablakan ellenőrizhetjük. Ha Entert ütünk vagy egérrel kattintunk
az "Új verzió" vagy a "Létező verzió" során, a fájlt a belső nézőke megnyitja.


@CopyRule
$ #Másolás: szabályok#
    A mappák és a ~szimbolikus linkek~@HardSymLink@ ~másolására/mozgatására~@CopyFiles@
a következő szabályok érvényesek:

    #Szimbolikus linkek másolása#

    Ha a "Szimbolikus linkek másolása" opció be van kapcsolva, vagy a másolás
cél- és forráslemeze távoli meghajtó, akkor a FAR egy mappát hoz létre a
célhelyen és belemásolja a forrás szimbolikus link tartalmát (önhivatkozással
a kapcsolt linkekhez).

    Ha a "Szimbolikus linkek másolása" opció ki van kapcsolva és a forrás és a
cél helyi meghajtó, akkor a céllemezen olyan szimbolikus link jön létre, ami a
forrás szimbolikus linkre mutat.

    #Szimbolikus linkek mozgatása#

    Ha a "Szimbolikus linkek másolása" opció be van kapcsolva, vagy a másolás
cél- és forráslemeze távoli meghajtó, akkor a FAR egy mappát hoz létre a
célhelyen és belemásolja a forrás szimbolikus link tartalmát (önhivatkozással
a kapcsolt linkekhez), majd a forráslinket törli.

    Ha a "Szimbolikus linkek másolása" opció ki van kapcsolva és a forrás és a
cél helyi meghajtó, akkor a FAR a forrás szimbolikus linket átmozgatja a célra.
Önhivatkozó öröklés a fastruktúrán ilyenkor nem történik.

    #Szimbolikus linkeket tartalmazó mappa mozgatása#

    Ha a forrás és a cél helyi meghajtó, akkor a FAR ugyanúgy helyezi át a
mappát, mint egy közönséges mappát.

    Ha a forrás vagy a cél távoli meghajtó, akkor a "Szimbolikus linkek
másolása" opció beállításától függetlenül a FAR egy mappát hoz létre a
célhelyen, belemásolja a forrás szimbolikus link tartalmát (önhivatkozással
a kapcsolt linkekhez), végül a forráslinket törli.


@HardSymLink
$ #Hardlinkek és szimbolikus linkek#
    NTFS partíciókon az #Alt-F6# paranccsal #hardlinkeket# hozhatunk létre
fájlokhoz, #csomópontokat# mappákhoz és #szimbolikus linkeket# mind
fájlokhoz, mind mappákhoz (Vista).

    #Hardlinkek#

    A #hardlink# fájlokhoz létrehozott kiegészítő könyvtárbejegyzés.
Ilyen esetben magát a fájlt nem másoljuk, mindössze alternatív fájlnév-
hivatkozásokat hozunk létre a fájl adattartalmához, így a fájl egy vagy több
másik helyen és másik néven is létezik, miközben eredeti nevén és helyén
érintetlenül megmarad. A hardlink létrejötte pillanatától
megkülönböztethetetlen az eredeti bejegyzéstől. Az egyetlen különbség,
hogy a hardlinkeknek nem lehet rövid fájlnevük, ezért a DOS-os programok
számára láthatatlanok.

    Ha egy ilyen fájl mérete vagy dátuma megváltozik, minden vele
összekapcsolt könyvtárbejegyzés önműködően frissül. Ha linkelt fájlt törlünk,
egészen addig nem törlődik fizikailag, amíg nem töröljük az összes hivatkozó
hardlinkjét. A törlés sorrendje nem fontos. Ha egy hardlinket a Lomtárba
dobunk, a fájl hardlinkjeinek száma változatlan marad.

    A FAR képes hardlinkeket készíteni, külön oszlopban megjeleníteni a
fájlokhoz tartozó hardlinkek számát (alapértelmezés szerint a 9-es nézet mód
utolsó oszlopában) és rendezni is tudja a fájlokat hardlinkjeik száma szerint.

    Hardlinkek kizárólag a forrásfájllal azonos meghajtón készíthetők.

    #Csomópontok#

    A mappa csomópont létrehozása olyan eljárást jelent, amellyel bármelyik
helyi mappát összerendelhetjük bármely másik helyi mappával. Például, ha a
D:\\SYMLINK mappa a C:\\WINNT\\SYSTEM32 mappára mutat, akkor az a
program, amelyik a D:\\SYMLINK\\DRIVERS mappához fér hozzá, valójában a
C:\\WINNT\\SYSTEM32\\DRIVERS mappát éri el. A hardlinkektől eltérően a
szimbolikus linkek célpontjainak nem kell a forrással azonos meghajtón lenniük.

    Windows 2000 alatt CD-ROM-ok mappáihoz közvetlenül nem készíthető szimlink,
de ez a korlátozás kikerülhető, ha a CD-ROM-ot egy NTFS partíció valamelyik
mappájához mountoljuk.

    #Szimbolikus linkek#

    Az NTFS a Windows Vista (NT 6.0) verziótól támogatja a szimbolikus
linkeket. Mivel a szimbolikus link a mappa csomópontok fejlettebb
változata, ezért az ilyen linkek fájlokra és nem-helyi mappákra is
mutathatnak, valamint relatív elérési útvonalak is használhatók.


@ErrCopyItSelf
$ #Hiba: nem másolható vagy mozgatható önmagára#
    Fájlt vagy mappát nem másolhatunk vagy mozgathatunk önmagára, mappát nem
másolhatunk vagy mozgathatunk önmaga belsejébe (saját almappájaként).

    Ez a hiba akkor is előfordulhat, ha két mappánk van és az egyik mappa a
másik ~szimbolikus linkje~@HardSymLink@.


@WarnCopyEncrypt
$ #Figyelem: a titkosítás el fog veszni#
    A forrásfájl titkosított, másik lemezre másolása vagy mozgatása csak akkor
lehetséges, ha a fájl a céllemezre titkosítatlanul kerülhet.

    A "Mégis" (vagy "Mégis mind") gombbal figyelmen kívül hagyhatjuk a
figyelmeztetést és a fájlt titkosítatlanul ugyan, de átmásolhatjuk.

    A FAR mindig az operációs rendszer belső másolási mechanizmusát használja
a titkosított fájlok aktuális lemezétől eltérő céllemezre másolásakor, a
"Másoláshoz a rendszerrutin használata" opció állapotától függetlenül.


@WarnCopyStream
$ #Figyelem: több streamet tartalmazó fájl másolása vagy mozgatása#
    A forrásfájl több adatstreamet tartalmaz, vagy a célhely fájlrendszere
nem támogatja az ilyen többszintű adatstruktúrával rendelkező fájlokat.

    A stream az NTFS fájlrendszer olyan lehetősége, amelyek a fájlokra
vonatkozó további információkat (például a szerző nevét, címet, kulcsszavakat
vagy egyéb adatokat) tartalmazhatnak. Ezek az adatok a fájlokkal
együtt tárolódnak el, de láthatatlanok az adatokat kiolvasni
és kezelni nem képes programok számára. Például a Windows Explorere a
streameket használja a fájlok járulékos információinak tárolására
(Summary = összegzés). A FAT/FAT32 fájlrendszer nem támogatja a streameket.

    Ha a fájlokat minden adatukkal együtt szeretnénk átmásolni (az összes
streamjükkel együtt), kapcsoljuk be a ~Rendszer beállítások~@SystemSettings@
menüben a "#Másoláshoz a rendszerrutin használata#" opciót.

    Ha nem NTFS fájlrendszerű lemezre másolunk több streamet tartalmazó
fájlokat, a művelet adatvesztéssel jár: csak a fájlok fő streamje másolódik
át.


@ErrLoadPlugin
$ #Hiba: plugin betöltési hiba#
   A hibaüzenet ezekben az esetekben jelenhet meg:

   1. ^<wrap>A plugin helyes működéséhez szükséges .dll fájl nem található
a rendszerben.

   2. ^<wrap>Valamilyen oknál fogva a plugin hibakóddal tér vissza és nem
engedi, hogy a plugin a rendszerbe töltődjön.

   3. A plugint képviselő .dll fájl hibás.


@ScrSwitch
$ #Képernyők váltása#
    A FAR-ban a belső nézőkét és a belső szerkesztőt egyidejűleg több
példányban használhatjuk és a #Ctrl-Tab#, #Ctrl-Shift-Tab# vagy az #F12#
billentyűkkel kapcsolgathatunk a panelek és az említett példányok között.
A #Ctrl-Tab# a következő, a #Ctrl-Shift-Tab# az előző képernyőre vált, az
#F12# pedig listát jelenít meg a megnyitott példányokról.

    \1B[1+3]════ C:\\\-    A háttérben futó nézőkék és szerkesztők
    \1B║\1E    Név     \-    példányszámát a FAR a bal panel bal
    \1B║\1F..          \-    felső sarkában jeleníti meg.

    A funkciót a ~Panel beállítások~@PanelSettings@ párbeszédablak
"Háttérképernyők száma mutatva" opciójával letilthatjuk.


@ApplyCmd
$ #Parancs végrehajtása#
    A ~Fájlok menü~@FilesMenu@ #Parancs végrehajtása# menüpontjával az összes
kijelölt fájlra közös parancsot adhatunk ki. A ~fájltársításoknál~@FileAssoc@
alkalmazható ~különleges szimbólumok~@MetaSymbols@ itt is használhatók.

    Például a "type !.!" parancs sorban egyenként a képernyőre irányítja a
kijelölt fájlok tartalmát, a "rar32 m !.!.rar !.!" parancs pedig a fájlnevekkel
megegyező nevű RAR tömörített fájlokba mozgatja a kijelölt fájlokat. Az
"explorer /select,!.!" parancs megnyitja a Windows Intézőt és ráállítja a
kurzort az aktuális fájlra vagy mappára.

    Lásd még ~Operációs rendszer parancsok~@OSCommands@.


@OSCommands
$ #Operációs rendszer parancsok#
    A FAR Manager önmaga is képes az operációs rendszer bizonyos parancsait
értelmezni. Ezek a következők:

    #CLS#

    A képernyő törlése.

    #MEGHAJTÓ BETŰJELE:#

    Az aktív panelt az aktuális meghajtóról a megadott betűjelű meghajtóra
váltja át.

    #CD [meghajtó:]elérési út# vagy #CHDIR [meghajtó:]elérési út#

    Az aktív panelt a megadott elérési útvonalra váltja. Ha a meghajtó
betűjelét is megadtuk, az aktuális meghajtó is megváltozik. Ha az
aktív panel ~pluginnel~@Plugins@ emulált fájlrendszert mutat, a "CD" paranccsal a
plugin fájlrendszerének mappái között mozoghatunk. A "CD" parancstól eltérően
a "CHDIR" mindig valódi mappaként kezeli az utána álló paramétert, a fájlpanel
jellegétől függetlenül.

    #CHCP [nnn]#

    Megjeleníti vagy beállítja az aktív kódlap számát (értéke "nnn"). A
paraméter nélküli CHCP parancs megjeleníti az aktív kódlap számát.

    #SET változó=[sztring]#

    A "változó" nevű környezeti változónak a "sztring" értéket adja. Ha az
egyenlőségjel után a "sztring" helyére nem írunk semmit, a "változó" nevű
környezeti változó törlődik. A FAR Manager indulásakor több
~környezeti változót~@FAREnv@ definiál.

    #IF [NOT] EXIST fájlnév parancs#

    Akkor hajtja végre a "parancs" nevű parancsot, ha a "fájlnév" létezik. A
"NOT" előtag hatására a parancs csak akkor hajtódik végre, ha a feltétel
hamis.

    #IF [NOT] DEFINED változó parancs#

    A DEFINED az EXIST-hez hasonlóan feltételesen működik, de nem
fájlnévtől függ a "parancs" végrehajtása, hanem attól, hogy a "változó" nevű
környezeti változó igaz vagy hamis értéket ad-e vissza, azaz létezik-e vagy
sem.

    Egész sor "IF" feltételt alkalmazhatunk, például a következő sor "parancs"
nevű parancsa

    #if exist fájl1 if not exist fájl2 if defined változó parancs#

    csak akkor hajtódik végre, ha "fájl1" fájl létezik, "fájl2" fájl nem
létezik és a "változó" nevű környezeti változó létezik.

    Megjegyzések:

    1. ^<wrap>A FAR a fentieken kívül minden más parancsot továbbít
az operációs rendszer parancsértelmezőjének.

    2. A fenti parancsok a következő helyeken működnek:
       ~Parancssor~@CmdLineCmd@
       ~Parancs végrehajtása~@ApplyCmd@
       ~Felhasználói menü~@UserMenu@
       ~Fájltársítások~@FileAssoc@


@FAREnv
$ #Környezeti változók#
    A FAR Manager indításakor a következő környezeti változókat definiálja
az utódfolyamatok részére:

    #FARHOME#          A mappa elérési útja, ahonnan a FAR indult.

    #FARLANG#          A kezelőfelület aktuális nyelve.

    #FARUSER#          ^<wrap>A ~parancssorban~@CmdLine@ a /u kapcsolóval
megadott felhasználói név.


@RegExp
$ #Regular expressions#


@ElevationDlg
$ #Запрос привилегий алминистратора#


@KeyMacro
$ #Makrók#
    A makrók (vagy másként billentyűmakrók) a billentyűleütések sorozatának
"felvételét" jelentik, amelyeket egyetlen gyorsbillentyű (egy billentyű vagy
billentyűkombináció) leütésével akárhányszor "visszajátszva" ismétlődő
feladatok ellátására használhatunk.

    Minden makrónak van:

    - ^<wrap>gyorsbillentyűje, ami elindítja az előzőleg rögzített
billentyűszekvencia végrehajtását;
    - további ~beállítási~@KeyMacroSetting@ lehetősége, amelyekkel
befolyásolhatjuk a végrehajtás módját és hatókörét.

    A makrók olyan különleges ~utasításokat~@KeyMacroLang@ is tartalmazhatnak,
amelyeket végrehajtásuk során a FAR speciális módon értelmez, így bonyolultabb
konstrukciókat is összeállíthatunk.

    A makrók általában a következő célokra praktikusak:

    1. ^<wrap>Ismétlődő feladatok ellátására a gyorsbillentyű korlátlan
számú leütésével.
    2. A makrók szövegében különleges utasításként megadható
funkciók végrehajtására.
    3. Az eredetileg a FAR belső parancsaihoz rendelt gyorsbillentyűk
alapértékeinek újradefiniálására.

    A makrók leggyakoribb alkalmazási területe gyorsbillentyűk hozzárendelése
külső pluginek meghívásához, illetve a FAR műveleteinek újradefiniálása.

    Lásd még:

    ~A makrók hatóköre~@KeyMacroArea@
    ~Gyorsbillentyűk~@KeyMacroAssign@
    ~Makrók rögzítése és visszajátszása~@KeyMacroRecPlay@
    ~Makrók törlése~@KeyMacroDelete@
    ~A makrók beállításai~@KeyMacroSetting@
    ~A makrók szövegében használható utasítások~@KeyMacroLang@


@KeyMacroArea
$ #Makrók: a végrehajtás hatóköre#
    A FAR lehetővé teszi, hogy hatókörönként azonos gyorsbillentyűvel induló,
de a hatókörtől függően eltérő működésű ~makrókat~@KeyMacro@ hozzunk létre.

    Vigyázat: ^<wrap>A végrehajtás hatókörét (ahol a makrót használhatjuk)
az a terület szabja meg, ahol a makró rögzítését #elindítottuk#.

    A jelenleg használható, egymástól elkülönített területek:

    - a fájlpanelek;
    - a belső nézőke;
    - a belső szerkesztő;
    - a párbeszédablakok;
    - a fájl gyorskeresés;
    - a meghajtók menü;
    - a főmenü;
    - egyéb menük;
    - a súgóablak;
    - az információs panel;
    - a gyorsnézet panel;
    - a fastruktúra panel;
    - a felhasználói menü;
    - a képernyőgrabber, vertikális menük.

    Foglalt gyorsbillentyűhöz nem rendelhetünk makrót. Ilyen próbálkozások
során figyelmeztető üzenetet kapunk: ha a foglalt gyorsbillentyűhöz új
makrót rendelünk, a régi makrószekvencia törlődik.

    A fent leírtakból belátható, hogy azonos gyorsbillentyűket csak eltérő
működési területeken rendelhetünk a különböző makrókhoz.


@KeyMacroAssign
$ #Makrók: gyorsbillentyűk#
    ~Makrókat~@KeyMacro@ rendelhetünk:

    1. bármelyik billentyűhöz;
    2. ^<wrap>bármelyik, módosítóval (#Ctrl#, #Alt# vagy #Shift#) együtt
lenyomott billentyűhöz;
    3. bármelyik két módosítóval együtt lenyomott billentyűhöz. A
lehetséges módosítópárok:  #Ctrl-Shift-<bill.>#, #Ctrl-Alt-<bill.># és
#Alt-Shift-<bill.>#

    A következő kombinációk #nem használhatók# makróhoz: #Alt-Ins#, #Ctrl-<.>#,
#Ctrl-Shift-<.>#, #Ctrl-Alt#, #Ctrl-Shift#, #Shift-Alt#, #Shift-<szimbólum>#.

    Néhány billentyűkombinációt és egérműveletet nem vihetünk be közvetlenül,
főleg az #Enter#, #Esc#, #F1#, #Ctrl-F5#, az #MsWheelUp#
(EgérGörgőFel) és az #MsWheelDown# (EgérGörgőLe) a #Ctrl#, a #Shift# és az
#Alt# módosítókkal, speciális funkcióik miatt. Ezeket legördülő
listából választhatjuk ki és rendelhetjük a makróhoz.


@KeyMacroRecPlay
$ #Makrók: rögzítés és visszajátszás#
    A ~makrók~@KeyMacro@ a következő módokban játszhatók vissza:

    1. ^<wrap>Normál mód: a felvétel vagy lejátszás közben lenyomott
billentyűket #elküldi# a pluginekhez.

    2. ^<wrap>Különleges mód: a felvétel vagy lejátszás közben lenyomott
billentyűket #nem küldi el# a szerkesztés eseményeit kezelő pluginekhez.

    Például, ha valamelyik plugin normál módban lekezeli a #Ctrl+A#
kombinációt, különleges módban "nem kerülhet a látókörébe", így nem is reagál
rá a szokott módon.

    A makrókat a következő lépésekben hozhatjuk létre:

    1. Makrórögzítés inditása

       ^<wrap>Normál módú makrófelvételhez nyomjuk le a #Ctrl-<.># (először
a #Ctrl# és utána rövid ideig vele kell nyomni a #<.>#-ot), a különleges módú
makrófelvételhez pedig a #Ctrl-Shift-<.># kombinációt (#Ctrl# és #Shift#,
utána rövid ideig velük kell nyomni a #<.>#-ot).

       ^<wrap>Bármelyik módú felvételt indítottuk ek, a makrórögzítésről
tájékoztató \4FR\- szimbólum megjelenik a képernyő bal felső sarkában.

    2. A makrók tartalma

       ^<wrap>Makrórögzítés során a FAR minden billentyűleütést tárol, a
következő kivétellel:

         ^<wrap>A FAR csak a közvetlenül általa feldolgozott műveleteket
jegyzi meg. Ez annyit jelent, hogy ha rögzítés közben egy külső program indul
el az aktuális konzolban, a FAR csak a program futása előtti és utáni
billentyűműveleteket tárolja a makróban.

    3. Makrórögzítés befejezése

       ^<wrap>A felvételek leállításához ki kell választanunk a célnak
megfelelőbb módszert. Mivel a makrók a rögzítés leállítása után is
konfigurálhatók, kétféle megoldás kínálkozik: a #Ctrl-<.># és a
#Ctrl-Shift-<.>#. Az első esetben a makró leállítása után csak gyorsbillentyűt
kell megadni és a makró az alapértelmezett beállítások szerint játszható
vissza. A második esetben is megtörténik a leállítás és a billentyű
hozzárendelése, de ezután egy párbeszédablakban módosíthatjuk a makró futási
feltételeinek ~beállításait~@KeyMacroSetting@.

    4. Gyorsbillentyű hozzárendelése a makróhoz

       ^<wrap>A makrórögzítés befejeztével a
~gyorsbillentyű hozzárendelés~@KeyMacroSetting@ párbeszédablak jelenik meg,
ahol kiválaszthatjuk a makrót indító billentyűkombinációt.


@KeyMacroDelete
$ #Makrók: makró törlése#
    A ~makrók~@KeyMacro@ törlését egy üres, utasításokat nem tartalmazó makró
rögzítésével tehetjük meg. Felvétel után rendeljük hozzá a törölni kívánt
makró gyorsbillentyűjét és a törlésre vonatkozó kérdésre adjunk igenlő
választ.

    Lépésekre bontva:

    1. Indítsuk el a makrórögzítést (#Ctrl-<.>#).
    2. Állítsuk le a rögzítést (#Ctrl-<.>#).
    3. ^<wrap>Nyomjuk le, vagy a listából válasszuk ki a törölni kívánt
gyorsbillentyűt.

    Vigyázat:  ^<wrap>a makró törlése után a gyorsbillentyű visszanyeri
eredeti funkcióját, tehát ha azelőtt a FAR vagy egy plugin kezelte a
billentyűkombinációt, az a korábbi beállításnak megfelelően működik ezután.


@KeyMacroSetting
$ #Makrók: beállítások#
    Ha a ~makrók~@KeyMacro@ egyéb tulajdonságait is szeretnénk módosítani,
a felvétel leállítására a #Ctrl-<.># helyett használjuk a #Ctrl-Shift-<.>#
kombinációt, így a rögzítést követően egy párbeszédablakban állíthatjuk be
a kívánt jellemzőket:

   #Szekvencia:#

    Itt szerkeszthető a rögzített billentyűsorozat.

   #Képernyőkimenet a makró futása közben#

    Ha az opció nincs bekapcsolva, a makró végrehajtása közben a FAR nem
frissíti a képernyőtartalmat és minden változás csak a visszajátszás után
jelenik meg a képernyőn.

   #Végrehajtás a FAR indítása után#

    Az így megjelölt makrók közvetlenül a FAR elindulása után végrehajtódnak.

    A végrehajtást a következő feltételekhez köthetjük, külön az aktív és a
passzív panelre:

     #Ha plugin panel#
         [x] - csak ha az aktuális panel plugin panel
         [ ] - csak ha az aktuális panel fájlpanel
         [?] - a panel típusától függetlenül

     #Ha mappa#
         [x] - csak ha a sávkurzor mappán áll
         [ ] - csak ha a sávkurzor fájlon áll
         [?] - mindkét esetben hajtsa végre

     #Ha van kijelölés#
         [x] - ^<wrap>csak ha van kijelölt fájl vagy mappa a panelen
         [ ] - csak ha nincs kijelölt fájl vagy mappa a panelen
         [?] - a kijelöléstől függetlenül

   További végrehajtási feltételek:

     #Ha üres a parancssor#
         [x] - csak ha a parancssor üres
         [ ] - csak ha a parancssor nem üres
         [?] - a parancssor állapotától függetlenül

     #Ha van kijelölt blokk#
         [x] - ^<wrap>csak ha a nézőke, a szerkesztő, a parancssor vagy a
párbeszédablak szövegsorában van kijelölt blokk
         [ ] - csak ha nincs kijelölt blokk
         [?] - a kijelöléstől függetlenül


   Megjegyzések:

    1. ^<wrap>A makró végrehajtása előtt a FAR minden fenti feltételt
ellenőriz.

    2. ^<wrap>Egyes billentyűkombinációkat, például az #Enter#, #Esc#, #F1# és
a #Ctrl-F5#; az #MsWheelUp# (EgérGörgőFel). az #MsWheelDown# (EgérGörgőLe),
valamint más egérgomb műveleteket a #Ctrl#, #Shift# és #Alt# módosítóval
együtt nem vihetünk be közvetlenül gyorsbillentyűként, a párbeszédablakban
betöltött speciális szerepük miatt. Ezeket a billentyűkombinációkat
legördülő listából választhatjuk ki és rendelhetjük makrókhoz.


@KeyMacroLang
$ #Makrók: makrónyelv#
    A FAR Managerbe alapszintű makrónyelv van beépítve, amellyel logikai
utasításokat illeszthetünk az egyszerű billentyűszekvenciák közé, ez teszi
a makrórögzítést a ~pluginek~@Plugins@ lehetőségei mellett a FAR Managerrel
végzett mindennapi munka hatékony segédeszközévé.

      Néhány utasítás, felsorolásszerűen:

    #$Exit#         - leállítja a makró lejátszását
    #$Text#         - tetszőleges szöveg beszúrása
    #$XLat#         - transzliteráló funkció
    #$If-$Else#     - feltétel operátor
    #$While#        - feltételciklus operátor
    #$Rep#          - ciklus operátor
    #%var#          - változók használata

      és így tovább...

    Makrónyelvi utasításokat csak a Windows regisztrációs adatbázisának
szerkesztésével (HKEY_CURRENT_USER\\Software\\Far2\\KeyMacros) vagy az
erre a célra kifejlesztett segédprogramokkal és pluginekkel adhatunk a
~makrókhoz~@KeyMacro@.

    A makrónyelv leírása megtalálható a kísérő dokumentációban.


@KeyMacroEditList
$ #Makrók: a szerkesztő makrói#
    A következő lista a jelenleg a szerkesztőből elérhető makrók
gyorsbillentyűit tartalmazza, a Windows regisztrációs
adatbázisából kiolvasott leírásaikkal együtt:

<!Macro:Common!>
<!Macro:Editor!>

@KeyMacroViewerList
$ #Makrók: a nézőke makrói#
    A következő lista a jelenleg a nézőkéből elérhető makrók
gyorsbillentyűit tartalmazza, a Windows regisztrációs
adatbázisából kiolvasott leírásaikkal együtt:

<!Macro:Common!>
<!Macro:Viewer!>


@Index
$ #A súgó betűrendes tartalomjegyzéke#
<%INDEX%>
