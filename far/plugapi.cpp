/*
plugapi.cpp

API, доступное плагинам (диалоги, меню, ...)
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
#include "plugapi.hpp"

// Internal:
#include "keys.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "scantree.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "plugins.hpp"
#include "savescr.hpp"
#include "flink.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "window.hpp"
#include "scrbuf.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "filefilter.hpp"
#include "fileowner.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "exitcode.hpp"
#include "processname.hpp"
#include "RegExp.hpp"
#include "taskbar.hpp"
#include "console.hpp"
#include "plugsettings.hpp"
#include "farversion.hpp"
#include "mix.hpp"
#include "uuids.far.hpp"
#include "clipboard.hpp"
#include "strmix.hpp"
#include "notification.hpp"
#include "panelmix.hpp"
#include "xlat.hpp"
#include "dirinfo.hpp"
#include "lang.hpp"
#include "viewer.hpp"
#include "datetime.hpp"
#include "string_utils.hpp"
#include "cvtname.hpp"
#include "filemasks.hpp"
#include "desktop.hpp"
#include "string_sort.hpp"
#include "global.hpp"
#include "lockscrn.hpp"
#include "exception_handler.hpp"
#include "color_picker.hpp"
#include "log.hpp"
#include "filestr.hpp"

// Platform:
#include "platform.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/enum_tokens.hpp"
#include "common/null_iterator.hpp"
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

static Plugin* UuidToPlugin(const UUID* Id)
{
	return Id && Global->CtrlObject? Global->CtrlObject->Plugins->FindPlugin(*Id) : nullptr;
}

static Panel* GetHostPanel(HANDLE Handle)
{
	if (!Handle || Handle == PANEL_ACTIVE)
	{
		return Global->CtrlObject->Cp()->ActivePanel().get();
	}
	else if (Handle == PANEL_PASSIVE)
	{
		return Global->CtrlObject->Cp()->PassivePanel().get();
	}

	return static_cast<Panel*>(Handle);
}


namespace cfunctions
{
	using comparer = int (WINAPI*)(const void*, const void*, void*);

	static thread_local comparer bsearch_comparer;
	static thread_local void* bsearch_param;

	static int bsearch_comparer_wrapper(const void* a, const void* b)
	{
		return bsearch_comparer(a, b, bsearch_param);
	}

	static void* bsearchex(const void* key, const void* base, size_t nelem, size_t width, comparer user_comparer, void* user_param) noexcept
	{
		bsearch_comparer = user_comparer;
		bsearch_param = user_param;
		return std::bsearch(key, base, nelem, width, bsearch_comparer_wrapper);
	}

	static thread_local comparer qsort_comparer;
	static thread_local void* qsort_param;

	static int qsort_comparer_wrapper(const void* a, const void* b)
	{
		return qsort_comparer(a, b, qsort_param);
	}

	static void qsortex(char *base, size_t nel, size_t width, comparer user_comparer, void *user_param) noexcept
	{
		qsort_comparer = user_comparer;
		qsort_param = user_param;
		return std::qsort(base, nel, width, qsort_comparer_wrapper);
	}
}

// BUGBUG duplicate
auto cpp_try(auto const& Callable, source_location const& Location = source_location::current())
{
	return cpp_try(Callable, save_exception_to(GlobalExceptionPtr()), {}, Location);
}

auto cpp_try(auto const& Callable, auto const Fallback, source_location const& Location = source_location::current())
{
	return cpp_try(Callable, [&](source_location const&)
	{
		save_exception_to{ GlobalExceptionPtr() }(Location);
		return Fallback;
	},
	{},
	Location);
}

class pluginapi_sort_accessor
{
public:
	static auto compare_ordinal_icase(string_view const Str1, string_view const Str2)
	{
		return string_sort::ordering_as_int(string_sort::keyhole::compare_ordinal_icase(Str1, Str2));
	}
};

namespace pluginapi
{
static int WINAPIV apiSprintf(wchar_t* Dest, const wchar_t* Format, ...) noexcept //?deprecated
{
	// noexcept
	va_list argptr;
	va_start(argptr, Format);
	SCOPE_EXIT noexcept { va_end(argptr); };
	// BUGBUG, do not use vswprintf here, %s treated as char* in GCC
	return _vsnwprintf(Dest, 32000, Format, argptr);
}

static int WINAPIV apiSnprintf(wchar_t* Dest, size_t Count, const wchar_t* Format, ...) noexcept
{
	// noexcept
	va_list argptr;
	va_start(argptr, Format);
	SCOPE_EXIT noexcept { va_end(argptr); };
	// BUGBUG, do not use vswprintf here, %s treated as char* in GCC
	return _vsnwprintf(Dest, Count, Format, argptr);
}

static int WINAPIV apiSscanf(const wchar_t* Src, const wchar_t* Format, ...) noexcept
{
	// noexcept
	va_list argptr;
	va_start(argptr, Format);
	SCOPE_EXIT noexcept { va_end(argptr); };

WARNING_PUSH()
WARNING_DISABLE_CLANG("-Wused-but-marked-unused")
	return vswscanf(Src, Format, argptr);
WARNING_POP()
}

static wchar_t *WINAPI apiItoa(int value, wchar_t *Str, int radix) noexcept
{
	// noexcept
	return _itow(value,Str,radix);
}

static wchar_t *WINAPI apiItoa64(long long value, wchar_t *Str, int radix) noexcept
{
	// noexcept
	return _i64tow(value, Str, radix);
}

static int WINAPI apiAtoi(const wchar_t *Str) noexcept
{
	// noexcept
	return static_cast<int>(std::wcstol(Str, nullptr, 10));
}

static long long WINAPI apiAtoi64(const wchar_t *Str) noexcept
{
	// noexcept
	return std::wcstoll(Str, nullptr, 10);
}

void WINAPI apiQsort(void *base, size_t nelem, size_t width, cfunctions::comparer fcmp, void *user) noexcept
{
	//noexcept
	return cfunctions::qsortex(static_cast<char*>(base),nelem,width,fcmp,user);
}

void *WINAPI apiBsearch(const void *key, const void *base, size_t nelem, size_t width, cfunctions::comparer fcmp, void *user) noexcept
{
	//noexcept
	return cfunctions::bsearchex(key, base, nelem, width, fcmp, user);
}

static void WINAPI apiUnquote(wchar_t *Str) noexcept
{
	return cpp_try(
	[&]
	{
		if (!Str)
			return;

		const auto Iterator = null_iterator(Str);
		*std::remove(Iterator, Iterator.end(), L'"') = 0;
	});
}

static wchar_t* WINAPI apiRemoveLeadingSpaces(wchar_t *Str) noexcept
{
	return cpp_try(
	[&]
	{
		const auto Iterator = null_iterator(Str);
		const auto NewBegin = std::find_if_not(Iterator, Iterator.end(), std::iswspace);
		if (NewBegin != Iterator)
		{
			*std::copy(NewBegin, Iterator.end(), Str) = {};
		}
		return Str;
	},
	nullptr);
}

static wchar_t* WINAPI apiRemoveTrailingSpaces(wchar_t *Str) noexcept
{
	return cpp_try(
	[&]
	{
		const auto REnd = std::make_reverse_iterator(Str);
		Str[REnd - std::find_if_not(REnd - std::wcslen(Str), REnd, std::iswspace)] = 0;
		return Str;
	},
	nullptr);
}

static wchar_t* WINAPI apiRemoveExternalSpaces(wchar_t *Str) noexcept
{
	//noexcept
	return apiRemoveTrailingSpaces(apiRemoveLeadingSpaces(Str));
}

static wchar_t* WINAPI apiQuoteSpaceOnly(wchar_t *Str) noexcept
{
	return cpp_try(
	[&]
	{
		return legacy::QuoteSpaceOnly(Str);
	},
	nullptr);
}

intptr_t WINAPI apiInputBox(
    const UUID* PluginId,
    const UUID* Id,
    const wchar_t *Title,
    const wchar_t *Prompt,
    const wchar_t *HistoryName,
    const wchar_t *SrcText,
    wchar_t *DestText,
    size_t DestSize,
    const wchar_t *HelpTopic,
    unsigned long long Flags
) noexcept
{
	return cpp_try(
	[&]
	{
		if (Global->WindowManager->ManagerIsDown())
			return false;

		string strDest;

		const auto Result = GetString(
			NullToEmpty(Title),
			NullToEmpty(Prompt),
			NullToEmpty(HistoryName),
			NullToEmpty(SrcText),
			strDest,
			NullToEmpty(HelpTopic),
			Flags&~FIB_CHECKBOX,
			{},
			{},
			UuidToPlugin(PluginId),
			Id);

		xwcsncpy(DestText, strDest.c_str(), DestSize);
		return Result;
	},
	false);
}

BOOL WINAPI apiShowHelp(const wchar_t *ModuleName, const wchar_t *HelpTopic, FARHELPFLAGS Flags) noexcept
{
	return cpp_try(
	[&]
	{
		if (Global->WindowManager->ManagerIsDown())
			return false;

		if (!HelpTopic)
			HelpTopic = L"Contents";

		auto OFlags = Flags;
		Flags &= ~(FHELP_NOSHOWERROR | FHELP_USECONTENTS);
		string strTopic;
		string strMask;

		// двоеточие в начале топика надо бы игнорировать и в том случае,
		// если стоит FHELP_FARHELP...
		if ((Flags&FHELP_FARHELP) || *HelpTopic == L':')
		{
			strTopic = HelpTopic + ((*HelpTopic == L':') ? 1 : 0);
		}
		else if (ModuleName && (Flags&FHELP_GUID))
		{
			if (!*ModuleName || view_as<UUID>(ModuleName) == FarUuid)
			{
				OFlags |= FHELP_FARHELP;
				strTopic = HelpTopic + ((*HelpTopic == L':') ? 1 : 0);
			}
			else
			{
				if (const auto plugin = Global->CtrlObject->Plugins->FindPlugin(view_as<UUID>(ModuleName)))
				{
					OFlags |= FHELP_CUSTOMPATH;
					strTopic = help::make_link(ExtractFilePath(plugin->ModuleName()), HelpTopic);
				}
			}
		}
		else
		{
			if (ModuleName)
			{
				// FHELP_SELFHELP=0 - трактовать первый пар-р как Info.ModuleName
				//                   и показать топик из хелпа вызвавшего плагина
				/* $ 17.11.2000 SVS
				А значение FHELP_SELFHELP равно чему? Правильно - 0
				И фигля здесь удивляться тому, что функция не работает :-(
				*/
				string_view Path;
				if (Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE | FHELP_CUSTOMPATH)))
				{
					Path = ModuleName;

					if (Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE)))
					{
						if (Flags&FHELP_CUSTOMFILE)
							strMask = PointToName(Path);
						else
							strMask.clear();

						CutToSlash(Path);
					}
				}
				else
					return false;

				strTopic = help::make_link(Path, HelpTopic);
			}
			else
				return false;
		}

		return help::show(strTopic, strMask, OFlags);
	},
	false);
}

