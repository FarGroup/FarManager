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
$^#File and archive manager#
`$^#'FULLVERSION`#'
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

 ~Panels~@Panels@:
  ~File panel~@FilePanel@
  ~Tree panel~@TreePanel@
  ~Info panel~@InfoPanel@
  ~Quick view panel~@QViewPanel@
  ~Drag and drop files~@DragAndDrop@
  ~Customizing file panel view modes~@PanelViewModes@
  ~Selecting files~@SelectFiles@

 ~Menus~@Menus@:
  ~Left and right menus~@LeftRightMenu@
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
 ~Sort groups~@SortGroups@
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
$ #Far: how to use help#
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
$ #Far: about#
 #Far# is a text mode file and archive manager for Windows. It supports
#long file names# and provides a wide set of file and folder operations.

 #Far# is #freeware# and #open source# software distributed under the
revised BSD ~license~@License@.

 #Far# does transparent #archive# processing. Files in the archive are
handled similarly as in a folder: when you operate with the archive, Far
transforms your commands into the corresponding external archiver calls.

 #Far# offers a number of service functions as well.


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
$ #Far: command line switches#
 The following switches can be used in the command line:

 #-e[<line>[:<pos>]] <filename>#
 Edit the specified file. After -e you can optionally specify editor start line
and line position.
 For example: far -e70:2 readme.

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

 #-u <username>#
 Allows to have separate registry settings for different users.
 Affects only 1.x Far Manager plugins
 For example: far -u guest

 Far Manager will set the ~environment variable~@FAREnv@ "FARUSER" to the value <username>.

 #-v <filename>#
 View the specified file. If <filename> is `#-#', data is read from the stdin.

 For example, "dir|far -v -" will view dir command output.

 If the input stream is empty when using ‘-’ (for example, you have not specified
the "dir" command in the provided example), Far will wait for the end of data
in the input stream until you press Ctrl+Break.

 #-w[-]#
 Stretch to console window instead of console buffer or vice versa.

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
$ #Keyboard reference#

 ~Panel control~@PanelCmd@

 ~Command line~@CmdLineCmd@

 ~File management and service commands~@FuncCmd@

 ~Mouse: wheel support~@MsWheel@

 ~Menu control commands~@MenuCmd@

 ~Screen grabber~@Grabber@

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
 Deselect files with the same name as the current file #Alt+<Gray ->#
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

 Create a ~folder shortcut~@FolderShortcuts@                            #Ctrl+Shift+0…9#
 Jump to a folder shortcut                            #RightCtrl+0…9#

 If the active panel is a ~quick view panel~@QViewPanel@, a ~tree panel~@TreePanel@ or
an ~information panel~@InfoPanel@, the directory is changed not on the
active, but on the passive panel.

 #Ctrl+Ins#
 Copy names of the selected files to clipboard (if the command line is empty).

 #Ctrl+Shift+Ins#
 Copy names of the selected files to clipboard.

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

 Notes:

 1. ^<wrap>If "Allow reverse sort modes" option in ~Panel settings~@PanelSettings@
dialog is enabled, pressing the same sort key second time toggles the sort direction
from ascending to descending and vice versa;

 2. ^<wrap>If #Alt+Left# and #Alt+Right# combinations, used to scroll long names
and descriptions, work only with non-numpad #Left# and #Right# keys. This is due to
the fact that when #Alt# is pressed, numpad cursor keys are used to enter characters
via their decimal codes.

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
‘*’ and ‘?’.

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
 Context help                                                    #F1#

 Show ~user menu~@UserMenu@                                                  #F2#

 View                                    #Ctrl+Shift+F3, Numpad5, F3#
 If pressed on a file, #Numpad5# and #F3# invoke ~internal~@Viewer@,
external or ~associated~@FileAssoc@ viewer, depending upon the file type and
~Viewer settings~@ViewerSettings@.
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
specified in the ~Viewer settings~@ViewerSettings@ or the ~associated viewer program~@FileAssoc@
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

 Display ~file view and edit history~@HistoryViews@                         #Alt+F11#

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
 - ^<wrap>if #d:\folder2\# exists, contents of #c:\folder1\# is
moved into #d:\folder2\folder1\#;
 - ^<wrap>otherwise contents of #c:\folder1\# is moved into the
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

 Change folder, enter an archive                  #Ctrl+[Shift+]PgDn#
 (including an SFX archive)
 If the cursor points to a directory, pressing #Ctrl+PgDn# changes to that
directory. If the cursor points to a file, then, depending on the file type,
an ~associated command~@FileAssoc@ is executed or the archive is opened.
#Ctrl+Shift+PgDn# always opens the archive, regardless of the associated
command configuration.

 Change to the parent folder                              #Ctrl+PgUp#
 The behavior in root folders depends on "Use Ctrl+PgUp to change drive~@InterfSettings@" option.

 Create shortcut to the current folder              #Ctrl+Shift+0…9#

 Use folder shortcut                                 #RightCtrl+0…9#

 Set ~file attributes~@FileAttrDlg@                                         #Ctrl+A#

 ~Apply command~@ApplyCmd@ to selected files                             #Ctrl+G#

 ~Describe~@FileDiz@ selected files                                     #Ctrl+Z#


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
$ #Screen grabber#
 Screen grabber                                             #Alt+Ins#

 Screen grabber allows to select and copy to the clipboard any screen area.

 To switch between stream and block selection mode use the #Space# key.
 To move the cursor use #arrow# keys or click the #left mouse button#.
 To select text use #Shift+arrow# keys or drag the mouse while holding the #left mouse button#.
 To extend or shrink selected area use #Alt+Shift+arrow# keys.
 To move selected area use #Alt+arrow# keys.
 #Enter#, #Ctrl+Ins#, #right mouse button# or #doubleclick# copy
selected text to the clipboard, #Ctrl+<Gray +># appends it to the clipboard contents.
 #Esc# leaves the grabbing mode.
 #Ctrl+A# - select whole screen.
 #Ctrl+U# - deselect block.
 #Ctrl+Shift+Left# and #Ctrl+Shift+Right# - extend or shrink selection by 10 characters left or right.
 #Ctrl+Shift+Up# and #Ctrl+Shift+Down# - extend or shrink selection by 5 lines up or down.


@MiscCmd
$ #Common control commands#
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

 Delete the current item in a dialog edit line history    #Shift+Del#
 (if it is not locked)

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

 If the plugin has no help file, then context-dependent help will not pop out.

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
$ #Panels#
 Normally Far shows two panels (left and right windows), with different
information. If you want to change the type of information displayed in the
panel, use the ~panel menu~@LeftRightMenu@ or corresponding ~keyboard commands~@KeyRef@.

 See also the following topics for more information:

 ~File panel~@FilePanel@
 ~Tree panel~@TreePanel@
 ~Info panel~@InfoPanel@
 ~Quick view panel~@QViewPanel@

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

 #Detailed#      ^<wrap>File names, sizes, allocation sizes, last write,
creation, access time and attributes are displayed. Fullscreen mode.

 #Descriptions#  File names and ~file descriptions~@FileDiz@

 #Long#          ^<wrap>File names, sizes and descriptions. Fullscreen mode.
 #descriptions#

 #File owners#   File names, sizes and owners.

 #File links#    File names, sizes and number of hard links.

 #Alternative#   ^<wrap>File name, size (formatted with commas) and date of the file are displayed.
 #full#

 You can ~customize file panel view modes~@PanelViewModes@.

 File owners and number of hard links have meaning for NTFS only. Some
file systems may not support file creation and access dates.

 If you wish to change the panel view mode, choose it from the
~panel menu~@LeftRightMenu@. After the mode change or drive change action,
if the initial panel type differs it will be automatically set to the file panel.

 ~Fast find~@FastFind@ action can be used to point to the required file
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

 You can find a folder quickly with the help of #fast find# action. Hold
the Alt key and type the folder name until you point to the right folder.
Pressing #Ctrl+Enter# keys simultaneously will select the next match.

 #Gray +# and #Gray -# keys move up and down the tree to the next branch
on the same level.

 See also the list of ~macro keys~@KeyMacroTreeList@, available in the folder tree panel.


@InfoPanel
$ #Panels: info panel#
 The information panel contains the following data:

 1. ^<wrap>#network# names of the computer and the current user (see ~Info panel settings~@InfoPanelSettings@);

 2. ^<wrap>name and type of the #current disk#, type of the file system, network
name, total and free space, disk volume label and serial number;

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
    ^<wrap>#memory# load percentage (100% means all of available memory is used),
size of the installed physical memory (in Vista and newer), total and free size of the physical
memory (available for Windows), virtual memory and paging file;

 4. ^<wrap>#folder description# file
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
 - ^<wrap>If the folder or its subfolders contain symbolic links and the option
"Scan symbolic links" in the ~System settings~@SystemSettings@ dialog is enabled.
 - ^<wrap>If the folder or its subfolders contain multiple hard links to the same file.

 #Ctrl+Shift+S# toggles size display mode: float with size suffixes or bytes.
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
$ #Menus#
 To choose an action from the menu you can press #F9# or click on top of the
screen.

 When the menu is activated by pressing #F9#, the menu for the active panel
is selected automatically. When the menu is active, pressing #Tab# switches
between the menus for the left and right panel. If the menus "Files",
"Commands" or "Options" are active, pressing #Tab# switches to the menu of the
passive panel.

 Use the #Shift+F10# key combination to select the last used menu command.

 Read the following topics for information about a particular menu:

 ~Left and right menus~@LeftRightMenu@
 ~Files menu~@FilesMenu@
 ~Commands menu~@CmdMenu@
 ~Options menu~@OptMenu@

 See also the list of ~macro keys~@KeyMacroMainMenuList@, available in the main menu.


@LeftRightMenu
$ #Menus: left and right menus#
 The #Left# and #Right# menus allow to change left and right panel settings
respectively. These menus include the following items:

 #Brief#                Display files within three columns.

 #Medium#               Display files within two columns.

 #Full#                 Display file name, size, date and time.

 #Wide#                 Display file name and size.

 #Detailed#             ^<wrap>Display file name, size, allocation size,
last write, creation and access time, attributes. Fullscreen mode.

 #Descriptions#         File name and ~file description~@FileDiz@.

 #Long descriptions#    ^<wrap>File name, size and description. Fullscreen mode.

 #File owners#          File name, size and owner.

 #File links#           ^<wrap>File name, size and number of hard links.

 #Alternative full#     ^<wrap>File name, formatted size and date.

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

 #Wipe#               ^<wrap>Wipe files and folders. Before file deletion
its data are overwritten with zeros, after which the file is truncated and renamed to
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

 #Restore selection#  ^<wrap>~Restore~@SelectFiles@ previous file selection after file
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

 #File view history#    Display ~file view and edit history~@HistoryViews@.

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

 #Edit sort groups#     Allows to edit user defined ~sort groups~@SortGroups@.

 #File panel filter#    Allows to control file panel contents.
                      See ~filters menu~@FiltersMenu@ for the detailed
                      description.

 #Plugin commands#      Show ~plugin commands~@Plugins@ list.

 #Screens list#         Show open ~screens list~@ScrSwitch@.

 #Task list#            Shows ~active tasks list~@TaskList@.

 #Hotplug devices list# Show ~hotplug devices list~@HotPlugList@.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@OptMenu
$ #Menus: options menu#
 #System settings#       Shows ~System settings~@SystemSettings@ dialog.

 #Panel settings#        Shows ~Panel settings~@PanelSettings@ dialog.

 #Tree settings#         Shows ~Tree settings~@TreeSettings@ dialog.
                       Available only if ~Panel.Tree.TurnOffCompletely~@Panel.Tree.TurnOffCompletely@
                       parameter in ~far:config~@FarConfig@ is set to “false.”

 #Interface settings#    Shows ~Interface settings~@InterfSettings@ dialog.

 #Languages#             Select main and help language.
                       Use "Save setup" to save selected languages.

 #Plugins#               Configure ~plugins~@Plugins@.
 #configuration#

 #Plugin manager#        Shows ~Plugin manager settings~@PluginsManagerSettings@ dialog.
 #settings#

 #Dialog settings#       Shows ~Dialog settings~@DialogSettings@ dialog.

 #Menu settings#         Shows ~Menu settings~@VMenuSettings@ dialog.

 #Command line settings# Shows ~Command line settings~@CmdlineSettings@ dialog.

 #AutoComplete settings# Shows ~AutoComplete settings~@AutoCompleteSettings@ dialog.

 #InfoPanel settings#    Shows ~InfoPanel settings~@InfoPanelSettings@ dialog.

 #Groups of file masks#  Shows ~Groups of file masks~@MaskGroupsSettings@ dialog.

 #Confirmations#         Shows dialog to turn on or off ~confirmations~@ConfirmDlg@
                       of some operations.

 #File panel modes#      Shows ~Edit panel modes~@PanelViewModes@ dialog.

 #File descriptions#     ~File descriptions~@FileDiz@ list names and update mode.

 #Folder description#    Specify names (~wildcards~@FileMasks@ are allowed) of
 #files#                 files displayed in the ~Info panel~@InfoPanel@ as folder
                       descriptions.

 #Viewer settings#       External and internal ~Viewer~@ViewerSettings@ setting.

 #Editor settings#       External and internal ~Editor~@EditorSettings@ settings.

 #Code pages#            Shows the ~Code pages~@CodePagesMenu@ menu.

 #Colors#                Allows to select colors for different
                       interface items, to change the entire Far
                       colors palette to black and white or to set
                       the colors to default.

 #Files highlighting#    Shows ~Files highlighting~@Highlight@ and sort groups
 #and sort groups#       dialog.

 #Save setup#            Save current configuration, colors and
                       screen layout.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@ConfirmDlg
$ #Confirmations#
 In the #Confirmations# dialog you can switch on or off confirmations for
following operations:

 - ^<wrap>overwrite destination files when performing file copying;
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
 - ^<wrap>in all non-removable drives;
 - in all local drives, except removable and network;
 - in all folders specified in the %PATH% environment variable (not including subfolders).
 - in all folders from the drive root, in the find dialog one can change disk drive of the search;
 - from the current folder;
 - in the current folder only
 - in selected folders.

 The search parameters are saved in the configuration.

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
depending on file type and ~Viewer settings~@ViewerSettings@.
#Ctrl+Shift+F3# always invokes internal viewer regardless of file associations.

 Edit                                    #F4, Alt+F4, Ctrl+Shift+F4#

 #F4# or #Alt+F4# invokes ~internal~@Editor@, external or ~associated~@FileAssoc@ editor,
depending on file type and ~Editor settings~@EditorSettings@.
#Ctrl+Shift+F4# always invokes internal editor regardless of file associations.

 Viewing and editing is supported for plugin emulated file systems. Note, that saving editor
changes by #F2# key in emulated file system will perform #SaveAs# operation,
instead of common #Save#.


@FindFolder
$ #Find folder#
 This command allows a quick look for the required folder in the folders tree.

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
$ #History#
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

 Show additional information                                      #F3#

 To go to the previous or next command directly from the command line, you
can press #Ctrl+E# or #Ctrl+X# respectively.

 If you want to save the commands history after exiting Far, use the
respective option in the ~System settings~@SystemSettings@ dialog.

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
titles. Note that !?<title>?<init>! symbol can be used to enter additional
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

 It is possible to use digits, letters and function keys (#F1#…#F24#) as
hot keys in user menu. If #F1# or #F4# is used, its original function in user
menu is overridden. However, you still can use #Shift+F4# to edit the menu.

 When you edit or create a menu item, you should enter the hot key for fast
item access, the item title which will be displayed in the menu and the command
sequence to execute when this item will be selected.

 When you edit or create a submenu, you should enter the hot key and the
item title only.

 User menus are stored in text files named #FarMenu.Ini#:
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

 The association can be described in the #Description of the association# field.

 ~Special symbols~@MetaSymbols@ can be used in the associated command.

 Notes:

 1. ^<wrap>If no execute command is associated with file and
#Use Windows registered types# option in ~System settings~@SystemSettings@
dialog is on, Far tries to use Windows association to execute this file type.
 2. ^<wrap>Operating system ~commands~@OSCommands@ "IF EXIST" and "IF DEFINED"
allow to configure "smarter" associations - if you have specified several
associations for a file type, the menu will show only the associations
for which the conditions are true.


@MetaSymbols
$ #Special symbols#
 The following special symbols can be used in ~associated commands~@FileAssoc@,
~user menu~@UserMenu@ and the command ~"Apply command"~@ApplyCmd@:

 #!!#       ^<wrap>The ‘!’ character
 #!#        Long file name without extension
 #!~~#       Short file name without extension
 #!`#       Long extension without file name (ext)
 #!`~~#      Short extension without file name (ext)
 #!.!#      Long file name with extension
 #!-!#      Short file name with extension
 #!+!#      Similar to !-! but if a long file name was lost after performing the command, Far will restore it
 #!@@!#      Name of a file with a list of selected file names
 #!$!#      Name of a file with a list of selected short file names
 #!&#       List of selected files
 #!&~~#      List of selected short file names
 #!:#       Current drive in the format "C:". For remote folders - "\\\\server\\share"
 #!\\#       Current path
 #!/#       Short name of the current path
 #!=\\#      Current path considering ~symbolic links~@HardSymLink@.
 #!=/#      Short name of the current path considering ~symbolic links~@HardSymLink@.
 #!?!#      Description of the current file

 #!?<title>?<init>!#
 This symbol is replaced by user input, when
