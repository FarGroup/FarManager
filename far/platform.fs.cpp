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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "platform.fs.hpp"

// Internal:
#include "cvtname.hpp"
#include "drivemix.hpp"
#include "elevation.hpp"
#include "flink.hpp"
#include "imports.hpp"
#include "pathmix.hpp"
#include "string_utils.hpp"
#include "strmix.hpp"
#include "exception.hpp"
#include "exception_handler.hpp"
#include "mix.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"
#include "platform.reg.hpp"
#include "platform.version.hpp"

// Common:
#include "common.hpp"
#include "common/algorithm.hpp"
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

namespace
{
	class i_find_file_handle_impl
	{
	public:
		virtual ~i_find_file_handle_impl() = 0;
	};

	i_find_file_handle_impl::~i_find_file_handle_impl() = default;

	class far_find_file_handle_impl final: public i_find_file_handle_impl
	{
	public:
		os::fs::file Object;
		block_ptr<BYTE> BufferBase;
		block_ptr<BYTE> Buffer2;
		ULONG NextOffset{};
		bool Extended{};
		bool ReadDone{};
	};

	class os_find_file_handle_impl final: public i_find_file_handle_impl, public os::fs::find_handle
	{
		using os::fs::find_handle::find_handle;
	};

	DWORD SHErrorToWinError(DWORD const SHError)
	{
		switch (SHError)
		{
		// https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shfileoperationw
		// To examine the nonzero values for troubleshooting purposes, they largely map to those defined in Winerror.h.
		// However, several of its possible return values are based on pre-Win32 error codes, which in some cases
		// overlap the later Winerror.h values without matching their meaning. Those particular values are detailed here,
		// and for these specific values only these meanings should be accepted over the Winerror.h codes.

		case 0x71:    return ERROR_ALREADY_EXISTS;    // DE_SAMEFILE            The source and destination files are the same file.
		case 0x72:    return ERROR_INVALID_PARAMETER; // DE_MANYSRC1DEST        Multiple file paths were specified in the source buffer, but only one destination file path.
		case 0x73:    return ERROR_NOT_SAME_DEVICE;   // DE_DIFFDIR             Rename operation was specified but the destination path is a different directory. Use the move operation instead.
		case 0x74:    return ERROR_INVALID_PARAMETER; // DE_ROOTDIR             The source is a root directory, which cannot be moved or renamed.
		case 0x75:    return ERROR_CANCELLED;         // DE_OPCANCELLED         The operation was canceled by the user, or silently canceled if the appropriate flags were supplied to SHFileOperation.
		case 0x76:    return ERROR_BAD_PATHNAME;      // DE_DESTSUBTREE         The destination is a subtree of the source.
		case 0x78:    return ERROR_ACCESS_DENIED;     // DE_ACCESSDENIEDSRC     Security settings denied access to the source.
		case 0x79:    return ERROR_BUFFER_OVERFLOW;   // DE_PATHTOODEEP         The source or destination path exceeded or would exceed MAX_PATH.
		case 0x7A:    return ERROR_INVALID_PARAMETER; // DE_MANYDEST            The operation involved multiple destination paths, which can fail in the case of a move operation.
		case 0x7C:    return ERROR_BAD_PATHNAME;      // DE_INVALIDFILES        The path in the source or destination or both was invalid.
		case 0x7D:    return ERROR_INVALID_PARAMETER; // DE_DESTSAMETREE        The source and destination have the same parent folder.
		case 0x7E:    return ERROR_ALREADY_EXISTS;    // DE_FLDDESTISFILE       The destination path is an existing file.
		case 0x80:    return ERROR_ALREADY_EXISTS;    // DE_FILEDESTISFLD       The destination path is an existing folder.
		case 0x81:    return ERROR_BUFFER_OVERFLOW;   // DE_FILENAMETOOLONG     The name of the file exceeds MAX_PATH.
		case 0x82:    return ERROR_WRITE_FAULT;       // DE_DEST_IS_CDROM       The destination is a read-only CD-ROM, possibly unformatted.
		case 0x83:    return ERROR_WRITE_FAULT;       // DE_DEST_IS_DVD         The destination is a read-only DVD, possibly unformatted.
		case 0x84:    return ERROR_WRITE_FAULT;       // DE_DEST_IS_CDRECORD    The destination is a writable CD-ROM, possibly unformatted.
		case 0x85:    return ERROR_DISK_FULL;         // DE_FILE_TOO_LARGE      The file involved in the operation is too large for the destination media or file system.
		case 0x86:    return ERROR_READ_FAULT;        // DE_SRC_IS_CDROM        The source is a read-only CD-ROM, possibly unformatted.
		case 0x87:    return ERROR_READ_FAULT;        // DE_SRC_IS_DVD          The source is a read-only DVD, possibly unformatted.
		case 0x88:    return ERROR_READ_FAULT;        // DE_SRC_IS_CDRECORD     The source is a writable CD-ROM, possibly unformatted.
		case 0xB7:    return ERROR_BUFFER_OVERFLOW;   // DE_ERROR_MAX           MAX_PATH was exceeded during the operation.
		case 0x402:   return ERROR_PATH_NOT_FOUND;    //                        An unknown error occurred. This is typically due to an invalid path in the source or destination. This error does not occur on Windows Vista and later.
		case 0x10000: return ERROR_GEN_FAILURE;       // ERRORONDEST            An unspecified error occurred on the destination.
		case 0x10074: return ERROR_INVALID_PARAMETER; // DE_ROOTDIR|ERRORONDEST Destination is a root directory and cannot be renamed.
		default:      return SHError;
		}
	}
}

namespace os::fs
{
	namespace detail
	{
		void find_handle_closer::operator()(HANDLE Handle) const noexcept
		{
			if (!FindClose(Handle))
				LOGWARNING(L"FindClose(): {}"sv, last_error());
		}

		void find_file_handle_closer::operator()(HANDLE Handle) const noexcept
		{
			delete static_cast<i_find_file_handle_impl*>(Handle);
		}

		void find_volume_handle_closer::operator()(HANDLE Handle) const noexcept
		{
			if (!FindVolumeClose(Handle))
				LOGWARNING(L"FindVolumeClose(): {}"sv, last_error());
		}

		void find_nt_handle_closer::operator()(HANDLE const Handle) const noexcept
		{
			if (const auto Status = imports.NtClose(Handle); !NT_SUCCESS(Status))
				LOGWARNING(L"NtClose(): {}"sv, Status);
		}
	}

	namespace state
	{
		static std::atomic_bool current_directory_syncronisation = false;

		void set_current_directory_syncronisation(bool const Value)
		{
			current_directory_syncronisation = Value;
		}
	}

	const string& find_data::AlternateFileName() const
	{
		return HasAlternateFileName()? AlternateFileNameData : FileName;
	}

	void find_data::SetAlternateFileName(string_view Name)
	{
		AlternateFileNameData = Name;
	}

	bool find_data::HasAlternateFileName() const
	{
		return !AlternateFileNameData.empty();
	}

	namespace drive
	{
		bool is_standard_letter(wchar_t Letter)
		{
			return in_closed_range(L'A', upper(Letter), L'Z');
		}

		size_t get_number(wchar_t Letter)
		{
			assert(is_standard_letter(Letter));

			return upper(Letter) - L'A';
		}

		wchar_t get_letter(size_t Number)
		{
			assert(Number < 26);

			return static_cast<wchar_t>(L'A' + Number);
		}

		string get_device_path(wchar_t Letter)
		{
			return { Letter, L':' };
		}

		string get_device_path(size_t const Number)
		{
			return { get_letter(Number), L':' };
		}

		string get_win32nt_device_path(wchar_t Letter)
		{
			return { L'\\', L'\\', L'?', L'\\', Letter, L':' };
		}

		string get_root_directory(wchar_t Letter)
		{
			return { Letter, L':', L'\\' };
		}

		string get_win32nt_root_directory(wchar_t Letter)
		{
			return { L'\\', L'\\', L'?', L'\\', Letter, L':', L'\\' };
		}

		unsigned get_type(string_view const Path)
		{
			bool IsRoot = false;
			if (const auto PathType = ParsePath(Path, {}, &IsRoot); IsRoot && (PathType == root_type::drive_letter || PathType == root_type::win32nt_drive_letter))
			{
				// It seems that Windows caches this information for drive letters, but not for other paths.
				// We want to utilise it, if possible, to avoid delays with network drives.
				return GetDriveType(get_root_directory(PathType == root_type::drive_letter? Path[0] : Path[4]).c_str());
			}

			auto NtPath = nt_path(Path.empty()? os::fs::get_current_directory() : Path);
			AddEndSlash(NtPath);

			return GetDriveType(NtPath.c_str());
		}
	}

	//-------------------------------------------------------------------------

	enum_drives::enum_drives(drives_set Drives):
		m_Drives(Drives)
	{
	}

	bool enum_drives::get(bool Reset, wchar_t& Value) const
	{
		if (Reset)
			m_CurrentIndex = 0;

		while (m_CurrentIndex != m_Drives.size() && !m_Drives[m_CurrentIndex])
			++m_CurrentIndex;

		if (m_CurrentIndex == m_Drives.size())
			return false;

		Value = drive::get_letter(m_CurrentIndex);
		++m_CurrentIndex;
		return true;
	}

