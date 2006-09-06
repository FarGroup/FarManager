/*
foldtree.cpp

ѕоиск каталога по Alt-F10

*/

/* Revision: 1.19 15.03.2006 $ */

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

FolderTree::FolderTree(string &strResultFolder,int ModalMode,int TX1,int TY1,int TX2,int TY2,int IsPanel)
{
  SetRestoreScreenMode(FALSE);
  SaveScreen SaveScr;
  if(ModalMode != MODALTREE_FREE)
    strResultFolder=L"";

  strNewFolder=L"";
  SetPosition(TX1,TY1,TX2,TY2);
  int CurMacroMode=CtrlObject->Macro.GetMode();

  if ((Tree=new TreeList(IsPanel))!=NULL)
  {
    CtrlObject->Macro.SetMode(MACRO_FINDFOLDER);

    strLastName=L"";
    Tree->SetModalMode(ModalMode);
    Tree->SetPosition(X1,Y1,X2,Y2);
    if(ModalMode == MODALTREE_FREE)
      Tree->SetRootDirW(strResultFolder);
    Tree->Update(0);
    Tree->Show();
    // если было прерывание в процессе сканировани€ и это было дерево копира...
    if(Tree->GetExitCode())
    {
      if(ModalMode == MODALTREE_FREE)
      {
        Tree->Update(UPDATE_KEEP_SELECTION);
        Tree->GoToFileW(strResultFolder);
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
    strResultFolder = strNewFolder;
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
            ѕоставим пока запрет на AltF9 в дереве,
            пока класс дерева не переписан на совместимость
            с Frame.
        */
        IsProcessAssignMacroKey++;
        Help Hlp (L"FindFolder");
        IsProcessAssignMacroKey--;
        /* KM $ */
      }
      break;
    case KEY_ESC:
    case KEY_F10:
      SetExitCode(1);
      break;
    case KEY_ENTER:
      Tree->GetCurDirW(strNewFolder);
      if (GetFileAttributesW(strNewFolder)!=0xFFFFFFFF)
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
        string strName;
        FindEdit->GetStringW(strName);
        Tree->FindPartName(strName,TRUE);
        DrawEdit();
      }
      break;
    case KEY_CTRLSHIFTENTER:
      {
        string strName;
        FindEdit->GetStringW(strName);
        Tree->FindPartName(strName,TRUE,-1);
        DrawEdit();
      }
      break;

    case KEY_UP:
    case KEY_DOWN:
    case KEY_PGUP:
    case KEY_PGDN:
    case KEY_HOME:
    case KEY_END:
      FindEdit->SetStringW(L"");
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
        string strName;
        FindEdit->GetStringW(strName);
        if (Tree->FindPartName(strName,FALSE))
          strLastName = strName;
        else
        {
          FindEdit->SetStringW(strLastName);
          strName = strLastName;
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
  wchar_t *SearchTxt=UMSG(MFoldTreeSearch);
  GotoXY(X1+1,FindY);
  SetColor(COL_PANELTEXT);
  mprintfW(L"%s  ",SearchTxt);
  FindEdit->SetPosition(X1+wcslen(SearchTxt)+2,FindY,Min(X2-1,X1+25),FindY);
  FindEdit->SetObjectColor(COL_DIALOGEDIT);
  FindEdit->Show();
  if (WhereX()<X2)
  {
    SetColor(COL_PANELTEXT);
    mprintfW(L"%*s",X2-WhereX(),L"");
  }
}
