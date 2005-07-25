/*
flshow.cpp

Файловая панель - вывод на экран

*/

/* Revision: 1.47 25.07.2005 $ */

/*
Modify:
  24.07.2005 WARP
    ! see 02033.LockUnlock.txt
  13.07.2005 SVS
    - BugZ#1253 - некорректная обработка PanelMode.FullScreen
      не учитывался факт того, что размеры могли измениться
  05.01.2005 WARP
    - Немного зажевало раскраску скобок { и } при длинном имени.
  18.12.2004 WARP
    - PanelMode.CaseConversion игнорирует тип колонки "NO" (Bugz#1139)
  08.11.2004 WARP
    ! Исправления в раксраске и работе панелей
  01.11.2004 SVS
    - Неотрисовка статуса
    - Кривизна с выбором цвета глобальной колонки
  01.11.2004 SVS & WARP ItSelf
    ! Новый способ разбиения на глобальные колонки
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  29.05.2004 SVS
    - BugZ#847 - рамка слева от колонки с именем файла не подсвечивается
      (вроде артефактов не замечено :-))
  18.05.2004 SVS
    ! IsNumeric() переехала из flshow.cpp в panel.hpp
  19.02.2004 SVS
    - неправильные функции использовались при отрисовке панелей.
  15.01.2004 SVS
    - BugZ#997 - падение FAR при большом выставлении высоты буфера экрана
  15.09.2003 SVS
    ! Корректировка на часы (X2) ведется только в том случае, если
      Opt.ShowMenuBar == 0
  11.07.2003 SVS
    + FileList::IsNumeric()
  05.07.2003 SVS
    - под масдаем неотрисовка верхних стыковочных линий (BoxText!)
  06.05.2003 SVS
    ! W-console: замена вида "BoxText(209);" на  "BoxText(BoxSymbols[0xD1-0x0B0]);"
  03.03.2003 SVS
    ! Оформим парвильное окончание при выделении (для "21" было "файлов") с помощью __FormatEndSelectedPhrase()
  20.02.2003 SVS
    ! Заменим strcmp(FooBar,"..") на TestParentFolderName(FooBar)
    ! При отображении колонки ATTR_COLUMN откажимся от sprintf()
      в пользу манипуляции байтовым массивом.
    ! При отображении колонки NUMLINK_COLUMN вместо sprintf()
      применим itoa().
  13.01.2003 SVS
    + Новая опция в настройках режимов панелей: "Выравнивать расширения
      папок" - позволяет показывать расширения папок выравненными независимо
      от опции "Выравнивать расширения файлов".
  22.06.2002 SVS
    ! Исправление исправлений в 1444 (предыдущие исправления ;-)
  15.06.2002 VVM
    ! Скорректировать позицию после подготовки режима панели. Т.к. колонки могут
      измениться и надо-юы сдвинуть курсор. (bug #472)
  25.05.2002 IS
    ! первый параметр у ConvertDate теперь ссылка на константу
  22.03.2002 SVS
    - strcpy - Fuck!
  21.03.2002 DJ
    ! не портим стек при отрисовке длинного заголовка панели
  20.03.2002 IS
    ! Косметика для предыдущего патча от SVS - вызываем
      PointToFolderNameIfFolder
  19.03.2002 SVS
    - BugZ#375 - Неправильное отображение каталогов в Alt-F7.
  02.03.2002 SVS
    ! Подготовим "полный прямоугольник" - SetScreen
  09.11.2001 IS
    ! Вместо явно прописанных фигурных скобок для обозначения границ длинных
      имен файлов используем данные из lng.
  01.10.2001 IS
    ! демонстрация функции TruncStrFromEnd
  27.09.2001 IS
    - Левый размер при использовании strncpy
  05.09.2001 SVS
    ! Немного оптимизации ;-)
    ! Вместо полей Color* в структе FileListItem используется
      структура HighlightDataColor
  23.08.2001 VVM
    ! Убран один лишний вызов. Остаток от предыдущего патча.
  01.08.2001 VVM
    + TotalStr в панели обрезается справа, а не слева
  17.06.2001 SVS
    ! MListSymLink & MListFolder
  14.06.2001 OT
    ! "Бунт" ;-)
  22.05.2001 tran
    ! по результам прогона на CodeGuard
  22.05.201 OT
    - Баг с определением, режиме панели -  на весь экран, или только на половину :)
  16.05.2001 SVS
    ! _D() -> _OT()  ;-)
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  20.03.2001 SVS
    ! подсократим - весь код по форматированию размера файла из
      FileList::ShowList() вынесен в отдельную функцию - FileSizeToStr()
      (strmix.cpp).
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  27.02.2001 VVM
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
  16.02.2001 OT
    - БАГА при очень широком столбце в панели
  14.01.2001 SVS
    ! 'P' - обозначающий reparse point заменен на 'L'
  20.10.2000 SVS
    + Добавлен Encrypted
      Поток может быть либо COMPRESSED (С) либо ENCRYPTED (E)
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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
static BYTE VerticalLine[2]={0x0B3,0x0BA};
static char OutCharacter[2]={0,0};

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
  if ( Locked () )
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

  if(CurFullScreen!=IsFullScreen())
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
  for (int I=0,ColumnPos=X1+1;I<ViewSettings.ColumnCount;I++)
  {
    if (ViewSettings.ColumnWidth[I]<0)
      continue;
    if (Opt.ShowColumnTitles)
    {
      char *Title="";
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
        char *SortStr=MSG(SortStrings[I]);
        char *Ch=strchr(SortStr,'&');
        if (Ch!=NULL)
        {
          if (Opt.ShowColumnTitles)
            GotoXY(X1+1,Y1+1);
          else
            GotoXY(X1+1,Y1);
          SetColor(COL_PANELCOLUMNTITLE);
          OutCharacter[0]=SortOrder==1 ? LocalLower(Ch[1]):LocalUpper(Ch[1]);
          Text(OutCharacter);
          if (Filter!=NULL && Filter->IsEnabled())
          {
            OutCharacter[0]='*';
            Text(OutCharacter);
          }
        }
        break;
      }
    }
  }

  if (!Fast && GetFocus())
  {
    CtrlObject->CmdLine->SetCurDir(PanelMode==PLUGIN_PANEL ? Info.CurDir:CurDir);
    CtrlObject->CmdLine->Show();
  }
  int TitleX2=Opt.Clock && !Opt.ShowMenuBar ? Min(ScrX-4,X2):X2;
  int TruncSize=TitleX2-X1-3;
  if (!Opt.ShowColumnTitles && Opt.ShowSortMode && Filter!=NULL && Filter->IsEnabled())
    TruncSize-=2;
  if (PanelMode==PLUGIN_PANEL)
  {
    /* $ 21.03.2002 DJ
       не портим стек
    */
    xstrncpy(Title,NullToEmpty(Info.PanelTitle),sizeof (Title)-1);
    /* DJ $ */
    TruncStr(Title,TruncSize);
  }
  else
  {
    char TitleDir[NM];
    if (ShowShortNames)
      ConvertNameToShort(CurDir,TitleDir);
    else
      xstrncpy(TitleDir,CurDir,sizeof(TitleDir)-1);
    TruncPathStr(TitleDir,TruncSize-2);
    sprintf(Title," %s ",TitleDir);
  }
  Length=strlen(Title);
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
    struct FileListItem *CurPtr=ListData+CurFile;
    if (!TestParentFolderName(CurPtr->Name))
    {
      strcpy(CurDir,CurPtr->Name);
      char *NamePtr=strrchr(CurDir,'\\');
      if (NamePtr!=NULL && NamePtr!=CurDir)
        if (*(NamePtr-1)!=':')
          *NamePtr=0;
        else
          *(NamePtr+1)=0;
      if (GetFocus())
      {
        CtrlObject->CmdLine->SetCurDir(CurDir);
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
    struct FileListItem *CurPtr=ListData+Position;

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
  char SelStr[256],FormStr[20];

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
    InsertCommas(SelFileSize,FormStr);
    sprintf(SelStr,MSG(__FormatEndSelectedPhrase(SelFileCount)),FormStr,SelFileCount);

    TruncStr(SelStr,X2-X1-1);
    Length=strlen(SelStr);
    SetColor(COL_PANELSELECTEDINFO);
    GotoXY(X1+(X2-X1+1-Length)/2,Y2-2*Opt.ShowPanelStatus);
    Text(SelStr);
  }
  else
    if (!RegVer)
    {
      char EvalStr[256];
      char *EvalMsg=MSG(MListEval);
      if (*EvalMsg==0)
        strcpy(EvalStr," Evaluation version ");
      else
        sprintf(EvalStr," %s ",MSG(MListEval));
      TruncStr(EvalStr,X2-X1-1);
      Length=strlen(EvalStr);
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

  InsertCommas(TotalFileSize,FormSize);
  *FreeSize=0;
  if (Opt.ShowPanelFree && (PanelMode!=PLUGIN_PANEL || (Info.Flags & OPIF_REALNAMES)))
    InsertCommas(FreeDiskSize,FreeSize);
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
        InsertCommas(FreeDiskSize>>20,FreeSize);
        InsertCommas(TotalFileSize>>20,FormSize);
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
  Length=strlen(TotalStr);
  GotoXY(X1+(X2-X1+1-Length)/2,Y2);

// UNICODE!!!
  char *FirstBox=strchr(TotalStr,0x0CD);
  int BoxPos=(FirstBox==NULL) ? -1:FirstBox-TotalStr;
  int BoxLength=0;
  if (BoxPos!=-1)
// UNICODE!!!
    for (int I=0;TotalStr[BoxPos+I]==0x0CD;I++)
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
  int SrcLength=strlen(SrcName);
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
    int DotLength=strlen(DotPtr+1);
    int NameLength=DotPtr-SrcName;
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
        if ( !ShowStatus )
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
            int Length=strlen(ColumnData);
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
          switch(ColumnType)
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
                  OutCharacter[0]=CurPtr->Colors.MarkChar;
                  Text(OutCharacter);
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
                  int Length=strlen(NamePtr);
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
                    else
                      if (RightAlign)
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
                  char *PtrName;
                  if (TestParentFolderName(CurPtr->Name))
                    PtrName=MSG(MListUp);
                  else
                    PtrName=MSG(CurPtr->FileAttr&FILE_ATTRIBUTE_REPARSE_POINT?MListSymLink:MListFolder);

                  if (strlen(PtrName) <= Width-2)
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
                           Packed?CurPtr->PackSizeHigh:CurPtr->UnpSizeHigh,
                           Packed?CurPtr->PackSize:CurPtr->UnpSize,
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
                char DateStr[30],TimeStr[30],OutStr[30];
                ConvertDate(*FileTime,DateStr,TimeStr,ColumnWidth-9,Brief,TextMonth,FullYear);
                sprintf(OutStr,"%s %s",DateStr,TimeStr);
                mprintf("%*.*s",ColumnWidth,ColumnWidth,OutStr);
              }
              break;
            case ATTR_COLUMN:
              {
                char OutStr[8];
                DWORD FileAttr=CurPtr->FileAttr;
                OutStr[0]=(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) ? 'L' : ' ';
                // $ 20.10.2000 SVS - Encrypted NTFS/Win2K - Поток может быть либо COMPRESSED (С) либо ENCRYPTED (E)
                OutStr[1]=(FileAttr & FILE_ATTRIBUTE_COMPRESSED) ? 'C':((FileAttr & FILE_ATTRIBUTE_ENCRYPTED)?'E':' ');
                OutStr[2]=(FileAttr & FILE_ATTRIBUTE_ARCHIVE) ? 'A':' ';
                OutStr[3]=(FileAttr & FILE_ATTRIBUTE_SYSTEM) ? 'S':' ';
                OutStr[4]=(FileAttr & FILE_ATTRIBUTE_HIDDEN) ? 'H':' ';
                OutStr[5]=(FileAttr & FILE_ATTRIBUTE_READONLY) ? 'R':' ';
                OutStr[6]=0;
                char *OutPtr=OutStr;
                if (ColumnWidth<6)
                  OutPtr=OutStr+6-ColumnWidth;
                mprintf("%*s",ColumnWidth,OutPtr);
              }
              break;
            case DIZ_COLUMN:
              {
                int CurLeftPos=0;
                if (!ShowStatus && LeftPos>0)
                {
                  int Length=CurPtr->DizText ? strlen(CurPtr->DizText):0;
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
                int CurLeftPos=0;
                if (!ShowStatus && LeftPos>0)
                {
                  int Length=strlen(CurPtr->Owner);
                  if (Length>ColumnWidth)
                  {
                    CurLeftPos=LeftPos;
                    if (CurLeftPos>Length-ColumnWidth)
                      CurLeftPos=Length-ColumnWidth;
                    if (CurLeftPos>MaxLeftPos)
                      MaxLeftPos=CurLeftPos;
                  }
                }
                mprintf("%-*.*s",ColumnWidth,ColumnWidth,CurPtr->Owner+CurLeftPos);
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
          BoxText((WORD)(CurX+ColumnWidth==X2 ? (Opt.UseUnicodeConsole?BoxSymbols[VerticalLine[1]-0x0B0]:VerticalLine[1]):0x20));
        else
          BoxText((WORD)(ShowStatus ? 0x20:(Opt.UseUnicodeConsole?BoxSymbols[VerticalLine[0]-0x0B0]:VerticalLine[0])));

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