intptr_t WINAPI apiAdvControl(const UUID* PluginId, ADVANCED_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		if (ACTL_SYNCHRO==Command) //must be first
		{
			message_manager::instance().notify(*PluginId, Param2);
			return 0;
		}
		if (ACTL_GETWINDOWTYPE==Command)
		{
			const auto info = static_cast<WindowType*>(Param2);
			if (CheckStructSize(info))
			{
				switch(const auto type = WindowTypeToPluginWindowType(Manager::GetCurrentWindowType()))
				{
				case WTYPE_DESKTOP:
				case WTYPE_PANELS:
				case WTYPE_VIEWER:
				case WTYPE_EDITOR:
				case WTYPE_DIALOG:
				case WTYPE_VMENU:
				case WTYPE_HELP:
				case WTYPE_COMBOBOX:
				case WTYPE_GRABBER:
				case WTYPE_HMENU:
				//case WTYPE_FINDFOLDER:
					info->Type=type;
					return TRUE;
				default:
					break;
				}
			}
			return FALSE;
		}

		switch (Command)
		{
		case ACTL_GETFARMANAGERVERSION:
		case ACTL_GETCOLOR:
		case ACTL_GETARRAYCOLOR:
		case ACTL_GETFARHWND:
		case ACTL_SETPROGRESSSTATE:
		case ACTL_SETPROGRESSVALUE:
		case ACTL_GETFARRECT:
		case ACTL_GETCURSORPOS:
		case ACTL_SETCURSORPOS:
		case ACTL_PROGRESSNOTIFY:
			break;

		default:
			if (Global->WindowManager->ManagerIsDown())
				return 0;
		}

		switch (Command)
		{
		case ACTL_GETFARMANAGERVERSION:
			if (Param2)
				*static_cast<VersionInfo*>(Param2) = build::version();

			return TRUE;

		/* $ 24.08.2000 SVS
			ожидать определенную (или любую) клавишу
			(const INPUT_RECORD*)Param2 - код клавиши, которую ожидаем, или nullptr
			если все равно какую клавишу ждать.
			возвращает 0;
		*/
		case ACTL_WAITKEY:
			WaitKey(Param2? InputRecordToKey(static_cast<const INPUT_RECORD*>(Param2)) : -1, {}, false);
			return 0;

		/* $ 04.12.2000 SVS
			ACTL_GETCOLOR - получить определенный цвет по индексу, определенному
			в farcolor.hpp
			Param2 - [OUT] значение цвета
			Return - TRUE если OK или FALSE если индекс неверен.
		*/
		case ACTL_GETCOLOR:
			if (static_cast<size_t>(Param1) < Global->Opt->Palette.size())
			{
				*static_cast<FarColor*>(Param2) = Global->Opt->Palette[static_cast<size_t>(Param1)];
				return TRUE;
			}
			return FALSE;

		/* $ 04.12.2000 SVS
			ACTL_GETARRAYCOLOR - получить весь массив цветов
			Param1 - размер буфера (в элементах FarColor)
			Param2 - указатель на буфер или nullptr, чтобы получить необходимый размер
			Return - размер массива.
		*/
		case ACTL_GETARRAYCOLOR:
			if (Param1 && Param2)
			{
				Global->Opt->Palette.CopyTo({ static_cast<FarColor*>(Param2), static_cast<size_t>(Param1) });
			}
			return Global->Opt->Palette.size();

		case ACTL_SETARRAYCOLOR:
		{
			const auto Pal = static_cast<const FarSetColors*>(Param2);
			if (CheckStructSize(Pal))
			{

				if (Pal->Colors && Pal->StartIndex+Pal->ColorsCount <= Global->Opt->Palette.size())
				{
					Global->Opt->Palette.Set(Pal->StartIndex, { Pal->Colors, Pal->ColorsCount });
					if (Pal->Flags&FSETCLR_REDRAW)
					{
						SCOPED_ACTION(LockScreen);

						Global->WindowManager->ResizeAllWindows();
						Global->WindowManager->PluginCommit(); // коммитим.
					}

					return TRUE;
				}
			}
			return FALSE;
		}

		/* $ 05.06.2001 tran
			новые ACTL_ для работы с окнами */
		case ACTL_GETWINDOWINFO:
		{
			const auto wi = static_cast<WindowInfo*>(Param2);
			if (CheckStructSize(wi))
			{
				string strType, strName;
				window_ptr f = nullptr;
				bool modal=false;

				/* $ 22.12.2001 VVM
					+ Если Pos == -1 то берем текущее окно */
				if (wi->Pos == -1)
				{
					f = Global->WindowManager->GetCurrentWindow();
					modal=Global->WindowManager->InModal();
				}
				else
				{
					if (wi->Pos >= 0 && wi->Pos < static_cast<intptr_t>(Global->WindowManager->GetWindowCount()))
					{
						f = Global->WindowManager->GetWindow(wi->Pos);
						modal = Global->WindowManager->IsModal(wi->Pos);
					}
				}

				if (!f)
					return FALSE;

				f->GetTypeAndName(strType, strName);

				if (wi->TypeNameSize && wi->TypeName)
				{
					xwcsncpy(wi->TypeName, strType.c_str(), wi->TypeNameSize);
				}
				else
				{
					wi->TypeNameSize=strType.size()+1;
				}

				if (wi->NameSize && wi->Name)
				{
					xwcsncpy(wi->Name, strName.c_str(), wi->NameSize);
				}
				else
				{
					wi->NameSize=strName.size()+1;
				}

				if(-1==wi->Pos) wi->Pos = Global->WindowManager->IndexOf(f);
				wi->Type=WindowTypeToPluginWindowType(f->GetType());
				wi->Flags=0;
				if (f->IsFileModified())
					wi->Flags|=WIF_MODIFIED;
				if (f == Global->WindowManager->GetCurrentWindow())
					wi->Flags|=WIF_CURRENT;
				if (modal)
					wi->Flags|=WIF_MODAL;

				switch (wi->Type)
				{
					case WTYPE_VIEWER:
						wi->Id = std::static_pointer_cast<FileViewer>(f)->GetId();
						break;
					case WTYPE_EDITOR:
						wi->Id = std::static_pointer_cast<FileEditor>(f)->GetId();
						break;
					case WTYPE_VMENU:
					case WTYPE_DIALOG:
						wi->Id = std::bit_cast<intptr_t>(f.get()); // BUGBUG
						break;
					case WTYPE_COMBOBOX:
						wi->Id = std::bit_cast<intptr_t>(std::static_pointer_cast<VMenu>(f)->GetDialog().get()); // BUGBUG
						break;
					default:
						wi->Id=0;
						break;
				}
				return TRUE;
			}

			return FALSE;
		}

		case ACTL_GETWINDOWCOUNT:
			return Global->WindowManager->GetWindowCount();

		case ACTL_SETCURRENTWINDOW:
		{
			// Запретим переключение фрэймов, если находимся в модальном редакторе/вьюере.
			const auto NextWindow = Global->WindowManager->GetWindow(Param1);
			if (!Global->WindowManager->InModal() && NextWindow)
			{
				Global->WindowManager->ActivateWindow(NextWindow);
				Global->WindowManager->PluginCommit();
				return TRUE;
			}

			return FALSE;
		}
		/*$ 26.06.2001 SKV
			Для полноценной работы с ACTL_SETCURRENTWINDOW
			(и может еще для чего в будущем)
		*/
		case ACTL_COMMIT:
			return TRUE;

		case ACTL_GETFARHWND:
			return std::bit_cast<intptr_t>(console.GetWindow());

		case ACTL_REDRAWALL:
		{
			Global->WindowManager->RefreshAll();
			Global->WindowManager->PluginCommit();
			return TRUE;
		}

		case ACTL_SETPROGRESSSTATE:
			taskbar::set_state(static_cast<TBPFLAG>(Param1));
			return TRUE;

		case ACTL_SETPROGRESSVALUE:
		{
			BOOL Result=FALSE;
			const auto PV = static_cast<const ProgressValue*>(Param2);
			if(CheckStructSize(PV))
			{
				taskbar::set_value(PV->Completed,PV->Total);
				Result=TRUE;
			}
			return Result;
		}

		case ACTL_QUIT:
			Global->WindowManager->ExitMainLoop(FALSE, Param1);
			return TRUE;

		case ACTL_GETFARRECT:
			{
				if (!Param2)
					return false;

				auto& Rect = *static_cast<SMALL_RECT*>(Param2);

				if(Global->Opt->WindowMode)
				{
					rectangle FarRect;
					if (!console.GetWorkingRect(FarRect))
						return false;

					Rect.Left = FarRect.left;
					Rect.Top = FarRect.top;
					Rect.Right = FarRect.right;
					Rect.Bottom = FarRect.bottom;

					return true;
				}
				else
				{
					point Size;
					if (!console.GetSize(Size))
						return false;

					Rect.Left = 0;
					Rect.Top = 0;
					Rect.Right = Size.x - 1;
					Rect.Bottom = Size.y - 1;

					return true;
				}
			}

		case ACTL_GETCURSORPOS:
			{
				if (!Param2)
					return false;

				point CursorPosition;
				if (!console.GetCursorPosition(CursorPosition))
					return false;

				auto& Pos = *static_cast<PCOORD>(Param2);
				Pos.X = CursorPosition.x;
				Pos.Y = CursorPosition.y;

				return true;
			}

		case ACTL_SETCURSORPOS:
			{
				if (!Param2)
					return false;

				return console.SetCursorPosition(*static_cast<COORD const*>(Param2));
			}

		case ACTL_PROGRESSNOTIFY:
		{
			taskbar::flash();
			return TRUE;
		}

		default:
			break;
		}

		return FALSE;
	},
	FALSE);
}

intptr_t WINAPI apiMenuFn(
    const UUID* PluginId,
    const UUID* Id,
    intptr_t X,
    intptr_t Y,
    intptr_t MaxHeight,
    unsigned long long Flags,
    const wchar_t *Title,
    const wchar_t *Bottom,
    const wchar_t *HelpTopic,
    const FarKey *BreakKeys,
    intptr_t *BreakCode,
    const FarMenuItem *Item,
    size_t ItemsNumber
) noexcept
{
	return cpp_try(
	[&]
	{
		if (Global->WindowManager->ManagerIsDown())
			return -1;

		if (Global->DisablePluginsOutput)
			return -1;

		int ExitCode;
		{
			DWORD MenuFlags = 0;

			if (Flags & FMENU_SHOWAMPERSAND)
				MenuFlags |= VMENU_SHOWAMPERSAND;

			if (Flags & FMENU_WRAPMODE)
				MenuFlags |= VMENU_WRAPMODE;

			if (Flags & FMENU_CHANGECONSOLETITLE)
				MenuFlags |= VMENU_CHANGECONSOLETITLE;

			const auto FarMenu = VMenu2::create(NullToEmpty(Title), {}, MaxHeight, MenuFlags);
			FarMenu->SetPosition({ static_cast<int>(X), static_cast<int>(Y), 0, 0 });
			if(Id)
			{
				FarMenu->SetId(*Id);
			}

			if (BreakCode)
				*BreakCode=-1;

			{
				const auto Topic = help::make_topic(UuidToPlugin(PluginId), NullToEmpty(HelpTopic));
				if (!Topic.empty())
					FarMenu->SetHelp(Topic);
			}

			if (Bottom)
				FarMenu->SetBottomTitle(Bottom);

			size_t Selected=0;

			for (const auto& i: std::span(Item, ItemsNumber))
			{
				MenuItemEx CurItem;
				CurItem.Flags = i.Flags;
				CurItem.Name.clear();
				// исключаем MultiSelected, т.к. у нас сейчас движок к этому не приспособлен, оставляем только первый
				const auto SelCurItem = CurItem.Flags&LIF_SELECTED;
				CurItem.Flags&=~LIF_SELECTED;

				if (!Selected && !(CurItem.Flags&LIF_SEPARATOR) && SelCurItem)
				{
					CurItem.Flags|=SelCurItem;
					Selected++;
				}

				CurItem.Name = NullToEmpty(i.Text);
				if(CurItem.Flags&LIF_SEPARATOR)
				{
					CurItem.AccelKey=0;
				}
				else
				{
					INPUT_RECORD input{};
					FarKeyToInputRecord(i.AccelKey,&input);
					CurItem.AccelKey=InputRecordToKey(&input);
				}
				FarMenu->AddItem(CurItem);
			}

			if (!Selected)
				FarMenu->SetSelectPos(0,1);

			if (Flags & (FMENU_AUTOHIGHLIGHT | FMENU_REVERSEAUTOHIGHLIGHT))
				FarMenu->AssignHighlights((Flags & FMENU_REVERSEAUTOHIGHLIGHT) != 0);

			FarMenu->SetTitle(NullToEmpty(Title));

			int BoxType = DOUBLE_BOX;
			if (Flags & FMENU_SHOWNOBOX)
				BoxType = NO_BOX;
			else if (Flags & FMENU_SHOWSHORTBOX)
				BoxType = (Flags & FMENU_SHOWSINGLEBOX) ? SHORT_SINGLE_BOX : SHORT_DOUBLE_BOX;
			else if (Flags & FMENU_SHOWSINGLEBOX)
				BoxType = SINGLE_BOX;
			FarMenu->SetBoxType(BoxType);
			if (Flags & FMENU_NODRAWSHADOW)
				FarMenu->SetDialogMode(DMODE_NODRAWSHADOW);

			ExitCode=FarMenu->RunEx([&](int Msg, void *param)
			{
				if (Msg!=DN_INPUT || !BreakKeys)
					return 0;

				const auto& ReadRec = *static_cast<INPUT_RECORD const*>(param);
				const auto ReadKey = InputRecordToKey(&ReadRec);

				if (ReadKey==KEY_NONE)
					return 0;

				for (size_t i = 0; BreakKeys[i].VirtualKeyCode; ++i)
				{
					if (ReadRec.Event.KeyEvent.wVirtualKeyCode != BreakKeys[i].VirtualKeyCode)
						continue;

					const auto NormalizeControlKeys = [](DWORD const Value)
					{
						// BUGBUG What if they actually want to handle left & right separately, or other control keys?
						return
							(Value & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED) ? LEFT_CTRL_PRESSED : 0) |
							(Value & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED) ? LEFT_ALT_PRESSED : 0) |
							(Value & SHIFT_PRESSED);
					};

					if (NormalizeControlKeys(ReadRec.Event.KeyEvent.dwControlKeyState) == NormalizeControlKeys(BreakKeys[i].ControlKeyState))
					{
						if (BreakCode)
							*BreakCode = i;

						FarMenu->Close(-2, true);
						return 1;
					}
				}
				return 0;
			});
		}
	//  CheckScreenLock();
		return ExitCode;
	},
	-1);
}

