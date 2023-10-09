// validator: no-self-include
/*
vc_crt_fix_impl.cpp

Workaround for Visual C++ CRT incompatibility with old Windows versions
*/
/*
Copyright © 2011 Far Group
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

#ifndef FAR_USE_INTERNALS
#define FAR_USE_INTERNALS
#endif // END FAR_USE_INTERNALS
#ifdef FAR_USE_INTERNALS
#include "disable_warnings_in_std_begin.hpp"
//----------------------------------------------------------------------------
#endif // END FAR_USE_INTERNALS
#include <memory>
#include <utility>

#include <windows.h>
#ifdef FAR_USE_INTERNALS
//----------------------------------------------------------------------------
#include "disable_warnings_in_std_end.hpp"
#endif // END FAR_USE_INTERNALS

#ifdef __clang__
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#endif


template<typename T>
static T GetFunctionPointer(const wchar_t* ModuleName, const char* FunctionName, T Replacement)
{
	const auto Module = GetModuleHandleW(ModuleName);
	const auto Address = Module? GetProcAddress(Module, FunctionName) : nullptr;
	return Address? reinterpret_cast<T>(reinterpret_cast<void*>(Address)) : Replacement;
}

#define WRAPPER(name) Wrapper_ ## name
#define CREATE_AND_RETURN_IMPL(ModuleName, FunctionName, ImplementationName, ...) \
	static const auto FunctionPointer = GetFunctionPointer(ModuleName, FunctionName, &implementation::ImplementationName); \
	return FunctionPointer(__VA_ARGS__)

#define CREATE_AND_RETURN_NAMED(ModuleName, FunctionName, ...) CREATE_AND_RETURN_IMPL(ModuleName, #FunctionName, FunctionName, __VA_ARGS__)
#define CREATE_AND_RETURN(ModuleName, ...)                     CREATE_AND_RETURN_IMPL(ModuleName, __func__ + sizeof("Wrapper_") - 1, impl, __VA_ARGS__)

namespace modules
{
	static const wchar_t kernel32[] = L"kernel32";
	static const wchar_t ntdll[] = L"ntdll";
}

static void* XorPointer(void* Ptr)
{
	static const auto Cookie = []
	{
		static void* Ptr;
		auto Result = reinterpret_cast<uintptr_t>(&Ptr);

		FILETIME CreationTime, NotUsed;
		if (GetThreadTimes(GetCurrentThread(), &CreationTime, &NotUsed, &NotUsed, &NotUsed))
		{
			Result ^= CreationTime.dwLowDateTime;
			Result ^= CreationTime.dwHighDateTime;
		}
		return Result;
	}();
	return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(Ptr) ^ Cookie);
}

// EncodePointer (VC2010)
extern "C" PVOID WINAPI WRAPPER(EncodePointer)(PVOID Ptr)
{
	struct implementation
	{
		static PVOID WINAPI impl(PVOID Ptr)
		{
			return XorPointer(Ptr);
		}
	};

	CREATE_AND_RETURN(modules::kernel32, Ptr);
}

// DecodePointer(VC2010)
extern "C" PVOID WINAPI WRAPPER(DecodePointer)(PVOID Ptr)
{
	struct implementation
	{
		static PVOID WINAPI impl(PVOID Ptr)
		{
			return XorPointer(Ptr);
		}
	};

	CREATE_AND_RETURN(modules::kernel32, Ptr);
}

// GetModuleHandleExW (VC2012)
extern "C" BOOL WINAPI WRAPPER(GetModuleHandleExW)(DWORD Flags, LPCWSTR ModuleName, HMODULE *Module)
{
	struct implementation
	{
		static BOOL WINAPI impl(DWORD Flags, LPCWSTR ModuleName, HMODULE *Module)
		{
			if (Flags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS)
			{
				MEMORY_BASIC_INFORMATION mbi;
				if (!VirtualQuery(ModuleName, &mbi, sizeof(mbi)))
					return FALSE;

				const auto ModuleValue = static_cast<HMODULE>(mbi.AllocationBase);

				if (!(Flags & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT))
				{
					wchar_t Buffer[MAX_PATH];
					if (!GetModuleFileNameW(ModuleValue, Buffer, ARRAYSIZE(Buffer)))
						return FALSE;
					// It's the same so not really necessary, but analysers report handle leak otherwise.
					*Module = LoadLibraryW(Buffer);
				}
				else
				{
					*Module = ModuleValue;
				}
				return TRUE;
			}

			// GET_MODULE_HANDLE_EX_FLAG_PIN not implemented

			if (const auto ModuleValue = (Flags & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT? GetModuleHandleW : LoadLibraryW)(ModuleName))
			{
				*Module = ModuleValue;
				return TRUE;
			}
			return FALSE;
		}
	};

	CREATE_AND_RETURN(modules::kernel32, Flags, ModuleName, Module);
}

namespace slist
{
	namespace implementation
	{
#ifdef _WIN64
		// These stubs are here only to unify compilation, they shall never be needed on x64.
		static void WINAPI InitializeSListHead(PSLIST_HEADER ListHead) {}
		static PSLIST_ENTRY WINAPI InterlockedFlushSList(PSLIST_HEADER ListHead) { return nullptr; }
		static PSLIST_ENTRY WINAPI InterlockedPopEntrySList(PSLIST_HEADER ListHead) { return nullptr; }
		static PSLIST_ENTRY WINAPI InterlockedPushEntrySList(PSLIST_HEADER ListHead, PSLIST_ENTRY ListEntry) { return nullptr; }
		static PSLIST_ENTRY WINAPI InterlockedPushListSListEx(PSLIST_HEADER ListHead, PSLIST_ENTRY List, PSLIST_ENTRY ListEnd, ULONG Count) { return nullptr; }
		static PSLIST_ENTRY WINAPI RtlFirstEntrySList(PSLIST_HEADER ListHead) { return nullptr; }
		static USHORT WINAPI QueryDepthSList(PSLIST_HEADER ListHead) { return 0; }
#else
		class critical_section
		{
		public:
			critical_section() { InitializeCriticalSection(&m_Lock); }
			~critical_section() { DeleteCriticalSection(&m_Lock); }

			critical_section(const critical_section&) = delete;
			critical_section(critical_section&&) = default;

			critical_section& operator=(const critical_section&) = delete;
			critical_section& operator=(critical_section&&) = default;

			void lock() { EnterCriticalSection(&m_Lock); }
			void unlock() { LeaveCriticalSection(&m_Lock); }

		private:
			CRITICAL_SECTION m_Lock;
		};

		struct service_entry: SLIST_ENTRY, critical_section
		{
			// InitializeSListHead might be called during runtime initialisation
			// when operator new might not be ready yet (especially in presence of leak detectors)

			void* operator new(size_t Size)
			{
				return malloc(Size);
			}

			void operator delete(void* Ptr)
			{
				free(Ptr);
			}

			service_entry* ServiceNext{};
		};

		class slist_lock
		{
		public:
			explicit slist_lock(PSLIST_HEADER ListHead):
				m_Entry(static_cast<service_entry&>(*ListHead->Next.Next))
			{
				m_Entry.lock();
			}

			~slist_lock()
			{
				m_Entry.unlock();
			}

			slist_lock(const slist_lock&) = delete;
			slist_lock& operator=(const slist_lock&) = delete;

		private:
			service_entry& m_Entry;
		};

		class service_deleter
		{
		public:
			~service_deleter()
			{
				while (m_Data.ServiceNext)
				{
					delete std::exchange(m_Data.ServiceNext, m_Data.ServiceNext->ServiceNext);
				}
			}

			void add(service_entry* Entry)
			{
				m_Data.lock();
				Entry->ServiceNext = m_Data.ServiceNext;
				m_Data.ServiceNext = Entry;
				m_Data.unlock();
			}

		private:
			service_entry m_Data{};
		};

		static SLIST_ENTRY*& top(PSLIST_HEADER ListHead)
		{
			return ListHead->Next.Next->Next;
		}

		static void WINAPI InitializeSListHead(PSLIST_HEADER ListHead)
		{
			*ListHead = {};

			const auto Entry = new service_entry();
			ListHead->Next.Next = Entry;

			static service_deleter Deleter;
			Deleter.add(Entry);
		}

		static PSLIST_ENTRY WINAPI InterlockedFlushSList(PSLIST_HEADER ListHead)
		{
			slist_lock Lock(ListHead);

			ListHead->Depth = 0;
			return std::exchange(top(ListHead), nullptr);
		}

		static PSLIST_ENTRY WINAPI InterlockedPopEntrySList(PSLIST_HEADER ListHead)
		{
			slist_lock Lock(ListHead);

			auto& Top = top(ListHead);
			if (!Top)
				return nullptr;

			--ListHead->Depth;
			return std::exchange(Top, Top->Next);
		}

		static PSLIST_ENTRY WINAPI InterlockedPushEntrySList(PSLIST_HEADER ListHead, PSLIST_ENTRY ListEntry)
		{
			slist_lock Lock(ListHead);

			auto& Top = top(ListHead);

			++ListHead->Depth;
			ListEntry->Next = Top;
			return std::exchange(Top, ListEntry);
		}

		static PSLIST_ENTRY WINAPI InterlockedPushListSListEx(PSLIST_HEADER ListHead, PSLIST_ENTRY List, PSLIST_ENTRY ListEnd, ULONG Count)
		{
			slist_lock Lock(ListHead);

			auto& Top = top(ListHead);

			ListHead->Depth += static_cast<WORD>(Count);
			ListEnd->Next = Top;
			return std::exchange(Top, List);
		}

		static PSLIST_ENTRY WINAPI RtlFirstEntrySList(PSLIST_HEADER ListHead)
		{
			slist_lock Lock(ListHead);

			return top(ListHead);
		}

		static USHORT WINAPI QueryDepthSList(PSLIST_HEADER ListHead)
		{
			slist_lock Lock(ListHead);

			return ListHead->Depth;
		}
#endif
	}
}

// VC2015
extern "C" void WINAPI WRAPPER(InitializeSListHead)(PSLIST_HEADER ListHead)
{
	using namespace slist;
	CREATE_AND_RETURN_NAMED(modules::kernel32, InitializeSListHead, ListHead);
}

// VC2015
extern "C" PSLIST_ENTRY WINAPI WRAPPER(InterlockedFlushSList)(PSLIST_HEADER ListHead)
{
	using namespace slist;
	CREATE_AND_RETURN_NAMED(modules::kernel32, InterlockedFlushSList, ListHead);
}

// VC2015
extern "C" PSLIST_ENTRY WINAPI WRAPPER(InterlockedPopEntrySList)(PSLIST_HEADER ListHead)
{
	using namespace slist;
	CREATE_AND_RETURN_NAMED(modules::kernel32, InterlockedPopEntrySList, ListHead);
}

// VC2015
extern "C" PSLIST_ENTRY WINAPI WRAPPER(InterlockedPushEntrySList)(PSLIST_HEADER ListHead, PSLIST_ENTRY ListEntry)
{
	using namespace slist;
	CREATE_AND_RETURN_NAMED(modules::kernel32, InterlockedPushEntrySList, ListHead, ListEntry);
}

// VC2015
extern "C" PSLIST_ENTRY WINAPI WRAPPER(InterlockedPushListSListEx)(PSLIST_HEADER ListHead, PSLIST_ENTRY List, PSLIST_ENTRY ListEnd, ULONG Count)
{
	using namespace slist;
	CREATE_AND_RETURN_NAMED(modules::kernel32, InterlockedPushListSListEx, ListHead, List, ListEnd, Count);
}

// VC2015
extern "C" PSLIST_ENTRY WINAPI WRAPPER(RtlFirstEntrySList)(PSLIST_HEADER ListHead)
{
	using namespace slist;
	CREATE_AND_RETURN_NAMED(modules::ntdll, RtlFirstEntrySList, ListHead);
}

// VC2015
extern "C" USHORT WINAPI WRAPPER(QueryDepthSList)(PSLIST_HEADER ListHead)
{
	using namespace slist;
	CREATE_AND_RETURN_NAMED(modules::kernel32, QueryDepthSList, ListHead);
}

// VC2017
extern "C" BOOL WINAPI WRAPPER(GetNumaHighestNodeNumber)(PULONG HighestNodeNumber)
{
	struct implementation
	{
		static BOOL WINAPI impl(PULONG HighestNodeNumber)
		{
			*HighestNodeNumber = 0;
			return TRUE;
		}
	};

	CREATE_AND_RETURN(modules::kernel32, HighestNodeNumber);
}

// VC2017
extern "C" BOOL WINAPI WRAPPER(GetLogicalProcessorInformation)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer, PDWORD ReturnLength)
{
	struct implementation
	{
		static BOOL WINAPI impl(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer, PDWORD ReturnLength)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return FALSE;
		}
	};

	CREATE_AND_RETURN(modules::kernel32, Buffer, ReturnLength);
}

// VC2019
extern "C" BOOL WINAPI WRAPPER(SetThreadStackGuarantee)(PULONG StackSizeInBytes)
{
	struct implementation
	{
		static BOOL WINAPI impl(PULONG StackSizeInBytes)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return FALSE;
		}
	};

	CREATE_AND_RETURN(modules::kernel32, StackSizeInBytes);
}

// VC2019
extern "C" BOOL WINAPI WRAPPER(InitializeCriticalSectionEx)(LPCRITICAL_SECTION CriticalSection, DWORD SpinCount, DWORD Flags)
{
	struct implementation
	{
		static int WINAPI impl(LPCRITICAL_SECTION CriticalSection, DWORD SpinCount, DWORD Flags)
		{
			InitializeCriticalSection(CriticalSection);
			CriticalSection->SpinCount = Flags | SpinCount;
			return TRUE;
		}
	};

	CREATE_AND_RETURN(modules::kernel32, CriticalSection, SpinCount, Flags);
}

// VC2019
extern "C" int WINAPI WRAPPER(CompareStringEx)(LPCWSTR LocaleName, DWORD CmpFlags, LPCWCH String1, int Count1, LPCWCH String2, int Count2, LPNLSVERSIONINFO VersionInformation, LPVOID Reserved, LPARAM Param)
{
	struct implementation
	{
		static int WINAPI impl(LPCWSTR LocaleName, DWORD CmpFlags, LPCWCH String1, int Count1, LPCWCH String2, int Count2, LPNLSVERSIONINFO VersionInformation, LPVOID Reserved, LPARAM Param)
		{
			return CompareStringW(LOCALE_USER_DEFAULT, CmpFlags, String1, Count1, String2, Count2);
		}
	};

	CREATE_AND_RETURN(modules::kernel32, LocaleName, CmpFlags, String1, Count1, String2, Count2, VersionInformation, Reserved, Param);
}

// VC2019
extern "C" int WINAPI WRAPPER(LCMapStringEx)(LPCWSTR LocaleName, DWORD MapFlags, LPCWSTR SrcStr, int SrcCount, LPWSTR DestStr, int DestCount, LPNLSVERSIONINFO VersionInformation, LPVOID Reserved, LPARAM SortHandle)
{
	struct implementation
	{
		static int WINAPI impl(LPCWSTR LocaleName, DWORD MapFlags, LPCWSTR SrcStr, int SrcCount, LPWSTR DestStr, int DestCount, LPNLSVERSIONINFO VersionInformation, LPVOID Reserved, LPARAM SortHandle)
		{
			return LCMapStringW(LOCALE_USER_DEFAULT, MapFlags, SrcStr, SrcCount, DestStr, DestCount);
		}
	};

	CREATE_AND_RETURN(modules::kernel32, LocaleName, MapFlags, SrcStr, SrcCount, DestStr, DestCount, VersionInformation, Reserved, SortHandle);
}

// VC2022
extern "C" BOOL WINAPI WRAPPER(SleepConditionVariableSRW)(PCONDITION_VARIABLE ConditionVariable, PSRWLOCK SRWLock, DWORD Milliseconds, ULONG Flags)
{
	struct implementation
	{
		static BOOL WINAPI impl(PCONDITION_VARIABLE ConditionVariable, PSRWLOCK SRWLock, DWORD Milliseconds, ULONG Flags)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return FALSE;
		}
	};

	CREATE_AND_RETURN(modules::kernel32, ConditionVariable, SRWLock, Milliseconds, Flags);
}

// VC2022
extern "C" void WINAPI WRAPPER(WakeAllConditionVariable)(PCONDITION_VARIABLE ConditionVariable)
{
	struct implementation
	{
		static void WINAPI impl(PCONDITION_VARIABLE ConditionVariable)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		}
	};

	CREATE_AND_RETURN(modules::kernel32, ConditionVariable);
}

// VC2022
extern "C" void WINAPI WRAPPER(AcquireSRWLockExclusive)(PSRWLOCK SRWLock)
{
	struct implementation
	{
		static void WINAPI impl(PSRWLOCK SRWLock)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		}
	};

	CREATE_AND_RETURN(modules::kernel32, SRWLock);
}

// VC2022
extern "C" void WINAPI WRAPPER(ReleaseSRWLockExclusive)(PSRWLOCK SRWLock)
{
	struct implementation
	{
		static void WINAPI impl(PSRWLOCK SRWLock)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		}
	};

	CREATE_AND_RETURN(modules::kernel32, SRWLock);
}

#undef CREATE_AND_RETURN
#undef CREATE_AND_RETURN_NAMED
#undef CREATE_AND_RETURN_IMPL
#undef WRAPPER

// disable VS2015 telemetry
extern "C"
{
	void __vcrt_initialize_telemetry_provider() {}
	void __telemetry_main_invoke_trigger() {}
	void __telemetry_main_return_trigger() {}
	void __vcrt_uninitialize_telemetry_provider() {}
}
