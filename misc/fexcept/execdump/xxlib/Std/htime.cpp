#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#define CHD(v)        if ( *p < '0' || *p > '9' )  break; \
                      v = v*10 + ((*p++)-'0');

#define CHD2(v)       v = 0;                           \
                      if ( *p < '0' || *p > '9' )  break; \
                      v = (*p++)-'0';                  \
                      if ( *p < '0' || *p > '9' )  break; \
                      v = v*10 + ((*p++)-'0');

/* --------------------------------------------------------------
   PRTimeOnly
*/
void PRTimeOnly::Zero( void )           { hour = min = sec = 0; }
BOOL PRTimeOnly::isNull( void ) const   { return hour == 0 && min == 0 && sec == 0; }
signed PRTimeOnly::Diff( const PRTimeOnly& tm ) const { return (hour-tm.hour)*3600 + (min-tm.min)*60 + (sec-tm.sec); }

BOOL PRTimeOnly::operator>( const PRTimeOnly& t )  const { return Cmp(t) > 0; }
BOOL PRTimeOnly::operator>=( const PRTimeOnly& t ) const { return Cmp(t) >= 0; }
BOOL PRTimeOnly::operator==( const PRTimeOnly& t ) const { return Cmp(t) == 0; }
BOOL PRTimeOnly::operator<=( const PRTimeOnly& t ) const { return Cmp(t) <= 0; }
BOOL PRTimeOnly::operator<( const PRTimeOnly& t )  const { return Cmp(t) < 0; }
BOOL PRTimeOnly::operator!=( const PRTimeOnly& t ) const { return Cmp(t) != 0; }

char *PRTimeOnly::GetStr( char *Buff/*=NULL*/,int BuffSize/*=0*/ ) const
  {  static char buff[10];
     if ( !Buff ) {
       Buff = buff;
       BuffSize = sizeof(buff);
     }
     SNprintf( Buff, BuffSize, "%02d:%02d:%02d", hour, min, sec );
 return Buff;
}

signed PRTimeOnly::Cmp( const PRTimeOnly& tm ) const
  {  register signed rc;
    rc = hour - tm.hour; if ( rc ) return rc;
    rc = min - tm.min;   if ( rc ) return rc;
    rc = sec - tm.sec;   if ( rc ) return rc;
 return 0;
}

#if defined(__HWIN32__)
void PRTimeOnly::Set( LPSYSTEMTIME st )
  {
     hour = st->wHour;
     min  = st->wMinute;
     sec  = st->wSecond;
}
void PRTimeOnly::Set( LPFILETIME ft )
  {  SYSTEMTIME st;
     FILETIME   tmp;

     if ( FileTimeToLocalFileTime(ft,&tmp) && FileTimeToSystemTime(&tmp,&st) )
       Set( &st );
      else
       Zero();
}
#endif

void PRTimeOnly::Set( CONSTSTR p )
  {
     do{
       if ( !p )
         break;

       CHD2( hour )
       if ( *p == 0 ) {
         min = sec = 0;
         return;
       } else
       if ( *p != ':' )
         break;
       p++;

       CHD2( min )
       if ( *p == 0 ) {
         sec = 0;
         return;
       } else
       if ( *p != ':' )
         break;
       p++;

       CHD2( sec )

       (void)*p;  //Mask `unused` warning
       return;
     }while(0);
     Zero();
}

#if defined(__VCL__)
TDateTime PRTimeOnly::GetDT( void ) const
  {
     try{
       return TDateTime(hour,min,sec,0);
     }catch(...){
       return TDateTime( 0.0 );
     }
}
void PRTimeOnly::Set( const TDateTime& p )
  {
     try{ unsigned short tmp;
          p.DecodeTime( &hour,&min,&sec,&tmp );
     }catch(...){ Zero(); }
}
#endif

/* --------------------------------------------------------------
  PRDateOnly
*/
void PRDateOnly::Zero( void )         { year = mon = mday = 0; }
BOOL PRDateOnly::isNull( void ) const { return year == 0 && mon == 0 && mday == 0; }
signed PRDateOnly::Diff( const PRDateOnly& tm ) const { return (year-tm.year)*364 + (mon-tm.mon)*30 + (mday-tm.mday); }

