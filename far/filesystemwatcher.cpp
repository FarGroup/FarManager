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
	Handle(INVALID_HANDLE_VALUE),
	WatchSubtree(false)
{
	PreviousLastWriteTime.dwLowDateTime = 0;
	PreviousLastWriteTime.dwHighDateTime = 0;
	CurrentLastWriteTime.dwLowDateTime = 0;
	CurrentLastWriteTime.dwHighDateTime = 0;
}


FileSystemWatcher::~FileSystemWatcher()
{
}

void FileSystemWatcher::Set(const wchar_t* Directory, bool WatchSubtree)
{
	this->Directory = Directory;
	this->WatchSubtree = WatchSubtree;

	FAR_FIND_DATA_EX data;
	if(apiGetFindDataEx(Directory, data))
	{
		PreviousLastWriteTime = data.ftLastWriteTime;
	}
}

bool FileSystemWatcher::Watch()
{
	DisableElevation de;
	if(Handle == INVALID_HANDLE_VALUE)
	{
		Handle=FindFirstChangeNotification(Directory, WatchSubtree,
		FILE_NOTIFY_CHANGE_FILE_NAME|
		FILE_NOTIFY_CHANGE_DIR_NAME|
		FILE_NOTIFY_CHANGE_ATTRIBUTES|
		FILE_NOTIFY_CHANGE_SIZE|
		FILE_NOTIFY_CHANGE_LAST_WRITE);
	}

	bool isFAT = false;
	string strRoot, strFileSystem;
	GetPathRoot(Directory.CPtr(), strRoot);
	if (!strRoot.IsEmpty())
	{
		if (apiGetVolumeInformation(strRoot, nullptr, nullptr, nullptr, nullptr, &strFileSystem))
			isFAT = (strFileSystem.SubStr(0,3) == L"FAT");
	}

	if (isFAT)
	{
		if (++CurrentLastWriteTime.dwLowDateTime == 0)
			++CurrentLastWriteTime.dwHighDateTime;
	}
	else
	{
		File dir;
		bool TimeOk = false;
		if (dir.Open(Directory,GENERIC_READ,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING))
		{
			FILETIME write_time;
			if (dir.GetTime(nullptr,nullptr,&write_time,nullptr))
			{
				CurrentLastWriteTime = write_time;
				TimeOk = true;
			}
		}
		if(!TimeOk)
		{
			PreviousLastWriteTime.dwLowDateTime = 0;
			PreviousLastWriteTime.dwHighDateTime = 0;
			CurrentLastWriteTime.dwLowDateTime = 0;
			CurrentLastWriteTime.dwHighDateTime = 0;
		}
	}

	return Handle != INVALID_HANDLE_VALUE;
}

void FileSystemWatcher::Release()
{
	if(Handle != INVALID_HANDLE_VALUE)
	{
		HANDLE hCloseThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(FindCloseChangeNotification), Handle, 0, nullptr);
		if (hCloseThread)
		{
			CloseHandle(hCloseThread);
		}
		else
		{
			FindCloseChangeNotification(Handle);
		}
		Handle = INVALID_HANDLE_VALUE;
	}
	PreviousLastWriteTime = CurrentLastWriteTime;
}

bool FileSystemWatcher::Signaled()
{
	return (Handle != INVALID_HANDLE_VALUE && WaitForSingleObject(Handle,0) == WAIT_OBJECT_0) || CompareFileTime(&PreviousLastWriteTime, &CurrentLastWriteTime);
}
