/*
cmdline.cpp

 омандна€ строка
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include "lang.hpp"
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
#include "vmenu.hpp"

CommandLine::CommandLine():
	CmdStr(CtrlObject->Cp(),0,true,CtrlObject->CmdHistory,0,(Opt.CmdLine.AutoComplete?EditControl::EC_ENABLEAUTOCOMPLETE:0)|EditControl::EC_ENABLEFNCOMPLETE),
	BackgroundScreen(nullptr),
	LastCmdPartLength(-1)
{
	CmdStr.SetEditBeyondEnd(FALSE);
	SetPersistentBlocks(Opt.CmdLine.EditBlock);
	SetDelRemovesBlocks(Opt.CmdLine.DelRemovesBlocks);
}

CommandLine::~CommandLine()
{
	if (BackgroundScreen)
		delete BackgroundScreen;
}

void CommandLine::SetPersistentBlocks(int Mode)
{
	CmdStr.SetPersistentBlocks(Mode);
}

void CommandLine::SetDelRemovesBlocks(int Mode)
{
	CmdStr.SetDelRemovesBlocks(Mode);
}

void CommandLine::SetAutoComplete(int Mode)
{
	if(Mode)
	{
		CmdStr.EnableAC(true);
	}
	else
	{
		CmdStr.DisableAC(true);
	}
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

BOOL CommandLine::SetLastCmdStr(const wchar_t *Ptr)
{
	strLastCmdStr = Ptr;
	return TRUE;
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

	if ((Key==KEY_CTRLEND || Key==KEY_CTRLNUMPAD1) && CmdStr.GetCurPos()==CmdStr.GetLength())
	{
		if (LastCmdPartLength==-1)
			SetLastCmdStr(CmdStr.GetStringAddr());

		strStr = strLastCmdStr;
		int CurCmdPartLength=(int)strStr.GetLength();
		CtrlObject->CmdHistory->GetSimilar(strStr,LastCmdPartLength);

		if (LastCmdPartLength==-1)
		{
			if (SetLastCmdStr(CmdStr.GetStringAddr()))
				LastCmdPartLength=CurCmdPartLength;
		}
		CmdStr.DisableAC();
		CmdStr.SetString(strStr);
		CmdStr.RevertAC();
		Show();
		return(TRUE);
	}

	if (Key == KEY_UP || Key == KEY_NUMPAD8)
	{
		if (CtrlObject->Cp()->LeftPanel->IsVisible() || CtrlObject->Cp()->RightPanel->IsVisible())
			return(FALSE);

		Key=KEY_CTRLE;
	}
	else if (Key == KEY_DOWN || Key == KEY_NUMPAD2)
	{
		if (CtrlObject->Cp()->LeftPanel->IsVisible() || CtrlObject->Cp()->RightPanel->IsVisible())
			return(FALSE);

		Key=KEY_CTRLX;
	}

	// $ 25.03.2002 VVM + ѕри погашенных панел€х колесом крутим историю
	if (!CtrlObject->Cp()->LeftPanel->IsVisible() && !CtrlObject->Cp()->RightPanel->IsVisible())
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
		case KEY_CTRLX:

			if (Key == KEY_CTRLE)
				CtrlObject->CmdHistory->GetPrev(strStr);
			else
				CtrlObject->CmdHistory->GetNext(strStr);

		case KEY_ESC:

			if (Key == KEY_ESC)
			{
				// $ 24.09.2000 SVS - ≈сли задано поведение по "Ќесохранению при Esc", то позицию в хистори не мен€ем и ставим в первое положение.
				if (Opt.CmdHistoryRule)
					CtrlObject->CmdHistory->ResetPosition();

				PStr=L"";
			}
			else
				PStr=strStr;

			SetString(PStr);
			return(TRUE);
		case KEY_F2:
		{
			UserMenu Menu(false);
			return TRUE;
		}
		case KEY_ALTF8:
		{
			int Type;
			// $ 19.09.2000 SVS - ѕри выборе из History (по Alt-F8) плагин не получал управление!
			int SelectType=CtrlObject->CmdHistory->Select(MSG(MHistoryTitle),L"History",strStr,Type);

			if (SelectType > 0 && SelectType <= 3)
			{
				if(SelectType<3)
				{
					CmdStr.DisableAC();
				}
				SetString(strStr);

				if (SelectType < 3)
				{
					ProcessKey(SelectType==1?(int)KEY_ENTER:(int)KEY_SHIFTENTER);
					CmdStr.RevertAC();
				}
			}
		}
		return(TRUE);
		case KEY_SHIFTF9:
			SaveConfig(1);
			return(TRUE);
		case KEY_F10:
			FrameManager->ExitMainLoop(TRUE);
			return(TRUE);
		case KEY_ALTF10:
		{
			Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
			{
				// TODO: здесь можно добавить проверку, что мы в корне диска и отсутствие файла Tree.Far...
				FolderTree Tree(strStr,MODALTREE_ACTIVE,TRUE,FALSE);
			}
			CtrlObject->Cp()->RedrawKeyBar();

			if (!strStr.IsEmpty())
			{
				ActivePanel->SetCurDir(strStr,TRUE);
				ActivePanel->Show();

				if (ActivePanel->GetType()==TREE_PANEL)
					ActivePanel->ProcessKey(KEY_ENTER);
			}
			else
			{
				// TODO: ... а здесь проверить факт изменени€/по€влени€ файла Tree.Far и мы оп€ть же в корне (чтобы лишний раз не апдейтить панель)
				ActivePanel->Update(UPDATE_KEEP_SELECTION);
				ActivePanel->Redraw();
				Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

				if (AnotherPanel->NeedUpdatePanel(ActivePanel))
				{
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);//|UPDATE_SECONDARY);
					AnotherPanel->Redraw();
				}
			}
		}
		return(TRUE);
		case KEY_F11:
			CtrlObject->Plugins.CommandsMenu(FALSE,FALSE,0);
			return(TRUE);
		case KEY_ALTF11:
			ShowViewEditHistory();
			CtrlObject->Cp()->Redraw();
			return(TRUE);
		case KEY_ALTF12:
		{
			int Type;
			int SelectType=CtrlObject->FolderHistory->Select(MSG(MFolderHistoryTitle),L"HistoryFolders",strStr,Type);

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
					CtrlObject->FolderHistory->SetAddMode(false,2,true);

				// пусть плагин сам прыгает... ;-)
				Panel *Panel=CtrlObject->Cp()->ActivePanel;

				if (SelectType == 6)
					Panel=CtrlObject->Cp()->GetAnotherPanel(Panel);

				//Type==1 - плагиновый путь
				//Type==0 - обычный путь
				//если путь плагиновый то сначала попробуем запустить его (а вдруг там префикс)
				//ну а если путь не плагиновый то запускать его точно не надо
				if (Type==0 || !CtrlObject->Plugins.ProcessCommandLine(strStr,Panel))
				{
					if (Panel->GetMode() == PLUGIN_PANEL || CheckShortcutFolder(&strStr,FALSE))
					{
						Panel->SetCurDir(strStr,Type==0 ? TRUE:FALSE);
						Panel->Redraw();
						CtrlObject->FolderHistory->SetAddMode(true,2,true);
					}
				}
			}
			else if (SelectType==3)
				SetString(strStr);
		}
		return(TRUE);
		case KEY_NUMENTER:
		case KEY_SHIFTNUMENTER:
		case KEY_ENTER:
		case KEY_SHIFTENTER:
		{
			Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
			CmdStr.Select(-1,0);
			CmdStr.Show();
			CmdStr.GetString(strStr);

			if (strStr.IsEmpty())
				break;

			ActivePanel->SetCurPath();

			if (!(Opt.ExcludeCmdHistory&EXCLUDECMDHISTORY_NOTCMDLINE))
				CtrlObject->CmdHistory->AddToHistory(strStr);

			ProcessOSAliases(strStr);

			if (!ActivePanel->ProcessPluginEvent(FE_COMMAND,(void *)(const wchar_t *)strStr))
				CmdExecute(strStr,FALSE,Key==KEY_SHIFTENTER||Key==KEY_SHIFTNUMENTER,FALSE);
		}
		return(TRUE);
		case KEY_CTRLU:
			CmdStr.Select(-1,0);
			CmdStr.Show();
			return(TRUE);
		case KEY_OP_XLAT:
		{
			// 13.12.2000 SVS - ! ƒл€ CmdLine - если нет выделени€, преобразуем всю строку (XLat)
			CmdStr.Xlat(Opt.XLat.Flags&XLAT_CONVERTALLCMDLINE?TRUE:FALSE);

			if (SetLastCmdStr(CmdStr.GetStringAddr()))
			{
				LastCmdPartLength=(int)strLastCmdStr.GetLength();
			}

			return TRUE;
		}
		/* дополнительные клавиши дл€ выделени€ в ком строке.
		   ¬Ќ»ћјЌ»≈!
		   ƒл€ сокращени€ кода этот кусок должен сто€ть перед "default"
		*/
		case KEY_ALTSHIFTLEFT:  case KEY_ALTSHIFTNUMPAD4:
		case KEY_ALTSHIFTRIGHT: case KEY_ALTSHIFTNUMPAD6:
		case KEY_ALTSHIFTEND:   case KEY_ALTSHIFTNUMPAD1:
		case KEY_ALTSHIFTHOME:  case KEY_ALTSHIFTNUMPAD7:
			Key&=~KEY_ALT;
		default:

			//   —брасываем выделение на некоторых клавишах
			if (!Opt.CmdLine.EditBlock)
			{
				static int UnmarkKeys[]=
				{
					KEY_LEFT,       KEY_NUMPAD4,
					KEY_CTRLS,
					KEY_RIGHT,      KEY_NUMPAD6,
					KEY_CTRLD,
					KEY_CTRLLEFT,   KEY_CTRLNUMPAD4,
					KEY_CTRLRIGHT,  KEY_CTRLNUMPAD6,
					KEY_CTRLHOME,   KEY_CTRLNUMPAD7,
					KEY_CTRLEND,    KEY_CTRLNUMPAD1,
					KEY_HOME,       KEY_NUMPAD7,
					KEY_END,        KEY_NUMPAD1
				};

				for (size_t I=0; I< countof(UnmarkKeys); I++)
					if (Key==UnmarkKeys[I])
					{
						CmdStr.Select(-1,0);
						break;
					}
			}

			if (Key == KEY_CTRLD)
				Key=KEY_RIGHT;

			if (!CmdStr.ProcessKey(Key))
				break;

			LastCmdPartLength=-1;

			if(!Opt.CmdLine.AutoComplete && (Key == KEY_CTRLSHIFTEND || Key == KEY_CTRLSHIFTNUMPAD1))
			{
				CmdStr.EnableAC();
				CmdStr.AutoComplete(true,false);
				CmdStr.RevertAC();
			}

			return(TRUE);
	}

	return(FALSE);
}


