/*
interf.cpp

Консольные функции ввода-вывода

*/

#include "headers.hpp"
#pragma hdrstop

#include "farqueue.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "plugin.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "fileedit.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"


BOOL __stdcall CtrlHandler(DWORD CtrlType);

static int CurX,CurY;
static int CurColor;

static int OutputCP;
BYTE RecodeOutTable[256];
static int InitCurVisible,InitCurSize;
static const char CONOUT[]="CONOUT$";
static const char CONIN[]="CONIN$";
#if defined(USE_WFUNC)
static const WCHAR LCONOUT[]=L"CONOUT$";
static const WCHAR LCONIN[]=L"CONIN$";
#endif

#if defined(USE_WFUNC)
WCHAR Oem2Unicode[256];
#endif

static void __Create_CONOUT()
{
#if defined(USE_WFUNC)

	if (Opt.UseUnicodeConsole)
		hConOut=CreateFileW(LCONOUT,GENERIC_READ|GENERIC_WRITE,
		                    FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	else
		hConOut=CreateFileA(CONOUT,GENERIC_READ|GENERIC_WRITE,
		                    FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

#else
	hConOut=CreateFile(CONOUT,GENERIC_READ|GENERIC_WRITE,
	                   FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
#endif
	ScrBuf.SetHandle(hConOut);
}

static void __Create_CONIN()
{
#if defined(USE_WFUNC_IN)

	if (Opt.UseUnicodeConsole)
		hConInp=CreateFileW(LCONIN,GENERIC_READ|GENERIC_WRITE,
		                    FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	else
		hConInp=CreateFileA(CONIN,GENERIC_READ|GENERIC_WRITE,
		                    FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

#else
	hConInp=CreateFile(CONIN,GENERIC_READ|GENERIC_WRITE,
	                   FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
#endif
}

void InitConsole(int FirstInit)
{
	InitRecodeOutTable();
	__Create_CONOUT();
	__Create_CONIN();
	SetConsoleCtrlHandler(CtrlHandler,TRUE);
	GetConsoleMode(hConInp,&InitialConsoleMode);
	GetRealCursorType(InitCurVisible,InitCurSize);
	GetRegKey("Interface","Mouse",Opt.Mouse,1);

	// размер клавиатурной очереди = 1024 кода клавиши
	if (!KeyQueue)
		KeyQueue=new FarQueue<DWORD>(1024);

	SetFarConsoleMode();

	//SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
	/* $ 09.04.2002 DJ
	   если мы под NT и размер консольного буфера больше размера окна, выставим
	   их равными
	*/
	if (FirstInit)
	{
		GetVideoMode(InitScreenBufferInfo);

		if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
		{
			if (InitScreenBufferInfo.srWindow.Left != 0 ||
			        InitScreenBufferInfo.srWindow.Top != 0 ||
			        InitScreenBufferInfo.srWindow.Right != InitScreenBufferInfo.dwSize.X-1 ||
			        InitScreenBufferInfo.srWindow.Bottom != InitScreenBufferInfo.dwSize.Y-1)
			{
				COORD newSize;
				newSize.X = InitScreenBufferInfo.srWindow.Right - InitScreenBufferInfo.srWindow.Left + 1;
				newSize.Y = InitScreenBufferInfo.srWindow.Bottom - InitScreenBufferInfo.srWindow.Top + 1;
				SetConsoleScreenBufferSize(hConOut, newSize);
				GetVideoMode(InitScreenBufferInfo);
			}
		}
	}

	/* DJ $ */
	GetVideoMode(CurScreenBufferInfo);
	ScrBuf.FillBuf();

	// было sizeof(Palette)
	/*$ 14.02.2001 SKV
	  для consoledetach не нужно, что бы инитилась палитра.
	*/
	if (FirstInit)
		memcpy(Palette,DefaultPalette,SizeArrayPalette);

	/* SKV$*/
#if defined(DETECT_ALT_ENTER)
	PrevFarAltEnterMode=FarAltEnter(FAR_CONSOLE_GET_MODE);
#endif
}


void ReopenConsole()
{
	HANDLE hOldOut=hConOut;
	__Create_CONOUT();
	SetStdHandle(STD_OUTPUT_HANDLE,hConOut);
	CloseHandle(hOldOut);
}


void CloseConsole()
{
	ScrBuf.Flush();
	SetRealCursorType(InitCurVisible,InitCurSize);
	ChangeConsoleMode(InitialConsoleMode);
	CloseHandle(hConInp);
	CloseHandle(hConOut);
	delete KeyQueue;
	KeyQueue=NULL;
}


void SetFarConsoleMode(BOOL SetsActiveBuffer)
{
	int Mode=ENABLE_WINDOW_INPUT;

	if (Opt.Mouse)
		Mode|=ENABLE_MOUSE_INPUT;

	if (SetsActiveBuffer)
		SetConsoleActiveScreenBuffer(hConOut);

	ChangeConsoleMode(Mode);
}

void ChangeConsoleMode(int Mode)
{
	DWORD CurrentConsoleMode;
	GetConsoleMode(hConInp,&CurrentConsoleMode);

	if (CurrentConsoleMode!=(DWORD)Mode)
		SetConsoleMode(hConInp,Mode);
}


void SetFarTitle(const char *Title)
{
	static char FarTitle[2*NM];
	char OldFarTitle[2*NM];
//_SVS(SysLog("SetFarTitle('%s')",Title));
	GetConsoleTitle(OldFarTitle,sizeof(OldFarTitle));

	if (Title)
	{
		sprintf(FarTitle,"%.256s%s",Title,FarTitleAddons);
		TitleModified=TRUE;

		if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
			FAR_OemToChar(FarTitle,FarTitle);

		if (strcmp(OldFarTitle,FarTitle) &&
		        (CtrlObject->Macro.IsExecuting() && !CtrlObject->Macro.IsDsableOutput() ||
		         !CtrlObject->Macro.IsExecuting() || CtrlObject->Macro.IsExecutingLastKey())
		   )
		{
			//_SVS(SysLog("  FarTitle='%s'",FarTitle));
			SetConsoleTitle(FarTitle);
			TitleModified=FALSE;
		}
	}
	else
	{
		/*
		  Title=NULL для случая, когда нужно выставить пред.заголовок
		  SetFarTitle(NULL) - это не для всех!
		  Этот вызов имеет право делать только макро-движок!
		*/
		SetConsoleTitle(FarTitle);
		TitleModified=FALSE;
		//_SVS(SysLog("  (NULL)FarTitle='%s'",FarTitle));
	}
}

void FlushInputBuffer()
{
	FlushConsoleInputBuffer(hConInp);
	LButtonPressed=PrevLButtonPressed=0;
	RButtonPressed=PrevRButtonPressed=0;
	MouseEventFlags=0;
}

//OT
int operator==(CONSOLE_SCREEN_BUFFER_INFO &csbi1,CONSOLE_SCREEN_BUFFER_INFO &csbi2)
{
	return csbi1.dwSize.X==csbi2.dwSize.X && csbi1.dwSize.Y==csbi2.dwSize.Y;
}

#define _OT_ConsoleInfo(_hCon) \
	GetConsoleScreenBufferInfo(_hCon, &_csbi);\
	_OT(SysLog("Current Screen Buffer info[0x%p]:\n\
    dwSize           = (%3i,%3i)\n\
    dwCursorPosition = (%3i,%3i)\n\
    wAttributes        =  0x%02x\n\
    srWindow           = (%3i,%3i), (%3i,%3i)\n\
    dwMaximumWindowSize= (%3i,%3i)\n"\
	           ,_hCon\
	           ,_csbi.dwSize.X,_csbi.dwSize.Y\
	           ,_csbi.dwCursorPosition.X,_csbi.dwCursorPosition.Y\
	           ,_csbi.wAttributes\
	           ,_csbi.srWindow.Left,_csbi.srWindow.Top,_csbi.srWindow.Right,_csbi.srWindow.Bottom\
	           ,_csbi.dwMaximumWindowSize.X,_csbi.dwMaximumWindowSize.Y\
	          ))

_OT(void ViewConsoleInfo()\
    {\
     CONSOLE_SCREEN_BUFFER_INFO _csbi; \
     _OT_ConsoleInfo(hConOut); \
     _OT_ConsoleInfo(hConInp); \
    })

void SetVideoMode(int ScreenMode)
{
	if (!ScreenMode && Opt.AltF9 /*&& WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT*/)
	{
		ChangeVideoMode(InitScreenBufferInfo==CurScreenBufferInfo);
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
		SendMessage(hFarWnd,WM_SYSCOMMAND,SC_MAXIMIZE,(LPARAM)0);
		coordScreen = GetLargestConsoleWindowSize(hConOut);
		coordScreen.X+=Opt.ScrSize.DeltaXY.X;
		coordScreen.Y+=Opt.ScrSize.DeltaXY.Y;
	}
	else
	{
		SendMessage(hFarWnd,WM_SYSCOMMAND,SC_RESTORE,(LPARAM)0);
		coordScreen = InitScreenBufferInfo.dwSize;
	}

	ChangeVideoMode(coordScreen.Y,coordScreen.X);
}

void ChangeVideoMode(int NumLines,int NumColumns)
{
	_OT(DWORD le);
	_OT(SysLog("ChangeVideoMode(NumLines=%i, NumColumns=%i)",NumLines,NumColumns));
	int retSetConsole;
	CONSOLE_SCREEN_BUFFER_INFO csbi; /* hold current console buffer info */
	SMALL_RECT srWindowRect; /* hold the new console size */
	COORD coordScreen;
	int xSize=NumColumns,ySize=NumLines;

	if (!GetConsoleScreenBufferInfo(hConOut, &csbi))
	{
		return;
	}

#if 1
	/*  if(WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT)
	  {
	    if (NumColumns < csbi.dwSize.X || NumLines < csbi.dwSize.Y )
	    {
	      csbi.srWindow.Right  = csbi.srWindow.Left + (NumColumns-1);
	      csbi.srWindow.Bottom = csbi.srWindow.Top  + (NumLines-1);
	      SetConsoleWindowInfo(hConOut,TRUE,&csbi.srWindow );

	      csbi.dwSize.X = NumColumns;
	      csbi.dwSize.Y = NumLines;
	      SetConsoleScreenBufferSize(hConOut,csbi.dwSize );
	    }
	    else
	    {
	      csbi.dwSize.X = NumColumns;
	      csbi.dwSize.Y = NumLines;
	      SetConsoleScreenBufferSize(hConOut,csbi.dwSize );

	      csbi.srWindow.Right  = csbi.srWindow.Left + (NumColumns-1);
	      csbi.srWindow.Bottom = csbi.srWindow.Top  + (NumLines-1);
	      SetConsoleWindowInfo(hConOut,TRUE,&csbi.srWindow );
	    }
	  }*/
#else

	if (WinVer.dwPlatformId != VER_PLATFORM_WIN32_NT ||
	        (NumColumns == 80 && (NumLines == 25 || NumLines == 50)) // обеспечим выполнение !Opt.AltF9
	   )
	{
		/* get the largest size we can size the console window to */
		coordScreen = GetLargestConsoleWindowSize(hConOut);
		/* define the new console window size and scroll position */
		srWindowRect.Right = (SHORT)(Min((short)xSize, coordScreen.X) - 1);
		srWindowRect.Bottom = (SHORT)(Min((short)ySize, coordScreen.Y) - 1);
		srWindowRect.Left = srWindowRect.Top = (SHORT) 0;
		/* define the new console buffer size */
		coordScreen.X = xSize;
		coordScreen.Y = ySize;

		/* if the current buffer is larger than what we want, resize the */

		/* console window first, then the buffer */
		if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y > (DWORD) xSize * ySize)
		{
			SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);
			SetConsoleScreenBufferSize(hConOut, coordScreen);
		}

		/* if the current buffer is smaller than what we want, resize the */

		/* buffer first, then the console window */
		if ((DWORD) csbi.dwSize.X * csbi.dwSize.Y < (DWORD) xSize * ySize)
		{
			SetConsoleScreenBufferSize(hConOut, coordScreen);
			SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);
		}

//    GetVideoMode();
	}

#endif
//  else
	{
		srWindowRect.Right = xSize-1;
		srWindowRect.Bottom = ySize-1;
		srWindowRect.Left = srWindowRect.Top = (SHORT) 0;
		/* define the new console buffer size */
		coordScreen.X = xSize;
		coordScreen.Y = ySize;

		if (xSize>csbi.dwSize.X || ySize > csbi.dwSize.Y)
		{
			if (csbi.dwSize.X < xSize-1)
			{
				srWindowRect.Right = csbi.dwSize.X - 1;
				retSetConsole=SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);

				if (!retSetConsole)
				{
					_OT(SysLog("LastError=%i, srWindowRect=(%i.%i)(%i.%i)",le=GetLastError(),srWindowRect.Left,srWindowRect.Top,srWindowRect.Right,srWindowRect.Bottom));
				}

				srWindowRect.Right = xSize-1;
			}

			if (csbi.dwSize.Y < ySize-1)
			{
				srWindowRect.Bottom=csbi.dwSize.Y - 1;
				retSetConsole=SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);

				if (!retSetConsole)
				{
					_OT(SysLog("LastError=%i, srWindowRect",le=GetLastError()));
				}

				srWindowRect.Bottom = ySize-1;
			}

			retSetConsole=SetConsoleScreenBufferSize(hConOut, coordScreen);

			if (!retSetConsole)
			{
				_OT(SysLog("SetConsoleScreenBufferSize failed... coordScreen=(%i,%i)\n\
          LastError=%i",coordScreen.X,coordScreen.Y,le=GetLastError()));
			}
		}

		if ((retSetConsole=SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect)) == 0)
		{
			_OT(SysLog("LastError=%i, srWindowRect",le=GetLastError()));
			retSetConsole=SetConsoleScreenBufferSize(hConOut, coordScreen);
			_OT(le=GetLastError());
			_OT(SysLog("SetConsoleScreenBufferSize(hConOut, coordScreen),  retSetConsole=%i",retSetConsole));
			retSetConsole=SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect);
			_OT(le=GetLastError());
			_OT(SysLog("SetConsoleWindowInfo(hConOut, TRUE, &srWindowRect),  retSetConsole=%i",retSetConsole));
			_OT(ViewConsoleInfo());
		}
		else
		{
			retSetConsole=SetConsoleScreenBufferSize(hConOut, coordScreen);
			_OT(le=GetLastError());
			_OT(SysLog("SetConsoleScreenBufferSize(hConOut, coordScreen),  retSetConsole=%i",retSetConsole));
		}
	}

	// зашлем эвент только в случае, когда макросы не исполняются
	if (CtrlObject && !CtrlObject->Macro.IsExecuting())
		GenerateWINDOW_BUFFER_SIZE_EVENT(NumColumns,NumLines);
}

