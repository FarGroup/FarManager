#include "Plugin.h"
#include "OleThread.h"
#include <cassert>

CPlugin *thePlug=NULL;

extern "C" BOOL WINAPI _DllMainCRTStartup(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
  if (dwReason==DLL_PROCESS_ATTACH)
  {
    thePlug = new CPlugin;
    thePlug->m_hModule=(HINSTANCE)hDllHandle;
    if (!DisableThreadLibraryCalls((HINSTANCE)hDllHandle))
    {
      assert(0);
    }
    OleThread::hNeedInvoke = new CHandle;
    OleThread::hInvokeDone = new CHandle;
    OleThread::hStop = new CHandle;
    OleThread::hTerminator = new OleThread::CThreadTerminator;
  }
  else if (dwReason==DLL_PROCESS_DETACH)
  {
    delete OleThread::hTerminator;
    delete OleThread::hStop;
    delete OleThread::hInvokeDone;
    delete OleThread::hNeedInvoke;
    delete thePlug;
  }
  return TRUE;
}

int WINAPI _export GetMinFarVersion()
{
  return thePlug->GetMinFarVersion();
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  thePlug->SetStartupInfo(Info);
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  thePlug->GetPluginInfo(Info);
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom, INT_PTR Item)
{
  return thePlug->OpenPlugin(OpenFrom, Item);
}

int WINAPI _export Configure(int)
{
  return thePlug->Configure();
}

void WINAPI _export ExitFAR()
{
  thePlug->ExitFAR();
}
