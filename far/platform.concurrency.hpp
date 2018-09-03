﻿#ifndef PLATFORM_CONCURRENCY_HPP_ED4F0813_C518_409B_8576_F2E7CF4166CC
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

#include "exception.hpp"

#include "platform.hpp"

namespace os::concurrency
{
	namespace detail
	{
		string make_name(string_view Namespace, const string& HashPart, string_view TextPart);
	}

	template<class T>
	string make_name(const string& HashPart, string_view const TextPart)
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

	using critical_section_lock = std::lock_guard<critical_section>;


	class thread: public handle
	{
	public:
		NONCOPYABLE(thread);
		MOVABLE(thread);

		using mode = void (thread::*)();

		thread() = default;

		template<typename callable, typename... args>
		thread(mode Mode, callable&& Callable, args&&... Args): m_Mode(Mode)
		{
			starter(std::bind(FWD(Callable), FWD(Args)...));
		}

		~thread();

		unsigned get_id() const;
		bool joinable() const;
		void detach();
		void join();

	private:
		void check_joinable() const;

		template<class T>
		void starter(T&& f)
		{
			auto Param = std::make_unique<T>(std::move(f));
			reset(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, wrapper<T>, Param.get(), 0, &m_ThreadId)));

			if (!*this)
				throw MAKE_FAR_EXCEPTION(L"Can't create thread"sv);

			Param.release();
		}

		template<class T>
		static unsigned int WINAPI wrapper(void* RawPtr)
		{
			std::invoke(*std::unique_ptr<T>(reinterpret_cast<T*>(RawPtr)));
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

		static string_view get_namespace();

		bool lock() const;
		bool unlock() const;
	};

	namespace detail
	{
		class i_shared_mutex
		{
		public:
			virtual ~i_shared_mutex() = default;

			virtual void lock() = 0;
			virtual bool try_lock() = 0;
			virtual void unlock() = 0;
			virtual void lock_shared() = 0;
			virtual bool try_lock_shared() = 0;
			virtual void unlock_shared() = 0;
		};
	}

	// Q: WTF is this, it's in the standard!
	// A: MSVC std version is incompatible with Win2k
	class shared_mutex
	{
	public:
		shared_mutex();

		shared_mutex(const shared_mutex&) = delete;
		shared_mutex& operator=(const shared_mutex&) = delete;

		void lock() { m_Impl->lock(); }
		[[nodiscard]] bool try_lock() { return m_Impl->try_lock(); }
		void unlock() { m_Impl->unlock(); }

		void lock_shared() { m_Impl->lock_shared(); }
		[[nodiscard]] bool try_lock_shared() { return m_Impl->try_lock_shared(); }
		void unlock_shared() { m_Impl->unlock_shared(); }

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

		static string_view get_namespace();

		bool set() const;
		bool reset() const;
		void associate(OVERLAPPED& o) const;

	private:
		void check_valid() const;
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

		multi_waiter();

		template<typename T>
		multi_waiter(T Begin, T End)
		{
			std::for_each(Begin, End, [this](const auto& i){ this->add(i); });
		}

		void add(const handle& Object);
		void add(HANDLE handle);
		DWORD wait(mode Mode, std::chrono::milliseconds Timeout) const;
		DWORD wait(mode Mode = mode::all) const;
		void clear();

	private:
		std::vector<HANDLE> m_Objects;
	};
}

namespace os
{
	using concurrency::make_name;
	using concurrency::critical_section;
	using concurrency::critical_section_lock;
	using concurrency::thread;
	using concurrency::mutex;
	using concurrency::shared_mutex;
	using concurrency::event;
	using concurrency::synced_queue;
	using concurrency::multi_waiter;
}

#endif // PLATFORM_CONCURRENCY_HPP_ED4F0813_C518_409B_8576_F2E7CF4166CC
