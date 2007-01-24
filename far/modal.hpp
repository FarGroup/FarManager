#ifndef __MODAL_HPP__
#define __MODAL_HPP__
/*
modal.hpp

Parent class для модальных объектов

*/

#include "scrobj.hpp"

class Modal: virtual public ScreenObject
{
  private:
    int     ReadKey,
            WriteKey;
    typedef ScreenObject inherited;
  protected:
    INPUT_RECORD ReadRec;
    char HelpTopic[512];
    int  ExitCode;
    int  EndLoop;

  public:
    Modal();
    virtual void GetDialogObjectsData() {};
    int Done();
    void ClearDone();
    int  GetExitCode();
    void SetExitCode(int Code);

    virtual void Process();

    int  ReadInput();
    void WriteInput(int Key);
    void ProcessInput();

    void SetHelp(const char *Topic);
    void ShowHelp();
//    void SetScreenPosition(){inherited::SetScreenPosition();}

};


#endif  //__MODAL_HPP__
