#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

static int    nSaveCount = 0;
static HANDLE hScreen   = NULL;
static char   SaveTitle[FAR_MAX_TITLE];

int DECLSPEC FP_Screen::isSaved( void )
  {
 return nSaveCount;
}

void DECLSPEC FP_Screen::Save( void )
  { CHK_INITED

    Log(( "SCREEN: SAVE[%d]",nSaveCount ));
    if ( IS_FLAG(FP_LastOpMode,OPM_FIND) ) return;

    if (nSaveCount == 0) {
      hScreen = FP_Info->SaveScreen(0,0,-1,-1);
      GetConsoleTitle(SaveTitle,sizeof(SaveTitle));
    }

    nSaveCount++;
}

void DECLSPEC FP_Screen::FullRestore( void )
  {
    while( isSaved() ) Restore();
}

void DECLSPEC FP_Screen::RestoreWithoutNotes( void )
  {
     if ( hScreen ) {
       FP_Info->RestoreScreen( hScreen );
       hScreen = FP_Info->SaveScreen( 0,0,-1,-1 );
     }
}

void DECLSPEC FP_Screen::Restore( void )
  {  CHK_INITED

     if (!hScreen || !nSaveCount) return;

     nSaveCount--;
     if (nSaveCount == 0) {
       Log(( "SCREEN: RESTORE" ));
       SetConsoleTitle(SaveTitle);
       FP_Info->RestoreScreen(hScreen);
       FP_Info->Text(0,0,0,NULL);
       hScreen=NULL;
     }
}