BOOL PRDateOnly::operator>( const PRDateOnly& t )  const { return Cmp(t) > 0; }
BOOL PRDateOnly::operator>=( const PRDateOnly& t ) const { return Cmp(t) >= 0; }
BOOL PRDateOnly::operator==( const PRDateOnly& t ) const { return Cmp(t) == 0; }
BOOL PRDateOnly::operator<=( const PRDateOnly& t ) const { return Cmp(t) <= 0; }
BOOL PRDateOnly::operator<( const PRDateOnly& t )  const { return Cmp(t) < 0; }
BOOL PRDateOnly::operator!=( const PRDateOnly& t ) const { return Cmp(t) != 0; }

char *PRDateOnly::GetStr( char *Buff/*=NULL*/,int BuffSize/*=0*/ ) const
  {  static char buff[12];
     if ( !Buff ) {
       Buff = buff;
       BuffSize = sizeof(buff);
     }
     SNprintf( Buff, BuffSize, "%02d-%02d-%04d", mday, mon, year );
 return Buff;
}

signed PRDateOnly::Cmp( const PRDateOnly& tm ) const
  {  register signed rc;
    rc = year - tm.year; if ( rc ) return rc;
    rc = mon - tm.mon;   if ( rc ) return rc;
    rc = mday - tm.mday; if ( rc ) return rc;
 return 0;
}

#if defined(__HWIN32__)
void PRDateOnly::Set( LPSYSTEMTIME st )
  {
     year = st->wYear;
     mon  = st->wMonth;
     mday = st->wDay;
}
void PRDateOnly::Set( LPFILETIME ft )
  {  SYSTEMTIME st;
     FILETIME   tmp;

     if ( FileTimeToLocalFileTime(ft,&tmp) && FileTimeToSystemTime(&tmp,&st) )
       Set( &st );
      else
       Zero();
}
#endif

/*  01234567890123456789
    DD-MM-YY
    DD/MM/YY
    DD-MM-YYYY
    DD/MM/YYYY
*/
void PRDateOnly::Set( CONSTSTR p )
  {
     do{
       if ( !p )
         break;

       CHD2( mday );
       if ( mday < 1 || mday > 31 || (*p != '-' && *p != '/') ) break;
       p++;

       CHD2( mon );
       if ( mon < 1 || mon > 12 || (*p != '-' && *p != '/') ) break;
       p++;

       CHD2( year );
       if ( *p >= '0' && *p <= '9' ) {
         year = year*10 + ((*p++)-'0');
         if ( *p >= '0' && *p <= '9' )
           year = year*10 + (*p-'0');
          else
           break;
       } else {
         if ( year < 10 )
           year += 2000;
          else
           year += 1900;
       }

       return;
     }while(0);
     Zero();
}

#if defined(__VCL__)
TDateTime PRDateOnly::GetDT( void ) const
  {
     try{
       return TDateTime(year,mon,mday);
     }catch(...){
       return TDateTime( 0.0 );
     }
}
void PRDateOnly::Set( const TDateTime& p )
  {
     try{ p.DecodeDate( &year,&mon,&mday );
     }catch(...){ Zero(); }
}
#endif

/* --------------------------------------------------------------
   PRTime
*/
void PRTime::Zero( void )         { sec  = min  = hour = mday = mon  = year = 0; }
BOOL PRTime::isNull( void ) const { return sec == 0 && min == 0 && hour == 0 && mday == 0 && mon == 0 && year == 0; }

signed PRTime::Cmp( const PRTime& tm ) const
  {  register signed rc;
    rc = year - tm.year; if ( rc ) return rc;
    rc = mon - tm.mon;   if ( rc ) return rc;
    rc = mday - tm.mday; if ( rc ) return rc;
    rc = hour - tm.hour; if ( rc ) return rc;
    rc = min - tm.min;   if ( rc ) return rc;
    rc = sec - tm.sec;   if ( rc ) return rc;
 return 0;
}

signed PRTime::Cmp( const PRTimeOnly& tm ) const
  {  register signed rc;
    rc = hour - tm.hour; if ( rc ) return rc;
    rc = min - tm.min;   if ( rc ) return rc;
    rc = sec - tm.sec;   if ( rc ) return rc;
 return 0;
}

signed PRTime::Cmp( const PRDateOnly& tm ) const
  {  register signed rc;
    rc = year - tm.year; if ( rc ) return rc;
    rc = mon - tm.mon;   if ( rc ) return rc;
    rc = mday - tm.mday; if ( rc ) return rc;
 return 0;
}

