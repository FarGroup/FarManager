/*
flmodes.cpp

�������� ������ - ������ � ��������

*/

/* Revision: 1.25 23.01.2006 $ */

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

int       ColumnTypeWidth[]={ 0,  6,  6,  8,  5,  14,  14,  14,  6,  0,  0,  3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0   };

static wchar_t *ColumnSymbol[]={L"N",L"S",L"P",L"D",L"T",L"DM",L"DC",L"DA",L"A",L"Z",L"O",L"LN",L"C0",L"C1",L"C2",L"C3",L"C4",L"C5",L"C6",L"C7",L"C8",L"C9"};

struct PanelViewSettings ViewSettingsArray[]=
{
/* 00 */{{COLUMN_MARK|NAME_COLUMN,SIZE_COLUMN|COLUMN_COMMAS,DATE_COLUMN},{0,10,0},3,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,0,1,0,0,0,1,0},
/* 01 */{{NAME_COLUMN,NAME_COLUMN,NAME_COLUMN},{0,0,0},3,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,1,0,0,0,1,0},
/* 02 */{{NAME_COLUMN,NAME_COLUMN},{0,0},2,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,0,0,0,0,1,0},
/* 03 */{{NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,0,1,0,0,0,1,0},
/* 04 */{{NAME_COLUMN,SIZE_COLUMN},{0,6},2,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,0,0,0,0,1,0},
/* 05 */{{NAME_COLUMN,SIZE_COLUMN,PACKED_COLUMN,MDATE_COLUMN,CDATE_COLUMN,ADATE_COLUMN,ATTR_COLUMN},{0,6,6,14,14,14,0},7,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,1,1,0,0,0,1,0},
/* 06 */{{NAME_COLUMN,DIZ_COLUMN},{12,0},2,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,1,0,0,0,1,0},
/* 07 */{{NAME_COLUMN,SIZE_COLUMN,DIZ_COLUMN},{0,6,54},3,{COLUMN_RIGHTALIGN|NAME_COLUMN},{0},1,1,1,0,0,0,1,0},
/* 08 */{{NAME_COLUMN,SIZE_COLUMN,OWNER_COLUMN},{0,6,15},3,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,1,0,0,0,1,0},
/* 09 */{{NAME_COLUMN,SIZE_COLUMN,NUMLINK_COLUMN},{0,6,3},3,{COLUMN_RIGHTALIGN|NAME_COLUMN,SIZE_COLUMN,DATE_COLUMN,TIME_COLUMN},{0,6,0,5},4,0,1,0,0,0,1,0}
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
    struct MenuDataEx ModeListMenu[]=
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
      VMenu ModeList(UMSG(MEditPanelModes),ModeListMenu,sizeof(ModeListMenu)/sizeof(ModeListMenu[0]),TRUE,ScrY-4);
      ModeList.SetPosition(-1,-1,0,0);
      ModeList.SetHelp(L"PanelViewModes");
      /* $ 16.06.2001 KM
         ! ���������� WRAPMODE � ����.
      */
      ModeList.SetFlags(VMENU_WRAPMODE);
      /* KM $ */
      ModeList.Process();
      ModeNumber=ModeList.Modal::GetExitCode();
    }
    if (ModeNumber<0)
      return;
    CurMode=ModeNumber;

    static struct DialogDataEx ModeDlgData[]=
    {
    /* 00 */DI_DOUBLEBOX,3,1,72,16,0,0,0,0,L"",
    /* 01 */DI_TEXT,5,2,0,0,0,0,0,0,(const wchar_t *)MEditPanelModeTypes,
    /* 02 */DI_EDIT,5,3,35,3,1,0,0,0,L"",
    /* 03 */DI_TEXT,5,4,0,0,0,0,0,0,(const wchar_t *)MEditPanelModeWidths,
    /* 04 */DI_EDIT,5,5,35,3,0,0,0,0,L"",
    /* 05 */DI_TEXT,38,2,0,0,0,0,0,0,(const wchar_t *)MEditPanelModeStatusTypes,
    /* 06 */DI_EDIT,38,3,70,3,0,0,0,0,L"",
    /* 07 */DI_TEXT,38,4,0,0,0,0,0,0,(const wchar_t *)MEditPanelModeStatusWidths,
    /* 08 */DI_EDIT,38,5,70,3,0,0,0,0,L"",
    /* 09 */DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
    /* 10 */DI_TEXT,-1,6,0,0,0,0,DIF_BOXCOLOR,0,(const wchar_t *)MEditPanelReadHelp,
    /* 11 */DI_CHECKBOX,5,7,0,0,0,0,0,0,(const wchar_t *)MEditPanelModeFullscreen,
    /* 12 */DI_CHECKBOX,5,8,0,0,0,0,0,0,(const wchar_t *)MEditPanelModeAlignExtensions,
    /* 13 */DI_CHECKBOX,5,9,0,0,0,0,0,0,(const wchar_t *)MEditPanelModeAlignFolderExtensions,
    /* 14 */DI_CHECKBOX,5,10,0,0,0,0,0,0,(const wchar_t *)MEditPanelModeFoldersUpperCase,
    /* 15 */DI_CHECKBOX,5,11,0,0,0,0,0,0,(const wchar_t *)MEditPanelModeFilesLowerCase,
    /* 16 */DI_CHECKBOX,5,12,0,0,0,0,0,0,(const wchar_t *)MEditPanelModeUpperToLowerCase,
    /* 17 */DI_CHECKBOX,5,13,0,0,0,0,0,0,(const wchar_t *)MEditPanelModeCaseSensitiveSort,
    /* 19 */DI_TEXT,3,14,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
    /* 20 */DI_BUTTON,0,15,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MOk,
    /* 21 */DI_BUTTON,0,15,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MCancel
    };
    MakeDialogItemsEx(ModeDlgData,ModeDlg);
    int ExitCode;

