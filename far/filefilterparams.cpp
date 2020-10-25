/*
filefilterparams.cpp

Параметры Файлового фильтра
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
#include "filefilterparams.hpp"

// Internal:
#include "farcolor.hpp"
#include "filemasks.hpp"
#include "dialog.hpp"
#include "filelist.hpp"
#include "colormix.hpp"
#include "message.hpp"
#include "interf.hpp"
#include "datetime.hpp"
#include "strmix.hpp"
#include "console.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "locale.hpp"
#include "fileattr.hpp"
#include "uuids.far.dialogs.hpp"
#include "FarDlgBuilder.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common.hpp"
#include "common/2d/point.hpp"
#include "common/2d/matrix.hpp"
#include "common/view/zip.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

const point ColorExampleSize{ 15, 4 };

filter_dates::filter_dates(os::chrono::duration After, os::chrono::duration Before):
	m_After(After),
	m_Before(Before),
	m_Relative(true)
{
}

filter_dates::filter_dates(os::chrono::time_point After, os::chrono::time_point Before):
	m_After(After.time_since_epoch()),
	m_Before(Before.time_since_epoch())
{
}

filter_dates::operator bool() const
{
	return m_After != 0s || m_Before != 0s;
}


FileFilterParams::FileFilterParams()
{
	SetMask(true, L"*"sv);

	for (auto& i: FHighlight.Colors.Color)
	{
		colors::make_opaque(i.FileColor.ForegroundColor);
		colors::make_opaque(i.MarkColor.ForegroundColor);
	}

	FHighlight.SortGroup=DEFAULT_SORT_GROUP;
}

FileFilterParams FileFilterParams::Clone() const
{
	FileFilterParams Result;
	Result.m_strTitle = m_strTitle;
	Result.SetMask(IsMaskUsed(), GetMask());
	Result.FSize = FSize;
	Result.FDate = FDate;
	Result.FAttr = FAttr;
	Result.FHardLinks = FHardLinks;
	Result.FHighlight = FHighlight;
	Result.FFlags = FFlags;
	return Result;
}

void FileFilterParams::SetTitle(string_view const Title)
{
	m_strTitle = Title;
}

void FileFilterParams::SetMask(bool const Used, string_view const Mask)
{
	FMask.Used = Used;
	FMask.strMask = Mask;
	if (Used)
	{
		FMask.FilterMask.assign(FMask.strMask, FMF_SILENT);
	}
}

void FileFilterParams::SetDate(bool const Used, enumFDateType const DateType, const filter_dates& Dates)
{
	FDate.Used=Used;
	FDate.DateType = DateType < FDATE_COUNT? DateType : FDATE_MODIFIED;
	FDate.Dates = Dates;
}

void FileFilterParams::SetSize(bool const Used, string_view const SizeAbove, string_view const SizeBelow)
{
	FSize.Used=Used;
	FSize.Above.Size = SizeAbove;
	FSize.Below.Size = SizeBelow;
	FSize.Above.SizeReal = ConvertFileSizeString(FSize.Above.Size);
	FSize.Below.SizeReal = ConvertFileSizeString(FSize.Below.Size);
}

void FileFilterParams::SetHardLinks(bool const Used, DWORD const HardLinksAbove, DWORD const HardLinksBelow)
{
	FHardLinks.Used=Used;
	FHardLinks.CountAbove=HardLinksAbove;
	FHardLinks.CountBelow=HardLinksBelow;
}

void FileFilterParams::SetAttr(bool const Used, os::fs::attributes const AttrSet, os::fs::attributes const AttrClear)
{
	FAttr.Used=Used;
	FAttr.AttrSet=AttrSet;
	FAttr.AttrClear=AttrClear;
}

void FileFilterParams::SetColors(const highlight::element& Colors)
{
	FHighlight.Colors = Colors;
}

const string& FileFilterParams::GetTitle() const
{
	return m_strTitle;
}

bool FileFilterParams::GetDate(DWORD* DateType, filter_dates* Dates) const
{
	if (DateType)
		*DateType=FDate.DateType;

	if (Dates)
		*Dates = FDate.Dates;

	return FDate.Used;
}

bool FileFilterParams::GetHardLinks(DWORD *HardLinksAbove, DWORD *HardLinksBelow) const
{
	if (HardLinksAbove)
		*HardLinksAbove = FHardLinks.CountAbove;
	if (HardLinksBelow)
		*HardLinksBelow = FHardLinks.CountBelow;
	return FHardLinks.Used;
}


bool FileFilterParams::GetAttr(os::fs::attributes* AttrSet, os::fs::attributes* AttrClear) const
{
	if (AttrSet)
		*AttrSet=FAttr.AttrSet;

	if (AttrClear)
		*AttrClear=FAttr.AttrClear;

	return FAttr.Used;
}

highlight::element FileFilterParams::GetColors() const
{
	return FHighlight.Colors;
}

wchar_t FileFilterParams::GetMarkChar() const
{
	return FHighlight.Colors.Mark.Char;
}

struct filter_file_object
{
	string_view Name;
	unsigned long long Size;
	os::chrono::time_point CreationTime;
	os::chrono::time_point ModificationTime;
	os::chrono::time_point AccessTime;
	os::chrono::time_point ChangeTime;
	os::fs::attributes Attributes;
	int NumberOfLinks;
};

bool FileFilterParams::FileInFilter(const FileListItem& Object, const FileList* Owner, os::chrono::time_point CurrentTime) const
{
	const filter_file_object FilterObject
	{
		Object.FileName,
		Object.FileSize,
		Object.CreationTime,
		Object.LastWriteTime,
		Object.LastAccessTime,
		Object.ChangeTime,
		Object.Attributes,
	};

	return FileInFilter(FilterObject, CurrentTime, [&]{ return Object.NumberOfLinks(Owner); });
}

bool FileFilterParams::FileInFilter(const os::fs::find_data& Object, os::chrono::time_point const CurrentTime, string_view const FullName) const
{
	const filter_file_object FilterObject
	{
		Object.FileName,
		Object.FileSize,
		Object.CreationTime,
		Object.LastWriteTime,
		Object.LastAccessTime,
		Object.ChangeTime,
		Object.Attributes,
	};

	return FileInFilter(FilterObject, CurrentTime, [&]
	{
		if (FullName.empty())
			return DWORD{1};

		const auto Hardlinks = GetNumberOfLinks(FullName);
		return Hardlinks? static_cast<DWORD>(*Hardlinks) : 1;
	});
}

bool FileFilterParams::FileInFilter(const PluginPanelItem& Object, os::chrono::time_point CurrentTime) const
{
	const filter_file_object FilterObject
	{
		Object.FileName,
		Object.FileSize,
		os::chrono::nt_clock::from_filetime(Object.CreationTime),
		os::chrono::nt_clock::from_filetime(Object.LastWriteTime),
		os::chrono::nt_clock::from_filetime(Object.LastAccessTime),
		os::chrono::nt_clock::from_filetime(Object.ChangeTime),
		static_cast<os::fs::attributes>(Object.FileAttributes),
		static_cast<int>(Object.NumberOfLinks),
	};

	return FileInFilter(FilterObject, CurrentTime, nullptr);
}

bool FileFilterParams::FileInFilter(const filter_file_object& Object, os::chrono::time_point CurrentTime, function_ref<int()> const HardlinkGetter) const
{
	// Режим проверки атрибутов файла включен?
	if (FAttr.Used)
	{
		// Проверка попадания файла по установленным атрибутам
		if ((Object.Attributes & FAttr.AttrSet) != FAttr.AttrSet)
			return false;

		// Проверка попадания файла по отсутствующим атрибутам
		if (Object.Attributes & FAttr.AttrClear)
			return false;
	}

	// Режим проверки размера файла включен?
	if (FSize.Used)
	{
		// Размер файла меньше минимального разрешённого по фильтру?
		if (!FSize.Above.Size.empty() && Object.Size < FSize.Above.SizeReal)
			return false;

		// Размер файла больше максимального разрешённого по фильтру?
		if (!FSize.Below.Size.empty() && Object.Size > FSize.Below.SizeReal)
			return false;
	}

	// Режим проверки времени файла включен?
	// Есть введённая пользователем начальная / конечная дата?
	if (FDate.Used && FDate.Dates)
	{
		const auto& ft = [&]() -> const auto&
		{
			switch (FDate.DateType)
			{
			case FDATE_CREATED:  return Object.CreationTime;
			case FDATE_OPENED:   return Object.AccessTime;
			case FDATE_CHANGED:  return Object.ChangeTime;
			case FDATE_MODIFIED: return Object.ModificationTime;
			default: UNREACHABLE; // Validated in SetDate()
			}
		}();

		// Дата файла меньше начальной / больше конечной даты по фильтру?
		if (FDate.Dates.visit(overload
		{
			[&](os::chrono::duration After, os::chrono::duration Before)
			{
				return (After != 0s && ft < CurrentTime - After) || (Before != 0s && ft > CurrentTime - Before);
			},
			[&](os::chrono::time_point After, os::chrono::time_point Before)
			{
				return (After != os::chrono::time_point{} && ft < After) || (Before != os::chrono::time_point{} && ft > Before);
			}
		}))
			return false;
	}

	// Режим проверки маски файла включен?
	if (FMask.Used)
	{
		// ЭТО ЕСТЬ УЗКОЕ МЕСТО ДЛЯ СКОРОСТНЫХ ХАРАКТЕРИСТИК Far Manager
		// при считывании директории

		// Файл не попадает под маску введённую в фильтре?
		if (!FMask.FilterMask.check(Object.Name))
		// Не пропускаем этот файл
			return false;
	}

	// Режим проверки количества жестких ссылок на файл включен?
	// Пока что, при включенном условии, срабатывание происходит при случае "ссылок больше чем одна"
	if (FHardLinks.Used)
	{
		if (Object.Attributes & FILE_ATTRIBUTE_DIRECTORY)
			return false;

		if (const auto NumberOfLinks = HardlinkGetter? HardlinkGetter() : Object.NumberOfLinks; NumberOfLinks < 2)
			return false;
	}

	// Да! Файл выдержал все испытания и будет допущен к использованию
	// в вызвавшей эту функцию операции.
	return true;
}

static string AttributesString(DWORD Include, DWORD Exclude)
{
	string IncludeStr, ExcludeStr;

	enum_attributes([&](os::fs::attributes const Attribute, wchar_t const Character)
	{
		if (Include & Attribute)
		{
			IncludeStr.push_back(Character);
		}
		else if (Exclude & Attribute)
		{
			ExcludeStr.push_back(Character);
		}
		return true;
	});

	string Result;

	const auto MaxAttributesToDisplay = 3;

	if (!IncludeStr.empty())
	{
		append(Result, L'+', truncate_right(IncludeStr, MaxAttributesToDisplay + 1));
	}

	if (!ExcludeStr.empty())
	{
		if (!Result.empty())
			Result += L' ';

		append(Result, L'-', truncate_right(ExcludeStr, MaxAttributesToDisplay + 1));
	}

	// "+ABC… -DEF…"
	static const auto MaxSize = (MaxAttributesToDisplay + 1 + 1) * 2 + 1;

	return pad_right(Result, MaxSize);
}

//Централизованная функция для создания строк меню различных фильтров.
string MenuString(const FileFilterParams* const FF, bool const bHighlightType, wchar_t const Hotkey, bool const bPanelType, string_view const FMask, string_view const Title)
{
	const wchar_t DownArrow = L'\x2193';
	string_view Name;
	auto Mask = L""sv;
	auto MarkChar = L"' '"s;
	os::fs::attributes IncludeAttr, ExcludeAttr;
	bool UseSize, UseHardLinks, UseDate, RelativeDate;

	if (bPanelType)
	{
		Name=Title;
		Mask=FMask;
		IncludeAttr=0;
		ExcludeAttr=FILE_ATTRIBUTE_DIRECTORY;
		RelativeDate=UseDate=UseSize=UseHardLinks=false;
	}
	else
	{
		if (const auto Char = FF->GetMarkChar())
		{
			MarkChar[1] = Char;
			if (Char == L'&')
				MarkChar.insert(1, 1, L'&');
		}
		else
			MarkChar.clear();

		Name=FF->GetTitle();

		if (FF->IsMaskUsed())
			Mask = FF->GetMask();

		if (!FF->GetAttr(&IncludeAttr,&ExcludeAttr))
			IncludeAttr=ExcludeAttr=0;

		UseSize=FF->IsSizeUsed();
		filter_dates Dates;
		UseDate=FF->GetDate(nullptr, &Dates);
		Dates.visit(overload
		{
			[&](os::chrono::duration, os::chrono::duration) { RelativeDate = true; },
			[&](os::chrono::time_point, os::chrono::time_point) { RelativeDate = false; }
		});
		UseHardLinks=FF->GetHardLinks(nullptr,nullptr);
	}

	Mask = trim_right(Mask);

	string EscapedMask;

	if (contains(Mask, L"&"sv))
	{
		EscapedMask = escape_ampersands(Mask);
		Mask = EscapedMask;
	}

	const auto AttrStr = AttributesString(IncludeAttr, ExcludeAttr);

	string OtherFlags;
	OtherFlags.reserve(4);

	const auto SetFlag = [&OtherFlags](bool const Set, wchar_t const Flag)
	{
		OtherFlags.push_back(Set? Flag : L'.');
	};

	SetFlag(UseSize, L'S');
	SetFlag(UseDate, RelativeDate? L'R' : L'D');
	SetFlag(UseHardLinks, L'H');

	const auto MaxNameSize = 21;

	if (bHighlightType)
	{
		SetFlag(FF->GetContinueProcessing(), DownArrow);

		const auto AmpFix = Name.size() - HiStrlen(Name);

		return format(FSTR(L"{1:3} {0} {2:{3}.{3}} {0} {4} {5} {0} {6}"),
			BoxSymbols[BS_V1],
			MarkChar,
			Name,
			MaxNameSize + AmpFix,
			AttrStr,
			OtherFlags,
			Mask
		);
	}

	const auto HotkeyStr = Hotkey? format(FSTR(L"&{0}. "), Hotkey) : bPanelType? L"   "s : L""s;
	const auto AmpFix = Hotkey? 1 : Name.size() - HiStrlen(Name);

	return format(FSTR(L"{1}{2:{3}.{3}} {0} {4} {5} {0} {6}"),
			BoxSymbols[BS_V1],
			HotkeyStr,
			Name,
			MaxNameSize + AmpFix - HotkeyStr.size(),
			AttrStr,
			OtherFlags,
			Mask
	);
}

enum enumFileFilterConfig
{
	ID_FF_TITLE,

	ID_FF_NAME,
	ID_FF_NAMEEDIT,

	ID_FF_SEPARATOR1,

	ID_FF_MATCHMASK,
	ID_FF_MASKEDIT,

	ID_FF_SEPARATOR2,

	ID_FF_MATCHSIZE,
	ID_FF_SIZEFROMSIGN,
	ID_FF_SIZEFROMEDIT,
	ID_FF_SIZETOSIGN,
	ID_FF_SIZETOEDIT,

	ID_FF_MATCHDATE,
	ID_FF_DATETYPE,
	ID_FF_DATERELATIVE,
	ID_FF_DATEBEFORESIGN,
	ID_FF_DATEBEFOREEDIT,
	ID_FF_DAYSBEFOREEDIT,
	ID_FF_TIMEBEFOREEDIT,
	ID_FF_DATEAFTERSIGN,
	ID_FF_DATEAFTEREDIT,
	ID_FF_DAYSAFTEREDIT,
	ID_FF_TIMEAFTEREDIT,
	ID_FF_CURRENT,
	ID_FF_BLANK,

	ID_FF_SEPARATOR3,
	ID_FF_SEPARATOR4,

	ID_FF_CHECKBOX_ATTRIBUTES,
	ID_FF_BUTTON_ATTRIBUTES,
	ID_FF_HARDLINKS,

	ID_HER_SEPARATOR1,
	ID_HER_MARK_TITLE,
	ID_HER_MARKEDIT,
	ID_HER_MARKTRANSPARENT,

	ID_HER_NORMALFILE,
	ID_HER_NORMALMARKING,
	ID_HER_SELECTEDFILE,
	ID_HER_SELECTEDMARKING,
	ID_HER_CURSORFILE,
	ID_HER_CURSORMARKING,
	ID_HER_SELECTEDCURSORFILE,
	ID_HER_SELECTEDCURSORMARKING,

	ID_HER_COLOREXAMPLE,

	ID_HER_CONTINUEPROCESSING,

	ID_FF_SEPARATOR5,

	ID_FF_OK,
	ID_FF_RESET,
	ID_FF_MAKETRANSPARENT,
	ID_FF_CANCEL,

	ID_FF_COUNT
};

struct attribute_map
{
	os::fs::attributes Attribute;
	lng Name;
	int State;
};

struct context
{
	highlight::element* Colors;
	span<attribute_map> Attributes;
};

static void HighlightDlgUpdateUserControl(matrix_view<FAR_CHAR_INFO> const& VBufColorExample, const highlight::element &Colors)
{
	const size_t ColorIndices[]{ highlight::color::normal, highlight::color::selected, highlight::color::normal_current, highlight::color::selected_current };

	int VBufRow = 0;
	for (const auto& [ColorRef, Index, Row]: zip(Colors.Color, ColorIndices, VBufColorExample))
	{
		auto BakedColor = ColorRef;
		highlight::configuration::ApplyFinalColor(BakedColor, Index);

		Row.front() = { BoxSymbols[BS_V2], colors::PaletteColorToFarColor(COL_PANELBOX) };

		auto Iterator = Row.begin() + 1;

		if (Colors.Mark.Char)
		{
			Iterator->Char = Colors.Mark.Transparent? L' ' : Colors.Mark.Char;
			Iterator->Attributes = BakedColor.MarkColor;
			++Iterator;
		}

		const span FileArea(Iterator, Row.end() - 1);
		const auto Str = fit_to_left(msg(lng::MHighlightExample), FileArea.size());

		for (const auto& [Cell, Char] : zip(FileArea, Str))
		{
			Cell = { Char, BakedColor.FileColor };
		}

		Row.back() = { BoxSymbols[BS_V1], Row.front().Attributes };

		++VBufRow;
	}
}

static void FilterDlgRelativeDateItemsUpdate(Dialog* Dlg, bool bClear)
{
	SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

	if (Dlg->SendMessage(DM_GETCHECK, ID_FF_DATERELATIVE, nullptr))
	{
		Dlg->SendMessage(DM_SHOWITEM, ID_FF_DATEBEFOREEDIT, nullptr);
		Dlg->SendMessage(DM_SHOWITEM, ID_FF_DATEAFTEREDIT, nullptr);
		Dlg->SendMessage(DM_SHOWITEM, ID_FF_CURRENT, nullptr);
		Dlg->SendMessage(DM_SHOWITEM,ID_FF_DAYSBEFOREEDIT,ToPtr(1));
		Dlg->SendMessage(DM_SHOWITEM,ID_FF_DAYSAFTEREDIT,ToPtr(1));
	}
	else
	{
		Dlg->SendMessage(DM_SHOWITEM, ID_FF_DAYSBEFOREEDIT, nullptr);
		Dlg->SendMessage(DM_SHOWITEM, ID_FF_DAYSAFTEREDIT, nullptr);
		Dlg->SendMessage(DM_SHOWITEM,ID_FF_DATEBEFOREEDIT,ToPtr(1));
		Dlg->SendMessage(DM_SHOWITEM,ID_FF_DATEAFTEREDIT,ToPtr(1));
		Dlg->SendMessage(DM_SHOWITEM,ID_FF_CURRENT,ToPtr(1));
	}

	if (bClear)
	{
		Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_DATEAFTEREDIT,nullptr);
		Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_DAYSAFTEREDIT,nullptr);
		Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,nullptr);
		Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_DATEBEFOREEDIT,nullptr);
		Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,nullptr);
		Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_DAYSBEFOREEDIT,nullptr);
	}
}

static bool AttributesDialog(span<attribute_map> const Attributes)
{
	DialogBuilder Builder(lng::MSetAttrTitle);

	for (auto& i: Attributes)
		Builder.AddCheckbox(i.Name, i.State, 0, true);

	Builder.AddOKCancel();

	return Builder.ShowDialog();
}

static intptr_t FileFilterConfigDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	const auto DM_REFRESHCOLORS = DM_USER + 1;

	switch (Msg)
	{
	case DN_INITDIALOG:
		FilterDlgRelativeDateItemsUpdate(Dlg, false);
		break;

	case DN_BTNCLICK:
		{
			if (Param1 == ID_FF_BUTTON_ATTRIBUTES)
			{
				const auto Context = reinterpret_cast<const context*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
				AttributesDialog(Context->Attributes);
				break;
			}
			else if (Param1==ID_FF_CURRENT || Param1==ID_FF_BLANK)
			{
				string Date, Time;

				if (Param1==ID_FF_CURRENT)
				{
					ConvertDate(os::chrono::nt_clock::now(), Date, Time, 16, 2);
				}

				SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

				const auto relative = Dlg->SendMessage(DM_GETCHECK, ID_FF_DATERELATIVE, nullptr) != 0;
				const auto db = relative? ID_FF_DAYSBEFOREEDIT : ID_FF_DATEBEFOREEDIT;
				const auto da = relative? ID_FF_DAYSAFTEREDIT  : ID_FF_DATEAFTEREDIT;

				Dlg->SendMessage(DM_SETTEXTPTR,da, UNSAFE_CSTR(Date));
				Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, da, nullptr);
				Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT, UNSAFE_CSTR(Time));
				Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, ID_FF_TIMEAFTEREDIT, nullptr);

				Dlg->SendMessage(DM_SETTEXTPTR,db, UNSAFE_CSTR(Date));
				Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, db, nullptr);
				Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT, UNSAFE_CSTR(Time));
				Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, ID_FF_TIMEBEFOREEDIT, nullptr);

				Dlg->SendMessage(DM_SETFOCUS, db, nullptr);

				COORD r{};
				Dlg->SendMessage(DM_SETCURSORPOS,db,&r);
				break;
			}
			else if (Param1==ID_FF_RESET)
			{
				SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

				const auto Context = reinterpret_cast<const context*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
				Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_MASKEDIT,const_cast<wchar_t*>(L"*"));
				Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_SIZEFROMEDIT,nullptr);
				Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_SIZETOEDIT,nullptr);

				for (auto& i: Context->Attributes)
				{
					i.State = !Context->Colors && i.Attribute == FILE_ATTRIBUTE_DIRECTORY?
						BSTATE_UNCHECKED:
						BSTATE_3STATE;
				}

				FarListPos LPos={sizeof(FarListPos)};
				Dlg->SendMessage(DM_LISTSETCURPOS,ID_FF_DATETYPE,&LPos);
				Dlg->SendMessage(DM_SETCHECK,ID_FF_MATCHMASK,ToPtr(BSTATE_CHECKED));
				Dlg->SendMessage(DM_SETCHECK,ID_FF_MATCHSIZE,ToPtr(BSTATE_UNCHECKED));
				Dlg->SendMessage(DM_SETCHECK,ID_FF_HARDLINKS,ToPtr(BSTATE_UNCHECKED));
				Dlg->SendMessage(DM_SETCHECK,ID_FF_MATCHDATE,ToPtr(BSTATE_UNCHECKED));
				Dlg->SendMessage(DM_SETCHECK,ID_FF_DATERELATIVE,ToPtr(BSTATE_UNCHECKED));
				FilterDlgRelativeDateItemsUpdate(Dlg, true);
				Dlg->SendMessage(DM_SETCHECK,ID_FF_CHECKBOX_ATTRIBUTES, ToPtr(Context->Colors? BSTATE_UNCHECKED : BSTATE_CHECKED));

				break;
			}
			else if (Param1==ID_FF_MAKETRANSPARENT)
			{
				const auto Context = reinterpret_cast<const context*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

				for (auto& i: Context->Colors->Color)
				{
					colors::make_transparent(i.FileColor.ForegroundColor);
					colors::make_transparent(i.FileColor.BackgroundColor);
					colors::make_transparent(i.MarkColor.ForegroundColor);
					colors::make_transparent(i.MarkColor.BackgroundColor);
				}

				Dlg->SendMessage(DM_SETCHECK,ID_HER_MARKTRANSPARENT,ToPtr(BSTATE_CHECKED));
				Dlg->SendMessage(DM_REFRESHCOLORS, 0, nullptr);
				break;
			}
			else if (Param1==ID_FF_DATERELATIVE)
			{
				FilterDlgRelativeDateItemsUpdate(Dlg, true);
				break;
			}
			else if (Param1 == ID_HER_MARKTRANSPARENT)
			{
				Dlg->SendMessage(DM_REFRESHCOLORS, 0, nullptr);
				break;
			}
		}
		[[fallthrough]];
	case DN_CONTROLINPUT:

		if ((Msg==DN_BTNCLICK && Param1 >= ID_HER_NORMALFILE && Param1 <= ID_HER_SELECTEDCURSORMARKING)
			|| (Msg==DN_CONTROLINPUT && Param1==ID_HER_COLOREXAMPLE && static_cast<const INPUT_RECORD*>(Param2)->EventType == MOUSE_EVENT && static_cast<const INPUT_RECORD*>(Param2)->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED))
		{
			const auto Context = reinterpret_cast<const context*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

			if (Msg==DN_CONTROLINPUT)
			{
				Param1 = ID_HER_NORMALFILE + static_cast<const INPUT_RECORD*>(Param2)->Event.MouseEvent.dwMousePosition.Y*2;

				if (static_cast<const INPUT_RECORD*>(Param2)->Event.MouseEvent.dwMousePosition.X==1 && Context->Colors->Mark.Char)
					Param1 = ID_HER_NORMALMARKING + static_cast<const INPUT_RECORD*>(Param2)->Event.MouseEvent.dwMousePosition.Y*2;
			}

			//Color[0=file, 1=mark][0=normal,1=selected,2=undercursor,3=selectedundercursor]
			static const PaletteColors BaseIndices[]{ COL_PANELTEXT, COL_PANELSELECTEDTEXT, COL_PANELCURSOR, COL_PANELSELECTEDCURSOR };
			const auto BaseColor = colors::PaletteColorToFarColor(BaseIndices[(Param1 - ID_HER_NORMALFILE) / 2]);

			if (console.GetColorDialog(((Param1-ID_HER_NORMALFILE)&1)? Context->Colors->Color[(Param1-ID_HER_NORMALFILE)/2].MarkColor : Context->Colors->Color[(Param1-ID_HER_NORMALFILE)/2].FileColor, true, &BaseColor))
				Dlg->SendMessage(DM_REFRESHCOLORS, 0, nullptr);
		}

		break;

	case DM_REFRESHCOLORS:
		{
			const auto Context = reinterpret_cast<const context*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
			const size_t Size = Dlg->SendMessage(DM_GETDLGITEM, ID_HER_COLOREXAMPLE, nullptr);
			const block_ptr<FarDialogItem> Buffer(Size);
			FarGetDialogItem gdi = {sizeof(FarGetDialogItem), Size, Buffer.data()};
			Dlg->SendMessage(DM_GETDLGITEM,ID_HER_COLOREXAMPLE,&gdi);
			//MarkChar это FIXEDIT размером в 1 символ
			wchar_t MarkChar[2];
			FarDialogItemData item={sizeof(FarDialogItemData),1,MarkChar};
			Dlg->SendMessage(DM_GETTEXT,ID_HER_MARKEDIT,&item);
			Context->Colors->Mark.Char = *MarkChar;
			Context->Colors->Mark.Transparent = Dlg->SendMessage(DM_GETCHECK, ID_HER_MARKTRANSPARENT, nullptr) == BST_CHECKED;
			HighlightDlgUpdateUserControl(matrix_view(gdi.Item->VBuf, ColorExampleSize.y, ColorExampleSize.x), *Context->Colors);
			Dlg->SendMessage(DM_SETDLGITEM,ID_HER_COLOREXAMPLE,gdi.Item);
			return TRUE;
		}

	case DN_EDITCHANGE:
		if (Param1 == ID_HER_MARKEDIT)
			Dlg->SendMessage(DM_REFRESHCOLORS, 0, nullptr);
		break;

	case DN_CLOSE:

		if (Param1 == ID_FF_OK && Dlg->SendMessage(DM_GETCHECK, ID_FF_MATCHSIZE, nullptr))
		{
			string Size = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, ID_FF_SIZEFROMEDIT, nullptr));
			bool Ok = Size.empty() || CheckFileSizeStringFormat(Size);
			if (Ok)
			{
				Size = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, ID_FF_SIZETOEDIT, nullptr));
				Ok = Size.empty() || CheckFileSizeStringFormat(Size);
			}
			if (!Ok)
			{
				const auto Context = reinterpret_cast<const context*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
				Message(MSG_WARNING,
					msg(Context->Colors? lng::MFileHilightTitle : lng::MFileFilterTitle),
					{
						msg(lng::MBadFileSizeFormat)
					},
					{ lng::MOk });
				return FALSE;
			}
		}

		break;

	default:
		break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

bool FileFilterConfig(FileFilterParams *FF, bool ColorConfig)
{
	// Временная маска.
	filemasks FileMask;
	string strDateMask;
	// Определение параметров даты и времени в системе.
	wchar_t DateSeparator = locale.date_separator();
	wchar_t TimeSeparator = locale.time_separator();
	wchar_t DecimalSeparator = locale.decimal_separator();
	const auto DateFormat = locale.date_format();

	switch (DateFormat)
	{
	default:
	case date_type::ymd:
		// Маска даты для формата YYYYY.MM.DD
		strDateMask = format(FSTR(L"N9999{0}99{0}99"), DateSeparator);
		break;

	case date_type::mdy:
	case date_type::dmy:
		// Маска даты для форматов DD.MM.YYYYY и MM.DD.YYYYY
		strDateMask = format(FSTR(L"99{0}99{0}9999N"), DateSeparator);
		break;
	}

	// Маска времени
	const auto strTimeMask = format(FSTR(L"99{0}99{0}99{1}9999999"), TimeSeparator, DecimalSeparator);
	const wchar_t VerticalLine[] = {BoxSymbols[BS_T_H1V1],BoxSymbols[BS_V1],BoxSymbols[BS_V1],BoxSymbols[BS_V1],BoxSymbols[BS_B_H1V1],0};

	std::pair<lng, string> NameLabels[] =
	{
		{lng::MHighlightFileName1, {}},
		{lng::MHighlightFileName2, {}},
		{lng::MHighlightFileName3, {}},
		{lng::MHighlightFileName4, {}},
	};

	const auto ColumnSize = msg(std::max_element(ALL_CONST_RANGE(NameLabels), [](const auto& a, const auto& b) { return msg(a.first).size() < msg(b.first).size(); })->first).size() + 1;

	for (auto& [LngId, Label]: NameLabels)
	{
		Label = pad_right(msg(LngId), ColumnSize);
	}

	auto FilterDlg = MakeDialogItems<ID_FF_COUNT>(
	{
		{ DI_DOUBLEBOX,   {{3,  1 }, {76, 22}}, DIF_SHOWAMPERSAND, msg(ColorConfig? lng::MFileHilightTitle : lng::MFileFilterTitle), },
		{ DI_TEXT,        {{5,  2 }, {0,  2 }}, DIF_FOCUS, msg(lng::MFileFilterName), },
		{ DI_EDIT,        {{5,  2 }, {74, 2 }}, DIF_HISTORY, },
		{ DI_TEXT,        {{-1, 3 }, {0,  3 }}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,    {{5,  4 }, {0,  4 }}, DIF_AUTOMATION, msg(lng::MFileFilterMatchMask), },
		{ DI_EDIT,        {{5,  4 }, {74, 4 }}, DIF_HISTORY, },
		{ DI_TEXT,        {{-1, 5 }, {0,  5 }}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,    {{5,  6 }, {0,  6 }}, DIF_AUTOMATION, msg(lng::MFileFilterSize), },
		{ DI_TEXT,        {{5,  7 }, {8,  7 }}, DIF_RIGHTTEXT, msg(lng::MFileFilterSizeFromSign), },
		{ DI_EDIT,        {{10, 7 }, {20, 7 }}, DIF_NONE, },
		{ DI_TEXT,        {{5,  8 }, {8,  8 }}, DIF_RIGHTTEXT, msg(lng::MFileFilterSizeToSign), },
		{ DI_EDIT,        {{10, 8 }, {20, 8 }}, DIF_NONE, },
		{ DI_CHECKBOX,    {{24, 6 }, {0,  6 }}, DIF_AUTOMATION, msg(lng::MFileFilterDate), },
		{ DI_COMBOBOX,    {{24, 7 }, {38, 7 }}, DIF_DROPDOWNLIST | DIF_LISTNOAMPERSAND, },
		{ DI_CHECKBOX,    {{24, 8 }, {0,  8 }}, DIF_NONE, msg(lng::MFileFilterDateRelative), },
		{ DI_TEXT,        {{41, 7 }, {44, 7 }}, DIF_RIGHTTEXT, msg(lng::MFileFilterDateBeforeSign), },
		{ DI_FIXEDIT,     {{47, 7 }, {57, 7 }}, DIF_MASKEDIT, },
		{ DI_FIXEDIT,     {{47, 7 }, {57, 7 }}, DIF_MASKEDIT, },
		{ DI_FIXEDIT,     {{59, 7 }, {74, 7 }}, DIF_MASKEDIT, },
		{ DI_TEXT,        {{41, 8 }, {44, 8 }}, DIF_RIGHTTEXT, msg(lng::MFileFilterDateAfterSign), },
		{ DI_FIXEDIT,     {{47, 8 }, {57, 8 }}, DIF_MASKEDIT, },
		{ DI_FIXEDIT,     {{47, 8 }, {57, 8 }}, DIF_MASKEDIT, },
		{ DI_FIXEDIT,     {{59, 8 }, {74, 8 }}, DIF_MASKEDIT, },
		{ DI_BUTTON,      {{0,  6 }, {0,  6 }}, DIF_BTNNOCLOSE, msg(lng::MFileFilterCurrent), },
		{ DI_BUTTON,      {{0,  6 }, {74, 6 }}, DIF_BTNNOCLOSE, msg(lng::MFileFilterBlank), },
		{ DI_TEXT,        {{-1, 9 }, {0,  9 }}, DIF_SEPARATOR, },
		{ DI_VTEXT,       {{22, 5 }, {22, 9 }}, DIF_BOXCOLOR, VerticalLine },
		{ DI_CHECKBOX,    {{5,  10}, {0,  10}}, DIF_AUTOMATION, msg(lng::MFileFilterAttr)},
		{ DI_BUTTON,      {{5,  10}, {0,  10}}, DIF_AUTOMATION | DIF_BTNNOCLOSE, msg(lng::MFileFilterAttrChoose), },
		{ DI_CHECKBOX,    {{41, 10}, {0,  10}}, DIF_NONE, msg(lng::MFileHardLinksCount), },

		{ DI_TEXT,        {{-1, 12}, {0,  12}}, DIF_SEPARATOR, msg(lng::MHighlightColors), },
		{ DI_TEXT,        {{7,  13}, {0,  13}}, DIF_NONE,  msg(lng::MHighlightMarkChar), },
		{ DI_FIXEDIT,     {{5,  13}, {5,  13}}, DIF_NONE, },
		{ DI_CHECKBOX,    {{0,  13}, {0,  13}}, DIF_NONE, msg(lng::MHighlightTransparentMarkChar), },
		{ DI_BUTTON,      {{5,  14}, {0,  14}}, DIF_BTNNOCLOSE | DIF_NOBRACKETS, NameLabels[0].second, },
		{ DI_BUTTON,      {{0,  14}, {0,  14}}, DIF_BTNNOCLOSE | DIF_NOBRACKETS, msg(lng::MHighlightMarking1), },
		{ DI_BUTTON,      {{5,  15}, {0,  15}}, DIF_BTNNOCLOSE | DIF_NOBRACKETS, NameLabels[1].second, },
		{ DI_BUTTON,      {{0,  15}, {0,  15}}, DIF_BTNNOCLOSE | DIF_NOBRACKETS, msg(lng::MHighlightMarking2), },
		{ DI_BUTTON,      {{5,  16}, {0,  16}}, DIF_BTNNOCLOSE | DIF_NOBRACKETS, NameLabels[2].second, },
		{ DI_BUTTON,      {{0,  16}, {0,  16}}, DIF_BTNNOCLOSE | DIF_NOBRACKETS, msg(lng::MHighlightMarking3), },
		{ DI_BUTTON,      {{5,  17}, {0,  17}}, DIF_BTNNOCLOSE | DIF_NOBRACKETS, NameLabels[3].second, },
		{ DI_BUTTON,      {{0,  17}, {0,  17}}, DIF_BTNNOCLOSE | DIF_NOBRACKETS, msg(lng::MHighlightMarking4), },
		{ DI_USERCONTROL, {{57, 14}, {71, 17}}, DIF_NOFOCUS, },
		{ DI_CHECKBOX,    {{5,  19}, {0,  19}}, DIF_NONE, msg(lng::MHighlightContinueProcessing), },
		{ DI_TEXT,        {{-1, 20}, {0,  20}}, DIF_SEPARATOR, },
		{ DI_BUTTON,      {{0,  21}, {0,  21}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,      {{0,  21}, {0,  21}}, DIF_CENTERGROUP | DIF_BTNNOCLOSE, msg(lng::MFileFilterReset), },
		{ DI_BUTTON,      {{0,  21}, {0,  21}}, DIF_CENTERGROUP | DIF_BTNNOCLOSE, msg(lng::MFileFilterMakeTransparent), },
		{ DI_BUTTON,      {{0,  21}, {0,  21}}, DIF_CENTERGROUP, msg(lng::MFileFilterCancel), },
	});

	FilterDlg[ID_FF_NAMEEDIT].strHistory = L"FilterName"sv;
	FilterDlg[ID_FF_MASKEDIT].strHistory = L"FilterMasks"sv;

	FilterDlg[ID_FF_DATEBEFOREEDIT].strMask = FilterDlg[ID_FF_DATEAFTEREDIT].strMask = strDateMask;
	FilterDlg[ID_FF_TIMEBEFOREEDIT].strMask = FilterDlg[ID_FF_TIMEAFTEREDIT].strMask = strTimeMask;
	// Маска для ввода дней для относительной даты
	FilterDlg[ID_FF_DAYSBEFOREEDIT].strMask = FilterDlg[ID_FF_DAYSAFTEREDIT].strMask = L"9999"sv;

	if (!ColorConfig)
	{
		for (int i = ID_HER_SEPARATOR1; i <= ID_HER_CONTINUEPROCESSING; ++i)
			FilterDlg[i].Flags|=DIF_HIDDEN;

		const auto YDelta = FilterDlg[ID_FF_SEPARATOR5].Y2 - FilterDlg[ID_HER_SEPARATOR1].Y2 + 1;

		for (int i = ID_FF_SEPARATOR5; i <= ID_FF_CANCEL; ++i)
		{
			FilterDlg[i].Y1 -= YDelta;
			FilterDlg[i].Y2 -= YDelta;
		}

		FilterDlg[ID_FF_MAKETRANSPARENT].Flags |= DIF_HIDDEN;

		FilterDlg[ID_FF_TITLE].Y2 -= YDelta;
	}

	const auto AmpFixup = [&](size_t Index)
	{
		return contains(FilterDlg[Index].strData, L'&')? 1 : 0;
	};

	const auto GetPosAfter = [&](int const Id)
	{
		return FilterDlg[Id].X1 + FilterDlg[Id].strData.size() - AmpFixup(Id);
	};

	FilterDlg[ID_FF_NAMEEDIT].X1 = GetPosAfter(ID_FF_NAME) + 1;
	FilterDlg[ID_FF_MASKEDIT].X1 = GetPosAfter(ID_FF_MATCHMASK) + 5;

	FilterDlg[ID_FF_BLANK].X1 = FilterDlg[ID_FF_BLANK].X2 - FilterDlg[ID_FF_BLANK].strData.size() + AmpFixup(ID_FF_BLANK) - 3;
	FilterDlg[ID_FF_CURRENT].X2 = FilterDlg[ID_FF_BLANK].X1 - 2;
	FilterDlg[ID_FF_CURRENT].X1 = FilterDlg[ID_FF_CURRENT].X2 - FilterDlg[ID_FF_CURRENT].strData.size() + AmpFixup(ID_FF_CURRENT) - 3;

	FilterDlg[ID_FF_BUTTON_ATTRIBUTES].X1 = GetPosAfter(ID_FF_CHECKBOX_ATTRIBUTES) + 5;
	FilterDlg[ID_HER_MARKTRANSPARENT].X1 = GetPosAfter(ID_HER_MARK_TITLE) + 1;

	for (int i = ID_HER_NORMALMARKING; i <= ID_HER_SELECTEDCURSORMARKING; i += 2)
		FilterDlg[i].X1 = GetPosAfter(ID_HER_NORMALFILE) + 1;

	matrix<FAR_CHAR_INFO> VBufColorExample(ColorExampleSize.y, ColorExampleSize.x);
	auto Colors = FF->GetColors();
	HighlightDlgUpdateUserControl(VBufColorExample,Colors);
	FilterDlg[ID_HER_COLOREXAMPLE].VBuf = VBufColorExample.data();
	FilterDlg[ID_HER_MARKEDIT].strData.assign(Colors.Mark.Char ? 1 : 0, Colors.Mark.Char);
	FilterDlg[ID_HER_MARKTRANSPARENT].Selected = Colors.Mark.Transparent;
	FilterDlg[ID_HER_CONTINUEPROCESSING].Selected=(FF->GetContinueProcessing()?1:0);
	FilterDlg[ID_FF_NAMEEDIT].strData = FF->GetTitle();
	FilterDlg[ID_FF_MATCHMASK].Selected = FF->IsMaskUsed();
	FilterDlg[ID_FF_MASKEDIT].strData = FF->GetMask();

	if (!FilterDlg[ID_FF_MATCHMASK].Selected)
		FilterDlg[ID_FF_MASKEDIT].Flags|=DIF_DISABLE;

	FilterDlg[ID_FF_MATCHSIZE].Selected = FF->IsSizeUsed();
	FilterDlg[ID_FF_SIZEFROMEDIT].strData = FF->GetSizeAbove();
	FilterDlg[ID_FF_SIZETOEDIT].strData = FF->GetSizeBelow();
	FilterDlg[ID_FF_HARDLINKS].Selected=FF->GetHardLinks(nullptr,nullptr)?1:0; //пока что мы проверяем только флаг использования данного условия

	if (!FilterDlg[ID_FF_MATCHSIZE].Selected)
		for (int i=ID_FF_SIZEFROMSIGN; i <= ID_FF_SIZETOEDIT; i++)
			FilterDlg[i].Flags|=DIF_DISABLE;

	// Лист для комбобокса времени файла
	FarList DateList={sizeof(FarList)};
	FarListItem TableItemDate[FDATE_COUNT]={};
	// Настройка списка типов дат файла
	DateList.Items=TableItemDate;
	DateList.ItemsNumber=FDATE_COUNT;

	for (int i=0; i < FDATE_COUNT; ++i)
		TableItemDate[i].Text = msg(lng::MFileFilterWrited+i).c_str();

	DWORD DateType;
	filter_dates Dates;
	FilterDlg[ID_FF_MATCHDATE].Selected = FF->GetDate(&DateType, &Dates)? 1 : 0;
	FilterDlg[ID_FF_DATETYPE].ListItems=&DateList;
	TableItemDate[DateType].Flags=LIF_SELECTED;

	const auto ProcessDuration = [&](auto Duration, auto DateId, auto TimeId)
	{
		FilterDlg[ID_FF_DATERELATIVE].Selected = BSTATE_CHECKED;
		std::tie(FilterDlg[DateId].strData, FilterDlg[TimeId].strData) = ConvertDuration(Duration);
	};

	const auto ProcessPoint = [&](auto Point, auto DateId, auto TimeId)
	{
		FilterDlg[ID_FF_DATERELATIVE].Selected = BSTATE_UNCHECKED;
		ConvertDate(Point, FilterDlg[DateId].strData, FilterDlg[TimeId].strData, 16, 2);
	};

	Dates.visit(overload
	{
		[&](os::chrono::duration After, os::chrono::duration Before)
		{
			ProcessDuration(After, ID_FF_DAYSAFTEREDIT, ID_FF_TIMEAFTEREDIT);
			ProcessDuration(Before, ID_FF_DAYSBEFOREEDIT, ID_FF_TIMEBEFOREEDIT);
		},
		[&](os::chrono::time_point After, os::chrono::time_point Before)
		{
			ProcessPoint(After, ID_FF_DATEAFTEREDIT, ID_FF_TIMEAFTEREDIT);
			ProcessPoint(Before, ID_FF_DATEBEFOREEDIT, ID_FF_TIMEBEFOREEDIT);
		}
	});

	if (!FilterDlg[ID_FF_MATCHDATE].Selected)
		for (int i=ID_FF_DATETYPE; i <= ID_FF_BLANK; i++)
			FilterDlg[i].Flags|=DIF_DISABLE;

	attribute_map AttributeMapping[]
	{
		{ FILE_ATTRIBUTE_READONLY,                     lng::MFileFilterAttrReadOnly,           },
		{ FILE_ATTRIBUTE_ARCHIVE,                      lng::MFileFilterAttrArcive,             },
		{ FILE_ATTRIBUTE_HIDDEN,                       lng::MFileFilterAttrHidden,             },
		{ FILE_ATTRIBUTE_SYSTEM,                       lng::MFileFilterAttrSystem,             },
		{ FILE_ATTRIBUTE_COMPRESSED,                   lng::MFileFilterAttrCompressed,         },
		{ FILE_ATTRIBUTE_ENCRYPTED,                    lng::MFileFilterAttrEncrypted,          },
		{ FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,          lng::MFileFilterAttrNotIndexed,         },
		{ FILE_ATTRIBUTE_DIRECTORY,                    lng::MFileFilterAttrDirectory,          },
		{ FILE_ATTRIBUTE_SPARSE_FILE,                  lng::MFileFilterAttrSparse,             },
		{ FILE_ATTRIBUTE_TEMPORARY,                    lng::MFileFilterAttrTemporary,          },
		{ FILE_ATTRIBUTE_OFFLINE,                      lng::MFileFilterAttrOffline,            },
		{ FILE_ATTRIBUTE_REPARSE_POINT,                lng::MFileFilterAttrReparse,            },
		{ FILE_ATTRIBUTE_VIRTUAL,                      lng::MFileFilterAttrVirtual,            },
		{ FILE_ATTRIBUTE_INTEGRITY_STREAM,             lng::MFileFilterAttrIntegrityStream,    },
		{ FILE_ATTRIBUTE_NO_SCRUB_DATA,                lng::MFileFilterAttrNoScrubData,        },
		{ FILE_ATTRIBUTE_PINNED,                       lng::MFileFilterAttrPinned,             },
		{ FILE_ATTRIBUTE_UNPINNED,                     lng::MFileFilterAttrUnpinned,           },
		{ FILE_ATTRIBUTE_RECALL_ON_OPEN,               lng::MFileFilterAttrRecallOnOpen,       },
		{ FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS,        lng::MFileFilterAttrRecallOnDataAccess, },
		{ FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL,          lng::MFileFilterAttrStrictlySequential, },
	};

	os::fs::attributes AttrSet, AttrClear;
	FilterDlg[ID_FF_CHECKBOX_ATTRIBUTES].Selected = FF->GetAttr(&AttrSet,&AttrClear)? BSTATE_CHECKED : BSTATE_UNCHECKED;

	for (auto& i: AttributeMapping)
	{
		i.State = AttrSet & i.Attribute? BSTATE_CHECKED : AttrClear & i.Attribute? BSTATE_UNCHECKED : BSTATE_3STATE;
	}

	if (!FilterDlg[ID_FF_CHECKBOX_ATTRIBUTES].Selected)
	{
		FilterDlg[ID_FF_BUTTON_ATTRIBUTES].Flags |= DIF_DISABLE;
	}

	context Context{ ColorConfig? &Colors : nullptr, AttributeMapping };

	const auto Dlg = Dialog::create(FilterDlg, FileFilterConfigDlgProc, &Context);
	Dlg->SetHelp(ColorConfig? L"HighlightEdit"sv : L"Filter"sv);
	Dlg->SetPosition({ -1, -1, static_cast<int>(FilterDlg[ID_FF_TITLE].X2 + 4), static_cast<int>(FilterDlg[ID_FF_TITLE].Y2 + 2) });
	Dlg->SetId(ColorConfig?HighlightConfigId:FiltersConfigId);
	Dlg->SetAutomation(ID_FF_MATCHMASK,ID_FF_MASKEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROMSIGN,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROMEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETOSIGN,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETOEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_DATETYPE,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_DATERELATIVE,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTERSIGN,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTEREDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_DAYSAFTEREDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEAFTEREDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFORESIGN,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFOREEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_DAYSBEFOREEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEBEFOREEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_CURRENT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg->SetAutomation(ID_FF_MATCHDATE,ID_FF_BLANK,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);

	Dlg->SetAutomation(ID_FF_CHECKBOX_ATTRIBUTES, ID_FF_BUTTON_ATTRIBUTES, DIF_DISABLE, DIF_NONE, DIF_NONE, DIF_DISABLE);

	for (;;)
	{
		Dlg->ClearDone();
		Dlg->Process();
		int ExitCode=Dlg->GetExitCode();

		if (ExitCode==ID_FF_OK) // Ok
		{
			// Если введённая пользователем маска не корректна, тогда вернёмся в диалог
			if (FilterDlg[ID_FF_MATCHMASK].Selected && !FileMask.assign(FilterDlg[ID_FF_MASKEDIT].strData, 0))
				continue;

			Colors.Mark.Transparent = FilterDlg[ID_HER_MARKTRANSPARENT].Selected == BSTATE_CHECKED;

			FF->SetColors(Colors);
			FF->SetContinueProcessing(FilterDlg[ID_HER_CONTINUEPROCESSING].Selected!=0);
			FF->SetTitle(FilterDlg[ID_FF_NAMEEDIT].strData);
			FF->SetMask(FilterDlg[ID_FF_MATCHMASK].Selected!=0,
			            FilterDlg[ID_FF_MASKEDIT].strData);
			FF->SetSize(FilterDlg[ID_FF_MATCHSIZE].Selected!=0,
			            FilterDlg[ID_FF_SIZEFROMEDIT].strData,
			            FilterDlg[ID_FF_SIZETOEDIT].strData);
			FF->SetHardLinks(FilterDlg[ID_FF_HARDLINKS].Selected!=0,0,0); //пока устанавливаем только флаг использования признака
			const auto IsRelative = FilterDlg[ID_FF_DATERELATIVE].Selected!=0;

			FilterDlg[ID_FF_TIMEBEFOREEDIT].strData[8] = TimeSeparator;
			FilterDlg[ID_FF_TIMEAFTEREDIT].strData[8] = TimeSeparator;

			const auto NewDates = IsRelative?
				filter_dates
				(
					ParseDuration(FilterDlg[ID_FF_DAYSAFTEREDIT].strData, FilterDlg[ID_FF_TIMEAFTEREDIT].strData),
					ParseDuration(FilterDlg[ID_FF_DAYSBEFOREEDIT].strData, FilterDlg[ID_FF_TIMEBEFOREEDIT].strData)
				) :
				filter_dates
				(
					ParseTimePoint(FilterDlg[ID_FF_DATEAFTEREDIT].strData, FilterDlg[ID_FF_TIMEAFTEREDIT].strData, static_cast<int>(DateFormat)),
					ParseTimePoint(FilterDlg[ID_FF_DATEBEFOREEDIT].strData, FilterDlg[ID_FF_TIMEBEFOREEDIT].strData, static_cast<int>(DateFormat))
				);

			FF->SetDate(FilterDlg[ID_FF_MATCHDATE].Selected != 0, static_cast<enumFDateType>(FilterDlg[ID_FF_DATETYPE].ListPos), NewDates);
			AttrSet=0;
			AttrClear=0;

			for (const auto& i: AttributeMapping)
			{
				switch (i.State)
				{
				case BSTATE_CHECKED:
					AttrSet |= i.Attribute;
					break;

				case BSTATE_UNCHECKED:
					AttrClear |= i.Attribute;
					break;

				default:
					break;
				}
			}

			FF->SetAttr(FilterDlg[ID_FF_CHECKBOX_ATTRIBUTES].Selected == BSTATE_CHECKED, AttrSet, AttrClear);
			return true;
		}
		else
			break;
	}

	return false;
}
