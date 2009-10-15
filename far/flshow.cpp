/*
flshow.cpp

Файловая панель - вывод на экран
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
#include "colors.hpp"
#include "lang.hpp"
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

extern PanelViewSettings ViewSettingsArray[];
extern int ColumnTypeWidth[];

static wchar_t OutCharacter[8]={0,0,0,0,0,0,0,0};

static int __FormatEndSelectedPhrase(int Count)
{
  int M_Fmt=MListFileSize;
  if(Count != 1)
  {
    char StrItems[32];
    itoa(Count,StrItems,10);
    int LenItems=(int)strlen(StrItems);
    if(StrItems[LenItems-1] == '1' && Count != 11)
      M_Fmt=MListFilesSize1;
    else
      M_Fmt=MListFilesSize2;
  }
  return M_Fmt;
}


void FileList::DisplayObject()
{
  Height=Y2-Y1-4+!Opt.ShowColumnTitles+(Opt.ShowPanelStatus ? 0:2);
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
  if ( Locked() )
  {
    CorrectPosition();
    return;
  }
  string strTitle;
  string strInfoCurDir;
  int Length;
	OpenPluginInfo Info;

  if (PanelMode==PLUGIN_PANEL)
  {
    if (ProcessPluginEvent(FE_REDRAW,NULL))
      return;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    strInfoCurDir=Info.CurDir;
  }

  int CurFullScreen=IsFullScreen();
  PrepareViewSettings(ViewMode,&Info);
  CorrectPosition();

  if(CurFullScreen!=IsFullScreen())
  {
    CtrlObject->Cp()->SetScreenPosition();
    CtrlObject->Cp()->GetAnotherPanel(this)->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  }

  SetScreen(X1+1,Y1+1,X2-1,Y2-1,L' ',COL_PANELTEXT);
  Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
  if (Opt.ShowColumnTitles)
  {
//    SetScreen(X1+1,Y1+1,X2-1,Y1+1,' ',COL_PANELTEXT);
    SetColor(COL_PANELTEXT); //???
    //GotoXY(X1+1,Y1+1);
    //mprintf("%*s",X2-X1-1,"");
  }
  for (int I=0,ColumnPos=X1+1;I<ViewSettings.ColumnCount;I++)
  {
    if (ViewSettings.ColumnWidth[I]<0)
      continue;
    if (Opt.ShowColumnTitles)
    {
      string strTitle;
      int IDMessage=-1;
      switch(ViewSettings.ColumnType[I] & 0xff)
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
				case NUMSTREAMS_COLUMN:
					IDMessage=MColumnNumStreams;
					break;
				case STREAMSSIZE_COLUMN:
					IDMessage=MColumnStreamsSize;
					break;
      }
      if(IDMessage != -1)
        strTitle=MSG(IDMessage);

      if (PanelMode==PLUGIN_PANEL && Info.PanelModesArray!=NULL &&
          ViewMode<Info.PanelModesNumber &&
          Info.PanelModesArray[ViewMode].ColumnTitles!=NULL)
      {
        wchar_t *NewTitle=Info.PanelModesArray[ViewMode].ColumnTitles[I];
        if (NewTitle!=NULL)
          strTitle=NewTitle;
      }
      string strTitleMsg;
      CenterStr(strTitle,strTitleMsg,ViewSettings.ColumnWidth[I]);
      SetColor(COL_PANELCOLUMNTITLE);
      GotoXY(ColumnPos,Y1+1);
			FS<<fmt::Precision(ViewSettings.ColumnWidth[I])<<strTitleMsg;
    }
    if (I>=ViewSettings.ColumnCount-1)
      break;
    if (ViewSettings.ColumnWidth[I+1]<0)
      continue;
    SetColor(COL_PANELBOX);
    ColumnPos+=ViewSettings.ColumnWidth[I];
    GotoXY(ColumnPos,Y1);
    BoxText(BoxSymbols[BS_T_H2V1]);
    if (Opt.ShowColumnTitles)
    {
      GotoXY(ColumnPos,Y1+1);
      BoxText(BoxSymbols[BS_V1]);
    }
    if (!Opt.ShowPanelStatus)
    {
      GotoXY(ColumnPos,Y2);
      BoxText(BoxSymbols[BS_B_H2V1]);
    }
    ColumnPos++;
  }

  int NextX1=X1+1;
  if (Opt.ShowSortMode)
  {
    static int SortModes[]={UNSORTED,BY_NAME,BY_EXT,BY_MTIME,BY_CTIME,
                            BY_ATIME,BY_SIZE,BY_DIZ,BY_OWNER,
														BY_COMPRESSEDSIZE,BY_NUMLINKS,
														BY_NUMSTREAMS,BY_STREAMSSIZE};
    static int SortStrings[]={MMenuUnsorted,MMenuSortByName,
      MMenuSortByExt,MMenuSortByModification,MMenuSortByCreation,
      MMenuSortByAccess,MMenuSortBySize,MMenuSortByDiz,MMenuSortByOwner,
      MMenuSortByCompressedSize,MMenuSortByNumLinks,MMenuSortByNumStreams,MMenuSortByStreamsSize};
    for (size_t I=0;I<countof(SortModes);I++)
    {
      if (SortModes[I]==SortMode)
      {
        const wchar_t *SortStr=MSG(SortStrings[I]);
        const wchar_t *Ch=wcschr(SortStr,L'&');
        if (Ch!=NULL)
        {
          if (Opt.ShowColumnTitles)
            GotoXY(NextX1,Y1+1);
          else
            GotoXY(NextX1,Y1);
          SetColor(COL_PANELCOLUMNTITLE);
          OutCharacter[0]=SortOrder==1 ? Lower(Ch[1]):Upper(Ch[1]);
          Text(OutCharacter);
          NextX1++;
          if (Filter!=NULL && Filter->IsEnabledOnPanel())
          {
            OutCharacter[0]=L'*';
            Text(OutCharacter);
            NextX1++;
          }
        }
        break;
      }
    }
  }

  /* <режимы сортировки> */
  if(GetNumericSort() || GetSortGroups() || GetSelectedFirstMode())
  {
    if (Opt.ShowColumnTitles)
      GotoXY(NextX1,Y1+1);
    else
      GotoXY(NextX1,Y1);

    SetColor(COL_PANELCOLUMNTITLE);
    wchar_t *PtrOutCharacter=OutCharacter;
    *PtrOutCharacter=0;

    if(GetSelectedFirstMode())
      *PtrOutCharacter++=L'^';

/*
    if(GetNumericSort())
      *PtrOutCharacter++=L'#';
    if(GetSortGroups())
      *PtrOutCharacter++=L'@';
*/
    *PtrOutCharacter=0;

    Text(OutCharacter);
    PtrOutCharacter[1]=0;
  }
  /* </режимы сортировки> */

  if (!Fast && GetFocus())
  {
    if ( PanelMode==PLUGIN_PANEL )
        CtrlObject->CmdLine->SetCurDir(Info.CurDir);
    else
      CtrlObject->CmdLine->SetCurDir(strCurDir);

    CtrlObject->CmdLine->Show();
  }
  int TitleX2=Opt.Clock && !Opt.ShowMenuBar ? Min(ScrX-4,X2):X2;
  int TruncSize=TitleX2-X1-3;
  if (!Opt.ShowColumnTitles && Opt.ShowSortMode && Filter!=NULL && Filter->IsEnabledOnPanel())
    TruncSize-=2;

  GetTitle(strTitle,TruncSize,2);//,(PanelMode==PLUGIN_PANEL?0:2));
  Length=(int)strTitle.GetLength();
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
  Text(strTitle);
  if (FileCount==0)
  {
    SetScreen(X1+1,Y2-1,X2-1,Y2-1,L' ',COL_PANELTEXT);
    SetColor(COL_PANELTEXT); //???
    //GotoXY(X1+1,Y2-1);
    //mprintf("%*s",X2-X1-1,"");
  }

  if (PanelMode==PLUGIN_PANEL && FileCount>0 && (Info.Flags & OPIF_REALNAMES))
  {
    if (!strInfoCurDir.IsEmpty())
    {
      strCurDir = strInfoCurDir;
    }
    else
    {
			if(!TestParentFolderName(ListData[CurFile]->strName))
      {
				strCurDir=ListData[CurFile]->strName;
        size_t pos;
				if(LastSlash(strCurDir,pos))
        {
          if (pos)
          {
            if (strCurDir.At(pos-1)!=L':')
              strCurDir.SetLength(pos);
            else
              strCurDir.SetLength(pos+1);
          }
        }
      }
    }
    if (GetFocus())
    {
      CtrlObject->CmdLine->SetCurDir(strCurDir);
      CtrlObject->CmdLine->Show();
    }
  }

  if ((Opt.ShowPanelTotals || Opt.ShowPanelFree) &&
      (Opt.ShowPanelStatus || SelFileCount==0))
  {
    ShowTotalSize(Info);
  }
  ShowList(FALSE,0);
  ShowSelectedSize();
  if (Opt.ShowPanelScrollbar)
  {
    SetColor(COL_PANELSCROLLBAR);
		ScrollBarEx(X2,Y1+1+Opt.ShowColumnTitles,Height,Round(CurTopFile,Columns),Round(FileCount,Columns));
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
    int Pos = HIGHLIGHTCOLOR_NORMAL;

    if (CurFile==Position && Focus && FileCount > 0)
    {
			Pos=ListData[Position]->Selected?HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR:HIGHLIGHTCOLOR_UNDERCURSOR;
    }
    else
			if (ListData[Position]->Selected)
        Pos = HIGHLIGHTCOLOR_SELECTED;

		ColorAttr=ListData[Position]->Colors.Color[ColorType][Pos];
    if (!ColorAttr || !Opt.Highlight)
      ColorAttr=FarColor[Pos];
  }

  return ColorAttr;
}

