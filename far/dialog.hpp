#ifndef __DIALOG_HPP__
#define __DIALOG_HPP__
/*
dialog.hpp

����� ������� Dialog.

������������ ��� ����������� ��������� ��������.
�������� ����������� �� ������ Frame.

*/

/* Revision: 1.92 25.05.2006 $ */

#include "frame.hpp"
#include "plugin.hpp"
#include "vmenu.hpp"
#include "bitflags.hpp"
#include "CriticalSections.hpp"
#include "UnicodeString.hpp"

// ����� �������� ������ �������
#define DMODE_INITOBJECTS   0x00000001 // �������� ����������������?
#define DMODE_CREATEOBJECTS 0x00000002 // ������� (Edit,...) �������?
#define DMODE_WARNINGSTYLE  0x00000004 // Warning Dialog Style?
#define DMODE_DRAGGED       0x00000008 // ������ ���������?
#define DMODE_ISCANMOVE     0x00000010 // ����� �� ������� ������?
#define DMODE_ALTDRAGGED    0x00000020 // ������ ��������� �� Alt-�������?
#define DMODE_SMALLDIALOG   0x00000040 // "�������� ������"
#define DMODE_DRAWING       0x00001000 // ������ ��������?
#define DMODE_KEY           0x00002000 // ���� ������� ������?
#define DMODE_SHOW          0x00004000 // ������ �����?
#define DMODE_MOUSEEVENT    0x00008000 // ����� �������� MouseMove � ����������?
#define DMODE_RESIZED       0x00010000 //
#define DMODE_ENDLOOP       0x00020000 // ����� ����� ��������� �������?
#define DMODE_BEGINLOOP     0x00040000 // ������ ����� ��������� �������?
#define DMODE_OWNSITEMS     0x00080000 // ���� TRUE, Dialog ����������� ������ Item � �����������
#define DMODE_NODRAWSHADOW  0x00100000 // �� �������� ����?
#define DMODE_NODRAWPANEL   0x00200000 // �� �������� ��������?
#define DMODE_CLICKOUTSIDE  0x20000000 // ���� ������� ���� ��� �������?
#define DMODE_MSGINTERNAL   0x40000000 // ���������� Message?
#define DMODE_OLDSTYLE      0x80000000 // ������ � ������ (�� 1.70) �����

#define DIMODE_REDRAW       0x00000001 // ��������� �������������� ���������� �����?

// ����� ��� ������� ConvertItem
#define CVTITEM_TOPLUGIN    0
#define CVTITEM_FROMPLUGIN  1

#define DMOUSEBUTTON_LEFT   0x00000001
#define DMOUSEBUTTON_RIGHT  0x00000002

enum DLGEDITLINEFLAGS {
  DLGEDITLINE_CLEARSELONKILLFOCUS = 0x00000001, // ��������� ���������� ����� ��� ������ ������ �����
  DLGEDITLINE_SELALLGOTFOCUS      = 0x00000002, // ��������� ���������� ����� ��� ��������� ������ �����
  DLGEDITLINE_NOTSELONGOTFOCUS    = 0x00000004, // �� ��������������� ��������� ������ �������������� ��� ��������� ������ �����
  DLGEDITLINE_NEWSELONGOTFOCUS    = 0x00000008, // ��������� ��������� ��������� ����� ��� ��������� ������
  DLGEDITLINE_GOTOEOLGOTFOCUS     = 0x00000010, // ��� ��������� ������ ����� ����������� ������ � ����� ������
  DLGEDITLINE_PERSISTBLOCK        = 0x00000020, // ���������� ����� � ������� �����
  DLGEDITLINE_AUTOCOMPLETE        = 0x00000040, // �������������� � ������� �����
  DLGEDITLINE_AUTOCOMPLETECTRLEND = 0x00000040, // ��� �������������� ������������ ����������� Ctrl-End
  DLGEDITLINE_HISTORY             = 0x00000100, // ������� � ������� ����� ��������
};


enum DLGITEMINTERNALFLAGS {
  DLGIIF_LISTREACTIONFOCUS        = 0x00000001, // MouseReaction ��� ��������� ��������
  DLGIIF_LISTREACTIONNOFOCUS      = 0x00000002, // MouseReaction ��� �� ��������� ��������
  DLGIIF_EDITPATH                 = 0x00000004, // ����� Ctrl-End � ������ �������������� ����� �������� �� ���� �������������� ������������ ����� � ���������� � ������ �� �������
};


#define MakeDialogItemsEx(Data,Item) \
  struct DialogItemEx Item[sizeof(Data)/sizeof(Data[0])]; \
  Dialog::DataToItemEx(Data,Item,sizeof(Data)/sizeof(Data[0]));

