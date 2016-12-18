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
#include "language.hpp"
#include "fileattr.hpp"
#include "colormix.hpp"

static constexpr struct column_info
{
	PANEL_COLUMN_TYPE Type;
	int DefaultWidth;
	const wchar_t* Symbol;
}
ColumnInfo[] =
{
	{ NAME_COLUMN, 0, L"N" },
	{ SIZE_COLUMN, 6, L"S" },
	{ PACKED_COLUMN, 6, L"P" },
	{ DATE_COLUMN, 8, L"D" },
	{ TIME_COLUMN, 5, L"T" },
	{ WDATE_COLUMN, 14, L"DM" },
	{ CDATE_COLUMN, 14, L"DC" },
	{ ADATE_COLUMN, 14, L"DA" },
	{ CHDATE_COLUMN, 14, L"DE" },
	{ ATTR_COLUMN, 6, L"A" },
	{ DIZ_COLUMN, 0, L"Z" },
	{ OWNER_COLUMN, 0, L"O" },
	{ NUMLINK_COLUMN, 3, L"LN" },
	{ NUMSTREAMS_COLUMN, 3, L"F" },
	{ STREAMSSIZE_COLUMN, 6, L"G" },
	{ EXTENSION_COLUMN, 0, L"X", },
	{ CUSTOM_COLUMN0, 0, L"C0" },
};

TERSE_STATIC_ASSERT(std::size(ColumnInfo) == COLUMN_TYPES_COUNT);

void ShellUpdatePanels(panel_ptr SrcPanel,BOOL NeedSetUpADir)
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

int CheckUpdateAnotherPanel(panel_ptr SrcPanel, const string& SelName)
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
			return TRUE;
		}
	}

	return FALSE;
}

int _MakePath1(DWORD Key, string &strPathName, const wchar_t *Param2,int ShortNameAsIs)
{
	int RetCode=FALSE;
	int NeedRealName=FALSE;
	strPathName.clear();

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
			NeedRealName=TRUE;
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
			FilePanels *Cp=Global->CtrlObject->Cp();

			switch (Key)
			{
				case KEY_CTRLALTBRACKET:
				case KEY_RCTRLRALTBRACKET:
				case KEY_CTRLRALTBRACKET:
				case KEY_RCTRLALTBRACKET:
				case KEY_CTRLBRACKET:
				case KEY_RCTRLBRACKET:
					SrcPanel=Cp->LeftPanel();
					break;
				case KEY_CTRLALTBACKBRACKET:
				case KEY_RCTRLRALTBACKBRACKET:
				case KEY_CTRLRALTBACKBRACKET:
				case KEY_RCTRLALTBACKBRACKET:
				case KEY_CTRLBACKBRACKET:
				case KEY_RCTRLBACKBRACKET:
					SrcPanel=Cp->RightPanel();
					break;
				case KEY_SHIFTNUMENTER:
				case KEY_SHIFTENTER:
				case KEY_ALTSHIFTBRACKET:
				case KEY_RALTSHIFTBRACKET:
				case KEY_CTRLSHIFTBRACKET:
				case KEY_RCTRLSHIFTBRACKET:
					SrcPanel=Cp->ActivePanel();
					break;
				case KEY_CTRLSHIFTNUMENTER:
				case KEY_RCTRLSHIFTNUMENTER:
				case KEY_CTRLSHIFTENTER:
				case KEY_RCTRLSHIFTENTER:
				case KEY_ALTSHIFTBACKBRACKET:
				case KEY_RALTSHIFTBACKBRACKET:
				case KEY_CTRLSHIFTBACKBRACKET:
				case KEY_RCTRLSHIFTBACKBRACKET:
					SrcPanel=Cp->PassivePanel();
					break;
			}

			if (SrcPanel)
			{
				if (Key == KEY_SHIFTENTER || Key == KEY_CTRLSHIFTENTER || Key == KEY_RCTRLSHIFTENTER || Key == KEY_SHIFTNUMENTER || Key == KEY_CTRLSHIFTNUMENTER || Key == KEY_RCTRLSHIFTNUMENTER)
				{
					string strShortFileName;
					SrcPanel->GetCurName(strPathName,strShortFileName);

					if (SrcPanel->GetShowShortNamesMode()) // учтем короткость имен :-)
						strPathName = strShortFileName;
				}
				else
				{
					if (!(SrcPanel->GetType() == panel_type::FILE_PANEL || SrcPanel->GetType() == panel_type::TREE_PANEL))
						return FALSE;

					strPathName = SrcPanel->GetCurDir();

					if (SrcPanel->GetMode() != panel_mode::PLUGIN_PANEL)
					{
						if (NeedRealName)
							SrcPanel->CreateFullPathName(strPathName, strPathName, FILE_ATTRIBUTE_DIRECTORY, strPathName, TRUE, ShortNameAsIs);

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
								strPathName += Info.HostFile;
								strPathName += L"/";
							}
							strPathName += NullToEmpty(Info.CurDir);
						}
					}

					AddEndSlash(strPathName);
				}

				if (Global->Opt->QuotedName&QUOTEDNAME_INSERT)
					QuoteSpace(strPathName);

				if (Param2)
					strPathName += Param2;

				RetCode=TRUE;
			}
		}
		break;
	}

	return RetCode;
}


