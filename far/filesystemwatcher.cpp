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

FileSystemWatcher::FileSystemWatcher():
	PreviousLastWriteTime(),
	CurrentLastWriteTime(),
	bOpen(false),
	WatchSubtree(false)
{
	WatchRegistered.Open(true, true);
	Changed.Open(true, false);
	Done.Open(true, false);
	DoneDone.Open(true, false);
}


FileSystemWatcher::~FileSystemWatcher()
{
	Release();
}

void FileSystemWatcher::Set(const string& Directory, bool WatchSubtree)
{
	WatchRegistered.Wait();
	this->Directory = Directory;
	this->WatchSubtree = WatchSubtree;

	if (api::GetFileTimeSimple(Directory,nullptr,nullptr,&PreviousLastWriteTime,nullptr))
		CurrentLastWriteTime = PreviousLastWriteTime;
}

unsigned int FileSystemWatcher::WatchRegister(LPVOID lpParameter)
{
	HANDLE Handle=FindFirstChangeNotification(Directory.data(), WatchSubtree,
									FILE_NOTIFY_CHANGE_FILE_NAME|
									FILE_NOTIFY_CHANGE_DIR_NAME|
									FILE_NOTIFY_CHANGE_ATTRIBUTES|
									FILE_NOTIFY_CHANGE_SIZE|
									FILE_NOTIFY_CHANGE_LAST_WRITE);
	WatchRegistered.Set();

	MultiWaiter waiter;
	waiter.Add(Handle);
	waiter.Add(Done);
	if (waiter.Wait(false, INFINITE) == WAIT_OBJECT_0)
	{
		Changed.Set();
		Done.Wait();
	}

	DoneDone.Set();
	FindCloseChangeNotification(Handle);

	return 0;
}

void FileSystemWatcher::Watch(bool got_focus, bool check_time)
{
	SCOPED_ACTION(elevation::suppress);

	if(!bOpen)
	{
		bOpen = true;
		Done.Reset();
		DoneDone.Reset();
		WatchRegistered.Reset();
		Thread WatchThread;
		WatchThread.Start(&FileSystemWatcher::WatchRegister, this);
	}

	if (got_focus)
	{
		bool isFAT = false;
		string strRoot, strFileSystem;
		GetPathRoot(Directory, strRoot);
		if (!strRoot.empty())
		{
			if (api::GetVolumeInformation(strRoot, nullptr, nullptr, nullptr, nullptr, &strFileSystem))
				isFAT = (strFileSystem.substr(0,3) == L"FAT");
		}
		if (isFAT)             // emulate FAT folder time change
		{                      // otherwise changes missed (FAT folder time is NOT modified)
			check_time = false; // the price is directory reload on each GOT_FOCUS event
			PreviousLastWriteTime.dwLowDateTime = CurrentLastWriteTime.dwLowDateTime - 1;
		}
	}

	if (check_time)
	{
		if (!api::GetFileTimeSimple(Directory,nullptr,nullptr,&CurrentLastWriteTime,nullptr))
		{
			PreviousLastWriteTime.dwLowDateTime = 0;
			PreviousLastWriteTime.dwHighDateTime = 0;
			CurrentLastWriteTime.dwLowDateTime = 0;
			CurrentLastWriteTime.dwHighDateTime = 0;
		}
	}
}

void FileSystemWatcher::Release()
{
	if (bOpen)
	{
		Done.Set();
		DoneDone.Wait();
		bOpen = false;
		Changed.Reset();
	}
	PreviousLastWriteTime = CurrentLastWriteTime;
}

bool FileSystemWatcher::Signaled()
{
	return Changed.Signaled() || CompareFileTime(&PreviousLastWriteTime, &CurrentLastWriteTime);
}
