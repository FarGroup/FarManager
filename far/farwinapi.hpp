#pragma once

/*
farwinapi.hpp

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


namespace api
{
	enum
	{
		NT_MAX_PATH = 32768
	};

	struct FAR_FIND_DATA
	{
		FILETIME ftCreationTime;
		FILETIME ftLastAccessTime;
		FILETIME ftLastWriteTime;
		FILETIME ftChangeTime;
		unsigned __int64 nFileSize;
		unsigned __int64 nAllocationSize;
		unsigned __int64 FileId;
		string strFileName;
		string strAlternateFileName;
		DWORD dwFileAttributes;
		DWORD dwReserved0;

		FAR_FIND_DATA()
		{
			Clear();
		}

		void Clear()
		{
			ClearStruct(ftCreationTime);
			ClearStruct(ftLastAccessTime);
			ClearStruct(ftLastWriteTime);
			ClearStruct(ftChangeTime);
			nFileSize=0;
			nAllocationSize=0;
			FileId = 0;
			strFileName.clear();
			strAlternateFileName.clear();
			dwFileAttributes=0;
			dwReserved0=0;
		}
	};

	class enum_file: NonCopyable, public enumerator<FAR_FIND_DATA>
	{
	public:
		enum_file(const string& Object, bool ScanSymLink = true);
		~enum_file();

	private:
		virtual bool get(size_t index, FAR_FIND_DATA& value) override;

		string m_Object;
		HANDLE m_Handle;
		bool m_ScanSymLink;
	};

	class File: NonCopyable
	{
	public:
		File();
		~File();
		File(File&& rhs):
			Handle(INVALID_HANDLE_VALUE),
			Pointer(),
			NeedSyncPointer(),
			share_mode()
		{
			*this = std::move(rhs);
		}

		MOVE_OPERATOR_BY_SWAP(File);

		void swap(File& rhs) noexcept
		{
			std::swap(Handle, rhs.Handle);
			std::swap(Pointer, rhs.Pointer);
			std::swap(NeedSyncPointer, rhs.NeedSyncPointer);
			name.swap(rhs.name);
			std::swap(share_mode, rhs.share_mode);
		}

		bool Open(const string& Object, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes=0, File* TemplateFile=nullptr, bool ForceElevation=false);
		bool Read(LPVOID Buffer, DWORD NumberOfBytesToRead, DWORD& NumberOfBytesRead, LPOVERLAPPED Overlapped = nullptr);
		bool Write(LPCVOID Buffer, DWORD NumberOfBytesToWrite, DWORD& NumberOfBytesWritten, LPOVERLAPPED Overlapped = nullptr);
		bool Read(LPVOID Buffer, size_t Nr, size_t& NumberOfBytesRead);
		bool Write(LPCVOID Buffer, size_t Nw, size_t& NumberOfBytesWritten);
		bool SetPointer(INT64 DistanceToMove, PINT64 NewFilePointer, DWORD MoveMethod);
		INT64 GetPointer(){return Pointer;}
		bool SetEnd();
		bool GetTime(LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime);
		bool SetTime(const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime, const FILETIME* ChangeTime);
		bool GetSize(UINT64& Size);
		bool FlushBuffers();
		bool GetInformation(BY_HANDLE_FILE_INFORMATION& info);
		bool IoControl(DWORD IoControlCode, LPVOID InBuffer, DWORD InBufferSize, LPVOID OutBuffer, DWORD OutBufferSize, LPDWORD BytesReturned, LPOVERLAPPED Overlapped = nullptr);
		bool GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed);
		bool NtQueryDirectoryFile(PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, bool ReturnSingleEntry, LPCWSTR FileName, bool RestartScan, NTSTATUS* Status = nullptr);
		bool NtQueryInformationFile(PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, NTSTATUS* Status = nullptr);
		bool Close();
		bool Eof();
		bool Opened() const {return Handle != INVALID_HANDLE_VALUE;}
		const string& GetName() const { return name; }
		HANDLE GetHandle() const { return Handle; }

	private:
		HANDLE Handle;
		INT64 Pointer;
		bool NeedSyncPointer;
		string name;
		DWORD share_mode;

		inline void SyncPointer();
	};

	class FileWalker: public File
	{
	public:
		FileWalker();
		bool InitWalk(size_t BlockSize);
		bool Step();
		UINT64 GetChunkOffset() const;
		DWORD GetChunkSize() const;
		int GetPercent() const;

	private:
		struct Chunk
		{
			UINT64 Offset;
			DWORD Size;
			Chunk(UINT64 Offset, DWORD Size):Offset(Offset), Size(Size) {}
		};
		std::list<Chunk> ChunkList;
		UINT64 FileSize;
		UINT64 AllocSize;
		UINT64 ProcessedSize;
		std::list<Chunk>::iterator CurrentChunk;
		DWORD ChunkSize;
		bool Sparse;
	};

	class sid_object: NonCopyable
	{
	public:
		sid_object(PSID_IDENTIFIER_AUTHORITY IdentifierAuthority, BYTE SubAuthorityCount, DWORD SubAuthority0 = 0, DWORD SubAuthority1 = 0, DWORD SubAuthority2 = 0, DWORD SubAuthority3 = 0, DWORD SubAuthority4 = 0, DWORD SubAuthority5 = 0, DWORD SubAuthority6 = 0, DWORD SubAuthority7 = 0)
		{
			if (!AllocateAndInitializeSid(IdentifierAuthority, SubAuthorityCount, SubAuthority0, SubAuthority1, SubAuthority2, SubAuthority3, SubAuthority4, SubAuthority5, SubAuthority6, SubAuthority7, &m_value))
			{
				throw FarRecoverableException("unable to allocate and initialize SID");
			}
		}

		~sid_object()
		{
			FreeSid(m_value);
		}

		PSID get() const { return m_value; }

	private:
		PSID m_value;
	};

	NTSTATUS GetLastNtStatus();
	DWORD GetCurrentDirectory(string &strCurDir);
	DWORD GetTempPath(string &strBuffer);
	DWORD GetModuleFileName(HMODULE hModule, string &strFileName);
	DWORD GetModuleFileNameEx(HANDLE hProcess, HMODULE hModule, string &strFileName);
	DWORD WNetGetConnection(const string& LocalName, string &RemoteName);
	BOOL GetVolumeInformation(const string& RootPathName, string *pVolumeName, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, string *pFileSystemName);
	BOOL FindStreamClose(HANDLE hFindFile);
	bool GetFindDataEx(const string& FileName, FAR_FIND_DATA& FindData, bool ScanSymLink=true);
	bool GetFileSizeEx(HANDLE hFile, UINT64 &Size);
	BOOL DeleteFile(const string& FileName);
	BOOL RemoveDirectory(const string& DirName);
	HANDLE CreateFile(const string& Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes=0, HANDLE TemplateFile=nullptr, bool ForceElevation = false);
	BOOL CopyFileEx(const string& ExistingFileName, const string& NewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
	BOOL MoveFile(const string& ExistingFileName, const string& NewFileName);
	BOOL MoveFileEx(const string& ExistingFileName, const string& NewFileName, DWORD dwFlags);
	BOOL IsDiskInDrive(const string& Root);
	int GetFileTypeByName(const string& Name);
	bool GetDiskSize(const string& Path, unsigned __int64 *TotalSize, unsigned __int64 *TotalFree, unsigned __int64 *UserFree);
	HANDLE FindFirstFileName(const string& FileName, DWORD dwFlags, string& LinkName);
	BOOL FindNextFileName(HANDLE hFindStream, string& LinkName);
	BOOL CreateDirectory(const string& PathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
	BOOL CreateDirectoryEx(const string& TemplateDirectory, const string& NewDirectory, LPSECURITY_ATTRIBUTES SecurityAttributes);
	DWORD GetFileAttributes(const string& FileName);
	BOOL SetFileAttributes(const string& FileName, DWORD dwFileAttributes);
	void InitCurrentDirectory();
	BOOL SetCurrentDirectory(const string& PathName, bool Validate = true);
	bool CreateSymbolicLink(const string& SymlinkFileName, const string& TargetFileName, DWORD dwFlags);
	bool SetFileEncryption(const string& Name, bool Encrypt);
	BOOL CreateHardLink(const string& FileName, const string& ExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
	HANDLE FindFirstStream(const string& FileName, STREAM_INFO_LEVELS InfoLevel, LPVOID lpFindStreamData, DWORD dwFlags = 0);
	BOOL FindNextStream(HANDLE hFindStream, LPVOID lpFindStreamData);
	std::vector<string> GetLogicalDriveStrings();
	bool GetFinalPathNameByHandle(HANDLE hFile, string& FinalFilePath);
	bool SearchPath(const wchar_t *Path, const string& FileName, const wchar_t *Extension, string &strDest);
	bool QueryDosDevice(const string& DeviceName, string& Path);
	bool GetVolumeNameForVolumeMountPoint(const string& VolumeMountPoint, string& VolumeName);
	bool GetFileTimeSimple(const string &FileName, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime);
	void EnableLowFragmentationHeap();
	typedef block_ptr<SECURITY_DESCRIPTOR> FAR_SECURITY_DESCRIPTOR;
	bool GetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, FAR_SECURITY_DESCRIPTOR& SecurityDescriptor);
	bool SetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, const FAR_SECURITY_DESCRIPTOR& SecurityDescriptor);
	bool OpenVirtualDisk(VIRTUAL_STORAGE_TYPE& VirtualStorageType, const string& Object, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, OPEN_VIRTUAL_DISK_PARAMETERS& Parameters, HANDLE& Handle);
	string GetPrivateProfileString(const string& AppName, const string& KeyName, const string& Default, const string& FileName);
	DWORD GetAppPathsRedirectionFlag();

	bool CreateSymbolicLinkInternal(const string& Object, const string& Target, DWORD dwFlags);
	bool SetFileEncryptionInternal(const wchar_t* Name, bool Encrypt);
	bool GetFileTimeEx(HANDLE Object, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime);
	bool SetFileTimeEx(HANDLE Object, const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime, const FILETIME* ChangeTime);
	bool OpenVirtualDiskInternal(VIRTUAL_STORAGE_TYPE& VirtualStorageType, const string& Object, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, OPEN_VIRTUAL_DISK_PARAMETERS& Parameters, HANDLE& Handle);

	namespace reg
	{
		inline bool IsStringType(DWORD Type)
		{
			return Type == REG_SZ || Type == REG_EXPAND_SZ || Type == REG_MULTI_SZ;
		}

		class key:NonCopyable
		{
		public:
			key(HKEY Key): m_Key(Key), m_Opened() {}
			key(HKEY RootKey, const wchar_t* SubKey, REGSAM Sam);
			~key();

			operator bool() const { return m_Key != nullptr; }
			HKEY Key() const { return m_Key; }

		private:
			HKEY m_Key;
			bool m_Opened;
		};

		class value
		{
		public:
			value(): m_Type(REG_NONE), m_Key(nullptr) {}

			const string& Name() const { return m_Name; }
			DWORD Type() const { return m_Type; }

			string GetString() const;
			unsigned int GetUnsigned() const;
			uint64_t GetUnsigned64() const;

		private:
			friend bool EnumValue(HKEY, size_t, value&);

			string m_Name;
			DWORD m_Type;
			HKEY m_Key;
		};

		bool EnumKey(HKEY Key, size_t Index, string& Name);
		bool EnumValue(HKEY Key, size_t Index, value& Value);

		bool GetValue(HKEY Key, const wchar_t* Name, string& Value);
		bool GetValue(HKEY Key, const wchar_t* Name, unsigned int& Value);
		bool GetValue(HKEY Key, const wchar_t* Name, uint64_t& Value);

#define IS_SAME(T1, T2) std::is_same<T1, T2>::value
#define CHECK_TYPE(T) static_assert(IS_SAME(T, string) || IS_SAME(T, unsigned int) || IS_SAME(T, uint64_t), "this type is not supported");

		template<class T>
		bool GetValue(HKEY Key, const string& Name, T& Value) { CHECK_TYPE(T); return GetValue(Key, Name.data(), Value); }

		template<class T>
		bool GetValue(HKEY RootKey, const wchar_t* SubKey, const wchar_t* Name, T& Value, REGSAM Sam = 0)
		{
			CHECK_TYPE(T);
			key Key(RootKey, SubKey, KEY_QUERY_VALUE | Sam);
			return Key && GetValue(Key.Key(), Name, Value);
		}

		template<class T>
		bool GetValue(HKEY RootKey, const string& SubKey, const wchar_t* Name, T& Value, REGSAM Sam = 0) { CHECK_TYPE(T); return GetValue(RootKey, SubKey.data(), Name, Value, Sam); }
		template<class T>
		bool GetValue(HKEY RootKey, const wchar_t* SubKey, const string& Name, T& Value, REGSAM Sam = 0) { CHECK_TYPE(T); return GetValue(RootKey, SubKey, Name.data(), Value, Sam); }
		template<class T>
		bool GetValue(HKEY RootKey, const string& SubKey, const string& Name, T& Value, REGSAM Sam = 0) { CHECK_TYPE(T); return GetValue(RootKey, SubKey.data(), Name.data(), Value, Sam); }

#undef CHECK_TYPE
#undef IS_SAME

		class enum_key: NonCopyable, public enumerator<string>
		{
		public:
			enum_key(HKEY Key): m_Key(Key) {}
			enum_key(HKEY RootKey, const wchar_t* SubKey, REGSAM Sam = 0): m_Key(RootKey, SubKey, KEY_ENUMERATE_SUB_KEYS | Sam) {}
			virtual bool get(size_t Index, string& Value) override { return m_Key && EnumKey(m_Key.Key(), Index, Value); }

		private:
			key m_Key;
		};

		class enum_value: NonCopyable, public enumerator<value>
		{
		public:
			enum_value(HKEY Key): m_Key(Key) {}
			enum_value(HKEY RootKey, const wchar_t* SubKey, REGSAM Sam = 0): m_Key(RootKey, SubKey, KEY_QUERY_VALUE | Sam) {}
			virtual bool get(size_t Index, value& Value) override { return m_Key && EnumValue(m_Key.Key(), Index, Value); }

		private:
			key m_Key;
		};
	}

	namespace env
	{
		class enum_strings: NonCopyable, public enumerator<const wchar_t*>
		{
		public:
			enum_strings();
			~enum_strings();

			virtual bool get(size_t index, const wchar_t*& value) override;

		private:
			wchar_t* m_Environment;
		};

		bool get_variable(const wchar_t* Name, string& strBuffer);
		inline bool get_variable(const string& Name, string& strBuffer) { return get_variable(Name.data(), strBuffer); }

		bool set_variable(const wchar_t* Name, const wchar_t* Value);
		inline bool set_variable(const wchar_t* Name, const string& Value) { return set_variable(Name, Value.data()); }
		inline bool set_variable(const string& Name, const wchar_t* Value) { return set_variable(Name.data(), Value); }
		inline bool set_variable(const string& Name, const string& Value) { return set_variable(Name.data(), Value.data()); }

		bool delete_variable(const wchar_t* Name);
		inline bool delete_variable(const string& Name) { return delete_variable(Name.data()); }

		string expand_strings(const wchar_t* str);
		inline string expand_strings(const string& str) { return expand_strings(str.data()); }
	}
}

STD_SWAP_SPEC(api::File);
