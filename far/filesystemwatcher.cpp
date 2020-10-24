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

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

FileSystemWatcher::FileSystemWatcher():
	m_WatchSubtree(false),
	m_Cancelled(os::event::type::manual, os::event::state::nonsignaled)
{
}


FileSystemWatcher::~FileSystemWatcher()
{
	try
	{
		Release();
	}
	catch (...)
	{
		// TODO: log
	}
}

void FileSystemWatcher::Set(string_view const Directory, bool const WatchSubtree)
{
	Release();

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
	m_Notification = {};
	m_PreviousLastWriteTime = m_CurrentLastWriteTime;
}

bool FileSystemWatcher::Signaled() const
{
	PropagateException();

	return (m_Notification && m_Notification.is_signaled()) || m_PreviousLastWriteTime != m_CurrentLastWriteTime;
}

void FileSystemWatcher::Register()
{
	seh_try_thread(m_ExceptionPtr, [this]
	{
		cpp_try(
		[&]
		{
			m_Notification = os::fs::FindFirstChangeNotification(m_Directory, m_WatchSubtree,
				FILE_NOTIFY_CHANGE_FILE_NAME |
				FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_ATTRIBUTES |
				FILE_NOTIFY_CHANGE_SIZE |
				FILE_NOTIFY_CHANGE_LAST_WRITE);

			if (!m_Notification)
				return;

			(void)os::handle::wait_any({ m_Notification.native_handle(), m_Cancelled.native_handle() });
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
