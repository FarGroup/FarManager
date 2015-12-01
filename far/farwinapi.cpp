/*
farwinapi.cpp

Враперы вокруг некоторых WinAPI функций
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "flink.hpp"
#include "imports.hpp"
#include "pathmix.hpp"
#include "ctrlobj.hpp"
#include "elevation.hpp"
#include "plugins.hpp"
#include "datetime.hpp"

namespace os
{
	HandleWrapper::~HandleWrapper() {}

	namespace detail
	{
		class i_find_handle_impl
		{
		public:
			virtual ~i_find_handle_impl() = 0;
		};
		i_find_handle_impl::~i_find_handle_impl() {}

		bool detail::find_handle_closer::close(HANDLE Handle) { delete static_cast<i_find_handle_impl*>(Handle); return true; }
		bool detail::handle_closer::close(HANDLE Handle) { return CloseHandle(Handle) != FALSE; }
		struct os_find_handle_closer { static bool close(HANDLE Handle) { return FindClose(Handle) != FALSE; } };

		class far_find_handle_impl: public i_find_handle_impl
		{
		public:
			far_find_handle_impl():
				NextOffset(),
				BufferSize(),
				Extended(),
				ReadDone()
			{
			}

			fs::file Object;
			block_ptr<BYTE> BufferBase;
			block_ptr<BYTE> Buffer2;
			ULONG NextOffset;
			ULONG BufferSize;
			bool Extended;
			bool ReadDone;
		};

		class os_find_handle_impl: public i_find_handle_impl
		{
		public:
			os_find_handle_impl(HANDLE Handle): m_Handle(Handle) {}
			HANDLE hative_handle() const { return m_Handle.native_handle(); }

		private:
			handle_t<os_find_handle_closer> m_Handle;
		};
	}

static find_handle FindFirstFileInternal(const string& Name, FAR_FIND_DATA& FindData)
{
	FN_RETURN_TYPE(FindFirstFileInternal) Result;

	if(!Name.empty() && !IsSlash(Name.back()))
	{
			auto Handle = std::make_unique<detail::far_find_handle_impl>();

			string strDirectory(Name);
			CutToSlash(strDirectory);
			if(Handle->Object.Open(strDirectory, FILE_LIST_DIRECTORY, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
			{

				// for network paths buffer size must be <= 64k
				Handle->BufferSize = 0x10000;
				Handle->BufferBase.reset(Handle->BufferSize);
				if (Handle->BufferBase)
				{
					LPCWSTR NamePtr = PointToName(Name);
					Handle->Extended = true;

					bool QueryResult = Handle->Object.NtQueryDirectoryFile(Handle->BufferBase.get(), Handle->BufferSize, FileIdBothDirectoryInformation, false, NamePtr, true);
					if (QueryResult) // try next read immediately to avoid M#2128 bug
					{
						Handle->Buffer2.reset(Handle->BufferSize);
						bool QueryResult2 = Handle->Object.NtQueryDirectoryFile(Handle->Buffer2.get(), Handle->BufferSize, FileIdBothDirectoryInformation, false, NamePtr, false);
						if (!QueryResult2)
						{
							Handle->Buffer2.reset();
							if (GetLastError() != ERROR_INVALID_LEVEL)
								Handle->ReadDone = true;
							else
								QueryResult = false;
						}
					}

					if(!QueryResult)
					{
						Handle->Extended = false;

						// re-create handle to avoid weird bugs with some network emulators
						Handle->Object.Close();
						if(Handle->Object.Open(strDirectory, FILE_LIST_DIRECTORY, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
						{
							QueryResult = Handle->Object.NtQueryDirectoryFile(Handle->BufferBase.get(), Handle->BufferSize, FileBothDirectoryInformation, false, NamePtr, true);
						}
					}
					if(QueryResult)
					{
						const auto DirectoryInfo = reinterpret_cast<const FILE_ID_BOTH_DIR_INFORMATION*>(Handle->BufferBase.get());
						FindData.dwFileAttributes = DirectoryInfo->FileAttributes;
						FindData.ftCreationTime = UI64ToFileTime(DirectoryInfo->CreationTime.QuadPart);
						FindData.ftLastAccessTime = UI64ToFileTime(DirectoryInfo->LastAccessTime.QuadPart);
						FindData.ftLastWriteTime = UI64ToFileTime(DirectoryInfo->LastWriteTime.QuadPart);
						FindData.ftChangeTime = UI64ToFileTime(DirectoryInfo->ChangeTime.QuadPart);
						FindData.nFileSize = DirectoryInfo->EndOfFile.QuadPart;
						FindData.nAllocationSize = DirectoryInfo->AllocationSize.QuadPart;
						FindData.dwReserved0 = FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT?DirectoryInfo->EaSize:0;

						if(Handle->Extended)
						{
							FindData.FileId = DirectoryInfo->FileId.QuadPart;
							FindData.strFileName.assign(DirectoryInfo->FileName,DirectoryInfo->FileNameLength/sizeof(WCHAR));
							FindData.strAlternateFileName.assign(DirectoryInfo->ShortName,DirectoryInfo->ShortNameLength/sizeof(WCHAR));
						}
						else
						{
							FindData.FileId = 0;
							const auto DirectoryInfoSimple = reinterpret_cast<const FILE_BOTH_DIR_INFORMATION*>(DirectoryInfo);
							FindData.strFileName.assign(DirectoryInfoSimple->FileName,DirectoryInfoSimple->FileNameLength/sizeof(WCHAR));
							FindData.strAlternateFileName.assign(DirectoryInfoSimple->ShortName,DirectoryInfoSimple->ShortNameLength/sizeof(WCHAR));
						}

						// Bug in SharePoint: FileName is zero-terminated and FileNameLength INCLUDES this zero.
						if(!FindData.strFileName.empty() && !FindData.strFileName.back())
						{
							FindData.strFileName.pop_back();
						}
						if(!FindData.strAlternateFileName.empty() && !FindData.strAlternateFileName.back())
						{
							FindData.strAlternateFileName.pop_back();
						}

						Handle->NextOffset = DirectoryInfo->NextEntryOffset;
						Result.reset(Handle.release());
					}
					else
					{
						Handle->BufferBase.reset();
					}
				}
			}
			else
			{
				// fix error code if we looking for FILE(S) in non-existent directory, not directory itself
				if(GetLastError() == ERROR_FILE_NOT_FOUND && *PointToName(Name))
				{
					SetLastError(ERROR_PATH_NOT_FOUND);
				}
			}
	}
	return Result;
}

static bool FindNextFileInternal(const find_handle& Find, FAR_FIND_DATA& FindData)
{
	bool Result = false;
	const auto Handle = static_cast<detail::far_find_handle_impl*>(Find.native_handle());
	bool Status = true, set_errcode = true;
	auto DirectoryInfo = reinterpret_cast<const FILE_ID_BOTH_DIR_INFORMATION*>(Handle->BufferBase.get());
	if(Handle->NextOffset)
	{
		DirectoryInfo = reinterpret_cast<const FILE_ID_BOTH_DIR_INFORMATION*>(reinterpret_cast<const char*>(DirectoryInfo)+Handle->NextOffset);
	}
	else
	{
		if (Handle->ReadDone)
		{
			Status = false;
		}
		else
		{
			if (Handle->Buffer2)
			{
				Handle->BufferBase.reset();
				Handle->BufferBase.swap(Handle->Buffer2);
				DirectoryInfo = reinterpret_cast<PFILE_ID_BOTH_DIR_INFORMATION>(Handle->BufferBase.get());
			}
			else
			{
				Status = Handle->Object.NtQueryDirectoryFile(Handle->BufferBase.get(), Handle->BufferSize, Handle->Extended ? FileIdBothDirectoryInformation : FileBothDirectoryInformation, false, nullptr, false);
				set_errcode = false;
			}
		}
	}

	if(Status)
	{
		FindData.dwFileAttributes = DirectoryInfo->FileAttributes;
		FindData.ftCreationTime = UI64ToFileTime(DirectoryInfo->CreationTime.QuadPart);
		FindData.ftLastAccessTime = UI64ToFileTime(DirectoryInfo->LastAccessTime.QuadPart);
		FindData.ftLastWriteTime = UI64ToFileTime(DirectoryInfo->LastWriteTime.QuadPart);
		FindData.ftChangeTime = UI64ToFileTime(DirectoryInfo->ChangeTime.QuadPart);
		FindData.nFileSize = DirectoryInfo->EndOfFile.QuadPart;
		FindData.nAllocationSize = DirectoryInfo->AllocationSize.QuadPart;
		FindData.dwReserved0 = FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT?DirectoryInfo->EaSize:0;

		if(Handle->Extended)
		{
			FindData.FileId = DirectoryInfo->FileId.QuadPart;
			FindData.strFileName.assign(DirectoryInfo->FileName,DirectoryInfo->FileNameLength/sizeof(WCHAR));
			FindData.strAlternateFileName.assign(DirectoryInfo->ShortName,DirectoryInfo->ShortNameLength/sizeof(WCHAR));
		}
		else
		{
			FindData.FileId = 0;
			const auto DirectoryInfoSimple = reinterpret_cast<const FILE_BOTH_DIR_INFORMATION*>(DirectoryInfo);
			FindData.strFileName.assign(DirectoryInfoSimple->FileName,DirectoryInfoSimple->FileNameLength/sizeof(WCHAR));
			FindData.strAlternateFileName.assign(DirectoryInfoSimple->ShortName,DirectoryInfoSimple->ShortNameLength/sizeof(WCHAR));
		}

		// Bug in SharePoint: FileName is zero-terminated and FileNameLength INCLUDES this zero.
		if(!FindData.strFileName.empty() && !FindData.strFileName.back())
		{
			FindData.strFileName.pop_back();
		}
		if(!FindData.strAlternateFileName.empty() && !FindData.strAlternateFileName.back())
		{
			FindData.strAlternateFileName.pop_back();
		}

		Handle->NextOffset = DirectoryInfo->NextEntryOffset?Handle->NextOffset+DirectoryInfo->NextEntryOffset:0;
		Result = true;
	}

	if (set_errcode)
		SetLastError(Result ? ERROR_SUCCESS : ERROR_NO_MORE_FILES);

	return Result;
}

//-------------------------------------------------------------------------
namespace fs
{
	file_status::file_status():
		m_Data(INVALID_FILE_ATTRIBUTES)
	{
	}

	file_status::file_status(const string& Object) :
		m_Data(os::GetFileAttributes(Object))
	{

	}

	file_status::file_status(const wchar_t* Object):
		m_Data(os::GetFileAttributes(Object))
	{

	}

	bool file_status::check(DWORD Data)
	{
		return m_Data != INVALID_FILE_ATTRIBUTES && m_Data & Data;
	}

	bool exists(file_status Status)
	{
		return Status.check(~0);
	}

	bool is_file(file_status Status)
	{
		return exists(Status) && !is_directory(Status);
	}

	bool is_directory(file_status Status)
	{
		return Status.check(FILE_ATTRIBUTE_DIRECTORY);
	}

	bool is_not_empty_directory(const string& Object)
	{
		os::fs::enum_file Find(Object + L"\\*");
		return Find.begin() != Find.end();
	}

enum_file::enum_file(const string& Object, bool ScanSymLink):
	m_Object(NTPath(Object)),
	m_ScanSymLink(ScanSymLink)
{
	bool Root = false;
	const auto Type = ParsePath(m_Object, nullptr, &Root);
	if(Root && (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC || Type == PATH_VOLUMEGUID))
	{
		AddEndSlash(m_Object);
	}
	else
	{
		DeleteEndSlash(m_Object);
	}
}

bool enum_file::get(size_t index, value_type& FindData)
{
	bool Result = false;
	if (!index)
	{
		// temporary disable elevation to try "real" name first
		{
			SCOPED_ACTION(elevation::suppress);
			m_Handle = FindFirstFileInternal(m_Object, FindData);
		}

		if (!m_Handle && GetLastError() == ERROR_ACCESS_DENIED)
		{
			if(m_ScanSymLink)
			{
				string strReal(m_Object);
				// only links in path should be processed, not the object name itself
				CutToSlash(strReal);
				ConvertNameToReal(strReal, strReal);
				AddEndSlash(strReal);
				strReal+=PointToName(m_Object);
				strReal = NTPath(strReal);
				m_Handle = FindFirstFileInternal(strReal, FindData);
			}

			if (!m_Handle && ElevationRequired(ELEVATION_READ_REQUEST))
			{
				m_Handle = FindFirstFileInternal(m_Object, FindData);
			}
		}
		Result = m_Handle? true : false;
	}
	else
	{
		if (m_Handle)
		{
			Result = FindNextFileInternal(m_Handle, FindData);
		}
	}

	// skip ".." & "."
	if(Result && FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && FindData.strFileName[0] == L'.' &&
		((FindData.strFileName.size() == 2 && FindData.strFileName[1] == L'.') || FindData.strFileName.size() == 1) &&
		// хитрый способ - у виртуальных папок не бывает SFN, в отличие от. (UPD: или бывает, но такое же)
		(FindData.strAlternateFileName.empty() || FindData.strAlternateFileName == FindData.strFileName))
	{
		// index not important here, anything but 0 is fine
		Result = get(1, FindData);
	}
	return Result;
}

//-------------------------------------------------------------------------

bool enum_name::get(size_t index, value_type& value)
{
	if (!index)
	{
		m_Handle = FindFirstFileName(m_Object, 0, value);
		return m_Handle? true : false;
	}
	else
	{
		return FindNextFileName(m_Handle, value);
	}
}

//-------------------------------------------------------------------------

bool enum_stream::get(size_t index, value_type& value)
{
	if (!index)
	{
		m_Handle = FindFirstStream(m_Object, FindStreamInfoStandard, &value);
		return m_Handle ? true : false;
	}
	else
	{
		return FindNextStream(m_Handle, &value);
	}
}

//-------------------------------------------------------------------------

bool file::Open(const string& Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, file* TemplateFile, bool ForceElevation)
{
	assert(!Handle);

	Pointer = 0;
	NeedSyncPointer = false;

	HANDLE TemplateFileHandle = TemplateFile ? TemplateFile->Handle.native_handle() : nullptr;
	Handle = CreateFile(Object, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFileHandle, ForceElevation);
	if (Handle)
	{
		name = Object;
		share_mode = ShareMode;
	}
	else
	{
		name.clear();
		share_mode = 0;
	}
	return Handle? true : false;
}

inline void file::SyncPointer()
{
	if(NeedSyncPointer)
	{
		LARGE_INTEGER Distance, NewPointer;
		Distance.QuadPart = Pointer;
		if (SetFilePointerEx(Handle.native_handle(), Distance, &NewPointer, FILE_BEGIN))
		{
			Pointer = NewPointer.QuadPart;
			NeedSyncPointer = false;
		}
	}
}


bool file::Read(LPVOID Buffer, size_t NumberOfBytesToRead, size_t& NumberOfBytesRead, LPOVERLAPPED Overlapped)
{
	assert(NumberOfBytesToRead <= std::numeric_limits<DWORD>::max());

	SyncPointer();
	DWORD BytesRead = 0;
	bool Result = ReadFile(Handle.native_handle(), Buffer, static_cast<DWORD>(NumberOfBytesToRead), &BytesRead, Overlapped) != FALSE;
	NumberOfBytesRead = BytesRead;
	if(Result)
	{
		Pointer += NumberOfBytesRead;
	}
	return Result;
}

bool file::Write(LPCVOID Buffer, size_t NumberOfBytesToWrite, size_t& NumberOfBytesWritten, LPOVERLAPPED Overlapped)
{
	assert(NumberOfBytesToWrite <= std::numeric_limits<DWORD>::max());

	SyncPointer();
	DWORD BytesWritten = 0;
	bool Result = WriteFile(Handle.native_handle(), Buffer, static_cast<DWORD>(NumberOfBytesToWrite), &BytesWritten, Overlapped) != FALSE;
	NumberOfBytesWritten = BytesWritten;
	if(Result)
	{
		Pointer += NumberOfBytesWritten;
	}
	return Result;
}

bool file::SetPointer(int64_t DistanceToMove, uint64_t* NewFilePointer, DWORD MoveMethod)
{
	const auto OldPointer = Pointer;
	switch (MoveMethod)
	{
	case FILE_BEGIN:
		Pointer = DistanceToMove;
		break;
	case FILE_CURRENT:
		Pointer+=DistanceToMove;
		break;
	case FILE_END:
		{
			uint64_t Size = 0;
			GetSize(Size);
			Pointer = Size+DistanceToMove;
		}
		break;
	}
	if(OldPointer != Pointer)
	{
		NeedSyncPointer = true;
	}
	if(NewFilePointer)
	{
		*NewFilePointer = Pointer;
	}
	return true;
}

bool file::SetEnd()
{
	SyncPointer();
	bool ok = SetEndOfFile(Handle.native_handle()) != FALSE;
	if (!ok && !name.empty() && GetLastError() == ERROR_INVALID_PARAMETER) // OSX buggy SMB workaround
	{
		const auto fsize = GetPointer();
		Close();
		if (Open(name, GENERIC_WRITE, share_mode, nullptr, OPEN_EXISTING, 0))
		{
			SetPointer(fsize, nullptr, FILE_BEGIN);
			SyncPointer();
			ok = SetEndOfFile(Handle.native_handle()) != FALSE;
		}
	}
	return ok;
}

bool file::GetTime(LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime)
{
	return GetFileTimeEx(Handle.native_handle(), CreationTime, LastAccessTime, LastWriteTime, ChangeTime);
}

bool file::SetTime(const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime, const FILETIME* ChangeTime)
{
	return SetFileTimeEx(Handle.native_handle(), CreationTime, LastAccessTime, LastWriteTime, ChangeTime);
}

bool file::GetSize(UINT64& Size)
{
	return GetFileSizeEx(Handle.native_handle(), Size);
}

bool file::FlushBuffers()
{
	return FlushFileBuffers(Handle.native_handle()) != FALSE;
}

bool file::GetInformation(BY_HANDLE_FILE_INFORMATION& info)
{
	return GetFileInformationByHandle(Handle.native_handle(), &info) != FALSE;
}

bool file::IoControl(DWORD IoControlCode, LPVOID InBuffer, DWORD InBufferSize, LPVOID OutBuffer, DWORD OutBufferSize, LPDWORD BytesReturned, LPOVERLAPPED Overlapped)
{
	return ::DeviceIoControl(Handle.native_handle(), IoControlCode, InBuffer, InBufferSize, OutBuffer, OutBufferSize, BytesReturned, Overlapped) != FALSE;
}

bool file::GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed)
{
	DWORD Result = Imports().GetStorageDependencyInformation(Handle.native_handle(), Flags, StorageDependencyInfoSize, StorageDependencyInfo, SizeUsed);
	SetLastError(Result);
	return Result == ERROR_SUCCESS;
}

bool file::NtQueryDirectoryFile(PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, bool ReturnSingleEntry, LPCWSTR FileName, bool RestartScan, NTSTATUS* Status)
{
	IO_STATUS_BLOCK IoStatusBlock;
	PUNICODE_STRING pNameString = nullptr;
	UNICODE_STRING NameString;
	if(FileName && *FileName)
	{
		NameString.Buffer = const_cast<LPWSTR>(FileName);
		NameString.Length = static_cast<USHORT>(StrLength(FileName)*sizeof(WCHAR));
		NameString.MaximumLength = NameString.Length;
		pNameString = &NameString;
	}
	const auto di = reinterpret_cast<FILE_ID_BOTH_DIR_INFORMATION*>(FileInformation);
	di->NextEntryOffset = 0xffffffffUL;

	NTSTATUS Result = Imports().NtQueryDirectoryFile(Handle.native_handle(), nullptr, nullptr, nullptr, &IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, pNameString, RestartScan);
	SetLastError(Imports().RtlNtStatusToDosError(Result));
	if(Status)
	{
		*Status = Result;
	}

	return (Result == STATUS_SUCCESS) && (di->NextEntryOffset != 0xffffffffUL);
}

bool file::NtQueryInformationFile(PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, NTSTATUS* Status)
{
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS Result = Imports().NtQueryInformationFile(Handle.native_handle(), &IoStatusBlock, FileInformation, Length, FileInformationClass);
	SetLastError(Imports().RtlNtStatusToDosError(Result));
	if(Status)
	{
		*Status = Result;
	}
	return Result == STATUS_SUCCESS;
}

bool file::Close()
{
	return Handle.close();
}

bool file::Eof()
{
	const auto Ptr = GetPointer();
	uint64_t Size=0;
	GetSize(Size);
	return Ptr >= Size;
}
//-------------------------------------------------------------------------
file_walker::file_walker():
	FileSize(0),
	AllocSize(0),
	ProcessedSize(0),
	CurrentChunk(ChunkList.begin()),
	ChunkSize(0),
	Sparse(false)
{
}

bool file_walker::InitWalk(size_t BlockSize)
{
	bool Result = false;
	ChunkSize = static_cast<DWORD>(BlockSize);
	if(GetSize(FileSize) && FileSize)
	{
		BY_HANDLE_FILE_INFORMATION bhfi;
		Sparse = GetInformation(bhfi) && bhfi.dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE;

		if(Sparse)
		{
			FILE_ALLOCATED_RANGE_BUFFER QueryRange = {};
			QueryRange.Length.QuadPart = FileSize;
			static FILE_ALLOCATED_RANGE_BUFFER Ranges[1024];
			DWORD BytesReturned;
			for(;;)
			{
				const bool QueryResult = IoControl(FSCTL_QUERY_ALLOCATED_RANGES, &QueryRange, sizeof(QueryRange), Ranges, sizeof(Ranges), &BytesReturned);
				if((QueryResult || GetLastError() == ERROR_MORE_DATA) && BytesReturned)
				{
					FOR(const auto& i, make_range(Ranges, Ranges + BytesReturned / sizeof(*Ranges)))
					{
						AllocSize += i.Length.QuadPart;
						const UINT64 RangeEndOffset = i.FileOffset.QuadPart + i.Length.QuadPart;
						for(UINT64 j = i.FileOffset.QuadPart; j < RangeEndOffset; j+=ChunkSize)
						{
							ChunkList.emplace_back(Chunk(j, std::min(static_cast<DWORD>(RangeEndOffset - j), ChunkSize)));
						}
					}
					QueryRange.FileOffset.QuadPart = ChunkList.back().Offset+ChunkList.back().Size;
					QueryRange.Length.QuadPart = FileSize - QueryRange.FileOffset.QuadPart;
				}
				else
				{
					break;
				}
			}
			Result = !ChunkList.empty();
		}
		else
		{
			AllocSize = FileSize;
			ChunkList.emplace_back(Chunk(0, static_cast<DWORD>(std::min(static_cast<UINT64>(BlockSize), FileSize))));
			Result = true;
		}
		CurrentChunk = ChunkList.begin();
	}
	return Result;
}


bool file_walker::Step()
{
	bool Result = false;
	if(Sparse)
	{
		++CurrentChunk;
		if(CurrentChunk != ChunkList.end())
		{
			SetPointer(CurrentChunk->Offset, nullptr, FILE_BEGIN);
			ProcessedSize += CurrentChunk->Size;
			Result = true;
		}
	}
	else
	{
		UINT64 NewOffset = (!CurrentChunk->Size)? 0 : CurrentChunk->Offset + ChunkSize;
		if(NewOffset < FileSize)
		{
			CurrentChunk->Offset = NewOffset;
			UINT64 rest = FileSize - NewOffset;
			CurrentChunk->Size = (rest>=ChunkSize)?ChunkSize:rest;
			ProcessedSize += CurrentChunk->Size;
			Result = true;
		}
	}
	return Result;
}

UINT64 file_walker::GetChunkOffset() const
{
	return CurrentChunk->Offset;
}

DWORD file_walker::GetChunkSize() const
{
	return CurrentChunk->Size;
}

int file_walker::GetPercent() const
{
	return AllocSize? (ProcessedSize) * 100 / AllocSize : 0;
}

} // fs
//-------------------------------------------------------------------------

NTSTATUS GetLastNtStatus()
{
	return Imports().RtlGetLastNtStatus? Imports().RtlGetLastNtStatus() : STATUS_SUCCESS;
}

bool DeleteFile(const string& FileName)
{
	NTPath strNtName(FileName);
	bool Result = ::DeleteFile(strNtName.data()) != FALSE;
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Global->Elevation->fDeleteFile(strNtName);
	}

	if (!Result && !os::fs::exists(strNtName))
	{
		// Someone deleted it already,
		// but job is done, no need to report error.
		Result = true;
	}

	return Result;
}

bool RemoveDirectory(const string& DirName)
{
	NTPath strNtName(DirName);
	bool Result = ::RemoveDirectory(strNtName.data()) != FALSE;
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Global->Elevation->fRemoveDirectory(strNtName);
	}

	if (!Result && !os::fs::exists(strNtName))
	{
		// Someone deleted it already,
		// but job is done, no need to report error.
		Result = true;
	}

	return Result;
}

os::handle CreateFile(const string& Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile, bool ForceElevation)
{
	NTPath strObject(Object);
	FlagsAndAttributes|=FILE_FLAG_BACKUP_SEMANTICS|(CreationDistribution==OPEN_EXISTING?FILE_FLAG_POSIX_SEMANTICS:0);

	os::handle Handle(::CreateFile(strObject.data(), DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile));
	if(!Handle)
	{
		DWORD Error=::GetLastError();
		if(Error==ERROR_FILE_NOT_FOUND||Error==ERROR_PATH_NOT_FOUND)
		{
			FlagsAndAttributes&=~FILE_FLAG_POSIX_SEMANTICS;
			Handle.reset(::CreateFile(strObject.data(), DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile));
		}
		if (STATUS_STOPPED_ON_SYMLINK == GetLastNtStatus() && ERROR_STOPPED_ON_SYMLINK != GetLastError())
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	}

	if((!Handle && ElevationRequired(DesiredAccess&(GENERIC_ALL|GENERIC_WRITE|WRITE_OWNER|WRITE_DAC|DELETE|FILE_WRITE_DATA|FILE_ADD_FILE|FILE_APPEND_DATA|FILE_ADD_SUBDIRECTORY|FILE_CREATE_PIPE_INSTANCE|FILE_WRITE_EA|FILE_DELETE_CHILD|FILE_WRITE_ATTRIBUTES)?ELEVATION_MODIFY_REQUEST:ELEVATION_READ_REQUEST)) || ForceElevation)
	{
		Handle.reset(Global->Elevation->fCreateFile(strObject, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile));
	}
	return Handle;
}

bool CopyFileEx(
    const string& ExistingFileName,
    const string& NewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine,
    LPVOID lpData,
    LPBOOL pbCancel,
    DWORD dwCopyFlags
)
{
	NTPath strFrom(ExistingFileName), strTo(NewFileName);
	if(IsSlash(strTo.back()))
	{
		strTo += PointToName(strFrom);
	}
	bool Result = ::CopyFileEx(strFrom.data(), strTo.data(), lpProgressRoutine, lpData, pbCancel, dwCopyFlags) != FALSE;
	if(!Result)
	{
		if (STATUS_STOPPED_ON_SYMLINK == GetLastNtStatus() && ERROR_STOPPED_ON_SYMLINK != GetLastError())
			::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);

		else if (STATUS_FILE_IS_A_DIRECTORY == GetLastNtStatus())
			::SetLastError(ERROR_FILE_EXISTS);

		else if (ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
			Result = Global->Elevation->fCopyFileEx(strFrom, strTo, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	}
	return Result;
}

bool MoveFile(
    const string& ExistingFileName, // address of name of the existing file
    const string& NewFileName   // address of new name for the file
)
{
	NTPath strFrom(ExistingFileName), strTo(NewFileName);
	if(IsSlash(strTo.back()))
	{
		strTo += PointToName(strFrom);
	}
	bool Result = ::MoveFile(strFrom.data(), strTo.data()) != FALSE;

	if(!Result)
	{
		if (STATUS_STOPPED_ON_SYMLINK == GetLastNtStatus() && ERROR_STOPPED_ON_SYMLINK != GetLastError())
			::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);

		else if (ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
			Result = Global->Elevation->fMoveFileEx(strFrom, strTo, 0);
	}
	return Result;
}

bool MoveFileEx(
    const string& ExistingFileName, // address of name of the existing file
    const string& NewFileName,   // address of new name for the file
    DWORD dwFlags   // flag to determine how to move file
)
{
	NTPath strFrom(ExistingFileName), strTo(NewFileName);
	if(IsSlash(strTo.back()))
	{
		strTo += PointToName(strFrom);
	}
	bool Result = ::MoveFileEx(strFrom.data(), strTo.data(), dwFlags) != FALSE;
	if(!Result)
	{
		if (STATUS_STOPPED_ON_SYMLINK == GetLastNtStatus() && ERROR_STOPPED_ON_SYMLINK != GetLastError())
			::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);

		else if (ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
		{
			// exclude fake elevation request for: move file over existing directory with same name
			DWORD f = GetFileAttributes(strFrom);
			DWORD t = GetFileAttributes(strTo);

			if (f!=INVALID_FILE_ATTRIBUTES && t!=INVALID_FILE_ATTRIBUTES && 0==(f & FILE_ATTRIBUTE_DIRECTORY) && 0!=(t & FILE_ATTRIBUTE_DIRECTORY))
				::SetLastError(ERROR_FILE_EXISTS); // existing directory name == moved file name
			else
				Result = Global->Elevation->fMoveFileEx(strFrom, strTo, dwFlags);
		}
	}
	return Result;
}

string& strCurrentDirectory()
{
	static string strCurrentDirectory;
	return strCurrentDirectory;
}

void InitCurrentDirectory()
{
	//get real curdir:
	WCHAR Buffer[MAX_PATH];
	DWORD Size=::GetCurrentDirectory(ARRAYSIZE(Buffer),Buffer);
	if(Size)
	{
		string strInitCurDir;
		if(Size < ARRAYSIZE(Buffer))
		{
			strInitCurDir.assign(Buffer, Size);
		}
		else
		{
			std::vector<wchar_t> vBuffer(Size);
			::GetCurrentDirectory(Size, vBuffer.data());
			strInitCurDir.assign(vBuffer.data(), Size);
		}
		//set virtual curdir:
		SetCurrentDirectory(strInitCurDir);
	}
}

string GetCurrentDirectory()
{
	//never give outside world a direct pointer to our internal string
	//who knows what they gonna do
	return strCurrentDirectory();
}

bool SetCurrentDirectory(const string& PathName, bool Validate)
{
	// correct path to our standard
	string strDir=PathName;
	ReplaceSlashToBackslash(strDir);
	bool Root = false;
	const auto Type = ParsePath(strDir, nullptr, &Root);
	if(Root && (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC || Type == PATH_VOLUMEGUID))
	{
		AddEndSlash(strDir);
	}
	else
	{
		DeleteEndSlash(strDir);
	}

	if (strDir == strCurrentDirectory())
		return true;

	if (Validate)
	{
		string TestDir=PathName;
		AddEndSlash(TestDir);
		TestDir += L"*";
		FAR_FIND_DATA fd;
		if (!GetFindDataEx(TestDir, fd))
		{
			DWORD LastError = ::GetLastError();
			if(!(LastError == ERROR_FILE_NOT_FOUND || LastError == ERROR_NO_MORE_FILES))
				return false;
		}
	}

	strCurrentDirectory()=strDir;

#ifndef NO_WRAPPER
	// try to synchronize far cur dir with process cur dir
	if(Global->CtrlObject && Global->CtrlObject->Plugins->OemPluginsPresent())
	{
		::SetCurrentDirectory(strCurrentDirectory().data());
	}
#endif // NO_WRAPPER
	return true;
}

DWORD GetTempPath(string &strBuffer)
{
	WCHAR Buffer[MAX_PATH];
	DWORD Size = ::GetTempPath(ARRAYSIZE(Buffer), Buffer);
	if(Size)
	{
		if(Size < ARRAYSIZE(Buffer))
		{
			strBuffer.assign(Buffer, Size);
		}
		else
		{
			std::vector<wchar_t> vBuffer(Size);
			Size = ::GetTempPath(Size, vBuffer.data());
			strBuffer.assign(vBuffer.data(), Size);
		}
	}
	return Size;
};


DWORD GetModuleFileName(HMODULE hModule, string &strFileName)
{
	return GetModuleFileNameEx(nullptr, hModule, strFileName);
}

DWORD GetModuleFileNameEx(HANDLE hProcess, HMODULE hModule, string &strFileName)
{
	DWORD Size = 0;
	DWORD BufferSize = MAX_PATH;
	wchar_t_ptr FileName;

	do
	{
		BufferSize *= 2;
		FileName.reset(BufferSize);
		if (hProcess)
		{
			if (Imports().QueryFullProcessImageNameW && !hModule)
			{
				DWORD sz = BufferSize;
				Size = 0;
				if (Imports().QueryFullProcessImageNameW(hProcess, 0, FileName.get(), &sz))
				{
					Size = sz;
				}
			}
			else
			{
				Size = ::GetModuleFileNameEx(hProcess, hModule, FileName.get(), BufferSize);
			}
		}
		else
		{
			Size = ::GetModuleFileName(hModule, FileName.get(), BufferSize);
		}
	}
	while ((Size >= BufferSize) || (!Size && GetLastError() == ERROR_INSUFFICIENT_BUFFER));

	if (Size)
		strFileName.assign(FileName.get(), Size);

	return Size;
}

DWORD WNetGetConnection(const string& LocalName, string &RemoteName)
{
	wchar_t Buffer[MAX_PATH];
	DWORD Size = ARRAYSIZE(Buffer);
	auto Result = ::WNetGetConnection(LocalName.data(), Buffer, &Size);
	if (Result == NO_ERROR || Result == ERROR_NOT_CONNECTED || Result == ERROR_CONNECTION_UNAVAIL)
	{
		RemoteName = Buffer;
	}
	else if (Result == ERROR_MORE_DATA)
	{
		std::vector<wchar_t> vBuffer(Size);
		Result = ::WNetGetConnection(LocalName.data(), vBuffer.data(), &Size);
		RemoteName.assign(vBuffer.data(), Size - 1);
	}
	return Result;
}

bool GetVolumeInformation(
    const string& RootPathName,
    string *pVolumeName,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    string *pFileSystemName
)
{
	wchar_t_ptr VolumeNameBuffer, FileSystemNameBuffer;
	if (pVolumeName)
	{
		VolumeNameBuffer.reset(MAX_PATH + 1);
		VolumeNameBuffer[0] = L'\0';
	}
	if (pFileSystemName)
	{
		FileSystemNameBuffer.reset(MAX_PATH + 1);
		FileSystemNameBuffer[0] = L'\0';
	}
	bool bResult = ::GetVolumeInformation(RootPathName.data(), VolumeNameBuffer.get(), static_cast<DWORD>(VolumeNameBuffer.size()), lpVolumeSerialNumber,
		lpMaximumComponentLength, lpFileSystemFlags, FileSystemNameBuffer.get(), static_cast<DWORD>(FileSystemNameBuffer.size())) != FALSE;

	if (pVolumeName)
		*pVolumeName = VolumeNameBuffer.get();

	if (pFileSystemName)
		*pFileSystemName = FileSystemNameBuffer.get();

	return bResult;
}

bool GetFindDataEx(const string& FileName, FAR_FIND_DATA& FindData,bool ScanSymLink)
{
	fs::enum_file Find(FileName, ScanSymLink);
	auto ItemIterator = Find.begin();
	if(ItemIterator != Find.end())
	{
		FindData = std::move(*ItemIterator);
		return true;
	}
	else
	{
		size_t DirOffset = 0;
		ParsePath(FileName, &DirOffset);
		if (FileName.find_first_of(L"*?", DirOffset) == string::npos)
		{
			DWORD dwAttr=GetFileAttributes(FileName);

			if (dwAttr!=INVALID_FILE_ATTRIBUTES)
			{
				// Ага, значит файл таки есть. Заполним структуру ручками.
				FindData.Clear();
				FindData.dwFileAttributes=dwAttr;
				fs::file file;
				if(file.Open(FileName, FILE_READ_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING))
				{
					file.GetTime(&FindData.ftCreationTime,&FindData.ftLastAccessTime,&FindData.ftLastWriteTime,&FindData.ftChangeTime);
					file.GetSize(FindData.nFileSize);
					file.Close();
				}

				if (FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
				{
					string strTmp;
					GetReparsePointInfo(FileName,strTmp,&FindData.dwReserved0); //MSDN
				}
				else
				{
					FindData.dwReserved0=0;
				}

				FindData.strFileName=PointToName(FileName);
				ConvertNameToShort(FileName,FindData.strAlternateFileName);
				return true;
			}
		}
	}
	FindData.Clear();
	FindData.dwFileAttributes=INVALID_FILE_ATTRIBUTES; //BUGBUG

	return false;
}

bool GetFileSizeEx(HANDLE hFile, UINT64 &Size)
{
	bool Result=false;

	if (::GetFileSizeEx(hFile,reinterpret_cast<PLARGE_INTEGER>(&Size)))
	{
		Result=true;
	}
	else
	{
		GET_LENGTH_INFORMATION gli;
		DWORD BytesReturned;

		if (::DeviceIoControl(hFile,IOCTL_DISK_GET_LENGTH_INFO,nullptr,0,&gli,sizeof(gli),&BytesReturned,nullptr))
		{
			Size=gli.Length.QuadPart;
			Result=true;
		}
	}

	return Result;
}

bool IsDiskInDrive(const string& Root)
{
	string strVolName;
	DWORD  MaxComSize;
	DWORD  Flags;
	string strFS;
	auto strDrive = Root;
	AddEndSlash(strDrive);
	return GetVolumeInformation(strDrive, &strVolName, nullptr, &MaxComSize, &Flags, &strFS);
}

int GetFileTypeByName(const string& Name)
{
	if (const auto File = CreateFile(Name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0))
	{
		return ::GetFileType(File.native_handle());
	}
	return FILE_TYPE_UNKNOWN;
}

bool GetDiskSize(const string& Path,unsigned __int64 *TotalSize, unsigned __int64 *TotalFree, unsigned __int64 *UserFree)
{
	NTPath strPath(Path);
	AddEndSlash(strPath);

	ULARGE_INTEGER FreeBytesAvailableToCaller, TotalNumberOfBytes, TotalNumberOfFreeBytes;

	auto Result = GetDiskFreeSpaceEx(strPath.data(), &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes) != FALSE;
	if(!Result && ElevationRequired(ELEVATION_READ_REQUEST))
	{
		Result = Global->Elevation->fGetDiskFreeSpaceEx(strPath.data(), &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes);
	}

	if (Result)
	{
		if (TotalSize)
			*TotalSize = TotalNumberOfBytes.QuadPart;

		if (TotalFree)
			*TotalFree = TotalNumberOfFreeBytes.QuadPart;

		if (UserFree)
			*UserFree = FreeBytesAvailableToCaller.QuadPart;
	}
	return Result;
}

find_handle FindFirstFileName(const string& FileName, DWORD dwFlags, string& LinkName)
{
	HANDLE hRet=INVALID_HANDLE_VALUE;
	DWORD StringLength=0;
	NTPath NtFileName(FileName);
	if (Imports().FindFirstFileNameW(NtFileName.data(), 0, &StringLength, nullptr)==INVALID_HANDLE_VALUE && GetLastError()==ERROR_MORE_DATA)
	{
		wchar_t_ptr Buffer(StringLength);
		hRet=Imports().FindFirstFileNameW(NtFileName.data(), 0, &StringLength, Buffer.get());
		LinkName = Buffer.get();
	}
	return find_handle(hRet != INVALID_HANDLE_VALUE? new detail::os_find_handle_impl(hRet) : nullptr);
}

bool FindNextFileName(const find_handle& hFindStream, string& LinkName)
{
	bool Ret = false;
	DWORD StringLength=0;
	const auto Handle = static_cast<detail::os_find_handle_impl*>(hFindStream.native_handle())->hative_handle();
	if (!Imports().FindNextFileNameW(Handle, &StringLength, nullptr) && GetLastError() == ERROR_MORE_DATA)
	{
		wchar_t_ptr Buffer(StringLength);
		Ret = Imports().FindNextFileNameW(Handle, &StringLength, Buffer.get()) != FALSE;
		LinkName = Buffer.get();
	}
	return Ret;
}

bool CreateDirectory(const string& PathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return CreateDirectoryEx(L"", PathName, lpSecurityAttributes);
}

bool CreateDirectoryEx(const string& TemplateDirectory, const string& NewDirectory, LPSECURITY_ATTRIBUTES SecurityAttributes)
{
	NTPath NtTemplateDirectory(TemplateDirectory);
	NTPath NtNewDirectory(NewDirectory);
	BOOL Result = FALSE;
	for(size_t i = 0; i < 2 && !Result; ++i)
	{
		Result = NtTemplateDirectory.empty()? ::CreateDirectory(NtNewDirectory.data(), SecurityAttributes) : ::CreateDirectoryEx(NtTemplateDirectory.data(), NtNewDirectory.data(), SecurityAttributes);
		if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
		{
			Result = Global->Elevation->fCreateDirectoryEx(NtTemplateDirectory, NtNewDirectory, SecurityAttributes);
		}
		if(!Result)
		{
			// CreateDirectoryEx may fail on some FS, try to create anyway.
			NtTemplateDirectory.clear();
		}
	}
	return Result != FALSE;
}

DWORD GetFileAttributes(const string& FileName)
{
	NTPath NtName(FileName);
	DWORD Result = ::GetFileAttributes(NtName.data());
	if(Result == INVALID_FILE_ATTRIBUTES && ElevationRequired(ELEVATION_READ_REQUEST))
	{
		Result = Global->Elevation->fGetFileAttributes(NtName);
	}
	return Result;
}

bool SetFileAttributes(const string& FileName, DWORD dwFileAttributes)
{
	NTPath NtName(FileName);
	bool Result = ::SetFileAttributes(NtName.data(), dwFileAttributes) != FALSE;
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Global->Elevation->fSetFileAttributes(NtName, dwFileAttributes);
	}
	return Result;

}

bool CreateSymbolicLinkInternal(const string& Object, const string& Target, DWORD dwFlags)
{
	return Imports().CreateSymbolicLinkW?
		(Imports().CreateSymbolicLink(Object.data(), Target.data(), dwFlags) != FALSE) :
		CreateReparsePoint(Target, Object, dwFlags&SYMBOLIC_LINK_FLAG_DIRECTORY?RP_SYMLINKDIR:RP_SYMLINKFILE);
}

bool CreateSymbolicLink(const string& SymlinkFileName, const string& TargetFileName,DWORD dwFlags)
{
	NTPath NtSymlinkFileName(SymlinkFileName);
	bool Result = CreateSymbolicLinkInternal(NtSymlinkFileName, TargetFileName, dwFlags);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result=Global->Elevation->fCreateSymbolicLink(NtSymlinkFileName, TargetFileName, dwFlags);
	}
	return Result;
}

bool SetFileEncryptionInternal(const wchar_t* Name, bool Encrypt)
{
	return Encrypt? EncryptFile(Name)!=FALSE : DecryptFile(Name, 0)!=FALSE;
}

bool SetFileEncryption(const string& Name, bool Encrypt)
{
	NTPath NtName(Name);
	bool Result = SetFileEncryptionInternal(NtName.data(), Encrypt);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST, false)) // Encryption implemented in adv32, NtStatus not affected
	{
		Result=Global->Elevation->fSetFileEncryption(NtName, Encrypt);
	}
	return Result;
}

bool CreateHardLinkInternal(const string& Object, const string& Target,LPSECURITY_ATTRIBUTES SecurityAttributes)
{
	bool Result = ::CreateHardLink(Object.data(), Target.data(), SecurityAttributes) != FALSE;
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Global->Elevation->fCreateHardLink(Object, Target, SecurityAttributes);
	}
	return Result;
}

bool CreateHardLink(const string& FileName, const string& ExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	bool Result = CreateHardLinkInternal(NTPath(FileName),NTPath(ExistingFileName), lpSecurityAttributes) != FALSE;
	//bug in win2k: \\?\ fails
	if (!Result && !IsWindowsXPOrGreater())
	{
		Result = CreateHardLinkInternal(FileName, ExistingFileName, lpSecurityAttributes);
	}
	return Result;
}

find_handle FindFirstStream(const string& FileName,STREAM_INFO_LEVELS InfoLevel,LPVOID lpFindStreamData,DWORD dwFlags)
{
	find_handle Ret;
	if(Imports().FindFirstStreamW)
	{
		const auto Handle = Imports().FindFirstStreamW(NTPath(FileName).data(), InfoLevel, lpFindStreamData, dwFlags);
		if (Handle != INVALID_HANDLE_VALUE)
		{
			Ret.reset(new detail::os_find_handle_impl(Handle));
		}
	}
	else
	{
		if (InfoLevel==FindStreamInfoStandard)
		{
				auto Handle = std::make_unique<detail::far_find_handle_impl>();

				if (Handle->Object.Open(FileName, 0, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING))
				{
					// for network paths buffer size must be <= 64k
					// we double it in a first loop, so starting value is 32k
					Handle->BufferSize = 0x8000;
					NTSTATUS Result = STATUS_SEVERITY_ERROR;
					do
					{
						Handle->BufferSize *= 2;
						Handle->BufferBase.reset(Handle->BufferSize);
						if (Handle->BufferBase)
						{
							// sometimes for directories NtQueryInformationFile returns STATUS_SUCCESS but doesn't fill the buffer
							const auto StreamInfo = reinterpret_cast<FILE_STREAM_INFORMATION*>(Handle->BufferBase.get());
							StreamInfo->StreamNameLength = 0;
							Handle->Object.NtQueryInformationFile(Handle->BufferBase.get(), Handle->BufferSize, FileStreamInformation, &Result);
						}
					}
					while(Result == STATUS_BUFFER_OVERFLOW || Result == STATUS_BUFFER_TOO_SMALL);

					if (Result == STATUS_SUCCESS)
					{
						const auto pFsd = static_cast<WIN32_FIND_STREAM_DATA*>(lpFindStreamData);
						const auto StreamInfo = reinterpret_cast<const FILE_STREAM_INFORMATION*>(Handle->BufferBase.get());
						Handle->NextOffset = StreamInfo->NextEntryOffset;
						if (StreamInfo->StreamNameLength)
						{
							std::copy_n(std::cbegin(StreamInfo->StreamName), StreamInfo->StreamNameLength/sizeof(wchar_t), pFsd->cStreamName);
							pFsd->cStreamName[StreamInfo->StreamNameLength / sizeof(wchar_t)] = L'\0';
							pFsd->StreamSize=StreamInfo->StreamSize;
							Ret.reset(Handle.release());
						}
					}
				}
		}
	}

	return Ret;
}

bool FindNextStream(const find_handle& hFindStream,LPVOID lpFindStreamData)
{
	bool Ret=FALSE;
	if(Imports().FindFirstStreamW)
	{
		Ret = Imports().FindNextStreamW(static_cast<detail::os_find_handle_impl*>(hFindStream.native_handle())->hative_handle(), lpFindStreamData) != FALSE;
	}
	else
	{
		const auto Handle = static_cast<detail::far_find_handle_impl*>(hFindStream.native_handle());

		if (Handle->NextOffset)
		{
			const auto pStreamInfo = reinterpret_cast<const FILE_STREAM_INFORMATION*>(Handle->BufferBase.get() + Handle->NextOffset);
			const auto pFsd = static_cast<WIN32_FIND_STREAM_DATA*>(lpFindStreamData);
			Handle->NextOffset = pStreamInfo->NextEntryOffset?Handle->NextOffset+pStreamInfo->NextEntryOffset:0;
			if (pStreamInfo->StreamNameLength && pStreamInfo->StreamNameLength < sizeof(pFsd->cStreamName))
			{
				std::copy_n(std::cbegin(pStreamInfo->StreamName), pStreamInfo->StreamNameLength / sizeof(wchar_t), pFsd->cStreamName);
				pFsd->cStreamName[pStreamInfo->StreamNameLength / sizeof(wchar_t)] = L'\0';
				pFsd->StreamSize=pStreamInfo->StreamSize;
				Ret = true;
			}
		}
	}

	return Ret;
}

std::vector<string> GetLogicalDriveStrings()
{
	FN_RETURN_TYPE(GetLogicalDriveStrings) Result;
	wchar_t Buffer[MAX_PATH];
	DWORD Size = ::GetLogicalDriveStrings(ARRAYSIZE(Buffer), Buffer);

	if (Size)
	{
		const wchar_t* Ptr;
		wchar_t_ptr vBuffer;

		if (Size < ARRAYSIZE(Buffer))
		{
			Ptr = Buffer;
		}
		else
		{
			vBuffer.reset(Size);
			Size = ::GetLogicalDriveStrings(Size, vBuffer.get());
			Ptr = vBuffer.get();
		}

		FOR(const auto& i, os::enum_strings(Ptr))
		{
			Result.emplace_back(string(i.data(), i.size()));
		}
	}
	return Result;
}

bool internalNtQueryGetFinalPathNameByHandle(HANDLE hFile, string& FinalFilePath)
{
	ULONG RetLen;
	NTSTATUS Res = STATUS_SUCCESS;
	string NtPath;

	{
		ULONG BufSize = NT_MAX_PATH;
		block_ptr<OBJECT_NAME_INFORMATION> oni(BufSize);
		Res = Imports().NtQueryObject(hFile, ObjectNameInformation, oni.get(), BufSize, &RetLen);

		if (Res == STATUS_BUFFER_OVERFLOW || Res == STATUS_BUFFER_TOO_SMALL)
		{
			oni.reset(BufSize = RetLen);
			Res = Imports().NtQueryObject(hFile, ObjectNameInformation, oni.get(), BufSize, &RetLen);
		}

		if (Res == STATUS_SUCCESS)
		{
			NtPath.assign(oni->Name.Buffer, oni->Name.Length / sizeof(WCHAR));
		}
	}

	FinalFilePath.clear();

	if (Res == STATUS_SUCCESS)
	{
		// simple way to handle network paths
		if (NtPath.compare(0, 24, L"\\Device\\LanmanRedirector") == 0)
			FinalFilePath = NtPath.replace(0, 24, 1, L'\\');

		if (FinalFilePath.empty())
		{
			// try to convert NT path (\Device\HarddiskVolume1) to drive letter
			const auto Strings = GetLogicalDriveStrings();
			std::any_of(CONST_RANGE(Strings, i) -> bool
			{
				int Len = MatchNtPathRoot(NtPath, i);

				if (Len)
				{
					if (NtPath.compare(0, 14, L"\\Device\\WinDfs") == 0)
						FinalFilePath = NtPath.replace(0, Len, 1, L'\\');
					else
						FinalFilePath = NtPath.replace(0, Len, i);
					return true;
				}
				return false;
			});
		}

		if (FinalFilePath.empty())
		{
			// try to convert NT path (\Device\HarddiskVolume1) to \\?\Volume{...} path
			wchar_t VolumeName[MAX_PATH];
			HANDLE hEnum = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));
			BOOL Result = hEnum != INVALID_HANDLE_VALUE;

			while (Result)
			{
				DeleteEndSlash(VolumeName);
				int Len = MatchNtPathRoot(NtPath, VolumeName + 4 /* w/o prefix */);

				if (Len)
				{
					FinalFilePath = NtPath.replace(0, Len, VolumeName);
					break;
				}

				Result = FindNextVolumeW(hEnum, VolumeName, ARRAYSIZE(VolumeName));
			}

			if (hEnum != INVALID_HANDLE_VALUE)
				FindVolumeClose(hEnum);
		}
	}

	return !FinalFilePath.empty();
}

