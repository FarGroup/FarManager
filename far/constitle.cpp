/*
constitle.cpp

Заголовок консоли

*/

/* Revision: 1.03 01.04.2002 $ */

/*
Modify:
  01.04.2002 SVS
    ! Про заголовок - заюзаем SetFarTitle для корректного восстановления после
      макроса
  14.05.2001 SVS
    + изменен конструктор
  06.05.2001 DJ
    + перетрях #include
  20.03.2001 tran
    ! created
*/

#include "headers.hpp"
#pragma hdrstop

#include "constitle.hpp"
#include "global.hpp"
#include "fn.hpp"

ConsoleTitle::ConsoleTitle(char *title)
{
  GetConsoleTitle(OldTitle,512);
//  _SVS(SysLog(1,"ConsoleTitle> '%s'",OldTitle));
  if(title)
    SetFarTitle(title);
}

ConsoleTitle::~ConsoleTitle()
{
  char *Ptr=OldTitle+strlen(OldTitle)-strlen(FarTitleAddons);
  if(!stricmp(Ptr,FarTitleAddons))
    *Ptr=0;
  SetFarTitle(OldTitle);
//  _SVS(SysLog(-1,"~ConsoleTitle '%s'",OldTitle));
}

void ConsoleTitle::Set(char *fmt,...)
{
  char msg[2048];

  va_list argptr;
  va_start( argptr, fmt );

  vsprintf( msg, fmt, argptr );
  va_end(argptr);
  SetFarTitle(msg);
}
