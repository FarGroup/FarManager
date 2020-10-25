/*
panel.cpp

Parent class для панелей
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
#include "panel.hpp"

// Internal:
#include "keyboard.hpp"
#include "flink.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "treelist.hpp"
#include "filelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "shortcuts.hpp"
#include "dirmix.hpp"
#include "constitle.hpp"
#include "uuids.far.hpp"
#include "lang.hpp"
#include "plugins.hpp"
#include "keybar.hpp"
#include "strmix.hpp"
#include "diskmenu.hpp"
#include "cvtname.hpp"
#include "pathmix.hpp"
#include "global.hpp"
#include "fastfind.hpp"

// Platform:
#include "platform.env.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static int DragX,DragY,DragMove;
static Panel *SrcDragPanel;
static std::unique_ptr<SaveScreen> DragSaveScr;

Panel::Panel(window_ptr Owner):
	ScreenObject(std::move(Owner))
{
	_OT(SysLog(L"[%p] Panel::Panel()", this));
	SrcDragPanel=nullptr;
	DragX=DragY=-1;
}


Panel::~Panel()
{
	_OT(SysLog(L"[%p] Panel::~Panel()", this));
	EndDrag();
}


void Panel::SetViewMode(int ViewMode)
{
	m_PrevViewMode=ViewMode;
	m_ViewMode=ViewMode;
}


void Panel::ChangeDirToCurrent()
{
	SetCurDir(os::fs::GetCurrentDirectory(), true);
}

long long Panel::VMProcess(int OpCode, void* vParam, long long iParam)
{
	return 0;
}

void Panel::FastFind(const Manager::Key& FirstKey)
{
	// // _SVS(CleverSysLog Clev(L"Panel::FastFind"));
	Manager::Key KeyToProcess;
	{
		const auto search = FastFind::create(this, FirstKey);
		search->Process();

		if (search->GetExitCode() < 0)
			return;

		KeyToProcess=search->KeyToProcess();
	}
	Show();
	Parent()->GetKeybar().Redraw();
	Global->ScrBuf->Flush();

	const auto TreePanel = std::dynamic_pointer_cast<TreeList>(Parent()->ActivePanel());
	if (TreePanel && (KeyToProcess() == KEY_ENTER || KeyToProcess() == KEY_NUMENTER))
		TreePanel->ProcessEnter();
	else
		Parent()->ProcessKey(KeyToProcess);
}

bool Panel::IsFocused() const
{
	if (const auto FilePanels = Parent())
	{
		return this == FilePanels->ActivePanel().get();
	}
	return true;
}

void Panel::OnFocusChange(bool Get)
{
	ProcessPluginEvent(Get? FE_GOTFOCUS : FE_KILLFOCUS, nullptr);
	if (Get) Redraw();
}

bool Panel::IsMouseInClientArea(const MOUSE_EVENT_RECORD* MouseEvent) const
{
	return IsVisible() &&
		in_closed_range(m_Where.left, MouseEvent->dwMousePosition.X, m_Where.right) &&
		in_closed_range(m_Where.top, MouseEvent->dwMousePosition.Y, m_Where.bottom);
}

bool Panel::ProcessMouseDrag(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (DragX!=-1)
	{
		if (!(MouseEvent->dwButtonState & MOUSE_ANY_BUTTON_PRESSED))
		{
			EndDrag();

			if (!MouseEvent->dwEventFlags && SrcDragPanel!=this)
			{
				MoveToMouse(MouseEvent);
				Redraw();
				SrcDragPanel->ProcessKey(Manager::Key(DragMove ? KEY_DRAGMOVE:KEY_DRAGCOPY));
			}

			return true;
		}

		if (MouseEvent->dwMousePosition.Y <= m_Where.top || MouseEvent->dwMousePosition.Y >= m_Where.bottom ||
			!Parent()->GetAnotherPanel(SrcDragPanel)->IsVisible())
		{
			EndDrag();
			return true;
		}

		if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED && !MouseEvent->dwEventFlags)
			DragMove=!DragMove;

		if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			if ((abs(MouseEvent->dwMousePosition.X-DragX)>15 || SrcDragPanel!=this) && !m_ModalMode)
			{
				if (SrcDragPanel->GetSelCount()==1 && !DragSaveScr)
				{
					SrcDragPanel->GoToFile(strDragName);
					SrcDragPanel->Show();
				}

				DragMessage(MouseEvent->dwMousePosition.X,MouseEvent->dwMousePosition.Y,DragMove);
				return true;
			}
			else
			{
				DragSaveScr.reset();
			}
		}
	}

	if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED && !MouseEvent->dwEventFlags && m_Where.width() - 1 < ScrX)
	{
		MoveToMouse(MouseEvent);
		os::fs::find_data Data;
		if (!get_first_selected(Data))
			return false;

		strDragName = Data.FileName;

		if (!IsParentDirectory(Data))
		{
			SrcDragPanel=this;
			DragX=MouseEvent->dwMousePosition.X;
			DragY=MouseEvent->dwMousePosition.Y;
			DragMove = IntKeyState.ShiftPressed();
		}
	}

	return false;
}


bool Panel::IsDragging()
{
	return DragSaveScr!=nullptr;
}


void Panel::EndDrag()
{
	DragSaveScr.reset();
	DragX=DragY=-1;
}


void Panel::DragMessage(int X,int Y,int Move)
{
	const auto SelCount = SrcDragPanel->GetSelCount();

	if (!SelCount)
		return;

	string strSelName;

	if (SelCount == 1)
	{
		os::fs::find_data Data;
		if (!SrcDragPanel->get_first_selected(Data))
			return;

		strSelName = PointToName(Data.FileName);
		QuoteSpace(strSelName);
	}
	else
	{
		strSelName = format(msg(lng::MDragFiles), SelCount);
	}

	auto strDragMsg = format(msg(Move? lng::MDragMove : lng::MDragCopy), strSelName);

	auto Length = static_cast<int>(strDragMsg.size());
	int MsgX = X;

	if (Length + X > ScrX)
	{
		MsgX=ScrX-Length;

		if (MsgX<0)
		{
			MsgX=0;
			inplace::truncate_right(strDragMsg, ScrX);
			Length = static_cast<int>(strDragMsg.size());
		}
	}

	// Important - the old one must be deleted before creating a new one, not after
	DragSaveScr.reset();
	DragSaveScr = std::make_unique<SaveScreen>(rectangle{ MsgX, Y, MsgX + Length - 1, Y });
	GotoXY(MsgX,Y);
	SetColor(COL_PANELDRAGTEXT);
	Text(strDragMsg);
}


const string& Panel::GetCurDir() const
{
	return m_CurDir;
}


bool Panel::SetCurDir(string_view const NewDir, bool const ClosePanel, bool const IsUpdated, bool const Silent)
{
	InitCurDir(NewDir);
	return true;
}


void Panel::InitCurDir(string_view const CurDir)
{
	if (!equal_icase(m_CurDir, CurDir) || !equal_icase(os::fs::GetCurrentDirectory(), CurDir))
	{
		m_CurDir = CurDir;

		if (m_PanelMode != panel_mode::PLUGIN_PANEL)
		{
			PrepareDiskPath(m_CurDir);
			if(!IsRootPath(m_CurDir))
			{
				DeleteEndSlash(m_CurDir);
			}
		}
	}
}


/* $ 14.06.2001 KM
   + Добавлена установка переменных окружения, определяющих
     текущие директории дисков как для активной, так и для
     пассивной панели. Это необходимо программам запускаемым
     из FAR.
*/
/* $ 05.10.2001 SVS
   ! Давайте для начала выставим нужные значения для пассивной панели,
     а уж потом...
     А то фигня какая-то получается...
*/
/* $ 14.01.2002 IS
   ! Убрал установку переменных окружения, потому что она производится
     в FarChDir, которая теперь используется у нас для установления
     текущего каталога.
*/
bool Panel::SetCurPath()
{
	if (GetMode() == panel_mode::PLUGIN_PANEL)
		return true;

	const auto AnotherPanel = Parent()->GetAnotherPanel(this);

	if (AnotherPanel->GetMode() != panel_mode::PLUGIN_PANEL)
	{
		// Propagate passive panel curent directory to the environment
		// (only if it won't be overwritten by the active)
		if (!AnotherPanel->m_CurDir.empty() && (m_CurDir.empty() || !equal_icase_t{}(AnotherPanel->m_CurDir[0], m_CurDir[0])))
		{
			set_drive_env_curdir(AnotherPanel->m_CurDir);
		}
	}

	if (!FarChDir(m_CurDir))
	{
		while (!FarChDir(m_CurDir))
		{
			const auto strRoot = GetPathRoot(m_CurDir);

			if (os::fs::drive::get_type(strRoot) != DRIVE_REMOVABLE || os::fs::IsDiskInDrive(strRoot))
			{
				if (!os::fs::is_directory(m_CurDir))
				{
					if (CheckShortcutFolder(m_CurDir, true, true) && FarChDir(m_CurDir))
					{
						SetCurDir(m_CurDir,true);
						return true;
					}
				}
				else
					break;
			}

			if (Global->WindowManager->ManagerStarted()) // сначала проверим - а запущен ли менеджер
			{
				SetCurDir(Global->g_strFarPath,true);                    // если запущен - выставим путь который мы точно знаем что существует
				ChangeDisk(shared_from_this());                          // и вызовем меню выбора дисков
			}
			else                                               // оппа...
			{
				string strTemp(m_CurDir);
				CutToParent(m_CurDir);             // подымаемся вверх, для очередной порции ChDir

				if (strTemp.size()==m_CurDir.size())  // здесь проблема - видимо диск недоступен
				{
					SetCurDir(Global->g_strFarPath,true);                 // тогда просто сваливаем в каталог, откуда стартанул FAR.
					break;
				}
				else
				{
					if (FarChDir(m_CurDir))
					{
						SetCurDir(m_CurDir,true);
						break;
					}
				}
			}
		}
		return false;
	}

	return true;
}

