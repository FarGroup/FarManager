#include <all_far.h>
#pragma hdrstop

#include "ftp_Int.h"

LOCALSTRUCT( DeleteData )
  BOOL        DeleteAllFolders;
  BOOL        SkipAll;
  int         OpMode;
  int         ShowHosts;
  Connection *hConnect;
};

BOOL idDeleteCB( PPluginPanelItem p,LPVOID dt )
  {  PROC(( "idDeleteCB", "%p,%p [%s]", p, dt, FTP_FILENAME(p) ));
     int      rres;
     PFTPHost h       = FTPHost::Convert(p);
     char    *CurName = FTP_FILENAME( p );
     char    *m;

     if ( ((PDeleteData)dt)->ShowHosts && !h )
       return TRUE;

//Ask
     if ( IS_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY) ) {
       if ( !((PDeleteData)dt)->DeleteAllFolders &&
            IS_SILENT( ((PDeleteData)dt)->OpMode ) ) {
         CONSTSTR MsgItems[]={
           ((PDeleteData)dt)->ShowHosts ? FP_GetMsg(MDeleteHostsTitle):FP_GetMsg(MDeleteTitle),
           ((PDeleteData)dt)->ShowHosts ? FP_GetMsg(MDeleteHostFolder):FP_GetMsg(MDeleteFolder),
           CurName,
           FP_GetMsg(MDeleteGroupDelete), FP_GetMsg(MDeleteGroupAll), FP_GetMsg(MDeleteGroupCancel)};

         rres = FMessage( FMSG_WARNING|FMSG_DOWN, NULL,
                          MsgItems, ARRAY_SIZE(MsgItems),
                          3 );

         switch(rres) {
            /*ESC*/
           case -1: return FALSE;

           /*Del*/
           case  0: break;

           /*DelAll*/
           case  1: ((PDeleteData)dt)->DeleteAllFolders = TRUE;
                 break;

           /*Cancel*/
           case  2: return FALSE;

         }
       }
     }

//Display
     SaveConsoleTitle::Text( CurName );
     FtpConnectMessage( ((PDeleteData)dt)->hConnect, MDeleteTitle, PointToName(CurName) );

//===============================
//Hosts
      if ( ((PDeleteData)dt)->ShowHosts )
        return FP_DeleteRegKeyAll(h->RegKey);

//===============================
//FTP
      m = strrchr( CurName,'/' );
      if (m) CurName = m+1;

      if ( IS_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY) ) {
//FTP directory
        //Succ delete
        if ( FtpRemoveDirectory( ((PDeleteData)dt)->hConnect, CurName ) ) {
          if (Opt.UpdateDescriptions) p->Flags |= PPIF_PROCESSDESCR;
          return TRUE;
        }
      } else {
//FTP file
        //Succ delete
        if ( FtpDeleteFile( ((PDeleteData)dt)->hConnect,CurName ) ) {
          if (Opt.UpdateDescriptions) p->Flags |= PPIF_PROCESSDESCR;
          return TRUE;
        }
      }

//Error
      Log(( "Del error: %s", ((PDeleteData)dt)->SkipAll ? "SkipAll" : "Ask" ));
      if ( ((PDeleteData)dt)->SkipAll == FALSE ) {
        rres = FtpConnectMessage( ((PDeleteData)dt)->hConnect, MCannotDelete, CurName, -MCopySkip, MCopySkipAll );
        switch( rres ) {
          /*skip*/     case 0: Log(( "Skip" ));
                          return TRUE;

          /*skip all*/ case 1: ((PDeleteData)dt)->SkipAll = TRUE;
                               Log(( "SkipAll" ));
                          return TRUE;

                      default: Log(( "Other" ));
                               SetLastError( ERROR_CANCELLED );
                          return FALSE;
        }
      } else
        return TRUE;
}

//---------------------------------------------------------------------------------
int FTP::DeleteFilesINT(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
  {  PROC(( "DeleteFilesINT", NULL ))

     if ( !hConnect && !ShowHosts) return FALSE;
     if ( ItemsNumber == 0 )       return FALSE;

   //Ask
     if ( !IS_SILENT(OpMode) ) {
       CONSTSTR MsgItems[]={
         FMSG( ShowHosts ? MDeleteHostsTitle : MDeleteTitle ),
         FMSG( ShowHosts ? MDeleteHosts      : MDeleteFiles ),
         FMSG( MDeleteDelete ),
         FMSG( MDeleteCancel ) };

         int rres = FMessage( 0, NULL, MsgItems, ARRAY_SIZE(MsgItems), 2 );
         if ( rres != 0 ) return TRUE;

         if ( ItemsNumber > 1 ) {
           String Msg;
           Msg.printf( ShowHosts ? FP_GetMsg(MDeleteNumberOfHosts) : FP_GetMsg(MDeleteNumberOfFiles) ,
                       ItemsNumber );
           MsgItems[1] = Msg.c_str();
           rres = FMessage( FMSG_WARNING|FMSG_DOWN,NULL,MsgItems,ARRAY_SIZE(MsgItems),2 );
           if ( rres != 0 ) return TRUE;
         }
     }

   //LIST
     DeleteData data;

     data.DeleteAllFolders = FALSE;
     data.SkipAll          = FALSE;
     data.OpMode           = OpMode;
     data.ShowHosts        = ShowHosts;
     data.hConnect         = hConnect;

 return ExpandList( PanelItem, ItemsNumber, NULL, TRUE, idDeleteCB, &data );
}

//---------------------------------------------------------------------------------
int FTP::DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
  {  PROC(( "DeleteFiles", NULL ))
     FP_Screen _scr;

     int res = DeleteFilesINT( PanelItem,ItemsNumber,OpMode );

     if ( res ) {
       LongBeepEnd( TRUE );
       return TRUE;
     }

     res = GetLastError();
     if ( res == ERROR_CANCELLED )
       return TRUE;

     LongBeepEnd( TRUE );

     //Show FAR error
     if ( ShowHosts )
       return FALSE;

     //Show self error message
     CONSTSTR nm;
     if ( ItemsNumber == 1 )
       nm = FTP_FILENAME( &PanelItem[0] );
      else
       nm = Message( "%d files/folders", ItemsNumber );

     FtpConnectMessage( hConnect, MCannotDelete, nm, -MOk );

 return TRUE;
}
