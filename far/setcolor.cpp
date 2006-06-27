/*
setcolor.cpp

��������� ��������� ������

*/

/* Revision: 1.28 15.03.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "savescr.hpp"
#include "scrbuf.hpp"

static void SetItemColors(struct MenuDataEx *Items,int *PaletteItems,int Size,int TypeSub);
void GetColor(int PaletteIndex);
static VMenu *MenuToRedraw1=NULL,*MenuToRedraw2=NULL,*MenuToRedraw3=NULL;

static struct MenuDataEx ListItems[]=
{
  (const wchar_t *)MSetColorDialogListText,LIF_SELECTED,0,
  (const wchar_t *)MSetColorDialogListHighLight,0,0,
  (const wchar_t *)MSetColorDialogListSelectedText,0,0,
  (const wchar_t *)MSetColorDialogListSelectedHighLight,0,0,
  (const wchar_t *)MSetColorDialogListDisabled,0,0,
  (const wchar_t *)MSetColorDialogListBox,0,0,
  (const wchar_t *)MSetColorDialogListTitle,0,0,
  (const wchar_t *)MSetColorDialogListScrollBar,0,0,
};

// 0,1 - dialog,warn List
// 2,3 - dialog,warn Combobox
static int ListPaletteItems[4][8]=
{
  // Listbox
  { // normal
    COL_DIALOGLISTTEXT,
    COL_DIALOGLISTHIGHLIGHT,
    COL_DIALOGLISTSELECTEDTEXT,
    COL_DIALOGLISTSELECTEDHIGHLIGHT,
    COL_DIALOGLISTDISABLED,
    COL_DIALOGLISTBOX,
    COL_DIALOGLISTTITLE,
    COL_DIALOGLISTSCROLLBAR,
  },
  { // warn
    COL_WARNDIALOGLISTTEXT,
    COL_WARNDIALOGLISTHIGHLIGHT,
    COL_WARNDIALOGLISTSELECTEDTEXT,
    COL_WARNDIALOGLISTSELECTEDHIGHLIGHT,
    COL_WARNDIALOGLISTDISABLED,
    COL_WARNDIALOGLISTBOX,
    COL_WARNDIALOGLISTTITLE,
    COL_WARNDIALOGLISTSCROLLBAR,
  },
  // Combobox
  { // normal
    COL_DIALOGCOMBOTEXT,
    COL_DIALOGCOMBOHIGHLIGHT,
    COL_DIALOGCOMBOSELECTEDTEXT,
    COL_DIALOGCOMBOSELECTEDHIGHLIGHT,
    COL_DIALOGCOMBODISABLED,
    COL_DIALOGCOMBOBOX,
    COL_DIALOGCOMBOTITLE,
    COL_DIALOGCOMBOSCROLLBAR,
  },
  { // warn
    COL_WARNDIALOGCOMBOTEXT,
    COL_WARNDIALOGCOMBOHIGHLIGHT,
    COL_WARNDIALOGCOMBOSELECTEDTEXT,
    COL_WARNDIALOGCOMBOSELECTEDHIGHLIGHT,
    COL_WARNDIALOGCOMBODISABLED,
    COL_WARNDIALOGCOMBOBOX,
    COL_WARNDIALOGCOMBOTITLE,
    COL_WARNDIALOGCOMBOSCROLLBAR,
  },
};

void SetColors()
{
  struct MenuDataEx Groups[]=
  {
    (const wchar_t *)MSetColorPanel,LIF_SELECTED,0,
    (const wchar_t *)MSetColorDialog,0,0,
    (const wchar_t *)MSetColorWarning,0,0,
    (const wchar_t *)MSetColorMenu,0,0,
    (const wchar_t *)MSetColorHMenu,0,0,
    (const wchar_t *)MSetColorKeyBar,0,0,
    (const wchar_t *)MSetColorCommandLine,0,0,
    (const wchar_t *)MSetColorClock,0,0,
    (const wchar_t *)MSetColorViewer,0,0,
    (const wchar_t *)MSetColorEditor,0,0,
    (const wchar_t *)MSetColorHelp,0,0,
    L"",LIF_SEPARATOR,0,
    (const wchar_t *)MSetDefaultColors,0,0,
    (const wchar_t *)MSetBW,0,0,
  };

  struct MenuDataEx PanelItems[]=
  {
    (const wchar_t *)MSetColorPanelNormal,LIF_SELECTED,0,
    (const wchar_t *)MSetColorPanelSelected,0,0,
    (const wchar_t *)MSetColorPanelHighlightedInfo,0,0,
    (const wchar_t *)MSetColorPanelDragging,0,0,
    (const wchar_t *)MSetColorPanelBox,0,0,
    (const wchar_t *)MSetColorPanelNormalCursor,0,0,
    (const wchar_t *)MSetColorPanelSelectedCursor,0,0,
    (const wchar_t *)MSetColorPanelNormalTitle,0,0,
    (const wchar_t *)MSetColorPanelSelectedTitle,0,0,
    (const wchar_t *)MSetColorPanelColumnTitle,0,0,
    (const wchar_t *)MSetColorPanelTotalInfo,0,0,
    (const wchar_t *)MSetColorPanelSelectedInfo,0,0,
    (const wchar_t *)MSetColorPanelScrollbar,0,0,
    (const wchar_t *)MSetColorPanelScreensNumber,0,0,
  };
  int PanelPaletteItems[]={
    COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELINFOTEXT,
    COL_PANELDRAGTEXT,COL_PANELBOX,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR,
    COL_PANELTITLE,COL_PANELSELECTEDTITLE,COL_PANELCOLUMNTITLE,
    COL_PANELTOTALINFO,COL_PANELSELECTEDINFO,COL_PANELSCROLLBAR,
    COL_PANELSCREENSNUMBER
  };

  struct MenuDataEx DialogItems[]=
  {
    (const wchar_t *)MSetColorDialogNormal,LIF_SELECTED,0,
    (const wchar_t *)MSetColorDialogHighlighted,0,0,
    (const wchar_t *)MSetColorDialogDisabled,0,0,
    (const wchar_t *)MSetColorDialogBox,0,0,
    (const wchar_t *)MSetColorDialogBoxTitle,0,0,
    (const wchar_t *)MSetColorDialogHighlightedBoxTitle,0,0,
    (const wchar_t *)MSetColorDialogTextInput,0,0,
    (const wchar_t *)MSetColorDialogUnchangedTextInput,0,0,
    (const wchar_t *)MSetColorDialogSelectedTextInput,0,0,
    (const wchar_t *)MSetColorDialogEditDisabled,0,0,
    (const wchar_t *)MSetColorDialogButtons,0,0,
    (const wchar_t *)MSetColorDialogSelectedButtons,0,0,
    (const wchar_t *)MSetColorDialogHighlightedButtons,0,0,
    (const wchar_t *)MSetColorDialogSelectedHighlightedButtons,0,0,
    (const wchar_t *)MSetColorDialogListBoxControl,0,0,
    (const wchar_t *)MSetColorDialogComboBoxControl,0,0,
  };
  int DialogPaletteItems[]={
    COL_DIALOGTEXT,
    COL_DIALOGHIGHLIGHTTEXT,
    COL_DIALOGDISABLED,
    COL_DIALOGBOX,
    COL_DIALOGBOXTITLE,
    COL_DIALOGHIGHLIGHTBOXTITLE,
    COL_DIALOGEDIT,
    COL_DIALOGEDITUNCHANGED,
    COL_DIALOGEDITSELECTED,
    COL_DIALOGEDITDISABLED,
    COL_DIALOGBUTTON,
    COL_DIALOGSELECTEDBUTTON,
    COL_DIALOGHIGHLIGHTBUTTON,
    COL_DIALOGHIGHLIGHTSELECTEDBUTTON,
    0,
    2,
  };

  struct MenuDataEx WarnDialogItems[]=
  {
    (const wchar_t *)MSetColorDialogNormal,LIF_SELECTED,0,
    (const wchar_t *)MSetColorDialogHighlighted,0,0,
    (const wchar_t *)MSetColorDialogDisabled,0,0,
    (const wchar_t *)MSetColorDialogBox,0,0,
    (const wchar_t *)MSetColorDialogBoxTitle,0,0,
    (const wchar_t *)MSetColorDialogHighlightedBoxTitle,0,0,
    (const wchar_t *)MSetColorDialogTextInput,0,0,
    (const wchar_t *)MSetColorDialogUnchangedTextInput,0,0,
    (const wchar_t *)MSetColorDialogSelectedTextInput,0,0,
    (const wchar_t *)MSetColorDialogEditDisabled,0,0,
    (const wchar_t *)MSetColorDialogButtons,0,0,
    (const wchar_t *)MSetColorDialogSelectedButtons,0,0,
    (const wchar_t *)MSetColorDialogHighlightedButtons,0,0,
    (const wchar_t *)MSetColorDialogSelectedHighlightedButtons,0,0,
    (const wchar_t *)MSetColorDialogListBoxControl,0,0,
    (const wchar_t *)MSetColorDialogComboBoxControl,0,0,
  };
  int WarnDialogPaletteItems[]={
    COL_WARNDIALOGTEXT,
    COL_WARNDIALOGHIGHLIGHTTEXT,
    COL_WARNDIALOGDISABLED,
    COL_WARNDIALOGBOX,
    COL_WARNDIALOGBOXTITLE,
    COL_WARNDIALOGHIGHLIGHTBOXTITLE,
    COL_WARNDIALOGEDIT,
    COL_WARNDIALOGEDITUNCHANGED,
    COL_WARNDIALOGEDITSELECTED,
    COL_WARNDIALOGEDITDISABLED,
    COL_WARNDIALOGBUTTON,
    COL_WARNDIALOGSELECTEDBUTTON,
    COL_WARNDIALOGHIGHLIGHTBUTTON,
    COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON,
    1,
    3,
  };

    /* $ 29.06.2000 SVS
      ����� ����� ��� ��������� ������ Menu Scrollbar
    */
  struct MenuDataEx MenuItems[]=
  {
    (const wchar_t *)MSetColorMenuNormal,LIF_SELECTED,0,
    (const wchar_t *)MSetColorMenuSelected,0,0,
    (const wchar_t *)MSetColorMenuHighlighted,0,0,
    (const wchar_t *)MSetColorMenuSelectedHighlighted,0,0,
    (const wchar_t *)MSetColorMenuDisabled,0,0,
    (const wchar_t *)MSetColorMenuBox,0,0,
    (const wchar_t *)MSetColorMenuTitle,0,0,
    (const wchar_t *)MSetColorMenuScrollBar,0,0,
  };
  int MenuPaletteItems[]={
    COL_MENUTEXT,COL_MENUSELECTEDTEXT,COL_MENUHIGHLIGHT,
    COL_MENUSELECTEDHIGHLIGHT,COL_MENUDISABLEDTEXT,
    COL_MENUBOX,COL_MENUTITLE,COL_MENUSCROLLBAR,
    /* SVS $ */
  };

  struct MenuDataEx HMenuItems[]=
  {
    (const wchar_t *)MSetColorHMenuNormal,LIF_SELECTED,0,
    (const wchar_t *)MSetColorHMenuSelected,0,0,
    (const wchar_t *)MSetColorHMenuHighlighted,0,0,
    (const wchar_t *)MSetColorHMenuSelectedHighlighted,0,0,
  };
  int HMenuPaletteItems[]={
    COL_HMENUTEXT,COL_HMENUSELECTEDTEXT,COL_HMENUHIGHLIGHT,
    COL_HMENUSELECTEDHIGHLIGHT
  };

  struct MenuDataEx KeyBarItems[]=
  {
    (const wchar_t *)MSetColorKeyBarNumbers,LIF_SELECTED,0,
    (const wchar_t *)MSetColorKeyBarNames,0,0,
    (const wchar_t *)MSetColorKeyBarBackground,0,0,
  };
  int KeyBarPaletteItems[]={
    COL_KEYBARNUM,COL_KEYBARTEXT,COL_KEYBARBACKGROUND
  };

  struct MenuDataEx CommandLineItems[]=
  {
    (const wchar_t *)MSetColorCommandLineNormal,LIF_SELECTED,0,
    (const wchar_t *)MSetColorCommandLineSelected,0,0,
    (const wchar_t *)MSetColorCommandLinePrefix,0,0
  };
  int CommandLinePaletteItems[]={
    COL_COMMANDLINE,COL_COMMANDLINESELECTED,COL_COMMANDLINEPREFIX
  };

  struct MenuDataEx ClockItems[]=
  {
    (const wchar_t *)MSetColorClockNormal,LIF_SELECTED,0,
    (const wchar_t *)MSetColorClockNormalEditor,0,0,
    (const wchar_t *)MSetColorClockNormalViewer,0,0,
  };
  int ClockPaletteItems[]={
    COL_CLOCK,
    COL_EDITORCLOCK,COL_VIEWERCLOCK,
  };

  /* $ 18.07.2000 tran
     ����� ���� ��� MenuScrollbar*/
  struct MenuDataEx ViewerItems[]=
  {
    (const wchar_t *)MSetColorViewerNormal,LIF_SELECTED,0,
    (const wchar_t *)MSetColorViewerSelected,0,0,
    (const wchar_t *)MSetColorViewerStatus,0,0,
    (const wchar_t *)MSetColorViewerArrows,0,0,
    (const wchar_t *)MSetColorViewerScrollbar,0,0
  };
  int ViewerPaletteItems[]={
    COL_VIEWERTEXT,COL_VIEWERSELECTEDTEXT,COL_VIEWERSTATUS,COL_VIEWERARROWS,COL_VIEWERSCROLLBAR
  };
  /* tran 18.07.2000 $ */


  struct MenuDataEx EditorItems[]=
  {
    (const wchar_t *)MSetColorEditorNormal,LIF_SELECTED,0,
    (const wchar_t *)MSetColorEditorSelected,0,0,
    (const wchar_t *)MSetColorEditorStatus,0,0,
  };
  int EditorPaletteItems[]={
    COL_EDITORTEXT,COL_EDITORSELECTEDTEXT,COL_EDITORSTATUS
  };

  struct MenuDataEx HelpItems[]=
  {
    (const wchar_t *)MSetColorHelpNormal,LIF_SELECTED,0,
    (const wchar_t *)MSetColorHelpHighlighted,0,0,
    (const wchar_t *)MSetColorHelpReference,0,0,
    (const wchar_t *)MSetColorHelpSelectedReference,0,0,
    (const wchar_t *)MSetColorHelpBox,0,0,
    (const wchar_t *)MSetColorHelpBoxTitle,0,0,
    (const wchar_t *)MSetColorHelpScrollbar,0,0,
  };
  int HelpPaletteItems[]={
    COL_HELPTEXT,COL_HELPHIGHLIGHTTEXT,COL_HELPTOPIC,COL_HELPSELECTEDTOPIC,
    COL_HELPBOX,COL_HELPBOXTITLE,COL_HELPSCROLLBAR
  };

  {
    int GroupsCode;
    VMenu GroupsMenu(UMSG(MSetColorGroupsTitle),Groups,sizeof(Groups)/sizeof(Groups[0]),TRUE,0);
    MenuToRedraw1=&GroupsMenu;
    while (1)
    {
      GroupsMenu.SetPosition(2,1,0,0);
      /* $ 16.06.2001 KM
         ! ���������� WRAPMODE � ����.
      */
      /* $ 13.04.2002 KM
        - �������� VMENU_NOTCHANGE, ������� ������������� ������
          ���� �� ������ ��� AltF9 � ������� �������������� ������.
      */
      GroupsMenu.SetFlags(VMENU_WRAPMODE|VMENU_NOTCHANGE);
      /* KM $ */
      /* KM $ */
      GroupsMenu.ClearDone();
      GroupsMenu.Process();
      if ((GroupsCode=GroupsMenu.Modal::GetExitCode())<0)
        break;

      if (GroupsCode==12)
      {
        //                   ���� sizeof(Palette)
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
          SetItemColors(PanelItems,PanelPaletteItems,sizeof(PanelItems)/sizeof(PanelItems[0]),0);
          break;
        case 1:
          SetItemColors(DialogItems,DialogPaletteItems,sizeof(DialogItems)/sizeof(DialogItems[0]),1);
          break;
        case 2:
          SetItemColors(WarnDialogItems,WarnDialogPaletteItems,sizeof(WarnDialogItems)/sizeof(WarnDialogItems[0]),1);
          break;
        case 3:
          SetItemColors(MenuItems,MenuPaletteItems,sizeof(MenuItems)/sizeof(MenuItems[0]),0);
          break;
        case 4:
          SetItemColors(HMenuItems,HMenuPaletteItems,sizeof(HMenuItems)/sizeof(HMenuItems[0]),0);
          break;
        case 5:
          SetItemColors(KeyBarItems,KeyBarPaletteItems,sizeof(KeyBarItems)/sizeof(KeyBarItems[0]),0);
          break;
        case 6:
          SetItemColors(CommandLineItems,CommandLinePaletteItems,sizeof(CommandLineItems)/sizeof(CommandLineItems[0]),0);
          break;
        case 7:
          SetItemColors(ClockItems,ClockPaletteItems,sizeof(ClockItems)/sizeof(ClockItems[0]),0);
          break;
        case 8:
          SetItemColors(ViewerItems,ViewerPaletteItems,sizeof(ViewerItems)/sizeof(ViewerItems[0]),0);
          break;
        case 9:
          SetItemColors(EditorItems,EditorPaletteItems,sizeof(EditorItems)/sizeof(EditorItems[0]),0);
          break;
        case 10:
          SetItemColors(HelpItems,HelpPaletteItems,sizeof(HelpItems)/sizeof(HelpItems[0]),0);
          break;
      }
    }
  }
  CtrlObject->Cp()->SetScreenPosition();
}


