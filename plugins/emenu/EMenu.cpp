#include "Plugin.h"
#include "OleThread.h"
#include <cassert>

#ifndef UNICODE
#define EXP_NAME(p) _export p
#else
#define EXP_NAME(p) _export p ## W
#endif

CPlugin *thePlug=NULL;

#ifdef DEBUG
extern "C" void __cdecl main(void) {}
#endif

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

int WINAPI EXP_NAME(GetMinFarVersion)()
{
  return thePlug->GetMinFarVersion();
}

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  thePlug->SetStartupInfo(Info);
}

void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  thePlug->GetPluginInfo(Info);
}

HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item)
{
  return thePlug->OpenPlugin(OpenFrom, Item);
}

int WINAPI EXP_NAME(Configure)(int)
{
  return thePlug->Configure();
}

void WINAPI EXP_NAME(ExitFAR)()
{
  thePlug->ExitFAR();
}
