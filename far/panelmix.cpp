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
#include "lang.hpp"
#include "datetime.hpp"

int ColumnTypeWidth[]={0, 6, 6, 8, 5, 14, 14, 14, 14, 6, 0, 0, 3, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static const wchar_t *ColumnSymbol[]={L"N",L"S",L"P",L"D",L"T",L"DM",L"DC",L"DA",L"DE",L"A",L"Z",L"O",L"LN",L"F",L"G",L"C0",L"C1",L"C2",L"C3",L"C4",L"C5",L"C6",L"C7",L"C8",L"C9"};


void ShellUpdatePanels(Panel *SrcPanel,BOOL NeedSetUpADir)
{
	if (!SrcPanel)
		SrcPanel=CtrlObject->Cp()->ActivePanel;

	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);

	switch (SrcPanel->GetType())
	{
		case QVIEW_PANEL:
		case INFO_PANEL:
			SrcPanel=CtrlObject->Cp()->GetAnotherPanel(AnotherPanel=SrcPanel);
	}

	int AnotherType=AnotherPanel->GetType();

	if (AnotherType!=QVIEW_PANEL && AnotherType!=INFO_PANEL)
	{
		if (NeedSetUpADir)
		{
			string strCurDir;
			SrcPanel->GetCurDir(strCurDir);
			AnotherPanel->SetCurDir(strCurDir,TRUE);
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

				AnotherPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
			}
		}
	}

	SrcPanel->Update(UPDATE_KEEP_SELECTION);

	if (AnotherType==QVIEW_PANEL)
		AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);

	CtrlObject->Cp()->Redraw();
}

int CheckUpdateAnotherPanel(Panel *SrcPanel,const wchar_t *SelName)
{
	if (!SrcPanel)
		SrcPanel=CtrlObject->Cp()->ActivePanel;

	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
	AnotherPanel->CloseFile();

	if (AnotherPanel->GetMode() == NORMAL_PANEL)
	{
		string strAnotherCurDir;
		string strFullName;
		AnotherPanel->GetCurDir(strAnotherCurDir);
		AddEndSlash(strAnotherCurDir);
		ConvertNameToFull(SelName, strFullName);
		AddEndSlash(strFullName);

		if (wcsstr(strAnotherCurDir,strFullName))
		{
			((FileList*)AnotherPanel)->CloseChangeNotification();
			return TRUE;
		}
	}

	return FALSE;
}