intptr_t WINAPI apiDefDlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2) noexcept
{
	return cpp_try(
	[&]
	{
		return static_cast<Dialog*>(hDlg)->DefProc(Msg, Param1, Param2);
	},
	0);
}

intptr_t WINAPI apiSendDlgMessage(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2) noexcept
{
	const auto ErrorResult = [Msg]
	{
		switch (Msg)
		{
		case DM_GETFOCUS:
		case DM_LISTADDSTR:
			return -1;

		default:
			return 0;
		}
	};

	return cpp_try(
	[&]
	{
		const auto dialog = static_cast<Dialog*>(hDlg);
		return Dialog::IsValid(dialog)? dialog->SendMessage(Msg, Param1, Param2) : ErrorResult();
	},
	ErrorResult());
}

HANDLE WINAPI apiDialogInit(const UUID* PluginId, const UUID* Id, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2,
                            const wchar_t *HelpTopic, const FarDialogItem *Item,
                            size_t ItemsNumber, intptr_t Reserved, unsigned long long Flags,
                            FARWINDOWPROC DlgProc, void* Param) noexcept
{
	return cpp_try(
	[&]
	{
		auto hDlg = INVALID_HANDLE_VALUE;

		if (Global->WindowManager->ManagerIsDown())
			return hDlg;

		if (Global->DisablePluginsOutput || !ItemsNumber || !Item)
			return hDlg;

		// ФИЧА! нельзя указывать отрицательные X2 и Y2
		const auto fixCoord = [](intptr_t first, intptr_t second) { return (first < 0 && second == 0)? 1 : second; };
		X2 = fixCoord(X1, X2);
		Y2 = fixCoord(Y1, Y2);
		const auto checkCoord = [](intptr_t first, intptr_t second) { return second >= 0 && ((first < 0)? (second > 0) : (first <= second)); };
		if (!checkCoord(X1, X2) || !checkCoord(Y1, Y2))
			return hDlg;

		if (const auto Plugin = Global->CtrlObject->Plugins->FindPlugin(*PluginId))
		{
			class plugin_dialog: public Dialog
			{
				struct private_tag { explicit private_tag() = default; };

			public:
				static dialog_ptr create(std::span<const FarDialogItem> const Src, FARWINDOWPROC const DlgProc, void* const InitParam)
				{
					return std::make_shared<plugin_dialog>(private_tag(), Src, DlgProc, InitParam);
				}

				intptr_t Proc(Dialog* hDlg, intptr_t Msg, intptr_t Param1, void* Param2) const
				{
					return m_Proc(hDlg, Msg, Param1, Param2);
				}

				plugin_dialog(private_tag, std::span<const FarDialogItem> const Src, FARWINDOWPROC const DlgProc, void* const InitParam):
					Dialog(Dialog::private_tag(), Src, DlgProc? [this](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2) { return Proc(Dlg, Msg, Param1, Param2); } : dialog_handler(), InitParam),
					m_Proc(DlgProc)
				{}

			private:
				FARWINDOWPROC m_Proc;
			};

			const auto FarDialog = plugin_dialog::create({ Item, ItemsNumber }, DlgProc, Param);

			if (FarDialog->InitOK())
			{
				bool modalInPlace = (Flags & FDLG_NONMODAL) && Global->WindowManager->InModal();

				if (modalInPlace)
				{
					Flags &= ~FDLG_NONMODAL;
				}

				if (Flags & FDLG_NONMODAL)
					FarDialog->SetCanLoseFocus(TRUE);
				else
					Plugin->AddDialog(FarDialog);

				hDlg = FarDialog.get();

				FarDialog->SetPosition({ static_cast<int>(X1), static_cast<int>(Y1), static_cast<int>(X2), static_cast<int>(Y2) });

				if (Flags & FDLG_WARNING)
					FarDialog->SetDialogMode(DMODE_WARNINGSTYLE);

				if (Flags & FDLG_SMALLDIALOG)
					FarDialog->SetDialogMode(DMODE_SMALLDIALOG);

				if (Flags & FDLG_NODRAWSHADOW)
					FarDialog->SetDialogMode(DMODE_NODRAWSHADOW);

				if (Flags & FDLG_NODRAWPANEL)
					FarDialog->SetDialogMode(DMODE_NODRAWPANEL);

				if (Flags & FDLG_KEEPCONSOLETITLE)
					FarDialog->SetDialogMode(DMODE_KEEPCONSOLETITLE);

				FarDialog->SetHelp(NullToEmpty(HelpTopic));

				FarDialog->SetId(*Id);
				/* $ 29.08.2000 SVS
				   Запомним номер плагина - сейчас в основном для формирования HelpTopic
				*/
				FarDialog->SetPluginOwner(UuidToPlugin(PluginId));

				if (FarDialog->GetCanLoseFocus())
				{
					FarDialog->Process();
					Global->WindowManager->PluginCommit();
				}

				if (modalInPlace)
				{
					apiDialogRun(hDlg);
					apiDialogFree(hDlg);
					hDlg = INVALID_HANDLE_VALUE;
				}
			}
		}
		return hDlg;
	},
	INVALID_HANDLE_VALUE);
}

intptr_t WINAPI apiDialogRun(HANDLE hDlg) noexcept
{
	return cpp_try(
	[&]
	{
		if (Global->WindowManager->ManagerIsDown())
			return -1;

		const auto FarDialog = static_cast<Dialog*>(hDlg);

		if (FarDialog->GetCanLoseFocus())
			return -1;

		FarDialog->Process();
		return FarDialog->GetExitCode();
	},
	-1);
}

void WINAPI apiDialogFree(HANDLE hDlg) noexcept
{
	return cpp_try(
	[&]
	{
		if (hDlg == INVALID_HANDLE_VALUE)
			return;

		const auto FarDialog = static_cast<Dialog*>(hDlg);
		if (FarDialog->GetCanLoseFocus())
			return;

		const auto Dlg = FarDialog->shared_from_this();

		for (const auto& i: *Global->CtrlObject->Plugins)
		{
			if (i->RemoveDialog(Dlg))
				break;
		}
	});
}

static const wchar_t* WINAPI apiGetMsgFn(const UUID* PluginId, intptr_t MsgId) noexcept
{
	return cpp_try(
	[&]
	{
		if (const auto pPlugin = UuidToPlugin(PluginId))
		{
			string_view Path = pPlugin->ModuleName();
			CutToSlash(Path);

			if (pPlugin->InitLang(Path, Global->Opt->strLanguage))
				return pPlugin->Msg(MsgId);
		}
		return L"";
	},
	L"");
}

intptr_t WINAPI apiMessageFn(const UUID* PluginId, const UUID* Id, unsigned long long Flags, const wchar_t* HelpTopic,
                        const wchar_t * const *Items,size_t ItemsNumber,
                        intptr_t ButtonsNumber) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		const auto CaptureErrors = (Flags & FMSG_ERRORTYPE) != 0;
		const error_state_ex ErrorState(CaptureErrors? os::last_error() : os::error_state{}, {}, CaptureErrors? errno : 0);

		if (Global->WindowManager->ManagerIsDown())
			return -1;

		if (Global->DisablePluginsOutput)
			return -1;

		if ((!(Flags&(FMSG_ALLINONE|FMSG_ERRORTYPE)) && ItemsNumber<2) || !Items)
			return -1;

		string Title;
		std::vector<string> MsgItems;
		std::vector<string> Buttons;

		switch (Flags & 0x000F0000)
		{
		case FMSG_MB_OK:
			Buttons = { msg(lng::MOk) };
			break;

		case FMSG_MB_OKCANCEL:
			Buttons = { msg(lng::MOk), msg(lng::MCancel) };
			break;

		case FMSG_MB_ABORTRETRYIGNORE:
			Buttons = { msg(lng::MAbort), msg(lng::MRetry), msg(lng::MIgnore) };
			break;

		case FMSG_MB_YESNO:
			Buttons = { msg(lng::MYes), msg(lng::MNo) };
			break;

		case FMSG_MB_YESNOCANCEL:
			Buttons = { msg(lng::MYes), msg(lng::MNo), msg(lng::MCancel) };
			break;

		case FMSG_MB_RETRYCANCEL:
			Buttons = { msg(lng::MRetry), msg(lng::MCancel) };
			break;
		}

		const auto AssignStrings = [&](auto&& Source)
		{
			if (Source.empty())
				return;

			Title = std::move(*Source.begin());
			if (Buttons.empty())
			{
				std::move(std::next(Source.begin()), Source.end() - ButtonsNumber, std::back_inserter(MsgItems));
				std::move(Source.end() - ButtonsNumber, Source.end(), std::back_inserter(Buttons));
			}
			else
			{
				// FMSG_MB_* is active
				std::move(std::next(Source.begin()), Source.end(), std::back_inserter(MsgItems));
			}
		};

		if (Flags & FMSG_ALLINONE)
		{
			std::vector<string> Strings;
			string_view StrItems = std::bit_cast<const wchar_t*>(Items);

			// Plugins expect that the trailing \n is ignored here, even though it's not promised anywhere.
			if (StrItems.ends_with(L'\n'))
				StrItems.remove_suffix(1);

			for (const auto& i: enum_tokens(StrItems, L"\n"sv))
			{
				Strings.emplace_back(i);
			}
			AssignStrings(std::move(Strings));
		}
		else
		{
			std::vector<const wchar_t*> ItemsCopy;
			ItemsCopy.reserve(ItemsNumber);
			// They believe nullptr works as empty string /o
			std::ranges::transform(Items, Items + ItemsNumber, std::back_inserter(ItemsCopy), NullToEmpty<wchar_t>);
			AssignStrings(std::move(ItemsCopy));
		}

		const auto PluginNumber = UuidToPlugin(PluginId);
		// запоминаем топик
		const auto strTopic = PluginNumber? help::make_topic(PluginNumber, NullToEmpty(HelpTopic)) : L""s;

		const DWORD InternalFlags =
			((Flags & FMSG_WARNING)? MSG_WARNING : 0) |
			((Flags & FMSG_KEEPBACKGROUND)? MSG_KEEPBACKGROUND : 0) |
			((Flags & FMSG_LEFTALIGN)? MSG_LEFTALIGN : 0);

		return static_cast<intptr_t>(Message(
			InternalFlags,
			Flags & FMSG_ERRORTYPE? &ErrorState : nullptr,
			Title,
			std::move(MsgItems),
			std::move(Buttons),
			strTopic, Id, PluginNumber)
		);
	},
	-1);
}

