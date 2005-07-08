#include <all_far.h>
#pragma hdrstop

#include "ftp_Int.h"

//--------------------------------------------------------------------------------
void Connection::sendrequest(char *cmd, char *local, char *remote )
  {  //??SaveConsoleTitle _title;

     sendrequestINT( cmd,local,remote );
     IdleMessage( NULL,0 );
}

void Connection::sendrequestINT(char *cmd, char *local, char *remote )
  {  PROC(( "sendrequestINT","%s,%s,%s",cmd,local,remote ))

  if ( type == TYPE_A )
    restart_point=0;

  WIN32_FIND_DATA   ffi;
  FHandle           fin;
  SOCKET            dout = 0;
  LONG              hi;
  FTPCurrentStates  oState = CurrentState;
  BOOL              oldBrk = FtpSetBreakable( this, -1 );
  __int64           fsz;
  FTNNotify         ni;

  ni.Upload       = TRUE;
  ni.Starting     = TRUE;
  ni.Success      = TRUE;
  ni.RestartPoint = restart_point;
  ni.Port         = ntohs( portnum );
  ni.Password[0] = 0; //TStrCpy( ni.Password,   UserPassword );
  TStrCpy( ni.User,       UserName );
  TStrCpy( ni.HostName,   hostname );
  TStrCpy( ni.LocalFile,  local );
  TStrCpy( ni.RemoteFile, remote );

  if (proxy) {
    proxtrans(cmd, local, remote);
    return;
  }

  HANDLE ff = FindFirstFile(local,&ffi);
  if ( ff == INVALID_HANDLE_VALUE ) {
    ErrorCode = ERROR_OPEN_FAILED;
    return;
  }
  FindClose(ff);

  if ( IS_FLAG(ffi.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY) ) {
    ErrorCode = ERROR_OPEN_FAILED;
    Log(( "local is directory [%s]",local ));
    if ( !ConnectMessage( MNotPlainFile,local,-MRetry ) )
      ErrorCode = ERROR_CANCELLED;
    return;
  }

  fin.Handle = Fopen( local,"r" );
  if ( !fin.Handle ) {
    Log(( "!open local" ));
    ErrorCode = ERROR_OPEN_FAILED;
    if ( !ConnectMessage( MErrorOpenFile,local,-MRetry ) )
      ErrorCode = ERROR_CANCELLED;
    return;
  }
  fsz = Fsize(fin.Handle);

  if ( restart_point && fsz == restart_point) {
    AddCmdLine( FMSG(MFileComplete) );
    ErrorCode = ERROR_SUCCESS;
    return;
  }

  if ( !initconn() ) {
    Log(( "!initconn" ));
    return;
  }

  if ( Host.SendAllo ) {
    if ( cmd[0] != Opt.cmdAppe[0] ) {
      __int64 v = ((__int64)ffi.nFileSizeHigh) * MAXDWORD + ffi.nFileSizeLow;
      Log(( "ALLO %I64u", v ));
      if ( command("%s %I64u",Opt.cmdAllo,v) != RPL_COMPLETE ) {
        Log(( "!allo" ));
        return;
      }
    }
  }

  if ( restart_point ) {
    if ( !ResumeSupport && cmd[0] != Opt.cmdAppe[0] ) {
      AddCmdLine( FMSG(MResumeRestart) );
      restart_point = 0;
    } else {
      Log(( "restart_point %I64u",restart_point ));

      if ( !Fmove( fin.Handle, restart_point ) ) {
        ErrorCode = GetLastError();
        Log(( "!setfilepointer(%I64u)",restart_point ));
        if ( !ConnectMessage( MErrorPosition,local,-MRetry ) )
          ErrorCode = ERROR_CANCELLED;
        return;
      }

      if ( cmd[0] != Opt.cmdAppe[0] &&
           command("%s %I64u",Opt.cmdRest,restart_point) != RPL_CONTINUE )
        return;

      TrafficInfo->Resume( restart_point );
    }
  }

  if( Host.PassiveMode ) {
    Log(( "pasv" ));
    dout = dataconn();
    if ( dout == INVALID_SOCKET )
      goto abort;
  }

  if (remote) {
    Log(( "Upload remote [%s]",remote ));
    if (command("%s %s", cmd, remote) != RPL_PRELIM) {
      return;
    }
  } else {
    Log(( "!remote" ));
    if (command("%s", cmd) != RPL_PRELIM) {
      return;
    }
  }

  if( !Host.PassiveMode ) {
    dout = dataconn();
    if ( dout == INVALID_SOCKET ) goto abort;
  }

  switch (type) {
    case TYPE_I:
    case TYPE_L:
    case TYPE_A: if ( fsz != 0 ) {

                  if ( PluginAvailable(PLUGIN_NOTIFY) ) FTPNotify().Notify( &ni );

                  Log(( "Uploading %s->%s from %I64u",local,remote,restart_point ));

                  FTPConnectionBreakable _brk( this,FALSE );
                  CurrentState = fcsProcessFile;
                  //-------- READ
                  while( 1 ) {

                    hi = 0;
           Log(( "read %d",Host.IOBuffSize ));
                    if ( !ReadFile(fin.Handle,IOBuff,Host.IOBuffSize,(LPDWORD)&hi,NULL) ) {
                      Log(("pf: !read buff" ));
                      ErrorCode = GetLastError();
                      goto abort;
                    }

                    if ( hi == 0 ) {
                      ErrorCode = GetLastError();
                      break;
                    }

                    //-------- SEND
                    LONG  d;
                    char *bufp;

           Log(( "doSend" ));
                    for ( bufp = IOBuff; hi > 0; hi -= d, bufp += d) {

           Log(( "ndsend %d",hi ));
                      if ( (d=(LONG)nb_send(&dout, bufp,(int)hi, 0)) <= 0 ) {
                        Log(("pf(%d,%s): !send %d!=%d",code,GetSocketErrorSTR(),d,hi ));
                        code = RPL_TRANSFERERROR;
                        goto abort;
                      }
           Log(( "sent %d",d ));

                      if (IOCallback && !TrafficInfo->Callback( (int)d ) ) {
                        Log(("pf(%d,%s): canceled",code,GetSocketErrorSTR() ));
                        ErrorCode = ERROR_CANCELLED;
                        goto abort;
                      }
                    }//-- SEND
           Log(( "sended" ));

                    //Sleep(1);

                  }//-- READ
                  if ( IOCallback ) TrafficInfo->Callback(0);
           Log(( "done" ));

                 } /*fsz != 0*/
               break;
  }/*SWITCH*/

//NormExit
  FtpSetBreakable( this, oldBrk );
  CurrentState = oState;
  if(data_peer != INVALID_SOCKET) {
    scClose( data_peer,1 );

    if ( getreply(0) > RPL_COMPLETE ) {
      ErrorCode = ERROR_WRITE_FAULT;
    }
  } else
    getreply(0);

  if ( PluginAvailable(PLUGIN_NOTIFY) ) {
    ni.Starting = FALSE;
    ni.Success  = TRUE;
    FTPNotify().Notify( &ni );
  }
 return;

abort:
  FtpSetBreakable( this, oldBrk );
  CurrentState = oState;

  if ( !cpend ) {
    Log(( "!!!cpend" ));
  }

 int ocode = code,
      oecode = ErrorCode;

  scClose( data_peer, SD_SEND );

  if ( !SendAbort(data_peer) ) {
    Log(( "!send abort" ));
    lostpeer();
  } else {
    setascii();
    ProcessCommand( "pwd" );
    code      = ocode;
    ErrorCode = oecode;
  }

  if ( PluginAvailable(PLUGIN_NOTIFY) ) {
    ni.Starting = FALSE;
    ni.Success  = FALSE;
    FTPNotify().Notify( &ni );
  }
}
