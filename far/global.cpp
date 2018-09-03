﻿/*
global.cpp

Глобальные переменные
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

#include "global.hpp"

#include "scrbuf.hpp"
#include "config.hpp"
#include "configdb.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"

global::global():
	OnlyEditorViewerUsed(),
	m_MainThreadId(GetCurrentThreadId()),
	m_MainThreadHandle(os::OpenCurrentThread()),
	m_SearchHex(),
	m_ConfigProvider(),
	Opt(std::make_unique<Options>()),
	ScrBuf(std::make_unique<ScreenBuf>()),
	WindowManager(std::make_unique<Manager>()),
	CtrlObject(nullptr)
{
	Global = this;

	// BUGBUG

	// идет процесс назначения клавиши в макросе?
	IsProcessAssignMacroKey = 0;
	PluginPanelsCount = 0;
	GlobalSearchCase=false;
	GlobalSearchWholeWords=false; // значение "Whole words" для поиска
	GlobalSearchReverse=false;
	ScreenSaverActive = false;
	CloseFAR = false;
	CloseFARMenu = false;
	AllowCancelExit = true;
	DisablePluginsOutput = false;
	ProcessException = false;
	HelpFileMask = L"*.hlf"sv;
#if defined(SYSLOG)
	StartSysLog = false;
#endif
#ifdef DIRECT_RT
	DirectRT = false;
#endif
	GlobalSaveScrPtr=nullptr;
	CriticalInternalError = false;
	KeepUserScreen = 0;
	Macro_DskShowPosType=0; // для какой панели вызывали меню выбора дисков (0 - ничерта не вызывали, 1 - левая (AltF1), 2 - правая (AltF2))
	ErrorMode = SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX;

	// BUGBUG end
}

global::~global()
{
	delete CtrlObject;
	CtrlObject = nullptr;
	delete m_ConfigProvider;
	m_ConfigProvider = nullptr;
	Global = nullptr;
}

void global::StoreSearchString(const string& Str, bool Hex)
{
	m_SearchHex = Hex;
	m_SearchString = Str;
}

bool global::IsPanelsActive() const
{
	return Global->WindowManager && Global->WindowManager->IsPanelsActive(true, true);
}

global::far_clock::far_clock()
{
	update();
}

const string& global::far_clock::get() const
{
	return m_CurrentTime;
}

size_t global::far_clock::size() const
{
	return m_CurrentTime.size();
}

void global::far_clock::update()
{
	m_CurrentTime = os::chrono::format_time();
}


#include "bootstrap/copyright.inc"
