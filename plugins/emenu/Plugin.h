#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <plugin.hpp>
#include <shlobj.h>
#include "auto_sz.h"

#define PFX_RCLK L"rclk"
#define PFX_RCLK_TXT L"rclk_txt"
#define PFX_RCLK_GUI L"rclk_gui"
#define PFX_RCLK_CMD L"rclk_cmd"
#define PFX_RCLK_ITEM L"rclk_item"

class CPlugin : public PluginStartupInfo
{
  PluginPanelItem **SelectedItems;
  int SelectedItemsCount;
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
  enum CallMode
  {
    CALL_NORMAL,
    CALL_RIGHTCLICK,
    CALL_APPS,
  };
  EDoMenu OpenPluginBkg(int nOpenFrom, INT_PTR nItem);
  int Configure();
  void ExitFAR();
  HINSTANCE m_hModule;

protected:
  LPCWSTR GetMsg(int nMsgId);
  intptr_t Message(DWORD nFlags, LPCWSTR szHelpTopic, const LPCWSTR* pItems, int nItemsNumber, int nButtonsNumber);
  EDoMenu DoMenu(LPWSTR szCmdLine, CallMode Mode);
  EDoMenu SelectDrive();
  enum EAutoItem
  {
    AI_NONE,
    AI_VERB,
    AI_ITEM
  };
  EDoMenu MenuForPanelOrCmdLine(LPWSTR szCmdLine=NULL, EAutoItem enAutoItem=AI_NONE);
  EDoMenu DoMenu(LPSHELLFOLDER pCurFolder, LPCITEMIDLIST* pPiids, LPCWSTR pFiles[], unsigned nFiles, unsigned nFolders, LPCWSTR szCommand=NULL, EAutoItem enAutoItem=AI_NONE);
  bool ShowGuiMenu(HMENU hMenu, LPCONTEXTMENU pMenu1, LPCONTEXTMENU2 pMenu2, LPCONTEXTMENU3 pMenu, int* pnCmd);
  bool ShowTextMenu(HMENU hMenu, LPCONTEXTMENU pPreferredMenu, LPCONTEXTMENU2 pMenu2, LPCONTEXTMENU3 pMenu3, int* pnCmd, LPCWSTR szTitle, LPSHELLFOLDER pCurFolder, LPCITEMIDLIST* ppiid, unsigned nPiidCnt);
  bool ShowFolder(LPSHELLFOLDER pParentFolder, LPCITEMIDLIST piid, int* pnCmd, LPCWSTR szTitle, LPDROPTARGET* ppDropTarget);
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
  void Init();
  bool GetFilesFromParams(LPWSTR szCmdLine, LPCWSTR** ppFiles, unsigned* pnFiles, unsigned* pnFolders, auto_sz* pstrCurDir, bool bSkipFirst);
  bool GetFilesFromPanel(LPCWSTR** ppFiles, unsigned* pnFiles, unsigned* pnFolders, auto_sz* pstrCurDir);
  static bool IsSpace(wchar_t ch) {return L' '==ch || L'\t'==ch;}
  unsigned ParseParams(LPWSTR szParams, LPCWSTR* pFiles=NULL);
  void ReadRegValues();
  static intptr_t WINAPI CfgDlgProcStatic(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2);
  void CfgDlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2);
  enum EAdditionalStr {AS_NONE=0, AS_HELPTEXT=1, AS_VERB=2};
  bool GetAdditionalString(IContextMenu* pContextMenu, UINT nID, EAdditionalStr enAdditionalString, auto_sz* pstr);
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
    LNG_ERR_DIFFERENT_FOLDERS,
    LNG_ERR_SHOW,
    LNG_ERR_INVOKE,
    LNG_CLOSE
  };
  HWND NULL_HWND;
  FarStandardFunctions m_fsf;
  LPCWSTR REG_WaitToContinue;
  LPCWSTR REG_UseGUI;
  LPCWSTR REG_DelUsingFar;
  LPCWSTR REG_ClearSel;
  LPCWSTR REG_Silent;
  LPCWSTR REG_Helptext;
  LPCWSTR REG_DifferentOnly;
  LPCWSTR REG_GuiPos;
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
  bool m_bWin2K;
  LPCWSTR m_PluginMenuString;
  LPCWSTR m_PluginConfigString;
  int m_nShowMessId;
  int m_nSilentId;
  int m_nDifferentId;
};

extern CPlugin *thePlug;

#endif
