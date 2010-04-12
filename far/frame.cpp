/*
frame.cpp

Parent class для немодальных объектов

*/

#include "headers.hpp"
#pragma hdrstop

#include "frame.hpp"
#include "fn.hpp"
#include "keybar.hpp"
#include "manager.hpp"

Frame::Frame()
{
	_OT(SysLog("[%p] Frame::Frame()", this));
	CanLoseFocus=FALSE;
	ExitCode=-1;
	TitleBarVisible=KeyBarVisible=MacroMode=0;
	FrameKeyBar=NULL;
//  ModalStack=NULL;
//  ModalStackCount = ModalStackSize=0;
	DynamicallyBorn=TRUE;
	FrameToBack=NULL;
	NextModal=PrevModal=NULL;
}

Frame::~Frame()
{
	_OT(SysLog("[%p] Frame::~Frame()", this));
	DestroyAllModal();
//  xf_free(ModalStack);
}

void Frame::SetKeyBar(KeyBar *FrameKeyBar)
{
	Frame::FrameKeyBar=FrameKeyBar;
}

void Frame::UpdateKeyBar()
{
	if (FrameKeyBar!=NULL && KeyBarVisible)
		FrameKeyBar->RedrawIfChanged();
}

/* $ 12.05.2001 DJ */
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
			/* $ 28.04.2002 KM
			    Если модальный объект - комбобокс, то
			    не отображаем его.
			*/
			if (iModal->GetType()!=MODALTYPE_COMBOBOX && iModal->IsVisible())
				iModal->Show();

			/* KM $ */
			iModal=iModal->NextModal;
		}
	}
	else
	{
		Hide();
	}
}
/* DJ $ */

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
  Frame *Result=NULL;
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
		Prev->NextModal=NULL;
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
		Prev->NextModal=NULL;
		return true;
	}
	else
	{
		return false;
	}
}

/* $ 13.04.2002 KM */
void Frame::ResizeConsole()
{
	FrameManager->ResizeAllModal(this);
}
/* KM $ */

bool Frame::HasSaveScreen()
{
	if (this->SaveScr||this->ShadowSaveScr)
	{
		return true;
	}

	return false;
}

//bool Frame::ifFullConsole() {
//  return X1==0&&Y1==0&&X2>=ScrX&&Y2>=ScrY-1;
//}
