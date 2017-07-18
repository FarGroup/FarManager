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

#include "exception.hpp"

namespace os
{

class critical_section
{
public:
	NONCOPYABLE(critical_section);
	MOVABLE(critical_section);

	critical_section() { InitializeCriticalSection(&m_object); }
	~critical_section() { DeleteCriticalSection(&m_object); }

	void lock() { EnterCriticalSection(&m_object); }
	void unlock() { LeaveCriticalSection(&m_object); }

private:
	CRITICAL_SECTION m_object;
};

using critical_section_lock = std::lock_guard<critical_section>;

template<class T, class S>
auto make_name(const S& HashPart, const S& TextPart)
{
	auto Str = concat(T::get_namespace(), str(make_hash(HashPart)), L'_', TextPart);
	ReplaceBackslashToSlash(Str);
	return Str;
}

class thread: public handle
{
public:
	NONCOPYABLE(thread);
	MOVABLE(thread);

	using mode = void (thread::*)();

	thread(): m_Mode(), m_ThreadId() {}

	template<typename callable, typename... args>
	thread(mode Mode, callable&& Callable, args&&... Args): m_Mode(Mode)
	{
		starter(std::bind(FWD(Callable), FWD(Args)...));
	}

	~thread()
	{
		if (joinable())
		{
			std::invoke(m_Mode, this);
		}
	}

	auto get_id() const { return m_ThreadId; }

	bool joinable() const { return *this != nullptr; }

	void detach()
	{
		check_joinable();
		close();
		m_ThreadId = 0;
	}

	void join()
	{
		check_joinable();
		wait();
		close();
		m_ThreadId = 0;
	}

private:
	void check_joinable() const
	{
		if (!joinable())
			throw MAKE_FAR_EXCEPTION(L"Thread is not joinable");
	}

	template<class T>
	void starter(T&& f)
	{
		auto Param = std::make_unique<T>(std::move(f));
		reset(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, wrapper<T>, Param.get(), 0, &m_ThreadId)));
		if (*this)
		{
			Param.release();
		}
		else
		{
			throw MAKE_FAR_EXCEPTION(L"Can't create thread");
		}
	}

	template<class T>
	static unsigned int WINAPI wrapper(void* RawPtr)
	{
		std::invoke(*std::unique_ptr<T>(reinterpret_cast<T*>(RawPtr)));
		return 0;
	}

	mode m_Mode;
	unsigned int m_ThreadId;
};

class mutex: public handle
{
public:
	NONCOPYABLE(mutex);
	MOVABLE(mutex);

	explicit mutex(const wchar_t* Name = nullptr): handle(CreateMutex(nullptr, false, EmptyToNull(Name))) {}

	static auto get_namespace() { return L"Far_Manager_Mutex_"; }

	bool lock() const { return wait(); }

	bool unlock() const { return ReleaseMutex(native_handle()) != FALSE; }
};

class event: public handle
{
public:
	NONCOPYABLE(event);
	MOVABLE(event);

	enum class type { automatic, manual };
	enum class state { nonsignaled, signaled };

	event() = default;
	event(type Type, state InitialState, const wchar_t* Name = nullptr): handle(CreateEvent(nullptr, Type == type::manual, InitialState == state::signaled, EmptyToNull(Name))) {}

	static auto get_namespace() { return L"Far_Manager_Event_"; }

	bool set() const { check_valid(); return SetEvent(get()) != FALSE; }

	bool reset() const { check_valid(); return ResetEvent(get()) != FALSE; }

	void associate(OVERLAPPED& o) const { check_valid(); o.hEvent = get(); }

private:
	void check_valid() const
	{
		if (!*this)
		{
			throw MAKE_FAR_EXCEPTION(L"Event not initialized properly");
		}
	}
};

template<class T>
class synced_queue: noncopyable
{
public:
	using value_type = T;

	bool empty() const
	{
		SCOPED_ACTION(critical_section_lock)(m_QueueCS);
		return m_Queue.empty();
	}

	void push(const T& item)
	{
		SCOPED_ACTION(critical_section_lock)(m_QueueCS);
		m_Queue.push(item);
	}

	template<typename... args>
	void emplace(args... Args)
	{
		SCOPED_ACTION(critical_section_lock)(m_QueueCS);
		m_Queue.emplace(FWD(Args)...);
	}

	void push(T&& item)
	{
		SCOPED_ACTION(critical_section_lock)(m_QueueCS);
		m_Queue.push(FWD(item));
	}

	bool try_pop(T& To)
	{
		SCOPED_ACTION(critical_section_lock)(m_QueueCS);

		if (m_Queue.empty())
			return false;

		To = std::move(m_Queue.front());
		m_Queue.pop();
		return true;
	}

	auto size() const
	{
		SCOPED_ACTION(critical_section_lock)(m_QueueCS);
		return m_Queue.size();
	}

	void clear()
	{
		SCOPED_ACTION(critical_section_lock)(m_QueueCS);
		clear_and_shrink(m_Queue);
	}

	auto scoped_lock() { return make_raii_wrapper(this, &synced_queue::lock, &synced_queue::unlock); }

private:
	void lock() { return m_QueueCS.lock(); }
	void unlock() { return m_QueueCS.unlock(); }

	std::queue<T> m_Queue;
	mutable critical_section m_QueueCS;
};

class multi_waiter: noncopyable
{
public:
	enum class mode
	{
		any,
		all
	};
	multi_waiter() { m_Objects.reserve(10); }
	template<typename T>
	multi_waiter(T Begin, T End) { std::for_each(Begin, End, [this](const auto& i){ this->add(i); }); }
	void add(const handle& Object) { assert(m_Objects.size() < MAXIMUM_WAIT_OBJECTS); m_Objects.emplace_back(Object.native_handle()); }
	void add(HANDLE handle) { assert(m_Objects.size() < MAXIMUM_WAIT_OBJECTS); m_Objects.emplace_back(handle); }
	DWORD wait(mode Mode = mode::all, DWORD Milliseconds = INFINITE) const { return WaitForMultipleObjects(static_cast<DWORD>(m_Objects.size()), m_Objects.data(), Mode == mode::all, Milliseconds); }
	void clear() {m_Objects.clear();}

private:
	std::vector<HANDLE> m_Objects;
};

}

#endif // SYNCHRO_HPP_ED4F0813_C518_409B_8576_F2E7CF4166CC
