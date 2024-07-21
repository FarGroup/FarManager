/*
cmdline.cpp

Командная строка
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
#include "cmdline.hpp"

// Internal:
#include "execute.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "history.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "foldtree.hpp"
#include "treelist.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "scrbuf.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "usermenu.hpp"
#include "datetime.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "console.hpp"
#include "panelmix.hpp"
#include "network.hpp"
#include "plugins.hpp"
#include "colormix.hpp"
#include "constitle.hpp"
#include "lang.hpp"
#include "components.hpp"
#include "desktop.hpp"
#include "keybar.hpp"
#include "string_utils.hpp"
#include "farversion.hpp"
#include "global.hpp"
#include "log.hpp"
#include "char_width.hpp"
#include "stddlg.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.fs.hpp"
#include "platform.security.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/enum_substrings.hpp"
#include "common/from_string.hpp"
#include "common/function_traits.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

enum
{
	FCMDOBJ_LOCKUPDATEPANEL = 16_bit,
	DEFAULT_CMDLINE_WIDTH = 50,
};

CommandLine::CommandLine(window_ptr Owner):
	SimpleScreenObject(Owner),
	PromptSize(DEFAULT_CMDLINE_WIDTH),
	CmdStr(
		Owner,
		this,
		[](const Manager::Key& Key){ return Global->CtrlObject->Cp()->ProcessKey(Key); },
		nullptr,
		Global->CtrlObject->CmdHistory.get(),
		nullptr,
		(Global->Opt->CmdLine.AutoComplete ? EditControl::EC_ENABLEAUTOCOMPLETE : 0) | EditControl::EC_COMPLETE_HISTORY | EditControl::EC_COMPLETE_FILESYSTEM | EditControl::EC_COMPLETE_PATH | EditControl::EC_COMPLETE_ENVIRONMENT
	),
	LastCmdPartLength(-1)
{
	CmdStr.SetEditBeyondEnd(false);
	CmdStr.SetMacroAreaAC(MACROAREA_SHELLAUTOCOMPLETION);
	SetPersistentBlocks(Global->Opt->CmdLine.EditBlock);
	SetDelRemovesBlocks(Global->Opt->CmdLine.DelRemovesBlocks);
}

CommandLine::~CommandLine() = default;

void CommandLine::SetPersistentBlocks(bool Mode)
{
	CmdStr.SetPersistentBlocks(Mode);
}

void CommandLine::SetDelRemovesBlocks(bool Mode)
{
	CmdStr.SetDelRemovesBlocks(Mode);
}

void CommandLine::SetAutoComplete(int Mode)
{
	CmdStr.SetAutocomplete(Mode!=0);
}

size_t CommandLine::DrawPrompt()
{
	const auto PromptList = GetPrompt();
	const size_t MaxLength = PromptSize*ObjWidth() / 100;

	std::vector<size_t> Sizes;
	Sizes.reserve(PromptList.size());

	size_t PromptLength = 0;
	size_t FixedLength = 0;
	size_t CollapsibleCount = 0;
	for (const auto& i: PromptList)
	{
		Sizes.emplace_back(i.Text.size());
		PromptLength += i.Text.size();
		if (i.Collapsible)
			++CollapsibleCount;
		else
			FixedLength += i.Text.size();
	}

	size_t CollapsibleItemLength = 0;
	bool TryCollapse = false;
	if (PromptLength > MaxLength)
	{
		if (CollapsibleCount)
		{
			if (FixedLength < MaxLength)
				CollapsibleItemLength = (MaxLength - FixedLength) / CollapsibleCount;
			TryCollapse = true;
		}
	}

	size_t CurLength = 0;
	GotoXY(m_Where.left, m_Where.top);

	for (const auto& i: PromptList)
	{
		auto str = i.Text;

		if (TryCollapse && i.Collapsible)
		{
			inplace::truncate_path(str, CollapsibleItemLength);
		}

		if (CurLength + str.size() > MaxLength)
			inplace::truncate_path(str, std::max(0uz, MaxLength - CurLength));
		SetColor(i.Colour);
		Text(str);
		CurLength += str.size();

		if (CurLength >= MaxLength)
			break;
	}
	return CurLength;
}

void CommandLine::DisplayObject()
{
	const auto CurLength = DrawPrompt();

	CmdStr.SetObjectColor(COL_COMMANDLINE,COL_COMMANDLINESELECTED);
	CmdStr.SetPosition({ m_Where.left + static_cast<int>(CurLength), m_Where.top, m_Where.right, m_Where.bottom });
	CmdStr.Show();

	GotoXY(m_Where.right + 1, m_Where.top);
	SetColor(COL_COMMANDLINEPREFIX);
	Text(L'↑');
}

void CommandLine::DrawFakeCommand(string_view const FakeCommand)
{
	console.SetCursorPosition({ m_Where.left, m_Where.top });
	console.start_prompt();
	DrawPrompt();

	console.SetCursorPosition({ WhereX(), WhereY() });
	console.start_command();
	SetColor(COL_COMMANDLINE);
	// TODO: wrap & scroll if too long
	Text(FakeCommand);
}

void CommandLine::SetCurPos(int Pos, int LeftPos, bool Redraw)
{
	CmdStr.SetLeftPos(LeftPos);
	CmdStr.SetCurPos(Pos);
	//CmdStr.AdjustMarkBlock();

	if (Redraw)
		CmdStr.Redraw();
}

long long CommandLine::VMProcess(int OpCode, void* vParam, long long iParam)
{
	if (OpCode >= MCODE_C_CMDLINE_BOF && OpCode <= MCODE_C_CMDLINE_SELECTED)
		return CmdStr.VMProcess(OpCode-MCODE_C_CMDLINE_BOF+MCODE_C_BOF,vParam,iParam);

	if (OpCode >= MCODE_C_BOF && OpCode <= MCODE_C_SELECTED)
		return CmdStr.VMProcess(OpCode,vParam,iParam);

	if (OpCode == MCODE_V_ITEMCOUNT || OpCode == MCODE_V_CURPOS)
		return CmdStr.VMProcess(OpCode,vParam,iParam);

	if (OpCode == MCODE_V_CMDLINE_ITEMCOUNT || OpCode == MCODE_V_CMDLINE_CURPOS)
		return CmdStr.VMProcess(OpCode-MCODE_V_CMDLINE_ITEMCOUNT+MCODE_V_ITEMCOUNT,vParam,iParam);

	if (OpCode == MCODE_F_EDITOR_SEL)
		return CmdStr.VMProcess(MCODE_F_EDITOR_SEL,vParam,iParam);

	return 0;
}

void CommandLine::ResizeConsole()
{
	CmdStr.SetPosition({ CmdStr.GetPosition().left, m_Where.top, m_Where.right, m_Where.bottom });
	CmdStr.ResizeConsole();
}

bool CommandLine::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key;

	if (any_of(LocalKey(), KEY_CTRLEND, KEY_RCTRLEND, KEY_CTRLNUMPAD1, KEY_RCTRLNUMPAD1) && CmdStr.GetCurPos() == CmdStr.GetLength())
	{
		if (LastCmdPartLength==-1)
			strLastCmdStr = CmdStr.GetString();

		auto strStr = strLastCmdStr;
		const auto CurCmdPartLength = strStr.size();
		Global->CtrlObject->CmdHistory->GetSimilar(strStr,LastCmdPartLength);

		if (LastCmdPartLength==-1)
		{
			strLastCmdStr = CmdStr.GetString();
			LastCmdPartLength = static_cast<int>(CurCmdPartLength);
		}

		{
			SCOPED_ACTION(SetAutocomplete)(&CmdStr);
			CmdStr.SetString(strStr);
			CmdStr.Select(LastCmdPartLength,static_cast<int>(strStr.size()));
		}

		Show();
		return true;
	}

	if (any_of(LocalKey(), KEY_UP, KEY_NUMPAD8))
	{
		if (Global->CtrlObject->Cp()->LeftPanel()->IsVisible() || Global->CtrlObject->Cp()->RightPanel()->IsVisible())
			return false;

		LocalKey=KEY_CTRLE;
	}
	else if (any_of(LocalKey(), KEY_DOWN, KEY_NUMPAD2))
	{
		if (Global->CtrlObject->Cp()->LeftPanel()->IsVisible() || Global->CtrlObject->Cp()->RightPanel()->IsVisible())
			return false;

		LocalKey=KEY_CTRLX;
	}

	// $ 25.03.2002 VVM + При погашенных панелях колесом крутим историю
	if (!Global->CtrlObject->Cp()->LeftPanel()->IsVisible() && !Global->CtrlObject->Cp()->RightPanel()->IsVisible())
	{
		switch (LocalKey())
		{
			case KEY_MSWHEEL_UP:    LocalKey = KEY_CTRLE; break;
			case KEY_MSWHEEL_DOWN:  LocalKey = KEY_CTRLX; break;
			case KEY_MSWHEEL_LEFT:  LocalKey = KEY_CTRLS; break;
			case KEY_MSWHEEL_RIGHT: LocalKey = KEY_CTRLD; break;
		}
	}

	switch (LocalKey())
	{
		case KEY_CTRLE:
		case KEY_RCTRLE:
		case KEY_CTRLX:
		case KEY_RCTRLX:
			{
				if (Global->CtrlObject->CmdHistory->IsOnTop())
				{
					m_CurCmdStr = CmdStr.GetString();
				}

				SCOPED_ACTION(SetAutocomplete)(&CmdStr);
				const auto strStr = any_of(LocalKey(), KEY_CTRLE, KEY_RCTRLE)?
					Global->CtrlObject->CmdHistory->GetPrev() :
					Global->CtrlObject->CmdHistory->GetNext();
				SetString(Global->CtrlObject->CmdHistory->IsOnTop()? m_CurCmdStr : strStr, true);
			}
			return true;

		case KEY_ESC:
		{
			// $ 24.09.2000 SVS - Если задано поведение по "Несохранению при Esc", то позицию в хистори не меняем и ставим в первое положение.
			if (Global->Opt->CmdHistoryRule)
				Global->CtrlObject->CmdHistory->ResetPosition();
			SetString({}, true);
			return true;
		}

		case KEY_F2:
		{
			user_menu(false);
			return true;
		}
		case KEY_ALTF8:
		case KEY_RALTF8:
		{
			history_record_type Type;
			string strStr;
			const auto SelectType = Global->CtrlObject->CmdHistory->Select(msg(lng::MHistoryTitle), L"History"sv, strStr, Type);
			if (SelectType == HRT_ENTER || SelectType == HRT_SHIFTENTER || SelectType == HRT_CTRLENTER || SelectType == HRT_CTRLALTENTER)
			{
				std::optional<SetAutocomplete> disable;
				if(SelectType != HRT_CTRLENTER)
				{
					disable.emplace(&CmdStr);
				}
				SetString(strStr, true);

				if (SelectType != HRT_CTRLENTER)
				{
					ProcessKey(SelectType == HRT_CTRLALTENTER? Manager::Key(KEY_CTRLALTENTER) : (SelectType == HRT_ENTER? Manager::Key(KEY_ENTER) : Manager::Key(KEY_SHIFTENTER)));
				}
			}
		}
		return true;

		case KEY_ALTF10:
		case KEY_RALTF10:
		if (!Global->Opt->Tree.TurnOffCompletely)
		{
			string strStr;
			const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
			{
				// TODO: здесь можно добавить проверку, что мы в корне диска и отсутствие файла Tree.Far...
				FolderTree::create(strStr, MODALTREE_ACTIVE, TRUE, false);
			}
			Global->CtrlObject->Cp()->RedrawKeyBar();

			if (!strStr.empty())
			{
				ActivePanel->SetCurDir(strStr,true);
				ActivePanel->Show();

				if (ActivePanel->GetType() == panel_type::TREE_PANEL)
					ActivePanel->ProcessKey(Manager::Key(KEY_ENTER));
			}
			else
			{
				// TODO: ... а здесь проверить факт изменения/появления файла Tree.Far и мы опять же в корне (чтобы лишний раз не апдейтить панель)
				ActivePanel->Update(UPDATE_KEEP_SELECTION);
				ActivePanel->Redraw();
				const auto AnotherPanel = Global->CtrlObject->Cp()->PassivePanel();

				if (AnotherPanel->NeedUpdatePanel(ActivePanel.get()))
				{
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);//|UPDATE_SECONDARY);
					AnotherPanel->Redraw();
				}
			}
		}
		return true;

		case KEY_ALTF11:
		case KEY_RALTF11:
			ShowViewEditHistory();
			Global->CtrlObject->Cp()->Redraw();
			return true;

		case KEY_ALTF12:
		case KEY_RALTF12:
		{
			history_record_type Type;
			UUID Uuid;
			string strFile, strData, strStr;

			switch(const auto SelectType = Global->CtrlObject->FolderHistory->Select(msg(lng::MFolderHistoryTitle), L"HistoryFolders"sv, strStr, Type, &Uuid, &strFile, &strData))
			{
			case HRT_ENTER:
			case HRT_SHIFTENTER:
			case HRT_CTRLSHIFTENTER:
				{
					auto Suppressor = Global->CtrlObject->FolderHistory->suppressor();
					if (SelectType != HRT_SHIFTENTER)
						Suppressor.reset();

					// пусть плагин сам прыгает... ;-)
					auto Panel = Global->CtrlObject->Cp()->ActivePanel();

					if (SelectType == HRT_CTRLSHIFTENTER)
						Panel = Global->CtrlObject->Cp()->PassivePanel();

					//Type==1 - плагиновый путь
					//Type==0 - обычный путь
					Panel->ExecFolder(strStr, Uuid, strFile, strData, true, false);
					// Panel may be changed
					if(SelectType == HRT_CTRLSHIFTENTER)
					{
						Panel = Global->CtrlObject->Cp()->ActivePanel();
						Panel->SetCurPath();
						Panel = Global->CtrlObject->Cp()->PassivePanel();
					}
					else
					{
						Panel = Global->CtrlObject->Cp()->ActivePanel();
					}
					Panel->Redraw();
				}
				break;

			case HRT_CTRLENTER:
				SetString(strStr, true);
				break;

			default:
				break;
			}
		}
		return true;

		case KEY_NUMENTER:
		case KEY_SHIFTNUMENTER:
		case KEY_ENTER:
		case KEY_SHIFTENTER:
		case KEY_CTRLALTENTER:
		case KEY_RCTRLRALTENTER:
		case KEY_CTRLRALTENTER:
		case KEY_RCTRLALTENTER:
		case KEY_CTRLALTNUMENTER:
		case KEY_RCTRLRALTNUMENTER:
		case KEY_CTRLRALTNUMENTER:
		case KEY_RCTRLALTNUMENTER:
		{
			const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
			CmdStr.RemoveSelection();
			Refresh();
			const auto& strStr = CmdStr.GetString();

			bool TryExecute = true;

			if (!strStr.empty())
			{
				ActivePanel->SetCurPath();

				if (!(Global->Opt->ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTCMDLINE))
					Global->CtrlObject->CmdHistory->AddToHistory(strStr, HR_DEFAULT, nullptr, {}, m_CurDir);
				TryExecute = !ActivePanel->ProcessPluginEvent(FE_COMMAND, UNSAFE_CSTR(strStr));
			}

			if (TryExecute)
			{
				const auto KeyCode = LocalKey();
				const auto IsNewWindow = (KeyCode & KEY_SHIFT) != 0;
				const auto IsRunAs = (KeyCode & KEY_CTRL || KeyCode & KEY_RCTRL) && (KeyCode & KEY_ALT || KeyCode & KEY_RALT);

				execute_info Info;
				Info.DisplayCommand = strStr;
				Info.Command = strStr;
				Info.WaitMode = IsNewWindow? execute_info::wait_mode::no_wait : execute_info::wait_mode::if_needed;
				Info.RunAs = IsRunAs;

				SetString({}, false);
				ExecString(Info);
			}
		}
		return true;

		case KEY_CTRLU:
		case KEY_RCTRLU:
			CmdStr.RemoveSelection();
			Refresh();
			return true;

		case KEY_OP_XLAT:
		{
			// 13.12.2000 SVS - ! Для CmdLine - если нет выделения, преобразуем всю строку (XLat)
			CmdStr.Xlat((Global->Opt->XLat.Flags&XLAT_CONVERTALLCMDLINE) != 0);

			// иначе неправильно работает ctrl-end
			strLastCmdStr = CmdStr.GetString();
			LastCmdPartLength = static_cast<int>(strLastCmdStr.size());

			return true;
		}

		/* дополнительные клавиши для выделения в ком строке.
		   ВНИМАНИЕ!
		   Для сокращения кода этот кусок должен стоять перед "default"
		*/
		case KEY_ALTSHIFTLEFT:   case KEY_ALTSHIFTNUMPAD4:
		case KEY_RALTSHIFTLEFT:  case KEY_RALTSHIFTNUMPAD4:
		case KEY_ALTSHIFTRIGHT:  case KEY_ALTSHIFTNUMPAD6:
		case KEY_RALTSHIFTRIGHT: case KEY_RALTSHIFTNUMPAD6:
		case KEY_ALTSHIFTEND:    case KEY_ALTSHIFTNUMPAD1:
		case KEY_RALTSHIFTEND:   case KEY_RALTSHIFTNUMPAD1:
		case KEY_ALTSHIFTHOME:   case KEY_ALTSHIFTNUMPAD7:
		case KEY_RALTSHIFTHOME:  case KEY_RALTSHIFTNUMPAD7:
			LocalKey&=~(KEY_ALT|KEY_RALT);
			[[fallthrough]];
		default:

			//   Сбрасываем выделение на некоторых клавишах
			if (!Global->Opt->CmdLine.EditBlock)
			{
				static const unsigned int UnmarkKeys[]=
				{
					KEY_LEFT,       KEY_NUMPAD4,
					KEY_CTRLS,      KEY_RCTRLS,
					KEY_RIGHT,      KEY_NUMPAD6,
					KEY_CTRLD,      KEY_RCTRLD,
					KEY_CTRLLEFT,   KEY_CTRLNUMPAD4,
					KEY_RCTRLLEFT,  KEY_RCTRLNUMPAD4,
					KEY_CTRLRIGHT,  KEY_CTRLNUMPAD6,
					KEY_RCTRLRIGHT, KEY_RCTRLNUMPAD6,
					KEY_CTRLHOME,   KEY_CTRLNUMPAD7,
					KEY_RCTRLHOME,  KEY_RCTRLNUMPAD7,
					KEY_CTRLEND,    KEY_CTRLNUMPAD1,
					KEY_RCTRLEND,   KEY_RCTRLNUMPAD1,
					KEY_HOME,       KEY_NUMPAD7,
					KEY_END,        KEY_NUMPAD1
				};

				if (contains(UnmarkKeys, LocalKey()))
				{
					CmdStr.RemoveSelection();
				}
			}

			if (any_of(LocalKey(), KEY_CTRLD, KEY_RCTRLD))
				LocalKey=KEY_RIGHT;

			if(any_of(LocalKey(), KEY_CTRLSPACE, KEY_RCTRLSPACE))
			{
				SCOPED_ACTION(SetAutocomplete)(&CmdStr, true);
				CmdStr.AutoComplete(true,false);
				return true;
			}

			if (!CmdStr.ProcessKey(LocalKey))
				return Global->WindowManager->Desktop()->ProcessKey(Key);

			LastCmdPartLength=-1;

			return true;
	}
}


