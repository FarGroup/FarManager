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
#include "fileedit.hpp"

long Manager::CurrentWindowType=-1;

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
	MessageOneWindow(window_ptr Param, const std::function<void(window_ptr)>& Callback): m_Param(Param), m_Callback(Callback) {}
	virtual bool Process(void) override { m_Callback(m_Param); return true; }

private:
	window_ptr m_Param;
	std::function<void(window_ptr)> m_Callback;
};

class MessageTwoWindows: public Manager::MessageAbstract
{
public:
	MessageTwoWindows(window_ptr Param1, window_ptr Param2, const std::function<void(window_ptr, window_ptr)>& Callback): m_Param1(Param1), m_Param2(Param2), m_Callback(Callback) {}
	virtual bool Process(void) override { m_Callback(m_Param1, m_Param2); return true; }

private:
	window_ptr m_Param1;
	window_ptr m_Param2;
	std::function<void(window_ptr, window_ptr)> m_Callback;
};

class MessageStop: public Manager::MessageAbstract
{
public:
	MessageStop() {}
	virtual bool Process(void) override { return false; }
};

Manager::Manager():
	m_currentWindow(nullptr),
	EndLoop(false),
	ModalExitCode(-1),
	StartManager(false)
{
	m_windows.reserve(1024);
	m_modalWindows.reserve(1024);
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
	for(size_t i = m_modalWindows.size(); i; --i)
	{
		if (i - 1 >= m_modalWindows.size())
			continue;
		auto CurrentWindow = m_modalWindows[i - 1];
		if (!CurrentWindow->GetCanLoseFocus(TRUE))
		{
			auto PrevWindowCount = m_modalWindows.size();
			CurrentWindow->ProcessKey(Manager::Key(KEY_ESC));
			Commit();

			if (PrevWindowCount == m_modalWindows.size())
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
	while(!m_modalWindows.empty())
	{
		DeleteWindow(m_modalWindows.back());
		Commit();
	}
	while(!m_windows.empty())
	{
		DeleteWindow(m_windows.back());
		Commit();
	}
	m_windows.clear();
}

void Manager::PushWindow(window_ptr Param, window_callback Callback)
{
	m_Queue.push_back(std::make_unique<MessageOneWindow>(Param,[this,Callback](window_ptr Param){(this->*Callback)(Param);}));
}

void Manager::CheckAndPushWindow(window_ptr Param, window_callback Callback)
{
	//assert(Param);
	if (Param&&!Param->IsDeleting()) PushWindow(Param,Callback);
}

void Manager::CallbackWindow(const std::function<void(void)>& Callback)
{
	m_Queue.push_back(std::make_unique<MessageCallback>(Callback));
}

void Manager::InsertWindow(window_ptr Inserted)
{
	_MANAGER(CleverSysLog clv(L"Manager::InsertWindow(window *Inserted, int Index)"));
	_MANAGER(SysLog(L"Inserted=%p, Index=%i",Inserted, Index));

	CheckAndPushWindow(Inserted,&Manager::InsertCommit);
}

void Manager::DeleteWindow(window_ptr Deleted)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeleteWindow(window *Deleted)"));
	_MANAGER(SysLog(L"Deleted=%p",Deleted));

	window_ptr Window=Deleted?Deleted:m_currentWindow;
	assert(Window);
	CheckAndPushWindow(Window,&Manager::DeleteCommit);
	Window->SetDeleting();
}

void Manager::RedeleteWindow(window_ptr Deleted)
{
	m_Queue.push_back(std::make_unique<MessageStop>());
	PushWindow(Deleted,&Manager::DeleteCommit);
}

void Manager::ExecuteNonModal(window_ptr NonModal)
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

void Manager::ExecuteModal(window_ptr Executed)
{
	_MANAGER(CleverSysLog clv(L"Manager::ExecuteModal (window *Executed)"));
	_MANAGER(SysLog(L"Executed=%p",Executed));

	bool stop=false;
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
window_ptr Manager::WindowMenu()
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
		auto ModalMenu = VMenu2::create(MSG(MScreensTitle), nullptr, 0, ScrY - 4);
		ModalMenu->SetHelp(L"ScrSwitch");
		ModalMenu->SetFlags(VMENU_WRAPMODE);
		ModalMenu->SetPosition(-1,-1,0,0);
		ModalMenu->SetId(ScreensSwitchId);

		size_t n = 0;
		auto windows=GetSortedWindows();
		std::for_each(CONST_RANGE(windows, i)
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
			auto tmp = i.get();
			ModalMenuItem.UserData = &tmp;
			ModalMenuItem.UserDataSize = sizeof(tmp);
			ModalMenuItem.SetSelect(i==m_windows.back());
			ModalMenu->AddItem(ModalMenuItem);
			++n;
		});

		AlreadyShown=TRUE;
		ExitCode=ModalMenu->Run();
		AlreadyShown=FALSE;

		if (CheckCanLoseFocus)
		{
			if (ExitCode>=0)
			{
				window* ActivatedWindow;
				ModalMenu->GetUserData(&ActivatedWindow,sizeof(ActivatedWindow),ExitCode);
				ActivateWindow(ActivatedWindow->shared_from_this());
				return (ActivatedWindow == m_currentWindow.get() || !m_currentWindow->GetCanLoseFocus())? nullptr : m_currentWindow;
			}
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
window_ptr Manager::FindWindowByFile(int ModalType,const string& FileName, const wchar_t *Dir)
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

void Manager::ActivateWindow(window_ptr Activated)
{
	_MANAGER(CleverSysLog clv(L"Manager::ActivateWindow(window *Activated)"));
	_MANAGER(SysLog(L"Activated=%i",Activated));

	if (Activated) CheckAndPushWindow(Activated,&Manager::ActivateCommit);
}

void Manager::DeactivateWindow(window_ptr Deactivated, DirectionType Direction)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeactivateWindow(window *Deactivated,int Direction)"));
	_MANAGER(SysLog(L"Deactivated=%p, Direction=%d",Deactivated,Direction));

	if (Direction==NextWindow || Direction == PreviousWindow)
	{
		auto windows = GetSortedWindows();
		auto pos = windows.find(m_windows.back());
		auto process = [&,this]()
		{
			if (Direction==Manager::NextWindow)
			{
				std::advance(pos,1);
				if (pos==windows.end()) pos = windows.begin();
			}
			else if (Direction==Manager::PreviousWindow)
			{
				if (pos==windows.begin()) pos=windows.end();
				std::advance(pos,-1);
			}
		};
		process();
		// For now we don't want to switch to the desktop window with Ctrl-[Shift-]Tab
		if (std::dynamic_pointer_cast<desktop>(*pos))
			process();
		ActivateWindow(*pos);
	}

	CheckAndPushWindow(Deactivated,&Manager::DeactivateCommit);
}

void Manager::RefreshWindow(window_ptr Refreshed)
{
	_MANAGER(CleverSysLog clv(L"Manager::RefreshWindow(window *Refreshed)"));
	_MANAGER(SysLog(L"Refreshed=%p",Refreshed));

	CheckAndPushWindow(Refreshed?Refreshed:m_currentWindow,&Manager::RefreshCommit);
}

void Manager::ExecuteWindow(window_ptr Executed)
{
	_MANAGER(CleverSysLog clv(L"Manager::ExecuteWindow(window *Executed)"));
	_MANAGER(SysLog(L"Executed=%p",Executed));
	CheckAndPushWindow(Executed,&Manager::ExecuteCommit);
}

void Manager::ReplaceWindow(window_ptr Old, window_ptr New)
{
	m_Queue.push_back(std::make_unique<MessageTwoWindows>(Old,New,[this](window_ptr Param1,window_ptr Param2){this->ReplaceCommit(Param1,Param2);}));
}

void Manager::SwitchToPanels()
{
	_MANAGER(CleverSysLog clv(L"Manager::SwitchToPanels()"));
	if (!Global->OnlyEditorViewerUsed)
	{
		std::find_if(CONST_RANGE(m_windows, i) -> bool
		{
			if (std::dynamic_pointer_cast<FilePanels>(i))
			{
				ActivateWindow(i);
				return true;
			}
			return false;
		});
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

	if (!Ask || !Global->Opt->Confirm.Exit || !Message(0, MSG(MQuit),
		make_vector<string>(MSG(MAskQuit)),
		make_vector<string>(MSG(MYes), MSG(MNo)),
		nullptr, nullptr, &FarAskQuitId))
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

void Manager::AddGlobalKeyHandler(const std::function<int(const Key&)>& Handler)
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
					if (!std::dynamic_pointer_cast<Modal>(Global->WindowManager->GetCurrentWindow()))
					{
						DeactivateWindow(WindowMenu(),NoneWindow);
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
				}
				case KEY_CTRLTAB:
				case KEY_RCTRLTAB:
				case KEY_CTRLSHIFTTAB:
				case KEY_RCTRLSHIFTTAB:

					if (m_currentWindow->GetCanLoseFocus())
					{
						DeactivateWindow(m_currentWindow,(key.FarKey==KEY_CTRLTAB||key.FarKey==KEY_RCTRLTAB)?NextWindow:PreviousWindow);
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
	if (!m_windows.empty() && m_currentWindow)
	{
		auto fp = std::dynamic_pointer_cast<FilePanels>(m_currentWindow);
		return fp && (!and_not_qview || fp->ActivePanel()->GetType() != QVIEW_PANEL);
	}
	else
	{
		return false;
	}
}

window_ptr Manager::GetWindow(size_t Index) const
{
	if (Index >= m_windows.size() || m_windows.empty())
	{
		return nullptr;
	}

	return m_windows[Index];
}

window_ptr Manager::GetSortedWindow(size_t Index) const
{
	if (Index >= m_windows.size() || m_windows.empty())
	{
		return nullptr;
	}

	auto windows = GetSortedWindows();
	auto pos = windows.begin();
	std::advance(pos,Index);
	return *pos;
}

int Manager::IndexOfStack(window_ptr Window)
{
	auto ItemIterator = std::find(ALL_CONST_RANGE(m_modalWindows), Window);
	return ItemIterator != m_modalWindows.cend()? ItemIterator - m_modalWindows.cbegin() : -1;
}

int Manager::IndexOf(window_ptr Window)
{
	auto ItemIterator = std::find(ALL_CONST_RANGE(m_windows), Window);
	return ItemIterator != m_windows.cend() ? ItemIterator - m_windows.cbegin() : -1;
}

int Manager::SortedIndexOf(window_ptr Window)
{
	auto windows = GetSortedWindows();
	auto ItemIterator = std::find(ALL_CONST_RANGE(windows), Window);
	return ItemIterator != windows.cend() ? std::distance(windows.cbegin(), ItemIterator) : -1;
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

void Manager::InsertCommit(window_ptr Param)
{
	_MANAGER(CleverSysLog clv(L"Manager::InsertCommit()"));
	_MANAGER(SysLog(L"InsertedWindow=%p",Param));
	if (Param && AddWindow(Param))
	{
		m_windows.emplace_back(Param);
		ActivateCommit(Param);
	}
}

void Manager::DeleteCommit(window_ptr Param)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeleteCommit()"));
	_MANAGER(SysLog(L"DeletedWindow=%p",Param));

	if (!Param)
		return;

	if (Param->IsPinned())
	{
		RedeleteWindow(Param);
		return;
	}

	Param->OnDestroy();

	int ModalIndex=IndexOfStack(Param);
	int WindowIndex=IndexOf(Param);
	assert(!(-1!=ModalIndex&&-1!=WindowIndex));

	auto ClearCurrentWindow=[this]
	{
		this->m_currentWindow=nullptr;
		InterlockedExchange(&CurrentWindowType,-1);
	};
	if (ModalIndex!=-1)
	{
		m_modalWindows.erase(m_modalWindows.begin() + ModalIndex);

		if (m_currentWindow==Param)
		{
			if (!m_modalWindows.empty())
			{
				ActivateCommit(m_modalWindows.back());
			}
			else if (!m_windows.empty())
			{
				ActivateCommit(m_windows.back());
			}
			else ClearCurrentWindow();
		}
	}
	else if (-1!=WindowIndex)
	{
		m_windows.erase(m_windows.begin() + WindowIndex);

		if (m_currentWindow==Param)
		{
			if (!m_windows.empty())
			{
				ActivateCommit(m_windows.back());
			}
			else ClearCurrentWindow();
		}
		else
		{
			if (!m_windows.empty())
			{
				RefreshWindow(m_windows.back());
			}
		}
	}

	assert(m_currentWindow!=Param);

	auto stop=m_Executed.find(Param);
	if (stop != m_Executed.end())
	{
		*(stop->second)=true;
		m_Executed.erase(stop);
	}
	auto size=m_Added.erase(Param);
	(void)size;
	assert(size==1);
}

void Manager::ActivateCommit(window_ptr Param)
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
		m_windows.erase(m_windows.begin() + WindowIndex);
		m_windows.push_back(Param);
	}

	/* 14.05.2002 SKV
	  Если мы пытаемся активировать полумодальный фрэйм,
	  то надо его вытащит на верх стэка модалов.
	*/

	auto ItemIterator = std::find(ALL_RANGE(m_modalWindows), Param);
	if (ItemIterator != m_modalWindows.end())
	{
		m_modalWindows.erase(ItemIterator);
		m_modalWindows.push_back(Param);
	}

	m_currentWindow=Param;
	InterlockedExchange(&CurrentWindowType,m_currentWindow->GetType());
	UpdateMacroArea();
	RefreshCommit(Param);
}

void Manager::RefreshCommit(window_ptr Param)
{
	_MANAGER(CleverSysLog clv(L"Manager::RefreshCommit()"));
	_MANAGER(SysLog(L"RefreshedWindow=%p",Param));

	if (!Param)
		return;

	auto WindowIndex=IndexOf(Param);
	auto ModalIndex=IndexOfStack(Param);

	if (-1==WindowIndex && -1==ModalIndex)
		return;

	assert(!(-1!=WindowIndex && -1!=ModalIndex));

	auto process=[](const windows& List, int Index) -> void
	{
		std::for_each(std::next(List.begin(), Index), List.end(), LAMBDA_PREDICATE(List, i)
		{
			if (!i->Locked()) i->Refresh();
			if
			(
				(Global->Opt->ViewerEditorClock && (i->GetType() == windowtype_editor || i->GetType() == windowtype_viewer))
				||
				(Global->WaitInMainLoop && Global->Opt->Clock)
			)
				ShowTime(1);
		});
	};

	if (WindowIndex >= 0)
	{
		process(m_windows, WindowIndex);
		ModalIndex = 0;
	}

	if (ModalIndex >= 0)
	{
		process(m_modalWindows, ModalIndex);
	}

	assert(m_currentWindow);
	if (!m_currentWindow->Locked())
	{
		if (!Global->IsRedrawWindowInProcess) m_currentWindow->ShowConsoleTitle();
	}
}

void Manager::DeactivateCommit(window_ptr Param)
{
	_MANAGER(CleverSysLog clv(L"Manager::DeactivateCommit()"));
	_MANAGER(SysLog(L"DeactivatedWindow=%p",Param));
	if (Param)
	{
		Param->OnChangeFocus(0);
	}
}

void Manager::ExecuteCommit(window_ptr Param)
{
	_MANAGER(CleverSysLog clv(L"Manager::ExecuteCommit()"));
	_MANAGER(SysLog(L"ExecutedWindow=%p",Param));

	if (Param && AddWindow(Param))
	{
		m_modalWindows.emplace_back(Param);
		ActivateCommit(Param);
	}
}

void Manager::ReplaceCommit(window_ptr Old,window_ptr New)
{
	int WindowIndex = IndexOf(Old);

	if (-1 != WindowIndex)
	{
		New->SetID(Old->ID());
		DeleteCommit(Old);
		InsertCommit(New);
	}
	else
	{
		_MANAGER(SysLog(L"ERROR! DeletedWindow not found"));
	}
}

bool Manager::AddWindow(window_ptr Param)
{
	return m_Added.insert(Param).second;
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
	if (m_windows.empty())
		return;

	// Сначала проверяем, есть ли у прятываемого окна SaveScreen
	if (m_currentWindow->HasSaveScreen())
	{
		m_currentWindow->Hide();
		return;
	}

	// Окна перерисовываются, значит для нижних
	// не выставляем заголовок консоли, чтобы не мелькал.
	if (!m_modalWindows.empty())
	{
		/* $ 28.04.2002 KM
		    Проверим, а не модальный ли редактор или вьювер на вершине
		    модального стека? И если да, покажем User screen.
		*/
		if (m_modalWindows.back()->GetType()==windowtype_editor || m_modalWindows.back()->GetType()==windowtype_viewer)
		{
			ShowBackground();
		}
		else
		{
			int UnlockCount=0;
			Global->IsRedrawWindowInProcess++;

			while (m_windows.back()->Locked())
			{
				m_windows.back()->Unlock();
				UnlockCount++;
			}

			RefreshWindow(m_windows.back());
			Commit();

			for (int i=0; i<UnlockCount; i++)
			{
				m_windows.back()->Lock();
			}

			if (m_modalWindows.size() > 1)
			{
				FOR(const auto& i, make_range(m_modalWindows.cbegin(), m_modalWindows.cend() - 1))
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
	std::for_each(ALL_CONST_RANGE(m_modalWindows), std::mem_fn(&window::ResizeConsole));

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

Manager::sorted_windows Manager::GetSortedWindows(void) const
{
	return sorted_windows(ALL_CONST_RANGE(m_windows),[](window_ptr lhs,window_ptr rhs){return lhs->ID()<rhs->ID();});
}

void* Manager::GetCurrent(std::function<void*(windows::const_reverse_iterator)> Check) const
{
	auto process=[this,&Check](const windows& List,void*& Result) -> bool
	{
		auto iterator = std::find_if(CONST_REVERSE_RANGE(List, i) { return !std::dynamic_pointer_cast<Modal>(i); });
		if (iterator!=List.crend())
		{
			Result=Check(iterator);
			return true;
		}
		return false;
	};
	void* result=nullptr;
	if (!process(m_modalWindows,result)) process(m_windows,result);
	return result;
}

Viewer* Manager::GetCurrentViewer(void) const
{
	return reinterpret_cast<Viewer*>(GetCurrent([](windows::const_reverse_iterator Iterator)->void*
	{
		auto result = std::dynamic_pointer_cast<ViewerContainer>(*Iterator);
		return result?result->GetViewer():nullptr;
	}
	));
}

FileEditor* Manager::GetCurrentEditor(void) const
{
	return reinterpret_cast<FileEditor*>(GetCurrent([](windows::const_reverse_iterator Iterator)
	{
		return std::dynamic_pointer_cast<FileEditor>(*Iterator).get();
	}
	));
}

template<typename window_type, typename windows_type>
static window_ptr GetContainerById(const windows_type& NonModal, const windows_type& Modal, int ID)
{
	const windows_type* Lists[] =
	{
		&NonModal,
		&Modal
	};
	FOR(const auto& List, Lists)
	{
		auto ItemIterator = std::find_if(CONST_RANGE(*List, i) -> bool
		{
			auto window = std::dynamic_pointer_cast<window_type>(i);
			if (window && window->GetById(ID)) return true;
			return false;
		});
		if (ItemIterator != List->cend()) return *ItemIterator;
	}
	return nullptr;
}

window_ptr Manager::GetViewerContainerById(int ID) const
{
	return GetContainerById<ViewerContainer>(m_windows, m_modalWindows, ID);
}

window_ptr Manager::GetEditorContainerById(int ID) const
{
	return GetContainerById<FileEditor>(m_windows, m_modalWindows, ID);
}
