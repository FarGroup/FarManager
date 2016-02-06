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

#include "headers.hpp"
#pragma hdrstop

#include "cmdline.hpp"
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
#include "syslog.hpp"
#include "config.hpp"
#include "usermenu.hpp"
#include "datetime.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "keyboard.hpp"
#include "mix.hpp"
#include "console.hpp"
#include "panelmix.hpp"
#include "message.hpp"
#include "network.hpp"
#include "plugins.hpp"
#include "colormix.hpp"
#include "constitle.hpp"
#include "language.hpp"
#include "components.hpp"
#include "desktop.hpp"
#include "keybar.hpp"

enum
{
	FCMDOBJ_LOCKUPDATEPANEL   = 0x00010000,
	DEFAULT_CMDLINE_WIDTH = 50,
};

CommandLine::CommandLine(window_ptr Owner):
	SimpleScreenObject(Owner),
	PromptSize(DEFAULT_CMDLINE_WIDTH),
	CmdStr(
		Owner,
		this,
		[this](const Manager::Key& Key){ return Global->CtrlObject->Cp()->ProcessKey(Key); },
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
	size_t CurLength = 0;
	GotoXY(m_X1, m_Y1);

	std::for_each(CONST_RANGE(PromptList, i)
	{
		SetColor(i.second);
		string str(i.first);
		if (CurLength + str.size() > MaxLength)
			TruncPathStr(str, std::max(0, static_cast<int>(MaxLength - CurLength)));
		Text(str);
		CurLength += str.size();
	});
	return CurLength;
}

void CommandLine::DisplayObject()
{
	_OT(SysLog(L"[%p] CommandLine::DisplayObject()",this));

	size_t CurLength = DrawPrompt();

	CmdStr.SetObjectColor(COL_COMMANDLINE,COL_COMMANDLINESELECTED);
	CmdStr.SetPosition(m_X1+(int)CurLength,m_Y1,m_X2,m_Y2);
	CmdStr.Show();

	GotoXY(m_X2+1,m_Y1);
	SetColor(COL_COMMANDLINEPREFIX);
	Text(L'\x2191'); // up arrow
}

