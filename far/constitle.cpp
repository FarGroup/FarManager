/*
constitle.cpp

Заголовок консоли

*/

#include "headers.hpp"
#pragma hdrstop

#include "constitle.hpp"
#include "global.hpp"
#include "fn.hpp"

ConsoleTitle::ConsoleTitle(const wchar_t *title)
{
  apiGetConsoleTitle (strOldTitle);

  if( title )
    SetFarTitle(title);

}

ConsoleTitle::~ConsoleTitle()
{
    wchar_t *lpwszTitle = strOldTitle.GetBuffer ();

    if ( *lpwszTitle )
    {
        lpwszTitle += wcslen (lpwszTitle);
        lpwszTitle -= wcslen (FarTitleAddons);

        if ( !LocalStricmpW (lpwszTitle, FarTitleAddons) )
            *lpwszTitle = 0;
    }

    strOldTitle.ReleaseBuffer ();

    SetFarTitle(strOldTitle);
}

void ConsoleTitle::Set(const wchar_t *fmt,...)
{
  wchar_t msg[2048];

  va_list argptr;
  va_start( argptr, fmt );

  vsnwprintf( msg, countof(msg)-1, fmt, argptr );
  va_end(argptr);
  SetFarTitle(msg);
}