bool GetFinalPathNameByHandle(HANDLE hFile, string& FinalFilePath)
{
	if (Imports().GetFinalPathNameByHandleW)
	{
		const auto GetFinalPathNameByHandleGuarded = [](HANDLE File, wchar_t* Buffer, DWORD Size, DWORD Flags) -> DWORD
		{
			try
			{
				// It seems that Microsoft has forgotten to put an exception handler around this function.
				// It causes EXCEPTION_ACCESS_VIOLATION (read from 0) in kernel32 under certain conditions,
				// e.g. badly written file system drivers or weirdly formatted volumes.
				return Imports().GetFinalPathNameByHandle(File, Buffer, Size, Flags);
			}
			catch (const SException& e)
			{
				SetLastError(ERROR_UNHANDLED_EXCEPTION);
				return 0;
			}
		};

		wchar_t Buffer[MAX_PATH];
		if (size_t Size = GetFinalPathNameByHandleGuarded(hFile, Buffer, ARRAYSIZE(Buffer), VOLUME_NAME_GUID))
		{
			if (Size < ARRAYSIZE(Buffer))
			{
				FinalFilePath.assign(Buffer, Size);
			}
			else
			{
				wchar_t_ptr vBuffer(Size);
				Size = GetFinalPathNameByHandleGuarded(hFile, vBuffer.get(), static_cast<DWORD>(vBuffer.size()), VOLUME_NAME_GUID);
				FinalFilePath.assign(vBuffer.get(), Size);
			}
			return Size != 0;
		}
	}

	return internalNtQueryGetFinalPathNameByHandle(hFile, FinalFilePath);
}

