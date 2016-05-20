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

#include "headers.hpp"
#pragma hdrstop

#include "plugapi.hpp"
#include "keys.hpp"
#include "help.hpp"
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
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "colormix.hpp"
#include "message.hpp"
#include "eject.hpp"
#include "filefilter.hpp"
#include "fileowner.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "exitcode.hpp"
#include "processname.hpp"
#include "RegExp.hpp"
#include "TaskBar.hpp"
#include "console.hpp"
#include "plugsettings.hpp"
#include "farversion.hpp"
#include "mix.hpp"
#include "FarGuid.hpp"
#include "clipboard.hpp"
#include "strmix.hpp"
#include "notification.hpp"
#include "panelmix.hpp"
#include "xlat.hpp"
#include "dirinfo.hpp"
#include "language.hpp"
#include "viewer.hpp"
#include "datetime.hpp"

static inline Plugin* GuidToPlugin(const GUID* Id) { return (Id && Global->CtrlObject) ? Global->CtrlObject->Plugins->FindPlugin(*Id) : nullptr; }

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

	void* bsearchex(const void* key, const void* base, size_t nelem, size_t width, comparer user_comparer, void* user_param)
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

	void qsortex(char *base, size_t nel, size_t width, comparer user_comparer, void *user_param)
	{
		qsort_comparer = user_comparer;
		qsort_param = user_param;
		return std::qsort(base, nel, width, qsort_comparer_wrapper);
	}
};

