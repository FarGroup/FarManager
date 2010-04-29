/*
flmodes.cpp

Файловая панель - работа с режимами
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "filelist.hpp"
#include "lang.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "registry.hpp"
#include "interf.hpp"
#include "strmix.hpp"

int ColumnTypeWidth[]={ 0,  6,  6,  8,  5,  14,  14,  14,  6,  0,  0,  3,  3,  6,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0   };

static const wchar_t *ColumnSymbol[]={L"N",L"S",L"P",L"D",L"T",L"DM",L"DC",L"DA",L"A",L"Z",L"O",L"LN",L"F",L"G",L"C0",L"C1",L"C2",L"C3",L"C4",L"C5",L"C6",L"C7",L"C8",L"C9"};

PanelViewSettings ViewSettingsArray[]=
{
	/* 00 */{{COLUMN_MARK|NAME_COLUMN,SIZE_COLUMN|COLUMN_COMMAS,DATE_COLUMN},{0,10,0},3,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,0,1,0,0,0,0,0,{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH},{COUNT_WIDTH}},
	/* 01 */{{NAME_COLUMN,NAME_COLUMN,NAME_COLUMN},{0,0,0},3,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,1,0,0,0,0,0,{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH},{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH}},
	/* 02 */{{NAME_COLUMN,NAME_COLUMN},{0,0},2,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,0,0,0,0,0,0,{COUNT_WIDTH,COUNT_WIDTH},{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH}},
	/* 03 */{{NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,0,1,0,0,0,0,0,{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH},{COUNT_WIDTH}},
	/* 04 */{{NAME_COLUMN,SIZE_COLUMN},{0,6},2,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,0,0,0,0,0,0,{COUNT_WIDTH,COUNT_WIDTH},{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH}},
	/* 05 */{{NAME_COLUMN,SIZE_COLUMN,PACKED_COLUMN,MDATE_COLUMN,CDATE_COLUMN,ADATE_COLUMN,ATTR_COLUMN},{0,6,6,14,14,14,0},7,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,1,1,0,0,0,0,0,{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH},{COUNT_WIDTH}},
	/* 06 */{{NAME_COLUMN,DIZ_COLUMN},{40,0},2,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,1,0,0,0,0,0,{PERCENT_WIDTH,COUNT_WIDTH},{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH}},
	/* 07 */{{NAME_COLUMN,SIZE_COLUMN,DIZ_COLUMN},{0,6,54},3,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,1,1,0,0,0,0,0,{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH},{COUNT_WIDTH}},
	/* 08 */{{NAME_COLUMN,SIZE_COLUMN,OWNER_COLUMN},{0,6,15},3,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,1,0,0,0,0,0,{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH},{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH}},
	/* 09 */{{NAME_COLUMN,SIZE_COLUMN,NUMLINK_COLUMN},{0,6,3},3,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,1,0,0,0,0,0,{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH},{COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH,COUNT_WIDTH}}
};

size_t SizeViewSettingsArray=countof(ViewSettingsArray);

