#ifndef _HMENU_H_
#define _HMENU_H_

#include <windows.h>
#include <cassert>
class CHMenu
{
public:
  CHMenu() : m_hMenu(CreatePopupMenu()) {}
  ~CHMenu() {Destroy();}
  
  void Destroy()
  {
    if (m_hMenu && !DestroyMenu(m_hMenu))
    {
      assert(0);
    }
    m_hMenu=NULL;
  }

  void Clear()
  {
    for (int i=GetMenuItemCount()-1; i>=0; i--)
    {
      if (!DeleteMenu(m_hMenu, i, MF_BYPOSITION))
      {
        assert(0);
      }
    }
  }

  int GetMenuItemCount() {return ::GetMenuItemCount(m_hMenu);}
  bool operator !() {return (NULL==m_hMenu);}
  operator HMENU() {return m_hMenu;}
protected:
  HMENU m_hMenu;
};

#endif
