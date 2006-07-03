/*
config.cpp

������������

*/

/* Revision: 1.220 04.07.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "lang.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "panel.hpp"
#include "help.hpp"
#include "filter.hpp"
#include "poscache.hpp"
#include "findfile.hpp"
#include "hilight.hpp"

/* $ 03.08.2000 SVS
   ����������� ����� ������������
*/
static const wchar_t *WordDiv0 = L"~!%^&*()+|{}:\"<>?`-=\\[];',./";
/* SVS $ */

/* $ 12.10.2000 IS
   ����������� ����� ������������ ��� ������� Xlat
*/
static const wchar_t *WordDivForXlat0=L" \t!#$%^&*()+|=\\/@?";
/* IS $ */

string strPersonalPluginsPath;
string strKeyNameConsoleDetachKey;
static const wchar_t szCtrlShiftX[]=L"CtrlShiftX";
static const wchar_t szCtrlDot[]=L"Ctrl.";
static const wchar_t szCtrlShiftDot[]=L"CtrlShift.";

// KeyName
const wchar_t NKeyColorsW[]=L"Colors";
const wchar_t NKeyScreenW[]=L"Screen";
const wchar_t NKeyInterfaceW[]=L"Interface";
const wchar_t NKeyViewerW[]=L"Viewer";
const wchar_t NKeyDialogW[]=L"Dialog";
const wchar_t NKeyEditorW[]=L"Editor";
const wchar_t NKeyXLatW[]=L"XLat";
const wchar_t NKeySystemW[]=L"System";
const wchar_t NKeySystemExecutorW[]=L"System\\Executor";
const wchar_t NKeySystemNowellW[]=L"System\\Nowell";
const wchar_t NKeyHelpW[]=L"Help";
const wchar_t NKeyLanguageW[]=L"Language";
const wchar_t NKeyConfirmationsW[]=L"Confirmations";
const wchar_t NKeyPanelW[]=L"Panel";
const wchar_t NKeyPanelLeftW[]=L"Panel\\Left";
const wchar_t NKeyPanelRightW[]=L"Panel\\Right";
const wchar_t NKeyPanelLayoutW[]=L"Panel\\Layout";
const wchar_t NKeyPanelTreeW[]=L"Panel\\Tree";
const wchar_t NKeyLayoutW[]=L"Layout";
const wchar_t NKeyDescriptionsW[]=L"Descriptions";
const wchar_t NKeyKeyMacrosW[]=L"KeyMacros";
const wchar_t NKeyPoliciesW[]=L"Policies";
const wchar_t NKeyFileFilterW[]=L"OperationsFilter";
const wchar_t NKeySavedHistoryW[]=L"SavedHistory";
const wchar_t NKeySavedViewHistoryW[]=L"SavedViewHistory";
const wchar_t NKeySavedFolderHistoryW[]=L"SavedFolderHistory";
const wchar_t NKeySavedDialogHistoryW[]=L"SavedDialogHistory";
const wchar_t NParamHistoryCountW[]=L"HistoryCount";

const wchar_t *constBatchExtW=L".BAT;.CMD;";

void SystemSettings()
{
  const wchar_t *HistoryName=L"PersPath";
  string strPersonalPluginsPath;
  /* $ 15.07.2000 SVS
     + ������� � ���� ������� ��������������� ���� ��� ������ ��������
  */
  struct DialogDataEx CfgDlgData[]={
  /* 00 */ DI_DOUBLEBOX,3,1,52,20,0,0,0,0,(const wchar_t *)MConfigSystemTitle,
  /* 01 */ DI_CHECKBOX,5,2,0,0,1,0,0,0,(const wchar_t *)MConfigRO,
  /* 02 */ DI_CHECKBOX,5,3,0,0,0,0,0,0,(const wchar_t *)MConfigRecycleBin,
  /* 03 */ DI_CHECKBOX,5,4,0,0,0,0,0,0,(const wchar_t *)MConfigSystemCopy,
  /* 04 */ DI_CHECKBOX,5,5,0,0,0,0,0,0,(const wchar_t *)MConfigCopySharing,
  /* 05 */ DI_CHECKBOX,5,6,0,0,0,0,0,0,(const wchar_t *)MConfigScanJunction,
  /* 06 */ DI_CHECKBOX,5,7,0,0,0,0,0,0,(const wchar_t *)MConfigCreateUppercaseFolders,
  /* 07 */ DI_CHECKBOX,5,8,0,0,0,0,DIF_AUTOMATION,0,(const wchar_t *)MConfigInactivity,
  /* 08 */ DI_FIXEDIT,9,9,11,9,0,0,0,0,L"",
  /* 09 */ DI_TEXT,13,9,0,0,0,0,0,0,(const wchar_t *)MConfigInactivityMinutes,
  /* 10 */ DI_CHECKBOX,5,10,0,0,0,0,0,0,(const wchar_t *)MConfigSaveHistory,
  /* 11 */ DI_CHECKBOX,5,11,0,0,0,0,0,0,(const wchar_t *)MConfigSaveFoldersHistory,
  /* 12 */ DI_CHECKBOX,5,12,0,0,0,0,0,0,(const wchar_t *)MConfigSaveViewHistory,
  /* 13 */ DI_CHECKBOX,5,13,0,0,0,0,0,0,(const wchar_t *)MConfigRegisteredTypes,
  /* 14 */ DI_CHECKBOX,5,14,0,0,0,0,0,0,(const wchar_t *)MConfigCloseCDGate,
  /* 15 */ DI_TEXT,5,15,0,0,0,0,0,0,(const wchar_t *)MConfigPersonalPath,
  /* 16 */ DI_EDIT,5,16,50,16,0,(DWORD)HistoryName,DIF_HISTORY,0,L"",
  /* 17 */ DI_CHECKBOX,5,17,0,0,0,0,0,0,(const wchar_t *)MConfigAutoSave,
  /* 18 */ DI_TEXT,5,18,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 19 */ DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 20 */ DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
  };
  MakeDialogItemsEx(CfgDlgData,CfgDlg);

  CfgDlg[1].Selected=Opt.ClearReadOnly;
  CfgDlg[2].Selected=Opt.DeleteToRecycleBin;
  CfgDlg[3].Selected=Opt.CMOpt.UseSystemCopy;
  CfgDlg[4].Selected=Opt.CMOpt.CopyOpened;
  if (!RegVer)
  {
    CfgDlg[5].Flags|=DIF_DISABLE;
    CfgDlg[5].Selected=0;
  }
  else
  {
    CfgDlg[5].Selected=Opt.ScanJunction;
  }


  CfgDlg[6].Selected=Opt.CreateUppercaseFolders;
  CfgDlg[7].Selected=Opt.InactivityExit;

  CfgDlg[8].strData.Format (L"%d", Opt.InactivityExitTime);

  if(!Opt.InactivityExit)
  {
    CfgDlg[8].Flags|=DIF_DISABLE;
    CfgDlg[9].Flags|=DIF_DISABLE;
  }

  CfgDlg[10].Selected=Opt.SaveHistory;
  CfgDlg[11].Selected=Opt.SaveFoldersHistory;
  CfgDlg[12].Selected=Opt.SaveViewHistory;
  CfgDlg[13].Selected=Opt.UseRegisteredTypes;
  CfgDlg[14].Selected=Opt.CloseCDGate;

  CfgDlg[16].strData = Opt.LoadPlug.strPersonalPluginsPath;

  CfgDlg[17].Selected=Opt.AutoSaveSetup;

  {
    Dialog Dlg((DialogItemEx*)CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp(L"SystemSettings");
    Dlg.SetPosition(-1,-1,56,22);
    Dlg.SetAutomation(7,8,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.SetAutomation(7,9,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.Process();
    if (Dlg.GetExitCode()!=19)
      return;
  }

  Opt.ClearReadOnly=CfgDlg[1].Selected;
  Opt.DeleteToRecycleBin=CfgDlg[2].Selected;
  Opt.CMOpt.UseSystemCopy=CfgDlg[3].Selected;
  Opt.CMOpt.CopyOpened=CfgDlg[4].Selected;
  Opt.ScanJunction=CfgDlg[5].Selected;
  Opt.CreateUppercaseFolders=CfgDlg[6].Selected;
  Opt.InactivityExit=CfgDlg[7].Selected;

  if ((Opt.InactivityExitTime=_wtoi(CfgDlg[8].strData))<=0)
    Opt.InactivityExit=Opt.InactivityExitTime=0;

  Opt.SaveHistory=CfgDlg[10].Selected;
  Opt.SaveFoldersHistory=CfgDlg[11].Selected;
  Opt.SaveViewHistory=CfgDlg[12].Selected;
  Opt.UseRegisteredTypes=CfgDlg[13].Selected;
  Opt.CloseCDGate=CfgDlg[14].Selected;
  Opt.AutoSaveSetup=CfgDlg[17].Selected;

  Opt.LoadPlug.strPersonalPluginsPath = CfgDlg[16].strData;
}


#define DLG_PANEL_HIDDEN              1
#define DLG_PANEL_HIGHLIGHT           2
#define DLG_PANEL_CHANGEFOLDER        3
#define DLG_PANEL_SELECTFOLDERS       4
#define DLG_PANEL_SORTFOLDEREXT       5
#define DLG_PANEL_REVERSESORT         6
#define DLG_PANEL_AUTOUPDATELIMIT     7
#define DLG_PANEL_AUTOUPDATELIMIT2    8
#define DLG_PANEL_AUTOUPDATELIMITVAL  9
#define DLG_PANEL_AUTOUPDATEREMOTE   10

#define DLG_PANEL_SHOWCOLUMNTITLES   12
#define DLG_PANEL_SHOWPANELSTATUS    13
#define DLG_PANEL_SHOWPANELTOTALS    14
#define DLG_PANEL_SHOWPANELFREE      15
#define DLG_PANEL_SHOWPANELSCROLLBAR 16
#define DLG_PANEL_SHOWSCREENSNUMBER  17
#define DLG_PANEL_SHOWSORTMODE       18
#define DLG_PANEL_OK                 20

void PanelSettings()
{
  static struct DialogDataEx CfgDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,52,21,0,0,0,0,(const wchar_t *)MConfigPanelTitle,
  /* 01 */DI_CHECKBOX,5,2,0,0,1,0,0,0,(const wchar_t *)MConfigHidden,
  /* 02 */DI_CHECKBOX,5,3,0,0,0,0,0,0,(const wchar_t *)MConfigHighlight,
  /* 03 */DI_CHECKBOX,5,4,0,0,0,0,0,0,(const wchar_t *)MConfigAutoChange,
  /* 04 */DI_CHECKBOX,5,5,0,0,0,0,0,0,(const wchar_t *)MConfigSelectFolders,
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(const wchar_t *)MConfigSortFolderExt,
  /* 06 */DI_CHECKBOX,5,7,0,0,0,0,0,0,(const wchar_t *)MConfigReverseSort,
  /* 07 */DI_CHECKBOX,5,8,0,0,0,0,DIF_AUTOMATION,0,(const wchar_t *)MConfigAutoUpdateLimit,
  /* 08 */DI_TEXT,9,9,0,0,0,0,0,0,(const wchar_t *)MConfigAutoUpdateLimit2,
  /* 09 */DI_EDIT,9,9,15,8,0,0,0,0,L"",
  /* 10 */DI_CHECKBOX,5,10,0,0,0,0,0,0,(const wchar_t *)MConfigAutoUpdateRemoteDrive,
  /* 11 */DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 12 */DI_CHECKBOX,5,12,0,0,0,0,0,0,(const wchar_t *)MConfigShowColumns,
  /* 13 */DI_CHECKBOX,5,13,0,0,0,0,0,0,(const wchar_t *)MConfigShowStatus,
  /* 14 */DI_CHECKBOX,5,14,0,0,0,0,0,0,(const wchar_t *)MConfigShowTotal,
  /* 15 */DI_CHECKBOX,5,15,0,0,0,0,0,0,(const wchar_t *)MConfigShowFree,
  /* 16 */DI_CHECKBOX,5,16,0,0,0,0,0,0,(const wchar_t *)MConfigShowScrollbar,
  /* 17 */DI_CHECKBOX,5,17,0,0,0,0,0,0,(const wchar_t *)MConfigShowScreensNumber,
  /* 18 */DI_CHECKBOX,5,18,0,0,0,0,0,0,(const wchar_t *)MConfigShowSortMode,
  /* 19 */DI_TEXT,3,19,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 20 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 21 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
  };
  MakeDialogItemsEx(CfgDlgData,CfgDlg);

  CfgDlg[DLG_PANEL_HIDDEN].Selected=Opt.ShowHidden;
  CfgDlg[DLG_PANEL_HIGHLIGHT].Selected=Opt.Highlight;
  CfgDlg[DLG_PANEL_CHANGEFOLDER].Selected=Opt.Tree.AutoChangeFolder;
  CfgDlg[DLG_PANEL_SELECTFOLDERS].Selected=Opt.SelectFolders;
  CfgDlg[DLG_PANEL_SORTFOLDEREXT].Selected=Opt.SortFolderExt;
  CfgDlg[DLG_PANEL_REVERSESORT].Selected=Opt.ReverseSort;

  CfgDlg[DLG_PANEL_SHOWCOLUMNTITLES].Selected=Opt.ShowColumnTitles;
  CfgDlg[DLG_PANEL_SHOWPANELSTATUS].Selected=Opt.ShowPanelStatus;
  CfgDlg[DLG_PANEL_SHOWPANELTOTALS].Selected=Opt.ShowPanelTotals;
  CfgDlg[DLG_PANEL_SHOWPANELFREE].Selected=Opt.ShowPanelFree;
  CfgDlg[DLG_PANEL_SHOWPANELSCROLLBAR].Selected=Opt.ShowPanelScrollbar;
  CfgDlg[DLG_PANEL_SHOWSCREENSNUMBER].Selected=Opt.ShowScreensNumber;
  CfgDlg[DLG_PANEL_SHOWSORTMODE].Selected=Opt.ShowSortMode;
  CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].X1+=strlen(MSG(MConfigAutoUpdateLimit2))+1;
  CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].X2+=strlen(MSG(MConfigAutoUpdateLimit2))+1;
  CfgDlg[DLG_PANEL_AUTOUPDATELIMIT].Selected=Opt.AutoUpdateLimit!=0;

  if (!RegVer)
  {
    CfgDlg[DLG_PANEL_AUTOUPDATELIMIT2].Flags|=DIF_DISABLE;
    CfgDlg[DLG_PANEL_AUTOUPDATELIMIT].Flags|=DIF_DISABLE;
    CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].Flags|=DIF_DISABLE;
    CfgDlg[DLG_PANEL_AUTOUPDATEREMOTE].Selected=Opt.AutoUpdateRemoteDrive=1;
    CfgDlg[DLG_PANEL_AUTOUPDATEREMOTE].Flags|=DIF_DISABLE;
  }
  else
  {
    CfgDlg[DLG_PANEL_AUTOUPDATEREMOTE].Selected=Opt.AutoUpdateRemoteDrive;
    CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].strData.Format (L"%u", Opt.AutoUpdateLimit);;
  }

  if(Opt.AutoUpdateLimit==0)
    CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].Flags|=DIF_DISABLE;

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp(L"PanelSettings");
    Dlg.SetPosition(-1,-1,56,23);
    Dlg.SetAutomation(DLG_PANEL_AUTOUPDATELIMIT,DLG_PANEL_AUTOUPDATELIMITVAL,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.Process();
    if (Dlg.GetExitCode() != DLG_PANEL_OK)
      return;
  }

  Opt.ShowHidden=CfgDlg[DLG_PANEL_HIDDEN].Selected;
  Opt.Highlight=CfgDlg[DLG_PANEL_HIGHLIGHT].Selected;
  Opt.Tree.AutoChangeFolder=CfgDlg[DLG_PANEL_CHANGEFOLDER].Selected;
  Opt.SelectFolders=CfgDlg[DLG_PANEL_SELECTFOLDERS].Selected;
  Opt.SortFolderExt=CfgDlg[DLG_PANEL_SORTFOLDEREXT].Selected;
  Opt.ReverseSort=CfgDlg[DLG_PANEL_REVERSESORT].Selected;
  if(!CfgDlg[DLG_PANEL_AUTOUPDATELIMIT].Selected)
    Opt.AutoUpdateLimit=0;
  else
  {
    wchar_t *endptr;
    Opt.AutoUpdateLimit=wcstoul(CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].strData, &endptr, 10);
  }
  Opt.ShowColumnTitles=CfgDlg[DLG_PANEL_SHOWCOLUMNTITLES].Selected;
  Opt.ShowPanelStatus=CfgDlg[DLG_PANEL_SHOWPANELSTATUS].Selected;
  Opt.ShowPanelTotals=CfgDlg[DLG_PANEL_SHOWPANELTOTALS].Selected;
  Opt.ShowPanelFree=CfgDlg[DLG_PANEL_SHOWPANELFREE].Selected;
  Opt.ShowPanelScrollbar=CfgDlg[DLG_PANEL_SHOWPANELSCROLLBAR].Selected;
  Opt.ShowScreensNumber=CfgDlg[DLG_PANEL_SHOWSCREENSNUMBER].Selected;
  Opt.ShowSortMode=CfgDlg[DLG_PANEL_SHOWSORTMODE].Selected;
  Opt.AutoUpdateRemoteDrive=CfgDlg[DLG_PANEL_AUTOUPDATEREMOTE].Selected;