bool SearchPath(const wchar_t *Path, const string& FileName, const wchar_t *Extension, string &strDest)
{
	DWORD dwSize = ::SearchPath(Path,FileName.data(),Extension,0,nullptr,nullptr);

	if (dwSize)
	{
		wchar_t_ptr Buffer(dwSize);
		dwSize = ::SearchPath(Path, FileName.data(), Extension, dwSize, Buffer.get(), nullptr);
		strDest.assign(Buffer.get(), dwSize);
		return true;
	}

	return false;
}

bool QueryDosDevice(const string& DeviceName, string &Path)
{
	SetLastError(NO_ERROR);
	wchar_t Buffer[MAX_PATH];
	DWORD Size = ::QueryDosDevice(DeviceName.data(), Buffer, ARRAYSIZE(Buffer));
	if (Size)
	{
		Path = Buffer; // don't copy trailing '\0'
	}
	else if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		wchar_t_ptr vBuffer(NT_MAX_PATH);
		SetLastError(NO_ERROR);
		Size = ::QueryDosDevice(DeviceName.data(), vBuffer.get(), static_cast<DWORD>(vBuffer.size()));
		if (Size)
		{
			Path = vBuffer.get();
		}
	}

	return Size && ::GetLastError() == NO_ERROR;
}

bool GetVolumeNameForVolumeMountPoint(const string& VolumeMountPoint,string& VolumeName)
{
	bool Result=false;
	WCHAR VolumeNameBuffer[50];
	NTPath strVolumeMountPoint(VolumeMountPoint);
	AddEndSlash(strVolumeMountPoint);
	if(::GetVolumeNameForVolumeMountPoint(strVolumeMountPoint.data(),VolumeNameBuffer,ARRAYSIZE(VolumeNameBuffer)))
	{
		VolumeName=VolumeNameBuffer;
		Result=true;
	}
	return Result;
}

