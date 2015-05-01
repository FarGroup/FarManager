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
#include "pathmix.hpp"

class CriticalSection: noncopyable
{

public:
	CriticalSection() { InitializeCriticalSection(&m_object); }
	~CriticalSection() { DeleteCriticalSection(&m_object); }

	void lock() { EnterCriticalSection(&m_object); }
	void unlock() { LeaveCriticalSection(&m_object); }

private:
	CRITICAL_SECTION m_object;
};

template<class T> class lock_guard: noncopyable
{
public:
	explicit lock_guard(T& object): m_object(object) { m_object.lock(); }
	~lock_guard() { m_object.unlock(); }

private:
	T& m_object;
};

typedef lock_guard<CriticalSection> CriticalSectionLock;

template<class T, class S>
inline string make_name(const S& HashPart, const S& TextPart)
{
	auto Str = T::GetNamespace() + std::to_wstring(make_hash(HashPart)) + L"_" + TextPart;
	ReplaceBackslashToSlash(Str);
	return Str;
}

class Thread: public os::HandleWrapper
{
public:
	typedef void (Thread::*mode)();

	Thread(): m_Mode(), m_ThreadId() {}
	Thread(Thread&& rhs) noexcept: m_Mode(), m_ThreadId() { *this = std::move(rhs); }

#if defined _MSC_VER && _MSC_VER < 1800

	template<typename T> \
	Thread(mode Mode, T&& Function): m_Mode(Mode) { Starter(std::bind(std::forward<T>(Function))); }

	#define THREAD_VTE(TYPENAME_LIST, ARG_LIST, REF_ARG_LIST, FWD_ARG_LIST) \
	template<typename T, TYPENAME_LIST> \
	Thread(mode Mode, T&& Function, REF_ARG_LIST): m_Mode(Mode) { Starter(std::bind(std::forward<T>(Function), FWD_ARG_LIST)); }

	#include "common/variadic_emulation_helpers_begin.hpp"
	VTE_GENERATE(THREAD_VTE)
	#include "common/variadic_emulation_helpers_end.hpp"

	#undef THREAD_VTE
#else
	template<class T, class... Args>
	Thread(mode Mode, T&& Function, Args&&... args): m_Mode(Mode)
	{
		Starter(std::bind(std::forward<T>(Function), std::forward<Args>(args)...));
	}
#endif

	virtual ~Thread()
	{
		if (joinable())
		{
			(this->*m_Mode)();
		}
	}

	MOVE_OPERATOR_BY_SWAP(Thread);

	void swap(Thread& rhs) noexcept
	{
		using std::swap;
		m_Handle.swap(rhs.m_Handle);
		swap(m_Mode, rhs.m_Mode);
		swap(m_ThreadId, rhs.m_ThreadId);
	}

	FREE_SWAP(Thread);

	unsigned int get_id() const { return m_ThreadId; }

	bool joinable() const { return m_ThreadId != 0; }

	void detach()
	{
		check_joinable();
		m_Handle.close();
		m_ThreadId = 0;
	}

	void join()
	{
		check_joinable();
		m_Handle.wait();
		m_Handle.close();
		m_ThreadId = 0;
	}

private:
	void check_joinable()
	{
		if (!joinable())
			throw std::runtime_error("Thread is not joinable"); // TODO: std::system_error
	}

	template<class T>
	void Starter(T&& f)
	{
		auto Param = new T(std::move(f));
		if (!(m_Handle = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, Wrapper<T>, Param, 0, &m_ThreadId))))
		{
			delete Param;
			throw std::runtime_error("Can't create thread"); // TODO: std::system_error
		}
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

	mode m_Mode;
	unsigned int m_ThreadId;
};

class Mutex: public os::HandleWrapper
{
public:
	Mutex(const wchar_t* Name = nullptr): HandleWrapper(CreateMutex(nullptr, false, EmptyToNull(Name))) {}
	Mutex(Mutex& rhs) noexcept { *this = std::move(rhs); }

	virtual ~Mutex() {}

	static const wchar_t *GetNamespace() { return L"Far_Manager_Mutex_"; }

	MOVE_OPERATOR_BY_SWAP(Mutex);

	void swap(Mutex& rhs) noexcept
	{
		m_Handle.swap(rhs.m_Handle);
	}

	FREE_SWAP(Mutex);

	bool lock() const { return m_Handle.wait(); }

	bool unlock() const { return ReleaseMutex(m_Handle.native_handle()) != FALSE; }
};

class Event: public os::HandleWrapper
{
public:
	enum event_type { automatic, manual };
	enum event_state { nonsignaled, signaled };

	Event() {}
	Event(event_type Type, event_state InitialState, const wchar_t* Name = nullptr): HandleWrapper(CreateEvent(nullptr, Type == manual, InitialState == signaled, EmptyToNull(Name))) {}
	Event(Event& rhs) noexcept { *this = std::move(rhs); }
	virtual ~Event() {}

	static const wchar_t *GetNamespace() { return L"Far_Manager_Event_"; }

	MOVE_OPERATOR_BY_SWAP(Event);

	void swap(Event& rhs) noexcept
	{
		m_Handle.swap(rhs.m_Handle);
	}

	FREE_SWAP(Event);

	bool Set() const { check_valid(); return SetEvent(m_Handle.native_handle()) != FALSE; }

	bool Reset() const { check_valid(); return ResetEvent(m_Handle.native_handle()) != FALSE; }

	void Associate(OVERLAPPED& o) const { check_valid(); o.hEvent = m_Handle.native_handle(); }

private:
	void check_valid() const
	{
		if (!m_Handle)
		{
			throw std::runtime_error("not initialized properly");
		}
	}
};

template<class T> class SyncedQueue: noncopyable {
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

class MultiWaiter: noncopyable
{
public:
	enum wait_mode
	{
		wait_any,
		wait_all
	};
	MultiWaiter() { Objects.reserve(10); }
	~MultiWaiter() {}
	void Add(const os::HandleWrapper& Object) { assert(Objects.size() < MAXIMUM_WAIT_OBJECTS); Objects.emplace_back(Object.m_Handle.native_handle()); }
	void Add(HANDLE handle) { assert(Objects.size() < MAXIMUM_WAIT_OBJECTS); Objects.emplace_back(handle); }
	DWORD Wait(wait_mode Mode = wait_all, DWORD Milliseconds = INFINITE) const { return WaitForMultipleObjects(static_cast<DWORD>(Objects.size()), Objects.data(), Mode == wait_all, Milliseconds); }
	void Clear() {Objects.clear();}

private:
	std::vector<HANDLE> Objects;
};
