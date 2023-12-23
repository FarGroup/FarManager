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
#include "common/preprocessor.hpp"
#include "common/singleton.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

enum event_id
{
	update_intl,
	update_power,
	update_devices,
	update_environment,

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
	template<typename type>
	concept with_payload = requires(type t, std::any a)
	{
		t(a);
	};

	template<typename type>
	concept without_payload = requires(type t)
	{
		t();
	};

	class event_handler: public std::function<void(const std::any&)>
	{
	public:
		explicit event_handler(with_payload auto&& Handler):
			function(FWD(Handler))
		{
		}

		explicit event_handler(without_payload auto&& Handler):
			function([Handler = FWD(Handler)](const std::any&) { Handler(); })
		{
		}
	};
}

class message_manager: public singleton<message_manager>
{
	IMPLEMENTS_SINGLETON;

public:
	using handlers_map = unordered_string_multimap<const detail::event_handler*>;

	handlers_map::iterator subscribe(UUID const& EventId, const detail::event_handler& EventHandler);
	handlers_map::iterator subscribe(event_id EventId, const detail::event_handler& EventHandler);
	handlers_map::iterator subscribe(string_view EventName, const detail::event_handler& EventHandler);
	void unsubscribe(handlers_map::iterator HandlerIterator);
	void notify(UUID const& EventId, std::any&& Payload = {});
	void notify(event_id EventId, std::any&& Payload = {});
	void notify(string_view EventId, std::any&& Payload = {});
	bool dispatch();

	void enable_power_notifications();
	void disable_power_notifications();

private:
	struct message
	{
		string Id;
		std::any Payload;
	};

	using message_queue = std::list<message>;

	message_manager();
	~message_manager();

	void commit_add();
	void commit_remove();

	os::concurrency::critical_section
		m_QueueLock,
		m_PendingLock,
		m_ActiveLock;

	message_queue m_Messages;

	handlers_map
		m_PendingHandlers,
		m_ActiveHandlers;

	std::unique_ptr<wm_listener> m_Window;

	std::atomic_size_t m_DispatchInProgress{};
};

class listener: noncopyable
{
public:
	struct scope
	{
		string_view ScopeName;
	};

	listener(const auto& EventId, const auto& EventHandler):
		m_Handler(EventHandler),
		m_Iterator(message_manager::instance().subscribe(EventId, m_Handler))
	{
	}

	explicit listener(scope const Scope, auto const& EventHandler):
		listener(CreateEventName(Scope.ScopeName), EventHandler)
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
	static string CreateEventName(string_view ScopeName);

	detail::event_handler m_Handler;
	message_manager::handlers_map::iterator m_Iterator;
};

#endif // NOTIFICATION_HPP_B0BB0D31_61E8_49C3_AA4F_E8C1D7D71A25
