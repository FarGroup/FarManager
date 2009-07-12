m4_include(`farversion.m4')m4_dnl
.Language=English,English
.Options CtrlColorChar=\
.Options CtrlStartPosChar=^<wrap>

@Contents
$^#File and archive manager#
`$^#'FULLVERSIONNOBRACES`#'
$^#Copyright (C) 1996-2000 Eugene Roshal#
`$^#Copyright (C)' COPYRIGHTYEARS `FAR Group#'
   ~Help file index~@Index@
   ~How to use help~@Help@

   ~About FAR~@About@
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
   ~Interface settings~@InterfSettings@
   ~Dialog settings~@DialogSettings@

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
$ # FAR: how to use help#
    Help screens may have reference items on them that lead to another help
screen. Also, the main page has the "~Help Index~@Index@", which lists all the
topics available in the help file and in some cases helps to find the needed
information faster.

    You may use #Tab# and #Shift-Tab# keys to move the cursor from one
reference item to another, then press #Enter# to go to a help screen describing
that item. With the mouse, you may click a reference to go to the help screen
about that item.

    If text does not completely fit in the help window, a scroll bar is
displayed. In such case #cursor keys# can be used to scroll text.

    You may press #Alt-F1# or #BS# to go back to a previous help screen and
#Shift-F1# to view the help contents.

    Press #Shift-F2# for ~plugins~@Plugins@ help.

    #Help# is shown by default in a reduced windows, you can maximize it by
pressing #F5# "#Zoom#", pressing #F5# again will restore the window to the
previous size.


@About
$ # FAR: about#
    #FAR# is a text mode file and archive manager for Windows
2000/XP/2003/Vista/2008/7. It supports #long file names# and provides a wide set
of file and folder operations.

    #FAR# is #freeware# and #open source# software distributed under the
revised BSD ~license~@License@.

    #FAR# does transparent #archive# processing. Files in the archive are
handled similarly as in a folder: when you operate with the archive, FAR
transforms your commands into the corresponding external archiver calls.

    #FAR# offers a number of service functions as well.


@License
$ # FAR: License#

 Copyright (c) 1996 Eugene Roshal
 Copyright (c) 2000 Far Group
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
$ # FAR: command line switches#
    The following switches may be used in the command line:

  #/a#    Disable display of characters with codes 0 - 31
        and 255.
        May be useful when executing FAR under telnet.

  #/ag#   Disable display of pseudo-graphic characters.

  #/e[<line>[:<pos>]] <filename>#
        Edit the specified file. After /e you may optionally
        specify editor start line and line position.

        For example: far /e70:2 readme.

  #/i#    Set small (16x16) icon for FAR console window.
        In some configurations this switch may lead to
        unstable work.

  #/p[<path>]#
        Search for "main" plugins in the folder given in <path>.
        Several search paths may be given separated by ';'.

        Example: far /p%SystemRoot%\\Profiles\\%USERNAME%\\FAR

  #/co#   Forces FAR to load plugins from cache only.
        Plugins are loaded faster this way, but new or changed
        plugins are not discovered. Should be used ONLY with a
        stable list of plugins. After adding, replacing or
        deleting a plugin FAR should be loaded without this
        switch. If the cache is empty, no plugins will be loaded.

        Remarks about switches /p and /co:

        - if /p is empty, then FAR will be loaded with
          no plugins;
        - if /p is given with a <path>, then only plugins
          from <path> will be loaded;
        - if only the /co switch is given and plugins cache
          is not empty, then plugins will be loaded from
          cache;
        - /co is ignored, if /p is given;
        - if /p and /co are not given, then plugins will
          be loaded from the main folder, and from the path
          given at the "~Path for personal plugins~@SystemSettings@" parameter.

  #/rc#   Upon closing, allows to restore console window title
        and size that were set before running FAR Manager.

  #/m#    FAR will not load macros from the registry when
        started.

  #/ma#   Macros with the "Run after FAR start" option set
        will not be run when FAR is started.

  #/u <username>#
        Allows to have separate settings for different users.

        For example: far /u guest

        FAR Manager will set the ~environment variable~@FAREnv@
        "FARUSER" to the value <username>.

  #/v <filename>#
        View the specified file. If <filename> is `#-#', data
        is read from the stdin.

        For example, "dir|far /v -" will view dir command output.

        If the input stream is empty when using '-' (for example,
        you have not specified the "dir" command in the provided
        example), FAR will wait forever for the end of data in the
        input stream. This will probably be fixed in a later
        version of FAR.


  #/x#    Disable exception handling. This option has been designed
        for plugin developers, and it is not recommended to specify
        it during normal operation.

    It is possible to specify at most two paths to folder, files or archives in
the command line. The first path applies to the active panel, the second path -
to the passive one:

  - if a folder or archive is specified, FAR will show its contents

  - if a file is specified, FAR will change to the folder where it
    resides and place the cursor on the file, if it exists.


@KeyRef
$ #Keyboard reference#

 ~Panel control~@PanelCmd@

 ~Command line~@CmdLineCmd@

 ~File management and service commands~@FuncCmd@

 ~Mouse: wheel support~@MsWheel@

 ~Miscellaneous~@MiscCmd@


@PanelCmd
$ #Panel control commands  #
    #Common panel commands#

  Change active panel                                            #Tab#
  Swap panels                                                 #Ctrl-U#
  Re-read panel                                               #Ctrl-R#
  Toggle info panel                                           #Ctrl-L#
  Toggle ~quick view panel~@QViewPanel@                                     #Ctrl-Q#
  Toggle tree panel                                           #Ctrl-T#
  Hide/show both panels                                       #Ctrl-O#
  Temporarily hide both panels                        #Ctrl-Alt-Shift#
    (as long as these keys are held down)
  Hide/show inactive panel                                    #Ctrl-P#
  Hide/show left panel                                       #Ctrl-F1#
  Hide/show right panel                                      #Ctrl-F2#
  Change panels height                             #Ctrl-Up,Ctrl-Down#
  Change panels width                           #Ctrl-Left,Ctrl-Right#
    (when the command line is empty)
  Restore default panels width                          #Ctrl-Numpad5#
  Restore default panels height                   #Ctrl-Shift-Numpad5#
  Show/Hide functional key bar at the bottom line.            #Ctrl-B#

    #File panel commands#

  Select/deselect file                        #Ins, Shift-Cursor keys#
  Select group                                                #Gray +#
  Deselect group                                              #Gray -#
  Invert selection                                            #Gray *#
  Select files with the same extension as the          #Ctrl-<Gray +>#
    current file
  Deselect files with the same extension as the        #Ctrl-<Gray ->#
    current file
  Invert selection including folders                   #Ctrl-<Gray *>#
    (ignore command line state)
  Select files with the same name as the current file   #Alt-<Gray +>#
  Deselect files with the same name as the current      #Alt-<Gray ->#
    file
  Select all files                                    #Shift-<Gray +>#
  Deselect all files                                  #Shift-<Gray ->#
  Restore previous selection                                  #Ctrl-M#

  Scroll long names and descriptions              #Alt-Left,Alt-Right#
                                                    #Alt-Home,Alt-End#

  Set brief view mode                                     #LeftCtrl-1#
  Set medium view mode                                    #LeftCtrl-2#
  Set full view mode                                      #LeftCtrl-3#
  Set wide view mode                                      #LeftCtrl-4#
  Set detailed view mode                                  #LeftCtrl-5#
  Set descriptions view mode                              #LeftCtrl-6#
  Set long descriptions view mode                         #LeftCtrl-7#
  Set file owners view mode                               #LeftCtrl-8#
  Set file links view mode                                #LeftCtrl-9#
  Set alternative full view mode                          #LeftCtrl-0#

  Toggle hidden and system files displaying                   #Ctrl-H#
  Toggle long/short file names view mode                      #Ctrl-N#

  Hide/Show left panel                                       #Ctrl-F1#
  Hide/Show right panel                                      #Ctrl-F2#

  Sort files in the active panel by name                     #Ctrl-F3#
  Sort files in the active panel by extension                #Ctrl-F4#
  Sort files in the active panel by modification time        #Ctrl-F5#
  Sort files in the active panel by size                     #Ctrl-F6#
  Keep files in the active panel unsorted                    #Ctrl-F7#
  Sort files in the active panel by creation time            #Ctrl-F8#
  Sort files in the active panel by access time              #Ctrl-F9#
  Sort files in the active panel by description             #Ctrl-F10#
  Sort files in the active panel by file owner              #Ctrl-F11#
  Display ~sort modes~@PanelCmdSort@ menu                                   #Ctrl-F12#
  Use group sorting                                        #Shift-F11#
  Show selected files first                                #Shift-F12#

  Create a ~folder shortcut~@FolderShortcuts@              #Ctrl-Shift-0# to #Ctrl-Shift-9#
  Jump to a folder shortcut               #RightCtrl-0# to #RightCtrl-9#

      If the active panel is a ~quick view panel~@QViewPanel@, a ~tree panel~@TreePanel@ or
    an ~information panel~@InfoPanel@, the directory is changed not on the
    active, but on the passive panel.

  Copy the names of selected files to the clipboard         #Ctrl-Ins#
   (if the command line is empty)
  Copy the names of selected files to the clipboard   #Ctrl-Shift-Ins#
   (ignore command line state)
  Copy full names of selected files to the clipboard   #Alt-Shift-Ins#
   (ignore command line state)
  Copy network (UNC) names of selected files to the     #Ctrl-Alt-Ins#
   clipboard (ignore command line state)

  Notes:

  1. If "Allow reverse sort modes" option in ~Panel settings~@PanelSettings@
     dialog is enabled, pressing the same sort key second time
     toggles the sort direction from ascending to descending
     and vice versa;

  2. #Alt-Left# and #Alt-Right# combinations, used to scroll long names
     and descriptions, work only with non-numpad #Left# and #Right# keys.
     This is due to the fact that when #Alt# is pressed, numpad cursor
     keys are used to enter characters via their decimal codes.

  3. The key combination #Ctrl-Alt-Ins# puts the following text into
     the clipboard:
     * for network drives - the network (UNC) name of the file
       object;
     * for local drives - the local name of the file taking into
       account ~symbolic links~@HardSymLink@.

  4. If #Alt-Shift-Ins# or #Ctrl-Alt-Ins# is pressed when the cursor
     is on the file "#..#", the name of the current folder is copied.


@PanelCmdSort
$ #Sort modes#
    The sort modes menu is called by #Ctrl-F12# and applies to the currently
active panel. The following sort modes are available:

  Sort files by name                                         #Ctrl-F3#
  Sort files by extension                                    #Ctrl-F4#
  Sort files by modification time                            #Ctrl-F5#
  Sort files by size                                         #Ctrl-F6#
  Keep files unsorted                                        #Ctrl-F7#
  Sort files by creation time                                #Ctrl-F8#
  Sort files by access time                                  #Ctrl-F9#
  Sort files by description                                 #Ctrl-F10#
  Sort files by file owner                                  #Ctrl-F11#

  Use group sorting                                        #Shift-F11#
  Show selected files first                                #Shift-F12#
  Use numeric sort.

  #Remarks on the numeric sort#

    The sorting algorithm which is used by the operating system to sort file
lists was changed in Windows XP. A numeric, not a string sort is used. FAR also
allows to use a numeric sort as in Windows XP - leading zeros in a file name
are ignored. The following example shows how the files are sorted:

    Numeric sort (Windows XP)    String sort (Windows 2000)

    Ie4_01                       Ie4_01
    Ie4_128                      Ie4_128
    Ie5                          Ie401sp2
    Ie6                          Ie5
    Ie401sp2                     Ie501sp2
    Ie501sp2                     Ie6
    5.txt                        11.txt
    11.txt                       5.txt
    88.txt                       88.txt


@FastFind
$ #Fast find in panels#
    To locate a file quickly, you can use the #fast find# operation and enter
the starting characters of the file name. In order to use that, hold down the
#Alt# (or #Alt-Shift#) keys and start typing the name of the needed file, until
the cursor is positioned to it.

    By pressing #Ctrl-Enter#, you can cycle through the files matching the part
of the filename that you have already entered. #Ctrl-Shift-Enter# allows to
cycle backwards.

    Besides the filename characters, you can also use the wildcard characters
'*' and '?'.

    Insertion of text, pasted from clipboard (#Ctrl-V# or #Shift-Ins#), to the
fast find dialog will continue as long as there is a match found.

    It is possible to use the transliteration function while entering text in
the search field. If used the entered text will be transliterated and a new
match corresponding to the new text will be searched. See TechInfo##10 on how
to set the hotkey for the transliteration.


@CmdLineCmd
$ #Command line commands#
 #Common command line commands#

  Character left                                         #Left,Ctrl-S#
  Character right                                       #Right,Ctrl-D#
  Word left                                                #Ctrl-Left#
  Word right                                              #Ctrl-Right#
  Start of line                                            #Ctrl-Home#
  End of line                                               #Ctrl-End#
  Delete char                                                    #Del#
  Delete char left                                                #BS#
  Delete to end of line                                       #Ctrl-K#
  Delete word left                                           #Ctrl-BS#
  Delete word right                                         #Ctrl-Del#
  Copy to clipboard                                         #Ctrl-Ins#
  Paste from clipboard                                     #Shift-Ins#
  Previous command                                            #Ctrl-E#
  Next command                                                #Ctrl-X#
  Clear command line                                          #Ctrl-Y#

 #Insertion commands#

  Insert current file name from the active panel   #Ctrl-J,Ctrl-Enter#

     In the ~fast find~@FastFind@ mode, #Ctrl-Enter# does not insert a
     file name, but instead cycles the files matching the
     file mask entered in the fast find box.

  Insert current file name from the passive panel   #Ctrl-Shift-Enter#
  Insert full file name from the active panel                 #Ctrl-F#
  Insert full file name from the passive panel                #Ctrl-;#
  Insert network (UNC) file name from the active panel    #Ctrl-Alt-F#
  Insert network (UNC) file name from the passive panel   #Ctrl-Alt-;#

  Insert path from the left panel                             #Ctrl-[#
  Insert path from the right panel                            #Ctrl-]#
  Insert network (UNC) path from the left panel           #Ctrl-Alt-[#
  Insert network (UNC) path from the right panel          #Ctrl-Alt-]#

  Insert path from the active panel                     #Ctrl-Shift-[#
  Insert path from the passive panel                    #Ctrl-Shift-]#
  Insert network (UNC) path from the active panel        #Alt-Shift-[#
  Insert network (UNC) path from the passive panel       #Alt-Shift-]#

  Notes:

  1. If the command line is empty, #Ctrl-Ins# copies selected file
     names from a panel to the clipboard like #Ctrl-Shift-Ins#
     (see ~Panel control commands~@PanelCmd@);

  2. #Ctrl-End# pressed at the end of the command line, replaces
     its current contents with a command from ~history~@History@
     beginning with the characters that are in the command line,
     if such a command exists. You may press #Ctrl-End# again to go
     to the next such command.

  3. Most of the described above commands are valid for all edit
     strings including edit controls in dialogs and internal editor.

  4. #Alt-Shift-Left#, #Alt-Shift-Right#, #Alt-Shift-Home# and
     #Alt-Shift-End# select the block in the command line also when the
     panels are on.

  5. For local drives, the commands to insert the network (UNC)
     name of a file object insert the local name of the file with
     ~symbolic links~@HardSymLink@ expanded.


@FuncCmd
$ #Panel control commands - service commands#
  Online help                                                     #F1#

  Show ~user menu~@UserMenu@                                                  #F2#

  View                                   #Ctrl-Shift-F3, Numpad 5, F3#

    If pressed on a file, #Numpad 5# and #F3# invoke ~internal~@Viewer@,
external or ~associated~@FileAssoc@ viewer, depending upon the file type and
~external viewer settings~@ViewerSettings@.
    #Ctrl-Shift-F3# always calls internal viewer ignoring file associations.
    If pressed on a folder, calculates and shows the size of selected folders.

  Edit                                             #Ctrl-Shift-F4, F4#

    #F4# invokes ~internal~@Editor@, external or ~associated~@FileAssoc@
editor, depending upon the file type and ~external editor settings~@EditorSettings@.
    #Ctrl-Shift-F4# always calls internal editor ignoring file associations.
    #F4# and #Ctrl-Shift-F4# for directories invoke the change file
~attributes~@FileAttrDlg@ dialog.

  ~Copy~@CopyFiles@                                                            #F5#

    Copies files and folders. If you wish to create the destination folder
before copying, terminate the name with a backslash.

  ~Rename or move~@CopyFiles@                                                  #F6#

    Moves or renames files and folders. If you wish to create the destination
folder before moving, terminate the name with a backslash.

  ~Create new folder~@MakeFolder@                                               #F7#

  ~Delete~@DeleteFile@                                     #Shift-Del, Shift-F8, F8#

  ~Wipe~@DeleteFile@                                                       #Alt-Del#

  Show ~menus~@Menus@ bar                                                  #F9#

  Quit FAR                                                       #F10#

  Show ~plugin~@Plugins@ commands                                           #F11#

  Change the current drive for left panel                     #Alt-F1#

  Change the current drive for right panel                    #Alt-F2#

  Internal/external viewer                                    #Alt-F3#

    If the internal viewer is used by default, invokes the external viewer
specified in the ~settings~@ViewerSettings@ or the ~associated viewer program~@FileAssoc@
for the file type. If the external viewer is used by default, invokes the
internal viewer.

  Internal/external editor                                    #Alt-F4#

    If the internal editor is used by default, invokes the external editor
specified in the ~settings~@EditorSettings@ or the ~associated editor program~@FileAssoc@
for the file type. If the external editor is used by default, invokes the
internal editor.

  Print files                                                 #Alt-F5#

    If the "Print Manager" plugin is installed then the printing of
    the selected files will be carried out using that plugin,
    otherwise by using internal facilities.

  Create ~file links~@HardSymLink@ (NTFS only)                               #Alt-F6#

    Using hard file links you may have several different file names referring
to the same data.

  Perform ~find file~@FindFile@ command                                   #Alt-F7#

  Display ~commands history~@History@                                    #Alt-F8#

  Toggles the size of the FAR console window                  #Alt-F9#

    In the windowed mode, toggles between the current size and the maximum
possible size of a console window. In the fullscreen mode, #Alt-F9# toggles the
screen height between 25 and 50 lines. See TechInfo##38 for details.

  Configure ~plugin~@Plugins@ modules.                             #Alt-Shift-F9#

  Perform ~find folder~@FindFolder@ command                                #Alt-F10#

  Display ~view and edit history~@HistoryViews@                              #Alt-F11#

  Display ~folders history~@HistoryFolders@                                    #Alt-F12#

  Add files to archive                                      #Shift-F1#
  Extract files from archive                                #Shift-F2#
  Perform archive managing commands                         #Shift-F3#
  Edit ~new file~@FileOpenCreate@                                             #Shift-F4#

    When a new file is opened, the same code page is used as in the last opened
editor. If the editor is opened for the first time in the current FAR session,
the default code page is used.

  Copy file under cursor                                    #Shift-F5#
  Rename or move file under cursor                          #Shift-F6#

    For folders: if the specified path (absolute or relative) points to an
existing folder, the source folder is moved inside that folder. Otherwise the
folder is renamed/moved to the new path.
    E.g. when moving #c:\folder1\# to #d:\folder2\#:
    - if #d:\folder2\# exists, contents of #c:\folder1\# is
moved into #d:\folder2\folder1\#;
    - otherwise contents of #c:\folder1\# is moved into the
newly created #d:\folder2\#.

  ~Delete file~@DeleteFile@ under cursor                                  #Shift-F8#
  Save configuration                                        #Shift-F9#
  Selects last executed menu item                          #Shift-F10#

  Execute, change folder, enter to an archive                  #Enter#
  Execute in the separate window                         #Shift-Enter#

    Pressing #Shift-Enter# on a directory invokes the Windows Explorer and
shows the selected directory. To show a root directory in the Explorer, you
should press #Shift-Enter# on the required drive in the ~drive selection menu~@DriveDlg@.
Pressing #Shift-Enter# on "#..#" opens the current directory in the Explorer.

  Change to the root folder                                   #Ctrl-\\#

  Change folder, enter an archive (also a SFX archive)     #Ctrl-PgDn#

    If the cursor points to a directory, pressing #Ctrl-PgDn# changes to that
directory. If the cursor points to a file, then, depending on the file type,
an ~associated command~@FileAssoc@ is executed or the archive is opened.

  Change to the parent folder                              #Ctrl-PgUp#

    If the option "~Use Ctrl-PgUp to change drive~@InterfSettings@" is enabled,
pressing #Ctrl-PgUp# in the root directory switches to the network plugin or
shows the ~drive selection menu~@DriveDlg@.

  Create shortcut to the current folder              #Ctrl-Shift-0..9#

  Use folder shortcut                                 #RightCtrl-0..9#

  Set ~file attributes~@FileAttrDlg@                                         #Ctrl-A#
  ~Apply command~@ApplyCmd@ to selected files                             #Ctrl-G#
  ~Describe~@FileDiz@ selected files                                     #Ctrl-Z#


@DeleteFile
$ #Deleting and wiping files and folders#
    The following hotkeys are used to delete or wipe out files and folders:

    #F8#         - if any files or folders are selected in the panel
                 then the selected group will be deleted, otherwise
                 the object currently under cursor will be deleted;

    #Shift-F8#   - delete only the file under cursor
                 (with no regard to selection in the panel);

    #Shift-Del#  - delete selected objects, skipping the Recycle Bin;

    #Alt-Del#    - Wipe out files and folders.


    Remarks:

    1. In accordance to ~System Settings~@SystemSettings@ the hotkeys #F8# and
#Shift-F8# do or do not move the deleted files to the Recycle Bin. The
#Shift-Del# hotkey always deletes, skipping the Recycle Bin.

    2. Before file deletion its data is overwritten with zeroes (you can
specify other overwrite characters - see TechInfo##29), after which the file
is truncated to a zero sized file, renamed to a temporary name and then
deleted.


@ErrCannotExecute
$ #Error: Cannot execute#
    The program you tries to execute is not recognized as an internal or
external command, operable program or batch file.

    When executing the contents of the command line, FAR searches for the
executable in the following sequence (sequentially substituting all extensions
listed in the environment variable %PATHEXT%):

  1. The current directory
  2. The directories that are listed in the PATH environment variable
  3. The 32-bit Windows system directory.
  4. The 16-bit Windows system directory.
  5. The Windows directory.


@MiscCmd
$ #Panel control commands - miscellaneous#
  Screen grabber                                             #Alt-Ins#

    Screen grabber allows to select and copy to the clipboard any screen area.
Use #arrow# keys or click the #left mouse button# to move the cursor. To select
text use #Shift-arrow# keys or drag the mouse while holding the #left mouse#
#button#. #Enter#, #Ctrl-Ins#, #right mouse button# or #doubleclick# copy
selected text to the clipboard, #Ctrl-<Gray +># appends it to the clipboard
contents, #Esc# leaves the grabbing mode. #Ctrl-U# - deselect block.

  Record a ~keyboard macro~@KeyMacro@                                   #Ctrl-<.>#

  History in dialog edit controls                 #Ctrl-Up, Ctrl-Down#

    In dialog edit control history you may use #Enter# to copy the current item
to the edit control and #Ins# to mark or unmark an item. Marked items are not
pushed out of history by new items, so you may mark frequently used strings so
that you will always have them in the history.

  Clear history in dialog edit controls                          #Del#

  Delete the current item in a dialog edit line history
  list (if it is not locked)                               #Shift-Del#

  Set the dialog focus to the default element                   #PgDn#

  Insert a file name under cursor to dialog              #Shift-Enter#

  Insert a file name from passive panel to dialog   #Ctrl-Shift-Enter#

    This key combination is valid for all edit controls except the command
line, including dialogs and the ~internal editor~@Editor@.

    Pressing #Ctrl-Enter# in dialogs executes the default action (pushes the
default button or does another similar thing).

    In dialogs, when the current control is a check box:

  - turn on (#[x]#)                                             #Gray +#
  - turn off (#[ ]#)                                            #Gray -#
  - change to undefined (#[?]#)                                 #Gray *#
    (for three-state checkboxes)

    #Left clicking# outside the dialog works the same as pressing #Esc#.

    #Right clicking# outside the dialog works the same as pressing #Enter#.

    FAR Manager also supports the ~mouse wheel~@MsWheel@.

    You can move a dialog (window) by dragging it with mouse or by pressing
#Ctrl-F5# and using #arrow# keys.


@MsWheel
$ #Mouse: wheel support#

   #Panels#    Rotating the wheel scrolls the file list without
             changing the cursor position on the screen.

   #Editor#    Rotating the wheel scrolls the text without changing
             the cursor position on the screen (similar to #Ctrl-Up#/
             #Ctrl-Down#).

   #Viewer#    Rotating the wheel scrolls the text.

   #Help#      Rotating the wheel scrolls the text.

   #Menus#     Wheel scrolling works as #Up#/#Down# keys.
             It is possible to choose items without moving the cursor.

   #Dialogs#   In dialogs, when the wheel is rotated at an edit line
             with a history list or a combo box, the drop-down list
             is opened. In the drop-down list scrolling works the
             same as in menus.

    You can specify the number of lines to scroll at a time in the panels,
editor and viewer (see TechInfo##33).

@Plugins
$ #Plugins support#
    External DLL modules (plugins) may be used to implement new FAR commands
and emulate file systems. For example, archives support, FTP client, temporary
panel and network browser are plugins that emulate file systems.

    All plugins are stored in separate folders within the 'Plugins' folder,
which is in the same folder as FAR.EXE. When detecting a new plugin FAR saves
information about it and later loads the plugin only when necessary, so unused
plugins do not require additional memory. But if you are sure that some plugins
are useless for you, you may remove them to save disk space.

    Plugins may be called either from ~Change drive menu~@DriveDlg@ or from
#Plugin commands# menu, activated by #F11# or by corresponding item of
~Commands menu~@CmdMenu@. #F4# in "Plugin commands" menu allows to assign hot
keys to menu items (this makes easier to call them from ~keyboard macros~@KeyMacro@).
This menu is accessible from file panels and (only by #F11#) from the
internal viewer and editor. Only specially designed plugins will be shown when
calling the menu from the viewer and editor.

    You may set plugin parameters using ~Plugin configuration~@PluginsConfig@
command from ~Options menu~@OptMenu@.

    File processing operations like copy, move, delete, edit or ~Find file~@FindFile@
work with plugins, which emulate file systems, if these plugins provide
necessary functionality. Search from the current folder in the "Find file"
command requires less functionality than search from the root folder, so try to
use it if search from the root folder does not work correctly.

    The modules have their own message and help files. You can get a list of
available help on the modules by pressing

    #Shift-F2# - anywhere in the FAR help system

    #Shift-F1# - in the list of plugins (context-dependent help).

    If the plugin has no help file, then context-dependent help will not pop
out.

    If the active panel shows a plugin emulated file system, the command "CD"
in the command line may be used to change the plugin file system folder. Unlike
"CD", "CHDIR" command always treats the specified parameter as a real folder
name regardless a file panel type.

    Use #Alt-Shift-F9# to ~configure plugins~@PluginsConfig@.


@PluginsConfig
$ #Plugins configuration#
    You can configure the installed ~plugin modules~@Plugins@ using the command
#"Plugins configuration"# from the ~Options menu~@OptMenu@ or by pressing
#Alt-Shift-F9# in the ~drive selection menu~@DriveDlg@ or plugins menu.

    To get the help on the currently selected plugin, press #Shift-F1# -
context-sensitive help on plugin configuration. If the plugin doesn't have a
help file, the context-sensitive help will not be shown.

    When the context-sensitive help is invoked, FAR will try to show the topic
#Config#. If such a topic does not exist in the plugin help file, the main help
topic for the selected plugin will be shown.

    Pressing #F4# in the #"Plugins configuration"# menu allows to assign
hotkeys for the items of that menu, allowing to invoke them later through
~keyboard macro commands~@KeyMacro@.


@PluginsReviews
$ #Overview of plugin capabilities#
    The FAR manager is so tightly integrated with its plugins that it is simply
meaningless to talk about FAR and not to mention the plugins. Plugins present
an almost limitless expansion of the features of FAR.

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
    editing text in FAR editor.
  * Windows help files contents displaying (.hlp and .chm)
  * Calculators with different possibilities.
  * Several games :-)
  * Spell checker functions while editing text in FAR editor.
  * Removable drives catalog preparation and much more...

    As an information source, which can be used to search for specific plugins,
one can recommend:

  - Far Group site
    ~http://www.farmanager.com~@http://www.farmanager.com@
  - Online forum
    ~http://enforum.farmanager.com~@http://enforum.farmanager.com@
  - PlugRinG site
    ~http://plugring.farmanager.com~@http://plugring.farmanager.com@
  - USENET echo conference
    ~news:fido7.far.support~@news:fido7.far.support@
    ~news:fido7.far.development~@news:fido7.far.development@
  - FidoNet echo conference
    far.support
    far.development
  - Free email group service
    ~http://groups.yahoo.com/group/plugring~@http://groups.yahoo.com/group/plugring@
    ~http://groups.yahoo.com/group/farpluginsapi~@http://groups.yahoo.com/group/farpluginsapi@


@Panels
$ #Panels #
    Normally FAR shows two panels (left and right windows), with different
information. If you want to change the type of information displayed in the
panel, use the ~panel menu~@LeftRightMenu@ or corresponding ~keyboard commands~@KeyRef@.

    See also the following topics to obtain more information:

      ~File panel~@FilePanel@                 ~Tree panel~@TreePanel@
      ~Info panel~@InfoPanel@                 ~Quick view panel~@QViewPanel@

      ~Drag and drop files~@DragAndDrop@
      ~Selecting files~@SelectFiles@
      ~Customizing file panel view modes~@PanelViewModes@


@FilePanel
$ #Panels: file panel#
    The file panel displays the current folder. You may select or deselect
files and folders, perform different file and archive operations. Read
~Keyboard reference~@KeyRef@ for commands list.

    Default view modes of the file panel are:

 #Brief#         File names are displayed within three columns.

 #Medium#        File names are displayed within two columns.

 #Full#          Name, size, date and time of the file are displayed.

 #Wide#          File names and sizes are displayed.

 #Detailed#      File names, sizes, packed sizes, last modification,
               creation, access time and attributes are displayed.
               Fullscreen mode.

 #Descriptions#  File names and ~file descriptions~@FileDiz@

 #Long#          File names, sizes and descriptions.
 #descriptions#  Fullscreen mode.

 #File owners#   File names, sizes and owners.

 #File links#    File names, sizes and number of hard links.

 #Alternative#   File name, size (formatted with commas) and date
 #full#          of the file are displayed.

    You may ~customize file panel view modes~@PanelViewModes@.

    Packed sizes are valid for NTFS compressed files or files inside an
archive. File owners and number of hard links have meaning for NTFS only. Some
file systems may not support file creation and access dates.

    If you wish to change the panel view mode, choose it from the
~panel menu~@LeftRightMenu@. After the mode change or drive change action,
if the initial panel type differs it will be automatically set to the file
panel.

    ~Speed search~@FastFind@ action may be used to point to the required file
by the first letters of its name.


@TreePanel
$ #Panels: tree panel#
    The tree panel displays the folder structure of the current disk as a tree.
Within tree mode you may change to a folder quickly and perform folder
operations.

    FAR stores folder tree information in the file named #Tree.Far# at root
folder of each drive. For read-only drives this information is stored in the
hidden folder Tree.Cache within the folder containing FAR.EXE. The Tree.FAR
file doesn't exist by default. It will be automatically created after the first
use of the #Tree Panel# or the #Find Folder# command. If that file exists, FAR
updates it with the changes to the tree structure it is aware of. If such
changes were made outside of FAR and Tree.far is no longer current, it can be
refreshed by pressing #Ctrl-R#.

    You can find a folder quickly with the help of #speed search# action. Hold
the Alt key and type the folder name until you point to the right folder.
Pressing #Ctrl-Enter# keys simultaneously will select the next match.

    #Gray +# and #Gray -# keys move up and down the tree to the next branch
on the same level.

@InfoPanel
$ #Panels: info panel#
    The information panel contains the following data:

 - #network# names of the computer and the current user;

 - name and type of the #current disk#, type of the file system, network
name, total and free space, disk volume label and serial number;

 - #memory# load percentage (100% means all of available memory is used),
total and free size of the physical and virtual memory;

 - #folder description# file

    You may view the contents of the folder description file in full screen by
pressing the #F3# key or clicking the #left mouse button#. To edit or create the
description file, press #F4# or click the #right mouse button#. You can also use
many of the ~viewer commands~@Viewer@ (search, code page selection and so on)
for viewing the folder description file.

    A list of possible folder description file names may be defined using
"Folder description files" command in the ~Options menu~@OptMenu@.

    FAR will attempt to determine the type of each of the CD drives available
in the system. Known types are as follows: CD-ROM, CD-RW, CD-RW/DVD, DVD-ROM,
DVD-RW and DVD-RAM. This function is available only for users either with
administrative privileges or all local users, when it's stated explicitly in
the Local Policy Editor (to do this, run a #secpol.msc# from the command
prompt, and set the '#Local Policies/Security Options/Devices: Restrict#
#CD-ROM access to locally logged-on user only#' setting to '#Enabled#')

    For virtual devices (SUBST-disk) the parameters of the primary disk are
shown.

@QViewPanel
$ #Panels: quick view panel#
    The quick view panel is used to show information about the selected item in
the ~file panel~@FilePanel@ or ~tree panel~@TreePanel@.

    If the selected item is a file then the contents of the file is displayed.
Many of the ~internal viewer~@Viewer@ commands can be used with the file
displayed in the panel. For files of registered Windows types the type is shown
as well.

    For folders, the quick view panel displays total size, total compressed
size, number of files and subfolders in the folder, current disk cluster size,
real files size, including files slack (sum of the unused cluster parts).
Compressed size has meaning for NTFS drives only.

    When viewing reparse points, the path to the source folder is also displayed.

    For folders, the total size value may not match the actual value:

    1. If the folder or its subfolders contain symbolic links and the option
"Scan symbolic links" in the ~System parameters dialog~@SystemSettings@ is
enabled.

    2. If the folder or its subfolders contain multiple hard links to the same
file.


@DragAndDrop
$ #Copying: drag and drop files#
    It is possible to perform #Copy# and #Move# file operations using #drag and
drop#. Press left mouse button on the source file or folder, drag it to the
another panel and release the mouse button.

    If you wish to process a group of files or folders, select them before
dragging, click the left mouse button on the source panel and drag the files
group to the other panel.

    You may switch between copy and move operations by pressing the right mouse
button while dragging. Also to move files you can hold the #Shift# key while
pressing the left mouse button.


@Menus
$ #Menus #
    To choose an action from the menu you may press F9 or click on top of the
screen.

    When the menu is activated by pressing #F9#, the menu for the active panel
is selected automatically. When the menu is active, pressing #Tab# switches
between the menus for the left and right panel. If the menus "Files",
"Commands" or "Options" are active, pressing #Tab# switches to the menu of the
passive panel.

    Use the #Shift-F10# key combination to select the last used menu command.

    Read the following topics for information about a particular menu:

     ~Left and right menus~@LeftRightMenu@          ~Files menu~@FilesMenu@

     ~Commands menu~@CmdMenu@                 ~Options menu~@OptMenu@


@LeftRightMenu
$ #Menus: left and right menus#
    The #Left# and #Right# menus allow to change left and right panel settings
respectively. These menus include the following items:

   #Brief#                Display files within three columns.

   #Medium#               Display files within two columns.

   #Full#                 Display file name, size, date and time.

   #Wide#                 Display file name and size.

   #Detailed#             Display file name, size, packed size,
                        modification, creation and access time,
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


@FilesMenu
$ #Menus: files menu#
   #View#               ~View files~@Viewer@, count folder sizes.

   #Edit#               ~Edit~@Editor@ files.

   #Copy#               ~Copy~@CopyFiles@ files and folders.

   #Rename or move#     ~Rename or move~@CopyFiles@ files and folders.

   #Make folder#        ~Create~@MakeFolder@ new folder.

   #Delete#             Delete files and folders.

   #Wipe#               Wipe files and folders. Before file deletion
                      its data are overwritten with zeroes, after
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


@CmdMenu
$ #Menus: commands menu#
   #Find file#            Search for files in the folders tree,
                        wildcards may be used.
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
                        selection. Use #Shift-Enter# to select item
                        without changing its position.

   #Swap panels#          Swap left and right panels.

   #Panels On/Off#        Show/hide both panels.

   #Compare folders#      Compare contents of folders.
                        See ~Compare folders~@CompFolders@ for the
                        detailed description.

   #Edit user menu#       Allows to edit main or local ~user menu~@UserMenu@.
                        You may press #Ins# to insert, #Del# to delete
                        and #F4# to edit menu records.

   #Edit associations#    Displays the list of ~file associations~@FileAssoc@.
                        You may press #Ins# to insert, #Del# to delete
                        and #F4# to edit file associations.

   #Folder shortcuts#     Displays current ~folder shortcuts~@FolderShortcuts@.

   #File panel filter#    Allows to control file panel contents.
                        See ~filters menu~@FiltersMenu@ for the detailed
                        description.

   #Plugin commands#      Show ~plugin commands~@Plugins@ list

   #Screens list#         Show open ~screens list~@ScrSwitch@

   #Task list#            Shows ~active tasks list~@TaskList@.

   #Hotplug devices list# Show ~hotplug devices list~@HotPlugList@.


@OptMenu
$ #Menus: options menu#
   #System settings#      Shows ~system settings~@SystemSettings@ dialog.

   #Panel settings#       Shows ~panel settings~@PanelSettings@ dialog.

   #Interface settings#   Shows ~interface settings~@InterfSettings@ dialog.

   #Dialog settings#      Shows ~dialog settings~@DialogSettings@ dialog.

   #Languages#            Select main and help language.
                        Use "Save setup" to save selected languages.

   #Plugins#              Configure ~plugin~@Plugins@ modules.
   #configuration#

   #Confirmation#         Switch on or off ~confirmations~@ConfirmDlg@ for
                        some operations.

   #File panel modes#     ~Customize file panel view modes~@PanelViewModes@ settings.

   #File descriptions#    ~File descriptions~@FileDiz@ list names and update mode.

   #Folder description#   Specify names (~wildcards~@FileMasks@ are allowed) of
   #files#                files displayed in the ~Info panel~@InfoPanel@ as folder
                        descriptions.

   #Viewer settings#      External and internal ~viewer settings~@ViewerSettings@.

   #Editor settings#      External and internal ~editor settings~@EditorSettings@.

   #Colors#               Allows to select colors for different
                        interface items, to change the entire FAR
                        colors palette to black and white or to set
                        the colors to default.

   #Files highlighting#   Change ~files highlighting and sort groups~@Highlight@
   #and sort groups#      settings.

   #Save setup#           Save current configuration, colors and
                        screen layout.


@ConfirmDlg
$ #Confirmations#
    In the #Confirmations# dialog you can switch on or off confirmations for
following operations:

    - overwrite destination files when performing file copying;

    - overwrite destination files when performing file moving;

    - ~drag and drop~@DragAndDrop@ files;

    - delete files;

    - delete folders;

    - interrupt operation;

    - ~disconnect network drives~@DisconnectDrive@ from the Disks menu;

    - disconnect SUBST-disks from the Disks menu;

    - removal of USB storage devices from the Disks menu;

    - ~reloading~@EditorReload@ a file in the editor;

    - clearing the view/edit, folder and command history lists;

    - exit from FAR.


@MakeFolder
$ #Make folder#
    This function allows you to create folders. You can use environment
variables in the input line, which are expanded to their values before creating
the folder. Also you can create multiple nested subfolders at the same time:
simply separate the folder names with the backslash character. For example:

    #%USERDOMAIN%\\%USERNAME%\\Folder3#

    If the option "#Process multiple names#" is enabled, it is possible to
create multiple folders in a single operation. In this case, folder names
should be separated with the character "#;#" or "#,#". If the option is enabled
and the name of the folder contains a character "#;#" (or "#,#"), it must be
enclosed in quotes. For example, if the user enters
#C:\\Foo1;"E:\\foo,2;";D:\\foo3#, folders called "#C:\\Foo1#", "#E:\\foo,2;#"
and "#D:\\foo3#" will be created.


@FindFile
$ #Find file #
    This command allows to locate in the folder tree one or more files and
folders matching one or more ~wildcards~@FileMasks@ (delimited with commas). It
may also be used with file systems emulated by ~plugins~@Plugins@.

    Optional text string may be specified to find only files containing this
text. If the string is entered, the option #Case sensitive# selects case
sensitive comparison.

    The option #Whole words# will let to find only the text that is separated
from other text with spaces, tab characters, line breaks or standard
separators, which by default are: #!%^&*()+|{}:"<>?`-=\\[];',./#.

    By checking the #Search for hex# option you can search for the files
containing hexadecimal sequence of the specified bytes. In this case #Case#
#sensitive#, #Whole words#, #Using code page# and #Search for folders#
options are disabled and their values doesn't affect the search process.

    Выпадающий список #Используя кодовую страницу# позволяет выбрать конкретную
кодовую страницу, применяемую для поиска текста. Если в выпадающем списке выбрать
пункт #Все кодовые страницы#, то FAR будет использовать для поиска все стандартные
и #Любимые# кодовые страницы (список #Любимых# кодовых страниц можно настроить в
меню выбора кодовой страницы редактора или программы просмотра). Если перечень
кодовых страниц, поиск по которым производится при выборе пункта #Все кодовые#
#страницы#, является для вас избыточным, то вы можете, при помощи клавиш #Ins# и
#Space#, выбрать из списка стандартных и #Любимых# кодовых страниц только те кодовые
страницы, по которым вам необходимо осуществлять поиск.

    If the option #Search in archives# is set, FAR also performs the search in
