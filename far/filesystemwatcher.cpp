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
	void add(FileSystemWatcher* Client)
	{
		{
			SCOPED_ACTION(std::scoped_lock)(m_CS);

			m_Clients.emplace_back(Client);

			m_Synchronised.reset();
			m_Update.set();

			if (!m_Thread.joinable() || m_Thread.is_signaled())
			{
				m_Thread = os::thread(&background_watcher::process, this);
			}
		}

		m_Synchronised.wait();
	}

	void remove(const FileSystemWatcher* Client)
	{
		{
			SCOPED_ACTION(std::scoped_lock)(m_CS);

			std::erase(m_Clients, Client);

			m_Synchronised.reset();
			m_Update.set();
		}

		// We have to ensure that the client event handle is no longer used by the watcher before letting the client go
		m_Synchronised.wait();
	}

private:
	void process()
	{
		os::debug::set_thread_name(L"FS watcher");

		std::vector Handles{ m_Update.native_handle() };
		HANDLE PendingHandle{};

		for (;;)
		{
			{
				SCOPED_ACTION(std::scoped_lock)(m_CS);

				if (!m_Synchronised.is_signaled())
				{
					SCOPE_EXIT{ m_Synchronised.set(); };

					auto PendingHandleCopy = std::exchange(PendingHandle, {});
					Handles.resize(1);
					std::ranges::transform(m_Clients, std::back_inserter(Handles), [&](FileSystemWatcher* const Client)
					{
						const auto Handle = Client->m_Event.native_handle();
						if (Handle == PendingHandleCopy)
						{
							Client->callback_notify();
						}
						return Handle;
					});
				}
			}

			const auto Result = os::handle::wait_any(Handles);

			if (Result == 0)
			{
				if (m_Exit)
				{
					LOGDEBUG(L"FS Watcher exit"sv);
					return;
				}

				continue;
			}

			{
				SCOPED_ACTION(std::scoped_lock)(m_CS);

				if (!m_Synchronised.is_signaled())
				{
					PendingHandle = Handles[Result];
					continue;
				}

				m_Clients[Result - 1]->callback_notify();

				for (auto& Client: m_Clients)
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
	os::event m_Update{ os::event::type::automatic, os::event::state::nonsignaled };
	os::event m_Synchronised{ os::event::type::manual, os::event::state::nonsignaled };
	std::vector<FileSystemWatcher*> m_Clients;
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

	background_watcher::instance().add(this);

	LOGDEBUG(L"Start monitoring {} {}"sv, m_Directory, WatchSubtree? L"tree"sv : L"directory"sv);
	read_async();
}

FileSystemWatcher::~FileSystemWatcher()
{
	background_watcher::instance().remove(this);

	{
		SCOPED_ACTION(std::scoped_lock)(m_CS);

		if (!m_DirectoryHandle)
			return;

		LOGDEBUG(L"Stop monitoring {}"sv, m_Directory);

		m_DirectoryHandle = {};

		switch (const auto Status = m_Overlapped.Internal)
		{
		case STATUS_NOTIFY_CLEANUP:
		case STATUS_NOTIFY_ENUM_DIR:
			break;

		case STATUS_PENDING:
			(void)get_result();
			break;

		default:
			LOGDEBUG(L"Overlapped.Internal: {}"sv, os::error_state{ ERROR_SUCCESS, static_cast<NTSTATUS>(m_Overlapped.Internal) });
			break;
		}
	}
}

void FileSystemWatcher::read_async()
{
	m_Event.reset();

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
		m_DirectoryHandle = {};
	}
}

bool FileSystemWatcher::get_result() const
{
	if (DWORD BytesReturned; GetOverlappedResult(m_DirectoryHandle.native_handle(), &m_Overlapped, &BytesReturned, true))
		return true;

	switch (const auto LastError = os::last_error(); LastError.Win32Error)
	{
	case ERROR_OPERATION_ABORTED:
		return false; // BAU, no need to make noise

	case ERROR_ACCESS_DENIED:
		if (LastError.NtError == STATUS_DELETE_PENDING)
			return false; // BAU, no need to make noise

		[[fallthrough]];

	default:
		// Something exotic, better to report
		LOGWARNING(L"GetOverlappedResult({}): {}"sv, m_Directory, LastError);
		return false;
	}
}

void FileSystemWatcher::callback_notify()
{
	if (!m_DirectoryHandle)
		return;

	if (!get_result())
	{
		LOGDEBUG(L"Stop monitoring {}"sv, m_Directory);
		return;
	}

	LOGDEBUG(L"Change event in {}"sv, m_Directory);

	LOGDEBUG(L"Notifying the listener {}"sv, m_EventId);
	message_manager::instance().notify(m_EventId);

	LOGDEBUG(L"Continue monitoring {}"sv, m_Directory);
	read_async();
}