void CommandLine::SetCurDir(string_view const CurDir)
{
	if (!equal_icase(m_CurDir, CurDir) || !equal_icase(os::fs::get_current_directory(), CurDir))
	{
		m_CurDir = CurDir;

		//Mantis#2350 - тормоз, это и так делается выше
		//if (Global->CtrlObject->Cp()->ActivePanel()->GetMode()!=PLUGIN_PANEL)
			//PrepareDiskPath(strCurDir);
	}
}

void CommandLine::SetString(string_view const Str, bool Redraw)
{
	LastCmdPartLength=-1;
	CmdStr.SetString(Str);
	CmdStr.SetLeftPos(0);

	if (Redraw)
		Refresh();
}

void CommandLine::InsertString(string_view const Str)
{
	LastCmdPartLength=-1;
	CmdStr.InsertString(Str);
	Refresh();
}


bool CommandLine::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED && MouseEvent->dwMousePosition.X == m_Where.right + 1)
	{
		return ProcessKey(Manager::Key(KEY_ALTF8));
	}
	return CmdStr.ProcessMouse(MouseEvent);
}

std::list<CommandLine::segment> CommandLine::GetPrompt()
{
	FN_RETURN_TYPE(CommandLine::GetPrompt) Result;
	size_t NewPromptSize = DEFAULT_CMDLINE_WIDTH;

	const auto& PrefixColor = colors::PaletteColorToFarColor(COL_COMMANDLINEPREFIX);

	if (Global->Opt->CmdLine.UsePromptFormat)
	{
		const string_view Format = Global->Opt->CmdLine.strPromptFormat.Get();
		auto SegmentBegin = Format.cbegin();
		auto Color = PrefixColor;
		for (auto Iterator = Format.cbegin(); Iterator != Format.cend();)
		{
			bool Stop;
			auto NewColor = PrefixColor;
			string_view const Str{ Iterator, Format.cend() };
			const auto Tail = colors::ExtractColorInNewFormat(Str, NewColor, Stop);
			if (Tail.size() == Str.size())
			{
				if (Stop)
					break;

				++Iterator;
				continue;
			}

			if (Iterator != Format.cbegin())
			{
				Result.emplace_back(segment{ { SegmentBegin, Iterator }, Color });
			}
			Iterator = Format.cend() - Tail.size();
			SegmentBegin = Iterator;
			Color = NewColor;
		}
		Result.emplace_back(segment{ { SegmentBegin, Format.cend() }, Color });

		for (auto Iterator = Result.begin(); Iterator != Result.end(); ++Iterator)
		{
			const auto strExpandedDestStr = os::env::expand(Iterator->Text);
			Iterator->Text.clear();

			const auto escaped_char = [](wchar_t const Char)
			{
				switch (Char)
				{
				case L'A': return L'&';
				case L'B': return L'|';
				case L'C': return L'(';
				case L'F': return L')';
				case L'G': return L'>';
				case L'L': return L'<';
				case L'Q': return L'=';
				case L'S': return L' ';
				case L'$': return L'$';
				default:   return L'\0';
				}
			};

			FOR_CONST_RANGE(strExpandedDestStr, it)
			{
				auto& strDestStr = Iterator->Text;

				if (*it == L'$' && it + 1 != strExpandedDestStr.cend())
				{
					const auto Chr = upper(*++it);
					if (const auto EscapedChar = escaped_char(Chr))
					{
						strDestStr += EscapedChar;
					}
					else
					{
						const auto AddCollapsible = [&](string&& Str)
						{
							if (strDestStr.empty())
							{
								strDestStr = std::move(Str);
								Iterator->Collapsible = true;
							}
							else
							{
								Iterator = Result.insert(std::next(Iterator), segment{ std::move(Str), Iterator->Colour, true });
							}

							// No need to introduce a new segment if we're at the very end
							if (std::next(Iterator) != Result.end() && std::next(it) != strExpandedDestStr.cend())
							{
								Iterator = Result.insert(std::next(Iterator), segment{ {}, Iterator->Colour, false });
							}
						};

						switch (Chr)
						{
								/* эти не реaлизованы
								$E - Escape code (ASCII code 27)
								$V - Windows version number
								$_ - Carriage return and linefeed
								*/
							case L'M': // $M - Отображение полного имени удаленного диска, связанного с именем текущего диска, или пустой строки, если текущий диск не является сетевым.
							{
								string strTemp;
								if (DriveLocalToRemoteName(true, m_CurDir, strTemp))
								{
									AddCollapsible(std::move(strTemp));
								}
								break;
							}
							case L'+': // $+  - Отображение нужного числа знаков плюс (+) в зависимости от текущей глубины стека каталогов PUSHD, по одному знаку на каждый сохраненный путь.
							{
								strDestStr.append(ppstack.size(), L'+');
								break;
							}
							case L'H': // $H - Backspace (erases previous character)
							{
								if (!strDestStr.empty())
								{
									strDestStr.pop_back();
								}
								else
								{
									auto Prev = Iterator;
									while (Prev != Result.begin())
									{
										--Prev;
										if (!Prev->Text.empty())
										{
											Prev->Text.pop_back();
											break;
										}
									}
								}

								break;
							}
							case L'@': // $@xx - Admin
							{
								if (it + 1 != strExpandedDestStr.cend())
								{
									const auto lb = *++it;
									if (it + 1 != strExpandedDestStr.cend())
									{
										const auto rb = *++it;
										if (os::security::is_admin())
										{
											append(strDestStr, lb, msg(lng::MConfigCmdlinePromptFormatAdmin), rb);
										}
									}
								}
								break;
							}
							case L'D': // $D - Current date
							case L'T': // $T - Current time
							{
								strDestStr += MkStrFTime(Chr == L'D'? L"%D"sv : L"%T"sv);
								break;
							}
							case L'N': // $N - Current drive
							{
								const auto Type = ParsePath(m_CurDir);
								if(Type == root_type::drive_letter)
									strDestStr += upper(m_CurDir[0]);
								else if(Type == root_type::win32nt_drive_letter)
									strDestStr += upper(m_CurDir[4]);
								else
									strDestStr += L'?';
								break;
							}
							case L'W': // $W - Текущий рабочий каталог (без указания пути)
							{
								const auto pos = FindLastSlash(m_CurDir);
								if (pos != string::npos)
								{
									AddCollapsible(m_CurDir.substr(pos + 1));
								}
								break;
							}
							case L'P': // $P - Current drive and path
							{
								AddCollapsible(string{ m_CurDir });
								break;
							}
							case L'#': //$#nn - max prompt width in %
							{
								if (it + 1 != strExpandedDestStr.end())
								{
									size_t pos;
									if (from_string(string_view(strExpandedDestStr).substr(it + 1 - strExpandedDestStr.cbegin()), NewPromptSize, &pos))
										it += pos;
									else
										LOGWARNING(L"Incorrect prompt width in {}"sv, strExpandedDestStr);
								}
							}
						}

						if (it == strExpandedDestStr.cend())
						{
							break;
						}
					}
				}
				else
				{
					strDestStr += *it;
				}
			}
		}
	}
	else
	{
		// default prompt = "$p$g"
		Result =
		{
			{ m_CurDir, PrefixColor, true },
			{ L">"s, PrefixColor, false },
		};
	}
	SetPromptSize(static_cast<int>(NewPromptSize));
	return Result;
}


