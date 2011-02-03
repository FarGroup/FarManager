{
  FarKeysW.pas

  Internal key names for FAR Manager <%VERSION%>
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

EXCEPTION:
Far Manager plugins that use this header file can be distributed under any
other possible license with no implications from the above license on them.
}

unit FarKeysW;

interface

uses
  Windows;

{ BaseDefKeyboard }

const
   KEY_CTRLMASK            = $FFF00000;
   KEY_CTRL                = $01000000;
   KEY_ALT                 = $02000000;
   KEY_SHIFT               = $04000000;
   KEY_RCTRL               = $10000000;
   KEY_RALT                = $20000000;

   KEY_BRACKET             = Byte('[');
   KEY_BACKBRACKET         = Byte(']');
   KEY_COMMA               = Byte(';');
   KEY_QUOTE               = Byte('"');
   KEY_DOT                 = Byte('.');
   KEY_SLASH               = Byte('/');
   KEY_COLON               = Byte(':');
   KEY_BACKSLASH           = Byte('\');

   KEY_BS                  = $00000008;
   KEY_TAB                 = $00000009;
   KEY_ENTER               = $0000000D;
   KEY_ESC                 = $0000001B;
   KEY_SPACE               = $00000020;

   KEY_MASKF               = $00000FFF;

   EXTENDED_KEY_BASE       = $00010000;
   INTERNAL_KEY_BASE       = $00020000;
   INTERNAL_KEY_BASE_2     = $00030000;

   KEY_FKEY_BEGIN          = EXTENDED_KEY_BASE;

   KEY_BREAK               = EXTENDED_KEY_BASE+VK_CANCEL;

   KEY_PAUSE               = EXTENDED_KEY_BASE+VK_PAUSE;
   KEY_CAPSLOCK            = EXTENDED_KEY_BASE+VK_CAPITAL;

   KEY_PGUP                = EXTENDED_KEY_BASE+VK_PRIOR;
   KEY_PGDN                = EXTENDED_KEY_BASE+VK_NEXT;
   KEY_END                 = EXTENDED_KEY_BASE+VK_END;
   KEY_HOME                = EXTENDED_KEY_BASE+VK_HOME;
   KEY_LEFT                = EXTENDED_KEY_BASE+VK_LEFT;
   KEY_UP                  = EXTENDED_KEY_BASE+VK_UP;
   KEY_RIGHT               = EXTENDED_KEY_BASE+VK_RIGHT;
   KEY_DOWN                = EXTENDED_KEY_BASE+VK_DOWN;
   KEY_PRNTSCRN            = EXTENDED_KEY_BASE+VK_SNAPSHOT;   
   KEY_INS                 = EXTENDED_KEY_BASE+VK_INSERT;
   KEY_DEL                 = EXTENDED_KEY_BASE+VK_DELETE;

   KEY_LWIN                = EXTENDED_KEY_BASE+VK_LWIN;
   KEY_RWIN                = EXTENDED_KEY_BASE+VK_RWIN;
   KEY_APPS                = EXTENDED_KEY_BASE+VK_APPS;
// KEY_STANDBY             = EXTENDED_KEY_BASE+VK_SLEEP;

   KEY_NUMPAD0             = EXTENDED_KEY_BASE+VK_NUMPAD0;
   KEY_NUMPAD1             = EXTENDED_KEY_BASE+VK_NUMPAD1;
   KEY_NUMPAD2             = EXTENDED_KEY_BASE+VK_NUMPAD2;
   KEY_NUMPAD3             = EXTENDED_KEY_BASE+VK_NUMPAD3;
   KEY_NUMPAD4             = EXTENDED_KEY_BASE+VK_NUMPAD4;
   KEY_NUMPAD5             = EXTENDED_KEY_BASE+VK_NUMPAD5;
   KEY_CLEAR               = KEY_NUMPAD5;
   KEY_NUMPAD6             = EXTENDED_KEY_BASE+VK_NUMPAD6;
   KEY_NUMPAD7             = EXTENDED_KEY_BASE+VK_NUMPAD7;
   KEY_NUMPAD8             = EXTENDED_KEY_BASE+VK_NUMPAD8;
   KEY_NUMPAD9             = EXTENDED_KEY_BASE+VK_NUMPAD9;

   KEY_MULTIPLY            = EXTENDED_KEY_BASE+VK_MULTIPLY;
   KEY_ADD                 = EXTENDED_KEY_BASE+VK_ADD;
   KEY_SUBTRACT            = EXTENDED_KEY_BASE+VK_SUBTRACT;
   KEY_DECIMAL             = EXTENDED_KEY_BASE+VK_DECIMAL;
   KEY_DIVIDE              = EXTENDED_KEY_BASE+VK_DIVIDE;

   KEY_F1                  = EXTENDED_KEY_BASE+VK_F1;
   KEY_F2                  = EXTENDED_KEY_BASE+VK_F2;
   KEY_F3                  = EXTENDED_KEY_BASE+VK_F3;
   KEY_F4                  = EXTENDED_KEY_BASE+VK_F4;
   KEY_F5                  = EXTENDED_KEY_BASE+VK_F5;
   KEY_F6                  = EXTENDED_KEY_BASE+VK_F6;
   KEY_F7                  = EXTENDED_KEY_BASE+VK_F7;
   KEY_F8                  = EXTENDED_KEY_BASE+VK_F8;
   KEY_F9                  = EXTENDED_KEY_BASE+VK_F9;
   KEY_F10                 = EXTENDED_KEY_BASE+VK_F10;
   KEY_F11                 = EXTENDED_KEY_BASE+VK_F11;
   KEY_F12                 = EXTENDED_KEY_BASE+VK_F12;

   KEY_F13                 = EXTENDED_KEY_BASE+VK_F13;
   KEY_F14                 = EXTENDED_KEY_BASE+VK_F14;
   KEY_F15                 = EXTENDED_KEY_BASE+VK_F15;
   KEY_F16                 = EXTENDED_KEY_BASE+VK_F16;
   KEY_F17                 = EXTENDED_KEY_BASE+VK_F17;
   KEY_F18                 = EXTENDED_KEY_BASE+VK_F18;
   KEY_F19                 = EXTENDED_KEY_BASE+VK_F19;
   KEY_F20                 = EXTENDED_KEY_BASE+VK_F20;
   KEY_F21                 = EXTENDED_KEY_BASE+VK_F21;
   KEY_F22                 = EXTENDED_KEY_BASE+VK_F22;
   KEY_F23                 = EXTENDED_KEY_BASE+VK_F23;
   KEY_F24                 = EXTENDED_KEY_BASE+VK_F24;

   KEY_NUMLOCK             = EXTENDED_KEY_BASE+VK_NUMLOCK;
   KEY_SCROLLLOCK          = EXTENDED_KEY_BASE+VK_SCROLL;
(*
   KEY_BROWSER_BACK        = EXTENDED_KEY_BASE+VK_BROWSER_BACK;
   KEY_BROWSER_FORWARD     = EXTENDED_KEY_BASE+VK_BROWSER_FORWARD;
   KEY_BROWSER_REFRESH     = EXTENDED_KEY_BASE+VK_BROWSER_REFRESH;
   KEY_BROWSER_STOP        = EXTENDED_KEY_BASE+VK_BROWSER_STOP;
   KEY_BROWSER_SEARCH      = EXTENDED_KEY_BASE+VK_BROWSER_SEARCH;
   KEY_BROWSER_FAVORITES   = EXTENDED_KEY_BASE+VK_BROWSER_FAVORITES;
   KEY_BROWSER_HOME        = EXTENDED_KEY_BASE+VK_BROWSER_HOME;
   KEY_VOLUME_MUTE         = EXTENDED_KEY_BASE+VK_VOLUME_MUTE;
   KEY_VOLUME_DOWN         = EXTENDED_KEY_BASE+VK_VOLUME_DOWN;
   KEY_VOLUME_UP           = EXTENDED_KEY_BASE+VK_VOLUME_UP;
   KEY_MEDIA_NEXT_TRACK    = EXTENDED_KEY_BASE+VK_MEDIA_NEXT_TRACK;
   KEY_MEDIA_PREV_TRACK    = EXTENDED_KEY_BASE+VK_MEDIA_PREV_TRACK;
   KEY_MEDIA_STOP          = EXTENDED_KEY_BASE+VK_MEDIA_STOP;
   KEY_MEDIA_PLAY_PAUSE    = EXTENDED_KEY_BASE+VK_MEDIA_PLAY_PAUSE;
   KEY_LAUNCH_MAIL         = EXTENDED_KEY_BASE+VK_LAUNCH_MAIL;
   KEY_LAUNCH_MEDIA_SELECT = EXTENDED_KEY_BASE+VK_LAUNCH_MEDIA_SELECT;
   KEY_LAUNCH_APP1         = EXTENDED_KEY_BASE+VK_LAUNCH_APP1;
   KEY_LAUNCH_APP2         = EXTENDED_KEY_BASE+VK_LAUNCH_APP2;
*)
   KEY_CTRLALTSHIFTPRESS   = INTERNAL_KEY_BASE+1;
   KEY_CTRLALTSHIFTRELEASE = INTERNAL_KEY_BASE+2;

   KEY_MSWHEEL_UP          = INTERNAL_KEY_BASE+3;
   KEY_MSWHEEL_DOWN        = INTERNAL_KEY_BASE+4;

   KEY_NUMDEL              = INTERNAL_KEY_BASE+9;
   KEY_NUMENTER            = INTERNAL_KEY_BASE+$0B;

   KEY_MSWHEEL_LEFT        = INTERNAL_KEY_BASE+$0C;
   KEY_MSWHEEL_RIGHT       = INTERNAL_KEY_BASE+$0D;

   KEY_MSLCLICK            = INTERNAL_KEY_BASE+$0F;
   KEY_MSRCLICK            = INTERNAL_KEY_BASE+$10;

   KEY_MSM1CLICK           = INTERNAL_KEY_BASE+$11;
   KEY_MSM2CLICK           = INTERNAL_KEY_BASE+$12;
   KEY_MSM3CLICK           = INTERNAL_KEY_BASE+$13;

   KEY_VK_0xFF_BEGIN       = EXTENDED_KEY_BASE+$000000100;
   KEY_VK_0xFF_END         = EXTENDED_KEY_BASE+$0000001FF;

   KEY_END_FKEY            = $0001FFFF;

   KEY_NONE                = INTERNAL_KEY_BASE_2+1;
   KEY_IDLE                = INTERNAL_KEY_BASE_2+2;

   KEY_KILLFOCUS           = INTERNAL_KEY_BASE_2+6;
   KEY_GOTFOCUS            = INTERNAL_KEY_BASE_2+7;
   KEY_CONSOLE_BUFFER_RESIZE = INTERNAL_KEY_BASE_2+8;

   KEY_END_SKEY            = $0003FFFF;
   KEY_LAST_BASE           = KEY_END_SKEY;

 { AddDefKeyboard }

 const
   KEY_CTRLSHIFT            = KEY_CTRL or KEY_SHIFT;
   KEY_ALTSHIFT             = KEY_ALT or KEY_SHIFT;
   KEY_CTRLALT              = KEY_CTRL or KEY_ALT;

   KEY_CTRL0                = KEY_CTRL+Byte('0');
   KEY_CTRL1                = KEY_CTRL+Byte('1');
   KEY_CTRL2                = KEY_CTRL+Byte('2');
   KEY_CTRL3                = KEY_CTRL+Byte('3');
   KEY_CTRL4                = KEY_CTRL+Byte('4');
   KEY_CTRL5                = KEY_CTRL+Byte('5');
   KEY_CTRL6                = KEY_CTRL+Byte('6');
   KEY_CTRL7                = KEY_CTRL+Byte('7');
   KEY_CTRL8                = KEY_CTRL+Byte('8');
   KEY_CTRL9                = KEY_CTRL+Byte('9');

   KEY_RCTRL0               = KEY_RCTRL+Byte('0');
   KEY_RCTRL1               = KEY_RCTRL+Byte('1');
   KEY_RCTRL2               = KEY_RCTRL+Byte('2');
   KEY_RCTRL3               = KEY_RCTRL+Byte('3');
   KEY_RCTRL4               = KEY_RCTRL+Byte('4');
   KEY_RCTRL5               = KEY_RCTRL+Byte('5');
   KEY_RCTRL6               = KEY_RCTRL+Byte('6');
   KEY_RCTRL7               = KEY_RCTRL+Byte('7');
   KEY_RCTRL8               = KEY_RCTRL+Byte('8');
   KEY_RCTRL9               = KEY_RCTRL+Byte('9');

   KEY_CTRLA                = KEY_CTRL+Byte('A');
   KEY_CTRLB                = KEY_CTRL+Byte('B');
   KEY_CTRLC                = KEY_CTRL+Byte('C');
   KEY_CTRLD                = KEY_CTRL+Byte('D');
   KEY_CTRLE                = KEY_CTRL+Byte('E');
   KEY_CTRLF                = KEY_CTRL+Byte('F');
   KEY_CTRLG                = KEY_CTRL+Byte('G');
   KEY_CTRLH                = KEY_CTRL+Byte('H');
   KEY_CTRLI                = KEY_CTRL+Byte('I');
   KEY_CTRLJ                = KEY_CTRL+Byte('J');
   KEY_CTRLK                = KEY_CTRL+Byte('K');
   KEY_CTRLL                = KEY_CTRL+Byte('L');
   KEY_CTRLM                = KEY_CTRL+Byte('M');
   KEY_CTRLN                = KEY_CTRL+Byte('N');
   KEY_CTRLO                = KEY_CTRL+Byte('O');
   KEY_CTRLP                = KEY_CTRL+Byte('P');
   KEY_CTRLQ                = KEY_CTRL+Byte('Q');
   KEY_CTRLR                = KEY_CTRL+Byte('R');
   KEY_CTRLS                = KEY_CTRL+Byte('S');
   KEY_CTRLT                = KEY_CTRL+Byte('T');
   KEY_CTRLU                = KEY_CTRL+Byte('U');
   KEY_CTRLV                = KEY_CTRL+Byte('V');
   KEY_CTRLW                = KEY_CTRL+Byte('W');
   KEY_CTRLX                = KEY_CTRL+Byte('X');
   KEY_CTRLY                = KEY_CTRL+Byte('Y');
   KEY_CTRLZ                = KEY_CTRL+Byte('Z');

   KEY_CTRLBRACKET          = KEY_CTRL or KEY_BRACKET;
   KEY_CTRLBACKBRACKET      = KEY_CTRL or KEY_BACKBRACKET;
   KEY_CTRLCOMMA            = KEY_CTRL or KEY_COMMA;
   KEY_CTRLQUOTE            = KEY_CTRL or KEY_QUOTE;
   KEY_CTRLDOT              = KEY_CTRL or KEY_DOT;

   KEY_ALT0                 = KEY_ALT+Byte('0');
   KEY_ALT1                 = KEY_ALT+Byte('1');
   KEY_ALT2                 = KEY_ALT+Byte('2');
   KEY_ALT3                 = KEY_ALT+Byte('3');
   KEY_ALT4                 = KEY_ALT+Byte('4');
   KEY_ALT5                 = KEY_ALT+Byte('5');
   KEY_ALT6                 = KEY_ALT+Byte('6');
   KEY_ALT7                 = KEY_ALT+Byte('7');
   KEY_ALT8                 = KEY_ALT+Byte('8');
   KEY_ALT9                 = KEY_ALT+Byte('9');

   KEY_ALTADD               = KEY_ALT or KEY_ADD;
   KEY_ALTDOT               = KEY_ALT or KEY_DOT;
   KEY_ALTCOMMA             = KEY_ALT or KEY_COMMA;
   KEY_ALTMULTIPLY          = KEY_ALT or KEY_MULTIPLY;

   KEY_ALTA                 = KEY_ALT+Byte('A');
   KEY_ALTB                 = KEY_ALT+Byte('B');
   KEY_ALTC                 = KEY_ALT+Byte('C');
   KEY_ALTD                 = KEY_ALT+Byte('D');
   KEY_ALTE                 = KEY_ALT+Byte('E');
   KEY_ALTF                 = KEY_ALT+Byte('F');
   KEY_ALTG                 = KEY_ALT+Byte('G');
   KEY_ALTH                 = KEY_ALT+Byte('H');
   KEY_ALTI                 = KEY_ALT+Byte('I');
   KEY_ALTJ                 = KEY_ALT+Byte('J');
   KEY_ALTK                 = KEY_ALT+Byte('K');
   KEY_ALTL                 = KEY_ALT+Byte('L');
   KEY_ALTM                 = KEY_ALT+Byte('M');
   KEY_ALTN                 = KEY_ALT+Byte('N');
   KEY_ALTO                 = KEY_ALT+Byte('O');
   KEY_ALTP                 = KEY_ALT+Byte('P');
   KEY_ALTQ                 = KEY_ALT+Byte('Q');
   KEY_ALTR                 = KEY_ALT+Byte('R');
   KEY_ALTS                 = KEY_ALT+Byte('S');
   KEY_ALTT                 = KEY_ALT+Byte('T');
   KEY_ALTU                 = KEY_ALT+Byte('U');
   KEY_ALTV                 = KEY_ALT+Byte('V');
   KEY_ALTW                 = KEY_ALT+Byte('W');
   KEY_ALTX                 = KEY_ALT+Byte('X');
   KEY_ALTY                 = KEY_ALT+Byte('Y');
   KEY_ALTZ                 = KEY_ALT+Byte('Z');

   KEY_CTRLSHIFTADD         = KEY_CTRL or KEY_SHIFT or KEY_ADD;
   KEY_CTRLSHIFTSUBTRACT    = KEY_CTRL or KEY_SHIFT or KEY_SUBTRACT;
   KEY_CTRLSHIFTDOT         = KEY_CTRL or KEY_SHIFT or KEY_DOT;
   KEY_CTRLSHIFTSLASH       = KEY_CTRL or KEY_SHIFT or KEY_SLASH;

   KEY_CTRLSHIFT0           = KEY_CTRL or KEY_SHIFT+Byte('0');
   KEY_CTRLSHIFT1           = KEY_CTRL or KEY_SHIFT+Byte('1');
   KEY_CTRLSHIFT2           = KEY_CTRL or KEY_SHIFT+Byte('2');
   KEY_CTRLSHIFT3           = KEY_CTRL or KEY_SHIFT+Byte('3');
   KEY_CTRLSHIFT4           = KEY_CTRL or KEY_SHIFT+Byte('4');
   KEY_CTRLSHIFT5           = KEY_CTRL or KEY_SHIFT+Byte('5');
   KEY_CTRLSHIFT6           = KEY_CTRL or KEY_SHIFT+Byte('6');
   KEY_CTRLSHIFT7           = KEY_CTRL or KEY_SHIFT+Byte('7');
   KEY_CTRLSHIFT8           = KEY_CTRL or KEY_SHIFT+Byte('8');
   KEY_CTRLSHIFT9           = KEY_CTRL or KEY_SHIFT+Byte('9');

   KEY_RCTRLSHIFT0          = KEY_RCTRL+KEY_SHIFT+Byte('0');
   KEY_RCTRLSHIFT1          = KEY_RCTRL+KEY_SHIFT+Byte('1');
   KEY_RCTRLSHIFT2          = KEY_RCTRL+KEY_SHIFT+Byte('2');
   KEY_RCTRLSHIFT3          = KEY_RCTRL+KEY_SHIFT+Byte('3');
   KEY_RCTRLSHIFT4          = KEY_RCTRL+KEY_SHIFT+Byte('4');
   KEY_RCTRLSHIFT5          = KEY_RCTRL+KEY_SHIFT+Byte('5');
   KEY_RCTRLSHIFT6          = KEY_RCTRL+KEY_SHIFT+Byte('6');
   KEY_RCTRLSHIFT7          = KEY_RCTRL+KEY_SHIFT+Byte('7');
   KEY_RCTRLSHIFT8          = KEY_RCTRL+KEY_SHIFT+Byte('8');
   KEY_RCTRLSHIFT9          = KEY_RCTRL+KEY_SHIFT+Byte('9');

   KEY_CTRLSHIFTA           = KEY_CTRL or KEY_SHIFT+Byte('A');
   KEY_CTRLSHIFTB           = KEY_CTRL or KEY_SHIFT+Byte('B');
   KEY_CTRLSHIFTC           = KEY_CTRL or KEY_SHIFT+Byte('C');
   KEY_CTRLSHIFTD           = KEY_CTRL or KEY_SHIFT+Byte('D');
   KEY_CTRLSHIFTE           = KEY_CTRL or KEY_SHIFT+Byte('E');
   KEY_CTRLSHIFTF           = KEY_CTRL or KEY_SHIFT+Byte('F');
   KEY_CTRLSHIFTG           = KEY_CTRL or KEY_SHIFT+Byte('G');
   KEY_CTRLSHIFTH           = KEY_CTRL or KEY_SHIFT+Byte('H');
   KEY_CTRLSHIFTI           = KEY_CTRL or KEY_SHIFT+Byte('I');
   KEY_CTRLSHIFTJ           = KEY_CTRL or KEY_SHIFT+Byte('J');
   KEY_CTRLSHIFTK           = KEY_CTRL or KEY_SHIFT+Byte('K');
   KEY_CTRLSHIFTL           = KEY_CTRL or KEY_SHIFT+Byte('L');
   KEY_CTRLSHIFTM           = KEY_CTRL or KEY_SHIFT+Byte('M');
   KEY_CTRLSHIFTN           = KEY_CTRL or KEY_SHIFT+Byte('N');
   KEY_CTRLSHIFTO           = KEY_CTRL or KEY_SHIFT+Byte('O');
   KEY_CTRLSHIFTP           = KEY_CTRL or KEY_SHIFT+Byte('P');
   KEY_CTRLSHIFTQ           = KEY_CTRL or KEY_SHIFT+Byte('Q');
   KEY_CTRLSHIFTR           = KEY_CTRL or KEY_SHIFT+Byte('R');
   KEY_CTRLSHIFTS           = KEY_CTRL or KEY_SHIFT+Byte('S');
   KEY_CTRLSHIFTT           = KEY_CTRL or KEY_SHIFT+Byte('T');
   KEY_CTRLSHIFTU           = KEY_CTRL or KEY_SHIFT+Byte('U');
   KEY_CTRLSHIFTV           = KEY_CTRL or KEY_SHIFT+Byte('V');
   KEY_CTRLSHIFTW           = KEY_CTRL or KEY_SHIFT+Byte('W');
   KEY_CTRLSHIFTX           = KEY_CTRL or KEY_SHIFT+Byte('X');
   KEY_CTRLSHIFTY           = KEY_CTRL or KEY_SHIFT+Byte('Y');
   KEY_CTRLSHIFTZ           = KEY_CTRL or KEY_SHIFT+Byte('Z');

   KEY_CTRLSHIFTBRACKET     = KEY_CTRL or KEY_SHIFT or KEY_BRACKET;
   KEY_CTRLSHIFTBACKSLASH   = KEY_CTRL or KEY_SHIFT or KEY_BACKSLASH;
   KEY_CTRLSHIFTBACKBRACKET = KEY_CTRL or KEY_SHIFT or KEY_BACKBRACKET;

   KEY_ALTSHIFT0            = KEY_ALT or KEY_SHIFT+Byte('0');
   KEY_ALTSHIFT1            = KEY_ALT or KEY_SHIFT+Byte('1');
   KEY_ALTSHIFT2            = KEY_ALT or KEY_SHIFT+Byte('2');
   KEY_ALTSHIFT3            = KEY_ALT or KEY_SHIFT+Byte('3');
   KEY_ALTSHIFT4            = KEY_ALT or KEY_SHIFT+Byte('4');
   KEY_ALTSHIFT5            = KEY_ALT or KEY_SHIFT+Byte('5');
   KEY_ALTSHIFT6            = KEY_ALT or KEY_SHIFT+Byte('6');
   KEY_ALTSHIFT7            = KEY_ALT or KEY_SHIFT+Byte('7');
   KEY_ALTSHIFT8            = KEY_ALT or KEY_SHIFT+Byte('8');
   KEY_ALTSHIFT9            = KEY_ALT or KEY_SHIFT+Byte('9');

   KEY_ALTSHIFTA            = KEY_ALT or KEY_SHIFT+Byte('A');
   KEY_ALTSHIFTB            = KEY_ALT or KEY_SHIFT+Byte('B');
   KEY_ALTSHIFTC            = KEY_ALT or KEY_SHIFT+Byte('C');
   KEY_ALTSHIFTD            = KEY_ALT or KEY_SHIFT+Byte('D');
   KEY_ALTSHIFTE            = KEY_ALT or KEY_SHIFT+Byte('E');
   KEY_ALTSHIFTF            = KEY_ALT or KEY_SHIFT+Byte('F');
   KEY_ALTSHIFTG            = KEY_ALT or KEY_SHIFT+Byte('G');
   KEY_ALTSHIFTH            = KEY_ALT or KEY_SHIFT+Byte('H');
   KEY_ALTSHIFTI            = KEY_ALT or KEY_SHIFT+Byte('I');
   KEY_ALTSHIFTJ            = KEY_ALT or KEY_SHIFT+Byte('J');
   KEY_ALTSHIFTK            = KEY_ALT or KEY_SHIFT+Byte('K');
   KEY_ALTSHIFTL            = KEY_ALT or KEY_SHIFT+Byte('L');
   KEY_ALTSHIFTM            = KEY_ALT or KEY_SHIFT+Byte('M');
   KEY_ALTSHIFTN            = KEY_ALT or KEY_SHIFT+Byte('N');
   KEY_ALTSHIFTO            = KEY_ALT or KEY_SHIFT+Byte('O');
   KEY_ALTSHIFTP            = KEY_ALT or KEY_SHIFT+Byte('P');
   KEY_ALTSHIFTQ            = KEY_ALT or KEY_SHIFT+Byte('Q');
   KEY_ALTSHIFTR            = KEY_ALT or KEY_SHIFT+Byte('R');
   KEY_ALTSHIFTS            = KEY_ALT or KEY_SHIFT+Byte('S');
   KEY_ALTSHIFTT            = KEY_ALT or KEY_SHIFT+Byte('T');
   KEY_ALTSHIFTU            = KEY_ALT or KEY_SHIFT+Byte('U');
   KEY_ALTSHIFTV            = KEY_ALT or KEY_SHIFT+Byte('V');
   KEY_ALTSHIFTW            = KEY_ALT or KEY_SHIFT+Byte('W');
   KEY_ALTSHIFTX            = KEY_ALT or KEY_SHIFT+Byte('X');
   KEY_ALTSHIFTY            = KEY_ALT or KEY_SHIFT+Byte('Y');
   KEY_ALTSHIFTZ            = KEY_ALT or KEY_SHIFT+Byte('Z');

   KEY_ALTSHIFTBRACKET      = KEY_ALT or KEY_SHIFT or KEY_BRACKET;
   KEY_ALTSHIFTBACKBRACKET  = KEY_ALT or KEY_SHIFT or KEY_BACKBRACKET;

   KEY_CTRLALT0             = KEY_CTRL or KEY_ALT+Byte('0');
   KEY_CTRLALT1             = KEY_CTRL or KEY_ALT+Byte('1');
   KEY_CTRLALT2             = KEY_CTRL or KEY_ALT+Byte('2');
   KEY_CTRLALT3             = KEY_CTRL or KEY_ALT+Byte('3');
   KEY_CTRLALT4             = KEY_CTRL or KEY_ALT+Byte('4');
   KEY_CTRLALT5             = KEY_CTRL or KEY_ALT+Byte('5');
   KEY_CTRLALT6             = KEY_CTRL or KEY_ALT+Byte('6');
   KEY_CTRLALT7             = KEY_CTRL or KEY_ALT+Byte('7');
   KEY_CTRLALT8             = KEY_CTRL or KEY_ALT+Byte('8');
   KEY_CTRLALT9             = KEY_CTRL or KEY_ALT+Byte('9');

   KEY_CTRLALTA             = KEY_CTRL or KEY_ALT+Byte('A');
   KEY_CTRLALTB             = KEY_CTRL or KEY_ALT+Byte('B');
   KEY_CTRLALTC             = KEY_CTRL or KEY_ALT+Byte('C');
   KEY_CTRLALTD             = KEY_CTRL or KEY_ALT+Byte('D');
   KEY_CTRLALTE             = KEY_CTRL or KEY_ALT+Byte('E');
   KEY_CTRLALTF             = KEY_CTRL or KEY_ALT+Byte('F');
   KEY_CTRLALTG             = KEY_CTRL or KEY_ALT+Byte('G');
   KEY_CTRLALTH             = KEY_CTRL or KEY_ALT+Byte('H');
   KEY_CTRLALTI             = KEY_CTRL or KEY_ALT+Byte('I');
   KEY_CTRLALTJ             = KEY_CTRL or KEY_ALT+Byte('J');
   KEY_CTRLALTK             = KEY_CTRL or KEY_ALT+Byte('K');
   KEY_CTRLALTL             = KEY_CTRL or KEY_ALT+Byte('L');
   KEY_CTRLALTM             = KEY_CTRL or KEY_ALT+Byte('M');
   KEY_CTRLALTN             = KEY_CTRL or KEY_ALT+Byte('N');
   KEY_CTRLALTO             = KEY_CTRL or KEY_ALT+Byte('O');
   KEY_CTRLALTP             = KEY_CTRL or KEY_ALT+Byte('P');
   KEY_CTRLALTQ             = KEY_CTRL or KEY_ALT+Byte('Q');
   KEY_CTRLALTR             = KEY_CTRL or KEY_ALT+Byte('R');
   KEY_CTRLALTS             = KEY_CTRL or KEY_ALT+Byte('S');
   KEY_CTRLALTT             = KEY_CTRL or KEY_ALT+Byte('T');
   KEY_CTRLALTU             = KEY_CTRL or KEY_ALT+Byte('U');
   KEY_CTRLALTV             = KEY_CTRL or KEY_ALT+Byte('V');
   KEY_CTRLALTW             = KEY_CTRL or KEY_ALT+Byte('W');
   KEY_CTRLALTX             = KEY_CTRL or KEY_ALT+Byte('X');
   KEY_CTRLALTY             = KEY_CTRL or KEY_ALT+Byte('Y');
   KEY_CTRLALTZ             = KEY_CTRL or KEY_ALT+Byte('Z');

   KEY_CTRLALTBRACKET       = KEY_CTRL or KEY_ALT or KEY_BRACKET;
   KEY_CTRLALTBACKBRACKET   = KEY_CTRL or KEY_ALT or KEY_BACKBRACKET;

   KEY_CTRLF1               = KEY_CTRL or KEY_F1;
   KEY_CTRLF2               = KEY_CTRL or KEY_F2;
   KEY_CTRLF3               = KEY_CTRL or KEY_F3;
   KEY_CTRLF4               = KEY_CTRL or KEY_F4;
   KEY_CTRLF5               = KEY_CTRL or KEY_F5;
   KEY_CTRLF6               = KEY_CTRL or KEY_F6;
   KEY_CTRLF7               = KEY_CTRL or KEY_F7;
   KEY_CTRLF8               = KEY_CTRL or KEY_F8;
   KEY_CTRLF9               = KEY_CTRL or KEY_F9;
   KEY_CTRLF10              = KEY_CTRL or KEY_F10;
   KEY_CTRLF11              = KEY_CTRL or KEY_F11;
   KEY_CTRLF12              = KEY_CTRL or KEY_F12;

   KEY_SHIFTF1              = KEY_SHIFT or KEY_F1;
   KEY_SHIFTF2              = KEY_SHIFT or KEY_F2;
   KEY_SHIFTF3              = KEY_SHIFT or KEY_F3;
   KEY_SHIFTF4              = KEY_SHIFT or KEY_F4;
   KEY_SHIFTF5              = KEY_SHIFT or KEY_F5;
   KEY_SHIFTF6              = KEY_SHIFT or KEY_F6;
   KEY_SHIFTF7              = KEY_SHIFT or KEY_F7;
   KEY_SHIFTF8              = KEY_SHIFT or KEY_F8;
   KEY_SHIFTF9              = KEY_SHIFT or KEY_F9;
   KEY_SHIFTF10             = KEY_SHIFT or KEY_F10;
   KEY_SHIFTF11             = KEY_SHIFT or KEY_F11;
   KEY_SHIFTF12             = KEY_SHIFT or KEY_F12;

   KEY_ALTF1                = KEY_ALT or KEY_F1;
   KEY_ALTF2                = KEY_ALT or KEY_F2;
   KEY_ALTF3                = KEY_ALT or KEY_F3;
   KEY_ALTF4                = KEY_ALT or KEY_F4;
   KEY_ALTF5                = KEY_ALT or KEY_F5;
   KEY_ALTF6                = KEY_ALT or KEY_F6;
   KEY_ALTF7                = KEY_ALT or KEY_F7;
   KEY_ALTF8                = KEY_ALT or KEY_F8;
   KEY_ALTF9                = KEY_ALT or KEY_F9;
   KEY_ALTF10               = KEY_ALT or KEY_F10;
   KEY_ALTF11               = KEY_ALT or KEY_F11;
   KEY_ALTF12               = KEY_ALT or KEY_F12;

   KEY_CTRLSHIFTF1          = KEY_CTRL or KEY_SHIFT or KEY_F1;
   KEY_CTRLSHIFTF2          = KEY_CTRL or KEY_SHIFT or KEY_F2;
   KEY_CTRLSHIFTF3          = KEY_CTRL or KEY_SHIFT or KEY_F3;
   KEY_CTRLSHIFTF4          = KEY_CTRL or KEY_SHIFT or KEY_F4;
   KEY_CTRLSHIFTF5          = KEY_CTRL or KEY_SHIFT or KEY_F5;
   KEY_CTRLSHIFTF6          = KEY_CTRL or KEY_SHIFT or KEY_F6;
   KEY_CTRLSHIFTF7          = KEY_CTRL or KEY_SHIFT or KEY_F7;
   KEY_CTRLSHIFTF8          = KEY_CTRL or KEY_SHIFT or KEY_F8;
   KEY_CTRLSHIFTF9          = KEY_CTRL or KEY_SHIFT or KEY_F9;
   KEY_CTRLSHIFTF10         = KEY_CTRL or KEY_SHIFT or KEY_F10;
   KEY_CTRLSHIFTF11         = KEY_CTRL or KEY_SHIFT or KEY_F11;
   KEY_CTRLSHIFTF12         = KEY_CTRL or KEY_SHIFT or KEY_F12;

   KEY_ALTSHIFTF1           = KEY_ALT or KEY_SHIFT or KEY_F1;
   KEY_ALTSHIFTF2           = KEY_ALT or KEY_SHIFT or KEY_F2;
   KEY_ALTSHIFTF3           = KEY_ALT or KEY_SHIFT or KEY_F3;
   KEY_ALTSHIFTF4           = KEY_ALT or KEY_SHIFT or KEY_F4;
   KEY_ALTSHIFTF5           = KEY_ALT or KEY_SHIFT or KEY_F5;
   KEY_ALTSHIFTF6           = KEY_ALT or KEY_SHIFT or KEY_F6;
   KEY_ALTSHIFTF7           = KEY_ALT or KEY_SHIFT or KEY_F7;
   KEY_ALTSHIFTF8           = KEY_ALT or KEY_SHIFT or KEY_F8;
   KEY_ALTSHIFTF9           = KEY_ALT or KEY_SHIFT or KEY_F9;
   KEY_ALTSHIFTF10          = KEY_ALT or KEY_SHIFT or KEY_F10;
   KEY_ALTSHIFTF11          = KEY_ALT or KEY_SHIFT or KEY_F11;
   KEY_ALTSHIFTF12          = KEY_ALT or KEY_SHIFT or KEY_F12;

   KEY_CTRLALTF1            = KEY_CTRL or KEY_ALT or KEY_F1;
   KEY_CTRLALTF2            = KEY_CTRL or KEY_ALT or KEY_F2;
   KEY_CTRLALTF3            = KEY_CTRL or KEY_ALT or KEY_F3;
   KEY_CTRLALTF4            = KEY_CTRL or KEY_ALT or KEY_F4;
   KEY_CTRLALTF5            = KEY_CTRL or KEY_ALT or KEY_F5;
   KEY_CTRLALTF6            = KEY_CTRL or KEY_ALT or KEY_F6;
   KEY_CTRLALTF7            = KEY_CTRL or KEY_ALT or KEY_F7;
   KEY_CTRLALTF8            = KEY_CTRL or KEY_ALT or KEY_F8;
   KEY_CTRLALTF9            = KEY_CTRL or KEY_ALT or KEY_F9;
   KEY_CTRLALTF10           = KEY_CTRL or KEY_ALT or KEY_F10;
   KEY_CTRLALTF11           = KEY_CTRL or KEY_ALT or KEY_F11;
   KEY_CTRLALTF12           = KEY_CTRL or KEY_ALT or KEY_F12;

   KEY_CTRLHOME             = KEY_CTRL or KEY_HOME;
   KEY_CTRLUP               = KEY_CTRL or KEY_UP;
   KEY_CTRLPGUP             = KEY_CTRL or KEY_PGUP;
   KEY_CTRLLEFT             = KEY_CTRL or KEY_LEFT;
   KEY_CTRLRIGHT            = KEY_CTRL or KEY_RIGHT;
   KEY_CTRLEND              = KEY_CTRL or KEY_END;
   KEY_CTRLDOWN             = KEY_CTRL or KEY_DOWN;
   KEY_CTRLPGDN             = KEY_CTRL or KEY_PGDN;
   KEY_CTRLINS              = KEY_CTRL or KEY_INS;
   KEY_CTRLDEL              = KEY_CTRL or KEY_DEL;

   KEY_SHIFTHOME            = KEY_SHIFT or KEY_HOME;
   KEY_SHIFTUP              = KEY_SHIFT or KEY_UP;
   KEY_SHIFTPGUP            = KEY_SHIFT or KEY_PGUP;
   KEY_SHIFTLEFT            = KEY_SHIFT or KEY_LEFT;
   KEY_SHIFTRIGHT           = KEY_SHIFT or KEY_RIGHT;
   KEY_SHIFTEND             = KEY_SHIFT or KEY_END;
   KEY_SHIFTDOWN            = KEY_SHIFT or KEY_DOWN;
   KEY_SHIFTPGDN            = KEY_SHIFT or KEY_PGDN;
   KEY_SHIFTINS             = KEY_SHIFT or KEY_INS;
   KEY_SHIFTDEL             = KEY_SHIFT or KEY_DEL;

   KEY_ALTHOME              = KEY_ALT or KEY_HOME;
   KEY_ALTUP                = KEY_ALT or KEY_UP;
   KEY_ALTPGUP              = KEY_ALT or KEY_PGUP;
   KEY_ALTLEFT              = KEY_ALT or KEY_LEFT;
   KEY_ALTRIGHT             = KEY_ALT or KEY_RIGHT;
   KEY_ALTEND               = KEY_ALT or KEY_END;
   KEY_ALTDOWN              = KEY_ALT or KEY_DOWN;
   KEY_ALTPGDN              = KEY_ALT or KEY_PGDN;
   KEY_ALTINS               = KEY_ALT or KEY_INS;
   KEY_ALTDEL               = KEY_ALT or KEY_DEL;

   KEY_CTRLSHIFTHOME        = KEY_CTRL or KEY_SHIFT or KEY_HOME;
   KEY_CTRLSHIFTUP          = KEY_CTRL or KEY_SHIFT or KEY_UP;
   KEY_CTRLSHIFTPGUP        = KEY_CTRL or KEY_SHIFT or KEY_PGUP;
   KEY_CTRLSHIFTLEFT        = KEY_CTRL or KEY_SHIFT or KEY_LEFT;
   KEY_CTRLSHIFTRIGHT       = KEY_CTRL or KEY_SHIFT or KEY_RIGHT;
   KEY_CTRLSHIFTEND         = KEY_CTRL or KEY_SHIFT or KEY_END;
   KEY_CTRLSHIFTDOWN        = KEY_CTRL or KEY_SHIFT or KEY_DOWN;
   KEY_CTRLSHIFTPGDN        = KEY_CTRL or KEY_SHIFT or KEY_PGDN;
   KEY_CTRLSHIFTINS         = KEY_CTRL or KEY_SHIFT or KEY_INS;
   KEY_CTRLSHIFTDEL         = KEY_CTRL or KEY_SHIFT or KEY_DEL;

   KEY_ALTSHIFTHOME         = KEY_ALT or KEY_SHIFT or KEY_HOME;
   KEY_ALTSHIFTUP           = KEY_ALT or KEY_SHIFT or KEY_UP;
   KEY_ALTSHIFTPGUP         = KEY_ALT or KEY_SHIFT or KEY_PGUP;
   KEY_ALTSHIFTLEFT         = KEY_ALT or KEY_SHIFT or KEY_LEFT;
   KEY_ALTSHIFTRIGHT        = KEY_ALT or KEY_SHIFT or KEY_RIGHT;
   KEY_ALTSHIFTEND          = KEY_ALT or KEY_SHIFT or KEY_END;
   KEY_ALTSHIFTDOWN         = KEY_ALT or KEY_SHIFT or KEY_DOWN;
   KEY_ALTSHIFTPGDN         = KEY_ALT or KEY_SHIFT or KEY_PGDN;
   KEY_ALTSHIFTINS          = KEY_ALT or KEY_SHIFT or KEY_INS;
   KEY_ALTSHIFTDEL          = KEY_ALT or KEY_SHIFT or KEY_DEL;

   KEY_CTRLALTHOME          = KEY_CTRL or KEY_ALT or KEY_HOME;
   KEY_CTRLALTUP            = KEY_CTRL or KEY_ALT or KEY_UP;
   KEY_CTRLALTPGUP          = KEY_CTRL or KEY_ALT or KEY_PGUP;
   KEY_CTRLALTLEFT          = KEY_CTRL or KEY_ALT or KEY_LEFT;
   KEY_CTRLALTRIGHT         = KEY_CTRL or KEY_ALT or KEY_RIGHT;
   KEY_CTRLALTEND           = KEY_CTRL or KEY_ALT or KEY_END;
   KEY_CTRLALTDOWN          = KEY_CTRL or KEY_ALT or KEY_DOWN;
   KEY_CTRLALTPGDN          = KEY_CTRL or KEY_ALT or KEY_PGDN;
   KEY_CTRLALTINS           = KEY_CTRL or KEY_ALT or KEY_INS;

   KEY_CTRLNUMPAD0          = KEY_CTRL or KEY_NUMPAD0;
   KEY_CTRLNUMPAD1          = KEY_CTRL or KEY_NUMPAD1;
   KEY_CTRLNUMPAD2          = KEY_CTRL or KEY_NUMPAD2;
   KEY_CTRLNUMPAD3          = KEY_CTRL or KEY_NUMPAD3;
   KEY_CTRLNUMPAD4          = KEY_CTRL or KEY_NUMPAD4;
   KEY_CTRLNUMPAD5          = KEY_CTRL or KEY_NUMPAD5;
   KEY_CTRLNUMPAD6          = KEY_CTRL or KEY_NUMPAD6;
   KEY_CTRLNUMPAD7          = KEY_CTRL or KEY_NUMPAD7;
   KEY_CTRLNUMPAD8          = KEY_CTRL or KEY_NUMPAD8;
   KEY_CTRLNUMPAD9          = KEY_CTRL or KEY_NUMPAD9;

   KEY_SHIFTNUMPAD0         = KEY_SHIFT or KEY_NUMPAD0;
   KEY_SHIFTNUMPAD1         = KEY_SHIFT or KEY_NUMPAD1;
   KEY_SHIFTNUMPAD2         = KEY_SHIFT or KEY_NUMPAD2;
   KEY_SHIFTNUMPAD3         = KEY_SHIFT or KEY_NUMPAD3;
   KEY_SHIFTNUMPAD4         = KEY_SHIFT or KEY_NUMPAD4;
   KEY_SHIFTNUMPAD5         = KEY_SHIFT or KEY_NUMPAD5;
   KEY_SHIFTNUMPAD6         = KEY_SHIFT or KEY_NUMPAD6;
   KEY_SHIFTNUMPAD7         = KEY_SHIFT or KEY_NUMPAD7;
   KEY_SHIFTNUMPAD8         = KEY_SHIFT or KEY_NUMPAD8;
   KEY_SHIFTNUMPAD9         = KEY_SHIFT or KEY_NUMPAD9;

   KEY_CTRLSHIFTNUMPAD0     = KEY_CTRL or KEY_SHIFT or KEY_NUMPAD0;
   KEY_CTRLSHIFTNUMPAD1     = KEY_CTRL or KEY_SHIFT or KEY_NUMPAD1;
   KEY_CTRLSHIFTNUMPAD2     = KEY_CTRL or KEY_SHIFT or KEY_NUMPAD2;
   KEY_CTRLSHIFTNUMPAD3     = KEY_CTRL or KEY_SHIFT or KEY_NUMPAD3;
   KEY_CTRLSHIFTNUMPAD4     = KEY_CTRL or KEY_SHIFT or KEY_NUMPAD4;
   KEY_CTRLSHIFTNUMPAD5     = KEY_CTRL or KEY_SHIFT or KEY_NUMPAD5;
   KEY_CTRLSHIFTNUMPAD6     = KEY_CTRL or KEY_SHIFT or KEY_NUMPAD6;
   KEY_CTRLSHIFTNUMPAD7     = KEY_CTRL or KEY_SHIFT or KEY_NUMPAD7;
   KEY_CTRLSHIFTNUMPAD8     = KEY_CTRL or KEY_SHIFT or KEY_NUMPAD8;
   KEY_CTRLSHIFTNUMPAD9     = KEY_CTRL or KEY_SHIFT or KEY_NUMPAD9;

   KEY_CTRLALTNUMPAD0       = KEY_CTRL or KEY_ALT or KEY_NUMPAD0;
   KEY_CTRLALTNUMPAD1       = KEY_CTRL or KEY_ALT or KEY_NUMPAD1;
   KEY_CTRLALTNUMPAD2       = KEY_CTRL or KEY_ALT or KEY_NUMPAD2;
   KEY_CTRLALTNUMPAD3       = KEY_CTRL or KEY_ALT or KEY_NUMPAD3;
   KEY_CTRLALTNUMPAD4       = KEY_CTRL or KEY_ALT or KEY_NUMPAD4;
   KEY_CTRLALTNUMPAD5       = KEY_CTRL or KEY_ALT or KEY_NUMPAD5;
   KEY_CTRLALTNUMPAD6       = KEY_CTRL or KEY_ALT or KEY_NUMPAD6;
   KEY_CTRLALTNUMPAD7       = KEY_CTRL or KEY_ALT or KEY_NUMPAD7;
   KEY_CTRLALTNUMPAD8       = KEY_CTRL or KEY_ALT or KEY_NUMPAD8;
   KEY_CTRLALTNUMPAD9       = KEY_CTRL or KEY_ALT or KEY_NUMPAD9;

   KEY_ALTSHIFTNUMPAD0      = KEY_ALT or KEY_SHIFT or KEY_NUMPAD0;
   KEY_ALTSHIFTNUMPAD1      = KEY_ALT or KEY_SHIFT or KEY_NUMPAD1;
   KEY_ALTSHIFTNUMPAD2      = KEY_ALT or KEY_SHIFT or KEY_NUMPAD2;
   KEY_ALTSHIFTNUMPAD3      = KEY_ALT or KEY_SHIFT or KEY_NUMPAD3;
   KEY_ALTSHIFTNUMPAD4      = KEY_ALT or KEY_SHIFT or KEY_NUMPAD4;
   KEY_ALTSHIFTNUMPAD5      = KEY_ALT or KEY_SHIFT or KEY_NUMPAD5;
   KEY_ALTSHIFTNUMPAD6      = KEY_ALT or KEY_SHIFT or KEY_NUMPAD6;
   KEY_ALTSHIFTNUMPAD7      = KEY_ALT or KEY_SHIFT or KEY_NUMPAD7;
   KEY_ALTSHIFTNUMPAD8      = KEY_ALT or KEY_SHIFT or KEY_NUMPAD8;
   KEY_ALTSHIFTNUMPAD9      = KEY_ALT or KEY_SHIFT or KEY_NUMPAD9;

   KEY_CTRLSLASH            = KEY_CTRL or KEY_SLASH;
   KEY_CTRLBACKSLASH        = KEY_CTRL or KEY_BACKSLASH;
   KEY_CTRLCLEAR            = KEY_CTRL or KEY_CLEAR;
   KEY_CTRLSHIFTCLEAR       = KEY_CTRL or KEY_SHIFT or KEY_CLEAR;
   KEY_CTRLALTCLEAR         = KEY_CTRL or KEY_ALT or KEY_CLEAR;
   KEY_CTRLADD              = KEY_CTRL or KEY_ADD;
   KEY_SHIFTADD             = KEY_SHIFT or KEY_ADD;

   KEY_CTRLSUBTRACT         = KEY_CTRL or KEY_SUBTRACT;
   KEY_ALTSUBTRACT          = KEY_ALT or KEY_SUBTRACT;
   KEY_SHIFTSUBTRACT        = KEY_SHIFT or KEY_SUBTRACT;
   KEY_CTRLMULTIPLY         = KEY_CTRL or KEY_MULTIPLY;

   KEY_CTRLBS               = KEY_CTRL or KEY_BS;
   KEY_ALTBS                = KEY_ALT or KEY_BS;
   KEY_CTRLSHIFTBS          = KEY_CTRL or KEY_SHIFT or KEY_BS;
   KEY_SHIFTBS              = KEY_SHIFT or KEY_BS;

   KEY_CTRLSHIFTTAB         = KEY_CTRL or KEY_SHIFT or KEY_TAB;
   KEY_CTRLTAB              = KEY_CTRL or KEY_TAB;
   KEY_SHIFTTAB             = KEY_SHIFT or KEY_TAB;

   KEY_CTRLENTER            = KEY_CTRL or KEY_ENTER;
   KEY_SHIFTENTER           = KEY_SHIFT or KEY_ENTER;
   KEY_ALTSHIFTENTER        = KEY_ALT or KEY_SHIFT or KEY_ENTER;
   KEY_CTRLALTENTER         = KEY_CTRL or KEY_ALT or KEY_ENTER;
   KEY_CTRLSHIFTENTER       = KEY_CTRL or KEY_SHIFT or KEY_ENTER;

   KEY_CTRLAPPS             = KEY_CTRL or KEY_APPS;
   KEY_ALTAPPS              = KEY_ALT or KEY_APPS;
   KEY_SHIFTAPPS            = KEY_SHIFT or KEY_APPS;
   KEY_CTRLSHIFTAPPS        = KEY_CTRL or KEY_SHIFT or KEY_APPS;
   KEY_ALTSHIFTAPPS         = KEY_ALT or KEY_SHIFT or KEY_APPS;
   KEY_CTRLALTAPPS          = KEY_CTRL or KEY_ALT or KEY_APPS;

   KEY_CTRLSPACE            = KEY_CTRL or KEY_SPACE;
   KEY_SHIFTSPACE           = KEY_SHIFT or KEY_SPACE;
   KEY_CTRLSHIFTSPACE       = KEY_CTRL or KEY_SHIFT or KEY_SPACE;

   KEY_ALT_BASE             = KEY_ALT;
   KEY_ALTSHIFT_BASE        = KEY_ALTSHIFT;

implementation
end.