void EnableLowFragmentationHeap()
{
	if (Imports().HeapSetInformation)
	{
		std::vector<HANDLE> Heaps(10);
		DWORD ActualNumHeaps = ::GetProcessHeaps(static_cast<DWORD>(Heaps.size()), Heaps.data());
		if(ActualNumHeaps > Heaps.size())
		{
			Heaps.resize(ActualNumHeaps);
			ActualNumHeaps = ::GetProcessHeaps(static_cast<DWORD>(Heaps.size()), Heaps.data());
		}
		Heaps.resize(ActualNumHeaps);
		std::for_each(CONST_RANGE(Heaps, i)
		{
			ULONG HeapFragValue = 2;
			Imports().HeapSetInformation(i, HeapCompatibilityInformation, &HeapFragValue, sizeof(HeapFragValue));
		});
	}
}

bool GetFileTimeSimple(const string &FileName, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime)
{
	fs::file dir;
	if (dir.Open(FileName,FILE_READ_ATTRIBUTES,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING))
	{
		return dir.GetTime(CreationTime,LastAccessTime,LastWriteTime,ChangeTime);
	}
	return false;
}

bool GetFileTimeEx(HANDLE Object, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime)
{
	bool Result = false;
	const ULONG Length = 40;
	BYTE Buffer[Length] = {};
	const auto fbi = reinterpret_cast<FILE_BASIC_INFORMATION*>(Buffer);
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS Status = Imports().NtQueryInformationFile(Object, &IoStatusBlock, fbi, Length, FileBasicInformation);
	::SetLastError(Imports().RtlNtStatusToDosError(Status));
	if (Status == STATUS_SUCCESS)
	{
		if(CreationTime)
		{
			*CreationTime = UI64ToFileTime(fbi->CreationTime.QuadPart);
		}
		if(LastAccessTime)
		{
			*LastAccessTime = UI64ToFileTime(fbi->LastAccessTime.QuadPart);
		}
		if(LastWriteTime)
		{
			*LastWriteTime = UI64ToFileTime(fbi->LastWriteTime.QuadPart);
		}
		if(ChangeTime)
		{
			*ChangeTime = UI64ToFileTime(fbi->ChangeTime.QuadPart);
		}
		Result = true;
	}
	return Result;
}

