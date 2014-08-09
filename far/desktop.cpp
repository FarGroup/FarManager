/*
desktop.cpp


*/
/*
Copyright © 2014 Far Group
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

#include "desktop.hpp"

#include "global.hpp"
#include "manager.hpp"
#include "savescr.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "keys.hpp"
#include "scrbuf.hpp"
#include "help.hpp"

desktop::desktop()
{
	SetCanLoseFocus(TRUE);
	SetPosition(0, 0, ScrX, ScrY);
}

desktop::~desktop()
{
}

void desktop::ResizeConsole()
{
	m_Background->Resize(ScrX + 1, ScrY + 1, 2, Global->Opt->WindowMode != FALSE);
	SetPosition(0, 0, ScrX, ScrY);
}

void desktop::DisplayObject()
{
	m_Background->RestoreArea();
}

int desktop::ProcessKey(const Manager::Key& Key)
{
	switch (Key.FarKey)
	{
	case KEY_F1:
		{
			Help hlp(L"Contents");
		}
		break;

	case KEY_SHIFTF9:
		Global->Opt->Save(true);
		return TRUE;

	case KEY_F10:
		Global->FrameManager->ExitMainLoop(TRUE);
		return TRUE;
	}
	return FALSE;
}

void desktop::FillFromBuffer()
{
	if (!m_Background)
		m_Background = std::make_unique<SaveScreen>(0, 0, ScrX, ScrY);
	else
		m_Background->SaveArea();
}

void desktop::FillFromConsole()
{
	Global->ScrBuf->FillBuf();

	if (!m_Background)
		m_Background = std::make_unique<SaveScreen>(0, 0, ScrX, ScrY);
	else
		m_Background->SaveArea();
}