void CommandLine::DrawFakeCommand(const string& FakeCommand)
{
	DrawPrompt();
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

__int64 CommandLine::VMProcess(int OpCode,void *vParam,__int64 iParam)
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

int CommandLine::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key;

	if ((LocalKey()==KEY_CTRLEND || LocalKey()==KEY_RCTRLEND || LocalKey()==KEY_CTRLNUMPAD1 || LocalKey()==KEY_RCTRLNUMPAD1) && (CmdStr.GetCurPos()==CmdStr.GetLength()))
	{
		if (LastCmdPartLength==-1)
			strLastCmdStr = CmdStr.GetString();

		auto strStr = strLastCmdStr;
		int CurCmdPartLength=(int)strStr.size();
		Global->CtrlObject->CmdHistory->GetSimilar(strStr,LastCmdPartLength);

		if (LastCmdPartLength==-1)
		{
			strLastCmdStr = CmdStr.GetString();
			LastCmdPartLength=CurCmdPartLength;
		}

		{
			SCOPED_ACTION(SetAutocomplete)(&CmdStr);
			CmdStr.SetString(strStr);
			CmdStr.Select(LastCmdPartLength,static_cast<int>(strStr.size()));
		}

		Show();
		return TRUE;
	}

	if (LocalKey() == KEY_UP || LocalKey() == KEY_NUMPAD8)
	{
		if (Global->CtrlObject->Cp()->LeftPanel()->IsVisible() || Global->CtrlObject->Cp()->RightPanel()->IsVisible())
			return FALSE;

		LocalKey=KEY_CTRLE;
	}
	else if (LocalKey() == KEY_DOWN || LocalKey() == KEY_NUMPAD2)
	{
		if (Global->CtrlObject->Cp()->LeftPanel()->IsVisible() || Global->CtrlObject->Cp()->RightPanel()->IsVisible())
			return FALSE;

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

				string strStr;
				if (LocalKey() == KEY_CTRLE || LocalKey() == KEY_RCTRLE)
				{
					Global->CtrlObject->CmdHistory->GetPrev(strStr);
				}
				else
				{
					Global->CtrlObject->CmdHistory->GetNext(strStr);
				}

				{
					SCOPED_ACTION(SetAutocomplete)(&CmdStr);
					SetString(Global->CtrlObject->CmdHistory->IsOnTop()? m_CurCmdStr : strStr, true);
				}

			}
			return TRUE;

		case KEY_ESC:
		{
			// $ 24.09.2000 SVS - Если задано поведение по "Несохранению при Esc", то позицию в хистори не меняем и ставим в первое положение.
			if (Global->Opt->CmdHistoryRule)
				Global->CtrlObject->CmdHistory->ResetPosition();
			SetString(L"", true);
			return TRUE;
		}

		case KEY_F2:
		{
			UserMenu(false);
			return TRUE;
		}
		case KEY_ALTF8:
		case KEY_RALTF8:
		{
			history_record_type Type;
			string strStr;
			const auto SelectType = Global->CtrlObject->CmdHistory->Select(MSG(MHistoryTitle), L"History", strStr, Type);
			if (SelectType == HRT_ENTER || SelectType == HRT_SHIFTETNER || SelectType == HRT_CTRLENTER || SelectType == HRT_CTRLALTENTER)
			{
				std::unique_ptr<SetAutocomplete> disable;
				if(SelectType != HRT_CTRLENTER)
				{
					disable = std::make_unique<SetAutocomplete>(&CmdStr);
				}
				SetString(strStr, true);

				if (SelectType != HRT_CTRLENTER)
				{
					ProcessKey(SelectType == HRT_CTRLALTENTER? Manager::Key(KEY_CTRLALTENTER) : (SelectType == HRT_ENTER? Manager::Key(KEY_ENTER) : Manager::Key(KEY_SHIFTENTER)));
				}
			}
		}
		return TRUE;
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
		return TRUE;
		case KEY_ALTF11:
		case KEY_RALTF11:
			ShowViewEditHistory();
			Global->CtrlObject->Cp()->Redraw();
			return TRUE;
		case KEY_ALTF12:
		case KEY_RALTF12:
		{
			history_record_type Type;
			GUID Guid;
			string strFile, strData, strStr;
			const auto SelectType = Global->CtrlObject->FolderHistory->Select(MSG(MFolderHistoryTitle), L"HistoryFolders", strStr, Type, &Guid, &strFile, &strData);

			switch(SelectType)
			{
			case HRT_ENTER:
			case HRT_SHIFTETNER:
			case HRT_CTRLSHIFTENTER:
				{

					if (SelectType == HRT_SHIFTETNER)
						Global->CtrlObject->FolderHistory->SetAddMode(false,2,true);

					// пусть плагин сам прыгает... ;-)
					auto Panel = Global->CtrlObject->Cp()->ActivePanel();

					if (SelectType == HRT_CTRLSHIFTENTER)
						Panel = Global->CtrlObject->Cp()->PassivePanel();

					//Type==1 - плагиновый путь
					//Type==0 - обычный путь
					Panel->ExecShortcutFolder(strStr,Guid,strFile,strData,true);
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
					Global->CtrlObject->FolderHistory->SetAddMode(true,2,true);
				}
				break;

			case HRT_CTRLENTER:
				SetString(strStr, true);
				break;

			default:
				break;
			}
		}
		return TRUE;
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
					Global->CtrlObject->CmdHistory->AddToHistory(strStr, HR_DEFAULT, nullptr, nullptr, m_CurDir.data());
				TryExecute = !ActivePanel->ProcessPluginEvent(FE_COMMAND, UNSAFE_CSTR(strStr));
			}

			if (TryExecute)
			{
				const auto KeyCode = LocalKey();
				const auto IsNewWindow = (KeyCode & KEY_SHIFT) != 0;
				const auto IsRunAs = (KeyCode & KEY_CTRL || KeyCode & KEY_RCTRL) && (KeyCode & KEY_ALT || KeyCode & KEY_RALT);

				execute_info Info;
				Info.Command = strStr;
				Info.WaitMode = Info.no_wait;
				Info.NewWindow = IsNewWindow;
				Info.DirectRun = false;
				Info.RunAs = IsRunAs;

				SetString(L"", false);
				ExecString(Info);
			}
		}
		return TRUE;
		case KEY_CTRLU:
		case KEY_RCTRLU:
			CmdStr.RemoveSelection();
			Refresh();
			return TRUE;
		case KEY_OP_XLAT:
		{
			// 13.12.2000 SVS - ! Для CmdLine - если нет выделения, преобразуем всю строку (XLat)
			CmdStr.Xlat((Global->Opt->XLat.Flags&XLAT_CONVERTALLCMDLINE) != 0);

			// иначе неправильно работает ctrl-end
			strLastCmdStr = CmdStr.GetString();
			LastCmdPartLength=(int)strLastCmdStr.size();

			return TRUE;
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
		default:

			//   Сбрасываем выделение на некоторых клавишах
			if (!Global->Opt->CmdLine.EditBlock)
			{
				static int UnmarkKeys[]=
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

				if (std::find(ALL_CONST_RANGE(UnmarkKeys), LocalKey()) != std::cend(UnmarkKeys))
				{
					CmdStr.RemoveSelection();
				}
			}

			if (LocalKey() == KEY_CTRLD || LocalKey() == KEY_RCTRLD)
				LocalKey=KEY_RIGHT;

			if(LocalKey() == KEY_CTRLSPACE || LocalKey() == KEY_RCTRLSPACE)
			{
				SCOPED_ACTION(SetAutocomplete)(&CmdStr, true);
				CmdStr.AutoComplete(true,false);
				return TRUE;
			}

			if (!CmdStr.ProcessKey(LocalKey))
				return Global->CtrlObject->Desktop->ProcessKey(Key);

			LastCmdPartLength=-1;

			return TRUE;
	}
}


