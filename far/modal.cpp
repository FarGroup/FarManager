/*
modal.cpp

Parent class для модальных объектов
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "modal.hpp"
#include "keys.hpp"
#include "help.hpp"
#include "lockscrn.hpp"
#include "interf.hpp"
#include "keyboard.hpp"

Modal::Modal():
	ReadKey(-1),
	WriteKey(-1),
	ExitCode(-1),
	EndLoop(0)
{
}


void Modal::Process()
{
	Show();

	while (!Done())
	{
		ReadInput();
		ProcessInput();
	}

	GetDialogObjectsData();
}


int Modal::ReadInput(INPUT_RECORD *GetReadRec)
{
	if (GetReadRec)
		ClearStruct(*GetReadRec);

	if (WriteKey>=0)
	{
		ReadKey=WriteKey;
		WriteKey=-1;
	}
	else
	{
		ReadKey=GetInputRecord(&ReadRec);

		if (GetReadRec)
		{
			*GetReadRec=ReadRec;
		}
	}

	if (ReadKey == KEY_CONSOLE_BUFFER_RESIZE)
	{
		LockScreen LckScr;
		Hide();
		Show();
	}

	if (CloseFARMenu)
	{
		SetExitCode(-1);
	}

	return(ReadKey);
}


void Modal::WriteInput(int Key)
{
	WriteKey=Key;
}


void Modal::ProcessInput()
{
	if (ReadRec.EventType==MOUSE_EVENT && !(ReadKey==KEY_MSWHEEL_UP || ReadKey==KEY_MSWHEEL_DOWN || ReadKey==KEY_MSWHEEL_RIGHT || ReadKey==KEY_MSWHEEL_LEFT))
		ProcessMouse(&ReadRec.Event.MouseEvent);
	else
		ProcessKey(ReadKey);
}


int Modal::Done()
{
	return(EndLoop);
}


void Modal::ClearDone()
{
	EndLoop=0;
}


int Modal::GetExitCode()
{
	return(ExitCode);
}


void Modal::SetExitCode(int Code)
{
	ExitCode=Code;
	EndLoop=TRUE;
}


void Modal::SetHelp(const wchar_t *Topic)
{
	strHelpTopic = Topic;
}


void Modal::ShowHelp()
{
	if (!strHelpTopic.IsEmpty())
		Help Hlp(strHelpTopic);
}