void CommandLine::ShowViewEditHistory()
{
	string strStr;
	history_record_type Type;

	switch(const auto SelectType = Global->CtrlObject->ViewHistory->Select(msg(lng::MViewHistoryTitle), L"HistoryViews"sv, strStr, Type))
	{
	case HRT_ENTER:
	case HRT_SHIFTENTER:
		{
			auto Suppressor = Global->CtrlObject->ViewHistory->suppressor();
			if (SelectType != HRT_SHIFTENTER)
				Suppressor.reset();

			switch (Type)
			{
				case HR_VIEWER:
				{
					FileViewer::create(strStr, true);
					break;
				}

				case HR_EDITOR:
				case HR_EDITOR_RO:
				{
					// пусть файл создается
					const auto FEdit = FileEditor::create(
						strStr,
						CP_DEFAULT,
						FFILEEDIT_CANNEWFILE | FFILEEDIT_ENABLEF6 | (Type == HR_EDITOR_RO? FFILEEDIT_LOCKED : 0)
					);

					break;
				}

				case HR_EXTERNAL:
				case HR_EXTERNAL_WAIT:
				{
					execute_info Info;
					Info.DisplayCommand = strStr;
					Info.Command = strStr;
					Info.WaitMode = Type == HR_EXTERNAL_WAIT? execute_info::wait_mode::wait_finish : execute_info::wait_mode::if_needed;

					ExecString(Info);
					break;
				}
			}
		}
		break;

	case HRT_CTRLENTER:
		SetString(strStr, true);
		break;

	default:
		break;
	}
}

