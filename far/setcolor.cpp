/*
setcolor.cpp

Установка фаровских цветов

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define STRICT

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#include <windows.h>
#endif

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __FARLANG_HPP__
#include "lang.hpp"
#endif
#ifndef __COLOROS_HPP__
#include "colors.hpp"
#endif
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif
#ifndef __PLUGIN_HPP__
#include "plugin.hpp"
#endif
#ifndef __CLASSES_HPP__
#include "classes.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif
#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif


static void SetItemColors(struct MenuData *Items,int *PaletteItems,int Size);
void GetColor(int PaletteIndex);
static VMenu *MenuToRedraw1,*MenuToRedraw2;

void SetColors()
{
  struct MenuData Groups[]=
  {
    (char *)MSetColorPanel,1,0,0,
    (char *)MSetColorDialog,0,0,0,
    (char *)MSetColorWarning,0,0,0,
    (char *)MSetColorMenu,0,0,0,
    (char *)MSetColorHMenu,0,0,0,
    (char *)MSetColorKeyBar,0,0,0,
    (char *)MSetColorCommandLine,0,0,0,
    (char *)MSetColorClock,0,0,0,
    (char *)MSetColorViewer,0,0,0,
    (char *)MSetColorEditor,0,0,0,
    (char *)MSetColorHelp,0,0,0,
    "",0,0,1,
    (char *)MSetDefaultColors,0,0,0,
    (char *)MSetBW,0,0,0
  };

  struct MenuData PanelItems[]=
  {
    (char *)MSetColorPanelNormal,1,0,0,
    (char *)MSetColorPanelSelected,0,0,0,
    (char *)MSetColorPanelHighlightedInfo,0,0,0,
    (char *)MSetColorPanelDragging,0,0,0,
    (char *)MSetColorPanelBox,0,0,0,
    (char *)MSetColorPanelNormalCursor,0,0,0,
    (char *)MSetColorPanelSelectedCursor,0,0,0,
    (char *)MSetColorPanelNormalTitle,0,0,0,
    (char *)MSetColorPanelSelectedTitle,0,0,0,
    (char *)MSetColorPanelColumnTitle,0,0,0,
    (char *)MSetColorPanelTotalInfo,0,0,0,
    (char *)MSetColorPanelSelectedInfo,0,0,0,
    (char *)MSetColorPanelScrollbar,0,0,0,
    (char *)MSetColorPanelScreensNumber,0,0,0,
  };
  int PanelPaletteItems[]={
    COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELINFOTEXT,
    COL_PANELDRAGTEXT,COL_PANELBOX,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR,
    COL_PANELTITLE,COL_PANELSELECTEDTITLE,COL_PANELCOLUMNTITLE,
    COL_PANELTOTALINFO,COL_PANELSELECTEDINFO,COL_PANELSCROLLBAR,
    COL_PANELSCREENSNUMBER
  };

  struct MenuData DialogItems[]=
  {
    (char *)MSetColorDialogNormal,1,0,0,
    (char *)MSetColorDialogHighlighted,0,0,0,
    (char *)MSetColorDialogBox,0,0,0,
    (char *)MSetColorDialogBoxTitle,0,0,0,
    (char *)MSetColorDialogHighlightedBoxTitle,0,0,0,
    (char *)MSetColorDialogTextInput,0,0,0,
    (char *)MSetColorDialogUnchangedTextInput,0,0,0,
    (char *)MSetColorDialogSelectedTextInput,0,0,0,
    (char *)MSetColorDialogButtons,0,0,0,
    (char *)MSetColorDialogSelectedButtons,0,0,0,
    (char *)MSetColorDialogHighlightedButtons,0,0,0,
    (char *)MSetColorDialogSelectedHighlightedButtons,0,0,0,
    (char *)MSetColorDialogListText,0,0,0,
    (char *)MSetColorDialogSelectedListText,0,0,0
  };
  int DialogPaletteItems[]={
    COL_DIALOGTEXT,COL_DIALOGHIGHLIGHTTEXT,COL_DIALOGBOX,COL_DIALOGBOXTITLE,
    COL_DIALOGHIGHLIGHTBOXTITLE,COL_DIALOGEDIT,COL_DIALOGEDITUNCHANGED,
    COL_DIALOGEDITSELECTED,COL_DIALOGBUTTON,COL_DIALOGSELECTEDBUTTON,
    COL_DIALOGHIGHLIGHTBUTTON,COL_DIALOGHIGHLIGHTSELECTEDBUTTON,
    COL_DIALOGMENUTEXT,COL_DIALOGMENUSELECTEDTEXT,COL_DIALOGMENUHIGHLIGHT,
    COL_DIALOGMENUSELECTEDHIGHLIGHT
  };

  struct MenuData WarnDialogItems[]=
  {
    (char *)MSetColorWarningNormal,1,0,0,
    (char *)MSetColorWarningHighlighted,0,0,0,
    (char *)MSetColorWarningBox,0,0,0,
    (char *)MSetColorWarningBoxTitle,0,0,0,
    (char *)MSetColorWarningHighlightedBoxTitle,0,0,0,
    (char *)MSetColorWarningTextInput,0,0,0,
    (char *)MSetColorWarningButtons,0,0,0,
    (char *)MSetColorWarningSelectedButtons,0,0,0,
    (char *)MSetColorWarningHighlightedButtons,0,0,0,
    (char *)MSetColorWarningSelectedHighlightedButtons,0,0,0
  };
  int WarnDialogPaletteItems[]={
    COL_WARNDIALOGTEXT,COL_WARNDIALOGHIGHLIGHTTEXT,COL_WARNDIALOGBOX,
    COL_WARNDIALOGBOXTITLE,COL_WARNDIALOGHIGHLIGHTBOXTITLE,
    COL_WARNDIALOGEDIT,COL_WARNDIALOGBUTTON,COL_WARNDIALOGSELECTEDBUTTON,
    COL_WARNDIALOGHIGHLIGHTBUTTON,COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON
  };

  struct MenuData MenuItems[]=
  {
    (char *)MSetColorMenuNormal,1,0,0,
    (char *)MSetColorMenuSelected,0,0,0,
    (char *)MSetColorMenuHighlighted,0,0,0,
    (char *)MSetColorMenuSelectedHighlighted,0,0,0,
    (char *)MSetColorMenuBox,0,0,0,
    (char *)MSetColorMenuTitle,0,0,0
  };
  int MenuPaletteItems[]={
    COL_MENUTEXT,COL_MENUSELECTEDTEXT,COL_MENUHIGHLIGHT,
    COL_MENUSELECTEDHIGHLIGHT,COL_MENUBOX,COL_MENUTITLE,
  };

  struct MenuData HMenuItems[]=
  {
    (char *)MSetColorHMenuNormal,1,0,0,
    (char *)MSetColorHMenuSelected,0,0,0,
    (char *)MSetColorHMenuHighlighted,0,0,0,
    (char *)MSetColorHMenuSelectedHighlighted,0,0,0
  };
  int HMenuPaletteItems[]={
    COL_HMENUTEXT,COL_HMENUSELECTEDTEXT,COL_HMENUHIGHLIGHT,
    COL_HMENUSELECTEDHIGHLIGHT
  };

  struct MenuData KeyBarItems[]=
  {
    (char *)MSetColorKeyBarNumbers,1,0,0,
    (char *)MSetColorKeyBarNames,0,0,0,
    (char *)MSetColorKeyBarBackground,0,0,0
  };
  int KeyBarPaletteItems[]={
    COL_KEYBARNUM,COL_KEYBARTEXT,COL_KEYBARBACKGROUND
  };

  struct MenuData CommandLineItems[]=
  {
    (char *)MSetColorCommandLineNormal,1,0,0,
    (char *)MSetColorCommandLineSelected,0,0,0
  };
  int CommandLinePaletteItems[]={
    COL_COMMANDLINE,COL_COMMANDLINESELECTED
  };

  struct MenuData ClockItems[]=
  {
    (char *)MSetColorClockNormal,1,0,0
  };
  int ClockPaletteItems[]={
    COL_CLOCK
  };

  struct MenuData ViewerItems[]=
  {
    (char *)MSetColorViewerNormal,1,0,0,
    (char *)MSetColorViewerSelected,0,0,0,
    (char *)MSetColorViewerStatus,0,0,0,
    (char *)MSetColorViewerArrows,0,0,0
  };
  int ViewerPaletteItems[]={
    COL_VIEWERTEXT,COL_VIEWERSELECTEDTEXT,COL_VIEWERSTATUS,COL_VIEWERARROWS
  };

  struct MenuData EditorItems[]=
  {
    (char *)MSetColorEditorNormal,1,0,0,
    (char *)MSetColorEditorSelected,0,0,0,
    (char *)MSetColorEditorStatus,0,0,0
  };
  int EditorPaletteItems[]={
    COL_EDITORTEXT,COL_EDITORSELECTEDTEXT,COL_EDITORSTATUS
  };

  struct MenuData HelpItems[]=
  {
    (char *)MSetColorHelpNormal,1,0,0,
    (char *)MSetColorHelpHighlighted,0,0,0,
    (char *)MSetColorHelpReference,0,0,0,
    (char *)MSetColorHelpSelectedReference,0,0,0,
    (char *)MSetColorHelpBox,0,0,0,
    (char *)MSetColorHelpBoxTitle,0,0,0,
    (char *)MSetColorHelpScrollbar,0,0,0,
  };
  int HelpPaletteItems[]={
    COL_HELPTEXT,COL_HELPHIGHLIGHTTEXT,COL_HELPTOPIC,COL_HELPSELECTEDTOPIC,
    COL_HELPBOX,COL_HELPBOXTITLE,COL_HELPSCROLLBAR
  };

  {
    int GroupsCode;
    VMenu GroupsMenu(MSG(MSetColorGroupsTitle),Groups,sizeof(Groups)/sizeof(Groups[0]),0);
    MenuToRedraw1=&GroupsMenu;
    GroupsMenu.SetPosition(2,1,0,0);
    while (1)
    {
      GroupsMenu.ClearDone();
      GroupsMenu.Process();
      if ((GroupsCode=GroupsMenu.GetExitCode())<0)
        break;

      if (GroupsCode==12)
      {
        //                   было sizeof(Palette)
        memcpy(Palette,DefaultPalette,SizeArrayPalette);
        break;
      }
      if (GroupsCode==13)
      {
        memcpy(Palette,BlackPalette,SizeArrayPalette);
        break;
      }
      switch(GroupsCode)
      {
        case 0:
          SetItemColors(PanelItems,PanelPaletteItems,sizeof(PanelItems)/sizeof(PanelItems[0]));
          break;
        case 1:
          SetItemColors(DialogItems,DialogPaletteItems,sizeof(DialogItems)/sizeof(DialogItems[0]));
          break;
        case 2:
          SetItemColors(WarnDialogItems,WarnDialogPaletteItems,sizeof(WarnDialogItems)/sizeof(WarnDialogItems[0]));
          break;
        case 3:
          SetItemColors(MenuItems,MenuPaletteItems,sizeof(MenuItems)/sizeof(MenuItems[0]));
          break;
        case 4:
          SetItemColors(HMenuItems,HMenuPaletteItems,sizeof(HMenuItems)/sizeof(HMenuItems[0]));
          break;
        case 5:
          SetItemColors(KeyBarItems,KeyBarPaletteItems,sizeof(KeyBarItems)/sizeof(KeyBarItems[0]));
          break;
        case 6:
          SetItemColors(CommandLineItems,CommandLinePaletteItems,sizeof(CommandLineItems)/sizeof(CommandLineItems[0]));
          break;
        case 7:
          SetItemColors(ClockItems,ClockPaletteItems,sizeof(ClockItems)/sizeof(ClockItems[0]));
          break;
        case 8:
          SetItemColors(ViewerItems,ViewerPaletteItems,sizeof(ViewerItems)/sizeof(ViewerItems[0]));
          break;
        case 9:
          SetItemColors(EditorItems,EditorPaletteItems,sizeof(EditorItems)/sizeof(EditorItems[0]));
          break;
        case 10:
          SetItemColors(HelpItems,HelpPaletteItems,sizeof(HelpItems)/sizeof(HelpItems[0]));
          break;
      }
    }
  }
  CtrlObject->SetScreenPositions();
}


void SetItemColors(struct MenuData *Items,int *PaletteItems,int Size)
{
  int ItemsCode;

  VMenu ItemsMenu(MSG(MSetColorItemsTitle),Items,Size,0);
  MenuToRedraw2=&ItemsMenu;
  while (1)
  {
    ItemsMenu.SetPosition(17,5,0,0);
    ItemsMenu.ClearDone();
    ItemsMenu.Process();
    if ((ItemsCode=ItemsMenu.GetExitCode())<0)
      break;
    GetColor(PaletteItems[ItemsCode]);
  }
}


void GetColor(int PaletteIndex)
{
  unsigned int NewColor=Palette[PaletteIndex-COL_FIRSTPALETTECOLOR];
  if (GetColorDialog(NewColor))
  {
    Palette[PaletteIndex-COL_FIRSTPALETTECOLOR]=NewColor;
    MenuToRedraw2->Hide();
    MenuToRedraw1->Hide();
    CtrlObject->SetScreenPositions();
    MenuToRedraw1->Show();
    MenuToRedraw2->Show();
  }
}


int GetColorDialog(unsigned int &Color)
{
  static struct DialogData ColorDlgData[]={
    DI_DOUBLEBOX,3,1,35,13,0,0,0,0,(char *)MSetColorTitle,
    DI_SINGLEBOX,5,2,18,7,0,0,0,0,(char *)MSetColorForeground,
    DI_RADIOBUTTON,6,3,0,0,0,0,F_LIGHTGRAY|B_BLACK|DIF_GROUP|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,6,4,0,0,0,0,F_BLACK|B_RED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,6,5,0,0,0,0,F_LIGHTGRAY|B_DARKGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,6,6,0,0,0,0,F_BLACK|B_LIGHTRED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,9,3,0,0,0,0,F_LIGHTGRAY|B_BLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,9,4,0,0,0,0,F_BLACK|B_MAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,9,5,0,0,0,0,F_BLACK|B_LIGHTBLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,9,6,0,0,0,0,F_BLACK|B_LIGHTMAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,12,3,0,0,0,0,F_BLACK|B_GREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,12,4,0,0,0,0,F_BLACK|B_BROWN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,12,5,0,0,0,0,F_BLACK|B_LIGHTGREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,12,6,0,0,0,0,F_BLACK|B_YELLOW|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,15,3,0,0,0,0,F_BLACK|B_CYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,15,4,0,0,0,0,F_BLACK|B_LIGHTGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,15,5,0,0,0,0,F_BLACK|B_LIGHTCYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,15,6,0,0,0,0,F_BLACK|B_WHITE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_SINGLEBOX,20,2,33,7,0,0,0,0,(char *)MSetColorBackground,
    DI_RADIOBUTTON,21,3,0,0,0,0,F_LIGHTGRAY|B_BLACK|DIF_GROUP|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,21,4,0,0,0,0,F_BLACK|B_RED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,21,5,0,0,0,0,F_LIGHTGRAY|B_DARKGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,21,6,0,0,0,0,F_BLACK|B_LIGHTRED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,24,3,0,0,0,0,F_LIGHTGRAY|B_BLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,24,4,0,0,0,0,F_BLACK|B_MAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,24,5,0,0,0,0,F_BLACK|B_LIGHTBLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,24,6,0,0,0,0,F_BLACK|B_LIGHTMAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,27,3,0,0,0,0,F_BLACK|B_GREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,27,4,0,0,0,0,F_BLACK|B_BROWN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,27,5,0,0,0,0,F_BLACK|B_LIGHTGREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,27,6,0,0,0,0,F_BLACK|B_YELLOW|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,30,3,0,0,0,0,F_BLACK|B_CYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,30,4,0,0,0,0,F_BLACK|B_LIGHTGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,30,5,0,0,0,0,F_BLACK|B_LIGHTCYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_RADIOBUTTON,30,6,0,0,0,0,F_BLACK|B_WHITE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    DI_TEXT,5,8,0,0,0,0,DIF_SETCOLOR,0,(char *)MSetColorSample,
    DI_TEXT,5,9,0,0,0,0,DIF_SETCOLOR,0,(char *)MSetColorSample,
    DI_TEXT,5,10,0,0,0,0,DIF_SETCOLOR,0,(char *)MSetColorSample,
    DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,1,(char *)MSetColorSet,
    DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,0,(char *)MSetColorCancel,
  };
  MakeDialogItems(ColorDlgData,ColorDlg);
  int ExitCode,I,CurColor=Color;

  for (I=2;I<18;I++)
    if (((ColorDlg[I].Flags & B_MASK)>>4)==(Color & F_MASK))
    {
      ColorDlg[I].Selected=ColorDlg[I].Focus=1;
      break;
    }
  for (I=19;I<35;I++)
    if ((ColorDlg[I].Flags & B_MASK)==(Color & B_MASK))
    {
      ColorDlg[I].Selected=1;
      break;
    }

  for (I=35;I<38;I++)
    ColorDlg[I].Flags=(ColorDlg[I].Flags & ~DIF_COLORMASK) | Color;

  {
    Dialog Dlg(ColorDlg,sizeof(ColorDlg)/sizeof(ColorDlg[0]));
    Dlg.SetPosition(37,2,75,16);
    Dlg.Show();
    while (!Dlg.Done())
    {
      int NewColor;
      Dlg.ReadInput();
      Dlg.ProcessInput();
      for (I=2;I<18;I++)
        if (ColorDlg[I].Selected)
        {
          NewColor=(ColorDlg[I].Flags & B_MASK)>>4;
          break;
        }
      for (I=19;I<35;I++)
        if (ColorDlg[I].Selected)
        {
          NewColor|=ColorDlg[I].Flags & B_MASK;
          break;
        }
      if (NewColor!=CurColor)
      {
        CurColor=NewColor;
        for (I=35;I<38;I++)
          ColorDlg[I].Flags=(ColorDlg[I].Flags & ~DIF_COLORMASK) | CurColor;
        Dlg.FastShow();
      }
    }
    ExitCode=Dlg.GetExitCode();
  }
  if (ExitCode==39)
  {
    Color=CurColor;
    return(TRUE);
  }
  return(FALSE);
}