BOOL CommandLine::SetCurDir(const wchar_t *CurDir)
{
	if (StrCmpI(strCurDir,CurDir) || !TestCurrentDirectory(CurDir))
	{
		strCurDir = CurDir;

		if (CtrlObject->Cp()->ActivePanel->GetMode()!=PLUGIN_PANEL)
			PrepareDiskPath(strCurDir);
	}

	return TRUE;
}


int CommandLine::GetCurDir(string &strCurDir)
{
	strCurDir = CommandLine::strCurDir;
	return (int)strCurDir.GetLength();
}


void CommandLine::SetString(const wchar_t *Str,BOOL Redraw)
{
	LastCmdPartLength=-1;
	CmdStr.SetString(Str);
	CmdStr.SetLeftPos(0);

	if (Redraw)
		CmdStr.Show();
}


void CommandLine::ExecString(const wchar_t *Str,int AlwaysWaitFinish,int SeparateWindow,
                             int DirectRun)
{
	CmdStr.DisableAC();
	SetString(Str);
	CmdStr.RevertAC();
	CmdExecute(Str,AlwaysWaitFinish,SeparateWindow,DirectRun);
}


void CommandLine::InsertString(const wchar_t *Str)
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
	return(CmdStr.ProcessMouse(MouseEvent));
}