	//-------------------------------------------------------------------------

	static string enum_files_prepare(const string_view Object)
	{
		string PreparedObject = nt_path(Object);
		auto Root = false;
		ParsePath(PreparedObject, nullptr, &Root);
		if (Root)
		{
			AddEndSlash(PreparedObject);
		}
		else
		{
			DeleteEndSlash(PreparedObject);
		}
		return PreparedObject;
	}

	enum_files::enum_files(const string_view Object, const bool ScanSymlink):
		m_Object(enum_files_prepare(Object)),
		m_ScanSymlink(ScanSymlink)
	{
	}

	// MSDN verse 2.4.17:
	// "When working with this field, use FileNameLength to determine the length of the file name
	// rather than assuming the presence of a trailing null delimiter."

	// Some buggy implementations (e. g. ms sharepoint, rdesktop) set the length incorrectly
	// (e. g. including the terminating \0 or as ((number of bytes in the source string) * 2) when source is in UTF-8),
	// so instead of, say, "name" (4) they return "name\0\0\0\0" (8).
	// Generally speaking, it's their own problems and we shall use it as is, as per the verse above.
	// However, most of applications use FindFirstFile API, which copies this string
	// to a fixed-size buffer, WIN32_FIND_DATA.cFileName, leaving the burden of finding its length
	// to the application itself, which, by coincidence, does it correctly, effectively masking the initial error.
	// So people come to us and claim that Far isn't working properly while other programs are fine.
	static auto trim_trailing_zeros(string_view const Str)
	{
		const auto Pos = Str.find_last_not_of(L'\0');
		return Pos != string::npos? Str.substr(0, Pos + 1) : string_view{};
	}

	// Some other buggy implementations just set the first char of AlternateFileName to '\0' to make it "empty", leaving rubbish in others. Double facepalm.
	static auto empty_if_zero(string_view const Str)
	{
		return Str.starts_with(L'\0')? string_view{} : Str;
	}

	static void DirectoryInfoToFindData(const FILE_ID_BOTH_DIR_INFORMATION& DirectoryInfo, find_data& FindData, bool IsExtended)
	{
		FindData.Attributes = DirectoryInfo.FileAttributes;
		FindData.CreationTime = os::chrono::nt_clock::from_hectonanoseconds(DirectoryInfo.CreationTime.QuadPart);
		FindData.LastAccessTime = os::chrono::nt_clock::from_hectonanoseconds(DirectoryInfo.LastAccessTime.QuadPart);
		FindData.LastWriteTime = os::chrono::nt_clock::from_hectonanoseconds(DirectoryInfo.LastWriteTime.QuadPart);
		FindData.ChangeTime = os::chrono::nt_clock::from_hectonanoseconds(DirectoryInfo.ChangeTime.QuadPart);
		FindData.FileSize = DirectoryInfo.EndOfFile.QuadPart;
		FindData.AllocationSize = DirectoryInfo.AllocationSize.QuadPart;
		FindData.ReparseTag = FindData.Attributes&FILE_ATTRIBUTE_REPARSE_POINT? DirectoryInfo.EaSize : 0;

		const auto CopyNames = [&FindData](const auto& DirInfo)
		{
			FindData.FileName = trim_trailing_zeros({ DirInfo.FileName, DirInfo.FileNameLength / sizeof(wchar_t) });
			FindData.SetAlternateFileName(trim_trailing_zeros(empty_if_zero({ DirInfo.ShortName, DirInfo.ShortNameLength / sizeof(wchar_t) })));
		};

		if (IsExtended)
		{
			FindData.FileId = DirectoryInfo.FileId.QuadPart;
			CopyNames(DirectoryInfo);
		}
		else
		{
			FindData.FileId = 0;
			CopyNames(view_as<FILE_BOTH_DIR_INFORMATION>(&DirectoryInfo));
		}
	}

