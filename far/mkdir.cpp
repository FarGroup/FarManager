/*
mkdir.cpp

Создание каталога

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define STRICT

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#include <windows.h>
#endif

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __KEYS_HPP__
#include "keys.hpp"
#endif
#ifndef __FARLANG_HPP__
#include "lang.hpp"
#endif
#ifndef __COLOROS_HPP__
#include "colors.hpp"
#endif
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif
#ifndef __PLUGIN_HPP__
#include "plugin.hpp"
#endif
#ifndef __CLASSES_HPP__
#include "classes.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif
#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif


void ShellMakeDir(Panel *SrcPanel)
{
  char DirName[NM];
  if (!GetString(MSG(MMakeFolderTitle),MSG(MCreateFolder),"NewFolder","",DirName,sizeof(DirName)))
    return;
  Unquote(DirName);
  if (Opt.CreateUppercaseFolders && !IsCaseMixed(DirName))
    LocalStrupr(DirName);

  int Length=strlen(DirName);
  if (Length>0 && (DirName[Length-1]=='/' || DirName[Length-1]=='\\'))
    DirName[Length-1]=0;

  for (char *ChPtr=DirName;*ChPtr!=0;ChPtr++)
    if (*ChPtr=='\\' || *ChPtr=='/')
    {
      *ChPtr=0;
      if (CreateDirectory(DirName,NULL))
        TreeList::AddTreeName(DirName);
      *ChPtr='\\';
    }

  while (!CreateDirectory(DirName,NULL))
  {
    int LastError=GetLastError();
    if (LastError==ERROR_ALREADY_EXISTS || LastError==ERROR_BAD_PATHNAME ||
        LastError==ERROR_INVALID_NAME)
    {
      Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotCreateFolder),DirName,MSG(MOk));
      return;
    }
    else
      if (Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),MSG(MCannotCreateFolder),DirName,MSG(MRetry),MSG(MCancel))!=0)
        return;
  }
  TreeList::AddTreeName(DirName);
  SrcPanel->Update(UPDATE_KEEP_SELECTION);

  char *Slash=strchr(DirName,'\\');
  if (Slash!=NULL)
    *Slash=0;
  SrcPanel->GoToFile(DirName);
  SrcPanel->Redraw();
  Panel *AnotherPanel=CtrlObject->GetAnotherPanel(SrcPanel);
  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  AnotherPanel->Redraw();
}