    ModeDlg[0].strData = UMSG((int)ModeListMenu[ModeNumber].Name);
    RemoveHighlightsW(ModeDlg[0].strData);

    if (ModeNumber==9)
      ModeNumber=0;
    else
      ModeNumber++;

    struct PanelViewSettings NewSettings=ViewSettingsArray[ModeNumber];

    ModeDlg[11].Selected=NewSettings.FullScreen;
    ModeDlg[12].Selected=NewSettings.AlignExtensions;
    ModeDlg[13].Selected=NewSettings.FolderAlignExtensions;
    ModeDlg[14].Selected=NewSettings.FolderUpperCase;
    ModeDlg[15].Selected=NewSettings.FileLowerCase;
    ModeDlg[16].Selected=NewSettings.FileUpperToLowerCase;
    ModeDlg[17].Selected=NewSettings.CaseSensitiveSort;

    ViewSettingsToText(NewSettings.ColumnType,NewSettings.ColumnWidth,
        NewSettings.ColumnCount,ModeDlg[2].strData,ModeDlg[4].strData);
    ViewSettingsToText(NewSettings.StatusColumnType,NewSettings.StatusColumnWidth,
        NewSettings.StatusColumnCount,ModeDlg[6].strData,ModeDlg[8].strData);

    {
      Dialog Dlg(ModeDlg,sizeof(ModeDlg)/sizeof(ModeDlg[0]));
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
                       NewSettings.ColumnWidth,NewSettings.ColumnCount);
    TextToViewSettings(ModeDlg[6].strData,ModeDlg[8].strData,NewSettings.StatusColumnType,
                       NewSettings.StatusColumnWidth,NewSettings.StatusColumnCount);

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
  for (int I=0;I<10;I++)
  {
    string strColumnTitles, strColumnWidths;
    string strStatusColumnTitles, strStatusColumnWidths, strRegKey;
    strRegKey.Format (L"Panel\\ViewModes\\Mode%d",I);
    GetRegKeyW(strRegKey,L"Columns",strColumnTitles,L"");
    GetRegKeyW(strRegKey,L"ColumnWidths",strColumnWidths,L"");
    GetRegKeyW(strRegKey,L"StatusColumns",strStatusColumnTitles,L"");
    GetRegKeyW(strRegKey,L"StatusColumnWidths",strStatusColumnWidths,L"");
    if ( strColumnTitles.IsEmpty() || strColumnWidths.IsEmpty() )
      continue;

    struct PanelViewSettings NewSettings=ViewSettingsArray[VIEW_0+I];

    if ( !strColumnTitles.IsEmpty() )
      TextToViewSettings(strColumnTitles,strColumnWidths,NewSettings.ColumnType,
                         NewSettings.ColumnWidth,NewSettings.ColumnCount);
    if ( !strStatusColumnTitles.IsEmpty() )
      TextToViewSettings(strStatusColumnTitles,strStatusColumnWidths,NewSettings.StatusColumnType,
                         NewSettings.StatusColumnWidth,NewSettings.StatusColumnCount);

    GetRegKeyW(strRegKey,L"FullScreen",NewSettings.FullScreen,0);
    GetRegKeyW(strRegKey,L"AlignExtensions",NewSettings.AlignExtensions,1);
    GetRegKeyW(strRegKey,L"FolderAlignExtensions",NewSettings.FolderAlignExtensions,0);
    GetRegKeyW(strRegKey,L"FolderUpperCase",NewSettings.FolderUpperCase,0);
    GetRegKeyW(strRegKey,L"FileLowerCase",NewSettings.FileLowerCase,0);
    GetRegKeyW(strRegKey,L"FileUpperToLowerCase",NewSettings.FileUpperToLowerCase,1);
    GetRegKeyW(strRegKey,L"CaseSensitiveSort",NewSettings.CaseSensitiveSort,0);

    ViewSettingsArray[VIEW_0+I]=NewSettings;
  }
}


