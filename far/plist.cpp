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

#include "plist.hpp"

#include "keys.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "lang.hpp"
#include "message.hpp"
#include "interf.hpp"
#include "imports.hpp"
#include "string_sort.hpp"
#include "exception.hpp"

#include "platform.fs.hpp"

#include "common/scope_exit.hpp"

#include "format.hpp"

struct menu_data
{
	string Title;
	DWORD Pid;
	HWND Hwnd;
};

struct ProcInfo
{
	VMenu2 *procList;
	bool ShowImage;
	std::exception_ptr ExceptionPtr;
};

// https://blogs.msdn.microsoft.com/oldnewthing/20071008-00/?p=24863/
static bool is_alttab_window(HWND const Window)
{
	if (!IsWindowVisible(Window))
		return false;

	auto Try = GetAncestor(Window, GA_ROOTOWNER);
	HWND Walk = nullptr;
	while (Try != Walk)
	{
		Walk = Try;
		Try = GetLastActivePopup(Walk);
		if (IsWindowVisible(Try))
			break;
	}
	if (Walk != Window)
		return false;

	// Tool windows should not be displayed either, these do not appear in the task bar
	if (GetWindowLongPtr(Window, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		return false;

	if (IsWindows8OrGreater())
	{
		int Cloaked = 0;
		if (SUCCEEDED(imports.DwmGetWindowAttribute(Window, DWMWA_CLOAKED, &Cloaked, sizeof(Cloaked))) && Cloaked)
			return false;
	}

	return true;
}

static BOOL CALLBACK EnumWindowsProc(HWND Window, LPARAM Param)
{
	const auto Info = reinterpret_cast<ProcInfo*>(Param);

	try
	{
		if (!is_alttab_window(Window))
			return true;

		string WindowTitle;
		os::GetWindowText(Window, WindowTitle);

		DWORD ProcID;
		GetWindowThreadProcessId(Window, &ProcID);

		string MenuItem;

		if (Info->ShowImage)
		{
			if (const auto Process = os::handle(OpenProcess(imports.QueryFullProcessImageNameW? PROCESS_QUERY_LIMITED_INFORMATION : PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, ProcID)))
				os::fs::GetModuleFileName(Process.native_handle(), nullptr, MenuItem);
			
			if (MenuItem.empty())
				MenuItem = L"???"s;
		}
		else
		{
			MenuItem = WindowTitle;
		}

		MenuItemEx NewItem(format(L"{0:9} {1} {2}"sv, ProcID, BoxSymbols[BS_V1], MenuItem));
		// for sorting
		NewItem.ComplexUserData = menu_data{ WindowTitle, ProcID, Window };
		Info->procList->AddItem(NewItem);

		return true;
	}
	CATCH_AND_SAVE_EXCEPTION_TO(Info->ExceptionPtr)

	return false;
}

void ShowProcessList()
{
	static bool Active = false;
	if (Active)
		return;
	Active = true;
	SCOPE_EXIT{ Active = false; };

	const auto ProcList = VMenu2::create(msg(lng::MProcessListTitle), {}, ScrY - 4);
	ProcList->SetMenuFlags(VMENU_WRAPMODE);
	ProcList->SetPosition({ -1, -1, 0, 0 });
	bool ShowImage = false;

	const auto& FillProcList = [&]
	{
		ProcList->clear();

		ProcInfo Info{ ProcList.get(), ShowImage };
		if (!EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&Info)))
		{
			rethrow_if(Info.ExceptionPtr);
			return false;
		}

		ProcList->SortItems([](const MenuItemEx& a, const MenuItemEx& b, SortItemParam& p)
		{
			return string_sort::less(std::any_cast<const menu_data&>(a.ComplexUserData).Title, std::any_cast<const menu_data&>(b.ComplexUserData).Title);
		});

		return true;
	};

	if (!FillProcList())
		return;

	ProcList->AssignHighlights(FALSE);
	ProcList->SetBottomTitle(msg(lng::MProcessListBottom));

	ProcList->Run([&](const Manager::Key& RawKey)
	{
		const auto Key=RawKey();
		int KeyProcessed = 1;
		switch (Key)
		{
			case KEY_F1:
				help::show(L"TaskList"sv);
				break;

			case KEY_NUMDEL:
			case KEY_DEL:
			{
				if (const auto MenuData = ProcList->GetComplexUserDataPtr<menu_data>())
				{
					if (Message(MSG_WARNING,
						msg(lng::MKillProcessTitle),
						{
							msg(lng::MAskKillProcess),
							MenuData->Title,
							msg(lng::MKillProcessWarning)
						},
						{ lng::MKillProcessKill, lng::MCancel }) == Message::first_button)
					{
						const os::handle Process(OpenProcess(PROCESS_TERMINATE, FALSE, MenuData->Pid));
						if (!Process || !TerminateProcess(Process.native_handle(), 0xFFFFFFFF))
						{
							const auto ErrorState = error_state::fetch();

							Message(MSG_WARNING, ErrorState,
								msg(lng::MKillProcessTitle),
								{
									msg(lng::MCannotKillProcess)
								},
								{ lng::MOk });
						}
					}
				}
			}
			[[fallthrough]];
			case KEY_CTRLR:
			case KEY_RCTRLR:
			{
				if (!FillProcList())
					ProcList->Close(-1);
				break;
			}

			case KEY_F2:
			{
				// TODO: change titles, don't enumerate again
				ShowImage = !ShowImage;
				const auto SelectPos = ProcList->GetSelectPos();
				if (!FillProcList())
				{
					ProcList->Close(-1);
				}
				else
				{
					ProcList->SetSelectPos(SelectPos);
				}
				break;
			}


			default:
				KeyProcessed = 0;
		}
		return KeyProcessed;
	});

	if (ProcList->GetExitCode() < 0)
		return;

	const auto MenuData = ProcList->GetComplexUserDataPtr<menu_data>();
	if (!MenuData)
		return;

	SwitchToWindow(MenuData->Hwnd);
}

void SwitchToWindow(HWND Window)
{
	SetForegroundWindow(Window);

	if (IsIconic(Window))
		ShowWindowAsync(Window, SW_RESTORE);
}
