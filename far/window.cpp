/*
window.cpp

Parent class для немодальных объектов

*/

#include "headers.hpp"
#pragma hdrstop

#include "internalheaders.hpp"

Frame::Frame()
{
  EnableSwitch=FALSE;
  ExitCode=-1;
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