void FileList::SetFilePanelModes()
{
	int CurMode=0;

	if (CtrlObject->Cp()->ActivePanel->GetType()==FILE_PANEL)
	{
		CurMode=CtrlObject->Cp()->ActivePanel->GetViewMode();
		CurMode=(CurMode==0) ? 9:CurMode-1;
	}

	while (1)
	{
		MenuDataEx ModeListMenu[]=
		{
			(const wchar_t *)MEditPanelModesBrief,0,0,
			(const wchar_t *)MEditPanelModesMedium,0,0,
			(const wchar_t *)MEditPanelModesFull,0,0,
			(const wchar_t *)MEditPanelModesWide,0,0,
			(const wchar_t *)MEditPanelModesDetailed,0,0,
			(const wchar_t *)MEditPanelModesDiz,0,0,
			(const wchar_t *)MEditPanelModesLongDiz,0,0,
			(const wchar_t *)MEditPanelModesOwners,0,0,
			(const wchar_t *)MEditPanelModesLinks,0,0,
			(const wchar_t *)MEditPanelModesAlternative,0,0,
		};
		int ModeNumber;
		ModeListMenu[CurMode].SetSelect(1);
		{
			VMenu ModeList(MSG(MEditPanelModes),ModeListMenu,countof(ModeListMenu),ScrY-4);
			ModeList.SetPosition(-1,-1,0,0);
			ModeList.SetHelp(L"PanelViewModes");
			ModeList.SetFlags(VMENU_WRAPMODE);
			ModeList.Process();
			ModeNumber=ModeList.Modal::GetExitCode();
		}

		if (ModeNumber<0)
			return;

		CurMode=ModeNumber;
		DialogDataEx ModeDlgData[]=
		{
			DI_DOUBLEBOX, 3, 1,72,16,0,0,MSG((int)(DWORD_PTR)ModeListMenu[ModeNumber].Name),
			DI_TEXT,      5, 2, 0, 2,0,0,MSG(MEditPanelModeTypes),
			DI_EDIT,      5, 3,35, 3,0,DIF_FOCUS,L"",
			DI_TEXT,      5, 4, 0, 4,0,0,MSG(MEditPanelModeWidths),
			DI_EDIT,      5, 5,35, 5,0,0,L"",
			DI_TEXT,     38, 2, 0, 2,0,0,MSG(MEditPanelModeStatusTypes),
			DI_EDIT,     38, 3,70, 3,0,0,L"",
			DI_TEXT,     38, 4, 0, 4,0,0,MSG(MEditPanelModeStatusWidths),
			DI_EDIT,     38, 5,70, 5,0,0,L"",
			DI_TEXT,      3, 6, 0, 6,0,DIF_SEPARATOR,L"",
			DI_TEXT,     -1, 6, 0, 6,0,DIF_BOXCOLOR,MSG(MEditPanelReadHelp),
			DI_CHECKBOX,  5, 7, 0, 7,0,0,MSG(MEditPanelModeFullscreen),
			DI_CHECKBOX,  5, 8, 0, 8,0,0,MSG(MEditPanelModeAlignExtensions),
			DI_CHECKBOX,  5, 9, 0, 9,0,0,MSG(MEditPanelModeAlignFolderExtensions),
			DI_CHECKBOX,  5,10, 0,10,0,0,MSG(MEditPanelModeFoldersUpperCase),
			DI_CHECKBOX,  5,11, 0,11,0,0,MSG(MEditPanelModeFilesLowerCase),
			DI_CHECKBOX,  5,12, 0,12,0,0,MSG(MEditPanelModeUpperToLowerCase),
			DI_CHECKBOX,  5,13, 0,13,0,0,MSG(MEditPanelModeCaseSensitiveSort),
			DI_TEXT,      3,14, 0,14,0,DIF_SEPARATOR,L"",
			DI_BUTTON,    0,15, 0,15,0,DIF_DEFAULT|DIF_CENTERGROUP,MSG(MOk),
			DI_BUTTON,    0,15, 0,15,0,DIF_CENTERGROUP,MSG(MCancel),
		};
		MakeDialogItemsEx(ModeDlgData,ModeDlg);
		int ExitCode;
		RemoveHighlights(ModeDlg[0].strData);

		if (ModeNumber==9)
			ModeNumber=0;
		else
			ModeNumber++;

		PanelViewSettings NewSettings=ViewSettingsArray[ModeNumber];
		ModeDlg[11].Selected=NewSettings.FullScreen;
		ModeDlg[12].Selected=NewSettings.AlignExtensions;
		ModeDlg[13].Selected=NewSettings.FolderAlignExtensions;
		ModeDlg[14].Selected=NewSettings.FolderUpperCase;
		ModeDlg[15].Selected=NewSettings.FileLowerCase;
		ModeDlg[16].Selected=NewSettings.FileUpperToLowerCase;
		ModeDlg[17].Selected=NewSettings.CaseSensitiveSort;
		ViewSettingsToText(NewSettings.ColumnType,NewSettings.ColumnWidth,NewSettings.ColumnWidthType,
		                   NewSettings.ColumnCount,ModeDlg[2].strData,ModeDlg[4].strData);
		ViewSettingsToText(NewSettings.StatusColumnType,NewSettings.StatusColumnWidth,NewSettings.StatusColumnWidthType,
		                   NewSettings.StatusColumnCount,ModeDlg[6].strData,ModeDlg[8].strData);
		{
			Dialog Dlg(ModeDlg,countof(ModeDlg));
			Dlg.SetPosition(-1,-1,76,18);
			Dlg.SetHelp(L"PanelViewModes");
			Dlg.Process();
			ExitCode=Dlg.GetExitCode();
		}

		if (ExitCode!=19)
			continue;

		memset(&NewSettings,0,sizeof(NewSettings));
		NewSettings.FullScreen=ModeDlg[11].Selected;
		NewSettings.AlignExtensions=ModeDlg[12].Selected;
		NewSettings.FolderAlignExtensions=ModeDlg[13].Selected;
		NewSettings.FolderUpperCase=ModeDlg[14].Selected;
		NewSettings.FileLowerCase=ModeDlg[15].Selected;
		NewSettings.FileUpperToLowerCase=ModeDlg[16].Selected;
		NewSettings.CaseSensitiveSort=ModeDlg[17].Selected;
		TextToViewSettings(ModeDlg[2].strData,ModeDlg[4].strData,NewSettings.ColumnType,
		                   NewSettings.ColumnWidth,NewSettings.ColumnWidthType,NewSettings.ColumnCount);
		TextToViewSettings(ModeDlg[6].strData,ModeDlg[8].strData,NewSettings.StatusColumnType,
		                   NewSettings.StatusColumnWidth,NewSettings.StatusColumnWidthType,NewSettings.StatusColumnCount);
		ViewSettingsArray[ModeNumber]=NewSettings;
		CtrlObject->Cp()->LeftPanel->SortFileList(TRUE);
		CtrlObject->Cp()->RightPanel->SortFileList(TRUE);
		CtrlObject->Cp()->SetScreenPosition();
		int LeftMode=CtrlObject->Cp()->LeftPanel->GetViewMode();
		int RightMode=CtrlObject->Cp()->RightPanel->GetViewMode();
//    CtrlObject->Cp()->LeftPanel->SetViewMode(ModeNumber);
//    CtrlObject->Cp()->RightPanel->SetViewMode(ModeNumber);
		CtrlObject->Cp()->LeftPanel->SetViewMode(LeftMode);
		CtrlObject->Cp()->RightPanel->SetViewMode(RightMode);
		CtrlObject->Cp()->LeftPanel->Redraw();
		CtrlObject->Cp()->RightPanel->Redraw();
	}
}