void GenerateWINDOW_BUFFER_SIZE_EVENT(int Sx, int Sy)
{
	INPUT_RECORD Rec;
	DWORD Writes;
	CONSOLE_SCREEN_BUFFER_INFO csbi; /* hold current console buffer info */

	if (Sx==-1 || Sy==-1)
		GetConsoleScreenBufferInfo(hConOut, &csbi);

	Rec.EventType=WINDOW_BUFFER_SIZE_EVENT;
	Rec.Event.WindowBufferSizeEvent.dwSize.X=Sx==-1?csbi.dwSize.X:Sx;
	Rec.Event.WindowBufferSizeEvent.dwSize.Y=Sy==-1?csbi.dwSize.Y:Sy;
//_SVS(SysLog("[%d:%d] = [%d:%d, %d:%d]",csbi.dwSize.X,csbi.dwSize.Y,csbi.srWindow.Left,csbi.srWindow.Top,csbi.srWindow.Right,csbi.srWindow.Bottom));
	WriteConsoleInput(hConInp,&Rec,1,&Writes);
}

void GetVideoMode(CONSOLE_SCREEN_BUFFER_INFO &csbi)
{
	memset(&csbi,0,sizeof(csbi));
	GetConsoleScreenBufferInfo(hConOut,&csbi);
	ScrX=csbi.dwSize.X-1;
	ScrY=csbi.dwSize.Y-1;
	WidthNameForMessage=(ScrX*38)/100+1;

	if (PrevScrX == -1) PrevScrX=ScrX;

	if (PrevScrY == -1) PrevScrY=ScrY;

	_OT(SysLog("ScrX=%d ScrY=%d",ScrX,ScrY));
	ScrBuf.AllocBuf(csbi.dwSize.X,csbi.dwSize.Y);
	_OT(ViewConsoleInfo());
}

