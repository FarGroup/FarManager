#pragma once

/*
hmenu.hpp

Горизонтальное меню
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

#include "modal.hpp"
#include "frame.hpp"
#include "CriticalSections.hpp"

struct HMenuData
{
	const wchar_t *Name;
	const wchar_t *SubMenuHelp;
	struct MenuDataEx *SubMenu;
	int SubMenuSize;
	int Selected;
};

class VMenu;

class HMenu: public Modal
{
	private:
		VMenu *SubMenu;
		struct HMenuData *Item;
		int SelectPos;
		int ItemCount;
		int VExitCode;
		int ItemX[16];
		CriticalSection CS;

	private:
		virtual void DisplayObject();
		void ShowMenu();
		void ProcessSubMenu(struct MenuDataEx *Data,int DataCount,const wchar_t *SubMenuHelp,
		                    int X,int Y,int &Position);
		wchar_t GetHighlights(const struct HMenuData *_item);
		int CheckHighlights(WORD CheckSymbol,int StartPos=0);

	public:
		HMenu(struct HMenuData *Item,int ItemCount);
		virtual ~HMenu();

	public:
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
#ifdef FAR_LUA
#else
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0);
#endif

		virtual void Process();
		virtual void ResizeConsole();

		void GetExitCode(int &ExitCode,int &VExitCode);
};
