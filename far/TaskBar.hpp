#pragma once

/*
TaskBar.hpp

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

class TaskBarCore:NonCopyable
{
public:
	TaskBarCore();
	~TaskBarCore();
	TBPFLAG GetProgressState();
	void SetProgressState(TBPFLAG tbpFlags);
	void SetProgressValue(UINT64 Completed, UINT64 Total);
	void Flash();

private:
	bool CoInited;
	TBPFLAG State;
	ITaskbarList3* pTaskbarList;
};

extern TaskBarCore TBC;

class TaskBar
{
public:
	TaskBar(bool EndFlash=true);
	~TaskBar();

private:
	bool EndFlash;
};

template<TBPFLAG T>
class TaskBarState:NonCopyable
{
public:
	TaskBarState():PrevState(TBC.GetProgressState())
	{
		if (PrevState!=TBPF_ERROR && PrevState!=TBPF_PAUSED)
		{
			if (PrevState==TBPF_INDETERMINATE||PrevState==TBPF_NOPROGRESS)
			{
				TBC.SetProgressValue(1,1);
			}
			TBC.SetProgressState(T);
			TBC.Flash();
		}
	}

	~TaskBarState()
	{
		TBC.SetProgressState(PrevState);
	}

private:
	TBPFLAG PrevState;
};

typedef TaskBarState<TBPF_PAUSED> TaskBarPause;
typedef TaskBarState<TBPF_ERROR> TaskBarError;
