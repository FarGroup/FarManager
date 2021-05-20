﻿#include "OleThread.h"
#include <strsafe.h>
#include <windows.h>
#include <comdef.h>
#include <cassert>
#include "Handle.h"
#include "FarMenu.h"

namespace OleThread
{
  static CHandle *hNeedInvoke{};
  static CHandle *hInvokeDone{};
  static CHandle *hStop{};
  static CHandle *hShowMenu{};
  static CHandle *hMenuDone{};
  static CThreadTerminator *hTerminator;

  namespace OpenPluginArgs
  {
    static int nOpenFrom;
    static INT_PTR nItem;
    static CPlugin::EDoMenu enRes;
  }

  namespace ShowMenuArgs
  {
    static CFarMenu *Menu;
    static LPCWSTR szTitle;
    static int nSelItem;
    static bool bAtCursorPos;
    static int Res;
  }

  static DWORD WINAPI ThreadProc(LPVOID)
  {
    //HRESULT hr=CoInitializeEx(0, COINIT_MULTITHREADED);//! COINIT_APARTMENTTHREADED
    if (FAILED(OleInitialize({})))
    {
      assert(0);
    }
    DWORD nWaitTime=100;
    for(;;)
    {
      HANDLE phEvents[2]={*hStop, *hNeedInvoke};
      DWORD nRes=WaitForMultipleObjects(2, phEvents, FALSE, nWaitTime);
      if (WAIT_OBJECT_0==nRes) break;
      if (WAIT_OBJECT_0+1==nRes)
      {
        OpenPluginArgs::enRes=thePlug->OpenPluginBkg(OpenPluginArgs::nOpenFrom, OpenPluginArgs::nItem);
        if (!SetEvent(*hInvokeDone))
        {
          assert(0);
        }
//break;//!
      }

      // * Объясняю зачем  здесь ловить собщения *
      // Если в Build950, OSR2 или в ME скопировать/вырезать файл
      // в буфер. а потом не выходя из FARа попробовать вызвать
      // контекстное меню для какой-нибудь папки в эксплорере
      // , то FARу бедет послано сообщение и до тех пор пока оно
      // не обработается меню не покажется.
      MSG msg;
      if (PeekMessage(&msg, {}, 0, 0, PM_NOREMOVE))
      {
        GetMessage(&msg, {}, 0, 0);
        if (msg.message>=WM_USER)
        {
          DispatchMessage(&msg);
        }
        else
        {
          ReplyMessage(0);
        }
        nWaitTime=0;
      }
      else
      {
        if (nWaitTime<100) nWaitTime+=1;
      }
    }
//    В Build950 вызывает падение
//    OleUninitialize();
    return 0;
  }

  static HANDLE hThread{};

  static bool EnsureThreadStarted()
  {
    if (!*hNeedInvoke) *hNeedInvoke=CreateEvent({}, FALSE, FALSE, {});
    if (!*hInvokeDone) *hInvokeDone=CreateEvent({}, FALSE, FALSE, {});
    if (!*hStop) *hStop=CreateEvent({}, FALSE, FALSE, {});
    if (!*hShowMenu) *hShowMenu=CreateEvent({}, FALSE, FALSE, {});
    if (!*hMenuDone) *hMenuDone=CreateEvent({}, FALSE, FALSE, {});
    if (!*hNeedInvoke || !*hInvokeDone || !*hStop || !*hShowMenu || !*hMenuDone)
    {
      assert(0);
      return false;
    }
    DWORD nStatus;
    if (hThread && GetExitCodeThread(hThread, &nStatus))
    {
      if (STILL_ACTIVE==nStatus) return true;
    }
    DWORD nId;
    hThread=CreateThread({}, 0, ThreadProc, {}, 0, &nId);
    return (hThread != nullptr);
  }

  CPlugin::EDoMenu OpenPlugin(int nOpenFrom, INT_PTR nItem)
  {
    OpenPluginArgs::nOpenFrom=nOpenFrom;
    OpenPluginArgs::nItem=nItem;
    if (!EnsureThreadStarted())
    {
      assert(0);
      return CPlugin::DOMNU_ERR_SHOW;
    }
    if (!SetEvent(*hNeedInvoke))
    {
      assert(0);
      return CPlugin::DOMNU_ERR_SHOW;
    }
    HANDLE phEvents[2]={*hInvokeDone, *hShowMenu};
    DWORD nRes;
    do
    {
      nRes=WaitForMultipleObjects(2, phEvents, FALSE, INFINITE);
      if (WAIT_OBJECT_0+1==nRes)
      {
        ShowMenuArgs::Res = ShowMenuArgs::Menu->Show(ShowMenuArgs::szTitle, ShowMenuArgs::nSelItem, ShowMenuArgs::bAtCursorPos);
        if (!SetEvent(*hMenuDone))
        {
          assert(0);
        }
      }
    } while (nRes != WAIT_OBJECT_0);
    return OpenPluginArgs::enRes;
  }

  int ShowMenu(CFarMenu &Menu, LPCWSTR szTitle, int nSelItem, bool bAtCursorPos)
  {
    ShowMenuArgs::Menu = &Menu;
    ShowMenuArgs::szTitle = szTitle;
    ShowMenuArgs::nSelItem = nSelItem;
    ShowMenuArgs::bAtCursorPos = bAtCursorPos;

    if (!SetEvent(*hShowMenu))
    {
      assert(0);
      return -1;
    }
    if (WAIT_OBJECT_0!=WaitForSingleObject(*hMenuDone, INFINITE))
    {
      assert(0);
    }
    return ShowMenuArgs::Res;
  }

  void Stop()
  {
    if (!hThread) return;
    if (!SetEvent(*hStop))
    {
      assert(0);
    }
  }

  void Startup()
  {
    hNeedInvoke = new CHandle;
    hInvokeDone = new CHandle;
    hStop = new CHandle;
    hShowMenu = new CHandle;
    hMenuDone = new CHandle;
    hTerminator = new OleThread::CThreadTerminator;
  }

  void Cleanup()
  {
    delete hTerminator;
    delete hMenuDone;
    delete hShowMenu;
    delete hStop;
    delete hInvokeDone;
    delete hNeedInvoke;
  }


  CThreadTerminator::~CThreadTerminator()
  {
    if (!hThread) return;
    if (WaitForSingleObject(hThread, 100)==WAIT_TIMEOUT)
    {
      if (!TerminateThread(hThread, 0))
      {
        assert(0);
      }
    }
    if (!CloseHandle(hThread))
    {
      assert(0);
    }
  }
}