executing command. <title> and <init> - title and initial text of edit control.
 Several such symbols are allowed in the same line, for example:
 grep !?Search for:?! !?In:?*.*!|c:\\far\\Far.exe -v -

 A history name for the <init> string can be supplied in the <title>. In such case the command has the following format:
#!?$<history>$<title>?<init>!#, for example:
 grep !?#$GrepHist$#Search for:?! !?In:?*.*!|Far.exe -v -

 Leave the name empty to disable history.

 The entered string can also be accessed later as #%<history># (or as #%UserVarN#, where N is the index of the corresponding input).

 In <title> and <init> the usage of other meta-symbols is allowed by enclosing them in brackets, e.g.
grep !?Find in (!.!):?! |Far.exe -v -.

 #!###
 "!##" prefix forces all subsequent special symbols
to refer to the passive panel (see note 4).
For example, !##!.! denotes the name of the current
file on the passive panel.

 #!^#
 "!^" prefix forces all subsequent special symbols
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

 4. ^<wrap>The prefixes "!##" and "!^" work as toggles. The effect
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
$ #Settings dialog: system#
 #Delete to Recycle Bin#
 Enables file deletion via the Recycle Bin.The operation of deleting to the Recycle
Bin can be performed only for local hard disks.

 #Use system copy routine#
 Use the file copy functions provided by the operating system instead of internal
