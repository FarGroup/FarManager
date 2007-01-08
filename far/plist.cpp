/*
plist.cpp

Список процессов (Ctrl-W)

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
  VMenu ProcList(UMSG(MProcessListTitle),NULL,0,TRUE, ScrY-4);
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

      case KEY_DEL:
        {
          BlockExtKey blockExtKey;

          // Полиция 21
          if(Opt.Policies.DisabledOptions&FFPOL_KILLTASK)
          {
            MessageW(MSG_WARNING,1,UMSG(MKillProcessTitle),UMSG(MCannotKillProcessPerm),UMSG(MOk));
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
            if (MessageW(MSG_WARNING,2,UMSG(MKillProcessTitle),UMSG(MAskKillProcess),
                        strWinTitle,UMSG(MKillProcessWarning),UMSG(MKillProcessKill),UMSG(MCancel))==0)
              if (KillProcess(ProcID))
              {
                Sleep(500);
                ProcList.Hide();
                ShowProcessList();
                return;
              }
              else
                MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MKillProcessTitle),UMSG(MCannotKillProcess),UMSG(MOk));
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
      ProcList->SetUserData((void*)hwnd,sizeof(hwnd),ProcList->AddItemW(&ListItem));
    }
  }
  return(TRUE);
}