//  FrameManager->RefreshFrame();
  CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->Redraw();

}


#define DLG_INTERF_CLOCK                1
#define DLG_INTERF_VIEWEREDITORCLOCK    2
#define DLG_INTERF_MOUSE                3
#define DLG_INTERF_MOUSEPMCLICKRULE     4
#define DLG_INTERF_SHOWKEYBAR           5
#define DLG_INTERF_SHOWMENUBAR          6
#define DLG_INTERF_SCREENSAVER          7
#define DLG_INTERF_SCREENSAVERTIME      8
#define DLG_INTERF_SAVERMINUTES         9
#define DLG_INTERF_USEPROMPTFORMAT     10
#define DLG_INTERF_PROMPTFORMAT        11
#define DLG_INTERF_ALTGR               12
#define DLG_INTERF_COPYSHOWTOTAL       13
#define DLG_INTERF_COPYTIMERULE        14
#define DLG_INTERF_PGUPCHANGEDISK      15
#define DLG_INTERF_OK                  17

/* $ 17.12.2001 IS
   ��������� ������� ������ ���� ��� �������. ������� ���� ����, ����� ����
   ��������� � ����������� ������ �� ���������������� ����.
*/
void InterfaceSettings()
{
  static struct DialogDataEx CfgDlgData[]={
    /* $ 04.07.2000 SVS
       + ���������� �� ScrollBar ��� Menu|Options|Interface settings
    */
    /* $ 26.07.2000 SVS
       + ��������� �� �������������� � ������� �����
    */
  /* 00 */DI_DOUBLEBOX,3,1,54,18,0,0,0,0,(const wchar_t *)MConfigInterfaceTitle,
  /* 01 */DI_CHECKBOX,5,2,0,0,1,0,0,0,(const wchar_t *)MConfigClock,
  /* 02 */DI_CHECKBOX,5,3,0,0,0,0,0,0,(const wchar_t *)MConfigViewerEditorClock,
  /* 03 */DI_CHECKBOX,5,4,0,0,0,0,DIF_AUTOMATION,0,(const wchar_t *)MConfigMouse,
  /* 04 */DI_CHECKBOX,9,5,0,0,0,0,0,0,(const wchar_t *)MConfigMousePanelMClickRule,
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(const wchar_t *)MConfigKeyBar,
  /* 06 */DI_CHECKBOX,5,7,0,0,0,0,0,0,(const wchar_t *)MConfigMenuBar,
  /* 07 */DI_CHECKBOX,5,8,0,0,0,0,DIF_AUTOMATION,0,(const wchar_t *)MConfigSaver,
  /* 08 */DI_FIXEDIT,9,9,11,8,0,0,0,0,L"",
  /* 09 */DI_TEXT,13,9,0,0,0,0,0,0,(const wchar_t *)MConfigSaverMinutes,
  /* 10 */DI_CHECKBOX,5,10,0,0,0,0,DIF_AUTOMATION,0,(const wchar_t *)MConfigUsePromptFormat,
  /* 11 */DI_EDIT,9,11,24,12,0,0,0,0,L"",
  /* 12 */DI_CHECKBOX,5,12,0,0,0,0,0,0,(const wchar_t *)MConfigAltGr,
  /* 13 */DI_CHECKBOX,5,13,0,0,0,0,0,0,(const wchar_t *)MConfigCopyTotal,
  /* 14 */DI_CHECKBOX,5,14,0,0,0,0,0,0,(const wchar_t *)MConfigCopyTimeRule,
  /* 15 */DI_CHECKBOX,5,15,0,0,0,0,0,0,(const wchar_t *)MConfigPgUpChangeDisk,
  /* 16 */DI_TEXT,3,16,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 17 */DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 18 */DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
  };
  MakeDialogItemsEx(CfgDlgData,CfgDlg);

  if (!RegVer)
  {
    CfgDlg[DLG_INTERF_VIEWEREDITORCLOCK].Flags|=DIF_DISABLE;
    CfgDlg[DLG_INTERF_VIEWEREDITORCLOCK].Selected=Opt.ViewerEditorClock=0;
  }
  else
    CfgDlg[DLG_INTERF_VIEWEREDITORCLOCK].Selected=Opt.ViewerEditorClock;

  CfgDlg[DLG_INTERF_CLOCK].Selected=Opt.Clock;
  CfgDlg[DLG_INTERF_MOUSE].Selected=Opt.Mouse;
  CfgDlg[DLG_INTERF_MOUSEPMCLICKRULE].Selected=Opt.PanelMiddleClickRule;
  if(!Opt.Mouse)
    CfgDlg[DLG_INTERF_MOUSEPMCLICKRULE].Flags|=DIF_DISABLE;
  CfgDlg[DLG_INTERF_SHOWKEYBAR].Selected=Opt.ShowKeyBar;
  CfgDlg[DLG_INTERF_SHOWMENUBAR].Selected=Opt.ShowMenuBar;
  CfgDlg[DLG_INTERF_SCREENSAVER].Selected=Opt.ScreenSaver;

  CfgDlg[DLG_INTERF_SCREENSAVERTIME].strData.Format (L"%u", Opt.ScreenSaverTime);

  if(!Opt.ScreenSaver)
  {
    CfgDlg[DLG_INTERF_SCREENSAVERTIME].Flags|=DIF_DISABLE;
    CfgDlg[DLG_INTERF_SAVERMINUTES].Flags|=DIF_DISABLE;
  }
  CfgDlg[DLG_INTERF_USEPROMPTFORMAT].Selected=Opt.UsePromptFormat;
  CfgDlg[DLG_INTERF_PROMPTFORMAT].strData = Opt.strPromptFormat;
  if(!Opt.UsePromptFormat)
    CfgDlg[DLG_INTERF_PROMPTFORMAT].Flags|=DIF_DISABLE;
  CfgDlg[DLG_INTERF_ALTGR].Selected=Opt.AltGr;
  CfgDlg[DLG_INTERF_COPYSHOWTOTAL].Selected=Opt.CMOpt.CopyShowTotal;

  CfgDlg[DLG_INTERF_COPYTIMERULE].Selected=Opt.CMOpt.CopyTimeRule!=0;

  CfgDlg[DLG_INTERF_PGUPCHANGEDISK].Selected=Opt.PgUpChangeDisk;

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp(L"InterfSettings");
    Dlg.SetPosition(-1,-1,58,20);
    Dlg.SetAutomation(DLG_INTERF_SCREENSAVER,DLG_INTERF_SCREENSAVERTIME,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.SetAutomation(DLG_INTERF_SCREENSAVER,DLG_INTERF_SAVERMINUTES,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.SetAutomation(DLG_INTERF_USEPROMPTFORMAT,DLG_INTERF_PROMPTFORMAT,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.SetAutomation(DLG_INTERF_MOUSE,DLG_INTERF_MOUSEPMCLICKRULE,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.Process();
    if (Dlg.GetExitCode() != DLG_INTERF_OK)
      return;
  }

  Opt.Clock=CfgDlg[DLG_INTERF_CLOCK].Selected;
  Opt.ViewerEditorClock=CfgDlg[DLG_INTERF_VIEWEREDITORCLOCK].Selected;
  Opt.Mouse=CfgDlg[DLG_INTERF_MOUSE].Selected;
  Opt.PanelMiddleClickRule=CfgDlg[DLG_INTERF_MOUSEPMCLICKRULE].Selected;
  Opt.ShowKeyBar=CfgDlg[DLG_INTERF_SHOWKEYBAR].Selected;
  Opt.ShowMenuBar=CfgDlg[DLG_INTERF_SHOWMENUBAR].Selected;
  Opt.ScreenSaver=CfgDlg[DLG_INTERF_SCREENSAVER].Selected;

  wchar_t *endptr;
  Opt.ScreenSaverTime=wcstoul(CfgDlg[DLG_INTERF_USEPROMPTFORMAT].strData, &endptr, 10);
  Opt.UsePromptFormat=CfgDlg[DLG_INTERF_USEPROMPTFORMAT].Selected;

  Opt.strPromptFormat = CfgDlg[DLG_INTERF_PROMPTFORMAT].strData;

  Opt.AltGr=CfgDlg[DLG_INTERF_ALTGR].Selected;
  Opt.CMOpt.CopyShowTotal=CfgDlg[DLG_INTERF_COPYSHOWTOTAL].Selected;
  Opt.PgUpChangeDisk=CfgDlg[DLG_INTERF_PGUPCHANGEDISK].Selected;
  Opt.CMOpt.CopyTimeRule=0;
  if(CfgDlg[DLG_INTERF_COPYTIMERULE].Selected)
    Opt.CMOpt.CopyTimeRule=3;

  SetFarConsoleMode();
  CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->SetScreenPosition();
  /*$ 10.07.2001 SKV
    ! ���� ��� ������, ����� ���� ������ ��������,
      ����� ������ ����.
  */
  CtrlObject->Cp()->Redraw();
  /* SKV$*/
}
/* IS 17.12.2001 $ */

void DialogSettings()
{
  #define DLG_DIALOGS_DIALOGSEDITHISTORY    1
  #define DLG_DIALOGS_DIALOGSEDITBLOCK      2
  #define DLG_DIALOGS_DIALOGDELREMOVESBLOCKS    3
  #define DLG_DIALOGS_AUTOCOMPLETE          4
  #define DLG_DIALOGS_EULBSCLEAR            5
  #define DLG_DIALOGS_MOUSEBUTTON           6
  #define DLG_DIALOGS_OK                    8

  static DialogDataEx CfgDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,54,9,0,0,0,0,(const wchar_t *)MConfigDlgSetsTitle,
  /* 01 */DI_CHECKBOX,5,2,0,0,0,0,0,0,(const wchar_t *)MConfigDialogsEditHistory,
  /* 02 */DI_CHECKBOX,5,3,0,0,0,0,0,0,(const wchar_t *)MConfigDialogsEditBlock,
  /* 03 */DI_CHECKBOX,5,4,0,0,0,0,0,0,(const wchar_t *)MConfigDialogsDelRemovesBlocks,
  /* 04 */DI_CHECKBOX,5,5,0,0,0,0,0,0,(const wchar_t *)MConfigDialogsAutoComplete,
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(const wchar_t *)MConfigDialogsEULBsClear,
  /* 06 */DI_CHECKBOX,5,7,0,0,0,0,0,0,(const wchar_t *)MConfigDialogsMouseButton,
  /* 07 */DI_TEXT,3,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 08 */DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 09 */DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
  };
  MakeDialogItemsEx(CfgDlgData,CfgDlg);

  CfgDlg[DLG_DIALOGS_DIALOGSEDITHISTORY].Selected=Opt.Dialogs.EditHistory;
  CfgDlg[DLG_DIALOGS_DIALOGSEDITBLOCK].Selected=Opt.Dialogs.EditBlock;
  CfgDlg[DLG_DIALOGS_DIALOGDELREMOVESBLOCKS].Selected=Opt.Dialogs.DelRemovesBlocks;
  CfgDlg[DLG_DIALOGS_AUTOCOMPLETE].Selected=Opt.Dialogs.AutoComplete;
  CfgDlg[DLG_DIALOGS_EULBSCLEAR].Selected=Opt.Dialogs.EULBsClear;
  CfgDlg[DLG_DIALOGS_MOUSEBUTTON].Selected=Opt.Dialogs.MouseButton;

  {
    Dialog Dlg((DialogItemEx*)CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp(L"DialogSettings");
    Dlg.SetPosition(-1,-1,58,12);
    Dlg.Process();
    if (Dlg.GetExitCode() != DLG_DIALOGS_OK)
      return;
  }

  Opt.Dialogs.EditHistory=CfgDlg[DLG_DIALOGS_DIALOGSEDITHISTORY].Selected;
  Opt.Dialogs.EditBlock=CfgDlg[DLG_DIALOGS_DIALOGSEDITBLOCK].Selected;
  Opt.Dialogs.DelRemovesBlocks=CfgDlg[DLG_DIALOGS_DIALOGDELREMOVESBLOCKS].Selected;
  Opt.Dialogs.AutoComplete=CfgDlg[DLG_DIALOGS_AUTOCOMPLETE].Selected;
  Opt.Dialogs.EULBsClear=CfgDlg[DLG_DIALOGS_EULBSCLEAR].Selected;
  if((Opt.Dialogs.MouseButton=CfgDlg[DLG_DIALOGS_MOUSEBUTTON].Selected) != 0)
    Opt.Dialogs.MouseButton=0xFFFF;

  CtrlObject->CmdLine->SetPersistentBlocks(Opt.Dialogs.EditBlock);
}

/* $ 09.02.2001 IS
   ����� Esc
*/
/* $ 15.03.2001 SVS
    ������������� �������� ��������� ������ �� ���� ������
*/
void SetConfirmations()
{
  static struct DialogDataEx ConfDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,46,16,0,0,0,0,(const wchar_t *)MSetConfirmTitle,
  /* 01 */DI_CHECKBOX,5,2,0,0,1,0,0,0,(const wchar_t *)MSetConfirmCopy,
  /* 02 */DI_CHECKBOX,5,3,0,0,0,0,0,0,(const wchar_t *)MSetConfirmMove,
  /* 03 */DI_CHECKBOX,5,4,0,0,0,0,0,0,(const wchar_t *)MSetConfirmDrag,
  /* 04 */DI_CHECKBOX,5,5,0,0,0,0,0,0,(const wchar_t *)MSetConfirmDelete,
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(const wchar_t *)MSetConfirmDeleteFolders,
  /* 06 */DI_CHECKBOX,5,7,0,0,0,0,0,0,(const wchar_t *)MSetConfirmEsc,
  /* 07 */DI_CHECKBOX,5,8,0,0,0,0,0,0,(const wchar_t *)MSetConfirmRemoveConnection,
  /* 08 */DI_CHECKBOX,5,9,0,0,0,0,0,0,(const wchar_t *)MSetConfirmRemoveSUBST,
  /* 09 */DI_CHECKBOX,5,10,0,0,0,0,0,0,(const wchar_t *)MSetConfirmRemoveHotPlug,
  /* 10 */DI_CHECKBOX,5,11,0,0,0,0,0,0,(const wchar_t *)MSetConfirmAllowReedit,
  /* 11 */DI_CHECKBOX,5,12,0,0,0,0,0,0,(const wchar_t *)MSetConfirmHistoryClear,
  /* 12 */DI_CHECKBOX,5,13,0,0,0,0,0,0,(const wchar_t *)MSetConfirmExit,
  /* 13 */DI_TEXT,3,14,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 14 */DI_BUTTON,0,15,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 15 */DI_BUTTON,0,15,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel

  };
  MakeDialogItemsEx(ConfDlgData,ConfDlg);
  ConfDlg[1].Selected=Opt.Confirm.Copy;
  ConfDlg[2].Selected=Opt.Confirm.Move;
  ConfDlg[3].Selected=Opt.Confirm.Drag;
  ConfDlg[4].Selected=Opt.Confirm.Delete;
  ConfDlg[5].Selected=Opt.Confirm.DeleteFolder;
  ConfDlg[6].Selected=Opt.Confirm.Esc;
  ConfDlg[7].Selected=Opt.Confirm.RemoveConnection;
  ConfDlg[8].Selected=Opt.Confirm.RemoveSUBST;
  ConfDlg[9].Selected=Opt.Confirm.RemoveHotPlug;
  ConfDlg[10].Selected=Opt.Confirm.AllowReedit;
  ConfDlg[11].Selected=Opt.Confirm.HistoryClear;
  ConfDlg[12].Selected=Opt.Confirm.Exit;

  Dialog Dlg(ConfDlg,sizeof(ConfDlg)/sizeof(ConfDlg[0]));
  Dlg.SetHelp(L"ConfirmDlg");
  Dlg.SetPosition(-1,-1,50,18);
  Dlg.Process();
  if (Dlg.GetExitCode()!=14)
    return;
  Opt.Confirm.Copy=ConfDlg[1].Selected;
  Opt.Confirm.Move=ConfDlg[2].Selected;
  Opt.Confirm.Drag=ConfDlg[3].Selected;
  Opt.Confirm.Delete=ConfDlg[4].Selected;
  Opt.Confirm.DeleteFolder=ConfDlg[5].Selected;
  Opt.Confirm.Esc=ConfDlg[6].Selected;
  Opt.Confirm.RemoveConnection=ConfDlg[7].Selected;
  Opt.Confirm.RemoveSUBST=ConfDlg[8].Selected;
  Opt.Confirm.RemoveHotPlug=ConfDlg[9].Selected;
  Opt.Confirm.AllowReedit=ConfDlg[10].Selected;
  Opt.Confirm.HistoryClear=ConfDlg[11].Selected;
  Opt.Confirm.Exit=ConfDlg[12].Selected;
}
/* SVS $ */
/* IS $ */

void SetDizConfig()
{
  static struct DialogDataEx DizDlgData[]=
  {
  /* 00 */DI_DOUBLEBOX,3,1,72,14,0,0,0,0,(const wchar_t *)MCfgDizTitle,
  /* 01 */DI_TEXT,5,2,0,0,0,0,0,0,(const wchar_t *)MCfgDizListNames,
  /* 02 */DI_EDIT,5,3,70,3,1,0,0,0,L"",
  /* 03 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 04 */DI_CHECKBOX,5,5,0,0,0,0,0,0,(const wchar_t *)MCfgDizSetHidden,
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(const wchar_t *)MCfgDizROUpdate,
  /* 06 */DI_FIXEDIT,5,7,7,7,0,0,0,0,L"",
  /* 07 */DI_TEXT,9,7,0,0,0,0,0,0,(const wchar_t *)MCfgDizStartPos,
  /* 08 */DI_TEXT,3,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 09 */DI_RADIOBUTTON,5,9,0,0,0,0,DIF_GROUP,0,(const wchar_t *)MCfgDizNotUpdate,
  /* 10 */DI_RADIOBUTTON,5,10,0,0,0,0,0,0,(const wchar_t *)MCfgDizUpdateIfDisplayed,
  /* 11 */DI_RADIOBUTTON,5,11,0,0,0,0,0,0,(const wchar_t *)MCfgDizAlwaysUpdate,
  /* 12 */DI_TEXT,3,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 13 */DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 14 */DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
  };
  MakeDialogItemsEx(DizDlgData,DizDlg);

  Dialog Dlg((DialogItemEx*)DizDlg,sizeof(DizDlg)/sizeof(DizDlg[0]));
  Dlg.SetPosition(-1,-1,76,16);
  Dlg.SetHelp(L"FileDiz");

  DizDlg[2].strData = Opt.Diz.strListNames;

  if (Opt.Diz.UpdateMode==DIZ_NOT_UPDATE)
    DizDlg[9].Selected=TRUE;
  else
    if (Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED)
      DizDlg[10].Selected=TRUE;
    else
      DizDlg[11].Selected=TRUE;

  DizDlg[4].Selected=Opt.Diz.SetHidden;
  DizDlg[5].Selected=Opt.Diz.ROUpdate;

  DizDlg[6].strData.Format (L"%d", Opt.Diz.StartPos);

  Dlg.Process();
  if (Dlg.GetExitCode()!=13)
    return;

  Opt.Diz.strListNames = DizDlg[2].strData;

  if (DizDlg[9].Selected)
    Opt.Diz.UpdateMode=DIZ_NOT_UPDATE;
  else
    if (DizDlg[10].Selected)
      Opt.Diz.UpdateMode=DIZ_UPDATE_IF_DISPLAYED;
    else
      Opt.Diz.UpdateMode=DIZ_UPDATE_ALWAYS;
  Opt.Diz.SetHidden=DizDlg[4].Selected;
  Opt.Diz.ROUpdate=DizDlg[5].Selected;
  Opt.Diz.StartPos=_wtoi(DizDlg[6].strData);
}

void ViewerConfig(struct ViewerOptions &ViOpt,int Local)
{
/* $ 18.07.2000 tran
   ��������� ���� ����� ���������� ��� ������� */
/*
 +----------------- Viewer ------------------+
 | + External viewer ----------------------+ |
 | | ( ) Use for F3                        | |
 | | () Use for Alt-F3                    | |
 | | Viewer command:                       | |
 | |                                       | |
 | +---------------------------------------+ |
 | + Internal viewer ----------------------+ |
 | | [x] Save file position                | |
 | | [x] Save shortcut position            | |
 | | [x] Autodetect character table        | |
 | | 8   Tab size                          | |
 | | [x] Show scrollbar                    | |
 | | [x] Show arrows                       | |
 | +---------------------------------------+ |
 |            [ Ok ]  [ Cancel ]             |
 +-------------------------------------------+
*/

  enum enumViewerConfig {
      ID_VC_TITLE,
      ID_VC_EXTERNALCONFIGTITLE,
      ID_VC_EXTERNALUSEF3,
      ID_VC_EXTERNALUSEALTF3,
      ID_VC_EXTENALCOMMAND,
      ID_VC_EXTERALCOMMANDEDIT,
      ID_VC_SEPARATOR1,
      ID_VC_INTERNALCONFIGTITLE,
      ID_VC_SAVEPOSITION,
      ID_VC_SAVEBOOKMARKS,
      ID_VC_AUTODETECTTABLE,
      ID_VC_TABSIZEEDIT,
      ID_VC_TABSIZE,
      ID_VC_SHOWSCROLLBAR,
      ID_VC_SHOWARROWS,
      ID_VC_PERSISTENTSELECTION,
      ID_VC_ANSIASDEFAULT,
      ID_VC_SEPARATOR2,
      ID_VC_OK,
      ID_VC_CANCEL
  };

  static struct DialogDataEx CfgDlgData[]={
  /*  0 */  DI_DOUBLEBOX , 3, 1,70,19,0,0,0,0,(const wchar_t *)MViewConfigTitle,
  /*  1 */  DI_TEXT, 5, 2,68, 7,0,0,DIF_LEFTTEXT,0,(const wchar_t *)MViewConfigExternal,
  /*  2 */  DI_RADIOBUTTON,6, 3, 0, 0,1,0,DIF_GROUP,0,(const wchar_t *)MViewConfigExternalF3,
  /*  3 */  DI_RADIOBUTTON,6, 4, 0, 0,0,0,0,0,(const wchar_t *)MViewConfigExternalAltF3,
  /*  4 */  DI_TEXT      , 6, 5, 0, 0,0,0,0,0,(const wchar_t *)MViewConfigExternalCommand,
  /*  5 */  DI_EDIT      , 6, 6,68, 6,0,(DWORD)L"ExternalViewer", DIF_HISTORY,0,L"",
  /*  6 */  DI_TEXT, 0, 7, 0, 0, 0, 0, DIF_SEPARATOR, 0, L"",
  /*  7 */  DI_TEXT, 5, 8,68,16,0,0,DIF_LEFTTEXT,0,(const wchar_t *)MViewConfigInternal,
  /*  8 */  DI_CHECKBOX  , 6, 9, 0, 0,0,0,0,0,(const wchar_t *)MViewConfigSavePos,
  /*  9 */  DI_CHECKBOX  , 6,10, 0, 0,0,0,0,0,(const wchar_t *)MViewConfigSaveShortPos,
  /* 10 */  DI_CHECKBOX  , 6,11, 0, 0,0,0,0,0,(const wchar_t *)MViewAutoDetectTable,
  /* 11 */  DI_FIXEDIT   , 6,12, 9,15,0,0,0,0,L"",
  /* 12 */  DI_TEXT      ,11,12, 0, 0,0,0,0,0,(const wchar_t *)MViewConfigTabSize,
  /* 13 */  DI_CHECKBOX  , 6,13, 0, 0,0,0,0,0,(const wchar_t *)MViewConfigScrollbar,
  /* 14 */  DI_CHECKBOX  , 6,14, 0, 0,0,0,0,0,(const wchar_t *)MViewConfigArrows,
  /* 15 */  DI_CHECKBOX  , 6,15, 0, 0,0,0,0,0,(const wchar_t *)MViewConfigPersistentSelection,
  /* 16 */  DI_CHECKBOX  , 6,16, 0, 0,0,0,0,0,(const wchar_t *)MViewConfigAnsiTableAsDefault,
  /* 17 */  DI_TEXT, 0, 17, 0, 0, 0, 0, DIF_SEPARATOR, 0, L"",
  /* 18 */  DI_BUTTON    , 0,18, 0, 0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 19 */  DI_BUTTON    , 0,18, 0, 0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
  };

  MakeDialogItemsEx(CfgDlgData,CfgDlg);

  CfgDlg[ID_VC_EXTERNALUSEF3].Selected = Opt.ViOpt.UseExternalViewer;
  CfgDlg[ID_VC_EXTERNALUSEALTF3].Selected = !Opt.ViOpt.UseExternalViewer;
  CfgDlg[ID_VC_SAVEPOSITION].Selected = Opt.ViOpt.SaveViewerPos;
  CfgDlg[ID_VC_SAVEBOOKMARKS].Selected = Opt.ViOpt.SaveViewerShortPos;
  CfgDlg[ID_VC_AUTODETECTTABLE].Selected = ViOpt.AutoDetectTable && DistrTableExist();
  CfgDlg[ID_VC_SHOWSCROLLBAR].Selected = ViOpt.ShowScrollbar;
  CfgDlg[ID_VC_SHOWARROWS].Selected = ViOpt.ShowArrows;
  CfgDlg[ID_VC_PERSISTENTSELECTION].Selected = ViOpt.PersistentBlocks;
  CfgDlg[ID_VC_ANSIASDEFAULT].Selected = ViOpt.AnsiTableAsDefault;

  CfgDlg[ID_VC_EXTERALCOMMANDEDIT].strData = Opt.strExternalViewer;
  CfgDlg[ID_VC_TABSIZEEDIT].strData.Format (L"%d",ViOpt.TabSize);

  if ( !RegVer )
  {
    CfgDlg[ID_VC_TABSIZEEDIT].Flags |= DIF_DISABLE;
    CfgDlg[ID_VC_TABSIZE].Flags |= DIF_DISABLE;
  }

  int DialogHeight = 21;

  if (Local)
  {
    int i;

    for (i=ID_VC_EXTERNALCONFIGTITLE; i<=ID_VC_SEPARATOR1; i++)
      CfgDlg[i].Flags |= DIF_HIDDEN;

    for (i = ID_VC_INTERNALCONFIGTITLE; i <= ID_VC_CANCEL; i++)
      CfgDlg[i].Y1 -= 6;

    CfgDlg[ID_VC_TITLE].Y2 -= 6;
    DialogHeight -= 6;
  }

  {
    Dialog Dlg((DialogItemEx*)CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp(L"ViewerSettings");
    Dlg.SetPosition(-1,-1,74,DialogHeight);
    Dlg.Process();
    if (Dlg.GetExitCode()!= ID_VC_OK)
      return;
  }

  if (!Local)
  {
    Opt.ViOpt.UseExternalViewer=CfgDlg[ID_VC_EXTERNALUSEF3].Selected;
    Opt.strExternalViewer = CfgDlg[ID_VC_EXTERALCOMMANDEDIT].strData;
  }
  Opt.ViOpt.SaveViewerPos=CfgDlg[ID_VC_SAVEPOSITION].Selected;
  Opt.ViOpt.SaveViewerShortPos=CfgDlg[ID_VC_SAVEBOOKMARKS].Selected;

  ViOpt.AutoDetectTable=CfgDlg[ID_VC_AUTODETECTTABLE].Selected;

  if(!DistrTableExist() && ViOpt.AutoDetectTable)
  {
    ViOpt.AutoDetectTable=0;
    MessageW(MSG_WARNING,1,UMSG(MWarning),
              UMSG(MDistributionTableWasNotFound),UMSG(MAutoDetectWillNotWork),
              UMSG(MOk));
  }
  /* IS $ */
  ViOpt.TabSize=_wtoi(CfgDlg[ID_VC_TABSIZEEDIT].strData);
  ViOpt.ShowScrollbar=CfgDlg[ID_VC_SHOWSCROLLBAR].Selected;
  ViOpt.ShowArrows=CfgDlg[ID_VC_SHOWARROWS].Selected;
  ViOpt.PersistentBlocks=CfgDlg[ID_VC_PERSISTENTSELECTION].Selected;
  ViOpt.AnsiTableAsDefault=CfgDlg[ID_VC_ANSIASDEFAULT].Selected;
  if (ViOpt.TabSize<1 || ViOpt.TabSize>512)
    ViOpt.TabSize=8;
}

void EditorConfig(struct EditorOptions &EdOpt,int Local)
{
  enum enumEditorConfig {
    ID_EC_TITLE,
    ID_EC_EXTERNALCONFIGTITLE,
    ID_EC_EXTERNALUSEF4,
    ID_EC_EXTERNALUSEALTF4,
    ID_EC_EXTERNALCOMMAND,
    ID_EC_EXTERNALCOMMANDEDIT,
    ID_EC_SEPARATOR1,
    ID_EC_INTERNALCONFIGTITLE,
    ID_EC_EXPANDTABSTITLE,
    ID_EC_EXPANDTABS,
    ID_EC_PERSISTENTBLOCKS,
    ID_EC_DELREMOVESBLOCKS,
    ID_EC_SAVEPOSITION,
    ID_EC_SAVEBOOKMARKS,
    ID_EC_AUTOINDENT,
    ID_EC_AUTODETECTTABLE,
    ID_EC_CURSORBEYONDEOL,
    ID_EC_LOCKREADONLY,
    ID_EC_READONLYWARNING,
    ID_EC_TABSIZEEDIT,
    ID_EC_TABSIZE,
    ID_EC_ANSIASDEFAULT,
    ID_EC_ANSIFORNEWFILE,
    ID_EC_SEPARATOR2,
    ID_EC_OK,
    ID_EC_CANCEL
  };

  static struct DialogDataEx CfgDlgData[]={
  /*  0 */  DI_DOUBLEBOX,3,1,70,22,0,0,0,0,(const wchar_t *)MEditConfigTitle,
  /*  1 */  DI_TEXT,5,2,0,0,0,0,DIF_LEFTTEXT,0,(const wchar_t *)MEditConfigExternal,
  /*  2 */  DI_RADIOBUTTON,6,3,0,0,1,0,DIF_GROUP,0,(const wchar_t *)MEditConfigEditorF4,
  /*  3 */  DI_RADIOBUTTON,6,3,0,0,0,0,0,0,(const wchar_t *)MEditConfigEditorAltF4,
  /*  4 */  DI_TEXT,6,4,0,0,0,0,0,0,(const wchar_t *)MEditConfigEditorCommand,
  /*  5 */  DI_EDIT,6,5,68,5,0,(DWORD)L"ExternalEditor",DIF_HISTORY,0,L"",
  /*  6 */  DI_TEXT, 0, 6, 0, 0, 0, 0, DIF_SEPARATOR, 0, L"",
  /*  7 */  DI_TEXT,5,7,0,0,0,0,DIF_LEFTTEXT,0,(const wchar_t *)MEditConfigInternal,
  /*  8 */  DI_TEXT,6,8,0,0,0,0,0,0,(const wchar_t *)MEditConfigExpandTabsTitle,
  /*  9 */  DI_COMBOBOX,6,9,68,0,1,0,DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE,0,L"",
  /* 10 */  DI_CHECKBOX,6,10,0,0,0,0,0,0,(const wchar_t *)MEditConfigPersistentBlocks,
  /* 11 */  DI_CHECKBOX,6,10,0,0,0,0,0,0,(const wchar_t *)MEditConfigDelRemovesBlocks,
  /* 12 */  DI_CHECKBOX,6,11,0,0,0,0,0,0,(const wchar_t *)MEditConfigSavePos,
  /* 13 */  DI_CHECKBOX,6,11,0,0,0,0,0,0,(const wchar_t *)MEditConfigSaveShortPos,
  /* 14 */  DI_CHECKBOX,6,12,0,0,0,0,0,0,(const wchar_t *)MEditConfigAutoIndent,
  /* 15 */  DI_CHECKBOX,6,13,0,0,0,0,0,0,(const wchar_t *)MEditAutoDetectTable,
  /* 16 */  DI_CHECKBOX,6,14,0,0,0,0,0,0,(const wchar_t *)MEditCursorBeyondEnd,
  /* 17 */  DI_CHECKBOX,6,15,0,0,0,0,0,0,(const wchar_t *)MEditLockROFileModification,
  /* 18 */  DI_CHECKBOX,6,16,0,0,0,0,0,0,(const wchar_t *)MEditWarningBeforeOpenROFile,
  /* 19 */  DI_FIXEDIT,6,17,9,20,0,0,0,0,L"",
  /* 20 */  DI_TEXT,11,17,0,0,0,0,0,0,(const wchar_t *)MEditConfigTabSize,
  /* 21 */  DI_CHECKBOX,6,18,0,0,0,0,0,0,(const wchar_t *)MEditConfigAnsiTableAsDefault,
  /* 22 */  DI_CHECKBOX,6,19,0,0,0,0,0,0,(const wchar_t *)MEditConfigAnsiTableForNewFile,
  /* 23 */  DI_TEXT, 0, 20, 0, 0, 0, 0, DIF_SEPARATOR, 0, L"",
  /* 24 */  DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
  /* 25 */  DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel,
  };
  MakeDialogItemsEx(CfgDlgData,CfgDlg);

  {
    const wchar_t *Str = UMSG(MEditConfigEditorF4);

    CfgDlg[ID_EC_EXTERNALUSEALTF4].X1+=wcslen(Str)-(wcschr(Str, L'&')?1:0)+5;

    Str = UMSG(MEditConfigPersistentBlocks);
    CfgDlg[ID_EC_DELREMOVESBLOCKS].X1+=wcslen(Str)-(wcschr(Str, L'&')?1:0)+5+3;

    Str = UMSG(MEditConfigSavePos);

    CfgDlg[ID_EC_SAVEBOOKMARKS].X1+=wcslen(Str)-(wcschr(Str, L'&')?1:0)+5+3;

    if (CfgDlg[ID_EC_DELREMOVESBLOCKS].X1 > CfgDlg[ID_EC_SAVEBOOKMARKS].X1)
      CfgDlg[ID_EC_SAVEBOOKMARKS].X1 = CfgDlg[ID_EC_DELREMOVESBLOCKS].X1;
    else
      CfgDlg[ID_EC_DELREMOVESBLOCKS].X1 = CfgDlg[ID_EC_SAVEBOOKMARKS].X1;
  }

  CfgDlg[ID_EC_EXTERNALUSEF4].Selected=Opt.EdOpt.UseExternalEditor;
  CfgDlg[ID_EC_EXTERNALUSEALTF4].Selected=!Opt.EdOpt.UseExternalEditor;

  CfgDlg[ID_EC_EXTERNALCOMMANDEDIT].strData = Opt.strExternalEditor;

  //���������� ������ ID_EC_EXPANDTABSTITLE ���� ��������� ��������� �����
  //CfgDlg[ID_EC_EXPANDTABS].X1 = CfgDlg[ID_EC_EXPANDTABSTITLE].X1 + strlen(CfgDlg[ID_EC_EXPANDTABSTITLE].Data);

  FarListItem ExpandTabListItems[3];
  memset(ExpandTabListItems,0,sizeof(ExpandTabListItems));
  FarList ExpandTabList = {3,ExpandTabListItems};
  CfgDlg[ID_EC_EXPANDTABS].ListItems = &ExpandTabList;
  wcscpy(ExpandTabListItems[0].Text,UMSG(MEditConfigDoNotExpandTabs));
  wcscpy(ExpandTabListItems[1].Text,UMSG(MEditConfigExpandTabs));
  wcscpy(ExpandTabListItems[2].Text,UMSG(MEditConfigConvertAllTabsToSpaces));

  //������� ������������ ������, ����� ��������� (�� �����������) ������ ���������

  if ( EdOpt.ExpandTabs == EXPAND_NOTABS )
    ExpandTabListItems[0].Flags = LIF_SELECTED;

  if ( EdOpt.ExpandTabs == EXPAND_NEWTABS )
    ExpandTabListItems[1].Flags = LIF_SELECTED;

  if ( EdOpt.ExpandTabs == EXPAND_ALLTABS )
    ExpandTabListItems[2].Flags = LIF_SELECTED;

  CfgDlg[ID_EC_PERSISTENTBLOCKS].Selected = EdOpt.PersistentBlocks;
  CfgDlg[ID_EC_DELREMOVESBLOCKS].Selected = EdOpt.DelRemovesBlocks;
  CfgDlg[ID_EC_AUTOINDENT].Selected = EdOpt.AutoIndent;
  CfgDlg[ID_EC_SAVEPOSITION].Selected = EdOpt.SavePos;
  CfgDlg[ID_EC_SAVEBOOKMARKS].Selected = EdOpt.SaveShortPos;
  CfgDlg[ID_EC_AUTODETECTTABLE].Selected = EdOpt.AutoDetectTable&&DistrTableExist();
  CfgDlg[ID_EC_CURSORBEYONDEOL].Selected = EdOpt.CursorBeyondEOL;
  CfgDlg[ID_EC_LOCKREADONLY].Selected = EdOpt.ReadOnlyLock & 1;
  CfgDlg[ID_EC_READONLYWARNING].Selected = EdOpt.ReadOnlyLock & 2;
  CfgDlg[ID_EC_ANSIASDEFAULT].Selected = EdOpt.AnsiTableAsDefault;
  CfgDlg[ID_EC_ANSIFORNEWFILE].Selected = EdOpt.AnsiTableForNewFile;

  CfgDlg[ID_EC_TABSIZEEDIT].strData.Format (L"%d",EdOpt.TabSize);
  if ( !RegVer )
  {
    CfgDlg[ID_EC_TABSIZEEDIT].Flags |= DIF_DISABLE;
    CfgDlg[ID_EC_TABSIZE].Flags |= DIF_DISABLE;
  }

  int DialogHeight=24;

  if (Local)
  {
    int i;

    for (i = ID_EC_EXTERNALCONFIGTITLE; i <= ID_EC_SEPARATOR1; i++)
      CfgDlg[i].Flags |= DIF_HIDDEN;

    for (i = ID_EC_INTERNALCONFIGTITLE; i <= ID_EC_CANCEL; i++)
      CfgDlg[i].Y1 -= 5;

    CfgDlg[ID_EC_TITLE].Y2 -= 5;
    DialogHeight -= 5;
  }

  {
    Dialog Dlg((DialogItemEx*)CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp(L"EditorSettings");
    Dlg.SetPosition(-1,-1,74,DialogHeight);
    Dlg.Process();
    if (Dlg.GetExitCode()!=ID_EC_OK)
      return;
  }

  if (!Local)
  {
    Opt.EdOpt.UseExternalEditor=CfgDlg[ID_EC_EXTERNALUSEF4].Selected;
    Opt.strExternalEditor = CfgDlg[ID_EC_EXTERNALCOMMANDEDIT].strData;
  }

  switch (CfgDlg[ID_EC_EXPANDTABS].ListPos)
  {
    case 0:
      EdOpt.ExpandTabs = EXPAND_NOTABS;
      break;

    case 1:
      EdOpt.ExpandTabs = EXPAND_NEWTABS;
      break;

    case 2:
      EdOpt.ExpandTabs = EXPAND_ALLTABS;
      break;
  }

  EdOpt.PersistentBlocks = CfgDlg[ID_EC_PERSISTENTBLOCKS].Selected;
  EdOpt.DelRemovesBlocks = CfgDlg[ID_EC_DELREMOVESBLOCKS].Selected;
  EdOpt.AutoIndent = CfgDlg[ID_EC_AUTOINDENT].Selected;
  EdOpt.SavePos = CfgDlg[ID_EC_SAVEPOSITION].Selected;
  EdOpt.SaveShortPos = CfgDlg[ID_EC_SAVEBOOKMARKS].Selected;
  EdOpt.AutoDetectTable = CfgDlg[ID_EC_AUTODETECTTABLE].Selected;
  EdOpt.AnsiTableAsDefault = CfgDlg[ID_EC_ANSIASDEFAULT].Selected;
  EdOpt.AnsiTableForNewFile = CfgDlg[ID_EC_ANSIFORNEWFILE].Selected;

  if(!DistrTableExist() && EdOpt.AutoDetectTable)
  {
    EdOpt.AutoDetectTable=0;
    MessageW(MSG_WARNING,1,UMSG(MWarning),
              UMSG(MDistributionTableWasNotFound),UMSG(MAutoDetectWillNotWork),
              UMSG(MOk));
  }

  EdOpt.TabSize=_wtoi(CfgDlg[ID_EC_TABSIZEEDIT].strData);

  if (EdOpt.TabSize<1 || EdOpt.TabSize>512)
    EdOpt.TabSize=8;

  EdOpt.CursorBeyondEOL=CfgDlg[ID_EC_CURSORBEYONDEOL].Selected;

  EdOpt.ReadOnlyLock&=~3;

  if ( CfgDlg[ID_EC_LOCKREADONLY].Selected )
    EdOpt.ReadOnlyLock|=1;

  if( CfgDlg[ID_EC_READONLYWARNING].Selected )
    EdOpt.ReadOnlyLock|=2;
}


void SetFolderInfoFiles()
{
  string strFolderInfoFiles;
  if (GetStringW(UMSG(MSetFolderInfoTitle),UMSG(MSetFolderInfoNames),L"FolderInfoFiles",
      Opt.strFolderInfoFiles,strFolderInfoFiles,260,L"OptMenu",FIB_ENABLEEMPTY|FIB_BUTTONS))
  {
    Opt.strFolderInfoFiles = strFolderInfoFiles;
    if (CtrlObject->Cp()->LeftPanel->GetType() == INFO_PANEL)
      CtrlObject->Cp()->LeftPanel->Update(0);
    if (CtrlObject->Cp()->RightPanel->GetType() == INFO_PANEL)
      CtrlObject->Cp()->RightPanel->Update(0);
  }
  /* DJ $ */
}


// ���������, ����������� ��� ������������(!)
static struct FARConfig{
  int   IsSave;   // =1 - ����� ������������ � SaveConfig()
  DWORD ValType;  // REG_DWORD, REG_SZ, REG_BINARY
  const wchar_t *KeyName;
  const wchar_t *ValName;
  void *ValPtr;   // ����� ����������, ���� �������� ������
  DWORD DefDWord; // �� �� ������ ������ ��� REG_SZ � REG_BINARY
  const wchar_t *DefStr;   // ������/������ �� ���������
} CFG[]={
  {1, REG_BINARY, NKeyColorsW, L"CurrentPalette",(char*)Palette,SizeArrayPalette,(wchar_t*)DefaultPalette},

  {1, REG_DWORD,  NKeyScreenW, L"Clock", &Opt.Clock, 1, 0},
  {1, REG_DWORD,  NKeyScreenW, L"ViewerEditorClock",&Opt.ViewerEditorClock,0, 0},
  {1, REG_DWORD,  NKeyScreenW, L"KeyBar",&Opt.ShowKeyBar,1, 0},
  {1, REG_DWORD,  NKeyScreenW, L"ScreenSaver",&Opt.ScreenSaver, 0, 0},
  {1, REG_DWORD,  NKeyScreenW, L"ScreenSaverTime",&Opt.ScreenSaverTime,5, 0},
  {1, REG_DWORD,  NKeyScreenW, L"UsePromptFormat", &Opt.UsePromptFormat,0, 0},
  {1, REG_SZ,     NKeyScreenW, L"PromptFormat",&Opt.strPromptFormat, 0, L"$p>"},
  {0, REG_DWORD,  NKeyScreenW, L"DeltaXY", &Opt.ScrSize.DeltaXY, 0, 0},


  {1, REG_DWORD,  NKeyInterfaceW, L"Mouse",&Opt.Mouse,1, 0},
  {1, REG_DWORD,  NKeyInterfaceW, L"AltGr",&Opt.AltGr,1, 0},
  {0, REG_DWORD,  NKeyInterfaceW, L"UseVk_oem_x",&Opt.UseVk_oem_x,1, 0},
  {1, REG_DWORD,  NKeyInterfaceW, L"ShowMenuBar",&Opt.ShowMenuBar,0, 0},
  {0, REG_DWORD,  NKeyInterfaceW, L"CursorSize1",&Opt.CursorSize[0],15, 0},
  {0, REG_DWORD,  NKeyInterfaceW, L"CursorSize2",&Opt.CursorSize[1],10, 0},
  {0, REG_DWORD,  NKeyInterfaceW, L"CursorSize3",&Opt.CursorSize[2],99, 0},
  {0, REG_DWORD,  NKeyInterfaceW, L"CursorSize4",&Opt.CursorSize[3],99, 0},
  {0, REG_DWORD,  NKeyInterfaceW, L"ShiftsKeyRules",&Opt.ShiftsKeyRules,1, 0},
  {0, REG_DWORD,  NKeyInterfaceW, L"AltF9",&Opt.AltF9, -1, 0},
  {1, REG_DWORD,  NKeyInterfaceW, L"CtrlPgUp",&Opt.PgUpChangeDisk, 1, 0},
  {0, REG_DWORD,  NKeyInterfaceW, L"ShowTimeoutDelFiles",&Opt.ShowTimeoutDelFiles, 50, 0},
  {0, REG_DWORD,  NKeyInterfaceW, L"ShowTimeoutDACLFiles",&Opt.ShowTimeoutDACLFiles, 50, 0},

  {1, REG_SZ,     NKeyViewerW,L"ExternalViewerName",&Opt.strExternalViewer, 0, L""},
  {1, REG_DWORD,  NKeyViewerW,L"UseExternalViewer",&Opt.ViOpt.UseExternalViewer,0, 0},
  {1, REG_DWORD,  NKeyViewerW,L"SaveViewerPos",&Opt.ViOpt.SaveViewerPos,1, 0},
  {1, REG_DWORD,  NKeyViewerW,L"SaveViewerShortPos",&Opt.ViOpt.SaveViewerShortPos,1, 0},
  {1, REG_DWORD,  NKeyViewerW,L"AutoDetectTable",&Opt.ViOpt.AutoDetectTable,0, 0},
  {1, REG_DWORD,  NKeyViewerW,L"TabSize",&Opt.ViOpt.TabSize,8, 0},
  {1, REG_DWORD,  NKeyViewerW,L"ShowKeyBar",&Opt.ViOpt.ShowKeyBar,1, 0},
  {1, REG_DWORD,  NKeyViewerW,L"ShowArrows",&Opt.ViOpt.ShowArrows,1, 0},
  {1, REG_DWORD,  NKeyViewerW,L"ShowScrollbar",&Opt.ViOpt.ShowScrollbar,0, 0},
  {1, REG_DWORD,  NKeyViewerW,L"IsWrap",&Opt.ViOpt.ViewerIsWrap,1, 0},
  {1, REG_DWORD,  NKeyViewerW,L"Wrap",&Opt.ViOpt.ViewerWrap,0, 0},
  {1, REG_DWORD,  NKeyViewerW,L"PersistentBlocks",&Opt.ViOpt.PersistentBlocks,0, 0},
  {1, REG_DWORD,  NKeyViewerW,L"AnsiTableAsDefault",&Opt.ViOpt.AnsiTableAsDefault,1, 0},

  {1, REG_DWORD,  NKeyInterfaceW, L"DialogsEditHistory",&Opt.Dialogs.EditHistory,1, 0},
  {1, REG_DWORD,  NKeyInterfaceW, L"DialogsEditBlock",&Opt.Dialogs.EditBlock,0, 0},
  {1, REG_DWORD,  NKeyInterfaceW, L"AutoComplete",&Opt.Dialogs.AutoComplete,1, 0},
  {1, REG_DWORD,  NKeyDialogW,L"EULBsClear",&Opt.Dialogs.EULBsClear,0, 0},
  {1, REG_DWORD,  NKeyDialogW,L"SelectFromHistory",&Opt.Dialogs.SelectFromHistory,0, 0},
  {0, REG_DWORD,  NKeyDialogW,L"EditLine",&Opt.Dialogs.EditLine,0, 0},
  {1, REG_DWORD,  NKeyDialogW,L"MouseButton",&Opt.Dialogs.MouseButton,0xFFFF, 0},
  {0, REG_DWORD,  NKeyDialogW,L"DelRemovesBlocks",&Opt.Dialogs.DelRemovesBlocks,1, 0},
  {0, REG_DWORD,  NKeyDialogW,L"CBoxMaxHeight",&Opt.Dialogs.CBoxMaxHeight,8, 0},

  {1, REG_SZ,     NKeyEditorW,L"ExternalEditorName",&Opt.strExternalEditor, 0, L""},
  {1, REG_DWORD,  NKeyEditorW,L"UseExternalEditor",&Opt.EdOpt.UseExternalEditor,0, 0},
  {1, REG_DWORD,  NKeyEditorW,L"ExpandTabs",&Opt.EdOpt.ExpandTabs,0, 0},
  {1, REG_DWORD,  NKeyEditorW,L"TabSize",&Opt.EdOpt.TabSize,8, 0},
  {1, REG_DWORD,  NKeyEditorW,L"PersistentBlocks",&Opt.EdOpt.PersistentBlocks,0, 0},
  {1, REG_DWORD,  NKeyEditorW,L"DelRemovesBlocks",&Opt.EdOpt.DelRemovesBlocks,0, 0},
  {1, REG_DWORD,  NKeyEditorW,L"AutoIndent",&Opt.EdOpt.AutoIndent,0, 0},
  {1, REG_DWORD,  NKeyEditorW,L"SaveEditorPos",&Opt.EdOpt.SavePos,1, 0},
  {1, REG_DWORD,  NKeyEditorW,L"SaveEditorShortPos",&Opt.EdOpt.SaveShortPos,1, 0},
  {1, REG_DWORD,  NKeyEditorW,L"AutoDetectTable",&Opt.EdOpt.AutoDetectTable,0, 0},
  {1, REG_DWORD,  NKeyEditorW,L"EditorCursorBeyondEOL",&Opt.EdOpt.CursorBeyondEOL,1, 0},
  {1, REG_DWORD,  NKeyEditorW,L"ReadOnlyLock",&Opt.EdOpt.ReadOnlyLock,0, 0}, // ����� ����� ������ 1.65 - �� ������������� � �� �����������
  {0, REG_DWORD,  NKeyEditorW,L"EditorUndoSize",&Opt.EdOpt.UndoSize,2048,0}, // $ 03.12.2001 IS ������ ������ undo � ���������
  {0, REG_SZ,     NKeyEditorW,L"WordDiv",&Opt.strWordDiv, 0, WordDiv0},
  {0, REG_DWORD,  NKeyEditorW,L"BSLikeDel",&Opt.EdOpt.BSLikeDel,1, 0},
  {0, REG_DWORD,  NKeyEditorW,L"EditorF7Rules",&Opt.EdOpt.F7Rules,1, 0},
  {0, REG_DWORD,  NKeyEditorW,L"FileSizeLimit",&Opt.EdOpt.FileSizeLimitLo,(DWORD)0, 0},
  {0, REG_DWORD,  NKeyEditorW,L"FileSizeLimitHi",&Opt.EdOpt.FileSizeLimitHi,(DWORD)0, 0},
  {0, REG_DWORD,  NKeyEditorW,L"CharCodeBase",&Opt.EdOpt.CharCodeBase,1, 0},
  {0, REG_DWORD,  NKeyEditorW,L"AllowEmptySpaceAfterEof", &Opt.EdOpt.AllowEmptySpaceAfterEof,0,0},//skv
  {1, REG_DWORD,  NKeyEditorW,L"AnsiTableForNewFile",&Opt.EdOpt.AnsiTableForNewFile,1, 0},
  {1, REG_DWORD,  NKeyEditorW,L"AnsiTableAsDefault",&Opt.EdOpt.AnsiTableAsDefault,1, 0},
  {1, REG_DWORD,  NKeyEditorW,L"ShowKeyBar",&Opt.EdOpt.ShowKeyBar,1, 0},

  {0, REG_DWORD,  NKeyXLatW,L"Flags",&Opt.XLat.Flags,(DWORD)XLAT_SWITCHKEYBLAYOUT|XLAT_CONVERTALLCMDLINE, 0},
  {0, REG_BINARY, NKeyXLatW,L"Table1",(BYTE*)&Opt.XLat.Table[0][1],sizeof(Opt.XLat.Table[0])-1,NULL},
  {0, REG_BINARY, NKeyXLatW,L"Table2",(BYTE*)&Opt.XLat.Table[1][1],sizeof(Opt.XLat.Table[1])-1,NULL},
  {0, REG_BINARY, NKeyXLatW,L"Rules1",(BYTE*)&Opt.XLat.Rules[0][1],sizeof(Opt.XLat.Rules[0])-1,NULL},
  {0, REG_BINARY, NKeyXLatW,L"Rules2",(BYTE*)&Opt.XLat.Rules[1][1],sizeof(Opt.XLat.Rules[1])-1,NULL},
  {0, REG_BINARY, NKeyXLatW,L"Rules3",(BYTE*)&Opt.XLat.Rules[2][1],sizeof(Opt.XLat.Rules[2])-1,NULL},
  {0, REG_SZ,     NKeyXLatW,L"WordDivForXlat",&Opt.XLat.strWordDivForXlat, 0,WordDivForXlat0},

  {0, REG_DWORD,  NKeySavedHistoryW, NParamHistoryCountW,&Opt.HistoryCount,64, 0},
  {0, REG_DWORD,  NKeySavedFolderHistoryW, NParamHistoryCountW,&Opt.FoldersHistoryCount,64, 0},
  {0, REG_DWORD,  NKeySavedViewHistoryW, NParamHistoryCountW,&Opt.ViewHistoryCount,64, 0},
  {0, REG_DWORD,  NKeySavedDialogHistoryW, NParamHistoryCountW,&Opt.DialogsHistoryCount,64, 0},

  {1, REG_DWORD,  NKeySystemW,L"SaveHistory",&Opt.SaveHistory,1, 0},
  {1, REG_DWORD,  NKeySystemW,L"SaveFoldersHistory",&Opt.SaveFoldersHistory,1, 0},
  {0, REG_DWORD,  NKeySystemW,L"SavePluginFoldersHistory",&Opt.SavePluginFoldersHistory,0, 0},
  {1, REG_DWORD,  NKeySystemW,L"SaveViewHistory",&Opt.SaveViewHistory,1, 0},
  {1, REG_DWORD,  NKeySystemW,L"UseRegisteredTypes",&Opt.UseRegisteredTypes,1, 0},
  {1, REG_DWORD,  NKeySystemW,L"AutoSaveSetup",&Opt.AutoSaveSetup,0, 0},
  {1, REG_DWORD,  NKeySystemW,L"ClearReadOnly",&Opt.ClearReadOnly,0, 0},
  {1, REG_DWORD,  NKeySystemW,L"DeleteToRecycleBin",&Opt.DeleteToRecycleBin,1, 0},
  {0, REG_DWORD,  NKeySystemW,L"WipeSymbol",&Opt.WipeSymbol,0, 0},

  {1, REG_DWORD,  NKeySystemW,L"UseSystemCopy",&Opt.CMOpt.UseSystemCopy,0, 0},
  {0, REG_DWORD,  NKeySystemW,L"CopySecurityOptions",&Opt.CMOpt.CopySecurityOptions,0, 0},
  {1, REG_DWORD,  NKeySystemW,L"CopyOpened",&Opt.CMOpt.CopyOpened,1, 0},
  {1, REG_DWORD,  NKeyInterfaceW, L"CopyShowTotal",&Opt.CMOpt.CopyShowTotal,0, 0},
  {1, REG_DWORD,  NKeySystemW, L"MultiCopy",&Opt.CMOpt.MultiCopy,0, 0},
  {1, REG_DWORD,  NKeySystemW,L"CopyTimeRule",  &Opt.CMOpt.CopyTimeRule, 3, 0},

  {1, REG_DWORD,  NKeySystemW,L"CreateUppercaseFolders",&Opt.CreateUppercaseFolders,0, 0},
  {1, REG_DWORD,  NKeySystemW,L"InactivityExit",&Opt.InactivityExit,0, 0},
  {1, REG_DWORD,  NKeySystemW,L"InactivityExitTime",&Opt.InactivityExitTime,15, 0},
  {1, REG_DWORD,  NKeySystemW,L"DriveMenuMode",&Opt.ChangeDriveMode,DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS|DRIVE_SHOW_SIZE_FLOAT, 0},
  {1, REG_DWORD,  NKeySystemW,L"DriveDisconnetMode",&Opt.ChangeDriveDisconnetMode,1, 0},
  {1, REG_DWORD,  NKeySystemW,L"AutoUpdateRemoteDrive",&Opt.AutoUpdateRemoteDrive,1, 0},
  {1, REG_DWORD,  NKeySystemW,L"FileSearchMode",&Opt.FindOpt.FileSearchMode,SEARCH_FROM_CURRENT, 0},
  {0, REG_DWORD,  NKeySystemW,L"CollectFiles",&Opt.FindOpt.CollectFiles, 1, 0},
  /* $ 11.10.2005 KM */
  {1, REG_DWORD,  NKeySystemW,L"SearchInFirst",&Opt.FindOpt.SearchInFirst,0,0},
  {1, REG_SZ,     NKeySystemW,L"SearchInFirstSize",&Opt.FindOpt.strSearchInFirstSize, 0, L""},
  /* KM $ */
  /* $ 24.10.2001 KM
     ��������� ���� ���������� ������ ��������� � Alt-F7
  */
  {1, REG_DWORD,  NKeySystemW,L"FindFolders",&Opt.FindOpt.FindFolders, 1, 0},
  /* KM $ */
  /* $ 17.09.2003 KM */
  {1, REG_BINARY, NKeySystemW,L"FindCharTable",&Opt.CharTable, sizeof(Opt.CharTable), 0},
  /* KM $ */
  {1, REG_SZ,     NKeySystemW,L"FolderInfo",&Opt.strFolderInfoFiles, 0, L"DirInfo,File_Id.diz,Descript.ion,ReadMe,Read.Me,ReadMe.txt,ReadMe.*"},
  {0, REG_DWORD,  NKeySystemW,L"SubstPluginPrefix",&Opt.SubstPluginPrefix, 0, 0},
  {0, REG_DWORD,  NKeySystemW,L"CmdHistoryRule",&Opt.CmdHistoryRule,0, 0},
  {0, REG_DWORD,  NKeySystemW,L"SetAttrFolderRules",&Opt.SetAttrFolderRules,1, 0},
  {0, REG_DWORD,  NKeySystemW,L"MaxPositionCache",&Opt.MaxPositionCache,64, 0},
  {0, REG_SZ,     NKeySystemW,L"ConsoleDetachKey", &strKeyNameConsoleDetachKey, 0, L"CtrlAltTab"},
  {1, REG_SZ,     NKeySystemW,L"PersonalPluginsPath",&Opt.LoadPlug.strPersonalPluginsPath, 0, strPersonalPluginsPath}, //BUGBUG
  {0, REG_DWORD,  NKeySystemW,L"SilentLoadPlugin",  &Opt.LoadPlug.SilentLoadPlugin, 0, 0},
  {1, REG_DWORD,  NKeySystemW,L"MultiMakeDir",&Opt.MultiMakeDir,0, 0},
  {0, REG_DWORD,  NKeySystemW,L"FlagPosixSemantics", &Opt.FlagPosixSemantics, 1, 0},
  {0, REG_DWORD,  NKeySystemW,L"MsWheelDelta", &Opt.MsWheelDelta, 1, 0},
  {0, REG_DWORD,  NKeySystemW,L"MsWheelDeltaView", &Opt.MsWheelDeltaView, 1, 0},
  {0, REG_DWORD,  NKeySystemW,L"MsWheelDeltaEdit", &Opt.MsWheelDeltaEdit, 1, 0},
  {0, REG_DWORD,  NKeySystemW,L"MsWheelDeltaHelp", &Opt.MsWheelDeltaHelp, 1, 0},
  {0, REG_DWORD,  NKeySystemW,L"SubstNameRule", &Opt.SubstNameRule, 2, 0},
  {0, REG_DWORD,  NKeySystemW,L"ShowCheckingFile", &Opt.ShowCheckingFile, 0, 0},
  {0, REG_DWORD,  NKeySystemW,L"DelThreadPriority", &Opt.DelThreadPriority, THREAD_PRIORITY_NORMAL, 0},
  {0, REG_SZ,     NKeySystemW,L"QuotedSymbols",&Opt.strQuotedSymbols, 0, L" &()[]{}^=;!'+,`"},
  {0, REG_DWORD,  NKeySystemW,L"QuotedName",&Opt.QuotedName,0xFFFFFFFFU, 0},
  /* KM $ */
  //{0, REG_DWORD,  NKeySystemW,L"CPAJHefuayor",&Opt.strCPAJHefuayor,0, 0},
  {0, REG_DWORD,  NKeySystemW,L"CloseConsoleRule",&Opt.CloseConsoleRule,1, 0},
  {0, REG_DWORD,  NKeySystemW,L"PluginMaxReadData",&Opt.PluginMaxReadData,0x20000, 0},
  {1, REG_DWORD,  NKeySystemW,L"CloseCDGate",&Opt.CloseCDGate,-1, 0},
  {0, REG_DWORD,  NKeySystemW,L"UseNumPad",&Opt.UseNumPad,0, 0},
  {0, REG_DWORD,  NKeySystemW,L"CASRule",&Opt.CASRule,0xFFFFFFFFU, 0},
  {0, REG_DWORD,  NKeySystemW,L"AllCtrlAltShiftRule",&Opt.AllCtrlAltShiftRule,0x0000FFFF, 0},
  {1, REG_DWORD,  NKeySystemW,L"ScanJunction",&Opt.ScanJunction,1, 0},
  {0, REG_DWORD,  NKeySystemW,L"IgnoreErrorBadPathName",&Opt.IgnoreErrorBadPathName,0, 0},
  {0, REG_DWORD,  NKeySystemW,L"UsePrintManager",&Opt.UsePrintManager,1, 0},

  {0, REG_DWORD,  NKeySystemNowellW,L"MoveRO",&Opt.Nowell.MoveRO,1, 0},

  {0, REG_DWORD,  NKeySystemExecutorW,L"RestoreCP",&Opt.RestoreCPAfterExecute,1, 0},
  {0, REG_DWORD,  NKeySystemExecutorW,L"UseAppPath",&Opt.ExecuteUseAppPath,1, 0},
  {0, REG_DWORD,  NKeySystemExecutorW,L"ShowErrorMessage",&Opt.ExecuteShowErrorMessage,1, 0},
  {0, REG_SZ,     NKeySystemExecutorW,L"BatchType",&Opt.strExecuteBatchType,0,constBatchExtW},
  {0, REG_DWORD,  NKeySystemExecutorW,L"FullTitle",&Opt.ExecuteFullTitle,0, 0},

  {0, REG_DWORD,  NKeyPanelTreeW,L"MinTreeCount",&Opt.Tree.MinTreeCount, 4, 0},
  {0, REG_DWORD,  NKeyPanelTreeW,L"LocalDisk",&Opt.Tree.LocalDisk, 2, 0},
  {0, REG_DWORD,  NKeyPanelTreeW,L"NetDisk",&Opt.Tree.NetDisk, 2, 0},
  {0, REG_DWORD,  NKeyPanelTreeW,L"RemovableDisk",&Opt.Tree.RemovableDisk, 2, 0},
  {0, REG_DWORD,  NKeyPanelTreeW,L"NetPath",&Opt.Tree.NetPath, 2, 0},
  {1, REG_DWORD,  NKeyPanelTreeW,L"AutoChangeFolder",&Opt.Tree.AutoChangeFolder,0, 0}, // ???

  {0, REG_DWORD,  NKeyHelpW,L"ActivateURL",&Opt.HelpURLRules,1, 0},

  {1, REG_SZ,     NKeyLanguageW,L"Main",&Opt.strLanguage, 0, L"English"},
  {1, REG_SZ,     NKeyLanguageW,L"Help",&Opt.strHelpLanguage, 0, L"English"},

  {1, REG_DWORD,  NKeyConfirmationsW,L"Copy",&Opt.Confirm.Copy,1, 0},
  {1, REG_DWORD,  NKeyConfirmationsW,L"Move",&Opt.Confirm.Move,1, 0},
  {1, REG_DWORD,  NKeyConfirmationsW,L"Drag",&Opt.Confirm.Drag,1, 0},
  {1, REG_DWORD,  NKeyConfirmationsW,L"Delete",&Opt.Confirm.Delete,1, 0},
  {1, REG_DWORD,  NKeyConfirmationsW,L"DeleteFolder",&Opt.Confirm.DeleteFolder,1, 0},
  {1, REG_DWORD,  NKeyConfirmationsW,L"Esc",&Opt.Confirm.Esc,1, 0},
  {1, REG_DWORD,  NKeyConfirmationsW,L"RemoveConnection",&Opt.Confirm.RemoveConnection,1, 0},
  {1, REG_DWORD,  NKeyConfirmationsW,L"RemoveSUBST",&Opt.Confirm.RemoveSUBST,1, 0},
  {1, REG_DWORD,  NKeyConfirmationsW,L"RemoveHotPlug",&Opt.Confirm.RemoveHotPlug,1, 0},
  {1, REG_DWORD,  NKeyConfirmationsW,L"AllowReedit",&Opt.Confirm.AllowReedit,1, 0},
  {1, REG_DWORD,  NKeyConfirmationsW,L"HistoryClear",&Opt.Confirm.HistoryClear,1, 0},
  {1, REG_DWORD,  NKeyConfirmationsW,L"Exit",&Opt.Confirm.Exit,1, 0},

  {1, REG_DWORD,  NKeyPanelW,L"ShowHidden",&Opt.ShowHidden,1, 0},
  {1, REG_DWORD,  NKeyPanelW,L"Highlight",&Opt.Highlight,1, 0},
  {1, REG_DWORD,  NKeyPanelW,L"SortFolderExt",&Opt.SortFolderExt,0, 0},
  {1, REG_DWORD,  NKeyPanelW,L"SelectFolders",&Opt.SelectFolders,0, 0},
  {1, REG_DWORD,  NKeyPanelW,L"ReverseSort",&Opt.ReverseSort,1, 0},
  {0, REG_DWORD,  NKeyPanelW,L"RightClickRule",&Opt.PanelRightClickRule,2, 0},
  {0, REG_DWORD,  NKeyPanelW,L"CtrlFRule",&Opt.PanelCtrlFRule,1, 0},
  {0, REG_DWORD,  NKeyPanelW,L"CtrlAltShiftRule",&Opt.PanelCtrlAltShiftRule,0, 0},
  {0, REG_DWORD,  NKeyPanelW,L"RememberLogicalDrives",&Opt.RememberLogicalDrives, 0, 0},
  {1, REG_DWORD,  NKeyPanelW,L"AutoUpdateLimit",&Opt.AutoUpdateLimit, 0, 0},
  {1, REG_DWORD,  NKeyPanelW,L"MiddleClickRule",&Opt.PanelMiddleClickRule,1, 0}, // $ 17.12.2001 IS ��������� ������� ������ ���� � �������

  {1, REG_DWORD,  NKeyPanelLeftW,L"Type",&Opt.LeftPanel.Type,0, 0},
  {1, REG_DWORD,  NKeyPanelLeftW,L"Visible",&Opt.LeftPanel.Visible,1, 0},
  {1, REG_DWORD,  NKeyPanelLeftW,L"Focus",&Opt.LeftPanel.Focus,1, 0},
  {1, REG_DWORD,  NKeyPanelLeftW,L"ViewMode",&Opt.LeftPanel.ViewMode,2, 0},
  {1, REG_DWORD,  NKeyPanelLeftW,L"SortMode",&Opt.LeftPanel.SortMode,1, 0},
  {1, REG_DWORD,  NKeyPanelLeftW,L"SortOrder",&Opt.LeftPanel.SortOrder,1, 0},
  {1, REG_DWORD,  NKeyPanelLeftW,L"SortGroups",&Opt.LeftPanel.SortGroups,0, 0},
  {1, REG_DWORD,  NKeyPanelLeftW,L"ShortNames",&Opt.LeftPanel.ShowShortNames,0, 0},
  {1, REG_DWORD,  NKeyPanelLeftW,L"NumericSort",&Opt.LeftPanel.NumericSort,0, 0},
  {1, REG_SZ,     NKeyPanelLeftW,L"Folder",&Opt.strLeftFolder, 0, L""},
  {1, REG_SZ,     NKeyPanelLeftW,L"CurFile",&Opt.strLeftCurFile, 0, L""},
  {1, REG_DWORD,  NKeyPanelLeftW,L"SelectedFirst",&Opt.LeftSelectedFirst,0,0},

  {1, REG_DWORD,  NKeyPanelRightW,L"Type",&Opt.RightPanel.Type,0, 0},
  {1, REG_DWORD,  NKeyPanelRightW,L"Visible",&Opt.RightPanel.Visible,1, 0},
  {1, REG_DWORD,  NKeyPanelRightW,L"Focus",&Opt.RightPanel.Focus,0, 0},
  {1, REG_DWORD,  NKeyPanelRightW,L"ViewMode",&Opt.RightPanel.ViewMode,2, 0},
  {1, REG_DWORD,  NKeyPanelRightW,L"SortMode",&Opt.RightPanel.SortMode,1, 0},
  {1, REG_DWORD,  NKeyPanelRightW,L"SortOrder",&Opt.RightPanel.SortOrder,1, 0},
  {1, REG_DWORD,  NKeyPanelRightW,L"SortGroups",&Opt.RightPanel.SortGroups,0, 0},
  {1, REG_DWORD,  NKeyPanelRightW,L"ShortNames",&Opt.RightPanel.ShowShortNames,0, 0},
  {1, REG_DWORD,  NKeyPanelRightW,L"NumericSort",&Opt.RightPanel.NumericSort,0, 0},
  {1, REG_SZ,     NKeyPanelRightW,L"Folder",&Opt.strRightFolder, 0,L""},
  {1, REG_SZ,     NKeyPanelRightW,L"CurFile",&Opt.strRightCurFile, 0,L""},
  {1, REG_DWORD,  NKeyPanelRightW,L"SelectedFirst",&Opt.RightSelectedFirst,0, 0},

  {1, REG_DWORD,  NKeyPanelLayoutW,L"ColumnTitles",&Opt.ShowColumnTitles,1, 0},
  {1, REG_DWORD,  NKeyPanelLayoutW,L"StatusLine",&Opt.ShowPanelStatus,1, 0},
  {1, REG_DWORD,  NKeyPanelLayoutW,L"TotalInfo",&Opt.ShowPanelTotals,1, 0},
  {1, REG_DWORD,  NKeyPanelLayoutW,L"FreeInfo",&Opt.ShowPanelFree,0, 0},
  {1, REG_DWORD,  NKeyPanelLayoutW,L"Scrollbar",&Opt.ShowPanelScrollbar,0, 0},
  {0, REG_DWORD,  NKeyPanelLayoutW,L"ScrollbarMenu",&Opt.ShowMenuScrollbar,1, 0},
  {1, REG_DWORD,  NKeyPanelLayoutW,L"ScreensNumber",&Opt.ShowScreensNumber,1, 0},
  {1, REG_DWORD,  NKeyPanelLayoutW,L"SortMode",&Opt.ShowSortMode,1, 0},

  {1, REG_DWORD,  NKeyLayoutW,L"HeightDecrement",&Opt.HeightDecrement,0, 0},
  {1, REG_DWORD,  NKeyLayoutW,L"WidthDecrement",&Opt.WidthDecrement,0, 0},
  {1, REG_SZ,     NKeyLayoutW,L"PassiveFolder",&Opt.strPassiveFolder, 0, L""},
  {1, REG_DWORD,  NKeyLayoutW,L"FullscreenHelp",&Opt.FullScreenHelp,0, 0},

  {1, REG_SZ,     NKeyDescriptionsW,L"ListNames",&Opt.Diz.strListNames, 0, L"Descript.ion,Files.bbs"},
  {1, REG_DWORD,  NKeyDescriptionsW,L"UpdateMode",&Opt.Diz.UpdateMode,DIZ_UPDATE_IF_DISPLAYED, 0},
  {1, REG_DWORD,  NKeyDescriptionsW,L"ROUpdate",&Opt.Diz.ROUpdate,0, 0},
  {1, REG_DWORD,  NKeyDescriptionsW,L"SetHidden",&Opt.Diz.SetHidden,TRUE, 0},
  {1, REG_DWORD,  NKeyDescriptionsW,L"StartPos",&Opt.Diz.StartPos,0, 0},

  {0, REG_DWORD,  NKeyKeyMacrosW,L"MacroReuseRules",&Opt.MacroReuseRules,0, 0},

  {0, REG_DWORD,  NKeyPoliciesW,L"ShowHiddenDrives",&Opt.Policies.ShowHiddenDrives,1, 0},
  {0, REG_DWORD,  NKeyPoliciesW,L"DisabledOptions",&Opt.Policies.DisabledOptions,0, 0},

  {0, REG_SZ,     NKeyKeyMacrosW,L"DateFormat",&Opt.strDateFormat, 0, L"%a %b %d %H:%M:%S %Z %Y"},

  /* $ 05.10.2003 KM
     ���������� ���������� ������� ��������
  */
  {1, REG_DWORD,  NKeyFileFilterW,L"UseMask",&Opt.OpFilter.FMask.Used,0,0},
  {1, REG_SZ,     NKeyFileFilterW,L"Mask",&Opt.OpFilter.FMask.strMask, 0, L"*.*"},

  {1, REG_DWORD,  NKeyFileFilterW,L"UseDate",&Opt.OpFilter.FDate.Used,0,0},
  {1, REG_DWORD,  NKeyFileFilterW,L"DateType",&Opt.OpFilter.FDate.DateType,0,0},
  {1, REG_BINARY, NKeyFileFilterW,L"DateAfter",&Opt.OpFilter.FDate.DateAfter,sizeof(Opt.OpFilter.FDate.DateAfter),0},
  {1, REG_BINARY, NKeyFileFilterW,L"DateBefore",&Opt.OpFilter.FDate.DateBefore,sizeof(Opt.OpFilter.FDate.DateBefore),0},

  {1, REG_DWORD,  NKeyFileFilterW,L"UseSize",&Opt.OpFilter.FSize.Used,0,0},
  {1, REG_DWORD,  NKeyFileFilterW,L"SizeType",&Opt.OpFilter.FSize.SizeType,0,0},
  {1, REG_BINARY, NKeyFileFilterW,L"SizeAbove",&Opt.OpFilter.FSize.SizeAbove,sizeof(Opt.OpFilter.FSize.SizeAbove),0},
  {1, REG_BINARY, NKeyFileFilterW,L"SizeBelow",&Opt.OpFilter.FSize.SizeBelow,sizeof(Opt.OpFilter.FSize.SizeBelow),0},

  {1, REG_DWORD,  NKeyFileFilterW,L"UseAttr",&Opt.OpFilter.FAttr.Used,0,0},
  {1, REG_DWORD,  NKeyFileFilterW,L"AttrSet",&Opt.OpFilter.FAttr.AttrSet,0,0},
  {1, REG_DWORD,  NKeyFileFilterW,L"AttrClear",&Opt.OpFilter.FAttr.AttrClear,0,0},
  /* KM $ */
  {0, REG_DWORD,  NKeySystemW,L"ExcludeCmdHistory",&Opt.ExcludeCmdHistory,0, 0}, //AN
};


void ReadConfig()
{
  int I, J;
  DWORD OptPolicies_ShowHiddenDrives,  OptPolicies_DisabledOptions;
  string strKeyNameFromReg;

  /* <�����������> *************************************************** */
  // "��������" ���� ��� ��������������� ������ ��������
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  GetRegKeyW(NKeySystemW,L"TemplatePluginsPath",strPersonalPluginsPath,L"");
  OptPolicies_ShowHiddenDrives=GetRegKeyW(NKeyPoliciesW,L"ShowHiddenDrives",1)&1;
  OptPolicies_DisabledOptions=GetRegKeyW(NKeyPoliciesW,L"DisabledOptions",0);
  SetRegRootKey(HKEY_CURRENT_USER);
  if(Opt.ExceptRules == -1)
    GetRegKeyW(L"System",L"ExceptRules",Opt.ExceptRules,1);

  //Opt.LCIDSort=LOCALE_USER_DEFAULT; // ����������������� �� ������ ������
  /* *************************************************** </�����������> */

  for(I=0; I < sizeof(CFG)/sizeof(CFG[0]); ++I)
  {
    switch(CFG[I].ValType)
    {
      case REG_DWORD:
       GetRegKeyW(CFG[I].KeyName, CFG[I].ValName,*(int *)CFG[I].ValPtr,(DWORD)CFG[I].DefDWord);
       break;
      case REG_SZ:
       GetRegKeyW(CFG[I].KeyName, CFG[I].ValName,*(string *)CFG[I].ValPtr,CFG[I].DefStr);
       break;
      case REG_BINARY:
       int Size=GetRegKeyW(CFG[I].KeyName, CFG[I].ValName,(BYTE*)CFG[I].ValPtr,(BYTE*)CFG[I].DefStr,CFG[I].DefDWord);
       if(Size && Size < (int)CFG[I].DefDWord)
         memset(((BYTE*)CFG[I].ValPtr)+Size,0,CFG[I].DefDWord-Size);
       break;
    }
  }

  /* <������������> *************************************************** */
  /* $ 02.04.2001 VVM
    + Opt.FlagPosixSemantics �� ����� ��� 9x */
  if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
    Opt.FlagPosixSemantics=0;
  /* VVM $ */

  GetRegKeyW(NKeyConfirmationsW,L"EscTwiceToInterrupt",Opt.Confirm.EscTwiceToInterrupt,0);

  if(Opt.PluginMaxReadData < 0x1000 || Opt.PluginMaxReadData > 0x80000)
    Opt.PluginMaxReadData=0x20000;

  // ��������� ������ ��� ������ ��������.
  if(Opt.AltF9 == -1)
    Opt.AltF9=WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT?1:0;

  if(Opt.CloseCDGate == -1)
    Opt.CloseCDGate=WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT?1:0;

  Opt.HelpTabSize=8; // ���� ������ ��������...

  //   �������� �������� "������" �������.
  for(I=COL_PRIVATEPOSITION_FOR_DIF165ABOVE-COL_FIRSTPALETTECOLOR+1;
      I < (COL_LASTPALETTECOLOR-COL_FIRSTPALETTECOLOR);
      ++I)
  {
    if(!Palette[I])
      if(!Palette[COL_PRIVATEPOSITION_FOR_DIF165ABOVE-COL_FIRSTPALETTECOLOR])
        Palette[I]=DefaultPalette[I];
      else if(Palette[COL_PRIVATEPOSITION_FOR_DIF165ABOVE-COL_FIRSTPALETTECOLOR] == 1)
        Palette[I]=BlackPalette[I];
      /*
      else
        � ������ ������� ������ ������ �� ������, �.�.
        ���� ������ �������...
      */
  }
  /* SVS 13.12.2000 $ */

  Opt.ViOpt.ViewerIsWrap&=1;
  if(RegVer) Opt.ViOpt.ViewerWrap&=1; else Opt.ViOpt.ViewerWrap=0;

  if (!Opt.ViOpt.AnsiTableAsDefault)
  {
    ViewerInitUseDecodeTable=FALSE;
    ViewerInitTableNum=0;
    ViewerInitAnsiText=FALSE;
  }

  /* $ 03.12.2001 IS
      ���� EditorUndoSize ������� ��������� ��� ������� �������,
      �� ������� ������ undo ����� ��, ��� � � ������ �������
  */
  if(Opt.EdOpt.UndoSize<64 || Opt.EdOpt.UndoSize>(0x7FFFFFFF-2))
    Opt.EdOpt.UndoSize=64;
  /* IS $ */

  if (!Opt.EdOpt.AnsiTableAsDefault)
  {
    EditorInitUseDecodeTable=FALSE;
    EditorInitTableNum=0;
    EditorInitAnsiText=FALSE;
  }

  // ��������� ��������� �������� ������������ ;-)
  if ( Opt.strWordDiv.IsEmpty() )
     Opt.strWordDiv = WordDiv0;
  // ��������� ��������� �������� ������������
  if( Opt.XLat.strWordDivForXlat.IsEmpty() )
     Opt.XLat.strWordDivForXlat = WordDivForXlat0;
  if(Opt.MaxPositionCache < 16 || Opt.MaxPositionCache > 128)
    Opt.MaxPositionCache=64;
  Opt.PanelRightClickRule%=3;
  Opt.PanelCtrlAltShiftRule%=3;
  Opt.ConsoleDetachKey=KeyNameToKey(strKeyNameConsoleDetachKey);
  if (Opt.EdOpt.TabSize<1 || Opt.EdOpt.TabSize>512)
    Opt.EdOpt.TabSize=8;
  if (Opt.ViOpt.TabSize<1 || Opt.ViOpt.TabSize>512)
    Opt.ViOpt.TabSize=8;

  GetRegKeyW(NKeyKeyMacrosW,L"KeyRecordCtrlDot",strKeyNameFromReg,szCtrlDot);
  if((Opt.KeyMacroCtrlDot=KeyNameToKey(strKeyNameFromReg)) == -1)
    Opt.KeyMacroCtrlDot=KEY_CTRLDOT;

  GetRegKeyW(NKeyKeyMacrosW,L"KeyRecordCtrlShiftDot",strKeyNameFromReg,szCtrlShiftDot);
  if((Opt.KeyMacroCtrlShiftDot=KeyNameToKey(strKeyNameFromReg)) == -1)
    Opt.KeyMacroCtrlShiftDot=KEY_CTRLSHIFTDOT;

  GetRegKeyW(NKeyXLatW,L"EditorKey",strKeyNameFromReg,szCtrlShiftX);
  if((Opt.XLat.XLatEditorKey=KeyNameToKey(strKeyNameFromReg)) == -1)
    Opt.XLat.XLatEditorKey=0;
  GetRegKeyW(NKeyXLatW,L"CmdLineKey",strKeyNameFromReg,szCtrlShiftX);
  if((Opt.XLat.XLatCmdLineKey=KeyNameToKey(strKeyNameFromReg)) == -1)
    Opt.XLat.XLatCmdLineKey=0;
  GetRegKeyW(NKeyXLatW,L"DialogKey",strKeyNameFromReg,szCtrlShiftX);
  if((Opt.XLat.XLatDialogKey=KeyNameToKey(strKeyNameFromReg)) == -1)
    Opt.XLat.XLatDialogKey=0;
  GetRegKeyW(NKeyXLatW,L"FastFindKey",strKeyNameFromReg,szCtrlShiftX);
  if((Opt.XLat.XLatFastFindKey=KeyNameToKey(strKeyNameFromReg)) == -1)
    Opt.XLat.XLatFastFindKey=0;

  GetRegKeyW(NKeyXLatW,L"AltEditorKey",strKeyNameFromReg,szCtrlShiftX);
  if((Opt.XLat.XLatAltEditorKey=KeyNameToKey(strKeyNameFromReg)) == -1)
    Opt.XLat.XLatAltEditorKey=0;
  GetRegKeyW(NKeyXLatW,L"AltCmdLineKey",strKeyNameFromReg,szCtrlShiftX);
  if((Opt.XLat.XLatAltCmdLineKey=KeyNameToKey(strKeyNameFromReg)) == -1)
    Opt.XLat.XLatAltCmdLineKey=0;
  GetRegKeyW(NKeyXLatW,L"AltDialogKey",strKeyNameFromReg,szCtrlShiftX);
  if((Opt.XLat.XLatAltDialogKey=KeyNameToKey(strKeyNameFromReg)) == -1)
    Opt.XLat.XLatAltDialogKey=0;
  GetRegKeyW(NKeyXLatW,L"AltFastFindKey",strKeyNameFromReg,szCtrlShiftX);
  if((Opt.XLat.XLatAltFastFindKey=KeyNameToKey(strKeyNameFromReg)) == -1)
    Opt.XLat.XLatAltFastFindKey=0;


  for(I=0; I < 2; ++I)
  {
    for(J=1; J < sizeof(Opt.XLat.Table[0]); ++J)
    {
      if(!Opt.XLat.Table[I][J])
      {
        if(J > 0) --J;
        Opt.XLat.Table[I][0]=(BYTE)J;
        break;
      }
    }
  }
  for(I=0; I < 3; ++I)
  {
    for(J=1; J < sizeof(Opt.XLat.Rules[0]); ++J)
    {
      if(!Opt.XLat.Rules[I][J])
      {
        if(J > 0) --J;
        Opt.XLat.Rules[I][0]=(BYTE)J;
        break;
      }
    }
  }

  Opt.EdOpt.strWordDiv = Opt.strWordDiv;
  FileList::ReadPanelModes();

  apiGetTempPath (Opt.strTempPath);
  RemoveTrailingSpacesW(Opt.strTempPath);
  AddEndSlashW(Opt.strTempPath);
  CtrlObject->EditorPosCache->Read(L"Editor\\LastPositions");
  CtrlObject->ViewerPosCache->Read(L"Viewer\\LastPositions");

  // �������� ��������� ��������
  // ��� ������ HKCU ����� ������ �������� �����
  Opt.Policies.ShowHiddenDrives&=OptPolicies_ShowHiddenDrives;
  // ��� ����� HKCU ����� ������ ��������� ��������� �������
  Opt.Policies.DisabledOptions|=OptPolicies_DisabledOptions;

  if(Opt.strExecuteBatchType.IsEmpty()) // ��������������
    Opt.strExecuteBatchType=constBatchExtW;
  ReplaceStringsW(Opt.strExecuteBatchType,L";",L"",-1);
  Opt.strExecuteBatchType+=L""; //???
  /* *************************************************** </������������> */
}


void SaveConfig(int Ask)
{
  if(Opt.Policies.DisabledOptions&0x20000) // Bit 17 - ��������� ���������
    return;

  if (Ask && MessageW(0,2,UMSG(MSaveSetupTitle),UMSG(MSaveSetupAsk1),UMSG(MSaveSetupAsk2),UMSG(MSaveSetup),UMSG(MCancel))!=0)
    return;

  string strTemp;
  int I;

  /* <�����������> *************************************************** */
  Panel *LeftPanel=CtrlObject->Cp()->LeftPanel;
  Panel *RightPanel=CtrlObject->Cp()->RightPanel;

  Opt.LeftPanel.Focus=LeftPanel->GetFocus();
  Opt.LeftPanel.Visible=LeftPanel->IsVisible();
  Opt.RightPanel.Focus=RightPanel->GetFocus();
  Opt.RightPanel.Visible=RightPanel->IsVisible();

  CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel)->GetCurDirW(Opt.strPassiveFolder);

  if (LeftPanel->GetMode()==NORMAL_PANEL)
  {
    Opt.LeftPanel.Type=LeftPanel->GetType();
    Opt.LeftPanel.ViewMode=LeftPanel->GetViewMode();
    Opt.LeftPanel.SortMode=LeftPanel->GetSortMode();
    Opt.LeftPanel.SortOrder=LeftPanel->GetSortOrder();
    Opt.LeftPanel.SortGroups=LeftPanel->GetSortGroups();
    Opt.LeftPanel.ShowShortNames=LeftPanel->GetShowShortNamesMode();
    Opt.LeftPanel.NumericSort=LeftPanel->GetNumericSort();
    Opt.LeftSelectedFirst=LeftPanel->GetSelectedFirstMode();
  }
  LeftPanel->GetCurDirW(Opt.strLeftFolder);
  LeftPanel->GetCurBaseNameW(Opt.strLeftCurFile, strTemp);

  if (RightPanel->GetMode()==NORMAL_PANEL)
  {
    Opt.RightPanel.Type=RightPanel->GetType();
    Opt.RightPanel.ViewMode=RightPanel->GetViewMode();
    Opt.RightPanel.SortMode=RightPanel->GetSortMode();
    Opt.RightPanel.SortOrder=RightPanel->GetSortOrder();
    Opt.RightPanel.SortGroups=RightPanel->GetSortGroups();
    Opt.RightPanel.ShowShortNames=RightPanel->GetShowShortNamesMode();
    Opt.RightPanel.NumericSort=RightPanel->GetNumericSort();
    Opt.RightSelectedFirst=RightPanel->GetSelectedFirstMode();
  }
  RightPanel->GetCurDirW(Opt.strRightFolder);
  RightPanel->GetCurBaseNameW(Opt.strRightCurFile,strTemp);
  CtrlObject->HiFiles->SaveHiData();
  /* *************************************************** </�����������> */

  for(I=0; I < sizeof(CFG)/sizeof(CFG[0]); ++I)
  {
    if(CFG[I].IsSave)
      switch(CFG[I].ValType)
      {
        case REG_DWORD:
         SetRegKeyW(CFG[I].KeyName, CFG[I].ValName,*(int *)CFG[I].ValPtr);
         break;
        case REG_SZ:
         SetRegKeyW(CFG[I].KeyName, CFG[I].ValName,*(string *)CFG[I].ValPtr);
         break;
        case REG_BINARY:
         SetRegKeyW(CFG[I].KeyName, CFG[I].ValName,(BYTE*)CFG[I].ValPtr,CFG[I].DefDWord);
         break;
      }
  }
  /* <������������> *************************************************** */
  PanelFilter::SaveSelection();
  FileList::SavePanelModes();
  if (Ask)
    CtrlObject->Macro.SaveMacros();
  /* *************************************************** </������������> */
}