bool SetFileTimeEx(HANDLE Object, const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime, const FILETIME* ChangeTime)
{
	const ULONG Length = 40;
	BYTE Buffer[Length] = {};
	const auto fbi = reinterpret_cast<FILE_BASIC_INFORMATION*>(Buffer);
	if(CreationTime)
	{
		fbi->CreationTime.QuadPart = FileTimeToUI64(*CreationTime);
	}
	if(LastAccessTime)
	{
		fbi->LastAccessTime.QuadPart = FileTimeToUI64(*LastAccessTime);
	}
	if(LastWriteTime)
	{
		fbi->LastWriteTime.QuadPart = FileTimeToUI64(*LastWriteTime);
	}
	if(ChangeTime)
	{
		fbi->ChangeTime.QuadPart = FileTimeToUI64(*ChangeTime);
	}
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS Status = Imports().NtSetInformationFile(Object, &IoStatusBlock, fbi, Length, FileBasicInformation);
	::SetLastError(Imports().RtlNtStatusToDosError(Status));
	return Status == STATUS_SUCCESS;
}

bool GetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, FAR_SECURITY_DESCRIPTOR& SecurityDescriptor)
{
	bool Result = false;
	NTPath NtObject(Object);
	DWORD LengthNeeded = 0;
	::GetFileSecurity(NtObject.data(), RequestedInformation, nullptr, 0, &LengthNeeded);
	if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		SecurityDescriptor.reset(LengthNeeded);
		Result = ::GetFileSecurity(NtObject.data(), RequestedInformation, SecurityDescriptor.get(), LengthNeeded, &LengthNeeded) != FALSE;
	}
	return Result;
}

