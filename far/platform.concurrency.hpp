#ifndef PLATFORM_CONCURRENCY_HPP_ED4F0813_C518_409B_8576_F2E7CF4166CC
#define PLATFORM_CONCURRENCY_HPP_ED4F0813_C518_409B_8576_F2E7CF4166CC
#pragma once

/*
platform.concurrency.hpp

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

// Internal:

// Platform:
#include "platform.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

namespace os::concurrency
{
	namespace detail
	{
		[[nodiscard]]
		string make_name(string_view Namespace, string_view HashPart, string_view TextPart);
	}

	template<class T>
	[[nodiscard]]
	string make_name(string_view const HashPart, string_view const TextPart)
	{
		return detail::make_name(T::get_namespace(), HashPart, TextPart);
	}

	class critical_section
	{
	public:
		NONCOPYABLE(critical_section);
		NONMOVABLE(critical_section);

		critical_section();
		~critical_section();

		void lock();
		void unlock();

	private:
		CRITICAL_SECTION m_Object;
	};

	class thread: public handle
	{
	public:
		NONCOPYABLE(thread);
		MOVABLE(thread);

		enum class mode
		{
			join,
			detach,
		};

		thread() = default;

		template<typename callable, typename... args>
		explicit thread(mode Mode, callable&& Callable, args&&... Args): m_Mode(Mode)
		{
			starter([Callable = FWD(Callable), Args = std::make_tuple(FWD(Args)...)]() mutable // make_tuple for GCC 8.1
			{
				std::apply(FWD(Callable), FWD(Args));
			});
		}

		~thread();

		[[nodiscard]]
		unsigned get_id() const;

		[[nodiscard]]
		bool joinable() const;

		void detach();
		void join();

	private:
		void check_joinable() const;

		template<class T>
		void starter(T&& f)
		{
			auto Param = std::make_unique<T>(std::move(f));
			starter_impl(wrapper<T>, Param.get());
			Param.release();
		}

		using proc_type = unsigned(WINAPI*)(void*);

		void starter_impl(proc_type Proc, void* Param);

		template<class T>
		static unsigned int WINAPI wrapper(void* RawPtr)
		{
			std::invoke(*std::unique_ptr<T>(static_cast<T*>(RawPtr)));
			return 0;
		}

		mode m_Mode{};
		unsigned int m_ThreadId{};
	};

	class mutex: public handle
	{
	public:
		NONCOPYABLE(mutex);
		MOVABLE(mutex);

		explicit mutex(string_view Name = {});

		[[nodiscard]]
		static string_view get_namespace();

		void lock() const;
		void unlock() const;
	};

	namespace detail
	{
		class i_shared_mutex;
	}

	// Q: WTF is this, it's in the standard!
	// A: MSVC std version is incompatible with Win2k
	class shared_mutex
	{
	public:
		NONCOPYABLE(shared_mutex);

		shared_mutex();
		~shared_mutex();

		void lock();
		[[nodiscard]]
		bool try_lock();
		void unlock();
		void lock_shared();
		[[nodiscard]]
		bool try_lock_shared();
		void unlock_shared();

	private:
		std::unique_ptr<detail::i_shared_mutex> m_Impl;
	};

	class event: public handle
	{
	public:
		NONCOPYABLE(event);
		MOVABLE(event);

		enum class type { automatic, manual };
		enum class state { nonsignaled, signaled };

		event() = default;
		event(type Type, state InitialState, string_view Name = {});

		[[nodiscard]]
		static string_view get_namespace();

		void set() const;
		void reset() const;
		void associate(OVERLAPPED& o) const;

	private:
		void check_valid() const;
	};

	template<class T>
	class synced_queue: noncopyable
	{
	public:
		using value_type = T;

		[[nodiscard]]
		bool empty() const
		{
			SCOPED_ACTION(guard_t)(m_QueueCS);
			return m_Queue.empty();
		}

		void push(const T& item)
		{
			SCOPED_ACTION(guard_t)(m_QueueCS);
			m_Queue.push(item);
		}

		template<typename... args>
		void emplace(args&&... Args)
		{
			SCOPED_ACTION(guard_t)(m_QueueCS);
			m_Queue.emplace(FWD(Args)...);
		}

		void push(T&& item)
		{
			SCOPED_ACTION(guard_t)(m_QueueCS);
			m_Queue.push(FWD(item));
		}

		[[nodiscard]]
		bool try_pop(T& To)
		{
			SCOPED_ACTION(guard_t)(m_QueueCS);

			if (m_Queue.empty())
				return false;

			To = std::move(m_Queue.front());
			m_Queue.pop();
			return true;
		}

		[[nodiscard]]
		auto size() const
		{
			SCOPED_ACTION(guard_t)(m_QueueCS);
			return m_Queue.size();
		}

		void clear()
		{
			SCOPED_ACTION(guard_t)(m_QueueCS);
			clear_and_shrink(m_Queue);
		}

		[[nodiscard]]
		auto scoped_lock() { return make_raii_wrapper(this, &synced_queue::lock, &synced_queue::unlock); }

	private:
		// Q: Why not just use CTAD?
		// A: To make Coverity and its compiler happy: 'Internal error #2688: assertion failed at: "edg/src/overload.c", line 27123'
		using guard_t = std::lock_guard<critical_section>;

		void lock() { return m_QueueCS.lock(); }
		void unlock() { return m_QueueCS.unlock(); }

		std::queue<T> m_Queue;
		mutable critical_section m_QueueCS;
	};
}

namespace os
{
	using concurrency::make_name;
	using concurrency::critical_section;
	using concurrency::thread;
	using concurrency::mutex;
	using concurrency::shared_mutex;
	using concurrency::event;
	using concurrency::synced_queue;
}

#endif // PLATFORM_CONCURRENCY_HPP_ED4F0813_C518_409B_8576_F2E7CF4166CC
