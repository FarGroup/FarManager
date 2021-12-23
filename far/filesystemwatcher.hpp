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
#include "platform.concurrency.hpp"

// Common:
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

class FileSystemWatcher
{
public:
	NONCOPYABLE(FileSystemWatcher);

	FileSystemWatcher(string_view EventId, string_view Directory, bool WatchSubtree);
	~FileSystemWatcher();

private:
	friend class background_watcher;

	void read_async() const;
	void callback_notify() const;

	string m_EventId;
	string m_Directory;
	bool m_WatchSubtree{};
	os::handle m_DirectoryHandle;
	os::event m_Event{ os::event::type::automatic, os::event::state::nonsignaled };
	// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-readdirectorychangesw
	// If the buffer overflows, ReadDirectoryChangesW will still return true, but the entire contents
	// of the buffer are discarded and the BytesReturned parameter will be zero, which indicates
	// that your buffer was too small to hold all of the changes that occurred.
	// We don't care about individual changes, so the buffer is intentionally small.
	mutable FILE_NOTIFY_INFORMATION Buffer;
	mutable OVERLAPPED m_Overlapped{};
};

#endif // FILESYSTEMWATCHER_HPP_A4DC2834_A694_4E86_B8BA_FDA8DBF728CD
