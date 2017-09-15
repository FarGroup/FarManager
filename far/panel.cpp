/*
panel.cpp

Parent class для панелей
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

#include "panel.hpp"
#include "keyboard.hpp"
#include "flink.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "cmdline.hpp"
#include "chgprior.hpp"
#include "treelist.hpp"
#include "filelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "help.hpp"
#include "syslog.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "clipboard.hpp"
#include "shortcuts.hpp"
#include "dirmix.hpp"
#include "constitle.hpp"
#include "colormix.hpp"
#include "FarGuid.hpp"
#include "lang.hpp"
#include "plugins.hpp"
#include "keybar.hpp"
#include "strmix.hpp"
#include "diskmenu.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "pathmix.hpp"

static int DragX,DragY,DragMove;
static Panel *SrcDragPanel;
static SaveScreen *DragSaveScr=nullptr;

Panel::Panel(window_ptr Owner):
	ScreenObject(Owner),
	ProcessingPluginCommand(0),
	m_Type(panel_type::FILE_PANEL),
	m_EnableUpdate(TRUE),
	m_PanelMode(panel_mode::NORMAL_PANEL),
	m_SortMode(panel_sort::UNSORTED),
	m_ReverseSortOrder(false),
	m_SortGroups(false),
	m_PrevViewMode(VIEW_3),
	m_ViewMode(0),
	m_CurTopFile(0),
	m_CurFile(0),
	m_ShowShortNames(false),
	m_NumericSort(false),
	m_CaseSensitiveSort(false),
	m_DirectoriesFirst(true),
	m_ModalMode(0),
	m_PluginCommand(0)
{
	_OT(SysLog(L"[%p] Panel::Panel()", this));
	SrcDragPanel=nullptr;
	DragX=DragY=-1;
};


Panel::~Panel()
{
	_OT(SysLog(L"[%p] Panel::~Panel()", this));
	EndDrag();
}


void Panel::SetViewMode(int ViewMode)
{
	m_PrevViewMode=ViewMode;
	m_ViewMode=ViewMode;
};


void Panel::ChangeDirToCurrent()
{
	SetCurDir(os::GetCurrentDirectory(), true);
}

long long Panel::VMProcess(int OpCode, void* vParam, long long iParam)
{
	return 0;
}

// корректировка букв
static DWORD _CorrectFastFindKbdLayout(const INPUT_RECORD& rec,DWORD Key)
{
	if ((Key&(KEY_ALT|KEY_RALT)))// && Key!=(KEY_ALT|0x3C))
	{
		// // _SVS(SysLog(L"_CorrectFastFindKbdLayout>>> %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
		if (rec.Event.KeyEvent.uChar.UnicodeChar && (Key&KEY_MASKF) != rec.Event.KeyEvent.uChar.UnicodeChar) //???
			Key = (Key & 0xFFF10000) | rec.Event.KeyEvent.uChar.UnicodeChar;   //???

		// // _SVS(SysLog(L"_CorrectFastFindKbdLayout<<< %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
	}

	return Key;
}

class Search: public Modal
{
	struct private_tag {};

public:
	static search_ptr create(Panel* Owner, const Manager::Key& FirstKey);

	Search(private_tag, Panel* Owner, const Manager::Key& FirstKey);

	void Process(void);
	virtual bool ProcessKey(const Manager::Key& Key) override;
	virtual bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual int GetType() const override { return windowtype_search; }
	virtual int GetTypeAndName(string &, string &) override { return windowtype_search; }
	virtual void ResizeConsole(void) override;
	const Manager::Key& KeyToProcess(void) const { return m_KeyToProcess; }

private:
	void InitPositionAndSize(void);
	void init(void);

	Panel* m_Owner;
	Manager::Key m_FirstKey;
	std::unique_ptr<EditControl> m_FindEdit;
	Manager::Key m_KeyToProcess;
	virtual void DisplayObject(void) override;
	virtual string GetTitle() const override { return {}; }
	void ProcessName(const string& Src) const;
	void ShowBorder(void) const;
	void Close(void);
};

Search::Search(private_tag, Panel* Owner, const Manager::Key& FirstKey):
	m_Owner(Owner),
	m_FirstKey(FirstKey),
	m_FindEdit(),
	m_KeyToProcess()
{
}

void Search::InitPositionAndSize(void)
{
	int X1, Y1, X2, Y2;
	m_Owner->GetPosition(X1, Y1, X2, Y2);
	int FindX=std::min(X1+9,ScrX-22);
	int FindY=std::min(Y2,ScrY-2);
	SetPosition(FindX,FindY,FindX+21,FindY+2);
	m_FindEdit->SetPosition(FindX+2,FindY+1,FindX+19,FindY+1);
}

search_ptr Search::create(Panel* Owner, const Manager::Key& FirstKey)
{
	const auto SearchPtr = std::make_shared<Search>(private_tag(), Owner, FirstKey);
	SearchPtr->init();
	return SearchPtr;
}

void Search::init(void)
{
	SetMacroMode(MACROAREA_SEARCH);
	SetRestoreScreenMode(true);

	m_FindEdit = std::make_unique<EditControl>(shared_from_this(), this);
	m_FindEdit->SetEditBeyondEnd(false);
	m_FindEdit->SetObjectColor(COL_DIALOGEDIT);

	InitPositionAndSize();
}

void Search::Process(void)
{
	Global->WindowManager->ExecuteWindow(shared_from_this());
	Global->WindowManager->CallbackWindow([this](){ ProcessKey(m_FirstKey); });
	Global->WindowManager->ExecuteModal(shared_from_this());
}

bool Search::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key;

	// для вставки воспользуемся макродвижком...
	if (LocalKey()==KEY_CTRLV || LocalKey()==KEY_RCTRLV || LocalKey()==KEY_SHIFTINS || LocalKey()==KEY_SHIFTNUMPAD0)
	{
		string ClipText;
		if (GetClipboardText(ClipText))
		{
			if (!ClipText.empty())
			{
				ProcessName(ClipText);
				ShowBorder();
			}
		}

		return true;
	}
	else if (LocalKey() == KEY_OP_XLAT)
	{
		m_FindEdit->Xlat();
		const auto strTempName = m_FindEdit->GetString();
		m_FindEdit->ClearString();
		ProcessName(strTempName);
		Redraw();
		return true;
	}
	else if (LocalKey() == KEY_OP_PLAINTEXT)
	{
		m_FindEdit->ProcessKey(LocalKey);
		const auto strTempName = m_FindEdit->GetString();
		m_FindEdit->ClearString();
		ProcessName(strTempName);
		Redraw();
		return true;
	}
	else
		LocalKey=_CorrectFastFindKbdLayout(Key.Event(),LocalKey());

	if (LocalKey()==KEY_ESC || LocalKey()==KEY_F10)
	{
		m_KeyToProcess=KEY_NONE;
		Close();
		return true;
	}

	// // _SVS(if (!FirstKey) SysLog(L"Panel::FastFind  Key=%s  %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(&rec)));
	if (LocalKey()>=KEY_ALT_BASE+0x01 && LocalKey()<=KEY_ALT_BASE+65535)
		LocalKey=lower(static_cast<WCHAR>(LocalKey()-KEY_ALT_BASE));
	else if (LocalKey()>=KEY_RALT_BASE+0x01 && LocalKey()<=KEY_RALT_BASE+65535)
		LocalKey=lower(static_cast<WCHAR>(LocalKey()-KEY_RALT_BASE));

	if (LocalKey()>=KEY_ALTSHIFT_BASE+0x01 && LocalKey()<=KEY_ALTSHIFT_BASE+65535)
		LocalKey=lower(static_cast<WCHAR>(LocalKey()-KEY_ALTSHIFT_BASE));
	else if (LocalKey()>=KEY_RALTSHIFT_BASE+0x01 && LocalKey()<=KEY_RALTSHIFT_BASE+65535)
		LocalKey=lower(static_cast<WCHAR>(LocalKey()-KEY_RALTSHIFT_BASE));

	if (LocalKey()==KEY_MULTIPLY)
		LocalKey=L'*';

	switch (LocalKey())
	{
		case KEY_F1:
		{
			Hide();
			{
				Help::create(L"FastFind");
			}
			Show();
			break;
		}
		case KEY_CTRLNUMENTER:   case KEY_RCTRLNUMENTER:
		case KEY_CTRLENTER:      case KEY_RCTRLENTER:
			m_Owner->FindPartName(m_FindEdit->GetString(), TRUE, 1);
			Redraw();
			break;
		case KEY_CTRLSHIFTNUMENTER:  case KEY_RCTRLSHIFTNUMENTER:
		case KEY_CTRLSHIFTENTER:     case KEY_RCTRLSHIFTENTER:
			m_Owner->FindPartName(m_FindEdit->GetString(), TRUE, -1);
			Redraw();
			break;
		case KEY_NONE:
		case KEY_IDLE:
			break;
		default:

			if ((LocalKey()<32 || LocalKey()>=65536) && LocalKey()!=KEY_BS && LocalKey()!=KEY_CTRLY && LocalKey()!=KEY_RCTRLY &&
			        LocalKey()!=KEY_CTRLBS && LocalKey()!=KEY_RCTRLBS && LocalKey()!=KEY_ALT && LocalKey()!=KEY_SHIFT &&
			        LocalKey()!=KEY_CTRL && LocalKey()!=KEY_RALT && LocalKey()!=KEY_RCTRL &&
			        !(LocalKey()==KEY_CTRLINS||LocalKey()==KEY_CTRLNUMPAD0) && // KEY_RCTRLINS/NUMPAD0 passed to panels
			        !(LocalKey()==KEY_SHIFTINS||LocalKey()==KEY_SHIFTNUMPAD0) &&
			        !((LocalKey() == KEY_KILLFOCUS || LocalKey() == KEY_GOTFOCUS) && IsWindowsVistaOrGreater()) // Mantis #2903
			        )
			{
				m_KeyToProcess=LocalKey;
				Close();
				return true;
			}
			auto strLastName = m_FindEdit->GetString();
			if (m_FindEdit->ProcessKey(LocalKey))
			{
				auto strName = m_FindEdit->GetString();

				// уберем двойные '**'
				if (strName.size() > 1
				        && strName.back() == L'*'
				        && strName[strName.size()-2] == L'*')
				{
					strName.pop_back();
					m_FindEdit->SetString(strName);
				}

				/* $ 09.04.2001 SVS
				   проблемы с быстрым поиском.
				   Подробнее в 00573.ChangeDirCrash.txt
				*/
				if (!strName.empty() && strName.front() == L'"')
				{
					strName.erase(0, 1);
					m_FindEdit->SetString(strName);
				}

				if (m_Owner->FindPartName(strName, FALSE, 1))
				{
					strLastName = strName;
				}
				else
				{
					if (Global->CtrlObject->Macro.IsExecuting())// && Global->CtrlObject->Macro.GetLevelState() > 0) // если вставка макросом...
					{
						//Global->CtrlObject->Macro.DropProcess(); // ... то дропнем макропроцесс
						//Global->CtrlObject->Macro.PopState();
						;
					}

					m_FindEdit->SetString(strLastName);
				}

				Redraw();
			}

			break;
	}
	return true;
}

