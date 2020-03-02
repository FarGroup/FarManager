#include "Plugin.h"
#include "OleThread.h"
#include <cassert>
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"

CPlugin *thePlug=NULL;

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=PLUGIN_VERSION;
  Info->Guid=MainGuid;
  Info->Title=PLUGIN_NAME;
  Info->Description=PLUGIN_DESC;
  Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  thePlug = new CPlugin(Info);

  OleThread::Startup();
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  thePlug->GetPluginInfo(Info);
}

HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
  return thePlug->OpenPlugin(OInfo->OpenFrom, OInfo->Data);
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* Info)
{
  return thePlug->Configure();
}

void WINAPI ExitFARW(const ExitInfo* Info)
{
  thePlug->ExitFAR();

  OleThread::Cleanup();

  delete thePlug;
}
