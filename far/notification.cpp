/*
notification.cpp

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "notification.hpp"

// Internal:
#include "wm_listener.hpp"

// Platform:

// Common:
#include "common/range.hpp"
#include "common/uuid.hpp"

// External:

//----------------------------------------------------------------------------

static const string_view EventNames[]
{
	WSTRVIEW(update_intl),
	WSTRVIEW(update_power),
	WSTRVIEW(update_devices),
	WSTRVIEW(update_environment),
	WSTRVIEW(plugin_synchro),
};

static_assert(std::size(EventNames) == event_id_count);

message_manager::message_manager():
	m_Window(std::make_unique<wm_listener>())
{
}

message_manager::~message_manager() = default;

message_manager::handlers_map::iterator message_manager::subscribe(event_id EventId, const detail::event_handler& EventHandler)
{
	SCOPED_ACTION(std::unique_lock)(m_RWLock);
	return m_Handlers.emplace(EventNames[EventId], &EventHandler);
}

message_manager::handlers_map::iterator message_manager::subscribe(string_view const EventName, const detail::event_handler& EventHandler)
{
	SCOPED_ACTION(std::unique_lock)(m_RWLock);
	return m_Handlers.emplace(EventName, &EventHandler);
}

void message_manager::unsubscribe(handlers_map::iterator HandlerIterator)
{
	SCOPED_ACTION(std::unique_lock)(m_RWLock);
	m_Handlers.erase(std::move(HandlerIterator));
}

void message_manager::notify(event_id EventId, std::any&& Payload)
{
	m_Messages.emplace(EventNames[EventId], std::move(Payload));
}

void message_manager::notify(string_view const EventName, std::any&& Payload)
{
	m_Messages.emplace(EventName, std::move(Payload));
}

bool message_manager::dispatch()
{
	bool Result = false;
	message_queue::value_type EventData;
	while (m_Messages.try_pop(EventData))
	{
		SCOPED_ACTION(std::shared_lock)(m_RWLock);
		const auto RelevantListeners = m_Handlers.equal_range(EventData.first);

		for (const auto& [Name, Instance]: range(RelevantListeners.first, RelevantListeners.second))
		{
			std::invoke(*Instance, EventData.second);
		}

		Result = Result || RelevantListeners.first != RelevantListeners.second;
	}
	m_Window->Check();
	return Result;
}

string listener::CreateEventName()
{
	return uuid::str(os::uuid::generate());
}
