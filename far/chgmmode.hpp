#ifndef __CHANGEMACROMODE_HPP__
#define __CHANGEMACROMODE_HPP__
/*
chgmmode.hpp

class ChangeMacroMode

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class ChangeMacroMode
{
  private:
    int PrevMacroMode;
  public:
    ChangeMacroMode(int NewMode);
    ~ChangeMacroMode();
};

#endif	// __CHANGEMACROMODE_HPP__