	// gh-425 Incorrect file sizes shown/calculated for files compressed with LZX
	// WOF-based compression doesn't set FILE_ATTRIBUTE_COMPRESSED and doesn't fill AllocationSize
	static void fill_allocation_size_alternative(find_data& FindData, string_view Directory)
	{
		if (FindData.AllocationSize)
			return;

		if (!FindData.FileSize)
			return;

		if (flags::check_any(FindData.Attributes, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
			return;

		static const auto IsWeirdCompressionAvailable = IsWindows10OrGreater();
		if (!IsWeirdCompressionAvailable && !flags::check_any(FindData.Attributes, FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE))
			return;

		// TODO: It's a separate call so we might need an elevation for it
		ULARGE_INTEGER Size;
		Size.LowPart = GetCompressedFileSize(nt_path(path::join(Directory, FindData.FileName)).c_str(), &Size.HighPart);
		if (Size.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
			return;

		FindData.AllocationSize = Size.QuadPart;
	}

	static find_file_handle FindFirstFileInternal(string_view const Name, find_data& FindData)
	{
		if (Name.empty() || path::get_is_separator(Name)(Name.back()))
			return nullptr;

		auto Handle = std::make_unique<far_find_file_handle_impl>();

		string_view NamePart;
		auto Directory = Name;
		if (CutToSlash(Directory))
		{
			NamePart = Name.substr(Directory.size());
		}

		const auto OpenDirectory = [&]
		{
			return Handle->Object.Open(Directory, FILE_LIST_DIRECTORY, file_share_all, nullptr, OPEN_EXISTING);
		};

		if (!OpenDirectory())
		{
			// fix error code if we are looking for FILE(S) in a non-existent directory, not for a directory itself
			if (GetLastError() == ERROR_FILE_NOT_FOUND && !NamePart.empty())
			{
				SetLastError(ERROR_PATH_NOT_FOUND);
			}
			return nullptr;
		}

		// for network paths buffer size must be <= 64k
		Handle->BufferBase.reset(65536);
		Handle->Extended = true;

		bool QueryResult = Handle->Object.NtQueryDirectoryFile(Handle->BufferBase.data(), Handle->BufferBase.size(), FileIdBothDirectoryInformation, false, NamePart, true);
		if (QueryResult) // try next read immediately to avoid M#2128 bug
		{
			block_ptr<BYTE> Buffer2(Handle->BufferBase.size());
			if (Handle->Object.NtQueryDirectoryFile(Buffer2.data(), Buffer2.size(), FileIdBothDirectoryInformation, false, NamePart, false))
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
				QueryResult = Handle->Object.NtQueryDirectoryFile(Handle->BufferBase.data(), Handle->BufferBase.size(), FileBothDirectoryInformation, false, NamePart, true);
			}
		}

		if (!QueryResult)
			return nullptr;

		const auto& DirectoryInfo = view_as<FILE_ID_BOTH_DIR_INFORMATION>(Handle->BufferBase.data());
		DirectoryInfoToFindData(DirectoryInfo, FindData, Handle->Extended);
		fill_allocation_size_alternative(FindData, Directory);
		Handle->NextOffset = DirectoryInfo.NextEntryOffset;
		return find_file_handle(Handle.release());
	}

	static bool FindNextFileInternal(const find_file_handle& Find, find_data& FindData)
	{
		bool Result = false;
		auto& Handle = *static_cast<far_find_file_handle_impl*>(Find.native_handle());
		bool Status = true, set_errcode = true;
		auto DirectoryInfo = std::bit_cast<const FILE_ID_BOTH_DIR_INFORMATION*>(Handle.BufferBase.data());
		if (Handle.NextOffset)
		{
			DirectoryInfo = view_as<const FILE_ID_BOTH_DIR_INFORMATION*>(DirectoryInfo, Handle.NextOffset);
		}
		else
		{
			if (Handle.ReadDone)
			{
				Status = false;
			}
			else
			{
				if (Handle.Buffer2)
				{
					Handle.BufferBase = std::move(Handle.Buffer2);
					DirectoryInfo = view_as<const FILE_ID_BOTH_DIR_INFORMATION*>(Handle.BufferBase.data());
				}
				else
				{
					Status = Handle.Object.NtQueryDirectoryFile(Handle.BufferBase.data(), Handle.BufferBase.size(), Handle.Extended? FileIdBothDirectoryInformation : FileBothDirectoryInformation, false, {}, false);
					set_errcode = false;
				}
			}
		}

		if (Status)
		{
			DirectoryInfoToFindData(*DirectoryInfo, FindData, Handle.Extended);
			fill_allocation_size_alternative(FindData, Handle.Object.GetName());
			Handle.NextOffset = DirectoryInfo->NextEntryOffset? Handle.NextOffset + DirectoryInfo->NextEntryOffset : 0;
			Result = true;
		}

		if (set_errcode)
			SetLastError(Result? ERROR_SUCCESS : ERROR_NO_MORE_FILES);

		return Result;
	}

	bool enum_files::get(bool Reset, find_data& Value) const
	{
		if (Reset)
		{
			// temporarily disable elevation to try the requested name first
			{
				SCOPED_ACTION(auto)(elevation::suppress(true));
				m_Handle = FindFirstFileInternal(m_Object, Value);
			}

			if (!m_Handle && GetLastError() == ERROR_ACCESS_DENIED)
			{
				if (m_ScanSymlink)
				{
					string_view Str = m_Object;
					// only links in the path should be processed, not the object name itself
					CutToSlash(Str);
					m_Handle = FindFirstFileInternal(nt_path(path::join(ConvertNameToReal(Str), PointToName(m_Object))), Value);
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
		if (Value.Attributes & FILE_ATTRIBUTE_DIRECTORY &&
			Value.FileName[0] == L'.' && ((Value.FileName.size() == 2 && Value.FileName[1] == L'.') || Value.FileName.size() == 1) &&
			// These "virtual" folders either don't have an SFN at all or it's the same as LFN:
			(!Value.HasAlternateFileName() || Value.AlternateFileName() == Value.FileName))
		{
			return get(false, Value);
		}

		return true;
	}

	//-------------------------------------------------------------------------

	static find_handle FindFirstFileName(const string_view FileName, const DWORD Flags, string& LinkName)
	{
		if (!imports.FindFirstFileNameW)
			return {};

		const auto NtFileName = nt_path(FileName);
		find_handle Handle;
		// BUGBUG check result
		(void)os::detail::ApiDynamicStringReceiver(LinkName, [&](std::span<wchar_t> Buffer)
		{
			auto BufferSize = static_cast<DWORD>(Buffer.size());
			Handle.reset(imports.FindFirstFileNameW(NtFileName.c_str(), Flags, &BufferSize, Buffer.data()));
			if (Handle)
				// FindFirstFileNameW always includes terminating \0 in the returned size
				return BufferSize - 1;
			return GetLastError() == ERROR_MORE_DATA? BufferSize : 0;
		});
		return Handle;
	}

	static bool FindNextFileName(const find_handle& hFindStream, string& LinkName)
	{
		if (!imports.FindNextFileNameW)
			return false;

		return os::detail::ApiDynamicStringReceiver(LinkName, [&](std::span<wchar_t> Buffer)
		{
			auto BufferSize = static_cast<DWORD>(Buffer.size());
			if (imports.FindNextFileNameW(hFindStream.native_handle(), &BufferSize, Buffer.data()))
				// FindNextFileNameW always includes terminating \0 in the returned size
				return BufferSize - 1;
			return GetLastError() == ERROR_MORE_DATA? BufferSize : 0;
		});
	}

	enum_names::enum_names(const string_view Object):
		m_Object(Object)
	{
	}

	bool enum_names::get(bool Reset, string& Value) const
	{
		if (Reset)
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

		*std::copy_n(std::cbegin(StreamInfo.StreamName), Length, StreamData.cStreamName) = {};
		StreamData.StreamSize = StreamInfo.StreamSize;
		return true;
	}

	static find_file_handle FindFirstStream(const string_view FileName, const STREAM_INFO_LEVELS InfoLevel, void* const FindStreamData, const DWORD Flags)
	{
		if (imports.FindFirstStreamW && imports.FindNextStreamW)
		{
			os_find_file_handle_impl Handle(imports.FindFirstStreamW(nt_path(FileName).c_str(), InfoLevel, FindStreamData, Flags));
			return find_file_handle(Handle? std::make_unique<os_find_file_handle_impl>(Handle.release()).release() : nullptr);
		}

		if (InfoLevel != FindStreamInfoStandard)
			return nullptr;

		auto Handle = std::make_unique<far_find_file_handle_impl>();

		if (!Handle->Object.Open(FileName, 0, file_share_all, nullptr, OPEN_EXISTING))
			return nullptr;

		// for network paths buffer size must be <= 64k
		// we double it in a first loop, so starting value is 32k
		size_t BufferSize = 32768;
		auto Result = STATUS_UNSUCCESSFUL;
		do
		{
			BufferSize *= 2;
			Handle->BufferBase.reset(BufferSize);
			// sometimes for directories NtQueryInformationFile returns STATUS_SUCCESS but doesn't fill the buffer
			auto& StreamInfo = edit_as<FILE_STREAM_INFORMATION>(Handle->BufferBase.data());
			StreamInfo.StreamNameLength = 0;
			// BUGBUG check result
			(void)Handle->Object.NtQueryInformationFile(Handle->BufferBase.data(), Handle->BufferBase.size(), FileStreamInformation, &Result);
		}
		while (any_of(Result, STATUS_INFO_LENGTH_MISMATCH, STATUS_BUFFER_OVERFLOW, STATUS_BUFFER_TOO_SMALL));

		if (!NT_SUCCESS(Result))
			return nullptr;

		const auto& StreamInfo = view_as<FILE_STREAM_INFORMATION>(Handle->BufferBase.data());
		Handle->NextOffset = StreamInfo.NextEntryOffset;
		const auto StreamData = static_cast<WIN32_FIND_STREAM_DATA*>(FindStreamData);

		if (!FileStreamInformationToFindStreamData(StreamInfo, *StreamData))
			return nullptr;

		return find_file_handle(Handle.release());
	}

	static bool FindNextStream(const find_file_handle& hFindStream, void* FindStreamData)
	{
		if (imports.FindFirstStreamW && imports.FindNextStreamW)
		{
			return imports.FindNextStreamW(static_cast<os_find_file_handle_impl*>(hFindStream.native_handle())->native_handle(), FindStreamData) != FALSE;
		}

		const auto Handle = static_cast<far_find_file_handle_impl*>(hFindStream.native_handle());

		if (!Handle->NextOffset)
			return false;

		const auto& StreamInfo = view_as<FILE_STREAM_INFORMATION>(Handle->BufferBase.data() + Handle->NextOffset);
		Handle->NextOffset = StreamInfo.NextEntryOffset? Handle->NextOffset + StreamInfo.NextEntryOffset : 0;
		const auto StreamData = static_cast<WIN32_FIND_STREAM_DATA*>(FindStreamData);

		return FileStreamInformationToFindStreamData(StreamInfo, *StreamData);
	}

	enum_streams::enum_streams(const string_view Object):
		m_Object(Object)
	{
	}

	bool enum_streams::get(bool Reset, WIN32_FIND_STREAM_DATA& Value) const
	{
		if (Reset)
		{
			m_Handle = FindFirstStream(m_Object, FindStreamInfoStandard, &Value, 0);
			return m_Handle != nullptr;
		}

		return FindNextStream(m_Handle, &Value);
	}

	//-------------------------------------------------------------------------

	enum_volumes::enum_volumes() = default;

	bool enum_volumes::get(bool Reset, string& Value) const
	{
		// A reasonable size for the buffer to accommodate the largest possible volume UUID path is 50 characters.
		wchar_t VolumeName[50];

		if (Reset)
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

	enum_devices::enum_devices(string_view const Object)
	{
		m_Object.Buffer = const_cast<wchar_t*>(Object.data());
		m_Object.Length = m_Object.MaximumLength = static_cast<USHORT>(Object.size() * sizeof(wchar_t));

		OBJECT_ATTRIBUTES Attributes;
		InitializeObjectAttributes(&Attributes, &m_Object, 0, nullptr, nullptr)

		if (!NT_SUCCESS(imports.NtOpenDirectoryObject(&ptr_setter(m_Handle), GENERIC_READ, &Attributes)))
			return;

		m_Buffer.reset(32 * 1024);
	}

	bool enum_devices::get(bool Reset, string_view& Value) const
	{
		if (!m_Handle)
			return false;

		if (Reset)
			m_Index.reset();

		auto RestartScan = Reset;

		const auto fill = [&]
		{
			ULONG Size;
			if (!NT_SUCCESS(imports.NtQueryDirectoryObject(m_Handle.native_handle(), m_Buffer.data(), static_cast<ULONG>(m_Buffer.size()), false, RestartScan, &m_Context, &Size)))
				return false;

			RestartScan = false;
			m_Index = 0;
			return true;
		};

		if (!m_Index)
		{
			if (!fill())
				return false;
		}

		const auto Entries = std::bit_cast<OBJECT_DIRECTORY_INFORMATION const*>(m_Buffer.data());

		if (!Entries[*m_Index].Name.Length)
		{
			m_Index.reset();
			if (!fill())
				return false;
		}

		Value = { Entries[*m_Index].Name.Buffer, Entries[*m_Index].Name.Length / sizeof(wchar_t) };
		++*m_Index;

		return true;
	}

	//-------------------------------------------------------------------------

	file::file():
		m_Pointer(),
		m_NeedSyncPointer(),
		m_ShareMode()
	{
	}

	file::file(handle&& Handle):
		m_Handle(std::move(Handle)),
		m_Pointer(),
		m_NeedSyncPointer(),
		m_ShareMode()
	{
		LARGE_INTEGER NewPointer;
		SetFilePointerEx(m_Handle.native_handle(), {}, &NewPointer, FILE_CURRENT);
		m_Pointer = NewPointer.QuadPart;
	}

	file::operator bool() const noexcept
	{
		return m_Handle.operator bool();
	}

	bool file::Open(const string_view Object, const DWORD DesiredAccess, const DWORD ShareMode, SECURITY_ATTRIBUTES* SecurityAttributes, const DWORD CreationDistribution, const DWORD FlagsAndAttributes, const file* TemplateFile, const bool ForceElevation)
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

		m_Name = Object;
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
				// BUGBUG check result
				(void)GetSize(Size);
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
		if (SetEndOfFile(m_Handle.native_handle()))
			return true;

		if (!m_Name.empty() && GetLastError() == ERROR_INVALID_PARAMETER) // OSX buggy SMB workaround
		{
			const auto fsize = GetPointer();
			Close();
			if (Open(m_Name, GENERIC_WRITE, m_ShareMode, nullptr, OPEN_EXISTING, 0))
			{
				SetPointer(fsize, nullptr, FILE_BEGIN);
				SyncPointer();
				return SetEndOfFile(m_Handle.native_handle()) != FALSE;
			}
		}

		return false;
	}

	bool file::GetTime(os::chrono::time_point* CreationTime, os::chrono::time_point* LastAccessTime, os::chrono::time_point* LastWriteTime, os::chrono::time_point* ChangeTime) const
	{
		const auto convert_time = [](LARGE_INTEGER const& From, os::chrono::time_point* const To)
		{
			if (To)
				*To = os::chrono::nt_clock::from_hectonanoseconds(From.QuadPart);
		};

		FILE_BASIC_INFORMATION fbi;

		if (!NtQueryInformationFile(&fbi, sizeof fbi, FileBasicInformation))
			return false;

		convert_time(fbi.CreationTime, CreationTime);
		convert_time(fbi.LastAccessTime, LastAccessTime);
		convert_time(fbi.LastWriteTime, LastWriteTime);
		convert_time(fbi.ChangeTime, ChangeTime);

		return true;
	}

	bool file::SetTime(const os::chrono::time_point* CreationTime, const os::chrono::time_point* LastAccessTime, const os::chrono::time_point* LastWriteTime, const os::chrono::time_point* ChangeTime) const
	{
		const auto convert_time = [](os::chrono::time_point const* const From, LARGE_INTEGER& To)
		{
			if (From)
				To.QuadPart = os::chrono::nt_clock::to_hectonanoseconds(*From);
		};

		FILE_BASIC_INFORMATION fbi{};

		convert_time(CreationTime, fbi.CreationTime);
		convert_time(LastAccessTime, fbi.LastAccessTime);
		convert_time(LastWriteTime, fbi.LastWriteTime);
		convert_time(ChangeTime, fbi.ChangeTime);

		IO_STATUS_BLOCK IoStatusBlock;
		const auto Status = imports.NtSetInformationFile(m_Handle.native_handle(), &IoStatusBlock, &fbi, sizeof fbi, FileBasicInformation);
		set_last_error_from_ntstatus(Status);

		return NT_SUCCESS(Status);
	}

	bool file::GetSize(unsigned long long& Size) const
	{
		GET_LENGTH_INFORMATION gli;

		if (!GetFileSizeEx(m_Handle.native_handle(), &gli.Length) && !IoControl(IOCTL_DISK_GET_LENGTH_INFO, nullptr, 0, &gli, sizeof(gli)))
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
		DWORD BytesReturnedFallback = 0;
		if (!BytesReturned)
			BytesReturned = &BytesReturnedFallback;
		return DeviceIoControl(m_Handle.native_handle(), IoControlCode, InBuffer, InBufferSize, OutBuffer, OutBufferSize, BytesReturned, Overlapped) != FALSE;
	}

	bool file::GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed) const
	{
		if (!imports.GetStorageDependencyInformation)
			return false;

		const auto Result = imports.GetStorageDependencyInformation(m_Handle.native_handle(), Flags, StorageDependencyInfoSize, StorageDependencyInfo, SizeUsed);
		SetLastError(Result);
		return Result == ERROR_SUCCESS;
	}

	bool file::NtQueryDirectoryFile(void* FileInformation, size_t Length, FILE_INFORMATION_CLASS FileInformationClass, bool ReturnSingleEntry, string_view const FileName, bool RestartScan, NTSTATUS* Status) const
	{
		IO_STATUS_BLOCK IoStatusBlock;
		PUNICODE_STRING pNameString = nullptr;
		UNICODE_STRING NameString;
		if (!FileName.empty())
		{
			NameString.Buffer = const_cast<wchar_t*>(FileName.data());
			NameString.Length = static_cast<USHORT>(FileName.size() * sizeof(wchar_t));
			NameString.MaximumLength = NameString.Length;
			pNameString = &NameString;
		}
		const auto di = static_cast<FILE_ID_BOTH_DIR_INFORMATION*>(FileInformation);
		di->NextEntryOffset = 0xffffffffUL;

		const auto Result = imports.NtQueryDirectoryFile(m_Handle.native_handle(), nullptr, nullptr, nullptr, &IoStatusBlock, FileInformation, static_cast<ULONG>(Length), FileInformationClass, ReturnSingleEntry, pNameString, RestartScan);
		set_last_error_from_ntstatus(Result);

		if (Status)
		{
			*Status = Result;
		}

		return NT_SUCCESS(Result) && (di->NextEntryOffset != 0xffffffffUL);
	}

	bool file::NtQueryInformationFile(void* FileInformation, size_t Length, FILE_INFORMATION_CLASS FileInformationClass, NTSTATUS* Status) const
	{
		IO_STATUS_BLOCK IoStatusBlock;
		const auto Result = imports.NtQueryInformationFile(m_Handle.native_handle(), &IoStatusBlock, FileInformation, static_cast<ULONG>(Length), FileInformationClass);
		set_last_error_from_ntstatus(Result);

		if (Status)
		{
			*Status = Result;
		}
		return NT_SUCCESS(Result);
	}

	static bool GetObjectName(HANDLE hFile, string& ObjectName)
	{
		// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/content/ntifs/nf-ntifs-obquerynamestring
		// A reasonable size for the buffer to accommodate most object names is 1024 bytes.
		const auto ReasonableSize = 1024;
		block_ptr<OBJECT_NAME_INFORMATION, ReasonableSize> oni(ReasonableSize);
		ULONG ReturnLength;

		const auto QueryObject = [&]
		{
			return imports.NtQueryObject(hFile, ObjectNameInformation, oni.data(), static_cast<unsigned long>(oni.size()), &ReturnLength);
		};

		auto Result = QueryObject();
		if (any_of(Result, STATUS_INFO_LENGTH_MISMATCH, STATUS_BUFFER_OVERFLOW, STATUS_BUFFER_TOO_SMALL))
		{
			oni.reset(ReturnLength);
			Result = QueryObject();
		}

		if (!NT_SUCCESS(Result))
			return false;

		ObjectName.assign(oni->Name.Buffer, oni->Name.Length / sizeof(wchar_t));
		return true;
	}

	static int MatchNtPathRoot(string_view const NtPath, const string_view DeviceName)
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

		InitializeObjectAttributes(&ObjAttrs, &ObjName, 0, nullptr, nullptr)

		HANDLE hSymLink;
		if (!NT_SUCCESS(imports.NtOpenSymbolicLinkObject(&hSymLink, GENERIC_READ, &ObjAttrs)))
			return 0;

		SCOPE_EXIT{ imports.NtClose(hSymLink); };

		const auto BufSize = 32767;
		const wchar_t_ptr Buffer(BufSize);
		UNICODE_STRING LinkTarget{ 0, static_cast<USHORT>(BufSize * sizeof(wchar_t)), Buffer.data() };

		if (!NT_SUCCESS(imports.NtQuerySymbolicLinkObject(hSymLink, &LinkTarget, nullptr)))
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

		const auto ReplaceRoot = [&](const auto& OldRoot, const auto& NewRoot)
		{
			if (!NtPath.starts_with(OldRoot))
				return false;

			FinalFilePath = NtPath.replace(0, OldRoot.size(), NewRoot);
			return true;
		};

		// simple way to handle network paths
		for (const auto NetworkPrefix: {
			L"\\Device\\LanmanRedirector\\"sv,
			L"\\Device\\Mup\\"sv,
			L"\\Device\\WinDfs\\Root\\"sv
		})
		{
			if (ReplaceRoot(NetworkPrefix, L"\\\\"sv))
				return true;
		}

		// Mapped drives resolve to \Device\WinDfs\X:<whatever>\server\share
		if (const auto DfsPrefix = L"\\Device\\WinDfs\\"sv; NtPath.starts_with(DfsPrefix))
		{
			const auto ServerStart = NtPath.find(L"\\", DfsPrefix.size());
			FinalFilePath = NtPath.replace(0, ServerStart, 1, L'\\');
			return true;
		}

		// try to convert NT path (\Device\HarddiskVolume1) to drive letter
		for (const auto& i: enum_drives(get_logical_drives()))
		{
			const auto Device = drive::get_device_path(i);
			if (const auto Len = MatchNtPathRoot(NtPath, Device))
			{
				FinalFilePath = NtPath.replace(0, Len, Device);
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
		const auto GetFinalPathNameByHandleGuarded = [](HANDLE File, wchar_t* Buffer, DWORD Size, DWORD Flags)
		{
			// It seems that Microsoft has forgotten to put an exception handler around this function.
			// It causes EXCEPTION_ACCESS_VIOLATION (read from 0) in kernel32 under certain conditions,
			// e.g. badly written file system drivers or weirdly formatted volumes.
			return seh_try_no_ui(
			[&]
			{
				return imports.GetFinalPathNameByHandle(File, Buffer, Size, Flags);
			},
			[](DWORD)
			{
				SetLastError(ERROR_UNHANDLED_EXCEPTION);
				return 0ul;
			});
		};

		const auto GetFinalPathNameByHandleImpl = [&]
		{
			return os::detail::ApiDynamicStringReceiver(FinalFilePath, [&](std::span<wchar_t> Buffer)
			{
				return GetFinalPathNameByHandleGuarded(m_Handle.native_handle(), Buffer.data(), static_cast<DWORD>(Buffer.size()), VOLUME_NAME_GUID);
			});
		};

		return (imports.GetFinalPathNameByHandleW && GetFinalPathNameByHandleImpl()) || internalNtQueryGetFinalPathNameByHandle(m_Handle.native_handle(), FinalFilePath);
	}

	void file::Close()
	{
		m_Handle.close();
	}

	bool file::Eof() const
	{
		const auto Ptr = GetPointer();
		unsigned long long Size = 0;
		// BUGBUG check result
		(void)GetSize(Size);
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
			FILE_ALLOCATED_RANGE_BUFFER Ranges[512];
			DWORD BytesReturned;
			const auto QueryResult = IoControl(FSCTL_QUERY_ALLOCATED_RANGES, &QueryRange, sizeof(QueryRange), Ranges, sizeof(Ranges), &BytesReturned);
			if ((!QueryResult && GetLastError() != ERROR_MORE_DATA) || !BytesReturned)
				break;

			for (const auto& i: std::span(Ranges, BytesReturned / sizeof(*Ranges)))
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
		return ToPercent(m_ProcessedSize, m_AllocSize);
	}

	//-------------------------------------------------------------------------
	filebuf::filebuf(const file& File, std::ios::openmode Mode, size_t BufferSize):
		m_File(File),
		m_Mode(Mode),
		m_Buffer(BufferSize)
	{
		if (!(m_Mode & std::ios::in) == !(m_Mode & std::ios::out))
			throw far_fatal_exception(L"Unsupported mode"sv);

		reset_get_area();
		reset_put_area();
	}

	filebuf::int_type filebuf::underflow()
	{
		if (!m_File)
			throw far_fatal_exception(L"File not opened"sv);

		if (!(m_Mode & std::ios::in))
			throw far_fatal_exception(L"Buffer not opened for reading"sv);

		size_t Read;
		if (!m_File.Read(m_Buffer.data(), m_Buffer.size() * sizeof(char), Read))
			throw far_exception(L"Read error"sv);

		if (!Read)
			return traits_type::eof();

		setg(m_Buffer.data(), m_Buffer.data(), m_Buffer.data() + Read / sizeof(char));
		return m_Buffer[0];
	}

	filebuf::int_type filebuf::overflow(int_type Ch)
	{
		if (!m_File)
			throw far_fatal_exception(L"File not opened"sv);

		if (!(m_Mode & std::ios::out))
			throw far_fatal_exception(L"Buffer not opened for writing"sv);

		if (pptr() != pbase())
		{
			if (!m_File.Write(pbase(), static_cast<size_t>(pptr() - pbase()) * sizeof(char)))
				throw far_exception(L"Write error"sv);
		}

		reset_put_area();

		if (!traits_type::eq_int_type(Ch, traits_type::eof()))
			sputc(Ch);

		return 0;
	}

	int filebuf::sync()
	{
		if (m_Mode & std::ios::in)
			reset_get_area();

		if (m_Mode & std::ios::out)
			overflow(traits_type::eof());

		return 0;
	}

	bool filebuf::adjust_pos_in_cache(std::streamoff Offset)
	{
		if (m_Mode & std::ios::in)
		{
			if (gptr() + Offset >= eback() && gptr() + Offset < egptr())
			{
				gbump(Offset);
				return true;
			}
		}
		else if (m_Mode & std::ios::out)
		{
			if (pptr() + Offset >= pbase() && pptr() + Offset < epptr())
			{
				pbump(Offset);
				return true;
			}
		}
		return false;
	}

	filebuf::pos_type filebuf::seekoff(off_type Offset, std::ios::seekdir Way, std::ios::openmode Which)
	{
		if (!m_File)
			throw far_fatal_exception(L"File not opened"sv);

		switch (Way)
		{
		case std::ios::beg:
			sync();
			return set_pointer(Offset, FILE_BEGIN);

		case std::ios::end:
			sync();
			return set_pointer(Offset, FILE_END);

		case std::ios::cur:
			{
				int CacheShift = 0;

				if (m_Mode & std::ios::in)
					CacheShift = -(egptr() - gptr());
				else if (m_Mode & std::ios::out)
					CacheShift = pptr() - pbase();

				if (adjust_pos_in_cache(Offset))
					return m_File.GetPointer() + CacheShift + Offset;
			}

			sync();
			return set_pointer(Offset, FILE_CURRENT);

		default:
			throw far_fatal_exception(L"Unknown seekdir"sv);
		}
	}

	filebuf::pos_type filebuf::seekpos(pos_type Pos, std::ios::openmode Which)
	{
		return seekoff(Pos, std::ios::beg, Which);
	}

	filebuf::int_type filebuf::pbackfail(int_type Ch)
	{
		throw far_fatal_exception(L"Not implemented"sv);
	}

	void filebuf::reset_get_area()
	{
		// buffer is not available for reading
		setg(m_Buffer.data(), m_Buffer.data() + m_Buffer.size(), m_Buffer.data() + m_Buffer.size());
	}

	void filebuf::reset_put_area()
	{
		// buffer is available for writing
		setp(m_Buffer.data(), m_Buffer.data() + m_Buffer.size());
	}

	unsigned long long filebuf::set_pointer(unsigned long long Value, int Way)
	{
		unsigned long long Result;
		if (!m_File.SetPointer(Value, &Result, Way))
			throw far_exception(L"SetFilePointer error"sv);

		return Result;
	}

	//-------------------------------------------------------------------------

	file_status::file_status():
		m_Data(INVALID_FILE_ATTRIBUTES)
	{
	}

	file_status::file_status(const string_view Object):
		m_Data(get_file_attributes(Object))
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

	bool exists(const string_view Object)
	{
		return exists(file_status(Object));
	}

	bool is_file(file_status Status)
	{
		return exists(Status) && !is_directory(Status);
	}

	bool is_file(const string_view Object)
	{
		return is_file(file_status(Object));
	}

	bool is_file(find_data const& Data)
	{
		return is_file(Data.Attributes);
	}

	bool is_file(attributes const Attributes)
	{
		return Attributes != INVALID_FILE_ATTRIBUTES && !flags::check_one(Attributes, FILE_ATTRIBUTE_DIRECTORY);
	}

	bool is_directory(file_status Status)
	{
		return Status.check(FILE_ATTRIBUTE_DIRECTORY);
	}

	bool is_directory(const string_view Object)
	{
		return is_directory(file_status(Object));
	}

	bool is_directory(find_data const& Data)
	{
		return is_directory(Data.Attributes);
	}

	bool is_directory(attributes const Attributes)
	{
		return Attributes != INVALID_FILE_ATTRIBUTES && flags::check_one(Attributes, FILE_ATTRIBUTE_DIRECTORY);
	}

	bool is_not_empty_directory(string_view const Object)
	{
		const auto Find = enum_files(path::join(Object, L'*'));
		return !Find.empty();
	}

	//-------------------------------------------------------------------------

	current_directory_guard::current_directory_guard(const string_view Directory):
		m_Directory(get_current_directory()),
		m_Active(set_current_directory(Directory))
	{
	}

	current_directory_guard::~current_directory_guard()
	{
		// No need to validate, we can trust ourselves
		if (m_Active)
			set_current_directory(m_Directory, false);
	}

	process_current_directory_guard::process_current_directory_guard(const string_view Directory):
		m_Active(GetProcessRealCurrentDirectory(m_Directory) && SetProcessRealCurrentDirectory(Directory))
	{
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

		attributes get_file_attributes(const wchar_t* const FileName)
		{
			return ::GetFileAttributes(FileName);
		}

		bool set_file_attributes(const wchar_t* const FileName, attributes const Attributes)
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

		bool replace_file(const wchar_t* ReplacedFileName, const wchar_t* ReplacementFileName, const wchar_t* BackupFileName, DWORD ReplaceFlags)
		{
			static bool IgnoreAclErrorsSupported = IsWindowsVistaOrGreater();
			if (!IgnoreAclErrorsSupported)
				ReplaceFlags &= ~REPLACEFILE_IGNORE_ACL_ERRORS;

			return ::ReplaceFile(ReplacedFileName, ReplacementFileName, EmptyToNull(BackupFileName), ReplaceFlags, nullptr, nullptr) != FALSE;
		}

		bool detach_virtual_disk(const wchar_t* Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType)
		{
			handle Handle;
			DWORD Result = imports.OpenVirtualDisk(&VirtualStorageType, Object, VIRTUAL_DISK_ACCESS_DETACH, OPEN_VIRTUAL_DISK_FLAG_NONE, nullptr, &ptr_setter(Handle));
			if (Result != ERROR_SUCCESS)
			{
				SetLastError(Result);
				return false;
			}

			Result = imports.DetachVirtualDisk(Handle.native_handle(), DETACH_VIRTUAL_DISK_FLAG_NONE, 0);
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

			const auto SetOpt = [](const ULARGE_INTEGER& From, unsigned long long* To)
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

		security::descriptor get_file_security(const wchar_t* Object, SECURITY_INFORMATION RequestedInformation)
		{
			security::descriptor Result(default_buffer_size);

			if (!os::detail::ApiDynamicReceiver(Result.bytes(),
				[&](std::span<std::byte> const Buffer)
				{
					DWORD LengthNeeded = 0;
					if (!::GetFileSecurity(Object, RequestedInformation, static_cast<SECURITY_DESCRIPTOR*>(static_cast<void*>(Buffer.data())), static_cast<DWORD>(Buffer.size()), &LengthNeeded))
						return static_cast<size_t>(LengthNeeded);
					return Buffer.size();
				},
				[](size_t ReturnedSize, size_t AllocatedSize)
				{
					return ReturnedSize > AllocatedSize;
				},
				[](std::span<std::byte const>)
				{}
			))
			{
				Result.reset();
			}

			return Result;
		}

		bool set_file_security(const wchar_t* Object, SECURITY_INFORMATION RequestedInformation, SECURITY_DESCRIPTOR* SecurityDescriptor)
		{
			return ::SetFileSecurity(Object, RequestedInformation, SecurityDescriptor) != FALSE;
		}

		bool reset_file_security(const wchar_t* Object)
		{
			ACL EmptyAcl{};
			if (!InitializeAcl(&EmptyAcl, sizeof(EmptyAcl), ACL_REVISION))
				return false;

			const auto Result = SetNamedSecurityInfo(const_cast<wchar_t*>(Object), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION, nullptr, nullptr, &EmptyAcl, nullptr);
			SetLastError(Result);
			return Result == ERROR_SUCCESS;
		}

		bool move_to_recycle_bin(string_view const Object)
		{
			const auto ObjectsArray = Object + L"\0"sv;
			SHFILEOPSTRUCT fop{};
			fop.wFunc = FO_DELETE;
			fop.pFrom = ObjectsArray.c_str();
			fop.fFlags = FOF_NO_UI | FOF_ALLOWUNDO;

			const auto Result = SHErrorToWinError(SHFileOperation(&fop));
			SetLastError(Result);
			return Result == ERROR_SUCCESS && !fop.fAnyOperationsAborted;
		}
	}

	//-------------------------------------------------------------------------

	bool GetProcessRealCurrentDirectory(string& Directory)
	{
		return os::detail::ApiDynamicStringReceiver(Directory, [](std::span<wchar_t> Buffer)
		{
			return ::GetCurrentDirectory(static_cast<DWORD>(Buffer.size()), Buffer.data());
		});
	}

	bool SetProcessRealCurrentDirectory(const string_view Directory)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setcurrentdirectory
		// The final character before the null character must be a backslash ('\').
		// If you do not specify the backslash, it will be added for you

		// It seems that "it will be added for you" doesn't work for funny names with trailing dots or spaces, so we add it ourselves.

		if (!Directory.empty() && path::get_is_separator(Directory)(Directory.back()))
			return ::SetCurrentDirectory(null_terminated(Directory).c_str()) != FALSE;

		return ::SetCurrentDirectory(AddEndSlash(Directory).c_str()) != FALSE;
	}

	void InitCurrentDirectory()
	{
		string InitCurDir;
		if (GetProcessRealCurrentDirectory(InitCurDir))
		{
			set_current_directory(InitCurDir);
		}
	}

	static string& CurrentDirectory()
	{
		static string sCurrentDirectory;
		return sCurrentDirectory;
	}

	const string& get_current_directory()
	{
		return CurrentDirectory();
	}

	bool set_current_directory(const string_view PathName, const bool Validate)
	{
		// correct path to our standard
		auto strDir = path::normalize_separators(PathName);
		bool Root = false;
		ParsePath(strDir, nullptr, &Root);
		if (Root)
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
			find_data fd;
			if (!get_find_data(path::join(PathName, L'*'), fd))
			{
				const auto LastError = GetLastError();
				if (!(LastError == ERROR_FILE_NOT_FOUND || LastError == ERROR_NO_MORE_FILES))
					return false;
			}
		}

		CurrentDirectory() = strDir;

		// try to synchronize far current directory with process current directory
		if (
			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setcurrentdirectory
			// Starting with Windows 10, version 1607, for the unicode version of this function (SetCurrentDirectoryW), you can opt-in to remove the MAX_PATH limitation
			// I'm not yet sure we should though
			(features::win10_curdir && version::is_win10_1607_or_later()) ||
#ifndef NO_WRAPPER

			// Legacy plugins expect that the current directory is set
			state::current_directory_syncronisation
#else
			false
#endif // NO_WRAPPER
		)
		{
			SetProcessRealCurrentDirectory(strDir);
		}
		return true;
	}

	bool create_directory(const string_view PathName, SECURITY_ATTRIBUTES* SecurityAttributes)
	{
		return create_directory(L""sv, PathName, SecurityAttributes);
	}

	bool create_directory(const string_view TemplateDirectory, const string_view NewDirectory, SECURITY_ATTRIBUTES* SecurityAttributes)
	{
		const auto NtNewDirectory = nt_path(NewDirectory);

		const auto Create = [&](const string& Template)
		{
			if (low::create_directory(Template.c_str(), NtNewDirectory.c_str(), SecurityAttributes))
				return true;

			if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
				return elevation::instance().create_directory(Template, NtNewDirectory, SecurityAttributes);

			return false;
		};

		const auto Existed = exists(NtNewDirectory);

		if (TemplateDirectory.empty())
			return Create({});

		if (Create(nt_path(TemplateDirectory)))
			return true;

		// CreateDirectoryEx not only can fail on some buggy FS (e.g. Samba),
		// but can also leave rubbish (see gh-368). Shame on them.
		// Try to restore the status quo and fall back to the simple method:
		if (!Existed && exists(NtNewDirectory))
			(void)remove_directory(NtNewDirectory);

		return Create({});
	}

	bool remove_directory(const string_view DirName)
	{
		const auto strNtName = nt_path(DirName);
		if (low::remove_directory(strNtName.c_str()))
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

	handle create_file(const string_view Object, const DWORD DesiredAccess, const DWORD ShareMode, SECURITY_ATTRIBUTES* SecurityAttributes, const DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile, const bool ForceElevation)
	{
		const auto strObject = nt_path(Object);
		FlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
		if (CreationDistribution == OPEN_EXISTING || CreationDistribution == TRUNCATE_EXISTING)
		{
			FlagsAndAttributes |= FILE_FLAG_POSIX_SEMANTICS;
		}

		const auto create_elevated = [&]
		{
			return handle(elevation::instance().create_file(strObject, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile));
		};

		if (ForceElevation)
			return create_elevated();

		if (auto Handle = handle(low::create_file(strObject.c_str(), DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile)))
			return Handle;

		const auto LastError = last_error();

		if (LastError.NtError == STATUS_STOPPED_ON_SYMLINK && LastError.Win32Error != ERROR_STOPPED_ON_SYMLINK)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return nullptr;
		}

		if (LastError.Win32Error == ERROR_FILE_NOT_FOUND || LastError.Win32Error == ERROR_PATH_NOT_FOUND)
		{
			FlagsAndAttributes &= ~FILE_FLAG_POSIX_SEMANTICS;
			if (auto Handle = handle(low::create_file(strObject.c_str(), DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile)))
				return Handle;
		}

		if (ElevationRequired(DesiredAccess & (GENERIC_ALL | GENERIC_WRITE | WRITE_OWNER | WRITE_DAC | DELETE | FILE_WRITE_DATA | FILE_ADD_FILE | FILE_APPEND_DATA | FILE_ADD_SUBDIRECTORY | FILE_CREATE_PIPE_INSTANCE | FILE_WRITE_EA | FILE_DELETE_CHILD | FILE_WRITE_ATTRIBUTES)? ELEVATION_MODIFY_REQUEST : ELEVATION_READ_REQUEST))
			return create_elevated();

		return nullptr;
	}

	bool delete_file(const string_view FileName)
	{
		const auto strNtName = nt_path(FileName);

		if (low::delete_file(strNtName.c_str()))
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

	static DWORD WINAPI progress_routine_wrapper(
		LARGE_INTEGER const TotalFileSize,
		LARGE_INTEGER const TotalBytesTransferred,
		LARGE_INTEGER const StreamSize,
		LARGE_INTEGER const StreamBytesTransferred,
		DWORD const StreamNumber,
		DWORD const CallbackReason,
		HANDLE const SourceFile,
		HANDLE const DestinationFile,
		void* const Data)
	{
		const auto Routine = view_as<progress_routine>(Data);
		return Routine(
			TotalFileSize.QuadPart,
			TotalBytesTransferred.QuadPart,
			StreamSize.QuadPart,
			StreamBytesTransferred.QuadPart,
			StreamNumber,
			CallbackReason,
			SourceFile,
			DestinationFile
		);
	}

	bool copy_file(const string_view ExistingFileName, const string_view NewFileName, progress_routine ProgressRoutine, BOOL* const Cancel, const DWORD CopyFlags)
	{
		const auto strFrom = nt_path(ExistingFileName);
		auto strTo = nt_path(NewFileName);

		if (path::get_is_separator(strTo)(strTo.back()))
		{
			append(strTo, PointToName(strFrom));
		}

		const auto RoutinePtr = ProgressRoutine? &progress_routine_wrapper : nullptr;
		const auto DataPtr = ProgressRoutine? &ProgressRoutine : nullptr;

		if (low::copy_file(strFrom.c_str(), strTo.c_str(), RoutinePtr, DataPtr, Cancel, CopyFlags))
			return true;

		const auto LastError = last_error();

		if (LastError.NtError == STATUS_STOPPED_ON_SYMLINK && LastError.Win32Error != ERROR_STOPPED_ON_SYMLINK)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return false;
		}

		if (LastError.NtError == STATUS_FILE_IS_A_DIRECTORY)
		{
			SetLastError(ERROR_FILE_EXISTS);
			return false;
		}

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
			return elevation::instance().copy_file(strFrom, strTo, RoutinePtr, DataPtr, Cancel, CopyFlags);

		return false;
	}

	bool move_file(const string_view ExistingFileName, const string_view NewFileName, const DWORD Flags)
	{
		const auto strFrom = nt_path(ExistingFileName);
		auto strTo = nt_path(NewFileName);

		if (path::get_is_separator(strTo)(strTo.back()))
		{
			append(strTo, PointToName(strFrom));
		}

		if (low::move_file(strFrom.c_str(), strTo.c_str(), Flags))
			return true;

		const auto LastError = last_error();

		if (LastError.NtError == STATUS_STOPPED_ON_SYMLINK && LastError.Win32Error != ERROR_STOPPED_ON_SYMLINK)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return false;
		}

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
		{
			last_error_guard gle;
			// exclude fake elevation request for: move file over existing directory with same name
			const file_status SrcStatus(strFrom), DstStatus(strTo);
			if (is_directory(DstStatus) && is_file(SrcStatus))
			{
				SetLastError(ERROR_FILE_EXISTS); // existing directory name == moved file name
				gle.dismiss();
				return false;
			}
			return elevation::instance().move_file(strFrom, strTo, Flags);
		}

		return false;
	}

	bool replace_file(string_view ReplacedFileName, string_view ReplacementFileName, string_view BackupFileName, DWORD Flags)
	{
		const auto
			To = nt_path(ReplacedFileName),
			From = nt_path(ReplacementFileName),
			Backup = nt_path(BackupFileName);

		if (low::replace_file(To.c_str(), From.c_str(), Backup.c_str(), Flags))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST)) //BUGBUG, really unknown
			return elevation::instance().replace_file(To, From, Backup, Flags);

		return false;
	}


	attributes get_file_attributes(const string_view FileName)
	{
		const auto NtName = nt_path(FileName);

		const auto Result = low::get_file_attributes(NtName.c_str());
		if (Result != INVALID_FILE_ATTRIBUTES)
			return Result;

		if (ElevationRequired(ELEVATION_READ_REQUEST))
			return elevation::instance().get_file_attributes(NtName);

		return INVALID_FILE_ATTRIBUTES;
	}

	bool set_file_attributes(string_view const FileName, attributes const Attributes)
	{
		const auto NtName = nt_path(FileName);

		if (low::set_file_attributes(NtName.c_str(), Attributes))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
			return elevation::instance().set_file_attributes(NtName, Attributes);

		return false;
	}

	bool GetLongPathName(string_view const ShortPath, string& LongPath)
	{
		return os::detail::ApiDynamicStringReceiver(LongPath, [&](std::span<wchar_t> Buffer)
		{
			return ::GetLongPathName(null_terminated(ShortPath).c_str(), Buffer.data(), static_cast<DWORD>(Buffer.size()));
		});
	}

	bool GetShortPathName(string_view const LongPath, string& ShortPath)
	{
		return os::detail::ApiDynamicStringReceiver(ShortPath, [&](std::span<wchar_t> Buffer)
		{
			return ::GetShortPathName(null_terminated(LongPath).c_str(), Buffer.data(), static_cast<DWORD>(Buffer.size()));
		});
	}

	bool GetVolumeInformation(string_view const RootPathName, string* VolumeName, DWORD* VolumeSerialNumber, DWORD* MaximumComponentLength, DWORD* FileSystemFlags, string* FileSystemName)
	{
		wchar_t VolumeNameBuffer[MAX_PATH + 1], FileSystemNameBuffer[MAX_PATH + 1];
		if (VolumeName)
		{
			VolumeNameBuffer[0] = {};
		}
		if (FileSystemName)
		{
			FileSystemNameBuffer[0] = {};
		}

		if (!::GetVolumeInformation(null_terminated(RootPathName).c_str(), VolumeNameBuffer, static_cast<DWORD>(std::size(VolumeNameBuffer)), VolumeSerialNumber,
			MaximumComponentLength, FileSystemFlags, FileSystemNameBuffer, static_cast<DWORD>(std::size(FileSystemNameBuffer))))
			return false;

		if (VolumeName)
			*VolumeName = VolumeNameBuffer;

		if (FileSystemName)
			*FileSystemName = FileSystemNameBuffer;

		return true;
	}

	bool GetVolumeNameForVolumeMountPoint(string_view const VolumeMountPoint, string& VolumeName)
	{
		wchar_t VolumeNameBuffer[50];
		auto strVolumeMountPoint = nt_path(VolumeMountPoint);
		AddEndSlash(strVolumeMountPoint);
		if (!::GetVolumeNameForVolumeMountPoint(strVolumeMountPoint.c_str(), VolumeNameBuffer, static_cast<DWORD>(std::size(VolumeNameBuffer))))
			return false;

		VolumeName = VolumeNameBuffer;
		return true;
	}

	bool GetVolumePathNamesForVolumeName(const string& VolumeName, string& VolumePathNames)
	{
		if (!imports.GetVolumePathNamesForVolumeNameW)
			return false;

		return os::detail::ApiDynamicStringReceiver(VolumePathNames, [&](std::span<wchar_t> Buffer)
		{
			DWORD ReturnLength = 0;
			return (imports.GetVolumePathNamesForVolumeNameW(VolumeName.c_str(), Buffer.data(), static_cast<DWORD>(Buffer.size()), &ReturnLength) || !ReturnLength)?
				ReturnLength :
				ReturnLength + 1;
		});
	}

	bool QueryDosDevice(string_view const DeviceName, string &Path)
	{
		null_terminated const C_DeviceName(DeviceName);
		const auto DeviceNamePtr = EmptyToNull(C_DeviceName);
		return os::detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, Path, [&](std::span<wchar_t> Buffer)
		{
			const auto ReturnedSize = ::QueryDosDevice(DeviceNamePtr, Buffer.data(), static_cast<DWORD>(Buffer.size()));
			// Upon success it includes two trailing '\0', we don't need them
			return ReturnedSize? ReturnedSize - 2 : 0;
		});
	}

