/*
interf.cpp

Консольные функции ввода-вывода
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

#include "interf.hpp"
#include "keyboard.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "fileedit.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "syslog.hpp"
#include "palette.hpp"
#include "strmix.hpp"
#include "console.hpp"
#include "configdb.hpp"
#include "colormix.hpp"
#include "imports.hpp"
#include "event.hpp"
#include "res.hpp"

consoleicons ConsoleIcons;


consoleicons::consoleicons():
	LargeIcon(nullptr),
	SmallIcon(nullptr),
	PreviousLargeIcon(nullptr),
	PreviousSmallIcon(nullptr),
	Loaded(false),
	LargeChanged(false),
	SmallChanged(false)
{
}

void consoleicons::setFarIcons()
{
	if(Opt.SetIcon)
	{
		if(!Loaded)
		{
			int IconId = (Opt.SetAdminIcon && Opt.IsUserAdmin)? FAR_ICON_A : FAR_ICON;
			LargeIcon = reinterpret_cast<HICON>(LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IconId), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0));
			SmallIcon = reinterpret_cast<HICON>(LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IconId), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));
			Loaded = true;
		}

		HWND hWnd = Console.GetWindow();
		if (hWnd)
		{
			if(LargeIcon)
			{
				PreviousLargeIcon = reinterpret_cast<HICON>(SendMessage(hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LargeIcon)));
				LargeChanged = true;
			}
			if(SmallIcon)
			{
				PreviousSmallIcon = reinterpret_cast<HICON>(SendMessage(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(SmallIcon)));
				SmallChanged = true;
			}
		}
	}
}

void consoleicons::restorePreviousIcons()
{
	if(Opt.SetIcon)
	{
		HWND hWnd = Console.GetWindow();
		if (hWnd)
		{
			if(LargeChanged)
			{
				SendMessage(hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(PreviousLargeIcon));
				LargeChanged = false;
			}
			if(SmallChanged)
			{
				SendMessage(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(PreviousSmallIcon));
				SmallChanged = false;
			}
		}
	}
}

static int CurX,CurY;
static FarColor CurColor;

CONSOLE_CURSOR_INFO InitialCursorInfo;

static SMALL_RECT windowholder_rect;

WCHAR Oem2Unicode[256];
WCHAR BoxSymbols[64];

COORD InitSize={};
COORD CurSize={};
SHORT ScrX=0,ScrY=0;
SHORT PrevScrX=-1,PrevScrY=-1;
DWORD InitialConsoleMode=0;
string strInitTitle;
SMALL_RECT InitWindowRect;
COORD InitialSize;

//stack buffer size + stack vars size must be less than 16384
const size_t StackBufferSize=0x3FC0;
Event CancelIoInProgress(true);

DWORD WINAPI CancelSynchronousIoWrapper(LPVOID Thread)
{
	DWORD Result = ifn.CancelSynchronousIo(Thread);
	CancelIoInProgress.Reset();
	return Result;
}

BOOL WINAPI CtrlHandler(DWORD CtrlType)
{
	switch(CtrlType)
	{
	case CTRL_C_EVENT:
		return TRUE;

	case CTRL_BREAK_EVENT:
		if(!CancelIoInProgress.Signaled())
		{
			CancelIoInProgress.Set();
			HANDLE Thread = CreateThread(nullptr, 0, CancelSynchronousIoWrapper, MainThreadHandle, 0, nullptr);
			if (Thread)
			{
				CloseHandle(Thread);
			}
		}
		WriteInput(KEY_BREAK);

		if (CtrlObject && CtrlObject->Cp())
		{
			if (CtrlObject->Cp()->LeftPanel && CtrlObject->Cp()->LeftPanel->GetMode()==PLUGIN_PANEL)
				CtrlObject->Plugins->ProcessEvent(CtrlObject->Cp()->LeftPanel->GetPluginHandle(),FE_BREAK, ToPtr(CtrlType));

			if (CtrlObject->Cp()->RightPanel && CtrlObject->Cp()->RightPanel->GetMode()==PLUGIN_PANEL)
				CtrlObject->Plugins->ProcessEvent(CtrlObject->Cp()->RightPanel->GetPluginHandle(),FE_BREAK, ToPtr(CtrlType));
		}
		return TRUE;

	case CTRL_CLOSE_EVENT:
		CloseFAR=TRUE;
		AllowCancelExit=FALSE;

		// trick to let wmain() finish correctly
		ExitThread(1);
		//return TRUE;
	}
	return FALSE;
}

void InitConsole(int FirstInit)
{
	InitRecodeOutTable();

	if (FirstInit)
	{
		DWORD Mode;
		if(!Console.GetMode(Console.GetInputHandle(), Mode))
		{
			HANDLE ConIn = CreateFile(L"CONIN$", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
			SetStdHandle(STD_INPUT_HANDLE, ConIn);
		}
		if(!Console.GetMode(Console.GetOutputHandle(), Mode))
		{
			HANDLE ConOut = CreateFile(L"CONOUT$", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
			SetStdHandle(STD_OUTPUT_HANDLE, ConOut);
			SetStdHandle(STD_ERROR_HANDLE, ConOut);
		}
	}

	Console.SetControlHandler(CtrlHandler,TRUE);
	Console.GetMode(Console.GetInputHandle(),InitialConsoleMode);
	Console.GetTitle(strInitTitle);
	Console.GetWindowRect(InitWindowRect);
	Console.GetSize(InitialSize);
	Console.GetCursorInfo(InitialCursorInfo);

	// размер клавиатурной очереди = 1024 кода клавиши
	if (!KeyQueue)
		KeyQueue=new FarQueue<DWORD>(1024);

	SetFarConsoleMode();

	/* $ 09.04.2002 DJ
	   если размер консольного буфера больше размера окна, выставим
	   их равными
	*/
	if (FirstInit)
	{
		SMALL_RECT WindowRect;
		Console.GetWindowRect(WindowRect);
		GetVideoMode(InitSize);

		if(Opt.WindowMode)
		{
			Console.ResetPosition();
		}
		else
		{
			if (WindowRect.Left || WindowRect.Top || WindowRect.Right != InitSize.X-1 || WindowRect.Bottom != InitSize.Y-1)
			{
				COORD newSize;
				newSize.X = WindowRect.Right - WindowRect.Left + 1;
				newSize.Y = WindowRect.Bottom - WindowRect.Top + 1;
				Console.SetSize(newSize);
				GetVideoMode(InitSize);
			}
		}
		if (IsZoomed(Console.GetWindow()))
			ChangeVideoMode(1);
	}

	GetVideoMode(CurSize);
	ScrBuf.FillBuf();

	ConsoleIcons.setFarIcons();
}
void CloseConsole()
{
	ScrBuf.Flush();
	Console.SetCursorInfo(InitialCursorInfo);
	ChangeConsoleMode(InitialConsoleMode);

	Console.SetTitle(strInitTitle);
	Console.SetSize(InitialSize);
	COORD CursorPos = {};
	Console.GetCursorPosition(CursorPos);
	SHORT Height = InitWindowRect.Bottom-InitWindowRect.Top, Width = InitWindowRect.Right-InitWindowRect.Left;
	if (CursorPos.Y > InitWindowRect.Bottom || CursorPos.Y < InitWindowRect.Top)
		InitWindowRect.Top = Max(0, CursorPos.Y-Height);
	if (CursorPos.X > InitWindowRect.Right || CursorPos.X < InitWindowRect.Left)
		InitWindowRect.Left = Max(0, CursorPos.X-Width);
	InitWindowRect.Bottom = InitWindowRect.Top + Height;
	InitWindowRect.Right = InitWindowRect.Left + Width;
	Console.SetWindowRect(InitWindowRect);
	Console.SetSize(InitialSize);

	delete KeyQueue;
	KeyQueue=nullptr;

	ConsoleIcons.restorePreviousIcons();
}


