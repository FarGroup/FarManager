#include "headers.hpp"
#include "baseinp.hpp"
#include "chgmmode.hpp"
#include "chgprior.hpp"
#include "classes.hpp"
#include "cmdline.hpp"
#include "colors.hpp"
#include "constitle.hpp"
#include "copy.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "dizlist.hpp"
#include "edit.hpp"
#include "editor.hpp"
//#include "exception.hpp"
#include "farconst.hpp"
#include "farftp.hpp"
#include "farqueue.hpp"
#include "fileedit.hpp"
#include "filelist.hpp"
#include "filepanels.hpp"
#include "filestr.hpp"
#include "fileview.hpp"
#include "filter.hpp"
#include "findfile.hpp"
#include "flink.hpp"
#include "fn.hpp"
#include "foldtree.hpp"
#include "frame.hpp"
#include "global.hpp"
#include "grabber.hpp"
#include "grpsort.hpp"
#include "headers.hpp"
#include "help.hpp"
#include "hilight.hpp"
#include "history.hpp"
#include "hmenu.hpp"
#include "infolist.hpp"
#include "int64.hpp"
#include "internalheaders.hpp"
#include "keybar.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "lockscrn.hpp"
#include "macro.hpp"
#include "manager.hpp"
#include "menubar.hpp"
#include "modal.hpp"
#include "namelist.hpp"
#include "panel.hpp"
#include "plognmn.hpp"
#include "plugin.hpp"
#include "plugins.hpp"
#include "poscache.hpp"
#include "qview.hpp"
#include "rdrwdsk.hpp"
#include "res.hpp"
#include "savefpos.hpp"
#include "savescr.hpp"
#include "scantree.hpp"
#include "scrbuf.hpp"
#include "scrobj.hpp"
#include "struct.hpp"
#include "treelist.hpp"
#include "udlist.hpp"
#include "viewer.hpp"
#include "vmenu.hpp"

#define S(a,b) printf("sizeof(%s)%*.*s = %6i\n",a,20-strlen(a),20-strlen(a),"                                  ",sizeof(b));AllSize+=sizeof(b)

int main()
{
   DWORD AllSize=0;
    S("BaseInput",BaseInput);
    S("ChangeMacroMode",ChangeMacroMode);
    S("ChangePriority",ChangePriority);
    S("CommandLine",CommandLine);
    S("ConsoleTitle",ConsoleTitle);
    S("ControlObject",ControlObject);
    S("Dialog",Dialog);
    S("DizList",DizList);
    S("Edit",Edit);
    S("Editor",Editor);
    S("FileEditor",FileEditor);
    S("FileList",FileList);
    S("FilePanels",FilePanels);
    S("FilePositionCache",FilePositionCache);
    S("FileViewer",FileViewer);
    S("FindFiles",FindFiles);
    S("FolderTree",FolderTree);
    S("Frame",Frame);
    S("GetFileString",GetFileString);
    S("Grabber",Grabber);
    S("GroupSort",GroupSort);
    S("Help",Help);
    S("HighlightFiles",HighlightFiles);
    S("History",History);
    S("HMenu",HMenu);
    S("InfoList",InfoList);
    S("int64",int64);
    S("KeyBar",KeyBar);
    S("KeyMacro",KeyMacro);
    S("Language",Language);
    S("LockScreen",LockScreen);
    S("Manager",Manager);
    S("MenuBar",MenuBar);
    S("Modal",Modal);
    S("NamesList",NamesList);
    S("Panel",Panel);
    S("PanelFilter",PanelFilter);
    S("PluginsSet",PluginsSet);
    S("PreserveLongName",PreserveLongName);
    S("QuickView",QuickView);
    S("RedrawDesktop",RedrawDesktop);
    S("SaveFilePos",SaveFilePos);
    S("SaveScreen",SaveScreen);
    S("ScanTree",ScanTree);
    S("ScreenBuf",ScreenBuf);
    S("ScreenObject",ScreenObject);
    S("ShellCopy",ShellCopy);
    S("TreeList",TreeList);
    S("UserDefinedList",UserDefinedList);
    S("Viewer",Viewer);
    S("VMenu",VMenu);
    S("FileListItem",FileListItem);

    printf("------\n%d\n",AllSize);
    return 0;
}
