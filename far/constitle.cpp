/*
constitle.cpp

Заголовок консоли

*/

/* Revision: 1.01 06.05.2001 $ */

/*
Modify:
  06.05.2001 DJ
    + перетрях #include
  20.03.2001 tran
    ! created
*/

#include "headers.hpp"
#pragma hdrstop

#include "constitle.hpp"
#include "fn.hpp"

ConsoleTitle::ConsoleTitle(char *title)
{
    GetConsoleTitle(OldTitle,512);
    SetFarTitle(title);
}

ConsoleTitle::~ConsoleTitle()
{
    SetConsoleTitle(OldTitle);
}

void ConsoleTitle::Set(char *fmt,...)
{
    char msg[512];

    va_list argptr;
    va_start( argptr, fmt );

    vsprintf( msg, fmt, argptr );
    va_end(argptr);
    SetFarTitle(msg);
}
