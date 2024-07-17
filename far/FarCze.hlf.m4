m4_include(`farversion.m4')m4_dnl
.Language=Czech,Czech (Čeština)
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
$^#Manažer souborů a archívů#
$^#M4_MACRO_GET(FULLVERSION)#
$^#Copyright © 1996-2000 Eugene Roshal#
$^#Copyright © 2000-M4_MACRO_GET(COPYRIGHTYEAR) Far Group#
 ~Help file index~@Index@
 ~Jak používat Nápovědu~@Help@

 ~O Faru~@About@
 ~License~@License@

 ~Přepínače příkazové řádky~@CmdLine@
 ~Popis funkčních kláves~@KeyRef@
 ~Podpora pluginů~@Plugins@
 ~Přehled schopností pluginů~@PluginsReviews@

 ~Panely~@Panels@:
  ~Souborový panel~@FilePanel@
  ~Panel Strom~@TreePanel@
  ~Panel Info~@InfoPanel@
  ~Panel Zběžné zobrazení~@QViewPanel@
  ~Drag and drop (Uchop a táhni) soubory~@DragAndDrop@
  ~Úprava módů pohledu souborového panelu~@PanelViewModes@
  ~Vybírání souborů~@SelectFiles@

 ~Menu~@Menus@:
  ~Levý a Pravý~@LeftRightMenu@
  ~Soubory~@FilesMenu@
  ~Příkazy~@CmdMenu@
  ~Nastavení~@OptMenu@

 ~Hledat soubor~@FindFile@
 ~Historie~@History@
 ~Hledat adresář~@FindFolder@
 ~Porovnat adresáře~@CompFolders@
 ~Uživatelské menu~@UserMenu@
 ~Menu změnit jednotku~@DriveDlg@

 ~Závislosti souborů~@FileAssoc@
 ~Příkazy operačního systému~@OSCommands@
 ~Adresářové zkratky~@FolderShortcuts@
 ~Třídění skupin~@SortGroups@
 ~Filtr panelu souborů~@FiltersMenu@
 ~Přepínání obrazovek~@ScrSwitch@
 ~Seznam úloh~@TaskList@
 ~Hotplug devices list~@HotPlugList@

 ~Nastavení systému~@SystemSettings@
 ~Nastavení panelů~@PanelSettings@
 ~Tree settings~@TreeSettings@
 ~Nastavení rozhraní~@InterfSettings@
 ~Dialog settings~@DialogSettings@
 ~Menu settings~@VMenuSettings@
 ~Command line settings~@CmdlineSettings@

 ~Zvýraznění souborů~@Highlight@
 ~Popisy souborů~@FileDiz@
 ~Nastavení prohlížeče~@ViewerSettings@
 ~Nastavení editoru~@EditorSettings@

 ~Kopírování, přesouvání, přejmenovávání a vytváření linků~@CopyFiles@

 ~Interní prohlížeč~@Viewer@
 ~Interní editor~@Editor@

 ~Masky souborů~@FileMasks@
 ~Klávesová makra~@KeyMacro@

 ~Customizing UI elements~@CustomizingUI@


@Help
$ #Jak používat Nápovědu#
 V nápovědě můžete nalézt odkazy, které vedou do dalších obrazovek
nápovědy. Also, the main page has the "~Help Index~@Index@", which lists all the
topics available in the help file and in some cases helps to find the needed
information faster.

 Můžete použít klávesy #Tab# a #Shift+Tab# pro přesouvání kurzoru
z jednoho odkazu na jiný, potom stiskněte Enter pro přechod na stránku
nápovědy popisující danou položku. S myší můžete kliknout přímo na odkaz
vedoucí na obrazovku o dané položce.

 Pokud se text nevešel na jednu stránku, zobrazí se posuvník.
V tom případě můžete použít #kurzorové šipky# pro skrolování textem.

 Pro návrat na předchozí stránku nápovědy můžete stisknout #Alt+F1#
nebo #BackSpace# a #Shift+F1# pro zobrazení obsahu nápovědy.

 Stiskněte #Shift+F2# pro ~pluginy~@Plugins@ help.

 Press #F7# to search for text in the current help file. Search results
will be displayed as links to relevant topics.

 #Help# is shown by default in a reduced windows, you can maximize it by
pressing #F5# "#Zoom#", pressing #F5# again will restore the window to the
previous size.


@About
$ #O Faru#
 #Far# je textový manažer souborů a archívů pro Windows. Podporuje #dlouhé
názvy souborů# a zajišťuje širokou sadu souborových a adresářových operací.

 #Far# is #freeware# and #open source# software distributed under the
revised BSD ~license~@License@.

 #Far# umožňuje #transparentní zpracování archívů#. Se soubory v archívu
je možno pracovat stejně snadno, jako v adresáři: pokud pracujete
s archívem, Far transforuje vaše příkazy do odpovídajících externích
volání archivačních programů.

 #Far# také nabízí mnoho dalších služeb.


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
$ #Přepínače příkazové řádky#
 Následující přepínače mohou být použity na příkazové řádce:

 #-e[<řádka>[:<poz>]] <název souboru>#
 Upravuje určený soubor. Po /e můžete volitelně
specifikovat počáteční řádku a pozici řádky pro úpravu.
 Například: far /e70:2 readme.

 #-p[<cesta>]#
 Vyhledávání pluginů v adresáři, specifikovaném <cestou>.
 Several search paths can be specified, separated by ‘;’.

 Například: #far -p%USERPROFILE%\\Far\\Plugins#

 #-co#
 Přinutí Far nahrávat pluginy pouze z cache. V tomto módu se Far nahrává rychleji,
ale nové, nebo updatované pluginy nejsou rozpoznány. Použití tohoto parametru má
význam #jen# tehdy, pokud je seznam instalovaných pluginů neměnný. Po updatování pluginu
je nutné spustit Far bez tohoto parametru. Pokud není plugin cache, nebudou žádné pluginy nhrány.

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

 #-u <jméno uživatele>#
 Umožňuje oddělit nastavení pro různé uživatele.
 Affects only 1.x Far Manager plugins
 Například: far -u host

 Manažer Far přepne ~systémovou proměnnou~@FAREnv@ "FARUSER" na hodnotu <jméno uživatele>.

 #-v <název souboru>#
 Zobrazuje určený soubor. Pokud je <název souboru> `#-#', jsou data čteny z stdin.

 Například, "dir|far /v -" zobrazí výstup příkazu dir.

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
 Vypne obsluhu vyjímek (exception handling). Toto nastavení je navrženo pro
vývojáře a nedoporučuje se používat jej v průběhu normální práce s Farem.

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
$ #Popis funkčních kláves#
 ~Ovládání panelů~@PanelCmd@

 ~Příkazová řádka~@CmdLineCmd@

 ~Správa souborů a příkazy služeb~@FuncCmd@

 ~Podpora kolečka myši~@MsWheel@

 ~Menu control commands~@MenuCmd@

 ~Screen grabber~@Grabber@

 ~Různé~@MiscCmd@

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
$ #Ovládání panelů#
 #Obecné příkazy panelů#

 Změna aktivního panelu                                         #Tab#
 Přehození panelu                                            #Ctrl+U#
 Znovu načíst panel                                          #Ctrl+R#
 Zobrazení info panelu                                       #Ctrl+L#
 ~Zběžné zobrazení~@QViewPanel@                                            #Ctrl+Q#
 ~Zobrazení stromu~@TreePanel@                                            #Ctrl+T#
 Skrytí/ukázání obou panelů                                  #Ctrl+O#
 Dočasné skrytí obou panelů                          #Ctrl+Alt+Shift#
 (tak dlouho, dokud jsou stiskuty klávesy)
 Skrytí/ukázání neaktivního panelu                           #Ctrl+P#
 Skrytí/ukázání levého panelu                               #Ctrl+F1#
 Skrytí/ukázání pravého panelu                              #Ctrl+F2#
 Změna výšky panelu                           #Ctrl+Nahoru,Ctrl+Dolů#
 Změna šířky panelu                          #Ctrl+Vlevo,Ctrl+Vpravo#
 Change panels width                           #Ctrl+Left,Ctrl+Right#
 (pokud je příkazová řádka prázdná)
 Obnovení nastavení implicitní šířky panelů            #Ctrl+Numpad5#
 Restore default panels height                     #Ctrl+Alt+Numpad5#
 Zobrazení/Skrytí panelu funkčních kláves na spodní části.   #Ctrl+B#
 Toggle total and free size show mode                  #Ctrl+Shift+S#
 in bytes (if possible) or with size suffixes K/M/G/T

 #Příkazy panelu souborů#

 ~Označit/odznačit soubor~@SelectFiles@                 #Ins, Shift+Kurzorové šipky#
                                                 #Right mouse button#
 Označit skupinu                                              #Num +#
 Odznačit skupinu                                             #Num -#
 Obrátit výběr                                                #Num *#
 Označit soubory se stejnou příponou                   #Ctrl+<Num +>#
   jakou má nynější soubor
 Odznačit soubory se stejnou příponou                  #Ctrl+<Num ->#
   jakou má nynější soubor
 Obrátit výběr a zahrnout adresáře                     #Ctrl+<Num *>#
   (ignoruje stav příkazové řádky)
 Označit soubory se stejným jménem                      #Alt+<Num +>#
   jaké má nynější soubor
 Odznačit soubory se stejným jménem                     #Alt+<Num ->#
   jaké má nynější soubor
 Označit všechny soubory                              #Shift+<Num +>#
 Odznačit všechny soubory                             #Shift+<Num ->#
 Obnovit předešlý výběr                                      #Ctrl+M#

 Skrolování dlouhých jmen a popisů             #Alt+Vlevo,Alt+Vpravo#
                                                   #Alt+Home,Alt+End#

 Nastavit mód stručného pohledu                          #LeftCtrl+1#
 Nastavit mód středního pohledu                          #LeftCtrl+2#
 Nastavit mód plného pohledu                             #LeftCtrl+3#
 Nastavit mód širokého pohledu                           #LeftCtrl+4#
 Nastavit mód detailního pohledu                         #LeftCtrl+5#
 Nastavit mód popisového pohledu                         #LeftCtrl+6#
 Nastavit mód dlouhého popisového pohledu                #LeftCtrl+7#
 Nastavit mód vlastníka souboru                          #LeftCtrl+8#
 Nastavit mód linkového pohledu                          #LeftCtrl+9#
 Nastavit mód alternativního plného pohledu              #LeftCtrl+0#

 Nastavit zobrazování skrytých a systémových souborů         #Ctrl+H#
 Nastavit zobrazování dlouhých/krátkých jmen souborů         #Ctrl+N#

 Skrýt/Ukázat levý panel                                    #Ctrl+F1#
 Skrýt/Ukázat pravý panel                                   #Ctrl+F2#

 Třídit soubory v aktivním okně podle názvu                 #Ctrl+F3#
 Třídit soubory v aktivním okně podle přípony               #Ctrl+F4#
 Třídit soubory v aktivním okně podle data modifikace       #Ctrl+F5#
 Třídit soubory v aktivním okně podle velikosti             #Ctrl+F6#
 Nechat soubory v aktivním okně nesetříděné                 #Ctrl+F7#
 Třídit soubory v aktivním okně podle data vytvoření        #Ctrl+F8#
 Třídit soubory v aktivním okně podle data přístupu         #Ctrl+F9#
 Třídit soubory v aktivním okně podle popisu               #Ctrl+F10#
 Třídit soubory v aktivním okně podle vlastníka souboru    #Ctrl+F11#
 Zobrazit menu ~módů třídění~@PanelCmdSort@                                #Ctrl+F12#
 Použít třídění skupin                                    #Shift+F11#
 Zobrazovat nejdříve vybrané soubory                      #Shift+F12#

 Vytvoření ~adresářové zkratky~@FolderShortcuts@                        #Ctrl+Shift+0…9#
 Skok na adresářovou zkratku                          #RightCtrl+0…9#

 If the active panel is a ~quick view panel~@QViewPanel@, a ~tree panel~@TreePanel@ or
an ~information panel~@InfoPanel@, the directory is changed not on the
active, but on the passive panel.

 #Ctrl+Ins#
 Kopírování názvů vybraných souborů do schránky (pokud je příkazový řádek prázdný)

 #Ctrl+Shift+Ins#
 Kopírování názvů vybraných souborů do schránky

 #Alt+Shift+Ins#
 Kopíruje plné názvy vybraných souborů do schránky

 #Ctrl+Alt+Ins#
 Kopíruje skutečná názvy vybraných souborů do schránky

 #Ctrl+Shift+C#
 Copy the selected files to clipboard.

 #Ctrl+Shift+X#
 Cut the selected files to clipboard.

 Files, copied or cut from the panels, can be pasted to other applications, e.g. Explorer.

 See also the list of ~macro keys~@KeyMacroShellList@, available in the panels.

 Poznámka:

 1. ^<wrap>Pokud je zapnuto nastavení "Dovolit změnit mód třídění" v
~Nastavení Panelů~@PanelSettings@, stisknutím stejné kombinace
kláves pro třídění se změní orientace ze vzrůstající na klesající a obráceně;

 2. ^<wrap>Kombinace kláves #Alt+Vlevo# a #Alt+Vpravo# pro skrolování dlouhých
názvů a popisů souborů funguje pouze s kurzorovymi šipkami,
které se nenacházejí na Numpadu. Toto je způsobeno tím, že pokud
je stisknut Alt slouží klávesy na Numpadu pro psaní číslic.

 3. ^<wrap>Kombinace kláves #Ctrl+Alt+Ins# vloží následující text do schránky:
 - ^<wrap>pro síťové disky - síťový (UNC) název souborového objektu;
 - ^<wrap>pro lokální disky - lokální název souboru berouc v úvahu
~symbolické linky~@HardSymLink@.

 4. ^<wrap>Pokud je stisknuto #Alt+Shift+Ins# nebo #Ctrl+Alt+Ins# když je
kurzor na souboru "#..#", zkopíruje se název aktuálního adresáře.


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
$ #Příkazy příkazové řádky#
 #Nejčastější povely příkazové řádky#

 O znak doleva                                         #Vlevo,Ctrl+S#
 O zank doprava                                       #Vpravo,Ctrl+D#
 O slovo doleva                                          #Ctrl+Vlevo#
 O slovo doprava                                        #Ctrl+Vpravo#
 Začtek řádky                                             #Ctrl+Home#
 Konec řádky                                               #Ctrl+End#
 Smazání znaku                                                  #Del#
 Smazání znaku vlevo                                      #BackSpace#
 Smazat do konce řádky                                       #Ctrl+K#
 Smazat slovo vlevo                                  #Ctrl+BackSpace#
 Smazat slovo vpravo                                       #Ctrl+Del#
 Kopírovat do schránky                                     #Ctrl+Ins#
 Vložit ze schránky                                       #Shift+Ins#
 Předchozí příkaz                                            #Ctrl+E#
 Další příkaz                                                #Ctrl+X#
 Smazání příkazové řádky                                     #Ctrl+Y#

 #Vkládací příkazy#

 Vložit aktuální název souboru z aktivního panelu #Ctrl+J, Ctrl+Enter#

 In the ~fast find~@FastFind@ mode, #Ctrl+Enter# does not insert a
file name, but instead cycles the files matching the
file mask entered in the fast find box.

 Vložit aktuální název souboru z pasivního panelu  #Ctrl+Shift+Enter#
 Vložit plný název souboru z aktivního panelu                #Ctrl+F#
 Vložit plný název souboru z pasivního panelu                #Ctrl+;#
 Vložit síťový (UNC) náev souboru z aktivního panelu     #Ctrl+Alt+F#
 Vložit síťový (UNC) název souboru z pasivního panelu    #Ctrl+Alt+;#

 Vložit cestu z levého panelu                                #Ctrl+[#
 Vložit cestu z pravého panelu                               #Ctrl+]#
 Vložit síťovou (UNC) cestu z levého panelu              #Ctrl+Alt+[#
 Vložit síťovou (UNC) cestu z pravého panelu             #Ctrl+Alt+]#

 Vložit cestu z aktivního panelu                       #Ctrl+Shift+[#
 Vložit cestu z pasivního panelu                       #Ctrl+Shift+]#
 Vložit síťovou (UNC) cestu z aktivního panelu          #Alt+Shift+[#
 Vložit síťovou (UNC) cestu z pasivního panelu          #Alt+Shift+]#

 Poznámka:

 1. ^<wrap>Pokud je příkazová řádka prázdná, #Ctrl+Ins# kopíruje vybrané
jména souborů z panelu do schránky stejně jako #Ctrl+Shift+Ins#
(podívejte se na ~příkazy Ovládání panelů~@PanelCmd@);

 2. ^<wrap>Stlačení #Ctrl+End# na konci příkazové řádky, nahradí současný
obsah příkazové řádky příkazem z ~historie~@History@
začínajícím znakem který je obsažen v příkazové řádce, pokud
takový existuje. Po dalším stisknutí Ctrl+End se zobrazí
následující podobný příkaz.

 3. ^<wrap>Většina výše popsaných příkazů funguje i pro editování řetězců
v dialozích ovládání a interním editoru.

 4. ^<wrap>#Alt+Shift+Vlevo#, #Alt+Shift+Vpravo#, #Alt+Shift+Home# a
#Alt+Shift+End# vybírají blok v příkazové řádce i pokud jsou
panely zapnuty.

 5. ^<wrap>Kombinace kláves umožňují vložit do síťového (UNC) názvu
souborového objektu následující vložený text v příkazovém řádku:
 - ^<wrap>pro síťové disky - síťový (UNC) název souborového objektu;
 - ^<wrap>pro lokální disky - lokální název souboru berouc v úvahu ~symbolické linky~@HardSymLink@.


@FuncCmd
$ #Správa souborů a příkazy služeb#
 Online nápověda                                                 #F1#

 Ukázat ~menu uživatele~@UserMenu@                                           #F2#

 Prohlížeč                              #Ctrl+Shift+F3, Numpad 5, F3#
 Pokud stisknete na souboru #Numpad 5# nebo #F3# vyvoláte ~interní~@Viewer@,
externí nebo ~asociovaný~@FileAssoc@ prohlížeč, v závislosti na typu souboru
a ~nastavení externího prohlížeče~@ViewerSettings@.
 #Ctrl+Shift+F3# ignoruje závislosti souborů a vždy volá interní prohlížeč.
 Pokud je stisknuto na adresáři, vypočte se a zobrazí kapacita vybraných
adresářů.

 Editor                                           #Ctrl+Shift+F4, F4#
 F4 vyvolá ~interní~@Editor@, externí nebo ~asociovaný~@FileAssoc@ editor,
v závislosti na typu souboru a ~nastavení externího editoru~@EditorSettings@.
 #Ctrl+Shift+F4# ignoruje závislosti souborů a vždy volá interní editor.
 #F4# a #Ctrl+Shift+F4# stisknuté na adresáři vyvolá dialog pro změnu ~atributů~@FileAttrDlg@ souboru.

 ~Kopírovat~@CopyFiles@                                                       #F5#
 Kopírování souborů a adresářů. Pokud si přejete vytvořit cílový adresář
před kopírováním, ukončete název zpětným lomítkem.

 ~Přejmenovat nebo přesunout~@CopyFiles@                                      #F6#
 Přesouvání nebo přejmenovávání souborů a adresářů. Pokud si přejete před
přesunutím vytvořit cílový adresář, ukončete název zpětným lomítkem.

 ~Vytvořit nový adresář~@MakeFolder@                                           #F7#

 ~Smazat~@DeleteFile@                                     #Shift+Del, Shift+F8, F8#

 ~Zničit~@DeleteFile@                                                     #Alt+Del#

  Ukázat ~menu~@Menus@                                                     #F9#

 Konec Faru                                                     #F10#

 Ukázat příkazy ~pluginů~@Plugins@                                         #F11#

 Změni aktuální jednotku pro levý panel                      #Alt+F1#

 Změní aktuální jednotku pro pravý panel                     #Alt+F2#

 Interní/externí prohlížeč                                   #Alt+F3#
 Pokud je implicitně používán interní prohlížeč, vyvolává
prohlížeč externí, specifikovaný v ~nastavení~@ViewerSettings@,
nebo ~asociovaný prohlížecí program~@FileAssoc@ pro daný typ souboru.
Pokud je implicitně používán externí prohlížeč, vyvolává prohlížeč interní.

 Interní/externí editor                                      #Alt+F4#
 Pokud je implicitně používán interní editor, vyvolává
editor externí, specifikovaný v ~nastavení~@EditorSettings@,
nebo ~asociovaný editor~@FileAssoc@ pro daný typ souboru. Pokud je
implicitně používán externí editor, vyvolává editor interní.

 Tisknout soubory                                            #Alt+F5#
 If the "Print Manager" plugin is installed then the printing of
the selected files will be carried out using that plugin,
otherwise by using internal facilities.

 Vytvoření ~souborového linku~@HardSymLink@ (jen NTFS)                      #Alt+F6#
 Použitím pevných linků můžete mít různé názvy souborů odkazujících
na stejná data.

 Provádí příkaz ~hledat soubor~@FindFile@                                #Alt+F7#

 Zobrazuje ~historii příkazů~@History@                                  #Alt+F8#

 Vybírá velikost konzolového okna Faru                       #Alt+F9#
 V okením módu přepíná mezi nynější velikostí a maximální možnou
velikostí konzolového okna. V celoobrazovkovém režimu přepíná #Alt+F9#
výšku obrazu mezi 25 a 50 řádky. Pro více informací se podívejte na
~Interface.AltF9~@Interface.AltF9@.

 Nastavení ~plugin~@Plugins@ modulů.                              #Alt+Shift+F9#

 Provádí příkaz ~hledat adresář~@FindFolder@                              #Alt+F10#

 Zobrazuje ~Historii prohlížeče a editoru~@HistoryViews@                    #Alt+F11#

 Zobrazuje ~historii adresářů~@HistoryFolders@                                #Alt+F12#

 Přidává soubory do archívu                                #Shift+F1#
 Rozbaluje soubory z archívu                               #Shift+F2#
 Provádí příkazy správy archívu                            #Shift+F3#

 Vytvoření a úprava ~nového souboru~@FileOpenCreate@                         #Shift+F4#
 When a new file is opened, the same code page is used as in the last opened
editor. If the editor is opened for the first time in the current Far session,
the default code page is used.

 Kopíruje soubor pod kurzorem                              #Shift+F5#
 Přejmenuje nebo přesune soubor pod kurzorem               #Shift+F6#
 For folders: if the specified path (absolute or relative) points to an
existing folder, the source folder is moved inside that folder. Otherwise the
folder is renamed/moved to the new path.
 E.g. when moving #c:\folder1\# to #d:\folder2\#:
 - ^<wrap>if #d:\folder2\# exists, contents of #c:\folder1\# is
moved into #d:\folder2\folder1\#;
 - ^<wrap>otherwise contents of #c:\folder1\# is moved into the
newly created #d:\folder2\#.

 ~Smaže soubor~@DeleteFile@ pod kurzorem                                 #Shift+F8#

 Uložení konfigurace                                       #Shift+F9#

 Vybere poslední provedenou položku menu                  #Shift+F10#

 Vykonání, změna adresáře, vstup do archívu                   #Enter#
 Vykonání v odděleném okně                              #Shift+Enter#
 Execute as administrator                            #Ctrl+Alt+Enter#

 Stisknutí kombinace #Shift+Enter# na adresáři vyvolá Průzkumníka
Windows a ukáže v něm vybraný adresář.
Pokud chcete v Průzkumníkovi zobrazit kořenový adresář, stiskněte
#Shift+Enter# na požadované jednotce v ~menu pro změnu disku~@DriveDlg@.
Ztisknutím #Shift+Enter# na "#..#" otevřete v Průzkumníkovi aktuální
adresář.

 Změní na kořenový adresář                                   #Ctrl+\\#

 Změní adresář, vstup do archívu                  #Ctrl+[Shift+]PgDn#
 (také do SFX archívu)
 Pokud je kurzor na adresáři, stisknutím #Ctrl+PgDn# se přepnete do tohoto
adresáře. Pokud je kurzor na souboru, pak je v závislosti na typu souboru,
vykonán ~přiřazený příkaz~@FileAssoc@, nebo je otevřen archív.
#Ctrl+Shift+PgDn# always opens the archive, regardless of the associated
command configuration.

 Změní na rodičovský adresář                              #Ctrl+PgUp#
 The behavior in root folders depends on "~Use Ctrl+PgUp to change drive~@InterfSettings@" option.

 Vytvoří skratku na aktuální adresář                 #Ctrl+Shift+0…9#

 Použití adresářové skratky                           #RightCtrl+0…9#

 Nastavení ~atributů souboru~@FileAttrDlg@                                  #Ctrl+A#

 ~Aplikuje příkaz~@ApplyCmd@ na vybrané soubory                          #Ctrl+G#

 ~Popíše~@FileDiz@ vybrané soubory                                      #Ctrl+Z#


@DeleteFile
$ #Deleting and wiping files and folders#
 The following hotkeys are used to delete or wipe out files and folders:

 #F8#
 If any files or folders are selected in the panel then the selected group will be deleted, otherwise
the object currently under cursor will be deleted.

 #Shift+F8#
 Delete only the file under cursor (with no regard to selection in the panel).

 #Shift+Del#
 Delete selected objects, skipping the Recycle Bin.

 #Alt+Del#
 Wipe out files and folders.

 Remarks:

 1. ^<wrap>In accordance to ~System settings~@SystemSettings@ the hotkeys #F8# and
#Shift+F8# do or do not move the deleted files to the Recycle Bin. The
#Shift+Del# hotkey always deletes, skipping the Recycle Bin.

 2. ^<wrap>A file is wiped by first overwriting its data with zeroes
(a different value can be specified in
~System.WipeSymbol~@System.WipeSymbol@), then truncating to zero
length, following by renaming to a temporary name, and finally deleting.


@ErrCannotExecute
$ #Error: Cannot execute#
 The program you tries to execute is not recognized as an internal or
external command, operable program or batch file.


@Grabber
$ #Snímač obrazu#
 Snímač obrazu                                              #Alt+Ins#

 Snímač obrazu umožňuje vybrat a zkopírovat do schránky oblast obrazovky.

 To switch between stream and block selection mode use the #Space# key.
 Použijte kurzorové šipky, nebo klikněte levým tlačítkem myši pro přesunutí kurzoru.
 Pro označení textu použijte #Shift+kurzorové šipky#, nebo táhněte myší
se stisknutým #levým tlačítkem myši#.
 To extend or shrink selected area use #Alt+Shift+arrow# keys.
 To move selected area use #Alt+arrow# keys.
 #Enter#, #Ctrl+Ins#, pravé tlačítko myši, nebo dvojklik zkopíruje
text do schránky, #Ctrl+ <Num +># ho připojí k obsahu schránky,
 #Esc# opustí snímací mód.
 #Ctrl+A# - select whole screen.
 #Ctrl+U# - deselect block.
 #Ctrl+Shift+Left# and #Ctrl+Shift+Right# - extend or shrink selection by 10 characters left or right.
 #Ctrl+Shift+Up# and #Ctrl+Shift+Down# - extend or shrink selection by 5 lines up or down.


@MiscCmd
$ #Různé#
 #Keyboard macros#

 Nahrávání ~klávesových maker~@KeyMacro@                               #Ctrl+<.>#

 #Menus and dropdown lists#

 Enable/disable filtering mode                     #RAlt, Ctrl+Alt+F#
 Lock/unlock filter                                      #Ctrl+Alt+L#

 When in filter mode, you can filter the displayed items by entering
text.

 #Dialogs#

 Historie v editačních dialozích                 #Ctrl+Up, Ctrl+Down#

 V historii editačních dialogů můžete použít #Enter# pro zkopírování aktuální
položky do editačního dialogu a #Ins# pro označení nebo odznačení některé položky.
Označené položky nejsou z historie odstraněny novými a díky tomu můžete mít v
historii vždy ty nejpoužívanější řetězce.

 Vymazání historie v editačních dialozích                       #Del#

 Vymaže aktuální položku v seznamu historie editačních    #Shift+Del#
 dialogů (pokud to není zakázáno)

 Set the dialog focus to the first element                     #Home#

 Nastavuje fokus dialogu na implicitní element            #PgDn, End#

 The #Home# and #End# keys move the focus if it is currently not
on a control which handles these keys internally, like edit control.

 Tyto klávesové kombinace jsou platné pro všechny editační dialogy, včetně
~interního editoru~@Editor@, vyjma příkazové řádky.

 Vložit název souboru pod kurzorem do dialogu           #Shift+Enter#

 Vloží název souboru z pasivního panelu do dialogu #Ctrl+Shift+Enter#

 Stisknutí #Ctrl+Enter# v dialogu vykoná implicitní akci (stiskne
implicitní tlačítko, nebo nějakou podobnou věc).

 V dialogu, pokud je ovládání check box:

 - zapnuto (#[x]#)                                              #Num +#
 - vypnuto (#[ ]#)                                              #Num -#
 - změna na nedefinováno (#[?]#)                                #Num *#
   (pro třístupňové checkboxy)

 You can move a dialog (window) by dragging it with mouse or by pressing #Ctrl+F5# and using #arrow# keys.

 #Levý klik# mimo dialog funguje stejně jako stisknutí klávesy #Esc#.

 #Pravý klik# mimo dialog funguje stejně jako stisknutí klávesy #Enter#.

 #Mouse#

 Kliknutím postředního tlačítka myši v ~panelech~@PanelCmd@ má stejný
efekt jako stisknutí klávesy #Enter#, lze použít i stejné kombinace (#Ctrl#, #Alt#,
#Shift#). Pokud není ~příkazový řádek~@CmdLineCmd@ prázdný, bude vykonán příkaz,
který obsahuje.

  Podporuje manažer Far ~kolečko myši~@MsWheel@.


@SpecCmd
$ #Special commands#
 ~Version information~@FarAbout@
 ~Configuration editor~@FarConfig@


@MsWheel
$ #Podpora kolečka myši#
   #Panely#
 Rotací kolečka skrolujete seznamem souborů bez změny pozice kurzoru na obrazovce.
Stisknutí prostředního tlačítka má stejný efekt, jako stisk klávesy #Enter#.

 #Editor#
 Rotací kolečka skrolujete text bez změny pozice kurzoru na obrazovce
(obdoba Ctrl+Nahoru/Ctrl+Dolů).

 #Prohlížeč, Nápověda#
 Rotací kolečka skrolujete text.

 #Help#
 Rotating the wheel scrolls the text.

 #Menu#
 Skrolování kolečkem pracuje stejně jako tlačítka #Nahoru#/#Dolů#.
Stiskuní #prostředního tlačítka# má stejný efekt jako stisknutí
klávesy #Enter#. Je možné zvolit položky bez posuvu kurzoru.

 #Dialogy#
 Pokud použijete kolečko v dialozích na editačním řádku se
seznamem historie, nebo combo boxem, otevře se drop-down
seznam. Skrolování v tomto seznamu probíhá stejně jako v menu.

 Počet najednou skrolovaných řádků v panelech, editoru a prohlížeči
lze nastavit (pro bližší informace se podívejte do ~System.MsWheelDelta~@System.MsWheelDelta@).


@Plugins
$ #Podpora pluginů#
 Externí DLL moduly (pluginy) mohou být použity pro implementaci nových
příkazů do Faru a emulaci souborových systémů. Například, podpora archívů,
FTP klient, dočasný panel a síťový prohlížeč jsou pluginy, které emulují
souborové systémy.

 Všechy pluginy jsou uloženy v oddělených adresářích v adresáři 'Plugins',
který se nachází ve stejném adresáři, jako Far.exe, and the 'Plugins' folder, which is in the
user profile folder (#%APPDATA%\\Far Manager\\Profile# by default).
Pokud Far zjistí nový plugin, uloží o něm informace a později ho načte pouze tehdy když bude potřeba,
takže nepoužívané pluginy nezabírají další paměť. Pokud jste si jisti, že
některé pluginy nepotřebujete, můžete je pro ušetření místa na disku smazat.

 Pluginy mohou být volány buď z ~menu změny jednotky~@DriveDlg@, nebo
z menu ~Příkazy pluginů~@PluginCommands@, spouštěném klávesou #F11#, nebo odpovídající
položkou z ~Příkazového menu~@CmdMenu@. #F4# v menu "Příkazy pluginů"
umožňuje přiřadit položce horkou klávesu (to dělá snadnější volání pluginů
pomocí ~klávesových maker~@KeyMacro@). Toto menu je přístupné ze souborových
panelů a z interního prohlížeče a editoru (jen pomocí #F11#). Při volání menu
z prohlížeče a editoru budou zobrazeny pouze s peciálně navržené pluginy.

 Parametry pluginů můžete nastavit pomocí menu ~Nastavení pluginů~@PluginsConfig@
z menu ~Nastavení~@OptMenu@.

 Souborové operace, jako kopírování, přesouvání, mazaní, úpravy, nebo
~Hledání souboru~@FindFile@ pracují s pluginy, které emulují souborové
systémy tehdy, pokud tyto pluginy zajistí nezbytnou funkčnost. Hledání z
aktuálního adresáře v "Hledat soubor" vyžaduje menší funkčnost, než hledání
z kořenového adresáře, takže ho použijte, pokud hledání z kořenového
adresáře nefunguje korektně.

 Moduly mají své vlastní soubory zpráv a nápovědy. Obsah nápovědy o
modulech můžete získat pokud stisknete

 #Shift+F2# - kdekoliv v systému nápovědy Faru

  #Shift+F1# - seznamu pluginů.

 Pokud plugin nemá soubor nápovědy, pak se kontextově-závislá nápověda
nezobrazí.

 Když panel zobrazí pluginem emulovaný souborový systém, pak může být
příkaz "CD" z příkazové řádky použit pro změnu adresáře souborového systému
pluginu. Narozdíl od normálních souborových systémů, kde příkazy "CD", "CHDIR"
vždy zpracovávají zadané parametry jako opravdové názvy adresářů, bez ohledu
na typ souborového panelu.

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
$ #Nastavení Pluginů#
 Nainstalované ~plugin moduly~@Plugins@ můžete nakonfiurovat pomocí
příkazu #"Nastavení pluginů"# z menu ~Nastavení~@OptMenu@ or by pressing
#Alt+Shift+F9# in the ~Change drive menu~@DriveDlg@ or plugins menu.

 Pro získání nápovědy o aktuálně vybraném pluginu, stiskněte #Shift+F1#
- kontextově-závislá nápověda o konfiguraci pluginu. Pokud plugin nemá
soubor nápovědy, nebude kontextově-závislá nápověda zobrazena.

 Pokud je vyvolána kontextově-závislá nápověda, pokusí se Far zobrazit
téma #Config#. Pokud téma v souboru nápovědy pluginu neexistuje,
bude zobrazeno hlavní téma nápovědy pluinu.

 Stisknutí klávesy #F4# v menu #"Nastavení pluginů"# umožní přiřadit položkám v tomto menu zkratkové klávesy,
které je umožní později vyvolat pomocí ~příkazů klávesových maker~@KeyMacro@. The assigned hotkey is
displayed left to the item. The #A# character in the leftmost menu column means that
the corresponding plugin is written for Far 1.7x and it does not support all
possibilities available in Far 3 (these are, in particular, Unicode characters
in filenames and in editor).

 Pressing #F3# will show some technical information about the plugin.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@PluginsReviews
$ #Přehled schopností pluginů#
 Manažer Far je tak úzce integrován se svými pluginy, že nemá smysl
jednoduše hovořit o Faru a nemzmínit se o pluginech.
Pluginy představují téměř neomezenou možnost rozšíření schopností Faru.

 Bez zacházení do detailů, některé možné zajímavé schopnosti:

 * ^<wrap>Zvýrazňování syntaxe ve zdrojových textexh programů.
 * Práce s FTP-servery (zahrnující přístup přes proxy).
 * Vyhledávání a nahrazování v mnoha souborech v ten samý čas, použitím regulárních výrazů.
 * Přejmenovávání skupin souborů s podporou pro komplexní skládání masek sestávajících z náhradních symbolů a schémat.
 * NNTP/SMTP/POP3/IMAP4 klienty a zasílání zpráv na pager.
 * Funkčnost s nestandardními rozlišeními obrazovky.
 * Konverze textu z jedné národní znakové sady na jinou.
 * Manipulace s obsahem Koše.
 * Ovládání priority procesů na lokálních, nebo síťových PC.
 * Autodoplňování slov v editoru a práce se schématy.
 * Úprava registrů systému Windows.
 * Vytváření a modifikace zástupců Windows.
 * Souborové a textové operace dělající více komfortní užívání FidoNetu.
 * UU-kódování a UU-dekódování souborů.
 * Ovládání Winampu a modifikace MP3-tagů.
 * Zpracování Quake PAK-souborů.
 * Ovládání tiskáren, připojených do PC a sitě.
 * Připojení a debugging front do databází kompatibilních s ODBC.
 * Ovládání služeb RAS.
 * Vykonávání externích programů (kompilátory, konvertory atd.), pokud upravujete text v editoru Faru.
 * Zobrazování obsahu souborů nápovědy Windows (.hlp a .chm)
 * Kalkulátory s různými schopnostmi.
 * Několik her :-)
 * Kontrola pravopisu při úpravě textu v editoru Faru.
 * Úprava seznamu výměnných jednotek a mnoho dalších…

 Jako zdroje informací, které mohou být použity pro vyhledání
specifických pluginů mohu doporučit:

 - Far Manager official site
   ~https://www.farmanager.com~@https://www.farmanager.com@
 - Online forum
   ~https://forum.farmanager.com~@https://forum.farmanager.com@
 - Registration and handling of problems
   ~https://bugs.farmanager.com~@https://bugs.farmanager.com@
 - Stránku PlugRinG
   ~http://plugring.farmanager.com~@http://plugring.farmanager.com@
 - Bezplatné skupinové email služby
   ~https://groups.google.com/group/fardeven/~@https://groups.google.com/group/fardeven@
 - USENET echo konferenci
   ~news:fido7.far.support~@news:fido7.far.support@
   ~news:fido7.far.development~@news:fido7.far.development@
 - FidoNet echo konferenci
   far.support
   far.development


@Panels
$ #Panely#
 Normálně Far zobrazuje dva panely (levé a pravé okno), s různými
informacemi. Pokud si přejete změnit typ informací zobrazených v panelu,
použijte ~menu panelu~@LeftRightMenu@, nebo odpovídající ~klávesový příkaz~@KeyRef@.

 Pro získání výce informací se podívejte na následující témata:

 ~Panel souborů~@FilePanel@
 ~Panel stromu~@TreePanel@
 ~Panel Info~@InfoPanel@
 ~Panel zběžného zobrazení~@QViewPanel@

 ~Drag and drop (uchop a táhni) soubory~@DragAndDrop@
 ~Výběr souborů~@SelectFiles@
 ~Úprava módů souborových panelů~@PanelViewModes@


@FilePanel
$ #Souborový panel#
 Souborový panel, zobrazující aktuální adresář.
Můžete označovat a odznačovat soubory a adresáře, provádět různé souborové
a archivní operace. Číst ~Klávesové odkazy~@KeyRef@ pro seznam příkazů.

 Implicitní módy zobrazení souborových panelů jsou:

 #Stručný#       Soubory jsou zobrazeny ve třech sloupcích.

 #Střední#       Soubory jsou zobrazeny ve dvou sloupcích.

 #Plný#          Zobrazuje názvy souborů, velikost, datum a čas.

 #Široký#        Zobrazuje názvy souborů a velikost.

 #Detailní#      ^<wrap>Zobrazuje názvy souborů, velikost, komprimovanou
velikost, datup poslední modifikace, vytvoření, přístupu a atributy. Celoobrazovkový mód.

 #Popisový#      Zobrazuje názvy souborů a ~popisy souborů~@FileDiz@

 #Dlouhý#        Zobrazuje názvy souborů, velikost a popisy.
 #popisový#      Celoobrazovkový mód.

 #Vlastník soub# Zobrazuje názvy souborů, velikost a vlastníka souborů.

 #Linkový#       Zobrazuje názvy souborů, velikost a čísla pevných linků.

 #Alternativní#  Zobrazuje názvy souborů, velikost (formátovanou čárkami)
 #plný#          a datum vytvoření souborů.

 ~Zobrazení souborových panelů můžete nastavovat~@PanelViewModes@.

 Majitelé souborů a čísla pevných linků jsou myšleny
pouze pro NTFS. Některé souborové systémy nemusí podporovat datum vytvoření
souboru a datum přístupu.

 Pokud si přejete změnit mód zobrazení panelu, udělejte to z
~menu panelů~@LeftRightMenu@. Pokud je zobrazen jiný panel, než
souborový, potom je po změně módu nebo jednotky automaticky
přepnut na souborový.

 ~Rychlé hledání~@FastFind@ může být použito pro dosažení požadovaného souboru
pomocí prvního znaku jeho názvu.

 See also the list of ~macro keys~@KeyMacroShellList@, available in the panels.


@TreePanel
$ #Panel Strom#
 Panel Strom zobrazuje stuktůru adresářů daného disku, jako strom.
V módu stromu můžete rychle měnit adresáře a provádět souborové
operace.

 Far ukádá informace o adresářové struktůře do souboru s názvem #tree3.far#,
do kořenového adresáře každé jednotky. Pro jednotky určené pouze pro čtení
jsou tyto informace uloženy ve skrytém adresáři Tree.Cache v adresáři s Far.exe. The tree3.far
file doesn't exist by default. It will be automatically created after the first
use of the #Tree Panel# or the #Find Folder# command. If that file exists, Far
updates it with the changes to the tree structure it is aware of. If such
changes were made outside of Far and Tree.far is no longer current, it can be
refreshed by pressing #Ctrl+R#.

 Nejrychleji můžete najít požadovaný adresáře pomocí #rychlého hledání#.
Stiskněte klávesu Alt a pište název adresáře, dokud není požadovaný adresář
vybrán. Současným stisknutím kláves #Ctrl+Enter# vyberete další, se stejným názvem.

 #Gray +# and #Gray -# keys move up and down the tree to the next branch
on the same level.

 See also the list of ~macro keys~@KeyMacroTreeList@, available in the folder tree panel.


@InfoPanel
$ #Info panel#
 Informační panel obsahuje následující data:

 1. ^<wrap>#síťové# názvy počítače a současného uživatele (see ~Info panel settings~@InfoPanelSettings@);

 2. ^<wrap>název a typ #zvoleného disku#, typ souborového systému, název
sítě, celkové a volné místo, název disku a jeho sériové číslo;

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
    ^<wrap>procentuální zatížení #paměti# (100% znamená, že všechna dostupná paměť
se využívá), celkové a volné místo fyzické paměti
(available for Windows), virtual memory and paging file;

 4. ^<wrap>soubor #popisu adresáře#
    ^<wrap>Popis adresáře můžete zobrazit v celoobrazovkovém režimu stisknutím
klávesy #F3#, nebo kliknutím levého tlačítka myši. Pro úpravu, nebo
vytvoření popisového souboru, stiskněte #F4#, nebo klikněte pravým tlačítkem
myši. Pro zobrazení popisového souboru adresáře můžete rovněž použít mnoho
~příkazů prohlížeče~@Viewer@ (hledání, výběr znakové sady atd.).
    ^<wrap>Seznam souborů používaných jako popisy můžete definovat a upravit
v "Soubory popisů adresářu" v ~menu Nastavení~@OptMenu@.

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
$ #Panel zběžné zobrazení#
 Panel zběžného zobrazení je používán pro zobrazení informací o položkách
v ~panelu souborů~@FilePanel@, nebo ~panelu Stromu~@TreePanel@.

 Pokud je vybranou položkou soubor, je zobrazen jeho obsah.
Many of the ~internal viewer~@Viewer@ commands can be used with the file
displayed in the panel. Soubory registrovaných typů Windows jsou zobrazeny jako zdroj.

 Pokud je vybranou položkou adresář, je zobrazena jeho velikost, celková
komprimovaná velikost, počet souborů a podadresářů, velikost clusterů,
opravdová velikost a mrtvé místo (součet nevyužitých částí clusterů).

 Pokud provedete ve zběžné zobrazení na ~oddělovacích tečkách~@HardSymLink@, bude také zobrazena cesta do zdrojového adresáře.

 For folders, the total size value may not match the actual value:
 - ^<wrap>If the folder or its subfolders contain symbolic links and the option
"Scan symbolic links" in the ~System settings~@SystemSettings@ dialog is enabled.
 - ^<wrap>If the folder or its subfolders contain multiple hard links to the same file.

 #Ctrl+Shift+S# toggles size display mode: float with size suffixes or bytes.
~Info panel~@InfoPanel@ and ~file panel~@FilePanel@ status line also affected.
Current mode - far:config #Panel.ShowBytes# (default=false).

 See also the list of ~macro keys~@KeyMacroQViewList@, available in the quick view panel.


@DragAndDrop
$ #Drag and drop soubory#
 Je možné provádět #Kopírování# a #Přesouvání# souborů pomocí
#drag and drop# (uchopení souboru myší a přetáhnutím na cílové místo).
Stiskněte levé tlačítko myši na souboru nebo adresáři, který chcete
zkopírovat/přesunout, přetáhněte ho do druhého panelu a pusťte tlačítko.

 Pokud chcete pracovat se skupinou souborů, nebo adresářů, vyberte je,
klikněte levým tlačítkem myši na zdrojový panel a přetáhněte skupinu souborů
do druhého panelu.

 Mezi kopírováním a přesouváním můžete přepínat pravým tlačítkem myší,
během přetahování. Pokud chcete soubory přesouvat, můžete rovněž podržet
klávesu #Shift#, spolu s levým tlačítkem myši.


@Menus
$ #Menu#
 Akci z menu můžete zvolit stisknutím klávesy F9, nebo kliknutím na horní
část obrazovky.

 Pokud je menu aktivováno stiskem klávesy #F9#, vybere se automaticky
menu pro aktivní panel. Když je menu aktivní, stiskem klávesy #Tab#,
přepínáte mezi menu pro levý a pravý panel. Pokud je aktivní něteré z menu
"Soubory", "Příkazy" nebo "Nastavení", stiskem klávesy #Tab# přepínáte do
menu pro pasivní panel.

 Pokud chcete použít naposledy zvolený příkaz z menu, stiskněte #Shift+F10#.

 Pro informace o jednotlivých položkách si přečtěte následující témata:

 ~Menu Levý a Pravý~@LeftRightMenu@
 ~Menu Soubory~@FilesMenu@
 ~Menu Příkazy~@CmdMenu@
 ~Menu Nastavení~@OptMenu@

 See also the list of ~macro keys~@KeyMacroMainMenuList@, available in the main menu.


@LeftRightMenu
$ #Menu Levý a Pravý#
 Menu #Levý# a #Pravý# umožňuje samostatně měnit nastavení levého
a pravého panelu souborů. Tyto menu obsahují následující položky:

 #Stručný#              Soubory jsou zobrazeny ve třech sloupcích.

 #Střední#              Soubory jsou zobrazeny ve dvou sloupcích.

 #Plný#                 Zobrazuje názvy souborů, velikost, datum a čas.

 #Široký#               Zobrazuje názvy souborů a velikost.

 #Detailní#             ^<wrap>Zobrazuje názvy souborů, velikost,
komprimovanou velikost, datup poslední modifikace, vytvoření, přístupu a atributy.
Celoobrazovkový mód.

 #Popisový#             Zobrazuje názvy souborů a ~popisy souborů~@FileDiz@.

 #Dlouhý popisový#      ^<wrap>Zobrazuje názvy souborů, velikost a popisy. Celoobrazovkový mód.

 #Vlastník soub#        Zobrazuje názvy souborů, velikost a vlastníka
                        souborů.

 #Linkový#              ^<wrap>Zobrazuje názvy souborů, velikost a čísla pevných linků.

 #Alternativní plný#    ^<wrap>Zobrazuje názvy souborů a formátovanou velikost.

 #Panel Info#           Změna panelu na ~info panel~@InfoPanel@.

 #Panel Strom#          Změna panelu na ~panel stromu~@TreePanel@.

 #Zběžné zobrazení#     Změna panelu na ~zběžné zobrazení~@QViewPanel@.

 #Módy třídění#         Zobrazí dostupné módy třídění.

 #Ukázat dlouhé názvy#  Zobrazí dlouhé/krátké názvy souborů.

 #Panel Zap/Vyp#        Ukáže/skryje panel.

 #Obnovit#              Znovu načte panel.

 #Změnit jednotku#      Změní aktuální jednotku.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@FilesMenu
$ #Menu Soubory#
 #Zobrazit#           ~Prohlížení~@Viewer@ souborů, výpočet velikosti adresáře.

 #Upravit#            ~Úprava~@Editor@ souborů.

 #Kopírovat#          ~Kopírování~@CopyFiles@ souborů a adresářů.

 #Přejmenuj / Přesuň# ~Přejmen. nebo přesouvání~@CopyFiles@ souborů a adresářů.

 #Link#               Create ~file links~@HardSymLink@.

 #Vytvořit adresář#   ~Vytvoření~@MakeFolder@ nového adresáře.

 #Smazat#             Smazání souborů a adresářů.

 #Wipe#               ^<wrap>Wipe files and folders. Before file deletion
its data are overwritten with zeros, after which the file is truncated and renamed to
a temporary name.

 #Přidat do archívu#  Přidání vybraných souborů do archívu.

 #Rozbalit soubory#   Rozbalení vybraných souborů z archívu.

 #Příkazy archívu#    Zobrazení příkazů pro práci s archívy.

 #Atributy souboru#   ~Změna atributů souboru~@FileAttrDlg@ a času.

 #Aplikovat příkaz#   ~Aplikace příkazů~@ApplyCmd@ na vybrané soubory.

 #Popsat soubory#     Přidání ~popisu~@FileDiz@ o vybraných souborech.

 #Označit skupinu#    ~Označení~@SelectFiles@ skupiny souborů pomocí značek.

 #Odznačit skupinu#   ~Odznačení~@SelectFiles@ skupiny souboru pomocí značek.

 #Obrátit výběr#      ~Obrácení~@SelectFiles@ aktuálního výběru souborů.

 #Obnovit výběr#      ~Obnovení~@SelectFiles@ předešlého označení souborů po
                      provedení nenbo označení souborů.

   Někeré příkazy z tohoto menu jsou také popsány v tématu
~Správa souborů a příkazy služeb~@FuncCmd@.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@CmdMenu
$ #Menu Příkazy#
 #Hledat soubor#        Vyhledávání souborů v adresářovém stromu,
                      mohou být použity i značky.
                      Pro více informací se podívejte na
                      ~Hledat soubor~@FindFile@.

 #Historie#             Zobrazuje předešlé příkazy. Pro více
                      informací se podívejte na ~Historii~@History@.

 #Video mód#            Přepíná mezi 25 a 50 řádky obrazovky.

 #Hledat adresář#       Vyhledávání adresářů v adresářovém stromu.
                      Pro více informací se podívejte na
                      ~Hledat adresář~@FindFolder@.

 #Hist. zobr. souborů#  Zobrazí ~historii prohlížení a úprav~@HistoryViews@ souborů.

 #Historie adresářů#    Zobrazí ~historii změn~@HistoryFolders@ adresářů.

                      Položky v "Historii adresářů" a "Historii
                      zobrazení souborů" jsou po vybrání přesunuty
                      na konec seznamu. Shift+Enter vybere položku
                      bez změny její pozice.

 #Změnit panely#        Zamění levý a pravý panel.

 #Panely Zap/Vyp#       Ukáže/skryje oba panely.

 #Porovnat adresáře#    Porovnává obsahy adresářů.
                      Pro více informací se podívejte na
                      ~Porovnat adresáře~@CompFolders@.

 #Upr. menu uživatele#  Umožňuje upravit hlavní nebo lokání
                      ~uživatelské menu~@UserMenu@.
                      Můžete stisknout Ins pro vložení, Del pro
                      smazání a F4 pro úpravu záznamu.

 #Závislosti souborů#   Zobrazí seznam ~závislosti souborů~@FileAssoc@.
                      Můžete stisknout Ins pro vložení, Del pro
                      smazání a F4 pro úpravu závislostí.

 #Adresářové zkratky#   Zobrazí ~adresářové zkratky~@FolderShortcuts@.

 #Upr. třídění skupin#  Umožňuje upravit uživatelem definované
                      ~třídění skupin~@SortGroups@.

 #Filtr panelu souborů# Umožňuje ovládat obsah souborových panelů.
                      Pro detailní popis se podívejte na
                      ~Filtr panelu souborů~@FiltersMenu@.

 #Příkazy pluginů#      Zobrazí seznam ~příkazů pluginů~@Plugins@.

 #Seznam obrazovek#     Zobrazí seznam otevřených ~obrazovek~@ScrSwitch@

 #Seznam úloh#          Zobrazí ~seznam aktivních úloh~@TaskList@.

 #Hotplug devices list# Show ~hotplug devices list~@HotPlugList@.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@OptMenu
$ #Menu Nastavení#
 #Nastavení Systému#    Zobrazuje dialog ~nastavení systému~@SystemSettings@.

 #Nastavení Panelů#     Zobrazuje dialog ~nastavení panelů~@PanelSettings@.

 #Tree settings#         Shows ~Tree settings~@TreeSettings@ dialog.
                       Available only if ~Panel.Tree.TurnOffCompletely~@Panel.Tree.TurnOffCompletely@
                       parameter in ~far:config~@FarConfig@ is set to “false.”

 #Nastavení Rozhraní#   Zobrazuje dialog ~nastavení rozhraní~@InterfSettings@.

 #Nastavení Jazyka#     Výběr hlavního jazyka a jazyka nápovědy.
                       Pro uložení zvolených jazyku použijte
                       "Uložit nastavení".
                       You can ~customize UI elements~@CustomizingUI@ to you needs and taste.

 #Konfigurace Pluginů#  Konfigurace ~plugin~@PluginsConfig@ modulů.

 #Plugin manager#        Shows ~Plugin manager settings~@PluginsManagerSettings@ dialog.
 #settings#

 #Dialog settings#       Shows ~Dialog settings~@DialogSettings@ dialog.

 #Menu settings#         Shows ~Menu settings~@VMenuSettings@ dialog.

 #Command line settings# Shows ~Command line settings~@CmdlineSettings@ dialog.

 #AutoComplete settings# Shows ~AutoComplete settings~@AutoCompleteSettings@ dialog.

 #InfoPanel settings#    Shows ~InfoPanel settings~@InfoPanelSettings@ dialog.

 #Groups of file masks#  Shows ~Groups of file masks~@MaskGroupsSettings@ dialog.

 #Potvrzení#            Zapínání a vypínání ~potvrzování~@ConfirmDlg@ některých
                      operací.

 #Módy soub. panelů#    ~Úprava módů pohledu souborového panelu~@PanelViewModes@.

 #Popisy souborů#       Seznam souborů obsahujících ~popisy souborů~@FileDiz@.

 #Soubory popisů#       Shows ~Soubory popisů adresářů~@FolderDiz@ dialog.
 #adresářů#

 #Nastavení Prohlížeče# Nastavené externího a interního ~prohlížeče~@ViewerSettings@.

 #Nastavení Editoru#    Nastavení externího a interního ~editoru~@EditorSettings@.

 #Code pages#            Shows the ~Code pages~@CodePagesMenu@ menu.

 #Barvy#                Shows the ~Color groups~@ColorGroups@ menu.

 #Zvýraznění souborů#   Změny nastavení ~zvýraznění souborů~@Highlight@.

 #Uložit nastavení#     Uloží aktuální nastavení barev a rozvržení
                      obrazovky.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@ConfirmDlg
$ #Potvrzení#
 V dialogu #Potvrzení# můžete zapínat a vypínat nastavení
pro následující operace:

 - ^<wrap>přepsání cílových souborů, když provádíte kopírování souborů;
 - přepsání cílových souborů, když provádíte přesouvání souborů;
 - overwrite and delete files with "read only" atrtibute;
 - ~drag and drop~@DragAndDrop@ soubory;
 - mazání souborů;
 - mazání adresářů;
 - přerušení operace;
 - ~odpojení síťové jednotky~@DisconnectDrive@ z menu disků;
 - delete SUBST-disks from the Disks menu;
 - detach virtual disks from the Disks menu;
 - removal of USB storage devices from the Disks menu;
 - ~znovu nahrát~@EditorReload@ upravovaný soubor;
 - vymazání adresáře a seznamů historie příkazů prohlížení/editace;
 - konec Faru.


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
$ #Vytvoření adresáře#
 Tato funkce umožňuje vytvořit adresář. Ve vstupní řádce můžete použít
systémové proměnné, které jsou před vytvořením adresáře rozvynuty do svých
hodnot. Rovněž můžete v jeden čas vytvořit několikanásobně vnořené podadresáře:
jednoduše oddělíte názvy adresářů znakem zpětného lomítka. Například:

 #%UŽIVATELOVADOMÉNA%\%UŽIVATELOVOJMÉNO%\Složka3#

 The "#Link Type#" option allows to choose the link ~type~@HardSymLink@
("#directory junction#" or "#symbolic link#"). If selected, the #Target# field
should contain a target directory path.

 Pokud je povoleno nastavení "#Zpracovat více názvů#" je možné vytvořit
několik adresářů jednou operací. V tomto případě musí být názvy adresářů
odděleny znakem "#;#" nebo "#,#". Pokud je nastavení zapnuto a jméno
adresáře obsahuje znak "#;#" (nebo "#,#"), musí být celá cesta a název
adresáře uzavřeny v úvozovkách. Například, pokud uživatel vloží
#C:\\Foo1;"E:\\foo,2;";D:\\foo3#, budou vytvořeny adresáře s názvy "#C:\\Foo1#", "#E:\\foo,2;#"
 a "#D:\\foo3#".


@FindFile
$ #Hledat soubor#
 Tento příkaz umožňuje najít v adresářovém stromu jeden, nebo více
souborů a adresářů, odpovídajících jednomu, nebo více ~značkám~@FileMasks@ (oddělených
čárkami). To může být použito i se souborovými systémy emulovanými
~pluginy~@Plugins@.

 Můžete zadat nepovinný textový řetězec, pro nalezení jen souborů obsahujících
tento text. Pokud je řetězec zadán, nastavení #"Rozlišovat velikost písma"# nastaví
citlivost porovnávání.

 Nastavení #"Celá slova"# dovolí nalézt pouze text, který je oddělen
od ostatního textu mezerami, tabulátory, zalomeními, nebo
standardními oddělovači, které jsou: #!%^&*()+|{}:"<>?`-=\\[];',./#.

 #Fuzzy search# is diacritical insensitive, treats ligatures equivalent
to their corresponding multicharacter sequences and fancy numerals
to corresponding number characters, and ignores some other minor
differences.

 Seznam #"Použít znakovou sadu"# umožňuje zvolit konkrétní znakovou sadu
pro vyhledávání textu, nebo lze pro vyhledávání textu v souborech různými
znakovými sadami použít nastavení #Všechny znakové sady#, kdy jsou použity
všechny instalované znakové sady přístupné Faru.

 #Not containing# allows to find files #not# containing the specified text or code.

 Dropdown list #Using code page# allows to choose a specific code page
to be used for the search. If the item #All standard code pages# is selected
in the dropdown list, Far will use all standard code pages (ANSI, OEM, UTF-8,
UTF-16, UTF-16 BE), as well as #Favorite# code pages (the list of #favorite#
code pages can be specified in the ~Code pages~@CodepagesMenu@ menu
in the options, editor, or viewer). To search using a custom set of code pages,
select required code pages in the dropdown list with the #Ins# or #Space# keys,
then choose #Selected code pages# menu item.

 Pokud je nastavena položka #"Hledat v archívech"#, Far provádí vyhledávání
i v archívech známých formátů. Nicméně, použití tohoto nastavní podstatně
snižuje výkon vyhledávání. Far nemůže vyhledávat v archívech, které jsou
znovu zabaleny.

 Volba #"Hledat adresáře"# zahrnuje ve vyhledávacím seznamu ty adresáře,
které odpovídají souborovým značkám (wildcards). Počítadlo nalezených souborů
zobrazuje i počet nalezených adresářů.

 The #Search in symbolic links# option allows searching files in
~symbolic links~@HardSymLink@ along with normal sub-folders.

 #Search in alternate streams# - besides the primary data stream (which is
the content of the file itself), allows to search alternate named data streams
supported by some file systems (for example, #NTFS#).

 Vyhledávání může být prováděno:
 - ^<wrap>na všech pevných discích
 - in all local drives, except removable and network;
 - in all folders specified in the %PATH% environment variable (not including subfolders).
 - ve všech adresářích v kořenovém adresáři, in the find dialog one can change disk drive of the search;
 - z aktuálního adresáře;
 - pouze v aktuálním adresáři
 - nebo ve vybraných adresářích.

 Zdroj hledání je uložen v konfiguraci.

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
 Když je ~vyhledávání~@FindFile@ v běhu, nebo bylo dokončeno, můžete použít
kurzorové šipky pro skrolování seznamem nalezených souborů a tlačítky,
pro vykonání požadované akce.

 V průběhu, nebo po skončení vyhledávání jsou k dispozici následující tlačítka:

 #Nové hledání#    Spustí nový vyhledávácí dialog.

 #Jdi na#          Přeruší vyhledávání, změní adresářový panel
                 a přesune kurzor na vybraný soubor.

 #Zobraz#          Zobrazí vybraný soubor. Pokud nebylo vyhledávání
                 dokončeno, bude pokračovat na pozadí.

 #Panel#           Vytvoří dočasný panel a vyplní ho výsledky
                 posledního vyhledávání.

 #Stop#            Přeruší vyhledávání. K dispozici jen pokud je
                 vyhledávání v chodu.

 #Storno#          Ukončí vyhledávací dilog.

 During or after search you can view or edit found files.

 View                          #F3, Alt+F3, Numpad5, Ctrl+Shift+F3#

 #F3#, #Alt+F3# or #Numpad5# invokes ~internal~@Viewer@, external or ~associated ~@FileAssoc@ viewer,
depending on file type and ~Viewer settings~@ViewerSettings@.
#Ctrl+Shift+F3# always invokes internal viewer regardless of file associations.

 Edit                                    #F4, Alt+F4, Ctrl+Shift+F4#

 #F4# or #Alt+F4# invokes ~internal~@Editor@, external or ~associated~@FileAssoc@ editor,
depending on file type and ~Editor settings~@EditorSettings@.
#Ctrl+Shift+F4# always invokes internal editor regardless of file associations.

 Prohlížení a úprava jsou podporovány i pro souborové systémy
emulované pluginy. Poznámka, uložení změn v editoru klávesou F2 v emulovaných
souborových systémech provede příkaz #Uložit Jako#, místo klasického #Uložit#.


@FindFolder
$ #Hledat adresář#
 Tento příkaz umožňuje rychle vyhledat požadovaný adresář v adresářovém stromu.

 Pro výběr adresáře můžete použít kurzorové šipky, nebo
vepište první znaky názvu adresáře.

 Pro přepnutí do vybraného adresáře stiskněte #Enter#.

 #Ctrl+R# a #F2# způsobí znovunačtení adresářového stromu.

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
$ #Historie#
 Příkaz historie zobrazí seznam dříve vykonaných příkazů.
Kromě ovládání kurzorovými šipkami jsou k dispozici tyto
klávesové zkratky:

 Provede dříve vykonaný příkaz                                 #Enter#

 Provede dříve vykonaný příkaz v novém okně              #Shift+Enter#

 Re-execute a command as administrator                #Ctrl+Alt+Enter#

 Zkopíruje příkaz do příkazové řádky                      #Ctrl+Enter#

 Vymaže historii příkazů                                         #Del#

 Lock/unlock the current history item                            #Ins#

 Delete the current history item                           #Shift+Del#

 Zkopíruje text příkazu do schránky, bez uzavření             #Ctrl+C#
 seznamu historie                                      nebo #Ctrl+Ins#

 Show additional information                                      #F3#

 Pro přepnutí předešlého, nebo dalšího příkazu z příkazové řádky
můžete stisknout #Ctrl+E# respektive #Ctrl+X#.

 Pokud chcete před ukončením Faru historii příkazů ukládat, použijte
nastavení v dialogu ~nastavení systému~@SystemSettings@.

 Locked items will not be deleted when clearing the history.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@HistoryViews
$ #Historie zobrazování a úpravy souborů#
 Historie zobrazování souborů zobrazuje seznam souborů, které byly
v poslední době prohlíženy, nebo upravovány. Kromě kurzorových kláves
můžete používat následující zkratkové klávesy:

 Znovu otevřít soubor pro prohlížení, nebo úpravu              #Enter#

 Kopírovat název souboru do příkazové řádky               #Ctrl+Enter#

 Vymazat seznam historie                                         #Del#

 Delete the current history item                           #Shift+Del#

 Lock/unlock the current history item                            #Ins#

 Refresh list and remove non-existing entries                 #Ctrl+R#

 Zkopíruje text aktuální položky historie do schránky         #Ctrl+C#
 bez uzavření seznamu historie.                        nebo #Ctrl+Ins#

 Open a file in the ~editor~@Editor@                                        #F4#

 Open a file in the ~viewer~@Viewer@                             #F3# or #Numpad5#

 Po vybrání jsou položky v historii přesunuty na konec seznamu.
Pro vybrání položky bez změny pozice můžete použít kombinaci kláves
#Shift+Enter#.

 Pokud chcete před ukončením Faru historii příkazů ukládat, použijte
nastavení v dialogu ~nastavení systému~@SystemSettings@.

  Postřeh:

 1. ^<wrap>List refresh operation (Ctrl+R) can take a considerable amount
of time if a file was located on a currently unavailable remote resource.
 2. ^<wrap>Locked items will not be deleted when clearing or refreshing the history.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@HistoryFolders
$ #Historie adresářů#
 Historie adresářů zobrazuje seznam adresářů, které byly v poslední
době navštíveny. Kromě kurzorových kláves můžete použít následující klávesové zkratky:

 Jít do aktuálního adresáře v seznamu                          #Enter#

 Open the selected folder in the passive panel      #Ctrl+Shift+Enter#

 Zkopírovat název adresáře do příkazové řádky             #Ctrl+Enter#

 Vymazat seznam historie                                         #Del#

 Delete the current history item                           #Shift+Del#

 Lock/unlock the current history item                            #Ins#

 Refresh list and remove non-existing entries                 #Ctrl+R#

 Zkopírovat text aktuální položky historie do                 #Ctrl+C#
 schránky, bez uzavření seznamu                        nebo #Ctrl+Ins#

 Po vybrání jsou položky v historii přesunuty na konec seznamu.
Pro vybrání položky bez změny pozice můžete použít kombinaci kláves
#Shift+Enter#.

 Pokud chcete před ukončením Faru historii příkazů ukládat, použijte
nastavení v dialogu ~nastavení systému~@SystemSettings@.

 Postřeh:

 1. ^<wrap>List refresh operation (Ctrl+R) can take a considerable amount
of time if a folder was located on a currently unavailable remote resource.
 2. ^<wrap>Locked items will not be deleted when clearing or refreshing the history.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@TaskList
$ #Seznam úloh#
 Seznam úloh zobrazuje aktivní úlohy. Každá řádka seznamu
znamená jedno okno běžící úlohy.

 Ze seznamu úloh se můžete přepnout okna úlohy, nebo ji zabít
(ukončit) klávesou #Del#. Při ukončování úloh buďte opatrní. Při
okamžitém ukončení úlohy budou všechna neuložená data ztracena,
proto by se tato funkce měla používat pouze tehdy, pokud je to
nezbytné, např. pro přerušení programu, který neodpovídá.

 Seznam úloh může být volán buď z ~menu Příkazy~@CmdMenu@, nebo
použitím #Ctrl+W#. Klávesová zkratka Ctrl+W může být použita i v
prohlížeči nebo editoru.

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
$ #Porovnat adresáře#
 Příkaz porovnat adresáře je použitelný pouze tehdy, pokud jsou
zobrazeny oba ~panely souborů~@FilePanel@. Porovná obsahy adresářů
zobrazených ve dvou panelech. Subory, existující pouze v jednom panelu,
nebo ty, které mají jiné datum než soubory se stejným názvem
v druhém panelu, jsou označeny.

 Podadresáře nejsou porovnávány. Soubory jsou porovnávány pouze
podle názvu, velikosti a času. Obsah souborů nemá na porovnvání vliv.


@UserMenu
$ #Menu uživatele#
 Uživatelské menu existuje pro usnadnění volání často používaných
operací. Obsahuje množství uživatelem definovaných příkazů a příkazových
sekvencí, které mohou být po vyvolání menu spuštěny. Menu může obsahovat
podmenu. ~Speciální symboly~@MetaSymbols@ jsou podporovány jak v
příkazech, tak i v názvech příkazů. Poznámka, tento symbol
!?<název>?<řetězec>! může být použit pro zadání dodatečných parametrů
přímo před vykonáním příkazu.

 Vytvořit, nebo upravit hlavní, nebo lokální uživatelské menu je možné
pomocí #Upravit menu uživatele# z ~menu Příkazy~@CmdMenu@. Může být pouze
jedno hlavní menu. Pokud není pro danou složku lokální menu, je voláno
menu hlavní. Lokální menu může být umístěno v kterémkoliv adresáři. Mezi
hlavním a lokálním menu můžete přepínat pomocí Shift+F2. Uživatelské
menů rodičovského adresáře můžete zavolat stisknutím BackSpace.

 Do uživatelského menu můžete vložit i oddělovače. Pro vytvoření oddělovače
přidejte nový příkaz, jako "horkou klávesu" zvolte "--" a pole "Popis" ponechte
prázdné. Pro smazání oddělovače se musíte přepnout do souborového módu pomocí #Alt+F4#.

 User menu items can be moved with #Ctrl+Up# or #Ctrl+Down# key combinations.

 Pro vykonání příkazu v uživatelském menu, zvolte požadovaný příkaz
kurzorovými šipkami a stiskněte Enter. Také můžete stisknout "horkou klávesu",
přiřazenou danému příkazu.

 Smazat podmenu, nebo položku v menu můžete klávesou Del. Vložit nové
podmenu, nebo položku v menu můžete klávesou Ins. Upravit existující
podmenu, nebo položku v menu můžete klávesou F4. Pokud chcete menu upravovat
v podobě textového souboru stiskněte  Alt+F4.

 V uživatelském menu je možné použít jako zkratkové klávesy číslice,
písmena a funkční klávezy (#F1#…#F24#). Pokud jsou použity  #F1# nebo #F4#, je
jejich původní funkce v menu potlačena. Nicméně, můžete pro úpravu menu
použít #Shift+F4#.

 Když tvoříte, nebo upravujet položku v menu, měli byste zadat horkou
klávesu, pro rychlý přístup k položce, popis položky, který bude v menu
zobrazen a sekvenci příkazů, která má být vykonána.

 Když tvoříte, nebo upravujete podmenu, měli byste zadat horkou klávesu
a popis položky.

 Lokální uživatelské menu jsou uložena v textových souborech #FarMenu.Ini#:
 - ^<wrap>Global user menu, by default, is located in the Far Manager folder.
If global user menu file exists it will override the user specific menu.
 - User specific user menu is located in the user profile.
 - Local user menu is located in the current folder.

 To close the menu even if submenus are open use #Shift+F10#.

 See also:

 The list of ~macro keys~@KeyMacroUserMenuList@, available in the user menu.
 Common ~menu~@MenuCmd@ keyboard commands.


@FileAssoc
$ #Závislosti souborů#
 Manažer Far podporuje závislosti souborů, které umožňují asociovat
různé akce spouštění, prohlížení a upravování souborů se specifickými
~maskami~@FileMasks@.

 Přidat nové asociace můžete pomocí příkazu #Závislosti souborů#
v menu ~Příkazy~@CmdMenu@.

 Pro jeden typ souboru můžete definovat několik asociací a požadovanou
asociaci vybrat z menu.

 Následující akce jsou k dispozici v seznamu asociací:

 #Ins#        - ~přidat~@FileAssocModify@ novou asociaci

 #F4#         - ~upravit~@FileAssocModify@ aktuální asociaci

 #Del#        - smazat aktuální asociaci

 #Ctrl+Up#    - move association up

 #Ctrl+Down#  - move association down

 See also: common ~menu~@MenuCmd@ keyboard commands.


@FileAssocModify
$ #Úprava závislosti souborů#
 Far umožňuje nastavit šest příkazů asociovaných s každým typem
souboru, specifikovaných jako ~maska~@FileMasks@:

 #Vykonat příkaz#                Použito po stisknutí Enteru
 #(použito pro Enter)#

 #Vykonat příkaz#                Použito po stisknutí Ctrl+PgDn
 #(použito pro Ctrl+PgDn)#

 #Příkaz Zobraz#                 Použito po stisknutí F3
 #(použito pro F3)#

 #Příkaz Zobraz#                 Použito po stisknutí Alt+F3
 #(použito pro Alt+F3)#

 #Příkaz Uprav#                  Použito po stisknutí F4
 #(použito pro F4)#

 #Příkaz Uprav#                  Použito po stisknutí Alt+F4
 #(použito pro Alt+F4)#

 Závislost může být popsána v poli #Popis závislosti#.

 V asociovaných příkazech mohou být použity následující
~speciální symboly~@MetaSymbols@.

 Poznámka:

 1. ^<wrap>Pokud není se souborem asociovaný žádný vykonatelný příkaz a
volba #Používat registrované typy Windows# v ~Nastavení Systému~@SystemSettings@
je zapnuta, pokusí se Far pro vykonání tohoto typu souboru použít asociaci Windows;
 2. ^<wrap>~Příkazy~@OSCommands@ operačního systému "IF EXIST" (jestliže existuje)
a "IF DEFINED" (jestliže je definován) umožňují nastavit "chytřejší" asociace
- pokud máte pro jeden typ souboru specifikováno několik asociací, menu
zobrazí jen ty asociace, pro které jsou podmínky pravdivé.
 3. ^<wrap>If the specified mask is a regular expression, its capturing groups can be referenced in the commands as %RegexGroup#N# or %RegexGroup{#Name#}.


@MetaSymbols
$ #Speciální symboly#
 Následující speciální symboly mohou být použity v ~závislostech souborů~@FileAssoc@,
~menu uživatele~@UserMenu@ a příkazu ~"Aplikovat příkaz"~@ApplyCmd@:

 #!!#       ^<wrap>Znak '!'
 #!#        Dlouhý název souboru bez přípony
 #!~~#       Krátký název souboru bez přípony
 #!`#       Dlouhá přípona bez názvu souboru (ext)
 #!`~~#      Krátká přípona bez názvu souboru (ext)
 #!.!#      Dlouhý název souboru s příponou
 #!-!#      Krátký název souboru s příponou
 #!+!#      Podobný jako !-!, ale pokud byl dlouhý název po provedení příkazu ztracen Far ho obnoví
 #!@@!#      Název souboru s vybraným seznamem názvů souborů
 #!$!#      Název souboru s vybraným seznamem krátkých názvů souborů
 #!&#       Seznam vybraných souborů
 #!&~~#      Seznam vybraných krátkých názvů souborů
 #!:#       Aktuální jednotka
 #!\\#       Aktuální cesta
 #!/#       Krátký název aktuální cesty
 #!=\\#      Current path considering ~symbolic links~@HardSymLink@.
 #!=/#      Short name of the current path considering ~symbolic links~@HardSymLink@.
 #!?!#      Description of the current file

 #!?<název>?<řetězec>!#
 Tento symbol bude nahrazen při provádění příkazu uživatelovým vstupem. <název> a <řetězec> - název
a počáteční text upravovaného příkazu.
 Několik podobných symbolů na jednom řádku jsou přípustné, například:
 grep !?Vyhledat:?! !?V:?*.*!|c:\\far\\far.exe -v -

 A history name for the <init> string can be supplied in the <title>. In such case the command has the following format:
#!?$<history>$<title>?<init>!#, for example:
 grep !?#$GrepHist$#Search for:?! !?In:?*.*!|Far.exe -v -

 Leave the name empty to disable history.

 The entered string can also be accessed later as #%<history># (or as #%UserVarN#, where N is the index of the corresponding input).

 In <title> and <init> the usage of other meta-symbols is allowed by enclosing them in brackets, e.g.
grep !?Find in (!.!):?! |Far.exe -v -.

 #!###
 Modifikátor "!##" zadaný před symbolem závislosti
souboru ji odkazuje do pasivního panelu (see note 4).
Například !##!.! označí název daného souboru v pasivním panelu.

 #!^#
 Modifikátor "!^" zadaný před symbolem závislosti
souboru ji odkazuje do aktivního panelu (see note 4).
Například !^!.! označí název daného souboru v
aktivním panelu, !##!\\!^!.! - soubor v pasivním
panelu se stejným názvem jako daný soubor v aktivním panelu.

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

  Poznámka:

 1. ^<wrap>Když používáte speciální znaky, Far nahrazuje pouze znaky
odpovídající speciálním znakům. Žádné nejsou přidány (například
úvozovky), ale můžete je přidat samy pokud je to nutné. Například,
pokud použitý program  vyžaduje název souboru v asociaci uzavřený
mezi vykřičníky, můsíte ho specifikovat #program.exe "!.!"#
a ne #program.exe !.!#.

 2. ^<wrap>Následující modifikátory mohou být použity se závislostmi #!@@!# a #!$!#:
    #Q# - uzavřít názvy obsahující mezery do úvozovek;
    #S# - použít v názvech cest ‘/’ namísto ‘\\’;
    #F# - použít plné názvy cest;
    #A# - použít ANSI kódování.
    #U# - use UTF-8 code page;
    #W# - use UTF-16 (Little endian) code page.

    ^<wrap>Například závislost #!@@AFQ!# myslí "název souboru se seznamem vybraných názvů souborů,
v ANSI kódování, zahrnuje plné názvy cest, názvy s mezerami budou v úvozovkách".

    ^<wrap>The following modifiers can be used with the special symbols #!&# and #!&~~#:
    #Q# - enclose each name in quotes. This is the default, if no modifier is specified.
    #q# - do not enclose names in quotes (as it was before build 5466).

    ^<wrap>For example, #!&Q# denotes the list of selected file names, each enclosed in quotes.

 3. ^<wrap>Pokud je specifikovaných více závislostí, jsou meta-znaky !@@!
a !$! zobrazeny v menu tak jak jsou. Tyto znaky jsou přeloženy tehdy, když je příkaz proveden.

 4. ^<wrap>Předpony "!##", "!^", "![" a "!]" pracují jako spoje pro závislosti. Efekt
těchto předpon pokračuje až k další podobné předponě. Například:

    if exist !##!\\!^!.! diff -c -p !##!\\!^!.! !\\!.!

    ^<wrap>"Pokud existuje v pasivním panelu stejný soubor, jako pod
kurzorem v aktivním panelu, zobraz rozdíly mezi souborem v
pasivním panelu a aktivním panelu, bez ohledu na název daného
souboru v pasivním panelu"

 5. ^<wrap>If it is needed to pass to a program a name with an ending
backslash, use quotes, e.g. #"!"\#.
    ^<wrap>For example, to extract a rar archive to a folder with the same name:

    #winrar x "!.!" "!"\#


@SystemSettings
$ #Dialog Nastavení systému#
 #Mazat do Koše#
 Zapíná mazání souborů do koše. Operace mazání do koše může být provedena
jen pro lokální pevné disky.

 #Používat kopírovací rutiny systému#
 Používání funkce Windows CopyFileEx místo interní implementace Faru.
Může to být užitečné na NTFS, protože CopyFileEx kopíruje
rozšířené atributy souborů. On the other hand, when using the system
function, the possibility of "smart" ~copying~@CopyFiles@ of sparse files is not available.

 #Kopírovat soubory otevřené pro zápis#
 Umožňuje kopírovat soubory, které jsou již otevřeny jiným programem pro zápis.
Tento mód je vhodný pro kopírování dlouho otevřených souborů, ale může to být
nebezpečné, pokud je soubor upravován ve stejnou dobu, kdy je kopírován.

 #Scan symbolic links#
 Scan ~symbolic links~@HardSymLink@ along with normal sub-folders when building the folder tree,
determining the total file size in the sub-folders.

 #Ukládat historii příkazů#
 Způsobí uložení ~historie příkazů~@History@ před ukončením a obnovení po startu Faru.

 #Ukládat historii adresářů#
 Způsobí uložení ~historie adresářů~@HistoryFolders@ před ukončením a obnovení po startu Faru. Seznam
historie adresářů muže být aktivován i pomocí #Alt+F12#.

 #Ukládat historii Zobraz a Uprav#
 Způsobí uložení ~historie zobrazení a úpravy~@HistoryViews@ souborů před ukončením a obnovení po startu
Faru. Seznam historie zobrazení a úpravy může být aktivován i pomocí #Alt+F11#.

 #Používat registrované typy Windows#
 Při této zapnuté volbě, pokud stisknete Enter na souboru, který zná Windows a chybí
v seznamu ~závislosti souborů~@FileAssoc@ Faru, Windows spustí program, který je
registrován pro práci s tímto souborem.

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

 #Auto ukládání nastavení#
 Pokud je povolen, Far vždy automaticky uloží nastavení. Vždycky také uloží aktuální adresáře pro oba panely.


@PanelSettings
$ #Dialog nastavení panelů#
 #Ukázat skryté a systémové soubory#
 Zapní pro zobrazování souborů s atributem skrytý a systémový. Toto nastavení může být kdykoliv přepnuto pomocí #Ctrl+H#.

 #Zvýrazňovat soubory#
 Zapíná ~zvýraznění souborů~@Highlight@.

 #Vybírat adresáře#
 Povoluje vybírat adresáře, stisknutím #Num +# a #Num *#. Jinak tyto klávesy vybírají pouze soubory.

 #Right click selects files#
 If this option is on, #right mouse click# selects files. Otherwise, it opens Windows  Explorer Context menu.

 #Sort folder names by extension#
 Applies sort by extension not only to files, but also to folders. When this option is turned on, sorting
by extension works the same as it did in Far 1.65. If the option is turned off, in the extension sort mode the
folders will be sorted by name, as they are in the name sort mode.

 #Dovolit změnit mód třídění#
 Pokud je toto nastavení zapnuto a mód třídění aktuálního souborového panelu je znovu vybrán, bude nastaven obrácený mód.

 #Vypnout automatickou aktualizaci panelů#
 Mechanismus pro automatické updatování panelů když se změní stav souborového
systému bude vypnut tehdy, když počet souborových objektů překročí zadanou hodnotu.
 Auto-update mechanismus pracuje jen na souborových systémech FAT/FAT32/NTFS.
Hodnotou 0 je myšleno "updatovat vždy".
Pro ruční provedení updatu panelů stiskněte #Ctrl+R#.

 #Network drives autorefresh#
 This option enables panel autorefresh when the file system state of a network drive is changed.
It may be helpful to disable this option on slow network connections.

 #Zobrazovat nadpisy sloupců#
 Zapne zobrazování nadpisů sloupců v ~souborovém panelu~@FilePanel@.

 #Zobrazovat stavový řádek#
 Zapne zobrazování stavového řádku souborového panelu.

 #Detect volume mount points#
 Distiguishes between normal directory links (Junctions) and volume mount points.
This option significanty slows down displaying directories on slow network connections.

 #Zobrazovat informace o kapacitě souborů#
 Zapne zobrazování kapacity souborů na spodním okraji souborového panelu.

 #Zobrazovat volné místo#
 Zapne zobrazování volného místa na disku.

 #Zobrazovat posuvníky#
 Zapne zobrazování posuvníků v souborovém panelu a ~panelu strom~@TreePanel@.

 #Zobrazovat počet obrazovek na pozadí#
 Zapne zobrazování počtu ~obrazovek na pozadí~@ScrSwitch@.

  #Zobrazovat písmeno módu třídění#
 Zobrazovat v levém rohu panelu písmeno aktuálního módu třídění.

 #Show ".." in root folders#
 Enables displaying of ".." item in root folders.
Pressing #Enter# on this item opens ~Change drive menu~@DriveDlg@.


@TreeSettings
$ #Tree settings#
 #Automaticky měnit adresář#
 Pokud je povolen, mění pohybující se kurzor v ~panelu strom~@TreePanel@will adresář v druhém panelu.
Pokud není povolen, musíte pro změnu adresáře ze stromu stisknout #Enter#.

 #Minimum number of folders#
 The minimal number of folders on the disk for which folder tree cache file #tree3.far# will be created.


@InterfSettings
$ #Dialog nastavení rozhraní#
 #Hodiny#
 Ukazovat hodiny v pravém roku obrazovky.

 #Myš#
 Používat myš.

 #Zobrazovat zkratkové klávesy#
 Zobrazovat funkční klávesy na spodu obrazovky.
Toto nastavení může být přepnuto i pomocí #Ctrl+B#.

 #Vždy zobrazovat hlavní menu#
 Zobrazovat vždy hlavní menu na horním okraji obrazovky.

 #Šetřič#
 Spustit šetřič obrazovky po daném intervalu určeném v minutách. When this option
is enabled, screen saver will also activate when mouse pointer is brought
to the upper right corner of Far window.

 #Zobraz. ukazatel celkového stavu kopírování#
 Zobrazuje kolik je celkově hotovo při kopírování. Toto může mýt za následek
určitou prodlevu před započetím kopírování,
než se vypočte celková velikost souborů.

 #Zobrazovat informace o čase kopírovní#
 Zobrazuje informace o průměrné rychlosti kopírování, čase kopírování a odhadovaném
zbývajícím čase kopírování v kopír.dialogu.
 Ikdyž tato funkce vyžaduje nějaký čas pro vypočtení statistiky, je dobré, že můžete
vidět nějaké informace, pokud například kopírujete mnoho malých souborů
a nastavení indikátoru "Zobrazuj ukazatel celkového kopírování" je vypnut.

 #Show total delete indicator#
 Show total progress bar, when performing a file delete operation.
This could require some additional time before starting deleting
to calculate the total files count.

 #Použít Ctrl+PgUp pro změnu disku#
 Stisknutí #Ctrl+PgUp# v kořenovém adresáři zobrazí panel s menu pro výběr disku.
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
 #Historie v dialozích#    Ponechávat historii v editačních dilozích
                         Faru. Předchozí řetězec v seznamu historie
                         může být aktivován myší, nebo pomocí
                         Ctrl+Up a Ctrl+Down. Pokud si nepřejete
                         historii ukládat, například kvůli
                         bezpečnostním důvodům, odznačte toto
                         nastavení.

 #Trvaké bloky v#          Po přesunutí kurzoru v dialogu úpravy
 #editačních dialozích#    ovládání, přákazové řádce a adresářové
                         historii, nedojde ke zrušení výběru blokem.

 #Del removes blocks#      If a block is selected, pressing Del will
 #in edit controls#        not remove the character under cursor, but
                         this block.

 #AutoDokončování#         Umožňuje používat AutoDokončování v
 #v editačních dialozích#  editačních dialozích, které mají seznam
                         historie, nebo combo boxy. Když je toto
                         nastavení vypnuto, můžete pro
                         autodokončování použít klávesy #Ctrl+End#.
                         Při nahrávání, nebo provádění makra bývá
                         funkce autodokončování vypnutá.

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
 Far allows to change the command line prompt format.
To change it you have to enter the needed sequence of variables and
special code words in the #Set command line prompt format# input field
of the ~Command line settings~@CmdlineSettings@ dialog, this will allow showing
additional information in the command prompt.

 It is allowed to use environment variables and the following special code words:

 $a - ^<wrap>the & character
 $b - the | character
 $c - the ( character
 $d - current date (depends on system settings)
 $f - the ) character
 $g - the > character
 $h - delete the previous character
 $l - the < character
 $m - full network path of the current drive or empty, if the current drive is not a network drive
 $n - drive letter of the current drive
 $p - current drive and path
 $q - the = character
 $s - space
 $t - current time in HH:MM:SS format
 $w - current working directory (without the path)
 $$ - the $ character
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
$ #Interní prohlížeč: control keys#
 Navigation keys

 The behavior of navigation keys depends on the ~view mode~@ViewerMode@.

 The following keys work in all modes:

 #Nahoru#             ^<wrap>O řádek nahoru
 #Dolů#               O řádek dolů
 #PgUp#               O stránku nahoru
 #PgDn#               O stránku dolů
 #Home, Ctrl+Home#    Na začátek souboru
 #End, Ctrl+End#      Na konec souboru

 The following additional keys work in #text mode without line wrap#:

 #Vlevo#              ^<wrap>O znak doleva
 #Vpravo#             O znak doprava
 #Ctrl+Vlevo#         O 20 znaků doleva
 #Ctrl+Vpravo#        O 20 znaků doprava
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

 #F1#                 ^<wrap>Nápověda
 #F2#                 Zapnout/vypnout zalamování řádků in #text# mode, or change ~view mode~@ViewerMode@
 #Shift+F2#           Přepínání způsobu zalamování (písmena/slova) in #text# ~view mode~@ViewerMode@
 #F4#                 Přepínání ~view mode~@ViewerMode@ to #hex# and back
 #Shift+F4#           Select ~view mode~@ViewerMode@: #text#, #hex#, or #dump#
 #F6#                 Přepnutí do ~editoru~@Editor@
 #F7#                 ~Hledání~@ViewerSearch@
 #Shift+F7, Mezerník# Continue searching forward
 #Alt+F7#             Continue searching backwards
 #F8#                 Přepíná mezi OEM/ANSI textovým režimem
 #Shift+F8#           Výběr code page using the ~Code pages~@CodePagesMenu@ menu
 #Alt+F8#             ~Změna aktuální pozice~@ViewerGotoPos@
 #Alt+F9#             Přepíná velikost konzolového okna Faru;
see also ~Interface.AltF9~@Interface.AltF9@
 #Alt+Shift+F9#       Zobrazí dialog ~Nastavení Prohlížeče~@ViewerSettings@
 #Numpad5,F3,F10,Esc# Konec
 #Ctrl+F10#           Zaznamenat pozici do aktuálního souboru.
 #F11#                Zobrazí menu "~Příkazy pluginů~@Plugins@"
 #Alt+F11#            Display ~file view and edit history~@HistoryViews@
 #Num +#              Jdi do dalšího souboru
 #Num -#              Jdi do předchozího souboru
 #Ctrl+O#             Zobrazí obrazovku uživatele
 #Ctrl+Alt+Shift#     Dočasně zobrazí obrazovku uživatele
(tak dlouho, dokud jsou klávesy stisknuty)
 #Ctrl+B#             Zobrazí/Schová funkční klávesy na spodu obrazovky
 #Ctrl+Shift+B#       Toggle status line
 #Ctrl+S#             Zobrazí/Schová posuvník.
 #Alt+BackSp, Ctrl+Z# Vrátí změnu pozice
 #PravýCtrl+0…9#      Nastaví záložku 0…9 na aktuální pozici
 #Ctrl+Shift+0…9#     Nastaví záložku 0…9 na aktuální pozici
 #LevýCtrl+0…9#       Jít na záložku 0…9
 #Ctrl+Ins, Ctrl+C#   Zkopíruje výběr do schránky
The text can be selected manually or as the result of a ~search~@ViewerSearch@.
 #Ctrl+U#             Unselect the text
 #Shift+Mouse click#  Select text manually. The first mouse click indicates the
beginning of the selected area; the second click indicates the end.
Use navigation keys after the first click to bring the end position into
the view. The end of the selected area may be set before or after the
beginning in the text.

 See also the list of ~macro keys~@KeyMacroViewerList@ available in the viewer.

 Poznámka:

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
$ #Jít do nastavené pozice#
 Tento dialog umožňuje změnit pozici v interním prohlížeči.

 You can enter an absolute or relative value or percentage, in decimal or hexadecimal.
 For relative add #+# or #-# before the value.
 For percentage add #%# after the value.
 For decimal either add #m# after the value or uncheck the #Hex value#.
 For hexadecimal either add #0x# or #$# before the value, #h# after the value, or check the #Hex value#.

 The value will be interpreted as an offset from the beginning of the file.
If the current view mode is #unwrapped text# it is possible to enter an additional value
which will be interpreted as a first visible column.
Čísla musí být odděleny jedním z následujících znaků: #,.;:#.
If a value is omitted the corresponding parameter will not be changed.


@ViewerSearch
$ #Hledání v prohlížeči#
 Pro vyhledávání v ~prohlížeči~@Viewer@ jsou k dispozici následující
módy a nastavení:

 #Hledat text#
 Vyhledává jakýkoliv text zadaný ve #vyhledávacím# příkazovém řádku.

 #Hledat hex#
 Vyhledávání řetězce odpovídajícího hexadecimálnímu kódu vloženému ve #vyhledávacím# řetězci.

 #Rozlišovat velikost písma#
 Při vyhledávání se bere v úvahu velikost písmen v zadaném textu
(například #Text# nebude nalezen, pokud budete hledat slovo #text#).

 #Celá slova#
 Zadaný tet bude nalezen pouze tehdy, pokud ve v textu nachází jako celé slovo.

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

 Pohyb kurzoru

 #Vlevo#                   ^<wrap>O znak doleva
 #Ctrl+S#                  Move the cursor one character to the left, but don't move to the previous line if the line beginning is reached.
 #Vpravo#                  O znak doprava
 #Nahoru#                  O řádek nahoru
 #Dolů#                    O řádek dolů
 #Ctrl+Vlevo#              O slovo doleva
 #Ctrl+Vpravo#             O slovo doprava
 #Ctrl+Nahoru#             Skroluj obraz nahoru
 #Ctrl+Dolů#               Skroluj obraz dolů
 #PgUp#                    O stránku nahoru
 #PgDn#                    O stránku dolů
 #Home#                    Na začátek řádku
 #End#                     Na konec řádku
 #Ctrl+Home, Ctrl+PgUp#    Na začátek souboru
 #Ctrl+End, Ctrl+PgDn#     Na konec souboru
 #Ctrl+N#                  Na začátek obrazovky
 #Ctrl+E#                  Na konec obrazovky

 Operace mazání

 #Del#                     ^<wrap>Smaže znak (také může smazat blok, podle nastavení v ~Nastavení Editoru~@EditorSettings@).
 #BackSpace#               Smaže znak vlevo
 #Ctrl+Y#                  Smaže řádek
 #Ctrl+K#                  Smaže vše od aktuální pozice do koce řádku
 #Ctrl+BackSpace#          Smaže slovo vlevo
 #Ctrl+T, Ctrl+Del#        Smaže slovo vpravo

 Operace s bloky

 #Shift+Kurzorové šipky#   ^<wrap>Vybrat blok
 #Ctrl+Shift+Kurzor.šipky# Vybrat blok
 #Alt+Num Kurzorové šipky# Vybrat vertikální blok
 #Alt+Shift+Kurzor. šipky# Vybrat vertikální blok
 #Ctrl+Alt+Num klávesy#    Vybrat vertikální blok
 #Ctrl+A#                  Vybrat celý text
 #Ctrl+U#                  Odznačit blok
 #Shift+Ins, Ctrl+V#       Vložit blok ze schránky
 #Shift+Del, Ctrl+X#       Vyjmout blok
 #Ctrl+Ins, Ctrl+C#        Zkopírovat blok do schránky
 #Ctrl+<Num +>#            Přidat blok do schránky
 #Ctrl+D#                  Smazat blok
 #Ctrl+P#                  Zkopírovat blok na aktuální pozici kurzoru (pouze v módu trvalých bloků)
 #Ctrl+M#                  Přesunout blok na aktuální pozici kurzoru (pouze v módu trvalých bloků)
 #Alt+U#                   Posune blok vlevo
 #Alt+I#                   Posune blok vpravo

 Ostatní operace

 #F1#                      ^<wrap>Nápověda
 #F2#                      Uložit soubor
 #Shift+F2#                ~Uložit soubor jako…~@FileSaveAs@
 #Shift+F4#                Edit ~new file~@FileOpenCreate@
 #F6#                      Přepnutí do ~prohlížeče~@Viewer@
 #F7#                      ~Hledat~@EditorSearch@
 #Ctrl+F7#                 ~Nahradit~@EditorSearch@
 #Shift+F7#                Continue searching or replacing forward
 #Alt+F7#                  Continue searching or replacing backwards
 #F8#                      Přepíná mezi DOS/Windows textovým režimem
 #Shift+F8#                Výběr OEM/ANSI code page
 #Alt+F8#                  ~Jdi na~@EditorGotoPos@ určený řádek a sloupec
 #Alt+F9#                  Přepíná velikost konzolového okna Faru; see also ~Interface.AltF9~@Interface.AltF9@
 #Alt+Shift+F9#            Zobrazí dialog ~Nastavení Editoru~@EditorSettings@
 #F10, F4, Esc#            Konec
 #Shift+F10#               Ulož a konec
 #Ctrl+F10#                Zaznamenat pozici do aktuálního souboru
 #F11#                     Zobrazí menu "~Příkazy pluginů~@Plugins@"
 #Alt+F11#                 Display ~file view and edit history~@HistoryViews@
 #Alt+BackSpace, Ctrl+Z#   Vrátit změnu
 #Ctrl+Shift+Z#            Redo
 #Ctrl+L#                  Zakáže změny v textu
 #Ctrl+O#                  Zobrazí obrazovku uživatele
 #Ctrl+Alt+Shift#          Dočasně zobrazí obrazovku uživatele (tak dlouho, dokud jsou klávesy stisknuty)
 #Ctrl+Q#                  Považovat následující kombinaci kláves za znakový kód
 #PravýCtrl+0…9#           Nastaví záložku 0…9 na aktuální pozici
 #Ctrl+Shift+0…9#          Nastaví záložku 0…9 na aktuální pozici
 #LevýCtrl+0…9#            Jít na záložku 0…9
 #Shift+Enter#             Vložit název aktuálního souboru v aktivním okně na pozici kurzoru.
 #Ctrl+Shift+Enter#        Vložit název aktuálního souboru v pasivním okně na pozici kurzoru.
 #Ctrl+F#                  Vložit celý název (i s cestou) editovaného souboru na pozici kurzoru
 #Ctrl+B#                  Show/Hide functional key bar at the bottom line.
 #Ctrl+Shift+B#            Show/Hide status line

 See also the list of ~macro keys~@KeyMacroEditList@, available in the editor.

 Poznámka:

 1. ^<wrap>Pokud není označen blok posune #Alt+U#/#Alt+I# aktuální řádek.
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
$ #Uložit soubor jako…#
 Editovaný soubor můžete uložit pod jiným názvem stisknutím #Shift+F2#
a zadáním nového názvu, code page and End of Line characters format.

 If file has been edited in one of the following code pages: UTF-8,
UTF-16 (Little endian) or UTF-16 (Big endian), then if the option #Add signature (BOM)# is on,
the appropriate marker is inserted into the beginning of the file, which
helps applications to identify the code page of this file.

 Můžete změnit formát zakončení řádků:

 #Beze změny#
 Formát zakončení řádků zůstane beze změny.

 #Dos/Windows formát (CR LF)#
 Formát zakončení řádků bude představovat dvou-znakovou sekvenci -
 Carriage Return a Line Feed (CR LF), tak jak se používá pod DOSem/Windows.

 #Unix formát (LF)#
 Formát zakončení řádků bude představovat jeden znak - Line
Feed (LF), tak jak se používá pod Unixem.

 #Mac format (CR)#
 Line breaks will be represented as a single character - Carriage
Return (CR), as used in Mac OS.


@EditorGotoPos
$ #Jdi na určený řádek a sloupec#
 Tento dialog umožňuje změnit pozici v interním editoru.

 You can enter an absolute or relative value or percentage, in decimal or hexadecimal.
 For relative add #+# or #-# before the value.
 For percentage add #%# after the value.
 For decimal either add #m# after the value or uncheck the #Hex value#.
 For hexadecimal either add #0x# or #$# before the value, #h# after the value, or check the #Hex value#.

 První číslo bude považováno za číslo řádku, druhé, za číslo sloupce.
Čísla musí být odděleny jedním z následujících znaků: #,.;:#.
If a value is omitted the corresponding parameter will not be changed.


@EditorReload
$ #Znovunačtení souboru v editoru#
 Manažer Far sleduje všechny pokusy o opakované otevření k editaci
již upravovaného souboru. Pravidla pro znovunačtení souborů jsou následující:

 1. ^<wrap>Pokud soubor nebyl změněn a volba "Znovu nahrát upravovaný soubor"
v dialogu ~Potvrzení~@ConfirmDlg@ není zapnutá, přepne Far automaticky
do otevřené instance editoru.

 2. ^<wrap>Pokud byl soubor změněn, nebo volba "Znovu nahrát upravovaný soubor"
je zapnutá, jsou tři možné volby:

 #Stávající#
 Pokračuje v úpravě stávajícího souboru

 #Nová instance#
 Soubor bude pro editování otevřen v nové instanci
editoru. V tomto případě buďte opatrní: obsah
souboru na disku bude odpovídat obsahu instance
editoru, kde byl soubor naposledy uložen.

 #Znovu otevřít#
 Aktuální změny nebudou uloženy a obsah
souboru na disku bude znovu nahrán do editoru.


@WarnEditorPath
$ #Varování: Cesta k souboru pro úpravu neexistuje#
 Když otevřete nový soubor pro ~editaci~@Editor@, máte zadaný
název adresáře, který neexistuje. Před uložením souboru Far
vytvoří adresář, s tou podmínkou, že cesta je správná (např.
cesta začínající neexistujícím písmenem disku nebude správná)
a máte dostatečná práva pro vytvoření adresáře.


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
$ #Změna jednotky#
 Toto menu umožňuje změnit v panelu aktuální jednotku, odpojit se od síťové jednotky, nebo otevřít nový panel ~pluginu~@Plugins@.

 Vyberte písmeno odpovídající jednotce pro její změnu, nebo položku
s názvem pluginu pro vytvoření nového panelu pluginu. Pokud není
typ panelu ~souborový panel~@FilePanel@, bude na něj změněn.

 #Ctrl+A#, #F4# hotkeys invoke the ~file attributes~@FileAttrDlg@ for drives.

 #Ctrl+A#, #F4# hotkeys can be used to assign a hotkey to plugin item.

 #F3# key shows plugin technical information.

 Klávesa #Del# může být použita:
 - ^<wrap>pro ~odpojení~@DisconnectDrive@ od síťové jednotky;
 - pro smazání substituované jednotky;
 - pro smazání virtuální jednotky;
 - pro vysunutí disku z mechaniky CD-ROM a výměnných jednotek.

 Pro vysunutí disku ze ZIP - mechaniky požadována administrátorská práva.

 CD-ROM může být zasunuta stisknutím klávesy #Ins#.

 The #Shift+Del# hotkey is used to prepare a USB storage device for safe
removal. If the disk, for which the removal function is used, is a flash-card
inserted into a card-reader that supports several flash-cards then the
card-reader itself will be stopped.

 #Ctrl+1# - #Ctrl+9# přepíná zobrazování různých informací:

 #Ctrl+1# - ^<wrap>typ disku;
 #Ctrl+2# - ^<wrap>název sítě / a cestu asociovanou se SUBST jednotkou
/ path to virtual disk container;
 #Ctrl+3# - ^<wrap>název disku;
 #Ctrl+4# - ^<wrap>souborový systém;
 #Ctrl+5# - ^<wrap>celkové a volné místo na disku (this option has two
display modes, press twice to see);
 #Ctrl+6# - ^<wrap>zobrazuje parametry výměnných disků;
 #Ctrl+7# - ^<wrap>zobrazuje položky pluginů;
 #Ctrl+8# - ^<wrap>zobrazuje parametry CD;
 #Ctrl+9# - ^<wrap>network parameters.

 Nastavení menu #Změnit jednotku# je uloženo v konfiguraci Faru.

 #F9# shows the ~dialog~@ChangeDriveMode@ to control displaying
of this information.

 Pokud je zapnuto nastavení "~Použít Ctrl+PgUp pro změnu disku~@InterfSettings@",
stisk #Ctrl+PgUp# funguje stejně jako stisknutí #Esc# - ukončí výběr
disku a zavře menu.

 Stisknutí #Shift+Enter# vyvolá  Průzkumníka Windows a zobrazí v něm
kořenový adresář vybraného diksu (funguje jen pro diskové jednotky a
ne pro pluginy).

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
$ #Odpojit síťovou jednotku#
 Odpojit siťovou jednotku můžete stisknutím klávesy #Del# v menu
~Změnit jednotku~@DriveDlg@.

 Nastavení #[x] Připojit po přihlášení# je zapnuto pouze pro stále
připojené síťové jednotky.

  Potvrzování může být vypnuto v dialogu ~Potvrzení~@ConfirmDlg@.


@Highlight
$ #Zvýraznění souborů#
 Pro více pohodlné a zřetelné zobrazení souborů a adresářů v
panelech, má manažer Far schopnost barevného zvýraznění pro souborové
objekty. Souborové objekty můžete vkládat do skupin podle různých kritérií
(~maska souboru~@FileMasks@, atribut souboru) a této skupině přiřadit barvu.

 Zvýraznění souborů může být zapnuto, nebo vypnuto v dialogu pro
~nastavení panelů~@PanelSettings@ (položka menu Nastavení | Nastavení Panelů).

 ~Upravovat~@HighlightEdit@ parametry některé zvýrazněné skupiny můžete
skrze menu "~Nastavení~@OptMenu@" (položka "Zvýraznění souborů").


@HighlightList
$ #Seznam zvýrazněných skupin#
 Menu ~Zvýraznění souborů~@Highlight@ vám umožňuje provádět
různé operace se seznamem skupin. Jsou k dispozice následující
kombinace kláves:

 #Ins#           - ^<wrap>Přidá novou zvýrazněnou skupinu

 #F5#            - ^<wrap>Zduplikuje aktuální skupinu

 #Del#           - ^<wrap>Vymaže aktuální skupinu

 #Enter# nebo #F4# - ^<wrap>~Upraví~@HighlightEdit@ aktuální zvýrazněnou skupinu

 #Ctrl+R#        - ^<wrap>Obnoví implicitní zvýraznění skupin souborů

 #Ctrl+Nahoru#   - ^<wrap>Přesune skupinu výše.

 #Ctrl+Dolů#     - ^<wrap>Přesune skupinu níže.

 Zvýrazněné skupiny jsou kontrolovány z hora dolů. Pokud je zjištěno,
že daný soubor patří do skupiny, nejsou další skupiny kontrolovány,
unless #[x] Continue processing# is set in the group.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@HighlightEdit
$ #Zvýraznění souborů#
 Dialog #Zvýraznění souborů# v menu ~Nastavení~@OptMenu@ umožňuje
definovat skupiny zvýraznění souborů. Každá definice skupiny ~zahrnuje~@Filter@:

 - jednu, nebo několik ~souborových masek~@FileMasks@;

 - atributy pro zahrnutí nebo vyjmutí:
   #[x]# - ^<wrap>inclusion attribute - file must have this attribute.
   #[ ]# - ^<wrap>exclusion attribute - file must not have this attribute.
   #[?]# - ^<wrap>ignore this attribute;

 - ^<wrap>normální název, vybraný název, název pod kurzorem a barvu
vybraného názvu pod kurzorem pro zobrazení názvů souboru.
Pokud si přejete používat implicitní barvu, nastavte "černou na černé";

 - ^<wrap>označení pro soubory ze skupiny.
Te může být použit spolu z barevným zvýrazněním, nebo místo něho.

 Soubory patří do zvýrazněné skupiny, pokud:
 - ^<wrap>je zapnutá analýza masek a název souboru je shodný s některou
souborovou maskou (pokud je analýza masek vypnutá, nemá
název souboru význam);
 - má všechny zahrnuté atributy;
 - nemá žádné vyňaté atributy.

 Atributy Komprimovaný, Zašifrovaný, Not indexed, Sparse, Temporary and Reparse point
jsou platné jen pro jednotky s NTFS. The #Integrity stream# and
#No scrub data# attributes are supported only on ReFS volumes starting from
Windows Server 2012.


@ViewerSettings
$ #Nastavení Prohlížeče#
 Tento dialog umožňuje změnít nastavení interního, nebo externího ~prohlížeče~@Viewer@.

@=
^#Prohlížeč#
@=
 #Použít externí prohlížeč pro F3#
 Start external viewer on #F3# key and internal viewer on #Alt+F3# namísto Alt+F3# key combination.

 #Příkaz prohlížeče#
 Příkaz pro spuštění externího prohlížeče.
Pro zadání souboru pro prohlížení použijte ~speciální symboly~@MetaSymbols@.

@=
^#Interní prohlížeč#
@=
 #Persistent selection#
 Do not remove block selection after moving the cursor.

 #Velikost Tabulátoru#
 Počet mezer ve znaku tabulátoru.

 #Zobrazovat skrolovací šipky#
 Zobrazovat v prohlížeči skrolovací šipky, pokud se text nevejde horizontálně do okna.

 #Visible '\0'#
 Show a printable character instead of space for the character '\0'.
The character to display can be set in ~far:config~@FarConfig@ #Viewer.ZeroChar#.

 #Zobrazovat posuvník#
 Zobrazovat posuvník v interním prohlížeči. Toto nastavení může být kdykoliv přepnuto stisknutím #Ctrl+S# v interním prohlížeči.

@=
 #Ukládat pozici v souboru#
 Uloží a potom obnoví pozici v nedávno prohlížených souborech. Toto nastavení také
obnoví znakovou sadu, pokud byla uživatelem manuálně zadána a ~mód~@ViewerMode@ prohlížení.

 #Save file code page#
 Save and restore the code page selected for a file. This is automatically enabled if #Save file position#
is enabled, as file position depends on the encoding.

 #Ukládat záložky#
 Uloží a potom obnoví záložky (aktuální pozici) v nedávno prohlížených souborech
(vytvořené #PravýCtrl+0…9# or #Ctrl+Shift+0…9# key combinations.)

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
 Pokud je klávesa #F3# nastavena pro externí prohlížeč, bude spuštěn
pouze tehdy, pokud není pro daný typ souborů ~přiřazen~@FileAssoc@ jiný prohlížeč.

 Úpravy nastavení v tomto dialogu nemají vliv na dříve otevřená okna interního prohlížeče.

 Dialog pro nastavení může být kdykoliv vyvolán z ~interního
prohlížeče~@Viewer@ stisknutím #Alt+Shift+F9#. Změny nabydou platnosti
okamžitě, ale budou mít vliv pouze na aktuální relaci.


@EditorSettings
$ #Nastavení Editoru#
 Tento dialog umožňuje změnít nastavení ~interního~@Editor@, nebo externího editoru.

@=
^#Externí editor#
@=
 #Použít pro F4#
 Run external editor using #F4# instead of #Alt+F4#.

 #Příkaz editoru#
 Příkaz pro spuštění externího editoru.
 Pro zadání souboru pro editaci použijte ~speciální symboly~@MetaSymbols@.

@=
^#Interní editor#
@=
 #Do not expand tabs#
 Do not convert tabs to spaces while editing the document.

 #Expand newly entered tabs to spaces#
 While editing the document, convert each newly entered #Tab# into the appropriate number of spaces.
Other tabs won't be converted.

 #Expand all tabs to spaces#
 Upon opening the document, all tabs in the document will be automatically converted to spaces.

 #Trvalé bloky#
 Nerušit výběr bloku po přesunutí kurzoru.

 #Del maže bloky#
 Pokud je blok označen, stisk kávesy Del nesmaže znak pod kurzorem, ale smaže celý tento blok.

 #Auto Odsazování#
 Zapíná auto odsazovací mód při vkládání textu.

 #Velikost Tabulátoru#
 Počet mezer ve znaku tabulátoru.

 #Show white space#
 Make while space characters (spaces, tabulations, line breaks) visible.

 #Kurzor za koncem řádky#
 Umožňuje přesunout kurzor za konec řádky.

 #Select found#
 Found text is selected.

 #Cursor at the end#
 Place the cursor at the end of the found block.

 #Show a scrollbar#
 Show a scrollbar.

@=
 #Ukládat pozici v souboru#
 Uloží a potom obnoví pozici v nedávno upravených souborech. Toto nastavení také
obnoví znakovou sadu, pokud byla uživatelem manuálně zadána.

 #Ukládat záložky#
 Uloží a potom obnoví záložky (aktuální pozici) v nedávno upravených souborech
(vytvořené #PravýCtrl+0…9# or #Ctrl+Shift+0…9#)

 #Allow editing files opened for writing#
 Allows to edit files that are opened by other programs for writing. This mode is handy to edit
a file opened for a long time, but it could be dangerous, if a file is being modified at the same time as editing.

 #Zamknout úpravu souborů určených jen pro čtení#
 Pokud je pro úpravu otevřen soubor s atributem jen pro čtení,
editor vypne možnost editovat text, stejně, jako při stisku #Ctrl+L#.

 #Varovat při otev. soub. určených jen pro čtení#
 Pokud je pro úpravu otevřen soubor s atributem jen pro čtení, je zobrazeno varování.

 #Autodetect code page#
 ~Autodetect~@CodePageAuto@ the code page of the file being edited.

 #Default code page#
 Select the default code page.

 Pokud je klávesa #F4# nastavena pro externí editor, bude spuštěn pouze
tehdy, pokud není pro daný typ souborů ~přiřazen~@FileAssoc@ jiný editor.

 Úpravy nastavení v tomto dialogu nemají vliv na dříve otevřená okna interního editoru.

 Dialog pro nastavení může být kdykoliv vyvolán z ~interního
editoru~@Editor@ stisknutím #Alt+Shift+F9#. Změny nabydou platnosti
okamžitě, ale budou mít vliv pouze na aktuální relaci.


@CodePageAuto
$ #Autodetect code pages#
 Far will try to choose the correct code page for viewing/editing a file.
Note that correct detection is not guaranteed, especially for small or
non-typical text files.

 See also the ~Code pages~@CodePagesMenu@ menu and
~far:config Codepages.NoAutoDetectCP~@Codepages.NoAutoDetectCP@


@FileAttrDlg
$ #Dialog Atributy souboru#
 Tento příkaz umí změnit atributy a časy souboru. Aplikován může být na kterýkoliv soubor,
nebo skupinu souborů.

 #Atributy souboru#

 Checkboxy použité v tomto dialogu mohou nabýt následující 3 stavy:

 #[x]# - atribut je nastaven pro všechny vybrané položky
       (nastaví atribut pro všechny položky)

 #[ ]# - atribut není nastaven u žádné vybrané položky
       (zruší atribut pro všechny položky)

 #[?]# - stav atributu není u všech vybraných položek stejný
       (nemění atribut)

 Pokud mají všechny vybrané soubory stejnou hodnoru atributu, příslušný
checkbox bude v 2-stavovém módu - jen nastav/zruš. Pokud budou vybrány
adresáře, všechny checkboxy budou vždy 3-stavové.

 Bude změněn jen ten atribut, u kterého byl změněn stav příslušného
checkboxu, oproti původnímu stavu.

 Atributy #Komprimovaný#, #Zašifrovaný#, #Not indexed#, #Sparse#, #Temporary#,
#Offline#, #Reparse point# mohou být změněny pouze na jednotkách s NTFS.
The #Compressed# and #Encrypted# attributes jsou navzájem exkluzivní, to znamená,
že můžete nastavit jen jeden z nich. You cannot clear the #Sparse# attribute in Windows 2000/XP/2003. The
#Integrity stream# and #No scrub data# attributes are supported only on ReFS volumes starting from
Windows Server 2012.

 Pro ~adresářové linky~@HardSymLink@ zobrazí dialog originální informace o adresáři.
If this information is not available, then the "#(data not available)#" message will be shown.

 #Datum a čas souboru#

 Jsou podporovány tři různé časy:
 - čas poslední úpravy;
 - čas vytvoření;
 - čas posledního přístupu;
 - change time.

 Na jednotkách s FAT je hodina, minuta a sekunda posledního přístupu
vždy rovna nule.

 Pokud si nepřejete měnit čas, nechte příslušná pole prázdná. Pro
vymazání všech polí a individuální změnu časů a datumů můžete použít
tlačítko #Prázdný#, například, jen měsíc, nebo jen minuty. Všechny
ostatní datumy a časy zůstanou nezměněny.

 Tlačítko #Současný# vyplní všechna pole aktuálním časem a datumem.
 Tlačítko #Originál# vyplní pole času souboru jeho originálními hodnotami.
Tato volba je k dispozici jen tehdy, pokud je dialog vyvolán pro jeden souborový objekt.

 The #System properties# button invoke the system properties dialog for
selected objects.


@FolderShortcuts
$ #Adresářové zkratky#
 Adresářové zkratky jsou navrženy pro zajištění rychlého přístupu do
často používaných adresářů. Pro vytvoření zkratky do aktuálního adresáře
stiskněte Ctrl+Shift+0…9. Pro vstup do požadovaného
adresáře stiskněte PravýCtrl+0…9. Pokud je PravýCtrl+0…9 stisknut v
editačním dialogu, bude do něj vložena cesta k danému adresáři.

 Položka #Adresářové zkratky# v ~menu Příkazy~@CmdMenu@ může být
použita k prohlížení, nastavení, úpravě a vymazání adresářových zkratek.

 When you are editing a shortcut (#F4#), you cannot create a shortcut to a
plugin panel.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@FiltersMenu
$ #Filters menu#
 Using the #Filters menu# you can define a set of file types with user
defined rules according to which files will be processed in the area of
operation this menu was called from.

 Filtrové menu obsahuje dvě části. V horní části jsou zorazeny #uživatelské
filtry#, spodní část filtrového menu obsahuje masky všech souborů z aktivního
souborového panelu (including file masks that are selected in the
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

 Vlastní výběr filtrů je uložen v konfiguraci Faru.

 Pokud je v panelu použit filtr je panel označen za písmenem módu třídění
v levém horním rohu znakem '*'.

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
$ #Popisy souboru#
 Popisy souboru mohou být použity pro sloučení textové informace se
souborem. Popisy souborů v daném adresáři jsou v něm uloženy v souboru
se seznamem popisů. Formát popisového souboru je název souboru následovaný
mezerami a popisem.

 Popisy mohou být zobrazeny v příslušném ~módu zobrazení~@PanelViewModes@
souborového panelu. Implicitně jsou tyto módy #Popisový# a #Douhý popisový#.

 Příkaz #Popsat soubory# (#Ctrl+Z#) z ~menu Soubory~@FilesMenu@ je
použit pro popsání vybraných souborů.

 Seznam popisových souborů může být změněn v dialogu #Popisy souborů#
z ~menu Nastavení~@OptMenu@. V tomto dialogu také můžete nastavit
lokální updatový mód popisů. Updatování může být vyputo, zapnuto jen
tehdy pokud je nastavený mód schopen zobrazovat popisy, nebo vždy
zapnuto. Implicitně Far nastavuje souborům s popisy atribut "Skrytý",
ale toto nastavení můžete změnit vypnutím volby v tomto dialogu "Pro
nové soubory s popisy nastavit atribut "Skrytý"". Také zde můžete
nastavit pozici zarovnání nových popisů souborů v seznamu popisů.

 Pokud má soubor s popisem nastavený atribut "jen pro čtení",
nepokusí se Far popis updatovat a po přesunutí, nebo smazání souborového
objektu bude zobrazena chybová hláška. Pokud je zapnutá volba
"#Updatovat popisové soubory s atributem Jen pro čtení#", pokusí se Far
popísový soubor korektně updatovat.

 Pokud je updatování povoleno, Far updatuje soubory popisů vždy,
když kopíruje, přesouvá a maže soubory. Pokud se manipuluje se soubory
v podadresářích, popisy v podadresářích nebudou updatovány.

 #Use ANSI code page by default#
 By default Far uses the OEM codepage for file descriptions, both for reading and writing.
This option changes it to ANSI.

 #Save in UTF-8#
 If set, the description file will be read as OEM or ANSI, depending on the option above,
but saved in UTF-8 after you add, remove or update the descriptions.

 #Note#: these options are irrelevant when the file has the UTF-8 signature. In this case it is always read and written in UTF-8.


@PanelViewModes
$ #Úprava módů pohledu souborového panelu#
 ~Souborový panel~@FilePanel@ může zobrazovat informace s použitím 10
předdefinovaných módů: stručný, střední, plný, široký, detailní, popisový,
dlouhý popisový, vlastníka souborů, linkový a alternativní plný. Obvykle
je to dostatečné, ale pokud chcete, můžete každému upravit parametry, nebo
je dokonce nahradit zcela novými módy.

 Příkaz #Módy souborových panelů# z ~menu Nastavení~@OptMenu@ umožňuje
změnit nastavení módů zobrazení panelů. Předně nabízí výběr požadovaného
módu ze seznamu. Položka "Stručný mód" v tomto seznamu odpovídá stručnému
módu souborového panelu (#LeftCtrl+1#), "Střední" odpovídá střednímu módu
souborového panelu (#LeftCtrl+2#), atd. Poslední položka, "Alternativní plný",
odpovídá módu nastavovanému pomocí #LeftCtrl+0#. Po výběru módu můžete změnit
následující nastavení:

 #Typ sloupců# - a comma-separated list. Each column type starts with
a file property character, such as name, size, etc. Some file properties
may be followed by modifiers. Supported column types (properties and
their modifiers) are listed below.

 If the list of column types consists of two or more repeated groups,
the files on the panel will be listed in “stripes”. Properties of each
file will be displayed in the columns of a stripe, and the list of files
will wrap from one stripe to the next like text of a newspaper article.
If column type list cannot be properly split into the equal groups, the
files will be listed on a single stripe.

 Existují následující typy sloupců:

 N[M[D],O,R[F],N] - název souboru, kde:
                    M - ^<wrap>zobrazovat značky výběru, kde:
                        D - dynamic selection marks;
                    O - ^<wrap>zobrazovat názvy bez cest (zamýšleno hlavně pro ~pluginy~@Plugins@);
                    R - ^<wrap>zarovnávat názvy doprava, kde:
                        F - right align all names;
                    N - ^<wrap>do not show extensions in name column;

 Tyto modifikátory mohou být použity v kombinacích, například NMR.

 X[R]       - file extension, where:
              R - ^<wrap>right align file extension;

 S[C,T,F,E] - velikost souboru
 P[C,T,F,E] - komprimovaná velikost souboru
 G[C,T,F,E] - velikost file streams, kde:
              C - ^<wrap>group digits using the character from Windows settings;
              T - ^<wrap>pokud je sloupec příliš malý pro zobrazení
plné velikosti, používat jako dělič 1000
místo 1024; in this mode unit character is shown in lower case, e.g. #k#,
#m#, #g# instead of #K#, #M#, #G#;
              F - ^<wrap>show size as a decimal fraction with
no more than three digits before decimal point, e.g. 999 bytes will
be shown as #999#, while 1024 bytes as #1.00 K#; note that the behavior
depends on whether the #T# modifier is used;
              E - ^<wrap>economic mode, no space between the
size and the unit character, e.g. #1.00k#;

 D          - datum úpravy souboru
 T          - čas úpravy souboru

 DM[B,M]    - datum a čas úpravy souboru
 DC[B,M]    - datum a čas vytvoření souboru
 DA[B,M]    - datum a čas posledního přístupu k souboru
 DE[B,M]    - file change date and time, where:
              B - stručný (styl Unixu) formát času souboru;
              M - používat textové názvy měsíců;

 A          - atributy souboru
 Z          - popis souboru

 O[L]       - vlastník souboru
              L - show domain name;

 LN        - číslo pevného linku;

 F          - number of streams.

 Pokud popis typů sloupců obsahuje více než jeden sloupec názvu
souborů, bude souborový panel zobrazen v multisloupcové formě.

 Atributy souborů mají následující označení:

 #N# - Attributes not set
 #R# - Pouze pro čtení
 #H# - Skrytý
 #S# - Systémový
 #D# - Directory
 #A# - Archív
 #T# - Temporary
 #$# - Sparse
 #L# - Reparse point
 #C# - Komprimovaný
 #O# - Offline
 #I# - Not content indexed
 #E# - Zašifrovaný
 #V# - Integrity stream
 #?# - Virtual
 #X# - No scrub data
 #P# - Pinned
 #U# - Unpinned

 By default the size of the attributes column is 6 characters. To display
the additional attributes it is necessary to manually increase the size of the column.

 #Šířka sloupců# - pro nastavení šírky sloupců panelu.
Pokud je šířka rovna 0, bude použita implicitní hodnota. Pokud je šířka
sloupce Název, Popis nebo Vlastník rovna 0, bude vypočtena automaticky,
v závislosti na šířce panelu. Je důrazně doporučeno, pro správné operace s
rozdílnými šířkami obrazu, nechat alespoň u jednoho sloupce nechat
počítat šířku automaticky. Width can be also set as a percentage of
remaining free space after the fixed width columns by adding the "%" character
after the numerical value. If the total size of such columns exceeds 100%,
their sizes are scaled.

 Zvětšení implicitní šířky sloupce času nebo data souboru a časového
sloupce o 1 způsobí zobrazení 12-hodinového formátu času. Další zvětšení
šířky povede k zobrazení sekund a milisekund.

 Pro zobrazení roku v 4-číselném formátu zvětšete sloupec data o 2.

 Enabling links, streams and owner columns (G, LN, F and O) can significantly
slow down the directory reading.

 #Typ sloupce stavového řádku# a #Šířka sloupce stavového řádku# -
podobně jako "Typ sloupců" a "Šířka sloupců", ale pro panel stavové řádky.

 #Celoobrazovkový režim# - nastaví celoobrazovkový pohled místo půlobrazového.

 #Zarovnat přípony souborů# - zobrazení přípon souborů zarovnaně.

 #Align folder extensions# - show folder extensions aligned.

 #Zobrazit adresáře velkými písmeny# - zobrazení všech názvů adresářů
velkými písmeny, bez ohledu na původní velikost písmen.

 #Zobrazit soubory malými písmeny# - zobrazení všech názvů souborů
malými písmeny, bez ohledu na původní velikost písmen.

 #Zobrazit velké znaky ve jménech souborů jako malá písmena# -
zobrazovat všechna velká písmena v návzech souborů jako malá. Implicitně
je toto nastavení zapnuto, ale pokud si přejete vidět skutečné názvy souborů,
vypněte nastavení "Zobrazit adresáře velkými písmeny" a "Zobrazit soubory malými
písmeny". Všechny tyto metody pouze mění způsob zobrazování souborů, při práci
se soubory Far vždy používá skutečné názvy.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@ColorGroups
$ #Color groups#
 Toto menu umožňuje nastavit barvy různých položek rozhraní nebo nastavit barvy na implicitní.

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
$ #Třídění skupin#
 Třídění skupin souborů může být použito v módech třídění ~souborového panelu~@FilePanel@
#podle názvu# a #podle přípony#. Třídění skupin jsou aplikovány stisknutím
#Shift+F11# a umožňují definovat dodatečné pravidla třídění souborů,
doplňujících již používaná.

 Každé třídění skupin obsahuje jednu, nebo více ~souborových masek~@FileMasks@,
ohraničených čárkami. Pokud je pozice jednoho třídění skupin v seznamu skupin
výše, než jiné a je provedeno vzestupné třídění, všechny soubory náležející
do této skupiny souborů budou výše než ty, náležející do skupin následujích.

 Příkaz #Upravit třídění skupin# v ~menu Příkazy~@CmdMenu@ je určen pro
mazání, vytváření a ůpravu třídění skupin, pomocí Del, Ins a F4. Skupiny
nad předělem menu jsou určeny na začátek souborového panelu a v nich zahrnuté
soubory budou umístěny výše než ty, které nejsou zahrnuty v žádné skupině.
Skupiny pod předělem menu jsou určeny na konec souborového panelu a v nich
zahrnuté soubory budou umístěny níže než ty, které nejsou zahrnuty v žádné skupině.


@FileMasks
$ #Masky souborů#
 Masky souborů jsou v příkazech Faru často použity pro výběr jednotlivých
souborů a adresářů, nebo jejich skupin. Masky mohou obsahovat společné znaky
názvu souboru, značky ('*' a '?') a speciální výrazy:

 #*#           jakékoliv číslo, nebo znak;

 #?#           jakýkoliv samotný znak;

 #[cx-z]#     jakýkoliv znak obsažený mezi hranatými závorkami.
             Oba oddělovací znaky a mezery jsou povoleny.

 Například soubory ftp.exe, fc.exe a f.ext mohou být vybrány použitím
masky f*.ex?, maska *co* vybere color.ini a edit.com, maska [c-ft]*.txt
vybere config.txt, demo.txt, faq.txt a tips.txt.

 V mnoha příkazech Faru můžete zadat několik souborových masek oddělených
čárkami, nebo středníky. Například, pro výběr všech dokumentů můžete
v příkazu "Označit" zadat #*.doc,*.txt,*.wri#.

 Každou masku můžete uzavřít (ale ne celý seznam) mezi úvozovky.
Tohle provedete například tehdy, když maska obsahuje nějaký oddělovací
znak (čárku nebo sředník), tím nebude maska zaměněna za seznam.

 File mask surrounded with slashes #/# is treated as ~Perl regular expression~@RegExp@.

 Example:
 #/(eng|rus)/i# - any files with filenames containing string “eng” or “rus”,
the character case is not taken into account.

 #Vyřazovascí maska# je jedna, nebo několik souborových masek, které se nesmí shodovat s maskami,
které se shodují se soubory. Vyřazovací maska je oddělena od hlavní masky
znakem '#|#'.

 Příklad použití vyřazovacích masek:

 1. *.cpp
    Všechny soubory s příponou #cpp#.
 2. *.*|*.bak,*.tmp
    Všechny soubory kromě souborů s příponou #bak# a #tmp#.
 3. *.*|
    Tato maska je chybná - znak | je zadán, ale samotná
    maska není specifikovaná.
 4. *.*|*.bak|*.tmp
    Také chyba - znak | nemůže být v masce opakován více,
    než jednou.
 5. |*.bak
    Stejné jako *|*.bak
 6. *.*|/^pict\d{1,3}\.gif$/i
    All files except for pict0.gif — pict999.gif, disregard the character case.

 Čárka (nebo středník) je použita pro oddělení jedné souborové masky
od druhé a znak '|' odděluje vložené masky od vyřazovacích masek.

 File masks can be joined into ~groups~@MaskGroupsSettings@.


@SelectFiles
$ #Vybírání souborů#
 ~File panel~@FilePanel@ items (files and folders) can be selected
for group processing. Pokud nejsou označeny žádné soubory,
bude zpracován pouze soubor pod kurzorem.

 #Keyboard Selection#

 #Ins# toggles selection on the item pod kurzorem a posune
kurzor dolu.

#Shift+kurzorová šipka# přesouvá kurzor while selecting or deselecting
items along the way. The action (selection or deselection) depends
on the state of the item under cursor before pressing the key
combination.

 #Num +# a #Num -# provádí označení a odznačení skupin souborů
pomocí jedné, nebo několika ~souborových masek~@FileMasks@. #†#

 #Num *# obrací aktuální výběr. #†#

 #Ctrl+<Num +># a #Ctrl+<Num -># označí a odznačí všechny
soubory se stejnou příponou, jako má soubor pod kurzorem.

 #Alt+<Num +># a #Alt+<Num -># označí a odznačí všechny soubory
se stejným názvem, jaký má soubor pod kurzorem.

 #Shift+<Num +># a #Shift+<Num -># označí a odznačí všechny soubory.

 #Ctrl+<Num *># obrátí aktuální výběr a zahrne i adresáře.

 #Alt+<Gray *># inverts the current selection on files only,
folders are deselected.

 #Ctrl+M# obnovuje předchozí vybranou skupinu.

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
$ #Kopírování, přesouvání, přejmenovávání a vytváření linků#
 Následující příkazy mohou být použity pro kopírování, přesouvání a přejmenovávání souborů a adresářů:

 Kopírování ~vybraných~@SelectFiles@ souborů                                  #F5#

 Kopírování souboru pod kurzorem, bez ohledu na výběr    #Shift+F5#

 Přejmenování nebo přesunutí vybraných souborů                 #F6#

 Přejmenování nebo přesunutí souboru pod kurzorem,       #Shift+F6#
 bez ohledu na výběr

 Vytvoření ~souborových linků~@HardSymLink@                               #Alt+F6#

 For a folder: if the folder at the specified target path (relative
or absolute) exists, the source folder will be copied / moved inside the
target folder. Otherwise, a new folder will be created at the target
path and the contents of the source folder will be copied / moved into
the newly created folder.

 For example, when moving #c:\folder1\# to #d:\folder2\#:

 - ^<wrap>if #d:\folder2\# exists, the contents of #c:\folder1\# will be moved into
#d:\folder2\folder1\#. Otherwise, the contents of #c:\folder1\# will be moved into the newly
created #d:\folder2\#.

 Pokud je zapnuta volba "#Zpracovat více míst určení#", můžete
ve vstupním dialogu určit několik cílů pro kopírování, nebo přesouvání.
V tomto případě musí být cíle odděleny znakem "#;#" nebo "#,#". Pokud
název cíle obsahuje tyto znaky, musí být uzavřen v úvozovkách.

 Pokud si přejete před kopírováním vytvořit cílovou složku, ukončete
název zpětným lomítkem.

 If ~Panel.Tree.TurnOffCompletely~@Panel.Tree.TurnOffCompletely@
parameter in ~far:config~@FarConfig@ is set to “false,” you can use
~Find folder~@FindFolder@ dialog to select the target path.
 The following shortcuts open the dialog with different pre-selected folders:
 - ^<wrap>#F10# provézt výběr adresáře z adresářového stromu aktivního panelu.
 - ^<wrap>#Alt+F10# provézt výběr adresáře z adresářového stromu pasivního panelu.
 - ^<wrap>#Shift+F10# umožňuje pro zadání cesty v kopírovacím dialogu otevřít adresářový strom. Ppokud
je vloženo několik cest, je brána v úvahu pouze první.

 Pokud je povolena volba "#Zpracovat více míst určení#",
dialog vybraný ve stromu je připojen k editačnímu řádku.

 Schopnosti kopírování, přesouvání a přejmenovávání pro pluginy
závisí na funkčnosti modulů pluginů.

 The #Access rights# parameter is valid only for the NTFS file system
and controls how access rights of the created files and folders are set.
The #Default# option leaves access rights processing to the operating system.
The #Copy# option applies the access rights of the original objects. The
#Inherit# option applies the inheritable access rights of the
destination’s parent folder.

 The “#Already existing files#” parameter controls Far behavior
pokud již cílový soubor existuje.

 Possible values:
 - ^<wrap>#Ask# - a ~confirmation dialog~@CopyAskOverwrite@ will be shown;
 - #Overwrite# - all target files will be replaced;
 - #Skip# - target files will not be replaced;
 - #Rename# - existing target files will stay unchanged, copied files will be renamed;
 - #Append# - target file will be appended with the file being copied;
 - #Pouze nové soubory# - soubory, pro které je datum úpravy
novější, než datum úpravy cílového souboru;
 - #Also ask on R/O files# - controls whether an additional confirmation
dialog should be displayed for the read-only files.

 Nastavení "Používat kopírovací rutiny systému" z dialogu ~Nastavení Systému~@SystemSettings@,
zajistí používání kopírovací funkce Windows CopyFileEx, místo interních funkcí pro kopírování souborů,
implementovaných ve Faru. To může být užitečné na NTFS, protože CopyFileEx kopíruje rozšířené
atributy souborů. If this option is off, the internal
implementation of the file copy routine is used. The internal
function is also used if the source file is encrypted and is being
copied to a different volume.

 The “#Copy contents of symbolic links#” parameter controls the
~logic~@CopyRule@ of ~symbolic links~@HardSymLink@ processing.

 Při přesouvání souborů, pro určení, zda má být operace provedena
jako kopírování s následujícím vymazáním, nebo jako přímé přesunutí
(na jednom fyzickém disku), bere Far v úvahu ~symbolické linky~@HardSymLink@
Windows 2000/XP.

 Far zachází s kopírováním do #con# stejně, jako s kopírováním do #nul#
nabo #\\\\.\\nul# - to znamená, že soubor je z disku přečten, ale
není nikam zapsán.

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
$ #Pevné a Symbolické linky#
 Pomocí #Alt+F6# můžete na NTFS partition vytvářet #pevné linky# a
#symbolické linky#.

 #Pevné linky#

 #Pevný link# je dodatečný adresářový údaj pro daný soubor. Pokud
je pevný link vytvořen, není kopírován soubor, ale vrací další název,
nebo lokaci, dokud jeho předchozí název a lokace zůstává neporušena.
Protože je čas vytvoření pevného linku a čas vytvoření originálního souboru
stejný, nelze je od sebe rozeznat. Jediný rozdíl je v tom, že pro pevné
linky nelze vytvořit krátký název a proto není vidět pro programy pod DOSem.

 Pokud se změní velikost, nebo datum souboru, všechny odpovídající
adresářové údaje budou automaticky updatovány. Pokud bude soubor smazán,
nebude smazán fyzicky, dokud nebudou smazány všechny pevné linky. Pořadí
mazání nemá vliv. Když je pevný link smazán do Koše, číslo linků souboru
nebude změněno.

 Far může pevné linky vytvářet, může zobrazovat čísla pevných linků
souborů v oddělených sloupcích (implicitně je to poslední sloupec v
devátém módu panelu) a třídit doubory podle čísel pevných linků.

 Pevné linky mohou být vytvořeny jen na stejném disku, na kterém se nachází zdrojový soubor.

 #Directory junctions#

 Directory junctions je technologie, umožňující namapovat kterýkoliv lokální adresář
do kteréhokoliv jiného lokálního adresáře. Například, pokud adresář D:\\JUNCTION má jako
svůj cíl C:\\WINNT\\SYSTEM32, program přistupující k D:\\JUNCTION\\DRIVERS vlastně přistupuje k
C:\\WINNT\\SYSTEM32\\DRIVERS.

 Directory junctions can not point to network folders.

 Under Windows 2000 to neumožňuje výtvářet symbolické linky ukazující na adresáře na jednotkách
CD-ROM, ale toto omezení může být obejíto namountováním CD-ROM do adresáře
na NTFS partition.

 #Symbolic links#

 NTFS supports symbolic links starting from Windows Vista (NT 6.0). It's an
improved version of directory junctions - symbolic links can also point to files
and non-local folders, relative paths also supported.

 By default, only members of "Administrators" group can create symbolic links,
this can be changed in the local security settings.


@ErrCopyItSelf
$ #Chyba: kopírování/přesun sám na sebe.#
 Nemůžete kopírovat, nebo přesouvat soubory, nebo adresáře samy na sebe.

 Tato chyby se může stát pod Windows 2000/XP, pokud jsou dva adresáře
a jeden z nich je ~symbolický link~@HardSymLink@ na druhý.


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
$ #Chyba: kopírování, nebo přesun souboru s několika proudy#
 Zdrojový soubor obsahuje více, než jeden datový proud, nebo cílový
souborový systém nepodporuje soubory s více proudy.

 Proudy jsou vlastnost souborového systému NTFS, umožňující připojit
k souboru další informace (například, autorovo jméno, nadpis, klíčová
slova, nebo nějaká jiná data). Tyto informace jsou uloženy spolu se
souborem a jsou pro proramy, které nepodporují datové proudy, neviditelné.
Například, proudy používané Průzkumníkem Windows pro uložení dalších
vlastností souborů (přehled). Souborové systémy FAT/FAT32 proudy nepodporují.

 Pro kompletní zkopírování souboru (spolu se všemi jeho proudy), zapněte
volbu "#Používat kopírovací rutiny systému#" v dialogu
~Nastavení systému~@SystemSettings@.

 Pokud kopírujete soubor s několika proudy na svazek se souborovým
systémem jiným než NTFS, příjdete o informace - bude zkopírován jen
hlavní proud.


@ErrLoadPlugin
$ #Error: plugin not loaded#
 This error message can appear in the following cases:

 1. ^<wrap>A dynamic link library not present on your system is required for correct operation of the plugin.
 2. ^<wrap>For some reason, the plugin returned an error code telling the system to abort plugin loading.
 3. ^<wrap>The DLL file of the plugin is corrupt.


@ScrSwitch
$ #Přepínání obrazovek#
 Far umožňuje otevřít najednou několik instancí interního prohlížeče
a editoru. Pro přepínání mezi panely a těmito obrazovkami můžete použít
#Ctrl+Tab#, #Ctrl+Shift+Tab#, nebo #F12#. #Ctrl+Tab# přepíná na další obrazovku,
#Ctrl+Shift+Tab# na předchozí a #F12# zobrazuje seznam všech dostupných
obrazovek.

 Číslo prohlížečů a editorů na pozadí je zobrazeno v levém horním rohu
levého panelu. Může být vypnuto v dialogu ~Nastavení Panelů~@PanelSettings@.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@ApplyCmd
$ #Aplikovat příkaz#
 S položkou #Aplikovat příkaz# z ~menu Soubory~@FilesMenu@ je možné
aplikovat příkaz pro každý vybraný soubor. Pro označení názvu souboru
mohou být použity stejné ~speciální symboly~@MetaSymbols@ jako v dialogu
~Závislosti souborů~@FileAssoc@.

 Například 'type !.!' vytiskne na obrazovku, jeden po druhém, všechny
vybrané soubory a příkaz 'rar32 m !.!.rar !.!' přesune všechny vybrané
soubory do archívů RAR, se stejnými názvy. Příkaz 'explorer /select,!.!'
spustí Průzkumníka Windows a nastaví kurzor na aktuální soubor, nebo
adresář.

 Podívejte se rovněž na ~"Příkazy operačního systému"~@OSCommands@


@OSCommands
$ #Příkazy operačního systému#
 Manažer Far provádí následující příkazy operačního systému:

 #CLS#
 Vymaže obrazovku.

 #disk:#
 Pro změnu aktuálního disku v aktivním panelu na zadaný disk.

 #CD [disk:]cesta# nebo #CHDIR [disk:]cesta#
 Pro změnu aktuální cesty v aktivním panelu na zadanou cestu.
Pokud je určeno písmeno diskové jednotky, je aktuální disk změněn.
Pokud panel zobrazuje souborové systémy emulované ~pluginy~@Plugins@,
může být příkaz "CD" na příkazovém řádku použit pro změnu adresáře v
souborovém systému pluginu. Narozdíl od "CD", příkaz "CHDIR" vždy zachází
se specifikovaným parametrem, jako s opravdovým názvem adresáře, nedbajíce
na typ souborového panelu.
 The #CD ~~# command changes to the home directory (if there is no
real “~~” file or directory in the current directory). The home
directory is specified in the #Use home dir# option of the
~Command line settings~@CmdlineSettings@ dialog. By default, it is the
string “%FARHOME%” denoting the Far Manager home directory.

 #CHCP [nnn]#
 Displays or sets the active code page number. Parameter “nnn”
specifies the code page number to set. “CHCP” without a parameter
displays the active code page number.

 #SET proměnná=[řetězec]#
 Nastaví systémovou proměnnou "proměnná" na hodnotu "řetězec". Pokud není
"řetězec" specifikován, bude systémová proměnná "proměnná" odstraněna.
Při startu manažer Far nastaví několik ~systémových proměnných~@FAREnv@ sám.

 #IF [NOT] EXIST název_souboru příkaz#
 Provede příkaz "příkaz" pokud soubor "název_souboru" existuje. Prefix "NOT" -
provede příkaz pouze tehdy, pokud je podmínka nepravdivá.

 #IF [NOT] DEFINED proměnná příkaz#
 Podmínka "DEFINED" pracuje podobně jako "EXISTS", ale bere název systémové
proměnné a vrací true pokud je systémová proměnná definovná.
 Příkaz "IF" může být vložen, například příkaz "příkaz"  bude proveden pokud
soubor "soubor1" existuje, soubor "soubor2" neexistuje a systémová
proměnná "proměnná" je definovaná:
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

 Poznámka:

 1. ^<wrap>Jakýkoliv jiný příkaz bude odeslán procesoru
operačního systému.
 2. ^<wrap>Vypsané příkazy pracují v:
 - ~Příkazovém řádku~@CmdLineCmd@
 - Dialogu ~Aplikovat příkaz~@ApplyCmd@
 - ~Menu uživatele~@UserMenu@
 - Dialogu ~Závislosti souborů~@FileAssoc@


@FAREnv
$ #Systémové proměnné#
 Při startu manažer Far nastavuje následující systémové proměnné
přístupné dětským procesům:

 #FARHOME#            ^<wrap>path to the folder containing main Far executable module.

 #FARPROFILE#         ^<wrap>path to the folder containing roaming user data (Far & plugins settings, additional plugins etc.)

 #FARLOCALPROFILE#    ^<wrap>path to the folder containing local user data (histories, plugin cache etc.)

 #FARLANG#            název aktuálního jazyku rohraní.

 #FARUSER#            jméno uživatele předané v /u parametru ~příkazové řádky~@CmdLine@.

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
$ #Nastavení příkazů maker#
 Pro určení dalších nastavení ~příkazů maker~@KeyMacro@, začněte, nebo
zakončete nahrávání makra s #Ctrl+Shift+<.># místo #Ctrl+<.># a vyberte
v dialogu požadovanou volbu:

 #Sequence:#
 Allows to edit the recorded key sequence.

 #Description:#
 Allows to edit the description of key sequence.

 #Povolit výstup na obrazovku dokud se provádí makro#
 Pokud tato volba není během spuštění příkazu makra zapnuta,
manažer Far nepřekreslí obrazovku. Všechny updaty budou
zobrazeny po ukončení přehrávání příkazu makra.

 #Spustit po nastartování Faru#
 Umožňuje spustit příkaz makra ihned po spuštění manažeru Far.

 Spouštěcí podmínky příkazu makra:

 #Panel pluginů#
 [x] - ^<wrap>spustit jedině, pokud je aktuální panel plugin panel
 [ ] - spustit jedině, pokud je aktuální panel souborový panel
 [?] - ignorovat typ panelu

 #Spustit pro adresáře#
 [x] - ^<wrap>spustit jedině, pokud je adresář pod kurzorem panelu
 [ ] - spustit jedině, pokud je soubor pod kurzorem panelu
 [?] - spustit pro obojí, adresáře i soubory

 #Existuje výběr#
 [x] - ^<wrap>spustit jedině, pokud jsou označeny soubory/adresáře v panelu, nebo vybraný blok v editoru
 [ ] - spustit jedině, pokud nejsou označeny soubory/adresáře v panelu, nebo není vybraný blok v editoru
 [?] - ignorovat stav výběru bloku/souboru

 Other execution conditions:

 #Prázdný příkazový řádek#
 [x] - ^<wrap>spustit jedině, pokud je příkazový řádek prázdný
 [ ] - spustit jedině, pokud není příkazový řádek prázdný
 [?] - ignorovat stav příkazového řádku

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
