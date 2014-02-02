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
#include "vmenu2.hpp"
#include "language.hpp"
#include "message.hpp"
#include "config.hpp"
#include "interf.hpp"
#include "imports.hpp"

static BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam);
static BOOL KillProcess(DWORD dwPID);
const size_t PID_LENGTH = 6;

struct ProcInfo
{
	VMenu2 *procList;
	bool bShowImage;
};

static struct task_sort
{
	bool operator()(const MenuItemEx& a, const MenuItemEx& b, SortItemParam& p) const
	{
		return StrCmp(a.strName.data() + PID_LENGTH + 1, b.strName.data() + PID_LENGTH + 1) < 0;
	}
}
TaskSort;

void ShowProcessList()
{
	static bool Active = false;
	if (Active)
		return;
	Active = true;

	VMenu2 ProcList(MSG(MProcessListTitle),nullptr,0,ScrY-4);
	ProcList.SetFlags(VMENU_WRAPMODE);
	ProcList.SetPosition(-1,-1,0,0);
	static bool bShowImage = false;

	struct ProcInfo pi={&ProcList,bShowImage};

	if (EnumWindows(EnumWindowsProc,(LPARAM)&pi))
	{
		ProcList.AssignHighlights(FALSE);
		ProcList.SetBottomTitle(MSG(MProcessListBottom));
		ProcList.SortItems(TaskSort);

		ProcList.Run([&](int Key)->int
		{
			int KeyProcessed = 1;
			switch (Key)
			{
				case KEY_F1:
				{
					Help Hlp(L"TaskList");
					break;
				}

				case KEY_NUMDEL:
				case KEY_DEL:
				{
					HWND ProcWnd=*static_cast<HWND*>(ProcList.GetUserData(nullptr,0));

					if (ProcWnd)
					{
						wchar_t_ptr Title;
						int LenTitle=GetWindowTextLength(ProcWnd);

						if (LenTitle)
						{
							Title.reset(LenTitle + 1);

							if (Title && (LenTitle=GetWindowText(ProcWnd, Title.get(), LenTitle+1)))
								Title[LenTitle]=0;
						}

						DWORD ProcID;
						GetWindowThreadProcessId(ProcWnd,&ProcID);

						if (!Message(MSG_WARNING,2,MSG(MKillProcessTitle),MSG(MAskKillProcess),
									NullToEmpty(Title.get()),MSG(MKillProcessWarning),MSG(MKillProcessKill),MSG(MCancel)))
						{
							if (KillProcess(ProcID))
								Sleep(500);
							else
							{
								Global->CatchError();
								Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MKillProcessTitle),MSG(MCannotKillProcess),MSG(MOk));
							}
						}
					}
				}
				case KEY_CTRLR:
				case KEY_RCTRLR:
				{
					ProcList.DeleteItems();

					if (!EnumWindows(EnumWindowsProc,(LPARAM)&pi))
						ProcList.Close(-1);
					else
						ProcList.SortItems(TaskSort);
					break;
				}
				case KEY_F2:
				{
					pi.bShowImage=(bShowImage=!bShowImage);
					int SelectPos=ProcList.GetSelectPos();
					ProcList.DeleteItems();

					if (!EnumWindows(EnumWindowsProc,(LPARAM)&pi))
						ProcList.Close(-1);
					else
					{
						ProcList.SortItems(TaskSort);
						ProcList.SetSelectPos(SelectPos);
					}
					break;
				}


				default:
					KeyProcessed = 0;
			}
			return KeyProcessed;
		});

		if (ProcList.GetExitCode()>=0)
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
	Active = false;
}


BOOL KillProcess(DWORD dwPID)
{
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

	return bRet;
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam)
{
	struct ProcInfo *pi=(struct ProcInfo *)lParam;
	VMenu2 *ProcList=pi->procList;

	if (IsWindowVisible(hwnd) || (IsIconic(hwnd) && !(GetWindowLongPtr(hwnd,GWL_STYLE) & WS_DISABLED)))
	{
		DWORD ProcID;
		GetWindowThreadProcessId(hwnd,&ProcID);
		string strTitle;

		int LenTitle=GetWindowTextLength(hwnd);
		if (LenTitle)
		{
			if (pi->bShowImage)
			{
				HANDLE hProc = OpenProcess(Global->ifn->QueryFullProcessImageNameWPresent()? PROCESS_QUERY_LIMITED_INFORMATION : PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, false, ProcID);
				if (hProc)
				{
					api::GetModuleFileNameEx(hProc, nullptr, strTitle);
					CloseHandle(hProc);
				}
			}
			else
			{
				wchar_t_ptr Title(LenTitle + 1);
				if (Title)
				{
					if ((LenTitle=GetWindowText(hwnd, Title.get(), LenTitle+1)))
						Title[LenTitle]=0;
					else
						Title[0]=0;
					strTitle=Title.get();
				}
			}
		}
		if (!strTitle.empty())
		{
			ProcList->SetUserData(&hwnd,sizeof(hwnd),ProcList->AddItem(FormatString()<<fmt::MinWidth(PID_LENGTH)<<ProcID<< L' ' << BoxSymbols[BS_V1]<< L' ' << strTitle));
		}

	}

	return TRUE;
}
