/*
manager.cpp

Переключение между несколькими file panels, viewers, editors, dialogs
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

#include "manager.hpp"
#include "keys.hpp"
#include "window.hpp"
#include "vmenu2.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "savescr.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "grabber.hpp"
#include "message.hpp"
#include "config.hpp"
#include "plist.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "exitcode.hpp"
#include "scrbuf.hpp"
#include "console.hpp"
#include "configdb.hpp"
#include "DlgGuid.hpp"
#include "plugins.hpp"
#include "language.hpp"
#include "desktop.hpp"
#include "keybar.hpp"

long Manager::CurrentWindowType=-1;

// fixed indexes
enum
{
	DesktopIndex,
	FilePanelsIndex,
};

class Manager::MessageAbstract
{
public:
	virtual ~MessageAbstract() {}
	virtual bool Process(void) = 0;
};

class MessageCallback: public Manager::MessageAbstract
{
public:
	MessageCallback(const std::function<void(void)>& Callback): m_Callback(Callback) {}
	virtual bool Process(void) override { m_Callback(); return true; }

private:
	std::function<void(void)> m_Callback;
};

class MessageOneWindow: public Manager::MessageAbstract
{
public:
	MessageOneWindow(window* Param, const std::function<void(window*)>& Callback): m_Param(Param), m_Callback(Callback) {}
	virtual bool Process(void) override { m_Callback(m_Param); return true; }

private:
	window* m_Param;
	std::function<void(window*)> m_Callback;
};

class MessageTwoWindows: public Manager::MessageAbstract
{
public:
	MessageTwoWindows(window* Param1, window* Param2, const std::function<void(window*, window*)>& Callback): m_Param1(Param1), m_Param2(Param2), m_Callback(Callback) {}
	virtual bool Process(void) override { m_Callback(m_Param1, m_Param2); return true; }

private:
	window* m_Param1;
	window* m_Param2;
	std::function<void(window*, window*)> m_Callback;
};

class MessageStop: public Manager::MessageAbstract
{
public:
	MessageStop() {}
	virtual bool Process(void) override { return false; }
};

Manager::Manager():
	m_currentWindow(nullptr),
	m_windowPos(-1),
	ModalEVCount(0),
	EndLoop(false),
	ModalExitCode(-1),
	StartManager(false)
{
	m_windows.reserve(1024);
	m_nodalWindows.reserve(1024);
}

Manager::~Manager()
{
}

/* $ 29.12.2000 IS
  Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
  если пользователь продолжил редактировать файл.
  Возвращает TRUE, если все закрыли и можно выходить из фара.
*/
BOOL Manager::ExitAll()
{
	_MANAGER(CleverSysLog clv(L"Manager::ExitAll()"));

	// BUGBUG don't use iterators here, may be invalidated by DeleteCommit()
	for(size_t i = m_nodalWindows.size(); i; --i)
	{
		if (i - 1 >= m_nodalWindows.size())
			continue;
		auto CurrentWindow = m_nodalWindows[i - 1];
		if (!CurrentWindow->GetCanLoseFocus(TRUE))
		{
			auto PrevWindowCount = m_nodalWindows.size();
			CurrentWindow->ProcessKey(Manager::Key(KEY_ESC));
			Commit();

			if (PrevWindowCount == m_nodalWindows.size())
			{
				return FALSE;
			}
		}
	}

	// BUGBUG don't use iterators here, may be invalidated by DeleteCommit()
	for(size_t i = m_windows.size(); i; --i)
	{
		if (i - 1 >= m_windows.size())
			continue;
		auto CurrentWindow = m_windows[i - 1];
		if (!CurrentWindow->GetCanLoseFocus(TRUE))
		{
			ActivateWindow(CurrentWindow);
			Commit();
			auto PrevWindoowCount = m_windows.size();
			CurrentWindow->ProcessKey(Manager::Key(KEY_ESC));
			Commit();

			if (PrevWindoowCount == m_windows.size())
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

void Manager::CloseAll()
{
	_MANAGER(CleverSysLog clv(L"Manager::CloseAll()"));
	while(!m_nodalWindows.empty())
	{
		DeleteWindow(m_nodalWindows.back());
		Commit();
	}
	while(!m_windows.empty())
	{
		DeleteWindow(m_windows.back());
		Commit();
	}
	m_windows.clear();
}

void Manager::PushWindow(window* Param, window_callback Callback)
{
	m_Queue.push_back(std::make_unique<MessageOneWindow>(Param,[this,Callback](window* Param){(this->*Callback)(Param);}));
}

void Manager::CheckAndPushWindow(window* Param, window_callback Callback)
{
	//assert(Param);
	if (Param&&!Param->IsDeleting()) PushWindow(Param,Callback);
}

void Manager::ProcessWindowByPos(int Index, window_callback Callback)
{
	window* Window=GetWindow(Index);
	assert(Window); //eсли Window == nullptr -> используется устаревший индекс.
	(this->*Callback)(Window);
}

void Manager::CallbackWindow(const std::function<void(void)>& Callback)
{
	m_Queue.push_back(std::make_unique<MessageCallback>(Callback));
}

void Manager::InsertWindow(window *Inserted)
{
	_MANAGER(CleverSysLog clv(L"Manager::InsertWindow(window *Inserted, int Index)"));
	_MANAGER(SysLog(L"Inserted=%p, Index=%i",Inserted, Index));

	CheckAndPushWindow(Inserted,&Manager::InsertCommit);
}

void Manager::DeleteWindow(window *Deleted)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeleteWindow(window *Deleted)"));
	_MANAGER(SysLog(L"Deleted=%p",Deleted));

	window* Window=Deleted?Deleted:m_currentWindow;
	assert(Window);
	CheckAndPushWindow(Window,&Manager::DeleteCommit);
	if (Window->GetDynamicallyBorn())
		Window->SetDeleting();
}

void Manager::DeleteWindow(int Index)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeleteWindow(int Index)"));
	_MANAGER(SysLog(L"Index=%i",Index));
	ProcessWindowByPos(Index,&Manager::DeleteWindow);
}

void Manager::RedeleteWindow(window *Deleted)
{
	m_Queue.push_back(std::make_unique<MessageStop>());
	PushWindow(Deleted,&Manager::DeleteCommit);
}

void Manager::ExecuteNonModal(const window *NonModal)
{
	_MANAGER(CleverSysLog clv(L"Manager::ExecuteNonModal ()"));
	if (!NonModal) return;
	for (;;)
	{
		Commit();

		if (m_currentWindow!=NonModal || EndLoop)
		{
			break;
		}

		ProcessMainLoop();
	}
}

void Manager::ExecuteModal(window *Executed)
{
	_MANAGER(CleverSysLog clv(L"Manager::ExecuteModal (window *Executed)"));
	_MANAGER(SysLog(L"Executed=%p",Executed));

	volatile bool stop=false;
	auto& stop_ref=m_Executed[Executed];
	if (stop_ref) return;
	stop_ref=&stop;

	auto OriginalStartManager = StartManager;
	StartManager = true;

	for (;;)
	{
		Commit();

		if (stop)
		{
			break;
		}

		ProcessMainLoop();
	}

	StartManager = OriginalStartManager;
	return;// GetModalExitCode();
}

int Manager::GetModalExitCode() const
{
	return ModalExitCode;
}

/* $ 11.10.2001 IS
   Подсчитать количество окон с указанным именем.
*/
int Manager::CountWindowsWithName(const string& Name, BOOL IgnoreCase)
{
	int Counter=0;
	typedef int (*CompareFunction)(const string&, const string&);
	CompareFunction CaseSenitive = StrCmp, CaseInsensitive = StrCmpI;
	CompareFunction CmpFunction = IgnoreCase? CaseInsensitive : CaseSenitive;

	string strType, strCurName;

	std::for_each(CONST_RANGE(m_windows, i)
	{
		i->GetTypeAndName(strType, strCurName);
		if (!CmpFunction(Name, strCurName))
			++Counter;
	});

	return Counter;
}

/*!
  \return Возвращает nullptr если нажат "отказ" или если нажато текущее окно.
  Другими словами, если немодальное окно не поменялось.
  Если же поменялось, то тогда функция должна возвратить
  указатель на предыдущее окно.
*/
window *Manager::WindowMenu()
{
	/* $ 28.04.2002 KM
	    Флаг для определения того, что меню переключения
	    экранов уже активировано.
	*/
	static int AlreadyShown=FALSE;

	if (AlreadyShown)
		return nullptr;

	int ExitCode, CheckCanLoseFocus=m_currentWindow->GetCanLoseFocus();
	{
		VMenu2 ModalMenu(MSG(MScreensTitle),nullptr,0,ScrY-4);
		ModalMenu.SetHelp(L"ScrSwitch");
		ModalMenu.SetFlags(VMENU_WRAPMODE);
		ModalMenu.SetPosition(-1,-1,0,0);
		ModalMenu.SetId(ScreensSwitchId);

		size_t n = 0;
		std::for_each(CONST_RANGE(m_windows, i)
		{
			string strType, strName, strNumText;
			i->GetTypeAndName(strType, strName);
			MenuItemEx ModalMenuItem;

			if (n < 10)
				strNumText = str_printf(L"&%d. ", n);
			else if (n < 36)
				strNumText = str_printf(L"&%c. ", n + 55);  // 55='A'-10
			else
				strNumText = L"&   ";

			//TruncPathStr(strName,ScrX-24);
			ReplaceStrings(strName, L"&", L"&&");
			/*  добавляется "*" если файл изменен */
			ModalMenuItem.strName = str_printf(L"%s%-10.10s %c %s", strNumText.data(), strType.data(),(i->IsFileModified()?L'*':L' '), strName.data());
			ModalMenuItem.SetSelect(static_cast<int>(n) == m_windowPos);
			ModalMenu.AddItem(ModalMenuItem);
			++n;
		});

		AlreadyShown=TRUE;
		ExitCode=ModalMenu.Run();
		AlreadyShown=FALSE;
	}

	if (CheckCanLoseFocus)
	{
		if (ExitCode>=0)
		{
			window* ActivatedWindow=GetWindow(ExitCode);
			ActivateWindow(ActivatedWindow);
			return (ActivatedWindow == m_currentWindow || !m_currentWindow->GetCanLoseFocus())? nullptr : m_currentWindow;
		}
	}

	return nullptr;
}


int Manager::GetWindowCountByType(int Type)
{
	int ret=0;

	std::for_each(CONST_RANGE(m_windows, i)
	{
		if (!i->IsDeleting() && i->GetExitCode() != XC_QUIT && i->GetType() == Type)
			ret++;
	});

	return ret;
}

/*$ 11.05.2001 OT Теперь можно искать файл не только по полному имени, но и отдельно - путь, отдельно имя */
window* Manager::FindWindowByFile(int ModalType,const string& FileName, const wchar_t *Dir)
{
	string strBufFileName;
	string strFullFileName = FileName;

	if (Dir)
	{
		strBufFileName = Dir;
		AddEndSlash(strBufFileName);
		strBufFileName += FileName;
		strFullFileName = strBufFileName;
	}

	auto ItemIterator = std::find_if(CONST_RANGE(m_windows, i) -> bool
	{
		string strType, strName;

		// Mantis#0000469 - получать Name будем только при совпадении ModalType
		if (!i->IsDeleting() && i->GetType() == ModalType)
		{
			i->GetTypeAndName(strType, strName);

			if (!StrCmpI(strName, strFullFileName))
				return true;
		}
		return false;
	});

	return ItemIterator == m_windows.cend()? nullptr : *ItemIterator;
}

bool Manager::ShowBackground()
{
	Global->CtrlObject->Desktop->Show();
	return true;
}

void Manager::ActivateWindow(window *Activated)
{
	_MANAGER(CleverSysLog clv(L"Manager::ActivateWindow(window *Activated)"));
	_MANAGER(SysLog(L"Activated=%i",Activated));

	if (Activated) CheckAndPushWindow(Activated,&Manager::ActivateCommit);
}

void Manager::ActivateWindow(int Index)
{
	_MANAGER(CleverSysLog clv(L"Manager::ActivateWindow(int Index)"));
	_MANAGER(SysLog(L"Index=%i",Index));
	ProcessWindowByPos(Index,&Manager::ActivateWindow);
}

void Manager::DeactivateWindow(window *Deactivated,int Direction)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeactivateWindow(window *Deactivated,int Direction)"));
	_MANAGER(SysLog(L"Deactivated=%p, Direction=%d",Deactivated,Direction));

	if (Direction)
	{

		m_windowPos += Direction;

		// For now we don't want to switch to the desktop window with Ctrl-[Shift-]Tab
		// Replace with DesktopIndex if needed
		static const auto StartingIndex = FilePanelsIndex;

		if (m_windowPos < StartingIndex)
			m_windowPos = static_cast<int>(m_windows.size() - 1);
		else if (m_windowPos >= static_cast<int>(m_windows.size()))
			m_windowPos = StartingIndex;

		ActivateWindow(m_windowPos);
	}
	else
	{
		// Direction==0
		// Direct access from menu or (in future) from plugin
	}

	CheckAndPushWindow(Deactivated,&Manager::DeactivateCommit);
}

