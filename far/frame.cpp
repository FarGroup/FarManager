/*
frame.cpp

Parent class для немодальных объектов

*/

/* Revision: 1.08 16.05.2001 $ */

/*
Modify:
  16.05.2001 SVS
    ! _D() -> _OT()
  15.05.2001 OT
    ! NWZ -> NFZ
  12.05.2001 DJ
    + IsTopFrame()
    + отрисовка по OnChangeFocus сделана дефолтным поведением
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  07.05.2001 DJ
    ! причешем идентификаторы
  06.05.2001 DJ
    ! перетрях #include
  07.05.2001 OT
    - В конструкторе инициализировались не все члены. Из-за этого фар трапался :(
  05.05.2001 DJ
    + created
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
  KeyBarVisible=MacroMode=0;
  FrameKeyBar=NULL;
  ModalStack=NULL;
  ModalStackCount = ModalStackSize=0;

}

Frame::~Frame()
{
  _OT(SysLog("[%p] Frame::~Frame()", this));
  DestroyAllModal();
  free(ModalStack);
}

void Frame::SetKeyBar(KeyBar *FrameKeyBar)
{
  Frame::FrameKeyBar=FrameKeyBar;
}

void Frame::UpdateKeyBar()
{
    if ( FrameKeyBar!=NULL && KeyBarVisible )
        FrameKeyBar->RedrawIfChanged();
}

/* $ 12.05.2001 DJ */
int Frame::IsTopFrame()
{
  return FrameManager->GetCurrentFrame() == this;
}

void Frame::OnChangeFocus (int focus)
{

  if (focus) {
    Show();
    for (int i=0;i<ModalStackCount;i++){
      ModalStack[i]->Show();
    }
  }
}
/* DJ $ */

void Frame::Push(Frame* Modalized){
  if (ModalStackCount == ModalStackSize)
    ModalStack = (Frame **) realloc (ModalStack, ++ModalStackSize * sizeof (Frame *));
  ModalStack [ModalStackCount++] = Modalized;
}

bool Frame::Pop(){
  if (ModalStackCount>0){
    ModalStack[--ModalStackCount]->OnDestroy();
    delete ModalStack[ModalStackCount];
    return true;
  } else {
    return false;
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

void Frame::DestroyAllModal()
{
  while(Pop());
//  ModalStackSize=0;
//  ModalStackCount=0;
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