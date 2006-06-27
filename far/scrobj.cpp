/*
scrobj.cpp

Parent class ��� ���� screen objects

*/

/* Revision: 1.13 26.07.2005 $ */

/*
Modify:
  26.07.2005 WARP
    ! see 00033.LoskUnLock.txt
  27.05.2003 SVS
    ! ������� ������������ ���� :-)
      ������ ScreenObject *ScreenObject::CaptureMouseObject, �������
      ��������� �� ������, ����������� ����.
  26.02.2003 SVS
    - BugZ#813 - DM_RESIZEDIALOG � DN_DRAWDIALOG -> ��������
      �������� �������� ����� ���������� �������� �� ����� ������ ������
  25.02.2003 SVS
    ! � ScreenObject::Show() ������������� ������ FSCROBJ_ISREDRAWING � ���,
      ����� ��������� �������� ��� ���������� ��������!
  25.06.2002 SVS
    ! ���������:  BitFlags::Skip -> BitFlags::Clear
  18.05.2002 SVS
    ! ������� ��������� ���������� �� �����
  13.04.2002 KM
    - ��� ������ ������� ������� �� ������, �� ����� ����� ����
      ����������� ����������� ��� ��� ��� ��������������
      �������������� ����� ����������� ����������� � ����� �����.
  18.07.2001 OT
    ���������
  14.06.2001 OT
    ! "����" ;-)
  12.05.2001 SVS
    ! ������� �������� �� ����������� �������
  09.05.2001 OT
    - ��������� � ������������ EnableRestoreScreen=FALSE;
  06.05.2001 DJ
    ! �������� #include
  15.07.2000 tran
    + add new dirty method - Hide0(), jys set Visible to False
      used in FileViewer, for keybar hiding
  25.06.2000 SVS
    ! ���������� Master Copy
    ! ��������� � �������� ���������������� ������
*/

#include "headers.hpp"
#pragma hdrstop

#include "scrobj.hpp"
#include "fn.hpp"
#include "savescr.hpp"

ScreenObject *ScreenObject::CaptureMouseObject=NULL;

ScreenObject::ScreenObject()
{
//  _OT(SysLog("[%p] ScreenObject::ScreenObject()", this));
  ObjWidth=ObjHeight=X1=Y1=X2=Y2=0;
  SaveScr=ShadowSaveScr=NULL;
  nLockCount = 0;
  pOwner = NULL;
}


ScreenObject::~ScreenObject()
{
//  _OT(SysLog("[%p] ScreenObject::~ScreenObject()", this));
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

void ScreenObject::Lock ()
{
  nLockCount++;
}

void ScreenObject::Unlock ()
{
  if ( nLockCount > 0 )
    nLockCount--;
  else
    nLockCount = 0;
}

bool ScreenObject::Locked ()
{
  return  (nLockCount > 0) || (pOwner?pOwner->Locked():false);
}

void ScreenObject::SetOwner (ScreenObject *pOwner)
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
    - ��� ������ ������� ������� �� ������, �� �����
      ����� ���� ����������� ����������� ��� ��� ���
      �������������� �������������� ����� �����������
      ����������� � ����� �����.
  */
  if (SaveScr)
  {
    delete SaveScr;
    SaveScr=NULL;
  }
  /* KM $ */
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
//  _tran(SysLog("[%p] ScreenObject::Hide()",this));
  if (!Flags.Check(FSCROBJ_VISIBLE))
    return;

  Flags.Clear(FSCROBJ_VISIBLE);

  if (ShadowSaveScr)
  {
    delete ShadowSaveScr;
    ShadowSaveScr=NULL;
  }

  if (SaveScr)
  {
    delete SaveScr;
    SaveScr=NULL;
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
  if ( Locked () )
    return;

//  _tran(SysLog("[%p] ScreenObject::Show()",this));
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
    if (Flags.Check(FSCROBJ_ENABLERESTORESCREEN) && SaveScr==NULL)
      SaveScr=new SaveScreen(X1,Y1,X2,Y2);
  }
}


void ScreenObject::Redraw()
{
//  _tran(SysLog("[%p] ScreenObject::Redraw()",this));
  if (Flags.Check(FSCROBJ_VISIBLE))
    Show();
}


void ScreenObject::Shadow()
{
  if (Flags.Check(FSCROBJ_VISIBLE))
  {
    if (ShadowSaveScr==NULL)
      ShadowSaveScr=new SaveScreen(X1,Y1,X2+2,Y2+1);
    MakeShadow(X1+2,Y2+1,X2+1,Y2+1);
    MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
  }
}

void ScreenObject::SetCapture(ScreenObject *Obj)
{
  ScreenObject::CaptureMouseObject=Obj;
}