archives with known formats. However, using this option significantly decreases
the performance of the search. FAR cannot search in nested archives.

    The #Search for folders# option includes in search list those folders, that
match the wildcards. Also the counter of found files takes account of found
folders.

    The #Search in symbolic links# option allows searching files in
~symbolic links~@HardSymLink@ along with normal sub-folders.

    Remark: if the file system contains recursive cycles of symbolic
links (e.g. a symlink pointing to one of its parent folders), then the
search will be conducted until the maximum allowed path length is
reached. In this case search may take much more time.

    Search may be performed:

    - in all non-removable drives;

    - in all local drives, except removable and network;

    - in all folders specified in the %PATH% environment variable
      (not including subfolders).

    - in all folders from the drive root, in the find
      dialog one can change disk drive of the search. ;

    - from the current folder;

    - in the current folder only or in selected folders
      (the current version of FAR does not search in
      directories that are ~symbolic links~@HardSymLink@).

    The search parameters is saved in the configuration.

    Check the #Use filter# checkbox to search for files according to user
defined conditions. Press the #Filter# button to open the ~filters menu~@FiltersMenu@.

    The #Advanced# button invokes the ~find file advanced options~@FindFileAdvanced@
dialog that can be used to specify extended set of search properties.


@FindFileAdvanced
$ #Find file advanced options#
    The text string that is entered in #Containing text# (or #Containing hex#)
