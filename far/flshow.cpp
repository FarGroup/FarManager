/*
flshow.cpp

Файловая панель - вывод на экран
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

#include "filelist.hpp"
#include "colors.hpp"
#include "filefilter.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "datetime.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "flink.hpp"
#include "panelmix.hpp"
#include "valuename.hpp"
#include "colormix.hpp"
#include "lang.hpp"
#include "plugins.hpp"

extern int ColumnTypeWidth[];

static wchar_t OutCharacter[8]={};

static LNGID __FormatEndSelectedPhrase(size_t Count)
{
	LNGID M_Fmt=MListFileSize;

	if (Count != 1)
	{
		FormatString StrItems;
		StrItems << Count;
		size_t LenItems= StrItems.size();

		if (StrItems.at(LenItems-1) == '1' && Count != 11)
			M_Fmt=MListFilesSize1;
		else
			M_Fmt=MListFilesSize2;
	}

	return M_Fmt;
}


void FileList::DisplayObject()
{
	Height=Y2-Y1-4+!Global->Opt->ShowColumnTitles+(Global->Opt->ShowPanelStatus ? 0:2);
	_OT(SysLog(L"[%p] FileList::DisplayObject()",this));

	if (UpdateRequired)
	{
		UpdateRequired=FALSE;
		Update(UpdateRequiredMode);
	}

	ProcessPluginCommand();
	ShowFileList(FALSE);
}


void FileList::ShowFileList(int Fast)
{
	if (Locked())
	{
		CorrectPosition();
		return;
	}

	string strTitle;
	string strInfoCurDir;
	int Length;
	OpenPanelInfo Info;

	if (PanelMode==PLUGIN_PANEL)
	{
		if (ProcessPluginEvent(FE_REDRAW,nullptr))
			return;

		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		strInfoCurDir = NullToEmpty(Info.CurDir);
	}

	bool CurFullScreen=IsFullScreen();
	PrepareViewSettings(ViewMode,&Info);
	CorrectPosition();

	if (CurFullScreen!=IsFullScreen())
	{
		Global->CtrlObject->Cp()->SetScreenPosition();
		Global->CtrlObject->Cp()->GetAnotherPanel(this)->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	}

	SetScreen(X1+1,Y1+1,X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));
	Box(X1,Y1,X2,Y2,ColorIndexToColor(COL_PANELBOX),DOUBLE_BOX);

	if (Global->Opt->ShowColumnTitles)
	{
//    SetScreen(X1+1,Y1+1,X2-1,Y1+1,' ',COL_PANELTEXT);
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y1+1);
		//Global->FS << fmt::Width(X2-X1-1)<<L"";
	}

	for (size_t I=0,ColumnPos=X1+1; I < ViewSettings.PanelColumns.size(); I++)
	{
		if (ViewSettings.PanelColumns[I].width < 0)
			continue;

		if (Global->Opt->ShowColumnTitles)
		{
			LNGID IDMessage=MColumnUnknown;

			switch (ViewSettings.PanelColumns[I].type & 0xff)
			{
				case NAME_COLUMN:
					IDMessage=MColumnName;
					break;
				case EXTENSION_COLUMN:
					IDMessage=MColumnExtension;
					break;
				case SIZE_COLUMN:
					IDMessage=MColumnSize;
					break;
				case PACKED_COLUMN:
					IDMessage=MColumnAlocatedSize;
					break;
				case DATE_COLUMN:
					IDMessage=MColumnDate;
					break;
				case TIME_COLUMN:
					IDMessage=MColumnTime;
					break;
				case WDATE_COLUMN:
					IDMessage=MColumnWrited;
					break;
				case CDATE_COLUMN:
					IDMessage=MColumnCreated;
					break;
				case ADATE_COLUMN:
					IDMessage=MColumnAccessed;
					break;
				case CHDATE_COLUMN:
					IDMessage=MColumnChanged;
					break;
				case ATTR_COLUMN:
					IDMessage=MColumnAttr;
					break;
				case DIZ_COLUMN:
					IDMessage=MColumnDescription;
					break;
				case OWNER_COLUMN:
					IDMessage=MColumnOwner;
					break;
				case NUMLINK_COLUMN:
					IDMessage=MColumnMumLinks;
					break;
				case NUMSTREAMS_COLUMN:
					IDMessage=MColumnNumStreams;
					break;
				case STREAMSSIZE_COLUMN:
					IDMessage=MColumnStreamsSize;
					break;
			}

			strTitle=MSG(IDMessage);

			if (PanelMode==PLUGIN_PANEL && Info.PanelModesArray &&
			        ViewMode<static_cast<int>(Info.PanelModesNumber) &&
			        Info.PanelModesArray[ViewMode].ColumnTitles)
			{
				const wchar_t *NewTitle=Info.PanelModesArray[ViewMode].ColumnTitles[I];

				if (NewTitle)
					strTitle=NewTitle;
			}

			string strTitleMsg;
			CenterStr(strTitle,strTitleMsg,ViewSettings.PanelColumns[I].width);
			SetColor(COL_PANELCOLUMNTITLE);
			GotoXY(static_cast<int>(ColumnPos),Y1+1);
			Global->FS << fmt::MaxWidth(ViewSettings.PanelColumns[I].width) << strTitleMsg;
		}

		if (I == ViewSettings.PanelColumns.size() - 1)
			break;

		if (ViewSettings.PanelColumns[I + 1].width < 0)
			continue;

		SetColor(COL_PANELBOX);
		ColumnPos += ViewSettings.PanelColumns[I].width;
		GotoXY(static_cast<int>(ColumnPos),Y1);

		bool DoubleLine = Global->Opt->DoubleGlobalColumnSeparator && (!((I+1)%ColumnsInGlobal));

		BoxText(BoxSymbols[DoubleLine?BS_T_H2V2:BS_T_H2V1]);

		if (Global->Opt->ShowColumnTitles)
		{
			FarColor c = ColorIndexToColor(COL_PANELBOX);
			c.BackgroundColor = ColorIndexToColor(COL_PANELCOLUMNTITLE).BackgroundColor;
			SetColor(c);

			GotoXY(static_cast<int>(ColumnPos),Y1+1);
			BoxText(BoxSymbols[DoubleLine?BS_V2:BS_V1]);
		}

		if (!Global->Opt->ShowPanelStatus)
		{
			GotoXY(static_cast<int>(ColumnPos),Y2);
			BoxText(BoxSymbols[DoubleLine?BS_B_H2V2:BS_B_H2V1]);
		}

		ColumnPos++;
	}

	int NextX1=X1+1;

	if (Global->Opt->ShowSortMode)
	{
		static const value_name_pair<int, LNGID> ModeNames[] =
		{
			{UNSORTED, MMenuUnsorted},
			{BY_NAME, MMenuSortByName},
			{BY_EXT, MMenuSortByExt},
			{BY_MTIME, MMenuSortByWrite},
			{BY_CTIME, MMenuSortByCreation},
			{BY_ATIME, MMenuSortByAccess},
			{BY_CHTIME, MMenuSortByChange},
			{BY_SIZE, MMenuSortBySize},
			{BY_DIZ, MMenuSortByDiz},
			{BY_OWNER, MMenuSortByOwner},
			{BY_COMPRESSEDSIZE, MMenuSortByAllocatedSize},
			{BY_NUMLINKS, MMenuSortByNumLinks},
			{BY_NUMSTREAMS, MMenuSortByNumStreams},
			{BY_STREAMSSIZE, MMenuSortByStreamsSize},
			{BY_FULLNAME, MMenuSortByFullName},
			{BY_CUSTOMDATA, MMenuSortByCustomData},
		};

		const wchar_t *Ch = wcschr(MSG(GetNameOfValue(SortMode, ModeNames)), L'&');

		if (Ch)
		{
			if (Global->Opt->ShowColumnTitles)
				GotoXY(NextX1,Y1+1);
			else
				GotoXY(NextX1,Y1);

			SetColor(COL_PANELCOLUMNTITLE);
			OutCharacter[0]=SortOrder==1 ? Lower(Ch[1]):Upper(Ch[1]);
			Text(OutCharacter);
			NextX1++;

			if (Filter && Filter->IsEnabledOnPanel())
			{
				OutCharacter[0]=L'*';
				Text(OutCharacter);
				NextX1++;
			}
		}
	}

	/* <режимы сортировки> */
	if (/*GetNumericSort() || GetCaseSensitiveSort() || GetSortGroups() || */GetSelectedFirstMode())
	{
		if (Global->Opt->ShowColumnTitles)
			GotoXY(NextX1,Y1+1);
		else
			GotoXY(NextX1,Y1);

		SetColor(COL_PANELCOLUMNTITLE);
		wchar_t *PtrOutCharacter=OutCharacter;
		*PtrOutCharacter=0;

		//if (GetSelectedFirstMode())
			*PtrOutCharacter++=L'^';

		/*
		    if(GetNumericSort())
		      *PtrOutCharacter++=L'#';
		    if(GetSortGroups())
		      *PtrOutCharacter++=L'@';
		*/
		/*
		if(GetCaseSensitiveSort())
		{

		}
		*/
		*PtrOutCharacter=0;
		Text(OutCharacter);
		PtrOutCharacter[1]=0;
	}

	/* </режимы сортировки> */

	if (!Fast && GetFocus())
	{
		if (PanelMode==PLUGIN_PANEL)
			Global->CtrlObject->CmdLine->SetCurDir(NullToEmpty(Info.CurDir));
		else
			Global->CtrlObject->CmdLine->SetCurDir(strCurDir);

		Global->CtrlObject->CmdLine->Show();
	}

	int TitleX2=Global->Opt->Clock && !Global->Opt->ShowMenuBar ? std::min(ScrX-4,(int)X2):X2;
	int TruncSize=TitleX2-X1-3;

	if (!Global->Opt->ShowColumnTitles && Global->Opt->ShowSortMode && Filter && Filter->IsEnabledOnPanel())
		TruncSize-=2;

	if(TruncSize > 2)
		GetTitle(strTitle,TruncSize, 2);//,(PanelMode==PLUGIN_PANEL?0:2));
	Length=(int)strTitle.size();
	int ClockCorrection=FALSE;

	if ((Global->Opt->Clock && !Global->Opt->ShowMenuBar) && TitleX2==ScrX-4)
	{
		ClockCorrection=TRUE;
		TitleX2+=4;
	}

	int TitleX=X1+(TitleX2-X1+1-Length)/2;

	if (ClockCorrection)
	{
		int Overlap=TitleX+Length-TitleX2+5;

		if (Overlap > 0)
			TitleX-=Overlap;
	}

	if (TitleX <= X1)
		TitleX = X1+1;

	SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
	GotoXY(TitleX,Y1);
	Text(strTitle);

	if (ListData.empty())
	{
		SetScreen(X1+1,Y2-1,X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y2-1);
		//Global->FS << fmt::Width(X2-X1-1)<<L"";
	}

	if (PanelMode==PLUGIN_PANEL && !ListData.empty() && (Info.Flags & OPIF_REALNAMES))
	{
		if (!strInfoCurDir.empty())
		{
			strCurDir = strInfoCurDir;
		}
		else
		{
			if (!TestParentFolderName(ListData[CurFile]->strName))
			{
				strCurDir=ListData[CurFile]->strName;
				size_t pos;

				if (FindLastSlash(pos,strCurDir))
				{
					if (pos)
					{
						if (strCurDir.at(pos-1)!=L':')
							strCurDir.resize(pos);
						else
							strCurDir.resize(pos+1);
					}
				}
			}
			else
			{
				strCurDir = strOriginalCurDir;
			}
		}

		if (GetFocus())
		{
			Global->CtrlObject->CmdLine->SetCurDir(strCurDir);
			Global->CtrlObject->CmdLine->Show();
		}
	}

	if ((Global->Opt->ShowPanelTotals || Global->Opt->ShowPanelFree) &&
	        (Global->Opt->ShowPanelStatus || !SelFileCount))
	{
		ShowTotalSize(Info);
	}

	ShowList(FALSE,0);
	ShowSelectedSize();

	if (Global->Opt->ShowPanelScrollbar)
	{
		SetColor(COL_PANELSCROLLBAR);
		ScrollBarEx(X2,Y1+1+Global->Opt->ShowColumnTitles,Height,Round(CurTopFile,Columns),Round(static_cast<int>(ListData.size()), Columns));
	}

	ShowScreensCount();

	if (!ProcessingPluginCommand && LastCurFile!=CurFile)
	{
		LastCurFile=CurFile;
		UpdateViewPanel();
	}

	if (PanelMode==PLUGIN_PANEL)
		Global->CtrlObject->Cp()->RedrawKeyBar();
}


