#ifndef __FTP_PROGRESS_INTERNAL
#define __FTP_PROGRESS_INTERNAL

#include "fstdlib.h"         //FAR plugin stdlib
#include "../ftp_Plugin.h"

#define MAX_TRAF_LINES    20
#define MAX_TRAF_WIDTH    200
#define MAX_TRAF_ITEMS    50

enum tAlignment {
  tNone,
  tLeft,
  tRight,
  tCenter,
  tRightFill
};

//One element drawed inside dialog lines array
STRUCT( InfoItem )
  int  Type;   //Type of element (
  int  Line;   //Line number (Y)
  int  Pos;    //Starting position in line (X)
  int  Size;   //Width of element (not for all alignment)
  int  Align;  //Element alignment (tAlignment)
  char Fill;   //Filler character
};

STRUCTBASE( TrafficInformation, public ProgressInterface )
  HANDLE      hConnect;
  char        ConsoleTitle[FAR_MAX_TITLE];
  char        Lines[MAX_TRAF_LINES][MAX_TRAF_WIDTH+1];
  int         LineCount;

  char        SrcFileName[FAR_MAX_PATHSIZE];
  char        DestFileName[FAR_MAX_PATHSIZE];
  __int64     FileSize;
  __int64     StartFileSize;
  __int64     FullFileSize;
  time_t      FileStartTime;
  time_t      FileWaitTime;
  double      Cps;
  double      AvCps[3];

  __int64     TotalFiles;
  __int64     TotalComplete;
  __int64     TotalSkipped;

  __int64     TotalSize;
  __int64     TotalStartSize;
  __int64     TotalFullSize;
  time_t      TotalStartTime;
  time_t      TotalWaitTime;
  double      TotalCps;

  int         TitleMsg;
  BOOL        ShowStatus;
  TIME_TYPE   LastTime;
  __int64     LastSize;

  InfoItem    Items[ MAX_TRAF_ITEMS ];
  int         Count;

  // ------------- INTERNAL
//Format and draw lines
  void FormatLine( int num,CONSTSTR line,time_t tm );
  void DrawInfo( PInfoItem it,time_t tm );
  void DrawInfos( time_t tm );

//Current infos
  __int64 CurrentSz( void )         { return FileSize + StartFileSize; }
  __int64 CurrentRemain( void )     { return FullFileSize - CurrentSz(); }
  __int64 CurrentDoRemain( void )   { return FullFileSize - StartFileSize; }
  __int64 TotalSz( void )           { return TotalSize + TotalStartSize + CurrentSz(); }
  __int64 TotalRemain( void )       { return TotalFullSize - TotalSz(); }
  __int64 TotalDoRemain( void )     { return TotalFullSize - TotalStartSize; }

  // ------------- PUBLICS
//Resume
  void Resume( CONSTSTR LocalFileName );
  void Resume( __int64 size );

//Called for every copyed portion
  BOOL Callback( int Size );

//Start using traffic (start of whole operation)
  void Init( HANDLE h,int tMsg,int OpMode,PFP_SizeItemList il );

//Start of every file
  void InitFile( PluginPanelItem *pi, CONSTSTR SrcName, CONSTSTR DestName );
  void InitFile( __int64 sz,            CONSTSTR SrcName, CONSTSTR DestName );

//Skip last part of current file
  void Skip( void );

//Inserts pause to values (f.e. while plugin wait Y/N dialog input)
  void Waiting( time_t paused );

//Attach to specified connection
  void SetConnection( HANDLE Connection)  { hConnect = Connection; }
};

CONSTSTR FCps4( char *buff,double val );
void     PPercent( char *str,int x,int x1,int percent );        // Draws a percent gouge.
double   ToPercent( __int64 Value,__int64 ValueLimit );             // Calculate float percent.
void     StrYTime( char *str,struct tm *tm );
void     StrTTime( char *str,struct tm *tm );

#endif