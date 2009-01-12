#ifndef __MY_STRING
#define __MY_STRING

/************************************
            CLASSES
 ************************************
  Strings
     - MyString           - dynamic resized string
     - StatString         - string with fixed maximul length
        PathString        - static string with size of MAX_PATH_SIZE
     - ConstString        - holder for const string emulated all MyString acess methods
 ************************************/

/************************************
            MyString
 ************************************/
#define MString MyString
CLASS( MyString )
  public:
   char *str;
   int   len,maxchar;
  private:
   void BeginSet( void );
  public:
   virtual ~MyString();

   MyString( void );
   MyString( const MyString& s );
   MyString( CONSTSTR s );
   MyString( CONSTSTR s,int count );
//Convert <type> to string by `itol` etc.
   MyString( signed int n );
   MyString( unsigned n );
   MyString( signed long n );
   MyString( unsigned long n );
   MyString( float n );
   MyString( double n );
   MyString( char n );
   MyString( void CT_FAR *n );
   MyString( DWORD n,CONSTSTR Format );
   MyString( double n,CONSTSTR Format );
#if defined( __HDOS__ ) || defined( __QNX__ )
   MyString( void CT_NEAR *n );
#endif

#if defined( __VCL__ )
   MyString( const AnsiString& v );
   operator AnsiString(void) const;
#endif

   char       *c_str( void )                           const;
   char       *Text( void )                            const;
   int         Length( void )                          const;
   BOOL        RecalcLength( void );
   void        SetLength( int sz );

   CONSTSTR    Alloc( CONSTSTR s,int maxLen = -1 );
   CONSTSTR    Alloc( int l );

   CONSTSTR    Set( CONSTSTR s );
   CONSTSTR    Set( CONSTSTR s, int from, int to /*=-1*/ );
   CONSTSTR    Set( const MyString& s );

   int         printf( CONSTSTR fmt,... );
   int         vprintf( CONSTSTR fmt,va_list list );

   void        cat( CONSTSTR s,... );
   void        vcat( CONSTSTR s,va_list a );

   MyString&   Add( const MyString& s );
   MyString&   Add( CONSTSTR s );
   MyString&   Add( char s );
   MyString&   Add( CONSTSTR s, int StartToAdd_Inclusive, int EndToAdd_Exclusive /*-1*/ );

   MyString    SubStr( int start, int end = -1 ) const;

   void        Del( int StartChar, int Count );
   void        DelChars( char AllCharsToDel = ' ' );
   void        InsCharPos( int InsertPos,char ch );

   // Rets -1 if not found.
   int         Chr( char ch,int StartToSearchFrom = 0 )     const; //strchr
   int         Chr( CONSTSTR set,int StartToSearchFrom = 0 )const; //search first char contains in set
   int         RChr( char ch,int EndToSearchAt = -1 )       const; //strrchr
   int         Str( CONSTSTR ch,int pos = 0 )               const; //strstr
   MyString&   operator=( const MyString& s );
   MyString&   operator=( CONSTSTR s );

   BOOL        operator!=( const MyString& s )         const;
   BOOL        operator!=( CONSTSTR s )                const;
   BOOL        operator==( const MyString& s )         const;
   BOOL        operator==( CONSTSTR s )                const;
   BOOL        Cmp( CONSTSTR s,int count = -1, BOOL isCase = FALSE ) const;

   char        operator[]( int num )                   const;
   char        SetChar( int num,char ch );

   void        Trim( char ch = ' ' );                         //changes current string
   void        RTrim( char ch = ' ' );
   void        LTrim( char ch = ' ' );

   MyString&   Trimmed( char ch = ' ' );                         //changes current string
   MyString&   RTrimmed( char ch = ' ' );
   MyString&   LTrimmed( char ch = ' ' );

   MyString&   ChangeChar( char och, char nch );
   MyString&   FixSlashChars( char ch = SLASH_CHAR );

   void        Case( msCaseTypes type );                      //changes current string
   void        MakeAlign( int maxWidth, talTypes align, talTypes *over );
};

/************************************
   StatString
 ************************************/
#define _SS_SET( v ) StrCpy(Data,v,SizeT); Len = strLen(Data);