void CommandLine::SetPromptSize(int NewSize)
{
	PromptSize = NewSize? std::clamp(NewSize, 5, 95) : DEFAULT_CMDLINE_WIDTH;
}

static bool ProcessFarCommands(string_view Command, function_ref<void()> const ConsoleActivatior)
{
	inplace::trim(Command);

	if (constexpr auto Prefix = L"far:"sv; Command.starts_with(Prefix))
		Command.remove_prefix(Prefix.size());
	else
		return false;

	if (equal_icase(Command, L"config"sv))
	{
		ConsoleActivatior();
		Global->Opt->AdvancedConfig();
		return true;
	}

	if (equal_icase(Command, L"about"sv))
	{
		ConsoleActivatior();

		std::wcout << L'\n' << build::version_string() << L'\n' << build::copyright() << L'\n';

		if (const auto Revision = build::scm_revision(); !Revision.empty())
		{
			std::wcout << L"\nSCM revision:\n"sv << Revision << L'\n';
		}

		std::wcout << L"\nCompiler:\n"sv << build::compiler() << L'\n';
		std::wcout << L"\nStandard library:\n"sv << build::library() << L'\n';

		if (const auto& ComponentsInfo = components::GetComponentsInfo(); !ComponentsInfo.empty())
		{
			std::wcout << L"\nThird party libraries:\n"sv;

			for (const auto& [Name, Version]: ComponentsInfo)
			{
				std::wcout << Name;
				if (!Version.empty())
				{
					std::wcout << L", version "sv << Version;
				}
				std::wcout << L'\n';
			}
		}

		if (const auto& Factories = Global->CtrlObject->Plugins->Factories(); std::ranges::any_of(Factories, [](const auto& i) { return i->IsExternal(); }))
		{
			std::wcout << L"\nPlugin adapters:\n"sv;
			for (const auto& i: Factories)
			{
				if (i->IsExternal())
					std::wcout << i->Title() << L", version "sv << version_to_string(i->version()) << L'\n';
			}
		}

		if (Global->CtrlObject->Plugins->size())
		{
			std::wcout << L"\nPlugins:\n"sv;

			for (const auto& i: *Global->CtrlObject->Plugins)
			{
				std::wcout << i->Title() << L", version "sv << version_to_string(i->version()) << L'\n';
			}
		}

		std::wcout << std::flush;

		return true;
	}

	if (const auto LogCommand = L"log"sv; starts_with_icase(Command, LogCommand))
	{
		if (const auto LogParameters = Command.substr(LogCommand.size()); LogParameters.starts_with(L' ') || LogParameters.empty())
		{
			ConsoleActivatior();
			logging::configure(trim(LogParameters));
			return true;
		}
	}

	if (equal_icase(Command, L"regex"sv))
	{
		regex_playground();
		return true;
	}

	return false;
}