void CommandLine::SetCurDir(const string& CurDir)
{
	if (StrCmpI(m_CurDir, CurDir) || !TestCurrentDirectory(CurDir))
	{
		m_CurDir = CurDir;

		//Mantis#2350 - тормоз, это и так делается выше
		//if (Global->CtrlObject->Cp()->ActivePanel()->GetMode()!=PLUGIN_PANEL)
			//PrepareDiskPath(strCurDir);
	}
}

void CommandLine::SetString(const string& Str, bool Redraw)
{
	LastCmdPartLength=-1;
	CmdStr.SetString(Str);
	CmdStr.SetLeftPos(0);

	if (Redraw)
		Refresh();
}

void CommandLine::InsertString(const string& Str)
{
	LastCmdPartLength=-1;
	CmdStr.InsertString(Str);
	Refresh();
}


int CommandLine::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if(MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED && MouseEvent->dwMousePosition.X==m_X2+1)
	{
		return ProcessKey(Manager::Key(KEY_ALTF8));
	}
	return CmdStr.ProcessMouse(MouseEvent);
}

inline COLORREF ARGB2ABGR(COLORREF Color)
{
	return (Color & 0xFF000000) | ((Color & 0x00FF0000) >> 16) | (Color & 0x0000FF00) | ((Color & 0x000000FF) << 16);
}

inline void AssignColor(const string& Color, COLORREF& Target, FARCOLORFLAGS& TargetFlags, FARCOLORFLAGS SetFlag)
{
	if (!Color.empty())
	{
		if (ToUpper(Color[0]) == L'T')
		{
			TargetFlags &= ~SetFlag;
			Target = ARGB2ABGR(std::wcstoul(Color.data() + 1, nullptr, 16));
		}
		else
		{
			TargetFlags |= SetFlag;
			Target = std::stoul(Color, nullptr, 16);
		}
	}
}

