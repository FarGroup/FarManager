/*
cache.cpp

Кеширование записи в файл/чтения из файла
*/
/*
Copyright (c) 2009 Far Group
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

CachedRead::CachedRead(File& file):
	Buffer(reinterpret_cast<LPBYTE>(xf_malloc(BufferSize))),
	file(file),
	ReadSize(0),
	BytesLeft(0),
	LastPtr(0)
{
}

CachedRead::~CachedRead()
{
	if (Buffer)
	{
		xf_free(Buffer);
	}
}

bool CachedRead::Read(LPVOID Data, DWORD DataSize, LPDWORD BytesRead)
{
	INT64 Ptr=0;
	file.GetPointer(Ptr);

	if(Ptr!=LastPtr)
	{
		INT64 MaxValidPtr=LastPtr+BytesLeft, MinValidPtr=MaxValidPtr-ReadSize;
		if(Ptr>=MinValidPtr && Ptr<=MaxValidPtr)
		{
			BytesLeft-=static_cast<int>(Ptr-LastPtr);
		}
		else
		{
			BytesLeft=0;
		}
	}
	bool Result=true;
	*BytesRead=0;
	if(DataSize<BufferSize)
	{
		if (Buffer)
		{
			if(!BytesLeft)
			{
				FillBuffer();
			}

			if(BytesLeft)
			{
				DWORD Actual=Min(BytesLeft, DataSize);
				memcpy(Data, &Buffer[ReadSize-BytesLeft], Actual);
				BytesLeft-=Actual;

				file.SetPointer(Actual, &LastPtr, FILE_CURRENT);
				*BytesRead=Actual;
				DataSize-=Actual;
				if(BytesLeft<DataSize)
				{
					FillBuffer();
					if(BytesLeft)
					{
						Actual=Min(BytesLeft, DataSize);
						memcpy(Data, &Buffer[ReadSize-BytesLeft], Actual);
						BytesLeft-=Actual;
						file.SetPointer(Actual, &LastPtr, FILE_CURRENT);
						*BytesRead=Actual;
					}
				}
			}
		}
	}
	else
	{
		Result = file.Read(Data, DataSize, BytesRead);
	}
	return Result;
}

bool CachedRead::FillBuffer()
{
	bool Result=false;
	if(!file.Eof())
	{
		INT64 Pointer=0;
		file.GetPointer(Pointer);
		bool Bidirection=false;
		if(Pointer>BufferSize/2)
		{
			Bidirection=true;
			file.SetPointer(-BufferSize/2, nullptr, FILE_CURRENT);
		}
		Result = file.Read(Buffer, BufferSize, &ReadSize);
		if(Result)
		{
			BytesLeft = ReadSize;
			if(Bidirection && BytesLeft>BufferSize/2)
			{
				BytesLeft-=BufferSize/2;
			}
			file.SetPointer(Pointer, nullptr, FILE_BEGIN);
		}
	}
	return Result;
}


CachedWrite::CachedWrite(File& file):
	Buffer(reinterpret_cast<LPBYTE>(xf_malloc(BufferSize))),
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

bool CachedWrite::Write(LPCVOID Data, DWORD DataSize)
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
				DWORD WrittenSize=0;

				if (file.Write(Data, DataSize,&WrittenSize) && DataSize==WrittenSize)
				{
					Result=true;
				}
			}
			else
			{
				memcpy(&Buffer[BufferSize-FreeSize],Data,DataSize);
				FreeSize-=DataSize;
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

			if (file.Write(Buffer, BufferSize-FreeSize, &WrittenSize, nullptr) && BufferSize-FreeSize==WrittenSize)
			{
				Flushed=true;
				FreeSize=BufferSize;
			}
		}
	}

	return Flushed;
}
