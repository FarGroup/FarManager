#include <all_far.h>
#pragma hdrstop

#include "ftp_Int.h"

//------------------------------------------------------------------------
FTPCopyInfo::FTPCopyInfo( void )
  {
     asciiMode       = FALSE;
     ShowProcessList = FALSE;
     AddToQueque     = FALSE;
     MsgCode         = ocNone;
     Download        = FALSE;
     UploadLowCase   = FALSE;
     FTPRename       = FALSE;
}
//------------------------------------------------------------------------
void AddWait( time_t tm )
  {
   for ( int n = 0; n < 3; n++ )
    if ( FTPPanels[n] && FTPPanels[n]->hConnect &&
         FTPPanels[n]->hConnect->IOCallback )
      FTPPanels[n]->hConnect->TrafficInfo->Waiting(tm);
}

//------------------------------------------------------------------------
BOOL DECLSPEC IsAbsolutePath( CONSTSTR path )
  {
  return path[0] && path[1] && path[2] &&
         path[1] == ':' && path[2] == SLASH_CHAR;
}
//------------------------------------------------------------------------
const char quotes[] = " \"%,;[]";

void DECLSPEC QuoteStr( char *str )
  {  char buff[ 1024 ],*m,*src;

    if ( StrChr(str,quotes) == NULL )
      return;

    m = buff;
    src = str;
    *m++ = '\"';
    for( int n = 0; n < sizeof(buff)-3 && *src; n++ )
      if ( *src == '\"' ) {
        *m++ = '\"';
        *m++ = '\"';
        n++;
        src++;
      } else
        *m++ = *src++;

    *m++ = '\"';
    *m = 0;
    strcpy( str,buff );
}

void DECLSPEC QuoteStr( String& str )
  {  String  buff;

    if ( str.Chr(quotes) == -1 )
      return;

    buff.Add( '\"' );
    for( int n = 0; n < str.Length(); n++ )
      if ( str[n] == '\"' ) {
        buff.Add( '\"' );
        buff.Add( '\"' );
      } else
        buff.Add( str[n] );

    buff.Add( '\"' );
    str = buff;
}

//------------------------------------------------------------------------
#define SIZE_M 1024*1024
#define SIZE_K 1024

void DECLSPEC Size2Str( char *buff,DWORD sz )
  {  char   letter = 0;
     double size = (double)sz;
     int    rc;

     if ( size >= SIZE_M ) {
       size /= SIZE_M;
       letter = 'M';
     } else
     if ( size >= SIZE_K ) {
       size /= SIZE_K;
       letter = 'K';
     }

     if ( !letter ) {
       Sprintf( buff,"%d",sz );
       return;
     }

     Sprintf( buff,"%f",size );
     rc = strlen(buff);
     if ( !rc || strchr(buff,'.') == NULL )
       return;

     for ( rc--; rc && buff[rc] == '0'; rc-- );
     if ( buff[rc] != '.' )
       rc++;
     buff[rc]   = letter;
     buff[rc+1] = 0;
}