// ���������, ����������� ������������� ��� DIF_AUTOMATION
// �� ������ ����� - ����������� - ����������� ������ � ��������� ��� CheckBox
struct DialogItemAutomation{
  WORD ID;                    // ��� ����� ��������...
  DWORD Flags[3][2];          // ...��������� ��� ��� �����
                              // [0] - Unchecked, [1] - Checked, [2] - 3Checked
                              // [][0] - Set, [][1] - Skip
};

// ������ ��� DI_USERCONTROL
class DlgUserControl{
  public:
    COORD CursorPos;
    int   CursorVisible,CursorSize;

  public:
    DlgUserControl(){CursorSize=CursorPos.X=CursorPos.Y=-1;CursorVisible=0;}
   ~DlgUserControl(){};
};

/*
��������� ���� ������� ������� - ��������� �������������.
��� �������� ��� FarDialogItem (�� ����������� ObjPtr)
*/
struct DialogItemEx
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  union
  {
    int Selected;
    const wchar_t *History;
    const wchar_t *Mask;
    struct FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  int DefaultButton;

  string strData;
  int nMaxLength;

  WORD ID;
  BitFlags IFlags;
  int AutoCount;   // �������������
  struct DialogItemAutomation* AutoPtr;
  DWORD UserData; // ��������������� ������

  // ������
  void *ObjPtr;
  VMenu *ListPtr;
  DlgUserControl *UCData;

  int SelStart;
  int SelEnd;
};

/*
��������� ���� ������� ������� - ��� ���������� �������
��������� ����������� ��������� InitDialogItem (��. "Far PlugRinG
Russian Help Encyclopedia of Developer")
*/

struct DialogDataEx
{
  WORD  Type;
  short X1,Y1,X2,Y2;
  BYTE  Focus;
  union {
    unsigned int Selected;
    const wchar_t *History;
    char *Mask;
    struct FarList *ListItems;
    int  ListPos;
    CHAR_INFO *VBuf;
  };
  DWORD Flags;
  BYTE  DefaultButton;

  const wchar_t *Data;
};


struct FarDialogMessage{
  HANDLE hDlg;
  int    Msg;
  int    Param1;
  long   Param2;
};

class DlgEdit;
class ConsoleTitle;

typedef long (__stdcall *SENDDLGMESSAGE) (HANDLE hDlg,int Msg,int Param1,long Param2);

class Dialog: public Frame
{
  /* $ 21.07.2001 KM
    ! ���������� FindFiles ������ ������� ��� ������� � ����� Item.
  */
  friend class FindFiles;
  /* KM $ */
  private:
    /* $ 29.08.2000 SVS
       + ����� �������, ��� ������������ HelpTopic
    */
    int PluginNumber;
    /* SVS $ */
    /* $ 23.08.2000 SVS
       + ���������� ������ FocusPos
    */
    int FocusPos;               // ������ �������� ����� ������� � ������
    /* SVS $ */
    int PrevFocusPos;           // ������ �������� ����� ������� ��� � ������
    /* $ 18.08.2000 SVS
      + ���� IsEnableRedraw - �����������/����������� ����������� �������
      + DialogMode - ����� �������� ������ �������
    */
    int IsEnableRedraw;         // ��������� ����������� �������? ( 0 - ���������)
    BitFlags DialogMode;        // ����� �������� ������ �������
    /* SVS $ */
    /* $ 11.08.2000 SVS
      + ������, ������������� ��� ����������� ���������� �������
    */
    long DataDialog;            // ������������� ����� ��������,
                                //   ���������� � �����������
    /* SVS $ */
    struct DialogItemEx **Item;    // ������ ��������� �������
    DialogItemEx *pSaveItemEx;

    int ItemCount;              // ���������� ��������� �������

    ConsoleTitle *OldTitle;     // ���������� ���������
    int DialogTooLong;          //
    int PrevMacroMode;          // ���������� ����� �����

    FARWINDOWPROC DlgProc;      // ������� ��������� �������
    SENDDLGMESSAGE pfnSendDlgMessage;

    /* $ 31.07.2000 tran
       ���������� ��� ����������� ������� */
    int  OldX1,OldX2,OldY1,OldY2;
    /* tran 31.07.2000 $ */

    /* $ 17.05.2001 DJ */
    wchar_t *HelpTopic;
    /* DJ $ */
    /* $ 23.06.2001 KM
       + �������� ������ ���������� � �������:
         TRUE - ������, FALSE - ������.
    */
    volatile int DropDownOpened;
    /* KM $ */
    CriticalSection CS;

    int RealWidth, RealHeight;

