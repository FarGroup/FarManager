/*
farrtl.cpp

Переопределение различных CRT функций
*/

#include "headers.hpp"
#pragma hdrstop

#include "console.hpp"
#include "colormix.hpp"
#include "imports.hpp"
#include "synchro.hpp"

#ifdef _MSC_VER
#pragma intrinsic (memcpy)
#endif

#ifdef MEMCHECK
#undef xf_malloc
#undef xf_realloc
#undef xf_realloc_nomove
#undef DuplicateString
#undef new
#endif

static bool InsufficientMemoryHandler()
{
	if (!Global)
		return false;
	Global->Console->SetTextAttributes(Colors::ConsoleColorToFarColor(FOREGROUND_RED|FOREGROUND_INTENSITY));
	COORD OldPos,Pos={};
	Global->Console->GetCursorPosition(OldPos);
	Global->Console->SetCursorPosition(Pos);
	static WCHAR ErrorMessage[] = L"Not enough memory is available to complete this operation.\nPress Enter to retry or Esc to continue...";
	Global->Console->Write(ErrorMessage, ARRAYSIZE(ErrorMessage)-1);
	Global->Console->Commit();
	Global->Console->SetCursorPosition(OldPos);
	INPUT_RECORD ir={};
	size_t Read;
	do
	{
		Global->Console->ReadInput(&ir, 1, Read);
	}
	while(!(ir.EventType == KEY_EVENT && !ir.Event.KeyEvent.bKeyDown && (ir.Event.KeyEvent.wVirtualKeyCode == VK_RETURN || ir.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)));
	return ir.Event.KeyEvent.wVirtualKeyCode == VK_RETURN;
}

static void* ReleaseAllocator(size_t size)
{
	void* newBlock;
	do newBlock = malloc(size);
	while (!newBlock && InsufficientMemoryHandler());
	return newBlock;
}

static void ReleaseDeallocator(void* block)
{
	return free(block);
}

static void* ReleaseReallocator(void* block, size_t size)
{
	void* newBlock;
	do newBlock = realloc(block, size);
	while (!newBlock && InsufficientMemoryHandler());
	return newBlock;
}

static void* ReleaseExpander(void* block, size_t size)
{
#ifdef _MSC_VER
	return _expand(block, size);
#else
	return nullptr;
#endif
}

#ifndef MEMCHECK

void* xf_malloc(size_t size)
{
	return ReleaseAllocator(size);
}

void xf_free(void* block)
{
	return ReleaseDeallocator(block);
}

void* xf_realloc(void* block, size_t size)
{
	return ReleaseReallocator(block, size);
}

void* xf_realloc_nomove(void * block, size_t size)
{
	if (!block)
	{
		return xf_malloc(size);
	}
	else if (ReleaseExpander(block, size))
	{
		return block;
	}
	else
	{
		xf_free(block);
		return xf_malloc(size);
	}
}

void* operator new(size_t size) throw()
{
	return ReleaseAllocator(size);
}

void* operator new[](size_t size) throw()
{
	return ReleaseAllocator(size);
}

void operator delete(void* block)
{
	return ReleaseDeallocator(block);
}

void operator delete[](void* block)
{
	return ReleaseDeallocator(block);
}

char* DuplicateString(const char * string)
{
	return string? strcpy(new char[strlen(string) + 1], string) : nullptr;
}

wchar_t* DuplicateString(const wchar_t * string)
{
	return string? wcscpy(new wchar_t[wcslen(string) + 1], string) : nullptr;
}

