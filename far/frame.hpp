#ifndef __FRAME_HPP__
#define __FRAME_HPP__

/*
frame.hpp

Немодальное окно (базовый класс для FilePanels, FileEditor, FileViewer)

*/

/* Revision: 1.04 12.05.2001 */

/*
  Modify:
    12.05.2001 DJ
      ! отрисовка по OnChangeFocus сделана дефолтным поведением
    12.05.2001 DJ
      + IsTopFrame(), GetMacroMode()
    07.05.2001 DJ
      ! причешем идентификаторы
    06.05.2001 DJ
      ! перетрях #include
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
    int  CanLoseFocus;
    int  ExitCode;
    int  KeyBarVisible;
    KeyBar *ModalKeyBar;
    int MacroMode;

  public:
    Frame();
    virtual ~Frame();

    int  GetCanLoseFocus() { return(CanLoseFocus); };
    void SetCanLoseFocus(int Mode) { CanLoseFocus=Mode; };
    int  GetExitCode() { return ExitCode; };
    void SetExitCode(int Code) { ExitCode=Code; };

    virtual BOOL IsFileModified() {return(FALSE);};
    virtual char *GetTypeName() {return "Modal";};
    virtual int GetTypeAndName(char *Type,char *Name) {return(MODALTYPE_VIRTUAL);};
    virtual int GetType() { return MODALTYPE_VIRTUAL; }

    virtual void OnDestroy() {};  // вызывается перед уничтожением окна
    virtual void OnCreate() {};   // вызывается перед созданием окна
    virtual void OnChangeFocus(int focus); // вызывается при смене фокуса

    void SetKeyBar(KeyBar *ModalKeyBar);
    void UpdateKeyBar();
    virtual void RedrawKeyBar() { Frame::UpdateKeyBar(); };

    /* $ 12.05.2001 DJ */
    int IsTopFrame();
    int GetMacroMode() { return MacroMode; }
    /* DJ $ */
};

#endif // __FRAME_HPP__
