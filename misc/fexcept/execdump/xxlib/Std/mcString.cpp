#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/************************************
            MyString
 ************************************/
MyString::~MyString()                          { if (str) { StrFree(str); str = NULL; } }
MyString::MyString( void )                     { BeginSet(); }
MyString::MyString( const MyString& s )        { BeginSet(); Alloc( s.Text() ); }
MyString::MyString( CONSTSTR s )               { BeginSet(); Alloc( s,-1 ); }
MyString::MyString( CONSTSTR s,int c )         { BeginSet(); Alloc( s,c ); }
MyString::MyString( signed int n )             { char s[20]; BeginSet(); Sprintf( s,"%d",n );   Alloc(s); }
MyString::MyString( unsigned n )               { char s[20]; BeginSet(); Sprintf( s,"%u",n );   Alloc(s); }
MyString::MyString( signed long n )            { char s[20]; BeginSet(); Sprintf( s,"%ld",n );  Alloc(s); }
MyString::MyString( unsigned long n )          { char s[20]; BeginSet(); Sprintf( s,"%lu",n );  Alloc(s); }
MyString::MyString( float n )                  { char s[20]; BeginSet(); Sprintf( s,"%f",n);    Alloc(s); }
MyString::MyString( double n )                 { char s[20]; BeginSet(); Sprintf( s,"%lf",n);   Alloc(s); }
MyString::MyString( void CT_FAR *n )           { char s[20]; BeginSet(); Sprintf( s,"%lp",n);   Alloc(s); }
MyString::MyString( DWORD n,CONSTSTR Format )  { char s[20]; BeginSet(); Sprintf( s,Format,n ); Alloc(s); }
MyString::MyString( double n,CONSTSTR Format ) { char s[40]; BeginSet(); Sprintf( s,Format,n ); Alloc(s); }
#if defined( __HDOS__ )
MyString::MyString( void CT_NEAR *n )          { char s[20]; BeginSet(); Sprintf( s,"%p",n);   Alloc(s); }
#endif

MyString::MyString( char n )
  {  char s[10];

    BeginSet();
    if ( IsPrintChar(n) || n == '\n' || n == '\t' || n == '\r' || n == '\b' )
      { s[0] = n; s[1] = 0; }
     else
      Sprintf( s,"\\0x%03X",(int)n );
    Alloc(s);
}

int MyString::printf( CONSTSTR fmt,... )
  {  va_list a;
     int     sz;

     SetLength( 0 );

     va_start( a,fmt );
      sz = VSNprintf( NULL,0,fmt,a );
     va_end( a );

     if ( !sz )
       return 0;

     Alloc( sz+1 );

     va_start( a,fmt );
      len = VSNprintf( str,maxchar,fmt,a );
     va_end( a );

 return len;
}

int MyString::vprintf( CONSTSTR fmt,va_list a )
  {  int sz = VSNprintf( NULL,0,fmt,a );
     Alloc( sz+1 );
 return len = VSNprintf( str,maxchar,fmt,a );
}

void MyString::BeginSet( void )
  {
#if defined(DEF_STR_ALLOC)
   str = _Alloc( maxchar = 50 );
   len = 0;
#else
   str = NULL;
   len = maxchar = 0;
#endif
}

CONSTSTR MyString::Alloc( CONSTSTR s,int maxLen )
  {
     if ( maxLen == -1 )
       maxLen = strLen(s);

     Alloc( maxLen+1 );

     if ( !s || !maxLen ) {
       str[len = 0] = 0;
       return c_str();
     }

     strncpy( str, s, maxLen );
     str[ maxLen ] = 0;
     len = strLen(str);

 return Text();
}

CONSTSTR MyString::Alloc( int t )
  {
     if ( t < maxchar ) return Text();

     if ( str )
       str = StrRealloc( str,maxchar = (t+10) );
      else
       str = StrAlloc( maxchar = (t+10) );
 return Text();
}

