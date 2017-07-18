#ifndef FARWINAPI_HPP_632CB91D_08A9_4793_8FC7_2E38C30CE234
#define FARWINAPI_HPP_632CB91D_08A9_4793_8FC7_2E38C30CE234
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

namespace os
{
	namespace detail
	{
		template<class deleter>
		class handle_t: public base<std::unique_ptr<std::remove_pointer_t<HANDLE>, deleter>>
		{
			using base_type = typename handle_t::base_type;

		public:
			MOVABLE(handle_t);

			constexpr handle_t() = default;
			constexpr handle_t(std::nullptr_t) {}
			explicit handle_t(HANDLE Handle): base_type(normalise(Handle)) {}
			void reset(HANDLE Handle = nullptr) { base_type::reset(normalise(Handle)); }
			auto native_handle() const { return base_type::get(); }
			void close() { reset(); }
			bool wait(DWORD Milliseconds = INFINITE) const { return WaitForSingleObject(native_handle(), Milliseconds) == WAIT_OBJECT_0; }
			bool is_signaled() const { return wait(0); }

		private:
			static HANDLE normalise(HANDLE Handle) { return Handle == INVALID_HANDLE_VALUE? nullptr : Handle; }
		};

		struct handle_closer { void operator()(HANDLE Handle) const; };
		struct find_handle_closer { void operator()(HANDLE Handle) const; };
		struct find_file_handle_closer { void operator()(HANDLE Handle) const; };
		struct find_volume_handle_closer { void operator()(HANDLE Handle) const; };
		struct find_notification_handle_closer { void operator()(HANDLE Handle) const; };
		struct printer_handle_closer { void operator()(HANDLE Handle) const; };
	}

	using handle = detail::handle_t<detail::handle_closer>;
	using find_handle = detail::handle_t<detail::find_handle_closer>;
	using find_file_handle = detail::handle_t<detail::find_file_handle_closer>;
	using find_volume_handle = detail::handle_t<detail::find_volume_handle_closer>;
	using find_notification_handle = detail::handle_t<detail::find_notification_handle_closer>;
	using printer_handle = detail::handle_t<detail::printer_handle_closer>;

	enum
	{
		NT_MAX_PATH = 32768
	};

	struct FAR_FIND_DATA
	{
		FILETIME ftCreationTime{};
		FILETIME ftLastAccessTime{};
		FILETIME ftLastWriteTime{};
		FILETIME ftChangeTime{};
		unsigned long long nFileSize{};
		unsigned long long nAllocationSize{};
		unsigned long long FileId{};
		string strFileName;
		string strAlternateFileName;
		DWORD dwFileAttributes{};
		DWORD dwReserved0{};
	};

	namespace detail
	{
		struct sid_deleter { void operator()(PSID Sid) const noexcept { FreeSid(Sid); } };
	}

	using sid_ptr = std::unique_ptr<std::remove_pointer_t<PSID>, detail::sid_deleter>;

	inline auto make_sid(PSID_IDENTIFIER_AUTHORITY IdentifierAuthority, BYTE SubAuthorityCount, DWORD SubAuthority0 = 0, DWORD SubAuthority1 = 0, DWORD SubAuthority2 = 0, DWORD SubAuthority3 = 0, DWORD SubAuthority4 = 0, DWORD SubAuthority5 = 0, DWORD SubAuthority6 = 0, DWORD SubAuthority7 = 0)
	{
		PSID Sid;
		return sid_ptr(AllocateAndInitializeSid(IdentifierAuthority, SubAuthorityCount, SubAuthority0, SubAuthority1, SubAuthority2, SubAuthority3, SubAuthority4, SubAuthority5, SubAuthority6, SubAuthority7, &Sid)? Sid : nullptr);
	}

