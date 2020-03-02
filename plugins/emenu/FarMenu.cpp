#include "FarMenu.h"
#include "Plugin.h"
#include "guid.hpp"
#include <cassert>

static const wchar_t empty_wstr[] = {0};

CFarMenu::CFarMenu(LPCWSTR szHelp/*=NULL*/, const GUID* MenuId/*= nullptr*/, unsigned nMaxItems/*=40*/)
  : m_szHelp(szHelp)
  , m_pfmi(NULL)
  , m_nItemCnt(0)
  , m_szArrow(L"  \x25BA")
  , m_bArrowsAdded(false)
  , m_Id(MenuId? new GUID(*MenuId) : nullptr)
  , m_nMaxItems(nMaxItems)
{
  m_pfmi=new FarMenuItem[nMaxItems];
  m_pbHasSubMenu=new bool[nMaxItems];
}

CFarMenu::~CFarMenu()
{
  for(unsigned i = 0; i < m_nItemCnt; i++)
    if(m_pfmi[i].Text != empty_wstr)
      delete m_pfmi[i].Text;
  delete[] m_pfmi;
  delete[] m_pbHasSubMenu;
  delete m_Id;
}

void CFarMenu::AddSeparator()
{
  AddItem(NULL, false);
}

unsigned CFarMenu::AddItem(LPCWSTR szText, bool bHasSubMenu/*=false*/, ECheck enChecked/*=UNCHECKED*/, bool bDisabled/*=false*/)
{
  return InsertItem(m_nItemCnt, szText, bHasSubMenu, enChecked, bDisabled);
}

unsigned CFarMenu::InsertItem(unsigned nIndex, LPCWSTR szText, bool bHasSubMenu/*=false*/, ECheck enChecked/*=UNCHECKED*/, bool bDisabled/*=false*/)
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
    if(!*szText)
    {
      m_pfmi[nIndex].Text = empty_wstr;
    }
    else
	{
      m_pfmi[nIndex].Text = new wchar_t[lstrlen(szText)+1];
      lstrcpy((wchar_t *)m_pfmi[nIndex].Text,szText);
    }
  }
  else
  {
    m_pfmi[nIndex].Text = empty_wstr;
    m_pfmi[nIndex].Flags|=MIF_SEPARATOR;
  }
  switch (enChecked)
  {
  case CHECKED:
    m_pfmi[nIndex].Flags|=MIF_CHECKED;
      break;

  case UNCHECKED:
    break;

  case RADIO:
    m_pfmi[nIndex].Flags|=7|MIF_CHECKED;
    break;
  }
  if (bDisabled) m_pfmi[nIndex].Flags|=MIF_DISABLE;
  return nIndex;
}

unsigned CFarMenu::MenuItemLen(LPCWSTR szText)
{
  unsigned nLen=lstrlen(szText);
  for (unsigned n=0; n<nLen; n++) if (szText[n]==L'&') nLen--;
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
      unsigned nLen=MenuItemLen(m_pfmi[i].Text);
      if (nLen>nMaxLen) nMaxLen=nLen;
    }
    for (i=0; i<m_nItemCnt; i++)
    {
      unsigned nLen=MenuItemLen(m_pfmi[i].Text);
      if (m_pbHasSubMenu[i])
      {
        {
          size_t  len=lstrlen(m_pfmi[i].Text)+lstrlen(m_szArrow)+(nMaxLen-nLen);
          wchar_t *pn = new wchar_t[len+1];
          wcscpy(pn, m_pfmi[i].Text);
          if (m_pfmi[i].Text != empty_wstr)  // paranoya
            delete m_pfmi[i].Text;
          m_pfmi[i].Text = pn;
        }
        for (unsigned n=0; n<nMaxLen-nLen; n++)
        {
          lstrcat((wchar_t*)m_pfmi[i].Text, L" ");
        }
        lstrcat((wchar_t*)m_pfmi[i].Text, m_szArrow);
      }
    }
  }
}

LPCWSTR CFarMenu::operator[](unsigned nIndex)
{
  return m_pfmi[nIndex].Text;
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
    HWND hFarWnd=(HWND)thePlug->AdvControl(&MainGuid, ACTL_GETFARHWND, 0, NULL);
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
        SMALL_RECT console_rect;
        if (thePlug->AdvControl(&MainGuid, ACTL_GETFARRECT, 0, &console_rect))
        {
          success=true;
          console_size.X=console_rect.Right-console_rect.Left+1;
          console_size.Y=console_rect.Bottom-console_rect.Top+1;
        }
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

int CFarMenu::Show(LPCWSTR szTitle, int nSelItem/*=0*/, bool bAtCursorPos/*=false*/)
{
  int nX=-1, nY=-1;
  if (bAtCursorPos) GetCursorXY(&nX, &nY);
  AddArrows();
  FarKey pBreakKeys[]={{VK_RIGHT,0},
                       {VK_LEFT,0},
                       {VK_NEXT,LEFT_CTRL_PRESSED},
                       {VK_PRIOR, LEFT_CTRL_PRESSED},
                       {VK_BACK,0},
                       {VK_SPACE,0},
                       {VK_RETURN,SHIFT_PRESSED},
                       {VK_SPACE,SHIFT_PRESSED},
                       {VK_F9,SHIFT_PRESSED|LEFT_ALT_PRESSED},
                       {0,0}};
  intptr_t nBreakCode;
  while (1)
  {
    SetSelectedItem(nSelItem);
    nSelItem=(int)thePlug->Menu(&MainGuid, m_Id, nX, nY, MAX_HEIGHT, FMENU_WRAPMODE, szTitle, NULL, m_szHelp, pBreakKeys, &nBreakCode, m_pfmi, m_nItemCnt);
    if (-1==nBreakCode) return nSelItem;
    assert(-1!=nSelItem);
    switch (pBreakKeys[nBreakCode].VirtualKeyCode)
    {
    case VK_RIGHT:
    case VK_NEXT:
      if (m_pbHasSubMenu[nSelItem]) return nSelItem;
      break;
    case VK_BACK:
    case VK_LEFT:
    case VK_PRIOR:
      return SHOW_BACK;
      break;
    case VK_SPACE:
    case VK_RETURN:
      return nSelItem;
    case VK_F9:
      thePlug->Configure();
    }
  }
}