std::list<std::pair<string, FarColor>> CommandLine::GetPrompt()
{
	FN_RETURN_TYPE(CommandLine::GetPrompt) Result;
	int NewPromptSize = DEFAULT_CMDLINE_WIDTH;

	const auto& PrefixColor = colors::PaletteColorToFarColor(COL_COMMANDLINEPREFIX);

	if (Global->Opt->CmdLine.UsePromptFormat)
	{
		const auto& Format = Global->Opt->CmdLine.strPromptFormat.Get();
		auto F = PrefixColor;
		auto Str = Format;
		FOR_CONST_RANGE(Format, Ptr)
		{
			// color in ([[T]ffffffff][:[T]bbbbbbbb]) format
			if (*Ptr == L'(')
			{
				string Color = &*Ptr + 1;
				auto Pos = Color.find(L')');
				if(Pos != string::npos)
				{
					if (Ptr != Format.cbegin())
					{
						Str.resize(Str.find(L'('));
						Result.emplace_back(VALUE_TYPE(Result)(Str, F));
					}
					Ptr += Pos+2;
					Color.resize(Pos);
					string BgColor;
					Pos = Color.find(L':');
					if (Pos != string::npos)
					{
						BgColor = Color.substr(Pos+1);
						Color.resize(Pos);
					}

					F = PrefixColor;

					AssignColor(Color, F.ForegroundColor, F.Flags, FCF_FG_4BIT);
					AssignColor(BgColor, F.BackgroundColor, F.Flags, FCF_BG_4BIT);

					Str = &*Ptr;
				}
			}
		}
		Result.emplace_back(VALUE_TYPE(Result)(Str, F));

		std::for_each(RANGE(Result, i)
		{
			auto& strDestStr = i.first;
			const auto strExpandedDestStr = os::env::expand_strings(strDestStr);
			strDestStr.clear();
			static const simple_pair<wchar_t, wchar_t> ChrFmt[] =
			{
				{L'A', L'&'},   // $A - & (Ampersand)
				{L'B', L'|'},   // $B - | (pipe)
				{L'C', L'('},   // $C - ( (Left parenthesis)
				{L'F', L')'},   // $F - ) (Right parenthesis)
				{L'G', L'>'},   // $G - > (greater-than sign)
				{L'L', L'<'},   // $L - < (less-than sign)
				{L'Q', L'='},   // $Q - = (equal sign)
				{L'S', L' '},   // $S - (space)
				{L'$', L'$'},   // $$ - $ (dollar sign)
			};

			FOR_CONST_RANGE(strExpandedDestStr, it)
			{
				if (*it == L'$' && it + 1 != strExpandedDestStr.cend())
				{
					const auto Chr = ToUpper(*++it);

					const auto ItemIterator = std::find_if(CONST_RANGE(ChrFmt, i)
					{
						return i.first == Chr;
					});

					if (ItemIterator != std::cend(ChrFmt))
					{
						strDestStr += ItemIterator->second;
					}
					else
					{
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
								if (DriveLocalToRemoteName(DRIVE_UNKNOWN, m_CurDir[0], strTemp))
								{
									strDestStr += strTemp;
									//strDestStr += L" "; // ???
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
									strDestStr.pop_back();

								break;
							}
							case L'@': // $@xx - Admin
							{
								if (it + 1 != strExpandedDestStr.cend())
								{
									wchar_t lb = *(++it);
									if (it + 1 != strExpandedDestStr.cend())
									{
										wchar_t rb = *(++it);
										if (os::security::is_admin())
										{
											strDestStr += lb;
											strDestStr += MSG(MConfigCmdlinePromptFormatAdmin);
											strDestStr += rb;
										}
									}
								}
								break;
							}
							case L'D': // $D - Current date
							case L'T': // $T - Current time
							{
								strDestStr += MkStrFTime(Chr == L'D'? L"%D" : L"%T");
								break;
							}
							case L'N': // $N - Current drive
							{
								const auto Type = ParsePath(m_CurDir);
								if(Type == PATH_DRIVELETTER)
									strDestStr += ToUpper(m_CurDir[0]);
								else if(Type == PATH_DRIVELETTERUNC)
									strDestStr += ToUpper(m_CurDir[4]);
								else
									strDestStr += L'?';
								break;
							}
							case L'W': // $W - Текущий рабочий каталог (без указания пути)
							{
								const auto pos = FindLastSlash(m_CurDir);
								if (pos != string::npos)
								{
									strDestStr += m_CurDir.substr(pos + 1);
								}
								break;
							}
							case L'P': // $P - Current drive and path
							{
								strDestStr += m_CurDir;
								break;
							}
							case L'#': //$#nn - max promt width in %
							{
								if (it + 1 != strExpandedDestStr.end())
								{
									try
									{
										size_t pos = 0;
										NewPromptSize = std::stoi(string(it+1, strExpandedDestStr.cend()), &pos);
										it += pos;
									}
									catch (const std::exception&)
									{
										// bad format, NewPromptSize unchanged
										// TODO: diagnostics
									}
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
		});

	}
	else
	{
		// default prompt = "$p$g"
		Result.emplace_back(VALUE_TYPE(Result)(m_CurDir + L">", PrefixColor));
	}
	SetPromptSize(NewPromptSize);
	return Result;
}


void CommandLine::ShowViewEditHistory()
{
	string strStr;
	history_record_type Type;
	const auto SelectType = Global->CtrlObject->ViewHistory->Select(MSG(MViewHistoryTitle), L"HistoryViews", strStr, Type);

	switch(SelectType)
	{
	case HRT_ENTER:
	case HRT_SHIFTETNER:
		{
			if (SelectType == HRT_ENTER)
				Global->CtrlObject->ViewHistory->AddToHistory(strStr,Type);

			Global->CtrlObject->ViewHistory->SetAddMode(false,Global->Opt->FlagPosixSemantics?1:2,true);

			switch (Type)
			{
				case HR_VIEWER:
				{
					FileViewer::create(strStr, TRUE);
					break;
				}

				case HR_EDITOR:
				case HR_EDITOR_RO:
				{
					// пусть файл создается
					const auto FEdit = FileEditor::create(strStr, CP_DEFAULT, FFILEEDIT_CANNEWFILE | FFILEEDIT_ENABLEF6);

					if (Type == HR_EDITOR_RO)
						FEdit->SetLockEditor(TRUE);

					break;
				}

				case HR_EXTERNAL:
				case HR_EXTERNAL_WAIT:
				{
					execute_info Info;
					Info.Command = strStr;
					Info.WaitMode = Type == HR_EXTERNAL_WAIT? Info.wait_finish : Info.no_wait;
					Info.NewWindow = false;
					Info.DirectRun = false;
					Info.RunAs = false;

					ExecString(Info);
					break;
				}
			}

			Global->CtrlObject->ViewHistory->SetAddMode(true,Global->Opt->FlagPosixSemantics?1:2,true);
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
	PromptSize = NewSize? std::max(5, std::min(95, NewSize)) : DEFAULT_CMDLINE_WIDTH;
}

class execution_context: noncopyable
{
public:
	execution_context(const string& Command):
		m_CurrentWindow(Global->WindowManager->GetCurrentWindow()),
		m_Command(Command),
		m_Activated(),
		m_Consolised()
	{
		++Global->ProcessShowClock;
	}

	void Activate()
	{
		if (m_Activated)
			return;

		m_Activated = true;

		Global->WindowManager->ActivateWindow(Global->CtrlObject->Desktop);
		Global->WindowManager->ShowBackground();
		Global->CtrlObject->CmdLine()->DrawFakeCommand(m_Command);
		ScrollScreen(1);
		Global->CtrlObject->Desktop->TakeSnapshot();
		int X1, Y1, X2, Y2;
		Global->CtrlObject->CmdLine()->GetPosition(X1, Y1, X2, Y2);
		GotoXY(0, Y1);
	}

	void ConsoleContext()
	{
		Activate();
		if (m_Consolised)
			return;
		m_Consolised = true;
		
		Global->ScrBuf->MoveCursor(0, WhereY());
		SetInitialCursorType();

		// BUGBUG, implement better & safer way to do this
		const auto LockCount = Global->ScrBuf->GetLockCount();
		Global->ScrBuf->SetLockCount(0);

		Global->ScrBuf->Flush();

		// BUGBUG, implement better & safer way to do this
		Global->ScrBuf->SetLockCount(LockCount);

		Console().SetTextAttributes(colors::PaletteColorToFarColor(COL_COMMANDLINEUSERSCREEN));
	}

	const string& Command() const
	{
		return m_Command;
	}

	~execution_context()
	{
		if (!m_Activated)
			return;

		if (m_Consolised)
		{
			if (Global->Opt->ShowKeyBar)
			{
				Console().Write(L"\n");
			}
			Console().Commit();
			Global->ScrBuf->FillBuf();
		}

		if (!m_Command.empty())
		{
			ScrollScreen(1);
		}

		Global->CtrlObject->Desktop->TakeSnapshot();
		Global->WindowManager->SubmergeWindow(Global->CtrlObject->Desktop);
		Global->WindowManager->ActivateWindow(m_CurrentWindow);
		Global->WindowManager->RefreshAll();
		--Global->ProcessShowClock;
	}

private:
	const window_ptr m_CurrentWindow;
	const string m_Command;
	bool m_Activated;
	bool m_Consolised;
};

static bool ProcessFarCommands(const string& Command, execution_context& ExecutionContext)
{
	if (!StrCmpI(Command.data(), L"far:config"))
	{
		Global->Opt->AdvancedConfig();
		return true;
	}
	else if (!StrCmpI(Command.data(), L"far:about"))
	{
		string strOut(L"\n");

		strOut.append(Global->Version()).append(1, L'\n');
		strOut.append(Global->Copyright()).append(1, L'\n');

		const auto& ComponentsInfo = components::GetComponentsInfo();
		if (!ComponentsInfo.empty())
		{
			strOut += L"\nLibraries:\n";

			FOR(const auto& i, ComponentsInfo)
			{
				strOut.append(i).append(1, L'\n');
			}
		}

		// TODO: enum model adapters

		if (Global->CtrlObject->Plugins->size())
		{
			strOut += L"\nPlugins:\n";

			FOR(const auto& i, *Global->CtrlObject->Plugins)
			{
				strOut.append(i->GetTitle()).append(L", version ").append(i->GetVersionString()).append(1, L'\n');
			}
		}

		ExecutionContext.ConsoleContext();
		Console().Write(strOut);

		return true;
	}

	return false;
}

void CommandLine::ExecString(execute_info& Info)
{
	bool Silent = false;
	bool IsUpdateNeeded = false;

	SCOPE_EXIT
	{
		if (!IsUpdateNeeded)
			return;

		if (!m_Flags.Check(FCMDOBJ_LOCKUPDATEPANEL))
		{
			ShellUpdatePanels(Global->CtrlObject->Cp()->ActivePanel(), FALSE);
			if (Global->Opt->ShowKeyBar)
			{
				Global->CtrlObject->Cp()->GetKeybar().Show();
			}
		}
		if (Global->Opt->Clock)
			ShowTime(0);

		if (!Silent)
		{
			Global->ScrBuf->Flush();
		}
	};

	execution_context ExecutionContext(Info.Command);

	if (Info.Command.empty())
	{
		// Just scroll the screen
		ExecutionContext.Activate();
		return;
	}

	LastCmdPartLength = -1;

	FarChDir(m_CurDir);

	if (!Info.DirectRun)
	{
		if (!Info.Command.empty() && Info.Command[0] == L'@')
		{
			Info.Command.erase(0, 1);
			Silent=true;
		}

		ExpandOSAliases(Info.Command);

		if (!ExtractIfExistCommand(Info.Command))
			return;

		if (!Info.NewWindow && !Info.RunAs)
		{
			if (ProcessFarCommands(Info.Command, ExecutionContext))
				return;

			if (Global->CtrlObject->Plugins->ProcessCommandLine(Info.Command))
				return;

			if (ProcessOSCommands(Info.Command, ExecutionContext))
				return;
		}
	}


	SCOPED_ACTION(ConsoleTitle);
	if (!Silent)
	{
		ConsoleTitle::SetFarTitle(ExecutionContext.Command());
	}

	ExecutionContext.Activate();
	Execute(Info, false, Silent, [&ExecutionContext](){ ExecutionContext.ConsoleContext(); });
	
	// BUGBUG do we really need to update panels at all?
	IsUpdateNeeded = true;
}

bool CommandLine::ProcessOSCommands(const string& CmdLine, class execution_context& ExecutionContext)
{
	auto SetPanel = Global->CtrlObject->Cp()->ActivePanel();
	
	if (SetPanel->GetType() != panel_type::FILE_PANEL && Global->CtrlObject->Cp()->PassivePanel()->GetType() == panel_type::FILE_PANEL)
		SetPanel=Global->CtrlObject->Cp()->PassivePanel();

	const auto IsCommand = [&CmdLine](const string& cmd, bool bslash)->bool
	{
		const auto n = cmd.size();
		return (!StrCmpNI(CmdLine.data(), cmd.data(), n)
			&& (n == CmdLine.size() || nullptr != wcschr(L"/ \t", CmdLine[n]) || (bslash && CmdLine[n] == L'\\')));
	};

	const auto FindKey = [&CmdLine](wchar_t Key) -> bool
	{
		const auto FirstSpacePos = CmdLine.find(L' ');
		const auto NotSpacePos = CmdLine.find_first_not_of(L' ', FirstSpacePos);

		return NotSpacePos != string::npos &&
			CmdLine.size() > NotSpacePos + 1 &&
			CmdLine[NotSpacePos] == L'/' &&
			ToUpper(CmdLine[NotSpacePos + 1]) == ToUpper(Key);
	};

	const auto FindHelpKey = [&FindKey]() { return FindKey(L'?'); };

	ExecutionContext.Activate();

	if (CmdLine.size() > 1 && CmdLine[1] == L':' && (CmdLine.size() == 2 || !CmdLine.find_first_not_of(L' ', 2)))
	{
		wchar_t NewDir[] = { ToUpper(CmdLine[0]), L':', 0, 0 };

		if (!FarChDir(NewDir))
		{
			NewDir[2] = L'\\';
			FarChDir(NewDir);
		}
		SetPanel->ChangeDirToCurrent();
		return true;
	}

	if (FindHelpKey())
		return false;

	// SET [variable=[value]]
	if (IsCommand(L"SET",false))
	{
		if (FindKey(L'A') || FindKey(L'P'))
			return false; //todo: /p - dialog, /a - calculation; then set variable ...

		size_t pos;
		auto strCmdLine = CmdLine.substr(3);
		RemoveLeadingSpaces(strCmdLine);

		// "set" (display all) or "set var" (display all that begin with "var")
		if (strCmdLine.empty() || ((pos = strCmdLine.find(L'=')) == string::npos) || !pos)
		{
			//forward "set [prefix]| command" and "set [prefix]> file" to COMSPEC
			static const wchar_t CharsToFind[] = L"|>";
			if (std::find_first_of(ALL_CONST_RANGE(strCmdLine), ALL_CONST_RANGE(CharsToFind)) != strCmdLine.cend())
				return false;

			string strOut;
			FOR(const auto& i, os::env::enum_strings())
			{
				if (!StrCmpNI(i.data(), strCmdLine.data(), strCmdLine.size()))
				{
					strOut.append(i.data(), i.size()).append(L"\n");
				}
			}

			if (!strOut.empty())
			{
				ExecutionContext.ConsoleContext();
				Console().Write(strOut);
			}

			return true;
		}

		if (strCmdLine.size() == pos+1) //set var=
		{
			strCmdLine.resize(pos);
			os::env::delete_variable(strCmdLine);
		}
		else
		{
			const auto strExpandedStr = os::env::expand_strings(strCmdLine.substr(pos + 1));
			strCmdLine.resize(pos);
			os::env::set_variable(strCmdLine, strExpandedStr);
		}

		return true;
	}
	else if (IsCommand(L"CLS",false))
	{
		ClearScreen(colors::PaletteColorToFarColor(COL_COMMANDLINEUSERSCREEN));
		return true;
	}
	// PUSHD путь | ..
	else if (IsCommand(L"PUSHD",false))
	{
		auto strCmdLine = CmdLine.substr(5);
		RemoveLeadingSpaces(strCmdLine);

		const auto PushDir = m_CurDir;

		if (IntChDir(strCmdLine, true))
		{
			ppstack.push(PushDir);
			os::env::set_variable(L"FARDIRSTACK", PushDir);
		}
		else
		{
			;
		}

		return true;
	}
	// POPD
	// TODO: добавить необязательный параметр - число, сколько уровней пропустить, после чего прыгнуть.
	else if (IsCommand(L"POPD",false))
	{
		if (!ppstack.empty())
		{
			const auto Result = IntChDir(ppstack.top(), true);
			ppstack.pop();
			if (!ppstack.empty())
			{
				os::env::set_variable(L"FARDIRSTACK", ppstack.top());
			}
			else
			{
				os::env::delete_variable(L"FARDIRSTACK");
			}
			return Result;
		}

		return true;
	}
	// CLRD
	else if (IsCommand(L"CLRD",false))
	{
		clear_and_shrink(ppstack);
		os::env::delete_variable(L"FARDIRSTACK");
		return true;
	}
	/*
		Displays or sets the active code page number.
		CHCP [nnn]
			nnn   Specifies a code page number (Dec or Hex).
		Type CHCP without a parameter to display the active code page number.
	*/
	else if (IsCommand(L"CHCP",false))
	{
		auto strCmdLine = CmdLine.substr(4);
		RemoveExternalSpaces(strCmdLine);
		uintptr_t cp;
		try
		{
			cp = std::stoul(strCmdLine);
		}
		catch (const std::exception&)
		{
			return false;
		}

		if (!Console().SetInputCodepage(cp) || !Console().SetOutputCodepage(cp))
			return false;

		Text(strCmdLine);
		ScrollScreen(1);
		return true;
	}
	else if (IsCommand(L"CD",true) || IsCommand(L"CHDIR",true))
	{
		const int Length = IsCommand(L"CD", true)? 2 : 5;

		auto strCmdLine = CmdLine.substr(Length);
		RemoveLeadingSpaces(strCmdLine);

		//проигнорируем /D
		//мы и так всегда меняем диск а некоторые в алайсах или по привычке набирают этот ключ
		if (!StrCmpNI(strCmdLine.data(),L"/D",2) && IsSpaceOrEos(strCmdLine[2]))
		{
			strCmdLine.erase(0, 2);
			RemoveLeadingSpaces(strCmdLine);
		}

		if (strCmdLine.empty())
			return false;

		IntChDir(strCmdLine, Length == 5);
		return true;
	}
	else if (IsCommand(L"TITLE", false))
	{
		auto Title = CmdLine.data() + 5; // wcslen(L"title")

		SetUserTitle(*Title? Title + 1 : Title);

		if (!(Global->CtrlObject->Cp()->LeftPanel()->IsVisible() || Global->CtrlObject->Cp()->RightPanel()->IsVisible()))
		{
			Global->CtrlObject->Cp()->ActivePanel()->SetTitle();
		}
		return true;
	}
	else if (IsCommand(L"EXIT",false))
	{
		Global->WindowManager->ExitMainLoop(FALSE);
		return true;
	}

	return false;
}

bool CommandLine::IntChDir(const string& CmdLine,int ClosePanel,bool Selent)
{
	auto SetPanel = Global->CtrlObject->Cp()->ActivePanel();

	if (SetPanel->GetType() != panel_type::FILE_PANEL && Global->CtrlObject->Cp()->PassivePanel()->GetType() == panel_type::FILE_PANEL)
		SetPanel=Global->CtrlObject->Cp()->PassivePanel();

	string strExpandedDir = Unquote(os::env::expand_strings(CmdLine));

	if (SetPanel->GetMode() != panel_mode::PLUGIN_PANEL && strExpandedDir[0] == L'~' && ((strExpandedDir.size() == 1 && !os::fs::exists(strExpandedDir)) || IsSlash(strExpandedDir[1])))
	{
		if (Global->Opt->Exec.UseHomeDir && !Global->Opt->Exec.strHomeDir.empty())
		{
			string strTemp=Global->Opt->Exec.strHomeDir.Get();

			if (strExpandedDir.size() > 1)
			{
				AddEndSlash(strTemp);
				strTemp += strExpandedDir.data()+2;
			}

			DeleteEndSlash(strTemp);
			strExpandedDir = os::env::expand_strings(strTemp);
		}
	}

	size_t DirOffset = 0;
	ParsePath(strExpandedDir, &DirOffset);
	if (strExpandedDir.find_first_of(L"?*", DirOffset) != string::npos) // это маска?
	{
		os::FAR_FIND_DATA wfd;

		if (os::GetFindDataEx(strExpandedDir, wfd))
		{
			const auto pos = FindLastSlash(strExpandedDir);
			if (pos != string::npos)
			{
				strExpandedDir.resize(pos + 1);
			}
			else
			{
				strExpandedDir.clear();
			}
			strExpandedDir += wfd.strFileName;
		}
	}

	/* $ 15.11.2001 OT
		Сначала проверяем есть ли такая "обычная" директория.
		если уж нет, то тогда начинаем думать, что это директория плагинная
	*/
	if (os::fs::is_directory(strExpandedDir) && IsAbsolutePath(strExpandedDir))
	{
		ReplaceSlashToBackslash(strExpandedDir);
		SetPanel->SetCurDir(strExpandedDir,true);
		return true;
	}

	if (SetPanel->GetType() == panel_type::FILE_PANEL && SetPanel->GetMode() == panel_mode::PLUGIN_PANEL)
	{
		SetPanel->SetCurDir(strExpandedDir,ClosePanel!=0);
		return true;
	}

	if (FarChDir(strExpandedDir))
	{
		SetPanel->ChangeDirToCurrent();

		if (!SetPanel->IsVisible())
			SetPanel->SetTitle();
	}
	else
	{
		Global->CatchError();
		if (!Selent)
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strExpandedDir.data(),MSG(MOk));

		return false;
	}

	return true;
}

void CommandLine::LockUpdatePanel(bool Mode)
{
	m_Flags.Change(FCMDOBJ_LOCKUPDATEPANEL,Mode);
}