void Panel::Hide()
{
	ScreenObject::Hide();
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);

	if (AnotherPanel->IsVisible())
	{
		//if (AnotherPanel->IsFocused())
		if ((AnotherPanel->GetType() == panel_type::FILE_PANEL && AnotherPanel->IsFullScreen()) ||
			(GetType() == panel_type::FILE_PANEL && IsFullScreen()))
				AnotherPanel->Show();
	}
}


void Panel::Show()
{
	if (!GetModalMode())
	{
		const auto AnotherPanel = Parent()->GetAnotherPanel(this);
		if (AnotherPanel->IsVisible())
		{
			if (SaveScr)
			{
				SaveScr->AppendArea(AnotherPanel->SaveScr.get());
			}

			if (AnotherPanel->IsFocused())
			{
				if (AnotherPanel->IsFullScreen())
				{
					SetVisible(true);
					return;
				}

				if (GetType() == panel_type::FILE_PANEL && IsFullScreen())
				{
					ScreenObject::Show();
					AnotherPanel->Show();
					return;
				}
			}
		}
	}

	ScreenObject::Show();
	ShowScreensCount();
}

void Panel::ShowConsoleTitle()
{
	if (!IsFocused())
		return;

	ConsoleTitle::SetFarTitle(m_Title);
}

