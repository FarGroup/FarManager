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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "plist.hpp"

// Internal:
#include "keys.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "lang.hpp"
#include "message.hpp"
#include "interf.hpp"
#include "imports.hpp"
#include "exception_handler.hpp"
#include "console.hpp"
#include "keyboard.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"
#include "platform.process.hpp"

// Common:
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

struct menu_data
{
	string Title;
	DWORD Pid;
	HWND Hwnd;
};

struct ProcInfo
{
	struct proc_item
	{
		HWND Window;
		DWORD Pid;
	};

	std::vector<proc_item> Windows;
	std::exception_ptr ExceptionPtr;
};

static bool is_cloaked_window(HWND const Window)
{
	if (!imports.DwmGetWindowAttribute)
		return false;

	int Cloaked = 0;
	if (const auto Result = imports.DwmGetWindowAttribute(Window, DWMWA_CLOAKED, &Cloaked, sizeof(Cloaked)); FAILED(Result))
	{
		LOGWARNING(L"DwmGetWindowAttribute"sv, os::format_error(Result));
		return false;
	}

	return Cloaked != 0;
}

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

	return !is_cloaked_window(Window);
}

static BOOL CALLBACK EnumWindowsProc(HWND const Window, LPARAM const Param)
{
	auto& Info = edit_as<ProcInfo>(Param);

	return cpp_try(
	[&]
	{
		if (!is_alttab_window(Window))
			return true;

		DWORD Pid;
		if (!GetWindowThreadProcessId(Window, &Pid))
			return true;

		Info.Windows.emplace_back(Window, Pid);
		return true;
	},
	save_exception_and_return<false>(Info.ExceptionPtr)
	);
}

static void AddMenuItem(HWND const Window, DWORD const Pid, size_t const PidWidth, bool const ShowImage, vmenu2_ptr const& Menu)
{
	string WindowTitle;
	os::GetWindowText(Window, WindowTitle);

	string MenuItem;

	if (ShowImage)
	{
		MenuItem = os::process::get_process_name(Pid);

		if (MenuItem.empty())
			MenuItem = L"???"sv;
	}
	else
	{
		MenuItem = WindowTitle;
	}

	const auto Self = Pid == GetCurrentProcessId() || Window == console.GetWindow();

	MenuItemEx NewItem(far::format(L"{:{}} {} {}"sv, Pid, PidWidth, BoxSymbols[BS_V1], MenuItem), Self? MIF_CHECKED : MIF_NONE);
	NewItem.ComplexUserData = menu_data{ WindowTitle, Pid, Window };
	Menu->AddItem(NewItem);
}

void ShowProcessList()
{
	static bool Active = false;
	if (Active)
		return;
	Active = true;
	SCOPE_EXIT{ Active = false; };

	const auto ProcList = VMenu2::create(msg(lng::MProcessListTitle), {}, ScrY - 4);
	ProcList->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
	ProcList->SetPosition({ -1, -1, 0, 0 });
	bool ShowImage = false;

	ProcInfo Info;
	Info.Windows.reserve(128);

	const auto FillProcList = [&]
	{
		SCOPED_ACTION(Dialog::suppress_redraw)(ProcList.get());

		ProcList->clear();
		Info.Windows.clear();

		if (!EnumWindows(EnumWindowsProc, std::bit_cast<LPARAM>(&Info)))
		{
			rethrow_if(Info.ExceptionPtr);
			return false;
		}

		const auto MaxPid = std::ranges::max_element(Info.Windows, {}, &ProcInfo::proc_item::Pid)->Pid;
		const auto PidWidth = static_cast<size_t>(std::log10(MaxPid)) + 1;

		for (const auto& i: Info.Windows)
		{
			AddMenuItem(i.Window, i.Pid, PidWidth, ShowImage, ProcList);
		}

		return true;
	};

	if (!FillProcList())
		return;

	ProcList->AssignHighlights();
	ProcList->SetBottomTitle(KeysToLocalizedText(KEY_DEL, KEY_F2, KEY_CTRLR));

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
						{ lng::MKillProcessKill, lng::MCancel }) == message_result::first_button)
					{
						if (!os::process::terminate_other(MenuData->Pid))
						{
							const auto ErrorState = os::last_error();

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
