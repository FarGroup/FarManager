/*
RefreshFrameManager.cpp

Класс для решрешки

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "lockscrn.hpp"
#include "frame.hpp"
#include "manager.hpp"
#include "savescr.hpp"
#include "RefreshFrameManager.hpp"

UndoGlobalSaveScrPtr::UndoGlobalSaveScrPtr(SaveScreen *SaveScr)
{
	GlobalSaveScrPtr=SaveScr;
}

UndoGlobalSaveScrPtr::~UndoGlobalSaveScrPtr()
{
	GlobalSaveScrPtr=NULL;
}


RefreshFrameManager::RefreshFrameManager(int OScrX,int OScrY, int MsgWaitTime, BOOL DontRedrawFrame)
{
	RefreshFrameManager::OScrX=OScrX;
	RefreshFrameManager::OScrY=OScrY;
	RefreshFrameManager::MsgWaitTime=MsgWaitTime;
	RefreshFrameManager::DontRedrawFrame=DontRedrawFrame;
}
RefreshFrameManager::~RefreshFrameManager()
{
	if (DontRedrawFrame || !FrameManager || !FrameManager->ManagerStarted())
		return;
	else if (OScrX != ScrX || OScrY != ScrY || MsgWaitTime!=0xffffffff)
	{
		LockScreen LckScr;
		FrameManager->ResizeAllFrame();
		FrameManager->GetCurrentFrame()->Show();
	}
}