void FileList::SetShowColor (int Position, int ColorType)
{
  SetColor (GetShowColor(Position,ColorType));
}

void FileList::ShowSelectedSize()
{
  int Length;
  string strSelStr, strFormStr;

  if (Opt.ShowPanelStatus)
  {
    SetColor(COL_PANELBOX);
    DrawSeparator(Y2-2);
    for (int I=0,ColumnPos=X1+1;I<ViewSettings.ColumnCount-1;I++)
    {
      if (ViewSettings.ColumnWidth[I]<0 ||
          (I==ViewSettings.ColumnCount-2 && ViewSettings.ColumnWidth[I+1]<0))
        continue;
      ColumnPos+=ViewSettings.ColumnWidth[I];
      GotoXY(ColumnPos,Y2-2);
      BoxText(BoxSymbols[BS_B_H1V1]);
      ColumnPos++;
    }
  }

  if (SelFileCount)
  {
    InsertCommas(SelFileSize,strFormStr);
    strSelStr.Format (MSG(__FormatEndSelectedPhrase(SelFileCount)),(const wchar_t*)strFormStr,SelFileCount);

    TruncStr(strSelStr,X2-X1-1);
    Length=(int)strSelStr.GetLength();
    SetColor(COL_PANELSELECTEDINFO);
    GotoXY(X1+(X2-X1+1-Length)/2,Y2-2*Opt.ShowPanelStatus);
    Text(strSelStr);
  }
}


