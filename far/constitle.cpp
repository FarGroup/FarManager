/*
constitle.cpp

«аголовок консоли

*/

/* Revision: 1.02 14.05.2001 $ */

/*
Modify:
  14.05.2001 SVS
    + изменен конструктор
  06.05.2001 DJ
    + перетр€х #include
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
    if(title)
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
