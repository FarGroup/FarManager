#ifndef SYNCHRO_HPP_ED4F0813_C518_409B_8576_F2E7CF4166CC
#define SYNCHRO_HPP_ED4F0813_C518_409B_8576_F2E7CF4166CC
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

using CriticalSectionLock = std::lock_guard<CriticalSection>;

template<class T, class S>
string make_name(const S& HashPart, const S& TextPart)
{
	auto Str = T::GetNamespace() + str(make_hash(HashPart)) + L"_" + TextPart;
	ReplaceBackslashToSlash(Str);
	return Str;
}

class waitable
{
public:
	NONCOPYABLE(waitable);
	TRIVIALLY_MOVABLE(waitable);

	bool Wait(DWORD Milliseconds = INFINITE) const { return m_Handle.wait(Milliseconds); }
	bool Signaled() const { return m_Handle.signaled(); }
	void Close() { m_Handle.close(); }
	auto native_handle() const { return m_Handle.native_handle(); }

protected:
	explicit waitable(HANDLE Handle = nullptr): m_Handle(Handle) {}
	os::handle m_Handle;
};

class Thread: public waitable
{
public:
	NONCOPYABLE(Thread);
	TRIVIALLY_MOVABLE(Thread);

	using mode = void (Thread::*)();

	Thread(): m_Mode(), m_ThreadId() {}

	template<typename callable, typename... args>
	Thread(mode Mode, callable&& Callable, args&&... Args): m_Mode(Mode)
	{
		Starter(std::bind(std::forward<callable>(Callable), std::forward<args>(Args)...));
	}

	~Thread()
	{
		if (joinable())
		{
			std::invoke(m_Mode, this);
		}
	}

	unsigned int get_id() const { return m_ThreadId; }

	bool joinable() const { return !!m_Handle; }

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
	void check_joinable() const
	{
		if (!joinable())
			throw MAKE_FAR_EXCEPTION("Thread is not joinable");
	}

	template<class T>
	void Starter(T&& f)
	{
		auto Param = std::make_unique<T>(std::move(f));
		m_Handle.reset(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, Wrapper<T>, Param.get(), 0, &m_ThreadId)));
		if (m_Handle)
		{
			Param.release();
		}
		else
		{
			throw MAKE_FAR_EXCEPTION("Can't create thread");
		}
	}

	template<class T>
	static unsigned int WINAPI Wrapper(void* RawPtr)
	{
		std::invoke(*std::unique_ptr<T>(reinterpret_cast<T*>(RawPtr)));
		return 0;
	}

	mode m_Mode;
	unsigned int m_ThreadId;
};

class Mutex: public waitable
{
public:
	NONCOPYABLE(Mutex);
	TRIVIALLY_MOVABLE(Mutex);

	Mutex(const wchar_t* Name = nullptr): waitable(CreateMutex(nullptr, false, EmptyToNull(Name))) {}

	static constexpr const wchar_t *GetNamespace() { return L"Far_Manager_Mutex_"; }

	bool lock() const { return m_Handle.wait(); }

	bool unlock() const { return ReleaseMutex(m_Handle.native_handle()) != FALSE; }
};

class Event: public waitable
{
public:
	NONCOPYABLE(Event);
	TRIVIALLY_MOVABLE(Event);

	enum event_type { automatic, manual };
	enum event_state { nonsignaled, signaled };

	Event() = default;
	Event(event_type Type, event_state InitialState, const wchar_t* Name = nullptr): waitable(CreateEvent(nullptr, Type == manual, InitialState == signaled, EmptyToNull(Name))) {}

	static constexpr const wchar_t *GetNamespace() { return L"Far_Manager_Event_"; }

	bool Set() const { check_valid(); return SetEvent(m_Handle.native_handle()) != FALSE; }

	bool Reset() const { check_valid(); return ResetEvent(m_Handle.native_handle()) != FALSE; }

	void Associate(OVERLAPPED& o) const { check_valid(); o.hEvent = m_Handle.native_handle(); }

private:
	void check_valid() const
	{
		if (!m_Handle)
		{
			throw MAKE_FAR_EXCEPTION("Event not initialized properly");
		}
	}
};

template<class T> class SyncedQueue: noncopyable
{
public:
	using value_type = T;

	bool empty() const
	{
		SCOPED_ACTION(CriticalSectionLock)(m_QueueCS);
		return m_Queue.empty();
	}

	void push(const T& item)
	{
		SCOPED_ACTION(CriticalSectionLock)(m_QueueCS);
		m_Queue.push(item);
	}

	template<typename... args>
	void emplace(args... Args)
	{
		SCOPED_ACTION(CriticalSectionLock)(m_QueueCS);
		m_Queue.emplace(std::forward<args>(Args)...);
	}

	void push(T&& item)
	{
		SCOPED_ACTION(CriticalSectionLock)(m_QueueCS);
		m_Queue.push(std::forward<T>(item));
	}

	bool try_pop(T& To)
	{
		SCOPED_ACTION(CriticalSectionLock)(m_QueueCS);
		if (!m_Queue.empty())
		{
			To = std::move(m_Queue.front());
			m_Queue.pop();
			return true;
		}
		return false;
	}

	size_t size() const
	{
		SCOPED_ACTION(CriticalSectionLock)(m_QueueCS);
		return m_Queue.size();
	}

	void clear()
	{
		SCOPED_ACTION(CriticalSectionLock)(m_QueueCS);
		clear_and_shrink(m_Queue);
	}

	auto scoped_lock() { return make_raii_wrapper(this, &SyncedQueue::lock, &SyncedQueue::unlock); }

private:
	void lock() { return m_QueueCS.lock(); }
	void unlock() { return m_QueueCS.unlock(); }

	std::queue<T> m_Queue;
	mutable CriticalSection m_QueueCS;
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
	template<typename T>
	MultiWaiter(T Begin, T End) { std::for_each(Begin, End, [this](const auto& i){ this->Add(i); }); }
	void Add(const waitable& Object) { assert(Objects.size() < MAXIMUM_WAIT_OBJECTS); Objects.emplace_back(Object.native_handle()); }
	void Add(HANDLE handle) { assert(Objects.size() < MAXIMUM_WAIT_OBJECTS); Objects.emplace_back(handle); }
	DWORD Wait(wait_mode Mode = wait_all, DWORD Milliseconds = INFINITE) const { return WaitForMultipleObjects(static_cast<DWORD>(Objects.size()), Objects.data(), Mode == wait_all, Milliseconds); }
	void Clear() {Objects.clear();}

private:
	std::vector<HANDLE> Objects;
};

#endif // SYNCHRO_HPP_ED4F0813_C518_409B_8576_F2E7CF4166CC
