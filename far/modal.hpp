#ifndef __MODAL_HPP__
#define __MODAL_HPP__
/*
modal.hpp

Parent class для модальных объектов

*/

/* Revision: 1.02 29.04.2001 $ */

/*
Modify:
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  29.06.2000 tran
    - (NT Console resize bug)
      adding virtual method SetScreenPosition
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class Modal:public ScreenObject
{
  private:
    int     ReadKey,
            WriteKey;
    KeyBar *ModalKeyBar;

  protected:
    INPUT_RECORD ReadRec;
    char HelpTopic[512];
    int  ExitCode;
    int  EndLoop;
    int  EnableSwitch;

  public:
    Modal();
    virtual void GetDialogObjectsData() {};
    int Done();
    void ClearDone();
    int  GetExitCode();
    void SetExitCode(int Code);

    int  GetEnableSwitch() {return(EnableSwitch);};
    void SetEnableSwitch(int Mode) {EnableSwitch=Mode;};

    virtual void Process();

    int  ReadInput();
    void WriteInput(int Key);
    void ProcessInput();

    void SetHelp(char *Topic);
    void ShowHelp();

    void SetKeyBar(KeyBar *ModalKeyBar);
    void UpdateKeyBar();
    virtual void RedrawKeyBar() { Modal::UpdateKeyBar(); };
    int  KeyBarVisible;

    virtual int GetTypeAndName(char *Type,char *Name) {return(MODALTYPE_VIRTUAL);};
    virtual int IsFileModified() {return(FALSE);};
    /* $ 28.06.2000 tran
       (NT Console resize bug) adding virtual method */
    virtual void SetScreenPosition();
    /* tran $ */
    virtual char *GetTypeName() {return "Modal";};

    virtual void OnDestroy() {};  // вызывается перед уничтожением окна
    virtual void OnCreate() {};   // вызывается перед созданием окна
    virtual void OnChangeFocus(int focus) {}; // вызывается при смене фокуса


    int MacroMode;
    int Focus;
};


#endif  //__MODAL_HPP__


