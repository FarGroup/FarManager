/*
cache.cpp

Кеширование записи в файл/чтения из файла
*/
/*
Copyright © 2009 Far Group
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

#include "cache.hpp"

CachedRead::CachedRead(api::File& file, DWORD buffer_size):
	file(file),
	ReadSize(0),
	BytesLeft(0),
	LastPtr(0),
	Alignment(512),
	Buffer(buffer_size ? (buffer_size + Alignment - 1) & ~(Alignment - 1) : DefaultBufferSize)
{
}

CachedRead::~CachedRead()
{
}

void CachedRead::AdjustAlignment()
{
	if (!file.Opened())
		return;

	DWORD ret;
	size_t buff_size = Buffer.size();
	DISK_GEOMETRY g;

	if (file.IoControl(IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr,0, &g, sizeof(g), &ret, nullptr))
	{
		if (g.BytesPerSector > 512 && g.BytesPerSector <= 256*1024)
		{
			Alignment = (int)g.BytesPerSector;
			buff_size = 16 * g.BytesPerSector;
		}
		file.IoControl(FSCTL_ALLOW_EXTENDED_DASD_IO, nullptr,0, nullptr,0, &ret,nullptr);
	}

	if (buff_size > Buffer.size())
	{
		Buffer.resize(buff_size);
	}

	Clear();
}

void CachedRead::Clear()
{
	ReadSize=0;
	BytesLeft=0;
	LastPtr=0;
}

bool CachedRead::Read(LPVOID Data, size_t DataSize, size_t* BytesRead)
{
	INT64 Ptr = file.GetPointer();

	if(Ptr!=LastPtr)
	{
		INT64 MaxValidPtr=LastPtr+BytesLeft, MinValidPtr=MaxValidPtr-ReadSize;
		if(Ptr>=MinValidPtr && Ptr<MaxValidPtr)
		{
			BytesLeft-=static_cast<int>(Ptr-LastPtr);
		}
		else
		{
			BytesLeft=0;
		}
		LastPtr=Ptr;
	}
	bool Result=false;
	*BytesRead=0;
	if(DataSize<=Buffer.size())
	{
		while (DataSize)
		{
			if (!BytesLeft)
			{
				FillBuffer();

				if (!BytesLeft)
					break;
			}

			Result=true;

			size_t Actual = std::min(BytesLeft, DataSize);
			memcpy(Data, &Buffer[ReadSize-BytesLeft], Actual);
			Data=((LPBYTE)Data)+Actual;
			BytesLeft-=Actual;
			file.SetPointer(Actual, &LastPtr, FILE_CURRENT);
			*BytesRead+=Actual;
			DataSize-=Actual;
		}
	}
	else
	{
		Result = file.Read(Data, DataSize, *BytesRead);
	}
	return Result;
}

bool CachedRead::Unread(size_t BytesUnread)
{
	if (BytesUnread + BytesLeft <= ReadSize)
	{
		BytesLeft += BytesUnread;
		__int64 off = BytesUnread;
		file.SetPointer(-off, &LastPtr, FILE_CURRENT);
		return true;
	}
	return false;
}

bool CachedRead::FillBuffer()
{
	bool Result=false;
	if (!file.Eof())
	{
		UINT64 Pointer = file.GetPointer();

		int shift = (int)(Pointer % Alignment);
		if (Pointer-shift > Buffer.size()/2)
			shift += static_cast<int>(Buffer.size() / 2);

		if (shift)
			file.SetPointer(-shift, nullptr, FILE_CURRENT);

		size_t read_size = Buffer.size();
		UINT64 FileSize = 0;
		if (file.GetSize(FileSize) && Pointer - shift + Buffer.size() > FileSize)
			read_size = FileSize - Pointer + shift;

		Result = file.Read(Buffer.data(), static_cast<DWORD>(read_size), ReadSize);
		if (Result)
		{
			if (ReadSize > (DWORD)shift)
			{
				BytesLeft = ReadSize - shift;
				file.SetPointer(Pointer, nullptr, FILE_BEGIN);
			}
			else
			{
				BytesLeft = 0;
			}
		}
		else
		{
			if (shift)
				file.SetPointer(Pointer, nullptr, FILE_BEGIN);
			ReadSize=0;
			BytesLeft=0;
		}
	}

	return Result;
}

CachedWrite::CachedWrite(api::File& file):
	file(file),
	Buffer(0x10000),
	FreeSize(Buffer.size()),
	Flushed(false)
{
}

CachedWrite::~CachedWrite()
{
	Flush();
}

bool CachedWrite::Write(LPCVOID Data, size_t DataSize)
{
	bool Result=false;

	bool SuccessFlush=true;
	if (DataSize>FreeSize)
	{
		SuccessFlush=Flush();
	}

	if(SuccessFlush)
	{
		if (DataSize>FreeSize)
		{
			size_t WrittenSize=0;

			if (file.Write(Data, DataSize,WrittenSize) && DataSize==WrittenSize)
			{
				Result=true;
			}
		}
		else
		{
			memcpy(&Buffer[Buffer.size() - FreeSize], Data, DataSize);
			FreeSize -= DataSize;
			Flushed=false;
			Result=true;
		}
	}
	return Result;
}

bool CachedWrite::Flush()
{
	if (!Flushed)
	{
		size_t WrittenSize = 0;

		if (file.Write(Buffer.data(), static_cast<DWORD>(Buffer.size() - FreeSize), WrittenSize, nullptr) && Buffer.size() - FreeSize == WrittenSize)
		{
			Flushed=true;
			FreeSize = Buffer.size();
		}
	}

	return Flushed;
}
