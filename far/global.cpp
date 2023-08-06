/*
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "global.hpp"

// Internal:
#include "scrbuf.hpp"
#include "config.hpp"
#include "configdb.hpp"
#include "manager.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

global::global() :
	g_strFarModuleName(os::fs::get_current_process_file_name()),
	m_MainThreadId(GetCurrentThreadId()),
	m_MainThreadHandle(os::OpenCurrentThread()),
	m_FarStartTime(std::chrono::steady_clock::now()),
	Opt(std::make_unique<Options>()),
	ScrBuf(std::make_unique<ScreenBuf>()),
	WindowManager(std::make_unique<Manager>())
{
	Global = this;
}

global::~global()
{
	Global = nullptr;
}

std::chrono::steady_clock::duration global::FarUpTime() const
{
	return std::chrono::steady_clock::now() - m_FarStartTime;
}

void global::FolderChanged()
{
	WindowManager->FolderChanged();
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

void global::far_clock::update(bool const Force)
{
	const auto Now = os::chrono::nt_clock::now();
	const auto Value = Now.time_since_epoch() / 1min;

	if (!Force && Value == m_LastValue)
		return;

	m_CurrentTime = os::chrono::wall_time(Now);
	m_LastValue = Value;
}