void SetFarConsoleMode(BOOL SetsActiveBuffer)
{
	int Mode=ENABLE_WINDOW_INPUT;

	if (Opt.Mouse)
	{
		//ENABLE_EXTENDED_FLAGS actually disables all the extended flags.
		Mode|=ENABLE_MOUSE_INPUT|ENABLE_EXTENDED_FLAGS;
	}
	else
	{
		//если вдруг изменили опцию во время работы фара, то включим то что надо
		Mode|=InitialConsoleMode&(ENABLE_EXTENDED_FLAGS|ENABLE_QUICK_EDIT_MODE);
	}

	if (SetsActiveBuffer)
		Console.SetActiveScreenBuffer(Console.GetOutputHandle());

	ChangeConsoleMode(Mode);

	//востановим дефолтный режим вывода, а то есть такие проги что сбрасывают
	ChangeConsoleMode(ENABLE_PROCESSED_OUTPUT|ENABLE_WRAP_AT_EOL_OUTPUT, 1);
	ChangeConsoleMode(ENABLE_PROCESSED_OUTPUT|ENABLE_WRAP_AT_EOL_OUTPUT, 2);
}

void ChangeConsoleMode(int Mode, int Choose)
{
	DWORD CurrentConsoleMode;
	HANDLE hCon = (Choose == 0) ? Console.GetInputHandle() : ((Choose == 1) ? Console.GetOutputHandle() : Console.GetErrorHandle());
	Console.GetMode(hCon, CurrentConsoleMode);

	if (CurrentConsoleMode!=(DWORD)Mode)
		Console.SetMode(hCon, Mode);
}

void SaveConsoleWindowRect()
{
	Console.GetWindowRect(windowholder_rect);
}

