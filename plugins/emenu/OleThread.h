#ifndef _OLETHREAD_H_
#define _OLETHREAD_H_

#include "Plugin.h"
#include "Handle.h"

namespace OleThread
{
  class CThreadTerminator
  {
  public:
    ~CThreadTerminator();
  };

  extern CHandle *hNeedInvoke;
  extern CHandle *hInvokeDone;
  extern CHandle *hStop;
  extern CThreadTerminator *hTerminator;

  CPlugin::EDoMenu OpenPlugin(int nOpenFrom, int nItem);
  void Stop();
};

#endif