void Manager::RefreshWindow(window *Refreshed)
{
	_MANAGER(CleverSysLog clv(L"Manager::RefreshWindow(window *Refreshed)"));
	_MANAGER(SysLog(L"Refreshed=%p",Refreshed));

	CheckAndPushWindow(Refreshed?Refreshed:m_currentWindow,&Manager::RefreshCommit);
}

void Manager::RefreshWindow(int Index)
{
	_MANAGER(CleverSysLog clv(L"Manager::RefreshWindow(int Index)"));
	_MANAGER(SysLog(L"Index=%d",Index));
	ProcessWindowByPos(Index,&Manager::RefreshWindow);
}

void Manager::ExecuteWindow(window *Executed)
{
	_MANAGER(CleverSysLog clv(L"Manager::ExecuteWindow(window *Executed)"));
	_MANAGER(SysLog(L"Executed=%p",Executed));
	CheckAndPushWindow(Executed,&Manager::ExecuteCommit);
}

void Manager::UpdateWindow(window* Old,window* New)
{
	m_Queue.push_back(std::make_unique<MessageTwoWindows>(Old,New,[this](window* Param1,window* Param2){this->UpdateCommit(Param1,Param2);}));
}

void Manager::SwitchToPanels()
{
	_MANAGER(CleverSysLog clv(L"Manager::SwitchToPanels()"));
	if (!Global->OnlyEditorViewerUsed)
	{
		ActivateWindow(FilePanelsIndex);
	}
}