void RestoreConsoleWindowRect()
{
	SMALL_RECT WindowRect;
	Console.GetWindowRect(WindowRect);
	if(WindowRect.Right-WindowRect.Left<windowholder_rect.Right-windowholder_rect.Left ||
		WindowRect.Bottom-WindowRect.Top<windowholder_rect.Bottom-windowholder_rect.Top)
	{
		Console.SetWindowRect(windowholder_rect);
	}
}

void FlushInputBuffer()
{
	Console.FlushInputBuffer();
	IntKeyState.MouseButtonState=0;
	IntKeyState.MouseEventFlags=0;
}

void SetVideoMode()
{
	if (!IsConsoleFullscreen() && Opt.AltF9)
	{
		ChangeVideoMode(InitSize.X==CurSize.X && InitSize.Y==CurSize.Y);
	}
	else
	{
		ChangeVideoMode(ScrY==24?50:25,80);
	}
}

void ChangeVideoMode(int Maximized)
{
	COORD coordScreen;

	if (Maximized)
	{
		SendMessage(Console.GetWindow(),WM_SYSCOMMAND,SC_MAXIMIZE,0);
		coordScreen = Console.GetLargestWindowSize();
		coordScreen.X+=Opt.ScrSize.DeltaX;
		coordScreen.Y+=Opt.ScrSize.DeltaY;
	}
	else
	{
		SendMessage(Console.GetWindow(),WM_SYSCOMMAND,SC_RESTORE,0);
		coordScreen = InitSize;
	}

	ChangeVideoMode(coordScreen.Y,coordScreen.X);
}

void ChangeVideoMode(int NumLines,int NumColumns)
{
	short xSize=NumColumns,ySize=NumLines;

	COORD Size;
	Console.GetSize(Size);

	SMALL_RECT srWindowRect;
	srWindowRect.Right = xSize-1;
	srWindowRect.Bottom = ySize-1;
	srWindowRect.Left = srWindowRect.Top = 0;

	COORD coordScreen={xSize,ySize};

	if (xSize>Size.X || ySize > Size.Y)
	{
		if (Size.X < xSize-1)
		{
			srWindowRect.Right = Size.X - 1;
			Console.SetWindowRect(srWindowRect);
			srWindowRect.Right = xSize-1;
		}

		if (Size.Y < ySize-1)
		{
			srWindowRect.Bottom=Size.Y - 1;
			Console.SetWindowRect(srWindowRect);
			srWindowRect.Bottom = ySize-1;
		}

		Console.SetSize(coordScreen);
	}

	if (!Console.SetWindowRect(srWindowRect))
	{
		Console.SetSize(coordScreen);
		Console.SetWindowRect(srWindowRect);
	}
	else
	{
		Console.SetSize(coordScreen);
	}

	GenerateWINDOW_BUFFER_SIZE_EVENT(NumColumns,NumLines);
}

void GenerateWINDOW_BUFFER_SIZE_EVENT(int Sx, int Sy)
{
	COORD Size={};
	if (Sx==-1 || Sy==-1)
	{
		Console.GetSize(Size);
	}
	INPUT_RECORD Rec;
	Rec.EventType=WINDOW_BUFFER_SIZE_EVENT;
	Rec.Event.WindowBufferSizeEvent.dwSize.X=Sx==-1?Size.X:Sx;
	Rec.Event.WindowBufferSizeEvent.dwSize.Y=Sy==-1?Size.Y:Sy;
	size_t Writes;
	Console.WriteInput(&Rec,1,Writes);
}

void GetVideoMode(COORD& Size)
{
	//чтоб решить баг винды приводящий к появлению скролов и т.п. после потери фокуса
	SaveConsoleWindowRect();
	Size.X=0;
	Size.Y=0;
	Console.GetSize(Size);
	ScrX=Size.X-1;
	ScrY=Size.Y-1;
	assert(ScrX>0);
	assert(ScrY>0);
	WidthNameForMessage=(ScrX*38)/100+1;

	if (PrevScrX == -1) PrevScrX=ScrX;

	if (PrevScrY == -1) PrevScrY=ScrY;

	_OT(SysLog(L"ScrX=%d ScrY=%d",ScrX,ScrY));
	ScrBuf.AllocBuf(Size.X,Size.Y);
	_OT(ViewConsoleInfo());
}

void ShowTime(int ShowAlways)
{
	string strClockText;
	static SYSTEMTIME lasttm={};
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	FAR_CHAR_INFO ScreenClockText[5];
	GetText(ScrX-4,0,ScrX,0,ScreenClockText,sizeof(ScreenClockText));

	if (ShowAlways==2)
	{
		ClearStruct(lasttm);
		return;
	}

	if ((!ShowAlways && lasttm.wMinute==tm.wMinute && lasttm.wHour==tm.wHour &&
	        ScreenClockText[2].Char==L':') || ScreenSaverActive)
		return;

	ProcessShowClock++;
	lasttm=tm;
	strClockText.Format(L"%02d:%02d",tm.wHour,tm.wMinute);
	GotoXY(ScrX-4,0);
	// Здесь хрень какая-то получается с ModType - все время не верное значение!
	Frame *CurFrame=FrameManager->GetCurrentFrame();

	if (CurFrame)
	{
		int ModType=CurFrame->GetType();
		SetColor(ModType==MODALTYPE_VIEWER?COL_VIEWERCLOCK:
		         (ModType==MODALTYPE_EDITOR?COL_EDITORCLOCK:COL_CLOCK));
		Text(strClockText);
		//ScrBuf.Flush();
	}

	ProcessShowClock--;
}