file copy implementation. It can be useful on NTFS, because the system function
(CopyFileEx) copies file extended attributes. On the other hand, when using the system
function, the possibility of "smart" ~copying~@CopyFiles@ of sparse files is not available.

 #Copy files opened for writing#
 Allows to copy files that are opened by other programs for writing. This mode
is handy to copy a file opened for a long time, but it could be dangerous, if a file
is being modified at the same time as copying.

 #Scan symbolic links#
 Scan ~symbolic links~@HardSymLink@ along with normal sub-folders when building the folder tree,
determining the total file size in the sub-folders.

 #Update panels only when Far is active#
 If enabled, file panels will be monitored only when Far is active, i.e. panels will not be updated until Far window is focused.
This allows to avoid blocking the directories opened on panels.
However, sometimes the update is not triggered after receiving focus,
so this option is disabled by default and directories are always monitored.

 #Save commands history#
 Forces saving ~commands history~@History@ before exit and restoring after starting Far.

 #Save folders history#
 Forces saving ~folders history~@HistoryFolders@ before exit and restoring after starting Far.
Folders history list can be activated by #Alt+F12#.

 #Save view and edit history#
 Forces saving ~file view and edit history~@HistoryViews@ before exit and restoring it after
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

 #Auto save setup#
 If checked, Far will save setup automatically. The current folders for both panels will be also saved.


