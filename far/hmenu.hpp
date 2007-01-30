#ifndef __HMENU_HPP__
#define __HMENU_HPP__
/*
hmenu.hpp

Горизонтальное меню

*/

#include "modal.hpp"
#include "frame.hpp"

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
    void DisplayObject();
    void ShowMenu();
    void ProcessSubMenu(struct MenuData *Data,int DataCount,char *SubMenuHelp,
                        int X,int Y,int &Position);
    VMenu *SubMenu;
    struct HMenuData *Item;
    int SelectPos;
    int ItemCount;
    int VExitCode;
    int ItemX[16];
  public:
    HMenu(struct HMenuData *Item,int ItemCount);
    ~HMenu();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void GetExitCode(int &ExitCode,int &VExitCode);
    void Process();
    void ResizeConsole();
};


#endif	// __HMENU_HPP__
