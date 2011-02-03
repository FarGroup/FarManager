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
	FrameToBack(nullptr),
	NextModal(nullptr),
	PrevModal(nullptr),
	DynamicallyBorn(TRUE),
	CanLoseFocus(FALSE),
	ExitCode(-1),
	KeyBarVisible(0),
	TitleBarVisible(0),
	FrameKeyBar(nullptr),
	MacroMode(0)
{
	_OT(SysLog(L"[%p] Frame::Frame()", this));
}

Frame::~Frame()
{
	_OT(SysLog(L"[%p] Frame::~Frame()", this));
	DestroyAllModal();
//  xf_free(ModalStack);
}

void Frame::SetKeyBar(KeyBar *FrameKeyBar)
{
	Frame::FrameKeyBar=FrameKeyBar;
}

void Frame::UpdateKeyBar()
{
	if (FrameKeyBar && KeyBarVisible)
		FrameKeyBar->RedrawIfChanged();
}

int Frame::IsTopFrame()
{
	return FrameManager->GetCurrentFrame() == this;
}

void Frame::OnChangeFocus(int focus)
{
	if (focus)
	{
		Show();
		Frame *iModal=NextModal;

		while (iModal)
		{
			if (iModal->GetType()!=MODALTYPE_COMBOBOX && iModal->IsVisible())
				iModal->Show();

			iModal=iModal->NextModal;
		}
	}
	else
	{
		Hide();
	}
}

void Frame::Push(Frame* Modalized)
{
	if (!NextModal)
	{
		NextModal=Modalized;
		NextModal->PrevModal=this;
	}
	else
	{
		NextModal->Push(Modalized);
	}
}

/*
bool Frame::Pop(){
  if (!NextModal) {
    return false;
  }
  while (NextFrame->Pop()){
    NextFrame->Pop();
    return true;
  }
}

Frame *Frame::operator[](int Index)
{
  Frame *Result=nullptr;
  if (Index>=0 && Index<ModalStackSize){
    Result=ModalStack[Index];
  }
  return Result;
}

int Frame::operator[](Frame *ModalFrame)
{
  int Result=-1;
  for (int i=0;i<ModalStackSize;i++){
    if (ModalStack[i]==ModalFrame){
      Result=i;
      break;
    }
  }
  return Result;
}
*/

void Frame::DestroyAllModal()
{
	// найти вершину
	Frame *Prev=this;
	Frame *Next=NextModal;

	while (NextModal)
	{
		Prev->NextModal=nullptr;
		Prev=Next;
		Next=Next->NextModal;
//    if (GetDynamicallyBorn())
	}
}

/*
int Frame::ProcessKey(int Key)
{
  if (ModalSize()){
    return (ModalStack[ModalStackSize-1])->ProcessKey(Key);
  }
  return FALSE;
}

int Frame::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  return FALSE;
}
*/

int Frame::FastHide()
{
	return TRUE;
}

void Frame::OnDestroy()
{
	DestroyAllModal();
}


bool Frame::RemoveModal(Frame *aFrame)
{
	if (!aFrame)
	{
		return false;
	}

	Frame *Prev=this;
	Frame *Next=NextModal;
	bool fFound=false;

	while (Next)
	{
		if (Next==aFrame)
		{
			fFound=true;
			break;
		}

		Prev=Next;
		Next=Next->NextModal;
	}

	if (fFound)
	{
		RemoveModal(Next->NextModal);
		Prev->NextModal=nullptr;
		return true;
	}
	else
	{
		return false;
	}
}

void Frame::ResizeConsole()
{
	FrameManager->ResizeAllModal(this);
}

bool Frame::HasSaveScreen()
{
	if (this->SaveScr||this->ShadowSaveScr)
	{
		return true;
	}

	return false;
}

//bool Frame::ifFullConsole() {
//  return !X1 && !Y1 && X2>=ScrX && Y2>=ScrY-1;
//}