intptr_t WINAPI apiPanelControl(HANDLE hPlugin,FILE_CONTROL_COMMANDS Command,intptr_t Param1,void* Param2) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		if (Command == FCTL_CHECKPANELSEXIST)
			return Global->OnlyEditorViewerUsed? FALSE : TRUE;

		if (!Global->CtrlObject || Global->WindowManager->ManagerIsDown())
			return FALSE;

		if (Command == FCTL_GETUSERSCREEN)
		{
			Global->WindowManager->Desktop()->ConsoleSession().EnterPluginContext(!Param1);
			return TRUE;
		}

		if (Command == FCTL_SETUSERSCREEN)
		{
			Global->WindowManager->Desktop()->ConsoleSession().LeavePluginContext(!Param1);
			return TRUE;
		}

		if (Global->OnlyEditorViewerUsed)
			return FALSE;

		const auto FPanels = Global->CtrlObject->Cp();
		const auto CmdLine = Global->CtrlObject->CmdLine();

		switch (Command)
		{
		case FCTL_CLOSEPANEL:
		case FCTL_GETPANELINFO:
		case FCTL_GETPANELITEM:
		case FCTL_GETSELECTEDPANELITEM:
		case FCTL_GETCURRENTPANELITEM:
		case FCTL_GETPANELDIRECTORY:
		case FCTL_GETCOLUMNTYPES:
		case FCTL_GETCOLUMNWIDTHS:
		case FCTL_UPDATEPANEL:
		case FCTL_REDRAWPANEL:
		case FCTL_SETPANELDIRECTORY:
		case FCTL_BEGINSELECTION:
		case FCTL_SETSELECTION:
		case FCTL_CLEARSELECTION:
		case FCTL_ENDSELECTION:
		case FCTL_SETVIEWMODE:
		case FCTL_SETSORTMODE:
		case FCTL_SETSORTORDER:
		case FCTL_SETDIRECTORIESFIRST:
		case FCTL_GETPANELFORMAT:
		case FCTL_GETPANELHOSTFILE:
		case FCTL_GETPANELPREFIX:
		case FCTL_SETACTIVEPANEL:
		{
			if (!FPanels)
				return FALSE;

			if (!hPlugin || hPlugin == PANEL_ACTIVE || hPlugin == PANEL_PASSIVE)
			{
				const auto pPanel = (!hPlugin || hPlugin == PANEL_ACTIVE) ? FPanels->ActivePanel() : FPanels->PassivePanel();

				if (Command == FCTL_SETACTIVEPANEL && hPlugin == PANEL_ACTIVE)
					return TRUE;

				if (pPanel)
				{
					return pPanel->SetPluginCommand(Command,Param1,Param2);
				}

				return FALSE; //???
			}

			HANDLE hInternal;
			const auto LeftPanel = FPanels->LeftPanel();
			const auto RightPanel = FPanels->RightPanel();
			int Processed=FALSE;

			if (LeftPanel && LeftPanel->GetMode() == panel_mode::PLUGIN_PANEL)
			{
				if (const auto PlHandle = LeftPanel->GetPluginHandle())
				{
					hInternal=PlHandle->panel();

					if (hPlugin==hInternal)
					{
						Processed=LeftPanel->SetPluginCommand(Command,Param1,Param2);
					}
				}
			}

			if (RightPanel && RightPanel->GetMode() == panel_mode::PLUGIN_PANEL)
			{
				if (const auto PlHandle = RightPanel->GetPluginHandle())
				{
					hInternal=PlHandle->panel();

					if (hPlugin==hInternal)
					{
						Processed=RightPanel->SetPluginCommand(Command,Param1,Param2);
					}
				}
			}

			return Processed;
		}

		case FCTL_GETCMDLINE:
		{
			const auto& Str = CmdLine->GetString();
			if (Param1&&Param2)
			{
				xwcsncpy(static_cast<wchar_t*>(Param2), Str.c_str(), Param1);
			}

			return Str.size() + 1;
		}

		case FCTL_SETCMDLINE:
		case FCTL_INSERTCMDLINE:
		{
			{
				SCOPED_ACTION(SetAutocomplete)(CmdLine);
				if (Command==FCTL_SETCMDLINE)
					CmdLine->SetString(static_cast<const wchar_t*>(Param2), true);
				else
					CmdLine->InsertString(static_cast<const wchar_t*>(Param2));
			}
			CmdLine->Redraw();
			return TRUE;
		}

		case FCTL_SETCMDLINEPOS:
		{
			CmdLine->SetCurPos(Param1);
			CmdLine->Redraw();
			return TRUE;
		}

		case FCTL_GETCMDLINEPOS:
		{
			if (Param2)
			{
				*static_cast<int*>(Param2) = CmdLine->GetCurPos();
				return TRUE;
			}

			return FALSE;
		}

		case FCTL_GETCMDLINESELECTION:
		{
			const auto sel = static_cast<CmdLineSelect*>(Param2);
			if (CheckStructSize(sel))
			{
				CmdLine->GetSelection(sel->SelStart,sel->SelEnd);
				return TRUE;
			}

			return FALSE;
		}

		case FCTL_SETCMDLINESELECTION:
		{
			const auto sel=static_cast<const CmdLineSelect*>(Param2);
			if (CheckStructSize(sel))
			{
				CmdLine->Select(sel->SelStart,sel->SelEnd);
				CmdLine->Redraw();
				return TRUE;
			}

			return FALSE;
		}

		case FCTL_ISACTIVEPANEL:
		{
			if (!hPlugin || hPlugin == PANEL_ACTIVE)
				return TRUE;

			const auto pPanel = FPanels->ActivePanel();

			if (pPanel && (pPanel->GetMode() == panel_mode::PLUGIN_PANEL))
			{
				if (const auto PlHandle = pPanel->GetPluginHandle())
				{
					if (PlHandle->panel() == hPlugin)
						return TRUE;
				}
			}

			return FALSE;
		}

		default:
			return FALSE;
		}
	},
	FALSE);
}


HANDLE WINAPI apiSaveScreen(intptr_t X1,intptr_t Y1,intptr_t X2,intptr_t Y2) noexcept
{
	return cpp_try(
	[&]() -> SaveScreen*
	{
		if (Global->DisablePluginsOutput || Global->WindowManager->ManagerIsDown())
			return nullptr;

		if (X2 == -1)
			X2 = ScrX;

		if (Y2 == -1)
			Y2 = ScrY;

		return std::make_unique<SaveScreen>(rectangle{ static_cast<int>(X1), static_cast<int>(Y1), static_cast<int>(X2), static_cast<int>(Y2) }).release();
	},
	nullptr);
}

void WINAPI apiRestoreScreen(HANDLE hScreen) noexcept
{
	return cpp_try(
	[&]
	{
		std::unique_ptr<SaveScreen> Screen(static_cast<SaveScreen*>(hScreen));

		if (Global->DisablePluginsOutput || Global->WindowManager->ManagerIsDown())
			return;

		if (Screen)
		{
			Screen.reset();
			Global->ScrBuf->Flush();
		}
		else
		{
			Global->ScrBuf->FillBuf();
		}
	});
}

static void WINAPI apiFreeScreen(HANDLE hScreen) noexcept
{
	return cpp_try(
	[&]
	{
		std::unique_ptr<SaveScreen> const Screen(static_cast<SaveScreen*>(hScreen));

		if (Screen)
			Screen->Discard();
	});
}

namespace magic
{
	template<typename T>
	static auto CastVectorToRawData(std::unique_ptr<std::vector<T>>&& Items)
	{
		T Item{};
		Item.Reserved[0] = std::bit_cast<intptr_t>(Items.get());
		Items->emplace_back(Item);
		const std::tuple Result(Items->data(), Items->size() - 1);
		Items.release();
		return Result;
	}

	template<typename T>
	static auto CastRawDataToVector(std::span<T> const RawItems)
	{
		const auto Items = std::bit_cast<std::vector<T>*>(RawItems.data()[RawItems.size()].Reserved[0]);
		Items->pop_back(); // not needed anymore
		return std::unique_ptr<std::vector<T>>(Items);
	}
}

intptr_t WINAPI apiGetDirList(const wchar_t *Dir,PluginPanelItem **pPanelItem,size_t *pItemsNumber) noexcept
{
	return cpp_try(
	[&]
	{
		if (Global->WindowManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
			return FALSE;

		{
			const auto PR_FarGetDirListMsg = []
			{
				Message(0,
					{},
					{
						msg(lng::MPreparingList)
					},
					{});
			};

			SCOPED_ACTION(SaveScreen);
			os::fs::find_data FindData;
			string strFullName;
			ScanTree ScTree(false);
			ScTree.SetFindPath(ConvertNameToFull(Dir), L"*"sv);
			*pItemsNumber=0;
			*pPanelItem=nullptr;

			auto Items = std::make_unique<std::vector<PluginPanelItem>>();

			const time_check TimeCheck;
			bool MsgOut = false;
			while (ScTree.GetNextName(FindData,strFullName))
			{
				if (TimeCheck)
				{
					if (CheckForEscAndConfirmAbort())
					{
						FreePluginPanelItemsData(*Items);
						return FALSE;
					}

					if (!MsgOut)
					{
						HideCursor();
						PR_FarGetDirListMsg();
						MsgOut = true;
					}
				}

				FindData.FileName = strFullName;
				PluginPanelItemHolderHeapNonOwning Item;
				FindDataExToPluginPanelItemHolder(FindData, Item);
				Items->emplace_back(Item.Item);
			}

			std::tie(*pPanelItem, *pItemsNumber) = magic::CastVectorToRawData(std::move(Items));
		}
		return TRUE;
	},
	FALSE);
}

intptr_t WINAPI apiGetPluginDirList(const UUID* PluginId, HANDLE hPlugin, const wchar_t* Dir, PluginPanelItem** pPanelItem, size_t* pItemsNumber) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		if (Global->WindowManager->ManagerIsDown())
			return false;

		if (IsParentDirectory(Dir))
			return false;

		auto Items = std::make_unique<std::vector<PluginPanelItem>>();

		// BUGBUG This is API, shouldn't the callback be empty?

		const time_check TimeCheck;
		std::optional<dirinfo_progress> DirinfoProgress;

		const auto DirInfoCallback = [&](string_view const Name, unsigned long long const ItemsCount, unsigned long long const Size)
		{
			if (!TimeCheck)
				return;

			if (!DirinfoProgress)
				DirinfoProgress.emplace(msg(lng::MPreparingList));

			DirinfoProgress->set_name(Name);
			DirinfoProgress->set_count(ItemsCount);
			DirinfoProgress->set_size(Size);
		};

		const auto Result = GetPluginDirList(UuidToPlugin(PluginId), hPlugin, Dir, nullptr, *Items, DirInfoCallback);
		std::tie(*pPanelItem, *pItemsNumber) = magic::CastVectorToRawData(std::move(Items));
		return Result;
	},
	FALSE);
}