MyString& MyString::Add( const MyString& s )
  {
    Alloc( Length() + s.Length()+1 );
    strcat( str,s.Text() );
    len += s.Length();
 return *this;
}

MyString& MyString::Add( CONSTSTR s )
  {  int slen = strLen(s);

     if ( slen ) {
       Alloc( Length() + slen+1 );
       strcpy( str+Length(),s );
       len += slen;
     }
 return *this;
}

void MyString::cat( CONSTSTR s,... )
  {  va_list a;
     int     sz;

     if ( !s || !s[0] ) return;

     va_start( a, s );
       sz = VSNprintf( NULL,0,s,a );
     va_end( a );
     if ( !sz ) return;

     Alloc( len+sz+1 );

     va_start( a, s );
       VSNprintf( str+len,sz+1,s,a );
     va_end( a );

     len += sz;
}

void MyString::vcat( CONSTSTR s,va_list a )
  {  int slen, clen;

     if ( !s || !s[0] ) return;

     slen = VSNprintf( NULL,0,s,a );
     if ( !slen ) return;

     clen = Length();

     Alloc( clen + slen + 1 );

     VSNprintf( str+clen,maxchar,s,a );
     len += slen;
}

MyString& MyString::Add( CONSTSTR s, int from, int to /*-1*/ )
  {
    if ( to == -1 )
      return Add( s+from );

    Alloc( len+(to-from+1) );
    StrCpy( str+len, s+from, to-from );

    len = strLen( str );

 return *this;
}

MyString& MyString::Add( char ch )
  {
    if ( ch == 0 ) return *this;
    if ( len + 1 >= maxchar )
      Alloc( len + 10 );

    str[len++] = ch;
    str[len] = 0;
 return *this;
}

void MyString::InsCharPos( int pos,char ch )
  {
    if ( !str ) pos = 0;
    pos = Max(Min(pos,len),0);
    Alloc( Length()+1 );
    if (pos < len) MemMove( str+pos+1,str+pos,len-pos );
    str[pos]   = ch;
    str[++len] = 0;
}

void MyString::Del( int pos, int cn )
  {
    if ( !str || !cn || pos < 0 || pos >= len )
      return;
    cn = Min( len-pos, cn );
    MemMove( str+pos, str+pos+cn, len-pos-cn );
    len -= cn;
    str[len] = 0;
}

void MyString::DelChars( char ch )
  {
    if ( str ) {
      for ( int n = 0; str[n]; n++ )
        if ( str[n] == ch ) {
          MemMove( str+n,str+n+1,len-n );
          len--;
        }
      str[len] = 0;
    }
}

int MyString::RChr( char ch,int pos ) const
  {
     if ( !str || !str[0] ) return -1;
     if ( pos == -1 ) pos = Length()-1;
     for( pos = Max(0,Min(Length(),pos));
          str[pos] && str[pos] != ch;
          pos-- );
 return str[pos]?pos:(-1);
}

char MyString::SetChar( int num,char ch )
  {
    if ( !str || num < 0 || num > maxchar ) return 0;
    str[num] = ch;
    if ( ch && num == len )
      str[++len] = 0;
     else
    if ( !ch && num < len )
      len = strLen(str);
 return ch;
}

#if defined( __VCL__ )
MyString::MyString( const AnsiString& v )                {  BeginSet(); Alloc( v.c_str() ); }
MyString::operator AnsiString(void)                const { return AnsiString(Text()); }
#endif