void GotoXY(int X,int Y)
{
	CurX=X;
	CurY=Y;
}


int WhereX()
{
	return(CurX);
}


int WhereY()
{
	return(CurY);
}


void MoveCursor(int X,int Y)
{
	ScrBuf.MoveCursor(X,Y);
}


void GetCursorPos(SHORT& X,SHORT& Y)
{
	ScrBuf.GetCursorPos(X,Y);
}


void SetCursorType(bool Visible, DWORD Size)
{
	if (Size==(DWORD)-1 || !Visible)
		Size=IsConsoleFullscreen()?
		     (Opt.CursorSize[1]?(int)Opt.CursorSize[1]:InitialCursorInfo.dwSize):
				     (Opt.CursorSize[0]?(int)Opt.CursorSize[0]:InitialCursorInfo.dwSize);

	ScrBuf.SetCursorType(Visible,Size);
}

void SetInitialCursorType()
{
	ScrBuf.SetCursorType(InitialCursorInfo.bVisible!=FALSE,InitialCursorInfo.dwSize);
}


void GetCursorType(bool& Visible, DWORD& Size)
{
	ScrBuf.GetCursorType(Visible,Size);
}


void MoveRealCursor(int X,int Y)
{
	COORD C={static_cast<SHORT>(X),static_cast<SHORT>(Y)};
	Console.SetCursorPosition(C);
}


void GetRealCursorPos(SHORT& X,SHORT& Y)
{
	COORD CursorPosition;
	Console.GetCursorPosition(CursorPosition);
	X=CursorPosition.X;
	Y=CursorPosition.Y;
}

void InitRecodeOutTable()
{
	for (size_t i=0; i<ARRAYSIZE(Oem2Unicode); i++)
	{
		char c = static_cast<char>(i);
		MultiByteToWideChar(CP_OEMCP, MB_USEGLYPHCHARS, &c, 1, &Oem2Unicode[i], 1);
	}

	if (Opt.CleanAscii)
	{
		for (size_t i=0; i<0x20; i++)
			Oem2Unicode[i]=L'.';

		Oem2Unicode[0x07]=L'*';
		Oem2Unicode[0x10]=L'>';
		Oem2Unicode[0x11]=L'<';
		Oem2Unicode[0x15]=L'$';
		Oem2Unicode[0x16]=L'-';
		Oem2Unicode[0x18]=L'|';
		Oem2Unicode[0x19]=L'V';
		Oem2Unicode[0x1A]=L'>';
		Oem2Unicode[0x1B]=L'<';
		Oem2Unicode[0x1E]=L'X';
		Oem2Unicode[0x1F]=L'X';

		Oem2Unicode[0x7F]=L'.';
	}

	if (Opt.NoGraphics)
	{
		for (int i=0xB3; i<=0xDA; i++)
		{
			Oem2Unicode[i]=L'+';
		}
		Oem2Unicode[0xB3]=L'|';
		Oem2Unicode[0xBA]=L'|';
		Oem2Unicode[0xC4]=L'-';
		Oem2Unicode[0xCD]=L'=';
	}

	{
		// перед [пере]инициализацией восстановим буфер (либо из реестра, либо...)
		xwcsncpy(BoxSymbols,Opt.strBoxSymbols,ARRAYSIZE(BoxSymbols)-1);

		if (Opt.NoGraphics)
		{
			for (int i=BS_V1; i<=BS_LT_H1V1; i++)
				BoxSymbols[i]=L'+';

			BoxSymbols[BS_V1]=BoxSymbols[BS_V2]=L'|';
			BoxSymbols[BS_H1]=L'-';
			BoxSymbols[BS_H2]=L'=';
		}
	}

	//_SVS(SysLogDump("Oem2Unicode",0,(LPBYTE)Oem2Unicode,sizeof(Oem2Unicode),nullptr));
}


void Text(int X, int Y, const FarColor& Color, const WCHAR *Str)
{
	CurColor=Color;
	CurX=X;
	CurY=Y;
	Text(Str);
}

