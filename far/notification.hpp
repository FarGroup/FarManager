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

// Internal:

// Platform:
#include "platform.concurrency.hpp"

// Common:
#include "common/singleton.hpp"
#include "common/type_traits.hpp"

// External:

//----------------------------------------------------------------------------

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
	unsigned Drives;
	bool Arrival;
	bool Media;
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
		template<typename callable_type, REQUIRES(is_detected_v<with_payload, callable_type>)>
		explicit event_handler(callable_type&& Handler):
			function(FWD(Handler))
		{
		}

		template<typename callable_type, REQUIRES(is_detected_v<without_payload, callable_type>)>
		explicit event_handler(callable_type&& Handler):
			function([Handler = FWD(Handler)](const std::any&) { Handler(); })
		{
		}
	};
}

class message_manager: public singleton<message_manager>
{
	IMPLEMENTS_SINGLETON;

public:
	using handlers_map = std::unordered_multimap<string, const detail::event_handler*>;

	handlers_map::iterator subscribe(event_id EventId, const detail::event_handler& EventHandler);
	handlers_map::iterator subscribe(string_view EventName, const detail::event_handler& EventHandler);
	void unsubscribe(handlers_map::iterator HandlerIterator);
	void notify(event_id EventId, std::any&& Payload = {});
	void notify(string_view EventName, std::any&& Payload = {});
	bool dispatch();

	[[nodiscard]]
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

	// Note: non-std - std is incompatible with Win2k
	using mutex_type = os::concurrency::shared_mutex;
	mutex_type m_RWLock;
	message_queue m_Messages;
	handlers_map m_Handlers;
	std::unique_ptr<wm_listener> m_Window;
	std::atomic_ulong m_suppressions{};
};

class listener: noncopyable
{
public:
	template<class id_type, typename callable_type>
	listener(const id_type& EventId, const callable_type& EventHandler):
		m_Handler(EventHandler),
		m_Iterator(message_manager::instance().subscribe(EventId, m_Handler))
	{
	}

	template<typename callable_type>
	explicit listener(const callable_type& EventHandler):
		listener(CreateEventName(), EventHandler)
	{
	}

	~listener()
	{
		message_manager::instance().unsubscribe(m_Iterator);
	}

	const string& GetEventName() const
	{
		return m_Iterator->first;
	}

private:
	static string CreateEventName();

	detail::event_handler m_Handler;
	message_manager::handlers_map::iterator m_Iterator;
};


#endif // NOTIFICATION_HPP_B0BB0D31_61E8_49C3_AA4F_E8C1D7D71A25
