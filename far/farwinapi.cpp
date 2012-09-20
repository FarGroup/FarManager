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
#include "mix.hpp"
#include "ctrlobj.hpp"
#include "elevation.hpp"
#include "config.hpp"

struct PSEUDO_HANDLE
{
	HANDLE ObjectHandle;
	PVOID BufferBase;
	ULONG NextOffset;
	ULONG BufferSize;
   PVOID Buffer2;
	bool Extended;
	bool ReadDone;
};

HANDLE FindFirstFileInternal(const string& Name, FAR_FIND_DATA_EX& FindData)
{
	HANDLE Result = INVALID_HANDLE_VALUE;
	if(!Name.IsEmpty() && !IsSlash(Name.At(Name.GetLength()-1)))
	{
		PSEUDO_HANDLE* Handle = new PSEUDO_HANDLE;
		if(Handle)
		{
			string strDirectory(Name);
			CutToSlash(strDirectory);
			File* Directory = new File;
			if(Directory)
			{
				if(Directory->Open(strDirectory, FILE_LIST_DIRECTORY, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
				{
					Handle->ObjectHandle =static_cast<HANDLE>(Directory);

					// for network paths buffer size must be <= 65k
					Handle->BufferSize = 0x10000;
					Handle->BufferBase = xf_malloc(Handle->BufferSize);
					if (Handle->BufferBase)
					{
						LPCWSTR NamePtr = PointToName(Name);
						Handle->Extended = true;
						Handle->Buffer2 = nullptr;
						Handle->ReadDone = false;

						bool QueryResult = Directory->NtQueryDirectoryFile(Handle->BufferBase, Handle->BufferSize, FileIdBothDirectoryInformation, FALSE, NamePtr, TRUE);
						if (QueryResult) // try next read immediately to avoid M#2128 bug
						{
							Handle->Buffer2 = xf_malloc(Handle->BufferSize);
							if (!Handle->Buffer2)
								QueryResult = false;
							else
							{
								bool QueryResult2 = Directory->NtQueryDirectoryFile(Handle->Buffer2, Handle->BufferSize, FileIdBothDirectoryInformation, FALSE, NamePtr, FALSE);
								if (!QueryResult2)
								{
									xf_free(Handle->Buffer2);
									Handle->Buffer2 = nullptr;
									if (GetLastError() != ERROR_INVALID_LEVEL)
										Handle->ReadDone = true;
									else
										QueryResult = false;
								}
							}
						}

						if(!QueryResult)
						{
							Handle->Extended = false;

							// re-create handle to avoid weird bugs with some network emulators
							Directory->Close();
							if(Directory->Open(strDirectory, FILE_LIST_DIRECTORY, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
							{
								Handle->ObjectHandle =static_cast<HANDLE>(Directory);
								QueryResult = Directory->NtQueryDirectoryFile(Handle->BufferBase, Handle->BufferSize, FileBothDirectoryInformation, FALSE, NamePtr, TRUE);
							}
						}
						if(QueryResult)
						{
							PFILE_ID_BOTH_DIR_INFORMATION DirectoryInfo = static_cast<PFILE_ID_BOTH_DIR_INFORMATION>(Handle->BufferBase);
							FindData.dwFileAttributes = DirectoryInfo->FileAttributes;
							FindData.ftCreationTime.dwLowDateTime = DirectoryInfo->CreationTime.LowPart;
							FindData.ftCreationTime.dwHighDateTime = DirectoryInfo->CreationTime.HighPart;
							FindData.ftLastAccessTime.dwLowDateTime = DirectoryInfo->LastAccessTime.LowPart;
							FindData.ftLastAccessTime.dwHighDateTime = DirectoryInfo->LastAccessTime.HighPart;
							FindData.ftLastWriteTime.dwLowDateTime = DirectoryInfo->LastWriteTime.LowPart;
							FindData.ftLastWriteTime.dwHighDateTime = DirectoryInfo->LastWriteTime.HighPart;
							FindData.ftChangeTime.dwLowDateTime = DirectoryInfo->ChangeTime.LowPart;
							FindData.ftChangeTime.dwHighDateTime = DirectoryInfo->ChangeTime.HighPart;
							FindData.nFileSize = DirectoryInfo->EndOfFile.QuadPart;
							FindData.nAllocationSize = DirectoryInfo->AllocationSize.QuadPart;
							FindData.dwReserved0 = FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT?DirectoryInfo->EaSize:0;
							FindData.dwReserved1 = 0;

							if(Handle->Extended)
							{
								FindData.FileId = DirectoryInfo->FileId.QuadPart;
								FindData.strFileName.Copy(DirectoryInfo->FileName,DirectoryInfo->FileNameLength/sizeof(WCHAR));
								FindData.strAlternateFileName.Copy(DirectoryInfo->ShortName,DirectoryInfo->ShortNameLength/sizeof(WCHAR));
							}
							else
							{
								FindData.FileId = 0;
								PFILE_BOTH_DIR_INFORMATION DirectoryInfoSimple = reinterpret_cast<PFILE_BOTH_DIR_INFORMATION>(DirectoryInfo);
								FindData.strFileName.Copy(DirectoryInfoSimple->FileName,DirectoryInfoSimple->FileNameLength/sizeof(WCHAR));
								FindData.strAlternateFileName.Copy(DirectoryInfoSimple->ShortName,DirectoryInfoSimple->ShortNameLength/sizeof(WCHAR));
							}

							// Bug in SharePoint: FileName is zero-terminated and FileNameLength INCLUDES this zero.
							if(!FindData.strFileName.At(FindData.strFileName.GetLength()-1))
							{
								FindData.strFileName.SetLength(FindData.strFileName.GetLength()-1);
							}
							if(!FindData.strAlternateFileName.At(FindData.strAlternateFileName.GetLength()-1))
							{
								FindData.strAlternateFileName.SetLength(FindData.strAlternateFileName.GetLength()-1);
							}

							Handle->NextOffset = DirectoryInfo->NextEntryOffset;
							Result = static_cast<HANDLE>(Handle);
						}
						else
						{
							xf_free(Handle->BufferBase);
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
				if(Result == INVALID_HANDLE_VALUE)
				{
					delete Directory;
				}
			}
			if(Result == INVALID_HANDLE_VALUE)
			{
				delete Handle;
			}
		}
	}
	return Result;
}

bool FindNextFileInternal(HANDLE Find, FAR_FIND_DATA_EX& FindData)
{
	bool Result = false;
	PSEUDO_HANDLE* Handle = static_cast<PSEUDO_HANDLE*>(Find);
	bool Status = true, set_errcode = true;
	PFILE_ID_BOTH_DIR_INFORMATION DirectoryInfo = static_cast<PFILE_ID_BOTH_DIR_INFORMATION>(Handle->BufferBase);
	if(Handle->NextOffset)
	{
		DirectoryInfo = reinterpret_cast<PFILE_ID_BOTH_DIR_INFORMATION>(reinterpret_cast<LPBYTE>(DirectoryInfo)+Handle->NextOffset);
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
				xf_free(Handle->BufferBase);
				DirectoryInfo = reinterpret_cast<PFILE_ID_BOTH_DIR_INFORMATION>(Handle->BufferBase = Handle->Buffer2);
				Handle->Buffer2 = nullptr;
			}
			else
			{
				File* Directory = static_cast<File*>(Handle->ObjectHandle);
				Status = Directory->NtQueryDirectoryFile(Handle->BufferBase, Handle->BufferSize, Handle->Extended? FileIdBothDirectoryInformation : FileBothDirectoryInformation, FALSE, nullptr, FALSE);
				set_errcode = false;
			}
		}
	}

	if(Status)
	{
		FindData.dwFileAttributes = DirectoryInfo->FileAttributes;
		FindData.ftCreationTime.dwLowDateTime = DirectoryInfo->CreationTime.LowPart;
		FindData.ftCreationTime.dwHighDateTime = DirectoryInfo->CreationTime.HighPart;
		FindData.ftLastAccessTime.dwLowDateTime = DirectoryInfo->LastAccessTime.LowPart;
		FindData.ftLastAccessTime.dwHighDateTime = DirectoryInfo->LastAccessTime.HighPart;
		FindData.ftLastWriteTime.dwLowDateTime = DirectoryInfo->LastWriteTime.LowPart;
		FindData.ftLastWriteTime.dwHighDateTime = DirectoryInfo->LastWriteTime.HighPart;
		FindData.ftChangeTime.dwLowDateTime = DirectoryInfo->ChangeTime.LowPart;
		FindData.ftChangeTime.dwHighDateTime = DirectoryInfo->ChangeTime.HighPart;
		FindData.nFileSize = DirectoryInfo->EndOfFile.QuadPart;
		FindData.nAllocationSize = DirectoryInfo->AllocationSize.QuadPart;
		FindData.dwReserved0 = FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT?DirectoryInfo->EaSize:0;
		FindData.dwReserved1 = 0;

		if(Handle->Extended)
		{
			FindData.FileId = DirectoryInfo->FileId.QuadPart;
			FindData.strFileName.Copy(DirectoryInfo->FileName,DirectoryInfo->FileNameLength/sizeof(WCHAR));
			FindData.strAlternateFileName.Copy(DirectoryInfo->ShortName,DirectoryInfo->ShortNameLength/sizeof(WCHAR));
		}
		else
		{
			FindData.FileId = 0;
			PFILE_BOTH_DIR_INFORMATION DirectoryInfoSimple = reinterpret_cast<PFILE_BOTH_DIR_INFORMATION>(DirectoryInfo);
			FindData.strFileName.Copy(DirectoryInfoSimple->FileName,DirectoryInfoSimple->FileNameLength/sizeof(WCHAR));
			FindData.strAlternateFileName.Copy(DirectoryInfoSimple->ShortName,DirectoryInfoSimple->ShortNameLength/sizeof(WCHAR));
		}

		// Bug in SharePoint: FileName is zero-terminated and FileNameLength INCLUDES this zero.
		if(!FindData.strFileName.At(FindData.strFileName.GetLength()-1))
		{
			FindData.strFileName.SetLength(FindData.strFileName.GetLength()-1);
		}
		if(!FindData.strAlternateFileName.At(FindData.strAlternateFileName.GetLength()-1))
		{
			FindData.strAlternateFileName.SetLength(FindData.strAlternateFileName.GetLength()-1);
		}

		Handle->NextOffset = DirectoryInfo->NextEntryOffset?Handle->NextOffset+DirectoryInfo->NextEntryOffset:0;
		Result = true;
	}

	if (set_errcode)
		SetLastError(Result ? ERROR_SUCCESS : ERROR_NO_MORE_FILES);

	return Result;
}

bool FindCloseInternal(HANDLE Find)
{
	PSEUDO_HANDLE* Handle = static_cast<PSEUDO_HANDLE*>(Find);
   xf_free(Handle->Buffer2);
	xf_free(Handle->BufferBase);
	File* Directory = static_cast<File*>(Handle->ObjectHandle);
	delete Directory;
	delete Handle;
	return true;
}

//-------------------------------------------------------------------------
FindFile::FindFile(const string& Object, bool ScanSymLink):
	Handle(INVALID_HANDLE_VALUE),
	empty(false)
{
	NTPath strName(Object);
	bool Root = false;
	PATH_TYPE Type = ParsePath(strName, nullptr, &Root);
	if(Root && (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC || Type == PATH_VOLUMEGUID))
	{
		AddEndSlash(strName);
	}
	else
	{
		DeleteEndSlash(strName);
	}
	// temporary disable elevation to try "real" name first
	{
		DisableElevation DE;
		Handle = FindFirstFileInternal(strName, Data);
	}

	if (Handle == INVALID_HANDLE_VALUE && GetLastError() == ERROR_ACCESS_DENIED)
	{
		if(ScanSymLink)
		{
			string strReal(strName);
			// only links in path should be processed, not the object name itself
			CutToSlash(strReal);
			ConvertNameToReal(strReal, strReal);
			AddEndSlash(strReal);
			strReal+=PointToName(Object);
			strReal = NTPath(strReal);
			Handle = FindFirstFileInternal(strReal, Data);
		}

		if (Handle == INVALID_HANDLE_VALUE && ElevationRequired(ELEVATION_READ_REQUEST))
		{
			Handle = FindFirstFileInternal(strName, Data);
		}
	}
	empty = Handle == INVALID_HANDLE_VALUE;
}

FindFile::~FindFile()
{
	if(Handle != INVALID_HANDLE_VALUE)
	{
		FindCloseInternal(Handle);
	}
}

bool FindFile::Get(FAR_FIND_DATA_EX& FindData)
{
	bool Result = false;
	if (!empty)
	{
		FindData = Data;
		Result = true;
	}
	if(Result)
	{
		empty = !FindNextFileInternal(Handle, Data);
	}

	// skip ".." & "."
	if(Result && FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && FindData.strFileName.At(0) == L'.' &&
		((FindData.strFileName.At(1) == L'.' && !FindData.strFileName.At(2)) || !FindData.strFileName.At(1)) &&
		// хитрый способ - у виртуальных папок не бывает SFN, в отличие от. (UPD: или бывает, но такое же)
		(FindData.strAlternateFileName.IsEmpty() || FindData.strAlternateFileName == FindData.strFileName))
	{
		Result = Get(FindData);
	}
	return Result;
}



//-------------------------------------------------------------------------
File::File():
	Handle(INVALID_HANDLE_VALUE),
	Pointer(0),
	NeedSyncPointer(false)
{
}

File::~File()
{
	Close();
}

bool File::Open(const string& Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, File* TemplateFile, bool ForceElevation)
{
	HANDLE TemplateFileHandle = TemplateFile? TemplateFile->Handle : nullptr;
	Handle = apiCreateFile(Object, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFileHandle, ForceElevation);
	return Handle != INVALID_HANDLE_VALUE;
}

inline void File::SyncPointer()
{
	if(NeedSyncPointer)
	{
		SetFilePointerEx(Handle, *reinterpret_cast<PLARGE_INTEGER>(&Pointer), reinterpret_cast<PLARGE_INTEGER>(&Pointer), FILE_BEGIN);
		NeedSyncPointer = false;
	}
}


bool File::Read(LPVOID Buffer, DWORD NumberOfBytesToRead, DWORD& NumberOfBytesRead, LPOVERLAPPED Overlapped)
{
	SyncPointer();
	bool Result = ReadFile(Handle, Buffer, NumberOfBytesToRead, &NumberOfBytesRead, Overlapped) != FALSE;
	if(Result)
	{
		Pointer += NumberOfBytesRead;
	}
	return Result;
}

bool File::Write(LPCVOID Buffer, DWORD NumberOfBytesToWrite, DWORD& NumberOfBytesWritten, LPOVERLAPPED Overlapped)
{
	SyncPointer();
	bool Result = WriteFile(Handle, Buffer, NumberOfBytesToWrite, &NumberOfBytesWritten, Overlapped) != FALSE;
	if(Result)
	{
		Pointer += NumberOfBytesWritten;
	}
	return Result;
}

bool File::Read(LPVOID Buffer, size_t Nr, size_t& NumberOfBytesRead)
{
	bool Result = false;
	NumberOfBytesRead = 0;
	while (Nr)
	{
		DWORD nread = 0, nr = (Nr >= 2*1024*1024*1024U ? 1024*1024*1024 : static_cast<DWORD>(Nr));
		Result = Read(Buffer, nr, nread);
		NumberOfBytesRead += nread;
		if (!Result)
			break;
		Buffer = static_cast<LPVOID>(static_cast<char *>(Buffer) + nread);
		Nr -= nread;
	}
	return Result;
}

bool File::Write(LPCVOID Buffer, size_t Nw, size_t& NumberOfBytesWritten)
{
	bool Result = false;
	NumberOfBytesWritten = 0;
	while (Nw)
	{
		DWORD written = 0, nw = (Nw >= 2*1024*1024*1024U ? 1024*1024*1024 : static_cast<DWORD>(Nw));
		Result = Write(Buffer, nw, written);
		NumberOfBytesWritten += written;
		if (!Result)
			break;
		Buffer = static_cast<LPCVOID>(static_cast<const char *>(Buffer) + written);
		Nw -= written;
	}
	return Result;
}


bool File::SetPointer(INT64 DistanceToMove, PINT64 NewFilePointer, DWORD MoveMethod)
{
	INT64 OldPointer = Pointer;
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
			UINT64 Size=0;
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

bool File::SetEnd()
{
	SyncPointer();
	return SetEndOfFile(Handle) != FALSE;
}

bool File::GetTime(LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime)
{
	return GetFileTimeEx(Handle, CreationTime, LastAccessTime, LastWriteTime, ChangeTime);
}

bool File::SetTime(const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime, const FILETIME* ChangeTime)
{
	return SetFileTimeEx(Handle, CreationTime, LastAccessTime, LastWriteTime, ChangeTime);
}

bool File::GetSize(UINT64& Size)
{
	return apiGetFileSizeEx(Handle, Size);
}

bool File::FlushBuffers()
{
	return FlushFileBuffers(Handle) != FALSE;
}

bool File::GetInformation(BY_HANDLE_FILE_INFORMATION& info)
{
	return GetFileInformationByHandle(Handle, &info) != FALSE;
}

bool File::IoControl(DWORD IoControlCode, LPVOID InBuffer, DWORD InBufferSize, LPVOID OutBuffer, DWORD OutBufferSize, LPDWORD BytesReturned, LPOVERLAPPED Overlapped)
{
	return DeviceIoControl(Handle, IoControlCode, InBuffer, InBufferSize, OutBuffer, OutBufferSize, BytesReturned, Overlapped) != FALSE;
}

bool File::GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed)
{
	DWORD Result = ifn.GetStorageDependencyInformation(Handle, Flags, StorageDependencyInfoSize, StorageDependencyInfo, SizeUsed);
	SetLastError(Result);
	return Result == ERROR_SUCCESS;
}

bool File::NtQueryDirectoryFile(PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, bool ReturnSingleEntry, LPCWSTR FileName, bool RestartScan, NTSTATUS* Status)
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
	NTSTATUS Result = ifn.NtQueryDirectoryFile(Handle, nullptr, nullptr, nullptr, &IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, pNameString, RestartScan);
	SetLastError(ifn.RtlNtStatusToDosError(Result));
	if(Status)
	{
		*Status = Result;
	}
	return Result == STATUS_SUCCESS;
}

bool File::Close()
{
	bool Result=true;
	if(Handle!=INVALID_HANDLE_VALUE)
	{
		Result = CloseHandle(Handle) != FALSE;
		Handle = INVALID_HANDLE_VALUE;
	}
	Pointer = 0;
	NeedSyncPointer = false;
	return Result;
}

bool File::Eof()
{
	INT64 Ptr = GetPointer();
	UINT64 Size=0;
	GetSize(Size);
	return static_cast<UINT64>(Ptr) >= Size;
}
//-------------------------------------------------------------------------
FileWalker::FileWalker():
FileSize(0),
	AllocSize(0),
	ProcessedSize(0),
	CurrentChunk(nullptr),
	ChunkSize(0),
	Sparse(false)
{
	SingleChunk.Offset = 0;
	SingleChunk.Size = 0;
}

bool FileWalker::InitWalk(size_t BlockSize)
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
				bool QueryResult = IoControl(FSCTL_QUERY_ALLOCATED_RANGES, &QueryRange, sizeof(QueryRange), Ranges, sizeof(Ranges), &BytesReturned);
				if((QueryResult || GetLastError() == ERROR_MORE_DATA) && BytesReturned)
				{
					for(size_t i = 0; i < BytesReturned/sizeof(FILE_ALLOCATED_RANGE_BUFFER); ++i)
					{
						AllocSize += Ranges[i].Length.QuadPart;
						UINT64 RangeEndOffset = Ranges[i].FileOffset.QuadPart + Ranges[i].Length.QuadPart;
						for(UINT64 j = Ranges[i].FileOffset.QuadPart; j < RangeEndOffset; j+=ChunkSize)
						{
							Chunk c = {j, Min(static_cast<DWORD>(RangeEndOffset - j), ChunkSize)};
							ChunkList.Push(&c);
						}
					}
					QueryRange.FileOffset.QuadPart = ChunkList.Last()->Offset+ChunkList.Last()->Size;
					QueryRange.Length.QuadPart = FileSize - QueryRange.FileOffset.QuadPart;
				}
				else
				{
					break;
				}
			}
			Result = !ChunkList.Empty();
		}
		else
		{
			AllocSize = FileSize;
			CurrentChunk = &SingleChunk;
			Result = true;
		}
	}
	return Result;
}


