#ifndef SAVESCR_HPP_778D9931_B748_4300_B9AF_330A1B2C80B9
#define SAVESCR_HPP_778D9931_B748_4300_B9AF_330A1B2C80B9
#pragma once

/*
savescr.hpp

Сохраняем и восстанавливаем экран кусками и целиком
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


// Internal:
#include "plugin.hpp"

// Platform:

// Common:
#include "common/2d/matrix.hpp"
#include "common/2d/rectangle.hpp"
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

class SaveScreen: noncopyable
{
public:
	SaveScreen();
	explicit SaveScreen(rectangle Where);
	~SaveScreen();

	void SaveArea(rectangle Where);
	void SaveArea();
	void RestoreArea(int RestoreCursor = TRUE);
	void Discard();
	void AppendArea(const SaveScreen *NewArea);
	void Resize(int DesiredWidth, int DesiredHeight, bool SyncWithConsole);
	void DumpBuffer(const wchar_t *Title);

private:
	friend class Grabber;

	int width() const { return m_Where.width(); }
	int height() const { return m_Where.height(); }

	matrix<FAR_CHAR_INFO> ScreenBuf;
	point m_Cursor;
	bool CurVisible;
	size_t CurSize;
	rectangle m_Where;
};

#endif // SAVESCR_HPP_778D9931_B748_4300_B9AF_330A1B2C80B9
