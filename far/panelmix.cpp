/*
panelmix.cpp

Commonly used panel related functions
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
#include "panelmix.hpp"

// Internal:
#include "strmix.hpp"
#include "filepanels.hpp"
#include "config.hpp"
#include "panel.hpp"
#include "ctrlobj.hpp"
#include "keys.hpp"
#include "filelist.hpp"
#include "pathmix.hpp"
#include "panelctype.hpp"
#include "datetime.hpp"
#include "flink.hpp"
#include "plugins.hpp"
#include "lang.hpp"
#include "fileattr.hpp"
#include "cvtname.hpp"
#include "global.hpp"
#include "exception.hpp"
#include "log.hpp"

// Platform:

// Common:
#include "common/enum_tokens.hpp"
#include "common/from_string.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static const struct column_info
{
	column_type Type;
	int DefaultWidth;
	const string_view String;
}
ColumnInfo[]
{
	{ column_type::name,                   0,     L"N"sv,     },
	{ column_type::size,                   6,     L"S"sv,     },
	{ column_type::size_compressed,        6,     L"P"sv,     },
	{ column_type::date,                   8,     L"D"sv,     },
	{ column_type::time,                   5,     L"T"sv,     },
	{ column_type::date_write,             14,    L"DM"sv,    },
	{ column_type::date_creation,          14,    L"DC"sv,    },
	{ column_type::date_access,            14,    L"DA"sv,    },
	{ column_type::date_change,            14,    L"DE"sv,    },
	{ column_type::attributes,             6,     L"A"sv,     },
	{ column_type::description,            0,     L"Z"sv,     },
	{ column_type::owner,                  0,     L"O"sv,     },
	{ column_type::links_number,           3,     L"LN"sv,    },
	{ column_type::streams_number,         3,     L"F"sv,     },
	{ column_type::streams_size,           6,     L"G"sv,     },
	{ column_type::extension,              0,     L"X"sv,     },
	{ column_type::custom_0,               0,     L"C0"sv,    },
};

static_assert(std::size(ColumnInfo) == static_cast<size_t>(column_type::count));

void ShellUpdatePanels(panel_ptr SrcPanel, bool NeedSetUpADir)
{
	if (!SrcPanel)
		SrcPanel = Global->CtrlObject->Cp()->ActivePanel();

	auto AnotherPanel = Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel);

	switch (SrcPanel->GetType())
	{
	case panel_type::FILE_PANEL:
	case panel_type::TREE_PANEL:
		break;

	case panel_type::QVIEW_PANEL:
	case panel_type::INFO_PANEL:
		SrcPanel.swap(AnotherPanel);
	}

	const auto AnotherType = AnotherPanel->GetType();

	if (AnotherType != panel_type::QVIEW_PANEL && AnotherType != panel_type::INFO_PANEL)
	{
		if (NeedSetUpADir)
		{
			AnotherPanel->SetCurDir(SrcPanel->GetCurDir(), true);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
		}
		else
		{
			// TODO: ???
			//if(AnotherPanel->NeedUpdatePanel(SrcPanel))
			//  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			//else
			{
				AnotherPanel->UpdateIfChanged();
			}
		}
	}

	SrcPanel->Update(UPDATE_KEEP_SELECTION);

	if (AnotherType == panel_type::QVIEW_PANEL)
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);

	Global->CtrlObject->Cp()->Redraw();
}

bool CheckUpdateAnotherPanel(panel_ptr SrcPanel, string_view const SelName)
{
	if (!SrcPanel)
		SrcPanel = Global->CtrlObject->Cp()->ActivePanel();

	const auto AnotherPanel = Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
	AnotherPanel->CloseFile();

	if (AnotherPanel->GetMode() == panel_mode::NORMAL_PANEL)
	{
		string strAnotherCurDir(AnotherPanel->GetCurDir());
		AddEndSlash(strAnotherCurDir);
		auto strFullName = ConvertNameToFull(SelName);
		AddEndSlash(strFullName);

		if (contains(strAnotherCurDir, strFullName))
		{
			AnotherPanel->StopFSWatcher();
			return true;
		}
	}

	return false;
}

bool MakePath(const panel_ptr& SrcPanel, bool FilePath, bool RealName, bool ShortNameAsIs, string& strPathName)
{
	if (FilePath)
	{
		string strShortFileName;
		if (!SrcPanel->GetCurName(strPathName, strShortFileName))
			return false;

		if (ShortNameAsIs && SrcPanel->GetShowShortNamesMode()) // учтем короткость имен :-)
			strPathName = strShortFileName;
	}
	else
	{
		if (!(SrcPanel->GetType() == panel_type::FILE_PANEL || SrcPanel->GetType() == panel_type::TREE_PANEL))
			return false;

		strPathName = SrcPanel->GetCurDir();

		if (SrcPanel->GetMode() != panel_mode::PLUGIN_PANEL)
		{
			if (RealName)
				strPathName = SrcPanel->CreateFullPathName(strPathName, true, true, ShortNameAsIs);

			if (SrcPanel->GetShowShortNamesMode() && ShortNameAsIs)
				strPathName = ConvertNameToShort(strPathName);
		}
		else
		{
			if (const auto SrcFileList = std::dynamic_pointer_cast<FileList>(SrcPanel))
			{
				OpenPanelInfo Info;
				Global->CtrlObject->Plugins->GetOpenPanelInfo(SrcFileList->GetPluginHandle(), &Info);
				strPathName = SrcFileList->GetPluginPrefix();
				if (Info.HostFile && *Info.HostFile)
				{
					append(strPathName, Info.HostFile, L'/');
				}
				strPathName += NullToEmpty(Info.CurDir);
			}
		}

		AddEndSlash(strPathName);
	}

	return true;
}

bool MakePathForUI(DWORD Key, string &strPathName)
{
	auto RealName = false;

	switch (Key)
	{
	case KEY_CTRLALTBRACKET:       // Вставить сетевое (UNC) путь из левой панели
	case KEY_RCTRLRALTBRACKET:
	case KEY_CTRLRALTBRACKET:
	case KEY_RCTRLALTBRACKET:
	case KEY_CTRLALTBACKBRACKET:   // Вставить сетевое (UNC) путь из правой панели
	case KEY_RCTRLRALTBACKBRACKET:
	case KEY_CTRLRALTBACKBRACKET:
	case KEY_RCTRLALTBACKBRACKET:
	case KEY_ALTSHIFTBRACKET:      // Вставить сетевое (UNC) путь из активной панели
	case KEY_RALTSHIFTBRACKET:
	case KEY_ALTSHIFTBACKBRACKET:  // Вставить сетевое (UNC) путь из пассивной панели
	case KEY_RALTSHIFTBACKBRACKET:
		RealName = true;
		[[fallthrough]];
	case KEY_CTRLBRACKET:          // Вставить путь из левой панели
	case KEY_RCTRLBRACKET:
	case KEY_CTRLBACKBRACKET:      // Вставить путь из правой панели
	case KEY_RCTRLBACKBRACKET:
	case KEY_CTRLSHIFTBRACKET:     // Вставить путь из активной панели
	case KEY_RCTRLSHIFTBRACKET:
	case KEY_CTRLSHIFTBACKBRACKET: // Вставить путь из пассивной панели
	case KEY_RCTRLSHIFTBACKBRACKET:
	case KEY_CTRLSHIFTNUMENTER:    // Текущий файл с пасс.панели
	case KEY_RCTRLSHIFTNUMENTER:
	case KEY_SHIFTNUMENTER:        // Текущий файл с актив.панели
	case KEY_CTRLSHIFTENTER:       // Текущий файл с пасс.панели
	case KEY_RCTRLSHIFTENTER:
	case KEY_SHIFTENTER:           // Текущий файл с актив.панели
		{
			panel_ptr SrcPanel;
			const auto Cp = Global->CtrlObject->Cp();

			switch (Key)
			{
			case KEY_CTRLALTBRACKET:
			case KEY_RCTRLRALTBRACKET:
			case KEY_CTRLRALTBRACKET:
			case KEY_RCTRLALTBRACKET:
			case KEY_CTRLBRACKET:
			case KEY_RCTRLBRACKET:
				SrcPanel = Cp->LeftPanel();
				break;

			case KEY_CTRLALTBACKBRACKET:
			case KEY_RCTRLRALTBACKBRACKET:
			case KEY_CTRLRALTBACKBRACKET:
			case KEY_RCTRLALTBACKBRACKET:
			case KEY_CTRLBACKBRACKET:
			case KEY_RCTRLBACKBRACKET:
				SrcPanel = Cp->RightPanel();
				break;

			case KEY_SHIFTNUMENTER:
			case KEY_SHIFTENTER:
			case KEY_ALTSHIFTBRACKET:
			case KEY_RALTSHIFTBRACKET:
			case KEY_CTRLSHIFTBRACKET:
			case KEY_RCTRLSHIFTBRACKET:
				SrcPanel = Cp->ActivePanel();
				break;

			case KEY_CTRLSHIFTNUMENTER:
			case KEY_RCTRLSHIFTNUMENTER:
			case KEY_CTRLSHIFTENTER:
			case KEY_RCTRLSHIFTENTER:
			case KEY_ALTSHIFTBACKBRACKET:
			case KEY_RALTSHIFTBACKBRACKET:
			case KEY_CTRLSHIFTBACKBRACKET:
			case KEY_RCTRLSHIFTBACKBRACKET:
				SrcPanel = Cp->PassivePanel();
				break;
			}

			const auto FilePath = any_of(Key, KEY_SHIFTENTER, KEY_CTRLSHIFTENTER, KEY_RCTRLSHIFTENTER, KEY_SHIFTNUMENTER, KEY_CTRLSHIFTNUMENTER, KEY_RCTRLSHIFTNUMENTER);
			if (!SrcPanel || !MakePath(SrcPanel, FilePath, RealName, true, strPathName))
				return false;

			if (Global->Opt->QuotedName & QUOTEDNAME_INSERT)
				inplace::QuoteSpace(strPathName);

			return true;
		}

	default:
		return false;
	}
}

std::vector<column> DeserialiseViewSettings(string_view const ColumnTitles, string_view const ColumnWidths)
{
	// BUGBUG, add error checking

	std::vector<column> Columns;

	for (const auto& Type: enum_tokens(ColumnTitles, L","sv))
	{
		if (Type.empty())
			continue;

		column NewColumn{};

		if (Type.front() == L'N')
		{
			NewColumn.type = column_type::name;

			for (const auto& i: Type.substr(1))
			{
				switch (i)
				{
				case L'M': NewColumn.type_flags |= COLFLAGS_MARK;               break;
				case L'D': NewColumn.type_flags |= COLFLAGS_MARK_DYNAMIC;       break;
				case L'O': NewColumn.type_flags |= COLFLAGS_NAMEONLY;           break;
				case L'R': NewColumn.type_flags |= COLFLAGS_RIGHTALIGN;         break;
				case L'F': NewColumn.type_flags |= COLFLAGS_RIGHTALIGNFORCE;    break;
				case L'N': NewColumn.type_flags |= COLFLAGS_NOEXTENSION;        break;
				}
			}
		}
		else if (Type.front() == L'S' || Type.front() == L'P' || Type.front() == L'G')
		{
			NewColumn.type = Type.front() == L'S'?
				column_type::size :
				Type.front() == L'P'?
					column_type::size_compressed :
					column_type::streams_size;

			for (const auto& i: Type.substr(1))
			{
				switch (i)
				{
				case L'C': NewColumn.type_flags |= COLFLAGS_GROUPDIGITS;    break;
				case L'E': NewColumn.type_flags |= COLFLAGS_ECONOMIC;       break;
				case L'F': NewColumn.type_flags |= COLFLAGS_FLOATSIZE;      break;
				case L'T': NewColumn.type_flags |= COLFLAGS_THOUSAND;       break;
				}
			}
		}
		else if (
			Type.starts_with(L"DM"sv) ||
			Type.starts_with(L"DC"sv) ||
			Type.starts_with(L"DA"sv) ||
			Type.starts_with(L"DE"sv))
		{
			switch (Type[1])
			{
			case L'M': NewColumn.type = column_type::date_write;      break;
			case L'C': NewColumn.type = column_type::date_creation;   break;
			case L'A': NewColumn.type = column_type::date_access;     break;
			case L'E': NewColumn.type = column_type::date_change;     break;
			}

			for (const auto& i: Type.substr(2))
			{
				switch (i)
				{
				case L'B': NewColumn.type_flags |= COLFLAGS_BRIEF;    break;
				case L'M': NewColumn.type_flags |= COLFLAGS_MONTH;    break;
				}
			}
		}
		else if (Type.front() == L'O')
		{
			NewColumn.type = column_type::owner;

			if (Type.size() > 1 && Type[1] == L'L')
				NewColumn.type_flags |= COLFLAGS_FULLOWNER;
		}
		else if (Type.front() == L'X')
		{
			NewColumn.type = column_type::extension;

			if (Type.size() > 1 && Type[1] == L'R')
				NewColumn.type_flags |= COLFLAGS_RIGHTALIGN;
		}
		else if (Type.size() > 2 && Type.front() == L'<' && Type.back() == L'>')
		{
			NewColumn.title = Type.substr(1, Type.size() - 2);
			NewColumn.type = column_type::custom_0;
		}
		else
		{
			const auto ItemIterator = std::ranges::find(ColumnInfo, Type, &column_info::String);
			if (ItemIterator != std::cend(ColumnInfo))
				NewColumn.type = ItemIterator->Type;
			else if (Type.size() >= 2 && Type.size() <= 3 && Type.front() == L'C')
			{
				size_t Index;
				if (from_string(Type.substr(1), Index))
					NewColumn.type = static_cast<column_type>(static_cast<size_t>(column_type::custom_0) + Index);
				else
				{
					LOGWARNING(L"Incorrect custom column {}"sv, Type);
					// TODO: error message?
				}
			}
			else
			{
				LOGWARNING(L"Unknown column type {}"sv, Type);
				// TODO: error message?
				continue;
			}
		}

		Columns.emplace_back(NewColumn);
	}

	enum_tokens const EnumWidths(ColumnWidths, L","sv);
	std::ranges::subrange EnumWidthsRange(EnumWidths);

	for (auto& i: Columns)
	{
		auto Width = L""sv;

		if (!EnumWidthsRange.empty())
		{
			Width = *EnumWidthsRange.begin();
			EnumWidthsRange.advance(1);
		}

		// "column types" is a determinant here (see the loop header) so we can't break or continue here -
		// if "column sizes" ends earlier or if user entered two commas we just use default size.
		if (!Width.empty() && !from_string(Width, i.width))
		{
			LOGWARNING(L"Incorrect column width {}"sv, Width);
		}

		i.width_type = col_width::fixed;

		if (Width.size()>1)
		{
			switch (Width.back())
			{
				case L'%':
					i.width_type = col_width::percent;
					break;
			}
		}
	}

	if (Columns.empty())
	{
		Columns.emplace_back();
	}

	return Columns;
}


std::pair<string, string> SerialiseViewSettings(const std::vector<column>& Columns)
{
	std::pair<string, string> Result;
	auto& [strColumnTitles, strColumnWidths] = Result;

	const auto GetModeSymbol = [](FILEPANEL_COLUMN_FLAGS Mode)
	{
		switch (Mode)
		{
		case COLFLAGS_MARK:            return L'M';
		case COLFLAGS_NAMEONLY:        return L'O';
		case COLFLAGS_RIGHTALIGN:      return L'R';
		case COLFLAGS_GROUPDIGITS:     return L'C';
		case COLFLAGS_THOUSAND:        return L'T';
		case COLFLAGS_BRIEF:           return L'B';
		case COLFLAGS_MONTH:           return L'M';
		case COLFLAGS_FLOATSIZE:       return L'F';
		case COLFLAGS_ECONOMIC:        return L'E';
		case COLFLAGS_FULLOWNER:       return L'L';
		case COLFLAGS_NOEXTENSION:     return L'N';
		case COLFLAGS_RIGHTALIGNFORCE: return L'F';
		case COLFLAGS_MARK_DYNAMIC:    return L'D';
		default:
			throw far_fatal_exception(far::format(L"Unexpected mode {}"sv, std::to_underlying(Mode)));
		}
	};

	for (const auto& i: Columns)
	{
		string strType;

		if (i.type <= column_type::custom_0)
		{
			strType = ColumnInfo[static_cast<size_t>(i.type)].String;
		}
		else
		{
			strType = L'C' + str(static_cast<size_t>(i.type) - static_cast<size_t>(column_type::custom_0));
		}

		const auto AddFlag = [&](auto Flag)
		{
			if (!(i.type_flags & Flag))
				return false;
			strType += GetModeSymbol(Flag);
			return true;
		};

		switch (i.type)
		{
		case column_type::name:
			AddFlag(COLFLAGS_MARK) && AddFlag(COLFLAGS_MARK_DYNAMIC);
			AddFlag(COLFLAGS_NAMEONLY);
			AddFlag(COLFLAGS_RIGHTALIGN) && AddFlag(COLFLAGS_RIGHTALIGNFORCE);
			AddFlag(COLFLAGS_NOEXTENSION);
			break;

		case column_type::size:
		case column_type::size_compressed:
		case column_type::streams_size:
			AddFlag(COLFLAGS_GROUPDIGITS);
			AddFlag(COLFLAGS_ECONOMIC);
			AddFlag(COLFLAGS_FLOATSIZE);
			AddFlag(COLFLAGS_THOUSAND);
			break;

		case column_type::date_write:
		case column_type::date_access:
		case column_type::date_creation:
		case column_type::date_change:
			AddFlag(COLFLAGS_BRIEF);
			AddFlag(COLFLAGS_MONTH);
			break;

		case column_type::owner:
			AddFlag(COLFLAGS_FULLOWNER);
			break;

		case column_type::extension:
			AddFlag(COLFLAGS_RIGHTALIGN);
			break;

		case column_type::custom_0:
			if(!i.title.empty())
				strType = concat(L'<', i.title, L'>');
			break;

		default:
			break;
		}

		strColumnTitles += strType;

		strColumnWidths += str(i.width);

		switch (i.width_type)
		{
			case col_width::percent:
				strColumnWidths += L'%';
				break;

			case col_width::fixed:
				break;
		}

		strColumnTitles += L',';
		strColumnWidths += L',';
	}

	if (!strColumnTitles.empty())
		strColumnTitles.pop_back();
	if (!strColumnWidths.empty())
		strColumnWidths.pop_back();

	return Result;
}

string FormatStr_Attribute(os::fs::attributes FileAttributes, size_t const Width)
{
	string OutStr;

	if (!FileAttributes)
		FileAttributes = FILE_ATTRIBUTE_NORMAL;

	enum_attributes([&](os::fs::attributes const Attribute, wchar_t const Character)
	{
		if (FileAttributes & Attribute)
		{
			OutStr.push_back(Character);
		}
		return OutStr.size() < Width;
	});

	inplace::fit_to_left(OutStr, Width);
	return OutStr;
}

string FormatStr_DateTime(os::chrono::time_point FileTime, column_type const ColumnType, unsigned long long Flags, int Width)
{
	if (Width < 0)
	{
		if (ColumnType == column_type::date)
			Width=0;
		else
			return {};
	}

	int ColumnWidth=Width;
	bool Brief = (Flags & COLFLAGS_BRIEF) != 0;
	bool TextMonth = (Flags & COLFLAGS_MONTH) != 0;
	bool FullYear = false;

	switch(ColumnType)
	{
		case column_type::date:
		case column_type::time:
		{
			Brief = false;
			TextMonth = false;
			if (ColumnType == column_type::date)
				FullYear=ColumnWidth>9;
			break;
		}
		case column_type::date_write:
		case column_type::date_creation:
		case column_type::date_access:
		case column_type::date_change:
		{
			if (!Brief)
			{
				const auto CmpWidth = ColumnWidth - (TextMonth? 1 : 0);

				if (CmpWidth==15 || CmpWidth==16 || CmpWidth==18 || CmpWidth==19 || CmpWidth>21)
					FullYear = true;
			}
			ColumnWidth-=9;
			break;
		}

		default:
			break;
	}

	const auto& [Date, Time] = ConvertDate(FileTime, ColumnWidth, FullYear, Brief, TextMonth);

	string OutStr;

	switch(ColumnType)
	{
		case column_type::date:
			OutStr = Date;
			break;

		case column_type::time:
			OutStr = Time;
			break;

		default:
			OutStr = concat(Date, L' ', Time);
			break;
	}

	return fit_to_right(OutStr, Width);
}

string FormatStr_Size(
	long long const Size,
	string_view const strName,
	os::fs::attributes const FileAttributes,
	DWORD const ShowFolderSize,
	DWORD const ReparseTag,
	column_type const ColumnType,
	unsigned long long const Flags,
	int Width,
	string_view const CurDir)
{
	string strResult;

	const auto Streams = ColumnType == column_type::streams_size;

	if (ShowFolderSize==2)
	{
		Width--;
		strResult += L'~';
	}

	const auto dir = (0 != (FileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	const auto rpt = (0 != (FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT));
	const auto have_size = !dir && (!rpt || ReparseTag==IO_REPARSE_TAG_DEDUP || Size > 0);

	if (!Streams && !have_size && !ShowFolderSize)
	{
		static const lng FolderLabels[] =
		{
			lng::MListUp,
			lng::MListFolder,
		};

		const auto LabelSize = std::ranges::fold_left(FolderLabels, 0uz, [](size_t const Value, lng const Id) { return std::max(Value, msg(Id).size()); });

		string TypeName;

		if (IsParentDirectory(strName))
		{
			TypeName = fit_to_center(msg(lng::MListUp), LabelSize);
		}
		else
		{
			if (rpt)
			{
				switch(ReparseTag)
				{
				// Directory Junction or Volume Mount Point
				case IO_REPARSE_TAG_MOUNT_POINT:
					{
						auto ID_Msg = lng::MListJunction;
						if (Global->Opt->PanelDetailedJunction && !CurDir.empty())
						{
							if (string strLinkName; GetReparsePointInfo(path::join(CurDir, PointToName(strName)), strLinkName))
							{
								NormalizeSymlinkName(strLinkName);

								if(bool Root; ParsePath(strLinkName, nullptr, &Root) == root_type::volume && Root)
									ID_Msg = lng::MListVolMount;
							}
						}
						TypeName = msg(ID_Msg);
					}
					break;

				case IO_REPARSE_TAG_SYMLINK:
					TypeName = msg(lng::MListSymlink);
					break;

				default:
					if (!reparse_tag_to_string(ReparseTag, TypeName, Global->Opt->ShowUnknownReparsePoint) && TypeName.empty())
						TypeName = msg(lng::MListUnknownReparsePoint);
					break;
				}
			}
			else if (dir)
			{
				TypeName = fit_to_center(msg(lng::MListFolder), LabelSize);
			}
		}

		if (static_cast<int>(TypeName.size()) <= Width-2 && msg(lng::MListBrackets).size() > 1)
		{
			TypeName = concat(msg(lng::MListBrackets)[0], TypeName, msg(lng::MListBrackets)[1]);
		}
		strResult += fit_to_right(TypeName, Width);
	}
	else
	{
		strResult += FileSizeToStr(Size, Width, Flags);
	}

	return strResult;
}

int GetDefaultWidth(const column& Column)
{
	int Width = (Column.type > column_type::custom_0)? 0 : ColumnInfo[static_cast<size_t>(Column.type)].DefaultWidth;

	if (
		Column.type == column_type::date_write ||
		Column.type == column_type::date_creation ||
		Column.type == column_type::date_access ||
		Column.type == column_type::date_change
	)
	{
		if (Column.type_flags & COLFLAGS_BRIEF)
			Width -= 3;

		if (Column.type_flags & COLFLAGS_MONTH)
			++Width;
	}
	return Width;
}
