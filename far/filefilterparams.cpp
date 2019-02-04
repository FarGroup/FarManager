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

#include "filefilterparams.hpp"

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
#include "DlgGuid.hpp"

#include "platform.fs.hpp"

#include "common/zip_view.hpp"

#include "format.hpp"

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
	return m_After != m_After.zero() || m_Before != m_Before.zero();
}


FileFilterParams::FileFilterParams()
{
	SetMask(true, L"*"sv);

	std::for_each(RANGE(FHighlight.Colors.Color, i)
	{
		colors::make_opaque(i.FileColor.ForegroundColor);
		colors::make_opaque(i.MarkColor.ForegroundColor);
	});

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
		FMask.FilterMask.Set(FMask.strMask, FMF_SILENT);
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

void FileFilterParams::SetAttr(bool const Used, DWORD const AttrSet, DWORD const AttrClear)
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


bool FileFilterParams::GetAttr(DWORD *AttrSet, DWORD *AttrClear) const
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
	DWORD Attributes;
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

	return FileInFilter(FilterObject, CurrentTime, [&](){ return Object.NumberOfLinks(Owner); });
}

bool FileFilterParams::FileInFilter(const os::fs::find_data& Object, os::chrono::time_point CurrentTime,const string* FullName) const
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

	return FileInFilter(FilterObject, CurrentTime, [&](){ return FullName? GetNumberOfLinks(*FullName) : 1; });
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
		static_cast<DWORD>(Object.FileAttributes),
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
		if (!FSize.Above.Size.empty())
		{
			if (Object.Size < FSize.Above.SizeReal) // Размер файла меньше минимального разрешённого по фильтру?
				return false;                          // Не пропускаем этот файл
		}

		if (!FSize.Below.Size.empty())
		{
			if (Object.Size > FSize.Below.SizeReal) // Размер файла больше максимального разрешённого по фильтру?
				return false;                          // Не пропускаем этот файл
		}
	}

	// Режим проверки времени файла включен?
	if (FDate.Used)
	{
		// Есть введённая пользователем начальная / конечная дата?
		if (FDate.Dates)
		{
			const os::chrono::time_point* ft = nullptr;

			switch (FDate.DateType)
			{
			case FDATE_CREATED:
				ft = &Object.CreationTime;
				break;
			case FDATE_OPENED:
				ft = &Object.AccessTime;
				break;
			case FDATE_CHANGED:
				ft = &Object.ChangeTime;
				break;
			case FDATE_MODIFIED:
				ft = &Object.ModificationTime;
				break;

				// dummy label to make compiler happy
			case FDATE_COUNT:
				break;
			}

			if (ft)
			{
				// Дата файла меньше начальной / больше конечной даты по фильтру?
				if (FDate.Dates.visit(overload
				(
					[&](os::chrono::duration After, os::chrono::duration Before)
					{
						return (After != After.zero() && *ft < CurrentTime - After) || (Before != Before.zero() && *ft > CurrentTime - Before);
					},
					[&](os::chrono::time_point After, os::chrono::time_point Before)
					{
						return (After != os::chrono::time_point{} && *ft < After) || (Before != os::chrono::time_point{} && *ft > Before);
					}
				)))
				{
					// Не пропускаем этот файл
					return false;
				}
			}
		}
	}

	// Режим проверки маски файла включен?
	if (FMask.Used)
	{
		// ЭТО ЕСТЬ УЗКОЕ МЕСТО ДЛЯ СКОРОСТНЫХ ХАРАКТЕРИСТИК Far Manager
		// при считывании директории

		// Файл не попадает под маску введённую в фильтре?
		if (!FMask.FilterMask.Compare(Object.Name))
		// Не пропускаем этот файл
			return false;
	}

	// Режим проверки количества жестких ссылок на файл включен?
	// Пока что, при включенном условии, срабатывание происходит при случае "ссылок больше чем одна"
	if (FHardLinks.Used)
	{
		if (Object.Attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			return false;
		}

		const auto NumberOfLinks = HardlinkGetter? HardlinkGetter() : Object.NumberOfLinks;

		if (NumberOfLinks < 2)
		{
			return false;
		}
	}

	// Да! Файл выдержал все испытания и будет допущен к использованию
	// в вызвавшей эту функцию операции.
	return true;
}

