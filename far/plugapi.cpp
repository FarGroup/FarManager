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
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "scantree.hpp"
#include "rdrwdsk.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "plugins.hpp"
#include "savescr.hpp"
#include "flink.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "frame.hpp"
#include "scrbuf.hpp"
#include "farexcpt.hpp"
#include "lockscrn.hpp"
#include "constitle.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "palette.hpp"
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
#include "synchro.hpp"
#include "copy.hpp"
#include "panelmix.hpp"
#include "xlat.hpp"
#include "dirinfo.hpp"

namespace pluginapi
{
inline Plugin* GuidToPlugin(const GUID* Id) {return (Id && CtrlObject)? CtrlObject->Plugins->FindPlugin(*Id) : nullptr;}

int WINAPIV apiSprintf(wchar_t* Dest, const wchar_t* Format, ...) //?deprecated
{
	va_list argptr;
	va_start(argptr, Format);
	int Result = _vsnwprintf(Dest, 32000, Format, argptr); //vswprintf(Det,L"%s",(const char *)str) -- MinGW gcc >= 4.6
	va_end(argptr);
	return Result;
}

int WINAPIV apiSnprintf(wchar_t* Dest, size_t Count, const wchar_t* Format, ...)
{
	va_list argptr;
	va_start(argptr, Format);
	int Result =  _vsnwprintf(Dest, Count, Format, argptr);
	va_end(argptr);
	return Result;
}

wchar_t *WINAPI apiItoa(int value, wchar_t *string, int radix)
{
	if (string)
		return _itow(value,string,radix);

	return nullptr;
}

wchar_t *WINAPI apiItoa64(__int64 value, wchar_t *string, int radix)
{
	if (string)
		return _i64tow(value, string, radix);

	return nullptr;
}

int WINAPI apiAtoi(const wchar_t *s)
{
	if (s)
		return _wtoi(s);

	return 0;
}
__int64 WINAPI apiAtoi64(const wchar_t *s)
{
	return s?_wtoi64(s):0;
}

void WINAPI apiQsort(void *base, size_t nelem, size_t width, int (WINAPI *fcmp)(const void *, const void *,void *),void *user)
{
	if (base && fcmp)
	{
		qsortex((char*)base,nelem,width,fcmp,user);
	}
}

void *WINAPI apiBsearch(const void *key, const void *base, size_t nelem, size_t width, int (WINAPI *fcmp)(const void *, const void *, void *),void *user)
{
	if (key && fcmp && base)
		return bsearchex(key,base,nelem,width,fcmp,user);

	return nullptr;
}

wchar_t* WINAPI apiQuoteSpace(wchar_t *Str)
{
	return QuoteSpace(Str);
}

wchar_t* WINAPI apiInsertQuote(wchar_t *Str)
{
	return InsertQuote(Str);
}

void WINAPI apiUnquote(wchar_t *Str)
{
	return Unquote(Str);
}

wchar_t* WINAPI apiRemoveLeadingSpaces(wchar_t *Str)
{
	return RemoveLeadingSpaces(Str);
}

wchar_t * WINAPI apiRemoveTrailingSpaces(wchar_t *Str)
{
	return RemoveTrailingSpaces(Str);
}

wchar_t* WINAPI apiRemoveExternalSpaces(wchar_t *Str)
{
	return RemoveExternalSpaces(Str);
}

wchar_t* WINAPI apiQuoteSpaceOnly(wchar_t *Str)
{
	return QuoteSpaceOnly(Str);
}

void WINAPI apiDeleteBuffer(void *Buffer)
{
	if (Buffer) xf_free(Buffer);
}

int WINAPI apiInputBox(
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
)
{
	if (FrameManager->ManagerIsDown())
		return FALSE;

	string strDest;
	int nResult = GetString(Title,Prompt,HistoryName,SrcText,strDest,HelpTopic,Flags&~FIB_CHECKBOX,nullptr,nullptr,GuidToPlugin(PluginId),Id);
	xwcsncpy(DestText, strDest, DestSize);
	return nResult;
}

/* Функция вывода помощи */
BOOL WINAPI apiShowHelp(
    const wchar_t *ModuleName,
    const wchar_t *HelpTopic,
    FARHELPFLAGS Flags
)
{
	if (FrameManager->ManagerIsDown())
		return FALSE;

	if (!HelpTopic)
		HelpTopic=L"Contents";

	UINT64 OFlags=Flags;
	Flags&=~(FHELP_NOSHOWERROR|FHELP_USECONTENTS);
	string strPath, strTopic;
	string strMask;

	// двоеточие в начале топика надо бы игнорировать и в том случае,
	// если стоит FHELP_FARHELP...
	if ((Flags&FHELP_FARHELP) || *HelpTopic==L':')
		strTopic = HelpTopic+((*HelpTopic == L':')?1:0);
	else
	{
		if (ModuleName)
		{
			// FHELP_SELFHELP=0 - трактовать первый пар-р как Info.ModuleName
			//                   и показать топик из хелпа вызвавшего плагина
			/* $ 17.11.2000 SVS
			   А значение FHELP_SELFHELP равно чему? Правильно - 0
			   И фигля здесь удивлятся тому, что функция не работает :-(
			*/
			if (Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE|FHELP_CUSTOMPATH)))
			{
				strPath = ModuleName;

				if (Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE)))
				{
					if (Flags&FHELP_CUSTOMFILE)
						strMask=PointToName(strPath);
					else
						strMask.Clear();

					CutToSlash(strPath);
				}
			}
			else
				return FALSE;

			strTopic.Format(HelpFormatLink,strPath.CPtr(),HelpTopic);
		}
		else
			return FALSE;
	}

	{
		Help Hlp(strTopic,strMask,OFlags);

		if (Hlp.GetError())
			return FALSE;
	}

	return TRUE;
}

