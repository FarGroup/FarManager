#ifndef FILESYSTEMWATCHER_HPP_A4DC2834_A694_4E86_B8BA_FDA8DBF728CD
#define FILESYSTEMWATCHER_HPP_A4DC2834_A694_4E86_B8BA_FDA8DBF728CD
#pragma once

/*
filesystemwatcher.hpp

Класс FileSystemWatcher
*/
/*
Copyright © 2012 Far Group
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
#include "platform.chrono.hpp"
#include "platform.concurrency.hpp"
#include "platform.fs.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

class FileSystemWatcher: noncopyable
{
public:
	FileSystemWatcher();
	~FileSystemWatcher();
	void Set(string_view Directory, bool WatchSubtree);
	void Watch(bool got_focus=false, bool check_time=true);
	void Release();
	bool Signaled() const;

private:
	void Register();
	void PropagateException() const;

	string m_Directory;
	os::chrono::time_point m_PreviousLastWriteTime;
	os::chrono::time_point m_CurrentLastWriteTime;
	bool m_WatchSubtree;
	mutable os::thread m_RegistrationThread;
	os::fs::find_notification_handle m_Notification;
	os::event m_Cancelled;
	std::optional<bool> m_IsFatFilesystem;
	mutable std::exception_ptr m_ExceptionPtr;
	bool m_IsRegularException{};
};

#endif // FILESYSTEMWATCHER_HPP_A4DC2834_A694_4E86_B8BA_FDA8DBF728CD
