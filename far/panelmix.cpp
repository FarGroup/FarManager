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

#include "headers.hpp"
#pragma hdrstop

#include "panelmix.hpp"
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
#include "string_utils.hpp"
#include "cvtname.hpp"

static const struct column_info
{
	PANEL_COLUMN_TYPE Type;
	int DefaultWidth;
	const string_view String;
}
ColumnInfo[] =
{
	{ NAME_COLUMN, 0, L"N"_sv },
	{ SIZE_COLUMN, 6, L"S"_sv },
	{ PACKED_COLUMN, 6, L"P"_sv },
	{ DATE_COLUMN, 8, L"D"_sv },
	{ TIME_COLUMN, 5, L"T"_sv },
	{ WDATE_COLUMN, 14, L"DM"_sv },
	{ CDATE_COLUMN, 14, L"DC"_sv },
	{ ADATE_COLUMN, 14, L"DA"_sv },
	{ CHDATE_COLUMN, 14, L"DE"_sv },
	{ ATTR_COLUMN, 6, L"A"_sv },
	{ DIZ_COLUMN, 0, L"Z"_sv },
	{ OWNER_COLUMN, 0, L"O"_sv },
	{ NUMLINK_COLUMN, 3, L"LN"_sv },
	{ NUMSTREAMS_COLUMN, 3, L"F"_sv },
	{ STREAMSSIZE_COLUMN, 6, L"G"_sv },
	{ EXTENSION_COLUMN, 0, L"X"_sv, },
	{ CUSTOM_COLUMN0, 0, L"C0"_sv },
};

static_assert(std::size(ColumnInfo) == COLUMN_TYPES_COUNT);

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
				// Сбросим время обновления панели. Если там есть нотификация - обновится сама.
				if (const auto AnotherFileList = std::dynamic_pointer_cast<FileList>(AnotherPanel))
				{
					AnotherFileList->ResetLastUpdateTime();
				}

				AnotherPanel->UpdateIfChanged(false);
			}
		}
	}

	SrcPanel->Update(UPDATE_KEEP_SELECTION);

	if (AnotherType == panel_type::QVIEW_PANEL)
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);

	Global->CtrlObject->Cp()->Redraw();
}

bool CheckUpdateAnotherPanel(panel_ptr SrcPanel, const string& SelName)
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
		SrcPanel->GetCurName(strPathName,strShortFileName);

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
				SrcPanel->CreateFullPathName(strPathName, strPathName, FILE_ATTRIBUTE_DIRECTORY, strPathName, true, ShortNameAsIs);

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

			const auto FilePath = Key == KEY_SHIFTENTER || Key == KEY_CTRLSHIFTENTER || Key == KEY_RCTRLSHIFTENTER || Key == KEY_SHIFTNUMENTER || Key == KEY_CTRLSHIFTNUMENTER || Key == KEY_RCTRLSHIFTNUMENTER;
			if (!SrcPanel || !MakePath(SrcPanel, FilePath, RealName, true, strPathName))
				return false;

			if (Global->Opt->QuotedName & QUOTEDNAME_INSERT)
				QuoteSpace(strPathName);

			return true;
		}

	default:
		return false;
	}
}

