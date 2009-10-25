#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <CRT/crt.hpp>
#include "plugin.hpp"
#include <shlobj.h>
#include <tchar.h>
#include "auto_sz.h"

#define PFX_RCLK _T("rclk")
#define PFX_RCLK_TXT _T("rclk_txt")
#define PFX_RCLK_GUI _T("rclk_gui")
#define PFX_RCLK_CMD _T("rclk_cmd")
#define PFX_RCLK_ITEM _T("rclk_item")

class CPlugin : public PluginStartupInfo
{
#ifdef UNICODE
  PluginPanelItem **SelectedItems;
  int SelectedItemsCount;
#endif
  CPlugin();

public:
  explicit CPlugin(const struct PluginStartupInfo *Info);
  ~CPlugin(void);
  void GetPluginInfo(PluginInfo *Info);
  HANDLE OpenPlugin(int nOpenFrom, INT_PTR nItem);
  enum EDoMenu
  {
    DOMNU_OK,
    DOMENU_CANCELLED,
    DOMENU_BACK,
    DOMNU_ERR_DIFFERENT_FOLDERS,
    DOMNU_ERR_SHOW,
    DOMNU_ERR_INVOKE,
  };
  EDoMenu OpenPluginBkg(int nOpenFrom, INT_PTR nItem);
  int Configure();
  void ExitFAR();
  int Menu(int nX, int nY, int nMaxHeight, DWORD nFlags
          , LPCTSTR szTitle, LPCTSTR szBottom, LPCTSTR szHelpTopic
          , const int* pnBreakKeys, int* pnBreakCode
          , const FarMenuItem* pItems, int nItemsNumber);
#ifndef UNICODE
  int DialogEx(int X1, int Y1, int X2, int Y2, LPCTSTR szHelpTopic
    , FarDialogItem* pItem, int nItemsNumber, DWORD nReserved, DWORD nFlags
    , FARWINDOWPROC DlgProc, LONG_PTR pParam);
#else
  HANDLE DialogInit(int X1, int Y1, int X2, int Y2, LPCTSTR szHelpTopic
    , FarDialogItem* pItem, int nItemsNumber, DWORD nReserved, DWORD nFlags
    , FARWINDOWPROC DlgProc, LONG_PTR pParam);
  int DialogRun(HANDLE hDlg);
  void DialogFree(HANDLE hDlg);
#endif
  INT_PTR AdvControl(int nCommand, void *pParam);
  HINSTANCE m_hModule;
  IMalloc* m_pMalloc;
protected:
  LPCTSTR GetMsg(int nMsgId);
  int Message(DWORD nFlags, LPCTSTR szHelpTopic, const LPCTSTR* pItems
    , int nItemsNumber, int nButtonsNumber);
#ifndef UNICODE
  int Control(int nCommand, void* pParam);
#else
  int Control(int nCommand, int Param1,INT_PTR Param2);
#endif
  EDoMenu DoMenu(LPTSTR szCmdLine);
  EDoMenu SelectDrive();
  enum EAutoItem
  {
    AI_NONE,
    AI_VERB,
    AI_ITEM
  };
  EDoMenu MenuForPanelOrCmdLine(LPTSTR szCmdLine=NULL
    , EAutoItem enAutoItem=AI_NONE);
  EDoMenu DoMenu(LPSHELLFOLDER pCurFolder, LPCITEMIDLIST* pPiids
    , LPCTSTR pFiles[], unsigned nFiles, unsigned nFolders, LPCTSTR szCommand=NULL
    , EAutoItem enAutoItem=AI_NONE);
  bool ShowGuiMenu(HMENU hMenu, LPCONTEXTMENU pMenu1, LPCONTEXTMENU2 pMenu2
    , LPCONTEXTMENU3 pMenu, int* pnCmd);
  bool ShowTextMenu(HMENU hMenu, LPCONTEXTMENU pPreferredMenu
    , LPCONTEXTMENU2 pMenu2, LPCONTEXTMENU3 pMenu3
    , int* pnCmd, LPCTSTR szTitle, LPSHELLFOLDER pCurFolder
    , LPCITEMIDLIST* ppiid, unsigned nPiidCnt);
  bool ShowFolder(LPSHELLFOLDER pParentFolder, LPCITEMIDLIST piid
    , int* pnCmd, LPCTSTR szTitle, LPDROPTARGET* ppDropTarget);
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT nMsg, WPARAM wParam
    , LPARAM lParam);
  void Init();
  bool GetFilesFromParams(LPTSTR szCmdLine, LPCTSTR** ppFiles
    , unsigned* pnFiles, unsigned* pnFolders, auto_sz* pstrCurDir
    , bool bSkipFirst);
  bool GetFilesFromPanel(LPCTSTR** ppFiles, unsigned* pnFiles
    , unsigned* pnFolders, auto_sz* pstrCurDir);
  static bool IsSpace(TCHAR ch) {return _T(' ')==ch || _T('\t')==ch;}
  unsigned ParseParams(LPTSTR szParams, LPCTSTR* pFiles=NULL);
  void ReadRegValues();
  static LONG_PTR WINAPI CfgDlgProcStatic(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
  void CfgDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
  enum EAdditionalStr {AS_NONE=0, AS_HELPTEXT=1, AS_VERB=2};
  bool GetAdditionalString(IContextMenu* pContextMenu, UINT nID
    , EAdditionalStr enAdditionalString, auto_sz* pstr, BOOL bToOEM=TRUE);
protected:
  enum
  {
    LNG_TITLE,
    LNG_SHOWMESS,
    LNG_SHOWGUI,
    LNG_DELETE_USING_FAR,
    LNG_CLEAR_SELECTION,
    LNG_SILENT,
    LNG_ADDITIONAL_INFO,
    LNG_ADDITIONAL_INFO_NONE,
    LNG_ADDITIONAL_INFO_HELPTEXT,
    LNG_ADDITIONAL_INFO_VERBS,
    LNG_ADDITIONAL_DIFFERENT_ONLY,
    LNG_GUI_POSITION,
    LNG_GUI_MOUSE_CURSOR,
    LNG_GUI_WINDOW_CENTER,
    LNG_SAVE,
    LNG_CANCEL,
    LNG_CONTEXT_MENU,
    LNG_SELECT_DRIVE,
    LNG_MNU_GUI,
    LNG_MNU_TEXT,
    LNG_FILES,
    LNG_FOLDERS,
    LNG_MT_OWNERDRAWN,
    LNG_MT_BITMAP,
    LNG_ERR_DIFFERENT_FOLDERS,
    LNG_ERR_SHOW,
    LNG_ERR_INVOKE,
    LNG_CLOSE
  };
  HWND NULL_HWND;
  FarStandardFunctions m_fsf;
  LPCTSTR REG_Key;
  LPCTSTR REG_WaitToContinue;
  LPCTSTR REG_UseGUI;
  LPCTSTR REG_DelUsingFar;
  LPCTSTR REG_ClearSel;
  LPCTSTR REG_Silent;
  LPCTSTR REG_Helptext;
  LPCTSTR REG_DifferentOnly;
  LPCTSTR REG_GuiPos;
  int m_UseGUI;
  int m_WaitToContinue;
  int m_DelUsingFar;
  int m_ClearSel;
  int m_Silent;
  EAdditionalStr m_enHelptext;
  int m_DifferentOnly;
  int m_GuiPos;
  LPSHELLFOLDER m_pDesktop;
  enum
  {
    MENUID_CANCELLED=0,
    MENUID_BACK=1,
    MENUID_SENDTO_DONE=2,
    MENUID_CMDOFFSET=3,
    MENUID_SENDTO_WIN95NT4=MENUID_CMDOFFSET+91,
    MENUID_SENDTO_WIN98=MENUID_CMDOFFSET+27,
    MENUID_SENDTO_WINME=MENUID_CMDOFFSET+28,
  };
  bool m_bWin95;
  bool m_bWin98;
  bool m_bWinME;
  bool m_bWin9x;
  bool m_bWinNT;
  bool m_bWin2K;
  LPCTSTR m_PluginMenuString;
  LPCTSTR m_PluginConfigString;
  int m_nShowMessId;
  int m_nSilentId;
  int m_nDifferentId;
};

extern CPlugin *thePlug;

#endif
