/*
plist.cpp

—писок процессов (Ctrl-W)

*/

/* Revision: 1.05 03.06.2001 $ */

/*
Modify:
  03.06.2001 SVS
    ! »зменени€ в св€зи с переделкой UserData в VMenu
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      ѕол€ Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    !  онстанты MENU_ - в морг
  06.05.2001 DJ
    ! перетр€х #include
  11.02.2001 SVS
    ! Ќесколько уточнений кода в св€зи с изменени€ми в структуре MenuItem
  31.01.2001 IS
    ! “еперь это меню более дружелюбно - попытка перемещени€ курсора выше
      первого пункта или ниже последнего будет приводить к перемещению
      соответственно к последнему или к первому пункту.
  25.06.2000 SVS
    ! ѕодготовка Master Copy
    ! ¬ыделение в качестве самосто€тельного модул€
*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "vmenu.hpp"

static BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam);
static BOOL KillProcess(DWORD dwPID);

void ShowProcessList()
{
  VMenu ProcList(MSG(MProcessListTitle),NULL,0,ScrY-4);
  /* $ 31.01.2001 IS
     ! “еперь это меню более дружелюбно ;)
  */
  ProcList.SetFlags(VMENU_WRAPMODE);
  /* IS $ */
  ProcList.SetHelp("TaskList");
  ProcList.SetPosition(-1,-1,0,0);
  if (!EnumWindows(EnumWindowsProc,(LPARAM)&ProcList))
    return;
  ProcList.AssignHighlights(FALSE);
  ProcList.Show();

  while (!ProcList.Done())
  {
    int Key=ProcList.ReadInput();
    switch(Key)
    {
      case KEY_DEL:
        {
          HWND ProcWnd=(HWND)ProcList.GetUserData(NULL,0);
          if (ProcWnd!=NULL)
          {
            char WinTitle[512];
            GetWindowText(ProcWnd,WinTitle,sizeof(WinTitle));
            CharToOem(WinTitle,WinTitle);
            DWORD ProcID;
            GetWindowThreadProcessId(ProcWnd,&ProcID);
            if (Message(MSG_WARNING,2,MSG(MKillProcessTitle),MSG(MAskKillProcess),
                        WinTitle,MSG(MKillProcessWarning),MSG(MKillProcessKill),MSG(MCancel))==0)
              if (KillProcess(ProcID))
              {
                Sleep(500);
                ProcList.Hide();
                ShowProcessList();
                return;
              }
              else
                Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MKillProcessTitle),MSG(MCannotKillProcess),MSG(MOk));
          }
        }
        break;
      default:
        ProcList.ProcessInput();
        break;
    }
  }
  if (ProcList.GetExitCode()>=0)
  {
    HWND ProcWnd=(HWND)ProcList.GetUserData(NULL,0);
    if (ProcWnd!=NULL)
    {
      SetForegroundWindow(ProcWnd);

      WINDOWPLACEMENT wp;
      wp.length=sizeof(wp);
      if (!GetWindowPlacement(ProcWnd,&wp) || wp.showCmd!=SW_SHOWMAXIMIZED)
        ShowWindowAsync(ProcWnd,SW_RESTORE);
    }
  }
}


BOOL KillProcess(DWORD dwPID)
{
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
      IsIconic(hwnd) && (GetWindowLong(hwnd,GWL_STYLE) & WS_DISABLED)==0)
  {
    char Title[256];
    GetWindowText(hwnd,Title,sizeof(Title));
    if (*Title)
    {
      struct MenuItem ListItem;
      memset(&ListItem,0,sizeof(ListItem));
      TruncStr(Title,sizeof(ListItem.Name)-1);
      sprintf(ListItem.Name,"%-25s",Title);
      CharToOem(ListItem.Name,ListItem.Name);
      ProcList->SetUserData((void*)hwnd,sizeof(hwnd),ProcList->AddItem(&ListItem));
    }
  }
  return(TRUE);
}
