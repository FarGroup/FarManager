#ifndef __CONSTITLE_HPP__
#define __CONSTITLE_HPP__
/*
constitle.hpp

Заголовок консоли

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
