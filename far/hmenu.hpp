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
  const wchar_t *Name;
  int Selected;
  struct MenuDataEx *SubMenu;
  int SubMenuSize;
  const wchar_t *SubMenuHelp;
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
    void ProcessSubMenu(struct MenuDataEx *Data,int DataCount,const wchar_t *SubMenuHelp,
                        int X,int Y,int &Position);
    wchar_t GetHighlights(const struct HMenuData *_item);
    BOOL CheckHighlights(WORD CheckSymbol);

  public:
    HMenu(struct HMenuData *Item,int ItemCount);
    virtual ~HMenu();

  public:
    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

    virtual void Process();
    virtual void ResizeConsole();

    void GetExitCode(int &ExitCode,int &VExitCode);
};


#endif  // __HMENU_HPP__
