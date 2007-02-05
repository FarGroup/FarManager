#include "FarMenu.h"
#include "Plugin.h"
#include <tchar.h>
#include <cassert>

CFarMenu::CFarMenu(LPCTSTR szHelp/*=NULL*/, unsigned nMaxItems/*=40*/)
  : m_szHelp(szHelp)
  , m_pfmi(NULL)
  , m_nItemCnt(0)
  , m_szArrow(_T("  \x10"))
  , m_bArrowsAdded(false)
  , m_nMaxItems(nMaxItems)
{
  m_pfmi=new FarMenuItemEx[nMaxItems];
  m_pbHasSubMenu=new bool[nMaxItems];

  // Это кривоватый способ вычисления макс. ширины
  // Хотелось бы от FARа получать эту инфу...
  const unsigned nArrowLen=sizeof(m_szArrow)/sizeof(m_szArrow[0]);
  m_nMaxTextLen=sizeof(m_pfmi->Text.Text)/sizeof(m_pfmi->Text.Text[0])-nArrowLen;
  HANDLE hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (INVALID_HANDLE_VALUE!=hStdOutput
    && GetConsoleScreenBufferInfo(hStdOutput, &csbi))
  {
    m_nMaxTextLen=min(m_nMaxTextLen, csbi.dwSize.X-4*2+1-nArrowLen);
  }
  else
  {
    assert(0);
  }
}

CFarMenu::~CFarMenu()
{
  delete[] m_pfmi;
  delete[] m_pbHasSubMenu;
}

void CFarMenu::AddSeparator()
{
  AddItem(NULL, false);
}

unsigned CFarMenu::AddItem(LPCTSTR szText, bool bHasSubMenu/*=false*/
             , ECheck enChecked/*=UNCHECKED*/
             , bool bDisabled/*=false*/)
{
  return InsertItem(m_nItemCnt, szText, bHasSubMenu, enChecked, bDisabled);
}

unsigned CFarMenu::InsertItem(unsigned nIndex, LPCTSTR szText
                , bool bHasSubMenu/*=false*/
                , ECheck enChecked/*=UNCHECKED*/
                , bool bDisabled/*=false*/)
{
  if (m_nItemCnt>=m_nMaxItems || nIndex>m_nItemCnt)
  {
    assert(0);
    return 0;
  }
  memmove(m_pfmi+nIndex+1, m_pfmi+nIndex, sizeof(*m_pfmi)*(m_nItemCnt-nIndex));
  memmove(m_pbHasSubMenu+nIndex+1, m_pbHasSubMenu+nIndex, sizeof(*m_pbHasSubMenu)*(m_nItemCnt-nIndex));
  m_nItemCnt++;
  m_pbHasSubMenu[nIndex]=bHasSubMenu;
  m_pfmi[nIndex].Flags=0;
  if (szText)
  {
    lstrcpyn(m_pfmi[nIndex].Text.Text, szText, m_nMaxTextLen);
  }
  else
  {
    m_pfmi[nIndex].Text.Text[0]=0;
    m_pfmi[nIndex].Flags|=MIF_SEPARATOR;
  }
  switch (enChecked)
  {
  case CHECKED:
    m_pfmi[nIndex].Flags|=MIF_CHECKED;
      break;
  case RADIO:
    m_pfmi[nIndex].Flags|=7|MIF_CHECKED;
    break;
  }
  if (bDisabled) m_pfmi[nIndex].Flags|=MIF_DISABLE;
  return nIndex;
}

unsigned CFarMenu::MenuItemLen(LPCTSTR szText)
{
  unsigned nLen=lstrlen(szText);
  for (unsigned n=0; n<nLen; n++) if (szText[n]==_T('&')) nLen--;
  return nLen;
}