bool Search::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!(MouseEvent->dwButtonState & 3))
		;
	else
		Close();
	return true;
}

void Search::ShowBorder(void) const
{
	SetColor(COL_DIALOGTEXT);
	GotoXY(m_X1+1,m_Y1+1);
	Text(L' ');
	GotoXY(m_X1+20,m_Y1+1);
	Text(L' ');
	Box(m_X1,m_Y1,m_X1+21,m_Y1+2,colors::PaletteColorToFarColor(COL_DIALOGBOX),DOUBLE_BOX);
	GotoXY(m_X1+7,m_Y1);
	SetColor(COL_DIALOGBOXTITLE);
	Text(lng::MSearchFileTitle);
}

void Search::DisplayObject(void)
{
	ShowBorder();
	m_FindEdit->Show();
}

void Search::ProcessName(const string& Src) const
{
	auto Buffer = unquote(m_FindEdit->GetString() + Src);

	for (; !Buffer.empty() && !m_Owner->FindPartName(Buffer, FALSE, 1); Buffer.pop_back())
		;

	if (!Buffer.empty())
	{
		m_FindEdit->SetString(Buffer);
		m_FindEdit->Show();
	}
}

void Search::ResizeConsole(void)
{
	InitPositionAndSize();
}

void Search::Close(void)
{
	Hide();
	Global->WindowManager->DeleteWindow(shared_from_this());
}