BOOL __stdcall CtrlHandler(DWORD CtrlType)
{
	if (CtrlType==CTRL_C_EVENT || CtrlType==CTRL_BREAK_EVENT)
	{
		if (CtrlType==CTRL_BREAK_EVENT)
			WriteInput(KEY_BREAK);

		if (CtrlObject && CtrlObject->Cp())
		{
			if (CtrlObject->Cp()->LeftPanel!=NULL && CtrlObject->Cp()->LeftPanel->GetMode()==PLUGIN_PANEL)
				CtrlObject->Plugins.ProcessEvent(CtrlObject->Cp()->LeftPanel->GetPluginHandle(),FE_BREAK,(void *)(DWORD_PTR)CtrlType);

			if (CtrlObject->Cp()->RightPanel!=NULL && CtrlObject->Cp()->RightPanel->GetMode()==PLUGIN_PANEL)
				CtrlObject->Plugins.ProcessEvent(CtrlObject->Cp()->RightPanel->GetPluginHandle(),FE_BREAK,(void *)(DWORD_PTR)CtrlType);
		}

		return(TRUE);
	}

	CloseFAR=TRUE;

	/* $ 30.08.2001 IS
	   При закрытии окна "по кресту" всегда возвращаем TRUE, в противном случае
	   Фар будет закрыт системой и не будут выполнены обычные при закрытии
	   процедуры: оповещены плагины, вызваны деструкторы, сохранены настройки и
	   т.п.
	*/
	if (!Opt.CloseConsoleRule)
	{
		if (CurrentEditor!=NULL && CurrentEditor->IsFileModified() ||
		        (FrameManager && FrameManager->IsAnyFrameModified(FALSE)))
			return(TRUE);

		return(FALSE);
	}

	return TRUE;
	/* IS $ */
}


