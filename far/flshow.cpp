/*
flshow.cpp

Файловая панель - вывод на экран

*/

#include "headers.hpp"
#pragma hdrstop

#include "filelist.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "plugin.hpp"
#include "colors.hpp"
#include "lang.hpp"
#include "filefilter.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "flink.hpp"

extern struct PanelViewSettings ViewSettingsArray[];
extern int ColumnTypeWidth[];

//static char VerticalLine[2][2]={{0x0B3,0x00},{0x0BA,0x00}};
static BYTE VerticalLine[2]={0x0B3,0x0BA};
static char OutCharacter[8]={0,0,0,0,0,0,0,0};

static int __FormatEndSelectedPhrase(int Count)
{
	int M_Fmt=MListFileSize;

	if (Count != 1)
	{
		char StrItems[32];
		itoa(Count,StrItems,10);
		int LenItems=(int)strlen(StrItems);

		if (StrItems[LenItems-1] == '1' && Count != 11)
			M_Fmt=MListFilesSize1;
		else
			M_Fmt=MListFilesSize2;
	}

	return M_Fmt;
}


void FileList::DisplayObject()
{
	Height=Y2-Y1-4+!Opt.ShowColumnTitles+(Opt.ShowPanelStatus ? 0:2);
	_OT(SysLog("[%p] FileList::DisplayObject()",this));

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

	char Title[NM];
	int Length;
	struct OpenPluginInfo Info;

	if (PanelMode==PLUGIN_PANEL)
	{
		if (ProcessPluginEvent(FE_REDRAW,NULL))
			return;

		CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
	}

	int CurFullScreen=IsFullScreen();
	PrepareViewSettings(ViewMode,&Info);
	CorrectPosition();

	if (CurFullScreen!=IsFullScreen())
	{
		CtrlObject->Cp()->SetScreenPosition();
		CtrlObject->Cp()->GetAnotherPanel(this)->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	}

	SetScreen(X1+1,Y1+1,X2-1,Y2-1,' ',COL_PANELTEXT);
	Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);

	if (Opt.ShowColumnTitles)
	{
//    SetScreen(X1+1,Y1+1,X2-1,Y1+1,' ',COL_PANELTEXT);
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y1+1);
		//mprintf("%*s",X2-X1-1,"");
	}

	for (int I=0,ColumnPos=X1+1; I<ViewSettings.ColumnCount; I++)
	{
		if (ViewSettings.ColumnWidth[I]<0)
			continue;

		if (Opt.ShowColumnTitles)
		{
			char *Title="";
			int IDMessage=-1;

			switch (ViewSettings.ColumnType[I] & 0xff)
			{
				case NAME_COLUMN:
					IDMessage=MColumnName;
					break;
				case SIZE_COLUMN:
					IDMessage=MColumnSize;
					break;
				case PACKED_COLUMN:
					IDMessage=MColumnPacked;
					break;
				case DATE_COLUMN:
					IDMessage=MColumnDate;
					break;
				case TIME_COLUMN:
					IDMessage=MColumnTime;
					break;
				case MDATE_COLUMN:
					IDMessage=MColumnModified;
					break;
				case CDATE_COLUMN:
					IDMessage=MColumnCreated;
					break;
				case ADATE_COLUMN:
					IDMessage=MColumnAccessed;
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
			}

			if (IDMessage != -1)
				Title=MSG(IDMessage);

			if (PanelMode==PLUGIN_PANEL && Info.PanelModesArray!=NULL &&
			        ViewMode<Info.PanelModesNumber &&
			        Info.PanelModesArray[ViewMode].ColumnTitles!=NULL)
			{
				char *NewTitle=Info.PanelModesArray[ViewMode].ColumnTitles[I];

				if (NewTitle!=NULL)
					Title=NewTitle;
			}

			char TitleMsg[256];
			CenterStr(Title,TitleMsg,ViewSettings.ColumnWidth[I]);
			SetColor(COL_PANELCOLUMNTITLE);
			GotoXY(ColumnPos,Y1+1);
			mprintf("%.*s",ViewSettings.ColumnWidth[I],TitleMsg);
		}

		if (I>=ViewSettings.ColumnCount-1)
			break;

		if (ViewSettings.ColumnWidth[I+1]<0)
			continue;

		SetColor(COL_PANELBOX);
		ColumnPos+=ViewSettings.ColumnWidth[I];
		GotoXY(ColumnPos,Y1);
		BoxText(Opt.UseUnicodeConsole?BoxSymbols[0xD1-0x0B0]:0xD1);

		if (Opt.ShowColumnTitles)
		{
			GotoXY(ColumnPos,Y1+1);
			BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[VerticalLine[0]-0x0B0]:VerticalLine[0]));
		}

		if (!Opt.ShowPanelStatus)
		{
			GotoXY(ColumnPos,Y2);
			BoxText(Opt.UseUnicodeConsole?BoxSymbols[0xCF-0x0B0]:0xCF);
		}

		ColumnPos++;
	}

	int NextX1=X1+1;

	if (Opt.ShowSortMode)
	{
		static int SortModes[]={UNSORTED,BY_NAME,BY_EXT,BY_MTIME,BY_CTIME,
		                        BY_ATIME,BY_SIZE,BY_DIZ,BY_OWNER,
		                        BY_COMPRESSEDSIZE,BY_NUMLINKS
		                       };
		static int SortStrings[]={MMenuUnsorted,MMenuSortByName,
		                          MMenuSortByExt,MMenuSortByModification,MMenuSortByCreation,
		                          MMenuSortByAccess,MMenuSortBySize,MMenuSortByDiz,MMenuSortByOwner,
		                          MMenuSortByCompressedSize,MMenuSortByNumLinks
		                         };

		for (int I=0; I<sizeof(SortModes)/sizeof(SortModes[0]); I++)
		{
			if (SortModes[I]==SortMode)
			{
				char *SortStr=MSG(SortStrings[I]);
				char *Ch=strchr(SortStr,'&');

				if (Ch!=NULL)
				{
					if (Opt.ShowColumnTitles)
						GotoXY(NextX1,Y1+1);
					else
						GotoXY(NextX1,Y1);

					SetColor(COL_PANELCOLUMNTITLE);
					OutCharacter[0]=SortOrder==1 ? LocalLower(Ch[1]):LocalUpper(Ch[1]);
					Text(OutCharacter);
					NextX1++;

					if (Filter!=NULL && Filter->IsEnabledOnPanel())
					{
						OutCharacter[0]='*';
						Text(OutCharacter);
						NextX1++;
					}
				}

				break;
			}
		}
	}

	/* <режимы сортировки> */
	if (GetNumericSort() || GetSortGroups() || GetSelectedFirstMode())
	{
		if (Opt.ShowColumnTitles)
			GotoXY(NextX1,Y1+1);
		else
			GotoXY(NextX1,Y1);

		SetColor(COL_PANELCOLUMNTITLE);
		char *PtrOutCharacter=OutCharacter;
		*PtrOutCharacter=0;

		if (GetSelectedFirstMode())
			*PtrOutCharacter++='^';

		/*
		    if(GetNumericSort())
		      *PtrOutCharacter++='#';
		    if(GetSortGroups())
		      *PtrOutCharacter++='@';
		*/
		*PtrOutCharacter=0;
		Text(OutCharacter);
		PtrOutCharacter[1]=0;
	}

	/* </режимы сортировки> */

	if (!Fast && GetFocus())
	{
		CtrlObject->CmdLine->SetCurDir(PanelMode==PLUGIN_PANEL ? Info.CurDir:CurDir);
		CtrlObject->CmdLine->Show();
	}

	int TitleX2=Opt.Clock && !Opt.ShowMenuBar ? Min(ScrX-4,X2):X2;
	int TruncSize=TitleX2-X1-3;

	if (!Opt.ShowColumnTitles && Opt.ShowSortMode && Filter!=NULL && Filter->IsEnabledOnPanel())
		TruncSize-=2;

	GetTitle(Title,TruncSize,2);//(PanelMode==PLUGIN_PANEL?0:2));
	Length=(int)strlen(Title);
	int ClockCorrection=FALSE;

	if ((Opt.Clock && !Opt.ShowMenuBar) && TitleX2==ScrX-4)
	{
		ClockCorrection=TRUE;
		TitleX2+=4;
	}

	int TitleX=X1+(TitleX2-X1+1-Length)/2;

	if (ClockCorrection)
	{
		int Overlap=TitleX+Length-TitleX2+5;

		if (Overlap>0)
			TitleX-=Overlap;
	}

	SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
	GotoXY(TitleX,Y1);
	Text(Title);

	if (FileCount==0)
	{
		SetScreen(X1+1,Y2-1,X2-1,Y2-1,' ',COL_PANELTEXT);
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y2-1);
		//mprintf("%*s",X2-X1-1,"");
	}

	if (PanelMode==PLUGIN_PANEL && FileCount>0 && (Info.Flags & OPIF_REALNAMES))
	{
		if (*Info.CurDir)
		{
			strcpy(CurDir,Info.CurDir);
		}
		else
		{
			struct FileListItem *CurPtr=ListData+CurFile;

			if (!TestParentFolderName(CurPtr->Name))
			{
				strcpy(CurDir,CurPtr->Name);
				char *NamePtr=strrchr(CurDir,'\\');

				if (NamePtr!=NULL && NamePtr!=CurDir)
				{
					if (*(NamePtr-1)!=':')
						*NamePtr=0;
					else
						*(NamePtr+1)=0;
				}
			}
		}

		if (GetFocus())
		{
			CtrlObject->CmdLine->SetCurDir(CurDir);
			CtrlObject->CmdLine->Show();
		}
	}

	if ((Opt.ShowPanelTotals || Opt.ShowPanelFree) &&
	        (Opt.ShowPanelStatus || SelFileCount==0))
		ShowTotalSize(Info);

	ShowList(FALSE,0);
	ShowSelectedSize();

	if (Opt.ShowPanelScrollbar)
	{
		SetColor(COL_PANELSCROLLBAR);
		ScrollBar(X2,Y1+1+Opt.ShowColumnTitles,Height,CurFile,FileCount>1 ? FileCount-1:FileCount);
	}

	ShowScreensCount();

	if (!ProcessingPluginCommand && LastCurFile!=CurFile)
	{
		LastCurFile=CurFile;
		UpdateViewPanel();
	}

	if (PanelMode==PLUGIN_PANEL)
		CtrlObject->Cp()->RedrawKeyBar();
}


