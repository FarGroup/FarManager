{
  FarColorW.pas

  Colors Index for Far Manager <%VERSION%>
  HKCU\Software\Far Manager\Colors\CurrentPalette
}

{
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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

EXCEPTION:
Far Manager plugins that use this header file can be distributed under any
other possible license with no implications from the above license on them.
}

unit FarColorW;

interface

{ PaletteColors }

const
   COL_MENUTEXT                          = 0;
   COL_MENUSELECTEDTEXT                  = 1;
   COL_MENUHIGHLIGHT                     = 2;
   COL_MENUSELECTEDHIGHLIGHT             = 3;
   COL_MENUBOX                           = 4;
   COL_MENUTITLE                         = 5;

   COL_HMENUTEXT                         = 6;
   COL_HMENUSELECTEDTEXT                 = 7;
   COL_HMENUHIGHLIGHT                    = 8;
   COL_HMENUSELECTEDHIGHLIGHT            = 9;

   COL_PANELTEXT                         = 10;
   COL_PANELSELECTEDTEXT                 = 11;
   COL_PANELHIGHLIGHTTEXT                = 12;
   COL_PANELINFOTEXT                     = 13;
   COL_PANELCURSOR                       = 14;
   COL_PANELSELECTEDCURSOR               = 15;
   COL_PANELTITLE                        = 16;
   COL_PANELSELECTEDTITLE                = 17;
   COL_PANELCOLUMNTITLE                  = 18;
   COL_PANELTOTALINFO                    = 19;
   COL_PANELSELECTEDINFO                 = 20;

   COL_DIALOGTEXT                        = 21;
   COL_DIALOGHIGHLIGHTTEXT               = 22;
   COL_DIALOGBOX                         = 23;
   COL_DIALOGBOXTITLE                    = 24;
   COL_DIALOGHIGHLIGHTBOXTITLE           = 25;
   COL_DIALOGEDIT                        = 26;
   COL_DIALOGBUTTON                      = 27;
   COL_DIALOGSELECTEDBUTTON              = 28;
   COL_DIALOGHIGHLIGHTBUTTON             = 29;
   COL_DIALOGHIGHLIGHTSELECTEDBUTTON     = 30;

   COL_DIALOGLISTTEXT                    = 31;
   COL_DIALOGLISTSELECTEDTEXT            = 32;
   COL_DIALOGLISTHIGHLIGHT               = 33;
   COL_DIALOGLISTSELECTEDHIGHLIGHT       = 34;

   COL_WARNDIALOGTEXT                    = 35;
   COL_WARNDIALOGHIGHLIGHTTEXT           = 36;
   COL_WARNDIALOGBOX                     = 37;
   COL_WARNDIALOGBOXTITLE                = 38;
   COL_WARNDIALOGHIGHLIGHTBOXTITLE       = 39;
   COL_WARNDIALOGEDIT                    = 40;
   COL_WARNDIALOGBUTTON                  = 41;
   COL_WARNDIALOGSELECTEDBUTTON          = 42;
   COL_WARNDIALOGHIGHLIGHTBUTTON         = 43;
   COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON = 44;

   COL_KEYBARNUM                         = 45;
   COL_KEYBARTEXT                        = 46;
   COL_KEYBARBACKGROUND                  = 47;

   COL_COMMANDLINE                       = 48;

   COL_CLOCK                             = 49;

   COL_VIEWERTEXT                        = 50;
   COL_VIEWERSELECTEDTEXT                = 51;
   COL_VIEWERSTATUS                      = 52;

   COL_EDITORTEXT                        = 53;
   COL_EDITORSELECTEDTEXT                = 54;
   COL_EDITORSTATUS                      = 55;

   COL_HELPTEXT                          = 56;
   COL_HELPHIGHLIGHTTEXT                 = 57;
   COL_HELPTOPIC                         = 58;
   COL_HELPSELECTEDTOPIC                 = 59;
   COL_HELPBOX                           = 60;
   COL_HELPBOXTITLE                      = 61;

   COL_PANELDRAGTEXT                     = 62;
   COL_DIALOGEDITUNCHANGED               = 63;
   COL_PANELSCROLLBAR                    = 64;
   COL_HELPSCROLLBAR                     = 65;
   COL_PANELBOX                          = 66;
   COL_PANELSCREENSNUMBER                = 67;
   COL_DIALOGEDITSELECTED                = 68;
   COL_COMMANDLINESELECTED               = 69;
   COL_VIEWERARROWS                      = 70;

   COL_RESERVED0                         = 71;

   COL_DIALOGLISTSCROLLBAR               = 72;
   COL_MENUSCROLLBAR                     = 73;
   COL_VIEWERSCROLLBAR                   = 74;
   COL_COMMANDLINEPREFIX                 = 75;
   COL_DIALOGDISABLED                    = 76;
   COL_DIALOGEDITDISABLED                = 77;
   COL_DIALOGLISTDISABLED                = 78;
   COL_WARNDIALOGDISABLED                = 79;
   COL_WARNDIALOGEDITDISABLED            = 80;
   COL_WARNDIALOGLISTDISABLED            = 81;

   COL_MENUDISABLEDTEXT                  = 82;

   COL_EDITORCLOCK                       = 83;
   COL_VIEWERCLOCK                       = 84;

   COL_DIALOGLISTTITLE                   = 85;
   COL_DIALOGLISTBOX                     = 86;

   COL_WARNDIALOGEDITSELECTED            = 87;
   COL_WARNDIALOGEDITUNCHANGED           = 88;

   COL_DIALOGCOMBOTEXT                   = 89;
   COL_DIALOGCOMBOSELECTEDTEXT           = 90;
   COL_DIALOGCOMBOHIGHLIGHT              = 91;
   COL_DIALOGCOMBOSELECTEDHIGHLIGHT      = 92;
   COL_DIALOGCOMBOBOX                    = 93;
   COL_DIALOGCOMBOTITLE                  = 94;
   COL_DIALOGCOMBODISABLED               = 95;
   COL_DIALOGCOMBOSCROLLBAR              = 96;

   COL_WARNDIALOGLISTTEXT                = 97;
   COL_WARNDIALOGLISTSELECTEDTEXT        = 98;
   COL_WARNDIALOGLISTHIGHLIGHT           = 99;
   COL_WARNDIALOGLISTSELECTEDHIGHLIGHT   = 100;
   COL_WARNDIALOGLISTBOX                 = 101;
   COL_WARNDIALOGLISTTITLE               = 102;
   COL_WARNDIALOGLISTSCROLLBAR           = 103;

   COL_WARNDIALOGCOMBOTEXT               = 104;
   COL_WARNDIALOGCOMBOSELECTEDTEXT       = 105;
   COL_WARNDIALOGCOMBOHIGHLIGHT          = 106;
   COL_WARNDIALOGCOMBOSELECTEDHIGHLIGHT  = 107;
   COL_WARNDIALOGCOMBOBOX                = 108;
   COL_WARNDIALOGCOMBOTITLE              = 109;
   COL_WARNDIALOGCOMBODISABLED           = 110;
   COL_WARNDIALOGCOMBOSCROLLBAR          = 111;

   COL_DIALOGLISTARROWS                  = 112;
   COL_DIALOGLISTARROWSDISABLED          = 113;
   COL_DIALOGLISTARROWSSELECTED          = 114;
   COL_DIALOGCOMBOARROWS                 = 115;
   COL_DIALOGCOMBOARROWSDISABLED         = 116;
   COL_DIALOGCOMBOARROWSSELECTED         = 117;
   COL_WARNDIALOGLISTARROWS              = 118;
   COL_WARNDIALOGLISTARROWSDISABLED      = 119;
   COL_WARNDIALOGLISTARROWSSELECTED      = 120;
   COL_WARNDIALOGCOMBOARROWS             = 121;
   COL_WARNDIALOGCOMBOARROWSDISABLED     = 122;
   COL_WARNDIALOGCOMBOARROWSSELECTED     = 123;
   COL_MENUARROWS                        = 124;
   COL_MENUARROWSDISABLED                = 125;
   COL_MENUARROWSSELECTED                = 126;
   COL_COMMANDLINEUSERSCREEN             = 127;
   COL_EDITORSCROLLBAR                   = 128;

   COL_MENUGRAYTEXT                      = 129;
   COL_MENUSELECTEDGRAYTEXT              = 130;
   COL_DIALOGCOMBOGRAY                   = 131;
   COL_DIALOGCOMBOSELECTEDGRAYTEXT       = 132;
   COL_DIALOGLISTGRAY                    = 133;
   COL_DIALOGLISTSELECTEDGRAYTEXT        = 134;
   COL_WARNDIALOGCOMBOGRAY               = 135;
   COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT   = 136;
   COL_WARNDIALOGLISTGRAY                = 137;
   COL_WARNDIALOGLISTSELECTEDGRAYTEXT    = 138;

   COL_DIALOGDEFAULTBUTTON                      = 139;
   COL_DIALOGSELECTEDDEFAULTBUTTON              = 140;
   COL_DIALOGHIGHLIGHTDEFAULTBUTTON             = 141;
   COL_DIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON     = 142;
   COL_WARNDIALOGDEFAULTBUTTON                  = 143;
   COL_WARNDIALOGSELECTEDDEFAULTBUTTON          = 144;
   COL_WARNDIALOGHIGHLIGHTDEFAULTBUTTON         = 145;
   COL_WARNDIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON = 146;

   COL_LASTPALETTECOLOR                  = 147;

implementation
end.
