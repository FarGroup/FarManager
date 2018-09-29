#ifndef TASKBAR_HPP_2522B9DF_D677_4AA9_8777_B5A1F588D4C1
#define TASKBAR_HPP_2522B9DF_D677_4AA9_8777_B5A1F588D4C1
#pragma once

/*
taskbar.hpp

Windows 7 taskbar support
*/
/*
Copyright © 2009 Far Group
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

#include "platform.hpp"

#include "common/singleton.hpp"

class taskbar: public singleton<taskbar>
{
	IMPLEMENTS_SINGLETON;

public:
	TBPFLAG GetProgressState() const;
	void SetProgressState(TBPFLAG tbpFlags);
	void SetProgressValue(unsigned long long Completed, unsigned long long Total);
	static void Flash();

private:
	taskbar();

	os::com::ptr<ITaskbarList3> m_TaskbarList;
	TBPFLAG m_State;
};

class IndeterminateTaskbar: noncopyable
{
public:
	explicit IndeterminateTaskbar(bool EndFlash = true);
	~IndeterminateTaskbar();

private:
	bool EndFlash;
};

template<TBPFLAG T>
class taskbar_state: noncopyable
{
public:
	taskbar_state():
		m_PreviousState(taskbar::instance().GetProgressState())
	{
		if (m_PreviousState != TBPF_ERROR && m_PreviousState != TBPF_PAUSED)
		{
			if (m_PreviousState == TBPF_INDETERMINATE || m_PreviousState == TBPF_NOPROGRESS)
			{
				taskbar::instance().SetProgressValue(1, 1);
			}
			taskbar::instance().SetProgressState(T);
			taskbar::instance().Flash();
		}
	}

	~taskbar_state()
	{
		taskbar::instance().SetProgressState(m_PreviousState);
	}

private:
	TBPFLAG m_PreviousState;
};

using TaskbarPause = taskbar_state<TBPF_PAUSED>;
using TaskbarError = taskbar_state<TBPF_ERROR>;

#endif // TASKBAR_HPP_2522B9DF_D677_4AA9_8777_B5A1F588D4C1