void CommandLine::GetPrompt(string &strDestStr)
{
	if (Opt.CmdLine.UsePromptFormat)
	{
		string strFormatStr, strExpandedFormatStr;
		strFormatStr = Opt.CmdLine.strPromptFormat;
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

				for (I=0; I < countof(ChrFmt); ++I)
				{
					if (ChrFmt[I][0] == Chr)
					{
						strDestStr += ChrFmt[I][1];
						break;
					}
				}

				if (I == countof(ChrFmt))
				{
					switch (Chr)
					{
							/* эти не раелизованы
							$E - Escape code (ASCII code 27)
							$V - Windows XP version number
							$_ - Carriage return and linefeed
							$M - ќтображение полного имени удаленного диска, св€занного с именем текущего диска, или пустой строки, если текущий диск не €вл€етс€ сетевым.
							*/
						case L'+': // $+  - ќтображение нужного числа знаков плюс (+) в зависимости от текущей глубины стека каталогов PUSHD, по одному знаку на каждый сохраненный путь.
						{
							DWORD ppstacksize=ppstack.size();

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
							if ( Opt.IsUserAdmin )
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
							if (IsLocalPath(strCurDir) && IsSlash(strCurDir.At(2)))
								strDestStr += Upper(strCurDir.At(0));
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
	int SelectType=CtrlObject->ViewHistory->Select(MSG(MViewHistoryTitle),L"HistoryViews",strStr,Type);
	/*
	   SelectType = 0 - Esc
	                1 - Enter
	                2 - Shift-Enter
	                3 - Ctrl-Enter
	*/

	if (SelectType == 1 || SelectType == 2)
	{
		if (SelectType!=2)
			CtrlObject->ViewHistory->AddToHistory(strStr,Type);

		CtrlObject->ViewHistory->SetAddMode(false,Opt.FlagPosixSemantics?1:2,true);

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
				// пусть файл создаетс€
				FileEditor *FEdit=new FileEditor(strStr,CP_AUTODETECT,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6);

				if (Type == 4)
					FEdit->SetLockEditor(TRUE);

				break;
			}
			// 2 и 3 - заполн€етс€ в ProcessExternal
			case 2:
			case 3:
			{
				if (strStr.At(0) !=L'@')
				{
					ExecString(strStr,Type-2);
				}
				else
				{
					SaveScreen SaveScr;
					CtrlObject->Cp()->LeftPanel->CloseFile();
					CtrlObject->Cp()->RightPanel->CloseFile();
					Execute(strStr.CPtr()+1,Type-2);
				}

				break;
			}
		}

		CtrlObject->ViewHistory->SetAddMode(true,Opt.FlagPosixSemantics?1:2,true);
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
	BackgroundScreen->Resize(ScrX+1,ScrY+1,2);
//  this->DisplayObject();
}
