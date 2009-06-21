/*
synchro.cpp
синхронизация для плагинов.
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2009 Far Group
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
#include "plclass.hpp"
#include "plugin.hpp"

PluginSynchro PluginSynchroManager;

PluginSynchro::PluginSynchro()
{
	Mutex=CreateMutex(NULL,FALSE,NULL);
}

PluginSynchro::~PluginSynchro()
{
	CloseHandle(Mutex);
}

void PluginSynchro::Synchro(INT_PTR ModuleNumber,void* Param)
{
	if(Mutex)
	{
		WaitForSingleObject(Mutex,INFINITE);
		SynchroData* item=Data.Push();
		item->ModuleNumber=ModuleNumber;
		item->Param=Param;
		ReleaseMutex(Mutex);
	}
}

void PluginSynchro::Process(void)
{
	if(Mutex)
	{
		bool process=false; INT_PTR module=0; void* param=NULL;
		WaitForSingleObject(Mutex,INFINITE);
		SynchroData* item=Data.First();
		if(item)
		{
			process=true;
			module=item->ModuleNumber;
			param=item->Param;
			Data.Delete(item);
		}
		ReleaseMutex(Mutex);
		if(process)
		{
			Plugin* pPlugin=(Plugin*)module;
			if(pPlugin) pPlugin->ProcessSynchroEvent(SE_COMMONSYNCHRO,param);
		}
	}
}
