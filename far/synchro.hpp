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

class CriticalSection: NonCopyable
{

public:
	CriticalSection() { InitializeCriticalSection(&m_object); }
	~CriticalSection() { DeleteCriticalSection(&m_object); }

	void lock() { EnterCriticalSection(&m_object); }
	void unlock() { LeaveCriticalSection(&m_object); }

private:
	CRITICAL_SECTION m_object;
};

template<class T> class lock_guard: NonCopyable
{
public:
	explicit lock_guard(T& object): m_object(object) { m_object.lock(); }
	~lock_guard() { m_object.unlock(); }

private:
	T& m_object;
};

typedef lock_guard<CriticalSection> CriticalSectionLock;

class HandleWrapper: NonCopyable
{
public:
	HandleWrapper(): m_Handle() {}
	virtual ~HandleWrapper() { Close(); }

	virtual const wchar_t *GetNamespace() const = 0;

	void SetName(const string& HashPart, const string& TextPart)
	{
		m_Name = GetNamespace() + std::to_wstring(make_hash(HashPart)) + L"_" + TextPart;
	}

	bool Opened() const { return m_Handle != nullptr; }

	bool Close()
	{
		if (!m_Handle)
			return true;
		bool ret = CloseHandle(m_Handle) != FALSE;
		m_Handle = nullptr;
		return ret;
	}

	bool Wait(DWORD Milliseconds = INFINITE) const { return WaitForSingleObject(m_Handle, Milliseconds) == WAIT_OBJECT_0; }

	bool Signaled() const { return Wait(0); }

protected:
	void swap(HandleWrapper& rhs)
	{
		std::swap(m_Handle, rhs.m_Handle);
		m_Name.swap(rhs.m_Name);
	}

	HANDLE m_Handle;
	string m_Name;

private:
	friend class MultiWaiter;

	HANDLE GetHandle() const { return m_Handle; }
};

class Thread: public HandleWrapper
{
public:
	Thread(): m_ThreadId() {}
	Thread(Thread&& rhs): m_ThreadId() { *this = std::move(rhs); }
	virtual ~Thread() {}

	virtual const wchar_t *GetNamespace() const override { return L""; }

	MOVE_OPERATOR_BY_SWAP(Thread);

	void swap(Thread& rhs)
	{
		HandleWrapper::swap(rhs);
		std::swap(m_ThreadId, rhs.m_ThreadId);
	}

#if defined _MSC_VER && _MSC_VER < 1800
	template<typename T>
	bool Start(T&& Function) { return Starter(std::bind(Function)); }

	template<typename T, typename A1>
	bool Start(T&& Function, A1&& Arg1) { return Starter(std::bind(Function, Arg1)); }

	template<typename T, typename A1, typename A2>
	bool Start(T&& Function, A1&& Arg1, A2&& Arg2) { return Starter(std::bind(Function, Arg1, Arg2)); }

	template<typename T, typename A1, typename A2, typename A3>
	bool Start(T&& Function, A1&& Arg1, A2&& Arg2, A3&& Arg3) { return Starter(std::bind(Function, Arg1, Arg2, Arg3)); }

	template<typename T, typename A1, typename A2, typename A3, typename A4>
	bool Start(T&& Function, A1&& Arg1, A2&& Arg2, A3&& Arg3, A4&& Arg4) { return Starter(std::bind(Function, Arg1, Arg2, Arg3, Arg4)); }

	// and so on...
#else
	template<class T, class... A>
	bool Start(T&& Function, A&&... Args)
	{
		return Starter(std::bind(Function, Args...));
	}
#endif

	unsigned int GetId() const { return m_ThreadId; }

private:
	template<class T>
	bool Starter(T&& f)
	{
		assert(!m_Handle);

		auto Param = new T(std::move(f));
		if (!(m_Handle = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, Wrapper<T>, Param, 0, &m_ThreadId))))
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
		auto Param = std::move(*pParam);
		delete pParam;
		Param();
		return 0;
	}

	unsigned int m_ThreadId;
};

STD_SWAP_SPEC(Thread);