static void SetItemColors(struct MenuDataEx *Items,int *PaletteItems,int Size,int TypeSub)
{
  int ItemsCode;

  VMenu ItemsMenu(UMSG(MSetColorItemsTitle),Items,Size,TRUE,0);
  if(TypeSub == 2)
    MenuToRedraw3=&ItemsMenu;
  else
    MenuToRedraw2=&ItemsMenu;

  while (1)
  {
    ItemsMenu.SetPosition(17-(TypeSub == 2?7:0),5+(TypeSub == 2?2:0),0,0);
    /* $ 09.04.2002 KM
      - �������� VMENU_NOTCHANGE, ������� ������������� ������
        ���� �� ������ ��� AltF9 � ������� �������������� ������.
    */
    ItemsMenu.SetFlags(VMENU_WRAPMODE|VMENU_NOTCHANGE);
    /* KM $ */
    ItemsMenu.ClearDone();
    ItemsMenu.Process();
    if ((ItemsCode=ItemsMenu.Modal::GetExitCode())<0)
      break;

// 0,1 - dialog,warn List
// 2,3 - dialog,warn Combobox
    if(TypeSub == 1 && PaletteItems[ItemsCode] < 4)
    {
      SetItemColors(ListItems,ListPaletteItems[PaletteItems[ItemsCode]],sizeof(ListItems)/sizeof(ListItems[0]),2);
      MenuToRedraw3=NULL;
    }
    else
      GetColor(PaletteItems[ItemsCode]);
  }
}