std::vector<column> DeserialiseViewSettings(const string& ColumnTitles,const string& ColumnWidths)
{
	// BUGBUG, add error checking

	FN_RETURN_TYPE(DeserialiseViewSettings) Columns;

	for (const auto& Type: enum_tokens(ColumnTitles, L","_sv))
	{
		if (Type.empty())
			continue;

		column NewColumn{};

		const auto TypeOrig = upper(string(Type));

		if (Type.front() == L'N')
		{
			NewColumn.type = NAME_COLUMN;
			for (const auto& i: Type.substr(1))
			{
				switch (i)
				{
				case L'M':
					NewColumn.type |= COLUMN_MARK;
					break;
				case L'D':
					NewColumn.type |= COLUMN_MARK_DYNAMIC;
					break;
				case L'O':
					NewColumn.type |= COLUMN_NAMEONLY;
					break;
				case L'R':
					NewColumn.type |= COLUMN_RIGHTALIGN;
					break;
				case L'F':
					NewColumn.type |= COLUMN_RIGHTALIGNFORCE;
					break;
				case L'N':
					NewColumn.type |= COLUMN_NOEXTENSION;
					break;
				}
			}
		}
		else if (Type.front() == L'S' || Type.front() == L'P' || Type.front() == L'G')
		{
			NewColumn.type = Type.front() == L'S'?
				SIZE_COLUMN :
				Type.front() == L'P'?
					PACKED_COLUMN :
					STREAMSSIZE_COLUMN;

			for (const auto& i: Type.substr(1))
			{
				switch (i)
				{
				case L'C':
					NewColumn.type |= COLUMN_COMMAS;
					break;
				case L'E':
					NewColumn.type |= COLUMN_ECONOMIC;
					break;
				case L'F':
					NewColumn.type |= COLUMN_FLOATSIZE;
					break;
				case L'T':
					NewColumn.type |= COLUMN_THOUSAND;
					break;
				}
			}
		}
		else if (
			starts_with(Type, L"DM"_sv) ||
			starts_with(Type, L"DC"_sv) ||
			starts_with(Type, L"DA"_sv) ||
			starts_with(Type, L"DE"_sv))
		{
			switch (Type[1])
			{
			case L'M':
				NewColumn.type = WDATE_COLUMN;
				break;
			case L'C':
				NewColumn.type = CDATE_COLUMN;
				break;
			case L'A':
				NewColumn.type = ADATE_COLUMN;
				break;
			case L'E':
				NewColumn.type = CHDATE_COLUMN;
				break;
			}

			for (const auto& i: Type.substr(2))
			{
				switch (i)
				{
				case L'B':
					NewColumn.type |= COLUMN_BRIEF;
					break;
				case L'M':
					NewColumn.type |= COLUMN_MONTH;
					break;
				}
			}
		}
		else if (Type.front() == L'O')
		{
			NewColumn.type = OWNER_COLUMN;

			if (Type.size() > 1 && Type[1] == L'L')
				NewColumn.type |= COLUMN_FULLOWNER;
		}
		else if (Type.front() == L'X')
		{
			NewColumn.type = EXTENSION_COLUMN;

			if (Type.size() > 1 && Type[1] == L'R')
				NewColumn.type |= COLUMN_RIGHTALIGN;
		}
		else if (Type.size() > 2 && Type.front() == L'<' && Type.back() == L'>')
		{
			NewColumn.title = string(Type.substr(1, Type.size() - 2));
			NewColumn.type = CUSTOM_COLUMN0;
		}
		else
		{
			const auto ItemIterator = std::find_if(CONST_RANGE(ColumnInfo, i) { return Type == i.String; });
			if (ItemIterator != std::cend(ColumnInfo))
				NewColumn.type = ItemIterator->Type;
			else if (Type.size() >= 2 && Type.size() <= 3 && Type.front() == L'C')
			{
				try
				{
					NewColumn.type = CUSTOM_COLUMN0 + std::stoi(TypeOrig.substr(1));
				}
				catch (const std::exception&)
				{
					// TODO: diagnostics
				}
			}
			else
			{
				// Unknown column type
				// TODO: error message
				continue;
			}
		}

		Columns.emplace_back(NewColumn);
	}

	const auto EnumWidths = enum_tokens(ColumnWidths, L","_sv);
	auto EnumWidthsRange = make_range(EnumWidths);

	for (auto& i: Columns)
	{
		auto Width = EnumWidthsRange.empty()? L""_sv : *EnumWidthsRange.pop_front();

		// "column types" is a determinant here (see the loop header) so we can't break or continue here -
		// if "column sizes" ends earlier or if user entered two commas we just use default size.
		if (Width.empty())
		{
			Width = L"0"_sv;
		}

		try
		{
			i.width = std::stoi(string(Width));
		}
		catch (const std::exception&)
		{
			// TODO: diagnostics
			i.width = 0;
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
		column NewColumn;
		NewColumn.type = NAME_COLUMN;
		Columns.emplace_back(NewColumn);
	}

	return Columns;
}


std::pair<string, string> SerialiseViewSettings(const std::vector<column>& Columns)
{
	FN_RETURN_TYPE(SerialiseViewSettings) Result;
	auto& strColumnTitles = Result.first;
	auto& strColumnWidths = Result.second;

	const auto& GetModeSymbol = [](FILEPANEL_COLUMN_MODES Mode)
	{
		switch (Mode)
		{
		case COLUMN_MARK:            return L'M';
		case COLUMN_NAMEONLY:        return L'O';
		case COLUMN_RIGHTALIGN:      return L'R';
		case COLUMN_COMMAS:          return L'C';
		case COLUMN_THOUSAND:        return L'T';
		case COLUMN_BRIEF:           return L'B';
		case COLUMN_MONTH:           return L'M';
		case COLUMN_FLOATSIZE:       return L'F';
		case COLUMN_ECONOMIC:        return L'E';
		case COLUMN_FULLOWNER:       return L'L';
		case COLUMN_NOEXTENSION:     return L'N';
		case COLUMN_RIGHTALIGNFORCE: return L'F';
		case COLUMN_MARK_DYNAMIC:    return L'D';
		default:                     throw MAKE_FAR_EXCEPTION(L"Unexpected mode");
		}
	};

	std::for_each(CONST_RANGE(Columns, i)
	{
		string strType;
		const auto ColumnType=static_cast<int>(i.type & 0xff);

		if (ColumnType <= CUSTOM_COLUMN0)
		{
			strType = string(ColumnInfo[ColumnType].String);
		}
		else
		{
			strType = L'C' + str(ColumnType - CUSTOM_COLUMN0);
		}

		const auto& AddFlag = [&](auto Flag)
		{
			if (!(i.type & Flag))
				return false;
			strType += GetModeSymbol(Flag);
			return true;
		};

		switch (ColumnType)
		{
		case NAME_COLUMN:
			AddFlag(COLUMN_MARK) && AddFlag(COLUMN_MARK_DYNAMIC);
			AddFlag(COLUMN_NAMEONLY);
			AddFlag(COLUMN_RIGHTALIGN) && AddFlag(COLUMN_RIGHTALIGNFORCE);
			AddFlag(COLUMN_NOEXTENSION);
			break;

		case SIZE_COLUMN:
		case PACKED_COLUMN:
		case STREAMSSIZE_COLUMN:
			AddFlag(COLUMN_COMMAS);
			AddFlag(COLUMN_ECONOMIC);
			AddFlag(COLUMN_FLOATSIZE);
			AddFlag(COLUMN_THOUSAND);
			break;

		case WDATE_COLUMN:
		case ADATE_COLUMN:
		case CDATE_COLUMN:
		case CHDATE_COLUMN:
			AddFlag(COLUMN_BRIEF);
			AddFlag(COLUMN_MONTH);
			break;

		case OWNER_COLUMN:
			AddFlag(COLUMN_FULLOWNER);
			break;

		case EXTENSION_COLUMN:
			AddFlag(COLUMN_RIGHTALIGN);
			break;

		case CUSTOM_COLUMN0:
			if(!i.title.empty())
				strType = concat(L'<', i.title, L'>');
		}

		strColumnTitles += strType;

		strColumnWidths += str(i.width);

		switch (i.width_type)
		{
			case col_width::percent:
				strColumnWidths += L'%';
				break;
		}

		strColumnTitles += L',';
		strColumnWidths += L',';
	});

	if (!strColumnTitles.empty())
		strColumnTitles.pop_back();
	if (!strColumnWidths.empty())
		strColumnWidths.pop_back();

	return Result;
}

string FormatStr_Attribute(DWORD FileAttributes, size_t Width)
{
	string OutStr;

	enum_attributes([&](DWORD Attribute, wchar_t Character)
	{
		if (FileAttributes & Attribute)
		{
			OutStr.push_back(Character);
		}
		return OutStr.size() < Width;
	});

	return inplace::fit_to_left(OutStr, Width);
}

string FormatStr_DateTime(os::chrono::time_point FileTime, int ColumnType, unsigned long long Flags, int Width)
{
	if (Width < 0)
	{
		if (ColumnType == DATE_COLUMN)
			Width=0;
		else
			return {};
	}

	int ColumnWidth=Width;
	bool Brief = (Flags & COLUMN_BRIEF) != 0;
	bool TextMonth = (Flags & COLUMN_MONTH) != 0;
	bool FullYear = false;

	switch(ColumnType)
	{
		case DATE_COLUMN:
		case TIME_COLUMN:
		{
			Brief = false;
			TextMonth = false;
			if (ColumnType == DATE_COLUMN)
				FullYear=ColumnWidth>9;
			break;
		}
		case WDATE_COLUMN:
		case CDATE_COLUMN:
		case ADATE_COLUMN:
		case CHDATE_COLUMN:
		{
			if (!Brief)
			{
				int CmpWidth=ColumnWidth-(TextMonth? 1: 0);

				if (CmpWidth==15 || CmpWidth==16 || CmpWidth==18 || CmpWidth==19 || CmpWidth>21)
					FullYear = true;
			}
			ColumnWidth-=9;
			break;
		}
	}

	string strDateStr,strTimeStr;

	ConvertDate(FileTime,strDateStr,strTimeStr,ColumnWidth,Brief,TextMonth,FullYear);

	string strOutStr;
	switch(ColumnType)
	{
		case DATE_COLUMN:
			strOutStr=strDateStr;
			break;
		case TIME_COLUMN:
			strOutStr=strTimeStr;
			break;
		default:
			strOutStr = concat(strDateStr, L' ', strTimeStr);
			break;
	}

	return fit_to_right(strOutStr, Width);
}

string FormatStr_Size(long long Size, const string& strName,
							DWORD FileAttributes,DWORD ShowFolderSize,DWORD ReparseTag,int ColumnType,
							unsigned long long Flags,int Width,const wchar_t *CurDir)
{
	string strResult;

	bool Packed=(ColumnType==PACKED_COLUMN);
	bool Streams=(ColumnType==STREAMSSIZE_COLUMN);

	if (ShowFolderSize==2)
	{
		Width--;
		strResult += L'~';
	}

	bool dir = (0 != (FileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	bool rpt = (0 != (FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT));
	bool have_size = !dir && (!rpt || ReparseTag==IO_REPARSE_TAG_DEDUP || Size > 0);

	if (!Streams && !Packed && !have_size && !ShowFolderSize)
	{
		auto TypeName = msg(lng::MListFolder);

		if (TestParentFolderName(strName))
		{
			TypeName = msg(lng::MListUp);
		}
		else
		{
			if (rpt)
			{
				switch(ReparseTag)
				{
				// 0xA0000003L = Directory Junction or Volume Mount Point
				case IO_REPARSE_TAG_MOUNT_POINT:
					{
						lng ID_Msg = lng::MListJunction;
						if (Global->Opt->PanelDetailedJunction && CurDir)
						{
							string strLinkName;
							if (GetReparsePointInfo(path::join(CurDir, PointToName(strName)), strLinkName))
							{
								NormalizeSymlinkName(strLinkName);
								bool Root;
								if(ParsePath(strLinkName, nullptr, &Root) == root_type::volume && Root)
								{
									ID_Msg = lng::MListVolMount;
								}
							}
						}
						TypeName=msg(ID_Msg);
					}
					break;
				// 0xA000000CL = Directory or File Symbolic Link
				case IO_REPARSE_TAG_SYMLINK:
					TypeName = msg(lng::MListSymlink);
					break;
				// 0x8000000AL = Distributed File System
				case IO_REPARSE_TAG_DFS:
					TypeName = msg(lng::MListDFS);
					break;
				// 0x80000012L = Distributed File System Replication
				case IO_REPARSE_TAG_DFSR:
					TypeName = msg(lng::MListDFSR);
					break;
				// 0xC0000004L = Hierarchical Storage Management
				case IO_REPARSE_TAG_HSM:
					TypeName = msg(lng::MListHSM);
					break;
				// 0x80000006L = Hierarchical Storage Management2
				case IO_REPARSE_TAG_HSM2:
					TypeName = msg(lng::MListHSM2);
					break;
				// 0x80000007L = Single Instance Storage
				case IO_REPARSE_TAG_SIS:
					TypeName = msg(lng::MListSIS);
					break;
				// 0x80000008L = Windows Imaging Format
				case IO_REPARSE_TAG_WIM:
					TypeName = msg(lng::MListWIM);
					break;
				// 0x80000009L = Cluster Shared Volumes
				case IO_REPARSE_TAG_CSV:
					TypeName = msg(lng::MListCSV);
					break;
				case IO_REPARSE_TAG_DEDUP:
					TypeName = msg(lng::MListDEDUP);
					break;
				case IO_REPARSE_TAG_NFS:
					TypeName = msg(lng::MListNFS);
					break;
				case IO_REPARSE_TAG_FILE_PLACEHOLDER:
					TypeName = msg(lng::MListPlaceholder);
					break;
					// 0x????????L = anything else
				default:
					if (Global->Opt->ShowUnknownReparsePoint)
					{
						TypeName = format(L":{0:0>8X}", ReparseTag);
					}
					else
					{
						TypeName = msg(lng::MListUnknownReparsePoint);
					}
				}
			}
		}

		if (static_cast<int>(TypeName.size()) <= Width-2 && msg(lng::MListBrackets)[0] && msg(lng::MListBrackets)[1])
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

int GetDefaultWidth(unsigned long long Type)
{
	int ColumnType = Type & 0xff;
	int Width = (ColumnType > CUSTOM_COLUMN0) ? 0 : ColumnInfo[ColumnType].DefaultWidth;

	if (ColumnType == WDATE_COLUMN || ColumnType == CDATE_COLUMN || ColumnType == ADATE_COLUMN || ColumnType == CHDATE_COLUMN)
	{
		if (Type & COLUMN_BRIEF)
			Width -= 3;

		if (Type & COLUMN_MONTH)
			++Width;
	}
	return Width;
}