void WINAPI apiFreeDirList(PluginPanelItem *PanelItems, size_t ItemsNumber) noexcept
{
	return cpp_try(
	[&]
	{
		const auto Items = magic::CastRawDataToVector(std::span{ PanelItems, ItemsNumber });
		FreePluginPanelItemsData(*Items);
	});
}

static void WINAPI apiFreePluginDirList(HANDLE hPlugin, PluginPanelItem *PanelItems, size_t ItemsNumber) noexcept
{
	return cpp_try(
	[&]
	{
		const auto Items = magic::CastRawDataToVector(std::span{ PanelItems, ItemsNumber });
		FreePluginDirList(hPlugin, *Items);
	});
}

intptr_t WINAPI apiViewer(const wchar_t *FileName,const wchar_t *Title,
                     intptr_t X1,intptr_t Y1,intptr_t X2, intptr_t Y2,unsigned long long Flags, uintptr_t CodePage) noexcept
{
	return cpp_try(
	[&]
	{
		if (Global->WindowManager->ManagerIsDown())
			return FALSE;

		const auto DisableHistory = (Flags & VF_DISABLEHISTORY) != 0;

		// $ 15.05.2002 SKV - Запретим вызов немодального редактора viewer-а из модального.
		if (Global->WindowManager->InModal())
		{
			Flags&=~VF_NONMODAL;
		}

		if (Flags & VF_NONMODAL)
		{
			/* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
			const auto Viewer = FileViewer::create(
				FileName,
				true,
				DisableHistory,
				NullToEmpty(Title),
				{
					static_cast<int>(X1),
					static_cast<int>(Y1),
					static_cast<int>(X2),
					static_cast<int>(Y2)
				},
				CodePage);

			if (!Viewer)
				return FALSE;

			/* $ 14.06.2002 IS
			   Обработка VF_DELETEONLYFILEONCLOSE - этот флаг имеет более низкий
			   приоритет по сравнению с VF_DELETEONCLOSE
			*/
			if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
				Viewer->SetTempViewName(FileName, (Flags&VF_DELETEONCLOSE) != 0);

			Viewer->SetEnableF6(Flags & VF_ENABLE_F6);

			/* $ 21.05.2002 SKV
			  Запускаем свой цикл только если не был указан флаг.
			*/
			if (!(Flags&VF_IMMEDIATERETURN))
			{
				Global->WindowManager->ExecuteNonModal(Viewer);
			}
			else
			{
				if (Global->GlobalSaveScrPtr)
					Global->GlobalSaveScrPtr->Discard();

				Global->WindowManager->PluginCommit();
			}
		}
		else
		{
			/* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
			const auto Viewer = FileViewer::create(
				FileName,
				false,
				DisableHistory,
				NullToEmpty(Title),
				{
					static_cast<int>(X1),
					static_cast<int>(Y1),
					static_cast<int>(X2),
					static_cast<int>(Y2)
				},
				CodePage);

			Viewer->SetEnableF6(Flags & VF_ENABLE_F6);

			/* $ 28.05.2001 По умолчанию viewer, поэтому нужно здесь признак выставить явно */
			if(Viewer->GetExitCode()) Global->WindowManager->ExecuteModal(Viewer);

			/* $ 14.06.2002 IS
			   Обработка VF_DELETEONLYFILEONCLOSE - этот флаг имеет более низкий
			   приоритет по сравнению с VF_DELETEONCLOSE
			*/
			if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
				Viewer->SetTempViewName(FileName, (Flags&VF_DELETEONCLOSE) != 0);

			if (!Viewer->GetExitCode())
			{
				return FALSE;
			}
		}

		return TRUE;
	},
	FALSE);
}

intptr_t WINAPI apiEditor(const wchar_t* FileName, const wchar_t* Title, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2, unsigned long long Flags, intptr_t StartLine, intptr_t StartChar, uintptr_t CodePage) noexcept
{
	return cpp_try(
	[&]
	{
		if (Global->WindowManager->ManagerIsDown())
			return EEC_OPEN_ERROR;

		/* $ 12.07.2000 IS
		 Проверка флагов редактора (раньше они игнорировались) и открытие
		 немодального редактора, если есть соответствующий флаг
		 */
		const auto CreateNew = (Flags & EF_CREATENEW) != 0;
		const auto  Locked = (Flags & EF_LOCKED) != 0;
		const auto  DisableHistory = (Flags & EF_DISABLEHISTORY) != 0;
		const auto  DisableSavePos = (Flags & EF_DISABLESAVEPOS) != 0;
		/* $ 14.06.2002 IS
		   Обработка EF_DELETEONLYFILEONCLOSE - этот флаг имеет более низкий
		   приоритет по сравнению с EF_DELETEONCLOSE
		   */
		int DeleteOnClose = 0;

		if (Flags & EF_DELETEONCLOSE)
			DeleteOnClose = 1;
		else if (Flags & EF_DELETEONLYFILEONCLOSE)
			DeleteOnClose = 2;

		int OpMode = EF_OPENMODE_QUERY;

		if ((Flags&EF_OPENMODE_MASK))
			OpMode = Flags&EF_OPENMODE_MASK;

		/*$ 15.05.2002 SKV
		  Запретим вызов немодального редактора, если находимся в модальном
		  редакторе или viewer-е.
		  */
		if (Global->WindowManager->InModal())
		{
			Flags&=~EF_NONMODAL;
		}

		auto ExitCode = EEC_OPEN_ERROR;
		const string strTitle = NullToEmpty(Title);

		if (Flags & EF_NONMODAL)
		{
			/* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
			if (const auto Editor = FileEditor::create(NullToEmpty(FileName), CodePage,
				(CreateNew ? FFILEEDIT_CANNEWFILE : 0) | FFILEEDIT_ENABLEF6 |
				(DisableHistory ? FFILEEDIT_DISABLEHISTORY : 0) |
				(Locked ? FFILEEDIT_LOCKED : 0) |
				(DisableSavePos ? FFILEEDIT_DISABLESAVEPOS : 0),
				StartLine, StartChar, &strTitle,
				{ static_cast<int>(X1), static_cast<int>(Y1), static_cast<int>(X2), static_cast<int>(Y2) },
				DeleteOnClose, nullptr, OpMode))
			{
				const auto editorExitCode = Editor->GetExitCode();

				// добавочка - проверка кода возврата (почему возникает XC_OPEN_ERROR - см. код FileEditor::Init())
				if (editorExitCode == XC_OPEN_ERROR || editorExitCode == XC_LOADING_INTERRUPTED)
				{
					return editorExitCode == XC_OPEN_ERROR ? EEC_OPEN_ERROR : EEC_LOADING_INTERRUPTED;
				}
				else if (editorExitCode == XC_EXISTS)
				{
					if (Global->GlobalSaveScrPtr)
						Global->GlobalSaveScrPtr->Discard();

					Global->WindowManager->PluginCommit();

					if constexpr (features::mantis_2562)
					{
						return EEC_OPENED_EXISTING;
					}
					else
					{
						return EEC_MODIFIED;
					}
				}

				Editor->SetEnableF6((Flags & EF_ENABLE_F6) != 0);
				Editor->SetPluginTitle(&strTitle);

				/* $ 21.05.2002 SKV - Запускаем свой цикл, только если не был указан флаг. */
				if (!(Flags&EF_IMMEDIATERETURN))
				{
					Global->WindowManager->ExecuteNonModal(Editor);
					if (Global->WindowManager->IndexOf(Editor) != -1)
						ExitCode = Editor->WasFileSaved()? EEC_MODIFIED : EEC_NOT_MODIFIED;
					else
						ExitCode = EEC_NOT_MODIFIED;//??? editorExitCode
				}
				else
				{
					if (Global->GlobalSaveScrPtr)
						Global->GlobalSaveScrPtr->Discard();

					Global->WindowManager->PluginCommit();

					if constexpr (features::mantis_2562)
					{
						ExitCode = editorExitCode == XC_RELOAD ? EEC_RELOAD : Editor->WasFileSaved()? EEC_MODIFIED : EEC_NOT_MODIFIED;
					}
					else
					{
						ExitCode = EEC_MODIFIED;
					}
				}
			}
		}
		else
		{
			/* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
			const auto Editor = FileEditor::create(FileName, CodePage,
				(CreateNew ? FFILEEDIT_CANNEWFILE : 0) |
				(DisableHistory ? FFILEEDIT_DISABLEHISTORY : 0) |
				(Locked ? FFILEEDIT_LOCKED : 0) |
				(DisableSavePos ? FFILEEDIT_DISABLESAVEPOS : 0),
				StartLine, StartChar, &strTitle,
				{ static_cast<int>(X1), static_cast<int>(Y1), static_cast<int>(X2), static_cast<int>(Y2) },
				DeleteOnClose, nullptr, OpMode);

			// выполним предпроверку (ошибки разные могут быть)
			switch (const auto editorExitCode = Editor->GetExitCode())
			{
				case XC_OPEN_ERROR:
					return EEC_OPEN_ERROR;
				case XC_LOADING_INTERRUPTED:
				case XC_EXISTS:
					return EEC_LOADING_INTERRUPTED;
				default:
				{
					Editor->SetEnableF6((Flags & EF_ENABLE_F6) != 0);
					Editor->SetPluginTitle(&strTitle);
					/* $ 15.05.2002 SKV
					  Зафиксируем вход и выход в/из модального редактора.
					  */
					if (any_of(editorExitCode, -1, XC_OPEN_NEWINSTANCE))
						Global->WindowManager->ExecuteModal(Editor);

					if (Editor->GetExitCode() == XC_OPEN_ERROR)
					{
						ExitCode = EEC_OPEN_ERROR;
					}
					else
					{
#if 0

						if (OpMode == EF_OPENMODE_BREAKIFOPEN && ExitCode == XC_QUIT)
							ExitCode = XC_OPEN_ERROR;
						else
#endif
							ExitCode = Editor->WasFileSaved()? EEC_MODIFIED : EEC_NOT_MODIFIED;
					}
				}
				break;
			}
		}

		return ExitCode;
	},
	EEC_OPEN_ERROR);
}

void WINAPI apiText(intptr_t X,intptr_t Y,const FarColor* Color,const wchar_t *Str) noexcept
{
	return cpp_try(
	[&]
	{
		if (Global->DisablePluginsOutput || Global->WindowManager->ManagerIsDown())
			return;

		if (!Str)
		{
			const auto PrevLockCount = Global->ScrBuf->GetLockCount();
			Global->ScrBuf->SetLockCount(0);
			Global->ScrBuf->Flush();
			Global->ScrBuf->SetLockCount(PrevLockCount);
		}
		else
		{
			Text({ static_cast<int>(X), static_cast<int>(Y) }, *Color, Str);
		}
	});
}

template<class window_type>
static intptr_t apiTControl(intptr_t Id, auto Command, intptr_t Param1, void* Param2, auto Getter, auto Control)
{
	if (Global->WindowManager->ManagerIsDown())
		return 0;

	if (Id == -1)
	{
		const auto CurrentObject = std::invoke(Getter, Global->WindowManager);
		return CurrentObject? std::invoke(Control, CurrentObject, Command, Param1, Param2) : 0;
	}
	else
	{
		for (const auto i: std::views::iota(0uz, Global->WindowManager->GetWindowCount()))
		{
			if (const auto CurrentWindow = std::dynamic_pointer_cast<window_type>(Global->WindowManager->GetWindow(i)))
			{
				if (const auto CurrentControlWindow = CurrentWindow->GetById(Id))
				{
					return std::invoke(Control, CurrentControlWindow, Command, Param1, Param2);
				}
			}
		}
	}
	return 0;
}


intptr_t WINAPI apiEditorControl(intptr_t EditorID, EDITOR_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]
	{
		return apiTControl<FileEditor>(EditorID, Command, Param1, Param2, &Manager::GetCurrentEditor, &FileEditor::EditorControl);
	},
	0);
}

