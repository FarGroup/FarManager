#ifndef PLATFORM_FS_HPP_1094D8B6_7681_46C8_9C08_C5253376E988
#define PLATFORM_FS_HPP_1094D8B6_7681_46C8_9C08_C5253376E988
#pragma once

/*
platform.fs.hpp

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

// Internal:

// Platform:
#include "platform.hpp"
#include "platform.chrono.hpp"
#include "platform.security.hpp"

// Common:
#include "common/enumerator.hpp"
#include "common/function_ref.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::fs
{
	namespace detail
	{
		struct find_handle_closer
		{
			void operator()(HANDLE Handle) const noexcept;
		};

		struct find_file_handle_closer
		{
			void operator()(HANDLE Handle) const noexcept;
		};

		struct find_volume_handle_closer
		{
			void operator()(HANDLE Handle) const noexcept;
		};

		struct find_nt_handle_closer
		{
			void operator()(HANDLE Handle) const noexcept;
		};
	}

	namespace state
	{
		void set_current_directory_syncronisation(bool Value);
	}

	using find_handle = os::detail::handle_t<detail::find_handle_closer>;
	using find_file_handle = os::detail::handle_t<detail::find_file_handle_closer>;
	using find_volume_handle = os::detail::handle_t<detail::find_volume_handle_closer>;
	using find_nt_handle = os::detail::handle_t<detail::find_nt_handle_closer>;

	using drives_set = std::bitset<26>;

	using security_descriptor = block_ptr<SECURITY_DESCRIPTOR, default_buffer_size>;

	using attributes = DWORD;

	struct find_data
	{
	public:
		string FileName;
	private:
		string AlternateFileNameData;
	public:
		chrono::time_point CreationTime;
		chrono::time_point LastAccessTime;
		chrono::time_point LastWriteTime;
		chrono::time_point ChangeTime;
		unsigned long long FileSize{};
		unsigned long long AllocationSize{};
		unsigned long long FileId{};
		attributes Attributes{ INVALID_FILE_ATTRIBUTES };
		DWORD ReparseTag{};

		[[nodiscard]]
		const string& AlternateFileName() const;
		void SetAlternateFileName(string_view Name);
		[[nodiscard]]
		bool HasAlternateFileName() const;
	};

	namespace drive
	{
		[[nodiscard]]
		bool is_standard_letter(wchar_t Letter);

		[[nodiscard]]
		size_t get_number(wchar_t Letter);

		[[nodiscard]]
		wchar_t get_letter(size_t Number);

		[[nodiscard]]
		string get_device_path(wchar_t Letter);

		[[nodiscard]]
		string get_device_path(size_t Number);

		[[nodiscard]]
		string get_win32nt_device_path(wchar_t Letter);

		[[nodiscard]]
		string get_root_directory(wchar_t Letter);

		[[nodiscard]]
		string get_win32nt_root_directory(wchar_t Letter);

		unsigned get_type(string_view Path);
	}

	class [[nodiscard]] enum_drives: public enumerator<enum_drives, wchar_t>
	{
		IMPLEMENTS_ENUMERATOR(enum_drives);

	public:
		explicit enum_drives(drives_set Drives);

	private:
		[[nodiscard]]
		bool get(bool Reset, wchar_t& Value) const;

		drives_set m_Drives;
		mutable size_t m_CurrentIndex{};
	};

	class [[nodiscard]] enum_files: public enumerator<enum_files, find_data>
	{
		IMPLEMENTS_ENUMERATOR(enum_files);

	public:
		explicit enum_files(string_view Object, bool ScanSymlink = true);

	private:
		[[nodiscard]]
		bool get(bool Reset, find_data& Value) const;

		string m_Object;
		bool m_ScanSymlink;
		mutable find_file_handle m_Handle;
	};

	class [[nodiscard]] enum_names: public enumerator<enum_names, string>
	{
		IMPLEMENTS_ENUMERATOR(enum_names);

	public:
		explicit enum_names(string_view Object);

	private:
		[[nodiscard]]
		bool get(bool Reset, string& Value) const;

		string m_Object;
		mutable find_handle m_Handle;
	};

	class [[nodiscard]] enum_streams: public enumerator<enum_streams, WIN32_FIND_STREAM_DATA>
	{
		IMPLEMENTS_ENUMERATOR(enum_streams);

	public:
		explicit enum_streams(string_view Object);

	private:
		[[nodiscard]]
		bool get(bool Reset, WIN32_FIND_STREAM_DATA& Value) const;

		string m_Object;
		mutable find_file_handle m_Handle;
	};

	class [[nodiscard]] enum_volumes: public enumerator<enum_volumes, string>
	{
		IMPLEMENTS_ENUMERATOR(enum_volumes);

	public:
		enum_volumes();

	private:
		[[nodiscard]]
		bool get(bool Reset, string& Value) const;

		string m_Object;
		mutable find_volume_handle m_Handle;
	};

	class [[nodiscard]] enum_devices: public enumerator<enum_devices, string_view>
	{
		IMPLEMENTS_ENUMERATOR(enum_devices);

	public:
		explicit enum_devices(string_view Object);

	private:
		[[nodiscard]]
		bool get(bool Reset, string_view& Value) const;

		UNICODE_STRING m_Object;
		mutable find_nt_handle m_Handle;
		mutable char_ptr m_Buffer;
		mutable std::optional<size_t> m_Index{};
		mutable ULONG m_Context{};
	};

	class file
	{
	public:
		NONCOPYABLE(file);
		MOVABLE(file);

		file();

		explicit file(handle&& Handle);

		explicit file(auto&&... Args)
		{
			(void)Open(FWD(Args)...);
		}

		explicit operator bool() const noexcept;
		// TODO: half of these should be free functions

		[[nodiscard]]
		bool Open(string_view Object, DWORD DesiredAccess, DWORD ShareMode, SECURITY_ATTRIBUTES* SecurityAttributes, DWORD CreationDistribution, attributes FlagsAndAttributes = {}, const file* TemplateFile = nullptr, bool ForceElevation = false);
		// TODO: async overloads when needed

		[[nodiscard]]
		bool Read(void* Buffer, size_t NumberOfBytesToRead, size_t& NumberOfBytesRead) const;

		[[nodiscard]]
		bool Write(const void* Buffer, size_t NumberOfBytesToWrite) const;

		[[nodiscard]]
		unsigned long long GetPointer() const;

		bool SetPointer(long long DistanceToMove, unsigned long long* NewFilePointer, DWORD MoveMethod) const;

		bool SetEnd();

		[[nodiscard]]
		bool GetTime(chrono::time_point* CreationTime, chrono::time_point* LastAccessTime, chrono::time_point* LastWriteTime, chrono::time_point* ChangeTime) const;

		bool SetTime(const chrono::time_point* CreationTime, const chrono::time_point* LastAccessTime, const chrono::time_point* LastWriteTime, const chrono::time_point* ChangeTime) const;

		[[nodiscard]]
		bool GetSize(unsigned long long& Size) const;

		bool FlushBuffers() const;

		[[nodiscard]]
		bool GetInformation(BY_HANDLE_FILE_INFORMATION& info) const;

		[[nodiscard]]
		bool IoControl(DWORD IoControlCode, void* InBuffer, DWORD InBufferSize, void* OutBuffer, DWORD OutBufferSize, DWORD* BytesReturned = nullptr, OVERLAPPED* Overlapped = nullptr) const;

		[[nodiscard]]
		bool GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed) const;

		[[nodiscard]]
		bool NtQueryDirectoryFile(void* FileInformation, size_t Length, FILE_INFORMATION_CLASS FileInformationClass, bool ReturnSingleEntry, string_view FileName, bool RestartScan, NTSTATUS* Status = nullptr) const;

		[[nodiscard]]
		bool NtQueryInformationFile(void* FileInformation, size_t Length, FILE_INFORMATION_CLASS FileInformationClass, NTSTATUS* Status = nullptr) const;

		[[nodiscard]]
		bool GetFinalPathName(string& FinalFilePath) const;

		void Close();

		[[nodiscard]]
		bool Eof() const;

		[[nodiscard]]
		const string& GetName() const;

		[[nodiscard]]
		const handle& get() const;

	private:
		void SyncPointer() const;

		handle m_Handle;
		mutable unsigned long long m_Pointer;
		mutable bool m_NeedSyncPointer;
		string m_Name;
		DWORD m_ShareMode;
	};

	class file_walker: public file
	{
	public:
		file_walker();
		~file_walker();

		[[nodiscard]]
		bool InitWalk(size_t BlockSize);

		[[nodiscard]]
		bool Step();

		[[nodiscard]]
		unsigned long long GetChunkOffset() const;

		[[nodiscard]]
		DWORD GetChunkSize() const;

		[[nodiscard]]
		int GetPercent() const;

	private:
		struct Chunk;
		std::vector<Chunk> m_ChunkList;
		unsigned long long m_FileSize{};
		unsigned long long m_AllocSize{};
		unsigned long long m_ProcessedSize{};
		std::vector<Chunk>::iterator m_CurrentChunk{};
		DWORD m_ChunkSize{};
		bool m_IsSparse{};
	};

	class filebuf final: public std::streambuf
	{
	public:
		NONCOPYABLE(filebuf);

		filebuf(const file& File, std::ios::openmode Mode, size_t BufferSize = 65536);

	protected:
		int_type underflow() override;
		int_type overflow(int_type Ch) override;
		int sync() override;
		pos_type seekoff(off_type Offset, std::ios::seekdir Way, std::ios::openmode Which) override;
		pos_type seekpos(pos_type Pos, std::ios::openmode Which) override;
		int_type pbackfail(int_type Ch) override;

	private:
		void reset_get_area();
		void reset_put_area();
		bool adjust_pos_in_cache(std::streamoff Offset);
		unsigned long long set_pointer(unsigned long long Value, int Way);

		const file& m_File;
		std::ios::openmode m_Mode;
		std::vector<char> m_Buffer;
	};

	class file_status
	{
	public:
		file_status();
		explicit file_status(string_view Object);

		[[nodiscard]]
		bool check(DWORD Data) const;

	private:
		DWORD m_Data;
	};

	[[nodiscard]]
	bool exists(file_status Status);

	[[nodiscard]]
	bool exists(string_view Object);

	[[nodiscard]]
	bool is_file(file_status Status);

	[[nodiscard]]
	bool is_file(string_view Object);

	[[nodiscard]]
	bool is_file(find_data const& Data);

	[[nodiscard]]
	bool is_file(attributes Attributes);

	[[nodiscard]]
	bool is_directory(file_status Status);

	[[nodiscard]]
	bool is_directory(string_view Object);

	[[nodiscard]]
	bool is_directory(find_data const& Data);

	[[nodiscard]]
	bool is_directory(attributes Attributes);

	[[nodiscard]]
	bool is_not_empty_directory(string_view Object);

	bool is_file_name_too_long(string_view LongName);
	bool is_directory_name_too_long(string_view LongName);

	bool shorten(string_view Name, string& ShortName, function_ref<bool(string_view)> IsTooLong);

	class current_directory_guard
	{
	public:
		NONCOPYABLE(current_directory_guard);

		explicit current_directory_guard(string_view Directory);
		~current_directory_guard();

	private:
		string m_Directory;
		bool m_Active;
	};

	class process_current_directory_guard
	{
	public:
		NONCOPYABLE(process_current_directory_guard);

		explicit process_current_directory_guard(string_view Directory);
		~process_current_directory_guard();

	private:
		string m_Directory;
		bool m_Active;
	};


	constexpr DWORD
		// Usually there is no reason to forbid external deletion while reading
		file_share_read = FILE_SHARE_READ | FILE_SHARE_DELETE,
		file_share_all = file_share_read | FILE_SHARE_WRITE;

	namespace low
	{
		[[nodiscard]]
		HANDLE create_file(const wchar_t* FileName, DWORD DesiredAccess, DWORD ShareMode, SECURITY_ATTRIBUTES* SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile);

		[[nodiscard]]
		bool create_directory(const wchar_t* TemplateDirectory, const wchar_t* NewDirectory, SECURITY_ATTRIBUTES* SecurityAttributes);

		[[nodiscard]]
		bool remove_directory(const wchar_t* PathName);

		[[nodiscard]]
		bool delete_file(const wchar_t* FileName);

		[[nodiscard]]
		attributes get_file_attributes(const wchar_t* FileName);

		[[nodiscard]]
		bool set_file_attributes(const wchar_t* FileName, attributes Attributes);

		[[nodiscard]]
		bool create_hard_link(const wchar_t* FileName, const wchar_t* ExistingFileName, SECURITY_ATTRIBUTES* SecurityAttributes);

		[[nodiscard]]
		bool copy_file(const wchar_t* ExistingFileName, const wchar_t* NewFileName, LPPROGRESS_ROUTINE ProgressRoutine, void* Data, BOOL* Cancel, DWORD CopyFlags);

		[[nodiscard]]
		bool move_file(const wchar_t* ExistingFileName, const wchar_t* NewFileName, DWORD Flags);

		[[nodiscard]]
		bool replace_file(const wchar_t* ReplacedFileName, const wchar_t* ReplacementFileName, const wchar_t* BackupFileName, DWORD ReplaceFlags);

		[[nodiscard]]
		bool detach_virtual_disk(const wchar_t* Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType);

		[[nodiscard]]
		bool get_disk_free_space(const wchar_t* DirectoryName, unsigned long long* FreeBytesAvailableToCaller, unsigned long long* TotalNumberOfBytes, unsigned long long* TotalNumberOfFreeBytes);

		[[nodiscard]]
		bool set_file_encryption(const wchar_t* FileName, bool Encrypt);

		[[nodiscard]]
		security::descriptor get_file_security(const wchar_t* Object, SECURITY_INFORMATION RequestedInformation);

		[[nodiscard]]
		bool set_file_security(const wchar_t* Object, SECURITY_INFORMATION RequestedInformation, SECURITY_DESCRIPTOR* SecurityDescriptor);

		[[nodiscard]]
		bool reset_file_security(const wchar_t* Object);

		[[nodiscard]]
		bool move_to_recycle_bin(string_view Object);
	}

	[[nodiscard]]
	bool GetProcessRealCurrentDirectory(string& Directory);

	bool SetProcessRealCurrentDirectory(string_view Directory);

	void InitCurrentDirectory();

	[[nodiscard]]
	const string& get_current_directory();

	bool set_current_directory(string_view PathName, bool Validate = true);

	[[nodiscard]]
	bool create_directory(string_view PathName, SECURITY_ATTRIBUTES* SecurityAttributes = nullptr);

	[[nodiscard]]
	bool create_directory(string_view TemplateDirectory, string_view NewDirectory, SECURITY_ATTRIBUTES* SecurityAttributes = nullptr);

	[[nodiscard]]
	bool remove_directory(string_view DirName);

	[[nodiscard]]
	handle create_file(string_view Object, DWORD DesiredAccess, DWORD ShareMode, SECURITY_ATTRIBUTES* SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes = 0, HANDLE TemplateFile = nullptr, bool ForceElevation = false);

	[[nodiscard]]
	bool delete_file(string_view FileName);

	using progress_routine = function_ref<DWORD(
		unsigned long long TotalFileSize,
		unsigned long long TotalBytesTransferred,
		unsigned long long StreamSize,
		unsigned long long StreamBytesTransferred,
		DWORD StreamNumber,
		DWORD CallbackReason,
		HANDLE SourceFile,
		HANDLE DestinationFile
	)>;

	[[nodiscard]]
	bool copy_file(string_view ExistingFileName, string_view NewFileName, progress_routine ProgressRoutine, BOOL* Cancel, DWORD CopyFlags);

	[[nodiscard]]
	bool move_file(string_view ExistingFileName, string_view NewFileName, DWORD Flags = 0);

	[[nodiscard]]
	bool replace_file(string_view ReplacedFileName, string_view ReplacementFileName, string_view BackupFileName, DWORD Flags = 0);

	[[nodiscard]]
	attributes get_file_attributes(string_view FileName);

	[[nodiscard]]
	bool set_file_attributes(string_view FileName, attributes Attributes);

	[[nodiscard]]
	bool GetLongPathName(string_view ShortPath, string& LongPath);

	[[nodiscard]]
	bool GetShortPathName(string_view LongPath, string& ShortPath);

	[[nodiscard]]
	bool GetVolumeInformation(string_view RootPathName, string *VolumeName, DWORD* VolumeSerialNumber, DWORD* MaximumComponentLength, DWORD* FileSystemFlags, string* FileSystemName);

	[[nodiscard]]
	bool GetVolumeNameForVolumeMountPoint(string_view VolumeMountPoint, string& VolumeName);

	[[nodiscard]]
	bool GetVolumePathNamesForVolumeName(const string& VolumeName, string& VolumePathNames);

	[[nodiscard]]
	bool QueryDosDevice(string_view DeviceName, string& Path);

	[[nodiscard]]
	bool SearchPath(const wchar_t* Path, string_view FileName, const wchar_t* Extension, string& strDest);

	[[nodiscard]]
	bool GetTempPath(string& strBuffer);

	[[nodiscard]]
	bool get_module_file_name(HANDLE hProcess, HMODULE hModule, string& strFileName);

	[[nodiscard]]
	string get_current_process_file_name();

	[[nodiscard]]
	security::descriptor get_file_security(string_view Object, SECURITY_INFORMATION RequestedInformation);

	[[nodiscard]]
	bool set_file_security(string_view Object, SECURITY_INFORMATION RequestedInformation, const security::descriptor& SecurityDescriptor);

	[[nodiscard]]
	bool reset_file_security(string_view Object);

	[[nodiscard]]
	bool move_to_recycle_bin(string_view Object);

	[[nodiscard]]
	bool get_disk_size(string_view Path, unsigned long long* UserTotal, unsigned long long* TotalFree, unsigned long long* UserFree);

	[[nodiscard]]
	bool GetFileTimeSimple(string_view FileName, chrono::time_point* CreationTime, chrono::time_point* LastAccessTime, chrono::time_point* LastWriteTime, chrono::time_point* ChangeTime);

	[[nodiscard]]
	bool get_find_data(string_view FileName, find_data& FindData, bool ScanSymLink = true);

	[[nodiscard]]
	bool IsDiskInDrive(string_view Root);

	[[nodiscard]]
	bool create_hard_link(string_view FileName, string_view ExistingFileName, SECURITY_ATTRIBUTES* SecurityAttributes);

	[[nodiscard]]
	bool CreateSymbolicLink(string_view SymlinkFileName, string_view TargetFileName, DWORD Flags);

	[[nodiscard]]
	bool set_file_encryption(string_view FileName, bool Encrypt);

	[[nodiscard]]
	bool detach_virtual_disk(string_view Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType);

	[[nodiscard]]
	bool is_directory_reparse_point(attributes Attributes);

	[[nodiscard]]
	bool is_directory_symbolic_link(const find_data& Data);

	[[nodiscard]]
	bool can_create_file(string_view Name);


	[[nodiscard]]
	bool CreateSymbolicLinkInternal(string_view Object, string_view Target, DWORD Flags);

	[[nodiscard]]
	drives_set allowed_drives_mask();
}

#endif // PLATFORM_FS_HPP_1094D8B6_7681_46C8_9C08_C5253376E988
