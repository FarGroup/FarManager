#ifndef __FARSTRUCT_HPP__
#define __FARSTRUCT_HPP__
/*
struct.hpp

��� ����������� ��������� (������� �������� ������ ������� ����)

*/

/* Revision: 1.152 07.07.2006 $ */

#include "farconst.hpp"
#include "UnicodeString.hpp"

struct FilterParams
{
  struct
  {
    DWORD Used;
    string strMask;
  } FMask;
  struct
  {
    DWORD Used;
    FDateType DateType;
    FILETIME DateAfter;
    FILETIME DateBefore;
  } FDate;
  struct
  {
    DWORD Used;
    FSizeType SizeType;
    __int64 SizeAbove;
    __int64 SizeBelow;
  } FSize;
  struct
  {
    DWORD Used;
    DWORD AttrSet;
    DWORD AttrClear;
  } FAttr;
};

struct PanelOptions
{
  int Type;
  int Visible;
  int Focus;
  int ViewMode;
  int SortMode;
  int SortOrder;
  int SortGroups;
  int ShowShortNames;
  int NumericSort;
};

struct Confirmation
{
  int Copy;
  int Move;
  int Drag;
  int Delete;
  int DeleteFolder;
  int Exit;
  /* $ 09.02.2001 IS
     ��� CheckForEsc
  */
  int Esc;
  /* IS $ */
  /* $ 12.03.2002 VVM
    + Opt.EscTwiceToInterrupt
      ���������� ��������� ��� ���������� ���������� ��������
      0 - ������ ESC ���������� ��������
      1 - ������ ESC ��������� �������� */
  int EscTwiceToInterrupt;
  /* VVM $ */
  int RemoveConnection;
  /* $ 23.05.2001
    +  Opt.Confirmation.AllowReedit - ����, ������� �������� ��������� ��������
      ����� �� �������������� ����, ������ ���� ��� �������������. �� ��������� - 1
      0 - ���� ��� �������� ���� �� ��� �������, �� ���������� ������� � ��������� ���������
          ��� �������������� ��������. ���� ���� ��� �������, �� �������� ������, � � ������
          ���� ������� ������� Reload, �� ����������� ����� ����� �����, ��� ���� ���������
          ��������� ��������.
      1 - ��� ��� ���� ������. �������� ������ � ���������� ������� ���� ��� � ��������� �����
          ���� ����������� ����� ������ ���������.
      */
  int AllowReedit;
  int HistoryClear;
  int RemoveSUBST;
  int RemoveHotPlug;
};


struct RegInfo
{
  char RegName[256];
  char RegCode[256];
  int Done;
};


struct DizOptions
{
  string strListNames;
  int ROUpdate;
  int UpdateMode;
  int SetHidden;
  int StartPos;
};

/* $ 05.09.2000 SVS
   ��������� CodeQWERTY, ����������� QWERTY-��������������
*/
struct CodeXLAT{
  DWORD Flags;       // �������������� �����
  /* $ 05.09.2000 SVS
     � Opt ��������� �������, ���������� ������� Xlat
  */
  int XLatEditorKey;
  int XLatCmdLineKey;
  int XLatDialogKey;
  int XLatFastFindKey;
  /* SVS $*/
  /* $ 04.11.2000 SVS
     � Opt ��������� �������������� �������, ���������� ������� Xlat
  */
  int XLatAltEditorKey;
  int XLatAltCmdLineKey;
  int XLatAltDialogKey;
  int XLatAltFastFindKey;
  /* SVS $*/
  /* $ 25.11.2000 IS
     �������������� ���� �� ������� ��� ������� Xlat
  */
  string strWordDivForXlat;
  /* IS $ */
  // ������ ���� - ������ �������
  BYTE Table[2][81]; // [0] non-english �����, [1] english �����
  BYTE Rules[3][81]; // 3 �� 40 ������:
                    //  [0] "���� ���������� ������ ���������"
                    //  [1] "���� ���������� ������ ����������� ������"
                    //  [2] "���� ���������� ������ �� ���/lat"
};
/* SVS $*/

/* $ 21.02.2001 IS
     ����� ���������: ��������� ���������
*/
struct EditorOptions
{
  int TabSize;
  int ExpandTabs;
  int PersistentBlocks;
  int DelRemovesBlocks;
  int AutoIndent;
  int AutoDetectTable;
  int AnsiTableForNewFile;
  int AnsiTableAsDefault;
  int CursorBeyondEOL;
  int BSLikeDel;
  int CharCodeBase;
  int SavePos;
  int SaveShortPos;
  int F7Rules; // $ 28.11.2000 SVS - ������� �� ���� ������ � ���������
  int AllowEmptySpaceAfterEof; // $ 21.06.2005 SKV - ��������� ���������� ������ ������������ ����� ��������� ������ �������������� �����.
  string strWordDiv;