void Panel::DrawSeparator(int Y) const
{
	if (Y < m_Where.bottom)
	{
		SetColor(COL_PANELBOX);
		GotoXY(m_Where.left, Y);
		DrawLine(m_Where.width(), line_type::h1_to_v2);
	}
}

string Panel::GetTitleForDisplay() const
{
	return truncate_left(concat(L' ', GetTitle()), m_Where.width() - 3) + L' ';
}

void Panel::ShowScreensCount() const
{
	if (Global->Opt->ShowScreensNumber && !m_Where.left)
	{
		const auto Viewers = Global->WindowManager->GetWindowCountByType(windowtype_viewer);
		const auto Editors = Global->WindowManager->GetWindowCountByType(windowtype_editor);
		const auto Dialogs = Global->Opt->ShowScreensNumber > 1? Global->WindowManager->GetWindowCountByType(windowtype_dialog) : 0;

		if (Viewers>0 || Editors>0 || Dialogs > 0)
		{
			GotoXY(m_Where.left + (Global->Opt->ShowColumnTitles? 0 : 2), m_Where.top);
			SetColor(COL_PANELSCREENSNUMBER);

			auto Counter = L'[' + str(Viewers);

			if (Editors > 0)
			{
				Counter += L'+' + str(Editors);
			}

			if (Dialogs > 0)
			{
				Counter += L',' + str(Dialogs);
			}

			Counter += L']';

			Text(Counter);
		}
	}
}


void Panel::GetOpenPanelInfo(OpenPanelInfo* Info) const
{
	*Info = {};
}

void Panel::RefreshTitle()
{
	m_Title = concat(L'{', GetTitle(), L'}');
}

string Panel::GetTitle() const
{
	if (m_PanelMode == panel_mode::NORMAL_PANEL)
		return m_ShowShortNames? ConvertNameToShort(m_CurDir) : m_CurDir;

	OpenPanelInfo Info;
	GetOpenPanelInfo(&Info);
	return string(trim(string_view(NullToEmpty(Info.PanelTitle))));
}

