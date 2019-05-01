#ifndef REFRESHWINDOWMANAGER_HPP_D95D197C_2FDC_4A4E_9110_0AFAA3ABCDAB
#define REFRESHWINDOWMANAGER_HPP_D95D197C_2FDC_4A4E_9110_0AFAA3ABCDAB
#pragma once

/*
refreshwindowmanager.hpp

Класс для рефрешки
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

// Platform:

// Common:
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

class SaveScreen;

class UndoGlobalSaveScrPtr: noncopyable
{
public:
	explicit UndoGlobalSaveScrPtr(SaveScreen *SaveScr);
	~UndoGlobalSaveScrPtr();
};

class RefreshWindowManager: noncopyable
{
public:
	RefreshWindowManager(int OScrX,int OScrY, bool Force = false);
	~RefreshWindowManager();

private:
	int OScrX,OScrY;
	bool m_Force;
};

#endif // REFRESHWINDOWMANAGER_HPP_D95D197C_2FDC_4A4E_9110_0AFAA3ABCDAB