  int ReadOnlyLock; // $ 29.11.2000 SVS - ������ ���� ��� �������� � ���������, ���� �� ����� �������� R|S|H
  int UndoSize; // $ 03.12.2001 IS - ������ ������ undo � ���������
  int UseExternalEditor;
  /* $ 29.11.2000 SVS
   + Opt.EditorFileSizeLimit - ���������� ���������� ������ �����, �����
     �������� ����� ����� ������ � ���������������� �������� ���������
     ����� �� ��������������
  */
  DWORD FileSizeLimitLo;
  DWORD FileSizeLimitHi;
  /* SVS $ */
  int ShowKeyBar;
};
/* IS $ */

/* $ 29.03.2001 IS
     ��� ������� ������� "���������" ��������� ��� ��������� ���������
*/
struct ViewerOptions
{
  int TabSize;
  int AutoDetectTable;
  int ShowScrollbar;     // $ 18.07.2000 tran ���� �������� ��� viewer
  int ShowArrows;
  int PersistentBlocks; // $ 14.05.2002 VVM ���������� ����� �� ������
  int ViewerIsWrap; // (Wrap|WordWarp)=1 | UnWrap=0
  int ViewerWrap; // Wrap=0|WordWarp=1
  int SaveViewerPos;
  int SaveViewerShortPos;
  int UseExternalViewer;
  int ShowKeyBar; // $ 15.07.2000 tran + ShowKeyBar
  int AnsiTableAsDefault;
};
/* IS $ */

// "�������"
struct PoliciesOptions {
  int DisabledOptions;  // ������������� ���� ������������
  int ShowHiddenDrives; // ���������� ������� ���������� �����
};

struct DialogsOptions{
  int   EditBlock;          // ���������� ����� � ������� �����
  int   EditHistory;        // ��������� � �������?
  int   AutoComplete;       // ��������� ��������������?
  int   EULBsClear;         // = 1 - BS � �������� ��� UnChanged ������ ������� ����� ������ �����, ��� � Del
  int   SelectFromHistory;  // = 0 then (ctrl-down � ������ � �������� ������ �������������� �� ����� ������� ������)
  DWORD EditLine;           // ����� ���������� � ������ ����� (������ ��� ����... ��������� ��������� ����������)
  int   MouseButton;        // ���������� ���������� ������/����� ������ ���� ��� ������ �������� ���� �������
  int   DelRemovesBlocks;
  int   CBoxMaxHeight;      // ������������ ������ ������������ ������ (�� ���������=8)
};

struct NowellOptions{
  int MoveRO;               // ����� ��������� Move ������� R/S/H ��������, ����� �������� - ���������� �������
};

// ��������� ���������� ������ character table
struct FindCharTable
{
  int AllTables;
  int AnsiTable;
  int UnicodeTable;
  int TableNum;
};

struct ScreenSizes{
  COORD DeltaXY;            // �� ������� ���. �������� ������� ��� ������������ ������
#if defined(DETECT_ALT_ENTER)
  /*
    Opt.WScreenSize - Windowed/Full Screen Size
       COORD[0].X - Windowed Width  mode 1
       COORD[0].Y - Windowed Height mode 1
       COORD[1].X - Windowed Width  mode 2
       COORD[1].Y - Windowed Height mode 2

       COORD[2].X - FullScreen Width  mode 1
       COORD[2].Y - FullScreen Height mode 1
       COORD[3].X - FullScreen Width  mode 2
       COORD[3].Y - FullScreen Height mode 2
  */
  int WScreenSizeSet;
  COORD WScreenSize[4];
#endif
};

struct LoadPluginsOptions{
//  DWORD TypeLoadPlugins;       // see TYPELOADPLUGINSOPTIONS
  /* $ 03.08.2000 SVS
     TRUE - ������������ ����������� ���� � �������� ��������
  */
  int MainPluginDir;
  /* SVS $*/
  /* $ 01.09.2000 tran
     seting by '/co' switch, not saved in registry. */
  int PluginsCacheOnly;
  /* tran $ */
  int PluginsPersonal;