const FarColor FileList::GetShowColor(int Position, int ColorType)
{
	FarColor ColorAttr=ColorIndexToColor(COL_PANELTEXT);

	if (static_cast<size_t>(Position) < ListData.size())
	{
		int Pos = HIGHLIGHTCOLOR_NORMAL;

		if (CurFile==Position && Focus && !ListData.empty())
		{
			Pos=ListData[Position]->Selected?HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR:HIGHLIGHTCOLOR_UNDERCURSOR;
		}
		else if (ListData[Position]->Selected)
			Pos = HIGHLIGHTCOLOR_SELECTED;

		ColorAttr=ListData[Position]->Colors.Color[ColorType][Pos];

		const PaletteColors PalColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};
		if (!(ColorAttr.ForegroundColor || ColorAttr.BackgroundColor) || !Global->Opt->Highlight)
			ColorAttr=ColorIndexToColor(PalColor[Pos]);
	}

	return ColorAttr;
}

void FileList::SetShowColor(int Position, int ColorType)
{
	SetColor(GetShowColor(Position,ColorType));
}

void FileList::ShowSelectedSize()
{

	if (Global->Opt->ShowPanelStatus)
	{
		SetColor(COL_PANELBOX);
		DrawSeparator(Y2-2);
		for (size_t I=0,ColumnPos=X1+1; I<ViewSettings.PanelColumns.size() - 1; I++)
		{
			if (ViewSettings.PanelColumns[I].width < 0 || (I == ViewSettings.PanelColumns.size() - 2 && ViewSettings.PanelColumns[I+1].width < 0))
				continue;

			ColumnPos += ViewSettings.PanelColumns[I].width;
			GotoXY(static_cast<int>(ColumnPos),Y2-2);

			bool DoubleLine = Global->Opt->DoubleGlobalColumnSeparator && (!((I+1)%ColumnsInGlobal));
			BoxText(BoxSymbols[DoubleLine?BS_B_H1V2:BS_B_H1V1]);
			ColumnPos++;
		}
	}

	if (SelFileCount)
	{
		string strFormStr;
		InsertCommas(SelFileSize,strFormStr);
		LangString strSelStr(__FormatEndSelectedPhrase(SelFileCount));
		strSelStr << strFormStr << SelFileCount;
		TruncStr(strSelStr,X2-X1-1);
		int Length=(int)strSelStr.size();
		SetColor(COL_PANELSELECTEDINFO);
		GotoXY(X1+(X2-X1+1-Length)/2,Y2-2*Global->Opt->ShowPanelStatus);
		Text(strSelStr);
	}
}


