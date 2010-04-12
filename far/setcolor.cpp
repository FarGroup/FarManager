/*
setcolor.cpp

Установка фаровских цветов

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
#include "panel.hpp"
#include "chgmmode.hpp"

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
	(char *)MSetColorDialogListArrows,0,0,
	(char *)MSetColorDialogListArrowsSelected,0,0,
	(char *)MSetColorDialogListArrowsDisabled,0,0,
	(char *)MSetColorDialogListGrayed,0,0,
	(char *)MSetColorDialogSelectedListGrayed,0,0,
};

// 0,1 - dialog,warn List
// 2,3 - dialog,warn Combobox
static int ListPaletteItems[4][13]=
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
		COL_DIALOGLISTARROWS,             // Arrow
		COL_DIALOGLISTARROWSSELECTED,     // Выбранный - Arrow
		COL_DIALOGLISTARROWSDISABLED,     // Arrow disabled
		COL_DIALOGLISTGRAY,                        // "серый"
		COL_DIALOGLISTSELECTEDGRAYTEXT,            // выбранный "серый"
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
		COL_WARNDIALOGLISTARROWS,                   // Arrow
		COL_WARNDIALOGLISTARROWSSELECTED,           // Выбранный - Arrow
		COL_WARNDIALOGLISTARROWSDISABLED,           // Arrow disabled
		COL_WARNDIALOGLISTGRAY,                    // "серый"
		COL_WARNDIALOGLISTSELECTEDGRAYTEXT,        // выбранный "серый"
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
		COL_DIALOGCOMBOARROWS,                      // Arrow
		COL_DIALOGCOMBOARROWSSELECTED,              // Выбранный - Arrow
		COL_DIALOGCOMBOARROWSDISABLED,              // Arrow disabled
		COL_DIALOGCOMBOGRAY,                       // "серый"
		COL_DIALOGCOMBOSELECTEDGRAYTEXT,           // выбранный "серый"
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
		COL_WARNDIALOGCOMBOARROWS,                  // Arrow
		COL_WARNDIALOGCOMBOARROWSSELECTED,          // Выбранный - Arrow
		COL_WARNDIALOGCOMBOARROWSDISABLED,          // Arrow disabled
		COL_WARNDIALOGCOMBOGRAY,                   // "серый"
		COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT,       // выбранный "серый"
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
	int PanelPaletteItems[]=
	{
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
	int DialogPaletteItems[]=
	{
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
	int WarnDialogPaletteItems[]=
	{
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
		(char *)MSetColorMenuArrows,0,0,
		(char *)MSetColorMenuArrowsSelected,0,0,
		(char *)MSetColorMenuArrowsDisabled,0,0,
		(char *)MSetColorMenuGrayed,0,0,
		(char *)MSetColorMenuSelectedGrayed,0,0,
	};
	int MenuPaletteItems[]=
	{
		COL_MENUTEXT,
		COL_MENUSELECTEDTEXT,
		COL_MENUHIGHLIGHT,
		COL_MENUSELECTEDHIGHLIGHT,
		COL_MENUDISABLEDTEXT,
		COL_MENUBOX,
		COL_MENUTITLE,
		COL_MENUSCROLLBAR,
		COL_MENUARROWS,                             // Arrow
		COL_MENUARROWSSELECTED,                     // Выбранный - Arrow
		COL_MENUARROWSDISABLED,
		COL_MENUGRAYTEXT,                          // "серый"
		COL_MENUSELECTEDGRAYTEXT,                  // выбранный "серый"
	};
	struct MenuData HMenuItems[]=
	{
		(char *)MSetColorHMenuNormal,LIF_SELECTED,0,
		(char *)MSetColorHMenuSelected,0,0,
		(char *)MSetColorHMenuHighlighted,0,0,
		(char *)MSetColorHMenuSelectedHighlighted,0,0,
	};
	int HMenuPaletteItems[]=
	{
		COL_HMENUTEXT,COL_HMENUSELECTEDTEXT,COL_HMENUHIGHLIGHT,
		COL_HMENUSELECTEDHIGHLIGHT
	};
	struct MenuData KeyBarItems[]=
	{
		(char *)MSetColorKeyBarNumbers,LIF_SELECTED,0,
		(char *)MSetColorKeyBarNames,0,0,
		(char *)MSetColorKeyBarBackground,0,0,
	};
	int KeyBarPaletteItems[]=
	{
		COL_KEYBARNUM,COL_KEYBARTEXT,COL_KEYBARBACKGROUND
	};
	struct MenuData CommandLineItems[]=
	{
		(char *)MSetColorCommandLineNormal,LIF_SELECTED,0,
		(char *)MSetColorCommandLineSelected,0,0,
		(char *)MSetColorCommandLinePrefix,0,0,
		(char *)MSetColorCommandLineUserScreen,0,0,
	};
	int CommandLinePaletteItems[]=
	{
		COL_COMMANDLINE,COL_COMMANDLINESELECTED,COL_COMMANDLINEPREFIX,COL_COMMANDLINEUSERSCREEN
	};
	struct MenuData ClockItems[]=
	{
		(char *)MSetColorClockNormal,LIF_SELECTED,0,
		(char *)MSetColorClockNormalEditor,0,0,
		(char *)MSetColorClockNormalViewer,0,0,
	};
	int ClockPaletteItems[]=
	{
		COL_CLOCK,
		COL_EDITORCLOCK,COL_VIEWERCLOCK,
	};
	struct MenuData ViewerItems[]=
	{
		(char *)MSetColorViewerNormal,LIF_SELECTED,0,
		(char *)MSetColorViewerSelected,0,0,
		(char *)MSetColorViewerStatus,0,0,
		(char *)MSetColorViewerArrows,0,0,
		(char *)MSetColorViewerScrollbar,0,0
	};
	int ViewerPaletteItems[]=
	{
		COL_VIEWERTEXT,COL_VIEWERSELECTEDTEXT,COL_VIEWERSTATUS,COL_VIEWERARROWS,COL_VIEWERSCROLLBAR
	};
	struct MenuData EditorItems[]=
	{
		(char *)MSetColorEditorNormal,LIF_SELECTED,0,
		(char *)MSetColorEditorSelected,0,0,
		(char *)MSetColorEditorStatus,0,0,
		(char *)MSetColorEditorScrollbar,0,0,
	};
	int EditorPaletteItems[]=
	{
		COL_EDITORTEXT,COL_EDITORSELECTEDTEXT,COL_EDITORSTATUS,COL_EDITORSCROLLBAR
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
	int HelpPaletteItems[]=
	{
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

			switch (GroupsCode)
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
	CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
	CtrlObject->Cp()->LeftPanel->Redraw();
	CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
	CtrlObject->Cp()->RightPanel->Redraw();
}


static void SetItemColors(struct MenuData *Items,int *PaletteItems,int Size,int TypeSub)
{
	int ItemsCode;
	VMenu ItemsMenu(MSG(MSetColorItemsTitle),Items,Size,0);

	if (TypeSub == 2)
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

		if (TypeSub == 1 && PaletteItems[ItemsCode] < 4)
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
	ChangeMacroMode chgMacroMode(MACRO_MENU);
	unsigned int NewColor=Palette[PaletteIndex-COL_FIRSTPALETTECOLOR];

	if (GetColorDialog(NewColor))
	{
		Palette[PaletteIndex-COL_FIRSTPALETTECOLOR]=NewColor;
		ScrBuf.Lock(); // отменяем всякую прорисовку
		CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->LeftPanel->Redraw();
		CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->RightPanel->Redraw();

		if (MenuToRedraw3)
			MenuToRedraw3->Hide();

		MenuToRedraw2->Hide(); // гасим
		MenuToRedraw1->Hide();
		FrameManager->RefreshFrame(); // рефрешим
		FrameManager->PluginCommit(); // коммитим.
		MenuToRedraw1->SetColors();
		MenuToRedraw1->Show(); // кажем
		MenuToRedraw2->SetColors();
		MenuToRedraw2->Show();

		if (MenuToRedraw3)
		{
			MenuToRedraw3->SetColors();
			MenuToRedraw3->Show();
		}

		if (Opt.Clock)
			ShowTime(1);

		ScrBuf.Unlock(); // разрешаем прорисовку
		FrameManager->PluginCommit(); // коммитим.
	}
}


static LONG_PTR WINAPI GetColorDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	switch (Msg)
	{
		case DN_CTLCOLORDLGITEM:

			if (Param1 >= 37 && Param1 <= 39)
			{
				int *CurColor=(int *)Dialog::SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
				return (Param2&0xFFFFFF00U)|((*CurColor)&0xFF);
			}

			break;
		case DN_BTNCLICK:

			if (Param1 >= 2 && Param1 <= 34)
			{
				FarDialogItem DlgItem;
				int NewColor;
				int *CurColor = (int *) Dialog::SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
				Dialog::SendDlgMessage(hDlg, DM_GETDLGITEM, Param1, (LONG_PTR) &DlgItem);
				NewColor=*CurColor;

				if (Param1 >= 2 && Param1 <= 17) // Fore
				{
					NewColor&=~0x0F;
					NewColor|=(DlgItem.Flags & B_MASK)>>4;
				}

				if (Param1 >= 19 && Param1 <= 34) // Back
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

	return Dialog::DefDlgProc(hDlg, Msg, Param1, Param2);
}

int GetColorDialog(unsigned int &Color,bool bCentered,bool bAddTransparent)
{
	static struct DialogData ColorDlgData[]=
	{
		/*   0 */ DI_DOUBLEBOX,   3, 1,35,13, 0,0,0,0,(char *)MSetColorTitle,
		/*   1 */ DI_SINGLEBOX,   5, 2,18, 7, 0,0,0,0,(char *)MSetColorForeground,
		/*   2 */ DI_RADIOBUTTON, 6, 3, 0, 3, 0,0,F_LIGHTGRAY|B_BLACK|DIF_GROUP|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*   3 */ DI_RADIOBUTTON, 6, 4, 0, 4, 0,0,F_BLACK|B_RED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*   4 */ DI_RADIOBUTTON, 6, 5, 0, 5, 0,0,F_LIGHTGRAY|B_DARKGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*   5 */ DI_RADIOBUTTON, 6, 6, 0, 6, 0,0,F_BLACK|B_LIGHTRED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*   6 */ DI_RADIOBUTTON, 9, 3, 0, 3, 0,0,F_LIGHTGRAY|B_BLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*   7 */ DI_RADIOBUTTON, 9, 4, 0, 4, 0,0,F_BLACK|B_MAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*   8 */ DI_RADIOBUTTON, 9, 5, 0, 5, 0,0,F_BLACK|B_LIGHTBLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*   9 */ DI_RADIOBUTTON, 9, 6, 0, 6, 0,0,F_BLACK|B_LIGHTMAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  10 */ DI_RADIOBUTTON,12, 3, 0, 3, 0,0,F_BLACK|B_GREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  11 */ DI_RADIOBUTTON,12, 4, 0, 4, 0,0,F_BLACK|B_BROWN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  12 */ DI_RADIOBUTTON,12, 5, 0, 5, 0,0,F_BLACK|B_LIGHTGREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  13 */ DI_RADIOBUTTON,12, 6, 0, 6, 0,0,F_BLACK|B_YELLOW|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  14 */ DI_RADIOBUTTON,15, 3, 0, 3, 0,0,F_BLACK|B_CYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  15 */ DI_RADIOBUTTON,15, 4, 0, 4, 0,0,F_BLACK|B_LIGHTGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  16 */ DI_RADIOBUTTON,15, 5, 0, 5, 0,0,F_BLACK|B_LIGHTCYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  17 */ DI_RADIOBUTTON,15, 6, 0, 6, 0,0,F_BLACK|B_WHITE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  18 */ DI_SINGLEBOX,  20, 2,33, 7, 0,0,0,0,(char *)MSetColorBackground,
		/*  19 */ DI_RADIOBUTTON,21, 3, 0, 3, 0,0,F_LIGHTGRAY|B_BLACK|DIF_GROUP|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  20 */ DI_RADIOBUTTON,21, 4, 0, 4, 0,0,F_BLACK|B_RED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  21 */ DI_RADIOBUTTON,21, 5, 0, 5, 0,0,F_LIGHTGRAY|B_DARKGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  22 */ DI_RADIOBUTTON,21, 6, 0, 6, 0,0,F_BLACK|B_LIGHTRED|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  23 */ DI_RADIOBUTTON,24, 3, 0, 3, 0,0,F_LIGHTGRAY|B_BLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  24 */ DI_RADIOBUTTON,24, 4, 0, 4, 0,0,F_BLACK|B_MAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  25 */ DI_RADIOBUTTON,24, 5, 0, 5, 0,0,F_BLACK|B_LIGHTBLUE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  26 */ DI_RADIOBUTTON,24, 6, 0, 6, 0,0,F_BLACK|B_LIGHTMAGENTA|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  27 */ DI_RADIOBUTTON,27, 3, 0, 3, 0,0,F_BLACK|B_GREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  28 */ DI_RADIOBUTTON,27, 4, 0, 4, 0,0,F_BLACK|B_BROWN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  29 */ DI_RADIOBUTTON,27, 5, 0, 5, 0,0,F_BLACK|B_LIGHTGREEN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  30 */ DI_RADIOBUTTON,27, 6, 0, 6, 0,0,F_BLACK|B_YELLOW|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  31 */ DI_RADIOBUTTON,30, 3, 0, 3, 0,0,F_BLACK|B_CYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  32 */ DI_RADIOBUTTON,30, 4, 0, 4, 0,0,F_BLACK|B_LIGHTGRAY|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  33 */ DI_RADIOBUTTON,30, 5, 0, 5, 0,0,F_BLACK|B_LIGHTCYAN|DIF_SETCOLOR|DIF_MOVESELECT,0,"",
		/*  34 */ DI_RADIOBUTTON,30, 6, 0, 6, 0,0,F_BLACK|B_WHITE|DIF_SETCOLOR|DIF_MOVESELECT,0,"",

		/*  35 */ DI_CHECKBOX,    5, 10,0, 10,0,0,0,0,(char *)MSetColorForeTransparent,
		/*  36 */ DI_CHECKBOX,   22, 10,0, 10,0,0,0,0,(char *)MSetColorBackTransparent,

		/*  37 */ DI_TEXT,        5, 8, 33,8, 0,0,DIF_SETCOLOR,0,(char *)MSetColorSample,
		/*  38 */ DI_TEXT,        5, 9, 33,9, 0,0,DIF_SETCOLOR,0,(char *)MSetColorSample,
		/*  39 */ DI_TEXT,        5,10, 33,10,0,0,DIF_SETCOLOR,0,(char *)MSetColorSample,
		/*  40 */ DI_TEXT,        0,11, 0, 11,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
		/*  41 */ DI_BUTTON,      0,12, 0, 12,0,0,DIF_CENTERGROUP,1,(char *)MSetColorSet,
		/*  42 */ DI_BUTTON,      0,12, 0, 12,0,0,DIF_CENTERGROUP,0,(char *)MSetColorCancel,

	};
	MakeDialogItems(ColorDlgData,ColorDlg);
	int ExitCode,I,CurColor=Color;

	for (I=2; I<18; I++)
		if (((ColorDlg[I].Flags & B_MASK)>>4)==(Color & F_MASK))
		{
			ColorDlg[I].Selected=ColorDlg[I].Focus=1;
			break;
		}

	for (I=19; I<35; I++)
		if ((ColorDlg[I].Flags & B_MASK)==(Color & B_MASK))
		{
			ColorDlg[I].Selected=1;
			break;
		}

	for (I=37; I<40; I++)
		ColorDlg[I].Flags=(ColorDlg[I].Flags & ~DIF_COLORMASK) | Color;

	if (bAddTransparent)
	{
		ColorDlg[0].Y2++;

		for (I=37; I<=42; I++)
		{
			ColorDlg[I].Y1+=3;
			ColorDlg[I].Y2+=3;
		}

		ColorDlg[0].X2+=4;
		ColorDlg[0].Y2+=2;
		ColorDlg[1].X2+=2;
		ColorDlg[1].Y2+=2;
		ColorDlg[18].X1+=2;
		ColorDlg[18].X2+=4;
		ColorDlg[18].Y2+=2;

		for (I=2; I<=17; I++)
		{
			ColorDlg[I].X1+=1;
			ColorDlg[I].Y1+=1;
			ColorDlg[I].Y2+=1;
		}

		for (I=19; I<=34; I++)
		{
			ColorDlg[I].X1+=3;
			ColorDlg[I].Y1+=1;
			ColorDlg[I].Y2+=1;
		}

		for (I=37; I<=39; I++)
			ColorDlg[I].X2+=4;

		ColorDlg[35].Selected=(Color&0x0F00?1:0);
		ColorDlg[36].Selected=(Color&0xF000?1:0);
	}
	else
	{
		ColorDlg[35].Flags|=DIF_HIDDEN;
		ColorDlg[36].Flags|=DIF_HIDDEN;
	}

	{
		Dialog Dlg(ColorDlg,sizeof(ColorDlg)/sizeof(ColorDlg[0]), GetColorDlgProc, (LONG_PTR) &CurColor);

		if (bCentered)
			Dlg.SetPosition(-1,-1,39+(bAddTransparent?4:0),15+(bAddTransparent?3:0));
		else
			Dlg.SetPosition(37,2,75+(bAddTransparent?4:0),16+(bAddTransparent?3:0));

		Dlg.Process();
		ExitCode=Dlg.GetExitCode();
	}

	if (ExitCode==41)
	{
		Color=CurColor;

		if (ColorDlg[35].Selected)
			Color|=0x0F00;
		else
			Color&=0xF0FF;

		if (ColorDlg[36].Selected)
			Color|=0xF000;
		else
			Color&=0x0FFF;

		return(TRUE);
	}

	return(FALSE);
}
