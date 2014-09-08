#pragma once

/*
manager.hpp

Переключение между несколькими file panels, viewers, editors
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

class window;
class Viewer;
class FileEditor;

class Manager: NonCopyable
{
public:
	struct Key
	{
		INPUT_RECORD Event;
		int FarKey;
		bool EventFilled;
		Key(): Event(), FarKey(0), EventFilled(false) {}
		explicit Key(int Key): Event(), FarKey(Key), EventFilled(false) {}
		//Key(INPUT_RECORD Key): EventFilled(true), Event(Key) {FarKey=0; /*FIXME*/ }
	};

	class MessageAbstract;

public:
	Manager();
	~Manager();

	enum DirectionType {
		NoneWindow,
		PreviousWindow,
		NextWindow
	};
	// Эти функции можно безопасно вызывать практически из любого места кода
	// они как бы накапливают информацию о том, что нужно будет сделать с окнами при следующем вызове Commit()
	void InsertWindow(window *NewWindow);
	void DeleteWindow(window *Deleted = nullptr);
	void DeactivateWindow(window *Deactivated, DirectionType Direction);
	void ActivateWindow(window *Activated);
	void RefreshWindow(window *Refreshed = nullptr);
	void ReplaceWindow(window* Old,window* New);
	void CallbackWindow(const std::function<void(void)>& Callback);
	//! Функции для запуска модальных окон.
	void ExecuteWindow(window *Executed);
	//! Входит в новый цикл обработки событий
	void ExecuteModal(window *Executed);
	//! Запускает немодальное окно в модальном режиме
	void ExecuteNonModal(const window *NonModal);
	void CloseAll();
	/* $ 29.12.2000 IS
	Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
	если пользователь продолжил редактировать файл.
	Возвращает TRUE, если все закрыли и можно выходить из фара.
	*/
	BOOL ExitAll();
	size_t GetWindowCount()const { return m_windows.size(); }
	int  GetWindowCountByType(int Type);
	/*$ 26.06.2001 SKV
	Для вызова через ACTL_COMMIT
	*/
	void PluginCommit();
	int CountWindowsWithName(const string& Name, BOOL IgnoreCase = TRUE);
	bool IsPanelsActive(bool and_not_qview = false) const; // используется как признак Global->WaitInMainLoop
	window* FindWindowByFile(int ModalType, const string& FileName, const wchar_t *Dir = nullptr);
	void EnterMainLoop();
	void ProcessMainLoop();
	void ExitMainLoop(int Ask);
	int ProcessKey(Key key);
	int ProcessMouse(const MOUSE_EVENT_RECORD *me);
	void PluginsMenu() const; // вызываем меню по F11
	void SwitchToPanels();
	const INPUT_RECORD& GetLastInputRecord() const { return LastInputRecord; }
	void SetLastInputRecord(const INPUT_RECORD *Rec);
	void ResetLastInputRecord() { LastInputRecord.EventType = 0; }
	window *GetCurrentWindow() { return m_currentWindow; }
	window* GetWindow(size_t Index) const;
	window* GetSortedWindow(size_t Index) const;
	int IndexOf(window* Window);
	int SortedIndexOf(window* Window);
	int IndexOfStack(window* Window);
	window *GetBottomWindow() { return m_windows.back(); }
	bool ManagerIsDown() const { return EndLoop; }
	bool ManagerStarted() const { return StartManager; }
	void InitKeyBar();
	bool InModal(void) const { return !m_modalWindows.empty(); }
	void ResizeAllWindows();
	size_t GetModalWindowCount() const { return m_modalWindows.size(); }
	window* GetModalWindow(size_t index) const { return m_modalWindows[index]; }

	void AddGlobalKeyHandler(const std::function<int(const Key&)>& Handler);

	static long GetCurrentWindowType() { return CurrentWindowType; }
	static bool ShowBackground();

	void UpdateMacroArea(void);

	typedef std::set<window*,std::function<bool(window*,window*)>> sorted_windows;
	sorted_windows GetSortedWindows(void) const;

	Viewer* GetCurrentViewer(void) const;
	FileEditor* GetCurrentEditor(void) const;

private:
#if defined(SYSLOG)
	friend void ManagerClass_Dump(const wchar_t *Title, FILE *fp);
#endif

	window *WindowMenu(); //    вместо void SelectWindow(); // show window menu (F12)
	bool HaveAnyWindow() const;
	bool OnlyDesktop() const;
	void Commit(void);         // завершает транзакцию по изменениям в контейнерах окон
	// Она в цикле вызывает себя, пока хотябы один из указателей отличен от nullptr
	// Функции, "подмастерья начальника" - Commit'a
	// Иногда вызываются не только из него и из других мест
	void InsertCommit(window* Param);
	void DeleteCommit(window* Param);
	void ActivateCommit(window* Param);
	void RefreshCommit(window* Param);
	void DeactivateCommit(window* Param);
	void ExecuteCommit(window* Param);
	void ReplaceCommit(window* Old,window* New);
	int GetModalExitCode() const;
	// BUGBUG, do we need this?
	void ImmediateHide();

	typedef void(Manager::*window_callback)(window*);

	void PushWindow(window* Param, window_callback Callback);
	void CheckAndPushWindow(window* Param, window_callback Callback);
	void RedeleteWindow(window *Deleted);
	bool AddWindow(window *Param);

	INPUT_RECORD LastInputRecord;
	window *m_currentWindow;     // текущее окно. Оно может находиться как в немодальном, так и в модальном контейнере, его можно получить с помощью WindowManager->GetCurrentWindow();
	typedef std::vector<window*> windows;
	void* GetCurrent(std::function<void*(windows::const_reverse_iterator)> Check) const;
	windows m_modalWindows;
	windows m_windows;
	// текущее немодальное окно можно получить с помощью WindowManager->GetBottomWindow();
	/* $ 15.05.2002 SKV
		Так как есть полумодалы, что б не было путаницы,
		заведём счётчик модальных editor/viewer'ов.
		Дёргать его  надо ручками перед вызовом ExecuteModal.
		А автоматом нельзя, так как ExecuteModal вызывается
		1) не только для настоящих модалов (как это не пародоксально),
		2) не только для editor/viewer'ов.
	*/
	bool EndLoop;            // Признак выхода из цикла
	int ModalExitCode;
	bool StartManager;
	static long CurrentWindowType;
	std::list<std::unique_ptr<MessageAbstract>> m_Queue;
	std::vector<std::function<int(const Key&)>> m_GlobalKeyHandlers;
	std::unordered_map<window*,bool*> m_Executed;
	std::unordered_set<window*> m_Added;
};
