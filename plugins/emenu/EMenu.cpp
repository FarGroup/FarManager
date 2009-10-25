#include "Plugin.h"
#include "OleThread.h"
#include <cassert>

CPlugin *thePlug=NULL;

#if defined(__GNUC__)
  #define DLLMAINFUNC DllMainCRTStartup
#else
  #define DLLMAINFUNC _DllMainCRTStartup
#endif

#ifdef __cplusplus
extern "C" {
#endif
BOOL WINAPI DLLMAINFUNC(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  if (dwReason==DLL_PROCESS_ATTACH)
  {
    if (!DisableThreadLibraryCalls((HINSTANCE)hDll))
    {
      assert(0);
    }
  }

  return TRUE;
}
#ifdef __cplusplus
};
#endif

#ifdef DEBUG
extern "C" void __cdecl main(void) {}
#endif

int WINAPI EXP_NAME(GetMinFarVersion)()
{
  return FARMANAGERVERSION;
}

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  thePlug = new CPlugin(Info);

  OleThread::hNeedInvoke = new CHandle;
  OleThread::hInvokeDone = new CHandle;
  OleThread::hStop = new CHandle;
  OleThread::hTerminator = new OleThread::CThreadTerminator;
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

  delete OleThread::hTerminator;
  delete OleThread::hStop;
  delete OleThread::hInvokeDone;
  delete OleThread::hNeedInvoke;

  delete thePlug;
}
