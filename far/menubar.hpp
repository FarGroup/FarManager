#ifndef __MENUBAR_HPP__
#define __MENUBAR_HPP__
/*
menubar.hpp

Показ горизонтального меню при включенном "Always show menu bar"

*/

/* Revision: 1.01 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "scrobj.hpp"

class MenuBar:public ScreenObject
{
  private:
    void DisplayObject();
};

#endif	// __MENUBAR_HPP__
