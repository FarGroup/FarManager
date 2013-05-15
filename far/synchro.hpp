#pragma once

/*
synchro.hpp

Критические секции, мютексы, эвенты и т.п.
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

class CriticalSection
{

public:
	CriticalSection() { InitializeCriticalSection(&object); }
	~CriticalSection() { DeleteCriticalSection(&object); }

	void Enter() { EnterCriticalSection(&object); }
	void Leave() { LeaveCriticalSection(&object); }

private:
	CRITICAL_SECTION object;
};

class CriticalSectionLock:NonCopyable
{
public:
	CriticalSectionLock(CriticalSection &object): object(object) { object.Enter(); }
	~CriticalSectionLock() { object.Leave(); }

private:
	CriticalSection &object;
};

class HandleWrapper:NonCopyable
{
protected:

	HANDLE h;
	FormatString strName;

public:

	HandleWrapper() : h(nullptr) {}

	void SetName(const wchar_t *HashPart, const wchar_t *TextPart)
	{
		if (!TextPart)
			return;
		unsigned hs = 0;
		if (HashPart)
			while (*HashPart)
				hs = hs*17 + *HashPart++;
		strName << GetNamespace() << hs << L" " << TextPart;
	}

	virtual const wchar_t *GetNamespace() const = 0;

	bool Opened() { return h != nullptr; }

	bool Close()
	{
		if (!h) return true;
		bool ret = CloseHandle(h) != FALSE;
		h = nullptr;
		return ret;
	}

	bool Wait(DWORD Milliseconds=INFINITE) { return WaitForSingleObject(h, Milliseconds)==WAIT_OBJECT_0; }


	virtual ~HandleWrapper() { Close(); }

private:
	HANDLE GetHandle() const { return h; }
	friend class MultiWaiter;
};

class Thread: public HandleWrapper
{
public:

	Thread() {}

	virtual ~Thread() {}

	const wchar_t *GetNamespace() const { return L""; }

	bool Start(unsigned int (*Function)(void*), void* Parameter = nullptr, unsigned int* ThreadId = nullptr)
	{
		return Starter(std::bind(Function, Parameter), ThreadId);
	}

	template<class T>
	bool MemberStart(T* Object, unsigned int (T::*Function)(void*), void* Parameter = nullptr, unsigned int* ThreadId = nullptr)
	{
		return Starter(std::bind(std::mem_fn(Function), Object, Parameter), ThreadId);
	}

private:
	template<class T>
	bool Starter(T&& f, unsigned int* ThreadId)
	{
		assert(!h);

		auto Param = new T(f);
		if (!(h = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, Wrapper<T>, Param, 0, ThreadId))))
		{
			delete Param;
			return false;
		}
		return true;
	}

	template<class T>
	static unsigned int WINAPI Wrapper(void* p)
	{
		EnableSeTranslation();

		auto pParam = reinterpret_cast<T*>(p);
		auto Param = *pParam;
		delete pParam;

		return Param();
	}
};

class Mutex: public HandleWrapper
{
public:

	Mutex() {}

	virtual ~Mutex() {}

	const wchar_t *GetNamespace() const { return L"Far_Manager_Mutex_"; }

	bool Open()
	{
		assert(!h);

		h = CreateMutex(nullptr, FALSE, strName.IsEmpty() ? nullptr : strName.CPtr());
		return h != nullptr;
	}

	bool Lock() { return Wait(); }

	bool Unlock() { return ReleaseMutex(h) != FALSE; }
};

class AutoMutex:NonCopyable
{
public:

	AutoMutex(const wchar_t *HashPart=nullptr, const wchar_t *TextPart=nullptr)
	{
		m.SetName(HashPart, TextPart);
		m.Open();
		m.Lock();
	}

	~AutoMutex() { m.Unlock(); }

private:

	Mutex m;
};

class Event: public HandleWrapper
{
public:

	Event() {}

	virtual ~Event() {}

	const wchar_t *GetNamespace() const { return L"Far_Manager_Event_"; }

	bool Open(bool ManualReset=false, bool InitialState=false)
	{
		assert(!h);

		h = CreateEvent(nullptr, ManualReset, InitialState, strName.IsEmpty() ? nullptr : strName.CPtr());
		return h != nullptr;
	}

	bool Set() { return SetEvent(h)!=FALSE; }

	bool Reset() { return ResetEvent(h)!=FALSE; }

	bool Signaled() { return Wait(0); }

	void Associate(OVERLAPPED& o) { o.hEvent = h; }
};

template<class T> class SyncedQueue:NonCopyable {
	std::queue<T> Queue;
	CriticalSection csQueueAccess;

public:

	SyncedQueue() {}
	~SyncedQueue() {}

	bool Empty()
	{
		CriticalSectionLock cslock(csQueueAccess);
		return Queue.empty();
	}

	void Push(T &item)
	{
		CriticalSectionLock cslock(csQueueAccess);
		Queue.push(item);
	}

	T Pop()
	{
		CriticalSectionLock cslock(csQueueAccess);
		T item = Queue.front();
		Queue.pop();
		return item;
	}
};

class MultiWaiter:NonCopyable
{
public:
	MultiWaiter() { Objects.reserve(10); }
	~MultiWaiter() {}
	void Add(HandleWrapper& Object) { Objects.emplace_back(Object.GetHandle()); }
	void Add(HANDLE handle) { Objects.emplace_back(handle); }
	DWORD Wait(bool WaitAll, DWORD Milliseconds) { return WaitForMultipleObjects(static_cast<DWORD>(Objects.size()), Objects.data(), WaitAll, Milliseconds); }
	void Clear() {Objects.clear();}

private:
	std::vector<HANDLE> Objects;
};