/* $ 05.07.2000 IS
  Функция, которая будет действовать и в редакторе, и в панелях, и...
*/
intptr_t WINAPI apiAdvControl(const GUID* PluginId, ADVANCED_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	if (ACTL_SYNCHRO==Command) //must be first
	{
		PluginSynchroManager.Synchro(true, *PluginId, Param2);
		return 0;
	}
	if (ACTL_GETWINDOWTYPE==Command)
	{
		WindowType* info=(WindowType*)Param2;
		if (CheckStructSize(info))
		{
			WINDOWINFO_TYPE type=ModalType2WType(CurrentWindowType);
			switch(type)
			{
			case WTYPE_PANELS:
			case WTYPE_VIEWER:
			case WTYPE_EDITOR:
			case WTYPE_DIALOG:
			case WTYPE_VMENU:
			case WTYPE_HELP:
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

			if (FrameManager->ManagerIsDown())
				return 0;
	}

	switch (Command)
	{
		case ACTL_GETFARMANAGERVERSION:
		{
			if (Param2)
				*(VersionInfo*)Param2=FAR_VERSION;

			return TRUE;
		}
		/* $ 24.08.2000 SVS
		   ожидать определенную (или любую) клавишу
		   (const INPUT_RECORD*)Param2 - код клавиши, которую ожидаем, или NULL
		   если все равно какую клавишу ждать.
		   возвращает 0;
		*/
		case ACTL_WAITKEY:
		{
			return WaitKey(Param2?InputRecordToKey((const INPUT_RECORD*)Param2):-1,0,false);
		}
		/* $ 04.12.2000 SVS
		  ACTL_GETCOLOR - получить определенный цвет по индексу, определенному
		   в farcolor.hpp
		  Param2 - [OUT] значение цвета
		  Return - TRUE если OK или FALSE если индекс неверен.
		*/
		case ACTL_GETCOLOR:
		{
			if (static_cast<UINT>(Param1) < Opt.Palette.SizeArrayPalette)
			{
				*static_cast<FarColor*>(Param2) = Opt.Palette.CurrentPalette[static_cast<size_t>(Param1)];
				return TRUE;
			}
			return FALSE;
		}
		/* $ 04.12.2000 SVS
		  ACTL_GETARRAYCOLOR - получить весь массив цветов
		  Param1 - размер буфера (в элементах FarColor)
		  Param2 - указатель на буфер или nullptr, чтобы получить необходимый размер
		  Return - размер массива.
		*/
		case ACTL_GETARRAYCOLOR:
		{
			if (Param2 && static_cast<size_t>(Param1) >= Opt.Palette.SizeArrayPalette)
			{
				memcpy(Param2, Opt.Palette.CurrentPalette, Opt.Palette.SizeArrayPalette*sizeof(FarColor));
			}
			return Opt.Palette.SizeArrayPalette;
		}
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
			if (Param2)
			{
				FarSetColors *Pal=(FarSetColors*)Param2;

				if (Pal->Colors && Pal->StartIndex+Pal->ColorsCount <= Opt.Palette.SizeArrayPalette)
				{
					memmove(Opt.Palette.CurrentPalette+Pal->StartIndex,Pal->Colors,Pal->ColorsCount*sizeof(FarColor));
					Opt.Palette.SetChanged();
					if (Pal->Flags&FSETCLR_REDRAW)
					{
						ScrBuf.Lock(); // отменяем всякую прорисовку
						FrameManager->ResizeAllFrame();
						FrameManager->PluginCommit(); // коммитим.
						ScrBuf.Unlock(); // разрешаем прорисовку
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
			return Param2?EjectVolume((wchar_t)((ActlEjectMedia*)Param2)->Letter,
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
		      return FAR_GetDriveType(DiskLetter,nullptr,(amt && !(amt->Flags&MEDIATYPE_NODETECTCDROM)?TRUE:FALSE));
		    }
		*/
		/* $ 05.06.2001 tran
		   новые ACTL_ для работы с фреймами */
		case ACTL_GETWINDOWINFO:
		{
			WindowInfo *wi=(WindowInfo*)Param2;
			if (CheckStructSize(wi))
			{
				string strType, strName;
				Frame *f=nullptr;
				bool modal=false;

				/* $ 22.12.2001 VVM
				  + Если Pos == -1 то берем текущий фрейм */
				if (wi->Pos == -1)
				{
					f=FrameManager->GetCurrentFrame();
					modal=(FrameManager->IndexOfStack(f)>=0);
				}
				else
				{
					if (wi->Pos>=0&&wi->Pos<FrameManager->GetFrameCount())
					{
						f=(*FrameManager)[wi->Pos];
					}
					else if(wi->Pos>=FrameManager->GetFrameCount()&&wi->Pos<(FrameManager->GetFrameCount()+FrameManager->GetModalStackCount()))
					{
						f=FrameManager->GetModalFrame(wi->Pos-FrameManager->GetFrameCount());
						modal=true;
					}
				}

				if (!f)
					return FALSE;

				f->GetTypeAndName(strType, strName);

				if (wi->TypeNameSize && wi->TypeName)
				{
					xwcsncpy(wi->TypeName,strType,wi->TypeNameSize);
				}
				else
				{
					wi->TypeNameSize=static_cast<int>(strType.GetLength()+1);
				}

				if (wi->NameSize && wi->Name)
				{
					xwcsncpy(wi->Name,strName,wi->NameSize);
				}
				else
				{
					wi->NameSize=static_cast<int>(strName.GetLength()+1);
				}

				if(-1==wi->Pos) wi->Pos=FrameManager->IndexOf(f);
				if(-1==wi->Pos) wi->Pos=FrameManager->IndexOfStack(f)+FrameManager->GetFrameCount();
				wi->Type=ModalType2WType(f->GetType());
				wi->Flags=0;
				if (f->IsFileModified())
					wi->Flags|=WIF_MODIFIED;
				if (f==FrameManager->GetCurrentFrame())
					wi->Flags|=WIF_CURRENT;
				if (modal)
					wi->Flags|=WIF_MODAL;

				switch (wi->Type)
				{
					case WTYPE_VIEWER:
						wi->Id=static_cast<FileViewer*>(f)->GetId();
						break;
					case WTYPE_EDITOR:
						wi->Id=static_cast<FileEditor*>(f)->GetId();
						break;
					case WTYPE_DIALOG:
						wi->Id=(intptr_t)f;
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
		{
			return FrameManager->GetFrameCount()+FrameManager->GetModalStackCount();
		}
		case ACTL_SETCURRENTWINDOW:
		{
			// Запретим переключение фрэймов, если находимся в модальном редакторе/вьюере.
			if (!FrameManager->InModalEV() && (*FrameManager)[Param1])
			{
				int TypeFrame=FrameManager->GetCurrentFrame()->GetType();

				// Запретим переключение фрэймов, если находимся в хелпе или диалоге (тоже модальных)
				if (TypeFrame != MODALTYPE_HELP && TypeFrame != MODALTYPE_DIALOG)
				{
					Frame* PrevFrame = FrameManager->GetCurrentFrame();
					FrameManager->ActivateFrame(Param1);
					FrameManager->DeactivateFrame(PrevFrame, 0);
					return TRUE;
				}
			}

			return FALSE;
		}
		/*$ 26.06.2001 SKV
		  Для полноценной работы с ACTL_SETCURRENTWINDOW
		  (и может еще для чего в будущем)
		*/
		case ACTL_COMMIT:
		{
			return FrameManager->PluginCommit();
		}
		/* $ 15.09.2001 tran
		   пригодится плагинам */
		case ACTL_GETFARHWND:
		{
			return (intptr_t)Console.GetWindow();
		}
		case ACTL_REDRAWALL:
		{
			int Ret=FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
			FrameManager->PluginCommit();
			return Ret;
		}

		case ACTL_SETPROGRESSSTATE:
		{
			TBC.SetProgressState(static_cast<TBPFLAG>(Param1));
			return TRUE;
		}

		case ACTL_SETPROGRESSVALUE:
		{
			BOOL Result=FALSE;
			if(Param2)
			{
				ProgressValue* PV=static_cast<ProgressValue*>(Param2);
				TBC.SetProgressValue(PV->Completed,PV->Total);
				Result=TRUE;
			}
			return Result;
		}

		case ACTL_QUIT:
		{
			CloseFARMenu=TRUE;
			FrameManager->ExitMainLoop(FALSE);
			return TRUE;
		}

		case ACTL_GETFARRECT:
			{
				BOOL Result=FALSE;
				if(Param2)
				{
					SMALL_RECT& Rect=*static_cast<PSMALL_RECT>(Param2);
					if(Opt.WindowMode)
					{
						Result=Console.GetWorkingRect(Rect);
					}
					else
					{
						COORD Size;
						if(Console.GetSize(Size))
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
					COORD& Pos=*static_cast<PCOORD>(Param2);
					Result=Console.GetCursorPosition(Pos);
				}
				return Result;
			}
			break;

		case ACTL_SETCURSORPOS:
			{
				BOOL Result=FALSE;
				if(Param2)
				{
					COORD& Pos=*static_cast<PCOORD>(Param2);
					Result=Console.SetCursorPosition(Pos);
				}
				return Result;
			}
			break;

		case ACTL_PROGRESSNOTIFY:
		{
			TBC.Flash();
			return TRUE;
		}
		default:
			break;

	}

	return FALSE;
}

static DWORD NormalizeControlKeys(DWORD Value)
{
	DWORD result=Value&(LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED|SHIFT_PRESSED);
	if(Value&RIGHT_CTRL_PRESSED) result|=LEFT_CTRL_PRESSED;
	if(Value&RIGHT_ALT_PRESSED) result|=LEFT_ALT_PRESSED;
	return result;
}

#ifdef FAR_LUA
class MenuLock
{
	private:
		Frame* frame;
	public:
		MenuLock(bool enable) {
			frame = enable ? FrameManager->GetBottomFrame():nullptr;
			if (frame) frame->Lock();
		}
		~MenuLock() { if (frame) frame->Unlock(); }
};
#endif

int WINAPI apiMenuFn(
    const GUID* PluginId,
    const GUID* Id,
    int X,
    int Y,
    int MaxHeight,
    unsigned __int64 Flags,
    const wchar_t *Title,
    const wchar_t *Bottom,
    const wchar_t *HelpTopic,
    const FarKey *BreakKeys,
    int *BreakCode,
    const FarMenuItem *Item,
    size_t ItemsNumber
)
{
	if (FrameManager->ManagerIsDown())
		return -1;

	if (DisablePluginsOutput)
		return -1;

	int ExitCode;
	{
		VMenu FarMenu(Title,nullptr,0,MaxHeight);
		CtrlObject->Macro.SetMode(MACRO_MENU);
		FarMenu.SetPosition(X,Y,0,0);
		if(Id)
		{
			FarMenu.SetId(*Id);
		}

		if (BreakCode)
			*BreakCode=-1;

		{
			string strTopic;

			if (Help::MkTopic(GuidToPlugin(PluginId),HelpTopic,strTopic))
				FarMenu.SetHelp(strTopic);
		}

		if (Bottom)
			FarMenu.SetBottomTitle(Bottom);

		// общие флаги меню
		DWORD MenuFlags=0;

		if (Flags & FMENU_SHOWAMPERSAND)
			MenuFlags|=VMENU_SHOWAMPERSAND;

		if (Flags & FMENU_WRAPMODE)
			MenuFlags|=VMENU_WRAPMODE;

		if (Flags & FMENU_CHANGECONSOLETITLE)
			MenuFlags|=VMENU_CHANGECONSOLETITLE;

		FarMenu.SetFlags(MenuFlags);
		MenuItemEx CurItem;
		CurItem.Clear();
		size_t Selected=0;

		for (size_t i=0; i < ItemsNumber; i++)
		{
			CurItem.Flags=Item[i].Flags;
			CurItem.strName.Clear();
			// исключаем MultiSelected, т.к. у нас сейчас движок к этому не приспособлен, оставляем только первый
			DWORD SelCurItem=CurItem.Flags&LIF_SELECTED;
			CurItem.Flags&=~LIF_SELECTED;

			if (!Selected && !(CurItem.Flags&LIF_SEPARATOR) && SelCurItem)
			{
				CurItem.Flags|=SelCurItem;
				Selected++;
			}

			CurItem.strName=Item[i].Text;
			if(CurItem.Flags&LIF_SEPARATOR)
			{
				CurItem.AccelKey=0;
			}
			else
			{
				INPUT_RECORD input={0};
				FarKeyToInputRecord(Item[i].AccelKey,&input);
				CurItem.AccelKey=InputRecordToKey(&input);
			}
			FarMenu.AddItem(&CurItem);
		}

		if (!Selected)
			FarMenu.SetSelectPos(0,1);

		// флаги меню, с забитым контентом
		if (Flags & FMENU_AUTOHIGHLIGHT)
			FarMenu.AssignHighlights(FALSE);

		if (Flags & FMENU_REVERSEAUTOHIGHLIGHT)
			FarMenu.AssignHighlights(TRUE);

#ifdef FAR_LUA
		MenuLock menuLock(CtrlObject->Macro.IsExecuting()); //FIXME: dirty hack.
#endif
		FarMenu.SetTitle(Title);
		FarMenu.Show();

		while (!FarMenu.Done() && !CloseFARMenu)
		{
			INPUT_RECORD ReadRec;
			int ReadKey=GetInputRecord(&ReadRec);

			if (ReadKey==KEY_CONSOLE_BUFFER_RESIZE)
			{
				LockScreen LckScr;
				FarMenu.Hide();
				FarMenu.Show();
			}
			else if (ReadRec.EventType==MOUSE_EVENT && !(ReadKey==KEY_MSWHEEL_UP || ReadKey==KEY_MSWHEEL_DOWN || ReadKey==KEY_MSWHEEL_RIGHT || ReadKey==KEY_MSWHEEL_LEFT))
			{
				FarMenu.ProcessMouse(&ReadRec.Event.MouseEvent);
			}
			else if (ReadKey!=KEY_NONE)
			{
				if (BreakKeys)
				{
					for (int I=0; BreakKeys[I].VirtualKeyCode; I++)
					{
						if (CtrlObject->Macro.IsExecuting())
						{
							int VirtKey,ControlState;
							TranslateKeyToVK(ReadKey,VirtKey,ControlState,&ReadRec);
						}

						if (ReadRec.Event.KeyEvent.wVirtualKeyCode==BreakKeys[I].VirtualKeyCode)
						{
							if (NormalizeControlKeys(ReadRec.Event.KeyEvent.dwControlKeyState) == NormalizeControlKeys(BreakKeys[I].ControlKeyState))
							{
								if (BreakCode)
									*BreakCode=I;

								FarMenu.Hide();
//								CheckScreenLock();
								return FarMenu.GetSelectPos();
							}
						}
					}
				}

				FarMenu.ProcessKey(ReadKey);
			}
		}

		ExitCode=FarMenu.Modal::GetExitCode();
	}
//  CheckScreenLock();
	return(ExitCode);
}

// Функция FarDefDlgProc обработки диалога по умолчанию
intptr_t WINAPI apiDefDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	if (hDlg) // исключаем лишний вызов для hDlg=0
		return DefDlgProc(hDlg,Msg,Param1,Param2);

	return 0;
}

// Посылка сообщения диалогу
intptr_t WINAPI apiSendDlgMessage(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	if (hDlg) // исключаем лишний вызов для hDlg=0
		return SendDlgMessage(hDlg,Msg,Param1,Param2);

	return 0;
}

HANDLE WINAPI apiDialogInit(const GUID* PluginId, const GUID* Id, int X1, int Y1, int X2, int Y2,
                            const wchar_t *HelpTopic, const FarDialogItem *Item,
                            size_t ItemsNumber, intptr_t Reserved, unsigned __int64 Flags,
                            FARWINDOWPROC DlgProc, void* Param)
{
	HANDLE hDlg=INVALID_HANDLE_VALUE;

	if (FrameManager->ManagerIsDown())
		return hDlg;

	if (DisablePluginsOutput || ItemsNumber <= 0 || !Item)
		return hDlg;

	// ФИЧА! нельзя указывать отрицательные X2 и Y2
	if (X2 < 0 || Y2 < 0)
		return hDlg;

	{
		Dialog *FarDialog = new Dialog(Item,ItemsNumber,DlgProc,Param);

		if (!FarDialog)
			return hDlg;

		if (!FarDialog->InitOK())
		{
			delete FarDialog;
			return hDlg;
		}

		hDlg = (HANDLE)FarDialog;
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

		FarDialog->SetHelp(HelpTopic);

		FarDialog->SetId(*Id);
		/* $ 29.08.2000 SVS
		   Запомним номер плагина - сейчас в основном для формирования HelpTopic
		*/
		FarDialog->SetPluginOwner(GuidToPlugin(PluginId));
	}
	return hDlg;
}

int WINAPI apiDialogRun(HANDLE hDlg)
{
	if (FrameManager->ManagerIsDown())
		return -1;

	if (hDlg==INVALID_HANDLE_VALUE)
		return -1;

	int ExitCode=-1;

	Dialog *FarDialog = (Dialog *)hDlg;

	FarDialog->Process();
	ExitCode=FarDialog->GetExitCode();

	FrameManager->RefreshFrame(); //?? - //AY - это нужно чтоб обновлять панели после выхода из диалога
	return ExitCode;
}

void WINAPI apiDialogFree(HANDLE hDlg)
{
	if (hDlg != INVALID_HANDLE_VALUE)
	{
		Dialog *FarDialog = (Dialog *)hDlg;
		delete FarDialog;
	}
}

const wchar_t* WINAPI apiGetMsgFn(const GUID* PluginId,int MsgId)
{
	Plugin *pPlugin = GuidToPlugin(PluginId);
	if (pPlugin)
	{
		string strPath = pPlugin->GetModuleName();
		CutToSlash(strPath);

		if (pPlugin->InitLang(strPath))
			return pPlugin->GetMsg(static_cast<LNGID>(MsgId));
	}
	return L"";
}

int WINAPI apiMessageFn(const GUID* PluginId,const GUID* Id,unsigned __int64 Flags,const wchar_t *HelpTopic,
                        const wchar_t * const *Items,size_t ItemsNumber,
                        int ButtonsNumber)
{
	if (FrameManager->ManagerIsDown())
		return -1;

	if (DisablePluginsOutput)
		return -1;

	if ((!(Flags&(FMSG_ALLINONE|FMSG_ERRORTYPE)) && ItemsNumber<2) || !Items)
		return -1;

	wchar_t *SingleItems=nullptr;

	// анализ количества строк для FMSG_ALLINONE
	if (Flags&FMSG_ALLINONE)
	{
		ItemsNumber=0;

		if (!(SingleItems=(wchar_t *)xf_malloc((StrLength((const wchar_t *)Items)+2)*sizeof(wchar_t))))
			return -1;

		wchar_t *Msg=wcscpy(SingleItems,(const wchar_t *)Items);

		while ((Msg = wcschr(Msg, L'\n')) )
		{
			if (*++Msg == L'\0')
				break;

			++ItemsNumber;
		}

		ItemsNumber++; //??
	}

	const wchar_t **MsgItems=(const wchar_t **)xf_malloc(sizeof(wchar_t*)*(ItemsNumber+ADDSPACEFORPSTRFORMESSAGE));

	if (!MsgItems)
	{
		xf_free(SingleItems);
		return -1;
	}

	memset(MsgItems,0,sizeof(wchar_t*)*(ItemsNumber+ADDSPACEFORPSTRFORMESSAGE));

	if (Flags&FMSG_ALLINONE)
	{
		int I=0;
		wchar_t *Msg=SingleItems;
		// анализ количества строк и разбивка на пункты
		wchar_t *MsgTemp;

		while ((MsgTemp = wcschr(Msg, L'\n')) )
		{
			*MsgTemp=L'\0';
			MsgItems[I]=Msg;
			Msg=MsgTemp+1;

			if (*Msg == L'\0')
				break;

			++I;
		}

		if (*Msg)
		{
			MsgItems[I]=Msg;
		}
	}
	else
	{
		for (size_t i=0; i < ItemsNumber; i++)
			MsgItems[i]=Items[i];
	}

	/* $ 22.03.2001 tran
	   ItemsNumber++ -> ++ItemsNumber
	   тереялся последний элемент */
	switch (Flags&0x000F0000)
	{
		case FMSG_MB_OK:
			ButtonsNumber=1;
			MsgItems[ItemsNumber++]=MSG(MOk);
			break;
		case FMSG_MB_OKCANCEL:
			ButtonsNumber=2;
			MsgItems[ItemsNumber++]=MSG(MOk);
			MsgItems[ItemsNumber++]=MSG(MCancel);
			break;
		case FMSG_MB_ABORTRETRYIGNORE:
			ButtonsNumber=3;
			MsgItems[ItemsNumber++]=MSG(MAbort);
			MsgItems[ItemsNumber++]=MSG(MRetry);
			MsgItems[ItemsNumber++]=MSG(MIgnore);
			break;
		case FMSG_MB_YESNO:
			ButtonsNumber=2;
			MsgItems[ItemsNumber++]=MSG(MYes);
			MsgItems[ItemsNumber++]=MSG(MNo);
			break;
		case FMSG_MB_YESNOCANCEL:
			ButtonsNumber=3;
			MsgItems[ItemsNumber++]=MSG(MYes);
			MsgItems[ItemsNumber++]=MSG(MNo);
			MsgItems[ItemsNumber++]=MSG(MCancel);
			break;
		case FMSG_MB_RETRYCANCEL:
			ButtonsNumber=2;
			MsgItems[ItemsNumber++]=MSG(MRetry);
			MsgItems[ItemsNumber++]=MSG(MCancel);
			break;
	}

	// ограничение на строки
	size_t MaxLinesNumber = static_cast<size_t>(ScrY-3-(ButtonsNumber?1:0));
	size_t LinesNumber = ItemsNumber-ButtonsNumber-1;
	if (LinesNumber > MaxLinesNumber)
	{
		ItemsNumber -= (LinesNumber-MaxLinesNumber);
		for (int i=1; i <= ButtonsNumber; i++)
			MsgItems[MaxLinesNumber+i]=MsgItems[LinesNumber+i];
	}

	Plugin* PluginNumber = GuidToPlugin(PluginId);
	// запоминаем топик
	if (PluginNumber)
	{
		string strTopic;

		if (Help::MkTopic(reinterpret_cast<Plugin*>(PluginNumber),HelpTopic,strTopic))
			SetMessageHelp(strTopic);
	}

	// непосредственно... вывод
	Frame *frame;

	if ((frame=FrameManager->GetBottomFrame()))
		frame->Lock(); // отменим прорисовку фрейма

	int MsgCode=Message(Flags&(FMSG_WARNING|FMSG_ERRORTYPE|FMSG_KEEPBACKGROUND|FMSG_LEFTALIGN),ButtonsNumber,MsgItems[0],MsgItems+1,ItemsNumber-1,PluginNumber,Id);

	/* $ 15.05.2002 SKV
	  Однако разлочивать надо ровно то, что залочили.
	*/
	if (frame)
		frame->Unlock(); // теперь можно :-)

	//CheckScreenLock();

	if (SingleItems)
		xf_free(SingleItems);

	xf_free(MsgItems);

	return MsgCode;
}

intptr_t WINAPI apiPanelControl(HANDLE hPlugin,FILE_CONTROL_COMMANDS Command,int Param1,void* Param2)
{
	_FCTLLOG(CleverSysLog CSL(L"Control"));
	_FCTLLOG(SysLog(L"(hPlugin=0x%08X, Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));
	_ALGO(CleverSysLog clv(L"FarPanelControl"));
	_ALGO(SysLog(L"(hPlugin=0x%08X, Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));

	if (Command == FCTL_CHECKPANELSEXIST)
		return Opt.OnlyEditorViewerUsed? FALSE:TRUE;

	if (Opt.OnlyEditorViewerUsed || !CtrlObject || FrameManager->ManagerIsDown())
		return 0;

	FilePanels *FPanels=CtrlObject->Cp();
	CommandLine *CmdLine=CtrlObject->CmdLine;

	switch (Command)
	{
		case FCTL_CLOSEPANEL:
			g_strDirToSet = (wchar_t *)Param2;
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
		{
			if (!FPanels)
				return FALSE;

			if (!hPlugin || hPlugin == PANEL_ACTIVE || hPlugin == PANEL_PASSIVE)
			{
				Panel *pPanel = (!hPlugin || hPlugin == PANEL_ACTIVE)?FPanels->ActivePanel:FPanels->GetAnotherPanel(FPanels->ActivePanel);

				if (pPanel)
				{
					return pPanel->SetPluginCommand(Command,Param1,Param2);
				}

				return FALSE; //???
			}

			HANDLE hInternal;
			Panel *LeftPanel=FPanels->LeftPanel;
			Panel *RightPanel=FPanels->RightPanel;
			int Processed=FALSE;
			PluginHandle *PlHandle;

			if (LeftPanel && LeftPanel->GetMode()==PLUGIN_PANEL)
			{
				PlHandle=(PluginHandle *)LeftPanel->GetPluginHandle();

				if (PlHandle)
				{
					hInternal=PlHandle->hPlugin;

					if (hPlugin==hInternal)
					{
						Processed=LeftPanel->SetPluginCommand(Command,Param1,Param2);
					}
				}
			}

			if (RightPanel && RightPanel->GetMode()==PLUGIN_PANEL)
			{
				PlHandle=(PluginHandle *)RightPanel->GetPluginHandle();

				if (PlHandle)
				{
					hInternal=PlHandle->hPlugin;

					if (hPlugin==hInternal)
					{
						Processed=RightPanel->SetPluginCommand(Command,Param1,Param2);
					}
				}
			}

			return(Processed);
		}
		case FCTL_SETUSERSCREEN:
		{
			if (!FPanels || !FPanels->LeftPanel || !FPanels->RightPanel)
				return FALSE;

			KeepUserScreen++;
			FPanels->LeftPanel->ProcessingPluginCommand++;
			FPanels->RightPanel->ProcessingPluginCommand++;
			ScrBuf.FillBuf();
			ScrollScreen(1);
			SaveScreen SaveScr;
			{
				RedrawDesktop Redraw;
				CmdLine->Hide();
				SaveScr.RestoreArea(FALSE);
			}
			KeepUserScreen--;
			FPanels->LeftPanel->ProcessingPluginCommand--;
			FPanels->RightPanel->ProcessingPluginCommand--;
			return TRUE;
		}
		case FCTL_GETUSERSCREEN:
		{
			FrameManager->ShowBackground();
			int Lock=ScrBuf.GetLockCount();
			ScrBuf.SetLockCount(0);
			MoveCursor(0,ScrY-1);
			SetInitialCursorType();
			ScrBuf.Flush();
			ScrBuf.SetLockCount(Lock);
			return TRUE;
		}
		case FCTL_GETCMDLINE:
		{
			string strParam;

			CmdLine->GetString(strParam);

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strParam,Param1);

			return (int)strParam.GetLength()+1;
		}
		case FCTL_SETCMDLINE:
		case FCTL_INSERTCMDLINE:
		{
			{
				SetAutocomplete disable(CmdLine);
				if (Command==FCTL_SETCMDLINE)
					CmdLine->SetString((const wchar_t*)Param2);
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
			if (Param2)
			{
				CmdLineSelect *sel=(CmdLineSelect*)Param2;
				CmdLine->GetSelection(sel->SelStart,sel->SelEnd);
				return TRUE;
			}

			return FALSE;
		}
		case FCTL_SETCMDLINESELECTION:
		{
			if (Param2)
			{
				CmdLineSelect *sel=(CmdLineSelect*)Param2;
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

			Panel *pPanel = FPanels->ActivePanel;
			PluginHandle *PlHandle;

			if (pPanel && (pPanel->GetMode() == PLUGIN_PANEL))
			{
				PlHandle = (PluginHandle *)pPanel->GetPluginHandle();

				if (PlHandle)
				{
					if (PlHandle->hPlugin == hPlugin)
						return TRUE;
				}
			}

			return FALSE;
		}
		default:
			break;
	}

	return FALSE;
}


HANDLE WINAPI apiSaveScreen(int X1,int Y1,int X2,int Y2)
{
	if (DisablePluginsOutput || FrameManager->ManagerIsDown())
		return nullptr;

	if (X2==-1)
		X2=ScrX;

	if (Y2==-1)
		Y2=ScrY;

	return new SaveScreen(X1,Y1,X2,Y2);
}


void WINAPI apiRestoreScreen(HANDLE hScreen)
{
	if (DisablePluginsOutput || FrameManager->ManagerIsDown())
		return;

	if (!hScreen)
		ScrBuf.FillBuf();

	if (hScreen)
		delete(SaveScreen *)hScreen;
}


static void PR_FarGetDirListMsg()
{
	Message(0,0,L"",MSG(MPreparingList));
}

void FreeDirList(PluginPanelItem *PanelItem, size_t nItemsNumber)
{
	for (size_t I=0; I<nItemsNumber; I++)
	{
		PluginPanelItem *CurPanelItem=PanelItem+I;
		FreePluginPanelItem(CurPanelItem);
	}

	xf_free(PanelItem);
}

int WINAPI apiGetDirList(const wchar_t *Dir,PluginPanelItem **pPanelItem,size_t *pItemsNumber)
{
	if (FrameManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
		return FALSE;

	string strDirName;
	ConvertNameToFull(Dir, strDirName);
	{
		TPreRedrawFuncGuard preRedrawFuncGuard(PR_FarGetDirListMsg);
		SaveScreen SaveScr;
		clock_t StartTime=clock();
		int MsgOut=0;
		FAR_FIND_DATA_EX FindData;
		string strFullName;
		ScanTree ScTree(FALSE);
		ScTree.SetFindPath(strDirName,L"*");
		*pItemsNumber=0;
		*pPanelItem=nullptr;
		PluginPanelItem *ItemsList=nullptr;
		int ItemsNumber=0;

		while (ScTree.GetNextName(&FindData,strFullName))
		{
			if (!(ItemsNumber & 31))
			{
				if (CheckForEsc())
				{
					if (ItemsList)
						FreeDirList(ItemsList,ItemsNumber);

					return FALSE;
				}

				if (!MsgOut && clock()-StartTime > 500)
				{
					SetCursorType(FALSE,0);
					PR_FarGetDirListMsg();
					MsgOut=1;
				}

				ItemsList=(PluginPanelItem*)xf_realloc(ItemsList,sizeof(*ItemsList)*(ItemsNumber+32+1));

				if (!ItemsList)
				{
					return FALSE;
				}
			}

			ItemsList[ItemsNumber].FileAttributes = FindData.dwFileAttributes;
			ItemsList[ItemsNumber].FileSize = FindData.nFileSize;
			ItemsList[ItemsNumber].AllocationSize = FindData.nAllocationSize;
			ItemsList[ItemsNumber].CreationTime = FindData.ftCreationTime;
			ItemsList[ItemsNumber].LastAccessTime = FindData.ftLastAccessTime;
			ItemsList[ItemsNumber].LastWriteTime = FindData.ftLastWriteTime;
			ItemsList[ItemsNumber].FileName = xf_wcsdup(strFullName.CPtr());
			ItemsList[ItemsNumber].AlternateFileName = xf_wcsdup(FindData.strAlternateFileName);
			ItemsNumber++;
		}

		*pPanelItem=ItemsList;
		*pItemsNumber=ItemsNumber;
	}
	return TRUE;
}

int WINAPI apiGetPluginDirList(const GUID* PluginId, HANDLE hPlugin, const wchar_t *Dir, PluginPanelItem **pPanelItem, size_t *pItemsNumber)
{
	if (FrameManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
		return FALSE;
	return GetPluginDirList(GuidToPlugin(PluginId), hPlugin, Dir, pPanelItem, pItemsNumber);
}

void WINAPI apiFreeDirList(PluginPanelItem *PanelItem, size_t nItemsNumber)
{
	return FreeDirList(PanelItem, nItemsNumber);
}

void WINAPI apiFreePluginDirList(PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	if (!PanelItem)
		return;

	for (size_t I=0; I<ItemsNumber; I++)
	{
		PluginPanelItem *CurPanelItem=PanelItem+I;

		if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
		{
			xf_free((void *)CurPanelItem->UserData);
		}

		FreePluginPanelItem(CurPanelItem);
	}

	xf_free(PanelItem);
}

int WINAPI apiViewer(const wchar_t *FileName,const wchar_t *Title,
                     int X1,int Y1,int X2, int Y2,unsigned __int64 Flags, UINT CodePage)
{
	if (FrameManager->ManagerIsDown())
		return FALSE;

	class ConsoleTitle ct;
	int DisableHistory=(Flags & VF_DISABLEHISTORY)?TRUE:FALSE;

	// $ 15.05.2002 SKV - Запретим вызов немодального редактора вьюера из модального.
	if (FrameManager->InModalEV())
	{
		Flags&=~VF_NONMODAL;
	}

	if (Flags & VF_NONMODAL)
	{
		/* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
		FileViewer *Viewer=new FileViewer(FileName,TRUE,DisableHistory,Title,X1,Y1,X2,Y2,CodePage);

		if (!Viewer)
			return FALSE;

		/* $ 14.06.2002 IS
		   Обработка VF_DELETEONLYFILEONCLOSE - этот флаг имеет более низкий
		   приоритет по сравнению с VF_DELETEONCLOSE
		*/
		if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
			Viewer->SetTempViewName(FileName,(Flags&VF_DELETEONCLOSE)?TRUE:FALSE);

		Viewer->SetEnableF6(Flags & VF_ENABLE_F6);

		/* $ 21.05.2002 SKV
		  Запускаем свой цикл только если не был указан флаг.
		*/
		if (!(Flags&VF_IMMEDIATERETURN))
		{
			FrameManager->ExecuteNonModal();
		}
		else
		{
			if (GlobalSaveScrPtr)
				GlobalSaveScrPtr->Discard();

			FrameManager->PluginCommit();
		}
	}
	else
	{
		/* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
		FileViewer Viewer(FileName,FALSE,DisableHistory,Title,X1,Y1,X2,Y2,CodePage);

		Viewer.SetEnableF6(Flags & VF_ENABLE_F6);

		/* $ 28.05.2001 По умолчанию Вьюер, поэтому нужно здесь признак выставиль явно */
		Viewer.SetDynamicallyBorn(false);
		FrameManager->EnterModalEV();
		FrameManager->ExecuteModal();
		FrameManager->ExitModalEV();

		/* $ 14.06.2002 IS
		   Обработка VF_DELETEONLYFILEONCLOSE - этот флаг имеет более низкий
		   приоритет по сравнению с VF_DELETEONCLOSE
		*/
		if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
			Viewer.SetTempViewName(FileName,(Flags&VF_DELETEONCLOSE)?TRUE:FALSE);

		if (!Viewer.GetExitCode())
		{
			return FALSE;
		}
	}

	return TRUE;
}

int WINAPI apiEditor(const wchar_t* FileName, const wchar_t* Title, int X1, int Y1, int X2, int Y2, unsigned __int64 Flags, int StartLine, int StartChar, UINT CodePage)
{
	if (FrameManager->ManagerIsDown())
		return EEC_OPEN_ERROR;

	ConsoleTitle ct;
	/* $ 12.07.2000 IS
	 Проверка флагов редактора (раньше они игнорировались) и открытие
	 немодального редактора, если есть соответствующий флаг
	*/
	int CreateNew = (Flags & EF_CREATENEW)?TRUE:FALSE;
	int Locked=(Flags & EF_LOCKED)?TRUE:FALSE;
	int DisableHistory=(Flags & EF_DISABLEHISTORY)?TRUE:FALSE;
	int DisableSavePos=(Flags & EF_DISABLESAVEPOS)?TRUE:FALSE;
	/* $ 14.06.2002 IS
	   Обработка EF_DELETEONLYFILEONCLOSE - этот флаг имеет более низкий
	   приоритет по сравнению с EF_DELETEONCLOSE
	*/
	int DeleteOnClose = 0;

	if (Flags & EF_DELETEONCLOSE)
		DeleteOnClose = 1;
	else if (Flags & EF_DELETEONLYFILEONCLOSE)
		DeleteOnClose = 2;

	int OpMode=FEOPMODE_QUERY;

	if ((Flags&EF_OPENMODE_MASK) )
		OpMode=Flags&EF_OPENMODE_MASK;

	/*$ 15.05.2002 SKV
	  Запретим вызов немодального редактора, если находимся в модальном
	  редакторе или вьюере.
	*/
	if (FrameManager->InModalEV())
	{
		Flags&=~EF_NONMODAL;
	}

	int editorExitCode;
	int ExitCode=EEC_OPEN_ERROR;
	string strTitle(Title);

	if (Flags & EF_NONMODAL)
	{
		/* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
		FileEditor *Editor=new FileEditor(FileName,CodePage,
		                                  (CreateNew?FFILEEDIT_CANNEWFILE:0)|FFILEEDIT_ENABLEF6|
		                                   (DisableHistory?FFILEEDIT_DISABLEHISTORY:0)|
		                                   (Locked?FFILEEDIT_LOCKED:0)|
		                                   (DisableSavePos?FFILEEDIT_DISABLESAVEPOS:0),
		                                  StartLine,StartChar,&strTitle,
		                                  X1,Y1,X2,Y2,
		                                  DeleteOnClose,OpMode);

		if (Editor)
		{
			editorExitCode=Editor->GetExitCode();

			// добавочка - проверка кода возврата (почему возникает XC_OPEN_ERROR - см. код FileEditor::Init())
			if (editorExitCode == XC_OPEN_ERROR || editorExitCode == XC_LOADING_INTERRUPTED)
			{
				delete Editor;
				Editor=nullptr;
				return editorExitCode;
			}

			Editor->SetEnableF6((Flags & EF_ENABLE_F6) );
			Editor->SetPluginTitle(&strTitle);

			/* $ 21.05.2002 SKV - Запускаем свой цикл, только если не был указан флаг. */
			if (!(Flags&EF_IMMEDIATERETURN))
			{
				FrameManager->ExecuteNonModal();
			}
			else
			{
				if (GlobalSaveScrPtr)
					GlobalSaveScrPtr->Discard();

				FrameManager->PluginCommit();
			}

			ExitCode=XC_MODIFIED;
		}
	}
	else
	{
		/* 09.09.2001 IS ! Добавим имя файла в историю, если потребуется */
		FileEditor Editor(FileName,CodePage,
		                  (CreateNew?FFILEEDIT_CANNEWFILE:0)|
		                    (DisableHistory?FFILEEDIT_DISABLEHISTORY:0)|
		                    (Locked?FFILEEDIT_LOCKED:0)|
		                    (DisableSavePos?FFILEEDIT_DISABLESAVEPOS:0),
		                  StartLine,StartChar,&strTitle,
		                  X1,Y1,X2,Y2,
		                  DeleteOnClose,OpMode);
		editorExitCode=Editor.GetExitCode();

		// выполним предпроверку (ошибки разные могут быть)
		if (editorExitCode == XC_OPEN_ERROR || editorExitCode == XC_LOADING_INTERRUPTED)
			ExitCode=editorExitCode;
		else
		{
			Editor.SetDynamicallyBorn(false);
			Editor.SetEnableF6((Flags & EF_ENABLE_F6) );
			Editor.SetPluginTitle(&strTitle);
			/* $ 15.05.2002 SKV
			  Зафиксируем вход и выход в/из модального редактора.
			*/
			FrameManager->EnterModalEV();
			FrameManager->ExecuteModal();
			FrameManager->ExitModalEV();
			ExitCode = Editor.GetExitCode();

			if (ExitCode)
			{
#if 0

				if (OpMode==FEOPMODE_BREAKIFOPEN && ExitCode==XC_QUIT)
					ExitCode = XC_OPEN_ERROR;
				else
#endif
					ExitCode = Editor.IsFileChanged()?XC_MODIFIED:XC_NOT_MODIFIED;
			}
		}
	}

	return ExitCode;
}

void WINAPI apiText(int X,int Y,const FarColor* Color,const wchar_t *Str)
{
	if (DisablePluginsOutput || FrameManager->ManagerIsDown())
		return;

	if (!Str)
	{
		int PrevLockCount=ScrBuf.GetLockCount();
		ScrBuf.SetLockCount(0);
		ScrBuf.Flush();
		ScrBuf.SetLockCount(PrevLockCount);
	}
	else
	{
		Text(X,Y,*Color,Str);
	}
}

intptr_t WINAPI apiEditorControl(int EditorID, EDITOR_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	if (FrameManager->ManagerIsDown())
		return 0;

	if (EditorID == -1)
	{
		if (CtrlObject->Plugins->CurEditor)
			return CtrlObject->Plugins->CurEditor->EditorControl(Command,(void *)Param2);

		return 0;
	}
	else
	{
		typedef Frame* (Manager::*ItemFn)(size_t index)const;
		typedef int (Manager::*CountFn)(void)const;
		ItemFn getitem[]={&Manager::operator[],&Manager::GetModalFrame};
		CountFn getcount[]={&Manager::GetFrameCount,&Manager::GetModalStackCount};
		for(size_t ii=0;ii<ARRAYSIZE(getitem);++ii)
		{
			Frame *frame;
			int count=(FrameManager->*getcount[ii])();
			for(int jj=0;jj<count;++jj)
			{
				frame=(FrameManager->*getitem[ii])(jj);
				if (frame->GetType() == MODALTYPE_EDITOR)
				{
					if (((FileEditor*)frame)->GetId() == EditorID)
					{
						return ((FileEditor*)frame)->EditorControl(Command,(void *)Param2);
					}
				}
			}
		}
	}

	return 0;
}

intptr_t WINAPI apiViewerControl(int ViewerID, VIEWER_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	if (FrameManager->ManagerIsDown())
		return 0;

	if (ViewerID == -1)
	{
		if (CtrlObject->Plugins->CurViewer)
			return CtrlObject->Plugins->CurViewer->ViewerControl(Command,(void *)Param2);

		return 0;
	}
	else
	{
		int idx=0;
		Frame *frame;
		while((frame=FrameManager->Manager::operator[](idx++)) != nullptr)
		{
			if (frame->GetType() == MODALTYPE_VIEWER)
			{
				if (((FileViewer*)frame)->GetId() == ViewerID)
				{
					return ((FileViewer*)frame)->ViewerControl(Command,(void *)Param2);
				}
			}
		}
	}

	return 0;
}

void WINAPI apiUpperBuf(wchar_t *Buf, int Length)
{
	return UpperBuf(Buf, Length);
}

void WINAPI apiLowerBuf(wchar_t *Buf, int Length)
{
	return LowerBuf(Buf, Length);
}

void WINAPI apiStrUpper(wchar_t *s1)
{
	return StrUpper(s1);
}

void WINAPI apiStrLower(wchar_t *s1)
{
	return StrLower(s1);
}

wchar_t WINAPI apiUpper(wchar_t Ch)
{
	return Upper(Ch);
}

wchar_t WINAPI apiLower(wchar_t Ch)
{
	return Lower(Ch);
}

int WINAPI apiStrCmpNI(const wchar_t *s1, const wchar_t *s2, int n)
{
	return StrCmpNI(s1, s2, n);
}

int WINAPI apiStrCmpI(const wchar_t *s1, const wchar_t *s2)
{
	return StrCmpI(s1, s2);
}

int WINAPI apiIsLower(wchar_t Ch)
{
	return IsLower(Ch);
}

int WINAPI apiIsUpper(wchar_t Ch)
{
	return IsUpper(Ch);
}

int WINAPI apiIsAlpha(wchar_t Ch)
{
	return IsAlpha(Ch);
}

int WINAPI apiIsAlphaNum(wchar_t Ch)
{
	return IsAlphaNum(Ch);
}

wchar_t* WINAPI apiTruncStr(wchar_t *Str,int MaxLength)
{
	return TruncStr(Str, MaxLength);
}

wchar_t* WINAPI apiTruncStrFromCenter(wchar_t *Str, int MaxLength)
{
	return TruncStrFromCenter(Str, MaxLength);
}

wchar_t* WINAPI apiTruncStrFromEnd(wchar_t *Str,int MaxLength)
{
	return TruncStrFromEnd(Str, MaxLength);
}

wchar_t* WINAPI apiTruncPathStr(wchar_t *Str, int MaxLength)
{
	return TruncPathStr(Str, MaxLength);
}

const wchar_t* WINAPI apiPointToName(const wchar_t *lpwszPath)
{
	return PointToName(lpwszPath);
}

size_t WINAPI apiGetFileOwner(const wchar_t *Computer,const wchar_t *Name, wchar_t *Owner,size_t Size)
{
	string strOwner;
	GetFileOwner(Computer,Name,strOwner);

	if (Owner && Size)
		xwcsncpy(Owner,strOwner,Size);

	return strOwner.GetLength()+1;
}

size_t WINAPI apiConvertPath(CONVERTPATHMODES Mode,const wchar_t *Src, wchar_t *Dest, size_t DestSize)
{
	if (Src && *Src)
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
			xwcsncpy(Dest, strDest.CPtr(), DestSize);

		return strDest.GetLength() + 1;
	}
	else
	{
		if (Dest && DestSize)
			*Dest = 0;

		return 1;
	}
}

size_t WINAPI apiGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest, size_t DestSize)
{
	if (Src && *Src)
	{
		string strSrc(Src);
		string strDest;
		AddEndSlash(strDest);
		GetReparsePointInfo(strSrc,strDest,nullptr);

		if (DestSize && Dest)
			xwcsncpy(Dest,strDest,DestSize);

		return strDest.GetLength()+1;
	}
	else
	{
		if (DestSize && Dest)
			*Dest = 0;
		return 1;
	}
}

size_t WINAPI apiGetNumberOfLinks(const wchar_t* Name)
{
	string strName(Name);
	return GetNumberOfLinks(strName);
}

size_t WINAPI apiGetPathRoot(const wchar_t *Path, wchar_t *Root, size_t DestSize)
{
	if (Path && *Path)
	{
		string strPath(Path), strRoot;
		GetPathRoot(strPath,strRoot);

		if (DestSize && Root)
			xwcsncpy(Root,strRoot,DestSize);

		return strRoot.GetLength()+1;
	}
	else
	{
		if (DestSize && Root)
			*Root = 0;

		return 1;
	}
}

int WINAPI apiCopyToClipboard(const wchar_t *Data)
{
	return CopyToClipboard(Data);
}

wchar_t* WINAPI apiPasteFromClipboard()
{
	return PasteFromClipboard();
}

intptr_t WINAPI apiMacroControl(const GUID* PluginId, FAR_MACRO_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	if (CtrlObject) // все зависит от этой бадяги.
	{
		KeyMacro& Macro=CtrlObject->Macro; //??

		switch (Command)
		{
			// Param1=0, Param2 - 0
			case MCTL_LOADALL: // из реестра в память ФАР с затиранием предыдущего
			{
				if (Macro.IsRecording())
					return FALSE;
				return Macro.LoadMacros(!Macro.IsExecuting());
			}

			// Param1=0, Param2 - 0
			case MCTL_SAVEALL: // из памяти ФАРа в реестра
			{
				if (Macro.IsRecording()) // || Macro.IsExecuting())
					return FALSE;

				Macro.SaveMacros();
				return TRUE;
			}

			// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroSendMacroText
			case MCTL_SENDSTRING:
			{
				if (!Param2)
					break;

				MacroSendMacroText *PlainText=(MacroSendMacroText*)Param2;

				if (!CheckStructSize(PlainText) || !PlainText->SequenceText || !*PlainText->SequenceText)
					break;

				switch (Param1)
				{
#ifdef FAR_LUA
					// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroSendMacroText*
					case MSSC_POST:
					{
						return Macro.PostNewMacro(PlainText->SequenceText,PlainText->Flags|MFLAGS_POSTFROMPLUGIN,InputRecordToKey(&PlainText->AKey));
					}

					// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroSendMacroText*
					case MSSC_CHECK:
					{
						return Macro.ParseMacroString(PlainText->SequenceText,(PlainText->Flags&KMFLAGS_SILENTCHECK)!=0);
					}
#else
					// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroSendMacroText*
					case MSSC_POST:
					{
						return Macro.PostNewMacro(PlainText->SequenceText,(PlainText->Flags<<8)|MFLAGS_POSTFROMPLUGIN,InputRecordToKey(&PlainText->AKey));
					}

					// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroSendMacroText*
					case MSSC_EXEC:
					{
						break;
					}

					// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroSendMacroText*
					case MSSC_CHECK:
					{
						MacroRecord CurMacro={};
						int Ret=Macro.ParseMacroString(&CurMacro,PlainText->SequenceText,(PlainText->Flags&KMFLAGS_SILENTCHECK)?TRUE:FALSE);

						if (Ret)
						{
							if (CurMacro.BufferSize > 1)
								xf_free(CurMacro.Buffer);
						}

						return Ret;
					}
#endif
				}

				break;
			}

			// Param1=0, Param2 - 0
			case MCTL_GETSTATE:
			{
				return Macro.GetCurRecord(nullptr,nullptr);
			}

			// Param1=0, Param2 - 0
			case MCTL_GETAREA:
			{
				int Area=Macro.GetMode();
				if (Area == MACRO_COMMON)
					Area = MACROAREA_COMMON;
				return Area;
			}

			case MCTL_ADDMACRO:
			{
				if (!Param2)
					break;

				MacroAddMacro *Data=(MacroAddMacro*)Param2;
				MACROFLAGS_MFLAGS Flags=0;

				if (Data->Flags&KMFLAGS_DISABLEOUTPUT)
					Flags|=MFLAGS_DISABLEOUTPUT;

				if (Data->Flags&KMFLAGS_NOSENDKEYSTOPLUGINS)
					Flags|=MFLAGS_NOSENDKEYSTOPLUGINS;

				if (CheckStructSize(Data) && Data->SequenceText && *Data->SequenceText)
				{
					MACROMODEAREA Area=static_cast<MACROMODEAREA>(Data->Area);
					if (Data->Area == MACROAREA_COMMON)
						Area=MACRO_COMMON;

					return Macro.AddMacro(Data->SequenceText,Data->Description,Area,Flags,Data->AKey,*PluginId,Data->Id,Data->Callback);
				}
				break;
			}

			case MCTL_DELMACRO:
			{
				return Macro.DelMacro(*PluginId,Param2);
			}

#ifdef FAR_LUA
			case MCTL_ISFARSPRING:
			{
				return 1; // Дадим LuaFAR'у возможность узнать, что это "Spring-версия" Фара.
			}

			case MCTL_CALLFAR:
			{
				return Macro.CallFar(Param1, (FarMacroCall*)Param2);
			}

			default: //FIXME
				break;
#else
			//Param1=size of buffer, Param2 - MacroParseResult*
			case MCTL_GETLASTERROR:
			{
				DWORD ErrCode=MPEC_SUCCESS;
				COORD ErrPos={};
				string ErrSrc;

				Macro.GetMacroParseError(&ErrCode,&ErrPos,&ErrSrc);

				int Size = ALIGN(sizeof(MacroParseResult));
				size_t stringOffset = Size;
				Size += static_cast<int>((ErrSrc.GetLength() + 1)*sizeof(wchar_t));

				MacroParseResult *Result = (MacroParseResult *)Param2;

				if (Param1 >= Size && CheckStructSize(Result))
				{
					Result->StructSize = sizeof(MacroParseResult);
					Result->ErrCode = ErrCode;
					Result->ErrPos = ErrPos;
					Result->ErrSrc = (const wchar_t *)((char*)Param2+stringOffset);
					wmemcpy((wchar_t*)Result->ErrSrc,ErrSrc,ErrSrc.GetLength()+1);
				}

				return Size;
			}
#endif
		}
	}


	return 0;
}

intptr_t WINAPI apiPluginsControl(HANDLE Handle, FAR_PLUGINS_CONTROL_COMMANDS Command, int Param1, void* Param2)
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
					return reinterpret_cast<intptr_t>(CtrlObject->Plugins->LoadPluginExternal(strPath, Command == PCTL_FORCEDLOADPLUGIN));
				}
			}
			break;

		case PCTL_FINDPLUGIN:
		{
			Plugin* plugin = nullptr;
			switch(Param1)
			{
				case PFM_GUID:
					plugin = CtrlObject->Plugins->FindPlugin(*reinterpret_cast<GUID*>(Param2));
					break;

				case PFM_MODULENAME:
				{
					string strPath;
					ConvertNameToFull(reinterpret_cast<const wchar_t*>(Param2), strPath);
					for (size_t i = 0; i < CtrlObject->Plugins->GetPluginsCount(); ++i)
					{
						Plugin* p = CtrlObject->Plugins->GetPlugin(i);
						if (!StrCmpI(p->GetModuleName(), strPath))
						{
							plugin = p;
							break;
						}
					}
					break;
				}
			}
			if(plugin&&CtrlObject->Plugins->IsPluginUnloaded(plugin)) plugin=nullptr;
			return reinterpret_cast<intptr_t>(plugin);
		}

		case PCTL_UNLOADPLUGIN:
			{
				return CtrlObject->Plugins->UnloadPluginExternal(Handle);
			}
			break;

		case PCTL_GETPLUGININFORMATION:
			{
				FarGetPluginInformation* Info = reinterpret_cast<FarGetPluginInformation*>(Param2);
				if (!Info || (CheckStructSize(Info) && static_cast<size_t>(Param1) > sizeof(FarGetPluginInformation)))
				{
					Plugin* plugin = reinterpret_cast<Plugin*>(Handle);
					if(plugin)
					{
						return CtrlObject->Plugins->GetPluginInformation(plugin, Info, Param1);
					}
				}
			}
			break;

		case PCTL_GETPLUGINS:
			{
				size_t PluginsCount = CtrlObject->Plugins->GetPluginsCount();
				if(Param1 && Param2)
				{
					HANDLE* Plugins = static_cast<HANDLE*>(Param2);
					size_t Count = Min(static_cast<size_t>(Param1), PluginsCount);
					for(size_t i = 0; i < Count; ++i)
					{
						Plugins[i] = CtrlObject->Plugins->GetPlugin(i);
					}
				}
				return PluginsCount;
			}
			break;
	}

	return 0;
}

intptr_t WINAPI apiFileFilterControl(HANDLE hHandle, FAR_FILE_FILTER_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	FileFilter *Filter=nullptr;

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

			Filter = new FileFilter((Panel *)hHandle, (FAR_FILE_FILTER_TYPE)Param1);

			if (Filter)
			{
				*((HANDLE *)Param2) = (HANDLE)Filter;
				return TRUE;
			}

			break;
		}
		case FFCTL_FREEFILEFILTER:
		{
			delete Filter;
			return TRUE;
		}
		case FFCTL_OPENFILTERSMENU:
		{
			return Filter->FilterEdit() ? TRUE : FALSE;
		}
		case FFCTL_STARTINGTOFILTER:
		{
			Filter->UpdateCurrentTime();
			return TRUE;
		}
		case FFCTL_ISFILEINFILTER:
		{
			if (!Param2)
				break;
			return Filter->FileInFilter(*reinterpret_cast<const PluginPanelItem*>(Param2)) ? TRUE : FALSE;
		}
	}

	return FALSE;
}

intptr_t WINAPI apiRegExpControl(HANDLE hHandle, FAR_REGEXP_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	RegExp* re=nullptr;

	if (Command != RECTL_CREATE)
	{
		if (!hHandle || hHandle == INVALID_HANDLE_VALUE)
			return FALSE;

		re = (RegExp*)hHandle;
	}

	switch (Command)
	{
		case RECTL_CREATE:

			if (!Param2)
				break;

			*((HANDLE*)Param2) = INVALID_HANDLE_VALUE;
			re = new RegExp;

			if (re)
			{
				*((HANDLE*)Param2) = (HANDLE)re;
				return TRUE;
			}

			break;
		case RECTL_FREE:
			delete re;
			return TRUE;
		case RECTL_COMPILE:
			return re->Compile((const wchar_t*)Param2,OP_PERLSTYLE);
		case RECTL_OPTIMIZE:
			return re->Optimize();
		case RECTL_MATCHEX:
		{
			RegExpSearch* data=(RegExpSearch*)Param2;
			return re->MatchEx(data->Text,data->Text+data->Position,data->Text+data->Length,data->Match,data->Count
#ifdef NAMEDBRACKETS
			                   ,data->Reserved
#endif
			                  );
		}
		case RECTL_SEARCHEX:
		{
			RegExpSearch* data=(RegExpSearch*)Param2;
			return re->SearchEx(data->Text,data->Text+data->Position,data->Text+data->Length,data->Match,data->Count
#ifdef NAMEDBRACKETS
			                    ,data->Reserved
#endif
			                   );
		}
		case RECTL_BRACKETSCOUNT:
			return re->GetBracketsCount();
	}

	return FALSE;
}

intptr_t WINAPI apiSettingsControl(HANDLE hHandle, FAR_SETTINGS_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	AbstractSettings* settings=nullptr;

	if (Command != SCTL_CREATE)
	{
		if (!hHandle || hHandle == INVALID_HANDLE_VALUE)
			return FALSE;

		settings = (AbstractSettings*)hHandle;
	}

	switch (Command)
	{
		case SCTL_CREATE:

			if (!Param2)
				break;

			{
				FarSettingsCreate* data = (FarSettingsCreate*)Param2;
				if (CheckStructSize(data))
				{
					if(IsEqualGUID(FarGuid,data->Guid)) settings=new FarSettings();
					else settings=new PluginSettings(data->Guid, Param1 == PSL_LOCAL);
					if (settings->IsValid())
					{
						data->Handle=settings;
						return TRUE;
					}
					delete settings;
				}
			}
			break;
		case SCTL_FREE:
			delete settings;
			return TRUE;
		case SCTL_SET:
			return settings->Set(*(const FarSettingsItem*)Param2);
		case SCTL_GET:
			return settings->Get(*(FarSettingsItem*)Param2);
		case SCTL_ENUM:
			return settings->Enum(*(FarSettingsEnum*)Param2);
		case SCTL_DELETE:
			return settings->Delete(*(const FarSettingsValue*)Param2);
		case SCTL_CREATESUBKEY:
		case SCTL_OPENSUBKEY:
			return settings->SubKey(*(const FarSettingsValue*)Param2, Command==SCTL_CREATESUBKEY);
	}

	return FALSE;
}

size_t WINAPI apiGetCurrentDirectory(size_t Size,wchar_t* Buffer)
{
	string strCurDir;
	apiGetCurrentDirectory(strCurDir);

	if (Buffer && Size)
	{
		xwcsncpy(Buffer,strCurDir,Size);
	}

	return strCurDir.GetLength()+1;
}

size_t WINAPI apiFormatFileSize(unsigned __int64 Size, int Width, FARFORMATFILESIZEFLAGS Flags, wchar_t *Dest, size_t DestSize)
{
	static unsigned __int64 FlagsPair[]={
		FFFS_COMMAS,            COLUMN_COMMAS,         // Вставлять разделитель между тысячами
		FFFS_THOUSAND,          COLUMN_THOUSAND,       // Вместо делителя 1024 использовать делитель 1000
		FFFS_FLOATSIZE,         COLUMN_FLOATSIZE,      // Показывать размер файла в стиле Windows Explorer (т.е. 999 байт будут показаны как 999, а 1000 байт как 0.97 K)
		FFFS_ECONOMIC,          COLUMN_ECONOMIC,       // Экономичный режим, не показывать пробел перед суффиксом размера файла (т.е. 0.97K)
		FFFS_MINSIZEINDEX,      COLUMN_MINSIZEINDEX,   // Минимально допустимый индекс при форматировании
		FFFS_SHOWBYTESINDEX,    COLUMN_SHOWBYTESINDEX, // Показывать суффиксы B,K,M,G,T,P,E
	};

	string strDestStr;
	unsigned __int64 FinalFlags=Flags & COLUMN_MINSIZEINDEX_MASK;
	for (size_t I=0; I < ARRAYSIZE(FlagsPair); I+=2)
		if (Flags & FlagsPair[I])
			FinalFlags |= FlagsPair[I+1];

	FileSizeToStr(strDestStr,Size,Width,FinalFlags);

	if (Dest && DestSize)
	{
		xwcsncpy(Dest,strDestStr,DestSize);
	}

	return strDestStr.GetLength()+1;
}

/* $ 30.07.2001 IS
     1. Проверяем правильность параметров.
     2. Теперь обработка каталогов не зависит от маски файлов
     3. Маска может быть стандартного фаровского вида (со скобками,
        перечислением и пр.). Может быть несколько масок файлов, разделенных
        запятыми или точкой с запятой, можно указывать маски исключения,
        можно заключать маски в кавычки. Короче, все как и должно быть :-)
*/

void WINAPI apiRecursiveSearch(const wchar_t *InitDir,const wchar_t *Mask,FRSUSERFUNC Func,unsigned __int64 Flags,void *Param)
{
	if (Func && InitDir && *InitDir && Mask && *Mask)
	{
		CFileMask FMask;

		if (!FMask.Set(Mask, FMF_SILENT)) return;

		Flags=Flags&0x000000FF; // только младший байт!
		ScanTree ScTree(Flags & FRS_RETUPDIR,Flags & FRS_RECUR, Flags & FRS_SCANSYMLINK);
		FAR_FIND_DATA_EX FindData;
		string strFullName;
		ScTree.SetFindPath(InitDir,L"*");

		while (ScTree.GetNextName(&FindData,strFullName))
		{
			if (FMask.Compare(FindData.strFileName))
			{
				PluginPanelItem fdata;
				FindDataExToPluginPanelItem(&FindData, &fdata);

				if (!Func(&fdata,strFullName,Param))
				{
					FreePluginPanelItem(&fdata);
					break;
				}

				FreePluginPanelItem(&fdata);
			}
		}
	}
}

/* $ 14.09.2000 SVS
 + Функция FarMkTemp - получение имени временного файла с полным путем.
    Dest - приемник результата
    Template - шаблон по правилам функции mktemp, например "FarTmpXXXXXX"
    Вернет требуемый размер приемника.
*/
size_t WINAPI apiMkTemp(wchar_t *Dest, size_t DestSize, const wchar_t *Prefix)
{
	string strDest;
	if (FarMkTempEx(strDest, Prefix, TRUE) && Dest && DestSize)
	{
		xwcsncpy(Dest, strDest, DestSize);
	}
	return strDest.GetLength()+1;
}

size_t WINAPI apiProcessName(const wchar_t *param1, wchar_t *param2, size_t size, PROCESSNAME_FLAGS flags)
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
		{
			return CmpName(param1, param2, (Flags&PN_SKIPPATH)!=0);
		}

	case PN_CMPNAMELIST:
	case PN_CHECKMASK:
		{
			static CFileMask Masks;
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
			string strResult = param2;
			int nResult = ConvertWildcards(param1, strResult, Length);
			xwcsncpy(param2, strResult, size);
			return nResult;
		}
	}
	return FALSE;
}

BOOL WINAPI apiColorDialog(const GUID* PluginId, COLORDIALOGFLAGS Flags, struct FarColor *Color)
{
	BOOL Result = FALSE;
	if (!FrameManager->ManagerIsDown())
	{
		Result = Console.GetColorDialog(*Color, true, false);
	}
	return Result;
}

size_t WINAPI apiInputRecordToKeyName(const INPUT_RECORD* Key, wchar_t *KeyText, size_t Size)
{
	int iKey = InputRecordToKey(Key);
	string strKT;
	if (!KeyToText(iKey,strKT))
		return 0;
	size_t len = strKT.GetLength();
	if (Size && KeyText)
	{
		if (Size <= len)
			len = Size-1;
		wmemcpy(KeyText, strKT.CPtr(), len);
		KeyText[len] = 0;
	}
	else if (KeyText)
		*KeyText = 0;
	return (len+1);
}

BOOL WINAPI apiKeyNameToInputRecord(const wchar_t *Name,INPUT_RECORD* RecKey)
{
	int Key=KeyNameToKey(Name);
	return Key > 0?(KeyToInputRecord(Key,RecKey)!=0?TRUE:FALSE):FALSE;
}

BOOL WINAPI apiMkLink(const wchar_t *Src,const wchar_t *Dest, LINK_TYPE Type, MKLINK_FLAGS Flags)
{
	int Result=0;

	if (Src && *Src && Dest && *Dest)
	{
		switch (Type)
		{
		case LINK_HARDLINK:
			Result=MkHardLink(Src,Dest);
			break;
		case LINK_JUNCTION:
		case LINK_VOLMOUNT:
		case LINK_SYMLINKFILE:
		case LINK_SYMLINKDIR:
			{
				ReparsePointTypes LinkType=RP_JUNCTION;

				switch (Type)
				{
				case LINK_VOLMOUNT:
					LinkType=RP_VOLMOUNT;
					break;
				case LINK_SYMLINK:
					LinkType=RP_SYMLINK;
					break;
				case LINK_SYMLINKFILE:
					LinkType=RP_SYMLINKFILE;
					break;
				case LINK_SYMLINKDIR:
					LinkType=RP_SYMLINKDIR;
					break;
				default:
					break;
				}

				Result=MkSymLink(Src,Dest,LinkType,(Flags&MLF_SHOWERRMSG?0:FCOPY_NOSHOWMSGLINK));
			}
			break;
		default:
			break;
		}
	}

	if (Result && !(Flags&MLF_DONOTUPDATEPANEL))
		ShellUpdatePanels(nullptr,FALSE);

	return Result;
}

BOOL WINAPI apiAddEndSlash(wchar_t *Path)
{
	return AddEndSlash(Path);
}

wchar_t* WINAPI apiXlat(wchar_t *Line,int StartPos,int EndPos,XLAT_FLAGS Flags)
{
	return Xlat(Line, StartPos, EndPos, Flags);
}

HANDLE WINAPI apiCreateFile(const wchar_t *Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile)
{
	return ::apiCreateFile(Object,DesiredAccess,ShareMode,SecurityAttributes,CreationDistribution,FlagsAndAttributes,TemplateFile);
}

DWORD WINAPI apiGetFileAttributes(const wchar_t *FileName)
{
	return ::apiGetFileAttributes(FileName);
}

BOOL WINAPI apiSetFileAttributes(const wchar_t *FileName,DWORD dwFileAttributes)
{
	return ::apiSetFileAttributes(FileName,dwFileAttributes);
}

BOOL WINAPI apiMoveFileEx(const wchar_t *ExistingFileName,const wchar_t *NewFileName,DWORD dwFlags)
{
	return ::apiMoveFileEx(ExistingFileName,NewFileName,dwFlags);
}

BOOL WINAPI apiDeleteFile(const wchar_t *FileName)
{
	return ::apiDeleteFile(FileName);
}

BOOL WINAPI apiRemoveDirectory(const wchar_t *DirName)
{
	return ::apiRemoveDirectory(DirName);
}

BOOL WINAPI apiCreateDirectory(const wchar_t *PathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return ::apiCreateDirectory(PathName,lpSecurityAttributes);
}
};