//Централизованная функция для создания строк меню различных фильтров.
string MenuString(const FileFilterParams* const FF, bool const bHighlightType, wchar_t const Hotkey, bool const bPanelType, string_view const FMask, string_view const Title)
{
	string strDest;

	const wchar_t DownArrow = L'\x2193';
	string_view Name;
	auto Mask = L""sv;
	auto MarkChar = L"' '"s;
	DWORD IncludeAttr, ExcludeAttr;
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
			MarkChar[1] = Char;
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
		(
			[&](os::chrono::duration, os::chrono::duration) { RelativeDate = true; },
			[&](os::chrono::time_point, os::chrono::time_point) { RelativeDate = false; }
		));
		UseHardLinks=FF->GetHardLinks(nullptr,nullptr);
	}

	string Attr;

	enum_attributes([&](DWORD Attribute, wchar_t Character)
	{
		if (IncludeAttr & Attribute)
		{
			Attr.push_back(Character);
			Attr.push_back(L'+');
		}
		else if (ExcludeAttr & Attribute)
		{
			Attr.push_back(Character);
			Attr.push_back(L'-');
		}
		else
		{
			Attr.push_back(L'.');
			Attr.push_back(L'.');
		}
		return true;
	});

	wchar_t SizeDate[] = L"....";

	if (UseSize)
	{
		SizeDate[0]=L'S';
	}

	if (UseDate)
	{
		if (RelativeDate)
			SizeDate[1]=L'R';
		else
			SizeDate[1]=L'D';
	}

	if (UseHardLinks)
	{
		SizeDate[2]=L'H';
	}

	if (bHighlightType)
	{
		if (FF->GetContinueProcessing())
			SizeDate[3]=DownArrow;
		strDest = format(L"{1:3} {0} {2} {3} {0} {4}"sv, BoxSymbols[BS_V1], MarkChar, Attr, SizeDate, Mask);
	}
	else
	{
		SizeDate[3]=0;

		if (!Hotkey && !bPanelType)
		{
			strDest = format(L"{1:{2}.{2}} {0} {3} {4} {0} {5}"sv, BoxSymbols[BS_V1], Name, 21 + (contains(Name, L'&')? 1 : 0), Attr, SizeDate, Mask);
		}
		else
		{
			if (Hotkey)
				strDest = format(L"&{1}. {2:18.18} {0} {3} {4} {0} {5}"sv, BoxSymbols[BS_V1], Hotkey, Name, Attr, SizeDate, Mask);
			else
				strDest = format(L"   {1:18.18} {0} {2} {3} {0} {4}"sv, BoxSymbols[BS_V1], Name, Attr, SizeDate, Mask);
		}
	}

	inplace::trim_right(strDest);
	return strDest;
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

	ID_FF_MATCHATTRIBUTES,
	ID_FF_READONLY,
	ID_FF_ARCHIVE,
	ID_FF_HIDDEN,
	ID_FF_SYSTEM,
	ID_FF_COMPRESSED,
	ID_FF_ENCRYPTED,
	ID_FF_NOTINDEXED,
	ID_FF_DIRECTORY,
	ID_FF_SPARSE,
	ID_FF_TEMP,
	ID_FF_OFFLINE,
	ID_FF_REPARSEPOINT,
	ID_FF_VIRTUAL,
	ID_FF_INTEGRITY_STREAM,
	ID_FF_NO_SCRUB_DATA,

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

	ID_FF_HARDLINKS,

	ID_FF_SEPARATOR6,

	ID_FF_OK,
	ID_FF_RESET,
	ID_FF_CANCEL,
	ID_FF_MAKETRANSPARENT,
};

static void HighlightDlgUpdateUserControl(FAR_CHAR_INFO *VBufColorExample, const highlight::element &Colors)
{
	const PaletteColors DefaultColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};
	int VBufRow = 0;

	for (const auto& i: zip(Colors.Color, DefaultColor))
	{
		auto& CurColor= std::get<0>(i);
		const auto pal = std::get<1>(i);

		auto Color = CurColor.FileColor;
		const auto BaseColor = colors::PaletteColorToFarColor(pal);

		// Black-on-black = default
		// It's for a preview, so we don't care about the extended flags here
		Color = !colors::color_value(Color.BackgroundColor) && !colors::color_value(Color.ForegroundColor)?
			BaseColor :
			colors::merge(BaseColor, Color);

		const auto Str = concat(BoxSymbols[BS_V2], Colors.Mark.Char? L" "sv : L""sv, fit_to_left(msg(lng::MHighlightExample), Colors.Mark.Char? 12 : 13), BoxSymbols[BS_V1]);

		for (int k=0; k<15; k++)
		{
			VBufColorExample[15*VBufRow+k].Char = Str[k];
			VBufColorExample[15*VBufRow+k].Attributes=Color;
		}

		if (Colors.Mark.Char)
		{
			// inherit only color mode, not style
			VBufColorExample[15*VBufRow+1].Attributes.Flags = Color.Flags&FCF_4BITMASK;
			VBufColorExample[15*VBufRow+1].Char = Colors.Mark.Char;
			if (colors::color_value(CurColor.MarkColor.ForegroundColor) || colors::color_value(CurColor.MarkColor.BackgroundColor))
			{
				VBufColorExample[15 * VBufRow + 1].Attributes = CurColor.MarkColor;
			}
			else
			{
				// apply all except color mode
				VBufColorExample[15 * VBufRow + 1].Attributes.Flags |= CurColor.MarkColor.Flags & FCF_EXTENDEDFLAGS;
			}
		}

		VBufColorExample[15 * VBufRow].Attributes = VBufColorExample[15 * VBufRow + 14].Attributes = colors::PaletteColorToFarColor(COL_PANELBOX);
		++VBufRow;
	}
}

