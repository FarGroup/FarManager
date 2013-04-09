/*
synchro.cpp

Критические секции, мютексы, эвенты и т.п.
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "synchro.hpp"

class Thread::ThreadParam
{
public:
	ThreadParam(Thread::ThreadOwner* Owner, Thread::ThreadHandlerFunction HandlerFunction, void* Parameter):
		m_Owner(Owner),
		m_HandlerFunction(HandlerFunction),
		m_Parameter(Parameter)
	{}
	Thread::ThreadOwner* Owner() {return m_Owner;}
	Thread::ThreadHandlerFunction HandlerFunction() {return m_HandlerFunction;}
	void* Parameter() {return m_Parameter;}
private:
	Thread::ThreadOwner* m_Owner;
	Thread::ThreadHandlerFunction m_HandlerFunction;
	void* m_Parameter;
};

unsigned int WINAPI ThreadHandler(void* Parameter)
{
	auto pParam = reinterpret_cast<Thread::ThreadParam*>(Parameter);
	Thread::ThreadParam Param = *pParam;
	delete pParam;
	DWORD Result = (Param.Owner()->*Param.HandlerFunction())(Param.Parameter());
	return Result;
}

HANDLE Thread::CreateMemberThread(LPSECURITY_ATTRIBUTES ThreadAttributes, unsigned int StackSize, Thread::ThreadOwner* Owner, Thread::ThreadHandlerFunction HandlerFunction, void* Parameter, DWORD CreationFlags, unsigned int* ThreadId)
{
	ThreadParam* p = new ThreadParam(Owner, HandlerFunction, Parameter);
	return reinterpret_cast<HANDLE>(_beginthreadex(ThreadAttributes, StackSize, ThreadHandler, p, CreationFlags, ThreadId));
}