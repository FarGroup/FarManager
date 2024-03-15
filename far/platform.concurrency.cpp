/*
platform.concurrency.cpp

Threads, mutexes, events, critical sections etc.
*/
/*
Copyright © 2017 Far Group
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "platform.concurrency.hpp"

// Internal:
#include "exception.hpp"
#include "imports.hpp"
#include "pathmix.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"

// Common:
#include "common.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

#include <process.h>

//----------------------------------------------------------------------------

namespace os::concurrency
{
	string detail::make_name(string_view const Namespace, string_view const HashPart, string_view const TextPart)
	{
		auto Str = concat(Namespace, str(hash_range(HashPart)), L'_', TextPart);
		ReplaceBackslashToSlash(Str);
		return Str;
	}

	critical_section::critical_section()
	{
		InitializeCriticalSection(&m_Object);
	}

	critical_section::~critical_section()
	{
		DeleteCriticalSection(&m_Object);
	}

	void critical_section::lock()
	{
		EnterCriticalSection(&m_Object);
	}

	void critical_section::unlock()
	{
		LeaveCriticalSection(&m_Object);
	}


	thread::~thread()
	{
		finalise();
	}

	void thread::finalise()
	{
		if (joinable())
			join();
	}

	thread& thread::operator=(thread&& rhs) noexcept
	{
		finalise();

		handle::operator=(std::move(rhs));

		m_ThreadId = rhs.m_ThreadId;
		rhs.m_ThreadId = {};

		return *this;
	}

	unsigned thread::get_id() const
	{
		return m_ThreadId;
	}

	bool thread::joinable() const
	{
		return *this != nullptr;
	}

	void thread::detach()
	{
		check_joinable();
		close();
		m_ThreadId = 0;
	}

	void thread::join()
	{
		check_joinable();
		wait();
		close();
		m_ThreadId = 0;
	}

	void thread::check_joinable() const
	{
		if (!joinable())
			throw far_fatal_exception(L"Thread is not joinable"sv);
	}

	void thread::starter_impl(proc_type Proc, void* Param)
	{
		reset(std::bit_cast<HANDLE>(_beginthreadex(nullptr, 0, Proc, Param, 0, &m_ThreadId)));

		if (!*this)
			throw far_fatal_exception(L"Can't create thread"sv);
	}


	mutex::mutex(string_view const Name):
		handle(CreateMutex(nullptr, false, EmptyToNull(null_terminated(Name))))
	{
	}

	string_view mutex::get_namespace()
	{
		return L"Far_Manager_Mutex"sv;
	}

	void mutex::lock() const
	{
		wait();
	}

	void mutex::unlock() const
	{
		if (!ReleaseMutex(native_handle()))
			throw far_fatal_exception(L"ReleaseMutex failed"sv);
	}


	namespace detail
	{
		class i_shared_mutex
		{
		public:
			virtual ~i_shared_mutex() = default;
			virtual void lock() = 0;
			[[nodiscard]]
			virtual bool try_lock() = 0;
			virtual void unlock() = 0;
			virtual void lock_shared() = 0;
			[[nodiscard]]
			virtual bool try_lock_shared() = 0;
			virtual void unlock_shared() = 0;
		};

		class shared_mutex_legacy final: public i_shared_mutex
		{
		public:
			NONCOPYABLE(shared_mutex_legacy);

			shared_mutex_legacy() { imports.RtlInitializeResource(&m_Lock); m_Lock.Flags |= RTL_RESOURCE_FLAG_LONG_TERM; }
			~shared_mutex_legacy() override { imports.RtlDeleteResource(&m_Lock); }

			void lock() override { imports.RtlAcquireResourceExclusive(&m_Lock, TRUE); }
			bool try_lock() override { return imports.RtlAcquireResourceExclusive(&m_Lock, FALSE) != FALSE; }
			void unlock() override { imports.RtlReleaseResource(&m_Lock); }
			void lock_shared() override { imports.RtlAcquireResourceShared(&m_Lock, TRUE); }
			bool try_lock_shared() override { return imports.RtlAcquireResourceShared(&m_Lock, FALSE) != FALSE; }
			void unlock_shared() override { imports.RtlReleaseResource(&m_Lock); }

		private:
			RTL_RESOURCE m_Lock{};
		};

		class shared_mutex_srw final: public i_shared_mutex
		{
		public:
			NONCOPYABLE(shared_mutex_srw);

			shared_mutex_srw() = default;

			void lock() override { imports.AcquireSRWLockExclusive(&m_Lock); }
			bool try_lock() override { return imports.TryAcquireSRWLockExclusive(&m_Lock) != FALSE; }
			void unlock() override { imports.ReleaseSRWLockExclusive(&m_Lock); }
			void lock_shared() override { imports.AcquireSRWLockShared(&m_Lock); }
			bool try_lock_shared() override { return imports.TryAcquireSRWLockShared(&m_Lock) != FALSE; }
			void unlock_shared() override { imports.ReleaseSRWLockShared(&m_Lock); }

		private:
			SRWLOCK m_Lock{};
		};
	}

	static std::unique_ptr<detail::i_shared_mutex> make_shared_mutex()
	{
		// Windows 7 and above
		if (imports.TryAcquireSRWLockExclusive)
			return std::make_unique<detail::shared_mutex_srw>();

		return std::make_unique<detail::shared_mutex_legacy>();
	}


	shared_mutex::shared_mutex():
		m_Impl(make_shared_mutex())
	{
	}

	shared_mutex::~shared_mutex() = default;

	void shared_mutex::lock()            { return m_Impl->lock(); }
	bool shared_mutex::try_lock()        { return m_Impl->try_lock(); }
	void shared_mutex::unlock()          { return m_Impl->unlock(); }
	void shared_mutex::lock_shared()     { return m_Impl->lock_shared(); }
	bool shared_mutex::try_lock_shared() { return m_Impl->try_lock_shared(); }
	void shared_mutex::unlock_shared()   { return m_Impl->unlock_shared(); }


	event::event(type const Type, state const InitialState, string_view const Name):
		handle(CreateEvent(nullptr, Type == type::manual, InitialState == state::signaled, EmptyToNull(null_terminated(Name))))
	{
	}

	string_view event::get_namespace()
	{
		return L"Far_Manager_Event"sv;
	}

	void event::set() const
	{
		check_valid();
		if (!SetEvent(get()))
			throw far_fatal_exception(L"SetEvent failed"sv);
	}

	void event::reset() const
	{
		check_valid();
		if (!ResetEvent(get()))
			throw far_fatal_exception(L"ResetEvent failed"sv);
	}

	void event::associate(OVERLAPPED& o) const
	{
		check_valid();
		o.hEvent = get();
	}

	void event::check_valid() const
	{
		if (!*this)
		{
			throw far_fatal_exception(L"Event is not initialized properly"sv);
		}
	}

	static void CALLBACK wrapper(void* const Parameter, BOOLEAN)
	{
		const auto& Callable = view_as<std::function<void()>>(Parameter);
		Callable();
	}

	void timer::initialise_impl(std::chrono::milliseconds const DueTime, std::chrono::milliseconds Period)
	{
		if (!CreateTimerQueueTimer(&ptr_setter(m_Timer), {}, &wrapper, m_Callable.get(), DueTime / 1ms, Period / 1ms, WT_EXECUTEDEFAULT))
			throw far_fatal_exception(L"CreateTimerQueueTimer failed"sv);
	}

	void timer::timer_closer::operator()(HANDLE const Handle) const
	{
		while (!(DeleteTimerQueueTimer({}, Handle, INVALID_HANDLE_VALUE) || GetLastError() == ERROR_IO_PENDING))
			LOGWARNING(L"DeleteTimerQueueTimer(): {}"sv, last_error());
	}
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("platform.concurrency.make_name")
{
	struct s
	{
		static auto get_namespace()
		{
			return L"banana"sv;
		}
	};

	const auto Suffix = L"peel"sv;
	const auto Name = os::make_name<s>(L"random"sv, Suffix);

	REQUIRE(Name.starts_with(s::get_namespace()));
	REQUIRE(Name.ends_with(Suffix));
}

TEMPLATE_TEST_CASE("platform.concurrency.locking", "", os::critical_section, os::mutex, os::shared_mutex)
{
	std::atomic_int Cookie;
	TestType Lock1, Lock2;

	os::event const Event(os::event::type::automatic, os::event::state::nonsignaled);
	os::thread const Thread(
		[&]
		{
			SCOPED_ACTION(std::scoped_lock)(Lock2);
			Event.set();

			{
				SCOPED_ACTION(std::scoped_lock)(Lock1);
				REQUIRE(Cookie == 1);
				Cookie = 2;
			}
		}
	);

	{
		SCOPED_ACTION(std::scoped_lock)(Lock1);
		Event.wait();

		Cookie = 1;
	}

	{
		SCOPED_ACTION(std::scoped_lock)(Lock2);
		REQUIRE(Cookie == 2);
	}
}

TEST_CASE("platform.concurrency.thread.forwarding")
{
	{
		const auto Magic = 42;
		os::thread Thread(
			[Ptr = std::make_unique<int>(Magic)](auto&& Param)
			{
				REQUIRE(*Ptr * 2 == *Param);
			},
			std::make_unique<int>(Magic * 2)
		);
	}
	SUCCEED();
}

TEST_CASE("platform.concurrency.event")
{
	{
		os::event const Event(os::event::type::manual, os::event::state::nonsignaled);
		REQUIRE(!Event.is_signaled());

		Event.set();
		REQUIRE(Event.is_signaled());

		Event.reset();
		REQUIRE(!Event.is_signaled());
	}

	{
		os::event const Event(os::event::type::automatic, os::event::state::signaled);
		REQUIRE(Event.is_signaled());
		REQUIRE(!Event.is_signaled());

		Event.set();
		REQUIRE(Event.is_signaled());
		REQUIRE(!Event.is_signaled());

		Event.reset();
		REQUIRE(!Event.is_signaled());
	}

	{
		os::event const Event;
		OVERLAPPED o;
		REQUIRE_THROWS_AS(Event.associate(o), far_fatal_exception);
		REQUIRE_THROWS_AS(Event.set(), far_fatal_exception);
		REQUIRE_THROWS_AS(Event.reset(), far_fatal_exception);
	}
}

TEST_CASE("platform.concurrency.timer")
{
	size_t Count{};
	size_t const Max = 3;
	os::event const Event(os::event::type::manual, os::event::state::nonsignaled);

	os::concurrency::timer const Timer({}, 1ms, [&]
	{
		if (Count != Max)
			++Count;
		else
			Event.set();
	});

	Event.wait();
	REQUIRE(Max == Count);
}
#endif
