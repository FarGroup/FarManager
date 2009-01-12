#ifndef __MY_SRITICAL_SECTION
#define __MY_SRITICAL_SECTION

#if defined(__HWIN__)
  //Alredy defined
#else
#if defined(__QNX__)
  typedef sem_t  CRITICAL_SECTION;
  typedef sem_t *LPCRITICAL_SECTION;
#else
#if defined(__GNUC__)
  typedef BOOL  CRITICAL_SECTION;
  typedef BOOL *LPCRITICAL_SECTION;
#else
#if defined(__TEC32__) || defined(__REALDOS__) || defined(__PROTDOS__)
  typedef BOOL  CRITICAL_SECTION;
  typedef BOOL *LPCRITICAL_SECTION;
#else
  #error ERR_PLATFORM
#endif //
#endif //GNUC
#endif //QNX
#endif //Win

#if !defined(__HWIN__)
  HDECLSPEC BOOL MYRTLEXP DeleteCriticalSection( LPCRITICAL_SECTION cs );
  HDECLSPEC BOOL MYRTLEXP InitializeCriticalSection( LPCRITICAL_SECTION cs );
  HDECLSPEC void MYRTLEXP EnterCriticalSection( LPCRITICAL_SECTION cs );
  HDECLSPEC BOOL MYRTLEXP LeaveCriticalSection( LPCRITICAL_SECTION cs );
  HDECLSPEC BOOL MYRTLEXP TryEnterCriticalSection( LPCRITICAL_SECTION cs );
#else
  #if !defined(TryEnterCriticalSection)
    #define TryEnterCriticalSection(cs) TRUE
  #endif
#endif

STRUCT( PRcs )
   CRITICAL_SECTION cs;
  public:
   PRcs( void );
   ~PRcs();

   void Enter( void );
   BOOL Try( void );
   void Leave( void );
};

/****************************************************
   RW access
   Uses semaphores to acces single writer or multiply readers
 ****************************************************/
#if defined(__HWIN__)
enum AccTypes {
  ACC_READ  = 0x0001,
  ACC_WRITE = 0x0002,
  ACC_RW    = 0x0003
};

HDECLSPEC HANDLE MYRTLEXP PRAccessCreate( void );
HDECLSPEC void   MYRTLEXP PRAccessDestroy( HANDLE acc );
HDECLSPEC BOOL   MYRTLEXP PRAccessEnter( HANDLE acc,AccTypes op );
HDECLSPEC void   MYRTLEXP PRAccessLeave( HANDLE acc );

/****************************************************
   HSafeVal
      Multithreaded accessible value
 ****************************************************/
template <class T> class HSafeVal {
   T      Val;
   HANDLE Handle;
  public:
    HSafeVal( const T& v )    { Val = v; Handle = PRAccessCreate(); }
    virtual ~HSafeVal()       { PRAccessDestroy(Handle); }

    T     GetValue( void )        { T v; PRAccessEnter(Handle,ACC_READ);  v = Val; PRAccessLeave(Handle); return v; }
    T     SetValue( const T& v )  {      PRAccessEnter(Handle,ACC_WRITE); Val = v; PRAccessLeave(Handle); return v; }
    BOOL  Access( void )          { return PRAccessEnter(Handle,ACC_WRITE); }
    void  Leave( void )           { return PRAccessLeave(Handle); }
};

template <class T> class HOrdSafeVal : public HSafeVal<T> {
  public:
    HOrdSafeVal( T v = 0 ) : HSafeVal(v) {}
};

STRUCT( PRAccess )
   HANDLE Handle;
  public:
   PRAccess( void );
   ~PRAccess();

   BOOL Enter( AccTypes op );
   void Leave( void );
};
#endif //HWIN

#endif