bool Manager::HaveAnyWindow() const
{
	return !m_windows.empty() || !m_Queue.empty() || m_currentWindow;
}

bool Manager::OnlyDesktop() const
{
	return m_windows.size() == 1 && m_Queue.empty();
}

void Manager::EnterMainLoop()
{
	Global->WaitInFastFind=0;
	StartManager = true;

	for (;;)
	{
		Commit();

		if (EndLoop || (!HaveAnyWindow() || OnlyDesktop()))
		{
			break;
		}

		ProcessMainLoop();
	}
}

void Manager::SetLastInputRecord(const INPUT_RECORD *Rec)
{
	if (&LastInputRecord != Rec)
		LastInputRecord=*Rec;
}


void Manager::ProcessMainLoop()
{
	if ( m_currentWindow && !m_currentWindow->ProcessEvents() )
	{
		ProcessKey(Manager::Key(KEY_IDLE));
	}
	else
	{
		// Mantis#0000073: Не работает автоскролинг в QView
		Global->WaitInMainLoop=IsPanelsActive(true);
		//WaitInFastFind++;
		int Key=GetInputRecord(&LastInputRecord);
		//WaitInFastFind--;
		Global->WaitInMainLoop=FALSE;

		if (EndLoop)
			return;

		if (LastInputRecord.EventType==MOUSE_EVENT && !(Key==KEY_MSWHEEL_UP || Key==KEY_MSWHEEL_DOWN || Key==KEY_MSWHEEL_RIGHT || Key==KEY_MSWHEEL_LEFT))
		{
				// используем копию структуры, т.к. LastInputRecord может внезапно измениться во время выполнения ProcessMouse
				MOUSE_EVENT_RECORD mer=LastInputRecord.Event.MouseEvent;
				ProcessMouse(&mer);
		}
		else
			ProcessKey(Manager::Key(Key));
	}

	if(IsPanelsActive())
	{
		if(!Global->PluginPanelsCount)
		{
			Global->CtrlObject->Plugins->RefreshPluginsList();
		}
	}
}