static void ProcessEcho(execute_info& Info)
{
	if (!Info.Command.starts_with(L'@'))
		return;

	Info.Echo = false;
	if (Info.DisplayCommand.empty())
		Info.DisplayCommand = Info.Command;
	Info.Command.erase(0, Info.Command.find_first_not_of(L"@"sv));
}

void CommandLine::ExecString(execute_info& Info)
{
	bool IsUpdateNeeded = false;

	std::shared_ptr<i_context> ExecutionContext;

	SCOPE_EXIT
	{
		if (ExecutionContext)
			ExecutionContext->DoEpilogue(Info.Echo && !Info.Command.empty(), true);

		if (!IsUpdateNeeded)
			return;

		if (!m_Flags.Check(FCMDOBJ_LOCKUPDATEPANEL))
		{
			ShellUpdatePanels(Global->CtrlObject->Cp()->ActivePanel(), false);
			if (Global->Opt->ShowKeyBar)
			{
				Global->CtrlObject->Cp()->GetKeybar().Show();
			}
		}

		if (Info.WaitMode != execute_info::wait_mode::no_wait)
		{
			Global->ScrBuf->Flush();
		}
	};

	const auto Activator = [&]
	{
		if (!ExecutionContext)
			ExecutionContext = Global->WindowManager->Desktop()->ConsoleSession().GetContext();

		ExecutionContext->Activate();

		if (Info.Echo)
			ExecutionContext->DrawCommand(Info.DisplayCommand.empty()? Info.Command : Info.DisplayCommand);

		ExecutionContext->DoPrologue();
		ExecutionContext->Consolise();

		if (Info.Echo)
			std::wcout << std::endl;
	};

	if (Info.Command.empty())
	{
		// Just scroll the screen
		Activator();
		console.start_output();
		console.command_finished();
		return;
	}

	LastCmdPartLength = -1;

	FarChDir(m_CurDir);

	if (Info.SourceMode == execute_info::source_mode::unknown)
	{
		ProcessEcho(Info);
		if (Info.Command.starts_with(L'@'))
		{
			Info.Echo = false;
			if (Info.DisplayCommand.empty())
				Info.DisplayCommand = Info.Command;
			Info.Command.erase(0, Info.Command.find_first_not_of(L"@"sv));
		}

		ExpandOSAliases(Info.Command);

		if (!ExtractIfExistCommand(Info.Command))
		{
			Activator();
			return;
		}

		ProcessEcho(Info);
		if (Info.Command.empty())
		{
			Activator();
			return;
		}

		if (Info.WaitMode != execute_info::wait_mode::no_wait && !Info.RunAs)
		{
			if (ProcessFarCommands(Info.Command, Activator))
				return;

			if (Global->CtrlObject->Plugins->ProcessCommandLine(Info.Command))
				return;

			if (ProcessOSCommands(Info.Command, Activator))
			{
				console.command_finished(EXIT_SUCCESS);
				return;
			}
		}
	}

	Execute(Info, Activator);

	// BUGBUG do we really need to update panels at all?
	IsUpdateNeeded = true;
}

