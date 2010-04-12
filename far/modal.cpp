/*
modal.cpp

Parent class для модальных объектов

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "modal.hpp"
#include "keys.hpp"
#include "fn.hpp"
#include "help.hpp"
#include "lockscrn.hpp"

Modal::Modal()
{
	ExitCode=-1;
	WriteKey=-1;
	EndLoop=0;
	*HelpTopic=0;
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
		memset(GetReadRec,0,sizeof(INPUT_RECORD));

	if (WriteKey>=0)
	{
		ReadKey=WriteKey;
		WriteKey=-1;
	}
	else
	{
		ReadKey=GetInputRecord(&ReadRec);

		if (GetReadRec)
			memmove(GetReadRec,&ReadRec,sizeof(INPUT_RECORD));
	}

	if (ReadKey == KEY_CONSOLE_BUFFER_RESIZE)
	{
		LockScreen LckScr;
		Hide();
		Show();
	}

	if (CloseFARMenu)
	{
		SetExitCode(TRUE);
	}

	return(ReadKey);
}


void Modal::WriteInput(int Key)
{
	WriteKey=Key;
}


void Modal::ProcessInput()
{
	if (ReadRec.EventType==MOUSE_EVENT)
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


void Modal::SetHelp(const char *Topic)
{
	xstrncpy(HelpTopic,Topic,sizeof(HelpTopic)-1);
}


void Modal::ShowHelp()
{
	if (*HelpTopic)
		Help Hlp(HelpTopic);
}
