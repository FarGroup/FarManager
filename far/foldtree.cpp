/*
foldtree.cpp

Поиск каталога по Alt-F10

*/

/* Revision: 1.14 17.10.2004 $ */

/*
Modify:
  17.10.2004 SVS
    + MACRO_FINDFOLDER
    + Навигация Gray+ Gray- по OFM
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  22.04.2002 KM
    - Затычка: запрет на AltF9 в помощи. Пока.
  22.03.2002 SVS
    - strcpy - Fuck!
  18.03.2002 SVS
    ! Уточнения, в связи с введением Opt.Dialogs
  24.10.2001 SVS
    + дополнительный параметр у FolderTree - "ЭТО НЕ ПАНЕЛЬ!"
    - бага с прорисовкой при вызове дерева из диалога копирования
  08.09.2001 VVM
    + Использовать Opt.DialogsEditBlock
  30.08.2001 VVM
    ! Блоки в строке поиска непостоянны :)
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

#include "global.hpp"
#include "foldtree.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "treelist.hpp"
#include "edit.hpp"
#include "ctrlobj.hpp"
#include "help.hpp"
#include "savescr.hpp"

FolderTree::FolderTree(char *ResultFolder,int ModalMode,int TX1,int TY1,int TX2,int TY2,int IsPanel)
{
  SetRestoreScreenMode(FALSE);
  SaveScreen SaveScr;
  if(ModalMode != MODALTREE_FREE)
    *ResultFolder=0;
  *NewFolder=0;
  SetPosition(TX1,TY1,TX2,TY2);
  int CurMacroMode=CtrlObject->Macro.GetMode();

  if ((Tree=new TreeList(IsPanel))!=NULL)
  {
    CtrlObject->Macro.SetMode(MACRO_FINDFOLDER);
    *LastName=0;
    Tree->SetModalMode(ModalMode);
    Tree->SetPosition(X1,Y1,X2,Y2);
    if(ModalMode == MODALTREE_FREE)
      Tree->SetRootDir(ResultFolder);
    Tree->Update(0);
    Tree->Show();
    // если было прерывание в процессе сканирования и это было дерево копира...
    if(Tree->GetExitCode())
    {
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
          CtrlObject->Macro.SetMode(CurMacroMode);
          return;
        }
        FindEdit->SetEditBeyondEnd(FALSE);
        FindEdit->SetPersistentBlocks(Opt.Dialogs.EditBlock);
        DrawEdit();
      }

      Process();
      delete FindEdit;

    }
    delete Tree;

    CtrlObject->Macro.SetMode(CurMacroMode);
    strcpy(ResultFolder,NewFolder);
  }
}


int FolderTree::ProcessKey(int Key)
{
  if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+255)
    Key=tolower(Key-KEY_ALT_BASE);
  switch(Key)
  {
    case KEY_F1:
      {
        /* $ 22.04.2002 KM
            Поставим пока запрет на AltF9 в дереве,
            пока класс дерева не переписан на совместимость
            с Frame.
        */
        IsProcessAssignMacroKey++;
        Help Hlp ("FindFolder");
        IsProcessAssignMacroKey--;
        /* KM $ */
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
      if(Key == KEY_ADD || Key == KEY_SUBTRACT) // OFM: Gray+/Gray- navigation
      {
        Tree->ProcessKey(Key);
        DrawEdit();
        break;
      }
/*
      else
      {
        if((Key&(~KEY_CTRLMASK)) == KEY_ADD)
          Key='+';
        else if((Key&(~KEY_CTRLMASK)) == KEY_SUBTRACT)
          Key='-';
      }
*/
      if (FindEdit->ProcessKey(Key))
      {
        char Name[NM];
        FindEdit->GetString(Name,sizeof(Name));
        if (Tree->FindPartName(Name,FALSE))
          strcpy(LastName,Name);
        else
        {
          FindEdit->SetString(LastName);
          xstrncpy(Name,LastName,sizeof(Name)-1);
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