	NTSTATUS GetLastNtStatus();
	string GetCurrentDirectory();
	bool GetProcessRealCurrentDirectory(string& Directory);
	bool SetProcessRealCurrentDirectory(const string& Directory);
	DWORD GetTempPath(string &strBuffer);
	bool GetModuleFileName(HMODULE hModule, string &strFileName);
	bool GetModuleFileNameEx(HANDLE hProcess, HMODULE hModule, string &strFileName);
	bool WNetGetConnection(const string& LocalName, string &RemoteName);
	bool GetVolumeInformation(const string& RootPathName, string *pVolumeName, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, string *pFileSystemName);
	bool GetFindDataEx(const string& FileName, FAR_FIND_DATA& FindData, bool ScanSymLink=true);
	bool GetFileSizeEx(HANDLE hFile, unsigned long long& Size);
	bool DeleteFile(const string& FileName);
	bool RemoveDirectory(const string& DirName);
	handle CreateFile(const string& Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes=0, HANDLE TemplateFile=nullptr, bool ForceElevation = false);
	bool CopyFileEx(const string& ExistingFileName, const string& NewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
	bool MoveFile(const string& ExistingFileName, const string& NewFileName);
	bool MoveFileEx(const string& ExistingFileName, const string& NewFileName, DWORD dwFlags);
	bool IsDiskInDrive(const string& Root);
	int GetFileTypeByName(const string& Name);
	bool GetDiskSize(const string& Path, unsigned long long* TotalSize, unsigned long long* TotalFree, unsigned long long* UserFree);
	find_handle FindFirstFileName(const string& FileName, DWORD dwFlags, string& LinkName);
	bool FindNextFileName(const find_handle& hFindStream, string& LinkName);
	bool CreateDirectory(const string& PathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
	bool CreateDirectoryEx(const string& TemplateDirectory, const string& NewDirectory, LPSECURITY_ATTRIBUTES SecurityAttributes);
	DWORD GetFileAttributes(const string& FileName);
	bool SetFileAttributes(const string& FileName, DWORD dwFileAttributes);
	void InitCurrentDirectory();
	bool SetCurrentDirectory(const string& PathName, bool Validate = true);
	bool CreateSymbolicLink(const string& SymlinkFileName, const string& TargetFileName, DWORD dwFlags);
	bool SetFileEncryption(const string& Name, bool Encrypt);
	bool CreateHardLink(const string& FileName, const string& ExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
	find_file_handle FindFirstStream(const string& FileName, STREAM_INFO_LEVELS InfoLevel, LPVOID lpFindStreamData, DWORD dwFlags = 0);
	bool FindNextStream(const find_file_handle& hFindStream, LPVOID lpFindStreamData);
	bool GetFinalPathNameByHandle(HANDLE hFile, string& FinalFilePath);
	bool SearchPath(const wchar_t *Path, const string& FileName, const wchar_t *Extension, string &strDest);
	bool QueryDosDevice(const string& DeviceName, string& Path);
	bool GetVolumeNameForVolumeMountPoint(const string& VolumeMountPoint, string& VolumeName);
	bool GetFileTimeSimple(const string &FileName, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime);
	void EnableLowFragmentationHeap();
	using FAR_SECURITY_DESCRIPTOR = block_ptr<SECURITY_DESCRIPTOR>;
	bool GetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, FAR_SECURITY_DESCRIPTOR& SecurityDescriptor);
	bool SetFileSecurity(const string& Object, SECURITY_INFORMATION RequestedInformation, const FAR_SECURITY_DESCRIPTOR& SecurityDescriptor);
	bool DetachVirtualDisk(const string& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType);
	string GetPrivateProfileString(const string& AppName, const string& KeyName, const string& Default, const string& FileName);
	bool GetWindowText(HWND Hwnd, string& Text);
	bool IsWow64Process();
	DWORD GetAppPathsRedirectionFlag();
	bool GetDefaultPrinter(string& Printer);
	find_notification_handle FindFirstChangeNotification(const string& PathName, bool WatchSubtree, DWORD NotifyFilter);

	bool CreateSymbolicLinkInternal(const string& Object, const string& Target, DWORD dwFlags);
	bool SetFileEncryptionInternal(const wchar_t* Name, bool Encrypt);
	bool DetachVirtualDiskInternal(const string& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType);

	string GetLocaleValue(LCID lcid, LCTYPE id);

	handle OpenCurrentThread();

	namespace fs
	{
		using drives_set = std::bitset<26>;

		inline auto enum_drives(drives_set Drives)
		{
			using value_type = wchar_t;
			size_t CurrentIndex = 0;
			return make_inline_enumerator<value_type>([CurrentIndex, Drives](size_t Index, value_type& Value) mutable
			{
				if (!Index)
					CurrentIndex = 0;

				while (CurrentIndex != Drives.size() && !Drives[CurrentIndex])
					++CurrentIndex;

				if (CurrentIndex == Drives.size())
					return false;

				Value = static_cast<wchar_t>(L'A' + CurrentIndex);
				++CurrentIndex;
				return true;
			});
		}

		bool is_standard_drive_letter(wchar_t Letter);
		int get_drive_number(wchar_t Letter);
		string get_drive(wchar_t Letter);
		string get_root_directory(wchar_t Letter);

		namespace detail
		{
			string enum_files_prepare(const string&);
			bool get_file_impl(find_file_handle&, const string&, size_t, FAR_FIND_DATA&, bool);
		}

		void enum_files(const string&&, bool = true) = delete;
		inline auto enum_files(const string& Object, bool ScanSymLink = true)
		{
			using value_type = FAR_FIND_DATA;
			find_file_handle Handle;
			return make_inline_enumerator<value_type>([ScanSymLink, m_Object = detail::enum_files_prepare(Object), m_Handle = std::move(Handle)](size_t Index, value_type& Value) mutable
			{
				return detail::get_file_impl(m_Handle, m_Object, Index, Value, ScanSymLink);
			});
		}

		namespace detail
		{
			bool get_name_impl(find_handle&, const string&, size_t, string&);
		}

		void enum_names(const string&&) = delete;
		inline auto enum_names(const string& Object)
		{
			using value_type = string;
			find_handle Handle;
			return make_inline_enumerator<value_type>([&Object, m_Handle = std::move(Handle)](size_t Index, value_type& Value) mutable
			{
				return detail::get_name_impl(m_Handle, Object, Index, Value);
			});
		}

		namespace detail
		{
			bool get_stream_impl(find_file_handle&, const string&, size_t, WIN32_FIND_STREAM_DATA&);
		}

		void enum_streams(const string&&) = delete;
		inline auto enum_streams(const string& Object)
		{
			using value_type = WIN32_FIND_STREAM_DATA;
			find_file_handle Handle;
			return make_inline_enumerator<value_type>([&Object, m_Handle = std::move(Handle)](size_t Index, value_type& Value) mutable
			{
				return detail::get_stream_impl(m_Handle, Object, Index, Value);
			});
		}

		namespace detail
		{
			bool get_volume_impl(find_volume_handle&, size_t, string&);
		}

		inline auto enum_volumes()
		{
			using value_type = string;
			find_volume_handle Handle;
			return make_inline_enumerator<value_type>([m_Handle = std::move(Handle)](size_t Index, value_type& Value) mutable
			{
				return detail::get_volume_impl(m_Handle, Index, Value);
			});
		}

		class file: public conditional<file>
		{
		public:
			NONCOPYABLE(file);
			MOVABLE(file);

			file():
				m_Pointer(),
				m_NeedSyncPointer(),
				m_ShareMode()
			{
			}

			template<typename... args>
			explicit file(args... Args)
			{
				Open(FWD(Args)...);
			}

			bool operator!() const noexcept { return !m_Handle; }

			// TODO: half of these should be free functions
			bool Open(const string& Object, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes = 0, const file* TemplateFile = nullptr, bool ForceElevation = false);
			bool Read(LPVOID Buffer, size_t NumberOfBytesToRead, size_t& NumberOfBytesRead, LPOVERLAPPED Overlapped = nullptr) const;
			bool Write(LPCVOID Buffer, size_t NumberOfBytesToWrite, size_t& NumberOfBytesWritten, LPOVERLAPPED Overlapped = nullptr) const;
			bool SetPointer(long long DistanceToMove, unsigned long long* NewFilePointer, DWORD MoveMethod) const;
			unsigned long long GetPointer() const { return m_Pointer; }
			bool SetEnd();
			bool GetTime(LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime) const;
			bool SetTime(const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime, const FILETIME* ChangeTime) const;
			bool GetSize(unsigned long long& Size) const;
			bool FlushBuffers() const;
			bool GetInformation(BY_HANDLE_FILE_INFORMATION& info) const;
			bool IoControl(DWORD IoControlCode, LPVOID InBuffer, DWORD InBufferSize, LPVOID OutBuffer, DWORD OutBufferSize, LPDWORD BytesReturned, LPOVERLAPPED Overlapped = nullptr) const;
			bool GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG Flags, ULONG StorageDependencyInfoSize, PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo, PULONG SizeUsed) const;
			bool NtQueryDirectoryFile(PVOID FileInformation, size_t Length, FILE_INFORMATION_CLASS FileInformationClass, bool ReturnSingleEntry, LPCWSTR FileName, bool RestartScan, NTSTATUS* Status = nullptr) const;
			bool NtQueryInformationFile(PVOID FileInformation, size_t Length, FILE_INFORMATION_CLASS FileInformationClass, NTSTATUS* Status = nullptr) const;
			void Close();
			bool Eof() const;
			const string& GetName() const { return m_Name; }
			const auto& handle() const { return m_Handle; }

		private:
			void SyncPointer() const;

			os::handle m_Handle;
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

			bool InitWalk(size_t BlockSize);
			bool Step();
			unsigned long long GetChunkOffset() const;
			DWORD GetChunkSize() const;
			int GetPercent() const;

		private:
			struct Chunk;
			std::vector<Chunk> m_ChunkList;
			unsigned long long m_FileSize;
			unsigned long long m_AllocSize;
			unsigned long long m_ProcessedSize;
			std::vector<Chunk>::iterator m_CurrentChunk;
			DWORD m_ChunkSize;
			bool m_IsSparse;
		};

		class file_status
		{
		public:
			file_status();
			file_status(const string& Object);
			file_status(const wchar_t* Object);

			bool check(DWORD Data) const;

		private:
			DWORD m_Data;
		};

		bool exists(file_status Status);
		bool is_file(file_status Status);
		bool is_directory(file_status Status);

		bool is_not_empty_directory(const string& Object);

		class process_current_directory_guard: noncopyable
		{
		public:
			process_current_directory_guard(bool Active, const std::function<string()>& Provider):
				m_Active(Active && os::GetProcessRealCurrentDirectory(m_Directory))
			{
				if (m_Active && Provider)
					os::SetProcessRealCurrentDirectory(Provider());
			}

			~process_current_directory_guard()
			{
				if (m_Active)
					os::SetProcessRealCurrentDirectory(m_Directory);
			}

		private:
			string m_Directory;
			bool m_Active;
		};
	}

	namespace reg
	{
		namespace detail
		{
			inline bool IsStringType(DWORD Type)
			{
				return Type == REG_SZ || Type == REG_EXPAND_SZ || Type == REG_MULTI_SZ;
			}

			struct hkey_deleter { void operator()(HKEY Key) const { RegCloseKey(Key); } };
		}

		using key = std::unique_ptr<std::remove_pointer_t<HKEY>, detail::hkey_deleter>;

		key open_key(HKEY RootKey, const wchar_t* SubKey, DWORD SamDesired);

		class value
		{
		public:
			value(): m_Type(REG_NONE), m_Key() {}

			const string& Name() const { return m_Name; }
			DWORD Type() const { return m_Type; }

			string GetString() const;
			unsigned int GetUnsigned() const;
			unsigned long long GetUnsigned64() const;

		private:
			friend bool EnumValue(const key&, size_t, value&);

			string m_Name;
			DWORD m_Type;
			const key* m_Key;
		};

		bool EnumKey(const key& Key, size_t Index, string& Name);
		bool EnumValue(const key& Key, size_t Index, value& Value);

		bool GetValue(const key& Key, const wchar_t* Name);
		bool GetValue(const key& Key, const wchar_t* Name, string& Value);
		bool GetValue(const key& Key, const wchar_t* Name, unsigned int& Value);
		bool GetValue(const key& Key, const wchar_t* Name, unsigned long long& Value);

		namespace detail
		{
			template<typename T>
			using is_supported_type = typename is_one_of<T, string, unsigned int, unsigned long long>::type;
		}

		template<class T>
		bool GetValue(const key& Key, const string& Name, T& Value)
		{
			static_assert(detail::is_supported_type<T>::value); return GetValue(Key, Name.data(), Value);
		}

		template<class T>
		bool GetValue(HKEY RootKey, const wchar_t* SubKey, const wchar_t* Name, T& Value, REGSAM Sam = 0)
		{
			static_assert(detail::is_supported_type<T>::value);
			const auto Key = open_key(RootKey, SubKey, KEY_QUERY_VALUE | Sam);
			return Key && GetValue(Key, Name, Value);
		}

		template<class T>
		bool GetValue(HKEY RootKey, const string& SubKey, const wchar_t* Name, T& Value, REGSAM Sam = 0)
		{
			static_assert(detail::is_supported_type<T>::value); return GetValue(RootKey, SubKey.data(), Name, Value, Sam);
		}
		
		template<class T>
		bool GetValue(HKEY RootKey, const wchar_t* SubKey, const string& Name, T& Value, REGSAM Sam = 0)
		{
			static_assert(detail::is_supported_type<T>::value); return GetValue(RootKey, SubKey, Name.data(), Value, Sam);
		}
		
		template<class T>
		bool GetValue(HKEY RootKey, const string& SubKey, const string& Name, T& Value, REGSAM Sam = 0)
		{
			static_assert(detail::is_supported_type<T>::value); return GetValue(RootKey, SubKey.data(), Name.data(), Value, Sam);
		}

		class enum_key: noncopyable, public enumerator<enum_key, string>
		{
			IMPLEMENTS_ENUMERATOR(enum_key);

		public:
			explicit enum_key(const key& Key): m_KeyRef(Key) {}
			enum_key(HKEY RootKey, const wchar_t* SubKey, REGSAM Sam = 0): m_Key(open_key(RootKey, SubKey, KEY_ENUMERATE_SUB_KEYS | Sam)), m_KeyRef(m_Key) {}

		private:
			bool get(size_t Index, value_type& Value) const { return m_KeyRef && EnumKey(m_KeyRef, Index, Value); }

			const key m_Key;
			const key& m_KeyRef;
		};

		class enum_value: noncopyable, public enumerator<enum_value, value>
		{
			IMPLEMENTS_ENUMERATOR(enum_value);

		public:
			explicit enum_value(const key& Key): m_KeyRef(Key) {}
			enum_value(HKEY RootKey, const wchar_t* SubKey, REGSAM Sam = 0): m_Key(open_key(RootKey, SubKey, KEY_QUERY_VALUE | Sam)), m_KeyRef(m_Key) {}

		private:
			bool get(size_t Index, value_type& Value) const { return m_KeyRef && EnumValue(m_KeyRef, Index, Value); }

			const key m_Key;
			const key& m_KeyRef;
		};
	}

	namespace env
	{
		namespace provider
		{
			namespace detail
			{
				class provider: noncopyable
				{
				public:
					const wchar_t* data() const { return m_Data; }

				protected:
					provider() = default;

					wchar_t* m_Data;
				};
			}

			class strings: public detail::provider
			{
			public:
				strings();
				~strings();
			};

			class block: public detail::provider
			{
			public:
				block();
				~block();
			};
		}

		bool get_variable(const wchar_t* Name, string& strBuffer);
		inline bool get_variable(const string& Name, string& strBuffer) { return get_variable(Name.data(), strBuffer); }
		template<class T>
		string get_variable(const T& Name) { string Result; get_variable(Name, Result); return Result; }

		bool set_variable(const wchar_t* Name, const wchar_t* Value);
		inline bool set_variable(const wchar_t* Name, const string& Value) { return set_variable(Name, Value.data()); }
		inline bool set_variable(const string& Name, const wchar_t* Value) { return set_variable(Name.data(), Value); }
		inline bool set_variable(const string& Name, const string& Value) { return set_variable(Name.data(), Value.data()); }

		bool delete_variable(const wchar_t* Name);
		inline bool delete_variable(const string& Name) { return delete_variable(Name.data()); }

		string expand_strings(const wchar_t* str);
		inline string expand_strings(const string& str) { return expand_strings(str.data()); }

		string get_pathext();
	}

	// Run-Time Dynamic Linking
	namespace rtdl
	{
		class module: public conditional<module>
		{
		public:
			NONCOPYABLE(module);
			MOVABLE(module);

			explicit module(string name, bool AlternativeLoad = false):
				m_name(std::move(name)),
				m_tried(),
				m_AlternativeLoad(AlternativeLoad)
			{}

			void* GetProcAddress(const char* name) const { return reinterpret_cast<void*>(::GetProcAddress(get_module(), name)); }
			bool operator!() const noexcept { return !get_module(); }

		private:
			HMODULE get_module() const;

			struct module_deleter { void operator()(HMODULE Module) const; };
			using module_ptr = std::unique_ptr<std::remove_pointer_t<HMODULE>, module_deleter>;

			string m_name;
			mutable module_ptr m_module;
			mutable bool m_tried;
			bool m_AlternativeLoad;
		};

		template<typename T>
		class function_pointer: noncopyable
		{
		public:
			function_pointer(const module& Module, const char* Name):
				m_module(Module),
				m_pointer(reinterpret_cast<T>(m_module.GetProcAddress(Name)))
			{}
			operator T() const { return m_pointer; }

		private:
			const module& m_module;
			mutable T m_pointer;
		};
	}

	namespace memory
	{
		namespace global
		{
			namespace detail
			{
				struct deleter { void operator()(HGLOBAL MemoryBlock) const { GlobalFree(MemoryBlock); } };
				struct unlocker { void operator()(const void* MemoryBlock) const { GlobalUnlock(const_cast<HGLOBAL>(MemoryBlock)); } };
			};

			using ptr = std::unique_ptr<std::remove_pointer_t<HGLOBAL>, detail::deleter>;

			inline ptr alloc(UINT Flags, size_t size) { return ptr(GlobalAlloc(Flags, size)); }

			template<class T>
			using lock_t = std::unique_ptr<std::remove_pointer_t<T>, detail::unlocker>;

			template<class T>
			auto lock(HGLOBAL Ptr) { return lock_t<T>(static_cast<T>(GlobalLock(Ptr))); }

			template<class T>
			auto lock(const ptr& Ptr) { return lock<T>(Ptr.get()); }

			template<class T>
			ptr copy(const T& Object)
			{
				static_assert(std::is_pod<T>::value);
				if (auto Memory = alloc(GMEM_MOVEABLE, sizeof(Object)))
				{
					if (const auto Copy = lock<T*>(Memory))
					{
						*Copy = Object;
						return Memory;
					}
				}
				return nullptr;
			}

			inline ptr copy(const wchar_t* Data, size_t Size)
			{
				if (auto Memory = alloc(GMEM_MOVEABLE, (Size + 1) * sizeof(wchar_t)))
				{
					if (const auto Copy = lock<wchar_t*>(Memory))
					{
						*std::copy_n(Data, Size, Copy.get()) = L'\0';
						return Memory;
					}
				}
				return nullptr;
			}

		}

		namespace local
		{
			namespace detail
			{
				struct deleter { void operator()(HLOCAL MemoryBlock) const { LocalFree(MemoryBlock); } };
			};

			template<class T>
			using ptr = std::unique_ptr<T, detail::deleter>;

			template<class T>
			auto alloc(UINT Flags, size_t size) { return ptr<T>(static_cast<T*>(LocalAlloc(Flags, size))); }
		};

		bool is_pointer(const void* Address);
	
		namespace netapi
		{
			namespace detail
			{
				template<class T>
				struct deleter { void operator()(T* Ptr) const { NetApiBufferFree(Ptr); } };
			};

			template<class T>
			using ptr = std::unique_ptr<T, detail::deleter<T>>;
		}
	}

	namespace security
	{
		bool is_admin();
	}

	namespace com
	{
		class co_initialize: noncopyable
		{
		public:
			co_initialize(): m_hr(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)) {}
			~co_initialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
			operator HRESULT() const { return m_hr; }

		private:
			const HRESULT m_hr;
		};

		namespace detail
		{
			template<typename T>
			struct releaser
			{
				void operator()(T* Object) const
				{
					Object->Release();
				}
			};
		}
		template<typename T>
		using ptr = std::unique_ptr<T, detail::releaser<T>>;
	}
}

UUID CreateUuid();
string GuidToStr(const GUID& Guid);
bool StrToGuid(const wchar_t* Value,GUID& Guid);
inline bool StrToGuid(const string& Value, GUID& Guid) { return StrToGuid(Value.data(), Guid); }

#endif // FARWINAPI_HPP_632CB91D_08A9_4793_8FC7_2E38C30CE234
