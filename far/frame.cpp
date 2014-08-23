/*
frame.cpp

Parent class для немодальных объектов
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

#include "frame.hpp"
#include "keybar.hpp"
#include "manager.hpp"
#include "syslog.hpp"

Frame::Frame():
	m_DynamicallyBorn(true),
	m_CanLoseFocus(FALSE),
	m_ExitCode(-1),
	m_KeyBarVisible(0),
	m_TitleBarVisible(0),
	m_FrameKeyBar(nullptr),
	m_FrameToBack(nullptr),
	m_Deleting(false),
	m_BlockCounter(0),
	m_MacroMode(MACROAREA_OTHER)
{
	_OT(SysLog(L"[%p] Frame::Frame()", this));
}

Frame::~Frame()
{
	_OT(SysLog(L"[%p] Frame::~Frame()", this));
}

void Frame::SetKeyBar(KeyBar *FrameKeyBar)
{
	m_FrameKeyBar=FrameKeyBar;
}

void Frame::UpdateKeyBar()
{
	if (m_FrameKeyBar && m_KeyBarVisible)
		m_FrameKeyBar->RedrawIfChanged();
}

int Frame::IsTopFrame() const
{
	return Global->FrameManager->GetCurrentFrame() == this;
}

void Frame::OnChangeFocus(int focus)
{
	if (focus)
	{
		Show();
	}
	else
	{
		Hide();
	}
}

bool Frame::CanFastHide() const
{
	return true;
}

bool Frame::HasSaveScreen() const
{
	if (this->SaveScr||this->ShadowSaveScr)
	{
		return true;
	}

	return false;
}

void Frame::SetDeleting(void)
{
	m_Deleting=true;
}

bool Frame::IsDeleting(void) const
{
	return m_Deleting;
}

void Frame::SetBlock(void)
{
	++m_BlockCounter;
}

void Frame::RemoveBlock(void)
{
	assert(m_BlockCounter>0);
	--m_BlockCounter;
}

bool Frame::IsBlocked(void) const
{
	return m_BlockCounter>0;
}

void Frame::SetMacroMode(FARMACROAREA Area)
{
	m_MacroMode=Area;
}