bool FileWalker::Step()
{
	bool Result = false;
	if(Sparse)
	{
		CurrentChunk = ChunkList.Next(CurrentChunk);
		if(CurrentChunk)
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

UINT64 FileWalker::GetChunkOffset() const
{
	return CurrentChunk->Offset;
}

DWORD FileWalker::GetChunkSize() const
{
	return CurrentChunk->Size;
}

int FileWalker::GetPercent() const
{
	return AllocSize? (ProcessedSize) * 100 / AllocSize : 0;
}

//-------------------------------------------------------------------------

NTSTATUS GetLastNtStatus()
{
	return ifn.RtlGetLastNtStatusPresent()?ifn.RtlGetLastNtStatus():STATUS_SUCCESS;
}

BOOL apiDeleteFile(const string& FileName)
{
	NTPath strNtName(FileName);
	BOOL Result = DeleteFile(strNtName);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Elevation.fDeleteFile(strNtName);
	}
	return Result;
}

BOOL apiRemoveDirectory(const string& DirName)
{
	NTPath strNtName(DirName);
	BOOL Result = RemoveDirectory(strNtName);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Elevation.fRemoveDirectory(strNtName);
	}
	return Result;
}

HANDLE apiCreateFile(const string& Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile, bool ForceElevation)
{
	NTPath strObject(Object);
	FlagsAndAttributes|=FILE_FLAG_BACKUP_SEMANTICS|(CreationDistribution==OPEN_EXISTING?FILE_FLAG_POSIX_SEMANTICS:0);

	HANDLE Handle=CreateFile(strObject, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
	if(Handle == INVALID_HANDLE_VALUE)
	{
		DWORD Error=GetLastError();
		if(Error==ERROR_FILE_NOT_FOUND||Error==ERROR_PATH_NOT_FOUND)
		{
			FlagsAndAttributes&=~FILE_FLAG_POSIX_SEMANTICS;
			Handle = CreateFile(strObject, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
		}
	}

	if((Handle == INVALID_HANDLE_VALUE && ElevationRequired(DesiredAccess&(GENERIC_ALL|GENERIC_WRITE|WRITE_OWNER|WRITE_DAC|DELETE|FILE_WRITE_DATA|FILE_ADD_FILE|FILE_APPEND_DATA|FILE_ADD_SUBDIRECTORY|FILE_CREATE_PIPE_INSTANCE|FILE_WRITE_EA|FILE_DELETE_CHILD|FILE_WRITE_ATTRIBUTES)?ELEVATION_MODIFY_REQUEST:ELEVATION_READ_REQUEST)) || ForceElevation)
	{
		if(ForceElevation && Handle!=INVALID_HANDLE_VALUE)
		{
			CloseHandle(Handle);
		}
		Handle = Elevation.fCreateFile(strObject, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
	}
	return Handle;
}

BOOL apiCopyFileEx(
    const string& ExistingFileName,
    const string& NewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine,
    LPVOID lpData,
    LPBOOL pbCancel,
    DWORD dwCopyFlags
)
{
	NTPath strFrom(ExistingFileName), strTo(NewFileName);
	if(IsSlash(strTo.Last()))
	{
		strTo += PointToName(strFrom);
	}
	BOOL Result = CopyFileEx(strFrom, strTo, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	if(!Result)
	{
		if (STATUS_FILE_IS_A_DIRECTORY == GetLastNtStatus())
			SetLastError(ERROR_FILE_EXISTS);

		else if (ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
			Result = Elevation.fCopyFileEx(strFrom, strTo, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	}
	return Result;
}

BOOL apiMoveFile(
    const string& ExistingFileName, // address of name of the existing file
    const string& NewFileName   // address of new name for the file
)
{
	NTPath strFrom(ExistingFileName), strTo(NewFileName);
	if(IsSlash(strTo.Last()))
	{
		strTo += PointToName(strFrom);
	}
	BOOL Result = MoveFile(strFrom, strTo);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
	{
		Result = Elevation.fMoveFileEx(strFrom, strTo, 0);
	}
	return Result;
}

BOOL apiMoveFileEx(
    const string& ExistingFileName, // address of name of the existing file
    const string& NewFileName,   // address of new name for the file
    DWORD dwFlags   // flag to determine how to move file
)
{
	NTPath strFrom(ExistingFileName), strTo(NewFileName);
	if(IsSlash(strTo.Last()))
	{
		strTo += PointToName(strFrom);
	}
	BOOL Result = MoveFileEx(strFrom, strTo, dwFlags);
	if(!Result)
	{
		if (ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
		{
			// exclude fake elevation request for: move file over existing directory with same name
			DWORD f = apiGetFileAttributes(strFrom);
			DWORD t = apiGetFileAttributes(strTo);

			if (f!=INVALID_FILE_ATTRIBUTES && t!=INVALID_FILE_ATTRIBUTES && 0==(f & FILE_ATTRIBUTE_DIRECTORY) && 0!=(t & FILE_ATTRIBUTE_DIRECTORY))
				SetLastError(ERROR_FILE_EXISTS); // existing directory name == moved file name
			else
				Result = Elevation.fMoveFileEx(strFrom, strTo, dwFlags);
		}
	}
	return Result;
}

DWORD apiGetEnvironmentVariable(const string& Name, string &strBuffer)
{
	WCHAR Buffer[MAX_PATH];
	DWORD Size = GetEnvironmentVariable(Name, Buffer, ARRAYSIZE(Buffer));

	if (Size)
	{
		if(Size>ARRAYSIZE(Buffer))
		{
			wchar_t *lpwszBuffer = strBuffer.GetBuffer(Size);
			Size = GetEnvironmentVariable(Name, lpwszBuffer, Size);
			strBuffer.ReleaseBuffer();
		}
		else
		{
			strBuffer.Copy(Buffer, Size);
		}
	}

	return Size;
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
	DWORD Size=GetCurrentDirectory(ARRAYSIZE(Buffer),Buffer);
	if(Size)
	{
		string strInitCurDir;
		if(Size>ARRAYSIZE(Buffer))
		{
			LPWSTR InitCurDir=strInitCurDir.GetBuffer(Size);
			GetCurrentDirectory(Size,InitCurDir);
			strInitCurDir.ReleaseBuffer(Size-1);
		}
		else
		{
			strInitCurDir.Copy(Buffer, Size);
		}
		//set virtual curdir:
		apiSetCurrentDirectory(strInitCurDir);
	}
}

DWORD apiGetCurrentDirectory(string &strCurDir)
{
	//never give outside world a direct pointer to our internal string
	//who knows what they gonna do
	strCurDir.Copy(strCurrentDirectory().CPtr(),strCurrentDirectory().GetLength());
	return static_cast<DWORD>(strCurDir.GetLength());
}

BOOL apiSetCurrentDirectory(const string& PathName, bool Validate)
{
	// correct path to our standard
	string strDir=PathName;
	ReplaceSlashToBSlash(strDir);
	bool Root = false;
	PATH_TYPE Type = ParsePath(strDir, nullptr, &Root);
	if(Root && (Type == PATH_DRIVELETTER || Type == PATH_DRIVELETTERUNC || Type == PATH_VOLUMEGUID))
	{
		AddEndSlash(strDir);
	}
	else
	{
		DeleteEndSlash(strDir);
	}

	if (strDir == strCurrentDirectory())
		return TRUE;

	if (Validate)
	{
		string strDir=PathName;
		AddEndSlash(strDir);
		strDir+=L"*";
		FAR_FIND_DATA_EX fd;
		if (!apiGetFindDataEx(strDir, fd))
		{
			DWORD LastError = GetLastError();
			if(!(LastError == ERROR_FILE_NOT_FOUND || LastError == ERROR_NO_MORE_FILES))
				return FALSE;
		}
	}

	strCurrentDirectory()=strDir;

#ifndef NO_WRAPPER
	// try to synchronize far cur dir with process cur dir
	if(CtrlObject && CtrlObject->Plugins->GetOemPluginsCount())
	{
		SetCurrentDirectory(strCurrentDirectory());
	}
#endif // NO_WRAPPER
	return TRUE;
}

DWORD apiGetTempPath(string &strBuffer)
{
	WCHAR Buffer[MAX_PATH];
	DWORD Size = GetTempPath(ARRAYSIZE(Buffer), Buffer);
	if(Size)
	{
		if(Size>ARRAYSIZE(Buffer))
		{
			wchar_t *lpwszBuffer = strBuffer.GetBuffer(Size);
			Size = GetTempPath(Size, lpwszBuffer);
			strBuffer.ReleaseBuffer(Size-1);
		}
		else
		{
			strBuffer.Copy(Buffer, Size);
		}
	}
	return Size;
};


DWORD apiGetModuleFileName(HMODULE hModule, string &strFileName)
{
	return apiGetModuleFileNameEx(nullptr, hModule, strFileName);
}

DWORD apiGetModuleFileNameEx(HANDLE hProcess, HMODULE hModule, string &strFileName)
{
	DWORD dwSize = 0;
	DWORD dwBufferSize = MAX_PATH;
	wchar_t *lpwszFileName = nullptr;

	do
	{
		dwBufferSize <<= 1;
		lpwszFileName = (wchar_t*)xf_realloc_nomove(lpwszFileName, dwBufferSize*sizeof(wchar_t));
		if (hProcess)
		{
			if (ifn.QueryFullProcessImageNameWPresent() && !hModule)
			{
				DWORD sz = dwBufferSize;
				dwSize = 0;
				if (ifn.QueryFullProcessImageNameW(hProcess, 0, lpwszFileName, &sz))
				{
					dwSize = sz;
				}
			}
			else
			{
				dwSize = GetModuleFileNameEx(hProcess, hModule, lpwszFileName, dwBufferSize);
			}
		}
		else
		{
			dwSize = GetModuleFileName(hModule, lpwszFileName, dwBufferSize);
		}
	}
	while ((dwSize >= dwBufferSize) || (!dwSize && GetLastError() == ERROR_INSUFFICIENT_BUFFER));

	if (dwSize)
		strFileName.Copy(lpwszFileName, dwSize);

	xf_free(lpwszFileName);
	return dwSize;
}

bool apiExpandEnvironmentStrings(const string& Src, string &Dest)
{
	bool Result = false;
	WCHAR Buffer[MAX_PATH];
	DWORD Size = ExpandEnvironmentStrings(Src, Buffer, ARRAYSIZE(Buffer));
	if (Size)
	{
		if (Size > ARRAYSIZE(Buffer))
		{
			string SrcCopy(Src);
			SrcCopy.Unlink(); //src can point to strDest data
			wchar_t *lpwszDest = Dest.GetBuffer(Size);
			Dest.ReleaseBuffer(ExpandEnvironmentStrings(SrcCopy, lpwszDest, Size)-1);
		}
		else
		{
			Dest.Copy(Buffer, Size-1);
		}
		Result = true;
	}

	return Result;
}

DWORD apiWNetGetConnection(const string& LocalName, string &RemoteName)
{
	DWORD dwRemoteNameSize = 0;
	DWORD dwResult = WNetGetConnection(LocalName, nullptr, &dwRemoteNameSize);

	if (dwResult == ERROR_SUCCESS || dwResult == ERROR_MORE_DATA)
	{
		wchar_t *lpwszRemoteName = RemoteName.GetBuffer(dwRemoteNameSize);
		dwResult = WNetGetConnection(LocalName, lpwszRemoteName, &dwRemoteNameSize);
		RemoteName.ReleaseBuffer();
	}

	return dwResult;
}

BOOL apiGetVolumeInformation(
    const string& RootPathName,
    string *pVolumeName,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    string *pFileSystemName
)
{
	wchar_t *lpwszVolumeName = pVolumeName?pVolumeName->GetBuffer(MAX_PATH+1):nullptr;  //MSDN!
	wchar_t *lpwszFileSystemName = pFileSystemName?pFileSystemName->GetBuffer(MAX_PATH+1):nullptr;
	BOOL bResult = GetVolumeInformation(
	                   RootPathName,
	                   lpwszVolumeName,
	                   lpwszVolumeName?MAX_PATH:0,
	                   lpVolumeSerialNumber,
	                   lpMaximumComponentLength,
	                   lpFileSystemFlags,
	                   lpwszFileSystemName,
	                   lpwszFileSystemName?MAX_PATH:0
	               );

	if (lpwszVolumeName)
		pVolumeName->ReleaseBuffer();

	if (lpwszFileSystemName)
		pFileSystemName->ReleaseBuffer();

	return bResult;
}

bool apiGetFindDataEx(const string& FileName, FAR_FIND_DATA_EX& FindData,bool ScanSymLink)
{
	FindFile Find(FileName, ScanSymLink);
	if(Find.Get(FindData))
	{
		return true;
	}
	else
	{
		const wchar_t* DirPtr = FileName;
		ParsePath(FileName, &DirPtr);
		if (!wcspbrk(DirPtr,L"*?"))
		{
			DWORD dwAttr=apiGetFileAttributes(FileName);

			if (dwAttr!=INVALID_FILE_ATTRIBUTES)
			{
				// Ага, значит файл таки есть. Заполним структуру ручками.
				FindData.Clear();
				FindData.dwFileAttributes=dwAttr;
				File file;
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

				FindData.dwReserved1=0;
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

bool apiGetFileSizeEx(HANDLE hFile, UINT64 &Size)
{
	bool Result=false;

	if (GetFileSizeEx(hFile,reinterpret_cast<PLARGE_INTEGER>(&Size)))
	{
		Result=true;
	}
	else
	{
		GET_LENGTH_INFORMATION gli;
		DWORD BytesReturned;

		if (DeviceIoControl(hFile,IOCTL_DISK_GET_LENGTH_INFO,nullptr,0,&gli,sizeof(gli),&BytesReturned,nullptr))
		{
			Size=gli.Length.QuadPart;
			Result=true;
		}
	}

	return Result;
}

int apiRegEnumKeyEx(HKEY hKey,DWORD dwIndex,string &strName,PFILETIME lpftLastWriteTime)
{
	int ExitCode=ERROR_MORE_DATA;

	for (DWORD Size=512; ExitCode==ERROR_MORE_DATA; Size<<=1)
	{
		wchar_t *Name=strName.GetBuffer(Size);
		DWORD Size0=Size;
		ExitCode=RegEnumKeyEx(hKey,dwIndex,Name,&Size0,nullptr,nullptr,nullptr,lpftLastWriteTime);
		strName.ReleaseBuffer();
	}

	return ExitCode;
}

BOOL apiIsDiskInDrive(const string& Root)
{
	string strVolName;
	string strDrive;
	DWORD  MaxComSize;
	DWORD  Flags;
	string strFS;
	strDrive = Root;
	AddEndSlash(strDrive);
	BOOL Res = apiGetVolumeInformation(strDrive, &strVolName, nullptr, &MaxComSize, &Flags, &strFS);
	return Res;
}

int apiGetFileTypeByName(const string& Name)
{
	HANDLE hFile=apiCreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0);

	if (hFile==INVALID_HANDLE_VALUE)
		return FILE_TYPE_UNKNOWN;

	int Type=GetFileType(hFile);
	CloseHandle(hFile);
	return Type;
}

bool apiGetDiskSize(const string& Path,unsigned __int64 *TotalSize, unsigned __int64 *TotalFree, unsigned __int64 *UserFree)
{
	bool Result = false;
	unsigned __int64 uiTotalSize,uiTotalFree,uiUserFree;
	uiUserFree=0;
	uiTotalSize=0;
	uiTotalFree=0;
	NTPath strPath(Path);
	AddEndSlash(strPath);
	if(GetDiskFreeSpaceEx(strPath,(PULARGE_INTEGER)&uiUserFree,(PULARGE_INTEGER)&uiTotalSize,(PULARGE_INTEGER)&uiTotalFree))
	{
		Result = true;

		if (TotalSize)
			*TotalSize = uiTotalSize;

		if (TotalFree)
			*TotalFree = uiTotalFree;

		if (UserFree)
			*UserFree = uiUserFree;
	}
	return Result;
}

HANDLE apiFindFirstFileName(const string& FileName, DWORD dwFlags, string& LinkName)
{
	HANDLE hRet=INVALID_HANDLE_VALUE;
	DWORD StringLength=0;
	NTPath NtFileName(FileName);
	if (ifn.FindFirstFileNameW(NtFileName, 0, &StringLength, nullptr)==INVALID_HANDLE_VALUE && GetLastError()==ERROR_MORE_DATA)
	{
		hRet=ifn.FindFirstFileNameW(NtFileName, 0, &StringLength, LinkName.GetBuffer(StringLength));
		LinkName.ReleaseBuffer();
	}
	return hRet;
}

BOOL apiFindNextFileName(HANDLE hFindStream, string& LinkName)
{
	BOOL Ret=FALSE;
	DWORD StringLength=0;
	if (!ifn.FindNextFileNameW(hFindStream, &StringLength, nullptr) && GetLastError()==ERROR_MORE_DATA)
	{
		Ret=ifn.FindNextFileNameW(hFindStream, &StringLength, LinkName.GetBuffer(StringLength));
		LinkName.ReleaseBuffer();
	}
	return Ret;
}

BOOL apiCreateDirectory(const string& PathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return apiCreateDirectoryEx(L"", PathName, lpSecurityAttributes);
}

BOOL apiCreateDirectoryEx(const string& TemplateDirectory, const string& NewDirectory, LPSECURITY_ATTRIBUTES SecurityAttributes)
{
	NTPath NtTemplateDirectory(TemplateDirectory);
	NTPath NtNewDirectory(NewDirectory);
	BOOL Result = FALSE;
	for(size_t i = 0; i < 2 && !Result; ++i)
	{
		Result = NtTemplateDirectory.IsEmpty()?CreateDirectory(NtNewDirectory, SecurityAttributes):CreateDirectoryEx(NtTemplateDirectory, NtNewDirectory, SecurityAttributes);
		if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
		{
			Result = Elevation.fCreateDirectoryEx(NtTemplateDirectory, NtNewDirectory, SecurityAttributes);
		}
		if(!Result)
		{
			// CreateDirectoryEx may fail on some FS, try to create anyway.
			NtTemplateDirectory.Clear();
		}
	}
	return Result;
}

DWORD apiGetFileAttributes(const string& FileName)
{
	NTPath NtName(FileName);
	DWORD Result = GetFileAttributes(NtName);
	if(Result == INVALID_FILE_ATTRIBUTES && ElevationRequired(ELEVATION_READ_REQUEST))
	{
		Result = Elevation.fGetFileAttributes(NtName);
	}
	return Result;
}

BOOL apiSetFileAttributes(const string& FileName,DWORD dwFileAttributes)
{
	NTPath NtName(FileName);
	BOOL Result = SetFileAttributes(NtName, dwFileAttributes);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Elevation.fSetFileAttributes(NtName, dwFileAttributes);
	}
	return Result;

}

bool CreateSymbolicLinkInternal(const string& Object, const string& Target, DWORD dwFlags)
{
	return ifn.CreateSymbolicLinkWPresent()?
		(ifn.CreateSymbolicLink(Object, Target, dwFlags) != FALSE) :
		CreateReparsePoint(Target, Object, dwFlags&SYMBOLIC_LINK_FLAG_DIRECTORY?RP_SYMLINKDIR:RP_SYMLINKFILE);
}

bool apiCreateSymbolicLink(const string& SymlinkFileName, const string& TargetFileName,DWORD dwFlags)
{
	bool Result=false;
	NTPath NtSymlinkFileName(SymlinkFileName);
	Result=CreateSymbolicLinkInternal(NtSymlinkFileName, TargetFileName, dwFlags);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result=Elevation.fCreateSymbolicLink(NtSymlinkFileName, TargetFileName, dwFlags);
	}
	return Result;
}

bool apiSetFileEncryptionInternal(const wchar_t* Name, bool Encrypt)
{
	return Encrypt? EncryptFile(Name)!=FALSE : DecryptFile(Name, 0)!=FALSE;
}

bool apiSetFileEncryption(const string& Name, bool Encrypt)
{
	bool Result=false;
	NTPath NtName(Name);
	Result = apiSetFileEncryptionInternal(NtName, Encrypt);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST, false)) // Encryption implemented in advapi32, NtStatus not affected
	{
		Result=Elevation.fSetFileEncryption(NtName, Encrypt);
	}
	return Result;
}

bool CreateHardLinkInternal(const string& Object, const string& Target,LPSECURITY_ATTRIBUTES SecurityAttributes)
{
	bool Result = CreateHardLink(Object, Target, SecurityAttributes) != FALSE;
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Elevation.fCreateHardLink(Object, Target, SecurityAttributes);
	}
	return Result;
}

BOOL apiCreateHardLink(const string& FileName, const string& ExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return CreateHardLinkInternal(NTPath(FileName),NTPath(ExistingFileName), lpSecurityAttributes) ||
	       //bug in win2k: \\?\ fails
	       CreateHardLinkInternal(FileName, ExistingFileName, lpSecurityAttributes);
}

HANDLE apiFindFirstStream(const string& FileName,STREAM_INFO_LEVELS InfoLevel,LPVOID lpFindStreamData,DWORD dwFlags)
{
	HANDLE Ret=INVALID_HANDLE_VALUE;
	if(ifn.FindFirstStreamWPresent())
	{
		Ret=ifn.FindFirstStreamW(NTPath(FileName),InfoLevel,lpFindStreamData,dwFlags);
	}
	else
	{
		if (InfoLevel==FindStreamInfoStandard)
		{
			PSEUDO_HANDLE* Handle=new PSEUDO_HANDLE;
			if(Handle)
			{
				Handle->ObjectHandle = apiCreateFile(FileName,0,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0);

				if (Handle->ObjectHandle!=INVALID_HANDLE_VALUE)
				{
					// for network paths buffer size must be <= 65k
					// we double it in a first loop, so starting value is 32k
					Handle->BufferSize = 0x8000;
					Handle->BufferBase = nullptr;

					NTSTATUS Result = STATUS_SEVERITY_ERROR;
					do
					{
						Handle->BufferSize<<=1;
						Handle->BufferBase=static_cast<LPBYTE>(xf_realloc_nomove(Handle->BufferBase, Handle->BufferSize));
						if (Handle->BufferBase)
						{
							// sometimes for directories NtQueryInformationFile returns STATUS_SUCCESS but doesn't fill the buffer
							PFILE_STREAM_INFORMATION StreamInfo = static_cast<PFILE_STREAM_INFORMATION>(Handle->BufferBase);
							StreamInfo->StreamNameLength = 0;

							IO_STATUS_BLOCK IoStatusBlock;
							Result = ifn.NtQueryInformationFile(Handle->ObjectHandle, &IoStatusBlock, Handle->BufferBase, Handle->BufferSize, FileStreamInformation);
						}
					}
					while(Result == STATUS_BUFFER_OVERFLOW || Result == STATUS_BUFFER_TOO_SMALL);

					if (Result == STATUS_SUCCESS)
					{
						PWIN32_FIND_STREAM_DATA pFsd=static_cast<PWIN32_FIND_STREAM_DATA>(lpFindStreamData);
						PFILE_STREAM_INFORMATION StreamInfo = static_cast<PFILE_STREAM_INFORMATION>(Handle->BufferBase);
						Handle->NextOffset = StreamInfo->NextEntryOffset;
						if (StreamInfo->StreamNameLength)
						{
							memcpy(pFsd->cStreamName,StreamInfo->StreamName,StreamInfo->StreamNameLength);
							pFsd->cStreamName[StreamInfo->StreamNameLength/sizeof(WCHAR)]=L'\0';
							pFsd->StreamSize=StreamInfo->StreamSize;
							Ret=Handle;
						}
					}

					CloseHandle(Handle->ObjectHandle);

					if (Ret==INVALID_HANDLE_VALUE)
					{
						if(Handle->BufferBase)
						{
							xf_free(Handle->BufferBase);
						}
						delete Handle;
					}
				}
			}
		}
	}

	return Ret;
}

BOOL apiFindNextStream(HANDLE hFindStream,LPVOID lpFindStreamData)
{
	BOOL Ret=FALSE;
	if(ifn.FindFirstStreamWPresent())
	{
		Ret=ifn.FindNextStreamW(hFindStream,lpFindStreamData);
	}
	else
	{
		PSEUDO_HANDLE* Handle = static_cast<PSEUDO_HANDLE*>(hFindStream);

		if (Handle->NextOffset)
		{
			PFILE_STREAM_INFORMATION pStreamInfo=reinterpret_cast<PFILE_STREAM_INFORMATION>(reinterpret_cast<LPBYTE>(Handle->BufferBase)+Handle->NextOffset);
			PWIN32_FIND_STREAM_DATA pFsd=static_cast<PWIN32_FIND_STREAM_DATA>(lpFindStreamData);
			Handle->NextOffset = pStreamInfo->NextEntryOffset?Handle->NextOffset+pStreamInfo->NextEntryOffset:0;
			if (pStreamInfo->StreamNameLength && pStreamInfo->StreamNameLength < sizeof(pFsd->cStreamName))
			{
				memcpy(pFsd->cStreamName,pStreamInfo->StreamName,pStreamInfo->StreamNameLength);
				pFsd->cStreamName[pStreamInfo->StreamNameLength/sizeof(WCHAR)]=L'\0';
				pFsd->StreamSize=pStreamInfo->StreamSize;
				Ret=TRUE;
			}
		}
	}

	return Ret;
}

BOOL apiFindStreamClose(HANDLE hFindStream)
{
	BOOL Ret=FALSE;

	if(ifn.FindFirstStreamWPresent())
	{
		Ret=FindClose(hFindStream);
	}
	else
	{
		PSEUDO_HANDLE* Handle = static_cast<PSEUDO_HANDLE*>(hFindStream);
		xf_free(Handle->BufferBase);
		delete Handle;
		Ret=TRUE;
	}

	return Ret;
}

bool apiGetLogicalDriveStrings(string& DriveStrings)
{
	DWORD BufSize = MAX_PATH;
	DWORD Size = GetLogicalDriveStringsW(BufSize, DriveStrings.GetBuffer(BufSize));

	if (Size > BufSize)
	{
		BufSize = Size;
		Size = GetLogicalDriveStringsW(BufSize, DriveStrings.GetBuffer(BufSize));
	}

	if ((Size > 0) && (Size <= BufSize))
	{
		DriveStrings.ReleaseBuffer(Size);
		return true;
	}

	return false;
}

bool internalNtQueryGetFinalPathNameByHandle(HANDLE hFile, string& FinalFilePath)
{
	ULONG RetLen;
	ULONG BufSize = NT_MAX_PATH;
	OBJECT_NAME_INFORMATION* oni = static_cast<OBJECT_NAME_INFORMATION*>(xf_malloc(BufSize));
	NTSTATUS Res = ifn.NtQueryObject(hFile, ObjectNameInformation, oni, BufSize, &RetLen);

	if (Res == STATUS_BUFFER_OVERFLOW || Res == STATUS_BUFFER_TOO_SMALL)
	{
		BufSize = RetLen;
		oni = static_cast<OBJECT_NAME_INFORMATION*>(xf_realloc_nomove(oni, BufSize));
		Res = ifn.NtQueryObject(hFile, ObjectNameInformation, oni, BufSize, &RetLen);
	}

	string NtPath;

	if (Res == STATUS_SUCCESS)
	{
		NtPath.Copy(oni->Name.Buffer, oni->Name.Length / sizeof(WCHAR));
	}

	xf_free(oni);

	FinalFilePath.Clear();

	if (Res == STATUS_SUCCESS)
	{
		// simple way to handle network paths
		if (NtPath.IsSubStrAt(0, L"\\Device\\LanmanRedirector"))
			FinalFilePath = NtPath.Replace(0, 24, L'\\');

		if (FinalFilePath.IsEmpty())
		{
			// try to convert NT path (\Device\HarddiskVolume1) to drive letter
			string DriveStrings;

			if (apiGetLogicalDriveStrings(DriveStrings))
			{
				string DiskName(L"A:");
				const wchar_t* Drive = DriveStrings.CPtr();

				while (*Drive)
				{
					DiskName.Replace(0, *Drive);
					int Len = MatchNtPathRoot(NtPath, DiskName);

					if (Len)
					{
						if (NtPath.IsSubStrAt(0, L"\\Device\\WinDfs"))
							FinalFilePath = NtPath.Replace(0, Len, L'\\');
						else
							FinalFilePath = NtPath.Replace(0, Len, DiskName);
						break;
					}

					Drive += StrLength(Drive) + 1;
				}
			}
		}

		if (FinalFilePath.IsEmpty())
		{
			// try to convert NT path (\Device\HarddiskVolume1) to \\?\Volume{...} path
			wchar_t VolumeName[cVolumeGuidLen + 1 + 1];
			HANDLE hEnum = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));
			BOOL Res = hEnum != INVALID_HANDLE_VALUE;

			while (Res)
			{
				if (StrLength(VolumeName) >= (int)cVolumeGuidLen)
				{
					DeleteEndSlash(VolumeName);
					int Len = MatchNtPathRoot(NtPath, VolumeName + 4 /* w/o prefix */);

					if (Len)
					{
						FinalFilePath = NtPath.Replace(0, Len, VolumeName);
						break;
					}
				}

				Res = FindNextVolumeW(hEnum, VolumeName, ARRAYSIZE(VolumeName));
			}

			if (hEnum != INVALID_HANDLE_VALUE)
				FindVolumeClose(hEnum);
		}
	}

	return !FinalFilePath.IsEmpty();
}

bool apiGetFinalPathNameByHandle(HANDLE hFile, string& FinalFilePath)
{
	if (ifn.GetFinalPathNameByHandleWPresent())
	{
		DWORD BufLen = NT_MAX_PATH;
		DWORD Len = ifn.GetFinalPathNameByHandle(hFile, FinalFilePath.GetBuffer(BufLen+1), BufLen, VOLUME_NAME_GUID);

		if (Len > BufLen)
		{
			BufLen = Len;
			Len = ifn.GetFinalPathNameByHandle(hFile, FinalFilePath.GetBuffer(BufLen+1), BufLen, VOLUME_NAME_GUID);
		}

		if (Len <= BufLen)
			FinalFilePath.ReleaseBuffer(Len);
		else
			FinalFilePath.Clear();

		return Len  && Len <= BufLen;
	}

	return internalNtQueryGetFinalPathNameByHandle(hFile, FinalFilePath);
}

bool apiSearchPath(const wchar_t *Path, const string& FileName, const wchar_t *Extension, string &strDest)
{
	DWORD dwSize = SearchPath(Path,FileName,Extension,0,nullptr,nullptr);

	if (dwSize)
	{
		wchar_t *lpwszFullName=strDest.GetBuffer(dwSize);
		dwSize = SearchPath(Path,FileName,Extension,dwSize,lpwszFullName,nullptr);
		strDest.ReleaseBuffer(dwSize);
		return true;
	}

	return false;
}

bool apiQueryDosDevice(const string& DeviceName, string &Path) {
	SetLastError(NO_ERROR);
	DWORD Res = QueryDosDeviceW(DeviceName, Path.GetBuffer(MAX_PATH), MAX_PATH);
	if (!Res && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		SetLastError(NO_ERROR);
		Res = QueryDosDeviceW(DeviceName, Path.GetBuffer(NT_MAX_PATH), NT_MAX_PATH);
	}
	if (!Res || GetLastError() != NO_ERROR)
		return false;
	Path.ReleaseBuffer();
	return true;
}

bool apiGetVolumeNameForVolumeMountPoint(const string& VolumeMountPoint,string& VolumeName)
{
	bool Result=false;
	WCHAR VolumeNameBuffer[50];
	NTPath strVolumeMountPoint(VolumeMountPoint);
	AddEndSlash(strVolumeMountPoint);
	if(GetVolumeNameForVolumeMountPoint(strVolumeMountPoint,VolumeNameBuffer,ARRAYSIZE(VolumeNameBuffer)))
	{
		VolumeName=VolumeNameBuffer;
		Result=true;
	}
	return Result;
}

void apiEnableLowFragmentationHeap()
{
	if (ifn.HeapSetInformationPresent())
	{
		DWORD NumHeaps = 10;
		HANDLE* Heaps = new HANDLE[NumHeaps];
		DWORD ActualNumHeaps = GetProcessHeaps(NumHeaps, Heaps);
		if(ActualNumHeaps > NumHeaps)
		{
			delete[] Heaps;
			Heaps = new HANDLE[ActualNumHeaps];
			GetProcessHeaps(ActualNumHeaps, Heaps);
		}
		for (DWORD i = 0; i < ActualNumHeaps; i++)
		{
			ULONG HeapFragValue = 2;
			ifn.HeapSetInformation(Heaps[i], HeapCompatibilityInformation, &HeapFragValue, sizeof(HeapFragValue));
		}
		delete[] Heaps;
	}
}

bool GetFileTimeEx(HANDLE Object, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime)
{
	bool Result = false;
	const ULONG Length = 40;
	BYTE Buffer[Length] = {};
	PFILE_BASIC_INFORMATION fbi = reinterpret_cast<PFILE_BASIC_INFORMATION>(Buffer);
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS Status = ifn.NtQueryInformationFile(Object, &IoStatusBlock, fbi, Length, FileBasicInformation);
	SetLastError(ifn.RtlNtStatusToDosError(Status));
	if (Status == STATUS_SUCCESS)
	{
		if(CreationTime)
		{
			CreationTime->dwLowDateTime = fbi->CreationTime.LowPart;
			CreationTime->dwHighDateTime = fbi->CreationTime.HighPart;
		}
		if(LastAccessTime)
		{
			LastAccessTime->dwLowDateTime = fbi->LastAccessTime.LowPart;
			LastAccessTime->dwHighDateTime = fbi->LastAccessTime.HighPart;
		}
		if(LastWriteTime)
		{
			LastWriteTime->dwLowDateTime = fbi->LastWriteTime.LowPart;
			LastWriteTime->dwHighDateTime = fbi->LastWriteTime.HighPart;
		}
		if(ChangeTime)
		{
			ChangeTime->dwLowDateTime = fbi->ChangeTime.LowPart;
			ChangeTime->dwHighDateTime = fbi->ChangeTime.HighPart;
		}
		Result = true;
	}
	return Result;
}

bool SetFileTimeEx(HANDLE Object, const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime, const FILETIME* ChangeTime)
{
	bool Result = false;
	const ULONG Length = 40;
	BYTE Buffer[Length] = {};
	PFILE_BASIC_INFORMATION fbi = reinterpret_cast<PFILE_BASIC_INFORMATION>(Buffer);
	if(CreationTime)
	{
		fbi->CreationTime.HighPart = CreationTime->dwHighDateTime;
		fbi->CreationTime.LowPart = CreationTime->dwLowDateTime;
	}
	if(LastAccessTime)
	{
		fbi->LastAccessTime.HighPart = LastAccessTime->dwHighDateTime;
		fbi->LastAccessTime.LowPart = LastAccessTime->dwLowDateTime;
	}
	if(LastWriteTime)
	{
		fbi->LastWriteTime.HighPart = LastWriteTime->dwHighDateTime;
		fbi->LastWriteTime.LowPart = LastWriteTime->dwLowDateTime;
	}
	if(ChangeTime)
	{
		fbi->ChangeTime.HighPart = ChangeTime->dwHighDateTime;
		fbi->ChangeTime.LowPart = ChangeTime->dwLowDateTime;
	}
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS Status = ifn.NtSetInformationFile(Object, &IoStatusBlock, fbi, Length, FileBasicInformation);
	SetLastError(ifn.RtlNtStatusToDosError(Status));
	Result = Status == STATUS_SUCCESS;
	return Result;
}

int RegQueryStringValue(HKEY hKey, const string& ValueName, string &strData, const wchar_t *lpwszDefault)
{
	DWORD cbSize = 0;
	int nResult = RegQueryValueEx(hKey, ValueName, nullptr, nullptr, nullptr, &cbSize);

	if (nResult == ERROR_SUCCESS)
	{
		wchar_t *lpwszData = strData.GetBuffer(cbSize/sizeof(wchar_t)+1);
		DWORD Type=REG_SZ;
		nResult = RegQueryValueEx(hKey, ValueName, nullptr, &Type, reinterpret_cast<LPBYTE>(lpwszData), &cbSize);
		int Size=cbSize/sizeof(wchar_t);

		if (Type==REG_SZ||Type==REG_EXPAND_SZ||Type==REG_MULTI_SZ)
		{
			if (!lpwszData[Size-1])
				Size--;

			strData.ReleaseBuffer(Size);
		}
		else
		{
			lpwszData[Size] = 0;
			strData.ReleaseBuffer();
		}
	}

	if (nResult != ERROR_SUCCESS)
	{
		strData = lpwszDefault;
	}

	return nResult;
}


int EnumRegValueEx(HKEY hRegRootKey, const string& Key, DWORD Index, string &strDestName, string &strSData, LPDWORD IData, __int64* IData64, DWORD *lpType)
{
	HKEY hKey;
	int RetCode=REG_NONE;
	DWORD Type=(DWORD)-1;
	if(RegOpenKeyEx(hRegRootKey, Key, 0, KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS)
	{
		string strValueName;
		DWORD ValNameSize=512, ValNameSize0;
		LONG ExitCode=ERROR_MORE_DATA;

		// get size value name
		for (; ExitCode==ERROR_MORE_DATA; ValNameSize<<=1)
		{
			wchar_t *Name=strValueName.GetBuffer(ValNameSize);
			ValNameSize0=ValNameSize;
			ExitCode=RegEnumValue(hKey,Index,Name,&ValNameSize0,nullptr,nullptr,nullptr,nullptr);
			strValueName.ReleaseBuffer();
		}

		if (ExitCode != ERROR_NO_MORE_ITEMS)
		{
			DWORD Size = 0, Size0;
			ValNameSize0=ValNameSize;
			// Get DataSize
			/*ExitCode = */RegEnumValue(hKey,Index,(LPWSTR)strValueName.CPtr(),&ValNameSize0, nullptr, &Type, nullptr, &Size);
			// здесь ExitCode == ERROR_SUCCESS

			// корректировка размера
			if (Type == REG_DWORD)
			{
				if (Size < sizeof(DWORD))
					Size = sizeof(DWORD);
			}
			else if (Type == REG_QWORD)
			{
				if (Size < sizeof(__int64))
					Size = sizeof(__int64);
			}

			wchar_t *Data = strSData.GetBuffer(Size/sizeof(wchar_t)+1);
			wmemset(Data,0,Size/sizeof(wchar_t)+1);
			ValNameSize0=ValNameSize;
			Size0=Size;
			ExitCode=RegEnumValue(hKey,Index,(LPWSTR)strValueName.CPtr(),&ValNameSize0,nullptr,&Type,(LPBYTE)Data,&Size0);

			if (ExitCode == ERROR_SUCCESS)
			{
				if (Type == REG_DWORD)
				{
					if (IData)
						*IData=*(DWORD*)Data;
				}
				else if (Type == REG_QWORD)
				{
					if (IData64)
						*IData64=*(__int64*)Data;
				}

				RetCode=Type;
				strDestName = strValueName;
			}

			strSData.ReleaseBuffer(Size/sizeof(wchar_t));
		}

		RegCloseKey(hKey);
	}
	if (lpType)
	{
		*lpType=Type;
	}
	return RetCode;
}

bool apiGetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, FAR_SECURITY_DESCRIPTOR& SecurityDescriptor)
{
	bool Result = false;
	NTPath NtObject(Object);
	DWORD LengthNeeded = 0;
	GetFileSecurity(NtObject, RequestedInformation, nullptr, 0, &LengthNeeded);
	if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		SecurityDescriptor.Size = LengthNeeded;
		SecurityDescriptor.SecurityDescriptor = static_cast<PSECURITY_DESCRIPTOR>(xf_malloc(SecurityDescriptor.Size));
		Result = GetFileSecurity(NtObject, RequestedInformation, SecurityDescriptor.SecurityDescriptor, LengthNeeded, &LengthNeeded) != FALSE;
	}
	return Result;
}

bool apiSetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, const FAR_SECURITY_DESCRIPTOR& SecurityDescriptor)
{
	return SetFileSecurity(NTPath(Object), RequestedInformation, SecurityDescriptor.SecurityDescriptor) != FALSE;
}

bool apiOpenVirtualDiskInternal(VIRTUAL_STORAGE_TYPE& VirtualStorageType, const string& Object, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, OPEN_VIRTUAL_DISK_PARAMETERS& Parameters, HANDLE& Handle)
{
	DWORD Result = ifn.OpenVirtualDisk(&VirtualStorageType, Object, VirtualDiskAccessMask, Flags, &Parameters, &Handle);
	SetLastError(Result);
	return Result == ERROR_SUCCESS;
}
bool apiOpenVirtualDisk(VIRTUAL_STORAGE_TYPE& VirtualStorageType, const string& Object, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, OPEN_VIRTUAL_DISK_PARAMETERS& Parameters, HANDLE& Handle)
{
	NTPath NtObject(Object);
	bool Result = apiOpenVirtualDiskInternal(VirtualStorageType, NtObject, VirtualDiskAccessMask, Flags, Parameters, Handle);
	if(!Result && ElevationRequired(ELEVATION_READ_REQUEST))
	{
		Result = Elevation.fOpenVirtualDisk(VirtualStorageType, NtObject, VirtualDiskAccessMask, Flags, Parameters, Handle);
	}
	return Result;
}