void FileList::ShowTotalSize(OpenPluginInfo &Info)
{
  if (!Opt.ShowPanelTotals && PanelMode==PLUGIN_PANEL && (Info.Flags & OPIF_REALNAMES)==0)
    return;
  string strTotalStr, strFormSize, strFreeSize;
  int Length;

  InsertCommas(TotalFileSize,strFormSize);
  if (Opt.ShowPanelFree && (PanelMode!=PLUGIN_PANEL || (Info.Flags & OPIF_REALNAMES)))
    InsertCommas(FreeDiskSize,strFreeSize);
  if (Opt.ShowPanelTotals)
  {
    if (!Opt.ShowPanelFree || strFreeSize.IsEmpty())
      strTotalStr.Format (MSG(__FormatEndSelectedPhrase(TotalFileCount)),(const wchar_t*)strFormSize,TotalFileCount);
    else
    {
      wchar_t DHLine[4]={BoxSymbols[BS_H2],BoxSymbols[BS_H2],BoxSymbols[BS_H2],0};
      strTotalStr.Format (L" %s (%d) %s %s ",(const wchar_t*)strFormSize,TotalFileCount,DHLine,(const wchar_t*)strFreeSize);
      if ((int)strTotalStr.GetLength()> X2-X1-1)
      {
        InsertCommas(FreeDiskSize>>20,strFreeSize);
        InsertCommas(TotalFileSize>>20,strFormSize);
        strTotalStr.Format (L" %s %s (%d) %s %s %s ",(const wchar_t*)strFormSize,MSG(MListMb),TotalFileCount,DHLine,(const wchar_t*)strFreeSize,MSG(MListMb));
      }
    }
  }
  else
    strTotalStr.Format (MSG(MListFreeSize), !strFreeSize.IsEmpty() ? (const wchar_t*)strFreeSize:L"???");

  SetColor(COL_PANELTOTALINFO);
  /* $ 01.08.2001 VVM
    + Обрезаем строчку справа, а не слева */
  TruncStrFromEnd(strTotalStr, X2-X1-1);
  Length=(int)strTotalStr.GetLength();
  GotoXY(X1+(X2-X1+1-Length)/2,Y2);

  const wchar_t *FirstBox=wcschr(strTotalStr,BoxSymbols[BS_H2]);
  int BoxPos=(FirstBox==NULL) ? -1:(int)(FirstBox-(const wchar_t*)strTotalStr);
  int BoxLength=0;
  if (BoxPos!=-1)
    for (int I=0;strTotalStr.At(BoxPos+I)==BoxSymbols[BS_H2];I++)
      BoxLength++;

  if (BoxPos==-1 || BoxLength==0)
    Text(strTotalStr);
  else
  {
		FS<<fmt::Precision(BoxPos)<<strTotalStr;
    SetColor(COL_PANELBOX);
		FS<<fmt::Precision(BoxLength)<<strTotalStr.CPtr()+BoxPos;
    SetColor(COL_PANELTOTALINFO);
		Text(strTotalStr.CPtr()+BoxPos+BoxLength);
  }
}

