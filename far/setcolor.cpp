/*
setcolor.cpp

Установка фаровских цветов

*/

/* Revision: 1.23 07.10.2002 $ */

/*
Modify:
  07.10.2002 SVS
    ! Обработка настройки новых цветов
    ! SetItemColors() обрабатывает вложенность! (про листбоксы и комбобоксы)
  26.09.2002 SVS
    - ФАР в режиме 80x25. на правой панели штук 15 файлов, кое-какие
      выделены. идем в настройку цветов и меняем цвет выделения и цвет
      обычного текста. после закрытия диалога и меню цвета под ними
      восстанавливаются старые
  20.09.2002 SVS
    - BugZ#645 - Не подряд ооднотипные настройки цветов
  13.04.2002 KM
    - Добавлен VMENU_NOTCHANGE, который предотвращает скачки
      меню по экрану при AltF9 в диалоге редактирования цветов.
      Убран SaveScreen.
  11.02.2002 SVS
    + Добавка в меню - акселератор - решение BugZ#299
  07.08.2001 SVS
    ! Уточнение принудительного рефреша
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
  25.07.2001 OT
    + Разрезервируем "резерв" для "цветных часов" :-)
    - принудительно рефрешим активный фрейм после выбора цвета.
  18.07.2001 OT
    ! VFMenu
  16.06.2001 KM
    ! Добавление WRAPMODE в меню.
  14.06.2001 OT
    ! "Бунт" ;-)
  07.06.2001 SVS
    + Резерв для "цветных часов"
  04.06.2001 SVS
    ! Упростим функцию обработчик и корректно обработаем DN_BTNCLICK
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
  18.05.2001 DJ
    ! GetColorDialog() переписан с использованием функции-обработчика диалога
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  04.12.2000 SVS
    + пункты в меню - COL_DIALOG*DISABLED и COL_WARNDIALOG*DISABLED
  22.11.2000 SVS
    + пункт в меню - COL_DIALOGMENUSCROLLBAR - полоса прокрутки для списка
  13.09.2000 tran 1.04
    + COL_COMMANDLINEPREFIX
  18.03.2000 tran 1.03
    + COL_VIEWERSCROLLBAR
  06.07.2000 SVS
    + Новый пункт для настройки цветов
        COL_DIALOGMENUHIGHLIGHT
        COL_DIALOGMENUSELECTEDHIGHLIGHT
  29.06.2000 SVS
    + Новый пункт для настройки цветов Menu для Menu Scrollbar
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

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

static void SetItemColors(struct MenuData *Items,int *PaletteItems,int Size,int TypeSub);
void GetColor(int PaletteIndex);
static VMenu *MenuToRedraw1=NULL,*MenuToRedraw2=NULL,*MenuToRedraw3=NULL;

static struct MenuData ListItems[]=
{
  (char *)MSetColorDialogListText,LIF_SELECTED,0,
  (char *)MSetColorDialogListHighLight,0,0,
  (char *)MSetColorDialogListSelectedText,0,0,
  (char *)MSetColorDialogListSelectedHighLight,0,0,
  (char *)MSetColorDialogListDisabled,0,0,
  (char *)MSetColorDialogListBox,0,0,
  (char *)MSetColorDialogListTitle,0,0,
  (char *)MSetColorDialogListScrollBar,0,0,
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
  struct MenuData Groups[]=
  {
    (char *)MSetColorPanel,LIF_SELECTED,0,
    (char *)MSetColorDialog,0,0,
    (char *)MSetColorWarning,0,0,
    (char *)MSetColorMenu,0,0,
    (char *)MSetColorHMenu,0,0,
    (char *)MSetColorKeyBar,0,0,
    (char *)MSetColorCommandLine,0,0,
    (char *)MSetColorClock,0,0,
    (char *)MSetColorViewer,0,0,
    (char *)MSetColorEditor,0,0,
    (char *)MSetColorHelp,0,0,
    "",LIF_SEPARATOR,0,
    (char *)MSetDefaultColors,0,0,
    (char *)MSetBW,0,0,
  };

  struct MenuData PanelItems[]=
  {
    (char *)MSetColorPanelNormal,LIF_SELECTED,0,
    (char *)MSetColorPanelSelected,0,0,
    (char *)MSetColorPanelHighlightedInfo,0,0,
    (char *)MSetColorPanelDragging,0,0,
    (char *)MSetColorPanelBox,0,0,
    (char *)MSetColorPanelNormalCursor,0,0,
    (char *)MSetColorPanelSelectedCursor,0,0,
    (char *)MSetColorPanelNormalTitle,0,0,
    (char *)MSetColorPanelSelectedTitle,0,0,
    (char *)MSetColorPanelColumnTitle,0,0,
    (char *)MSetColorPanelTotalInfo,0,0,
    (char *)MSetColorPanelSelectedInfo,0,0,
    (char *)MSetColorPanelScrollbar,0,0,
    (char *)MSetColorPanelScreensNumber,0,0,
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
    (char *)MSetColorDialogNormal,LIF_SELECTED,0,
    (char *)MSetColorDialogHighlighted,0,0,
    (char *)MSetColorDialogDisabled,0,0,
    (char *)MSetColorDialogBox,0,0,
    (char *)MSetColorDialogBoxTitle,0,0,
    (char *)MSetColorDialogHighlightedBoxTitle,0,0,
    (char *)MSetColorDialogTextInput,0,0,
    (char *)MSetColorDialogUnchangedTextInput,0,0,
    (char *)MSetColorDialogSelectedTextInput,0,0,
    (char *)MSetColorDialogEditDisabled,0,0,
    (char *)MSetColorDialogButtons,0,0,
    (char *)MSetColorDialogSelectedButtons,0,0,
    (char *)MSetColorDialogHighlightedButtons,0,0,
    (char *)MSetColorDialogSelectedHighlightedButtons,0,0,
    (char *)MSetColorDialogListBoxControl,0,0,
    (char *)MSetColorDialogComboBoxControl,0,0,
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

  struct MenuData WarnDialogItems[]=
  {
    (char *)MSetColorDialogNormal,LIF_SELECTED,0,
    (char *)MSetColorDialogHighlighted,0,0,
    (char *)MSetColorDialogDisabled,0,0,
    (char *)MSetColorDialogBox,0,0,
    (char *)MSetColorDialogBoxTitle,0,0,
    (char *)MSetColorDialogHighlightedBoxTitle,0,0,
    (char *)MSetColorDialogTextInput,0,0,
    (char *)MSetColorDialogUnchangedTextInput,0,0,
    (char *)MSetColorDialogSelectedTextInput,0,0,
    (char *)MSetColorDialogEditDisabled,0,0,
    (char *)MSetColorDialogButtons,0,0,
    (char *)MSetColorDialogSelectedButtons,0,0,
    (char *)MSetColorDialogHighlightedButtons,0,0,
    (char *)MSetColorDialogSelectedHighlightedButtons,0,0,
    (char *)MSetColorDialogListBoxControl,0,0,
    (char *)MSetColorDialogComboBoxControl,0,0,
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
      Новый пункт для настройки цветов Menu Scrollbar
    */
  struct MenuData MenuItems[]=
  {
    (char *)MSetColorMenuNormal,LIF_SELECTED,0,
    (char *)MSetColorMenuSelected,0,0,
    (char *)MSetColorMenuHighlighted,0,0,
    (char *)MSetColorMenuSelectedHighlighted,0,0,
    (char *)MSetColorMenuDisabled,0,0,
    (char *)MSetColorMenuBox,0,0,
    (char *)MSetColorMenuTitle,0,0,
    (char *)MSetColorMenuScrollBar,0,0,
  };
  int MenuPaletteItems[]={
    COL_MENUTEXT,COL_MENUSELECTEDTEXT,COL_MENUHIGHLIGHT,
    COL_MENUSELECTEDHIGHLIGHT,COL_MENUDISABLEDTEXT,
    COL_MENUBOX,COL_MENUTITLE,COL_MENUSCROLLBAR,
    /* SVS $ */
  };

  struct MenuData HMenuItems[]=
  {
    (char *)MSetColorHMenuNormal,LIF_SELECTED,0,
    (char *)MSetColorHMenuSelected,0,0,
    (char *)MSetColorHMenuHighlighted,0,0,
    (char *)MSetColorHMenuSelectedHighlighted,0,0,
  };
  int HMenuPaletteItems[]={
    COL_HMENUTEXT,COL_HMENUSELECTEDTEXT,COL_HMENUHIGHLIGHT,
    COL_HMENUSELECTEDHIGHLIGHT
  };

  struct MenuData KeyBarItems[]=
  {
    (char *)MSetColorKeyBarNumbers,LIF_SELECTED,0,
    (char *)MSetColorKeyBarNames,0,0,
    (char *)MSetColorKeyBarBackground,0,0,
  };
  int KeyBarPaletteItems[]={
    COL_KEYBARNUM,COL_KEYBARTEXT,COL_KEYBARBACKGROUND
  };

  struct MenuData CommandLineItems[]=
  {
    (char *)MSetColorCommandLineNormal,LIF_SELECTED,0,
    (char *)MSetColorCommandLineSelected,0,0,
    (char *)MSetColorCommandLinePrefix,0,0
  };
  int CommandLinePaletteItems[]={
    COL_COMMANDLINE,COL_COMMANDLINESELECTED,COL_COMMANDLINEPREFIX
  };

  struct MenuData ClockItems[]=
  {
    (char *)MSetColorClockNormal,LIF_SELECTED,0,
    (char *)MSetColorClockNormalEditor,0,0,
    (char *)MSetColorClockNormalViewer,0,0,
  };
  int ClockPaletteItems[]={
    COL_CLOCK,
    COL_EDITORCLOCK,COL_VIEWERCLOCK,
  };

  /* $ 18.07.2000 tran
     новый пунт для MenuScrollbar*/
  struct MenuData ViewerItems[]=
  {
    (char *)MSetColorViewerNormal,LIF_SELECTED,0,
    (char *)MSetColorViewerSelected,0,0,
    (char *)MSetColorViewerStatus,0,0,
    (char *)MSetColorViewerArrows,0,0,
    (char *)MSetColorViewerScrollbar,0,0
  };
  int ViewerPaletteItems[]={
    COL_VIEWERTEXT,COL_VIEWERSELECTEDTEXT,COL_VIEWERSTATUS,COL_VIEWERARROWS,COL_VIEWERSCROLLBAR
  };
  /* tran 18.07.2000 $ */


  struct MenuData EditorItems[]=
  {
    (char *)MSetColorEditorNormal,LIF_SELECTED,0,
    (char *)MSetColorEditorSelected,0,0,
    (char *)MSetColorEditorStatus,0,0,
  };
  int EditorPaletteItems[]={
    COL_EDITORTEXT,COL_EDITORSELECTEDTEXT,COL_EDITORSTATUS
  };

  struct MenuData HelpItems[]=
  {
    (char *)MSetColorHelpNormal,LIF_SELECTED,0,
    (char *)MSetColorHelpHighlighted,0,0,
    (char *)MSetColorHelpReference,0,0,
    (char *)MSetColorHelpSelectedReference,0,0,
    (char *)MSetColorHelpBox,0,0,
    (char *)MSetColorHelpBoxTitle,0,0,
    (char *)MSetColorHelpScrollbar,0,0,
  };
  int HelpPaletteItems[]={
    COL_HELPTEXT,COL_HELPHIGHLIGHTTEXT,COL_HELPTOPIC,COL_HELPSELECTEDTOPIC,
    COL_HELPBOX,COL_HELPBOXTITLE,COL_HELPSCROLLBAR
  };

  {
    int GroupsCode;
    VMenu GroupsMenu(MSG(MSetColorGroupsTitle),Groups,sizeof(Groups)/sizeof(Groups[0]),0);
    MenuToRedraw1=&GroupsMenu;
    while (1)
    {
      GroupsMenu.SetPosition(2,1,0,0);
      /* $ 16.06.2001 KM
         ! Добавление WRAPMODE в меню.
      */
      /* $ 13.04.2002 KM
        - Добавлен VMENU_NOTCHANGE, который предотвращает скачки
          меню по экрану при AltF9 в диалоге редактирования цветов.
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


static void SetItemColors(struct MenuData *Items,int *PaletteItems,int Size,int TypeSub)
{
  int ItemsCode;

  VMenu ItemsMenu(MSG(MSetColorItemsTitle),Items,Size,0);
  if(TypeSub == 2)
    MenuToRedraw3=&ItemsMenu;
  else
    MenuToRedraw2=&ItemsMenu;

  while (1)
  {
    ItemsMenu.SetPosition(17-(TypeSub == 2?7:0),5+(TypeSub == 2?2:0),0,0);
    /* $ 09.04.2002 KM
      - Добавлен VMENU_NOTCHANGE, который предотвращает скачки
        меню по экрану при AltF9 в диалоге редактирования цветов.
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
    ScrBuf.Lock(); // отменяем всякую прорисовку
    if(MenuToRedraw3)
      MenuToRedraw3->Hide();
    MenuToRedraw2->Hide(); // гасим
    MenuToRedraw1->Hide();
    FrameManager->RefreshFrame(); // рефрешим
    FrameManager->PluginCommit(); // коммитим.
    MenuToRedraw1->Show(); // кажем
    MenuToRedraw2->Show();
    if(MenuToRedraw3)
      MenuToRedraw3->Show();
    ScrBuf.Unlock(); // разрешаем прорисовку
    FrameManager->PluginCommit(); // коммитим.
  }
}

/* $ 18.05.2001 DJ
   обработка установки цвета вынесена в функцию-обработчик диалога
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

int GetColorDialog(unsigned int &Color)
{
  static struct DialogData ColorDlgData[]={
    /*   0 */ DI_DOUBLEBOX,3,1,35,13,0,0,0,0,(char *)MSetColorTitle,
    /*   1 */ DI_SINGLEBOX,5,2,18,7,0,0,0,0,(char *)MSetColorForeground,
    /*   2 */ DI_RADIOBUTTON,6,3,0,0,0,0,F_LIGHTGRAY|B_BLACK|DIF_GROUP|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*   3 */ DI_RADIOBUTTON,6,4,0,0,0,0,F_BLACK|B_RED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*   4 */ DI_RADIOBUTTON,6,5,0,0,0,0,F_LIGHTGRAY|B_DARKGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*   5 */ DI_RADIOBUTTON,6,6,0,0,0,0,F_BLACK|B_LIGHTRED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*   6 */ DI_RADIOBUTTON,9,3,0,0,0,0,F_LIGHTGRAY|B_BLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*   7 */ DI_RADIOBUTTON,9,4,0,0,0,0,F_BLACK|B_MAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*   8 */ DI_RADIOBUTTON,9,5,0,0,0,0,F_BLACK|B_LIGHTBLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*   9 */ DI_RADIOBUTTON,9,6,0,0,0,0,F_BLACK|B_LIGHTMAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  10 */ DI_RADIOBUTTON,12,3,0,0,0,0,F_BLACK|B_GREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  11 */ DI_RADIOBUTTON,12,4,0,0,0,0,F_BLACK|B_BROWN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  12 */ DI_RADIOBUTTON,12,5,0,0,0,0,F_BLACK|B_LIGHTGREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  13 */ DI_RADIOBUTTON,12,6,0,0,0,0,F_BLACK|B_YELLOW|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  14 */ DI_RADIOBUTTON,15,3,0,0,0,0,F_BLACK|B_CYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  15 */ DI_RADIOBUTTON,15,4,0,0,0,0,F_BLACK|B_LIGHTGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  16 */ DI_RADIOBUTTON,15,5,0,0,0,0,F_BLACK|B_LIGHTCYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  17 */ DI_RADIOBUTTON,15,6,0,0,0,0,F_BLACK|B_WHITE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  18 */ DI_SINGLEBOX,20,2,33,7,0,0,0,0,(char *)MSetColorBackground,
    /*  19 */ DI_RADIOBUTTON,21,3,0,0,0,0,F_LIGHTGRAY|B_BLACK|DIF_GROUP|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  20 */ DI_RADIOBUTTON,21,4,0,0,0,0,F_BLACK|B_RED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  21 */ DI_RADIOBUTTON,21,5,0,0,0,0,F_LIGHTGRAY|B_DARKGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  22 */ DI_RADIOBUTTON,21,6,0,0,0,0,F_BLACK|B_LIGHTRED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  23 */ DI_RADIOBUTTON,24,3,0,0,0,0,F_LIGHTGRAY|B_BLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  24 */ DI_RADIOBUTTON,24,4,0,0,0,0,F_BLACK|B_MAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  25 */ DI_RADIOBUTTON,24,5,0,0,0,0,F_BLACK|B_LIGHTBLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  26 */ DI_RADIOBUTTON,24,6,0,0,0,0,F_BLACK|B_LIGHTMAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  27 */ DI_RADIOBUTTON,27,3,0,0,0,0,F_BLACK|B_GREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  28 */ DI_RADIOBUTTON,27,4,0,0,0,0,F_BLACK|B_BROWN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  29 */ DI_RADIOBUTTON,27,5,0,0,0,0,F_BLACK|B_LIGHTGREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  30 */ DI_RADIOBUTTON,27,6,0,0,0,0,F_BLACK|B_YELLOW|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  31 */ DI_RADIOBUTTON,30,3,0,0,0,0,F_BLACK|B_CYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  32 */ DI_RADIOBUTTON,30,4,0,0,0,0,F_BLACK|B_LIGHTGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  33 */ DI_RADIOBUTTON,30,5,0,0,0,0,F_BLACK|B_LIGHTCYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  34 */ DI_RADIOBUTTON,30,6,0,0,0,0,F_BLACK|B_WHITE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
    /*  35 */ DI_TEXT,5,8,0,0,0,0,DIF_SETCOLOR,0,(char *)MSetColorSample,
    /*  36 */ DI_TEXT,5,9,0,0,0,0,DIF_SETCOLOR,0,(char *)MSetColorSample,
    /*  37 */ DI_TEXT,5,10,0,0,0,0,DIF_SETCOLOR,0,(char *)MSetColorSample,
    /*  38 */ DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    /*  39 */ DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,1,(char *)MSetColorSet,
    /*  40 */ DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,0,(char *)MSetColorCancel,

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
    /* $ 18.05.2001 DJ
       обработка установки цвета вынесена в функцию-обработчик диалога
    */
    //SaveScreen SaveScr;
    Dialog Dlg(ColorDlg,sizeof(ColorDlg)/sizeof(ColorDlg[0]), GetColorDlgProc, (long) &CurColor);
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