void Text(const WCHAR *Str)
{
	size_t Length=StrLength(Str);

	if (Length<=0)
		return;

	FAR_CHAR_INFO StackBuffer[StackBufferSize/sizeof(FAR_CHAR_INFO)];
	FAR_CHAR_INFO* HeapBuffer=nullptr;
	FAR_CHAR_INFO* BufPtr=StackBuffer;

	if (Length >= StackBufferSize/sizeof(FAR_CHAR_INFO))
	{
		HeapBuffer=new FAR_CHAR_INFO[Length+1];
		BufPtr=HeapBuffer;
	}

	for (size_t i=0; i < Length; i++)
	{
		BufPtr[i].Char=Str[i];
		BufPtr[i].Attributes=CurColor;
	}

	ScrBuf.Write(CurX, CurY, BufPtr, static_cast<int>(Length));
	if(HeapBuffer)
	{
		delete[] HeapBuffer;
	}
	CurX+=static_cast<int>(Length);
}


void Text(LNGID MsgId)
{
	Text(MSG(MsgId));
}

void VText(const WCHAR *Str)
{
	int Length=StrLength(Str);

	if (Length<=0)
		return;

	int StartCurX=CurX;
	WCHAR ChrStr[2]={};

	for (int I=0; I<Length; I++)
	{
		GotoXY(CurX,CurY);
		ChrStr[0]=Str[I];
		Text(ChrStr);
		CurY++;
		CurX=StartCurX;
	}
}

void HiText(const wchar_t *Str,const FarColor& HiColor,int isVertText)
{
	string strTextStr;
	FarColor SaveColor;
	size_t pos;
	strTextStr = Str;

	if (!strTextStr.Pos(pos,L'&'))
	{
		if (isVertText)
			VText(strTextStr);
		else
			Text(strTextStr);
	}
	else
	{
		/*
		   &&      = '&'
		   &&&     = '&'
		              ^H
		   &&&&    = '&&'
		   &&&&&   = '&&'
		              ^H
		   &&&&&&  = '&&&'
		*/
		wchar_t *ChPtr = strTextStr.GetBuffer() + pos;
		int I=0;
		wchar_t *ChPtr2=ChPtr;

		while (*ChPtr2++ == L'&')
			++I;

		if (I&1) // нечет?
		{
			*ChPtr=0;

			if (isVertText)
				VText(strTextStr);
			else
				Text(strTextStr); //BUGBUG BAD!!!

			if (ChPtr[1])
			{
				wchar_t Chr[]={ChPtr[1],0};
				SaveColor=CurColor;
				SetColor(HiColor);

				if (isVertText)
					VText(Chr);
				else
					Text(Chr);

				SetColor(SaveColor);
				string strText = (ChPtr+1);
				strTextStr.ReleaseBuffer();
				ReplaceStrings(strText,L"&&",L"&",-1);

				if (isVertText)
					VText(strText.CPtr()+1);
				else
					Text(strText.CPtr()+1);
			}
		}
		else
		{
			strTextStr.ReleaseBuffer();
			ReplaceStrings(strTextStr,L"&&",L"&",-1);

			if (isVertText)
				VText(strTextStr);
			else
				Text(strTextStr); //BUGBUG BAD!!!
		}
	}
}



void SetScreen(int X1,int Y1,int X2,int Y2,wchar_t Ch,const FarColor& Color)
{
	if (X1<0) X1=0;

	if (Y1<0) Y1=0;

	if (X2>ScrX) X2=ScrX;

	if (Y2>ScrY) Y2=ScrY;

	ScrBuf.FillRect(X1, Y1, X2, Y2, Ch, Color);
}


void MakeShadow(int X1,int Y1,int X2,int Y2)
{
	if (X1<0) X1=0;

	if (Y1<0) Y1=0;

	if (X2>ScrX) X2=ScrX;

	if (Y2>ScrY) Y2=ScrY;
	ScrBuf.ApplyShadow(X1,Y1,X2,Y2);
}

void ChangeBlockColor(int X1,int Y1,int X2,int Y2,const FarColor& Color)
{
	if (X1<0) X1=0;

	if (Y1<0) Y1=0;

	if (X2>ScrX) X2=ScrX;

	if (Y2>ScrY) Y2=ScrY;

	ScrBuf.ApplyColor(X1, Y1, X2, Y2, Color, true);
}

void vmprintf(const WCHAR *fmt,...)
{
	va_list argptr;
	va_start(argptr,fmt);
	WCHAR OutStr[2048];
	_vsnwprintf(OutStr,ARRAYSIZE(OutStr)-1,fmt,argptr);
	VText(OutStr);
	va_end(argptr);
}

void SetColor(int Color)
{
	CurColor=ColorIndexToColor((PaletteColors)Color);
}

void SetColor(PaletteColors Color)
{
	CurColor=ColorIndexToColor(Color);
}

void SetColor(const FarColor& Color)
{
	CurColor=Color;
}

void SetRealColor(const FarColor& Color)
{
	Console.SetTextAttributes(Color);
}