void FileList::ReadPanelModes()
{
	for (int I=0; I<10; I++)
	{
		string strColumnTitles, strColumnWidths;
		string strStatusColumnTitles, strStatusColumnWidths, strRegKey;
		strRegKey.Format(L"Panel\\ViewModes\\Mode%d",I);
		GetRegKey(strRegKey,L"Columns",strColumnTitles,L"");
		GetRegKey(strRegKey,L"ColumnWidths",strColumnWidths,L"");
		GetRegKey(strRegKey,L"StatusColumns",strStatusColumnTitles,L"");
		GetRegKey(strRegKey,L"StatusColumnWidths",strStatusColumnWidths,L"");

		if (strColumnTitles.IsEmpty() || strColumnWidths.IsEmpty())
			continue;

		PanelViewSettings NewSettings=ViewSettingsArray[VIEW_0+I];

		if (!strColumnTitles.IsEmpty())
			TextToViewSettings(strColumnTitles,strColumnWidths,NewSettings.ColumnType,
			                   NewSettings.ColumnWidth,NewSettings.ColumnWidthType,NewSettings.ColumnCount);

		if (!strStatusColumnTitles.IsEmpty())
			TextToViewSettings(strStatusColumnTitles,strStatusColumnWidths,NewSettings.StatusColumnType,
			                   NewSettings.StatusColumnWidth,NewSettings.StatusColumnWidthType,NewSettings.StatusColumnCount);

		GetRegKey(strRegKey,L"FullScreen",NewSettings.FullScreen,0);
		GetRegKey(strRegKey,L"AlignExtensions",NewSettings.AlignExtensions,1);
		GetRegKey(strRegKey,L"FolderAlignExtensions",NewSettings.FolderAlignExtensions,0);
		GetRegKey(strRegKey,L"FolderUpperCase",NewSettings.FolderUpperCase,0);
		GetRegKey(strRegKey,L"FileLowerCase",NewSettings.FileLowerCase,0);
		GetRegKey(strRegKey,L"FileUpperToLowerCase",NewSettings.FileUpperToLowerCase,0);
		GetRegKey(strRegKey,L"CaseSensitiveSort",NewSettings.CaseSensitiveSort,0);
		ViewSettingsArray[VIEW_0+I]=NewSettings;
	}
}


