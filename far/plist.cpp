/*
plist.cpp

Список процессов (Ctrl-W)
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "plist.hpp"
#include "keys.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "language.hpp"
#include "message.hpp"
#include "config.hpp"
#include "interf.hpp"

static BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam);
static BOOL KillProcess(DWORD dwPID);

void ShowProcessList()
{
	VMenu ProcList(MSG(MProcessListTitle),nullptr,0,ScrY-4);
	ProcList.SetFlags(VMENU_WRAPMODE);
	ProcList.SetPosition(-1,-1,0,0);

	if (!EnumWindows(EnumWindowsProc,(LPARAM)&ProcList))
		return;

	ProcList.AssignHighlights(FALSE);
	ProcList.SetBottomTitle(MSG(MProcessListBottom));
	ProcList.Show();

	while (!ProcList.Done())
	{
		int Key=ProcList.ReadInput();

		switch (Key)
		{
			case KEY_F1:
			{
				Help Hlp(L"TaskList");
				break;
			}
			case KEY_CTRLR:
			case KEY_RCTRLR:
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
				// Полиция 21
				if (Opt.Policies.DisabledOptions&FFPOL_KILLTASK)
				{
					Message(MSG_WARNING,1,MSG(MKillProcessTitle),MSG(MCannotKillProcessPerm),MSG(MOk));
					break;
				}

				HWND ProcWnd=*static_cast<HWND*>(ProcList.GetUserData(nullptr,0));

				if (ProcWnd)
				{
					wchar_t *lpwszTitle=0;
					int LenTitle=GetWindowTextLength(ProcWnd);

					if (LenTitle)
					{
						lpwszTitle=(wchar_t *)xf_malloc((LenTitle+1)*sizeof(wchar_t));

						if (lpwszTitle && (LenTitle=GetWindowText(ProcWnd,lpwszTitle,LenTitle+1)))
							lpwszTitle[LenTitle]=0;
					}

					DWORD ProcID;
					GetWindowThreadProcessId(ProcWnd,&ProcID);

					if (!Message(MSG_WARNING,2,MSG(MKillProcessTitle),MSG(MAskKillProcess),
					            lpwszTitle?lpwszTitle:L"",MSG(MKillProcessWarning),MSG(MKillProcessKill),MSG(MCancel)))
					{
						if (KillProcess(ProcID))
						{
							Sleep(500);
							ProcList.Hide();
							ShowProcessList();

							if (lpwszTitle) xf_free(lpwszTitle);

							return;
						}
						else
							Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MKillProcessTitle),MSG(MCannotKillProcess),MSG(MOk));
					}

					if (lpwszTitle) xf_free(lpwszTitle);
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
		HWND ProcWnd=*static_cast<HWND*>(ProcList.GetUserData(nullptr,0));

		if (ProcWnd)
		{
			//SetForegroundWindow(ProcWnd);
			// Allow SetForegroundWindow on Win98+.
			DWORD dwMs;
			// Remember the current value.
			BOOL bSPI = SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &dwMs, 0);

			if (bSPI) // Reset foreground lock timeout
				bSPI = SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0, 0);

			SetForegroundWindow(ProcWnd);

			if (bSPI) // Restore old value
				SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, ToPtr(dwMs), 0);

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
	if (Opt.Policies.DisabledOptions&FFPOL_KILLTASK)
		return FALSE;

	HANDLE hProcess;
	BOOL bRet;
	hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,dwPID);

	if (hProcess)
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
	        (IsIconic(hwnd) && !(GetWindowLongPtr(hwnd,GWL_STYLE) & WS_DISABLED)))
	{
		int LenTitle=GetWindowTextLength(hwnd);

		if (LenTitle)
		{
			wchar_t *lpwszTitle=(wchar_t *)xf_malloc((LenTitle+1)*sizeof(wchar_t));

			if (lpwszTitle)
			{
				if ((LenTitle=GetWindowText(hwnd,lpwszTitle,LenTitle+1)))
				{
					lpwszTitle[LenTitle]=0;
					MenuItemEx ListItem;
					ListItem.Clear();
					ListItem.strName=lpwszTitle;
					ProcList->SetUserData(&hwnd,sizeof(hwnd),ProcList->AddItem(&ListItem));
				}

				xf_free(lpwszTitle);
			}
		}
	}

	return TRUE;
}
