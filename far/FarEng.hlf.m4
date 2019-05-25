m4_include(`farversion.m4')m4_dnl
.Language=English,English
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
@Interface.DelShowSelected=Interface.DelHighlightSelected
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
@XLat.Rules1=XLat.Tables
@XLat.Rules2=XLat.Tables
@XLat.Rules3=XLat.Tables
@XLat.Table1=XLat.Tables
@XLat.Table2=XLat.Tables

@Contents
$^#File and archive manager#
`$^#'FULLVERSIONNOBRACES`#'
$^#Copyright © 1996-2000 Eugene Roshal#
`$^#Copyright © 2000-'COPYRIGHTYEAR` Far Group#'
   ~Help file index~@Index@
   ~How to use help~@Help@

   ~About Far~@About@
   ~License~@License@

   ~Command line switches~@CmdLine@
   ~Keyboard reference~@KeyRef@
   ~Plugins support~@Plugins@
   ~Overview of plugin capabilities~@PluginsReviews@

   ~Panels:~@Panels@  ~File panel~@FilePanel@
            ~Tree panel~@TreePanel@
            ~Info panel~@InfoPanel@
            ~Quick view panel~@QViewPanel@
            ~Drag and drop files~@DragAndDrop@
            ~Customizing file panel view modes~@PanelViewModes@
            ~Selecting files~@SelectFiles@

   ~Menus:~@Menus@   ~Left and right menus~@LeftRightMenu@
            ~Files menu~@FilesMenu@
            ~Commands menu~@CmdMenu@
            ~Options menu~@OptMenu@

   ~Find file~@FindFile@
   ~History~@History@
   ~Find folder~@FindFolder@
   ~Compare folders~@CompFolders@
   ~User menu~@UserMenu@
   ~Change drive menu~@DriveDlg@

   ~File associations~@FileAssoc@
   ~Operating system commands~@OSCommands@
   ~Folder shortcuts~@FolderShortcuts@
   ~Filters menu~@FiltersMenu@
   ~Screens switching~@ScrSwitch@
   ~Task list~@TaskList@
   ~Hotplug devices list~@HotPlugList@

   ~System settings~@SystemSettings@
   ~Panel settings~@PanelSettings@
   ~Tree settings~@TreeSettings@
   ~Interface settings~@InterfSettings@
   ~Dialog settings~@DialogSettings@
   ~Menu settings~@VMenuSettings@
   ~Command line settings~@CmdlineSettings@

   ~Files highlighting and sort groups~@Highlight@
   ~File descriptions~@FileDiz@
   ~Viewer settings~@ViewerSettings@
   ~Editor settings~@EditorSettings@

   ~Copying, moving, renaming and creating links~@CopyFiles@

   ~Internal viewer~@Viewer@
   ~Internal editor~@Editor@

   ~File masks~@FileMasks@
   ~Keyboard macro commands (macro command)~@KeyMacro@


@Help
$ # Far: how to use help#
    Help screens can have reference items on them that lead to another help
screen. Also, the main page has the "~Help Index~@Index@", which lists all the
topics available in the help file and in some cases helps to find the needed
information faster.

    You can use #Tab# and #Shift+Tab# keys to move the cursor from one
reference item to another, then press #Enter# to go to a help screen describing
that item. With the mouse, you can click a reference to go to the help screen
about that item.

    If text does not completely fit in the help window, a scroll bar is
displayed. In such case #cursor keys# can be used to scroll text.

    You can press #Alt+F1# or #BS# to go back to a previous help screen and
#Shift+F1# to view the help contents.

    Press #Shift+F2# for ~plugins~@Plugins@ help.

    Press #F7# to search for text in the current help file. Search results
will be displayed as links to relevant topics.

    #Help# is shown by default in a reduced windows, you can maximize it by
pressing #F5# "#Zoom#", pressing #F5# again will restore the window to the
previous size.


@About
$ # Far: about#
    #Far# is a text mode file and archive manager for Windows. It supports
#long file names# and provides a wide set of file and folder operations.

    #Far# is #freeware# and #open source# software distributed under the
revised BSD ~license~@License@.

    #Far# does transparent #archive# processing. Files in the archive are
handled similarly as in a folder: when you operate with the archive, Far
transforms your commands into the corresponding external archiver calls.

    #Far# offers a number of service functions as well.


@License
$ # Far: License#

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
$ # Far: command line switches#
  The following switches can be used in the command line:

  #/e[<line>[:<pos>]] <filename>#
  Edit the specified file. After /e you can optionally specify editor start line
and line position.
  For example: far /e70:2 readme.

  #/p[<path>]#
  Search for "main" plugins in the folder given in <path>.
  Several search paths can be specified, separated by ';'.

  Example: #far /p%USERPROFILE%\\Far\\Plugins#

  #/co#
  Forces Far to load plugins from cache only. Plugins are loaded faster this way,
but new or changed plugins are not discovered. Should be used ONLY with a stable
list of plugins. After adding, replacing or deleting a plugin Far should be loaded
without this switch. If the cache is empty, no plugins will be loaded.

  Remarks about switches /p and /co:

  - ^<wrap>if /p is empty, then Far will be loaded with no plugins;
  - ^<wrap>if /p is given with a <path>, then only plugins from <path> will be loaded;
  - ^<wrap>if only the /co switch is given and plugins cache is not empty, then plugins
will be loaded from cache;
  - ^<wrap>/co is ignored, if /p is given;
  - ^<wrap>if /p and /co are not given, then plugins will be loaded from the 'Plugins'
folder, which is in the same folder as Far.exe, and the 'Plugins' folder, which is in the
user profile folder (#%APPDATA%\\Far Manager\\Profile# by default).

  #/m#
  Far will not load macros when started.

  #/ma#
  Macros with the "Run after Far start" option set will not be run when Far is started.

  #/s <profilepath> [<localprofilepath>]#
  Custom location for Far configuration files - overrides Far.exe.ini.

  #/u <username>#
  Allows to have separate registry settings for different users.
  Affects only 1.x Far Manager plugins
  For example: far /u guest

  Far Manager will set the ~environment variable~@FAREnv@ "FARUSER" to the value <username>.

  #/v <filename>#
  View the specified file. If <filename> is `#-#', data is read from the stdin.

  For example, "dir|far /v -" will view dir command output.

  If the input stream is empty when using '-' (for example, you have not specified
the "dir" command in the provided example), Far will wait for the end of data
in the input stream until you press Ctrl+Break.

  #/w[-]#
  Stretch to console window instead of console buffer or vice versa.

  #/t templateprofile#
  Location of Far template configuration file - overrides Far.exe.ini.

  #/title[:<valuestring>]#
  If <valuestring> is given, use it as the window title;
  otherwise, inherit the console window's title.

  #/clearcache [profilepath [localprofilepath]]#
  Clear plugins cache.

  #/export <out.farconfig> [profilepath [localprofilepath]]#
  Export settings to file out.farconfig.

  #/import <in.farconfig> [profilepath [localprofilepath]]#
  Import settings from file in.farconfig.

  #/ro#
  Let's you work without writing any changes to Far configuration.
Overrides Far.exe.ini.

  #/ro-#
  Normal (Read-Write) mode of Far configuration - overrides Far.exe.ini.

  #/set:<parameter>=<value>#
  Override the configuration parameter, see ~far:config~@FarConfig@ for details.

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
$ #Keyboard reference#

 ~Panel control~@PanelCmd@

 ~Command line~@CmdLineCmd@

 ~File management and service commands~@FuncCmd@

 ~Mouse: wheel support~@MsWheel@

 ~Menu control commands~@MenuCmd@

 ~Common controls~@MiscCmd@

 ~Special commands~@SpecCmd@


@MenuCmd
$ #Menu control commands#
 #Common menu and drop-down list commands#

  Filter menu or list items                          #Ctrl+Alt+F,RAlt#
  Lock filter                                             #Ctrl+Alt+L#

  See also the list of ~macro keys~@KeyMacroMenuList@, available in the menus.


@PanelCmd
$ #Panel control commands#
    #Common panel commands#

  Change active panel                                            #Tab#
  Swap panels                                                 #Ctrl+U#
  Re-read panel                                               #Ctrl+R#
  Toggle info panel                                           #Ctrl+L#
  Toggle ~quick view panel~@QViewPanel@                                     #Ctrl+Q#
  Toggle ~tree panel~@TreePanel@                                           #Ctrl+T#
  Hide/show both panels                                       #Ctrl+O#
  Temporarily hide both panels                        #Ctrl+Alt+Shift#
    (as long as these keys are held down)
  Hide/show inactive panel                                    #Ctrl+P#
  Hide/show left panel                                       #Ctrl+F1#
  Hide/show right panel                                      #Ctrl+F2#
  Change panels height                             #Ctrl+Up,Ctrl+Down#
  Change current panel height          #Ctrl+Shift+Up,Ctrl+Shift+Down#
  Change panels width                           #Ctrl+Left,Ctrl+Right#
    (when the command line is empty)
  Restore default panels width                          #Ctrl+Numpad5#
  Restore default panels height                     #Ctrl+Alt+Numpad5#
  Show/Hide functional key bar at the bottom line.            #Ctrl+B#
  Toggle total and free size show mode                  #Ctrl+Shift+S#
   in bytes (if possible) or with size suffices K/M/G/T

    #File panel commands#

  ~Select/deselect file~@SelectFiles@                        #Ins, Shift+Cursor keys#
                                                  #Right mouse button#
  Select group                                                #Gray +#
  Deselect group                                              #Gray -#
  Invert selection                                            #Gray *#
  Select files with the same extension as the          #Ctrl+<Gray +>#
    current file
  Deselect files with the same extension as the        #Ctrl+<Gray ->#
    current file
  Invert selection including folders                   #Ctrl+<Gray *>#
    (ignore command line state)
  Select files with the same name as the current file   #Alt+<Gray +>#
  Deselect files with the same name as the current      #Alt+<Gray ->#
    file
  Select all files                                    #Shift+<Gray +>#
  Deselect all files                                  #Shift+<Gray ->#
  Restore previous selection                                  #Ctrl+M#

  Scroll long names and descriptions              #Alt+Left,Alt+Right#
                                                    #Alt+Home,Alt+End#

  Set brief view mode                                     #LeftCtrl+1#
  Set medium view mode                                    #LeftCtrl+2#
  Set full view mode                                      #LeftCtrl+3#
  Set wide view mode                                      #LeftCtrl+4#
  Set detailed view mode                                  #LeftCtrl+5#
  Set descriptions view mode                              #LeftCtrl+6#
  Set long descriptions view mode                         #LeftCtrl+7#
  Set file owners view mode                               #LeftCtrl+8#
  Set file links view mode                                #LeftCtrl+9#
  Set alternative full view mode                          #LeftCtrl+0#

  Toggle hidden and system files displaying                   #Ctrl+H#
  Toggle long/short file names view mode                      #Ctrl+N#

  Hide/Show left panel                                       #Ctrl+F1#
  Hide/Show right panel                                      #Ctrl+F2#

  Sort files in the active panel by name                     #Ctrl+F3#
  Sort files in the active panel by extension                #Ctrl+F4#
  Sort files in the active panel by last write time          #Ctrl+F5#
  Sort files in the active panel by size                     #Ctrl+F6#
  Keep files in the active panel unsorted                    #Ctrl+F7#
  Sort files in the active panel by creation time            #Ctrl+F8#
  Sort files in the active panel by access time              #Ctrl+F9#
  Sort files in the active panel by description             #Ctrl+F10#
  Sort files in the active panel by file owner              #Ctrl+F11#
  Display ~sort modes~@PanelCmdSort@ menu                                   #Ctrl+F12#
  Use group sorting                                        #Shift+F11#
  Show selected files first                                #Shift+F12#

  Create a ~folder shortcut~@FolderShortcuts@              #Ctrl+Shift+0# to #Ctrl+Shift+9#
  Jump to a folder shortcut               #RightCtrl+0# to #RightCtrl+9#

      If the active panel is a ~quick view panel~@QViewPanel@, a ~tree panel~@TreePanel@ or
    an ~information panel~@InfoPanel@, the directory is changed not on the
    active, but on the passive panel.

  Copy the names of selected files to the clipboard         #Ctrl+Ins#
   (if the command line is empty)
  Copy the files to clipboard                                 #Ctrl+C#
   (ignore command line state)
   ^<wrap>Files, copied from the panels, can be pasted to other applications,
e.g. Explorer.
  Copy the names of selected files to the clipboard   #Ctrl+Shift+Ins#
   (ignore command line state)
  Copy full names of selected files to the clipboard   #Alt+Shift+Ins#
   (ignore command line state)
  Copy network (UNC) names of selected files to the     #Ctrl+Alt+Ins#
   clipboard (ignore command line state)

  See also the list of ~macro keys~@KeyMacroShellList@, available in the panels.

  Notes:

  1. ^<wrap>If "Allow reverse sort modes" option in ~Panel settings~@PanelSettings@
dialog is enabled, pressing the same sort key second time toggles the sort direction
from ascending to descending and vice versa;

  2. ^<wrap>If #Alt+Left# and #Alt+Right# combinations, used to scroll long names
and descriptions, work only with non-numpad #Left# and #Right# keys. This is due to
the fact that when #Alt# is pressed, numpad cursor keys are used to enter characters
via their decimal codes.

  3. ^<wrap>The key combination #Ctrl+Alt+Ins# puts the following text into the clipboard:
       ^<wrap>* for network drives - the network (UNC) name of the file object;
       ^<wrap>* for local drives - the local name of the file taking into account
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

    See also: common ~menu~@MenuCmd@ keyboard commands.


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
'*' and '?'.

    Insertion of text, pasted from clipboard (#Ctrl+V# or #Shift+Ins#), to the
fast find dialog will continue as long as there is a match found.

    It is possible to use the transliteration function while entering text in
the search field. If used the entered text will be transliterated and a new
match corresponding to the new text will be searched. See ~XLat.Flags~@XLat.Flags@ on how
to set the hotkey for the transliteration.

   See also the list of ~macro keys~@KeyMacroSearchList@, available in fast find.

@CmdLineCmd
$ #Command line commands#
 #Common command line commands#

  Character left                                         #Left,Ctrl+S#
  Character right                                       #Right,Ctrl+D#
  Word left                                                #Ctrl+Left#
  Word right                                              #Ctrl+Right#
  Start of line                                            #Ctrl+Home#
  End of line                                               #Ctrl+End#
  Delete char                                                    #Del#
  Delete char left                                                #BS#
  Delete to end of line                                       #Ctrl+K#
  Delete word left                                           #Ctrl+BS#
  Delete word right                                         #Ctrl+Del#
  Copy to clipboard                                         #Ctrl+Ins#
  Paste from clipboard                                     #Shift+Ins#
  Previous command                                            #Ctrl+E#
  Next command                                                #Ctrl+X#
  Clear command line                                          #Ctrl+Y#

 #Insertion commands#

  Insert current file name from the active panel   #Ctrl+J,Ctrl+Enter#

     In the ~fast find~@FastFind@ mode, #Ctrl+Enter# does not insert a
     file name, but instead cycles the files matching the
     file mask entered in the fast find box.

  Insert current file name from the passive panel   #Ctrl+Shift+Enter#
  Insert full file name from the active panel                 #Ctrl+F#
  Insert full file name from the passive panel                #Ctrl+;#
  Insert network (UNC) file name from the active panel    #Ctrl+Alt+F#
  Insert network (UNC) file name from the passive panel   #Ctrl+Alt+;#

  Insert path from the left panel                             #Ctrl+[#
  Insert path from the right panel                            #Ctrl+]#
  Insert network (UNC) path from the left panel           #Ctrl+Alt+[#
  Insert network (UNC) path from the right panel          #Ctrl+Alt+]#

  Insert path from the active panel                     #Ctrl+Shift+[#
  Insert path from the passive panel                    #Ctrl+Shift+]#
  Insert network (UNC) path from the active panel        #Alt+Shift+[#
  Insert network (UNC) path from the passive panel       #Alt+Shift+]#

  Notes:

  1. ^<wrap>If the command line is empty, #Ctrl+Ins# copies selected file names
from a panel to the clipboard like #Ctrl+Shift+Ins# (see ~Panel control commands~@PanelCmd@);

  2. ^<wrap>#Ctrl+End# pressed at the end of the command line, replaces its current contents
with a command from ~history~@History@ beginning with the characters that are in the command line,
if such a command exists. You can press #Ctrl+End# again to go to the next such command.

  3. ^<wrap>Most of the described above commands are valid for all edit strings including edit
controls in dialogs and internal editor.

  4. ^<wrap>#Alt+Shift+Left#, #Alt+Shift+Right#, #Alt+Shift+Home# and #Alt+Shift+End# select
the block in the command line also when the panels are on.

  5. ^<wrap>For local drives, the commands to insert the network (UNC) name of a file object
insert the local name of the file with ~symbolic links~@HardSymLink@ expanded.


@FuncCmd
$ #Panel control commands - service commands#
  Online help                                                     #F1#

  Show ~user menu~@UserMenu@                                                  #F2#

  View                                   #Ctrl+Shift+F3, Numpad5, F3#

    If pressed on a file, #Numpad5# and #F3# invoke ~internal~@Viewer@,
external or ~associated~@FileAssoc@ viewer, depending upon the file type and
~external viewer settings~@ViewerSettings@.
    #Ctrl+Shift+F3# always calls internal viewer ignoring file associations.
    If pressed on a folder, calculates and shows the size of selected folders.

  Edit                                             #Ctrl+Shift+F4, F4#

    #F4# invokes ~internal~@Editor@, external or ~associated~@FileAssoc@
editor, depending upon the file type and ~external editor settings~@EditorSettings@.
    #Ctrl+Shift+F4# always calls internal editor ignoring file associations.
    #F4# and #Ctrl+Shift+F4# for directories invoke the change file
~attributes~@FileAttrDlg@ dialog.

  ~Copy~@CopyFiles@                                                            #F5#

    Copies files and folders. If you wish to create the destination folder
before copying, terminate the name with a backslash.

  ~Rename or move~@CopyFiles@                                                  #F6#

    Moves or renames files and folders. If you wish to create the destination
folder before moving, terminate the name with a backslash.

  ~Create new folder~@MakeFolder@                                               #F7#

  ~Delete~@DeleteFile@                                     #Shift+Del, Shift+F8, F8#

  ~Wipe~@DeleteFile@                                                       #Alt+Del#

  Show ~menus~@Menus@ bar                                                  #F9#

  Quit Far                                                       #F10#

  Show ~plugin~@Plugins@ commands                                           #F11#

  Change the current drive for left panel                     #Alt+F1#

  Change the current drive for right panel                    #Alt+F2#

  Internal/external viewer                                    #Alt+F3#

    If the internal viewer is used by default, invokes the external viewer
specified in the ~settings~@ViewerSettings@ or the ~associated viewer program~@FileAssoc@
for the file type. If the external viewer is used by default, invokes the
internal viewer.

  Internal/external editor                                    #Alt+F4#

    If the internal editor is used by default, invokes the external editor
specified in the ~settings~@EditorSettings@ or the ~associated editor program~@FileAssoc@
for the file type. If the external editor is used by default, invokes the
internal editor.

  Print files                                                 #Alt+F5#

    If the "Print Manager" plugin is installed then the printing of
    the selected files will be carried out using that plugin,
    otherwise by using internal facilities.

  Create ~file links~@HardSymLink@ (NTFS only)                               #Alt+F6#

    Using hard file links you can have several different file names referring
to the same data.

  Perform ~find file~@FindFile@ command                                   #Alt+F7#

  Display ~commands history~@History@                                    #Alt+F8#

  Toggles the size of the Far console window                  #Alt+F9#

    In the windowed mode, toggles between the current size and the maximum
possible size of a console window. In the fullscreen mode, #Alt+F9# toggles the
screen height between 25 and 50 lines. See ~Interface.AltF9~@Interface.AltF9@ for details.

  Configure ~plugins~@Plugins@.                                    #Alt+Shift+F9#

  Perform ~find folder~@FindFolder@ command                                #Alt+F10#

  Display ~view and edit history~@HistoryViews@                              #Alt+F11#

  Display ~folders history~@HistoryFolders@                                    #Alt+F12#

  Add files to archive                                      #Shift+F1#
  Extract files from archive                                #Shift+F2#
  Perform archive managing commands                         #Shift+F3#
  Edit ~new file~@FileOpenCreate@                                             #Shift+F4#

    When a new file is opened, the same code page is used as in the last opened
editor. If the editor is opened for the first time in the current Far session,
the default code page is used.

  Copy file under cursor                                    #Shift+F5#
  Rename or move file under cursor                          #Shift+F6#

    For folders: if the specified path (absolute or relative) points to an
existing folder, the source folder is moved inside that folder. Otherwise the
folder is renamed/moved to the new path.
    E.g. when moving #c:\folder1\# to #d:\folder2\#:
    - if #d:\folder2\# exists, contents of #c:\folder1\# is
moved into #d:\folder2\folder1\#;
    - otherwise contents of #c:\folder1\# is moved into the
newly created #d:\folder2\#.

  ~Delete file~@DeleteFile@ under cursor                                  #Shift+F8#
  Save configuration                                        #Shift+F9#
  Selects last executed menu item                          #Shift+F10#

  Execute, change folder, enter to an archive                  #Enter#
  Execute in the separate window                         #Shift+Enter#
  Execute as administrator                            #Ctrl+Alt+Enter#

    Pressing #Shift+Enter# on a directory invokes the Windows Explorer and
shows the selected directory. To show a root directory in the Explorer, you
should press #Shift+Enter# on the required drive in the ~Change drive menu~@DriveDlg@.
Pressing #Shift+Enter# on "#..#" opens the current directory in the Explorer.

  Change to the root folder                                   #Ctrl+\\#

  Change folder, enter an archive (also an SFX archive)     #Ctrl+[Shift+]PgDn#

    If the cursor points to a directory, pressing #Ctrl+PgDn# changes to that
directory. If the cursor points to a file, then, depending on the file type,
an ~associated command~@FileAssoc@ is executed or the archive is opened.
    #Ctrl+Shift+PgDn# always opens the archive, regardless of the associated
command configuration.

  Change to the parent folder                              #Ctrl+PgUp#

    If the option "~Use Ctrl+PgUp to change drive~@InterfSettings@" is enabled,
pressing #Ctrl+PgUp# in the root directory switches to the network plugin or
shows the ~Change drive menu~@DriveDlg@.

  Create shortcut to the current folder              #Ctrl+Shift+0..9#

  Use folder shortcut                                 #RightCtrl+0..9#

  Set ~file attributes~@FileAttrDlg@                                         #Ctrl+A#
  ~Apply command~@ApplyCmd@ to selected files                             #Ctrl+G#
  ~Describe~@FileDiz@ selected files                                     #Ctrl+Z#


@DeleteFile
$ #Deleting and wiping files and folders#
    The following hotkeys are used to delete or wipe out files and folders:

    #F8#         - if any files or folders are selected in the panel
                 then the selected group will be deleted, otherwise
                 the object currently under cursor will be deleted;

    #Shift+F8#   - delete only the file under cursor
                 (with no regard to selection in the panel);

    #Shift+Del#  - delete selected objects, skipping the Recycle Bin;

    #Alt+Del#    - Wipe out files and folders.


    Remarks:

    1. ^<wrap>In accordance to ~System Settings~@SystemSettings@ the hotkeys #F8# and
#Shift+F8# do or do not move the deleted files to the Recycle Bin. The
#Shift+Del# hotkey always deletes, skipping the Recycle Bin.

    2. ^<wrap>Before file deletion its data is overwritten with zeros (you can
specify other overwrite characters - see ~System.WipeSymbol~@System.WipeSymbol@), after which the file
is truncated to a zero sized file, renamed to a temporary name and then
deleted.


@ErrCannotExecute
$ #Error: Cannot execute#
    The program you tries to execute is not recognized as an internal or
external command, operable program or batch file.

    When executing the contents of the command line, Far searches for the
executable in the following sequence (sequentially substituting all extensions
listed in the environment variable %PATHEXT%):

  1. The current directory
  2. The directories that are listed in the PATH environment variable
  3. The 32-bit Windows system directory.
  4. The 16-bit Windows system directory.
  5. The Windows directory.


@MiscCmd
$ #Common control commands#
 #Screen grabber#

  Screen grabber                                             #Alt+Ins#

  Screen grabber allows to select and copy to the clipboard any screen area.

  To switch between stream and block selection mode use the #Space# key.
  To move the cursor use #arrow# keys or click the #left mouse button#.
  To select text use #Shift+arrow# keys or drag the mouse while holding the
#left mouse button#.
  To extend or shrink selected area use #Alt+Shift+arrow# keys.
  To move selected area use #Alt+arrow# keys.
  #Enter#, #Ctrl+Ins#, #right mouse button# or #doubleclick# copy
selected text to the clipboard, #Ctrl+<Gray +># appends it to the clipboard
contents, #Esc# leaves the grabbing mode.
  #Ctrl+A# - select whole screen.
  #Ctrl+U# - deselect block.
  #Ctrl+Shift+Left# and #Ctrl+Shift+Right# - extend or shrink selection by 10 characters left or right.
  #Ctrl+Shift+Up# and #Ctrl+Shift+Down# - extend or shrink selection by 5 lines up or down.

 #Keyboard macros#

  Record a ~keyboard macro~@KeyMacro@                                   #Ctrl+<.>#

 #Menus and dropdown lists#

  Enable/disable filtering mode                     #RAlt, Ctrl+Alt+F#
  Lock/unlock filter                                      #Ctrl+Alt+L#

  When in filter mode, you can filter the displayed items by entering
text.

 #Dialogs#

  History in dialog edit controls                 #Ctrl+Up, Ctrl+Down#

  In dialog edit control history you can use #Enter# to copy the current item
to the edit control and #Ins# to mark or unmark an item. Marked items are not
pushed out of history by new items, so you can mark frequently used strings so
that you will always have them in the history.

  Clear history in dialog edit controls                          #Del#

  Delete the current item in a dialog edit line history
  list (if it is not locked)                               #Shift+Del#

  Set the dialog focus to the default element                   #PgDn#

  The following combinations are valid for all edit controls except the
command line, including dialogs and the ~internal editor~@Editor@.

  Insert a file name under cursor to dialog              #Shift+Enter#

  Insert a file name from passive panel to dialog   #Ctrl+Shift+Enter#

  Pressing #Ctrl+Enter# in dialogs executes the default action (pushes the
default button or does another similar thing).

  In dialogs, when the current control is a check box:

  - turn on (#[x]#)                                             #Gray +#
  - turn off (#[ ]#)                                            #Gray -#
  - change to undefined (#[?]#)                                 #Gray *#
    (for three-state checkboxes)

  #Left clicking# outside the dialog works the same as pressing #Esc#.

  #Right clicking# outside the dialog works the same as pressing #Enter#.

  You can move a dialog (window) by dragging it with mouse or by pressing
#Ctrl+F5# and using #arrow# keys.

 #Mouse#

  Clicking the #middle mouse button# in the ~panels~@PanelCmd@ has the same
effect as pressing the #Enter# key with the same modifiers (#Ctrl#, #Alt#,
#Shift#). If the ~command line~@CmdLineCmd@ is not empty, its contents will be
executed.

  Far Manager also supports the ~mouse wheel~@MsWheel@.


@SpecCmd
$ #Special commands#

  ~Version information~@FarAbout@
  ~Configuration editor~@FarConfig@


@MsWheel
$ #Mouse: wheel support#

   #Panels#    Rotating the wheel scrolls the file list without
             changing the cursor position on the screen. Pressing
             the #middle button# has the same effect as pressing
             #Enter#.

   #Editor#    Rotating the wheel scrolls the text without changing
             the cursor position on the screen (similar to #Ctrl+Up#/
             #Ctrl+Down#).

   #Viewer#    Rotating the wheel scrolls the text.

   #Help#      Rotating the wheel scrolls the text.

   #Menus#     Wheel scrolling works as #Up#/#Down# keys.
             Pressing the #middle button# has the same effect as
             pressing #Enter#. It is possible to choose items without
             moving the cursor.

   #Dialogs#   In dialogs, when the wheel is rotated at an edit line
             with a history list or a combo box, the drop-down list
             is opened. In the drop-down list scrolling works the
             same as in menus.

    You can specify the number of lines to scroll at a time in the panels,
editor and viewer (see ~System.MsWheelDelta~@System.MsWheelDelta@).


@Plugins
$ #Plugins support#
    Plugins can be used to implement new Far commands and emulate file systems.
For example, archives support, FTP client, temporary panel and network browser
are plugins that emulate file systems.

    All plugins are stored in separate folders within the 'Plugins' folder,
which is in the same folder as Far.exe, and the 'Plugins' folder, which is in the
user profile folder (#%APPDATA%\\Far Manager\\Profile# by default).
When detecting a new plugin Far saves information about it and later loads the
plugin only when necessary, so unused plugins do not require additional memory.
But if you are sure that some plugins are useless for you, you can remove them
to save disk space.

    Plugins can be called either from ~Change drive menu~@DriveDlg@ or from
#Plugin commands# menu, activated by #F11# or by corresponding item of
~Commands menu~@CmdMenu@. #F4# in ~"Plugin commands"~@PluginCommands@ menu allows to assign hot
keys to menu items (this makes easier to call them from ~keyboard macros~@KeyMacro@).
This menu is accessible from file panels, dialogs and (only by #F11#) from the
internal viewer and editor. Only specially designed plugins will be shown when
calling the menu from dialogs, the viewer or the editor.

    You can set plugin parameters using ~Plugin configuration~@PluginsConfig@
command from ~Options menu~@OptMenu@.

    File processing operations like copy, move, delete, edit or ~Find file~@FindFile@
work with plugins, which emulate file systems, if these plugins provide
necessary functionality. Search from the current folder in the "Find file"
command requires less functionality than search from the root folder, so try to
use it if search from the root folder does not work correctly.

    Plugins have their own message and help files. You can get a list of
available help on the plugins by pressing

    #Shift+F2# - anywhere in the Far help system

    #Shift+F1# - in the list of plugins (context-dependent help).

    If the plugin has no help file, then context-dependent help will not pop
out.

    If the active panel shows a plugin emulated file system, the command "CD"
in the command line can be used to change the plugin file system folder. Unlike
"CD", "CHDIR" command always treats the specified parameter as a real folder
name regardless a file panel type.

    Use #Alt+Shift+F9# to ~configure plugins~@PluginsConfig@.


@PluginCommands
$ #Plugin commands#
    This menu provides user with ability to use plugins functionality (other
ways are listed in ~"Plugins support"~@Plugins@).
The contents of the menu and actions triggered on menu items selection are
controlled by plugins.

    The menu can be invoked in the following ways:

  - #F11# at file panels or #Plugins# item at ~commands menu~@CmdMenu@, herewith
    the commands intended for use from file panels are shown;
  - #F11# in viewer or editor, herewith the commands intended for use from
    viewer and editor accordingly are shown.

    Each item of plugin commands menu can be assigned a hotkey with #F4#, this
possibility is widely used in ~key macros~@KeyMacro@. The assigned hotkey is
displayed left to the item. The #A# symbol in leftmost menu column means that
the corresponding plugin is written for Far 1.7x and it does not support all
possibilities available in Far 3 (these are, in particular, Unicode characters
in filenames and in editor).

    #Plugin commands# menu hotkeys:

    #Shift+F1#    - help on use for selected menu item. The text of the help
                  is taken from HLF file, associated with the plugin
                  that owns the menu item.
    #F4#          - assign a hotkey for selected menu item. If #Space# is
                  entered, then Far sets the hotkey automatically.
    #F3#          - show plugin technical information.
    #Shift+F9#    - settings of the selected plugin.
    #Alt+Shift+F9# - open ~"Plugins configuration"~@PluginsConfig@ menu.

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
displayed left to the item. The #A# symbol in leftmost menu column means that
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

  * Syntax highlighting in program source texts.
  * Working with FTP-servers (including access through proxy).
  * Search and replace in many files at the same time, using regular
    expressions.
  * Renaming groups of files with support for complex compound masks
    consisting of substitution symbols and templates.
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
  * WinAmp control and MP3-tags modifying.
  * Quake PAK-files processing.
  * Printers control, both connected to PC and network.
  * Connection and debugging of queries to ODBC-compatible
    databases.
  * RAS service control.
  * External programs executing (compilers, converters etc.) while
    editing text in Far editor.
  * Windows help files contents displaying (.hlp and .chm)
  * Calculators with different possibilities.
  * Several games :-)
  * Spell checker functions while editing text in Far editor.
  * Removable drives catalog preparation and much more...

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
$ #Panels #
    Normally Far shows two panels (left and right windows), with different
information. If you want to change the type of information displayed in the
panel, use the ~panel menu~@LeftRightMenu@ or corresponding ~keyboard commands~@KeyRef@.

    See also the following topics for more information:

      ~File panel~@FilePanel@                 ~Tree panel~@TreePanel@
      ~Info panel~@InfoPanel@                 ~Quick view panel~@QViewPanel@

      ~Drag and drop files~@DragAndDrop@
      ~Selecting files~@SelectFiles@
      ~Customizing file panel view modes~@PanelViewModes@


@FilePanel
$ #Panels: file panel#
    The file panel displays the current folder. You can select or deselect
files and folders, perform different file and archive operations. Read
~Keyboard reference~@KeyRef@ for commands list.

    Default view modes of the file panel are:

 #Brief#         File names are displayed within three columns.

 #Medium#        File names are displayed within two columns.

 #Full#          Name, size, date and time of the file are displayed.

 #Wide#          File names and sizes are displayed.

 #Detailed#      File names, sizes, allocation sizes, last write,
               creation, access time and attributes are displayed.
               Fullscreen mode.

 #Descriptions#  File names and ~file descriptions~@FileDiz@

 #Long#          File names, sizes and descriptions.
 #descriptions#  Fullscreen mode.

 #File owners#   File names, sizes and owners.

 #File links#    File names, sizes and number of hard links.

 #Alternative#   File name, size (formatted with commas) and date
 #full#          of the file are displayed.

    You can ~customize file panel view modes~@PanelViewModes@.

    File owners and number of hard links have meaning for NTFS only. Some
file systems may not support file creation and access dates.

    If you wish to change the panel view mode, choose it from the
~panel menu~@LeftRightMenu@. After the mode change or drive change action,
if the initial panel type differs it will be automatically set to the file
panel.

    ~Speed search~@FastFind@ action can be used to point to the required file
by the first letters of its name.

    See also the list of ~macro keys~@KeyMacroShellList@, available in the panels.

@TreePanel
$ #Panels: tree panel#
    The tree panel displays the folder structure of the current disk as a tree.
Within tree mode you can change to a folder quickly and perform folder
operations.

    Far stores folder tree information in the file named #tree3.far# at root
folder of each drive. For read-only drives this information is stored in the
hidden folder Tree.Cache within the folder containing Far.exe. The tree3.far
file doesn't exist by default. It will be automatically created after the first
use of the #Tree Panel# or the #Find Folder# command. If that file exists, Far
updates it with the changes to the tree structure it is aware of. If such
changes were made outside of Far and Tree.far is no longer current, it can be
refreshed by pressing #Ctrl+R#.

    You can find a folder quickly with the help of #speed search# action. Hold
the Alt key and type the folder name until you point to the right folder.
Pressing #Ctrl+Enter# keys simultaneously will select the next match.

    #Gray +# and #Gray -# keys move up and down the tree to the next branch
on the same level.

    See also the list of ~macro keys~@KeyMacroTreeList@, available in the folder tree panel.

@InfoPanel
$ #Panels: info panel#
    The information panel contains the following data:

 1. ^<wrap>#network# names of the computer and the current user (see ~Info panel settings~@InfoPanelSettings@);

    ^<wrap>Far will attempt to determine the type of each of the CD drives available
in the system. Known types are as follows: CD-ROM, CD-RW, CD-RW/DVD, DVD-ROM,
DVD-RW and DVD-RAM. This function is available only for users either with
administrative privileges or all local users, when it's stated explicitly in
the Local Policy Editor (to do this, run #secpol.msc# from the command
prompt, and set the '#Local Policies/Security Options/Devices: Restrict#
#CD-ROM access to locally logged-on user only#' setting to '#Enabled#')

    ^<wrap>For virtual devices (SUBST-disk) the parameters of the primary disk are
shown.

    ^<wrap>#Ctrl+Shift+S# toggles size display mode: float with size suffixes or bytes.
Memory size display mode also changes. ~Quick view panel~@QViewPanel@ and ~file panel~@FilePanel@ status line also affected.
Current mode - far:config #Panel.ShowBytes# (default=false).

 2. ^<wrap>name and type of the #current disk#, type of the file system, network
name, total and free space, disk volume label and serial number;

 3. ^<wrap>#memory# load percentage (100% means all of available memory is used),
size of the installed physical memory (in Vista and newer), total and free size of the physical
memory (available for Windows), virtual memory and paging file;

 4. #folder description# file
    ^<wrap>You can view the contents of the folder description file in full screen by
pressing the #F3# key or clicking the #left mouse button#. To edit or create the
description file, press #F4# or click the #right mouse button#. You can also use
many of the ~viewer commands~@Viewer@ (search, code page selection and so on)
for viewing the folder description file.

    ^<wrap>A list of possible folder description file names can be defined using
"Folder description files" command in the ~Options menu~@OptMenu@.

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

  Drive information
  Memory information
  Description file content
  Plugin panel information
  Power status information

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

    #NetBIOS#
      ^<wrap>The NetBIOS name of the local computer. If the local computer is a node in a cluster, the NetBIOS name of the cluster virtual server.

    #DNS hostname#
      ^<wrap>The DNS host name of the local computer. If the local computer is a node in a cluster, the DNS host name of the cluster virtual server.

    #DNS domain#
      ^<wrap>The name of the DNS domain assigned to the local computer. If the local computer is a node in a cluster, the DNS domain name of the cluster virtual server.

    #DNS fully-qualified#
      ^<wrap>The fully qualified DNS name that uniquely identifies the local computer. This name is a combination of the DNS host name and the DNS domain name, using the form HostName.DomainName. If the local computer is a node in a cluster, the fully qualified DNS name of the cluster virtual server.

    #Physical NetBIOS#
      ^<wrap>The NetBIOS name of the local computer. If the local computer is a node in a cluster, the NetBIOS name of the local computer, not the name of the cluster virtual server.

    #Physical DNS hostname#
      ^<wrap>The DNS host name of the local computer. If the local computer is a node in a cluster, the DNS host name of the local computer, not the name of the cluster virtual server.

    #Physical DNS domain#
      ^<wrap>The name of the DNS domain assigned to the local computer. If the local computer is a node in a cluster, the DNS domain name of the local computer, not the name of the cluster virtual server.

    #Physical DNS fully-qualified#
      ^<wrap>The fully qualified DNS name that uniquely identifies the computer. If the local computer is a node in a cluster, the fully qualified DNS name of the local computer, not the name of the cluster virtual server. The fully qualified DNS name is a combination of the DNS host name and the DNS domain name, using the form HostName.DomainName.

    The output format depends on the domain structure, group policies and DNS settings.

  #User name format#

  Can be one of:

    #Fully Qualified Domain Name#
      ^<wrap>The fully qualified distinguished name (for example, CN=Jeff Smith,OU=Users,DC=Engineering,DC=Microsoft,DC=Com).

    #Sam Compatible#
      ^<wrap>A legacy account name (for example, Engineering\JSmith). The domain-only version includes trailing backslashes (\\).

    #Display Name#
      ^<wrap>A "friendly" display name (for example, Jeff Smith). The display name is not necessarily the defining relative distinguished name (RDN).

    #Unique Id#
      ^<wrap>A GUID string (for example, {4fa050f0-f561-11cf-bdd9-00aa003a77b6}).

    #Canonical Name#
      ^<wrap>The complete canonical name (for example, engineering.microsoft.com/software/someone). The domain-only version includes a trailing forward slash (/).

    #User Principial Name#
      ^<wrap>The user principal name (for example, someone@example.com).

    #Service Principal#
      ^<wrap>The generalized service principal name (for example, www/www.microsoft.com@microsoft.com).

    #DNS Domain#
      ^<wrap>The DNS domain name followed by a backward-slash and the SAM user name.

    The ouput format depends on the domain structure.

@QViewPanel
$ #Panels: quick view panel#
    The quick view panel is used to show information about the selected item in
the ~file panel~@FilePanel@ or ~tree panel~@TreePanel@.

    If the selected item is a file then the contents of the file is displayed.
Many of the ~internal viewer~@Viewer@ commands can be used with the file
displayed in the panel. For files of registered Windows types the type is shown
as well.

    For folders, the quick view panel displays total size, total allocation
size, number of files and subfolders in the folder, current disk cluster size,
real files size, including files slack (sum of the unused cluster parts).

    When viewing reparse points, the path to the source folder is also displayed.

    For folders, the total size value may not match the actual value:

    1. ^<wrap>If the folder or its subfolders contain symbolic links and the option
"Scan symbolic links" in the ~System parameters dialog~@SystemSettings@ is
enabled.

    2. ^<wrap>If the folder or its subfolders contain multiple hard links to the same
file.

    ^<wrap>#Ctrl+Shift+S# toggles size display mode: float with size suffixes or bytes.
~Info panel~@InfoPanel@ and ~file panel~@FilePanel@ status line also affected.
Current mode - far:config #Panel.ShowBytes# (default=false).

    See also the list of ~macro keys~@KeyMacroQViewList@, available in the quick view panel.

@DragAndDrop
$ #Copying: drag and drop files#
    It is possible to perform #Copy# and #Move# file operations using #drag and
drop#. Press left mouse button on the source file or folder, drag it to the
another panel and release the mouse button.

    If you wish to process a group of files or folders, select them before
dragging, click the left mouse button on the source panel and drag the files
group to the other panel.

    You can switch between copy and move operations by pressing the right mouse
button while dragging. Also to move files you can hold the #Shift# key while
pressing the left mouse button.


@Menus
$ #Menus #
    To choose an action from the menu you can press F9 or click on top of the
screen.

    When the menu is activated by pressing #F9#, the menu for the active panel
is selected automatically. When the menu is active, pressing #Tab# switches
between the menus for the left and right panel. If the menus "Files",
"Commands" or "Options" are active, pressing #Tab# switches to the menu of the
passive panel.

    Use the #Shift+F10# key combination to select the last used menu command.

    Read the following topics for information about a particular menu:

     ~Left and right menus~@LeftRightMenu@          ~Files menu~@FilesMenu@

     ~Commands menu~@CmdMenu@                 ~Options menu~@OptMenu@

    See also the list of ~macro keys~@KeyMacroMainMenuList@, available in the main menu.

@LeftRightMenu
$ #Menus: left and right menus#
    The #Left# and #Right# menus allow to change left and right panel settings
respectively. These menus include the following items:

   #Brief#                Display files within three columns.

   #Medium#               Display files within two columns.

   #Full#                 Display file name, size, date and time.

   #Wide#                 Display file name and size.

   #Detailed#             Display file name, size, allocation size,
                        last write, creation and access time,
                        attributes. Fullscreen mode.

   #Descriptions#         File name and ~file description~@FileDiz@.

   #Long descriptions#    File name, size and description.
                        Fullscreen mode.

   #File owners#          File name, size and owner.

   #File links#           File name, size and number of hard links.

   #Alternative full#     File name, formatted size and date.

   #Info panel#           Change panel type to ~info panel~@InfoPanel@.

   #Tree panel#           Change panel type to ~tree panel~@TreePanel@.

   #Quick view#           Change panel type to ~quick view~@QViewPanel@.

   #Sort modes#           Show available sort modes.

   #Show long names#      Show long/short file names.

   #Panel On/Off#         Show/hide panel.

   #Re-read#              Re-read panel.

   #Change drive#         Change current drive.

    See also: common ~menu~@MenuCmd@ keyboard commands.


@FilesMenu
$ #Menus: files menu#
   #View#               ~View files~@Viewer@, count folder sizes.

   #Edit#               ~Edit~@Editor@ files.

   #Copy#               ~Copy~@CopyFiles@ files and folders.

   #Rename or move#     ~Rename or move~@CopyFiles@ files and folders.

   #Link#               Create ~file links~@HardSymLink@.

   #Make folder#        ~Create~@MakeFolder@ new folder.

   #Delete#             Delete files and folders.

   #Wipe#               Wipe files and folders. Before file deletion
                      its data are overwritten with zeros, after
                      which the file is truncated and renamed to
                      a temporary name.

   #Add to archive#     Add selected files to an archive.

   #Extract files#      Extract selected files from an archive.

   #Archive commands#   Perform archive managing commands.

   #File attributes#    ~Change file attributes~@FileAttrDlg@ and time.

   #Apply command#      ~Apply command~@ApplyCmd@ to selected files.

   #Describe files#     Add ~descriptions~@FileDiz@ to the selected files.

   #Select group#       ~Select~@SelectFiles@ a group of files with a wildcard.

   #Deselect group#     ~Deselect~@SelectFiles@ a group of files with a wildcard.

   #Invert selection#   ~Invert~@SelectFiles@ current file selection.

   #Restore selection#  ~Restore~@SelectFiles@ previous file selection after file
                      processing or group select operation.

   Some commands from this menu are also described in the
~File management and service commands~@FuncCmd@ topic.

   See also: common ~menu~@MenuCmd@ keyboard commands.

@CmdMenu
$ #Menus: commands menu#
   #Find file#            Search for files in the folders tree,
                        wildcards can be used.
                        See ~Find file~@FindFile@ for more info.

   #History#              Display the previous commands.
                        See ~History~@History@ for more info.

   #Video mode#           Switch between 25 and 50 lines on the screen.

   #Find folder#          Search for a folder in the folders
                        tree. See ~Find folder~@FindFolder@ for more info.

   #File view history#    Display files ~view and edit history~@HistoryViews@.

   #Folders history#      Display folders ~changing history~@HistoryFolders@.

                        Items in "Folders history" and "File view
                        history" are moved to the end of list after
                        selection. Use #Shift+Enter# to select item
                        without changing its position.

   #Swap panels#          Swap left and right panels.

   #Panels On/Off#        Show/hide both panels.

   #Compare folders#      Compare contents of folders.
                        See ~Compare folders~@CompFolders@ for the
                        detailed description.

   #Edit user menu#       Allows to edit main or local ~user menu~@UserMenu@.
                        You can press #Ins# to insert, #Del# to delete
                        and #F4# to edit menu records.

   #Edit associations#    Displays the list of ~file associations~@FileAssoc@.
                        You can press #Ins# to insert, #Del# to delete
                        and #F4# to edit file associations.

   #Folder shortcuts#     Displays current ~folder shortcuts~@FolderShortcuts@.

   #File panel filter#    Allows to control file panel contents.
                        See ~filters menu~@FiltersMenu@ for the detailed
                        description.

   #Plugin commands#      Show ~plugin commands~@Plugins@ list

   #Screens list#         Show open ~screens list~@ScrSwitch@

   #Task list#            Shows ~active tasks list~@TaskList@.

   #Hotplug devices list# Show ~hotplug devices list~@HotPlugList@.

   See also: common ~menu~@MenuCmd@ keyboard commands.

@OptMenu
$ #Menus: options menu#
   #System settings#       Shows ~system settings~@SystemSettings@ dialog.

   #Panel settings#        Shows ~panel settings~@PanelSettings@ dialog.

   #Tree settings#         Shows ~Tree settings~@TreeSettings@ dialog.
                         Available only if ~Panel.Tree.TurnOffCompletely~@Panel.Tree.TurnOffCompletely@
                         parameter in ~far:config~@FarConfig@ is set to “false.”

   #Interface settings#    Shows ~interface settings~@InterfSettings@ dialog.

   #Dialog settings#       Shows ~dialog settings~@DialogSettings@ dialog.

   #Menu settings#         Shows ~menu settings~@VMenuSettings@ dialog.

   #Command line settings# Shows ~command line settings~@CmdlineSettings@ dialog.

   #AutoComplete settings# Shows ~auto complete settings~@AutoCompleteSettings@.
   
   #InfoPanel settings#    Shows ~info panel settings~@InfoPanelSettings@.

   #Languages#             Select main and help language.
                         Use "Save setup" to save selected languages.

   #Plugins#               Configure ~plugins~@Plugins@.
   #configuration#

   #Plugin manager#        Shows ~plugin manager settings~@PluginsManagerSettings@.
   #settings#

   #File group mask#       Shows ~file group mask settings~@MaskGroupsSettings@.
   #settings#

   #Confirmations#         Shows dialog to turn on or off ~confirmations~@ConfirmDlg@
                         of some operations.

   #File panel modes#      ~Customize file panel view modes~@PanelViewModes@ settings.

   #File descriptions#     ~File descriptions~@FileDiz@ list names and update mode.

   #Folder description#    Specify names (~wildcards~@FileMasks@ are allowed) of
   #files#                 files displayed in the ~Info panel~@InfoPanel@ as folder
                         descriptions.

   #Viewer settings#       External and internal ~viewer settings~@ViewerSettings@.

   #Editor settings#       External and internal ~editor settings~@EditorSettings@.

   #Code pages#            Shows the ~Code pages~@CodePagesMenu@ menu.


   #Colors#                Allows to select colors for different
                         interface items, to change the entire Far
                         colors palette to black and white or to set
                         the colors to default.

   #Files highlighting#    Change ~files highlighting and sort groups~@Highlight@
   #and sort groups#       settings.

   #Save setup#            Save current configuration, colors and
                         screen layout.


   See also: common ~menu~@MenuCmd@ keyboard commands.

@ConfirmDlg
$ #Confirmations#
    In the #Confirmations# dialog you can switch on or off confirmations for
following operations:

    - overwrite destination files when performing file copying;

    - overwrite destination files when performing file moving;

    - overwrite and delete files with "read only" atrtibute;

    - ~drag and drop~@DragAndDrop@ files;

    - delete files;

    - delete folders;

    - interrupt operation;

    - ~disconnect network drives~@DisconnectDrive@ from the Disks menu;

    - delete SUBST-disks from the Disks menu;

    - detach virtual disks from the Disks menu;

    - removal of USB storage devices from the Disks menu;

    - ~reloading~@EditorReload@ a file in the editor;

    - clearing the view/edit, folder and command history lists;

    - exit from Far.


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
    Allows to choose the processing plugin if the host file (e. g. an archive) 
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
$ #Find file#
    This command allows to locate in the folder tree one or more files and
folders matching one or more ~wildcards~@FileMasks@ (delimited with commas). It
can also be used with file systems emulated by ~plugins~@Plugins@.

    Optional text string can be specified to find only files containing this
text. If the string is entered, the option #Case sensitive# selects case
sensitive comparison.

    The option #Whole words# will let to find only the text that is separated
from other text with spaces, tab characters, line breaks or standard
separators, which by default are: #!%^&*()+|{}:"<>?`-=\\[];',./#.

    By checking the #Search for hex# option you can search for the files
containing hexadecimal sequence of the specified bytes. In this case #Case#
#sensitive#, #Whole words#, #Using code page# and #Search for folders#
options are disabled and their values doesn't affect the search process.

    #Not containing# allows to find files #not# containing the specified text or code.

    Dropdown list #Using code page# allows to choose a specific code page
to be used for the search. If the item #All standard code pages# is selected
in the dropdown list, Far will use all standard code pages (ANSI, OEM, UTF-8,
UTF-16, UTF-16 BE), as well as #Favorite# code pages (the list of #favorite#
code pages can be specified in the ~Code pages~@CodepagesMenu@ menu
in the options, editor, or viewer). To search using a custom set of code pages,
select required code pages in the dropdown list with the #Ins# or #Space# keys,
then choose #Selected code pages# menu item.

    If the option #Search in archives# is set, Far also performs the search in
archives with known formats. However, using this option significantly decreases
the performance of the search. Far cannot search in nested archives.

    The #Search for folders# option includes in search list those folders, that
match the wildcards. Also the counter of found files takes account of found
folders.

    The #Search in symbolic links# option allows searching files in
~symbolic links~@HardSymLink@ along with normal sub-folders.

    #Search in alternate streams# - besides the primary data stream (which is
the content of the file itself), allows to search alternate named data streams
supported by some file systems (for example, #NTFS#).

    Search can be performed:

    - in all non-removable drives;

    - in all local drives, except removable and network;

    - in all folders specified in the %PATH% environment variable
      (not including subfolders).

    - in all folders from the drive root, in the find
      dialog one can change disk drive of the search. ;

    - from the current folder;

    - in the current folder only or in selected folders
      (the current version of Far does not search in
      directories that are ~symbolic links~@HardSymLink@).

    The search parameters is saved in the configuration.

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
    B - for bytes (no suffix also means bytes);
    K - for kilobytes;
    M - for megabytes;
    G - for gigabytes;
    T - for terabytes;
    P - for petabytes;
    E - for exabytes.

    #Column types# - Allows to specify search results output format.
Column types are encoded with one or more characters delimited with commas.
The following column types are supported:

    S[C,T,F,E] - file size
    P[C,T,F,E] - allocation file size
    G[C,T,F,E] - size of alternate file streams
                 where: C - format file size;
                        T - use 1000 instead of 1024 as the divisor;
                        F - show size as a decimal fraction
                            using the most appropriate unit,
                            e.g. 0.97 K, 1.44 M, 3.5 G;
                        E - compact mode, no space before
                            the file size suffix (e.g 0.97K).

    D          - file last write date
    T          - file last write time

    DM[B,M]    - file last write date and time
    DC[B,M]    - file creation date and time
    DA[B,M]    - file last access date and time
    DE[B,M]    - file change date and time
                 where: B - brief (Unix style) file time format;
                        M - use text month names.

    A          - file attributes
    Z          - file description

    O[L]       - file owner
                 where: L - show domain name.

    LN         - number of hard links

    F          - number of alternate streams


    File attributes are denoted as follows:

       #R#         - Read only
       #A#         - Archive
       #H#         - Hidden
       #S#         - System
       #C#         - Compressed
       #E#         - Encrypted
       #I#         - Not content indexed
       #D#         - Directory
       #$#         - Sparse
       #T#         - Temporary
       #O#         - Offline
       #L#         - Reparse point
       #V#         - Virtual
       #G#         - Integrity stream
       #N#         - No scrub data

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
    While ~search~@FindFile@ is in progress or when it is finished, you can use
the cursor keys to scroll the files list and the buttons to perform required
actions.

    During or after search the following buttons are available:

   #New search#      Start new search session.

   #Go to#           Breaks current search, changes panel folder
                   and moves cursor to the selected file.

   #View#            View selected file. If search is not completed,
                   it will continue in the background while the file
                   is viewed.

   #Panel#           Create a temporary panel and fill it with the
                   results of the last file search.

   #Stop#            Break current search. Available while search
                   is in progress.

   #Cancel#          Close the search dialog.

    During or after search you can view or edit found files.

   View                          #F3, Alt+F3, Numpad5, Ctrl+Shift+F3#

    #F3#, #Alt+F3# or #Numpad5# invokes ~internal~@Viewer@, external or ~associated ~@FileAssoc@ viewer,
depending on file type and ~viewer settings~@ViewerSettings@.
    #Ctrl+Shift+F3# always invokes internal viewer regardless of file associations.

   Edit                                    #F4, Alt+F4, Ctrl+Shift+F4#

    #F4# or #Alt+F4# invokes ~internal~@Editor@, external or ~associated~@FileAssoc@ editor,
depending on file type and ~editor settings~@EditorSettings@.
    #Ctrl+Shift+F4# always invokes internal editor regardless of file associations.

    Viewing and editing is supported for plugin emulated file systems. Note, that saving editor
changes by #F2# key in emulated file system will perform #SaveAs# operation,
instead of common #Save#.


@FindFolder
$ #Find folder#
    This command allows a quick look for the required folder in the folders
tree.

    To select a folder you can use the cursor keys or type first characters of
the folder.

    Press #Enter# to switch to the selected folder.

    #Ctrl+R# and #F2# force the rescan of the folders tree.

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

   #Filter name#     A title that will be shown in the filters menu.
                   This field can be empty.

                   Filter name is not available if the filter was
                   opened from ~Files highlighting and sort groups~@Highlight@.


   #Mask#            One or several ~file masks~@FileMasks@.

                   Filter conditions are met if file mask analysis
                   is on and its name matches any of the given file
                   masks. If mask analysis is off, file name is
                   ignored.


   #Size#            Minimum and maximum files size. The following
                   file size suffixes can be used:
                   B - for bytes (no suffix also means bytes);
                   K - for kilobytes;
                   M - for megabytes;
                   G - for gigabytes;
                   T - for terabytes;
                   P - for petabytes;
                   E - for exabytes.

                   Filter conditions are met if file size analysis
                   is on, and it is inside the given range.
                   If nothing is specified (empty line) for one
                   or both range boundaries then file size for that
                   boundary is not limited.

                   Example:
                   >= 1K - select files greater than or equal to 1 kilobyte
                   <= 1M - to less than or equal to 1 megabyte


   #Date/time#       Starting and ending file date/time.
                   You can specify the date/time of last file
                   #write#, file #creation#, last #access#
                   and #change#.

                   The button #Current# allows to fill the date/time
                   fields with the current date/time after which you
                   can change only the needed part of the date/time
                   fields, for example only the month or minutes.
                   The button #Blank# will clear the date/time
                   fields.

                   Filter conditions are met if file date/time
                   analysis is on and its date/time is in the
                   given range and corresponds to the given
                   type. If one or both of the date/time limit
                   fields are empty then the date/time for that
                   type is not limited.

                   Example:
                   <= 31.01.2010 - select files up to 31 numbers
                   >= 01.01.2010 - but after Jan. 1, 2010

                   Option #Relative# allows you to switch
                   to work with the date in relative time.
                   The logic at work this option is similar to
                   arithmetic with negative numbers.

                   Example:
                   <= 0 - select files in the period from the "Today"
                   >= 30 - and 30-days ago, including


   #Attributes#      Inclusion and exclusion attributes.

                   Filter conditions are met if file attributes
                   analysis is on and it has all of the inclusion
                   attributes and none of the exclusion attributes:
                   #[x]# - inclusion attribute - the file must have
                         this attribute.
                   #[ ]# - exclusion attribute - the file must
                         not have this attribute.
                   #[?]# - ignore this attribute.

                   The #Compressed#, #Encrypted#, #Not indexed#,
                   #Sparse#, #Temporary# and #Offline# attributes
                   are used only on disks with the NTFS file system.
                   #Virtual# attribute is not used in Windows
                   2000/XP/2003.
                   The #Integrity stream# and #No scrub data# attributes
                   only supported on ReFS voumes starting from
                   Windows Server 2012.

   #Has more than one hardlink#

    Used only on disks with NTFS file system. Condition evaluates to true,
if piece of data, which current file is pointing, is also pointed at least
by one another file.
    #Warning#: Enabling of this option can cause a significant slowdown.  


    To quickly disable one or several conditions, uncheck the corresponding
checkboxes. The #Reset# button will clear all of the filter conditions.


@History
$ #History #
    The commands history shows the list of previously executed commands.
Besides the cursor control keys, the following keyboard shortcuts are
available:

  Re-execute a command                                          #Enter#

  Re-execute a command in a new window                    #Shift+Enter#

  Re-execute a command as administrator                #Ctrl+Alt+Enter#

  Copy a command to the command line                       #Ctrl+Enter#

  Clear the commands history                                      #Del#

  Lock/unlock the current history item                            #Ins#

  Delete the current history item                           #Shift+Del#

  Copy the text of the current command to the clipboard        #Ctrl+C#
  without closing the list                                or #Ctrl+Ins#

    To go to the previous or next command directly from the command line, you
can press #Ctrl+E# or #Ctrl+X# respectively.

    If you want to save the commands history after exiting Far, use the
respective option in the ~system settings dialog~@SystemSettings@.

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

  Open a file in the ~viewer~@Viewer@                                        #F3#
                                                          or #Numpad5#

    Items of the view and edit history are moved to the end of the list after
they are selected. You can use #Shift+Enter# to select an item without changing
its position.

    If you want to save the view and edit history after exiting Far, use the
respective option in the ~system settings dialog~@SystemSettings@.

  Remarks:

  1. ^<wrap>List refresh operation (Ctrl+R) can take a considerable amount
of time if a file was located on a currently unavailable remote resource.

  2. ^<wrap>Locked items will not be deleted when clearing or refreshing the history.

    See also: common ~menu~@MenuCmd@ keyboard commands.

@HistoryFolders
$ #History: folders#
    The folders history shows the list of folders that have been recently
visited. Besides the cursor control keys, the following keyboard shortcuts are
available:

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
are selected. You can use #Shift+Enter# to select an item without changing its
position.

    If you want to save the folders history after exiting Far, use the
respective option in the ~system settings dialog~@SystemSettings@.

  Remarks:

  1. ^<wrap>List refresh operation (Ctrl+R) can take a considerable amount
of time if a folder was located on a currently unavailable remote resource.

  2. ^<wrap>Locked items will not be deleted when clearing or refreshing the history.

    See also: common ~menu~@MenuCmd@ keyboard commands.

@TaskList
$ #Task list#
    The task list displays active tasks. Every line contains PID and window
caption or path to executable.

    From the task list you can switch to the task window, or kill the task with
the #Del# key. Be careful when killing a task. It stops the task immediately,
and any unsaved information will be lost, so it should be used only when really
necessary, for example to interrupt a program which does not respond.

    The task list can be called either from ~Commands menu~@CmdMenu@ or using
#Ctrl+W#. The keyboard shortcut #Ctrl+W# can also be used in the viewer or the
editor.

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
$ #Compare folders#
    The compare folders command is applicable only when two ~file panels~@FilePanel@
are displayed. It compares the contents of folders displayed in the two panels.
Files existing in one panel only, or those which have a date more recent than
files with the same name in the other panel, become marked.

    Subfolders are not compared. Files are compared only by name, size and
time, and file contents have no effect on the operation.


@UserMenu
$ #User menu#
    The user menu exists to facilitate calls of frequently used operations. It
contains a number of user defined commands and command sequences, executed
when invoking the user menu. The menu can contain submenus.
~Special symbols~@MetaSymbols@ are supported both in the commands and in the command
titles. Note, that !?<title>?<init>! symbol can be used to enter additional
parameters directly before executing commands.

    With the #Edit user menu# command from the ~Commands menu~@CmdMenu@, you
can edit or create main or local user menu. There can only be one main user
menu. The main user menu is called if no local menu for the current folder is
available. The local menu can be placed in any folder. You can switch between
the main menu and the user menu by pressing #Shift+F2#. Also you can call the
user menu of the parent folder by pressing #BS#.

    You can add command separators to the user menu. To do this, you should add
a new menu command and define "#--#" as "hot key". To delete a menu separator,you
must switch to file mode with #Alt+F4# key.

    User menu items can be moved with #Ctrl+Up# or #Ctrl+Down# key combinations.

    To execute a user menu command, select it with cursor keys and press #Enter#.
You can also press the hot key assigned to the required menu item.

    You can delete a submenu or menu item with the #Del# key, insert new
submenu or menu item with the #Ins# key or edit an existing submenu or menu
item with the #F4# key. Press #Alt+F4# to edit the menu in text file form.

    It is possible to use digits, letters and function keys (#F1#..#F24#) as
hot keys in user menu. If #F1# or #F4# is used, its original function in user
menu is overridden. However, you still can use #Shift+F4# to edit the menu.

    When you edit or create a menu item, you should enter the hot key for fast
item access, the item title which will be displayed in the menu and the command
sequence to execute when this item will be selected.

    When you edit or create a submenu, you should enter the hot key and the
item title only.

    User menus are stored in text files named #FarMenu.Ini#:
    - Global user menu, by default, is located in the Far Manager folder.
      If global user menu file exists it will override the user specific menu.
    - User specific user menu is located in the user profile.
    - Local user menu is located in the current folder.

    To close the menu even if submenus are open use #Shift+F10#.

    See also:

    The list of ~macro keys~@KeyMacroUserMenuList@, available in the user menu.
    Common ~menu~@MenuCmd@ keyboard commands.

@FileAssoc
$ #File associations #
    Far Manager supports file associations, that allow to associate various
actions to running, viewing and editing files with a specified
~mask~@FileMasks@.

    You can add new associations with the #Edit associations# command in the
~Commands menu~@CmdMenu@.

    You can define several associations for one file type and select the
desired association from the menu.

    The following actions are available in the associations list:

    #Ins#        - ~add~@FileAssocModify@ a new association

    #F4#         - ~edit~@FileAssocModify@ the current association

    #Del#        - delete the current association

    #Ctrl+Up#    - move association up

    #Ctrl+Down#  - move association down

    If no execute command is associated with file and
#Use Windows registered types# option in ~System settings~@SystemSettings@
is on, Far tries to use Windows association to execute this file type;

    See also: common ~menu~@MenuCmd@ keyboard commands.

@FileAssocModify
$ #File associations: editing#
    Far allows to specify six commands associated with each file type specified
as a ~mask~@FileMasks@:

   #Execute command#               Performed if #Enter# is pressed
   #(used for Enter)#

   #Execute command#               Performed if #Ctrl+PgDn# is pressed
   #(used for Ctrl+PgDn)#

   #View command#                  Performed if #F3# is pressed
   #(used for F3)#

   #View command#                  Performed if #Alt+F3# is pressed
   #(used for Alt+F3)#

   #Edit command#                  Performed if #F4# is pressed
   #(used for F4)#

   #Edit command#                  Performed if #Alt+F4# is pressed
   #(used for Alt+F4)#

    The association can be described in the #Description of the association#
field.

    The following ~special symbols~@MetaSymbols@ can be used in the associated
command.

  Notes:

  1. ^<wrap>If no execute command is associated with file and
#Use Windows registered types# option in ~System settings~@SystemSettings@
is on, Far tries to use Windows association to execute this file type;

  2. ^<wrap>Operating system ~commands~@OSCommands@ "IF EXIST" and "IF DEFINED"
allow to configure "smarter" associations - if you have specified several
associations for a file type, the menu will show only the associations
for which the conditions are true.


@MetaSymbols
$ #Special symbols#
    The following special symbols can be used in ~associated commands~@FileAssoc@,
~user menu~@UserMenu@ and the command ~"Apply command"~@ApplyCmd@:

    #!!#       '!' character
    #!#        Long file name without extension
    #!~~#       Short file name without extension
    #!`#       Long extension without file name (ext)
    #!`~~#      Short extension without file name (ext)
    #!.!#      Long file name with extension
    #!-!#      Short file name with extension
    #!+!#      Similar to !-! but if a long file name was lost
             after performing the command, Far will restore it
    #!@@!#      Name of a file with a list of selected file names
    #!$!#      Name of a file with a list of selected short file names
    #!&#       List of selected files
    #!&~~#      List of selected short file names
    #!:#       Current drive in the format "C:"
             For remote folders - "\\\\server\\share"
    #!\\#       Current path
    #!/#       Short name of the current path
    #!=\\#      Current path considering ~symbolic links~@HardSymLink@.
    #!=/#      Short name of the current path considering
             ~symbolic links~@HardSymLink@.
    #!?!#      Description of the current file

    #!?<title>?<init>!#
             This symbol is replaced by user input, when
             executing command. <title> and <init> - title
             and initial text of edit control.

             Several such symbols are allowed in the same line,
             for example:

             grep !?Search for:?! !?In:?*.*!|c:\\far\\Far.exe -v -

             A history name for the <init> string can be supplied
             in the <title>. In such case the command has the
             following format:

             #!?$<history>$<title>?<init>!#

             for example:

             grep !?#$GrepHist$#Search for:?! !?In:?*.*!|Far.exe -v -

             Leave the name empty to disable history.

             In <title> and <init> the usage of other meta-symbols is
             allowed by enclosing them in brackets.

             (e.g. grep !?Find in (!.!):?! |Far.exe -v -)

    #!###       "!##" modifier specified before a file association
             symbol forces it (and all the following characters)
             to refer to the passive panel (see note 4).
             For example, !##!.! denotes the name of the current
             file on the passive panel.

    #!^#       "!^" modifier specified before a file association
             symbol forces it (and all the following characters)
             to refer to the active panel (see note 4).
             For example, !^!.! denotes a current file name on
             the active panel, !##!\\!^!.! - a file on the passive
             panel with the same name as the name of the current
             file on the active panel.

  Notes:

    1. ^<wrap>When handling special characters, Far substitutes only the string
corresponding to the special character. No additional characters (for example,
quotes) are added, and you should add them yourself if it is needed. For
example, if a program used in the associations requires a file name to be
enclosed in quotes, you should specify #program.exe "!.!"# and not
#program.exe !.!#.

    2. ^<wrap>The following modifiers can be used with the associations !@@! and !$! :

     'Q' - enclose names containing spaces in quotes;
     'S' - use '/' instead of '\\' in pathnames;
     'F' - use full pathnames;
     'A' - use ANSI code page;
     'U' - use UTF-8 code page;
     'W' - use UTF-16 (Little endian) code page.

    For example, the association #!@@AFQ!# means "name of file with the list of
selected file names, in ANSI encoding, include full pathnames, names with
spaces will be in quotes".

    3. ^<wrap>When there are multiple associations specified, the meta-characters !@@!
and !$! are shown in the menu as is. Those characters are translated when the
command is executed.

    4. ^<wrap>The prefixes "!##" and "!^" work as toggles for associations. The effect
of these prefixes continues up to the next similar prefix. For example:

    if exist !##!\\!^!.! diff -c -p !##!\\!^!.! !\\!.!

  "If the same file exists on the passive panel as the file under
   the cursor on the active panel, show the differences between
   the file on the passive panel and the file on the active panel,
   regardless of the name of the current file on the passive panel"

    5. ^<wrap>If it is needed to pass to a program a name with an ending
backslash, use quotes, e. g. #"!"\#.
    For example, to extract a rar archive to a folder with the same name:

    winrar x "!.!" "!"\

@SystemSettings
$ #Settings dialog: system#
  #Delete to Recycle Bin#
  Enables file deletion via the Recycle Bin.The operation of deleting to the Recycle
Bin can be performed only for local hard disks.

  #Use system copy routine#
  Use the file copy functions provided by the operating system instead of internal
file copy implementation. It can be useful on NTFS, because the system function
(CopyFileEx) copies file extended attributes. On the other hand, when using the system
function, the possibility to split files when ~copying~@CopyFiles@ or moving is not available.

  #Copy files opened for writing#
  Allows to copy files that are opened by other programs for writing. This mode
is handy to copy a file opened for a long time, but it could be dangerous, if a file
is being modified at the same time as copying.

  #Scan symbolic links#
  Scan ~symbolic links~@HardSymLink@ along with normal sub-folders when building the folder tree,
determining the total file size in the sub-folders.

  #Update panels only when Far is active#
  If enabled, file panels will be monitored only when Far is active, i.e. panels will not be updated until Far window is focused.
This allows to avoid blocking the directories opened on panels. However, sometimes the update is not triggered after receiving focus,
so this option is disabled by default and directories are always monitored.

  #Save commands history#
  Forces saving ~commands history~@History@ before exit and restoring after starting Far.

  #Save folders history#
  Forces saving ~folders history~@HistoryFolders@ before exit and restoring after starting Far.
Folders history list can be activated by #Alt+F12#.

  #Save view and edit history#
  Forces saving ~history of viewed and edited~@HistoryViews@ files before exit and restoring it after
starting Far. View and edit history list can be activated by #Alt+F11#.

  #Use Windows registered types#
  When this option is on and #Enter# is pressed on a file, the type of which is known to
Windows and absent in the list of Far ~file associations~@FileAssoc@, the Windows program
registered to process this file type will be executed.

  #CD drive auto mount#
  When a CD-ROM drive is selected from the ~Change drive menu~@DriveDlg@, Far will close the open
tray of a CD drive. Turn off this option if automatic CD-ROM mounting does not work
correctly (this can happen because of bugs in the drivers of some CD-ROM drives).

  #Automatic update of environment variables#
  Automatically update the environment variables if they have been changed globally.

  #Request administrator rights#
  The current user might not always has the required rights to work with certain file system objects.
Far allows to retry the operation using the privileged account.

  Available options:

    #for modification#
    ^<wrap>allow requesting rights for operations that change the state of the file system (e.g. file or directory creation/modification/deletion)

    #for read#
    ^<wrap>allow requesting rights for operations that do not change the state of the file system (e.g. reading files or listing directories).

    #use additional privileges#
    ^<wrap>Try to ignore Access Control List if possible.

  #Sorting collation#
  ^<wrap>Allows to choose and configure the sorting collation.

    #ordinal# - based on the ordinal value of the characters in the string
    #invariant# - invariant collation
    #linguistic# - based on the culture-specific sorting conventions

    #Treat digits as numbers#
    ^<wrap>When enabled, sequential groups of digits are treated as numbers. The following example shows how the files are sorted:

    Disabled                    Enabled

    Ie4_01                      Ie4_01
    Ie4_128                     Ie4_128
    Ie401sp2                    Ie5
    Ie5                         Ie6
    Ie501sp2                    Ie401sp2
    Ie6                         Ie501sp2
    11.txt                      5.txt
    5.txt                       11.txt
    88.txt                      88.txt

    ^<wrap>Note: treating digits as numbers in linguistic collation is possible in Windows 7 and above. In older systems invariant collation will be used automatically.

    #Case sensitive#
    ^<wrap>Take into account the case of the characters in the string.

    ^<wrap>Note: how exactly the case of the characters in the string will be taken into account depends on the sorting collation.

  #Auto save setup#
  If checked, Far will save setup automatically. The current folders for both panels will be also saved.


@PanelSettings
$ #Settings dialog: panel#
  #Show hidden and#         Enables displaying files with Hidden
  #system files#            or System attributes. This option can
                          also be switched by #Ctrl+H#.

  #Highlight files#         Enables ~files highlighting~@Highlight@.

  #Select folders#          Enables folder selection with #Gray +#
                          and #Gray *#. If this option is off, these
                          keys select only files.

  #Right click#             If this option is on, #right mouse click#
  #selects files#           selects files. Otherwise, it opens Windows
                          Explorer Context menu.

  #Sort folder names#       Applies sort by extension not only
  #by extension#            to files, but also to folders.
                          When this option is turned on, sorting
                          by extension works the same as it did
                          in Far 1.65. If the option is turned
                          off, in the extension sort mode the
                          folders will be sorted by name, as
                          they are in the name sort mode.

  #Allow reverse#           If this option is on and the current file
  #sort modes#              panel sort mode is reselected, reverse
                          sort mode will be set.

  #Disable automatic#       If this option is on and the number of file
  #panel update#            objects exceeds the specified value,
                          the automatic panel update when the file
                          system state changes is disabled.

    Auto-update works only for FAT/FAT32/NTFS file systems.
    The value of 0 means "update always".
    To force an update of the panels, press #Ctrl+R#.

  #Network drives#          This option enables panel autorefresh
  #autorefresh#             when the file system state of a network
                          drive is changed. It may be helpful
                          to disable this option on slow network
                          connections.

  #Show column titles#      Enables displaying ~file panel~@FilePanel@ column titles.

  #Show status line#        Enables displaying file panel status line.

  #Detect volume#           Distiguishes between normal directory links
  #mount points#            (Junctions) and volume mount points.
                          This option significanty slows down displaying
                          directories on slow network connections.

  #Show files total#        Enables displaying the file totals
  #information#             at the bottom line of the file panel.

  #Show free size#          Enables displaying the current disk free space.

  #Show scrollbar#          Enables displaying file and ~tree panel~@TreePanel@ scrollbars.

  #Show background#         Enables displaying the number of ~background screens~@ScrSwitch@.
  #screens number#

  #Show sort mode#          Displays current sort mode in the upper
  #letter#                  left-hand corner of the panel.

  #Show ".." in#            Enables displaying the ".." item on root folder
  #root folders#            lists. Pressing #Enter# on this item opens
                          ~Change drive menu~@DriveDlg@.

@TreeSettings
$ #Tree settings#
  #Auto change folder#      If turned on, moving cursor on the ~tree panel~@TreePanel@
                          will synchronously change the folder on the other panel.
                          If turned off, to change the folder from the tree panel,
                          you need to press #Enter#.

  #Minimum number#          The minimal number of folders on the disk for which
  #of folders#              folder tree cache file #tree3.far# will be created.

@InterfSettings
$ #Settings dialog: interface#
  #Clock in panels#
  Show clock at the right top corner of the screen.

  #Clock in viewer and editor#
  Show clock in viewer and editor.

  #Mouse#
  Use the mouse.

  #Show key bar#
  Show the functional key bar at the bottom line.
This option can also be switched by #Ctrl+B#.

  #Always show menu bar#
  Show top screen menu bar even when it is inactive.

  #Screen saver#
  Run screen saver after the inactivity interval in minutes. When this option
is enabled, screen saver will also activate when mouse pointer is brought
to the upper right corner of Far window.

  #Show total copy indicator#
  Show total progress bar, when performing a file copy operation.
This could require some additional time before starting copying
to calculate the total files size.

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

  #ClearType friendly redraw#
  Redraw the window in such a way that ClearType related artifacts do not appear.
  #Attention!#: Enabling this option can considerably slow down the redraw speed.

  #Set console icon#
  If this option is activated Far will actively set the icon of the console window
to its inbuilt icon. If the option is not activated it's possible to set the icon of
the console window to any desired icon via the properties of the Far launching link
in Windows.

  #Alternate for Administrator#
  If this option is activated and Far is executed by a user with administrator privileges,
then the used icon will be red instead of blue. This option is only relevant if the
option #Set console icon# is activated.
  
  #Far window title addons#
  Additional information, displayed in the window title.
Can contain any text, including environment variables (e.g. "%USERDOMAIN%\%USERNAME%") and the following special variables:

  - #%Ver# - Far version;
  - #%Build# - Far build number;
  - #%Platform# - Far platform;
  - #%Admin# - ^<wrap>"Administrator" if running as administrator, otherwise an empty string.
  - #%PID# - Far process ID;


@DialogSettings
$ #Settings dialog: dialogs#
  #History in dialog#       Keep history in edit controls of some
  #edit controls#           Far dialogs. The previous strings history
                          list can be activated with the mouse or using
                          #Ctrl+Up# and #Ctrl+Down#. If you do not wish
                          to track such history, for example due to
                          security reasons, switch this option off.

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
  If a block is selected, pressing Del will not remove the character under cursor,
but this block.

  #AutoComplete#
  Allows to use the AutoComplete function in command line. When the option is
disabled, you can use the #Ctrl+Space# key to autocomplete a line. The autocomplete
feature is disabled while a macro is being recorded or executed.

  #Set command line prompt format#
  This option allows to set the default Far command ~line prompt~@CommandPrompt@.

@AutoCompleteSettings
$ #Settings dialog: AutoComplete#
  #Show a list#
  Show completion suggestions as a list.

  #Modal mode#
  Make the list modal.

  #Append the first matched item#
  Append the first matched item to the command line text as you type.

  There are several additional options to control what data sources will be used to populate the completion list:
    #Interface.Completion.UseFilesystem#
    #Interface.Completion.UseHistory#
    #Interface.Completion.UsePath#
    #Interface.Completion.UseEnvironment#
  All parameters are 3-state - yes / no / only if called manually (via #Ctrl+Space#).
These parameters can be changed via ~far:config~@FarConfig@


@CommandPrompt
$ #Command line prompt format#
   Far allows to change the command line prompt format.
To change it you have to enter the needed sequence of variables and
special code words in the #Set command line prompt format# input field
of the ~Command line settings~@CmdlineSettings@ dialog, this will allow showing
additional information in the command prompt.

   It is allowed to use environment variables and the following special
code words:

     $a - the & character
     $b - the | character
     $c - the ( character
     $d - current date (depends on system settings)
     $f - the ) character
     $g - the > character
     $h - delete the previous character
     $l - the < character
     $n - drive letter of the current drive
     $p - current drive and path
     $m - ^<wrap>full network path of the current drive or empty, if the current drive is not a network drive
     $w - ^<wrap>current working directory (without the path)
     $q - the = character
     $s - space
     $t - current time in HH:MM:SS format
     $$ - the $ character
     $+ - the depth of the folders stack
     $##nn - max promt width, given in percents relative to the width of the window

     $@@xx - ^<wrap>"Administrator", if Far Manager is running as administrator.
xx is a placeholder for two symbols that will surround the "Administrator" word.
For example, #$@@{}$s$p$g# will be shown as "{Administrator} C:\>"

   By default the #$p$g# sequence is used - current drive and path ("C:\>").

   Examples.

   1. ^<wrap>A prompt of the following format #[%COMPUTERNAME%]$S$P$G#
will contain the computer name, current drive and path
(the %COMPUTERNAME% environment variable must be defined)

   2. ^<wrap>A prompt of the following format #[$T$H$H$H]$S$P$G# will
display the current time in HH:MM format before the current
drive and path

   3. ^<wrap>Code "$+" displays the number of pluses (+) needed according to
current ~PUSHD~@OSCommands@ directory stack depth, one character per each
saved path.

   Prompt elements can be highlighted with #colours#.
   Format:
   #([[T]FFFFFFFF][:[T]BBBBBBBB])#, where:

   #FFFFFFFF#
   Foreground colour in aarrggbb format or index in the console palette.

   #BBBBBBBB#
   Background colour in aarrggbb format or index in the console palette.

   #T#
   "TrueColour" flag. If absent, value is treated as the console palette index (0-F):

   \00 \11 \22 \33 \44 \55 \66 \77 \88 \99 \AA \BB \CC \DD \EE \FF \-
   0123456789ABCDEF

   If foreground or background colour is omitted, the corresponding default value will be used.

   Examples:

   #(E)#               \0eBright yellow text on default background\-
   #(:F)#              \f7Default text on white background\-
   #(B:C)#             \cbBright blue text on bright red background\-
   #()#                \07Default text on default background\-
   #(T00CCCC:TE34234)# \(T00CCCC:TE34234)Robin egg blue text on Vermilion background\-

   The specified colour will be active till the end of the prompt or the next colour entry.

   Example:

   #(a)%username%(f)@@(b)%computername%() $p$g# \0aadmin\0f@@\0bserver\07 C:\\>\-

@Viewer
$ #Viewer: control keys#
  Navigation keys

  The behavior of navigation keys depends on the ~view mode~@ViewerMode@.

  The following keys work in all modes:

    #Up#                 ^<wrap>Line up
    #Down#               Line down
    #PgUp#               Screenful up
    #PgDn#               Screenful down
    #Home, Ctrl+Home#    Beginning of the file
    #End, Ctrl+End#      End of the file

  The following additional keys work in #text mode without line wrap#:

    #Left#               ^<wrap>One column left
    #Right#              One column right
    #Ctrl+Left#          20 columns left
    #Ctrl+Right#         20 columns right
    #Ctrl+Shift+Left#    Show the leftmost column
    #Ctrl+Shift+Right#   Show the rightmost column of all lines
currently visible on the screen

  In #hex# and #dump# ~view modes~@ViewerMode@, #Ctrl+Left# and
#Ctrl+Right# keys shift the content within the window one byte at a time
in the corresponding direction.

  Viewer commands

    #F1#                 ^<wrap>Help
    #F2#                 Toggle line wrap/unwrap in #text# mode, or change ~view mode~@ViewerMode@
    #Shift+F2#           Toggle wrap type (characters or words) in #text# ~view mode~@ViewerMode@
    #F4#                 Toggle ~view mode~@ViewerMode@ to #hex# and back
    #Shift+F4#           Select ~view mode~@ViewerMode@: #text#, #hex#, or #dump#
    #F6#                 Switch to ~editor~@Editor@
    #F7#                 ~Search~@ViewerSearch@
    #Shift+F7, Space#    Continue search
    #Alt+F7#             Continue search in reverse direction
    #F8#                 Switch between OEM and ANSI code pages
    #Shift+F8#           Select code page using the ~Code pages~@CodePagesMenu@ menu
    #Alt+F8#             ~Change current position~@ViewerGotoPos@
    #Alt+F9#             Maximize or restore the size of the Far console window;
see also ~Interface.AltF9~@Interface.AltF9@
    #Alt+Shift+F9#       Open ~Viewer settings~@ViewerSettings@ dialog
    #Numpad5,F3,F10,Esc# Quit viewer
    #Ctrl+F10#           Jump to the current file on the active file panel
    #F11#                Open ~Plugin commands~@Plugins@ menu
    #Alt+F11#            Display view and edit ~history~@HistoryViews@
    #Gray +#             View the next file on the active file panel
    #Gray -#             View the previous file on the active file panel
    #Ctrl+O#             Show user screen
    #Ctrl+Alt+Shift#     Temporarily show user screen
(while the key combination is held down)
    #Ctrl+B#             Toggle functional key bar at the bottom of the screen
    #Ctrl+Shift+B#       Toggle status line
    #Ctrl+S#             Toggle the scrollbar
    #Alt+BS, Ctrl+Z#     Undo position change
    #RightCtrl+0..9#     Set bookmark 0..9 at the current position
    #Ctrl+Shift+0..9#    Set bookmark 0..9 at the current position
    #LeftCtrl+0..9#      Go to bookmark 0..9
    #Ctrl+Ins, Ctrl+C#   Copy the selected text to the clipboard.
The text can be selected manually or as the result of a ~search~@ViewerSearch@.
    #Ctrl+U#             Unselect the text
    #Shift+Mouse click#  Select text manually
                       The first mouse click indicates the
beginning of the selected area; the second click indicates the end.
Use navigation keys after the first click to bring the end position into
the view. The end of the selected area may be set before or after the
beginning in the text.

  See also the list of ~macro keys~@KeyMacroViewerList@ available in the viewer.

  Notes:

    1. ^<wrap>Start typing the search pattern to open the
~search~@ViewerSearch@ dialog.

    2. ^<wrap>The viewer opens files with the permission to be deleted.
If another process attempts to delete the file while it is open in the
viewer, the file will be deleted after the viewer is closed. Any
operation on a file while its deletion is pending will fail. This is
a feature of the Windows operating system.

    3. ^<wrap>The maximum number of columns displayed in the #text#
~view mode~@ViewerMode@ can be configured in the
~settings~@ViewerSettings@ dialog. The range is between 100 to 100,000,
the default is 10,000. Lines longer than the maximum will be split into
several lines even if word wrap mode is turned off.

    4. ^<wrap>Far starts ~searching~@ViewerSearch@ (#F7#) from the
beginning of the currently visible area.

    5. ^<wrap>To auto-scroll a file which is being appended by another
process (conf. Linux “tail”), go to the end of the file (press the #End#
key).

@ViewerMode
$ #Viewer: view modes#
    В программе просмотра предусмотрено три режима отображения содержимого просматриваемого файла: 
#Текст#, #Код# и #Дамп#. При открытии файла, если включено сохранение режима просмотра
и открываемый файл в истории есть - выбирается последний использованный режим.
В противном случае, если включена ~опция~@ViewerSettings@ #Автовыбор дамп режима просмотра#
и Far считает файл бинарным - выбирается режим #дамп#, иначе - #текст#.

    Режим просмотра можно изменить вручную клавишами:

    #F4# - переключение между режимом #Код# и последним из использованных #Текст# или #Дамп#
    #Shift+F4# - будет отображено меню для выбора одного из трех режимов
    #F2# - для режима #Текст# меняет режим переноса, режим #Дамп# переключается в #Текст#,
режим #Код# переключается в #Дамп# или #Текст# (#F4# и #F2# переключают в разные режимы). 

    Текущий режим отображается одной буквой в первой строке окна, перед значением
текущей кодовой страницы: #t# - Текст, #h# - Код, #d# - Дамп. 

    Режим #Текст#

    В режиме #Текст# Far отображает содержимое файла, интерпретируя последовательность байтов
как строки символов в текущей кодировке (кодировки не только однобайтовые). Непечатаемые символы
или символы, для которых нет отображения в текущем шрифте/кодировке, будут отображены как знаки
вопроса, либо как знак "пустой прямоугольник", либо как знак "маленький вопросительный знак в
прямоугольнике", либо не будут отображены - это зависит от выбора шрифта окна Far. Строки
переводятся после нахождения признака окончания текстовой строки.
    Для просмотра длинных строк, выходящих за границу окна, можно клавишей #F2#
включить/выключить режим переноса.
    Если перенос разрешен, строки принудительно переводятся (сворачиваются) при достижении правой
границы экрана. Можно разрешить/запретить перенос внутри слова #Shift+F2#.
    Если перенос отключен и строка не помещается на экране, то отображается только часть строки
и у правой границы окна просмотра другим цветом рисуется знак #»# (если включена настройка
#Показывать стрелки сдвига#). Клавишами #Right#/#Ctrl+Right# можно сдвигать начало отображаемой
части строк вправо на 1/20 символов (#Left#/#Ctrl+Left# влево). Если строки отображаются не с
первого символа, у левой границы окна просмотра рисуется знак #«# (настраивается).
Максимальная длина строки просмотра ограничена: #Максимальная ширина строки# в диалоге
~настроек~@ViewerSettings@. Более длинные строки разбиваются на несколько, даже если они
не содержат символов перевода строки.

    Режим #Дамп#

    В режиме #Дамп# Far отображает содержимое файла с текущей позиции, интерпретируя каждый байт
(для кодовых страниц UTF-16 - каждые два байта, UTF-8 символы переменной длины) как отображаемый
символ в текущей кодировке. При достижении края окна следующий символ отображается с начала новой
строки. Переводы строк и управляющие коды отображаются как обычные печатные символы.
    В режиме #Дамп# нет понятия строки, клавиши горизонтального перемещения работают не так
как в режиме #Текст# (а почти как в режиме #Код#). Клавиши #Left#/#Right# игнорируются,
а клавиши #Ctrl+Left#/#Ctrl+Right# начало отображаемой в окне части файла на 1 символ (не 1 байт!).
Таким образом, отображаемые символы как бы "перетекают" через края окна отображения.

    Режим #Код# (Hex / 16-ричные коды)

    В режиме #Код# Far отображает содержимое файла аналогично шестнадцатеричным редакторам:
в строке указывается шестнадцатеричное смещение каждых 16 байт от начала файла, их 16-ричные
коды и отображение в виде символов. Для кодировок UTF-16: 8 16-ричных слов и 8 символов.
Для кодировки UTF-8 многобайтные символы дополняются знаками '»'.
#Ctrl+Left#/#Ctrl+Right# (в отличии от режима #Дамп#) сдвигают начало отображаемой части файла
всегда на 1 байт, многобайтные символы могут 'разрезаться' на части.


@ViewerGotoPos
$ #Viewer: go to specified position#
    This dialog allows to change the position in the internal viewer.

    ^<wrap>You can enter an absolute or relative value or percentage, in decimal or hexadecimal.
    ^<wrap>For relative add #+# or #-# before the value.
    ^<wrap>For percentage add #%# after the value.
    ^<wrap>For decimal either add #m# after the value or uncheck the #Hex value#.
    ^<wrap>For hexadecimal either add #0x# or #$# before the value, #h# after the value, or check the #Hex value#.

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
      The following options are available in that mode:

        #Case sensitive#      - ^<wrap>the case of the characters entered will be taken into account
while searching (so, for example, #Text# will not be found when searching for #text#).

        #Whole words#         - ^<wrap>the given text will be found only if it occurs in the text as a whole word.

        #Regular expressions# - ^<wrap>enable the use of ~regular expressions~@RegExp@ in the search string.
The multiline search is not supported.

    #Search for hex#

      ^<wrap>Search for a string corresponding to hexadecimal codes entered in the #Search for# string.

    #Reverse search#

      ^<wrap>Reverse the search direction - search from the end of file towards the beginning.


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
then a "~Path to the file to edit does not exist~@WarnEditorPath@" warning
will be shown.

    2. ^<wrap>When trying to reload a file already opened in the editor the
"~reloading a file~@EditorReload@" warning message will be shown.

    3. ^<wrap>The WIN encoding is used by default when creating new files, this
behavior can be changed in the ~Editor settings~@EditorSettings@ dialog.

  #Control keys#

  Cursor movement

   #Left#                    Character left
   #Ctrl+S#                  ^<wrap>Move the cursor one character to the left, but don't move to the previous line if the line beginning is reached.
   #Right#                   Character right
   #Up#                      Line up
   #Down#                    Line down
   #Ctrl+Left#               Word left
   #Ctrl+Right#              Word right
   #Ctrl+Up#                 Scroll screen up
   #Ctrl+Down#               Scroll screen down
   #PgUp#                    Page up
   #PgDn#                    Page down
   #Home#                    Start of line
   #End#                     End of line
   #Ctrl+Home, Ctrl+PgUp#    Start of file
   #Ctrl+End, Ctrl+PgDn#     End of file
   #Ctrl+N#                  Start of screen
   #Ctrl+E#                  End of screen

  Delete operations

   #Del#                     ^<wrap>Delete char (also can delete block, depending upon ~Editor settings~@EditorSettings@).
   #BS#                      Delete char left
   #Ctrl+Y#                  Delete line
   #Ctrl+K#                  Delete to end of line
   #Ctrl+BS#                 Delete word left
   #Ctrl+T, Ctrl+Del#        Delete word right

  Block operations

   #Shift+Cursor keys#       Select block
   #Ctrl+Shift+Cursor keys#  Select block
   #Alt+gray cursor keys#    Select vertical block
   #Alt+Shift+Cursor keys#   Select vertical block
   #Ctrl+Alt+gray keys#      Select vertical block
   #Ctrl+A#                  Select all text
   #Ctrl+U#                  Deselect block
   #Shift+Ins, Ctrl+V#       Paste block from clipboard
   #Shift+Del, Ctrl+X#       Cut block
   #Ctrl+Ins, Ctrl+C#        Copy block to clipboard
   #Ctrl+<Gray +>#           Append block to clipboard
   #Ctrl+D#                  Delete block
   #Ctrl+P#                  ^<wrap>Copy block to current cursor position (in persistent blocks mode only)
   #Ctrl+M#                  ^<wrap>Move block to current cursor position (in persistent blocks mode only)
   #Alt+U#                   Shift block left
   #Alt+I#                   Shift block right

  Other operations

   #F1#                      Help
   #F2#                      Save file
   #Shift+F2#                ~Save file as...~@FileSaveAs@
   #Shift+F4#                Edit ~new file~@FileOpenCreate@
   #F6#                      Switch to ~viewer~@Viewer@
   #F7#                      ~Search~@EditorSearch@
   #Ctrl+F7#                 ~Replace~@EditorSearch@
   #Shift+F7#                Continue search/replace
   #Alt+F7#                  Continue search/replace in "reverse" mode
   #F8#                      Toggle OEM/ANSI code page
   #Shift+F8#                Select code page
   #Alt+F8#                  ~Go to~@EditorGotoPos@ specified line and column
   #Alt+F9#                  ^<wrap>Maximize or restore the size of the Far console window; see also ~Interface.AltF9~@Interface.AltF9@
   #Alt+Shift+F9#            Call ~Editor settings~@EditorSettings@ dialog
   #F10, F4, Esc#            Quit
   #Shift+F10#               Save and quit
   #Ctrl+F10#                Position to the current file
   #F11#                     Call "~Plugin commands~@Plugins@" menu
   #Alt+F11#                 Display ~edit history~@HistoryViews@
   #Alt+BS, Ctrl+Z#          Undo
   #Ctrl+Shift+Z#            Redo
   #Ctrl+L#                  Disable edited text modification
   #Ctrl+O#                  Show user screen
   #Ctrl+Alt+Shift#          ^<wrap>Temporarily show user screen (as long as these keys are held down)
   #Ctrl+Q#                  ^<wrap>Treat the next key combination as a character code
   #RightCtrl+0..9#          ^<wrap>Set a bookmark 0..9 at the current position
   #Ctrl+Shift+0..9#         ^<wrap>Set a bookmark 0..9 at the current position
   #LeftCtrl+0..9#           Go to bookmark 0..9
   #Shift+Enter#             ^<wrap>Insert the name of the current file on the active panel at the cursor position.
   #Ctrl+Shift+Enter#        ^<wrap>Insert the name of the current file on the passive panel at the cursor position.
   #Ctrl+F#                  ^<wrap>Insert the full name of the file being edited at the cursor position.
   #Ctrl+B#                  ^<wrap>Show/Hide functional key bar at the bottom line.
   #Ctrl+Shift+B#            Show/Hide status line

   See also the list of ~macro keys~@KeyMacroEditList@, available in the editor.

    Notes:

    1. #Alt+U#/#Alt+I# indent the current line if no block is selected.

    2. ^<wrap>Holding down #Alt# and typing a character code on the numeric
keypad inserts the character that has the specified code (0-65535).

    3. ^<wrap>If no block is selected, #Ctrl+Ins#/#Ctrl+C# marks the current
line as a block and copies it to the clipboard.


@EditorSearch
$ #Editor: search/replace#
    The following options are available for search and replace in the ~editor~@Editor@:

      #Case sensitive#      - ^<wrap>the case of the characters entered will be taken into account while searching (so, for example,
#Text# will not be found when searching for #text#).

      #Whole words#         - ^<wrap>the given text will be found only if it occurs in the text as a whole word.

      #Reverse search#      - ^<wrap>change the direction of search (from the end of file towards the beginning)

      #Regular expressions# - ^<wrap>treat input as Perl regular expression (~search~@RegExp@ and ~replace~@RegExpRepl@).
Each line is processed individually, so multi-line expressions and line-break characters will not be found.

      ~Preserve style~@PreserveStyle@      - ^<wrap>preserve style (case and delimiters in program source code) of the replaced text.

    The #All# button will show All matching entries ~menu~@FindAllMenu@.


@PreserveStyle
$ #Editor: Replace mode - Preserve style#
    The #“Preserve style”# ~replace~@EditorSearch@ mode in the
~Editor~@Editor@ preserves the style (case, delimiters) of the replaced
text. This mode may be useful when editing program source code. Some
examples are below. Note how the style of the replaced strings
is preserved in each case.

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

    #More formally.#

    The main operation used in the algorithm is parsing a string into
tokens. The tokens are divided at a single separator character
or between a lowercase and an uppercase letter. Token separator
characters are #underscore “_”#, #hyphen “-”#, and #dot “.”#. All tokens
must be divided with the same separator. If the parse is ambiguous, the
entire string is treated as a single token. For example:

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

    If the found string and the replace pattern have different number
of tokens, the first token is processed separately from the rest of the
tokens. The first replace token inherits the style of the first found
token. The rest of the replace tokens are transformed to the common
style of the rest of the found tokens. If the common style cannot
be defined, the rest of the replace tokens are not changed. As in the
previous case, the replace tokens are joined with the separator of the
parse of the found string and the result is used as the replace string.
Examples:

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

    In the special case when the found string consists of a single token
but the replace string has several tokens, the first replace token
inherits the style of the found token. The common style for the rest
of the replace tokens and the separator type are deduced from the
context of the found string. If this is not possible, the common style
is the style of the (single) found token and the separator is empty.
Again, the transformed replace tokens are joined and used as the replace
string. More examples:

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

    If the search pattern is not found according to the rules above but
found as an ordinary string, and both the found string and the replace
string start with letters, the case of the first letter of the replace
string is changed to that of the found string. For example:

    ┌────────────────┬────────────────────┬──────────────────────┐
    │ Find / Replace │ Before             │ After                │
    ├────────────────┼────────────────────┼──────────────────────┤
    │ ab.cd / wx-yz  │ #A#b.cD              │ #W#x-yz                │
    └────────────────┴────────────────────┴──────────────────────┘

@FindAllMenu
$ #Editor: All matching entries menu#
    The following key combinations are available in this menu:

        #F5#              - Toggle menu size.

        #Ctrl+Up#         - Scroll the text in the editor.
        #Ctrl+Down#

        #Ctrl+Enter#      - Go to the position of the found text.
        #Ctrl+Left#
          #mouse click#

        #Gray +#          - ^<wrap>Add session bookmark with the current position.

        #RightCtrl+0..9#  - Set bookmark 0..9 at the current position.
        #Ctrl+Shift+0..9#

        #LeftCtrl+0..9#   - Go to the bookmark 0..9.

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

    #Default#   - ^<wrap>If the file has already been opened and its
code page was saved (depends on the #Save file position# option of the
~Editor settings~@EditorSettings@ dialog), the saved code page is used.
Otherwise, if the file has the Byte Order Mark, the corresponding
Unicode code page -- UTF-8, UTF-16 (Little endian), or UTF-16 (Big
endian) -- is used. Otherwise, the code page is ~autodetected~@CodePageAuto@.

    #Automatic# - An attempt is made to ~autodetect~@CodePageAuto@
    #detection#   code page based on the file contents.

    #Specific#  - The selected code page is used.
    #code page#

@FileSaveAs
$ #Editor: save file as...#
    To save edited file with another name press #Shift+F2# and specify
new name, code page and carriage return symbols format.

    If file has been edited in one of the following code pages: UTF-8,
UTF-16 (Little endian) or UTF-16 (Big endian), then if the option #Add signature (BOM)# is on,
the appropriate marker is inserted into the beginning of the file, which
helps applications to identify the code page of this file.

    You can also specify the format of the line break characters:

    #Do not change#
    Do not change the line break characters.

    #Dos/Windows format (CR LF)#
    Line breaks will be represented as a two-character sequence -
    Carriage Return and Line Feed (CR LF), as used in DOS/Windows.

    #Unix format (LF)#
    Line breaks will be represented as a single character - Line
    Feed (LF), as used in UNIX.

    #Mac format (CR)#
    Line breaks will be represented as a single character - Carriage
    Return (CR), as used in Mac OS.


@EditorGotoPos
$ #Editor: go to specified line and character#
    This dialog allows to change the position in the internal editor.

    ^<wrap>You can enter an absolute or relative value or percentage, in decimal or hexadecimal.
    ^<wrap>For relative add #+# or #-# before the value.
    ^<wrap>For percentage add #%# after the value.
    ^<wrap>For decimal either add #m# after the value or uncheck the #Hex value#.
    ^<wrap>For hexadecimal either add #0x# or #$# before the value, #h# after the value, or check the #Hex value#.

    The first value will be interpreted as a row number, the second as a character number.
Values must be delimited by space or one of the following characters: #,.;:#.
If a value is omitted the corresponding parameter will not be changed.


@EditorReload
$ #Editor: reloading a file#
    Far Manager tracks all attempts to repeatedly open for editing a file that
is already being edited. The rules for reloading files are as follows:

    1. If the file was not changed and the option "Reload edited file" in the
~confirmations~@ConfirmDlg@ dialog is not enabled, Far switches to the open
editor instance without further prompts.

    2. If the file was changed or the option "Reload edited file" is enabled,
there are three possible options:

    #Current#      - Continue editing the same file

    #New instance# - The file will be opened for editing in a new
                   editor instance. In this case, be attentive: the
                   contents of the file on the disk will correspond
                   to the contents of the editor instance where the
                   file was last saved.

    #Reload#       - The current changes are not saved and the
                   contents of the file on the disk is reloaded into
                   the editor.


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

        #Automatic# - Far will try to ~autodetect~@CodePageAuto@
        #detection#   the code page of the text;

        #System#    - ^<wrap>main single-byte system code pages - ANSI
and OEM;

        #Unicode#   - ^<wrap>Unicode code pages;

        #Favorites# - ^<wrap>code pages selected by the user;

        #Other#     - ^<wrap>the rest of code pages installed
in the system.

    The following key combinations are available in this menu:

        #Ctrl+H# - ^<wrap>Shows or hides the #Other# menu section.

        #Ins#    - ^<wrap>Moves the code page from the #Other#
section to the #Favorites# section.

        #Del#    - ^<wrap>Moves the code page from the #Favorites#
section back to the #Other# section.

        #F4#     - ^<wrap>Opens the
~Rename the code page~@EditCodePageNameDlg@ dialog. Only #Favorites# and
#Other# code pages can be renamed. The renamed code pages are indicated
with the #*# symbol.

    See also common ~menu keyboard commands~@MenuCmd@.

@EditCodePageNameDlg
$ #Rename code page#
    This dialog allows to rename the #Favorites# and #Other# code pages.
Far will display new code page names in the ~Code pages~@CodePagesMenu@ menu.

    The #Reset# button sets the code page name to the default system
name. Another way to reset the name is to leave it empty and press #OK#.

@DriveDlg
$ #Change drive#
    This menu allows to change the current drive of a panel or open a new ~plugin~@Plugins@ panel.

    Select the item with the corresponding drive letter to change the drive or
the item with the plugin name to create a new plugin panel. If the panel type
is not a ~file panel~@FilePanel@, it will be changed to the file panel.

    #Ctrl+A#, #F4# hotkeys invoke the ~file attributes~@FileAttrDlg@ for drives.

    #Ctrl+A#, #F4# hotkeys can be used to assign a hotkey to plugin item.

    #F3# key shows plugin technical information.

    #Del# key can be used:

     - to ~disconnect~@DisconnectDrive@ network drives;

     - to delete a substituted disk;

     - to detach a virtual disk;

     - to eject disks from CD-ROM and removable drives.
       Ejecting a disk from a ZIP-drive requires administrative privileges.
       A CD-ROM can be closed by pressing #Ins#.

    The #Shift+Del# hotkey is used to prepare a USB storage device for safe
removal. If the disk, for which the removal function is used, is a flash-card
inserted into a card-reader that supports several flash-cards then the
card-reader itself will be stopped.

    #Ctrl+1# - #Ctrl+9# toggle displaying of various information:

    #Ctrl+1# - ^<wrap>disk type;
    #Ctrl+2# - ^<wrap>network name / path associated with a SUBST disk
/ path to virtual disk container;
    #Ctrl+3# - ^<wrap>disk label;
    #Ctrl+4# - ^<wrap>file system;
    #Ctrl+5# - ^<wrap>disk size and free space (this option has two
display modes, press twice to see);
    #Ctrl+6# - ^<wrap>removable disk parameters;
    #Ctrl+7# - ^<wrap>plugins;
    #Ctrl+8# - ^<wrap>CD parameters;
    #Ctrl+9# - ^<wrap>network parameters.

    #Change drive# menu settings are saved in the Far configuration.

    #F9# shows the ~dialog~@ChangeDriveMode@ to control displaying
of this information.

    If the option "~Use Ctrl+PgUp to change drive~@InterfSettings@" is enabled,
pressing #Ctrl+PgUp# works the same as pressing #Esc# - cancels drive selection
and closes the menu.

    Pressing #Shift+Enter# invokes the Windows Explorer showing the root
directory of the selected drives (works only for disk drives and not for
plugins).

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

    The #A# symbol in leftmost menu column means that the corresponding plugin is
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

  #Ins#          - Add a new highlighting group

  #F5#           - Duplicate the current group

  #Del#          - Delete the current group

  #Enter# or #F4#  - ~Edit~@HighlightEdit@ the current highlighting group

  #Ctrl+R#       - Restore the default file highlighting groups

  #Ctrl+Up#      - Move a group up.

  #Ctrl+Down#    - Move a group down.

    The highlighting groups are checked from top to bottom. If it is detected
that a file belongs to a group, no further groups are checked.

    See also: common ~menu~@MenuCmd@ keyboard commands.

@HighlightEdit
$ #Files highlighting and sort groups: editing#
    The #Files highlighting# dialog in the ~Options menu~@OptMenu@ allows to
define file highlighting groups. Each group definition ~includes~@Filter@:

     - one or more ~file masks~@FileMasks@;

     - attributes to include or exclude:
       #[x]# - inclusion attribute - file must have this attribute.
       #[ ]# - exclusion attribute - file must not have this attribute.
       #[?]# - ignore this attribute;

     - normal name, selected name, name under cursor and
       selected name under cursor colors to display file names.
       If you wish to use the default color, set color to "Black
       on black";

     - an optional character to mark files from the group.
       It can be used both with or instead of color highlighting.

    If the option "A file mask or several file masks" is turned off, file masks
will not be analyzed, and only file attributes will be taken into account.

    A file belongs to a highlighting group if:

     - file mask analysis is enabled and the name of the file matches
       at least one file mask (if file mask analysis is disabled,
       the file name doesn't matter);

     - it has all of the included attributes;

     - it has none of the excluded attributes.

    The Compressed, Encrypted, Not indexed, Sparse, Temporary and Reparse point
attributes are valid for NTFS drives only. The #Integrity stream# and
#No scrub data# attributes only supported on ReFS voumes starting from
Windows Server 2012.

@ViewerSettings
$ #Settings dialog: viewer#
    This dialog allows to change the default external or
~internal viewer~@Viewer@ settings.

    External viewer

  #Use for F3#              Run external viewer using #F3#.

  #Use for Alt+F3#          Run external viewer using #Alt+F3#.

  #Viewer command#          Command to execute external viewer.
                          Use ~special symbols~@MetaSymbols@ to specify the
                          name of the file to view.

    Internal viewer

  #Persistent selection#    Do not remove block selection after
                          moving the cursor.

  #Show arrows#             Show scrolling arrows in viewer if the text
                          doesn't fit in the window horizontally.

  #Save file position#      Save and restore positions in the recently
                          viewed files. This option also forces
                          restoring of code page, if the page
                          was manually selected by user, and the file
                          viewing mode (normal/hex).

  #Save bookmarks#          Save and restore bookmarks (current
                          positions) in recently viewed files
                          (created with #RightCtrl+0..9# or
                          #Ctrl+Shift+0..9#)

  #Save file code page#     Save and restore selected file code page.
                          This is automatically enabled if #Save file position#
                          is enabled, as file position depends on its encoding.

  #Save wrap mode#          Save and restore file Wrap/WordWrap mode.

  #Search dialog#           Always returns focus to search text field in
  #auto-focus#              the ~Viewer~@Viewer@ search dialog.

  #Visible '\0'#            Shows visible character instead of space for '\0' character.
                          Character can be set in ~far:config~@FarConfig@ #Viewer.ZeroChar#

  #Tab size#                Number of spaces in a tab character.

  #Show scrollbar#          Show scrollbar in internal viewer. This
                          option can also be switched by pressing
                          #Ctrl+S# in the internal viewer.

  #Maximum line width#      Maximum number of columns for text mode viewer.
                          Min=100, Max=100000, Default=10000.

  #Autodetect#              ~Autodetect~@CodePageAuto@ the code page of
  #code page#               the file being viewed.

  #Default code page#       Select the default code page.

    If the external viewer is assigned to #F3# key, it will be executed only if
the ~associated~@FileAssoc@ viewer for the current file type is not defined.

    Modifications of settings in this dialog do not affect previously opened
internal viewer windows.

    The settings dialog can also be invoked from the ~internal viewer~@Viewer@
by pressing #Alt+Shift+F9#. The changes will come into force immediately but
will affect only the current session.


@EditorSettings
$ #Settings dialog: editor#
    This dialog allows to change the default external and
~internal editor~@Editor@ settings.

    External editor

  #Use for F4#              Run external editor using #F4# instead of
                          #Alt+F4#.

  #Editor command#          Command to execute the external editor.
                          Use ~special symbols~@MetaSymbols@ to specify the name
                          of the file to edit.

    Internal editor

  #Do not expand tabs#      Do not convert tabs to spaces while
                          editing the document.

  #Expand newly entered#    While editing the document, convert each
  #tabs to spaces#          newly entered #Tab# into the appropriate
                          number of spaces. Other tabs won't be
                          converted.

  #Expand all tabs to#      Upon opening the document, all tabs in
  #spaces#                  the document will be automatically
                          converted to spaces.

  #Persistent blocks#       Do not remove block selection after
                          moving the cursor.

  #Del removes blocks#      If a block is selected, pressing #Del# will
                          not remove the character under cursor, but
                          this block.

  #Save file position#      Save and restore positions in the recently
                          edited files. This option also forces
                          restoring of code page, if the page
                          was manually selected by user.

  #Save bookmarks#          Save and restore bookmarks (current
                          positions) in recently edited files
                          (created with #RightCtrl+0..9# or
                          #Ctrl+Shift+0..9#)

  #Auto indent#             Enables auto indent mode when entering
                          text.

  #Cursor beyond#           Allow moving cursor beyond the end of line.
  #end of line#

  #Tab size#                Number of spaces in a tab character.

  #Show scrollbar#          Show scrollbar.

  #Show white space#        Make while space characters (spaces, tabulations,
                          line endings) visible.

  #Select found#            Found text is selected

  #Cursor at the end#       Place the cursor at the end of the found block.

  #Autodetect#              ~Autodetect~@CodePageAuto@ the code page of
  #code page#               the file being edited.

  #Edit files opened#       Allows to edit files that are opened
  #for writing#             by other programs for writing. This mode
                          is handy to edit a file opened for a long
                          time, but it could be dangerous, if a file
                          is being modified at the same time as
                          editing.

  #Lock editing of#         When a file with the Read-only attribute
  #read-only files#         is opened for editing, the editor also
                          disables the modification of the edited
                          text, just as if #Ctrl+L# was pressed.

  #Warn when opening#       When a file with the Read-only attribute
  #read-only files#         is opened for editing, a warning message
                          will be shown.

  #Default code page#       Select the default code page.

    If the external editor is assigned to #F4# key, it will be executed only if
~associated~@FileAssoc@ editor for the current file type is not defined.

    Modifications of settings in this dialog do not affect previously opened
internal editor windows.

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
$ #File attributes dialog#
    With this command it is possible to change file attributes and file time.
Either single file or group of files can be processed. If you do not want to
process files in subfolders, clear the "Process subfolders" option.

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

    The #Compressed#, #Encrypted#, #Not indexed#, #Sparse#, #Temporary#,
#Offline#, #Reparse point# and #Virtual# attributes are available only on NTFS drives. The
#Virtual# attribute is not used in Windows 2000/XP/2003. The #Compressed#
and #Encrypted# attributes are mutually exclusive, that is, you can set only
one of them. You cannot clear the #Sparse# attribute in Windows 2000/XP/2003. The
#Integrity stream# and #No scrub data# attributes only supported on ReFS voumes starting from
Windows Server 2012.


    For ~symbolic links~@HardSymLink@ the dialog will display the path where it refers to.
If this information is not available, then the "#(data not available)#" message will be shown.

  #File date and time#

    Four different file times are supported:

    - last write time;

    - creation time;

    - last access time;

    - change time.

    On FAT drives the hours, minutes, seconds and milliseconds of the last access time are
always equal to zero.

    If you do not want to change the file time, leave the respective field
empty. You can push the #Blank# button to clear all the date and time fields
and then change an individual component of the date or time, for example, only
month or only minutes. All the other date and time components will remain
unchanged.

    The #Current# button fills the file time fields with the current time.

    The #Original# button fills the file time fields with their original
values. Available only when the dialog is invoked for a single file object.


    The #System properties# button invoke the system properties dialog for
selected objects.



@FolderShortcuts
$ #Folder shortcuts#
    Folder shortcuts are designed to provide fast access to frequently used
folders. Press #Ctrl+Shift+0..9#, to create a shortcut to the current folder.
To change to the folder recorded in the shortcut, press #RightCtrl+0..9#. If
#RightCtrl+0..9# pressed in edit line, it inserts the shortcut path into the
line.

    The #Show folder shortcuts# item in the ~Commands menu~@CmdMenu@ can be
used to view, set, edit and delete folder shortcuts.

    When you are editing a shortcut (#F4#), you cannot create a shortcut to a
plugin panel.

    See also: common ~menu~@MenuCmd@ keyboard commands.

@FiltersMenu
$ #Filters menu#
    Using the #Filters menu# you can define a set of file types with user
defined rules according to which files will be processed in the area of
operation this menu was called from.

    The filters menu consists of two parts. In the upper part custom #User#
#filters# are shown, the lower part contains file masks of all the files that
exist in the current panel (including file masks that are selected in the
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

   #Space#,              Items selected using #Space# or '#+#' are
   #Plus#                marked by '+'. If such items are present
                       then only files that match them will be
                       processed.

   #Minus#               Items selected using '#-#' are marked by '-',
                       and files that match then will not be
                       processed.

   #I# and #X#             Similar to #Plus# and #Minus# respectively,
                       but have higher priority when matching.

   #Backspace#           Clear selection from the current item.

   #Shift+Backspace#     Clear selection from all items.


    Filters selection is stored in the Far configuration.

    When a filter is used in a panel, it is indicated by '*' after the sort
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
$ #File descriptions#
    File descriptions can be used to associate text information with a file.
Descriptions of the files in the current folder are stored in this folder in a
description list file. The format of the description file is the file name
followed by spaces and the description.

    Descriptions can be viewed in the appropriate file panel
~view modes~@PanelViewModes@. By default these modes are #Descriptions#
and #Long descriptions#.

    The command #Describe# (#Ctrl+Z#) from the ~Files menu~@FilesMenu@ is used
to describe selected files.

    Description list names can be changed using #File descriptions# dialog from
the ~Options menu~@OptMenu@. In this dialog you can also set local descriptions
update mode. Updating can be disabled, enabled only if panel current view mode
displays descriptions or always enabled. By default Far sets "Hidden" attribute
to created description lists, but you can disable it by switching off the
option "Set "Hidden" attribute to new description lists" in this dialog. Also
here you can specify the position to align new file descriptions in a
description list.

    If a description file has the "read-only" attribute set, Far does not
attempt to update descriptions, and after moving or deleting file objects, an
error message is shown. If the option "#Update read only description file#" is
enabled, Far will attempt to update the descriptions correctly.

    If it is enabled in the configuration, Far updates file descriptions when
copying, moving and deleting files. But if a command processes files from
subfolders, descriptions in the subfolders are not updated.


@PanelViewModes
$ #Customizing file panel view modes#
    The ~file panel~@FilePanel@ can represent information using 10 predefined
modes: brief, medium, full, wide, detailed, descriptions, long descriptions,
file owners, file links and alternative full. Usually it is enough, but if you
wish, you can either customize its parameters or even replace them with
completely new modes.

    The command #File panel modes# from the ~Options menu~@OptMenu@ allows to
change the view mode settings. First, it offers to select the desired mode from
the list. In this list "Brief mode" item corresponds to brief panel mode
(#LeftCtrl+1#), "Medium" corresponds to medium panel mode (#LeftCtrl+2#) and so
on. The last item, "Alternative full", corresponds to view mode called with
#LeftCtrl+0#. After selecting the mode, you can change the following settings:

  - #Column types# - a comma-separated list. Each column type starts with
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

    N[M[D],O,R[F],N] - file name
                 where: M - ^<wrap>show selection marks where: D
- dynamic selection marks;
                        O - ^<wrap>show names without paths (intended
mainly for plugins);
                        R - ^<wrap>right align names that do not fit in
column where: F - right align all names;
                        N - ^<wrap>do not show extensions in name
column;
                 ^<wrap>These modifiers can be used in combination, for
example NMR

    X[R]       - file extension
                 where: R - ^<wrap>right align file extension;

    S[C,T,F,E] - file size
    P[C,T,F,E] - allocation file size
    G[C,T,F,E] - size of file streams
                 where: C - ^<wrap>group digits using the symbol from
Windows settings;
                        T - ^<wrap>use decimal units instead of binary,
i.e., to calculate kilobytes, the size will be divided by 1000 instead
of by 1024; in this mode unit symbol is shown in lower case, e.g., #k#,
#m#, #g# instead of #K#, #M#, #G#;
                        F - ^<wrap>show size as a decimal fraction with
no more than three digits before decimal point, e.g., 999 bytes will
be shown as #999#, while 1024 bytes as #1.00 K#; note that the behavior
depends on whether the #T# modifier is used;
                        E - ^<wrap>economic mode, no space between the
size and the unit symbol, e.g., #1.00k#;

    D          - file last write date
    T          - file last write time

    DM[B,M]    - file last write date and time
    DC[B,M]    - file creation date and time
    DA[B,M]    - file last access date and time
    DE[B,M]    - file change date and time
                 where: B - brief (Unix style) file time format;
                        M - use text month names;

    A          - file attributes
    Z          - file descriptions

    O[L]       - file owner
                 where: L - show domain name;

    LN         - number of hard links

    F          - number of streams

    If the column types description contains more than one file name column,
the file panel will be displayed in multicolumn form.

    File attributes are denoted as follows:

       #R#         - Read only
       #A#         - Archive
       #H#         - Hidden
       #S#         - System
       #C#         - Compressed
       #E#         - Encrypted
       #I#         - Not content indexed
       #D#         - Directory
       #$#         - Sparse
       #T#         - Temporary
       #O#         - Offline
       #L#         - Reparse point
       #V#         - Virtual
       #G#         - Integrity stream
       #N#         - No scrub data

    By default the size of the attributes column is 6 characters. To display
the additional attributes it is necessary to manually increase the size of
the column.

  - #Column widths# - used to change width of panel columns.
If the width is equal to 0, the default value will be used. If the width of
the Name, Description or Owner column is 0, it will be calculated
automatically, depending upon the panel width. For correct operation with
different screen widths, it is highly recommended to have at least one column
with automatically calculated width. Width can be also set as a percentage of
remaining free space after the fixed width columns by adding the "%" character
after the numerical value. If the total size of such columns exceeds 100%,
their sizes are scaled.

    Incrementing the default width of the file time column or file date and
time column by 1 will force a 12-hour time format. Further increase will lead
to the display of seconds and milliseconds.

    To display years in 4-digits format increase the date column width by 2.

    Enabling links, streams and owner columns (G, LN, F and O) can significantly
slow down the directory reading.

  - #Status line column types# and #Status line column widths# -
similar to "Column types" and "Column widths", but for panel status line.

  - #Fullscreen view# - force fullscreen view instead of half-screen.

  - #Align file extensions# - show file extensions aligned.

  - #Align folder extensions# - show folder extensions aligned.

  - #Show folders in uppercase# - display all folder names in upper
case, ignoring the original case.

  - #Show files in lowercase# - display all file names in lower case,
ignoring the original case.

  - #Show uppercase file names in lowercase# - display all uppercase
file names in lower case. By default this option is on, but if you wish
to always see the real files case, switch it, "Show folders in uppercase"
and "Show files in lowercase" options off. All these settings only change
the method of displaying files, when processing files Far always uses the
real case.

    See also: common ~menu~@MenuCmd@ keyboard commands.

@SortGroups
$ #Sort groups#
    File sort groups can be used in #by name# and #by extension#
~file panel~@FilePanel@ sort modes. Sort groups are applied by
pressing #Shift+F11# and allow to define additional file sorting rules,
complementary to those already used.

    Each sort group contains one or more comma delimited
~file masks~@FileMasks@. If one sort group position in the group list
is higher than another and an ascending sort is performed, all files
belonging to this group files will be higher than those belonging to
following groups.

    The command #Edit sort groups# from the ~Commands menu~@CmdMenu@ is used to
delete, create and edit sort groups, using #Del#, #Ins# and #F4#. The groups
above the menu separator are applicable to the file panel start, and included
files will be placed higher than those not included to any group. The groups
below the menu separator are applicable to the file panel end, and included
files will be placed lower than those not included.


@FileMasks
$ #File masks#
    File masks are frequently used in Far commands to select single file or
folder, or group of items. Masks can contain common valid file name symbols,
wildcards ('*' and '?') and special expressions:

    #*#           zero or more characters;

    #?#           any single character;

    #[cx-z]#      any character enclosed by the brackets.
                Both separate characters and character intervals
                are allowed.

    For example, files ftp.exe, fc.exe and f.ext can be selected using mask
f*.ex?, mask *co* will select both color.ini and edit.com, mask [c-ft]*.txt
can select config.txt, demo.txt, faq.txt and tips.txt.

    In many Far commands you can enter several file masks separated with commas
or semicolons. For example, to select all the documents, you can enter
#*.doc,*.txt,*.wri# in the "Select" command.

    It is allowed to put any of the masks in quotes but not the whole list. For
example, you have to do this when a mask contains any of the delimiter
characters (a comma or a semicolon), so that the mask doesn't get confused with
a list.

    File mask surrounded with slashes #/# is treated as ~Perl regular expression~@RegExp@.

    Example:
    #/(eng|rus)/i#  any files with filenames containing string “eng” or “rus”,
                  the character case is not taken into account.

    An #exclude mask# is one or multiple file masks that must not be matched by the
files matching the mask. The exclude mask is delimited from the main mask by
the character '#|#'.

^Usage examples of exclude masks:
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
and the '|' character separates include masks from exclude masks.

    File masks can be joined into ~groups~@MaskGroupsSettings@.

@SelectFiles
$ #Selecting files#
    ~File panel~@FilePanel@ items (files and folders) can be selected
for group processing. If no items are selected, only the item under
cursor will be processed.

  #Keyboard Selection#

    #Ins# toggles selection on the item under cursor and moves the
cursor down.

    #Shift+Arrow keys# move the cursor while selecting or deselecting
items along the way. The action (selection or deselection) depends
on the state of the item under cursor before pressing the key
combination.

    #Gray +# selects, and #Gray -# deselects the items using one or more
~File masks~@FileMasks@. #†#

    #Gray *# inverts the current selection. #†#

    #Ctrl+<Gray +># selects, and #Ctrl+<Gray -># deselects all items
with the same #extension# as that of the item under cursor. #†#

    #Alt+<Gray +># selects, and #Alt+<Gray -># deselects all items with
the same #name# as that of the item under cursor. #†#

    #Shift+<Gray +># selects, and #Shift+<Gray -># deselects all items. #†#

    #Ctrl+<Gray *># inverts the current selection on all items,
including folders.

    #Ctrl+M# restores the last selection.

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
or deselected. Otherwise, the selection on the folders is changed
as well.

@CopyFiles
$ #Copying, moving, renaming and creating links#
    The following commands can be used to copy, move and rename files and folders:

  Copy ~selected~@SelectFiles@ files                                           #F5#

  Copy the file under cursor regardless of selection      #Shift+F5#

  Rename or move selected files                                 #F6#

  Rename or move the file under the cursor                #Shift+F6#
  regardless of selection

  Create ~file links~@HardSymLink@                                         #Alt+F6#

    For a folder: if the folder at the specified target path (relative
or absolute) exists, the source folder will be copied / moved inside the
target folder. Otherwise, a new folder will be created at the target
path and the contents of the source folder will be copied / moved into
the newly created folder.

    For example, when moving #c:\folder1\# to #d:\folder2\#:

    - ^<wrap>if #d:\folder2\# exists, the contents of #c:\folder1\# will be moved into
#d:\folder2\folder1\#;

    - ^<wrap>otherwise, the contents of #c:\folder1\# will be moved into the newly
created #d:\folder2\#.

    If the option “#Process multiple destinations#” is enabled, you can specify
multiple copy or move targets on the input line. The targets should be separated
with character “#;#” or “#,#”. If a target name contains these characters,
enclose it in double quotes.

    If you want to create the destination folder before copying,
append backslash to its name.

    If ~Panel.Tree.TurnOffCompletely~@Panel.Tree.TurnOffCompletely@
parameter in ~far:config~@FarConfig@ is set to “false,” you can use
~Find folder~@FindFolder@ dialog to select the target path. The
following shortcuts open the dialog with different pre-selected folders:

    - ^<wrap>#F10# selects the folder from the active panel.

    - ^<wrap>#Alt+F10# selects the folder from the passive panel.

    - ^<wrap>#Shift+F10# selects the specified target folder. If several
paths are entered on the input line, only the first one is used.

    If the option “#Process multiple destinations#” is enabled, the folder
selected in the tree is appended to the input line.

    Whether copying, moving or renaming files works for a plugin depends
upon the plugin functionality.

    If the destination disk of the copy or move operation becomes full,
it is possible to either cancel the operation or replace the disk and
select the “Split” option. In the latter case the file being copied will
be split between disks. This feature is available only when the
“#Use system copy routine#” option in the ~System settings~@SystemSettings@
dialog is switched off.

    The #Access rights# parameter is valid only for the NTFS file system
and controls how access rights of the created files and folders are set.
The #Default# option leaves access rights processing to the file system.
The #Copy# option applies the access rights of the original objects. The
#Inherit# option applies the inheritable access rights of the
destination’s parent folder.

    The “#Already existing files#” parameter controls Far behavior
if the target file with the same name already exists.
    Possible values:
    ^<wrap>#Ask# - a ~confirmation dialog~@CopyAskOverwrite@ will be shown;
    ^<wrap>#Overwrite# - all target files will be replaced;
    ^<wrap>#Skip# - target files will not be replaced;
    ^<wrap>#Rename# - existing target files will stay unchanged, copied
files will be renamed;
    ^<wrap>#Append# - target file will be appended with the file being copied;
    ^<wrap>#Only newer file(s)# - only files with the newer write date and time
will be copied;
    ^<wrap>#Also ask on R/O files# - controls whether an additional confirmation
dialog should be displayed for the read-only files.

    The “#Use system copy routine#” option of the ~System settings~@SystemSettings@
dialog enables the use of Windows operating system function CopyFileEx
(or CopyFile if CopyFileEx is not available). This may be useful
on NTFS, because CopyFileEx optimizes disk space allocation and copies
extended file attributes. If this option is off, the internal
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
option "#Use system copy routine#" in the ~system settings~@SystemSettings@
dialog.

    If you are copying a file with multiple streams to a volume with a file
system other than NTFS, you will also lose data - only the main stream will be
copied.


@ErrLoadPlugin
$ #Error: plugin not loaded#
   This error message can appear in the following cases:

   1. A dynamic link library not present on your system is required
      for correct operation of the plugin.

   2. For some reason, the plugin returned an error code
      telling the system to abort plugin loading.

   3. The Dll file of the plugin is corrupt.


@ScrSwitch
$ #Screens switching#
    Far allows to open several instances of the internal viewer and editor at
the same time. Use #Ctrl+Tab#, #Ctrl+Shift+Tab# or #F12# to switch between
panels and screens with these instances. #Ctrl+Tab# switches to the next
screen, #Ctrl+Shift+Tab# to the previous, #F12# shows a list of all available
screens.

    The number of background viewers and editors is displayed in the left panel
upper left corner. This can be disabled in ~Panel settings~@PanelSettings@.

    See also: common ~menu~@MenuCmd@ keyboard commands.

@ApplyCmd
$ #Apply command#
    With #Apply command# item in ~Files menu~@FilesMenu@ it is possible to
apply a command to each selected file. The same ~special symbols~@MetaSymbols@
as in ~File associations~@FileAssoc@ should be used to denote the file name.

    For example, 'type !.!' will output to the screen all selected files, one
at a time, and the command 'rar32 m !.!.rar !.!' will move all selected files
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
environment “variable” is defined.

    #if exist file1 if not exist file2 if defined variable command#

    #PUSHD path#

    Stores the current path for use by the “POPD” command, then changes
the current path on the active panel to the specified “path”.

    #POPD#

    Changes the current path on the active panel to that stored by the
“PUSHD” command.

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
above, Far Manager will invoke the operating system command processor
to execute the command.

    2. ^<wrap>Far Manager executes the commands listed above in the
following contexts:
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
    #s# - ^<wrap>consider the whole text as one line, '.' matches any character;
    #m# - ^<wrap>consider the whole text as multiple lines. ^ and $ match the
    beginning and the end of any "inner" string;
    #x# - ^<wrap>ignore space characters (unscreened ones, i.e. without backslash before).
This is useful to outline the complex expressions.

    #regexp# - the sequence of characters and metacharacters. The characters are
letters and digits, any other symbol becomes character when screened, i.e.
prepended the backslash #\#.

    Pay attention that all slashes and backslashes in regular expression must
be prepended with the symbol #\# to differ from other special symbols or with
the end of expression. An example: the string "big\white/scary" looks in the
form of regular expression like "big\\\\white\/scary".

    #Metacharacters#

    #\#  - ^<wrap>the next symbol is treated as itself, not a metacharacter
    #^#  - ^<wrap>the beginning of string
    #$#  - ^<wrap>the end of string
    #|#  - ^<wrap>the alternative. Either expression before or after #|# has to match.

          ^<wrap>An example: "\d+\w+|Hello\d+" means "(\d+\w+)|(Hello\d+)", not "\d+(\w+|H)ello\d+".

    #()# - ^<wrap>grouping - it is used for references or when replacing matched text.
    #[]# - ^<wrap>character class - the metacharacter which matches any symbol
or range of symbols enumerated in #[]#. Ranges are defined as [a-z].
Metacharacters are not taken into account in character classes. If the first
symbol in class is #^# then this is a negative class. If the character #^# has
to be added to class, then it either must not to be at first place or it must
be prepended with #\#.

    Except grouping, the parentheses are used for the following operations:
    #(?:pattern)#  - ^<wrap>usual grouping, but it does not get a number.
    #(?=pattern)#  - ^<wrap>the forward lookup. The matching continues from
the same place, but only if the pattern in these parentheses has matched. For
example, #\w+(?=\s)# matches the word followed by space symbol, and the space
is not included into the search result.
    #(?!pattern)#  - ^<wrap>the negation of forward lookup. The matching
continues from the same place if the pattern does not match. For example,
#foo(?!bar)# matches any "foo" without following "bar". Remember that this
expression has zero size, which means that #a(?!b)d# matches #ad# because #a#
is followed by the symbol, which is not #b# (but #d#), and #d# follows the
zero-size expression.
    #(?<=pattern)# - ^<wrap>the backward lookup. Unfortunately, the pattern must have fixed length.
    #(?<!pattern)# - ^<wrap>the negation of backward lookup. The same restriction.

    #(?{name}pattern)# - brackets with name.
    Name can be empty (in such case you cannot refer to this brackets) or must
contain word symbols (\w) and spaces (\s).


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

    Greedy quantifier captures as much symbols as possible, and only if
    further match fails, it "returns" the captured string (the rollback
happens, which is rather expensive).
    When expression "A.*Z" is matched to string
"AZXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", #.*# captures the whole string, and then
rolls back symbol by symbol until it finds Z. On the opposite, if the expression
is "A.*?Z" then Z is found at once. Not greedy quantifier is also known as
#minimizing#, it captures minimal possible quantity of symbols, and only if
further match fails it captures more.

    #Special symbols#

   Non-letter and non-digit symbol can be prepended by '#\#' in most cases,
but in case of letters and digits this must be done with care because this is
the way the special symbols are written:

    #.#    - ^<wrap>any symbol except carriage return. If there is “s” among
the options then this can be any symbol.
    #\t#   - tab (0x09)
    #\n#   - new line (lf, 0x0a)
    #\r#   - carriage return (cr, 0x0d)
    #\f#   - form feed (0x0c)
    #\a#   - bell (0x07)
    #\e#   - escape (0x1b)
    #\xNNNN# - hex character, where N - [0-9A-Fa-f].
    #\Q#   - ^<wrap>the beginning of metacharacters quoting - the whole quoted
text is treated as text itself, not the regular expression
    #\E#   - the end of metacharacters quoting
    #\w#   - letter, digit or '_'.
    #\W#   - not \w
    #\s#   - space symbol (tab/space/lf/cr).
    #\S#   - not \s
    #\d#   - digit
    #\D#   - not digit
    #\i#   - letter
    #\I#   - not letter
    #\l#   - lower case symbol
    #\L#   - not lower case symbol
    #\u#   - upper case symbol
    #\U#   - not upper case symbol
    #\b#   - ^<wrap>the word margin - means that to the left or right from the
current position there is a word symbol, and to the right or left,
accordingly, there is non-word symbol
    #\B#   - not \b
    #\A#   - the beginning of the text, disregard the option “m”
    #\Z#   - the end of the text, disregard the option “m”
    #\O#   - ^<wrap>the no-return point. If the matching has passed by this symbol,
it won't roll back and and will return "no match". It can be used in complex expressions
after mandatory fragment with quantifier. This special symbol can be used when
big amounts of data are processed.
         Example:
         /.*?name\O=(['"])(.*?)\1\O.*?value\O=(['"])(.*?)\3/
         ^<wrap>Strings containing "name=", but not containing "value=", are processed (in fact, skipped) faster.

    #\NN#  - ^<wrap>reference to earlier matched parentheses . NN is a positive integer.
Each parentheses except (?:pattern), (?=pattern), (?!pattern), (?<=pattern), (?<!pattern) and (?{name}pattern)
have a number (in the order of appearance).

         Example:
         "(['"])hello\1" matches to "hello" or 'hello'.

    #\p{name}# - ^<wrap>inner regexp reference to it's parsed bracket with specified #name#.


    #Examples:#

    #/foobar/#
       matches to "foobar", but not to "FOOBAR"
    #/ FOO bar /ix#
       matches to "foobar" and "FOOBAR"
    #/(foo)?bar/#
       matches to "foobar" and "bar"
    #/^foobar$/#
       matches to "foobar" only, but not to "foofoofoobarfoobar"
    #/[\d\.]+/#
       matches to any number with decimal point
    #/(foo|bar)+/#
       matches to "foofoofoobarfoobar" and "bar"
    #/\Q.)))$\E/#
       equals to "\.\)\)\)\$"

@RegExpRepl
$ #Regular expressions in replace#
    In "Replace with" line one can use special replace string regular
expressions:

    #$0#..#$N#

    The found group numbers, they are replaced with appropriate groups.
The numbers are assigned to the groups in order of opening parentheses
sequence in regular expression. #$0# means the whole found sequence.

    $#{#name#}#     Found pattern with specified #name#.


@ElevationDlg
$ #Запрос привилегий администратора#
    Во время выполнения разнообразных операций с файловой системой у текущего
пользователя может не хватать прав. В этом случае Far Manager пытается повторить 
попытку от имени администратора (повысить права текущего пользователя).

    Доступны опции:

    #Выполнить это действие для всех текущих объектов#
    Не задавать вопросов о повышении прав для текущей файловой операции.

    #Больше не спрашивать в текущей сессии#
    В текущем сеансе Far Manager будет самостоятельно повышать привилегии без запроса пользователя.

    См. также опцию #Запрос прав администратора# в ~Системных параметрах~@SystemSettings@.

@KeyMacro
$ #Macro command #
    By default macros are loaded from files with #.lua# and #.moon# extensions residing in folder
#%FARPROFILE%\\Macros\\scripts#. See more details in #%FARHOME%\\Encyclopedia\\macroapi_manual.en.chm#.

    Keyboard macro commands or macro commands - are recorded sequences of key
presses that can be used to perform repetitive task unlimited number of times
by pressing a single hotkey.

    Each macro command has the following parameters:

    - a hotkey, that will execute the recorded sequence when
      pressed;
    - additional ~settings~@KeyMacroSetting@, that influence the method and
      the area of execution of the recorded sequence.

    Macro commands are mostly used for:

    1. Performing repetitive task unlimited number of times by
       pressing a single hotkey.
    2. Execution of special functions, which are represented by
       special commands in the text of the macro command.
    3. Redefine standard hotkeys, which are used by Far for
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

    Attention: The area of execution, to which the macro command will
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

    1. any key;
    2. any key combination with #Ctrl#, #Alt# and #Shift# modifiers;
    3. any key combination with two modifiers.
       Far allows to use the following double modifiers:
       #Ctrl+Shift+<key>#, #Ctrl+Alt+<key># and #Alt+Shift+<key>#

    A macro command #can't# be assigned to the following key combinations:
#Alt+Ins#, #Ctrl+<.>#, #Ctrl+Shift+<.>#, #Ctrl+Alt#, #Ctrl+Shift#, #Shift+Alt#,
#Shift+<symbol>#.

    It is impossible to enter some key combinations (in particular #Enter#,
#Esc#, #F1#, #Ctrl+F5#, #MsWheelUp# and #MsWheelDown# with #Ctrl#, #Shift#,
#Alt#) in the hotkey assignment dialog because of their special meanings. To
assign a macro command to such key combination, select it from the dropdown
list.


@KeyMacroRecPlay
$ #Macro command: recording and playing-back#
    A ~macro command~@KeyMacro@ can be played-back in one of the two following
modes:

    1. General mode: keys pressed during the recording or the
       playing-back #will be# sent to plugins.

    2. Special mode: keys pressed during the recording or the
       playing-back #will not be# sent to plugins that intercept
       editor events.

    For example, if some plugin processes the key combination - #Ctrl+A#, then
in the special mode this plugin will not receive focus and will not do what it
usually does as a reaction to this combination.

    Creation of a macro command is achieved by the following actions:

    1. To start recording a macro command

       Press #Ctrl+<.># (#Ctrl# and a period pressed together) to record
       a macro in the general mode or #Ctrl+Shift+<.># (#Ctrl#, #Shift# and
       a period pressed together) to record a macro in the special
       mode.

       As the recording begins the symbol '\4FR\-' will appear at the
       upper left corner of the screen.

    2. Contents of the macro command.

       All keys pressed during the recording will be saved with the
       following exceptions:

       - only keys processed by Far will be saved. Meaning that if
         during the macro recording process an external program is
         run inside the current console then only the keys pressed
         before the execution and after completion of that program
         will be saved.

    3. To finish recording the macro command.

       To finish a macro recording there are special key
       combinations. Because a macro command can be additionally
       configured there are two such combinations: #Ctrl+<.># (#Ctrl#
       and a period pressed together) and #Ctrl+Shift+<.># (#Ctrl#,
       Shift and a period pressed together). Pressing the first
       combination will end the recording of the macro command
       and will use the default settings for its playback. Pressing
       the second combination will end the recording of the macro
       command and a dialog showing macro command ~options~@KeyMacroSetting@
       will appear.

    4. Assign a hotkey to the macro command

    When the macro recording is finished and all the options are set the
    ~hotkey assignment~@KeyMacroSetting@ dialog will appear, where the hotkey that
    will be used to execute the recorded sequence can be set.

    Playing back a macro is indicated by showing the character '\2FP\-' in the upper-left screen corner.
See also "~Macros.ShowPlayIndicator~@Macros.ShowPlayIndicator@" for turning that indication on/off.


@KeyMacroDelete
$ #Macro command: deleting a macro command#
    To delete a ~macro command~@KeyMacro@ an empty (containing no commands)
macro should be recorded and assigned the hotkey of the macro command that
needs to be deleted.

    This can be achieved by the following steps:

    1. Start recording a macro command (#Ctrl+<.>#)
    2. Stop recording a macro command (#Ctrl+<.>#)
    3. Enter or select in the hotkey assignment
       dialog the hotkey of the macro command that
       needs to be deleted.

    Attention: after deleting a macro command, the key combination
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

    Allows to execute the macro command immediately after the Far Manager is
started.

    The following execution conditions can be applied for the active and
passive panels:

     #Plugin panel#
         [x] - execute only if the current panel is a plugin panel
         [ ] - execute only if the current panel is a file panel
         [?] - ignore the panel type

     #Execute for folders#
         [x] - execute only if a folder is under the panel cursor
         [ ] - execute only if a file is under the panel cursor
         [?] - execute for both folders and files

     #Selection exists#
         [x] - execute only if there are marked files/directories
               on the panel
         [ ] - execute only if there are no marked files/directories
               on the panel
         [?] - ignore the file selection state

   Other execution conditions:

     #Empty command line#
         [x] - execute only if the command line is empty
         [ ] - execute only if the command line is not empty
         [?] - ignore the command line state

     #Selection block present#
         [x] - execute only if there is a selection block present
              in the editor, viewer, command line or dialog
              input line
         [ ] - execute only if there is no selection present
         [?] - ignore selection state


   Notes:

    1. Before executing a macro command, all of the above conditions are
checked.

    2. Some key combinations (including #Enter#, #Esc#, #F1# and #Ctrl+F5#,
#MsWheelUp#, #MsWheelDown# and other mouse keys combined with #Ctrl#, #Shift#, #Alt#) cannot be entered
directly because they have special functions in the dialog. To assign a macro
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

@FarConfig
$ #Configuration editor#
    Starts with the command #far:config#

    Allows to view and edit all Far Manager’s options.
    Most options can be changed from the ~Options menu~@OptMenu@, however some options are available only here or using configuration import.
The options are displayed in a list with three fields per item: the name in the SectionName.ParamName format (for example, Editor.TabSize),
the type (boolean, 3-state, integer, string), and the value (for the integer type, hexadecimal and symbolic representations additionally displayed).
If current value of an option is other than the default, the option is marked with the '*' symbol to the left of the name.

    Besides the list navigation keys, the following key combinations are supported:

    #Enter# or #F4#   Change option value
                  boolean and 3-state are changed in place,
                  for integer and string a dialog is opened.

    #Shift+F4#      For the integer type, hexadecimal editor dialog is opened,
                  for other types works as #F4#.

    #Ctrl+H#        Hide/show options having default values.

    #Shift+F1#      Show option help, if available.

    #Ctrl+Alt+F#    Toggle quick filtering mode.

@Codepages.NoAutoDetectCP
$ #far:config Codepages.NoAutoDetectCP#
    This string parameter defines the code pages which will be excluded
from Universal Codepage Detector (UCD) autodetect. Sometimes, especially
on small files, UCD annoyingly choses wrong code pages.

    The default value is empty string #""#. In this case all code pages
detectable by UCD (about 20, much less than there is usually available
in the system) are enabled.

    If this parameter is set to string #"-1"# and the #Other# section
of the ~Code pages~@CodePagesMenu@ menu is hidden (#Ctrl+H# key
combination), only #System# (ANSI, OEM), #Unicode#, and #Favorites# code
pages will be enabled for UCD. If the #Other# section is visible, all
code pages are enabled.

    Otherwise, this parameter should contain comma separated list
of code page numbers disabled for UCD. For example,
#"1250,1252,1253,1255,855,10005,28592,28595,28597,28598,38598"#.

    Since Unicode code pages (1200, 1201, 65001) are detected outside
of UCD, they cannot be disabled even if they appear on the exclusions
list.

    This parameter can be changed via ~far:config~@FarConfig@ only.

@Help.ActivateURL
$ #far:config Help.ActivateURL#
    Параметр позволяет управлять активацией URL ссылок в HLF-файлах:

     0 - отключить активацию.
     1 - активация включена.
     2 - активация включена, но выдавать предупреждающее сообщение.

    По умолчанию значение = 1 (разрешено).

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Confirmations.EscTwiceToInterrupt
$ #far:config Confirmations.EscTwiceToInterrupt#
    Параметр позволяет менять поведение при нажатии Esc в диалоге подтверждения прерывания операции.

    Может быть одним из следующих значений:

     0 - ^<wrap>Нажатие кнопки ESC закрывает сообщение и продолжает выполнение операции.
     1 - ^<wrap>Нажатие кнопки ESC закрывает сообщение и прерывает выполнение операции

    По умолчанию значение = 0 (закрыть сообщение и продолжить выполнение операции).

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.AllCtrlAltShiftRule
$ #far:config System.AllCtrlAltShiftRule#
    Параметр задаёт поведение комбинации Ctrl+Alt+Shift для временного гашения объектов интерфейса.

    Номера битов:
     0 - Панели.
     1 - Редактор.
     2 - Внутренняя программа просмотра.
     3 - Окно подсказки.
     4 - Диалоги.

    Если бит установлен, гашение разрешено.

    По умолчанию разрешено гашение всех объектов.

    See also ~System.CASRule~@System.CASRule@

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.CASRule
$ #far:config System.CASRule#
    Параметр позволяет отключать комбинацию Ctrl+Alt+Shift для временного гашения объектов интерфейса.
    Различаются комбинации левого и правого Ctrl+Alt+Shift.

    Номера битов:
      0 - левая комбинация Ctrl+Alt+Shift.
      1 - правая комбинация Ctrl+Alt+Shift.

    Если бит установлен, срабатывает гашение экрана.

    По умолчанию разрешены обе комбинации.

    See also ~System.AllCtrlAltShiftRule~@System.AllCtrlAltShiftRule@

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Panel.ShellRightLeftArrowsRule
$ #far:config Panel.ShellRightLeftArrowsRule#
    Параметр позволяет управлять поведением стрелок влево/вправо (как на основной, так и на дополнительной клавиатуре).

    Значения:
      0 - ^<wrap>поведение как у 1.70: если командная строка непустая, то клавиши
Left/Right и Num4/Num6 действуют по-разному в зависимости от
режима панели: если имена файлов отображаются в несколько колонок
(по умолчанию режимы 2 и 3), то команды вправо/влево применяются
к панели (как и при пустой командной строке); а если имена файлов
отображаются в одну колонку (по умолчанию все остальные режимы),
то команды вправо/влево применяются к командной строке.
      1 - ^<wrap>клавиши Left/Right и Num4/Num6 при включённой панели всегда
применяются только к ней, независимо от содержимого командной
строки и режима панели.
          Примечание: в командной строке есть CtrlD/CtrlS.

    По умолчанию значение = 0.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Panel.Layout.ScrollbarMenu
$ #far:config Panel.ShellRightLeftArrowsRule#
    Параметр разрешает показ полосы прокрутки в меню, если пунктов больше, чем высота меню. Если значение =0, то Far не будет отображать полосу прокрутки.

    По умолчанию значение = 1 (отображать полосу прокрутки).

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Panel.CtrlFRule
$ #far:config Panel.CtrlFRule#
    Параметр задаёт поведение Ctrl+F.

    Если = 0, то название файла помещается в командную строку как есть,
иначе - с учётом отображения на панелях (т.е. может приводиться к
нижнему регистру или к короткому имени).
    
    По умолчанию значение = 0.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Panel.CtrlAltShiftRule
$ #far:config Panel.CtrlAltShiftRule#
    Параметр задаёт поведение комбинации Ctrl+Alt+Shift для временного гашения панелей:

     0 - гасить только панели (подобно Ctrl+O).
     1 - гасить панели и командную строку.
     2 - гасить панели, командную строку и KeyBar.

    По умолчанию действует правило 0.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Panel.RightClickRule
$ #far:config Panel.RightClickRule#
    Параметр задаёт поведение правой кнопки мыши для случая, если нажали кнопку на пустой колонке панели:

     0 - ^<wrap>позиционирование и пометка последнего файла в предыдущей колонке.
     1 - ^<wrap>в предыдущей колонке файл позиционируется без пометки (аналогично нажатию левой кнопки мыши).
     2 - ^<wrap>не изменять позицию и не помечать файл (по умолчанию).

    В любом случае - если колонка не пуста, то происходит пометка файла.

    По умолчанию значение = 2.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.ExcludeCmdHistory
$ #far:config System.ExcludeCmdHistory#
    Параметр позволяет определять, какие типы команд не будут помещаться в историю. 
Проверка идёт по битовой маске. Если бит установлен, данный тип команд в историю не помещается.

    Номера битов:

     0 - не помещать в историю команды ассоциаций Windows
     1 - не помещать в историю команды ассоциаций Far
     2 - не помещать в историю команды запуска с панели
     3 - не помещать в историю команды запуска из командной строки
    
    По умолчанию значение = 0 (помещать в историю все команды).

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.Executor.RestoreCP
$ #far:config System.Executor.RestoreCP#
    Параметр позволяет управлять восстановлением кодовой страницы после запуска и отработки внешних
программ в окне Far Manager.

    Некоторые программы изменяют кодовую страницу консольного окна и после
своей обработки не восстанавливают предыдущее значение. Может быть одним из следующих значений:

     0 - "оставить всё как есть" (не восстанавливать значение)
     1 - восстанавливать предыдущее значение кодовой страницы

    По умолчанию значение = 1 (восстанавливать значение).

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.Executor.UseAppPath
$ #far:config System.Executor.UseAppPath#
    При запуске на исполнение содержимого командной строки Far ищет
исполняемый модуль по  следующей логике (попеременно подставляя расширения, перечисленные в переменной окружения  %PATHEXT%):

     1. Текущий каталог
     2. Каталоги, которые перечислены в переменной окружения %PATH%
     3. Windows 95: Системный каталог Windows (SYSTEM).
        Windows NT: 32-битный системный каталог Windows (SYSTEM32)
     4. Windows NT: 16-битный системный каталог Windows (SYSTEM)
     5. Каталог Windows.

     Если параметр "System.Executor.UseAppPath" равен 1, то дополнительно производится поиск исполняемых модулей в реестре:

     6. Содержимое ветки реестра:
        [HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\App Paths]

     7. Содержимое ветки реестра:
        [HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\App Paths]

    Независимо от состояния этого параметра, модуль, прописанный в "App Paths", будет запущен проводником, если для запуска
используется комбинация Shift+Enter.

    По умолчанию значение = 1 (проверять ветки реестра)

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.Executor.ExcludeCmds
$ #far:config System.Executor.ExcludeCmds#
    Параметр позволяет задавать набор команд которые будут сразу передаваться 
в %comspec% для выполнения, поиск в PATH и т.п. не будет произведён. 
    
    Разделитель команд - символ ';'. Например, если "System.Executor.ExcludeCmds" задан списком "DATE;ECHO",  
то при вводе 'date' будет исполнена внутренняя команда CMD.EXE/COMMAND.COM. Для исполнения внешней команды 
"date.exe" необходимо точно написать её название. В тоже время, если "date.exe" доступно в %PATH% и из списка
"ExcludeCmds" убрать "DATE", то внутренняя команда ком.процессора никогда не будет исполнена.

    Готовые настройки для CMD.EXE, COMMAND.COM и TCCLE.EXE (известный ранее как 4NT.EXE) находятся в каталоге Addons\\SetUp, файлы "Executor.???.farconfig".

    Команды "CLS", "REM", "CD" и "CHDIR" Far обрабатывает самостоятельно. Эти команды не включены в "Executor.???.farconfig".

    Команды "IF", "CHCP" и "SET" Far обрабатывает с ограниченной функциональностью - если синтаксис 
отличается от приведённого в разделе "~Команды операционной системы~@OSCommands@", то команда 
передаётся на дальнейшую обработку ком.процессору.

    По умолчанию список "ExcludeCmds" пуст.

    В параметре можно использовать переменные среды.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.Executor.ComspecArguments
$ #far:config System.Executor.ComspecArguments#

    Arguments for command processor. #{0}# is a placeholder for the entire executing command.
    If your processor uses different keys or quotes you can change them here.

    Default value: #/S /C "{0}"# (compatible with cmd.exe)

    This parameter can be changed via ~far:config~@FarConfig@ only.

@System.Executor.FullTitle
$ #far:config System.Executor.FullTitle#
    Параметр позволяет задавать вид заголовка консоли при запуске файла на исполнение.

    Может быть одним из следующих значений:

      0 - в заголовке консоли отображается то, что вводил пользователь.
      1 - в заголовке консоли отображается полный путь к исполняемому файлу.

    По умолчанию значение = 0 (то, что вводил пользователь).

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Interface.FormatNumberSeparators
$ #far:config Interface.FormatNumberSeparators#
    Параметр позволяет определять символы, используемые в качестве разделителей групп разрядов и целой/дробной части чисел.
    Первый символ - разделитель групп разрядов.
    Второй символ - разделитель целой и дробной части.

    По умолчанию значение - "" (использовать региональные настройки ОС).

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.Executor.BatchType
$ #far:config System.Executor.BatchType#
    Параметр позволяет задавать список расширений файлов, по которым Far Manager будет различать какие
файлы являются пакетными (Batch-файлы, обрабатываемые командным процессором) и будут исполняться 
в консоли Far Manager при нажатии Enter на соответствующем элементе панели  
(прочие - в отдельном консольном окне).

    Формат параметра: <.><Расширение><;>[<.><Расширение><;>]
    Разделитель расширений - символ ';'.
    Одиночный символ ";" задаёт пустой список (в этом случае ни один Batch-файл Far не будет исполнять).

    Например, список ".BAT;.BTM;" указывает, что Batch-файлами являются файлы "*.BAT" и "*.BTM".

    В DOS/Windows 9x пакетными файлами считаются файлы, имеющие расширение ".BAT". В линейке Windows на базе NT - ".BAT" и ".CMD".  
Командный процессор TCC/LE (известный ранее как 4NT; ~http://jpsoft.com~@http://jpsoft.com@) считает Batch-файлами файлы с расширением ".BTM". 
Если в списке указано расширение ".BTM", но в системе не прописаны настройки для этого типа файлов (например, не
установлен TCCLE/4NT), то штатный командный процессор (command.com или cmd.exe) не будет исполнять такие пакетные файлы.

    По умолчанию значение = ".BAT;.CMD;".

    В параметре можно использовать переменные среды.
    Если после раскрытия переменных среды список окажется пуст, будет использован список ".BAT;.CMD;".

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.Executor.~
$ #far:config System.Executor.~~#
    Параметр позволяет менять путь к домашней папке для команды "~CD ~~~@OSCommands@".

    По умолчанию значение = "%FARHOME%".

    Значение также можно поменять в диалоге ~Настройка командной строки~@CmdlineSettings@.

@System.CmdHistoryRule
$ #far:config System.CmdHistoryRule#
    Параметр задаёт поведение выбора истории команд в командной строке, если после Ctrl+E/Ctrl+X нажали Esc:

      0 - Изменять положение в History.
      1 - Не изменять положение в History.

    По умолчанию действует правило 0.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.ConsoleDetachKey
$ #far:config System.ConsoleDetachKey#
    Параметр позволяет задавать сочетание клавиш для отделения консоли Far Manager от не интерактивного процесса, запущенного в ней.
    
    Если в консоли Far'а был запущен длительный процесс, например архивация, и по тем или иным причинам именно 
эта копия Far Manager нужна (редактор в фоне), или нежелательно запускать новый Far, то если у вас установлена 
эта опция, можно создать новую консоль для Far, где он продолжит  работу как если бы запущенный процесс уже 
завершился, а сам процесс продолжит работу в старой консоли.
    
    Например, значение "System.ConsoleDetachKey" равное "CtrlAltX" назначает процессу разделения сочетание клавиш Ctrl+Alt+X.

    По умолчанию значение = "CtrlShiftTab"

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.QuotedSymbols
$ #far:config System.QuotedSymbols#
    Параметр позволяет задавать набор символов, присутствие которых в именах файлов/папок заставит Far Manager заключать такие имена в кавычки.

    Максимум 32 символа.

    По умолчанию значение = " &()[]{}^=;!'+,`" и символ с кодом 0xA0.

    See also ~System.QuotedName~@System.QuotedName@

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.QuotedName
$ #far:config System.QuotedName#
    Имена файлов/папок (содержащие символы, перечисленные в правиле 34) при
вставке в редактор/командную строку или в буфер обмена заключаются в кавычки.
    
    Параметр "System.QuotedName" управляет этим поведением:

    Биты:
      0 - ^<wrap>если установлен, то заключать имена файлов/папок в кавычки при вставке в редактор или командную строку;
      1 - ^<wrap>если установлен, то заключать имена файлов/папок в кавычки при запоминании в буфере обмена.

    По умолчанию установлен нулевой бит.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Interface.AltF9
$ #far:config Interface.AltF9#
    Параметр позволяет выбрать механизм работы комбинации Alt+F9 (Изменение размера экрана) в оконном режиме:

     1 - ^<wrap>использовать усовершенствованный механизм - окно Far Manager
будет переключаться с нормального на максимально доступный размер
консольного окна и обратно. Размер шрифта консольного окна
меняться не будет.
     0 - ^<wrap>использовать механизм, совместимый с Far версии 1.65 и
ниже, т.е. переключение 25/50 линий.

    Данный параметр влияет только на оконный режим работы Far Manager.

    По умолчанию значение = 1

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Dialog.CBoxMaxHeight
$ #far:config Dialog.CBoxMaxHeight#
    Параметр задаёт максимальную высоту открываемого списка истории в диалогах.

    По умолчанию значение = 8

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Editor.UndoDataSize
$ #far:config Editor.UndoDataSize#
    Параметр позволяет ограничить количество памяти, используемой для операций отмены действий в редакторе.

    По умолчанию значение = 104857600 (100MB).

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Editor.CharCodeBase
$ #far:config Editor.CharCodeBase#
    Параметр позволяет менять представление кода символа под курсором в статусной строке в редакторе.

    Может принимать следующие значения:

      0 - восьмеричное значение (6 символов с ведущим нулём)
      1 - десятеричное значение (5 символов)
      2 - шестнадцатеричное значение (4 символа под цифру + символ ''h')

    По умолчанию значение = 1 (десятеричное значение).

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Editor.BSLikeDel
$ #far:config Editor.BSLikeDel#
    Параметр позволяет управлять поведением клавиши BackSpace в редакторе, когда выделен вертикальный блок. 
    
    Если значение отлично от 0, то BS удаляет вертикальный блок подобно клавише Del.

    По умолчанию значение = 1 (BS удаляет помеченный вертикальный блок).

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Editor.AllowEmptySpaceAfterEof
$ #far:config Editor.AllowEmptySpaceAfterEof#
    Окончание файла в редакторе всегда находится внизу экрана, если строк в файле больше чем строк экрана. 
При построчном скроллировании вниз (например, с помощью Ctrl+Down), скроллирование прекращается, когда
показывается последняя строка.

    Параметр "Editor.AllowEmptySpaceAfterEof" позволяет изменить такое поведение редактора.

    Может принимать следующие значения:
    
    0 - прекратить скроллинг, если последняя строка внизу экрана
    1 - продолжать скроллинг, при этом:
        a) поместить курсор за пределы файла по прежнему нельзя
        b) скроллинг с помощью Ctrl+Down сдвинет текст до курсора

    По умолчанию значение = 0 (прекратить скроллинг).

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Interface.RedrawTimeout
$ #far:config Interface.RedrawTimeout#
    Параметр  "Interface.RedrawTimeout" позволяет контролировать время обновления (в мс) 
сообщения в процессе копирования файлов, применения прав доступа после перемещения файлов или папок, 
удаления и поиска файлов, сканирование файловой системы.

    Чем больше значение "Interface.RedrawTimeout", тем реже выводится информацию о процессе и тем быстрее проходит этот самый процесс.

    По умолчанию значение = 200 мс.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@TitleFormat
$ #far:config Interface.ViewerTitleFormat, Interface.EditorTitleFormat#
    Параметры "Interface.ViewerTitleFormat" и "Interface.EditorTitleFormat" позволяют задавать 
формат заголовка консольного окна для ~редактора~@Editor@ и ~программы просмотра~@Viewer@.

    Допускаются шаблонные символы "%File" - имя файла, "%Lng" - строка из lng-файла ("edit" или "view")

    Кроме этого есть шаблон "Interface.TitleAddons", который добавляется в конец заголовка (задаётся в диалоге ~Настройка интерфейса~@InterfSettings@).

    По умолчанию заголовок содержит слово "редактор" (в зависимости от языка интерфейса) и "имя файла" (шаблон "%Lng %File").

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.WipeSymbol
$ #far:config System.WipeSymbol#
    Параметр позволяет задать код символа-заполнителя для операции "~Уничтожить файл~@DeleteFile@" (Alt+Del). 
Использует младший байт параметра.
    If parameter is set to -1, random values will be used.

    По умолчанию значение = 0.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.FlagPosixSemantics
$ #far:config System.FlagPosixSemantics#
    Параметр "System.FlagPosixSemantics" задаёт поведение для процесса добавления отредактированного или просмотренного файла в историю.
Если значение параметра равно true, то при поиске дублей учитывается регистр символов в именах файлов.

    По умолчанию значение = true.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.ShowCheckingFile
$ #far:config System.ShowCheckingFile#
    Параметр "System.ShowCheckingFile" позволяет отображать в заголовке окна Far Manager имя плагина, 
претендующего на файл, который хотим запустить или отобразить в качестве файловой панели.

    По умолчанию значение = false - не отображать информацию.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.PluginMaxReadData
$ #far:config System.PluginMaxReadData#
    Параметр "System.PluginMaxReadData" позволяет задавать максимальный размер читаемых данных из 
файла в который попытались войти из панелей (Enter или Ctrl+PgDn). Считанные данные будут переданы
плагинам для определения плагина поддерживающего файл этого типа.

    Минимальное значение - 0x1000. Максимальное - 0xFFFFFFFF.
 
    Не рекомендуется выставлять значение этого параметра больше 5 Mb.
 
    По умолчанию значение = 0x20000.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.SetAttrFolderRules
$ #far:config System.SetAttrFolderRules#
    Параметр "System.SetAttrFolderRules" позволяет задавать значение по умолчанию опции "Process subfolders"
в диалоге установки атрибутов для одиночной папки:

     true  - ^<wrap>опция "Process subfolders" выключена, файловые дата и время установлены.
     false - ^<wrap>опция "Process subfolders" включена, файловые дата и время очищены.
    
    По умолчанию значение = true.
 
    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.CopyBufferSize
$ #far:config System.CopyBufferSize#
    Параметр "System.CopyBufferSize" задаёт размер буфера, когда не используется
~системная функция копирования~@SystemSettings@. Если параметр равен 0, то используется 
размер по умолчанию - 32768 байт.

    По умолчанию значение равно 0.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.SubstNameRule
$ #far:config System.SubstNameRule#
    Параметр "System.SubstNameRule" задаёт правило опроса приводов на предмет сканирования SUBST-дисков.
 
    Биты:
     0 - если установлен, то опрашивать сменные диски
     1 - если установлен, то опрашивать все остальные
 
    По умолчанию значение = 2 - опрашивать все диски кроме сменных.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.SubstPluginPrefix
$ #far:config System.SubstPluginPrefix#
    Параметр "System.SubstPluginPrefix" позволяет управлять подстановкой префиксов плагинов в операциях 
вставки пути к объекту (Ctrl+F, Ctrl+[...), находящемуся на панели плагина. Если значение равно #true#, то 
Far Manager автоматически добавит в командную строку префикс плагина перед вставляемым путём (кроме панелей, 
которые указывают на реальные файлы, например, "Временная панель"). Значение #false# не добавляет префиксы.
 
    По умолчанию значение = false.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.CopySecurityOptions
$ #far:config System.CopySecurityOptions#
    Параметр "System.CopySecurityOptions" позволяет управлять поведением опции "Права доступа" в диалоге копирования/перемещения.
   
    Номера битов:
     0 и 1 - ^<wrap>Диалог Move: по умолчанию выставлять опцию копирования (бит 0 выставлен в 1, 
бит 1 сброшен в 0) или наследования (биты 0 и 1 выставлены в 1) прав доступа;
     2     - ^<wrap>Диалог Move: запоминать состояние опции до конца сеанса работы Far Manager;
     3 и 4 - ^<wrap>Диалог Copy: по умолчанию выставлять опцию копирования (бит 3 выставлен в 1, бит 4 
сброшен в 0) или наследования (биты 3 и 4 выставлены в 1) прав доступа;
     5     - ^<wrap>Диалог Copy: запоминать состояние опции до конца сеанса работы Far Manager.

    Воздействие битов 0 и 1 зависит от состояния бита 2:
    1. ^<wrap>Если бит 2 выставлен в 1, то опция "Права доступа" будет установлена в зависимости от 
битов 0 и 1 только при первом вызове диалога перемещения после запуска Far Manager. Если вы 
переключите эту опцию в диалоге вручную, то при следующем вызове диалог предложит значение
опции, выбранное вами в прошлый раз. Значение этой опции запоминается только до конца сеанса 
работы Far Manager. При следующем запуске Far опция снова будет установлена в зависимости от битов 0 и 1.
    2. ^<wrap>Если бит 2 сброшен в 0, то опция "Права доступа" будет установлена в зависимости от 
битов 0 и 1 всякий раз при вызове диалога перемещения. Вы можете переключать эту опцию в диалоге 
вручную, но выбранное вами значение будет действовать только на текущую операцию переноса файлов.

    Аналогично, для операции копирования воздействие битов 3 и 4 зависит от состояния бита 5.

    Примеры:
      0x21 - ^<wrap>для операции перемещения опцию "Права доступа" выставлять всегда в "Копировать", 
для операции копирования выставлять опцию в значение по умолчанию и запоминать её значение до конца сеанса работы Far Manager.
      0xС0 - ^<wrap>для перемещения запоминать значение опции до конца сеанса работы Far (при первом вызове диалога опция 
выставлена в значение по умолчанию), для операции копирования опцию "Права доступа" выставлять всегда в "Копировать".

    По умолчанию значение параметра = 0 (опция "Права доступа" устанавливается в значение по 
умолчанию и до конца сеанса работы не запоминается).

    Примечание:

    Параметр "System.CopySecurityOptions" не влияет на создание связей (Alt+F6). В этом случае 
права всегда копируются.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Interface.CursorSize
$ #far:config Interface.CursorSizeX#
    Параметры "Interface.CursorSize1" и "Interface.CursorSize2" позволяют задавать размер курсора в оконном и полноэкранном режимах Far Manager для режима вставки.
   
    Параметры "Interface.CursorSize3" и "Interface.CursorSize4" позволяют задавать размер курсора в оконном и полноэкранном режимах Far Manager для режима замены.

    Значения параметров: число между 1 и 100 - процент символьной ячейки, который заполняется курсором. 
Курсор изменяется от полностью заполненной ячейки до горизонтальной строки внизу ячейки.

    Значения параметров равные нулю позволяют использовать системные настройки консоли.

    По умолчанию:
       Interface.CursorSize1 = 15
       Interface.CursorSize2 = 10
       Interface.CursorSize3 = 99
       Interface.CursorSize4 = 99

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.WordDiv
$ #far:config System.WordDiv#
    Параметр "System.WordDiv" позволяет задавать дополнительные (кроме пробела и табуляции) cимволы-разделители слов.

    По умолчанию: #~~!%^&*()+|{}:"<>?`-=\\[];',./#

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@XLat.WordDivForXlat
$ #far:config XLat.WordDivForXlat#
    Параметр "XLat.WordDivForXlat" позволяет задавать используемые в функции транслитерации (XLat, 
для преобразования текущего слова без выделения) символы-разделители слов.

    По умолчанию: пробел табуляция и символы #!##$%^&*()+|=\\/@?#

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Editor.ReadOnlyLock
$ #far:config Editor.ReadOnlyLock#
    Параметр "Editor.ReadOnlyLock" задаёт поведение редактора при открытии файлов с атрибутами ReadOnly, Hidden и System.
 
    Биты:
     0 - Блокировать редактирование файла с атрибутом R/O
     1 - Предупреждать при открытии файла с атрибутом R/O
     2 - не используется
     3 - не используется
     4 - не используется
     5 - применять дополнительно для файлов с атрибутом Hidden
     6 - применять дополнительно для файлов с атрибутом System

    Значения в битах 0 и 1 соответствуют опциям в диалоге ~настроек редактора~@EditorSettings@  ("Блокировать 
редактирование файла с атрибутом R/O" и "Предупреждать при открытии файла с атрибутом R/O").

    Например, значение 0x43 - предупреждать и блокировать изменения в файлах с атрибутами ReadOnly и System.

    По умолчанию значение = 0.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Editor.FileSizeLimit
$ #far:config Editor.FileSizeLimit#
    Параметр "Editor.FileSizeLimit" задаёт максимальный размер редактируемого файла в байтах. 
Если размер редактируемого файла превышает максимально допустимый, то будет показано предупреждающее сообщение перед 
открытием такого файла.

    По умолчанию значение = 0 (отключает проверку и вывод сообщения)

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.MsWheelDelta
$ #far:config System.MsWheelDelta* & System.MsHWheelDelta*#
    Параметры "System.MsWheelDelta*" и "System.MsHWheelDelta*" позволяют менять смещения для
прокрутки колесом мыши по вертикали и горизонтали.

    Параметры для вертикальной прокрутки:

    System.MsWheelDeltaView  - в программе просмотра
    System.MsWheelDeltaEdit  - во встроенном редакторе
    System.MsWheelDeltaHelp  - в системе помощи
    System.MsWheelDelta      - в прочих областях

    Параметры для горизонтальной прокрутки (Windows Vista и выше):

    System.MsHWheelDeltaView - в программе просмотра
    System.MsHWheelDeltaEdit - во встроенном редакторе
    System.MsHWheelDeltaHelp - в системе помощи
    System.MsHWheelDelta     - в прочих областях

    По умолчанию значение = 1

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@System.CopyTimeRule
$ #far:config System.CopyTimeRule#
    Параметр "System.CopyTimeRule" задаёт режим отображения вывода информации 
о средней скорости копирования, времени копирования и примерном времени до конца операции в диалоге копирования.
 
    Номера битов:
     0 - ^<wrap>если установлен, то показывать при копировании в NUL.
     1 - ^<wrap>если установлен, то показывать при обычных операциях копирования.

    Так как эта функция требует времени для сбора статистики, то на небольших файлах при выключенном
"общем индикаторе копирования" Вы можете ничего не увидеть.

    Параметр доступен в ~Настройках интерфейса~@InterfSettings@, но в диалоге можно выставить только два значения 
- показывать информацию везде или отключить режим отображения.

    Примеры:
     1 - ^<wrap>показывать информацию только при копировании в NUL
     2 - ^<wrap>показывать информацию при обычных операциях копирования.
     3 - ^<wrap>всегда показывать информацию о времени и скорости.

    По умолчанию значение = 3 (всегда показывать информацию о времени и скорости)

    Изменение этого параметра возможно через ~far:config~@FarConfig@ или в ~Настройках интерфейса~@InterfSettings@

@Policies.ShowHiddenDrives
$ #far:config Policies.ShowHiddenDrives#
    Параметр "Policies.ShowHiddenDrives" позволяет позволяет наследовать свойства Windows 
по сокрытию логических дисков из системы ("Hide  Drives in My Computer").

    Значение:

     false - ^<wrap>Far показывает только доступные диски (учитывается значение параметра 
"NoDrives" системной политики - [HKLM или HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer]).
Если "NoDrives" из HKLM=0 (не показывать скрытые диски для всех пользователей), 
то значение этого параметра из HKCU не имеет никакого эффекта.

     true  - ^<wrap>функция отключена, Far показывает все диски, независимо от
значения параметра "NoDrives" в реестре.

    По умолчанию значение = true

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Editor.KeepEditorEOL
$ #far:config Editor.KeepEditorEOL#
    Параметр "Editor.KeepEditorEOL" позволяет управлять типом перевода строк для текста
вставляемого в редактор из буфера обмена.

     false - ^<wrap>текст вставляется без изменений, т.е. берётся тип перевода строк источника (при этом можно получить текст
с разными типами перевода строк).

     true  - ^<wrap>если файл не пуст, сохраняется текущий тип перевода строк редактируемого файла, 
в противном случае параметр ни на что не влияет, т.е. будет использован стиль переводов строк оригинала.

    По умолчанию значение = true

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Editor.AddUnicodeBOM
$ #far:config Editor.AddUnicodeBOM#
    Параметр "Editor.AddUnicodeBOM" указывает надо ли добавлять BOM (Byte Order Mark) в начало создаваемых
редактором файлов в юникодной кодировке (1200, 1201, 65001).

     false - ^<wrap>BOM не добавляется.

     true  - ^<wrap>BOM добавляется.

    По умолчанию значение = true

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Editor.NewFileUnixEOL
$ #far:config Editor.NewFileUnixEOL#
    Параметр "Editor.NewFileUnixEOL" позволяет задавать умолчательные символы конца строки в редакторе
для вновь создаваемых файлов.

     false - ^<wrap>Во вновь создаваемых файлах строки заканчиваются символами #<CR><LF># - (как в Windows).

     true  - ^<wrap>Во вновь создаваемых файлах строки заканчиваются символом #<LF># - (как в Unix).

    По умолчанию значение = false

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Panel.ShortcutAlwaysChdir
$ #far:config Panel.ShortcutAlwaysChdir#
    Параметр "Panel.ShortcutAlwaysChdir" управляет поведением при нажатии клавиш перехода на папку
#RCtrl+0# .. #RCtrl+9#, когда панели невидимы.

     false - ^<wrap>нажатия передаются редактору командной строки, что приводит к появлению имени папки,
связанной с нажатой клавишей, в командной строке.

     true  - ^<wrap>всегда осуществляется переход на папку, связанную с нажатой клавишей (если задана),
даже если панели невидимы.

    По умолчанию значение = false

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Macros.ShowPlayIndicator
$ #far:config Macros.ShowPlayIndicator#
    Параметр "Macros.ShowPlayIndicator" позволяет включать или отключать отображение в левом верхнем углу экрана символа '\2FP\-'
во время воспроизведения макропоследовательности.

     false - ^<wrap>Символ не отображается

     true  - ^<wrap>Символ отображается

    По умолчанию значение = true

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Viewer.SearchWrapStop
$ #far:config Viewer.SearchWrapStop#
    Параметр "Viewer.SearchWrapStop" позволяет изменять поведение при прохождении через начало/конец
файла при поиске в просмотрщике.

     False - ^<wrap>Тихий переход через начало/конец файла

     True  - ^<wrap>Показывается сообщение о переходе через начало/конец файла

     Other - ^<wrap>Сообщение выводится при переходе через точку начала поиска (пройден весь файл)

    По умолчанию значение = True

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@XLat.Layouts
$ #far:config XLat.Layouts#
    Параметр "XLat.Layouts" позволяет задавать номера раскладок клавиатуры (через ';'), которые будут переключаться,
независимо от количества установленных в системе раскладок.

    Например, "04090409;04190419" (или "0409;0419").

    Если указано меньше двух, то механизм "отключается" и раскладки переключаются по кругу.

    Far для "Layouts" считывает первые 10 значений, остальные, если есть, игнорируются.

    По умолчанию значение отсутствует.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@XLat.Flags
$ #far:config XLat.Flags#
    Параметр "XLat.Flags" определяет поведение функции Xlat:

    Биты:
       0 - ^<wrap>Автоматическое переключение раскладки клавиатуры после перекодирования
           ^<wrap>Переключение происходит по кругу: RU->EN->RU->...
           ^<wrap>В семействе Windows NT позволяет переключить раскладку клавиатуры на следующую доступную (см. также описание бита 2).
       1 - ^<wrap>При переключении раскладки выдать звуковой сигнал.
       2 - ^<wrap>Использовать предопределённые именованные правила для "двойных" клавиш.
           ^<wrap>Также, если задано автоматическое переключение, то переключение раскладок происходит только 
по списку значений, перечисленных в ~XLat.Layouts~@XLat.Layouts@, независимо от количества установленных в системе раскладок.
           ^<wrap>Пример см. в Addons\XLat\Russian\Qwerty.farconfig (name="00000409" и name="00000419")
Такие правила возможно поменять только из ~командной строки~@CmdLine@ (параметр /import)
      16 - ^<wrap>Конвертировать всю командную строку при отсутствии выделенного блока.

    По умолчанию значение = 0x00010001 (переключить раскладку и конвертировать всю командную строку при отсутствии выделенного блока)

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@XLat.Tables
$ #far:config XLat.Tables#
    Параметры "XLat.Table*" и "XLat.Rules*" задают перекодировочные таблицы и особые правила конвертации строк.

    Перекодировочная таблица #XLat.Table1# содержит набор символов национальной кодировки.
    
    Перекодировочная таблица #XLat.Table2# содержит набор латинских символов, соответствующих символам национальной кодировки на клавиатуре (см. свою клавиатуру).

    Значение #XLat.Rules1# содержит пары правил для случая "если предыдущий символ русский". Первый символ - что меняем, второй - на что меняем. Допускается 

    Значение #XLat.Rules2# содержит пары правил для случая "если предыдущий символ латинский". Первый символ - что меняем, второй - на что меняем.

    Значение #XLat.Rules3# содержит пары правил для случая "если предыдущий символ не буква". Первый символ - что меняем, второй - на что меняем - крутим по кругу.

    По умолчанию параметры не содержат значений. Если в системе установлена русская раскладка (0x0419) и значение параметра XLat.Table1 
пусто, то Far Manager выставляет следующие умолчания:
    Table1=№АВГДЕЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЯавгдезийклмнопрстуфхцчшщъыьэяёЁБЮ
    Table2=##FDULTPBQRKVYJGHCNEA{WXIO}SMZfdultpbqrkvyjghcnea[wxio]sm'z`~~<>
    Rules1=,??&./б,ю.:^Ж:ж;;$"@@Э"
    Rules2=?,&?/.,б.ю^::Ж;ж$;@@""Э
    Rules3=^::ЖЖ^$;;жж$@""ЭЭ@@&??,,бб&/..юю/

    Изменить эти параметры можно через ~far:config~@FarConfig@

@Interface.DelHighlightSelected
$ #far:config Interface.DelHighlightSelected#
$ #far:config Interface.DelShowSelected#

Interface.DelHighlightSelected -- bool, default = true.
true -- выделяем случай, когда список удаляемых объектов отличаеся от объекта под курсором.

Interface.DelShowSelected -- int, default = 10.
При множественном удалении показываем имена удаляемых объектов. Не более чем заданное число,
приведённое к диапазону 1..высота_окна/2

Старое поведение = (false, 1)

    Изменить эти параметры можно через ~far:config~@FarConfig@

@History.Config
$ #far:config History.*#
    Данный блок параметров позволяет ограничить размеры списков и время жизни элементов следующих историй:
  
    - история ~команд~@History@ в командной строке:
      #History.CommandHistory.Count#
      #History.CommandHistory.Lifetime#

    - история строк ввода в диалогах:
      #History.DialogHistory.Count#
      #History.DialogHistory.Lifetime#
    
    - история ~посещения папок~@HistoryFolders@:
      #History.FolderHistory.Count#
      #History.FolderHistory.Lifetime#

    - история ~просмотра и редактирования~@HistoryViews@:
      #History.ViewEditHistory.Count#
      #History.ViewEditHistory.Lifetime#

    По умолчанию:
      максимальный размер списка (.Count) = 1000 элементов
      время жизни элемента (.Lifetime) = 90 дней

    Изменить эти параметры можно через ~far:config~@FarConfig@

@Editor.F8CPs
$ #far:config Editor.F8CPs#
$ #far:config Viewer.F8CPs#
    Строка позволяющая задавать список кодовых страниц используемых при переключении
кодировки клавишей #F8# в редакторе или просмотрщике.

    Умолчательное значение - #""#, в этом случае используются только кодовые
страницы ANSI и OEM.

    Если задать строку #"-1"#, то кроме ANSI и OEM в список переключения добавляется
умолчательная кодовая страница редактора/просмотрщика (если отличается).

    В противном случае строка должна быть списком номеров кодовых страниц.
Кроме номеров можно использовать также имена - ANSI/OEM/UTF8/DEFAULT.
Дубликаты и неподдерживаемые кодовые страницы удаляются.
Пример: #"ANSI,OEM,65001"#.

    Изменение этого параметра возможно через ~far:config~@FarConfig@

@Panel.Tree.TurnOffCompletely
$ #far:config Panel.Tree.TurnOffCompletely#
    If “true”, all folder tree operations are unavailable:

  - ~Tree panel~@TreePanel@ mode in ~left and right menus~@LeftRightMenu@,
as well as toggle tree panel shortcut key #Ctrl+T#.

  - ~Find folder~@FindFolder@ panel command (#Alt+F10#).

  - Folder tree operations in ~copy, move and rename~@CopyFiles@
dialog (#F10# #Alt+F10# #Shift+F10#).

    Also, folder tree cache files, even if already exist, are not updated
during folder create / delete / rename operations.

    Default value: True

    This parameter can be changed via ~far:config~@FarConfig@ only.

@Index
$ #Index help file#
<%INDEX%>