@PanelSettings
$ #Settings dialog: panel#
 #Show hidden and system files#
 Enables displaying files with Hidden or System attributes. This option can also be switched by #Ctrl+H#.

 #Highlight files#
 Enables ~files highlighting~@Highlight@.

 #Select folders#
 Enables folder selection with #Gray +# and #Gray *#. If this option is off, these keys select only files.

 #Right click selects files#
 If this option is on, #right mouse click# selects files. Otherwise, it opens Windows  Explorer Context menu.

 #Sort folder names by extension#
 Applies sort by extension not only to files, but also to folders. When this option is turned on, sorting
by extension works the same as it did in Far 1.65. If the option is turned off, in the extension sort mode the
folders will be sorted by name, as they are in the name sort mode.

 #Allow reverse sort modes#
 If this option is on and the current file panel sort mode is reselected, reverse sort mode will be set.

 #Disable automatic panel update#
 If this option is on and the number of file objects exceeds the specified value,
the automatic panel update when the file system state changes is disabled.
 Auto-update works only for FAT/FAT32/NTFS file systems.
The value of 0 means "update always".
To force an update of the panels press #Ctrl+R#.

 #Network drives autorefresh#
 This option enables panel autorefresh when the file system state of a network drive is changed.
It may be helpful to disable this option on slow network connections.

 #Show column titles#
 Enables displaying ~file panel~@FilePanel@ column titles.

 #Show status line#
 Enables displaying file panel status line.

 #Detect volume mount points#
 Distiguishes between normal directory links (Junctions) and volume mount points.
