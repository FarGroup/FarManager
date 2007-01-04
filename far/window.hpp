#ifndef __FRAME_HPP__
#define __FRAME_HPP__

/*
frame.hpp

Немодальное окно (базовый класс для FilePanels, FileEditor, FileViewer)

*/

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

    virtual void OnDestroy() {};  // вызывается перед уничтожением окна
    virtual void OnCreate() {};   // вызывается перед созданием окна
    virtual void OnChangeFocus(int focus) {}; // вызывается при смене фокуса

    void SetKeyBar(KeyBar *ModalKeyBar);
    void UpdateKeyBar();
    virtual void RedrawKeyBar() { Frame::UpdateKeyBar(); };
    int  KeyBarVisible;

    int MacroMode;
};

#endif // __FRAME_HPP__
