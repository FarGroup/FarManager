#ifndef __CONSTITLE_HPP__
#define __CONSTITLE_HPP__
/*
constitle.hpp

Заголовок консоли

*/

/* Revision: 1.01 14.05.2001 $ */

/*
Modify:
  14.05.2001 SVS
    + Изменен конструктор - по умолчанию title=NULL
  20.03.2001 tran
    ! created
*/


class ConsoleTitle
{
    public:
    ConsoleTitle(char *title=NULL);
    ~ConsoleTitle();

    void Set(char *fmt,...);

    private:
    char OldTitle[512];
};

#endif
