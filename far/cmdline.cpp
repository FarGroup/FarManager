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
#include "rdrwdsk.hpp"
#include "savescr.hpp"
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

enum
{
	FCMDOBJ_LOCKUPDATEPANEL   = 0x00010000,
	DEFAULT_CMDLINE_WIDTH = 50,
};

CommandLine::CommandLine():
	PromptSize(DEFAULT_CMDLINE_WIDTH),
	CmdStr(Global->CtrlObject->Cp(),0,true,Global->CtrlObject->CmdHistory,0,(Global->Opt->CmdLine.AutoComplete?EditControl::EC_ENABLEAUTOCOMPLETE:0)|EditControl::EC_COMPLETE_HISTORY|EditControl::EC_COMPLETE_FILESYSTEM|EditControl::EC_COMPLETE_PATH),
	BackgroundScreen(nullptr),
	LastCmdPartLength(-1)
{
	CmdStr.SetEditBeyondEnd(false);
	CmdStr.SetMacroAreaAC(MACROAREA_SHELLAUTOCOMPLETION);
	SetPersistentBlocks(Global->Opt->CmdLine.EditBlock);
	SetDelRemovesBlocks(Global->Opt->CmdLine.DelRemovesBlocks);
}

CommandLine::~CommandLine()
{
	delete BackgroundScreen;

	Global->ScrBuf->Flush(true);
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

void CommandLine::DisplayObject()
{
	_OT(SysLog(L"[%p] CommandLine::DisplayObject()",this));
	auto PromptList = GetPrompt();
	size_t MaxLength = PromptSize*ObjWidth()/100;
	size_t CurLength = 0;
	GotoXY(X1,Y1);

	std::for_each(CONST_RANGE(PromptList, i)
	{
		SetColor(i.second);
		string str(i.first);
		if(CurLength + str.size() > MaxLength)
		TruncPathStr(str, std::max(0, static_cast<int>(MaxLength - CurLength)));
		Text(str);
		CurLength += str.size();
	});

	CmdStr.SetObjectColor(COL_COMMANDLINE,COL_COMMANDLINESELECTED);

	CmdStr.SetPosition(X1+(int)CurLength,Y1,X2,Y2);

	CmdStr.Show();

	GotoXY(X2+1,Y1);
	SetColor(COL_COMMANDLINEPREFIX);
	Text(L"\x2191");
}


void CommandLine::SetCurPos(int Pos, int LeftPos, bool Redraw)
{
	CmdStr.SetLeftPos(LeftPos);
	CmdStr.SetCurPos(Pos);
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

int CommandLine::ProcessKey(int Key)
{
	const wchar_t *PStr;
	string strStr;

	if ((Key==KEY_CTRLEND || Key==KEY_RCTRLEND || Key==KEY_CTRLNUMPAD1 || Key==KEY_RCTRLNUMPAD1) && (CmdStr.GetCurPos()==CmdStr.GetLength()))
	{
		if (LastCmdPartLength==-1)
			strLastCmdStr = CmdStr.GetStringAddr();

		strStr = strLastCmdStr;
		int CurCmdPartLength=(int)strStr.size();
		Global->CtrlObject->CmdHistory->GetSimilar(strStr,LastCmdPartLength);

		if (LastCmdPartLength==-1)
		{
			strLastCmdStr = CmdStr.GetStringAddr();
			LastCmdPartLength=CurCmdPartLength;
		}

		{
			SCOPED_ACTION(SetAutocomplete)(&CmdStr);
			CmdStr.SetString(strStr.data());
			CmdStr.Select(LastCmdPartLength,static_cast<int>(strStr.size()));
		}

		Show();
		return TRUE;
	}

	if (Key == KEY_UP || Key == KEY_NUMPAD8)
	{
		if (Global->CtrlObject->Cp()->LeftPanel->IsVisible() || Global->CtrlObject->Cp()->RightPanel->IsVisible())
			return FALSE;

		Key=KEY_CTRLE;
	}
	else if (Key == KEY_DOWN || Key == KEY_NUMPAD2)
	{
		if (Global->CtrlObject->Cp()->LeftPanel->IsVisible() || Global->CtrlObject->Cp()->RightPanel->IsVisible())
			return FALSE;

		Key=KEY_CTRLX;
	}

	// $ 25.03.2002 VVM + При погашенных панелях колесом крутим историю
	if (!Global->CtrlObject->Cp()->LeftPanel->IsVisible() && !Global->CtrlObject->Cp()->RightPanel->IsVisible())
	{
		switch (Key)
		{
			case KEY_MSWHEEL_UP:    Key = KEY_CTRLE; break;
			case KEY_MSWHEEL_DOWN:  Key = KEY_CTRLX; break;
			case KEY_MSWHEEL_LEFT:  Key = KEY_CTRLS; break;
			case KEY_MSWHEEL_RIGHT: Key = KEY_CTRLD; break;
		}
	}

	switch (Key)
	{
		case KEY_CTRLE:
		case KEY_RCTRLE:
		case KEY_CTRLX:
		case KEY_RCTRLX:
			{
				if (Key == KEY_CTRLE || Key == KEY_RCTRLE)
				{
					Global->CtrlObject->CmdHistory->GetPrev(strStr);
				}
				else
				{
					Global->CtrlObject->CmdHistory->GetNext(strStr);
				}

				{
					SCOPED_ACTION(SetAutocomplete)(&CmdStr);
					SetString(strStr);
				}

			}
			return TRUE;

		case KEY_ESC:

			if (Key == KEY_ESC)
			{
				// $ 24.09.2000 SVS - Если задано поведение по "Несохранению при Esc", то позицию в хистори не меняем и ставим в первое положение.
				if (Global->Opt->CmdHistoryRule)
					Global->CtrlObject->CmdHistory->ResetPosition();

				PStr=L"";
			}
			else
				PStr=strStr.data();

			SetString(PStr);
			return TRUE;
		case KEY_F2:
		{
			UserMenu Menu(false);
			return TRUE;
		}
		case KEY_ALTF8:
		case KEY_RALTF8:
		{
			history_record_type Type;
			auto SelectType = Global->CtrlObject->CmdHistory->Select(MSG(MHistoryTitle), L"History", strStr, Type);
			if (SelectType == HRT_ENTER || SelectType == HRT_SHIFTETNER || SelectType == HRT_CTRLENTER || SelectType == HRT_CTRLALTENTER)
			{
				std::unique_ptr<SetAutocomplete> disable;
				if(SelectType != HRT_CTRLENTER)
				{
					disable = std::make_unique<SetAutocomplete>(&CmdStr);
				}
				SetString(strStr);

				if (SelectType != HRT_CTRLENTER)
				{
					ProcessKey(SelectType == HRT_CTRLALTENTER? static_cast<int>(KEY_CTRLALTENTER) : (SelectType == HRT_ENTER? static_cast<int>(KEY_ENTER) : static_cast<int>(KEY_SHIFTENTER)));
				}
			}
		}
		return TRUE;
		case KEY_SHIFTF9:
			Global->Opt->Save(true);
			return TRUE;
		case KEY_F10:
			Global->FrameManager->ExitMainLoop(TRUE);
			return TRUE;
		case KEY_ALTF10:
		case KEY_RALTF10:
		{
			Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
			{
				// TODO: здесь можно добавить проверку, что мы в корне диска и отсутствие файла Tree.Far...
				FolderTree Tree(strStr, MODALTREE_ACTIVE, TRUE, false);
			}
			Global->CtrlObject->Cp()->RedrawKeyBar();

			if (!strStr.empty())
			{
				ActivePanel->SetCurDir(strStr,true);
				ActivePanel->Show();

				if (ActivePanel->GetType()==TREE_PANEL)
					ActivePanel->ProcessKey(KEY_ENTER);
			}
			else
			{
				// TODO: ... а здесь проверить факт изменения/появления файла Tree.Far и мы опять же в корне (чтобы лишний раз не апдейтить панель)
				ActivePanel->Update(UPDATE_KEEP_SELECTION);
				ActivePanel->Redraw();
				Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

				if (AnotherPanel->NeedUpdatePanel(ActivePanel))
				{
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);//|UPDATE_SECONDARY);
					AnotherPanel->Redraw();
				}
			}
		}
		return TRUE;
		case KEY_F11:
			Global->CtrlObject->Plugins->CommandsMenu(FALSE,FALSE,0);
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
			string strFile, strData;
			auto SelectType = Global->CtrlObject->FolderHistory->Select(MSG(MFolderHistoryTitle), L"HistoryFolders", strStr, Type, &Guid, &strFile, &strData);

			switch(SelectType)
			{
			case HRT_ENTER:
			case HRT_SHIFTETNER:
			case HRT_CTRLSHIFTENTER:
				{

					if (SelectType == HRT_SHIFTETNER)
						Global->CtrlObject->FolderHistory->SetAddMode(false,2,true);

					// пусть плагин сам прыгает... ;-)
					Panel *Panel=Global->CtrlObject->Cp()->ActivePanel;

					if (SelectType == HRT_CTRLSHIFTENTER)
						Panel=Global->CtrlObject->Cp()->GetAnotherPanel(Panel);

					//Type==1 - плагиновый путь
					//Type==0 - обычный путь
					Panel->ExecShortcutFolder(strStr,Guid,strFile,strData,true);
					// Panel may be changed
					if(SelectType == HRT_CTRLSHIFTENTER)
					{
						Panel=Global->CtrlObject->Cp()->ActivePanel;
						Panel->SetCurPath();
						Panel=Global->CtrlObject->Cp()->GetAnotherPanel(Panel);
					}
					else
					{
						Panel=Global->CtrlObject->Cp()->ActivePanel;
					}
					Panel->Redraw();
					Global->CtrlObject->FolderHistory->SetAddMode(true,2,true);
				}
				break;

			case HRT_CTRLENTER:
				SetString(strStr);
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
			Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
			CmdStr.Select(-1,0);
			CmdStr.Show();
			CmdStr.GetString(strStr);

			if (strStr.empty())
				break;

			ActivePanel->SetCurPath();

			if (!(Global->Opt->ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTCMDLINE))
				Global->CtrlObject->CmdHistory->AddToHistory(strStr, HR_DEFAULT, nullptr, nullptr, strCurDir.data());


			if (!ActivePanel->ProcessPluginEvent(FE_COMMAND, UNSAFE_CSTR(strStr.data())))
				ExecString(strStr, false, Key==KEY_SHIFTENTER||Key==KEY_SHIFTNUMENTER, false, false,
						Key == KEY_CTRLALTENTER || Key == KEY_RCTRLRALTENTER || Key == KEY_CTRLRALTENTER || Key == KEY_RCTRLALTENTER ||
						Key == KEY_CTRLALTNUMENTER || Key == KEY_RCTRLRALTNUMENTER || Key == KEY_CTRLRALTNUMENTER || Key == KEY_RCTRLALTNUMENTER);
		}
		return TRUE;
		case KEY_CTRLU:
		case KEY_RCTRLU:
			CmdStr.Select(-1,0);
			CmdStr.Show();
			return TRUE;
		case KEY_OP_XLAT:
		{
			// 13.12.2000 SVS - ! Для CmdLine - если нет выделения, преобразуем всю строку (XLat)
			CmdStr.Xlat((Global->Opt->XLat.Flags&XLAT_CONVERTALLCMDLINE) != 0);

			// иначе неправильно работает ctrl-end
			strLastCmdStr = CmdStr.GetStringAddr();
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
			Key&=~(KEY_ALT|KEY_RALT);
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

				if (std::find(ALL_CONST_RANGE(UnmarkKeys), Key) != std::cend(UnmarkKeys))
				{
					CmdStr.Select(-1,0);
				}
			}

			if (Key == KEY_CTRLD || Key == KEY_RCTRLD)
				Key=KEY_RIGHT;

			if(Key == KEY_CTRLSPACE || Key == KEY_RCTRLSPACE)
			{
				SCOPED_ACTION(SetAutocomplete)(&CmdStr, true);
				CmdStr.AutoComplete(true,false);
				return TRUE;
			}

			if (!CmdStr.ProcessKey(Key))
				break;

			LastCmdPartLength=-1;

			return TRUE;
	}

	return FALSE;
}


void CommandLine::SetCurDir(const string& CurDir)
{
	if (StrCmpI(strCurDir, CurDir) || !TestCurrentDirectory(CurDir))
	{
		strCurDir = CurDir;

		//Mantis#2350 - тормоз, это и так делается выше
		//if (Global->CtrlObject->Cp()->ActivePanel->GetMode()!=PLUGIN_PANEL)
			//PrepareDiskPath(strCurDir);
	}
}


int CommandLine::GetCurDir(string &strCurDir)
{
	strCurDir = CommandLine::strCurDir;
	return (int)strCurDir.size();
}


void CommandLine::SetString(const string& Str, bool Redraw)
{
	LastCmdPartLength=-1;
	CmdStr.SetString(Str.data());
	CmdStr.SetLeftPos(0);

	if (Redraw)
		CmdStr.Show();
}

void CommandLine::InsertString(const string& Str)
{
	LastCmdPartLength=-1;
	CmdStr.InsertString(Str);
	CmdStr.Show();
}


int CommandLine::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if(MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED && MouseEvent->dwMousePosition.X==X2+1)
	{
		return ProcessKey(KEY_ALTF8);
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
		if (Upper(Color[0]) == L'T')
		{
			TargetFlags &= ~SetFlag;
			Target = ARGB2ABGR(wcstoul(Color.data() + 1, nullptr, 16));
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
	std::list<std::pair<string, FarColor>> Result;
	int NewPromptSize = DEFAULT_CMDLINE_WIDTH;

	FarColor PrefixColor(ColorIndexToColor(COL_COMMANDLINEPREFIX));

	if (Global->Opt->CmdLine.UsePromptFormat)
	{
		string Format(Global->Opt->CmdLine.strPromptFormat.Get());
		FarColor F(PrefixColor);
		string Str(Format);
		FOR_CONST_RANGE(Format, Ptr)
		{
			// color in ([[T]ffffffff][:[T]bbbbbbbb]) format
			if (*Ptr == L'(')
			{
				string Color = &*Ptr + 1;
				size_t Pos = Color.find(L')');
				if(Pos != string::npos)
				{
					if (Ptr != Format.cbegin())
					{
						size_t PrevPos = Str.find(L'(');
						Str.resize(PrevPos);
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
			string& strDestStr = i.first;
			string strExpandedDestStr;
			strExpandedDestStr = api::ExpandEnvironmentStrings(strDestStr);
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
					wchar_t Chr = Upper(*++it);

					auto ItemIterator = std::find_if(CONST_RANGE(ChrFmt, i)
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
								if (DriveLocalToRemoteName(DRIVE_REMOTE,strCurDir[0],strTemp))
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
										if (Global->IsUserAdmin())
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
								string strDateTime;
								MkStrFTime(strDateTime,(Chr==L'D'?L"%D":L"%T"));
								strDestStr += strDateTime;
								break;
							}
							case L'N': // $N - Current drive
							{
								PATH_TYPE Type = ParsePath(strCurDir);
								if(Type == PATH_DRIVELETTER)
									strDestStr += Upper(strCurDir[0]);
								else if(Type == PATH_DRIVELETTERUNC)
									strDestStr += Upper(strCurDir[4]);
								else
									strDestStr += L'?';
								break;
							}
							case L'W': // $W - Текущий рабочий каталог (без указания пути)
							{
								const wchar_t *ptrCurDir=LastSlash(strCurDir.data());
								if (ptrCurDir)
									strDestStr += ptrCurDir+1;
								break;
							}
							case L'P': // $P - Current drive and path
							{
								strDestStr += strCurDir;
								break;
							}
							case L'#': //$#nn - max promt width in %
							{
								if (it + 1 != strExpandedDestStr.end())
								{
									try
									{
										size_t pos = 0;
										NewPromptSize = std::stoi(string(++it, strExpandedDestStr.cend()), &pos);
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
		Result.emplace_back(VALUE_TYPE(Result)(strCurDir + L">", PrefixColor));
	}
	SetPromptSize(NewPromptSize);
	return Result;
}


void CommandLine::ShowViewEditHistory()
{
	string strStr;
	history_record_type Type;
	auto SelectType = Global->CtrlObject->ViewHistory->Select(MSG(MViewHistoryTitle), L"HistoryViews", strStr, Type);

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
					new FileViewer(strStr,TRUE);
					break;
				}

				case HR_EDITOR:
				case HR_EDITOR_RO:
				{
					// пусть файл создается
					FileEditor *FEdit=new FileEditor(strStr,CP_DEFAULT,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6);

					if (Type == HR_EDITOR_RO)
						FEdit->SetLockEditor(TRUE);

					break;
				}

				case HR_EXTERNAL:
				case HR_EXTERNAL_WAIT:
				{
					ExecString(strStr, Type == HR_EXTERNAL_WAIT, false, false, false, false, true);
					break;
				}
			}

			Global->CtrlObject->ViewHistory->SetAddMode(true,Global->Opt->FlagPosixSemantics?1:2,true);
		}
		break;

	case HRT_CTRLENTER:
		SetString(strStr);
		break;

	default:
		break;
	}
}

void CommandLine::SaveBackground(int X1,int Y1,int X2,int Y2)
{
	delete BackgroundScreen;
	BackgroundScreen=new SaveScreen(X1,Y1,X2,Y2);
}

void CommandLine::SaveBackground()
{
	if (BackgroundScreen)
	{
//		BackgroundScreen->Discard();
		BackgroundScreen->SaveArea();
	}
}
void CommandLine::ShowBackground()
{
	if (BackgroundScreen)
	{
		BackgroundScreen->RestoreArea();
	}
}

void CommandLine::CorrectRealScreenCoord()
{
	if (BackgroundScreen)
	{
		BackgroundScreen->CorrectRealScreenCoord();
	}
}

void CommandLine::ResizeConsole()
{
	BackgroundScreen->Resize(ScrX+1,ScrY+1,2,Global->Opt->WindowMode!=FALSE);
//  this->DisplayObject();
}

void CommandLine::SetPromptSize(int NewSize)
{
	PromptSize = NewSize? std::max(5, std::min(95, NewSize)) : DEFAULT_CMDLINE_WIDTH;
}

int CommandLine::ExecString(const string& InputCmdLine, bool AlwaysWaitFinish, bool SeparateWindow, bool DirectRun, bool WaitForIdle, bool RunAs, bool RestoreCmd)
{
	class preservecmdline
	{
	public:
		preservecmdline(bool Preserve):
			Preserve(Preserve)
		{
			if(Preserve)
			{
				OldCmdLineCurPos = Global->CtrlObject->CmdLine->GetCurPos();
				OldCmdLineLeftPos = Global->CtrlObject->CmdLine->GetLeftPos();
				Global->CtrlObject->CmdLine->GetString(strOldCmdLine);
				Global->CtrlObject->CmdLine->GetSelection(OldCmdLineSelStart,OldCmdLineSelEnd);
			}
		}
		~preservecmdline()
		{
			if(Preserve)
			{
				bool redraw = Global->FrameManager->IsPanelsActive();
				Global->CtrlObject->CmdLine->SetString(strOldCmdLine, redraw);
				Global->CtrlObject->CmdLine->SetCurPos(OldCmdLineCurPos, OldCmdLineLeftPos, redraw);
				Global->CtrlObject->CmdLine->Select(OldCmdLineSelStart, OldCmdLineSelEnd);
			}
		}
	private:
		bool Preserve;
		string strOldCmdLine;
		int OldCmdLineCurPos, OldCmdLineLeftPos;
		intptr_t OldCmdLineSelStart, OldCmdLineSelEnd;
	}
	PreserveCmdline(RestoreCmd);

	string CmdLine(InputCmdLine);

	bool Silent=false;

	if (!CmdLine.empty() && CmdLine[0] == L'@')
	{
		CmdLine.erase(0, 1);
		Silent=true;
	}

	{
		SCOPED_ACTION(SetAutocomplete)(&CmdStr);
		SetString(CmdLine);
	}

	ProcessOSAliases(CmdLine);

	LastCmdPartLength=-1;

	if(!StrCmpI(CmdLine.data(),L"far:config"))
	{
		SetString(L"", false);
		Show();
		return Global->Opt->AdvancedConfig();
	}

	if (!SeparateWindow && Global->CtrlObject->Plugins->ProcessCommandLine(CmdLine))
	{
		/* $ 12.05.2001 DJ - рисуемся только если остались верхним фреймом */
		if (Global->CtrlObject->Cp()->IsTopFrame())
		{
			//CmdStr.SetString(L"");
			GotoXY(X1,Y1);
			Global->FS << fmt::MinWidth(X2-X1+1)<<L"";
			Show();
			Global->ScrBuf->Flush();
		}

		return -1;
	}

	int Code;
	COORD Size0;
	Global->Console->GetSize(Size0);

	if (strCurDir.size() > 1 && strCurDir[1]==L':')
		FarChDir(strCurDir);

	string strPrevDir=strCurDir;
	bool PrintCommand=true;
	if ((Code=ProcessOSCommands(CmdLine,SeparateWindow,PrintCommand)) == TRUE)
	{
		if (PrintCommand)
		{
			ShowBackground();
			string strNewDir=strCurDir;
			strCurDir=strPrevDir;
			Redraw();
			strCurDir=strNewDir;
			GotoXY(X2+1,Y1);
			Text(L" ");
			ScrollScreen(2);
			SaveBackground();
		}

		SetString(L"", false);

		Code=-1;
	}
	else
	{
		string strTempStr;
		strTempStr = CmdLine;

		if (Code == -1)
			ReplaceStrings(strTempStr,L"/",L"\\",-1);

		Code=Execute(strTempStr,AlwaysWaitFinish,SeparateWindow,DirectRun, 0, WaitForIdle, Silent, RunAs);
	}

	COORD Size1;
	Global->Console->GetSize(Size1);

	if (Size0.X != Size1.X || Size0.Y != Size1.Y)
	{
		GotoXY(X2+1,Y1);
		Text(L" ");
		Global->CtrlObject->CmdLine->CorrectRealScreenCoord();
	}
	if (!Flags.Check(FCMDOBJ_LOCKUPDATEPANEL))
	{
		ShellUpdatePanels(Global->CtrlObject->Cp()->ActivePanel,FALSE);
		if (Global->Opt->ShowKeyBar)
		{
			Global->CtrlObject->MainKeyBar->Show();
		}
	}
	if (Global->Opt->Clock)
		ShowTime(0);

	if (!Silent)
	{
		Global->ScrBuf->Flush();
	}
	return Code;
}

int CommandLine::ProcessOSCommands(const string& CmdLine, bool SeparateWindow, bool &PrintCommand)
{
	string strCmdLine = CmdLine;
	Panel *SetPanel=Global->CtrlObject->Cp()->ActivePanel;
	PrintCommand=true;

	if (SetPanel->GetType()!=FILE_PANEL && Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
		SetPanel=Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel);

	RemoveTrailingSpaces(strCmdLine);
	bool SilentInt=false;

	if (!CmdLine.empty() && CmdLine[0] == L'@')
	{
		SilentInt=true;
		strCmdLine.erase(0, 1);
	}

	auto IsCommand = [&strCmdLine](const string& cmd, bool bslash)->bool
	{
		size_t n = cmd.size();
		return (!StrCmpNI(strCmdLine.data(), cmd.data(), n)
		 && (n==strCmdLine.size() || nullptr != wcschr(L"/ \t",strCmdLine[n]) || (bslash && strCmdLine[n]==L'\\')));
	};

	if (!SeparateWindow && strCmdLine.size() == 2 && strCmdLine[1]==L':')
	{
		if(!FarChDir(strCmdLine))
		{
			wchar_t NewDir[]={Upper(strCmdLine[0]),L':',L'\\',0};
			FarChDir(NewDir);
		}
		SetPanel->ChangeDirToCurrent();
		return TRUE;
	}
	// SET [переменная=[строка]]
	else if (IsCommand(L"SET",false))
	{
		size_t pos;
		strCmdLine.erase(0, 3);
		RemoveLeadingSpaces(strCmdLine);

		if (CheckCmdLineForHelp(strCmdLine.data()))
			return FALSE; // отдадимся COMSPEC`у

		// "set" (display all) or "set var" (display all that begin with "var")
		if (strCmdLine.empty() || ((pos = strCmdLine.find(L'=')) == string::npos) || !pos)
		{
			//forward "set [prefix]| command" and "set [prefix]> file" to COMSPEC
			static const wchar_t CharsToFind[] = L"|>";
			if (std::find_first_of(ALL_CONST_RANGE(strCmdLine), ALL_CONST_RANGE(CharsToFind)) != strCmdLine.cend())
				return FALSE;

			ShowBackground();  //??? почему не отдаём COMSPEC'у
			// display command //???
			Redraw();
			GotoXY(X2+1,Y1);
			Text(L" ");
			Global->ScrBuf->Flush();
			Global->Console->SetTextAttributes(ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));
			string strOut(L"\n");
			int CmdLength = static_cast<int>(strCmdLine.size());
			LPWCH Environment = GetEnvironmentStrings();
			for (LPCWSTR Ptr = Environment; *Ptr;)
			{
				int PtrLength = StrLength(Ptr);
				if (!StrCmpNI(Ptr, strCmdLine.data(), CmdLength))
				{
					strOut.append(Ptr, PtrLength).append(L"\n");
				}
				Ptr+=PtrLength+1;
			}
			FreeEnvironmentStrings(Environment);
			strOut.append(L"\n\n", Global->Opt->ShowKeyBar?2:1);
			Global->Console->Write(strOut);
			Global->Console->Commit();
			Global->ScrBuf->FillBuf();
			SaveBackground();
			PrintCommand = false;
			return TRUE;
		}

		if (CheckCmdLineForSet(strCmdLine)) // вариант для /A и /P
			return FALSE; //todo: /p - dialog, /a - calculation; then set variable ...

		if (strCmdLine.size() == pos+1) //set var=
		{
			strCmdLine.resize(pos);
			api::DeleteEnvironmentVariable(strCmdLine);
		}
		else
		{
			string strExpandedStr = api::ExpandEnvironmentStrings(strCmdLine.substr(pos+1));
			strCmdLine.resize(pos);
			api::SetEnvironmentVariable(strCmdLine, strExpandedStr);
		}

		return TRUE;
	}
	// REM все остальное
	else if (IsCommand(L"REM",false))
	{
		if (CheckCmdLineForHelp(strCmdLine.data()+3))
			return FALSE; // отдадимся COMSPEC`у
	}
	else if (!strCmdLine.compare(0, 2, L"::", 2))
	{
		return TRUE;
	}
	else if (IsCommand(L"CLS",false))
	{
		if (CheckCmdLineForHelp(strCmdLine.data()+3))
			return FALSE; // отдадимся COMSPEC`у

		ClearScreen(ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));
		SaveBackground();
		PrintCommand=false;
		return TRUE;
	}
	// PUSHD путь | ..
	else if (IsCommand(L"PUSHD",false))
	{
		strCmdLine.erase(0, 5);
		RemoveLeadingSpaces(strCmdLine);

		if (CheckCmdLineForHelp(strCmdLine.data()))
			return FALSE; // отдадимся COMSPEC`у

		string PushDir = strCurDir;

		if (IntChDir(strCmdLine,true,SilentInt))
		{
			ppstack.push(PushDir);
			api::SetEnvironmentVariable(L"FARDIRSTACK", PushDir);
		}
		else
		{
			;
		}

		return TRUE;
	}
	// POPD
	// TODO: добавить необязательный параметр - число, сколько уровней пропустить, после чего прыгнуть.
	else if (IsCommand(L"POPD",false))
	{
		if (CheckCmdLineForHelp(strCmdLine.data()+4))
			return FALSE; // отдадимся COMSPEC`у

		if (!ppstack.empty())
		{
			int Ret=IntChDir(ppstack.top(),true,SilentInt);
			ppstack.pop();
			if (!ppstack.empty())
			{
				api::SetEnvironmentVariable(L"FARDIRSTACK", ppstack.top());
			}
			else
			{
				api::DeleteEnvironmentVariable(L"FARDIRSTACK");
			}
			return Ret;
		}

		return TRUE;
	}
	// CLRD
	else if (IsCommand(L"CLRD",false))
	{
		clear_and_shrink(ppstack);
		api::DeleteEnvironmentVariable(L"FARDIRSTACK");
		return TRUE;
	}
	/*
		Displays or sets the active code page number.
		CHCP [nnn]
			nnn   Specifies a code page number (Dec or Hex).
		Type CHCP without a parameter to display the active code page number.
	*/
	else if (IsCommand(L"CHCP",false))
	{
		strCmdLine.erase(0, 4);

		const wchar_t *Ptr=RemoveExternalSpaces(strCmdLine).data();

		if (CheckCmdLineForHelp(Ptr))
			return FALSE; // отдадимся COMSPEC`у

		if (!iswdigit(*Ptr))
			return FALSE;

		wchar_t Chr;

		while ((Chr=*Ptr) )
		{
			if (!iswdigit(Chr))
				break;

			++Ptr;
		}

		UINT cp = std::stoul(strCmdLine, nullptr, 10); //BUGBUG
		BOOL r1=Global->Console->SetInputCodepage(cp);
		BOOL r2=Global->Console->SetOutputCodepage(cp);

		if (r1 && r2) // Если все ОБИ, то так  и...
		{
			InitRecodeOutTable();
			InitKeysArray();
			Global->ScrBuf->ResetShadow();
			Global->ScrBuf->Flush();
			return TRUE;
		}
		else  // про траблы внешняя chcp сама скажет ;-)
		{
			return FALSE;
		}
	}
	else if (IsCommand(L"IF",false))
	{
		if (CheckCmdLineForHelp(strCmdLine.data()+2))
			return FALSE; // отдадимся COMSPEC`у

		const wchar_t *PtrCmd=PrepareOSIfExist(strCmdLine);
		// здесь PtrCmd - уже готовая команда, без IF

		if (PtrCmd && *PtrCmd && Global->CtrlObject->Plugins->ProcessCommandLine(PtrCmd))
		{
			//CmdStr.SetString(L"");
			GotoXY(X1,Y1);
			Global->FS << fmt::MinWidth(X2-X1+1)<<L"";
			Show();
			return TRUE;
		}

		return FALSE;
	}
	// пропускаем обработку, если нажат Shift-Enter
	else if (!SeparateWindow && (IsCommand(L"CD",true) || IsCommand(L"CHDIR",true)))
	{
		int Length = IsCommand(L"CD",true)? 2 : 5;

		strCmdLine.erase(0, Length);
		RemoveLeadingSpaces(strCmdLine);

		//проигнорируем /D
		//мы и так всегда меняем диск а некоторые в алайсах или по привычке набирают этот ключ
		if (!StrCmpNI(strCmdLine.data(),L"/D",2) && IsSpaceOrEos(strCmdLine[2]))
		{
			strCmdLine.erase(0, 2);
			RemoveLeadingSpaces(strCmdLine);
		}

		if (strCmdLine.empty() || CheckCmdLineForHelp(strCmdLine.data()))
			return FALSE; // отдадимся COMSPEC`у

		IntChDir(strCmdLine,Length==5,SilentInt);
		return TRUE;
	}
	else if (IsCommand(L"TITLE", false))
	{
		if (CheckCmdLineForHelp(strCmdLine.data() + 5))
			return FALSE; // отдадимся COMSPEC`у

		SetUserTitle(strCmdLine.data() + 5);
		return TRUE;
	}
	else if (IsCommand(L"EXIT",false))
	{
		if (CheckCmdLineForHelp(strCmdLine.data()+4))
			return FALSE; // отдадимся COMSPEC`у

		Global->FrameManager->ExitMainLoop(FALSE);
		return TRUE;
	}

	return FALSE;
}

bool CommandLine::CheckCmdLineForHelp(const wchar_t *CmdLine)
{
	if (CmdLine && *CmdLine)
	{
		while (IsSpace(*CmdLine))
			CmdLine++;

		if (*CmdLine && (CmdLine[0] == L'/' || CmdLine[0] == L'-') && CmdLine[1] == L'?')
			return true;
	}

	return false;
}

bool CommandLine::CheckCmdLineForSet(const string& CmdLine)
{
	if (CmdLine.size()>1 && CmdLine[0]==L'/' && IsSpaceOrEos(CmdLine[2]))
		return true;

	return false;
}

bool CommandLine::IntChDir(const string& CmdLine,int ClosePanel,bool Selent)
{
	Panel *SetPanel;
	SetPanel=Global->CtrlObject->Cp()->ActivePanel;

	if (SetPanel->GetType()!=FILE_PANEL && Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
		SetPanel=Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel);

	string strExpandedDir = Unquote(api::ExpandEnvironmentStrings(CmdLine));

	if (SetPanel->GetMode()!=PLUGIN_PANEL && strExpandedDir[0] == L'~' && ((strExpandedDir.size() == 1 && api::GetFileAttributes(strExpandedDir) == INVALID_FILE_ATTRIBUTES) || IsSlash(strExpandedDir[1])))
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
			strExpandedDir = api::ExpandEnvironmentStrings(strTemp);
		}
	}

	size_t DirOffset = 0;
	ParsePath(strExpandedDir, &DirOffset);
	if (wcspbrk(strExpandedDir.data() + DirOffset, L"?*")) // это маска?
	{
		api::FAR_FIND_DATA wfd;

		if (api::GetFindDataEx(strExpandedDir, wfd))
		{
			size_t pos;

			if (FindLastSlash(pos,strExpandedDir))
				strExpandedDir.resize(pos+1);
			else
				strExpandedDir.clear();

			strExpandedDir += wfd.strFileName;
		}
	}

	/* $ 15.11.2001 OT
		Сначала проверяем есть ли такая "обычная" директория.
		если уж нет, то тогда начинаем думать, что это директория плагинная
	*/
	DWORD DirAtt=api::GetFileAttributes(strExpandedDir);

	if (DirAtt!=INVALID_FILE_ATTRIBUTES && (DirAtt & FILE_ATTRIBUTE_DIRECTORY) && IsAbsolutePath(strExpandedDir))
	{
		ReplaceSlashToBSlash(strExpandedDir);
		SetPanel->SetCurDir(strExpandedDir,true);
		return true;
	}

	/* $ 20.09.2002 SKV
	  Это отключает возможность выполнять такие команды как:
	  cd net:server и cd ftp://server/dir
	  Так как под ту же гребёнку попадают и
	  cd s&r:, cd make: и т.д., которые к смене
	  каталога не имеют никакого отношения.
	*/
	/*
	if (Global->CtrlObject->Plugins->ProcessCommandLine(ExpandedDir))
	{
	  //CmdStr.SetString(L"");
	  GotoXY(X1,Y1);
	  Global->FS << fmt::Width(X2-X1+1)<<L"";
	  Show();
	  return true;
	}
	*/

	if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
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
	Flags.Change(FCMDOBJ_LOCKUPDATEPANEL,Mode);
}