void Manager::ExitMainLoop(int Ask)
{
	if (Global->CloseFAR)
	{
		Global->CloseFARMenu=TRUE;
	};

	const wchar_t* const Items[] = {MSG(MAskQuit),MSG(MYes),MSG(MNo)};

	if (!Ask || !Global->Opt->Confirm.Exit || !Message(0,2,MSG(MQuit),Items, ARRAYSIZE(Items), nullptr, nullptr, &FarAskQuitId))
	{
		/* $ 29.12.2000 IS
		   + Проверяем, сохранены ли все измененные файлы. Если нет, то не выходим
		     из фара.
		*/
		if (ExitAll() || Global->CloseFAR)
		{
			FilePanels *cp;

			if (!(cp = Global->CtrlObject->Cp())
			        || (!cp->LeftPanel->ProcessPluginEvent(FE_CLOSE,nullptr) && !cp->RightPanel->ProcessPluginEvent(FE_CLOSE,nullptr)))
				EndLoop=true;
		}
		else
		{
			Global->CloseFARMenu=FALSE;
		}
	}
}

void Manager::AddGlobalKeyHandler(const std::function<int(Key)>& Handler)
{
	m_GlobalKeyHandlers.emplace_back(Handler);
}

int Manager::ProcessKey(Key key)
{
	if (m_currentWindow)
	{
		/*** БЛОК ПРИВЕЛЕГИРОВАННЫХ КЛАВИШ ! ***/
		/***   КОТОРЫЕ НЕЛЬЗЯ НАМАКРОСИТЬ    ***/

		switch (key.FarKey)
		{
			case KEY_ALT|KEY_NUMPAD0:
			case KEY_RALT|KEY_NUMPAD0:
			case KEY_ALTINS:
			case KEY_RALTINS:
			{
				RunGraber();
				return TRUE;
			}
			case KEY_CONSOLE_BUFFER_RESIZE:
				Sleep(1);
				ResizeAllWindows();
				return TRUE;
		}

		/*** А вот здесь - все остальное! ***/
		if (!Global->IsProcessAssignMacroKey)
		{
			FOR(const auto& i, m_GlobalKeyHandlers)
			{
				if (i(key))
				{
					return TRUE;
				}
			}

			switch (key.FarKey)
			{
				case KEY_CTRLW:
				case KEY_RCTRLW:
					ShowProcessList();
					return TRUE;

				case KEY_F11:
				{
					int WindowType = Global->WindowManager->GetCurrentWindow()->GetType();
					static int reentry=0;
					if(!reentry && (WindowType == windowtype_dialog || WindowType == windowtype_menu))
					{
						++reentry;
						int r=m_currentWindow->ProcessKey(key);
						--reentry;
						return r;
					}

					PluginsMenu();
					Global->WindowManager->RefreshWindow();
					//_MANAGER(SysLog(-1));
					return TRUE;
				}
				case KEY_ALTF9:
				case KEY_RALTF9:
				{
					//_MANAGER(SysLog(1,"Manager::ProcessKey, KEY_ALTF9 pressed..."));
					Sleep(1);
					SetVideoMode();
					Sleep(1);

					/* В процессе исполнения Alt-F9 (в нормальном режиме) в очередь
					   консоли попадает WINDOW_BUFFER_SIZE_EVENT, формируется в
					   ChangeVideoMode().
					   В режиме исполнения макросов ЭТО не происходит по вполне понятным
					   причинам.
					*/
					if (Global->CtrlObject->Macro.IsExecuting())
					{
						int PScrX=ScrX;
						int PScrY=ScrY;
						Sleep(1);
						GetVideoMode(CurSize);

						if (PScrX+1 == CurSize.X && PScrY+1 == CurSize.Y)
						{
							//_MANAGER(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_NONE"));
							return TRUE;
						}
						else
						{
							PrevScrX=PScrX;
							PrevScrY=PScrY;
							//_MANAGER(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_CONSOLE_BUFFER_RESIZE"));
							Sleep(1);
							return ProcessKey(Manager::Key(KEY_CONSOLE_BUFFER_RESIZE));
						}
					}

					//_MANAGER(SysLog(-1));
					return TRUE;
				}
				case KEY_F12:
				{
					int WindowType = Global->WindowManager->GetCurrentWindow()->GetType();

					if (WindowType != windowtype_help && WindowType != windowtype_dialog && WindowType != windowtype_menu && WindowType != windowtype_grabber && WindowType != windowtype_hmenu)
					{
						DeactivateWindow(WindowMenu(),0);
						//_MANAGER(SysLog(-1));
						return TRUE;
					}

					break; // отдадим F12 дальше по цепочке
				}

				case KEY_CTRLALTSHIFTPRESS:
				case KEY_RCTRLALTSHIFTPRESS:
				{
					if (!(Global->Opt->CASRule&1) && key.FarKey == KEY_CTRLALTSHIFTPRESS)
						break;

					if (!(Global->Opt->CASRule&2) && key.FarKey == KEY_RCTRLALTSHIFTPRESS)
						break;

						if (m_currentWindow->CanFastHide())
						{
							int isPanelFocus=m_currentWindow->GetType() == windowtype_panels;

							if (isPanelFocus)
							{
								int LeftVisible=Global->CtrlObject->Cp()->LeftPanel->IsVisible();
								int RightVisible=Global->CtrlObject->Cp()->RightPanel->IsVisible();
								int CmdLineVisible=Global->CtrlObject->CmdLine->IsVisible();
								int KeyBarVisible=Global->CtrlObject->Cp()->GetKeybar().IsVisible();
								ShowBackground();
								Global->CtrlObject->Cp()->LeftPanel->HideButKeepSaveScreen();
								Global->CtrlObject->Cp()->RightPanel->HideButKeepSaveScreen();

								switch (Global->Opt->PanelCtrlAltShiftRule)
								{
									case 0:
										if (CmdLineVisible)
											Global->CtrlObject->CmdLine->Show();
										if (KeyBarVisible)
											Global->CtrlObject->Cp()->GetKeybar().Show();
										break;
									case 1:
										if (KeyBarVisible)
											Global->CtrlObject->Cp()->GetKeybar().Show();
										break;
								}

								WaitKey(key.FarKey==KEY_CTRLALTSHIFTPRESS?KEY_CTRLALTSHIFTRELEASE:KEY_RCTRLALTSHIFTRELEASE);

								if (LeftVisible)      Global->CtrlObject->Cp()->LeftPanel->Show();

								if (RightVisible)     Global->CtrlObject->Cp()->RightPanel->Show();

								if (CmdLineVisible)   Global->CtrlObject->CmdLine->Show();

								if (KeyBarVisible)    Global->CtrlObject->Cp()->GetKeybar().Show();
							}
							else
							{
								ImmediateHide();
								WaitKey(key.FarKey==KEY_CTRLALTSHIFTPRESS?KEY_CTRLALTSHIFTRELEASE:KEY_RCTRLALTSHIFTRELEASE);
							}

							Global->WindowManager->RefreshWindow();
						}

						return TRUE;

					break;
				}
				case KEY_CTRLTAB:
				case KEY_RCTRLTAB:
				case KEY_CTRLSHIFTTAB:
				case KEY_RCTRLSHIFTTAB:

					if (m_currentWindow->GetCanLoseFocus())
					{
						DeactivateWindow(m_currentWindow,(key.FarKey==KEY_CTRLTAB||key.FarKey==KEY_RCTRLTAB)?1:-1);
					}
					else
						break;

					_MANAGER(SysLog(-1));
					return TRUE;
			}
		}

		m_currentWindow->UpdateKeyBar();
		m_currentWindow->ProcessKey(key);
	}

	_MANAGER(SysLog(-1));
	return FALSE;
}

