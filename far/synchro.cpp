/*
synchro.cpp
синхронизация для плагинов.
*/
/*
Copyright © 2009 Far Group
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
#include "elevation.hpp"
#include "FarGuid.hpp"
#include "ctrlobj.hpp"

PluginSynchro PluginSynchroManager;

PluginSynchro::PluginSynchro()
{
}

PluginSynchro::~PluginSynchro()
{
}

void PluginSynchro::Synchro(bool Plugin, const GUID& PluginId,void* Param)
{
	CriticalSectionLock Lock(CS);
	SynchroData* item=Data.Push();
	item->Plugin=Plugin;
	item->PluginId=PluginId;
	item->Param=Param;
}

bool PluginSynchro::Process(void)
{
	bool res=false;
	bool process=false; bool plugin=false; GUID PluginId=FarGuid; void* param=nullptr;
	{
		CriticalSectionLock Lock(CS);
		SynchroData* item=Data.First();

		if (item)
		{
			process=true;
			plugin=item->Plugin;
			PluginId=item->PluginId;
			param=item->Param;
			Data.Delete(item);
		}
	}

	if (process)
	{
		if(plugin)
		{
			Plugin* pPlugin=CtrlObject?CtrlObject->Plugins->FindPlugin(PluginId):nullptr;

			if (pPlugin)
			{
				pPlugin->ProcessSynchroEvent(SE_COMMONSYNCHRO,param);
				res=true;
			}
		}
		else
		{
			ElevationApproveDlgSync(param);
			res=true;
		}
	}


	return res;
}