void ShowTime(int ShowAlways)
{
	char ClockText[10];
	static SYSTEMTIME lasttm={0,0,0,0,0,0,0,0};
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	CHAR_INFO ScreenClockText[5];
	GetText(ScrX-4,0,ScrX,0,ScreenClockText,sizeof(ScreenClockText));

	if (ShowAlways==2)
	{
		memset(&lasttm,0,sizeof(lasttm));
		return;
	}

	if (!ShowAlways && lasttm.wMinute==tm.wMinute && lasttm.wHour==tm.wHour &&
	        GetVidChar(ScreenClockText[2])==':' || ScreenSaverActive)
		return;

	ProcessShowClock++;
	lasttm=tm;
	sprintf(ClockText,"%02d:%02d",tm.wHour,tm.wMinute);
	GotoXY(ScrX-4,0);
	// Здесь хрень какая-то получается с ModType - все время не верное значение!
	Frame *CurFrame=FrameManager->GetCurrentFrame();

	if (CurFrame)
	{
		int ModType=CurFrame->GetType();
		SetColor(ModType==MODALTYPE_VIEWER?COL_VIEWERCLOCK:
		         (ModType==MODALTYPE_EDITOR?COL_EDITORCLOCK:COL_CLOCK));
		Text(ClockText);
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


void GetCursorPos(int& X,int& Y)
{
	ScrBuf.GetCursorPos(X,Y);
}


void SetCursorType(int Visible,int Size)
{
	if (Size==-1 || !Visible)
		Size=IsWindowed()?
		     (Opt.CursorSize[0]?Opt.CursorSize[0]:InitCurSize):
				     (Opt.CursorSize[1]?Opt.CursorSize[1]:InitCurSize);

	ScrBuf.SetCursorType(Visible,Size);
}

void SetInitialCursorType()
{
	ScrBuf.SetCursorType(InitCurVisible,InitCurSize);
}


void GetCursorType(int &Visible,int &Size)
{
	ScrBuf.GetCursorType(Visible,Size);
}


void MoveRealCursor(int X,int Y)
{
	COORD C;
	C.X=X;
	C.Y=Y;
	SetConsoleCursorPosition(hConOut,C);
}


void GetRealCursorPos(int& X,int& Y)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConOut,&csbi);
	X=csbi.dwCursorPosition.X;
	Y=csbi.dwCursorPosition.Y;
}


void SetRealCursorType(int Visible,int Size)
{
	CONSOLE_CURSOR_INFO cci;
	cci.dwSize=Size;
	cci.bVisible=Visible;
	SetConsoleCursorInfo(hConOut,&cci);
}


void GetRealCursorType(int &Visible,int &Size)
{
	CONSOLE_CURSOR_INFO cci;
	GetConsoleCursorInfo(hConOut,&cci);
	Size=cci.dwSize;
	Visible=cci.bVisible;
}

#if defined(USE_WFUNC)

static BOOL DetectTTFFont(void)
{
	char AppName[NM*2], OptRegRoot[NM];
	BOOL UseTTFConsoleFont=FALSE;
	GetModuleFileName(NULL,AppName,sizeof(AppName));
	SetRegRootKey(HKEY_CURRENT_USER);
	strcpy(OptRegRoot,Opt.RegRoot);
	strcpy(Opt.RegRoot,"Console");
	ReplaceStrings(AppName,"\\","_",-1);

	if (!CheckRegKey(AppName))
	{
		Opt.RegRoot[0]=0;
		strcpy(AppName,"Console");
	}

	int FontFamily=GetRegKey(AppName,"FontFamily",0);

	if (FontFamily && Opt.UseUnicodeConsole == -1)
		UseTTFConsoleFont=(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && FontFamily==0x36)?TRUE:FALSE;

	strcpy(Opt.RegRoot,OptRegRoot);
	return UseTTFConsoleFont;
}

#endif

void InitRecodeOutTable(UINT cp)
{
	int I;
	OutputCP=!cp?GetConsoleOutputCP():cp;

	for (I=0; I<sizeof(RecodeOutTable)/sizeof(RecodeOutTable[0]); I++)
		RecodeOutTable[I]=I;

	if (Opt.CleanAscii)
	{
		for (I=0x00; I<0x20; I++)
			RecodeOutTable[I]='.';

		RecodeOutTable[0x07]='*';
		RecodeOutTable[0x18]=RecodeOutTable[0x19]='|';
		RecodeOutTable[0x1E]='X';
		RecodeOutTable[0x1F]='X';
		RecodeOutTable[0xFF]=' ';
		RecodeOutTable[0x10]='>';
		RecodeOutTable[0x11]='<';
	}

	if (Opt.NoGraphics)
	{
		for (I=0xB3; I<=0xDA; I++)
			RecodeOutTable[I]='+';

		RecodeOutTable[0xB3]=RecodeOutTable[0xBA]='|';
		RecodeOutTable[0xC4]='-';
		RecodeOutTable[0xCD]='=';
	}

#if defined(USE_WFUNC)

	if (Opt.UseUnicodeConsole)
	{
		static WCHAR _Oem2Unicode[256] =
		{
			/*00*/ 0x0000, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
			0x25D8, 0x0000, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
			/*10*/ 0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25A0, 0x21A8,
			0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC,
			/*20*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*30*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*40*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*50*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*60*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*70*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2206,
			/*80*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*90*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*A0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*B0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*C0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*D0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*E0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			/*F0*/ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x221A, 0x0000, 0x0000, 0x0000, 0x0000,
		};
		// перед [пере]инициализацией восстановим буфер (либо из реестра, либо...)
		Oem2Unicode[0]=0;
		GetRegKey("System","Oem2Unicode",(BYTE *)Oem2Unicode,(BYTE*)_Oem2Unicode,sizeof(Oem2Unicode));

		if (Opt.CleanAscii)
		{
			for (I=0; I<0x20; I++)
				Oem2Unicode[I]=0;

			Oem2Unicode[0x07]='*';
			Oem2Unicode[0x18]=Oem2Unicode[0x19]='|';
			Oem2Unicode[0x1E]='X';
			Oem2Unicode[0x1F]='X';
			Oem2Unicode[0xFF]=' ';
			Oem2Unicode[0x10]='>';
			Oem2Unicode[0x11]='<';
		}

		if (!DetectTTFFont())
			Oem2Unicode[0x7F]=0;

		/*
		    if (Opt.NoGraphics)
		    {
		      for (I=0xB3;I<=0xDA;I++)
		        Oem2Unicode[I]='+';
		      Oem2Unicode[0xB3]=Oem2Unicode[0xBA]='|';
		      Oem2Unicode[0xC4]='-';
		      Oem2Unicode[0xCD]='=';
		    }
		*/
		for (I=1; I<sizeof(RecodeOutTable)/sizeof(RecodeOutTable[0]); I++)
		{
			if (!Oem2Unicode[I])
			{
				CHAR Chr = (CHAR)RecodeOutTable[I];
				MultiByteToWideChar(CP_OEMCP, MB_USEGLYPHCHARS, &Chr, 1, Oem2Unicode+I, 1);
			}
		}

		static WCHAR _BoxSymbols[48] =
		{
			0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
			0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
			0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
			0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
			0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
			0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
		};
		// перед [пере]инициализацией восстановим буфер (либо из реестра, либо...)
		GetRegKey("System","BoxSymbols",(BYTE *)BoxSymbols,(BYTE*)_BoxSymbols,sizeof(_BoxSymbols));

		if (Opt.NoGraphics)
		{
			for (I=0xB3-0xB0; I<=0xDA-0xB0; I++)
				BoxSymbols[I]='+';

			BoxSymbols[0xB3-0xB0]=BoxSymbols[0xBA-0xB0]='|';
			BoxSymbols[0xC4-0xB0]='-';
			BoxSymbols[0xCD-0xB0]='=';
		}

		WideCharToMultiByte(CP_OEMCP,0 /* MB_USEGLYPHCHARS */, BoxSymbols,sizeof(_BoxSymbols)/sizeof(WCHAR),(LPSTR)BoxSymbolsA, sizeof(_BoxSymbols)/sizeof(WCHAR),  NULL, NULL);
	}

	//_SVS(SysLogDump("Oem2Unicode",0,(LPBYTE)Oem2Unicode,sizeof(Oem2Unicode),NULL));
#endif
}