void ClearScreen(const FarColor& Color)
{
	ScrBuf.FillRect(0,0,ScrX,ScrY,L' ',Color);
	if(Opt.WindowMode)
	{
		Console.ClearExtraRegions(Color);
	}
	ScrBuf.ResetShadow();
	ScrBuf.Flush();
	Console.SetTextAttributes(Color);
}

const FarColor& GetColor()
{
	return(CurColor);
}


void ScrollScreen(int Count)
{
	ScrBuf.Scroll(Count);
	ScrBuf.FillRect(0,ScrY+1-Count,ScrX,ScrY,L' ',ColorIndexToColor(COL_COMMANDLINEUSERSCREEN));
}


void GetText(int X1,int Y1,int X2,int Y2,FAR_CHAR_INFO* Dest,size_t DestSize)
{
	ScrBuf.Read(X1,Y1,X2,Y2,Dest,DestSize);
}

void PutText(int X1,int Y1,int X2,int Y2,const void *Src)
{
	int Width=X2-X1+1;
	int Y;
	FAR_CHAR_INFO *SrcPtr=(FAR_CHAR_INFO*)Src;

	for (Y=Y1; Y<=Y2; ++Y,SrcPtr+=Width)
		ScrBuf.Write(X1,Y,SrcPtr,Width);
}

void BoxText(wchar_t Chr)
{
	wchar_t Str[]={Chr,L'\0'};
	BoxText(Str);
}


void BoxText(const wchar_t *Str,int IsVert)
{
	if (IsVert)
		VText(Str);
	else
		Text(Str);
}


/*
   Отрисовка прямоугольника.
*/
void Box(int x1,int y1,int x2,int y2,const FarColor& Color,int Type)
{
	if (x1>=x2 || y1>=y2)
		return;

	SetColor(Color);
	Type=(Type==DOUBLE_BOX || Type==SHORT_DOUBLE_BOX);

	WCHAR StackBuffer[StackBufferSize/sizeof(WCHAR)];
	LPWSTR HeapBuffer=nullptr;
	LPWSTR BufPtr=StackBuffer;

	const size_t height=y2-y1;
	if(height>StackBufferSize/sizeof(WCHAR))
	{
		HeapBuffer=new WCHAR[height];
		BufPtr=HeapBuffer;
	}
	wmemset(BufPtr, BoxSymbols[Type?BS_V2:BS_V1], height-1);
	BufPtr[height-1]=0;
	GotoXY(x1,y1+1);
	VText(BufPtr);
	GotoXY(x2,y1+1);
	VText(BufPtr);
	const size_t width=x2-x1+2;
	if(width>StackBufferSize/sizeof(WCHAR))
	{
		if(width>height)
		{
			if(HeapBuffer)
			{
				delete[] HeapBuffer;
			}
			HeapBuffer=new WCHAR[width];
		}
		BufPtr=HeapBuffer;
	}
	BufPtr[0]=BoxSymbols[Type?BS_LT_H2V2:BS_LT_H1V1];
	wmemset(BufPtr+1, BoxSymbols[Type?BS_H2:BS_H1], width-3);
	BufPtr[width-2]=BoxSymbols[Type?BS_RT_H2V2:BS_RT_H1V1];
	BufPtr[width-1]=0;
	GotoXY(x1,y1);
	Text(BufPtr);
	BufPtr[0]=BoxSymbols[Type?BS_LB_H2V2:BS_LB_H1V1];
	BufPtr[width-2]=BoxSymbols[Type?BS_RB_H2V2:BS_RB_H1V1];
	GotoXY(x1,y2);
	Text(BufPtr);

	if(HeapBuffer)
	{
		delete[] HeapBuffer;
	}
}

bool ScrollBarRequired(UINT Length, UINT64 ItemsCount)
{
	return Length>2 && ItemsCount && Length<ItemsCount;
}

bool ScrollBarEx(UINT X1,UINT Y1,UINT Length,UINT64 TopItem,UINT64 ItemsCount)
{
	if ( !ScrollBarRequired(Length, ItemsCount) )
		return false;
	return
		ScrollBarEx3(X1, Y1, Length, TopItem,TopItem+Length,ItemsCount);
}

