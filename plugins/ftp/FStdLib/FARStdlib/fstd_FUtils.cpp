#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

//------------------------------------------------------------------------
SaveConsoleTitle::SaveConsoleTitle( BOOL rest /*= TRUE*/ )
  {
    Usage         = rest;
    NeedToRestore = rest;
    GET_TIME( LastChange );
    GetConsoleTitle(SaveTitle,sizeof(SaveTitle));
    Log(( "TITLE: Save" ));
}
SaveConsoleTitle::~SaveConsoleTitle()
  {
    Restore();
}

void SaveConsoleTitle::Set( CONSTSTR buff )
  {
     GET_TIME( LastChange );
     Text( buff );
     NeedToRestore = TRUE;
}

void SaveConsoleTitle::Text( CONSTSTR buff )
  {  char _buff[ 1024 ];

     Log(( "TITLE: Set [%s]", buff ));
     if ( FP_WinVer->dwPlatformId != VER_PLATFORM_WIN32_NT ) {
       OemToCharBuff( buff, _buff, sizeof(_buff) );
       buff = _buff;
     }
     SetConsoleTitle(buff);
}

void SaveConsoleTitle::Restore( void )
  {
}

void SaveConsoleTitle::Using( void )
  {
    Usage++;
    Log(( "TITLE: Use[%d]",Usage ));
}

CMP_TIME_TYPE SaveConsoleTitle::Changed( void )
  {  TIME_TYPE t;
     GET_TIME( t );
 return CMP_TIME(t,LastChange);
}

//------------------------------------------------------------------------
int DECLSPEC CheckForKeyPressed( WORD *Codes,int NumCodes )
  {  static HANDLE hConInp = CreateFile("CONIN$", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
     INPUT_RECORD  rec;
     DWORD         ReadCount;
     int           rc = 0;
     int           n;

     while (1) {
       PeekConsoleInput(hConInp,&rec,1,&ReadCount);
       if (ReadCount==0) break;

       ReadConsoleInput(hConInp,&rec,1,&ReadCount);

       for( n = 0; n < NumCodes; n++ )
         if ( rec.EventType == KEY_EVENT &&
              rec.Event.KeyEvent.bKeyDown &&
              rec.Event.KeyEvent.wVirtualKeyCode == Codes[n] )
           rc = n+1;
     }

 return rc;
}
//------------------------------------------------------------------------
int DECLSPEC FP_Color( int tp )
  {
 return FP_Info->AdvControl( FP_Info->ModuleNumber, ACTL_GETCOLOR, (void*)tp );
}