int       MyString::Chr( char ch,int pos )         const { return StrPosChr( Text(),ch,pos ); }
int       MyString::Str( CONSTSTR s,int pos )      const { return StrPosStr( Text(),s,pos ); }
MyString& MyString::FixSlashChars( char ch )             { if (str) ::FixSlashChars( str, ch ); return *this; }
void      MyString::Case( msCaseTypes type )             { StrCase(Text(),type); }
char     *MyString::c_str( void )                  const { return Text(); }
char     *MyString::Text( void )                   const { return (char*)(str?str:""); }
int       MyString::Length( void )                 const { return len; }
BOOL      MyString::RecalcLength( void )                 { if (str) len = strLen(str); else len = maxchar = 0; return str != NULL; }
void      MyString::SetLength( int sz )                  { if (sz >= 0 && sz < maxchar) { len = sz; str[sz] = 0; } }
CONSTSTR  MyString::Set( CONSTSTR s )                    { return Alloc(s); }
CONSTSTR  MyString::Set( const MyString& s )             { return Alloc(s.Text()); }
MyString& MyString::operator=( const MyString& s )       { Alloc( s.Text() ); return *this; }
MyString& MyString::operator=( CONSTSTR s )              { Alloc( s ); return *this; }
BOOL      MyString::operator!=( const MyString& s )const { return len != s.len || strcmp(Text(),s.Text()) != NULL; }
BOOL      MyString::operator!=( CONSTSTR s )       const { return StrCmp( str, s ) != 0; }
BOOL      MyString::operator==( const MyString& s )const { return len == s.len && strcmp(Text(),s.Text()) == NULL; }
BOOL      MyString::operator==( CONSTSTR s )       const { return StrCmp( s, str ) == 0; }
char      MyString::operator[]( int num )          const { return (num >= 0 && num <= len && str)?str[num]:'\0'; }

CONSTSTR MyString::Set( CONSTSTR s, int from, int to )
  {
    if ( to == -1 ) {
      Alloc( s+from );
      return Text();
    }

    Alloc( to-from+1 );
    StrCpy( str, s+from, to-from );
 return Text();
}

BOOL MyString::Cmp( CONSTSTR s,int count, BOOL isCase ) const
  {
    if ( count < 0 )
      return strcmp( Text(),(s)?s:"" ) == 0;
     else
      return StrCmp( Text(),(s)?s:"",count,isCase ) == 0;
}

MyString& MyString::Trimmed( char ch )   { Trim( ch );  return *this; }
MyString& MyString::RTrimmed( char ch )  { RTrim( ch ); return *this; }
MyString& MyString::LTrimmed( char ch )  { LTrim( ch ); return *this; }

void MyString::LTrim( char ch )
  {  int      n;

     if ( len == 0 ) return;

     for ( n = 0; str[n] && str[n] == ch; n++ );
     if ( n ) MemMove( str,str+n,len-n );
     len-=n; str[len] = 0;
}

void MyString::RTrim( char ch )
  {  int      n;

     if ( len == 0 ) return;

     for ( n = len-1; n >= 0 && str[n] == ch; len--,n-- )
       str[n] = 0;
 return;
}

void MyString::Trim( char ch )
  {  int      n;

     if ( len == 0 ) return;

     for ( n = 0; str[n] && str[n] == ch; n++ );
     if ( n ) MemMove( str,str+n,len-n );
     len-=n; str[len] = 0;
     for ( n = len-1; n >= 0 && str[n] == ch; len--,n-- )
       str[n] = 0;
}

void MyString::MakeAlign( int maxWidth, talTypes align, talTypes *over )
  {  MyString s;

     s.Alloc( maxWidth+2 );
     StrMakeAlign( s.Text(),Text(),maxWidth,align,over );
     Set( s );
}

MyString MyString::SubStr( int start, int end /*= -1*/ ) const
  {  MyString s;

     if ( Length() <= 0 )
       return s;

     if ( end == -1 ) end = Length();
     if ( start >= Length() ||
          end <= start )
       return s;

     start = Max( 0, start );
     end   = Min( Length(), end );
     if ( end <= start )
       return s;

     s.Set( c_str(), start, end+1 );
 return s;
}

MyString& MyString::ChangeChar( char och, char nch )
  {
     if ( och && str )
       for( ; *str; str++ )
         if ( *str == och )
           *str = nch;

 return *this;
}
