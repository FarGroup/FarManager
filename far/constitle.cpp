/*
constitle.cpp

Заголовок консоли

*/

#include "headers.hpp"
#pragma hdrstop

#include "constitle.hpp"
#include "global.hpp"
#include "fn.hpp"

ConsoleTitle::ConsoleTitle(char *title)
{
	GetConsoleTitle(OldTitle,512);

	if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
		FAR_CharToOem(OldTitle,OldTitle);

//  _SVS(SysLog(1,"ConsoleTitle> '%s'",OldTitle));
	if (title)
		SetFarTitle(title);
}

ConsoleTitle::~ConsoleTitle()
{
	char *Ptr=OldTitle+strlen(OldTitle)-strlen(FarTitleAddons);

	if (!stricmp(Ptr,FarTitleAddons))
		*Ptr=0;

	SetFarTitle(OldTitle);
//  _SVS(SysLog(-1,"~ConsoleTitle '%s'",OldTitle));
}

void ConsoleTitle::Set(char *fmt,...)
{
	char msg[2048];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(msg, sizeof(msg)-1, fmt, argptr);
	va_end(argptr);
	SetFarTitle(msg);
}