void GetColor(int PaletteIndex)
{
  unsigned int NewColor=Palette[PaletteIndex-COL_FIRSTPALETTECOLOR];
  if (GetColorDialog(NewColor))
  {
    Palette[PaletteIndex-COL_FIRSTPALETTECOLOR]=NewColor;
    ScrBuf.Lock(); // �������� ������ ����������
    if(MenuToRedraw3)
      MenuToRedraw3->Hide();
    MenuToRedraw2->Hide(); // �����
    MenuToRedraw1->Hide();
    FrameManager->RefreshFrame(); // ��������
    FrameManager->PluginCommit(); // ��������.
    MenuToRedraw1->SetColors();
    MenuToRedraw1->Show(); // �����
    MenuToRedraw2->SetColors();
    MenuToRedraw2->Show();
    if(MenuToRedraw3)
    {
      MenuToRedraw3->SetColors();
      MenuToRedraw3->Show();
    }
    if(Opt.Clock)
      ShowTime(1);
    ScrBuf.Unlock(); // ��������� ����������
    FrameManager->PluginCommit(); // ��������.
  }
}

/* $ 18.05.2001 DJ
   ��������� ��������� ����� �������� � �������-���������� �������
*/

static long WINAPI GetColorDlgProc(HANDLE hDlg, int Msg, int Param1, long Param2)
{
  switch (Msg)
  {
    case DN_CTLCOLORDLGITEM:
      if(Param1 >= 35 && Param1 <= 37)
      {
        int *CurColor=(int *)Dialog::SendDlgMessage (hDlg,DM_GETDLGDATA,0,0);
        return (Param2&0xFFFFFF00U)|((*CurColor)&0xFF);
      }
      break;

    case DN_BTNCLICK:
      if(Param1 >= 2 && Param1 <= 34)
      {
        FarDialogItem DlgItem;
        int NewColor;
        int *CurColor = (int *) Dialog::SendDlgMessage (hDlg, DM_GETDLGDATA, 0, 0);

        Dialog::SendDlgMessage (hDlg, DM_GETDLGITEM, Param1, (long) &DlgItem);

        NewColor=*CurColor;
        if(Param1 >= 2 && Param1 <= 17) // Fore
        {
          NewColor&=~0x0F;
          NewColor|=(DlgItem.Flags & B_MASK)>>4;
        }
        if(Param1 >= 19 && Param1 <= 34) // Back
        {
          NewColor&=~0xF0;
          NewColor|=DlgItem.Flags & B_MASK;
        }

        if (NewColor!=*CurColor)
          *CurColor=NewColor;
        return TRUE;
      }
      break;
  }
  return Dialog::DefDlgProc (hDlg, Msg, Param1, Param2);
}

