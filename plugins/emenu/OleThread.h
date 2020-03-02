#ifndef _OLETHREAD_H_
#define _OLETHREAD_H_

#include "Plugin.h"
#include "Handle.h"
class CFarMenu;

namespace OleThread
{
  class CThreadTerminator
  {
  public:
    ~CThreadTerminator();
  };

  CPlugin::EDoMenu OpenPlugin(int nOpenFrom, INT_PTR nItem);
  int ShowMenu(CFarMenu &Menu, LPCWSTR szTitle, int nSelItem=0, bool bAtCursorPos=false);

  void Stop();
  void Startup();
  void Cleanup();
};

#endif
