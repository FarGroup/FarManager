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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "cache.hpp"

// Internal:
#include "platform.fs.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

CachedRead::CachedRead(os::fs::file& File, size_t BufferSize):
	m_File(File),
	m_Alignment(512),
	m_Buffer(BufferSize? aligned_size(BufferSize, m_Alignment) : 65536)
{
}

void CachedRead::AdjustAlignment()
{
	if (!m_File)
		return;

	auto BufferSize = m_Buffer.size();

	STORAGE_PROPERTY_QUERY Spq{};
	Spq.QueryType  = PropertyStandardQuery;
	Spq.PropertyId = StorageAccessAlignmentProperty;

	STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR Saad;

	if (m_File.IoControl(IOCTL_STORAGE_QUERY_PROPERTY, &Spq, sizeof(Spq), &Saad, sizeof(Saad)))
	{
		if (Saad.BytesPerPhysicalSector > 512 && Saad.BytesPerPhysicalSector <= 256*1024)
		{
			m_Alignment = static_cast<int>(Saad.BytesPerPhysicalSector);
			BufferSize = 16 * Saad.BytesPerPhysicalSector;
		}
		(void)m_File.IoControl(FSCTL_ALLOW_EXTENDED_DASD_IO, nullptr, 0, nullptr, 0);
	}

	if (BufferSize > m_Buffer.size())
	{
		m_Buffer.resize(BufferSize);
	}

	Clear();
}

void CachedRead::Clear()
{
	m_ReadSize = 0;
	m_BytesLeft = 0;
	m_LastPtr = 0;
}

bool CachedRead::Read(void* Data, size_t DataSize, size_t* BytesRead)
{
	const auto Ptr = m_File.GetPointer();

	if (Ptr != m_LastPtr)
	{
		const auto ValidRangeBegin = m_LastPtr + m_BytesLeft;
		const auto ValidRangeEnd = ValidRangeBegin - m_ReadSize;

		if (Ptr >= ValidRangeEnd && Ptr < ValidRangeBegin)
		{
			m_BytesLeft -= static_cast<int>(Ptr - m_LastPtr);
		}
		else
		{
			m_BytesLeft = 0;
		}
		m_LastPtr = Ptr;
	}

	bool Result = false;
	*BytesRead = 0;

	if (DataSize <= m_Buffer.size())
	{
		while (DataSize)
		{
			if (!m_BytesLeft)
			{
				FillBuffer();

				if (!m_BytesLeft)
					break;
			}

			Result = true;

			const auto Actual = std::min(m_BytesLeft, DataSize);
			copy_memory(&m_Buffer[m_ReadSize - m_BytesLeft], Data, Actual);
			Data = static_cast<char*>(Data) + Actual;
			m_BytesLeft -= Actual;
			m_File.SetPointer(Actual, &m_LastPtr, FILE_CURRENT);
			*BytesRead += Actual;
			DataSize -= Actual;
		}
	}
	else
	{
		Result = m_File.Read(Data, DataSize, *BytesRead);
	}
	return Result;
}

bool CachedRead::Unread(size_t BytesUnread)
{
	if (m_BytesLeft + BytesUnread > m_ReadSize)
		return false;

	m_BytesLeft += BytesUnread;
	const long long Offset = BytesUnread;
	m_File.SetPointer(-Offset, &m_LastPtr, FILE_CURRENT);
	return true;
}

bool CachedRead::FillBuffer()
{
	if (m_File.Eof())
		return false;

	const auto Pointer = m_File.GetPointer();

	auto Shift = static_cast<int>(Pointer % m_Alignment);
	if (Pointer > m_Buffer.size() / 2 + Shift)
		Shift += static_cast<int>(m_Buffer.size() / 2);

	if (Shift)
		m_File.SetPointer(-Shift, nullptr, FILE_CURRENT);

	auto ReadSize = m_Buffer.size();
	unsigned long long FileSize = 0;
	if (m_File.GetSize(FileSize) && Pointer - Shift + m_Buffer.size() > FileSize)
		ReadSize = FileSize - Pointer + Shift;

	const auto Result = m_File.Read(m_Buffer.data(), ReadSize, m_ReadSize);
	if (Result)
	{
		if (m_ReadSize > static_cast<size_t>(Shift))
		{
			m_BytesLeft = m_ReadSize - Shift;
			m_File.SetPointer(Pointer, nullptr, FILE_BEGIN);
		}
		else
		{
			m_BytesLeft = 0;
		}
	}
	else
	{
		if (Shift)
			m_File.SetPointer(Pointer, nullptr, FILE_BEGIN);
		m_ReadSize = 0;
		m_BytesLeft = 0;
	}

	return Result;
}
