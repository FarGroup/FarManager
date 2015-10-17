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

#include "farexcpt.hpp"

namespace os
{
	namespace detail 
	{
		struct handle_closer { static bool close(HANDLE Handle); };
		struct find_handle_closer { static bool close(HANDLE Handle); };

		template<class T>
		class handle_t: noncopyable, swapable<handle_t<T>>, public conditional<handle_t<T>>
		{
		public:
			explicit handle_t(HANDLE Handle = nullptr): m_Handle(normalise(Handle)) {}

			handle_t(handle_t&& rhs) noexcept: m_Handle() { *this = std::move(rhs); }

			~handle_t() { close(); }

			MOVE_OPERATOR_BY_SWAP(handle_t);

			void swap(handle_t& rhs) noexcept
			{
				using std::swap;
				swap(m_Handle, rhs.m_Handle);
			}

			bool operator!() const noexcept { return !m_Handle; }

			bool close()
			{
				if (!m_Handle)
					return true;
				bool ret = T::close(m_Handle);
				m_Handle = nullptr;
				return ret;
			}

			void reset(HANDLE Handle = nullptr) { close(); m_Handle = normalise(Handle); }

			HANDLE native_handle() const { return m_Handle; }

			HANDLE release() { const auto Result = m_Handle; m_Handle = nullptr; return Result; }

			bool wait(DWORD Milliseconds = INFINITE) const { return WaitForSingleObject(m_Handle, Milliseconds) == WAIT_OBJECT_0; }

			bool signaled() const { return wait(0); }

		private:
			static HANDLE normalise(HANDLE Handle) { return Handle == INVALID_HANDLE_VALUE? nullptr : Handle; }

			HANDLE m_Handle;
		};
	}

	typedef detail::handle_t<detail::handle_closer> handle;
	typedef detail::handle_t<detail::find_handle_closer> find_handle;

	class HandleWrapper
	{
	public:
		explicit HandleWrapper(HANDLE Handle = nullptr): m_Handle(Handle) {}
		virtual ~HandleWrapper() = 0;

		bool Wait(DWORD Milliseconds = INFINITE) const { return m_Handle.wait(Milliseconds); }
		bool Signaled() const { return m_Handle.signaled(); }
		void Close() { m_Handle.reset(); }

		os::handle m_Handle;
	};

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