int FileList::GetShowColor(int Position, int ColorType)
{
	DWORD ColorAttr=COL_PANELTEXT;
	const DWORD FarColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};

	if (ListData && Position < FileCount)
	{
		struct FileListItem *CurPtr=ListData+Position;
		int Pos = HIGHLIGHTCOLOR_NORMAL;

		if (CurFile==Position && Focus && FileCount > 0)
		{
			if (CurPtr->Selected)
				Pos = HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR;
			else
				Pos = HIGHLIGHTCOLOR_UNDERCURSOR;
		}
		else if (CurPtr->Selected)
			Pos = HIGHLIGHTCOLOR_SELECTED;

		ColorAttr=CurPtr->Colors.Color[ColorType][Pos];

		if (!ColorAttr || !Opt.Highlight)
			ColorAttr=FarColor[Pos];
	}

	return ColorAttr;
}


void FileList::SetShowColor(int Position, int ColorType)
{
	SetColor(GetShowColor(Position,ColorType));
}

void FileList::ShowSelectedSize()
{
	int Length;
	char SelStr[256],FormStr[20];

	if (Opt.ShowPanelStatus)
	{
		SetColor(COL_PANELBOX);
		DrawSeparator(Y2-2);

		for (int I=0,ColumnPos=X1+1; I<ViewSettings.ColumnCount-1; I++)
		{
			if (ViewSettings.ColumnWidth[I]<0 ||
			        I==ViewSettings.ColumnCount-2 && ViewSettings.ColumnWidth[I+1]<0)
				continue;

			ColumnPos+=ViewSettings.ColumnWidth[I];
			GotoXY(ColumnPos,Y2-2);
			BoxText(Opt.UseUnicodeConsole?BoxSymbols[0x0C1-0x0B0]:0x0C1);
			ColumnPos++;
		}
	}

	if (SelFileCount)
	{
		InsertCommas((unsigned __int64)SelFileSize,FormStr,sizeof(FormStr));
		sprintf(SelStr,MSG(__FormatEndSelectedPhrase(SelFileCount)),FormStr,SelFileCount);
		TruncStr(SelStr,X2-X1-1);
		Length=(int)strlen(SelStr);
		SetColor(COL_PANELSELECTEDINFO);
		GotoXY(X1+(X2-X1+1-Length)/2,Y2-2*Opt.ShowPanelStatus);
		Text(SelStr);
	}
	else if (!RegVer)
	{
		char EvalStr[256];
		char *EvalMsg=MSG(MListEval);

		if (*EvalMsg==0)
			strcpy(EvalStr," Evaluation version ");
		else
			sprintf(EvalStr," %s ",MSG(MListEval));

		TruncStr(EvalStr,X2-X1-1);
		Length=(int)strlen(EvalStr);
		SetColor(COL_PANELTEXT);
		GotoXY(X1+(X2-X1+1-Length)/2,Y2-2*Opt.ShowPanelStatus);
		Text(EvalStr);
	}
}