int Panel::SetPluginCommand(int Command,int Param1,void* Param2)
{
	_ALGO(CleverSysLog clv(L"Panel::SetPluginCommand"));
	_ALGO(SysLog(L"(Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));
	int Result=FALSE;
	ProcessingPluginCommand++;

	switch (Command)
	{
		case FCTL_SETVIEWMODE:
			Result = Parent()->ChangePanelViewMode(shared_from_this(), Param1, Parent()->IsTopWindow());
			break;

		case FCTL_SETSORTMODE:
		{
			int Mode=Param1;

			if ((Mode>SM_DEFAULT) && (Mode < SM_COUNT))
			{
				SetSortMode(panel_sort(Mode - 1)); // Уменьшим на 1 из-за SM_DEFAULT
				Result=TRUE;
			}
			break;
		}

		case FCTL_SETSORTORDER:
		{
			ChangeSortOrder(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_SETDIRECTORIESFIRST:
		{
			ChangeDirectoriesFirst(Param1 != 0);
			Result=TRUE;
			break;
		}

		case FCTL_CLOSEPANEL:
			if (m_PanelMode == panel_mode::PLUGIN_PANEL)
			{
				string folder = NullToEmpty(static_cast<const wchar_t*>(Param2));
				SetCurDir(folder,true);
				if (folder.empty())
					Update(UPDATE_KEEP_SELECTION);
				Redraw();
			}
			Result=TRUE;
			break;

		case FCTL_GETPANELINFO:
		{
			const auto Info = static_cast<PanelInfo*>(Param2);

			if(!CheckStructSize(Info))
				break;

			*Info = {};
			Info->StructSize = sizeof(PanelInfo);

			UpdateIfRequired();
			Info->OwnerGuid = FarUuid;
			Info->PluginHandle=nullptr;

			switch (GetType())
			{
			case panel_type::FILE_PANEL:
				Info->PanelType=PTYPE_FILEPANEL;
				break;
			case panel_type::TREE_PANEL:
				Info->PanelType=PTYPE_TREEPANEL;
				break;
			case panel_type::QVIEW_PANEL:
				Info->PanelType=PTYPE_QVIEWPANEL;
				break;
			case panel_type::INFO_PANEL:
				Info->PanelType=PTYPE_INFOPANEL;
				break;
			}

			const auto Rect = GetPosition();
			Info->PanelRect.left = Rect.left;
			Info->PanelRect.top = Rect.top;
			Info->PanelRect.right = Rect.right;
			Info->PanelRect.bottom = Rect.bottom;
			Info->ViewMode=GetViewMode();
			Info->SortMode = static_cast<OPENPANELINFO_SORTMODES>(internal_sort_mode_to_plugin(GetSortMode()));

			Info->Flags |= Global->Opt->ShowHidden? PFLAGS_SHOWHIDDEN : 0;
			Info->Flags |= Global->Opt->Highlight? PFLAGS_HIGHLIGHT : 0;
			Info->Flags |= GetSortOrder()? PFLAGS_REVERSESORTORDER : 0;
			Info->Flags |= GetSortGroups()? PFLAGS_USESORTGROUPS : 0;
			Info->Flags |= GetSelectedFirstMode()? PFLAGS_SELECTEDFIRST : 0;
			Info->Flags |= GetDirectoriesFirst()? PFLAGS_DIRECTORIESFIRST : 0;
			Info->Flags |= (GetMode() == panel_mode::PLUGIN_PANEL)? PFLAGS_PLUGIN : 0;
			Info->Flags |= IsVisible()? PFLAGS_VISIBLE : 0;
			Info->Flags |= IsFocused()? PFLAGS_FOCUS : 0;
			Info->Flags |= Parent()->IsLeft(this)? PFLAGS_PANELLEFT : 0;

			if (GetType() == panel_type::FILE_PANEL)
			{
				const auto DestFilePanel = static_cast<FileList*>(this);

				if (Info->Flags&PFLAGS_PLUGIN)
				{
					Info->OwnerGuid = DestFilePanel->GetPluginHandle()->plugin()->Id();
					Info->PluginHandle = DestFilePanel->GetPluginHandle()->panel();
					static int Reenter=0;
					if (!Reenter)
					{
						Reenter++;
						OpenPanelInfo PInfo;
						DestFilePanel->GetOpenPanelInfo(&PInfo);

						if (PInfo.Flags & OPIF_REALNAMES)
							Info->Flags |= PFLAGS_REALNAMES;

						if (PInfo.Flags & OPIF_DISABLEHIGHLIGHTING)
							Info->Flags &= ~PFLAGS_HIGHLIGHT;

						if (PInfo.Flags & OPIF_USECRC32)
							Info->Flags |= PFLAGS_USECRC32;

						if (PInfo.Flags & OPIF_SHORTCUT)
							Info->Flags |= PFLAGS_SHORTCUT;

						Reenter--;
					}
				}

				DestFilePanel->PluginGetPanelInfo(*Info);
			}

			if (!(Info->Flags&PFLAGS_PLUGIN)) // $ 12.12.2001 DJ - на неплагиновой панели - всегда реальные имена
				Info->Flags |= PFLAGS_REALNAMES;

			Result=TRUE;
			break;
		}

		case FCTL_GETPANELPREFIX:
		{
			string strTemp;

			if (GetType() == panel_type::FILE_PANEL && GetMode() == panel_mode::PLUGIN_PANEL)
			{
				PluginInfo PInfo = {sizeof(PInfo)};
				const auto DestPanel = static_cast<const FileList*>(this);
				if (DestPanel->GetPluginInfo(&PInfo))
					strTemp = NullToEmpty(PInfo.CommandPrefix);
			}

			if (Param1&&Param2)
				xwcsncpy(static_cast<wchar_t*>(Param2), strTemp.c_str(), Param1);

			Result = static_cast<int>(strTemp.size() + 1);
			break;
		}

		case FCTL_GETPANELHOSTFILE:
		case FCTL_GETPANELFORMAT:
		{
			string strTemp;

			if (GetType() == panel_type::FILE_PANEL)
			{
				const auto DestFilePanel = static_cast<const FileList*>(this);
				static int Reenter=0;

				if (!Reenter && GetMode() == panel_mode::PLUGIN_PANEL)
				{
					Reenter++;

					OpenPanelInfo PInfo;
					DestFilePanel->GetOpenPanelInfo(&PInfo);

					switch (Command)
					{
						case FCTL_GETPANELHOSTFILE:
							strTemp=NullToEmpty(PInfo.HostFile);
							break;
						case FCTL_GETPANELFORMAT:
							strTemp=NullToEmpty(PInfo.Format);
							break;
					}

					Reenter--;
				}
			}

			if (Param1&&Param2)
				xwcsncpy(static_cast<wchar_t*>(Param2), strTemp.c_str(), Param1);

			Result = static_cast<int>(strTemp.size()) + 1;
			break;
		}
		case FCTL_GETPANELDIRECTORY:
		{
			static int Reenter=0;
			if(!Reenter)
			{
				Reenter++;
				ShortcutInfo Info;
				GetShortcutInfo(Info);
				Result = static_cast<int>(aligned_sizeof<FarPanelDirectory>());
				const auto folderOffset = Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.ShortcutFolder.size()+1));
				const auto pluginFileOffset = Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.PluginFile.size()+1));
				const auto pluginDataOffset = Result;
				Result+=static_cast<int>(sizeof(wchar_t)*(Info.PluginData.size()+1));
				const auto dirInfo = static_cast<FarPanelDirectory*>(Param2);
				if(Param1>=Result && CheckStructSize(dirInfo))
				{
					dirInfo->StructSize=sizeof(FarPanelDirectory);
					dirInfo->PluginId=Info.PluginUuid;
					dirInfo->Name = static_cast<wchar_t*>(static_cast<void*>(static_cast<char*>(Param2) + folderOffset));
					dirInfo->Param = static_cast<wchar_t*>(static_cast<void*>(static_cast<char*>(Param2) + pluginDataOffset));
					dirInfo->File = static_cast<wchar_t*>(static_cast<void*>(static_cast<char*>(Param2) + pluginFileOffset));
					*copy_string(Info.ShortcutFolder, const_cast<wchar_t*>(dirInfo->Name)) = {};
					*copy_string(Info.PluginData, const_cast<wchar_t*>(dirInfo->Param)) = {};
					*copy_string(Info.PluginFile, const_cast<wchar_t*>(dirInfo->File)) = {};
				}
				Reenter--;
			}
			break;
		}

		case FCTL_GETCOLUMNTYPES:
		case FCTL_GETCOLUMNWIDTHS:

			if (GetType() == panel_type::FILE_PANEL)
			{
				string strColumnTypes,strColumnWidths;
				static_cast<FileList*>(this)->PluginGetColumnTypesAndWidths(strColumnTypes, strColumnWidths);

				if (Command==FCTL_GETCOLUMNTYPES)
				{
					if (Param1&&Param2)
						xwcsncpy(static_cast<wchar_t*>(Param2), strColumnTypes.c_str(), Param1);

					Result = static_cast<int>(strColumnTypes.size()) + 1;
				}
				else
				{
					if (Param1&&Param2)
						xwcsncpy(static_cast<wchar_t*>(Param2), strColumnWidths.c_str(), Param1);

					Result = static_cast<int>(strColumnWidths.size()) + 1;
				}
			}
			break;

		case FCTL_GETPANELITEM:
		{
			if (GetType() == panel_type::FILE_PANEL && CheckNullOrStructSize(static_cast<FarGetPluginPanelItem*>(Param2)))
				Result = static_cast<int>(static_cast<FileList*>(this)->PluginGetPanelItem(Param1, static_cast<FarGetPluginPanelItem*>(Param2)));
			break;
		}

		case FCTL_GETSELECTEDPANELITEM:
		{
			if (GetType() == panel_type::FILE_PANEL && CheckNullOrStructSize(static_cast<FarGetPluginPanelItem*>(Param2)))
				Result = static_cast<int>(static_cast<FileList*>(this)->PluginGetSelectedPanelItem(Param1, static_cast<FarGetPluginPanelItem*>(Param2)));
			break;
		}

		case FCTL_GETCURRENTPANELITEM:
		{
			if (GetType() == panel_type::FILE_PANEL && CheckNullOrStructSize(static_cast<FarGetPluginPanelItem*>(Param2)))
			{
				PanelInfo Info;
				const auto DestPanel = static_cast<FileList*>(this);
				DestPanel->PluginGetPanelInfo(Info);
				Result = static_cast<int>(DestPanel->PluginGetPanelItem(static_cast<int>(Info.CurrentItem), static_cast<FarGetPluginPanelItem*>(Param2)));
			}
			break;
		}

		case FCTL_BEGINSELECTION:
		{
			if (GetType() == panel_type::FILE_PANEL)
			{
				static_cast<FileList*>(this)->PluginBeginSelection();
				Result=TRUE;
			}
			break;
		}

		case FCTL_SETSELECTION:
		{
			if (GetType() == panel_type::FILE_PANEL)
			{
				static_cast<FileList*>(this)->PluginSetSelection(Param1, Param2 != nullptr);
				Result=TRUE;
			}
			break;
		}

		case FCTL_CLEARSELECTION:
		{
			if (GetType() == panel_type::FILE_PANEL)
			{
				static_cast<FileList*>(this)->PluginClearSelection(Param1);
				Result=TRUE;
			}
			break;
		}

		case FCTL_ENDSELECTION:
		{
			if (GetType() == panel_type::FILE_PANEL)
			{
				static_cast<FileList*>(this)->PluginEndSelection();
				Result=TRUE;
			}
			break;
		}

		case FCTL_UPDATEPANEL:
			Update(Param1?UPDATE_KEEP_SELECTION:0);

			if (GetType() == panel_type::QVIEW_PANEL)
				UpdateViewPanel();

			Result=TRUE;
			break;

		case FCTL_REDRAWPANEL:
		{
			const auto Info = static_cast<const PanelRedrawInfo*>(Param2);

			if (CheckStructSize(Info))
			{
				m_CurFile=static_cast<int>(Info->CurrentItem);
				m_CurTopFile=static_cast<int>(Info->TopPanelItem);
			}

			// $ 12.05.2001 DJ перерисовываемся только в том случае, если мы - текущее окно
			if (Parent()->IsTopWindow())
				Redraw();

			Result=TRUE;
			break;
		}

		case FCTL_SETPANELDIRECTORY:
		{
			const auto dirInfo = static_cast<const FarPanelDirectory*>(Param2);
			if (CheckStructSize(dirInfo))
			{
				Result = ExecFolder(NullToEmpty(dirInfo->Name), dirInfo->PluginId, NullToEmpty(dirInfo->File), NullToEmpty(dirInfo->Param), false, false, true);
				// restore current directory to active panel path
				if (!IsFocused())
				{
					Parent()->ActivePanel()->SetCurPath();
				}
			}
			break;
		}

		case FCTL_SETACTIVEPANEL:
		{
			if (IsVisible())
			{
				Parent()->SetActivePanel(this);
				Result=TRUE;
			}
			break;
		}
	}

	ProcessingPluginCommand--;
	return Result;
}


