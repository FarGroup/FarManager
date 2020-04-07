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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS' AND ANY EXPRESS OR
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

#include <utility>

#include <windows.h>

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

#define CREATE_FUNCTION_POINTER(ModuleName, FunctionName)\
static const auto Function = GetFunctionPointer(ModuleName, #FunctionName, &implementation::FunctionName)

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
extern "C" PVOID WINAPI EncodePointerWrapper(PVOID Ptr)
{
	struct implementation
	{
		static PVOID WINAPI EncodePointer(PVOID Ptr)
		{
			return XorPointer(Ptr);
		}
	};

	CREATE_FUNCTION_POINTER(modules::kernel32, EncodePointer);
	return Function(Ptr);
}

// DecodePointer(VC2010)
extern "C" PVOID WINAPI DecodePointerWrapper(PVOID Ptr)
{
	struct implementation
	{
		static PVOID WINAPI DecodePointer(PVOID Ptr)
		{
			return XorPointer(Ptr);
		}
	};

	CREATE_FUNCTION_POINTER(modules::kernel32, DecodePointer);
	return Function(Ptr);
}

// GetModuleHandleExW (VC2012)
extern "C" BOOL WINAPI GetModuleHandleExWWrapper(DWORD Flags, LPCWSTR ModuleName, HMODULE *Module)
{
	struct implementation
	{
		static BOOL WINAPI GetModuleHandleExW(DWORD Flags, LPCWSTR ModuleName, HMODULE *Module)
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

	CREATE_FUNCTION_POINTER(modules::kernel32, GetModuleHandleExW);
	return Function(Flags, ModuleName, Module);
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

// InitializeSListHead (VC2015)
extern "C" void WINAPI InitializeSListHeadWrapper(PSLIST_HEADER ListHead)
{
	using namespace slist;
	CREATE_FUNCTION_POINTER(modules::kernel32, InitializeSListHead);
	return Function(ListHead);
}

// InterlockedFlushSList (VC2015)
extern "C" PSLIST_ENTRY WINAPI InterlockedFlushSListWrapper(PSLIST_HEADER ListHead)
{
	using namespace slist;
	CREATE_FUNCTION_POINTER(modules::kernel32, InterlockedFlushSList);
	return Function(ListHead);
}

// InterlockedPopEntrySList (VC2015)
extern "C" PSLIST_ENTRY WINAPI InterlockedPopEntrySListWrapper(PSLIST_HEADER ListHead)
{
	using namespace slist;
	CREATE_FUNCTION_POINTER(modules::kernel32, InterlockedPopEntrySList);
	return Function(ListHead);
}

// InterlockedPushEntrySList (VC2015)
extern "C" PSLIST_ENTRY WINAPI InterlockedPushEntrySListWrapper(PSLIST_HEADER ListHead, PSLIST_ENTRY ListEntry)
{
	using namespace slist;
	CREATE_FUNCTION_POINTER(modules::kernel32, InterlockedPushEntrySList);
	return Function(ListHead, ListEntry);
}

// InterlockedPushListSListEx (VC2015)
extern "C" PSLIST_ENTRY WINAPI InterlockedPushListSListExWrapper(PSLIST_HEADER ListHead, PSLIST_ENTRY List, PSLIST_ENTRY ListEnd, ULONG Count)
{
	using namespace slist;
	CREATE_FUNCTION_POINTER(modules::kernel32, InterlockedPushListSListEx);
	return Function(ListHead, List, ListEnd, Count);
}

// RtlFirstEntrySList (VC2015)
extern "C" PSLIST_ENTRY WINAPI RtlFirstEntrySListWrapper(PSLIST_HEADER ListHead)
{
	using namespace slist;
	CREATE_FUNCTION_POINTER(modules::ntdll, RtlFirstEntrySList);
	return Function(ListHead);
}

// QueryDepthSList (VC2015)
extern "C" USHORT WINAPI QueryDepthSListWrapper(PSLIST_HEADER ListHead)
{
	using namespace slist;
	CREATE_FUNCTION_POINTER(modules::kernel32, QueryDepthSList);
	return Function(ListHead);
}

// GetNumaHighestNodeNumber (VC2017)
extern "C" BOOL WINAPI GetNumaHighestNodeNumberWrapper(PULONG HighestNodeNumber)
{
	struct implementation
	{
		static BOOL WINAPI GetNumaHighestNodeNumber(PULONG HighestNodeNumber)
		{
			*HighestNodeNumber = 0;
			return TRUE;
		}
	};

	CREATE_FUNCTION_POINTER(modules::kernel32, GetNumaHighestNodeNumber);
	return Function(HighestNodeNumber);
}

// GetLogicalProcessorInformation (VC2017)
extern "C" BOOL WINAPI GetLogicalProcessorInformationWrapper(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer, PDWORD ReturnLength)
{
	struct implementation
	{
		static BOOL WINAPI GetLogicalProcessorInformation(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer, PDWORD ReturnLength)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return FALSE;
		}
	};

	CREATE_FUNCTION_POINTER(modules::kernel32, GetLogicalProcessorInformation);
	return Function(Buffer, ReturnLength);
}

// SetThreadStackGuarantee (VC2019)
extern "C" BOOL WINAPI SetThreadStackGuaranteeWrapper(PULONG StackSizeInBytes)
{
	struct implementation
	{
		static BOOL WINAPI SetThreadStackGuarantee(PULONG StackSizeInBytes)
		{
			SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
			return FALSE;
		}
	};

	CREATE_FUNCTION_POINTER(modules::kernel32, SetThreadStackGuarantee);
	return Function(StackSizeInBytes);
}


// disable VS2015 telemetry
extern "C"
{
	void __vcrt_initialize_telemetry_provider() {}
	void __telemetry_main_invoke_trigger() {}
	void __telemetry_main_return_trigger() {}
	void __vcrt_uninitialize_telemetry_provider() {}
}
