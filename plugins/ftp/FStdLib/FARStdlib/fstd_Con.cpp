#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int WINAPI FP_ConWidth(void)
{
	CONSOLE_SCREEN_BUFFER_INFO ci;
	return GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&ci) ? ci.dwSize.X : 0;
}

int WINAPI FP_ConHeight(void)
{
	CONSOLE_SCREEN_BUFFER_INFO ci;
	return GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&ci) ? ci.dwSize.Y : 0;
}