#if defined(__HWIN32__)
void PRTime::Set( LPSYSTEMTIME st )
  {
     year = st->wYear;
     mon  = st->wMonth;
     mday = st->wDay;
     hour = st->wHour;
     min  = st->wMinute;
     sec  = st->wSecond;
}
void PRTime::Set( LPFILETIME ft )
  {  SYSTEMTIME st;
     FILETIME   tmp;

     if ( FileTimeToLocalFileTime(ft,&tmp) && FileTimeToSystemTime(&tmp,&st) )
       Set( &st );
      else
       Zero();
}
#endif

void PRTime::Set( time_t p )
  {  struct tm *t;

     if ( !p ) { Zero(); return; }

     t = localtime(&p);
     if ( !t ) { Zero(); return; }

     sec    = (WORD)t->tm_sec;
     min    = (WORD)t->tm_min;
     hour   = (WORD)t->tm_hour;
     mday   = (WORD)t->tm_mday;
     mon    = (WORD)(t->tm_mon + 1);
     year   = (WORD)(t->tm_year + 1900);
}

/*  01234567890123456789
    DD-MM-YY HH:MM:SS
    DD/MM/YY HH:MM:SS
    DD-MM-YYYY HH:MM:SS
    DD/MM/YYYY HH:MM:SS
*/
void PRTime::Set( CONSTSTR p )
  {
     do{
       if ( !p )
         break;

       CHD2( mday );
       if ( mday < 1 || mday > 31 || (*p != '-' && *p != '/') ) break;
       p++;

       CHD2( mon );
       if ( mon < 1 || mon > 12 || (*p != '-' && *p != '/') ) break;
       p++;

       CHD2( year );
       if ( *p >= '0' && *p <= '9' ) {
         year = year*10 + ((*p++)-'0');
         if ( *p >= '0' && *p <= '9' )
           year = year*10 + ((*p++)-'0');
          else
           break;
       } else {
         if ( year < 10 )
           year += 2000;
          else
           year += 1900;
       }

       if ( *p == 0 ) {
         hour = min = sec = 0;
         return;
       } else
       if ( *p != ' ' )
         break;
       p++;

       CHD2( hour )
       if ( *p == 0 ) {
         min = sec = 0;
         return;
       } else
       if ( *p != ':' )
         break;
       p++;

       CHD2( min )
       if ( *p == 0 ) {
         sec = 0;
         return;
       } else
       if ( *p != ':' )
         break;
       p++;

       CHD2( sec )

       (void)*p;  //Mask `unused` warning
       return;
     }while(0);
     Zero();
}

void PRTime::Set( PPRDateOnly d,PPRTimeOnly t )
  {
     Zero();
     if (d) {
       year = d->year;
       mon  = d->mon;
       mday = d->mday;
     }
     if (t) {
       hour = t->hour;
       min  = t->min;
       sec  = t->sec;
     }
}

struct tm *PRTime::GetTM( struct tm *t ) const
  {  static struct tm tmp;
     if (!t) t = &tmp;
     t->tm_sec   = sec;
     t->tm_min   = min;
     t->tm_hour  = hour;
     t->tm_mday  = mday;
     t->tm_mon   = mon-1;
     t->tm_year  = year - 1900;
     t->tm_isdst = -1; /* Disable summer time */
 return t;
}

time_t PRTime::GetTT( time_t *p ) const
  {  static time_t tmp;
     struct tm     t;
     if (!p) p = &tmp;
     *p = isNull() ? 0 : mktime( GetTM(&t) );
 return *p;
}

char *PRTime::GetStr( char *p,int sz,CONSTSTR Format ) const
  {  static char tmp[ 30 ];
     struct tm t;

     if (!p) {
       p = tmp;
       sz = sizeof(tmp);
     }

     if ( !Format )
       Format = "%d-%m-%Y %H:%M:%S";

     strftime( p,sz,Format,GetTM(&t) );
 return p;
}

void PRTime::GetTO( PPRTimeOnly t ) const
  {
    t->hour = hour;
    t->min  = min;
    t->sec  = sec;
}

void PRTime::GetDO( PPRDateOnly d ) const
  {
    d->year = year;
    d->mon  = mon;
    d->mday = mday;
}