int Manager::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	int ret=FALSE;

//    _D(SysLog(1,"Manager::ProcessMouse()"));
	if (m_currentWindow)
		ret=m_currentWindow->ProcessMouse(MouseEvent);

//    _D(SysLog(L"Manager::ProcessMouse() ret=%i",ret));
	_MANAGER(SysLog(-1));
	return ret;
}

void Manager::PluginsMenu() const
{
	_MANAGER(SysLog(1));
	int curType = m_currentWindow->GetType();

	if (curType == windowtype_panels || curType == windowtype_editor || curType == windowtype_viewer || curType == windowtype_dialog || curType == windowtype_menu)
	{
		/* 02.01.2002 IS
		   ! Вывод правильной помощи по Shift-F1 в меню плагинов в редакторе/вьюере/диалоге
		   ! Если на панели QVIEW или INFO открыт файл, то считаем, что это
		     полноценный вьюер и запускаем с соответствующим параметром плагины
		*/
		if (curType==windowtype_panels)
		{
			int pType=Global->CtrlObject->Cp()->ActivePanel()->GetType();

			if (pType==QVIEW_PANEL || pType==INFO_PANEL)
			{
				string strType, strCurFileName;
				Global->CtrlObject->Cp()->GetTypeAndName(strType, strCurFileName);

				if (!strCurFileName.empty())
				{
					DWORD Attr=api::GetFileAttributes(strCurFileName);

					// интересуют только обычные файлы
					if (Attr!=INVALID_FILE_ATTRIBUTES && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
						curType=windowtype_viewer;
				}
			}
		}

		// в редакторе, вьюере или диалоге покажем свою помощь по Shift-F1
		const wchar_t *Topic=curType==windowtype_editor?L"Editor":
		                     curType==windowtype_viewer?L"Viewer":
		                     curType==windowtype_dialog?L"Dialog":nullptr;
		Global->CtrlObject->Plugins->CommandsMenu(curType,0,Topic);
	}

	_MANAGER(SysLog(-1));
}

bool Manager::IsPanelsActive(bool and_not_qview) const
{
	if (m_windowPos>=0 && m_currentWindow)
	{
		auto fp = dynamic_cast<FilePanels*>(m_currentWindow);
		return fp && (!and_not_qview || fp->ActivePanel()->GetType() != QVIEW_PANEL);
	}
	else
	{
		return false;
	}
}

window* Manager::GetWindow(size_t Index) const
{
	if (Index >= m_windows.size() || m_windows.empty())
	{
		return nullptr;
	}

	return m_windows[Index];
}

int Manager::IndexOfStack(window *Window)
{
	auto ItemIterator = std::find(ALL_CONST_RANGE(m_nodalWindows), Window);
	return ItemIterator != m_nodalWindows.cend()? ItemIterator - m_nodalWindows.cbegin() : -1;
}

int Manager::IndexOf(window *Window)
{
	auto ItemIterator = std::find(ALL_CONST_RANGE(m_windows), Window);
	return ItemIterator != m_windows.cend() ? ItemIterator - m_windows.cbegin() : -1;
}

void Manager::Commit(void)
{
	_MANAGER(CleverSysLog clv(L"Manager::Commit()"));
	_MANAGER(ManagerClass_Dump(L"ManagerClass"));
	while (!m_Queue.empty())
	{
		auto message=std::move(m_Queue.front());
		m_Queue.pop_front();
		if (!message->Process()) break;
	}
}

void Manager::InsertCommit(window* Param)
{
	_MANAGER(CleverSysLog clv(L"Manager::InsertCommit()"));
	_MANAGER(SysLog(L"InsertedWindow=%p",Param));
	if (Param)
	{
		Param->m_previousWindow=m_currentWindow;
		m_windows.emplace_back(Param);
		ActivateCommit(Param);
	}
}

void Manager::DeleteCommit(window* Param)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeleteCommit()"));
	_MANAGER(SysLog(L"DeletedWindow=%p",Param));

	if (!Param)
		return;

	if (Param->IsBlocked())
	{
		RedeleteWindow(Param);
		return;
	}

	Param->OnDestroy();

	int ModalIndex=IndexOfStack(Param);
	int WindowIndex=IndexOf(Param);
	assert(!(-1!=ModalIndex&&-1!=WindowIndex));

	std::for_each(CONST_RANGE(m_windows, i)
	{
		if (i->m_previousWindow == Param)
		{
			i->m_previousWindow = Global->CtrlObject->Cp();
		}
	});

	auto ClearCurrentWindow=[this]
	{
		this->m_currentWindow=nullptr;
		InterlockedExchange(&CurrentWindowType,-1);
	};
	if (ModalIndex!=-1)
	{
		m_nodalWindows.erase(m_nodalWindows.begin() + ModalIndex);

		if (m_currentWindow==Param)
		{
			if (!m_nodalWindows.empty())
			{
				ActivateCommit(m_nodalWindows.back());
			}
			else if (!m_windows.empty())
			{
				assert(m_windowPos < static_cast<int>(m_windows.size()));
				assert(m_windowPos>=0);
				ActivateCommit(m_windowPos);
			}
			else ClearCurrentWindow();
		}
	}
	else if (-1!=WindowIndex)
	{
		m_windows.erase(m_windows.begin() + WindowIndex);

		if (m_windowPos >= static_cast<int>(m_windows.size()))
		{
			m_windowPos = FilePanelsIndex;
			if (m_windowPos >= static_cast<int>(m_windows.size()))
			{
				m_windowPos = DesktopIndex;
			}
		}

		if (m_currentWindow==Param)
		{
			if (!m_windows.empty())
			{
				if (Param->m_previousWindow == Global->CtrlObject->Desktop || Param->m_previousWindow == Global->CtrlObject->Cp())
				{
					ActivateCommit(m_windowPos);
				}
				else
				{
					assert(Param->m_previousWindow);
					ActivateCommit(Param->m_previousWindow);
				}
			}
			else ClearCurrentWindow();
		}
		else
		{
			if (!m_windows.empty())
			{
				RefreshWindow(m_windowPos);
				RefreshWindow(m_currentWindow);
			}
		}
	}

	assert(m_currentWindow!=Param);

	if (Param->GetDynamicallyBorn())
	{
		_MANAGER(SysLog(L"delete DeletedWindow %p", Param));
		delete Param;
	}
	auto stop=m_Executed.find(Param);
	if (stop != m_Executed.end())
	{
		*(stop->second)=true;
		m_Executed.erase(stop);
	}
}