void Panel::FastFind(const Manager::Key& FirstKey)
{
	// // _SVS(CleverSysLog Clev(L"Panel::FastFind"));
	Manager::Key KeyToProcess;
	{
		const auto search = Search::create(this, FirstKey);
		search->Process();
		KeyToProcess=search->KeyToProcess();
	}
	Show();
	Parent()->GetKeybar().Redraw();
	Global->ScrBuf->Flush();

	const auto TreePanel = std::dynamic_pointer_cast<TreeList>(Parent()->ActivePanel());
	if (TreePanel && (KeyToProcess() == KEY_ENTER || KeyToProcess() == KEY_NUMENTER))
		TreePanel->ProcessEnter();
	else
		Parent()->ProcessKey(KeyToProcess);
}

bool Panel::IsFocused() const
{
	if (const auto FilePanels = Parent())
	{
		return this == FilePanels->ActivePanel().get();
	}
	return true;
}

void Panel::OnFocusChange(bool Get)
{
	ProcessPluginEvent(Get? FE_GOTFOCUS : FE_KILLFOCUS, nullptr);
	Redraw();
}

bool Panel::IsMouseInClientArea(const MOUSE_EVENT_RECORD* MouseEvent) const
{
	return IsVisible() &&
		InRange(m_X1, MouseEvent->dwMousePosition.X, m_X2) &&
		InRange(m_Y1, MouseEvent->dwMousePosition.Y, m_Y2);
}

bool Panel::ProcessMouseDrag(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (DragX!=-1)
	{
		if (!(MouseEvent->dwButtonState & MOUSE_ANY_BUTTON_PRESSED))
		{
			EndDrag();

			if (!MouseEvent->dwEventFlags && SrcDragPanel!=this)
			{
				MoveToMouse(MouseEvent);
				Redraw();
				SrcDragPanel->ProcessKey(Manager::Key(DragMove ? KEY_DRAGMOVE:KEY_DRAGCOPY));
			}

			return true;
		}

		if (MouseEvent->dwMousePosition.Y<=m_Y1 || MouseEvent->dwMousePosition.Y>=m_Y2 ||
			!Parent()->GetAnotherPanel(SrcDragPanel)->IsVisible())
		{
			EndDrag();
			return true;
		}

		if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED && !MouseEvent->dwEventFlags)
			DragMove=!DragMove;

		if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			if ((abs(MouseEvent->dwMousePosition.X-DragX)>15 || SrcDragPanel!=this) && !m_ModalMode)
			{
				if (SrcDragPanel->GetSelCount()==1 && !DragSaveScr)
				{
					SrcDragPanel->GoToFile(strDragName);
					SrcDragPanel->Show();
				}

				DragMessage(MouseEvent->dwMousePosition.X,MouseEvent->dwMousePosition.Y,DragMove);
				return true;
			}
			else
			{
				delete DragSaveScr;
				DragSaveScr=nullptr;
			}
		}
	}

	if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED && !MouseEvent->dwEventFlags && m_X2 - m_X1<ScrX)
	{
		DWORD FileAttr;
		MoveToMouse(MouseEvent);
		GetSelName(nullptr,FileAttr);

		if (GetSelName(&strDragName,FileAttr) && !TestParentFolderName(strDragName))
		{
			SrcDragPanel=this;
			DragX=MouseEvent->dwMousePosition.X;
			DragY=MouseEvent->dwMousePosition.Y;
			DragMove = IntKeyState.ShiftPressed();
		}
	}

	return false;
}


bool Panel::IsDragging()
{
	return DragSaveScr!=nullptr;
}


void Panel::EndDrag()
{
	delete DragSaveScr;
	DragSaveScr=nullptr;
	DragX=DragY=-1;
}


void Panel::DragMessage(int X,int Y,int Move)
{

	string strSelName;
	int MsgX,Length;

	const auto SelCount = SrcDragPanel->GetSelCount();

	if (!SelCount)
	{
		return;
	}
	else if (SelCount == 1)
	{
		string strCvtName;
		DWORD FileAttr;
		SrcDragPanel->GetSelName(nullptr,FileAttr);
		SrcDragPanel->GetSelName(&strSelName,FileAttr);
		strCvtName = PointToName(strSelName);
		QuoteSpace(strCvtName);
		strSelName = strCvtName;
	}
	else
	{
		strSelName = format(lng::MDragFiles, SelCount);
	}

	auto strDragMsg = format(Move? lng::MDragMove : lng::MDragCopy, strSelName);

	if ((Length=(int)strDragMsg.size())+X>ScrX)
	{
		MsgX=ScrX-Length;

		if (MsgX<0)
		{
			MsgX=0;
			TruncStrFromEnd(strDragMsg,ScrX);
			Length=(int)strDragMsg.size();
		}
	}
	else
		MsgX=X;

	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	delete DragSaveScr;
	DragSaveScr=new SaveScreen(MsgX,Y,MsgX+Length-1,Y);
	GotoXY(MsgX,Y);
	SetColor(COL_PANELDRAGTEXT);
	Text(strDragMsg);
}


