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
#include "treelist.hpp"
#include "filelist.hpp"
#include "pathmix.hpp"
#include "panelctype.hpp"
#include "datetime.hpp"
#include "flink.hpp"
#include "plugins.hpp"
#include "language.hpp"
#include "fileattr.hpp"

static const struct column_info
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
	{ CUSTOM_COLUMN1, 0, L"C1" },
	{ CUSTOM_COLUMN2, 0, L"C2" },
	{ CUSTOM_COLUMN3, 0, L"C3" },
	{ CUSTOM_COLUMN4, 0, L"C4" },
	{ CUSTOM_COLUMN5, 0, L"C5" },
	{ CUSTOM_COLUMN6, 0, L"C6" },
	{ CUSTOM_COLUMN7, 0, L"C7" },
	{ CUSTOM_COLUMN8, 0, L"C8" },
	{ CUSTOM_COLUMN9, 0, L"C9" },
};

static_assert(ARRAYSIZE(ColumnInfo) == COLUMN_TYPES_COUNT, "wrong size of ColumnInfo array");

void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir)
{
	if (!SrcPanel)
		SrcPanel = Global->CtrlObject->Cp()->ActivePanel();

	Panel *AnotherPanel=Global->CtrlObject->Cp()->GetAnotherPanel(SrcPanel);

	switch (SrcPanel->GetType())
	{
		case QVIEW_PANEL:
		case INFO_PANEL:
			SrcPanel=Global->CtrlObject->Cp()->GetAnotherPanel(AnotherPanel=SrcPanel);
	}

	int AnotherType=AnotherPanel->GetType();

	if (AnotherType!=QVIEW_PANEL && AnotherType!=INFO_PANEL)
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
				if (AnotherType==FILE_PANEL)
					((FileList *)AnotherPanel)->ResetLastUpdateTime();

				AnotherPanel->UpdateIfChanged(false);
			}
		}
	}

	SrcPanel->Update(UPDATE_KEEP_SELECTION);

	if (AnotherType==QVIEW_PANEL)
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);

	Global->CtrlObject->Cp()->Redraw();
}

