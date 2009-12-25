/*
iswind.cpp

Проверка fullscreen/windowed
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "iswind.hpp"
#include "imports.hpp"
#include "config.hpp"

static BOOL WindowedMode=FALSE;
static HICON hOldLargeIcon=NULL, hOldSmallIcon=NULL;

void DetectWindowedMode()
{
	HWND hWnd = GetConsoleWindow();
	if (hWnd)
		WindowedMode=!IsIconic(hWnd);
}

void InitDetectWindowedMode()
{
	HWND hWnd = GetConsoleWindow();

	if (hWnd && Opt.SmallIcon)
	{
		string strFarName;
		apiGetModuleFileName(NULL, strFarName);
		HICON hSmallIcon=NULL,hLargeIcon=NULL;
		ExtractIconEx(strFarName,0,&hLargeIcon,&hSmallIcon,1);

		if (hLargeIcon!=NULL)
			hOldLargeIcon=(HICON)SendMessage(hWnd,WM_SETICON,1,(LPARAM)hLargeIcon);

		if (hSmallIcon!=NULL)
			hOldSmallIcon=(HICON)SendMessage(hWnd,WM_SETICON,0,(LPARAM)hSmallIcon);
	}

	DetectWindowedMode();
}


int IsWindowed()
{
	return WindowedMode;
}

void RestoreIcons()
{
	HWND hWnd = GetConsoleWindow();

	if (hWnd && Opt.SmallIcon)
	{
		if (hOldLargeIcon!=NULL)
		{
			SendMessage(hWnd,WM_SETICON,1,(LPARAM)hOldLargeIcon);
			SendMessage(hWnd,WM_SETICON,0,(LPARAM)(hOldSmallIcon!=NULL ? hOldSmallIcon:hOldLargeIcon));
		}
	}
}

/* $ 25.07.2000 SVS
   Программое переключение FulScreen <-> Windowed
   (с подачи "Vasily V. Moshninov" <vmoshninov@newmail.ru>)
   mode = -2 - получить текущее состояние
          -1 - как тригер
           0 - Windowed
           1 - FulScreen
   Return
           0 - Windowed
           1 - FulScreen
*/
int FarAltEnter(int mode)
{
	if (mode != FAR_CONSOLE_GET_MODE)
	{
		COORD dwOldMode;

		if (!ifn.pfnSetConsoleDisplayMode)
			ifn.pfnSetConsoleDisplayMode(
			    GetStdHandle(STD_OUTPUT_HANDLE),
			    (mode == FAR_CONSOLE_TRIGGER)?(IsWindowed()?FAR_CONSOLE_SET_FULLSCREEN:FAR_CONSOLE_SET_WINDOWED):(mode&1),
					    &dwOldMode
					);
	}

	DetectWindowedMode();
	return IsWindowed()?FAR_CONSOLE_WINDOWED:FAR_CONSOLE_FULLSCREEN;
}