bool Panel::GetCurName(string &strName, string &strShortName) const
{
	return false;
}


bool Panel::GetCurBaseName(string &strName, string &strShortName) const
{
	return false;
}

bool Panel::NeedUpdatePanel(const Panel *AnotherPanel) const
{
	/* Обновить, если обновление разрешено и пути совпадают */
	return (!Global->Opt->AutoUpdateLimit || static_cast<unsigned>(GetFileCount()) <= static_cast<unsigned>(Global->Opt->AutoUpdateLimit)) && equal_icase(AnotherPanel->m_CurDir, m_CurDir);
}

bool Panel::GetShortcutInfo(ShortcutInfo& Info) const
{
	bool result=true;
	if (m_PanelMode == panel_mode::PLUGIN_PANEL)
	{
		const auto ph = GetPluginHandle();
		Info.PluginUuid = ph->plugin()->Id();
		OpenPanelInfo OpInfo;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(ph, &OpInfo);
		Info.PluginFile = NullToEmpty(OpInfo.HostFile);
		Info.ShortcutFolder = NullToEmpty(OpInfo.CurDir);
		Info.PluginData = NullToEmpty(OpInfo.ShortcutData);
		if(!(OpInfo.Flags&OPIF_SHORTCUT))
			result = false;
	}
	else
	{
		Info.PluginUuid = FarUuid;
		Info.PluginFile.clear();
		Info.PluginData.clear();
		Info.ShortcutFolder = m_CurDir;
	}
	return result;
}

