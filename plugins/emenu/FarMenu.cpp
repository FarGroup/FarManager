#include "FarMenu.h"
#include "Plugin.h"
#include <tchar.h>
#include <cassert>

#ifdef UNICODE
static const wchar_t  empty_wstr;
#define TextPtr Text
#else
#define TextPtr Text.Text
#endif

CFarMenu::CFarMenu(LPCTSTR szHelp/*=NULL*/, unsigned nMaxItems/*=40*/)
  : m_szHelp(szHelp)
  , m_pfmi(NULL)
  , m_nItemCnt(0)
#ifndef UNICODE
  , m_szArrow("  \x10")
#else
  , m_szArrow(L"  \x25BA")
#endif
  , m_bArrowsAdded(false)
  , m_nMaxItems(nMaxItems)
{
  m_pfmi=new FarMenuItemEx[nMaxItems];
  m_pbHasSubMenu=new bool[nMaxItems];
}

CFarMenu::~CFarMenu()
{
#ifdef UNICODE
  for(unsigned i = 0; i < m_nItemCnt; i++)
    if(m_pfmi[i].Text != &empty_wstr)
      delete m_pfmi[i].Text;
#endif
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
#ifndef UNICODE
    lstrcpyn(m_pfmi[nIndex].Text.Text, szText, sizeof(m_pfmi[nIndex].Text.Text));
#else
    if(!*szText)
      m_pfmi[nIndex].Text=&empty_wstr;
    else
	{
      m_pfmi[nIndex].Text = new wchar_t[lstrlen(szText)+1];
      lstrcpy((wchar_t *)m_pfmi[nIndex].Text,szText);
    }
#endif
  }
  else
  {
#ifndef UNICODE
    m_pfmi[nIndex].Text.Text[0]=0;
#else
    m_pfmi[nIndex].Text=&empty_wstr;
#endif
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
#ifdef UNICODE
    size_t  arrLen = lstrlen(m_szArrow);
#endif
    for (i=0; i<m_nItemCnt; i++)
    {
      unsigned nLen=MenuItemLen(m_pfmi[i].TextPtr);
      if (nLen>nMaxLen) nMaxLen=nLen;
    }
    for (i=0; i<m_nItemCnt; i++)
    {
      unsigned nLen=MenuItemLen(m_pfmi[i].TextPtr);
      if (m_pbHasSubMenu[i])
      {
#ifdef UNICODE
        {
          size_t  len=lstrlen(m_pfmi[i].Text)+lstrlen(m_szArrow)+(nMaxLen-nLen);
          wchar_t *pn = new wchar_t[len+1];
          wcscpy(pn, m_pfmi[i].Text);
          if(m_pfmi[i].Text != &empty_wstr)  // paranoya
            delete m_pfmi[i].Text;
          m_pfmi[i].Text = pn;
        }
#endif
        for (unsigned n=0; n<nMaxLen-nLen; n++)
        {
          lstrcat((TCHAR*)m_pfmi[i].TextPtr, _T(" "));
        }
        lstrcat((TCHAR*)m_pfmi[i].TextPtr, m_szArrow);
      }
    }
  }
}

LPCTSTR CFarMenu::operator[](unsigned nIndex)
{
  return m_pfmi[nIndex].TextPtr;
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
      if (!GetClientRect(hFarWnd, &rc))
        assert(0);
      else {
        bool success=false;
        COORD console_size;
#ifndef UNICODE
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        HANDLE hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
        if (INVALID_HANDLE_VALUE!=hStdOutput
          && GetConsoleScreenBufferInfo(hStdOutput, &csbi))
        {
          success=true;
          console_size.X=csbi.dwSize.X;
          console_size.Y=csbi.dwSize.Y;
        }
#else
        SMALL_RECT console_rect;
        if (thePlug->AdvControl(ACTL_GETFARRECT,(void*)&console_rect))
        {
          success=true;
          console_size.X=console_rect.Right-console_rect.Left+1;
          console_size.Y=console_rect.Bottom-console_rect.Top+1;
        }
#endif
        if (!success)
        {
          assert(0);
        }
        else
        {
          if (pt.x>=rc.left && pt.x<=rc.right && pt.y>=rc.top && pt.y<=rc.bottom)
          {
            *pnX=int(console_size.X*pt.x/(rc.right-rc.left));
            *pnY=int(console_size.Y*pt.y/(rc.bottom-rc.top));
          }
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
