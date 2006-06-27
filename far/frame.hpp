#ifndef __FRAME_HPP__
#define __FRAME_HPP__

/*
frame.hpp

����������� ���� (������� ����� ��� FilePanels, FileEditor, FileViewer)

*/

/* Revision: 1.24 04.06.2006 $ */

#include "scrobj.hpp"
#include "UnicodeString.hpp"

class KeyBar;

// ������� �� �������� ���������������� ���������
// WTYPE_* (plugin.hpp) � MODALTYPE_*!!!
// (� �� ���� ������� ���� �����������, ���� �������� �� ��������� ;)
enum { MODALTYPE_VIRTUAL,
  MODALTYPE_PANELS=1,
  MODALTYPE_VIEWER,
  MODALTYPE_EDITOR,
  MODALTYPE_DIALOG,
  MODALTYPE_VMENU,
  MODALTYPE_HELP,
  MODALTYPE_COMBOBOX,
  MODALTYPE_USER,
};

class Frame: virtual public ScreenObject
{
  friend class Manager;
  private:
//    Frame **ModalStack;
//    int  ModalStackCount, ModalStackSize;
    Frame *FrameToBack;
    Frame *NextModal,*PrevModal;

  protected:
    int  DynamicallyBorn;
    int  CanLoseFocus;
    int  ExitCode;
    int  KeyBarVisible;
    KeyBar *FrameKeyBar;
    int MacroMode;

  public:
    Frame();
    virtual ~Frame();

//    int ProcessKey(int Key);
//    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

    virtual int GetCanLoseFocus(int DynamicMode=FALSE) { return(CanLoseFocus); };
    void SetCanLoseFocus(int Mode) { CanLoseFocus=Mode; };
    int  GetExitCode() { return ExitCode; };
    virtual void SetExitCode(int Code) { ExitCode=Code; };

    virtual BOOL IsFileModified() {return(FALSE);};

    virtual const wchar_t *GetTypeName() {return L"[FarModal]";};
    virtual int GetTypeAndName(string &strType, string &strName) {return(MODALTYPE_VIRTUAL);};
    virtual int GetType() { return MODALTYPE_VIRTUAL; }

    virtual void OnDestroy();  // ���������� ����� ������������ ����
    virtual void OnCreate() {};   // ���������� ����� ��������� ����
    virtual void OnChangeFocus(int focus); // ���������� ��� ����� ������
    virtual void Refresh() {OnChangeFocus(1);};  // ������ �������������� :)

    virtual void InitKeyBar(void) {}
    void SetKeyBar(KeyBar *FrameKeyBar);
    void UpdateKeyBar();
    virtual void RedrawKeyBar() { Frame::UpdateKeyBar(); };

    /* $ 12.05.2001 DJ */
    int IsTopFrame();
    virtual int GetMacroMode() { return MacroMode; }
    /* DJ $ */
    void Push(Frame* Modalized);
    Frame *GetTopModal(){return NextModal;};
//    bool Pop();
//    Frame *operator[](int Index);
//    int operator[](Frame *ModalFarame);
//    int ModalCount() {return ModalStackCount;}
    void DestroyAllModal();
    void SetDynamicallyBorn(int Born) {DynamicallyBorn=Born;}
    int GetDynamicallyBorn(){return DynamicallyBorn;};
    virtual int FastHide();
//    int IndexOf(Frame *aFrame);
    bool RemoveModal(Frame *aFrame);
    void ResizeConsole();
    bool HasSaveScreen();
//    bool ifFullConsole();
    virtual void GetTitle(string &Title,int SubLen=-1,int TruncSize=0){};
};

#endif // __FRAME_HPP__