void FileList::ShowTotalSize(struct OpenPluginInfo &Info)
{
	if (!Opt.ShowPanelTotals && PanelMode==PLUGIN_PANEL && (Info.Flags & OPIF_REALNAMES)==0)
		return;

	char TotalStr[256],FormSize[20],FreeSize[20];
	int Length;
	InsertCommas((unsigned __int64)TotalFileSize,FormSize,sizeof(FormSize));
	*FreeSize=0;

	if (Opt.ShowPanelFree && (PanelMode!=PLUGIN_PANEL || (Info.Flags & OPIF_REALNAMES)))
		InsertCommas((unsigned __int64)FreeDiskSize,FreeSize,sizeof(FreeSize));

	if (Opt.ShowPanelTotals)
	{
		if (!Opt.ShowPanelFree || *FreeSize==0)
			sprintf(TotalStr,MSG(__FormatEndSelectedPhrase(TotalFileCount)),FormSize,TotalFileCount);
		else
		{
// UNICODE!!!
			static unsigned char DHLine[4]={0x0CD,0x0CD,0x0CD,0x00};
			sprintf(TotalStr," %s (%d) %s %s ",FormSize,TotalFileCount,DHLine,FreeSize);

			if ((int) strlen(TotalStr)> X2-X1-1)
			{
				InsertCommas(((unsigned __int64)FreeDiskSize)>>20,FreeSize,sizeof(FreeSize));
				InsertCommas(((unsigned __int64)TotalFileSize)>>20,FormSize,sizeof(FormSize));
				sprintf(TotalStr," %s %s (%d) %s %s %s ",FormSize,MSG(MListMb),TotalFileCount,DHLine,FreeSize,MSG(MListMb));
			}
		}
	}
	else
		sprintf(TotalStr,MSG(MListFreeSize),*FreeSize ? FreeSize:"???");

	SetColor(COL_PANELTOTALINFO);
	/* $ 01.08.2001 VVM
	  + Обрезаем строчку справа, а не слева */
	TruncStrFromEnd(TotalStr, X2-X1-1);
	/* VVM $ */
	Length=(int)strlen(TotalStr);
	GotoXY(X1+(X2-X1+1-Length)/2,Y2);
// UNICODE!!!
	char *FirstBox=strchr(TotalStr,0x0CD);
	int BoxPos=(FirstBox==NULL) ? -1:(int)(FirstBox-TotalStr);
	int BoxLength=0;

	if (BoxPos!=-1)

// UNICODE!!!
		for (int I=0; TotalStr[BoxPos+I]==0x0CD; I++)
			BoxLength++;

	if (BoxPos==-1 || BoxLength==0)
		Text(TotalStr);
	else
	{
		mprintf("%.*s",BoxPos,TotalStr);
		SetColor(COL_PANELBOX);
		mprintf("%.*s",BoxLength,TotalStr+BoxPos);
		SetColor(COL_PANELTOTALINFO);
		Text(TotalStr+BoxPos+BoxLength);
	}
}


