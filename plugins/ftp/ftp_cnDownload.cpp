#include <all_far.h>
#pragma hdrstop

#include "ftp_Int.h"

//--------------------------------------------------------------------------------
void Connection::recvrequest(char *cmd, char *local, char *remote, char *mode )
  {  //??SaveConsoleTitle _title;

     recvrequestINT( cmd,local,remote,mode );
     IdleMessage( NULL,0 );
}

void Connection::recvrequestINT(char *cmd, char *local, char *remote, char *mode )
{
  int              oldtype = 0,
                   is_retr;
  FHandle          fout;
  SOCKET           din = INVALID_SOCKET;
  int              ocode, oecode;
  BOOL             oldBrk = FtpSetBreakable( this, -1 );
  FTPCurrentStates oState = CurrentState;
  FTNNotify        ni;

  if ( type == TYPE_A )
    restart_point = 0;

  ni.Upload       = FALSE;
  ni.Starting     = TRUE;
  ni.Success      = TRUE;
  ni.RestartPoint = restart_point;
  ni.Port         = ntohs( portnum );
  ni.Password[0] = 0; //TStrCpy( ni.Password,   UserPassword );
  TStrCpy( ni.User,       UserName );
  TStrCpy( ni.HostName,   hostname );
  TStrCpy( ni.LocalFile,  local );
  TStrCpy( ni.RemoteFile, remote );

  if ( local[0] == '-' && local[1] == 0 ) {
    ;
  } else {
    fout.Handle = Fopen( local, mode, Opt.SetHiddenOnAbort ? FILE_ATTRIBUTE_HIDDEN : FILE_ATTRIBUTE_NORMAL );
    Log(("recv file [%d] \"%s\"=%p",Host.IOBuffSize,local,fout.Handle ));
    if ( !fout.Handle ) {
      ErrorCode = GetLastError();
      Log(( "!Fopen [%s] %s",mode,FIO_ERROR ));
      if ( !ConnectMessage( MErrorOpenFile,local,-MRetry ) )
        ErrorCode = ERROR_CANCELLED;
      //goto abort;
      return;
    }

    if ( restart_point != -1 ) {
      if ( !Fmove( fout.Handle,restart_point ) ) {
        ErrorCode = GetLastError();
        if ( !ConnectMessage( MErrorPosition,local,-MRetry ) )
          ErrorCode = ERROR_CANCELLED;
        return;
      }
    }
    TrafficInfo->Resume( restart_point == -1 ? 0 : restart_point );
  }

  is_retr = StrCmp(cmd,Opt.cmdRetr) == 0;
  if (proxy && is_retr) {
    proxtrans(cmd, local, remote);
    return;
  }

  if ( !initconn() ) {
    Log(("!initconn"));
    return;
  }

  if (!is_retr) {
    if ( type != TYPE_A ) {
      oldtype = type;
      setascii();
    }
  } else
  if ( restart_point ) {
    if ( !ResumeSupport ) {
      AddCmdLine( FMSG(MResumeRestart) );
      restart_point = 0;
    } else
    if ( restart_point != -1 ) {
      if ( command("%s %I64u",Opt.cmdRest,restart_point) != RPL_CONTINUE ) {
        Log(("!restart SIZE"));
        return;
      }
    }
  }

  if ( Host.PassiveMode ) {
    din = dataconn();
    if(din == INVALID_SOCKET) {
      Log(("!dataconn: PASV ent"));
      goto abort;
    }
  }

  if (remote) {
    if (command("%s %s", cmd, remote) != RPL_PRELIM) {
      if (oldtype)
        SetType(oldtype);
      Log(("!command [%s]",cmd));
      fout.Close();
      if ( Fsize(local) )
        DeleteFile( local );
      return;
    }
  } else
  if (command("%s", cmd) != RPL_PRELIM) {
    if (oldtype)
      SetType(oldtype);
    return;
  }

  if( !Host.PassiveMode ) {
    din = dataconn();
    if (din == INVALID_SOCKET) {
      Log(("!dataconn: PASV ret"));
      goto abort;
    }
  }

/**/

  switch (type) {
  case TYPE_A:
  case TYPE_I:
  case TYPE_L: {

      __int64 totalValue = 0;
      TIME_TYPE b,e;
      GET_TIME(b);
      FtpSetBreakable( this, FALSE );
      CurrentState = fcsProcessFile;

      if ( fout.Handle && PluginAvailable(PLUGIN_NOTIFY) )
        FTPNotify().Notify( &ni );

      while( 1 ) {
       //Recv
        int c = nb_recv(&din, IOBuff, Host.IOBuffSize, 0);
        if ( c < 0 ) {
          Log(("gf(%d,%s)=%I64u: !read buff",code,GetSocketErrorSTR(),totalValue ));
          code = RPL_TRANSFERERROR;
          goto NormExit;
        } else
        if ( c == 0 ) {
          Log(("gf(%d,%s)=%I64u: read zero",code,GetSocketErrorSTR(),totalValue ));
          break;
        }
        totalValue += c;

        if ( !fout.Handle ) {
          //Add readed to buffer
          Log(( "AddOutput: +%d bytes", c ));
          AddOutput( (BYTE*)IOBuff,c );
        } else
        if ( Fwrite(fout.Handle,IOBuff,c) != c ) {
        //Write to file
          Log(("!write local"));
          ErrorCode = GetLastError();
          goto abort;
        }

        //Call user CB
        if ( IOCallback ) {
          if ( !TrafficInfo->Callback(c) ) {
            Log(("gf: canceled by CB" ));
            ErrorCode = ERROR_CANCELLED;
            goto abort;
          }
        } else
        //Show Quite progressing
        if ( Opt.ShowIdle && !remote ) {
          char   digit[ 20 ];
          String str;
          GET_TIME( e );
          if ( CMP_TIME(e,b) > 0.5 ) {
            str.printf( "%s%s ", FP_GetMsg(MReaded), FCps(digit,(double)totalValue) );
            SetLastError( ERROR_SUCCESS );
            IdleMessage( str.c_str(),Opt.ProcessColor );
            b = e;
            if ( CheckForEsc(FALSE) ) {
              ErrorCode = ERROR_CANCELLED;
              goto abort;
            }
          }
        }
      }

      if ( IOCallback )
        TrafficInfo->Callback(0);

      break;
    }
  }

NormExit:
  FtpSetBreakable( this, oldBrk );
  ocode              = code;
  oecode             = ErrorCode;
  CurrentState       = oState;

  scClose( data_peer,-1 );

  if ( getreply(FALSE) == RPL_ERROR ||
       oldtype && !SetType(oldtype) ) {
    lostpeer();
  } else {
    code      = ocode;
    ErrorCode = oecode;
  }

  if ( fout.Handle && PluginAvailable(PLUGIN_NOTIFY) ) {
    ni.Starting = FALSE;
    ni.Success  = TRUE;
    FTPNotify().Notify( &ni );
  }
 return;

abort:
  FtpSetBreakable( this, oldBrk );
  if ( !cpend ) {
    Log(( "!!!cpend" ));
  }

  ocode        = code;
  oecode       = ErrorCode;
  CurrentState = oState;

  if ( !SendAbort(din) ||
       (oldtype && !SetType(oldtype)) )
    lostpeer();
   else {
    code      = ocode;
    ErrorCode = oecode;
  }

  scClose( data_peer,-1 );

  if ( fout.Handle && PluginAvailable(PLUGIN_NOTIFY) ) {
    ni.Starting = FALSE;
    ni.Success  = FALSE;
    FTPNotify().Notify( &ni );
  }

 return;
}

