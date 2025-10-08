/*
filepanels.cpp

Файловые панели
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
#include "filepanels.hpp"

// Internal:
#include "keys.hpp"
#include "macroopcode.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "treelist.hpp"
#include "qview.hpp"
#include "infolist.hpp"
#include "help.hpp"
#include "filefilter.hpp"
#include "findfile.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "interf.hpp"
#include "lang.hpp"
#include "config.hpp"
#include "keybar.hpp"
#include "menubar.hpp"
#include "strmix.hpp"
#include "diskmenu.hpp"
#include "global.hpp"
#include "keyboard.hpp"
#include "colormix.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.fs.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

FilePanels::FilePanels(private_tag):
	m_ActivePanelIndex(panel_left)
{
}

FilePanels::~FilePanels() = default;

filepanels_ptr FilePanels::create(bool CreateRealPanels)
{
	const auto FilePanelsPtr = std::make_shared<FilePanels>(private_tag());

	FilePanelsPtr->m_windowKeyBar = std::make_unique<KeyBar>(FilePanelsPtr);
	FilePanelsPtr->SetMacroMode(MACROAREA_SHELL);

	if (CreateRealPanels)
	{
		FilePanelsPtr->m_Panels[panel_left].m_Panel = FilePanelsPtr->CreatePanel(static_cast<panel_type>(Global->Opt->LeftPanel.m_Type.Get()));
		FilePanelsPtr->m_Panels[panel_right].m_Panel = FilePanelsPtr->CreatePanel(static_cast<panel_type>(Global->Opt->RightPanel.m_Type.Get()));
		FilePanelsPtr->Init();
	}
	else
	{
		FilePanelsPtr->m_Panels[panel_left].m_Panel = std::make_unique<dummy_panel>(FilePanelsPtr);
		FilePanelsPtr->m_Panels[panel_right].m_Panel = std::make_unique<dummy_panel>(FilePanelsPtr);
		FilePanelsPtr->CmdLine = std::make_unique<CommandLine>(FilePanelsPtr);
	}
	return FilePanelsPtr;
}

static void PrepareOptFolder(string &strSrc, bool IsLocalPath_FarPath)
{
	if (strSrc.empty())
	{
		strSrc = Global->g_strFarPath;
		DeleteEndSlash(strSrc);
	}
	else
	{
		strSrc = os::env::expand(strSrc);
	}

	if (strSrc == L"/"sv)
	{
		strSrc = Global->g_strFarPath;

		if (IsLocalPath_FarPath)
		{
			strSrc.resize(2);
			strSrc += path::separator;
		}
	}
	else
	{
		if (!os::fs::is_directory(strSrc))
			CutToExistingParent(strSrc);
	}

	//ConvertNameToFull(strSrc,strSrc);
}

void FilePanels::Init()
{
	CmdLine = std::make_unique<CommandLine>(shared_from_this());
	TopMenuBar = std::make_unique<MenuBar>(shared_from_this());

	m_ActivePanelIndex = Global->Opt->LeftFocus? panel_left : panel_right;

	const std::pair<panel_ptr, Options::PanelOptions&>
		Left(LeftPanel(), Global->Opt->LeftPanel),
		Right(RightPanel(), Global->Opt->RightPanel);

	SetPanelPositions(
		FileList::IsModeFullScreen(Left.second.ViewMode),
		FileList::IsModeFullScreen(Right.second.ViewMode)
	);

	const auto InitPanel = [](const std::pair<panel_ptr, const Options::PanelOptions&>& Params)
	{
		Params.first->SetViewMode(Params.second.ViewMode);

		if (static_cast<panel_sort>(Params.second.SortMode.Get()) < panel_sort::COUNT)
			Params.first->SetSortMode(static_cast<panel_sort>(Params.second.SortMode.Get()));

		Params.first->SetSortOrder(Params.second.ReverseSortOrder);
		Params.first->SetSortGroups(Params.second.SortGroups);
		Params.first->SetShowShortNamesMode(Params.second.ShowShortNames);
		Params.first->SetSelectedFirstMode(Params.second.SelectedFirst);
		Params.first->SetDirectoriesFirst(Params.second.DirectoriesFirst);
	};

	InitPanel(Left);
	InitPanel(Right);

	SetCanLoseFocus(true);

	SetActivePanelInternal(ActivePanel());

	// пытаемся избавится от зависания при запуске
	const auto IsLocalPath_FarPath = ParsePath(Global->g_strFarPath) == root_type::drive_letter;

	const auto SetFolder = [&](const std::pair<panel_ptr, Options::PanelOptions&>& Params)
	{
		auto Folder = Params.second.Folder.Get();
		PrepareOptFolder(Folder, IsLocalPath_FarPath);
		Params.second.Folder = Folder;
	};

	SetFolder(Left);
	SetFolder(Right);

	const auto InitCurDir_checked = [&](const std::pair<panel_ptr, const Options::PanelOptions&>& Params)
	{
		Params.first->InitCurDir(os::fs::exists(Params.second.Folder.Get())? Params.second.Folder.Get() : Global->g_strFarPath);
	};

	InitCurDir_checked(Left);
	InitCurDir_checked(Right);

#if 1
	const auto show_if_visible = [](const std::pair<panel_ptr, Options::PanelOptions&>& Params)
	{
		if (Params.second.Visible)
			Params.first->Show();
	};

	//! Вначале "показываем" пассивную панель
	if (m_ActivePanelIndex == panel_right)
	{
		show_if_visible(Left);
		show_if_visible(Right);
	}
	else
	{
		show_if_visible(Right);
		show_if_visible(Left);
	}

#endif

	// при погашенных панелях не забыть бы выставить корректно каталог в CmdLine
	if (!Right.second.Visible && !Left.second.Visible)
	{
		CmdLine->SetCurDir(m_ActivePanelIndex == panel_right? Right.second.Folder : Left.second.Folder);
	}

	SetScreenPosition();
}

void FilePanels::SetPanelPositions(bool LeftFullScreen, bool RightFullScreen) const
{
	const auto AbsMaxWidthDecrement = std::max(0ll, ScrX / 2 - 10ll);
	Global->Opt->WidthDecrement = std::max(-AbsMaxWidthDecrement, std::min(Global->Opt->WidthDecrement.Get(), AbsMaxWidthDecrement));
	Global->Opt->LeftHeightDecrement = std::max(0ll, std::min(Global->Opt->LeftHeightDecrement.Get(), ScrY - 7ll));
	Global->Opt->RightHeightDecrement = std::max(0ll, std::min(Global->Opt->RightHeightDecrement.Get(), ScrY - 7ll));

	const auto Left = LeftPanel();
	const auto Right = RightPanel();

	if (LeftFullScreen)
	{
		Left->SetPosition(
			{
				0,
				Global->Opt->ShowMenuBar? 1 : 0,
				ScrX,
				static_cast<int>(ScrY - 1 - Global->Opt->ShowKeyBar - Global->Opt->LeftHeightDecrement)
			});
		Left->SetFullScreen();
	}
	else
	{
		Left->SetPosition(
			{
				0,
				Global->Opt->ShowMenuBar? 1 : 0,
				static_cast<int>(ScrX / 2 - Global->Opt->WidthDecrement),
				static_cast<int>(ScrY - 1 - Global->Opt->ShowKeyBar - Global->Opt->LeftHeightDecrement)
			});
	}

	if (RightFullScreen)
	{
		Right->SetPosition(
			{
				0,
				Global->Opt->ShowMenuBar? 1 : 0,
				ScrX,
				static_cast<int>(ScrY - 1 - Global->Opt->ShowKeyBar - Global->Opt->RightHeightDecrement)
			});
		Right->SetFullScreen();
	}
	else
	{
		Right->SetPosition(
			{
				static_cast<int>(ScrX / 2 + 1 - Global->Opt->WidthDecrement),
				Global->Opt->ShowMenuBar? 1 : 0,
				ScrX,
				static_cast<int>(ScrY - 1 - Global->Opt->ShowKeyBar - Global->Opt->RightHeightDecrement)
			});
	}
}

void FilePanels::SetScreenPosition()
{
	CmdLine->SetPosition({ 0, ScrY - Global->Opt->ShowKeyBar, ScrX - 1, ScrY - Global->Opt->ShowKeyBar });
	TopMenuBar->SetPosition({ 0, 0, ScrX, 0 });
	m_windowKeyBar->SetPosition({ 0, ScrY, ScrX, ScrY });
	SetPanelPositions(LeftPanel()->IsFullScreen(), RightPanel()->IsFullScreen());
	SetPosition({ 0, 0, ScrX, ScrY });
}

void FilePanels::RedrawKeyBar()
{
	ActivePanel()->UpdateKeyBar();
	m_windowKeyBar->Redraw();
}


panel_ptr FilePanels::CreatePanel(panel_type Type)
{
	switch (Type)
	{
	default:
	case panel_type::FILE_PANEL:
		return FileList::create(shared_from_this());
	case panel_type::TREE_PANEL:
		return TreeList::create(shared_from_this());
	case panel_type::QVIEW_PANEL:
		return QuickView::create(shared_from_this());
	case panel_type::INFO_PANEL:
		return InfoList::create(shared_from_this());
	}
}

int FilePanels::SetAnhoterPanelFocus()
{
	bool Result = false;

	if (IsLeftActive())
	{
		const auto Right = RightPanel();
		if (Right->IsVisible())
		{
			SetActivePanel(Right);
			Result = true;
		}
	}
	else
	{
		const auto Left = LeftPanel();
		if (Left->IsVisible())
		{
			SetActivePanel(Left);
			Result = true;
		}
	}

	return Result;
}


int FilePanels::SwapPanels()
{
	if (!LeftPanel()->IsVisible() && !RightPanel()->IsVisible())
		return false;

	std::ranges::swap(m_Panels[panel_left], m_Panels[panel_right]);
	m_Panels[panel_left].m_Panel->on_swap();
	m_Panels[panel_right].m_Panel->on_swap();
	filters::SwapPanelFilters();
	m_ActivePanelIndex = IsLeftActive()? panel_right : panel_left;

	SetScreenPosition();
	Global->WindowManager->RefreshWindow();
	return true;
}

long long FilePanels::VMProcess(int OpCode, void* vParam, long long iParam)
{
	if (OpCode == MCODE_F_KEYBAR_SHOW)
	{
		int PrevMode=Global->Opt->ShowKeyBar?2:1;
		switch (iParam)
		{
			case 0:
				break;
			case 1:
				Global->Opt->ShowKeyBar = true;
				m_windowKeyBar->Show();
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
				break;
			case 2:
				Global->Opt->ShowKeyBar = false;
				m_windowKeyBar->Hide();
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
				break;
			case 3:
				ProcessKey(Manager::Key(KEY_CTRLB));
				break;
			default:
				PrevMode=0;
				break;
		}
		return PrevMode;
	}
	return ActivePanel()->VMProcess(OpCode, vParam, iParam);
}

bool FilePanels::ProcessKey(const Manager::Key& Key)
{
	const auto LocalKey = Key();

	// Handle ESC key during resizing to cancel and revert
	if (LocalKey == KEY_ESC)
	{
		if (m_MouseState == MouseState::Resizing)
		{
			// Cancel width resizing and revert to original state
			Global->Opt->WidthDecrement = m_OriginalWidthDecrement;
			m_MouseState = MouseState::None;
			ClearWidthBorderFeedback();
			m_HoverStartTime = 0;
			ResetAllMouseStates();
			SetScreenPosition();
			Global->WindowManager->RefreshWindow();
			return true;
		}
		else if (m_MouseState == MouseState::HeightResizing)
		{
			// Cancel height resizing and revert to original state
			Global->Opt->LeftHeightDecrement = m_OriginalLeftHeightDecrement;
			Global->Opt->RightHeightDecrement = m_OriginalRightHeightDecrement;
			m_MouseState = MouseState::None;
			ClearHeightBorderFeedback();
			m_HeightHoverStartTime = 0;
			ResetAllMouseStates();
			SetScreenPosition();
			Global->WindowManager->RefreshWindow();
			return true;
		}
	}

	if (
		any_of(LocalKey,
			KEY_CTRLLEFT, KEY_CTRLRIGHT, KEY_CTRLNUMPAD4, KEY_CTRLNUMPAD6,
			KEY_RCTRLLEFT, KEY_RCTRLRIGHT, KEY_RCTRLNUMPAD4, KEY_RCTRLNUMPAD6
			/*KEY_CTRLUP, KEY_CTRLDOWN, KEY_CTRLNUMPAD8, KEY_CTRLNUMPAD2,
			KEY_RCTRLUP, KEY_RCTRLDOWN, KEY_RCTRLNUMPAD8, KEY_RCTRLNUMPAD2*/
		) &&
		(!CmdLine->GetString().empty() || (!LeftPanel()->IsVisible() && !RightPanel()->IsVisible())))
	{
		CmdLine->ProcessKey(Key);
		return true;
	}

	bool process_default = false;
	switch (LocalKey)
	{
		case KEY_F1:
		{
			if (!ActivePanel()->ProcessKey(Manager::Key(KEY_F1)))
			{
				help::show(L"Contents"sv);
			}

			return true;
		}
		case KEY_TAB:
		{
			SetAnhoterPanelFocus();
			break;
		}
		case KEY_CTRLF1:
		case KEY_RCTRLF1:
		{
			if (LeftPanel()->IsVisible())
			{
				LeftPanel()->Hide();

				if (RightPanel()->IsVisible())
					SetActivePanel(RightPanel());
			}
			else
			{
				if (!RightPanel()->IsVisible())
					SetActivePanel(LeftPanel());

				LeftPanel()->Show();
			}

			Redraw();
			break;
		}
		case KEY_CTRLF2:
		case KEY_RCTRLF2:
		{
			if (RightPanel()->IsVisible())
			{
				RightPanel()->Hide();

				if (LeftPanel()->IsVisible())
					SetActivePanel(LeftPanel());
			}
			else
			{
				if (!LeftPanel()->IsVisible())
					SetActivePanel(RightPanel());

				RightPanel()->Show();
			}

			Redraw();
			break;
		}
		case KEY_CTRLB:
		case KEY_RCTRLB:
		{
			Global->Opt->ShowKeyBar=!Global->Opt->ShowKeyBar;

			if (!IsKeyBarVisible())
				m_windowKeyBar->Hide();

			SetScreenPosition();
			Global->WindowManager->RefreshWindow();
			break;
		}
		case KEY_CTRLT: case KEY_RCTRLT:
			if (Global->Opt->Tree.TurnOffCompletely)
				break;
			[[fallthrough]];
		case KEY_CTRLL: case KEY_RCTRLL:
		case KEY_CTRLQ: case KEY_RCTRLQ:
		{
			if (ActivePanel()->IsVisible())
			{
				auto AnotherPanel = PassivePanel();
				const auto NewType =
					any_of(LocalKey, KEY_CTRLL, KEY_RCTRLL)?
						panel_type::INFO_PANEL :
						any_of(LocalKey, KEY_CTRLQ, KEY_RCTRLQ)?
							panel_type::QVIEW_PANEL :
							panel_type::TREE_PANEL;

				if (ActivePanel()->GetType() == NewType)
					AnotherPanel = ActivePanel();

				if (!AnotherPanel->ProcessPluginEvent(FE_CLOSE,nullptr))
				{
					if (AnotherPanel->GetType()==NewType)
						/* $ 19.09.2000 IS
						  Повторное нажатие на ctrl-l|q|t всегда включает файловую панель
						*/
						AnotherPanel = ChangePanel(AnotherPanel, panel_type::FILE_PANEL, FALSE, FALSE);
					else
						AnotherPanel=ChangePanel(AnotherPanel,NewType,FALSE,FALSE);

					if (AnotherPanel->GetType() == panel_type::FILE_PANEL || AnotherPanel->GetType() == panel_type::TREE_PANEL)
					{
						// BUGBUG gh-674 make sure to recreate FS watcher
						AnotherPanel->InitCurDir(AnotherPanel->GetCurDir());

						if (ActivePanel() == AnotherPanel)
							os::fs::set_current_directory(AnotherPanel->GetCurDir());
					}
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);

					AnotherPanel->Show();

					ActivePanel()->Show();
				}
			}
			break;
		}
		case KEY_CTRLSHIFTS:
		case KEY_RCTRLSHIFTS:
		{
			process_default = true;
			if (ActivePanel()->IsVisible())
			{
				const auto atype = ActivePanel()->GetType();
				const auto active_redraw = (atype == panel_type::FILE_PANEL || atype == panel_type::INFO_PANEL || atype == panel_type::QVIEW_PANEL);
				bool passive_redraw = false;
				if (PassivePanel()->IsVisible())
				{
					const auto ptype = PassivePanel()->GetType();
					passive_redraw = (ptype == panel_type::FILE_PANEL || ptype == panel_type::INFO_PANEL || ptype == panel_type::QVIEW_PANEL);
				}
				if (active_redraw || passive_redraw)
				{
					process_default = false;
					Global->Opt->ShowBytes = !Global->Opt->ShowBytes;
					if (active_redraw)
						ActivePanel()->Redraw();
					if (passive_redraw)
						PassivePanel()->Redraw();
				}
			}
			break;
		}
		case KEY_CTRLO:
		case KEY_RCTRLO:
		{
			{
				const auto LeftVisible = LeftPanel()->IsVisible();
				const auto RightVisible = RightPanel()->IsVisible();
				const auto HideState = !LeftVisible && !RightVisible;

				if (!HideState)
				{
					m_Panels[panel_left].m_StateBeforeHide = LeftVisible;
					m_Panels[panel_right].m_StateBeforeHide = RightVisible;
					LeftPanel()->Hide();
					RightPanel()->Hide();
					Global->WindowManager->RefreshWindow();
				}
				else
				{
					if (!m_Panels[panel_left].m_StateBeforeHide && !m_Panels[panel_right].m_StateBeforeHide)
						m_Panels[panel_left].m_StateBeforeHide = m_Panels[panel_right].m_StateBeforeHide = TRUE;

					if (m_Panels[panel_left].m_StateBeforeHide)
						LeftPanel()->Show();

					if (m_Panels[panel_right].m_StateBeforeHide)
						RightPanel()->Show();

					if (!ActivePanel()->IsVisible())
					{
						SetActivePanel(IsRightActive()? LeftPanel() : RightPanel());
					}
				}
			}
			break;
		}
		case KEY_CTRLP:
		case KEY_RCTRLP:
		{
			if (ActivePanel()->IsVisible())
			{
				const auto AnotherPanel = PassivePanel();

				if (AnotherPanel->IsVisible())
					AnotherPanel->Hide();
				else
					AnotherPanel->Show();

				CmdLine->Redraw();
			}

			Global->WindowManager->RefreshWindow();
			break;
		}
		case KEY_CTRLI:
		case KEY_RCTRLI:
		{
			ActivePanel()->EditFilter();
			return true;
		}
		case KEY_CTRLU:
		case KEY_RCTRLU:
		{
			if (!LeftPanel()->IsVisible() && !RightPanel()->IsVisible())
				CmdLine->ProcessKey(Key);
			else
				SwapPanels();

			break;
		}
		/* $ 08.04.2002 IS
		   При смене диска установим принудительно текущий каталог на активной
		   панели, т.к. система не знает ничего о том, что у Фара две панели, и
		   текущим для системы после смены диска может быть каталог и на пассивной
		   панели
		*/
		case KEY_ALTF1:
		case KEY_RALTF1:
		{
			ChangeDisk(LeftPanel());

			if (!IsLeftActive())
				ActivePanel()->SetCurPath();

			break;
		}
		case KEY_ALTF2:
		case KEY_RALTF2:
		{
			ChangeDisk(RightPanel());

			if (!IsRightActive())
				ActivePanel()->SetCurPath();

			break;
		}
		case KEY_ALTF7:
		case KEY_RALTF7:
		{
			find_files();
			break;
		}
		case KEY_CTRLUP:  case KEY_CTRLNUMPAD8:
		case KEY_RCTRLUP: case KEY_RCTRLNUMPAD8:
		{
			bool Set=false;
			if (Global->Opt->LeftHeightDecrement<ScrY-7)
			{
				++Global->Opt->LeftHeightDecrement;
				Set=true;
			}
			if (Global->Opt->RightHeightDecrement<ScrY-7)
			{
				++Global->Opt->RightHeightDecrement;
				Set=true;
			}
			if(Set)
			{
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}
		case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
		case KEY_RCTRLDOWN: case KEY_RCTRLNUMPAD2:
		{
			bool Set=false;
			if (Global->Opt->LeftHeightDecrement>0)
			{
				--Global->Opt->LeftHeightDecrement;
				Set=true;
			}
			if (Global->Opt->RightHeightDecrement>0)
			{
				--Global->Opt->RightHeightDecrement;
				Set=true;
			}
			if(Set)
			{
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}

		case KEY_CTRLSHIFTUP:  case KEY_CTRLSHIFTNUMPAD8:
		case KEY_RCTRLSHIFTUP: case KEY_RCTRLSHIFTNUMPAD8:
		{
			IntOption& HeightDecrement = IsLeftActive()? Global->Opt->LeftHeightDecrement : Global->Opt->RightHeightDecrement;
			if (HeightDecrement<ScrY-7)
			{
				++HeightDecrement;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}
			break;
		}

		case KEY_CTRLSHIFTDOWN:  case KEY_CTRLSHIFTNUMPAD2:
		case KEY_RCTRLSHIFTDOWN: case KEY_RCTRLSHIFTNUMPAD2:
		{
			IntOption& HeightDecrement = IsLeftActive()? Global->Opt->LeftHeightDecrement : Global->Opt->RightHeightDecrement;
			if (HeightDecrement>0)
			{
				--HeightDecrement;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}
			break;
		}

		case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
		case KEY_RCTRLLEFT: case KEY_RCTRLNUMPAD4:
		{
			if (Global->Opt->WidthDecrement<ScrX/2-10)
			{
				++Global->Opt->WidthDecrement;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}
		case KEY_CTRLRIGHT:  case KEY_CTRLNUMPAD6:
		case KEY_RCTRLRIGHT: case KEY_RCTRLNUMPAD6:
		{
			if (Global->Opt->WidthDecrement>-(ScrX/2-10))
			{
				--Global->Opt->WidthDecrement;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}
		case KEY_CTRLCLEAR:
		case KEY_RCTRLCLEAR:
		{
			if (Global->Opt->WidthDecrement)
			{
				Global->Opt->WidthDecrement=0;
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}
		case KEY_CTRLALTCLEAR:
		case KEY_RCTRLRALTCLEAR:
		case KEY_CTRLRALTCLEAR:
		case KEY_RCTRLALTCLEAR:
		{
			bool Set=false;
			if (Global->Opt->LeftHeightDecrement)
			{
				Global->Opt->LeftHeightDecrement=0;
				Set=true;
			}
			if (Global->Opt->RightHeightDecrement)
			{
				Global->Opt->RightHeightDecrement=0;
				Set=true;
			}
			if(Set)
			{
				SetScreenPosition();
				Global->WindowManager->RefreshWindow();
			}

			break;
		}
		case KEY_F9:
		{
			Global->Opt->ShellOptions(false,nullptr);
			return true;
		}
		case KEY_SHIFTF10:
		{
			Global->Opt->ShellOptions(true,nullptr);
			return true;
		}
		case KEY_F11:
		{
			// We will never get here in normal flow since F11 is processed by manager itself,
			// but auto-completion menu can forward keys back to the owner
			return Global->WindowManager->ProcessKey(Key);
		}
		default:
		{
			process_default = true;
			break;
		}
	}
	if (process_default)
	{
		if (LocalKey >= KEY_CTRL0 && LocalKey <= KEY_CTRL9)
			ChangePanelViewMode(ActivePanel(), LocalKey - KEY_CTRL0, true);
		if (!ActivePanel()->ProcessKey(Key))
			CmdLine->ProcessKey(Key);
	}

	return true;
}

bool FilePanels::ChangePanelViewMode(panel_ptr Current, int Mode, bool RefreshWindow)
{
	if (Current && Mode >= VIEW_0 && static_cast<size_t>(Mode) < Global->Opt->ViewSettings.size())
	{
		Current->SetViewMode(Mode);
		Current = ChangePanelToFilled(Current, panel_type::FILE_PANEL);
		Current->SetViewMode(Mode);
		// ВНИМАНИЕ! Костыль! Но Работает!
		SetScreenPosition();

		if (RefreshWindow)
			Global->WindowManager->RefreshWindow();

		return true;
	}

	return false;
}

void FilePanels::SetActivePanel(Panel* ToBeActive)
{
	if (ActivePanel().get() != ToBeActive)
	{
		Global->FolderChanged();
		SetPassivePanelInternal(ActivePanel());
		SetActivePanelInternal(ToBeActive->shared_from_this());
	}
}

void FilePanels::SetPassivePanelInternal(panel_ptr ToBePassive)
{
	ToBePassive->OnFocusChange(false);
}

void FilePanels::SetActivePanelInternal(panel_ptr ToBeActive)
{
	m_ActivePanelIndex = IsLeft(ToBeActive)? panel_left : panel_right;

	Global->WindowManager->UpdateMacroArea();

	PassivePanel()->Redraw();
	ToBeActive->OnFocusChange(true);

	FarChDir(ToBeActive->GetCurDir());
	RedrawKeyBar();

	ToBeActive->RefreshTitle();
	ToBeActive->ShowConsoleTitle();
}

panel_ptr FilePanels::ChangePanelToFilled(panel_ptr Current, panel_type NewType)
{
	if (Current->GetType()!=NewType && !Current->ProcessPluginEvent(FE_CLOSE,nullptr))
	{
		Current->Hide();
		Current=ChangePanel(Current,NewType,FALSE,FALSE);

		// BUGBUG gh-674 make sure to recreate FS watcher
		Current->InitCurDir(Current->GetCurDir());

		Current->Update(0);
		Current->Show();
	}

	return Current;
}

panel_ptr FilePanels::GetAnotherPanel(const Panel* Current) const
{
	if (Current==LeftPanel().get())
		return RightPanel();
	else
		return LeftPanel();
}


panel_ptr FilePanels::ChangePanel(panel_ptr Current, panel_type NewType, int CreateNew, int Force)
{
	assert(Current == m_Panels[panel_left].m_Panel || Current == m_Panels[panel_right].m_Panel);

	std::unique_ptr<SaveScreen> TemporarySaveScr;
	// OldType не инициализировался...
	const auto OldType = Current->GetType();
	const auto OldPanelMode = Current->GetMode();

	if (!Force && NewType == OldType && OldPanelMode == panel_mode::NORMAL_PANEL)
		return Current;

	bool UsedLastPanel = false;

	const auto OldViewMode = Current->GetPrevViewMode();
	const auto OldFullScreen = Current->IsFullScreen();
	const auto OldSortMode = Current->GetPrevSortMode();
	const auto OldSortOrder = Current->GetPrevSortOrder();
	const auto OldSortGroups = Current->GetSortGroups();
	const auto OldShowShortNames = Current->GetShowShortNamesMode();
	const auto OldFocus = Current->IsFocused();
	const auto OldSelectedFirst = Current->GetSelectedFirstMode();
	const auto OldDirectoriesFirst = Current->GetPrevDirectoriesFirst();
	const auto LeftPosition = (Current == LeftPanel());

	auto& LastFilePanel = m_Panels[LeftPosition? panel_left : panel_right].m_LastFilePanel;
	const auto Rect = Current->GetPosition();
	const auto ChangePosition = (OldType == panel_type::FILE_PANEL && NewType != panel_type::FILE_PANEL && OldFullScreen) ||
		(NewType==panel_type::FILE_PANEL && OldFullScreen != FileList::IsModeFullScreen(OldViewMode));

	if (OldFocus) SetPassivePanelInternal(Current);
	if (!ChangePosition)
	{
		TemporarySaveScr = std::move(Current->SaveScr);
	}

	Current->OnDestroy();

	if (OldType == panel_type::FILE_PANEL && NewType != panel_type::FILE_PANEL)
	{
		Current->SaveScr.reset();
		Current->Hide();

		LastFilePanel = std::move(Current);

		if (LastFilePanel->SaveScr)
		{
			LastFilePanel->SaveScr->Discard();
			LastFilePanel->SaveScr.reset();
		}
	}
	else
	{
		Current->Hide();
		Current.reset();

		if (OldType == panel_type::FILE_PANEL && NewType == panel_type::FILE_PANEL)
		{
			LastFilePanel.reset();
		}
	}

	auto& NewPanel = m_Panels[LeftPosition? panel_left : panel_right].m_Panel;

	m_Panels[LeftPosition? panel_left : panel_right].m_LastType = OldType;

	if (!CreateNew && NewType == panel_type::FILE_PANEL && LastFilePanel)
	{
		if (LastFilePanel->IsFullScreen())
		{
			const auto LastRect = LastFilePanel->GetPosition();
			LastFilePanel->SetPosition({ LastRect.left, Rect.top, LastRect.right, Rect.bottom });
		}
		else
		{
			LastFilePanel->SetPosition(Rect);
		}

		NewPanel = std::move(LastFilePanel);

		if (!ChangePosition)
		{
			if (NewPanel->IsFullScreen() != OldFullScreen)
			{
				const auto AnotherPanel = GetAnotherPanel(Current);

				if (TemporarySaveScr && AnotherPanel->IsVisible() &&
					AnotherPanel->GetType() == panel_type::FILE_PANEL && AnotherPanel->IsFullScreen())
					TemporarySaveScr->Discard();
			}
			else
				NewPanel->SaveScr = std::move(TemporarySaveScr);
		}

		if (!OldFocus && NewPanel->IsFocused())
			SetActivePanel(PassivePanel());

		UsedLastPanel = true;
	}
	else
	{
		if (CreateNew)
			NewPanel->dispose();

		NewPanel = CreatePanel(NewType);
	}

	if (!UsedLastPanel)
	{
		if (ChangePosition)
		{
			if (LeftPosition)
			{
				NewPanel->SetPosition({ 0, Rect.top, static_cast<int>(ScrX / 2 - Global->Opt->WidthDecrement), Rect.bottom });
				RightPanel()->Redraw();
			}
			else
			{
				NewPanel->SetPosition({ static_cast<int>(ScrX / 2 + 1 - Global->Opt->WidthDecrement), Rect.top, ScrX, Rect.bottom });
				LeftPanel()->Redraw();
			}
		}
		else
		{
			NewPanel->SaveScr = std::move(SaveScr);
			NewPanel->SetPosition(Rect);
		}

		NewPanel->SetSortMode(OldSortMode);
		NewPanel->SetSortOrder(OldSortOrder);
		NewPanel->SetSortGroups(OldSortGroups);
		NewPanel->SetShowShortNamesMode(OldShowShortNames);
		NewPanel->SetPrevViewMode(OldViewMode);
		NewPanel->SetViewMode(OldViewMode);
		NewPanel->SetSelectedFirstMode(OldSelectedFirst);
		NewPanel->SetDirectoriesFirst(OldDirectoriesFirst);
	}

	if (NewPanel->IsFocused()) SetActivePanelInternal(NewPanel);
	return NewPanel;
}

int FilePanels::GetTypeAndName(string &strType, string &strName)
{
	strType = msg(lng::MScreensPanels);

	switch (ActivePanel()->GetType())
	{
	case panel_type::TREE_PANEL:
	case panel_type::QVIEW_PANEL:
	case panel_type::FILE_PANEL:
	case panel_type::INFO_PANEL:
		{
			string strShortName;
			if (ActivePanel()->GetCurName(strName, strShortName))
			{
				auto Directory = ActivePanel()->GetCurDir();
				AddEndSlash(Directory);
				strName.insert(0, Directory);
			}
			else
			{
				strName.clear();
			}
		}
		break;
	}

	return windowtype_panels;
}

void FilePanels::DisplayObject()
{
//  if ( !Focus )
//      return;
	Global->WindowManager->ShowBackground();

	if (Global->Opt->ShowMenuBar)
		TopMenuBar->Show();

	CmdLine->Show();

	if (Global->Opt->ShowKeyBar)
		m_windowKeyBar->Show();
	else if (m_windowKeyBar->IsVisible())
		m_windowKeyBar->Hide();

#if 1

	if (LeftPanel()->IsVisible())
		LeftPanel()->Show();

	if (RightPanel()->IsVisible())
		RightPanel()->Show();

	// Draw the border feedback if currently resizing
	if (m_MouseState == MouseState::Resizing)
	{
		DrawWidthBorderFeedback(false, true);
	}

	// Draw the height border feedback if currently height resizing
	if (m_MouseState == MouseState::HeightResizing)
	{
		DrawHeightBorderFeedback(false, true);
	}

#else
	Panel *PassivePanel=nullptr;
	int PassiveIsLeftFlag=TRUE;

	if (Global->Opt->LeftPanel.Focus)
	{
		ActivePanel=LeftPanel;
		PassivePanel=RightPanel;
		PassiveIsLeftFlag=FALSE;
	}
	else
	{
		ActivePanel=RightPanel;
		PassivePanel=LeftPanel;
		PassiveIsLeftFlag=TRUE;
	}

	//! Вначале "показываем" пассивную панель
	if (PassiveIsLeftFlag)
	{
		if (Global->Opt->LeftPanel.Visible)
		{
			LeftPanel->Show();
		}

		if (Global->Opt->RightPanel.Visible)
		{
			RightPanel->Show();
		}
	}
	else
	{
		if (Global->Opt->RightPanel.Visible)
		{
			RightPanel->Show();
		}

		if (Global->Opt->LeftPanel.Visible)
		{
			LeftPanel->Show();
		}
	}

#endif
}

bool FilePanels::ProcessMouse(const MOUSE_EVENT_RECORD* MouseEvent)
{
	// Cache essential inputs for better performance
	// Defer expensive calculations until needed
	const auto& pos = MouseEvent->dwMousePosition;
	const DWORD  btn = MouseEvent->dwButtonState;
	const DWORD  flags = MouseEvent->dwEventFlags;
	auto& opts = Global->Opt;

	// 1) Far-style title‐bar click
	if (pos.Y == 0)
	{
		if (!opts->ShowColumnTitles)  // Sort Mark letter in the menu area
		{
			if (ActivePanel()->ProcessMouse(MouseEvent))
				return true;
			if (PassivePanel()->ProcessMouse(MouseEvent))
				return true;
		}

		if ((btn & 3) && !flags)
		{
			if (pos.X == 0)
				ProcessKey(Manager::Key(KEY_CTRLO));
			else
				opts->ShellOptions(false, MouseEvent);
			return true;
		}
	}

	// 2) Middle‐click -> ENTER Key
	if (btn & FROM_LEFT_2ND_BUTTON_PRESSED)
	{
		if (!IsMouseButtonEvent(flags))
			return true;

		int Key = KEY_ENTER;
		if (MouseEvent->dwControlKeyState & SHIFT_PRESSED)
		{
			Key |= KEY_SHIFT;
		}
		if (MouseEvent->dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
		{
			Key |= KEY_CTRL;
		}
		if (MouseEvent->dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED))
		{
			Key |= KEY_ALT;
		}
		ProcessKey(Manager::Key(Key));
		return true;
	}

	// 3) Continue/finish a Resizing operation
	if (m_MouseState == MouseState::Resizing)
	{
		const bool leftPressed = (btn & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
		if (leftPressed)
		{
			const int dx = pos.X - m_ResizeStartX;
			const int newDec = m_ResizeStartWidthDecrement - dx;
			const int halfXminus10 = ScrX / 2 - 10;  // Calculate only when needed
			const int clamped = std::clamp(newDec, -halfXminus10, halfXminus10);
			if (opts->WidthDecrement != clamped)
			{
				opts->WidthDecrement = clamped;
				Global->WindowManager->ResizeAllWindows();
			}
		}
		else
		{
			// mouse released -> stop resizing
			m_MouseState = MouseState::None;
			ClearWidthBorderFeedback();
			m_HoverStartTime = 0;
			ResetAllMouseStates();
		}
		return true;
	}

	// 3a) Continue/finish a Height Resizing operation
	if (m_MouseState == MouseState::HeightResizing)
	{
		const bool leftPressed = (btn & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
		if (leftPressed)
		{
			const int dy = m_ResizeStartY - pos.Y;  // Inverted: positive dy means mouse moved up (decrease height)
			const auto leftPanel = LeftPanel();
			const auto rightPanel = RightPanel();
			const auto leftPanelPos = leftPanel->GetPosition();
			const auto rightPanelPos = rightPanel->GetPosition();

			// Determine which panel(s) to resize based on mouse X position at start of resize
			const int startX = m_ResizeStartX;

			// Calculate boundaries for each panel
			const int leftPanelMidX = leftPanelPos.left + (leftPanelPos.right - leftPanelPos.left) / 2;
			const int rightPanelMidX = rightPanelPos.left + (rightPanelPos.right - rightPanelPos.left) / 2;

			// First determine which panel the mouse is over
			const bool overLeftPanel = (startX >= leftPanelPos.left && startX <= leftPanelPos.right);
			const bool overRightPanel = (startX >= rightPanelPos.left && startX <= rightPanelPos.right);

			if (overLeftPanel)
			{
				// Mouse is over left panel
				if (startX <= leftPanelMidX)
				{
					// Left half of left panel - resize left panel only
					const int newLeftDec = m_ResizeStartLeftHeightDecrement + dy;
					const int maxDec = ScrY - 7;
					const int clampedLeft = std::clamp(newLeftDec, 0, maxDec);
					if (opts->LeftHeightDecrement == clampedLeft)
						return true;

					opts->LeftHeightDecrement = clampedLeft;
					SetScreenPosition();
					Global->WindowManager->RefreshWindow();
				}
				else
				{
					// Right half of left panel (inner half, closer to center) - resize both panels
					const int newLeftDec = m_ResizeStartLeftHeightDecrement + dy;
					const int newRightDec = m_ResizeStartRightHeightDecrement + dy;
					const int maxDec = ScrY - 7;
					const int clampedLeft = std::clamp(newLeftDec, 0, maxDec);
					const int clampedRight = std::clamp(newRightDec, 0, maxDec);
					if (opts->LeftHeightDecrement == clampedLeft && opts->RightHeightDecrement == clampedRight)
						return true;

					opts->LeftHeightDecrement = clampedLeft;
					opts->RightHeightDecrement = clampedRight;
					SetScreenPosition();
					Global->WindowManager->RefreshWindow();
				}
			}
			else if (overRightPanel)
			{
				// Mouse is over right panel
				if (startX < rightPanelMidX)
				{
					// Left half of right panel (inner half, closer to center) - resize both panels
					const int newLeftDec = m_ResizeStartLeftHeightDecrement + dy;
					const int newRightDec = m_ResizeStartRightHeightDecrement + dy;
					const int maxDec = ScrY - 7;
					const int clampedLeft = std::clamp(newLeftDec, 0, maxDec);
					const int clampedRight = std::clamp(newRightDec, 0, maxDec);
					if (opts->LeftHeightDecrement == clampedLeft && opts->RightHeightDecrement == clampedRight)
						return true;

					opts->LeftHeightDecrement = clampedLeft;
					opts->RightHeightDecrement = clampedRight;
					SetScreenPosition();
					Global->WindowManager->RefreshWindow();
				}
				else
				{
					// Right half of right panel - resize right panel only
					const int newRightDec = m_ResizeStartRightHeightDecrement + dy;
					const int maxDec = ScrY - 7;
					const int clampedRight = std::clamp(newRightDec, 0, maxDec);
					if (opts->RightHeightDecrement == clampedRight)
						return true;

					opts->RightHeightDecrement = clampedRight;
					SetScreenPosition();
					Global->WindowManager->RefreshWindow();
				}
			}
		}
		else
		{
			// mouse released -> stop height resizing
			m_MouseState = MouseState::None;
			ClearHeightBorderFeedback();
			m_HeightHoverStartTime = 0;
			ResetAllMouseStates();
		}
		return true;
	}

	// 4) Border interaction: hover, double‐click, start resize
	// Check width border first, but only if NOT already in height resizing mode
	const bool overWidthBorder = IsMouseOverPanelInnerBorder(MouseEvent);
	const bool overHeightBorder = IsMouseOverPanelBottomBorder(MouseEvent);

	// Priority: if in height resizing mode, ignore width border
	// If over both borders, prefer height border (bottom border takes priority)
	const bool processWidthBorder = overWidthBorder && !overHeightBorder &&
	                                 m_MouseState != MouseState::HeightHovering &&
	                                 m_MouseState != MouseState::HeightResizing;
	const bool processHeightBorder = overHeightBorder &&
	                                  m_MouseState != MouseState::Hovering &&
	                                  m_MouseState != MouseState::Resizing;

	if (processWidthBorder)
	{
		// 4a) Double‐click resets to center
		if (flags & DOUBLE_CLICK)
		{
			m_MouseState = MouseState::None;
			m_HoverStartTime = 0;
			opts->WidthDecrement = 0;
			SetScreenPosition();
			Global->WindowManager->RefreshWindow();

			// Final flush and reset after the refresh to ensure clean state
			ResetWidthMouseStates();

			// Consume the event completely to prevent propagation to panels
			return true;
		}

		// 4b) Hover (no button pressed)
		const bool leftPressed = (btn & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
		if (!leftPressed)
		{
			const DWORD now = GetTickCount();
			if (m_MouseState == MouseState::None)
			{
				if (m_HoverStartTime == 0)
				{
					m_HoverStartTime = now;
					return true;
				}

				const int hoverThreshold = 300;
				if (now - m_HoverStartTime < hoverThreshold)
					return true;

				m_MouseState = MouseState::Hovering;
				DrawWidthBorderFeedback(true, false);
			}
			else if (m_MouseState == MouseState::Hovering)
			{
				DrawWidthBorderFeedback(true, false);
			}
			return true;
		}

		// 4c) Begin resize on first click (no movement yet)
		if (m_MouseState == MouseState::Hovering)
		{
			const bool moved = (flags & MOUSE_MOVED) != 0;
			if (!moved)
			{
				m_MouseState = MouseState::Resizing;
				m_ResizeStartX = pos.X;
				m_ResizeStartWidthDecrement = opts->WidthDecrement;
				m_OriginalWidthDecrement = opts->WidthDecrement; // Save for ESC cancellation
				m_HoverStartTime = 0;

				// No ResetWidthMouseStates() here, as we are starting the resize
				return true;
			}
		}
	}
	else if (m_MouseState == MouseState::Hovering && !overWidthBorder)
	{
		// left border, but moved off -> clear hover
		m_MouseState = MouseState::None;
		ClearWidthBorderFeedback();
		m_HoverStartTime = 0;
		ResetWidthMouseStates();
	}

	// 4d) Height border interaction: hover, double‐click, start resize
	if (processHeightBorder)
	{
		// 4d1) Double‐click resets to full height
		if (flags & DOUBLE_CLICK)
		{
			m_MouseState = MouseState::None;
			m_HeightHoverStartTime = 0;
			opts->LeftHeightDecrement = 0;
			opts->RightHeightDecrement = 0;
			SetScreenPosition();
			Global->WindowManager->RefreshWindow();

			// Final flush and reset after the refresh to ensure clean state
			ResetHeightMouseStates();

			// Consume the event completely to prevent propagation to panels
			return true;
		}

		// 4d2) Hover (no button pressed)
		const bool leftPressed = (btn & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
		if (!leftPressed)
		{
			const DWORD now = GetTickCount();
			if (m_MouseState == MouseState::None)
			{
				if (m_HeightHoverStartTime == 0)
				{
					m_HeightHoverStartTime = now;
					return true;
				}

				const int hoverThreshold = 300;
				if (now - m_HeightHoverStartTime < hoverThreshold)
					return true;

				m_MouseState = MouseState::HeightHovering;
				DrawHeightBorderFeedback(true, false);
			}
			else if (m_MouseState == MouseState::HeightHovering)
			{
				DrawHeightBorderFeedback(true, false);
			}
			return true;
		}

		// 4d3) Begin height resize on first click (no movement yet)
		if (m_MouseState == MouseState::HeightHovering)
		{
			const bool moved = (flags & MOUSE_MOVED) != 0;
			if (!moved)
			{
				m_MouseState = MouseState::HeightResizing;
				m_ResizeStartX = pos.X;  // Store X position to determine which panel(s) to resize
				m_ResizeStartY = pos.Y;
				m_ResizeStartLeftHeightDecrement = opts->LeftHeightDecrement;
				m_ResizeStartRightHeightDecrement = opts->RightHeightDecrement;
				m_OriginalLeftHeightDecrement = opts->LeftHeightDecrement; // Save for ESC cancellation
				m_OriginalRightHeightDecrement = opts->RightHeightDecrement; // Save for ESC cancellation
				m_HeightHoverStartTime = 0;

				// No ResetHeightMouseStates() here, as we are starting the resize
				return true;
			}
		}
	}
	else if (m_MouseState == MouseState::HeightHovering && !overHeightBorder)
	{
		// left height border, but moved off -> clear hover
		m_MouseState = MouseState::None;
		ClearHeightBorderFeedback();
		m_HeightHoverStartTime = 0;
		ResetHeightMouseStates();
	}

	// 5) Additional safeguard: Don't process double-clicks that might have been 
	// intended for border but are now over panel area due to resize
	if ((flags & DOUBLE_CLICK) && IsMouseOverPanelInnerBorder(MouseEvent))
	{
		// This is a double-click that should have been handled by border logic above
		// but somehow reached here. Consume it to prevent unwanted panel actions.
		return true;
	}

	if (!ActivePanel()->ProcessMouse(MouseEvent))
	{
		if (!PassivePanel()->ProcessMouse(MouseEvent) &&
			!m_windowKeyBar->ProcessMouse(MouseEvent))
		{
			CmdLine->ProcessMouse(MouseEvent);
		}

		ActivePanel()->SetCurPath();
	}

	return true;
}

bool FilePanels::IsMouseOverPanelInnerBorder(const MOUSE_EVENT_RECORD *MouseEvent) const
{
	const auto MouseX = MouseEvent->dwMousePosition.X;
	const auto MouseY = MouseEvent->dwMousePosition.Y;

	// Cache frequently accessed values
	const auto& opt = Global->Opt;
	const auto BorderX = ScrX / 2 - opt->WidthDecrement;

	// Check a 2-column wide area for the border between panels
	if (!(MouseX >= BorderX && MouseX <= BorderX + 1))
		return false;

	// Exclude scroll bar areas from border detection to give scroll bars priority
	// But allow 1 additional row below the scrollbar as grip area for width resizing
	// Somehow it works only for hightlighting - need to fix
	if (opt->ShowPanelScrollbar)
	{
		const auto leftPanel = LeftPanel();
		const auto leftPanelPos = leftPanel->GetPosition();

	// Left panel - border (BorderX) is on the right side - we need to avoid the scrollbar area
	if (!leftPanel->IsVisible() || MouseX != BorderX)
		return true;

	if (MouseX != leftPanelPos.right || leftPanel->GetType() != panel_type::FILE_PANEL)
		return true;

	const auto filePanel = std::dynamic_pointer_cast<FileList>(leftPanel);
	if (!filePanel)
		return true;

	const int height = leftPanelPos.bottom - leftPanelPos.top - 1;
	if (filePanel->GetFileCount() <= static_cast<size_t>(height))
		return true;

	const auto scrollBarStartY = leftPanelPos.top + 1 + (opt->ShowColumnTitles ? 1 : 0);
	// There is conflict with scrollbar area, let's grab one cell from it for the  better grip
	const auto scrollBarEndY = scrollBarStartY + (height - (opt->ShowColumnTitles ? 1 : 0)) - 1;
	if (MouseY >= scrollBarStartY && MouseY < scrollBarEndY)
		return false; // This is scroll bar area, not border for width resizing

	// Right panel - scrollbar area doesn't conflict with border (BorderX + 1) - do nothing

		return true; // not scrollbar area or grip area above/below scrollbar
	}

	return true;
}

bool FilePanels::IsMouseOverPanelBottomBorder(const MOUSE_EVENT_RECORD *MouseEvent) const
{
	const auto MouseX = MouseEvent->dwMousePosition.X;
	const auto MouseY = MouseEvent->dwMousePosition.Y;

	// Cache frequently accessed values
	const auto leftPanel = LeftPanel();
	const auto rightPanel = RightPanel();
	const auto leftPanelPos = leftPanel->GetPosition();
	const auto rightPanelPos = rightPanel->GetPosition();

	// Check if mouse is over the bottom border of either panel
	const auto leftBottomY = leftPanelPos.bottom;
	const auto rightBottomY = rightPanelPos.bottom;

	// Left panel bottom border
	if (leftPanel->IsVisible() && MouseY == leftBottomY &&
		MouseX >= leftPanelPos.left && MouseX <= leftPanelPos.right)
	{
		return true;
	}

	// Right panel bottom border
	if (rightPanel->IsVisible() && MouseY == rightBottomY &&
		MouseX >= rightPanelPos.left && MouseX <= rightPanelPos.right)
	{
		return true;
	}

	return false;
}

FarColor FilePanels::GetBorderFeedbackColor()
{
	// Get the dragging border color first - this respects user's custom color choices
	auto Result = colors::PaletteColorToFarColor(COL_PANELDRAGBORDER);
	const auto& DragTextColor = colors::PaletteColorToFarColor(COL_PANELDRAGTEXT);
	const auto& SelectedTextColor = colors::PaletteColorToFarColor(COL_PANELSELECTEDTEXT);
	const auto& PanelTextColor = colors::PaletteColorToFarColor(COL_PANELTEXT);

	// Check if the dragging border color is using the default palette value
	// Default palette value for COL_PANELDRAGBORDER is F_YELLOW|B_BLUE
	const auto DefaultDragBorderColor = F_YELLOW | B_BLUE;

	// If the current color matches the default palette value, or if it seems uninitialized,
	// apply the fallback logic (theme doesn't define this color or user hasn't customized it)
	if (const auto CurrentDragBorderRaw = (Result.ForegroundColor & 0x0F) | ((Result.BackgroundColor & 0x0F) << 4);
		CurrentDragBorderRaw == DefaultDragBorderColor ||
		Result.ForegroundColor == Result.BackgroundColor ||
		Result.ForegroundColor == 0)
	{
		// Apply fallback logic: foreground from dragging text (or selected text), background from panel
		Result.ForegroundColor = DragTextColor.ForegroundColor != 0 ? DragTextColor.ForegroundColor : SelectedTextColor.ForegroundColor;
		Result.BackgroundColor = PanelTextColor.BackgroundColor;
	}

	return Result;
}

void FilePanels::DrawWidthBorderFeedback(bool IsHovering, bool IsDragging)
{
	// Show visual feedback on hover and during dragging
	if (!IsHovering && !IsDragging)
		return;

	// Cache frequently accessed pointers and values
	const auto leftPanel = LeftPanel();
	const auto rightPanel = RightPanel();
	const auto leftPanelPos = leftPanel->GetPosition();
	const auto rightPanelPos = rightPanel->GetPosition();
	const auto& opt = Global->Opt;

	const auto BorderX = ScrX / 2 - opt->WidthDecrement;
	const auto borderX = static_cast<int>(BorderX);

	const auto BorderColor = GetBorderFeedbackColor();

	static constexpr wchar_t BorderChar[] = L"║";

	// Draw full height border feedback
	const auto StartY = 1;
	const auto EndY = ScrY - 2;

	const int borderXPlus = borderX + 1;
	const bool drawLeft = leftPanel->IsVisible();
	const bool drawRight = rightPanel->IsVisible() && borderX < ScrX - 1;

	// Calculate actual drawing boundaries based on panel heights
	const auto leftPanelStartY = std::max(StartY, static_cast<int>(leftPanelPos.top + 1));
	const auto leftPanelEndY = std::min(EndY, static_cast<int>(leftPanelPos.bottom));
	const auto rightPanelStartY = std::max(StartY, static_cast<int>(rightPanelPos.top + 1));
	const auto rightPanelEndY = std::min(EndY, static_cast<int>(rightPanelPos.bottom));

	// Helper function to get scroll bar boundaries for a panel
	auto getScrollBarBounds = [&](const panel_ptr& panel) -> std::tuple<bool, int, int> {
		if (!opt->ShowPanelScrollbar || !panel->IsVisible() || panel->GetType() != panel_type::FILE_PANEL)
			return {false, 0, 0};

		const auto filePanel = std::dynamic_pointer_cast<FileList>(panel);
		if (!filePanel) return {false, 0, 0};

		const auto panelPos = panel->GetPosition();
		const bool hasScrollBar = filePanel->GetFileCount() > static_cast<size_t>(panelPos.bottom - panelPos.top - 1);

		if (!hasScrollBar) return {false, 0, 0};

		const auto scrollBarStartY = panelPos.top + 1 + (opt->ShowColumnTitles ? 1 : 0);
		// There is conflict with scrollbar area, let's grab one cell from it for the  better grip
		const auto scrollBarEndY = scrollBarStartY + (panelPos.bottom - panelPos.top - 2 - (opt->ShowColumnTitles ? 1 : 0)) - 1;

		return {true, scrollBarStartY, scrollBarEndY};
	};

	const auto [leftHasScrollBar, leftScrollBarStart, leftScrollBarEnd] = getScrollBarBounds(LeftPanel());

	// Draw borders for full height, respecting panel boundaries and scrollbars
	for (int y = StartY; y < EndY; ++y)
	{
		if (drawLeft && y >= leftPanelStartY && y < leftPanelEndY)
		{
			bool skipForScrollBar = leftHasScrollBar && (borderX == leftPanelPos.right) &&
			                       (y >= leftScrollBarStart && y < leftScrollBarEnd);

			if (!skipForScrollBar)
			{
				GotoXY(borderX, y);
				Text({ borderX, y }, BorderColor, BorderChar);
			}
		}
		if (drawRight && y >= rightPanelStartY && y < rightPanelEndY)
		{
			GotoXY(borderXPlus, y);
			Text({ borderXPlus, y }, BorderColor, BorderChar);
		}
	}

	m_LastWidthFeedbackY = StartY; // Mark that width feedback is drawn
}

void FilePanels::ClearWidthBorderFeedback()
{
	if (m_LastWidthFeedbackY == -1)
		return;

	// Clear feedback by refreshing the window
	Global->WindowManager->RefreshWindow();
	m_LastWidthFeedbackY = -1;
}

std::pair<bool, bool> FilePanels::DetermineHeightHighlightPanels(bool IsDragging, const rectangle& leftPanelPos, const rectangle& rightPanelPos)
{
	const int mouseX = IsDragging ? m_ResizeStartX : IntKeyState.MousePos.x;
	const int leftPanelMidX = leftPanelPos.left + (leftPanelPos.right - leftPanelPos.left) / 2;
	const int rightPanelMidX = rightPanelPos.left + (rightPanelPos.right - rightPanelPos.left) / 2;

	bool highlightLeft = false;
	bool highlightRight = false;

	const bool overLeftPanel = (mouseX >= leftPanelPos.left && mouseX <= leftPanelPos.right);
	const bool overRightPanel = (mouseX >= rightPanelPos.left && mouseX <= rightPanelPos.right);

	if (overLeftPanel)
	{
		if (mouseX <= leftPanelMidX)
		{
			highlightLeft = true;
		}
		else
		{
			highlightLeft = true;
			highlightRight = true;
		}
	}
	else if (overRightPanel)
	{
		if (mouseX < rightPanelMidX)
		{
			highlightLeft = true;
			highlightRight = true;
		}
		else
		{
			highlightRight = true;
		}
	}

	return { highlightLeft, highlightRight };
}

void FilePanels::DrawPanelBottomBorder(const std::shared_ptr<Panel>& panel, const rectangle& panelPos, const FarColor& BorderColor, const wchar_t* BorderChar)
{
	if (!panel->IsVisible())
		return;

	const auto bottomY = panelPos.bottom;
	int statusStartX = -1, statusEndX = -1;

	if (const auto fileListPanel = std::dynamic_pointer_cast<FileList>(panel))
	{
		fileListPanel->GetBottomStatusTextBounds(statusStartX, statusEndX);
	}

	if (statusStartX != -1 && statusEndX != -1)
	{
		// Draw border segments around status text
		for (int x = panelPos.left + 1; x < statusStartX; ++x)
		{
			GotoXY(x, bottomY);
			Text({ x, bottomY }, BorderColor, BorderChar);
		}

		for (int x = statusEndX + 3; x < panelPos.right; ++x)
		{
			GotoXY(x, bottomY);
			Text({ x, bottomY }, BorderColor, BorderChar);
		}
	}
	else
	{
		// Draw full bottom border (except corners)
		for (int x = panelPos.left + 1; x < panelPos.right; ++x)
		{
			GotoXY(x, bottomY);
			Text({ x, bottomY }, BorderColor, BorderChar);
		}
	}
}

void FilePanels::DrawHeightBorderFeedback(bool IsHovering, bool IsDragging)
{
	if (!IsHovering && !IsDragging)
		return;

	const auto leftPanel = LeftPanel();
	const auto rightPanel = RightPanel();
	const auto leftPanelPos = leftPanel->GetPosition();
	const auto rightPanelPos = rightPanel->GetPosition();

	const auto BorderColor = GetBorderFeedbackColor();
	static constexpr wchar_t BorderChar[] = L"═";

	const auto [highlightLeft, highlightRight] = DetermineHeightHighlightPanels(IsDragging, leftPanelPos, rightPanelPos);

	if (highlightLeft)
		DrawPanelBottomBorder(leftPanel, leftPanelPos, BorderColor, BorderChar);

	if (highlightRight)
		DrawPanelBottomBorder(rightPanel, rightPanelPos, BorderColor, BorderChar);

	m_LastHeightFeedbackX = leftPanelPos.left; // Mark that height feedback is drawn
}
void FilePanels::ClearHeightBorderFeedback()
{
	if (m_LastHeightFeedbackX == -1)
		return;

	// Clear feedback by refreshing the window
	Global->WindowManager->RefreshWindow();
	m_LastHeightFeedbackX = -1;
}

void FilePanels::ShowConsoleTitle()
{
	ActivePanel()->RefreshTitle();
	ActivePanel()->ShowConsoleTitle();
}

void FilePanels::ResizeConsole()
{
	window::ResizeConsole();

	SetScreenPosition();

	CmdLine->ResizeConsole();
	m_windowKeyBar->ResizeConsole();
	TopMenuBar->ResizeConsole();
}

bool FilePanels::CanFastHide() const
{
	return (Global->Opt->AllCtrlAltShiftRule & CASR_PANEL) != 0;
}

void FilePanels::GoToFile(const string_view FileName)
{
	if (FindSlash(FileName) != string::npos)
	{
		string ADir,PDir;

		const auto PassiveMode = PassivePanel()->GetMode();

		if (PassiveMode == panel_mode::NORMAL_PANEL)
		{
			PDir = PassivePanel()->GetCurDir();
			AddEndSlash(PDir);
		}

		const auto ActiveMode = ActivePanel()->GetMode();

		if (ActiveMode == panel_mode::NORMAL_PANEL)
		{
			ADir = ActivePanel()->GetCurDir();
			AddEndSlash(ADir);
		}

		const auto strNameFile = PointToName(FileName);
		string_view NameDir(FileName);
		CutToSlash(NameDir);
		/* $ 10.04.2001 IS
		     Не делаем SetCurDir, если нужный путь уже есть на открытых
		     панелях, тем самым добиваемся того, что выделение с элементов
		     панелей не сбрасывается.
		*/
		const auto AExist = (ActiveMode == panel_mode::NORMAL_PANEL) && equal_icase(ADir, NameDir);
		const auto PExist = (PassiveMode == panel_mode::NORMAL_PANEL) && equal_icase(PDir, NameDir);

		// если нужный путь есть на пассивной панели
		if (!AExist && PExist)
			ProcessKey(Manager::Key(KEY_TAB));

		if (!AExist && !PExist)
			ActivePanel()->SetCurDir(NameDir, true);

		ActivePanel()->GoToFile(strNameFile);
		// всегда обновим заголовок панели, чтобы дать обратную связь, что
		// Ctrl-F10 обработан
		ActivePanel()->RefreshTitle();
	}
}


FARMACROAREA FilePanels::GetMacroArea() const
{
	switch (ActivePanel()->GetType())
	{
	case panel_type::FILE_PANEL:
		return MACROAREA_SHELL;
	case panel_type::TREE_PANEL:
		return MACROAREA_TREEPANEL;
	case panel_type::QVIEW_PANEL:
		return MACROAREA_QVIEWPANEL;
	case panel_type::INFO_PANEL:
		return MACROAREA_INFOPANEL;
	}
	return MACROAREA_INVALID;
}

bool FilePanels::IsKeyBarVisible() const
{
	return Global->Opt->ShowKeyBar;
}

Viewer* FilePanels::GetViewer()
{
	auto result=ActivePanel()->GetViewer();
	if (!result) result=PassivePanel()->GetViewer();
	return result;
}

Viewer* FilePanels::GetById(int ID)
{
	auto result=LeftPanel()->GetById(ID);
	if (!result) result=RightPanel()->GetById(ID);
	return result;
}

CommandLine* FilePanels::GetCmdLine() const
{
	return CmdLine.get();
}

void FilePanels::Show()
{
	window::Show();

	if (Global->Opt->ShowMenuBar)
	{
		TopMenuBar->Show();
	}
}

void FilePanels::ResetAllMouseStates()
{
	FlushInputBuffer();

	// Reset Far's internal mouse state tracking
	IntKeyState.MouseButtonState = 0;
	IntKeyState.PrevMouseButtonState = 0;
	IntKeyState.PrevLButtonPressed = false;
	IntKeyState.PrevRButtonPressed = false;
	IntKeyState.PrevMButtonPressed = false;
	IntKeyState.MouseEventFlags = 0;
	IntKeyState.PreMouseEventFlags = 0;

	// Reset drag-and-drop state
	Panel::EndDrag();
}

void FilePanels::ResetWidthMouseStates() const
{
	// Reset width-specific states only
	m_HoverStartTime = 0;
}

void FilePanels::ResetHeightMouseStates() const
{
	// Reset height-specific states only
	m_HeightHoverStartTime = 0;
	m_LastHeightFeedbackX = -1;
	m_ResizeStartY = 0;
	m_ResizeStartLeftHeightDecrement = 0;
	m_ResizeStartRightHeightDecrement = 0;
}
