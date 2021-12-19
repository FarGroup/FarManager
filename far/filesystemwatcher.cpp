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
#include "exception.hpp"
#include "log.hpp"
#include "notification.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

FileSystemWatcher::~FileSystemWatcher()
{
	try
	{
		Release();
	}
	catch (...)
	{
		LOGERROR(L"Unknown exception"sv);
	}
}

void FileSystemWatcher::Set(string_view const EventId, string_view const Directory, bool const WatchSubtree)
{
	Release();

	m_EventId = EventId;
	m_Directory = NTPath(Directory);
	m_WatchSubtree = WatchSubtree;
}

void FileSystemWatcher::Watch()
{
	PropagateException();

	SCOPED_ACTION(elevation::suppress);

	if(!m_RegistrationThread)
		m_RegistrationThread = os::thread(os::thread::mode::join, &FileSystemWatcher::Register, this);
}

void FileSystemWatcher::Release()
{
	PropagateException();

	if (m_RegistrationThread)
	{
		m_Cancelled.set();
		m_RegistrationThread = {};
	}

	m_Cancelled.reset();
}

void FileSystemWatcher::Register()
{
	os::debug::set_thread_name(L"FS watcher");

	seh_try_thread(m_ExceptionPtr, [this]
	{
		cpp_try(
		[&]
		{
			LOGDEBUG(L"Start monitoring {}"sv, m_Directory);

			const auto DirectoryHandle = os::fs::create_file(
				m_Directory,
				FILE_LIST_DIRECTORY,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				{},
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED
			);

			if (!DirectoryHandle)
			{
				LOGWARNING(L"create_file({}): {}"sv, m_Directory, last_error());
				return;
			}

			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-readdirectorychangesw
			// If the buffer overflows, ReadDirectoryChangesW will still return true, but the entire contents
			// of the buffer are discarded and the BytesReturned parameter will be zero, which indicates
			// that your buffer was too small to hold all of the changes that occurred.

			// We don't care about individual changes, so the buffer is intentionally small.
			FILE_NOTIFY_INFORMATION Buffer;
			const os::event Event(os::event::type::automatic, os::event::state::nonsignaled);
			OVERLAPPED Overlapped{};
			Overlapped.hEvent = Event.native_handle();

			for (;;)
			{
				if (!ReadDirectoryChangesW(
					DirectoryHandle.native_handle(),
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
					&Overlapped,
					{}
				))
				{
					LOGWARNING(L"ReadDirectoryChangesW({}): {}"sv, m_Directory, last_error());
					return;
				}

				switch (os::handle::wait_any({ Event.native_handle(), m_Cancelled.native_handle() }))
				{
				case 0:
					if (DWORD BytesReturned = 0; !GetOverlappedResult(DirectoryHandle.native_handle(), &Overlapped, &BytesReturned, false))
					{
						const auto LastError = last_error();
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

					// FS changes can occur at a high rate.
					// We don't care about individual events, so we wait a little here to let them collapse to one event per sec at most:
					if (m_Cancelled.is_signaled(1s))
					{
						LOGDEBUG(L"Stop monitoring {}"sv, m_Directory);
						return;
					}

					LOGDEBUG(L"Continue monitoring {}"sv, m_Directory);
					break;

				case 1:
					LOGDEBUG(L"Stop monitoring {}"sv, m_Directory);
					return;
				}
			}
		},
		[&]
		{
			SAVE_EXCEPTION_TO(m_ExceptionPtr);
			m_IsRegularException = true;
		});
	});
}

void FileSystemWatcher::PropagateException() const
{
	if (m_ExceptionPtr && !m_IsRegularException)
	{
		// You're someone else's problem
		m_RegistrationThread.detach();
	}
	rethrow_if(m_ExceptionPtr);
}