bool ScrollBarEx3(UINT X1,UINT Y1,UINT Length, UINT64 Start,UINT64 End,UINT64 Size)
{
	WCHAR StackBuffer[4000/sizeof(WCHAR)], *Buffer = StackBuffer;
	if ( Length <= 2 || End < Start )
		return false;
	if ( Length >= ARRAYSIZE(StackBuffer) )
		Buffer = new WCHAR[Length + 1];

	Buffer[0] = Oem2Unicode[0x1E];
	Buffer[Length--] = L'\0';
	Buffer[Length--] = Oem2Unicode[0x1F];
	WCHAR b0 = BoxSymbols[BS_X_B0], b2 = BoxSymbols[BS_X_B2];
	UINT i, i1, i2;

	if ( End > Size )
		End = Size;

	if ( !Size || Start >= End )
	{
		i1 = Length+1;
		i2 = 0;
	}
	else
	{
		UINT thickness = static_cast<UINT>(((End - Start) * Length) / Size);
		if ( !thickness )
			++thickness;

		if ( thickness >= Length )
			i2 = (i1 = 1) + Length;
		else if ( End >= Size )
			i1 = (i2 = 1 + Length) - thickness;
		else
		{
			i1 = 1 + static_cast<UINT>((Start*Length + Size/2)/ Size);
			if ( i1 == 1 && Start > 0 )
				++i1;
			i2 = i1 + thickness;
			if ( i2 >= Length+1 )
			{
				i2 = Length + 1;
			   if ( i1 < i2-1 )
					--i2;
			}
		}
	}

	for (i = 1; i <= Length; i++)
		Buffer[i] = (i >= i1 && i < i2 ? b2 : b0);

	GotoXY(X1, Y1);
	VText(Buffer);
	if ( Buffer != StackBuffer )
		delete[] Buffer;

	return true;
}

void ScrollBar(int X1,int Y1,int Length,unsigned long Current,unsigned long Total)
{
	int ThumbPos;

	if ((Length-=2)<1)
		return;

	if (Total>0)
		ThumbPos=Length*Current/Total;
	else
		ThumbPos=0;

	if (ThumbPos>=Length)
		ThumbPos=Length-1;

	GotoXY(X1,Y1);
	{
		WCHAR StackBuffer[StackBufferSize/sizeof(WCHAR)];
		LPWSTR HeapBuffer=nullptr;
		LPWSTR BufPtr=StackBuffer;
		if(static_cast<size_t>(Length+3)>=StackBufferSize/sizeof(WCHAR))
		{
			HeapBuffer=new WCHAR[Length+3];
			BufPtr=HeapBuffer;
		}
		wmemset(BufPtr+1,BoxSymbols[BS_X_B0],Length);
		BufPtr[ThumbPos+1]=BoxSymbols[BS_X_B2];
		BufPtr[0]=Oem2Unicode[0x1E];
		BufPtr[Length+1]=Oem2Unicode[0x1F];
		BufPtr[Length+2]=0;
		VText(BufPtr);
		if(HeapBuffer)
		{
			delete[] HeapBuffer;
		}
	}
}

void DrawLine(int Length,int Type, const wchar_t* UserSep)
{
	if (Length>1)
	{
		WCHAR StackBuffer[StackBufferSize/sizeof(WCHAR)];
		LPWSTR HeapBuffer=nullptr;
		LPWSTR BufPtr=StackBuffer;
		if(static_cast<size_t>(Length)>=StackBufferSize/sizeof(WCHAR))
		{
			HeapBuffer=new WCHAR[Length+1];
			BufPtr=HeapBuffer;
		}
		MakeSeparator(Length,BufPtr,Type,UserSep);

		(Type >= 4 && Type <= 7) || (Type >= 10 && Type <= 11)? VText(BufPtr) : Text(BufPtr);
		if(HeapBuffer)
		{
			delete[] HeapBuffer;
		}
	}
}

// "Нарисовать" сепаратор в памяти.
WCHAR* MakeSeparator(int Length,WCHAR *DestStr,int Type, const wchar_t* UserSep)
{
	wchar_t BoxType[12][3]=
	{
		// h-horiz, s-space, v-vert, b-border, 1-one line, 2-two line
		/* 00 */{L' ',                 L' ',                 BoxSymbols[BS_H1]}, //  -     h1s
		/* 01 */{BoxSymbols[BS_L_H1V2],BoxSymbols[BS_R_H1V2],BoxSymbols[BS_H1]}, // ||-||  h1b2
		/* 02 */{BoxSymbols[BS_L_H1V1],BoxSymbols[BS_R_H1V1],BoxSymbols[BS_H1]}, // |-|    h1b1
		/* 03 */{BoxSymbols[BS_L_H2V2],BoxSymbols[BS_R_H2V2],BoxSymbols[BS_H2]}, // ||=||  h2b2

		/* 04 */{L' ',                 L' ',                 BoxSymbols[BS_V1]}, //  |     v1s
		/* 05 */{BoxSymbols[BS_T_H2V1],BoxSymbols[BS_B_H2V1],BoxSymbols[BS_V1]}, // =|=    v1b2
		/* 06 */{BoxSymbols[BS_T_H1V1],BoxSymbols[BS_B_H1V1],BoxSymbols[BS_V1]}, // -|-    v1b1
		/* 07 */{BoxSymbols[BS_T_H2V2],BoxSymbols[BS_B_H2V2],BoxSymbols[BS_V2]}, // =||=   v2b2

		/* 08 */{BoxSymbols[BS_H1],    BoxSymbols[BS_H1],    BoxSymbols[BS_H1]}, // -      h1
		/* 09 */{BoxSymbols[BS_H2],    BoxSymbols[BS_H2],    BoxSymbols[BS_H2]}, // =      h2
		/* 10 */{BoxSymbols[BS_V1],    BoxSymbols[BS_V1],    BoxSymbols[BS_V1]}, // |      v1
		/* 11 */{BoxSymbols[BS_V2],    BoxSymbols[BS_V2],    BoxSymbols[BS_V2]}, // ||     v2
	};

	if (Length>1 && DestStr)
	{
		Type%=ARRAYSIZE(BoxType);
		wmemset(DestStr,BoxType[Type][2],Length);
		DestStr[0]=BoxType[Type][0];
		DestStr[Length-1]=BoxType[Type][1];
		DestStr[Length]=0;
	}

	return DestStr;
}