class Mutex: public HandleWrapper
{
public:

	Mutex() {}
	Mutex(const wchar_t *HashPart, const wchar_t *TextPart)
	{
		SetName(HashPart, TextPart);
		Open();
	}
	Mutex(Mutex& rhs) { *this = std::move(rhs); }

	virtual ~Mutex() {}

	virtual const wchar_t *GetNamespace() const override { return L"Far_Manager_Mutex_"; }

	MOVE_OPERATOR_BY_SWAP(Mutex);

	void swap(Mutex& rhs)
	{
		HandleWrapper::swap(rhs);
	}

	bool Open()
	{
		assert(!m_Handle);

		m_Handle = CreateMutex(nullptr, FALSE, EmptyToNull(m_Name.data()));
		return m_Handle != nullptr;
	}

	bool lock() const { return Wait(); }

	bool unlock() const { return ReleaseMutex(m_Handle) != FALSE; }
};

STD_SWAP_SPEC(Mutex);

class Event: public HandleWrapper
{
public:
	Event() {}
	Event(bool ManualReset, bool InitialState) { Open(ManualReset, InitialState); }
	Event(Event& rhs) { *this = std::move(rhs); }
	virtual ~Event() {}

	virtual const wchar_t *GetNamespace() const override { return L"Far_Manager_Event_"; }

	MOVE_OPERATOR_BY_SWAP(Event);

	void swap(Event& rhs)
	{
		HandleWrapper::swap(rhs);
	}

	bool Open(bool ManualReset=false, bool InitialState=false)
	{
		assert(!m_Handle);

		m_Handle = CreateEvent(nullptr, ManualReset, InitialState, EmptyToNull(m_Name.data()));
		return m_Handle != nullptr;
	}

	bool Set() const { return SetEvent(m_Handle) != FALSE; }

	bool Reset() const { return ResetEvent(m_Handle) != FALSE; }

	void Associate(OVERLAPPED& o) const { o.hEvent = m_Handle; }
};

STD_SWAP_SPEC(Event);

template<class T> class SyncedQueue: NonCopyable {
	std::queue<T> Queue;
	CriticalSection csQueueAccess;

public:
	typedef T value_type;

	SyncedQueue() {}
	~SyncedQueue() {}

	bool Empty()
	{
		SCOPED_ACTION(CriticalSectionLock)(csQueueAccess);
		return Queue.empty();
	}

	void Push(const T& item)
	{
		SCOPED_ACTION(CriticalSectionLock)(csQueueAccess);
		Queue.push(item);
	}

	void Push(T&& item)
	{
		SCOPED_ACTION(CriticalSectionLock)(csQueueAccess);
		Queue.push(std::forward<T>(item));
	}

	bool PopIfNotEmpty(T& To)
	{
		SCOPED_ACTION(CriticalSectionLock)(csQueueAccess);
		if (!Queue.empty())
		{
			To = std::move(Queue.front());
			Queue.pop();
			return true;
		}
		return false;
	}

	size_t Size()
	{
		SCOPED_ACTION(CriticalSectionLock)(csQueueAccess);
		return Queue.size();
	}
};

class MultiWaiter: NonCopyable
{
public:
	enum wait_mode
	{
		wait_any,
		wait_all
	};
	MultiWaiter() { Objects.reserve(10); }
	~MultiWaiter() {}
	void AddThread(Thread&& Object) { Objects.emplace_back(Object.GetHandle()); Threads.emplace_back(std::move(Object)); }
	void Add(const HandleWrapper& Object) { Objects.emplace_back(Object.GetHandle()); }
	void Add(HANDLE handle) { Objects.emplace_back(handle); }
	DWORD Wait(wait_mode Mode = wait_all, DWORD Milliseconds = INFINITE) const { return WaitForMultipleObjects(static_cast<DWORD>(Objects.size()), Objects.data(), Mode == wait_all, Milliseconds); }
	void Clear() {Objects.clear();}

private:
	std::vector<HANDLE> Objects;
	std::vector<Thread> Threads;
};