This option significanty slows down displaying directories on slow network connections.

 #Show files total information#
 Enables displaying the file totals at the bottom line of the file panel.

 #Show free size#
 Enables displaying the current disk free space.

 #Show scrollbar#
 Enables displaying file and ~tree panel~@TreePanel@ scrollbars.

 #Show background screens number#
 Enables displaying the number of ~background screens~@ScrSwitch@.

 #Show sort mode letter#
 Displays current sort mode in the upper left-hand corner of the panel.

 #Show ".." in root folders#
 Enables displaying of ".." item in root folders.
Pressing #Enter# on this item opens ~Change drive menu~@DriveDlg@.


@TreeSettings
$ #Tree settings#
 #Auto change folder#
 If turned on, moving cursor on the ~tree panel~@TreePanel@ will synchronously change the folder on the other panel.
If turned off, to change the folder from the tree panel, you need to press #Enter#.

 #Minimum number of folders#
 The minimal number of folders on the disk for which folder tree cache file #tree3.far# will be created.


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
 #([[T]FFFFFFFF][:[T]BBBBBBBB])#, where:

  #FFFFFFFF#
  Foreground color in aarrggbb format or index in the console palette.

  #BBBBBBBB#
  Background color in aarrggbb format or index in the console palette.

  #T#
  "TrueColor" flag. If absent, value is treated as the console palette index (0-F):

  \00 \11 \22 \33 \44 \55 \66 \77 \88 \99 \AA \BB \CC \DD \EE \FF \-
  0123456789ABCDEF

 If foreground or background color is omitted, the corresponding default value will be used.

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
 #Ctrl+Shift+Right#   Show the rightmost column of all lines currently visible on the screen

 In the #hex# and #dump# ~view modes~@ViewerMode@, #Ctrl+Left# and
#Ctrl+Right# keys shift the content within the window one byte at a time
in the corresponding direction.

 In the #hex# ~view mode~@ViewerMode@, #Alt+Left# and #Alt+Right# key
combinations decrease or increase the number of bytes displayed on each
row by one byte, respectively. #Ctrl+Alt+Left# and #Ctrl+Alt+Right# key
combinations adjust the number of displayed bytes by 16 at a time.

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
 #Alt+F11#            Display ~file view and edit history~@HistoryViews@
 #Gray +#             View the next file on the active file panel
 #Gray -#             View the previous file on the active file panel
 #Ctrl+O#             Show user screen
 #Ctrl+Alt+Shift#     Temporarily show user screen
(while the key combination is held down)
 #Ctrl+B#             Toggle functional key bar at the bottom of the screen
 #Ctrl+Shift+B#       Toggle status line
 #Ctrl+S#             Toggle the scrollbar
 #Alt+BS, Ctrl+Z#     Undo position change
 #RightCtrl+0…9#      Set bookmark 0…9 at the current position
 #Ctrl+Shift+0…9#     Set bookmark 0…9 at the current position
 #LeftCtrl+0…9#       Go to bookmark 0…9
 #Ctrl+Ins, Ctrl+C#   Copy the selected text to the clipboard.
