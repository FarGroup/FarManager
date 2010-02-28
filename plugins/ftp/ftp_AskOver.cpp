#include <all_far.h>
#pragma hdrstop

#include "ftp_Int.h"

void MkFileInfo( char *buff,int bsz,CONSTSTR title,LPFAR_FIND_DATA p )
  {  char       str[ 100 ];
     SYSTEMTIME tm;

    //Text
    SNprintf( buff,bsz,"%-20s",FP_GetMsg(title) );

    if ( p ) {
      //Size
      FDigit( str,((__int64)p->nFileSizeHigh) << 32 | p->nFileSizeLow,14 );
      StrCat( buff,str,bsz );

      //Time
      FileTimeToSystemTime( &p->ftLastWriteTime,&tm );
      SNprintf( str,sizeof(str)," %02d.%02d.%04d %02d:%02d:%02d",
                tm.wDay, tm.wMonth, tm.wYear, tm.wHour, tm.wMinute, tm.wSecond );
      StrCat( buff,str,bsz );
    }
}

static BOOL DlgResume;
LONG_PTR WINAPI idDlgProc( HANDLE hDlg, int Msg, int id, LONG_PTR key )
  {  FP_Dialog d( hDlg );

     if ( Msg == DN_KEY && (id >= 8 && id <= 14) )
       switch( key ) {
         case  KEY_LEFT: if ( (id > 8 && id <= 11) ||
                              (id > 12 && id <= 14) )
                           d.Focused( id - 1 - (!DlgResume && id==11) );

                      return TRUE;
         case KEY_RIGHT: if ( (id >= 8 && id < 11) ||
                              (id >= 12 && id < 14) )
                           d.Focused( id + 1 + (!DlgResume && id==9) );
                      return TRUE;
         case    KEY_UP:
         case  KEY_DOWN: if ( id >= 12 )
                           d.Focused( id - 4 );
                          else
                         if ( id < 11 )
                           d.Focused( id + 4 );
                      return TRUE;
       }

 return FP_Info->DefDlgProc( hDlg,Msg,id,key );
}

overCode FTP::AskOverwrite( int title, BOOL Download,LPFAR_FIND_DATA dest,LPFAR_FIND_DATA src,overCode last,bool haveTimes )
  {
    if ( !hConnect )
      return ocCancel;

static FP_DECL_DIALOG( InitItems )
   /*00*/    FDI_CONTROL( DI_DOUBLEBOX, 3, 1,66,12, 0, NULL )

   /*01*/      FDI_LABEL( 5, 2,    FMSG(MAlreadyExist) )
   /*02*/      FDI_LABEL( 6, 3,    NULL )
   /*03*/      FDI_LABEL( 5, 4,    FMSG(MAskOverwrite) )

   /*04*/      FDI_HLINE( 3, 5 )

   /*05*/      FDI_LABEL( 5, 6,    NULL )
   /*06*/      FDI_LABEL( 5, 7,    NULL )

   /*07*/      FDI_HLINE( 3, 8 )

   /*08*/      FDI_CHECK( 5, 9,    FMSG(MBtnRemember) )

   /*09*/      FDI_HLINE( 3, 10 )

   /*10*/ FDI_GDEFBUTTON( 5,11,    FMSG(MBtnOverwrite) )
   /*11*/    FDI_GBUTTON(21,11,    FMSG(MBtnCopySkip) )
   /*12*/    FDI_GBUTTON(37,11,    FMSG(MBtnCopyResume) )
   /*13*/    FDI_GBUTTON(21,11,    FMSG(MBtnCopyNewer) )
   /*14*/    FDI_GBUTTON(37,11,    FMSG(MBtnCopyCancel) )
FP_END_DIALOG

     FarDialogItem  DialogItems[ FP_DIALOG_SIZE(InitItems) ];

     if ( last == ocOverAll || last == ocSkipAll || last == ocNewerAll)
       return last;

     if ( hConnect->ResumeSupport && last == ocResumeAll )
         return last;

//Set values
     //Title
     InitItems[ 0].Text = FMSG(title);

//Create items
     FP_InitDialogItems( InitItems,DialogItems );

//Set flags
     //File name
     StrCpy( DialogItems[2].Data, dest->cFileName, sizeof(DialogItems[0].Data) );
     DialogItems[2].Data[60] = 0;

     //Gray resume
     if ( Download && !hConnect->ResumeSupport ) {
       SET_FLAG( DialogItems[12].Flags,DIF_DISABLE );
     }

     if ( !haveTimes ) {
       SET_FLAG( DialogItems[13].Flags,DIF_DISABLE );
     }
     //Info
     MkFileInfo( DialogItems[5].Data, sizeof(DialogItems[0].Data), FMSG(MBtnCopyNew),      src );
     MkFileInfo( DialogItems[6].Data, sizeof(DialogItems[0].Data), FMSG(MBtnCopyExisting), dest );

//Dialog
     DlgResume = !Download || hConnect->ResumeSupport;
     int rc = FDialogEx( 70,14,NULL,DialogItems,FP_DIALOG_SIZE(InitItems),FDLG_WARNING,idDlgProc );
     int remember = DialogItems[8].Selected;

     if ( LongBeep )
       FP_PeriodReset( LongBeep );

     switch( rc ) {
       case 10: return (remember?ocOverAll:ocOver);
       case 11: return (remember?ocSkipAll:ocSkip);
       case 12: return (remember?ocResumeAll:ocResume);
       case 13: return (remember?ocNewerAll:ocNewer);
       default: return ocCancel;
     }
}
