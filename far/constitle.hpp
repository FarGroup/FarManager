#ifndef __CONSTITLE_HPP__
#define __CONSTITLE_HPP__
/*
constitle.hpp

Заголовок консоли

*/

/* Revision: 1.00 20.03.2001 $ */

/*
Modify:
  20.03.2001 tran
    ! created
*/


class ConsoleTitle
{
    public:
    ConsoleTitle(char *title);
    ~ConsoleTitle();

    void Set(char *fmt,...);

    private:
    char OldTitle[512];
};

#endif