const string& Panel::GetCurDir() const
{
	return m_CurDir;
}


bool Panel::SetCurDir(const string& CurDir,bool ClosePanel,bool /*IsUpdated*/)
{
	InitCurDir(CurDir);
	return true;
}


void Panel::InitCurDir(const string& CurDir)
{
	if (!equal_icase(m_CurDir, CurDir) || !TestCurrentDirectory(CurDir))
	{
		m_CurDir = CurDir;

		if (m_PanelMode != panel_mode::PLUGIN_PANEL)
		{
			PrepareDiskPath(m_CurDir);
			if(!IsRootPath(m_CurDir))
			{
				DeleteEndSlash(m_CurDir);
			}
		}
	}
}


/* $ 14.06.2001 KM
   + Добавлена установка переменных окружения, определяющих
     текущие директории дисков как для активной, так и для
     пассивной панели. Это необходимо программам запускаемым
     из FAR.
*/
/* $ 05.10.2001 SVS
   ! Давайте для начала выставим нужные значения для пассивной панели,
     а уж потом...
     А то фигня какая-то получается...
*/
/* $ 14.01.2002 IS
   ! Убрал установку переменных окружения, потому что она производится
     в FarChDir, которая теперь используется у нас для установления
     текущего каталога.
*/
bool Panel::SetCurPath()
{
	if (GetMode() == panel_mode::PLUGIN_PANEL)
		return true;

	const auto AnotherPanel = Parent()->GetAnotherPanel(this);

	if (AnotherPanel->GetMode() != panel_mode::PLUGIN_PANEL)
	{
		if (AnotherPanel->m_CurDir.size() > 1 && AnotherPanel->m_CurDir[1]==L':' &&
		        (m_CurDir.empty() || upper(AnotherPanel->m_CurDir[0])!=upper(m_CurDir[0])))
		{
			// сначала установим переменные окружения для пассивной панели
			// (без реальной смены пути, чтобы лишний раз пассивный каталог
			// не перечитывать)
			FarChDir(AnotherPanel->m_CurDir, false);
		}
	}

	if (!FarChDir(m_CurDir))
	{
		while (!FarChDir(m_CurDir))
		{
			const auto strRoot = GetPathRoot(m_CurDir);

			if (FAR_GetDriveType(strRoot) != DRIVE_REMOVABLE || os::IsDiskInDrive(strRoot))
			{
				if (!os::fs::is_directory(m_CurDir))
				{
					if (CheckShortcutFolder(m_CurDir, true, true) && FarChDir(m_CurDir))
					{
						SetCurDir(m_CurDir,true);
						return true;
					}
				}
				else
					break;
			}

			if (Global->WindowManager->ManagerStarted()) // сначала проверим - а запущен ли менеджер
			{
				SetCurDir(Global->g_strFarPath,true);                    // если запущен - выставим путь который мы точно знаем что существует
				ChangeDisk(shared_from_this());                          // и вызовем меню выбора дисков
			}
			else                                               // оппа...
			{
				string strTemp(m_CurDir);
				CutToParent(m_CurDir);             // подымаемся вверх, для очередной порции ChDir

				if (strTemp.size()==m_CurDir.size())  // здесь проблема - видимо диск недоступен
				{
					SetCurDir(Global->g_strFarPath,true);                 // тогда просто сваливаем в каталог, откуда стартанул FAR.
					break;
				}
				else
				{
					if (FarChDir(m_CurDir))
					{
						SetCurDir(m_CurDir,true);
						break;
					}
				}
			}
		}
		return false;
	}

	return true;
}


void Panel::Hide()
{
	ScreenObject::Hide();
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);

	if (AnotherPanel->IsVisible())
	{
		//if (AnotherPanel->IsFocused())
		if ((AnotherPanel->GetType() == panel_type::FILE_PANEL && AnotherPanel->IsFullScreen()) ||
			(GetType() == panel_type::FILE_PANEL && IsFullScreen()))
				AnotherPanel->Show();
	}
}


void Panel::Show()
{
	if (!GetModalMode())
	{
		const auto AnotherPanel = Parent()->GetAnotherPanel(this);
		if (AnotherPanel->IsVisible())
		{
			if (SaveScr)
			{
				SaveScr->AppendArea(AnotherPanel->SaveScr.get());
			}

			if (AnotherPanel->IsFocused())
			{
				if (AnotherPanel->IsFullScreen())
				{
					SetVisible(true);
					return;
				}

				if (GetType() == panel_type::FILE_PANEL && IsFullScreen())
				{
					ScreenObject::Show();
					AnotherPanel->Show();
					return;
				}
			}
		}
	}

	ScreenObject::Show();
	ShowScreensCount();
}

void Panel::ShowConsoleTitle()
{
	if (!IsFocused())
		return;

	ConsoleTitle::SetFarTitle(m_Title);
}

void Panel::DrawSeparator(int Y) const
{
	if (Y<m_Y2)
	{
		SetColor(COL_PANELBOX);
		GotoXY(m_X1,Y);
		ShowSeparator(m_X2-m_X1+1,1);
	}
}

string Panel::GetTitleForDisplay() const
{
	auto Title = concat(L' ', GetTitle());
	TruncStr(Title, m_X2 - m_X1 - 2);
	Title += L' ';
	return Title;
}

