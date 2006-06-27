#ifndef __CONSTITLE_HPP__
#define __CONSTITLE_HPP__
/*
constitle.hpp

��������� �������

*/

/* Revision: 1.03 16.07.2005 $ */

/*
Modify:
  16.07.2005 WARP
    ! ����� ConsoleTitle ��������� ���������.
  01.04.2002 SVS
    ! ��� ���������
  14.05.2001 SVS
    + ������� ����������� - �� ��������� title=NULL
  20.03.2001 tran
    ! created
*/
#include "UnicodeString.hpp"

class ConsoleTitle
{
  private:
    string strOldTitle;

  public:
    ConsoleTitle(const wchar_t *title=NULL);
    ~ConsoleTitle();

  public:
    void Set(const wchar_t *fmt,...);

};

#endif