#if defined(USE_WFUNC)
void TextW(int X, int Y, int Color, const WCHAR *Str)
{
	CurColor=FarColorToReal(Color);

	if (X<0) X=0;

	if (Y<0) Y=0;

	CurX=X;
	CurY=Y;
	TextW(Str);
}

void TextW(const WCHAR *Str)
{
	int Length=(int)wcslen(Str), I;

	if (CurX+Length>ScrX)
		Length=ScrX-CurX+1;

	if (Length<=0)
		return;

	CHAR_INFO CharBuf[1024], *PtrCharBuf;

	if (Length >= sizeof(CharBuf))
		Length=sizeof(CharBuf)-1;

	PtrCharBuf=CharBuf;

	for (I=0; I < Length; I++, ++PtrCharBuf)
	{
		PtrCharBuf->Char.UnicodeChar=Str[I];
		PtrCharBuf->Attributes=CurColor;
	}

	ScrBuf.Write(CurX,CurY,CharBuf,Length);
	CurX+=Length;
}
#endif

/* $ 23.07.2000 SVS
   + две полных функции Text
*/
/* $ 02.02.2001 VVM
    ! Переделал функции Text(...). Т.к. они вызываются очень часто,
      то основной вывод будет идти в Text(char *Str) */
void Text(int X, int Y, int Color, const char *Str)
{
	CurColor=FarColorToReal(Color);
	CurX=X;
	CurY=Y;
	Text(Str);
}

void Text(const char *Str)
{
	int Length=(int)strlen(Str), I;

	if (CurX+Length>ScrX)
		Length=ScrX-CurX+1;

	if (Length<=0)
		return;

	CHAR_INFO CharBuf[1024], *PtrCharBuf;

	if (Length >= sizeof(CharBuf)/sizeof(CharBuf[0]))
		Length=sizeof(CharBuf)/sizeof(CharBuf[0])-1;

	PtrCharBuf=CharBuf;
#if defined(USE_WFUNC)

	if (Opt.UseUnicodeConsole)
	{
		for (I=0; I < Length; I++, ++PtrCharBuf)
		{
			PtrCharBuf->Char.UnicodeChar=Oem2Unicode[RecodeOutTable[(BYTE)Str[I]]];
			PtrCharBuf->Attributes=CurColor;
		}
	}
	else
#endif
	{
		for (I=0; I < Length; I++, ++PtrCharBuf)
		{
			PtrCharBuf->Char.AsciiChar=RecodeOutTable[(BYTE)Str[I]];
			PtrCharBuf->Attributes=CurColor;
		}
	}

	ScrBuf.Write(CurX,CurY,CharBuf,Length);
	CurX+=Length;
}

void Text(int X, int Y, int Color,int MsgId)
{
	Text(X,Y,Color,MSG(MsgId));
}

void Text(int MsgId)
{
	Text(MSG(MsgId));
}
/* VVM $ */
/* SVS $ */

#if defined(USE_WFUNC)
void VTextW(const WCHAR *Str)
{
	int Length=(int)wcslen(Str);

	if (CurY+Length>ScrY)
		Length=ScrY-CurY+1;

	if (Length<=0)
		return;

	int StartCurX=CurX;
	WCHAR ChrStr[2]={0,0};

	for (int I=0; I<Length; I++)
	{
		GotoXY(CurX,CurY);
		ChrStr[0]=Str[I];
		TextW(ChrStr);
		CurY++;
		CurX=StartCurX;
	}
}
#endif

void VText(const char *Str)
{
	int Length=(int)strlen(Str);

	if (CurY+Length>ScrY)
		Length=ScrY-CurY+1;

	if (Length<=0)
		return;

	int StartCurX=CurX;
	char ChrStr[2]={0,0};

	for (int I=0; I<Length; I++)
	{
		GotoXY(CurX,CurY);
		ChrStr[0]=Str[I];
		Text(ChrStr);
		CurY++;
		CurX=StartCurX;
	}
}

void HiText(const char *Str,int HiColor,int isVertText)
{
	char TextStr[600];
	int SaveColor;
	char *ChPtr;
	xstrncpy(TextStr,Str,sizeof(TextStr)-1);

	if ((ChPtr=strchr(TextStr,'&'))==NULL)
	{
		if (isVertText)
			VText(TextStr);
		else
			Text(TextStr);
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
		int I=0;
		char *ChPtr2=ChPtr;

		while (*ChPtr2++ == '&')
			++I;

		if (I&1) // нечет?
		{
			*ChPtr=0;

			if (isVertText)
				VText(TextStr);
			else
				Text(TextStr);

			if (ChPtr[1])
			{
				char Chr[2];
				SaveColor=CurColor;
				SetColor(HiColor);
				Chr[0]=ChPtr[1]; Chr[1]=0;

				if (isVertText)
					VText(Chr);
				else
					Text(Chr);

				SetColor(SaveColor);
				ReplaceStrings(ChPtr+1,"&&","&",-1);

				if (isVertText)
					VText(ChPtr+2);
				else
					Text(ChPtr+2);
			}
		}
		else
		{
			ReplaceStrings(ChPtr,"&&","&",-1);

			if (isVertText)
				VText(TextStr);
			else
				Text(TextStr);
		}
	}
}


