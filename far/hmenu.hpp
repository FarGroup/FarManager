#ifndef __HMENU_HPP__
#define __HMENU_HPP__
/*
hmenu.hpp

Горизонтальное меню

*/

#include "modal.hpp"
#include "frame.hpp"
#include "CriticalSections.hpp"

struct HMenuData
{
  char *Name;
  int Selected;
  struct MenuData *SubMenu;
  int SubMenuSize;
  char *SubMenuHelp;
};

class VMenu;

class HMenu: public Modal
{
  private:
    VMenu *SubMenu;
    struct HMenuData *Item;
    int SelectPos;
    int ItemCount;
    int VExitCode;
    int ItemX[16];
    CriticalSection CS;

  private:
    virtual void DisplayObject();
    void ShowMenu();
    void ProcessSubMenu(struct MenuData *Data,int DataCount,char *SubMenuHelp,int X,int Y,int &Position);
    char GetHighlights(const struct HMenuData *_item);
    int CheckHighlights(BYTE CheckSymbol,int StartPos=0);

  public:
    HMenu(struct HMenuData *Item,int ItemCount);
    virtual ~HMenu();

  public:
    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

    void GetExitCode(int &ExitCode,int &VExitCode);
    virtual void Process();
    virtual void ResizeConsole();
};


#endif  // __HMENU_HPP__