int FileList::ConvertName(char *SrcName,char *DestName,int MaxLength,int RightAlign,int ShowStatus,DWORD FileAttr)
{
	memset(DestName,' ',MaxLength);
	int SrcLength=(int)strlen(SrcName);

	if (RightAlign && SrcLength>MaxLength)
	{
		memcpy(DestName,SrcName+SrcLength-MaxLength,MaxLength);
		DestName[MaxLength]=0;
		return(TRUE);
	}

	char *DotPtr;

	if (!ShowStatus &&
	        (!(FileAttr&FA_DIREC) && ViewSettings.AlignExtensions || (FileAttr&FA_DIREC) && ViewSettings.FolderAlignExtensions)
	        && SrcLength<=MaxLength &&
	        (DotPtr=strrchr(SrcName,'.'))!=NULL && DotPtr!=SrcName &&
	        (SrcName[0]!='.' || SrcName[2]!=0) && strchr(DotPtr+1,' ')==NULL)
	{
		int DotLength=(int)strlen(DotPtr+1);
		int NameLength=(int)(DotPtr-SrcName);
		int DotPos=MaxLength-Max(DotLength,3);

		if (DotPos<=NameLength)
			DotPos=NameLength+1;

		if (DotPos>0 && NameLength>0 && SrcName[NameLength-1]==' ')
			DestName[NameLength]='.';

		memcpy(DestName,SrcName,NameLength);
		memcpy(DestName+DotPos,DotPtr+1,DotLength);
	}
	else
		memcpy(DestName,SrcName,SrcLength);

	DestName[MaxLength]=0;
	return(SrcLength>MaxLength);
}


void FileList::PrepareViewSettings(int ViewMode,struct OpenPluginInfo *PlugInfo)
{
	struct OpenPluginInfo Info;

	if (PanelMode==PLUGIN_PANEL)
		if (PlugInfo==NULL)
			CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
		else
			Info=*PlugInfo;

	ViewSettings=ViewSettingsArray[ViewMode];

	if (PanelMode==PLUGIN_PANEL)
	{
		if (Info.PanelModesArray!=NULL && ViewMode<Info.PanelModesNumber &&
		        Info.PanelModesArray[ViewMode].ColumnTypes!=NULL &&
		        Info.PanelModesArray[ViewMode].ColumnWidths!=NULL)
		{
			TextToViewSettings(Info.PanelModesArray[ViewMode].ColumnTypes,
			                   Info.PanelModesArray[ViewMode].ColumnWidths,
			                   ViewSettings.ColumnType,ViewSettings.ColumnWidth,
			                   ViewSettings.ColumnCount);

			if (Info.PanelModesArray[ViewMode].StatusColumnTypes!=NULL &&
			        Info.PanelModesArray[ViewMode].StatusColumnWidths!=NULL)
				TextToViewSettings(Info.PanelModesArray[ViewMode].StatusColumnTypes,
				                   Info.PanelModesArray[ViewMode].StatusColumnWidths,
				                   ViewSettings.StatusColumnType,ViewSettings.StatusColumnWidth,
				                   ViewSettings.StatusColumnCount);
			else if (Info.PanelModesArray[ViewMode].DetailedStatus)
			{
				ViewSettings.StatusColumnType[0]=COLUMN_RIGHTALIGN|NAME_COLUMN;
				ViewSettings.StatusColumnType[1]=SIZE_COLUMN;
				ViewSettings.StatusColumnType[2]=DATE_COLUMN;
				ViewSettings.StatusColumnType[3]=TIME_COLUMN;
				ViewSettings.StatusColumnWidth[0]=0;
				ViewSettings.StatusColumnWidth[1]=8;
				ViewSettings.StatusColumnWidth[2]=0;
				ViewSettings.StatusColumnWidth[3]=5;
				ViewSettings.StatusColumnCount=4;
			}
			else
			{
				ViewSettings.StatusColumnType[0]=COLUMN_RIGHTALIGN|NAME_COLUMN;
				ViewSettings.StatusColumnWidth[0]=0;
				ViewSettings.StatusColumnCount=1;
			}

			ViewSettings.FullScreen=Info.PanelModesArray[ViewMode].FullScreen;
			ViewSettings.AlignExtensions=Info.PanelModesArray[ViewMode].AlignExtensions;

			if (!Info.PanelModesArray[ViewMode].CaseConversion)
			{
				ViewSettings.FolderUpperCase=0;
				ViewSettings.FileLowerCase=0;
				ViewSettings.FileUpperToLowerCase=0;
			}
		}
		else
			for (int I=0; I<ViewSettings.ColumnCount; I++)
				if ((ViewSettings.ColumnType[I] & 0xff)==NAME_COLUMN)
				{
					if (Info.Flags & OPIF_SHOWNAMESONLY)
						ViewSettings.ColumnType[I]|=COLUMN_NAMEONLY;

					if (Info.Flags & OPIF_SHOWRIGHTALIGNNAMES)
						ViewSettings.ColumnType[I]|=COLUMN_RIGHTALIGN;

					if (Info.Flags & OPIF_SHOWPRESERVECASE)
					{
						ViewSettings.FolderUpperCase=0;
						ViewSettings.FileLowerCase=0;
						ViewSettings.FileUpperToLowerCase=0;
					}
				}
	}

	Columns=PreparePanelView(&ViewSettings);
	Height=Y2-Y1-4;

	if (!Opt.ShowColumnTitles)
		Height++;

	if (!Opt.ShowPanelStatus)
		Height+=2;
}