void SetScreen(int X1,int Y1,int X2,int Y2,int Ch,int Color)
{
	if (X1<0) X1=0;

	if (Y1<0) Y1=0;

	if (X2>ScrX) X2=ScrX;

	if (Y2>ScrY) Y2=ScrY;

#if defined(USE_WFUNC)
	ScrBuf.FillRect(X1,Y1,X2,Y2,(Opt.UseUnicodeConsole?Ch:RecodeOutTable[Ch]),FarColorToReal(Color));
#else
	ScrBuf.FillRect(X1,Y1,X2,Y2,RecodeOutTable[Ch],FarColorToReal(Color));
#endif
//  SetColor(Color);
//  GotoXY(X2,Y2);
}


void MakeShadow(int X1,int Y1,int X2,int Y2)
{
	if (X1<0) X1=0;

	if (Y1<0) Y1=0;

	if (X2>ScrX) X2=ScrX;

	if (Y2>ScrY) Y2=ScrY;

	ScrBuf.ApplyColorMask(X1,Y1,X2,Y2,0xF8);
}

void ChangeBlockColor(int X1,int Y1,int X2,int Y2,int Color)
{
	if (X1<0) X1=0;

	if (Y1<0) Y1=0;

	if (X2>ScrX) X2=ScrX;

	if (Y2>ScrY) Y2=ScrY;

	ScrBuf.ApplyColor(X1,Y1,X2,Y2,FarColorToReal(Color));
}

#if defined(USE_WFUNC)
void mprintfW(WCHAR *fmt,...)
{
	va_list argptr;
	va_start(argptr,fmt);
	WCHAR OutStr[2048];
	vswprintf(OutStr,fmt,argptr);
	TextW(OutStr);
	va_end(argptr);
}

void vmprintfW(WCHAR *fmt,...)
{
	va_list argptr;
	va_start(argptr,fmt);
	WCHAR OutStr[2048];
	vswprintf(OutStr,fmt,argptr);
	VTextW(OutStr);
	va_end(argptr);
}

void mprintf(char *fmt,...)
{
	va_list argptr;
	va_start(argptr,fmt);
	char OutStr[2048];
	vsnprintf(OutStr,sizeof(OutStr)-1,fmt,argptr);
	Text(OutStr);
	va_end(argptr);
}

void vmprintf(char *fmt,...)
{
	va_list argptr;
	va_start(argptr,fmt);
	char OutStr[2048];
	vsnprintf(OutStr,sizeof(OutStr)-1,fmt,argptr);
	VText(OutStr);
	va_end(argptr);
}

void mprintf(int MsgId,...)
{
	va_list argptr;
	va_start(argptr,MsgId);
	char OutStr[2048];
	vsnprintf(OutStr,sizeof(OutStr)-1,MSG(MsgId),argptr);
	Text(OutStr);
	va_end(argptr);
}

#else

void mprintf(char *fmt,...)
{
	va_list argptr;
	va_start(argptr,fmt);
	char OutStr[2048];
	vsnprintf(OutStr,sizeof(OutStr)-1,fmt,argptr);
	Text(OutStr);
	va_end(argptr);
}

void mprintf(int MsgId,...)
{
	va_list argptr;
	va_start(argptr,MsgId);
	char OutStr[2048];
	vsnprintf(OutStr,sizeof(OutStr)-1,MSG(MsgId),argptr);
	Text(OutStr);
	va_end(argptr);
}

void vmprintf(char *fmt,...)
{
	va_list argptr;
	va_start(argptr,fmt);
	char OutStr[2048];
	vsnprintf(OutStr,sizeof(OutStr)-1,fmt,argptr);
	VText(OutStr);
	va_end(argptr);
}
#endif

void SetColor(int Color)
{
	CurColor=FarColorToReal(Color);
}

void SetRealColor(int Color)
{
	CurColor=FarColorToReal(Color);
	SetConsoleTextAttribute(hConOut,CurColor);
}

void ClearScreen(int Color)
{
	Color=FarColorToReal(Color);
	ScrBuf.FillRect(0,0,ScrX,ScrY,' ',Color);
	ScrBuf.ResetShadow();
	ScrBuf.Flush();
	SetConsoleTextAttribute(hConOut,Color);
}

int GetColor()
{
	return(CurColor);
}


void ScrollScreen(int Count)
{
	ScrBuf.Scroll(Count);
	ScrBuf.FillRect(0,ScrY+1-Count,ScrX,ScrY,' ',FarColorToReal(COL_COMMANDLINEUSERSCREEN));
}


void GetText(int X1,int Y1,int X2,int Y2,void *Dest,int DestSize)
{
	ScrBuf.Read(X1,Y1,X2,Y2,(CHAR_INFO *)Dest,DestSize);
}

#if defined(USE_WFUNC)
void PutTextA(int X1,int Y1,int X2,int Y2,const void *Src)
{
	int Width=X2-X1+1;
	int Y;
	CHAR_INFO *SrcPtr=(CHAR_INFO*)Src;

	for (Y=Y1; Y<=Y2; ++Y,SrcPtr+=Width)
		ScrBuf.WriteA(X1,Y,SrcPtr,Width);
}
#endif

void PutText(int X1,int Y1,int X2,int Y2,const void *Src)
{
	int Width=X2-X1+1;
	int Y;
	CHAR_INFO *SrcPtr=(CHAR_INFO*)Src;

	for (Y=Y1; Y<=Y2; ++Y,SrcPtr+=Width)
		ScrBuf.Write(X1,Y,SrcPtr,Width);
}


void BoxText(WORD Chr)
{
#if defined(USE_WFUNC)

	if (Opt.UseUnicodeConsole)
	{
		WCHAR Str[2];
		Str[0]=Chr;
		Str[1]=0;
		TextW(Str);
	}
	else
#endif
	{
		char Str[2];
		Str[0]=(char)Chr;
		Str[1]=0;
		BoxText(Str);
	}
}


#if defined(USE_WFUNC)
void BoxTextW(WCHAR *Str,int IsVert)
{
	if (IsVert)
		VTextW(Str);
	else
		TextW(Str);
}