string& HiText2Str(string& strDest, const wchar_t *Str)
{
	const wchar_t *ChPtr;
	string strDestTemp = Str;

	if ((ChPtr=wcschr(Str,L'&')) )
	{
		/*
		   &&      = '&'
		   &&&     = '&'
		              ^H
		   &&&&    = '&&'
		   &&&&&   = '&&'
		              ^H
		   &&&&&&  = '&&&'
		*/
		int I=0;
		const wchar_t *ChPtr2=ChPtr;

		while (*ChPtr2++ == L'&')
			++I;

		if (I&1) // нечет?
		{
			strDestTemp.SetLength(ChPtr-Str);

			if (ChPtr[1])
			{
				wchar_t Chr[]={ChPtr[1],0};
				strDestTemp+=Chr;
				string strText = (ChPtr+1);
				ReplaceStrings(strText,L"&&",L"&",-1);
				strDestTemp+=strText.CPtr()+1;
			}
		}
		else
		{
			ReplaceStrings(strDestTemp,L"&&",L"&",-1);
		}
	}

	strDest = strDestTemp;

	return strDest;
}

int HiStrlen(const wchar_t *Str)
{
	/*
			&&      = '&'
			&&&     = '&'
                       ^H
			&&&&    = '&&'
			&&&&&   = '&&'
                       ^H
			&&&&&&  = '&&&'
	*/

	int Length=0;
	bool Hi=false;

	if (Str)
	{
		while (*Str)
		{
			if (*Str == L'&')
			{
				int Count=0;

				while (*Str == L'&')
				{
					Str++;
					Count++;
				}

				if (Count&1) //нечёт?
				{
					if (Hi)
						Length++;
					else
						Hi=true;
				}

				Length+=Count/2;
			}
			else
			{
				Str++;
				Length++;
			}
		}
	}

	return Length;

}

int HiFindRealPos(const wchar_t *Str, int Pos, BOOL ShowAmp)
{
	/*
			&&      = '&'
			&&&     = '&'
                       ^H
			&&&&    = '&&'
			&&&&&   = '&&'
                       ^H
			&&&&&&  = '&&&'
	*/

	if (ShowAmp)
	{
		return Pos;
	}

	int RealPos = 0;

	if (Str)
	{
		int VisPos = 0;
		while (VisPos < Pos && *Str)
		{
			if (*Str == L'&')
			{
				Str++;
				RealPos++;

				if (*Str == L'&' && *(Str+1) == L'&' && *(Str+2) != L'&')
				{
					Str++;
					RealPos++;
				}
			}

			Str++;
			VisPos++;
			RealPos++;
		}
	}

	return RealPos;
}

int HiFindNextVisualPos(const wchar_t *Str, int Pos, int Direct)
{
	/*
			&&      = '&'
			&&&     = '&'
                       ^H
			&&&&    = '&&'
			&&&&&   = '&&'
                       ^H
			&&&&&&  = '&&&'
	*/

	if (Str)
	{
		if (Direct < 0)
		{
			if (!Pos || Pos == 1)
				return 0;

			if (Str[Pos-1] != L'&')
			{
				if (Str[Pos-2] == L'&')
				{
					if (Pos-3 >= 0 && Str[Pos-3] == L'&')
						return Pos-1;

					return Pos-2;
				}

				return Pos-1;
			}
			else
			{
				if (Pos-3 >= 0 && Str[Pos-3] == L'&')
					return Pos-3;

				return Pos-2;
			}
		}
		else
		{
			if (!Str[Pos])
				return Pos+1;

			if (Str[Pos] == L'&')
			{
				if (Str[Pos+1] == L'&' && Str[Pos+2] == L'&')
					return Pos+3;

				return Pos+2;
			}
			else
			{
				return Pos+1;
			}
		}
	}

	return 0;
}

bool IsConsoleFullscreen()
{
	bool Result=false;
	static bool Supported = Console.IsFullscreenSupported();
	if(Supported)
	{
		DWORD ModeFlags=0;
		if(Console.GetDisplayMode(ModeFlags) && ModeFlags&CONSOLE_FULLSCREEN_HARDWARE)
		{
			Result=true;
		}
	}
	return Result;
}
