#ifndef __MY_HTIME
#define __MY_HTIME

/*
   Uniplatform time and date holders

   PRTimeOnly  - contains only time
   PRDateOnly  - contains only date
   PRTime      - containt time and date both
*/
STRUCT( PRTimeOnly )
  WORD hour; // 0..23
  WORD min;  // 0..59
  WORD sec;  // 0..59

  void       Zero( void );                       // Set all field to zero.
  BOOL       isNull( void ) const;               // Check if all fields are zero
  signed     Cmp( const PRTimeOnly& tm ) const;  // Ret: >0 - if this > tm
  signed     Diff( const PRTimeOnly& tm ) const; // Ret difference (this-tm) in seconds
  char      *GetStr( char *Buff/*=NULL*/,int BuffSize/*=0*/ ) const; //HH:MM:SS
  void       Set( CONSTSTR p );                  // Format: HH:MM:SS
#if defined(__VCL__)
  TDateTime  GetDT( void ) const;
  void       Set( const TDateTime& p );
#endif
#if defined(__HWIN32__)
  void       Set( LPSYSTEMTIME st );
  void       Set( LPFILETIME ft );
#endif

  BOOL operator>( const PRTimeOnly& t )  const;
  BOOL operator>=( const PRTimeOnly& t ) const;
  BOOL operator==( const PRTimeOnly& t ) const;
  BOOL operator<=( const PRTimeOnly& t ) const;
  BOOL operator<( const PRTimeOnly& t )  const;
  BOOL operator!=( const PRTimeOnly& t ) const;
};

STRUCT( PRDateOnly )
  WORD year; // full number
  WORD mon;  // 1..12
  WORD mday; // 1..31

  void       Zero( void );                       // Set all field to zero.
  BOOL       isNull( void ) const;               // Check if all fields are zero
  signed     Cmp( const PRDateOnly& tm ) const;  // Ret: >0 - if this > tm
  signed     Diff( const PRDateOnly& tm ) const; // Ret difference (this-tm) in days (average: year=365 and month=30 days)
  char      *GetStr( char *Buff/*=NULL*/,int BuffSize/*=0*/ ) const; //DD-MM-YYYY

  /* Formats:
       DD-MM-YY
       DD/MM/YY
       DD-MM-YYYY
       DD/MM/YYYY
  */
  void       Set( CONSTSTR p );
#if defined(__HWIN32__)
  void       Set( LPSYSTEMTIME st );
  void       Set( LPFILETIME ft );
#endif
#if defined(__VCL__)
  void       Set( const TDateTime& p );
  TDateTime  GetDT( void ) const;
#endif

  BOOL operator>( const PRDateOnly& t ) const;
  BOOL operator>=( const PRDateOnly& t ) const;
  BOOL operator==( const PRDateOnly& t ) const;
  BOOL operator<=( const PRDateOnly& t ) const;
  BOOL operator<( const PRDateOnly& t ) const;
  BOOL operator!=( const PRDateOnly& t ) const;
};

STRUCT( PRTime )
  WORD year; // full number
  WORD mon;  // 1..12
  WORD mday; // 1..31
  WORD hour; // 0..23
  WORD min;  // 0..59
  WORD sec;  // 0..59

  /* Formats:
      DD-MM-YY HH:MM:SS
      DD/MM/YY HH:MM:SS
      DD-MM-YYYY HH:MM:SS
      DD/MM/YYYY HH:MM:SS
  */
  void       Set( CONSTSTR p );
  void       Set( PPRDateOnly d/*=NULL*/,PPRTimeOnly t/*=NULL*/ ); // Leaves fields empty if date or time not specified
  void       Set( time_t p );
#if defined(__VCL__)
  void       Set( const TDateTime& p );
  void       Set( const TDate& d,const TTime& t );
#endif
#if defined(__HWIN32__)
  void       Set( LPSYSTEMTIME st );
  void       Set( LPFILETIME ft );
#endif

  void       Zero( void );                        // Set all field to zero.
  BOOL       isNull( void ) const;                // Check if all fields are zero
  signed     Cmp( const PRTime& tm ) const;       // Ret: >0 - if this > tm
  signed     Cmp( const PRTimeOnly& tm ) const;   //      <0 - if this < tm
  signed     Cmp( const PRDateOnly& tm ) const;   //       0 - if this = tm

  struct tm *GetTM( struct tm *Buff/*=NULL*/ ) const;           //If Buff=NULL rets local buff
  time_t     GetTT( time_t *Buff/*=NULL*/ ) const;              //If Buff=NULL rets local buff
  char      *GetStr( char *Buff/*=NULL*/,int BuffSize/*=0*/,
                     CONSTSTR Format = NULL ) const;            //If Buff=NULL rets local buff and BuffSize not used.
                                                                //Format==NULL means "%d-%m-%Y %H:%M:%S".
  void       GetTO( PRTimeOnly *t ) const;
  void       GetDO( PRDateOnly *d ) const;
#if defined(__VCL__)
  TDateTime  GetDT( void ) const;
#endif

  BOOL operator>( const PRTime& t ) const;
  BOOL operator>=( const PRTime& t ) const;
  BOOL operator==( const PRTime& t ) const;
  BOOL operator<=( const PRTime& t ) const;
  BOOL operator<( const PRTime& t ) const;
  BOOL operator!=( const PRTime& t ) const;
};

HDECLSPEC MyString MYRTLEXP Time2Str( time_t tm, bool TimeOnly /*=false*/ );
HDECLSPEC MyString MYRTLEXP Time2Str( const PRTime& t, bool TimeOnly /*=false*/);
HDECLSPEC MyString MYRTLEXP Sec2Str( DWORD tm );
HDECLSPEC MyString MYRTLEXP Sec2Str( const CMP_TIME_TYPE& tm );

HDECLSPEC void     MYRTLEXP DoubleToDate( PRTime& t, double v );
HDECLSPEC double   MYRTLEXP DateToDouble( const PRTime& t );
HDECLSPEC double   MYRTLEXP DateToDouble( time_t t );

HDECLSPEC pchar    MYRTLEXP TimeDiff2TimeStr( const CMP_TIME_TYPE& diff, char *buff = NULL );

#endif