  string strCustomPluginsPath;  // ���� ��� ������ ��������, ��������� � /p
  string strPersonalPluginsPath;
  /* SVS $*/
  int SilentLoadPlugin; // ��� �������� ������� � ������...
};

struct FindFileOptions{
  int FindFolders;
  int CollectFiles;
  int FileSearchMode;
  int SearchInFirst;
  string strSearchInFirstSize;
};

struct TreeOptions{
  int LocalDisk;         // ������� ���� ��������� ����� ��� ��������� ������
  int NetDisk;           // ������� ���� ��������� ����� ��� ������� ������
  int NetPath;           // ������� ���� ��������� ����� ��� ������� �����
  int RemovableDisk;     // ������� ���� ��������� ����� ��� ������� ������
  int MinTreeCount;      // ����������� ���������� ����� ��� ���������� ������ � �����.
  int AutoChangeFolder;  // ��������� ����� ��� ����������� �� ������
};

struct CopyMoveOptions{
  int UseSystemCopy;         // ������������ ��������� ������� �����������
  int CopyOpened;            // ���������� �������� �� ������ �����
  int CopyShowTotal;         // �������� ����� ��������� �����������
  int MultiCopy;             // "��������� �����������������/�����������/�������� ������"
  DWORD CopySecurityOptions; // ��� �������� Move - ��� ������ � ������ "Copy access rights"
  int CopyTimeRule;          // $ 30.01.2001 VVM  ���������� ����� �����������,���������� ����� � ������� ��������
};

struct Options
{
  int Clock;
  int Mouse;
  int ShowKeyBar;
  int ScreenSaver;
  int ScreenSaverTime;
  int UsePromptFormat;
  string strPromptFormat;
  int AltGr;
  int UseVk_oem_x;
  int InactivityExit;
  int InactivityExitTime;
  int ShowHidden;
  int Highlight;

  string strLeftFolder;
  string strRightFolder;

  string strLeftCurFile;
  string strRightCurFile;

  int RightSelectedFirst;
  int LeftSelectedFirst;
  int SelectFolders;
  int ReverseSort;
  int ClearReadOnly;
  int SortFolderExt;
  int DeleteToRecycleBin;
  int WipeSymbol; // ������ ����������� ��� "ZAP-��������"

  struct CopyMoveOptions CMOpt;

  /* IS $ */
  /* $ 07.12.2001 IS ����� �������� ���������� ��������� �� ���� ����� */
  int MultiMakeDir;
  /* IS $ */
  int CreateUppercaseFolders;
  int UseRegisteredTypes;

  int ViewerEditorClock;
  int OnlyEditorViewerUsed; // =1, ���� ����� ��� /e ��� /v
  int SaveViewHistory;
  int ViewHistoryCount;

  string strExternalEditor;
  struct EditorOptions EdOpt;
  string strExternalViewer;
  struct ViewerOptions ViOpt;


  string strWordDiv; // $ 03.08.2000 SVS �������������� ���� �� �������
  string strQuotedSymbols;
  DWORD QuotedName;
  int AutoSaveSetup;
  int SetupArgv; // ���������� ��������� � ���������� ����
  int ChangeDriveMode;
  int ChangeDriveDisconnetMode;

  int SaveHistory;
  int HistoryCount;
  int SaveFoldersHistory;
  int SavePluginFoldersHistory;
  int FoldersHistoryCount;
  int DialogsHistoryCount;

  struct FindFileOptions FindOpt;

  string strTempPath;
  int HeightDecrement;
  int WidthDecrement;

  string strPassiveFolder;

  int ShowColumnTitles;
  int ShowPanelStatus;
  int ShowPanelTotals;
  int ShowPanelFree;
  int ShowPanelScrollbar;
  int ShowMenuScrollbar; // $ 29.06.2000 SVS �������� ������� ������ Scroll Bar � ����.
  int ShowScreensNumber;
  int ShowSortMode;
  int ShowMenuBar;

  int CleanAscii;
  int NoGraphics;
  string strFolderInfoFiles;

  struct Confirmation Confirm;
  struct DizOptions Diz;
  struct PanelOptions LeftPanel;
  struct PanelOptions RightPanel;

  DWORD  AutoUpdateLimit; // ���� ����� ���������� ������������� �� ��������� ������.
  int AutoUpdateRemoteDrive;

