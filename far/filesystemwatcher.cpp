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
#include "flink.hpp"
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

	if (os::fs::GetFileTimeSimple(Directory, nullptr, nullptr, &m_PreviousLastWriteTime, nullptr))
		m_CurrentLastWriteTime = m_PreviousLastWriteTime;

	m_IsFatFilesystem.reset();
}

void FileSystemWatcher::Watch(bool got_focus, bool check_time)
{
	PropagateException();

	SCOPED_ACTION(elevation::suppress);

	if(!m_RegistrationThread)
		m_RegistrationThread = os::thread(os::thread::mode::join, &FileSystemWatcher::Register, this);

	if (got_focus)
	{
		if (!m_IsFatFilesystem.has_value())
		{
			m_IsFatFilesystem = false;

			const auto strRoot = GetPathRoot(m_Directory);
			if (!strRoot.empty())
			{
				string strFileSystem;
				if (os::fs::GetVolumeInformation(strRoot, nullptr, nullptr, nullptr, nullptr, &strFileSystem))
					m_IsFatFilesystem = starts_with(strFileSystem, L"FAT"sv);
			}
		}

		if (*m_IsFatFilesystem)
		{
			// emulate FAT folder time change
			// otherwise changes missed (FAT folder time is NOT modified)
			// the price is directory reload on each GOT_FOCUS event
			check_time = false;
			m_PreviousLastWriteTime = m_CurrentLastWriteTime - 1s;
		}
	}

	if (check_time)
	{
		if (!os::fs::GetFileTimeSimple(m_Directory, nullptr, nullptr, &m_CurrentLastWriteTime, nullptr))
		{
			m_PreviousLastWriteTime = {};
			m_CurrentLastWriteTime = {};
		}
	}
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
	m_PreviousLastWriteTime = m_CurrentLastWriteTime;
}

bool FileSystemWatcher::TimeChanged() const
{
	PropagateException();

	return m_PreviousLastWriteTime != m_CurrentLastWriteTime;
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
			const auto Notification = os::fs::find_first_change_notification(
				m_Directory,
				m_WatchSubtree,
					FILE_NOTIFY_CHANGE_FILE_NAME |
					FILE_NOTIFY_CHANGE_DIR_NAME |
					FILE_NOTIFY_CHANGE_ATTRIBUTES |
					FILE_NOTIFY_CHANGE_SIZE |
					FILE_NOTIFY_CHANGE_LAST_WRITE
			);

			if (!Notification)
			{
				LOGWARNING(L"find_first_change_notification({}): {}"sv, m_Directory, last_error());
				return;
			}

			for (;;)
			{
				try
				{
					switch (os::handle::wait_any({ Notification.native_handle(), m_Cancelled.native_handle() }))
					{
					case 0:
						LOGDEBUG(L"Change event in {}"sv, m_Directory);

						message_manager::instance().notify(m_EventId);

						// FS changes can occur at a high rate.
						// We don't want to DoS ourselves here, so notifications are throttled down to one per second at most:
						if (m_Cancelled.is_signaled(1s))
						{
							LOGDEBUG(L"Stop monitoring {}"sv, m_Directory);
							return;
						}

						// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findnextchangenotification
						// If a change occurs after a call to FindFirstChangeNotification
						// but before a call to FindNextChangeNotification, the operating system records the change.
						// When FindNextChangeNotification is executed, the recorded change
						// immediately satisfies a wait for the change notification.

						// In other words, even with throttled notifications we shouldn't miss anything.
						if (!os::fs::find_next_change_notification(Notification))
						{
							LOGWARNING(L"find_next_change_notification({}): {}"sv, m_Directory, last_error());
							return;
						}
						break;

					case 1:
						LOGDEBUG(L"Stop monitoring {}"sv, m_Directory);
						return;
					}
				}
				catch(far_fatal_exception const& e)
				{
					if (e.Win32Error == ERROR_INVALID_HANDLE)
					{
						// https://docs.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
						// Some functions use ERROR_INVALID_HANDLE to indicate that the object itself is no longer valid.
						// For example, a function that attempts to use a handle to a file on a network might fail
						// with ERROR_INVALID_HANDLE if the network connection is severed, because the file object
						// is no longer available. In this case, the application should close the handle.
						LOGWARNING(L"Wait for change in {} failed: {}"sv, m_Directory, e);
						return;
					}

					throw;
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
