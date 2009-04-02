#ifndef _FARMENU_H_
#define _FARMENU_H_

#include "plugin.hpp"

class CFarMenu
{
public:
  CFarMenu(LPCTSTR szHelp=NULL, unsigned nMaxItems=40);
  ~CFarMenu();
  enum ECheck {CHECKED, UNCHECKED, RADIO};
  unsigned AddItem(LPCTSTR szText, bool bHasSubMenu=false
    , ECheck enChecked=UNCHECKED, bool bDisabled=false);
  unsigned InsertItem(unsigned nIndex, LPCTSTR szText, bool bHasSubMenu=false
    , ECheck enChecked=UNCHECKED, bool bDisabled=false);
  void AddSeparator();
  int Show(LPCTSTR szTitle, int nSelItem=0, bool bAtCursorPos=false);
  void GetCursorXY(int* pnX, int* pnY);
  enum {SHOW_CANCEL=-1, SHOW_BACK=-2};
  LPCTSTR operator[](unsigned nIndex);
protected:
  void AddArrows();
  void SetSelectedItem(unsigned nIndex);
  unsigned MenuItemLen(LPCTSTR szText);
protected:
  LPCTSTR m_szHelp;
  bool m_bArrowsAdded;
  FarMenuItemEx* m_pfmi;
  unsigned m_nItemCnt;
  unsigned m_nMaxItems;
  LPCTSTR m_szArrow;
  bool* m_pbHasSubMenu;
  enum {MAX_HEIGHT=45};
};

#endif