bool Panel::SaveShortcutFolder(int Pos) const
{
	ShortcutInfo Info;
	if(GetShortcutInfo(Info))
	{
		Shortcuts(Pos).Add(Info.ShortcutFolder, Info.PluginUuid, Info.PluginFile, Info.PluginData);
		return true;
	}
	return false;
}

/*
int Panel::ProcessShortcutFolder(int Key,bool ProcTreePanel)
{
	string strShortcutFolder, strPluginModule, strPluginFile, strPluginData;

	if (GetShortcutFolder(Key-KEY_RCTRL0,&strShortcutFolder,&strPluginModule,&strPluginFile,&strPluginData))
	{
		const auto AnotherPanel = Parent()->GetAnotherPanel(this);

		if (ProcTreePanel)
		{
			if (AnotherPanel->GetType()==FILE_PANEL)
			{
				AnotherPanel->SetCurDir(strShortcutFolder,true);
				AnotherPanel->Redraw();
			}
			else
			{
				SetCurDir(strShortcutFolder,true);
				ProcessKey(KEY_ENTER);
			}
		}
		else
		{
			if (AnotherPanel->GetType()==FILE_PANEL && !strPluginModule.empty())
			{
				AnotherPanel->SetCurDir(strShortcutFolder,true);
				AnotherPanel->Redraw();
			}
		}

		return true;
	}

	return false;
}
*/

