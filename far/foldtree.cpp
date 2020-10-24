/*
foldtree.cpp

Поиск каталога по Alt-F10
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "foldtree.hpp"

// Internal:
#include "keyboard.hpp"
#include "keys.hpp"
#include "treelist.hpp"
#include "editcontrol.hpp"
#include "help.hpp"
#include "manager.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "exitcode.hpp"
#include "lang.hpp"
#include "keybar.hpp"
#include "string_utils.hpp"
#include "global.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

FolderTree::FolderTree(private_tag, int ModalMode, int IsStandalone, bool IsFullScreen):
	m_Tree(nullptr),
	m_FindEdit(nullptr),
	m_ModalMode(ModalMode),
	m_IsFullScreen(IsFullScreen),
	m_IsStandalone(IsStandalone)
{
}

foldertree_ptr FolderTree::create(string &strResultFolder, int ModalMode, int IsStandalone, bool IsFullScreen)
{
	auto FoldertreePtr = std::make_shared<FolderTree>(private_tag(), ModalMode, IsStandalone, IsFullScreen);
	FoldertreePtr->init(strResultFolder);
	return FoldertreePtr;
}

void FolderTree::init(string &strResultFolder)
{
	m_windowKeyBar = std::make_unique<KeyBar>(shared_from_this());

	SetRestoreScreenMode(true);
	if (m_ModalMode != MODALTREE_FREE)
		strResultFolder.clear();
	SetCoords();

	m_Tree = TreeList::create(nullptr, m_ModalMode);

		SetMacroMode(MACROAREA_FINDFOLDER);
		m_LastName.clear();
		m_Tree->SetPosition(m_Where);

		if (m_ModalMode == MODALTREE_FREE)
			m_Tree->SetRootDir(strResultFolder);

		m_Tree->SetVisible(true);
		m_Tree->Update(0);

		// если было прерывание в процессе сканирования и это было дерево копира...
		if (m_Tree->GetExitCode())
		{
			m_FindEdit = std::make_unique<EditControl>(shared_from_this(),shared_from_this().get());
			m_FindEdit->SetEditBeyondEnd(false);
			m_FindEdit->SetPersistentBlocks(Global->Opt->Dialogs.EditBlock);
			InitKeyBar();
			Global->WindowManager->ExecuteWindow(shared_from_this()); //OT
			Global->WindowManager->ExecuteModal(shared_from_this()); //OT
		}

		strResultFolder = m_NewFolder;
}

void FolderTree::DisplayObject()
{
	//if(!TopScreen) TopScreen=new SaveScreen;
	if (m_ModalMode == MODALTREE_FREE)
	{
		const auto strSelFolder = m_Tree->GetCurDir();
		//Tree->Update(UPDATE_KEEP_SELECTION);
		m_Tree->Update(0);
		m_Tree->GoToFile(strSelFolder);
	}

	m_Tree->Redraw();
	Shadow();
	DrawEdit();

	if (!m_IsFullScreen)
	{
		m_windowKeyBar->SetPosition({ 0, ScrY, ScrX, ScrY });
		m_windowKeyBar->Show();
	}
	else
		m_windowKeyBar->Hide();
}


void FolderTree::SetCoords()
{
	if (m_IsFullScreen)
	{
		SetPosition({ 0, 0, ScrX, ScrY });
	}
	else
	{
		if (m_IsStandalone)
			SetPosition({ 4, 2, ScrX - 4, ScrY - 4 });
		else
			SetPosition({ ScrX / 3, 2, ScrX - 7, ScrY - 4 });
	}
}

void FolderTree::ResizeConsole()
{
	Hide();
	SetCoords();
	m_Tree->SetPosition(m_Where);
}

void FolderTree::SetScreenPosition()
{
	if (m_IsFullScreen)
		m_windowKeyBar->Hide();

	SetCoords();
	Show();
}

bool FolderTree::CanFastHide() const
{
	return (Global->Opt->AllCtrlAltShiftRule & CASR_DIALOG) != 0;
}


int FolderTree::GetTypeAndName(string &strType, string &strName)
{
	strType = msg(lng::MFolderTreeType);
	strName.clear();
	return windowtype_findfolder;
}


bool FolderTree::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key();
	if (LocalKey>=KEY_ALT_BASE+0x01 && LocalKey<=KEY_ALT_BASE+65535)
		LocalKey=lower(LocalKey-KEY_ALT_BASE);
	else if (LocalKey>=KEY_RALT_BASE+0x01 && LocalKey<=KEY_RALT_BASE+65535)
		LocalKey=lower(LocalKey-KEY_RALT_BASE);

	switch (LocalKey)
	{
		case KEY_F1:
			help::show(L"FindFolder"sv);
			break;

		case KEY_ESC:
		case KEY_F10:
			Global->WindowManager->DeleteWindow();
			SetExitCode(XC_MODIFIED);
			break;
		case KEY_NUMENTER:
		case KEY_ENTER:
			m_NewFolder = m_Tree->GetCurDir();

			if (os::fs::exists(m_NewFolder))
			{
				Global->WindowManager->DeleteWindow();
				SetExitCode(XC_MODIFIED);
			}
			else
			{
				m_Tree->ProcessKey(Manager::Key(KEY_ENTER));
				DrawEdit();
			}

			break;
		case KEY_F5:
			m_IsFullScreen=!m_IsFullScreen;
			ResizeConsole();
			Show();
			return true;
		case KEY_CTRLR:		case KEY_RCTRLR:
		case KEY_F2:
			m_Tree->ProcessKey(Manager::Key(KEY_CTRLR));
			DrawEdit();
			break;
		case KEY_CTRLNUMENTER:       case KEY_RCTRLNUMENTER:
		case KEY_CTRLSHIFTNUMENTER:  case KEY_RCTRLSHIFTNUMENTER:
		case KEY_CTRLENTER:          case KEY_RCTRLENTER:
		case KEY_CTRLSHIFTENTER:     case KEY_RCTRLSHIFTENTER:
		{
			m_Tree->FindPartName(m_FindEdit->GetString(), TRUE, any_of(LocalKey, KEY_CTRLSHIFTENTER, KEY_RCTRLSHIFTENTER, KEY_CTRLSHIFTNUMENTER, KEY_RCTRLSHIFTNUMENTER)? -1 : 1);
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
		case(KEY_MSWHEEL_UP | KEY_RALT):
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		case(KEY_MSWHEEL_DOWN | KEY_RALT):
			m_FindEdit->ClearString();
			m_Tree->ProcessKey(Key);
			DrawEdit();
			break;
		default:

			if (any_of(LocalKey, KEY_ADD, KEY_SUBTRACT)) // OFM: Gray+/Gray- navigation
			{
				m_Tree->ProcessKey(Key);
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
			if (m_FindEdit->ProcessKey(Key))
			{
				const auto& strName = m_FindEdit->GetString();

				if (m_Tree->FindPartName(strName, FALSE, 1))
					m_LastName = strName;
				else
				{
					m_FindEdit->SetString(m_LastName);
				}

				DrawEdit();
			}

			break;
	}

	return true;
}


bool FolderTree::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (m_windowKeyBar->ProcessMouse(MouseEvent))
		return true;

	if (MouseEvent->dwEventFlags==DOUBLE_CLICK)
	{
		ProcessKey(Manager::Key(KEY_ENTER));
		return true;
	}

	if (!m_Where.contains(MouseEvent->dwMousePosition) && IntKeyState.MouseEventFlags != MOUSE_MOVED)
	{
		if (!(MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (Global->Opt->Dialogs.MouseButton&DMOUSEBUTTON_LEFT))
			ProcessKey(Manager::Key(KEY_ESC));
		else if (!(MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&RIGHTMOST_BUTTON_PRESSED) && (Global->Opt->Dialogs.MouseButton&DMOUSEBUTTON_RIGHT))
			ProcessKey(Manager::Key(KEY_ENTER));

		return true;
	}

	if (MouseEvent->dwMousePosition.Y == m_Where.bottom - 2)
		m_FindEdit->ProcessMouse(MouseEvent);
	else
	{
		if (!m_Tree->ProcessMouse(MouseEvent))
			SetExitCode(XC_MODIFIED);
		else
			DrawEdit();
	}

	return true;
}


void FolderTree::DrawEdit() const
{
	const auto FindY = m_Where.bottom - 2;
	const auto& SearchTxt = msg(lng::MFoldTreeSearch);
	GotoXY(m_Where.left + 1, FindY);
	SetColor(COL_PANELTEXT);
	Text(concat(SearchTxt, L"  "sv));
	m_FindEdit->SetPosition({ m_Where.left + static_cast<int>(SearchTxt.size()) + 2, FindY, std::min(m_Where.right - 1, m_Where.left + 25), FindY });
	m_FindEdit->SetObjectColor(COL_DIALOGEDIT);
	m_FindEdit->Show();

	if (WhereX() < m_Where.right)
	{
		SetColor(COL_PANELTEXT);
		Text(string(m_Where.right - WhereX(), L' '));
	}
}


void FolderTree::InitKeyBar()
{
	m_windowKeyBar->SetLabels(lng::MFolderTreeF1);
	m_windowKeyBar->SetCustomLabels(KBA_FOLDERTREE);
}