bool CommandLine::ProcessOSCommands(string_view const CmdLine, function_ref<void()> const ConsoleActivatior)
{
	auto SetPanel = Global->CtrlObject->Cp()->ActivePanel();

	if (SetPanel->GetType() != panel_type::FILE_PANEL && Global->CtrlObject->Cp()->PassivePanel()->GetType() == panel_type::FILE_PANEL)
		SetPanel=Global->CtrlObject->Cp()->PassivePanel();

	const auto Trimmed = trim(CmdLine);

	const auto Command = Trimmed.substr(0, Trimmed.find_first_of(L' '));
	const auto Arguments = trim_left(Trimmed.substr(Command.size()));

	// Cursed spaceless DOS legacy syntax like cd\ or cd/d
	// Technically other commands can use this syntax too, but we limit it to cd/chdir
	const auto CommandCursed = Trimmed.substr(0, Trimmed.find_first_of(L" \\/"));
	const auto ArgumentsCursed = trim_left(Trimmed.substr(CommandCursed.size()));

	const auto FindKey = [&](string_view const Where, wchar_t const Key)
	{
		const wchar_t Param[] = { L'/', Key, {} };
		return starts_with_icase(Where, Param) && (Where.size() == 2 || Where[2] == L' ');
	};

	if (Command.size() == 2 && Command[1] == L':' && Arguments.empty())
	{
		ConsoleActivatior();

		const auto DriveLetter = upper(CmdLine[0]);
		if (!FarChDir(os::fs::drive::get_device_path(DriveLetter)))
		{
			FarChDir(os::fs::drive::get_root_directory(DriveLetter));
		}
		SetPanel->ChangeDirToCurrent();
		return true;
	}

	if (FindKey(Arguments, L'?'))
		return false;

	// SET [variable=[value]]
	if (equal_icase(Command, L"SET"sv))
	{
		if (FindKey(Arguments, L'A') || FindKey(Arguments, L'P'))
			return false; //todo: /p - dialog, /a - calculation; then set variable ...

		size_t pos;
		const auto SetParams = unquote(trim_right(Arguments));

		// "set" (display all) or "set var" (display all that begin with "var")
		if (SetParams.empty() || ((pos = SetParams.find(L'=')) == string::npos) || !pos)
		{
			//forward "set [prefix]| command" and "set [prefix]> file" to COMSPEC
			if (SetParams.find_first_of(L"|>"sv) != SetParams.npos)
				return false;

			ConsoleActivatior();

			const os::env::provider::strings EnvStrings;
			for (const auto& i: enum_substrings(EnvStrings.data()))
			{
				if (starts_with_icase(i, SetParams))
				{
					std::wcout << i << L'\n';
				}
			}
			std::wcout << std::flush;

			return true;
		}

		ConsoleActivatior();

		const auto VariableValue = SetParams.substr(pos + 1);
		const auto VariableName = SetParams.substr(0, pos);

		if (VariableValue.empty()) //set var=
		{
			os::env::del(VariableName);
		}
		else
		{
			os::env::set(VariableName, os::env::expand(VariableValue));
		}

		return true;
	}

	if (equal_icase(Command, L"CLS"sv))
	{
		if (!Arguments.empty())
		{
			// Theoretically, in cmd "cls" and "cls blablabla" are the same things.
			// But, if the user passed some parameters to cls it's quite probably
			// some chained command rather than parameters, e. g. "cls & dir".
			// We have more complex logic in execute::PartCmdLine, but using it here isn't worth the effort.
			return false;
		}

		ConsoleActivatior();
		console.Clear(colors::PaletteColorToFarColor(COL_COMMANDLINEUSERSCREEN));
		return true;
	}

	// PUSHD путь | ..
	if (equal_icase(Command, L"PUSHD"sv))
	{
		ConsoleActivatior();

		const auto PushDir = m_CurDir;

		if (const auto NewDir = trim_right(Arguments); NewDir.empty() || IntChDir(NewDir, true))
		{
			ppstack.push(PushDir);
			os::env::set(L"FARDIRSTACK"sv, PushDir);
		}

		return true;
	}

	// POPD
	// TODO: добавить необязательный параметр - число, сколько уровней пропустить, после чего прыгнуть.
	if (equal_icase(Command, L"POPD"sv))
	{
		ConsoleActivatior();

		if (!ppstack.empty())
		{
			IntChDir(ppstack.top(), true);

			ppstack.pop();
			if (!ppstack.empty())
			{
				os::env::set(L"FARDIRSTACK"sv, ppstack.top());
			}
			else
			{
				os::env::del(L"FARDIRSTACK"sv);
			}
		}

		return true;
	}

	// CLRD
	if (equal_icase(Command, L"CLRD"sv))
	{
		ConsoleActivatior();

		clear_and_shrink(ppstack);
		os::env::del(L"FARDIRSTACK"sv);
		return true;
	}

	/*
		Displays or sets the active code page number.
		CHCP [nnn]
			nnn   Specifies a code page number (Dec or Hex).
		Type CHCP without a parameter to display the active code page number.
	*/
	if (equal_icase(Command, L"CHCP"sv))
	{
		const auto ChcpParams = trim_right(Arguments);
		uintptr_t cp;
		if (!from_string(ChcpParams, cp))
			return false;

		const auto SavedOutputCp = console.GetOutputCodepage();

		if (!console.SetInputCodepage(cp) || !console.SetOutputCodepage(cp))
			return false;

		if (SavedOutputCp != cp)
		{
			char_width::invalidate();
		}

		ConsoleActivatior();

		Text(ChcpParams);

		return true;
	}

	if (const auto IsCommandCd = equal_icase(CommandCursed, L"CD"sv); IsCommandCd || equal_icase(CommandCursed, L"CHDIR"sv))
	{
		auto CdParams = trim_right(ArgumentsCursed);

		//проигнорируем /D
		//мы и так всегда меняем диск а некоторые в алайсах или по привычке набирают этот ключ
		const auto ParamD = L"/D"sv;
		if (starts_with_icase(CdParams, ParamD) && (CdParams.size() == ParamD.size() || std::iswblank(CdParams[2])))
		{
			CdParams.remove_prefix(ParamD.size());
			inplace::trim_left(CdParams);
		}

		if (CdParams.empty())
			return false;

		ConsoleActivatior();

		IntChDir(CdParams, !IsCommandCd);
		return true;
	}

	if (equal_icase(Command, L"TITLE"))
	{
		ConsoleActivatior();

		ConsoleTitle::SetUserTitle(Arguments);

		if (!(Global->CtrlObject->Cp()->LeftPanel()->IsVisible() || Global->CtrlObject->Cp()->RightPanel()->IsVisible()))
		{
			Global->CtrlObject->Cp()->ActivePanel()->RefreshTitle();
		}
		return true;
	}

	if (equal_icase(Command, L"EXIT"sv))
	{
		int ExitCode = EXIT_SUCCESS;
		if (!from_string(Arguments, ExitCode))
			LOGWARNING(L"Error parsing exit arguments: {}"sv, Arguments);

		ConsoleActivatior();
		Global->WindowManager->ExitMainLoop(FALSE, ExitCode);
		return true;
	}

	return false;
}