void CFarMenu::AddArrows()
{
  if (!m_bArrowsAdded)
  {
    m_bArrowsAdded=true;
    unsigned nMaxLen=0;
    unsigned i;
    for (i=0; i<m_nItemCnt; i++)
    {
      unsigned nLen=MenuItemLen(m_pfmi[i].Text.Text);
      if (nLen>nMaxLen) nMaxLen=nLen;
    }
    for (i=0; i<m_nItemCnt; i++)
    {
      unsigned nLen=MenuItemLen(m_pfmi[i].Text.Text);
      if (m_pbHasSubMenu[i])
      {
        for (unsigned n=0; n<nMaxLen-nLen; n++)
        {
          lstrcat(m_pfmi[i].Text.Text, _T(" "));
        }
        lstrcat(m_pfmi[i].Text.Text, m_szArrow);
      }
    }
  }
}

LPCTSTR CFarMenu::operator[](unsigned nIndex)
{
  return m_pfmi[nIndex].Text.Text;
}

void CFarMenu::SetSelectedItem(unsigned nIndex)
{
  for (unsigned i=0; i<m_nItemCnt; i++)
  {
    if (nIndex==i)
    {
      m_pfmi[i].Flags|=MIF_SELECTED;
    }
    else
    {
      m_pfmi[i].Flags&=~MIF_SELECTED;
    }
  }
}

void CFarMenu::GetCursorXY(int* pnX, int* pnY)
{
  POINT pt={0};
  if (!GetCursorPos(&pt))
  {
    assert(0);
  }
  else
  {
    HWND hFarWnd=(HWND)thePlug->AdvControl(ACTL_GETFARHWND, 0);
    if (!ScreenToClient(hFarWnd, &pt))
    {
      assert(0);
    }
    else
    {
      RECT rc;
      CONSOLE_SCREEN_BUFFER_INFO csbi;
      HANDLE hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
      if (!GetClientRect(hFarWnd, &rc)
        || INVALID_HANDLE_VALUE==hStdOutput
        || !GetConsoleScreenBufferInfo(hStdOutput, &csbi))
      {
        assert(0);
      }
      else
      {
        if (pt.x>=rc.left && pt.x<=rc.right && pt.y>=rc.top && pt.y<=rc.bottom)
        {
          *pnX=int(csbi.dwSize.X*pt.x/(rc.right-rc.left));
          *pnY=int(csbi.dwSize.Y*pt.y/(rc.bottom-rc.top));
        }
      }
    }
  }
}

int CFarMenu::Show(LPCTSTR szTitle, int nSelItem/*=0*/, bool bAtCursorPos/*=false*/)
{
  int nX=-1, nY=-1;
  if (bAtCursorPos) GetCursorXY(&nX, &nY);
  AddArrows();
  enum {CTRL_PGDN=MAKELONG(VK_NEXT, PKF_CONTROL)
    , CTRL_PGUP=MAKELONG(VK_PRIOR, PKF_CONTROL)
    , ALTSHIFT_F9=MAKELONG(VK_F9, PKF_ALT|PKF_SHIFT)
    , SHIFT_ENTER=MAKELONG(VK_RETURN, PKF_SHIFT)
    , SHIFT_SPACE=MAKELONG(VK_SPACE, PKF_SHIFT)
  };
  int pnBreakKeys[]={VK_RIGHT, VK_LEFT
    , CTRL_PGDN, CTRL_PGUP, VK_BACK, VK_SPACE
    , SHIFT_ENTER, SHIFT_SPACE, ALTSHIFT_F9, 0};
  int nBreakCode;
  while (1)
  {
    SetSelectedItem(nSelItem);
    nSelItem=thePlug->Menu(nX, nY, MAX_HEIGHT
      , FMENU_WRAPMODE|FMENU_USEEXT
      , szTitle, NULL, m_szHelp, pnBreakKeys, &nBreakCode
      , (const struct FarMenuItem*)m_pfmi, m_nItemCnt);
    if (-1==nBreakCode) return nSelItem;
    assert(-1!=nSelItem);
    switch (pnBreakKeys[nBreakCode])
    {
    case VK_RIGHT:
    case CTRL_PGDN:
      if (m_pbHasSubMenu[nSelItem]) return nSelItem;
      break;
    case VK_BACK:
    case VK_LEFT:
    case CTRL_PGUP:
      return SHOW_BACK;
      break;
    case VK_SPACE:
    case SHIFT_ENTER:
    case SHIFT_SPACE:
      return nSelItem;
    case ALTSHIFT_F9:
      thePlug->Configure();
    }
  }
}
