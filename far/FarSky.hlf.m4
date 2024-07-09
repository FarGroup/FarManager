m4_include(`farversion.m4')m4_dnl
.Language=Slovak,Slovak (Slovenčina)
.Options CtrlColorChar=\
.Options CtrlStartPosChar=^<wrap>


@CodepagesSettings=CodepagesMenu
@ConfirmationsSettings=ConfirmDlg
@DescriptionsSettings=FileDiz
@Editor.WordDiv=System.WordDiv
@History.CommandHistory.Count=History.Config
@History.CommandHistory.Lifetime=History.Config
@History.DialogHistory.Count=History.Config
@History.DialogHistory.Lifetime=History.Config
@History.FolderHistory.Count=History.Config
@History.FolderHistory.Lifetime=History.Config
@History.ViewEditHistory.Count=History.Config
@History.ViewEditHistory.Lifetime=History.Config
@Interface.CompletionSettings=AutoCompleteSettings
@Interface.CursorSize1=Interface.CursorSize
@Interface.CursorSize2=Interface.CursorSize
@Interface.CursorSize3=Interface.CursorSize
@Interface.CursorSize4=Interface.CursorSize
@Interface.EditorTitleFormat=TitleFormat
@Interface.ViewerTitleFormat=TitleFormat
@InterfaceSettings=InterfSettings
@Panel.InfoSettings=InfoPanel
@Panel.Layout.DoubleGlobalColumnSeparator=PanelSettings
@Panel.Left=PanelCmdSort
@Panel.Right=PanelCmdSort
@Panel.Tree.AutoChangeFolder=TreeSettings
@Panel.Tree.MinTreeCount=TreeSettings
@ScreenSettings=InterfSettings
@System.MsHWheelDelta=System.MsWheelDelta
@System.MsHWheelDeltaEdit=System.MsWheelDelta
@System.MsHWheelDeltaView=System.MsWheelDelta
@System.MsWheelDeltaEdit=System.MsWheelDelta
@System.MsWheelDeltaHelp=System.MsWheelDelta
@System.MsWheelDeltaView=System.MsWheelDelta
@Viewer.F8CPs=Editor.F8CPs
@XLat.Rules1=XLat.Rules
@XLat.Rules2=XLat.Rules
@XLat.Rules3=XLat.Rules
@XLat.Table1=XLat.Tables
@XLat.Table2=XLat.Tables


@Contents
$^#Manažér súborov a archívov#
$^#M4_MACRO_GET(FULLVERSION)#
$^#Copyright © 1996-2000 Eugene Roshal#
$^#Copyright © 2000-M4_MACRO_GET(COPYRIGHTYEAR) Far Group#
 ~Help file index~@Index@
 ~Ako používať nápovedu~@Help@

 ~O programe Far~@About@
 ~License~@License@

 ~Prepínače použiteľné z príkazového riadku~@CmdLine@
 ~Klávesové skratky~@KeyRef@
 ~Podpora prídavných modulov~@Plugins@
 ~Overview of plugin capabilities~@PluginsReviews@

 ~Panely~@Panels@:
  ~Súborový panel~@FilePanel@
  ~Stromový panel~@TreePanel@
  ~Informačný panel~@InfoPanel@
  ~Panel rýchleho prezerania~@QViewPanel@
  ~Ťahaj a pusť súbory~@DragAndDrop@
  ~Úprava zobrazovacích módov v súborovom paneli~@PanelViewModes@
  ~Označovanie súborov~@SelectFiles@

 ~Menu~@Menus@:
  ~Menu Ľavý a Pravý~@LeftRightMenu@
  ~Menu Súbory~@FilesMenu@
  ~Menu Príkazy~@CmdMenu@
  ~Menu Nastavenia~@OptMenu@

 ~Nájdi súbor~@FindFile@
 ~História~@History@
 ~Vyhľadanie adresára~@FindFolder@
 ~Porovnanie adresárov~@CompFolders@
 ~Užívateľské menu~@UserMenu@
 ~Zmena jednotky~@DriveDlg@

 ~Typy súborov~@FileAssoc@
 ~Operating system commands~@OSCommands@
 ~Skratky k adresárom~@FolderShortcuts@
 ~Skupiny triedenia~@SortGroups@
 ~Filter zobrazených súborov~@FiltersMenu@
 ~Prepínanie obrazoviek~@ScrSwitch@
 ~Zoznam úloh~@TaskList@
 ~Hotplug devices list~@HotPlugList@

 ~Nastavenia systému~@SystemSettings@
 ~Nastavenia panelov~@PanelSettings@
 ~Tree settings~@TreeSettings@
 ~Nastavenia prostredia~@InterfSettings@
 ~Dialog settings~@DialogSettings@
 ~Menu settings~@VMenuSettings@
 ~Command line settings~@CmdlineSettings@

 ~Zvýrazňovanie súborov~@Highlight@
 ~Popis súborov~@FileDiz@
 ~Nastavenie prezerača~@ViewerSettings@
 ~Nastavenie editora~@EditorSettings@

 ~Kopírovanie, presúvanie, premenovávanie a vytváranie linkov~@CopyFiles@

 ~Zabudovaný prezerač~@Viewer@
 ~Zabudovaný editor~@Editor@

 ~Masky súborov~@FileMasks@
 ~Klávesové makrá~@KeyMacro@

 ~Customizing UI elements~@CustomizingUI@


@Help
$ #Ako používať nápovedu#
 Obrazovky nápovedy môžu obsahovať odkazy na inú obrazovku nápovedy.
Also, the main page has the "~Help Index~@Index@", which lists all the
topics available in the help file and in some cases helps to find the needed
information faster.

 Použitím klávesov #Tab# a #Shift+Tab# môžete presúvať kurzor medzi týmito
odkazmi a potom stlačením klávesu Enter sa dostanete na obrazovku popisujúcu
danú položku. S myšou sa presuniete pomocou kliknutia na daný odkaz.

 Ak sa text nezmestí celý do okna nápovedy, zobrazí sa posuvník. Vtedy
možno na pohyb textu použiť #kurzorové klávesy#.

 Stlačením #Alt+F1# alebo #BS# (#BackSpace#) sa dostanete na predošlú
obrazovku a stlačením #Shift+F1# na obsah nápovedy.

 Pre nápovedu k ~prídavným modulom~@Plugins@ stlačte #Shift+F2#.

 Press #F7# to search for text in the current help file. Search results
will be displayed as links to relevant topics.

 #Help# is shown by default in a reduced windows, you can maximize it by
pressing #F5# "#Zoom#", pressing #F5# again will restore the window to the
previous size.


@About
$ #O programe Far#
 #Far# je manažér súborov a archívov pre Windows, pracujúci v textovom móde.
Podporuje #dlhé mená súborov# a ponúka širokú paletu operácií so súbormi a adresármi.

 #Far# is #freeware# and #open source# software distributed under the
revised BSD ~license~@License@.

 Far ponúka transparentnú prácu s #archívmi#. So súbormi v archívoch sa
pracuje podobne ako v adresári: keď pracujete s archívom, Far transformuje
vaše príkazy do zodpovedajúcich volaní externých archivačných programov.

 Far taktiež obsahuje množstvo servisných funkcií.


@License
$ #Far: License#
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

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


@CmdLine
$ #Prepínače použiteľné z príkazového riadku#
 V príkazovom riadku možno použiť nasledovné prepínače:

 #-e[<riadok>[:<pozícia>]] <názov súboru>#
 Edituj určený súbor. Po /e môžete voliteľne určiť na ktorom riadku
a stĺpci má editor umiestniť kurzor.
 Napríklad: far -e70:2 citaj.ma

 #-p[<path>]#
 Search for "main" plugins in the folder given in <path>.
 Several search paths can be specified, separated by ‘;’.

 Example: #far -p%USERPROFILE%\\Far\\Plugins#

 #-co#
 Forces Far to load plugins from cache only. Plugins are loaded faster this way,
but new or changed plugins are not discovered. Should be used #only# with a stable
list of plugins. After adding, replacing or deleting a plugin Far should be loaded
without this switch. If the cache is empty, no plugins will be loaded.

 Remarks about switches -p and -co:
 - ^<wrap>if -p is empty, then Far will be loaded with no plugins;
 - ^<wrap>if -p is given with a <path>, then only plugins from <path> will be loaded;
 - ^<wrap>if only the -co switch is given and plugins cache is not empty, then plugins
will be loaded from cache;
 - ^<wrap>-co is ignored, if -p is given;
 - ^<wrap>if -p and -co are not given, then plugins will be loaded from the 'Plugins'
folder, which is in the same folder as Far.exe, and the 'Plugins' folder, which is in the
user profile folder (#%APPDATA%\\Far Manager\\Profile# by default).

 #-m#
 Far will not load macros when started.

 #-ma#
 Macros with the "Run after Far start" option set will not be run when Far is started.

 #-s <profilepath> [<localprofilepath>]#
 Custom location for Far configuration files (overrides the ini file).

 #-u <názov užívateľa>#
 Dovoľuje mať oddelené nastavenia pre rôznych užívateľov.
 Affects only 1.x Far Manager plugins
 Napríklad: far -u guest

 Far Manager will set the ~environment variable~@FAREnv@ "FARUSER" to the value <username>.

 #-v <názov súboru>#
 Načíta do prezerača špecifikovaný súbor. Ak je <názov súboru> `#-#', dáta sú načítané zo štandardného vstupu (stdin).

 Napríklad, "dir|far -v -" načíta do prezerača výstup príkazu dir.

 If the input stream is empty when using ‘-’ (for example, you have not specified
the "dir" command in the provided example), Far will wait for the end of data
in the input stream until you press Ctrl+Break.

 #-w[-]#
 Show the interface within the console window instead of the console buffer or vice versa.

 #-t templateprofile#
 Location of Far template configuration file (overrides the ini file).

 #-title[:<title>]#
 If <title> string is provided, use it as the window title; otherwise
inherit the console window's title. Macro #%Default# in the custom title
string will be replaced with the standard context-dependent Far window's
title.

 #-clearcache [profilepath [localprofilepath]]#
 Clear plugins cache.

 #-export <out.farconfig> [profilepath [localprofilepath]]#
 Export settings to file out.farconfig.

 #-import <in.farconfig> [profilepath [localprofilepath]]#
 Import settings from file in.farconfig.

 #-ro[-]#
 Read-only or normal config mode (overrides the ini file).

 #-set:<parameter>=<value>#
 Override the configuration parameter, see ~far:config~@FarConfig@ for details.

 #-x#
 Disable exception handling. This parameter is for developers
and is not recommended for normal operations.

 It is possible to specify at most two paths (to folders, files or archives) or
two commands with plugin prefix in the command line. The first path applies to the
active panel, the second path - to the passive one:
 - ^<wrap>if a folder or archive is specified, Far will show its contents;
 - ^<wrap>if a file is specified, Far will change to the folder where it
resides and place the cursor on the file, if it exists;
 - ^<wrap>when prefixes specified (simultaneous use with common paths allowed)
passive command executes first (passive panel activates temporary).
Single letter prefixes A-Z or conflicted with disk letter will be ignored.
 Example: far arc:c:\\Far20.7z "lua:msgbox('Far Manager','Successfully started')"


@KeyRef
$ #Klávesové skratky#
 ~Príkazy na ovládanie panelov~@PanelCmd@

 ~Príkazy pre príkazový riadok~@CmdLineCmd@

 ~Príkazy pre prácu so súbormi a servisné príkazy~@FuncCmd@

 ~Mouse: wheel support~@MsWheel@

 ~Menu control commands~@MenuCmd@

 ~Screen grabber~@Grabber@

 ~Ostatné~@MiscCmd@

 ~Special commands~@SpecCmd@


@MenuCmd
$ #Menu control commands#
 #Common menu and drop-down list commands#

 #Ctrl+Alt+F#, #RAlt#
 Filter menu or list items.

 #Ctrl+Alt+L#
 Lock filter.

 #Alt+Left#, #Alt+Right#, #MsWheelLeft#, #MsWheelRight#
 Scroll all items horizontally.

 #Alt+Shift+Left#, #Alt+Shift+Right#
 Scroll the selected item horizontally.

 #Ctrl+Alt+Left#, #Ctrl+Alt+Right#, #Ctrl+MsWheelLeft#, #Ctrl+MsWheelRight#
 Scroll all items horizontally by 20 characters.

 #Ctrl+Shift+Left#, #Ctrl+Shift+Right#
 Scroll the selected item horizontally by 20 characters.

 #Alt+Home#
 Align all items to the left.

 #Alt+End#
 Align all items to the right.

 #Alt+Shift+Home#
 Align the selected item to the left.

 #Alt+Shift+End#
 Align the selected item to the right.

 See also the list of ~macro keys~@KeyMacroMenuList@, available in the menus.


@PanelCmd
$ #Príkazy na ovládanie panelov#
 #Základné príkazy panelu#

 Zmeň aktívny panel                                             #Tab#
 Vymeň panely                                                #Ctrl+U#
 Aktualizuj panel (znovu načítaj)                            #Ctrl+R#
 Zap./vyp. informačný panel                                  #Ctrl+L#
 Zap./vyp. ~panel rýchleho prezerania~@QViewPanel@                         #Ctrl+Q#
 Zap./vyp. ~stromový panel~@TreePanel@                                    #Ctrl+T#
 Schovaj/zobraz oba panely                                   #Ctrl+O#
 Temporarily hide both panels                        #Ctrl+Alt+Shift#
 (as long as these keys are held down)
 Schovaj/zobraz neaktívny panel                              #Ctrl+P#
 Schovaj/zobraz ľavý panel                                  #Ctrl+F1#
 Schovaj/zobraz pravý panel                                 #Ctrl+F2#
 Zmeň výšku panelov                           #Ctrl+Nahor,Ctrl+Nadol#
 Change current panel height          #Ctrl+Shift+Up,Ctrl+Shift+Down#
 Zmeň šírku panelov                              #Ctrl+Doľ,Ctrl+Dopr#
 (ak je prík. riadok prázdny)
 Vráť štandartnú šírku panelov                        #Ctrl+NumKlav5#
 Restore default panels height                     #Ctrl+Alt+Numpad5#
 Show/Hide functional key bar at the bottom line.            #Ctrl+B#
 Toggle total and free size show mode                  #Ctrl+Shift+S#
 in bytes (if possible) or with size suffixes K/M/G/T

 #Súborové príkazy panelu#

 ~Označ/odznač súbor~@SelectFiles@                    #Ins, Shift+Kurzorové klávesy#
                                                 #Right mouse button#
 Označ skupinu                                               #Sivý +#
 Odznač skupinu                                              #Sivý -#
 Invertuj označenie                                          #Sivý *#
 Označ súbory s príponou ako má aktuálny súbor        #Ctrl+<Sivý +>#
 Odznač súbory s príponou ako má aktuálny súbor       #Ctrl+<Sivý ->#
 Invertuj označenie vrátane adresárov                 #Ctrl+<Sivý *>#
 Označ súbory s menom ako má aktuálny súbor            #Alt+<Sivý +>#
 Odznač súbory s menom ako má aktuálny súbor           #Alt+<Sivý ->#
 Invert selection on files, deselect folders           #Alt+<Gray *>#
 Označ všetky súbory                                 #Shift+<Sivý +>#
 Odznač všetky súbory                                #Shift+<Sivý ->#
 Obnov predchádzajúce označenie                              #Ctrl+M#

 Skroluj dlhé mená súborov a popisy          #Alt+Doľava,Alt+Doprava#
                                                   #Alt+Home,Alt+End#

 Nastav "stručný" mód zobrazenia                         #ĽavýCtrl+1#
 Nastav "stredný" mód zobrazenia                         #ĽavýCtrl+2#
 Nastav "úplny" mód zobrazenia                           #ĽavýCtrl+3#
 Nastav "široký" mód zobrazenia                          #ĽavýCtrl+4#
 Nastav "detailný" mód zobrazenia                        #ĽavýCtrl+5#
 Nastav mód zobrazenia popisov                           #ĽavýCtrl+6#
 Nastav mód zobrazenia dlhých popisov                    #ĽavýCtrl+7#
 Mastav mód zobrazenia vlastníkov súborov                #ĽavýCtrl+8#
 Mastav mód zobrazenia liniek súborov                    #ĽavýCtrl+9#
 Mastav alternatívny "plný" mód zobrazenia               #ĽavýCtrl+0#

 Zap./vyp. zobrazovanie skrytých a systémových súborov       #Ctrl+H#
 Zap./vyp. zobrazovanie dlhých mien súborov                  #Ctrl+N#

 Schovaj/zobraz ľavý panel                                  #Ctrl+F1#
 Schovaj/zobraz pravý panel                                 #Ctrl+F2#

 Zoraď súbory aktívneho panelu podľa mena                   #Ctrl+F3#
 Zoraď súbory aktívneho panelu podľa prípony                #Ctrl+F4#
 Zoraď súbory aktívneho panelu podľa času modifikácie       #Ctrl+F5#
 Zoraď súbory aktívneho panelu podľa dĺžky                  #Ctrl+F6#
 Nechaj súbory aktívneho panelu podľa nezoradené            #Ctrl+F7#
 Zoraď súbory aktívneho panelu podľa času vytvorenia        #Ctrl+F8#
 Zoraď súbory aktívneho panelu podľa času prístupu          #Ctrl+F9#
 Zoraď súbory aktívneho panelu podľa času popisu           #Ctrl+F10#
 Zoraď súbory aktívneho panelu podľa vlastníka             #Ctrl+F11#
 Zobraz menu ~zoraďovacích módov~@PanelCmdSort@                            #Ctrl+F12#
 Používaj zoraďovanie skupín                              #Shift+F11#
 Zobrazuj označené súbory ako prvé                        #Shift+F12#

 Create a ~folder shortcut~@FolderShortcuts@                            #Ctrl+Shift+0…9#
 Jump to a folder shortcut                            #RightCtrl+0…9#

 If the active panel is a ~quick view panel~@QViewPanel@, a ~tree panel~@TreePanel@ or
an ~information panel~@InfoPanel@, the directory is changed not on the
active, but on the passive panel.

 #Ctrl+Ins#
 Copy names of the selected files to clipboard (if the command line is empty).

 #Ctrl+Shift+Ins#
 Skopíruj označené mená súborov do schránky.

 #Alt+Shift+Ins#
 Copy full names of selected files to clipboard.

 #Ctrl+Alt+Ins#
 Copy real names of selected files to clipboard.

 #Ctrl+Shift+C#
 Copy the selected files to clipboard.

 #Ctrl+Shift+X#
 Cut the selected files to clipboard.

 Files, copied or cut from the panels, can be pasted to other applications, e.g. Explorer.

 See also the list of ~macro keys~@KeyMacroShellList@, available in the panels.

 Poznámky:

 1. ^<wrap>Ak máte nastavené "Povoliť triedenie v opačnom poradí" v ~nastavení panelov~@PanelSettings@,
opakované stláčanie rovnakého klávesu zoraďovania vedie k zmene
spôsobu zoraďovania zo vzostupného na zostupné a naopak;

 2. ^<wrap>Kombinácie Alt+Doľava a Alt+Doprava, používané na skrolovanie
dlhých mien a popisov nepracujú s klávesmi na numerickej
klávesnici. Je to preto, lebo keď stlačíte Alt, čísla na nej sa
používajú na vkladanie znakov pomocou ich číselných kódov.

 3. ^<wrap>The key combination #Ctrl+Alt+Ins# puts the following text into the clipboard:
    - ^<wrap>for network drives: the network (UNC) name of the file object;
    - ^<wrap>for local drives: the local name of the file taking into account
~symbolic links~@HardSymLink@.

 4. ^<wrap>If #Ctrl+Ins#, #Alt+Shift+Ins# or #Ctrl+Alt+Ins# is pressed when the cursor
is on the file "#..#", the name of the current folder is copied.


@PanelCmdSort
$ #Sort modes#
 The sort modes menu is called by #Ctrl+F12# and applies to the currently
active panel. The following sort modes are available:

 Sort files by name                                         #Ctrl+F3#
 Sort files by extension                                    #Ctrl+F4#
 Sort files by last write time                              #Ctrl+F5#
 Sort files by size                                         #Ctrl+F6#
 Keep files unsorted                                        #Ctrl+F7#
 Sort files by creation time                                #Ctrl+F8#
 Sort files by access time                                  #Ctrl+F9#
 Sort files by description                                 #Ctrl+F10#
 Sort files by file owner                                  #Ctrl+F11#

 The #+# key sets the sorting order to be direct.
 The #-# key sets the sorting order to be reversed.
 The #*# key changes the sorting order to be reversed.

 Use group sorting                                        #Shift+F11#
 Show selected files first                                #Shift+F12#

 You can ~fine-tune~@PanelSortCriteria@ sort modes by pressing #F4#.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@PanelSortCriteria
$ #Sort criteria#
 When files are considered equivalent using the selected sort mode, additional sort criteria are taken into account.
 For example, if files are sorted by size and both "a.txt" and "b.txt" have the same size, "a.txt" will come first, as if they were sorted by name.
 In this menu you can adjust the set of criteria associated with the selected sort mode.

 #Ins#
 Add a criterion to the set.

 #Del#
 Remove the selected criterion.

 #F4#
 Replace the selected criterion.

 #+#
 Use ascending order.

 #-#
 Use descending order.

 #*#
 Change the order.

 #=#
 Inherit the order from the corresponding sort mode.

 #Ctrl+Up#
 Move the criterion up.

 #Ctrl+Down#
 Move the criterion down.

 #Ctrl+R#
 Reset the set of criteria to default.


@FastFind
$ #Fast find in panels#
 To locate a file quickly, you can use the #fast find# operation and enter
the starting characters of the file name. In order to use that, hold down the
#Alt# (or #Alt+Shift#) keys and start typing the name of the needed file, until
the cursor is positioned to it.

 By pressing #Ctrl+Enter#, you can cycle through the files matching the part
of the filename that you have already entered. #Ctrl+Shift+Enter# allows to
cycle backwards.

 Besides the filename characters, you can also use the wildcard characters
‘*’ and ‘?’.

 Insertion of text, pasted from clipboard (#Ctrl+V# or #Shift+Ins#), to the
fast find dialog will continue as long as there is a match found.

 It is possible to use the transliteration function while entering text in
the search field. If used the entered text will be transliterated and a new
match corresponding to the new text will be searched. See ~XLat.Flags~@XLat.Flags@ on how
to set the hotkey for the transliteration.

 See also the list of ~macro keys~@KeyMacroSearchList@, available in fast find.


@CmdLineCmd
$ #Príkazy pre príkazový riadok#
 #Common command line commands#

 Posun o znak doľava                                  #Doľava,Ctrl+S#
 Posun o znak doprava                                #Doprava,Ctrl+D#
 Posun o slovo doľava                                   #Ctrl+Doľava#
 Posun o slovo doprava                                 #Ctrl+Doprava#
 Presun na začiatok riadku                                #Ctrl+Home#
 Presun na koniec riadku                                   #Ctrl+End#
 Vymazanie znaku pod kurzorom                                   #Del#
 Vymazanie znaku pred kurzorom (naľavo od kurzora)               #BS#
 Vymazanie po koniec riadku                                  #Ctrl+K#
 Vymazanie slova naľavo                                     #Ctrl+BS#
 Vymazanie slova napravo                                   #Ctrl+Del#
 Skopíruj do schránky                                      #Ctrl+Ins#
 Vlož zo schránky                                         #Shift+Ins#
 Predošlý príkaz                                             #Ctrl+E#
 Nasledujúci príkaz                                          #Ctrl+X#
 Vymaž príkazový riadok                                      #Ctrl+Y#

 #Insertion commands#

  Vlož názov súboru z aktívneho panelu            #Ctrl+J, Ctrl+Enter#

 In the ~fast find~@FastFind@ mode, #Ctrl+Enter# does not insert a
file name, but instead cycles the files matching the
file mask entered in the fast find box.

 Insert current file name from the passive panel   #Ctrl+Shift+Enter#
 Vlož plný názov súboru z aktívneho panelu                   #Ctrl+F#
 Vlož plný názov súboru z pasívneho panelu                   #Ctrl+;#
 Insert network (UNC) file name from the active panel    #Ctrl+Alt+F#
 Insert network (UNC) file name from the passive panel   #Ctrl+Alt+;#

 Vlož cestu z ľavého panelu                                  #Ctrl+[#
 Vlož cestu z pravého panelu                                 #Ctrl+]#
 Insert network (UNC) path from the left panel           #Ctrl+Alt+[#
 Insert network (UNC) path from the right panel          #Ctrl+Alt+]#

 Vlož cestu z aktívneho panelu                         #Ctrl+Shift+[#
 Vlož cestu z pasívneho panelu                         #Ctrl+Shift+]#
 Insert network (UNC) path from the active panel        #Alt+Shift+[#
 Insert network (UNC) path from the passive panel       #Alt+Shift+]#

 Poznámky:

 1. ^<wrap>Ak je príkazový riadoch prázdny, #Ctrl+Ins# skopíruje mená
označených súborov z panelu do schránky ako #Ctrl+Shift+Ins# (see ~Panel control commands~@PanelCmd@);

 2. ^<wrap>#Ctrl+End# stlačené na konci príkazového riadku nahradí jeho obsah príkazom z
~histórie~@History@, začínajúc od už vložených znakov, ak taký príkaz existuje.
Môžete stlačiť #Ctrl+End# opäť a vložiť nasledujúci taký príkaz.

 3. ^<wrap>Väčšina z vyššie popísaných príkazov funguje všade, kde sa
zadáva text, vrátane vstupných polí v dialógových oknách a interného editora.

 4. ^<wrap>#Alt+Shift+Left#, #Alt+Shift+Right#, #Alt+Shift+Home# and #Alt+Shift+End# select
the block in the command line also when the panels are on.

 5. ^<wrap>For local drives, the commands to insert the network (UNC) name of a file object
insert the local name of the file with ~symbolic links~@HardSymLink@ expanded.


@FuncCmd
$ #Príkazy pre prácu so súbormi a servisné príkazy#
 Nápoveda                                                        #F1#

 Zobraz ~užívateľské menu~@UserMenu@                                         #F2#

 Prezeranie                             #Ctrl+Shift+F3, NumKlav5, F3#
 Ak sa stlačí na súbore, #F3# alebo #5# na numerickej klávesnici vyvolá
~interný~@Viewer@, externý alebo ~podľa typu priradený~@FileAssoc@
prezerač, v závislosti od typu súboru a ~nastavenia externých prezeračov~@ViewerSettings@.
 #Ctrl+Shift+F3# vždy vyvolá interný prezerač ignorujúc nastavenie externých prezeračov.
 Ak sa stlačí na adresári, vypočíta a zobrazí dĺžku označených adresárov.

 Editovanie                                       #Ctrl+Shift+F4, F4#
 #F4# vyvolá ~interný~@Editor@, externý alebo ~podľa typu priradený~@FileAssoc@
editor, v závislosti od typu súboru a ~nastavenia externých editorov~@EditorSettings@.
 #Ctrl+Shift+F4# vždy vyvolá interný editor ignorujúc nastavenie externých editorov.
#F4# and #Ctrl+Shift+F4# for directories invoke the change file
~attributes~@FileAttrDlg@ dialog.

 ~Kopírovanie~@CopyFiles@                                                     #F5#
 Kopíruje súbory a adresáre. Ak chcete vytvoriť cieľový adresár pred
kopírovaním, pridajte k jeho menu spätné lomítko.

 ~Premenovanie alebo presun~@CopyFiles@                                       #F6#
 Presunie alebo premenuje súbory a adresáre. Ak chcete vytvoriť cieľový
adresár pred presúvaním, pridajte k jeho menu spätné lomítko.

 ~Vytvorenie nového adresára~@MakeFolder@                                      #F7#

 ~Vymazanie~@DeleteFile@                                  #Shift+Del, Shift+F8, F8#

 ~Dôkladné vymazanie~@DeleteFile@                                         #Alt+Del#

 Zobraz ~menu~@Menus@                                                     #F9#

 Ukončiť Far                                                    #F10#

 Zobraz príkazy ~prídavných modulov (plugins)~@Plugins@ commands           #F11#

 Zmeň aktuálnu jednotku v ľavom paneli                       #Alt+F1#

 Zmeň aktuálnu jednotku v pravom panely                      #Alt+F2#

 Interný/externý prezerač                                    #Alt+F3#
 If the internal viewer is used by default, invokes the external viewer
specified in the ~Viewer settings~@ViewerSettings@ or the ~associated viewer program~@FileAssoc@
for the file type. Vyvolá interný prezerač ak je ako východzí nastavený externý a externý ak
je ako východzí nastavený interný.

 Interný/externý editor                                      #Alt+F4#
 If the internal editor is used by default, invokes the external editor
specified in the ~settings~@EditorSettings@ or the ~associated editor program~@FileAssoc@
for the file type. Vyvolá interný editor ak je ako východzí nastavený externý a externý ak
je ako východzí nastavený interný.

 Vytlačiť súbory                                             #Alt+F5#
 If the "Print Manager" plugin is installed then the printing of
the selected files will be carried out using that plugin,
otherwise by using internal facilities.

 Vytvoriť ~súboru linku~@HardSymLink@ (len NTFS)                            #Alt+F6#
 Použitím pevných liniek môžete mať niekoľko rôznych názvov súborov,
ktoré odkazujú na tie isté dáta.

 Vykonaj príkaz ~nájdenia súboru~@FindFile@                              #Alt+F7#

 Zobraz ~históriu príkazov~@History@                                    #Alt+F8#

 Nastavenie počtu riadkov obrazovky                          #Alt+F9#
 In the windowed mode, toggles between the current size and the maximum
possible size of a console window. In the fullscreen mode, #Alt+F9# toggles the
screen height between 25 and 50 lines. See ~Interface.AltF9~@Interface.AltF9@ for details.

 Configure ~plugins~@Plugins@.                                    #Alt+Shift+F9#

 Vykonaj príkaz ~nájdenia adresáru~@FindFolder@                           #Alt+F10#

 Zobraz ~históriu prezerania a editovania~@HistoryViews@                    #Alt+F11#

 Zobraz ~históriu adresárov~@HistoryFolders@                                  #Alt+F12#

 Pridanie súborov do archívu                               #Shift+F1#
 Rozbalenie súborov z archívu                              #Shift+F2#
 Vykonanie príkazov pracujúcich z archívmi                 #Shift+F3#

 Editovanie ~nového súboru~@FileOpenCreate@                                  #Shift+F4#
 When a new file is opened, the same code page is used as in the last opened
editor. If the editor is opened for the first time in the current Far session,
the default code page is used.

 Kopírovanie súboru pod kurzorom                           #Shift+F5#
 Premenovanie alebo presun súboru pod kurzorom             #Shift+F6#
 For folders: if the specified path (absolute or relative) points to an
existing folder, the source folder is moved inside that folder. Otherwise the
folder is renamed/moved to the new path.
 E.g. when moving #c:\folder1\# to #d:\folder2\#:
 - ^<wrap>if #d:\folder2\# exists, contents of #c:\folder1\# is
moved into #d:\folder2\folder1\#;
 - ^<wrap>otherwise contents of #c:\folder1\# is moved into the
newly created #d:\folder2\#.

 ~Vymazanie súboru~@DeleteFile@ pod kurzorom                             #Shift+F8#

 Uloženie konfigurácie                                     #Shift+F9#

 Návrat na naposledy spustenú položku menu                #Shift+F10#

 Spustenie, zmena adresára, vstup do archívu                  #Enter#
 Spustenie v osobitnom okne                             #Shift+Enter#
 Execute as administrator                            #Ctrl+Alt+Enter#

 Pressing #Shift+Enter# on a directory invokes the Windows Explorer and
shows the selected directory. To show a root directory in the Explorer, you
should press #Shift+Enter# on the required drive in the ~Change drive menu~@DriveDlg@.
Pressing #Shift+Enter# on "#..#" opens the current directory in the Explorer.

 Zmena adresára do koreňového                                #Ctrl+\\#

 Zmena adresára, vstup do archívu                 #Ctrl+[Shift+]PgDn#
 (aj do SFX archívu)
 If the cursor points to a directory, pressing #Ctrl+PgDn# changes to that
directory. If the cursor points to a file, then, depending on the file type,
an ~associated command~@FileAssoc@ is executed or the archive is opened.
#Ctrl+Shift+PgDn# always opens the archive, regardless of the associated
command configuration.

 Zmena adresára na rodičovský (o stupeň vyššie)           #Ctrl+PgUp#
 The behavior in root folders depends on "~Use Ctrl+PgUp to change drive~@InterfSettings@" option.

 Vytvorenie skratky k aktuálnemu adresáru            #Ctrl+Shift+0…9#

 Použitie skratky k adresáru                          #RightCtrl+0…9#

 Nastav ~atribúty súboru~@FileAttrDlg@                                      #Ctrl+A#

 ~Použi príkaz~@ApplyCmd@ na označené súbory                             #Ctrl+G#

 ~Popíš~@FileDiz@ označené súbory                                       #Ctrl+Z#


@DeleteFile
$ #Deleting and wiping files and folders#
 The following hotkeys are used to delete or wipe out files and folders:

 #F8#
 If any files or folders are selected in the panel then the selected group will be deleted, otherwise
the object currently under cursor will be deleted.

 #Shift+F8#
 Vymaže len súbor pod kurzorom (with no regard to selection in the panel).

 #Shift+Del#
 Delete selected objects okamžite a nie do Koša.

 #Alt+Del#
 Wipe out files and folders.

 Remarks:

 1. ^<wrap>In accordance to ~System settings~@SystemSettings@ the hotkeys #F8# and
#Shift+F8# do or do not move the deleted files to the Recycle Bin. The
#Shift+Del# hotkey always deletes, skipping the Recycle Bin.

 2. ^<wrap>A file is wiped by pred sú jeho dáta prepísané nulami
(a different value can be specified in
~System.WipeSymbol~@System.WipeSymbol@), následne je súbor rozdelený
a premenovaný na dočasné meno, and finally deleting.


@ErrCannotExecute
$ #Error: Cannot execute#
 The program you tries to execute is not recognized as an internal or
external command, operable program or batch file.


@Grabber
$ #Screen grabber#
 Zachytenie obrazovky                                       #Alt+Ins#

 Zachytenie obrazovky umožňuje zvoliť ľubovoľnú oblasť obrazovky a následne ju skopírovať do schránky.

 To switch between stream and block selection mode use the #Space# key.
 Pohyb kurzora možno riadiť #šípkami# alebo kliknutím #ľavého tlačítka myši#.
 Na označenie oblasti sa používajú šípky pri stlačenom klávese #Shift# alebo pohyb myši pri stlačenom #ľavom tlačítku#.
 To extend or shrink selected area use #Alt+Shift+arrow# keys.
 To move selected area use #Alt+arrow# keys.
 #Enter#, #Ctrl+Ins#, #pravé tlačítko myši# alebo #dvojité kliknutie# skopírujú
označený text do schránky, #Ctrl+<Sivý +># ho do schránky pridá k už existujúcemu obsahu.
 #Esc# ukončia zachytávanie.
 #Ctrl+A# - select whole screen.
 #Ctrl+U# - deselect block.
 #Ctrl+Shift+Left# and #Ctrl+Shift+Right# - extend or shrink selection by 10 characters left or right.
 #Ctrl+Shift+Up# and #Ctrl+Shift+Down# - extend or shrink selection by 5 lines up or down.


@MiscCmd
$ #Ostatné#
 #Keyboard macros#

 Načítaj ~klávesové makro~@KeyMacro@                                   #Ctrl+<.>#

 #Menus and dropdown lists#

 Enable/disable filtering mode                     #RAlt, Ctrl+Alt+F#
 Lock/unlock filter                                      #Ctrl+Alt+L#

 When in filter mode, you can filter the displayed items by entering
text.

 #Dialogs#

 História vo vstupných poliach okien             #Ctrl+Up, Ctrl+Down#

 In dialog edit control history you can use #Enter# to copy the current item
to the edit control and #Ins# to mark or unmark an item. Marked items are not
pushed out of history by new items, so you can mark frequently used strings so
that you will always have them in the history.

 Vymaž históriu vo vstupných poliach okien                      #Del#

 Delete the current item in a dialog edit line history    #Shift+Del#
 (if it is not locked)

 Set the dialog focus to the first element                     #Home#

 Set the dialog focus to the default element              #PgDn, End#

 The #Home# and #End# keys move the focus if it is currently not
on a control which handles these keys internally, like edit control.

 The following combinations are valid for all edit controls except the
command line, including dialogs and the ~internal editor~@Editor@.

 Vlož meno súboru pod kurzorom do dialógového okna      #Shift+Enter#

 Insert a file name from passive panel to dialog   #Ctrl+Shift+Enter#

 Pressing #Ctrl+Enter# in dialogs executes the default action (pushes the
default button or does another similar thing).

 In dialogs, when the current control is a check box:

 - turn on (#[x]#)                                             #Gray +#
 - turn off (#[ ]#)                                            #Gray -#
 - change to undefined (#[?]#)                                 #Gray *#
   (for three-state checkboxes)

 You can move a dialog (window) by dragging it with mouse or by pressing #Ctrl+F5# and using #arrow# keys.

 #Left clicking# outside the dialog works the same as pressing #Esc#.

 #Right clicking# outside the dialog works the same as pressing #Enter#.

 #Mouse#

 Clicking the #middle mouse button# in the ~panels~@PanelCmd@ has the same
effect as pressing the #Enter# key with the same modifiers (#Ctrl#, #Alt#,
#Shift#). If the ~command line~@CmdLineCmd@ is not empty, its contents will be executed.

 Far Manager also supports the ~mouse wheel~@MsWheel@.


@SpecCmd
$ #Special commands#
 ~Version information~@FarAbout@
 ~Configuration editor~@FarConfig@


@MsWheel
$ #Mouse: wheel support#
 #Panels#
 Rotating the wheel scrolls the file list without changing the cursor position on the screen.
Pressing the #middle button# has the same effect as pressing #Enter#.

 #Editor#
 Rotating the wheel scrolls the text without changing the cursor position on the screen
(similar to #Ctrl+Up#/#Ctrl+Down#).

 #Viewer#
 Rotating the wheel scrolls the text.

 #Help#
 Rotating the wheel scrolls the text.

 #Menus#
 Wheel scrolling works as #Up#/#Down# keys. Pressing the #middle button# has the same effect as
pressing #Enter#. It is possible to choose items without moving the cursor.

 #Dialogs#
 In dialogs, when the wheel is rotated at an edit line with a history list or a combo box,
the drop-down list is opened. In the drop-down list scrolling works the same as in menus.

 You can specify the number of lines to scroll at a time in the panels,
editor and viewer (see ~System.MsWheelDelta~@System.MsWheelDelta@).


@Plugins
$ #Podpora prídavných modulov (plugins)#
 Externé DLL moduly (plugins) môžu byť použité na pridanie nových
príkazov pre Far a emulovanie súborových systémov. Napríklad podpora
archívov, klient FTP, dočasný panel a prezerač siete emulujú súborový
systém.

 Všetky prídavné moduly sú uložené v osobitných adresároch v adresári 'Plugins',
ktorý je umiestnený v tom istom adresári ako Far.exe, and the 'Plugins' folder, which is in the
user profile folder (#%APPDATA%\\Far Manager\\Profile# by default).
Keď Far detekuje nový modul, uloží si informácie o ňom a
neskôr ho načíta len keď je potrebný, takže nepoužívané moduly
nevyžadujú nijakú dodatočnú pamäť. Ale ak ste si istý, že niektoré
moduly sú pre vás zbytočné, môžete ich vymazať a ušetriť miesto na disku.

 Moduly môžu byť vyvolané buď cez ~menu zmeny jednotky~@DriveDlg@
alebo z menu ~Príkazy prídavných modulov~@PluginCommands@, aktivovanom stlačením #F11# alebo príslušnou položkou
z menu ~Príkazy~@CmdMenu@. #F4# v tomto menu dovoľuje priradiť klávesovú skratku jednotlivým položkám
(toto umožňuje ich potom ľahšie vyvolať pomocou ~klávesových makier~@KeyMacro@).
Toto menu je prístupné zo súborových panelov a (len cez #F11#) z interného
prezerača a editora. Keď sa zavolá z prezerača alebo editora, zobrazia
sa len špeciálne preň vytvorené moduly.

 Parametre modulov môžete nastaviť pomocou ~Nastavenie prídavných modulov~@PluginsConfig@
z menu ~Nastavenia~@OptMenu@.

 Súbory, s ktorými sa pracuje operáciami ako kopírovanie, presúvanie,
vymazávanie, editovanie alebo ~vyhľadanie súboru~@FindFile@
môžu pracovať s modulmi, ktoré emulujú súborové systémy v prípade, že
tieto moduly poskytujú príslušné funkcie. Vyhľadávanie z aktuálneho
adresára vo "Vyhľadanie súboru" vyžaduje menšiu funkčnosť ako vyhľadávanie
z kmeňového adresára, takže skúste použiť to, ak vyhľadávanie z kmeňového
nefunguje správne.

 Moduly majú vlastné správy a súbory nápovedy. Zoznam použiteľných
nápovied pre moduly získate stlačením

 #Shift+F2# - v hlavnej nápovede Faru

 #Shift+F1# - in the list of plugins (context-dependent help).

 If the plugin has no help file, then context-dependent help will not pop out.

 Ak aktívny panel zobrazuje emulovaný súborový systém modulu, možno
v príkazovom riadku na zmenu súborového systému modulu použiť príkaz "CD".
Narozdiel od "CD" príkaz "CHDIR" vždy berie zadaný paramater ako skutočné
meno súboru bez ohľadu na typ súborového systému v paneli.

 Use #Alt+Shift+F9# to ~configure plugins~@PluginsConfig@.


@PluginCommands
$ #Plugin commands#
 This menu provides user with ability to use plugins functionality (other
ways are listed in ~"Plugins support"~@Plugins@).
The contents of the menu and actions triggered on menu items selection are
controlled by plugins.

 The menu can be invoked in the following ways:

 - ^<wrap>#F11# at file panels or #Plugins# item at ~commands menu~@CmdMenu@, herewith
the commands intended for use from file panels are shown;
 - ^<wrap>#F11# in viewer or editor, herewith the commands intended for use from
viewer and editor accordingly are shown.

 Each item of plugin commands menu can be assigned a hotkey with #F4#, this
possibility is widely used in ~key macros~@KeyMacro@. The assigned hotkey is
displayed left to the item. The #A# character in the leftmost menu column means that
the corresponding plugin is written for Far 1.7x and it does not support all
possibilities available in Far 3 (these are, in particular, Unicode characters
in filenames and in editor).

 #Plugin commands# menu hotkeys:

 #Shift+F1#
 Help on use for selected menu item. The text of the help
is taken from HLF file, associated with the plugin that owns the menu item.

 #F4#
 Assign a hotkey for selected menu item. If #Space# is
entered, then Far sets the hotkey automatically.

 #F3#
 Show plugin technical information.

 #Shift+F9#
 Settings of the selected plugin.

 #Alt+Shift+F9#
 Open ~"Plugins configuration"~@PluginsConfig@ menu.

 See also:

 ~Plugins support~@Plugins@.
 Common ~menu~@MenuCmd@ keyboard commands.


@PluginsConfig
$ #Plugins configuration#
 You can configure the installed ~plugins~@Plugins@ using the command
#"Plugins configuration"# from the ~Options menu~@OptMenu@ or by pressing
#Alt+Shift+F9# in the ~Change drive menu~@DriveDlg@ or plugins menu.

 To get the help on the currently selected plugin, press #Shift+F1# -
context-sensitive help on plugin configuration. If the plugin doesn't have a
help file, the context-sensitive help will not be shown.

 When the context-sensitive help is invoked, Far will try to show the topic
#Config#. If such a topic does not exist in the plugin help file, the main help
topic for the selected plugin will be shown.

 Each item of plugins configuration menu can be assigned a hotkey with #F4#,
this possibility is widely used in ~key macros~@KeyMacro@. The assigned hotkey is
displayed left to the item. The #A# character in the leftmost menu column means that
the corresponding plugin is written for Far 1.7x and it does not support all
possibilities available in Far 3 (these are, in particular, Unicode characters
in filenames and in editor).

 Pressing #F3# will show some technical information about the plugin.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@PluginsReviews
$ #Overview of plugin capabilities#
 Far Manager is so tightly integrated with its plugins that it is simply
meaningless to talk about Far and not to mention the plugins. Plugins present
an almost limitless expansion of the features of Far.

 Without going into details, some of the capabilities can be noted:

 * ^<wrap>Syntax highlighting in program source texts.
 * Working with FTP-servers (including access through proxy).
 * Search and replace in many files at the same time, using regular expressions.
 * Renaming groups of files with support for complex compound masks consisting of substitution symbols and templates.
 * NNTP/SMTP/POP3/IMAP4 clients and sending messages to a pager.
 * Working with non-standard text screen resolutions.
 * Conversion of texts from one national code page to another.
 * Manipulating the contents of the Recycle Bin.
 * Process priority control on local or network PC.
 * Words autocomplete in editor and working with templates.
 * Windows system registry editing.
 * Creating and modifying Windows shortcuts.
 * File and text operations making it more comfortable to use FidoNet.
 * Files UU-encode and UU-decode.
 * Winamp control and MP3-tags modifying.
 * Quake PAK-files processing.
 * Printers control, both connected to PC and network.
 * Connection and debugging of queries to ODBC-compatible databases.
 * RAS service control.
 * External programs executing (compilers, converters etc.) while editing text in Far editor.
 * Windows help files contents displaying (.hlp and .chm)
 * Calculators with different possibilities.
 * Several games :-)
 * Spell checker functions while editing text in Far editor.
 * Removable drives catalog preparation and much more…

 As an information source, which can be used to search for specific plugins,
one can recommend:

 - Far Manager official site
   ~https://www.farmanager.com~@https://www.farmanager.com@
 - Online forum
   ~https://forum.farmanager.com~@https://forum.farmanager.com@
 - Registration and handling of problems
   ~https://bugs.farmanager.com~@https://bugs.farmanager.com@
 - PlugRinG site
   ~https://plugring.farmanager.com~@https://plugring.farmanager.com@
 - Free email group service
   ~https://groups.google.com/group/fardeven/~@https://groups.google.com/group/fardeven@
 - USENET echo conference
   ~news:fido7.far.support~@news:fido7.far.support@
   ~news:fido7.far.development~@news:fido7.far.development@
 - FidoNet echo conference
   far.support
   far.development


@Panels
$ #Panely#
 Za normálnych okolností Far zobrazuje dva panely (ľavé a pravé okno),
s rôznymi informáciami. Ak chcete zmeniť typ informácie zobrazenej v paneli,
použite ~menu panelu~@LeftRightMenu@ alebo zodpovedajúci ~príkaz z klávesnice~@KeyRef@.

 Pre viac informácií sa si pozrite nasledovné položky:

 ~Súborový panel~@FilePanel@
 ~Stromový panel~@TreePanel@
 ~Informačný panel~@InfoPanel@
 ~Panel rýchleho prezerania~@QViewPanel@

 ~Ťahaj a pusť súbory~@DragAndDrop@
 ~Označovanie súborov~@SelectFiles@
 ~Prispôsobenie zobrazovacích módov v súborovom paneli~@PanelViewModes@


@FilePanel
$ #Súborový panel#
 Súborový panel zobrazuje aktuálny adresár. Môžete označiť alebo odznačiť
súbory a adresáre, vykonávať rôzne operácie so súbormi a archívmi. Pre zoznam
súborov si prečítajte ~zoznam klávesových skratiek~@KeyRef@.

 Základné zobrazovacie módy v súborovom paneli sú:

 #Stručne#       Mená súborov sú zobrazené v troch stĺpcoch.

 #Stredne#       Mená súborov sú zobrazené v dvoch stĺpcoch.

 #Úplne#         Zobrazuje meno, dĺžku, dátum a čas každého súboru.

 #Široko#        Zobrazuje mená a dĺžky súborov.

 #Detailne#      ^<wrap>Mená súborov, dĺžky, komprimované dĺžky, dátum a
čas vytvorenia, poslednej zmeny a posledného prístupu a atribúty. V celoobrazovkovom móde.

 #Popisy#        Mená súborov a ~ich popisy (descriptions)~@FileDiz@

 #Dlhé popisy#   ^<wrap>Mená súborov, ich dĺžky a popisy. V celoobrazovkovom móde.

 #Vlast. súborov# Mená súborov, ich dĺžky a vlastníci.

 #Linky súborov# Mená a dĺžky súborov a počet pevných liniek.

 #Alter. úplne#  ^<wrap>Zobrazí mená súborov, dĺžky (formátované s čiarkami) a ich dátumy.

 ~Zobrazovacie módy v súborovom panely si môžete prispôsobiť.~@PanelViewModes@

 Vlastníci súborov a počet pevných liniek majú taktiež význam len pre NTFS.
Niektoré systémy nemusia podporovať ani dátumy vytvorenia a posledného prístupu.

 Ak chcete zmeniť mód zobrazenia súborov v paneli, vyberte si z ~menu panelu~@LeftRightMenu@.
Po zmene módu alebo zmene jednotky sa panel automaticky nastaví na zobrazenie súborov,
ak bol predtým nastavený iný typ panelu.

 ~Rýchle vyhľadávanie~@FastFind@ možno použiť na nastavenie sa na
požadovaný súbor použitím prvých písmen jeho mena.

 See also the list of ~macro keys~@KeyMacroShellList@, available in the panels.


@TreePanel
$ #Stromový panel#
 Stromový panel zobrazuje štruktúru aktuálneho disku ako strom.
V tomto strome môžete veľmi rýchlo meniť adresár a vykonávať operácie
z adresármi.

 Far ukladá informácie o strome adresárov do súboru z názvom #Tree.Far# do kmeňového adresáru každej jednotky.
Pre jednotky prístupné iba na čítanie sa táto informácia ukladá do skrytého adresáru Tree.Cache
umiestneného v tom istom adresári ako Far.exe. The tree3.far
file doesn't exist by default. It will be automatically created after the first
use of the #Tree Panel# or the #Find Folder# command. If that file exists, Far
updates it with the changes to the tree structure it is aware of. If such
changes were made outside of Far and Tree.far is no longer current, it can be
refreshed by pressing #Ctrl+R#.

 Tak ako u súborov možno aj tu použiť funkciu #rýchleho vyhľadávania#. Držte
kláves Alt a píšte meno adresára kým sa kurzor nenastaví na požadované. Po stlačení
#Ctrl+Enter# (súčasne) preskočí na nasledujúci adresár začínajúci na zadané znaky, ak taký existuje.

 #Gray +# and #Gray -# keys move up and down the tree to the next branch
on the same level.

 See also the list of ~macro keys~@KeyMacroTreeList@, available in the folder tree panel.


@InfoPanel
$ #Informačný panel#
 Informačný panel obsahuje nasledovné údaje:

 1. ^<wrap>#sieťové# mená počítača a užívateľa (see ~Info panel settings~@InfoPanelSettings@);

 2. ^<wrap>meno a typ #aktuálneho disku# ako i typ súborového systému, sieťové
meno, celkové a voľné miesto, názov jednotky disku a sériové číslo;

    ^<wrap>Far will attempt to determine the type of each of the CD drives available
in the system. Known types are as follows: CD-ROM, CD-RW, CD-RW/DVD, DVD-ROM,
DVD-RW and DVD-RAM. This function is available only for users either with
administrative privileges or all local users, when it's stated explicitly in
the Local Policy Editor (to do this, run #secpol.msc# from the command
prompt, and set the '#Local Policies/Security Options/Devices: Restrict#
#CD-ROM access to locally logged-on user only#' setting to '#Enabled#')

    ^<wrap>For virtual devices (SUBST-disk) the parameters of the primary disk are shown.

    ^<wrap>#Ctrl+Shift+S# toggles size display mode: float with size suffixes or bytes.
Memory size display mode also changes. ~Quick view panel~@QViewPanel@ and ~file panel~@FilePanel@
status line also affected.
Current mode - far:config #Panel.ShowBytes# (default=false).

 3. ^<wrap>Memory information
    ^<wrap>percento obsadenia #pamäte# (100% znamená, že je obsadená celá pamäť),
size of the installed physical memory (in Vista and newer), cekovú a voľnú veľkosť fyzickej
pamäte (available for Windows) a virtuálnej pamäte and paging file;

 4. ^<wrap>súbor #popisu adresára#
    ^<wrap>Obsah celého súboru si môžete prezrieť na celej obrazovké stlačením
klávesu #F3# alebo ľavého tlačítka myši. Editovanie alebo vytvorenie je
k dipozícii po stlačení klávesu #F4# alebo pravého tlačítka myši. You can also use
many of the ~viewer commands~@Viewer@ (search, code page selection and so on)
for viewing the folder description file.
    ^<wrap>Zoznam možných súborov s popismi adresára môže byť definovaný
pomocou príkazu "Popisný súbor adresára" v menu ~Nastavenia (Options)~@OptMenu@.

 5. Plugin panel
    ^<wrap>Contains information about the opposite plugin panel, if provided by the plugin.

 6. Power status.
    - ^<wrap>AC power status (offline, online, unknown);
    - ^<wrap>Battery percentage;
    - ^<wrap>Charge status (High (more than 66%), low (less than 33%), critical, charging, no battery or unknown);
    - ^<wrap>Battery life time;
    - ^<wrap>Battery full time (the system is only capable of estimating this time based on calculations on
battery life time and battery life percent. Without smart battery subsystems, this value may not be accurate enough to be useful).
    ^<wrap>AC power status is updated automatically.
    ^<wrap>In Windows Vista and above charge status is update automatically.
    ^<wrap>Power status section can be turned on and off in ~settings~@InfoPanelSettings@.

 All sections (except computer and user names) can be hidden or shown (see ~InfoPanel display modes~@InfoPanelShowMode@).

 Also see the list of ~macro keys~@KeyMacroInfoList@, available in the info panel.


@InfoPanelShowMode
$ #InfoPanel display modes#
 ~InfoPanel~@InfoPanel@ display modes menu can be called with #Ctrl+F12# and affects currently
active informantion panel. Available display modes:

 - Drive information
 - Memory information
 - Description file content
 - Plugin panel information
 - Power status information

 #+# key shows the selected section.
 #-# key hides the selected section.
 #*# key flips the visibility of the selected section.

 Power status information can be enabled in InfoPanel ~settings~@InfoPanelSettings@.


@InfoPanelSettings
$ #InfoPanel settings#
 #Show power status#
 If enabled, ~InfoPanel~@InfoPanel@ will contain a section with power status details.

 #Show CD drive parameters#
 If enabled, Far will try to detect the CD drive type.
Supported types: CD-ROM, CD-RW, CD-RW/DVD, DVD-ROM, DVD-RW and DVD-RAM.

 #Computer name format#
 Can be one of:

 - #NetBIOS#
   ^<wrap>The NetBIOS name of the local computer. If the local computer is a node in a cluster, the NetBIOS name of the cluster virtual server.

 - #DNS hostname#
   ^<wrap>The DNS host name of the local computer. If the local computer is a node in a cluster, the DNS host name of the cluster virtual server.

 - #DNS domain#
   ^<wrap>The name of the DNS domain assigned to the local computer. If the local computer is a node in a cluster, the DNS domain name of the cluster virtual server.

 - #DNS fully-qualified#
   ^<wrap>The fully qualified DNS name that uniquely identifies the local computer. This name is a combination of the DNS host name and the DNS domain name, using the form HostName.DomainName. If the local computer is a node in a cluster, the fully qualified DNS name of the cluster virtual server.

 - #Physical NetBIOS#
   ^<wrap>The NetBIOS name of the local computer. If the local computer is a node in a cluster, the NetBIOS name of the local computer, not the name of the cluster virtual server.

 - #Physical DNS hostname#
   ^<wrap>The DNS host name of the local computer. If the local computer is a node in a cluster, the DNS host name of the local computer, not the name of the cluster virtual server.

 - #Physical DNS domain#
   ^<wrap>The name of the DNS domain assigned to the local computer. If the local computer is a node in a cluster, the DNS domain name of the local computer, not the name of the cluster virtual server.

 - #Physical DNS fully-qualified#
   ^<wrap>The fully qualified DNS name that uniquely identifies the computer. If the local computer is a node in a cluster, the fully qualified DNS name of the local computer, not the name of the cluster virtual server. The fully qualified DNS name is a combination of the DNS host name and the DNS domain name, using the form HostName.DomainName.

 The output format depends on the domain structure, group policies and DNS settings.

 #User name format#
 Can be one of:

 - #Logon Name#
   User login, (for example, JeffSmith)

 - #Fully Qualified Domain Name#
   ^<wrap>The fully qualified distinguished name (for example, CN=Jeff Smith,OU=Users,DC=Engineering,DC=Microsoft,DC=Com).

 - #Sam Compatible#
   ^<wrap>A legacy account name (for example, Engineering\JSmith). The domain-only version includes trailing backslashes (\\).

 - #Display Name#
   ^<wrap>A "friendly" display name (for example, Jeff Smith). The display name is not necessarily the defining relative distinguished name (RDN).

 - #Unique Id#
   ^<wrap>An UUID string (for example, {4fa050f0-f561-11cf-bdd9-00aa003a77b6}).

 - #Canonical Name#
   ^<wrap>The complete canonical name (for example, engineering.microsoft.com/software/someone). The domain-only version includes a trailing forward slash (/).

 - #User Principial Name#
   ^<wrap>The user principal name (for example, someone@@example.com).

 - #Service Principal#
   ^<wrap>The generalized service principal name (for example, www/www.microsoft.com@@microsoft.com).

 - #DNS Domain#
   ^<wrap>The DNS domain name followed by a backward-slash and the SAM user name.

   #Given Name#

   #Surname#

 The ouput format depends on the domain structure.


@QViewPanel
$ #Panel rýchleho zobrazenia#
 Panel rýchleho zobrazenia sa používa na zobrazenie informácie o
označenej položke v ~súborovom paneli~@FilePanel@ alebo ~stromovom paneli~@TreePanel@

 Ak je označená položka súbor, potom sa zobrazí jeho obsah.
Many of the ~internal viewer~@Viewer@ commands can be used with the file
displayed in the panel. Pre súbory typov registrovaných vo Windows sa zobrazí
aj tento typ.

 Pre adresáre vypíše panel rýchleho zobrazenia celkovú dĺžku, celkovú komprimovanú
dĺžku, počet súborov a podadresárov v adresári, veľkosť clustera aktuálneho disku,
reálnu dĺžku súborov, vrátane "straty" (súčet nepoužitých častí clusterov).

 When viewing ~reparse points~@HardSymLink@, the path to the source folder is also displayed.

 For folders, the total size value may not match the actual value:
 - ^<wrap>If the folder or its subfolders contain symbolic links and the option
"Scan symbolic links" in the ~System settings~@SystemSettings@ dialog is enabled.
 - ^<wrap>If the folder or its subfolders contain multiple hard links to the same file.

 #Ctrl+Shift+S# toggles size display mode: float with size suffixes or bytes.
~Info panel~@InfoPanel@ and ~file panel~@FilePanel@ status line also affected.
Current mode - far:config #Panel.ShowBytes# (default=false).

 See also the list of ~macro keys~@KeyMacroQViewList@, available in the quick view panel.


@DragAndDrop
$ #Ťahaj a pusť súbory#
 Operácie #kopírovania# a #presúvania# je možné vykonať aj pomocou #Ťahaj a pusť#.
Stačí stlačiť ľavé tlačítko myši na zdrojovom súbore alebo adresári,
pretiahnuť ho do druhého panelu a pustiť tlačítko myši.

 Ak chcete túto operáciu vykonať pre skupinu súborov alebo adresárov,
najprv ich označte, stlačte ľavé tlačítko na zdrojovom paneli a pretiahnite
skupinu súborov do druhého panelu.

 Prepínanie medzi operáciou kopírovania a presúvania sa vykonáva stlačením
pravého tlačítka myši počas ťahania. Presunúť možno súbory taktiež držaním
klávesu #Shift# počas držania ľavého tlačítka myši.


@Menus
$ #Menu#
 Na vybratie nejakej akcie z menu stlačte #F9# alebo kliknite myšou
na vrchu obrazovky.

 When the menu is activated by pressing #F9#, the menu for the active panel
is selected automatically. When the menu is active, pressing #Tab# switches
between the menus for the left and right panel. If the menus "Files",
"Commands" or "Options" are active, pressing #Tab# switches to the menu of the
passive panel.

 Kombinácia Shift+F10 vás nastaví na posledný použitý príkaz menu.

 Pre informácie o jednotlivých menu si prečítajte nasledovné položky:

 ~Menu Ľavý a Pravý~@LeftRightMenu@
 ~Menu Súbory~@FilesMenu@
 ~Menu Príkazy~@CmdMenu@
 ~Menu Nastavenia~@OptMenu@

 See also the list of ~macro keys~@KeyMacroMainMenuList@, available in the main menu.


@LeftRightMenu
$ #Menu Ľavý a Pravý#
 Menu #Ľavý# a #Pravý# vám dovoľujú zmeniť nastavenia
ľavého, resp. pravého panelu. Tieto menu majú nasledovné položky:

 #Stručne#         Mená súborov sú zobrazené v troch stĺpcoch.

 #Stredne#         Mená súborov sú zobrazené v dvoch stĺpcoch.

 #Úplne#           Zobrazuje meno, dĺžku, dátum a čas každého súboru.

 #Široko#          Zobrazuje mená a dĺžky súborov.

 #Detailne#        ^<wrap>Mená súborov, dĺžky, komprimované dĺžky, dátum a čas vytvorenia,
poslednej zmeny a posledného prístupu a atribúty. V celoobrazovkovom móde.

 #Popisy#          Mená súborov a ~ich popisy~@FileDiz@

 #Dlhé popisy#     ^<wrap>Mená súborov, ich dĺžky a popisy. V celoobrazovkovom móde.

 #Vlast. súborov#  Mená súborov, ich dĺžky a vlastníci.

 #Linky súborov#   Mená a dĺžky súborov a počet pevných liniek.

 #Alter. úplne#    ^<wrap>Zobrazí mená súborov, dĺžky a ich dátumy.

 #Informačný p./#  Zmení panel na ~informačný panel~@InfoPanel@.

 #Strom adresárov# Zmení panel na ~stromový panel~@TreePanel@.

 #Rýchle zobraz.#  Zmení panel na ~rýchle zobrazenie~@QViewPanel@.

 #Zoradiť podľa#   Zobrazí použiteľné spôsoby zoradenia.

 #Zobraz dlhé m.#  Prepína zobrazenie dlhých/krátkych názvov súborov.

 #Panel Zap/Vyp#   Zobrazí/skryje panel.

 #Aktualizuj ob.#  Znovu načíta adresár/iný obsah panelu.

 #Zmeň jednotku/#  Zmení aktuálnu jednotku.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@FilesMenu
$ #Menu Súbory#
 #Zobraziť#           ~Zobrazí súbory~@Viewer@, vypočíta dĺžku adresárov.

 #Editovať#           ~Editáciu~@Editor@ súborov.

 #Skopírovať#         ~Skopíruje~@CopyFiles@ súbory a adresáre.

 #Premenovať/presunúť# ~Premenuje alebo presunie~@CopyFiles@ súbory a adresáre.

 #Link#               Create ~file links~@HardSymLink@.

 #Vytvor adresár#     ~Vytvorí~@MakeFolder@ nový adresár.

 #Odstrániť#          Odstráni súbory a adresáre.

 #Wipe#               ^<wrap>Wipe files and folders. Before file deletion
its data are overwritten with zeros, after which the file is truncated and renamed to
a temporary name.

 #Pridať do archívu#  Pridá označené súbory do archívu.

 #Vybrať z archívu#   Vyberie (rozpakuje) označené súbory z archívu.

 #Príkazy archívu/#   Zobrazí ponuku príkazov na prácu z archívom.

 #Atribúry súborov/#  ~Zmení atribúty súborov~@FileAttrDlg@ a čas.

 #Vykonať príkaz/#    Na označené súbory ~použije zvolený príkaz~@ApplyCmd@.

 #Popis súborov/#     Pridá označeným súborom ~popis~@FileDiz@.

 #Označiť skupinu/#   ~Označí~@SelectFiles@ skupinu súborov podľa zadanej masky.

 #Odznačiť skupinu/#  ~Odznačí~@SelectFiles@ skupinu súborov podľa masky.

 #Inverzné označenie# ~Invertuje~@SelectFiles@ aktuálne označenie súborov.

 #Obnoviť označenie#  ^<wrap>~Obnoví~@SelectFiles@ predošlé označenie súborov
po nejakej operácii so súbormi alebo označením skupiny.

 Niektoré príkazy tohoto menu sú popísané taktiež v položke
~Príkazy pre prácu so súbormi a servisné príkazy~@FuncCmd@.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@CmdMenu
$ #Commands menu#
 #Nájdi súbor#          Prehľadá strom adresárov, pre určenie
                      hľadaného súboru možno použiť aj masky.
                      Viď ~Nájdí súbor~@FindFile@.

 #História#             Zobrazí predtým použité príkazy.
                      Viď ~História~@History@.

 #Video mód#            Prepína medzi 25 a 50-riadkovou obrazovkou.

 #Nájdi adresár#        Prehľadá strom adresárov.
                      Viď ~Nájdi adresár~@FindFolder@.

 #Hist.zobr.súborov#    Zobrazí ~históriu prezeraných a editovaných súborov~@HistoryViews@.

 #História adresárov# Zobrazí ~históriu aktuálnych adresárov~@HistoryFolders@.

                      Items in "Folders history" and "File view
                      history" are moved to the end of list after
                      selection. Use #Shift+Enter# to select item
                      without changing its position.

 #Vymeň panely#         Vymení ľavý a pravý panel.

 #Panely Zap/Vyp#       Zobrazí/skryje oba panely.

 #Porovnaj adresáre#    Porovná obsahy adresárov.
                      Viď ~Porovnaj adresáre~@CompFolders@.

 #Úprava užívateľského# Umožní úpravu hlavného alebo lokálneho
 #menu#               ~užívateľského menu~@UserMenu@. Môžete použiť #Ins#
                      pre vloženie, #Del# pre vymazanie a #F4# pre úpravu položiek menu.

 #Typy súborov#         Zobrazí ~typy súborov~@FileAssoc@.
                    Môžete použiť #Ins# pre vloženie, #Del# pre
                    vymazanie a #F4# pre úpravu typu súborov.

 #Skratky k adresárom# Zobrazí ~skratky~@FolderShortcuts@.

 #Nastavenie skupín#    Umožní užívateľovi zmeniť nastavenie
 #triedenia/#         ~skupín triedenia~@SortGroups@.

 #Filter zobr.súborov#  Umožní nastavenie obsahu súborového
                      panelu. Viď ~Filter zobrazených súborov~@FiltersMenu@.

 #Plugin commands#      Show ~plugin commands~@Plugins@ list.

 #Zoznam obrazoviek#    Zobrazí ~zoznam obrazoviek~@ScrSwitch@.

 #Zoznam úloh#          Zobrazí ~zoznam aktívnych úloh~@TaskList@.

 #Hotplug devices list# Show ~hotplug devices list~@HotPlugList@.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@OptMenu
$ #Menu Nastavenia#
 #Systémové nastavenia#  Zobrazí ~nastavenia systému~@SystemSettings@.

 #Nastavenie panelov#    Zobrazí ~nastavenie panelov~@PanelSettings@.

 #Tree settings#         Shows ~Tree settings~@TreeSettings@ dialog.
                       Available only if ~Panel.Tree.TurnOffCompletely~@Panel.Tree.TurnOffCompletely@
                       parameter in ~far:config~@FarConfig@ is set to “false.”

 #Nastavenie prostredia# Zobrazí ~nastavenie prostredia~@InterfSettings@.

 #Voľba jazyka#          Nastaví jazyk prostredia a nápovedy.
                       Toto nastavenie možno uložiť pomocou "Uložiť nastavenie".
                       You can ~customize UI elements~@CustomizingUI@ to you needs and taste.

 #Nast. prídavných#      Umožní konfiguráciu ~prídavných modulov~@PluginsConfig@.
 #modulov#

 #Plugin manager#        Shows ~Plugin manager settings~@PluginsManagerSettings@ dialog.
 #settings#

 #Dialog settings#       Shows ~Dialog settings~@DialogSettings@ dialog.

 #Menu settings#         Shows ~Menu settings~@VMenuSettings@ dialog.

 #Command line settings# Shows ~Command line settings~@CmdlineSettings@ dialog.

 #AutoComplete settings# Shows ~AutoComplete settings~@AutoCompleteSettings@ dialog.

 #InfoPanel settings#    Shows ~InfoPanel settings~@InfoPanelSettings@ dialog.

 #Groups of file masks#  Shows ~Groups of file masks~@MaskGroupsSettings@ dialog.

 #Potvrdzovanie#         Umožní zapnutie/vypnutie ~potvrdzovania~@ConfirmDlg@
                       pre niektoré operácie.

 #Módy zobraz. súborov#  Umožní ~Prispôsobenie zobrazovacích módov~@PanelViewModes@.

 #Popisy súborov#        Názvy zoznamov s ~popismi súborov~@FileDiz@ a spôsob ich aktualizácie.

 #Súbory s popismi#      Shows ~Folder description files~@FolderDiz@ dialog.
 #adresárov#

 #Nastavenie prezerača#  Nastevenie externého ~prezerača~@ViewerSettings@.

 #Nastavenie editora#    Nastavenie interného a externého ~editora~@EditorSettings@.

 #Code pages#            Shows the ~Code pages~@CodePagesMenu@ menu.

 #Farby#                 Shows the ~Color groups~@ColorGroups@ menu.

 #Zvýraznenie súborov#   Zmení nastavenie ~zvýrazňovania súborov~@Highlight@.

   #Uložiť nastavenie#   Uloží aktuálnu konfiguráciu, farby a stav
                       obrazovky.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@ConfirmDlg
$ #Potvrdzovanie (Confirmations)#
 V tomto okne môžete zapnúť alebo vypnúť potvrdzovanie nasledovných
operácií:

 - ^<wrap>prepísanie cieľových súborov pri kopírovaní;
 - prepísanie cieľových súborov pri presúvaní;
 - overwrite and delete files with "read only" atrtibute;
 - použitie techniky ~ťahaj a pusť~@DragAndDrop@;
 - vymazanie súborov;
 - vymazanie adresárov;
 - interrupt operation;
 - ~disconnect network drives~@DisconnectDrive@ from the Disks menu;
 - delete SUBST-disks from the Disks menu;
 - detach virtual disks from the Disks menu;
 - removal of USB storage devices from the Disks menu;
 - ~reloading~@EditorReload@ a file in the editor;
 - clearing the view/edit, folder and command history lists;
 - ukončenie Faru.


@PluginsManagerSettings
$ #Plugins manager settings#
 #OEM plugins support#
 Controls support for old Far Manager 1.x non-unicode plugins.

 #Scan symbolic links#
 Look for plugins in ~symbolic links~@HardSymLink@ as well as in normal directories.

 #File processing#

 #Show standard association item#

 #Even if only one plugin found#

 #Search results (SetFindList)#

 #Prefix processing#


@MaskGroupsSettings
$ #Groups of file masks#
 An arbirtary number of ~file masks~@FileMasks@ can be joined into a named group.

 Hereinafter the group name, enclosed in angle brackets (i.e. #<#name#>#), can be used wherever masks can be used.

 Groups can contain other groups.

 For example, the #<arc># group contains the "*.rar,*.zip,*.[zj],*.[bg7]z,*.[bg]zip,*.tar" masks.
To ~highlight~@Highlight@ all archives except "*.rar" #<arc>|*.rar# should be used.

 Control keys:

 #Ctrl+R#      - ^<wrap>restore the default predefined groups ("arc", "temp" and "exec").

 #Ins#         - ^<wrap>add a new group

 #Del#         - ^<wrap>remove the current group

 #Enter#/#F4#  - ^<wrap>edit the current group

 #F7#          - ^<wrap>find all groups containing the specified mask

 Also see ~Options menu~@OptMenu@ and ~Menu control commands~@MenuCmd@.


@ChoosePluginMenu
$ #Plugin selection menu#
 Allows to choose the processing plugin if the host file (e.g. an archive)
can be processed by multiple plugins.

 Also see common ~menu~@MenuCmd@ keyboard commands.


@MakeFolder
$ #Make folder#
 This function allows you to create folders. You can use environment
variables in the input line, which are expanded to their values before creating
the folder. Also you can create multiple nested subfolders at the same time:
simply separate the folder names with the backslash character. For example:

 #%USERDOMAIN%\\%USERNAME%\\Folder3#

 The "#Link Type#" option allows to choose the link ~type~@HardSymLink@
("#directory junction#" or "#symbolic link#"). If selected, the #Target# field
should contain a target directory path.

 If the option "#Process multiple names#" is enabled, it is possible to
create multiple folders in a single operation. In this case, folder names
should be separated with the character "#;#" or "#,#". If the option is enabled
and the name of the folder contains a character "#;#" (or "#,#"), it must be
enclosed in quotes. For example, if the user enters
#C:\\Foo1;"E:\\foo,2;";D:\\foo3#, folders called "#C:\\Foo1#", "#E:\\foo,2;#"
and "#D:\\foo3#" will be created.


@FindFile
$ #Nájdi súbor#
 Tento príkaz je určený na vyhľadanie jedného alebo viacerých súborov a adresárov v
adresárovom strome, podľa jednej alebo viacerých masiek (~wildcards~@FileMasks@), oddelených čiarkami.
Taktiež môže byť použitá pri súborových systémoch emulovaných ~prídavnými modulmi~@Plugins@.

 Zadaním voliteľného textového reťazca sa vyhľadajú iba súbory obsahujúce tento text.
Ak sa tento reťazec zadadá, možno nastavením "Rozlišovať veľké/malé písmená"
prikázať presné porovnávanie veľkých a malých písmen.

 The option #Whole words# will let to find only the text that is separated
from other text with spaces, tab characters, line breaks or standard
separators, which by default are: #!%^&*()+|{}:"<>?`-=\\[];',./#.

 #Fuzzy search# is diacritical insensitive, treats ligatures equivalent
to their corresponding multicharacter sequences and fancy numerals
to corresponding number characters, and ignores some other minor
differences.

 By selecting #Hex# option you can search for the files containing hexadecimal
sequence of the specified bytes. In this case, #Case sensitive#, #Whole words#,
#Fuzzy search#, #Using code page# and #Search for folders# options are disabled
and their values do not affect the search process.

 #Not containing# allows to find files #not# containing the specified text or code.

 Dropdown list #Using code page# allows to choose a specific code page
to be used for the search. If the item #All standard code pages# is selected
in the dropdown list, Far will use all standard code pages (ANSI, OEM, UTF-8,
UTF-16, UTF-16 BE), as well as #Favorite# code pages (the list of #favorite#
code pages can be specified in the ~Code pages~@CodepagesMenu@ menu
in the options, editor, or viewer). To search using a custom set of code pages,
select required code pages in the dropdown list with the #Ins# or #Space# keys,
then choose #Selected code pages# menu item.

 Ak chcete prehľadávať aj pre Far známe typy archívov, nastavte #Hľadať v archívoch#,
majte však napamäti, že to výrazne spomaľuje vyhľadávanie. Far nedokáže vyhľadávať vo
vnorených archívoch (čiže archívoch ktoré sú súčasťou iného archívu).

 The #Search for folders# option includes in search list those folders, that
match the wildcards. Also the counter of found files takes account of found
folders.

 The #Search in symbolic links# option allows searching files in
~symbolic links~@HardSymLink@ along with normal sub-folders.

 #Search in alternate streams# - besides the primary data stream (which is
the content of the file itself), allows to search alternate named data streams
supported by some file systems (for example, #NTFS#).

 Vyhľadávanie možno vykonávať:
 - ^<wrap>vo všetkých pevne pripojených jednotkách;
 - in all local drives, except removable and network;
 - in all folders specified in the %PATH% environment variable (not including subfolders).
 - v podadresároch kmeňového adresára, in the find dialog one can change disk drive of the search;
 - aktuálneho adresára;
 - len v aktuálnom adresári;
 - v označených adresároch.

 Toto nastavenie sa ukladá spolu s konfiguráciou.

 Check the #Use filter# checkbox to search for files according to user
defined conditions. Press the #Filter# button to open the ~filters menu~@FiltersMenu@.

 The #Advanced# button invokes the ~find file advanced options~@FindFileAdvanced@
dialog that can be used to specify extended set of search properties.


@FindFileAdvanced
$ #Find file advanced options#
 #Search only in the first# - The string entered in the #Containing text#
(or #Containing hex#) field can be searched for not only in the whole
file, but also within the specified range at the beginning of the file.
If the specified value is less than the file size, the rest of the file
is ignored even if the searched for sequence exists there.

 The following file size suffixes can be used:
 - #B# for bytes (no suffix also means bytes);
 - #K# for kilobytes;
 - #M# for megabytes;
 - #G# for gigabytes;
 - #T# for terabytes;
 - #P# for petabytes;
 - #E# for exabytes.

 #Column types# - Allows to specify search results output format.
Column types are encoded with one or more characters delimited with commas.
The following column types are supported:

 S[C,T,F,E] - ^<wrap>file size
 P[C,T,F,E] - allocation file size
 G[C,T,F,E] - size of alternate file streams, where:
              C - ^<wrap>format file size;
              T - use 1000 instead of 1024 as the divisor;
              F - show size as a decimal fraction using the most appropriate unit, e.g. 0.97 K, 1.44 M, 3.5 G;
              E - compact mode, no space before the file size suffix (e.g 0.97K).

 D          - file last write date
 T          - file last write time

 DM[B,M]    - file last write date and time
 DC[B,M]    - file creation date and time
 DA[B,M]    - file last access date and time
 DE[B,M]    - file change date and time, where:
              B - brief (Unix style) file time format;
              M - use text month names.

 A          - file attributes
 Z          - file description

 O[L]       - file owner, where:
              L - show domain name.

 LN         - number of hard links

 F          - number of alternate streams

 File attributes are denoted as follows:

 #N# - Attributes not set
 #R# - Read only
 #H# - Hidden
 #S# - System
 #D# - Directory
 #A# - Archive
 #T# - Temporary
 #$# - Sparse
 #L# - Reparse point
 #C# - Compressed
 #O# - Offline
 #I# - Not content indexed
 #E# - Encrypted
 #V# - Integrity stream
 #?# - Virtual
 #X# - No scrub data
 #P# - Pinned
 #U# - Unpinned

 #Column widths# - allows to change column widths in the search results
output. If the width equals 0, the default value is used.

 To use 12-hour time format, add one to the standard width of
the file time column or file date and time column. Increase the width
of these columns even more to show seconds and milliseconds.

 To display the year in 4-digit format, add two to the width of
the file date column.

 Unlike the panel modes, the search results can have only one column.
File name is always displayed and added automatically as the last column.

 Adding columns for the links and alternate streams information
(G, LN, and F) increases search time.

 To display only file names without additional attributes in the search
results, leave the “Column types” field empty.

 Default field values are:
 “Column types”  - D,S,A
 “Column widths” - 14,13,0


@FindFileResult
$ #Find file: control keys#
 Počas ~vyhľadávania~@FindFile@ alebo po jeho skončení môžete použiť kurzorové
klávesy pre skrolovanie zoznamu súborov a tlačítka na spustenie
želaných operácií.

 Počas alebo po skončení vyhľadávania máte k dispozícií nasledovné tlačítka:

 #Nové hľadanie#   Spustí nové vyhľadávanie (nové nastavenie).

 #Prejsť na#       Preruší bežiace vyhľadávanie, zmení aktuálny
                 adresár a presunie kurzor na označnený súbor.

 #Zobraziť#        Zobrazí označený súbor. Ak vyhľadávanie ešte
                 nie je ukončené, bude pokračovať po zobrazení.

 #Panel#           Vytvorí pomocný panel a naplní ho súbormi,
                 ktoré boli nájdené pri poslednom hľadaní.

 #Stop#            Preruší bežiace vyhľadávanie. Možné len ak
                 práve vyhľadávanie prebieha.

 #Storno#          Ukončí funkciu vyhľadávania.

 Počas alebo po skončení vyhľadávania môžu byť prezeranie a editovanie nájdených súborov.

 View                          #F3, Alt+F3, Numpad5, Ctrl+Shift+F3#

 #F3#, #Alt+F3# or #Numpad5# invokes ~internal~@Viewer@, external or ~associated ~@FileAssoc@ viewer,
depending on file type and ~Viewer settings~@ViewerSettings@.
#Ctrl+Shift+F3# always invokes internal viewer regardless of file associations.

 Edit                                    #F4, Alt+F4, Ctrl+Shift+F4#

 #F4# or #Alt+F4# invokes ~internal~@Editor@, external or ~associated~@FileAssoc@ editor,
depending on file type and ~Editor settings~@EditorSettings@.
#Ctrl+Shift+F4# always invokes internal editor regardless of file associations.

 Prezeranie a editovanie podporuje prídavnými modulmi (plugins) emulované súborové systémy. Note, that saving editor
changes by #F2# key in emulated file system will perform #SaveAs# operation,
instead of common #Save#.


@FindFolder
$ #Vyhľadanie adresára#
 Tento príkaz umožňuje rýchly pohľad na potrebný adresár v strome adresárov.

 Na nastavenie adresára môžete použiť kurzorové klávesy alebo
napísať prvé znaky mena adresára.

 Stlačením klávesu #Enter# sa prepnete do označeného adresára.

 Klávesmi #Ctrl+R# a #F2# môžete prikázať znovunačítanie stromu adresárov.

 #Gray +# and #Gray -# should move up and down the tree to the next branch
on the same level.

 #F5# allows to maximize the window, pressing #F5# again will restore the
window to the previous size.

 By pressing #Ctrl+Enter#, you can cycle through the folders matching the
part of the filename that you have already entered. #Ctrl+Shift+Enter# allows
to cycle backwards.

 See also the list of ~macro keys~@KeyMacroFindFolderList@, available in the find folder dialog.


@Filter
$ #Filter#
 Operations filter is used to process certain file groups according to
rules specified by the user, if those rules are met by some file it
will be processed by a corresponding operation. A filter can have several
rule sets.

 The filter dialog consists of the following elements:

 #Filter name#
 A title that will be shown in the filters menu. This field can be empty.

 #Mask#
 One or several ~file masks~@FileMasks@.
 Filter conditions are met if file mask analysis is on and its name matches any of the given file
masks. If mask analysis is off, file name is ignored.

 #Size#
 Minimum and maximum files size. The following file size suffixes can be used:
 - #B# for bytes (no suffix also means bytes);
 - #K# for kilobytes;
 - #M# for megabytes;
 - #G# for gigabytes;
 - #T# for terabytes;
 - #P# for petabytes;
 - #E# for exabytes.

 Filter conditions are met if file size analysis is on, and it is inside the given range.
If nothing is specified (empty line) for one or both range boundaries then file size for that
boundary is not limited.

 Example:
 >= 1K - select files greater than or equal to 1 kilobyte
 <= 1M - to less than or equal to 1 megabyte

 #Date/time#
 Starting and ending file date/time.
You can specify the date/time of last file #write#, file #creation#, last #access# and #change#.
 #Current# allows to fill the date/time fields with the current date/time after which you
can change only the needed part of the date/time fields, for example only the month or minutes.
 #Blank# will clear the date/time fields.
 Filter conditions are met if file date/time analysis is on and its date/time is in the
given range and corresponds to the given type. If one or both of the date/time limit
fields are empty then the date/time for that type is not limited.

 Example:
 <= 31.01.2010 - select files up to 31 numbers
 >= 01.01.2010 - but after Jan. 1, 2010

 Option #Relative# allows you to switch to work with the date in relative time.
The logic at work this option is similar to arithmetic with negative numbers.

 Example:
 <= 0  - select files in the period from the "Today"
 >= 30 - and 30-days ago, including

 #Attributes#
 Inclusion and exclusion attributes.

 Filter conditions are met if file attributes
analysis is on and it has all of the inclusion
attributes and none of the exclusion attributes:

 #[x]# - ^<wrap>inclusion attribute - the file must have this attribute.
 #[ ]# - ^<wrap>exclusion attribute - the file must not have this attribute.
 #[?]# - ^<wrap>ignore this attribute.

 The #Compressed#, #Encrypted#, #Not indexed#, #Sparse#, #Temporary# and #Offline# attributes
are used only on disks with the NTFS file system.
The #Integrity stream# and #No scrub data# attributes are supported only on ReFS volumes starting from Windows Server 2012.

 #Has more than one hardlink#
 Used only on disks with NTFS file system. Condition evaluates to true,
if piece of data, which current file is pointing, is also pointed at least by one another file.
 #Warning#: Enabling of this option can cause a significant slowdown.

 To quickly disable one or several conditions, uncheck the corresponding
checkboxes. The #Reset# button will clear all of the filter conditions.


@History
$ #História#
 Tento príkaz zobrazí zoznam príkazov, spustených s príkazového riadka Faru.
Besides the cursor control keys, the following keyboard shortcuts are
available:

 Opäť spustiť príkaz                                           #Enter#

 Spustiť v osobitnom okne                                #Shift+Enter#

 Re-execute a command as administrator                #Ctrl+Alt+Enter#

 Vložiť do príkazového riadku                             #Ctrl+Enter#

 Vymažete históriu príkazov                                      #Del#

 Lock/unlock the current history item                            #Ins#

 Delete the current history item                           #Shift+Del#

 Copy the text of the current command to the clipboard        #Ctrl+C#
 without closing the list                                or #Ctrl+Ins#

 Show additional information                                      #F3#

 Taktiež je možné skočiť na predošlý alebo nasledujúci príkaz
priamo z príkazového riadku použitím kombinácie #Ctrl+E#, resp. #Ctrl+X#.

 Ak chcete túto históriu uložiť pred ukončením programu, zvoľte
príslušné nastevenie v ~nastaveniach systému~@SystemSettings@.

 Locked items will not be deleted when clearing the history.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@HistoryViews
$ #History: file view and edit#
 The file view history shows the list of files that have been recently
viewed or edited. Besides the cursor control keys, the following keyboard
shortcuts are available:

 Reopen a file for viewing or editing                          #Enter#

 Copy the file name to the command line                   #Ctrl+Enter#

 Clear the history list                                          #Del#

 Delete the current history item                           #Shift+Del#

 Lock/unlock the current history item                            #Ins#

 Refresh list and remove non-existing entries                 #Ctrl+R#

 Copy the text of the current history item to the             #Ctrl+C#
 clipboard without closing the list                      or #Ctrl+Ins#

 Open a file in the ~editor~@Editor@                                        #F4#

 Open a file in the ~viewer~@Viewer@                             #F3# or #Numpad5#

 Items of the view and edit history are moved to the end of the list after
they are selected. You can use #Shift+Enter# to select an item without changing
its position.

 If you want to save the view and edit history after exiting Far, use the
respective option in the ~System settings~@SystemSettings@ dialog.

 Remarks:

 1. ^<wrap>List refresh operation (Ctrl+R) can take a considerable amount
of time if a file was located on a currently unavailable remote resource.
 2. ^<wrap>Locked items will not be deleted when clearing or refreshing the history.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@HistoryFolders
$ #History: folders#
 The folders history shows the list of folders that have been recently
visited. Besides the cursor control keys, the following keyboard shortcuts are available:

 Go to the current folder in the list                          #Enter#

 Open the selected folder in the passive panel      #Ctrl+Shift+Enter#

 Copy the folder name to the command line                 #Ctrl+Enter#

 Clear the history list                                          #Del#

 Delete the current history item                           #Shift+Del#

 Lock/unlock the current history item                            #Ins#

 Refresh list and remove non-existing entries                 #Ctrl+R#

 Copy the text of the current history item to the             #Ctrl+C#
 clipboard without closing the list                      or #Ctrl+Ins#

 Items of the folders history are moved to the end of the list after they
are selected. You can use #Shift+Enter# to select an item without changing its position.

 If you want to save the folders history after exiting Far, use the
respective option in the ~System settings~@SystemSettings@ dialog.

 Remarks:

 1. ^<wrap>List refresh operation (Ctrl+R) can take a considerable amount
of time if a folder was located on a currently unavailable remote resource.
 2. ^<wrap>Locked items will not be deleted when clearing or refreshing the history.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@TaskList
$ #Zoznam úloh#
 Zoznam úloh zobrazuje aktívne úlohy - programy. Každý
riadok zoznamu obsahuje hlavičku okna danej úlohy.

 Zo zoznamu úloh sa môžete prepnúť do okna danej úlohy, alebo ukončiť úlohu klávesom
#Del#. Pri ukončovaní úlohy buďte opatrní, lebo tým ukončíte úlohu okamžite a všetky
neuložené informácie sa stratie. Preto by táto funkcia mala byť používaná iba keď je
to skutočne nevyhnutné, napríklad na prerušenie programu, ktorý neodpovedá.

 Zoznam úloh možno vyvolať buď z ~menu príkazov~@CmdMenu@ alebo
pomocou #Ctrl+W#. Klávesovú skratku možno použiť aj v prezerači
a editore.

 #Ctrl+R# allows to refresh the task list.

 #F2# Switch between displaying of window caption and path to executable.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@HotPlugList
$ #Hotplug devices list#
 Hotplug devices list displays PC Card boards and other analogue devices
which are installed and work in the computer.

 To remove a device you need to select it in the list and press the #Del#
key. After that Windows will prepare the device for safe removal and a
notification will be displayed when it is safe to remove the device.

 #Ctrl+R# allows to refresh the list of connected devices.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@CompFolders
$ #Porovnanie adresárov#
 Príkaz porovnania adresárov je možné použiť len ak sú zobrazené dva
~súborové panely~@FilePanel@. Porovná sa obsah adresárov v nich zobrazených.
Súbory, ktoré existujú iba v jednom z panelov alebo ktoré majú novší dátum
 ako súbory s rovnakým menom v druhom paneli, sa označia.

 Podadresáre sa neporovnávajú. Súbory sa porovnávajú len podľa
mena, dĺžky a času, obsah súboru sa nekontroluje.


@UserMenu
$ #Užívateľské menu#
 Úlohou užívateľského menu je uľahčiť volanie často používaných
operácií. Obsahuje užívateľom definované príkazy a skupiny príkazov,
ktoré môžu byť spustené z užívateľského menu. Menu môže obsahovať aj podmenu.
~Špeciálne symboly~@MetaSymbols@ sú podporované tak v príkazoch samotných
ako i v ich názvoch. Samozrejme možno použiť aj symbol !?<title>?<init>!,
pomocou ktorého môžete zadať dodatočné parametre priamo pred spustením programov.

 Príkazom #Úprava užívateľského menu# z menu ~Príkazy~@CmdMenu@ môžete
upraviť alebo vytvoriť hlavné alebo lokálne užívateľské menu. Hlavné užívateľské
menu môže byť len jedno. Toto menu sa vyvolá ak pre daný adresár nebolo vytvorené
nijaké lokálne menu. Lokálne menu môže byť umiestnené v ľubovoľnom adresári. You can switch between
the main menu and the user menu by pressing #Shift+F2#. Also you can call the
user menu of the parent folder by pressing #BS#.

 You can add command separators to the user menu. To do this, you should add
a new menu command and define "#--#" as "hot key". To delete a menu separator,you
must switch to file mode with #Alt+F4# key.

 User menu items can be moved with #Ctrl+Up# or #Ctrl+Down# key combinations.

 Príkaz z užívateľského menu možno spustiť buď presunutím kurzora nad jeho
názov a stlačením klávesy Enter alebo jemu priradenou klávesovou skratkou.

 Podmenu alebo položku menu môžete zrušiť klávesom #Del#, nové menu alebo
položku môžete vložiť klávesom #Ins# a upraviť existujúce menu či položku
možno klávesom #F4#. Po stlačení #Alt+F4# budete môcť menu editovať v textovej forme.

 Ako klávesové skratky možno v menu použiť číslice, písmená a funkčné klávesy (#F1#…#F24#).
Ak použijete #F1# alebo #F4#, znemožníte tým ich originálnu funkciu.
Každopádne na úpravu môžete ešte vždy použiť kombináciu #Shift+F4#.

 Keď upravujete alebo vytvárate položku menu, mali by ste zadať klávesovú
skratku pre rýchly prítup, názov príkazu, ktorý sa bude zobrazovať v menu,
a postupnosť príkazov ktoré sa majú vykonať pri zvolení tejto položky.

 Keď upravujete alebo vytvárate podmenu, mali by ste zadať iba
klávesovú skratku a názov.

 Užívateľské menu sú uložené v textových súboroch #FarMenu.Ini#:
 - ^<wrap>Global user menu, by default, is located in the Far Manager folder.
If global user menu file exists it will override the user specific menu.
 - User specific user menu is located in the user profile.
 - Local user menu is located in the current folder.

 To close the menu even if submenus are open use #Shift+F10#.

 See also:

 The list of ~macro keys~@KeyMacroUserMenuList@, available in the user menu.
 Common ~menu~@MenuCmd@ keyboard commands.


@FileAssoc
$ #File associations#
 Far Manager supports file associations, that allow to associate various
actions to running, viewing and editing files with a specified
~mask~@FileMasks@.

 Nové typy súborov môžete vytvoriť pomocou príkazu #Typy súborov# z menu
~Príkazy~@CmdMenu@.

 You can define several associations for one file type and select the
desired association from the menu.

 The following actions are available in the associations list:

 #Ins#        - ~add~@FileAssocModify@ a new association

 #F4#         - ~edit~@FileAssocModify@ the current association

 #Del#        - delete the current association

 #Ctrl+Up#    - move association up

 #Ctrl+Down#  - move association down

 See also: common ~menu~@MenuCmd@ keyboard commands.


@FileAssocModify
$ #Typy súborov: editing#
 Far dovoľuje priradiť každému typu súborov, definovanému
~maskou~@FileMasks@, až šesť príkazy:

 #Vykonať príkaz#                Performed if #Enter# is pressed
 #(used for Enter)#

 #Príkaz pre zobrazenie#         Performed if #Ctrl+PgDn# is pressed
 #(used for Ctrl+PgDn)#

 #Príkaz pre zobrazenie#         Performed if #F3# is pressed
 #(used for F3)#

 #Príkaz pre zobrazenie#         Performed if #Alt+F3# is pressed
 #(used for Alt+F3)#

 #Príkaz pre editovanie#         Performed if #F4# is pressed
 #(used for F4)#

 #Príkaz pre editovanie#         Performed if #Alt+F4# is pressed
 #(used for Alt+F4)#

 Názov daného typu môžete uviesť do poľa #Popis typu#.

 ~Special symbols~@MetaSymbols@ can be used in the associated command.

 Notes:

 1. ^<wrap>If no execute command is associated with file and
#Use Windows registered types# option in ~System settings~@SystemSettings@
dialog is on, Far tries to use Windows association to execute this file type.
 2. ^<wrap>Operating system ~commands~@OSCommands@ "IF EXIST" and "IF DEFINED"
allow to configure "smarter" associations - if you have specified several
associations for a file type, the menu will show only the associations
for which the conditions are true.
 3. ^<wrap>If the specified mask is a regular expression, its capturing groups can be referenced in the commands as %RegexGroup#N# or %RegexGroup{#Name#}.


@MetaSymbols
$ #Special symbols#
 Môžete používať nasledovné špeciálne symboly v ~priradenom príkaze~@FileAssoc@,
~user menu~@UserMenu@ and the command ~"Apply command"~@ApplyCmd@:

 #!!#       ^<wrap>Znak '!'
 #!#        Dlhé meno súboru bez prípony
 #!~~#       Krátke meno súboru bez prípony
 #!`#       Long extension without file name (ext)
 #!`~~#      Short extension without file name (ext)
 #!.!#      Dlhé meno súboru s príponou
 #!-!#      Krátke meno súboru s príponou
 #!+!#      Podobné ako !-! ale ak sa dlhé meno súboru stratilo vykonaním prílazu, Far ho vráti späť
 #!@@!#      Názov súboru so zoznamom označených súborov
 #!$!#      Názov súboru so zoznamom krátkych mien označených súborov
 #!&#       List of selected files
 #!&~~#      List of selected short file names
 #!:#       Aktuálna jednotka
 #!\\#       Aktuálna cesta
 #!/#       Krátke meno aktuálnej cesty
 #!=\\#      Current path considering ~symbolic links~@HardSymLink@.
 #!=/#      Short name of the current path considering ~symbolic links~@HardSymLink@.
 #!?!#      Description of the current file

 #!?<title>?<init>!#
 Tento symbol je pri spustení príkazu nahradnený
užívateľských vstupom. <title> a <init> - hlavička a počiatočný text vstupného poľa dialógového okna.
 Dovolené je použiť aj niekoľko takýchto symbolov v jednom riadku, napríklad:
 grep !?Hľadaj:?! !?V súboroch:?*.*!|c:\\far\\Far.exe -v -

 A history name for the <init> string can be supplied in the <title>. In such case the command has the following format:
#!?$<history>$<title>?<init>!#, for example:
 grep !?#$GrepHist$#Search for:?! !?In:?*.*!|Far.exe -v -

 Leave the name empty to disable history.

 The entered string can also be accessed later as #%<history># (or as #%UserVarN#, where N is the index of the corresponding input).

 In <title> and <init> the usage of other meta-symbols is allowed by enclosing them in brackets, e.g.
grep !?Find in (!.!):?! |Far.exe -v -.

 #!###
 Ak pred niektorým špeciálnym symbolom uvediete "!##", znamená to
referenciu pasívneho panelu (see note 4).
Napríklad !##!.! označuje aktuálny súbor
v pasívnom paneli.

 #!^#
 "!^" prefix forces all subsequent special symbols
to refer to the active panel (see note 4).
For example, !^!.! denotes a current file name on
the active panel, !##!\\!^!.! - a file on the passive
panel with the same name as the name of the current
file on the active panel.

 #![#
 "![" prefix forces all subsequent special symbols
to refer to the left panel (see note 4).
For example, ![!.! denotes a current file name on
the left panel, ![!\\!^!.! - a file on the left
panel with the same name as the name of the current
file on the active panel.

 #!]#
 "!]" prefix forces all subsequent special symbols
to refer to the right panel (see note 4).
For example, !]!.! denotes a current file name on
the right panel, !]!\\!^!.! - a file on the right
panel with the same name as the name of the current
file on the active panel.

 Poznámky:

 1. ^<wrap>When handling special characters, Far substitutes only the string
corresponding to the special character. No additional characters (for example,
quotes) are added, and you should add them yourself if it is needed. For
example, if a program used in the associations requires a file name to be
enclosed in quotes, you should specify #program.exe "!.!"# and not
#program.exe !.!#.

 2. ^<wrap>The following modifiers can be used with the special symbols #!@@!# and #!$!#:
    #Q# - enclose names in quotes;
    #S# - use ‘/’ instead of ‘\\’ in pathnames;
    #F# - use full pathnames;
    #A# - use ANSI code page;
    #U# - use UTF-8 code page;
    #W# - use UTF-16 (Little endian) code page.

    ^<wrap>For example, #!@@AFQ!# denotes "name of file containing the list of
selected file names, in ANSI encoding, with full pathnames, each enclosed in quotes".

    ^<wrap>The following modifiers can be used with the special symbols #!&# and #!&~~#:
    #Q# - enclose each name in quotes. This is the default, if no modifier is specified.
    #q# - do not enclose names in quotes (as it was before build 5466).

    ^<wrap>For example, #!&Q# denotes the list of selected file names, each enclosed in quotes.

 3. ^<wrap>When there are multiple associations specified, the meta-characters !@@!
and !$! are shown in the menu as is. Those characters are translated when the command is executed.

 4. ^<wrap>The prefixes "!##", "!^", "![" and "!]" work as toggles. The effect
of these prefixes continues up to the next similar prefix. For example:

    if exist !##!\\!^!.! diff -c -p !##!\\!^!.! !\\!.!

    ^<wrap>"If the same file exists on the passive panel as the file under
the cursor on the active panel, show the differences between
the file on the passive panel and the file on the active panel,
regardless of the name of the current file on the passive panel"

 5. ^<wrap>If it is needed to pass to a program a name with an ending
backslash, use quotes, e.g. #"!"\#.
    ^<wrap>For example, to extract a rar archive to a folder with the same name:

    #winrar x "!.!" "!"\#


@SystemSettings
$ #Nastavenia systému#
 #Odstraňovať do Koša#
 Nastaví vymazávanie do koša. The operation of deleting to the Recycle
Bin can be performed only for local hard disks.

 #Použiť kopírovanie cez systém#
 Nastaví používanie funkcie CopyFileEx z Windows namiesto vnútornej implementácie
kopírovania súborov. Môže to byť výhodné u NTFS, lebo táto funkcia kopíruje rozšírené atribúty. On the other hand, when using the system
function, the possibility of "smart" ~copying~@CopyFiles@ of sparse files is not available.

 #Kopírovať aj súbory otvorené na zápis#
 Dovoľuje kopírovať súbory, ktoré sú iným programom otvorené pre zapisovanie.
Tento mód je šikovný na skopírovanie súboru otvoreného dlhú dobu, ale môže
byť nebezpečný, ak sa súbor modifikuje súčasne s tým ako sa kopíruje.

 #Scan symbolic links#
 Scan ~symbolic links~@HardSymLink@ along with normal sub-folders when building the folder tree,
determining the total file size in the sub-folders.

 #Ukladať históriu príkazov#
 Ak je táto možnosť zapnutá, ~história príkazov~@History@ sa bude ukladať pri ukončení a opätovne načítavať pri spustení Faru.

 #Ukladať históriu adresárov#
 Spôsobí uloženie ~histórie adresárov~@HistoryFolders@ pred ukončením a jej načítanie pri spustení Faru.
Históriu adresárov možno vyvolať stlačením #Alt+F12#.

 #Ukladať históriuzobrazovania a editovania#
 Spôsobí uloženie ~histórie prezeraných a editovaných súborov~@HistoryViews@ pred ukončením a jej načítanie pri spustení Faru.
Históriu prezeraných a editovaných súborov možno vyvolať stlačením #Alt+F11#.

 #Použiť registrované typy Windows#
 Ak je táto možnosť zapnutá, potom stlačenie klávesu Enter na súbore, ktorého typ je známy pre
Windows a nenachádza sa vo Fare nastavených ~typoch súborv~@FileAssoc@, spustí sa program,
ktorý je vo Windows nastavený pre tento typ súboru.

 #Automatic update of environment variables#
 Automatically update the environment variables if they have been changed globally.

 #Request administrator rights#
 The current user might not always has the required rights to work with certain file system objects.
Far allows to retry the operation using the privileged account.
 Available options:
 - ^<wrap>#for modification#: allow requesting rights for operations that change the state of the file system (e.g. file or directory creation/modification/deletion)
 - ^<wrap>#for read#: allow requesting rights for operations that do not change the state of the file system (e.g. reading files or listing directories).
 - ^<wrap>#use additional privileges#: attempt accessing all files bypassing Access Control Lists.
Use with caution.

 #Sorting collation#
 Allows to choose and configure the sorting collation.
 - ^<wrap>#ordinal#: based on the ordinal value of the characters in the string
 - ^<wrap>#invariant#: invariant collation
 - ^<wrap>#linguistic#: based on the culture-specific sorting conventions

 #Treat digits as numbers#
 When enabled, sequential groups of digits are treated as numbers. The following example shows how the files are sorted:
 ┌──────────┬──────────┐
 │ Disabled │ Enabled  │
 ├──────────┼──────────┤
 │ Ie4_01   │ Ie4_01   │
 │ Ie4_128  │ Ie4_128  │
 │ Ie401sp2 │ Ie5      │
 │ Ie5      │ Ie6      │
 │ Ie501sp2 │ Ie401sp2 │
 │ Ie6      │ Ie501sp2 │
 │ 11.txt   │ 5.txt    │
 │ 5.txt    │ 11.txt   │
 │ 88.txt   │ 88.txt   │
 └──────────┴──────────┘
 Note: treating digits as numbers in linguistic collation is possible in Windows 7 and above. In older systems invariant collation will be used automatically.

 #Case sensitive#
 Take into account the case of the characters in the string.

 Note: how exactly the case of the characters in the string will be taken into account depends on the sorting collation.

 #Automaticky ukladať nastavenia#
 Ak je zapnuté, Far bude nastavenia ukladať automaticky. Taktiež sa uložia adresáre v oboch aktívnych paneloch.


@PanelSettings
$ #Nastavenia panelov#
 #Zobrazovať skryté a systémové súbory#
 Umožňuje zobraziť súbory s nastaveným atribútom Skrytý a Systémový. Toto nastavenie možno prepínať klávesovou kombináciou #Ctrl+H#.

 #Zvýrazňovať súbory#
 Zapne ~zvýrazňovanie súborov~@Highlight@.

 #Označovať adresáre#
 Zapne označovanie adresárov pri použití sivého #+# a #*#. Ináč budú tieto klávesy označovať iba súbory.

 #Right click selects files#
 If this option is on, #right mouse click# selects files. Otherwise, it opens Windows  Explorer Context menu.

 #Sort folder names by extension#
 Applies sort by extension not only to files, but also to folders. When this option is turned on, sorting
by extension works the same as it did in Far 1.65. If the option is turned off, in the extension sort mode the
folders will be sorted by name, as they are in the name sort mode.

 #Povoliť triedenie v opačnom poradí#
 Ak je zapnuté a znovu sa vyberie aktuálny mód zoraďovania, nastaví zoraďovanie v opačnom poradí.

 #Disable automatic panel update#
 If this option is on and the number of file objects exceeds the specified value,
the automatic panel update when the file system state changes is disabled.
 Auto-update works only for FAT/FAT32/NTFS file systems.
The value of 0 means "update always".
To force an update of the panels press #Ctrl+R#.

 #Network drives autorefresh#
 This option enables panel autorefresh when the file system state of a network drive is changed.
It may be helpful to disable this option on slow network connections.

 #Zobrazovať názvy stĺpcov#
 Zapne zobrazovanie názvov stĺpcov v ~súborovom paneli~@FilePanel@.

 #Zobraz. stavový riadok#
 Zapne zobrazovanie stavového riadku v súborovom paneli.

 #Detect volume mount points#
 Distiguishes between normal directory links (Junctions) and volume mount points.
This option significanty slows down displaying directories on slow network connections.

 #Zobrazovať celkovú dĺžku a počet súborov#
 Zapne zobrazovanie sumárnych informácii v spodnom riadku súborového panelu.

 #Zobrazovať voľné miesto#
 Zapne zobrazovanie veľkosti voľného miesta na aktuálnom disku.

 #Zobrazovať posuvník#
 Zapne zobrazovanie posuvníkov (jazdcov) v súborovom a ~stromovom~@TreePanel@ paneli.

 #Zobrazovať počet obrazoviek v pozadí#
 Zapne zobrazovanie počtu obrazoviek ~v pozadí~@ScrSwitch@.

 #Zobrazovať písmeno zoradenia#
 Zobrazuje nastavený zoraďovanie v ľavom hornom rohu panelu.

 #Show ".." in root folders#
 Enables displaying of ".." item in root folders.
Pressing #Enter# on this item opens ~Change drive menu~@DriveDlg@.


@TreeSettings
$ #Tree settings#
 #Automaticky meniť adresár#
 Ak je zapnuté, pohyb kurzora v ~stromovom paneli~@TreePanel@ spôsobí zmenu adresára v druhom paneli.
Ak nie je zapnuté, na zmenu adresára je potrebné stlačiť #Enter#.

 #Minimum number of folders#
 The minimal number of folders on the disk for which folder tree cache file #tree3.far# will be created.


@InterfSettings
$ #Nastavenia prostredia#
 #Čas#
 Zobrazí v pravom hornom rohu obrazovky čas.

 #Myš#
 Zapne používanie myši.

 #Zobrazovať tlačítka funkčných kláves#
 Zobrazí na spodnom riadku obrazovky riadok s krátkymi popismi funkcií kláves.
Taktiež možno použiť kombináciu #Ctrl+B#.

 #Trvale zobrazovať horné menu#
 Horný riadok z položkami menu bude zobrazený aj keď menu nebude aktívne.

 #Šetrič obrazovky#
 Po nastavenom čase (v minútach) bez aktivity spustí šetrič obrazovky. When this option
is enabled, screen saver will also activate when mouse pointer is brought
to the upper right corner of Far window.

 #Zobrazuj indikátor celkovej dĺžky kopírovania#
 Vyvolá zobrazenie indikátora celkovej dĺžky pri kopírovaní súborov.
To môže vyžadovať pred začatím samotného kopírovania trochu času navyše
na výpočet celkovej dĺžky súborov.

 #Show copying time information#
 Show information about average copying speed, copying time and
estimated remaining time in the copy dialog.
 Since this function requires some time to gather statistics, it is likely
that you won't see any information if you're copying many small files
and the option "Show total copy progress indicator" is disabled.

 #Show total delete indicator#
 Show total progress bar, when performing a file delete operation.
This could require some additional time before starting deleting
to calculate the total files count.

 #Use Ctrl+PgUp to change drive#
 Pressing #Ctrl+PgUp# in the root directory shows the drive selection menu.
 If Network plugin is installed, for network folders (and network drives, if switch
is in the third state) a list of server shared resources  will be shown.

 #Use Virtual Terminal for rendering#
 Render the output using ANSI escape sequences. You can find more about it ~here~@https://docs.microsoft.com/en-us/windows/console/classic-vs-vt@.
 This allows using 8 and 24-bit colors, text styles, and may (or may not) work better (or worse) with some Unicode characters.
 Requires Windows 10 and above.

 #Fullwidth-aware rendering#
 Take into account the fact that East Asian characters require two screen cells instead of one.
 The support is rudimentary and experimental. It may work or not, depending on your OS, locale, terminal, font and other settings.

 #ClearType friendly redraw#
 Redraw the window in such a way that ClearType related artifacts do not appear.
 #Attention!#: Enabling this option can considerably slow down the redraw speed.

 #Console icon#
 If this option is activated, Far will set the selected embedded icon as the icon of the console window.
Otherwise you can choose any desired icon in the Far shourtcut properties.

 #Alternate for Administrator#
 If this option is activated, Far will use the red icon when running with administrator privileges.

 #Far window title addons#
 Additional information, displayed in the window title.
Can contain any text, including environment variables (e.g. "%USERDOMAIN%\%USERNAME%") and the following special variables:
 - #%Ver# - Far version;
 - #%Platform# - Far platform;
 - #%Admin# - ^<wrap>"Administrator" if running as administrator, otherwise an empty string.
 - #%PID# - Far process ID;


@DialogSettings
$ #Settings dialog: dialogs#
 #História vo vstupoch#    Zapne ukladanie histórie vo všetkých
 #dialógových okien#       dialógových vstupných oknách. Zoznam
                         predtým použitých reťazcov (vstupov)
                         možno vyvolať myšou alebo použitím
                         #Ctrl# zo šípkou nahor, resp. nadol. Ak
                         si takúto históriu neželáte, napr. z
                         bezpečnostných dôvodov, vypnite ju tu.

 #Persistent blocks#       Do not remove block selection after moving
 #in edit controls#        the cursor in dialog edit controls and
                         command line.

 #Del removes blocks#      If a block is selected, pressing Del will
 #in edit controls#        not remove the character under cursor, but
                         this block.

 #AutoComplete#            Allows to use the AutoComplete function
 #in edit controls#        in edit controls that have a history list
                         or in combo boxes. When the option is
                         disabled, you can use the #Ctrl+End# key
                         to autocomplete a line. The autocomplete
                         feature is disabled while a macro is
                         being recorded or executed.

 #Backspace deletes#       If the option is on, pressing #BackSpace#
 #unchanged text#          in an unchanged edit string deletes
                         the entire text, as if #Del# had been
                         pressed.

 #Mouse click outside#     #Right/left mouse click# outside a dialog
 #a dialog closes it#      closes the dialog (see ~Common~@MiscCmd@).
                         This option allows to switch off this
                         functionality.

 See also the list of ~macro keys~@KeyMacroDialogList@, available in dialogs.


@VMenuSettings
$ #Menu settings#
 #Left/Right/Middle mouse click outside a menu#
 You can choose action for mouse buttons, when click occures outside a menu:
#Cancel menu#, #Execute selected item# or #Do nothing#.


@CmdlineSettings
$ #Settings dialog: command line#
 #Persistent blocks#
 Do not remove block selection after moving the cursor in command line.

 #Del removes blocks#
 If a block is selected, pressing Del will not remove the character under cursor, but this block.

 #AutoComplete#
 Allows to use the AutoComplete function in command line. When the option is
disabled, you can use the #Ctrl+Space# key to autocomplete a line. The autocomplete
feature is disabled while a macro is being recorded or executed.

 #Set command line prompt format#
 This option allows to set the default Far command ~line prompt~@CommandPrompt@.

 #Use home dir#
 This option specifies the target folder of ~CD ~~~@OSCommands@ command.
If the string is empty, #CD ~~# will attempt to change the current path
to real “~~” directory (and fail if this is impossible, e.g. because
the directory does not exist). Default value is string “%FARHOME%” which
denotes Far home directory.


@AutoCompleteSettings
$ #Settings dialog: AutoComplete#
 #Show a list#
 Show completion suggestions as a list.

 #Modal mode#
 Make the list modal.

 #Append the first matched item#
 Append the first matched item to the command line text as you type.

 There are several additional options to control what data sources will be used to populate the completion list:
 - #Interface.Completion.UseFilesystem#
 - #Interface.Completion.UseHistory#
 - #Interface.Completion.UsePath#
 - #Interface.Completion.UseEnvironment#
 All parameters are 3-state - yes / no / only if called manually (using #Ctrl+Space#).
These parameters can be changed via ~far:config~@FarConfig@


@CommandPrompt
$ #Command line prompt format#
 Far umožňuje zmeniť štandard zobrazovania úvodnej informácie v príkazovom riadku.
To change it you have to enter the needed sequence of variables and
special code words in the #Set command line prompt format# input field
of the ~Command line settings~@CmdlineSettings@ dialog, this will allow showing
additional information in the command prompt.

 Môžete použiť environment variables and nasledovné formátovacie premenné:

 $a - ^<wrap>znak &
 $b - znak |
 $c - znak (
 $d - current date (depends on system settings)
 $f - znak )
 $g - znak >
 $h - delete the previous character
 $l - znak <
 $m - full network path of the current drive or empty, if the current drive is not a network drive
 $n - písmeno aktuálnej jednotky
 $p - aktuálna drive and cesta
 $q - znak =
 $s - space
 $t - current time in HH:MM:SS format
 $w - current working directory (without the path)
 $$ - znak $
 $+ - the depth of the folders stack
 $##nn - ^<wrap>max prompt width, given in percents relative to the width of the window
 $@@xx - ^<wrap>"Administrator", if Far Manager is running as administrator.
xx is a placeholder for two characters that will surround the "Administrator" word.
For example, #$@@{}$s$p$g# will be shown as "{Administrator} C:\>"

 By default the #$p$g# sequence is used - current drive and path ("C:\>").

 Examples:

 1. ^<wrap>A prompt of the following format #[%COMPUTERNAME%]$S$P$G#
will contain the computer name, current drive and path
(the %COMPUTERNAME% environment variable must be defined)

 2. ^<wrap>A prompt of the following format #[$T$H$H$H]$S$P$G# will
display the current time in HH:MM format before the current drive and path

 3. ^<wrap>Code "$+" displays the number of pluses (+) needed according to
current ~PUSHD~@OSCommands@ directory stack depth, one character per each
saved path.

 Prompt elements can be highlighted with #colors#.

 Format:
 #([[T]FFFFFFFF][:[T]BBBBBBBB][:style[:[T]UUUUUUUU]])#, where:

  #FFFFFFFF#
  Foreground color in aarrggbb format or index in the console palette.

  #BBBBBBBB#
  Background color in aarrggbb format or index in the console palette.

  #style#
  One or more text styles, separated by spaces:
  #bold#
  #italic#
  #overline#
  #strikeout#
  #faint#
  #blink#
  #inverse#
  #invisible#
  #underline#
  #underline_double#
  #underline_curly#
  #underline_dot#
  #underline_dash#

  #UUUUUUUU#
  Underline color in aarrggbb format or index in the console palette.

  #T#
  "TrueColor" flag. If absent, value is treated as the console palette index (00-FF):

  \00 \11 \22 \33 \44 \55 \66 \77 \88 \99 \AA \BB \CC \DD \EE \FF \-
  0123456789ABCDEF

 If a color is omitted, the corresponding default value will be used.

 Examples:

 #(E)#               \0e Bright yellow text on default background    \-
 #(:F)#              \f7 Default text on white background            \-
 #(B:C)#             \cb Bright blue text on bright red background   \-
 #()#                \07 Default text on default background          \-
 #(T00CCCC:TE34234)# \(T00CCCC:TE34234) Robin egg blue text on Vermilion background \-

 The specified color will be active till the end of the prompt or the next color entry.

 Example:

 #(a)%username%(f)@@(b)%computername%() $p$g# \0aadmin\0f@@\0bserver\07 C:\\>\-


@Viewer
$ #Zabudovaný prezerač: control keys#
 Príkazy prezerača

 The behavior of navigation keys depends on the ~view mode~@ViewerMode@.

 The following keys work in all modes:

 #Nahor#              Riadok vyššie
 #Nadol#              Riadok nižšie
 #PgUp#               Stranu vyššie
 #PgDn#               Stranu nižšie
 #Home, Ctrl+Home#    Začiatok súboru
 #End, Ctrl+End#      Koniec súboru

 The following additional keys work in #text mode without line wrap#:

 #Doľava#             Znak vľavo
 #Doprava#            Znak vpravo
 #Ctrl+Doľava#        20 znakov vľavo
 #Ctrl+Doprava#       20 znakov vpravo
 #Ctrl+Shift+Left#    Show the leftmost column
 #Ctrl+Shift+Right#   Show the rightmost column of all lines currently visible on the screen

 The following additional keys work in #dump# and #hex# modes:

 #Ctrl+Left#          ^<wrap>Shift all characters (#dump# mode) or bytes (#hex# mode) to the right
moving the last character (byte) of a row to the first positions of the next row
 #Ctrl+Right#         Shift all characters (#dump# mode) or bytes (#hex# mode) to the left
moving the first character (byte) of a row to the last position of the previous row

 The following additional keys work in #hex mode#:

 #Alt+Left#           ^<wrap>Decrement the number of bytes per row
 #Alt+Right#          Inrement the number of bytes per row
 #Ctrl+Alt+Left#      Decrease the number of bytes per row to the nearest multiple of 16-bytes
 #Ctrl+Alt+Right#     Increase the number of bytes per row to the nearest multiple of 16-bytes

 Viewer commands

 #F1#                 ^<wrap>Nápoveda
 #F2#                 Zapína/vypína rozdeľovanie dlhých riadkov in #text# mode, or change ~view mode~@ViewerMode@
 #Shift+F2#           Toggle wrap type (characters or words) in #text# ~view mode~@ViewerMode@
 #F4#                 Prepína ~view mode~@ViewerMode@ to #hex# and back
 #Shift+F4#           Select ~view mode~@ViewerMode@: #text#, #hex#, or #dump#
 #F6#                 Prepne do ~editora~@Editor@
 #F7#                 ~Hľadanie~@ViewerSearch@
 #Shift+F7, Space#    Continue searching forward
 #Alt+F7#             Continue searching backwards
 #F8#                 Prepína mód zobrazovania OEM/ANSI
 #Shift+F8#           Select code page using the ~Code pages~@CodePagesMenu@ menu
 #Alt+F8#             ~Zmeň aktuálnu pozíciu~@ViewerGotoPos@
 #Alt+F9#             Maximize or restore the size of the Far console window;
see also ~Interface.AltF9~@Interface.AltF9@
 #Alt+Shift+F9#       Open ~Viewer settings~@ViewerSettings@ dialog
 #NumKlav5,F3,F10,Esc# Koniec (opustenie prezerača)
 #Ctrl+F10#           Jump to the current file on the active file panel
 #F11#                Zobrazí menu ~Príkazy prídavných modulov~@Plugins@
 #Alt+F11#            Display ~file view and edit history~@HistoryViews@
 #+#                  Choď na nasledujúci súbor
 #-#                  Choď na predošlý súbor
 #Ctrl+O#             Ukáž užívateľskú obrazovku
 #Ctrl+Alt+Shift#     Temporarily show user screen
(while the key combination is held down)
 #Ctrl+B#             Toggle functional key bar at the bottom of the screen
 #Ctrl+Shift+B#       Toggle status line
 #Ctrl+S#             Toggle the scrollbar
 #Alt+BS, Ctrl+Z#     Späť na predošlú pozíciu
 #PravýCtrl+0…9#      Ulož aktuálnu pozíciu 0…9
 #Ctrl+Shift+0…9#     Ulož aktuálnu pozíciu 0…9
 #ĽavýCtrl+0…9#       Choď na uloženú pozíciu 0…9
 #Ctrl+Ins, Ctrl+C#   Copy the selected text to the clipboard.
The text can be selected manually or as the result of a ~search~@ViewerSearch@.
 #Ctrl+U#             Unselect the text
 #Shift+Mouse click#  Select text manually. The first mouse click indicates the
beginning of the selected area; the second click indicates the end.
Use navigation keys after the first click to bring the end position into
the view. The end of the selected area may be set before or after the
beginning in the text.

 See also the list of ~macro keys~@KeyMacroViewerList@ available in the viewer.

 Poznámky:

 1. ^<wrap>The viewer opens files with the permission to be deleted.
If another process attempts to delete the file while it is open in the
viewer, the file will be deleted after the viewer is closed. Any
operation on a file while its deletion is pending will fail. This is
a feature of the Windows operating system.

 2. ^<wrap>The maximum number of columns displayed in the #text#
~view mode~@ViewerMode@ can be configured in the
~Viewer settings~@ViewerSettings@ dialog. The range is between 100 to 100,000,
the default is 10,000. Lines longer than the maximum will be split into
several screen rows even if word wrap mode is turned off.

 3. ^<wrap>Far starts ~searching~@ViewerSearch@ (#F7#) from the
beginning of the currently visible area.

 4. ^<wrap>To auto-scroll a file which is being appended by another
process (conf. Linux “tail”), go to the end of the file (press the #End# key).


@ViewerMode
$ #Viewer: view modes#
 The viewer can render the content of the file in three modes:
#text#, #hex#, and #dump#. Current mode is indicated with a character
on the first (status) line of the window, to the left of the current
code page number:
 - #t#: text,
 - #h#: hex,
 - #d#: dump.

 When a file is opened, if #Save view mode# option in the
~Viewer settings~@ViewerSettings@ dialog is on and the file exists
in ~File view and edit history~@HistoryViews@, the last used view mode
is selected. Otherwise, if #Detect dump view mode# option in the
~Viewer settings~@ViewerSettings@ dialog is on and Far considers the
file binary, the #dump# mode is selected. Otherwise, the #text# mode is selected.

 The view mode can be changed manually with the following keys:

 #Shift+F4#
 Opens the #View mode# menu. If #text# or #dump#
mode is selected, it becomes the #base# mode; selecting #hex# mode
switches the current mode but does not change the base mode.

 #F4#
 Switches #text# or #dump# mode to #hex#, and
#hex# mode to the base (#text# or #dump#) mode most recently selected
in the #View mode# menu.

 #F2#
 In the #text# mode toggles line wrap/unwrap,
switches #dump# mode to #text#, and switches #hex# mode to the opposite
of the base mode (#dump# or #text#) most recently selected in the
#View mode# menu. Note: #F4# and #F2# switch #hex# mode to different
modes.

 See also the full list of ~viewer commands~@Viewer@.

 #Text# mode

 In the #text# mode viewer renders file content interpreting byte
sequences as character strings using the encoding defined by the current
code page. (Note that some encodings can use more than one byte
to represent a character.) Byte sequences invalid in the current
encoding and characters for which there are no glyphs in the console
window font are displayed as question marks, or empty rectangles,
or small question marks in a rectangle, or blanks. The representation
depends on the console window font.

 Text lines are broken at any conventional line delimiter, U+000A
U+000D (Dos/Windows format), U+000A (Unix format), or U+000D (Mac
format).

 Long text lines which do not fit into the window can be either
truncated or wrapped over multiple screen rows. The #F2# key switches
between #wrap# and #truncate# modes. In #wrap# mode, #Shift+F2# key
combination controls whether the lines can be broken inside a word.

 In #truncate# mode, the text can be scrolled horizontally within the
window. The #Right# key scrolls the text one column to the left; the
#Left# key scrolls one column to the right (think of moving the window
over the file content). The #Ctrl+Right# and #Ctrl+Left# key
combinations scroll 20 columns at a time. If #Show scrolling arrows#
in the ~Viewer settings~@ViewerSettings@ dialog is on, the truncated
lines are indicated with the #«# and #»# characters at the corresponding
edge of the window. The characters are displayed in a different color.

 The maximum length of text lines is limited to the #Maximum line width#
defined in the ~Viewer settings~@ViewerSettings@ dialog. Longer lines
are split into several screen rows even in #truncate# mode.

 #Dump# mode

 In the #dump# mode there is no notion of a text line.
The viewer renders file content character by character
without regard of line breaks or control codes which are treated
as ordinary characters. The characters are displayed on screen rows from
left to right. After reaching the end of the row, the next character
is displayed in the leftmost position of the next row.

 NOTE: Strictly speaking, text is rendered by code units, not by
characters. The size of a code unit depends on the encoding defined
by the current code page; it is one byte for single-byte encodings
(e.g. all ANSI code pages) and UTF-8, and two bytes for UTF-16 and
UTF-16BE encodings. For example:

 Code page 1252 (ANSI - Latin I): each byte is displayed in its
own screen position.

 Code page 65001 (UTF-8): the character is displayed in the
position corresponding to the leading byte of the UTF-8 sequence, and
the positions of continuation bytes are filled with the #›# characters
(code point U+203A).

 Code page 1200 (UTF-16): each screen position represents two
consecutive bytes starting at an even offset in the file.

 #Hex# mode (hexadecimal codes)

 In the #hex# mode viewer renders hexadecimal representation of the
bytes in the file. Each row starts with the hexadecimal offset of the
first byte and ends with the character representation of the bytes
of the row.

 The rendition depends on the encoding defined by the current code page.
For single-byte encodings (e.g. all ANSI code pages), the bytes on each
row are represented by the sequence of double-digit hex values followed
by the character sequence of the same length. For UTF-8 encoding, the
bytes are represented the same way, while the characters are displayed
at the positions of the leading bytes of the UTF-8 sequences with the
positions of continuation bytes being filled with the #›# characters
(code point U+203A). For UTF-16(BE) encodings, each pair of double-digit
hex values is represented by one character. For example:

 Code page 1252 (ANSI - Latin I)

@-
 \1b0000000000: 54 68 65 20 71 75 69 63 │ 6B 20 62 72 6F 77 6E 20  The quick brown \-
 \1b0000000010: 66 6F 78 20 6A 75 6D 70 │ 73 20 6F 76 65 72 20 74  fox jumps over t\-
 \1b0000000020: 68 65 20 6C 61 7A 79 20 │ 64 6F 67 27 73 20 62 61  he lazy dog's ba\-
 \1b0000000030: 63 6B 2E 0D 0A          │                          ck.♪◙           \-
@+

 Code page 65001 (UTF-8)

@-
 \1b0000000035: D0 92 20 D1 87 D0 B0 D1 │ 89 D0 B0 D1 85 20 D1 8E  В› ч›а›щ›а›х› ю›\-
 \1b0000000045: D0 B3 D0 B0 20 D0 B6 D0 │ B8 D0 BB 2D D0 B1 D1 8B  г›а› ж›и›л›-б›ы›\-
 \1b0000000055: D0 BB 20 D1 86 D0 B8 D1 │ 82 D1 80 D1 83 D1 81 2C  л› ц›и›т›р›у›с›,\-
@+

 Code page 1200 (UTF-16)

@-
 \1b00000000A2: 3D 04 3E 04 20 00 44 04 │ 30 04 3B 04 4C 04 48 04  но фальш\-
 \1b00000000B2: 38 04 32 04 4B 04 39 04 │ 20 00 4D 04 3A 04 37 04  ивый экз\-
 \1b00000000C2: 35 04 3C 04 3F 04 3B 04 │ 4F 04 40 04 2C 00 20 00  емпляр, \-
 \1b00000000D2: 34 04 30 04 2E 00 0D 00 │ 0A 00                    да.♪◙   \-
@+


@ViewerGotoPos
$ #Viewer: go to specified position#
 This dialog allows to change the position in the internal viewer.

 You can enter an absolute or relative value or percentage, in decimal or hexadecimal.
 For relative add #+# or #-# before the value.
 For percentage add #%# after the value.
 For decimal either add #m# after the value or uncheck the #Hex value#.
 For hexadecimal either add #0x# or #$# before the value, #h# after the value, or check the #Hex value#.

 The value will be interpreted as an offset from the beginning of the file.
If the current view mode is #unwrapped text# it is possible to enter an additional value
which will be interpreted as a first visible column.
Values must be delimited by space or one of the following characters: #,.;:#.
If a value is omitted the corresponding parameter will not be changed.


@ViewerSearch
$ #Viewer: search#
 For searching in the ~viewer~@Viewer@, the following modes and options are
available:

 #Search for text#
 Search for any text entered in the #Search for# edit line.

 #Search for hex#
 Search for a string corresponding to hexadecimal codes entered in the #Search for# string.

 #Case sensitive#
 The case of the characters entered will be taken into account
while searching (so, for example, #Text# will not be found when searching for #text#).

 #Whole words#
 The given text will be found only if it occurs in the text as a whole word.

 #Fuzzy search#
 The search will be diacritical insensitive (for example, #deja# will be found in #déjà vu#),
ligatures will be equivalent to corresponding multicharacter sequences (#fluffy# matches #ﬂuﬀy#),
fancy numbers to corresponding numbers (#42# matches #④②#), and so on.

 Note that case sensitive fuzzy search sometimes may be useful. For example, #Uber# will be found
in #Überwald# but not in #überwald#. However, #Æther# will match #AEther#, but not #Aether#.

 #Regular expressions#
 Enable the use of ~regular expressions~@RegExp@ in the search string.
The multiline search is not supported.

 The #Find next# button starts searching forward.

 The #Find previous# button starts searching backwards.


@Editor
$ #Editor#
 To edit the file currently under the cursor you should press #F4#. This
can be used to open the internal editor or any of the user defined external
editors which are defined in the ~Editor settings~@EditorSettings@ dialog.

 #Creating files using the editor#

 If a nonexistent file name is entered after pressing the #Shift+F4# hotkey
then a ~new file~@FileOpenCreate@ will be created.

 Remarks:

 1. ^<wrap>If a name of a nonexistent folder is entered when creating a new file
then a "~Path to the file to edit does not exist~@WarnEditorPath@" warning will be shown.
 2. ^<wrap>When trying to reload a file already opened in the editor the
"~reloading a file~@EditorReload@" warning message will be shown.
 3. ^<wrap>The ANSI code page is used by default when creating new files, this
behavior can be changed in the ~Editor settings~@EditorSettings@ dialog.

 #Control keys#

 Pohyb kurzora:

 #Doľava#                  ^<wrap>Znak vľavo
 #Ctrl+S#                  Move the cursor one character to the left, but don't move to the previous line if the line beginning is reached.
 #Doprava#                 Znak vpravo
 #Nahor#                   Riadok vyššie
 #Nadol#                   Riadok nižšie
 #Ctrl+Doľava#             Slovo vľavo
 #Ctrl+Doprava#            Slovo vpravo
 #Ctrl+Nahor#              Riadok vyššie skrolovaním obrazovky
 #Ctrl+Nadol#              Riadok nižšie skrolovaním obrazovky
 #PgUp#                    Stranu vyššie
 #PgDn#                    Stranu nižšie
 #Home#                    Začiatok riadku
 #End#                     Koniec riadku
 #Ctrl+Home, Ctrl+PgUp#    Začiatok súboru
 #Ctrl+End, Ctrl+PgDn#     Koniec súboru
 #Ctrl+N#                  Začiatok obrazovky
 #Ctrl+E#                  Koniec obrazovky

 Operácie vymazávania

 #Del#                     ^<wrap>Vymaže znak (taktiež môže vymazať blok, podľa ~nastavenia editoru~@EditorSettings@).
 #BS#                      Vymaže znak naľavo
 #Ctrl+Y#                  Vymaže riadok
 #Ctrl+K#                  Vymaže znaky po koniec riadku
 #Ctrl+BS#                 Vymaže slovo naľavo
 #Ctrl+T, Ctrl+Del#        Vymaže slovo napravo

 Operácie s blokmi

 #Shift+Kurzorové klávesy# Označí blok
 #Ctrl+Shift+Kur. klávesy# Označí blok
 #Alt+Sivé kurzorové kl.#  Označí vertikálny blok
 #Alt+Shift+Kurz. klávesy# Označí vertikálny blok
 #Ctrl+Alt+gray keys#      Označí vertikálny blok
 #Ctrl+A#                  Označí celý text
 #Ctrl+U#                  Odznačí blok
 #Shift+Ins, Ctrl+V#       Vloží blok zo schránky
 #Shift+Del, Ctrl+X#       Vymaže blok (presunie do schránky)
 #Ctrl+Ins, Ctrl+C#        Skopíruje blok do schránky
 #Ctrl+<Sivý +>#           Pridá blok do schránky
 #Ctrl+D#                  Vymaže blok
 #Ctrl+P#                  Skopíruje blok na pozíciu kurzora (len v móde trvalých blokov)
 #Ctrl+M#                  Presunie blok na pozíciu kurzora (len v móde trvalých blokov)
 #Alt+U#                   Posunie blok doľava
 #Alt+I#                   Posunie blok doprava

 Iné operácie

 #F1#                      ^<wrap>Nápoveda
 #F2#                      Uloženie súboru
 #Shift+F2#                Uloženie súboru ~pod iným menom~@FileSaveAs@
 #Shift+F4#                Edit ~new file~@FileOpenCreate@
 #F6#                      Prepnutie do ~prezerača~@Viewer@
 #F7#                      ~Hľadanie~@EditorSearch@
 #Ctrl+F7#                 ~Nahradzovanie~@EditorSearch@
 #Shift+F7#                Continue searching or replacing forward
 #Alt+F7#                  Continue searching or replacing backwards
 #F8#                      Prepínanie medzi DOS/Windows módom
 #Shift+F8#                Výber užívateľskej tabuľky znakov (pozri poznámky nižšie)
 #Alt+F8#                  ~Presun na~@EditorGotoPos@ zadaný riadok a stĺpec
 #Alt+F9#                  Maximize or restore the size of the Far console window; see also ~Interface.AltF9~@Interface.AltF9@
 #Alt+Shift+F9#            Call ~Editor settings~@EditorSettings@ dialog
 #F10, F4, Esc#            Koniec
 #Shift+F10#               Uložiť súbor a koniec
 #Ctrl+F10#                Position to the current file
 #F11#                     Zobrazí menu "~Príkazy prídavných modulov~@Plugins@"
 #Alt+F11#                 Display ~file view and edit history~@HistoryViews@
 #Alt+BS, Ctrl+Z#          Odvolanie poslednej operácie
 #Ctrl+Shift+Z#            Redo
 #Ctrl+L#                  Zákaz modifikácie editovaného textu
 #Ctrl+O#                  Zobrazenie užívateľskej obrazovky
 #Ctrl+Alt+Shift#          Temporarily show user screen (as long as these keys are held down)
 #Ctrl+Q#                  Považuj nasledovnú kombináciu kláves za kód znaku
 #RightCtrl+0…9#           Set a bookmark 0…9 at the current position
 #Ctrl+Shift+0…9#          Set a bookmark 0…9 at the current position
 #LeftCtrl+0…9#            Go to bookmark 0…9
 #Shift+Enter#             Insert the name of the current file on the active panel at the cursor position.
 #Ctrl+Shift+Enter#        Insert the name of the current file on the passive panel at the cursor position.
 #Ctrl+F#                  Insert the full name of the file being edited at the cursor position.
 #Ctrl+B#                  Show/Hide functional key bar at the bottom line.
 #Ctrl+Shift+B#            Show/Hide status line

 See also the list of ~macro keys~@KeyMacroEditList@, available in the editor.

 Poznámky:

 1. ^<wrap>#Alt+U#/#Alt+I# indent the current line if no block is selected.
 2. ^<wrap>Holding down #Alt# and typing a character code on the numeric
keypad inserts the character that has the specified code (0-65535).
 3. ^<wrap>If no block is selected, #Ctrl+Ins#/#Ctrl+C# marks the current
line as a block and copies it to the clipboard.


@EditorSearch
$ #Editor: search/replace#
 The following options are available for search and replace in the ~editor~@Editor@:

 #Case sensitive#
 The case of the characters entered will be taken into account while searching (so, for example,
#Text# will not be found when searching for #text#).

 #Whole words#
 The given text will be found only if it occurs in the text as a whole word.

 #Fuzzy search#
 The search will be diacritical insensitive (for example, #deja# will be found in #déjà vu#),
ligatures will be equivalent to corresponding multicharacter sequences (#fluffy# matches #ﬂuﬀy#),
fancy numbers to corresponding numbers (#42# matches #④②#), and so on.

 Note that case sensitive fuzzy search sometimes may be useful. For example, #Uber# will be found
in #Überwald# but not in #überwald#. However, #Æther# will match #AEther#, but not #Aether#.

 #Regular expressions#
 Treat input as Perl regular expression (~search~@RegExp@ and ~replace~@RegExpRepl@).
Each line is processed individually, so multi-line expressions and line break characters will not be found.

 ~Preserve style~@PreserveStyle@
 Preserve style (case and delimiters in program source code) of the replaced text.

 The #Find next# / #Replace next# buttons start searching / replacing forward.

 The #Find previous# / #Replace previous# buttons start searching / replacing backwards.

 The #All# button will show All matching entries ~menu~@FindAllMenu@.


@PreserveStyle
$ #Editor: Replace mode - Preserve style#
 The #“Preserve style”# ~replace~@EditorSearch@ mode in the
~Editor~@Editor@ preserves the style (case, delimiters) of the replaced
text. This mode may be useful when editing program source code. Some
examples are below. Note how the style of the replaced strings
is preserved in each case.

@-
 ┌────────────────┬────────────────────┬──────────────────────┐
 │ Find / Replace │ Before             │ After                │
 ├────────────────┼────────────────────┼──────────────────────┤
 │ tu / to        │ #Tu# be or not #tu# be │ #To# be or not #to# be   │
 ├────────────────┼────────────────────┼──────────────────────┤
 │ UserName       │ writerUserName     │ writerPersonLogin    │
 │  /             │ user.NAME          │ person.LOGIN         │
 │ PersonLogin    │ DEFAULT_USER_NAME  │ DEFAULT_PERSON_LOGIN │
 │                │ default-User-name  │ default-Person-login │
 └────────────────┴────────────────────┴──────────────────────┘
@+

 #More formally.#

 The main operation used in the algorithm is parsing a string into
tokens. The tokens are divided at a single separator character
or between a lowercase and an uppercase letter. Token separator
characters are #underscore “_”#, #hyphen “-”#, and #dot “.”#. All tokens
must be divided with the same separator. If the parse is ambiguous, the
entire string is treated as a single token. For example:

@-
 ┌──────────────────────┬──────────────────────┬──────────────┐
 │ Search Pattern       │ Tokens               │ Comments     │
 ├──────────────────────┼──────────────────────┼──────────────┤
 │ testMe               │ test Me              │              │
 │ WhatIsIt             │ What Is It           │              │
 │ far-manager          │ far manager          │              │
 │ Contact.Address.Type │ Contact Address Type │              │
 │ USER_FIRST_NAME      │ USER FIRST NAME      │              │
 ├──────────────────────┼──────────────────────┼──────────────┤
 │ test_userName        │ test_userName        │ Ambiguous    │
 │ one.two-three        │ one.two-three        │ separators   │
 │ aBc.dEf.gHi          │ aBc.dEf.gHi          │              │
 ├──────────────────────┼──────────────────────┼──────────────┤
 │ A..B                 │ A..B                 │ Adjacent     │
 │                      │                      │ separators   │
 └──────────────────────┴──────────────────────┴──────────────┘
@+

 The parse also defines the common separator type and the style of
each token. There are three token styles: #Title#case, #lower#case, and
#UPPER#case. If a token has a mix of uppercase and lowercase letters
or non-letter characters, its style is undefined. A token consisting
of a single uppercase character is deemed to be both #Title#case and
#UPPER#case.

 #The following algorithm is used to replace preserving style.#

 The search pattern and the replace string are parsed into tokens
according to the rules above. The text is searched for a string that can
be parsed into the same tokens as the search pattern. The tokens are
compared according to the #“Case sensitive”# and #“Whole words”# search
modes.

 If the found string and the replace string have the same number
of tokens and the found tokens have common style (#Title#case
is preferred over the #UPPER#case), the replace tokens are transformed
to this common style. If the common style cannot be defined, the replace
tokens are transformed to the style of the corresponding found tokens.
After the transformation, the replace tokens are joined with the
separator of the parse of the found string. The result is used as the
replace string. Some examples:

@-
 ┌────────────────┬────────────────────┬──────────────────────┐
 │ Find / Replace │ Before             │ After                │
 ├────────────────┼────────────────────┼──────────────────────┤
 │ abc-def-ghi    │ AbcDefGhi          │ PqRstXyz             │
 │  /             │ ABC_DEF_GHI        │ PQ_RST_XYZ           │
 │ pq.RST.Xyz     │ abc.def.ghi        │ pq.rst.xyz           │
 │                │ abcDefGhi          │ pqRstXyz             │
 │                │ ABC_Def_Ghi        │ PQ_Rst_Xyz           │
 ├────────────────┼────────────────────┼──────────────────────┤
 │ AA-B-C         │ Aa_B_C             │ Xxx_Yy_Zz            │
 │  /             │ aa-b-c             │ xxx-yy-zz            │
 │ xxx.Yy.ZZ      │ AA_B_C             │ XXX_YY_ZZ            │
 │                │ aa.B.C             │ xxx.Yy.Zz            │
 │                │ Aa.B.c             │ Xxx.Yy.zz            │
 └────────────────┴────────────────────┴──────────────────────┘
@+

 If the found string and the replace pattern have different number
of tokens, the first token is processed separately from the rest of the
tokens. The first replace token inherits the style of the first found
token. The rest of the replace tokens are transformed to the common
style of the rest of the found tokens. If the common style cannot
be defined, the rest of the replace tokens are not changed. As in the
previous case, the replace tokens are joined with the separator of the
parse of the found string and the result is used as the replace string.
Examples:

@-
 ┌────────────────┬────────────────────┬──────────────────────┐
 │ Find / Replace │ Before             │ After                │
 ├────────────────┼────────────────────┼──────────────────────┤
 │ abc-def-ghi    │ Abc.def.ghi        │ Pq.rst.uvw.xyz       │
 │  /             │ ABC.Def.Ghi        │ PQ.Rst.Uvw.Xyz       │
 │ pq.RST.uvw.Xyz │ abc.Def.ghi        │ pq.RST.uvw.Xyz       │
 │                │ ABC.DEF.ghi        │ PQ.RST.uvw.Xyz       │
 ├────────────────┼────────────────────┼──────────────────────┤
 │ A-B-C          │ A_B_C              │ Aa_Bb_Cc_Dd          │
 │  /             │ a-b-c              │ aa-bb-cc-dd          │
 │ aa.Bb.cc.DD    │ A.B.c              │ Aa.Bb.cc.DD          │
 └────────────────┴────────────────────┴──────────────────────┘
@+

 In the special case when the found string consists of a single token
but the replace string has several tokens, the first replace token
inherits the style of the found token. The common style for the rest
of the replace tokens and the separator type are deduced from the
context of the found string. If this is not possible, the common style
is the style of the (single) found token and the separator is empty.
Again, the transformed replace tokens are joined and used as the replace
string. More examples:

@-
 ┌────────────────┬────────────────────┬──────────────────────┐
 │ Find / Replace │ Before             │ After                │
 ├────────────────┼────────────────────┼──────────────────────┤
 │ ijk            │ ijk.Zzz            │ mno.Pqr.Stu.Zzz      │
 │  /             │ AAA-ijk            │ AAA-mno-pqr-stu      │
 │ MnoPqrStu      │ aaa-ijk_ZZZ        │ aaa-mno_PQR_STU_ZZZ  │
 │                │ AaaIjk             │ AaaMnoPqrStu         │
 │                │ 0_ijk_9            │ 0_mno_Pqr_Stu_9      │
 │                │ >ijk<              │ >mnopqrstu<          │
 └────────────────┴────────────────────┴──────────────────────┘
@+

 If the search pattern is not found according to the rules above but
found as an ordinary string, and both the found string and the replace
string start with letters, the case of the first letter of the replace
string is changed to that of the found string. For example:

@-
 ┌────────────────┬────────────────────┬──────────────────────┐
 │ Find / Replace │ Before             │ After                │
 ├────────────────┼────────────────────┼──────────────────────┤
 │ ab.cd / wx-yz  │ #A#b.cD              │ #W#x-yz                │
 └────────────────┴────────────────────┴──────────────────────┘
@+


@FindAllMenu
$ #Editor: All matching entries menu#
 The following key combinations are available in this menu:

 #F5#
 Toggle menu size.

 #Ctrl+Up#, #Ctrl+Down#
 Scroll the text in the editor.

 #Ctrl+Enter#, #Ctrl+Left mouse click#
 Go to the position of the found text.

 #Ctrl+Numpad5#
 Vertically align all found entries.

 #Gray +#
 Add session bookmark with the current position.

 #RightCtrl+0…9#, #Ctrl+Shift+0…9#
 Set bookmark 0…9 at the current position.

 #LeftCtrl+0…9#
 Go to the bookmark 0…9.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@FileOpenCreate
$ #Editor: Open/Create file#
 The #Shift+F4# key combination opens an existing file or creates
a new file.

 If the specified file does not exist, a new file will be created.
The code page for the new file is specified in the #Code page# list.
If #default# is selected in the list, the code page specified in the
~Editor settings~@EditorSettings@ dialog is used.

 The code page for an existing file is defined according to the
#Code page# list selection.

 #Default#
 If the file has already been opened and its
code page was saved (depends on the #Save file position# option of the
~Editor settings~@EditorSettings@ dialog), the saved code page is used.
Otherwise, if the file has the Byte Order Mark, the corresponding
Unicode code page -- UTF-8, UTF-16 (Little endian), or UTF-16 (Big
endian) -- is used. Otherwise, the code page is ~autodetected~@CodePageAuto@.

 #Automatic detection#
 An attempt is made to ~autodetect~@CodePageAuto@ code page based on the file contents.

 #Specific code page#
 The selected code page is used.


@FileSaveAs
$ #Editor: save file as…#
 To save edited file with another name press #Shift+F2# and specify
new name, code page and End of Line characters format.

 If file has been edited in one of the following code pages: UTF-8,
UTF-16 (Little endian) or UTF-16 (Big endian), then if the option #Add signature (BOM)# is on,
the appropriate marker is inserted into the beginning of the file, which
helps applications to identify the code page of this file.

 You can also specify the format of the line break characters:

 #Do not change#
 Do not change the line break characters.

 #Dos/Windows format (CR LF)#
 Line breaks will be represented as a two-character sequence -
Carriage Return and Line Feed (CR LF), as used in Dos/Windows.

 #Unix format (LF)#
 Line breaks will be represented as a single character - Line
Feed (LF), as used in Unix.

 #Mac format (CR)#
 Line breaks will be represented as a single character - Carriage
Return (CR), as used in Mac OS.


@EditorGotoPos
$ #Editor: go to specified line and character#
 This dialog allows to change the position in the internal editor.

 You can enter an absolute or relative value or percentage, in decimal or hexadecimal.
 For relative add #+# or #-# before the value.
 For percentage add #%# after the value.
 For decimal either add #m# after the value or uncheck the #Hex value#.
 For hexadecimal either add #0x# or #$# before the value, #h# after the value, or check the #Hex value#.

 The first value will be interpreted as a row number, the second as a character number.
Values must be delimited by space or one of the following characters: #,.;:#.
If a value is omitted the corresponding parameter will not be changed.


@EditorReload
$ #Editor: reloading a file#
 Far Manager tracks all attempts to repeatedly open for editing a file that
is already being edited. The rules for reloading files are as follows:

 1. ^<wrap>If the file was not changed and the option "Reload edited file" in the
~confirmations~@ConfirmDlg@ dialog is not enabled, Far switches to the open
editor instance without further prompts.

 2. ^<wrap>If the file was changed or the option "Reload edited file" is enabled,
there are three possible options:

 #Current#
 Continue editing the same file

 #New instance#
 The file will be opened for editing in a new
editor instance. In this case, be attentive: the
contents of the file on the disk will correspond
to the contents of the editor instance where the
file was last saved.

 #Reload#
 The current changes are not saved and the
contents of the file on the disk is reloaded into the editor.


@WarnEditorPath
$ #Warning: Path to the file to edit does not exist#
 When opening a new file for ~editing~@Editor@, you have entered the name of
a folder that does not exist. Before saving the file, Far will create the
folder, provided that the path is correct (for example, a path starting with a
non-existing drive letter would not be correct) and that you have enough rights
to create the folder.


@WarnEditorPluginName
$ #Warning: The name of the file to edit cannot be empty#
 To create a new file on a plugin's panel you must specify a
file name.


@WarnEditorSavedEx
$ #Warning: The file was changed by an external program#
 The write date and time of the file on the disk are not the same as
those saved by Far when the file was last accessed. This means that another
program, another user (or even yourself in a different editor instance) changed
the contents of the file on the disk.

 If you press "Save", the file will be overwritten and all changes made by
the external program will be lost.


@CodePagesMenu
$ #Code pages menu#
 This menu allows to select code page in the editor and viewer.

 The menu is divided into several sections:

 #Automatic detection#
 Far will try to ~autodetect~@CodePageAuto@ the code page of the text.

 #System#
 Main single-byte system code pages - ANSI and OEM.

 #Unicode#
 Unicode code pages.

 #Favorites#
 Code pages selected by the user.

 #Other#
 The rest of code pages installed in the system.

 The following key combinations are available in this menu:

 #Ctrl+H#
 Shows or hides the #Other# menu section.

 #Ins#
 Moves the code page from the #Other# section to the #Favorites# section.

 #Del#
 Moves the code page from the #Favorites# section back to the #Other# section.

 #F4#
 Opens the ~Rename the code page~@EditCodePageNameDlg@ dialog. Only #Favorites# and
#Other# code pages can be renamed. The renamed code pages are indicated
with the #*# character.

 See also common ~menu keyboard commands~@MenuCmd@.


@EditCodePageNameDlg
$ #Rename code page#
 This dialog allows to rename the #Favorites# and #Other# code pages.
Far will display new code page names in the ~Code pages~@CodePagesMenu@ menu.

 The #Reset# button sets the code page name to the default system
name. Another way to reset the name is to leave it empty and press #OK#.


@DriveDlg
$ #Zmena jednotky#
 Toto menu umožňuje zmeniť aktuálnu jednotku v paneli, odpojiť sieťovú jednotku alebo otvoriť nový panel ~prídavného modulu~@Plugins@.

 Zvolením položky so zodpovedajúcim písmenom jednotky zmení aktuálnu jednotku,
zvolenie položky s názvom modulu vytvorí nový panel tohoto modulu.
Ak typ panelu nebol ~súborový panel~@FilePanel@, zmení sa naň.

 #Ctrl+A#, #F4# hotkeys invoke the ~file attributes~@FileAttrDlg@ for drives.

 #Ctrl+A#, #F4# hotkeys can be used to assign a hotkey to plugin item.

 #F3# key shows plugin technical information.

 Klávesom #Del# možno:
 - ^<wrap>~odpojiť~@DisconnectDrive@ sieťové jednotky;
 - to delete a substituted disk;
 - to detach a virtual disk;
 - to eject disks from CD-ROM and removable drives.

 Ejecting a disk from a ZIP-drive requires administrative privileges.

 A CD-ROM can be closed by pressing #Ins#.

 The #Shift+Del# hotkey is used to prepare a USB storage device for safe
removal. If the disk, for which the removal function is used, is a flash-card
inserted into a card-reader that supports several flash-cards then the
card-reader itself will be stopped.

 #Ctrl+1# - #Ctrl+9# prepínajú zobrazenie rôznych informácií:

 #Ctrl+1# - ^<wrap>typ disku;
 #Ctrl+2# - ^<wrap>sieťový názov / a cestu, priradenú pomocou SUBST
/ path to virtual disk container;
 #Ctrl+3# - ^<wrap>názov disku;
 #Ctrl+4# - ^<wrap>súborový systém;
 #Ctrl+5# - ^<wrap>veľkosť celková a voľného miesta (this option has two
display modes, press twice to see);
 #Ctrl+6# - ^<wrap>zobrazovanie parametrov vynímateľných diskov;
 #Ctrl+7# - ^<wrap>zobrazovanie položiek modulov;
 #Ctrl+8# - ^<wrap>zobrazovanie parametrov CD;
 #Ctrl+9# - ^<wrap>network parameters.

 #Zmena jednotky# nastavenia sa ukladajú do konfigurácie Faru.

 #F9# shows the ~dialog~@ChangeDriveMode@ to control displaying
of this information.

 If the option "~Use Ctrl+PgUp to change drive~@InterfSettings@" is enabled,
pressing #Ctrl+PgUp# works the same as pressing #Esc# - cancels drive selection
and closes the menu.

 Pressing #Shift+Enter# invokes the Windows Explorer showing the root
directory of the selected drives (works only for disk drives and not for
plugins).

 #Ctrl+H# shows unmapped volumes.

 #Ctrl+R# allows to refresh the disk selection menu.

 If "#CD drive type#" mode is enabled (#Ctrl+8#), Far will attempt to
determine the type of each of the CD drives available in the system. Known
types are as follows: CD-ROM, CD-RW, CD-RW/DVD, DVD-ROM, DVD-RW and DVD-RAM.
This function is available only for users either with administrative privileges
or all local users, when it's stated explicitly in the Local Policy Editor
(to do this, run #secpol.msc# from the command prompt, and set the '#Local#
#Policies/Security Options/Devices: Restrict CD-ROM access to locally logged-on#
#user only#' setting to '#Enabled#').

 #Alt+Shift+F9# allows you to ~configure plugins~@PluginsConfig@ (it works only if
display of plugin items is enabled).

 #Shift+F9# in the plugins list opens the configuration dialog of the
currently selected plugin.

 #Shift+F1# in the plugins list displays the context-sensitive help of the
currently selected plugin, if the plugin has a help file.

 The #A# character in the leftmost menu column means that the corresponding plugin is
written for Far 1.7x and it does not support all possibilities available in
Far 3 (these are, in particular, Unicode characters in filenames and in editor).

 See also:

 The list of ~macro keys~@KeyMacroDisksList@, available in the disk menu.
 Common ~menu~@MenuCmd@ keyboard commands.


@ChangeDriveMode
$ #Change Drive Menu Options#
 The dialog allows to control the information shown in the
~Change drive~@DriveDlg@ menu.

 #Show disk type#
 Show disk type: “fixed”, “network”, etc.
 Key combination in #Change drive# menu: #Ctrl+1#.

 #Show disk label#
 Show disk label (if available).
 Key combination in #Change drive# menu: #Ctrl+3#.

 #Use shell name#
 Request the disk name from Windows Shell.
 Key combination in #Change drive# menu: #Ctrl+3# (press twice).

 #Show file system type#
 Show file system type: “NTFS”, “FAT”, etc.
 Key combination in #Change drive# menu: #Ctrl+4#.

 #Show size#
 Show disk size and free space.
 Key combination in #Change drive# menu: #Ctrl+5#.

 #Show size as a decimal fraction#
 Show disk size and free space as a decimal fraction with no more
than three digits before decimal point. If this option is on, 1 GiB (2
to the power of 30) will be shown as #1.00 G#, otherwise as #1024 M#.
 Key combination in #Change drive# menu: #Ctrl+5# (press twice).

 #Show network name / SUBST path / VHD name#
 Show network name / path associated with a SUBST disk / path
to virtual disk container.
 Key combination in #Change drive# menu: #Ctrl+2#.

 #Show plugins#
 Show plugins.
 Key combination in #Change drive# menu: #Ctrl+7#.

 #Sort plugins by hotkey#
 If this option is turned #off#, plugin list is sorted by name;
otherwise by hotkey.

 #Show removable drive parameters#
 Show removable drive parameters.
 Key combination in #Change drive# menu: #Ctrl+6#.

 #Show CD drive parameters#
 If this option is turned on, Far will attempt to detect CD drive
type: CD-ROM, CD-RW, CD-RW/DVD, DVD-ROM, DVD-RW, DVD-RAM, HD DVD-ROM,
HD DVD-RW, Blue-ray Disk-ROM, Blue-ray Disk-RW.
 Key combination in #Change drive# menu: #Ctrl+8#.

 #Show network drive parameters#
 Show network drive size and free space. Display format depends
on the #Show size# option.
 Key combination in #Change drive# menu: #Ctrl+9#.

 #Detect virtual disks#
 Detect virtual disks (VHD, VHDX). This could wake up sleeping hard drives.


@DisconnectDrive
$ #Disconnect network drive#
 You can disconnect a network drive by pressing #Del# in the
~Change Drive menu~@DriveDlg@.

 The option #[x] Reconnect at logon# is enabled only for permanently
connected network drives.

 The confirmation can be disabled in the ~confirmations~@ConfirmDlg@ dialog.


@Highlight
$ #Files highlighting and sort groups#
 For more convenient and obvious display of files and directories in the
panels, Far Manager has the possibility of color highlighting for file objects.
You can group file objects by different criteria (~file masks~@FileMasks@, file
attributes) and assign colors to those groups.

 File highlighting can be enabled or disabled in the ~panel settings~@PanelSettings@
dialog (menu item Options | Panel settings).

 You can ~edit~@HighlightEdit@ the parameters of any highlight group through
the "~Options~@OptMenu@" menu (item "Files highlighting and sort groups").


@HighlightList
$ #Files highlighting and sort groups: control keys#
 The ~file highlighting and sort groups~@Highlight@ menu allows you to
perform various operations with the list of the groups. The following key
combinations are available:

 #Ins#          - ^<wrap>Add a new highlighting group

 #F5#           - ^<wrap>Duplicate the current group

 #Del#          - ^<wrap>Delete the current group

 #Enter# or #F4#  - ^<wrap>~Edit~@HighlightEdit@ the current highlighting group

 #Ctrl+R#       - ^<wrap>Restore the default file highlighting groups

 #Ctrl+Up#      - ^<wrap>Move a group up.

 #Ctrl+Down#    - ^<wrap>Move a group down.

 Zvýrazňované skupiny sú analyzované od začiatku. Ak súbor vyhovuje
nejakej skupine, nebude už testovaný, či nepatrí do nejakej zo zvyšných,
unless #[x] Continue processing# is set in the group.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@HighlightEdit
$ #Zvýrazňovanie súborov#
 #Zvýrazňovanie súborov# z menu ~Nastavenia~@OptMenu@ dovoľuje
definovanie skupín súborov pre zvýrazňovanie. Definícia každej skupiny ~zahŕňa~@Filter@:

 - jednu alebo viac ~masiek~@FileMasks@;

 - atribúty "vrátane" alebo "okrem";
   #[x]# - ^<wrap>inclusion attribute - file musí mať this attribute.
   #[ ]# - ^<wrap>exclusion attribute - file nesmie mať this attribute.
   #[?]# - ^<wrap>ignore this attribute;

 - ^<wrap>farbu použitú na zobrazenie mena súboru normálneho, označeného,
pod kurzorom a označeného pod kurzorom. Ak chcete použiť štandardné
farby, nastavte "čiernu na čiernej";

 - ^<wrap>označenie pre súbory zo skupiny.
Môžu byť použité tak s farebným zvýraznením ako i namiesto neho.

 Súbor patrí do zvýrazňovanej skupiny ak:
 - ^<wrap>jeho meno zodpovedá aspoň jednej
zo súborových masiek (if file mask analysis is disabled,
the file name doesn't matter);
 - má všetky atribúty "vrátane";
 - nemá žiadne atribúty "okrem".

 The Compressed, Encrypted, Not indexed, Sparse, Temporary and Reparse point
attributes are valid for NTFS drives only. The #Integrity stream# and
#No scrub data# attributes are supported only on ReFS volumes starting from
Windows Server 2012.


@ViewerSettings
$ #Nastavenie prezerača#
 Toto okno dovoľuje zmeniť základné nastavenie interneho a externého ~prezerača~@Viewer@.

@=
^#Prezerač#
@=
 #Použiť F3 namiesto Alt+F3#
 Spusti externý prezerač cez #F3# a interný prezerač cez #Alt+F3#.

 #Príkaz pre zobrazenie#
 Príkaz, ktorý spustí externý viewer.
Na špecifikáciu mena súboru, ktorý sa má prezrieť použite ~špeciálne symboly~@MetaSymbols@.

@=
^#Interný prezerač#
@=
 #Persistent selection#
 Do not remove block selection after moving the cursor.

 #Veľkosť tabulátora#
 Počet medzier reprezentujúcich znak tabulátora.

 #Show scrolling arrows#
 Show scrolling arrows at the edges of the viewer window if the text does not fit horizontally.

 #Visible '\0'#
 Show a printable character instead of space for the character '\0'.
The character to display can be set in ~far:config~@FarConfig@ #Viewer.ZeroChar#.

 #Show a scrollbar#
 Show a scrollbar in the internal viewer. This option can also be toggled by pressing #Ctrl+S# in the internal viewer.

@=
 #Uložiť pozíciu v súbore#
 Uloží pozíciu a vráti sa na ňu v naposledy  prezeraných súboroch. This option also saves and restores
the code page (if it was selected manually) and ~view mode~@ViewerMode@.

 #Save file code page#
 Save and restore the code page selected for a file. This is automatically enabled if #Save file position#
is enabled, as file position depends on the encoding.

 #Save bookmarks#
 Save and restore bookmarks in the recently viewed files (bookmarks can be created with #RightCtrl+0…9#
or #Ctrl+Shift+0…9# key combinations.)

 #Maximum line width#
 Maximum number of columns for text mode viewer. Min=100, Max=100,000, Default=10,000.

 #Save view mode#
 Save and restore ~view modes~@ViewerMode@ of recently viewed files.

 #Save wrap mode#
 Save and restore #wrap# and #word wrap# ~modes~@ViewerMode@ of recently viewed files.

 #Detect dump view mode#
 If this option is on and Far considers the file binary, the #dump# ~mode~@ViewerMode@ is selected automatically
at the first view. Otherwise, the #text# mode is selected.

 #Autodetect code page#
 ~Autodetect~@CodePageAuto@ the code page of the file being viewed.

 #Default code page#
 Allows to select the default code page.

@=
 Ak je externý prezerač priradený klávesu #F3#, bude spustený len
ak danému ~typu súboru~@FileAssoc@ nie je priradný vlastný prezerač.

 Changing of settings does not affect currently opened internal viewer windows.

 The settings dialog can also be invoked from the ~internal viewer~@Viewer@
by pressing #Alt+Shift+F9#. The changes will come into effect immediately but
will affect only the current session.


@EditorSettings
$ #Nastavenie editora#
 Toto okno dovoľuje zmeniť základné nastavenie externého a ~interného editora~@Editor@.

@=
^#Externý editor#
@=
 #Použiť F4#
 Spustí externý editor po stlačení #F4# namiesto #Alt+F4#.

 #Príkaz pre editovanie#
 Príkaz, ktorý spustí externý editor.
Na špecifikáciu mena súboru, ktorý sa má editovať použite ~špeciálne symboly~@MetaSymbols@.

@=
^#Interný editor#
@=
 #Do not expand tabs#
 Do not convert tabs to spaces while editing the document.

 #Expand newly entered tabs to spaces#
 While editing the document, convert each newly entered #Tab# into the appropriate number of spaces.
Other tabs won't be converted.

 #Expand all tabs to spaces#
 Upon opening the document, all tabs in the document will be automatically converted to spaces.

 #Trvalé bloky#
 Neodstráni označenie bloku pri pohybe kurzora.

 #Del odstraňuje bloky#
 Ak je označený blok, kláves Del neodstráni iba znak pod kurzorom, ale celý tento blok.

 #Automatické odsadenie#
 Zapne mód automatického odsadzovania pri zadávaní textu.

 #Veľkosť tabulátora#
 Počet medzier reprezentujúcich znak tabulátora.

 #Show white space#
 Make while space characters (spaces, tabulations, line breaks) visible.

 #Kurzor za koncom riadku#
 Dovolí presunúť kurzor aj za koniec riadku.

 #Select found#
 Found text is selected.

 #Cursor at the end#
 Place the cursor at the end of the found block.

 #Show a scrollbar#
 Show a scrollbar.

@=
 #Uložiť pozíciu v súbore#
 Uloží pozíciu a vráti sa na ňu v naposledy editovaných súboroch. Ak bola predtým užívateľom
manuálne nastavená tabuľka znakov, táto sa tiež sa tiež uloží a znovu nastaví.

 #Save bookmarks#
 Save and restore bookmarks (current positions) in recently edited files
(created with #RightCtrl+0…9# or #Ctrl+Shift+0…9#)

 #Allow editing files opened for writing#
 Allows to edit files that are opened by other programs for writing. This mode is handy to edit
a file opened for a long time, but it could be dangerous, if a file is being modified at the same time as editing.

 #Lock editing of read-only files#
 When a file with the Read-only attribute is opened for editing, the editor also
disables the modification of the edited text, just as if #Ctrl+L# was pressed.

 #Warn when opening read-only files#
 When a file with the Read-only attribute is opened for editing, a warning message will be shown.

 #Autodetect code page#
 ~Autodetect~@CodePageAuto@ the code page of the file being edited.

 #Default code page#
 Select the default code page.

 Ak je externý editor priradený klávesu #F4#, bude spustený len
ak danému ~typu súboru~@FileAssoc@ nie je priradný vlastný editor.

 Modifications of settings in this dialog do not affect previously opened internal editor windows.

 The settings dialog can also be invoked from the ~internal editor~@Editor@
by pressing #Alt+Shift+F9#. The changes will come into force immediately but
will affect only the current session.


@CodePageAuto
$ #Autodetect code pages#
 Far will try to choose the correct code page for viewing/editing a file.
Note that correct detection is not guaranteed, especially for small or
non-typical text files.

 See also the ~Code pages~@CodePagesMenu@ menu and
~far:config Codepages.NoAutoDetectCP~@Codepages.NoAutoDetectCP@


@FileAttrDlg
$ #Atribúty súborov#
 S týmto príkazom je možné zmeniť atribúty súborov a ich čas.
Možno pracovať s jedným ako i skupinou súborov.

 #File attributes#

 Checkboxes used in the dialog can have the following 3 states:

 #[x]# - attribute is set for all selected items
       (set the attribute for all items)

 #[ ]# - attribute is not set for all selected items
       (clear the attribute for all items)

 #[?]# - attribute state is not the same for selected items
       (don't change the attribute)

 When all selected files have the same attribute value, the corresponding
checkbox will be in 2-state mode - set/clear only. When there are selected
folders, all checkboxes will always be 3-state.

 Only those attributes will be changed for which the state of the
corresponding checkboxes was changed from the initial state.

 Atribúty #Komprimovaný#, #Encrypted#, #Not indexed#, #Sparse#, #Temporary#,
#Offline#, #Reparse point# možno zmeniť len na jednotkách NTFS.
The #Compressed# and #Encrypted# attributes are mutually exclusive, that is, you can set only
one of them. You cannot clear the #Sparse# attribute in Windows 2000/XP/2003. The
#Integrity stream# and #No scrub data# attributes are supported only on ReFS volumes starting from
Windows Server 2012.

 For ~symbolic links~@HardSymLink@ the dialog will display the path where it refers to.
If this information is not available, then the "#(data not available)#" message will be shown.

 #File date and time#

 Podporované sú štyri rôzne časy súboru:
 - čas poslednej zmeny;
 - čas vytvorenia;
 - čas posledného prístupu;
 - change time.

 Na jednotkách FAT sa hodiny, minúty a sekundy posledného prístupu
vždy rovnajú nule.

 Ak nechcete zmeniť čas súboru, ponechajte príslušné pole prázdne.
You can push the #Blank# button to clear all the date and time fields
and then change an individual component of the date or time, for example, only
month or only minutes. All the other date and time components will remain
unchanged.

 Tlačítko #Súčasný# vyplní polia času súboru súčasným časom.
 The #Original# button fills the file time fields with their original
values. Available only when the dialog is invoked for a single file object.

 The #System properties# button invoke the system properties dialog for
selected objects.


@FolderShortcuts
$ #Skratky k adresárom#
 Skratky k adresárom boli vytvorené na to, aby umožnili rýchly prístup
k často používaným adresárom. Stlačením #Ctrl+Shift+0…9#,
vytvoríte skratku k aktuálnemu adresáru. Potom na opätovné nastavenie adresára
uloženého v skratke stačí stlačiť #PravýCtrl+0…9#. Ak sa #PravýCtrl+0…9# stlačí
pri zadávaní textu, vloží sa cesta príslušnej skratky do tohto textu.

 Príkazom #Skratky k adresárom# nachádzajúcim sa v menu ~Príkazy (Commands)~@CmdMenu@ otvoríte
okno, v ktorom môžete prezerať, nastavovať, editovať a vymazávať tieto skratky.

 When you are editing a shortcut (#F4#), you cannot create a shortcut to a
plugin panel.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@FiltersMenu
$ #Filters menu#
 Using the #Filters menu# you can define a set of file types with user
defined rules according to which files will be processed in the area of
operation this menu was called from.

 Menu filtra sa skladá z dvoch častí. V hornej časti sú zobrazené
#užívateľské filtre#. Dolná časť menu filtra obsahuje masky všetkých súborov aktívneho
súborového panelu (including file masks that are selected in the
current area of operation the menu was called from even if there are no files
that match those mask in the current panel).

 For the #User filters# the following commands are available:

 #Ins#        Create a new filter, an empty ~filter~@Filter@ settings
            dialog will open for you to set.

 #F4#         Edit an existing ~filter~@Filter@.

 #F5#         Duplicate an existing ~filter~@Filter@.

 #Del#        Remove a filter.

 #Ctrl+Up#    Move a filter one position up.

 #Ctrl+Down#  Move a filter one position down.

 To control the #User filters# and also the auto-generated filters (file
masks) the following commands are available:

 #Space#,              Items selected using #Space# or ‘#+#’ are
 #Plus#                marked by ‘+’. If such items are present
                     then only files that match them will be
                     processed.

 #Minus#               Items selected using ‘#-#’ are marked by ‘-’,
                     and files that match then will not be
                     processed.

 #I# and #X#             Similar to #Plus# and #Minus# respectively,
                     but have higher priority when matching.

 #Backspace#           Clear selection from the current item.

 #Shift+Backspace#     Clear selection from all items.

 Filters selection is stored in the Far configuration.

 When a filter is used in a panel, it is indicated by ‘*’ after the sort
mode letter in the upper left corner of the panel.

 Filters menu is used in the following areas:
 - ~File panel~@FilePanel@;
 - ~Copying, moving, renaming and creating links~@CopyFiles@;
 - ~Find file~@FindFile@.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@FolderDiz
$ #Folder descriptions#
 Specify names (~wildcards~@FileMasks@ are allowed) of files displayed
in the ~Info panel~@InfoPanel@ as folder descriptions.


@FileDiz
$ #Popis súborov#
 Popis súborov možno použiť na priradenie textovej informácie
k súboru. Popisy súborov aktuálneho adresára sú uložené v tomto
adresári v súbore so zoznamom popisov. V tomto súbore je na začiatku
každého riadku názov súboru nasledovaný medzerami a akýmkoľvek textom.

 Popisy si možno prezrieť v príslušných ~módoch~@PanelViewModes@
súborového panelu. Štandardne sú týmito módmi #Popisy#
a #Dlhé popisy#.

 Popis možno k súboru/označeným súborom pridať príkazom
#Popis súborov# (#Ctrl+Z#) z menu ~súbory~@FilesMenu@.

 Mená súborov z popismi môžu byť zmenené pomocou príkazu
#Popisy súborov (File descriptions)# z menu ~Nastavenia~@OptMenu@.
Tu možno nastaviť taktiež mód aktualizácie popisov. Táto môže byť
celkom vypnutá, zapnutá len ak aktuálny panel zobrazuje popisy alebo
zapnutá vždy. Štandardne Far nastavuje súborom s popismi príznak "Skrytý", ale
toto správanie môžete zmeniť pomocou nastavenia "V nových zoznamoch popisov
nastaviť atribút "Skrytý". Tu taktiež môžete určiť pozíciu na zarovnávanie
nových popisov súborov v zozname popisov.

 If a description file has the "read-only" attribute set, Far does not
attempt to update descriptions, and after moving or deleting file objects, an
error message is shown. If the option "#Update read only description file#" is
enabled, Far will attempt to update the descriptions correctly.

 Ak je aktualizácia zapnutá, Far aktualizuje popisy pri
kopírovaní, presúvaní a vymazávaní súborov. Ale ak sa príkaz vykonáva
aj na súboroch z podadresárov, popisy v nich sa neaktualizujú.

 #Use ANSI code page by default#
 By default Far uses the OEM codepage for file descriptions, both for reading and writing.
This option changes it to ANSI.

 #Save in UTF-8#
 If set, the description file will be read as OEM or ANSI, depending on the option above,
but saved in UTF-8 after you add, remove or update the descriptions.

 #Note#: these options are irrelevant when the file has the UTF-8 signature. In this case it is always read and written in UTF-8.


@PanelViewModes
$ #Úprava módov zobrazenia súborov#
~Súborový panel~@FilePanel@ môže zobrazovať informácie
pomocou 10 preddefinovaných módov: stručne, stredne, úplne, široko, detailne,
popisy, dlhé popisy, vlastníci súborov, linky súborov, alternatívne úplne.
To zvyčajne stačí, ale kto chce, ten si môže prispôsobiť ich parametre
alebo ich dokonca nahradiť úplne iným módmi.

 Príkaz #Módy zobrazenia súborov# v menu ~nastavenia~@OptMenu@ umožňuje
zmeniť tieto módy. Najprv je potrebné vybrať želaný mód zo zoznamu.
V tomto zozname "Stručný" zodpovedá stručnému/brief módu
(#ĽavýCtrl+1#), "Stredný" zodpovedá strednému/medium módu (#ĽavýCtrl+2#) a tak
ďalej. Posledné "Alternatívne úplne" zodpovedá módu vyvolanému cez
#ĽavýCtrl+0#. Po tomto výbere sa objaví dialógové okno, kde môžete zmeniť nasledovné nastavenia:

 #Typy stĺpcov# - a comma-separated list. Each column type starts with
a file property character, such as name, size, etc. Some file properties
may be followed by modifiers. Supported column types (properties and
their modifiers) are listed below.

 If the list of column types consists of two or more repeated groups,
the files on the panel will be listed in “stripes”. Properties of each
file will be displayed in the columns of a stripe, and the list of files
will wrap from one stripe to the next like text of a newspaper article.
If column type list cannot be properly split into the equal groups, the
files will be listed on a single stripe.

 The following column types are supported:

 N[M[D],O,R[F],N] - meno súboru, kde
                    M - ^<wrap>zobrazuj zvýrazňujúce značky, kde
                        D - dynamic selection marks;
                    O - ^<wrap>zobrazuj mená bez cesty (určené najmä pre ~moduly/plugins~@Plugins@);
                    R - ^<wrap>mená zarovnané doprava, kde
                        F - right align all names;
                    N - ^<wrap>do not show extensions in name column;

 Tieto parametre možno kombinovať, napr. NMR.

 X[R]       - file extension, where:
              R - ^<wrap>right align file extension;

 S[C,T,F,E] - dĺžka súboru
 P[C,T,F,E] - komprimovaná dĺžka súboru
 G[C,T,F,E] - size of file streams, where:
              C - ^<wrap>formátovanie dĺžky súboru čiarkami;
              T - ^<wrap>používať 1000 namiesto 1024
ako oddeľovač ak je stĺpec príliš úzky
na zobrazenie celej dĺžky; in this mode unit character is shown in lower case, e.g. #k#,
#m#, #g# instead of #K#, #M#, #G#;
              F - ^<wrap>show size as a decimal fraction with
no more than three digits before decimal point, e.g. 999 bytes will
be shown as #999#, while 1024 bytes as #1.00 K#; note that the behavior
depends on whether the #T# modifier is used;
              E - ^<wrap>economic mode, no space between the
size and the unit character, e.g. #1.00k#;

 D         - dátum modifikácie súboru
 T         - čas modifikácie súboru

 DM[B,M]   - dátum a čas modifikácie súboru
 DC[B,M]   - dátum a čas vytvorenia súboru
 DA[B,M]   - dátum a čas posledného prístupu
 DE[B,M]    - file change date and time, where:
              B - krátky (Unix štýl) formát času;
              M - používať názvy mesiacov;

 A         - atribúty súboru;
 Z         - popis súboru;

 O[L]      - majiteľ súboru, kde
              L - show domain name;

 LN        - počet pevných linkov;

 F          - number of streams.

 Ak popis typov stĺpcov obsahuje viac ako jeden stĺpec s menom
súboru, súborový panel bude zobrazený viacstĺpcovo.

 File attributes are denoted as follows:

 #N# - Attributes not set
 #R# - Read only
 #H# - Hidden
 #S# - System
 #D# - Directory
 #A# - Archive
 #T# - Temporary
 #$# - Sparse
 #L# - Reparse point
 #C# - Compressed
 #O# - Offline
 #I# - Not content indexed
 #E# - Encrypted
 #V# - Integrity stream
 #?# - Virtual
 #X# - No scrub data
 #P# - Pinned
 #U# - Unpinned

 By default the size of the attributes column is 6 characters. To display
the additional attributes it is necessary to manually increase the size of the column.

 #Šírky stĺpcov (Column widths)# - dovoľuje zmeniť šírky jednotlivých
stĺpcov panelu. Ak sa rovná nule, použije sa štandardná hodnota. Ak je
šírka názvu, popisu alebo majiteľa rovná nule, bude vypočítaná automaticky,
v závislosti od šírky panelu. Pre správnu prácu s rôznymi šírkami obrazovky
sa veľmi odporúča mať aspoň jeden stĺpec s automaticky vypočítavanou šírkou.
Width can be also set as a percentage of
remaining free space after the fixed width columns by adding the "%" character
after the numerical value. If the total size of such columns exceeds 100%,
their sizes are scaled.

 Zvýšenie štandardnej šírky stĺpca pre čas alebo dátum a čas súboru o 1
spôsobí zobrazenie v 12-hodinovom formáte. Pri ďalšom zväčšení sa zobrazia
sekundy resp. milisekundy.

 Pre zobrazovanie roku vo formáte štyroch čísel stačí zväčšiť šírku stĺpca o dva.

 Enabling links, streams and owner columns (G, LN, F and O) can significantly
slow down the directory reading.

 #Typy stavového riadku# a #Šírky stavového riadku# -
sú podobné "Typom stĺpcov" a "Šírkam stĺpcov", ale platia pre stavový riadok.

 #Celá obrazovka# - nastaví jeden panel na celú obrazovku namiesto zvyčajnej polovičky.

 #Zarovnať prípony súborov# - bude zobrazovať prípony súborov zarovnané pod seba.

 #Align folder extensions# - show folder extensions aligned.

 #Adresáre veľkými písmenami# - zobrazí všetky mená adresárov veľkými písmenami,
ignorujúc originálnu veľkosť písmen.

 #Súbory malými písmenami# - zobrazí všetky mená súborov malými písmenami,
ignorujúc originálnu veľkosť písmen.

 #Súbory veľkými písmenami zobraziť malými# -
zobrazí všetky mená súborov, ktoré obsahujú iba veľké písmená, iba malými
písmenami. Táto možnosť je štandardne zapnutá, kto chce vidieť všetky mená
so skutočnými názvami, ten môže túto, ako i dve predošlé vypnúť. Všetky
tieto nastavenia však ovplyvňujú iba zobrazovanie súborov, pri operáciách
Far vždy používa skutočné mená (vrátane veľkých/malých písmen).

 See also: common ~menu~@MenuCmd@ keyboard commands.


@ColorGroups
$ #Color groups#
 Toto menu dovoľuje zmeniť farbu rôznych častí prostredia, a taktiež nastaviť farby ako východzie (základné).

 #Set default colors#
 Set the colors to default values, expressed as indices in the console palette.

 #Set default colors (RGB)#
 Set the colors to default values, expressed as colors in RGB space, normally used for the corresponding console palette indices.
 Unlike the indices in the console palette, the RGB values are device-independent and will look the same in any terminal.
 For example, the default #index# value of panels background is #1#, which is usually, but not necessarily, mapped to some unspecified shade of blue.
 The default #RGB# value of panels background, on the contrary, is always exactly #000080#.

 #Note#: RGB colors require Virtual Terminal-based rendering, which can be enabled in ~Interface settings~@InterfSettings@.
If it is not enabled or if your terminal does not support RGB colors, they will be approximated to the closest console palette indices.

 This is the current palette:

 \00  \10  \20  \30  \40  \50  \60  \70  \-
 \80  \90  \A0  \B0  \C0  \D0  \E0  \F0  \-

 This is the default RGB representation:

 \(T0:T000000)  \(T0:T000080)  \(T0:T008000)  \(T0:T008080)  \(T0:T800000)  \(T0:T800080)  \(T0:T808000)  \(T0:TC0C0C0)  \-
 \(T0:T808080)  \(T0:T0000FF)  \(T0:T00FF00)  \(T0:T00FFFF)  \(T0:TFF0000)  \(T0:TFF00FF)  \(T0:TFFFF00)  \(T0:TFFFFFF)  \-


@ColorPicker
$ #Color Picker#
 This dialog allows to define a foreground color, a background color and a text style.

 The foreground and the background colors can be either:
 - one of the 16 colors from the standard Windows Console pallete,
 - one of the 256 colors from the Xterm pallette, or
 - one of the 16 million colors from the RGB color space.

 The standard 16-color palette is available in the dialog.
 To access the ~256-color palette~@ColorPicker256@ and the ~RGB color space~@ColorPickerRGB@ use the corresponding buttons.

 #Default# is the color used by your terminal when no colors are specified explicitly, e.g. \(800000:800000) C:\> \-.
 Usually it is one of the palette colors, e.g. \(7:0)silver on black\-, but not necessarily: some terminals could handle it differently, e.g. render as translucent.

 The color value is also represented in the hexadecimal form for convenience, where:
 - #AA______# - the alpha channel, representing the degree of transparency from fully transparent (00) to fully opaque (FF).
 - #______##### - the palette index from 00 to FF.
 - #__RRGGBB# - the red, green and blue channels in the RGB color space, from 00 to FF each.

 When the color is not fully opaque, the previous color in the logical Z-order is taken into account.

 The foreground text style can include ANSI/VT100-like attributes listed in the right section.
 When #Inherit# is checked, the previous foreground text style in the logical Z-order is taken into account.

 Default:   \(7:0) Example \-
 Bold:      \(7:0:bold) Example \-
 Italic:    \(7:0:italic) Example \-
 Overline:  \(7:0:overline) Example \-
 Strikeout: \(7:0:strikeout) Example \-
 Faint:     \(7:0:faint) Example \-
 Blink:     \(7:0:blink) Example \-
 Inverse:   \(7:0:inverse) Example \-
 Invisible: \(7:0:invisible) Example \-
 Underline:
   Single:  \(7:0:underline) Example \-
   Double:  \(7:0:underline_double) Example \-
   Curly:   \(7:0:underline_curly) Example \-
   Dotted:  \(7:0:underline_dot) Example \-
   Dashed:  \(7:0:underline_dash) Example \-

 The preview section below displays the final result.

 #Attention#
 Only the standard 16-color palette is guaranteed to work everywhere.
 Support for everything else is conditional and defined by your terminal.

 Extended colors and styles require Virtual Terminal-based rendering, which can be enabled in ~Interface settings~@InterfSettings@.
You can find more about it ~here~@https://docs.microsoft.com/en-us/windows/console/classic-vs-vt@.


@ColorPicker256
$ #256 Color Picker#
 This dialog allows to pick a color from the 256-color Xterm pallette.

 The first 16 colors are the same as the standard palette and are available in the ~main dialog~@ColorPicker@.

 \00  \10  \20  \30  \40  \50  \60  \70  \-
 \80  \90  \A0  \B0  \C0  \D0  \E0  \F0  \-

 The next 216 colors are represented as a 6x6x6 cube. The palette usually has 6 levels for every primary color and forms a homogeneous RGB cube.
 Use the buttons on the right to rotate the cube, access its inner levels or mix the primary colors directly.

 \(:10)  \(:11)  \(:12)  \(:13)  \(:14)  \(:15)  \-  \(:34)  \(:35)  \(:36)  \(:37)  \(:38)  \(:39)  \-  \(:58)  \(:59)  \(:5A)  \(:5B)  \(:5C)  \(:5D)  \-
 \(:16)  \(:17)  \(:18)  \(:19)  \(:1A)  \(:1B)  \-  \(:3A)  \(:3B)  \(:3C)  \(:3D)  \(:3E)  \(:3F)  \-  \(:5E)  \(:5F)  \(:60)  \(:61)  \(:62)  \(:63)  \-
 \(:1C)  \(:1D)  \(:1E)  \(:1F)  \(:20)  \(:21)  \-  \(:40)  \(:41)  \(:42)  \(:43)  \(:44)  \(:45)  \-  \(:64)  \(:65)  \(:66)  \(:67)  \(:68)  \(:69)  \-
 \(:22)  \(:23)  \(:24)  \(:25)  \(:26)  \(:27)  \-  \(:46)  \(:47)  \(:48)  \(:49)  \(:4A)  \(:4B)  \-  \(:6A)  \(:6B)  \(:6C)  \(:6D)  \(:6E)  \(:6F)  \-
 \(:28)  \(:29)  \(:2A)  \(:2B)  \(:2C)  \(:2D)  \-  \(:4C)  \(:4D)  \(:4E)  \(:4F)  \(:50)  \(:51)  \-  \(:70)  \(:71)  \(:72)  \(:73)  \(:74)  \(:75)  \-
 \(:2E)  \(:2F)  \(:30)  \(:31)  \(:32)  \(:33)  \-  \(:52)  \(:53)  \(:54)  \(:55)  \(:56)  \(:57)  \-  \(:76)  \(:77)  \(:78)  \(:79)  \(:7A)  \(:7B)  \-

 \(:7C)  \(:7D)  \(:7E)  \(:7F)  \(:80)  \(:81)  \-  \(:A0)  \(:A1)  \(:A2)  \(:A3)  \(:A4)  \(:A5)  \-  \(:C4)  \(:C5)  \(:C6)  \(:C7)  \(:C8)  \(:C9)  \-
 \(:82)  \(:83)  \(:84)  \(:85)  \(:86)  \(:87)  \-  \(:A6)  \(:A7)  \(:A8)  \(:A9)  \(:AA)  \(:AB)  \-  \(:CA)  \(:CB)  \(:CC)  \(:CD)  \(:CE)  \(:CF)  \-
 \(:88)  \(:89)  \(:8A)  \(:8B)  \(:8C)  \(:8D)  \-  \(:AC)  \(:AD)  \(:AE)  \(:AF)  \(:B0)  \(:B1)  \-  \(:D0)  \(:D1)  \(:D2)  \(:D3)  \(:D4)  \(:D5)  \-
 \(:8E)  \(:8F)  \(:90)  \(:91)  \(:92)  \(:93)  \-  \(:B2)  \(:B3)  \(:B4)  \(:B5)  \(:B6)  \(:B7)  \-  \(:D6)  \(:D7)  \(:D8)  \(:D9)  \(:DA)  \(:DB)  \-
 \(:94)  \(:95)  \(:96)  \(:97)  \(:98)  \(:99)  \-  \(:B8)  \(:B9)  \(:BA)  \(:BB)  \(:BC)  \(:BD)  \-  \(:DC)  \(:DD)  \(:DE)  \(:DF)  \(:E0)  \(:E1)  \-
 \(:9A)  \(:9B)  \(:9C)  \(:9D)  \(:9E)  \(:9F)  \-  \(:BE)  \(:BF)  \(:C0)  \(:C1)  \(:C2)  \(:C3)  \-  \(:E2)  \(:E3)  \(:E4)  \(:E5)  \(:E6)  \(:E7)  \-

 The last 24 colors are usually defined as a grayscale ramp.

 \(:E8)  \(:E9)  \(:EA)  \(:EB)  \(:EC)  \(:ED)  \(:EE)  \(:EF)  \(:F0)  \(:F1)  \(:F2)  \(:F3)  \(:F4)  \(:F5)  \(:F6)  \(:F7)  \(:F8)  \(:F9)  \(:FA)  \(:FB)  \(:FC)  \(:FD)  \(:FE)  \(:FF)  \-


@ColorPickerRGB
$ #RGB Color Picker#
 This dialog allows to pick a color from the RGB color space.

 The 16 777 216 RGB colors are represented as a 16x16x16 hypercube.

 Use the buttons on the right to rotate the cube, access its inner levels or mix the primary colors directly.

 Each of the 4096 cells in the hypercube represents a 16x16x16 cube with RGB colors. To switch between the cubes use the #↔# button.

 The #«# button allows to save the selected color to the custom palette for quick access.

 The #System# button opens the system RGB color picker.


@SortGroups
$ #Skupiny triedenia#
 Skupiny triedenia možno použiť len pri zoraďovaní podľa #mena#
alebo #prípony# v ~súborovom paneli~@FilePanel@.
Zapnúť/vypnúť ich možno pomocou #Shift+F11#. Dovoľujú definovať dodatočné
pravidlá pre zoraďovanie, rozširujúc práve nastavené.

 Každá skupina triedenia obsahuje jednu alebo niekoľko
~masiek~@FileMasks@. Ak je niektorá skupina umiestnená
v zozname skupín vyššie ako iná a je nastavené vzostupné zoraďovanie,
všetky súbory patriace do tejto skupiny budú vyššie ako tie, patriace
do druhej.

 Príkaz #Nastavenie skupín triedenia# z menu
~príkazy~@CmdMenu@ dovoľuje vymazávať, vytvárať a upravovať
skupiny triedenia, pomocou kláves #Del#, #Ins# a #F4#. Súbory patriace do niektorej
skupiny nad oddeľovačom budú umiestnené vyššie, ako tie, ktoré nepatria do
žiadnej skupiny. Súbory patriace do niektorej skupiny pod oddeľovačom budú
umiestnené nižšie, ako tie, ktoré nepatria do žiadnej skupiny.


@FileMasks
$ #Masky súborov#
 Masky sú často použivané v príkazoch Faru na označenie jednotlivých
súborov a adresárov alebo ich skupín. Masky môžu obsahovať zvyčajné
symboly v menách súborov, náhradné znaky ('*' a '?') a špeciálne výrazy:

 #*#           ľubovoľný počet znakov;

 #?#           ľubovoľný jeden znak;

 #[cx-z]#      ľubovoľný znak z tých, čo sú uzavreté v zátvorkách.
             Dovolené sú tak samostatné znaky, ako i intervaly.

 Napríklad, súbory ftp.exe, fc.exe a f.ext možno označiť použitím masky
f*.ex?, maska *co* označí tak color.ini ako i edit.com, maskou [c-ft]*.txt
možno označiť config.txt, demo.txt, faq.txt a tips.txt.

 Mnohé príkazy Faru dovoľujú zadať niekoľko masiek oddelených čiarkami.
Napr. na označenie všetkých dokumentov môžete zadať *.doc, *.txt, *.wri
v príkaze "Označiť skupinu".

 It is allowed to put any of the masks in quotes but not the whole list. For
example, you have to do this when a mask contains any of the delimiter
characters (a comma or a semicolon), so that the mask doesn't get confused with
a list.

 File mask surrounded with slashes #/# is treated as ~Perl regular expression~@RegExp@.

 Example:
 #/(eng|rus)/i# - any files with filenames containing string “eng” or “rus”,
the character case is not taken into account.

 An #exclude mask# is one or multiple file masks that must not be matched by the
files matching the mask. The exclude mask is delimited from the main mask by
the character ‘#|#’.

 Usage examples of exclude masks:

 1. *.cpp
    All files with the extension #cpp#.
 2. *.*|*.bak,*.tmp
    All files except for the files with extensions #bak# and #tmp#.
 3. *.*|
    The character | is entered, but the mask itself is not specified,
    expression treated as *.*
 4. *.*|*.bak|*.tmp
    The character | can be used in the mask only once,
    expression treated as *.*|*.bak
 5. |*.bak
    The same as *|*.bak
 6. *.*|/^pict\d{1,3}\.gif$/i
    All files except for pict0.gif — pict999.gif, disregard the character case.

 The comma (or semicolon) is used for separating file masks from each other,
and the ‘|’ character separates include masks from exclude masks.

 File masks can be joined into ~groups~@MaskGroupsSettings@.


@SelectFiles
$ #Označovanie súborov#
 ~File panel~@FilePanel@ items (files and folders) can be selected
for group processing. Ak nie sú označené žiadne súbory, bude sa pracovať len
so súborom pod kurzorom.

 #Keyboard Selection#

 #Ins# označí súbor pod kurzorom a posunie kurzor
nižšie.

 #Shift+kurzorové klávesy# dovoľuje posúvať kurzor rôznymi smermi.
#Medzerník (Space)# pracuje podobne ako Ins, ak sa použije príslušné
makro (je súčasťou ukážkových makier) a príkazový
riadok je prázdny.

 #Sivý +# a #Sivý -# umožňujú označenie a odznačenie skupiny
pomocou jednej alebo niekoľkých ~masiek~@FileMasks@ oddelených čiarkami. #†#

 #Sivý *# invertuje aktuálne označenie. #†#

 #Ctrl+<Sivý +># a #Ctrl+<Sivý -># označuje a odznačuje všetky
súbory s rovnakou príponou, akú má súbor pod kurzorom.

 #Alt+<Gray +># a #Alt+<Gray -># označuje a odznačuje všetky
súbory s rovnakým menom, aké má súbor pod kurzorom.

 #Shift+<Sivý +># a #Shift+<Sivý -># označuje a odznačuje všetky súbory. #†#

 #Ctrl+<Sivý *># invertuje aktuálne označenie
vrátane adresárov.

 #Alt+<Gray *># inverts the current selection on files only,
folders are deselected.

 #Ctrl+M# obnoví naposledy označenú skupinu.

 #Mouse Selection#

 #Right click# toggles selection on the clicked item and moves the
cursor to it. See also #Right click selects files# option of the
~Panel settings~@PanelSettings@ dialog.

 #Right click and hold# on the #status line# or #column titles# moves
the cursor forward or backward respectively, while selecting
or deselecting items along the way. The action (selection
or deselection) depends on the state of the item under cursor before the
click.

 ────────────────
 #†# If the #Select folders# option of the
~Panel settings~@PanelSettings@ dialog is off, only files are selected
or deselected. Otherwise, the selection on the folders is changed as well.


@CopyFiles
$ #Kopírovanie, presúvanie, premenovávanie a vytváranie linkov#
 Nasledovné príkazy možno použiť na kopírovanie, presúvanie a premenovávanie súborov a adresárov:

 Skopíruj ~označené~@SelectFiles@ súbory                                      #F5#

 Skopíruj súbor pod kurzorom bez ohľadu na označenie     #Shift+F5#

 Premenuj alebo presuň označené súbory                         #F6#

 Premenuj alebo presuň súbor pod kurzorom                #Shift+F6#
 bez ohľadu na označenie

 Create ~file links~@HardSymLink@                                         #Alt+F6#

 For a folder: if the folder at the specified target path (relative
or absolute) exists, the source folder will be copied / moved inside the
target folder. Otherwise, a new folder will be created at the target
path and the contents of the source folder will be copied / moved into
the newly created folder.

 For example, when moving #c:\folder1\# to #d:\folder2\#:

 - ^<wrap>if #d:\folder2\# exists, the contents of #c:\folder1\# will be moved into
#d:\folder2\folder1\#. Otherwise, the contents of #c:\folder1\# will be moved into the newly
created #d:\folder2\#.

 If the option “#Process multiple destinations#” is enabled, you can specify
multiple copy or move targets on the input line. The targets should be separated
with character “#;#” or “#,#”. If a target name contains these characters,
enclose it in double quotes.

 Ak chcete cieľový adresár vytvoriť pred kopírovaním, pripojte k jeho
menu opačné lomítko.

 If ~Panel.Tree.TurnOffCompletely~@Panel.Tree.TurnOffCompletely@
parameter in ~far:config~@FarConfig@ is set to “false,” you can use
~Find folder~@FindFolder@ dialog to select the target path.
 The following shortcuts open the dialog with different pre-selected folders:
 - ^<wrap>#F10# stromu adresárov aktívneho panelu.
 - ^<wrap>#Alt+F10# stromu adresárov pasívneho panelu.
 - ^<wrap>#Shift+F10# selects the specified target folder. If several
paths are entered on the input line, only the first one is used.

 If the option “#Process multiple destinations#” is enabled, the folder
selected in the tree is appended to the input line.

 Možnosť kopírovania, presúvania a premenovávania súborov z prídavných
modulov závisí od funkčnosti konkrétneho modulu.

 The #Access rights# parameter is valid only for the NTFS file system
and controls how access rights of the created files and folders are set.
The #Default# option leaves access rights processing to the operating system.
The #Copy# option applies the access rights of the original objects. The
#Inherit# option applies the inheritable access rights of the
destination’s parent folder.

 The “#Already existing files#” parameter controls Far behavior
if the target file with the same name already exists.

 Possible values:
 - ^<wrap>#Ask# - a ~confirmation dialog~@CopyAskOverwrite@ will be shown;
 - #Overwrite# - all target files will be replaced;
 - #Skip# - target files will not be replaced;
 - #Rename# - existing target files will stay unchanged, copied files will be renamed;
 - #Append# - target file will be appended with the file being copied;
 - #Only newer file(s)# - only files with the newer write date and time
will be copied;
 - #Also ask on R/O files# - controls whether an additional confirmation
dialog should be displayed for the read-only files.

 Možnosť "#Použiť kopírovanie cez systém#" z ~nastavení systému~@SystemSettings@ prikáže
používanie funkcie CopyFileEx z Windows namiesto internej implementácie kopírovania.
Toto môže byť užitočné na NTFS, lebo CopyFileEx kopíruje aj rozšírené atribúty súboru. If this option is off, the internal
implementation of the file copy routine is used. The internal
function is also used if the source file is encrypted and is being
copied to a different volume.

 The “#Copy contents of symbolic links#” parameter controls the
~logic~@CopyRule@ of ~symbolic links~@HardSymLink@ processing.

 When moving files, to determine whether the operation should be performed
as a copy with subsequent deletion or as a direct move (within the same physical
drive), Far takes into account ~symbolic links~@HardSymLink@.

 Far handles copying to #con# the same way as copying to #nul# or
#\\\\.\\nul#, that is the file is read from the disk but not written
anywhere.

 When moving to #nul#, #\\\\.\\nul# or #con#, the files are not deleted
from the disk.

 The parameters “#Access rights#” and “#Only newer files#” affect only the current
copy session.

 To copy only the files that match the user defined criteria, check
the #Use filter# checkbox, then press the #Filter# button to open the
~filters menu~@FiltersMenu@. Remember that if you copy a folder and none
of the files in it match the criteria, the empty folder will #not# be
created at the destination.


@CopyAskOverwrite
$ #Copying: confirmation dialog#
 If a file of the same name exists in the target folder the user will be
prompted to select on of the following actions:

 #Overwrite# - target file will be replaced;

 #Skip# - target file will not be replaced;

 #Rename# - existing file will not be changed, a new name will be given to
the file being copied;

 #Append# - target file will be appended with the file being copied;

 If #Remember choice# is checked, the selected action will be applied to
all existing files and the confirmation dialog will not be displayed again for
the current copying session.

 If sizes and last modification dates are not enough for you to make a decision,
you can try to inspect the content of the files with internal viewer by moving the cursor
to any of them and pressing the F3 key.


@CopyRule
$ #Copying: rules#
 When ~copying/moving~@CopyFiles@ folders and/or
~symbolic links~@HardSymLink@ the following rules apply:

 #Copying of symbolic links#

 If the "Copy contents of symbolic links" option is on or the source or the
destination are remote disks, then a folder will be created at the destination
and the contents of the source symbolic link will be copied to it (recursively
for enclosed links).

 If the "Copy contents of symbolic links" option is off and the source and
the destination are local disks, then a symbolic link pointing to the source
symbolic link will be created at the destination.

 #Moving of symbolic links#

 If the "Copy contents of symbolic links" option is on or the source or the
destination are remote disks, then a folder will be created at the destination
and the contents of the source symbolic links will be copied to it (recursively
for enclosed links). The source symbolic link is then deleted.

 If the "Copy contents of symbolic links" option is off and the source and
the destination are local disks, then the source symbolic link will be moved to
the destination. Recursive descent of the tree will not be made.

 #Moving of a folder, than contains symbolic links#

 If the source and the destination are local disk, then the folder will be
moved as a normal folder.

 If the source or the destination are remote disks, then with no regard to
the "Copy contents of symbolic links" option a folder will be created in the
destination and the contents of the source symbolic link will be copied to it
(recursively for enclosed links). The source symbolic link is then deleted.


@HardSymLink
$ #Hard and Symbolic link#
 On NTFS volumes you can create #hard links# for files, #directory junctions# for
folders and #symbolic links# for files and folders using the #Alt+F6# command.

 #Hard links#

 A #hard link# is an additional directory entry for the given file. When a
hard link is created, the file is not copied itself, but receives one more name
or location, while its previous name and location remain intact. Since the
moment of its creation, a hard link is indistinguishable from the original
entry. The only difference is that short file names are not created for hard
links, and so they cannot be seen from DOS programs.

 When the file size or date changes, all of the corresponding directory
entries are updated automatically. When a file is deleted, it is not deleted
physically until all the hard links pointing at it will be deleted. The
deletion order doesn't matter. When a hard link is deleted into the recycle
bin, the number of links of a file does not change.

 Far can create hard links and can show the number of the file's hard links
in a separate column (by default, it's the last column in the 9th panel mode)
and sort the files by hard link number.

 Hard links can only be created on the same partition as the source file.

 #Directory junctions#

 Directory junctions allows to access to any local folders as to any other
local folders. For example, if the directory junction D:\\JUNCTION points to
C:\\WINNT\\SYSTEM32, a program accessing D:\\JUNCTION\\DRIVERS will actually access
C:\\WINNT\\SYSTEM32\\DRIVERS.

 Directory junctions can not point to network folders.

 Under Windows 2000 it is not allowed to create directory junctions directly to
CD-ROM folders, but this restriction can be overcome by mounting a CD-ROM
as a folder on the NTFS partition.

 #Symbolic links#

 NTFS supports symbolic links starting from Windows Vista (NT 6.0). It's an
improved version of directory junctions - symbolic links can also point to files
and non-local folders, relative paths also supported.

 By default, only members of "Administrators" group can create symbolic links,
this can be changed in the local security settings.


@ErrCopyItSelf
$ #Error: copy/move onto itself.#
 You cannot copy or move a file or folder onto itself.

 This error can also happen if there are two directories, one of which is
a ~symbolic link~@HardSymLink@ to another.


@WarnCopyEncrypt
$ #Warning: Losing file encryption#
 The source file is encrypted. Copying or moving it outside of the current
disk is possible if in the destination the file will be decrypted.

 The "Ignore" (or "Ignore all") buttons ignore the given warning and copy
the file unencrypted to the destination.

 The internal copying mechanism will be used for copying encrypted files
outside of the current disk with no regard to the "Use system copy routine"
option.


@WarnCopyStream
$ #Warning: copying or moving file with multiple streams#
 The source file contains more than one data stream or the destination file
system does not support files with multiple streams.

 Streams are a feature of the NTFS file system allowing to associate
additional information with a file (for example, author's name, title, keywords
and so on, or any other data). This information is stored together with the
file and is invisible to programs that do not support streams. For example,
streams are used by Windows Explorer to store additional file properties
(summary). FAT/FAT32 file systems do not support streams.

 To copy a file completely (together with all its streams), turn on the
option "#Use system copy routine#" in the ~System settings~@SystemSettings@
dialog.

 If you are copying a file with multiple streams to a volume with a file
system other than NTFS, you will also lose data - only the main stream will be
copied.


@ErrLoadPlugin
$ #Error: plugin not loaded#
 This error message can appear in the following cases:

 1. ^<wrap>A dynamic link library not present on your system is required for correct operation of the plugin.
 2. ^<wrap>For some reason, the plugin returned an error code telling the system to abort plugin loading.
 3. ^<wrap>The DLL file of the plugin is corrupt.


@ScrSwitch
$ #Prepínanie obrazoviek#
 Far dovoľuje otvoriť niekoľko kópií interného prezerača a editora.
Medzi panelmi a obrazovkami týchto kópií možno prepínať pomocou
#Ctrl+Tab#, #Ctrl+Shift+Tab# alebo #F12#. #Ctrl+Tab# prepne na nasledujúcu
obrazovku, #Ctrl+Shift+Tab# na predošlú a #F12# zobrazí zoznam všetkých
existujúcich obrazoviek.

 Počet prezeračov a editorov v pozadí je zobrazovaný v ľavom hornom rohu ľavého panelu.
Jeho zobrazovanie môžete zakázať v ~nastavení panelov~@PanelSettings@.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@ApplyCmd
$ #Vykonaj príkaz#
 Pomocou položky #Vykonaj príkaz# z menu ~súbory~@FilesMenu@ je možné použiť zadaný príkaz
na všetky označené súbory. Na zadanie mena súboru možno použiť ~špeciálne symboly~@MetaSymbols@
rovanké ako ~pre typy súborov~@FileAssoc@.

 Napríklad, 'type !.!' vypíše na obrazovku všetky označené súbory,
jeden za druhým, and the command 'rar32 m !.!.rar !.!' will move all selected files
into RAR archives with the same names. The command 'explorer /select,!.!' will
start the Windows Explorer and set the cursor to the current file or directory.

 See also ~"Operating system commands"~@OSCommands@


@OSCommands
$ #Operating system commands#
 Far Manager executes the following operating system commands
internally, without invoking operating system command processor:

 #CLS#
 Clears the screen.

 #disk:#
 Changes the current drive on the active panel to the specified “disk”.

 #CD [disk:]path# or #CHDIR [disk:]path#
 Changes the current path on the active panel to the specified
“path”. If the drive letter is specified, the current drive is also
changed. If the active panel shows a ~plugin~@Plugins@ emulated file
system, the “CD” command changes the folder in the plugin file system.
Unlike “CD”, the “CHDIR” command treats its parameter as a path name
in the disk file system, regardless of the file panel type.
 The #CD ~~# command changes to the home directory (if there is no
real “~~” file or directory in the current directory). The home
directory is specified in the #Use home dir# option of the
~Command line settings~@CmdlineSettings@ dialog. By default, it is the
string “%FARHOME%” denoting the Far Manager home directory.

 #CHCP [nnn]#
 Displays or sets the active code page number. Parameter “nnn”
specifies the code page number to set. “CHCP” without a parameter
displays the active code page number.

 #SET variable=[string]#
 Sets environment “variable” to the value “string”. If “string”
is not specified, the environment “variable” is removed. On startup, Far
Manager sets several ~environment variables~@FAREnv@.

 #IF [NOT] EXIST filename command#
 Executes the “command” if the “filename” exists, or does not exist
(if used with “NOT”).

 #IF [NOT] DEFINED variable command#
 Executes the “command” if the environment “variable” is defined,
or not defined (if used with “NOT”).
 “IF” commands can be nested. In the following example the “command”
will be executed if “file1” exists, “file2” does not exist, and the
environment “variable” is defined:
 #if exist file1 if not exist file2 if defined variable command#

 #PUSHD [path]#
 Stores the current path for use by the “POPD” command.
If “path” is specified, changes the current path on the active panel to it.

 #POPD#
 Changes the current path on the active panel to that stored by the “PUSHD” command.

 #CLRD#
 Clears the stack of paths stored by the “PUSHD” command.

 #TITLE [string]#
 Sets the “string” as the permanent title of the Far Manager console
window. The title will not change with switching between panels, nor
with the commands being executed, nor with the #Far window title# option
of the ~Interface settings~@InterfSettings@ dialog. The “string” preset
will be used until the end of the current session or until the default
behavior is restored by the “TITLE” command with no parameters.

 #EXIT#
 Exits Far Manager.

 Notes:

 1. ^<wrap>If the command syntax does not match one of the listed
above, Far Manager will invoke the operating system command processor to execute the command.
 2. ^<wrap>Far Manager executes the commands listed above in the following contexts:
 - ~Command line~@CmdLineCmd@
 - ~Apply command~@ApplyCmd@
 - ~User menu~@UserMenu@
 - ~File associations~@FileAssoc@


@FAREnv
$ #Environment variables#
 On startup, Far Manager sets the following environment variables available
to child processes:

 #FARHOME#            ^<wrap>path to the folder containing main Far executable module.

 #FARPROFILE#         ^<wrap>path to the folder containing roaming user data (Far & plugins settings, additional plugins etc.)

 #FARLOCALPROFILE#    ^<wrap>path to the folder containing local user data (histories, plugin cache etc.)

 #FARLANG#            ^<wrap>the name of the current interface language.

 #FARUSER#            ^<wrap>the name of the current user given by the /u ~command line~@CmdLine@ option.

 #FARDIRSTACK#        ^<wrap>the contents of directories stack top (the stack is managed with #pushd# and #popd# commands)

 #FARADMINMODE#       ^<wrap>equals "1" if Far Manager is running under an administrator account.


@RegExp
$ #Regular expressions#
 The regular expressions syntax is almost equal to Perl regexps.

 General form: #regexp# or /#regexp#/#options#.

 #Options#:
 #i# - ignore character case;
 #s# - ^<wrap>consider the whole text as one line, ‘#.#’ matches any character;
 #m# - ^<wrap>consider the whole text as multiple lines. ‘#^#’ and ‘#$#’ match the
beginning and the end of any "inner" string;
 #x# - ^<wrap>ignore space characters (unescaped ones, i.e. without backslash before).
This is useful to outline the complex expressions.

 #regexp# - the sequence of characters and metacharacters. The characters are
letters and digits, any other character is used verbatim when escaped, i.e.
prepended by the backslash ‘#\#’.

 Pay attention that all slashes and backslashes in a regular expression must
be escaped (prepended by the ‘#\#’ character) to be treated literally
rather than escape other characters themselves or mark
the end of expression. An example: the string "big\white/scary" looks in the
form of regular expression like "big\\\\white\/scary".

 #Metacharacters#

 #\#  - ^<wrap>the next character is treated verbatim, not a metacharacter;
 #^#  - ^<wrap>the beginning of string;
 #$#  - ^<wrap>the end of string;
 #|#  - ^<wrap>the alternative. Either expression before or after ‘#|#’ has to match.
      Example: "\d+\w+|Hello\d+" means "(\d+\w+)|(Hello\d+)", not "\d+(\w+|H)ello\d+".
 #()# - ^<wrap>grouping - it is used for references or when replacing matched text.
 #[]# - ^<wrap>character class - the metacharacter which matches any character
or range of characters enumerated in #[]#. Ranges are defined as [a-z].
Metacharacters are not taken into account in character classes. If the first
character inside the brackets is ‘#^#’ then this is a negative class. If the ‘#^#’ character itself
needs to be added to a class, then it must be either not the first one or escaped by a ‘#\#’.

 Except grouping, the parentheses are used for the following operations:
 #(?:pattern)#  - ^<wrap>usual grouping, but it does not get a number.
 #(?=pattern)#  - ^<wrap>the forward lookup. The matching continues from
the same place, but only if the pattern in these parentheses has matched. For
example, #\w+(?=\s)# matches the word followed by space character, and the space
is not included into the search result.
 #(?!pattern)#  - ^<wrap>the negation of forward lookup. The matching
continues from the same place if the pattern does not match. For example,
#foo(?!bar)# matches any "foo" without following "bar". Remember that this
expression has zero size, which means that #a(?!b)d# matches #ad# because #a#
is followed by the character which is not #b# (but #d#), and #d# follows the
zero-size expression.
 #(?<=pattern)# - ^<wrap>the backward lookup. Unfortunately, the pattern must have fixed length.
 #(?<!pattern)# - ^<wrap>the negation of backward lookup. The same restriction.

 #(?{name}pattern)# - group with a name. The name must contain only word characters (#\w#) and spaces (#\s#).

 #Quantifiers#

 Any character, group or class can be followed by a quantifier:

 #?#      - ^<wrap>Match 0 or 1 time, greedily.
 #??#     - ^<wrap>Match 0 or 1 time, not greedily.
 #*#      - ^<wrap>Match 0 or more times, greedily.
 #*?#     - ^<wrap>Match 0 or more times, not greedily.
 #+#      - ^<wrap>Match 1 or more times, greedily.
 #+?#     - ^<wrap>Match 1 or more times, not greedily
 #{n}#    - ^<wrap>Match exactly n times.
 #{n,}#   - ^<wrap>Match at least n times, greedily.
 #{n,}?#  - ^<wrap>Match at least n times, not greedily.
 #{n,m}#  - ^<wrap>Match at least n but not more than m times, greedily.
 #{n,m}?# - ^<wrap>Match at least n but not more than m times, not greedily.
 #{,m}#   - ^<wrap>equals to {0,m}
 #{,m}?#  - ^<wrap>equals to {0,m}?

 #"Greedy" and "not greedy" quantifiers#

 Greedy quantifier captures as many characters as possible, and only if
further match fails, it "returns" the captured string (the rollback
happens, which is rather expensive).
 When expression "A.*Z" is matched to string
"AZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", #.*# captures the whole string, and then
rolls back character by character until it finds Z. On the opposite, if the expression
is "A.*?Z" then Z is found at once. Not greedy quantifier is also known as
#minimizing#, it captures as few characters as possible, and only if
further match fails it captures more.

 #Special characters#

 Non-letter and non-digit character can be prepended by ‘#\#’ in most cases,
but in case of letters and digits this must be done with care because this is
the way the special characters are written:

 #.#    - ^<wrap>any character except carriage return. If there is “#s#” among
the options then dot matches any character.
 #\t#   - tab (0x09)
 #\n#   - new line (LF, 0x0a)
 #\r#   - carriage return (CR, 0x0d)
 #\f#   - form feed (0x0c)
 #\a#   - bell (0x07)
 #\e#   - escape (0x1b)
 #\xNNNN# - hex character, where N - [0-9A-Fa-f].
 #\Q#   - ^<wrap>the beginning of metacharacters quoting - the whole quoted
text is treated as text itself, not the regular expression
 #\E#   - the end of metacharacters quoting
 #\w#   - letter, digit or underscore (‘_’).
 #\W#   - not \w
 #\s#   - space character (tab/space/LF/CR).
 #\S#   - not \s
 #\d#   - digit
 #\D#   - not digit
 #\i#   - letter
 #\I#   - not letter
 #\l#   - lower case character
 #\L#   - not lower case character
 #\u#   - upper case character
 #\U#   - not upper case character
 #\b#   - ^<wrap>the word boundary - means that there is a word character to either left or right
 from the current position, and to the right or left, accordingly, there is a non-word character.
 #\B#   - not \b
 #\A#   - the beginning of the text, disregard the option “m”
 #\Z#   - the end of the text, disregard the option “m”
 #\O#   - ^<wrap>the no-return point. If the matching has passed by this character,
it won't roll back and and will return "no match". It can be used in complex expressions
after mandatory fragment with quantifier. This special character can be used when
big amounts of data are processed.
        Example:
        /.*?name\O=(['"])(.*?)\1\O.*?value\O=(['"])(.*?)\3/
        ^<wrap>Strings containing "name=", but not containing "value=", are processed (in fact, skipped) faster.

 #\NN#  - ^<wrap>reference to earlier matched parentheses. NN is a positive integer.
Each parentheses except (?:pattern), (?=pattern), (?!pattern), (?<=pattern) and (?<!pattern)
have a number (in the order of appearance).
        Example:
        "(['"])hello\1" matches to "hello" or 'hello'.

 #\p{name}# - ^<wrap>inner regexp reference to it's parsed bracket with specified #name#.

 #Examples:#

 #/foobar/# matches to "foobar", but not to "FOOBAR"
 #/ FOO bar /ix# matches to "foobar" and "FOOBAR"
 #/(foo)?bar/# matches to "foobar" and "bar"
 #/^foobar$/# matches to "foobar" only, but not to "foofoofoobarfoobar"
 #/[\d\.]+/# matches to any number with decimal point
 #/(foo|bar)+/# matches to "foofoofoobarfoobar" and "bar"
 #/\Q.)))$\E/# equals to "\.\)\)\)\$"


@RegExpRepl
$ #Regular expressions in replace#
 In "Replace with" line one can use special replace string regular
expressions:

 #$0#…#$N#

 The found group numbers, they are replaced with appropriate groups.
The numbers are assigned to the groups in order of opening parentheses
sequence in regular expression. #$0# means the whole found sequence.

 $#{#name#}#     Found pattern with specified #name#.


@ElevationDlg
$ #Request administrative rights#
 The current user may not have enough rights for certain file system
operations. In this case Far asks permission to retry the operation with
the elevated (administrative) rights.

 Available options:

 #Do this for all current objects#
 Do not ask for elevated rights during the current file system
operation.

 #Do not ask again in the current session#
 During the current session Far will elevate rights without asking
the user.

 See also #Request administrator rights# option in the
~System settings~@SystemSettings@ dialog.


@KeyMacro
$ #Macro commands#
 By default macros are loaded from files with #.lua# and #.moon# extensions residing in folder
#%FARPROFILE%\\Macros\\scripts#. See more details in #%FARHOME%\\Encyclopedia\\macroapi_manual.en.chm#.

 Keyboard macro commands or macro commands - are recorded sequences of key
presses that can be used to perform repetitive task unlimited number of times
by pressing a single hotkey.

 Each macro command has the following parameters:
 - ^<wrap>a hotkey, that will execute the recorded sequence when pressed;
 - additional ~settings~@KeyMacroSetting@, that influence the method and
   the area of execution of the recorded sequence.

 Macro commands are mostly used for:
 - ^<wrap>Performing repetitive task unlimited number of times by
pressing a single hotkey.
 - Execution of special functions, which are represented by
special commands in the text of the macro command.
 - Redefine standard hotkeys, which are used by Far for
execution of internal commands.

 The main usage of macro commands is assignment of hotkeys for calling
external plugins and for overloading Far actions.

 See also:

 ~Macro command areas of execution~@KeyMacroArea@
 ~Hotkeys~@KeyMacroAssign@
 ~Recording and playing-back macro commands~@KeyMacroRecPlay@
 ~Deleting a macro command~@KeyMacroDelete@
 ~Macro command settings~@KeyMacroSetting@
 ~The list of installed macros~@KeyMacroList@


@KeyMacroArea
$ #Macro command: areas of execution#
 Far allows the creation of independent ~macro commands~@KeyMacro@ (commands with
identical hotkeys) for different areas of execution.

 #Attention#: The area of execution, to which the macro command will
belong, is determined by the location in which the
recording of the macro command has been #started#.

 Currently those are the available independent areas:
 - file panels;
 - internal viewer;
 - internal editor;
 - dialogs;
 - quick file search;
 - select drive menu;
 - main menu;
 - other menus;
 - help window;
 - info panel;
 - quick view panel;
 - tree panel;
 - user menu;
 - screen grabber, vertical menus.

 It is impossible to assign a macro command to an already used hotkey. When
such an attempt is made, a warning message will appear telling that the macro
command that is assigned to this hotkey will be deleted.

 This way you can have identical hotkeys for different macro commands only
in different areas of execution.


@KeyMacroAssign
$ #Macro command: hotkeys#
 A ~macro command~@KeyMacro@ can be assigned to:
 - ^<wrap>any key;
 - any key combination with #Ctrl#, #Alt# and #Shift# modifiers;
 - any key combination with two modifiers.
   Far allows to use the following double modifiers:
   #Ctrl+Shift+<key>#, #Ctrl+Alt+<key># and #Alt+Shift+<key>#

 A macro command #can't# be assigned to the following key combinations:
#Alt+Ins#, #Ctrl+<.>#, #Ctrl+Shift+<.>#, #Ctrl+Alt#, #Ctrl+Shift#, #Shift+Alt#,
#Shift+<character>#.

 It is impossible to enter some key combinations (in particular #Enter#,
#Esc#, #F1#, #Ctrl+F5#, #MsWheelUp# and #MsWheelDown# with #Ctrl#, #Shift#,
#Alt#) in the hotkey assignment dialog because of their special meanings. To
assign a macro command to such key combination, select it from the dropdown
list.


@KeyMacroRecPlay
$ #Macro command: recording and playing-back#
 A ~macro command~@KeyMacro@ can be played-back in one of the two following modes:

 1. ^<wrap>General mode: keys pressed during the recording or the
playing-back #will be# sent to plugins.
 2. ^<wrap>Special mode: keys pressed during the recording or the
playing-back #will not be# sent to plugins that intercept editor events.

 For example, if some plugin processes the key combination - #Ctrl+A#, then
in the special mode this plugin will not receive focus and will not do what it
usually does as a reaction to this combination.

 Creation of a macro command is achieved by the following actions:

 #Start recording a macro command#
 Press #Ctrl+<.># (#Ctrl# and a period pressed together) to record
a macro in the general mode or #Ctrl+Shift+<.># (#Ctrl#, #Shift# and
a period pressed together) to record a macro in the special mode.
 As the recording begins, the '\CFR\-' character will appear in the
upper left corner of the screen.

 #Enter Contents of the macro command#
 All keys pressed during the recording will be saved with the following exceptions:
 - ^<wrap>only keys processed by Far will be saved. Meaning that if
during the macro recording process an external program is
run inside the current console then only the keys pressed
before the execution and after completion of that program
will be saved.

 #Note#: During macro recording, all other macros are disabled. Thus,
it is impossible to create a “multilevel” macro which would call
previously recorded macros.

 #Finish recording the macro command#
 To finish a macro recording there are special key
combinations. Because a macro command can be additionally
configured there are two such combinations: #Ctrl+<.># (#Ctrl#
and a period pressed together) and #Ctrl+Shift+<.># (#Ctrl#,
#Shift# and a period pressed together). Pressing the first
combination will end the recording of the macro command
and will use the default settings for its playback. Pressing
the second combination will end the recording of the macro
command and a dialog showing macro command ~options~@KeyMacroSetting@
will appear.

 #Assign a hotkey to the macro command#
 When the macro recording is finished and all the options are set the
~hotkey assignment~@KeyMacroSetting@ dialog will appear, where the hotkey that
will be used to execute the recorded sequence can be set.

 Playing back a macro is indicated by showing the '\2FP\-' character in the upper-left screen corner.
See also "~Macros.ShowPlayIndicator~@Macros.ShowPlayIndicator@" for turning that indication on/off.


@KeyMacroDelete
$ #Macro command: deleting a macro command#
 To delete a ~macro command~@KeyMacro@ an empty (containing no commands)
macro should be recorded and assigned the hotkey of the macro command that
needs to be deleted.

 This can be achieved by the following steps:
 1. ^<wrap>Start recording a macro command (#Ctrl+<.>#)
 2. Stop recording a macro command (#Ctrl+<.>#)
 3. Enter or select in the hotkey assignment
dialog the hotkey of the macro command that
needs to be deleted.

 #Attention#: after deleting a macro command, the key combination
(hotkey) that was used for its execution will begin
to function as it was meant to, originally. That is
if that key combination was somehow processed by Far
or some plugin then after deleting the macro command
the key combination would be processed by them as in
the past.


@KeyMacroSetting
$ #Macro command: settings#
 To specify additional ~macro command~@KeyMacro@ settings, start or finish
macro recording with #Ctrl+Shift+<.># instead of #Ctrl+<.># and select the
desired options in the dialog:

 #Sequence:#
 Allows to edit the recorded key sequence.

 #Description:#
 Allows to edit the description of key sequence.

 #Allow screen output while executing macro#
 If this option is not set during the macro command execution Far Manager
does not redraw the screen. All the updates will be displayed when the macro
command playback is finished.

 #Execute after Far start#
 Allows to execute the macro command immediately after the Far Manager is started.

 The following execution conditions can be applied for the active and passive panels:

 #Plugin panel#
 [x] - ^<wrap>execute only if the current panel is a plugin panel
 [ ] - execute only if the current panel is a file panel
 [?] - ignore the panel type

 #Execute for folders#
 [x] - ^<wrap>execute only if a folder is under the panel cursor
 [ ] - execute only if a file is under the panel cursor
 [?] - execute for both folders and files

 #Selection exists#
 [x] - ^<wrap>execute only if there are marked files/directories on the panel
 [ ] - execute only if there are no marked files/directories on the panel
 [?] - ignore the file selection state

 Other execution conditions:

 #Empty command line#
 [x] - ^<wrap>execute only if the command line is empty
 [ ] - execute only if the command line is not empty
 [?] - ignore the command line state

 #Selection block present#
 [x] - ^<wrap>execute only if there is a selection block present in the editor, viewer, command line or dialog input line
 [ ] - execute only if there is no selection present
 [?] - ignore selection state

 Notes:

 1. ^<wrap>Before executing a macro command, all of the above conditions are checked.

 2. ^<wrap>Some key combinations (including #Enter#, #Esc#, #F1# and #Ctrl+F5#,
#MsWheelUp#, #MsWheelDown# and other mouse keys combined with #Ctrl#, #Shift#, #Alt#) cannot be entered
directly because they have special functions in the dialog. To ~assign a macro~@KeyMacroAssign@
to one of those key combinations, select it from the drop-down list.


@KeyMacroList
$ #Macros: The list of installed macros#
 The following is a list of topics where you can find out which ~macros~@KeyMacro@
are available in the current Far Manager session.

 ~Common macros#~@KeyMacroCommonList@

 ~File panels~@KeyMacroShellList@
 ~Quick View panel~@KeyMacroQViewList@
 ~Tree panel~@KeyMacroTreeList@
 ~Info panel~@KeyMacroInfoList@
 ~Autocompletion in panels~@KeyMacroShellAutoCompletionList@

 ~Fast Find in panels~@KeyMacroSearchList@
 ~Find Folder~@KeyMacroFindFolderList@

 ~Dialogs~@KeyMacroDialogList@
 ~Autocompletion in dialogs~@KeyMacroDialogAutoCompletionList@

 ~Main menu~@KeyMacroMainMenuList@
 ~Change drive menu~@KeyMacroDisksList@
 ~User menu~@KeyMacroUserMenuList@
 ~Other menus~@KeyMacroMenuList@

 ~Viewer~@KeyMacroViewerList@
 ~Editor~@KeyMacroEditList@

 ~Help window#~@KeyMacroHelpList@

 ~Other areas~@KeyMacroOtherList@


@KeyMacroCommonList
$ #Macros: Common macros#
 The following macro keys are available in all areas.

<!Macro:Common!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroQViewList
$ #Macros: Quick View panel#
 The following macro keys are available in Quick View panel.

<!Macro:Common!>

<!Macro:Qview!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroMainMenuList
$ #Macros: Main Menu#
 The following macro keys are available in Main Menu.

<!Macro:Common!>

<!Macro:MainMenu!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroTreeList
$ #Macros: Tree Panel#
 The following macro keys are available in Tree Panel.

<!Macro:Common!>

<!Macro:Tree!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroDialogList
$ #Macros: Dialogs#
 The following macro keys are available in dialogs.

<!Macro:Common!>

<!Macro:Dialog!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroInfoList
$ #Macros: Info Panel#
 The following macro keys are available in Info Panel.

<!Macro:Common!>

<!Macro:Info!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroDisksList
$ #Macros: Change Drive menu#
 The following macro keys are available in Change Drive menu.

<!Macro:Common!>

<!Macro:Disks!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroUserMenuList
$ #Macros: User Menu#
 The following macro keys are available in User Menu.

<!Macro:Common!>

<!Macro:UserMenu!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroShellList
$ #Macros: File panels#
 The following macro keys are available in file panels.

<!Macro:Common!>

<!Macro:Shell!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroSearchList
$ #Macros: Fast Find in panels#
 The following macro keys are available in Fast Find dialog.

<!Macro:Common!>

<!Macro:Search!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroFindFolderList
$ #Macros: Find Folder#
 The following macro keys are available in Find Folder window.

<!Macro:Common!>

<!Macro:FindFolder!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroEditList
$ #Macros: Editor#
 Macro-commands available in the editor are listed below.

<!Macro:Common!>

<!Macro:Editor!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroViewerList
$ #Macros: Viewer#
 Macro-commands available in the viewer are listed below.

<!Macro:Common!>

<!Macro:Viewer!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroMenuList
$ #Macros: Other menus#
 The following macro keys are available in other menus.

<!Macro:Common!>

<!Macro:Menu!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroHelpList
$ #Macros: Help window#
 The following macro keys are available in Help window.

<!Macro:Common!>

<!Macro:Help!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroOtherList
$ #Macros: Other areas#
 The following macro keys are available in other areas: screen grabber, vertical menus.

<!Macro:Common!>

<!Macro:Other!>

 See also ~"The list of installed macros"~@KeyMacroList@


@KeyMacroShellAutoCompletionList
$ #Macros: Autocompletion in panels#
 The following macro keys are available in Autocompletion in panels.

<!Macro:Common!>

<!Macro:ShellAutoCompletion!>

 See also ~«The list of installed macros»~@KeyMacroList@


@KeyMacroDialogAutoCompletionList
$ #Macros: Autocompletion in dialogs#
 The following macro keys are available in Autocompletion in dialogs.

<!Macro:Common!>

<!Macro:DialogAutoCompletion!>

 See also ~«The list of installed macros»~@KeyMacroList@


@FarAbout
$ #Version information#
 Starts with the command #far:about#

 Displays:
 - Far Manager version and bitness
 - versions of the third-party libraries used in the project
 - names and versions of the active plugins


@CustomizingUI
$ #Customizing UI elements#
 All Far user interface elements are defined in #.lng# files (e.g., #FarEng.lng#).
You can customize these elements to your needs. For example, you can
change hotkeys or labels on a certain Far dialog. To override
UI elements, create a file with the name #Far<Lng>.lng.custom#
in #%FARHOME%# or #%FARPROFILE%# directory and provide new values for
the UI IDs you want to customize.

 For example, to make file system link types on Far panels look like
in the listing of DIR DOS command, you can create file #FarEng.lng.custom#
in the #%FARPROFILE%# directory with the following content:

@-
 \0A┌────────────────────────────┐\-
 \0A│\30│  │UTF-8│Ln 1/7│Col│8:34 PM\0A│\-
 \0A│\1b//[MListUp]                 \0A│\-
 \0A│\1b"..↑"                       \0A│\-
 \0A│\1b                            \0A│\-
 \0A│\1bMListFolder   = "<DIR>"     \0A│\-
 \0A│\1bMListSymlink  = "<SYMLINK>" \0A│\-
 \0A│\1bMListJunction = "<JUNCTION>"\0A│\-
 \0A│\071\30Help  \07 2\30Save  \07 3\30      \07 \30    \0A│\-
 \0A└────────────────────────────┘\-
@+

 You can specify replacement UI elements in two ways, on two separate
lines or on a single line. The new value should always be enclosed
in double quotation marks. You can find UI IDs you want to redefine
in the original #Far<Lng>.lng# file.

 If custom UI language files exist in both directories, the file
in #%FARPROFILE%# has precedence over the file in #%FARHOME%#.


@FarConfig
$ #Configuration editor#
 Starts with the command #far:config#

 Allows to view and edit all Far Manager’s options.
 Most options can be changed from the ~Options menu~@OptMenu@, however some options are available only here or using configuration import.
The options are displayed in a list with three fields per item: the name in the SectionName.ParamName format (for example, Editor.TabSize),
the type (boolean, 3-state, integer, string), and the value (for the integer type, hexadecimal and symbolic representations additionally displayed).
If current value of an option is other than the default, the option is marked with the ‘*’ character to the left of the name.

 Besides the list navigation keys, the following key combinations are supported:

 #Enter# or #F4#
 Toggle or edit the value.

 #Shift+F4#
 Edit the integer value as a hexadecimal number. For other types works as #F4#.

 #Alt+F4#
 Edit the integer value as a binary number. For other types works as #F4#.

 #Del#
 Reset the option to its default value.

 #Ctrl+H#
 Toggle display of unchanged options.

 #Shift+F1#
 Show the help for the current option, if available.

 #Ctrl+Alt+F#
 Toggle quick filtering mode.


@Codepages.NoAutoDetectCP
$ #far:config Codepages.NoAutoDetectCP#
 This parameter allows to exclude specific code pages from the heuristic code page detection results.
Such detection is unreliable by definition: it depends on statistical data and could guess wrong, especially when the amount of input data is small.

 By default the parameter is empty and there are no restrictions which code pages could be detected heuristically.

 If this parameter is set to #-1#, only the code pages, currenltly visible in the ~Code pages~@CodePagesMenu@ menu, will be accepted.
You can control which code pages are visible there with the #Ctrl+H# key combination and the #Favorites# section.

 If this parameter contains a comma-separated list of code page numbers, all the specified code pages will be excluded from the heuristic detection.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Help.ActivateURL
$ #far:config Help.ActivateURL#
 This numeric parameter controls whether Far will open (activate) URL
links in help files:

 0 - ^<wrap>URL links are not opened;
 1 - URL links are opened;
 2 - URL links are opened after user’s confirmation.

 Default value: 1 (links are opened).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Confirmations.EscTwiceToInterrupt
$ #far:config Confirmations.EscTwiceToInterrupt#
 This Boolean parameter controls the behavior of #Esc# key in the
confirmation dialog for canceling an operation.

 False - ^<wrap>#Esc# key closes the dialog and continues the operation;
 True  - #Esc# key closes the dialog and cancels the operation.

 Default value: False (close the dialog and continue operation).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.AllCtrlAltShiftRule
$ #far:config System.AllCtrlAltShiftRule#
 This numeric parameter controls which user interface objects can be
temporarily hidden with #Ctrl+Alt+Shift# key combination. Each bit
corresponds to a certain object type.

 Bit numbers:
 0 - Panels;
 1 - Editor;
 2 - Viewer;
 3 - Help window;
 4 - Dialogs and menus.

 If a bit is set, objects of the corresponding type can be hidden.

 By default, all objects can be hidden.

 See also ~System.CASRule~@System.CASRule@.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.CASRule
$ #far:config System.CASRule#
 This numeric parameter allows to disable #Ctrl+Alt+Shift# key
combination for temporary hiding user interface objects. Individual
bits control the behavior of left and right key combinations.

 Bit numbers:
 0 - Left #Ctrl+Alt+Shift# key combination;
 1 - Right #Ctrl+Alt+Shift# key combination.

 If a bit is set, corresponding key combination hides interface objects.

 By default, both key combinations are enabled.

 See also ~System.AllCtrlAltShiftRule~@System.AllCtrlAltShiftRule@.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Panel.ShellRightLeftArrowsRule
$ #far:config Panel.ShellRightLeftArrowsRule#
 This Boolean parameter controls the behavior of left and right arrow
keys, both on main keyboard and numeric keypad.

 False - ^<wrap>As in Far 1.70. If command line is not empty, the
behavior of #Left#, #Right#, #Numpad4#, and #Numpad6# keys depends
on the ~panel view mode~@PanelViewModes@.
         - ^<wrap>If file names are displayed in multiple stripes (panel
modes 2 and 3 by default), then the keys move panel cursor, just like
with empty command line.
         - ^<wrap>If file names are displayed in a single stripe (all
other panel modes by default), the keys control the command line caret.
 True  - ^<wrap>When the panel is on, the #Left#, #Right#, #Numpad4#,
and #Numpad6# keys always move panel cursor, even if the command line
is not empty. The behavior does not depend on the current panel mode.

 Note: The #Ctrl+D# and #Ctrl+S# keys always move command line caret.

 Default value: False (traditional behavior).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Panel.Layout.ScrollbarMenu
$ #far:config Panel.Layout.ScrollbarMenu#
 This Boolean parameter enables menu scrollbar when there are more menu
items than can fit vertically.

 False - ^<wrap>Never show menu scrollbar;
 True  - Show menu scrollbar if needed.

 Default value: True (show menu scrollbar if needed).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Panel.CtrlFRule
$ #far:config Panel.CtrlFRule#
 This Boolean parameter controls the behavior of #Ctrl+F# key
combination in the ~command line~@CmdLineCmd@.

 False - ^<wrap>The file name is inserted into the command line as it is
recorded in the file system;
 True  - The file name is inserted as it appears on the file panel,
possibly in lowercase or using the short name.

 Default value: False (insert as is).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Panel.CtrlAltShiftRule
$ #far:config Panel.CtrlAltShiftRule#
 This numeric parameter controls the behavior of #Ctrl+Alt+Shift# key
combination for temporary hiding file panels.

 0 - ^<wrap>Hide panels only (like #Ctrl+O# key combination);
 1 - Hide panels and command line;
 2 - Hide panels, command line, and functional key bar at the bottom
line.

 Default value: 0 (hide panels only).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Panel.RightClickRule
$ #far:config Panel.RightClickRule#
 This numeric parameter controls the behavior of #right mouse click#
on an empty stripe of file panel.

 0 - ^<wrap>Move panel cursor to the last file in the previous stripe
and select the file;
 1 - Move panel cursor to the last file in the previous stripe without
selecting the file (like the #left mouse click#);
 2 - Do not move panel cursor or select any file.

 Note: If the stripe is not empty the last file is always selected.

 Default value: 2 (do nothing).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.ExcludeCmdHistory
$ #far:config System.ExcludeCmdHistory#
 This numeric parameter suppresses saving commands of certain categories
to the history. If a bit in the parameter’s value is set, commands
of the corresponding category will not be saved.

 Bit numbers and corresponding command categories:
 0 - ^<wrap>Windows file type associations;
 1 - Far ~file associations~@FileAssoc@;
 2 - Executable files under cursor on ~file panel~@FuncCmd@;
 3 - Commands entered on ~command line~@CmdLineCmd@.

 Default value: 0 (save commands of all categories).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.Executor.RestoreCP
$ #far:config System.Executor.RestoreCP#
 This Boolean parameter controls whether Far will restore console code
page after the execution of an external program has completed. Some
programs change console code page during execution and do not restore
it before exiting. Use this parameter to compensate for this behavior.

 False - ^<wrap>Leave it as is; do not restore console code page;
 True  - Restore console code page after an external program exited.

 Default value: True (restore console code page).

 See also #CHCP# ~Operating system~@OSCommands@ command.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.Executor.ExcludeCmds
$ #far:config System.Executor.ExcludeCmds#
 This string parameter defines commands which will be directly passed
for execution to the operating system command processor (specified
by the #ComSpec# environment variable), without searching the current
directory, directories listed on the #PATH# environment variable,
or any other predefined places.

 The commands are separated by semicolon (#;#). Environment variables
surrounded by the percent sign (#%#) will be substituted.

 For example, if the value of this parameter is “DATE;ECHO” and “date”
is entered on the command line, the internal command processor’s #DATE#
command will be executed. To execute an external program “date.exe”,
type the file name verbatim, including extension. However, if “DATE”
is not listed in this parameter and the program “date.exe” exists
in one of the #PATH# directories, the internal command processor’s
command can never be executed.

 Ready-made settings for CMD.EXE and other well-known
command processors can be found in the
#Addons\SetUp\Executor.*.farconfig# files.

 Note: Far executes some ~operating system~@OSCommands@ commands
internally, without invoking operating system command processor. These
commands are not included in #Executor.*.farconfig# files. Some other
OS commands Far executes with the limited functionality. If the syntax
does not match exactly that specified in the
~Operating system commands~@OSCommands@ help topic, the command will
be passed for execution to the command processor.

 Default value: empty string #""#.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.Executor.ComspecArguments
$ #far:config System.Executor.ComspecArguments#
 This string parameter defines the arguments which Far will use to
invoke the operating system command processor (specified by the
#ComSpec# environment variable).

 The #{0}# placeholder will be replaced with the text of the command.
This parameter is handy with non-standard command processors requiring
unusual command line options or quoting.

 Default value: #/S /C "{0}"# (compatible with CMD.EXE).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Interface.FormatNumberSeparators
$ #far:config Interface.FormatNumberSeparators#
 This string parameter allows to override digit grouping symbol and
decimal symbol in OS regional settings.

 First symbol  - ^<wrap>digit grouping symbol;
 Second symbol - decimal separator symbol.

 Default value: empty string #""# (use OS regional settings).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.CmdHistoryRule
$ #far:config System.CmdHistoryRule#
 This Boolean parameter defines whether the current position
in ~commands history~@History@ will change if #Esc# is pressed after
#Ctrl+E# or #Ctrl+X# key combinations.

 False - ^<wrap>The current command in the history will remain the one
recalled with #Ctrl+E# / #Ctrl+X#.
 True  - The current command in the history will be reset to the latest
(newest) command.

 Note: The order of the commands in the history does not change in any
case.

 Default value: False (change the current position in the commands
history).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.ConsoleDetachKey
$ #far:config System.ConsoleDetachKey#
 This string parameter specifies key combination to detach Far console
from a non-interactive process running in it.

 If a long-running process is using Far console, press this key
combination to create a new Far console where Far will continue running
as if the process has already ended, while leaving the old console
to the process.

 This feature can come handy if, for example, an archiver process
started from the Far command line is taking more time than you
expected, and you want to continue editing a file opened in background
Editor, or simply do not want to launch a new Far instance.

 Default value: #"CtrlShiftTab"# (the #Ctrl+Shift+Tab# key combination
detaches Far console).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.QuotedSymbols
$ #far:config System.QuotedSymbols#
 This string parameter defines special characters that require quoting
of file and folder names. If a name contains one of these characters,
Far will surround it with double quotes when inserting into the command
line or editor, or copying to the clipboard.

 Default value: #" &()[]{}^=;!'+,` "#. The first symbol is
~Space (U+0020)~@https://en.wikipedia.org/wiki/Space_(punctuation)@;
the last symbol is ~Non-breaking space (U+00A0)~@https://en.wikipedia.org/wiki/Non-breaking_space@.

 See also ~System.QuotedName~@System.QuotedName@.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.QuotedName
$ #far:config System.QuotedName#
 This numeric parameter controls whether Far will surround with double
quotes file and folder names containing special characters (see
~System.QuotedSymbols~@System.QuotedSymbols@ parameter). Individual
bits control the behavior in different contexts.

 Bit numbers:
 0 - ^<wrap>Quote names when inserting into the command line or editor;
 1 - Quote names when copying to the clipboard.

 Default value: 1 (quote when inserting into the command line or editor).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Interface.AltF9
$ #far:config Interface.AltF9#
 This Boolean parameter controls the behavior of the #Alt+F9# key
combination (toggle the size of the Far console window).

 False - ^<wrap>Toggle Far window height between 25 and 50 lines; set
window width to 80 columns;
 True  - Maximize Far window or restore it to normal size.

 Default value: True (maximize / restore).

 Note: This parameter affects the behavior only in windowed mode.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Dialog.CBoxMaxHeight
$ #far:config Dialog.CBoxMaxHeight#
 This numeric parameter specifies the maximum height of history list
in dialogs.

 Default value: 8.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Editor.UndoDataSize
$ #far:config Editor.UndoDataSize#
 This numeric parameter limits the size of undo memory buffer in Editor.

 Default value: 104857600 (100MB).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Editor.CharCodeBase
$ #far:config Editor.CharCodeBase#
 This numeric parameter defines how the code of the character under the
cursor is represented on Editor’s status line.

 0 - Octal value (6 digits with the leading zero);
 1 - Decimal value (up to 5 digits);
 2 - Hexadecimal value (4 digits followed by the character ‘h’).

 Default value: 1 (decimal).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Editor.BSLikeDel
$ #far:config Editor.BSLikeDel#
 This Boolean parameter defines the behavior of the #BackSpace# key when
a vertical block is selected in Editor.

 False - ^<wrap>Deletes the character to the left of the cursor; keeps
the (vertical) selection;
 True  - Deletes the selected vertical block, like the #Del# key does.

 Default value: True (deletes the selection).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Editor.AllowEmptySpaceAfterEof
$ #far:config Editor.AllowEmptySpaceAfterEof#
 This Boolean parameter defines whether scrolling down in Editor (e.g.,
with the #Ctrl+Down# key combination) can leave empty space at the
bottom of the window.

 False - ^<wrap>Stop scrolling when the last line of the file appears
at the bottom of the window;
 True  - Continue scrolling until the last line of the file reaches the
cursor leaving empty space under the cursor.

 Default value: False (stop scrolling).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Interface.RedrawTimeout
$ #far:config Interface.RedrawTimeout#
 This numeric parameter specifies the refresh time (in milliseconds)
of the progress dialog displayed during various long-running
operations, such as copying, moving, deleting and searching files and
folders, applying access rights after moving files and folders,
scanning the file system.

 The larger the value, the less frequently the information about the
operation is displayed, the faster the operation itself is performed.

 Default value: 200 ms.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@TitleFormat
$ #far:config Interface.ViewerTitleFormat, Interface.EditorTitleFormat#
 These string parameters define console window title for
~Editor~@Editor@ and ~Viewer~@Viewer@.

 Macro #%File# is expanded to the name of the file being edited
or viewed.

 Macro #%Lng# is replaced with the word “edit” or “view” in the current
language, see ~Options menu~@OptMenu@.

 The #Far window title addons# string of the
~Interface settings~@InterfSettings@ will be automatically appended
to the console window title.

 Default value: #"%Lng %File"#.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.WipeSymbol
$ #far:config System.WipeSymbol#
 This numeric parameter defines the filler byte for the
~wipe file~@DeleteFile@ operation (#Alt+Del# key combination).

 Each byte of the file is overwritten with the least significant byte
of the parameter. If the parameter is set to #-1#, random values will
be used.

 Default value: 0.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.ShowCheckingFile
$ #far:config System.ShowCheckingFile#
 This Boolean parameter controls whether plugin’s name is displayed
in the console window title while the plugin is checking a file.

 When the user presses #Enter# or #Ctrl+PgDn#, Far invokes registered
plugins one by one to check if they can “open” or otherwise render the
file. If this parameter is True, Far will show plugin’s name in the
console window title while the plugin is checking the file.

 Default value: False (do not show plugins’ names).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.PluginMaxReadData
$ #far:config System.PluginMaxReadData#
 This numeric parameter defines the maximum amount of file data used
to find the plugin which supports the file.

 When the user presses #Enter# or #Ctrl+PgDn#, Far reads the number
of bytes specified by this parameter from the beginning of the file and
passes the data to registered plugins to check if they can “open”
or otherwise render the file.

 Minimum value is 131072 (128 KiB). The maximum is limited only by the
size of the logical address space (2^32 - 1 or 2^64 – 1).

 Setting the value of this parameter above 5 MiB is not recommended.

 Default value: 131072 (0x20000).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.SetAttrFolderRules
$ #far:config System.SetAttrFolderRules#
 This Boolean parameter defines the default value of the
#Process subfolders# option of the file ~Attributes~@FileAttrDlg@ dialog
when changing attributes of a single directory.

 False - ^<wrap>The #Process subfolders# option is on; date and time
fields are cleared.
 True  - The #Process subfolders# option is off; date and time fields
are set to the actual values.

 Default value: True (do not process subfolders).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.CopyBufferSize
$ #far:config System.CopyBufferSize#
 This numeric parameter defines the size of the buffer used by the
internal file copy routine (see #Use system copy routine# option of the
~System settings~@SystemSettings@ dialog).

 If the value of this parameter is zero, the default buffer size
of 32768 bytes is used.

 Default value: 0 (buffer size is 32768 bytes).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.SubstNameRule
$ #far:config System.SubstNameRule#
 This numeric parameter specifies the types of physical drives which
will be queried when Far detects drives assigned using #SUBST# command.

 Far attempts to detect if a drive was substituted to display
appropriate information on the ~Change drive~@DriveDlg@ menu and
~Info panel~@InfoPanel@, as well as in some other cases.

 Bit numbers:
 0 - Query removable drives;
 1 - Query drives of all other types.

 Default value: 2 (query all non-removable drives). For example,
if a drive is associated with a CD-ROM path, it will not be detected
as substituted.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.SubstPluginPrefix
$ #far:config System.SubstPluginPrefix#
 This Boolean parameter controls whether Far prepends plugin prefix
to the path to plugin panel’s object when inserting the path into the
command line (#Ctrl+F#, #Ctrl+[#, etc.) or copying it to the clipboard
(#Alt+Shift+Ins#, #Ctrl+Alt+Ins#).

 False - ^<wrap>Do not prepend plugin prefix to the path to an object
on plugin panel.
 True  - Prepend plugin prefix except when the plugin manages real
files, like #Temporary panel# does.

 Default value: False (do not prepend plugin prefix).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.CopySecurityOptions
$ #far:config System.CopySecurityOptions#
 This numeric parameter controls the initial state of the
#Access rights# option in the ~Copy / Move~@CopyFiles@ dialog.

 The #Access rights# option specifies the access rights assigned
to newly created files and folders and can be one of:

 #Default# - ^<wrap>Access rights are controlled by the operating system;
 #Copy#    - Copy access rights of the source objects;
 #Inherit# - Inherit access rights of the parent folder.

 The initial state of the #Access rights# option when the dialog
is opened is controlled by three bits of the
#System.CopySecurityOptions# parameter. Bits 0, 1, and 2 control the
state of the option in the Move dialog; bits 3, 4, and 5 -- in the Copy
dialog.

@=
 Copy     Move     ^<wrap>Initial state of the #Access rights#
 Dialog   Dialog   option when the dialog is opened
@=
 Bit 0    Bit 3    0 - ^<wrap>#Default# (bits 1 / 4 are ignored)
                   1 - controlled by bits 1 / 4

 Bit 1    Bit 4    0 - ^<wrap>#Copy# (if bit 0 / 3 is set to 1)
                   1 - #Inherit# (if bit 0 / 3 is set to 1)

 Bit 2    Bit 5    0 - ^<wrap>Defined by bits 0 and 1 / 3 and 4 of this parameter
                   1 - The last user’s choice (within the current Far session)

 Default value: 0 (when the dialog is opened, the #Access rights# option
is always set to #Default#; user’s choices are not remembered).

 Examples:

 #0x21# (binary 100'001)

 - ^<wrap>In the Move dialog, the #Access rights# option is always set
to #Copy#.
 - In the Copy dialog, the option is initially set to #Default#; then
the previous user’s choice is remembered (within the current Far
session).

 #0x1C# (binary 011'100)

 - ^<wrap>In the Move dialog, the #Access rights# option is initially
set to #Default#; then the previous user’s choice is remembered (within
the current Far session).
 - In the Copy dialog, the option is always set to #Inherit#.

 Note: The #System.CopySecurityOptions# parameter does not affect
creation of links (#Alt+F6#). In this case access rights are always
copied.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Interface.CursorSize
$ #far:config Interface.CursorSizeX#
 These numeric parameters control cursor size in Far console window.
Cursor size can be set separately for insert and override mode, as well
as for windowed and fullscreen mode.

@-
 ┌──────────╥───────────────────────┬───────────────────────┐
 │ Mode     ║ Windowed              │ Fullscreen            │
 ╞══════════╬═══════════════════════╪═══════════════════════╡
 │ Insert   ║ Interface.CursorSize1 │ Interface.CursorSize2 │
 ├──────────╫───────────────────────┼───────────────────────┤
 │ Override ║ Interface.CursorSize3 │ Interface.CursorSize4 │
 └──────────╨───────────────────────┴───────────────────────┘
@+

 The parameters specify the fraction of the character cell in percents
filled by the cursor. Parameters’ values may vary from 1 to 100
corresponding to the cursor changing from the single horizontal line
at the bottom of the cell to the solid block filling the entire cell.
If parameter’s value is zero, the system console setting is used.

 Default values:
 Interface.CursorSize1: 15
 Interface.CursorSize2: 10
 Interface.CursorSize3: 99
 Interface.CursorSize4: 99

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.WordDiv
$ #far:config System.WordDiv#
 This string parameter defines additional word delimiters besides
#Space# and #Tab#.

 Default value: #~~!%^&*()+|{}:"<>?`-=\\[];',./#

 This parameter can be changed via ~far:config~@FarConfig@ only.


@XLat.WordDivForXlat
$ #far:config XLat.WordDivForXlat#
 This string parameter defines word delimiters for transliteration
(#XLat# function) of the current word without selecting it.

 Default value: #Space#, #Tab# and characters #!##$%^&*()+|=\\/@?#.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Editor.ReadOnlyLock
$ #far:config Editor.ReadOnlyLock#
 This numeric parameter controls the behavior of the Editor when opening
files with #Read-only#, #Hidden# or #System# attributes.

 Bit numbers:
 0 - ^<wrap>Lock down editing of read-only files;
 1 - Warn when opening read-only files;
 2 - Unused;
 3 - Unused;
 4 - Unused;
 5 - Also, apply the behavior defined by bits 0 and 1 to hidden files;
 6 - Also, apply the behavior defined by bits 0 and 1 to system files.

 Default value: 0 (allow editing of any files without warnings).

 For example, if this parameter is set to #0x43# (binary 0100'0011),
warning will be shown when opening read-only and system files; editing
of such files will be disabled.

 This parameter can be changed via ~far:config~@FarConfig@. Bits 0 and
1 are also controlled by the options #Lock editing of read-only files#
and #Warn when opening read-only files# of the ~Editor~@EditorSettings@
settings dialog.


@Editor.FileSizeLimit
$ #far:config Editor.FileSizeLimit#
 This numeric parameter defines file size limit; when exceeded,
a warning message will be shown before opening the file in Editor.

 If the value of this parameter is zero, the warning is disabled. The
limit is defined in bytes.

 Default value: 0 (file size is not checked, and the warning is never
displayed).

 Note: When a file is opened in Editor, its entire content is loaded
into memory. Thus, opening very large files could be undesirable. The
warning enabled by this parameter helps to avoid opening of large files
inadvertently.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.MsWheelDelta
$ #far:config System.MsWheelDelta* & System.MsHWheelDelta*#
 These numeric parameters define scroll speed when mouse wheel is rolled
or tilted. Scroll speed can be specified separately for different
directions and different areas.

 Roll the wheel one notch to scroll the specified number of lines at
a time vertically:

 System.MsWheelDeltaView  - ^<wrap>in the internal Viewer
 System.MsWheelDeltaEdit  - in the internal Editor
 System.MsWheelDeltaHelp  - on help pages
 System.MsWheelDelta      - in other areas

 Tilt the wheel to scroll the specified number of characters at a time
horizontally (Windows Vista and above):

 System.MsHWheelDeltaView - ^<wrap>in the internal Viewer
 System.MsHWheelDeltaEdit - in the internal Editor
 System.MsHWheelDelta     - in other areas

 Default value for all parameters: 0 (use system settings).

 Note: Rolling or tilting mouse wheel while holding #Alt# key always
scrolls one line or character at a time.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@System.CopyTimeRule
$ #far:config System.CopyTimeRule#
 This numeric parameter specifies whether the progress (speed, time, and
estimated remaining time) is displayed during file copy operations.

 Bit numbers:
 0 - ^<wrap>If set, show progress while copying to NUL;
 1 - If set, show progress during regular file copy operations.

 Default value: 3 (always display progress of file copy operations).

 Note: Since this feature requires some time to gather statistics, it is
likely that no progress is displayed for small files if the option
#Show total copy progress indicator# is turned off in the
~Interface settings~@InterfSettings@ dialog.

 This parameter can be changed via ~far:config~@FarConfig@ or by the
#Show copying time information# option of the
~Interface settings~@InterfSettings@ dialog. However, only values 0 or
3 can be set using this option.


@Policies.ShowHiddenDrives
$ #far:config Policies.ShowHiddenDrives#
 This Boolean parameter specifies whether Far honors the
#Hide these specified drives in My Computer# Windows Group Policy.

 False - ^<wrap>Far shows only drives visible (not hidden) in Windows
Explorer;
 True  - Far shows all drives (ignores the Group Policy).

 Default value: True (show all drives).

 Note: The state of this Group Policy is stored in the #NoDrives# value
of the
#\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer# key in
both #HKLM# and #HKCU# hives of Windows Registry. If the NoDrives value
in HKLM hive is zero (no hidden files on Local Machine), the value
in HKCU hive is ignored.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Editor.KeepEditorEOL
$ #far:config Editor.KeepEditorEOL#
 This Boolean parameter controls how line breaks within the text on the
clipboard are pasted into the edited file.

 False - ^<wrap>Line breaks in the pasted text are preserved. After the
paste operation, line breaks in the edited file may have different styles.
 True  - If the file is not empty, line breaks in the pasted text are
changed to match the line break style of the edited file. If the file
is empty, line breaks are not changed; this parameter has no effect.

 Default value: True (match line break style of the edited file).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Editor.AddUnicodeBOM
$ #far:config Editor.AddUnicodeBOM#
 This Boolean parameter specifies whether Byte Order Mark (BOM) is added
at the beginning of the files created by the Editor and saved in
a UNICODE encoding (UTF-8, UTF-16LE, UTF-16BE).

 False - ^<wrap>BOM is not added.
 True  - BOM is added.

 Default value: True (BOM is added).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Editor.NewFileUnixEOL
$ #far:config Editor.NewFileUnixEOL#
 This Boolean parameter specifies line break style in the files created
by the Editor.

 False - ^<wrap>Files are created with Windows line break style (CR LF).
 True  - Files are created with Unix line break style (LF).

 Default value: False (Windows line break style).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Panel.ShortcutAlwaysChdir
$ #far:config Panel.ShortcutAlwaysChdir#
 This Boolean parameter controls the behavior of
~folder shortcuts~@FolderShortcuts@ (#RightCtrl+0…9# key combinations)
when the panels are hidden.

 False - ^<wrap>Folder shortcuts insert the associated path into the
command line.
 True  - Folder shortcuts change the current folder even if the panels
are hidden.

 Default value: False (inset the shortcut path).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Macros.ShowPlayIndicator
$ #far:config Macros.ShowPlayIndicator#
 This Boolean parameter turns macro playback indicator (symbol ‘\2FP\-’
at the top left-hand corner of the screen) on or off.

 False - ^<wrap>The indicator is turned off.
 True  - The indicator is turned on.

 Default value: True (the indicator is on).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Viewer.SearchWrapStop
$ #far:config Viewer.SearchWrapStop#
 This tri-state parameter controls the behavior of the “continue search”
key combinations in the Viewer (#Shift+F7#, #Space#, #Alt+F7#) when
search wraps around the beginning or end of the file or passes the
search starting point.

 False - ^<wrap>Silently wrap around the beginning or end of the file
or the search starting point.
 True  - Show message when wrapping around the beginning or end of the file.
 Other - Show message when passing the search starting point.

 Default value: True (silently wrap around).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@XLat.Layouts
$ #far:config XLat.Layouts#
 This string parameter defines the input locales (keyboard layouts)
which Far will cycle through. If this parameter is specified, system
input locales will be ignored.

 This parameter contains semicolon (#;#) separated list of hexadecimal
input locale identifiers. For example, value #0409;0419# (or
#04090409;04190419#) can be used to switch between input locales
“en-US” and “ru-RU”.

 If less than two input locale identifiers are specified, Far will use
input locales installed in the system.

 Only first 10 locales are used, the rest of the list is ignored.

 Default value: empty string (use system input locales).

 See also Addons\XLat\Russian\Qwerty.farconfig.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@XLat.Flags
$ #far:config XLat.Flags#
 This numeric parameter controls the behavior of Far API function XLat
(string transcoding based on keyboard layout).

 Bit numbers:
 0  - ^<wrap>Automatically switch keyboard layout after transcoding
operation. Far cycles through all system keyboard layouts or layouts
defined in ~XLat.Layouts~@XLat.Layouts@ config parameter.
 1  - Sound beep after switching keyboard layout.
 2  - When a character could not be transcoded using
~XLat.Tables~@XLat.Tables@, Far will attempt to apply special
~XLat.Rules~@XLat.Rules@. If this bit is set and there is a named rule
corresponding to the current keyboard layout, this rule will be used;
otherwise, one of the three numbered rules will be used.
 16 - Transcode the entire command line if nothing is selected.

 Default value: 0x00010001 (switch keyboard layout and transcode the
entire command line if no selection).

 See also Addons\XLat\Russian\Qwerty.farconfig.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@XLat.Tables
$ #far:config XLat.Tables#
 These string parameters define two-way transcoding table which is used
by Far API function XLat (string transcoding based on keyboard layout).

 #XLat.Table1# ^<wrap>is the list of characters from the national
alphabet which will be replaced with their Latin counterparts defined
in #XLat.Table2#.
 #XLat.Table2# is the list of Latin characters which will be replaced
with their national counterparts defined in #XLat.Table1#.

 Default value: empty string (transcoding table is not defined).

 If a character cannot be transcoded using the table, Far will attempt
to apply special ~XLat.Rules~@XLat.Rules@.

 See also Addons\XLat\Russian\Qwerty.farconfig.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@XLat.Rules
$ #far:config XLat.Rules#
 These string parameters define special transcoding rules used by Far
API function XLat (string transcoding based on keyboard layout).

 Far will attempt to apply special rules if a character could not be
transcoded using the ~XLat.Tables~@XLat.Tables@.

 Each rule contains the sequence of character pairs. If a character
in the transcoded string matches the first character in the pair,
it will be replaced with the second character in the pair.

 One of the three numbered rules is used if the bit 2 (0x04)
in ~XLat.Flags~@XLat.Flags@ is zero or there is no named rule
corresponding to the current keyboard layout.

 #XLat.Rules1# ^<wrap>is applied if the previous character in the
transcoded string is from the national alphabet.
 #XLat.Rules2# is applied if the previous character in the transcoded
string is a Latin character.
 #XLat.Rules3# is applied if the previous character in the transcoded
string is neither from the national alphabet, nor a Latin character.

 A named special rule is applied if the bit 2 (0x04)
in ~XLat.Flags~@XLat.Flags@ is set to one. Far uses hexadecimal value
of the current input locale identifier (keyboard layout) to find the
corresponding rule. For example, if current keyboard layout is “en-US”,
Far will look up the rule named #XLat.00000409# and use it if it
exists. Otherwise, Far will fall back to the numbered rules.

 Default value: empty string for all rules (special rules are not
defined).

 See also Addons\XLat\Russian\Qwerty.farconfig.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Interface.DelHighlightSelected
$ #far:config Interface.DelHighlightSelected#
 This Boolean parameter controls how the items which will be deleted are
displayed in the file / folder #Delete# confirmation dialog.

 False - ^<wrap>The items to be deleted are always displayed in plain
text, without highlighting.
 True  - If more than one item is to be deleted or the deleted item
is not the item under cursor, the deleted item(s) will be highlighted
in the dialog.

 Default value: True (highlight the list if it does not match the item
under cursor).

 Note: This parameter does not affect which items will be deleted;
it only controls how the deleted items are shown in the dialog.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Interface.DelShowSelected
$ #far:config Interface.DelShowSelected#
 This numeric parameter controls the number of items which are displayed
in the file / folder #Delete# confirmation dialog.

 The maximum number of displayed items is either this parameter’s value
or half of Far window height, whichever is less. The minimum number
of items is one.

 Default value: 10.

 This parameter can be changed via ~far:config~@FarConfig@ only.


@History.Config
$ #far:config History.*#
 These parameters limit the number and the lifetime of the items of the
following histories:

 - History of command line ~commands~@History@:
   #History.CommandHistory.Count#
   #History.CommandHistory.Lifetime#

 - History of entries in dialog edit controls:
   #History.DialogHistory.Count#
   #History.DialogHistory.Lifetime#

 - History of recently ~visited folders~@HistoryFolders@:
   #History.FolderHistory.Count#
   #History.FolderHistory.Lifetime#

 - History of recently ~viewed and edited files~@HistoryViews@:
   #History.ViewEditHistory.Count#
   #History.ViewEditHistory.Lifetime#

 Default values:
 - Maximum history size (*.Count): 1000 items
 - Lifetime of an item (*.Lifetime): 90 days

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Editor.F8CPs
$ #far:config Editor.F8CPs#
$ #far:config Viewer.F8CPs#
 These string parameters define code pages which are cycled through when
#F8# key is pressed in ~Editor~@Editor@ or ~Viewer~@Viewer@.

 The value of each parameter is a list of code page numbers or symbolic
names listed below. Symbolic names are case insensitive. Duplicated
code pages, as well as unsupported code pages, are ignored.

 - #ANSI#    ^<wrap>variants #ACP#, #WIN#
 - #OEM#     variants #OEMCP#, #DOS#
 - #UTF8#    variant #UTF-8#
 - #DEFAULT# stands for the default code page defined in
~Editor~@EditorSettings@ or ~Viewer~@ViewerSettings@ settings dialog.

 If the string is empty or does not contain any supported code pages,
ANSI and OEM code pages are used.

 Special parameter value of #-1# stands for #ANSI;OEM;Default#.

 Example: #ANSI,OEM,65001#.

 Default value: empty string (ANSI and OEM code pages).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Panel.Tree.TurnOffCompletely
$ #far:config Panel.Tree.TurnOffCompletely#
 This Boolean parameter enables or disables all folder tree operations:

 - ^<wrap>~Tree panel~@TreePanel@ mode in
~left and right menus~@LeftRightMenu@;
 - The toggle tree panel shortcut key (#Ctrl+T#);
 - ~Find folder~@FindFolder@ panel command (#Alt+F10#);
 - Folder tree operations in ~copy, move and rename~@CopyFiles@
dialog (#F10#, #Alt+F10#, and #Shift+F10#).

 False - ^<wrap>Folder tree operations are #enabled#.
 True  - All folder tree operations are #disabled#.

 Note: If folder tree operations are disabled, folder tree cache files,
even if already exist, are not updated when folders are created,
deleted, or renamed.

 Default value: True (all folder tree operations are disabled).

 This parameter can be changed via ~far:config~@FarConfig@ only.


@Index
$ #Index help file#
<%INDEX%>
