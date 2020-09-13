#ifndef MANAGER_HPP_C3173B86_845B_4D8D_921F_803EA43A3C8A
#define MANAGER_HPP_C3173B86_845B_4D8D_921F_803EA43A3C8A
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

// Internal:
#include "windowsfwd.hpp"

// Platform:

// Common:
#include "common/function_ref.hpp"
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

class Viewer;

class Manager: noncopyable
{
public:
	class Key
	{
	public:
		Key(): m_Event(), m_FarKey(0), m_EventFilled(false) {}
		explicit Key(int Key);
		Key(unsigned int Key, const INPUT_RECORD& Event): m_Event(Event), m_FarKey(Key), m_EventFilled(true) {}
		const INPUT_RECORD& Event() const {return m_Event;}
		bool IsEvent() const {return m_EventFilled;}
		bool IsReal() const;
		Key& operator=(unsigned int Key);
		Key& operator&=(unsigned int Key);
		unsigned int operator()() const {return m_FarKey;}

	private:
		INPUT_RECORD m_Event;
		unsigned int m_FarKey;
		bool m_EventFilled;
		void Fill(unsigned int Key);
	};

	Manager();

	enum class direction
	{
		previous,
		next
	};

	void InitDesktop();

	// Эти функции можно безопасно вызывать практически из любого места кода
	// они как бы накапливают информацию о том, что нужно будет сделать с окнами при следующем вызове Commit()
	void InsertWindow(const window_ptr& Inserted);
	void DeleteWindow(const window_ptr& Deleted = nullptr);
	void ActivateWindow(const window_ptr& Activated);
	void RefreshWindow(const window_ptr& Refreshed = nullptr);
	void ReplaceWindow(const window_ptr& Old, const window_ptr& New);
	void ModalDesktopWindow();
	void UnModalDesktopWindow();
	void CallbackWindow(const std::function<void()>& Callback);
	//! Функции для запуска модальных окон.
	void ExecuteWindow(const window_ptr& Executed);
	//! Входит в новый цикл обработки событий
	void ExecuteModal(const window_ptr& Executed);
	//! Запускает немодальное окно в модальном режиме
	void ExecuteNonModal(const window_ptr& NonModal);
	void RefreshAll();
	void CloseAll();
	/* $ 29.12.2000 IS
	Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
	если пользователь продолжил редактировать файл.
	Возвращает TRUE, если все закрыли и можно выходить из фара.
	*/
	bool ExitAll();
	size_t GetWindowCount() const { return m_windows.size(); }
	int  GetWindowCountByType(int Type);
	/*
	This method can execute any far or plugins code. Never call from non-reentrant code.
	*/
	void PluginCommit();
	int CountWindowsWithName(string_view Name, bool IgnoreCase = true);
	bool IsPanelsActive(bool and_not_qview = false, bool or_autocomplete = false) const;
	window_ptr FindWindowByFile(int ModalType, string_view FileName);
	void EnterMainLoop();
	void ProcessMainLoop();
	void ExitMainLoop(int Ask);
	bool ProcessKey(Key key);
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) const;
	void PluginsMenu() const; // вызываем меню по F11
	void SwitchToPanels();
	window_ptr GetCurrentWindow() const { return m_windows.empty() ? nullptr : m_windows.back(); }
	window_ptr GetWindow(size_t Index) const;
	int IndexOf(const window_ptr& Window) const;
	window_ptr GetBottomWindow() { return m_NonModalSize ? m_windows[m_NonModalSize - 1] : nullptr; }
	bool ManagerIsDown() const { return EndLoop; }
	bool ManagerStarted() const { return StartManager; }
	void InitKeyBar() const;
	bool InModal() const { return m_NonModalSize < m_windows.size(); }
	bool IsModal(size_t Index) const { return Index >= m_NonModalSize; }
	void ResizeAllWindows();

	void AddGlobalKeyHandler(const std::function<bool(const Key&)>& Handler);

	static long GetCurrentWindowType() { return CurrentWindowType; }

	bool ShowBackground() const;
	void UpdateMacroArea() const;

	desktop* Desktop() const;
	Viewer* GetCurrentViewer() const;
	FileEditor* GetCurrentEditor() const;
	// BUGBUG, do we need this?
	void ImmediateHide();
	bool HaveAnyMessage() const;

private:
	struct window_comparer
	{
		bool operator()(const window_ptr& lhs, const window_ptr& rhs) const;
	};
	using sorted_windows = std::set<window_ptr, window_comparer>;
	sorted_windows GetSortedWindows() const;

#if defined(SYSLOG)
	friend void ManagerClass_Dump(const wchar_t *Title, FILE *fp);
#endif

	window_ptr WindowMenu(); //    вместо void SelectWindow(); // show window menu (F12)
	bool HaveAnyWindow() const;
	bool OnlyDesktop() const;
	void Commit();         // завершает транзакцию по изменениям в контейнерах окон
	// Она в цикле вызывает себя, пока хотябы один из указателей отличен от nullptr
	// Функции, "подмастерья начальника" - Commit'a
	// Иногда вызываются не только из него и из других мест
	void InsertCommit(const window_ptr& Param);
	void DeleteCommit(const window_ptr& Param);
	void ActivateCommit(const window_ptr& Param);
	void DoActivation(const window_ptr& Old, const window_ptr& New);
	void RefreshCommit(const window_ptr& Param);
	void DeactivateCommit(const window_ptr& Param);
	void ExecuteCommit(const window_ptr& Param);
	void ReplaceCommit(const window_ptr& Old, const window_ptr& New);
	void ModalDesktopCommit(const window_ptr& Param);
	void UnModalDesktopCommit(const window_ptr& Param);
	void RefreshAllCommit();
	int GetModalExitCode() const;

	using window_callback = void (Manager::*)(const window_ptr&);

	void PushWindow(const window_ptr& Param, window_callback Callback);
	void CheckAndPushWindow(const window_ptr& Param, window_callback Callback);
	void RedeleteWindow(const window_ptr& Deleted);
	bool AddWindow(const window_ptr& Param);
	void SwitchWindow(direction Direction);

	void WindowsChanged() { std::fill(m_windows_changed.begin(), m_windows_changed.end(), true); }

	using windows = std::vector<window_ptr>;
	void* GetCurrent(function_ref<void*(window_ptr const&)> Check) const;
	windows::const_iterator SpecialWindow();
	windows m_windows;
	size_t m_NonModalSize;
	bool EndLoop;            // Признак выхода из цикла
	int ModalExitCode;
	bool StartManager;
	int m_DesktopModalled;
	static inline std::atomic_long CurrentWindowType{-1};
	std::queue<std::function<void()>> m_Queue;
	std::vector<std::function<bool(const Key&)>> m_GlobalKeyHandlers;
	std::unordered_map<window_ptr, bool*> m_Executed;
	std::unordered_set<window_ptr> m_Added;
	desktop_ptr m_Desktop;
	std::vector<bool> m_windows_changed;
};

#endif // MANAGER_HPP_C3173B86_845B_4D8D_921F_803EA43A3C8A
