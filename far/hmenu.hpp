#ifndef __HMENU_HPP__
#define __HMENU_HPP__
/*
hmenu.hpp

Горизонтальное меню

*/

/* Revision: 1.03 26.07.2001 $ */

/*
Modify:
  26.07.2001 SVS
    ! HMenu прикручен к фреймам - попытка "раз"
  14.06.2001 OT
    ! "Бунт" ;-)
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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

class HMenu:virtual public Modal, virtual public Frame
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
