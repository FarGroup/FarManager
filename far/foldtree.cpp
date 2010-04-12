/*
foldtree.cpp

Поиск каталога по Alt-F10

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
#include "filepanels.hpp"
#include "help.hpp"
#include "manager.hpp"
#include "savescr.hpp"

FolderTree::FolderTree(char *ResultFolder,int iModalMode,int IsStandalone,int IsFullScreen)
{
	SetDynamicallyBorn(FALSE);
	ModalMode=iModalMode;
	PrevMacroMode=CtrlObject->Macro.GetMode();
	SetRestoreScreenMode(TRUE);

	if (ModalMode != MODALTREE_FREE)
		*ResultFolder=0;

	*NewFolder=0;
	FolderTree::IsFullScreen=IsFullScreen;
	FolderTree::IsStandalone=IsStandalone;
	FindEdit=NULL;
	Tree=NULL;
	KeyBarVisible = TRUE;  // Заставим обновлятся кейбар
	//TopScreen=new SaveScreen;
	SetCoords();

	if ((Tree=new TreeList(FALSE))!=NULL)
	{
		CtrlObject->Macro.SetMode(MACRO_FINDFOLDER);
		MacroMode = MACRO_FINDFOLDER;
		*LastName=0;
		Tree->SetModalMode(ModalMode);
		Tree->SetPosition(X1,Y1,X2,Y2);

		if (ModalMode == MODALTREE_FREE)
			Tree->SetRootDir(ResultFolder);

		Tree->SetVisible(TRUE);
		Tree->Update(0);

		// если было прерывание в процессе сканирования и это было дерево копира...
		if (Tree->GetExitCode())
		{
			if ((FindEdit=new Edit)==NULL)
			{
				SetExitCode(XC_OPEN_ERROR);
				return;
			}

			FindEdit->SetEditBeyondEnd(FALSE);
			FindEdit->SetPersistentBlocks(Opt.Dialogs.EditBlock);
			InitKeyBar();
			FrameManager->ExecuteModal(this); //OT
		}

		strcpy(ResultFolder,NewFolder);
	}
	else
	{
		SetExitCode(XC_OPEN_ERROR);
	}
}

FolderTree::~FolderTree()
{
	CtrlObject->Macro.SetMode(PrevMacroMode);

	//if ( TopScreen )    delete TopScreen;
	if (FindEdit)     delete FindEdit;

	if (Tree)         delete Tree;
}

void FolderTree::DisplayObject()
{
	//if(!TopScreen)
	//  TopScreen=new SaveScreen;
	if (ModalMode == MODALTREE_FREE)
	{
		char SelFolder[2*NM];
		Tree->GetCurDir(SelFolder);
		//Tree->Update(UPDATE_KEEP_SELECTION);
		Tree->Update(0);
		Tree->GoToFile(SelFolder);
	}

	Tree->Redraw();
	Shadow();
	DrawEdit();

	if (!IsFullScreen)
	{
		TreeKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
		TreeKeyBar.Show();
	}
	else
		TreeKeyBar.Hide();
}

void FolderTree::SetCoords()
{
	if (IsFullScreen)
		SetPosition(0,0,ScrX,ScrY);
	else
	{
		if (IsStandalone)
			SetPosition(4,2,ScrX-4,ScrY-4);
		else
			SetPosition(ScrX/3,2,ScrX-7,ScrY-5);
	}
}

void FolderTree::OnChangeFocus(int focus)
{
	if (focus)
		Show();
}

void FolderTree::ResizeConsole()
{
	//if ( TopScreen )
	//   delete TopScreen;
	//TopScreen=NULL;
	Hide();
	SetCoords();
	Tree->SetPosition(X1,Y1,X2,Y2);
	//ReadHelp(StackData.HelpMask);
	FrameManager->ImmediateHide();
	FrameManager->RefreshFrame();
}

void FolderTree::SetScreenPosition()
{
	if (IsFullScreen)
		TreeKeyBar.Hide();

	SetCoords();
	Show();
}

int FolderTree::FastHide()
{
	return Opt.AllCtrlAltShiftRule & CASR_DIALOG;
}


int FolderTree::GetTypeAndName(char *Type,char *Name)
{
	if (Type)
		strcpy(Type,MSG(MFolderTreeType));

	if (Name)
		strcpy(Name,"");

	return MODALTYPE_FINDFOLDER;
}

int FolderTree::ProcessKey(int Key)
{
	if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+255)
		Key=tolower(Key-KEY_ALT_BASE);

	switch (Key)
	{
		case KEY_F1:
		{
			Help Hlp("FindFolder");
		}
		break;
		case KEY_ESC:
		case KEY_F10:
			FrameManager->DeleteFrame();
			SetExitCode(XC_MODIFIED);
			break;
		case KEY_NUMENTER:
		case KEY_ENTER:
			Tree->GetCurDir(NewFolder);

			if (GetFileAttributes(NewFolder)!=INVALID_FILE_ATTRIBUTES)
			{
				FrameManager->DeleteFrame();
				SetExitCode(XC_MODIFIED);
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
		case KEY_CTRLNUMENTER:
		case KEY_CTRLSHIFTNUMENTER:
		case KEY_CTRLENTER:
		case KEY_CTRLSHIFTENTER:
		{
			char Name[NM];
			FindEdit->GetString(Name,sizeof(Name));
			Tree->FindPartName(Name,TRUE,Key == KEY_CTRLSHIFTENTER||Key == KEY_CTRLSHIFTNUMENTER?-1:1,1);
			DrawEdit();
		}
		break;
		case KEY_UP:
		case KEY_NUMPAD8:
		case KEY_DOWN:
		case KEY_NUMPAD2:
		case KEY_PGUP:
		case KEY_NUMPAD9:
		case KEY_PGDN:
		case KEY_NUMPAD3:
		case KEY_HOME:
		case KEY_NUMPAD7:
		case KEY_END:
		case KEY_NUMPAD1:
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
			FindEdit->SetString("");
			Tree->ProcessKey(Key);
			DrawEdit();
			break;
		default:

			if (Key == KEY_ADD || Key == KEY_SUBTRACT) // OFM: Gray+/Gray- navigation
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

				if (Tree->FindPartName(Name,FALSE,1,1))
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
	if (TreeKeyBar.ProcessMouse(MouseEvent))
		return TRUE;

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

	if (MsY == Y2-2)
		FindEdit->ProcessMouse(MouseEvent);
	else
	{
		if (!Tree->ProcessMouse(MouseEvent))
			SetExitCode(XC_MODIFIED);
		else
			DrawEdit();
	}

	return(TRUE);
}


void FolderTree::DrawEdit()
{
	int FindY=Y2-2;
	char *SearchTxt=MSG(MFoldTreeSearch);
	GotoXY(X1+1,FindY);
	SetColor(COL_PANELTEXT);
	mprintf("%s  ",SearchTxt);
	FindEdit->SetPosition(X1+(int)strlen(SearchTxt)+2,FindY,Min(X2-1,X1+25),FindY);
	FindEdit->SetObjectColor(COL_DIALOGEDIT);
	FindEdit->Show();

	if (WhereX()<X2)
	{
		SetColor(COL_PANELTEXT);
		mprintf("%*s",X2-WhereX(),"");
	}
}

void FolderTree::InitKeyBar(void)
{
	static const char *FTreeKeysLabel[]={"","","","","","","","","","","",""};
	TreeKeyBar.Set(FTreeKeysLabel,sizeof(FTreeKeysLabel)/sizeof(FTreeKeysLabel[0]));
	TreeKeyBar.SetAlt(FTreeKeysLabel,sizeof(FTreeKeysLabel)/sizeof(FTreeKeysLabel[0]));
	TreeKeyBar.Change(KBL_MAIN,MSG(MKBFolderTreeF1),1-1);
	TreeKeyBar.Change(KBL_MAIN,MSG(MKBFolderTreeF2),2-1);
	TreeKeyBar.Change(KBL_MAIN,MSG(MKBFolderTreeF5),5-1);
	TreeKeyBar.Change(KBL_MAIN,MSG(MKBFolderTreeF10),10-1);
	TreeKeyBar.Change(KBL_ALT,MSG(MKBFolderTreeAltF9),9-1);
	SetKeyBar(&TreeKeyBar);
}