#else
namespace memcheck
{
static intptr_t CallNewDeleteVector = 0;
static intptr_t CallNewDeleteScalar = 0;
static intptr_t CallMallocFree = 0;
static size_t AllocatedMemoryBlocks = 0;
static size_t AllocatedMemorySize = 0;
static size_t TotalAllocationCalls = 0;
static bool MonitoringEnabled = true;

enum ALLOCATION_TYPE
{
	AT_RAW    = 0xa7000ea8,
	AT_SCALAR = 0xa75ca1ae,
	AT_VECTOR = 0xa77ec10e,
};

static CRITICAL_SECTION CS;

struct MEMINFO
{
	union
	{
		struct
		{
			ALLOCATION_TYPE AllocationType;
			int Line;
			const char* File;
			const char* Function;
			size_t Size;
			MEMINFO* prev;
			MEMINFO* next;
		};
		char c[MEMORY_ALLOCATION_ALIGNMENT*4];
	};
};

static MEMINFO FirstMemBlock = {};
static MEMINFO* LastMemBlock = &FirstMemBlock;

static_assert(sizeof(MEMINFO) == MEMORY_ALLOCATION_ALIGNMENT*4, "MEMINFO not aligned");
inline MEMINFO* ToReal(void* address) { return static_cast<MEMINFO*>(address) - 1; }
inline void* ToUser(MEMINFO* address) { return address + 1; }

static void CheckChain()
{
#if 0
	auto p = &FirstMemBlock;

	while(p->next)
		p = p->next;
	assert(p==LastMemBlock);

	while(p->prev)
		p = p->prev;
	assert(p==&FirstMemBlock);
#endif
}

static inline void updateCallCount(ALLOCATION_TYPE type, bool increment)
{
	int op = increment? 1 : -1;
	switch(type)
	{
	case AT_RAW:    CallMallocFree += op;      break;
	case AT_SCALAR: CallNewDeleteScalar += op; break;
	case AT_VECTOR: CallNewDeleteVector += op; break;
	}
}

static void RegisterBlock(MEMINFO *block)
{
	if (!MonitoringEnabled)
		return;

	if (!AllocatedMemoryBlocks)
		InitializeCriticalSection(&CS);
	EnterCriticalSection(&CS);

	block->prev = LastMemBlock;
	block->next = nullptr;

	LastMemBlock->next = block;
	LastMemBlock = block;

	CheckChain();

	updateCallCount(block->AllocationType, true);
	++AllocatedMemoryBlocks;
	++TotalAllocationCalls;
	AllocatedMemorySize+=block->Size;

	LeaveCriticalSection(&CS);
}

static void UnregisterBlock(MEMINFO *block)
{
	if (!MonitoringEnabled)
		return;

	EnterCriticalSection(&CS);

	if (block->prev)
		block->prev->next = block->next;
	if (block->next)
		block->next->prev = block->prev;
	if(block == LastMemBlock)
		LastMemBlock = LastMemBlock->prev;

	CheckChain();

	updateCallCount(block->AllocationType, false);
	--AllocatedMemoryBlocks;
	AllocatedMemorySize-=block->Size;

	LeaveCriticalSection(&CS);

	if (!AllocatedMemoryBlocks)
		DeleteCriticalSection(&CS);
}

static void* DebugAllocator(size_t size, ALLOCATION_TYPE type,const char* Function,  const char* File, int Line)
{
	size_t realSize = size + sizeof(MEMINFO);
	MEMINFO* Info = static_cast<MEMINFO*>(ReleaseAllocator(realSize));
	Info->AllocationType = type;
	Info->Size = realSize;
	Info->Function = Function;
	Info->File = File;
	Info->Line = Line;
	RegisterBlock(Info);
	return ToUser(Info);
}

static void DebugDeallocator(void* block, ALLOCATION_TYPE type)
{
	void* realBlock = block? ToReal(block) : nullptr;
	if (realBlock)
	{
		MEMINFO* Info = static_cast<MEMINFO*>(realBlock);
		assert(Info->AllocationType == type);
		UnregisterBlock(Info);
	}
	ReleaseDeallocator(realBlock);
}

static void* DebugReallocator(void* block, size_t size, const char* Function, const char* File, int Line)
{
	if(!block)
		return DebugAllocator(size, AT_RAW, Function, File, Line);

	MEMINFO* Info = ToReal(block);
	assert(Info->AllocationType == AT_RAW);
	UnregisterBlock(Info);
	size_t realSize = size + sizeof(MEMINFO);

	Info = static_cast<MEMINFO*>(ReleaseReallocator(Info, realSize));

	Info->AllocationType = AT_RAW;
	Info->Size = realSize;
	RegisterBlock(Info);
	return ToUser(Info);
}

static void* DebugExpander(void* block, size_t size)
{
	MEMINFO* Info = ToReal(block);
	assert(Info->AllocationType == AT_RAW);
	size_t realSize = size + sizeof(MEMINFO);

	// _expand() calls HeapReAlloc which can change the status code, it's bad for us
	NTSTATUS status = Global->ifn->RtlGetLastNtStatus();
	Info = static_cast<MEMINFO*>(ReleaseExpander(Info, realSize));
	//RtlNtStatusToDosError also remembers the status code value in the TEB:
	Global->ifn->RtlNtStatusToDosError(status);

	if(Info)
	{
		AllocatedMemorySize-=Info->Size;
		Info->Size = realSize;
		AllocatedMemorySize+=Info->Size;
	}

	return Info? ToUser(Info) : nullptr;
}

static inline const char* getAllocationTypeString(ALLOCATION_TYPE type)
{
	switch(type)
	{
	case AT_RAW: return "malloc";
	case AT_SCALAR: return "operator new";
	case AT_VECTOR: return "operator new[]";
	}
	return "unknown";
}

void PrintMemory()
{
	bool MonitoringState = MonitoringEnabled;
	MonitoringEnabled = false;

	if (CallNewDeleteVector || CallNewDeleteScalar || CallMallocFree || AllocatedMemoryBlocks || AllocatedMemorySize)
	{
		std::wostringstream oss;
		oss << L"Memory leaks detected:" << std::endl;
		if (CallNewDeleteVector)
			oss << L"  delete[]:   " << CallNewDeleteVector << std::endl;
		if (CallNewDeleteScalar)
			oss << L"  delete:     " << CallNewDeleteScalar << std::endl;
		if (CallMallocFree)
			oss << L"  free():     " << CallMallocFree << std::endl;
		if (AllocatedMemoryBlocks)
			oss << L"Total blocks: " << AllocatedMemoryBlocks << std::endl;
		if (AllocatedMemorySize)
			oss << L"Total bytes:  " << AllocatedMemorySize - AllocatedMemoryBlocks * sizeof(MEMINFO) <<  L" payload, " << AllocatedMemoryBlocks * sizeof(MEMINFO) << L" overhead" << std::endl;
		oss << std::endl;

		oss << "Not freed blocks:" << std::endl;

		std::wcout << oss.str();
		OutputDebugString(oss.str().data());
		oss.clear();

		for(auto i = FirstMemBlock.next; i; i = i->next)
		{
			oss << i->File << L':' << i->Line << L" -> " << i->Function << L':' << getAllocationTypeString(i->AllocationType) << L" (" << i->Size - sizeof(MEMINFO) << L" bytes)" << std::endl;
			std::wcout << oss.str();
			OutputDebugString(oss.str().data());
			oss.clear();
		}
	}
	MonitoringEnabled = MonitoringState;
}

};