/* abort using RFC959 recommended ffIP,SYNC sequence  */
/*
  WarFTP
  |->‚ABOR
  |<-226 Transfer complete. 39600000 bytes in 18.84 sec. (2052.974 Kb/s)
  |<-225 ABOR command successful.

  FreeBSD, SunOS
  |->ÚABOR
  |<-426 Transfer aborted. Data connection closed.
  |<-226 Abort successful

  QNX
  |->‚ABOR
  |<-226 Transfer complete.
  |<-225 ABOR command successful.

  IIS
  |->‚ABOR
  |<-225 ABOR command successful.

  QNX
  |->ÚABOR
  |<-452 Error writing file: No child processes.
  |<-225 ABOR command successful.

  GuildFTP (have no ABOR notify at all)
  -> <ABORT>
  <- 226 Transfer complete. 11200000 bytes in 2 sec. (5600.00 Kb/s).

  ??
  ->ÚABOR
  <-450 Transfer aborted. Link to file server lost.
  <-500 ˇÙˇÚABOR not understood
*/
BOOL Connection::SendAbort( SOCKET din )
  {
     do{
       if ( !fprintfSocket(cout,"%c%c",ffIAC,ffIP) ) {
         Log(( "!send ffIAC" ));
         break;
       }

       char msg = ffIAC;
     /* send ffIAC in urgent mode instead of DM because UNIX places oob mark */
     /* after urgent byte rather than before as now is protocol            */
       if (nb_send(&cout,&msg,1,MSG_OOB) != 1) {
         Log(("rr: !send urgent" ));
         break;
       }

       if ( !fprintfSocket(cout,"%cABOR\r\n",ffDM) ) {
         Log(( "!send ABOR" ));
         break;
       }

       scClose( data_peer,-1 );
       int res;

       Log(( "Wait ABOT reply" ));
       res = getreply( FALSE );
       if ( res == RPL_TIMEOUT ) {
         Log(( "Error waiting first ABOR reply" ));
         return FALSE;
       }

       //Single line
       if ( code == 225 ) {
         Log(( "Single line ABOR reply" ));
         return TRUE;
       } else
       //Wait OPTIONAL second line
       if ( code == 226 ) {
         Log(( "Wait OPT second reply" ));
         do{
           res = getreply( FALSE,500 );
           if ( res == RPL_TIMEOUT ) {
             Log(( "Timeout: res: %d, code: %d", res, code ));
             return TRUE;
           } else
           if ( res == RPL_ERROR ) {
             Log(( "Error: res: %d, code: %d", res, code ));
             return FALSE;
           }
           Log(( "Result: res: %d, code: %d", res, code ));
         }while(1);
       } else {
       //Wait second line
         Log(( "Wait second reply" ));

         res = getreply( FALSE );
         if ( res == RPL_TIMEOUT ) {
           Log(( "Error waiting second ABOR reply" ));
           return FALSE;
         }
         Log(( "Second reply: res: %d, code: %d", res,code ));
         return TRUE;
       }

     }while(0); /*Error sending ABOR*/
  return FALSE;
}