void Panel::ShowScreensCount() const
{
	if (Global->Opt->ShowScreensNumber && !m_X1)
	{
		int Viewers = Global->WindowManager->GetWindowCountByType(windowtype_viewer);
		int Editors = Global->WindowManager->GetWindowCountByType(windowtype_editor);
		int Dialogs = Global->Opt->ShowScreensNumber > 1 ? Global->WindowManager->GetWindowCountByType(windowtype_dialog) : 0;

		if (Viewers>0 || Editors>0 || Dialogs > 0)
		{
			GotoXY(Global->Opt->ShowColumnTitles ? m_X1:m_X1+2,m_Y1);
			SetColor(COL_PANELSCREENSNUMBER);

			auto Counter = L'[' + str(Viewers);

			if (Editors > 0)
			{
				Counter += L'+' + str(Editors);
			}

			if (Dialogs > 0)
			{
				Counter += L',' + str(Dialogs);
			}

			Counter += L']';

			Text(Counter);
		}
	}
}


void Panel::RefreshTitle()
{
	m_Title = concat(L'{', GetTitle(), L'}');
}

string Panel::GetTitle() const
{
	if (m_PanelMode == panel_mode::NORMAL_PANEL)
		return m_ShowShortNames? ConvertNameToShort(m_CurDir) : m_CurDir;

	OpenPanelInfo Info;
	GetOpenPanelInfo(&Info);
	string strTitle = NullToEmpty(Info.PanelTitle);
	RemoveExternalSpaces(strTitle);
	return strTitle;
}