void FileList::SavePanelModes()
{
	for (int I=0; I<10; I++)
	{
		string strColumnTitles, strColumnWidths;
		string strStatusColumnTitles, strStatusColumnWidths, strRegKey;
		strRegKey.Format(L"Panel\\ViewModes\\Mode%d",I);
		PanelViewSettings NewSettings=ViewSettingsArray[VIEW_0+I];
		ViewSettingsToText(NewSettings.ColumnType,NewSettings.ColumnWidth,NewSettings.ColumnWidthType,
		                   NewSettings.ColumnCount,strColumnTitles,strColumnWidths);
		ViewSettingsToText(NewSettings.StatusColumnType,NewSettings.StatusColumnWidth,NewSettings.StatusColumnWidthType,
		                   NewSettings.StatusColumnCount,strStatusColumnTitles,strStatusColumnWidths);
		SetRegKey(strRegKey,L"Columns",strColumnTitles);
		SetRegKey(strRegKey,L"ColumnWidths",strColumnWidths);
		SetRegKey(strRegKey,L"StatusColumns",strStatusColumnTitles);
		SetRegKey(strRegKey,L"StatusColumnWidths",strStatusColumnWidths);
		SetRegKey(strRegKey,L"FullScreen",NewSettings.FullScreen);
		SetRegKey(strRegKey,L"AlignExtensions",NewSettings.AlignExtensions);
		SetRegKey(strRegKey,L"FolderAlignExtensions",NewSettings.FolderAlignExtensions);
		SetRegKey(strRegKey,L"FolderUpperCase",NewSettings.FolderUpperCase);
		SetRegKey(strRegKey,L"FileLowerCase",NewSettings.FileLowerCase);
		SetRegKey(strRegKey,L"FileUpperToLowerCase",NewSettings.FileUpperToLowerCase);
		SetRegKey(strRegKey,L"CaseSensitiveSort",NewSettings.CaseSensitiveSort);
	}
}


void FileList::TextToViewSettings(const wchar_t *ColumnTitles,const wchar_t *ColumnWidths,
                                  unsigned int *ViewColumnTypes,int *ViewColumnWidths,int *ViewColumnWidthsTypes,int &ColumnCount)
{
	const wchar_t *TextPtr=ColumnTitles;

	for (ColumnCount=0; ColumnCount<(int)countof(ViewSettingsArray[0].ColumnType); ColumnCount++)
	{
		string strArgName;

		if ((TextPtr=GetCommaWord(TextPtr,strArgName))==nullptr)
			break;

		strArgName.Upper();

		if (strArgName.At(0)==L'N')
		{
			unsigned int &ColumnType=ViewColumnTypes[ColumnCount];
			ColumnType=NAME_COLUMN;
			const wchar_t *Ptr = (const wchar_t*)strArgName+1;

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
				const wchar_t *Ptr = (const wchar_t*)strArgName+1;

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
				if (StrCmpN(strArgName,L"DM",2)==0 || StrCmpN(strArgName,L"DC",2)==0 || StrCmpN(strArgName,L"DA",2)==0)
				{
					unsigned int &ColumnType=ViewColumnTypes[ColumnCount];

					switch (strArgName.At(1))
					{
						case L'M':
							ColumnType=MDATE_COLUMN;
							break;
						case L'C':
							ColumnType=CDATE_COLUMN;
							break;
						case L'A':
							ColumnType=ADATE_COLUMN;
							break;
					}

					const wchar_t *Ptr = (const wchar_t*)strArgName+2;

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
						for (unsigned I=0; I<countof(ColumnSymbol); I++)
						{
							if (StrCmp(strArgName,ColumnSymbol[I])==0)
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

		if ((TextPtr=GetCommaWord(TextPtr,strArgName))==nullptr)
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


void FileList::ViewSettingsToText(unsigned int *ViewColumnTypes,int *ViewColumnWidths,
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

		if (ColumnType==MDATE_COLUMN || ColumnType==ADATE_COLUMN || ColumnType==CDATE_COLUMN)
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