#if defined(__VCL__)
TDateTime PRTime::GetDT( void ) const
  {  TDateTime tm;
     try{ tm = TDateTime(year,mon,mday) + TDateTime(hour,min,sec,0); }catch(...){ tm = 0.0; }
  return tm;
}

void PRTime::Set( const TDate& d,const TTime& t )
  {
     try{
       d.DecodeDate( &year,&mon,&mday );

       unsigned short tmp;
       t.DecodeTime( &hour,&min,&sec,&tmp );
     }catch(...){ Zero(); }
}

void PRTime::Set( const TDateTime& p )
  {
     try{
       p.DecodeDate( &year,&mon,&mday );

       unsigned short tmp;
       p.DecodeTime( &hour,&min,&sec,&tmp );
     }catch(...){ Zero(); }
}
#endif

BOOL PRTime::operator>( const PRTime& t )  const { return Cmp(t) > 0; }
BOOL PRTime::operator>=( const PRTime& t ) const { return Cmp(t) >= 0; }
BOOL PRTime::operator==( const PRTime& t ) const { return Cmp(t) == 0; }
BOOL PRTime::operator<=( const PRTime& t ) const { return Cmp(t) <= 0; }
BOOL PRTime::operator<( const PRTime& t )  const { return Cmp(t) < 0; }
BOOL PRTime::operator!=( const PRTime& t ) const { return Cmp(t) != 0; }


/* --------------------------------------------------------------
   FUNCTIONS
*/
MyString MYRTLEXP Time2Str( time_t tm, bool TimeOnly /*=false*/)
  {  PRTime t;
     t.Set( tm );
 return Time2Str( t, TimeOnly );
}

MyString MYRTLEXP Time2Str( const PRTime& t, bool TimeOnly /*=false*/)
  {  MyString s;

     if ( TimeOnly )
       s.printf( "%02d:%02d:%02d", t.hour, t.min, t.sec );
      else
       s.printf( "%02d-%02d-%04d %02d:%02d:%02d",
                 t.mday, t.mon, t.year,
                 t.hour, t.min, t.sec );

 return s;
}

MyString MYRTLEXP Sec2Str( DWORD tm )
  {  MyString s;

     s.printf( "%02d:%02d:%02d",
               tm/3600, (tm%3600) / 60, tm%60 );
 return s;
}

MyString MYRTLEXP Sec2Str( const CMP_TIME_TYPE& tm )
  {  MyString s;
     int      n = (int)tm;

     s.printf( "%02d:%02d:%02d.%02d",
               n/3600, (n%3600) / 60, n%60, (int)((tm-n)*100) );
 return s;
}
//---------------------------------------------------------------------------
#define TDATETIME_1990   32874
#define TDATETIME_1msec  1.15740740740741E-08
#define TDATETIME_1sec   1.15740740740741E-05

static time_t StartTime = 0;
static void FillStartTime( void )
  {
     if ( StartTime ) return;

     PRTime tt;
     tt.year = 1990;
     tt.mon  = 1;
     tt.mday = 1;
     tt.hour = 0;
     tt.min  = 0;
     tt.sec  = 0;

     StartTime = tt.GetTT(NULL);
}

void MYRTLEXP DoubleToDate( PRTime& t, double v )
  {
#if defined(__VCL__)
     t.Set( TDateTime(v) );
#else
     FillStartTime();
     t.Set( StartTime + time_t((v - TDATETIME_1990 ) / TDATETIME_1sec ));
#endif
}

double MYRTLEXP DateToDouble( const PRTime& t )
  {
#if defined(__VCL__)
 return t.GetDT();
#else
     FillStartTime();
 return TDATETIME_1990 + (t.GetTT(NULL) - StartTime) * TDATETIME_1sec;
#endif
}

double MYRTLEXP DateToDouble( time_t t )
  {
     FillStartTime();
 return TDATETIME_1990 + (t - StartTime) * TDATETIME_1sec;
}

pchar MYRTLEXP TimeDiff2TimeStr( const CMP_TIME_TYPE& d, char *b /*=NULL*/ )
  {  static char buff[ 10 ];
     if ( !b ) b = buff;

     sprintf( b, "%02d:%02d:%02d", ((int)d) / 3600, (((int)d) % 3600) / 60, ((int)d) % 60 );

 return b;
}