  string strLanguage;
  int SmallIcon;
  string strRegRoot;
  /* $ 12.09.2000 SVS
   + Opt.PanelRightClickRule ������ ��������� ������ ������� ����
     (��� �� ������ Bug#17)
  */
  int PanelRightClickRule;
  /* SVS $*/
  /* $ 17.12.2001 IS ��������� ������� ������ ���� � ������� */
  int PanelMiddleClickRule;
  /* IS $ */
  /* $ 19.09.2000 SVS
   + Opt.PanelCtrlAltShiftRule ������ ��������� Ctrl-Alt-Shift ��� �������.
  */
  int PanelCtrlAltShiftRule;
  /* SVS $*/
  /* $ 20.10.2000 SVS
    Panel/CtrlFRule � ������� - ������ ��������� Ctrl-F
    ���� = 0, �� ���������� ���� ��� ����, ����� - � ������
    ����������� �� ������
  */
  int PanelCtrlFRule;
  /* SVS $*/
  /* $ 27.09.2000 SVS
   + Opt.AllCtrlAltShiftRule - ������� �����, ������ ��������� Ctrl-Alt-Shift
     ��� ���������� - ������� ��������:
     0 - Panel
     1 - Edit
     2 - View
     3 - Help
     4 - Dialog
  */
  int AllCtrlAltShiftRule;
  /* SVS $*/
  int CASRule; // 18.12.2003 - ������� ��������� ����� � ������ CAS (������� #1).
  /* $ 24.09.2000 SVS
   + Opt.CmdHistoryRule ������ ��������� Esc ��� ��������� ������:
      =1 - �� �������� ��������� � History, ���� ����� Ctrl-E/Ctrl/-X
           ������ ESC (��������� - ��� VC).
      =0 - ��������� ��� � ���� - �������� ��������� � History
  */
  int CmdHistoryRule;
  /* SVS $*/
  DWORD ExcludeCmdHistory;
  /* $ 20.09.2000 SVS
   + Opt.SubstPluginPrefix - 1 = ��������������� ������� �������
     ��� Ctrl-[ � ��� ��������
  */
  int SubstPluginPrefix;
  /* SVS $*/
  /* $ 24.09.2000 SVS
   + Opt.MaxPositionCache - ���������� ������� � ���� ����������
  */
  int MaxPositionCache;
  /* SVS $*/
  /* $ 22.11.2000 SVS
   + ������� �� ���� ��������� ��������� �� ��������*/
  int SetAttrFolderRules;
  /* SVS $ */
  /* $ 27.11.2000 SVS
   + Opt.ExceptRules - ������� �� ���� ������ ���������� */
  int ExceptRules;
  /* $ 26.02.2001 VVM
   + Opt.ExceptCallDebugger - �������� �������� ��� ���������� */
  int ExceptCallDebugger;
  /* VVM $ */
  /* SVS $ */
  /* $ 28.12.2000 SVS
   + Opt.HotkeyRules - ������� �� ���� ������ ��������� ������� */
  int HotkeyRules;
  /* SVS $ */
  /* $ 09.01.2001 SVS
   + Opt.ShiftsKeyRules - ������� �� ���� ������ ��������� ����������
     Alt-����� ��� ����������� ������� � �������� "`-=[]\;',./" �
     �������������� Alt-, Ctrl-, Alt-Shift-, Ctrl-Shift-, Ctrl-Alt- */
  int ShiftsKeyRules;
  /* SVS $ */
  /* $ 19.01.2001 SVS
   + Opt.MacroReuseRules - ������� �� ���� �������� ������������� �����������
     ������ */
  int MacroReuseRules;
  /* SVS $ */
  int IgnoreErrorBadPathName;

  DWORD KeyMacroCtrlDot; // ��� KEY_CTRLDOT
  DWORD KeyMacroCtrlShiftDot; // ��� KEY_CTRLSHIFTDOT
  /* $ 22.01.2001 SVS
   + Opt.CursorSize - ������ ������� ���� :-)
     ������ */
  int CursorSize[4];
  /* SVS $ */
  /* $ 05.09.2000 SVS
     � Opt �������� ���� ����������, �������� QWERTY-�������������
  */
  struct CodeXLAT XLat;
  /* SVS $*/
  /*$ 08.02.2001 SKV
    ���������� ������ ��� ������ Far'������ �������
    �� ����������� ���������������� �������� � ��� �����������.
  */
  int ConsoleDetachKey;
  /* SKV$*/

  int UsePrintManager;

  string strHelpLanguage;
  int FullScreenHelp;
  int HelpTabSize;
  /* $ 27.09.2000 SVS
   + Opt.HelpURLRules - =0 ��������� ����������� ������� URL-����������
  */
  int HelpURLRules;

