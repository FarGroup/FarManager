#ifndef __FTP_PROGRESS_INTERNAL
#define __FTP_PROGRESS_INTERNAL

#include "fstdlib.h"         //FAR plugin stdlib
#include "../ftp_Plugin.h"

//------------------------------------------------------------------------
#define NET_TO_UPPER(x) ((((unsigned int) (x)) > 0x7f) ? x : toupper(x))
#define NET_TO_LOWER(x) ((((unsigned int) (x)) > 0x7f) ? x : tolower(x))

#define NET_IS_ALPHA(x) ((((unsigned int) (x)) > 0x7f) ? 0 : isalpha(x))
#define NET_IS_DIGIT(x) ((((unsigned int) (x)) > 0x7f) ? 0 : isdigit(x))
#define NET_IS_SPACE(x) ((((unsigned int) (x)) > 0x7f) ? 0 : isspace(x))

#define NOT_TIME  ((__int64)-1)
typedef const FILETIME *LPCFILETIME;

#define CHECK( v, ret ) if v { Log(( "Parser failed: [" #v "] at " __FILE__ " in %d", __LINE__ )); return ret; }

STRUCT( Time_t )
  union {
    FILETIME FileTime;
    __int64    Value;
  };

  bool operator==( __int64 v )      { return Value == v; }
  void operator=( __int64 v )       { Value = v; }
  void operator=( const Time_t& v ) { Value = v.Value; }

  operator LPCFILETIME()            { return &FileTime; }
  operator LPFILETIME()             { return &FileTime; }
};

STRUCTBASE( NET_FileEntryInfo, public FTPFileInfo )
    Time_t date;       //Last write time
    Time_t cr_date;    //Time of ceation
    Time_t acc_date;   //Time of last access
    __int64  size;

    NET_FileEntryInfo( void ) { memset( this,0,sizeof(*this) ); }
};

//------------------------------------------------------------------------
extern char *SkipSpace( char *l );
extern char *SkipDigit( char *l );
extern char *SkipNSpace( char *l );
extern char *SkipNX( char *l, char ch );

/*
 * Get month number (-1 on error).
 */
extern BOOL TwoDigits( CONSTSTR s,WORD& val );

/* remove front and back white space
 * modifies the original string
 */
extern char *XP_StripLine( char *string );

/*
 * Get month number by month name in range [1..12] or MAX_WORD on error.
 */
extern WORD NET_MonthNo( CONSTSTR month );

/*
 * Converts PNET_FileEntryInfo fields to PFTPFileInfo.
 */
extern BOOL ConvertEntry( PNET_FileEntryInfo inf, PFTPFileInfo p );

/*
 * Checks if line starts with one of specified lines.
 */
BOOL StartsWith( CONSTSTR line, CONSTSTR *lines );

/*
 * Check if string starts from std unix symbols
 */
BOOL is_unix_start( char *s, int len, int *off = NULL );

//------------------------------------------------------------------------
const char PCTCP_PWD_Title[] = "Current working directory is ";
#define PCTCP_PWD_TITLE_LEN (sizeof(PCTCP_PWD_Title)-1)

extern BOOL net_convert_unix_date( pchar& datestr, Time_t& decoded );

extern BOOL DECLSPEC idPRParceCMS( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idPRParceDos( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idPRParceEPLF( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idPRParceNETWARE( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idPRParceOS2( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idPRParcePCTCP( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idDirParcePCTCP( const PFTPServerInfo Server, CONSTSTR Line, char *CurDir, size_t CurDirSize );
extern BOOL DECLSPEC idPRParceSkirdin( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idDirParceSkirdin( const PFTPServerInfo Server, CONSTSTR Line, char *CurDir, size_t CurDirSize );
extern BOOL DECLSPEC idPRParceTCPC( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idPRParceUnix( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idPRParceVMS( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idDirPRParceVMS( const PFTPServerInfo Server, CONSTSTR Line, char *CurDir, size_t CurDirSize );
extern BOOL DECLSPEC idPRParceVX_DOS( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idPRParceOS400( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idPRParceMVS( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len );
extern BOOL DECLSPEC idDirParceMVS( const PFTPServerInfo Server, CONSTSTR Line, char *CurDir, size_t CurDirSize );

#endif