int _MakePath1(DWORD Key, string &strPathName, const wchar_t *Param2,int ShortNameAsIs)
{
	int RetCode=FALSE;
	int NeedRealName=FALSE;
	strPathName.Clear();

	switch (Key)
	{
		case KEY_CTRLALTBRACKET:       // Вставить сетевое (UNC) путь из левой панели
		case KEY_CTRLALTBACKBRACKET:   // Вставить сетевое (UNC) путь из правой панели
		case KEY_ALTSHIFTBRACKET:      // Вставить сетевое (UNC) путь из активной панели
		case KEY_ALTSHIFTBACKBRACKET:  // Вставить сетевое (UNC) путь из пассивной панели
			NeedRealName=TRUE;
		case KEY_CTRLBRACKET:          // Вставить путь из левой панели
		case KEY_CTRLBACKBRACKET:      // Вставить путь из правой панели
		case KEY_CTRLSHIFTBRACKET:     // Вставить путь из активной панели
		case KEY_CTRLSHIFTBACKBRACKET: // Вставить путь из пассивной панели
		case KEY_CTRLSHIFTNUMENTER:    // Текущий файл с пасс.панели
		case KEY_SHIFTNUMENTER:        // Текущий файл с актив.панели
		case KEY_CTRLSHIFTENTER:       // Текущий файл с пасс.панели
		case KEY_SHIFTENTER:           // Текущий файл с актив.панели
		{
			Panel *SrcPanel=nullptr;
			FilePanels *Cp=CtrlObject->Cp();

			switch (Key)
			{
				case KEY_CTRLALTBRACKET:
				case KEY_CTRLBRACKET:
					SrcPanel=Cp->LeftPanel;
					break;
				case KEY_CTRLALTBACKBRACKET:
				case KEY_CTRLBACKBRACKET:
					SrcPanel=Cp->RightPanel;
					break;
				case KEY_SHIFTNUMENTER:
				case KEY_SHIFTENTER:
				case KEY_ALTSHIFTBRACKET:
				case KEY_CTRLSHIFTBRACKET:
					SrcPanel=Cp->ActivePanel;
					break;
				case KEY_CTRLSHIFTNUMENTER:
				case KEY_CTRLSHIFTENTER:
				case KEY_ALTSHIFTBACKBRACKET:
				case KEY_CTRLSHIFTBACKBRACKET:
					SrcPanel=Cp->GetAnotherPanel(Cp->ActivePanel);
					break;
			}

			if (SrcPanel)
			{
				if (Key == KEY_SHIFTENTER || Key == KEY_CTRLSHIFTENTER || Key == KEY_SHIFTNUMENTER || Key == KEY_CTRLSHIFTNUMENTER)
				{
					string strShortFileName;
					SrcPanel->GetCurName(strPathName,strShortFileName);

					if (SrcPanel->GetShowShortNamesMode()) // учтем короткость имен :-)
						strPathName = strShortFileName;
				}
				else
				{
					/* TODO: Здесь нужно учесть, что у TreeList тоже есть путь :-) */
					if (!(SrcPanel->GetType()==FILE_PANEL || SrcPanel->GetType()==TREE_PANEL))
						return FALSE;

					SrcPanel->GetCurDir(strPathName);

					if (SrcPanel->GetMode()!=PLUGIN_PANEL)
					{
						FileList *SrcFilePanel=(FileList *)SrcPanel;
						SrcFilePanel->GetCurDir(strPathName);
						{
							if (NeedRealName)
								SrcFilePanel->CreateFullPathName(strPathName, strPathName,FILE_ATTRIBUTE_DIRECTORY, strPathName,TRUE,ShortNameAsIs);
						}

						if (SrcFilePanel->GetShowShortNamesMode() && ShortNameAsIs)
							ConvertNameToShort(strPathName,strPathName);
					}
					else
					{
						FileList *SrcFilePanel=(FileList *)SrcPanel;
						OpenPanelInfo Info;
						CtrlObject->Plugins.GetOpenPanelInfo(SrcFilePanel->GetPluginHandle(),&Info);
						FileList::AddPluginPrefix(SrcFilePanel,strPathName);
						if (Info.HostFile && *Info.HostFile)
						{
							strPathName += Info.HostFile;
							strPathName += L"/";
						}
						strPathName += Info.CurDir;
					}

					AddEndSlash(strPathName);
				}

				if (Opt.QuotedName&QUOTEDNAME_INSERT)
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


void TextToViewSettings(const wchar_t *ColumnTitles,const wchar_t *ColumnWidths,
                                  unsigned int *ViewColumnTypes,int *ViewColumnWidths,int *ViewColumnWidthsTypes,int &ColumnCount)
{
	const wchar_t *TextPtr=ColumnTitles;

	for (ColumnCount=0; ColumnCount < PANEL_COLUMNCOUNT; ColumnCount++)
	{
		string strArgName;

		if (!(TextPtr=GetCommaWord(TextPtr,strArgName)))
			break;

		strArgName.Upper();

		if (strArgName.At(0)==L'N')
		{
			unsigned int &ColumnType=ViewColumnTypes[ColumnCount];
			ColumnType=NAME_COLUMN;
			const wchar_t *Ptr = strArgName.CPtr()+1;

			while (*Ptr)
			{
				switch (*Ptr)
				{
					case L'M':
						ColumnType|=COLUMN_MARK;
						break;
					case L'O':
						ColumnType|=COLUMN_NAMEONLY;
						break;
					case L'R':
						ColumnType|=COLUMN_RIGHTALIGN;
						break;
				}

				Ptr++;
			}
		}
		else
		{
			if (strArgName.At(0)==L'S' || strArgName.At(0)==L'P' || strArgName.At(0)==L'G')
			{
				unsigned int &ColumnType=ViewColumnTypes[ColumnCount];
				ColumnType=(strArgName.At(0)==L'S') ? SIZE_COLUMN:(strArgName.At(0)==L'P')?PACKED_COLUMN:STREAMSSIZE_COLUMN;
				const wchar_t *Ptr = strArgName.CPtr()+1;

				while (*Ptr)
				{
					switch (*Ptr)
					{
						case L'C':
							ColumnType|=COLUMN_COMMAS;
							break;
						case L'E':
							ColumnType|=COLUMN_ECONOMIC;
							break;
						case L'F':
							ColumnType|=COLUMN_FLOATSIZE;
							break;
						case L'T':
							ColumnType|=COLUMN_THOUSAND;
							break;
					}

					Ptr++;
				}
			}
			else
			{
				if (!StrCmpN(strArgName,L"DM",2) || !StrCmpN(strArgName,L"DC",2) || !StrCmpN(strArgName,L"DA",2) || !StrCmpN(strArgName,L"DE",2))
				{
					unsigned int &ColumnType=ViewColumnTypes[ColumnCount];

					switch (strArgName.At(1))
					{
						case L'M':
							ColumnType=WDATE_COLUMN;
							break;
						case L'C':
							ColumnType=CDATE_COLUMN;
							break;
						case L'A':
							ColumnType=ADATE_COLUMN;
							break;
						case L'E':
							ColumnType=CHDATE_COLUMN;
							break;
					}

					const wchar_t *Ptr = strArgName.CPtr()+2;

					while (*Ptr)
					{
						switch (*Ptr)
						{
							case L'B':
								ColumnType|=COLUMN_BRIEF;
								break;
							case L'M':
								ColumnType|=COLUMN_MONTH;
								break;
						}

						Ptr++;
					}
				}
				else
				{
					if (strArgName.At(0)==L'O')
					{
						unsigned int &ColumnType=ViewColumnTypes[ColumnCount];
						ColumnType=OWNER_COLUMN;

						if (strArgName.At(1)==L'L')
							ColumnType|=COLUMN_FULLOWNER;
					}
					else
					{
						for (unsigned I=0; I<ARRAYSIZE(ColumnSymbol); I++)
						{
							if (!StrCmp(strArgName,ColumnSymbol[I]))
							{
								ViewColumnTypes[ColumnCount]=I;
								break;
							}
						}
					}
				}
			}
		}
	}

	TextPtr=ColumnWidths;

	for (int I=0; I<ColumnCount; I++)
	{
		string strArgName;

		if (!(TextPtr=GetCommaWord(TextPtr,strArgName)))
			break;

		ViewColumnWidths[I]=_wtoi(strArgName);
		ViewColumnWidthsTypes[I]=COUNT_WIDTH;

		if (strArgName.GetLength()>1)
		{
			switch (strArgName.At(strArgName.GetLength()-1))
			{
				case L'%':
					ViewColumnWidthsTypes[I]=PERCENT_WIDTH;
					break;
			}
		}
	}
}


void ViewSettingsToText(unsigned int *ViewColumnTypes,int *ViewColumnWidths,
                                  int *ViewColumnWidthsTypes,int ColumnCount,string &strColumnTitles,
                                  string &strColumnWidths)
{
	strColumnTitles.Clear();
	strColumnWidths.Clear();

	for (int I=0; I<ColumnCount; I++)
	{
		string strType;
		int ColumnType=ViewColumnTypes[I] & 0xff;
		strType = ColumnSymbol[ColumnType];

		if (ColumnType==NAME_COLUMN)
		{
			if (ViewColumnTypes[I] & COLUMN_MARK)
				strType += L"M";

			if (ViewColumnTypes[I] & COLUMN_NAMEONLY)
				strType += L"O";

			if (ViewColumnTypes[I] & COLUMN_RIGHTALIGN)
				strType += L"R";
		}

		if (ColumnType==SIZE_COLUMN || ColumnType==PACKED_COLUMN || ColumnType==STREAMSSIZE_COLUMN)
		{
			if (ViewColumnTypes[I] & COLUMN_COMMAS)
				strType += L"C";

			if (ViewColumnTypes[I] & COLUMN_ECONOMIC)
				strType += L"E";

			if (ViewColumnTypes[I] & COLUMN_FLOATSIZE)
				strType += L"F";

			if (ViewColumnTypes[I] & COLUMN_THOUSAND)
				strType += L"T";
		}

		if (ColumnType==WDATE_COLUMN || ColumnType==ADATE_COLUMN || ColumnType==CDATE_COLUMN  || ColumnType==CHDATE_COLUMN)
		{
			if (ViewColumnTypes[I] & COLUMN_BRIEF)
				strType += L"B";

			if (ViewColumnTypes[I] & COLUMN_MONTH)
				strType += L"M";
		}

		if (ColumnType==OWNER_COLUMN)
		{
			if (ViewColumnTypes[I] & COLUMN_FULLOWNER)
				strType += L"L";
		}

		strColumnTitles += strType;
		wchar_t *lpwszWidth = strType.GetBuffer(20);
		_itow(ViewColumnWidths[I],lpwszWidth,10);
		strType.ReleaseBuffer();
		strColumnWidths += strType;

		switch (ViewColumnWidthsTypes[I])
		{
			case PERCENT_WIDTH:
				strColumnWidths += L"%";
				break;
		}

		if (I<ColumnCount-1)
		{
			strColumnTitles += L",";
			strColumnWidths += L",";
		}
	}
}

const string FormatStr_Attribute(DWORD FileAttributes,int Width)
{
	FormatString strResult;

	const wchar_t OutStr[]=
	{
		FileAttributes&FILE_ATTRIBUTE_READONLY?L'R':L' ',
		FileAttributes&FILE_ATTRIBUTE_SYSTEM?L'S':L' ',
		FileAttributes&FILE_ATTRIBUTE_HIDDEN?L'H':L' ',
		FileAttributes&FILE_ATTRIBUTE_ARCHIVE?L'A':L' ',
		FileAttributes&FILE_ATTRIBUTE_REPARSE_POINT?L'L':FileAttributes&FILE_ATTRIBUTE_SPARSE_FILE?L'$':L' ',
		FileAttributes&FILE_ATTRIBUTE_COMPRESSED?L'C':FileAttributes&FILE_ATTRIBUTE_ENCRYPTED?L'E':L' ',
		FileAttributes&FILE_ATTRIBUTE_TEMPORARY?L'T':L' ',
		FileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?L'I':L' ',
		FileAttributes&FILE_ATTRIBUTE_OFFLINE?L'O':L' ',
		FileAttributes&FILE_ATTRIBUTE_VIRTUAL?L'V':L' ',
		0
	};

	if (Width > 0)
		strResult<<fmt::Width(Width)<<fmt::Precision(Width);

	strResult<<OutStr;

	return strResult.strValue();
}

const string FormatStr_DateTime(const FILETIME *FileTime,int ColumnType,DWORD Flags,int Width)
{
	FormatString strResult;

	if (Width < 0)
	{
		if (ColumnType == DATE_COLUMN)
			Width=0;
		else
			return strResult.strValue();
	}

	int ColumnWidth=Width;
	int Brief=Flags & COLUMN_BRIEF;
	int TextMonth=Flags & COLUMN_MONTH;
	int FullYear=FALSE;

	switch(ColumnType)
	{
		case DATE_COLUMN:
		case TIME_COLUMN:
		{
			Brief=FALSE;
			TextMonth=FALSE;
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
				int CmpWidth=ColumnWidth-TextMonth;

				if (CmpWidth==15 || CmpWidth==16 || CmpWidth==18 || CmpWidth==19 || CmpWidth>21)
					FullYear=TRUE;
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

	strResult<<fmt::Width(Width)<<fmt::Precision(Width)<<strOutStr;

	return strResult.strValue();
}

const string FormatStr_Size(__int64 UnpSize, __int64 PackSize, __int64 StreamsSize, const string& strName,DWORD FileAttributes,DWORD ShowFolderSize,DWORD ReparseTag,int ColumnType,DWORD Flags,int Width)
{
	FormatString strResult;

	bool Packed=(ColumnType==PACKED_COLUMN);
	bool Streams=(ColumnType==STREAMSSIZE_COLUMN);

	if (ShowFolderSize==2)
	{
		Width--;
		strResult<<L"~";
	}

	if (!Streams && !Packed && (FileAttributes & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)) && !ShowFolderSize)
	{
		const wchar_t *PtrName=MSG(MListFolder);

		if (TestParentFolderName(strName))
		{
			PtrName=MSG(MListUp);
		}
		else
		{
			if (FileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
			{
				PtrName=MSG(MListSymLink);
				switch (ReparseTag)
				{
					case IO_REPARSE_TAG_SYMLINK:
						break;

					case IO_REPARSE_TAG_MOUNT_POINT:
						PtrName=MSG(MListJunction);
						break;

					//case IO_REPARSE_TAG_DRIVER_EXTENDER:
						//...
						//break;
					//case ...
				}
			}
		}

		string strStr;
		if (StrLength(PtrName) <= Width-2)
			strStr.Format(L"<%s>", PtrName);
		else
			strStr = PtrName;

		strResult<<fmt::Width(Width)<<fmt::Precision(Width)<<strStr;
	}
	else
	{
		string strOutStr;
		strResult<<FileSizeToStr(strOutStr,Packed?PackSize:Streams?StreamsSize:UnpSize,Width,Flags).CPtr();
	}

	return strResult.strValue();
}