int Panel::SetPluginCommand(int Command,int Param1,void* Param2)
{
	_ALGO(CleverSysLog clv(L"Panel::SetPluginCommand"));
	_ALGO(SysLog(L"(Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));
	int Result=FALSE;
	ProcessingPluginCommand++;

	switch (Command)
	{
		case FCTL_SETVIEWMODE:
			Result = Parent()->ChangePanelViewMode(shared_from_this(), Param1, Parent()->IsTopWindow());
			break;

		case FCTL_SETSORTMODE:
		{
			int Mode=Param1;

			if ((Mode>SM_DEFAULT) && (Mode < SM_COUNT))
			{
				SetSortMode(panel_sort(Mode - 1)); // Уменьшим на 1 из-за SM_DEFAULT
				Result=TRUE;
			}
			break;
		}

		case FCTL_SETNUMERICSORT:
		{
			ChangeNumericSort(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_SETCASESENSITIVESORT:
		{
			ChangeCaseSensitiveSort(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_SETSORTORDER:
		{
			ChangeSortOrder(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_SETDIRECTORIESFIRST:
		{
			ChangeDirectoriesFirst(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_CLOSEPANEL:
			m_PluginCommand=Command;
			m_PluginParam = NullToEmpty((const wchar_t *)Param2);
			Result=TRUE;
			break;

		case FCTL_GETPANELINFO:
		{
			PanelInfo *Info=(PanelInfo *)Param2;

			if(!CheckStructSize(Info))
				break;

			*Info = {};
			Info->StructSize = sizeof(PanelInfo);

			UpdateIfRequired();
			Info->OwnerGuid=FarGuid;
			Info->PluginHandle=nullptr;

			switch (GetType())
			{
			case panel_type::FILE_PANEL:
				Info->PanelType=PTYPE_FILEPANEL;
				break;
			case panel_type::TREE_PANEL:
				Info->PanelType=PTYPE_TREEPANEL;
				break;
			case panel_type::QVIEW_PANEL:
				Info->PanelType=PTYPE_QVIEWPANEL;
				break;
			case panel_type::INFO_PANEL:
				Info->PanelType=PTYPE_INFOPANEL;
				break;
			}

			int X1,Y1,X2,Y2;
			GetPosition(X1,Y1,X2,Y2);
			Info->PanelRect.left=X1;
			Info->PanelRect.top=Y1;
			Info->PanelRect.right=X2;
			Info->PanelRect.bottom=Y2;
			Info->ViewMode=GetViewMode();
			Info->SortMode = static_cast<OPENPANELINFO_SORTMODES>((GetSortMode() < panel_sort::COUNT? SM_UNSORTED - static_cast<int>(panel_sort::UNSORTED) : 0) + static_cast<int>(GetSortMode()));

			Info->Flags |= Global->Opt->ShowHidden? PFLAGS_SHOWHIDDEN : 0;
			Info->Flags |= Global->Opt->Highlight? PFLAGS_HIGHLIGHT : 0;
			Info->Flags |= GetSortOrder()? PFLAGS_REVERSESORTORDER : 0;
			Info->Flags |= GetSortGroups()? PFLAGS_USESORTGROUPS : 0;
			Info->Flags |= GetSelectedFirstMode()? PFLAGS_SELECTEDFIRST : 0;
			Info->Flags |= GetDirectoriesFirst()? PFLAGS_DIRECTORIESFIRST : 0;
			Info->Flags |= GetNumericSort()? PFLAGS_NUMERICSORT : 0;
			Info->Flags |= GetCaseSensitiveSort()? PFLAGS_CASESENSITIVESORT : 0;
			Info->Flags |= (GetMode() == panel_mode::PLUGIN_PANEL)? PFLAGS_PLUGIN : 0;
			Info->Flags |= IsVisible()? PFLAGS_VISIBLE : 0;
			Info->Flags |= IsFocused()? PFLAGS_FOCUS : 0;
			Info->Flags |= Parent()->IsLeft(this)? PFLAGS_PANELLEFT : 0;

			if (GetType() == panel_type::FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)this;

				if (Info->Flags&PFLAGS_PLUGIN)
				{
					Info->OwnerGuid = DestFilePanel->GetPluginHandle()->plugin()->GetGUID();
					Info->PluginHandle = DestFilePanel->GetPluginHandle()->panel();
					static int Reenter=0;
					if (!Reenter)
					{
						Reenter++;
						OpenPanelInfo PInfo;
						DestFilePanel->GetOpenPanelInfo(&PInfo);

						if (PInfo.Flags & OPIF_REALNAMES)
							Info->Flags |= PFLAGS_REALNAMES;

						if (PInfo.Flags & OPIF_DISABLEHIGHLIGHTING)
							Info->Flags &= ~PFLAGS_HIGHLIGHT;

						if (PInfo.Flags & OPIF_USECRC32)
							Info->Flags |= PFLAGS_USECRC32;

						if (PInfo.Flags & OPIF_SHORTCUT)
							Info->Flags |= PFLAGS_SHORTCUT;

						Reenter--;
					}
				}

				DestFilePanel->PluginGetPanelInfo(*Info);
			}

			if (!(Info->Flags&PFLAGS_PLUGIN)) // $ 12.12.2001 DJ - на неплагиновой панели - всегда реальные имена
				Info->Flags |= PFLAGS_REALNAMES;

			Result=TRUE;
			break;
		}

		case FCTL_GETPANELPREFIX:
		{
			string strTemp;

			if (GetType() == panel_type::FILE_PANEL && GetMode() == panel_mode::PLUGIN_PANEL)
			{
				PluginInfo PInfo = {sizeof(PInfo)};
				FileList *DestPanel = ((FileList*)this);
				if (DestPanel->GetPluginInfo(&PInfo))
					strTemp = NullToEmpty(PInfo.CommandPrefix);
			}

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strTemp.data(),Param1);

			Result=(int)strTemp.size()+1;
			break;
		}

		case FCTL_GETPANELHOSTFILE:
		case FCTL_GETPANELFORMAT:
		{
			string strTemp;

			if (GetType() == panel_type::FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)this;
				static int Reenter=0;

				if (!Reenter && GetMode() == panel_mode::PLUGIN_PANEL)
				{
					Reenter++;

					OpenPanelInfo PInfo;
					DestFilePanel->GetOpenPanelInfo(&PInfo);

					switch (Command)
					{
						case FCTL_GETPANELHOSTFILE:
							strTemp=NullToEmpty(PInfo.HostFile);
							break;
						case FCTL_GETPANELFORMAT:
							strTemp=NullToEmpty(PInfo.Format);
							break;
					}

					Reenter--;
				}
			}

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strTemp.data(),Param1);

			Result=(int)strTemp.size()+1;
			break;
		}
		case FCTL_GETPANELDIRECTORY:
		{
			static int Reenter=0;
			if(!Reenter)
			{
				Reenter++;
				ShortcutInfo Info;
				GetShortcutInfo(Info);
				Result = aligned_sizeof<FarPanelDirectory>();
				size_t folderOffset=Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.ShortcutFolder.size()+1));
				size_t pluginFileOffset=Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.PluginFile.size()+1));
				size_t pluginDataOffset=Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.PluginData.size()+1));
				FarPanelDirectory* dirInfo=(FarPanelDirectory*)Param2;
				if(Param1>=Result && CheckStructSize(dirInfo))
				{
					dirInfo->StructSize=sizeof(FarPanelDirectory);
					dirInfo->PluginId=Info.PluginGuid;
					dirInfo->Name=(wchar_t*)((char*)Param2+folderOffset);
					dirInfo->Param=(wchar_t*)((char*)Param2+pluginDataOffset);
					dirInfo->File=(wchar_t*)((char*)Param2+pluginFileOffset);
					std::copy_n(Info.ShortcutFolder.data(), Info.ShortcutFolder.size() + 1, const_cast<wchar_t*>(dirInfo->Name));
					std::copy_n(Info.PluginData.data(), Info.PluginData.size() + 1, const_cast<wchar_t*>(dirInfo->Param));
					std::copy_n(Info.PluginFile.data(), Info.PluginFile.size() + 1, const_cast<wchar_t*>(dirInfo->File));
				}
				Reenter--;
			}
			break;
		}

		case FCTL_GETCOLUMNTYPES:
		case FCTL_GETCOLUMNWIDTHS:

			if (GetType() == panel_type::FILE_PANEL)
			{
				string strColumnTypes,strColumnWidths;
				((FileList *)this)->PluginGetColumnTypesAndWidths(strColumnTypes,strColumnWidths);

				if (Command==FCTL_GETCOLUMNTYPES)
				{
					if (Param1&&Param2)
						xwcsncpy((wchar_t*)Param2,strColumnTypes.data(),Param1);

					Result=(int)strColumnTypes.size()+1;
				}
				else
				{
					if (Param1&&Param2)
						xwcsncpy((wchar_t*)Param2,strColumnWidths.data(),Param1);

					Result=(int)strColumnWidths.size()+1;
				}
			}
			break;

		case FCTL_GETPANELITEM:
		{
			if (GetType() == panel_type::FILE_PANEL && CheckNullOrStructSize(static_cast<FarGetPluginPanelItem*>(Param2)))
				Result = static_cast<int>(static_cast<FileList*>(this)->PluginGetPanelItem(Param1, static_cast<FarGetPluginPanelItem*>(Param2)));
			break;
		}

		case FCTL_GETSELECTEDPANELITEM:
		{
			if (GetType() == panel_type::FILE_PANEL && CheckNullOrStructSize(static_cast<FarGetPluginPanelItem*>(Param2)))
				Result = static_cast<int>(static_cast<FileList*>(this)->PluginGetSelectedPanelItem(Param1, static_cast<FarGetPluginPanelItem*>(Param2)));
			break;
		}

		case FCTL_GETCURRENTPANELITEM:
		{
			if (GetType() == panel_type::FILE_PANEL && CheckNullOrStructSize(static_cast<FarGetPluginPanelItem*>(Param2)))
			{
				PanelInfo Info;
				const auto DestPanel = static_cast<FileList*>(this);
				DestPanel->PluginGetPanelInfo(Info);
				Result = static_cast<int>(DestPanel->PluginGetPanelItem(static_cast<int>(Info.CurrentItem), static_cast<FarGetPluginPanelItem*>(Param2)));
			}
			break;
		}

		case FCTL_BEGINSELECTION:
		{
			if (GetType() == panel_type::FILE_PANEL)
			{
				((FileList *)this)->PluginBeginSelection();
				Result=TRUE;
			}
			break;
		}

		case FCTL_SETSELECTION:
		{
			if (GetType() == panel_type::FILE_PANEL)
			{
				((FileList *)this)->PluginSetSelection(Param1, Param2 != nullptr);
				Result=TRUE;
			}
			break;
		}

		case FCTL_CLEARSELECTION:
		{
			if (GetType() == panel_type::FILE_PANEL)
			{
				static_cast<FileList*>(this)->PluginClearSelection(Param1);
				Result=TRUE;
			}
			break;
		}

		case FCTL_ENDSELECTION:
		{
			if (GetType() == panel_type::FILE_PANEL)
			{
				((FileList *)this)->PluginEndSelection();
				Result=TRUE;
			}
			break;
		}

		case FCTL_UPDATEPANEL:
			Update(Param1?UPDATE_KEEP_SELECTION:0);

			if (GetType() == panel_type::QVIEW_PANEL)
				UpdateViewPanel();

			Result=TRUE;
			break;

		case FCTL_REDRAWPANEL:
		{
			PanelRedrawInfo *Info=(PanelRedrawInfo *)Param2;

			if (CheckStructSize(Info))
			{
				m_CurFile=static_cast<int>(Info->CurrentItem);
				m_CurTopFile=static_cast<int>(Info->TopPanelItem);
			}

			// $ 12.05.2001 DJ перерисовываемся только в том случае, если мы - текущее окно
			if (Parent()->IsTopWindow())
				Redraw();

			Result=TRUE;
			break;
		}

		case FCTL_SETPANELDIRECTORY:
		{
			FarPanelDirectory* dirInfo=(FarPanelDirectory*)Param2;
			if (CheckStructSize(dirInfo))
			{
				string strName(NullToEmpty(dirInfo->Name)), strFile(NullToEmpty(dirInfo->File)), strParam(NullToEmpty(dirInfo->Param));
				Result = ExecShortcutFolder(strName, dirInfo->PluginId, strFile, strParam, false, false, true);
				// restore current directory to active panel path
				if (Result)
				{
					if (!IsFocused())
					{
						Parent()->ActivePanel()->SetCurPath();
					}
				}
			}
			break;
		}

		case FCTL_SETACTIVEPANEL:
		{
			if (IsVisible())
			{
				Parent()->SetActivePanel(this);
				Result=TRUE;
			}
			break;
		}
	}

	ProcessingPluginCommand--;
	return Result;
}


