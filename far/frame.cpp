/*
frame.cpp

Parent class для немодальных объектов

*/

/* Revision: 1.04 07.05.2001 $ */

/*
Modify:
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

Frame::Frame()
{
  CanLoseFocus=FALSE;
  ExitCode=-1;
  KeyBarVisible=MacroMode=0;
  ModalKeyBar=NULL;
}

void Frame::SetKeyBar(KeyBar *ModalKeyBar)
{
  Frame::ModalKeyBar=ModalKeyBar;
}

void Frame::UpdateKeyBar()
{
    _D(SysLog("Frame::UpdateKeyBar(), ModalKeyBar=0x%p",ModalKeyBar));
    if ( ModalKeyBar!=NULL && KeyBarVisible )
        ModalKeyBar->RedrawIfChanged();
}
