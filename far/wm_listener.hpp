#ifndef WM_LISTENER_HPP_6C668719_5279_4CB7_81B0_448AC5165C00
#define WM_LISTENER_HPP_6C668719_5279_4CB7_81B0_448AC5165C00
#pragma once

/*
wm_listener.hpp

Обработка оконных сообщений
*/
/*
Copyright © 2010 Far Group
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
#include "platform.concurrency.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

class wm_listener: noncopyable
{
public:
	wm_listener();
	~wm_listener();

	void Check();
	void enable_power_notifications();
	void disable_power_notifications();

private:
	void WindowThreadRoutine(const os::event& ReadyEvent);

	HWND m_Hwnd{};
	std::exception_ptr m_ExceptionPtr;
	os::thread m_Thread;
	struct powernotify_deleter
	{
		void operator()(HPOWERNOTIFY Ptr) const;
	};
	std::unique_ptr<std::remove_pointer_t<HPOWERNOTIFY>, powernotify_deleter> m_PowerNotify;
};

#endif // WM_LISTENER_HPP_6C668719_5279_4CB7_81B0_448AC5165C00