bool Panel::GetCurName(string &strName, string &strShortName) const
{
	strName.clear();
	strShortName.clear();
	return false;
}


bool Panel::GetCurBaseName(string &strName, string &strShortName) const
{
	strName.clear();
	strShortName.clear();
	return false;
}

bool Panel::NeedUpdatePanel(const Panel *AnotherPanel) const
{
	/* Обновить, если обновление разрешено и пути совпадают */
	if ((!Global->Opt->AutoUpdateLimit || static_cast<unsigned>(GetFileCount()) <= static_cast<unsigned>(Global->Opt->AutoUpdateLimit)) &&
		equal_icase(AnotherPanel->m_CurDir, m_CurDir))
		return true;

	return false;
}

bool Panel::GetShortcutInfo(ShortcutInfo& ShortcutInfo) const
{
	bool result=true;
	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		const auto ph = GetPluginHandle();
		ShortcutInfo.PluginGuid = ph->plugin()->GetGUID();
		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(ph, &Info);
		ShortcutInfo.PluginFile = NullToEmpty(Info.HostFile);
		ShortcutInfo.ShortcutFolder = NullToEmpty(Info.CurDir);
		ShortcutInfo.PluginData = NullToEmpty(Info.ShortcutData);
		if(!(Info.Flags&OPIF_SHORTCUT)) result=false;
	}
	else
	{
		ShortcutInfo.PluginGuid=FarGuid;
		ShortcutInfo.PluginFile.clear();
		ShortcutInfo.PluginData.clear();
		ShortcutInfo.ShortcutFolder = m_CurDir;
	}
	return result;
}

bool Panel::SaveShortcutFolder(int Pos, bool Add) const
{
	ShortcutInfo Info;
	if(GetShortcutInfo(Info))
	{
		const auto Function = Add? &Shortcuts::Add : &Shortcuts::Set;
		std::invoke(Function, Shortcuts(Pos), Info.ShortcutFolder, Info.PluginGuid, Info.PluginFile, Info.PluginData);
		return true;
	}
	return false;
}

/*
int Panel::ProcessShortcutFolder(int Key,bool ProcTreePanel)
{
	string strShortcutFolder, strPluginModule, strPluginFile, strPluginData;

	if (GetShortcutFolder(Key-KEY_RCTRL0,&strShortcutFolder,&strPluginModule,&strPluginFile,&strPluginData))
	{
		const auto AnotherPanel = Parent()->GetAnotherPanel(this);

		if (ProcTreePanel)
		{
			if (AnotherPanel->GetType()==FILE_PANEL)
			{
				AnotherPanel->SetCurDir(strShortcutFolder,true);
				AnotherPanel->Redraw();
			}
			else
			{
				SetCurDir(strShortcutFolder,true);
				ProcessKey(KEY_ENTER);
			}
		}
		else
		{
			if (AnotherPanel->GetType()==FILE_PANEL && !strPluginModule.empty())
			{
				AnotherPanel->SetCurDir(strShortcutFolder,true);
				AnotherPanel->Redraw();
			}
		}

		return true;
	}

	return false;
}
*/

bool Panel::ExecShortcutFolder(int Pos)
{
	string strShortcutFolder,strPluginFile,strPluginData;
	GUID PluginGuid;

	if (Shortcuts(Pos).Get(&strShortcutFolder, &PluginGuid, &strPluginFile, &strPluginData))
	{
		return ExecShortcutFolder(strShortcutFolder,PluginGuid,strPluginFile,strPluginData,true);
	}
	return false;
}