  private:
    void DisplayObject();
    void DeleteDialogObjects();
    int  LenStrItem(int ID, const wchar_t *lpwszStr = NULL);
    /* $ 22.08.2000 SVS
      ! ShowDialog - �������������� �������� - ����� ������� ������������
        ID=-1 - ���������� ���� ������
    */
    void ShowDialog(int ID=-1);
    /* SVS $ */

    DWORD CtlColorDlgItem(int ItemPos,int Type,int Focus,DWORD Flags);
    /* $ 28.07.2000 SVS
       + �������� ����� ����� ����� ����� ����������.
         ������� �������� ��� ����, ����� ���������� DMSG_KILLFOCUS & DMSG_SETFOCUS
    */
    int ChangeFocus2(int KillFocusPos,int SetFocusPos);
    /* SVS $ */
    int ChangeFocus(int FocusPos,int Step,int SkipGroup);
    static int IsEdit(int Type);
    /* $ 28.07.2000 SVS
       �������, ������������ - "����� �� ������� ������� ����� ����� �����"
    */
    static int IsFocused(int Type);
    /* SVS $ */
    /* $ 26.07.2000 SVS
      + �������������� �������� � SelectFromEditHistory ��� ���������
       ������ ������� � ������� (���� ��� ������������� ������ �����)
    */
    BOOL SelectFromEditHistory(struct DialogItemEx *CurItem,DlgEdit *EditLine,const wchar_t *HistoryName,string &strStr,int MaxLen);
    /* SVS $ */
    /* $ 18.07.2000 SVS
       + ������� SelectFromComboBox ��� ������ �� DI_COMBOBOX
    */
    int SelectFromComboBox(struct DialogItemEx *CurItem,DlgEdit*EditLine,VMenu *List,int MaxLen);
    /* SVS $ */
    /* $ 26.07.2000 SVS
       AutoComplite: ����� ��������� ��������� � �������
    */
    int FindInEditForAC(int TypeFind, const wchar_t *HistoryName, string &strFindStr);
    /* SVS $ */
    int AddToEditHistory(const wchar_t *AddStr,const wchar_t *HistoryName);

    /* $ 09.12.2001 DJ
       ��������� DIF_USELASTHISTORY
    */
    void ProcessLastHistory (struct DialogItemEx *CurItem, int MsgIndex);
    /* DJ $ */

    int ProcessHighlighting(int Key,int FocusPos,int Translate);
    BOOL CheckHighlights(WORD Chr);

    /* $ 08.09.2000 SVS
      ������� SelectOnEntry - ��������� ������ ��������������
      ��������� ����� DIF_SELECTONENTRY
    */
    void SelectOnEntry(int Pos,BOOL Selected);
    /* SVS $ */

    void CheckDialogCoord(void);
    BOOL GetItemRect(int I,RECT& Rect);

    /* $ 19.05.2001 DJ
       ���������� ��������� ������� (����� ������� ������ ��� ������)
    */
    const wchar_t *GetDialogTitle();
    /* DJ $ */

    /* $ 30.05.2000 KM
       ������ ���������� ��� ������ ����� �������.
    */
    BOOL SetItemRect(int ID,SMALL_RECT *Rect);
    /* KM $ */

    /* $ 23.06.2001 KM
       + ������� ������������ ��������/�������� ���������� � �������
         � ��������� ������� ����������/���������� ���������� � �������.
    */
    volatile void SetDropDownOpened(int Status){ DropDownOpened=Status; }
    volatile int GetDropDownOpened(){ return DropDownOpened; }
    /* KM $ */

    void ProcessCenterGroup(void);
    int ProcessRadioButton(int);

    /* $ 24.08.2000 SVS
       InitDialogObjects ����� �������� - ��� ���������� ���������������
       ���������
    */
    int  InitDialogObjects(int ID=-1);
    /* 24.08.2000 SVS $ */

    int ProcessOpenComboBox(int Type,struct DialogItemEx *CurItem,int CurFocusPos);
    int ProcessMoveDialog(DWORD Key);

    int Do_ProcessTab(int Next);
    int Do_ProcessNextCtrl(int Next,BOOL IsRedraw=TRUE);
    int Do_ProcessFirstCtrl();
    int Do_ProcessSpace();

    int CallDlgProc (int nMsg, int nParam1, int nParam2);

  public:
    Dialog(struct DialogItemEx *Item,int ItemCount,FARWINDOWPROC DlgProc=NULL,long Param=0);
    ~Dialog();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Show();
    /* $ 30.08.2000 SVS
       ������� ����������� Hide()
    */
    void Hide();
    /* SVS $ */
    void FastShow() {ShowDialog();}
    /* $ 28.07.2000 SVS
       ������ InitDialogObjects ���������� ID ��������
       � ������� �����
    */
    /* SVS $ */
    void GetDialogObjectsData();

