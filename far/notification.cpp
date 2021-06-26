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
#include "keyboard.hpp"
#include "wm_listener.hpp"
#include "log.hpp"

// Platform:

// Common:
#include "common/scope_exit.hpp"
#include "common/uuid.hpp"

// External:

//----------------------------------------------------------------------------

static const string_view EventNames[]
{
	WSTRVIEW(update_intl),
	WSTRVIEW(update_power),
	WSTRVIEW(update_devices),
	WSTRVIEW(update_environment),
};

static_assert(std::size(EventNames) == event_id_count);

message_manager::message_manager():
	m_Window(std::make_unique<wm_listener>())
{
}

message_manager::~message_manager() = default;

void message_manager::commit_add()
{
	handlers_map PendingHandlers;

	{
		SCOPED_ACTION(std::lock_guard)(m_PendingLock);
		PendingHandlers = std::move(m_PendingHandlers);
	}

	{
		SCOPED_ACTION(std::lock_guard)(m_ActiveLock);
		m_ActiveHandlers.merge(PendingHandlers);
	}
}

void message_manager::commit_remove()
{
	SCOPED_ACTION(std::lock_guard)(m_ActiveLock);
	std::erase_if(m_ActiveHandlers, [](handlers_map::value_type const& Handler)
	{
		return !Handler.second;
	});
}

message_manager::handlers_map::iterator message_manager::subscribe(UUID const& EventId, const detail::event_handler& EventHandler)
{
	SCOPED_ACTION(std::lock_guard)(m_PendingLock);
	return m_PendingHandlers.emplace(uuid::str(EventId), &EventHandler);
}

message_manager::handlers_map::iterator message_manager::subscribe(event_id EventId, const detail::event_handler& EventHandler)
{
	SCOPED_ACTION(std::lock_guard)(m_PendingLock);
	return m_PendingHandlers.emplace(EventNames[EventId], &EventHandler);
}

message_manager::handlers_map::iterator message_manager::subscribe(string_view const EventName, const detail::event_handler& EventHandler)
{
	SCOPED_ACTION(std::lock_guard)(m_PendingLock);
	return m_PendingHandlers.emplace(EventName, &EventHandler);
}

void message_manager::unsubscribe(handlers_map::iterator HandlerIterator)
{
	HandlerIterator->second = {};
}

void message_manager::notify(UUID const& EventId, std::any&& Payload)
{
	m_Messages.emplace(uuid::str(EventId), std::move(Payload));
	main_loop_process_messages();
}

void message_manager::notify(event_id const EventId, std::any&& Payload)
{
	m_Messages.emplace(EventNames[EventId], std::move(Payload));
	main_loop_process_messages();
}

void message_manager::notify(string_view const EventId, std::any&& Payload)
{
	m_Messages.emplace(EventId, std::move(Payload));
	main_loop_process_messages();
}

bool message_manager::dispatch()
{
	bool Result = false;

	if (!m_DispatchInProgress)
	{
		commit_add();
		commit_remove();
	}

	std::unordered_map<const detail::event_handler*, bool> EligibleHandlers;

	message_queue::value_type EventData;
	while (m_Messages.try_pop(EventData))
	{
		const auto& [EventId, Payload] = EventData;

		// For purist clang /o
		// TODO: remove once we enable C++20
		const auto& EventIdRef = EventId;

		const auto find_eligible_from = [&](handlers_map const& Handlers)
		{
			const auto EqualRange = Handlers.equal_range(EventIdRef);
			for (const auto& [Key, Value]: range(EqualRange.first, EqualRange.second))
			{
				EligibleHandlers.emplace(Value, false);
			}
		};

		EligibleHandlers.clear();

		{
			SCOPED_ACTION(std::lock_guard)(m_ActiveLock);
			find_eligible_from(m_ActiveHandlers);
		}

		{
			SCOPED_ACTION(std::lock_guard)(m_PendingLock);
			find_eligible_from(m_PendingHandlers);
		}

		auto HandlerIterator = EligibleHandlers.begin();

		if (EligibleHandlers.empty())
		{
			LOGWARNING(L"No handlers for {}"sv, EventId);
		}

		for (;;)
		{
			if (HandlerIterator == EligibleHandlers.cend())
				break;

			auto& [Handler, IsAlreadyProcessed] = *HandlerIterator;
			++HandlerIterator;

			if (!Handler)
				continue; // Pending remove

			if (IsAlreadyProcessed)
				continue;

			{
				++m_DispatchInProgress;
				SCOPE_EXIT{ --m_DispatchInProgress; };

				std::invoke(*Handler, Payload);
			}

			IsAlreadyProcessed = true;

			if (m_PendingHandlers.empty())
				continue;

			// New handlers have been added by the callback.
			// Them plugins can do dodgy things there.
			// Commit, recalculate the eligibility and restart the loop.
			// Already visited handlers will be ignored.
			commit_add();
			find_eligible_from(m_ActiveHandlers);
			HandlerIterator = EligibleHandlers.begin();
		}


		Result = Result || !EligibleHandlers.empty();
	}

	m_Window->Check();
	return Result;
}

string listener::CreateEventName()
{
	return uuid::str(os::uuid::generate());
}