int FileList::PreparePanelView(struct PanelViewSettings *PanelView)
{
	PrepareColumnWidths(PanelView->StatusColumnType,PanelView->StatusColumnWidth,
	                    PanelView->StatusColumnCount,PanelView->FullScreen);
	return(PrepareColumnWidths(PanelView->ColumnType,PanelView->ColumnWidth,
	                           PanelView->ColumnCount,PanelView->FullScreen));
}


int FileList::PrepareColumnWidths(unsigned int *ColumnTypes,int *ColumnWidths,
                                  int &ColumnCount,int FullScreen)
{
	int TotalWidth,ZeroLengthCount,EmptyColumns,I;
	ZeroLengthCount=EmptyColumns=0;
	TotalWidth=ColumnCount-1;

	for (I=0; I<ColumnCount; I++)
	{
		if (ColumnWidths[I]<0)
		{
			EmptyColumns++;
			continue;
		}

		int ColumnType=ColumnTypes[I] & 0xff;

		if (ColumnWidths[I]==0)
		{
			ColumnWidths[I]=ColumnTypeWidth[ColumnType];

			if (ColumnType==MDATE_COLUMN || ColumnType==CDATE_COLUMN || ColumnType==ADATE_COLUMN)
			{
				if (ColumnTypes[I] & COLUMN_BRIEF)
					ColumnWidths[I]-=3;

				if (ColumnTypes[I] & COLUMN_MONTH)
					ColumnWidths[I]++;
			}
		}

		if (ColumnWidths[I]==0)
			ZeroLengthCount++;

		TotalWidth+=ColumnWidths[I];
	}

	TotalWidth-=EmptyColumns;
	int PanelTextWidth=X2-X1-1;

	if (FullScreen)
		PanelTextWidth=ScrX-1;

	int ExtraWidth=PanelTextWidth-TotalWidth;

	for (I=0; I<ColumnCount && ZeroLengthCount>0; I++)
		if (ColumnWidths[I]==0)
		{
			int AutoWidth=ExtraWidth/ZeroLengthCount;

			if (AutoWidth<1)
				AutoWidth=1;

			ColumnWidths[I]=AutoWidth;
			ExtraWidth-=AutoWidth;
			ZeroLengthCount--;
		}

	while (1)
	{
		int LastColumn=ColumnCount-1;
		TotalWidth=LastColumn-EmptyColumns;

		for (I=0; I<ColumnCount; I++)
			if (ColumnWidths[I]>0)
				TotalWidth+=ColumnWidths[I];

		if (TotalWidth<=PanelTextWidth)
			break;

		if (ColumnCount<=1)
		{
			ColumnWidths[0]=PanelTextWidth;
			break;
		}
		else if (PanelTextWidth>=TotalWidth-ColumnWidths[LastColumn])
		{
			ColumnWidths[LastColumn]=PanelTextWidth-(TotalWidth-ColumnWidths[LastColumn]);
			break;
		}
		else
			ColumnCount--;
	}

	ColumnsInGlobal = 1;
	int GlobalColumns;
	bool UnEqual;
	int Remainder;

	for (int i = 0; i < ViewSettings.ColumnCount; i++)
	{
		UnEqual = false;
		Remainder = ViewSettings.ColumnCount%ColumnsInGlobal;
		GlobalColumns = ViewSettings.ColumnCount/ColumnsInGlobal;

		if (!Remainder)
		{
			for (int k = 0; k < GlobalColumns-1; k++)
			{
				for (int j = 0; j < ColumnsInGlobal; j++)
				{
					if ((ViewSettings.ColumnType[k*ColumnsInGlobal+j] & 0xFF) !=
					        (ViewSettings.ColumnType[(k+1)*ColumnsInGlobal+j] & 0xFF))
						UnEqual = true;
				}
			}

			if (!UnEqual)
				break;
		}

		ColumnsInGlobal++;
	}

	return(GlobalColumns);
}


extern void GetColor(int PaletteIndex);