intptr_t WINAPI apiViewerControl(intptr_t ViewerID, VIEWER_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]
	{
		return apiTControl<ViewerContainer>(ViewerID, Command, Param1, Param2, &Manager::GetCurrentViewer, &Viewer::ViewerControl);
	},
	0);
}

static void WINAPI apiUpperBuf(wchar_t *Buf, intptr_t Length) noexcept
{
	return cpp_try(
	[&]
	{
		inplace::upper({ Buf, static_cast<size_t>(Length) });
	});
}

static void WINAPI apiLowerBuf(wchar_t *Buf, intptr_t Length) noexcept
{
	return cpp_try(
	[&]
	{
		inplace::lower({ Buf, static_cast<size_t>(Length) });
	});
}

static void WINAPI apiStrUpper(wchar_t *s1) noexcept
{
	return cpp_try(
	[&]
	{
		inplace::upper(s1);
	});
}

static void WINAPI apiStrLower(wchar_t *s1) noexcept
{
	return cpp_try(
	[&]
	{
		inplace::lower(s1);
	});
}

static wchar_t WINAPI apiUpper(wchar_t Ch) noexcept
{
	return cpp_try(
	[&]
	{
		return upper(Ch);
	},
	Ch);
}

static wchar_t WINAPI apiLower(wchar_t Ch) noexcept
{
	return cpp_try(
	[&]
	{
		return lower(Ch);
	},
	Ch);
}


static int WINAPI apiStrCmpNI(const wchar_t* Str1, const wchar_t* Str2, intptr_t MaxSize) noexcept
{
	return cpp_try(
	[&]
	{
		return pluginapi_sort_accessor::compare_ordinal_icase(string_view(Str1).substr(0, MaxSize), string_view(Str2).substr(0, MaxSize));
	},
	-1);
}

static int WINAPI apiStrCmpI(const wchar_t* Str1, const wchar_t* Str2) noexcept
{
	return cpp_try(
	[&]
	{
		return pluginapi_sort_accessor::compare_ordinal_icase(Str1, Str2);
	},
	-1);
}

static int WINAPI apiIsLower(wchar_t Ch) noexcept
{
	return cpp_try(
	[&]
	{
		return is_lower(Ch);
	},
	false);
}

static int WINAPI apiIsUpper(wchar_t Ch) noexcept
{
	return cpp_try(
	[&]
	{
		return is_upper(Ch);
	},
	false);
}

static int WINAPI apiIsAlpha(wchar_t Ch) noexcept
{
	return cpp_try(
	[&]
	{
		return is_alpha(Ch);
	},
	false);
}

static int WINAPI apiIsAlphaNum(wchar_t Ch) noexcept
{
	return cpp_try(
	[&]
	{
		return is_alphanumeric(Ch);
	},
	false);
}

static wchar_t* WINAPI apiTruncStr(wchar_t *Str,intptr_t MaxLength) noexcept
{
	return cpp_try(
	[&]
	{
		return legacy::truncate_left(Str, MaxLength);
	},
	Str);
}

static wchar_t* WINAPI apiTruncPathStr(wchar_t *Str, intptr_t MaxLength) noexcept
{
	return cpp_try(
	[&]
	{
		return legacy::truncate_path(Str, MaxLength);
	},
	Str);
}

static const wchar_t* WINAPI apiPointToName(const wchar_t* Path) noexcept
{
	return cpp_try(
	[&]
	{
		return Path? PointToName(Path).data() : nullptr;
	},
	Path);
}

size_t WINAPI apiGetFileOwner(const wchar_t *Computer, const wchar_t *Name, wchar_t *Owner, size_t Size) noexcept
{
	return cpp_try(
	[&]
	{
		string strOwner;
		if (!GetFileOwner(NullToEmpty(Computer), NullToEmpty(Name), strOwner))
			return 0uz;

		if (Owner && Size)
			xwcsncpy(Owner, strOwner.c_str(), Size);

		return strOwner.size() + 1;
	},
	0);

}

static size_t WINAPI apiConvertPath(CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, size_t DestSize) noexcept
{
	return cpp_try(
	[&]
	{
		string strDest;

		switch (Mode)
		{
		case CPM_NATIVE:
			strDest = nt_path(string_view{ Src });
			break;

		case CPM_REAL:
			strDest = ConvertNameToReal(Src);
			break;

		case CPM_FULL:
		default:
			strDest = ConvertNameToFull(Src);
			break;
		}

		if (Dest && DestSize)
			xwcsncpy(Dest, strDest.c_str(), DestSize);

		return strDest.size() + 1;
	},
	0);
}

size_t WINAPI apiGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest, size_t DestSize) noexcept
{
	return cpp_try(
	[&]
	{
		const string strSrc = Src;
		string strDest;
		AddEndSlash(strDest);
		if (!GetReparsePointInfo(strSrc, strDest, nullptr))
			return 0uz;

		if (DestSize && Dest)
			xwcsncpy(Dest,strDest.c_str(),DestSize);

		return strDest.size()+1;
	},
	0);
}

size_t WINAPI apiGetNumberOfLinks(const wchar_t* Name) noexcept
{
	return cpp_try(
	[&]
	{
		const auto Hardlinks = GetNumberOfLinks(Name);
		return Hardlinks? *Hardlinks : 1;
	},
	0);
}

size_t WINAPI apiGetPathRoot(const wchar_t *Path, wchar_t *Root, size_t DestSize) noexcept
{
	return cpp_try(
	[&]
	{
		const auto strRoot = GetPathRoot(Path);

		if (DestSize && Root)
			xwcsncpy(Root,strRoot.c_str(),DestSize);

		return strRoot.size()+1;
	},
	0);
}

BOOL WINAPI apiCopyToClipboard(enum FARCLIPBOARD_TYPE Type, const wchar_t *Data) noexcept
{
	return cpp_try(
	[&]
	{
		switch (Type)
		{
		case FCT_STREAM:
			return Data? SetClipboardText(Data) : ClearClipboard();

		case FCT_COLUMN:
			return Data? SetClipboardVText(Data) : ClearClipboard();

		default:
			return false;
		}
	},
	false);
}

static size_t apiPasteFromClipboardEx(bool Type, std::span<wchar_t> Data)
{
	string str;
	if(Type? GetClipboardVText(str) : GetClipboardText(str))
	{
		if(!Data.empty())
		{
			const auto Size = std::min(Data.size(), str.size() + 1);
			std::copy_n(str.data(), Size, Data.data());
		}
		return str.size() + 1;
	}
	return 0;
}

size_t WINAPI apiPasteFromClipboard(enum FARCLIPBOARD_TYPE Type, wchar_t *Data, size_t Length) noexcept
{
	return cpp_try(
	[&]
	{
		size_t size = 0;
		switch (Type)
		{
		case FCT_STREAM:
		{
			string str;
			if (GetClipboardVText(str))
			{
				break;
			}
		}
		[[fallthrough]];
		case FCT_ANY:
			size = apiPasteFromClipboardEx(false, { Data, Length });
			break;

		case FCT_COLUMN:
			size = apiPasteFromClipboardEx(true, { Data, Length });
			break;
		}
		return size;
	},
	0);
}

static unsigned long long WINAPI apiFarClock() noexcept
{
	return cpp_try(
	[&]
	{
		return Global->FarUpTime() / 1us;
	},
	0);
}

static int WINAPI apiCompareStrings(const wchar_t* Str1, size_t Size1, const wchar_t* Str2, size_t Size2) noexcept
{
	return cpp_try(
	[&]
	{
		return string_sort::ordering_as_int(string_sort::compare({ Str1, Size1 }, { Str2, Size2 }));
	},
	-1);
}

static uintptr_t WINAPI apiDetectCodePage(DetectCodePageInfo* Info) noexcept
{
	return cpp_try(
	[&]
	{
		assert(Info);
		assert(Info->StructSize);

		os::fs::file const File(Info->FileName, FILE_READ_DATA, os::fs::file_share_all, nullptr, OPEN_EXISTING);
		if (!File)
			return uintptr_t{};

		return GetFileCodepage(File, CP_UTF8);
	},
	0);
}

intptr_t WINAPI apiMacroControl(const UUID* PluginId, FAR_MACRO_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		if (!Global->CtrlObject)
			return false;

		auto& Macro = Global->CtrlObject->Macro; //??

		switch (Command)
		{
		// Param1=0, Param2 - FarMacroLoad*
		case MCTL_LOADALL: // из реестра в память ФАР с затиранием предыдущего
			{
				const auto Data = static_cast<const FarMacroLoad*>(Param2);
				return
					!Macro.IsRecording() &&
					(!Data || CheckStructSize(Data)) &&
					Macro.LoadMacros(false, !Macro.IsExecuting(), Data);
			}

		// Param1=0, Param2 - 0
		case MCTL_SAVEALL:
			return !Macro.IsRecording() && Macro.SaveMacros(true);

		// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroSendMacroText*
		case MCTL_SENDSTRING:
			{
				const auto Data = static_cast<const MacroSendMacroText*>(Param2);
				if (CheckStructSize(Data) && Data->SequenceText)
				{
					if (Param1 == MSSC_POST)
					{
						return Macro.PostNewMacro(Data->SequenceText, Data->Flags, InputRecordToKey(&Data->AKey));
					}
					else if (Param1 == MSSC_CHECK)
					{
						return Macro.ParseMacroString(Data->SequenceText, Data->Flags, false);
					}
				}
			}
			break;

		// Param1=0, Param2 - MacroExecuteString*
		case MCTL_EXECSTRING:
			{
				const auto Data = static_cast<MacroExecuteString*>(Param2);
				return CheckStructSize(Data) && Macro.ExecuteString(Data) ? 1 : 0;
			}

		// Param1=0, Param2 - 0
		case MCTL_GETSTATE:
			return Macro.GetState();

		// Param1=0, Param2 - 0
		case MCTL_GETAREA:
			return Macro.GetArea();

		case MCTL_ADDMACRO:
			{
				const auto Data = static_cast<const MacroAddMacroV1*>(Param2);
				if (CheckStructSize(Data) && Data->SequenceText)
				{
					return Macro.AddMacro(*PluginId, Data);
				}
			}
			break;

		case MCTL_DELMACRO:
			return Macro.DelMacro(*PluginId, Param2) ? 1 : 0;

		//Param1=size of buffer, Param2 - MacroParseResult*
		case MCTL_GETLASTERROR:
			{
				point ErrPos;
				string ErrSrc;

				const auto ErrCode = Macro.GetMacroParseError(ErrPos, ErrSrc);

				auto Size = static_cast<int>(aligned_sizeof<MacroParseResult, alignof(wchar_t)>);
				const size_t stringOffset = Size;
				Size += static_cast<int>((ErrSrc.size() + 1)*sizeof(wchar_t));

				const auto Result = static_cast<MacroParseResult*>(Param2);

				if (Param1 >= Size && CheckStructSize(Result))
				{
					Result->StructSize = sizeof(MacroParseResult);
					Result->ErrCode = ErrCode;
					Result->ErrPos = { static_cast<short>(ErrPos.x), static_cast<short>(ErrPos.y) };
					Result->ErrSrc = view_as<const wchar_t*>(Param2, stringOffset);
					assert(is_aligned(*Result->ErrSrc));
					*copy_string(ErrSrc, const_cast<wchar_t*>(Result->ErrSrc)) = {};
				}

				return Size;
			}

		default: //FIXME
			break;
		}

		return false;
	},
	false);
}