bool Panel::SetPluginDirectory(string_view const Directory, bool Silent)
{
	UserDataItem UserData = {}; //????
	const auto Result = Global->CtrlObject->Plugins->SetDirectory(GetPluginHandle(), string(Directory), Silent?OPM_SILENT:0, &UserData) != 0;
	Update(0);
	Show();
	return Result;
}

bool Panel::ExecShortcutFolder(int Pos)
{
	Shortcuts::data Data;
	return Shortcuts(Pos).Get(Data) && ExecFolder(std::move(Data.Folder), Data.PluginUuid, Data.PluginFile, Data.PluginData, true, true, false);
}

bool Panel::ExecFolder(string_view const Folder, const UUID& PluginUuid, const string& strPluginFile, const string& strPluginData, bool CheckType, bool TryClosest, bool Silent)
{
	auto SrcPanel = shared_from_this();
	const auto AnotherPanel = Parent()->GetAnotherPanel(this);

	if(CheckType)
	{
		switch (GetType())
		{
		case panel_type::FILE_PANEL:
			break;

		case panel_type::TREE_PANEL:
		case panel_type::QVIEW_PANEL:
		case panel_type::INFO_PANEL:
			if (AnotherPanel->GetType() == panel_type::FILE_PANEL)
				SrcPanel=AnotherPanel;
			break;
		}
	}

	const auto CheckFullScreen = SrcPanel->IsFullScreen();

	if (PluginUuid != FarUuid)
	{
		bool Result = false;
		ShortcutInfo Info;
		GetShortcutInfo(Info);
		if (Info.PluginUuid == PluginUuid && Info.PluginFile == strPluginFile && Info.PluginData == strPluginData)
		{
			Result = SetPluginDirectory(Folder, Silent);
		}
		else
		{
			if (ProcessPluginEvent(FE_CLOSE, nullptr))
			{
				return false;
			}

			if (const auto pPlugin = Global->CtrlObject->Plugins->FindPlugin(PluginUuid))
			{
				if (pPlugin->has(iOpen))
				{
					if (!strPluginFile.empty())
					{
						string_view RealDir = strPluginFile;
						if (CutToSlash(RealDir))
						{
							SrcPanel->SetCurDir(RealDir, true);
							SrcPanel->GoToFile(PointToName(strPluginFile));

							SrcPanel->ClearAllItem();
						}
					}

					const auto IsActive = SrcPanel->IsFocused();
					OpenShortcutInfo info=
					{
						sizeof(OpenShortcutInfo),
						EmptyToNull(strPluginFile),
						EmptyToNull(strPluginData),
						IsActive? FOSF_ACTIVE : FOSF_NONE
					};

					if (auto hNewPlugin = Global->CtrlObject->Plugins->Open(pPlugin, OPEN_SHORTCUT, FarUuid, reinterpret_cast<intptr_t>(&info)))
					{
						const auto NewPanel = Parent()->ChangePanel(SrcPanel, panel_type::FILE_PANEL, TRUE, TRUE);
						NewPanel->SetPluginMode(std::move(hNewPlugin), {}, IsActive || !Parent()->GetAnotherPanel(NewPanel)->IsVisible());
						Result = NewPanel->SetPluginDirectory(Folder, Silent);
					}
				}
			}
		}
		return Result;
	}

	auto ExpandedFolder = os::env::expand(Folder);

	if ((TryClosest && !CheckShortcutFolder(ExpandedFolder, TryClosest, Silent)) || ProcessPluginEvent(FE_CLOSE, nullptr))
	{
		return false;
	}

	if (!SrcPanel->SetCurDir(ExpandedFolder, true, true, Silent))
		return false;

	if (CheckFullScreen!=SrcPanel->IsFullScreen())
		Parent()->GetAnotherPanel(SrcPanel)->Show();

	SrcPanel->Refresh();
	return true;
}