int CheckUpdateAnotherPanel(Panel *SrcPanel, const string& SelName)
{
	if (!SrcPanel)
		SrcPanel = Global->CtrlObject->Cp()->ActivePanel();

	Panel *AnotherPanel = Global->CtrlObject->Cp()->PassivePanel();
	AnotherPanel->CloseFile();

	if (AnotherPanel->GetMode() == NORMAL_PANEL)
	{
		string strFullName;
		string strAnotherCurDir(AnotherPanel->GetCurDir());
		AddEndSlash(strAnotherCurDir);
		ConvertNameToFull(SelName, strFullName);
		AddEndSlash(strFullName);

		if (strAnotherCurDir.find(strFullName) != string::npos)
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
			Panel *SrcPanel=nullptr;
			FilePanels *Cp=Global->CtrlObject->Cp();

			switch (Key)
			{
				case KEY_CTRLALTBRACKET:
				case KEY_RCTRLRALTBRACKET:
				case KEY_CTRLRALTBRACKET:
				case KEY_RCTRLALTBRACKET:
				case KEY_CTRLBRACKET:
				case KEY_RCTRLBRACKET:
					SrcPanel=Cp->LeftPanel;
					break;
				case KEY_CTRLALTBACKBRACKET:
				case KEY_RCTRLRALTBACKBRACKET:
				case KEY_CTRLRALTBACKBRACKET:
				case KEY_RCTRLALTBACKBRACKET:
				case KEY_CTRLBACKBRACKET:
				case KEY_RCTRLBACKBRACKET:
					SrcPanel=Cp->RightPanel;
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
					if (!(SrcPanel->GetType()==FILE_PANEL || SrcPanel->GetType()==TREE_PANEL))
						return FALSE;

					strPathName = SrcPanel->GetCurDir();

					if (SrcPanel->GetMode()!=PLUGIN_PANEL)
					{
						if (NeedRealName)
							SrcPanel->CreateFullPathName(strPathName, strPathName, FILE_ATTRIBUTE_DIRECTORY, strPathName, TRUE, ShortNameAsIs);

						if (SrcPanel->GetShowShortNamesMode() && ShortNameAsIs)
							ConvertNameToShort(strPathName,strPathName);
					}
					else
					{
						FileList *SrcFilePanel=(FileList *)SrcPanel;
						OpenPanelInfo Info;
						Global->CtrlObject->Plugins->GetOpenPanelInfo(SrcFilePanel->GetPluginHandle(),&Info);
						FileList::AddPluginPrefix(SrcFilePanel,strPathName);
						if (Info.HostFile && *Info.HostFile)
						{
							strPathName += Info.HostFile;
							strPathName += L"/";
						}
						strPathName += NullToEmpty(Info.CurDir);
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

		if (!(TextPtr=GetCommaWord(TextPtr,strArgName)))
			break;

		Columns.emplace_back(VALUE_TYPE(Columns)());

		string strArgOrig = strArgName;
		ToUpper(strArgName);

		if (strArgName.front() == L'N')
		{
			unsigned __int64 &ColumnType = Columns.back().type;
			ColumnType = NAME_COLUMN;
			const wchar_t *Ptr = strArgName.data() + 1;

			while (*Ptr)
			{
				switch (*Ptr)
				{
				case L'M':
					ColumnType |= COLUMN_MARK;
					break;
				case L'O':
					ColumnType |= COLUMN_NAMEONLY;
					break;
				case L'R':
					ColumnType |= COLUMN_RIGHTALIGN;
					break;
				case L'F':
					ColumnType |= COLUMN_RIGHTALIGNFORCE;
					break;
				case L'N':
					ColumnType |= COLUMN_NOEXTENSION;
					break;
				}

				Ptr++;
			}
		}
		else if (strArgName.front() == L'S' || strArgName.front() == L'P' || strArgName.front() == L'G')
		{
			unsigned __int64 &ColumnType = Columns.back().type;
			ColumnType = (strArgName.front() == L'S') ? SIZE_COLUMN : (strArgName.front() == L'P') ? PACKED_COLUMN : STREAMSSIZE_COLUMN;
			const wchar_t *Ptr = strArgName.data() + 1;

			while (*Ptr)
			{
				switch (*Ptr)
				{
				case L'C':
					ColumnType |= COLUMN_COMMAS;
					break;
				case L'E':
					ColumnType |= COLUMN_ECONOMIC;
					break;
				case L'F':
					ColumnType |= COLUMN_FLOATSIZE;
					break;
				case L'T':
					ColumnType |= COLUMN_THOUSAND;
					break;
				}

				Ptr++;
			}
		}
		else if (!StrCmpN(strArgName.data(), L"DM", 2) || !StrCmpN(strArgName.data(), L"DC", 2) || !StrCmpN(strArgName.data(), L"DA", 2) || !StrCmpN(strArgName.data(), L"DE", 2))
		{
			unsigned __int64 &ColumnType = Columns.back().type;

			switch (strArgName[1])
			{
			case L'M':
				ColumnType = WDATE_COLUMN;
				break;
			case L'C':
				ColumnType = CDATE_COLUMN;
				break;
			case L'A':
				ColumnType = ADATE_COLUMN;
				break;
			case L'E':
				ColumnType = CHDATE_COLUMN;
				break;
			}

			const wchar_t *Ptr = strArgName.data() + 2;

			while (*Ptr)
			{
				switch (*Ptr)
				{
				case L'B':
					ColumnType |= COLUMN_BRIEF;
					break;
				case L'M':
					ColumnType |= COLUMN_MONTH;
					break;
				}

				Ptr++;
			}
		}
		else if (strArgName.front() == L'O')
		{
			unsigned __int64 &ColumnType = Columns.back().type;
			ColumnType = OWNER_COLUMN;

			if (strArgName.size() > 1 && strArgName[1] == L'L')
				ColumnType |= COLUMN_FULLOWNER;
		}
		else if (strArgName.front() == L'X')
		{
			unsigned __int64 &ColumnType = Columns.back().type;
			ColumnType = EXTENSION_COLUMN;

			if (strArgName.size() > 1 && strArgName[1] == L'R')
				ColumnType |= COLUMN_RIGHTALIGN;
		}
		else if (strArgOrig.size() > 2 && strArgOrig.size()-2 < ARRAYSIZE(Columns.back().title) && strArgOrig.front() == L'<' && strArgOrig.back() == L'>')
		{
			wcsncpy(Columns.back().title, strArgOrig.data()+1, strArgOrig.size()-2);
			Columns.back().title[strArgOrig.size()-2] = 0;
			Columns.back().type = CUSTOM_COLUMN0;
		}
		else
		{
			auto ItemIterator = std::find_if(CONST_RANGE(ColumnInfo, i) { return strArgName == i.Symbol; });
			if (ItemIterator != std::cend(ColumnInfo))
				Columns.back().type = ItemIterator->Type;
		}
	}

	TextPtr=ColumnWidths.data();

	FOR(auto& i, Columns)
	{
		string strArgName;

		if (!(TextPtr=GetCommaWord(TextPtr,strArgName)))
			break;

		try
		{
			i.width = std::stoi(strArgName);
		}
		catch(const std::exception&)
		{
			// TODO: diagnostics
			i.width = 0;
		}

		i.width_type = COUNT_WIDTH;

		if (strArgName.size()>1)
		{
			switch (strArgName.back())
			{
				case L'%':
					i.width_type = PERCENT_WIDTH;
					break;
			}
		}
	}

	if (Columns.empty())
	{
		Columns.emplace_back(VALUE_TYPE(Columns)());
		Columns.front().type = NAME_COLUMN;
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
		// If ColumnType >= ARRAYSIZE(ColumnSymbol) ==> BUGBUG!!!
		strType = ColumnInfo[ColumnType].Symbol;

		if (ColumnType==NAME_COLUMN)
		{
			if (i.type & COLUMN_MARK)
				strType += L"M";

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

		if (ColumnType==CUSTOM_COLUMN0 && *i.title)
		{
			strType = string(L"<") + i.title + L">";
		}

		strColumnTitles += strType;

		strColumnWidths += std::to_wstring(i.width);

		switch (i.width_type)
		{
			case PERCENT_WIDTH:
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

const string FormatStr_Attribute(DWORD FileAttributes, size_t Width)
{
	string OutStr;

	enum_attributes([&](DWORD Attribute, wchar_t Character) -> bool
	{
		if (FileAttributes & Attribute)
		{
			OutStr.push_back(Character);
		}
		return OutStr.size() < Width;
	});

	return FormatString() << fmt::LeftAlign() << fmt::ExactWidth(Width) << OutStr;
}

const string FormatStr_DateTime(const FILETIME *FileTime,int ColumnType,unsigned __int64 Flags,int Width)
{
	FormatString strResult;

	if (Width < 0)
	{
		if (ColumnType == DATE_COLUMN)
			Width=0;
		else
			return strResult;
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

	strResult<<fmt::ExactWidth(Width)<<strOutStr;

	return strResult;
}

const string FormatStr_Size(__int64 FileSize, __int64 AllocationSize, __int64 StreamsSize, const string& strName,
							DWORD FileAttributes,DWORD ShowFolderSize,DWORD ReparseTag,int ColumnType,
							unsigned __int64 Flags,int Width,const wchar_t *CurDir)
{
	FormatString strResult;

	bool Packed=(ColumnType==PACKED_COLUMN);
	bool Streams=(ColumnType==STREAMSSIZE_COLUMN);

	if (ShowFolderSize==2)
	{
		Width--;
		strResult<<L"~";
	}

	if (!Streams && !Packed && FileAttributes & FILE_ATTRIBUTE_DIRECTORY && !ShowFolderSize)
	{
		string strMsg;
		const wchar_t *PtrName=MSG(MListFolder);

		if (TestParentFolderName(strName))
		{
			PtrName=MSG(MListUp);
		}
		else
		{
			if (FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
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
						strMsg = FormatString() << L":" << fmt::Radix(16) << fmt::ExactWidth(8) << fmt::FillChar(L'0') << ReparseTag;
						PtrName = strMsg.data();
					}
					else
					{
						PtrName=MSG(MListUnknownReparsePoint);
					}
				}
			}
		}

		FormatString strStr;
		if(*PtrName)
		{
			if (StrLength(PtrName) <= Width-2 && MSG(MListBrackets)[0] && MSG(MListBrackets)[1])
			{
				strStr << MSG(MListBrackets)[0] << PtrName << MSG(MListBrackets)[1];
			}
			else
			{
				strStr << PtrName;
			}
		}
		strResult<<fmt::ExactWidth(Width)<<strStr;
	}
	else
	{
		string strOutStr;
		strResult<<FileSizeToStr(strOutStr,Packed?AllocationSize:Streams?StreamsSize:FileSize,Width,Flags);
	}

	return strResult;
}

int GetDefaultWidth(uint64_t Type)
{
	int ColumnType = Type & 0xff;
	int Width = ColumnInfo[ColumnType].DefaultWidth;

	if (ColumnType == WDATE_COLUMN || ColumnType == CDATE_COLUMN || ColumnType == ADATE_COLUMN || ColumnType == CHDATE_COLUMN)
	{
		if (Type & COLUMN_BRIEF)
			Width -= 3;

		if (Type & COLUMN_MONTH)
			++Width;
	}
	return Width;
}
