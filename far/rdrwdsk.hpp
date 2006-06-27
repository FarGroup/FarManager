#ifndef __REDRAWDESKTOP_HPP__
#define __REDRAWDESKTOP_HPP__
/*
rdrwdsk.hpp

class RedrawDesktop

*/

/* Revision: 1.01 12.11.2001 $ */

/*
Modify:
  12.11.2001 SVS
    + данные про кейьбар и топменю + параметр у конструктора
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class RedrawDesktop
{
  private:
    int LeftVisible;
    int RightVisible;
    int KeyBarVisible;
    int TopMenuBarVisible;

  public:
    RedrawDesktop(BOOL IsHidden=FALSE);
    ~RedrawDesktop();
};


#endif  // __REDRAWDESKTOP_HPP__