intptr_t WINAPI apiPluginsControl(HANDLE Handle, FAR_PLUGINS_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		switch (Command)
		{
		case PCTL_LOADPLUGIN:
		case PCTL_FORCEDLOADPLUGIN:
			if (Param1 == PLT_PATH && Param2)
				return std::bit_cast<intptr_t>(Global->CtrlObject->Plugins->LoadPluginExternal(ConvertNameToFull(static_cast<const wchar_t*>(Param2)), Command == PCTL_FORCEDLOADPLUGIN));
			break;

		case PCTL_FINDPLUGIN:
		{
			Plugin* plugin = nullptr;
			switch (Param1)
			{
			case PFM_GUID:
				plugin = Global->CtrlObject->Plugins->FindPlugin(*static_cast<UUID*>(Param2));
				break;

			case PFM_MODULENAME:
				{
					const auto strPath = ConvertNameToFull(static_cast<const wchar_t*>(Param2));
					const auto ItemIterator = std::ranges::find_if(*Global->CtrlObject->Plugins, [&](Plugin const* const i)
					{
						return equal_icase(i->ModuleName(), strPath);
					});
					if (ItemIterator != Global->CtrlObject->Plugins->cend())
					{
						plugin = *ItemIterator;
					}
					break;
				}

			default:
				break;
			}
			if (plugin && Global->CtrlObject->Plugins->IsPluginUnloaded(plugin))
				plugin = nullptr;
			return std::bit_cast<intptr_t>(plugin);
		}

		case PCTL_UNLOADPLUGIN:
			return Global->CtrlObject->Plugins->UnloadPluginExternal(static_cast<Plugin*>(Handle));

		case PCTL_GETPLUGININFORMATION:
			{
				const auto Info = static_cast<FarGetPluginInformation*>(Param2);
				if (Handle && (!Info || (CheckStructSize(Info) && static_cast<size_t>(Param1) > sizeof(*Info))))
				{
					return Global->CtrlObject->Plugins->GetPluginInformation(static_cast<Plugin*>(Handle), Info, Param1);
				}
			}
			break;

		case PCTL_GETPLUGINS:
			{
				const auto PluginsCount = Global->CtrlObject->Plugins->size();
				if (Param1 && Param2)
				{
					std::copy_n(Global->CtrlObject->Plugins->begin(), std::min(static_cast<size_t>(Param1), PluginsCount), static_cast<HANDLE*>(Param2));
				}
				return PluginsCount;
			}

		default:
			break;
		}
		return 0;
	},
	0);
}

static intptr_t WINAPI apiFileFilterControl(HANDLE hHandle, FAR_FILE_FILTER_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		if (Command != FFCTL_CREATEFILEFILTER && !hHandle)
			return false;

		switch (Command)
		{
		case FFCTL_CREATEFILEFILTER:
		{
			if (none_of(hHandle,
				nullptr,
				PANEL_ACTIVE,
				PANEL_PASSIVE,
				PANEL_NONE
			))
				return false;

			if (none_of(
				Param1,
				FFT_PANEL,
				FFT_FINDFILE,
				FFT_COPY,
				FFT_SELECT,
				FFT_CUSTOM
			))
				return false;

			if (!Param2)
				return false;

			*static_cast<multifilter**>(Param2) = std::make_unique<multifilter>(GetHostPanel(hHandle), static_cast<FAR_FILE_FILTER_TYPE>(Param1)).release();
			return true;
		}

		case FFCTL_FREEFILEFILTER:
			delete static_cast<multifilter*>(hHandle);
			return true;

		case FFCTL_OPENFILTERSMENU:
			filters::EditFilters(static_cast<multifilter*>(hHandle)->area(), static_cast<multifilter*>(hHandle)->panel());
			return true;

		case FFCTL_STARTINGTOFILTER:
			static_cast<multifilter*>(hHandle)->UpdateCurrentTime();
			return true;

		case FFCTL_ISFILEINFILTER:
			if (!Param2)
				break;
			return static_cast<multifilter*>(hHandle)->FileInFilter(*static_cast<const PluginPanelItem*>(Param2));
		}
		return false;
	},
	false);
}

static intptr_t WINAPI apiRegExpControl(HANDLE hHandle, FAR_REGEXP_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		if (Command != RECTL_CREATE && !hHandle)
			return false;

		struct regex_handle
		{
			RegExp Regex;
			named_regex_match NamedMatch;
		};

		switch (Command)
		{
		case RECTL_CREATE:
			*static_cast<regex_handle**>(Param2) = std::make_unique<regex_handle>().release();
			return true;

		case RECTL_FREE:
			delete static_cast<regex_handle const*>(hHandle);
			return true;

		case RECTL_COMPILE:
			try
			{
				static_cast<regex_handle*>(hHandle)->Regex.Compile(static_cast<const wchar_t*>(Param2), OP_PERLSTYLE);
				return true;
			}
			catch (regex_exception const& e)
			{
				LOGERROR(L"RECTL_COMPILE error: {}; position {}"sv, e.message(), e.position());
				return false;
			}

		case RECTL_OPTIMIZE:
			return static_cast<regex_handle*>(hHandle)->Regex.Optimize();

		case RECTL_MATCHEX:
		{
			auto& Handle = *static_cast<regex_handle*>(hHandle);
			const auto data = static_cast<RegExpSearch*>(Param2);
			regex_match Match;

			if (!Handle.Regex.MatchEx({ data->Text, static_cast<size_t>(data->Length) }, data->Position, Match, &Handle.NamedMatch))
				return false;

			const auto MaxSize = std::min(static_cast<size_t>(data->Count), Match.Matches.size());
			std::copy_n(Match.Matches.cbegin(), MaxSize, data->Match);
			data->Count = MaxSize;
			return true;
		}

		case RECTL_SEARCHEX:
		{
			auto& Handle = *static_cast<regex_handle*>(hHandle);
			const auto data = static_cast<RegExpSearch*>(Param2);
			regex_match Match;

			if (!Handle.Regex.SearchEx({ data->Text, static_cast<size_t>(data->Length) }, data->Position, Match, &Handle.NamedMatch))
				return false;

			const auto MaxSize = std::min(static_cast<size_t>(data->Count), Match.Matches.size());
			std::copy_n(Match.Matches.cbegin(), MaxSize, data->Match);
			data->Count = MaxSize;
			return true;
		}

		case RECTL_BRACKETSCOUNT:
			return static_cast<regex_handle const*>(hHandle)->Regex.GetBracketsCount();

		case RECTL_NAMEDGROUPINDEX:
		{
			const auto& Handle = *static_cast<regex_handle const*>(hHandle);
			const auto Str = static_cast<wchar_t const*>(Param2);
			const auto Iterator = Handle.NamedMatch.Matches.find(Str);
			return Iterator == Handle.NamedMatch.Matches.cend()? 0 : Iterator->second;
		}

		default:
			return false;
		}
	},
	false);
}

intptr_t WINAPI apiSettingsControl(HANDLE hHandle, FAR_SETTINGS_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	return cpp_try(
	[&]() -> intptr_t
	{
		if (Command != SCTL_CREATE && !hHandle)
			return false;

		switch (Command)
		{
		case SCTL_CREATE:
		{
			const auto data = static_cast<FarSettingsCreate*>(Param2);
			if (!CheckStructSize(data))
				return false;

			if (data->Guid == FarUuid)
			{
				data->Handle = AbstractSettings::CreateFarSettings().release();
				return true;
			}

			if (!Global->CtrlObject->Plugins->FindPlugin(data->Guid))
				return false;

			auto Settings = AbstractSettings::CreatePluginSettings(data->Guid, Param1 == PSL_LOCAL);
			if (!Settings)
				return false;

			data->Handle = Settings.release();
			return true;
		}

		case SCTL_FREE:
			delete static_cast<AbstractSettings const*>(hHandle);
			return true;

		case SCTL_SET:
			{
				const auto Item = static_cast<FarSettingsItem const*>(Param2);
				return CheckStructSize(Item) && static_cast<AbstractSettings*>(hHandle)->Set(*Item);
			}

		case SCTL_GET:
			{
				const auto Item = static_cast<FarSettingsItem*>(Param2);
				return CheckStructSize(Item) && static_cast<AbstractSettings*>(hHandle)->Get(*Item);
			}

		case SCTL_ENUM:
			{
				const auto Enum = static_cast<FarSettingsEnum*>(Param2);
				return CheckStructSize(Enum) && static_cast<AbstractSettings*>(hHandle)->Enum(*Enum);
			}

		case SCTL_DELETE:
			{
				const auto Value = static_cast<FarSettingsValue const*>(Param2);
				return CheckStructSize(Value) && static_cast<AbstractSettings*>(hHandle)->Delete(*Value);
			}

		case SCTL_CREATESUBKEY:
		case SCTL_OPENSUBKEY:
			{
				const auto Value = static_cast<FarSettingsValue const*>(Param2);
				return CheckStructSize(Value)? static_cast<AbstractSettings*>(hHandle)->SubKey(*Value, Command == SCTL_CREATESUBKEY) : 0;
			}
		}
		return false;
	},
	false);
}

static size_t WINAPI apiGetCurrentDirectory(size_t Size, wchar_t* Buffer) noexcept
{
	return cpp_try(
	[&]
	{
		const auto strCurDir = os::fs::get_current_directory();

		if (Buffer && Size)
		{
			xwcsncpy(Buffer, strCurDir.c_str(), Size);
		}

		return strCurDir.size() + 1;
	},
	0);
}

static size_t WINAPI apiFormatFileSize(unsigned long long Size, intptr_t Width, FARFORMATFILESIZEFLAGS Flags, wchar_t *Dest, size_t DestSize) noexcept
{
	return cpp_try(
	[&]
	{
		static const std::pair<unsigned long long, unsigned long long> FlagsPair[]
		{
			{ FFFS_COMMAS,         COLFLAGS_GROUPDIGITS     },    // Вставлять разделитель между тысячами
			{ FFFS_THOUSAND,       COLFLAGS_THOUSAND        },    // Вместо делителя 1024 использовать делитель 1000
			{ FFFS_FLOATSIZE,      COLFLAGS_FLOATSIZE       },    // Показывать размер в виде десятичной дроби, используя наиболее подходящую единицу измерения, например 0,97 К, 1,44 М, 53,2 Г.
			{ FFFS_ECONOMIC,       COLFLAGS_ECONOMIC        },    // Экономичный режим, не показывать пробел перед суффиксом размера файла (т.е. 0.97K)
			{ FFFS_MINSIZEINDEX,   COLFLAGS_USE_MULTIPLIER  },    // Минимально допустимая единица измерения при форматировании
			{ FFFS_SHOWBYTESINDEX, COLFLAGS_SHOW_MULTIPLIER },    // Показывать суффиксы B,K,M,G,T,P,E
		};

		const auto strDestStr = FileSizeToStr(Size, Width, std::ranges::fold_left(FlagsPair, Flags & COLFLAGS_MULTIPLIER_MASK, [Flags](auto FinalFlags, const auto& i)
		{
			return FinalFlags | ((Flags & i.first) ? i.second : 0);
		}));

		if (Dest && DestSize)
		{
			xwcsncpy(Dest,strDestStr.c_str(),DestSize);
		}

		return strDestStr.size()+1;
	},
	0);
}

void WINAPI apiRecursiveSearch(const wchar_t *InitDir, const wchar_t *Mask, FRSUSERFUNC Func, unsigned long long Flags, void *Param) noexcept
{
	return cpp_try(
	[&]
	{
		filemasks FMask;

		if (!FMask.assign(Mask, FMF_SILENT)) return;

		Flags=Flags&0x000000FF; // только младший байт!
		ScanTree ScTree((Flags & FRS_RETUPDIR)!=0, (Flags & FRS_RECUR)!=0, (Flags & FRS_SCANSYMLINK)!=0, true);
		os::fs::find_data FindData;
		string strFullName;
		ScTree.SetFindPath(InitDir, L"*"sv);

		bool Found = false;
		while (!Found && ScTree.GetNextName(FindData,strFullName))
		{
			if (FMask.check(FindData.FileName))
			{
				PluginPanelItemHolderHeap fdata;
				FindDataExToPluginPanelItemHolder(FindData, fdata);
				Found = !Func(&fdata.Item, strFullName.c_str(), Param);
			}
		}
	});
}

