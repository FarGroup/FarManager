#ifndef __MODAL_HPP__
#define __MODAL_HPP__
/*
modal.hpp

Parent class для модальных объектов

*/

/* Revision: 1.01 29.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
  29.06.2000 tran
    - (NT Console resize bug)
      adding virtual method SetScreenPosition
*/

class Modal:public ScreenObject
{
  private:
    int ReadKey,WriteKey;
    KeyBar *ModalKeyBar;
  protected:
    INPUT_RECORD ReadRec;
    char HelpTopic[512];
    int ExitCode;
    int EndLoop;
    int EnableSwitch;
  public:
    Modal();
    virtual void GetDialogObjectsData() {};
    int Done();
    void ClearDone();
    int GetExitCode();
    void SetExitCode(int Code);
    int GetEnableSwitch() {return(EnableSwitch);};
    void SetEnableSwitch(int Mode) {EnableSwitch=Mode;};
    virtual void Process();
    int ReadInput();
    void WriteInput(int Key);
    void ProcessInput();
    void SetHelp(char *Topic);
    void ShowHelp();
    void SetKeyBar(KeyBar *ModalKeyBar);
    virtual int GetTypeAndName(char *Type,char *Name) {return(0);};
    virtual int IsFileModified() {return(FALSE);};
    /* $ 28.06.2000 tran
       (NT Console resize bug) adding virtual method */
    virtual void SetScreenPosition();
    /* tran $ */
};


#endif 	//__MODAL_HPP__