The text can be selected manually or as the result of a ~search~@ViewerSearch@.
 #Ctrl+U#             Unselect the text
 #Shift+Mouse click#  Select text manually. The first mouse click indicates the
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
~Viewer settings~@ViewerSettings@ dialog. The range is between 100 to 100,000,
the default is 10,000. Lines longer than the maximum will be split into
several screen rows even if word wrap mode is turned off.

 4. ^<wrap>Far starts ~searching~@ViewerSearch@ (#F7#) from the
beginning of the currently visible area.

 5. ^<wrap>To auto-scroll a file which is being appended by another
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

 #Text# mode

 In the #text# mode, viewer renders file content interpreting byte
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

 In the #dump# mode, viewer renders file content character by character
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

 In the #dump# mode, there is no notion of a text line. Instead
of horizontal scrolling (cf. #text# #truncate# mode), the text
is shifted one character at a time. The #Ctrl+Right# key combination
shifts all characters to the left; the first character on a row becomes
the last on the previous row. The #Ctrl+Left# key combination shifts all
characters to the right moving the last character of a row to the first
positions of the next row. The text “flows” from row to row. The #Right#
and #Left# keys are ignored.

 #Hex# mode (hexadecimal codes)

 In the #hex# mode, viewer renders file content 16 bytes per screen
row, with the hexadecimal offset of the first byte of each row at the
left, followed by the hexadecimal representation of the bytes, followed
by the character representation.

 The rendition depends on the encoding defined by the current code
page. For single-byte encodings (e.g. all ANSI code pages), the bytes
on each row are represented by 16 double-digit hex values followed by 16
characters. For UTF-8 encoding, the bytes are represented the same way,
while the characters are displayed at the positions of the leading bytes
of the UTF-8 sequences with the positions of continuation bytes being
filled with the #›# characters (code point U+203A). For UTF-16(BE)
encodings the hex values are followed by eight characters. For example:

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

 The #Ctrl+Right# key combination shifts all bytes to the left; the
first byte on a row becomes the last on the previous row. The
#Ctrl+Left# key combination shifts all bytes to the right moving the
last byte of a row to the first positions of the next row. Unlike
in #dump# mode, the content is shifted by a byte, not by a character.

 The #Alt+Right# key combination increases the number of bytes displayed
on each row by one byte. The #Ctrl+Alt+Right# key combination increases
the number of bytes by 16 at a time. The #Alt+Left# key combination
decreases the number of bytes displayed on each row by one byte. The
#Ctrl+Alt+Left# key combination decreases the number of bytes by 16 at
a time.

 The #Right# and #Left# keys are ignored.


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

 #Reverse search#
 Reverse the search direction - search from the end of file towards the beginning.

 #Regular expressions#
 Enable the use of ~regular expressions~@RegExp@ in the search string.
The multiline search is not supported.


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

 Cursor movement

 #Left#                    ^<wrap>Character left
 #Ctrl+S#                  Move the cursor one character to the left, but don't move to the previous line if the line beginning is reached.
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

 #Shift+Cursor keys#       ^<wrap>Select block
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
 #Ctrl+P#                  Copy block to current cursor position (in persistent blocks mode only)
 #Ctrl+M#                  Move block to current cursor position (in persistent blocks mode only)
 #Alt+U#                   Shift block left
 #Alt+I#                   Shift block right

 Other operations

 #F1#                      ^<wrap>Help
 #F2#                      Save file
 #Shift+F2#                ~Save file as…~@FileSaveAs@
 #Shift+F4#                Edit ~new file~@FileOpenCreate@
 #F6#                      Switch to ~viewer~@Viewer@
 #F7#                      ~Search~@EditorSearch@
 #Ctrl+F7#                 ~Replace~@EditorSearch@
 #Shift+F7#                Continue search/replace
 #Alt+F7#                  Continue search/replace in "reverse" mode
 #F8#                      Toggle OEM/ANSI code page
 #Shift+F8#                Select code page
 #Alt+F8#                  ~Go to~@EditorGotoPos@ specified line and column
 #Alt+F9#                  Maximize or restore the size of the Far console window; see also ~Interface.AltF9~@Interface.AltF9@
 #Alt+Shift+F9#            Call ~Editor settings~@EditorSettings@ dialog
 #F10, F4, Esc#            Quit
 #Shift+F10#               Save and quit
 #Ctrl+F10#                Position to the current file
 #F11#                     Call "~Plugin commands~@Plugins@" menu
 #Alt+F11#                 Display ~file view and edit history~@HistoryViews@
 #Alt+BS, Ctrl+Z#          Undo
 #Ctrl+Shift+Z#            Redo
 #Ctrl+L#                  Disable edited text modification
 #Ctrl+O#                  Show user screen
 #Ctrl+Alt+Shift#          Temporarily show user screen (as long as these keys are held down)
 #Ctrl+Q#                  Treat the next key combination as a character code
 #RightCtrl+0…9#           Set a bookmark 0…9 at the current position
 #Ctrl+Shift+0…9#          Set a bookmark 0…9 at the current position
 #LeftCtrl+0…9#            Go to bookmark 0…9
 #Shift+Enter#             Insert the name of the current file on the active panel at the cursor position.
 #Ctrl+Shift+Enter#        Insert the name of the current file on the passive panel at the cursor position.
 #Ctrl+F#                  Insert the full name of the file being edited at the cursor position.
 #Ctrl+B#                  Show/Hide functional key bar at the bottom line.
 #Ctrl+Shift+B#            Show/Hide status line

 See also the list of ~macro keys~@KeyMacroEditList@, available in the editor.

 Notes:

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

 #Reverse search#
 Change the direction of search (from the end of file towards the beginning)

 #Regular expressions#
 Treat input as Perl regular expression (~search~@RegExp@ and ~replace~@RegExpRepl@).
Each line is processed individually, so multi-line expressions and line break characters will not be found.

 ~Preserve style~@PreserveStyle@
 Preserve style (case and delimiters in program source code) of the replaced text.

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

 #Ctrl+Enter#, #Ctrl+Left#, #mouse click#
 Go to the position of the found text.

 #Gray +#
 Add session bookmark with the current position.

 #RightCtrl+0…9#, #Ctrl+Shift+0…9#
 Set bookmark 0…9 at the current position.

 #LeftCtrl+0…9#
 Go to the bookmark 0…9.


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
$ #Change drive#
 This menu allows to change the current drive of a panel or open a new ~plugin~@Plugins@ panel.

 Select the item with the corresponding drive letter to change the drive or
the item with the plugin name to create a new plugin panel. If the panel type
is not a ~file panel~@FilePanel@, it will be changed to the file panel.

 #Ctrl+A#, #F4# hotkeys invoke the ~file attributes~@FileAttrDlg@ for drives.

 #Ctrl+A#, #F4# hotkeys can be used to assign a hotkey to plugin item.

 #F3# key shows plugin technical information.

 #Del# key can be used:
 - ^<wrap>to ~disconnect~@DisconnectDrive@ network drives;
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

 The highlighting groups are checked from top to bottom. If it is detected
that a file belongs to a group, no further groups are checked.

 See also: common ~menu~@MenuCmd@ keyboard commands.


@HighlightEdit
$ #Files highlighting and sort groups: editing#
 The #Files highlighting# dialog in the ~Options menu~@OptMenu@ allows to
define file highlighting groups. Each group definition ~includes~@Filter@:

 - one or more ~file masks~@FileMasks@;

 - attributes to include or exclude:
   #[x]# - ^<wrap>inclusion attribute - file must have this attribute.
   #[ ]# - ^<wrap>exclusion attribute - file must not have this attribute.
   #[?]# - ^<wrap>ignore this attribute;

 - normal name, selected name, name under cursor and
selected name under cursor colors to display file names.
If you wish to use the default color, set color to "Black on black";

 - an optional character to mark files from the group.
   It can be used both with or instead of color highlighting.

 If the option "A file mask or several file masks" is turned off, file masks
will not be analyzed, and only file attributes will be taken into account.

 A file belongs to a highlighting group if:
 - ^<wrap>file mask analysis is enabled and the name of the file matches
at least one file mask (if file mask analysis is disabled,
the file name doesn't matter);
 - it has all of the included attributes;
 - it has none of the excluded attributes.

 The Compressed, Encrypted, Not indexed, Sparse, Temporary and Reparse point
attributes are valid for NTFS drives only. The #Integrity stream# and
#No scrub data# attributes are supported only on ReFS volumes starting from
Windows Server 2012.


@ViewerSettings
$ #Settings dialog: viewer#
 This dialog allows to change the settings of the internal
and external ~viewer~@Viewer@.

@=
^#Viewer#
@=
 #Use external viewer#     Start external viewer on #F3# key
 #for F3 instead of#       and internal viewer on #Alt+F3#
 #Alt+F3#                  key combination.

 #Viewer command#          Command to launch the external viewer.
                         Use ~special symbols~@MetaSymbols@ to specify the
                         name of the file to view.

@=
^#Internal viewer#
@=
 #Persistent selection#    Do not remove block selection after
                         moving the cursor.

 #Search dialog#           Always returns focus to the search text field in
 #auto-focus#              the ~Viewer~@Viewer@ search dialog.

 #Tab size#                Number of spaces per single tab position.

 #Show scrolling arrows#   Show scrolling arrows at the edges of the viewer
                         window if the text does not fit horizontally.

 #Visible '\0'#            Show a printable character instead of space for
                         the character '\0'. The character to display can be
                         set in ~far:config~@FarConfig@ #Viewer.ZeroChar#.

 #Show scrollbar#          Show scrollbar in the internal viewer. This
                         option can also be switched by pressing
                         #Ctrl+S# in the internal viewer.
@=
 #Save file position#      Save and restore positions in the recently
                         viewed files. This option also saves and restores
                         the code page (if it was selected manually) and
                         ~view mode~@ViewerMode@.

 #Save file code page#     Save and restore the code page selected for a file.
                         This is automatically enabled if #Save file position#
                         is enabled, as file position depends on the encoding.

 #Save bookmarks#          Save and restore bookmarks in the recently viewed
                         files. (Bookmarks can be created with #RightCtrl+0…9#
                         or #Ctrl+Shift+0…9# key combinations.)

 #Maximum line width#      Maximum number of columns for text mode viewer.
                         Min=100, Max=100,000, Default=10,000.

 #Save view mode#          Save and restore ~view modes~@ViewerMode@
                         of recently viewed files.

 #Save wrap mode#          Save and restore #wrap# and #word wrap# ~modes~@ViewerMode@
                         of recently viewed files.

 #Detect dump view mode#   If this option is on and Far considers the file binary,
                         the #dump# ~mode~@ViewerMode@ is selected automatically
                         at the first view. Otherwise, the #text# mode is selected.

 #Autodetect#              ~Autodetect~@CodePageAuto@ the code page of
 #code page#               the file being viewed.

 #Default code page#       Allows to select the default code page.
@=

 If the external viewer is assigned to #F3# key, it will be launched only if
the ~associated~@FileAssoc@ viewer for the current file type is not defined.

 Changing of settings does not affect currently opened internal
viewer windows.

 The settings dialog can also be invoked from the ~internal viewer~@Viewer@
by pressing #Alt+Shift+F9#. The changes will come into effect immediately but
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
                         (created with #RightCtrl+0…9# or
                         #Ctrl+Shift+0…9#)

 #Auto indent#             Enables auto indent mode when entering
                         text.

 #Cursor beyond#           Allow moving cursor beyond the end of line.
 #end of line#

 #Tab size#                Number of spaces in a tab character.

 #Show scrollbar#          Show scrollbar.

 #Show white space#        Make while space characters (spaces, tabulations,
                         line breaks) visible.

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
#Offline#, #Reparse point# attributes are available only on NTFS drives.
The #Compressed# and #Encrypted# attributes are mutually exclusive, that is, you can set only
one of them. You cannot clear the #Sparse# attribute in Windows 2000/XP/2003. The
#Integrity stream# and #No scrub data# attributes are supported only on ReFS volumes starting from
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
folders. Press #Ctrl+Shift+0…9#, to create a shortcut to the current folder.
To change to the folder recorded in the shortcut, press #RightCtrl+0…9#. If
#RightCtrl+0…9# pressed in edit line, it inserts the shortcut path into the
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

 Use ANSI code page by default

 Save in UTF8


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

 #Column types# - a comma-separated list. Each column type starts with
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

 N[M[D],O,R[F],N] - file name, where:
                    M - ^<wrap>show selection marks where:
                        D - dynamic selection marks;
                    O - ^<wrap>show names without paths (intended mainly for plugins);
                    R - ^<wrap>right align names that do not fit in column, where:
                        F - right align all names;
                    N - ^<wrap>do not show extensions in name column;

 These modifiers can be used in combination, for example NMR.

 X[R]       - file extension, where:
              R - ^<wrap>right align file extension;

 S[C,T,F,E] - file size
 P[C,T,F,E] - allocation file size
 G[C,T,F,E] - size of file streams, where:
              C - ^<wrap>group digits using the character from Windows settings;
              T - ^<wrap>use decimal units instead of binary,
i.e., to calculate kilobytes, the size will be divided by 1000 instead
of by 1024; in this mode unit character is shown in lower case, e.g. #k#,
#m#, #g# instead of #K#, #M#, #G#;
              F - ^<wrap>show size as a decimal fraction with
no more than three digits before decimal point, e.g. 999 bytes will
be shown as #999#, while 1024 bytes as #1.00 K#; note that the behavior
depends on whether the #T# modifier is used;
              E - ^<wrap>economic mode, no space between the
size and the unit character, e.g. #1.00k#;

 D          - file last write date;
 T          - file last write time;

 DM[B,M]    - file last write date and time;
 DC[B,M]    - file creation date and time;
 DA[B,M]    - file last access date and time;
 DE[B,M]    - file change date and time, where:
              B - brief (Unix style) file time format;
              M - use text month names;

 A          - file attributes;
 Z          - file descriptions;

 O[L]       - file owner, where:
              L - show domain name;

 LN         - number of hard links;

 F          - number of streams.

 If the column types description contains more than one file name column,
the file panel will be displayed in multicolumn form.

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


 #Column widths# - used to change width of panel columns.
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

 #Status line column types# and #Status line column widths# -
similar to "Column types" and "Column widths", but for panel status line.

 #Fullscreen view# - force fullscreen view instead of half-screen.

 #Align file extensions# - show file extensions aligned.

 #Align folder extensions# - show folder extensions aligned.

 #Show folders in uppercase# - display all folder names in upper
case, ignoring the original case.

 #Show files in lowercase# - display all file names in lower case,
ignoring the original case.

 #Show uppercase file names in lowercase# - display all uppercase
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
folder, or group of items. Masks can contain common valid file name characters,
wildcards (‘*’ and ‘?’) and special expressions:

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
or deselected. Otherwise, the selection on the folders is changed as well.


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
#d:\folder2\folder1\#. Otherwise, the contents of #c:\folder1\# will be moved into the newly
created #d:\folder2\#.

 If the option “#Process multiple destinations#” is enabled, you can specify
multiple copy or move targets on the input line. The targets should be separated
with character “#;#” or “#,#”. If a target name contains these characters,
enclose it in double quotes.

 If you want to create the destination folder before copying,
append backslash to its name.

 If ~Panel.Tree.TurnOffCompletely~@Panel.Tree.TurnOffCompletely@
parameter in ~far:config~@FarConfig@ is set to “false,” you can use
~Find folder~@FindFolder@ dialog to select the target path.
 The following shortcuts open the dialog with different pre-selected folders:
 - ^<wrap>#F10# selects the folder from the active panel.
 - ^<wrap>#Alt+F10# selects the folder from the passive panel.
 - ^<wrap>#Shift+F10# selects the specified target folder. If several
paths are entered on the input line, only the first one is used.

 If the option “#Process multiple destinations#” is enabled, the folder
selected in the tree is appended to the input line.

 Whether copying, moving or renaming files works for a plugin depends
upon the plugin functionality.

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

 The “#Use system copy routine#” option of the ~System settings~@SystemSettings@
dialog enables the use of Windows operating system function CopyFileEx. This may be useful
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
environment “variable” is defined:
 #if exist file1 if not exist file2 if defined variable command#

 #PUSHD path#
 Stores the current path for use by the “POPD” command, then changes
the current path on the active panel to the specified “path”.

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
 #s# - ^<wrap>consider the whole text as one line, ‘.’ matches any character;
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

 #(?{name}pattern)# - group with a name. The name can be empty (in such case you
cannot refer to this group) or must contain only word characters (#\w#) and spaces (#\s#).

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
Each parentheses except (?:pattern), (?=pattern), (?!pattern), (?<=pattern), (?<!pattern) and (?{name}pattern)
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
Shift and a period pressed together). Pressing the first
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
If current value of an option is other than the default, the option is marked with the ‘*’ character to the left of the name.

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
on small files, UCD annoyingly chooses wrong code pages.

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

 Ready-made settings for CMD.EXE, COMMAND.COM, and other well-known
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


@System.FlagPosixSemantics
$ #far:config System.FlagPosixSemantics#
 This Boolean parameter specifies whether inserting files into
~view and edit history~@HistoryViews@ is case sensitive.

 If a file being added already exists in the history, it is not inserted
again; instead, the existing history entry is moved to the most recent
position.

 False - ^<wrap>Case insensitive comparison is used to search the
history for duplicates.
 True  - Case sensitive comparison is used to search the history for
duplicates.

 Default value: True (the search is case sensitive).

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

 Default value: 1 (for all parameters).

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
