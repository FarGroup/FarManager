#ifndef __HMENU_HPP__
#define __HMENU_HPP__
/*
hmenu.hpp

Горизонтальное меню

*/

/* Revision: 1.06 23.04.2006 $ */

#include "modal.hpp"
#include "frame.hpp"

struct HMenuData
{
  const wchar_t *Name;
  int Selected;
  struct MenuDataEx *SubMenu;
  int SubMenuSize;
  const wchar_t *SubMenuHelp;
};

class VMenu;

class HMenu:virtual public Modal, virtual public Frame
{
  private:
    void DisplayObject();
    void ShowMenu();
    void ProcessSubMenu(struct MenuDataEx *Data,int DataCount,const wchar_t *SubMenuHelp,
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


#endif  // __HMENU_HPP__