DWORD DECLSPEC Str2Size( char *str )
  {  int    rc = strlen( str );
     double sz;
     char   letter;

     if ( !rc )
       return 0;

     rc--;
     if ( str[rc] == 'k' || str[rc] == 'K' )
       letter = 'K';
      else
     if ( str[rc] == 'm' || str[rc] == 'M' )
       letter = 'M';
      else
       letter = 0;

     if ( letter )
       str[rc] = 0;

     sz = atof( str );

     if ( letter == 'K' ) sz *= SIZE_K; else
     if ( letter == 'M' ) sz *= SIZE_M;

 return (DWORD)sz;
}
//------------------------------------------------------------------------
int DECLSPEC StrSlashCount( CONSTSTR m )
  {  int cn = 0;

    if ( m )
      for( ; *m; m++ )
        if ( *m == '/' || *m == '\\' )
           cn++;

 return cn;
}
//------------------------------------------------------------------------
BOOL DECLSPEC FTestOpen( CONSTSTR nm )
  {  HANDLE f;
     BOOL   rc;

     f = CreateFile( nm, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
     rc = f &&
          f != INVALID_HANDLE_VALUE &&
          GetFileType(f) == FILE_TYPE_DISK;

     CloseHandle( f );

 return rc;
}

BOOL DECLSPEC FRealFile( CONSTSTR nm,LPFAR_FIND_DATA fd )
  {  HANDLE f;
     BOOL   rc;

     f = CreateFile( nm, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
     rc = f &&
          f != INVALID_HANDLE_VALUE &&
          GetFileType(f) == FILE_TYPE_DISK;

     if ( rc && fd ) {
       StrCpy( fd->cFileName, nm );
       fd->dwFileAttributes = GetFileAttributes(nm);
       fd->nFileSizeLow     = GetFileSize( f, &fd->nFileSizeHigh );
       GetFileTime( f, &fd->ftCreationTime, &fd->ftLastAccessTime, &fd->ftLastWriteTime );
     }

     CloseHandle( f );

 return rc;
}

//------------------------------------------------------------------------
BOOL DECLSPEC DoCreateDirectory( char *nm )
  {  char *m;
     char  ch;
     UINT  tp;

//Skip UNC abs path
    if ( StrCmp( nm, "\\\\?\\",4 ) == 0 )
      nm += 4;

//Skip UNC share name
    if ( nm[0] == SLASH_CHAR && nm[1] == SLASH_CHAR ) {
      m = StrChr( nm+2,SLASH_CHAR );
      if ( !m ) {
        Log(( "UNC does not contains resource name" ));
        return FALSE;
      }
      m = StrChr( m+1,SLASH_CHAR );
      if ( !m )
        return TRUE;
      nm = m+1;
    }

//Normal
    m = nm;
    do{
      m = StrChr( m+1,SLASH_CHAR );
      if ( m ) {
        ch = *m;
        *m = 0;
      }

      tp = GetDriveType(nm);
      if ( tp == 0 ) return FALSE;

      if ( tp < DRIVE_REMOVABLE &&
           !CreateDirectory(nm,NULL) ) {

        if ( GetLastError() == ERROR_ALREADY_EXISTS ||
             GetLastError() == ERROR_INVALID_NAME ) {
          ;
        } else {
          Log(( "le: %d",GetLastError() ));
          return FALSE;
        }
      }

      if ( m )
        *m = ch;
    }while( m );

 return TRUE;
}

__int64 DECLSPEC Fsize( CONSTSTR nm )
  {  HANDLE f;
     DWORD lo,hi;

     f = CreateFile( nm, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
     if ( !f || f == INVALID_HANDLE_VALUE )
       return 0;
     lo = GetFileSize( f,&hi );
     CloseHandle( f );

     if ( lo == MAX_DWORD )
       return 0;
      else
       return ((__int64)hi) * ((__int64)MAX_DWORD) + ((__int64)lo);
}

__int64 DECLSPEC Fsize( HANDLE File )
  {  DWORD low,hi;

     low = GetFileSize( File,&hi );
     if ( low == MAX_DWORD )
       return 0;
      else
       return ((__int64)hi) * ((__int64)MAX_DWORD) + ((__int64)low);
}

BOOL DECLSPEC Fmove( HANDLE file,__int64 restart )
  {  LONG lo = (LONG)( restart % ((__int64)MAX_DWORD) ),
          hi = (LONG)( restart / ((__int64)MAX_DWORD) );

    if ( SetFilePointer( file,lo,&hi,FILE_BEGIN ) == 0xFFFFFFFF &&
         GetLastError() != NO_ERROR )
      return FALSE;

 return TRUE;
}

void DECLSPEC Fclose( HANDLE file )
  {
    if ( file ) {
      SetEndOfFile( file );
      CloseHandle( file );
    }
}

BOOL DECLSPEC Ftrunc( HANDLE h,DWORD move )
  {
     if ( move != FILE_CURRENT )
       if ( SetFilePointer(h,0,NULL,move) == 0xFFFFFFFF )
         return FALSE;

 return SetEndOfFile(h);
}

HANDLE DECLSPEC Fopen( CONSTSTR nm,CONSTSTR mode /*R|W|A[+]*/, DWORD attr )
  {  BOOL   rd  = toupper(mode[0]) == 'R';
     HANDLE h;

     if ( rd )
       h = CreateFile( nm, GENERIC_READ,
                       FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, attr, NULL );
      else
       h = CreateFile( nm, GENERIC_WRITE,
                       FILE_SHARE_READ, NULL, OPEN_ALWAYS, attr, NULL );

     if ( !h ||
          h == INVALID_HANDLE_VALUE )
       return NULL;

     do{
       if ( toupper(mode[0]) == 'A' || mode[1] == '+' )
         if ( SetFilePointer(h,0,NULL,FILE_END) == 0xFFFFFFFF )
           break;

       if ( !rd )
         SetEndOfFile(h);  //Ignore SetEndOfFile result in case of use with CON, NUL and others

       return h;
     }while(0);

     CloseHandle(h);
 return NULL;
}

int DECLSPEC Fwrite( HANDLE File,LPCVOID Buff,int Size )
  {  DWORD res;

 return WriteFile(File,Buff,(DWORD)Size,&res,NULL) ? ((int)res) : (-1);
}

int DECLSPEC Fread( HANDLE File,LPVOID Buff,int Size )
  {  DWORD res;

 return ReadFile(File,Buff,(DWORD)Size,&res,NULL) ? ((int)res) : (-1);
}

void DMessage( CONSTSTR str,BOOL full,int color,int y )
  {  char err[ FAR_MAX_PATHSIZE ];

     if ( full ) {
       TStrCpy( err, str );

       int len = strlen(err),
           w   = Min( (int)sizeof(err)-1, FP_ConWidth()-4 );

       while( len < w ) err[len++] = ' ';
       err[len] = 0;

       FP_Info->Text( 2,y,color,err );
     } else
       FP_Info->Text( 2,y,color,str );
}

void DECLSPEC IdleMessage( CONSTSTR str,int color )
  { static HANDLE hScreen;

//Clear
     if ( !str ) {
       if ( hScreen ) {
         FP_Info->RestoreScreen(hScreen);
         hScreen = NULL;
       }
       return;
     }

//Draw
     CONSTSTR msg = FP_GetMsg(str);

     if ( IS_FLAG(Opt.IdleMode,IDLE_CAPTION) )
       SaveConsoleTitle::Text( msg );

     if ( IS_FLAG(Opt.IdleMode,IDLE_CONSOLE) ) {
       DWORD    er  = GetLastError();
       BOOL     err = er != ERROR_CANCELLED &&
                      er != ERROR_SUCCESS &&
                      er != ERROR_NO_MORE_FILES;

       if ( !hScreen )
         hScreen = FP_Info->SaveScreen(0,0,FP_ConWidth(),2);

       DMessage( msg, err, color, 0 );
       if ( err )
         DMessage( FIO_ERROR, err, color, 1 );

       FP_Info->Text( 0,0,0,NULL );
     }
}

int DECLSPEC FMessage( unsigned int Flags,CONSTSTR HelpTopic,CONSTSTR *Items,int ItemsNumber,int ButtonsNumber )
  {  time_t b = time(NULL);
     BOOL   delayed;
     int    rc;

     rc = FP_Message( Flags, HelpTopic, Items, ItemsNumber, ButtonsNumber, &delayed);
     if ( delayed )
       AddWait( time(NULL)-b );

  return rc;
}

int DECLSPEC FDialog( int X2,int Y2,CONSTSTR HelpTopic,struct FarDialogItem *Item,int ItemsNumber )
  {  time_t b = time(NULL);
     int    rc;

     rc = FP_Info->Dialog(FP_Info->ModuleNumber,-1,-1,X2,Y2,HelpTopic,Item,ItemsNumber );

     AddWait( time(NULL)-b );

  return rc;
}

int DECLSPEC FDialogEx( int X2,int Y2,CONSTSTR HelpTopic,struct FarDialogItem *Item,int ItemsNumber,DWORD Flags,FARWINDOWPROC DlgProc,long Param )
  {  time_t b = time(NULL);
     int    rc;

     if ( DlgProc == (FARWINDOWPROC)MAX_DWORD )
       DlgProc = FP_Info->DefDlgProc;

     rc = FP_Info->DialogEx(FP_Info->ModuleNumber,-1,-1,X2,Y2,HelpTopic,Item,ItemsNumber,0,Flags,DlgProc,Param );

     AddWait( time(NULL)-b );

  return rc;
}

void DECLSPEC AddEndSlash( String& p, char slash )
  {
    if ( !p.Length() ) return;

    if ( !slash )
      slash = p.Chr('\\') ? '\\' : '/';

    if ( p[p.Length()-1] != slash )
      p.Add( slash );
}

void DECLSPEC AddEndSlash( char *Path,char slash, size_t ssz )
  {  size_t Length;

     if ( !Path || !Path[0] ) return;

     Length = strLen(Path)-1;
     if ( Length <= 0 || Length >= ssz ) return;

     if ( !slash )
       slash = strchr(Path,'\\') ? '\\' : '/';

     if ( Path[Length] != slash ) {
       Path[Length+1] = slash;
       Path[Length+2] = 0;
     }
}

void DECLSPEC DelEndSlash( String& p,char shash )
  {  int len;

     if ( (len=p.Length()-1) >= 0 &&
          p[len] == shash )
       p.SetLength( len );
}

void DECLSPEC DelEndSlash( char *Path,char shash )
  {  int len;

    if ( Path && (len=strLen(Path)-1) >= 0 &&
         Path[len] == shash )
      Path[len] = 0;
}

char* DECLSPEC TruncStr(char *Str,int MaxLength)
{
  int Length;
  if ((Length=strLen(Str))>MaxLength)
    if ( MaxLength>3 ) {
      char *TmpStr=new char[MaxLength+5];
      Sprintf(TmpStr,"...%s",Str+Length-MaxLength+3);
      StrCpy(Str,TmpStr);
      delete[] TmpStr;
    } else
      Str[MaxLength]=0;
  return(Str);
}

char *DECLSPEC PointToName( char *Path)
  {  char *NamePtr = Path;

    while( *Path ) {
      if (*Path=='\\' || *Path=='/' || *Path==':')
        NamePtr=Path+1;
      Path++;
    }

 return NamePtr;
}

BOOL DECLSPEC CheckForEsc( BOOL isConnection,BOOL IgnoreSilent )
  {  WORD  ESCCode = VK_ESCAPE;
     BOOL  rc;

     if ( !IgnoreSilent && IS_FLAG(FP_LastOpMode,OPM_FIND) )
       return FALSE;

     rc = CheckForKeyPressed( &ESCCode,1 );
     if ( !rc )
       return FALSE;

     rc = !Opt.AskAbort ||
          AskYesNo( FMSG( isConnection ? MTerminateConnection : MTerminateOp ) );

     if ( rc ) {
       Log(( "ESC: cancel detected" ));
     }

 return rc;
}

int DECLSPEC IsCaseMixed(char *Str)
{
  char AnsiStr[1024];
  OemToChar(Str,AnsiStr);
  while (*Str && !IsCharAlpha(*Str))
    Str++;
  int Case=IsCharLower( *Str );
  while (*(Str++))
    if (IsCharAlpha(*Str) && IsCharLower(*Str) != Case)
      return(TRUE);
  return(FALSE);
}

void DECLSPEC LocalLower(char *Str)
{
  OemToChar(Str,Str);
  CharLower(Str);
  CharToOem(Str,Str);
}

BOOL DECLSPEC IsDirExist( CONSTSTR nm )
  {  WIN32_FIND_DATA wfd;
     HANDLE          h;
     int             l;
     BOOL            res;
     char            str[ FAR_MAX_PATHSIZE ];

     StrCpy( str,nm );
     if ( (l=strLen(str)-1) > 0 && str[l] == SLASH_CHAR ) str[l] = 0;

     h = FindFirstFile( str,&wfd );
     if ( h == INVALID_HANDLE_VALUE ) return FALSE;
     res = IS_FLAG(wfd.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY);
     FindClose(h);
 return res;
}

void DECLSPEC FixFTPSlash( String& s ) { FixFTPSlash( (char*)s.c_str() ); }
void DECLSPEC FixFTPSlash( char *s )
  {
    if (!s) return;
    for ( ; *s; s++ )
      if ( *s == '\\' ) *s = '/';
}

void DECLSPEC FixLocalSlash( String& s ) { FixLocalSlash( (char*)s.c_str() ); }
void DECLSPEC FixLocalSlash( char *s )
  {
    if (!s) return;
    for ( ; *s; s++ )
      if ( *s == '/' ) *s = '\\';
}