static void FilterDlgRelativeDateItemsUpdate(Dialog* Dlg, bool bClear)
{
	Dlg->SendMessage(DM_ENABLEREDRAW, FALSE, nullptr);

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

	Dlg->SendMessage(DM_ENABLEREDRAW, TRUE, nullptr);
}

static intptr_t FileFilterConfigDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
	case DN_INITDIALOG:
		{
			FilterDlgRelativeDateItemsUpdate(Dlg, false);
			return TRUE;
		}
	case DN_BTNCLICK:
		{
			if (Param1==ID_FF_CURRENT || Param1==ID_FF_BLANK) //Current и Blank
			{
				string strDate, strTime;

				if (Param1==ID_FF_CURRENT)
				{
					ConvertDate(os::chrono::nt_clock::now(), strDate, strTime, 12, FALSE, FALSE, 2);
				}
				else
				{
					strDate.clear();
					strTime.clear();
				}

				Dlg->SendMessage(DM_ENABLEREDRAW, FALSE, nullptr);
				const auto relative = Dlg->SendMessage(DM_GETCHECK, ID_FF_DATERELATIVE, nullptr) != 0;
				const auto db = relative? ID_FF_DAYSBEFOREEDIT : ID_FF_DATEBEFOREEDIT;
				const auto da = relative? ID_FF_DAYSAFTEREDIT  : ID_FF_DATEAFTEREDIT;
				Dlg->SendMessage(DM_SETTEXTPTR,da, UNSAFE_CSTR(strDate));
				Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT, UNSAFE_CSTR(strTime));
				Dlg->SendMessage(DM_SETTEXTPTR,db, UNSAFE_CSTR(strDate));
				Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT, UNSAFE_CSTR(strTime));
				Dlg->SendMessage(DM_SETFOCUS, da, nullptr);
				COORD r;
				r.X=r.Y=0;
				Dlg->SendMessage(DM_SETCURSORPOS,da,&r);
				Dlg->SendMessage(DM_ENABLEREDRAW, TRUE, nullptr);
				break;
			}
			else if (Param1==ID_FF_RESET) // Reset
			{
				Dlg->SendMessage(DM_ENABLEREDRAW, FALSE, nullptr);
				intptr_t ColorConfig = Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr);
				Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_MASKEDIT,const_cast<wchar_t*>(L"*"));
				Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_SIZEFROMEDIT,nullptr);
				Dlg->SendMessage(DM_SETTEXTPTR,ID_FF_SIZETOEDIT,nullptr);

				for (int I=ID_FF_READONLY; I < ID_HER_SEPARATOR1 ; ++I)
				{
					Dlg->SendMessage(DM_SETCHECK,I,ToPtr(BSTATE_3STATE));
				}

				if (!ColorConfig)
					Dlg->SendMessage(DM_SETCHECK,ID_FF_DIRECTORY,ToPtr(BSTATE_UNCHECKED));

				FarListPos LPos={sizeof(FarListPos)};
				Dlg->SendMessage(DM_LISTSETCURPOS,ID_FF_DATETYPE,&LPos);
				Dlg->SendMessage(DM_SETCHECK,ID_FF_MATCHMASK,ToPtr(BSTATE_CHECKED));
				Dlg->SendMessage(DM_SETCHECK,ID_FF_MATCHSIZE,ToPtr(BSTATE_UNCHECKED));
				Dlg->SendMessage(DM_SETCHECK,ID_FF_HARDLINKS,ToPtr(BSTATE_UNCHECKED));
				Dlg->SendMessage(DM_SETCHECK,ID_FF_MATCHDATE,ToPtr(BSTATE_UNCHECKED));
				Dlg->SendMessage(DM_SETCHECK,ID_FF_DATERELATIVE,ToPtr(BSTATE_UNCHECKED));
				FilterDlgRelativeDateItemsUpdate(Dlg, true);
				Dlg->SendMessage(DM_SETCHECK,ID_FF_MATCHATTRIBUTES,ToPtr(ColorConfig?BSTATE_UNCHECKED:BSTATE_CHECKED));
				Dlg->SendMessage(DM_ENABLEREDRAW, TRUE, nullptr);
				break;
			}
			else if (Param1==ID_FF_MAKETRANSPARENT)
			{
				const auto Colors = reinterpret_cast<highlight::element*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

				std::for_each(RANGE(Colors->Color, i)
				{
					colors::make_transparent(i.FileColor.ForegroundColor);
					colors::make_transparent(i.FileColor.BackgroundColor);
					colors::make_transparent(i.MarkColor.ForegroundColor);
					colors::make_transparent(i.MarkColor.BackgroundColor);
				});

				Dlg->SendMessage(DM_SETCHECK,ID_HER_MARKTRANSPARENT,ToPtr(BSTATE_CHECKED));
				break;
			}
			else if (Param1==ID_FF_DATERELATIVE)
			{
				FilterDlgRelativeDateItemsUpdate(Dlg, true);
				break;
			}
		}
		[[fallthrough]];
	case DN_CONTROLINPUT:

		if ((Msg==DN_BTNCLICK && Param1 >= ID_HER_NORMALFILE && Param1 <= ID_HER_SELECTEDCURSORMARKING)
			|| (Msg==DN_CONTROLINPUT && Param1==ID_HER_COLOREXAMPLE && ((INPUT_RECORD *)Param2)->EventType == MOUSE_EVENT && ((INPUT_RECORD *)Param2)->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED))
		{
			const auto EditData = reinterpret_cast<highlight::element*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

			if (Msg==DN_CONTROLINPUT)
			{
				Param1 = ID_HER_NORMALFILE + ((INPUT_RECORD *)Param2)->Event.MouseEvent.dwMousePosition.Y*2;

				if (((INPUT_RECORD *)Param2)->Event.MouseEvent.dwMousePosition.X==1 && (EditData->Mark.Char))
					Param1 = ID_HER_NORMALMARKING + ((INPUT_RECORD *)Param2)->Event.MouseEvent.dwMousePosition.Y*2;
			}

			//Color[0=file, 1=mark][0=normal,1=selected,2=undercursor,3=selectedundercursor]
			static const PaletteColors BaseIndices[]{ COL_PANELTEXT, COL_PANELSELECTEDTEXT, COL_PANELCURSOR, COL_PANELSELECTEDCURSOR };
			const auto BaseColor = colors::PaletteColorToFarColor(BaseIndices[(Param1 - ID_HER_NORMALFILE) / 2]);

			console.GetColorDialog(((Param1-ID_HER_NORMALFILE)&1)? EditData->Color[(Param1-ID_HER_NORMALFILE)/2].MarkColor : EditData->Color[(Param1-ID_HER_NORMALFILE)/2].FileColor, true, &BaseColor);

			size_t Size = Dlg->SendMessage(DM_GETDLGITEM, ID_HER_COLOREXAMPLE, nullptr);
			block_ptr<FarDialogItem> Buffer(Size);
			FarGetDialogItem gdi = {sizeof(FarGetDialogItem), Size, Buffer.get()};
			Dlg->SendMessage(DM_GETDLGITEM,ID_HER_COLOREXAMPLE,&gdi);
			//MarkChar это FIXEDIT размером в 1 символ
			wchar_t MarkChar[2];
			FarDialogItemData item={sizeof(FarDialogItemData),1,MarkChar};
			Dlg->SendMessage(DM_GETTEXT,ID_HER_MARKEDIT,&item);
			EditData->Mark.Char = *MarkChar;
			HighlightDlgUpdateUserControl(gdi.Item->VBuf,*EditData);
			Dlg->SendMessage(DM_SETDLGITEM,ID_HER_COLOREXAMPLE,gdi.Item);
			return TRUE;
		}

		break;
	case DN_EDITCHANGE:

		if (Param1 == ID_HER_MARKEDIT)
		{
			const auto EditData = reinterpret_cast<highlight::element*>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
			size_t Size = Dlg->SendMessage(DM_GETDLGITEM, ID_HER_COLOREXAMPLE, nullptr);
			block_ptr<FarDialogItem> Buffer(Size);
			FarGetDialogItem gdi = {sizeof(FarGetDialogItem), Size, Buffer.get()};
			Dlg->SendMessage(DM_GETDLGITEM,ID_HER_COLOREXAMPLE,&gdi);
			//MarkChar это FIXEDIT размером в 1 символ
			wchar_t MarkChar[2];
			FarDialogItemData item={sizeof(FarDialogItemData),1,MarkChar};
			Dlg->SendMessage(DM_GETTEXT,ID_HER_MARKEDIT,&item);
			EditData->Mark.Char = *MarkChar;
			HighlightDlgUpdateUserControl(gdi.Item->VBuf,*EditData);
			Dlg->SendMessage(DM_SETDLGITEM,ID_HER_COLOREXAMPLE,gdi.Item);
			return TRUE;
		}

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
				const auto ColorConfig = Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr);
				Message(MSG_WARNING,
					msg(ColorConfig? lng::MFileHilightTitle : lng::MFileFilterTitle),
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
	// История для маски файлов
	const wchar_t FilterMasksHistoryName[] = L"FilterMasks";
	// История для имени фильтра
	const wchar_t FilterNameHistoryName[] = L"FilterName";
	// Маски для диалога настройки
	// Маска для ввода дней для относительной даты
	const wchar_t DaysMask[] = L"9999";
	string strDateMask;
	// Определение параметров даты и времени в системе.
	wchar_t DateSeparator = locale.date_separator();
	wchar_t TimeSeparator = locale.time_separator();
	wchar_t DecimalSeparator = locale.decimal_separator();
	const auto DateFormat = locale.date_format();
	date_ranges DateRanges;

	switch (DateFormat)
	{
	case 0:
	case 1:
		// Маска даты для форматов DD.MM.YYYYY и MM.DD.YYYYY
		strDateMask = format(L"99{0}99{0}9999N"sv, DateSeparator);
		DateRanges = {{ { 0, 2 }, { 3, 2 }, { 6, 5 } }};
		break;

	default:
		// Маска даты для формата YYYYY.MM.DD
		strDateMask = format(L"N9999{0}99{0}99"sv, DateSeparator);
		DateRanges = {{ { 0, 5 }, { 6, 2 }, { 9, 2 } }};
		break;
	}

	// Маска времени
	const auto strTimeMask = format(L"99{0}99{0}99{1}999"sv, TimeSeparator, DecimalSeparator);
	const time_ranges TimeRanges{{ {0, 2}, {3, 2}, {6, 2}, {9, 3} }};
	const wchar_t VerticalLine[] = {BoxSymbols[BS_T_H1V1],BoxSymbols[BS_V1],BoxSymbols[BS_V1],BoxSymbols[BS_V1],BoxSymbols[BS_B_H1V1],0};

	std::pair<lng, string> NameLabels[] =
	{
		{lng::MHighlightFileName1, {}},
		{lng::MHighlightFileName2, {}},
		{lng::MHighlightFileName3, {}},
		{lng::MHighlightFileName4, {}},
	};

	const auto ColumnSize = msg(std::max_element(ALL_CONST_RANGE(NameLabels), [](const auto& a, const auto& b) { return msg(a.first).size() < msg(b.first).size(); })->first).size() + 1;

	for (auto& i: NameLabels)
	{
		i.second = pad_right(msg(i.first), ColumnSize);
	}

	FarDialogItem FilterDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,76,23,0,nullptr,nullptr,DIF_SHOWAMPERSAND,msg(lng::MFileFilterTitle).c_str()},

		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,DIF_FOCUS,msg(lng::MFileFilterName).c_str()},
		{DI_EDIT,5,2,74,2,0,FilterNameHistoryName,nullptr,DIF_HISTORY,L""},

		{DI_TEXT,-1,3,0,3,0,nullptr,nullptr,DIF_SEPARATOR,L""},

		{DI_CHECKBOX,5,4,0,4,0,nullptr,nullptr,DIF_AUTOMATION,msg(lng::MFileFilterMatchMask).c_str()},
		{DI_EDIT,5,4,74,4,0,FilterMasksHistoryName,nullptr,DIF_HISTORY,L""},

		{DI_TEXT,-1,5,0,5,0,nullptr,nullptr,DIF_SEPARATOR,L""},

		{DI_CHECKBOX,5,6,0,6,0,nullptr,nullptr,DIF_AUTOMATION,msg(lng::MFileFilterSize).c_str()},
		{DI_TEXT,7,7,8,7,0,nullptr,nullptr,0,msg(lng::MFileFilterSizeFromSign).c_str()},
		{DI_EDIT,10,7,20,7,0,nullptr,nullptr,0,L""},
		{DI_TEXT,7,8,8,8,0,nullptr,nullptr,0,msg(lng::MFileFilterSizeToSign).c_str()},
		{DI_EDIT,10,8,20,8,0,nullptr,nullptr,0,L""},

		{DI_CHECKBOX,24,6,0,6,0,nullptr,nullptr,DIF_AUTOMATION,msg(lng::MFileFilterDate).c_str()},
		{DI_COMBOBOX,26,7,41,7,0,nullptr,nullptr,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,L""},
		{DI_CHECKBOX,26,8,0,8,0,nullptr,nullptr,0,msg(lng::MFileFilterDateRelative).c_str()},
		{DI_TEXT,48,7,50,7,0,nullptr,nullptr,0,msg(lng::MFileFilterDateBeforeSign).c_str()},
		{DI_FIXEDIT,51,7,61,7,0,nullptr,strDateMask.c_str(),DIF_MASKEDIT,L""},
		{DI_FIXEDIT,51,7,61,7,0,nullptr,DaysMask,DIF_MASKEDIT,L""},
		{DI_FIXEDIT,63,7,74,7,0,nullptr,strTimeMask.c_str(),DIF_MASKEDIT,L""},
		{DI_TEXT,48,8,50,8,0,nullptr,nullptr,0,msg(lng::MFileFilterDateAfterSign).c_str()},
		{DI_FIXEDIT,51,8,61,8,0,nullptr,strDateMask.c_str(),DIF_MASKEDIT,L""},
		{DI_FIXEDIT,51,8,61,8,0,nullptr,DaysMask,DIF_MASKEDIT,L""},
		{DI_FIXEDIT,63,8,74,8,0,nullptr,strTimeMask.c_str(),DIF_MASKEDIT,L""},
		{DI_BUTTON,0,6,0,6,0,nullptr,nullptr,DIF_BTNNOCLOSE,msg(lng::MFileFilterCurrent).c_str()},
		{DI_BUTTON,0,6,74,6,0,nullptr,nullptr,DIF_BTNNOCLOSE,msg(lng::MFileFilterBlank).c_str()},

		{DI_TEXT,-1,9,0,9,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_VTEXT,22,5,22,9,0,nullptr,nullptr,DIF_BOXCOLOR,VerticalLine},

		{DI_CHECKBOX, 5,10,0,10,0,nullptr,nullptr,DIF_AUTOMATION,msg(lng::MFileFilterAttr).c_str()},
		{DI_CHECKBOX, 7,11,0,11,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrR).c_str()},
		{DI_CHECKBOX, 7,12,0,12,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrA).c_str()},
		{DI_CHECKBOX, 7,13,0,13,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrH).c_str()},
		{DI_CHECKBOX, 7,14,0,14,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrS).c_str()},
		{DI_CHECKBOX, 7,15,0,15,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrC).c_str()},
		{DI_CHECKBOX, 7,16,0,16,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrE).c_str()},
		{DI_CHECKBOX, 7,17,0,17,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrNI).c_str()},
		{DI_CHECKBOX, 7,18,0,18,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrD).c_str()},

		{DI_CHECKBOX,42,11,0,11,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrSparse).c_str()},
		{DI_CHECKBOX,42,12,0,12,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrT).c_str()},
		{DI_CHECKBOX,42,13,0,13,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrOffline).c_str()},
		{DI_CHECKBOX,42,14,0,14,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrReparse).c_str()},
		{DI_CHECKBOX,42,15,0,15,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrVirtual).c_str()},
		{DI_CHECKBOX,42,16,0,16,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrIntegrityStream).c_str()},
		{DI_CHECKBOX,42,17,0,17,0,nullptr,nullptr,DIF_3STATE,msg(lng::MFileFilterAttrNoScrubData).c_str()},

		{DI_TEXT,-1,17,0,17,0,nullptr,nullptr,DIF_SEPARATOR,msg(lng::MHighlightColors).c_str()},
		{DI_TEXT,7,18,0,18,0,nullptr,nullptr,0,msg(lng::MHighlightMarkChar).c_str()},
		{DI_FIXEDIT,5,18,5,18,0,nullptr,nullptr,0,L""},
		{DI_CHECKBOX,0,18,0,18,0,nullptr,nullptr,0,msg(lng::MHighlightTransparentMarkChar).c_str()},

		{DI_BUTTON,5,19,0,19,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,NameLabels[0].second.c_str()},
		{DI_BUTTON,0,19,0,19,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,msg(lng::MHighlightMarking1).c_str()},
		{DI_BUTTON,5,20,0,20,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,NameLabels[1].second.c_str()},
		{DI_BUTTON,0,20,0,20,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,msg(lng::MHighlightMarking2).c_str()},
		{DI_BUTTON,5,21,0,21,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,NameLabels[2].second.c_str()},
		{DI_BUTTON,0,21,0,21,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,msg(lng::MHighlightMarking3).c_str()},
		{DI_BUTTON,5,22,0,22,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,NameLabels[3].second.c_str()},
		{DI_BUTTON,0,22,0,22,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,msg(lng::MHighlightMarking4).c_str()},

		{DI_USERCONTROL,73-15-1,19,73-2,22,0,nullptr,nullptr,DIF_NOFOCUS,L""},
		{DI_CHECKBOX,5,23,0,23,0,nullptr,nullptr,0,msg(lng::MHighlightContinueProcessing).c_str()},

		{DI_TEXT,-1,19,0,19,0,nullptr,nullptr,DIF_SEPARATOR,L""},

		{DI_CHECKBOX,5,20,0,20,0,nullptr,nullptr,0,msg(lng::MFileHardLinksCount).c_str()},//добавляем новый чекбокс в панель
		{DI_TEXT,-1,21,0,21,0,nullptr,nullptr,DIF_SEPARATOR,L""},// и разделитель

		{DI_BUTTON,0,22,0,22,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,msg(lng::MOk).c_str()},
		{DI_BUTTON,0,22,0,22,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_BTNNOCLOSE,msg(lng::MFileFilterReset).c_str()},
		{DI_BUTTON,0,22,0,22,0,nullptr,nullptr,DIF_CENTERGROUP,msg(lng::MFileFilterCancel).c_str()},
		{DI_BUTTON,0,22,0,22,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_BTNNOCLOSE,msg(lng::MFileFilterMakeTransparent).c_str()},
	};
	FilterDlgData[0].Data = msg(ColorConfig? lng::MFileHilightTitle : lng::MFileFilterTitle).c_str();
	auto FilterDlg = MakeDialogItemsEx(FilterDlgData);

	if (ColorConfig)
	{
		FilterDlg[ID_FF_TITLE].Y2+=5;

		for (int i=ID_FF_NAME; i<=ID_FF_SEPARATOR1; i++)
			FilterDlg[i].Flags|=DIF_HIDDEN;

		for (int i=ID_FF_MATCHMASK; i < ID_HER_SEPARATOR1; i++)
		{
			FilterDlg[i].Y1-=2;
			FilterDlg[i].Y2-=2;
		}

		for (int i=ID_FF_SEPARATOR5; i<=ID_FF_MAKETRANSPARENT; i++)
		{
			FilterDlg[i].Y1+=5;
			FilterDlg[i].Y2+=5;
		}
	}
	else
	{
		for (int i=ID_HER_SEPARATOR1; i<=ID_HER_CONTINUEPROCESSING; i++)
			FilterDlg[i].Flags|=DIF_HIDDEN;

		FilterDlg[ID_FF_MAKETRANSPARENT].Flags=DIF_HIDDEN;
	}

	const auto& AmpFixup = [&](size_t Index)
	{
		return contains(FilterDlg[Index].strData, L'&')? 1 : 0;
	};

	FilterDlg[ID_FF_NAMEEDIT].X1 = FilterDlg[ID_FF_NAME].X1 + FilterDlg[ID_FF_NAME].strData.size() - AmpFixup(ID_FF_NAME) + 1;
	FilterDlg[ID_FF_MASKEDIT].X1 = FilterDlg[ID_FF_MATCHMASK].X1 + FilterDlg[ID_FF_MATCHMASK].strData.size() - AmpFixup(ID_FF_MATCHMASK) + 5;
	FilterDlg[ID_FF_BLANK].X1 = FilterDlg[ID_FF_BLANK].X2 - FilterDlg[ID_FF_BLANK].strData.size() + AmpFixup(ID_FF_BLANK) - 3;
	FilterDlg[ID_FF_CURRENT].X2 = FilterDlg[ID_FF_BLANK].X1 - 2;
	FilterDlg[ID_FF_CURRENT].X1 = FilterDlg[ID_FF_CURRENT].X2 - FilterDlg[ID_FF_CURRENT].strData.size() + AmpFixup(ID_FF_CURRENT) - 3;
	FilterDlg[ID_HER_MARKTRANSPARENT].X1 = FilterDlg[ID_HER_MARK_TITLE].X1 + FilterDlg[ID_HER_MARK_TITLE].strData.size() - AmpFixup(ID_HER_MARK_TITLE) + 1;

	for (int i = ID_HER_NORMALMARKING; i <= ID_HER_SELECTEDCURSORMARKING; i += 2)
		FilterDlg[i].X1 = FilterDlg[ID_HER_NORMALFILE].X1 + FilterDlg[ID_HER_NORMALFILE].strData.size() - AmpFixup(ID_HER_NORMALFILE) + 1;

	FAR_CHAR_INFO VBufColorExample[15*4]={};
	auto Colors = FF->GetColors();
	HighlightDlgUpdateUserControl(VBufColorExample,Colors);
	FilterDlg[ID_HER_COLOREXAMPLE].VBuf=VBufColorExample;
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

	const auto& ProcessDuration = [&](auto Duration, auto DateId, auto TimeId)
	{
		FilterDlg[ID_FF_DATERELATIVE].Selected = BSTATE_CHECKED;
		ConvertDuration(Duration, FilterDlg[DateId].strData, FilterDlg[TimeId].strData);
	};

	const auto& ProcessPoint = [&](auto Point, auto DateId, auto TimeId)
	{
		FilterDlg[ID_FF_DATERELATIVE].Selected = BSTATE_UNCHECKED;
		ConvertDate(Point, FilterDlg[DateId].strData, FilterDlg[TimeId].strData, 12, FALSE, FALSE, 2);
	};

	Dates.visit(overload
	(
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
	));

	if (!FilterDlg[ID_FF_MATCHDATE].Selected)
		for (int i=ID_FF_DATETYPE; i <= ID_FF_BLANK; i++)
			FilterDlg[i].Flags|=DIF_DISABLE;

	static const std::pair<enumFileFilterConfig, DWORD> AttributeMapping[] =
	{
		{ ID_FF_READONLY, FILE_ATTRIBUTE_READONLY },
		{ ID_FF_ARCHIVE, FILE_ATTRIBUTE_ARCHIVE },
		{ ID_FF_HIDDEN, FILE_ATTRIBUTE_HIDDEN },
		{ ID_FF_SYSTEM, FILE_ATTRIBUTE_SYSTEM },
		{ ID_FF_COMPRESSED, FILE_ATTRIBUTE_COMPRESSED },
		{ ID_FF_ENCRYPTED, FILE_ATTRIBUTE_ENCRYPTED },
		{ ID_FF_NOTINDEXED, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED },
		{ ID_FF_DIRECTORY, FILE_ATTRIBUTE_DIRECTORY },
		{ ID_FF_SPARSE, FILE_ATTRIBUTE_SPARSE_FILE },
		{ ID_FF_TEMP, FILE_ATTRIBUTE_TEMPORARY },
		{ ID_FF_OFFLINE, FILE_ATTRIBUTE_OFFLINE },
		{ ID_FF_REPARSEPOINT, FILE_ATTRIBUTE_REPARSE_POINT },
		{ ID_FF_VIRTUAL, FILE_ATTRIBUTE_VIRTUAL },
		{ ID_FF_INTEGRITY_STREAM, FILE_ATTRIBUTE_INTEGRITY_STREAM },
		{ ID_FF_NO_SCRUB_DATA, FILE_ATTRIBUTE_NO_SCRUB_DATA },
	};

	DWORD AttrSet, AttrClear;
	FilterDlg[ID_FF_MATCHATTRIBUTES].Selected=FF->GetAttr(&AttrSet,&AttrClear)?1:0;

	for (const auto& i: AttributeMapping)
	{
		FilterDlg[i.first].Selected = AttrSet & i.second? BSTATE_CHECKED : AttrClear & i.second? BSTATE_UNCHECKED : BSTATE_3STATE;
	}

	if (!FilterDlg[ID_FF_MATCHATTRIBUTES].Selected)
	{
		for (int i=ID_FF_READONLY; i < ID_HER_SEPARATOR1; i++)
			FilterDlg[i].Flags|=DIF_DISABLE;
	}

	const auto Dlg = Dialog::create(FilterDlg, FileFilterConfigDlgProc, ColorConfig ? &Colors : nullptr);
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

	for (const auto& i: AttributeMapping)
	{
		Dlg->SetAutomation(ID_FF_MATCHATTRIBUTES, i.first, DIF_DISABLE, DIF_NONE, DIF_NONE, DIF_DISABLE);
	}

	for (;;)
	{
		Dlg->ClearDone();
		Dlg->Process();
		int ExitCode=Dlg->GetExitCode();

		if (ExitCode==ID_FF_OK) // Ok
		{
			// Если введённая пользователем маска не корректна, тогда вернёмся в диалог
			if (FilterDlg[ID_FF_MATCHMASK].Selected && !FileMask.Set(FilterDlg[ID_FF_MASKEDIT].strData,0))
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
					ParseDuration(FilterDlg[ID_FF_DAYSAFTEREDIT].strData, FilterDlg[ID_FF_TIMEAFTEREDIT].strData, DateFormat, TimeRanges),
					ParseDuration(FilterDlg[ID_FF_DAYSBEFOREEDIT].strData, FilterDlg[ID_FF_TIMEBEFOREEDIT].strData, DateFormat, TimeRanges)
				) :
				filter_dates
				(
					ParseDate(FilterDlg[ID_FF_DATEAFTEREDIT].strData, FilterDlg[ID_FF_TIMEAFTEREDIT].strData, DateFormat, DateRanges, TimeRanges),
					ParseDate(FilterDlg[ID_FF_DATEBEFOREEDIT].strData, FilterDlg[ID_FF_TIMEBEFOREEDIT].strData, DateFormat, DateRanges, TimeRanges)
				);

			FF->SetDate(FilterDlg[ID_FF_MATCHDATE].Selected != 0, static_cast<enumFDateType>(FilterDlg[ID_FF_DATETYPE].ListPos), NewDates);
			AttrSet=0;
			AttrClear=0;

			for(const auto& i: AttributeMapping)
			{
				switch (FilterDlg[i.first].Selected)
				{
				case BSTATE_CHECKED:
					AttrSet |= i.second;
					break;

				case BSTATE_UNCHECKED:
					AttrClear |= i.second;
					break;

				default:
					break;
				}
			}

			FF->SetAttr(FilterDlg[ID_FF_MATCHATTRIBUTES].Selected!=0,
			            AttrSet,
			            AttrClear);
			return true;
		}
		else
			break;
	}

	return false;
}