void Manager::ActivateCommit(window* Param)
{
	_MANAGER(CleverSysLog clv(L"Manager::ActivateCommit()"));
	_MANAGER(SysLog(L"ActivatedWindow=%p",Param));

	if (m_currentWindow==Param)
	{
		RefreshCommit(Param);
		return;
	}

	int WindowIndex=IndexOf(Param);

	if (-1!=WindowIndex)
	{
		m_windowPos=WindowIndex;
	}

	/* 14.05.2002 SKV
	  Если мы пытаемся активировать полумодальный фрэйм,
	  то надо его вытащит на верх стэка модалов.
	*/

	auto ItemIterator = std::find(ALL_RANGE(m_nodalWindows), Param);
	if (ItemIterator != m_nodalWindows.end())
	{
		std::swap(*ItemIterator, m_nodalWindows.back());
	}

	m_currentWindow=Param;
	InterlockedExchange(&CurrentWindowType,m_currentWindow->GetType());
	UpdateMacroArea();
	RefreshCommit(Param);
}

void Manager::ActivateCommit(int Index)
{
	ProcessWindowByPos(Index,&Manager::ActivateCommit);
}

void Manager::RefreshCommit(window* Param)
{
	_MANAGER(CleverSysLog clv(L"Manager::RefreshCommit()"));
	_MANAGER(SysLog(L"RefreshedWindow=%p",Param));

	if (!Param)
		return;

	if (IndexOf(Param)==-1 && IndexOfStack(Param)==-1)
		return;

	if (!Param->Locked())
	{
		if (!Global->IsRedrawWindowInProcess)
			Param->ShowConsoleTitle();

		Param->Refresh();
	}

	if
	(
		(Global->Opt->ViewerEditorClock && (Param->GetType() == windowtype_editor || Param->GetType() == windowtype_viewer))
		||
		(Global->WaitInMainLoop && Global->Opt->Clock)
	)
		ShowTime(1);
}

