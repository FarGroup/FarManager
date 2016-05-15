#ifndef SCROBJ_HPP_1BE17E05_38A9_47FA_B7B0_D362B61BCA38
#define SCROBJ_HPP_1BE17E05_38A9_47FA_B7B0_D362B61BCA38
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
#include "manager.hpp" //Manager::Key

class SaveScreen;

// можно использовать только младший байт (т.е. маска 0x000000FF), остальное отдается порожденным классам
enum
{
	FSCROBJ_VISIBLE              = 0x00000001,
	FSCROBJ_ENABLERESTORESCREEN  = 0x00000002,
	FSCROBJ_SETPOSITIONDONE      = 0x00000004,
	FSCROBJ_ISREDRAWING          = 0x00000008,   // идет процесс Show?
};

class SimpleScreenObject: noncopyable
{
public:
	SimpleScreenObject(window_ptr Owner);
	virtual ~SimpleScreenObject();

	int ObjWidth() const {return m_X2 - m_X1 + 1;}
	int ObjHeight() const {return m_Y2 - m_Y1 + 1;}

	virtual int ProcessKey(const Manager::Key& Key) { return 0; }
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) { return 0; }

	virtual void Hide();
	virtual void Show();
	virtual void ShowConsoleTitle() {};
	virtual void SetPosition(int X1,int Y1,int X2,int Y2);
	virtual void GetPosition(int& X1,int& Y1,int& X2,int& Y2) const;
	virtual void SetScreenPosition();
	virtual void ResizeConsole() {};
	virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) {return 0;}
	virtual void Refresh(void);

	void Redraw();
	bool IsVisible() const {return m_Flags.Check(FSCROBJ_VISIBLE);}
	void SetVisible(bool Visible) {m_Flags.Change(FSCROBJ_VISIBLE,Visible);}
	void SetRestoreScreenMode(bool Mode) {m_Flags.Change(FSCROBJ_ENABLERESTORESCREEN,Mode);}
	window_ptr GetOwner() const {return m_Owner.lock();}

private:
	virtual void DisplayObject() = 0;

protected:
	// KEEP ALIGNED!
	std::weak_ptr<window> m_Owner;
	BitFlags m_Flags;
	SHORT m_X1, m_Y1, m_X2, m_Y2;
};

class ScreenObject:public SimpleScreenObject
{
public:
	virtual void SetPosition(int X1, int Y1, int X2, int Y2) override;
	virtual void Show() override;
	virtual void Hide() override;

	void HideButKeepSaveScreen();

protected:
	ScreenObject(window_ptr Owner);
	virtual ~ScreenObject();

public: // BUGBUG
	std::unique_ptr<SaveScreen> SaveScr;
};


class ScreenObjectWithShadow:public ScreenObject
{
public:
	virtual void Hide() override;

protected:
	ScreenObjectWithShadow(window_ptr Owner);
	virtual ~ScreenObjectWithShadow();

	void Shadow(bool Full=false);

	std::unique_ptr<SaveScreen> ShadowSaveScr;
};

#endif // SCROBJ_HPP_1BE17E05_38A9_47FA_B7B0_D362B61BCA38
