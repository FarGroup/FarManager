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

CachedRead::CachedRead(File& file, DWORD buffer_size):
	file(file),
	ReadSize(0),
	BytesLeft(0),
	LastPtr(0),
	Alignment(512)
{
	BufferSize = (buffer_size ? (buffer_size+Alignment-1) & ~(Alignment-1) : DefaultBufferSize);
	Buffer = static_cast<LPBYTE>(xf_malloc(BufferSize));
}

CachedRead::~CachedRead()
{
	xf_free(Buffer);
}

bool CachedRead::AdjustAlignment()
{
	if (!file.Opened())
		return false;

	DWORD ret, buff_size = BufferSize;
	DISK_GEOMETRY g;

	if (file.IoControl(IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr,0, &g,(DWORD)sizeof(g), &ret,nullptr))
	{
		if (g.BytesPerSector > 512 && g.BytesPerSector <= 256*1024)
		{
			Alignment = (int)g.BytesPerSector;
			buff_size = 16 * g.BytesPerSector;
		}
		file.IoControl(FSCTL_ALLOW_EXTENDED_DASD_IO, nullptr,0, nullptr,0, &ret,nullptr);
	}

	if (buff_size > BufferSize)
	{
		xf_free(Buffer);
		Buffer = static_cast<LPBYTE>(xf_malloc(BufferSize = buff_size));
	}

	Clear();
	return Buffer != nullptr;
}

void CachedRead::Clear()
{
	ReadSize=0;
	BytesLeft=0;
	LastPtr=0;
}

bool CachedRead::Read(LPVOID Data, DWORD DataSize, LPDWORD BytesRead)
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
	if(DataSize<=BufferSize && Buffer)
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

			DWORD Actual=Min(BytesLeft, DataSize);
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

bool CachedRead::Unread(DWORD BytesUnread)
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
		INT64 Pointer = file.GetPointer();

		int shift = (int)(Pointer % Alignment);
		if (Pointer-shift > BufferSize/2)
			shift += BufferSize/2;

		if (shift)
			file.SetPointer(-shift, nullptr, FILE_CURRENT);

		DWORD read_size = BufferSize;
		UINT64 FileSize = 0;
		if (file.GetSize(FileSize) && Pointer-shift+BufferSize > (INT64)FileSize)
			read_size = (DWORD)((INT64)FileSize-Pointer+shift);

		Result = file.Read(Buffer, read_size, ReadSize);
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

CachedWrite::CachedWrite(File& file):
	Buffer(static_cast<LPBYTE>(xf_malloc(BufferSize))),
	file(file),
	FreeSize(BufferSize),
	Flushed(false)
{
}

CachedWrite::~CachedWrite()
{
	Flush();

	if (Buffer)
	{
		xf_free(Buffer);
	}
}

bool CachedWrite::Write(LPCVOID Data, size_t DataSize)
{
	bool Result=false;
	bool SuccessFlush=true;

	if (Buffer)
	{
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
				memcpy(&Buffer[BufferSize-FreeSize],Data,DataSize);
				FreeSize -= static_cast<DWORD>(DataSize);
				Flushed=false;
				Result=true;
			}
		}
	}
	return Result;
}

bool CachedWrite::Flush()
{
	if (Buffer)
	{
		if (!Flushed)
		{
			DWORD WrittenSize=0;

			if (file.Write(Buffer, BufferSize-FreeSize, WrittenSize, nullptr) && BufferSize-FreeSize==WrittenSize)
			{
				Flushed=true;
				FreeSize=BufferSize;
			}
		}
	}

	return Flushed;
}
