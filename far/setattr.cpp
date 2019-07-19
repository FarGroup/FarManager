/*
setattr.cpp

Установка атрибутов файлов
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

// Self:
#include "setattr.hpp"

// Internal:
#include "flink.hpp"
#include "dialog.hpp"
#include "chgprior.hpp"
#include "scantree.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "ctrlobj.hpp"
#include "constitle.hpp"
#include "TPreRedrawFunc.hpp"
#include "taskbar.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "datetime.hpp"
#include "fileattr.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "network.hpp"
#include "fileowner.hpp"
#include "wakeful.hpp"
#include "DlgGuid.hpp"
#include "interf.hpp"
#include "plugins.hpp"
#include "imports.hpp"
#include "lang.hpp"
#include "locale.hpp"
#include "string_utils.hpp"
#include "global.hpp"
#include "stddlg.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/view/zip.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

enum SETATTRDLG
{
	SA_DOUBLEBOX,
	SA_TEXT_LABEL,
	SA_TEXT_NAME,
	SA_COMBO_HARDLINK,
	SA_TEXT_SYMLINK,
	SA_EDIT_SYMLINK,
	SA_COMBO_SYMLINK,
	SA_SEPARATOR1,
	SA_ATTR_FIRST,
	SA_CHECKBOX_RO=SA_ATTR_FIRST,
	SA_CHECKBOX_ARCHIVE,
	SA_CHECKBOX_HIDDEN,
	SA_CHECKBOX_SYSTEM,
	SA_CHECKBOX_COMPRESSED,
	SA_CHECKBOX_ENCRYPTED,
	SA_CHECKBOX_NOTINDEXED,
	SA_CHECKBOX_SPARSE,
	SA_CHECKBOX_TEMP,
	SA_CHECKBOX_OFFLINE,
	SA_CHECKBOX_REPARSEPOINT,
	SA_CHECKBOX_VIRTUAL,
	SA_CHECKBOX_INTEGRITY_STREAM,
	SA_CHECKBOX_NO_SCRUB_DATA,
	SA_CHECKBOX_PINNED,
	SA_CHECKBOX_UNPINNED,
	SA_ATTR_LAST = SA_CHECKBOX_UNPINNED,
	SA_SEPARATOR2,
	SA_TEXT_TITLEDATE,
	SA_TEXT_TITLETIME,
	SA_TEXT_LASTWRITE,
	SA_EDIT_WDATE,
	SA_EDIT_WTIME,
	SA_TEXT_CREATION,
	SA_EDIT_CDATE,
	SA_EDIT_CTIME,
	SA_TEXT_LASTACCESS,
	SA_EDIT_ADATE,
	SA_EDIT_ATIME,
	SA_TEXT_CHANGE,
	SA_EDIT_XDATE,
	SA_EDIT_XTIME,
	SA_BUTTON_ORIGINAL,
	SA_BUTTON_CURRENT,
	SA_BUTTON_BLANK,
	SA_SEPARATOR3,
	SA_TEXT_OWNER,
	SA_EDIT_OWNER,
	SA_SEPARATOR4,
	SA_CHECKBOX_SUBFOLDERS,
	SA_SEPARATOR5,
	SA_BUTTON_SET,
	SA_BUTTON_SYSTEMDLG,
	SA_BUTTON_CANCEL,
};

enum DIALOGMODE
{
	MODE_FILE,
	MODE_FOLDER,
	MODE_MULTIPLE,
};

static const struct
{
	SETATTRDLG Id;
	DWORD Attribute;
}
AttributeMap[]
{
	{ SA_CHECKBOX_RO,                           FILE_ATTRIBUTE_READONLY,              },
	{ SA_CHECKBOX_ARCHIVE,                      FILE_ATTRIBUTE_ARCHIVE,               },
	{ SA_CHECKBOX_HIDDEN,                       FILE_ATTRIBUTE_HIDDEN,                },
	{ SA_CHECKBOX_SYSTEM,                       FILE_ATTRIBUTE_SYSTEM,                },
	{ SA_CHECKBOX_COMPRESSED,                   FILE_ATTRIBUTE_COMPRESSED,            },
	{ SA_CHECKBOX_ENCRYPTED,                    FILE_ATTRIBUTE_ENCRYPTED,             },
	{ SA_CHECKBOX_NOTINDEXED,                   FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,   },
	{ SA_CHECKBOX_SPARSE,                       FILE_ATTRIBUTE_SPARSE_FILE,           },
	{ SA_CHECKBOX_TEMP,                         FILE_ATTRIBUTE_TEMPORARY,             },
	{ SA_CHECKBOX_OFFLINE,                      FILE_ATTRIBUTE_OFFLINE,               },
	{ SA_CHECKBOX_REPARSEPOINT,                 FILE_ATTRIBUTE_REPARSE_POINT,         },
	{ SA_CHECKBOX_VIRTUAL,                      FILE_ATTRIBUTE_VIRTUAL,               },
	{ SA_CHECKBOX_INTEGRITY_STREAM,             FILE_ATTRIBUTE_INTEGRITY_STREAM,      },
	{ SA_CHECKBOX_NO_SCRUB_DATA,                FILE_ATTRIBUTE_NO_SCRUB_DATA,         },
	{ SA_CHECKBOX_PINNED,                       FILE_ATTRIBUTE_PINNED,                },
	{ SA_CHECKBOX_UNPINNED,                     FILE_ATTRIBUTE_UNPINNED,              },
};

static const struct time_map
{
	SETATTRDLG DateId;
	SETATTRDLG TimeId;
	os::chrono::time_point os::fs::find_data::* Accessor;
}
TimeMap[]
{
	{SA_EDIT_WDATE, SA_EDIT_WTIME, &os::fs::find_data::LastWriteTime  },
	{SA_EDIT_CDATE, SA_EDIT_CTIME, &os::fs::find_data::CreationTime   },
	{SA_EDIT_ADATE, SA_EDIT_ATIME, &os::fs::find_data::LastAccessTime },
	{SA_EDIT_XDATE, SA_EDIT_XTIME, &os::fs::find_data::ChangeTime     },
};

static int label_to_time_map_index(int Id)
{
	static_assert(std::size(TimeMap) == 4);

	switch (Id)
	{
	case SA_TEXT_LASTWRITE:    return 0;
	case SA_TEXT_CREATION:     return 1;
	case SA_TEXT_LASTACCESS:   return 2;
	case SA_TEXT_CHANGE:       return 3;
	default: UNREACHABLE;
	}
}

struct SetAttrDlgParam
{
	bool Plugin;
	DIALOGMODE DialogMode;
	string strSelName;
	string strOwner;
	bool OwnerChanged;
	// значения CheckBox`ов на момент старта диалога
	struct cb_data
	{
		FARDIALOGITEMFLAGS Flags;
		int Value;
		bool Changed;
	}
	cb[std::size(AttributeMap)];

	FARCHECKEDSTATE OSubfoldersState;
};

static void set_dates_and_times(Dialog* const Dlg, const time_map& TimeMapEntry, std::optional<os::chrono::time_point> const TimePoint, bool const MakeUnchanged)
{
	string Date, Time;

	if (TimePoint)
	{
		ConvertDate(*TimePoint, Date, Time, 12, 2);
	}

	const auto SetText = [&](int const Id, string const& Str)
	{
		Dlg->SendMessage(DM_SETTEXTPTR, Id, UNSAFE_CSTR(Str));
		Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, Id, ToPtr(MakeUnchanged));
	};

	SetText(TimeMapEntry.DateId, Date);
	SetText(TimeMapEntry.TimeId, Time);
}

static intptr_t SetAttrDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	const auto DlgParam = reinterpret_cast<SetAttrDlgParam*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

	switch (Msg)
	{
	case DN_BTNCLICK:
		if (Param1 >= SA_ATTR_FIRST && Param1 <= SA_ATTR_LAST)
		{
			DlgParam->cb[Param1 - SA_ATTR_FIRST].Value = static_cast<FARCHECKEDSTATE>(reinterpret_cast<intptr_t>(Param2));
			DlgParam->cb[Param1 - SA_ATTR_FIRST].Changed = true;

			// Compressed / Encrypted are mutually exclusive
			if ((Param1 == SA_CHECKBOX_COMPRESSED || Param1 == SA_CHECKBOX_ENCRYPTED) &&
				static_cast<FARCHECKEDSTATE>(reinterpret_cast<intptr_t>(Param2)) == BSTATE_CHECKED)
			{
				const auto OtherId = Param1 == SA_CHECKBOX_COMPRESSED? SA_CHECKBOX_ENCRYPTED : SA_CHECKBOX_COMPRESSED;

				if (static_cast<FARCHECKEDSTATE>(Dlg->SendMessage(DM_GETCHECK, OtherId, nullptr)) != BSTATE_UNCHECKED)
					Dlg->SendMessage(DM_SETCHECK, OtherId, ToPtr(BSTATE_UNCHECKED));
			}

			// only remove or keep, not set
			if (Param1 == SA_CHECKBOX_REPARSEPOINT && reinterpret_cast<intptr_t>(Param2) == BSTATE_CHECKED)
			{
				return FALSE;
			}
		}
		else if (Param1 == SA_CHECKBOX_SUBFOLDERS)
		{
			// этот кусок всегда работает если есть хотя бы одна папка
			// иначе SA_CHECKBOX_SUBFOLDERS недоступен и всегда снят.
			const auto SubfoldersState = static_cast<FARCHECKEDSTATE>(reinterpret_cast<intptr_t>(Param2));
			if (SubfoldersState == DlgParam->OSubfoldersState)
				break;

			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

			// Works in 2 modes: single directory or multiple selection
			for (int i = SA_ATTR_FIRST; i <= SA_ATTR_LAST; ++i)
			{
				Dlg->SendMessage(DM_SET3STATE, i, ToPtr(
					SubfoldersState? true :
					DlgParam->DialogMode == MODE_FOLDER?
						false :
						(DlgParam->cb[i - SA_ATTR_FIRST].Flags & DIF_3STATE) != 0));

				if (!SubfoldersState || !DlgParam->cb[i - SA_ATTR_FIRST].Changed)
					Dlg->SendMessage(DM_SETCHECK, i, SubfoldersState? ToPtr(BSTATE_3STATE) : ToPtr(DlgParam->cb[i - SA_ATTR_FIRST].Value));
			}

			if (!DlgParam->OwnerChanged)
			{
				Dlg->SendMessage(DM_SETTEXTPTR, SA_EDIT_OWNER, SubfoldersState? nullptr : UNSAFE_CSTR(DlgParam->strOwner));
			}

			if (DlgParam->DialogMode == MODE_FOLDER)
			{
				os::fs::find_data FindData;
				if (os::fs::get_find_data(DlgParam->strSelName, FindData))
				{
					for (const auto& i: TimeMap)
					{
						std::optional<os::chrono::time_point> TimePoint;
						if (!SubfoldersState)
								TimePoint = std::invoke(i.Accessor, FindData);

						set_dates_and_times(Dlg, i, TimePoint, true);
					}
				}
			}

			if (const auto Owner = reinterpret_cast<LPCWSTR>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, SA_EDIT_OWNER, nullptr)); *Owner)
			{
				std::optional<bool> Changed;

				if (!DlgParam->OwnerChanged)
				{
					Changed = !equal_icase(Owner, DlgParam->strOwner);

					if (*Changed)
					{
						DlgParam->OwnerChanged = true;
					}
				}

				if (Changed && *Changed)
				{
					DlgParam->strOwner = Owner;
				}
			}

			DlgParam->OSubfoldersState = SubfoldersState;
		}
		// Set Original? / Set All? / Clear All?
		else if (Param1 == SA_BUTTON_ORIGINAL)
		{
			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

			os::fs::find_data FindData;

			if (os::fs::get_find_data(DlgParam->strSelName, FindData))
			{
				for (const auto& i: TimeMap)
				{
					set_dates_and_times(Dlg, i, std::invoke(i.Accessor, FindData), true);
				}
			}

			Dlg->SendMessage(DM_SETFOCUS, SA_EDIT_WDATE, nullptr);
		}
		else if (Param1 == SA_BUTTON_CURRENT || Param1 == SA_BUTTON_BLANK)
		{
			std::optional<os::chrono::time_point> Time;
			if(Param1 == SA_BUTTON_CURRENT)
			{
				Time = os::chrono::nt_clock::now();
			}

			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

			for (const auto& i: TimeMap)
			{
				set_dates_and_times(Dlg, i, Time, false);
			}

			Dlg->SendMessage(DM_SETFOCUS, SA_EDIT_WDATE, nullptr);
		}

		break;

	//BUGBUG: DefDlgProc вызывается дважды, второй раз Param1 может быть другим.
	case DN_CONTROLINPUT:
		{
			if (Param1 != SA_TEXT_LASTWRITE && Param1 != SA_TEXT_CREATION && Param1 != SA_TEXT_LASTACCESS && Param1 != SA_TEXT_CHANGE)
				break;

			const auto Record = static_cast<const INPUT_RECORD*>(Param2);
			if (Record->EventType != MOUSE_EVENT)
				break;

			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

			if (Record->Event.MouseEvent.dwEventFlags == DOUBLE_CLICK)
				set_dates_and_times(Dlg, TimeMap[label_to_time_map_index(Param1)], os::chrono::nt_clock::now(), false);
			else
				Dlg->SendMessage(DM_SETFOCUS, Param1 + 1, nullptr);
		}
		break;

	case DN_EDITCHANGE:
		{
			if (Param1 != SA_COMBO_HARDLINK && Param1 != SA_COMBO_SYMLINK)
				break;

			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

			FarListInfo li{sizeof(FarListInfo)};
			Dlg->SendMessage(DM_LISTINFO, Param1, &li);
			const auto m = Param1 == SA_COMBO_HARDLINK? lng::MSetAttrHardLinks : lng::MSetAttrDfsTargets;
			Dlg->SendMessage(DM_SETTEXTPTR, Param1, UNSAFE_CSTR(concat(msg(m), L" ("sv, str(li.ItemsNumber), L')')));
		}
		break;

	case DN_GOTFOCUS:
		{
			if (!std::any_of(ALL_CONST_RANGE(TimeMap), [&](const auto& i) { return i.DateId == Param1; }))
				break;

			if (locale.date_format() != 2)
				break;

			if (reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, Param1, nullptr))[0] != L' ')
				break;

			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

			COORD Pos;
			Dlg->SendMessage(DM_GETCURSORPOS, Param1, &Pos);

			if (Pos.X != 0)
				break;

			Pos.X = 1;
			Dlg->SendMessage(DM_SETCURSORPOS, Param1, &Pos);
		}
		break;

	default:
		break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

static void PR_ShellSetFileAttributesMsg();

struct AttrPreRedrawItem : public PreRedrawItem
{
	AttrPreRedrawItem() : PreRedrawItem(PR_ShellSetFileAttributesMsg){}

	string Name;
};

static void ShellSetFileAttributesMsgImpl(const string& Name)
{
	static int Width=54;
	int WidthTemp;

	if (!Name.empty())
		WidthTemp=std::max(static_cast<int>(Name.size()), 54);
	else
		Width=WidthTemp=54;

	WidthTemp=std::min(WidthTemp, ScrX/2);
	Width=std::max(Width,WidthTemp);

	Message(0,
		msg(lng::MSetAttrTitle),
		{
			msg(lng::MSetAttrSetting),
			fit_to_center(truncate_path(Name, Width), Width + 4)
		},
		{});
}

static void ShellSetFileAttributesMsg(const string& Name)
{
	ShellSetFileAttributesMsgImpl(Name);

	TPreRedrawFunc::instance()([&](AttrPreRedrawItem& Item)
	{
		Item.Name = Name;
	});
}

static void PR_ShellSetFileAttributesMsg()
{
	TPreRedrawFunc::instance()([](const AttrPreRedrawItem& Item)
	{
		ShellSetFileAttributesMsgImpl(Item.Name);
	});
}

static bool construct_time(
	os::chrono::time_point const OriginalFileTime,
	os::chrono::time_point& FileTime,
	const string& OSrcDate,
	const date_ranges& DateRanges,
	const string& OSrcTime,
	const time_ranges& TimeRanges)
{
	SYSTEMTIME ost;
	if (!utc_to_local(OriginalFileTime, ost))
		return false;

	WORD DateN[3];
	ParseDateComponents(OSrcDate, DateRanges, DateN);
	WORD TimeN[4];
	ParseDateComponents(OSrcTime, TimeRanges, TimeN);

	enum indices { i_day, i_month, i_year };

	const auto Indices = []() -> std::array<indices, 3>
	{
		switch (locale.date_format())
		{
		case 0:  return { i_month, i_day, i_year };
		case 1:  return { i_day, i_month, i_year };
		default: return { i_year, i_month, i_day };
		}
	}();

	SYSTEMTIME st{};

	const auto set_or_inherit = [&](WORD SYSTEMTIME::* const Field, WORD const New)
	{
		std::invoke(Field, st) = New != date_none? New : std::invoke(Field, ost);
	};

	set_or_inherit(&SYSTEMTIME::wDay,          DateN[Indices[0]]);
	set_or_inherit(&SYSTEMTIME::wMonth,        DateN[Indices[1]]);
	set_or_inherit(&SYSTEMTIME::wYear,         DateN[Indices[2]]);
	set_or_inherit(&SYSTEMTIME::wHour,         TimeN[0]);
	set_or_inherit(&SYSTEMTIME::wMinute,       TimeN[1]);
	set_or_inherit(&SYSTEMTIME::wSecond,       TimeN[2]);
	set_or_inherit(&SYSTEMTIME::wMilliseconds, TimeN[3]);

	if (st.wYear < 100)
	{
		st.wYear = static_cast<WORD>(ConvertYearToFull(st.wYear));
	}

	return local_to_utc(st, FileTime);
}

struct state
{
	string const& Owner;
	os::fs::find_data const& FindData;
};

static bool process_single_file(
	string const& Name,
	state const& Current,
	state const& New,
	function_ref<const string&(int)> const DateTimeAccessor,
	date_ranges const& DateRanges,
	time_ranges const& TimeRanges,
	bool& SkipErrors)
{
	if (!New.Owner.empty() && !equal_icase(Current.Owner, New.Owner))
	{
		if (ESetFileOwner(Name, New.Owner, SkipErrors) == setattr_result::cancel)
			return false;
	}

	{
		os::chrono::time_point WriteTime, CreationTime, AccessTime, ChangeTime;
		std::array TimePointers{ &WriteTime, &CreationTime, &AccessTime, &ChangeTime };

		for (const auto& [i, TimePointer] : zip(TimeMap, TimePointers))
		{
			const auto OriginalTime = std::invoke(i.Accessor, Current.FindData);
			if (!construct_time(OriginalTime, *TimePointer, DateTimeAccessor(i.DateId), DateRanges, DateTimeAccessor(i.TimeId), TimeRanges)
				|| OriginalTime == *TimePointer)
			{
				TimePointer = {};
			}
		}

		if (ESetFileTime(Name, &WriteTime, &CreationTime, &AccessTime, &ChangeTime, Current.FindData.Attributes, SkipErrors) == setattr_result::cancel)
			return false;
	}

	if (New.FindData.Attributes == Current.FindData.Attributes)
		return true;

	if (ESetFileCompression(Name, (New.FindData.Attributes & FILE_ATTRIBUTE_COMPRESSED) != 0, Current.FindData.Attributes, SkipErrors) == setattr_result::cancel)
		return false;

	if (ESetFileEncryption(Name, (New.FindData.Attributes & FILE_ATTRIBUTE_ENCRYPTED) != 0, Current.FindData.Attributes, SkipErrors) == setattr_result::cancel)
		return false;

	if (ESetFileSparse(Name, (New.FindData.Attributes & FILE_ATTRIBUTE_SPARSE_FILE) != 0, Current.FindData.Attributes, SkipErrors) == setattr_result::cancel)
		return false;

	const auto IsChanged = [&](DWORD const Attributes)
	{
		return (New.FindData.Attributes & Attributes) != (Current.FindData.Attributes & Attributes);
	};

	if (IsChanged(FILE_ATTRIBUTE_REPARSE_POINT))
	{
		if (EDeleteReparsePoint(Name, Current.FindData.Attributes, SkipErrors) == setattr_result::cancel)
			return false;
	}

	const auto OtherAttributes = ~(FILE_ATTRIBUTE_ENCRYPTED | FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_REPARSE_POINT);
	if (IsChanged(OtherAttributes))
	{
		if (ESetFileAttributes(Name, New.FindData.Attributes & OtherAttributes, SkipErrors) == setattr_result::cancel)
			return false;
	}

	return true;
}

bool ShellSetFileAttributes(Panel *SrcPanel, const string* Object)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	short DlgX=74,DlgY=26;

	const auto C1 = 5;
	const auto C2 = C1 + (DlgX - 10) / 2;
	const auto AR = 12;

	auto AttrDlg = MakeDialogItems(
	{
		{ DI_DOUBLEBOX, {{ 3,      1     }, {DlgX-4,  DlgY-2}}, DIF_NONE, msg(lng::MSetAttrTitle), },
		{ DI_TEXT,      {{-1,      2     }, {0,       2     }}, DIF_NONE, msg(lng::MSetAttrFor), },
		{ DI_TEXT,      {{-1,      3     }, {0,       3     }}, DIF_SHOWAMPERSAND, },
		{ DI_COMBOBOX,  {{5,       3     }, {DlgX-6,  3     }}, DIF_SHOWAMPERSAND | DIF_DROPDOWNLIST | DIF_LISTWRAPMODE | DIF_HIDDEN, },
		{ DI_TEXT,      {{5,       3     }, {17,      3     }}, DIF_HIDDEN, },
		{ DI_EDIT,      {{18,      3     }, {DlgX-6,  3     }}, DIF_HIDDEN | DIF_EDITPATH, },
		{ DI_COMBOBOX,  {{18,      3     }, {DlgX-6,  3     }}, DIF_SHOWAMPERSAND | DIF_DROPDOWNLIST | DIF_LISTWRAPMODE | DIF_HIDDEN, },
		{ DI_TEXT,      {{-1,      4     }, {0,       4     }}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,  {{C1,      5     }, {0,       5     }}, DIF_FOCUS | DIF_3STATE, msg(lng::MSetAttrRO), },
		{ DI_CHECKBOX,  {{C1,      6     }, {0,       6     }}, DIF_3STATE, msg(lng::MSetAttrArchive), },
		{ DI_CHECKBOX,  {{C1,      7     }, {0,       7     }}, DIF_3STATE, msg(lng::MSetAttrHidden), },
		{ DI_CHECKBOX,  {{C1,      8     }, {0,       8     }}, DIF_3STATE, msg(lng::MSetAttrSystem), },
		{ DI_CHECKBOX,  {{C1,      9     }, {0,       9     }}, DIF_3STATE, msg(lng::MSetAttrCompressed), },
		{ DI_CHECKBOX,  {{C1,      10    }, {0,       10    }}, DIF_3STATE, msg(lng::MSetAttrEncrypted), },
		{ DI_CHECKBOX,  {{C1,      11    }, {0,       11    }}, DIF_3STATE, msg(lng::MSetAttrNotIndexed), },
		{ DI_CHECKBOX,  {{C1,      12    }, {0,       12    }}, DIF_3STATE, msg(lng::MSetAttrSparse), },
		{ DI_CHECKBOX,  {{C2,      5     }, {0,       5     }}, DIF_3STATE, msg(lng::MSetAttrTemp), },
		{ DI_CHECKBOX,  {{C2,      6     }, {0,       6     }}, DIF_3STATE, msg(lng::MSetAttrOffline), },
		{ DI_CHECKBOX,  {{C2,      7     }, {0,       7     }}, DIF_3STATE, msg(lng::MSetAttrReparsePoint), },
		{ DI_CHECKBOX,  {{C2,      8     }, {0,       8     }}, DIF_3STATE | DIF_DISABLE, msg(lng::MSetAttrVirtual), },
		{ DI_CHECKBOX,  {{C2,      9     }, {0,       9     }}, DIF_3STATE, msg(lng::MSetAttrIntegrityStream), },
		{ DI_CHECKBOX,  {{C2,      10    }, {0,       10    }}, DIF_3STATE, msg(lng::MSetAttrNoScrubData), },
		{ DI_CHECKBOX,  {{C2,      11    }, {0,       11    }}, DIF_3STATE, msg(lng::MSetAttrPinned), },
		{ DI_CHECKBOX,  {{C2,      12    }, {0,       12    }}, DIF_3STATE, msg(lng::MSetAttrUnpinned), },
		{ DI_TEXT,      {{-1,      AR+1  }, {0,       AR+1  }}, DIF_SEPARATOR, },
		{ DI_TEXT,      {{DlgX-29, AR+2  }, {0,       AR+2  }}, DIF_NONE, },
		{ DI_TEXT,      {{DlgX-17, AR+2  }, {0,       AR+2  }}, DIF_NONE, },
		{ DI_TEXT,      {{5,       AR+3  }, {0,       AR+3  }}, DIF_NONE, msg(lng::MSetAttrModification), },
		{ DI_FIXEDIT,   {{DlgX-29, AR+3  }, {DlgX-19, AR+3  }}, DIF_MASKEDIT, },
		{ DI_FIXEDIT,   {{DlgX-17, AR+3  }, {DlgX-6,  AR+3  }}, DIF_MASKEDIT, },
		{ DI_TEXT,      {{5,       AR+4  }, {0,       AR+4  }}, DIF_NONE, msg(lng::MSetAttrCreation), },
		{ DI_FIXEDIT,   {{DlgX-29, AR+4  }, {DlgX-19, AR+4  }}, DIF_MASKEDIT, },
		{ DI_FIXEDIT,   {{DlgX-17, AR+4  }, {DlgX-6,  AR+4  }}, DIF_MASKEDIT, },
		{ DI_TEXT,      {{5,       AR+5  }, {0,       AR+5  }}, DIF_NONE, msg(lng::MSetAttrLastAccess), },
		{ DI_FIXEDIT,   {{DlgX-29, AR+5  }, {DlgX-19, AR+5  }}, DIF_MASKEDIT, },
		{ DI_FIXEDIT,   {{DlgX-17, AR+5  }, {DlgX-6,  AR+5  }}, DIF_MASKEDIT, },
		{ DI_TEXT,      {{5,       AR+6  }, {0,       AR+6  }}, DIF_NONE, msg(lng::MSetAttrChange), },
		{ DI_FIXEDIT,   {{DlgX-29, AR+6  }, {DlgX-19, AR+6  }}, DIF_MASKEDIT, },
		{ DI_FIXEDIT,   {{DlgX-17, AR+6  }, {DlgX-6,  AR+6  }}, DIF_MASKEDIT, },
		{ DI_BUTTON,    {{0,       AR+7  }, {0,       AR+7  }}, DIF_CENTERGROUP | DIF_BTNNOCLOSE, msg(lng::MSetAttrOriginal), },
		{ DI_BUTTON,    {{0,       AR+7  }, {0,       AR+7  }}, DIF_CENTERGROUP | DIF_BTNNOCLOSE, msg(lng::MSetAttrCurrent), },
		{ DI_BUTTON,    {{0,       AR+7  }, {0,       AR+7  }}, DIF_CENTERGROUP | DIF_BTNNOCLOSE, msg(lng::MSetAttrBlank), },
		{ DI_TEXT,      {{-1,      AR+8  }, {0,       AR+8  }}, DIF_SEPARATOR, },
		{ DI_TEXT,      {{5,       AR+9  }, {17,      AR+9  }}, DIF_NONE, msg(lng::MSetAttrOwner), },
		{ DI_EDIT,      {{18,      AR+9  }, {DlgX-6,  AR+9  }}, DIF_NONE, },
		{ DI_TEXT,      {{-1,      AR+10 }, {0,       AR+10 }}, DIF_SEPARATOR | DIF_HIDDEN, },
		{ DI_CHECKBOX,  {{5,       AR+11 }, {0,       AR+11 }}, DIF_DISABLE | DIF_HIDDEN, msg(lng::MSetAttrSubfolders), },
		{ DI_TEXT,      {{-1,      DlgY-4}, {0,       DlgY-4}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,       DlgY-3}, {0,       DlgY-3}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MSetAttrSet), },
		{ DI_BUTTON,    {{0,       DlgY-3}, {0,       DlgY-3}}, DIF_CENTERGROUP | DIF_DISABLE, msg(lng::MSetAttrSystemDialog), },
		{ DI_BUTTON,    {{0,       DlgY-3}, {0,       DlgY-3}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	SetAttrDlgParam DlgParam={};
	const size_t SelCount = SrcPanel? SrcPanel->GetSelCount() : 1;

	if (!SelCount)
	{
		return false;
	}

	if(SelCount==1)
	{
		AttrDlg[SA_BUTTON_SYSTEMDLG].Flags&=~DIF_DISABLE;
	}

	if (SrcPanel && SrcPanel->GetMode() == panel_mode::PLUGIN_PANEL)
	{
		const auto hPlugin = SrcPanel->GetPluginHandle();

		if (hPlugin == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		OpenPanelInfo Info;
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

		if (!(Info.Flags & OPIF_REALNAMES))
		{
			AttrDlg[SA_BUTTON_SET].Flags|=DIF_DISABLE;
			AttrDlg[SA_BUTTON_SYSTEMDLG].Flags|=DIF_DISABLE;
			DlgParam.Plugin=true;
		}
	}

	FarList NameList={sizeof(FarList)};
	std::vector<string> Links;
	std::vector<FarListItem> ListItems;

	{
		string SingleSelFileName;
		os::fs::find_data SingleSelFindData;
		if(SrcPanel)
		{
			if (!SrcPanel->get_first_selected(SingleSelFindData))
				return false;

			SingleSelFileName = SingleSelFindData.FileName;
		}
		else
		{
			SingleSelFileName = *Object;
			// BUGBUG check result
			(void)os::fs::get_find_data(SingleSelFileName, SingleSelFindData);
		}

		if (SelCount==1 && IsParentDirectory(SingleSelFindData))
			return false;

		const auto DateSeparator = locale.date_separator();
		const auto TimeSeparator = locale.time_separator();
		const auto DecimalSeparator = locale.decimal_separator();

		string DateMask, DateFormat;
		date_ranges DateRanges;

		switch (locale.date_format())
		{
		case 0:
			DateMask = format(FSTR(L"99{0}99{0}9999N"), DateSeparator);
			DateFormat = format(msg(lng::MSetAttrDateTitle1), DateSeparator);
			DateRanges = {{ { 0, 2 }, { 3, 2 }, { 6, 5 } }};
			break;

		case 1:
			DateMask = format(FSTR(L"99{0}99{0}9999N"), DateSeparator);
			DateFormat = format(msg(lng::MSetAttrDateTitle2), DateSeparator);
			DateRanges = {{ { 0, 2 }, { 3, 2 }, { 6, 5 } }};
			break;

		default:
			DateMask = format(FSTR(L"N9999{0}99{0}99"), DateSeparator);
			DateFormat = format(msg(lng::MSetAttrDateTitle3), DateSeparator);
			DateRanges = {{ { 0, 5 }, { 6, 2 }, { 9, 2 } }};
			break;
		}

		const auto TimeMask = format(FSTR(L"99{0}99{0}99{1}999"), TimeSeparator, DecimalSeparator);
		const time_ranges TimeRanges{{ {0, 2}, {3, 2}, {6, 2}, {9, 3} }};

		AttrDlg[SA_TEXT_TITLEDATE].strData = DateFormat;
		AttrDlg[SA_TEXT_TITLETIME].strData = format(msg(lng::MSetAttrTimeTitle), TimeSeparator, DecimalSeparator);

		for (const auto& i: TimeMap)
		{
			AttrDlg[i.DateId].strMask = DateMask;
			AttrDlg[i.TimeId].strMask = TimeMask;
		}

		bool FolderPresent=false,LinkPresent=false;
		string strLinkName;

		const auto EnableSubfolders = [&]
		{
			AttrDlg[SA_SEPARATOR4].Flags &= ~DIF_HIDDEN;
			AttrDlg[SA_CHECKBOX_SUBFOLDERS].Flags &= ~(DIF_DISABLE | DIF_HIDDEN);
			AttrDlg[SA_DOUBLEBOX].Y2 += 2;

			for (int i = SA_SEPARATOR5; i <= SA_BUTTON_CANCEL; ++i)
			{
				AttrDlg[i].Y1 += 2;
				AttrDlg[i].Y2 += 2;
			}
			DlgY += 2;
		};

		if (SelCount==1)
		{
			if (SingleSelFindData.Attributes != INVALID_FILE_ATTRIBUTES && (SingleSelFindData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (!DlgParam.Plugin)
				{
					if (const auto AddFileAttr = os::fs::get_file_attributes(SingleSelFileName); AddFileAttr != INVALID_FILE_ATTRIBUTES)
					{
						SingleSelFindData.Attributes |= AddFileAttr;
					}
				}

				EnableSubfolders();
				AttrDlg[SA_CHECKBOX_SUBFOLDERS].Selected = Global->Opt->SetAttrFolderRules? BSTATE_UNCHECKED : BSTATE_CHECKED;

				if (Global->Opt->SetAttrFolderRules)
				{
					if (DlgParam.Plugin || os::fs::get_find_data(SingleSelFileName, SingleSelFindData))
					{
						for (const auto& i: TimeMap)
						{
							ConvertDate(std::invoke(i.Accessor, SingleSelFindData), AttrDlg[i.DateId].strData, AttrDlg[i.TimeId].strData, 12, 2);
						}
					}

					if (SingleSelFindData.Attributes != INVALID_FILE_ATTRIBUTES)
					{
						for (const auto& i: AttributeMap)
						{
							AttrDlg[i.Id].Selected = (SingleSelFindData.Attributes & i.Attribute)? BSTATE_CHECKED : BSTATE_UNCHECKED;
						}
					}

					for (const auto& i: AttributeMap)
					{
						AttrDlg[i.Id].Flags &= ~DIF_3STATE;
					}
				}

				FolderPresent = true;
			}
			else
			{
				for (const auto& i: AttributeMap)
				{
					AttrDlg[i.Id].Flags &= ~DIF_3STATE;
				}
			}

			bool IsMountPoint;
			{
				bool IsRoot = false;
				const auto PathType = ParsePath(SingleSelFileName, nullptr, &IsRoot);
				IsMountPoint = IsRoot && ((PathType == root_type::drive_letter || PathType == root_type::unc_drive_letter));
			}

			if ((SingleSelFindData.Attributes != INVALID_FILE_ATTRIBUTES && (SingleSelFindData.Attributes & FILE_ATTRIBUTE_REPARSE_POINT)) || IsMountPoint)
			{
				DWORD ReparseTag = SingleSelFindData.ReparseTag;
				DWORD ReparseTagAlternative = 0;
				bool KnownReparsePoint = false;
				if (!DlgParam.Plugin)
				{
					if (IsMountPoint)
					{
						// BUGBUG, cheating
						KnownReparsePoint = true;
						ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
						// BUGBUG check result
						(void)os::fs::GetVolumeNameForVolumeMountPoint(SingleSelFileName, strLinkName);
					}
					else
					{
						KnownReparsePoint = GetReparsePointInfo(SingleSelFileName, strLinkName, &ReparseTagAlternative);
						if (ReparseTagAlternative && !ReparseTag)
						{
							ReparseTag = ReparseTagAlternative;
						}

						if (!KnownReparsePoint)
						{
							if (ReparseTag == IO_REPARSE_TAG_DEDUP)
							{
								KnownReparsePoint = true;
								strLinkName = msg(lng::MListDEDUP);
							}
							else if (ReparseTag == IO_REPARSE_TAG_DFS)
							{
								const auto path = path::join(SrcPanel->GetCurDir(), SingleSelFileName);
								os::netapi::ptr<DFS_INFO_3> DfsInfo;
								auto Result = imports.NetDfsGetInfo(UNSAFE_CSTR(path), nullptr, nullptr, 3, reinterpret_cast<LPBYTE*>(&ptr_setter(DfsInfo)));
								if (Result != NERR_Success)
									Result = imports.NetDfsGetClientInfo(UNSAFE_CSTR(path), nullptr, nullptr, 3, reinterpret_cast<LPBYTE*>(&ptr_setter(DfsInfo)));
								if (Result == NERR_Success)
								{
									KnownReparsePoint = true;

									const span DfsStorages(DfsInfo->Storage, DfsInfo->NumberOfStorages);
									ListItems.resize(DfsStorages.size());
									Links.resize(DfsStorages.size());

									for (const auto& [Link, Item, Storage]: zip(Links, ListItems, DfsStorages))
									{
										Link = concat(L"\\\\"sv, Storage.ServerName, L'\\', Storage.ShareName);
										Item.Text = Link.c_str();
										Item.Flags =
											((Storage.State & DFS_STORAGE_STATE_ACTIVE)? (LIF_CHECKED | LIF_SELECTED) : LIF_NONE) |
											((Storage.State & DFS_STORAGE_STATE_OFFLINE)? LIF_GRAYED : LIF_NONE);
									}

									NameList.Items = ListItems.data();
									NameList.ItemsNumber = DfsInfo->NumberOfStorages;

									AttrDlg[SA_EDIT_SYMLINK].Flags |= DIF_HIDDEN;
									AttrDlg[SA_COMBO_SYMLINK].Flags &= ~DIF_HIDDEN;
									AttrDlg[SA_COMBO_SYMLINK].ListItems = &NameList;
									AttrDlg[SA_COMBO_SYMLINK].strData = concat(msg(lng::MSetAttrDfsTargets), L" ("sv, str(NameList.ItemsNumber), L')');
								}
							}
						}
					}
				}
				AttrDlg[SA_DOUBLEBOX].Y2++;

				for (size_t i=SA_TEXT_SYMLINK; i != AttrDlg.size(); i++)
				{
					AttrDlg[i].Y1++;

					if (AttrDlg[i].Y2)
					{
						AttrDlg[i].Y2++;
					}
				}

				LinkPresent=true;
				NormalizeSymlinkName(strLinkName);
				auto ID_Msg = lng::MSetAttrSymlink;

				if (ReparseTag==IO_REPARSE_TAG_MOUNT_POINT)
				{
					bool Root;
					if(ParsePath(strLinkName, nullptr, &Root) == root_type::volume && Root)
					{
						ID_Msg = lng::MSetAttrVolMount;
					}
					else
					{
						ID_Msg = lng::MSetAttrJunction;
					}
				}

				if (!KnownReparsePoint)
					strLinkName=msg(lng::MSetAttrUnknownJunction);

				AttrDlg[SA_TEXT_SYMLINK].Flags &= ~DIF_HIDDEN;
				AttrDlg[SA_TEXT_SYMLINK].strData = msg(ID_Msg);
				if (ReparseTag != IO_REPARSE_TAG_DFS)
					AttrDlg[SA_EDIT_SYMLINK].Flags &= ~DIF_HIDDEN;
				AttrDlg[SA_EDIT_SYMLINK].strData = strLinkName;
				if (ReparseTag == IO_REPARSE_TAG_DEDUP)
					AttrDlg[SA_EDIT_SYMLINK].Flags |= DIF_DISABLE;
				if (ReparseTag == IO_REPARSE_TAG_DEDUP || ReparseTag == IO_REPARSE_TAG_DFS)
					AttrDlg[SA_CHECKBOX_REPARSEPOINT].Flags |= DIF_DISABLE;
			}

			// обработка случая "несколько хардлинков"
			if (!(SingleSelFindData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if ((NameList.ItemsNumber = GetNumberOfLinks(SingleSelFileName)) > 1)
				{
					AttrDlg[SA_TEXT_NAME].Flags|=DIF_HIDDEN;
					AttrDlg[SA_COMBO_HARDLINK].Flags&=~DIF_HIDDEN;

					auto strRoot = GetPathRoot(SingleSelFileName);
					DeleteEndSlash(strRoot);

					Links.reserve(NameList.ItemsNumber);
					for (const auto& i: os::fs::enum_names(SingleSelFileName))
					{
						Links.emplace_back(strRoot + i);
					}

					if (!Links.empty())
					{
						ListItems.reserve(Links.size());
						std::transform(ALL_CONST_RANGE(Links), std::back_inserter(ListItems), [&](const string& i) { return FarListItem{ 0, i.c_str() }; });

						NameList.Items = ListItems.data();
						AttrDlg[SA_COMBO_HARDLINK].ListItems = &NameList;
					}
					else
					{
						AttrDlg[SA_COMBO_HARDLINK].Flags|=DIF_DISABLE;
					}

					AttrDlg[SA_COMBO_HARDLINK].strData = concat(msg(lng::MSetAttrHardLinks), L" ("sv, str(NameList.ItemsNumber), L')');
				}
			}

			AttrDlg[SA_TEXT_NAME].strData = truncate_left(QuoteOuterSpace(SingleSelFileName), DlgX - 10);

			if (SingleSelFindData.Attributes != INVALID_FILE_ATTRIBUTES)
			{
				for (const auto& i: AttributeMap)
				{
					AttrDlg[i.Id].Selected = (SingleSelFindData.Attributes & i.Attribute)? BSTATE_CHECKED : BSTATE_UNCHECKED;
				}
			}

			if (DlgParam.Plugin || os::fs::get_find_data(SingleSelFileName, SingleSelFindData))
			{
				for (const auto& i: TimeMap)
				{
					ConvertDate(std::invoke(i.Accessor, SingleSelFindData), AttrDlg[i.DateId].strData, AttrDlg[i.TimeId].strData, 12, 2);
				}
			}

			string strComputerName;
			if(SrcPanel)
			{
				strComputerName = ExtractComputerName(SrcPanel->GetCurDir());
			}
			GetFileOwner(strComputerName, SingleSelFileName, AttrDlg[SA_EDIT_OWNER].strData);
		}
		else
		{
			for (const auto& i: AttributeMap)
			{
				AttrDlg[i.Id].Selected = BSTATE_3STATE;
			}

			for (const auto& i: TimeMap)
			{
				AttrDlg[i.DateId].strData.clear();
				AttrDlg[i.TimeId].strData.clear();
			}

			AttrDlg[SA_BUTTON_ORIGINAL].Flags|=DIF_DISABLE;
			AttrDlg[SA_TEXT_NAME].strData = msg(lng::MSetAttrSelectedObjects);

			for (const auto& i: AttributeMap)
			{
				AttrDlg[i.Id].Selected = BSTATE_UNCHECKED;
			}

			// проверка - есть ли среди выделенных - каталоги?
			// так же проверка на атрибуты
			FolderPresent=false;

			const auto InspectFile = [&](os::fs::find_data const& FindData)
			{
				if (!FolderPresent && (FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					FolderPresent = true;
					EnableSubfolders();
				}

				for (const auto& i: AttributeMap)
				{
					if (FindData.Attributes & i.Attribute)
					{
						++AttrDlg[i.Id].Selected;
					}
				}
			};


			if(SrcPanel)
			{
				const auto strComputerName = ExtractComputerName(SrcPanel->GetCurDir());

				bool CheckOwner=true;
				for (const auto& i: SrcPanel->enum_selected())
				{
					InspectFile(i);

					if(CheckOwner)
					{
						string strCurOwner;
						GetFileOwner(strComputerName, i.FileName, strCurOwner);
						if(AttrDlg[SA_EDIT_OWNER].strData.empty())
						{
							AttrDlg[SA_EDIT_OWNER].strData=strCurOwner;
						}
						else if(AttrDlg[SA_EDIT_OWNER].strData != strCurOwner)
						{
							AttrDlg[SA_EDIT_OWNER].strData.clear();
							CheckOwner=false;
						}
					}
				}
			}
			else
			{
				InspectFile(SingleSelFindData);
			}

			if(SrcPanel)
			{
				SrcPanel->get_first_selected(SingleSelFindData);
				SingleSelFileName = SingleSelFindData.FileName;
			}

			// выставим "неопределенку" или то, что нужно
			for (const auto& i: AttributeMap)
			{
				// снимаем 3-state, если "есть все или нет ничего"
				// за исключением случая, если есть Фолдер среди объектов
				if ((!AttrDlg[i.Id].Selected || static_cast<size_t>(AttrDlg[i.Id].Selected) >= SelCount) && !FolderPresent)
				{
					AttrDlg[i.Id].Flags &= ~DIF_3STATE;
				}

				AttrDlg[i.Id].Selected = (static_cast<size_t>(AttrDlg[i.Id].Selected) >= SelCount)? BST_CHECKED : (!AttrDlg[i.Id].Selected? BSTATE_UNCHECKED : BSTATE_3STATE);
			}
		}

		// запомним состояние переключателей.
		for (size_t i=SA_ATTR_FIRST; i<=SA_ATTR_LAST; i++)
		{
			DlgParam.cb[i - SA_ATTR_FIRST].Flags = AttrDlg[i].Flags;
			DlgParam.cb[i - SA_ATTR_FIRST].Value = AttrDlg[i].Selected;
			DlgParam.cb[i - SA_ATTR_FIRST].Changed = false;
		}
		DlgParam.strOwner=AttrDlg[SA_EDIT_OWNER].strData;
		string strInitOwner=AttrDlg[SA_EDIT_OWNER].strData;

		// поведение для каталогов как у 1.65?
		if (FolderPresent && !Global->Opt->SetAttrFolderRules)
		{
			AttrDlg[SA_CHECKBOX_SUBFOLDERS].Selected=BSTATE_CHECKED;

			for (const auto& i: TimeMap)
			{
				AttrDlg[i.DateId].strData.clear();
				AttrDlg[i.TimeId].strData.clear();
			}

			for (const auto& i: AttributeMap)
			{
				AttrDlg[i.Id].Flags |= DIF_3STATE;
				AttrDlg[i.Id].Selected = BSTATE_3STATE;
			}

			AttrDlg[SA_EDIT_OWNER].strData.clear();
		}

		DlgParam.DialogMode = ((SelCount == 1 && !(SingleSelFindData.Attributes &FILE_ATTRIBUTE_DIRECTORY))? MODE_FILE :(SelCount == 1? MODE_FOLDER : MODE_MULTIPLE));
		DlgParam.strSelName = SingleSelFileName;
		DlgParam.OSubfoldersState=static_cast<FARCHECKEDSTATE>(AttrDlg[SA_CHECKBOX_SUBFOLDERS].Selected);

		const auto Dlg = Dialog::create(AttrDlg, SetAttrDlgProc, &DlgParam);
		Dlg->SetHelp(L"FileAttrDlg"sv);
		Dlg->SetId(FileAttrDlgId);

		if (LinkPresent)
		{
			DlgY++;
		}

		Dlg->SetPosition({ -1, -1, DlgX, DlgY });
		Dlg->Process();

		switch(Dlg->GetExitCode())
		{
		case SA_BUTTON_SET:
			{
				//reparse point editor, can be used even if the attributes are invalid (e.g. a CD drive)
				if (!equal_icase(AttrDlg[SA_EDIT_SYMLINK].strData, strLinkName))
				{
					for (;;)
					{
						if (ModifyReparsePoint(SingleSelFileName, unquote(AttrDlg[SA_EDIT_SYMLINK].strData)))
							break;

						const auto ErrorState = error_state::fetch();
						const auto OperationResult = OperationFailed(ErrorState, SingleSelFileName, lng::MError, msg(lng::MCopyCannotCreateLink), false);

						if (OperationResult == operation::retry)
						{
							continue;
						}
						if (OperationResult == operation::skip)
						{
							break;
						}
						else if (OperationResult == operation::cancel)
						{
							return false;
						}
					}
				}

				if (SingleSelFindData.Attributes == INVALID_FILE_ATTRIBUTES)
					return true;

				DWORD SetAttr = 0, ClearAttr = 0;

				for (const auto& i: AttributeMap)
				{
					switch (AttrDlg[i.Id].Selected)
					{
					case BSTATE_CHECKED:
						SetAttr |= i.Attribute;
						break;

					case BSTATE_UNCHECKED:
						ClearAttr |= i.Attribute;
						break;
					}
				}

				for (const auto& i: TimeMap)
				{
					AttrDlg[i.TimeId].strData[8] = locale.time_separator();
				}

				SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<AttrPreRedrawItem>());
				ShellSetFileAttributesMsg(SelCount==1? SingleSelFileName : string{});

				auto SkipErrors = false;

				const auto AttrDlgAccessor = [&AttrDlg](int const Id) -> auto& { return AttrDlg[Id].strData; };

				if (SelCount==1 && !(SingleSelFindData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					os::fs::find_data NewFindData;
					NewFindData.Attributes = (SingleSelFindData.Attributes | SetAttr) & ~ClearAttr;

					const state
						Current{ strInitOwner, SingleSelFindData },
						New{ AttrDlg[SA_EDIT_OWNER].strData, NewFindData };

					if (!process_single_file(SingleSelFileName, Current, New, AttrDlgAccessor, DateRanges, TimeRanges, SkipErrors))
					{
						return false;
					}
				}
				else
				{
					ConsoleTitle::SetFarTitle(msg(lng::MSetAttrTitle));

					if(SrcPanel)
					{
						Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->CloseFile();
					}

					if(SrcPanel)
					{
						SrcPanel->GetSelName(nullptr);
					}

					SCOPED_ACTION(taskbar::indeterminate);
					SCOPED_ACTION(wakeful);

					const time_check TimeCheck(time_check::mode::immediate, GetRedrawTimeout());

					while (!SrcPanel || SrcPanel->GetSelName(&SingleSelFileName, nullptr, &SingleSelFindData))
					{
						if (TimeCheck)
						{
							ShellSetFileAttributesMsg(SingleSelFileName);

							if (CheckForEsc())
								break;
						}

						{
							os::fs::find_data NewFindData;
							NewFindData.Attributes = (SingleSelFindData.Attributes | SetAttr) & ~ClearAttr;

							const state
								Current{ L""s, SingleSelFindData }, // BUGBUG, should we read the owner?
								New{ AttrDlg[SA_EDIT_OWNER].strData, NewFindData };

							if (!process_single_file(SingleSelFileName, Current, New, AttrDlgAccessor, DateRanges, TimeRanges, SkipErrors))
							{
								return false;
							}
						}

						if ((SingleSelFindData.Attributes & FILE_ATTRIBUTE_DIRECTORY) && AttrDlg[SA_CHECKBOX_SUBFOLDERS].Selected)
						{
							ScanTree ScTree(false);
							ScTree.SetFindPath(SingleSelFileName, L"*"sv);
							const time_check TreeTimeCheck(time_check::mode::delayed, GetRedrawTimeout());
							string strFullName;

							while (ScTree.GetNextName(SingleSelFindData, strFullName))
							{
								if (TreeTimeCheck)
								{
									ShellSetFileAttributesMsg(strFullName);

									if (CheckForEsc())
									{
										return false;
									}
								}

								os::fs::find_data NewFindData;
								NewFindData.Attributes = (SingleSelFindData.Attributes | SetAttr) & ~ClearAttr;

								const state
									Current{ L""s, SingleSelFindData }, // BUGBUG, should we read the owner?
									New{ AttrDlg[SA_EDIT_OWNER].strData, NewFindData };

								if (!process_single_file(strFullName, Current, New, AttrDlgAccessor, DateRanges, TimeRanges, SkipErrors))
								{
									return false;
								}
							}
						}

						if (!SrcPanel)
							break;
					}
				}
			}
			break;
		case SA_BUTTON_SYSTEMDLG:
			{
				SHELLEXECUTEINFOW seInfo={sizeof(seInfo)};
				seInfo.nShow = SW_SHOW;
				seInfo.fMask = SEE_MASK_INVOKEIDLIST;
				NTPath strFullName(SingleSelFileName);
				if(SingleSelFindData.Attributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					AddEndSlash(strFullName);
				}
				seInfo.lpFile = strFullName.c_str();
				if (!IsWindowsVistaOrGreater() && ParsePath(seInfo.lpFile) == root_type::unc_drive_letter)
				{	// "\\?\c:\..." fails on old windows
					seInfo.lpFile += 4;
				}
				seInfo.lpVerb = L"properties";
				const auto strCurDir = os::fs::GetCurrentDirectory();
				seInfo.lpDirectory=strCurDir.c_str();
				ShellExecuteExW(&seInfo);
			}
			return false;

		default:
			return false;
		}
	}

	if(SrcPanel)
	{
		SrcPanel->SaveSelection();
		SrcPanel->Update(UPDATE_KEEP_SELECTION);
		SrcPanel->ClearSelection();
		Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	}
	Global->WindowManager->RefreshWindow(Global->CtrlObject->Panels());
	return true;
}
