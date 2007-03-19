#ifndef __CONTROLOBJECT_HPP__
#define __CONTROLOBJECT_HPP__
/*
ctrlobj.hpp

”правление остальными объектами, раздача сообщений клавиатуры и мыши

*/

#include "macro.hpp"
#include "plugins.hpp"

class CommandLine;
class History;
class HistoryW;
class KeyBar;
class MenuBar;
class HighlightFiles;
class FilePositionCache;
class FilePanels;

class ControlObject
{
  private:
    FilePanels *FPanels;

  public:
    ControlObject();
    ~ControlObject();

  public:
    void Init();

  public:
    FilePanels *Cp();

    void CreateFilePanels();

    CommandLine *CmdLine;
    HistoryW *CmdHistory,*FolderHistory,*ViewHistory;

    KeyBar *MainKeyBar;
    MenuBar *TopMenuBar;
    HighlightFiles *HiFiles;
    FilePositionCache *ViewerPosCache,*EditorPosCache;
    KeyMacro Macro;
    PluginManager Plugins;

    static void ShowCopyright(DWORD Flags=0);
};

extern ControlObject *CtrlObject;

#endif // __CONTROLOBJECT_HPP__