int FileList::ConvertName(const wchar_t *SrcName,string &strDest,int MaxLength,int RightAlign,int ShowStatus,DWORD FileAttr)
{
  wchar_t *lpwszDest = strDest.GetBuffer (MaxLength+1);

  wmemset(lpwszDest,L' ',MaxLength);

  int SrcLength=StrLength(SrcName);
  if (RightAlign && SrcLength>MaxLength)
  {
    wmemcpy(lpwszDest,SrcName+SrcLength-MaxLength,MaxLength);
    strDest.ReleaseBuffer (MaxLength);
    return(TRUE);
  }
  const wchar_t *DotPtr;
  if (!ShowStatus &&
      ((!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && ViewSettings.AlignExtensions) || ((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && ViewSettings.FolderAlignExtensions))
      && SrcLength<=MaxLength &&
      (DotPtr=wcsrchr(SrcName,L'.'))!=NULL && DotPtr!=SrcName &&
      (SrcName[0]!=L'.' || SrcName[2]!=0) && wcschr(DotPtr+1,L' ')==NULL)
  {
    int DotLength=StrLength(DotPtr+1);
    int NameLength=(int)(DotPtr-SrcName);
    int DotPos=MaxLength-Max(DotLength,3);
    if (DotPos<=NameLength)
      DotPos=NameLength+1;
    if (DotPos>0 && NameLength>0 && SrcName[NameLength-1]==L' ')
      lpwszDest[NameLength]=L'.';
    wmemcpy(lpwszDest,SrcName,NameLength);
    wmemcpy(lpwszDest+DotPos,DotPtr+1,DotLength);
  }
  else
  {
    wmemcpy(lpwszDest,SrcName,Min(SrcLength, MaxLength));
  }

  strDest.ReleaseBuffer (MaxLength);

  return(SrcLength>MaxLength);
}


void FileList::PrepareViewSettings(int ViewMode,OpenPluginInfo *PlugInfo)
{
	OpenPluginInfo Info={0};
  if (PanelMode==PLUGIN_PANEL)
  {
    if (PlugInfo==NULL)
      CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    else
      Info=*PlugInfo;
  }

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
                         ViewSettings.ColumnWidthType,ViewSettings.ColumnCount);
      if (Info.PanelModesArray[ViewMode].StatusColumnTypes!=NULL &&
          Info.PanelModesArray[ViewMode].StatusColumnWidths!=NULL)
        TextToViewSettings(Info.PanelModesArray[ViewMode].StatusColumnTypes,
                           Info.PanelModesArray[ViewMode].StatusColumnWidths,
                           ViewSettings.StatusColumnType,ViewSettings.StatusColumnWidth,
                           ViewSettings.StatusColumnWidthType,ViewSettings.StatusColumnCount);
      else
        if (Info.PanelModesArray[ViewMode].DetailedStatus)
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
      for (int I=0;I<ViewSettings.ColumnCount;I++)
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


int FileList::PreparePanelView(PanelViewSettings *PanelView)
{
  PrepareColumnWidths(PanelView->StatusColumnType,PanelView->StatusColumnWidth,PanelView->StatusColumnWidthType,
                      PanelView->StatusColumnCount,PanelView->FullScreen);
  return(PrepareColumnWidths(PanelView->ColumnType,PanelView->ColumnWidth,PanelView->ColumnWidthType,
                             PanelView->ColumnCount,PanelView->FullScreen));
}


int FileList::PrepareColumnWidths(unsigned int *ColumnTypes,int *ColumnWidths,
              int *ColumnWidthsTypes,int &ColumnCount,int FullScreen)
{
  int TotalWidth,TotalPercentWidth,TotalPercentCount,ZeroLengthCount,EmptyColumns,I;
  ZeroLengthCount=EmptyColumns=0;
  TotalWidth=ColumnCount-1;
  TotalPercentCount=TotalPercentWidth=0;
  for (I=0;I<ColumnCount;I++)
  {
    if (ColumnWidths[I]<0)
    {
      EmptyColumns++;
      continue;
    }
    int ColumnType=ColumnTypes[I] & 0xff;
    if (ColumnWidths[I]==0)
    {
			ColumnWidthsTypes[I] = COUNT_WIDTH; //manage all zero-width columns in same way
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

    switch (ColumnWidthsTypes[I])
    {
    	case COUNT_WIDTH:
    		TotalWidth+=ColumnWidths[I];
    		break;
    	case PERCENT_WIDTH:
    		TotalPercentWidth+=ColumnWidths[I];
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
	  int ExtraPercentWidth=(TotalPercentWidth>100 || ZeroLengthCount==0)?ExtraWidth:ExtraWidth*TotalPercentWidth/100;
	  int TempWidth=0;

		for (I=0;I<ColumnCount && TotalPercentCount>0;I++)
			if (ColumnWidthsTypes[I]==PERCENT_WIDTH)
			{
				int PercentWidth = (TotalPercentCount>1)?(ExtraPercentWidth*ColumnWidths[I]/TotalPercentWidth):(ExtraPercentWidth-TempWidth);
				if (PercentWidth<1)
					PercentWidth=1;
				TempWidth+=PercentWidth;
				ColumnWidths[I]=PercentWidth;
				ColumnWidthsTypes[I] = COUNT_WIDTH;
				TotalPercentCount--;
			}

	  ExtraWidth-=TempWidth;
	 }

  for (I=0;I<ColumnCount && ZeroLengthCount>0;I++)
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
    for (I=0;I<ColumnCount;I++)
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

  int GlobalColumns=0;
  bool UnEqual;
  int Remainder;

  for (int i = 0; i < ViewSettings.ColumnCount; i++)
  {
    UnEqual = false;

    Remainder = ViewSettings.ColumnCount%ColumnsInGlobal;
    GlobalColumns = ViewSettings.ColumnCount/ColumnsInGlobal;

    if ( !Remainder )
    {
      for (int k = 0; k < GlobalColumns-1; k++)
      {
         for (int j = 0; j < ColumnsInGlobal; j++)
         {
           if ( (ViewSettings.ColumnType[k*ColumnsInGlobal+j] & 0xFF) !=
              (ViewSettings.ColumnType[(k+1)*ColumnsInGlobal+j] & 0xFF) )
           UnEqual = true;
         }
      }

      if ( !UnEqual )
        break;
    }

    ColumnsInGlobal++;
  }

  return(GlobalColumns);
}


extern void GetColor(int PaletteIndex);

void FileList::ShowList(int ShowStatus,int StartColumn)
{
  string strDateStr, strTimeStr;

  int StatusShown=FALSE;
  int MaxLeftPos=0,MinLeftPos=FALSE;
  int ColumnCount=ShowStatus ? ViewSettings.StatusColumnCount:ViewSettings.ColumnCount;
  for (int I=Y1+1+Opt.ShowColumnTitles,J=CurTopFile;I<Y2-2*Opt.ShowPanelStatus;I++,J++)
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

    for (int K=0;K<ColumnCount;K++)
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
          BoxText(CurX-1==X2 ? BoxSymbols[BS_V2]:L' ');
        }
        continue;
      }
      if (ListPos<FileCount)
      {
        if (!ShowStatus && !StatusShown && CurFile==ListPos && Opt.ShowPanelStatus)
        {
          ShowList(TRUE,CurColumn);
          GotoXY(CurX,CurY);
          StatusShown=TRUE;
          SetShowColor(ListPos);
        }
        if ( !ShowStatus )
           SetShowColor(ListPos);

        if (ColumnType>=CUSTOM_COLUMN0 && ColumnType<=CUSTOM_COLUMN9)
        {
          int ColumnNumber=ColumnType-CUSTOM_COLUMN0;
          const wchar_t *ColumnData=NULL;
					if (ColumnNumber<ListData[ListPos]->CustomColumnNumber)
						ColumnData=ListData[ListPos]->CustomColumnData[ColumnNumber];
          if (ColumnData==NULL)
            ColumnData=L"";
          int CurLeftPos=0;
          if (!ShowStatus && LeftPos>0)
          {
            int Length=StrLength(ColumnData);
            if (Length>ColumnWidth)
            {
              CurLeftPos=LeftPos;
              if (CurLeftPos>Length-ColumnWidth)
                CurLeftPos=Length-ColumnWidth;
              if (CurLeftPos>MaxLeftPos)
                MaxLeftPos=CurLeftPos;
            }
          }
          mprintf(L"%-*.*s",ColumnWidth,ColumnWidth,ColumnData+CurLeftPos);
        }
        else
        {
          switch(ColumnType)
          {
            case NAME_COLUMN:
              {
                int Width=ColumnWidth;
                int ViewFlags=ColumnTypes[K];
                if ((ViewFlags & COLUMN_MARK) && Width>2)
                {
									Text(ListData[ListPos]->Selected?L"\x221A ":L"  ");
                  Width-=2;
                }
								if (ListData[ListPos]->Colors.MarkChar && Opt.Highlight && Width>1)
                {
                  Width--;
									OutCharacter[0]=(wchar_t)ListData[ListPos]->Colors.MarkChar;
                  int OldColor=GetColor();
                  if (!ShowStatus)
                    SetShowColor(ListPos,HIGHLIGHTCOLORTYPE_MARKCHAR);
                  Text(OutCharacter);
                  SetColor(OldColor);
                }

								const wchar_t *NamePtr = ShowShortNames && !ListData[ListPos]->strShortName.IsEmpty () && !ShowStatus ? ListData[ListPos]->strShortName:ListData[ListPos]->strName;
                const wchar_t *NameCopy = NamePtr;

                if (ViewFlags & COLUMN_NAMEONLY)
                {
                    //BUGBUG!!!
                  // !!! НЕ УВЕРЕН, но то, что отображается пустое
                  // пространство вместо названия - бага
                  NamePtr=PointToFolderNameIfFolder(NamePtr);
                }

                int CurLeftPos=0;
                int RightAlign=(ViewFlags & COLUMN_RIGHTALIGN);
                int LeftBracket=FALSE,RightBracket=FALSE;
                if (!ShowStatus && LeftPos!=0)
                {
                  int Length = (int)wcslen (NamePtr);
                  if (Length>Width)
                  {
                    if (LeftPos>0)
                    {
                      if (!RightAlign)
                      {
                        CurLeftPos=LeftPos;
                        if (Length-CurLeftPos<Width)
                          CurLeftPos=Length-Width;

                        NamePtr += CurLeftPos;

                        if (CurLeftPos>MaxLeftPos)
                          MaxLeftPos=CurLeftPos;
                      }
                    }
                    else
                      if (RightAlign)
                      {
                        int CurRightPos=LeftPos;
                        if (Length+CurRightPos<Width)
                          CurRightPos=Width-Length;
                        else
                          RightBracket=TRUE;

                        NamePtr += Length+CurRightPos-Width;

                        RightAlign=FALSE;
                        if (CurRightPos<MinLeftPos)
                          MinLeftPos=CurRightPos;
                      }
                  }
                }

                string strName;

								int TooLong=ConvertName(NamePtr, strName, Width, RightAlign,ShowStatus,ListData[ListPos]->FileAttr);

                if (CurLeftPos!=0)
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
                  if (!ShowShortNames && ViewSettings.FileUpperToLowerCase)
										if (!(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY) && !IsCaseMixed(NameCopy))
                      strName.Lower ();
									if ((ShowShortNames || ViewSettings.FolderUpperCase) && (ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
                    strName.Upper ();
									if ((ShowShortNames || ViewSettings.FileLowerCase) && (ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
                    strName.Lower ();
                }
                Text(strName);
                int NameX=WhereX();

                if ( !ShowStatus )
                {
                  if (LeftBracket)
                  {
                    GotoXY(CurX-1,CurY);

                    if ( Level == 1 )
                      SetColor (COL_PANELBOX);

                    Text(openBracket);

                    SetShowColor(J);
                  }

                  if (RightBracket)
                  {
                    if ( Level == ColumnsInGlobal )
                      SetColor(COL_PANELBOX);

                    GotoXY(NameX,CurY);
                    Text(closeBracket);
                    ShowDivider=FALSE;

                    if ( Level == ColumnsInGlobal )
                      SetColor(COL_PANELTEXT);
                    else
                      SetShowColor(J);
                  }
                }
              }
              break;
            case SIZE_COLUMN:
            case PACKED_COLUMN:
						case STREAMSSIZE_COLUMN:
              {
								bool Packed=(ColumnType==PACKED_COLUMN);
								bool Streams=(ColumnType==STREAMSSIZE_COLUMN);
                string strStr;
                int Width=ColumnWidth;
								if (ListData[ListPos]->ShowFolderSize==2)
                {
                  Width--;
                  Text(L"~");
                }
								if (!Streams && !Packed && (ListData[ListPos]->FileAttr & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)) && !ListData[ListPos]->ShowFolderSize)
                {
                  const wchar_t *PtrName=MSG(MListFolder);
									if(TestParentFolderName(ListData[ListPos]->strName))
                    PtrName=MSG(MListUp);
                  else
                  {
                    if(ListData[ListPos]->FileAttr&FILE_ATTRIBUTE_REPARSE_POINT)
                    {
                      switch(ListData[ListPos]->ReparseTag)
                      {
                      case IO_REPARSE_TAG_SYMLINK:
                        PtrName=MSG(MListSymLink);
                        break;
                      case IO_REPARSE_TAG_MOUNT_POINT:
                        PtrName=MSG(MListJunction);
                        break;
                      }
                    }
                  }
                  if (StrLength(PtrName) <= Width-2 )
                    strStr.Format (L"<%s>", PtrName);
                  else
                    strStr = PtrName;

                  mprintf(L"%*.*s",Width,Width, (const wchar_t*)strStr);
                }
                else
                {
                  string strOutStr;
                  // подсократим - весь код по форматированию размера
                  //   в отдельную функцию - FileSizeToStr().
                  mprintf(L"%s",FileSizeToStr(strOutStr,
										Packed?ListData[ListPos]->PackSize:Streams?ListData[ListPos]->StreamsSize:ListData[ListPos]->UnpSize,
                           Width,ColumnTypes[K]).CPtr());
                }
              }
              break;
            case DATE_COLUMN:
              {
                string strOutStr;
								ConvertDate(ListData[ListPos]->WriteTime,strDateStr,strTimeStr,0,FALSE,FALSE,ColumnWidth>9);
								mprintf(L"%*.*s",ColumnWidth,ColumnWidth,strDateStr.CPtr());
              }
              break;
            case TIME_COLUMN:
              {
                string strOutStr;
								ConvertDate(ListData[ListPos]->WriteTime,strDateStr,strTimeStr,ColumnWidth);
								mprintf(L"%*.*s",ColumnWidth,ColumnWidth,strTimeStr.CPtr());
              }
              break;
            case MDATE_COLUMN:
            case CDATE_COLUMN:
            case ADATE_COLUMN:
              {
                FILETIME *FileTime;
                switch(ColumnType)
                {
                  case CDATE_COLUMN:
										FileTime=&ListData[ListPos]->CreationTime;
                    break;
                  case ADATE_COLUMN:
										FileTime=&ListData[ListPos]->AccessTime;
                    break;
                  default: //case MDATE_COLUMN:
										FileTime=&ListData[ListPos]->WriteTime;
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
								string strDateStr, strTimeStr;
                ConvertDate(*FileTime,strDateStr,strTimeStr,ColumnWidth-9,Brief,TextMonth,FullYear);
								string strOutStr=strDateStr+L" "+strTimeStr;
								FS<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<strOutStr;
              }
              break;
            case ATTR_COLUMN:
              {
                wchar_t OutStr[]=
								{
									(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_READONLY)?L'R':L' ',
									(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_SYSTEM)?L'S':L' ',
									(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_HIDDEN)?L'H':L' ',
									(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_ARCHIVE)?L'A':L' ',
									(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_REPARSE_POINT)?L'L':((ListData[ListPos]->FileAttr&FILE_ATTRIBUTE_SPARSE_FILE)?L'$':L' '),
									// $ 20.10.2000 SVS - Encrypted NTFS - Поток может быть либо COMPRESSED (С) либо ENCRYPTED (E)
									(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_COMPRESSED)?L'C':((ListData[ListPos]->FileAttr&FILE_ATTRIBUTE_ENCRYPTED)?L'E':L' '),
									(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_TEMPORARY)?L'T':L' ',
									(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)?L'I':L' ',
									(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_OFFLINE)?L'O':L' ',
									(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_VIRTUAL)?L'V':L' ',
									L'\0',
								};
								FS<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<OutStr;
              }
              break;
            case DIZ_COLUMN:
              {
                int CurLeftPos=0;
                if (!ShowStatus && LeftPos>0)
                {
                  int Length=ListData[ListPos]->DizText ? StrLength(ListData[ListPos]->DizText):0;
                  if (Length>ColumnWidth)
                  {
                    CurLeftPos=LeftPos;
                    if (CurLeftPos>Length-ColumnWidth)
                      CurLeftPos=Length-ColumnWidth;
                    if (CurLeftPos>MaxLeftPos)
                      MaxLeftPos=CurLeftPos;
                  }
                }
								string strDizText=ListData[ListPos]->DizText ? ListData[ListPos]->DizText+CurLeftPos:L"";

                size_t pos;
                if (strDizText.Pos(pos,L'\4'))
                  strDizText.SetLength(pos);

								FS<<fmt::LeftAlign()<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<strDizText;

              }
              break;
            case OWNER_COLUMN:
              {
								const wchar_t* Owner=ListData[ListPos]->strOwner;
                if (Owner && !(ColumnTypes[K]&COLUMN_FULLOWNER) && PanelMode!=PLUGIN_PANEL)
                {
									const wchar_t* SlashPos=FirstSlash(Owner);
                  if(SlashPos)
										Owner=SlashPos+1;
                }
                int CurLeftPos=0;
                if (!ShowStatus && LeftPos>0)
                {
                  int Length=StrLength(Owner);

                  if (Length>ColumnWidth)
                  {
                    CurLeftPos=LeftPos;
                    if (CurLeftPos>Length-ColumnWidth)
                      CurLeftPos=Length-ColumnWidth;
                    if (CurLeftPos>MaxLeftPos)
                      MaxLeftPos=CurLeftPos;
                  }
                }
								FS<<fmt::LeftAlign()<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<Owner+CurLeftPos;
              }
              break;
            case NUMLINK_COLUMN:
              {
								FS<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<ListData[ListPos]->NumberOfLinks;
              }
              break;
						case NUMSTREAMS_COLUMN:
							{
								FS<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<ListData[ListPos]->NumberOfStreams;
							}
							break;
          }
        }
      }
      else
        mprintf(L"%*s",ColumnWidth,L"");

      if (ShowDivider==FALSE)
        GotoXY(CurX+ColumnWidth+1,CurY);
      else
      {
        if ( !ShowStatus )
        {
          SetShowColor (ListPos);

          if ( Level == ColumnsInGlobal )
            SetColor (COL_PANELBOX);
        }

        if ( K == ColumnCount-1 )
          SetColor(COL_PANELBOX);

        GotoXY(CurX+ColumnWidth,CurY);

        if (K==ColumnCount-1)
          BoxText(CurX+ColumnWidth==X2 ? BoxSymbols[BS_V2]:L' ');
        else
          BoxText(ShowStatus ? L' ':BoxSymbols[BS_V1]);

        if ( !ShowStatus )
          SetColor (COL_PANELTEXT);
      }

      if(!ShowStatus)
      {
        if ( Level == ColumnsInGlobal )
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
      mprintf(L"%*s",X2-WhereX(),L"");
    }
  }
  if (!ShowStatus && !StatusShown && Opt.ShowPanelStatus)
  {
    SetScreen(X1+1,Y2-1,X2-1,Y2-1,L' ',COL_PANELTEXT);
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
  for (I=0;I<ViewSettings.ColumnCount;I++)
    if ((int)(ViewSettings.ColumnType[I] & 0xff)==Type)
      return(TRUE);
  for (I=0;I<ViewSettings.StatusColumnCount;I++)
    if ((int)(ViewSettings.StatusColumnType[I] & 0xff)==Type)
      return(TRUE);
  return(FALSE);
}


int FileList::IsCaseSensitive()
{
  return(ViewSettings.CaseSensitiveSort);
}