void* xf_malloc(size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugAllocator(size, memcheck::AT_RAW, Function, File, Line);
}

void xf_free(void* block)
{
	return memcheck::DebugDeallocator(block, memcheck::AT_RAW);
}

void* xf_realloc(void* block, size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugReallocator(block, size, Function, File, Line);
}

void* xf_realloc_nomove(void * block, size_t size, const char* Function, const char* File, int Line)
{
	if (!block)
	{
		return xf_malloc(size, Function, File, Line);
	}
	else if (memcheck::DebugExpander(block, size))
	{
		return block;
	}
	else
	{
		xf_free(block);
		return xf_malloc(size, File, Function, Line);
	}
}

void* operator new(size_t size)
{
	return memcheck::DebugAllocator(size, memcheck::AT_SCALAR, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new[](size_t size)
{
	return memcheck::DebugAllocator(size, memcheck::AT_VECTOR, __FUNCTION__, __FILE__, __LINE__);
}

void* operator new(size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugAllocator(size, memcheck::AT_SCALAR, Function, File, Line);
}

void* operator new[](size_t size, const char* Function, const char* File, int Line)
{
	return memcheck::DebugAllocator(size, memcheck::AT_VECTOR, Function, File, Line);
}

void operator delete(void* block)
{
	return memcheck::DebugDeallocator(block, memcheck::AT_SCALAR);
}

void operator delete[](void* block)
{
	return memcheck::DebugDeallocator(block, memcheck::AT_VECTOR);
}

void operator delete(void* block, const char* Function, const char* File, int Line)
{
	return memcheck::DebugDeallocator(block, memcheck::AT_SCALAR);
}

void operator delete[](void* block, const char* Function, const char* File, int Line)
{
	return memcheck::DebugDeallocator(block, memcheck::AT_VECTOR);
}

char* DuplicateString(const char * string, const char* Function, const char* File, int Line)
{
	return string ? strcpy(new(Function, File, Line) char[strlen(string) + 1], string) : nullptr;
}

wchar_t* DuplicateString(const wchar_t * string, const char* Function, const char* File, int Line)
{
	return string ? wcscpy(new(Function, File, Line) wchar_t[wcslen(string) + 1], string) : nullptr;
}

#endif

void PrintMemory()
{
#ifdef MEMCHECK
	memcheck::PrintMemory();
#endif
}


// dest и src НЕ ДОЛЖНЫ пересекаться
char * xstrncpy(char * dest,const char * src,size_t DestSize)
{
	char *tmpsrc = dest;

	while (DestSize>1 && (*dest++ = *src++))
	{
		DestSize--;
	}

	*dest = 0;
	return tmpsrc;
}

wchar_t * xwcsncpy(wchar_t * dest,const wchar_t * src,size_t DestSize)
{
	wchar_t *tmpsrc = dest;

	while (DestSize>1 && (*dest++ = *src++))
		DestSize--;

	*dest = 0;
	return tmpsrc;
}
