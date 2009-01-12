#ifndef __MY_PERIOD
#define __MY_PERIOD

HDECLSPEC HANDLE MYRTLEXP PRPeriodCreate( DWORD ms );
HDECLSPEC BOOL   MYRTLEXP PRPeriodEnd( HANDLE p );      //Check if period ends
HDECLSPEC DWORD  MYRTLEXP PRPeriodTime( HANDLE p );     //Get time since period start (since last reset)
HDECLSPEC DWORD  MYRTLEXP PRPeriodPeriod( HANDLE p );   //Get period base wait period
HDECLSPEC void   MYRTLEXP PRPeriodDestroy( HANDLE p );
HDECLSPEC void   MYRTLEXP PRPeriodReset( HANDLE p );    //Resets period counter to current time

inline DWORD PRPeriodLast( HANDLE p ) { return PRPeriodEnd(p) ? 0 : (PRPeriodPeriod(p) - PRPeriodTime(p)); }

LOCALSTRUCT( PRPeriod )
    HANDLE h;

  public:
    PRPeriod( void )     { h = NULL; }
    PRPeriod( DWORD ms ) { h = PRPeriodCreate(ms); }
    ~PRPeriod()          { PRPeriodDestroy(h); }

    void Set( DWORD ms ) { PRPeriodDestroy(h); h = ms ? PRPeriodCreate(ms) : 0; }
    BOOL End( void )     { return PRPeriodEnd(h); }
    void Reset( void )   { if (h) PRPeriodReset(h); }
    BOOL isSet( void )   { return h != NULL; }
};

#endif