void Manager::DeactivateCommit(window* Param)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeactivateCommit()"));
	_MANAGER(SysLog(L"DeactivatedWindow=%p",Param));
	if (Param)
	{
		Param->OnChangeFocus(0);
	}
}

void Manager::ExecuteCommit(window* Param)
{
	_MANAGER(CleverSysLog clv(L"Manager::ExecuteCommit()"));
	_MANAGER(SysLog(L"ExecutedWindow=%p",Param));

	if (Param)
	{
		m_nodalWindows.emplace_back(Param);
		ActivateCommit(Param);
	}
}

void Manager::UpdateCommit(window* Old,window* New)
{
	int WindowIndex = IndexOf(Old);

	if (-1 != WindowIndex)
	{
		m_windows[WindowIndex] = New;
		New->m_previousWindow=m_currentWindow;
		ActivateCommit(New);
		DeleteWindow(Old);
	}
	else
	{
		_MANAGER(SysLog(L"ERROR! DeletedWindow not found"));
	}
}

/*$ 26.06.2001 SKV
  Для вызова из плагинов посредством ACTL_COMMIT
*/
void Manager::PluginCommit()
{
	Commit();
}

/* $ Введена для нужд CtrlAltShift OT */
void Manager::ImmediateHide()
{
	if (m_windowPos<0)
		return;

	// Сначала проверяем, есть ли у прятываемого окна SaveScreen
	if (m_currentWindow->HasSaveScreen())
	{
		m_currentWindow->Hide();
		return;
	}

	// Окна перерисовываются, значит для нижних
	// не выставляем заголовок консоли, чтобы не мелькал.
	if (!m_nodalWindows.empty())
	{
		/* $ 28.04.2002 KM
		    Проверим, а не модальный ли редактор или вьювер на вершине
		    модального стека? И если да, покажем User screen.
		*/
		if (m_nodalWindows.back()->GetType()==windowtype_editor || m_nodalWindows.back()->GetType()==windowtype_viewer)
		{
			ShowBackground();
		}
		else
		{
			int UnlockCount=0;
			Global->IsRedrawWindowInProcess++;

			while (GetWindow(m_windowPos)->Locked())
			{
				GetWindow(m_windowPos)->Unlock();
				UnlockCount++;
			}

			RefreshWindow(GetWindow(m_windowPos));
			Commit();

			for (int i=0; i<UnlockCount; i++)
			{
				GetWindow(m_windowPos)->Lock();
			}

			if (m_nodalWindows.size() > 1)
			{
				FOR(const auto& i, make_range(m_nodalWindows.cbegin(), m_nodalWindows.cend() - 1))
				{
					RefreshWindow(i);
					Commit();
				}
			}

			/* $ 04.04.2002 KM
			   Перерисуем заголовок только у активного окна.
			   Этим мы предотвращаем мелькание заголовка консоли
			   при перерисовке всех окон.
			*/
			Global->IsRedrawWindowInProcess--;
			m_currentWindow->ShowConsoleTitle();
		}
	}
	else
	{
		ShowBackground();
	}
}

void Manager::ResizeAllWindows()
{
	Global->ScrBuf->Lock();
	std::for_each(ALL_CONST_RANGE(m_windows), std::mem_fn(&window::ResizeConsole));
	std::for_each(ALL_CONST_RANGE(m_nodalWindows), std::mem_fn(&window::ResizeConsole));

	ImmediateHide();
	RefreshWindow();
	Global->ScrBuf->Unlock();
}

void Manager::InitKeyBar()
{
	std::for_each(ALL_CONST_RANGE(m_windows), std::mem_fn(&window::InitKeyBar));
}

void Manager::UpdateMacroArea(void)
{
	if (m_currentWindow) Global->CtrlObject->Macro.SetMode(m_currentWindow->GetMacroMode());
}