void FileList::SavePanelModes()
{
  for (int I=0;I<10;I++)
  {
    string strColumnTitles, strColumnWidths;
    string strStatusColumnTitles, strStatusColumnWidths, strRegKey;
    strRegKey.Format (L"Panel\\ViewModes\\Mode%d",I);
    struct PanelViewSettings NewSettings=ViewSettingsArray[VIEW_0+I];
    ViewSettingsToText(NewSettings.ColumnType,NewSettings.ColumnWidth,
        NewSettings.ColumnCount,strColumnTitles,strColumnWidths);
    ViewSettingsToText(NewSettings.StatusColumnType,NewSettings.StatusColumnWidth,
        NewSettings.StatusColumnCount,strStatusColumnTitles,strStatusColumnWidths);

    SetRegKeyW(strRegKey,L"Columns",strColumnTitles);
    SetRegKeyW(strRegKey,L"ColumnWidths",strColumnWidths);
    SetRegKeyW(strRegKey,L"StatusColumns",strStatusColumnTitles);
    SetRegKeyW(strRegKey,L"StatusColumnWidths",strStatusColumnWidths);

    SetRegKeyW(strRegKey,L"FullScreen",NewSettings.FullScreen);
    SetRegKeyW(strRegKey,L"AlignExtensions",NewSettings.AlignExtensions);
    SetRegKeyW(strRegKey,L"FolderAlignExtensions",NewSettings.FolderAlignExtensions);
    SetRegKeyW(strRegKey,L"FolderUpperCase",NewSettings.FolderUpperCase);
    SetRegKeyW(strRegKey,L"FileLowerCase",NewSettings.FileLowerCase);
    SetRegKeyW(strRegKey,L"FileUpperToLowerCase",NewSettings.FileUpperToLowerCase);
    SetRegKeyW(strRegKey,L"CaseSensitiveSort",NewSettings.CaseSensitiveSort);
  }
}


void FileList::TextToViewSettings(const wchar_t *ColumnTitles,const wchar_t *ColumnWidths,
     unsigned int *ViewColumnTypes,int *ViewColumnWidths,int &ColumnCount)
{
  const wchar_t *TextPtr=ColumnTitles;
  for (ColumnCount=0;ColumnCount<sizeof(ViewSettingsArray[0].ColumnType)/sizeof(ViewSettingsArray[0].ColumnType[0]);ColumnCount++)
  {
    string strArgName;
    if ((TextPtr=GetCommaWordW(TextPtr,strArgName))==NULL)
      break;
    strArgName.Upper();
    if ( strArgName.At(0)==L'N')
    {
      unsigned int &ColumnType=ViewColumnTypes[ColumnCount];
      ColumnType=NAME_COLUMN;

      const wchar_t *Ptr = (const wchar_t*)strArgName+1;

      while ( *Ptr )
      {
        switch( *Ptr )
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
      if ( strArgName.At(0)==L'S' || strArgName.At(0)==L'P')
      {
        unsigned int &ColumnType=ViewColumnTypes[ColumnCount];
        ColumnType=(strArgName.At(0)==L'S') ? SIZE_COLUMN:PACKED_COLUMN;

        const wchar_t *Ptr = (const wchar_t*)strArgName+1;

        while ( *Ptr )
        {
          switch( *Ptr )
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
        if (wcsncmp(strArgName,L"DM",2)==0 || wcsncmp(strArgName,L"DC",2)==0 || wcsncmp(strArgName,L"DA",2)==0)
        {
          unsigned int &ColumnType=ViewColumnTypes[ColumnCount];

          switch(strArgName.At(1))
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

          while ( *Ptr )
          {
            switch( *Ptr )
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
          for (int I=0;I<sizeof(ColumnSymbol)/sizeof(ColumnSymbol[0]);I++)
            if (wcscmp(strArgName,ColumnSymbol[I])==0)
            {
              ViewColumnTypes[ColumnCount]=I;
              break;
            }
    }

  TextPtr=ColumnWidths;
  for (int I=0;I<ColumnCount;I++)
  {
    string strArgName;
    if ((TextPtr=GetCommaWordW(TextPtr,strArgName))==NULL)
      break;
    ViewColumnWidths[I]=_wtoi(strArgName);
  }
}


void FileList::ViewSettingsToText(unsigned int *ViewColumnTypes,
     int *ViewColumnWidths,int ColumnCount,string &strColumnTitles,
     string &strColumnWidths)
{
  strColumnTitles=L"";
  strColumnWidths=L"";

  for (int I=0;I<ColumnCount;I++)
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
    if (ColumnType==SIZE_COLUMN || ColumnType==PACKED_COLUMN)
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
    strColumnTitles += strType;

    wchar_t *lpwszWidth = strType.GetBuffer(20);

    _itow (ViewColumnWidths[I],lpwszWidth,10);

    strType.ReleaseBuffer();

    strColumnWidths += strType;

    if (I<ColumnCount-1)
    {
      strColumnTitles += L",";
      strColumnWidths += L",";
    }
  }
}