namespace pluginapi
{
int WINAPIV apiSprintf(wchar_t* Dest, const wchar_t* Format, ...) noexcept //?deprecated
{
	try
	{
		va_list argptr;
		va_start(argptr, Format);
		SCOPE_EXIT{ va_end(argptr); };
		// BUGBUG, do not use vswprintf here, %s treated as char* in GCC
		return _vsnwprintf(Dest, 32000, Format, argptr);
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

int WINAPIV apiSnprintf(wchar_t* Dest, size_t Count, const wchar_t* Format, ...) noexcept
{
	try
	{
		va_list argptr;
		va_start(argptr, Format);
		SCOPE_EXIT{ va_end(argptr); };
		// BUGBUG, do not use vswprintf here, %s treated as char* in GCC
		return _vsnwprintf(Dest, Count, Format, argptr);
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

int WINAPIV apiSscanf(const wchar_t* Src, const wchar_t* Format, ...) noexcept
{
	try
	{
		va_list argptr;
		va_start(argptr, Format);
		SCOPE_EXIT{ va_end(argptr); };
		return vswscanf(Src, Format, argptr);
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

wchar_t *WINAPI apiItoa(int value, wchar_t *string, int radix) noexcept
{
	try
	{
		return _itow(value,string,radix);
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

wchar_t *WINAPI apiItoa64(__int64 value, wchar_t *string, int radix) noexcept
{
	try
	{
		return _i64tow(value, string, radix);
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

int WINAPI apiAtoi(const wchar_t *s) noexcept
{
	try
	{
		return static_cast<int>(std::wcstol(s, nullptr, 10));
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

__int64 WINAPI apiAtoi64(const wchar_t *s) noexcept
{
	try
	{
		return std::wcstoll(s, nullptr, 10);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

void WINAPI apiQsort(void *base, size_t nelem, size_t width, cfunctions::comparer fcmp, void *user) noexcept
{
	try
	{
		return cfunctions::qsortex((char*)base,nelem,width,fcmp,user);
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

void *WINAPI apiBsearch(const void *key, const void *base, size_t nelem, size_t width, cfunctions::comparer fcmp, void *user) noexcept
{
	try
	{
		return cfunctions::bsearchex(key, base, nelem, width, fcmp, user);
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

wchar_t* WINAPI apiQuoteSpace(wchar_t *Str) noexcept
{
	try
	{
		return QuoteSpace(Str);
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

wchar_t* WINAPI apiInsertQuote(wchar_t *Str) noexcept
{
	try
	{
		return InsertQuote(Str);
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

void WINAPI apiUnquote(wchar_t *Str) noexcept
{	try
	{
		return Unquote(Str);
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

wchar_t* WINAPI apiRemoveLeadingSpaces(wchar_t *Str) noexcept
{
	try
	{
		return RemoveLeadingSpaces(Str);
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

wchar_t * WINAPI apiRemoveTrailingSpaces(wchar_t *Str) noexcept
{
	try
	{
		return RemoveTrailingSpaces(Str);
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

wchar_t* WINAPI apiRemoveExternalSpaces(wchar_t *Str) noexcept
{
	try
	{
		return RemoveExternalSpaces(Str);
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

wchar_t* WINAPI apiQuoteSpaceOnly(wchar_t *Str) noexcept
{
	try
	{
		return QuoteSpaceOnly(Str);
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

intptr_t WINAPI apiInputBox(
    const GUID* PluginId,
    const GUID* Id,
    const wchar_t *Title,
    const wchar_t *Prompt,
    const wchar_t *HistoryName,
    const wchar_t *SrcText,
    wchar_t *DestText,
    size_t DestSize,
    const wchar_t *HelpTopic,
    unsigned __int64 Flags
) noexcept
{
	try
	{
		if (Global->WindowManager->ManagerIsDown())
			return FALSE;

		string strDest;
		int nResult = GetString(Title, Prompt, HistoryName, SrcText, strDest, HelpTopic, Flags&~FIB_CHECKBOX, nullptr, nullptr, GuidToPlugin(PluginId), Id);
		xwcsncpy(DestText, strDest.data(), DestSize);
		return nResult;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

/* Функция вывода помощи */
BOOL WINAPI apiShowHelp(const wchar_t *ModuleName, const wchar_t *HelpTopic, FARHELPFLAGS Flags) noexcept
{
	try
	{
		if (Global->WindowManager->ManagerIsDown())
			return FALSE;

		if (!HelpTopic)
			HelpTopic = L"Contents";

		UINT64 OFlags = Flags;
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
			if (!*ModuleName || *reinterpret_cast<const GUID*>(ModuleName) == FarGuid)
			{
				Flags |= FHELP_FARHELP;
				strTopic = HelpTopic + ((*HelpTopic == L':') ? 1 : 0);
			}
			else
			{
				if (const auto plugin = Global->CtrlObject->Plugins->FindPlugin(*reinterpret_cast<const GUID*>(ModuleName)))
				{
					Flags |= FHELP_CUSTOMPATH;
					strTopic = Help::MakeLink(ExtractFilePath(plugin->GetModuleName()), HelpTopic);
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
				string strPath;
				if (Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE | FHELP_CUSTOMPATH)))
				{
					strPath = ModuleName;

					if (Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE)))
					{
						if (Flags&FHELP_CUSTOMFILE)
							strMask = PointToName(strPath);
						else
							strMask.clear();

						CutToSlash(strPath);
					}
				}
				else
					return FALSE;

				strTopic = Help::MakeLink(strPath, HelpTopic);
			}
			else
				return FALSE;
		}

		return !Help::create(strTopic, strMask.data(), OFlags)->GetError();
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

/* $ 05.07.2000 IS
  Функция, которая будет действовать и в редакторе, и в панелях, и...
*/
intptr_t WINAPI apiAdvControl(const GUID* PluginId, ADVANCED_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	try
	{
		if (ACTL_SYNCHRO==Command) //must be first
		{
			MessageManager().notify(plugin_synchro, std::make_pair(*PluginId, Param2));
			return 0;
		}
		if (ACTL_GETWINDOWTYPE==Command)
		{
			WindowType* info=(WindowType*)Param2;
			if (CheckStructSize(info))
			{
				WINDOWINFO_TYPE type=WindowTypeToPluginWindowType(Manager::GetCurrentWindowType());
				switch(type)
				{
				case WTYPE_DESKTOP:
				case WTYPE_PANELS:
				case WTYPE_VIEWER:
				case WTYPE_EDITOR:
				case WTYPE_DIALOG:
				case WTYPE_VMENU:
				case WTYPE_HELP:
				case WTYPE_COMBOBOX:
				//case WTYPE_FINDFOLDER:
				//case WTYPE_GRABBER:
				//case WTYPE_HMENU:
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
				*(VersionInfo*)Param2=FAR_VERSION;

			return TRUE;

		/* $ 24.08.2000 SVS
			ожидать определенную (или любую) клавишу
			(const INPUT_RECORD*)Param2 - код клавиши, которую ожидаем, или nullptr
			если все равно какую клавишу ждать.
			возвращает 0;
		*/
		case ACTL_WAITKEY:
			WaitKey(Param2?InputRecordToKey((const INPUT_RECORD*)Param2):-1,0,false);
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
				Global->Opt->Palette.CopyTo(reinterpret_cast<FarColor*>(Param2), Param1);
			}
			return Global->Opt->Palette.size();

		/*
			Param=FARColor{
			DWORD Flags;
			int StartIndex;
			int ColorItem;
			LPBYTE Colors;
			};
		*/
		case ACTL_SETARRAYCOLOR:
		{
			FarSetColors *Pal=(FarSetColors*)Param2;
			if (CheckStructSize(Pal))
			{

				if (Pal->Colors && Pal->StartIndex+Pal->ColorsCount <= Global->Opt->Palette.size())
				{
					Global->Opt->Palette.Set(Pal->StartIndex, Pal->Colors, Pal->ColorsCount);
					if (Pal->Flags&FSETCLR_REDRAW)
					{
						Global->ScrBuf->Lock(); // отменяем всякую прорисовку
						Global->WindowManager->ResizeAllWindows();
						Global->WindowManager->PluginCommit(); // коммитим.
						Global->ScrBuf->Unlock(); // разрешаем прорисовку
					}

					return TRUE;
				}
			}
			return FALSE;
		}

		/* $ 14.12.2000 SVS
			ACTL_EJECTMEDIA - извлечь диск из съемного накопителя
			Param - указатель на структуру ActlEjectMedia
			Return - TRUE - успешное извлечение, FALSE - ошибка.
		*/
		case ACTL_EJECTMEDIA:
		{
			return CheckStructSize((ActlEjectMedia*)Param2)?EjectVolume((wchar_t)((ActlEjectMedia*)Param2)->Letter,
										((ActlEjectMedia*)Param2)->Flags):FALSE;
			/*
					if(Param)
					{
							ActlEjectMedia *aem=(ActlEjectMedia *)Param;
					char DiskLetter[4]=" :\\";
					DiskLetter[0]=(char)aem->Letter;
					int DriveType = FAR_GetDriveType(DiskLetter,nullptr,FALSE); // здесь не определяем тип CD

					if(DriveType == DRIVE_USBDRIVE && RemoveUSBDrive((char)aem->Letter,aem->Flags))
						return TRUE;
					if(DriveType == DRIVE_SUBSTITUTE && DelSubstDrive(DiskLetter))
						return TRUE;
					if(IsDriveTypeCDROM(DriveType) && EjectVolume((char)aem->Letter,aem->Flags))
						return TRUE;

					}
					return FALSE;
			*/
		}
		/*
			case ACTL_GETMEDIATYPE:
			{
					ActlMediaType *amt=(ActlMediaType *)Param;
				char DiskLetter[4]=" :\\";
				DiskLetter[0]=(amt)?(char)amt->Letter:0;
				return FAR_GetDriveType(DiskLetter,nullptr,(amt && !(amt->Flags&MEDIATYPE_NODETECTCDROM)));
			}
		*/

		/* $ 05.06.2001 tran
			новые ACTL_ для работы с окнами */
		case ACTL_GETWINDOWINFO:
		{
			WindowInfo *wi=(WindowInfo*)Param2;
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
					modal=(Global->WindowManager->IndexOfStack(f)>=0);
				}
				else
				{
					if (wi->Pos >= 0 && wi->Pos < static_cast<intptr_t>(Global->WindowManager->GetWindowCount()))
					{
						f = Global->WindowManager->GetWindow(wi->Pos);
					}
					else if(wi->Pos >= static_cast<intptr_t>(Global->WindowManager->GetWindowCount()) && wi->Pos < static_cast<intptr_t>(Global->WindowManager->GetWindowCount() + Global->WindowManager->GetModalWindowCount()))
					{
						f = Global->WindowManager->GetModalWindow(wi->Pos - Global->WindowManager->GetWindowCount());
						modal=true;
					}
				}

				if (!f)
					return FALSE;

				f->GetTypeAndName(strType, strName);

				if (wi->TypeNameSize && wi->TypeName)
				{
					xwcsncpy(wi->TypeName,strType.data(),wi->TypeNameSize);
				}
				else
				{
					wi->TypeNameSize=strType.size()+1;
				}

				if (wi->NameSize && wi->Name)
				{
					xwcsncpy(wi->Name,strName.data(),wi->NameSize);
				}
				else
				{
					wi->NameSize=strName.size()+1;
				}

				if(-1==wi->Pos) wi->Pos = Global->WindowManager->IndexOf(f);
				if(-1==wi->Pos) wi->Pos = Global->WindowManager->IndexOfStack(f) + Global->WindowManager->GetWindowCount();
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
						wi->Id=(intptr_t)f.get(); // BUGBUG
						break;
					case WTYPE_COMBOBOX:
						wi->Id=(intptr_t)std::static_pointer_cast<VMenu>(f)->GetDialog().get(); // BUGBUG
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
			return Global->WindowManager->GetWindowCount() + Global->WindowManager->GetModalWindowCount();

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
			return (intptr_t)Console().GetWindow();

		case ACTL_REDRAWALL:
		{
			Global->WindowManager->RefreshAll();
			Global->WindowManager->PluginCommit();
			return TRUE;
		}

		case ACTL_SETPROGRESSSTATE:
			Taskbar().SetProgressState(static_cast<TBPFLAG>(Param1));
			return TRUE;

		case ACTL_SETPROGRESSVALUE:
		{
			BOOL Result=FALSE;
			const auto PV = static_cast<const ProgressValue*>(Param2);
			if(CheckStructSize(PV))
			{
				Taskbar().SetProgressValue(PV->Completed,PV->Total);
				Result=TRUE;
			}
			return Result;
		}

		case ACTL_QUIT:
			Global->CloseFARMenu=TRUE;
			Global->WindowManager->ExitMainLoop(FALSE);
			return TRUE;

		case ACTL_GETFARRECT:
			{
				BOOL Result=FALSE;
				if(Param2)
				{
					auto& Rect = *static_cast<PSMALL_RECT>(Param2);
					if(Global->Opt->WindowMode)
					{
						Result=Console().GetWorkingRect(Rect);
					}
					else
					{
						COORD Size;
						if(Console().GetSize(Size))
						{
							Rect.Left=0;
							Rect.Top=0;
							Rect.Right=Size.X-1;
							Rect.Bottom=Size.Y-1;
							Result=TRUE;
						}
					}
				}
				return Result;
			}
			break;

		case ACTL_GETCURSORPOS:
			{
				BOOL Result=FALSE;
				if(Param2)
				{
					auto& Pos = *static_cast<PCOORD>(Param2);
					Result=Console().GetCursorPosition(Pos);
				}
				return Result;
			}
			break;

		case ACTL_SETCURSORPOS:
			{
				BOOL Result=FALSE;
				if(Param2)
				{
					auto& Pos = *static_cast<PCOORD>(Param2);
					Result=Console().SetCursorPosition(Pos);
				}
				return Result;
			}
			break;

		case ACTL_PROGRESSNOTIFY:
		{
			Taskbar().Flash();
			return TRUE;
		}

		default:
			break;
		}

		return FALSE;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

intptr_t WINAPI apiMenuFn(
    const GUID* PluginId,
    const GUID* Id,
    intptr_t X,
    intptr_t Y,
    intptr_t MaxHeight,
    unsigned __int64 Flags,
    const wchar_t *Title,
    const wchar_t *Bottom,
    const wchar_t *HelpTopic,
    const FarKey *BreakKeys,
    intptr_t *BreakCode,
    const FarMenuItem *Item,
    size_t ItemsNumber
) noexcept
{
	try
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

			const auto FarMenu = VMenu2::create(NullToEmpty(Title), nullptr, 0, MaxHeight, MenuFlags);
			FarMenu->SetPosition(X,Y,0,0);
			if(Id)
			{
				FarMenu->SetId(*Id);
			}

			if (BreakCode)
				*BreakCode=-1;

			{
				string strTopic;

				if (Help::MkTopic(GuidToPlugin(PluginId), NullToEmpty(HelpTopic), strTopic))
					FarMenu->SetHelp(strTopic);
			}

			if (Bottom)
				FarMenu->SetBottomTitle(Bottom);

			size_t Selected=0;

			for (size_t i=0; i < ItemsNumber; i++)
			{
				MenuItemEx CurItem;
				CurItem.Flags=Item[i].Flags;
				CurItem.strName.clear();
				// исключаем MultiSelected, т.к. у нас сейчас движок к этому не приспособлен, оставляем только первый
				DWORD SelCurItem=CurItem.Flags&LIF_SELECTED;
				CurItem.Flags&=~LIF_SELECTED;

				if (!Selected && !(CurItem.Flags&LIF_SEPARATOR) && SelCurItem)
				{
					CurItem.Flags|=SelCurItem;
					Selected++;
				}

				CurItem.strName=NullToEmpty(Item[i].Text);
				if(CurItem.Flags&LIF_SEPARATOR)
				{
					CurItem.AccelKey=0;
				}
				else
				{
					INPUT_RECORD input = {};
					FarKeyToInputRecord(Item[i].AccelKey,&input);
					CurItem.AccelKey=InputRecordToKey(&input);
				}
				FarMenu->AddItem(CurItem);
			}

			if (!Selected)
				FarMenu->SetSelectPos(0,1);

			// флаги меню, с забитым контентом
			if (Flags & FMENU_AUTOHIGHLIGHT)
				FarMenu->AssignHighlights(FALSE);

			if (Flags & FMENU_REVERSEAUTOHIGHLIGHT)
				FarMenu->AssignHighlights(TRUE);

			FarMenu->SetTitle(NullToEmpty(Title));

			ExitCode=FarMenu->RunEx([&](int Msg, void *param)
			{
				if (Msg!=DN_INPUT || !BreakKeys)
					return 0;

				INPUT_RECORD *ReadRec=static_cast<INPUT_RECORD*>(param);
				int ReadKey=InputRecordToKey(ReadRec);

				if (ReadKey==KEY_NONE)
					return 0;

				for (int I=0; BreakKeys[I].VirtualKeyCode; I++)
				{
					if (Global->CtrlObject->Macro.IsExecuting())
					{
						int VirtKey,ControlState;
						TranslateKeyToVK(ReadKey,VirtKey,ControlState,ReadRec);
					}

					if (ReadRec->Event.KeyEvent.wVirtualKeyCode==BreakKeys[I].VirtualKeyCode)
					{

						const auto NormalizeControlKeys = [](DWORD Value)
						{
							DWORD result = Value&(LEFT_CTRL_PRESSED | LEFT_ALT_PRESSED | SHIFT_PRESSED);
							if (Value&RIGHT_CTRL_PRESSED) result |= LEFT_CTRL_PRESSED;
							if (Value&RIGHT_ALT_PRESSED) result |= LEFT_ALT_PRESSED;
							return result;
						};

						if (NormalizeControlKeys(ReadRec->Event.KeyEvent.dwControlKeyState) == NormalizeControlKeys(BreakKeys[I].ControlKeyState))
						{
							if (BreakCode)
								*BreakCode=I;

							FarMenu->Close(-2, true);
							return 1;
						}
					}
				}
				return 0;
			});
		}
	//  CheckScreenLock();
		return ExitCode;
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

// Функция FarDefDlgProc обработки диалога по умолчанию
intptr_t WINAPI apiDefDlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2) noexcept
{
	try
	{
		return static_cast<Dialog*>(hDlg)->DefProc(Msg, Param1, Param2);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

// Посылка сообщения диалогу
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

	try
	{
		const auto dialog = static_cast<Dialog*>(hDlg);
		return Dialog::IsValid(dialog)? dialog->SendMessage(Msg, Param1, Param2) : ErrorResult();
	}
	catch (...)
	{
		// TODO: log
		return ErrorResult();
	}
}

HANDLE WINAPI apiDialogInit(const GUID* PluginId, const GUID* Id, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2,
                            const wchar_t *HelpTopic, const FarDialogItem *Item,
                            size_t ItemsNumber, intptr_t Reserved, unsigned __int64 Flags,
                            FARWINDOWPROC DlgProc, void* Param) noexcept
{
	try
	{
		HANDLE hDlg=INVALID_HANDLE_VALUE;

		if (Global->WindowManager->ManagerIsDown())
			return hDlg;

		if (Global->DisablePluginsOutput || !ItemsNumber || !Item)
			return hDlg;

		// ФИЧА! нельзя указывать отрицательные X2 и Y2
		if (X2 < 0 || Y2 < 0)
			return hDlg;


		if (const auto Plugin = Global->CtrlObject->Plugins->FindPlugin(*PluginId))
		{
			class plugin_dialog: public Dialog
			{
				struct private_tag {};

			public:
				static dialog_ptr create(const range<const FarDialogItem*>& Src, FARWINDOWPROC DlgProc, void* InitParam)
				{
					return std::make_shared<plugin_dialog>(private_tag(), Src, DlgProc, InitParam);
				}

				intptr_t Proc(Dialog* hDlg, intptr_t Msg, intptr_t Param1, void* Param2)
				{
					return m_Proc(hDlg, Msg, Param1, Param2);
				}

				plugin_dialog(private_tag, const range<const FarDialogItem*>& Src, FARWINDOWPROC DlgProc, void* InitParam):
					Dialog(Dialog::private_tag(), Src, DlgProc? [this](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2) { return Proc(Dlg, Msg, Param1, Param2); } : dialog_handler(), InitParam),
					m_Proc(DlgProc)
				{}

			private:
				FARWINDOWPROC m_Proc;
			};

			const auto FarDialog = plugin_dialog::create(make_range(Item, ItemsNumber), DlgProc, Param);

			if (FarDialog->InitOK())
			{
				Plugin->AddDialog(FarDialog);
				hDlg = FarDialog.get();

				FarDialog->SetPosition(X1,Y1,X2,Y2);

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

				if (Flags & FDLG_NONMODAL)
					FarDialog->SetCanLoseFocus(TRUE);

				FarDialog->SetHelp(NullToEmpty(HelpTopic));

				FarDialog->SetId(*Id);
				/* $ 29.08.2000 SVS
				   Запомним номер плагина - сейчас в основном для формирования HelpTopic
				*/
				FarDialog->SetPluginOwner(GuidToPlugin(PluginId));
			}
		}
		return hDlg;
	}
	catch (...)
	{
		// TODO: log
		return INVALID_HANDLE_VALUE;
	}
}

intptr_t WINAPI apiDialogRun(HANDLE hDlg) noexcept
{
	try
	{
		if (Global->WindowManager->ManagerIsDown())
			return -1;

		const auto FarDialog = static_cast<Dialog*>(hDlg);

		FarDialog->Process();
		int ExitCode = FarDialog->GetExitCode();

		if (Global->IsMainThread()) // BUGBUG, findfile
			Global->WindowManager->RefreshWindow(); //?? - //AY - это нужно чтоб обновлять панели после выхода из диалога

		return ExitCode;
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

void WINAPI apiDialogFree(HANDLE hDlg) noexcept
{
	try
	{
		if (hDlg != INVALID_HANDLE_VALUE)
		{
			const auto Dlg = static_cast<Dialog*>(hDlg)->shared_from_this();
			const auto& Plugins = Global->CtrlObject->Plugins;
			std::any_of(RANGE(*Plugins, i) { return i->RemoveDialog(Dlg); });
		}
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

const wchar_t* WINAPI apiGetMsgFn(const GUID* PluginId,intptr_t MsgId) noexcept
{
	try
	{
		if (Plugin *pPlugin = GuidToPlugin(PluginId))
		{
			string strPath = pPlugin->GetModuleName();
			CutToSlash(strPath);

			if (pPlugin->InitLang(strPath))
				return pPlugin->GetMsg(static_cast<LNGID>(MsgId));
		}
		return L"";
	}
	catch (...)
	{
		// TODO: log
		return L"";
	}
}

intptr_t WINAPI apiMessageFn(const GUID* PluginId,const GUID* Id,unsigned __int64 Flags,const wchar_t *HelpTopic,
                        const wchar_t * const *Items,size_t ItemsNumber,
                        intptr_t ButtonsNumber) noexcept
{
	try
	{
		if (Flags&FMSG_ERRORTYPE)
			Global->CatchError();

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
			Buttons = { MSG(MOk) };
			break;

		case FMSG_MB_OKCANCEL:
			Buttons = { MSG(MOk), MSG(MCancel) };
			break;

		case FMSG_MB_ABORTRETRYIGNORE:
			Buttons = { MSG(MAbort), MSG(MRetry), MSG(MIgnore) };
			break;

		case FMSG_MB_YESNO:
			Buttons = { MSG(MYes), MSG(MNo) };
			break;

		case FMSG_MB_YESNOCANCEL:
			Buttons = { MSG(MYes), MSG(MNo), MSG(MCancel) };
			break;

		case FMSG_MB_RETRYCANCEL:
			Buttons = { MSG(MRetry), MSG(MCancel) };
			break;
		}

		const auto AssignStrings = [&](const auto& Source)
		{
			if (!Source.empty())
			{
				Title = *Source.begin();
				if (Buttons.empty())
				{
					MsgItems.assign(std::next(Source.begin()), Source.end() - ButtonsNumber);
					Buttons.assign(Source.end() - ButtonsNumber, Source.end());
				}
				else
				{
					// FMSG_MB_* is active
					MsgItems.assign(std::next(Source.begin()), Source.end());
				}
			}

		};

		if (Flags & FMSG_ALLINONE)
		{
			AssignStrings(split<std::vector<string>>(reinterpret_cast<const wchar_t*>(Items), STLF_NOTRIM | STLF_ALLOWEMPTY | STLF_NOUNQUOTE | STLF_NOQUOTING, L"\n"));
		}
		else
		{
			AssignStrings(make_range(Items, ItemsNumber));
		}

		Plugin* PluginNumber = GuidToPlugin(PluginId);
		// запоминаем топик
		string strTopic;
		if (PluginNumber)
		{
			Help::MkTopic(PluginNumber,NullToEmpty(HelpTopic),strTopic);
		}

		int MsgCode=Message(Flags&(FMSG_WARNING|FMSG_ERRORTYPE|FMSG_KEEPBACKGROUND|FMSG_LEFTALIGN),
			Title,
			MsgItems,
			Buttons,
			EmptyToNull(strTopic.data()), PluginNumber, Id);

		return MsgCode;
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

intptr_t WINAPI apiPanelControl(HANDLE hPlugin,FILE_CONTROL_COMMANDS Command,intptr_t Param1,void* Param2) noexcept
{
	try
	{
		_FCTLLOG(CleverSysLog CSL(L"Control"));
		_FCTLLOG(SysLog(L"(hPlugin=0x%08X, Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));
		_ALGO(CleverSysLog clv(L"FarPanelControl"));
		_ALGO(SysLog(L"(hPlugin=0x%08X, Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));

		if (Command == FCTL_CHECKPANELSEXIST)
			return !Global->OnlyEditorViewerUsed;

		if (Global->OnlyEditorViewerUsed || !Global->CtrlObject || Global->WindowManager->ManagerIsDown())
			return 0;

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
		case FCTL_SETNUMERICSORT:
		case FCTL_SETCASESENSITIVESORT:
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
					hInternal=PlHandle->hPlugin;

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
					hInternal=PlHandle->hPlugin;

					if (hPlugin==hInternal)
					{
						Processed=RightPanel->SetPluginCommand(Command,Param1,Param2);
					}
				}
			}

			return Processed;
		}

		case FCTL_GETUSERSCREEN:
		{
			Global->CtrlObject->CmdLine()->EnterPluginExecutionContext();
			return TRUE;
		}

		case FCTL_SETUSERSCREEN:
		{
			Global->CtrlObject->CmdLine()->LeavePluginExecutionContext();
			return TRUE;
		}

		case FCTL_GETCMDLINE:
		{
			const auto& Str = CmdLine->GetString();
			if (Param1&&Param2)
			{
				xwcsncpy((wchar_t*)Param2, Str.data(), Param1);
			}

			return Str.size() + 1;
		}

		case FCTL_SETCMDLINE:
		case FCTL_INSERTCMDLINE:
		{
			{
				SCOPED_ACTION(SetAutocomplete)(CmdLine);
				if (Command==FCTL_SETCMDLINE)
					CmdLine->SetString((const wchar_t*)Param2, true);
				else
					CmdLine->InsertString((const wchar_t*)Param2);
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
				*(int *)Param2=CmdLine->GetCurPos();
				return TRUE;
			}

			return FALSE;
		}

		case FCTL_GETCMDLINESELECTION:
		{
			CmdLineSelect *sel=(CmdLineSelect*)Param2;
			if (CheckStructSize(sel))
			{
				CmdLine->GetSelection(sel->SelStart,sel->SelEnd);
				return TRUE;
			}

			return FALSE;
		}

		case FCTL_SETCMDLINESELECTION:
		{
			CmdLineSelect *sel=(CmdLineSelect*)Param2;
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
					if (PlHandle->hPlugin == hPlugin)
						return TRUE;
				}
			}

			return FALSE;
		}

		default:
			return FALSE;
		}
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}


HANDLE WINAPI apiSaveScreen(intptr_t X1,intptr_t Y1,intptr_t X2,intptr_t Y2) noexcept
{
	try
	{
		if (Global->DisablePluginsOutput || Global->WindowManager->ManagerIsDown())
			return nullptr;

		if (X2 == -1)
			X2 = ScrX;

		if (Y2 == -1)
			Y2 = ScrY;

		return new SaveScreen(X1, Y1, X2, Y2);
	}
	catch (...)
	{
		// TODO: log
		return nullptr;
	}
}

void WINAPI apiRestoreScreen(HANDLE hScreen) noexcept
{
	try
	{
		if (Global->DisablePluginsOutput || Global->WindowManager->ManagerIsDown())
			return;

		if (!hScreen)
			Global->ScrBuf->FillBuf();

		if (hScreen)
			delete(SaveScreen *)hScreen;
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

static void ClearDirList(std::vector<PluginPanelItem>& Items)
{
	std::for_each(ALL_RANGE(Items), FreePluginPanelItem);
}

namespace magic
{
	static auto CastVectorToRawData(std::unique_ptr<std::vector<PluginPanelItem>>&& Items)
	{
		std::tuple<PluginPanelItem*, size_t> Result;
		std::get<1>(Result) = Items->size();
		PluginPanelItem Item;
		Item.Reserved[0] = reinterpret_cast<intptr_t>(Items.get());
		Items->emplace_back(Item);
		std::get<0>(Result) = Items->data();
		Items.release();
		return Result;
	}

	static auto CastRawDataToVector(PluginPanelItem* RawItems, size_t Size)
	{
		auto Items = reinterpret_cast<std::vector<PluginPanelItem>*>(RawItems[Size].Reserved[0]);
		Items->pop_back(); // not needed anymore
		return std::unique_ptr<std::vector<PluginPanelItem>>(Items);
	}
}

intptr_t WINAPI apiGetDirList(const wchar_t *Dir,PluginPanelItem **pPanelItem,size_t *pItemsNumber) noexcept
{
	try
	{
		if (Global->WindowManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
			return FALSE;

		string strDirName;
		ConvertNameToFull(Dir, strDirName);
		{
			const auto PR_FarGetDirListMsg = [](){ Message(0, 0, L"", MSG(MPreparingList)); };

			SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<PreRedrawItem>(PR_FarGetDirListMsg));
			SCOPED_ACTION(SaveScreen);
			os::FAR_FIND_DATA FindData;
			string strFullName;
			ScanTree ScTree(false);
			ScTree.SetFindPath(strDirName,L"*");
			*pItemsNumber=0;
			*pPanelItem=nullptr;

			auto Items = std::make_unique<std::vector<PluginPanelItem>>();

			time_check TimeCheck(time_check::delayed, GetRedrawTimeout());
			bool MsgOut = false;
			while (ScTree.GetNextName(FindData,strFullName))
			{
				if (TimeCheck)
				{
					if (CheckForEsc())
					{
						ClearDirList(*Items);
						return FALSE;
					}

					if (!MsgOut)
					{
						SetCursorType(false, 0);
						PR_FarGetDirListMsg();
						MsgOut = true;
					}
				}

				FindData.strFileName = strFullName;
				PluginPanelItem Item;
				FindDataExToPluginPanelItem(FindData, Item);
				Items->emplace_back(Item);
			}

			std::tie(*pPanelItem, *pItemsNumber) = magic::CastVectorToRawData(std::move(Items));
		}
		return TRUE;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

intptr_t WINAPI apiGetPluginDirList(const GUID* PluginId, HANDLE hPlugin, const wchar_t *Dir, PluginPanelItem **pPanelItem, size_t *pItemsNumber) noexcept
{
	try
	{
		if (Global->WindowManager->ManagerIsDown())
			return FALSE;

		auto Items = std::make_unique<std::vector<PluginPanelItem>>();
		auto Result = GetPluginDirList(GuidToPlugin(PluginId), hPlugin, Dir, *Items);
		std::tie(*pPanelItem, *pItemsNumber) = magic::CastVectorToRawData(std::move(Items));
		return Result;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

void WINAPI apiFreeDirList(PluginPanelItem *PanelItems, size_t ItemsNumber) noexcept
{
	try
	{
		auto Items = magic::CastRawDataToVector(PanelItems, ItemsNumber);
		ClearDirList(*Items);
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

void WINAPI apiFreePluginDirList(HANDLE hPlugin, PluginPanelItem *PanelItems, size_t ItemsNumber) noexcept
{
	try
	{
		auto Items = magic::CastRawDataToVector(PanelItems, ItemsNumber);
		FreePluginDirList(hPlugin, *Items);
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

intptr_t WINAPI apiViewer(const wchar_t *FileName,const wchar_t *Title,
                     intptr_t X1,intptr_t Y1,intptr_t X2, intptr_t Y2,unsigned __int64 Flags, uintptr_t CodePage) noexcept
{
	try
	{
		if (Global->WindowManager->ManagerIsDown())
			return FALSE;

		int DisableHistory = (Flags & VF_DISABLEHISTORY) != 0;

		// $ 15.05.2002 SKV - Запретим вызов немодального редактора viewer-а из модального.
		if (Global->WindowManager->InModal())
		{
			Flags&=~VF_NONMODAL;
		}

		if (Flags & VF_NONMODAL)
		{
			/* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
			const auto Viewer = FileViewer::create(FileName, TRUE, DisableHistory, Title, X1, Y1, X2, Y2, CodePage);

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
			const auto Viewer = FileViewer::create(FileName, FALSE, DisableHistory, Title, X1, Y1, X2, Y2, CodePage);

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
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

intptr_t WINAPI apiEditor(const wchar_t* FileName, const wchar_t* Title, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2, unsigned __int64 Flags, intptr_t StartLine, intptr_t StartChar, uintptr_t CodePage) noexcept
{
	try
	{
		if (Global->WindowManager->ManagerIsDown())
			return EEC_OPEN_ERROR;

		/* $ 12.07.2000 IS
		 Проверка флагов редактора (раньше они игнорировались) и открытие
		 немодального редактора, если есть соответствующий флаг
		 */
		int CreateNew = (Flags & EF_CREATENEW) != 0;
		int Locked=(Flags & EF_LOCKED) != 0;
		int DisableHistory=(Flags & EF_DISABLEHISTORY) != 0;
		int DisableSavePos=(Flags & EF_DISABLESAVEPOS) != 0;
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

		int editorExitCode;
		int ExitCode = EEC_OPEN_ERROR;
		string strTitle(NullToEmpty(Title));

		if (Flags & EF_NONMODAL)
		{
			/* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
			if (const auto Editor = FileEditor::create(NullToEmpty(FileName), CodePage,
				(CreateNew ? FFILEEDIT_CANNEWFILE : 0) | FFILEEDIT_ENABLEF6 |
				(DisableHistory ? FFILEEDIT_DISABLEHISTORY : 0) |
				(Locked ? FFILEEDIT_LOCKED : 0) |
				(DisableSavePos ? FFILEEDIT_DISABLESAVEPOS : 0),
				StartLine, StartChar, &strTitle,
				X1, Y1, X2, Y2,
				DeleteOnClose, nullptr, OpMode))
			{
				editorExitCode = Editor->GetExitCode();

				// добавочка - проверка кода возврата (почему возникает XC_OPEN_ERROR - см. код FileEditor::Init())
				if (editorExitCode == XC_OPEN_ERROR || editorExitCode == XC_LOADING_INTERRUPTED)
				{
					return editorExitCode == XC_OPEN_ERROR ? EEC_OPEN_ERROR : EEC_LOADING_INTERRUPTED;
				}

				if (editorExitCode == XC_EXISTS)
				{
					if (Global->GlobalSaveScrPtr)
						Global->GlobalSaveScrPtr->Discard();

					Global->WindowManager->PluginCommit();
#if defined(MANTIS_0002562)
					return EEC_OPENED_EXISTING;
#else
					return EEC_MODIFIED;
#endif
				}

				Editor->SetEnableF6((Flags & EF_ENABLE_F6) != 0);
				Editor->SetPluginTitle(&strTitle);

				/* $ 21.05.2002 SKV - Запускаем свой цикл, только если не был указан флаг. */
				if (!(Flags&EF_IMMEDIATERETURN))
				{
					Global->WindowManager->ExecuteNonModal(Editor);
					if (Global->WindowManager->IndexOf(Editor) != -1)
						ExitCode = Editor->IsFileChanged() ? EEC_MODIFIED : EEC_NOT_MODIFIED;
					else
						ExitCode = EEC_NOT_MODIFIED;//??? editorExitCode
				}
				else
				{
					if (Global->GlobalSaveScrPtr)
						Global->GlobalSaveScrPtr->Discard();

					Global->WindowManager->PluginCommit();
#if defined(MANTIS_0002562)
					ExitCode = editorExitCode == XC_RELOAD ? EEC_RELOAD : Editor->IsFileChanged() ? EEC_MODIFIED : EEC_NOT_MODIFIED;
#else
					ExitCode = EEC_MODIFIED;
#endif
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
				X1, Y1, X2, Y2,
				DeleteOnClose, nullptr, OpMode);
			editorExitCode = Editor->GetExitCode();

			// выполним предпроверку (ошибки разные могут быть)
			switch (editorExitCode)
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
					Global->WindowManager->ExecuteModal(Editor);
					editorExitCode = Editor->GetExitCode();

					if (editorExitCode)
					{
#if 0

						if (OpMode == EF_OPENMODE_BREAKIFOPEN && ExitCode == XC_QUIT)
							ExitCode = XC_OPEN_ERROR;
						else
#endif
							ExitCode = Editor->IsFileChanged() ? EEC_MODIFIED : EEC_NOT_MODIFIED;
					}
					else
					{
						ExitCode = EEC_OPEN_ERROR;
					}
				}
				break;
			}
		}

		return ExitCode;
	}
	catch (...)
	{
		// TODO: log
		return EEC_OPEN_ERROR;
	}
}

void WINAPI apiText(intptr_t X,intptr_t Y,const FarColor* Color,const wchar_t *Str) noexcept
{
	try
	{
		if (Global->DisablePluginsOutput || Global->WindowManager->ManagerIsDown())
			return;

		if (!Str)
		{
			int PrevLockCount = Global->ScrBuf->GetLockCount();
			Global->ScrBuf->SetLockCount(0);
			Global->ScrBuf->Flush();
			Global->ScrBuf->SetLockCount(PrevLockCount);
		}
		else
		{
			Text(X, Y, *Color, Str);
		}
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

template<class window_type, typename command_type, typename getter_type, typename control_type>
static intptr_t apiTControl(intptr_t Id, command_type Command, intptr_t Param1, void* Param2, getter_type Getter, control_type Control)
{
	if (Global->WindowManager->ManagerIsDown())
		return 0;

	if (Id == -1)
	{
		const auto CurrentObject = (*Global->WindowManager.*Getter)();
		return CurrentObject ? (CurrentObject->*Control)(Command, Param1, Param2) : 0;
	}
	else
	{
		static const std::pair<decltype(&Manager::GetWindow), decltype(&Manager::GetWindowCount)> Functions[] =
		{
			{ &Manager::GetWindow, &Manager::GetWindowCount },
			{ &Manager::GetModalWindow, &Manager::GetModalWindowCount },
		};

		for (const auto& i: Functions)
		{
			const size_t count = (*Global->WindowManager.*i.second)();
			for (size_t j = 0; j < count; ++j)
			{
				if (const auto CurrentWindow = std::dynamic_pointer_cast<window_type>((*Global->WindowManager.*i.first)(j)))
				{
					if (const auto CurrentControlWindow = CurrentWindow->GetById(Id))
					{
						return (CurrentControlWindow->*Control)(Command, Param1, Param2);
					}
				}
			}
		}
	}
	return 0;
}


intptr_t WINAPI apiEditorControl(intptr_t EditorID, EDITOR_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	try
	{
		return apiTControl<FileEditor>(EditorID, Command, Param1, Param2, &Manager::GetCurrentEditor, &FileEditor::EditorControl);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

intptr_t WINAPI apiViewerControl(intptr_t ViewerID, VIEWER_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	try
	{
		return apiTControl<ViewerContainer>(ViewerID, Command, Param1, Param2, &Manager::GetCurrentViewer, &Viewer::ViewerControl);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

void WINAPI apiUpperBuf(wchar_t *Buf, intptr_t Length) noexcept
{
	try
	{
		return UpperBuf(Buf, Length);
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

void WINAPI apiLowerBuf(wchar_t *Buf, intptr_t Length) noexcept
{
	try
	{
		return LowerBuf(Buf, Length);
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

void WINAPI apiStrUpper(wchar_t *s1) noexcept
{
	try
	{
		return StrUpper(s1);
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

void WINAPI apiStrLower(wchar_t *s1) noexcept
{
	try
	{
		return StrLower(s1);
	}
	catch (...)
	{
		// TODO: log
		return;
	}
}

wchar_t WINAPI apiUpper(wchar_t Ch) noexcept
{
	try
	{
		return Upper(Ch);
	}
	catch (...)
	{
		// TODO: log
		return Ch;
	}
}

wchar_t WINAPI apiLower(wchar_t Ch) noexcept
{
	try
	{
		return Lower(Ch);
	}
	catch (...)
	{
		// TODO: log
		return Ch;
	}
}

int WINAPI apiStrCmpNI(const wchar_t *s1, const wchar_t *s2, intptr_t n) noexcept
{
	try
	{
		return StrCmpNI(s1, s2, n);
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

int WINAPI apiStrCmpI(const wchar_t *s1, const wchar_t *s2) noexcept
{
	try
	{
		return StrCmpI(s1, s2);
	}
	catch (...)
	{
		// TODO: log
		return -1;
	}
}

int WINAPI apiIsLower(wchar_t Ch) noexcept
{
	try
	{
		return IsLower(Ch);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

int WINAPI apiIsUpper(wchar_t Ch) noexcept
{
	try
	{
		return IsUpper(Ch);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

int WINAPI apiIsAlpha(wchar_t Ch) noexcept
{
	try
	{
		return IsAlpha(Ch);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}

}

int WINAPI apiIsAlphaNum(wchar_t Ch) noexcept
{
	try
	{
		return IsAlphaNum(Ch);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

wchar_t* WINAPI apiTruncStr(wchar_t *Str,intptr_t MaxLength) noexcept
{
	try
	{
		return TruncStr(Str, MaxLength);
	}
	catch (...)
	{
		// TODO: log
		return Str;
	}
}

wchar_t* WINAPI apiTruncStrFromCenter(wchar_t *Str, intptr_t MaxLength) noexcept
{
	try
	{
		return TruncStrFromCenter(Str, MaxLength);
	}
	catch (...)
	{
		// TODO: log
		return Str;
	}
}

wchar_t* WINAPI apiTruncStrFromEnd(wchar_t *Str, intptr_t MaxLength) noexcept
{
	try
	{
		return TruncStrFromEnd(Str, MaxLength);
	}
	catch (...)
	{
		// TODO: log
		return Str;
	}
}

wchar_t* WINAPI apiTruncPathStr(wchar_t *Str, intptr_t MaxLength) noexcept
{
	try
	{
		return TruncPathStr(Str, MaxLength);
	}
	catch (...)
	{
		// TODO: log
		return Str;
	}
}

const wchar_t* WINAPI apiPointToName(const wchar_t* Path) noexcept
{
	try
	{
		return PointToName(Path);
	}
	catch (...)
	{
		// TODO: log
		return Path;
	}
}

size_t WINAPI apiGetFileOwner(const wchar_t *Computer, const wchar_t *Name, wchar_t *Owner, size_t Size) noexcept
{
	try
	{
		string strOwner;
		GetFileOwner(NullToEmpty(Computer), NullToEmpty(Name), strOwner);

		if (Owner && Size)
			xwcsncpy(Owner, strOwner.data(), Size);

		return strOwner.size() + 1;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

size_t WINAPI apiConvertPath(CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, size_t DestSize) noexcept
{
	try
	{
		string strDest;

		switch (Mode)
		{
		case CPM_NATIVE:
			strDest=NTPath(Src);
			break;

		case CPM_REAL:
			ConvertNameToReal(Src, strDest);
			break;

		case CPM_FULL:
		default:
			ConvertNameToFull(Src, strDest);
			break;
		}

		if (Dest && DestSize)
			xwcsncpy(Dest, strDest.data(), DestSize);

		return strDest.size() + 1;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

size_t WINAPI apiGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest, size_t DestSize) noexcept
{
	try
	{
		string strSrc(Src);
		string strDest;
		AddEndSlash(strDest);
		GetReparsePointInfo(strSrc,strDest,nullptr);

		if (DestSize && Dest)
			xwcsncpy(Dest,strDest.data(),DestSize);

		return strDest.size()+1;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

size_t WINAPI apiGetNumberOfLinks(const wchar_t* Name) noexcept
{
	try
	{
		return GetNumberOfLinks(Name);
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

size_t WINAPI apiGetPathRoot(const wchar_t *Path, wchar_t *Root, size_t DestSize) noexcept
{
	try
	{
		string strPath(Path), strRoot;
		GetPathRoot(strPath,strRoot);

		if (DestSize && Root)
			xwcsncpy(Root,strRoot.data(),DestSize);

		return strRoot.size()+1;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

BOOL WINAPI apiCopyToClipboard(enum FARCLIPBOARD_TYPE Type, const wchar_t *Data) noexcept
{
	try
	{
		switch (Type)
		{
		case FCT_STREAM:
			return SetClipboardText(Data, Data? wcslen(Data) : 0);

		case FCT_COLUMN:
			return SetClipboardVText(Data, Data? wcslen(Data) : 0);

		default:
			return FALSE;
		}
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

static size_t apiPasteFromClipboardEx(bool Type, wchar_t *Data, size_t Size)
{
	string str;
	if(Type? GetClipboardVText(str) : GetClipboardText(str))
	{
		if(Data && Size)
		{
			Size = std::min(Size, str.size() + 1);
			std::copy_n(str.data(), Size, Data);
		}
		return str.size() + 1;
	}
	return 0;
}

size_t WINAPI apiPasteFromClipboard(enum FARCLIPBOARD_TYPE Type, wchar_t *Data, size_t Length) noexcept
{
	try
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

		case FCT_ANY:
			size = apiPasteFromClipboardEx(false, Data, Length);
			break;

		case FCT_COLUMN:
			size = apiPasteFromClipboardEx(true, Data, Length);
			break;
		}
		return size;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

unsigned __int64 WINAPI apiFarClock() noexcept
{
	try
	{
		return Global->FarUpTime();
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

intptr_t WINAPI apiMacroControl(const GUID* PluginId, FAR_MACRO_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	try
	{
		if (Global->CtrlObject) // все зависит от этой бадяги.
		{
			KeyMacro& Macro = Global->CtrlObject->Macro; //??

			switch (Command)
			{
			// Param1=0, Param2 - FarMacroLoad*
			case MCTL_LOADALL: // из реестра в память ФАР с затиранием предыдущего
			{
				FarMacroLoad *Data = (FarMacroLoad*)Param2;
				return
					!Macro.IsRecording() &&
					(!Data || CheckStructSize(Data)) &&
					Macro.LoadMacros(false, !Macro.IsExecuting(), Data);
			}

			// Param1=0, Param2 - 0
			case MCTL_SAVEALL:
			{
				return !Macro.IsRecording() && Macro.SaveMacros(true);
			}

			// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroSendMacroText*
			case MCTL_SENDSTRING:
			{
				MacroSendMacroText *Data = (MacroSendMacroText*)Param2;
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
				break;
			}

			// Param1=0, Param2 - MacroExecuteString*
			case MCTL_EXECSTRING:
			{
				MacroExecuteString *Data = (MacroExecuteString*)Param2;
				return CheckStructSize(Data) && Macro.ExecuteString(Data) ? 1 : 0;
			}

			// Param1=0, Param2 - 0
			case MCTL_GETSTATE:
			{
				return Macro.GetState();
			}

			// Param1=0, Param2 - 0
			case MCTL_GETAREA:
			{
				return Macro.GetArea();
			}

			case MCTL_ADDMACRO:
			{
				MacroAddMacro *Data = (MacroAddMacro*)Param2;
				if (CheckStructSize(Data) && Data->SequenceText && *Data->SequenceText)
				{
					return Macro.AddMacro(*PluginId, Data) ? 1 : 0;
				}
				break;
			}

			case MCTL_DELMACRO:
			{
				return Macro.DelMacro(*PluginId, Param2) ? 1 : 0;
			}

			//Param1=size of buffer, Param2 - MacroParseResult*
			case MCTL_GETLASTERROR:
			{
				DWORD ErrCode = MPEC_SUCCESS;
				COORD ErrPos = {};
				string ErrSrc;

				Macro.GetMacroParseError(&ErrCode, &ErrPos, &ErrSrc);

				int Size = aligned_sizeof<MacroParseResult>::value;
				size_t stringOffset = Size;
				Size += static_cast<int>((ErrSrc.size() + 1)*sizeof(wchar_t));

				MacroParseResult *Result = (MacroParseResult *)Param2;

				if (Param1 >= Size && CheckStructSize(Result))
				{
					Result->StructSize = sizeof(MacroParseResult);
					Result->ErrCode = ErrCode;
					Result->ErrPos = ErrPos;
					Result->ErrSrc = (const wchar_t *)((char*)Param2 + stringOffset);
					std::copy_n(ErrSrc.data(), ErrSrc.size() + 1, const_cast<wchar_t*>(Result->ErrSrc));
				}

				return Size;
			}

			default: //FIXME
				break;
			}
		}
		return 0;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

intptr_t WINAPI apiPluginsControl(HANDLE Handle, FAR_PLUGINS_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	try
	{
		switch (Command)
		{
		case PCTL_LOADPLUGIN:
		case PCTL_FORCEDLOADPLUGIN:
			if (Param1 == PLT_PATH)
			{
				if (Param2)
				{
					string strPath;
					ConvertNameToFull(reinterpret_cast<const wchar_t*>(Param2), strPath);
					return reinterpret_cast<intptr_t>(Global->CtrlObject->Plugins->LoadPluginExternal(strPath, Command == PCTL_FORCEDLOADPLUGIN));
				}
			}
			break;

		case PCTL_FINDPLUGIN:
		{
			Plugin* plugin = nullptr;
			switch (Param1)
			{
			case PFM_GUID:
				plugin = Global->CtrlObject->Plugins->FindPlugin(*reinterpret_cast<GUID*>(Param2));
				break;

			case PFM_MODULENAME:
			{
				string strPath;
				ConvertNameToFull(reinterpret_cast<const wchar_t*>(Param2), strPath);
				const auto ItemIterator = std::find_if(CONST_RANGE(*Global->CtrlObject->Plugins, i)
				{
					return !StrCmpI(i->GetModuleName(), strPath);
				});
				if (ItemIterator != Global->CtrlObject->Plugins->cend())
				{
					plugin = *ItemIterator;
				}
				break;
			}
			}
			if (plugin&&Global->CtrlObject->Plugins->IsPluginUnloaded(plugin)) plugin = nullptr;
			return reinterpret_cast<intptr_t>(plugin);
		}

		case PCTL_UNLOADPLUGIN:
			return Global->CtrlObject->Plugins->UnloadPluginExternal(static_cast<Plugin*>(Handle));

		case PCTL_GETPLUGININFORMATION:
		{
			const auto Info = reinterpret_cast<FarGetPluginInformation*>(Param2);
			if (Handle && (!Info || (CheckStructSize(Info) && static_cast<size_t>(Param1) > sizeof(FarGetPluginInformation))))
			{
				return Global->CtrlObject->Plugins->GetPluginInformation(reinterpret_cast<Plugin*>(Handle), Info, Param1);
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
			break;
		}
		return 0;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

intptr_t WINAPI apiFileFilterControl(HANDLE hHandle, FAR_FILE_FILTER_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	try
	{
		FileFilter *Filter = nullptr;

		if (Command != FFCTL_CREATEFILEFILTER)
		{
			if (!hHandle || hHandle == INVALID_HANDLE_VALUE)
				return FALSE;

			Filter = (FileFilter *)hHandle;
		}

		switch (Command)
		{
		case FFCTL_CREATEFILEFILTER:
		{
			if (!Param2)
				break;

			*((HANDLE *)Param2) = INVALID_HANDLE_VALUE;

			if (hHandle != nullptr && hHandle != PANEL_ACTIVE && hHandle != PANEL_PASSIVE && hHandle != PANEL_NONE)
				break;

			switch (Param1)
			{
			case FFT_PANEL:
			case FFT_FINDFILE:
			case FFT_COPY:
			case FFT_SELECT:
			case FFT_CUSTOM:
				break;

			default:
				return FALSE;
			}

			Filter = new FileFilter(GetHostPanel(hHandle), (FAR_FILE_FILTER_TYPE)Param1);
			*((HANDLE *)Param2) = (HANDLE)Filter;
			return TRUE;
		}

		case FFCTL_FREEFILEFILTER:
			delete Filter;
			return TRUE;

		case FFCTL_OPENFILTERSMENU:
			return Filter->FilterEdit();

		case FFCTL_STARTINGTOFILTER:
			Filter->UpdateCurrentTime();
			return TRUE;

		case FFCTL_ISFILEINFILTER:
			if (!Param2)
				break;
			return Filter->FileInFilter(*reinterpret_cast<const PluginPanelItem*>(Param2));
		}
		return FALSE;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

intptr_t WINAPI apiRegExpControl(HANDLE hHandle, FAR_REGEXP_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	try
	{
		RegExp* re = nullptr;

		if (Command != RECTL_CREATE)
		{
			if (!hHandle || hHandle == INVALID_HANDLE_VALUE)
				return FALSE;

			re = (RegExp*)hHandle;
		}

		switch (Command)
		{
		case RECTL_CREATE:
			*((HANDLE*)Param2) = INVALID_HANDLE_VALUE;
			re = new RegExp;

			*((HANDLE*)Param2) = (HANDLE)re;
			return TRUE;

		case RECTL_FREE:
			delete re;
			return TRUE;

		case RECTL_COMPILE:
			return re->Compile((const wchar_t*)Param2, OP_PERLSTYLE);

		case RECTL_OPTIMIZE:
			return re->Optimize();

		case RECTL_MATCHEX:
		{
			RegExpSearch* data = (RegExpSearch*)Param2;
			return re->MatchEx(data->Text, data->Text + data->Position, data->Text + data->Length, data->Match, data->Count);
		}

		case RECTL_SEARCHEX:
		{
			RegExpSearch* data = (RegExpSearch*)Param2;
			return re->SearchEx(data->Text, data->Text + data->Position, data->Text + data->Length, data->Match, data->Count);
		}

		case RECTL_BRACKETSCOUNT:
			return re->GetBracketsCount();

		default:
			return FALSE;
		}
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}

}

intptr_t WINAPI apiSettingsControl(HANDLE hHandle, FAR_SETTINGS_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2) noexcept
{
	try
	{
		AbstractSettings* settings = nullptr;

		if (Command != SCTL_CREATE)
		{
			if (!hHandle || hHandle == INVALID_HANDLE_VALUE)
				return FALSE;

			settings = (AbstractSettings*)hHandle;
		}

		switch (Command)
		{
		case SCTL_CREATE:
		{
			FarSettingsCreate* data = (FarSettingsCreate*)Param2;
			if (CheckStructSize(data))
			{
				if (data->Guid == FarGuid)
				{
					settings = AbstractSettings::CreateFarSettings();
				}
				else
				{
					if (Global->CtrlObject->Plugins->FindPlugin(data->Guid))
					{
						settings = AbstractSettings::CreatePluginSettings(data->Guid, Param1 == PSL_LOCAL);
					}
				}
				if (settings && settings->IsValid())
				{
					data->Handle = settings;
					return TRUE;
				}
				delete settings;
			}
			break;
		}

		case SCTL_FREE:
			delete settings;
			return TRUE;

		case SCTL_SET:
			return CheckStructSize((const FarSettingsItem*)Param2) ? settings->Set(*(const FarSettingsItem*)Param2) : FALSE;

		case SCTL_GET:
			return CheckStructSize((const FarSettingsItem*)Param2) ? settings->Get(*(FarSettingsItem*)Param2) : FALSE;

		case SCTL_ENUM:
			return CheckStructSize((FarSettingsEnum*)Param2) ? settings->Enum(*(FarSettingsEnum*)Param2) : FALSE;

		case SCTL_DELETE:
			return CheckStructSize((const FarSettingsValue*)Param2) ? settings->Delete(*(const FarSettingsValue*)Param2) : FALSE;

		case SCTL_CREATESUBKEY:
		case SCTL_OPENSUBKEY:
			return CheckStructSize((const FarSettingsValue*)Param2) ? settings->SubKey(*(const FarSettingsValue*)Param2, Command == SCTL_CREATESUBKEY) : 0;
		}
		return FALSE;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

size_t WINAPI apiGetCurrentDirectory(size_t Size, wchar_t* Buffer) noexcept
{
	try
	{
		const auto strCurDir = os::GetCurrentDirectory();

		if (Buffer && Size)
		{
			xwcsncpy(Buffer, strCurDir.data(), Size);
		}

		return strCurDir.size() + 1;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

size_t WINAPI apiFormatFileSize(unsigned __int64 Size, intptr_t Width, FARFORMATFILESIZEFLAGS Flags, wchar_t *Dest, size_t DestSize) noexcept
{
	try
	{
		static const std::pair<unsigned __int64, unsigned __int64> FlagsPair[] =
		{
			{FFFS_COMMAS,         COLUMN_COMMAS},         // Вставлять разделитель между тысячами
			{FFFS_THOUSAND,       COLUMN_THOUSAND},       // Вместо делителя 1024 использовать делитель 1000
			{FFFS_FLOATSIZE,      COLUMN_FLOATSIZE},      // Показывать размер файла в стиле Windows Explorer (т.е. 999 байт будут показаны как 999, а 1000 байт как 0.97 K)
			{FFFS_ECONOMIC,       COLUMN_ECONOMIC},       // Экономичный режим, не показывать пробел перед суффиксом размера файла (т.е. 0.97K)
			{FFFS_MINSIZEINDEX,   COLUMN_MINSIZEINDEX},   // Минимально допустимый индекс при форматировании
			{FFFS_SHOWBYTESINDEX, COLUMN_SHOWBYTESINDEX}, // Показывать суффиксы B,K,M,G,T,P,E
		};

		unsigned __int64 FinalFlags=Flags & COLUMN_MINSIZEINDEX_MASK;
		std::for_each(CONST_RANGE(FlagsPair, i)
		{
			if (Flags & i.first)
				FinalFlags |= i.second;
		});

		auto strDestStr = FileSizeToStr(Size, Width, FinalFlags);

		if (Dest && DestSize)
		{
			xwcsncpy(Dest,strDestStr.data(),DestSize);
		}

		return strDestStr.size()+1;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

void WINAPI apiRecursiveSearch(const wchar_t *InitDir, const wchar_t *Mask, FRSUSERFUNC Func, unsigned __int64 Flags, void *Param) noexcept
{
	try
	{
		filemasks FMask;

		if (!FMask.Set(Mask, FMF_SILENT)) return;

		Flags=Flags&0x000000FF; // только младший байт!
		ScanTree ScTree((Flags & FRS_RETUPDIR)!=0, (Flags & FRS_RECUR)!=0, (Flags & FRS_SCANSYMLINK)!=0);
		os::FAR_FIND_DATA FindData;
		string strFullName;
		ScTree.SetFindPath(InitDir,L"*");

		bool Found = false;
		while (!Found && ScTree.GetNextName(FindData,strFullName))
		{
			if (FMask.Compare(FindData.strFileName))
			{
				PluginPanelItem fdata;
				FindDataExToPluginPanelItem(FindData, fdata);

				Found = !Func(&fdata,strFullName.data(),Param);
				FreePluginPanelItem(fdata);
			}
		}
	}
	catch (...)
	{
		// TODO: log
		return;
	}

}

size_t WINAPI apiMkTemp(wchar_t *Dest, size_t DestSize, const wchar_t *Prefix) noexcept
{
	try
	{
		string strDest;
		if (FarMkTempEx(strDest, Prefix, TRUE) && Dest && DestSize)
		{
			xwcsncpy(Dest, strDest.data(), DestSize);
		}
		return strDest.size() + 1;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

size_t WINAPI apiProcessName(const wchar_t *param1, wchar_t *param2, size_t size, PROCESSNAME_FLAGS flags) noexcept
{
	try
	{
		//             0xFFFF - length
		//           0xFF0000 - mode
		// 0xFFFFFFFFFF000000 - flags

		PROCESSNAME_FLAGS Flags = flags&0xFFFFFFFFFF000000;
		PROCESSNAME_FLAGS Mode = flags&0xFF0000;
		int Length = flags&0xFFFF;

		switch(Mode)
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
				ValidMask = Masks.Set(param1, FMF_SILENT);
				PrevMask = param1;
			}
			BOOL Result = FALSE;
			if(ValidMask)
			{
				Result = (Mode == PN_CHECKMASK)? TRUE : Masks.Compare((Flags&PN_SKIPPATH)? PointToName(param2) : param2);
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
			string strResult = NullToEmpty(param2);
			int nResult = ConvertWildcards(NullToEmpty(param1), strResult, Length);
			xwcsncpy(param2, strResult.data(), size);
			return nResult;
		}

		default:
			return FALSE;
		}
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

BOOL WINAPI apiColorDialog(const GUID* PluginId, COLORDIALOGFLAGS Flags, FarColor *Color) noexcept
{
	try
	{
		BOOL Result = FALSE;
		if (!Global->WindowManager->ManagerIsDown())
		{
			Result = Console().GetColorDialog(*Color, true, false);
		}
		return Result;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

size_t WINAPI apiInputRecordToKeyName(const INPUT_RECORD* Key, wchar_t *KeyText, size_t Size) noexcept
{
	try
	{
		int iKey = InputRecordToKey(Key);
		string strKT;
		if (iKey == KEY_NONE || !KeyToText(iKey, strKT))
			return 0;
		size_t len = strKT.size();
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
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

BOOL WINAPI apiKeyNameToInputRecord(const wchar_t *Name, INPUT_RECORD* RecKey) noexcept
{
	try
	{
		int Key = KeyNameToKey(Name);
		return Key > 0 ? KeyToInputRecord(Key, RecKey) : FALSE;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

BOOL WINAPI apiMkLink(const wchar_t *Target, const wchar_t *LinkName, LINK_TYPE Type, MKLINK_FLAGS Flags) noexcept
{
	try
	{
		int Result = 0;

		if (Target && *Target && LinkName && *LinkName)
		{
			switch (Type)
			{
			case LINK_HARDLINK:
				Result = MkHardLink(Target, LinkName, (Flags&MLF_SHOWERRMSG) == 0);
				break;

			case LINK_JUNCTION:
			case LINK_VOLMOUNT:
			case LINK_SYMLINKFILE:
			case LINK_SYMLINKDIR:
			case LINK_SYMLINK:
			{
				ReparsePointTypes LinkType = RP_JUNCTION;

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

				Result = MkSymLink(Target, LinkName, LinkType, (Flags&MLF_SHOWERRMSG) == 0, (Flags&MLF_HOLDTARGET) != 0);
				break;
			}

			default:
				break;
			}
		}

		if (Result && !(Flags&MLF_DONOTUPDATEPANEL))
			ShellUpdatePanels(nullptr, FALSE);

		return Result;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

BOOL WINAPI apiAddEndSlash(wchar_t *Path) noexcept
{
	try
	{
		return AddEndSlash(Path) ? TRUE : FALSE;
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

wchar_t* WINAPI apiXlat(wchar_t *Line, intptr_t StartPos, intptr_t EndPos, XLAT_FLAGS Flags) noexcept
{
	try
	{
		return Xlat(Line, StartPos, EndPos, Flags);
	}
	catch (...)
	{
		// TODO: log
		return Line;
	}
}

HANDLE WINAPI apiCreateFile(const wchar_t *Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile) noexcept
{
	try
	{
		return os::CreateFile(Object, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile).release();
	}
	catch (...)
	{
		// TODO: log
		return INVALID_HANDLE_VALUE;
	}
}

DWORD WINAPI apiGetFileAttributes(const wchar_t *FileName) noexcept
{
	try
	{
		return os::GetFileAttributes(FileName);
	}
	catch (...)
	{
		// TODO: log
		return INVALID_FILE_ATTRIBUTES;
	}
}

BOOL WINAPI apiSetFileAttributes(const wchar_t *FileName, DWORD dwFileAttributes) noexcept
{
	try
	{
		return os::SetFileAttributes(FileName, dwFileAttributes);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

BOOL WINAPI apiMoveFileEx(const wchar_t *ExistingFileName, const wchar_t *NewFileName, DWORD dwFlags) noexcept
{
	try
	{
		return os::MoveFileEx(ExistingFileName, NewFileName, dwFlags);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

BOOL WINAPI apiDeleteFile(const wchar_t *FileName) noexcept
{
	try
	{
		return os::DeleteFile(FileName);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

BOOL WINAPI apiRemoveDirectory(const wchar_t *DirName) noexcept
{
	try
	{
		return os::RemoveDirectory(DirName);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

BOOL WINAPI apiCreateDirectory(const wchar_t *PathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) noexcept
{
	try
	{
		return os::CreateDirectory(PathName, lpSecurityAttributes);
	}
	catch (...)
	{
		// TODO: log
		return FALSE;
	}
}

intptr_t WINAPI apiCallFar(intptr_t CheckCode, FarMacroCall* Data) noexcept
{
	try
	{
		return Global->CtrlObject ? Global->CtrlObject->Macro.CallFar(CheckCode, Data) : 0;
	}
	catch (...)
	{
		// TODO: log
		return 0;
	}
}

}
