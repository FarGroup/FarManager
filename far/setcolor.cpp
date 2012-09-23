/*
setcolor.cpp

Установка фаровских цветов
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "setcolor.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "savescr.hpp"
#include "scrbuf.hpp"
#include "panel.hpp"
#include "chgmmode.hpp"
#include "interf.hpp"
#include "palette.hpp"
#include "config.hpp"
#include "console.hpp"
#include "colormix.hpp"

static void SetItemColors(MenuDataEx *Items,int *PaletteItems,int Size,int TypeSub, VMenu* MenuToRedraw1, VMenu* MenuToRedraw2=nullptr);

void GetColor(int PaletteIndex, VMenu* MenuToRedraw1, VMenu* MenuToRedraw2, VMenu* MenuToRedraw3)
{
	ChangeMacroMode chgMacroMode(MACRO_MENU);
	FarColor NewColor = Opt.Palette.CurrentPalette[PaletteIndex-COL_FIRSTPALETTECOLOR];

	if (Console.GetColorDialog(NewColor))
	{
		Opt.Palette.CurrentPalette[PaletteIndex-COL_FIRSTPALETTECOLOR] = NewColor;
		Opt.Palette.SetChanged();
		ScrBuf.Lock(); // отменяем всякую прорисовку
		CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->LeftPanel->Redraw();
		CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->RightPanel->Redraw();

		if (MenuToRedraw3)
			MenuToRedraw3->Hide();

		if(MenuToRedraw2)
			MenuToRedraw2->Hide(); // гасим

		MenuToRedraw1->Hide();
		FrameManager->RefreshFrame(); // рефрешим
		FrameManager->PluginCommit(); // коммитим.
		MenuToRedraw1->SetColors();
		MenuToRedraw1->Show(); // кажем

		if(MenuToRedraw2)
		{
			MenuToRedraw2->SetColors();
			MenuToRedraw2->Show();
		}

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


void SetColors()
{
	MenuDataEx Groups[]=
	{
		MSG(MSetColorPanel),LIF_SELECTED,0,
		MSG(MSetColorDialog),0,0,
		MSG(MSetColorWarning),0,0,
		MSG(MSetColorMenu),0,0,
		MSG(MSetColorHMenu),0,0,
		MSG(MSetColorKeyBar),0,0,
		MSG(MSetColorCommandLine),0,0,
		MSG(MSetColorClock),0,0,
		MSG(MSetColorViewer),0,0,
		MSG(MSetColorEditor),0,0,
		MSG(MSetColorHelp),0,0,
		L"",LIF_SEPARATOR,0,
		MSG(MSetDefaultColors),0,0,
		MSG(MSetBW),0,0,
	};
	MenuDataEx PanelItems[]=
	{
		MSG(MSetColorPanelNormal),LIF_SELECTED,0,
		MSG(MSetColorPanelSelected),0,0,
		MSG(MSetColorPanelHighlightedInfo),0,0,
		MSG(MSetColorPanelDragging),0,0,
		MSG(MSetColorPanelBox),0,0,
		MSG(MSetColorPanelNormalCursor),0,0,
		MSG(MSetColorPanelSelectedCursor),0,0,
		MSG(MSetColorPanelNormalTitle),0,0,
		MSG(MSetColorPanelSelectedTitle),0,0,
		MSG(MSetColorPanelColumnTitle),0,0,
		MSG(MSetColorPanelTotalInfo),0,0,
		MSG(MSetColorPanelSelectedInfo),0,0,
		MSG(MSetColorPanelScrollbar),0,0,
		MSG(MSetColorPanelScreensNumber),0,0,
	};
	int PanelPaletteItems[]=
	{
		COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELINFOTEXT,
		COL_PANELDRAGTEXT,COL_PANELBOX,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR,
		COL_PANELTITLE,COL_PANELSELECTEDTITLE,COL_PANELCOLUMNTITLE,
		COL_PANELTOTALINFO,COL_PANELSELECTEDINFO,COL_PANELSCROLLBAR,
		COL_PANELSCREENSNUMBER
	};
	MenuDataEx DialogItems[]=
	{
		MSG(MSetColorDialogNormal),LIF_SELECTED,0,
		MSG(MSetColorDialogHighlighted),0,0,
		MSG(MSetColorDialogDisabled),0,0,
		MSG(MSetColorDialogBox),0,0,
		MSG(MSetColorDialogBoxTitle),0,0,
		MSG(MSetColorDialogHighlightedBoxTitle),0,0,
		MSG(MSetColorDialogTextInput),0,0,
		MSG(MSetColorDialogUnchangedTextInput),0,0,
		MSG(MSetColorDialogSelectedTextInput),0,0,
		MSG(MSetColorDialogEditDisabled),0,0,
		MSG(MSetColorDialogButtons),0,0,
		MSG(MSetColorDialogSelectedButtons),0,0,
		MSG(MSetColorDialogHighlightedButtons),0,0,
		MSG(MSetColorDialogSelectedHighlightedButtons),0,0,
		MSG(MSetColorDialogDefaultButton),0,0,
		MSG(MSetColorDialogSelectedDefaultButton),0,0,
		MSG(MSetColorDialogHighlightedDefaultButton),0,0,
		MSG(MSetColorDialogSelectedHighlightedDefaultButton),0,0,
		MSG(MSetColorDialogListBoxControl),0,0,
		MSG(MSetColorDialogComboBoxControl),0,0,
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
		COL_DIALOGDEFAULTBUTTON,
		COL_DIALOGSELECTEDDEFAULTBUTTON,
		COL_DIALOGHIGHLIGHTDEFAULTBUTTON,
		COL_DIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON,
		0,
		2,
	};
	MenuDataEx WarnDialogItems[]=
	{
		MSG(MSetColorDialogNormal),LIF_SELECTED,0,
		MSG(MSetColorDialogHighlighted),0,0,
		MSG(MSetColorDialogDisabled),0,0,
		MSG(MSetColorDialogBox),0,0,
		MSG(MSetColorDialogBoxTitle),0,0,
		MSG(MSetColorDialogHighlightedBoxTitle),0,0,
		MSG(MSetColorDialogTextInput),0,0,
		MSG(MSetColorDialogUnchangedTextInput),0,0,
		MSG(MSetColorDialogSelectedTextInput),0,0,
		MSG(MSetColorDialogEditDisabled),0,0,
		MSG(MSetColorDialogButtons),0,0,
		MSG(MSetColorDialogSelectedButtons),0,0,
		MSG(MSetColorDialogHighlightedButtons),0,0,
		MSG(MSetColorDialogSelectedHighlightedButtons),0,0,
		MSG(MSetColorDialogDefaultButton),0,0,
		MSG(MSetColorDialogSelectedDefaultButton),0,0,
		MSG(MSetColorDialogHighlightedDefaultButton),0,0,
		MSG(MSetColorDialogSelectedHighlightedDefaultButton),0,0,
		MSG(MSetColorDialogListBoxControl),0,0,
		MSG(MSetColorDialogComboBoxControl),0,0,
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
		COL_WARNDIALOGDEFAULTBUTTON,
		COL_WARNDIALOGSELECTEDDEFAULTBUTTON,
		COL_WARNDIALOGHIGHLIGHTDEFAULTBUTTON,
		COL_WARNDIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON,
		1,
		3,
	};
	MenuDataEx MenuItems[]=
	{
		MSG(MSetColorMenuNormal),LIF_SELECTED,0,
		MSG(MSetColorMenuSelected),0,0,
		MSG(MSetColorMenuHighlighted),0,0,
		MSG(MSetColorMenuSelectedHighlighted),0,0,
		MSG(MSetColorMenuDisabled),0,0,
		MSG(MSetColorMenuBox),0,0,
		MSG(MSetColorMenuTitle),0,0,
		MSG(MSetColorMenuScrollBar),0,0,
		MSG(MSetColorMenuArrows),0,0,
		MSG(MSetColorMenuArrowsSelected),0,0,
		MSG(MSetColorMenuArrowsDisabled),0,0,
		MSG(MSetColorMenuGrayed),0,0,
		MSG(MSetColorMenuSelectedGrayed),0,0,
	};
	int MenuPaletteItems[]=
	{
		COL_MENUTEXT,COL_MENUSELECTEDTEXT,COL_MENUHIGHLIGHT,
		COL_MENUSELECTEDHIGHLIGHT,COL_MENUDISABLEDTEXT,
		COL_MENUBOX,COL_MENUTITLE,COL_MENUSCROLLBAR,
		COL_MENUARROWS,                             // Arrow
		COL_MENUARROWSSELECTED,                     // Выбранный - Arrow
		COL_MENUARROWSDISABLED,
		COL_MENUGRAYTEXT,                          // "серый"
		COL_MENUSELECTEDGRAYTEXT,                  // выбранный "серый"
	};
	MenuDataEx HMenuItems[]=
	{
		MSG(MSetColorHMenuNormal),LIF_SELECTED,0,
		MSG(MSetColorHMenuSelected),0,0,
		MSG(MSetColorHMenuHighlighted),0,0,
		MSG(MSetColorHMenuSelectedHighlighted),0,0,
	};
	int HMenuPaletteItems[]=
	{
		COL_HMENUTEXT,COL_HMENUSELECTEDTEXT,COL_HMENUHIGHLIGHT,
		COL_HMENUSELECTEDHIGHLIGHT
	};
	MenuDataEx KeyBarItems[]=
	{
		MSG(MSetColorKeyBarNumbers),LIF_SELECTED,0,
		MSG(MSetColorKeyBarNames),0,0,
		MSG(MSetColorKeyBarBackground),0,0,
	};
	int KeyBarPaletteItems[]=
	{
		COL_KEYBARNUM,COL_KEYBARTEXT,COL_KEYBARBACKGROUND
	};
	MenuDataEx CommandLineItems[]=
	{
		MSG(MSetColorCommandLineNormal),LIF_SELECTED,0,
		MSG(MSetColorCommandLineSelected),0,0,
		MSG(MSetColorCommandLinePrefix),0,0,
		MSG(MSetColorCommandLineUserScreen),0,0,
	};
	int CommandLinePaletteItems[]=
	{
		COL_COMMANDLINE,COL_COMMANDLINESELECTED,COL_COMMANDLINEPREFIX,COL_COMMANDLINEUSERSCREEN
	};
	MenuDataEx ClockItems[]=
	{
		MSG(MSetColorClockNormal),LIF_SELECTED,0,
		MSG(MSetColorClockNormalEditor),0,0,
		MSG(MSetColorClockNormalViewer),0,0,
	};
	int ClockPaletteItems[]=
	{
		COL_CLOCK,
		COL_EDITORCLOCK,COL_VIEWERCLOCK,
	};
	MenuDataEx ViewerItems[]=
	{
		MSG(MSetColorViewerNormal),LIF_SELECTED,0,
		MSG(MSetColorViewerSelected),0,0,
		MSG(MSetColorViewerStatus),0,0,
		MSG(MSetColorViewerArrows),0,0,
		MSG(MSetColorViewerScrollbar),0,0
	};
	int ViewerPaletteItems[]=
	{
		COL_VIEWERTEXT,COL_VIEWERSELECTEDTEXT,COL_VIEWERSTATUS,COL_VIEWERARROWS,COL_VIEWERSCROLLBAR
	};
	MenuDataEx EditorItems[]=
	{
		MSG(MSetColorEditorNormal),LIF_SELECTED,0,
		MSG(MSetColorEditorSelected),0,0,
		MSG(MSetColorEditorStatus),0,0,
		MSG(MSetColorEditorScrollbar),0,0,
	};
	int EditorPaletteItems[]=
	{
		COL_EDITORTEXT,COL_EDITORSELECTEDTEXT,COL_EDITORSTATUS,COL_EDITORSCROLLBAR
	};
	MenuDataEx HelpItems[]=
	{
		MSG(MSetColorHelpNormal),LIF_SELECTED,0,
		MSG(MSetColorHelpHighlighted),0,0,
		MSG(MSetColorHelpReference),0,0,
		MSG(MSetColorHelpSelectedReference),0,0,
		MSG(MSetColorHelpBox),0,0,
		MSG(MSetColorHelpBoxTitle),0,0,
		MSG(MSetColorHelpScrollbar),0,0,
	};
	int HelpPaletteItems[]=
	{
		COL_HELPTEXT,COL_HELPHIGHLIGHTTEXT,COL_HELPTOPIC,COL_HELPSELECTEDTOPIC,
		COL_HELPBOX,COL_HELPBOXTITLE,COL_HELPSCROLLBAR
	};
	{
		int GroupsCode;
		VMenu GroupsMenu(MSG(MSetColorGroupsTitle),Groups,ARRAYSIZE(Groups),0);
		VMenu *MenuToRedraw1=&GroupsMenu;
		for (;;)
		{
			GroupsMenu.SetPosition(2,1,0,0);
			GroupsMenu.SetFlags(VMENU_WRAPMODE|VMENU_NOTCHANGE);
			GroupsMenu.ClearDone();
			GroupsMenu.Process();

			if ((GroupsCode=GroupsMenu.Modal::GetExitCode())<0)
				break;

			if (GroupsCode==12)
			{
				Opt.Palette.ResetToDefault();
				break;
			}

			if (GroupsCode==13)
			{
				Opt.Palette.ResetToBlack();
				break;
			}

			switch (GroupsCode)
			{
				case 0:
					SetItemColors(PanelItems,PanelPaletteItems,ARRAYSIZE(PanelItems),0,MenuToRedraw1);
					break;
				case 1:
					SetItemColors(DialogItems,DialogPaletteItems,ARRAYSIZE(DialogItems),1,MenuToRedraw1);
					break;
				case 2:
					SetItemColors(WarnDialogItems,WarnDialogPaletteItems,ARRAYSIZE(WarnDialogItems),1,MenuToRedraw1);
					break;
				case 3:
					SetItemColors(MenuItems,MenuPaletteItems,ARRAYSIZE(MenuItems),0,MenuToRedraw1);
					break;
				case 4:
					SetItemColors(HMenuItems,HMenuPaletteItems,ARRAYSIZE(HMenuItems),0,MenuToRedraw1);
					break;
				case 5:
					SetItemColors(KeyBarItems,KeyBarPaletteItems,ARRAYSIZE(KeyBarItems),0,MenuToRedraw1);
					break;
				case 6:
					SetItemColors(CommandLineItems,CommandLinePaletteItems,ARRAYSIZE(CommandLineItems),0,MenuToRedraw1);
					break;
				case 7:
					SetItemColors(ClockItems,ClockPaletteItems,ARRAYSIZE(ClockItems),0,MenuToRedraw1);
					break;
				case 8:
					SetItemColors(ViewerItems,ViewerPaletteItems,ARRAYSIZE(ViewerItems),0,MenuToRedraw1);
					break;
				case 9:
					SetItemColors(EditorItems,EditorPaletteItems,ARRAYSIZE(EditorItems),0,MenuToRedraw1);
					break;
				case 10:
					SetItemColors(HelpItems,HelpPaletteItems,ARRAYSIZE(HelpItems),0,MenuToRedraw1);
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


static void SetItemColors(MenuDataEx *Items,int *PaletteItems,int Size,int TypeSub, VMenu* MenuToRedraw1, VMenu* MenuToRedraw2)
{
	int ItemsCode;
	VMenu ItemsMenu(MSG(MSetColorItemsTitle),Items,Size,0);
	VMenu* MenuToRedraw3 = nullptr;
	if (TypeSub == 2)
		MenuToRedraw3=&ItemsMenu;
	else
		MenuToRedraw2=&ItemsMenu;

	for (;;)
	{
		ItemsMenu.SetPosition(17-(TypeSub == 2?7:0),5+(TypeSub == 2?2:0),0,0);
		ItemsMenu.SetFlags(VMENU_WRAPMODE|VMENU_NOTCHANGE);
		ItemsMenu.ClearDone();
		ItemsMenu.Process();

		if ((ItemsCode=ItemsMenu.Modal::GetExitCode())<0)
			break;

// 0,1 - dialog,warn List
// 2,3 - dialog,warn Combobox
		if (TypeSub == 1 && PaletteItems[ItemsCode] < 4)
		{
			MenuDataEx ListItems[]=
			{
				MSG(MSetColorDialogListText),LIF_SELECTED,0,
				MSG(MSetColorDialogListHighLight),0,0,
				MSG(MSetColorDialogListSelectedText),0,0,
				MSG(MSetColorDialogListSelectedHighLight),0,0,
				MSG(MSetColorDialogListDisabled),0,0,
				MSG(MSetColorDialogListBox),0,0,
				MSG(MSetColorDialogListTitle),0,0,
				MSG(MSetColorDialogListScrollBar),0,0,
				MSG(MSetColorDialogListArrows),0,0,
				MSG(MSetColorDialogListArrowsSelected),0,0,
				MSG(MSetColorDialogListArrowsDisabled),0,0,
				MSG(MSetColorDialogListGrayed),0,0,
				MSG(MSetColorDialogSelectedListGrayed),0,0,
			};

			// 0,1 - dialog,warn List
			// 2,3 - dialog,warn Combobox
			int ListPaletteItems[4][13]=
			{
				// Listbox
				{
					// normal
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
					COL_DIALOGLISTGRAY,               // "серый"
					COL_DIALOGLISTSELECTEDGRAYTEXT,   // выбранный "серый"
				},
				{
					// warn
					COL_WARNDIALOGLISTTEXT,
					COL_WARNDIALOGLISTHIGHLIGHT,
					COL_WARNDIALOGLISTSELECTEDTEXT,
					COL_WARNDIALOGLISTSELECTEDHIGHLIGHT,
					COL_WARNDIALOGLISTDISABLED,
					COL_WARNDIALOGLISTBOX,
					COL_WARNDIALOGLISTTITLE,
					COL_WARNDIALOGLISTSCROLLBAR,
					COL_WARNDIALOGLISTARROWS,            // Arrow
					COL_WARNDIALOGLISTARROWSSELECTED,    // Выбранный - Arrow
					COL_WARNDIALOGLISTARROWSDISABLED,    // Arrow disabled
					COL_WARNDIALOGLISTGRAY,              // "серый"
					COL_WARNDIALOGLISTSELECTEDGRAYTEXT,  // выбранный "серый"
				},
				// Combobox
				{
					// normal
					COL_DIALOGCOMBOTEXT,
					COL_DIALOGCOMBOHIGHLIGHT,
					COL_DIALOGCOMBOSELECTEDTEXT,
					COL_DIALOGCOMBOSELECTEDHIGHLIGHT,
					COL_DIALOGCOMBODISABLED,
					COL_DIALOGCOMBOBOX,
					COL_DIALOGCOMBOTITLE,
					COL_DIALOGCOMBOSCROLLBAR,
					COL_DIALOGCOMBOARROWS,               // Arrow
					COL_DIALOGCOMBOARROWSSELECTED,       // Выбранный - Arrow
					COL_DIALOGCOMBOARROWSDISABLED,       // Arrow disabled
					COL_DIALOGCOMBOGRAY,                 // "серый"
					COL_DIALOGCOMBOSELECTEDGRAYTEXT,     // выбранный "серый"
				},
				{
					// warn
					COL_WARNDIALOGCOMBOTEXT,
					COL_WARNDIALOGCOMBOHIGHLIGHT,
					COL_WARNDIALOGCOMBOSELECTEDTEXT,
					COL_WARNDIALOGCOMBOSELECTEDHIGHLIGHT,
					COL_WARNDIALOGCOMBODISABLED,
					COL_WARNDIALOGCOMBOBOX,
					COL_WARNDIALOGCOMBOTITLE,
					COL_WARNDIALOGCOMBOSCROLLBAR,
					COL_WARNDIALOGCOMBOARROWS,            // Arrow
					COL_WARNDIALOGCOMBOARROWSSELECTED,    // Выбранный - Arrow
					COL_WARNDIALOGCOMBOARROWSDISABLED,    // Arrow disabled
					COL_WARNDIALOGCOMBOGRAY,              // "серый"
					COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT,  // выбранный "серый"
				},
			};

			SetItemColors(ListItems,ListPaletteItems[PaletteItems[ItemsCode]],ARRAYSIZE(ListItems),2,MenuToRedraw1, MenuToRedraw2);
			MenuToRedraw3=nullptr;
		}
		else
			GetColor(PaletteItems[ItemsCode], MenuToRedraw1, MenuToRedraw2, MenuToRedraw3);
	}
}

int ColorIndex[]=
{
	F_LIGHTGRAY|B_BLACK,
	F_BLACK|B_RED,
	F_LIGHTGRAY|B_DARKGRAY,
	F_BLACK|B_LIGHTRED,
	F_LIGHTGRAY|B_BLUE,
	F_BLACK|B_MAGENTA,
	F_BLACK|B_LIGHTBLUE,
	F_BLACK|B_LIGHTMAGENTA,
	F_BLACK|B_GREEN,
	F_BLACK|B_BROWN,
	F_BLACK|B_LIGHTGREEN,
	F_BLACK|B_YELLOW,
	F_BLACK|B_CYAN,
	F_BLACK|B_LIGHTGRAY,
	F_BLACK|B_LIGHTCYAN,
	F_BLACK|B_WHITE
};

static intptr_t WINAPI GetColorDlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	switch (Msg)
	{
		case DN_CTLCOLORDLGITEM:

			if (Param1 >= 2 && Param1 <= 17) // Fore
			{
				FarDialogItemColors* Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors::ConsoleColorToFarColor(ColorIndex[Param1-2],Colors->Colors[0]);
			}

			if (Param1 >= 19 && Param1 <= 34) // Back
			{
				FarDialogItemColors* Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors::ConsoleColorToFarColor(ColorIndex[Param1-19],Colors->Colors[0]);
			}

			if (Param1 >= 37 && Param1 <= 39)
			{
				FarColor* CurColor=reinterpret_cast<FarColor*>(SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0));
				FarDialogItemColors* Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = *CurColor;
			}
			break;

		case DN_BTNCLICK:

			if (Param1 >= 2 && Param1 <= 34)
			{
				FarColor NewColor;
				FarColor *CurColor = reinterpret_cast<FarColor*>(SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0));
				FarDialogItem DlgItem = {};
				SendDlgMessage(hDlg, DM_GETDLGITEMSHORT, Param1, &DlgItem);
				NewColor=*CurColor;

				if (Param1 >= 2 && Param1 <= 17) // Fore
				{
					UINT B = NewColor.BackgroundColor;
					Colors::ConsoleColorToFarColor(ColorIndex[Param1-2],NewColor);
					NewColor.ForegroundColor = NewColor.BackgroundColor;
					NewColor.BackgroundColor = B;
				}

				if (Param1 >= 19 && Param1 <= 34) // Back
				{
					UINT F = NewColor.ForegroundColor;
					Colors::ConsoleColorToFarColor(ColorIndex[Param1-19],NewColor);
					//NewColor.BackgroundColor = NewColor.ForegroundColor;
					NewColor.ForegroundColor = F;
				}

				if (NewColor.BackgroundColor!=CurColor->BackgroundColor || NewColor.ForegroundColor!=CurColor->ForegroundColor)
					*CurColor=NewColor;

				return TRUE;
			}

			break;
		default:
			break;
	}

	return DefDlgProc(hDlg, Msg, Param1, Param2);
}


bool GetColorDialogInternal(FarColor& Color,bool bCentered,bool bAddTransparent)
{
	FarDialogItem ColorDlgData[]=
	{
		{DI_DOUBLEBOX,   3, 1,35,13, 0,nullptr,nullptr,0,MSG(MSetColorTitle)},
		{DI_SINGLEBOX,   5, 2,18, 7, 0,nullptr,nullptr,0,MSG(MSetColorForeground)},
		{DI_RADIOBUTTON, 6, 3, 0, 3, 0,nullptr,nullptr,DIF_GROUP|DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 6, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 6, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 6, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 9, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 9, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 9, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON, 9, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,12, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,12, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,12, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,12, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,15, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,15, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,15, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,15, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_SINGLEBOX,  20, 2,33, 7, 0,nullptr,nullptr,0,MSG(MSetColorBackground)},
		{DI_RADIOBUTTON,21, 3, 0, 3, 0,nullptr,nullptr,DIF_GROUP|DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,21, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,21, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,21, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,24, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,24, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,24, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,24, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,27, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,27, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,27, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,27, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,30, 3, 0, 3, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,30, 4, 0, 4, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,30, 5, 0, 5, 0,nullptr,nullptr,DIF_MOVESELECT,L""},
		{DI_RADIOBUTTON,30, 6, 0, 6, 0,nullptr,nullptr,DIF_MOVESELECT,L""},

		{DI_CHECKBOX,    5, 10,0, 10,0,nullptr,nullptr,0,MSG(MSetColorForeTransparent)},
		{DI_CHECKBOX,   22, 10,0, 10,0,nullptr,nullptr,0,MSG(MSetColorBackTransparent)},

		{DI_TEXT,        5, 8, 33,8, 0,nullptr,nullptr,0,MSG(MSetColorSample)},
		{DI_TEXT,        5, 9, 33,9, 0,nullptr,nullptr,0,MSG(MSetColorSample)},
		{DI_TEXT,        5,10, 33,10,0,nullptr,nullptr,0,MSG(MSetColorSample)},
		{DI_TEXT,        0,11, 0, 11,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,      0,12, 0, 12,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MSetColorSet)},
		{DI_BUTTON,      0,12, 0, 12,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MSetColorCancel)},

	};
	MakeDialogItemsEx(ColorDlgData,ColorDlg);
	int ExitCode;
	FarColor CurColor=Color;
	int ConsoleColor = Colors::FarColorToConsoleColor(Color);
	for (size_t i=2; i<18; i++)
	{
		if (((ColorIndex[i-2]&B_MASK)>>4) == (ConsoleColor&F_MASK))
		{
			ColorDlg[i].Selected=1;
			ColorDlg[i].Flags|=DIF_FOCUS;
			break;
		}
	}

	for (size_t i=19; i<35; i++)
	{
		if ((ColorIndex[i-19]&B_MASK) == (ConsoleColor&B_MASK))
		{
			ColorDlg[i].Selected=1;
			break;
		}
	}

	if (bAddTransparent)
	{
		ColorDlg[0].Y2++;

		for (size_t i=37; i<=42; i++)
		{
			ColorDlg[i].Y1+=3;
			ColorDlg[i].Y2+=3;
		}

		ColorDlg[0].X2+=4;
		ColorDlg[0].Y2+=2;
		ColorDlg[1].X2+=2;
		ColorDlg[1].Y2+=2;
		ColorDlg[18].X1+=2;
		ColorDlg[18].X2+=4;
		ColorDlg[18].Y2+=2;

		for (size_t i=2; i<=17; i++)
		{
			ColorDlg[i].X1+=1;
			ColorDlg[i].Y1+=1;
			ColorDlg[i].Y2+=1;
		}

		for (size_t i=19; i<=34; i++)
		{
			ColorDlg[i].X1+=3;
			ColorDlg[i].Y1+=1;
			ColorDlg[i].Y2+=1;
		}

		for (size_t i=37; i<=39; i++)
		{
			ColorDlg[i].X2+=4;
		}

		ColorDlg[35].Selected=IS_TRANSPARENT(Color.ForegroundColor);
		ColorDlg[36].Selected=IS_TRANSPARENT(Color.BackgroundColor);
	}
	else
	{
		ColorDlg[35].Flags|=DIF_HIDDEN;
		ColorDlg[36].Flags|=DIF_HIDDEN;
	}

	{
		Dialog Dlg(ColorDlg,ARRAYSIZE(ColorDlg), GetColorDlgProc, &CurColor);

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
		ColorDlg[35].Selected? MAKE_TRANSPARENT(Color.ForegroundColor) : MAKE_OPAQUE(Color.ForegroundColor);
		ColorDlg[36].Selected? MAKE_TRANSPARENT(Color.BackgroundColor) : MAKE_OPAQUE(Color.BackgroundColor);
		return true;
	}

	return false;
}