field can be searched not only in the whole file, but also inside a specified
range at the beginning of the file, defined by the #Search only in the first#
property. If the specified value is less than the file size, the rest of the
file will be ignored even if the required sequence exists there.

The following file size suffixes can be used:
B - for bytes (no suffix also means bytes);
K - for kilobytes;
M - for megabytes;
G - for gigabytes;
T - for terabytes;
P - for petabytes;
E - for exabytes.


    #Process alternate data streams#

    Помимо основного потока данных (представляющего собой непосредственно содержимое файла)
производить поиск также в альтернативных именованных потоках, поддерживаемых некоторыми
файловыми системами (например, #NTFS#).


@FindFileResult
$ #Find file: control keys#
    While ~search~@FindFile@ is in progress or when it is finished, you may use
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

    #F3# and #F4# may be used to view and edit found files. Also viewing and
editing is supported for plugin emulated file systems. Note, that saving editor
changes by #F2# key in emulated file system will perform #SaveAs# operation,
instead of common #Save#.


@FindFolder
$ #Find folder#
    This command allows a quick look for the required folder in the folders
tree.

    To select a folder you may use the cursor keys or type first characters of
the folder.

    Press #Enter# to switch to the selected folder.

    #Ctrl-R# and #F2# force the rescan of the folders tree.

    #Gray +# and #Gray -# should move up and down the tree to the next branch
on the same level.

    #F5# allows to maximize the window, pressing #F5# again will restore the
window to the previous size.

    By pressing #Ctrl-Enter#, you can cycle through the folders matching the
part of the filename that you have already entered. #Ctrl-Shift-Enter# allows
to cycle backwards.

@Filter
$ #Filter#
    Operations filter is used to process certain file groups according to
rules specified by the user, if those rules are met by some file it
will be processed by a corresponding operation. A filter may have several
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


   #Date/time#       Starting and ending file date/time.
                   You can specify the date/time of last file
                   #modification#, file #creation# and last file
                   #access#.

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
                   #Virtual# attribute is used only in Windows
                   Vista/2008/7.


    To quickly disable one or several conditions, uncheck the corresponding
checkboxes. The #Reset# button will clear all of the filter conditions.


@History
$ #History #
    The commands history shows the list of previously executed commands.
Besides the cursor control keys, the following keyboard shortcuts are
available:

  Execute a previously executed command                         #Enter#

  Execute a previously executed command in a new window   #Shift-Enter#

  Copy a command to the command line                       #Ctrl-Enter#

  Clear the commands history                                      #Del#

  Установить/снять пометку блокировки пункт истории               #Ins#

  Delete the current history item                           #Shift-Del#

  Copy the text of the current command to the clipboard        #Ctrl-C#
  without closing the list                                or #Ctrl-Ins#

    To go to the previous or next command directly from the command line, you
can press #Ctrl-E# or #Ctrl-X# respectively.

    For choosing a command, besides the cursor control keys and #Enter#, you can
use the highlighted shortcut letters.

    If you want to save the commands history after exiting FAR, use the
respective option in the ~system settings dialog~@SystemSettings@.

    Заблокированные пункты не будут удаляться при очистке истории.

@HistoryViews
$ #History: file view and edit#
    The file view history shows the list of files that have been recently
viewed or edited. Besides the cursor control keys, the following keyboard
shortcuts are available:

  Reopen a file for viewing or editing                          #Enter#

  Copy the file name to the command line                   #Ctrl-Enter#

  Clear the history list                                          #Del#

  Delete the current history item                           #Shift-Del#

  Установить/снять пометку блокировки пункт истории               #Ins#

  Refresh list and remove non-existing entries                 #Ctrl-R#

  Copy the text of the current history item to the             #Ctrl-C#
  clipboard without closing the list                      or #Ctrl-Ins#

  Open a file in the ~editor~@Editor@                                        #F4#

  Open a file in the ~viewer~@Viewer@                                        #F3#
                                                          or #Numpad 5#

    For choosing a history item, besides the cursor control keys and #Enter#,
you can use the highlighted shortcut letters.

    Items of the view and edit history are moved to the end of the list after
they are selected. You can use #Shift-Enter# to select an item without changing
its position.

    If you want to save the view and edit history after exiting FAR, use the
respective option in the ~system settings dialog~@SystemSettings@.

  Remarks:

  1. List refresh operation (Ctrl-R) can take a considerable amount
     of time if a file was located on a currently unavailable remote
     resource.

  2. Заблокированные пункты не будут удаляться при очистке или обновлении
     истории.

@HistoryFolders
$ #History: folders#
    The folders history shows the list of folders that have been recently
visited. Besides the cursor control keys, the following keyboard shortcuts are
available:

  Go to the current folder in the list                          #Enter#

  Open the selected folder in the passive panel      #Ctrl-Shift-Enter#

  Copy the folder name to the command line                 #Ctrl-Enter#

  Clear the history list                                          #Del#

  Delete the current history item                           #Shift-Del#

  Установить/снять пометку блокировки пункт истории               #Ins#

  Refresh list and remove non-existing entries                 #Ctrl-R#

  Copy the text of the current history item to the             #Ctrl-C#
  clipboard without closing the list                      or #Ctrl-Ins#

    For choosing a history item, besides the cursor control keys and #Enter#,
you can use the highlighted shortcut letters.

    Items of the folders history are moved to the end of the list after they
are selected. You can use #Shift-Enter# to select an item without changing its
position.

    If you want to save the folders history after exiting FAR, use the
respective option in the ~system settings dialog~@SystemSettings@.

  Remarks:

  1. List refresh operation (Ctrl-R) can take a considerable amount
     of time if a folder was located on a currently unavailable
     remote resource.

  2. Заблокированные пункты не будут удаляться при очистке или обновлении
     истории.

@TaskList
$ #Task list#
    The task list displays active tasks. Each line of the list contains a task
window title.

    From the task list you may switch to the task window, or kill the task with
the #Del# key. Be careful when killing a task. It stops the task immediately,
and any unsaved information will be lost, so it should be used only when really
necessary, for example to interrupt a program which does not respond.

    The task list may be called either from ~Commands menu~@CmdMenu@ or using
#Ctrl-W#. The keyboard shortcut #Ctrl-W# can also be used in the viewer or the
editor.

    #Ctrl-R# allows to refresh the task list.


@HotPlugList
$ #Hotplug devices list#
    Hotplug devices list displays PC Card boards and other analogue devices
which are installed and work in the computer.

    To remove a device you need to select it in the list and press the #Del#
key. After that Windows will prepare the device for safe removal and a
notification will be displayed when it is safe to remove the device.

    #Ctrl-R# allows to refresh the list of connected devices.


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
contains a number of user defined commands and command sequences, which may be
executed when invoking the user menu. The menu may contain submenus.
~Special symbols~@MetaSymbols@ are supported both in the commands and in the command
titles. Note, that !?<title>?<init>! symbol may be used to enter additional
parameters directly before executing commands.

    With the #Edit user menu# command from the ~Commands menu~@CmdMenu@, you
may edit or create main or local user menu. There may only be one main user
menu. The main user menu is called if no local menu for the current folder is
available. The local menu may be placed in any folder. You may switch between
the main menu and the user menu by pressing #Shift-F2#. Also you may call the
user menu of the parent folder by pressing #BS#.

    You may add command separators to the user menu. To do this, you should add
a new menu command, define "#-#" as "hot key" and leave field "Label" empty. To
delete a menu separator, you must switch to file mode with #Alt-F4# key.

    To execute a user menu command, select it with cursor keys and press #Enter#.
You may also press the hot key assigned to the required menu item.

    You may delete a submenu or menu item with the #Del# key, insert new
submenu or menu item with the #Ins# key or edit an existing submenu or menu
item with the #F4# key. Press #Alt-F4# to edit the menu in text file form.

    It is possible to use digits, letters and function keys (#F1#..#F12#) as
hot keys in user menu. If #F1# or #F4# is used, its original function in user
menu is overridden. However, you still may use #Shift-F4# to edit the menu.

    When you edit or create a menu item, you should enter the hot key for fast
item access, the item title which will be displayed in the menu and the command
sequence to execute when this item will be selected.

    When you edit or create a submenu, you should enter the hot key and the
item title only.

    Local user menus are stored in the text files #FarMenu.Ini#. The main menu,
by default, is stored in the registry, but it is possible to store it in a
file. If you create a local menu in the FAR folder, it will be used instead of
the main menu saved in the registry.

    To close the menu even if submenus are open use #Shift-F10#.


@FileAssoc
$ #File associations #
    FAR Manager supports file associations, that allow to associate various
actions to running, viewing and editing files with a specified
~mask~@FileMasks@.

    You may add new associations with the #Edit associations# command in the
~Commands menu~@CmdMenu@.

    You may define several associations for one file type and select the
desired association from the menu.

    The following actions are available in the associations list:

    #Ins#        - ~add~@FileAssocModify@ a new association

    #F4#         - ~edit~@FileAssocModify@ the current association

    #Del#        - delete the current association

    If no execute command is associated with file and
#Use Windows registered types# option in ~System settings~@SystemSettings@
is on, FAR tries to use Windows association to execute this file type;


@FileAssocModify
$ #File associations: editing#
    FAR allows to specify six commands associated with each file type specified
as a ~mask~@FileMasks@:

   #Execute command#               Performed if #Enter# is pressed
   #(used for Enter)#

   #Execute command#               Performed if #Ctrl-PgDn# is pressed
   #(used for Ctrl-PgDn)#

   #View command#                  Performed if #F3# is pressed
   #(used for F3)#

   #View command#                  Performed if #Alt-F3# is pressed
   #(used for Alt-F3)#

   #Edit command#                  Performed if #F4# is pressed
   #(used for F4)#

   #Edit command#                  Performed if #Alt-F4# is pressed
   #(used for Alt-F4)#

    The association can be described in the #Description of the association#
field.

    If you do not wish to switch panels off before executing the associated
program, start its command line with '#@@#' character.

    The following ~special symbols~@MetaSymbols@ may be used in the associated
command.

  Notes:

  1. If no execute command is associated with file and
#Use Windows registered types# option in ~System settings~@SystemSettings@
is on, FAR tries to use Windows association to execute this file type;

  2. Operating system ~commands~@OSCommands@ "IF EXIST" and "IF DEFINED"
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
             after performing the command, FAR will restore it
    #!@@!#      Name of file with selected file names list
    #!$!#      Name of file with selected short file names list
    #!&#       List of selected files
    #!&~~#      List of selected short file names
    #!:#       Current drive in the format "C:"
             For remote folders - "\\\\server\\share"
    #!\\#       Current path
    #!/#       Short name of the current path
    #!=\\#      Current path considering ~symbolic links~@HardSymLink@.
    #!=/#      Short name of the current path considering
             ~symbolic links~@HardSymLink@.

    #!?<title>?<init>!#
             This symbol is replaced by user input, when
             executing command. <title> and <init> - title
             and initial text of edit control.

             Several such symbols are allowed in the same line,
             for example:

             grep !?Search for:?! !?In:?*.*!|c:\\far\\far.exe -v -

             A history name for the <init> string can be supplied
             in the <title>. In such case the command has the
             following format:

             #!?$<history>$<title>?<init>!#

             for example:

             grep !?#$GrepHist$#Search for:?! !?In:?*.*!|far.exe -v -

             In <title> and <init> the usage of other meta-symbols is
             allowed by enclosing them in brackets.

             (e.g. grep !?Find in (!.!):?! |far.exe -v -)

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

    1. When handling special characters, FAR substitutes only the string
corresponding to the special character. No additional characters (for example,
quotes) are added, and you should add them yourself if it is needed. For
example, if a program used in the associations requires a file name to be
enclosed in quotes, you should specify #program.exe "!.!"# and not
#program.exe !.!#.

    2. The following modifiers may be used with the associations !@@! and !$! :

     'Q' - enclose names containing spaces in quotes;
     'S' - use '/' instead of '\\' in pathnames;
     'F' - use full pathnames;
     'A' - use ANSI code page;
     'U' - use UTF-8 code page;
     'W' - use UTF-16 (Little endian) code page.

    For example, the association #!@@AFQ!# means "name of file with the list of
selected file names, in ANSI encoding, include full pathnames, names with
spaces will be in quotes".

    3. When there are multiple associations specified, the meta-characters !@@!
and !$! are shown in the menu as is. Those characters are translated when the
command is executed.

    4. The prefixes "!##" and "!^" work as toggles for associations. The effect
of these prefixes continues up to the next similar prefix. For example:

    if exist !##!\\!^!.! diff -c -p !##!\\!^!.! !\\!.!

  "If the same file exists on the passive panel as the file under
   the cursor on the active panel, show the differences between
   the file on the passive panel and the file on the active panel,
   regardless of the name of the current file on the passive panel"

    5. If it is needed to pass to a program a name with an ending
backslash, use the following meta-symbol - #!.\#. For example, to
extract a rar archive to a folder with the same name

    winrar x "!.!" "!.\"

@SystemSettings
$ #Settings dialog: system#
  #Clear R/O attribute#     Clear read-only attribute from files copied
  #from CD files#           from CD.

  #Delete to Recycle Bin#   Enables file deletion via the Recycle Bin.
                          The operation of deleting to the Recycle
                          Bin can be performed only for local hard
                          disks.

  #Delete symbolic links#   Scan for and delete symbolic links to
                          subfolders before deleting to Recycle Bin.

  #Use system copy#         Use the file copy functions provided by
  #routine#                 the operating system instead of internal
                          file copy implementation. It may be useful
                          on NTFS, because the system function
                          (CopyFileEx) performs a more rational disk
                          space allocation and copies file extended
                          attributes. On the other hand, when using
                          the system function, the possibility to
                          split files when ~copying~@CopyFiles@ or moving is not
                          available.

  #Copy files opened#       Allows to copy files that are opened
  #for writing#             by other programs for writing. This mode
                          is handy to copy a file opened for a long
                          time, but it could be dangerous, if a file
                          is being modified at the same time as
                          copying.

  #Scan symbolic links#     Scan ~symbolic links~@HardSymLink@ along with normal
                          sub-folders when building the folder tree,
                          determining the total file size in the
                          sub-folders.

    Remark: if the file system contains recursive cycles of symbolic
links (e.g. a symlink pointing to one of its parent folders), then the
scanning will be conducted until the maximum allowed path length is
reached. In this case scanning may take much more time.


  #Create folders#          If the name of a new folder contains only
  #in uppercase#            lowercase letters and this option is on,
                          the folder will be created in uppercase.

  #Inactivity time#         Terminate FAR after a specified interval
                          without keyboard or mouse activity. This
                          works only if FAR waits for command line
                          input without viewer or editor screens
                          in the background.

  #Save commands history#   Forces saving ~commands history~@History@ before exit
                          and restoring after starting FAR.

  #Save folders history#    Forces saving ~folders history~@HistoryFolders@ before exit
                          and restoring after starting FAR. Folders
                          history list may be activated by #Alt-F12#.

  #Save view and edit#      Forces saving ~history of viewed and edited~@HistoryViews@
  #history#                 files before exit and restoring it after
                          starting FAR. View and edit history list
                          may be activated by #Alt-F11#.

  #Use Windows#             When this option is on and #Enter# is pressed
  #registered types#        on a file, the type of which is known to
                          Windows and absent in the list of FAR
                          ~file associations~@FileAssoc@, the Windows program
                          registered to process this file type
                          will be executed.

  #CD drive auto mount#     When a CD-ROM drive is selected from the
                          ~drive menu~@DriveDlg@, FAR will close the open
                          tray of a CD drive. Turn off this option
                          if automatic CD-ROM mounting does not work
                          correctly (this can happen because of bugs
                          in the drivers of some CD-ROM drives).

  #Path for#                Enter here the full path, where FAR will search
  #personal plugins#      for "personal" plugins in addition to the "main"
                          plugins. Several search paths may be given separated
                          by ';'. Environment variables can be entered in the
                          search path. Personal plugins will not be loaded, if
                          the switches /p or /co are given in the
                          ~command line~@CmdLine@.

  #Auto save setup#         If checked, FAR will save setup
                          automatically. The current folders for
                          both panels will be also saved.


@PanelSettings
$ #Settings dialog: panel#
  #Show hidden and#         Enable to show files with Hidden
  #system files#            and System attributes. This option may
                          also be switched by #Ctrl-H#.

  #Highlight files#         Enable ~files highlighting~@Highlight@.

  #Auto change folder#      If checked, cursor moves in the ~tree panel~@TreePanel@
                          will cause a folder change in the other
                          panel. If it is not checked, you must press
                          #Enter# to change the folder from the tree
                          panel.

  #Select folders#          Enable to select folders, using #Gray +#
                          and #Gray *#. Otherwise these keys will
                          select files only.

  #Sort folder names#       Apply sorting by extension not only
  #by extension#            to files, but also to folders.
                          When the option is turned on, sorting
                          by extension works the same as it did
                          in FAR 1.65. If the option is turned
                          off, in the extension sort mode the
                          folders will be sorted by name, as
                          they are in the name sort mode.

  #Allow reverse#           If this option is on and the current file
  #sort modes#              panel sort mode is reselected, reverse
                          sort mode will be set.

  #Disable automatic#       The mechanism for automatically updating
  #update of panels#        the panels when the state of the file
                          system changes will be disabled if the
                          count of file objects exceeds the
                          specified value.

    The auto-update mechanism works only for FAT/FAT32/NTFS file
    systems. The value of 0 means "update always". To force an
    update of the panels, press #Ctrl-R#.

  #Network drives#          This option enables panel autorefresh
  #autorefresh#             when state of filesystem on network
                          drives is being changed. It can be useful
                          to disable this option on slow network
                          connections

  #Show column titles#      Enable display of ~file panel~@FilePanel@ column titles.

  #Show status line#        Enable display of file panel status line.

  #Show total#              Enable display of total information data
  #information#             at the bottom line of file panel.

  #Show free size#          Enable display of the current disk free
                          size.

  #Show scrollbar#          Enable display of file and ~tree panel~@TreePanel@
  #in Panels#               scrollbars.

  #Show background#         Enable display of the number of
  #screens number#          ~background screens~@ScrSwitch@.

  #Show sort mode#          Indicate current sort mode in the
  #letter#                  upper left panel corner.


@InterfSettings
$ #Settings dialog: interface#
  #Clock in panels#         Show clock at the right top corner of
                          the screen.

  #Clock in viewer#         Show clock in viewer and editor.
  #and editor#

  #Mouse#                   Use the mouse.

  #Show key bar#            Show the functional key bar at the bottom
                          line. This option also may be switched
                          by #Ctrl-B#.

  #Always show menu bar#    Show top screen menu bar even when
                          it is inactive.

  #Screen saver#            Run screen saver after the inactivity
                          interval in minutes. When this option
                          is enabled, screen saver will also
                          activate when mouse pointer is brought
                          to the upper right corner of FAR window.

  #Set command line#        This option allows to set the default
  #prompt format#           FAR command ~line prompt~@CommandPrompt@.

  #Show total copy#         Show total progress bar, when performing
  #indicator#               a file copy operation. This may require
                          some additional time before starting
                          copying to calculate the total files size.

  #Show copying time#       Show information about average copying
  #information#             speed, copying time and estimated remaining
                          time in the copy dialog.

    Since this function requires some time to gather statistics,
    it is likely that you won't see any information if you're
    copying many small files and the option "Show total copy
    progress indicator" is disabled.

  #Use Ctrl-PgUp#           Pressing #Ctrl-PgUp# in the root directory:
  #to change drive#         - for local drives - shows the drive
                            selection menu;
                          - for network drives - activates the
                            Network plugin (if it is available)
                            or the drive selection menu (if the
                            Network plugin is not available).

@DialogSettings
$ #Settings dialog: settings#
  #History in dialog#       Keep history in edit controls of some
  #edit controls#           FAR dialogs. The previous strings history
                          list may be activated by mouse or using
                          #Ctrl-Up# and #Ctrl-Down#. If you do not wish
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
                          disabled, you may use the #Ctrl-End# key
                          to autocomplete a line. The autocomplete
                          feature is disabled while a macro is
                          being recorded or executed.

  #Backspace deletes#       If the option is on, pressing #BackSpace#
  #unchanged text#          in an unchanged edit string deletes
                          the entire text, as if #Del# had been
                          pressed.

  #Mouse click outside#     #Right/left mouse click# outside a dialog
  #a dialog closes it#      closes the dialog (see ~Miscellaneous~@MiscCmd@).
                          This option allows to switch off this
                          functionality.


@CommandPrompt
$ #Command line prompt format#
   FAR allows to change the command line prompt format.
To change it you have to enter the needed sequence of variables and
special code words in the #Set command line prompt format# input field
of the ~Interface settings~@InterfSettings@ dialog, this will allow showing
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
     $q - the = character
     $s - space
     $t - current time in HH:MM:SS format
     $$ - the $ character
     $+ - глубина стека каталогов

   By default the #$p$g# sequence is used - current drive and
path ("C:\>").

   Examples.

   1. ^<wrap>A prompt of the following format #[%COMPUTERNAME%]$S$P$G#
will contain the computer name, current drive and path
(the %COMPUTERNAME% environment variable must be defined)

   2. ^<wrap>A prompt of the following format #[$T$H$H$H]$S$P$G# will
display the current time in HH:MM format before the current
drive and path

   3. ^<wrap>Код "$+" отображает нужное число знаков плюс (+) в зависимости
от текущей глубины стека каталогов ~PUSHD~@OSCommands@, по одному знаку на
каждый сохраненный путь.

@Viewer
$ #Viewer: control keys#
   Viewer commands

    #Left#               Character left
    #Right#              Character right
    #Up#                 Line up
    #Down#               Line down
    #Ctrl-Left#          20 characters left
                       In Hex-mode - 1 position left
    #Ctrl-Right#         20 characters right
                       In Hex-mode - 1 position right
    #PgUp#               Page up
    #PgDn#               Page down
    #Ctrl-Shift-Left#    Start of lines on the screen
    #Ctrl-Shift-Right#   End of lines on the screen
    #Home, Ctrl-Home#    Start of file
    #End, Ctrl-End#      End of file

    #F1#                 Help
    #F2#                 Toggle line wrap/unwrap
    #Shift-F2#           Toggle wrap type (letters/words)
    #F4#                 Toggle text/hex mode
    #F6#                 Switch to ~editor~@Editor@
    #Alt-F5#             Print the file
                       ("Print manager" plugin is used).
    #F7#                 ~Search~@ViewerSearch@
    #Shift-F7, Space#    Continue search
    #Alt-F7#             Continue search in "reverse" mode
    #F8#                 Toggle OEM/ANSI code page
    #Shift-F8#           Select code page
    #Alt-F8#             ~Change current position~@ViewerGotoPos@
    #Alt-F9#             Toggles the size of the FAR console window
    #Alt-Shift-F9#       Call ~Viewer settings~@EditorSettings@ dialog
    #Numpad5,F3,F10,Esc# Quit
    #Ctrl-F10#           Position to the current file.
    #F11#                Call "~Plugin commands~@Plugins@" menu
    #Alt-F11#            Display ~view history~@HistoryViews@
    #+#                  Go to next file
    #-#                  Go to previous file
    #Ctrl-O#             Show user screen
    #Ctrl-Alt-Shift#     Temporarily show user screen
                       (as long as these keys are held down)
    #Ctrl-B#             Show/Hide functional key bar at the bottom
                       line.
    #Ctrl-Shift-B#       Show/Hide status line
    #Ctrl-S#             Show/Hide the scrollbar.
    #Alt-BS, Ctrl-Z#     Undo position change
    #RightCtrl-0..9#     Set a bookmark 0..9 at the current position
    #Ctrl-Shift-0..9#    Set a bookmark 0..9 at the current position
    #LeftCtrl-0..9#      Go to bookmark 0..9

    #Ctrl-Ins, Ctrl-C#   Copy the text highlighted as a result of
                       the search to the clipboard.
    #Ctrl-U#             Remove the highlighting of the search results.

    См. так же список ~макроклавиш~@KeyMacroViewerList@, доступных в программе просмотра.

    Notes:

    1. Also to call search dialog you may just start typing the
       text to be located.

    2. When the viewer opens a file, it permits the file to be
       deleted by other processes. If such a deletion happens,
       the file is actually deleted from the directory only after
       the viewer is closed, but any operations on the deleted
       file fail - this is a Windows feature.

    3. The current version of FAR has a limitation on the maximum
       number of columns in the internal viewer - the number
       cannot exceed 2048. If a file contains a line that does not
       fit in this number of columns, it will be split into several
       lines, even if the word wrap mode is turned off.

    4. FAR ~searches~@ViewerSearch@ the first occurrence of the string (#F7#) from
       the beginning of the area currently displayed.

    5. For automatic scrolling of a dynamically updating file,
       position the "cursor" to the end of the file (End key).


@ViewerGotoPos
$ #Viewer: go to specified position#
    This dialog allows to change the position in the internal viewer.

    You can enter decimal offset, percent, or hexadecimal offset. The meaning
of the value you enter is defined either by the radio buttons in the dialog or
by format specifiers added before or after the value.

    You can also enter relative values, just add + or - before the number.

    Hexadecimal offsets must be specified in one of the following formats:
       0xNNNN, NNNNh, $NNNN

    Decimal offsets (not percentages) must be specified in the format NNNNd.

  Examples
   #50%#                     Go to middle of file (50%)
   #-10%#                    Go to 10% percent back from current offset
                           If the old position was 50%, the new
                           position will be 40%
   #0x100#                   Go to offset 0x100 (256)
   #+0x300#                  Go 0x300 (768) bytes forward

  If you enter the value with one of the format specifiers (%, '0x', 'h',
'$', 'd'), the radio buttons selected in the dialog will be ignored.


@ViewerSearch
$ #Viewer: search#
    For searching in the ~viewer~@Viewer@, the following modes and options are
available:

    #Search for text#

      Search for any text entered in the #Search for# edit line.
      The following options are available in that mode:

        #Case sensitive#      - the case of the characters
                              entered will be taken into account
                              while searching (so, for example,
                              #Text# will not be found when
                              searching for #text#).

        #Whole words#         - the given text will be found only
                              if it occurs in the text as a whole
                              word.

    #Search for hex#

      Search for a string corresponding to hexadecimal codes
      entered in the #Search for# string.

    #Reverse search#

      Reverse the search direction - search from the end of file
      towards the beginning.


@Editor
$ #Editor#
    To edit the file currently under the cursor you should press #F4#. This
can be used to open the internal editor or any of the user defined external
editors which are defined in the ~Editor settings~@EditorSettings@ dialog.

    #Creating files using the editor#

    If a nonexistent file name is entered after pressing the #Shift-F4# hotkey
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
   #Ctrl-S#                  ^<wrap>Move the cursor one character to the left, but don't move to the previous line if the line beginning is reached.
   #Right#                   Character right
   #Up#                      Line up
   #Down#                    Line down
   #Ctrl-Left#               Word left
   #Ctrl-Right#              Word right
   #Ctrl-Up#                 Scroll screen up
   #Ctrl-Down#               Scroll screen down
   #PgUp#                    Page up
   #PgDn#                    Page down
   #Home#                    Start of line
   #End#                     End of line
   #Ctrl-Home, Ctrl-PgUp#    Start of file
   #Ctrl-End, Ctrl-PgDn#     End of file
   #Ctrl-N#                  Start of screen
   #Ctrl-E#                  End of screen

  Delete operations

   #Del#                     ^<wrap>Delete char (also may delete block, depending upon ~Editor settings~@EditorSettings@).
   #BS#                      Delete char left
   #Ctrl-Y#                  Delete line
   #Ctrl-K#                  Delete to end of line
   #Ctrl-BS#                 Delete word left
   #Ctrl-T, Ctrl-Del#        Delete word right

  Block operations

   #Shift-Cursor keys#       Select block
   #Ctrl-Shift-Cursor keys#  Select block
   #Alt-gray cursor keys#    Select vertical block
   #Alt-Shift-Cursor keys#   Select vertical block
   #Ctrl-Alt-gray keys#      Select vertical block
   #Ctrl-A#                  Select all text
   #Ctrl-U#                  Deselect block
   #Shift-Ins, Ctrl-V#       Paste block from clipboard
   #Shift-Del, Ctrl-X#       Cut block
   #Ctrl-Ins, Ctrl-C#        Copy block to clipboard
   #Ctrl-<Gray +>#           Append block to clipboard
   #Ctrl-D#                  Delete block
   #Ctrl-P#                  ^<wrap>Copy block to current cursor position (in persistent blocks mode only)
   #Ctrl-M#                  ^<wrap>Move block to current cursor position (in persistent blocks mode only)
   #Alt-U#                   Shift block left
   #Alt-I#                   Shift block right

  Other operations

   #F1#                      Help
   #F2#                      Save file
   #Shift-F2#                ~Save file as...~@FileSaveAs@
   #Shift-F4#                Edit ~new file~@FileOpenCreate@
   #Alt-F5#                  ^<wrap>Print file or selected block ("Print manager" plugin is used).
   #F6#                      Switch to ~viewer~@Viewer@
   #F7#                      Search
   #Ctrl-F7#                 Replace
   #Shift-F7#                Continue search/replace
   #Alt-F7#                  Continue search/replace in "reverse" mode
   #F8#                      Toggle OEM/ANSI code page
   #Shift-F8#                Select code page
   #Alt-F8#                  ~Go to~@EditorGotoPos@ specified line and column
   #Alt-F9#                  Toggles the size of the FAR console window
   #Alt-Shift-F9#            Call ~Editor settings~@EditorSettings@ dialog
   #F10, Esc#                Quit
   #Shift-F10#               Save and quit
   #Ctrl-F10#                Position to the current file
   #F11#                     Call "~Plugin commands~@Plugins@" menu
   #Alt-F11#                 Display ~edit history~@HistoryViews@
   #Alt-BS, Ctrl-Z#          Undo
   #Ctrl-Shift-Z#            Redo
   #Ctrl-L#                  Disable edited text modification
   #Ctrl-O#                  Show user screen
   #Ctrl-Alt-Shift#          ^<wrap>Temporarily show user screen (as long as these keys are held down)
   #Ctrl-Q#                  ^<wrap>Treat the next key combination as a character code
   #RightCtrl-0..9#          ^<wrap>Set a bookmark 0..9 at the current position
   #Ctrl-Shift-0..9#         ^<wrap>Set a bookmark 0..9 at the current position
   #LeftCtrl-0..9#           Go to bookmark 0..9
   #Shift-Enter#             ^<wrap>Insert the name of the current file on the active panel at the cursor position.
   #Ctrl-Shift-Enter#        ^<wrap>Insert the name of the current file on the passive panel at the cursor position.
   #Ctrl-F#                  ^<wrap>Insert the full name of the file being edited at the cursor position.
   #Ctrl-B#                  ^<wrap>Show/Hide functional key bar at the bottom line.
   #Ctrl-Shift-B#            Show/Hide status line

   См. так же список ~макроклавиш~@KeyMacroEditList@, доступных в редакторе.

    Notes:

    1. #Alt-U#/#Alt-I# indent the current line if no block is selected.

    2. ^<wrap>Holding down #Alt# and typing a character code on the numeric
keypad inserts the character that has the specified code (0-65535).

    3. ^<wrap>If no block is selected, #Ctrl-Ins#/#Ctrl-C# marks the current
line as a block and copies it to the clipboard.


@FileOpenCreate
$ #Редактор: Открыть/создать файл#
    С помощью комбинации #Shift-F4# можно открыть существующий или
создать новый файл.

    В зависимости от ~настроек редактора~@EditorSettings@ для нового файла
выбирается кодовая страница OEM или ANSI. При необходимости из #списка#
можно выбрать другую кодовую страницу.

    Для существующего файла изменять опцию #Кодовая страница:# имеет
смысл тогда, когда при открытии она определилась неправильно.


@FileSaveAs
$ #Editor: save file as...#
    Редактируемый файл можно сохранить под другим именем - нажать #Shift-F2# и
указать другое имя, кодовую страницу и формат представления символа перевода
строки.

    Если для сохраняемого файла выбрана одна из кодовых страниц: UTF-8,
UNICODE или REVERSEBOM, то при включенной опции #Добавить сигнатуру (BOM)#
в начало файла добавляется специальный маркер, позволяющий другим приложениям
однозначно идентифицировать этот файл.

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
$ #Editor: go to specified line and column#
    This dialog allows to change the position in the internal editor.

    You can enter a #row# or a #column#, or both.

    The first number will be interpreted as a row number, the second as a
column number. Numbers must be delimited by one of the following characters:
"," "." ";" ":" or space.

    If you enter the value in the form ",Col", the editor will jump to the
specified column in the current line.

    If you enter the row with "%" at the end, the editor will jump to the
specified percent of the file. For example, if you enter #50%#, the editor will
jump to the middle of the text.


@EditorReload
$ #Editor: reloading a file#
    FAR Manager tracks all attempts to repeatedly open for editing a file that
is already being edited. The rules for reloading files are as follows:

    1. If the file was not changed and the option "Reload edited file" in the
~confirmations~@ConfirmDlg@ dialog is not enabled, FAR switches to the open
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
a folder that does not exist. Before saving the file, FAR will create the
folder, provided that the path is correct (for example, a path starting with a
non-existing drive letter would not be correct) and that you have enough rights
to create the folder.

@WarnEditorPluginName
$ #Warning: The name of the file to edit cannot be empty#
    To create a new file on a plugin's panel you must specify a
file name.

@WarnEditorSavedEx
$ #Warning: The file was changed by an external program#
    The modification date and time of the file on the disk are not the same as
those saved by FAR when the file was last accessed. This means that another
program, another user (or even yourself in a different editor instance) changed
the contents of the file on the disk.

    If you press "Save", the file will be overwritten and all changes made by
the external program will be lost.

@CodePagesMenu
$ #Меню выбора кодовой страницы#
    Это меню позволяет выбрать кодовую страницу редактора или программы просмотра.

    Меню разделено на несколько частей:

    #Автоматическое определение# - определение правильной кодовой страницы;

    #Системные# - основные однобайтные системные кодовые страницы - ASNI и OEM;

    #Юникод# - юникодные кодовые страницы;

    #Избранные# - кодовые страницы, отмеченные пользователем;

    #Прочие# - все остальные установленные в системе кодовые страницы.

    Меню имеет два режима - полный, в котором раздел #Прочие# показывается, и
сокращённый, в котором он скрыт. Переключение режимов осуществляется сочетанием клавиш
#Ctrl-H#.

    Клавиша #Ins# перемещает кодовую страницу из раздела #Прочие# в раздел #Избранные#.
Клавиша #Del# производит обратное действие.

@DriveDlg
$ #Change drive#
    This menu allows to change the current drive of a panel, disconnect from a
network drive or open a new ~plugin~@Plugins@ panel.

    Select the item with the corresponding drive letter to change the drive or
the item with the plugin name to create a new plugin panel. If the panel type
is not a ~file panel~@FilePanel@, it will be changed to the file panel.

    #Del# key can be used:

     - to ~disconnect~@DisconnectDrive@ from network drives.

     - to delete a substituted (virtual) drive.

     - to eject disks from CD-ROM and removable drives.
       Ejecting a disk from a ZIP-drive requires administrative privileges.
       A CD-ROM can be closed by pressing #Ins#.

    The #Shift-Del# hotkey is used to prepare a USB storage device for safe
removal. If the disk, for which the removal function is used, is a flash-card
inserted into a card-reader that supports several flash-cards then the
card-reader itself will be stopped.

    #Ctrl-1# - #Ctrl-9# switch the display of different information:

    Ctrl-1 - disk type;
    Ctrl-2 - network name (and the path associated with a SUBST drive);
    Ctrl-3 - disk label;
    Ctrl-4 - file system;
    Ctrl-5 - total and free disk size
             (this option has two display modes, press twice to see);
    Ctrl-6 - display of removable disk parameters;
    Ctrl-7 - display of plugin items;
    Ctrl-8 - display of CD parameters;
    Ctrl-9 - display of network parameters.

    #Change drive# menu settings are saved in the FAR configuration.

    If the option "~Use Ctrl-PgUp to change drive~@InterfSettings@" is enabled,
pressing #Ctrl-PgUp# works the same as pressing #Esc# - cancels drive selection
and closes the menu.

    Pressing #Shift-Enter# invokes the Windows Explorer showing the root
directory of the selected drives (works only for disk drives and not for
plugins).

    #Ctrl-R# allows to refresh the disk selection menu.

    If "#CD drive type#" mode is enabled (#Ctrl-8#), FAR will attempt to
determine the type of each of the CD drives available in the system. Known
types are as follows: CD-ROM, CD-RW, CD-RW/DVD, DVD-ROM, DVD-RW and DVD-RAM.
This function is available only for users either with administrative privileges
or all local users, when it's stated explicitly in the Local Policy Editor
(to do this, run a #secpol.msc# from the command prompt, and set the '#Local#
#Policies/Security Options/Devices: Restrict CD-ROM access to locally logged-on#
#user only#' setting to '#Enabled#').

    #Alt-Shift-F9# allows you to ~configure plugins~@PluginsConfig@ (it works only if
display of plugin items is enabled).

    #Shift-F9# in the plugins list opens the configuration dialog of the
currently selected plugin.

    #Shift-F1# in the plugins list displays the context-sensitive help of the
currently selected plugin, if the plugin has a help file.

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
panels, FAR Manager has the possibility of color highlighting for file objects.
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

  #Ctrl-R#       - Restore the default file highlighting groups

  #Ctrl-Up#      - Move a group up.

  #Ctrl-Down#    - Move a group down.

    The highlighting groups are checked from top to bottom. If it is detected
that a file belongs to a group, no further groups are checked.


@HighlightEdit
$ #Files highlighting and sort groups: editing#
    The #Files highlighting# dialog in the ~Options menu~@OptMenu@ allows to
define file highlighting groups. Each group definition includes:

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
       It may be used both with or instead of color highlighting.

    If the option "A file mask or several file masks" is turned off, file masks
will not be analyzed, and only file attributes will be taken into account.

    A file belongs to a highlighting group if:

     - file mask analysis is enabled and the name of the file matches
       at least one file mask (if file mask analysis is disabled,
       the file name doesn't matter);

     - it has all of the included attributes;

     - it has none of the excluded attributes.

    The Compressed, Encrypted, Not indexed, Sparse, Temporary attributes and
Symbolic links are valid for NTFS drives only.


@ViewerSettings
$ #Settings dialog: viewer#
    This dialog allows to change the default external or
~internal viewer~@Viewer@ settings.

    External viewer

  #Use for F3#              Run external viewer using #F3#.

  #Use for Alt-F3#          Run external viewer using #Alt-F3#.

  #Viewer command#          Command to execute external viewer.
                          Use ~special symbols~@MetaSymbols@ to specify the
                          name of the file to view. If you do
                          not wish to switch panels off before
                          executing the external viewer, start
                          command from '@@' character.

    Internal viewer

  #Save file position#      Save and restore positions in the recently
                          viewed files. This option also forces
                          restoring of code page, if the page
                          was manually selected by user, and the file
                          viewing mode (normal/hex).

  #Save bookmarks#          Save and restore bookmarks (current
                          positions) in recently viewed files
                          (created with #RightCtrl-0..9# or
                          #Ctrl-Shift-0..9#)

  #Auto detect#             ~Auto detect~@CodePage@ the code page of
  #code page#               the file being viewed.

  #Tab size#                Number of spaces in a tab character.

  #Show scrollbar#          Show scrollbar in internal viewer. This
                          option can also be switched by pressing
                          #Ctrl-S# in the internal viewer.

  #Show arrows#             Show scrolling arrows in viewer if the text
                          doesn't fit in the window horizontally.

  #Persistent selection#    Do not remove block selection after
                          moving the cursor.

  #Use ANSI code page#      Use ANSI code page for viewing files,
  #by default#              instead of OEM.

    If the external viewer is assigned to #F3# key, it will be executed only if
the ~associated~@FileAssoc@ viewer for the current file type is not defined.

    Modifications of settings in this dialog do not affect previously opened
internal viewer windows.

    The settings dialog can also be invoked from the ~internal viewer~@Viewer@
by pressing #Alt-Shift-F9#. The changes will come into force immediately but
will affect only the current session.


@EditorSettings
$ #Settings dialog: editor#
    This dialog allows to change the default external and
~internal editor~@Editor@ settings.

    External editor

  #Use for F4#              Run external editor using #F4# instead of
                          #Alt-F4#.

  #Editor command#          Command to execute the external editor.
                          Use ~special symbols~@MetaSymbols@ to specify the name
                          of the file to edit. If you do not wish
                          to switch panels off before executing
                          the external editor, start the command with
                          the '@@' character.


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
                          (created with #RightCtrl-0..9# or
                          #Ctrl-Shift-0..9#)

  #Auto indent#             Enables auto indent mode when entering
                          text.

  #Cursor beyond#           Allow moving cursor beyond the end of line.
  #end of line#

  #Tab size#                Number of spaces in a tab character.

  #Show scrollbar#          Show scrollbar.

  #Auto detect#             ~Auto detect~@CodePage@ the code page of
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
                          text, just as if #Ctrl-L# was pressed.

  #Warn when opening#       When a file with the Read-only attribute
  #read-only files#         is opened for editing, a warning message
                          will be shown.

  #Use ANSI code page#      Use ANSI code page for editing files,
  #by default#              instead of OEM.

  #Use ANSI code page#      Use ANSI code page when creating new files,
  #when creating new files# instead of OEM.

    If the external editor is assigned to #F4# key, it will be executed only if
~associated~@FileAssoc@ editor for the current file type is not defined.

    Modifications of settings in this dialog do not affect previously opened
internal editor windows.

    The settings dialog can also be invoked from the ~internal editor~@Editor@
by pressing #Alt-Shift-F9#. The changes will come into force immediately but
will affect only the current session.


@CodePage
$ #Auto detect code pages#
    FAR will try to choose the correct code page for viewing/editing a file.
Note that correct detection is not guaranteed, especially for small or
non-typical text files.


@FileAttrDlg
$ #File attributes dialog#
    With this command it is possible to change file attributes and file time.
Either single file or group of files may be processed. If you do not want to
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

    The "Compressed", "Encrypted", "Not indexed", "Sparse", "Temporary",
"Offline" and "Virtual" attributes are available only on NTFS drives. The
"Virtual" attribute can be used in Windows Vista/2008/7 only. The "Compressed"
and "Encrypted" attributes are mutually exclusive, that is, you can set only
one of them. You cannot clear the "Sparse" attribute in Windows 2000/XP/2003.

    For ~folder links~@HardSymLink@ the dialog will display the original folder
information (NTFS only). If the information on the original folder is not
available (in particular, for symbolic links in remote folders), then the
"#(data not available)#" message will be shown.

  #File date and time#

    Three different file times are supported:

    - last modification time;

    - creation time;

    - last access time.

    On FAT drives the hours, minutes and seconds of the last access time are
always equal to zero.

    If you do not want to change the file time, leave the respective field
empty. You can push the #Blank# button to clear all the date and time fields
and then change an individual component of the date or time, for example, only
month or only minutes. All the other date and time components will remain
unchanged.

    The #Current# button fills the file time fields with the current time.

    The #Original# button fills the file time fields with their original
values. Available only when the dialog is invoked for a single file object.

    For ~symbolic links~@HardSymLink@ the date and time cannot be set.


@FolderShortcuts
$ #Folder shortcuts#
    Folder shortcuts are designed to provide fast access to frequently used
folders. Press #Ctrl-Shift-0..9#, to create a shortcut to the current folder.
To change to the folder recorded in the shortcut, press #RightCtrl-0..9#. If
#RightCtrl-0..9# pressed in edit line, it inserts the shortcut path into the
line.

    The #Show folder shortcuts# item in the ~Commands menu~@CmdMenu@ may be
used to view, set, edit and delete folder shortcuts.

    When you are editing a shortcut (#F4#), you cannot create a shortcut to a
plugin panel.

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

   #Ctrl-Up#    Move a filter one position up.

   #Ctrl-Down#  Move a filter one position down.


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

   #Shift-Backspace#     Clear selection from all items.


    Filters selection is stored in the FAR configuration.

    When a filter is used in a panel, it is indicated by '*' after the sort
mode letter in the upper left corner of the panel.

    Filters menu is used in the following areas:
     - ~File panel~@FilePanel@;
     - ~Copying, moving, renaming and creating links~@CopyFiles@;
     - ~Find file~@FindFile@.


@FileDiz
$ #File descriptions#
    File descriptions may be used to associate text information with a file.
Descriptions of the files in the current folder are stored in this folder in a
description list file. The format of the description file is the file name
followed by spaces and the description.

    Descriptions may be viewed in the appropriate file panel
~view modes~@PanelViewModes@. By default these modes are #Descriptions#
and #Long descriptions#.

    The command #Describe# (#Ctrl-Z#) from the ~Files menu~@FilesMenu@ is used
to describe selected files.

    Description list names may be changed using #File descriptions# dialog from
the ~Options menu~@OptMenu@. In this dialog you can also set local descriptions
update mode. Updating may be disabled, enabled only if panel current view mode
displays descriptions or always enabled. By default FAR sets "Hidden" attribute
to created description lists, but you may disable it by switching off the
option "Set "Hidden" attribute to new description lists" in this dialog. Also
here you may specify the position to align new file descriptions in a
description list.

    If a description file has the "read-only" attribute set, FAR does not
attempt to update descriptions, and after moving or deleting file objects, an
error message is shown. If the option "#Update read only description file#" is
enabled, FAR will attempt to update the descriptions correctly.

    If it is enabled in the configuration, FAR updates file descriptions when
copying, moving and deleting files. But if a command processes files from
subfolders, descriptions in the subfolders are not updated.


@PanelViewModes
$ #Customizing file panel view modes#
    The ~file panel~@FilePanel@ can represent information using 10 predefined
modes: brief, medium, full, wide, detailed, descriptions, long descriptions,
file owners, file links and alternative full. Usually it is enough, but if you
wish, you may either customize its parameters or even replace them with
completely new modes.

    The command #File panel modes# from the ~Options menu~@OptMenu@ allows to
change the view mode settings. First, it offers to select the desired mode from
the list. In this list "Brief mode" item corresponds to brief panel mode
(#LeftCtrl-1#), "Medium" corresponds to medium panel mode (#LeftCtrl-2#) and so
on. The last item, "Alternative full", corresponds to view mode called with
#LeftCtrl-0#. After selecting the mode, you may change the following settings:

  - #Column types# - column types are encoded as one or several
characters, delimited with commas. Allowed column types are:

    N[M,O,R]   - file name
                 where: M - show selection marks;
                        O - show names without paths
                            (intended mainly for plugins);
                        R - right aligned names;
                 These modifiers may be used in combination,
                 for example NMR

    S[C,T,F,E] - file size
    P[C,T,F,E] - packed file size
    G[C,T,F,E] - size of file streams
                 where: C - format file size;
                        T - use 1000 instead of 1024 as a divider;
                        F - show file sizes similar to Windows
                            Explorer (i.e. 999 bytes will be
                            displayed as 999 and 1000 bytes will
                            be displayed as 0.97 K);
                        E - economic mode, no space between file
                            size and suffix will be shown
                            (i.e. 0.97K);

    D          - file modification date
    T          - file modification time

    DM[B,M]    - file modification date and time
    DC[B,M]    - file creation date and time
    DA[B,M]    - file last access date and time
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

    File attributes have the following indications:

       #R#         - Read only
       #S#         - System
       #H#         - Hidden
       #A#         - Archive
       #L#         - Junction or symbolic link
       #C# or #E#    - Compressed or Encrypted
       #$#         - Sparse
       #T#         - Temporary
       #I#         - Not content indexed
       #O#         - Offline
       #V#         - Virtual

    The attributes are displayed in the following order - RSHALCTIOV. The
"Sparse" attribute applies only to files and is shown instead of 'L'. The
"Encrypted" attribute is shown instead of 'C' as a file/folder can not
have both attributes ("Compressed" and "Encrypted") set at the same time.
By default the size of the attributes column is 6 characters. To display
the additional 'T', 'I', 'O' and 'V' attributes it is necessary to manually
set the size of the column to 10 characters.

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
the method of displaying files, when processing files FAR always uses the
real case.

  - #Use case sensitive sort# - use case sensitive sort for file names.


@SortGroups
$ #Sort groups#
    File sort groups may be used in #by name# and #by extension#
~file panel~@FilePanel@ sort modes. Sort groups are applied by
pressing #Shift-F11# and allow to define additional file sorting rules,
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
    File masks are frequently used in FAR commands to select single files and
folders or groups of them. Masks may contain common valid file name symbols,
wildcards ('*' and '?') and special expressions:

    #*#           zero or more characters;

    #?#           any single character;

    #[c,x-z]#     any character enclosed by the brackets.
                Both separate characters and character intervals
                are allowed.

    For example, files ftp.exe, fc.exe and f.ext may be selected using mask
f*.ex?, mask *co* will select both color.ini and edit.com, mask [c-f,t]*.txt
can select config.txt, demo.txt, faq.txt and tips.txt.

    In many FAR commands you may enter several file masks separated with commas
or semicolons. For example, to select all the documents, you can enter
#*.doc,*.txt,*.wri# in the "Select" command.

    It is allowed to put any of the masks in quotes but not the whole list. For
example, you have to do this when a mask contains any of the delimiter
characters (a comma or a semicolon), so that the mask doesn't get confused with
a list.

    In some commands (~find files~@FindFile@, ~filter~@Filter@,
~filters menu~@FiltersMenu@, file ~selection~@SelectFiles@,
file ~associations~@FileAssoc@ and
~file highlighting and sort groups~@Highlight@) you may use exclude masks. An
#exclude mask# is one or multiple file masks that must not be matched by the
files matching the mask. The exclude mask is delimited from the main mask by
the character '#|#'.

^Usage examples of exclude masks:
 1. *.cpp
    All files with the extension #cpp#.
 2. *.*|*.bak,*.tmp
    All files except for the files with extensions #bak# and #tmp#.
 3. *.*|
    This mask has an error - the character | is entered, but the
    mask itself is not specified.
 4. *.*|*.bak|*.tmp
    Also an error - the character | may not be contained in the mask
    more than once.
 5. |*.bak
    The same as *|*.bak

    The comma (or semicolon) is used for separating file masks from each other,
and the '|' character separates include masks from exclude masks.


@SelectFiles
$ #Selecting files#
    To process several ~file panel~@FilePanel@ files and folders, they may be
selected using different methods. #Ins# selects the file under the cursor and
moves the cursor down, #Shift-Cursor keys# move the cursor in different
directions.

    #Gray +# and #Gray -# perform group select and deselect, using one or more
~file masks~@FileMasks@ delimited by commas. #Gray *# inverts the current
selection. #Restore selection# command (#Ctrl-M#) restores the previously
selected group.

    #Ctrl-<Gray +># and #Ctrl-<Gray -># selects and deselects all files with
the same extension as the file under cursor.

    #Alt-<Gray +># and #Alt-<Gray -># selects and deselects all files with the
same name as the file under cursor.

    #Ctrl-<Gray *># inverts the current selection including folders. If #Select#
#folders# option in ~Panel settings~@PanelSettings@ dialog is on, it has the
same function as #Gray *#.

    #Shift-<Gray +># and #Shift-<Gray -># selects and deselects all files.

    If no files are selected, only the file under the cursor will be processed.


@CopyFiles
$ #Copying, moving, renaming and creating links#
    Following commands may be used to copy, move and rename files and folders:

  Copy ~selected~@SelectFiles@ files                                           #F5#

  Copy the file under cursor regardless of selection      #Shift-F5#

  Rename or move selected files                                 #F6#

  Rename or move the file under the cursor                #Shift-F6#
  regardless of selection

    For folders: if the specified path (absolute or relative) points to an
existing folder, the source folder is moved inside that folder. Otherwise the
folder is renamed/moved to the new path.
    E.g. when moving #c:\folder1\# to #d:\folder2\#:
    - if #d:\folder2\# exists, contents of #c:\folder1\# is
moved into #d:\folder2\folder1\#;
    - otherwise contents of #c:\folder1\# is moved into the
newly created #d:\folder2\#.

  Create ~file links~@HardSymLink@                                         #Alt-F6#

    If the option "#Process multiple destinations#" is enabled, you may specify
multiple copy or move targets in the input line. In this case, targets should
be separated with a character "#;#" or "#,#". If the name of a target contains
the character ";" or ",", it must be enclosed in quotes.

    If you wish to create the destination folder before copying, terminate the
name with backslash. Also in the Copy dialog you may press #F10# to select a
folder from the active file panel tree or #Alt-F10# to select from the passive
file panel tree. #Shift-F10# allows to open the tree for the path entered in
the input line (if several paths are entered, only the first one is taken into
account). If the option "Process multiple destinations" is enabled, the dialog
selected in the tree is appended to the edit line.

    The possibility of copying, moving and renaming files for plugins depends
upon the plugin module functionality.

    If a destination file already exists, it can be overwritten, skipped or
appended with the file being copied.

    If during copying or moving the destination disk becomes full, it is
possible to either cancel the operation or replace the disk and select the
"Split" item. In the last case the file being copied will be split between
disks. This feature is available only when "Use system copy routine" option in
the ~System settings~@SystemSettings@ dialog is switched off.

    The "Access rights" option is valid only for the NTFS file system and
allows the copying of file access information. The "Default" action which
leaves the access rights processing to the underlying system is selected by
default for copy and move operations. If the "Copy" action is selected then
the original access rights will be applied to the copied/moved files and
folders. If the "Inherit" action is selected then after copying/moving the
inheritable access rights of the destination parent folder will be applied to
the copied/moved files and folders.

    The "Already existing files" option controls FAR behavior if a target file
of the same name already exists.
    Possible values:
    #Ask# - a ~confirmation dialog~@CopyAskOverwrite@ will be shown;
    #Overwrite# - all target files will be replaced;
    #Skip# - target files will not be replaced;
    #Append# - target file will be appended with the file being copied;
    #Only newer file(s)# - only files with newer modification date and time
will be copied;
    #Also ask on R/O files# - controls whether an additional confirmation
dialog should be displayed for read-only files.

    Option "Use system copy routine" from ~System settings~@SystemSettings@
dialog forces the Windows function CopyFileEx usage (or CopyFile if CopyFileEx
is not available) instead of the internal copy implementation to copy files. It
may be useful on NTFS, because CopyFileEx performs a more rational disk space
allocation and copies file extended attributes.
The system copy routine is not used when the file is encrypted and you are
copying it outside of the current disk.

    The "Copy contents of symbolic links" option allows to control the
~logic~@CopyRule@ of FAR processing of ~symbolic links~@HardSymLink@ when
copying/moving.

    When moving files, to determine whether the operation should be performed
as a copy with subsequent deletion or as a direct move (within one physical
drive), FAR takes into account ~symbolic links~@HardSymLink@.

    FAR handles copying to #con# in the same way as copying to #nul# or
#\\\\.\\nul# - that is, the file is read from the disk but not written
anywhere.

    When files are moved to #nul#, #\\\\.\\nul# or #con#, they are not deleted
from the disk.

    The options "Access rights" and "Only newer files" affect only the current
copy session and are not saved for later copy operations.

    Check the #Use filter# checkbox to copy the files that meet the user
defined conditions. Press the #Filter# button to open the ~filters menu~@FiltersMenu@.
Consider, that if you copy the folder with files and all of them does not meet
the filter conditions, then the empty folder will not be copied to the
destination.


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

    If the displayed information is not sufficient you can also view the files
in the internal viewer without exiting the confirmation dialog.


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
    On NTFS volumes you can create #hard links# for files, #junctions# for
folders and #symbolic links# for files and folders using the #Alt-F6# command.


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

    FAR can create hard links and can show the number of the file's hard links
in a separate column (by default, it's the last column in the 9th panel mode)
and sort the files by hard link number.

    Hard links can only be created on the same drive as the source file.

    #Junctions#

    This technology allowing to map any local directories to any other local
directories. For example, if the directory D:\\SYMLINK has C:\\WINNT\\SYSTEM32
as its target, a program accessing D:\\SYMLINK\\DRIVERS will actually access
C:\\WINNT\\SYSTEM32\\DRIVERS. Unlike hard links, symbolic links are not required
to have a target on the same drive.

    Under Windows 2000 it is not allowed to create symbolic links directly to
CD-ROM directories, but this restriction can be overcome by mounting a CD-ROM
to a directory on an NTFS partition.

    #Symbolic links#

    NTFS supports symbolic links starting from Windows Vista (NT 6.0). It's an
improved version of directory junctions - symbolic links can also point to files
and non-local folders, relative paths also supported.
    Keep in mind, that symbolic links created under Windows Vista will not be
accessible under previous versions of Windows.


@ErrCopyItSelf
$ #Error: copy/move onto itself.#
    You may not copy or move a file or folder onto itself.

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

    If a directory has multiple streams, the additional streams will be copied
only when the directory is moved (#F6#).


@ErrLoadPlugin
$ #Error: plugin not loaded#
   This error message can appear in the following cases:

   1. A dynamic link library not present on your system is required
      for correct operation of the plugin module.

   2. For some reason, the module returned an error code
      telling the system to abort plugin loading.

   3. The DLL file of the plugin is corrupt.


@ScrSwitch
$ #Screens switching#
    FAR allows to open several instances of the internal viewer and editor at
the same time. Use #Ctrl-Tab#, #Ctrl-Shift-Tab# or #F12# to switch between
panels and screens with these instances. #Ctrl-Tab# switches to the next
screen, #Ctrl-Shift-Tab# to the previous, #F12# shows a list of all available
screens.

    The number of background viewers and editors is displayed in the left panel
upper left corner. This may be disabled by using ~Panel settings~@PanelSettings@
dialog.


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
    FAR Manager by itself processes the following operating system commands:

    #CLS#

    Clears the screen.

    #disk:#

    To change the current disk on the active panel to the specified disk.

    #CD [disk:]path# or #CHDIR [disk:]path#

    To change the current path on the active panel to the specified path. If
the disk drive letter is specified, the current disk is also changed. If the
active panel shows a ~plugin~@Plugins@ emulated file system, the command "CD"
in the command line may be used to change the folder in the plugin file system.
Unlike "CD", "CHDIR" command always treats the specified parameter as a real
folder name, regardless of the file panel type.

    #CHCP [nnn]#

    Displays or sets the active code page number. "nnn" - specifies a code
page number. Type CHCP without a parameter to display the active code
page number.

    #SET variable=[string]#

    Set environment variable "variable" to the value "string". If "string" is
not specified, the environment variable "variable" will be removed. On startup,
FAR Manager sets several ~environment variables~@FAREnv@ by itself.

    #IF [NOT] EXIST filename command#

    Execute a command "command" if "filename" exists. Prefix "NOT" - execute
the command only if the condition is false.

    #IF [NOT] DEFINED variable command#

    The "DEFINED" conditional works just like "EXISTS" except it takes an
environment variable name and returns true if the environment variable is
defined.


    "IF" commands can be nested, for instance, command "command"

    #if exist file1 if not exist file2 if defined variable command#

    will be executed if the file "file1" exists, the file "file2" does not
exist and the environment variable "variable" is defined.

    #pushd path#

    Команда PUSHD сохраняет текущий каталог во внутреннем стеке и делает
текущим каталог path.

    #popd#

    Переходит в каталог, сохраненный командой PUSHD.

    #clrd#

    Очищает стек каталогов, сохраненных командой PUSHD.

    Notes:

    1. ^<wrap>Any other commands will be sent to the operating
system command processor.

    2. The commands listed above work in:
       - ~Command line~@CmdLineCmd@
       - ~Apply command~@ApplyCmd@
       - ~User menu~@UserMenu@
       - ~File associations~@FileAssoc@


@FAREnv
$ #Environment variables#
    On startup, FAR Manager sets the following environment variables available
to child processes:

    #FARHOME#            path to the folder from which FAR was started.

    #FARLANG#            the name of the current interface language.

    #FARUSER#            the name of the current user given by the /u
                       ~command line~@CmdLine@ option.

    #FARDIRSTACK#        содержимое вершины стека каталогов
                       (который управляется командами pushd и popd)


@KeyMacro
$ #Macro command #
    Keyboard macro commands or macro commands - are recorded sequences of key
presses that can be used to perform repetitive task unlimited number of times
by pressing a single hotkey.

    Each macro command has the following parameters:

    - an hotkey, that will execute the recorded sequence when
      pressed;
    - additional ~settings~@KeyMacroSetting@, that influence the method and
      the area of execution of the recorded sequence.

    Macro commands may contain special ~commands~@KeyMacroLang@, that will be
interpreted in a special way upon execution, those allowing to create complex
constructions.

    Macro commands are mostly used for:

    1. Performing repetitive task unlimited number of times by
       pressing a single hotkey.
    2. Execution of special functions, which are represented by
       special commands in the text of the macro command.
    3. Redefine standard hotkeys, which are used by FAR for
       execution of internal commands.

    The main usage of macro commands is assignment of hotkeys for calling
external plugins and for overloading FAR actions.

    See also:

    ~Macro command areas of execution~@KeyMacroArea@
    ~Hotkeys~@KeyMacroAssign@
    ~Recording and playing-back macro commands~@KeyMacroRecPlay@
    ~Deleting a macro command~@KeyMacroDelete@
    ~Macro command settings~@KeyMacroSetting@
    ~Commands, used inside the text of a macro command~@KeyMacroLang@


@KeyMacroArea
$ #Macro command: areas of execution#
    FAR allows the creation of independent ~macro commands~@KeyMacro@ (commands with
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
       FAR allows to use the following double modifiers:
       #Ctrl-Shift-<key>#, #Ctrl-Alt-<key># and #Alt-Shift-<key>#

    A macro command #can't# be assigned to the following key combinations:
#Alt-Ins#, #Ctrl-<.>#, #Ctrl-Shift-<.>#, #Ctrl-Alt#, #Ctrl-Shift#, #Shift-Alt#,
#Shift-<symbol>#.

    It is impossible to enter some key combinations (in particular #Enter#,
#Esc#, #F1#, #Ctrl-F5#, #MsWheelUp# and #MsWheelDown# with #Ctrl#, #Shift#,
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

       Press #Ctrl-<.># (#Ctrl# and a period pressed together) to record
       a macro in the general mode or #Ctrl-Shift-<.># (#Ctrl#, #Shift# and
       a period pressed together) to record a macro in the special
       mode.

       As the recording begins the symbol '\4FR\-' will appear at the
       upper left corner of the screen.

    2. Contents of the macro command.

       All keys pressed during the recording will be saved with the
       following exceptions:

       - only keys processed by FAR will be saved. Meaning that if
         during the macro recording process an external program is
         run inside the current console then only the keys pressed
         before the execution and after completion of that program
         will be saved.

    3. To finish recording the macro command.

       To finish a macro recording there are special key
       combinations. Because a macro command can be additionally
       configured there are two such combinations: #Ctrl-<.># (#Ctrl#
       and a period pressed together) and #Ctrl-Shift-<.># (#Ctrl#,
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


@KeyMacroDelete
$ #Macro command: deleting a macro command#
    To delete a ~macro command~@KeyMacro@ an empty (containing no commands)
macro should be recorded and assigned the hotkey of the macro command that
needs to be deleted.

    This can be achieved by the following steps:

    1. Start recording a macro command (#Ctrl-<.>#)
    2. Stop recording a macro command (#Ctrl-<.>#)
    3. Enter or select in the hotkey assignment
       dialog the hotkey of the macro command that
       needs to be deleted.

    Attention: after deleting a macro command, the key combination
               (hotkey) that was used for its execution will begin
               to function as it was meant to, originally. That is
               if that key combination was somehow processed by FAR
               or some plugin then after deleting the macro command
               the key combination would be processed by them as in
               the past.


@KeyMacroSetting
$ #Macro command: settings#
    To specify additional ~macro command~@KeyMacro@ settings, start or finish
macro recording with #Ctrl-Shift-<.># instead of #Ctrl-<.># and select the
desired options in the dialog:

    #Sequence:#

    Позволяет изменить записанную последовательность клавиш.

   #Allow screen output while executing macro#

    If this option is not set during the macro command execution FAR Manager
does not redraw the screen. All the updates will be displayed when the macro
command playback is finished.

   #Execute after FAR start#

    Allows to execute the macro command immediately after the FAR Manager is
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

    2. Some key combinations (including #Enter#, #Esc#, #F1# and #Ctrl-F5#,
#MsWheelUp#, #MsWheelDown# и другие клавиши мыши with #Ctrl#, #Shift#, #Alt#) cannot be entered
directly because they have special functions in the dialog. To assign a macro
to one of those key combinations, select it from the drop-down list.


@KeyMacroLang
$ #Macro command: macro language#
    A primitive macro language is implemented in FAR Manager. It allows to
insert logical commands into a simple keystrokes sequence, making macros (along
with ~plugins~@Plugins@) a powerful facility assisting in the everyday use of
FAR Manager.

    Several of the available commands are listed below:
    #$Date#         - insert current date/time
    #$Exit#         - stop macro playback
    #$IClip#        - working with clipboard
    #$MMode#        - toggle macro display mode
    #$Text#         - arbitrary text insertion
    #$XLat#         - transliteration function
    #$If-$Else#     - condition operator
    #$While#        - conditioned loop operator
    #$Rep#          - loop operator
    #%var#          - using variables
     and others...

    Addition of macro language commands to a ~macro~@KeyMacro@ can only be done
by manually editing the registry or by using special tools/plugins.

    Description of the macro language can be found in the accompanying
documentation.

@KeyMacroEditList
$ #Макросы: Редактор#
    Macro-commands available in the editor are listed below. Descriptions are read from the registry.

<!Macro:Common!>
<!Macro:Editor!>

@KeyMacroViewerList
$ #Макросы: Программа просмотра#
    Macro-commands available in the viewer are listed below. Descriptions are read from the registry.

<!Macro:Common!>
<!Macro:Viewer!>

@Index
$ #Index help file#
<%INDEX%>