bool SetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, const FAR_SECURITY_DESCRIPTOR& SecurityDescriptor)
{
	return ::SetFileSecurity(NTPath(Object).data(), RequestedInformation, SecurityDescriptor.get()) != FALSE;
}

bool DetachVirtualDiskInternal(const string& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType)
{
	HANDLE Handle;
	DWORD Result = Imports().OpenVirtualDisk(&VirtualStorageType, Object.data(), VIRTUAL_DISK_ACCESS_DETACH, OPEN_VIRTUAL_DISK_FLAG_NONE, nullptr, &Handle);
	if (Result == ERROR_SUCCESS)
	{
		Result = Imports().DetachVirtualDisk(Handle, DETACH_VIRTUAL_DISK_FLAG_NONE, 0);
		if (Result != ERROR_SUCCESS)
		{
			SetLastError(Result);
		}
	}
	else
	{
		SetLastError(Result);
	}
	return Result == ERROR_SUCCESS;
}

bool DetachVirtualDisk(const string& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType)
{
	NTPath NtObject(Object);
	bool Result = DetachVirtualDiskInternal(NtObject, VirtualStorageType);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Global->Elevation->fDetachVirtualDisk(NtObject, VirtualStorageType);
	}
	return Result;
}

string GetPrivateProfileString(const string& AppName, const string& KeyName, const string& Default, const string& FileName)
{
	wchar_t_ptr Buffer(NT_MAX_PATH);
	DWORD size = ::GetPrivateProfileString(AppName.data(), KeyName.data(), Default.data(), Buffer.get(), static_cast<DWORD>(Buffer.size()), FileName.data());
	return string(Buffer.get(), size);
}

