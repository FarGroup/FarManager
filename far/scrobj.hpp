#pragma once

/*
scrobj.hpp

Parent class для всех screen objects
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

#include "bitflags.hpp"

class SaveScreen;

// можно использовать только младший байт (т.е. маска 0x000000FF), остальное отдается порожденным классам
enum
{
	FSCROBJ_VISIBLE              = 0x00000001,
	FSCROBJ_ENABLERESTORESCREEN  = 0x00000002,
	FSCROBJ_SETPOSITIONDONE      = 0x00000004,
	FSCROBJ_ISREDRAWING          = 0x00000008,   // идет процесс Show?
};

class ScreenObject
{
	protected:
		BitFlags Flags;
		SaveScreen *ShadowSaveScr;
		int X1,Y1,X2,Y2;
		int ObjWidth,ObjHeight;

		int nLockCount;
		ScreenObject *pOwner;

	public:
		SaveScreen *SaveScr;
		static ScreenObject *CaptureMouseObject;

	private:
		virtual void DisplayObject() {};

	public:
		ScreenObject();
		virtual ~ScreenObject();

	public:
		virtual int ProcessKey(int Key) { return 0; };
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent) { return 0; };

		virtual void Hide();
		virtual void Hide0();   // 15.07.2000 tran - dirty hack :(
		virtual void Show();
		virtual void ShowConsoleTitle() {};
		virtual void SetPosition(int X1,int Y1,int X2,int Y2);
		virtual void GetPosition(int& X1,int& Y1,int& X2,int& Y2);
		virtual void SetScreenPosition();
		virtual void ResizeConsole() {};

#ifdef FAR_LUA
#else
		virtual __int64  VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) {return 0;};
#endif

		void Lock();
		void Unlock();
		bool Locked();

		void SetOwner(ScreenObject *pOwner);
		ScreenObject* GetOwner();

		void SavePrevScreen();
		void Redraw();
		int  IsVisible() { return Flags.Check(FSCROBJ_VISIBLE); };
		void SetVisible(int Visible) {Flags.Change(FSCROBJ_VISIBLE,Visible);};
		void SetRestoreScreenMode(int Mode) {Flags.Change(FSCROBJ_ENABLERESTORESCREEN,Mode);};
		void Shadow(bool Full=false);

		static void SetCapture(ScreenObject *Obj);
};