	bool SearchPath(const wchar_t* Path, string_view const FileName, const wchar_t* Extension, string& strDest)
	{
		return os::detail::ApiDynamicStringReceiver(strDest, [&](std::span<wchar_t> Buffer)
		{
			return ::SearchPath(Path, null_terminated(FileName).c_str(), Extension, static_cast<DWORD>(Buffer.size()), Buffer.data(), nullptr);
		});
	}

	bool GetTempPath(string& strBuffer)
	{
		return os::detail::ApiDynamicStringReceiver(strBuffer, [&](std::span<wchar_t> Buffer)
		{
			return ::GetTempPath(static_cast<DWORD>(Buffer.size()), Buffer.data());
		});
	}

	bool get_module_file_name(HANDLE hProcess, HMODULE hModule, string &strFileName)
	{
		return os::detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, strFileName, [&](std::span<wchar_t> Buffer)
		{
			if (!hProcess)
			{
				auto ReturnedSize = ::GetModuleFileName(hModule, Buffer.data(), static_cast<DWORD>(Buffer.size()));
				if (ReturnedSize == Buffer.size())
				{
					// os <= XP doesn't set this
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
					ReturnedSize = 0;
				}
				return ReturnedSize;
			}

			if (imports.QueryFullProcessImageNameW && !hModule)
			{
				auto Size = static_cast<DWORD>(Buffer.size());
				return imports.QueryFullProcessImageNameW(hProcess, 0, Buffer.data(), &Size)? Size : 0;
			}
			else
			{
				return ::GetModuleFileNameEx(hProcess, hModule, Buffer.data(), static_cast<DWORD>(Buffer.size()));
			}
		});
	}

	string get_current_process_file_name()
	{
		string FileName;

		// - This shouldn't fail
		// - It's tiresome to check every time
		// - There's not much we can do without it anyways
		if (!get_module_file_name({}, {}, FileName))
			throw far_fatal_exception(L"get_current_process_file_name");

		return FileName;
	}

	security::descriptor get_file_security(const string_view Object, const SECURITY_INFORMATION RequestedInformation)
	{
		const auto NtObject = nt_path(Object);

		if (auto Result = low::get_file_security(NtObject.c_str(), RequestedInformation))
			return Result;

		if (ElevationRequired(ELEVATION_READ_REQUEST))
			return elevation::instance().get_file_security(NtObject, RequestedInformation);

		return {};
	}

	bool set_file_security(const string_view Object, const SECURITY_INFORMATION RequestedInformation, const security::descriptor& SecurityDescriptor)
	{
		const auto NtObject = nt_path(Object);

		if (low::set_file_security(NtObject.c_str(), RequestedInformation, SecurityDescriptor.data()))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
			return elevation::instance().set_file_security(NtObject, RequestedInformation, SecurityDescriptor);

		return false;
	}

	bool reset_file_security(string_view const Object)
	{
		const auto NtObject = nt_path(Object);
		if (low::reset_file_security(NtObject.c_str()))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
			return elevation::instance().reset_file_security(NtObject);

		return false;
	}

	bool move_to_recycle_bin(string_view const Object)
	{
		if (low::move_to_recycle_bin(Object))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST, false)) // ShellAPI doesn't set LastNtStatus
			return elevation::instance().fMoveToRecycleBin(Object);

		return false;
	}

	bool get_disk_size(const string_view Path, unsigned long long* const UserTotal, unsigned long long* const TotalFree, unsigned long long* const UserFree)
	{
		auto strPath = nt_path(Path);
		AddEndSlash(strPath);

		if (low::get_disk_free_space(strPath.c_str(), UserFree, UserTotal, TotalFree))
			return true;

		if (ElevationRequired(ELEVATION_READ_REQUEST))
			return elevation::instance().get_disk_free_space(strPath.c_str(), UserFree, UserTotal, TotalFree);

		return false;
	}

	bool GetFileTimeSimple(const string_view FileName, chrono::time_point* const CreationTime, chrono::time_point* const LastAccessTime, chrono::time_point* const LastWriteTime, chrono::time_point* const ChangeTime)
	{
		file File;
		return File.Open(FileName, FILE_READ_ATTRIBUTES, file_share_all, nullptr, OPEN_EXISTING) && File.GetTime(CreationTime, LastAccessTime, LastWriteTime, ChangeTime);
	}

	bool get_find_data(string_view const FileName, find_data& FindData, bool ScanSymLink)
	{
		auto Find = enum_files(FileName, ScanSymLink);
		auto ItemIterator = Find.begin();
		if (ItemIterator != Find.end())
		{
			FindData = std::move(*ItemIterator);
			return true;
		}

		FindData = {};
		FindData.Attributes = INVALID_FILE_ATTRIBUTES;

		size_t DirOffset = 0;
		ParsePath(FileName, &DirOffset);
		if (FileName.find_first_of(L"*?"sv, DirOffset) != string::npos)
			return false;

		if ((FindData.Attributes = get_file_attributes(FileName)) == INVALID_FILE_ATTRIBUTES)
			return false;

		// Ага, значит файл таки есть. Заполним структуру ручками.
		if (const auto File = file(FileName, FILE_READ_ATTRIBUTES, file_share_all, nullptr, OPEN_EXISTING))
		{
			if (!File.GetTime(&FindData.CreationTime, &FindData.LastAccessTime, &FindData.LastWriteTime, &FindData.ChangeTime))
				return false;

			if (!File.GetSize(FindData.FileSize))
				return false;
		}

		if (FindData.Attributes & FILE_ATTRIBUTE_REPARSE_POINT)
		{
			string strTmp;
			const DWORD ReparseTag{};
			if (GetReparsePointInfo(FileName, strTmp, &FindData.ReparseTag))
				FindData.ReparseTag = ReparseTag;
		}

		FindData.FileName = PointToName(FileName);
		FindData.SetAlternateFileName(ConvertNameToShort(FileName));
		return true;
	}

	bool IsDiskInDrive(string_view const Root)
	{
		string strDrive(Root);
		AddEndSlash(strDrive);
		return os::fs::GetVolumeInformation(strDrive, {}, {}, {}, {}, {});
	}

	bool create_hard_link(string_view const FileName, string_view const ExistingFileName, SECURITY_ATTRIBUTES* SecurityAttributes)
	{
		const auto Create = [SecurityAttributes](const string& Object, const string& Target)
		{
			if (low::create_hard_link(Object.c_str(), Target.c_str(), SecurityAttributes))
				return true;

			if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
				return elevation::instance().create_hard_link(Object, Target, SecurityAttributes);

			return false;
		};

		if (Create(nt_path(FileName), nt_path(ExistingFileName)))
			return true;

		// win2k bug: \\?\ fails
		if (!IsWindowsXPOrGreater())
			return Create(ConvertNameToFull(FileName), ConvertNameToFull(ExistingFileName));

		return false;
	}

	bool CreateSymbolicLink(string_view const SymlinkFileName, string_view const TargetFileName, DWORD Flags)
	{
		const auto NtSymlinkFileName = nt_path(SymlinkFileName);

		if (CreateSymbolicLinkInternal(NtSymlinkFileName, TargetFileName, Flags))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
			return elevation::instance().fCreateSymbolicLink(NtSymlinkFileName, TargetFileName, Flags);

		return false;
	}

	bool set_file_encryption(const string_view FileName, const bool Encrypt)
	{
		const auto NtName = nt_path(FileName);

		if (low::set_file_encryption(NtName.c_str(), Encrypt))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST, false)) // Encryption implemented in adv32, NtStatus is not affected
			return elevation::instance().set_file_encryption(NtName, Encrypt);

		return false;
	}

	bool detach_virtual_disk(const string_view Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType)
	{
		const auto NtObject = nt_path(Object);

		if (low::detach_virtual_disk(NtObject.c_str(), VirtualStorageType))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
			return elevation::instance().detach_virtual_disk(NtObject, VirtualStorageType);

		return false;
	}

	bool is_directory_reparse_point(attributes const Attributes)
	{
		return Attributes != INVALID_FILE_ATTRIBUTES && flags::check_all(Attributes, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT);
	}

	bool is_directory_symbolic_link(const find_data& Data)
	{
		return is_directory_reparse_point(Data.Attributes) && (Data.ReparseTag == IO_REPARSE_TAG_MOUNT_POINT || Data.ReparseTag == IO_REPARSE_TAG_SYMLINK);
	}

	bool can_create_file(string_view const Name)
	{
		return file(Name, GENERIC_WRITE, file_share_all, nullptr, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE)? true : false;
	}

	bool CreateSymbolicLinkInternal(string_view const Object, string_view const Target, DWORD Flags)
	{
		if (!imports.CreateSymbolicLinkW)
			return CreateReparsePoint(Target, Object, Flags & SYMBOLIC_LINK_FLAG_DIRECTORY? RP_SYMLINKDIR : RP_SYMLINKFILE);

		static const DWORD unpriv_flag = version::is_win10_1703_or_later()? SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE : 0;

		return imports.CreateSymbolicLinkW(null_terminated(Object).c_str(), null_terminated(Target).c_str(), Flags | unpriv_flag) != FALSE;
	}

	drives_set allowed_drives_mask()
	{
		// It's good enough to read it once.
		static const auto AllowedDrivesMask = []
		{
			// Declared separately due to a VS19 bug
			const auto Where = { &reg::key::local_machine, &reg::key::current_user };
			for (const auto& i: Where)
			{
				if (const auto NoDrives = i->get<unsigned>(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"sv, L"NoDrives"sv))
					return ~*NoDrives;
			}

			return ~0u;
		}();

		return AllowedDrivesMask;
	}
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("drives")
{
	static const struct
	{
		wchar_t DriveLetter;
		string_view DriveDevicePath, Win32NtDriveDevicePath, DriveRootDirectory, Win32NtDriveRootDirectory;
		bool Standard;
		size_t Number;
	}
	Tests[]
	{
		{ L'A', L"A:"sv, L"\\\\?\\A:"sv, L"A:\\"sv, L"\\\\?\\A:\\"sv, true,  0,  },
		{ L'B', L"B:"sv, L"\\\\?\\B:"sv, L"B:\\"sv, L"\\\\?\\B:\\"sv, true,  1,  },
		{ L'C', L"C:"sv, L"\\\\?\\C:"sv, L"C:\\"sv, L"\\\\?\\C:\\"sv, true,  2,  },
		{ L'Z', L"Z:"sv, L"\\\\?\\Z:"sv, L"Z:\\"sv, L"\\\\?\\Z:\\"sv, true,  25, },
		{ L'1', L"1:"sv, L"\\\\?\\1:"sv, L"1:\\"sv, L"\\\\?\\1:\\"sv, false, 42, },
		{ L'λ', L"λ:"sv, L"\\\\?\\λ:"sv, L"λ:\\"sv, L"\\\\?\\λ:\\"sv, false, 42, },
	};

	for (const auto& i: Tests)
	{
		namespace osd = os::fs::drive;

		REQUIRE(osd::get_device_path(i.DriveLetter) == i.DriveDevicePath);
		REQUIRE(osd::get_win32nt_device_path(i.DriveLetter) == i.Win32NtDriveDevicePath);
		REQUIRE(osd::get_root_directory(i.DriveLetter) == i.DriveRootDirectory);
		REQUIRE(osd::get_win32nt_root_directory(i.DriveLetter) == i.Win32NtDriveRootDirectory);
		REQUIRE(osd::is_standard_letter(i.DriveLetter) == i.Standard);

		if (i.Standard)
		{
			REQUIRE(osd::get_number(i.DriveLetter) == i.Number);
			REQUIRE(osd::get_letter(i.Number) == i.DriveLetter);
			REQUIRE(osd::get_device_path(i.Number) == i.DriveDevicePath);
		}
	}
}
#endif
