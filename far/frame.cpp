/*
frame.cpp

Parent class для немодальных объектов

*/

/* Revision: 1.17 22.09.2001 $ */

/*
Modify:
  22.09.2001 OT
    Вызов Viewer и Editor из меню плагина засовывает куда-то в background window
  30.07.2001 OT
    - Очередное исправление отрисовки меню
  26.07.2001 OT
    ! Исправление отрисовки меню
  23.07.2001 OT
    ! Исправление отрисовки меню в новом MA в макросах
  18.07.2001 OT
    ! VFMenu
  11.07.2001 OT
    ! Перенос CtrlAltShift в Manager
  23.06.2001 OT
    - Решение проблемы "старика Мюллера"
  20.06.2001 tran
    * Refresh* внес в cpp из hpp - удобней отлаживать.
  26.05.2001 OT
    + Новый атрибут - DynamicallyBorn - показывает, статически или динамически был создан объект
    + Возможность блокировки перерисовки фрейма: LockRefreshCount, LockRefresh(),UnlockRefresh(),Refreshable()
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

Frame::Frame():mbtype(MBT_DEFAULT)
{
  _OT(SysLog("[%p] Frame::Frame()", this));
  CanLoseFocus=FALSE;
  ExitCode=-1;
  KeyBarVisible=MacroMode=0;
  FrameKeyBar=NULL;
//  ModalStack=NULL;
//  ModalStackCount = ModalStackSize=0;
  DynamicallyBorn=TRUE;
  LockRefreshCount=0;
  FrameToBack=NULL;
  NextModal=PrevModal=NULL;
}

Frame::~Frame()
{
  _OT(SysLog("[%p] Frame::~Frame()", this));
  DestroyAllModal();
//  free(ModalStack);
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
    Frame *iModal=NextModal;
    while (iModal) {
      if (!iModal->IsVisible()){
        break;
      }
      iModal->Show();
      iModal=iModal->NextModal;
    }
  } else {
    Hide();
  }
}
/* DJ $ */

void Frame::Push(Frame* Modalized){
  if (!NextModal){
    NextModal=Modalized;
    NextModal->PrevModal=this;
  } else {
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
  while (NextModal){
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

void Frame::LockRefresh()
{
  LockRefreshCount++;
}

void Frame::UnlockRefresh()
{
  LockRefreshCount=(LockRefreshCount>0)?LockRefreshCount-1:0;
}


int Frame::Refreshable()
{
  return LockRefreshCount==0;
}


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
  if (!aFrame) {
    return false;
  }
  Frame *Prev=this;
  Frame *Next=NextModal;
  bool fFound=false;
  while (Next){
    if (Next==aFrame){
      fFound=true;
      break;
    }
    Prev=Next;
    Next=Next->NextModal;
  }
  if (fFound){
    RemoveModal(Next->NextModal);
    Prev->NextModal=NULL;
    return true;
  } else {
    return false;
  }
}

void Frame::ResizeConsole()
{
  if (NextModal){
    NextModal->ResizeConsole();
  }
}

bool Frame::HasSaveScreen()
{
  if (this->SaveScr||this->ShadowSaveScr){
    return true;
  }
  return false;
}

void Frame::SetBackFrame(Frame *BackFrame)
{
  FrameToBack=BackFrame;
}