size_t WINAPI apiMkTemp(wchar_t* Dest, size_t DestSize, const wchar_t *Prefix) noexcept
{
	return cpp_try(
	[&]
	{
		const auto strDest = MakeTemp(NullToEmpty(Prefix));
		if (Dest && DestSize)
		{
			xwcsncpy(Dest, strDest.c_str(), DestSize);
		}
		return strDest.size() + 1;
	},
	0);
}

size_t WINAPI apiProcessName(const wchar_t *param1, wchar_t *param2, size_t size, PROCESSNAME_FLAGS flags) noexcept
{
	return cpp_try(
	[&]() -> size_t
	{
		//             0xFFFF - length
		//           0xFF0000 - mode
		// 0xFFFFFFFFFF000000 - flags

		const PROCESSNAME_FLAGS Flags = flags&0xFFFFFFFFFF000000;

		switch(const PROCESSNAME_FLAGS Mode = flags & 0xFF0000)
		{
		case PN_CMPNAME:
			return CmpName(param1, param2, (Flags&PN_SKIPPATH)!=0);

		case PN_CMPNAMELIST:
		case PN_CHECKMASK:
		{
			static filemasks Masks;
			static string PrevMask;
			static bool ValidMask = false;
			if(PrevMask != param1)
			{
				ValidMask = Masks.assign(param1, FMF_SILENT);
				PrevMask = param1;
			}
			bool Result = false;
			if(ValidMask)
			{
				Result = Mode == PN_CHECKMASK || Masks.check((Flags&PN_SKIPPATH)? PointToName(param2) : param2);
			}
			else
			{
				if(Flags&PN_SHOWERRORMESSAGE)
				{
					Masks.ErrorMessage();
				}
			}
			return Result;
		}

		case PN_GENERATENAME:
		{
			string_view const SrcName = NullToEmpty(param1);
			const size_t Size = flags & 0xFFFF;
			auto const SrcNamePart = Size? SrcName.substr(0, Size) : SrcName;

			auto strResult = ConvertWildcards(SrcNamePart, NullToEmpty(param2));
			if (Size && Size < SrcName.size())
				strResult += SrcName.substr(Size);

			xwcsncpy(param2, strResult.c_str(), size);
			return strResult.size() + 1;
		}

		default:
			return false;
		}
	},
	0);
}

static BOOL WINAPI apiColorDialog(const UUID* PluginId, COLORDIALOGFLAGS Flags, FarColor *Color) noexcept
{
	return cpp_try(
	[&]
	{
		return !Global->WindowManager->ManagerIsDown() && GetColorDialog(*Color, true);
	},
	false);
}

static size_t WINAPI apiInputRecordToKeyName(const INPUT_RECORD* Key, wchar_t *KeyText, size_t Size) noexcept
{
	return cpp_try(
	[&]() -> size_t
	{
		const auto iKey = InputRecordToKey(Key);
		if (iKey == KEY_NONE)
			return 0;

		const auto strKT = KeyToText(iKey);
		if (strKT.empty())
			return 0;

		auto len = strKT.size();
		if (Size && KeyText)
		{
			if (Size <= len)
				len = Size - 1;
			std::copy_n(strKT.data(), len, KeyText);
			KeyText[len] = 0;
		}
		else if (KeyText)
			*KeyText = 0;
		return len + 1;
	},
	0);
}

static BOOL WINAPI apiKeyNameToInputRecord(const wchar_t *Name, INPUT_RECORD* RecKey) noexcept
{
	return cpp_try(
	[&]
	{
		const auto Key = KeyNameToKey(Name);
		return Key && KeyToInputRecord(Key, RecKey);
	},
	false);
}

BOOL WINAPI apiMkLink(const wchar_t *Target, const wchar_t *LinkName, LINK_TYPE Type, MKLINK_FLAGS Flags) noexcept
{
	return cpp_try(
	[&]
	{
		bool Result{};

		if (Target && *Target && LinkName && *LinkName)
		{
			switch (Type)
			{
			case LINK_HARDLINK:
				{
					std::optional<error_state_ex> ErrorState;
					Result = MkHardLink(Target, LinkName, ErrorState, (Flags & MLF_SHOWERRMSG) == 0);
				}
				break;

			case LINK_JUNCTION:
			case LINK_VOLMOUNT:
			case LINK_SYMLINKFILE:
			case LINK_SYMLINKDIR:
			case LINK_SYMLINK:
			{
				auto LinkType = RP_JUNCTION;

				switch (Type)
				{
				case LINK_VOLMOUNT:
					LinkType = RP_VOLMOUNT;
					break;
				case LINK_SYMLINK:
					LinkType = RP_SYMLINK;
					break;
				case LINK_SYMLINKFILE:
					LinkType = RP_SYMLINKFILE;
					break;
				case LINK_SYMLINKDIR:
					LinkType = RP_SYMLINKDIR;
					break;
				default:
					break;
				}

				std::optional<error_state_ex> ErrorState;
				Result = MkSymLink(Target, LinkName, LinkType, ErrorState, (Flags&MLF_SHOWERRMSG) == 0, (Flags&MLF_HOLDTARGET) != 0);
				break;
			}

			default:
				break;
			}
		}

		if (Result && !(Flags&MLF_DONOTUPDATEPANEL))
			ShellUpdatePanels(nullptr, false);

		return Result;
	},
	false);
}

static BOOL WINAPI apiAddEndSlash(wchar_t *Path) noexcept
{
	return cpp_try(
	[&]
	{
		return legacy::AddEndSlash(Path);
	},
	false);
}

wchar_t* WINAPI apiXlat(wchar_t *Line, intptr_t StartPos, intptr_t EndPos, XLAT_FLAGS Flags) noexcept
{
	return cpp_try(
	[&]
	{
		Xlat({ Line + StartPos, Line + EndPos }, Flags);
		return Line;
	},
	Line);
}

HANDLE WINAPI apiCreateFile(const wchar_t *Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile) noexcept
{
	return cpp_try(
	[&]
	{
		const auto Result = os::fs::create_file(Object, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile).release();
		return Result? Result : INVALID_HANDLE_VALUE;
	},
	INVALID_HANDLE_VALUE);
}

DWORD WINAPI apiGetFileAttributes(const wchar_t *FileName) noexcept
{
	return cpp_try(
	[&]
	{
		return os::fs::get_file_attributes(FileName);
	},
	INVALID_FILE_ATTRIBUTES);
}

BOOL WINAPI apiSetFileAttributes(const wchar_t *FileName, DWORD dwFileAttributes) noexcept
{
	return cpp_try(
	[&]
	{
		return os::fs::set_file_attributes(FileName, dwFileAttributes);
	},
	false);
}

BOOL WINAPI apiMoveFileEx(const wchar_t *ExistingFileName, const wchar_t *NewFileName, DWORD dwFlags) noexcept
{
	return cpp_try(
	[&]
	{
		return os::fs::move_file(ExistingFileName, NewFileName, dwFlags);
	},
	false);
}

BOOL WINAPI apiDeleteFile(const wchar_t *FileName) noexcept
{
	return cpp_try(
	[&]
	{
		return os::fs::delete_file(FileName);
	},
	false);
}

BOOL WINAPI apiRemoveDirectory(const wchar_t *DirName) noexcept
{
	return cpp_try(
	[&]
	{
		return os::fs::remove_directory(DirName);
	},
	false);
}

BOOL WINAPI apiCreateDirectory(const wchar_t* PathName, SECURITY_ATTRIBUTES* SecurityAttributes) noexcept
{
	return cpp_try(
	[&]
	{
		return os::fs::create_directory(PathName, SecurityAttributes);
	},
	false);
}

intptr_t WINAPI apiCallFar(intptr_t CheckCode, FarMacroCall* Data) noexcept
{
	return cpp_try(
	[&]
	{
		if (Global->CtrlObject)
			Global->CtrlObject->Macro.CallFar(CheckCode, Data);

		return 0;
	},
	0);
}

}

static constexpr FarStandardFunctions NativeFSF
{
	sizeof(NativeFSF),
	pluginapi::apiAtoi,
	pluginapi::apiAtoi64,
	pluginapi::apiItoa,
	pluginapi::apiItoa64,
	pluginapi::apiSprintf,
	pluginapi::apiSscanf,
	pluginapi::apiQsort,
	pluginapi::apiBsearch,
	pluginapi::apiSnprintf,
	pluginapi::apiIsLower,
	pluginapi::apiIsUpper,
	pluginapi::apiIsAlpha,
	pluginapi::apiIsAlphaNum,
	pluginapi::apiUpper,
	pluginapi::apiLower,
	pluginapi::apiUpperBuf,
	pluginapi::apiLowerBuf,
	pluginapi::apiStrUpper,
	pluginapi::apiStrLower,
	pluginapi::apiStrCmpI,
	pluginapi::apiStrCmpNI,
	pluginapi::apiUnquote,
	pluginapi::apiRemoveLeadingSpaces,
	pluginapi::apiRemoveTrailingSpaces,
	pluginapi::apiRemoveExternalSpaces,
	pluginapi::apiTruncStr,
	pluginapi::apiTruncPathStr,
	pluginapi::apiQuoteSpaceOnly,
	pluginapi::apiPointToName,
	pluginapi::apiGetPathRoot,
	pluginapi::apiAddEndSlash,
	pluginapi::apiCopyToClipboard,
	pluginapi::apiPasteFromClipboard,
	pluginapi::apiInputRecordToKeyName,
	pluginapi::apiKeyNameToInputRecord,
	pluginapi::apiXlat,
	pluginapi::apiGetFileOwner,
	pluginapi::apiGetNumberOfLinks,
	pluginapi::apiRecursiveSearch,
	pluginapi::apiMkTemp,
	pluginapi::apiProcessName,
	pluginapi::apiMkLink,
	pluginapi::apiConvertPath,
	pluginapi::apiGetReparsePointInfo,
	pluginapi::apiGetCurrentDirectory,
	pluginapi::apiFormatFileSize,
	pluginapi::apiFarClock,
	pluginapi::apiCompareStrings,
	pluginapi::apiDetectCodePage,
};

static constexpr PluginStartupInfo NativeInfo
{
	sizeof(NativeInfo),
	nullptr, //ModuleName, dynamic
	pluginapi::apiMenuFn,
	pluginapi::apiMessageFn,
	pluginapi::apiGetMsgFn,
	pluginapi::apiPanelControl,
	pluginapi::apiSaveScreen,
	pluginapi::apiRestoreScreen,
	pluginapi::apiGetDirList,
	pluginapi::apiGetPluginDirList,
	pluginapi::apiFreeDirList,
	pluginapi::apiFreePluginDirList,
	pluginapi::apiViewer,
	pluginapi::apiEditor,
	pluginapi::apiText,
	pluginapi::apiEditorControl,
	nullptr, // FSF, dynamic
	pluginapi::apiShowHelp,
	pluginapi::apiAdvControl,
	pluginapi::apiInputBox,
	pluginapi::apiColorDialog,
	pluginapi::apiDialogInit,
	pluginapi::apiDialogRun,
	pluginapi::apiDialogFree,
	pluginapi::apiSendDlgMessage,
	pluginapi::apiDefDlgProc,
	pluginapi::apiViewerControl,
	pluginapi::apiPluginsControl,
	pluginapi::apiFileFilterControl,
	pluginapi::apiRegExpControl,
	pluginapi::apiMacroControl,
	pluginapi::apiSettingsControl,
	nullptr, // Private, dynamic
	nullptr, // Instance, dynamic
	pluginapi::apiFreeScreen,
};

void CreatePluginStartupInfo(PluginStartupInfo* PSI, FarStandardFunctions* FSF)
{
	*PSI = NativeInfo;
	*FSF = NativeFSF;
	PSI->FSF = FSF;
}
