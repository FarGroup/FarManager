#ifndef __REDRAWDESKTOP_HPP__
#define __REDRAWDESKTOP_HPP__
/*
rdrwdsk.hpp

class RedrawDesktop

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class RedrawDesktop
{
  private:
    int LeftVisible,RightVisible;
  public:
    RedrawDesktop();
    ~RedrawDesktop();
};


#endif	// __REDRAWDESKTOP_HPP__

