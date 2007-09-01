/*
plist.cpp

Список процессов (Ctrl-W)
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "global.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "help.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "BlockExtKey.hpp"

static BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam);
static BOOL KillProcess(DWORD dwPID);

void ShowProcessList()
{
  VMenu ProcList(UMSG(MProcessListTitle),NULL,0,ScrY-4);
  /* $ 31.01.2001 IS
     ! Теперь это меню более дружелюбно ;)
  */
  ProcList.SetFlags(VMENU_WRAPMODE);
  /* IS $ */
//  ProcList.SetHelp("TaskList");
  ProcList.SetPosition(-1,-1,0,0);
  if (!EnumWindows(EnumWindowsProc,(LPARAM)&ProcList))
    return;
  ProcList.AssignHighlights(FALSE);
  ProcList.SetBottomTitle(UMSG(MProcessListBottom));
  ProcList.Show();

  while (!ProcList.Done())
  {
    int Key=ProcList.ReadInput();
    switch(Key)
    {
      case KEY_F1:
      {
        BlockExtKey blockExtKey;
        {
          Help Hlp (L"TaskList");
        }
        break;
      }

      case KEY_CTRLR:
      {
        ProcList.Hide();
        ProcList.DeleteItems();
        ProcList.SetPosition(-1,-1,0,0);
        if (!EnumWindows(EnumWindowsProc,(LPARAM)&ProcList))
        {
          ProcList.Modal::SetExitCode(-1);
          break;
        }
        ProcList.Show();
        break;
      }

      case KEY_NUMDEL:
      case KEY_DEL:
        {
          BlockExtKey blockExtKey;

          // Полиция 21
          if(Opt.Policies.DisabledOptions&FFPOL_KILLTASK)
          {
            Message(MSG_WARNING,1,UMSG(MKillProcessTitle),UMSG(MCannotKillProcessPerm),UMSG(MOk));
            break;
          }

          HWND ProcWnd=(HWND)ProcList.GetUserData(NULL,0);
          if (ProcWnd!=NULL)
          {
            string strWinTitle;

            wchar_t *lpwszTitle = strWinTitle.GetBuffer (NM*2);

            GetWindowTextW(ProcWnd, lpwszTitle, NM*2);

            strWinTitle.ReleaseBuffer ();

            DWORD ProcID;
            GetWindowThreadProcessId(ProcWnd,&ProcID);
            if (Message(MSG_WARNING,2,UMSG(MKillProcessTitle),UMSG(MAskKillProcess),
                        strWinTitle,UMSG(MKillProcessWarning),UMSG(MKillProcessKill),UMSG(MCancel))==0)
              if (KillProcess(ProcID))
              {
                Sleep(500);
                ProcList.Hide();
                ShowProcessList();
                return;
              }
              else
                Message(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MKillProcessTitle),UMSG(MCannotKillProcess),UMSG(MOk));
          }
        }
        break;
      default:
        ProcList.ProcessInput();
        break;
    }
  }
  if (ProcList.Modal::GetExitCode()>=0)
  {
    HWND ProcWnd=(HWND)ProcList.GetUserData(NULL,0);
    if (ProcWnd!=NULL)
    {
      //SetForegroundWindow(ProcWnd);
      #ifndef SPI_GETFOREGROUNDLOCKTIMEOUT
      #define SPI_GETFOREGROUNDLOCKTIMEOUT        0x2000
      #define SPI_SETFOREGROUNDLOCKTIMEOUT        0x2001
      #endif

      // Allow SetForegroundWindow on Win98+.
      DWORD dwMs;
      // Remember the current value.
      BOOL bSPI = SystemParametersInfoW(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &dwMs, 0);
      if(bSPI) // Reset foreground lock timeout
        bSPI = SystemParametersInfoW(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0, 0);
      SetForegroundWindow(ProcWnd);
      if(bSPI) // Restore old value
        SystemParametersInfoW(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)(DWORD_PTR)dwMs, 0);

      WINDOWPLACEMENT wp;
      wp.length=sizeof(wp);
      if (!GetWindowPlacement(ProcWnd,&wp) || wp.showCmd!=SW_SHOWMAXIMIZED)
        ShowWindowAsync(ProcWnd,SW_RESTORE);
    }
  }
}


BOOL KillProcess(DWORD dwPID)
{
  // Полиция 21
  if(Opt.Policies.DisabledOptions&FFPOL_KILLTASK)
   return FALSE;

  HANDLE hProcess;
  BOOL bRet;
  hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,dwPID);
  if (hProcess!=NULL)
  {
    bRet=TerminateProcess(hProcess,0xFFFFFFFF);
    if (bRet)
      WaitForSingleObject(hProcess,5000);
    CloseHandle(hProcess);
  }
  else
    bRet=FALSE;
  return(bRet);
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam)
{
  VMenu *ProcList=(VMenu *)lParam;
  if (IsWindowVisible(hwnd) ||
      IsIconic(hwnd) && (GetWindowLongW(hwnd,GWL_STYLE) & WS_DISABLED)==0)
  {
    string strTitle;

    wchar_t *lpwszTitle = strTitle.GetBuffer (NM*2);

    GetWindowTextW (hwnd, lpwszTitle, NM*2);

    strTitle.ReleaseBuffer ();

    if ( !strTitle.IsEmpty() )
    {
      MenuItemEx ListItem;

      ListItem.Clear ();
      ListItem.strName = strTitle;
      ProcList->SetUserData((void*)hwnd,sizeof(hwnd),ProcList->AddItem(&ListItem));
    }
  }
  return(TRUE);
}
