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

#include "common/singleton.hpp"
#include "common/type_traits.hpp"

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
	class event_handler: public std::function<void(const std::any&)>
	{
		template<typename T>
		using with_payload = decltype(std::declval<T&>()(std::declval<std::any>()));

		template<typename T>
		using without_payload = decltype(std::declval<T&>()());

	public:
		template<typename callable_type, REQUIRES(is_valid_v<callable_type, with_payload>)>
		explicit event_handler(callable_type&& Handler):
			function(FWD(Handler))
		{
		}

		template<typename callable_type, REQUIRES(is_valid_v<callable_type, without_payload>)>
		explicit event_handler(callable_type&& Handler):
			function([Handler = FWD(Handler)](const std::any&) { Handler(); })
		{
		}
	};
}

class message_manager: public singleton<message_manager>
{
	IMPLEMENTS_SINGLETON(message_manager);

public:
	using handlers_map = std::multimap<string, const detail::event_handler*>;

	handlers_map::iterator subscribe(event_id EventId, const detail::event_handler& EventHandler);
	handlers_map::iterator subscribe(const string& EventName, const detail::event_handler& EventHandler);
	void unsubscribe(handlers_map::iterator HandlerIterator);
	void notify(event_id EventId, std::any&& Payload = {});
	void notify(const string& EventName, std::any&& Payload = {});
	bool dispatch();

	auto suppress()
	{
		return make_raii_wrapper(
			this,
			[](message_manager* Owner){ ++Owner->m_suppressions; },
			[](message_manager* Owner){ --Owner->m_suppressions; }
		);
	}

private:
	using message_queue = os::synced_queue<std::pair<string, std::any>>;

	message_manager();
	~message_manager();

	os::critical_section m_QueueCS;
	message_queue m_Messages;
	handlers_map m_Handlers;
	std::unique_ptr<wm_listener> m_Window;
	std::atomic_ulong m_suppressions{};
};

namespace detail
{
	string CreateEventName();

	template<class T>
	class listener_t: noncopyable
	{
	public:
		template<class id_type, typename callable_type>
		listener_t(const id_type& EventId, const callable_type& EventHandler):
			m_Handler(EventHandler),
			m_Iterator(message_manager::instance().subscribe(EventId, m_Handler))
		{
		}

		template<typename callable_type>
		explicit listener_t(const callable_type& EventHandler):
			listener_t(CreateEventName(), EventHandler)
		{
		}

		~listener_t()
		{
			message_manager::instance().unsubscribe(m_Iterator);
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

#endif // NOTIFICATION_HPP_B0BB0D31_61E8_49C3_AA4F_E8C1D7D71A25