void FileList::ShowList(int ShowStatus,int StartColumn)
{
	int StatusShown=FALSE;
	int MaxLeftPos=0,MinLeftPos=FALSE;
	int ColumnCount=ShowStatus ? ViewSettings.StatusColumnCount:ViewSettings.ColumnCount;

	for (int I=Y1+1+Opt.ShowColumnTitles,J=CurTopFile; I<Y2-2*Opt.ShowPanelStatus; I++,J++)
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

		for (int K=0; K<ColumnCount; K++)
		{
			int ListPos=J+CurColumn*Height;

			if (ShowStatus)
				if (CurFile!=ListPos)
				{
					CurColumn++;
					continue;
				}
				else
					StatusLine=TRUE;

			int CurX=WhereX();
			int CurY=WhereY();
			int ShowDivider=TRUE;
			unsigned int *ColumnTypes=ShowStatus ? ViewSettings.StatusColumnType:ViewSettings.ColumnType;
			int *ColumnWidths=ShowStatus ? ViewSettings.StatusColumnWidth:ViewSettings.ColumnWidth;
			int ColumnType=ColumnTypes[K] & 0xff;
			int ColumnWidth=ColumnWidths[K];

			if (ColumnWidth<0)
			{
				if (!ShowStatus && K==ColumnCount-1)
				{
					SetColor(COL_PANELBOX);
					GotoXY(CurX-1,CurY);
					BoxText((WORD)(CurX-1==X2 ? (Opt.UseUnicodeConsole?BoxSymbols[VerticalLine[1]-0x0B0]:VerticalLine[1]):0x20));
				}

				continue;
			}

			if (ListPos<FileCount)
			{
				struct FileListItem *CurPtr=ListData+ListPos;

				if (!ShowStatus && !StatusShown && CurFile==ListPos && Opt.ShowPanelStatus)
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
					int ColumnNumber=ColumnType-CUSTOM_COLUMN0;
					char *ColumnData=NULL;

					if (ColumnNumber<CurPtr->CustomColumnNumber)
						ColumnData=CurPtr->CustomColumnData[ColumnNumber];

					if (ColumnData==NULL)
						ColumnData="";

					int CurLeftPos=0;

					if (!ShowStatus && LeftPos>0)
					{
						int Length=(int)strlen(ColumnData);

						if (Length>ColumnWidth)
						{
							CurLeftPos=LeftPos;

							if (CurLeftPos>Length-ColumnWidth)
								CurLeftPos=Length-ColumnWidth;

							if (CurLeftPos>MaxLeftPos)
								MaxLeftPos=CurLeftPos;
						}
					}

					mprintf("%-*.*s",ColumnWidth,ColumnWidth,ColumnData+CurLeftPos);
				}
				else
				{
					switch (ColumnType)
					{
						case NAME_COLUMN:
						{
							char NewName[NM];
							int Width=ColumnWidth;
							int ViewFlags=ColumnTypes[K];

							if ((ViewFlags & COLUMN_MARK) && Width>2)
							{
								static char SelectedChar[4]={0x0FB,0x20,0x00,0x00};
								Text(CurPtr->Selected ? SelectedChar:"  ");
								Width-=2;
							}

							if (CurPtr->Colors.MarkChar && Opt.Highlight && Width>1)
							{
								Width--;
								OutCharacter[0]=(char)CurPtr->Colors.MarkChar;
								int OldColor=GetColor();

								if (!ShowStatus)
									SetShowColor(ListPos,HIGHLIGHTCOLORTYPE_MARKCHAR);

								Text(OutCharacter);
								SetColor(OldColor);
							}

							char *NamePtr=ShowShortNames && *CurPtr->ShortName && !ShowStatus ?
							              CurPtr->ShortName:CurPtr->Name;

							if (ViewFlags & COLUMN_NAMEONLY)
							{
								// !!! НЕ УВЕРЕН, но то, что отображается пустое
								// пространство вместо названия - бага
								NamePtr=PointToFolderNameIfFolder(NamePtr);
							}

							char *SrcNamePtr=NamePtr;
							int CurLeftPos=0;
							int RightAlign=(ViewFlags & COLUMN_RIGHTALIGN);
							int LeftBracket=FALSE,RightBracket=FALSE;

							if (!ShowStatus && LeftPos!=0)
							{
								int Length=(int)strlen(NamePtr);

								if (Length>Width)
								{
									if (LeftPos>0)
									{
										if (!RightAlign)
										{
											CurLeftPos=LeftPos;

											if (Length-CurLeftPos<Width)
												CurLeftPos=Length-Width;

											NamePtr+=CurLeftPos;

											if (CurLeftPos>MaxLeftPos)
												MaxLeftPos=CurLeftPos;
										}
									}
									else if (RightAlign)
									{
										int CurRightPos=LeftPos;

										if (Length+CurRightPos<Width)
											CurRightPos=Width-Length;
										else
											RightBracket=TRUE;

										NamePtr+=Length+CurRightPos-Width;
										RightAlign=FALSE;

										if (CurRightPos<MinLeftPos)
											MinLeftPos=CurRightPos;
									}
								}
							}

							int TooLong=ConvertName(NamePtr,NewName,
							                        Min((int)Width,(int)sizeof(NewName)-1), // BugZ#997 - падение FAR при большом выставлении высоты буфера экрана
							                        RightAlign,ShowStatus,CurPtr->FileAttr);

							if (CurLeftPos!=0)
								LeftBracket=TRUE;

							if (TooLong)
							{
								if (RightAlign)
									LeftBracket=TRUE;

								if (!RightAlign && (int) strlen(NamePtr)>Width)
									RightBracket=TRUE;
							}

							if (!ShowStatus)
							{
								if (!ShowShortNames && ViewSettings.FileUpperToLowerCase)
									if (!(CurPtr->FileAttr & FA_DIREC) && !IsCaseMixed(SrcNamePtr))
										LocalStrlwr(NewName);

								if ((ShowShortNames || ViewSettings.FolderUpperCase) && (CurPtr->FileAttr & FA_DIREC))
									LocalStrupr(NewName);

								if ((ShowShortNames || ViewSettings.FileLowerCase) && (CurPtr->FileAttr & FA_DIREC)==0)
									LocalStrlwr(NewName);
							}

							Text(NewName);
							int NameX=WhereX();
							/* $ 09.11.2001 IS
							     вместо фигурных скобок используем данные из lng
							*/

							if (!ShowStatus)
							{
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
									if (Level == ColumnsInGlobal)
										SetColor(COL_PANELBOX);

									GotoXY(NameX,CurY);
									Text(closeBracket);
									ShowDivider=FALSE;

									if (Level == ColumnsInGlobal)
										SetColor(COL_PANELTEXT);
									else
										SetShowColor(J);
								}
							}

							/* IS $ */
						}
						break;
						case SIZE_COLUMN:
						case PACKED_COLUMN:
						{
							int Packed=(ColumnType==PACKED_COLUMN);
							char Str[100];
							int Width=ColumnWidth;

							if (CurPtr->ShowFolderSize==2)
							{
								Width--;
								Text("~");
							}

							if (!Packed && (CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY) && !CurPtr->ShowFolderSize)
							{
								char *PtrName=MSG(MListFolder);;

								if (TestParentFolderName(CurPtr->Name))
									PtrName=MSG(MListUp);
								else
								{
									if (CurPtr->FileAttr&FILE_ATTRIBUTE_REPARSE_POINT)
									{
										switch (CurPtr->ReparseTag)
										{
											case IO_REPARSE_TAG_SYMLINK:
												PtrName=MSG(MListSymLink);
												break;
											case IO_REPARSE_TAG_MOUNT_POINT:
												PtrName=MSG(MListJunction);
												{
													char JuncName[NM*2];
													DWORD ReparseTag=0;
													DWORD LenJunction=GetReparsePointInfo(CurPtr->Name,
													                                      JuncName, sizeof(JuncName),
													                                      &ReparseTag);
													//"\??\D:\Junc\Src\" или "\\?\Volume{..."
													int offset = 0;

													if (!strncmp(JuncName,"\\??\\", sizeof("\\??\\")-1))
														offset += 4;

													if (IsLocalVolumePath(JuncName) && !JuncName[49])
														PtrName=MSG(MListVolMount);
												}
												break;
										}
									}
								}

								if (strlen(PtrName) <= static_cast<size_t>(Width-2))
									sprintf(Str,"<%.*s>",sizeof(Str)-3,PtrName);
								else
									xstrncpy(Str,PtrName,sizeof(Str)-1);

								mprintf("%*.*s",Width,Width,Str);
							}
							else
							{
								char OutStr[64];
								// подсократим - весь код по форматированию размера
								//   в отдельную функцию - FileSizeToStr().
								mprintf("%s",FileSizeToStr(OutStr,
								                           Packed?MKUINT64(CurPtr->PackSizeHigh,CurPtr->PackSize):MKUINT64(CurPtr->UnpSizeHigh,CurPtr->UnpSize),
								                           Width,ColumnTypes[K]));
							}
						}
						break;
						case DATE_COLUMN:
						{
							char OutStr[30];
							ConvertDate(CurPtr->WriteTime,OutStr,NULL,0,FALSE,FALSE,ColumnWidth>9);
							mprintf("%*.*s",ColumnWidth,ColumnWidth,OutStr);
						}
						break;
						case TIME_COLUMN:
						{
							char OutStr[30];
							ConvertDate(CurPtr->WriteTime,NULL,OutStr,ColumnWidth);
							mprintf("%*.*s",ColumnWidth,ColumnWidth,OutStr);
						}
						break;
						case MDATE_COLUMN:
						case CDATE_COLUMN:
						case ADATE_COLUMN:
						{
							FILETIME *FileTime;

							switch (ColumnType)
							{
								case MDATE_COLUMN:
									FileTime=&CurPtr->WriteTime;
									break;
								case CDATE_COLUMN:
									FileTime=&CurPtr->CreationTime;
									break;
								case ADATE_COLUMN:
									FileTime=&CurPtr->AccessTime;
									break;
							}

							int TextMonth=(ColumnTypes[K] & COLUMN_MONTH)!=0;
							int Brief=ColumnTypes[K] & COLUMN_BRIEF;
							int FullYear=FALSE;

							if (!Brief)
							{
								int CmpWidth=ColumnWidth-TextMonth;

								if (CmpWidth==15 || CmpWidth==16 || CmpWidth==18 ||
								        CmpWidth==19 || CmpWidth>21)
									FullYear=TRUE;
							}

							char DateStr[30],TimeStr[30],OutStr[30];
							ConvertDate(*FileTime,DateStr,TimeStr,ColumnWidth-9,Brief,TextMonth,FullYear);
							sprintf(OutStr,"%s %s",DateStr,TimeStr);
							mprintf("%*.*s",ColumnWidth,ColumnWidth,OutStr);
						}
						break;
						case ATTR_COLUMN:
						{
							char OutStr[16];
							DWORD FileAttr=CurPtr->FileAttr;
							//
							OutStr[0]=(FileAttr & FILE_ATTRIBUTE_READONLY) ? 'R':' ';
							OutStr[1]=(FileAttr & FILE_ATTRIBUTE_SYSTEM) ? 'S':' ';
							OutStr[2]=(FileAttr & FILE_ATTRIBUTE_HIDDEN) ? 'H':' ';
							OutStr[3]=(FileAttr & FILE_ATTRIBUTE_ARCHIVE) ? 'A':' ';
							OutStr[4]=(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) ? 'L' : ((FileAttr & FILE_ATTRIBUTE_SPARSE_FILE) ? '$':' ');
							// $ 20.10.2000 SVS - Encrypted NTFS/Win2K - Поток может быть либо COMPRESSED (С) либо ENCRYPTED (E)
							OutStr[5]=(FileAttr & FILE_ATTRIBUTE_COMPRESSED) ? 'C':((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)?'E':' ');
							OutStr[6]=(FileAttr & FILE_ATTRIBUTE_TEMPORARY) ? 'T':' ';
							OutStr[7]=(FileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) ? 'I':' ';
							OutStr[8]=(FileAttr & FILE_ATTRIBUTE_OFFLINE) ? 'O':' ';
							OutStr[9]=(FileAttr & FILE_ATTRIBUTE_VIRTUAL) ? 'V':' ';
							OutStr[10]=0;
							char *OutPtr=OutStr;
							//if (ColumnWidth<7)
							//  OutPtr=OutStr+7-ColumnWidth;
							mprintf("%*.*s",ColumnWidth,ColumnWidth,OutPtr);
						}
						break;
						case DIZ_COLUMN:
						{
							int CurLeftPos=0;

							if (!ShowStatus && LeftPos>0)
							{
								int Length=CurPtr->DizText ? (int)strlen(CurPtr->DizText):0;

								if (Length>ColumnWidth)
								{
									CurLeftPos=LeftPos;

									if (CurLeftPos>Length-ColumnWidth)
										CurLeftPos=Length-ColumnWidth;

									if (CurLeftPos>MaxLeftPos)
										MaxLeftPos=CurLeftPos;
								}
							}

							char DizText[1024];
							xstrncpy(DizText,CurPtr->DizText ? CurPtr->DizText+CurLeftPos:"",sizeof(DizText)-1);
							DizText[sizeof(DizText)-1]=0;
							char *DizEnd=strchr(DizText,'\4');

							if (DizEnd!=NULL)
								*DizEnd=0;

							mprintf("%-*.*s",ColumnWidth,ColumnWidth,DizText);
						}
						break;
						case OWNER_COLUMN:
						{
							const char* Owner=CurPtr->Owner;

							if (Owner && !(ColumnTypes[K]&COLUMN_FULLOWNER) && PanelMode!=PLUGIN_PANEL)
							{
								const char* SlashPos=strchr(Owner,'\\');

								if (SlashPos)Owner=SlashPos+1;
							}

							int CurLeftPos=0;

							if (!ShowStatus && LeftPos>0)
							{
								int Length=(int)strlen(Owner);

								if (Length>ColumnWidth)
								{
									CurLeftPos=LeftPos;

									if (CurLeftPos>Length-ColumnWidth)
										CurLeftPos=Length-ColumnWidth;

									if (CurLeftPos>MaxLeftPos)
										MaxLeftPos=CurLeftPos;
								}
							}

							mprintf("%-*.*s",ColumnWidth,ColumnWidth,Owner+CurLeftPos);
						}
						break;
						case NUMLINK_COLUMN:
						{
							char OutStr[20];
							mprintf("%*.*s",ColumnWidth,ColumnWidth,itoa(CurPtr->NumberOfLinks,OutStr,10));
						}
						break;
					}
				}
			}
			else
				mprintf("%*s",ColumnWidth,"");

			if (ShowDivider==FALSE)
				GotoXY(CurX+ColumnWidth+1,CurY);
			else
			{
				if (!ShowStatus)
				{
					SetShowColor(ListPos);

					if (Level == ColumnsInGlobal)
						SetColor(COL_PANELBOX);
				}

				if (K == ColumnCount-1)
					SetColor(COL_PANELBOX);

				GotoXY(CurX+ColumnWidth,CurY);

				if (K==ColumnCount-1)
					BoxText((WORD)(CurX+ColumnWidth==X2 ? (Opt.UseUnicodeConsole?BoxSymbols[VerticalLine[1]-0x0B0]:VerticalLine[1]):0x20));
				else
					BoxText((WORD)(ShowStatus ? 0x20:(Opt.UseUnicodeConsole?BoxSymbols[VerticalLine[0]-0x0B0]:VerticalLine[0])));

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
			mprintf("%*s",X2-WhereX(),"");
		}
	}

	if (!ShowStatus && !StatusShown && Opt.ShowPanelStatus)
	{
		SetScreen(X1+1,Y2-1,X2-1,Y2-1,' ',COL_PANELTEXT);
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y2-1);
		//mprintf("%*s",X2-X1-1,"");
	}

	if (!ShowStatus)
	{
		if (LeftPos<0)
			LeftPos=MinLeftPos;

		if (LeftPos>0)
			LeftPos=MaxLeftPos;
	}
}


int FileList::IsFullScreen()
{
	return this->ViewSettings.FullScreen;
}


int FileList::IsModeFullScreen(int Mode)
{
	return(ViewSettingsArray[Mode].FullScreen);
}


int FileList::IsDizDisplayed()
{
	return(IsColumnDisplayed(DIZ_COLUMN));
}


int FileList::IsColumnDisplayed(int Type)
{
	int I;

	for (I=0; I<ViewSettings.ColumnCount; I++)
		if ((ViewSettings.ColumnType[I] & 0xff)==Type)
			return(TRUE);

	for (I=0; I<ViewSettings.StatusColumnCount; I++)
		if ((ViewSettings.StatusColumnType[I] & 0xff)==Type)
			return(TRUE);

	return(FALSE);
}


int FileList::IsCaseSensitive()
{
	return(ViewSettings.CaseSensitiveSort);
}