template <unsigned SizeT> class StatString {
   char Data[ SizeT+1 ];
   int  Len;
  public:
   StatString( void )                                                { Data[0] = 0; Len = 0; }
   StatString( CONSTSTR str )                                        { _SS_SET(str) }
   StatString( const MyString& str )                                 { _SS_SET(str.Text()) }
   StatString( const StatString<SizeT>& str )                        { _SS_SET(str.Text()) }

#if defined( __VCL__ )
   operator           AnsiString(void)                         const { return AnsiString(Data); }
#endif
   operator           MyString(void)                           const { return MyString(Data); }

   operator CONSTSTR( void )                                   const { return Data; }
   CONSTSTR           Text( void )                             const { return Data; }
   CONSTSTR           c_str( void )                            const { return Data; }
   int                Length( void )                           const { return Len; }
   unsigned           Size( void )                             const { return SizeT; }
   BOOL               RecalcLength( void )                           { Len = strLen(Data); return Len > 0; }
   void               SetLength( int sz )                            { if (sz >= 0 && sz <= SizeT) { Len = sz; Data[sz] = 0; } }

   void               Set( CONSTSTR str )                            { _SS_SET(str)        }
   void               Set( const MyString& str )                     { _SS_SET(str.Text()) }
   void               Set( const StatString<SizeT>& str )            { _SS_SET(str.Data)   }

   StatString&        Add( const StatString<SizeT>& s )              { StrCat(Data,s.Data,SizeT); Len = strLen(Data); return *this; }
   StatString&        Add( char *s )                                 { StrCat(Data,s,SizeT);      Len = strLen(Data); return *this; }
   StatString&        Add( char s )                                  { if (Len<SizeT) { Data[Len++]=s; Data[Len]=0; } return *this; }

   StatString<SizeT>& operator=( const StatString<SizeT>& s )        { _SS_SET(s.Data); return *this; }
   StatString<SizeT>& operator+=( const StatString<SizeT>& s )       { StrCat(Data,s.Data,SizeT); Len = strLen(Data); return *this; }
   StatString<SizeT>  operator+( const StatString<SizeT>& s )  const { StatString<SizeT> tmp(*this); tmp+=s; return tmp; }

   BOOL               operator!=( const StatString& s )        const { return Len != s.Len || strcmp(Text(),s.Text()) != 0; }
   BOOL               operator==( const StatString& s )        const { return Len == s.Len && strcmp(Text(),s.Text()) == 0; }
   char               operator[]( int num )                    const { return (((unsigned)num) <= ((unsigned)Len))?Data[num]:'\0'; }
   char               SetChar( int num,char ch )                     { ch = (num >= 0 && num <= SizeT)?(Data[num]=ch):'\0'; Len = strLen(Data); return ch; }
   BOOL               Cmp( CONSTSTR s,int count = -1 )         const { return StrCmp(Data,s,count) == 0; }
   void               Case( msCaseTypes type )                       { StrCase(Data,type); }
   int                Chr( char ch,int pos = 0 )                     { return (pos<Len)?StrNChr(Data+pos,ch):(-1); }
   int                ColCount( char *seps )                   const { return StrColCount(Data,seps); }
   StatString<SizeT>  GetCol( int number,char *seps )          const { return StatString<SizeT>(StrGetCol(Data,number,seps)); }
   StatString<SizeT>  DelCol( int number,char *seps )                { return StatString<SizeT>(StrDelCol(Data,number,seps)); }
   StatString<SizeT>& FixSlashChars( char ch = SLASH_CHAR );

   int         printf( CONSTSTR fmt,... );
   int         vprintf( CONSTSTR fmt,va_list list );
   int         cat( CONSTSTR s,... );
   int         vcat( CONSTSTR s,va_list a );
};
#undef _SS_SET

template <unsigned SizeT> StatString<SizeT>& StatString<SizeT>::FixSlashChars( char ch )
  {
//Declared in [disk_io.h]
extern pchar FixSlashChars( char *str,char CharChangeTo );
    if (Len)
      ::FixSlashChars( Data, ch );
 return *this;
}

template <unsigned SizeT> int StatString<SizeT>::printf( CONSTSTR fmt,... )
  {  va_list  a;
     int      res;

     va_start( a,fmt );
       res = vprintf( fmt,a );
     va_end( a );
 return res;
}

template <unsigned SizeT> int StatString<SizeT>::vprintf( CONSTSTR fmt,va_list list )
  {
 return Len = VSNprintf( Data,SizeT,fmt,list );
}

template <unsigned SizeT> int StatString<SizeT>::cat( CONSTSTR fmt,... )
  {  va_list  a;
     int      res;

     va_start( a,fmt );
       res = vcat( fmt,a );
     va_end( a );
 return res;
}

template <unsigned SizeT> int StatString<SizeT>::vcat( CONSTSTR fmt,va_list list )
  {  int olen = Length();
     if ( olen >= SizeT ) return 0;

 return Len = (olen + VSNprintf( Data+olen,SizeT-olen,fmt,list ));
}

typedef StatString<MAX_PATH_SIZE> PathString;

/************************************
   ConstString
 ************************************/
LOCALCLASS( ConstString )
   CONSTSTR Data;
   int      Len;
  public:
   ConstString( CONSTSTR str = NULL )                { Data = (str)?str:""; Len = strLen(Data); }

#if defined( __VCL__ )
   operator   AnsiString(void)                 const { return AnsiString(Data); }
#endif
   operator   MyString(void)                   const { return MyString(Data); }

   operator   CONSTSTR( void )                 const { return Data; }
   CONSTSTR   Text( void )                     const { return Data; }
   CONSTSTR   c_str( void )                    const { return Data; }
   int        Length( void )                   const { return Len; }

   char       operator[]( int num )            const { return ( ((unsigned)num) <= ((unsigned)Len) )?Data[num]:'\0'; }
   BOOL       Cmp( CONSTSTR s,int count = -1 ) const { return StrCmp(Data,s,count) == 0; }
   int        Chr( char ch,int pos = 0 )             { return (pos<Len)?StrNChr(Data+pos,ch):(-1); }
   int        ColCount( char *seps )           const { return StrColCount(Data,seps); }
   MyString   GetCol( int number,char *seps )  const { return MyString(StrGetCol(Data,number,seps)); }
   MyString   DelCol( int number,char *seps )        { return MyString(StrDelCol(Data,number,seps)); }
};

#endif
