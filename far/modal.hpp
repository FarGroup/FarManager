#ifndef __MODAL_HPP__
#define __MODAL_HPP__
/*
modal.hpp

Parent class для модальных объектов

*/

/* Revision: 1.07 26.07.2001 $ */

/*
Modify:
  26.07.2001 OT
    Косметическое исправление (борьба варнингами)
  18.07.2001 OT
    VFMenu
  14.06.2001 OT
    ! "Бунт" ;-)
  06.05.2001 DJ
    ! перетрях #include
  05.05.2001 DJ
    + Перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  29.06.2000 tran
    - (NT Console resize bug)
      adding virtual method SetScreenPosition
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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

    void SetHelp(char *Topic);
    void ShowHelp();
//    void SetScreenPosition(){inherited::SetScreenPosition();}

};


#endif  //__MODAL_HPP__
