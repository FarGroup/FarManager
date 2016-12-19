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
#include "interf.hpp"
#include "imports.hpp"
#include "strmix.hpp"

static BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam);
using menu_data = std::pair<string, HWND>;

struct ProcInfo
{
	VMenu2 *procList;
	bool bShowImage;
};

static struct task_sort
{
	bool operator()(const MenuItemEx& a, const MenuItemEx& b, SortItemParam& p) const
	{
		return StrCmp(any_cast<menu_data>(a.UserData).first, any_cast<menu_data>(b.UserData).first) < 0;
	}
}
TaskSort;

bool KillProcess(DWORD dwPID)
{
	bool Result = false;
	if (const auto Process = os::handle(OpenProcess(PROCESS_TERMINATE, FALSE, dwPID)))
	{
		Result = TerminateProcess(Process.native_handle(), 0xFFFFFFFF) != FALSE;
	}
	return Result;
}

void ShowProcessList()
{
	static bool Active = false;
	if (Active)
		return;
	Active = true;

	const auto ProcList = VMenu2::create(MSG(lng::MProcessListTitle), nullptr, 0, ScrY - 4);
	ProcList->SetMenuFlags(VMENU_WRAPMODE);
	ProcList->SetPosition(-1,-1,0,0);
	static bool bShowImage = false;

	ProcInfo pi = {ProcList.get(), bShowImage};

	if (EnumWindows(EnumWindowsProc,(LPARAM)&pi))
	{
		ProcList->AssignHighlights(FALSE);
		ProcList->SetBottomTitle(MSG(lng::MProcessListBottom));
		ProcList->SortItems(TaskSort);

		ProcList->Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			int KeyProcessed = 1;
			switch (Key)
			{
				case KEY_F1:
				{
					Help::create(L"TaskList");
					break;
				}

				case KEY_NUMDEL:
				case KEY_DEL:
				{
					if (const auto MenuData = ProcList->GetUserDataPtr<menu_data>())
					{
						const auto ProcWnd = MenuData->second;
						wchar_t_ptr Title;
						int LenTitle=GetWindowTextLength(ProcWnd);

						if (LenTitle)
						{
							Title.reset(LenTitle + 1);

							if (Title && (LenTitle=GetWindowText(ProcWnd, Title.get(), LenTitle+1)) != 0)
								Title[LenTitle]=0;
						}

						DWORD ProcID;
						GetWindowThreadProcessId(ProcWnd,&ProcID);

						if (Message(MSG_WARNING,2,MSG(lng::MKillProcessTitle),MSG(lng::MAskKillProcess),
									NullToEmpty(Title.get()),MSG(lng::MKillProcessWarning),MSG(lng::MKillProcessKill),MSG(lng::MCancel)) == Message::first_button)
						{
							if (!KillProcess(ProcID))
							{
								Global->CatchError();
								Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(lng::MKillProcessTitle),MSG(lng::MCannotKillProcess),MSG(lng::MOk));
							}
						}
					}
				}
				case KEY_CTRLR:
				case KEY_RCTRLR:
				{
					ProcList->clear();

					if (!EnumWindows(EnumWindowsProc,(LPARAM)&pi))
						ProcList->Close(-1);
					else
						ProcList->SortItems(TaskSort);
					break;
				}
				case KEY_F2:
				{
					pi.bShowImage=(bShowImage=!bShowImage);
					int SelectPos=ProcList->GetSelectPos();
					ProcList->clear();

					if (!EnumWindows(EnumWindowsProc,(LPARAM)&pi))
						ProcList->Close(-1);
					else
					{
						ProcList->SortItems(TaskSort);
						ProcList->SetSelectPos(SelectPos);
					}
					break;
				}


				default:
					KeyProcessed = 0;
			}
			return KeyProcessed;
		});

		if (ProcList->GetExitCode()>=0)
		{
			if (const auto MenuData = ProcList->GetUserDataPtr<menu_data>())
			{
				const auto ProcWnd = MenuData->second;
				//SetForegroundWindow(ProcWnd);
				// Allow SetForegroundWindow on Win98+.
				DWORD dwMs;
				// Remember the current value.
				BOOL bSPI = SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &dwMs, 0);

				if (bSPI) // Reset foreground lock timeout
					bSPI = SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, nullptr, 0);

				SetForegroundWindow(ProcWnd);

				if (bSPI) // Restore old value
					SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, ToPtr(dwMs), 0);

				WINDOWPLACEMENT wp = { sizeof(wp) };
				if (!GetWindowPlacement(ProcWnd,&wp) || wp.showCmd!=SW_SHOWMAXIMIZED)
					ShowWindowAsync(ProcWnd,SW_RESTORE);
			}
		}
	}
	Active = false;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam)
{
	const auto pi = reinterpret_cast<const ProcInfo*>(lParam);
	const auto ProcList = pi->procList;

	if (IsWindowVisible(hwnd) || (IsIconic(hwnd) && !(GetWindowLongPtr(hwnd,GWL_STYLE) & WS_DISABLED)))
	{
		DWORD ProcID;
		GetWindowThreadProcessId(hwnd,&ProcID);
		string strTitle;

		if (auto LenTitle = GetWindowTextLength(hwnd))
		{
			if (pi->bShowImage)
			{
				if (const auto Process = os::handle(OpenProcess(Imports().QueryFullProcessImageNameW? PROCESS_QUERY_LIMITED_INFORMATION : PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, false, ProcID)))
				{
					os::GetModuleFileNameEx(Process.native_handle(), nullptr, strTitle);
				}
			}
			else
			{
				wchar_t_ptr Title(LenTitle + 1);
				if ((LenTitle=GetWindowText(hwnd, Title.get(), LenTitle+1)) != 0)
					Title[LenTitle]=0;
				else
					Title[0]=0;
				strTitle=Title.get();
			}
		}
		if (!strTitle.empty())
		{
			MenuItemEx NewItem(format(L"{0:9} {1} {2}", ProcID, BoxSymbols[BS_V1], strTitle));
			NewItem.UserData = std::make_pair(strTitle, hwnd);
			ProcList->AddItem(NewItem);
		}

	}

	return TRUE;
}
