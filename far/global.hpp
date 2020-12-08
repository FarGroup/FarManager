#ifndef GLOBAL_HPP_178AC66B_7DAB_4AD7_BBE7_D5D590BB9675
#define GLOBAL_HPP_178AC66B_7DAB_4AD7_BBE7_D5D590BB9675
#pragma once

/*
global.hpp

Описание глобальных переменных
Включать последним.
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

// Platform:
#include "platform.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

enum class lng;

class global: noncopyable
{
public:
	global();
	~global();

	HANDLE MainThreadHandle() const {return m_MainThreadHandle.native_handle();}
	bool IsMainThread() const {return GetCurrentThreadId() == m_MainThreadId;}
	std::chrono::steady_clock::duration FarUpTime() const;

	const string& GetSearchString() const { return m_SearchString; }
	bool GetSearchHex() const { return m_SearchHex; }
	void StoreSearchString(string_view Str, bool Hex);
	bool IsPanelsActive() const;

	// BUGBUG

	std::chrono::steady_clock::time_point StartIdleTime;
	string g_strFarModuleName;
	string g_strFarINI;
	string g_strFarPath;
	string strInitTitle;
	bool GlobalSearchCase{};
	bool GlobalSearchWholeWords{}; // значение "Whole words" для поиска
	bool GlobalSearchReverse{};
	std::atomic_ulong SuppressClock{};
	std::atomic_ulong SuppressIndicators{};
	bool CloseFAR{}, CloseFARMenu{}, AllowCancelExit{true};
	bool DisablePluginsOutput{};
	int IsProcessAssignMacroKey{};
	size_t PluginPanelsCount{};
	bool ProcessException{};

	class far_clock
	{
	public:
		NONCOPYABLE(far_clock);
		far_clock();
		const string& get() const;
		size_t size() const;
		void update();

	private:
		string m_CurrentTime;

	};

	far_clock CurrentTime;

	size_t LastShownTimeSize{};
	string_view HelpFileMask{L"*.hlf"sv};
	bool OnlyEditorViewerUsed{}; // -e or -v
#if defined(SYSLOG)
	bool StartSysLog{};
#endif
#ifdef DIRECT_RT
	bool DirectRT{};
#endif
	class SaveScreen *GlobalSaveScrPtr{};
	bool CriticalInternalError{};
	int Macro_DskShowPosType{}; // для какой панели вызывали меню выбора дисков (0 - ничерта не вызывали, 1 - левая (AltF1), 2 - правая (AltF2))
	DWORD ErrorMode;
#ifndef NO_WRAPPER
	string strRegUser;
#endif // NO_WRAPPER

	// BUGBUG end

private:
	DWORD m_MainThreadId;
	os::handle m_MainThreadHandle;
	std::chrono::steady_clock::time_point m_FarStartTime;

	string m_SearchString;
	bool m_SearchHex{};

public:
	// TODO: review the order and make private
	std::unique_ptr<class config_provider> m_ConfigProvider;
	std::unique_ptr<class Options> Opt;
	std::unique_ptr<class ScreenBuf> ScrBuf;
	std::unique_ptr<class Manager> WindowManager;

	// Non-owning
	class ControlObject* CtrlObject{};
};

extern global* Global;

#endif // GLOBAL_HPP_178AC66B_7DAB_4AD7_BBE7_D5D590BB9675
