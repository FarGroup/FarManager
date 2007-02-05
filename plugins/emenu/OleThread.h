#ifndef _OLETHREAD_H_
#define _OLETHREAD_H_

#include "Plugin.h"
#include "Handle.h"

namespace OleThread
{
  extern CHandle *hNeedInvoke;
  extern CHandle *hInvokeDone;
  extern CHandle *hStop;

  CPlugin::EDoMenu OpenPlugin(int nOpenFrom, int nItem);
  void Stop();
};

#endif