    void SetDialogMode(DWORD Flags){ DialogMode.Set(Flags); }

    /* $ 28.07.2000 SVS
       + ������� ConvertItem - �������������� �� ����������� �������������
        � FarDialogItem � �������
    */
    static void ConvertItemEx (int FromPlugin,struct FarDialogItem *Item,struct DialogItemEx *Data,
                           int Count,BOOL InternalCall=FALSE);

    /* SVS $ */
    static void DataToItemEx(struct DialogDataEx *Data,struct DialogItemEx *Item,
                           int Count);

    static int IsKeyHighlighted(const wchar_t *Str,int Key,int Translate,int AmpPos=-1);

    /* $ 31.07.2000 tran
       ����� ��� ����������� ������� */
    void AdjustEditPos(int dx,int dy);
    /* tran 31.07.2000 $ */

    /* $ 09.08.2000 KM
       ���������� �������, ������� ��������� ���������
       ��������� �� ������ � ������ �����������.
    */
    int IsMoving() {return DialogMode.Check(DMODE_DRAGGED);}
    /* KM $ */
    /* $ 10.08.2000 SVS
       ����� �� ������� ������ :-)
    */
    void SetModeMoving(int IsMoving) { DialogMode.Change(DMODE_ISCANMOVE,IsMoving);}
    int  GetModeMoving(void) {return DialogMode.Check(DMODE_ISCANMOVE);}
    /* SVS $ */
    /* $ 11.08.2000 SVS
       ������ � ���. ������� ���������� �������
    */
    void SetDialogData(long NewDataDialog);
    long GetDialogData(void) {return DataDialog;};
    /* SVS $ */

    void InitDialog(void);
    /* $ 11.08.2000 SVS
       ��� ����, ����� ������� DMSG_CLOSE ����� �������������� Process
    */
    void Process();
    /* SVS $ */
    /* $ 29.08.2000 SVS
       + ���������� ����� �������, ��� ������������ HelpTopic
    */
    void SetPluginNumber(int NewPluginNumber){PluginNumber=NewPluginNumber;}
    /* SVS $ */

    /* $ 17.05.2001 DJ */
    void SetHelp(const wchar_t *Topic);
    void ShowHelp();
    int Done() { return DialogMode.Check(DMODE_ENDLOOP); }
    void ClearDone();
    virtual void SetExitCode (int Code);

    void CloseDialog();
    /* DJ $ */

    /* $ 19.05.2001 DJ */
    // void SetOwnsItems (int AOwnsItems) { AOwnsItems = OwnsItems; } !!!!!!! :-)
    void SetOwnsItems (int AOwnsItems) { DialogMode.Change(DMODE_OWNSITEMS,AOwnsItems); }

    virtual int GetTypeAndName(string &strType, string &strName);
    virtual int GetType() { return MODALTYPE_DIALOG; }
    virtual const wchar_t *GetTypeName() {return L"[Dialog]";};
    /* DJ $ */

    int GetMacroMode();

    /* $ ������� ��� ���� CtrlAltShift OT */
    int FastHide();
    void ResizeConsole();
//    void OnDestroy();

    // For MACRO
    const struct DialogItemEx **GetAllItem(){return (const DialogItemEx**)Item;};
    int GetAllItemCount(){return ItemCount;};              // ���������� ��������� �������
    int GetDlgFocusPos(){return FocusPos;};


    int SetAutomation(WORD IDParent,WORD id,
                        DWORD UncheckedSet,DWORD UncheckedSkip,
                        DWORD CheckedSet,DWORD CheckedSkip,
                        DWORD Checked3Set=0,DWORD Checked3Skip=0);

    /* $ 23.07.2000 SVS: ������� ��������� ������� (�� ���������) */
    static long WINAPI DefDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    /* $ 28.07.2000 SVS: ������� ������� ��������� ������� */
    static long __stdcall SendDlgMessage (HANDLE hDlg,int Msg,int Param1,long Param2)
    {
        Dialog *D = (Dialog*)hDlg;
        return D->pfnSendDlgMessage (hDlg, Msg, Param1, Param2);
    }

    static long __stdcall SendDlgMessageAnsi(HANDLE hDlg,int Msg,int Param1,long Param2);
    static long __stdcall SendDlgMessageUnicode(HANDLE hDlg,int Msg,int Param1,long Param2);

    virtual void SetPosition(int X1,int Y1,int X2,int Y2);
};

#endif // __DIALOG_HPP__
