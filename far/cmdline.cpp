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

enum
{
	FCMDOBJ_LOCKUPDATEPANEL   = 0x00010000,
};

CommandLine::CommandLine():
	CmdStr(Global->CtrlObject->Cp(),0,true,Global->CtrlObject->CmdHistory,0,(Global->Opt->CmdLine.AutoComplete?EditControl::EC_ENABLEAUTOCOMPLETE:0)|EditControl::EC_COMPLETE_HISTORY|EditControl::EC_COMPLETE_FILESYSTEM|EditControl::EC_COMPLETE_PATH),
	BackgroundScreen(nullptr),
	LastCmdPartLength(-1)
{
	CmdStr.SetEditBeyondEnd(FALSE);
	CmdStr.SetMacroAreaAC(MACRO_SHELLAUTOCOMPLETION);
	SetPersistentBlocks(Global->Opt->CmdLine.EditBlock);
	SetDelRemovesBlocks(Global->Opt->CmdLine.DelRemovesBlocks);
}

CommandLine::~CommandLine()
{
	if (BackgroundScreen)
		delete BackgroundScreen;
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
	string strTruncDir;
	GetPrompt(strTruncDir);
	TruncPathStr(strTruncDir,(X2-X1)/2);
	GotoXY(X1,Y1);
	SetColor(COL_COMMANDLINEPREFIX);
	Text(strTruncDir);
	CmdStr.SetObjectColor(COL_COMMANDLINE,COL_COMMANDLINESELECTED);

	CmdStr.SetPosition(X1+(int)strTruncDir.GetLength(),Y1,X2,Y2);

	CmdStr.Show();

	GotoXY(X2+1,Y1);
	SetColor(COL_COMMANDLINEPREFIX);
	Text(L"\x2191");
}


