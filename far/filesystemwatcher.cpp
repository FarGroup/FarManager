/*
filesystemwatcher.cpp

class FileSystemWatcher
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "filesystemwatcher.hpp"

// Internal:
#include "elevation.hpp"
#include "exception_handler.hpp"
#include "pathmix.hpp"
#include "log.hpp"
#include "notification.hpp"

// Platform:
#include "platform.hpp"
#include "platform.concurrency.hpp"
#include "platform.debug.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/scope_exit.hpp"
#include "common/singleton.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

class background_watcher: public singleton<background_watcher>
{
	IMPLEMENTS_SINGLETON;

public:
	void add(const FileSystemWatcher* Client)
	{
		SCOPED_ACTION(std::scoped_lock)(m_CS);

		m_Clients.emplace_back(Client);

		if (!m_Thread.joinable() || m_Thread.is_signaled())
			m_Thread = os::thread(&background_watcher::process, this);

		m_Update.set();
	}

	void remove(const FileSystemWatcher* Client)
	{
		{
			SCOPED_ACTION(std::scoped_lock)(m_CS);

			std::erase(m_Clients, Client);
		}

		m_UpdateDone.reset();
		m_Update.set();

		// We have to ensure that the client event handle is no longer used by the watcher before letting the client go
		(void)os::handle::wait_any({ m_UpdateDone.native_handle(), m_Thread.native_handle()});
	}

private:
	void process()
	{
		os::debug::set_thread_name(L"FS watcher");

		for (;;)
		{
			{
				m_UpdateDone.reset();
				SCOPE_EXIT{ m_UpdateDone.set(); };

				{
					SCOPED_ACTION(std::scoped_lock)(m_CS);

					if (m_Clients.empty())
					{
						LOGDEBUG(L"FS Watcher exit"sv);
						return;
					}

					m_Handles.resize(1);
					std::ranges::transform(m_Clients, std::back_inserter(m_Handles), [](const FileSystemWatcher* const Client) { return Client->m_Event.native_handle(); });
				}
			}

			const auto Result = os::handle::wait_any(m_Handles);

			if (Result == 0)
			{
				if (m_Exit)
					return;

				continue;
			}

			{
				SCOPED_ACTION(std::scoped_lock)(m_CS);

				m_Clients[Result - 1]->callback_notify();

				for (const auto& Client : m_Clients)
				{
					if (Client != m_Clients[Result - 1] && Client->m_Event.is_signaled())
						Client->callback_notify();
				}
			}

			// FS changes can occur at a high rate.
			// We don't care about individual events, so we wait a little here to let them collapse to one event per sec at most:
			(void)m_Update.is_signaled(1s);
		}
	}

	~background_watcher()
	{
		m_Exit = true;
		m_Update.set();
	}

	os::critical_section m_CS;
	os::event
		m_Update{ os::event::type::automatic, os::event::state::nonsignaled },
		m_UpdateDone{ os::event::type::automatic, os::event::state::nonsignaled };
	std::vector<const FileSystemWatcher*> m_Clients;
	std::vector<HANDLE> m_Handles{ m_Update.native_handle() };
	std::atomic_bool m_Exit{};
	os::thread m_Thread;
};

static os::handle open(const string_view Directory)
{
	SCOPED_ACTION(elevation::suppress);

	auto DirectoryHandle = os::fs::create_file(
		Directory,
		FILE_LIST_DIRECTORY,
		os::fs::file_share_all,
		{},
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED
	);

	if (!DirectoryHandle)
		LOGERROR(L"create_file({}): {}"sv, Directory, os::last_error());

	return DirectoryHandle;
}

FileSystemWatcher::FileSystemWatcher(const string_view EventId, const string_view Directory, const bool WatchSubtree):
	m_EventId(EventId),
	m_Directory(nt_path(Directory)),
	m_WatchSubtree(WatchSubtree),
	m_DirectoryHandle(open(m_Directory))
{
	if (!m_DirectoryHandle)
	{
		LOGWARNING(L"Skip monitoring of {}"sv, Directory);
		return;
	}

	m_Overlapped.hEvent = m_Event.native_handle();
	read_async();

	LOGDEBUG(L"Start monitoring {}"sv, m_Directory);
	background_watcher::instance().add(this);
}

FileSystemWatcher::~FileSystemWatcher()
{
	if (!m_DirectoryHandle)
		return;

	LOGDEBUG(L"Stop monitoring {}"sv, m_Directory);
	background_watcher::instance().remove(this);
}

void FileSystemWatcher::read_async() const
{
	if (!ReadDirectoryChangesW(
		m_DirectoryHandle.native_handle(),
		&Buffer,
		sizeof(Buffer),
		m_WatchSubtree,
		FILE_NOTIFY_CHANGE_FILE_NAME |
		FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_NOTIFY_CHANGE_ATTRIBUTES |
		FILE_NOTIFY_CHANGE_SIZE |
		FILE_NOTIFY_CHANGE_LAST_WRITE |
		FILE_NOTIFY_CHANGE_LAST_ACCESS |
		FILE_NOTIFY_CHANGE_CREATION |
		FILE_NOTIFY_CHANGE_SECURITY,
		{},
		&m_Overlapped,
		{}
	))
	{
		LOGERROR(L"ReadDirectoryChangesW({}): {}"sv, m_Directory, os::last_error());
	}
}

void FileSystemWatcher::callback_notify() const
{
	if (DWORD BytesReturned = 0; !GetOverlappedResult(m_DirectoryHandle.native_handle(), &m_Overlapped, &BytesReturned, false))
	{
		const auto LastError = os::last_error();
		if (!(LastError.Win32Error == ERROR_ACCESS_DENIED && LastError.NtError == STATUS_DELETE_PENDING))
		{
			LOGWARNING(L"GetOverlappedResult({}): {}"sv, m_Directory, LastError);
		}

		LOGDEBUG(L"Stop monitoring {}"sv, m_Directory);
		return;
	}

	LOGDEBUG(L"Change event in {}"sv, m_Directory);

	LOGDEBUG(L"Notifying the listener {}"sv, m_EventId);
	message_manager::instance().notify(m_EventId);

	LOGDEBUG(L"Continue monitoring {}"sv, m_Directory);
	read_async();
}