DWORD GetAppPathsRedirectionFlag()
{
	const auto GetFlag = []() -> DWORD
	{
		// App Paths key is shared in Windows 7 and above
		if (!IsWindows7OrGreater())
		{
#ifdef _WIN64
			return KEY_WOW64_32KEY;
#else
			BOOL Wow64Process = FALSE;
			if (Imports().IsWow64Process(GetCurrentProcess(), &Wow64Process) && Wow64Process)
			{
				return KEY_WOW64_64KEY;
			}
#endif
		}
		return 0;
	};

	static const auto RedirectionFlag = GetFlag();
	return RedirectionFlag;
}

	namespace reg
	{
		key open_key(HKEY RootKey, const wchar_t* SubKey, DWORD SamDesired)
		{
			HKEY Result;
			return key(RegOpenKeyEx(RootKey, SubKey, 0, SamDesired, &Result) == ERROR_SUCCESS? Result : nullptr);
		}

		static bool QueryValue(const key& Key, const wchar_t* Name, DWORD* Type, void* Data, size_t* Size)
		{
			DWORD dwSize = Size? static_cast<DWORD>(*Size) : 0;
			const auto Result = RegQueryValueEx(Key.get(), Name, nullptr, Type, reinterpret_cast<LPBYTE>(Data), Size? &dwSize : nullptr);
			if (Size)
			{
				*Size = dwSize;
			}
			return Result == ERROR_SUCCESS;
		}

		static bool QueryValue(const key& Key, const wchar_t* Name, DWORD& Type, std::vector<char>& Value)
		{
			bool Result = false;
			size_t Size = 0;
			if (QueryValue(Key, Name, nullptr, nullptr, &Size))
			{
				Value.resize(Size);
				Result = QueryValue(Key, Name, &Type, Value.data(), &Size);
			}
			return Result;
		}

		bool EnumKey(const key& Key, size_t Index, string& Name)
		{
			LONG ExitCode = ERROR_MORE_DATA;

			for (DWORD Size = 512; ExitCode == ERROR_MORE_DATA; Size *= 2)
			{
				wchar_t_ptr Buffer(Size);
				DWORD RetSize = Size;
				ExitCode = RegEnumKeyEx(Key.get(), static_cast<DWORD>(Index), Buffer.get(), &RetSize, nullptr, nullptr, nullptr, nullptr);
				if (ExitCode == ERROR_SUCCESS)
				{
					Name.assign(Buffer.get(), RetSize);
				}
			}
			return ExitCode == ERROR_SUCCESS;
		}

		bool EnumValue(const key& Key, size_t Index, value& Value)
		{
			LONG ExitCode = ERROR_MORE_DATA;

			for (DWORD Size = 512; ExitCode == ERROR_MORE_DATA; Size *= 2)
			{
				wchar_t_ptr Buffer(Size);
				DWORD RetSize = Size;
				ExitCode = RegEnumValue(Key.get(), static_cast<DWORD>(Index), Buffer.get(), &RetSize, nullptr, &Value.m_Type, nullptr, nullptr);
				if (ExitCode == ERROR_SUCCESS)
				{
					Value.m_Name.assign(Buffer.get(), RetSize);
					Value.m_Key = &Key;
				}
			}

			return ExitCode == ERROR_SUCCESS;
		}

		bool GetValue(const key& Key, const wchar_t* Name)
		{
			return QueryValue(Key, Name, nullptr, nullptr, nullptr);
		}

		bool GetValue(const key& Key, const wchar_t* Name, string& Value)
		{
			bool Result = false;
			std::vector<char> Buffer;
			DWORD Type;
			if (QueryValue(Key, Name, Type, Buffer) && detail::IsStringType(Type))
			{
				Value = string(reinterpret_cast<const wchar_t*>(Buffer.data()), Buffer.size() / sizeof(wchar_t));
				if (!Value.empty() && Value.back() == L'\0')
				{
					Value.pop_back();
				}
				Result = true;
			}
			return Result;
		}

		bool GetValue(const key& Key, const wchar_t* Name, unsigned int& Value)
		{
			bool Result = false;
			std::vector<char> Buffer;
			DWORD Type;
			if (QueryValue(Key, Name, Type, Buffer) && Type == REG_DWORD)
			{
				Value = 0;
				memcpy(&Value, Buffer.data(), std::min(Buffer.size(), sizeof(Value)));
				Result = true;
			}
			return Result;
		}

		bool GetValue(const key& Key, const wchar_t* Name, uint64_t& Value)
		{
			bool Result = false;
			std::vector<char> Buffer;
			DWORD Type;
			if (QueryValue(Key, Name, Type, Buffer) && Type == REG_QWORD)
			{
				Value = 0;
				memcpy(&Value, Buffer.data(), std::min(Buffer.size(), sizeof(Value)));
				Result = true;
			}
			return Result;
		}

		string value::GetString() const
		{
			if (!detail::IsStringType(m_Type))
				throw std::runtime_error("bad value type");

			string Result;
			GetValue(*m_Key, m_Name.data(), Result);
			return Result;
		}

		unsigned int value::GetUnsigned() const
		{
			if (m_Type != REG_DWORD)
				throw std::runtime_error("bad value type");

			unsigned int Result = 0;
			GetValue(*m_Key, m_Name.data(), Result);
			return Result;
		}

		uint64_t value::GetUnsigned64() const
		{
			if (m_Type != REG_QWORD)
				throw std::runtime_error("bad value type");

			uint64_t Result = 0;
			GetValue(*m_Key, m_Name.data(), Result);
			return Result;
		}
	}


	namespace env
	{
		provider::strings::strings()
		{
			m_Environment = GetEnvironmentStrings();
		}

		provider::strings::~strings()
		{
			FreeEnvironmentStrings(*this);
		}

		provider::block::block()
		{
			m_Environment = nullptr;
			HANDLE RawHandle = nullptr;
			if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &RawHandle))
			{
				os::handle TokenHandle(RawHandle);
				CreateEnvironmentBlock(reinterpret_cast<void**>(&m_Environment), TokenHandle.native_handle(), TRUE);
			}
		}

		provider::block::~block()
		{
			DestroyEnvironmentBlock(*this);
		}

		std::pair<string, string> split(const wchar_t* Line)
		{
			const auto EqPos = wcschr(Line + 1, L'=');
			return std::make_pair(string(Line, EqPos - Line), string(EqPos + 1));
		}

		bool get_variable(const wchar_t* Name, string& strBuffer)
		{
			WCHAR Buffer[MAX_PATH];
			// GetEnvironmentVariable doesn't change error code on success
			SetLastError(ERROR_SUCCESS);
			DWORD Size = ::GetEnvironmentVariable(Name, Buffer, ARRAYSIZE(Buffer));
			const auto LastError = GetLastError();

			if (Size)
			{
				if (Size < ARRAYSIZE(Buffer))
				{
					strBuffer.assign(Buffer, Size);
				}
				else
				{
					std::vector<wchar_t> vBuffer(Size);
					Size = ::GetEnvironmentVariable(Name, vBuffer.data(), Size);
					strBuffer.assign(vBuffer.data(), Size);
				}
			}

			return LastError != ERROR_ENVVAR_NOT_FOUND;
		}

		bool set_variable(const wchar_t* Name, const wchar_t* Value)
		{
			return ::SetEnvironmentVariable(Name, Value) != FALSE;
		}

		bool delete_variable(const wchar_t* Name)
		{
			return ::SetEnvironmentVariable(Name, nullptr) != FALSE;
		}

		string expand_strings(const wchar_t* str)
		{
			WCHAR Buffer[MAX_PATH];
			DWORD Size = ::ExpandEnvironmentStrings(str, Buffer, ARRAYSIZE(Buffer));
			if (Size)
			{
				if (Size <= ARRAYSIZE(Buffer))
				{
					return string(Buffer, Size - 1);
				}
				else
				{
					std::vector<wchar_t> vBuffer(Size);
					Size = ::ExpandEnvironmentStrings(str, vBuffer.data(), Size);
					return string(vBuffer.data(), Size - 1);
				}
			}
			return str;
		}

		string get_pathext()
		{
			auto PathExt(os::env::get_variable(L"PATHEXT"));
			if (PathExt.empty())
				PathExt = L".COM;.EXE;.BAT;.CMD;.VBS;.JS;.JSE;.WSF;.WSH;.MSC";
			return PathExt;
		}
	}

	namespace rtdl
	{
		module::~module()
		{
			if (m_loaded)
			{
				FreeLibrary(m_module);
			}
		}

		HMODULE module::get_module() const
		{
			if (!m_tried && !m_module)
			{
				m_tried = true;
				m_module = LoadLibrary(m_name);

				if (!m_module && m_AlternativeLoad && IsAbsolutePath(m_name))
				{
					m_module = LoadLibraryEx(m_name, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
				}
				// TODO: log if nullptr
				m_loaded = m_module != nullptr;
			}
			return m_module;
		}
	}

	namespace memory
	{
		bool is_pointer(const void* Address)
		{
			const auto GetInfo = []() -> SYSTEM_INFO { SYSTEM_INFO Info; GetSystemInfo(&Info); return Info; };
			static const auto info = GetInfo();

			return InRange<const void*>(info.lpMinimumApplicationAddress, Address, info.lpMaximumApplicationAddress);
		}
	}

	namespace security
	{
		bool is_admin()
		{
			const auto GetResult = []() -> bool
			{
				SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
				try
				{
					os::sid_object AdministratorsGroup(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS);
					BOOL IsMember = FALSE;
					if (CheckTokenMembership(nullptr, AdministratorsGroup.get(), &IsMember) && IsMember)
					{
						return true;
					}
				}
				catch (const FarRecoverableException&)
				{
					// TODO: Log
				}
				return false;
			};

			static const auto Result = GetResult();
			return Result;
		}
	}
}
