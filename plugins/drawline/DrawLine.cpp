#include "plugin.hpp"
#include "CRT/crt.hpp"
#include "DrawLng.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"

#if defined(__GNUC__)

#ifdef __cplusplus
extern "C"
{
#endif
	BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
	(void) lpReserved;
	(void) dwReason;
	(void) hDll;
	return TRUE;
}
#endif

void ProcessShiftKey(int KeyCode,int LineWidth);
void GetEnvType(wchar_t *NewString,int StringLength,struct EditorInfo *ei, int &LeftLine,int &UpLine,int &RightLine,int &DownLine);
void SetTitle(int LineWidth,int IDTitle);
const wchar_t *GetMsg(int MsgId);

static wchar_t BoxChar[]  =
{
	// UNICODE
	0x2502,0x2524,0x2561,0x2562,0x2556,0x2555,0x2563,0x2551,0x2557,0x255d,0x255c,0x255b,0x2510,
	0x2514,0x2534,0x252c,0x251c,0x2500,0x253c,0x255e,0x255f,0x255a,0x2554,0x2569,0x2566,0x2560,0x2550,0x256c,0x2567,
	0x2568,0x2564,0x2565,0x2559,0x2558,0x2552,0x2553,0x256b,0x256a,0x2518,0x250c
};

static short BoxLeft[]  = { 0 , 1 , 2 , 1 , 1 , 2 , 2 , 0 , 2 , 2 , 1 , 2 , 1 , 0 , 1 , 1 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 2 , 2 , 0 , 2 , 2 , 2 , 1 , 2 , 1 , 0 , 0 , 0 , 0 , 1 , 2 , 1 , 0 };
static short BoxUp[]    = { 1 , 1 , 1 , 2 , 0 , 0 , 2 , 2 , 0 , 2 , 2 , 1 , 0 , 1 , 1 , 0 , 1 , 0 , 1 , 1 , 2 , 2 , 0 , 2 , 0 , 2 , 0 , 2 , 1 , 2 , 0 , 0 , 2 , 1 , 0 , 0 , 2 , 1 , 1 , 0 };
static short BoxRight[] = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 1 , 1 , 2 , 1 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 1 , 2 , 1 , 1 , 2 , 2 , 1 , 1 , 2 , 0 , 1 };
static short BoxDown[]  = { 1 , 1 , 1 , 2 , 2 , 1 , 2 , 2 , 2 , 0 , 0 , 0 , 1 , 0 , 0 , 1 , 1 , 0 , 1 , 1 , 2 , 0 , 2 , 0 , 2 , 2 , 0 , 2 , 0 , 0 , 1 , 2 , 0 , 0 , 1 , 2 , 2 , 1 , 0 , 1 };

static struct PluginStartupInfo Info;

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}


void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	::Info=*Info;
}


HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	static bool Reenter=false;

	if (Reenter)
		return INVALID_HANDLE_VALUE;

	Reenter=true;
	int LineWidth=1, KeyCode;
	bool Done=false;
	INPUT_RECORD rec;
	SetTitle(LineWidth,(LineWidth==1)?MTitleSingle:MTitleDouble);

	while (!Done)
	{
		Info.EditorControl(-1,ECTL_READINPUT,0,(INT_PTR)&rec);

		if ((rec.EventType&(~0x8000))!=KEY_EVENT || !rec.Event.KeyEvent.bKeyDown)
		{
#if 0
			COORD MousePos={-1,-1};

			if (rec.EventType==MOUSE_EVENT)
			{
				if ((rec.Event.MouseEvent.dwControlKeyState&SHIFT_PRESSED) &&
				        (rec.Event.MouseEvent.dwButtonState&(FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
				{
					int DX=MousePos.X - rec.Event.MouseEvent.dwMousePosition.X;
					int DY=MousePos.Y - rec.Event.MouseEvent.dwMousePosition.Y;
					int KeyCodeX=!DX?VK_NUMPAD0:(DX<0?VK_NUMPAD6:VK_NUMPAD4);
					int KeyCodeY=!DY?VK_NUMPAD0:(DY<0?VK_NUMPAD2:VK_NUMPAD8);
					int I;

					if (DX < 0) DX=-DX;

					if (DY < 0) DY=-DY;

					// Ctrl
					if (rec.Event.MouseEvent.dwControlKeyState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED))
					{
						if (DX < DY)
							DX=0;
						else if (DX > DY)
							DY=0;
					}

					if (DX && !DY)
					{
						for (I=0; I < DX; ++I)
							ProcessShiftKey(KeyCodeX,
							                (rec.Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)?1:2);
					}
					else if (!DX && DY)
					{
						for (I=0; I < DY; ++I)
							ProcessShiftKey(KeyCodeY,
							                (rec.Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)?1:2);
					}
					else if (DX && DY)
					{
						for (I=0;; ++I)
						{
							ProcessShiftKey(KeyCodeX,
							                (rec.Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)?1:2);
							ProcessShiftKey(KeyCodeY,
							                (rec.Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)?1:2);
							DX--;
							DY--;

							if (DX < 0 && DY < 0) //??
								break;
						}
					}
				}

				MousePos=rec.Event.MouseEvent.dwMousePosition;
			}
#endif
			Info.EditorControl(-1,ECTL_PROCESSINPUT,0,(INT_PTR)&rec);
			continue;
		}
		else
		{
			KeyCode=rec.Event.KeyEvent.wVirtualKeyCode;
		}

		switch (KeyCode)
		{
			case VK_ESCAPE:
			case VK_F10:

				if ((rec.Event.KeyEvent.dwControlKeyState & (SHIFT_PRESSED|LEFT_CTRL_PRESSED|
				        RIGHT_CTRL_PRESSED|LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))==0)
					Done=true;

				break;
			case VK_F1:
				Info.ShowHelp(Info.ModuleName,NULL,0);
				break;
			case VK_F2:

				if ((rec.Event.KeyEvent.dwControlKeyState & (SHIFT_PRESSED|LEFT_CTRL_PRESSED|
				        RIGHT_CTRL_PRESSED|LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))==0)
				{
					LineWidth=3-LineWidth;
					SetTitle(LineWidth,((LineWidth==1)?MTitleSingle:MTitleDouble));
				}

				break;
			default:

				if ((KeyCode>=VK_PRIOR && KeyCode<=VK_DOWN) ||
				        (KeyCode>=VK_NUMPAD0 && KeyCode<=VK_NUMPAD9))
				{
					if (rec.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
						ProcessShiftKey(KeyCode,LineWidth);
					else
						Info.EditorControl(-1,ECTL_PROCESSINPUT,0,(INT_PTR)&rec);
				}
				else
				{
					if (KeyCode < VK_F3 || KeyCode > VK_F12)
						Info.EditorControl(-1,ECTL_PROCESSINPUT,0,(INT_PTR)&rec);

					continue;
				}

				break;
		}
	}

	Info.EditorControl(-1,ECTL_SETTITLE,0,0);
	Info.EditorControl(-1,ECTL_SETKEYBAR,0,0);
	Reenter=false;
	return INVALID_HANDLE_VALUE;
}


void SetTitle(int LineWidth,int IDTitle)
{
	const DWORD control[] = {0, LEFT_CTRL_PRESSED, LEFT_ALT_PRESSED, SHIFT_PRESSED, SHIFT_PRESSED|LEFT_CTRL_PRESSED, SHIFT_PRESSED|LEFT_ALT_PRESSED, LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED};
	struct KeyBarLabel kbl[24*7];
	struct KeyBarTitles kbt = {ARRAYSIZE(kbl), kbl};

	for (int c=0, i=0; c<7; c++)
	{
		for (int k=0; k<24; k++, i++)
		{
			kbl[i].Text = L"";
			kbl[i].LongText = L"";
			kbl[i].Key.VirtualKeyCode = VK_F1 + k;
			kbl[i].Key.ControlKeyState = control[c];
		}
	}

	kbl[1-1].Text=kbl[1-1].LongText=GetMsg(MHelp);
	kbl[2-1].Text=kbl[2-1].LongText=GetMsg((LineWidth==1)?MDouble:MSingle);
	kbl[10-1].Text=kbl[10-1].LongText=GetMsg(MQuit);
	Info.EditorControl(-1,ECTL_SETKEYBAR,0,(INT_PTR)&kbt);
	Info.EditorControl(-1,ECTL_SETTITLE,0,(INT_PTR)GetMsg(IDTitle));
}

void ProcessShiftKey(int KeyCode,int LineWidth)
{
	EditorInfo ei;
	Info.EditorControl(-1,ECTL_GETINFO,0,(INT_PTR)&ei);
	struct EditorSetPosition esp;
	esp.CurLine=ei.CurLine;
	esp.CurPos=ei.CurTabPos;
	esp.CurTabPos=-1;
	esp.TopScreenLine=-1;
	esp.LeftPos=-1;
	esp.Overtype=-1;

	if (ei.CurLine>0)
	{
		int StringNumber=ei.CurLine-1;
		Info.EditorControl(-1,ECTL_EXPANDTABS,0,(INT_PTR)&StringNumber);
	}

	Info.EditorControl(-1,ECTL_EXPANDTABS,0,(INT_PTR)&ei.CurLine);

	if (ei.CurLine>=ei.TotalLines-1)
	{
		struct EditorGetString egs;
		egs.StringNumber=ei.CurLine;
		Info.EditorControl(-1,ECTL_GETSTRING,0,(INT_PTR)&egs);
		struct EditorSetPosition esp;
		esp.CurLine=ei.CurLine;
		esp.CurPos=egs.StringLength;
		esp.CurTabPos=-1;
		esp.TopScreenLine=-1;
		esp.LeftPos=-1;
		esp.Overtype=-1;
		Info.EditorControl(-1,ECTL_SETPOSITION,0,(INT_PTR)&esp);
		Info.EditorControl(-1,ECTL_INSERTSTRING,0,0);
		esp.CurLine=ei.CurLine;
		esp.CurPos=ei.CurTabPos;
		Info.EditorControl(-1,ECTL_SETPOSITION,0,(INT_PTR)&esp);
	}

	if (ei.CurLine<ei.TotalLines-1)
	{
		int StringNumber=ei.CurLine+1;
		Info.EditorControl(-1,ECTL_EXPANDTABS,0,(INT_PTR)&StringNumber);
	}

	Info.EditorControl(-1,ECTL_GETINFO,0,(INT_PTR)&ei);
	struct EditorGetString egs;
	egs.StringNumber=ei.CurLine;
	Info.EditorControl(-1,ECTL_GETSTRING,0,(INT_PTR)&egs);
	int StringLength=egs.StringLength>ei.CurPos ? egs.StringLength:ei.CurPos+1;
	wchar_t *NewString=(wchar_t *)malloc(StringLength*sizeof(wchar_t));

	if (StringLength>egs.StringLength)
		wmemset(NewString+egs.StringLength,L' ',StringLength-egs.StringLength);

	wmemcpy(NewString,egs.StringText,egs.StringLength);
	int LeftLine,UpLine,RightLine,DownLine;
	GetEnvType(NewString,StringLength,&ei,LeftLine,UpLine,RightLine,DownLine);

	switch (KeyCode)
	{
		case VK_UP:
		case VK_NUMPAD8:
			UpLine=LineWidth;

			if (LeftLine==0 && RightLine==0)
				DownLine=UpLine;

			if (esp.CurLine>0)
				esp.CurLine--;

			break;
		case VK_DOWN:
		case VK_NUMPAD2:
			DownLine=LineWidth;

			if (LeftLine==0 && RightLine==0)
				UpLine=DownLine;

			esp.CurLine++;
			break;
		case VK_LEFT:
		case VK_NUMPAD4:
			LeftLine=LineWidth;

			if (UpLine==0 && DownLine==0)
				RightLine=LeftLine;

			if (esp.CurPos>0)
				esp.CurPos--;

			break;
		case VK_RIGHT:
		case VK_NUMPAD6:
			RightLine=LineWidth;

			if (UpLine==0 && DownLine==0)
				LeftLine=RightLine;

			esp.CurPos++;
			break;
	}

	if (LeftLine!=0 && RightLine!=0 && LeftLine!=RightLine)
		LeftLine=RightLine=LineWidth;

	if (UpLine!=0 && DownLine!=0 && UpLine!=DownLine)
		UpLine=DownLine=LineWidth;

	for (size_t I=0; I<sizeof(BoxChar)/sizeof(wchar_t); I++)
	{
		if (LeftLine==BoxLeft[I] && UpLine==BoxUp[I] &&
		        RightLine==BoxRight[I] && DownLine==BoxDown[I])
		{
			NewString[ei.CurPos]=BoxChar[I];
			struct EditorSetString ess;
			ess.StringNumber=egs.StringNumber;
			ess.StringText=NewString;
			ess.StringEOL=(wchar_t*)egs.StringEOL;
			ess.StringLength=StringLength;
			Info.EditorControl(-1,ECTL_SETSTRING,0,(INT_PTR)&ess);
			Info.EditorControl(-1,ECTL_SETPOSITION,0,(INT_PTR)&esp);
			Info.EditorControl(-1,ECTL_REDRAW,0,0);
			break;
		}
	}

	free(NewString);
}


void GetEnvType(wchar_t *NewString,int StringLength,struct EditorInfo *ei,
                int &LeftLine,int &UpLine,int &RightLine,int &DownLine)
{
	wchar_t OldChar[3];
	OldChar[0]=ei->CurPos>0 ? NewString[ei->CurPos-1]:L' ';
	OldChar[1]=NewString[ei->CurPos];
	OldChar[2]=ei->CurPos<StringLength-1 ? NewString[ei->CurPos+1]:L' ';
	wchar_t LeftChar=OldChar[0];
	wchar_t RightChar=OldChar[2];
	wchar_t UpChar=L' ';
	wchar_t DownChar=L' ';

	if (ei->CurLine>0)
	{
		struct EditorGetString UpStr;
		UpStr.StringNumber=ei->CurLine-1;
		Info.EditorControl(-1,ECTL_GETSTRING,0,(INT_PTR)&UpStr);

		if (ei->CurPos<UpStr.StringLength)
			UpChar=UpStr.StringText[ei->CurPos];
	}

	if (ei->CurLine<ei->TotalLines-1)
	{
		struct EditorGetString DownStr;
		DownStr.StringNumber=ei->CurLine+1;
		Info.EditorControl(-1,ECTL_GETSTRING,0,(INT_PTR)&DownStr);

		if (ei->CurPos<DownStr.StringLength)
			DownChar=DownStr.StringText[ei->CurPos];
	}

	LeftLine=UpLine=RightLine=DownLine=0;

	for (size_t I=0; I<sizeof(BoxChar)/sizeof(wchar_t); I++)
	{
		if (LeftChar==BoxChar[I])
			LeftLine=BoxRight[I];

		if (UpChar==BoxChar[I])
			UpLine=BoxDown[I];

		if (RightChar==BoxChar[I])
			RightLine=BoxLeft[I];

		if (DownChar==BoxChar[I])
			DownLine=BoxUp[I];
	}
}


void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MDrawLines);
	Info->PluginMenu.Guids=&MenuGuid;
	Info->PluginMenu.Strings=PluginMenuStrings;
	Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
}

const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}