  /* SVS $*/
  /* $ 28.03.2001 VVM
    + RememberLogicalDrives = ���������� ���������� ����� � �� ����������
      ������ ���. ��� �������������� "����������" "�������" ������. */
  int RememberLogicalDrives;
  /* VVM $ */
  /* $ 02.04.2001 VVM
    + Opt.FlagPosixSemantics ����� ������ ��:
        ���������� ������ � ������� � ������ ���������
        ���������� LastPositions � ��������� � ������ */
  int FlagPosixSemantics;
  /* VVM $ */
  /* $ 16.04.2001 VVM
    + Opt.MouseWheelDelta - ������ �������� ��� ���������. */
  int MsWheelDelta;
  /* VVM $ */
  int MsWheelDeltaView;
  int MsWheelDeltaEdit;
  int MsWheelDeltaHelp;
  /* $ 28.04.2001 VVM
    + Opt.SubstNameRule ������� �����:
      0 - ���� ����������, �� ���������� ������� ����� ��� GetSubstName()
      1 - ���� ����������, �� ���������� ��� ��������� ��� GetSubstName() */
  int SubstNameRule;
  /* VVM $ */

  /* $ 23.05.2001 AltF9
    + Opt.AltF9 ���� ��������� ������� ��������  ������ ���������� Alt-F9
         (��������� ������� ������) � ������� ������. �� ��������� - 1.
      0 - ������������ ��������, ����������� � FAR ������ 1.70 beta 3 �
         ����, �.�. ������������ 25/50 �����.
      1 - ������������ ������������������� �������� - ���� FAR Manager
         ����� ������������� � ����������� �� ����������� ��������� ������
         ����������� ���� � �������.*/
  int AltF9;
  /* OT $ */
  int PgUpChangeDisk;
  int ShowCheckingFile;
  int CloseConsoleRule;
  int CloseCDGate;       // ���������������� CD

  DWORD LCIDSort;
  int RestoreCPAfterExecute;
  int ExecuteShowErrorMessage;
  int ExecuteUseAppPath;
  int ExecuteFullTitle;
  string strExecuteBatchType;

#if defined(FAR_ANSI)
  int FarAnsi;
#endif
  DWORD PluginMaxReadData;
  int UseNumPad;
  int UseUnicodeConsole;
  int ScanJunction;

  DWORD ShowTimeoutDelFiles; // ������ � �������� �������� (� ms)
  DWORD ShowTimeoutDACLFiles;
  int DelThreadPriority; // ��������� �������� ��������, �� ��������� = THREAD_PRIORITY_NORMAL

  //int CPAJHefuayor; // ����������� �� "Close Plugin And Jump:
                  // Highly experimental feature, use at your own risk"

  string strDateFormat; // ��� $Date
  struct LoadPluginsOptions LoadPlug;

  struct DialogsOptions Dialogs;
  struct PoliciesOptions Policies;
  struct NowellOptions Nowell;
  struct ScreenSizes ScrSize;
  /* $ 17.09.2003 KM
       ��������� ��� ����������� ���������� ������ �������� � ������
  */
  struct FindCharTable CharTable;
  /* KM $ */
  /* $ 28.09.2003 KM
     OpFilter - �������� ��� ����������� ������� ��������� �������
  */
  struct FilterParams OpFilter;
  /* KM $ */
  struct TreeOptions Tree;
};


struct PluginHandle
{
  HANDLE InternalHandle;
  int PluginNumber;
};

// for class Edit
struct ColorItem
{
  int StartPos;
  int EndPos;
  int Color;
};

/* ���������!!! ���������� ������� ������ :-))
   ��������� ������ ���� 16 ����!
*/
struct HighlightDataColor
{
  BYTE Color;
  BYTE SelColor;
  BYTE CursorColor;
  BYTE CursorSelColor;
  wchar_t MarkChar;
  BYTE Reserved[10];
};

struct PreRedrawParamStruct
{
  DWORD Flags;
  void *Param1;
  const void *Param2;
  const void *Param3;
  void *Param4;
  __int64 Param5;
};

struct FAR_FIND_DATA_EX
{
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    unsigned __int64 nFileSize;

    union {
        unsigned __int64 nPackSize; //same as reserved
        struct {
            DWORD dwReserved0;
            DWORD dwReserved1;
        };
    };

    string   strFileName;
    string   strAlternateFileName;
};


#endif // __FARSTRUCT_HPP__