/* DJ $ */

int GetColorDialog(unsigned int &Color,bool bCentered)
{
  static struct DialogDataEx ColorDlgData[]={
    /*   0 */ DI_DOUBLEBOX,3,1,35,13,0,0,0,0,(const wchar_t *)MSetColorTitle,
    /*   1 */ DI_SINGLEBOX,5,2,18,7,0,0,0,0,(const wchar_t *)MSetColorForeground,
    /*   2 */ DI_RADIOBUTTON,6,3,0,0,0,0,F_LIGHTGRAY|B_BLACK|DIF_GROUP|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*   3 */ DI_RADIOBUTTON,6,4,0,0,0,0,F_BLACK|B_RED|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*   4 */ DI_RADIOBUTTON,6,5,0,0,0,0,F_LIGHTGRAY|B_DARKGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*   5 */ DI_RADIOBUTTON,6,6,0,0,0,0,F_BLACK|B_LIGHTRED|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*   6 */ DI_RADIOBUTTON,9,3,0,0,0,0,F_LIGHTGRAY|B_BLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*   7 */ DI_RADIOBUTTON,9,4,0,0,0,0,F_BLACK|B_MAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*   8 */ DI_RADIOBUTTON,9,5,0,0,0,0,F_BLACK|B_LIGHTBLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*   9 */ DI_RADIOBUTTON,9,6,0,0,0,0,F_BLACK|B_LIGHTMAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  10 */ DI_RADIOBUTTON,12,3,0,0,0,0,F_BLACK|B_GREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  11 */ DI_RADIOBUTTON,12,4,0,0,0,0,F_BLACK|B_BROWN|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  12 */ DI_RADIOBUTTON,12,5,0,0,0,0,F_BLACK|B_LIGHTGREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  13 */ DI_RADIOBUTTON,12,6,0,0,0,0,F_BLACK|B_YELLOW|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  14 */ DI_RADIOBUTTON,15,3,0,0,0,0,F_BLACK|B_CYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  15 */ DI_RADIOBUTTON,15,4,0,0,0,0,F_BLACK|B_LIGHTGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  16 */ DI_RADIOBUTTON,15,5,0,0,0,0,F_BLACK|B_LIGHTCYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  17 */ DI_RADIOBUTTON,15,6,0,0,0,0,F_BLACK|B_WHITE|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  18 */ DI_SINGLEBOX,20,2,33,7,0,0,0,0,(const wchar_t *)MSetColorBackground,
    /*  19 */ DI_RADIOBUTTON,21,3,0,0,0,0,F_LIGHTGRAY|B_BLACK|DIF_GROUP|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  20 */ DI_RADIOBUTTON,21,4,0,0,0,0,F_BLACK|B_RED|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  21 */ DI_RADIOBUTTON,21,5,0,0,0,0,F_LIGHTGRAY|B_DARKGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  22 */ DI_RADIOBUTTON,21,6,0,0,0,0,F_BLACK|B_LIGHTRED|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  23 */ DI_RADIOBUTTON,24,3,0,0,0,0,F_LIGHTGRAY|B_BLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  24 */ DI_RADIOBUTTON,24,4,0,0,0,0,F_BLACK|B_MAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  25 */ DI_RADIOBUTTON,24,5,0,0,0,0,F_BLACK|B_LIGHTBLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  26 */ DI_RADIOBUTTON,24,6,0,0,0,0,F_BLACK|B_LIGHTMAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  27 */ DI_RADIOBUTTON,27,3,0,0,0,0,F_BLACK|B_GREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  28 */ DI_RADIOBUTTON,27,4,0,0,0,0,F_BLACK|B_BROWN|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  29 */ DI_RADIOBUTTON,27,5,0,0,0,0,F_BLACK|B_LIGHTGREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  30 */ DI_RADIOBUTTON,27,6,0,0,0,0,F_BLACK|B_YELLOW|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  31 */ DI_RADIOBUTTON,30,3,0,0,0,0,F_BLACK|B_CYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  32 */ DI_RADIOBUTTON,30,4,0,0,0,0,F_BLACK|B_LIGHTGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  33 */ DI_RADIOBUTTON,30,5,0,0,0,0,F_BLACK|B_LIGHTCYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  34 */ DI_RADIOBUTTON,30,6,0,0,0,0,F_BLACK|B_WHITE|DIF_SETCOLOR|DIF_MOVESELECT,0,L"",
    /*  35 */ DI_TEXT,5,8,0,0,0,0,DIF_SETCOLOR,0,(const wchar_t *)MSetColorSample,
    /*  36 */ DI_TEXT,5,9,0,0,0,0,DIF_SETCOLOR,0,(const wchar_t *)MSetColorSample,
    /*  37 */ DI_TEXT,5,10,0,0,0,0,DIF_SETCOLOR,0,(const wchar_t *)MSetColorSample,
    /*  38 */ DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
    /*  39 */ DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MSetColorSet,
    /*  40 */ DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MSetColorCancel,

  };
  MakeDialogItemsEx(ColorDlgData,ColorDlg);
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
    /* $ 18.05.2001 DJ
       ��������� ��������� ����� �������� � �������-���������� �������
    */
    //SaveScreen SaveScr;
    Dialog Dlg(ColorDlg,sizeof(ColorDlg)/sizeof(ColorDlg[0]), GetColorDlgProc, (long) &CurColor);
    if (bCentered)
      Dlg.SetPosition(-1,-1,39,15);
    else
      Dlg.SetPosition(37,2,75,16);
    Dlg.Process();
    /* DJ $ */
    ExitCode=Dlg.GetExitCode();
  }

  if (ExitCode==39)
  {
    Color=CurColor;
    return(TRUE);
  }
  return(FALSE);
}
