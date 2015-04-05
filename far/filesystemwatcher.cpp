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

#include "headers.hpp"
#pragma hdrstop

#include "filesystemwatcher.hpp"
#include "flink.hpp"
#include "elevation.hpp"
#include "datetime.hpp"

FileSystemWatcher::FileSystemWatcher():
	m_PreviousLastWriteTime(),
	m_CurrentLastWriteTime(),
	m_bOpen(false),
	m_WatchSubtree(false),
	m_WatchRegistered(Event::manual, Event::signaled),
	m_Done(Event::manual, Event::nonsignaled),
	m_DoneDone(Event::manual, Event::nonsignaled),
	m_Changed(Event::manual, Event::nonsignaled)
{
}


FileSystemWatcher::~FileSystemWatcher()
{
	Release();
}

void FileSystemWatcher::Set(const string& Directory, bool WatchSubtree)
{
	m_WatchRegistered.Wait();
	this->m_Directory = Directory;
	this->m_WatchSubtree = WatchSubtree;

	if (os::GetFileTimeSimple(Directory,nullptr,nullptr,&m_PreviousLastWriteTime,nullptr))
		m_CurrentLastWriteTime = m_PreviousLastWriteTime;
}

void FileSystemWatcher::WatchRegister()
{
	HANDLE Handle=FindFirstChangeNotification(m_Directory.data(), m_WatchSubtree,
									FILE_NOTIFY_CHANGE_FILE_NAME|
									FILE_NOTIFY_CHANGE_DIR_NAME|
									FILE_NOTIFY_CHANGE_ATTRIBUTES|
									FILE_NOTIFY_CHANGE_SIZE|
									FILE_NOTIFY_CHANGE_LAST_WRITE);
	m_WatchRegistered.Set();

	MultiWaiter waiter;
	waiter.Add(Handle);
	waiter.Add(m_Done);
	if (waiter.Wait(MultiWaiter::wait_any) == WAIT_OBJECT_0)
	{
		m_Changed.Set();
		m_Done.Wait();
	}

	m_DoneDone.Set();
	FindCloseChangeNotification(Handle);
}

void FileSystemWatcher::Watch(bool got_focus, bool check_time)
{
	SCOPED_ACTION(elevation::suppress);

	if(!m_bOpen)
	{
		m_bOpen = true;
		m_Done.Reset();
		m_DoneDone.Reset();
		m_WatchRegistered.Reset();
		Thread(&Thread::detach, &FileSystemWatcher::WatchRegister, this);
	}

	if (got_focus)
	{
		bool isFAT = false;
		string strRoot, strFileSystem;
		GetPathRoot(m_Directory, strRoot);
		if (!strRoot.empty())
		{
			if (os::GetVolumeInformation(strRoot, nullptr, nullptr, nullptr, nullptr, &strFileSystem))
				isFAT = (strFileSystem.substr(0,3) == L"FAT");
		}
		if (isFAT)             // emulate FAT folder time change
		{                      // otherwise changes missed (FAT folder time is NOT modified)
			check_time = false; // the price is directory reload on each GOT_FOCUS event
			m_PreviousLastWriteTime.dwLowDateTime = m_CurrentLastWriteTime.dwLowDateTime - 1;
		}
	}

	if (check_time)
	{
		if (!os::GetFileTimeSimple(m_Directory,nullptr,nullptr,&m_CurrentLastWriteTime,nullptr))
		{
			m_PreviousLastWriteTime.dwLowDateTime = 0;
			m_PreviousLastWriteTime.dwHighDateTime = 0;
			m_CurrentLastWriteTime.dwLowDateTime = 0;
			m_CurrentLastWriteTime.dwHighDateTime = 0;
		}
	}
}

void FileSystemWatcher::Release()
{
	if (m_bOpen)
	{
		m_Done.Set();
		m_DoneDone.Wait();
		m_bOpen = false;
		m_Changed.Reset();
	}
	m_PreviousLastWriteTime = m_CurrentLastWriteTime;
}

bool FileSystemWatcher::Signaled() const
{
	return m_Changed.Signaled() || m_PreviousLastWriteTime != m_CurrentLastWriteTime;
}
