/*
foldtree.cpp

Поиск каталога по Alt-F10

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

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

FolderTree::FolderTree(char *ResultFolder,int ModalMode,int TX1,int TY1,int TX2,int TY2)
{
  SetRestoreScreenMode(FALSE);
  SaveScreen SaveScr;
  *ResultFolder=*NewFolder=0;
  SetPosition(TX1,TY1,TX2,TY2);
  if ((Tree=new TreeList)==NULL)
    return;
  {
    *LastName=0;
    Tree->SetModalMode(ModalMode);
    Tree->SetPosition(X1,Y1,X2,Y2);
    Tree->Update(0);
    Tree->Show();
    MakeShadow(X1+2,Y2+1,X2+2,Y2+1);
    MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
    {
      if ((FindEdit=new Edit)==NULL)
      {
        delete Tree;
        return;
      }
      FindEdit->SetEditBeyondEnd(FALSE);
      DrawEdit();
    }
    Process();
    delete FindEdit;
    delete Tree;
  }
  strcpy(ResultFolder,NewFolder);
}


int FolderTree::ProcessKey(int Key)
{
  if (Key>=KEY_ALT_BASE && Key<=KEY_ALT_BASE+255)
    Key=tolower(Key-KEY_ALT_BASE);
  switch(Key)
  {
    case KEY_F1:
      {
        Help Hlp("FindFolder");
      }
      break;
    case KEY_ESC:
    case KEY_F10:
      SetExitCode(1);
      break;
    case KEY_ENTER:
      Tree->GetCurDir(NewFolder);
      if (GetFileAttributes(NewFolder)!=0xFFFFFFFF)
        SetExitCode(1);
      else
      {
        Tree->ProcessKey(KEY_ENTER);
        DrawEdit();
      }
      break;
    case KEY_CTRLR:
    case KEY_F2:
      Tree->ProcessKey(KEY_CTRLR);
      DrawEdit();
      break;
    case KEY_CTRLENTER:
      {
        char Name[NM];
        FindEdit->GetString(Name,sizeof(Name));
        Tree->FindPartName(Name,TRUE);
        DrawEdit();
      }
      break;
    case KEY_UP:
    case KEY_DOWN:
    case KEY_PGUP:
    case KEY_PGDN:
    case KEY_HOME:
    case KEY_END:
      FindEdit->SetString("");
      Tree->ProcessKey(Key);
      DrawEdit();
      break;
    default:
      if (FindEdit->ProcessKey(Key))
      {
        char Name[NM];
        FindEdit->GetString(Name,sizeof(Name));
        if (Tree->FindPartName(Name,FALSE))
          strcpy(LastName,Name);
        else
        {
          FindEdit->SetString(LastName);
          strcpy(Name,LastName);
        }
        DrawEdit();
      }
      break;
  }
  return(TRUE);
}


int FolderTree::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (MouseEvent->dwEventFlags==DOUBLE_CLICK)
  {
    ProcessKey(KEY_ENTER);
    return(TRUE);
  }
  if (MouseEvent->dwButtonState!=1)
    return(TRUE);
  if (!Tree->ProcessMouse(MouseEvent))
    SetExitCode(1);
  else
    DrawEdit();
  return(TRUE);
}


void FolderTree::DrawEdit()
{
  int FindY=Y2-2;
  char *SearchTxt=MSG(MFoldTreeSearch);
  GotoXY(X1+1,FindY);
  SetColor(COL_PANELTEXT);
  mprintf("%s  ",SearchTxt);
  FindEdit->SetPosition(X1+strlen(SearchTxt)+2,FindY,min(X2-1,X1+25),FindY);
  FindEdit->SetObjectColor(COL_DIALOGEDIT);
  FindEdit->Show();
  if (WhereX()<X2)
  {
    SetColor(COL_PANELTEXT);
    mprintf("%*s",X2-WhereX(),"");
  }
}
