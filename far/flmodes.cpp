/*
flmodes.cpp

Файловая панель - работа с режимами

*/

/* Revision: 1.03 21.05.2001 $ */

/*
Modify:
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "filelist.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"

int ColumnTypeWidth[]={0,8,8,8,5,14,14,14,6,0,0,3,
                              0,0,0,0,0,0,0,0,0,0};

static char *ColumnSymbol[]={"N","S","P","D","T",
                             "DM","DC","DA","A","Z","O","LN","C0","C1",
                             "C2","C3","C4","C5","C6","C7","C8","C9"};

struct PanelViewSettings ViewSettingsArray[]=
{
  {{COLUMN_MARK|NAME_COLUMN,SIZE_COLUMN|COLUMN_COMMAS,DATE_COLUMN},{0,10,0},3,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,0,1,0,0,1},
  {{NAME_COLUMN,NAME_COLUMN,NAME_COLUMN},{0,0,0},3,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,8,0,5},4,0,1,0,0,1},
  {{NAME_COLUMN,NAME_COLUMN},{0,0},2,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,8,0,5},4,0,0,0,0,1},
  {{NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,8,0,5},4,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,0,1,0,0,1},
  {{NAME_COLUMN,SIZE_COLUMN},{0,8},2,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,8,0,5},4,0,0,0,0,1},
  {{NAME_COLUMN,SIZE_COLUMN,PACKED_COLUMN,MDATE_COLUMN,CDATE_COLUMN,ADATE_COLUMN,ATTR_COLUMN},{0,6,6,14,14,14,0},7,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,1,1,0,0,1},
  {{NAME_COLUMN,DIZ_COLUMN},{12,0},2,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,8,0,5},4,0,1,0,0,1},
  {{NAME_COLUMN,SIZE_COLUMN,DIZ_COLUMN},{0,8,54},3,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,1,1,0,0,1},
  {{NAME_COLUMN,SIZE_COLUMN,OWNER_COLUMN},{0,8,15},3,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,8,0,5},4,0,1,0,0,1},
  {{NAME_COLUMN,SIZE_COLUMN,NUMLINK_COLUMN},{0,8,3},3,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,8,0,5},4,0,1,0,0,1}
};

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
    struct MenuData ModeListMenu[]=
    {
      (char *)MEditPanelModesBrief,0,
      (char *)MEditPanelModesMedium,0,
      (char *)MEditPanelModesFull,0,
      (char *)MEditPanelModesWide,0,
      (char *)MEditPanelModesDetailed,0,
      (char *)MEditPanelModesDiz,0,
      (char *)MEditPanelModesLongDiz,0,
      (char *)MEditPanelModesOwners,0,
      (char *)MEditPanelModesLinks,0,
      (char *)MEditPanelModesAlternative,0
    };
    int ModeNumber;
    ModeListMenu[CurMode].SetSelect(1);
    {
      VMenu ModeList(MSG(MEditPanelModes),ModeListMenu,sizeof(ModeListMenu)/sizeof(ModeListMenu[0]),ScrY-4);
      ModeList.SetPosition(-1,-1,0,0);
      ModeList.SetHelp("PanelViewModes");
      ModeList.Process();
      ModeNumber=ModeList.GetExitCode();
    }
    if (ModeNumber<0)
      return;
    CurMode=ModeNumber;

    static struct DialogData ModeDlgData[]=
    {
      DI_DOUBLEBOX,3,1,72,15,0,0,0,0,"",
      DI_TEXT,5,2,0,0,0,0,0,0,(char *)MEditPanelModeTypes,
      DI_EDIT,5,3,35,3,1,0,0,0,"",
      DI_TEXT,5,4,0,0,0,0,0,0,(char *)MEditPanelModeWidths,
      DI_EDIT,5,5,35,3,0,0,0,0,"",
      DI_TEXT,38,2,0,0,0,0,0,0,(char *)MEditPanelModeStatusTypes,
      DI_EDIT,38,3,70,3,0,0,0,0,"",
      DI_TEXT,38,4,0,0,0,0,0,0,(char *)MEditPanelModeStatusWidths,
      DI_EDIT,38,5,70,3,0,0,0,0,"",
      DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      DI_TEXT,-1,6,0,0,0,0,DIF_BOXCOLOR,0,(char *)MEditPanelReadHelp,
      DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MEditPanelModeFullscreen,
      DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MEditPanelModeAlignExtensions,
      DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MEditPanelModeFoldersUpperCase,
      DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MEditPanelModeFilesLowerCase,
      DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MEditPanelModeUpperToLowerCase,
      DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)MEditPanelModeCaseSensitiveSort,
      DI_TEXT,3,13,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
      DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
      DI_BUTTON,0,14,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
    };
    MakeDialogItems(ModeDlgData,ModeDlg);
    int ExitCode;

    strcpy(ModeDlg[0].Data,MSG((int)ModeListMenu[ModeNumber].Name));
    RemoveHighlights(ModeDlg[0].Data);

    if (ModeNumber==9)
      ModeNumber=0;
    else
      ModeNumber++;

    struct PanelViewSettings NewSettings=ViewSettingsArray[ModeNumber];

    ModeDlg[11].Selected=NewSettings.FullScreen;
    ModeDlg[12].Selected=NewSettings.AlignExtensions;
    ModeDlg[13].Selected=NewSettings.FolderUpperCase;
    ModeDlg[14].Selected=NewSettings.FileLowerCase;
    ModeDlg[15].Selected=NewSettings.FileUpperToLowerCase;
    ModeDlg[16].Selected=NewSettings.CaseSensitiveSort;

    ViewSettingsToText(NewSettings.ColumnType,NewSettings.ColumnWidth,
        NewSettings.ColumnCount,ModeDlg[2].Data,ModeDlg[4].Data);
    ViewSettingsToText(NewSettings.StatusColumnType,NewSettings.StatusColumnWidth,
        NewSettings.StatusColumnCount,ModeDlg[6].Data,ModeDlg[8].Data);

    {
      Dialog Dlg(ModeDlg,sizeof(ModeDlg)/sizeof(ModeDlg[0]));
      Dlg.SetPosition(-1,-1,76,17);
      Dlg.SetHelp("PanelViewModes");
      Dlg.Process();
      ExitCode=Dlg.GetExitCode();
    }
    if (ExitCode!=18)
      continue;

    memset(&NewSettings,0,sizeof(NewSettings));
    NewSettings.FullScreen=ModeDlg[11].Selected;
    NewSettings.AlignExtensions=ModeDlg[12].Selected;
    NewSettings.FolderUpperCase=ModeDlg[13].Selected;
    NewSettings.FileLowerCase=ModeDlg[14].Selected;
    NewSettings.FileUpperToLowerCase=ModeDlg[15].Selected;
    NewSettings.CaseSensitiveSort=ModeDlg[16].Selected;

    TextToViewSettings(ModeDlg[2].Data,ModeDlg[4].Data,NewSettings.ColumnType,
                       NewSettings.ColumnWidth,NewSettings.ColumnCount);
    TextToViewSettings(ModeDlg[6].Data,ModeDlg[8].Data,NewSettings.StatusColumnType,
                       NewSettings.StatusColumnWidth,NewSettings.StatusColumnCount);

    ViewSettingsArray[ModeNumber]=NewSettings;
    CtrlObject->Cp()->LeftPanel->SortFileList(TRUE);
    CtrlObject->Cp()->RightPanel->SortFileList(TRUE);
    CtrlObject->Cp()->SetScreenPositions();
    int LeftMode=CtrlObject->Cp()->LeftPanel->GetViewMode();
    int RightMode=CtrlObject->Cp()->RightPanel->GetViewMode();
    CtrlObject->Cp()->LeftPanel->SetViewMode(ModeNumber);
    CtrlObject->Cp()->RightPanel->SetViewMode(ModeNumber);
    CtrlObject->Cp()->LeftPanel->SetViewMode(LeftMode);
    CtrlObject->Cp()->RightPanel->SetViewMode(RightMode);
    CtrlObject->Cp()->LeftPanel->Redraw();
    CtrlObject->Cp()->RightPanel->Redraw();
  }
}


void FileList::ReadPanelModes()
{
  for (int I=0;I<10;I++)
  {
    char ColumnTitles[NM],ColumnWidths[NM];
    char StatusColumnTitles[NM],StatusColumnWidths[NM],RegKey[80];
    sprintf(RegKey,"Panel\\ViewModes\\Mode%d",I);
    GetRegKey(RegKey,"Columns",ColumnTitles,"",sizeof(ColumnTitles));
    GetRegKey(RegKey,"ColumnWidths",ColumnWidths,"",sizeof(ColumnWidths));
    GetRegKey(RegKey,"StatusColumns",StatusColumnTitles,"",sizeof(StatusColumnTitles));
    GetRegKey(RegKey,"StatusColumnWidths",StatusColumnWidths,"",sizeof(StatusColumnWidths));
    if (*ColumnTitles==0 || *ColumnWidths==0)
      continue;
    struct PanelViewSettings NewSettings=ViewSettingsArray[VIEW_0+I];

    if (*ColumnTitles)
      TextToViewSettings(ColumnTitles,ColumnWidths,NewSettings.ColumnType,
                         NewSettings.ColumnWidth,NewSettings.ColumnCount);
    if (*StatusColumnTitles)
      TextToViewSettings(StatusColumnTitles,StatusColumnWidths,NewSettings.StatusColumnType,
                         NewSettings.StatusColumnWidth,NewSettings.StatusColumnCount);

    GetRegKey(RegKey,"FullScreen",NewSettings.FullScreen,0);
    GetRegKey(RegKey,"AlignExtensions",NewSettings.AlignExtensions,1);
    GetRegKey(RegKey,"FolderUpperCase",NewSettings.FolderUpperCase,0);
    GetRegKey(RegKey,"FileLowerCase",NewSettings.FileLowerCase,0);
    GetRegKey(RegKey,"FileUpperToLowerCase",NewSettings.FileUpperToLowerCase,1);
    GetRegKey(RegKey,"CaseSensitiveSort",NewSettings.CaseSensitiveSort,0);

    ViewSettingsArray[VIEW_0+I]=NewSettings;
  }
}


void FileList::SavePanelModes()
{
  for (int I=0;I<10;I++)
  {
    char ColumnTitles[NM],ColumnWidths[NM];
    char StatusColumnTitles[NM],StatusColumnWidths[NM],RegKey[80];
    sprintf(RegKey,"Panel\\ViewModes\\Mode%d",I);
    struct PanelViewSettings NewSettings=ViewSettingsArray[VIEW_0+I];
    ViewSettingsToText(NewSettings.ColumnType,NewSettings.ColumnWidth,
        NewSettings.ColumnCount,ColumnTitles,ColumnWidths);
    ViewSettingsToText(NewSettings.StatusColumnType,NewSettings.StatusColumnWidth,
        NewSettings.StatusColumnCount,StatusColumnTitles,StatusColumnWidths);

    SetRegKey(RegKey,"Columns",ColumnTitles);
    SetRegKey(RegKey,"ColumnWidths",ColumnWidths);
    SetRegKey(RegKey,"StatusColumns",StatusColumnTitles);
    SetRegKey(RegKey,"StatusColumnWidths",StatusColumnWidths);

    SetRegKey(RegKey,"FullScreen",NewSettings.FullScreen);
    SetRegKey(RegKey,"AlignExtensions",NewSettings.AlignExtensions);
    SetRegKey(RegKey,"FolderUpperCase",NewSettings.FolderUpperCase);
    SetRegKey(RegKey,"FileLowerCase",NewSettings.FileLowerCase);
    SetRegKey(RegKey,"FileUpperToLowerCase",NewSettings.FileUpperToLowerCase);
    SetRegKey(RegKey,"CaseSensitiveSort",NewSettings.CaseSensitiveSort);
  }
}


void FileList::TextToViewSettings(char *ColumnTitles,char *ColumnWidths,
     unsigned int *ViewColumnTypes,int *ViewColumnWidths,int &ColumnCount)
{
  char *TextPtr=ColumnTitles;
  for (ColumnCount=0;ColumnCount<sizeof(ViewSettingsArray[0].ColumnType)/sizeof(ViewSettingsArray[0].ColumnType[0]);ColumnCount++)
  {
    char ArgName[NM];
    if ((TextPtr=GetCommaWord(TextPtr,ArgName))==NULL)
      break;
    strupr(ArgName);
    if (*ArgName=='N')
    {
      unsigned int &ColumnType=ViewColumnTypes[ColumnCount];
      ColumnType=NAME_COLUMN;
      for (int I=1;ArgName[I];I++)
        switch(ArgName[I])
        {
          case 'M':
            ColumnType|=COLUMN_MARK;
            break;
          case 'O':
            ColumnType|=COLUMN_NAMEONLY;
            break;
          case 'R':
            ColumnType|=COLUMN_RIGHTALIGN;
            break;
        }
    }
    else
      if (*ArgName=='S' || *ArgName=='P')
      {
        unsigned int &ColumnType=ViewColumnTypes[ColumnCount];
        ColumnType=(*ArgName=='S') ? SIZE_COLUMN:PACKED_COLUMN;
        for (int I=1;ArgName[I];I++)
          switch(ArgName[I])
          {
            case 'C':
              ColumnType|=COLUMN_COMMAS;
              break;
            case 'T':
              ColumnType|=COLUMN_THOUSAND;
              break;
          }
      }
      else
        if (strncmp(ArgName,"DM",2)==0 || strncmp(ArgName,"DC",2)==0 || strncmp(ArgName,"DA",2)==0)
        {
          unsigned int &ColumnType=ViewColumnTypes[ColumnCount];
          switch(ArgName[1])
          {
            case 'M':
              ColumnType=MDATE_COLUMN;
              break;
            case 'C':
              ColumnType=CDATE_COLUMN;
              break;
            case 'A':
              ColumnType=ADATE_COLUMN;
              break;
          }
          for (int I=2;ArgName[I];I++)
            switch(ArgName[I])
            {
              case 'B':
                ColumnType|=COLUMN_BRIEF;
                break;
              case 'M':
                ColumnType|=COLUMN_MONTH;
                break;
            }
        }
        else
          for (int I=0;I<sizeof(ColumnSymbol)/sizeof(ColumnSymbol[0]);I++)
            if (strcmp(ArgName,ColumnSymbol[I])==0)
            {
              ViewColumnTypes[ColumnCount]=I;
              break;
            }
    }

  TextPtr=ColumnWidths;
  for (int I=0;I<ColumnCount;I++)
  {
    char ArgName[NM];
    if ((TextPtr=GetCommaWord(TextPtr,ArgName))==NULL)
      break;
    ViewColumnWidths[I]=atoi(ArgName);
  }
}


void FileList::ViewSettingsToText(unsigned int *ViewColumnTypes,
     int *ViewColumnWidths,int ColumnCount,char *ColumnTitles,
     char *ColumnWidths)
{
  *ColumnTitles=*ColumnWidths=0;
  for (int I=0;I<ColumnCount;I++)
  {
    char Type[100];
    int ColumnType=ViewColumnTypes[I] & 0xff;
    strcpy(Type,ColumnSymbol[ColumnType]);
    if (ColumnType==NAME_COLUMN)
    {
      if (ViewColumnTypes[I] & COLUMN_MARK)
        strcat(Type,"M");
      if (ViewColumnTypes[I] & COLUMN_NAMEONLY)
        strcat(Type,"O");
      if (ViewColumnTypes[I] & COLUMN_RIGHTALIGN)
        strcat(Type,"R");
    }
    if (ColumnType==SIZE_COLUMN || ColumnType==PACKED_COLUMN)
    {
      if (ViewColumnTypes[I] & COLUMN_COMMAS)
        strcat(Type,"C");
      if (ViewColumnTypes[I] & COLUMN_THOUSAND)
        strcat(Type,"T");
    }
    if (ColumnType==MDATE_COLUMN || ColumnType==ADATE_COLUMN || ColumnType==CDATE_COLUMN)
    {
      if (ViewColumnTypes[I] & COLUMN_BRIEF)
        strcat(Type,"B");
      if (ViewColumnTypes[I] & COLUMN_MONTH)
        strcat(Type,"M");
    }
    strcat(ColumnTitles,Type);
    sprintf(ColumnWidths+strlen(ColumnWidths),"%d",ViewColumnWidths[I]);
    if (I<ColumnCount-1)
    {
      strcat(ColumnTitles,",");
      strcat(ColumnWidths,",");
    }
  }
}