void FileList::ShowTotalSize(const OpenPanelInfo &Info)
{
	if (!Global->Opt->ShowPanelTotals && PanelMode==PLUGIN_PANEL && !(Info.Flags & OPIF_REALNAMES))
		return;

	string strFormSize, strFreeSize, strTotalStr;
	int Length;
	InsertCommas(TotalFileSize,strFormSize);

	if (Global->Opt->ShowPanelFree && (PanelMode!=PLUGIN_PANEL || (Info.Flags & OPIF_REALNAMES)))
	{
		if(FreeDiskSize != static_cast<unsigned __int64>(-1))
		{
			InsertCommas(FreeDiskSize,strFreeSize);
		}
		else
		{
			strFreeSize = L"?";
		}
	}

	if (Global->Opt->ShowPanelTotals)
	{
		if (!Global->Opt->ShowPanelFree || strFreeSize.empty())
		{
			strTotalStr = LangString(__FormatEndSelectedPhrase(TotalFileCount)) << strFormSize << TotalFileCount;
		}
		else
		{
			wchar_t DHLine[4]={BoxSymbols[BS_H2],BoxSymbols[BS_H2],BoxSymbols[BS_H2],0};
 			FormatString str;
			str << L" " << strFormSize << L" (" << TotalFileCount << L") " << DHLine << L" " << strFreeSize << L" ";

			if ((int)str.size() > X2-X1-1)
			{
				if(FreeDiskSize != static_cast<unsigned __int64>(-1))
				{
					InsertCommas(FreeDiskSize>>20,strFreeSize);
				}
				else
				{
					strFreeSize = L"?";
				}
				InsertCommas(TotalFileSize>>20,strFormSize);
				str.clear();
				str << L" " << strFormSize << L" " << MSG(MListMb) << L" (" << TotalFileCount << L") " << DHLine << L" " << strFreeSize << L" " << MSG(MListMb) << L" ";
			}
			strTotalStr = str;
		}
	}
	else
	{
		strTotalStr = LangString(MListFreeSize) << (!strFreeSize.empty()? strFreeSize : L"?");
	}
	SetColor(COL_PANELTOTALINFO);
	/* $ 01.08.2001 VVM
	  + Обрезаем строчку справа, а не слева */
	TruncStrFromEnd(strTotalStr, std::max(0, X2-X1-1));
	Length=(int)strTotalStr.size();
	GotoXY(X1+(X2-X1+1-Length)/2,Y2);
	size_t BoxPos = strTotalStr.find(BoxSymbols[BS_H2]);
	int BoxLength=0;
	if (BoxPos != string::npos)
		for (int I=0; strTotalStr.at(BoxPos+I)==BoxSymbols[BS_H2]; I++)
			BoxLength++;

	if (BoxPos == string::npos || !BoxLength)
		Text(strTotalStr);
	else
	{
		Global->FS << fmt::MaxWidth(BoxPos)<<strTotalStr;
		SetColor(COL_PANELBOX);
		Global->FS << fmt::MaxWidth(BoxLength)<<strTotalStr.data()+BoxPos;
		SetColor(COL_PANELTOTALINFO);
		Text(strTotalStr.data()+BoxPos+BoxLength);
	}
}

