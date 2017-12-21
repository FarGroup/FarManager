/*
platform.fs.cpp

*/
/*
Copyright © 2017 Far Group
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

#include "platform.fs.hpp"

#include "ctrlobj.hpp"
#include "cvtname.hpp"
#include "datetime.hpp"
#include "drivemix.hpp"
#include "elevation.hpp"
#include "flink.hpp"
#include "imports.hpp"
#include "pathmix.hpp"
#include "plugins.hpp"
#include "string_utils.hpp"
#include "strmix.hpp"

namespace
{
	class i_find_file_handle_impl
	{
	public:
		virtual ~i_find_file_handle_impl() = 0;
	};

	i_find_file_handle_impl::~i_find_file_handle_impl() = default;

	class far_find_file_handle_impl: public i_find_file_handle_impl
	{
	public:
		os::fs::file Object;
		block_ptr<BYTE> BufferBase;
		block_ptr<BYTE> Buffer2;
		ULONG NextOffset{};
		bool Extended{};
		bool ReadDone{};
	};

	class os_find_file_handle_impl: public i_find_file_handle_impl, public os::fs::find_handle
	{
		using os::fs::find_handle::find_handle;
	};
}

namespace os::fs
{
	namespace detail
	{
		void find_handle_closer::operator()(HANDLE Handle) const
		{
			FindClose(Handle);
		}

		void find_file_handle_closer::operator()(HANDLE Handle) const
		{
			delete static_cast<i_find_file_handle_impl*>(Handle);
		}

		void find_volume_handle_closer::operator()(HANDLE Handle) const
		{
			FindVolumeClose(Handle);
		}

		void find_notification_handle_closer::operator()(HANDLE Handle) const
		{
			FindCloseChangeNotification(Handle);
		}
	}

	bool is_standard_drive_letter(wchar_t Letter)
	{
		return InRange(L'A', upper(Letter), L'Z');
	}

	int get_drive_number(wchar_t Letter)
	{
		return upper(Letter) - L'A';
	}

	string get_drive(wchar_t Letter)
	{
		return { Letter, L':' };
	}

	string get_root_directory(wchar_t Letter)
	{
		return { Letter, L':', L'\\' };
	}

	//-------------------------------------------------------------------------

	enum_drives::enum_drives(drives_set Drives):
		m_Drives(Drives)
	{
	}

	bool enum_drives::get(size_t Index, wchar_t& Value) const
	{
		if (!Index)
			m_CurrentIndex = 0;

		while (m_CurrentIndex != m_Drives.size() && !m_Drives[m_CurrentIndex])
			++m_CurrentIndex;

		if (m_CurrentIndex == m_Drives.size())
			return false;

		Value = static_cast<wchar_t>(L'A' + m_CurrentIndex);
		++m_CurrentIndex;
		return true;
	}

	//-------------------------------------------------------------------------

	static string enum_files_prepare(const string_view& Object)
	{
		auto PreparedObject = NTPath(Object);
		auto Root = false;
		const auto Type = ParsePath(PreparedObject, nullptr, &Root);
		if (Root && (Type == root_type::drive_letter || Type == root_type::unc_drive_letter || Type == root_type::volume))
		{
			AddEndSlash(PreparedObject);
		}
		else
		{
			DeleteEndSlash(PreparedObject);
		}
		return PreparedObject;
	}

	enum_files::enum_files(const string_view& Object, bool ScanSymlink):
		m_Object(enum_files_prepare(Object)),
		m_ScanSymlink(ScanSymlink)
	{
	}

	static void DirectoryInfoToFindData(const FILE_ID_BOTH_DIR_INFORMATION& DirectoryInfo, find_data& FindData, bool IsExtended)
	{
		FindData.dwFileAttributes = DirectoryInfo.FileAttributes;
		FindData.CreationTime = os::chrono::time_point(os::chrono::duration(DirectoryInfo.CreationTime.QuadPart));
		FindData.LastAccessTime = os::chrono::time_point(os::chrono::duration(DirectoryInfo.LastAccessTime.QuadPart));
		FindData.LastWriteTime = os::chrono::time_point(os::chrono::duration(DirectoryInfo.LastWriteTime.QuadPart));
		FindData.ChangeTime = os::chrono::time_point(os::chrono::duration(DirectoryInfo.ChangeTime.QuadPart));
		FindData.nFileSize = DirectoryInfo.EndOfFile.QuadPart;
		FindData.nAllocationSize = DirectoryInfo.AllocationSize.QuadPart;
		FindData.dwReserved0 = FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT? DirectoryInfo.EaSize : 0;

		const auto& CopyNames = [&FindData](const auto& DirInfo)
		{
			FindData.strFileName.assign(DirInfo.FileName, DirInfo.FileNameLength / sizeof(wchar_t));
			FindData.strAlternateFileName.assign(DirInfo.ShortName, DirInfo.ShortNameLength / sizeof(wchar_t));
		};

		if (IsExtended)
		{
			FindData.FileId = DirectoryInfo.FileId.QuadPart;
			CopyNames(DirectoryInfo);
		}
		else
		{
			FindData.FileId = 0;
			CopyNames(reinterpret_cast<const FILE_BOTH_DIR_INFORMATION&>(DirectoryInfo));
		}

		const auto& RemoveTrailingZeros = [](string& Where)
		{
			const auto Pos = Where.find_last_not_of(L'\0');
			Pos != string::npos? Where.resize(Pos + 1) : Where.clear();
		};

		// MSDN verse 2.4.17:
		// "When working with this field, use FileNameLength to determine the length of the file name
		// rather than assuming the presence of a trailing null delimiter."

		// Some buggy implementations (e. g. ms sharepoint, rdesktop) set the length incorrectly
		// (e. g. including the terminating \0 or as ((number of bytes in the source string) * 2) when source is in UTF-8),
		// so instead of, say, "name" (4) they return "name\0\0\0\0" (8).
		// Generally speaking, it's their own problems and we shall use it as is, as per the verse above.
		// However, most of the applications use FindFirstFile API, which copies this string
		// to the fixed-size buffer WIN32_FIND_DATA.cFileName, leaving the burden of finding its length
		// to the application itself, which, by coincidence, does it correctly, effectively masking the initial error.
		// So people come to us and claim that Far isn't working properly while other programs are fine.

		RemoveTrailingZeros(FindData.strFileName);
		RemoveTrailingZeros(FindData.strAlternateFileName);

		// Some other buggy implementations just set the first char of AlternateFileName to '\0' to make it "empty", leaving rubbish in others. Double facepalm.
		if (!FindData.strAlternateFileName.empty() && FindData.strAlternateFileName.front() == L'\0')
			FindData.strAlternateFileName.clear();
	}

	static find_file_handle FindFirstFileInternal(const string& Name, find_data& FindData)
	{
		if (Name.empty() || IsSlash(Name.back()))
			return nullptr;

		auto Handle = std::make_unique<far_find_file_handle_impl>();

		auto strDirectory(Name);
		CutToSlash(strDirectory);

		const auto& OpenDirectory = [&]
		{
			return Handle->Object.Open(strDirectory, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING);
		};

		if (!OpenDirectory())
		{
			// fix error code if we are looking for FILE(S) in a non-existent directory, not for a directory itself
			if (GetLastError() == ERROR_FILE_NOT_FOUND && !PointToName(Name).empty())
			{
				SetLastError(ERROR_PATH_NOT_FOUND);
			}
			return nullptr;
		}

		// for network paths buffer size must be <= 64k
		Handle->BufferBase.reset(0x10000);

		const auto NamePart = PointToName(Name);
		Handle->Extended = true;

		bool QueryResult = Handle->Object.NtQueryDirectoryFile(Handle->BufferBase.get(), Handle->BufferBase.size(), FileIdBothDirectoryInformation, false, null_terminated(NamePart).data(), true);
		if (QueryResult) // try next read immediately to avoid M#2128 bug
		{
			block_ptr<BYTE> Buffer2(Handle->BufferBase.size());
			if (Handle->Object.NtQueryDirectoryFile(Buffer2.get(), Buffer2.size(), FileIdBothDirectoryInformation, false, null_terminated(NamePart).data(), false))
			{
				Handle->Buffer2 = std::move(Buffer2);
			}
			else
			{
				if (GetLastError() != ERROR_INVALID_LEVEL)
					Handle->ReadDone = true;
				else
					QueryResult = false;
			}
		}

		if (!QueryResult)
		{
			Handle->Extended = false;

			// recreate the handle to avoid weird bugs with some network emulators
			Handle->Object.Close();
			if (OpenDirectory())
			{
				QueryResult = Handle->Object.NtQueryDirectoryFile(Handle->BufferBase.get(), Handle->BufferBase.size(), FileBothDirectoryInformation, false, null_terminated(NamePart).data(), true);
			}
		}

		if (!QueryResult)
			return nullptr;

		const auto DirectoryInfo = reinterpret_cast<const FILE_ID_BOTH_DIR_INFORMATION*>(Handle->BufferBase.get());
		DirectoryInfoToFindData(*DirectoryInfo, FindData, Handle->Extended);
		Handle->NextOffset = DirectoryInfo->NextEntryOffset;
		return find_file_handle(Handle.release());
	}

	static bool FindNextFileInternal(const find_file_handle& Find, find_data& FindData)
	{
		bool Result = false;
		const auto Handle = static_cast<far_find_file_handle_impl*>(Find.native_handle());
		bool Status = true, set_errcode = true;
		auto DirectoryInfo = reinterpret_cast<const FILE_ID_BOTH_DIR_INFORMATION*>(Handle->BufferBase.get());
		if (Handle->NextOffset)
		{
			DirectoryInfo = reinterpret_cast<const FILE_ID_BOTH_DIR_INFORMATION*>(reinterpret_cast<const char*>(DirectoryInfo) + Handle->NextOffset);
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
					using std::swap;
					swap(Handle->BufferBase, Handle->Buffer2);
					DirectoryInfo = reinterpret_cast<const FILE_ID_BOTH_DIR_INFORMATION*>(Handle->BufferBase.get());
				}
				else
				{
					Status = Handle->Object.NtQueryDirectoryFile(Handle->BufferBase.get(), Handle->BufferBase.size(), Handle->Extended? FileIdBothDirectoryInformation : FileBothDirectoryInformation, false, nullptr, false);
					set_errcode = false;
				}
			}
		}

		if (Status)
		{
			DirectoryInfoToFindData(*DirectoryInfo, FindData, Handle->Extended);
			Handle->NextOffset = DirectoryInfo->NextEntryOffset? Handle->NextOffset + DirectoryInfo->NextEntryOffset : 0;
			Result = true;
		}

		if (set_errcode)
			SetLastError(Result? ERROR_SUCCESS : ERROR_NO_MORE_FILES);

		return Result;
	}

	bool enum_files::get(size_t index, find_data& Value) const
	{
		if (!index)
		{
			// temporarily disable elevation to try the requested name first
			{
				SCOPED_ACTION(elevation::suppress);
				m_Handle = FindFirstFileInternal(m_Object, Value);
			}

			if (!m_Handle && GetLastError() == ERROR_ACCESS_DENIED)
			{
				if (m_ScanSymlink)
				{
					auto strReal = m_Object;
					// only links in the path should be processed, not the object name itself
					CutToSlash(strReal);
					strReal = ConvertNameToReal(strReal);
					AddEndSlash(strReal);
					append(strReal, PointToName(m_Object));
					strReal = NTPath(strReal);
					m_Handle = FindFirstFileInternal(strReal, Value);
				}

				if (!m_Handle && ElevationRequired(ELEVATION_READ_REQUEST))
				{
					m_Handle = FindFirstFileInternal(m_Object, Value);
				}
			}

			if (!m_Handle)
				return false;
		}
		else
		{
			if (!FindNextFileInternal(m_Handle, Value))
				return false;
		}

		// skip ".." & "."
		if (Value.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
			Value.strFileName[0] == L'.' && ((Value.strFileName.size() == 2 && Value.strFileName[1] == L'.') || Value.strFileName.size() == 1) &&
			// These "virtual" folders either don't have an SFN at all or it's the same as LFN:
			(Value.strAlternateFileName.empty() || Value.strAlternateFileName == Value.strFileName))
		{
			// index is not important here, anything but 0 is fine
			return get(1, Value);
		}

		return true;
	}

	//-------------------------------------------------------------------------

	find_handle FindFirstFileName(const string_view& FileName, DWORD Flags, string& LinkName)
	{
		NTPath NtFileName(FileName);
		find_handle Handle;
		os::detail::ApiDynamicStringReceiver(LinkName, [&](wchar_t* Buffer, size_t Size)
		{
			auto BufferSize = static_cast<DWORD>(Size);
			Handle.reset(Imports().FindFirstFileNameW(NtFileName.data(), Flags, &BufferSize, Buffer));
			if (Handle)
				// FindFirstFileNameW always includes terminating \0 in the returned size
				return BufferSize - 1;
			return GetLastError() == ERROR_MORE_DATA? BufferSize : 0;
		});
		return Handle;
	}

	bool FindNextFileName(const find_handle& hFindStream, string& LinkName)
	{
		return os::detail::ApiDynamicStringReceiver(LinkName, [&](wchar_t* Buffer, size_t Size)
		{
			auto BufferSize = static_cast<DWORD>(Size);
			if (Imports().FindNextFileNameW(hFindStream.native_handle(), &BufferSize, Buffer))
				// FindNextFileNameW always includes terminating \0 in the returned size
				return BufferSize - 1;
			return GetLastError() == ERROR_MORE_DATA? BufferSize : 0;
		});
	}

	enum_names::enum_names(const string_view& Object):
		m_Object(make_string(Object))
	{
	}

	bool enum_names::get(size_t Index, string& Value) const
	{
		if (!Index)
		{
			m_Handle = FindFirstFileName(m_Object, 0, Value);
			return m_Handle != nullptr;
		}

		return FindNextFileName(m_Handle, Value);
	}

	//-------------------------------------------------------------------------

	static bool FileStreamInformationToFindStreamData(const FILE_STREAM_INFORMATION& StreamInfo, WIN32_FIND_STREAM_DATA& StreamData)
	{
		const auto Length = StreamInfo.StreamNameLength / sizeof(wchar_t);
		if (!Length || Length >= std::size(StreamData.cStreamName))
			return false;

		*std::copy_n(std::cbegin(StreamInfo.StreamName), Length, StreamData.cStreamName) = L'\0';
		StreamData.StreamSize = StreamInfo.StreamSize;
		return true;
	}

	find_file_handle FindFirstStream(const string_view& FileName, STREAM_INFO_LEVELS InfoLevel, void* FindStreamData, DWORD Flags)
	{
		if (Imports().FindFirstStreamW && Imports().FindNextStreamW)
		{
			os_find_file_handle_impl Handle(Imports().FindFirstStreamW(NTPath(FileName).data(), InfoLevel, FindStreamData, Flags));
			return find_file_handle(Handle? std::make_unique<os_find_file_handle_impl>(Handle.release()).release() : nullptr);
		}

		if (InfoLevel != FindStreamInfoStandard)
			return nullptr;

		auto Handle = std::make_unique<far_find_file_handle_impl>();

		if (!Handle->Object.Open(FileName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING))
			return nullptr;

		// for network paths buffer size must be <= 64k
		// we double it in a first loop, so starting value is 32k
		size_t BufferSize = 0x8000;
		NTSTATUS Result = STATUS_SEVERITY_ERROR;
		do
		{
			BufferSize *= 2;
			Handle->BufferBase.reset(BufferSize);
			// sometimes for directories NtQueryInformationFile returns STATUS_SUCCESS but doesn't fill the buffer
			const auto StreamInfo = reinterpret_cast<FILE_STREAM_INFORMATION*>(Handle->BufferBase.get());
			StreamInfo->StreamNameLength = 0;
			Handle->Object.NtQueryInformationFile(Handle->BufferBase.get(), Handle->BufferBase.size(), FileStreamInformation, &Result);
		}
		while (Result == STATUS_BUFFER_OVERFLOW || Result == STATUS_BUFFER_TOO_SMALL);

		if (Result != STATUS_SUCCESS)
			return nullptr;

		const auto StreamInfo = reinterpret_cast<const FILE_STREAM_INFORMATION*>(Handle->BufferBase.get());
		Handle->NextOffset = StreamInfo->NextEntryOffset;
		const auto StreamData = static_cast<WIN32_FIND_STREAM_DATA*>(FindStreamData);

		if (!FileStreamInformationToFindStreamData(*StreamInfo, *StreamData))
			return nullptr;

		return find_file_handle(Handle.release());
	}

	bool FindNextStream(const find_file_handle& hFindStream, void* FindStreamData)
	{
		if (Imports().FindFirstStreamW && Imports().FindNextStreamW)
		{
			return Imports().FindNextStreamW(static_cast<os_find_file_handle_impl*>(hFindStream.native_handle())->native_handle(), FindStreamData) != FALSE;
		}

		const auto Handle = static_cast<far_find_file_handle_impl*>(hFindStream.native_handle());

		if (!Handle->NextOffset)
			return false;

		const auto StreamInfo = reinterpret_cast<const FILE_STREAM_INFORMATION*>(Handle->BufferBase.get() + Handle->NextOffset);
		Handle->NextOffset = StreamInfo->NextEntryOffset? Handle->NextOffset + StreamInfo->NextEntryOffset : 0;
		const auto StreamData = static_cast<WIN32_FIND_STREAM_DATA*>(FindStreamData);

		return FileStreamInformationToFindStreamData(*StreamInfo, *StreamData);
	}

	enum_streams::enum_streams(const string_view& Object):
		m_Object(make_string(Object))
	{
	}

	bool enum_streams::get(size_t Index, WIN32_FIND_STREAM_DATA& Value) const
	{
		if (!Index)
		{
			m_Handle = FindFirstStream(m_Object, FindStreamInfoStandard, &Value, 0);
			return m_Handle != nullptr;
		}

		return FindNextStream(m_Handle, &Value);
	}

	//-------------------------------------------------------------------------

	enum_volumes::enum_volumes() = default;

	bool enum_volumes::get(size_t Index, string& Value) const
	{
		wchar_t VolumeName[50];

		if (!Index)
		{
			m_Handle.reset(FindFirstVolume(VolumeName, static_cast<DWORD>(std::size(VolumeName))));
			if (!m_Handle)
			{
				return false;
			}
		}
		else
		{
			if (!FindNextVolume(m_Handle.native_handle(), VolumeName, static_cast<DWORD>(std::size(VolumeName))))
			{
				return false;
			}
		}

		Value = VolumeName;
		return true;
	}

	//-------------------------------------------------------------------------

	bool file::operator!() const noexcept
	{
		return !m_Handle;
	}

	bool file::Open(const string_view& Object, DWORD DesiredAccess, DWORD ShareMode, SECURITY_ATTRIBUTES* SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, const file* TemplateFile, bool ForceElevation)
	{
		assert(!m_Handle);

		m_Pointer = 0;
		m_NeedSyncPointer = false;

		const auto TemplateFileHandle = TemplateFile? TemplateFile->m_Handle.native_handle() : nullptr;

		m_Handle = create_file(Object, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFileHandle, ForceElevation);

		if (!m_Handle)
		{
			m_Name.clear();
			m_ShareMode = 0;
			return false;
		}

		m_Name = make_string(Object);
		m_ShareMode = ShareMode;
		return true;
	}

	bool file::Read(void* Buffer, size_t NumberOfBytesToRead, size_t& NumberOfBytesRead) const
	{
		assert(NumberOfBytesToRead <= std::numeric_limits<DWORD>::max());

		SyncPointer();

		NumberOfBytesRead = 0;
		DWORD BytesRead = 0;

		if (!ReadFile(m_Handle.native_handle(), Buffer, static_cast<DWORD>(NumberOfBytesToRead), &BytesRead, nullptr))
			return false;

		NumberOfBytesRead = BytesRead;
		m_Pointer += NumberOfBytesRead;
		return true;
	}

	bool file::Write(const void* Buffer, size_t NumberOfBytesToWrite) const
	{
		assert(NumberOfBytesToWrite <= std::numeric_limits<DWORD>::max());

		SyncPointer();

		DWORD BytesWritten = 0;
		if (!WriteFile(m_Handle.native_handle(), Buffer, static_cast<DWORD>(NumberOfBytesToWrite), &BytesWritten, nullptr))
			return false;

		m_Pointer += BytesWritten;
		return true;
	}

	unsigned long long file::GetPointer() const
	{
		return m_Pointer;
	}

	bool file::SetPointer(long long DistanceToMove, unsigned long long* NewFilePointer, DWORD MoveMethod) const
	{
		const auto OldPointer = m_Pointer;

		switch (MoveMethod)
		{
		case FILE_BEGIN:
			m_Pointer = DistanceToMove;
			break;

		case FILE_CURRENT:
			m_Pointer += DistanceToMove;
			break;

		case FILE_END:
			{
				unsigned long long Size = 0;
				GetSize(Size);
				m_Pointer = Size + DistanceToMove;
			}
			break;
		}

		if (OldPointer != m_Pointer)
		{
			m_NeedSyncPointer = true;
		}

		if (NewFilePointer)
		{
			*NewFilePointer = m_Pointer;
		}

		return true;
	}

	bool file::SetEnd()
	{
		SyncPointer();
		bool ok = SetEndOfFile(m_Handle.native_handle()) != FALSE;
		if (!ok && !m_Name.empty() && GetLastError() == ERROR_INVALID_PARAMETER) // OSX buggy SMB workaround
		{
			const auto fsize = GetPointer();
			Close();
			if (Open(m_Name, GENERIC_WRITE, m_ShareMode, nullptr, OPEN_EXISTING, 0))
			{
				SetPointer(fsize, nullptr, FILE_BEGIN);
				SyncPointer();
				ok = SetEndOfFile(m_Handle.native_handle()) != FALSE;
			}
		}
		return ok;
	}

	bool file::GetTime(os::chrono::time_point* CreationTime, os::chrono::time_point* LastAccessTime, os::chrono::time_point* LastWriteTime, os::chrono::time_point* ChangeTime) const
	{
		FILE_BASIC_INFORMATION fbi;

		if (!NtQueryInformationFile(&fbi, sizeof fbi, FileBasicInformation))
			return false;

		if (CreationTime)
			*CreationTime = os::chrono::time_point(os::chrono::duration(fbi.CreationTime.QuadPart));

		if (LastAccessTime)
			*LastAccessTime = os::chrono::time_point(os::chrono::duration(fbi.LastAccessTime.QuadPart));

		if (LastWriteTime)
			*LastWriteTime = os::chrono::time_point(os::chrono::duration(fbi.LastWriteTime.QuadPart));

		if (ChangeTime)
			*ChangeTime = os::chrono::time_point(os::chrono::duration(fbi.ChangeTime.QuadPart));

		return true;
	}

	bool file::SetTime(const os::chrono::time_point* CreationTime, const os::chrono::time_point* LastAccessTime, const os::chrono::time_point* LastWriteTime, const os::chrono::time_point* ChangeTime) const
	{
		FILE_BASIC_INFORMATION fbi{};

		if (CreationTime)
			fbi.CreationTime.QuadPart = CreationTime->time_since_epoch().count();

		if (LastAccessTime)
			fbi.LastAccessTime.QuadPart = LastAccessTime->time_since_epoch().count();

		if (LastWriteTime)
			fbi.LastWriteTime.QuadPart = LastWriteTime->time_since_epoch().count();

		if (ChangeTime)
			fbi.ChangeTime.QuadPart = ChangeTime->time_since_epoch().count();

		IO_STATUS_BLOCK IoStatusBlock;
		const auto Status = Imports().NtSetInformationFile(m_Handle.native_handle(), &IoStatusBlock, &fbi, sizeof fbi, FileBasicInformation);
		::SetLastError(Imports().RtlNtStatusToDosError(Status));

		return Status == STATUS_SUCCESS;
	}

	bool file::GetSize(unsigned long long& Size) const
	{
		if (::GetFileSizeEx(m_Handle.native_handle(), reinterpret_cast<PLARGE_INTEGER>(&Size)))
			return true;

		GET_LENGTH_INFORMATION gli;
		DWORD BytesReturned;

		if (!IoControl(IOCTL_DISK_GET_LENGTH_INFO, nullptr, 0, &gli, sizeof(gli), &BytesReturned))
			return false;

		Size = gli.Length.QuadPart;
		return true;
	}

	bool file::FlushBuffers() const
	{
		return FlushFileBuffers(m_Handle.native_handle()) != FALSE;
	}

	bool file::GetInformation(BY_HANDLE_FILE_INFORMATION& info) const
	{
		return GetFileInformationByHandle(m_Handle.native_handle(), &info) != FALSE;
	}

	bool file::IoControl(DWORD IoControlCode, void* InBuffer, DWORD InBufferSize, void* OutBuffer, DWORD OutBufferSize, DWORD* BytesReturned, OVERLAPPED* Overlapped) const
	{
		return ::DeviceIoControl(m_Handle.native_handle(), IoControlCode, InBuffer, InBufferSize, OutBuffer, OutBufferSize, BytesReturned, Overlapped) != FALSE;
	}

	bool file::GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed) const
	{
		DWORD Result = Imports().GetStorageDependencyInformation(m_Handle.native_handle(), Flags, StorageDependencyInfoSize, StorageDependencyInfo, SizeUsed);
		SetLastError(Result);
		return Result == ERROR_SUCCESS;
	}

	bool file::NtQueryDirectoryFile(void* FileInformation, size_t Length, FILE_INFORMATION_CLASS FileInformationClass, bool ReturnSingleEntry, const wchar_t* FileName, bool RestartScan, NTSTATUS* Status) const
	{
		IO_STATUS_BLOCK IoStatusBlock;
		PUNICODE_STRING pNameString = nullptr;
		UNICODE_STRING NameString;
		if (FileName && *FileName)
		{
			NameString.Buffer = const_cast<LPWSTR>(FileName);
			NameString.Length = static_cast<USHORT>(wcslen(FileName) * sizeof(WCHAR));
			NameString.MaximumLength = NameString.Length;
			pNameString = &NameString;
		}
		const auto di = reinterpret_cast<FILE_ID_BOTH_DIR_INFORMATION*>(FileInformation);
		di->NextEntryOffset = 0xffffffffUL;

		NTSTATUS Result = Imports().NtQueryDirectoryFile(m_Handle.native_handle(), nullptr, nullptr, nullptr, &IoStatusBlock, FileInformation, static_cast<ULONG>(Length), FileInformationClass, ReturnSingleEntry, pNameString, RestartScan);
		SetLastError(Imports().RtlNtStatusToDosError(Result));
		if (Status)
		{
			*Status = Result;
		}

		return (Result == STATUS_SUCCESS) && (di->NextEntryOffset != 0xffffffffUL);
	}

	bool file::NtQueryInformationFile(void* FileInformation, size_t Length, FILE_INFORMATION_CLASS FileInformationClass, NTSTATUS* Status) const
	{
		IO_STATUS_BLOCK IoStatusBlock;
		NTSTATUS Result = Imports().NtQueryInformationFile(m_Handle.native_handle(), &IoStatusBlock, FileInformation, static_cast<ULONG>(Length), FileInformationClass);
		SetLastError(Imports().RtlNtStatusToDosError(Result));
		if (Status)
		{
			*Status = Result;
		}
		return Result == STATUS_SUCCESS;
	}

	static bool GetObjectName(HANDLE hFile, string& ObjectName)
	{
		block_ptr<OBJECT_NAME_INFORMATION> oni(NT_MAX_PATH);
		ULONG ReturnLength;

		const auto& QueryObject = [&]
		{
			return Imports().NtQueryObject(hFile, ObjectNameInformation, oni.get(), static_cast<unsigned long>(oni.size()), &ReturnLength);
		};

		auto Result = QueryObject();
		if (Result == STATUS_BUFFER_OVERFLOW || Result == STATUS_BUFFER_TOO_SMALL)
		{
			oni.reset(ReturnLength);
			Result = QueryObject();
		}

		if (Result != STATUS_SUCCESS)
			return false;

		ObjectName.assign(oni->Name.Buffer, oni->Name.Length / sizeof(WCHAR));
		return true;
	}

	static int MatchNtPathRoot(const string& NtPath, const string& DeviceName)
	{
		string TargetPath;
		if (!os::fs::QueryDosDevice(DeviceName, TargetPath))
			return 0;

		if (PathStartsWith(NtPath, TargetPath))
			return static_cast<int>(TargetPath.size());

		// path could be an Object Manager symlink, try to resolve
		UNICODE_STRING ObjName;
		ObjName.Length = ObjName.MaximumLength = static_cast<USHORT>(TargetPath.size() * sizeof(wchar_t));
		ObjName.Buffer = UNSAFE_CSTR(TargetPath);
		OBJECT_ATTRIBUTES ObjAttrs;
		InitializeObjectAttributes(&ObjAttrs, &ObjName, 0, nullptr, nullptr);

		HANDLE hSymLink;
		if (Imports().NtOpenSymbolicLinkObject(&hSymLink, GENERIC_READ, &ObjAttrs) != STATUS_SUCCESS)
			return 0;

		SCOPE_EXIT{ Imports().NtClose(hSymLink); };

		ULONG BufSize = 32767;
		wchar_t_ptr Buffer(BufSize);
		UNICODE_STRING LinkTarget;
		LinkTarget.MaximumLength = static_cast<USHORT>(BufSize * sizeof(wchar_t));
		LinkTarget.Buffer = Buffer.get();

		if (Imports().NtQuerySymbolicLinkObject(hSymLink, &LinkTarget, nullptr) != STATUS_SUCCESS)
			return 0;

		TargetPath.assign(LinkTarget.Buffer, LinkTarget.Length / sizeof(wchar_t));

		if (PathStartsWith(NtPath, TargetPath))
			return static_cast<int>(TargetPath.size());

		return 0;
	}

	static bool internalNtQueryGetFinalPathNameByHandle(HANDLE hFile, string& FinalFilePath)
	{
		string NtPath;
		if (!GetObjectName(hFile, NtPath))
			return false;

		const auto& ReplaceRoot = [&](const auto& OldRoot, const auto& NewRoot)
		{
			if (!starts_with(NtPath, OldRoot))
				return false;

			FinalFilePath = NtPath.replace(0, OldRoot.size(), NewRoot.raw_data(), NewRoot.size());
			return true;
		};

		// simple way to handle network paths
		if (ReplaceRoot(L"\\Device\\LanmanRedirector"_sv, L"\\"_sv) || ReplaceRoot(L"\\Device\\Mup"_sv, L"\\"_sv))
			return true;

		// try to convert NT path (\Device\HarddiskVolume1) to drive letter
		for (const auto& i : enum_drives(get_logical_drives()))
		{
			const auto Device = fs::get_drive(i);
			if (const auto Len = MatchNtPathRoot(NtPath, Device))
			{
				FinalFilePath = starts_with(NtPath, L"\\Device\\WinDfs"_sv)? NtPath.replace(0, Len, 1, L'\\') : NtPath.replace(0, Len, Device);
				return true;
			}
		}

		// try to convert NT path (\Device\HarddiskVolume1) to \\?\Volume{...} path
		for (auto& VolumeName : enum_volumes())
		{
			if (const auto Len = MatchNtPathRoot(NtPath, VolumeName.substr(4, VolumeName.size() - 5))) // w/o prefix and trailing slash
			{
				FinalFilePath = NtPath.replace(0, Len, VolumeName);
				return true;
			}
		}

		return false;
	}

	bool file::GetFinalPathName(string& FinalFilePath) const
	{
		const auto& GetFinalPathNameByHandleGuarded = [](HANDLE File, wchar_t* Buffer, DWORD Size, DWORD Flags)
		{
			// It seems that Microsoft has forgotten to put an exception handler around this function.
			// It causes EXCEPTION_ACCESS_VIOLATION (read from 0) in kernel32 under certain conditions,
			// e.g. badly written file system drivers or weirdly formatted volumes.
			return seh_invoke_no_ui(
			[&]
			{
				return Imports().GetFinalPathNameByHandle(File, Buffer, Size, Flags);
			},
			[]
			{
				SetLastError(ERROR_UNHANDLED_EXCEPTION);
				return 0ul;
			});
		};

		const auto& GetFinalPathNameByHandleImpl = [&]
		{
			return os::detail::ApiDynamicStringReceiver(FinalFilePath, [&](wchar_t* Buffer, size_t Size)
			{
				return GetFinalPathNameByHandleGuarded(m_Handle.native_handle(), Buffer, static_cast<DWORD>(Size), VOLUME_NAME_GUID);
			});
		};

		return (Imports().GetFinalPathNameByHandleW && GetFinalPathNameByHandleImpl()) || internalNtQueryGetFinalPathNameByHandle(m_Handle.native_handle(), FinalFilePath);
	}

	void file::Close()
	{
		m_Handle.close();
	}

	bool file::Eof() const
	{
		const auto Ptr = GetPointer();
		unsigned long long Size = 0;
		GetSize(Size);
		return Ptr >= Size;
	}

	const string& file::GetName() const
	{
		return m_Name;
	}

	const handle& file::get() const
	{
		return m_Handle;
	}

	void file::SyncPointer() const
	{
		if (!m_NeedSyncPointer)
			return;

		LARGE_INTEGER Distance, NewPointer;
		Distance.QuadPart = m_Pointer;
		if (!SetFilePointerEx(m_Handle.native_handle(), Distance, &NewPointer, FILE_BEGIN))
			return;

		m_Pointer = NewPointer.QuadPart;
		m_NeedSyncPointer = false;
	}

	//-------------------------------------------------------------------------

	struct file_walker::Chunk
	{
		unsigned long long Offset;
		DWORD Size;

		Chunk(unsigned long long Offset, DWORD Size):
			Offset(Offset),
			Size(Size)
		{
		}
	};

	file_walker::file_walker() = default;

	file_walker::~file_walker() = default;

	bool file_walker::InitWalk(size_t BlockSize)
	{
		if (!GetSize(m_FileSize) || !m_FileSize)
			return false;

		m_ChunkSize = static_cast<DWORD>(BlockSize);

		BY_HANDLE_FILE_INFORMATION bhfi;
		m_IsSparse = GetInformation(bhfi) && bhfi.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE;

		if (!m_IsSparse)
		{
			m_AllocSize = m_FileSize;
			m_ChunkList.emplace_back(0, static_cast<DWORD>(std::min(static_cast<unsigned long long>(BlockSize), m_FileSize)));
			m_CurrentChunk = m_ChunkList.begin();
			return true;
		}

		FILE_ALLOCATED_RANGE_BUFFER QueryRange{};
		QueryRange.Length.QuadPart = m_FileSize;

		for (;;)
		{
			FILE_ALLOCATED_RANGE_BUFFER Ranges[1024];
			DWORD BytesReturned;
			const auto QueryResult = IoControl(FSCTL_QUERY_ALLOCATED_RANGES, &QueryRange, sizeof(QueryRange), Ranges, sizeof(Ranges), &BytesReturned);
			if ((!QueryResult && GetLastError() != ERROR_MORE_DATA) || !BytesReturned)
				break;

			for (const auto& i : make_range(Ranges, BytesReturned / sizeof(*Ranges)))
			{
				m_AllocSize += i.Length.QuadPart;
				for (unsigned long long j = i.FileOffset.QuadPart, RangeEndOffset = i.FileOffset.QuadPart + i.Length.QuadPart; j < RangeEndOffset; j += m_ChunkSize)
				{
					m_ChunkList.emplace_back(j, static_cast<DWORD>(std::min(RangeEndOffset - j, static_cast<unsigned long long>(m_ChunkSize))));
				}
			}

			QueryRange.FileOffset.QuadPart = m_ChunkList.back().Offset + m_ChunkList.back().Size;
			QueryRange.Length.QuadPart = m_FileSize - QueryRange.FileOffset.QuadPart;
		}

		m_CurrentChunk = m_ChunkList.begin();
		return !m_ChunkList.empty();
	}

	bool file_walker::Step()
	{
		if (!m_IsSparse)
		{
			const auto NewOffset = (!m_CurrentChunk->Size) ? 0 : m_CurrentChunk->Offset + m_ChunkSize;
			if (NewOffset >= m_FileSize)
				return false;

			m_CurrentChunk->Offset = NewOffset;
			const auto rest = m_FileSize - NewOffset;
			m_CurrentChunk->Size = (rest >= m_ChunkSize) ? m_ChunkSize : rest;
			m_ProcessedSize += m_CurrentChunk->Size;
			return true;
		}

		++m_CurrentChunk;
		if (m_CurrentChunk == m_ChunkList.end())
			return false;

		SetPointer(m_CurrentChunk->Offset, nullptr, FILE_BEGIN);
		m_ProcessedSize += m_CurrentChunk->Size;
		return true;
	}

	unsigned long long file_walker::GetChunkOffset() const
	{
		return m_CurrentChunk->Offset;
	}

	DWORD file_walker::GetChunkSize() const
	{
		return m_CurrentChunk->Size;
	}

	int file_walker::GetPercent() const
	{
		return m_AllocSize? (m_ProcessedSize) * 100 / m_AllocSize : 0;
	}

	//-------------------------------------------------------------------------

	file_status::file_status():
		m_Data(INVALID_FILE_ATTRIBUTES)
	{
	}

	file_status::file_status(const string_view& Object):
		m_Data(fs::get_file_attributes(Object))
	{
	}

	bool file_status::check(DWORD Data) const
	{
		return m_Data != INVALID_FILE_ATTRIBUTES && m_Data & Data;
	}

	//-------------------------------------------------------------------------

	bool exists(file_status Status)
	{
		return Status.check(~0);
	}

	bool exists(const string_view& Object)
	{
		return exists(file_status(Object));
	}

	bool is_file(file_status Status)
	{
		return exists(Status) && !is_directory(Status);
	}

	bool is_file(const string_view& Object)
	{
		return is_file(file_status(Object));
	}

	bool is_directory(file_status Status)
	{
		return Status.check(FILE_ATTRIBUTE_DIRECTORY);
	}

	bool is_directory(const string_view& Object)
	{
		return is_directory(file_status(Object));
	}

	bool is_not_empty_directory(const string& Object)
	{
		auto Pattern = Object;
		AddEndSlash(Pattern);
		Pattern += L"*";
		const auto Find = enum_files(Pattern);
		return Find.begin() != Find.end();
	}

	//-------------------------------------------------------------------------

	process_current_directory_guard::process_current_directory_guard(bool Active, const std::function<string()>& Provider):
		m_Active(Active && GetProcessRealCurrentDirectory(m_Directory))
	{
		if (m_Active && Provider)
			SetProcessRealCurrentDirectory(Provider());
	}

	process_current_directory_guard::~process_current_directory_guard()
	{
		if (m_Active)
			SetProcessRealCurrentDirectory(m_Directory);
	}

	//-------------------------------------------------------------------------

	namespace low
	{
		HANDLE create_file(const wchar_t* FileName, DWORD DesiredAccess, DWORD ShareMode, SECURITY_ATTRIBUTES* SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile)
		{
			return ::CreateFile(FileName, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
		}

		bool create_directory(const wchar_t* TemplateDirectory, const wchar_t* NewDirectory, SECURITY_ATTRIBUTES* SecurityAttributes)
		{
			return (*TemplateDirectory? ::CreateDirectoryEx(TemplateDirectory, NewDirectory, SecurityAttributes) : ::CreateDirectory(NewDirectory, SecurityAttributes)) != FALSE;
		}

		bool remove_directory(const wchar_t* PathName)
		{
			return ::RemoveDirectory(PathName) != FALSE;
		}

		bool delete_file(const wchar_t* FileName)
		{
			return ::DeleteFile(FileName) != FALSE;
		}

		DWORD get_file_attributes(const wchar_t* FileName)
		{
			return ::GetFileAttributes(FileName);
		}

		bool set_file_attributes(const wchar_t* FileName, DWORD Attributes)
		{
			return ::SetFileAttributes(FileName, Attributes) != FALSE;
		}

		bool create_hard_link(const wchar_t* FileName, const wchar_t* ExistingFileName, SECURITY_ATTRIBUTES* SecurityAttributes)
		{
			return ::CreateHardLink(FileName, ExistingFileName, SecurityAttributes) != FALSE;
		}

		bool copy_file(const wchar_t* ExistingFileName, const wchar_t* NewFileName, LPPROGRESS_ROUTINE ProgressRoutine, void* Data, BOOL* Cancel, DWORD CopyFlags)
		{
			return ::CopyFileEx(ExistingFileName, NewFileName, ProgressRoutine, Data, Cancel, CopyFlags) != FALSE;
		}

		bool move_file(const wchar_t* ExistingFileName, const wchar_t* NewFileName, DWORD Flags)
		{
			return (Flags? ::MoveFileEx(ExistingFileName, NewFileName, Flags) : ::MoveFile(ExistingFileName, NewFileName)) != FALSE;
		}

		bool detach_virtual_disk(const wchar_t* Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType)
		{
			handle Handle;
			DWORD Result = Imports().OpenVirtualDisk(&VirtualStorageType, Object, VIRTUAL_DISK_ACCESS_DETACH, OPEN_VIRTUAL_DISK_FLAG_NONE, nullptr, &ptr_setter(Handle));
			if (Result != ERROR_SUCCESS)
			{
				SetLastError(Result);
				return false;
			}

			Result = Imports().DetachVirtualDisk(Handle.native_handle(), DETACH_VIRTUAL_DISK_FLAG_NONE, 0);
			if (Result != ERROR_SUCCESS)
			{
				SetLastError(Result);
				return false;
			}

			return true;
		}

		bool get_disk_free_space(const wchar_t* DirectoryName, unsigned long long* FreeBytesAvailableToCaller, unsigned long long* TotalNumberOfBytes, unsigned long long* TotalNumberOfFreeBytes)
		{
			ULARGE_INTEGER uFreeBytesAvailableToCaller, uTotalNumberOfBytes, uTotalNumberOfFreeBytes;

			if (!::GetDiskFreeSpaceEx(DirectoryName, &uFreeBytesAvailableToCaller, &uTotalNumberOfBytes, &uTotalNumberOfFreeBytes))
				return false;

			const auto& SetOpt = [](const ULARGE_INTEGER& From, unsigned long long* To)
			{
				if (To)
					*To = From.QuadPart;
			};

			SetOpt(uFreeBytesAvailableToCaller, FreeBytesAvailableToCaller);
			SetOpt(uTotalNumberOfBytes, TotalNumberOfBytes);
			SetOpt(uTotalNumberOfFreeBytes, TotalNumberOfFreeBytes);

			return true;
		}

		bool set_file_encryption(const wchar_t* FileName, bool Encrypt)
		{
			return (Encrypt? ::EncryptFile(FileName) : ::DecryptFile(FileName, 0)) != FALSE;
		}
	}

	//-------------------------------------------------------------------------

	bool GetProcessRealCurrentDirectory(string& Directory)
	{
		return os::detail::ApiDynamicStringReceiver(Directory, [](wchar_t* Buffer, size_t Size)
		{
			return ::GetCurrentDirectory(static_cast<DWORD>(Size), Buffer);
		});
	}

	bool SetProcessRealCurrentDirectory(const string& Directory)
	{
		return ::SetCurrentDirectory(Directory.data()) != FALSE;
	}

	void InitCurrentDirectory()
	{
		string InitCurDir;
		if (GetProcessRealCurrentDirectory(InitCurDir))
		{
			SetCurrentDirectory(InitCurDir);
		}
	}

	static string& CurrentDirectory()
	{
		static string sCurrentDirectory;
		return sCurrentDirectory;
	}

	string GetCurrentDirectory()
	{
		//never give outside world a direct pointer to our internal string
		//who knows what they gonna do
		return CurrentDirectory();
	}

	bool SetCurrentDirectory(const string& PathName, bool Validate)
	{
		// correct path to our standard
		string strDir = PathName;
		ReplaceSlashToBackslash(strDir);
		bool Root = false;
		const auto Type = ParsePath(strDir, nullptr, &Root);
		if (Root && (Type == root_type::drive_letter || Type == root_type::unc_drive_letter || Type == root_type::volume))
		{
			AddEndSlash(strDir);
		}
		else
		{
			DeleteEndSlash(strDir);
		}

		if (strDir == CurrentDirectory())
			return true;

		if (Validate)
		{
			string TestDir = PathName;
			AddEndSlash(TestDir);
			TestDir += L'*';
			find_data fd;
			if (!get_find_data(TestDir, fd))
			{
				DWORD LastError = ::GetLastError();
				if (!(LastError == ERROR_FILE_NOT_FOUND || LastError == ERROR_NO_MORE_FILES))
					return false;
			}
		}

		CurrentDirectory() = strDir;

#ifndef NO_WRAPPER
		// try to synchronize far current directory with process current directory
		if (Global->CtrlObject && Global->CtrlObject->Plugins->OemPluginsPresent())
		{
			SetProcessRealCurrentDirectory(strDir);
		}
#endif // NO_WRAPPER
		return true;
	}

	bool create_directory(const string_view& PathName, SECURITY_ATTRIBUTES* SecurityAttributes)
	{
		return create_directory(L""_sv, PathName, SecurityAttributes);
	}

	bool create_directory(const string_view& TemplateDirectory, const string_view& NewDirectory, SECURITY_ATTRIBUTES* SecurityAttributes)
	{
		NTPath NtNewDirectory(NewDirectory);

		const auto& Create = [&](const string& Template)
		{
			if (low::create_directory(Template.data(), NtNewDirectory.data(), SecurityAttributes))
				return true;

			if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
				return elevation::instance().create_directory(Template, NtNewDirectory, SecurityAttributes);

			return false;
		};

		if (TemplateDirectory.empty())
			return Create({});

		return Create(NTPath(TemplateDirectory)) ||
			// CreateDirectoryEx may fail on some FS, try to create anyway.
			Create({});
	}

	bool remove_directory(const string_view& DirName)
	{
		NTPath strNtName(DirName);
		if (low::remove_directory(strNtName.data()))
			return true;

		const auto IsElevationRequired = ElevationRequired(ELEVATION_MODIFY_REQUEST);

		if (!exists(strNtName))
		{
			// Someone deleted it already,
			// but job is done, no need to report error.
			return true;
		}

		if (IsElevationRequired)
			return elevation::instance().remove_directory(strNtName);

		return false;
	}

	handle create_file(const string_view& Object, DWORD DesiredAccess, DWORD ShareMode, SECURITY_ATTRIBUTES* SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile, bool ForceElevation)
	{
		NTPath strObject(Object);
		FlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
		if (CreationDistribution == OPEN_EXISTING || CreationDistribution == TRUNCATE_EXISTING)
		{
			FlagsAndAttributes |= FILE_FLAG_POSIX_SEMANTICS;
		}

		const auto& create_elevated = [&]
		{
			return handle(elevation::instance().create_file(strObject, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile));
		};

		if (ForceElevation)
			return create_elevated();

		if (auto Handle = handle(low::create_file(strObject.data(), DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile)))
			return Handle;

		const auto LastError = GetLastError();

		if (STATUS_STOPPED_ON_SYMLINK == GetLastNtStatus() && ERROR_STOPPED_ON_SYMLINK != LastError)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return nullptr;
		}

		if (LastError == ERROR_FILE_NOT_FOUND || LastError == ERROR_PATH_NOT_FOUND)
		{
			FlagsAndAttributes &= ~FILE_FLAG_POSIX_SEMANTICS;
			if (auto Handle = handle(low::create_file(strObject.data(), DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile)))
				return Handle;
		}

		if (ElevationRequired(DesiredAccess & (GENERIC_ALL | GENERIC_WRITE | WRITE_OWNER | WRITE_DAC | DELETE | FILE_WRITE_DATA | FILE_ADD_FILE | FILE_APPEND_DATA | FILE_ADD_SUBDIRECTORY | FILE_CREATE_PIPE_INSTANCE | FILE_WRITE_EA | FILE_DELETE_CHILD | FILE_WRITE_ATTRIBUTES)? ELEVATION_MODIFY_REQUEST : ELEVATION_READ_REQUEST) || ForceElevation)
			return create_elevated();

		return nullptr;
	}

	bool delete_file(const string_view& FileName)
	{
		NTPath strNtName(FileName);

		if (low::delete_file(strNtName.data()))
			return true;

		const auto IsElevationRequired = ElevationRequired(ELEVATION_MODIFY_REQUEST);

		if (!exists(strNtName))
		{
			// Someone deleted it already,
			// but job is done, no need to report error.
			return true;
		}

		if (IsElevationRequired)
			return elevation::instance().delete_file(strNtName);

		return false;
	}

	bool copy_file(const string_view& ExistingFileName, const string_view& NewFileName, LPPROGRESS_ROUTINE ProgressRoutine, void* Data, BOOL* Cancel, DWORD CopyFlags)
	{
		NTPath strFrom(ExistingFileName), strTo(NewFileName);
		if (IsSlash(strTo.back()))
		{
			append(strTo, PointToName(strFrom));
		}

		if (low::copy_file(strFrom.data(), strTo.data(), ProgressRoutine, Data, Cancel, CopyFlags))
			return true;

		if (STATUS_STOPPED_ON_SYMLINK == GetLastNtStatus() && ERROR_STOPPED_ON_SYMLINK != GetLastError())
		{
			::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return false;
		}

		if (STATUS_FILE_IS_A_DIRECTORY == GetLastNtStatus())
		{
			::SetLastError(ERROR_FILE_EXISTS);
			return false;
		}

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
			return elevation::instance().copy_file(strFrom, strTo, ProgressRoutine, Data, Cancel, CopyFlags);

		return false;
	}

	bool move_file(const string_view& ExistingFileName, const string_view& NewFileName, DWORD dwFlags)
	{
		NTPath strFrom(ExistingFileName), strTo(NewFileName);
		if (IsSlash(strTo.back()))
		{
			append(strTo, PointToName(strFrom));
		}

		if (low::move_file(strFrom.data(), strTo.data(), dwFlags))
			return true;

		if (STATUS_STOPPED_ON_SYMLINK == GetLastNtStatus() && ERROR_STOPPED_ON_SYMLINK != GetLastError())
		{
			::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return false;
		}

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
		{
			// exclude fake elevation request for: move file over existing directory with same name
			const file_status SrcStatus(strFrom), DstStatus(strTo);
			if (is_directory(DstStatus) && is_file(SrcStatus))
			{
				::SetLastError(ERROR_FILE_EXISTS); // existing directory name == moved file name
				return false;
			}
			return elevation::instance().move_file(strFrom, strTo, dwFlags);
		}

		return false;
	}

	DWORD get_file_attributes(const string_view& FileName)
	{
		NTPath NtName(FileName);

		const auto Result = low::get_file_attributes(NtName.data());
		if (Result != INVALID_FILE_ATTRIBUTES)
			return Result;

		if (ElevationRequired(ELEVATION_READ_REQUEST))
			return elevation::instance().get_file_attributes(NtName);

		return INVALID_FILE_ATTRIBUTES;
	}

	bool set_file_attributes(const string_view& FileName, DWORD Attributes)
	{
		NTPath NtName(FileName);

		if (low::set_file_attributes(NtName.data(), Attributes))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
			return elevation::instance().set_file_attributes(NtName, Attributes);

		return false;
	}

	bool GetLongPathName(const string& ShortPath, string& LongPath)
	{
		return os::detail::ApiDynamicStringReceiver(LongPath, [&](wchar_t* Buffer, size_t Size)
		{
			return ::GetLongPathName(ShortPath.data(), Buffer, static_cast<DWORD>(Size));
		});
	}

	bool GetShortPathName(const string& LongPath, string& ShortPath)
	{
		return os::detail::ApiDynamicStringReceiver(ShortPath, [&](wchar_t* Buffer, size_t Size)
		{
			return ::GetShortPathName(LongPath.data(), Buffer, static_cast<DWORD>(Size));
		});
	}

	bool GetVolumeInformation(const string& RootPathName, string* VolumeName, DWORD* VolumeSerialNumber, DWORD* MaximumComponentLength, DWORD* FileSystemFlags, string* FileSystemName)
	{
		wchar_t VolumeNameBuffer[MAX_PATH + 1], FileSystemNameBuffer[MAX_PATH + 1];
		if (VolumeName)
		{
			VolumeNameBuffer[0] = L'\0';
		}
		if (FileSystemName)
		{
			FileSystemNameBuffer[0] = L'\0';
		}

		if (!::GetVolumeInformation(RootPathName.data(), VolumeNameBuffer, static_cast<DWORD>(std::size(VolumeNameBuffer)), VolumeSerialNumber,
			MaximumComponentLength, FileSystemFlags, FileSystemNameBuffer, static_cast<DWORD>(std::size(FileSystemNameBuffer))))
			return false;

		if (VolumeName)
			*VolumeName = VolumeNameBuffer;

		if (FileSystemName)
			*FileSystemName = FileSystemNameBuffer;

		return true;
	}

	bool GetVolumeNameForVolumeMountPoint(const string& VolumeMountPoint, string& VolumeName)
	{
		WCHAR VolumeNameBuffer[50];
		NTPath strVolumeMountPoint(VolumeMountPoint);
		AddEndSlash(strVolumeMountPoint);
		if (!::GetVolumeNameForVolumeMountPoint(strVolumeMountPoint.data(), VolumeNameBuffer, static_cast<DWORD>(std::size(VolumeNameBuffer))))
			return false;

		VolumeName = VolumeNameBuffer;
		return true;
	}

	bool GetVolumePathNamesForVolumeName(const string& VolumeName, string& VolumePathNames)
	{
		return os::detail::ApiDynamicStringReceiver(VolumePathNames, [&](wchar_t* Buffer, size_t Size)
		{
			DWORD ReturnLength = 0;
			return Imports().GetVolumePathNamesForVolumeNameW(VolumeName.data(), Buffer, static_cast<DWORD>(Size), &ReturnLength) || !ReturnLength?
				ReturnLength :
				ReturnLength + 1;
		});
	}

	bool QueryDosDevice(const string& DeviceName, string &Path)
	{
		const auto DeviceNamePtr = EmptyToNull(DeviceName.data());
		return os::detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, Path, [&](wchar_t* Buffer, size_t Size)
		{
			const auto ReturnedSize = ::QueryDosDevice(DeviceNamePtr, Buffer, static_cast<DWORD>(Size));
			// Upon success it includes two trailing '\0', we don't need them
			return ReturnedSize? ReturnedSize - 2 : 0;
		});
	}

	bool SearchPath(const wchar_t* Path, const string& FileName, const wchar_t* Extension, string& strDest)
	{
		return os::detail::ApiDynamicStringReceiver(strDest, [&](wchar_t* Buffer, size_t Size)
		{
			return ::SearchPath(Path, FileName.data(), Extension, static_cast<DWORD>(Size), Buffer, nullptr);
		});
	}

	bool GetTempPath(string& strBuffer)
	{
		return os::detail::ApiDynamicStringReceiver(strBuffer, [&](wchar_t* Buffer, size_t Size)
		{
			return ::GetTempPath(static_cast<DWORD>(Size), Buffer);
		});
	}

	bool GetModuleFileName(HANDLE hProcess, HMODULE hModule, string &strFileName)
	{
		return os::detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, strFileName, [&](wchar_t* Buffer, size_t Size)
		{
			if (!hProcess)
			{
				auto ReturnedSize = ::GetModuleFileName(hModule, Buffer, static_cast<DWORD>(Size));
				if (ReturnedSize == Size)
				{
					// os <= XP doesn't set this
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
					ReturnedSize = 0;
				}
				return ReturnedSize;
			}

			if (Imports().QueryFullProcessImageNameW && !hModule)
			{
				auto sz = static_cast<DWORD>(Size);
				return Imports().QueryFullProcessImageNameW(hProcess, 0, Buffer, &sz)? sz : 0;
			}
			else
			{
				return ::GetModuleFileNameEx(hProcess, hModule, Buffer, static_cast<DWORD>(Size));
			}
		});
	}

	security_descriptor get_file_security(const string_view& Object, SECURITY_INFORMATION RequestedInformation)
	{
		security_descriptor Result(default_buffer_size);
		NTPath NtObject(Object);
		os::detail::ApiDynamicReceiver(Result, [&](SECURITY_DESCRIPTOR* Buffer, size_t Size)
		{
			DWORD LengthNeeded = 0;
			if (!::GetFileSecurity(NtObject.data(), RequestedInformation, Buffer, static_cast<DWORD>(Size), &LengthNeeded))
				return static_cast<size_t>(LengthNeeded);
			return Size;
		},
		[](size_t ReturnedSize, size_t AllocatedSize)
		{
			return ReturnedSize > AllocatedSize;
		},
		[](...)
		{});

		return Result;
	}

	bool set_file_security(const string_view& Object, SECURITY_INFORMATION RequestedInformation, const security_descriptor& SecurityDescriptor)
	{
		return ::SetFileSecurity(NTPath(Object).data(), RequestedInformation, SecurityDescriptor.get()) != FALSE;
	}

	bool get_disk_size(const string_view& Path, unsigned long long* TotalSize, unsigned long long* TotalFree, unsigned long long* UserFree)
	{
		NTPath strPath(Path);
		AddEndSlash(strPath);

		if (low::get_disk_free_space(strPath.data(), UserFree, TotalSize, TotalFree))
			return true;

		if (ElevationRequired(ELEVATION_READ_REQUEST))
			return elevation::instance().get_disk_free_space(strPath.data(), UserFree, TotalSize, TotalFree);

		return false;
	}

	bool GetFileTimeSimple(const string_view& FileName, chrono::time_point* CreationTime, chrono::time_point* LastAccessTime, chrono::time_point* LastWriteTime, chrono::time_point* ChangeTime)
	{
		file File;
		return File.Open(FileName, FILE_READ_ATTRIBUTES, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING) && File.GetTime(CreationTime, LastAccessTime, LastWriteTime, ChangeTime);
	}

	bool get_find_data(const string& FileName, find_data& FindData, bool ScanSymLink)
	{
		const auto Find = enum_files(FileName, ScanSymLink);
		auto ItemIterator = Find.begin();
		if (ItemIterator != Find.end())
		{
			FindData = std::move(*ItemIterator);
			return true;
		}

		FindData = {};
		FindData.dwFileAttributes = INVALID_FILE_ATTRIBUTES;

		size_t DirOffset = 0;
		ParsePath(FileName, &DirOffset);
		if (FileName.find_first_of(L"*?", DirOffset) != string::npos)
			return false;

		if ((FindData.dwFileAttributes = get_file_attributes(FileName)) == INVALID_FILE_ATTRIBUTES)
			return false;

		// Ага, значит файл таки есть. Заполним структуру ручками.
		if (const auto File = file(FileName, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING))
		{
			File.GetTime(&FindData.CreationTime, &FindData.LastAccessTime, &FindData.LastWriteTime, &FindData.ChangeTime);
			File.GetSize(FindData.nFileSize);
		}

		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
		{
			string strTmp;
			GetReparsePointInfo(FileName, strTmp, &FindData.dwReserved0); //MSDN
		}

		FindData.strFileName = make_string(PointToName(FileName));
		FindData.strAlternateFileName = ConvertNameToShort(FileName);
		return true;
	}

	find_notification_handle FindFirstChangeNotification(const string& PathName, bool WatchSubtree, DWORD NotifyFilter)
	{
		return find_notification_handle(::FindFirstChangeNotification(NTPath(PathName).data(), WatchSubtree, NotifyFilter));
	}

	int GetFileTypeByName(const string& Name)
	{
		if (const auto File = create_file(Name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING))
		{
			return ::GetFileType(File.native_handle());
		}
		return FILE_TYPE_UNKNOWN;
	}

	bool IsDiskInDrive(const string& Root)
	{
		string strVolName;
		DWORD MaxComSize;
		DWORD Flags;
		string strFS;
		auto strDrive = Root;
		AddEndSlash(strDrive);
		return os::fs::GetVolumeInformation(strDrive, &strVolName, nullptr, &MaxComSize, &Flags, &strFS);
	}

	bool create_hard_link(const string& FileName, const string& ExistingFileName, SECURITY_ATTRIBUTES* SecurityAttributes)
	{
		const auto& Create = [](const string& Object, const string& Target, SECURITY_ATTRIBUTES* SecurityAttributes)
		{
			if (low::create_hard_link(Object.data(), Target.data(), SecurityAttributes))
				return true;

			if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
				return elevation::instance().create_hard_link(Object, Target, SecurityAttributes);

			return false;
		};

		if (Create(NTPath(FileName), NTPath(ExistingFileName), SecurityAttributes))
			return true;

		// win2k bug: \\?\ fails
		if (!IsWindowsXPOrGreater())
			return Create(FileName, ExistingFileName, SecurityAttributes);

		return false;
	}

	bool CreateSymbolicLink(const string& SymlinkFileName, const string& TargetFileName, DWORD dwFlags)
	{
		NTPath NtSymlinkFileName(SymlinkFileName);

		if (CreateSymbolicLinkInternal(NtSymlinkFileName, TargetFileName, dwFlags))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
			return elevation::instance().fCreateSymbolicLink(NtSymlinkFileName, TargetFileName, dwFlags);

		return false;
	}

	bool set_file_encryption(const string_view& FileName, bool Encrypt)
	{
		NTPath NtName(FileName);

		if (low::set_file_encryption(NtName.data(), Encrypt))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST, false)) // Encryption implemented in adv32, NtStatus is not affected
			return elevation::instance().set_file_encryption(NtName, Encrypt);

		return false;
	}

	bool detach_virtual_disk(const string_view& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType)
	{
		NTPath NtObject(Object);

		if (low::detach_virtual_disk(NtObject.data(), VirtualStorageType))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
			return elevation::instance().detach_virtual_disk(NtObject, VirtualStorageType);

		return false;
	}

	bool is_directory_symbolic_link(const find_data& Data)
	{
		const auto Attributes = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT;
		return (Data.dwFileAttributes & Attributes) == Attributes && (Data.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT || Data.dwReserved0 == IO_REPARSE_TAG_SYMLINK);
	}

	bool CreateSymbolicLinkInternal(const string& Object, const string& Target, DWORD dwFlags)
	{
		return Imports().CreateSymbolicLinkW?
			(Imports().CreateSymbolicLink(Object.data(), Target.data(), dwFlags) != FALSE) :
			CreateReparsePoint(Target, Object, dwFlags&SYMBOLIC_LINK_FLAG_DIRECTORY?RP_SYMLINKDIR:RP_SYMLINKFILE);
	}
}
