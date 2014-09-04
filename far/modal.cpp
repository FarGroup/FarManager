/*
modal.cpp

привет автодетектор кодировки!
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

SimpleModal::SimpleModal():
	m_EndLoop(false),
	m_ReadKey(-1),
	m_WriteKey(-1)
{
}


void SimpleModal::Process()
{
	Global->WindowManager->ExecuteWindow(this);
	Global->WindowManager->ExecuteModal(this);
}

int SimpleModal::ReadInput(INPUT_RECORD *GetReadRec)
{
	if (GetReadRec)
		ClearStruct(*GetReadRec);

	if (m_WriteKey>=0)
	{
		m_ReadKey=m_WriteKey;
		m_WriteKey=-1;
	}
	else
	{
		m_ReadKey=GetInputRecord(&m_ReadRec);

		if (GetReadRec)
		{
			*GetReadRec=m_ReadRec;
		}
	}

	if (m_ReadKey == KEY_CONSOLE_BUFFER_RESIZE)
	{
		LockScreen LckScr;
		Hide();
		Show();
	}

	if (Global->CloseFARMenu)
	{
		SetExitCode(-1);
	}

	return m_ReadKey;
}

void SimpleModal::WriteInput(int Key)
{
	m_WriteKey=Key;
}

void SimpleModal::ProcessInput()
{
	if (m_ReadRec.EventType==MOUSE_EVENT && !(m_ReadKey==KEY_MSWHEEL_UP || m_ReadKey==KEY_MSWHEEL_DOWN || m_ReadKey==KEY_MSWHEEL_RIGHT || m_ReadKey==KEY_MSWHEEL_LEFT))
		ProcessMouse(&m_ReadRec.Event.MouseEvent);
	else
		ProcessKey(Manager::Key(m_ReadKey));
}

bool SimpleModal::Done() const
{
	return m_EndLoop;
}


void SimpleModal::ClearDone()
{
	m_EndLoop=false;
}

void SimpleModal::SetDone(void)
{
	m_EndLoop=true;
}

int SimpleModal::GetExitCode() const
{
	return m_ExitCode;
}


void SimpleModal::SetExitCode(int Code)
{
	m_ExitCode=Code;
	SetDone();
}

void SimpleModal::Close(int Code)
{
	SetExitCode(Code);
	Hide();
	Global->WindowManager->DeleteWindow(this);
}

void SimpleModal::SetHelp(const wchar_t *Topic)
{
	m_HelpTopic = Topic;
}


void SimpleModal::ShowHelp()
{
	if (!m_HelpTopic.empty())
		Help Hlp(m_HelpTopic);
}