void BoxTextW2(const char *Str,int IsVert)
{
	WCHAR TmpStr[2];
	TmpStr[1]=0;

	for (; *Str; ++Str)
	{
		TmpStr[0]=BoxSymbols[((WORD)*Str)-0x0B0];

		if (IsVert)
			VTextW(TmpStr);
		else
			TextW(TmpStr);
	}
}


#endif

void BoxText(char *Str,int IsVert)
{
	if (OutputCP!=437 && OutputCP!=866)
	{
		for (int I=0; Str[I]!=0; I++)
		{
			switch (Str[I])
			{
				case 0x0C7:
				case 0x0B6:
					Str[I]=0x0BA;
					break;
				case 0x0CF:
				case 0x0D1:
					Str[I]=0x0CD;
					break;
				case 0x0C6:
				case 0x0B5:
					Str[I]=0x0B3;
					break;
				case 0x0D0:
				case 0x0D2:
					Str[I]=0x0C4;
					break;
			}
		}
	}

	if (IsVert)
		VText(Str);
	else
		Text(Str);
}

/*
   Отрисовка прямоугольника.
*/
/* $ 23.07.2000 SVS
   Немного оптимизации :-)
*/
void Box(int x1,int y1,int x2,int y2,int Color,int Type)
{
	static char ChrBox[2][6]=
	{
		{0xC4,0xB3,0xDA,0xC0,0xD9,0xBF},
		{0xCD,0xBA,0xC9,0xC8,0xBC,0xBB},
	};

	if (x1>=x2 || y1>=y2)
		return;

	SetColor(Color);
	Type=(Type==SINGLE_BOX || Type==SHORT_SINGLE_BOX)?0:1;
	int _width=x2-x1;
	int _height=y2-y1;
#if defined(USE_WFUNC)

	if (Opt.UseUnicodeConsole)
	{
		WCHAR OutStr[4096];
		_wmemset(OutStr,BoxSymbols[ChrBox[Type][0]-0x0B0],sizeof(OutStr)/sizeof(*OutStr));
		OutStr[_width+1]=0;
		OutStr[0]=BoxSymbols[ChrBox[Type][2]-0x0B0];
		OutStr[_width]=BoxSymbols[ChrBox[Type][5]-0x0B0];
		GotoXY(x1,y1);
		TextW(OutStr);
		//mprintfW(L"%.*s",x2-x1+1,OutStr);
		OutStr[0]=BoxSymbols[ChrBox[Type][3]-0x0B0];
		OutStr[_width]=BoxSymbols[ChrBox[Type][4]-0x0B0];
		GotoXY(x1,y2);
		TextW(OutStr);
		//mprintfW(L"%.*s",x2-x1+1,OutStr);
		_wmemset(OutStr,BoxSymbols[ChrBox[Type][1]-0x0B0],sizeof(OutStr)/sizeof(*OutStr));
		OutStr[_height-1]=0;
		GotoXY(x1,y1+1);
		VTextW(OutStr);
		//vmprintfW(L"%.*s",y2-y1-1,OutStr);
		GotoXY(x2,y1+1);
		VTextW(OutStr);
		//vmprintfW(L"%.*s",y2-y1-1,OutStr);
	}
	else
#endif
	{
		char OutStr[4096];
		memset(OutStr,ChrBox[Type][0],sizeof(OutStr));
		OutStr[_width+1]=0;
		OutStr[0]=ChrBox[Type][2];
		OutStr[_width]=ChrBox[Type][5];
		GotoXY(x1,y1);
		Text(OutStr);
		//mprintf("%.*s",x2-x1+1,OutStr);
		OutStr[0]=ChrBox[Type][3];
		OutStr[_width]=ChrBox[Type][4];
		GotoXY(x1,y2);
		Text(OutStr);
		//mprintf("%.*s",x2-x1+1,OutStr);
		memset(OutStr,ChrBox[Type][1],sizeof(OutStr));
		OutStr[_height-1]=0;
		GotoXY(x1,y1+1);
		VText(OutStr);
		//vmprintf("%.*s",y2-y1-1,OutStr);
		GotoXY(x2,y1+1);
		VText(OutStr);
		//vmprintf("%.*s",y2-y1-1,OutStr);
	}
}
/* SVS $ */

void _PutRealText(HANDLE hConsoleOutput,int X1,int Y1,int X2,int Y2,const void *Src,int BufX,int BufY)
{
	COORD Size,Corner;
	SMALL_RECT Coord;

	if (X2>BufX) X2=BufX;

	if (Y2>BufY) Y2=BufY;

	if (X1>X2) X1=X2;

	if (Y1>Y2) Y1=Y2;

	Size.X=X2-X1+1;
	Size.Y=Y2-Y1+1;
	Corner.X=0;
	Corner.Y=0;
	Coord.Left=X1;
	Coord.Top=Y1;
	Coord.Right=X2;
	Coord.Bottom=Y2;
#if defined(USE_WFUNC)

	if (Opt.UseUnicodeConsole)
		WriteConsoleOutputW(hConsoleOutput,(PCHAR_INFO)Src,Size,Corner,&Coord);
	else
		WriteConsoleOutputA(hConsoleOutput,(PCHAR_INFO)Src,Size,Corner,&Coord);

#else
	WriteConsoleOutput(hConsoleOutput,(PCHAR_INFO)Src,Size,Corner,&Coord);
#endif
}


void PutRealText(int X1,int Y1,int X2,int Y2,const void *Src)
{
	_PutRealText(hConOut,X1,Y1,X2,Y2,Src,ScrX,ScrY);
}

void _GetRealText(HANDLE hConsoleOutput,int X1,int Y1,int X2,int Y2,const void *Dest,int BufX,int BufY)
{
	COORD Size,Corner;
	SMALL_RECT Coord;

	if (X2>BufX) X2=BufX;

	if (Y2>BufY) Y2=BufY;

	if (X1>X2) X1=X2;

	if (Y1>Y2) Y1=Y2;

	Size.X=X2-X1+1;
	Size.Y=Y2-Y1+1;
	Corner.X=0;
	Corner.Y=0;
	Coord.Left=X1;
	Coord.Top=Y1;
	Coord.Right=X2;
	Coord.Bottom=Y2;
#if defined(USE_WFUNC)

	if (Opt.UseUnicodeConsole)
		ReadConsoleOutputW(hConsoleOutput,(PCHAR_INFO)Dest,Size,Corner,&Coord);
	else
		ReadConsoleOutputA(hConsoleOutput,(PCHAR_INFO)Dest,Size,Corner,&Coord);

#else
	ReadConsoleOutput(hConsoleOutput,(PCHAR_INFO)Dest,Size,Corner,&Coord);
#endif
}