void TextToViewSettings(const string& ColumnTitles,const string& ColumnWidths, std::vector<column>& Columns)
{
	// BUGBUG, add error checking

	const wchar_t *TextPtr=ColumnTitles.data();

	Columns.clear();

	for (;;)
	{
		string strArgName;
		TextPtr = GetCommaWord(TextPtr, strArgName);
		if (!TextPtr)
			break;

		column NewColumn;

		auto strArgOrig = Upper(strArgName);

		if (strArgName.front() == L'N')
		{
			NewColumn.type = NAME_COLUMN;
			for (const auto& i: make_range(strArgName.cbegin() + 1, strArgName.cend()))
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
		else if (strArgName.front() == L'S' || strArgName.front() == L'P' || strArgName.front() == L'G')
		{
			NewColumn.type = (strArgName.front() == L'S') ? SIZE_COLUMN : (strArgName.front() == L'P') ? PACKED_COLUMN : STREAMSSIZE_COLUMN;
			for (const auto& i: make_range(strArgName.cbegin() + 1, strArgName.cend()))
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
		else if (!StrCmpN(strArgName.data(), L"DM", 2) || !StrCmpN(strArgName.data(), L"DC", 2) || !StrCmpN(strArgName.data(), L"DA", 2) || !StrCmpN(strArgName.data(), L"DE", 2))
		{
			switch (strArgName[1])
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

			for (const auto& i: make_range(strArgName.cbegin() + 2, strArgName.cend()))
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
		else if (strArgName.front() == L'O')
		{
			NewColumn.type = OWNER_COLUMN;

			if (strArgName.size() > 1 && strArgName[1] == L'L')
				NewColumn.type |= COLUMN_FULLOWNER;
		}
		else if (strArgName.front() == L'X')
		{
			NewColumn.type = EXTENSION_COLUMN;

			if (strArgName.size() > 1 && strArgName[1] == L'R')
				NewColumn.type |= COLUMN_RIGHTALIGN;
		}
		else if (strArgOrig.size() > 2 && strArgOrig.front() == L'<' && strArgOrig.back() == L'>')
		{
			NewColumn.title = strArgOrig.substr(1, strArgOrig.size() - 2);
			NewColumn.type = CUSTOM_COLUMN0;
		}
		else
		{
			const auto ItemIterator = std::find_if(CONST_RANGE(ColumnInfo, i) { return strArgName == i.Symbol; });
			if (ItemIterator != std::cend(ColumnInfo))
				NewColumn.type = ItemIterator->Type;
			else if (strArgOrig.size() >= 2 && strArgOrig.size() <= 3 && strArgOrig.front() == L'C')
			{
				unsigned int num;
				if (1 == swscanf(strArgOrig.data()+1, L"%u", &num))
					NewColumn.type = CUSTOM_COLUMN0 + num;
			}
		}

		Columns.emplace_back(NewColumn);
	}

	TextPtr=ColumnWidths.data();

	for (auto& i: Columns)
	{
		string strArgName;
		TextPtr = GetCommaWord(TextPtr, strArgName);
		if (!TextPtr)
			break;

		try
		{
			i.width = std::stoi(strArgName);
		}
		catch (const std::exception&)
		{
			// TODO: diagnostics
			i.width = 0;
		}

		i.width_type = col_width::fixed;

		if (strArgName.size()>1)
		{
			switch (strArgName.back())
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
}


void ViewSettingsToText(const std::vector<column>& Columns, string &strColumnTitles, string &strColumnWidths)
{
	strColumnTitles.clear();
	strColumnWidths.clear();

	std::for_each(CONST_RANGE(Columns, i)
	{
		string strType;
		int ColumnType=static_cast<int>(i.type & 0xff);
		// If ColumnType >= std::size(ColumnSymbol) ==> BUGBUG!!!
		if (ColumnType <= CUSTOM_COLUMN0)
		{
			strType = ColumnInfo[ColumnType].Symbol;
		}
		else
		{
			strType = L"C" + str(ColumnType - CUSTOM_COLUMN0);
		}

		if (ColumnType==NAME_COLUMN)
		{
			if (i.type & COLUMN_MARK)
			{
				strType += L"M";
				if (i.type & COLUMN_MARK_DYNAMIC)
					strType += L"D";
			}

			if (i.type & COLUMN_NAMEONLY)
				strType += L"O";

			if (i.type & COLUMN_RIGHTALIGN)
			{
				strType += L"R";
				if (i.type & COLUMN_RIGHTALIGNFORCE)
					strType += L"F";
			}

			if (i.type & COLUMN_NOEXTENSION)
				strType += L"N";
		}

		if (ColumnType==SIZE_COLUMN || ColumnType==PACKED_COLUMN || ColumnType==STREAMSSIZE_COLUMN)
		{
			if (i.type & COLUMN_COMMAS)
				strType += L"C";

			if (i.type & COLUMN_ECONOMIC)
				strType += L"E";

			if (i.type & COLUMN_FLOATSIZE)
				strType += L"F";

			if (i.type & COLUMN_THOUSAND)
				strType += L"T";
		}

		if (ColumnType==WDATE_COLUMN || ColumnType==ADATE_COLUMN || ColumnType==CDATE_COLUMN  || ColumnType==CHDATE_COLUMN)
		{
			if (i.type & COLUMN_BRIEF)
				strType += L"B";

			if (i.type & COLUMN_MONTH)
				strType += L"M";
		}

		if (ColumnType==OWNER_COLUMN)
		{
			if (i.type & COLUMN_FULLOWNER)
				strType += L"L";
		}

		if (ColumnType==EXTENSION_COLUMN)
		{
			if (i.type & COLUMN_RIGHTALIGN)
				strType += L"R";
		}

		if (ColumnType==CUSTOM_COLUMN0 && !i.title.empty())
		{
			strType = L"<" + i.title + L">";
		}

		strColumnTitles += strType;

		strColumnWidths += str(i.width);

		switch (i.width_type)
		{
			case col_width::percent:
				strColumnWidths += L"%";
				break;
		}

		strColumnTitles += L",";
		strColumnWidths += L",";
	});

	if (!strColumnTitles.empty())
		strColumnTitles.pop_back();
	if (!strColumnWidths.empty())
		strColumnWidths.pop_back();
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

	return fit_to_left(OutStr, Width);
}

string FormatStr_DateTime(const FILETIME* FileTime, int ColumnType, unsigned long long Flags, int Width)
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

	ConvertDate(*FileTime,strDateStr,strTimeStr,ColumnWidth,Brief,TextMonth,FullYear);

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
			strOutStr=strDateStr+L" "+strTimeStr;
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
		string strMsg;
		const wchar_t *PtrName=MSG(MListFolder);

		if (TestParentFolderName(strName))
		{
			PtrName=MSG(MListUp);
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
						LNGID ID_Msg = MListJunction;
						if (Global->Opt->PanelDetailedJunction)
						{
							string strLinkName=CurDir?CurDir:L"";
							AddEndSlash(strLinkName);
							strLinkName+=PointToName(strName);

							if (GetReparsePointInfo(strLinkName, strLinkName))
							{
								NormalizeSymlinkName(strLinkName);
								bool Root;
								if(ParsePath(strLinkName, nullptr, &Root) == PATH_VOLUMEGUID && Root)
								{
									ID_Msg=MListVolMount;
								}
							}
						}
						PtrName=MSG(ID_Msg);
					}
					break;
				// 0xA000000CL = Directory or File Symbolic Link
				case IO_REPARSE_TAG_SYMLINK:
					PtrName = MSG(MListSymlink);
					break;
				// 0x8000000AL = Distributed File System
				case IO_REPARSE_TAG_DFS:
					PtrName = MSG(MListDFS);
					break;
				// 0x80000012L = Distributed File System Replication
				case IO_REPARSE_TAG_DFSR:
					PtrName = MSG(MListDFSR);
					break;
				// 0xC0000004L = Hierarchical Storage Management
				case IO_REPARSE_TAG_HSM:
					PtrName = MSG(MListHSM);
					break;
				// 0x80000006L = Hierarchical Storage Management2
				case IO_REPARSE_TAG_HSM2:
					PtrName = MSG(MListHSM2);
					break;
				// 0x80000007L = Single Instance Storage
				case IO_REPARSE_TAG_SIS:
					PtrName = MSG(MListSIS);
					break;
				// 0x80000008L = Windows Imaging Format
				case IO_REPARSE_TAG_WIM:
					PtrName = MSG(MListWIM);
					break;
				// 0x80000009L = Cluster Shared Volumes
				case IO_REPARSE_TAG_CSV:
					PtrName = MSG(MListCSV);
					break;
				case IO_REPARSE_TAG_DEDUP:
					PtrName = MSG(MListDEDUP);
					break;
				case IO_REPARSE_TAG_NFS:
					PtrName = MSG(MListNFS);
					break;
				case IO_REPARSE_TAG_FILE_PLACEHOLDER:
					PtrName = MSG(MListPlaceholder);
					break;
					// 0x????????L = anything else
				default:
					if (Global->Opt->ShowUnknownReparsePoint)
					{
						strMsg = format(L":{0:0>8X}", ReparseTag);
						PtrName = strMsg.data();
					}
					else
					{
						PtrName=MSG(MListUnknownReparsePoint);
					}
				}
			}
		}

		string strStr;
		if(*PtrName)
		{
			if (StrLength(PtrName) <= Width-2 && MSG(MListBrackets)[0] && MSG(MListBrackets)[1])
			{
				strStr = concat(MSG(MListBrackets)[0], PtrName, MSG(MListBrackets)[1]);
			}
			else
			{
				strStr = PtrName;
			}
		}
		strResult += fit_to_right(strStr, Width);
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
