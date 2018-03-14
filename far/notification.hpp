#ifndef NOTIFICATION_HPP_B0BB0D31_61E8_49C3_AA4F_E8C1D7D71A25
#define NOTIFICATION_HPP_B0BB0D31_61E8_49C3_AA4F_E8C1D7D71A25
#pragma once

/*
notification.hpp

*/
/*
Copyright © 2013 Far Group
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

#include "platform.concurrency.hpp"

enum event_id
{
	update_intl,
	update_power,
	update_devices,
	update_environment,

	plugin_synchro,

	event_id_count
};

struct update_devices_message
{
	bool Arrival;
	unsigned Drives;
};

class wm_listener;

namespace detail
{
	class i_event_handler
	{
	public:
		virtual ~i_event_handler() = default;
		virtual void operator()(const any&) const = 0;
	};

	class event_handler: public i_event_handler
	{
	public:
		using handler_type = std::function<void()>;

		explicit event_handler(handler_type Handler):
			m_Handler(std::move(Handler))
		{
		}

		void operator()(const any&) const override
		{
			m_Handler();
		}

	private:
		handler_type m_Handler;
	};

	class parametrized_event_handler: public i_event_handler
	{
	public:
		using handler_type = std::function<void(const any&)>;

		explicit parametrized_event_handler(handler_type Handler):
			m_Handler(std::move(Handler))
		{
		}

		void operator()(const any& Payload) const override
		{
			m_Handler(Payload);
		}

	private:
		handler_type m_Handler;
	};
}

class message_manager: noncopyable
{
public:
	using handlers_map = std::multimap<string, const detail::i_event_handler*>;

	handlers_map::iterator subscribe(event_id EventId, const detail::i_event_handler& EventHandler);
	handlers_map::iterator subscribe(const string& EventName, const detail::i_event_handler& EventHandler);
	void unsubscribe(handlers_map::iterator HandlerIterator);
	void notify(event_id EventId, any&& Payload = any());
	void notify(const string& EventName, any&& Payload = any());
	bool dispatch();

	class suppress: noncopyable
	{
	public:
		suppress();
		~suppress();

	private:
		message_manager& m_owner;
	};

private:
	friend message_manager& MessageManager();

	using message_queue = os::synced_queue<std::pair<string, any>>;

	message_manager();

	os::critical_section m_QueueCS;
	message_queue m_Messages;
	handlers_map m_Handlers;
	std::unique_ptr<wm_listener> m_Window;
	std::atomic_ulong m_suppressions;
};

message_manager& MessageManager();

namespace detail
{
	string CreateEventName();

	template<class T>
	class listener_t: noncopyable
	{
	public:
		template<class id_type>
		listener_t(const id_type& EventId, const typename T::handler_type& EventHandler):
			m_Handler(EventHandler),
			m_Iterator(MessageManager().subscribe(EventId, m_Handler))
		{
		}

		explicit listener_t(const typename T::handler_type& EventHandler):
			listener_t(CreateEventName(), EventHandler)
		{
		}

		~listener_t()
		{
			MessageManager().unsubscribe(m_Iterator);
		}

		const string& GetEventName() const
		{
			return m_Iterator->first;
		}

	private:
		T m_Handler;
		message_manager::handlers_map::iterator m_Iterator;
	};
}

using listener = detail::listener_t<detail::event_handler>;
using listener_ex = detail::listener_t<detail::parametrized_event_handler>;

#endif // NOTIFICATION_HPP_B0BB0D31_61E8_49C3_AA4F_E8C1D7D71A25
