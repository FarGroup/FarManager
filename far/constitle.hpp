#ifndef __CONSTITLE_HPP__
#define __CONSTITLE_HPP__
/*
constitle.hpp

Заголовок консоли

*/

/* Revision: 1.03 16.07.2005 $ */

/*
Modify:
  16.07.2005 WARP
    ! Класс ConsoleTitle полностью юникодный.
  01.04.2002 SVS
    ! Про заголовок
  14.05.2001 SVS
    + Изменен конструктор - по умолчанию title=NULL
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
