/*
scrobj.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "scrobj.hpp"
#include "savescr.hpp"
#include "interf.hpp"

ScreenObject *ScreenObject::CaptureMouseObject=nullptr;

ScreenObject::ScreenObject():
	ShadowSaveScr(nullptr),
	X1(0),
	Y1(0),
	X2(0),
	Y2(0),
	ObjWidth(0),
	ObjHeight(0),
	nLockCount(0),
	pOwner(nullptr),
	SaveScr(nullptr)
{
//  _OT(SysLog(L"[%p] ScreenObject::ScreenObject()", this));
}


ScreenObject::~ScreenObject()
{
//  _OT(SysLog(L"[%p] ScreenObject::~ScreenObject()", this));
	if (!Flags.Check(FSCROBJ_ENABLERESTORESCREEN))
	{
		if (ShadowSaveScr)
			ShadowSaveScr->Discard();

		if (SaveScr)
			SaveScr->Discard();
	}

	if (ShadowSaveScr)
		delete ShadowSaveScr;

	if (SaveScr)
		delete SaveScr;
}

void ScreenObject::Lock()
{
	nLockCount++;
}

void ScreenObject::Unlock()
{
	if (nLockCount > 0)
		nLockCount--;
	else
		nLockCount = 0;
}

bool ScreenObject::Locked()
{
	return (nLockCount > 0) || (pOwner?pOwner->Locked():false);
}

void ScreenObject::SetOwner(ScreenObject *pOwner)
{
	ScreenObject::pOwner = pOwner;
}

ScreenObject* ScreenObject::GetOwner()
{
	return pOwner;
}

void ScreenObject::SetPosition(int X1,int Y1,int X2,int Y2)
{
	/* $ 13.04.2002 KM
	  - Раз меняем позицию объекта на экране, то тогда
	    перед этим восстановим изображение под ним для
	    предотвращения восстановления ранее сохранённого
	    изображения в новом месте.
	*/
	if (SaveScr)
	{
		delete SaveScr;
		SaveScr=nullptr;
	}

	ScreenObject::X1=X1;
	ScreenObject::Y1=Y1;
	ScreenObject::X2=X2;
	ScreenObject::Y2=Y2;
	ObjWidth=X2-X1+1;
	ObjHeight=Y2-Y1+1;
	Flags.Set(FSCROBJ_SETPOSITIONDONE);
}

void ScreenObject::SetScreenPosition()
{
	Flags.Clear(FSCROBJ_SETPOSITIONDONE);
}


void ScreenObject::GetPosition(int& X1,int& Y1,int& X2,int& Y2)
{
	X1=ScreenObject::X1;
	Y1=ScreenObject::Y1;
	X2=ScreenObject::X2;
	Y2=ScreenObject::Y2;
}


void ScreenObject::Hide()
{
//  _tran(SysLog(L"[%p] ScreenObject::Hide()",this));
	if (!Flags.Check(FSCROBJ_VISIBLE))
		return;

	Flags.Clear(FSCROBJ_VISIBLE);

	if (ShadowSaveScr)
	{
		delete ShadowSaveScr;
		ShadowSaveScr=nullptr;
	}

	if (SaveScr)
	{
		delete SaveScr;
		SaveScr=nullptr;
	}
}

/* $ 15.07.2000 tran
   add ugly new method */
void ScreenObject::Hide0()
{
	Flags.Clear(FSCROBJ_VISIBLE);
}
/* tran 15.07.2000 $ */

void ScreenObject::Show()
{
	if (Locked())
		return;

//  _tran(SysLog(L"[%p] ScreenObject::Show()",this));
	if (!Flags.Check(FSCROBJ_SETPOSITIONDONE))
		return;

//  if (Flags.Check(FSCROBJ_ISREDRAWING))
//    return;
//  Flags.Set(FSCROBJ_ISREDRAWING);
	SavePrevScreen();
	DisplayObject();
//  Flags.Clear(FSCROBJ_ISREDRAWING);
}


void ScreenObject::SavePrevScreen()
{
	if (!Flags.Check(FSCROBJ_SETPOSITIONDONE))
		return;

	if (!Flags.Check(FSCROBJ_VISIBLE))
	{
		Flags.Set(FSCROBJ_VISIBLE);

		if (Flags.Check(FSCROBJ_ENABLERESTORESCREEN) && !SaveScr)
			SaveScr=new SaveScreen(X1,Y1,X2,Y2);
	}
}


void ScreenObject::Redraw()
{
//  _tran(SysLog(L"[%p] ScreenObject::Redraw()",this));
	if (Flags.Check(FSCROBJ_VISIBLE))
		Show();
}


void ScreenObject::Shadow(bool Full)
{
	if (Flags.Check(FSCROBJ_VISIBLE))
	{
		if(Full)
		{
			if (!ShadowSaveScr)
				ShadowSaveScr=new SaveScreen(0,0,ScrX,ScrY);

			MakeShadow(0,0,ScrX,ScrY);
		}
		else
		{
			if (!ShadowSaveScr)
				ShadowSaveScr=new SaveScreen(X1,Y1,X2+2,Y2+1);

			MakeShadow(X1+2,Y2+1,X2+1,Y2+1);
			MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
		}
	}
}

void ScreenObject::SetCapture(ScreenObject *Obj)
{
	ScreenObject::CaptureMouseObject=Obj;
}
