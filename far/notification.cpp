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

#include "headers.hpp"
#pragma hdrstop

#include "notification.hpp"
#include "wm_listener.hpp"

static const wchar_t* EventNames[] =
{
	WSTR(update_intl),
	WSTR(update_power),
	WSTR(update_devices),
	WSTR(update_environment),
	WSTR(elevation_dialog),
	WSTR(plugin_synchro),
};

static_assert(std::size(EventNames) == event_id_count, "Incomplete EventNames array");

message_manager::message_manager():
	m_Window(std::make_unique<wm_listener>()),
	m_suppressions()
{
}

message_manager::handlers_map::iterator message_manager::subscribe(event_id EventId, const detail::i_event_handler& EventHandler)
{
	return m_Handlers.emplace(EventNames[EventId], &EventHandler);
}

message_manager::handlers_map::iterator message_manager::subscribe(const string& EventName, const detail::i_event_handler& EventHandler)
{
	return m_Handlers.emplace(EventName, &EventHandler);
}

void message_manager::unsubscribe(handlers_map::iterator HandlerIterator)
{
	m_Handlers.erase(HandlerIterator);
}

void message_manager::notify(event_id EventId, any&& Payload)
{
	m_Messages.Push(message_queue::value_type(EventNames[EventId], std::move(Payload)));
}

void message_manager::notify(const string& EventName, any&& Payload)
{
	m_Messages.Push(message_queue::value_type(EventName, std::move(Payload)));
}

bool message_manager::dispatch()
{
	bool Result = false;
	message_queue::value_type EventData;
	while (m_Messages.PopIfNotEmpty(EventData))
	{
		const auto RelevantListeners = m_Handlers.equal_range(EventData.first);
		std::for_each(RelevantListeners.first, RelevantListeners.second, [&](const handlers_map::value_type& i)
		{
			(*i.second)(EventData.second);
		});
		Result = Result || RelevantListeners.first != RelevantListeners.second;
	}
	m_Window->Check();
	return Result;
}

message_manager::suppress::suppress():
	m_owner(MessageManager())
{
	++m_owner.m_suppressions;
}

message_manager::suppress::~suppress()
{
	--m_owner.m_suppressions;
}

message_manager& MessageManager()
{
	static message_manager sMessageManager;
	return sMessageManager;
}
