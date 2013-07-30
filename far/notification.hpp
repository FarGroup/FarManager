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

#include "synchro.hpp"

class payload
{
public:
	virtual ~payload() {}
};

class inotification;

class ilistener
{
public:
	ilistener(inotification& n);
	virtual ~ilistener();

	virtual void callback(const payload& p) = 0;

private:
	inotification& m_notification;
};

class listener : public ilistener
{
public:
	listener(const string& id, std::function<void()> function);
	listener(std::function<void(const payload&)> function, const string& id);
	virtual void callback(const payload& p) { m_ex_function? m_ex_function(p) : m_function(); }

private:
	std::function<void()> m_function;
	std::function<void(const payload&)> m_ex_function;
};

class inotification
{
public:
	virtual ~inotification() {}

	void notify(const payload* p);
	void dispatch();
	void subscribe(ilistener* l) { m_listeners.emplace_back(l); }
	void unsubscribe(ilistener* l) { m_listeners.remove(l); }
	const string& name() const { return m_name; }

protected:
	inotification(const string& name) : m_name(name) {}

private:
	string m_name;
	std::list<ilistener*> m_listeners;
	SyncedQueue<std::unique_ptr<const payload>> m_events;
};


class notification : public inotification
{
public:
	notification(const string& name) : inotification(name) {}
};

class notifier
{
public:
	inotification& at(const string& key) { return *m_notifications.at(key).get(); }

	void dispatch();
	void add(inotification* i);

private:
	std::map<string, std::unique_ptr<inotification>> m_notifications;
};