string Panel::CreateFullPathName(string_view const Name, bool const Directory, bool const UNC, bool const ShortNameAsIs) const
{
	auto FullName = FindSlash(Name) == string::npos? ConvertNameToFull(Name) : string(Name);

	if (m_ShowShortNames && ShortNameAsIs)
		FullName = ConvertNameToShort(FullName);

	/* $ 29.01.2001 VVM
	  + По CTRL+ALT+F в командную строку сбрасывается UNC-имя текущего файла. */
	if (UNC)
		FullName = ConvertNameToUNC(FullName);

	// $ 20.10.2000 SVS Сделаем фичу Ctrl-F опциональной!
	if (Global->Opt->PanelCtrlFRule)
	{
		/* $ 13.10.2000 tran
		  по Ctrl-f имя должно отвечать условиям на панели */
		if (m_ViewSettings.Flags&PVS_FOLDERUPPERCASE)
		{
			if (Directory)
			{
				inplace::upper(FullName);
			}
			else
			{
				inplace::upper(FullName, 0, FindLastSlash(FullName));
			}
		}

		if ((m_ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE) && !Directory)
		{
			const auto pos = FindLastSlash(FullName);
			if (pos != string::npos && !IsCaseMixed(string_view(FullName).substr(pos)))
			{
				inplace::lower(FullName, pos);
			}
		}

		if ((m_ViewSettings.Flags&PVS_FILELOWERCASE) && !Directory)
		{
			const auto pos = FindLastSlash(FullName);
			if (pos != string::npos)
			{
				inplace::lower(FullName, pos);
			}
		}
	}

	return FullName;
}

FilePanels* Panel::Parent() const
{
	return dynamic_cast<FilePanels*>(GetOwner().get());
}

const auto PluginSortModesOffset = 1;

int internal_sort_mode_to_plugin(panel_sort const Mode)
{
	const auto ModeValue = static_cast<int>(Mode);
	return Mode < panel_sort::BY_USER? ModeValue + PluginSortModesOffset : ModeValue;
}

panel_sort plugin_sort_mode_to_internal(int const Mode)
{
	return panel_sort{ Mode < SM_USER? Mode - PluginSortModesOffset : Mode };
}