bool Panel::ExecShortcutFolder(string& strShortcutFolder, const GUID& PluginGuid, const string& strPluginFile, const string& strPluginData, bool CheckType, bool TryClosest, bool Silent)
{
	auto SrcPanel = shared_from_this();
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);

	if(CheckType)
	{
		switch (GetType())
		{
		case panel_type::FILE_PANEL:
			break;

		case panel_type::TREE_PANEL:
		case panel_type::QVIEW_PANEL:
		case panel_type::INFO_PANEL:
			if (AnotherPanel->GetType() == panel_type::FILE_PANEL)
				SrcPanel=AnotherPanel;
			break;
		}
	}

	bool CheckFullScreen=SrcPanel->IsFullScreen();

	if (PluginGuid != FarGuid)
	{
		if (ProcessPluginEvent(FE_CLOSE, nullptr))
		{
			return true;
		}

		if (const auto pPlugin = Global->CtrlObject->Plugins->FindPlugin(PluginGuid))
		{
			if (pPlugin->has(iOpen))
			{
				if (!strPluginFile.empty())
				{
					string strRealDir;
					strRealDir = strPluginFile;

					if (CutToSlash(strRealDir))
					{
						SrcPanel->SetCurDir(strRealDir,true);
						SrcPanel->GoToFile(PointToName(strPluginFile));

						SrcPanel->ClearAllItem();
					}
				}

				const auto IsActive = SrcPanel->IsFocused();
				OpenShortcutInfo info=
				{
					sizeof(OpenShortcutInfo),
					strPluginFile.empty()?nullptr:strPluginFile.data(),
					strPluginData.empty()?nullptr:strPluginData.data(),
					IsActive? FOSF_ACTIVE : FOSF_NONE
				};

				if (auto hNewPlugin = Global->CtrlObject->Plugins->Open(pPlugin, OPEN_SHORTCUT, FarGuid, reinterpret_cast<intptr_t>(&info)))
				{
					const auto NewPanel = Parent()->ChangePanel(SrcPanel, panel_type::FILE_PANEL, TRUE, TRUE);
					const auto NewPluginCopy = hNewPlugin.get();
					NewPanel->SetPluginMode(std::move(hNewPlugin), L"", IsActive || !Parent()->GetAnotherPanel(NewPanel)->IsVisible());

					if (!strShortcutFolder.empty())
					{
						UserDataItem UserData = {}; //????
						Global->CtrlObject->Plugins->SetDirectory(NewPluginCopy, strShortcutFolder, 0, &UserData);
					}

					NewPanel->Update(0);
					NewPanel->Show();
				}
			}
		}

		return true;
	}

	if (!CheckShortcutFolder(strShortcutFolder, TryClosest, Silent) || ProcessPluginEvent(FE_CLOSE, nullptr))
	{
		return false;
	}

    /*
	if (SrcPanel->GetType()!=FILE_PANEL)
	{
		SrcPanel = Parent()->ChangePanel(SrcPanel,FILE_PANEL,TRUE,TRUE);
	}
    */

	SrcPanel->SetCurDir(strShortcutFolder,true);

	if (CheckFullScreen!=SrcPanel->IsFullScreen())
		Parent()->GetAnotherPanel(SrcPanel)->Show();

	SrcPanel->Refresh();
	return true;
}

bool Panel::CreateFullPathName(const string& Name, const string& ShortName, DWORD FileAttr, string& strDest, bool UNC, bool ShortNameAsIs) const
{
	string strFileName = strDest;
	if (FindSlash(Name) == string::npos && FindSlash(ShortName) == string::npos)
	{
		strFileName = ConvertNameToFull(strFileName);
	}

	/* BUGBUG весь этот if какая то чушь
	else if (ShowShortNames)
	{
	  string strTemp = Name;

	  if (NameLastSlash)
	    strTemp.SetLength(1+NameLastSlash-Name);

	  const wchar_t *NamePtr = wcsrchr(strFileName, L'\\');

	  if(NamePtr )
	    NamePtr++;
	  else
	    NamePtr=strFileName;

	  strTemp += NameLastSlash?NameLastSlash+1:Name; //??? NamePtr??? BUGBUG
	  strFileName = strTemp;
	}
	*/

	if (m_ShowShortNames && ShortNameAsIs)
		strFileName = ConvertNameToShort(strFileName);

	/* $ 29.01.2001 VVM
	  + По CTRL+ALT+F в командную строку сбрасывается UNC-имя текущего файла. */
	if (UNC)
		strFileName = ConvertNameToUNC(strFileName);

	// $ 20.10.2000 SVS Сделаем фичу Ctrl-F опциональной!
	if (Global->Opt->PanelCtrlFRule)
	{
		/* $ 13.10.2000 tran
		  по Ctrl-f имя должно отвечать условиям на панели */
		if (m_ViewSettings.Flags&PVS_FOLDERUPPERCASE)
		{
			if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
			{
				inplace::upper(strFileName);
			}
			else
			{
				inplace::upper(strFileName, 0, FindLastSlash(strFileName));
			}
		}

		if ((m_ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE) && !(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			const auto pos = FindLastSlash(strFileName);
			if (pos != string::npos && !IsCaseMixed(strFileName.data() + pos))
			{
				inplace::lower(strFileName, pos);
			}
		}

		if ((m_ViewSettings.Flags&PVS_FILELOWERCASE) && !(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			const auto pos = FindLastSlash(strFileName);
			if (pos != string::npos)
			{
				inplace::lower(strFileName, pos);
			}
		}
	}

	strDest = strFileName;
	return !strDest.empty();
}

void Panel::exclude_sets(string& mask)
{
	ReplaceStrings(mask, L"[", L"<[%>", true);
	ReplaceStrings(mask, L"]", L"[]]", true);
	ReplaceStrings(mask, L"<[%>", L"[[]", true);
}

FilePanels* Panel::Parent(void)const
{
	return dynamic_cast<FilePanels*>(GetOwner().get());
}