void CommandLine::SetCurPos(int Pos, int LeftPos)
{
	CmdStr.SetLeftPos(LeftPos);
	CmdStr.SetCurPos(Pos);
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
		int CurCmdPartLength=(int)strStr.GetLength();
		Global->CtrlObject->CmdHistory->GetSimilar(strStr,LastCmdPartLength);

		if (LastCmdPartLength==-1)
		{
			strLastCmdStr = CmdStr.GetStringAddr();
			LastCmdPartLength=CurCmdPartLength;
		}

		{
			SetAutocomplete disable(&CmdStr);
			CmdStr.SetString(strStr);
			CmdStr.Select(LastCmdPartLength,static_cast<int>(strStr.GetLength()));
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
					SetAutocomplete disable(&CmdStr);
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
				PStr=strStr;

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
			int Type;
			// $ 19.09.2000 SVS - При выборе из History (по Alt-F8) плагин не получал управление!
			int SelectType=Global->CtrlObject->CmdHistory->Select(MSG(MHistoryTitle),L"History",strStr,Type);
			// BUGBUG, magic numbers
			if ((SelectType > 0 && SelectType <= 3) || SelectType == 7)
			{
				SetAutocomplete* disable = nullptr;
				if(SelectType<3 || SelectType == 7)
				{
					disable = new SetAutocomplete(&CmdStr);
				}
				SetString(strStr);

				if (SelectType < 3 || SelectType == 7)
				{
					ProcessKey(SelectType==7?static_cast<int>(KEY_CTRLALTENTER):(SelectType==1?static_cast<int>(KEY_ENTER):static_cast<int>(KEY_SHIFTENTER)));
					delete disable;
				}
			}
		}
		return TRUE;
		case KEY_SHIFTF9:
			Global->Opt->Save(true);
			return TRUE;
		case KEY_F10:
			FrameManager->ExitMainLoop(TRUE);
			return TRUE;
		case KEY_ALTF10:
		case KEY_RALTF10:
		{
			Panel *ActivePanel=Global->CtrlObject->Cp()->ActivePanel;
			{
				// TODO: здесь можно добавить проверку, что мы в корне диска и отсутствие файла Tree.Far...
				FolderTree Tree(strStr,MODALTREE_ACTIVE,TRUE,FALSE);
			}
			Global->CtrlObject->Cp()->RedrawKeyBar();

			if (!strStr.IsEmpty())
			{
				ActivePanel->SetCurDir(strStr,TRUE);
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
			int Type;
			GUID Guid; string strFile, strData;
			int SelectType=Global->CtrlObject->FolderHistory->Select(MSG(MFolderHistoryTitle),L"HistoryFolders",strStr,Type,&Guid,&strFile,&strData);

			/*
			   SelectType = 0 - Esc
			                1 - Enter
			                2 - Shift-Enter
			                3 - Ctrl-Enter
			                6 - Ctrl-Shift-Enter - на пассивную панель со сменой позиции
			*/
			if (SelectType == 1 || SelectType == 2 || SelectType == 6)
			{
				if (SelectType==2)
					Global->CtrlObject->FolderHistory->SetAddMode(false,2,true);

				// пусть плагин сам прыгает... ;-)
				Panel *Panel=Global->CtrlObject->Cp()->ActivePanel;

				if (SelectType == 6)
					Panel=Global->CtrlObject->Cp()->GetAnotherPanel(Panel);

				//Type==1 - плагиновый путь
				//Type==0 - обычный путь
				Panel->ExecShortcutFolder(strStr,Guid,strFile,strData,true);
				// Panel may be changed
				if(SelectType == 6)
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
			else if (SelectType==3)
				SetString(strStr);
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

			if (strStr.IsEmpty())
				break;

			ActivePanel->SetCurPath();

			if (!(Global->Opt->ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTCMDLINE))
				Global->CtrlObject->CmdHistory->AddToHistory(strStr);

			ProcessOSAliases(strStr);

			if (!ActivePanel->ProcessPluginEvent(FE_COMMAND,(void *)strStr.CPtr()))
				ExecString(strStr, false, Key==KEY_SHIFTENTER||Key==KEY_SHIFTNUMENTER, false, false, false,
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
			CmdStr.Xlat(Global->Opt->XLat.Flags&XLAT_CONVERTALLCMDLINE?TRUE:FALSE);

			// иначе неправильно работает ctrl-end
			strLastCmdStr = CmdStr.GetStringAddr();
			LastCmdPartLength=(int)strLastCmdStr.GetLength();

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

				for (size_t I=0; I< ARRAYSIZE(UnmarkKeys); I++)
					if (Key==UnmarkKeys[I])
					{
						CmdStr.Select(-1,0);
						break;
					}
			}

			if (Key == KEY_CTRLD || Key == KEY_RCTRLD)
				Key=KEY_RIGHT;

			if(Key == KEY_CTRLSPACE || Key == KEY_RCTRLSPACE)
			{
				SetAutocomplete enable(&CmdStr, true);
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
	if (StrCmpI(strCurDir,CurDir) || !TestCurrentDirectory(CurDir))
	{
		strCurDir = CurDir;

		if (Global->CtrlObject->Cp()->ActivePanel->GetMode()!=PLUGIN_PANEL)
			PrepareDiskPath(strCurDir);
	}
}


int CommandLine::GetCurDir(string &strCurDir)
{
	strCurDir = CommandLine::strCurDir;
	return (int)strCurDir.GetLength();
}


void CommandLine::SetString(const string& Str, bool Redraw)
{
	LastCmdPartLength=-1;
	CmdStr.SetString(Str);
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


int CommandLine::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	if(MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED && MouseEvent->dwMousePosition.X==X2+1)
	{
		return ProcessKey(KEY_ALTF8);
	}
	return CmdStr.ProcessMouse(MouseEvent);
}

void CommandLine::GetPrompt(string &strDestStr)
{
	if (Global->Opt->CmdLine.UsePromptFormat)
	{
		string strFormatStr, strExpandedFormatStr;
		strFormatStr = Global->Opt->CmdLine.strPromptFormat.Get();
		apiExpandEnvironmentStrings(strFormatStr, strExpandedFormatStr);
		const wchar_t *Format=strExpandedFormatStr;
		wchar_t ChrFmt[][2]=
		{
			{L'A',L'&'},   // $A - & (Ampersand)
			{L'B',L'|'},   // $B - | (pipe)
			{L'C',L'('},   // $C - ( (Left parenthesis)
			{L'F',L')'},   // $F - ) (Right parenthesis)
			{L'G',L'>'},   // $G - > (greater-than sign)
			{L'L',L'<'},   // $L - < (less-than sign)
			{L'Q',L'='},   // $Q - = (equal sign)
			{L'S',L' '},   // $S - (space)
			{L'$',L'$'},   // $$ - $ (dollar sign)
		};

		while (*Format)
		{
			if (*Format==L'$')
			{
				wchar_t Chr=Upper(*++Format);
				size_t I;

				for (I=0; I < ARRAYSIZE(ChrFmt); ++I)
				{
					if (ChrFmt[I][0] == Chr)
					{
						strDestStr += ChrFmt[I][1];
						break;
					}
				}

				if (I == ARRAYSIZE(ChrFmt))
				{
					switch (Chr)
					{
							/* эти не раелизованы
							$E - Escape code (ASCII code 27)
							$V - Windows XP version number
							$_ - Carriage return and linefeed
							$M - Отображение полного имени удаленного диска, связанного с именем текущего диска, или пустой строки, если текущий диск не является сетевым.
							*/
						case L'+': // $+  - Отображение нужного числа знаков плюс (+) в зависимости от текущей глубины стека каталогов PUSHD, по одному знаку на каждый сохраненный путь.
						{
							size_t ppstacksize=ppstack.size();

							if (ppstacksize)
							{
								wchar_t * p = strDestStr.GetBuffer(strDestStr.GetLength()+ppstacksize+1);
								wmemset(p + strDestStr.GetLength(),L'+',ppstacksize);
								strDestStr.ReleaseBuffer(strDestStr.GetLength()+ppstacksize);
							}

							break;
						}
						case L'H': // $H - Backspace (erases previous character)
						{
							if (!strDestStr.IsEmpty())
								strDestStr.SetLength(strDestStr.GetLength()-1);

							break;
						}
						case L'@': // $@xx - Admin
						{
							wchar_t lb=*++Format;
							wchar_t rb=*++Format;
							if ( Global->IsUserAdmin() )
							{
								strDestStr += lb;
								strDestStr += MSG(MConfigCmdlinePromptFormatAdmin);
								strDestStr += rb;
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
								strDestStr += Upper(strCurDir.At(0));
							else if(Type == PATH_DRIVELETTERUNC)
								strDestStr += Upper(strCurDir.At(4));
							else
								strDestStr += L'?';
							break;
						}
						case L'P': // $P - Current drive and path
						{
							strDestStr += strCurDir;
							break;
						}
					}
				}

				Format++;
			}
			else
			{
				strDestStr += *(Format++);
			}
		}
	}
	else // default prompt = "$p$g"
	{
		strDestStr = strCurDir;
		strDestStr += L">";
	}
}


void CommandLine::ShowViewEditHistory()
{
	string strStr;
	int Type;
	int SelectType=Global->CtrlObject->ViewHistory->Select(MSG(MViewHistoryTitle),L"HistoryViews",strStr,Type);
	/*
	   SelectType = 0 - Esc
	                1 - Enter
	                2 - Shift-Enter
	                3 - Ctrl-Enter
	*/

	if (SelectType == 1 || SelectType == 2)
	{
		if (SelectType!=2)
			Global->CtrlObject->ViewHistory->AddToHistory(strStr,Type);

		Global->CtrlObject->ViewHistory->SetAddMode(false,Global->Opt->FlagPosixSemantics?1:2,true);

		switch (Type)
		{
			case 0: // вьювер
			{
				new FileViewer(strStr,TRUE);
				break;
			}
			case 1: // обычное открытие в редакторе
			case 4: // открытие с локом
			{
				// пусть файл создается
				FileEditor *FEdit=new FileEditor(strStr,CP_DEFAULT,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6);

				if (Type == 4)
					FEdit->SetLockEditor(TRUE);

				break;
			}
			// 2 и 3 - заполняется в ProcessExternal
			case 2:
			case 3:
			{
				if (strStr.At(0) !=L'@')
				{
					ExecString(strStr,Type>2);
				}
				else
				{
					SaveScreen SaveScr;
					Global->CtrlObject->Cp()->LeftPanel->CloseFile();
					Global->CtrlObject->Cp()->RightPanel->CloseFile();
					Execute(strStr.CPtr()+1,Type>2);
				}

				break;
			}
		}

		Global->CtrlObject->ViewHistory->SetAddMode(true,Global->Opt->FlagPosixSemantics?1:2,true);
	}
	else if (SelectType==3) // скинуть из истории в ком.строку?
		SetString(strStr);
}

void CommandLine::SaveBackground(int X1,int Y1,int X2,int Y2)
{
	if (BackgroundScreen)
	{
		delete BackgroundScreen;
	}

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

int CommandLine::ExecString(const string& CmdLine, bool AlwaysWaitFinish, bool SeparateWindow, bool DirectRun, bool WaitForIdle, bool Silent, bool RunAs)
{
	{
		SetAutocomplete disable(&CmdStr);
		SetString(CmdLine);
	}

	LastCmdPartLength=-1;

	if(!StrCmpI(CmdLine,L"far:config"))
	{
		SetString(L"", false);
		Show();
		return AdvancedConfig();
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

	if (!strCurDir.IsEmpty() && strCurDir.At(1)==L':')
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
	Global->ScrBuf->Flush();
	return Code;
}

int CommandLine::ProcessOSCommands(const string& CmdLine, bool SeparateWindow, bool &PrintCommand)
{
	int Length;
	string strCmdLine = CmdLine;
	Panel *SetPanel=Global->CtrlObject->Cp()->ActivePanel;
	PrintCommand=true;

	if (SetPanel->GetType()!=FILE_PANEL && Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
		SetPanel=Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel);

	RemoveTrailingSpaces(strCmdLine);
	bool SilentInt=false;

	if (*CmdLine == L'@')
	{
		SilentInt=true;
		strCmdLine.LShift(1);
	}

	if (!SeparateWindow && strCmdLine.At(0) && strCmdLine.At(1)==L':' && !strCmdLine.At(2))
	{
		if(!FarChDir(strCmdLine))
		{
			wchar_t NewDir[]={Upper(strCmdLine.At(0)),L':',L'\\',0};
			{
				FarChDir(NewDir);
			}
		}
		SetPanel->ChangeDirToCurrent();
		return TRUE;
	}
	// SET [переменная=[строка]]
	else if (!StrCmpNI(strCmdLine,L"SET",3) && (IsSpaceOrEos(strCmdLine.At(3)) || strCmdLine.At(3) == L'/'))
	{
		size_t pos;
		strCmdLine.LShift(3);
		RemoveLeadingSpaces(strCmdLine);

		if (CheckCmdLineForHelp(strCmdLine))
			return FALSE; // отдадимся COMSPEC`у

		// "set" (display all) or "set var" (display all that begin with "var")
		if (strCmdLine.IsEmpty() || !strCmdLine.Pos(pos,L'=') || !pos)
		{
			//forward "set [prefix]| command" and "set [prefix]> file" to COMSPEC
			if (strCmdLine.ContainsAny(L"|>"))
				return FALSE;

			ShowBackground();  //??? почему не отдаём COMSPEC'у
			// display command //???
			Redraw();
			GotoXY(X2+1,Y1);
			Text(L" ");
			Global->ScrBuf->Flush();
			Global->Console->SetTextAttributes(ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));
			string strOut("\n");
			int CmdLength = static_cast<int>(strCmdLine.GetLength());
			LPWCH Environment = GetEnvironmentStrings();
			for (LPCWSTR Ptr = Environment; *Ptr;)
			{
				int PtrLength = StrLength(Ptr);
				if (!StrCmpNI(Ptr, strCmdLine, CmdLength))
				{
					strOut.Append(Ptr, PtrLength).Append(L"\n");
				}
				Ptr+=PtrLength+1;
			}
			FreeEnvironmentStrings(Environment);
			strOut.Append(L"\n\n", Global->Opt->ShowKeyBar?2:1);
			Global->Console->Write(strOut);
			Global->Console->Commit();
			Global->ScrBuf->FillBuf();
			SaveBackground();
			PrintCommand = false;
			return TRUE;
		}

		if (CheckCmdLineForSet(strCmdLine)) // вариант для /A и /P
			return FALSE; //todo: /p - dialog, /a - calculation; then set variable ...

		if (strCmdLine.GetLength() == pos+1) //set var=
		{
			strCmdLine.SetLength(pos);
			SetEnvironmentVariable(strCmdLine,nullptr);
		}
		else
		{
			string strExpandedStr;

			if (apiExpandEnvironmentStrings(strCmdLine.CPtr()+pos+1,strExpandedStr))
			{
				strCmdLine.SetLength(pos);
				SetEnvironmentVariable(strCmdLine,strExpandedStr);
			}
		}

		return TRUE;
	}
	// REM все остальное
	else if ((!StrCmpNI(strCmdLine,L"REM",Length=3) && IsSpaceOrEos(strCmdLine.At(3))) || !StrCmpNI(strCmdLine,L"::",Length=2))
	{
		if (Length == 3 && CheckCmdLineForHelp(strCmdLine.CPtr()+Length))
			return FALSE; // отдадимся COMSPEC`у

		return TRUE;
	}
	else if (!StrCmpNI(strCmdLine,L"CLS",3) && IsSpaceOrEos(strCmdLine.At(3)))
	{
		if (CheckCmdLineForHelp(strCmdLine.CPtr()+3))
			return FALSE; // отдадимся COMSPEC`у

		ClearScreen(ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));
		SaveBackground();
		PrintCommand=false;
		return TRUE;
	}
	// PUSHD путь | ..
	else if (!StrCmpNI(strCmdLine,L"PUSHD",5) && IsSpaceOrEos(strCmdLine.At(5)))
	{
		strCmdLine.LShift(5);
		RemoveLeadingSpaces(strCmdLine);

		if (CheckCmdLineForHelp(strCmdLine))
			return FALSE; // отдадимся COMSPEC`у

		PushPopRecord prec;
		prec.strName = strCurDir;

		if (IntChDir(strCmdLine,true,SilentInt))
		{
			ppstack.push(prec);
			SetEnvironmentVariable(L"FARDIRSTACK",prec.strName);
		}
		else
		{
			;
		}

		return TRUE;
	}
	// POPD
	// TODO: добавить необязательный параметр - число, сколько уровней пропустить, после чего прыгнуть.
	else if (!StrCmpNI(CmdLine,L"POPD",4) && IsSpaceOrEos(strCmdLine.At(4)))
	{
		if (CheckCmdLineForHelp(strCmdLine.CPtr()+4))
			return FALSE; // отдадимся COMSPEC`у

		if (ppstack.size())
		{
			PushPopRecord& prec = ppstack.top();
			ppstack.pop();
			int Ret=IntChDir(prec.strName,true,SilentInt)?TRUE:FALSE;
			const wchar_t* Ptr = nullptr;
			if (ppstack.size())
			{
				Ptr = ppstack.top().strName;
			}
			SetEnvironmentVariable(L"FARDIRSTACK", Ptr);
			return Ret;
		}

		return TRUE;
	}
	// CLRD
	else if (!StrCmpI(CmdLine,L"CLRD"))
	{
		DECLTYPE(ppstack)().swap(ppstack);
		SetEnvironmentVariable(L"FARDIRSTACK",nullptr);
		return TRUE;
	}
	/*
		Displays or sets the active code page number.
		CHCP [nnn]
			nnn   Specifies a code page number (Dec or Hex).
		Type CHCP without a parameter to display the active code page number.
	*/
	else if (!StrCmpNI(strCmdLine,L"CHCP",4) && IsSpaceOrEos(strCmdLine.At(4)))
	{
		strCmdLine.LShift(4);

		const wchar_t *Ptr=RemoveExternalSpaces(strCmdLine);

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

		wchar_t *Ptr2;
		UINT cp=(UINT)wcstol(strCmdLine,&Ptr2,10); //BUGBUG
		BOOL r1=Global->Console->SetInputCodepage(cp);
		BOOL r2=Global->Console->SetOutputCodepage(cp);

		if (r1 && r2) // Если все ОБИ, то так  и...
		{
			InitRecodeOutTable();
#ifndef NO_WRAPPER
			wrapper::LocalUpperInit();
#endif // NO_WRAPPER
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
	else if (!StrCmpNI(strCmdLine,L"IF",2) && IsSpaceOrEos(strCmdLine.At(2)))
	{
		if (CheckCmdLineForHelp(strCmdLine.CPtr()+2))
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
	else if (!SeparateWindow && (!StrCmpNI(strCmdLine,L"CD",Length=2) || !StrCmpNI(strCmdLine,L"CHDIR",Length=5)))
	{
		if (!IsSpaceOrEos(strCmdLine.At(Length)))
		{
			if (!IsSlash(strCmdLine.At(Length)))
				return FALSE;
		}

		strCmdLine.LShift(Length);
		RemoveLeadingSpaces(strCmdLine);

		//проигнорируем /D
		//мы и так всегда меняем диск а некоторые в алайсах или по привычке набирают этот ключ
		if (!StrCmpNI(strCmdLine,L"/D",2) && IsSpaceOrEos(strCmdLine.At(2)))
		{
			strCmdLine.LShift(2);
			RemoveLeadingSpaces(strCmdLine);
		}

		if (strCmdLine.IsEmpty() || CheckCmdLineForHelp(strCmdLine))
			return FALSE; // отдадимся COMSPEC`у

		IntChDir(strCmdLine,Length==5,SilentInt);
		return TRUE;
	}
	else if (!StrCmpNI(strCmdLine,L"EXIT",4) && IsSpaceOrEos(strCmdLine.At(4)))
	{
		if (CheckCmdLineForHelp(strCmdLine.CPtr()+4))
			return FALSE; // отдадимся COMSPEC`у

		FrameManager->ExitMainLoop(FALSE);
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
	if (CmdLine.GetLength()>1 && CmdLine.At(0)==L'/' && IsSpaceOrEos(CmdLine.At(2)))
		return true;

	return false;
}

bool CommandLine::IntChDir(const string& CmdLine,int ClosePanel,bool Selent)
{
	Panel *SetPanel;
	SetPanel=Global->CtrlObject->Cp()->ActivePanel;

	if (SetPanel->GetType()!=FILE_PANEL && Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
		SetPanel=Global->CtrlObject->Cp()->GetAnotherPanel(SetPanel);

	string strExpandedDir(CmdLine);
	Unquote(strExpandedDir);
	apiExpandEnvironmentStrings(strExpandedDir,strExpandedDir);

	if (SetPanel->GetMode()!=PLUGIN_PANEL && strExpandedDir.At(0) == L'~' && ((!strExpandedDir.At(1) && apiGetFileAttributes(strExpandedDir) == INVALID_FILE_ATTRIBUTES) || IsSlash(strExpandedDir.At(1))))
	{
		if (Global->Opt->Exec.UseHomeDir && !Global->Opt->Exec.strHomeDir.IsEmpty())
		{
			string strTemp=Global->Opt->Exec.strHomeDir.Get();

			if (strExpandedDir.At(1))
			{
				AddEndSlash(strTemp);
				strTemp += strExpandedDir.CPtr()+2;
			}

			DeleteEndSlash(strTemp);
			strExpandedDir=strTemp;
			apiExpandEnvironmentStrings(strExpandedDir,strExpandedDir);
		}
	}

	const wchar_t* DirPtr = strExpandedDir;
	ParsePath(strExpandedDir, &DirPtr);
	if (wcspbrk(DirPtr, L"?*")) // это маска?
	{
		FAR_FIND_DATA wfd;

		if (apiGetFindDataEx(strExpandedDir, wfd))
		{
			size_t pos;

			if (FindLastSlash(pos,strExpandedDir))
				strExpandedDir.SetLength(pos+1);
			else
				strExpandedDir.Clear();

			strExpandedDir += wfd.strFileName;
		}
	}

	/* $ 15.11.2001 OT
		Сначала проверяем есть ли такая "обычная" директория.
		если уж нет, то тогда начинаем думать, что это директория плагинная
	*/
	DWORD DirAtt=apiGetFileAttributes(strExpandedDir);

	if (DirAtt!=INVALID_FILE_ATTRIBUTES && (DirAtt & FILE_ATTRIBUTE_DIRECTORY) && IsAbsolutePath(strExpandedDir))
	{
		ReplaceSlashToBSlash(strExpandedDir);
		SetPanel->SetCurDir(strExpandedDir,TRUE);
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
	strExpandedDir.ReleaseBuffer();

	if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
	{
		SetPanel->SetCurDir(strExpandedDir,ClosePanel);
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
		if (!Selent)
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strExpandedDir,MSG(MOk));

		return false;
	}

	return true;
}

void CommandLine::LockUpdatePanel(bool Mode)
{
	Flags.Change(FCMDOBJ_LOCKUPDATEPANEL,Mode);
}