int FileList::ConvertName(const wchar_t *SrcName,string &strDest,int MaxLength,unsigned __int64 RightAlign,int ShowStatus,DWORD FileAttr)
{
	wchar_t *lpwszDest = GetStringBuffer(strDest, MaxLength + 1);
	wmemset(lpwszDest,L' ',MaxLength);
	int SrcLength=StrLength(SrcName);

	if ((RightAlign & COLUMN_RIGHTALIGNFORCE) || (RightAlign && (SrcLength>MaxLength)))
	{
		if (SrcLength>MaxLength)
			wmemcpy(lpwszDest,SrcName+SrcLength-MaxLength,MaxLength);
		else
			wmemcpy(lpwszDest+MaxLength-SrcLength,SrcName,SrcLength);
		ReleaseStringBuffer(strDest, MaxLength);
		return (SrcLength>MaxLength);
	}

	const wchar_t *DotPtr;

	if (!ShowStatus &&
	        ((!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (ViewSettings.Flags&PVS_ALIGNEXTENSIONS))
	         || ((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (ViewSettings.Flags&PVS_FOLDERALIGNEXTENSIONS)))
	        && SrcLength<=MaxLength &&
	        (DotPtr=wcsrchr(SrcName,L'.')) && DotPtr!=SrcName &&
	        (SrcName[0]!=L'.' || SrcName[2]) && !wcschr(DotPtr+1,L' '))
	{
		int DotLength=StrLength(DotPtr+1);
		int NameLength=DotLength?(int)(DotPtr-SrcName):SrcLength;
		int DotPos=MaxLength-std::max(DotLength,3);

		if (DotPos<=NameLength)
			DotPos=NameLength+1;

		if (DotPos>0 && NameLength>0 && SrcName[NameLength-1]==L' ')
			lpwszDest[NameLength]=L'.';

		wmemcpy(lpwszDest,SrcName,NameLength);
		wmemcpy(lpwszDest+DotPos,DotPtr+1,DotLength);
	}
	else
	{
		wmemcpy(lpwszDest,SrcName,std::min(SrcLength, MaxLength));
	}

	ReleaseStringBuffer(strDest, MaxLength);
	return(SrcLength>MaxLength);
}


void FileList::PrepareViewSettings(int ViewMode, const OpenPanelInfo *PlugInfo)
{
	OpenPanelInfo Info={};

	if (PanelMode==PLUGIN_PANEL)
	{
		if (!PlugInfo)
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		else
			Info=*PlugInfo;
	}

	ViewSettings = Global->Opt->ViewSettings[ViewMode];

	if (PanelMode==PLUGIN_PANEL)
	{
		if (Info.PanelModesArray && ViewMode<static_cast<int>(Info.PanelModesNumber) &&
		        Info.PanelModesArray[ViewMode].ColumnTypes &&
		        Info.PanelModesArray[ViewMode].ColumnWidths)
		{
			TextToViewSettings(Info.PanelModesArray[ViewMode].ColumnTypes, Info.PanelModesArray[ViewMode].ColumnWidths, ViewSettings.PanelColumns);

			if (Info.PanelModesArray[ViewMode].StatusColumnTypes &&
			        Info.PanelModesArray[ViewMode].StatusColumnWidths)
			{
				TextToViewSettings(Info.PanelModesArray[ViewMode].StatusColumnTypes, Info.PanelModesArray[ViewMode].StatusColumnWidths, ViewSettings.StatusColumns);
			}
			else if (Info.PanelModesArray[ViewMode].Flags&PMFLAGS_DETAILEDSTATUS)
			{
				ViewSettings.StatusColumns.resize(4);
				ViewSettings.StatusColumns[0].type = COLUMN_RIGHTALIGN|NAME_COLUMN;
				ViewSettings.StatusColumns[1].type = SIZE_COLUMN;
				ViewSettings.StatusColumns[2].type = DATE_COLUMN;
				ViewSettings.StatusColumns[3].type = TIME_COLUMN;
				ViewSettings.StatusColumns[0].width = 0;
				ViewSettings.StatusColumns[1].width = 8;
				ViewSettings.StatusColumns[2].width = 0;
				ViewSettings.StatusColumns[3].width = 5;
			}
			else
			{
				ViewSettings.StatusColumns.resize(1);
				ViewSettings.StatusColumns[0].type = COLUMN_RIGHTALIGN|NAME_COLUMN;
				ViewSettings.StatusColumns[0].width = 0;
			}

			if (Info.PanelModesArray[ViewMode].Flags&PMFLAGS_FULLSCREEN)
				ViewSettings.Flags|=PVS_FULLSCREEN;
			else
				ViewSettings.Flags&=~PVS_FULLSCREEN;

			if (Info.PanelModesArray[ViewMode].Flags&PMFLAGS_ALIGNEXTENSIONS)
				ViewSettings.Flags|=PVS_ALIGNEXTENSIONS;
			else
				ViewSettings.Flags&=~PVS_ALIGNEXTENSIONS;

			if (!(Info.PanelModesArray[ViewMode].Flags&PMFLAGS_CASECONVERSION))
				ViewSettings.Flags&=~(PVS_FOLDERUPPERCASE|PVS_FILELOWERCASE|PVS_FILEUPPERTOLOWERCASE);
		}
		else
		{
			std::for_each(RANGE(ViewSettings.PanelColumns, i)
			{
				if ((i.type & 0xff) == NAME_COLUMN)
				{
					if (Info.Flags & OPIF_SHOWNAMESONLY)
						i.type |= COLUMN_NAMEONLY;

					if (Info.Flags & OPIF_SHOWRIGHTALIGNNAMES)
						i.type |= COLUMN_RIGHTALIGN;
				}
			});
			if (Info.Flags & OPIF_SHOWPRESERVECASE)
				ViewSettings.Flags&=~(PVS_FOLDERUPPERCASE|PVS_FILELOWERCASE|PVS_FILEUPPERTOLOWERCASE);
		}
	}

	Columns=PreparePanelView(&ViewSettings);
	Height=Y2-Y1-4;

	if (!Global->Opt->ShowColumnTitles)
		Height++;

	if (!Global->Opt->ShowPanelStatus)
		Height+=2;
}


int FileList::PreparePanelView(PanelViewSettings *PanelView)
{
	PrepareColumnWidths(PanelView->StatusColumns, (PanelView->Flags&PVS_FULLSCREEN) != 0, true);
	return PrepareColumnWidths(PanelView->PanelColumns, (PanelView->Flags&PVS_FULLSCREEN) != 0, false);
}


int FileList::PrepareColumnWidths(std::vector<column>& Columns, bool FullScreen, bool StatusLine)
{
	int TotalPercentWidth,TotalPercentCount,ZeroLengthCount,EmptyColumns;
	ZeroLengthCount=EmptyColumns=0;
	int TotalWidth = static_cast<int>(Columns.size()-1);
	TotalPercentCount=TotalPercentWidth=0;

	FOR_RANGE(Columns, i)
	{
		if (i->width < 0)
		{
			EmptyColumns++;
			continue;
		}

		int ColumnType = i->type & 0xff;

		if (!i->width)
		{
			i->width_type = COUNT_WIDTH; //manage all zero-width columns in same way
			i->width = ColumnTypeWidth[ColumnType];

			if (ColumnType==WDATE_COLUMN || ColumnType==CDATE_COLUMN || ColumnType==ADATE_COLUMN || ColumnType==CHDATE_COLUMN)
			{
				if (i->type & COLUMN_BRIEF)
					i->width -= 3;

				if (i->type & COLUMN_MONTH)
					++i->width;
			}
		}

		if (!i->width)
			ZeroLengthCount++;

		switch (i->width_type)
		{
			case COUNT_WIDTH:
				TotalWidth += i->width;
				break;
			case PERCENT_WIDTH:
				TotalPercentWidth += i->width;
				TotalPercentCount++;
				break;
		}
	}

	TotalWidth-=EmptyColumns;
	int PanelTextWidth=X2-X1-1;

	if (FullScreen)
		PanelTextWidth=ScrX-1;

	int ExtraWidth=PanelTextWidth-TotalWidth;

	if (TotalPercentCount>0)
	{
		int ExtraPercentWidth=(TotalPercentWidth>100 || !ZeroLengthCount)?ExtraWidth:ExtraWidth*TotalPercentWidth/100;
		int TempWidth=0;

		FOR_RANGE(Columns, i)
		{
			if (!TotalPercentCount)
				break;

			if (i->width_type == PERCENT_WIDTH)
			{
				int PercentWidth = (TotalPercentCount>1)?(ExtraPercentWidth*i->width/TotalPercentWidth):(ExtraPercentWidth-TempWidth);

				if (PercentWidth<1)
					PercentWidth=1;

				TempWidth+=PercentWidth;
				i->width = PercentWidth;
				i->width_type = COUNT_WIDTH;
				TotalPercentCount--;
			}
		}
		ExtraWidth-=TempWidth;
	}

	FOR_RANGE(Columns, i)
	{
		if (!ZeroLengthCount)
			break;

		if (!i->width)
		{
			int AutoWidth=ExtraWidth/ZeroLengthCount;

			if (AutoWidth<1)
				AutoWidth=1;

			i->width = AutoWidth;
			ExtraWidth-=AutoWidth;
			ZeroLengthCount--;
		}
	}

	while (1)
	{
		int LastColumn = static_cast<int>(Columns.size() - 1);
		TotalWidth=LastColumn-EmptyColumns;

		std::for_each(CONST_RANGE(Columns, i)
		{
			if (i.width > 0)
				TotalWidth += i.width;
		});

		if (TotalWidth<=PanelTextWidth)
			break;

		if (Columns.size() <= 1)
		{
			Columns.front().width = PanelTextWidth;
			break;
		}
		else if (PanelTextWidth >= TotalWidth - Columns[LastColumn].width)
		{
			Columns[LastColumn].width = PanelTextWidth - (TotalWidth - Columns[LastColumn].width);
			break;
		}
		else
			Columns.pop_back();
	}

	ColumnsInGlobal = 1;
	int GlobalColumns=0;

	FOR_RANGE(ViewSettings.PanelColumns, i)
	{
		bool UnEqual = false;
		int Remainder = ViewSettings.PanelColumns.size() % ColumnsInGlobal;
		GlobalColumns = static_cast<int>(ViewSettings.PanelColumns.size() / ColumnsInGlobal);

		if (!Remainder)
		{
			for (int k = 0; k < GlobalColumns-1; k++)
			{
				for (int j = 0; j < ColumnsInGlobal; j++)
				{
					if ((ViewSettings.PanelColumns[k*ColumnsInGlobal+j].type & 0xFF) !=
					        (ViewSettings.PanelColumns[(k+1)*ColumnsInGlobal+j].type & 0xFF))
						UnEqual = true;
				}
			}

			if (!UnEqual)
				break;
		}

		ColumnsInGlobal++;
	}

	return GlobalColumns;
}


void FileList::HighlightBorder(int Level, int ListPos)
{
	if (Level == ColumnsInGlobal)
	{
		SetColor(COL_PANELBOX);
	}
	else
	{
		FarColor FileColor = GetShowColor(ListPos, HIGHLIGHTCOLORTYPE_FILE);
		if (Global->Opt->HighlightColumnSeparator)
		{
			SetColor(FileColor);
		}
		else
		{
			FarColor Color = ColorIndexToColor(COL_PANELBOX);
			Color.BackgroundColor = FileColor.BackgroundColor;
			FileColor.Flags&FCF_BG_4BIT? Color.Flags|=FCF_BG_4BIT : Color.Flags&=~FCF_BG_4BIT;
			SetColor(Color);
		}
	}
}

void FileList::ShowList(int ShowStatus,int StartColumn)
{
	string strDateStr, strTimeStr;
	int StatusShown=FALSE;
	int MaxLeftPos=0,MinLeftPos=FALSE;
	size_t ColumnCount=ShowStatus ? ViewSettings.StatusColumns.size() : ViewSettings.PanelColumns.size();
	auto& Columns = ShowStatus ? ViewSettings.StatusColumns : ViewSettings.PanelColumns;

	for (int I=Y1+1+Global->Opt->ShowColumnTitles,J=CurTopFile; I<Y2-2*Global->Opt->ShowPanelStatus; I++,J++)
	{
		int CurColumn=StartColumn;

		if (ShowStatus)
		{
			SetColor(COL_PANELTEXT);
			GotoXY(X1+1,Y2-1);
		}
		else
		{
			SetShowColor(J);
			GotoXY(X1+1,I);
		}

		int StatusLine=FALSE;
		int Level = 1;

		for (size_t K=0; K<ColumnCount; K++)
		{
			int ListPos=J+CurColumn*Height;

			if (ShowStatus)
			{
				if (CurFile!=ListPos)
				{
					CurColumn++;
					continue;
				}
				else
					StatusLine=TRUE;
			}

			int CurX=WhereX();
			int CurY=WhereY();
			int ShowDivider=TRUE;
			int ColumnType=static_cast<int>(Columns[K].type & 0xff);
			int ColumnWidth=Columns[K].width;

			if (ColumnWidth<0)
			{
				if (!ShowStatus && K==ColumnCount-1)
				{
					SetColor(COL_PANELBOX);
					GotoXY(CurX-1,CurY);
					BoxText(CurX-1==X2 ? BoxSymbols[BS_V2]:L' ');
				}

				continue;
			}

			if (ListPos < static_cast<int>(ListData.size()))
			{
				if (!ShowStatus && !StatusShown && CurFile==ListPos && Global->Opt->ShowPanelStatus)
				{
					ShowList(TRUE,CurColumn);
					GotoXY(CurX,CurY);
					StatusShown=TRUE;
					SetShowColor(ListPos);
				}

				if (!ShowStatus)
					SetShowColor(ListPos);

				if (ColumnType>=CUSTOM_COLUMN0 && ColumnType<=CUSTOM_COLUMN9)
				{
					size_t ColumnNumber=ColumnType-CUSTOM_COLUMN0;
					const wchar_t *ColumnData=nullptr;

					if (ColumnNumber<ListData[ListPos]->CustomColumnNumber)
						ColumnData=ListData[ListPos]->CustomColumnData[ColumnNumber];

					if (!ColumnData)
					{
						ColumnData=ListData[ListPos]->strCustomData.data();//L"";
					}

					int CurLeftPos=0;

					if (!ShowStatus && LeftPos>0)
					{
						int Length=StrLength(ColumnData);
						if (Length>ColumnWidth)
						{
							CurLeftPos = std::min(LeftPos, Length-ColumnWidth);
							MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
						}
					}

					Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(ColumnWidth)<<ColumnData+CurLeftPos;
				}
				else
				{
					switch (ColumnType)
					{
						case NAME_COLUMN:
						{
							int Width=ColumnWidth;
							unsigned __int64 ViewFlags=Columns[K].type;

							if ((ViewFlags & COLUMN_MARK) && Width>2)
							{
								Text(ListData[ListPos]->Selected?L"\x221A ":L"  ");
								Width-=2;
							}

							if (ListData[ListPos]->Colors.MarkChar && Global->Opt->Highlight && Width>1)
							{
								Width--;
								OutCharacter[0]=(wchar_t)ListData[ListPos]->Colors.MarkChar;
								FarColor OldColor=GetColor();

								if (!ShowStatus)
									SetShowColor(ListPos,HIGHLIGHTCOLORTYPE_MARKCHAR);

								Text(OutCharacter);
								SetColor(OldColor);
							}

							const wchar_t *NamePtr = ShowShortNames && !ListData[ListPos]->strShortName.empty() && !ShowStatus ? ListData[ListPos]->strShortName.data():ListData[ListPos]->strName.data();

							string strNameCopy;
							if (!(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY) && (ViewFlags & COLUMN_NOEXTENSION))
							{
								const wchar_t *ExtPtr = PointToExt(NamePtr);
								if (ExtPtr)
								{
									strNameCopy.assign(NamePtr, ExtPtr-NamePtr);
									NamePtr = strNameCopy.data();
								}
							}

							const wchar_t *NameCopy = NamePtr;

							if (ViewFlags & COLUMN_NAMEONLY)
							{
								//BUGBUG!!!
								// !!! НЕ УВЕРЕН, но то, что отображается пустое
								// пространство вместо названия - бага
								NamePtr=PointToFolderNameIfFolder(NamePtr);
							}

							int CurLeftPos=0;
							unsigned __int64 RightAlign=(ViewFlags & (COLUMN_RIGHTALIGN|COLUMN_RIGHTALIGNFORCE));
							int LeftBracket=FALSE,RightBracket=FALSE;

							if (!ShowStatus && LeftPos)
							{
								int Length = StrLength(NamePtr);

								if (Length>Width)
								{
									if (LeftPos>0)
									{
										if (!RightAlign)
										{
											CurLeftPos = std::min(LeftPos, Length-Width);
											MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
											NamePtr += CurLeftPos;
										}
									}
									else if (RightAlign)
									{
										int CurRightPos=LeftPos;

										if (Length+CurRightPos<Width)
										{
											CurRightPos=Width-Length;
										}
										else
										{
											RightBracket=TRUE;
											LeftBracket=(ViewFlags & COLUMN_RIGHTALIGNFORCE)==COLUMN_RIGHTALIGNFORCE;
										}

										NamePtr += Length+CurRightPos-Width;
										RightAlign=FALSE;

										MinLeftPos = std::min(MinLeftPos, CurRightPos);
									}
								}
							}

							string strName;
							int TooLong=ConvertName(NamePtr, strName, Width, RightAlign,ShowStatus,ListData[ListPos]->FileAttr);

							if (CurLeftPos)
								LeftBracket=TRUE;

							if (TooLong)
							{
								if (RightAlign)
									LeftBracket=TRUE;

								if (!RightAlign && StrLength(NamePtr)>Width)
									RightBracket=TRUE;
							}

							if (!ShowStatus)
							{
								if (ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE)
									if (!(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY) && !IsCaseMixed(NameCopy))
										Lower(strName);

								if ((ViewSettings.Flags&PVS_FOLDERUPPERCASE) && (ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
									Upper(strName);

								if ((ViewSettings.Flags&PVS_FILELOWERCASE) && !(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
									Lower(strName);
							}

							Text(strName);


							if (!ShowStatus)
							{
								int NameX=WhereX();

								if (LeftBracket)
								{
									GotoXY(CurX-1,CurY);

									if (Level == 1)
										SetColor(COL_PANELBOX);

									Text(openBracket);
									SetShowColor(J);
								}

								if (RightBracket)
								{
									HighlightBorder(Level, ListPos);
									GotoXY(NameX,CurY);
									Text(closeBracket);
									ShowDivider=FALSE;

									if (Level == ColumnsInGlobal)
										SetColor(COL_PANELTEXT);
									else
										SetShowColor(J);
								}
							}
						}
						break;
						case EXTENSION_COLUMN:
						{
							const wchar_t *ExtPtr = nullptr;
							if (!(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
							{
								const wchar_t *NamePtr = ShowShortNames && !ListData[ListPos]->strShortName.empty() && !ShowStatus ? ListData[ListPos]->strShortName.data():ListData[ListPos]->strName.data();
								ExtPtr = PointToExt(NamePtr);
							}
							if (ExtPtr && *ExtPtr) ExtPtr++; else ExtPtr = L"";

							unsigned __int64 ViewFlags=Columns[K].type;
							if (ViewFlags&COLUMN_RIGHTALIGN)
								Global->FS << fmt::RightAlign()<<fmt::ExactWidth(ColumnWidth)<<ExtPtr;
							else
								Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(ColumnWidth)<<ExtPtr;

							if (!ShowStatus && StrLength(ExtPtr) > ColumnWidth)
							{
								int NameX=WhereX();

								HighlightBorder(Level, ListPos);

								GotoXY(NameX,CurY);
								Text(closeBracket);
								ShowDivider=FALSE;

								if (Level == ColumnsInGlobal)
									SetColor(COL_PANELTEXT);
								else
									SetShowColor(J);
							}
							break;
						}
						break;
						case SIZE_COLUMN:
						case PACKED_COLUMN:
						case STREAMSSIZE_COLUMN:
						{
							Text(FormatStr_Size(
								ListData[ListPos]->FileSize,
								ListData[ListPos]->AllocationSize,
								ListData[ListPos]->StreamsSize,
								ListData[ListPos]->strName,
								ListData[ListPos]->FileAttr,
								ListData[ListPos]->ShowFolderSize,
								ListData[ListPos]->ReparseTag,
								ColumnType,
								Columns[K].type,
								ColumnWidth,
								strCurDir.data()));
							break;
						}

						case DATE_COLUMN:
						case TIME_COLUMN:
						case WDATE_COLUMN:
						case CDATE_COLUMN:
						case ADATE_COLUMN:
						case CHDATE_COLUMN:
						{
							FILETIME *FileTime;

							switch (ColumnType)
							{
								case CDATE_COLUMN:
									FileTime=&ListData[ListPos]->CreationTime;
									break;
								case ADATE_COLUMN:
									FileTime=&ListData[ListPos]->AccessTime;
									break;
								case CHDATE_COLUMN:
									FileTime=&ListData[ListPos]->ChangeTime;
									break;
								case DATE_COLUMN:
								case TIME_COLUMN:
								case WDATE_COLUMN:
								default:
									FileTime=&ListData[ListPos]->WriteTime;
									break;
							}

							Global->FS << FormatStr_DateTime(FileTime,ColumnType,Columns[K].type,ColumnWidth);
							break;
						}

						case ATTR_COLUMN:
						{
							Global->FS << FormatStr_Attribute(ListData[ListPos]->FileAttr,ColumnWidth);
							break;
						}

						case DIZ_COLUMN:
						{
							int CurLeftPos=0;

							if (!ShowStatus && LeftPos>0)
							{
								int Length=ListData[ListPos]->DizText ? StrLength(ListData[ListPos]->DizText):0;
								if (Length>ColumnWidth)
								{
									CurLeftPos = std::min(LeftPos, Length-ColumnWidth);
									MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
								}
							}

							string strDizText=ListData[ListPos]->DizText ? ListData[ListPos]->DizText+CurLeftPos:L"";
							size_t pos = strDizText.find(L'\4');
							if (pos != string::npos)
								strDizText.resize(pos);

							Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(ColumnWidth)<<strDizText;
							break;
						}

						case OWNER_COLUMN:
						{
							const wchar_t* Owner=ListData[ListPos]->strOwner.data();

							if (Owner && !(Columns[K].type & COLUMN_FULLOWNER) && PanelMode!=PLUGIN_PANEL)
							{
								const wchar_t* SlashPos=FirstSlash(Owner);

								if (SlashPos)
									Owner=SlashPos+1;
							}
							else if(IsSlash(*Owner))
							{
								Owner++;
							}

							int CurLeftPos=0;

							if (!ShowStatus && LeftPos>0)
							{
								int Length=StrLength(Owner);
								if (Length>ColumnWidth)
								{
									CurLeftPos = std::min(LeftPos, Length-ColumnWidth);
									MaxLeftPos = std::max(MaxLeftPos, CurLeftPos);
								}
							}

							Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(ColumnWidth)<<Owner+CurLeftPos;
							break;
						}

						case NUMLINK_COLUMN:
						{
							int nlink = ListData[ListPos]->NumberOfLinks;
							if (nlink >= 0)
								Global->FS << fmt::ExactWidth(ColumnWidth) << nlink;
							else
								Global->FS << fmt::ExactWidth(ColumnWidth) << L"?";
							break;
						}

						case NUMSTREAMS_COLUMN:
						{
							Global->FS << fmt::ExactWidth(ColumnWidth)<<ListData[ListPos]->NumberOfStreams;
							break;
						}

					}
				}
			}
			else
			{
				Global->FS << fmt::MinWidth(ColumnWidth)<<L"";
			}

			if (ShowDivider==FALSE)
				GotoXY(CurX+ColumnWidth+1,CurY);
			else
			{
				if (!ShowStatus)
				{
					HighlightBorder(Level, ListPos);
				}

				if (K == ColumnCount-1)
					SetColor(COL_PANELBOX);

				GotoXY(CurX+ColumnWidth,CurY);

				if (K==ColumnCount-1)
					BoxText(CurX+ColumnWidth==X2 ? BoxSymbols[BS_V2]:L' ');
				else
					BoxText(ShowStatus ? L' ':BoxSymbols[(Global->Opt->DoubleGlobalColumnSeparator && Level == ColumnsInGlobal)?BS_V2:BS_V1]);

				if (!ShowStatus)
					SetColor(COL_PANELTEXT);
			}

			if (!ShowStatus)
			{
				if (Level == ColumnsInGlobal)
				{
					Level = 0;
					CurColumn++;
				}

				Level++;
			}
		}

		if ((!ShowStatus || StatusLine) && WhereX()<X2)
		{
			SetColor(COL_PANELTEXT);
			Global->FS << fmt::MinWidth(X2-WhereX())<<L"";
		}
	}

	if (!ShowStatus && !StatusShown && Global->Opt->ShowPanelStatus)
	{
		SetScreen(X1+1,Y2-1,X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y2-1);
		//Global->FS << fmt::Width(X2-X1-1)<<L"";
	}

	if (!ShowStatus)
	{
		if (LeftPos<0)
			LeftPos=MinLeftPos;

		if (LeftPos>0)
			LeftPos=MaxLeftPos;
	}
}

bool FileList::IsModeFullScreen(int Mode)
{
	return (Global->Opt->ViewSettings[Mode].Flags&PVS_FULLSCREEN)==PVS_FULLSCREEN;
}


bool FileList::IsDizDisplayed()
{
	return IsColumnDisplayed(DIZ_COLUMN);
}


bool FileList::IsColumnDisplayed(int Type)
{
	auto is_same_type = [&Type](const column& i) {return static_cast<int>(i.type & 0xff) == Type;};

	return std::any_of(ALL_CONST_RANGE(ViewSettings.PanelColumns), is_same_type) ||
		std::any_of(ALL_CONST_RANGE(ViewSettings.StatusColumns), is_same_type);
}