void GetRealText(int X1,int Y1,int X2,int Y2,void *Dest)
{
	_GetRealText(hConOut,X1,Y1,X2,Y2,Dest,ScrX,ScrY);
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
#if defined(USE_WFUNC)

	if (Opt.UseUnicodeConsole)
	{
		WCHAR OutStr[4096];

		if (Length > sizeof(OutStr)-3)
			Length=sizeof(OutStr)-3;

		_wmemset(OutStr+1,BoxSymbols[0xB0-0x0B0],Length);
		OutStr[ThumbPos+1]=BoxSymbols[0xB2-0x0B0];
		OutStr[0]=Oem2Unicode[0x1E];
		OutStr[Length+1]=Oem2Unicode[0x1F];
		OutStr[Length+2]=0;
		VTextW(OutStr);
	}
	else
#endif
	{
		char OutStr[4096];

		if (Length > sizeof(OutStr)-3)
			Length=sizeof(OutStr)-3;

		memset(OutStr+1,0xB0,Length);
		OutStr[ThumbPos+1]=0xB2;
		OutStr[0]=0x1E;
		OutStr[Length+1]=0x1F;
		OutStr[Length+2]=0;
		VText(OutStr);
	}
}

void DrawLine(int Length,int Type)
{
	if (Length>1)
	{
#if defined(USE_WFUNC)

		if (Opt.UseUnicodeConsole)
		{
			WCHAR Separator[4096];
			MakeSeparatorW(Length,Separator,Type);

			if ((Type >= 4 && Type <= 7) || (Type >= 10 && Type <= 11))
				VTextW(Separator);
			else
				TextW(Separator);
		}
		else
#endif
		{
			char Separator[4096];
			MakeSeparator(Length,Separator,Type);

			if ((Type >= 4 && Type <= 7) || (Type >= 10 && Type <= 11))
				VText(Separator);
			else
				Text(Separator);
		}
	}
}

// "Нарисовать" сепаратор в памяти.
static BYTE __BoxType[12][8]=
{
//       cp866, 437           cp other!                      h-horiz, s-space, v-vert, b-border, 1-one line, 2-two line
	/* 00 */{0x20,0x20,0xC4,0x00, 0x20,0x20,0xC4,0x00}, // -     h1s
	/* 01 */{0xC7,0xB6,0xC4,0x00, 0xBA,0xBA,0xC4,0x00}, // ||-|| h1b2
	/* 02 */{0xC3,0xB4,0xC4,0x00, 0xC3,0xB4,0xC4,0x00}, // |-|   h1b1
	/* 03 */{0xCC,0xB9,0xCD,0x00, 0xCC,0xB9,0xCD,0x00}, // ||=|| h2b2

	/* 04 */{0x20,0x20,0xB3,0x00, 0x20,0x20,0xB3,0x00}, //  |    v1s
	/* 05 */{0xD1,0xCF,0xB3,0x00, 0xCD,0xCD,0xB3,0x00}, // =|=   v1b2
	/* 06 */{0xC2,0xC1,0xB3,0x00, 0xC2,0xC1,0xB3,0x00}, //       v1b1
	/* 07 */{0xCB,0xCA,0xBA,0x00, 0xCB,0xCA,0xBA,0x00}, //       v2b2

	/* 08 */{0xC4,0xC4,0xC4,0x00, 0xC4,0xC4,0xC4,0x00}, // -     h1
	/* 09 */{0xCD,0xCD,0xCD,0x00, 0xCD,0xCD,0xCD,0x00}, // =     h2
	/* 10 */{0xB3,0xB3,0xB3,0x00, 0xB3,0xB3,0xB3,0x00}, // |     v1
	/* 11 */{0xBA,0xBA,0xBA,0x00, 0xBA,0xBA,0xBA,0x00}, // ||    v2
};

#if defined(USE_WFUNC)
WCHAR* MakeSeparatorW(int Length,WCHAR *DestStr,int Type)
{
	if (Length>1 && DestStr)
	{
		Type%=(sizeof(__BoxType)/sizeof(__BoxType[0]));
		_wmemset(DestStr,BoxSymbols[__BoxType[Type][2]-0x0B0],Length);

		if (Type)
		{
			DestStr[0]=BoxSymbols[__BoxType[Type][0]-0x0B0];
			DestStr[Length-1]=BoxSymbols[__BoxType[Type][1]-0x0B0];
		}

		DestStr[Length]=0x0000;
	}

	return DestStr;
}
#endif

char* MakeSeparator(int Length,char *DestStr,int Type)
{
	if (Length>1 && DestStr)
	{
		int I=0;

		if (OutputCP!=437 && OutputCP!=866)
			I=4;

		Type%=(sizeof(__BoxType)/sizeof(__BoxType[0]));
		memset(DestStr,__BoxType[Type][2+I],Length);

		if (Type)
		{
			DestStr[0]=__BoxType[Type][0+I];
			DestStr[Length-1]=__BoxType[Type][1+I];
		}

		DestStr[Length]=0;
	}

	return DestStr;
}

#if defined(USE_WFUNC)
WORD GetVidCharW(CHAR_INFO CI)
{
//  if(CI.Char.UnicodeChar < 0x100)
//    return CI.Char.UnicodeChar;
	for (int I=0; I < 256; ++I)
		if (CI.Char.UnicodeChar == Oem2Unicode[I])
			return (BYTE)I;

	return 0;
}
#endif

#if defined(USE_WFUNC)
int WINAPI TextToCharInfo(const char *Text,WORD Attr, CHAR_INFO *CharInfo, int Length, DWORD Reserved)
{
	int I;

	if (Opt.UseUnicodeConsole)
	{
		for (I=0; I < Length; I++, ++CharInfo)
		{
			CharInfo->Char.UnicodeChar=Oem2Unicode[RecodeOutTable[(BYTE)Text[I]]];
			CharInfo->Attributes=Attr;
		}
	}
	else
	{
		for (I=0; I < Length; I++, ++CharInfo)
		{
			CharInfo->Char.AsciiChar=RecodeOutTable[(BYTE)Text[I]];
			CharInfo->Attributes=Attr;
		}
	}

	return TRUE;
}
#endif