bool CommandLine::IntChDir(string_view const CmdLine, bool const ClosePanel, bool const Silent)
{
	auto SetPanel = Global->CtrlObject->Cp()->ActivePanel();

	if (SetPanel->GetType() != panel_type::FILE_PANEL && Global->CtrlObject->Cp()->PassivePanel()->GetType() == panel_type::FILE_PANEL)
		SetPanel=Global->CtrlObject->Cp()->PassivePanel();

	auto strExpandedDir = unquote(os::env::expand(CmdLine));

	if (SetPanel->GetMode() != panel_mode::PLUGIN_PANEL && strExpandedDir[0] == L'~' && ((strExpandedDir.size() == 1 && !os::fs::exists(strExpandedDir)) || path::is_separator(strExpandedDir[1])))
	{
		if (Global->Opt->Exec.UseHomeDir && !Global->Opt->Exec.strHomeDir.empty())
		{
			string strTemp=Global->Opt->Exec.strHomeDir.Get();

			if (strExpandedDir.size() > 1)
			{
				path::append(strTemp, string_view(strExpandedDir).substr(2));
			}

			DeleteEndSlash(strTemp);
			strExpandedDir = os::env::expand(strTemp);
		}
	}

	if (IsAbsolutePath(strExpandedDir))
	{
		path::inplace::normalize_separators(strExpandedDir);
		SetPanel->SetCurDir(strExpandedDir,true);
		return true;
	}

	if (SetPanel->GetType() == panel_type::FILE_PANEL && SetPanel->GetMode() == panel_mode::PLUGIN_PANEL)
	{
		SetPanel->SetCurDir(strExpandedDir, ClosePanel);
		return true;
	}

	while (!FarChDir(strExpandedDir))
	{
		if (Silent || !TryParentFolder(strExpandedDir))
			return false;
	}

	SetPanel->ChangeDirToCurrent();

	if (!SetPanel->IsVisible())
		SetPanel->RefreshTitle();

	return true;
}

void CommandLine::LockUpdatePanel(bool Mode)
{
	m_Flags.Change(FCMDOBJ_LOCKUPDATEPANEL,Mode);
}
