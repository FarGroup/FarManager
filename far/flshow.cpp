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
#include "filter.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"

extern struct PanelViewSettings ViewSettingsArray[];
extern int ColumnTypeWidth[];

//static char VerticalLine[2][2]={{0x0B3,0x00},{0x0BA,0x00}};
static BYTE VerticalLineEx[2]={0x0B3,0x0BA};
static wchar_t OutCharacter[2]={0,0};

static int __FormatEndSelectedPhrase(int Count)
{
  int M_Fmt=MListFileSize;
  if(Count != 1)
  {
    char StrItems[32];
    itoa(Count,StrItems,10);
    int LenItems=strlen(StrItems);
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
  if ( Locked() )
  {
    CorrectPosition();
    return;
  }
  string strTitle;
  int Length;
  struct OpenPluginInfoW Info;

  if (PanelMode==PLUGIN_PANEL)
  {
    if (ProcessPluginEvent(FE_REDRAW,NULL))
      return;
    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
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
      }
      if(IDMessage != -1)
        strTitle=UMSG(IDMessage);

      if (PanelMode==PLUGIN_PANEL && Info.PanelModesArray!=NULL &&
          ViewMode<Info.PanelModesNumber &&
          Info.PanelModesArray[ViewMode].ColumnTitles!=NULL)
      {
        wchar_t *NewTitle=Info.PanelModesArray[ViewMode].ColumnTitles[I];
        if (NewTitle!=NULL)
          strTitle=NewTitle;
      }
      string strTitleMsg;
      CenterStrW(strTitle,strTitleMsg,ViewSettings.ColumnWidth[I]);
      SetColor(COL_PANELCOLUMNTITLE);
      GotoXY(ColumnPos,Y1+1);
      mprintfW(L"%.*s",ViewSettings.ColumnWidth[I],(const wchar_t*)strTitleMsg);
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
      BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[VerticalLineEx[0]-0x0B0]:VerticalLineEx[0]));
    }
    if (!Opt.ShowPanelStatus)
    {
      GotoXY(ColumnPos,Y2);
      BoxText(Opt.UseUnicodeConsole?BoxSymbols[0xCF-0x0B0]:0xCF);
    }
    ColumnPos++;
  }

  if (Opt.ShowSortMode)
  {
    static int SortModes[]={UNSORTED,BY_NAME,BY_EXT,BY_MTIME,BY_CTIME,
                            BY_ATIME,BY_SIZE,BY_DIZ,BY_OWNER,
                            BY_COMPRESSEDSIZE,BY_NUMLINKS};
    static int SortStrings[]={MMenuUnsorted,MMenuSortByName,
      MMenuSortByExt,MMenuSortByModification,MMenuSortByCreation,
      MMenuSortByAccess,MMenuSortBySize,MMenuSortByDiz,MMenuSortByOwner,
      MMenuSortByCompressedSize,MMenuSortByNumLinks};
    for (int I=0;I<sizeof(SortModes)/sizeof(SortModes[0]);I++)
    {
      if (SortModes[I]==SortMode)
      {
        const wchar_t *SortStr=UMSG(SortStrings[I]);
        wchar_t *Ch=wcschr(SortStr,L'&');
        if (Ch!=NULL)
        {
          if (Opt.ShowColumnTitles)
            GotoXY(X1+1,Y1+1);
          else
            GotoXY(X1+1,Y1);
          SetColor(COL_PANELCOLUMNTITLE);
          OutCharacter[0]=SortOrder==1 ? LocalLowerW(Ch[1]):LocalUpperW(Ch[1]);
          TextW(OutCharacter);
          if (Filter!=NULL && Filter->IsEnabled())
          {
            OutCharacter[0]=L'*';
            TextW(OutCharacter);
          }
        }
        break;
      }
    }
  }

  if(SelectedFirst)
  {
    OutCharacter[0]=L'^';
    TextW(OutCharacter);
  }

  if (!Fast && GetFocus())
  {
    if ( PanelMode==PLUGIN_PANEL )
        CtrlObject->CmdLine->SetCurDirW(Info.CurDir);
    else
        CtrlObject->CmdLine->SetCurDirW(strCurDir);

    CtrlObject->CmdLine->Show();
  }
  int TitleX2=Opt.Clock && !Opt.ShowMenuBar ? Min(ScrX-4,X2):X2;
  int TruncSize=TitleX2-X1-3;
  if (!Opt.ShowColumnTitles && Opt.ShowSortMode && Filter!=NULL && Filter->IsEnabled())
    TruncSize-=2;

  GetTitle(strTitle,TruncSize,2);//,(PanelMode==PLUGIN_PANEL?0:2));
  Length=strTitle.GetLength();
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
  TextW(strTitle);
  if (FileCount==0)
  {
    SetScreen(X1+1,Y2-1,X2-1,Y2-1,L' ',COL_PANELTEXT);
    SetColor(COL_PANELTEXT); //???
    //GotoXY(X1+1,Y2-1);
    //mprintf("%*s",X2-X1-1,"");
  }
  if (PanelMode==PLUGIN_PANEL && FileCount>0 && (Info.Flags & OPIF_REALNAMES))
  {
    struct FileListItem *CurPtr=ListData[CurFile];
    if (!TestParentFolderNameW(CurPtr->strName))
    {
      strCurDir = CurPtr->strName;

      wchar_t *NamePtr = strCurDir.GetBuffer ();

      NamePtr = wcsrchr(NamePtr,L'\\');

      if (NamePtr!=NULL && NamePtr!=strCurDir) //BUGBUG, bad
        if (*(NamePtr-1)!=L':')
          *NamePtr=0;
        else
          *(NamePtr+1)=0;

      strCurDir.ReleaseBuffer ();

      if (GetFocus())
      {
        CtrlObject->CmdLine->SetCurDirW(strCurDir);
        CtrlObject->CmdLine->Show();
      }
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


int FileList::GetShowColor(int Position)
{
  DWORD ColorAttr=COL_PANELTEXT;

  if (ListData && Position < FileCount)
  {
    struct FileListItem *CurPtr=ListData[Position];

    if (CurFile==Position && Focus && FileCount > 0)
    {
      if (CurPtr->Selected)
      {
        if (CurPtr->Colors.CursorSelColor && Opt.Highlight)
          ColorAttr=CurPtr->Colors.CursorSelColor;
        else
          ColorAttr=COL_PANELSELECTEDCURSOR;
      }
      else
      {
        if (CurPtr->Colors.CursorColor && Opt.Highlight)
          ColorAttr=CurPtr->Colors.CursorColor;
        else
          ColorAttr=COL_PANELCURSOR;
      }
    }
    else
    {
      if (CurPtr->Selected)
      {
        if (CurPtr->Colors.SelColor && Opt.Highlight)
          ColorAttr=CurPtr->Colors.SelColor;
        else
          ColorAttr=COL_PANELSELECTEDTEXT;
      }
      else
      {
        if (CurPtr->Colors.Color && Opt.Highlight)
          ColorAttr=CurPtr->Colors.Color;
      }
    }
  }

  return ColorAttr;
}


void FileList::SetShowColor (int Position)
{
        SetColor (GetShowColor(Position));
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
    InsertCommasW(SelFileSize,strFormStr);
    strSelStr.Format (UMSG(__FormatEndSelectedPhrase(SelFileCount)),(const wchar_t*)strFormStr,SelFileCount);

    TruncStrW(strSelStr,X2-X1-1);
    Length=strSelStr.GetLength();
    SetColor(COL_PANELSELECTEDINFO);
    GotoXY(X1+(X2-X1+1-Length)/2,Y2-2*Opt.ShowPanelStatus);
    TextW(strSelStr);
  }
  else
    if (!RegVer)
    {
      string strEvalStr;
      const wchar_t *EvalMsg=UMSG(MListEval);
      if (*EvalMsg==0)
        strEvalStr = L" Evaluation version ";
      else
        strEvalStr.Format (L" %s ",UMSG(MListEval));
      TruncStrW(strEvalStr,X2-X1-1);
      Length=strEvalStr.GetLength();
      SetColor(COL_PANELTEXT);
      GotoXY(X1+(X2-X1+1-Length)/2,Y2-2*Opt.ShowPanelStatus);
      TextW(strEvalStr);
    }
}


void FileList::ShowTotalSize(struct OpenPluginInfoW &Info)
{
  if (!Opt.ShowPanelTotals && PanelMode==PLUGIN_PANEL && (Info.Flags & OPIF_REALNAMES)==0)
    return;
  string strTotalStr, strFormSize, strFreeSize;
  int Length;

  InsertCommasW(TotalFileSize,strFormSize);
  if (Opt.ShowPanelFree && (PanelMode!=PLUGIN_PANEL || (Info.Flags & OPIF_REALNAMES)))
    InsertCommasW(FreeDiskSize,strFreeSize);
  if (Opt.ShowPanelTotals)
  {
    if (!Opt.ShowPanelFree || strFreeSize.IsEmpty())
      strTotalStr.Format (UMSG(__FormatEndSelectedPhrase(TotalFileCount)),(const wchar_t*)strFormSize,TotalFileCount);
    else
    {
// UNICODE!!!
      static wchar_t DHLine[4]={0x0CD,0x0CD,0x0CD,0x00};
      strTotalStr.Format (L" %s (%d) %s %s ",(const wchar_t*)strFormSize,TotalFileCount,DHLine,(const wchar_t*)strFreeSize);
      if ((int)strTotalStr.GetLength()> X2-X1-1)
      {
        InsertCommasW(FreeDiskSize>>20,strFreeSize);
        InsertCommasW(TotalFileSize>>20,strFormSize);
        strTotalStr.Format (L" %s %s (%d) %s %s %s ",(const wchar_t*)strFormSize,UMSG(MListMb),TotalFileCount,DHLine,(const wchar_t*)strFreeSize,UMSG(MListMb));
      }
    }
  }
  else
    strTotalStr.Format (UMSG(MListFreeSize), !strFreeSize.IsEmpty() ? (const wchar_t*)strFreeSize:L"???");

  SetColor(COL_PANELTOTALINFO);
  /* $ 01.08.2001 VVM
    + Обрезаем строчку справа, а не слева */
  TruncStrFromEndW(strTotalStr, X2-X1-1);
  /* VVM $ */
  Length=strTotalStr.GetLength();
  GotoXY(X1+(X2-X1+1-Length)/2,Y2);

// UNICODE!!!
  wchar_t *FirstBox=wcschr(strTotalStr,0x0CD);
  int BoxPos=(FirstBox==NULL) ? -1:FirstBox-(const wchar_t*)strTotalStr;
  int BoxLength=0;
  if (BoxPos!=-1)
// UNICODE!!!
    for (int I=0;strTotalStr.At(BoxPos+I)==0x0CD;I++) //BUGBUG
      BoxLength++;

  if (BoxPos==-1 || BoxLength==0)
    TextW(strTotalStr);
  else
  {
    mprintfW(L"%.*s",BoxPos,(const wchar_t*)strTotalStr);
    SetColor(COL_PANELBOX);
    mprintfW(L"%.*s",BoxLength,(const wchar_t*)strTotalStr+BoxPos);
    SetColor(COL_PANELTOTALINFO);
    TextW((const wchar_t*)strTotalStr+BoxPos+BoxLength);
  }
}

int FileList::ConvertNameW(const wchar_t *SrcName,string &strDest,int MaxLength,int RightAlign,int ShowStatus,DWORD FileAttr)
{
  wchar_t *lpwszDest = strDest.GetBuffer (MaxLength+1);

  wmemset(lpwszDest,L' ',MaxLength);

  int SrcLength=wcslen(SrcName);
  if (RightAlign && SrcLength>MaxLength)
  {
    wmemcpy(lpwszDest,SrcName+SrcLength-MaxLength,MaxLength);
    lpwszDest[MaxLength]=0;

    strDest.ReleaseBuffer ();
    return(TRUE);
  }
  const wchar_t *DotPtr;
  if (!ShowStatus &&
      (!(FileAttr&FA_DIREC) && ViewSettings.AlignExtensions || (FileAttr&FA_DIREC) && ViewSettings.FolderAlignExtensions)
      && SrcLength<=MaxLength &&
      (DotPtr=wcsrchr(SrcName,L'.'))!=NULL && DotPtr!=SrcName &&
      (SrcName[0]!=L'.' || SrcName[2]!=0) && wcschr(DotPtr+1,L' ')==NULL)
  {
    int DotLength=wcslen(DotPtr+1);
    int NameLength=DotPtr-SrcName;
    int DotPos=MaxLength-Max(DotLength,3);
    if (DotPos<=NameLength)
      DotPos=NameLength+1;
    if (DotPos>0 && NameLength>0 && SrcName[NameLength-1]==' ')
      lpwszDest[NameLength]=L'.';
    wmemcpy(lpwszDest,SrcName,NameLength);
    wmemcpy(lpwszDest+DotPos,DotPtr+1,DotLength);
  }
  else
    wmemcpy(lpwszDest,SrcName,SrcLength);
  lpwszDest[MaxLength]=0;

  strDest.ReleaseBuffer ();

  return(SrcLength>MaxLength);
}


void FileList::PrepareViewSettings(int ViewMode,struct OpenPluginInfoW *PlugInfo)
{
  struct OpenPluginInfoW Info;
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

  int GlobalColumns;
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
          BoxText((WORD)(CurX-1==X2 ? (Opt.UseUnicodeConsole?BoxSymbols[VerticalLineEx[1]-0x0B0]:VerticalLineEx[1]):0x20));
        }
        continue;
      }
      if (ListPos<FileCount)
      {
        struct FileListItem *CurPtr=ListData[ListPos];

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
          wchar_t *ColumnData=NULL;
          if (ColumnNumber<CurPtr->CustomColumnNumber)
            ColumnData=CurPtr->CustomColumnData[ColumnNumber];
          if (ColumnData==NULL)
            ColumnData=L"";
          int CurLeftPos=0;
          if (!ShowStatus && LeftPos>0)
          {
            int Length=wcslen(ColumnData);
            if (Length>ColumnWidth)
            {
              CurLeftPos=LeftPos;
              if (CurLeftPos>Length-ColumnWidth)
                CurLeftPos=Length-ColumnWidth;
              if (CurLeftPos>MaxLeftPos)
                MaxLeftPos=CurLeftPos;
            }
          }
          mprintfW(L"%-*.*s",ColumnWidth,ColumnWidth,ColumnData+CurLeftPos);
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
                  static wchar_t SelectedChar[4]={0x0FB,0x20,0x00,0x00};
                  TextW(CurPtr->Selected ? SelectedChar:L"  ");
                  Width-=2;
                }
                if (CurPtr->Colors.MarkChar && Opt.Highlight && Width>1)
                {
                  Width--;
                  OutCharacter[0]=CurPtr->Colors.MarkChar;
                  TextW(OutCharacter);
                }

                const wchar_t *NamePtr = ShowShortNames && !CurPtr->strShortName.IsEmpty () && !ShowStatus ? CurPtr->strShortName:CurPtr->strName;
                const wchar_t *NameCopy = NamePtr;

                if (ViewFlags & COLUMN_NAMEONLY)
                {
                    //BUGBUG!!!
                  // !!! НЕ УВЕРЕН, но то, что отображается пустое
                  // пространство вместо названия - бага
                  NamePtr=PointToFolderNameIfFolderW (NamePtr);
                }

                int CurLeftPos=0;
                int RightAlign=(ViewFlags & COLUMN_RIGHTALIGN);
                int LeftBracket=FALSE,RightBracket=FALSE;
                if (!ShowStatus && LeftPos!=0)
                {
                  int Length = wcslen (NamePtr);
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

                int TooLong=ConvertNameW(NamePtr, strName, Width, RightAlign,ShowStatus,CurPtr->FileAttr);

                if (CurLeftPos!=0)
                  LeftBracket=TRUE;
                if (TooLong)
                {
                  if (RightAlign)
                    LeftBracket=TRUE;
                  if (!RightAlign && wcslen(NamePtr)>static_cast<size_t>(Width))
                    RightBracket=TRUE;
                }

                if (!ShowStatus)
                {
                  if (!ShowShortNames && ViewSettings.FileUpperToLowerCase)
                    if (!(CurPtr->FileAttr & FA_DIREC) && !IsCaseMixedW(NameCopy))
                      strName.Lower ();
                  if ((ShowShortNames || ViewSettings.FolderUpperCase) && (CurPtr->FileAttr & FA_DIREC))
                    strName.Upper ();
                  if ((ShowShortNames || ViewSettings.FileLowerCase) && (CurPtr->FileAttr & FA_DIREC)==0)
                    strName.Lower ();
                }
                TextW (strName);
                int NameX=WhereX();

                if ( !ShowStatus )
                {
                  if (LeftBracket)
                  {
                    GotoXY(CurX-1,CurY);

                    if ( Level == 1 )
                      SetColor (COL_PANELBOX);

                    TextW(openBracket);

                    SetShowColor(J);
                  }

                  if (RightBracket)
                  {
                    if ( Level == ColumnsInGlobal )
                      SetColor(COL_PANELBOX);

                    GotoXY(NameX,CurY);
                    TextW(closeBracket);
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
              {
                int Packed=(ColumnType==PACKED_COLUMN);
                string strStr;
                int Width=ColumnWidth;
                if (CurPtr->ShowFolderSize==2)
                {
                  Width--;
                  TextW(L"~");
                }
                if (!Packed && (CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY) && !CurPtr->ShowFolderSize)
                {
                  wchar_t *PtrName;
                  if (TestParentFolderNameW(CurPtr->strName))
                    PtrName=UMSG(MListUp);
                  else
                    PtrName=UMSG(CurPtr->FileAttr&FILE_ATTRIBUTE_REPARSE_POINT?MListSymLink:MListFolder);

                  if (wcslen(PtrName) <= static_cast<size_t>(Width-2))
                    strStr.Format (L"<%s>", PtrName);
                  else
                    strStr = PtrName;

                  mprintfW(L"%*.*s",Width,Width, (const wchar_t*)strStr);
                }
                else
                {
                  string strOutStr;
                  // подсократим - весь код по форматированию размера
                  //   в отдельную функцию - FileSizeToStr().
                  mprintfW(L"%s", (const wchar_t*)FileSizeToStrW(strOutStr,
                           Packed?CurPtr->PackSize:CurPtr->UnpSize,
                           Width,ColumnTypes[K]));
                }
              }
              break;
            case DATE_COLUMN:
              {
                string strOutStr;
                ConvertDateW(CurPtr->WriteTime,strDateStr,strTimeStr,0,FALSE,FALSE,ColumnWidth>9);
                mprintfW(L"%*.*s",ColumnWidth,ColumnWidth,(const wchar_t*)strDateStr);
              }
              break;
            case TIME_COLUMN:
              {
                string strOutStr;
                ConvertDateW(CurPtr->WriteTime,strDateStr,strTimeStr,ColumnWidth);
                mprintfW(L"%*.*s",ColumnWidth,ColumnWidth,(const wchar_t*)strTimeStr);
              }
              break;
            case MDATE_COLUMN:
            case CDATE_COLUMN:
            case ADATE_COLUMN:
              {
                FILETIME *FileTime;
                switch(ColumnType)
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
                string strDateStr, strTimeStr, strOutStr;
                ConvertDateW(*FileTime,strDateStr,strTimeStr,ColumnWidth-9,Brief,TextMonth,FullYear);
                strOutStr.Format (L"%s %s", (const wchar_t*)strDateStr, (const wchar_t*)strTimeStr);
                mprintfW(L"%*.*s",ColumnWidth,ColumnWidth,(const wchar_t*)strOutStr);
              }
              break;
            case ATTR_COLUMN:
              {
                wchar_t OutStr[16];
                DWORD FileAttr=CurPtr->FileAttr;
                OutStr[0]=(FileAttr & FILE_ATTRIBUTE_READONLY) ? L'R':L' ';
                OutStr[1]=(FileAttr & FILE_ATTRIBUTE_SYSTEM) ? L'S':L' ';
                OutStr[2]=(FileAttr & FILE_ATTRIBUTE_HIDDEN) ? L'H':L' ';
                OutStr[3]=(FileAttr & FILE_ATTRIBUTE_ARCHIVE) ? L'A':L' ';
                OutStr[4]=(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) ? L'L' : ((FileAttr & FILE_ATTRIBUTE_SPARSE_FILE) ? L'$':L' ');
                // $ 20.10.2000 SVS - Encrypted NTFS/Win2K - Поток может быть либо COMPRESSED (С) либо ENCRYPTED (E)
                OutStr[5]=(FileAttr & FILE_ATTRIBUTE_COMPRESSED) ? L'C':((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)?L'E':L' ');
                OutStr[6]=(FileAttr & FILE_ATTRIBUTE_TEMPORARY) ? L'T':L' ';
                OutStr[7]=(FileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) ? L'I':L' ';
                OutStr[8]=0;
                wchar_t *OutPtr=OutStr;
                mprintfW(L"%*.*s",ColumnWidth,ColumnWidth,OutPtr);
              }
              break;
            case DIZ_COLUMN:
              {
                int CurLeftPos=0;
                if (!ShowStatus && LeftPos>0)
                {
                  int Length=CurPtr->DizText ? wcslen(CurPtr->DizText):0;
                  if (Length>ColumnWidth)
                  {
                    CurLeftPos=LeftPos;
                    if (CurLeftPos>Length-ColumnWidth)
                      CurLeftPos=Length-ColumnWidth;
                    if (CurLeftPos>MaxLeftPos)
                      MaxLeftPos=CurLeftPos;
                  }
                }
                string strDizText;
                strDizText = CurPtr->DizText ? CurPtr->DizText+CurLeftPos:L"";

                wchar_t *DizEnd=strDizText.GetBuffer ();

                DizEnd = wcschr(DizEnd,L'\4');
                if (DizEnd!=NULL)
                  *DizEnd=0;

                strDizText.ReleaseBuffer();

                mprintfW(L"%-*.*s",ColumnWidth,ColumnWidth,(const wchar_t*)strDizText);
              }
              break;
            case OWNER_COLUMN:
              {
                int CurLeftPos=0;
                if (!ShowStatus && LeftPos>0)
                {
                  int Length=CurPtr->strOwner.GetLength();
                  if (Length>ColumnWidth)
                  {
                    CurLeftPos=LeftPos;
                    if (CurLeftPos>Length-ColumnWidth)
                      CurLeftPos=Length-ColumnWidth;
                    if (CurLeftPos>MaxLeftPos)
                      MaxLeftPos=CurLeftPos;
                  }
                }
                mprintfW(L"%-*.*s",ColumnWidth,ColumnWidth,(const wchar_t*)CurPtr->strOwner+CurLeftPos);
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
        mprintfW(L"%*s",ColumnWidth,L"");

      if (ShowDivider==FALSE)
        GotoXY(CurX+ColumnWidth+1,CurY);
      else
      {
        if ( !ShowStatus )
        {
          SetColor (GetShowColor (ListPos));

          if ( Level == ColumnsInGlobal )
             SetColor (COL_PANELBOX);
                }

                if ( K == ColumnCount-1 )
          SetColor(COL_PANELBOX);

        GotoXY(CurX+ColumnWidth,CurY);

        if (K==ColumnCount-1)
          BoxText((WORD)(CurX+ColumnWidth==X2 ? (Opt.UseUnicodeConsole?BoxSymbols[VerticalLineEx[1]-0x0B0]:VerticalLineEx[1]):0x20));
        else
          BoxText((WORD)(ShowStatus ? 0x20:(Opt.UseUnicodeConsole?BoxSymbols[VerticalLineEx[0]-0x0B0]:VerticalLineEx[0])));

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
      mprintfW(L"%*s",X2-WhereX(),L"");
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
    if ((ViewSettings.ColumnType[I] & 0xff)==Type)
      return(TRUE);
  for (I=0;I<ViewSettings.StatusColumnCount;I++)
    if ((ViewSettings.StatusColumnType[I] & 0xff)==Type)
      return(TRUE);
  return(FALSE);
}


int FileList::IsCaseSensitive()
{
  return(ViewSettings.CaseSensitiveSort);
}
