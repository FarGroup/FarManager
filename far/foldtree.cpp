/*
foldtree.cpp

ѕоиск каталога по Alt-F10

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
#include "manager.hpp"
#include "savescr.hpp"

FolderTree::FolderTree(string &strResultFolder,int iModalMode,int IsStandalone,int IsFullScreen)
{
  SetDynamicallyBorn(FALSE);

  ModalMode=iModalMode;
  PrevMacroMode=CtrlObject->Macro.GetMode();
  SetRestoreScreenMode(FALSE);

  if(ModalMode != MODALTREE_FREE)
    strResultFolder=L"";

  strNewFolder=L"";

  FolderTree::IsFullScreen=IsFullScreen;
  FolderTree::IsStandalone=IsStandalone;

  FindEdit=NULL;
  Tree=NULL;
  TopScreen=new SaveScreen;

  SetCoords();

  if ((Tree=new TreeList(FALSE))!=NULL)
  {
    CtrlObject->Macro.SetMode(MACRO_FINDFOLDER);
    MacroMode = MACRO_FINDFOLDER;

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
      if ((FindEdit=new Edit)==NULL)
      {
        SetExitCode (XC_OPEN_ERROR);
        return;
      }
      FindEdit->SetEditBeyondEnd(FALSE);
      FindEdit->SetPersistentBlocks(Opt.Dialogs.EditBlock);

      FrameManager->ExecuteModal (this);//OT
    }
    strResultFolder = strNewFolder;
  }
  else
  {
    SetExitCode (XC_OPEN_ERROR);
  }
}

FolderTree::~FolderTree()
{
  CtrlObject->Macro.SetMode(PrevMacroMode);
  SetRestoreScreenMode(FALSE);
  if ( TopScreen )    delete TopScreen;
  if ( FindEdit )     delete FindEdit;
  if ( Tree )         delete Tree;
}


void FolderTree::DisplayObject()
{
  if(!TopScreen)
    TopScreen=new SaveScreen;
  if(ModalMode == MODALTREE_FREE)
  {
    string strSelFolder;
    Tree->GetCurDirW(strSelFolder);
    //Tree->Update(UPDATE_KEEP_SELECTION);
    Tree->Update(0);
    Tree->GoToFileW(strSelFolder);
    Tree->Redraw();
  }
  else
    Tree->Redraw();
  MakeShadow(X1+2,Y2+1,X2+2,Y2+1);
  MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
  DrawEdit();
}


void FolderTree::SetCoords()
{
  if(IsFullScreen)
    SetPosition(0,0,ScrX,ScrY);
  else
  {
    if(IsStandalone)
      SetPosition(4,2,ScrX-4,ScrY-4);
    else
      SetPosition(ScrX/3,2,ScrX-7,ScrY-5);
  }
}

void FolderTree::Hide()
{
  ScreenObject::Hide();
}

void FolderTree::OnChangeFocus(int focus)
{
  if (focus)
  {
    DisplayObject();
  }
}

void FolderTree::ResizeConsole()
{
  if ( TopScreen )
     delete TopScreen;
  TopScreen=NULL;

  Hide();

  SetCoords();
  Tree->SetPosition(X1,Y1,X2,Y2);
  //ReadHelp(StackData.HelpMask);
  FrameManager->ImmediateHide();
  FrameManager->RefreshFrame();
}

int FolderTree::FastHide()
{
  return Opt.AllCtrlAltShiftRule & CASR_DIALOG;
}


int FolderTree::GetTypeAndName(string &strType, string &strName)
{
  strType = UMSG(MFolderTreeType);
  strName = L"";

  return MODALTYPE_FINDFOLDER;
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
      FrameManager->DeleteFrame();
      SetExitCode (XC_MODIFIED);
      break;
    case KEY_ENTER:
      Tree->GetCurDirW(strNewFolder);
      if (GetFileAttributesW(strNewFolder)!=0xFFFFFFFF)
      {
        FrameManager->DeleteFrame();
        SetExitCode (XC_MODIFIED);
      }
      else
      {
        Tree->ProcessKey(KEY_ENTER);
        DrawEdit();
      }
      break;
    case KEY_F5:
      IsFullScreen=!IsFullScreen;
      ResizeConsole();
      return(TRUE);
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

  int MsX=MouseEvent->dwMousePosition.X;
  int MsY=MouseEvent->dwMousePosition.Y;

  if ((MsX<X1 || MsY<Y1 || MsX>X2 || MsY>Y2) && MouseEventFlags != MOUSE_MOVED)
  {
    if (!(MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && PrevLButtonPressed && (Opt.Dialogs.MouseButton&DMOUSEBUTTON_LEFT))
      ProcessKey(KEY_ESC);
    else if (!(MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) && PrevRButtonPressed && (Opt.Dialogs.MouseButton&DMOUSEBUTTON_RIGHT))
      ProcessKey(KEY_ENTER);
    return(TRUE);
  }
  if(MsY == Y2-2)
    FindEdit->ProcessMouse(MouseEvent);
  else
  {
    if (!Tree->ProcessMouse(MouseEvent))
      SetExitCode (XC_MODIFIED);
    else
      DrawEdit();
  }
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
