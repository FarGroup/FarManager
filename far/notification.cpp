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

listener::listener(const string& id, const std::function<void()>& function):
	m_notification(Notifier().at(id)),
	m_function(function)
{
	m_notification.subscribe(this);
}

listener::listener(const std::function<void(const payload&)>& function, const string& id):
m_notification(Notifier().at(id)),
m_ex_function(function)
{
	m_notification.subscribe(this);
}

listener::~listener()
{
	m_notification.unsubscribe(this);
}

void notification::notify(std::unique_ptr<const payload>&& p)
{
	m_events.Push(std::move(p));
}

void notification::dispatch()
{
	decltype(m_events)::value_type item;

	while(m_events.PopIfNotEmpty(item))
	{
		std::for_each(RANGE(m_listeners, i)
		{
			i->callback(*item);
		});
	}
}

notifier& Notifier()
{
	static notifier n;
	return n;
}

notifier::notifier():
	m_Window(std::make_unique<wm_listener>(this))
{
}

void notifier::dispatch()
{
	std::for_each(RANGE(m_notifications, i)
	{
		i.second->dispatch();
	});

	m_Window->Check();
}

void notifier::add(std::unique_ptr<notification>&& i)
{
	auto Name = i->name();
	m_notifications.insert(VALUE_TYPE(m_notifications)(std::move(Name), std::move(i)));
}
