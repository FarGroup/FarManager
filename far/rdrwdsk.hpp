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
    + ������ ��� ������� � ������� + �������� � ������������
  25.06.2000 SVS
    ! ���������� Master Copy
    ! ��������� � �������� ���������������� ������
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
