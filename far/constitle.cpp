/*
constitle.cpp

Заголовок консоли

*/

/* Revision: 1.00 20.03.2001 $ */

/*
Modify:
  20.03.2001 tran
    ! created
*/

#include "headers.hpp"
#pragma hdrstop
#include "internalheaders.hpp"

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
