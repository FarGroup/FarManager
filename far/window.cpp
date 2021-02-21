/*
window.cpp

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "window.hpp"

// Internal:
#include "keybar.hpp"
#include "manager.hpp"
#include "savescr.hpp"
#include "global.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

static int windowID=0;

window::window():
	ScreenObjectWithShadow(nullptr),
	m_ID(windowID++)
{
}

window::~window()
{
}

void window::UpdateKeyBar() const
{
	if (m_windowKeyBar && IsKeyBarVisible())
		m_windowKeyBar->RedrawIfChanged();
}

bool window::IsTopWindow() const
{
	return Global->WindowManager->GetCurrentWindow().get() == this;
}

void window::OnChangeFocus(bool focus)
{
}

void window::Refresh()
{
	if (m_Flags.Check(FSCROBJ_ENABLERESTORESCREEN) && SaveScr) SaveScr->Discard();
	if (ShadowSaveScr) ShadowSaveScr->Discard();
	Hide();
	Show();
}

bool window::CanFastHide() const
{
	return true;
}

bool window::HasSaveScreen() const
{
	return SaveScr || ShadowSaveScr;
}

void window::SetDeleting()
{
	m_Deleting=true;
}

bool window::IsDeleting() const
{
	return m_Deleting;
}

void window::Pin()
{
	++m_BlockCounter;
}

void window::UnPin()
{
	assert(m_BlockCounter>0);
	--m_BlockCounter;
}

bool window::IsPinned() const
{
	return m_BlockCounter>0;
}

void window::SetMacroMode(FARMACROAREA Area)
{
	m_MacroArea=Area;
}
