/*
frame.cpp

Parent class для немодальных объектов

*/

/* Revision: 1.01 07.05.2001 $ */

/*
Modify:
  07.05.2001 OT
    - В конструкторе инициализировались не все члены. Из-за этого фар трапался :(
  05.05.2001 DJ
    + created
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

Frame::Frame()
{
  EnableSwitch=FALSE;
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
    SysLog("Frame::UpdateKeyBar(), ModalKeyBar=0x%p",ModalKeyBar);
    if ( ModalKeyBar!=NULL && KeyBarVisible )
        ModalKeyBar->RedrawIfChanged();
}
