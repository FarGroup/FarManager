#ifndef __FRAME_HPP__
#define __FRAME_HPP__

/*
frame.hpp

Ќемодальное окно (базовый класс дл€ FilePanels, FileEditor, FileViewer)

*/

/* Revision: 1.01 06.05.2001 */

/*
  Modify:
    06.05.2001 DJ
      ! перетр€х #include
    05.05.2001 DJ
      created
*/

#include "scrobj.hpp"

class KeyBar;

enum { MODALTYPE_VIRTUAL,
  MODALTYPE_PANELS,
  MODALTYPE_VIEWER,
  MODALTYPE_EDITOR,
  MODALTYPE_DIALOG,
  MODALTYPE_VMENU,
  MODALTYPE_HELP,
  MODALTYPE_USER
};

class Frame: public ScreenObject
{
  protected:
    int  EnableSwitch;
    int  ExitCode;
    KeyBar *ModalKeyBar;

  public:
    Frame();

    int  GetEnableSwitch() {return(EnableSwitch);};
    void SetEnableSwitch(int Mode) {EnableSwitch=Mode;};
    int  GetExitCode() { return ExitCode; };
    void SetExitCode(int Code) { ExitCode=Code; };

    virtual int IsFileModified() {return(FALSE);};
    virtual char *GetTypeName() {return "Modal";};
    virtual int GetTypeAndName(char *Type,char *Name) {return(MODALTYPE_VIRTUAL);};
    virtual int GetType() { return MODALTYPE_VIRTUAL; }

    virtual void OnDestroy() {};  // вызываетс€ перед уничтожением окна
    virtual void OnCreate() {};   // вызываетс€ перед созданием окна
    virtual void OnChangeFocus(int focus) {}; // вызываетс€ при смене фокуса

    void SetKeyBar(KeyBar *ModalKeyBar);
    void UpdateKeyBar();
    virtual void RedrawKeyBar() { Frame::UpdateKeyBar(); };
    int  KeyBarVisible;

    int MacroMode;
};

#endif // __FRAME_HPP__
