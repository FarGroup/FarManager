/*
foldtree.cpp

Поиск каталога по Alt-F10

*/

/* Revision: 1.06 06.06.2001 $ */

/*
Modify:
  06.06.2001 SVS
    ! Mix/Max
  16.05.2001 DJ
    ! proof-of-concept
  15.05.2001 OT
    ! NWZ -> NFZ
  06.05.2001 DJ
    ! перетрях #include
  25.04.2001 SVS
    + Обработка MODALTREE_FREE
  09.01.2001 SVS
    - Для KEY_XXX_BASE нужно прибавить 0x01
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "foldtree.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "treelist.hpp"
#include "edit.hpp"
#include "help.hpp"
#include "savescr.hpp"

FolderTree::FolderTree(char *ResultFolder,int ModalMode,int TX1,int TY1,int TX2,int TY2)
{
  SetRestoreScreenMode(FALSE);
  SaveScreen SaveScr;
  if(ModalMode != MODALTREE_FREE)
    *ResultFolder=0;
  *NewFolder=0;
  SetPosition(TX1,TY1,TX2,TY2);
  if ((Tree=new TreeList)==NULL)
    return;
  {
    *LastName=0;
    Tree->SetModalMode(ModalMode);
    Tree->SetPosition(X1,Y1,X2,Y2);
    if(ModalMode == MODALTREE_FREE)
      Tree->SetRootDir(ResultFolder);
    Tree->Update(0);
    Tree->Show();
    if(ModalMode == MODALTREE_FREE)
    {
      Tree->Update(UPDATE_KEEP_SELECTION);
      Tree->GoToFile(ResultFolder);
      Tree->Redraw();
    }
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
  if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+255)
    Key=tolower(Key-KEY_ALT_BASE);
  switch(Key)
  {
    case KEY_F1:
      {
        Help Hlp ("FindFolder");
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
  FindEdit->SetPosition(X1+strlen(SearchTxt)+2,FindY,Min(X2-1,X1+25),FindY);
  FindEdit->SetObjectColor(COL_DIALOGEDIT);
  FindEdit->Show();
  if (WhereX()<X2)
  {
    SetColor(COL_PANELTEXT);
    mprintf("%*s",X2-WhereX(),"");
  }
}
