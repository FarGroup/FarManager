#include <cassert>

#include <windows.h>

#include "OleThread.h"
#include "Handle.h"
#include "FarMenu.h"

namespace OleThread
{
  static CHandle *hThreadReady{};
  static CHandle *hInvokeDone{};
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

  static const auto WM_EMENU_INVOKE = WM_APP + 0x10;

  static DWORD WINAPI ThreadProc(LPVOID)
  {
    //HRESULT hr=CoInitializeEx(0, COINIT_MULTITHREADED);//! COINIT_APARTMENTTHREADED
    if (FAILED(OleInitialize({})))
    {
      assert(0);
    }

    {
      // Force the system to create the message queue
      // see https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postthreadmessagew#remarks
      MSG msg;
      PeekMessage(&msg, {}, WM_USER, WM_USER, PM_NOREMOVE);
    }

    if (!SetEvent(*hThreadReady))
    {
      assert(0);
    }

    for(;;)
    {
      MSG msg;
      if (!GetMessage(&msg, {}, 0, 0))
        break;

      if (msg.message == WM_EMENU_INVOKE)
      {
        OpenPluginArgs::enRes=thePlug->OpenPluginBkg(OpenPluginArgs::nOpenFrom, OpenPluginArgs::nItem);
        if (!SetEvent(*hInvokeDone))
        {
          assert(0);
        }
      }
      else
      // * Объясняю зачем  здесь ловить собщения *
      // Если в Build950, OSR2 или в ME скопировать/вырезать файл
      // в буфер. а потом не выходя из FARа попробовать вызвать
      // контекстное меню для какой-нибудь папки в эксплорере
      // , то FARу бедет послано сообщение и до тех пор пока оно
      // не обработается меню не покажется.
        if (msg.message>=WM_USER)
        {
          DispatchMessage(&msg);
        }
        else
        {
          ReplyMessage(0);
        }
    }
//    В Build950 вызывает падение
//    OleUninitialize();
    return 0;
  }

  static HANDLE hThread{};
  static DWORD nId;

  static bool EnsureThreadStarted()
  {
    if (!*hThreadReady) *hThreadReady=CreateEvent({}, FALSE, FALSE, {});
    if (!*hInvokeDone) *hInvokeDone=CreateEvent({}, FALSE, FALSE, {});
    if (!*hShowMenu) *hShowMenu=CreateEvent({}, FALSE, FALSE, {});
    if (!*hMenuDone) *hMenuDone=CreateEvent({}, FALSE, FALSE, {});
    if (!*hThreadReady || !*hInvokeDone || !*hShowMenu || !*hMenuDone)
    {
      assert(0);
      return false;
    }
    DWORD nStatus;
    if (hThread && GetExitCodeThread(hThread, &nStatus))
    {
      if (STILL_ACTIVE==nStatus) return true;
    }
    hThread=CreateThread({}, 0, ThreadProc, {}, 0, &nId);
    if (!hThread)
    {
      assert(0);
      return false;
    }

    if (WaitForSingleObject(*hThreadReady, INFINITE) != WAIT_OBJECT_0)
    {
      assert(0);
      return false;
    }

    return true;
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
    if (!PostThreadMessage(nId, WM_EMENU_INVOKE, {}, {}))
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
    if (!PostThreadMessage(nId, WM_QUIT, {}, {}))
    {
      assert(0);
    }
  }

  void Startup()
  {
    hThreadReady = new CHandle;
    hInvokeDone = new CHandle;
    hShowMenu = new CHandle;
    hMenuDone = new CHandle;
    hTerminator = new OleThread::CThreadTerminator;
  }

  void Cleanup()
  {
    delete hTerminator;
    delete hMenuDone;
    delete hShowMenu;
    delete hInvokeDone;
    delete hThreadReady;
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