	class sid_object: noncopyable
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
	string GetCurrentDirectory();
	DWORD GetTempPath(string &strBuffer);
	DWORD GetModuleFileName(HMODULE hModule, string &strFileName);
	DWORD GetModuleFileNameEx(HANDLE hProcess, HMODULE hModule, string &strFileName);
	DWORD WNetGetConnection(const string& LocalName, string &RemoteName);
	bool GetVolumeInformation(const string& RootPathName, string *pVolumeName, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, string *pFileSystemName);
	bool GetFindDataEx(const string& FileName, FAR_FIND_DATA& FindData, bool ScanSymLink=true);
	bool GetFileSizeEx(HANDLE hFile, UINT64 &Size);
	bool DeleteFile(const string& FileName);
	bool RemoveDirectory(const string& DirName);
	handle CreateFile(const string& Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes=0, HANDLE TemplateFile=nullptr, bool ForceElevation = false);
	bool CopyFileEx(const string& ExistingFileName, const string& NewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
	bool MoveFile(const string& ExistingFileName, const string& NewFileName);
	bool MoveFileEx(const string& ExistingFileName, const string& NewFileName, DWORD dwFlags);
	bool IsDiskInDrive(const string& Root);
	int GetFileTypeByName(const string& Name);
	bool GetDiskSize(const string& Path, unsigned __int64 *TotalSize, unsigned __int64 *TotalFree, unsigned __int64 *UserFree);
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
	find_handle FindFirstStream(const string& FileName, STREAM_INFO_LEVELS InfoLevel, LPVOID lpFindStreamData, DWORD dwFlags = 0);
	bool FindNextStream(const find_handle& hFindStream, LPVOID lpFindStreamData);
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
	bool DetachVirtualDisk(const string& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType);
	string GetPrivateProfileString(const string& AppName, const string& KeyName, const string& Default, const string& FileName);
	DWORD GetAppPathsRedirectionFlag();

	bool CreateSymbolicLinkInternal(const string& Object, const string& Target, DWORD dwFlags);
	bool SetFileEncryptionInternal(const wchar_t* Name, bool Encrypt);
	bool GetFileTimeEx(HANDLE Object, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime, LPFILETIME ChangeTime);
	bool SetFileTimeEx(HANDLE Object, const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime, const FILETIME* ChangeTime);
	bool DetachVirtualDiskInternal(const string& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType);


	namespace fs
	{
		class enum_file: noncopyable, public enumerator<enum_file, FAR_FIND_DATA>
		{
		public:
			enum_file(const string& Object, bool ScanSymLink = true);
			bool get(size_t index, value_type& value);

		private:
			string m_Object;
			find_handle m_Handle;
			bool m_ScanSymLink;
		};

		class enum_name: noncopyable, public enumerator<enum_name, string>
		{
		public:
			enum_name(const string& Object): m_Object(Object) {}
			bool get(size_t index, value_type& value);

		private:
			string m_Object;
			find_handle m_Handle;
		};

		class enum_stream: noncopyable, public enumerator<enum_stream, WIN32_FIND_STREAM_DATA>
		{
		public:
			enum_stream(const string& Object): m_Object(Object) {}
			bool get(size_t index, value_type& value);

		private:

			string m_Object;
			find_handle m_Handle;
		};

		class file: noncopyable, swapable<file>, public conditional<file>
		{
		public:
			file():
				Pointer(),
				NeedSyncPointer(),
				share_mode()
			{
			}

			file(file&& rhs) noexcept:
				Pointer(),
				NeedSyncPointer(),
				share_mode()
			{
				*this = std::move(rhs);
			}

			MOVE_OPERATOR_BY_SWAP(file);

			void swap(file& rhs) noexcept
			{
				using std::swap;
				swap(Handle, rhs.Handle);
				swap(Pointer, rhs.Pointer);
				swap(NeedSyncPointer, rhs.NeedSyncPointer);
				name.swap(rhs.name);
				swap(share_mode, rhs.share_mode);
			}

			bool operator!() const noexcept { return !Handle; }

			// TODO: half of these should be free functions
			bool Open(const string& Object, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes = 0, file* TemplateFile = nullptr, bool ForceElevation = false);
			bool Read(LPVOID Buffer, size_t NumberOfBytesToRead, size_t& NumberOfBytesRead, LPOVERLAPPED Overlapped = nullptr);
			bool Write(LPCVOID Buffer, size_t NumberOfBytesToWrite, size_t& NumberOfBytesWritten, LPOVERLAPPED Overlapped = nullptr);
			bool SetPointer(int64_t DistanceToMove, uint64_t* NewFilePointer, DWORD MoveMethod);
			uint64_t GetPointer(){ return Pointer; }
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
			const string& GetName() const { return name; }
			//HANDLE GetHandle() const { return Handle.native_handle(); }

		private:
			handle Handle;
			uint64_t Pointer;
			bool NeedSyncPointer;
			string name;
			DWORD share_mode;

			inline void SyncPointer();
		};

		class file_walker: public file
		{
		public:
			file_walker();
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

		class file_status
		{
		public:
			file_status();
			file_status(const string& Object);
			file_status(const wchar_t* Object);

			bool check(DWORD Data);

		private:
			DWORD m_Data;
		};

		bool exists(file_status Status);
		bool is_file(file_status Status);
		bool is_directory(file_status Status);

		bool is_not_empty_directory(const string& Object);
	}

	namespace reg
	{
		namespace detail
		{
			inline bool IsStringType(DWORD Type)
			{
				return Type == REG_SZ || Type == REG_EXPAND_SZ || Type == REG_MULTI_SZ;
			}

			struct hkey_deleter
			{
				void operator()(HKEY Key) const
				{
					if (Key)
					{
						RegCloseKey(Key);
					}
				}
			};
		}

		typedef std::unique_ptr<std::remove_pointer<HKEY>::type, detail::hkey_deleter> key;

		key open_key(HKEY RootKey, const wchar_t* SubKey, DWORD SamDesired);

		class value
		{
		public:
			value(): m_Type(REG_NONE), m_Key() {}

			const string& Name() const { return m_Name; }
			DWORD Type() const { return m_Type; }

			string GetString() const;
			unsigned int GetUnsigned() const;
			uint64_t GetUnsigned64() const;

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
		bool GetValue(const key& Key, const wchar_t* Name, uint64_t& Value);

#define IS_SAME(T1, T2) std::is_same<T1, T2>::value
#define CHECK_TYPE(T) static_assert(IS_SAME(T, string) || IS_SAME(T, unsigned int) || IS_SAME(T, uint64_t), "this type is not supported");

		template<class T>
		bool GetValue(const key& Key, const string& Name, T& Value) { CHECK_TYPE(T); return GetValue(Key, Name.data(), Value); }

		template<class T>
		bool GetValue(HKEY RootKey, const wchar_t* SubKey, const wchar_t* Name, T& Value, REGSAM Sam = 0)
		{
			CHECK_TYPE(T);
			const auto Key = open_key(RootKey, SubKey, KEY_QUERY_VALUE | Sam);
			return Key && GetValue(Key, Name, Value);
		}

		template<class T>
		bool GetValue(HKEY RootKey, const string& SubKey, const wchar_t* Name, T& Value, REGSAM Sam = 0) { CHECK_TYPE(T); return GetValue(RootKey, SubKey.data(), Name, Value, Sam); }
		template<class T>
		bool GetValue(HKEY RootKey, const wchar_t* SubKey, const string& Name, T& Value, REGSAM Sam = 0) { CHECK_TYPE(T); return GetValue(RootKey, SubKey, Name.data(), Value, Sam); }
		template<class T>
		bool GetValue(HKEY RootKey, const string& SubKey, const string& Name, T& Value, REGSAM Sam = 0) { CHECK_TYPE(T); return GetValue(RootKey, SubKey.data(), Name.data(), Value, Sam); }

#undef CHECK_TYPE
#undef IS_SAME

		class enum_key: noncopyable, public enumerator<enum_key, string>
		{
		public:
			enum_key(const key& Key): m_KeyRef(Key) {}
			enum_key(HKEY RootKey, const wchar_t* SubKey, REGSAM Sam = 0): m_Key(open_key(RootKey, SubKey, KEY_ENUMERATE_SUB_KEYS | Sam)), m_KeyRef(m_Key) {}
			bool get(size_t Index, value_type& Value) { return m_KeyRef && EnumKey(m_KeyRef, Index, Value); }

		private:
			const key m_Key;
			const key& m_KeyRef;
		};

		class enum_value: noncopyable, public enumerator<enum_value, value>
		{
		public:
			enum_value(const key& Key): m_KeyRef(Key) {}
			enum_value(HKEY RootKey, const wchar_t* SubKey, REGSAM Sam = 0): m_Key(open_key(RootKey, SubKey, KEY_QUERY_VALUE | Sam)), m_KeyRef(m_Key) {}
			bool get(size_t Index, value_type& Value) { return m_KeyRef && EnumValue(m_KeyRef, Index, Value); }

		private:
			const key m_Key;
			const key& m_KeyRef;
		};
	}

	// enumerator for string1\0string2\0string3\0...stringN\0\0
	template<class Provider, class ResultType = const wchar_t*>
	class enum_strings_t: noncopyable, public enumerator<enum_strings_t<Provider, ResultType>, range<ResultType>>
	{
	public:
		enum_strings_t() {}
		enum_strings_t(Provider Data): m_Provider(Data) {}
		bool get(size_t index, typename enum_strings_t<Provider, ResultType>::value_type& value)
		{
			const auto Begin = index ? value.data() + value.size() + 1 : m_Provider;
			auto End = Begin;
			while (*End) ++End;
			value = typename enum_strings_t<Provider, ResultType>::value_type(Begin, End);
			return !value.empty();
		}

	private:
		const Provider m_Provider;
	};

	typedef enum_strings_t<const wchar_t*> enum_strings;

	namespace env
	{
		namespace detail
		{
			class provider
			{
			public:
				operator const wchar_t*() const { return m_Environment; }
				operator wchar_t*() { return m_Environment; }

			protected:
				wchar_t* m_Environment;
			};
		}

		namespace provider
		{
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

		std::pair<string, string> split(const wchar_t* Line);

		typedef enum_strings_t<provider::strings> enum_strings;

		bool get_variable(const wchar_t* Name, string& strBuffer);
		inline bool get_variable(const string& Name, string& strBuffer) { return get_variable(Name.data(), strBuffer); }
		template<class T>
		inline string get_variable(const T& Name) { string Result; get_variable(Name, Result); return Result; }

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
		class module: noncopyable, public conditional<module>
		{
		public:
			module(const wchar_t* name, bool AlternativeLoad = false):
				m_name(name),
				m_module(),
				m_loaded(),
				m_tried(),
				m_AlternativeLoad(AlternativeLoad)
			{}
			~module();

			FARPROC GetProcAddress(const char* name) const { return ::GetProcAddress(get_module(), name); }
			bool operator!() const noexcept { return !get_module(); }

		private:
			HMODULE get_module() const;

			const wchar_t* m_name;
			mutable HMODULE m_module;
			mutable bool m_loaded;
			mutable bool m_tried;
			const bool m_AlternativeLoad;
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
				struct unlocker { void operator()(const void* MemoryBlock) { GlobalUnlock(const_cast<HGLOBAL>(MemoryBlock)); } };
			};

			typedef std::unique_ptr<std::remove_pointer<HGLOBAL>::type, detail::deleter> ptr;

			inline ptr alloc(UINT Flags, size_t size) { return ptr(GlobalAlloc(Flags, size)); }

			template<class T>
			struct lock_t { typedef std::unique_ptr<typename std::remove_pointer<T>::type, detail::unlocker> ptr; };

			template<class T>
			typename lock_t<T>::ptr lock(HGLOBAL Ptr) { return typename lock_t<T>::ptr(static_cast<T>(GlobalLock(Ptr))); }

			template<class T>
			typename lock_t<T>::ptr lock(const ptr& Ptr) { return lock<T>(Ptr.get()); }
		}

		namespace local
		{
			namespace detail
			{
				struct deleter { void operator()(HLOCAL MemoryBlock) const { LocalFree(MemoryBlock); } };
			};

			template<class T>
			struct ptr_t
			{
				typedef std::unique_ptr<T, detail::deleter> type;
			};

			template<class T>
			inline typename ptr_t<T>::type ptr(T* Pointer) { return typename ptr_t<T>::type(Pointer); }

			template<class T>
			inline typename ptr_t<T>::type alloc(UINT Flags, size_t size) { return ptr(static_cast<T*>(LocalAlloc(Flags, size))); }
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
			struct ptr_t { typedef std::unique_ptr<T, detail::deleter<T>> type; };

			template<class T>
			typename ptr_t<T>::type ptr(T* RawPtr) { return typename ptr_t<T>::type(RawPtr); }
		}
	}

	namespace security
	{
		bool is_admin();
	}

	class co_initialize: noncopyable
	{
	public:
		co_initialize(): m_hr(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)) {}
		~co_initialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
		operator HRESULT() const { return m_hr; }

	private:
		const HRESULT m_hr;
	};

	typedef std::bitset<26> drives_set;

	class hp_clock
	{
	public:
		hp_clock(): m_StartTime(call_function(QueryPerformanceCounter)) {}

		enum factor
		{
			seconds = 1,
			milliseconds = 1000,
			microseconds = 1000000,
		};

		uint64_t query(factor Factor) const
		{
			const auto Diff = call_function(QueryPerformanceCounter) - m_StartTime;
			const auto Frequency = get_frequency();
			const auto Whole = Diff / Frequency;
			const auto Fraction = Diff % Frequency;
			return Whole * Factor + (Fraction * Factor) / Frequency;
		}

	private:
		template<class T>
		static uint64_t call_function(const T& Getter)
		{
			LARGE_INTEGER Value;
			Getter(&Value);
			return Value.QuadPart;
		};

		static uint64_t get_frequency()
		{
			static const auto Frequency = call_function(QueryPerformanceFrequency);
			return Frequency;
		}

		uint64_t m_StartTime;
	};
}
